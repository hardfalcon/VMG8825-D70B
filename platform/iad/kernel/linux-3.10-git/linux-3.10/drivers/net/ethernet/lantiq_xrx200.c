/*
 *   This program is free software; you can redistribute it and/or modify it
 *   under the terms of the GNU General Public License version 2 as published
 *   by the Free Software Foundation.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 *
 *   Copyright (C) 2010 Lantiq Deutschland
 *   Copyright (C) 2012 John Crispin <blogic@openwrt.org>
 */

#include <linux/etherdevice.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/clk.h>
#include <asm/delay.h>

#include <linux/of_net.h>
#include <linux/of_mdio.h>
#include <linux/of_gpio.h>

#include <xway_dma.h>
#include <lantiq_soc.h>

#include "lantiq_pce.h"

#define SW_POLLING
#define SW_ROUTING
#define SW_PORTMAP

#ifdef SW_ROUTING
  #ifdef SW_PORTMAP
#define XRX200_MAX_DEV		2
  #else
#define XRX200_MAX_DEV		2
  #endif
#else
#define XRX200_MAX_DEV		1
#endif

#define XRX200_MAX_PORT		7
#define XRX200_MAX_DMA		8

#define XRX200_HEADROOM		4

#define XRX200_TX_TIMEOUT	(10 * HZ)

/* port type */
#define XRX200_PORT_TYPE_PHY	1
#define XRX200_PORT_TYPE_MAC	2

/* DMA */
#define XRX200_DMA_CRC_LEN	0x4
#define XRX200_DMA_DATA_LEN	0x600
#define XRX200_DMA_IRQ		INT_NUM_IM2_IRL0
#define XRX200_DMA_RX		0
#define XRX200_DMA_TX		1
#define XRX200_DMA_IS_TX(x)	(x%2)
#define XRX200_DMA_IS_RX(x)	(!XRX200_DMA_IS_TX(x))

/* fetch / store dma */
#define FDMA_PCTRL0		0x2A00
#define FDMA_PCTRLx(x)		(FDMA_PCTRL0 + (x * 0x18))
#define SDMA_PCTRL0		0x2F00
#define SDMA_PCTRLx(x)		(SDMA_PCTRL0 + (x * 0x18))

/* buffer management */
#define BM_PCFG0		0x200
#define BM_PCFGx(x)		(BM_PCFG0 + (x * 8))

/* MDIO */
#define MDIO_GLOB		0x0000
#define MDIO_CTRL		0x0020
#define MDIO_READ		0x0024
#define MDIO_WRITE		0x0028
#define MDIO_PHY0		0x0054
#define MDIO_PHY(x)		(0x0054 - (x * sizeof(unsigned)))
#define MDIO_CLK_CFG0		0x002C
#define MDIO_CLK_CFG1		0x0030

#define MDIO_GLOB_ENABLE	0x8000
#define MDIO_BUSY		BIT(12)
#define MDIO_RD			BIT(11)
#define MDIO_WR			BIT(10)
#define MDIO_MASK		0x1f
#define MDIO_ADDRSHIFT		5
#define MDIO1_25MHZ		9

#define MDIO_PHY_LINK_DOWN	0x4000
#define MDIO_PHY_LINK_UP	0x2000

#define MDIO_PHY_SPEED_M10	0x0000
#define MDIO_PHY_SPEED_M100	0x0800
#define MDIO_PHY_SPEED_G1	0x1000

#define MDIO_PHY_FDUP_EN	0x0600
#define MDIO_PHY_FDUP_DIS	0x0200

#define MDIO_PHY_LINK_MASK	0x6000
#define MDIO_PHY_SPEED_MASK	0x1800
#define MDIO_PHY_FDUP_MASK	0x0600
#define MDIO_PHY_ADDR_MASK	0x001f
#define MDIO_UPDATE_MASK	MDIO_PHY_ADDR_MASK | MDIO_PHY_LINK_MASK | \
					MDIO_PHY_SPEED_MASK | MDIO_PHY_FDUP_MASK

/* MII */
#define MII_CFG(p)		(p * 8)

#define MII_CFG_EN		BIT(14)

#define MII_CFG_MODE_MIIP	0x0
#define MII_CFG_MODE_MIIM	0x1
#define MII_CFG_MODE_RMIIP	0x2
#define MII_CFG_MODE_RMIIM	0x3
#define MII_CFG_MODE_RGMII	0x4
#define MII_CFG_MODE_MASK	0xf

#define MII_CFG_RATE_M2P5	0x00
#define MII_CFG_RATE_M25	0x10
#define MII_CFG_RATE_M125	0x20
#define MII_CFG_RATE_M50	0x30
#define MII_CFG_RATE_AUTO	0x40
#define MII_CFG_RATE_MASK	0x70

/* cpu port mac */
#define PMAC_HD_CTL		0x0000
#define PMAC_RX_IPG		0x0024
#define PMAC_EWAN		0x002c

#define PMAC_IPG_MASK		0xf
#define PMAC_HD_CTL_AS		0x0008
#define PMAC_HD_CTL_AC		0x0004
#define PMAC_HD_CTL_RXSH	0x0040
#define PMAC_HD_CTL_AST		0x0080
#define PMAC_HD_CTL_RST		0x0100

/* PCE */
#define PCE_TBL_KEY(x)		(0x1100 + ((7 - x) * 4))
#define PCE_TBL_MASK		0x1120
#define PCE_TBL_VAL(x)		(0x1124 + ((4 - x) * 4))
#define PCE_TBL_ADDR		0x1138
#define PCE_TBL_CTRL		0x113c
#define PCE_PMAP1		0x114c
#define PCE_PMAP2		0x1150
#define PCE_PMAP3		0x1154
#define PCE_GCTRL_REG(x)	(0x1158 + (x * 4))
#define PCE_PCTRL_REG(p, x)	(0x1200 + (((p * 0xa) + x) * 4))

