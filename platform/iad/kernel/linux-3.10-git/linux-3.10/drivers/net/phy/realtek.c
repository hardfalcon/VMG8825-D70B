/*
 * drivers/net/phy/realtek.c
 *
 * Driver for Realtek PHYs
 *
 * Author: Johnson Leung <r58129@freescale.com>
 *
 * Copyright (c) 2004 Freescale Semiconductor, Inc.
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 */
#include <linux/bitops.h>
#include <linux/phy.h>
#include <linux/module.h>

#define RTL821x_PHYSR				0x11
#define RTL821x_PHYSR_DUPLEX			BIT(13)
#define RTL821x_PHYSR_SPEED			GENMASK(15, 14)

#define RTL821x_INER				0x12
#define RTL8211B_INER_INIT			0x6400
#define RTL8211E_INER_LINK_STATUS		BIT(10)
#define RTL8211F_INER_LINK_STATUS		BIT(4)

#define RTL821x_INSR				0x13

#define RTL821x_PAGE_SELECT			0x1f

#define RTL8211F_INSR				0x1d
#define RTL8211F_SERDES_INER			0x11
#define RTL8211F_SERDES_INSR			0x12

#define RTL8211F_TX_DELAY			BIT(8)
#define RTL8211F_RX_DELAY			BIT(3)

#define RTL8201F_ISR				0x1e
#define RTL8201F_IER				0x13

#define RTL8211F_MODE				0x10
#define RTL8211F_CFG_MODE			0xf

/**
 * enum rtl8211f_cfg_mode - PHY CFG mode latched from strap pins
 * @RTL8211F_MODE_UTP_RGMII: UTP (media) <-> RGMII (MAC).
 * @RTL8211F_MODE_FIBER_RGMII: Fiber (media) <-> RGMII (MAC).
 * @RTL8211F_MODE_AUTO_RGMII: UTP/Fiber (media) <-> RGMII (MAC) with media
 * 	auto detection.
 * @RTL8211F_MODE_UTP_SGMII: UTP (media) <-> SGMII (MAC).
 * @RTL8211F_MODE_SGMII_PHY_RGMII_MAC: SGMII (PHY) <-> RGMII (MAC) bridge mode.
 * @RTL8211F_MODE_SGMII_MAC_RGMII_PHY: RGMII (MAC/PHY) <-> SGMII (MAC) bridge mode.
 * @RTL8211F_MODE_UTP_FIBER_AUTO: UTP <-> Fiber media converter
 * @RTL8211F_MODE_UTP_FIBER_FORCED: UTP <-> Fiber media converter
 */
enum rtl8211f_cfg_mode {
	RTL8211F_MODE_UTP_RGMII			= 0,
	RTL8211F_MODE_FIBER_RGMII		= 1,
	RTL8211F_MODE_AUTO_RGMII		= 2,
	RTL8211F_MODE_UTP_SGMII			= 3,
	RTL8211F_MODE_SGMII_PHY_RGMII_MAC	= 4,
	RTL8211F_MODE_SGMII_MAC_RGMII_PHY	= 5,
	RTL8211F_MODE_UTP_FIBER_AUTO		= 6,
	RTL8211F_MODE_UTP_FIBER_FORCED		= 7,
	RTL8211F_MODE_RESERVED
};

MODULE_DESCRIPTION("Realtek PHY driver");
MODULE_AUTHOR("Johnson Leung");
MODULE_LICENSE("GPL");

static int rtl8211x_page_read(struct phy_device *phydev, u16 page, u16 address)
{
	int ret;

	ret = phy_write(phydev, RTL821x_PAGE_SELECT, page);
	if (ret)
		return ret;

	ret = phy_read(phydev, address);

	/* restore to default page 0 */
	phy_write(phydev, RTL821x_PAGE_SELECT, 0x0);

	return ret;
}

