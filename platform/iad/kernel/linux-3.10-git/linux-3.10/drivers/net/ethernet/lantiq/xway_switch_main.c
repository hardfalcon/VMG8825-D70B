/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   Copyright (C) 2012-2014 Daniel Schwierzeck <daniel.schwierzeck@sphairon.com>
 */

#define DEBUG
/*#define DEBUG_REGISTER*/
/*#define DEBUG_MII*/
/*#define DEBUG_MII_REGISTER*/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of_device.h>
#include <linux/of_mdio.h>
#include <linux/of_net.h>
#include <linux/of_address.h>
#include <linux/firmware.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/phy.h>

#include <lantiq.h>
#include <switch-api/lantiq_ethsw_api.h>

#include "xway_switch.h"
#include "xway_switch_regs.h"

#define XWAY_SWITCH_MAX_PORTS	6

struct xway_switch;
struct xway_switch_port;

struct xway_switch_hwcfg {
	unsigned int cpu_port;
	unsigned int port_cnt;
	void (*init)(struct xway_switch *);
	void (*port_init)(struct xway_switch_port *);
	int (*port_open)(struct xway_switch_port *);
	int (*port_stop)(struct xway_switch_port *);
	void (*gmac_update)(struct xway_switch_port *);
};

struct xway_switch_port {
	struct net_device *netdev;
	struct phy_device *phydev;
	struct xway_switch *sw;
	struct device_node *phy_np;
	u8 port_id;
	u8 flow_ctrl;
	u8 flow_ctrl_old;
	phy_interface_t phy_if;
	int mii_link;
	int mii_duplex;
	int mii_speed;
	int mii_pause;
	int mii_asym_pause;
	int phy_link_old;
	int phy_duplex_old;
	int phy_speed_old;
	int rgmii_rx_delay;
	int rgmii_tx_delay;
	bool mii_isolate;
	bool is_wanoe;
};

struct xway_switch {
	struct device *dev;
	void __iomem *regbase;
	const struct xway_switch_hwcfg *hwcfg;
	struct clk *clk;
	struct mii_bus *mii_bus;
	struct net_device *wanoe_netdev;
	int mdio_irqs[PHY_MAX_ADDR];
	struct xway_switch_port *port[XWAY_SWITCH_MAX_PORTS];
	LTQ_ETHSW_API_HANDLE api_handle;
	struct notifier_block nb;
};

static inline u32 __xway_switch_r32(void __iomem *addr, const char *desc)
{
	u32 val = __raw_readl(addr);

	pr_info("r32:  %30s: %08x\n", desc, val);

	return val;
}

static inline void __xway_switch_w32(u32 val, void __iomem *addr,
					const char *desc)
{
	u32 data = __raw_readl(addr);

	pr_info("w32:  %30s: %08x -> %08x\n", desc, data, val);

	__raw_writel(val, addr);
}

static inline void __xway_switch_w32_mask(u32 clr, u32 set, void __iomem *addr,
					const char *desc)
{
	u32 data = __raw_readl(addr);
	u32 val = (data & (~clr)) | set;

	pr_info("w32m: %30s: %08x -> %08x\n", desc, data, val);

	__raw_writel(val, addr);
}

#ifdef DEBUG_REGISTER
#define xway_switch_reg_dump(a)		__xway_switch_r32(a, #a)
#define xway_switch_r32(a)		__xway_switch_r32(a, #a)
#define xway_switch_w32(v, a)		__xway_switch_w32(v, a, #a)
#define xway_switch_w32_mask(c, s, a)	__xway_switch_w32_mask(c, s, a, #a)
#else
#define xway_switch_reg_dump(a)
#define xway_switch_r32(a)		__raw_readl(a)
#define xway_switch_w32(v, a)		__raw_writel(v, a)
#define xway_switch_w32_mask(c, s, a)	__raw_writel((__raw_readl(a) & ~(c)) | (s), a)
#endif

#ifdef DEBUG_MII_REGISTER
#define xway_mii_r32(a)			__xway_switch_r32(a, #a)
#define xway_mii_w32(v, a)		__xway_switch_w32(v, a, #a)
#else
#define xway_mii_r32(a)			__raw_readl(a)
#define xway_mii_w32(v, a)		__raw_writel(v, a)
#endif

static int xway_switch_api_ioctl(struct xway_switch *xway_switch,
				unsigned int cmd, void *data)
{
	return ltq_ethsw_api_kioctl(xway_switch->api_handle, cmd,
		(unsigned int)data);
}

static inline int xway_switch_mdio_is_busy(struct xway_switch *xway_switch)
{
	struct vr9_switch_regs *regs = xway_switch->regbase;

	u32 mdio_ctrl = xway_mii_r32(&regs->mdio.mdio_ctrl);

	return mdio_ctrl & MDIO_CTRL_MBUSY;
}

static inline void xway_switch_mdio_poll(struct xway_switch *xway_switch)
{
	while (xway_switch_mdio_is_busy(xway_switch))
		cpu_relax();
}

static int xway_switch_mdio_read(struct xway_switch *xway_switch, int phyad,
				int regad)
{
	struct vr9_switch_regs *regs = xway_switch->regbase;
	u32 mdio_ctrl;
	int retval;

	mdio_ctrl = MDIO_CTRL_OP_READ |
		((phyad << MDIO_CTRL_PHYAD_SHIFT) & MDIO_CTRL_PHYAD_MASK) |
		(regad & MDIO_CTRL_REGAD_MASK);

	xway_switch_mdio_poll(xway_switch);
	xway_mii_w32(mdio_ctrl, &regs->mdio.mdio_ctrl);
	xway_switch_mdio_poll(xway_switch);
	retval = xway_mii_r32(&regs->mdio.mdio_read);

#ifdef DEBUG_MII
	dev_dbg(xway_switch->dev, "%s: phyad 0x%x, regad 0x%x, retval 0x%x\n",
		__func__, phyad, regad, retval);
#endif

	return retval;
}

