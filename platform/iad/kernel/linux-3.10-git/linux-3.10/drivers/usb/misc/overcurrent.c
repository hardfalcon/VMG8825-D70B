/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Copyright (C) 2013-2017 Sphairon GmbH (a ZyXEL company)
 */

#include <linux/module.h>
#include <linux/slab.h>
#include <linux/of_platform.h>
#include <linux/of_gpio.h>
#include <linux/interrupt.h>
#include <linux/notifier.h>
#include <linux/usb/overcurrent.h>

enum usb_oc_state {
	USB_POWEROFF,
	USB_POWERON,
	USB_DEBOUNCING,
};

struct usb_oc_priv {
	struct device *dev;
	struct usb_oc *usb_oc;
	spinlock_t lock;
	struct raw_notifier_head notifier;
	struct tasklet_struct irq_task;
	struct timer_list oc_timer;
	int irq;
	int gpio;
	unsigned int debounce_interval;
	unsigned long debounce_timeout;
	enum usb_oc_state state;
	bool gpio_active_low;
};

static bool __usb_oc_get_port_power(struct usb_oc_priv *priv)
{
	return gpio_get_value(priv->gpio) ^ priv->gpio_active_low;
}

static void __usb_oc_set_port_power(struct usb_oc_priv *priv, bool state)
{
	gpio_set_value(priv->gpio, state ^ priv->gpio_active_low);
}

static void __usb_oc_port_enable(struct usb_oc_priv *priv)
{
	if (priv->state == USB_POWERON)
		return;

	if (priv->state != USB_DEBOUNCING)
		enable_irq(priv->irq);

	priv->state = USB_POWERON;
	if (priv->usb_oc)
		priv->usb_oc->is_powered = true;

	__usb_oc_set_port_power(priv, true);
	raw_notifier_call_chain(&priv->notifier, USB_OC_PORT_POWER, NULL);
}

static void __usb_oc_port_disable(struct usb_oc_priv *priv)
{
	if (priv->state == USB_POWEROFF)
		return;

	if (priv->state != USB_DEBOUNCING)
		disable_irq(priv->irq);

	__usb_oc_set_port_power(priv, false);

	priv->state = USB_POWEROFF;
	if (priv->usb_oc)
		priv->usb_oc->is_powered = false;

	raw_notifier_call_chain(&priv->notifier, USB_OC_PORT_POWER, NULL);
}

static void __usb_oc_port_overcurrent(struct usb_oc_priv *priv)
{
	__usb_oc_set_port_power(priv, false);

	priv->state = USB_POWEROFF;
	if (priv->usb_oc) {
		priv->usb_oc->is_powered = false;
		priv->usb_oc->is_overcurrent = true;
	}

	raw_notifier_call_chain(&priv->notifier, USB_OC_PORT_OVERCURRENT, NULL);
}

static void usb_oc_task(unsigned long data)
{
	struct usb_oc_priv *priv = (struct usb_oc_priv *)data;
	unsigned long now;

	spin_lock(&priv->lock);

	switch (priv->state) {
	case USB_POWERON:
		if (priv->debounce_interval) {
			priv->state = USB_DEBOUNCING;
			now = jiffies;
			priv->debounce_timeout = now +
					msecs_to_jiffies(priv->debounce_interval);
			mod_timer(&priv->oc_timer, priv->debounce_timeout);

			dev_dbg(priv->dev, "%s: on -> debouncing\n", __func__);
			enable_irq(priv->irq);
		} else {
			dev_dbg(priv->dev, "%s: on -> over-current\n", __func__);
			__usb_oc_port_overcurrent(priv);
		}
		break;

	case USB_DEBOUNCING:
		now = jiffies;
		if (time_after(now, priv->debounce_timeout)) {
			dev_dbg(priv->dev, "%s: debouncing -> over-current\n", __func__);
			del_timer(&priv->oc_timer);
			__usb_oc_port_overcurrent(priv);
		} else {
			dev_dbg(priv->dev, "%s: debouncing -> debouncing\n", __func__);
			enable_irq(priv->irq);
		}
		break;

	default:
		break;
	}

	spin_unlock(&priv->lock);
}

static void usb_oc_timer(unsigned long data)
{
	struct usb_oc_priv *priv = (struct usb_oc_priv *)data;

	spin_lock(&priv->lock);

	if (priv->state == USB_DEBOUNCING) {
		dev_dbg(priv->dev, "%s: debouncing -> on\n", __func__);
		priv->state = USB_POWERON;
	}

	spin_unlock(&priv->lock);
}