static int rtl8211x_page_write(struct phy_device *phydev, u16 page,
			       u16 address, u16 val)
{
	int ret;

	ret = phy_write(phydev, RTL821x_PAGE_SELECT, page);
	if (ret)
		return ret;

	ret = phy_write(phydev, address, val);

	/* restore to default page 0 */
	phy_write(phydev, RTL821x_PAGE_SELECT, 0x0);

	return ret;
}

static int rtl8211f_read_cfg_mode(struct phy_device *phydev)
{
	int ret, mode;

	ret = rtl8211x_page_read(phydev, 0xd40, RTL8211F_MODE);
	if (ret < 0)
		return ret;

	/* register contains 4 bits whereas mode is latched from 3 strap pins */
	mode = ret & RTL8211F_CFG_MODE;
	if (mode >= RTL8211F_MODE_RESERVED)
		return -EINVAL;

	return mode;
}

static bool rtl8211f_media_is_utp(enum rtl8211f_cfg_mode mode)
{
	return mode == RTL8211F_MODE_UTP_RGMII ||
		mode == RTL8211F_MODE_UTP_SGMII;
}

static bool rtl8211f_media_is_fiber(enum rtl8211f_cfg_mode mode)
{
	return mode == RTL8211F_MODE_FIBER_RGMII;
}

static bool rtl8211f_media_is_auto(enum rtl8211f_cfg_mode mode)
{
	return mode == RTL8211F_MODE_AUTO_RGMII;
}

static int rtl8201_ack_interrupt(struct phy_device *phydev)
{
	int err;

	err = phy_read(phydev, RTL8201F_ISR);

	return (err < 0) ? err : 0;
}

static int rtl821x_ack_interrupt(struct phy_device *phydev)
{
	int err;

	err = phy_read(phydev, RTL821x_INSR);

	return (err < 0) ? err : 0;
}

static int rtl8211f_ack_interrupt(struct phy_device *phydev)
{
	int err;

	err = rtl8211x_page_read(phydev, 0xa43, RTL8211F_INSR);
	if (err < 0)
		return err;

	err = rtl8211x_page_read(phydev, 0xde1, RTL8211F_SERDES_INSR);
	return (err < 0) ? err : 0;
}

static int rtl8201_config_intr(struct phy_device *phydev)
{
	u16 val;

	if (phydev->interrupts == PHY_INTERRUPT_ENABLED)
		val = BIT(13) | BIT(12) | BIT(11);
	else
		val = 0;

	return rtl8211x_page_write(phydev, 0x7, RTL8201F_IER, val);
}

static int rtl8211b_config_intr(struct phy_device *phydev)
{
	int err;

	if (phydev->interrupts == PHY_INTERRUPT_ENABLED)
		err = phy_write(phydev, RTL821x_INER,
				RTL8211B_INER_INIT);
	else
		err = phy_write(phydev, RTL821x_INER, 0);

	return err;
}

static int rtl8211e_config_intr(struct phy_device *phydev)
{
	int err;

	if (phydev->interrupts == PHY_INTERRUPT_ENABLED)
		err = phy_write(phydev, RTL821x_INER,
				RTL8211E_INER_LINK_STATUS);
	else
		err = phy_write(phydev, RTL821x_INER, 0);

	return err;
}

static int rtl8211f_config_intr(struct phy_device *phydev)
{
	u16 val;
	int err, mode;

	if (phydev->interrupts == PHY_INTERRUPT_ENABLED)
		val = RTL8211F_INER_LINK_STATUS;
	else
		val = 0;

	err = rtl8211x_page_write(phydev, 0xa42, RTL821x_INER, val);
	if (err)
		return err;

	mode = rtl8211f_read_cfg_mode(phydev);
	if (mode < 0)
		return mode;

	if (rtl8211f_media_is_fiber(mode))
		return rtl8211x_page_write(phydev, 0xde1,
			RTL8211F_SERDES_INER, val);

	return 0;
}

