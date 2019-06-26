/*
 * Copyright (C) 2011-2014 Daniel Schwierzeck <daniel.schwierzeck@gmail.com>
 *
 * This program is free software; you can distribute it and/or modify it
 * under the terms of the GNU General Public License (Version 2) as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of_platform.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/gpio.h>
#include <linux/pm_runtime.h>
#include <linux/spi/spi.h>

#include <lantiq_soc.h>

/* USIF ID registers */
#define USIF_ID			0x0c	/* USIF identification */
#define USIF_FIFO_ID		0x10	/* FIFO identification */
#define USIF_SWCID		0x1c	/* software check identification */
/* USIF clock control registers */
#define USIF_CLC		0x0	/* clock control */
#define USIF_CLC_CNT		0x4	/* Clock control counter */
#define USIF_CLC_STAT		0x8	/* Clock control status */
/* Special function registers */
#define USIF_MODE_CFG		0x110	/* USIF mode configuration */
#define USIF_PRTC_CFG		0x114	/* protocol configuration */
#define USIF_PRTC_STAT		0x118	/* protocol status */
#define USIF_FRM_CTRL		0x11c	/* frame control */
#define USIF_FRM_STAT		0x120	/* frame status */
#define USIF_CS_CFG		0x130	/* chip select configuration */
#define USIF_FDIV_CFG		0x140	/* fractional divider config. */
#define USIF_BC_CFG		0x144	/* baudrate counter config. */
#define USIF_CS_TIM0_CFG	0x154	/* chip select timing config. 0 */
#define USIF_CS_TIM1_CFG	0x158	/* chip select timing config. 0 */
#define USIF_CS_TIM2_CFG	0x15c	/* chip select timing config. 0 */
/* FIFO registers */
#define USIF_DPLUS_CTRL		0x2c	/* DPLUS control */
#define USIF_FIFO_CFG		0x30	/* FIFO configuration */
#define USIF_FIFO_CTRL		0x34	/* FIFO control */
#define USIF_MRPS_CTRL		0x38	/* max. received packet size control */
#define USIF_FIFO_STAT		0x44	/* FIFO stages status */
#define USIF_TXD_SB		0x48	/* transmit data sideband */
#define USIF_DPLUS_STAT		0x4c	/* DPLUS status */
/* FIFO data registers */
#define USIF_TXD		0x40000	/* transmission data */
#define USIF_RXD		0x80000	/* reception data */
/* Interrupt and DMA registers */
#define USIF_RIS		0x80	/* raw interrupt status */
#define USIF_IMSC		0x84	/* interrupt mask control */
#define USIF_MIS		0x88	/* masked interrupt status */
#define USIF_ISR		0x90	/* interrupt set */
#define USIF_ICR		0x98	/* interrupt clear */

#define ID_TS_REV_NR_SHIFT	16	/* TOPSPIN revision number */
#define ID_TS_REV_NR_MASK	(0xffff << ID_TS_REV_NR_SHIFT)
#define ID_MOD_ID_SHIFT		8	/* module identification number */
#define ID_MOD_ID_MASK		(0xff << ID_MOD_ID_SHIFT)
#define ID_REV_NUMBER_MASK	0xff	/* module revision number */

#define FIFOID_RXSTAGE_SHIFT	8	/* number of RX FIFO stages */
#define FIFOID_RXSTAGE_MASK	(0xFF << FIFOID_RXSTAGE_SHIFT)
#define FIFOID_TXSTAGE_MASK	0xFF	/* number of TX FIFO stages */

#define SWCID_SSC		BIT(10)	/* USIF SSC support */
#define SWCID_CRC		BIT(7)	/* USIF HW CRC support */

#define FIFO_ID_RXSTAGE_SHIFT	8	/* RX FIFO stages */
#define FIFO_ID_RXSTAGE_MASK	(0xff << FIFO_ID_RXSTAGE_SHIFT)
#define FIFO_ID_TXSTAGE_MASK	0xff	/* TX FIFO stages */

#define CLC_MOD_EN_DIS		BIT(3)	/* module disable request */
#define CLC_MOD_EN_EN		BIT(2)	/* module enable request */
#define CLC_RUN_STOP		BIT(1)	/* enter configuration mode */
#define CLC_RUN_RUN		BIT(0)	/* enter run mode */

#define CLC_CNT_ORMC_SHIFT	8	/* clock divider for optional run mode */
#define CLC_CNT_ORMC_MASK	(0xFF << CLC_CNT_ORMC_SHIFT)
#define CLC_CNT_RMC_MASK	0xFF	/* clock divider for standard run mode */