#define PCE_TBL_BUSY		BIT(15)
#define PCE_TBL_CFG_ADDR_MASK	0x1f
#define PCE_TBL_CFG_ADWR	0x20
#define PCE_TBL_CFG_ADWR_MASK	0x60
#define PCE_INGRESS		BIT(11)

/* MAC */
#define MAC_FLEN_REG		(0x2314)
#define MAC_CTRL_REG(p, x)	(0x240c + (((p * 0xc) + x) * 4))

/* buffer management */
#define BM_PCFG(p)		(0x200 + (p * 8))

/* special tag in TX path header */
#define SPID_SHIFT		24
#define DPID_SHIFT		16
#define DPID_ENABLE		1
#define SPID_CPU_PORT		2
#define PORT_MAP_SEL		BIT(15)
#define PORT_MAP_EN		BIT(14)
#define PORT_MAP_SHIFT		1
#define PORT_MAP_MASK		0x3f

#define SPPID_MASK		0x7
#define SPPID_SHIFT		4

/* MII regs not yet in linux */
#define MDIO_DEVAD_NONE		(-1)
#define ADVERTIZE_MPD		(1 << 10)

struct xrx200_port {
	u8 num;
	u8 phy_addr;
	u16 flags;
	phy_interface_t phy_if;

	int link;
	int gpio;
	enum of_gpio_flags gpio_flags;

	struct phy_device *phydev;
	struct device_node *phy_node;
};

struct xrx200_chan {
	int idx;
	int refcount;
	int tx_free;

	struct net_device dummy_dev;
	struct net_device *devs[XRX200_MAX_DEV];

	struct tasklet_struct tasklet;
	struct napi_struct napi;
	struct ltq_dma_channel dma;
	struct sk_buff *skb[LTQ_DESC_NUM];
};

struct xrx200_hw {
	struct clk *clk;
	struct mii_bus *mii_bus;

	struct xrx200_chan chan[XRX200_MAX_DMA];

	struct net_device *devs[XRX200_MAX_DEV];
	int num_devs;

	int port_map[XRX200_MAX_PORT];
	unsigned short wan_map;

	spinlock_t lock;
};

struct xrx200_priv {
	struct net_device_stats stats;
	int id;

	struct xrx200_port port[XRX200_MAX_PORT];
	int num_port;
	int wan;
	unsigned short port_map;
	unsigned char mac[6];

	struct xrx200_hw *hw;
};

static __iomem void *xrx200_switch_membase;
static __iomem void *xrx200_mii_membase;
static __iomem void *xrx200_mdio_membase;
static __iomem void *xrx200_pmac_membase;

#define ltq_switch_r32(x)	ltq_r32(xrx200_switch_membase + (x))
#define ltq_switch_w32(x, y)	ltq_w32(x, xrx200_switch_membase + (y))
#define ltq_switch_w32_mask(x, y, z) \
			ltq_w32_mask(x, y, xrx200_switch_membase + (z))

#define ltq_mdio_r32(x)		ltq_r32(xrx200_mdio_membase + (x))
#define ltq_mdio_w32(x, y)	ltq_w32(x, xrx200_mdio_membase + (y))
#define ltq_mdio_w32_mask(x, y, z) \
			ltq_w32_mask(x, y, xrx200_mdio_membase + (z))

#define ltq_mii_r32(x)		ltq_r32(xrx200_mii_membase + (x))
#define ltq_mii_w32(x, y)	ltq_w32(x, xrx200_mii_membase + (y))
#define ltq_mii_w32_mask(x, y, z) \
			ltq_w32_mask(x, y, xrx200_mii_membase + (z))

#define ltq_pmac_r32(x)		ltq_r32(xrx200_pmac_membase + (x))
#define ltq_pmac_w32(x, y)	ltq_w32(x, xrx200_pmac_membase + (y))
#define ltq_pmac_w32_mask(x, y, z) \
			ltq_w32_mask(x, y, xrx200_pmac_membase + (z))

static int xrx200_open(struct net_device *dev)
{
	struct xrx200_priv *priv = netdev_priv(dev);
	unsigned long flags;
	int i;

	for (i = 0; i < XRX200_MAX_DMA; i++) {
		if (!priv->hw->chan[i].dma.irq)
			continue;
		spin_lock_irqsave(&priv->hw->lock, flags);
		if (!priv->hw->chan[i].refcount) {
			if (XRX200_DMA_IS_RX(i))
				napi_enable(&priv->hw->chan[i].napi);
			ltq_dma_open(&priv->hw->chan[i].dma);
		}
		priv->hw->chan[i].refcount++;
		spin_unlock_irqrestore(&priv->hw->lock, flags);
	}
	for (i = 0; i < priv->num_port; i++)
		if (priv->port[i].phydev)
			phy_start(priv->port[i].phydev);
	netif_start_queue(dev);

	return 0;
}

static int xrx200_close(struct net_device *dev)
{
	struct xrx200_priv *priv = netdev_priv(dev);
	unsigned long flags;
	int i;

	netif_stop_queue(dev);

	for (i = 0; i < priv->num_port; i++)
		if (priv->port[i].phydev)
			phy_stop(priv->port[i].phydev);

	for (i = 0; i < XRX200_MAX_DMA; i++) {
		if (!priv->hw->chan[i].dma.irq)
			continue;
		spin_lock_irqsave(&priv->hw->lock, flags);
		priv->hw->chan[i].refcount--;
		if (!priv->hw->chan[i].refcount) {
			if (XRX200_DMA_IS_RX(i))
				napi_disable(&priv->hw->chan[i].napi);
			ltq_dma_close(&priv->hw->chan[XRX200_DMA_RX].dma);
		}
		spin_unlock_irqrestore(&priv->hw->lock, flags);
	}

	return 0;
}

