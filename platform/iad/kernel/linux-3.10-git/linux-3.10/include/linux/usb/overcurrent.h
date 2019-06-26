/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Copyright (C) 2013-2017 Sphairon GmbH (a ZyXEL company)
 */

#ifndef __USB_OVERCURRENT_H__
#define __USB_OVERCURRENT_H__

enum usb_oc_event {
	USB_OC_NONE,
	USB_OC_PORT_OVERCURRENT,
	USB_OC_PORT_POWER,
};

struct device;
struct notifier_block;

struct usb_oc {
	struct device *dev;
	struct device *ctrl_dev;
	spinlock_t lock;
	bool is_powered;
	bool is_overcurrent;
};

struct usb_oc *usb_oc_get(struct device *dev, const char *name);
void usb_oc_put(struct usb_oc *usb_oc);

struct usb_oc *devm_usb_oc_get(struct device *dev, const char *name);
void devm_usb_oc_put(struct device *dev, struct usb_oc *usb_oc);

int usb_oc_register(struct usb_oc *usb_oc, struct notifier_block *nb);
int usb_oc_unregister(struct usb_oc *usb_oc, struct notifier_block *nb);

bool usb_oc_get_port_power(struct usb_oc *usb_oc);
void usb_oc_set_port_power(struct usb_oc *usb_oc, bool state);

static inline bool usb_oc_port_is_powered(const struct usb_oc *usb_oc)
{
	return usb_oc->is_powered;
}

static inline bool usb_oc_port_is_overcurrent(const struct usb_oc *usb_oc)
{
	return usb_oc->is_overcurrent;
}

#endif /* __USB_OVERCURRENT_H__ */
