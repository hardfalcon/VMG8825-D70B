/*
 * (C) Copyright 2005-2017 Sphairon GmbH (a ZyXEL company)
 *
 * SPDX-License-Identifier: GPL-2.0
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/workqueue.h>
#include <linux/delay.h>
#include <linux/notifier.h>
#include <linux/slab.h>
#include <linux/err.h>
#include <asm/atomic.h>

#if defined(CONFIG_OF)
#include <linux/of.h>
#include <linux/of_gpio.h>
#else
#include <ifx_devices.h>

int strtobool(const char *s, bool *res)
{
	switch (s[0]) {
	case 'y':
	case 'Y':
	case '1':
		*res = true;
		break;
	case 'n':
	case 'N':
	case '0':
		*res = false;
		break;
	default:
		return -EINVAL;
	}
	return 0;
}
#endif

#include "pcm_clk_gen.h"

#define PCM_CLK_GEN_THRESHOLD		10
#define PCM_CLK_GEN_POLL_INTERVAL	5000

enum pcm_intf_mode {
	PCM_INTF_DISABLED,
	PCM_INTF_MASTER,
	PCM_INTF_SLAVE,
};

enum pcm_clk_state {
	PCM_CLK_DISABLED,
	PCM_CLK_ENABLED,
	PCM_CLK_LOST,
	PCM_CLK_SYNC,
	PCM_CLK_GENERATING,
};

struct pcm_clk_gen_data {
	struct device *dev;
	int gpio_pcm_dcl;
	int gpio_pcm_fsc;
	int gpio_pcm_ext;
	bool gpio_pcm_ext_active_low;
	enum pcm_intf_mode mode;
	enum pcm_clk_state state;
	int pcm_dcl_out;
	int pcm_fsc_out;
	unsigned int threshold;
	unsigned int poll_interval;
	struct workqueue_struct *wq;
	struct delayed_work poll_ext_clock;
	struct srcu_notifier_head notifier;
	atomic_t clk_cycles;
#if defined(CONFIG_OF)
	struct pinctrl *pinctrl;
	struct pinctrl_state *pcm_default;
	struct pinctrl_state *pcm_master;
	struct pinctrl_state *pcm_slave;
#endif
};

static void pcm_intf_config(struct pcm_clk_gen_data *data,
				enum pcm_intf_mode mode)
{
	switch (mode) {
	case PCM_INTF_DISABLED:
		dev_dbg(data->dev, "Disable PCM interface\n");
#if defined(CONFIG_OF)
		pinctrl_select_state(data->pinctrl, data->pcm_default);
#else
		gpio_set_pull(data->gpio_pcm_fsc, LTQ_GPIO_PULL_UP);
		gpio_set_pull(data->gpio_pcm_dcl, LTQ_GPIO_PULL_UP);
#endif
		gpio_direction_input(data->gpio_pcm_fsc);
		gpio_direction_input(data->gpio_pcm_dcl);
		break;
	case PCM_INTF_MASTER:
		dev_dbg(data->dev, "Set PCM interface to master mode\n");
#if defined(CONFIG_OF)
		pinctrl_select_state(data->pinctrl, data->pcm_master);
#endif
		gpio_direction_output(data->gpio_pcm_fsc, 0);
		gpio_direction_output(data->gpio_pcm_dcl, 0);
		break;
	case PCM_INTF_SLAVE:
		dev_dbg(data->dev, "Set PCM interface to slave mode\n");
#if defined(CONFIG_OF)
		pinctrl_select_state(data->pinctrl, data->pcm_slave);
#endif
		gpio_direction_input(data->gpio_pcm_fsc);
		gpio_direction_input(data->gpio_pcm_dcl);
		break;
	}

	data->mode = mode;
}

static void pcm_clk_sync_ext_set(struct pcm_clk_gen_data *data, bool state)
{
	gpio_set_value(data->gpio_pcm_ext,
		state ^ data->gpio_pcm_ext_active_low);
}

static bool pcm_clk_sync_ext_get(struct pcm_clk_gen_data *data)
{
	return !!gpio_get_value(data->gpio_pcm_ext) ^
		data->gpio_pcm_ext_active_low;
}

static bool pcm_clk_detect(struct pcm_clk_gen_data *data)
{
	int i;
	int val, oldval;
	int cnt = 0;

	oldval = gpio_get_value(data->gpio_pcm_dcl);
	for (i = 0; i < 400; i++) {
		val = gpio_get_value(data->gpio_pcm_dcl);
		if (val != oldval) {
			cnt++;
			oldval = val;
		}
	}

	return cnt > data->threshold;
}

static void pcm_clk_poll_queue(struct pcm_clk_gen_data *data)
{
	queue_delayed_work(data->wq, &data->poll_ext_clock,
		msecs_to_jiffies(data->poll_interval));
}

static void pcm_clk_poll(struct pcm_clk_gen_data *data)
{
	bool detect = pcm_clk_detect(data);

	switch (data->state) {
	case PCM_CLK_ENABLED:
		if (detect) {
			dev_info(data->dev, "External PCM clocks are applied\n");
			data->state = PCM_CLK_SYNC;
			srcu_notifier_call_chain(&data->notifier, PCM_CLK_EVENT_SYNC, NULL);
		} else {
			dev_info(data->dev, "External PCM clocks are not applied\n");
			data->state = PCM_CLK_LOST;
			srcu_notifier_call_chain(&data->notifier, PCM_CLK_EVENT_LOST, NULL);
		}
		break;
	case PCM_CLK_LOST:
		if (detect) {
			dev_info(data->dev, "External PCM clocks are applied\n");
			data->state = PCM_CLK_SYNC;
			srcu_notifier_call_chain(&data->notifier, PCM_CLK_EVENT_SYNC, NULL);
		}
		break;
	case PCM_CLK_SYNC:
		if (!detect) {
			dev_info(data->dev, "External PCM clocks are lost\n");
			data->state = PCM_CLK_LOST;
			srcu_notifier_call_chain(&data->notifier, PCM_CLK_EVENT_LOST, NULL);
		}
		break;
	default:
		return;
	}

	pcm_clk_poll_queue(data);
}

static void pcm_clk_poll_work(struct work_struct *work)
{
	struct delayed_work *delayed_work =
		container_of(work, struct delayed_work, work);
	struct pcm_clk_gen_data *data =
		container_of(delayed_work, struct pcm_clk_gen_data, poll_ext_clock);

	pcm_clk_poll(data);
}

static void pcm_clk_gen_start(struct pcm_clk_gen_data *data)
{
	int i, pcm_dcl_out = 0;
	bool pcm_ext;

	if (data->state == PCM_CLK_DISABLED || data->state == PCM_CLK_SYNC)
		return;

	cancel_delayed_work_sync(&data->poll_ext_clock);
	data->state = PCM_CLK_GENERATING;

	pcm_ext = pcm_clk_sync_ext_get(data);
	pcm_clk_sync_ext_set(data, false);
	pcm_intf_config(data, PCM_INTF_MASTER);

	for (i = 0; i < 16; i++) {
		if (i == 0)
			gpio_set_value(data->gpio_pcm_fsc, 1);
		else if (i == 1)
			gpio_set_value(data->gpio_pcm_fsc, 0);

		pcm_dcl_out ^= 1;
		gpio_set_value(data->gpio_pcm_dcl, pcm_dcl_out);
		ndelay(10);
	}

	data->state = PCM_CLK_LOST;
	pcm_intf_config(data, PCM_INTF_SLAVE);

	if (pcm_ext)
		pcm_clk_sync_ext_set(data, true);

	pcm_clk_poll_queue(data);
}

static void pcm_clk_gen_enable(struct pcm_clk_gen_data *data)
{
	if (data->state != PCM_CLK_DISABLED)
		return;

	dev_info(data->dev, "Enable external PCM clock detection\n");

	data->state = PCM_CLK_ENABLED;
	pcm_intf_config(data, PCM_INTF_SLAVE);
	pcm_clk_sync_ext_set(data, true);
	pcm_clk_poll(data);
}

static void pcm_clk_gen_disable(struct pcm_clk_gen_data *data)
{
	if (data->state == PCM_CLK_DISABLED)
		return;

	dev_info(data->dev, "Disable external PCM clock detection\n");

	data->state = PCM_CLK_DISABLED;
	cancel_delayed_work_sync(&data->poll_ext_clock);
	pcm_intf_config(data, PCM_INTF_DISABLED);
	pcm_clk_sync_ext_set(data, false);
}

static ssize_t pcm_clk_mode_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct pcm_clk_gen_data *data = dev_get_drvdata(dev);

	return sprintf(buf, "%d\n", data->mode);
}

static ssize_t pcm_clk_mode_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct pcm_clk_gen_data *data = dev_get_drvdata(dev);
	unsigned long mode = simple_strtoul(buf, NULL, 10);

	pcm_intf_config(data, mode);

	return size;
}

static ssize_t pcm_clk_enable_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct pcm_clk_gen_data *data = dev_get_drvdata(dev);

	return sprintf(buf, "%d\n", data->state != PCM_CLK_DISABLED);
}

static ssize_t pcm_clk_enable_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct pcm_clk_gen_data *data = dev_get_drvdata(dev);
	bool state;

	if (strtobool(buf, &state) < 0)
		return -EINVAL;

	if (state)
		pcm_clk_gen_enable(data);
	else
		pcm_clk_gen_disable(data);

	return size;
}

static ssize_t pcm_clk_state_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct pcm_clk_gen_data *data = dev_get_drvdata(dev);

	return sprintf(buf, "%d\n", data->state);
}

static ssize_t pcm_clk_generate_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct pcm_clk_gen_data *data = dev_get_drvdata(dev);

	return sprintf(buf, "%d\n", data->state == PCM_CLK_GENERATING);
}

static ssize_t pcm_clk_generate_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct pcm_clk_gen_data *data = dev_get_drvdata(dev);
	bool state;

	if (strtobool(buf, &state) < 0)
		return -EINVAL;

	if (data->state == PCM_CLK_DISABLED) {
		dev_err(data->dev, "Driver is disabled\n");
		return size;
	}

	if (data->state == PCM_CLK_SYNC) {
		dev_err(data->dev, "External PCM clocks are applied\n");
		return size;
	}

	if (state)
		pcm_clk_gen_start(data);

	return size;
}

static ssize_t pcm_clk_threshold_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct pcm_clk_gen_data *data = dev_get_drvdata(dev);

	return sprintf(buf, "%d\n", data->threshold);
}

static ssize_t pcm_clk_threshold_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct pcm_clk_gen_data *data = dev_get_drvdata(dev);
	char *end;
	unsigned long var = simple_strtoul(buf, &end, 0);

	if (end == buf)
		return -EINVAL;

	data->threshold = var;

	return size;
}

static ssize_t pcm_clk_poll_interval_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct pcm_clk_gen_data *data = dev_get_drvdata(dev);

	return sprintf(buf, "%d\n", data->poll_interval);
}

static ssize_t pcm_clk_poll_interval_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct pcm_clk_gen_data *data = dev_get_drvdata(dev);
	char *end;
	unsigned long var = simple_strtoul(buf, &end, 0);

	if (end == buf)
		return -EINVAL;

	data->poll_interval = var;

	return size;
}

static ssize_t pcm_clk_sync_ext_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct pcm_clk_gen_data *data = dev_get_drvdata(dev);

	return sprintf(buf, "%d\n", pcm_clk_sync_ext_get(data));
}

static ssize_t pcm_clk_sync_ext_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct pcm_clk_gen_data *data = dev_get_drvdata(dev);
	bool state;

	if (strtobool(buf, &state) < 0)
		return -EINVAL;

	pcm_clk_sync_ext_set(data, state);

	return size;
}

static struct device_attribute pcm_clk_gen_dev_attrs[] = {
	__ATTR(mode, S_IWUSR | S_IRUGO,
		pcm_clk_mode_show, pcm_clk_mode_store),
	__ATTR(enable, S_IWUSR | S_IRUGO,
		pcm_clk_enable_show, pcm_clk_enable_store),
	__ATTR(state, S_IRUGO,
		pcm_clk_state_show, NULL),
	__ATTR(generate, S_IWUSR | S_IRUGO,
		pcm_clk_generate_show, pcm_clk_generate_store),
	__ATTR(threshold, S_IWUSR | S_IRUGO,
		pcm_clk_threshold_show, pcm_clk_threshold_store),
	__ATTR(poll_interval, S_IWUSR | S_IRUGO,
		pcm_clk_poll_interval_show, pcm_clk_poll_interval_store),
	__ATTR(sync_ext, S_IWUSR | S_IRUGO,
		pcm_clk_sync_ext_show, pcm_clk_sync_ext_store),
	__ATTR_NULL,
};

static void pcm_class_release(struct class *cls)
{
}

static struct class pcm_class = {
	.name = "pcm",
	.owner = THIS_MODULE,
	.dev_attrs = pcm_clk_gen_dev_attrs,
	.class_release = pcm_class_release,
};

#if defined(CONFIG_OF)
static int pcm_clk_gen_init_of(struct device *dev, struct pcm_clk_gen_data *data)
{
	struct device_node *np = dev->of_node;
	enum of_gpio_flags gpio_flags;
	int err;

	err = of_get_named_gpio_flags(np, "gpio-pcm-dcl", 0, NULL);
	if (err < 0)
		return err;
	data->gpio_pcm_dcl = err;

	err = of_get_named_gpio_flags(np, "gpio-pcm-fsc", 0, NULL);
	if (err < 0)
		return err;
	data->gpio_pcm_fsc = err;

	err = of_get_named_gpio_flags(np, "gpio-pcm-ext", 0, &gpio_flags);
	if (err < 0)
		return err;
	data->gpio_pcm_ext = err;
	data->gpio_pcm_ext_active_low = gpio_flags & OF_GPIO_ACTIVE_LOW;

	data->pinctrl = devm_pinctrl_get(dev);
	if (IS_ERR(data->pinctrl))
		return PTR_ERR(data->pinctrl);

	data->pcm_default = pinctrl_lookup_state(data->pinctrl, "default");
	if (IS_ERR(data->pcm_default))
		return PTR_ERR(data->pcm_default);

	data->pcm_master = pinctrl_lookup_state(data->pinctrl, "master");
	if (IS_ERR(data->pcm_master))
		return PTR_ERR(data->pcm_master);

	data->pcm_slave = pinctrl_lookup_state(data->pinctrl, "slave");
	if (IS_ERR(data->pcm_slave))
		return PTR_ERR(data->pcm_slave);

	return 0;
}
#else
static int pcm_clk_gen_init_platdata(struct device *dev,
					struct pcm_clk_gen_data *data)
{
	const struct pcm_clk_gen_platform_data *pdata = dev->platform_data;

	if (!pdata) {
		dev_err(dev, "Missing platform data\n");
		return -EINVAL;
	}

	data->gpio_pcm_dcl = pdata->pcm_dcl_gpio;
	data->gpio_pcm_fsc = pdata->pcm_fsc_gpio;
	data->gpio_pcm_ext = pdata->pcm_ext_gpio;
	data->gpio_pcm_ext_active_low = pdata->pcm_ext_active_low;

	return 0;
}
#endif

static int pcm_clk_gen_probe(struct platform_device *pdev)
{
	struct pcm_clk_gen_data *data;
	struct device *dev;
	int err;

	data = devm_kzalloc(&pdev->dev, sizeof(*data), GFP_KERNEL);
	if (!data)
		return -ENOMEM;

	data->dev = &pdev->dev;
	platform_set_drvdata(pdev, data);

	data->threshold = PCM_CLK_GEN_THRESHOLD;
	data->poll_interval = PCM_CLK_GEN_POLL_INTERVAL;

#if defined(CONFIG_OF)
	err = pcm_clk_gen_init_of(&pdev->dev, data);
#else
	err = pcm_clk_gen_init_platdata(&pdev->dev, data);
#endif
	if (err)
		return err;

	dev = device_create(&pcm_class, &pdev->dev, 0, data, "clk-gen");
	if (IS_ERR(dev))
		return PTR_ERR(dev);

	err = gpio_request(data->gpio_pcm_dcl, "tdm-dcl");
	if (err)
		goto err_device;

	err = gpio_request(data->gpio_pcm_fsc, "tdm-fsc");
	if (err)
		goto err_gpio_dcl;

	err = gpio_request(data->gpio_pcm_ext, "pcm-sync-ext");
	if (err)
		goto err_gpio_fsc;

	gpio_direction_output(data->gpio_pcm_ext,
		0 ^ data->gpio_pcm_ext_active_low);

	data->wq = create_singlethread_workqueue("pcm_clk_gen_wq");
	if (!data->wq) {
		err = -ENOMEM;
		goto err_gpio_ext;
	}

	INIT_DELAYED_WORK(&data->poll_ext_clock, pcm_clk_poll_work);
	srcu_init_notifier_head(&data->notifier);

	data->state = PCM_CLK_DISABLED;
	pcm_intf_config(data, PCM_INTF_DISABLED);

	return 0;

err_gpio_ext:
	gpio_free(data->gpio_pcm_ext);
err_gpio_fsc:
	gpio_free(data->gpio_pcm_fsc);
err_gpio_dcl:
	gpio_free(data->gpio_pcm_dcl);
err_device:
	device_destroy(&pcm_class, 0);
	return err;
}

static int pcm_clk_gen_remove(struct platform_device *pdev)
{
	struct pcm_clk_gen_data *data = platform_get_drvdata(pdev);

	pcm_intf_config(data, PCM_INTF_DISABLED);
	pcm_clk_sync_ext_set(data, false);

	srcu_cleanup_notifier_head(&data->notifier);
	cancel_delayed_work_sync(&data->poll_ext_clock);
	destroy_workqueue(data->wq);
	gpio_free(data->gpio_pcm_ext);
	gpio_free(data->gpio_pcm_dcl);
	gpio_free(data->gpio_pcm_fsc);
	device_destroy(&pcm_class, 0);
	platform_set_drvdata(pdev, NULL);

	return 0;
}

#if defined(CONFIG_OF)
static const struct of_device_id pcm_clk_gen_match[] = {
	{ .compatible = "sphairon,pcm-clk-gen" },
	{},
};
MODULE_DEVICE_TABLE(of, pcm_clk_gen_match);
#endif

static struct platform_driver pcm_clk_gen_driver = {
	.driver = {
		.name = "pcm-clk-gen",
		.owner = THIS_MODULE,
#if defined(CONFIG_OF)
		.of_match_table = pcm_clk_gen_match,
#endif
	},
	.probe = pcm_clk_gen_probe,
	.remove = pcm_clk_gen_remove,
};

#if defined(CONFIG_OF)
static int pcm_clk_match(struct device *dev, const void *data)
#else
static int pcm_clk_match(struct device *dev, void *data)
#endif
{
	const char *name = data;

	if (strcmp(dev_name(dev), name) == 0)
		return 1;

	return 0;
}

static struct pcm_clk_gen_data *pcm_clk_gen_get(void)
{
	struct device *dev, *parent;

	dev = class_find_device(&pcm_class, NULL, "clk-gen", pcm_clk_match);
	if (!dev)
		return NULL;

	parent = get_device(dev->parent);
	put_device(dev);

	return dev_get_drvdata(dev);
}

static void pcm_clk_gen_put(struct pcm_clk_gen_data *data)
{
	put_device(data->dev);
}

bool pcm_clk_start(void)
{
	struct pcm_clk_gen_data *data = pcm_clk_gen_get();

	if (!data)
		return false;

	if (data->state == PCM_CLK_DISABLED) {
		pcm_clk_gen_put(data);
		return false;
	}

	pcm_clk_gen_start(data);
	pcm_clk_gen_put(data);

	return true;
}

int pcm_clk_register_cb(struct notifier_block *nb)
{
	struct pcm_clk_gen_data *data = pcm_clk_gen_get();
	int err;

	if (!data)
		return -ENODEV;

	err = srcu_notifier_chain_register(&data->notifier, nb);
	pcm_clk_gen_put(data);
	return err;
}

int pcm_clk_unregister_cb(struct notifier_block *nb)
{
	struct pcm_clk_gen_data *data = pcm_clk_gen_get();
	int err;

	if (!data)
		return -ENODEV;

	err = srcu_notifier_chain_unregister(&data->notifier, nb);
	pcm_clk_gen_put(data);
	return err;
}

int pcm_clk_gen_init(bool enable)
{
	struct pcm_clk_gen_data *data;
	int err;

	err = class_register(&pcm_class);
	if (err)
		return err;

	err = platform_driver_register(&pcm_clk_gen_driver);
	if (err)
		class_unregister(&pcm_class);

	data = pcm_clk_gen_get();
	if (!data)
		return 0;

	if (enable)
		pcm_clk_gen_enable(data);

	pcm_clk_gen_put(data);
	return err;
}

void pcm_clk_gen_exit(void)
{
	platform_driver_unregister(&pcm_clk_gen_driver);
	class_unregister(&pcm_class);
}
