/*
 * Copyright (C) 2011-2014 Daniel Schwierzeck <daniel.schwierzeck@gmail.com>
 *
 * This program is free software; you can distribute it and/or modify it
 * under the terms of the GNU General Public License (Version 2) as
 * published by the Free Software Foundation.
 */

#ifndef __SPH_PLATFORM_H__
#define __SPH_PLATFORM_H__

extern int sph_platform_gpio_get_value(const char *name);
extern int sph_platform_gpio_set_value(const char *name, int value);

#endif /* __SPH_PLATFORM_H__ */