#define CLC_STAT_FIFOID		BIT(8)	/* FIFO idle */
#define CLC_STAT_KID		BIT(7)	/* kernel idle */
#define CLC_STAT_CUOK		BIT(6)	/* counter update done */
#define CLC_STAT_MODEN		BIT(1)	/* module enable status */
#define CLC_STAT_RUN		BIT(0)	/* module run status */

#define MODE_CFG_TXEN		BIT(16)	/* transmission enable */
#define MODE_CFG_HDEN		BIT(15)	/* half duplex mode enable */
#define MODE_CFG_TX_IDLE	BIT(11)	/* TX idle state */
#define MODE_CFG_SCPH		BIT(10)	/* shift clock phase config. */
#define MODE_CFG_SCPOL		BIT(9)	/* shift clock polarity config. */
#define MODE_CFG_SCFRC		BIT(8)	/* shift clock force */
#define MODE_CFG_LB		BIT(4)	/* loopback enable */
#define MODE_CFG_FCEN		BIT(3)	/* framing control enable */
#define MODE_CFG_RXEN		BIT(2)	/* receive enable */
#define MODE_CFG_MA		BIT(1)	/* master/slave mode */
#define MODE_CFG_SYNC		BIT(0)	/* async/sync mode */

#define PRTC_CFG_HD		BIT(7)	/* heading of databits (LSB/MSB) */
#define PRTC_CFG_CLEN_MASK	0x1F	/* character length */

#define CS_CFG_CSOINV_SHIFT	24	/* chip select output inverter */
#define CS_CFG_CSOINV_MASK	(0xff << CS_CFG_CSOINV_SHIFT)
#define CS_CFG_CSOFRM_SHIFT	20	/* framing chip select output */
#define CS_CFG_CSOFRM_MASk	(0xf << CS_CFG_CSOFRM_SHIFT)
#define CS_CFG_CSOCLK_SHIFT	16	/* clocked chip select output */
#define CS_CFG_CSOCLK_MASk	(0xf << CS_CFG_CSOCLK_SHIFT)
#define CS_CFG_CSO2		BIT(10)	/* chip select output 2 */
#define CS_CFG_CSO1		BIT(9)	/* chip select output 1 */
#define CS_CFG_CSO0		BIT(8)	/* chip select output 0 */
#define CS_CFG_EACS		BIT(1)	/* auto chip select enable */
#define CS_CFG_CSEN		BIT(0)	/* chip select enable */

#define BC_CFG_SCDIV		BIT(24)
#define BC_CFG_BCRV_MASK	0x1fff

#define DPLUS_CTRL_TX_DPLUS_DIS	BIT(23)
#define DPLUS_CTRL_TX_DPLUS_EN	BIT(22)
#define DPLUS_CTRL_RX_MASK	(0xff << 8)
#define DPLUS_CTRL_SETMASK	BIT(7)

#define FIFO_CFG_TX_SWAP	BIT(15)	/* TX data swap */
#define FIFO_CFG_TXFA_SHIFT	12	/* TX FIFO alignment */
#define FIFO_CFG_TXFA_MASK	(0x7 << FIFO_CFG_TXFA_SHIFT)
#define FIFO_CFG_RX_SWAP	BIT(11)	/* RX data swap */
#define FIFO_CFG_RXFA_SHIFT	8	/* RX FIFO alignment */
#define FIFO_CFG_RXFA_MASK	(0x7 << FIFO_CFG_RXFA_SHIFT)
#define FIFO_CFG_TXFC		BIT(7)	/* TX FIFO flow control */
#define FIFO_CFG_TXBS_SHIFT	4	/* TX burst size */
#define FIFO_CFG_TXBS_MASK	(0x7 << FIFO_CFG_TXBS_SHIFT)
#define FIFO_CFG_RXFC		BIT(3)	/* RX FIFO flow control */
#define FIFO_CFG_RXBS_MASK	0x7	/* RX burst size */

#define FIFO_CTRL_RX_AR_DIS	BIT(7)	/* RX autoreceive disable */
#define FIFO_CTRL_RX_AR_EN	BIT(6)	/* RX autoreceive enable */
#define FIFO_CTRL_RX_ABORT	BIT(5)	/* abort RX FIFO */
#define FIFO_CTRL_RX_START	BIT(4)	/* start RX FIFO */
#define FIFO_CTRL_TX_SETEOP	BIT(2)	/* set end of packet */
#define FIFO_CTRL_TX_ABORT	BIT(1)	/* abort TX FIFO */
#define FIFO_CTRL_TX_START	BIT(0)	/* start TX FIFO */

