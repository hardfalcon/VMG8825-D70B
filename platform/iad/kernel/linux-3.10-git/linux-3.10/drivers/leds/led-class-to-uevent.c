/*
 * @file led-class-to-uevent.c
 * @brief led driver which only transform /sys/class/leds/<xyc> access
 * uevent for further processing
 *
 * @copyright 2017 Sphairon GmbH (a Zyxel company)
 *
 * SPDX-License-Identifier: Zyxel
 *
 */

#include <linux/module.h>
#include <linux/slab.h>
#include <linux/leds.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>

struct uevent_led_data {
	struct led_classdev cdev;
	const char *backend_desc;
};

struct uevent_leds {
	int num_leds;
	struct uevent_led_data leds[];
};

static ssize_t backend_show(struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct uevent_led_data *led_data =
		container_of(led_cdev, struct uevent_led_data, cdev);

	return sprintf(buf, "%s\n", led_data->backend_desc);
}
DEVICE_ATTR(backend, 0444, backend_show, NULL);

static enum led_brightness uevent_led_get(struct led_classdev *cdev)
{
	return cdev->brightness;
}

static void uevent_led_set(struct led_classdev *cdev,
						   enum led_brightness b)
{
	char *event = NULL;
	char *uevent[2];

	dev_dbg(cdev->dev,
			"set stub brightness to %d\n",
			b);
	cdev->brightness = b;

	event = kasprintf(GFP_KERNEL, "BRIGHTNESS=%d", cdev->brightness);

	if (event) {
		uevent[0] = event;
		uevent[1] = NULL;

		kobject_uevent_env(&cdev->dev->kobj,
						   KOBJ_CHANGE,
						   uevent);
		kfree(event);
	}
}

static inline int sizeof_uevent_led_list(int num_leds)
{
	return sizeof(struct uevent_leds) +
		(sizeof(struct uevent_led_data) * num_leds);
}

static int uevent_led_probe(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	struct device_node *led_node;
	struct uevent_leds *led_list;
	struct uevent_led_data *led_data;
	int led_count = 0;
	struct led_classdev *cdev;

	led_count = of_get_child_count(np);

	dev_info(&pdev->dev, "LED: %d uevent leds defined in devicetree\n",
			 led_count);

	led_list = devm_kzalloc(&pdev->dev,
		sizeof_uevent_led_list(led_count),
		GFP_KERNEL);

	if (!led_list && led_count > 0)
		return -ENOMEM;

	led_list->num_leds = 0;

	for_each_child_of_node(np, led_node) {
		led_data = &led_list->leds[led_list->num_leds++];
		cdev = &led_data->cdev;

		cdev->name = of_get_property(led_node, "label", NULL);
		cdev->brightness_get = uevent_led_get;
		cdev->brightness_set = uevent_led_set;
		led_data->backend_desc =
			of_get_property(led_node, "backend", NULL);

		if (led_classdev_register(&pdev->dev, cdev) < 0) {
			dev_err(&pdev->dev, "failed to register uevent led '%s'",
					cdev->name);
			break;
		}

		dev_info(&pdev->dev, "LED: registered uevent LED '%s', backend '%s'\n",
				 cdev->name,
				 led_data->backend_desc);

		device_create_file(cdev->dev, &dev_attr_backend);
	}

	platform_set_drvdata(pdev, led_list);

	return 0;
}

static int uevent_led_remove(struct platform_device *pdev)
{
	struct uevent_leds *led_list = platform_get_drvdata(pdev);
	struct led_classdev *cdev;
	int i;

	if (led_list) {
		for (i = 0; i < led_list->num_leds; i++) {
			cdev = &led_list->leds[i].cdev;
			dev_info(&pdev->dev,
					 "unregister uevent led '%s'\n",
					 cdev->name);
			led_classdev_unregister(cdev);
			device_remove_file(cdev->dev, &dev_attr_backend);
		}
	}

	platform_set_drvdata(pdev, NULL);

	return 0;
}


static const struct of_device_id of_uevent_leds_match[] = {
	{ .compatible = "uevent-leds", },
};

static struct platform_driver uevent_led_driver = {
	.probe		= uevent_led_probe,
	.remove		= uevent_led_remove,
	.driver		= {
		.name	= "uevent-leds",
		.owner	= THIS_MODULE,
		.of_match_table = of_match_ptr(of_uevent_leds_match),
	},
};

module_platform_driver(uevent_led_driver);

MODULE_AUTHOR("Martin Volkmer <martin.volkmer@zyxel.eu>");
MODULE_DESCRIPTION("uevent LED driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:leds-uevent");