static int xway_switch_mdio_phylib_read_wait(struct mii_bus *bus, int phy_id,
						int regnum)
{
	struct xway_switch *xway_switch = bus->priv;
	unsigned long deadline = jiffies + msecs_to_jiffies(1000);
	int val;

	do {
		dev_dbg(xway_switch->dev, "wait until PHY %d is up\n", phy_id);
		msleep(10);

		val = xway_switch_mdio_read(xway_switch, phy_id, regnum);
		if (val)
			return val;
	} while (!time_after_eq(jiffies, deadline));

	return -ENODEV;
}

static int xway_switch_mdio_phylib_read(struct mii_bus *bus, int phy_id,
					int regnum)
{
	struct xway_switch *xway_switch = bus->priv;
	int val;


	val = xway_switch_mdio_read(xway_switch, phy_id, regnum);
	if (regnum == MII_PHYSID1 && !val)
		return xway_switch_mdio_phylib_read_wait(bus, phy_id, regnum);

	return val;
}

static int xway_switch_mdio_mii_read(struct net_device *dev, int mii_id,
					int regnum)
{
	struct xway_switch_port *port = netdev_priv(dev);
	struct xway_switch *sw = port->sw;

	return xway_switch_mdio_read(sw, mii_id, regnum);
}

static int xway_switch_mdio_write(struct xway_switch *xway_switch, int phyad,
					int regad, u16 val)
{
	struct vr9_switch_regs *regs = xway_switch->regbase;
	u32 mdio_ctrl;

#ifdef DEBUG_MII
	dev_dbg(xway_switch->dev, "%s: phyad 0x%x, regad 0x%x, val 0x%x\n",
		__func__, phyad, regad, val);
#endif

	mdio_ctrl = MDIO_CTRL_OP_WRITE |
		((phyad << MDIO_CTRL_PHYAD_SHIFT) & MDIO_CTRL_PHYAD_MASK) |
		(regad & MDIO_CTRL_REGAD_MASK);

	xway_switch_mdio_poll(xway_switch);
	xway_mii_w32(val, &regs->mdio.mdio_write);
	xway_switch_mdio_poll(xway_switch);
	xway_mii_w32(mdio_ctrl, &regs->mdio.mdio_ctrl);
	xway_switch_mdio_poll(xway_switch);

	return 0;
}

static int xway_switch_mdio_phylib_write(struct mii_bus *bus, int phy_id,
					int regnum, u16 val)
{
	struct xway_switch *xway_switch = bus->priv;

	return xway_switch_mdio_write(xway_switch, phy_id, regnum, val);
}

static void xway_switch_mdio_mii_write(struct net_device *dev, int mii_id,
				    int regnum, int val)
{
	struct xway_switch_port *port = netdev_priv(dev);
	struct xway_switch *sw = port->sw;

	xway_switch_mdio_write(sw, mii_id, regnum, val);
}

static void xway_switch_xrx200_gmac_update(struct xway_switch_port *port)
{
	struct xway_switch *sw = port->sw;
	struct vr9_switch_regs *regs = sw->regbase;
	struct phy_device *phydev = port->phydev;
	u32 phy_addr, mii_cfg;
	u16 phy_adv, rmt_adv;
	u8 flow_ctrl;
	int phy_link, phy_duplex, phy_speed, phy_pause, phy_asym_pause;

	if (phydev) {
		phy_addr = phydev->addr;
		phy_link = phydev->link;
		phy_duplex = phydev->duplex;
		phy_speed = phydev->speed;
		phy_pause = phydev->pause;
		phy_asym_pause = phydev->asym_pause;
		phy_adv = phydev->advertising;
	} else {
		phy_addr = 0;
		phy_link = port->mii_link;
		phy_duplex = port->mii_duplex;
		phy_speed = port->mii_speed;
		phy_pause = port->mii_pause;
		phy_asym_pause = port->mii_asym_pause;
		phy_adv = 0;
	}

	switch (port->port_id) {
	case 0:
	case 1:
	case 5:
		mii_cfg = xway_switch_r32(vr9_to_mii_miicfg(&regs->mii, port->port_id));
		break;
	default:
		mii_cfg = 0;
		break;
	}

#if 0
	netdev_dbg(port->netdev, "%s: port %u, addr 0x%x, link %d, speed %d, duplex %d, pause %d, asym_pause %d\n",
		__func__, port->port_id, phy_addr, phy_link, phy_speed, phy_duplex,
		phy_pause, phy_asym_pause);
#endif

	if (phy_link)
		phy_addr |= PHY_ADDR_LNKST_UP;
	else
		phy_addr |= PHY_ADDR_LNKST_DOWN;

	mii_cfg &= ~MII_CFG_MIIRATE_MASK;

	switch (phy_speed) {
	case SPEED_1000:
		phy_addr |= PHY_ADDR_SPEED_G1;
		mii_cfg |= MII_CFG_MIIRATE_M125;
		break;
	case SPEED_100:
		phy_addr |= PHY_ADDR_SPEED_M100;
		if (port->phy_if == PHY_INTERFACE_MODE_RMII)
			mii_cfg |= MII_CFG_MIIRATE_M50;
		else
			mii_cfg |= MII_CFG_MIIRATE_M25;
		break;
	default:
		phy_addr |= PHY_ADDR_SPEED_M10;
		mii_cfg |= MII_CFG_MIIRATE_M2P5;
		break;
	}

	if (phy_duplex == DUPLEX_FULL) {
		phy_addr |= PHY_ADDR_FDUP_EN;

		rmt_adv = 0;
		if (phy_pause)
			rmt_adv |= ADVERTISE_PAUSE_CAP;

		if (phy_asym_pause)
			rmt_adv |= ADVERTISE_PAUSE_ASYM;

		flow_ctrl = mii_resolve_flowctrl_fdx(phy_adv, rmt_adv);

		if (flow_ctrl & FLOW_CTRL_RX)
			phy_addr |= PHY_ADDR_FCONRX_EN;
		else
			phy_addr |= PHY_ADDR_FCONRX_DIS;

		if (flow_ctrl & FLOW_CTRL_TX)
			phy_addr |= PHY_ADDR_FCONTX_EN;
		else
			phy_addr |= PHY_ADDR_FCONTX_DIS;
	} else {
		phy_addr |= PHY_ADDR_FDUP_DIS;
		phy_addr |= PHY_ADDR_FCONTX_DIS | PHY_ADDR_FCONRX_DIS;
	}

	xway_switch_w32(phy_addr, to_mdio_phyaddr(&regs->mdio, port->port_id));

	switch (port->port_id) {
	case 0:
	case 1:
	case 5:
		xway_switch_w32(mii_cfg, vr9_to_mii_miicfg(&regs->mii, port->port_id));
		break;
	default:
		break;
	}
}