#define FIFO_STAT_TXFFS_SHIFT	16	/* TX filled FIFO stages */
#define FIFO_STAT_TXFFS_MASK	(0xFF << FIFO_STAT_TXFFS_SHIFT)
#define FIFO_STAT_RXFFS_MASK	0xFF	/* RX filled FIFO stages */

#define TXD_SB_XME		BIT(7)
#define TXD_SB_TX_BE_SHIFT	4
#define TXD_SB_TX_BE_MASK	(0x3 << TXD_SB_TX_BE_SHIFT)

#define INT_TX_FIN		BIT(29)	/* transmit finished */
#define INT_TX_OF		BIT(14)	/* transmit FIFO over-flow */
#define INT_TX_UR		BIT(13)	/* transmit FIFO under-run */
#define INT_RX_OF		BIT(12)	/* transmit FIFO over-flow */
#define INT_RX_UR		BIT(11)	/* transmit FIFO under-run */

enum fifo_burst_size {
	FIFO_BURST_1 = 0,
	FIFO_BURST_2 = 1,
	FIFO_BURST_4 = 2,
	FIFO_BURST_8 = 3,
	FIFO_BURST_16 = 4,
};

enum fifo_align {
	FIFO_ALIGN_1 = 0,
	FIFO_ALIGN_2 = 1,
	FIFO_ALIGN_4 = 2,
};

struct lantiq_usif_hwcfg {
	unsigned int num_chipselect;
};

struct lantiq_usif {
	struct spi_master		*master;
	struct device			*dev;
	void __iomem			*regbase;
	struct clk			*usif_clk;
	struct pinctrl			*pinctrl;
	struct pinctrl_state		*clk_active;
	struct pinctrl_state		*clk_high;
	struct pinctrl_state		*clk_low;
	const struct lantiq_usif_hwcfg	*hwcfg;

	const u8			*tx;
	u8				*rx;
	unsigned int			tx_todo;
	unsigned int			rx_todo;
	unsigned int			rx_req;
	u8				bits_per_word;
	int				status;
};

struct lantiq_usif_cstate {
	int cs_gpio;
};

static u32 lantiq_usif_readl(const struct lantiq_usif *spi, u32 reg)
{
	return readl_be(spi->regbase + reg);
}

static void lantiq_usif_writel(const struct lantiq_usif *spi, u32 val, u32 reg)
{
	writel_be(val, spi->regbase + reg);
}

static unsigned int tx_fifo_size(const struct lantiq_usif *spi)
{
	u32 fifo_id = lantiq_usif_readl(spi, USIF_FIFO_ID);
	return fifo_id & FIFO_ID_TXSTAGE_MASK;
}

static unsigned int rx_fifo_size(const struct lantiq_usif *spi)
{
	u32 fifo_id = lantiq_usif_readl(spi, USIF_FIFO_ID);
	return (fifo_id & FIFO_ID_RXSTAGE_MASK) >> FIFO_ID_RXSTAGE_SHIFT;
}

static unsigned int tx_fifo_level(const struct lantiq_usif *spi)
{
	u32 fifo_stat = lantiq_usif_readl(spi, USIF_FIFO_STAT);
	return (fifo_stat & FIFO_STAT_TXFFS_MASK) >> FIFO_STAT_TXFFS_SHIFT;
}

static unsigned int rx_fifo_level(const struct lantiq_usif *spi)
{
	u32 fifo_stat = lantiq_usif_readl(spi, USIF_FIFO_STAT);
	return fifo_stat & FIFO_STAT_RXFFS_MASK;
}

static unsigned int tx_fifo_free(const struct lantiq_usif *spi)
{
	return tx_fifo_size(spi) - tx_fifo_level(spi);
}

static int hw_is_in_run_mode(const struct lantiq_usif *spi)
{
	u32 stat = lantiq_usif_readl(spi, USIF_CLC_STAT);
	return stat & CLC_STAT_RUN;
}

static int hw_tx_finished(const struct lantiq_usif *spi)
{
	u32 ris = lantiq_usif_readl(spi, USIF_RIS);
	return ris & INT_TX_FIN;
}

static void hw_clear_tx_fin(const struct lantiq_usif *spi)
{
	lantiq_usif_writel(spi, INT_TX_FIN, USIF_ICR);
}