static ssize_t usb_oc_power_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct usb_oc_priv *priv = dev_get_drvdata(dev);
	return sprintf(buf, "%u\n", __usb_oc_get_port_power(priv));
}

static ssize_t usb_oc_power_store(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t size)
{
	struct usb_oc_priv *priv = dev_get_drvdata(dev);
	unsigned long state = simple_strtoul(buf, NULL, 10);

	dev_info(priv->dev, "set port power %ld\n", state);

	spin_lock_bh(&priv->lock);

	if (state)
		__usb_oc_port_enable(priv);
	else
		__usb_oc_port_disable(priv);

	spin_unlock_bh(&priv->lock);

	return size;
}

static DEVICE_ATTR(power, 0644, usb_oc_power_show, usb_oc_power_store);

static irqreturn_t usb_oc_irq(int irq, void *data)
{
	struct usb_oc_priv *priv = data;

	disable_irq_nosync(irq);
	tasklet_schedule(&priv->irq_task);

	return IRQ_HANDLED;
}

static void devm_usb_oc_release(struct device *dev, void *res)
{
	struct usb_oc *usb_oc = *(struct usb_oc **)res;
	usb_oc_put(usb_oc);
}

static int devm_usb_oc_match(struct device *dev, void *res, void *match_data)
{
	struct usb_oc **ptr = res;
	return *ptr == match_data;
}

struct usb_oc *usb_oc_get(struct device *dev, const char *name)
{
	struct usb_oc_priv *priv;
	struct usb_oc *usb_oc;
	struct device_node *np;
	struct platform_device *pdev;

	np = of_parse_phandle(dev->of_node, name, 0);
	if (!np)
		return ERR_PTR(-ENODEV);

	pdev = of_find_device_by_node(np);
	of_node_put(np);
	if (!pdev)
		return ERR_PTR(-ENODEV);

	priv = platform_get_drvdata(pdev);

	usb_oc = kzalloc(sizeof(*usb_oc), GFP_KERNEL);
	if (!usb_oc) {
		put_device(&pdev->dev);
		return ERR_PTR(-ENOMEM);
	}

	usb_oc->dev = get_device(dev);
	usb_oc->ctrl_dev = &pdev->dev;
	usb_oc->is_powered = __usb_oc_get_port_power(priv);
	usb_oc->is_overcurrent = false;

	spin_lock_bh(&priv->lock);
	priv->usb_oc = usb_oc;
	spin_unlock_bh(&priv->lock);

	return usb_oc;
}
EXPORT_SYMBOL(usb_oc_get);

void usb_oc_put(struct usb_oc *usb_oc)
{
	struct usb_oc_priv *priv;

	if (!usb_oc || IS_ERR(usb_oc))
		return;

	priv = dev_get_drvdata(usb_oc->ctrl_dev);
	spin_lock_bh(&priv->lock);
	priv->usb_oc = NULL;
	spin_unlock_bh(&priv->lock);

	put_device(usb_oc->dev);
	put_device(usb_oc->ctrl_dev);
	kfree(usb_oc);
}
EXPORT_SYMBOL(usb_oc_put);

struct usb_oc *devm_usb_oc_get(struct device *dev, const char *name)
{
	struct usb_oc **ptr, *usb_oc;

	ptr = devres_alloc(devm_usb_oc_release, sizeof(*ptr), GFP_KERNEL);
	if (!ptr)
		return ERR_PTR(-ENOMEM);

	usb_oc = usb_oc_get(dev, name);
	if (!IS_ERR(usb_oc)) {
		*ptr = usb_oc;
		devres_add(dev, ptr);
	} else {
		devres_free(ptr);
	}

	return usb_oc;
}
EXPORT_SYMBOL(devm_usb_oc_get);

void devm_usb_oc_put(struct device *dev, struct usb_oc *usb_oc)
{
	int err;

	if (!usb_oc)
		return;

	err = devres_destroy(dev, devm_usb_oc_release, devm_usb_oc_match, usb_oc);
	dev_WARN_ONCE(dev, err, "couldn't find USB OC resource\n");
}
EXPORT_SYMBOL(devm_usb_oc_put);

int usb_oc_register(struct usb_oc *usb_oc, struct notifier_block *nb)
{
	struct usb_oc_priv *priv = dev_get_drvdata(usb_oc->ctrl_dev);
	return raw_notifier_chain_register(&priv->notifier, nb);
}
EXPORT_SYMBOL(usb_oc_register);