static int rtl8211f_read_status(struct phy_device *phydev)
{
	int err, estatus, lpa, adv, lpagb = 0;

	/* Update the link, but return if there
	 * was an error */
	err = genphy_update_link(phydev);
	if (err)
		return err;

	if (AUTONEG_ENABLE != phydev->autoneg) {
		int bmcr = phy_read(phydev, MII_BMCR);
		if (bmcr < 0)
			return bmcr;

		if (bmcr & BMCR_FULLDPLX)
			phydev->duplex = DUPLEX_FULL;
		else
			phydev->duplex = DUPLEX_HALF;

		if (bmcr & BMCR_SPEED1000)
			phydev->speed = SPEED_1000;
		else if (bmcr & BMCR_SPEED100)
			phydev->speed = SPEED_100;
		else
			phydev->speed = SPEED_10;

		phydev->pause = phydev->asym_pause = 0;
		return 0;
	}

	lpa = phy_read(phydev, MII_LPA);
	if (lpa < 0)
		return lpa;

	adv = phy_read(phydev, MII_ADVERTISE);
	if (adv < 0)
		return adv;

	estatus = phy_read(phydev, MII_ESTATUS);
	if (estatus < 0)
		return estatus;

	lpa &= adv;
	phydev->duplex = DUPLEX_HALF;
	phydev->pause = phydev->asym_pause = 0;

	if (estatus & ESTATUS_1000_XFULL) {
		phydev->speed = SPEED_1000;

		if (lpa & LPA_1000XFULL)
			phydev->duplex = DUPLEX_FULL;

		if (phydev->duplex == DUPLEX_FULL){
			phydev->pause = lpa & LPA_1000XPAUSE ? 1 : 0;
			phydev->asym_pause = lpa & LPA_1000XPAUSE_ASYM ? 1 : 0;
		}

		return 0;
	}

	if (phydev->supported & (SUPPORTED_1000baseT_Half
				| SUPPORTED_1000baseT_Full)) {
		int advgb;

		lpagb = phy_read(phydev, MII_STAT1000);
		if (lpagb < 0)
			return lpagb;

		advgb = phy_read(phydev, MII_CTRL1000);
		if (advgb < 0)
			return advgb;

		lpagb &= advgb << 2;
	}

	phydev->speed = SPEED_10;

	if (lpagb & (LPA_1000FULL | LPA_1000HALF)) {
		phydev->speed = SPEED_1000;

		if (lpagb & LPA_1000FULL)
			phydev->duplex = DUPLEX_FULL;
	} else if (lpa & (LPA_100FULL | LPA_100HALF)) {
		phydev->speed = SPEED_100;

		if (lpa & LPA_100FULL)
			phydev->duplex = DUPLEX_FULL;
	} else
		if (lpa & LPA_10FULL)
			phydev->duplex = DUPLEX_FULL;

	if (phydev->duplex == DUPLEX_FULL){
		phydev->pause = lpa & LPA_PAUSE_CAP ? 1 : 0;
		phydev->asym_pause = lpa & LPA_PAUSE_ASYM ? 1 : 0;
	}

	return 0;
}

static int rtl8211f_config_advert(struct phy_device *phydev,
					enum rtl8211f_cfg_mode mode)
{
	int oldadv, adv;
	int err, changed = 0;

	/*
	 * Do not touch PHY advertisement registers in UTP/Fiber auto mode
	 * because there are conflicting bits between BaseT and BaseX. Thus use
	 * the PHY default register values to advertise the following:
	 * - 10Base-T (full)
	 * - 10Base-T (half)
	 * - 100Base-TX (full)
	 * - 100Base-TX (half)
	 * - 1000Base-T (full)
	 * - 1000Base-X (full)
	 */
	if (rtl8211f_media_is_auto(mode))
		return 0;

	/* Setup standard advertisement */
	oldadv = adv = phy_read(phydev, MII_ADVERTISE);
	if (adv < 0)
		return adv;

	if (rtl8211f_media_is_fiber(mode)) {
		adv &= ~(ADVERTISE_1000XHALF | ADVERTISE_1000XFULL |
			 ADVERTISE_1000XPAUSE | ADVERTISE_1000XPSE_ASYM);
		adv |= ethtool_adv_to_mii_adv_x(phydev->advertising);
	} else {
		adv &= ~(ADVERTISE_ALL | ADVERTISE_100BASE4 | ADVERTISE_PAUSE_CAP |
			 ADVERTISE_PAUSE_ASYM);
		adv |= ethtool_adv_to_mii_adv_t(phydev->advertising);
	}