static int hw_set_config_mode(const struct lantiq_usif *spi)
{
	int i;
	u32 clc_stat;
	const u32 update_done = CLC_STAT_MODEN | CLC_STAT_CUOK | CLC_STAT_KID |
				CLC_STAT_FIFOID;

	if (!hw_is_in_run_mode(spi))
		return 0;

	/*
	 * Hardware limitation: writes to USIF kernel config registers needs a
	 * delay to safely update their values and before module is switched
	 * to run mode. Read back the MODE_CFG register as recommended
	 * in errata sheet.
	 */
	lantiq_usif_readl(spi, USIF_MODE_CFG);
	rmb();
	lantiq_usif_writel(spi, CLC_RUN_STOP, USIF_CLC);

	for (i = 0; i < 1000; i++) {
		clc_stat = lantiq_usif_readl(spi, USIF_CLC_STAT);
		if ((clc_stat & update_done) == update_done)
			return 0;
	}

	dev_err(spi->dev, "timeout on setting config mode\n");

	return -EIO;
}

static int hw_set_run_mode(const struct lantiq_usif *spi)
{
	int i;
	u32 clc_stat;
	const u32 update_done = CLC_STAT_MODEN | CLC_STAT_CUOK | CLC_STAT_KID |
				CLC_STAT_FIFOID | CLC_STAT_RUN;

	if (hw_is_in_run_mode(spi))
		return 0;

	/*
	 * Hardware limitation: writes to USIF kernel config registers needs a
	 * delay to safely update their values and before module is switched
	 * to run mode. Read back the MODE_CFG register as recommended
	 * in errata sheet.
	 */
	lantiq_usif_readl(spi, USIF_MODE_CFG);
	rmb();
	lantiq_usif_writel(spi, CLC_RUN_RUN, USIF_CLC);

	for (i = 0; i < 1000; i++) {
		clc_stat = lantiq_usif_readl(spi, USIF_CLC_STAT);
		if ((clc_stat & update_done) == update_done)
			return 0;
	}

	dev_err(spi->dev, "timeout on setting run mode\n");

	return -EIO;
}

static void hw_setup_speed_hz(const struct lantiq_usif *spi,
				unsigned int max_speed_hz)
{
	u32 bcrv;
	const u32 usif_clk = 100000000;

	/*
	 * USIF kernel clock is always 100 MHz. The fractional divider is
	 * disabled. d is 2 or 4 dependent on bit BC_CFG.SCDIV.
	 *
	 *              f_kernel
	 * f_shift = ----------------
	 *            d * (BCRV + 1)
	 */
	if (max_speed_hz > usif_clk / 4) {
		bcrv = usif_clk / (max_speed_hz * 2) - 1;
		bcrv |= BC_CFG_SCDIV;
	} else {
		bcrv = usif_clk / (max_speed_hz * 4) - 1;
	}

	if (bcrv > BC_CFG_BCRV_MASK)
		bcrv = BC_CFG_BCRV_MASK;

	lantiq_usif_writel(spi, bcrv, USIF_BC_CFG);
}

static void hw_setup_bits_per_word(const struct lantiq_usif *spi,
					unsigned int bits_per_word)
{
	u32 prtc_cfg = lantiq_usif_readl(spi, USIF_PRTC_CFG);
	prtc_cfg &= ~PRTC_CFG_CLEN_MASK;
	prtc_cfg |= bits_per_word;
	lantiq_usif_writel(spi, prtc_cfg, USIF_PRTC_CFG);
}

static void hw_setup_clock_mode(const struct lantiq_usif *spi,
				struct spi_device *sdev,
				struct spi_transfer *t)
{
	u32 mode_cfg = lantiq_usif_readl(spi, USIF_MODE_CFG);
	u32 prtc_cfg = lantiq_usif_readl(spi, USIF_PRTC_CFG);
	u32 fifo_cfg = lantiq_usif_readl(spi, USIF_FIFO_CFG);

	/*
	 * SPI mode mapping in MODE_CFG register:
	 * Mode CPOL CPHA SCPOL SCPH
	 *  0    0    0    0     1
	 *  1    0    1    0     0
	 *  2    1    0    1     1
	 *  3    1    1    1     0
	 */
	if (sdev->mode & SPI_CPHA)
		mode_cfg &= ~MODE_CFG_SCPH;
	else
		mode_cfg |= MODE_CFG_SCPH;

	if (sdev->mode & SPI_CPOL)
		mode_cfg |= MODE_CFG_SCPOL;
	else
		mode_cfg &= ~MODE_CFG_SCPOL;

	/* Set loopback mode */
	if (sdev->mode & SPI_LOOP)
		mode_cfg |= MODE_CFG_LB;
	else
		mode_cfg &= ~MODE_CFG_LB;