static void xway_switch_xrx330_gmac_update(struct xway_switch_port *port)
{
	struct xway_switch *sw = port->sw;
	struct ar10_switch_regs *regs = sw->regbase;
	struct phy_device *phydev = port->phydev;
	u32 phy_addr, mii_cfg;
	u16 phy_adv, rmt_adv;
	u8 flow_ctrl;
	int phy_link, phy_duplex, phy_speed, phy_pause, phy_asym_pause;

	if (phydev) {
		phy_addr = phydev->addr;
		phy_link = phydev->link;
		phy_duplex = phydev->duplex;
		phy_speed = phydev->speed;
		phy_pause = phydev->pause;
		phy_asym_pause = phydev->asym_pause;
		phy_adv = phydev->advertising;
	} else {
		phy_addr = 0;
		phy_link = port->mii_link;
		phy_duplex = port->mii_duplex;
		phy_speed = port->mii_speed;
		phy_pause = port->mii_pause;
		phy_asym_pause = port->mii_asym_pause;
		phy_adv = 0;
	}

	switch (port->port_id) {
	case 0:
	case 5:
		mii_cfg = xway_switch_r32(ar10_to_mii_miicfg(&regs->mii, port->port_id));
		break;
	default:
		mii_cfg = 0;
		break;
	}

#if 0
	netdev_dbg(port->netdev, "%s: port %u, addr 0x%x, link %d, speed %d, duplex %d, pause %d, asym_pause %d\n",
		__func__, port->port_id, phy_addr, phy_link, phy_speed, phy_duplex,
		phy_pause, phy_asym_pause);
#endif

	if (phy_link)
		phy_addr |= PHY_ADDR_LNKST_UP;
	else
		phy_addr |= PHY_ADDR_LNKST_DOWN;

	mii_cfg &= ~MII_CFG_MIIRATE_MASK;

	switch (phy_speed) {
	case SPEED_1000:
		phy_addr |= PHY_ADDR_SPEED_G1;
		mii_cfg |= MII_CFG_MIIRATE_M125;
		break;
	case SPEED_100:
		phy_addr |= PHY_ADDR_SPEED_M100;
		if (port->phy_if == PHY_INTERFACE_MODE_RMII)
			mii_cfg |= MII_CFG_MIIRATE_M50;
		else
			mii_cfg |= MII_CFG_MIIRATE_M25;
		break;
	default:
		phy_addr |= PHY_ADDR_SPEED_M10;
		mii_cfg |= MII_CFG_MIIRATE_M2P5;
		break;
	}

	if (phy_duplex == DUPLEX_FULL) {
		phy_addr |= PHY_ADDR_FDUP_EN;

		rmt_adv = 0;
		if (phy_pause)
			rmt_adv |= ADVERTISE_PAUSE_CAP;

		if (phy_asym_pause)
			rmt_adv |= ADVERTISE_PAUSE_ASYM;

		flow_ctrl = mii_resolve_flowctrl_fdx(phy_adv, rmt_adv);

		if (flow_ctrl & FLOW_CTRL_RX)
			phy_addr |= PHY_ADDR_FCONRX_EN;
		else
			phy_addr |= PHY_ADDR_FCONRX_DIS;

		if (flow_ctrl & FLOW_CTRL_TX)
			phy_addr |= PHY_ADDR_FCONTX_EN;
		else
			phy_addr |= PHY_ADDR_FCONTX_DIS;
	} else {
		phy_addr |= PHY_ADDR_FDUP_DIS;
		phy_addr |= PHY_ADDR_FCONTX_DIS | PHY_ADDR_FCONRX_DIS;
	}

	xway_switch_w32(phy_addr, to_mdio_phyaddr(&regs->mdio, port->port_id));

	switch (port->port_id) {
	case 0:
	case 5:
		xway_switch_w32(mii_cfg, ar10_to_mii_miicfg(&regs->mii, port->port_id));
		break;
	default:
		break;
	}
}

static void xway_switch_mdio_adjust_link(struct net_device *dev)
{
	struct xway_switch_port *port = netdev_priv(dev);
	struct phy_device *phydev = port->phydev;

#if 0
	netdev_dbg(dev, "%s: port %u, addr 0x%x, state %d, link %d\n",
		__func__, port->port_id, phydev->addr, phydev->state, phydev->link);
#endif

	if (phydev->link != port->phy_link_old ||
		phydev->speed != port->phy_speed_old ||
		phydev->duplex != port->phy_duplex_old) {

		phy_print_status(phydev);
		port->sw->hwcfg->gmac_update(port);

		port->phy_link_old = phydev->link;
		port->phy_speed_old = phydev->speed;
		port->phy_duplex_old = phydev->duplex;
	}

	if (port->is_wanoe && port->sw->wanoe_netdev) {
		if (phydev->link)
			netif_carrier_on(port->sw->wanoe_netdev);
		else
			netif_carrier_off(port->sw->wanoe_netdev);
	}
}

static int xway_switch_port_ioctl(struct net_device *dev, struct ifreq *rq,
					int cmd)
{
	struct xway_switch_port *port = netdev_priv(dev);
	struct mii_if_info mii;

	if (port->phydev)
		return phy_mii_ioctl(port->phydev, rq, cmd);

	mii.dev = dev;
	mii.mdio_read = xway_switch_mdio_mii_read;
	mii.mdio_write = xway_switch_mdio_mii_write;
	mii.phy_id = 0;
	mii.phy_id_mask = 0x3f;
	mii.reg_num_mask = 0x1f;

	return generic_mii_ioctl(&mii, if_mii(rq), cmd, NULL);
}

