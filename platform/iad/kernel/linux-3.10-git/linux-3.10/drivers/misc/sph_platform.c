/*
 * Copyright (C) 2011-2014 Daniel Schwierzeck <daniel.schwierzeck@gmail.com>
 *
 * This program is free software; you can distribute it and/or modify it
 * under the terms of the GNU General Public License (Version 2) as
 * published by the Free Software Foundation.
 */

#define DEBUG

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/slab.h>
#include <linux/kdev_t.h>

#include <linux/sph_platform.h>
#include <lantiq_soc.h>

enum sph_platform_gpio_defstate {
	GPIO_DEFSTATE_OFF,
	GPIO_DEFSTATE_ON,
	GPIO_DEFSTATE_KEEP,
};

struct sph_platform_gpio {
	struct device *dev;
	int gpio;
	int active_low;
	enum sph_platform_gpio_defstate defstate;
	const char *desc;
};

struct sph_platform {
	struct device *dev;
	struct sph_platform_gpio *gpios;
	int ngpios;
};

static unsigned long base_platform;

static int __init base_platform_setup(char *name)
{
	if (!name)
		return 0;

	base_platform = simple_strtoul(name, NULL, 0);

	pr_info("Sphairon base platform: 0x%08lx\n", base_platform);

	return 0;
}
__setup("base_platform=", base_platform_setup);

static ssize_t base_platform_show(struct class *class,
				struct class_attribute *attr, char *buf)
{
	return sprintf(buf, "0x%08lx\n", base_platform);
}

static struct class_attribute sph_platform_class_attrs[] = {
	__ATTR(base_platform, 0444, base_platform_show, NULL),
	__ATTR_NULL,
};

static struct class sph_platform_class = {
	.name =		"stg_platform",
	.owner =	THIS_MODULE,
	.class_attrs =	sph_platform_class_attrs,
};

static int sph_gpio_get_value(const struct sph_platform_gpio *gpio)
{
	int val = !!gpio_get_value(gpio->gpio) ^ gpio->active_low;
	return val;
}

static void sph_gpio_set_value(const struct sph_platform_gpio *gpio, int value)
{
	int val = (value ? 1 : 0) ^ gpio->active_low;
	gpio_set_value(gpio->gpio, val);
}

static ssize_t sph_platform_value_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct sph_platform_gpio *gpio = dev_get_drvdata(dev);
	int value;

	value = sph_gpio_get_value(gpio);

	return sprintf(buf, "%u\n", value);
}

static ssize_t sph_platform_value_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct sph_platform_gpio *gpio = dev_get_drvdata(dev);
	unsigned long state = simple_strtoul(buf, NULL, 10);

	sph_gpio_set_value(gpio, state);

	return size;
}

static DEVICE_ATTR(value, 0644, sph_platform_value_show, sph_platform_value_store);

static int sph_platform_gpio_match(struct device *dev, const void *data)
{
	struct sph_platform_gpio *gpio = dev_get_drvdata(dev);
	const char *name = data;

	if (strcmp(name, gpio->desc) == 0)
		return 1;

	return 0;
}

int sph_platform_gpio_get_value(const char *name)
{
	struct sph_platform_gpio *gpio;
	struct device *dev;

	dev = class_find_device(&sph_platform_class, NULL, name,
				sph_platform_gpio_match);
	if (!dev)
		return -ENODEV;

	gpio = dev_get_drvdata(dev);

	return sph_gpio_get_value(gpio);
}
EXPORT_SYMBOL(sph_platform_gpio_get_value);

int sph_platform_gpio_set_value(const char *name, int value)
{
	struct sph_platform_gpio *gpio;
	struct device *dev;

	dev = class_find_device(&sph_platform_class, NULL, name,
				sph_platform_gpio_match);
	if (!dev)
		return -ENODEV;

	gpio = dev_get_drvdata(dev);
	sph_gpio_set_value(gpio, value);

	return 0;
}
EXPORT_SYMBOL(sph_platform_gpio_set_value);