	if (adv != oldadv) {
		err = phy_write(phydev, MII_ADVERTISE, adv);
		if (err < 0)
			return err;

		changed = 1;
	}

	if (!rtl8211f_media_is_utp(mode))
		return changed;

	oldadv = adv = phy_read(phydev, MII_CTRL1000);
	if (adv < 0)
		return adv;

	adv &= ~(ADVERTISE_1000FULL | ADVERTISE_1000HALF);
	adv |= ethtool_adv_to_mii_ctrl1000_t(phydev->advertising);

	if (adv != oldadv) {
		err = phy_write(phydev, MII_CTRL1000, adv);

		if (err < 0)
			return err;
		changed = 1;
	}

	return changed;
}

static int rtl8211f_config_aneg(struct phy_device *phydev)
{
	int result, mode;

	if (AUTONEG_ENABLE != phydev->autoneg) {
		int ctl = 0;

		phydev->pause = phydev->asym_pause = 0;

		if (SPEED_1000 == phydev->speed)
			ctl |= BMCR_SPEED1000;
		else if (SPEED_100 == phydev->speed)
			ctl |= BMCR_SPEED100;

		if (DUPLEX_FULL == phydev->duplex)
			ctl |= BMCR_FULLDPLX;

		return phy_write(phydev, MII_BMCR, ctl);
	}

	mode = rtl8211f_read_cfg_mode(phydev);
	if (mode < 0)
		return mode;

	result = rtl8211f_config_advert(phydev, mode);
	if (result < 0) /* error */
		return result;

	if (result == 0) {
		/* Advertisement hasn't changed, but maybe aneg was never on to
		 * begin with?  Or maybe phy was isolated? */
		int ctl = phy_read(phydev, MII_BMCR);

		if (ctl < 0)
			return ctl;

		if (!(ctl & BMCR_ANENABLE) || (ctl & BMCR_ISOLATE))
			result = 1; /* do restart aneg */
	}

	/* Only restart aneg if we are advertising something different
	 * than we were before.	 */
	if (result > 0)
		result = genphy_restart_aneg(phydev);

	return result;
}

static int rtl8211f_config_init(struct phy_device *phydev)
{
	int ret;
	u16 val;

	ret = rtl8211x_page_read(phydev, 0xd08, 0x11);
	if (ret < 0)
		return ret;

	val = ret & 0xffff;

	/* enable TX-delay for rgmii-id and rgmii-txid, otherwise disable it */
	if (phydev->interface == PHY_INTERFACE_MODE_RGMII_ID ||
	    phydev->interface == PHY_INTERFACE_MODE_RGMII_TXID)
		val |= RTL8211F_TX_DELAY;
	else
		val &= ~RTL8211F_TX_DELAY;

	ret = rtl8211x_page_write(phydev, 0xd08, 0x11, val);
	if (ret)
		return ret;

	ret = rtl8211x_page_read(phydev, 0xd08, 0x15);
	if (ret < 0)
		return ret;

	val = ret & 0xffff;

	/* enable RX-delay for rgmii-id and rgmii-rxid, otherwise disable it */
	if (phydev->interface == PHY_INTERFACE_MODE_RGMII_ID ||
	    phydev->interface == PHY_INTERFACE_MODE_RGMII_RXID)
		val |= RTL8211F_RX_DELAY;
	else
		val &= ~RTL8211F_RX_DELAY;

	ret = rtl8211x_page_write(phydev, 0xd08, 0x15, val);
	if (ret)
		return ret;

	return 0;
}