static struct rtnl_link_stats64 *
xway_switch_port_get_stats64(struct net_device *dev,
				struct rtnl_link_stats64 *stats)
{
	struct xway_switch_port *port = netdev_priv(dev);
	IFX_ETHSW_RMON_cnt_t rmon_cnt;
	int err;

	memset(&rmon_cnt, 0, sizeof(IFX_ETHSW_RMON_cnt_t));
	rmon_cnt.nPortId = port->port_id;

	err = xway_switch_api_ioctl(port->sw, IFX_ETHSW_RMON_GET, &rmon_cnt);
	if (err)
		return stats;

	stats->rx_packets = rmon_cnt.nRxGoodPkts;
	stats->tx_packets = rmon_cnt.nTxGoodPkts;
	stats->rx_bytes = rmon_cnt.nRxGoodBytes;
	stats->tx_bytes = rmon_cnt.nTxGoodBytes;
	stats->rx_dropped = rmon_cnt.nRxDroppedPkts;
	stats->tx_dropped = rmon_cnt.nTxDroppedPkts;
	stats->multicast = rmon_cnt.nRxMulticastPkts;
	stats->collisions = rmon_cnt.nTxCollCount;

	return stats;
}

static int xway_switch_xrx200_port_open(struct xway_switch_port *port)
{
	struct vr9_switch_regs *regs = port->sw->regbase;

	switch (port->port_id) {
	case 0:
	case 1:
	case 5:
		if (port->mii_isolate)
			xway_switch_w32_mask(MII_CFG_ISOL, 0,
				vr9_to_mii_miicfg(&regs->mii, port->port_id));
		break;
	default:
		break;
	}

	return 0;
}

static int xway_switch_xrx200_port_stop(struct xway_switch_port *port)
{
	struct vr9_switch_regs *regs = port->sw->regbase;

	switch (port->port_id) {
	case 0:
	case 1:
	case 5:
		if (port->mii_isolate)
			xway_switch_w32_mask(0, MII_CFG_ISOL,
				vr9_to_mii_miicfg(&regs->mii, port->port_id));
		break;
	default:
		break;
	}

	return 0;
}

static int xway_switch_xrx330_port_open(struct xway_switch_port *port)
{
	struct ar10_switch_regs *regs = port->sw->regbase;

	switch (port->port_id) {
	case 0:
	case 5:
		if (port->mii_isolate)
			xway_switch_w32_mask(MII_CFG_ISOL, 0,
				ar10_to_mii_miicfg(&regs->mii, port->port_id));
		break;
	default:
		break;
	}

	return 0;
}

static int xway_switch_xrx330_port_stop(struct xway_switch_port *port)
{
	struct ar10_switch_regs *regs = port->sw->regbase;

	switch (port->port_id) {
	case 0:
	case 5:
		if (port->mii_isolate)
			xway_switch_w32_mask(0, MII_CFG_ISOL,
				ar10_to_mii_miicfg(&regs->mii, port->port_id));
		break;
	default:
		break;
	}

	return 0;
}

static int xway_switch_port_ndo_open(struct net_device *dev)
{
	struct xway_switch_port *port = netdev_priv(dev);

	if (port->mii_isolate)
		netdev_info(dev, "disable xMII isolation\n");

	if (port->phydev)
		phy_start(port->phydev);

	if (port->sw->hwcfg->port_open)
		return port->sw->hwcfg->port_open(port);

	return 0;
}

static int xway_switch_port_ndo_stop(struct net_device *dev)
{
	struct xway_switch_port *port = netdev_priv(dev);

	if (port->mii_isolate)
		netdev_info(dev, "enable xMII isolation\n");

	if (port->phydev)
		phy_stop(port->phydev);

	if (port->sw->hwcfg->port_stop)
		return port->sw->hwcfg->port_stop(port);

	return 0;
}

static struct net_device_ops xway_switch_port_ops = {
	.ndo_set_mac_address	= eth_mac_addr,
	.ndo_validate_addr	= eth_validate_addr,
	.ndo_do_ioctl		= xway_switch_port_ioctl,
	.ndo_change_mtu		= eth_change_mtu,
	.ndo_get_stats64	= xway_switch_port_get_stats64,
	.ndo_open		= xway_switch_port_ndo_open,
	.ndo_stop		= xway_switch_port_ndo_stop,
};

static void xway_switch_get_drvinfo(struct net_device *dev,
					struct ethtool_drvinfo *info)
{
	strcpy(info->driver, "xway-switch");
	strcpy(info->bus_info, "internal");
	strcpy(info->version, "0.1");
}

static int xway_switch_get_settings(struct net_device *dev,
					struct ethtool_cmd *cmd)
{
	struct xway_switch_port *port = netdev_priv(dev);
	struct phy_device *phydev = port->phydev;

	if (phydev) {
		ethtool_cmd_speed_set(cmd, phydev->speed);
		cmd->autoneg = phydev->autoneg;
		cmd->supported = phydev->supported;
		cmd->advertising = phydev->advertising;
		cmd->duplex = phydev->duplex;
		cmd->phy_address = phydev->addr;
		cmd->port = PORT_TP;

	} else {
		ethtool_cmd_speed_set(cmd, port->mii_speed);
		cmd->autoneg = AUTONEG_DISABLE;
		cmd->supported = SUPPORTED_10baseT_Half | SUPPORTED_10baseT_Full |
			SUPPORTED_100baseT_Half | SUPPORTED_100baseT_Full |
			SUPPORTED_1000baseT_Half | SUPPORTED_1000baseT_Full;
		cmd->advertising = 0;
		cmd->duplex = port->mii_duplex;
		cmd->port = PORT_MII;
	}
	cmd->transceiver = XCVR_EXTERNAL;

	return 0;
}

static int xway_switch_set_settings(struct net_device *dev,
					struct ethtool_cmd *cmd)
{
	struct xway_switch_port *port = netdev_priv(dev);

	if (port->phydev)
		return phy_ethtool_sset(port->phydev, cmd);