	/* Set heading control */
	if (sdev->mode & SPI_LSB_FIRST)
		prtc_cfg &= ~PRTC_CFG_HD;
	else
		prtc_cfg |= PRTC_CFG_HD;

	/* Enable data transmission */
	if (t->tx_buf)
		mode_cfg |= MODE_CFG_TXEN;
	else
		mode_cfg &= ~MODE_CFG_TXEN;

	/* Enable data reception */
	if (t->rx_buf) {
		mode_cfg |= MODE_CFG_RXEN;
	} else
		mode_cfg &= ~MODE_CFG_RXEN;

	/* Enable forced shift clock and HW flow control in RX-only mode */
	if (t->rx_buf && !t->tx_buf) {
		mode_cfg |= MODE_CFG_SCFRC;
		fifo_cfg |= FIFO_CFG_RXFC;
	} else {
		mode_cfg &= ~MODE_CFG_SCFRC;
		fifo_cfg &= ~FIFO_CFG_RXFC;
	}

	lantiq_usif_writel(spi, mode_cfg, USIF_MODE_CFG);
	lantiq_usif_writel(spi, prtc_cfg, USIF_PRTC_CFG);
	lantiq_usif_writel(spi, fifo_cfg, USIF_FIFO_CFG);
}

static void hw_init(const struct lantiq_usif *spi)
{
	/*
	 * Set clock dividers for standard run mode and optional run mode to 1
	 * to run at the same frequency as FPI bus
	 */
	lantiq_usif_writel(spi, (1 << CLC_CNT_ORMC_SHIFT) | 1, USIF_CLC_CNT);

	/* Set module enable request */
	lantiq_usif_writel(spi, CLC_MOD_EN_EN, USIF_CLC);
	hw_set_config_mode(spi);

	/* Disable and clear all interrupts */
	lantiq_usif_writel(spi, 0, USIF_IMSC);
	lantiq_usif_writel(spi, 0xFFFFFFFF, USIF_ICR);

	/*
	 * Enable RXD/TXD swapping, set FIFO alignment to 1 Bytes,
	 * enable flow control
	 */
	lantiq_usif_writel(spi, (FIFO_ALIGN_4 << FIFO_CFG_TXFA_SHIFT) |
		(FIFO_ALIGN_4 << FIFO_CFG_RXFA_SHIFT) |
		(FIFO_BURST_4 << FIFO_CFG_TXBS_SHIFT) |
		FIFO_BURST_4, USIF_FIFO_CFG);

	/* Set master mode, sync mode, bits per word to 8, MSB first */
	lantiq_usif_writel(spi, MODE_CFG_MA | MODE_CFG_SYNC, USIF_MODE_CFG);
	lantiq_usif_writel(spi, PRTC_CFG_HD | 8, USIF_PRTC_CFG);

	/* Disable fractional divider */
	lantiq_usif_writel(spi, 0, USIF_FDIV_CFG);

	/* Reset chip select registers */
	lantiq_usif_writel(spi, 0, USIF_CS_CFG);

	/* Disable DPLUS TX port */
	lantiq_usif_writel(spi, DPLUS_CTRL_TX_DPLUS_DIS | DPLUS_CTRL_RX_MASK |
		DPLUS_CTRL_SETMASK, USIF_DPLUS_CTRL);
}

static void chipselect_enable(struct spi_device *sdev)
{
	struct lantiq_usif_cstate *cstate = spi_get_ctldata(sdev);

	gpio_set_value(cstate->cs_gpio, sdev->mode & SPI_CS_HIGH);
}

static void chipselect_disable(struct spi_device *sdev)
{
	struct lantiq_usif_cstate *cstate = spi_get_ctldata(sdev);

	gpio_set_value(cstate->cs_gpio, !(sdev->mode & SPI_CS_HIGH));
}

static int lantiq_usif_setup(struct spi_device *sdev)
{
	struct lantiq_usif *spi = spi_master_get_devdata(sdev->master);
	struct lantiq_usif_cstate *cstate = spi_get_ctldata(sdev);
	int err;

	switch (sdev->bits_per_word) {
	case 8:
	case 16:
	case 32:
		break;
	default:
		dev_err(spi->dev, "bits_per_word 8|16|32 supported only\n");
		return -EINVAL;
	}

	if (sdev->max_speed_hz > 50000000) {
		dev_err(spi->dev, "max. 50 MHz supported only\n");
		return -EINVAL;
	}

	if (!cstate) {
		cstate = kzalloc(sizeof(*cstate), GFP_KERNEL);
		if (!cstate)
			return -ENOMEM;

		spi_set_ctldata(sdev, cstate);
		cstate->cs_gpio = -ENOENT;

		if (sdev->cs_gpio < 0) {
			dev_err(spi->dev, "no gpio config for chip select %u\n",
				sdev->chip_select);
			return -EINVAL;
		}

		dev_dbg(spi->dev, "using chipselect %u on GPIO %d\n",
			sdev->chip_select, sdev->cs_gpio);

		err = gpio_request(sdev->cs_gpio, dev_name(spi->dev));
		if (err)
			return err;

		gpio_direction_output(sdev->cs_gpio,
			!(sdev->mode & SPI_CS_HIGH));

		cstate->cs_gpio = sdev->cs_gpio;
	}

	return 0;
}