static int sph_platform_create_gpios(struct sph_platform *plat)
{
	struct device_node *np = plat->dev->of_node;
	struct device_node *gpio_np;
	struct property *prop;
	struct sph_platform_gpio *gpios, *gpio;
	const char *state;
	enum of_gpio_flags gpio_flags;
	int err, i, status;

	plat->ngpios = of_get_child_count(np);
	if (!plat->ngpios)
		return 0;

	dev_dbg(plat->dev, "ngpios %d\n", plat->ngpios);

	gpios = kzalloc(plat->ngpios * sizeof(*gpio), GFP_KERNEL);
	if (!gpios)
		return -ENOMEM;

	i = 0;
	for_each_child_of_node(np, gpio_np) {
		prop = of_find_property(gpio_np, "gpios", NULL);
		if (!prop) {
			dev_err(plat->dev, "no gpios property found\n");
			err = -EINVAL;
			goto err_out;
		}

		gpio = &gpios[i++];
		gpio->gpio = of_get_gpio_flags(gpio_np, 0, &gpio_flags);
		gpio->active_low = gpio_flags & OF_GPIO_ACTIVE_LOW;
		gpio->desc = of_get_property(gpio_np, "label", NULL);

		state = of_get_property(gpio_np, "default-state", NULL);
		if (state) {
			if (!strcmp(state, "keep"))
				gpio->defstate = GPIO_DEFSTATE_KEEP;
			else if (!strcmp(state, "on"))
				gpio->defstate = GPIO_DEFSTATE_ON;
			else
				gpio->defstate = GPIO_DEFSTATE_OFF;
		}

		dev_dbg(plat->dev, "gpio %d, active_low %d, defstate %d, desc %s\n",
			gpio->gpio, gpio->active_low, gpio->defstate, gpio->desc);

		err = devm_gpio_request(plat->dev, gpio->gpio, gpio->desc);
		if (err)
			goto err_out;

		if (gpio->defstate == GPIO_DEFSTATE_KEEP)
			status = !!gpio_get_value(gpio->gpio) ^ gpio->active_low;
		else
			status = (gpio->defstate == GPIO_DEFSTATE_ON);

		gpio_direction_output(gpio->gpio, gpio->active_low ^ status);
	}

	plat->gpios = gpios;

	return 0;

err_out:
	kfree(gpios);
	return err;
}

static int sph_platform_sysfs_init(struct sph_platform *plat)
{
	struct device *dev;
	struct sph_platform_gpio *gpio;
	int err, i;

	for (i = 0; i < plat->ngpios; i++) {
		gpio = &plat->gpios[i];

		dev = device_create(&sph_platform_class, plat->dev, MKDEV(0, 0),
			gpio, gpio->desc);
		if (IS_ERR(dev))
			return PTR_ERR(dev);

		err = device_create_file(dev, &dev_attr_value);
		if (err) {
			device_unregister(gpio->dev);
			put_device(gpio->dev);
		}

		gpio->dev = dev;
	}

	return 0;
}

static void sph_platform_sysfs_exit(struct sph_platform *plat)
{
	struct sph_platform_gpio *gpio;
	int i;

	for (i = 0; i < plat->ngpios; i++) {
		gpio = &plat->gpios[i];

		if (!gpio->dev)
			continue;

		device_remove_file(gpio->dev, &dev_attr_value);
		device_unregister(gpio->dev);
		put_device(gpio->dev);
	}
}

static const struct of_device_id sph_platform_match[] = {
	{ .compatible = "sphairon,platform", },
	{},
};
MODULE_DEVICE_TABLE(of, sph_platform_match);

static int sph_platform_probe(struct platform_device *pdev)
{
	struct sph_platform *plat;
	int err;

	plat = devm_kzalloc(&pdev->dev, sizeof(*plat), GFP_KERNEL);
	if (!plat)
		return -ENOMEM;

	plat->dev = &pdev->dev;
	platform_set_drvdata(pdev, plat);

	err = sph_platform_create_gpios(plat);
	if (err)
		goto err_out;

	err = sph_platform_sysfs_init(plat);
	if (err)
		goto err_free;

	return 0;

err_free:
	kfree(plat->gpios);
err_out:
	platform_set_drvdata(pdev, NULL);

	return err;
}

static int sph_platform_remove(struct platform_device *pdev)
{
	struct sph_platform *plat = platform_get_drvdata(pdev);

	sph_platform_sysfs_exit(plat);
	kfree(plat->gpios);
	platform_set_drvdata(pdev, NULL);

	return 0;
}

static struct platform_driver sph_platform_driver = {
	.probe = sph_platform_probe,
	.remove = sph_platform_remove,
	.driver = {
		.name = "sph-platform",
		.owner = THIS_MODULE,
		.of_match_table = sph_platform_match,
	},
};

static int __init sph_platform_init(void)
{
	int err;

	err = class_register(&sph_platform_class);
	if (err)
		return err;

	err = platform_driver_register(&sph_platform_driver);
	if (err)
		goto err_unregister;

	return 0;

err_unregister:
	class_unregister(&sph_platform_class);

	return err;
}

static void __exit sph_platform_exit(void)
{
	platform_driver_unregister(&sph_platform_driver);
	class_unregister(&sph_platform_class);
}

module_init(sph_platform_init);
module_exit(sph_platform_exit);

MODULE_DESCRIPTION("Lantiq SPI controller driver");
MODULE_AUTHOR("Daniel Schwierzeck <daniel.schwierzeck@gmail.com>");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:sph-platform");