	if (cmd->autoneg != AUTONEG_DISABLE)
		return -EINVAL;

	if (cmd->port != PORT_MII && cmd->port != PORT_TP)
		return -EINVAL;

	if (cmd->speed != SPEED_1000 && cmd->speed != SPEED_100 &&
		cmd->speed != SPEED_10)
		return -EINVAL;

	port->mii_speed = ethtool_cmd_speed(cmd);
	port->mii_duplex = cmd->duplex;

	port->sw->hwcfg->gmac_update(port);

	return 0;
}

static int xway_switch_nway_reset(struct net_device *dev)
{
	struct xway_switch_port *port = netdev_priv(dev);

	return genphy_restart_aneg(port->phydev);
}

static struct ethtool_ops xway_switch_ethtool_ops = {
	.get_drvinfo	= xway_switch_get_drvinfo,
	.get_settings	= xway_switch_get_settings,
	.set_settings	= xway_switch_set_settings,
	.nway_reset	= xway_switch_nway_reset,
	.get_link	= ethtool_op_get_link,
};

static void xway_switch_xrx200_init(struct xway_switch *xway_switch)
{
	struct vr9_switch_regs *regs = xway_switch->regbase;
	const struct xway_switch_hwcfg *hwcfg = xway_switch->hwcfg;
	unsigned int i;

	/* Disable auto-polling */
	xway_switch_w32(0, &regs->mdio.mdc_cfg_0);

	/* Enable and set MDC clock */
	xway_switch_w32(MDIO_MDC_CFG1_RES | MDIO_MDC_CFG1_MCEN | 5,
		&regs->mdio.mdc_cfg_1);

	/* Reset and disable xMII ports */
	xway_switch_w32(MII_CFG_RES, vr9_to_mii_miicfg(&regs->mii, 0));
	xway_switch_w32(MII_CFG_RES, vr9_to_mii_miicfg(&regs->mii, 1));
	xway_switch_w32(MII_CFG_RES, vr9_to_mii_miicfg(&regs->mii, 5));

	/* Reset PHY status on all external ports */
	for (i = 0; i < hwcfg->port_cnt; i++)
		xway_switch_w32(PHY_ADDR_LNKST_DOWN | PHY_ADDR_SPEED_M10 |
			PHY_ADDR_FDUP_DIS | PHY_ADDR_FCONTX_DIS |
			PHY_ADDR_FCONRX_DIS, to_mdio_phyaddr(&regs->mdio, i));
}

static void xway_switch_xrx330_init(struct xway_switch *xway_switch)
{
	struct ar10_switch_regs *regs = xway_switch->regbase;
	const struct xway_switch_hwcfg *hwcfg = xway_switch->hwcfg;
	unsigned int i;

	/* Disable auto-polling */
	xway_switch_w32(0, &regs->mdio.mdc_cfg_0);

	/* Enable and set MDC clock */
	xway_switch_w32(MDIO_MDC_CFG1_RES | MDIO_MDC_CFG1_MCEN | 5,
		&regs->mdio.mdc_cfg_1);

	/* Reset and disable xMII ports */
	xway_switch_w32(MII_CFG_RES, ar10_to_mii_miicfg(&regs->mii, 0));
	xway_switch_w32(MII_CFG_RES, ar10_to_mii_miicfg(&regs->mii, 5));

	/* Reset PHY status on all external ports */
	for (i = 0; i < hwcfg->port_cnt; i++)
		xway_switch_w32(PHY_ADDR_LNKST_DOWN | PHY_ADDR_SPEED_M10 |
			PHY_ADDR_FDUP_DIS | PHY_ADDR_FCONTX_DIS |
			PHY_ADDR_FCONRX_DIS, to_mdio_phyaddr(&regs->mdio, i));
}

static void xway_switch_xrx200_port_init(struct xway_switch_port *port)
{
	struct vr9_switch_regs *regs = port->sw->regbase;
	u32 mii_cfg = MII_CFG_EN;

	switch (port->port_id) {
	case 0:
	case 1:
	case 5:
		break;
	default:
		return;
	}

	switch (port->phy_if) {
	case PHY_INTERFACE_MODE_MII:
		if (port->phydev)
			mii_cfg |= MII_CFG_MIIMODE_MIIM;
		else
			mii_cfg |= MII_CFG_MIIMODE_MIIP;
		break;
	case PHY_INTERFACE_MODE_RMII:
		if (port->phydev)
			mii_cfg |= MII_CFG_MIIMODE_RMIIM;
		else
			mii_cfg |= MII_CFG_MIIMODE_RMIIP;
		break;
	case PHY_INTERFACE_MODE_RGMII:
	case PHY_INTERFACE_MODE_RGMII_RXID:
	case PHY_INTERFACE_MODE_RGMII_TXID:
	case PHY_INTERFACE_MODE_RGMII_ID:
		if (port->phydev)
			mii_cfg |= MII_CFG_MIIMODE_RGMII;
		else
			mii_cfg |= MII_CFG_MIIMODE_RGMII;
		break;
	default:
		break;
	}

	if (port->mii_isolate)
		mii_cfg |= MII_CFG_ISOL;

	xway_switch_w32(mii_cfg, vr9_to_mii_miicfg(&regs->mii, port->port_id));
	xway_switch_w32(port->rgmii_rx_delay << PCDU_RXDLY_SHIFT |
		port->rgmii_tx_delay, vr9_to_mii_pcdu(&regs->mii, port->port_id));
}