static void lantiq_usif_cleanup(struct spi_device *sdev)
{
	struct lantiq_usif_cstate *cstate = spi_get_ctldata(sdev);

	if (cstate) {
		gpio_free(cstate->cs_gpio);
		kfree(cstate);
		spi_set_ctldata(sdev, NULL);
	}
}

static int hw_setup_clk_active(struct lantiq_usif *spi)
{
	return pinctrl_select_state(spi->pinctrl, spi->clk_active);
}

static int hw_setup_clk_drive(struct lantiq_usif *spi, int state)
{
	if (state)
		return pinctrl_select_state(spi->pinctrl, spi->clk_high);
	else
		return pinctrl_select_state(spi->pinctrl, spi->clk_low);
}

static int hw_setup_transfer(struct lantiq_usif *spi,
				struct spi_device *sdev,
				struct spi_transfer *t)
{
	u8 bits_per_word;
	u32 speed_hz;
	int err;

	if (t->bits_per_word)
		bits_per_word = t->bits_per_word;
	else
		bits_per_word = sdev->bits_per_word;

	if (t->speed_hz)
		speed_hz = t->speed_hz;
	else
		speed_hz = sdev->max_speed_hz;

	err = hw_setup_clk_drive(spi, sdev->mode & SPI_CPOL);
	if (err)
		return err;

	err = hw_set_config_mode(spi);
	if (err)
		return err;

	hw_setup_clock_mode(spi, sdev, t);
	hw_setup_bits_per_word(spi, bits_per_word);
	hw_setup_speed_hz(spi, speed_hz);

	spi->bits_per_word = bits_per_word;

	err = hw_set_run_mode(spi);
	if (err)
		return err;

	return hw_setup_clk_active(spi);
}

static void hw_finish_message(struct lantiq_usif *spi)
{
	hw_setup_clk_drive(spi, 0);
	hw_set_config_mode(spi);
}

static void tx_fifo_write(struct lantiq_usif *spi)
{
	const u8 *tx8 = spi->tx;
	const u16 *tx16 = (const u16 *)spi->tx;
	const u32 *tx32 = (const u32 *)spi->tx;
	u32 txd, txd_sb = 0;
	unsigned int bytes = spi->bits_per_word / 8;

	switch (spi->bits_per_word) {
	case 8:
		tx8 = spi->tx;
		txd = *tx8;
		break;
	case 16:
		tx16 = (u16 *)spi->tx;
		txd = *tx16;
		break;
	case 32:
		tx32 = (u32 *) spi->tx;
		txd = *tx32;
		break;
	}

	spi->tx_todo -= bytes;
	spi->tx += bytes;

	/* mark end of transmit message */
	if (!spi->tx_todo)
		txd_sb |= TXD_SB_XME;

	lantiq_usif_writel(spi, txd_sb, USIF_TXD_SB);
	lantiq_usif_writel(spi, txd, USIF_TXD);
}

static void rx_fifo_read(struct lantiq_usif *spi)
{
	u8 *rx8 = spi->rx;
	u16 *rx16 = (u16 *)spi->rx;
	u32 *rx32 = (u32 *)spi->rx;
	u32 rxd;
	unsigned int bytes = spi->bits_per_word / 8;

	rxd = lantiq_usif_readl(spi, USIF_RXD);

	switch (spi->bits_per_word) {
	case 8:
		rx8 = spi->rx;
		*rx8 = rxd;
		break;
	case 16:
		rx16 = (u16 *) spi->rx;
		*rx16 = rxd;
		break;
	case 32:
		rx32 = (u32 *) spi->rx;
		*rx32 = rxd;
		break;
	}

	spi->rx_todo -= bytes;
	spi->rx += bytes;

	if (spi->rx_req)
		spi->rx_req -= bytes;
}

static void rx_request(struct lantiq_usif *spi)
{
	spi->rx_req = min(spi->rx_todo, rx_fifo_size(spi));

	lantiq_usif_writel(spi, spi->rx_req, USIF_MRPS_CTRL);
}

