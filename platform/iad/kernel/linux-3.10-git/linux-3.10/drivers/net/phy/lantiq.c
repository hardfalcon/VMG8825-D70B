/*
 * PHY driver for various Lantiq PEF7071 compatible PHY devices
 *
 * Copyright (C) 2012-2015 Daniel Schwierzeck <daniel.schwierzeck@gmail.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/phy.h>
#include <linux/of.h>

#define REG_IMASK	0x19	/* interrupt mask */
#define REG_ISTAT	0x1a	/* interrupt status */

#define INT_WOL		BIT(15)	/* Wake-On-LAN */
#define INT_ANE		BIT(11)	/* Auto-Neg error */
#define INT_ANC		BIT(10)	/* Auto-Neg complete */
#define INT_ADSC	BIT(5)	/* Link auto-downspeed detect */
#define INT_DXMC	BIT(2)	/* Duplex mode change */
#define INT_LSPC	BIT(1)	/* Link speed change */
#define INT_LSTC	BIT(0)	/* Link state change */
#define INT_MASK	(INT_LSTC | INT_LSPC | INT_DXMC | INT_ADSC | \
			INT_ANC | INT_ANE)

#define ADVERTISED_MPD	BIT(10)	/* Multi-port device */

#define MMD_DEVAD	0x1f
#define MMD_LED0H	0x1e2
#define MMD_LED0L	0x1e3
#define MMD_LED1H	0x1e4
#define MMD_LED1L	0x1e5
#define MMD_LED2H	0x1e6
#define MMD_LED2L	0x1e7

static int pef7071_of_init(struct phy_device *phydev)
{
	const struct device_node *np = phydev->dev.of_node;
	const __be32 *prop;
	u32 led;

	if (!np)
		return 0;

	prop = of_get_property(np, "lantiq,led0", NULL);
	if (prop) {
		led = be32_to_cpup(prop);
		phy_write_mmd_indirect(phydev, MMD_LED0H, MMD_DEVAD,
			phydev->addr, (led >> 8) & 0xff);
		phy_write_mmd_indirect(phydev, MMD_LED0L, MMD_DEVAD,
			phydev->addr, led & 0xff);
	}

	prop = of_get_property(np, "lantiq,led1", NULL);
	if (prop) {
		led = be32_to_cpup(prop);
		phy_write_mmd_indirect(phydev, MMD_LED1H, MMD_DEVAD,
			phydev->addr, (led >> 8) & 0xff);
		phy_write_mmd_indirect(phydev, MMD_LED1L, MMD_DEVAD,
			phydev->addr, led & 0xff);
	}

	prop = of_get_property(np, "lantiq,led2", NULL);
	if (prop) {
		led = be32_to_cpup(prop);
		phy_write_mmd_indirect(phydev, MMD_LED2H, MMD_DEVAD,
			phydev->addr, (led >> 8) & 0xff);
		phy_write_mmd_indirect(phydev, MMD_LED2L, MMD_DEVAD,
			phydev->addr, led & 0xff);
	}

	return 0;
}

static int pef7071_config_init(struct phy_device *phydev)
{
	/* Mask all interrupts */
	phy_write(phydev, REG_IMASK, 0);

	/* Clear all pending interrupts */
	phy_read(phydev, REG_ISTAT);

	return pef7071_of_init(phydev);
}

static int pef7071_config_aneg(struct phy_device *phydev)
{
	int reg;

	/* Advertise as multi-port device */
	reg = phy_read(phydev, MII_CTRL1000);
	reg |= ADVERTISED_MPD;
	phy_write(phydev, MII_CTRL1000, reg);

	return genphy_config_aneg(phydev);
}

static int pef7071_ack_interrupt(struct phy_device *phydev)
{
	phy_read(phydev, REG_ISTAT);
	return 0;
}

static int pef7071_did_interrupt(struct phy_device *phydev)
{
	int reg = phy_read(phydev, REG_ISTAT);
	return reg & INT_MASK;
}

static int pef7071_config_intr(struct phy_device *phydev)
{
	u16 mask = 0;

	if (phydev->interrupts == PHY_INTERRUPT_ENABLED)
		mask = INT_MASK;

	return phy_write(phydev, REG_IMASK, mask);
}

static struct phy_driver pef7071_driver[] = {
{
	.phy_id		= 0xd565a418,
	.phy_id_mask	= 0xfffffff8,
	.name		= "Lantiq XWAY GPHY (PHY22F)",
	.features	= (PHY_BASIC_FEATURES | SUPPORTED_Pause |
				SUPPORTED_Asym_Pause),
	.flags		= PHY_HAS_INTERRUPT,
	.config_init	= pef7071_config_init,
	.config_aneg	= pef7071_config_aneg,
	.read_status	= genphy_read_status,
	.ack_interrupt	= pef7071_ack_interrupt,
	.did_interrupt	= pef7071_did_interrupt,
	.config_intr	= pef7071_config_intr,
	.driver		= { .owner = THIS_MODULE },
}, {
	.phy_id		= 0xd565a408,
	.phy_id_mask	= 0xfffffff8,
	.name		= "Lantiq XWAY GPHY (PHY11G)",
	.features	= (PHY_GBIT_FEATURES | SUPPORTED_Pause |
				SUPPORTED_Asym_Pause),
	.flags		= PHY_HAS_INTERRUPT,
	.config_init	= pef7071_config_init,
	.config_aneg	= pef7071_config_aneg,
	.read_status	= genphy_read_status,
	.ack_interrupt	= pef7071_ack_interrupt,
	.did_interrupt	= pef7071_did_interrupt,
	.config_intr	= pef7071_config_intr,
	.driver		= { .owner = THIS_MODULE },
}, {
	.phy_id		= 0xd565a400,
	.phy_id_mask	= 0xfffffff8,
	.name		= "Lantiq XWAY PEF7071",
	.features	= (PHY_GBIT_FEATURES | SUPPORTED_Pause |
				SUPPORTED_Asym_Pause),
	.flags		= PHY_HAS_INTERRUPT,
	.config_init	= pef7071_config_init,
	.config_aneg	= pef7071_config_aneg,
	.read_status	= genphy_read_status,
	.ack_interrupt	= pef7071_ack_interrupt,
	.did_interrupt	= pef7071_did_interrupt,
	.config_intr	= pef7071_config_intr,
	.driver		= { .owner = THIS_MODULE },
},
};

static int __init pef7071_init(void)
{
	return phy_drivers_register(pef7071_driver,
		ARRAY_SIZE(pef7071_driver));
}

static void __exit pef7071_exit(void)
{
	phy_drivers_unregister(pef7071_driver,
		ARRAY_SIZE(pef7071_driver));
}

module_init(pef7071_init);
module_exit(pef7071_exit);

MODULE_DESCRIPTION("Lantiq PHY drivers");
MODULE_AUTHOR("Daniel Schwierzeck <daniel.schwierzeck@gmail.com>");
MODULE_LICENSE("GPL");