static void xway_switch_xrx330_port_init(struct xway_switch_port *port)
{
	struct ar10_switch_regs *regs = port->sw->regbase;
	u32 mii_cfg = 0;

	switch (port->port_id) {
	case 0: /* xMII0 */
		switch (port->phy_if) {
		case PHY_INTERFACE_MODE_GMII:
			/* GMII MAC mode, connected to external PHY */
			mii_cfg = MII_CFG_EN | MII_CFG_MIIMODE_GMIIM;
			break;
		case PHY_INTERFACE_MODE_RMII:
			/* RMII MAC mode, connected to external PHY */
			mii_cfg = MII_CFG_EN | MII_CFG_MIIMODE_RMIIM;
			break;
		case PHY_INTERFACE_MODE_RGMII:
		case PHY_INTERFACE_MODE_RGMII_RXID:
		case PHY_INTERFACE_MODE_RGMII_TXID:
		case PHY_INTERFACE_MODE_RGMII_ID:
			/* RGMII MAC mode, connected to external PHY */
			mii_cfg = MII_CFG_EN | MII_CFG_MIIMODE_RGMII;
			break;
		default:
			break;
		}

		if (port->mii_isolate)
			mii_cfg |= MII_CFG_ISOL;

		xway_switch_w32(port->rgmii_rx_delay << PCDU_RXDLY_SHIFT |
			port->rgmii_tx_delay,
			ar10_to_mii_pcdu(&regs->mii, port->port_id));
		break;
	case 3: /* internal GPHY3 */
		switch (port->phy_if) {
		case PHY_INTERFACE_MODE_MII:
			/* internal GPHY0 (FE) */
			mii_cfg = 0;
			break;
		case PHY_INTERFACE_MODE_GMII:
			/* internal GPHY3 (GE) */
			mii_cfg = MII_CFG_MIIMODE_GMIIM_3;
			break;
		default:
			break;
		}
		break;
	case 5: /* internal GPHY1 or xMII1 */
		switch (port->phy_if) {
		case PHY_INTERFACE_MODE_MII:
			/* MII MAC mode, connected to internal GPHY */
			mii_cfg = MII_CFG_EN | MII_CFG_MIIMODE_MIIM;
			break;
		case PHY_INTERFACE_MODE_RMII:
			/* RMII MAC mode, connected to external PHY */
			mii_cfg = MII_CFG_EN | MII_CFG_MIIMODE_RMIIM;
			break;
		case PHY_INTERFACE_MODE_RGMII:
		case PHY_INTERFACE_MODE_RGMII_RXID:
		case PHY_INTERFACE_MODE_RGMII_TXID:
		case PHY_INTERFACE_MODE_RGMII_ID:
			/* RGMII MAC mode, connected to external PHY */
			mii_cfg = MII_CFG_EN | MII_CFG_MIIMODE_RGMII;
			break;
		default:
			break;
		}

		if (port->mii_isolate)
			mii_cfg |= MII_CFG_ISOL;

		xway_switch_w32(port->rgmii_rx_delay << PCDU_RXDLY_SHIFT |
			port->rgmii_tx_delay,
			ar10_to_mii_pcdu(&regs->mii, port->port_id));
		break;
	default:
		return;
	}

	xway_switch_w32(mii_cfg, ar10_to_mii_miicfg(&regs->mii, port->port_id));
}

static struct xway_switch_port *
xway_switch_port_create(struct xway_switch *xway_switch, struct device_node *np)
{
	struct net_device *netdev;
	struct xway_switch_port *port;
	struct platform_device *pdev;
	struct device_node *fixed_np;
	const char *port_name;
	u32 port_id, speed;
	int err, phy_if;
	const void *prop;

	err = of_property_read_u32(np, "reg", &port_id);
	if (err) {
		dev_err(xway_switch->dev,
			"no reg property for subnode %s found\n", np->name);
		return NULL;
	}

	err = of_property_read_string(np, "lantiq,port-name", &port_name);
	if (err) {
		dev_err(xway_switch->dev,
			"no lantiq,port-name property for subnode %s found\n",
			np->name);
		return NULL;
	}

	phy_if = of_get_phy_mode(np);
	if (phy_if < 0) {
		dev_err(xway_switch->dev,
			"no phy-mode property for subnode %s found\n",
			np->name);
		return NULL;
	}

	pdev = of_find_device_by_node(np);
	if (!pdev) {
		dev_err(xway_switch->dev,
			"no platform_device for subnode %s found\n", np->name);
		return NULL;
	}
	put_device(&pdev->dev);

	netdev = alloc_netdev(sizeof(*port), port_name, ether_setup);
	if (!netdev) {
		dev_err(&pdev->dev, "failed to alloc netdev\n");
		return NULL;
	}

	port = netdev_priv(netdev);
	port->netdev = netdev;
	port->sw = xway_switch;
	port->port_id = port_id;
	port->phy_if = phy_if;
	port->mii_link = port->phy_link_old = 0;
	port->mii_speed = port->phy_speed_old = SPEED_10;
	port->mii_duplex = port->phy_duplex_old = DUPLEX_HALF;
	port->flow_ctrl = port->flow_ctrl_old = FLOW_CTRL_RX | FLOW_CTRL_TX;

	SET_NETDEV_DEV(netdev, &pdev->dev);
	SET_ETHTOOL_OPS(netdev, &xway_switch_ethtool_ops);
	netdev->netdev_ops = &xway_switch_port_ops;

	random_ether_addr(netdev->dev_addr);
	netif_carrier_off(netdev);

	err = register_netdev(netdev);
	if (err) {
		dev_err(&pdev->dev, "failed to register netdev\n");
		goto err_free;
	}

	port->phy_np = of_parse_phandle(np, "phy-handle", 0);
	if (!port->phy_np) {
		fixed_np = of_get_child_by_name(np, "fixed-link");
		if (fixed_np) {
			of_property_read_u32(fixed_np, "speed", &speed);
			port->mii_speed = speed;
			port->mii_link = 1;
			port->mii_duplex = of_property_read_bool(fixed_np,
				"full-duplex");
			port->mii_pause = of_property_read_bool(fixed_np, "pause");
			port->mii_asym_pause = of_property_read_bool(fixed_np,
				"asym-pause");
			of_node_put(fixed_np);
			netif_carrier_on(netdev);
		}
	}

	prop = of_get_property(np, "lantiq,rgmii-rx-delay", NULL);
	if (prop) {
		port->rgmii_rx_delay = be32_to_cpup(prop);
		netdev_info(netdev, "rgmii-rx-delay %u\n",
			port->rgmii_rx_delay);
	}

	prop = of_get_property(np, "lantiq,rgmii-tx-delay", NULL);
	if (prop) {
		port->rgmii_tx_delay = be32_to_cpup(prop);
		netdev_info(netdev, "rgmii-tx-delay %u\n",
			port->rgmii_tx_delay);
	}

	port->mii_isolate = of_property_read_bool(np, "lantiq,mii-isolate");
	port->is_wanoe = of_property_read_bool(np, "lantiq,wanoe");

	if (port->is_wanoe)
		netdev_info(netdev, "is WANoE\n");

	return port;

err_free:
	free_netdev(netdev);

	return NULL;
}