static int xrx200_alloc_skb(struct xrx200_chan *ch)
{
#define DMA_PAD	(NET_IP_ALIGN + NET_SKB_PAD)
	ch->skb[ch->dma.desc] = dev_alloc_skb(XRX200_DMA_DATA_LEN + DMA_PAD);
	if (!ch->skb[ch->dma.desc])
		return -ENOMEM;

	skb_reserve(ch->skb[ch->dma.desc], NET_SKB_PAD);
	ch->dma.desc_base[ch->dma.desc].addr = dma_map_single(NULL,
		ch->skb[ch->dma.desc]->data, XRX200_DMA_DATA_LEN,
			DMA_FROM_DEVICE);
	ch->dma.desc_base[ch->dma.desc].addr =
		CPHYSADDR(ch->skb[ch->dma.desc]->data);
	ch->dma.desc_base[ch->dma.desc].ctl =
		LTQ_DMA_OWN | LTQ_DMA_RX_OFFSET(NET_IP_ALIGN) |
		XRX200_DMA_DATA_LEN;
	skb_reserve(ch->skb[ch->dma.desc], NET_IP_ALIGN);

	return 0;
}

static void xrx200_hw_receive(struct xrx200_chan *ch, int id)
{
	struct net_device *dev = ch->devs[id];
	struct xrx200_priv *priv = netdev_priv(dev);
	struct ltq_dma_desc *desc = &ch->dma.desc_base[ch->dma.desc];
	struct sk_buff *skb = ch->skb[ch->dma.desc];
	int len = (desc->ctl & LTQ_DMA_SIZE_MASK) - XRX200_DMA_CRC_LEN;
	unsigned long flags;

	spin_lock_irqsave(&priv->hw->lock, flags);
	if (xrx200_alloc_skb(ch)) {
		netdev_err(dev,
			"failed to allocate new rx buffer, stopping DMA\n");
		ltq_dma_close(&ch->dma);
	}

	ch->dma.desc++;
	ch->dma.desc %= LTQ_DESC_NUM;
	spin_unlock_irqrestore(&priv->hw->lock, flags);

	skb_put(skb, len);
#ifdef SW_ROUTING
	skb_pull(skb, 8);
#endif
	skb->dev = dev;
	skb->protocol = eth_type_trans(skb, dev);
	netif_receive_skb(skb);
	priv->stats.rx_packets++;
	priv->stats.rx_bytes+=len;
}

static int xrx200_poll_rx(struct napi_struct *napi, int budget)
{
	struct xrx200_chan *ch = container_of(napi,
				struct xrx200_chan, napi);
	struct xrx200_priv *priv = netdev_priv(ch->devs[0]);
	int rx = 0;
	int complete = 0;
	unsigned long flags;

	while ((rx < budget) && !complete) {
		struct ltq_dma_desc *desc = &ch->dma.desc_base[ch->dma.desc];
		if ((desc->ctl & (LTQ_DMA_OWN | LTQ_DMA_C)) == LTQ_DMA_C) {
#ifdef SW_ROUTING
			struct sk_buff *skb = ch->skb[ch->dma.desc];
			u32 *special_tag = (u32*)skb->data;
			int port = (special_tag[1] >> SPPID_SHIFT) & SPPID_MASK;
			xrx200_hw_receive(ch, priv->hw->port_map[port]);
#else
			xrx200_hw_receive(ch, 0);
#endif
			rx++;
		} else {
			complete = 1;
		}
	}
	if (complete || !rx) {
		napi_complete(&ch->napi);
		spin_lock_irqsave(&priv->hw->lock, flags);
		ltq_dma_ack_irq(&ch->dma);
		spin_unlock_irqrestore(&priv->hw->lock, flags);
	}
	return rx;
}

static void xrx200_tx_housekeeping(unsigned long ptr)
{
	struct xrx200_hw *hw = (struct xrx200_hw *) ptr;
	struct xrx200_chan *ch = &hw->chan[XRX200_DMA_TX];
	unsigned long flags;
	int i;

	spin_lock_irqsave(&hw->lock, flags);
	while ((ch->dma.desc_base[ch->tx_free].ctl & (LTQ_DMA_OWN | LTQ_DMA_C)) == LTQ_DMA_C) {
		dev_kfree_skb_any(ch->skb[ch->tx_free]);
		ch->skb[ch->tx_free] = NULL;
		memset(&ch->dma.desc_base[ch->tx_free], 0,
			sizeof(struct ltq_dma_desc));
		ch->tx_free++;
		ch->tx_free %= LTQ_DESC_NUM;
	}
	spin_unlock_irqrestore(&hw->lock, flags);

	for (i = 0; i < XRX200_MAX_DEV && ch->devs[i]; i++) {
		struct netdev_queue *txq =
			netdev_get_tx_queue(ch->devs[i], 0);
		if (netif_tx_queue_stopped(txq))
			netif_tx_start_queue(txq);
	}

	spin_lock_irqsave(&hw->lock, flags);
	ltq_dma_ack_irq(&ch->dma);
	spin_unlock_irqrestore(&hw->lock, flags);
}

static struct net_device_stats *xrx200_get_stats (struct net_device *dev)
{
	struct xrx200_priv *priv = netdev_priv(dev);

	return &priv->stats;
}