int usb_oc_unregister(struct usb_oc *usb_oc, struct notifier_block *nb)
{
	struct usb_oc_priv *priv = dev_get_drvdata(usb_oc->ctrl_dev);
	return raw_notifier_chain_unregister(&priv->notifier, nb);
}
EXPORT_SYMBOL(usb_oc_unregister);

bool usb_oc_get_port_power(struct usb_oc *usb_oc)
{
	struct usb_oc_priv *priv = dev_get_drvdata(usb_oc->ctrl_dev);
	return __usb_oc_get_port_power(priv);
}
EXPORT_SYMBOL(usb_oc_get_port_power);

void usb_oc_set_port_power(struct usb_oc *usb_oc, bool state)
{
	struct usb_oc_priv *priv = dev_get_drvdata(usb_oc->ctrl_dev);

	dev_info(priv->dev, "set port power %d\n", state);

	spin_lock_bh(&priv->lock);

	if (state)
		__usb_oc_port_enable(priv);
	else
		__usb_oc_port_disable(priv);

	spin_unlock_bh(&priv->lock);
}
EXPORT_SYMBOL(usb_oc_set_port_power);

static int usb_oc_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct usb_oc_priv *priv;
	enum of_gpio_flags gpio_flags;
	unsigned long gpio_req;
	int err;

	priv = devm_kzalloc(&pdev->dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	priv->dev = &pdev->dev;

	priv->irq = platform_get_irq(pdev, 0);
	if (priv->irq < 0) {
		dev_err(&pdev->dev, "missing IRQ number in DT\n");
		return -EINVAL;
	}

	priv->gpio = of_get_gpio_flags(dev->of_node, 0, &gpio_flags);
	if (priv->gpio < 0) {
		dev_err(&pdev->dev, "missing GPIO in DT\n");
		return -EINVAL;
	}
	priv->gpio_active_low = gpio_flags & OF_GPIO_ACTIVE_LOW;

	err = of_property_read_u32(dev->of_node, "debounce-interval",
		&priv->debounce_interval);
	if (err)
		priv->debounce_interval = 0;

	spin_lock_init(&priv->lock);
	RAW_INIT_NOTIFIER_HEAD(&priv->notifier);
	tasklet_init(&priv->irq_task, usb_oc_task, (unsigned long)priv);
	setup_timer(&priv->oc_timer, usb_oc_timer, (unsigned long)priv);
	priv->state = USB_POWEROFF;

	if (priv->gpio_active_low)
		gpio_req = GPIOF_ACTIVE_LOW | GPIOF_OUT_INIT_HIGH;
	else
		gpio_req = GPIOF_OUT_INIT_LOW;

	err = devm_gpio_request_one(dev, priv->gpio, gpio_req,
		dev_name(dev));
	if (err < 0)
		return err;

	err = devm_request_irq(dev, priv->irq, usb_oc_irq, 0,
		dev_name(dev), priv);
	if (err)
		return err;

	if (!priv->debounce_interval)
		disable_irq(priv->irq);

	err = device_create_file(dev, &dev_attr_power);
	if (err)
		return err;

	platform_set_drvdata(pdev, priv);

	dev_info(&pdev->dev, "debounce interval %u\n", priv->debounce_interval);

	return 0;
}

static int usb_oc_remove(struct platform_device *pdev)
{
	struct usb_oc_priv *priv = platform_get_drvdata(pdev);

	__usb_oc_set_port_power(priv, 0);
	tasklet_kill(&priv->irq_task);
	del_timer(&priv->oc_timer);
	device_remove_file(priv->dev, &dev_attr_power);
	platform_set_drvdata(pdev, NULL);

	return 0;
}

static const struct of_device_id usb_oc_of_match[] = {
	{ .compatible = "usb-over-current"},
	{},
};
MODULE_DEVICE_TABLE(of, usb_oc_of_match);

static struct platform_driver usb_oc_driver = {
	.driver = {
		.name = "usb-over-current",
		.owner = THIS_MODULE,
		.of_match_table = usb_oc_of_match,
	},
	.probe = usb_oc_probe,
	.remove = usb_oc_remove,
};
module_platform_driver(usb_oc_driver);

MODULE_AUTHOR("Daniel Schwierzeck <daniel.schwierzeck@gmail.com");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("USB over-current protection");