static void xway_switch_port_release(struct xway_switch_port *port)
{
	netdev_dbg(port->netdev, "%s: port_id %u\n", __func__, port->port_id);

	unregister_netdev(port->netdev);
	free_netdev(port->netdev);
}

static int xway_switch_ports_init(struct xway_switch *xway_switch)
{
	struct device_node *np = xway_switch->dev->of_node;
	struct device_node *port_np;
	struct xway_switch_port *port;
	unsigned int i;
	int err;

	for_each_available_child_of_node(np, port_np) {
		if (!of_device_is_compatible(port_np, "lantiq,switch-port"))
			continue;

		port = xway_switch_port_create(xway_switch, port_np);
		if (!port) {
			err = -ENOMEM;
			goto err_release;
		}

		port->sw->hwcfg->port_init(port);
		port->sw->hwcfg->gmac_update(port);

		xway_switch->port[port->port_id] = port;
	}

	return 0;

err_release:
	for (i = 0; i < XWAY_SWITCH_MAX_PORTS; i++) {
		port = xway_switch->port[i];
		if (port)
			xway_switch_port_release(port);
	}

	return err;
}

static void xway_switch_ports_exit(struct xway_switch *xway_switch)
{
	unsigned int i;

	for (i = 0; i < XWAY_SWITCH_MAX_PORTS; i++) {
		if (xway_switch->port[i])
			xway_switch_port_release(xway_switch->port[i]);
	}
}

static int xway_switch_phy_start(struct xway_switch *xway_switch)
{
	struct xway_switch_port *port;
	struct phy_device *phydev;
	int i, err;

	for (i = 0; i < XWAY_SWITCH_MAX_PORTS; i++) {
		port = xway_switch->port[i];
		if (!port)
			continue;

		if (!port->phy_np)
			continue;

		phydev = of_phy_connect(port->netdev, port->phy_np,
			xway_switch_mdio_adjust_link, 0, port->phy_if);
		if (!phydev) {
			netdev_err(port->netdev, "failed to connect phy\n");
			return err;
		}

		phydev->supported &= PHY_GBIT_FEATURES;
		phydev->supported |= SUPPORTED_Pause | SUPPORTED_Asym_Pause;
		phydev->advertising = phydev->supported;

		port->phydev = phydev;
		phy_start(phydev);
	}

	return 0;
}

static void xway_switch_phy_stop(struct xway_switch *xway_switch)
{
	struct xway_switch_port *port;
	int i;

	for (i = 0; i < XWAY_SWITCH_MAX_PORTS; i++) {
		port = xway_switch->port[i];
		if (!port)
			continue;

		if (!port->phydev)
			continue;

		phy_stop(port->phydev);
		phy_disconnect(port->phydev);
	}
}

static int xway_switch_mdio_init(struct xway_switch *xway_switch)
{
	struct device_node *np;
	struct platform_device *pdev;
	struct mii_bus *bus;
	int err;

	np = of_find_node_by_name(xway_switch->dev->of_node, "mdio");
	if (!np) {
		dev_err(xway_switch->dev, "no mdio subnode found\n");
		return -EINVAL;
	}

	pdev = of_find_device_by_node(np);
	if (!pdev) {
		of_node_put(np);
		return -ENODEV;
	}
	of_node_put(np);

	bus = mdiobus_alloc();
	if (!bus) {
		err = -ENOMEM;
		goto err_put_device;
	}

	sprintf(bus->id, "0");
	bus->name = "xway-switch-mdio";
	bus->read = xway_switch_mdio_phylib_read;
	bus->write = xway_switch_mdio_phylib_write;
	bus->parent = &pdev->dev;
	bus->priv = xway_switch;
	bus->irq = xway_switch->mdio_irqs;

	err = of_mdiobus_register(bus, np);
	if (err)
		goto err_mdio_free;

	put_device(&pdev->dev);
	xway_switch->mii_bus = bus;

	return 0;

err_mdio_free:
	mdiobus_free(bus);
err_put_device:
	put_device(&pdev->dev);

	return err;
}

static void xway_switch_mdio_exit(struct xway_switch *xway_switch)
{
	if (!xway_switch->mii_bus)
		return;

	mdiobus_unregister(xway_switch->mii_bus);
	mdiobus_free(xway_switch->mii_bus);
	xway_switch->mii_bus = NULL;
}

static int xway_switch_api_init(struct xway_switch *xway_switch)
{
	LTQ_ETHSW_API_HANDLE handle;
	int err;

	err = ltq_ethsw_api_init(xway_switch->regbase);
	if (err)
		return err;

	handle = ltq_ethsw_api_kopen("/dev/switch_api/0");
	if (!handle) {
		err = -ENODEV;
		goto err_close;
	}

	xway_switch->api_handle = handle;
	return 0;

err_close:
	ltq_ethsw_api_exit();
	return err;
}

static void xway_switch_api_exit(struct xway_switch *xway_switch)
{
	ltq_ethsw_api_kclose(xway_switch->api_handle);
	ltq_ethsw_api_exit();
	xway_switch->api_handle = 0;
}