static void xrx200_tx_timeout(struct net_device *dev)
{
	struct xrx200_priv *priv = netdev_priv(dev);

	printk(KERN_ERR "%s: transmit timed out, disable the dma channel irq\n", dev->name);

	priv->stats.tx_errors++;
	netif_wake_queue(dev);
}

static int xrx200_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
	int queue = skb_get_queue_mapping(skb);
	struct netdev_queue *txq = netdev_get_tx_queue(dev, queue);
	struct xrx200_priv *priv = netdev_priv(dev);
	struct xrx200_chan *ch = &priv->hw->chan[XRX200_DMA_TX];
	struct ltq_dma_desc *desc = &ch->dma.desc_base[ch->dma.desc];
	unsigned long flags;
	u32 byte_offset;
	int len;
#ifdef SW_ROUTING
  #ifdef SW_PORTMAP
	u32 special_tag = (SPID_CPU_PORT << SPID_SHIFT) | PORT_MAP_SEL | PORT_MAP_EN | DPID_ENABLE;
  #else
	u32 special_tag = (SPID_CPU_PORT << SPID_SHIFT) | DPID_ENABLE;
  #endif
#endif

	len = skb->len < ETH_ZLEN ? ETH_ZLEN : skb->len;

	if ((desc->ctl & (LTQ_DMA_OWN | LTQ_DMA_C)) || ch->skb[ch->dma.desc]) {
		netdev_err(dev, "tx ring full\n");
		netif_tx_stop_queue(txq);
		return NETDEV_TX_BUSY;
	}
#ifdef SW_ROUTING
  #ifdef SW_PORTMAP
	special_tag |= priv->port_map << PORT_MAP_SHIFT;
  #else
	if(priv->id)
		special_tag |= (1 << DPID_SHIFT);
  #endif
	if(skb_headroom(skb) < 4) {
		struct sk_buff *tmp = skb_realloc_headroom(skb, 4);
		dev_kfree_skb_any(skb);
		skb = tmp;
	}
	skb_push(skb, 4);
	memcpy(skb->data, &special_tag, sizeof(u32));
	len += 4;
#endif

	/* dma needs to start on a 16 byte aligned address */
	byte_offset = CPHYSADDR(skb->data) % 16;
	ch->skb[ch->dma.desc] = skb;

	dev->trans_start = jiffies;

	spin_lock_irqsave(&priv->hw->lock, flags);
	desc->addr = ((unsigned int) dma_map_single(NULL, skb->data, len,
						DMA_TO_DEVICE)) - byte_offset;
	wmb();
	desc->ctl = LTQ_DMA_OWN | LTQ_DMA_SOP | LTQ_DMA_EOP |
		LTQ_DMA_TX_OFFSET(byte_offset) | (len & LTQ_DMA_SIZE_MASK);
	ch->dma.desc++;
	ch->dma.desc %= LTQ_DESC_NUM;
	spin_unlock_irqrestore(&priv->hw->lock, flags);

	if (ch->dma.desc_base[ch->dma.desc].ctl & LTQ_DMA_OWN)
		netif_tx_stop_queue(txq);

	priv->stats.tx_packets++;
	priv->stats.tx_bytes+=len;

	return NETDEV_TX_OK;
}

static irqreturn_t xrx200_dma_irq(int irq, void *priv)
{
	struct xrx200_hw *hw = priv;
	int ch = irq - XRX200_DMA_IRQ;

	if (ch % 2)
		tasklet_schedule(&hw->chan[ch].tasklet);
	else
		napi_schedule(&hw->chan[ch].napi);

	return IRQ_HANDLED;
}

static int xrx200_dma_init(struct xrx200_hw *hw)
{
	int i, err = 0;

	ltq_dma_init_port(DMA_PORT_ETOP);

	for (i = 0; i < 8 && !err; i++) {
		int irq = XRX200_DMA_IRQ + i;
		struct xrx200_chan *ch = &hw->chan[i];

		ch->idx = ch->dma.nr = i;

		if (i == XRX200_DMA_TX) {
			ltq_dma_alloc_tx(&ch->dma);
			err = request_irq(irq, xrx200_dma_irq, 0, "vrx200_tx", hw);
		} else if (i == XRX200_DMA_RX) {
			ltq_dma_alloc_rx(&ch->dma);
			for (ch->dma.desc = 0; ch->dma.desc < LTQ_DESC_NUM;
					ch->dma.desc++)
				if (xrx200_alloc_skb(ch))
					err = -ENOMEM;
			ch->dma.desc = 0;
			err = request_irq(irq, xrx200_dma_irq, 0, "vrx200_rx", hw);
		} else
			continue;

		if (!err)
			ch->dma.irq = irq;
	}

	return err;
}

#ifdef SW_POLLING
static void xrx200_gmac_update(struct xrx200_port *port)
{
	u16 phyaddr = port->phydev->addr & MDIO_PHY_ADDR_MASK;
	u16 miimode = ltq_mii_r32(MII_CFG(port->num)) & MII_CFG_MODE_MASK;
	u16 miirate = 0;

	switch (port->phydev->speed) {
	case SPEED_1000:
		phyaddr |= MDIO_PHY_SPEED_G1;
		miirate = MII_CFG_RATE_M125;
		break;

	case SPEED_100:
		phyaddr |= MDIO_PHY_SPEED_M100;
		switch (miimode) {
		case MII_CFG_MODE_RMIIM:
		case MII_CFG_MODE_RMIIP:
			miirate = MII_CFG_RATE_M50;
			break;
		default:
			miirate = MII_CFG_RATE_M25;
			break;
		}
		break;

	default:
		phyaddr |= MDIO_PHY_SPEED_M10;
		miirate = MII_CFG_RATE_M2P5;
		break;
	}

	if (port->phydev->link)
		phyaddr |= MDIO_PHY_LINK_UP;
	else
		phyaddr |= MDIO_PHY_LINK_DOWN;

	if (port->phydev->duplex == DUPLEX_FULL)
		phyaddr |= MDIO_PHY_FDUP_EN;
	else
		phyaddr |= MDIO_PHY_FDUP_DIS;

	ltq_mdio_w32_mask(MDIO_UPDATE_MASK, phyaddr, MDIO_PHY(port->num));
	ltq_mii_w32_mask(MII_CFG_RATE_MASK, miirate, MII_CFG(port->num));
	udelay(1);
}
#else
static void xrx200_gmac_update(struct xrx200_port *port)
{

}
#endif

