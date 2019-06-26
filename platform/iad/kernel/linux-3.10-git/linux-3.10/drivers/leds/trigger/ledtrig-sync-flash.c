/*
 *  LED Sync Flashing Trigger
 *
 *  Copyright (C) 2010-2014 Daniel Schwierzeck <daniel.schwierzeck at sphairon.com>
 *
 *  This file was based on: drivers/led/ledtrig-timer.c
 *      Copyright 2005-2006 Openedhand Ltd.
 *      Author: Richard Purdie <rpurdie@openedhand.com>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/jiffies.h>
#include <linux/timer.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/leds.h>
#include "../leds.h"

struct sync_flash_data
{
	struct list_head next;
	unsigned int period;
	struct led_classdev *led_cdev;
};

static void sync_flash_trig_timer(unsigned long);

static const unsigned int flash_period_min = 2000;
static unsigned int flash_period;
static unsigned int flash_period_new;
static unsigned int flash_tick;
static unsigned int flash_tick_max;
static LIST_HEAD(flash_leds);
static DEFINE_SPINLOCK(flash_leds_lock);
static DEFINE_TIMER(flash_timer, sync_flash_trig_timer, 0, 0);

static void sync_flash_trig_timer(unsigned long data)
{
	struct sync_flash_data *trig_data;
	unsigned int mask;
	int brightness;

	spin_lock(&flash_leds_lock);

	if (flash_period_new && !flash_tick) {
		flash_tick_max = flash_period_min / flash_period_new;
		flash_period = flash_period_new;
		flash_period_new = 0;
	}

	if (0 < flash_period) {
		list_for_each_entry(trig_data, &flash_leds, next) {
			if (!trig_data->period)
				continue;

			mask = flash_tick_max;

			switch (trig_data->period) {
			case 1000:
				mask >>= 1;
				break;
			case 500:
				mask >>= 2;
				break;
			case 250:
				mask >>= 3;
				break;
			default:
				break;
			}

			brightness = (flash_tick & mask) ? LED_FULL : LED_OFF;
			__led_set_brightness(trig_data->led_cdev, brightness);
		}

		flash_tick++;
		if (flash_tick >= flash_tick_max)
			flash_tick = 0;

		mod_timer(&flash_timer, jiffies +
			msecs_to_jiffies(flash_period >> 1));
	}

	spin_unlock(&flash_leds_lock);
}

static void sync_flash_trig_update(void)
{
	const struct sync_flash_data *trig_data;
	unsigned long min_period = flash_period_min;

	spin_lock_bh(&flash_leds_lock);

	list_for_each_entry(trig_data, &flash_leds, next)
		if (min_period > trig_data->period && 0 < trig_data->period)
			min_period = trig_data->period;

	if (flash_period_min != min_period)
		flash_period_new = min_period;

	spin_unlock_bh(&flash_leds_lock);

	if (!timer_pending(&flash_timer))
		mod_timer(&flash_timer, jiffies + 1);
}

static ssize_t sync_flash_trig_period_show(struct device *dev,
			struct device_attribute* attr, char *buf)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct sync_flash_data *trig_data = led_cdev->trigger_data;

	return sprintf(buf, "%u\n", trig_data->period);
}

static ssize_t sync_flash_trig_period_store(struct device *dev,
			struct device_attribute* attr,
			const char *buf, size_t size)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct sync_flash_data *trig_data = led_cdev->trigger_data;
	int ret;
	unsigned long state;

	ret = kstrtoul(buf, 10, &state);
	if (ret)
		return ret;

	if (state % 250 != 0)
		return ret;

	trig_data->period = state;
	sync_flash_trig_update();

	return size;
}

static DEVICE_ATTR(period, 0666, sync_flash_trig_period_show,
				sync_flash_trig_period_store);

static void sync_flash_trig_activate(struct led_classdev *led_cdev)
{
	int ret;
	struct sync_flash_data *trig_data;

	trig_data = kzalloc(sizeof(*trig_data), GFP_KERNEL);
	if (!trig_data)
		return;

	led_cdev->trigger_data = trig_data;

	ret = device_create_file(led_cdev->dev, &dev_attr_period);
	if (ret)
		goto err_kfree;

	trig_data->led_cdev = led_cdev;

	spin_lock_bh(&flash_leds_lock);
	list_add_tail(&trig_data->next, &flash_leds);
	spin_unlock_bh(&flash_leds_lock);

	return;

err_kfree:
	led_cdev->trigger_data = NULL;
	kfree(trig_data);
}

static void sync_flash_trig_deactivate(struct led_classdev *led_cdev)
{
	struct sync_flash_data *trig_data = led_cdev->trigger_data;

	spin_lock_bh(&flash_leds_lock);
	list_del(&trig_data->next);
	spin_unlock_bh(&flash_leds_lock);

	device_remove_file(led_cdev->dev, &dev_attr_period);

	kfree(trig_data);
	led_cdev->trigger_data = NULL;

	sync_flash_trig_update();
}

static struct led_trigger sync_flash_trigger = {
	.name       = "flash",
	.activate   = sync_flash_trig_activate,
	.deactivate = sync_flash_trig_deactivate,
};

static int __init sync_flash_trigger_init(void)
{
	return led_trigger_register(&sync_flash_trigger);
}

static void __exit sync_flash_trigger_exit(void)
{
	del_timer_sync(&flash_timer);
	led_trigger_unregister(&sync_flash_trigger);
}

module_init(sync_flash_trigger_init);
module_exit(sync_flash_trigger_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Daniel Schwierzeck <daniel.schwierzeck@sphairon.com>");
MODULE_DESCRIPTION("LED sync flashing trigger");