static int xway_switch_netdev_notifier(struct notifier_block *nb,
					unsigned long action, void *data)
{
	struct xway_switch *xway_switch =
		container_of(nb, struct xway_switch, nb);
	struct net_device *dev = data;
	struct xway_switch_port *port;
	unsigned int i;

	if (!(dev->priv_flags & IFF_WANOE_DATAPATH))
		return NOTIFY_DONE;

	switch (action) {
	case NETDEV_REGISTER:
		if (!xway_switch->wanoe_netdev) {
			dev_dbg(xway_switch->dev,
				"registering %s as WANoE device\n", dev->name);
			dev_hold(dev);
			xway_switch->wanoe_netdev = dev;
		}
		return NOTIFY_OK;
	case NETDEV_UNREGISTER:
		if (xway_switch->wanoe_netdev) {
			dev_dbg(xway_switch->dev,
				"unregistering %s as WANoE device\n", dev->name);
			dev_put(dev);
			xway_switch->wanoe_netdev = NULL;
		}
		return NOTIFY_OK;
	case NETDEV_UP:
	case NETDEV_DOWN:
		for (i = 0; i < XWAY_SWITCH_MAX_PORTS; i++) {
			port = xway_switch->port[i];
			if (!port)
				continue;

			if (!port->is_wanoe)
				continue;

			if (!port->phydev)
				continue;

			if (port->phydev->link)
				netif_carrier_on(xway_switch->wanoe_netdev);
			else
				netif_carrier_off(xway_switch->wanoe_netdev);
		}
		return NOTIFY_OK;
	default:
		return NOTIFY_DONE;
	}
}

static const struct xway_switch_hwcfg switch_xrx200 = {
	.cpu_port = 6,
	.port_cnt = 6,
	.init = xway_switch_xrx200_init,
	.port_init = xway_switch_xrx200_port_init,
	.port_open = xway_switch_xrx200_port_open,
	.port_stop = xway_switch_xrx200_port_stop,
	.gmac_update = xway_switch_xrx200_gmac_update,
};

static const struct xway_switch_hwcfg switch_xrx330 = {
	.cpu_port = 6,
	.port_cnt = 6,
	.init = xway_switch_xrx330_init,
	.port_init = xway_switch_xrx330_port_init,
	.port_open = xway_switch_xrx330_port_open,
	.port_stop = xway_switch_xrx330_port_stop,
	.gmac_update = xway_switch_xrx330_gmac_update,
};

static const struct of_device_id xway_switch_match[] = {
	{ .compatible = "lantiq,switch-xrx200", .data = &switch_xrx200, },
	{ .compatible = "lantiq,switch-xrx330", .data = &switch_xrx330, },
	{ },
};
MODULE_DEVICE_TABLE(of, xway_switch_match);

static int xway_switch_probe(struct platform_device *pdev)
{
	struct xway_switch *xway_switch;
	const struct xway_switch_hwcfg *hwcfg;
	const struct of_device_id *match;
	struct resource *res;
	void __iomem *regbase;
	int err;

	build_check_registers();

	match = of_match_device(xway_switch_match, &pdev->dev);
	if (!match) {
		dev_err(&pdev->dev, "no device match\n");
		return -EINVAL;
	}
	hwcfg = match->data;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res)
		return -ENOENT;

	regbase = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(regbase))
		return PTR_ERR(regbase);

	xway_switch = devm_kzalloc(&pdev->dev, sizeof(*xway_switch), GFP_KERNEL);
	if (!xway_switch)
		return -ENOMEM;

	xway_switch->dev = &pdev->dev;
	xway_switch->regbase = regbase;
	xway_switch->hwcfg = hwcfg;

	xway_switch->clk = clk_get_sys("1e108000.eth", NULL);
	if (IS_ERR(xway_switch->clk))
		return PTR_ERR(xway_switch->clk);
	clk_enable(xway_switch->clk);

	err = xway_switch_api_init(xway_switch);
	if (err)
		goto err_clk;

	hwcfg->init(xway_switch);

	err = xway_switch_ports_init(xway_switch);
	if (err)
		goto err_ethsw_api;

	err = xway_switch_mdio_init(xway_switch);
	if (err)
		goto err_ports;

	err = xway_switch_phy_start(xway_switch);
	if (err)
		goto err_mdio;

	xway_switch->nb.notifier_call = xway_switch_netdev_notifier;
	err = register_netdevice_notifier(&xway_switch->nb);
	if (err)
		goto err_phy;

	platform_set_drvdata(pdev, xway_switch);

	return 0;

err_phy:
	xway_switch_phy_stop(xway_switch);
err_mdio:
	xway_switch_mdio_exit(xway_switch);
err_ports:
	xway_switch_ports_exit(xway_switch);
err_ethsw_api:
	xway_switch_api_exit(xway_switch);
err_clk:
	clk_disable(xway_switch->clk);
	clk_put(xway_switch->clk);

	return err;
}

static int xway_switch_remove(struct platform_device *pdev)
{
	struct xway_switch *xway_switch = platform_get_drvdata(pdev);

	unregister_netdevice_notifier(&xway_switch->nb);

	if (xway_switch->wanoe_netdev)
		dev_put(xway_switch->wanoe_netdev);

	xway_switch_phy_stop(xway_switch);
	xway_switch_mdio_exit(xway_switch);
	xway_switch_ports_exit(xway_switch);
	xway_switch_api_exit(xway_switch);

	clk_disable(xway_switch->clk);
	clk_put(xway_switch->clk);

	platform_set_drvdata(pdev, NULL);

	return 0;
}

static struct platform_driver xway_switch_driver = {
	.probe = xway_switch_probe,
	.remove = xway_switch_remove,
	.driver = {
		.name = "xway-switch",
		.owner = THIS_MODULE,
		.of_match_table = xway_switch_match,
	},
};

static int __init xway_switch_init(void)
{
	int err;

	err = xway_gphy_init();
	if (err)
		return err;

	msleep(100);

	err = platform_driver_register(&xway_switch_driver);
	if (err)
		goto err_gphy;

	return 0;

err_gphy:
	xway_gphy_exit();

	return err;
}

static void __exit xway_switch_exit(void)
{
	platform_driver_unregister(&xway_switch_driver);
	xway_gphy_exit();
}

module_init(xway_switch_init);
module_exit(xway_switch_exit);

MODULE_AUTHOR("Daniel Schwierzeck <daniel.schwierzeck@sphairon.com>");
MODULE_DESCRIPTION("Lantiq XWAY switch driver");
MODULE_LICENSE("GPL");