static void xrx200_mdio_link(struct net_device *dev)
{
	struct xrx200_priv *priv = netdev_priv(dev);
	int i;

	for (i = 0; i < priv->num_port; i++) {
		if (!priv->port[i].phydev)
			continue;

		if (priv->port[i].link != priv->port[i].phydev->link) {
			xrx200_gmac_update(&priv->port[i]);
			priv->port[i].link = priv->port[i].phydev->link;
			netdev_info(dev, "port %d %s link\n",
				priv->port[i].num,
				(priv->port[i].link)?("got"):("lost"));
		}
	}
}

static inline int xrx200_mdio_poll(struct mii_bus *bus)
{
	unsigned cnt = 10000;

	while (likely(cnt--)) {
		unsigned ctrl = ltq_mdio_r32(MDIO_CTRL);
		if ((ctrl & MDIO_BUSY) == 0)
			return 0;
	}

	return 1;
}

static int xrx200_mdio_wr(struct mii_bus *bus, int addr, int reg, u16 val)
{
	if (xrx200_mdio_poll(bus))
		return 1;

	ltq_mdio_w32(val, MDIO_WRITE);
	ltq_mdio_w32(MDIO_BUSY | MDIO_WR |
		((addr & MDIO_MASK) << MDIO_ADDRSHIFT) |
		(reg & MDIO_MASK),
		MDIO_CTRL);

	return 0;
}

static int xrx200_mdio_rd(struct mii_bus *bus, int addr, int reg)
{
	if (xrx200_mdio_poll(bus))
		return -1;

	ltq_mdio_w32(MDIO_BUSY | MDIO_RD |
		((addr & MDIO_MASK) << MDIO_ADDRSHIFT) |
		(reg & MDIO_MASK),
		MDIO_CTRL);

	if (xrx200_mdio_poll(bus))
		return -1;

	return ltq_mdio_r32(MDIO_READ);
}

static int xrx200_mdio_probe(struct net_device *dev, struct xrx200_port *port)
{
	struct xrx200_priv *priv = netdev_priv(dev);
	struct phy_device *phydev = NULL;
	unsigned val;

	phydev = priv->hw->mii_bus->phy_map[port->phy_addr];

	if (!phydev) {
		netdev_err(dev, "no PHY found\n");
		return -ENODEV;
	}

	phydev = phy_connect(dev, dev_name(&phydev->dev), &xrx200_mdio_link,
				port->phy_if);

	if (IS_ERR(phydev)) {
		netdev_err(dev, "Could not attach to PHY\n");
		return PTR_ERR(phydev);
	}

	phydev->supported &= (SUPPORTED_10baseT_Half
			| SUPPORTED_10baseT_Full
			| SUPPORTED_100baseT_Half
			| SUPPORTED_100baseT_Full
			| SUPPORTED_1000baseT_Half
			| SUPPORTED_1000baseT_Full
			| SUPPORTED_Autoneg
			| SUPPORTED_MII
			| SUPPORTED_TP);
	phydev->advertising = phydev->supported;
	port->phydev = phydev;

	pr_info("%s: attached PHY [%s] (phy_addr=%s, irq=%d)\n",
		dev->name, phydev->drv->name,
		dev_name(&phydev->dev), phydev->irq);

#ifdef SW_POLLING
	phy_read_status(phydev);

	val = xrx200_mdio_rd(priv->hw->mii_bus, MDIO_DEVAD_NONE, MII_CTRL1000);
	val |= ADVERTIZE_MPD;
	xrx200_mdio_wr(priv->hw->mii_bus, MDIO_DEVAD_NONE, MII_CTRL1000, val);
	xrx200_mdio_wr(priv->hw->mii_bus, 0, 0, 0x1040);

	phy_start_aneg(phydev);
#endif
	return 0;
}