static struct phy_driver realtek_drvs[] = {
	{
		.phy_id         = 0x00008201,
		.name           = "RTL8201CP Ethernet",
		.phy_id_mask    = 0x0000ffff,
		.features       = PHY_BASIC_FEATURES,
		.flags          = PHY_HAS_INTERRUPT,
		.config_aneg    = &genphy_config_aneg,
		.read_status    = &genphy_read_status,
		.driver         = { .owner = THIS_MODULE,},
	}, {
		.phy_id		= 0x001cc816,
		.name		= "RTL8201F 10/100Mbps Ethernet",
		.phy_id_mask	= 0x001fffff,
		.features	= PHY_BASIC_FEATURES,
		.flags		= PHY_HAS_INTERRUPT,
		.config_aneg	= &genphy_config_aneg,
		.read_status	= &genphy_read_status,
		.ack_interrupt	= &rtl8201_ack_interrupt,
		.config_intr	= &rtl8201_config_intr,
		.suspend	= genphy_suspend,
		.resume		= genphy_resume,
	}, {
		.phy_id		= 0x001cc912,
		.name		= "RTL8211B Gigabit Ethernet",
		.phy_id_mask	= 0x001fffff,
		.features	= PHY_GBIT_FEATURES,
		.flags		= PHY_HAS_INTERRUPT,
		.config_aneg	= &genphy_config_aneg,
		.read_status	= &genphy_read_status,
		.ack_interrupt	= &rtl821x_ack_interrupt,
		.config_intr	= &rtl8211b_config_intr,
		.driver		= { .owner = THIS_MODULE,},
	}, {
		.phy_id		= 0x001cc914,
		.name		= "RTL8211DN Gigabit Ethernet",
		.phy_id_mask	= 0x001fffff,
		.features	= PHY_GBIT_FEATURES,
		.flags		= PHY_HAS_INTERRUPT,
		.config_aneg	= genphy_config_aneg,
		.read_status	= genphy_read_status,
		.ack_interrupt	= rtl821x_ack_interrupt,
		.config_intr	= rtl8211e_config_intr,
		.suspend	= genphy_suspend,
		.resume		= genphy_resume,
		.driver		= { .owner = THIS_MODULE,},
	}, {
		.phy_id		= 0x001cc915,
		.name		= "RTL8211E Gigabit Ethernet",
		.phy_id_mask	= 0x001fffff,
		.features	= PHY_GBIT_FEATURES,
		.flags		= PHY_HAS_INTERRUPT,
		.config_aneg	= &genphy_config_aneg,
		.read_status	= &genphy_read_status,
		.ack_interrupt	= &rtl821x_ack_interrupt,
		.config_intr	= &rtl8211e_config_intr,
		.suspend	= genphy_suspend,
		.resume		= genphy_resume,
		.driver		= { .owner = THIS_MODULE,},
	}, {
		.phy_id		= 0x001cc916,
		.name		= "RTL8211F(S) Gigabit Ethernet",
		.phy_id_mask	= 0x001fffff,
		.features	= PHY_GBIT_FEATURES,
		.flags		= PHY_HAS_INTERRUPT,
		.config_aneg	= &rtl8211f_config_aneg,
		.config_init	= &rtl8211f_config_init,
		.read_status	= &rtl8211f_read_status,
		.ack_interrupt	= &rtl8211f_ack_interrupt,
		.config_intr	= &rtl8211f_config_intr,
		.suspend	= genphy_suspend,
		.resume		= genphy_resume,
		.driver		= { .owner = THIS_MODULE },
	},
};

static int __init realtek_init(void)
{
	return phy_drivers_register(realtek_drvs, ARRAY_SIZE(realtek_drvs));
}

static void __exit realtek_exit(void)
{
	phy_drivers_unregister(realtek_drvs, ARRAY_SIZE(realtek_drvs));
}

module_init(realtek_init);
module_exit(realtek_exit);

static struct mdio_device_id __maybe_unused realtek_tbl[] = {
	{ 0x001cc816, 0x001fffff },
	{ 0x001cc912, 0x001fffff },
	{ 0x001cc914, 0x001fffff },
	{ 0x001cc915, 0x001fffff },
	{ 0x001cc916, 0x001fffff },
	{ }
};

MODULE_DEVICE_TABLE(mdio, realtek_tbl);