static int tx_finished_wait(const struct lantiq_usif *spi)
{
	unsigned long deadline = jiffies + msecs_to_jiffies(500);

	do {
		if (hw_tx_finished(spi)) {
			hw_clear_tx_fin(spi);
			return 0;
		}
		cond_resched();
	} while (!time_after_eq(jiffies, deadline));

	dev_err(spi->dev, "timeout waiting for TX finish\n");

	return -ETIMEDOUT;
}

static int tx_wait(const struct lantiq_usif *spi)
{
	unsigned long deadline = jiffies + msecs_to_jiffies(200);
	int ret;

	do {
		ret = tx_fifo_free(spi);
		if (ret > 0)
			return ret;
		cond_resched();
	} while (!time_after_eq(jiffies, deadline));

	dev_err(spi->dev, "timeout waiting for TX\n");

	return -ETIMEDOUT;
}

static int rx_wait(const struct lantiq_usif *spi)
{
	unsigned long deadline = jiffies + msecs_to_jiffies(200);
	int ret;

	do {
		ret = rx_fifo_level(spi);
		if (ret > 0)
			return ret;
		cond_resched();
	} while (!time_after_eq(jiffies, deadline));

	dev_err(spi->dev, "timeout waiting for RX\n");

	return -ETIMEDOUT;
}

static int transfer_pio_readwrite(struct lantiq_usif *spi, struct spi_transfer *t)
{
	int tx_free, rx_fill;

	while (spi->tx_todo) {
		tx_free = tx_wait(spi);
		if (tx_free < 0)
			return tx_free;

		tx_fifo_write(spi);

		if (spi->rx_todo) {
			rx_fill = rx_wait(spi);
			if (rx_fill < 0)
				return rx_fill;

			rx_fifo_read(spi);
		}
	}

	return tx_finished_wait(spi);
}

static int transfer_pio_readonly(struct lantiq_usif *spi, struct spi_transfer *t)
{
	int rx_fill;

	while (spi->rx_todo) {
		if (!spi->rx_req)
			rx_request(spi);

		rx_fill = rx_wait(spi);
		if (rx_fill < 0)
			return rx_fill;

		if (rx_fill > 0)
			rx_fifo_read(spi);
	}

	return 0;
}

static int lantiq_usif_transfer_one_message(struct spi_master *master,
						struct spi_message *msg)
{
	struct lantiq_usif *spi = spi_master_get_devdata(master);
	struct spi_device *spidev = msg->spi;
	struct spi_transfer *t;
	int status;
	unsigned int cs_change = 1;

	list_for_each_entry(t, &msg->transfers, transfer_list) {
		status = hw_setup_transfer(spi, spidev, t);
		if (status)
			goto done;

		if (cs_change)
			chipselect_enable(spidev);
		cs_change = t->cs_change;

		spi->tx = t->tx_buf;
		spi->rx = t->rx_buf;
		spi->status = -EINPROGRESS;

		if (spi->rx && !spi->tx) {
			spi->rx_todo = t->len;
			spi->rx_req = 0;
			status = transfer_pio_readonly(spi, t);
		} else {
			spi->tx_todo = t->len;
			if (spi->rx)
				spi->rx_todo = t->len;

			status = transfer_pio_readwrite(spi, t);
		}
		if (status)
			goto done;

		msg->actual_length += t->len;

		if (t->delay_usecs)
			udelay(t->delay_usecs);

		if (cs_change)
			chipselect_disable(spidev);
	}

done:
	msg->status = status;

	if (!(status == 0 && cs_change))
		chipselect_disable(spidev);

	spi_finalize_current_message(master);
	hw_finish_message(spi);

	return status;
}

static int lantiq_usif_prepare_transfer(struct spi_master *master)
{
	struct lantiq_usif *spi = spi_master_get_devdata(master);

	pm_runtime_get_sync(spi->dev);

	return 0;
}

static int lantiq_usif_unprepare_transfer(struct spi_master *master)
{
	struct lantiq_usif *spi = spi_master_get_devdata(master);

	pm_runtime_put(spi->dev);

	return 0;
}

static const struct lantiq_usif_hwcfg usif_xrx200 = {
	.num_chipselect = 3,
};

static const struct lantiq_usif_hwcfg usif_xrx300 = {
	.num_chipselect = 1,
};