static void xrx200_port_config(struct xrx200_priv *priv,
		const struct xrx200_port *port)
{
	u16 miimode = 0;

	switch (port->num) {
	case 0: /* xMII0 */
	case 1: /* xMII1 */
		switch (port->phy_if) {
		case PHY_INTERFACE_MODE_MII:
			if (port->flags & XRX200_PORT_TYPE_PHY)
				/* MII MAC mode, connected to external PHY */
				miimode = MII_CFG_MODE_MIIM;
			else
				/* MII PHY mode, connected to external MAC */
				miimode = MII_CFG_MODE_MIIP;
			break;
		case PHY_INTERFACE_MODE_RMII:
			if (port->flags & XRX200_PORT_TYPE_PHY)
				/* RMII MAC mode, connected to external PHY */
				miimode = MII_CFG_MODE_RMIIM;
			else
				/* RMII PHY mode, connected to external MAC */
				miimode = MII_CFG_MODE_RMIIP;
			break;
		case PHY_INTERFACE_MODE_RGMII:
			/* RGMII MAC mode, connected to external PHY */
			miimode = MII_CFG_MODE_RGMII;
			break;
		default:
			break;
		}
		break;
	case 2: /* internal GPHY0 */
	case 3: /* internal GPHY0 */
	case 4: /* internal GPHY1 */
		switch (port->phy_if) {
			case PHY_INTERFACE_MODE_MII:
			case PHY_INTERFACE_MODE_GMII:
				/* MII MAC mode, connected to internal GPHY */
				miimode = MII_CFG_MODE_MIIM;
				break;
			default:
				break;
		}
		break;
	case 5: /* internal GPHY1 or xMII2 */
		switch (port->phy_if) {
		case PHY_INTERFACE_MODE_MII:
			/* MII MAC mode, connected to internal GPHY */
			miimode = MII_CFG_MODE_MIIM;
			break;
		case PHY_INTERFACE_MODE_RGMII:
			/* RGMII MAC mode, connected to external PHY */
			miimode = MII_CFG_MODE_RGMII;
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}

	ltq_mii_w32_mask(MII_CFG_MODE_MASK, miimode | MII_CFG_EN,
		MII_CFG(port->num));
}

static int xrx200_init(struct net_device *dev)
{
	struct xrx200_priv *priv = netdev_priv(dev);
	struct sockaddr mac;
	int err, i;

#ifndef SW_POLLING
	unsigned int reg = 0;

	/* enable auto polling */
	for (i = 0; i < priv->num_port; i++)
		reg |= BIT(priv->port[i].num);
	ltq_mdio_w32(reg, MDIO_CLK_CFG0);
	ltq_mdio_w32(MDIO1_25MHZ, MDIO_CLK_CFG1);
#endif

	/* setup each port */
	for (i = 0; i < priv->num_port; i++)
		xrx200_port_config(priv, &priv->port[i]);

	memcpy(&mac.sa_data, priv->mac, ETH_ALEN);
	if (!is_valid_ether_addr(mac.sa_data)) {
		pr_warn("net-xrx200: invalid MAC, using random\n");
		eth_random_addr(mac.sa_data);
		dev->addr_assign_type |= NET_ADDR_RANDOM;
	}

	err = eth_mac_addr(dev, &mac);
	if (err)
		goto err_netdev;

	for (i = 0; i < priv->num_port; i++)
		if (xrx200_mdio_probe(dev, &priv->port[i]))
			pr_warn("xrx200-mdio: probing phy of port %d failed\n",
					 priv->port[i].num);

	return 0;

err_netdev:
	unregister_netdev(dev);
	free_netdev(dev);
	return err;
}

static void xrx200_pci_microcode(void)
{
	int i;

	ltq_switch_w32_mask(PCE_TBL_CFG_ADDR_MASK | PCE_TBL_CFG_ADWR_MASK,
		PCE_TBL_CFG_ADWR, PCE_TBL_CTRL);
	ltq_switch_w32(0, PCE_TBL_MASK);

	for (i = 0; i < ARRAY_SIZE(pce_microcode); i++) {
		ltq_switch_w32(i, PCE_TBL_ADDR);
		ltq_switch_w32(pce_microcode[i].val[3], PCE_TBL_VAL(0));
		ltq_switch_w32(pce_microcode[i].val[2], PCE_TBL_VAL(1));
		ltq_switch_w32(pce_microcode[i].val[1], PCE_TBL_VAL(2));
		ltq_switch_w32(pce_microcode[i].val[0], PCE_TBL_VAL(3));

		// start the table access:
		ltq_switch_w32_mask(0, PCE_TBL_BUSY, PCE_TBL_CTRL);
		while (ltq_switch_r32(PCE_TBL_CTRL) & PCE_TBL_BUSY);
	}

	/* tell the switch that the microcode is loaded */
	ltq_switch_w32_mask(0, BIT(3), PCE_GCTRL_REG(0));
}

static void xrx200_hw_init(struct xrx200_hw *hw)
{
	int i;

	/* enable clock gate */
	clk_enable(hw->clk);

	ltq_switch_w32(1, 0);
	mdelay(100);
	ltq_switch_w32(0, 0);
	/*
	 * TODO: we should really disbale all phys/miis here and explicitly
	 * enable them in the device secific init function
	 */

	/* disable port fetch/store dma */
	for (i = 0; i < 7; i++ ) {
		ltq_switch_w32(0, FDMA_PCTRLx(i));
		ltq_switch_w32(0, SDMA_PCTRLx(i));
	}

	/* enable Switch */
	ltq_mdio_w32_mask(0, MDIO_GLOB_ENABLE, MDIO_GLOB);

	/* load the pce microcode */
	xrx200_pci_microcode();

	/* Default unknown Broadcat/Multicast/Unicast port maps */
	ltq_switch_w32(0x7f, PCE_PMAP1);
	ltq_switch_w32(0x7f, PCE_PMAP2);
	ltq_switch_w32(0x7f, PCE_PMAP3);

	/* RMON Counter Enable for all physical ports */
	for (i = 0; i < 7; i++)
		ltq_switch_w32(0x1, BM_PCFG(i));

	/* disable auto polling */
	ltq_mdio_w32(0x0, MDIO_CLK_CFG0);

	/* enable port statistic counters */
	for (i = 0; i < 7; i++)
		ltq_switch_w32(0x1, BM_PCFGx(i));

	/* set IPG to 12 */
	ltq_pmac_w32_mask(PMAC_IPG_MASK, 0xb, PMAC_RX_IPG);

#ifdef SW_ROUTING
	/* enable status header, enable CRC */
	ltq_pmac_w32_mask(0,
		PMAC_HD_CTL_RST | PMAC_HD_CTL_AST | PMAC_HD_CTL_RXSH | PMAC_HD_CTL_AS | PMAC_HD_CTL_AC,
		PMAC_HD_CTL);
#else
	/* disable status header, enable CRC */
	ltq_pmac_w32_mask(PMAC_HD_CTL_AST | PMAC_HD_CTL_RXSH | PMAC_HD_CTL_AS,
		PMAC_HD_CTL_AC,
		PMAC_HD_CTL);
#endif

	/* enable port fetch/store dma */
	for (i = 0; i < 7; i++ ) {
		ltq_switch_w32_mask(0, 0x01, FDMA_PCTRLx(i));
		ltq_switch_w32_mask(0, 0x01, SDMA_PCTRLx(i));
		ltq_switch_w32_mask(0, PCE_INGRESS, PCE_PCTRL_REG(i, 0));
	}

	/* enable special tag insertion on cpu port */
	ltq_switch_w32_mask(0, 0x02, FDMA_PCTRLx(6));
	ltq_switch_w32_mask(0, PCE_INGRESS, PCE_PCTRL_REG(6, 0));
	ltq_switch_w32_mask(0, BIT(3), MAC_CTRL_REG(6, 2));
	ltq_switch_w32(1518 + 8 + 4 * 2, MAC_FLEN_REG);
}

static void xrx200_hw_cleanup(struct xrx200_hw *hw)
{
	int i;

	/* disable the switch */
	ltq_mdio_w32_mask(MDIO_GLOB_ENABLE, 0, MDIO_GLOB);

	/* free the channels and IRQs */
	for (i = 0; i < 2; i++) {
		ltq_dma_free(&hw->chan[i].dma);
		if (hw->chan[i].dma.irq)
			free_irq(hw->chan[i].dma.irq, hw);
	}

	/* free the allocated RX ring */
	for (i = 0; i < LTQ_DESC_NUM; i++)
		dev_kfree_skb_any(hw->chan[XRX200_DMA_RX].skb[i]);

	/* clear the mdio bus */
	mdiobus_unregister(hw->mii_bus);
	mdiobus_free(hw->mii_bus);

	/* release the clock */
	clk_disable(hw->clk);
	clk_put(hw->clk);
}

static int xrx200_of_mdio(struct xrx200_hw *hw, struct device_node *np)
{
	hw->mii_bus = mdiobus_alloc();
	if (!hw->mii_bus)
		return -ENOMEM;

	hw->mii_bus->read = xrx200_mdio_rd;
	hw->mii_bus->write = xrx200_mdio_wr;
	hw->mii_bus->name = "lantiq,xrx200-mdio";
	snprintf(hw->mii_bus->id, MII_BUS_ID_SIZE, "%x", 0);

	if (of_mdiobus_register(hw->mii_bus, np)) {
		mdiobus_free(hw->mii_bus);
		return -ENXIO;
	}

	return 0;
}

static void xrx200_of_port(struct xrx200_priv *priv, struct device_node *port)
{
	const __be32 *addr, *id = of_get_property(port, "reg", NULL);
	struct xrx200_port *p = &priv->port[priv->num_port];

	if (!id)
		return;

	memset(p, 0, sizeof(struct xrx200_port));
	p->phy_node = of_parse_phandle(port, "phy-handle", 0);
	addr = of_get_property(p->phy_node, "reg", NULL);
	if (!addr)
		return;

	p->num = *id;
	p->phy_addr = *addr;
	p->phy_if = of_get_phy_mode(port);
	if (p->phy_addr > 0x10)
		p->flags = XRX200_PORT_TYPE_MAC;
	else
		p->flags = XRX200_PORT_TYPE_PHY;
	priv->num_port++;

	p->gpio = of_get_gpio_flags(port, 0, &p->gpio_flags);
	if (gpio_is_valid(p->gpio))
		if (!gpio_request(p->gpio, "phy-reset")) {
			gpio_direction_output(p->gpio,
				(p->gpio_flags & OF_GPIO_ACTIVE_LOW) ? (1) : (0));
			udelay(100);
			gpio_set_value(p->gpio, (p->gpio_flags & OF_GPIO_ACTIVE_LOW) ? (0) : (1));
		}
	/* is this port a wan port ? */
	if (priv->wan)
		priv->hw->wan_map |= BIT(p->num);

	priv->port_map |= BIT(p->num);

	/* store the port id in the hw struct so we can map ports -> devices */
	priv->hw->port_map[p->num] = priv->hw->num_devs;
}

static const struct net_device_ops xrx200_netdev_ops = {
	.ndo_init		= xrx200_init,
	.ndo_open		= xrx200_open,
	.ndo_stop		= xrx200_close,
	.ndo_start_xmit		= xrx200_start_xmit,
	.ndo_set_mac_address	= eth_mac_addr,
	.ndo_validate_addr	= eth_validate_addr,
	.ndo_change_mtu		= eth_change_mtu,
	.ndo_get_stats		= xrx200_get_stats,
	.ndo_tx_timeout		= xrx200_tx_timeout,
};

static void xrx200_of_iface(struct xrx200_hw *hw, struct device_node *iface)
{
	struct xrx200_priv *priv;
	struct device_node *port;
	const __be32 *wan;

	/* alloc the network device */
	hw->devs[hw->num_devs] = alloc_etherdev(sizeof(struct xrx200_priv));
	if (!hw->devs[hw->num_devs])
		return;

	/* setup the network device */
	strcpy(hw->devs[hw->num_devs]->name, "eth%d");
	hw->devs[hw->num_devs]->netdev_ops = &xrx200_netdev_ops;
	hw->devs[hw->num_devs]->watchdog_timeo = XRX200_TX_TIMEOUT;
	hw->devs[hw->num_devs]->needed_headroom = XRX200_HEADROOM;

	/* setup our private data */
	priv = netdev_priv(hw->devs[hw->num_devs]);
	priv->hw = hw;
	of_get_mac_address_mtd(iface, priv->mac);
	priv->id = hw->num_devs;

	/* is this the wan interface ? */
	wan = of_get_property(iface, "lantiq,wan", NULL);
	if (wan && (*wan == 1))
		priv->wan = 1;

	/* load the ports that are part of the interface */
	for_each_child_of_node(iface, port)
		if (of_device_is_compatible(port, "lantiq,xrx200-pdi-port"))
			xrx200_of_port(priv, port);

	/* register the actual device */
	if (!register_netdev(hw->devs[hw->num_devs]))
		hw->num_devs++;
}

static struct xrx200_hw xrx200_hw;

static int xrx200_probe(struct platform_device *pdev)
{
	struct resource *res[4];
	struct device_node *mdio_np, *iface_np;
	int i;

	/* load the memory ranges */
	for (i = 0; i < 4; i++) {
		res[i] = platform_get_resource(pdev, IORESOURCE_MEM, i);
		if (!res[i]) {
			dev_err(&pdev->dev, "failed to get resources\n");
			return -ENOENT;
		}
	}
	xrx200_switch_membase = devm_request_and_ioremap(&pdev->dev, res[0]);
	xrx200_mdio_membase = devm_request_and_ioremap(&pdev->dev, res[1]);
	xrx200_mii_membase = devm_request_and_ioremap(&pdev->dev, res[2]);
	xrx200_pmac_membase = devm_request_and_ioremap(&pdev->dev, res[3]);
	if (!xrx200_switch_membase || !xrx200_mdio_membase ||
			!xrx200_mii_membase || !xrx200_pmac_membase) {
		dev_err(&pdev->dev, "failed to request and remap io ranges \n");
		return -ENOMEM;
	}

	/* get the clock */
	xrx200_hw.clk = clk_get(&pdev->dev, NULL);
	if (IS_ERR(xrx200_hw.clk)) {
		dev_err(&pdev->dev, "failed to get clock\n");
		return PTR_ERR(xrx200_hw.clk);
	}

	/* bring up the dma engine and IP core */
	spin_lock_init(&xrx200_hw.lock);
	xrx200_dma_init(&xrx200_hw);
	xrx200_hw_init(&xrx200_hw);
	tasklet_init(&xrx200_hw.chan[XRX200_DMA_TX].tasklet, xrx200_tx_housekeeping, (u32) &xrx200_hw);

	/* bring up the mdio bus */
	mdio_np = of_find_compatible_node(pdev->dev.of_node, NULL,
				"lantiq,xrx200-mdio");
	if (mdio_np)
		if (xrx200_of_mdio(&xrx200_hw, mdio_np))
			dev_err(&pdev->dev, "mdio probe failed\n");

	/* load the interfaces */
	for_each_child_of_node(pdev->dev.of_node, iface_np)
		if (of_device_is_compatible(iface_np, "lantiq,xrx200-pdi")) {
			if (xrx200_hw.num_devs < XRX200_MAX_DEV)
				xrx200_of_iface(&xrx200_hw, iface_np);
			else
				dev_err(&pdev->dev,
					"only %d interfaces allowed\n",
					XRX200_MAX_DEV);
		}

	if (!xrx200_hw.num_devs) {
		xrx200_hw_cleanup(&xrx200_hw);
		dev_err(&pdev->dev, "failed to load interfaces\n");
		return -ENOENT;
	}

	/* set wan port mask */
	ltq_pmac_w32(xrx200_hw.wan_map, PMAC_EWAN);

	for (i = 0; i < xrx200_hw.num_devs; i++) {
		xrx200_hw.chan[XRX200_DMA_RX].devs[i] = xrx200_hw.devs[i];
		xrx200_hw.chan[XRX200_DMA_TX].devs[i] = xrx200_hw.devs[i];
	}

	/* setup NAPI */
	init_dummy_netdev(&xrx200_hw.chan[XRX200_DMA_RX].dummy_dev);
	netif_napi_add(&xrx200_hw.chan[XRX200_DMA_RX].dummy_dev,
			&xrx200_hw.chan[XRX200_DMA_RX].napi, xrx200_poll_rx, 32);

	platform_set_drvdata(pdev, &xrx200_hw);

	return 0;
}

static int xrx200_remove(struct platform_device *pdev)
{
	struct net_device *dev = platform_get_drvdata(pdev);
	struct xrx200_priv *priv;

	if (!dev)
		return 0;

	priv = netdev_priv(dev);

	/* free stack related instances */
	netif_stop_queue(dev);
	netif_napi_del(&xrx200_hw.chan[XRX200_DMA_RX].napi);

	/* shut down hardware */
	xrx200_hw_cleanup(&xrx200_hw);

	/* remove the actual device */
	unregister_netdev(dev);
	free_netdev(dev);

	return 0;
}

static const struct of_device_id xrx200_match[] = {
	{ .compatible = "lantiq,xrx200-net" },
	{},
};
MODULE_DEVICE_TABLE(of, xrx200_match);

static struct platform_driver xrx200_driver = {
	.probe = xrx200_probe,
	.remove = xrx200_remove,
	.driver = {
		.name = "lantiq,xrx200-net",
		.of_match_table = xrx200_match,
		.owner = THIS_MODULE,
	},
};

module_platform_driver(xrx200_driver);

MODULE_AUTHOR("John Crispin <blogic@openwrt.org>");
MODULE_DESCRIPTION("Lantiq SoC XRX200 ethernet");
MODULE_LICENSE("GPL");