static const struct of_device_id lantiq_usif_match[] = {
	{ .compatible = "lantiq,xrx200-usif-spi", .data = &usif_xrx200, },
	{ .compatible = "lantiq,xrx300-usif-spi", .data = &usif_xrx300, },
	{ .compatible = "lantiq,xrx330-usif-spi", .data = &usif_xrx300, },
	{},
};
MODULE_DEVICE_TABLE(of, lantiq_usif_match);

static int lantiq_usif_probe(struct platform_device *pdev)
{
	struct spi_master *master;
	struct resource *res;
	struct lantiq_usif *spi;
	const struct lantiq_usif_hwcfg *hwcfg;
	const struct of_device_id *match;
	int err;

	match = of_match_device(lantiq_usif_match, &pdev->dev);
	if (!match) {
		dev_err(&pdev->dev, "no device match\n");
		return -EINVAL;
	}
	hwcfg = match->data;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_err(&pdev->dev, "failed to get resources\n");
		return -ENXIO;
	}

	master = spi_alloc_master(&pdev->dev, sizeof(struct lantiq_usif));
	if (!master)
		return -ENOMEM;

	spi = spi_master_get_devdata(master);
	spi->master = master;
	spi->dev = &pdev->dev;
	spi->hwcfg = hwcfg;
	platform_set_drvdata(pdev, spi);

	spi->regbase = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(spi->regbase)) {
		err = PTR_ERR(spi->regbase);
		goto err_master_put;
	}

	spi->pinctrl = devm_pinctrl_get(&pdev->dev);
	if (IS_ERR(spi->pinctrl)) {
		err = PTR_ERR(spi->pinctrl);
		goto err_master_put;
	}

	spi->clk_active = pinctrl_lookup_state(spi->pinctrl, "clk active");
	if (IS_ERR(spi->clk_active)) {
		err = PTR_ERR(spi->clk_active);
		goto err_master_put;
	}

	spi->clk_high = pinctrl_lookup_state(spi->pinctrl, "clk high");
	if (IS_ERR(spi->clk_high)) {
		err = PTR_ERR(spi->clk_high);
		goto err_master_put;
	}

	spi->clk_low = pinctrl_lookup_state(spi->pinctrl, "clk low");
	if (IS_ERR(spi->clk_low)) {
		err = PTR_ERR(spi->clk_low);
		goto err_master_put;
	}

	spi->usif_clk = clk_get(&pdev->dev, NULL);
	if (IS_ERR(spi->usif_clk)) {
		err = PTR_ERR(spi->usif_clk);
		goto err_master_put;
	}
	clk_prepare_enable(spi->usif_clk);

	master->dev.of_node = pdev->dev.of_node;
	master->bus_num = 1;
	master->num_chipselect = hwcfg->num_chipselect;
	master->setup = lantiq_usif_setup;
	master->cleanup = lantiq_usif_cleanup;
	master->prepare_transfer_hardware = lantiq_usif_prepare_transfer;
	master->transfer_one_message = lantiq_usif_transfer_one_message;
	master->unprepare_transfer_hardware = lantiq_usif_unprepare_transfer;
	master->mode_bits = SPI_CPOL | SPI_CPHA | SPI_LSB_FIRST | SPI_CS_HIGH |
				SPI_LOOP;

	hw_init(spi);

	err = spi_register_master(master);
	if (err) {
		dev_err(&pdev->dev, "failed to register spi_master\n");
		goto err_clk_disable;
	}

	dev_info(&pdev->dev, "init ok\n");

	return 0;

err_clk_disable:
	clk_disable_unprepare(spi->usif_clk);
	clk_put(spi->usif_clk);
err_master_put:
	platform_set_drvdata(pdev, NULL);
	spi_master_put(master);

	return err;
}

static int lantiq_usif_remove(struct platform_device *pdev)
{
	struct lantiq_usif *spi = platform_get_drvdata(pdev);
	struct spi_master *master = spi->master;

	spi_unregister_master(master);

	clk_disable_unprepare(spi->usif_clk);
	clk_put(spi->usif_clk);

	platform_set_drvdata(pdev, NULL);
	spi_master_put(master);

	return 0;
}

static struct platform_driver lantiq_usif_driver = {
	.probe = lantiq_usif_probe,
	.remove = lantiq_usif_remove,
	.driver = {
		.name = "spi-lantiq-usif",
		.owner = THIS_MODULE,
		.of_match_table = lantiq_usif_match,
	},
};
module_platform_driver(lantiq_usif_driver);

MODULE_DESCRIPTION("Lantiq USIF SPI controller driver");
MODULE_AUTHOR("Daniel Schwierzeck <daniel.schwierzeck@gmail.com>");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:spi-lantiq-usif");
