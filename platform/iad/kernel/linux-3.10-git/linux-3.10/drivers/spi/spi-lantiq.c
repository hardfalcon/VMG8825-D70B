/*
 * Copyright (C) 2011-2014 Daniel Schwierzeck <daniel.schwierzeck@gmail.com>
 *
 * This program is free software; you can distribute it and/or modify it
 * under the terms of the GNU General Public License (Version 2) as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of_device.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/completion.h>
#include <linux/spinlock.h>
#include <linux/err.h>
#include <linux/gpio.h>
#include <linux/pm_runtime.h>
#include <linux/spi/spi.h>

#include <lantiq_soc.h>

#define SPI_RX_IRQ_NAME		"spi_rx"
#define SPI_TX_IRQ_NAME		"spi_tx"
#define SPI_ERR_IRQ_NAME	"spi_err"
#define SPI_FRM_IRQ_NAME	"spi_frm"

#define SPI_CLC			0x00
#define SPI_PISEL		0x04
#define SPI_ID			0x08
#define SPI_CON			0x10
#define SPI_STAT		0x14
#define SPI_WHBSTATE		0x18
#define SPI_TB			0x20
#define SPI_RB			0x24
#define SPI_RXFCON		0x30
#define SPI_TXFCON		0x34
#define SPI_FSTAT		0x38
#define SPI_BRT			0x40
#define SPI_BRSTAT		0x44
#define SPI_SFCON		0x60
#define SPI_SFSTAT		0x64
#define SPI_GPOCON		0x70
#define SPI_GPOSTAT		0x74
#define SPI_FPGO		0x78
#define SPI_RXREQ		0x80
#define SPI_RXCNT		0x84
#define SPI_DMACON		0xec
#define SPI_IRNEN		0xf4
#define SPI_IRNICR		0xf8
#define SPI_IRNCR		0xfc

#define SPI_CLC_SMC_S		16	/* Clock divider for sleep mode */
#define SPI_CLC_SMC_M		(0xFF << SPI_CLC_SMC_S)
#define SPI_CLC_RMC_S		8	/* Clock divider for normal run mode */
#define SPI_CLC_RMC_M		(0xFF << SPI_CLC_RMC_S)
#define SPI_CLC_DISS		BIT(1)	/* Disable status bit */
#define SPI_CLC_DISR		BIT(0)	/* Disable request bit */

#define SPI_ID_TXFS_S		24	/* Implemented TX FIFO size */
#define SPI_ID_TXFS_M		(0x3F << SPI_ID_TXFS_S)
#define SPI_ID_RXFS_S		16	/* Implemented RX FIFO size */
#define SPI_ID_RXFS_M		(0x3F << SPI_ID_RXFS_S)
#define SPI_ID_REV_M		0x1F	/* Hardware revision number */
#define SPI_ID_CFG		BIT(5)	/* DMA interface support */

#define SPI_CON_BM_S		16	/* Data width selection */
#define SPI_CON_BM_M		(0x1F << SPI_CON_BM_S)
#define SPI_CON_EM		BIT(24)	/* Echo mode */
#define SPI_CON_IDLE		BIT(23)	/* Idle bit value */
#define SPI_CON_ENBV		BIT(22)	/* Enable byte valid control */
#define SPI_CON_RUEN		BIT(12)	/* Receive underflow error enable */
#define SPI_CON_TUEN		BIT(11)	/* Transmit underflow error enable */
#define SPI_CON_AEN		BIT(10)	/* Abort error enable */
#define SPI_CON_REN		BIT(9)	/* Receive overflow error enable */
#define SPI_CON_TEN		BIT(8)	/* Transmit overflow error enable */
#define SPI_CON_LB		BIT(7)	/* Loopback control */
#define SPI_CON_PO		BIT(6)	/* Clock polarity control */
#define SPI_CON_PH		BIT(5)	/* Clock phase control */
#define SPI_CON_HB		BIT(4)	/* Heading control */
#define SPI_CON_RXOFF		BIT(1)	/* Switch receiver off */
#define SPI_CON_TXOFF		BIT(0)	/* Switch transmitter off */

#define SPI_STAT_RXBV_S		28
#define SPI_STAT_RXBV_M		(0x7 << SPI_STAT_RXBV_S)
#define SPI_STAT_BSY		BIT(13)	/* Busy flag */
#define SPI_STAT_RUE		BIT(12)	/* Receive underflow error flag */
#define SPI_STAT_TUE		BIT(11)	/* Transmit underflow error flag */
#define SPI_STAT_AE		BIT(10)	/* Abort error flag */
#define SPI_STAT_RE		BIT(9)	/* Receive error flag */
#define SPI_STAT_TE		BIT(8)	/* Transmit error flag */
#define SPI_STAT_ME		BIT(7)	/* Mode error flag */
#define SPI_STAT_MS		BIT(1)	/* Master/slave select bit */
#define SPI_STAT_EN		BIT(0)	/* Enable bit */

#define SPI_WHBSTATE_SETTUE	BIT(15)	/* Set transmit underflow error flag */
#define SPI_WHBSTATE_SETAE	BIT(14)	/* Set abort error flag */
#define SPI_WHBSTATE_SETRE	BIT(13)	/* Set receive error flag */
#define SPI_WHBSTATE_SETTE	BIT(12)	/* Set transmit error flag */
#define SPI_WHBSTATE_CLRTUE	BIT(11)	/* Clear transmit underflow error flag */
#define SPI_WHBSTATE_CLRAE	BIT(10)	/* Clear abort error flag */
#define SPI_WHBSTATE_CLRRE	BIT(9)	/* Clear receive error flag */
#define SPI_WHBSTATE_CLRTE	BIT(8)	/* Clear transmit error flag */
#define SPI_WHBSTATE_SETME	BIT(7)	/* Set mode error flag */
#define SPI_WHBSTATE_CLRME	BIT(6)	/* Clear mode error flag */
#define SPI_WHBSTATE_SETRUE	BIT(5)	/* Set receive underflow error flag */
#define SPI_WHBSTATE_CLRRUE	BIT(4)	/* Clear receive underflow error flag */
#define SPI_WHBSTATE_SETMS	BIT(3)	/* Set master select bit */
#define SPI_WHBSTATE_CLRMS	BIT(2)	/* Clear master select bit */
#define SPI_WHBSTATE_SETEN	BIT(1)	/* Set enable bit (operational mode) */
#define SPI_WHBSTATE_CLREN	BIT(0)	/* Clear enable bit (config mode */
#define SPI_WHBSTATE_CLR_ERRORS	0x0F50

#define SPI_RXFCON_RXFITL_S	8	/* FIFO interrupt trigger level */
#define SPI_RXFCON_RXFITL_M	(0x3F << SPI_RXFCON_RXFITL_S)
#define SPI_RXFCON_RXFLU	BIT(1)	/* FIFO flush */
#define SPI_RXFCON_RXFEN	BIT(0)	/* FIFO enable */

#define SPI_TXFCON_TXFITL_S	8	/* FIFO interrupt trigger level */
#define SPI_TXFCON_TXFITL_M	(0x3F << SPI_TXFCON_TXFITL_S)
#define SPI_TXFCON_TXFLU	BIT(1)	/* FIFO flush */
#define SPI_TXFCON_TXFEN	BIT(0)	/* FIFO enable */

#define SPI_FSTAT_RXFFL_S	0
#define SPI_FSTAT_RXFFL_M	(0x3f << SPI_FSTAT_RXFFL_S)
#define SPI_FSTAT_TXFFL_S	8
#define SPI_FSTAT_TXFFL_M	(0x3f << SPI_FSTAT_TXFFL_S)

#define SPI_GPOCON_ISCSBN_S	8
#define SPI_GPOCON_INVOUTN_S	0

#define SPI_FGPO_SETOUTN_S	8
#define SPI_FGPO_CLROUTN_S	0

#define SPI_RXREQ_RXCNT_M	0xFFFF	/* Receive count value */
#define SPI_RXCNT_TODO_M	0xFFFF	/* Recevie to-do value */

#define SPI_IRNEN_TFI		BIT(4)	/* TX finished interrupt */
#define SPI_IRNEN_F		BIT(3)	/* Frame end interrupt request */
#define SPI_IRNEN_E		BIT(2)	/* Error end interrupt request */
#define SPI_IRNEN_T_XWAY	BIT(1)	/* Transmit end interrupt request */
#define SPI_IRNEN_R_XWAY	BIT(0)	/* Receive end interrupt request */
#define SPI_IRNEN_R_XRX		BIT(1)	/* Transmit end interrupt request */
#define SPI_IRNEN_T_XRX		BIT(0)	/* Receive end interrupt request */
#define SPI_IRNEN_ALL		0x1F

struct lantiq_spi_hwcfg {
	unsigned int num_chipselect;
	unsigned int irnen_r;
	unsigned int irnen_t;
};

struct lantiq_spi {
	struct spi_master		*master;
	struct device			*dev;
	void __iomem			*regbase;
	struct clk			*spi_clk;
	struct clk			*fpi_clk;
	const struct lantiq_spi_hwcfg	*hwcfg;

	spinlock_t			lock;
	struct completion		xfer_complete;

	const u8			*tx;
	u8				*rx;
	unsigned int			tx_todo;
	unsigned int			rx_todo;
	unsigned int			bits_per_word;
	unsigned int			speed_hz;
	int				status;
	unsigned long			timeout;
	unsigned int			cs_delay;
};

struct lantiq_spi_cstate {
	int cs_gpio;
};

static u32 lantiq_spi_readl(const struct lantiq_spi *spi, u32 reg)
{
	return readl_be(spi->regbase + reg);
}

static void lantiq_spi_writel(const struct lantiq_spi *spi, u32 val, u32 reg)
{
	writel_be(val, spi->regbase + reg);
}

static void lantiq_spi_maskl(const struct lantiq_spi *spi, u32 clr, u32 set,
				u32 reg)
{
	u32 val = readl_be(spi->regbase + reg);
	val &= ~clr;
	val |= set;
	writel_be(val, spi->regbase + reg);
}

static int supports_dma(const struct lantiq_spi *spi)
{
	u32 id = lantiq_spi_readl(spi, SPI_ID);
	return id & SPI_ID_CFG;
}

static unsigned int tx_fifo_size(const struct lantiq_spi *spi)
{
	u32 id = lantiq_spi_readl(spi, SPI_ID);
	return (id & SPI_ID_TXFS_M) >> SPI_ID_TXFS_S;
}

static unsigned int rx_fifo_size(const struct lantiq_spi *spi)
{
	u32 id = lantiq_spi_readl(spi, SPI_ID);
	return (id & SPI_ID_RXFS_M) >> SPI_ID_RXFS_S;
}

static unsigned int tx_fifo_level(const struct lantiq_spi *spi)
{
	u32 fstat = lantiq_spi_readl(spi, SPI_FSTAT);
	return (fstat & SPI_FSTAT_TXFFL_M) >> SPI_FSTAT_TXFFL_S;
}

static unsigned int rx_fifo_level(const struct lantiq_spi *spi)
{
	u32 fstat = lantiq_spi_readl(spi, SPI_FSTAT);
	return fstat & SPI_FSTAT_RXFFL_M;
}

static unsigned int tx_fifo_free(const struct lantiq_spi *spi)
{
	return tx_fifo_size(spi) - tx_fifo_level(spi);
}

static void rx_fifo_reset(const struct lantiq_spi *spi)
{
	u32 val = rx_fifo_size(spi) << SPI_RXFCON_RXFITL_S;
	val |= SPI_RXFCON_RXFEN | SPI_RXFCON_RXFLU;
	lantiq_spi_writel(spi, val, SPI_RXFCON);
}

static void tx_fifo_reset(const struct lantiq_spi *spi)
{
	u32 val = 1 << SPI_TXFCON_TXFITL_S;
	val |= SPI_TXFCON_TXFEN | SPI_TXFCON_TXFLU;
	lantiq_spi_writel(spi, val, SPI_TXFCON);
}

static void rx_fifo_flush(const struct lantiq_spi *spi)
{
	lantiq_spi_maskl(spi, 0, SPI_RXFCON_RXFLU, SPI_RXFCON);
}

static void tx_fifo_flush(const struct lantiq_spi *spi)
{
	lantiq_spi_maskl(spi, 0, SPI_TXFCON_TXFLU, SPI_TXFCON);
}

static int hw_is_busy(const struct lantiq_spi *spi)
{
	u32 stat = lantiq_spi_readl(spi, SPI_STAT);
	return stat & SPI_STAT_BSY;
}

static void hw_enter_config_mode(const struct lantiq_spi *spi)
{
	lantiq_spi_writel(spi, SPI_WHBSTATE_CLREN, SPI_WHBSTATE);
}

static void hw_enter_active_mode(const struct lantiq_spi *spi)
{
	lantiq_spi_writel(spi, SPI_WHBSTATE_SETEN, SPI_WHBSTATE);
}

static void hw_setup_speed_hz(const struct lantiq_spi *spi,
				unsigned int max_speed_hz)
{
	u32 spi_clk, brt;

	/*
	 * SPI module clock is derived from FPI bus clock dependent on
	 * divider value in CLC.RMS which is always set to 1.
	 *
	 *                 f_SPI
	 * baudrate = --------------
	 *             2 * (BR + 1)
	 */
	spi_clk = clk_get_rate(spi->fpi_clk) / 2;

	if (max_speed_hz > spi_clk)
		brt = 0;
	else
		brt = spi_clk / max_speed_hz - 1;

	if (brt > 0xFFFF)
		brt = 0xFFFF;

	dev_dbg(spi->dev, "spi_clk %u, max_speed_hz %u, brt %u\n",
		spi_clk, max_speed_hz, brt);

	lantiq_spi_writel(spi, brt, SPI_BRT);
}

static void hw_setup_bits_per_word(const struct lantiq_spi *spi,
					unsigned int bits_per_word)
{
	u32 bm;

	/* CON.BM value = bits_per_word - 1 */
	bm = (bits_per_word - 1) << SPI_CON_BM_S;

	lantiq_spi_maskl(spi, SPI_CON_BM_M, bm, SPI_CON);
}

static void hw_setup_clock_mode(const struct lantiq_spi *spi,
				unsigned int mode)
{
	u32 con_set = 0, con_clr = 0;

	/*
	 * SPI mode mapping in CON register:
	 * Mode CPOL CPHA CON.PO CON.PH
	 *  0    0    0      0      1
	 *  1    0    1      0      0
	 *  2    1    0      1      1
	 *  3    1    1      1      0
	 */
	if (mode & SPI_CPHA)
		con_clr |= SPI_CON_PH;
	else
		con_set |= SPI_CON_PH;

	if (mode & SPI_CPOL)
		con_set |= SPI_CON_PO | SPI_CON_IDLE;
	else
		con_clr |= SPI_CON_PO | SPI_CON_IDLE;

	/* Set heading control */
	if (mode & SPI_LSB_FIRST)
		con_clr |= SPI_CON_HB;
	else
		con_set |= SPI_CON_HB;

	/* Set loopback mode */
	if (mode & SPI_LOOP)
		con_set |= SPI_CON_LB;
	else
		con_clr |= SPI_CON_LB;

	lantiq_spi_maskl(spi, con_clr, con_set, SPI_CON);
}

static void lantiq_spi_hw_init(const struct lantiq_spi *spi)
{
	/*
	 * Set clock divider for run mode to 1 to
	 * run at same frequency as FPI bus
	 */
	lantiq_spi_writel(spi, 1 << SPI_CLC_RMC_S, SPI_CLC);

	/* Put controller into config mode */
	hw_enter_config_mode(spi);

	/* Disable all interrupts */
	lantiq_spi_writel(spi, 0, SPI_IRNEN);

	/* Clear error flags */
	lantiq_spi_maskl(spi, 0, SPI_WHBSTATE_CLR_ERRORS, SPI_WHBSTATE);

	/* Enable error checking, disable TX/RX */
	lantiq_spi_writel(spi, SPI_CON_RUEN | SPI_CON_AEN | SPI_CON_TEN |
		SPI_CON_REN | SPI_CON_TXOFF | SPI_CON_RXOFF, SPI_CON);

	/* Setup default SPI mode */
	hw_setup_bits_per_word(spi, spi->bits_per_word);
	hw_setup_clock_mode(spi, SPI_MODE_0);

	/* Enable master mode and clear error flags */
	lantiq_spi_writel(spi, SPI_WHBSTATE_SETMS | SPI_WHBSTATE_CLR_ERRORS,
		SPI_WHBSTATE);

	/* Reset GPIO/CS registers */
	lantiq_spi_writel(spi, 0, SPI_GPOCON);
	lantiq_spi_writel(spi, 0xFF00, SPI_FPGO);

	/* Enable and flush FIFOs */
	rx_fifo_reset(spi);
	tx_fifo_reset(spi);
}

static void hw_chipselect_set(struct lantiq_spi *spi, unsigned int cs)
{
	u32 fgpo = (1 << (cs - 1 + SPI_FGPO_SETOUTN_S));
	lantiq_spi_writel(spi, fgpo, SPI_FPGO);
}

static void hw_chipselect_clear(struct lantiq_spi *spi, unsigned int cs)
{
	u32 fgpo = (1 << (cs - 1));
	lantiq_spi_writel(spi, fgpo, SPI_FPGO);
}

static void hw_chipselect_init(struct lantiq_spi *spi, unsigned int cs,
				unsigned int cs_high)
{
	u32 gpocon;

	/* set GPO pin to CS mode */
	gpocon = 1 << ((cs - 1) + SPI_GPOCON_ISCSBN_S);

	/* invert GPO pin */
	if (cs_high)
		gpocon |= 1 << (cs - 1);

	lantiq_spi_maskl(spi, 0, gpocon, SPI_GPOCON);
}

static void chipselect_enable(struct spi_device *spidev)
{
	struct lantiq_spi *spi = spi_master_get_devdata(spidev->master);
	struct lantiq_spi_cstate *cstate = spi_get_ctldata(spidev);

	if (cstate->cs_gpio >= 0)
		gpio_set_value(cstate->cs_gpio, spidev->mode & SPI_CS_HIGH);
	else
		hw_chipselect_clear(spi, spidev->chip_select);

	/* CS setup/recovery time */
	if (spi->cs_delay)
		ndelay(spi->cs_delay);
}

static void chipselect_disable(struct spi_device *spidev)
{
	struct lantiq_spi *spi = spi_master_get_devdata(spidev->master);
	struct lantiq_spi_cstate *cstate = spi_get_ctldata(spidev);

	/* CS hold time */
	if (spi->cs_delay)
		ndelay(spi->cs_delay);

	if (cstate->cs_gpio >= 0)
		gpio_set_value(cstate->cs_gpio, !(spidev->mode & SPI_CS_HIGH));
	else
		hw_chipselect_set(spi, spidev->chip_select);

	/* CS setup/recovery time */
	if (spi->cs_delay)
		ndelay(spi->cs_delay);
}

static int lantiq_spi_setup(struct spi_device *spidev)
{
	struct lantiq_spi *spi = spi_master_get_devdata(spidev->master);
	struct lantiq_spi_cstate *cstate = spi_get_ctldata(spidev);
	int err;

	if (!cstate) {
		cstate = kzalloc(sizeof(*cstate), GFP_KERNEL);
		if (!cstate)
			return -ENOMEM;

		spi_set_ctldata(spidev, cstate);
		cstate->cs_gpio = -ENOENT;

		if (spidev->cs_gpio >= 0) {
			dev_dbg(spi->dev, "using chipselect %u on GPIO %d\n",
				spidev->chip_select, spidev->cs_gpio);

			err = gpio_request(spidev->cs_gpio, dev_name(spi->dev));
			if (err)
				return err;

			gpio_direction_output(spidev->cs_gpio,
				!(spidev->mode & SPI_CS_HIGH));

			cstate->cs_gpio = spidev->cs_gpio;
		} else {
			dev_dbg(spi->dev, "using internal chipselect %u\n",
				spidev->chip_select);

			hw_chipselect_init(spi, spidev->chip_select,
				spidev->mode & SPI_CS_HIGH);
			hw_chipselect_set(spi, spidev->chip_select);
		}
	}

	return 0;
}

static void lantiq_spi_cleanup(struct spi_device *spidev)
{
	struct lantiq_spi_cstate *cstate = spi_get_ctldata(spidev);

	if (cstate->cs_gpio >= 0)
		gpio_free(cstate->cs_gpio);

	kfree(cstate);
	spi_set_ctldata(spidev, NULL);
}

static void hw_setup_message(const struct lantiq_spi *spi,
				struct spi_device *spidev)
{
	const struct lantiq_spi_hwcfg *hwcfg = spi->hwcfg;

	hw_enter_config_mode(spi);
	hw_setup_clock_mode(spi, spidev->mode);
	hw_enter_active_mode(spi);

	/* Enable interrupts */
	lantiq_spi_writel(spi, hwcfg->irnen_t | hwcfg->irnen_r | SPI_IRNEN_E,
		SPI_IRNEN);
}

static void hw_setup_transfer(struct lantiq_spi *spi, struct spi_device *spidev,
				struct spi_transfer *t)
{
	unsigned int speed_hz, bits_per_word;
	u32 con;

	if (t->speed_hz)
		speed_hz = t->speed_hz;
	else
		speed_hz = spidev->max_speed_hz;

	if (t->bits_per_word)
		bits_per_word = t->bits_per_word;
	else
		bits_per_word = spidev->bits_per_word;

	if (bits_per_word != spi->bits_per_word ||
		speed_hz != spi->speed_hz) {
		hw_enter_config_mode(spi);
		hw_setup_speed_hz(spi, speed_hz);
		hw_setup_bits_per_word(spi, bits_per_word);
		hw_enter_active_mode(spi);

		spi->speed_hz = speed_hz;
		spi->bits_per_word = bits_per_word;
	}

	/* Configure transmitter and receiver */
	con = lantiq_spi_readl(spi, SPI_CON);
	if (t->tx_buf)
		con &= ~SPI_CON_TXOFF;
	else
		con |= SPI_CON_TXOFF;

	if (t->rx_buf)
		con &= ~SPI_CON_RXOFF;
	else
		con |= SPI_CON_RXOFF;

	lantiq_spi_writel(spi, con, SPI_CON);
}

static void hw_finish_message(const struct lantiq_spi *spi)
{
	/* Disable interrupts */
	lantiq_spi_writel(spi, 0, SPI_IRNEN);

	/* Disable transmitter and receiver */
	lantiq_spi_maskl(spi, 0, SPI_CON_TXOFF | SPI_CON_RXOFF, SPI_CON);
}

static void tx_fifo_write(struct lantiq_spi *spi)
{
	const u8 *tx8;
	const u16 *tx16;
	const u32 *tx32;
	u32 data;
	unsigned int tx_free = tx_fifo_free(spi);

	while (spi->tx_todo && tx_free) {
		switch (spi->bits_per_word) {
		case 8:
			tx8 = spi->tx;
			data = *tx8;
			spi->tx_todo--;
			spi->tx++;
			break;
		case 16:
			tx16 = (u16 *) spi->tx;
			data = *tx16;
			spi->tx_todo -= 2;
			spi->tx += 2;
			break;
		case 32:
			tx32 = (u32 *) spi->tx;
			data = *tx32;
			spi->tx_todo -= 4;
			spi->tx += 4;
			break;
		default:
			BUG();
		}

		lantiq_spi_writel(spi, data, SPI_TB);
		tx_free--;
	}
}

static void rx_fifo_read_full_duplex(struct lantiq_spi *spi)
{
	u8 *rx8;
	u16 *rx16;
	u32 *rx32;
	u32 data;
	unsigned int rx_fill = rx_fifo_level(spi);

	while (rx_fill) {
		data = lantiq_spi_readl(spi, SPI_RB);

		switch (spi->bits_per_word) {
		case 8:
			rx8 = spi->rx;
			*rx8 = data;
			spi->rx_todo--;
			spi->rx++;
			break;
		case 16:
			rx16 = (u16 *) spi->rx;
			*rx16 = data;
			spi->rx_todo -= 2;
			spi->rx += 2;
			break;
		case 32:
			rx32 = (u32 *) spi->rx;
			*rx32 = data;
			spi->rx_todo -= 4;
			spi->rx += 4;
			break;
		default:
			BUG();
		}

		rx_fill--;
	}
}

static void rx_fifo_read_half_duplex(struct lantiq_spi *spi)
{
	u32 data, *rx32;
	u8 *rx8;
	unsigned int rxbv, shift;
	unsigned int rx_fill = rx_fifo_level(spi);

	/*
	 * In RX-only mode the bits per word value is ignored by HW. A value
	 * of 32 is used instead. Thus all 4 bytes per FIFO must be read.
	 * If remaining RX bytes are less than 4, the FIFO must be read
	 * differently. The amount of received and valid bytes is indicated
	 * by STAT.RXBV register value.
	 */
	while (rx_fill) {
		if (spi->rx_todo < 4)  {
			rxbv = (lantiq_spi_readl(spi, SPI_STAT) &
				SPI_STAT_RXBV_M) >> SPI_STAT_RXBV_S;
			data = lantiq_spi_readl(spi, SPI_RB);

			shift = (rxbv - 1) * 8;
			rx8 = spi->rx;

			while (rxbv) {
				*rx8++ = (data >> shift) & 0xFF;
				rxbv--;
				shift -= 8;
				spi->rx_todo--;
				spi->rx++;
			}
		} else {
			data = lantiq_spi_readl(spi, SPI_RB);
			rx32 = (u32 *) spi->rx;

			*rx32++ = data;
			spi->rx_todo -= 4;
			spi->rx += 4;
		}
		rx_fill--;
	}
}

static void rx_request(struct lantiq_spi *spi)
{
	unsigned int rxreq, rxreq_max;

	/*
	 * To avoid receive overflows at high clocks it is better to request
	 * only the amount of bytes that fits into all FIFOs. This value
	 * depends on the FIFO size implemented in hardware.
	 */
	rxreq = spi->rx_todo;
	rxreq_max = rx_fifo_size(spi) * 4;
	if (rxreq > rxreq_max)
		rxreq = rxreq_max;

	lantiq_spi_writel(spi, rxreq, SPI_RXREQ);
}

static irqreturn_t lantiq_spi_xmit_interrupt(int irq, void *data)
{
	struct lantiq_spi *spi = data;

	/* handle possible interrupts after device initialization */
	if (!spi->rx && !spi->tx)
		return IRQ_HANDLED;

	if (spi->tx) {
		if (spi->rx && spi->rx_todo)
			rx_fifo_read_full_duplex(spi);

		if (spi->tx_todo)
			tx_fifo_write(spi);
		else
			goto completed;
	} else if (spi->rx) {
		if (spi->rx_todo) {
			rx_fifo_read_half_duplex(spi);

			if (spi->rx_todo)
				rx_request(spi);
			else
				goto completed;
		} else
			goto completed;
	}

	return IRQ_HANDLED;

completed:
	spi->status = 0;
	complete(&spi->xfer_complete);

	return IRQ_HANDLED;
}

static irqreturn_t lantiq_spi_err_interrupt(int irq, void *data)
{
	struct lantiq_spi *spi = data;
	u32 stat = lantiq_spi_readl(spi, SPI_STAT);

	if (stat & SPI_STAT_RUE)
		dev_err(spi->dev, "receive underflow error\n");
	if (stat & SPI_STAT_TUE)
		dev_err(spi->dev, "transmit underflow error\n");
	if (stat & SPI_STAT_RE)
		dev_err(spi->dev, "receive overflow error\n");
	if (stat & SPI_STAT_TE)
		dev_err(spi->dev, "transmit overflow error\n");
	if (stat & SPI_STAT_ME)
		dev_err(spi->dev, "mode error\n");

	/* Disable all interrupts */
	lantiq_spi_writel(spi, 0, SPI_IRNEN);

	/* Clear error flags */
	lantiq_spi_maskl(spi, 0, SPI_WHBSTATE_CLR_ERRORS, SPI_WHBSTATE);

	/* flush FIFOs */
	rx_fifo_flush(spi);
	tx_fifo_flush(spi);

	/* set bad status so it can be retried */
	spi->status = -EIO;
	complete(&spi->xfer_complete);

	return IRQ_HANDLED;
}

static int transfer_start(struct lantiq_spi *spi, struct spi_device *spidev,
				struct spi_transfer *t)
{
	unsigned long flags;

	spin_lock_irqsave(&spi->lock, flags);

	spi->tx = t->tx_buf;
	spi->rx = t->rx_buf;
	spi->status = -EINPROGRESS;

	if (t->tx_buf) {
		spi->tx_todo = t->len;

		/* initially fill TX FIFO */
		tx_fifo_write(spi);
	}

	if (spi->rx) {
		spi->rx_todo = t->len;

		/* start shift clock in RX-only mode */
		if (!spi->tx)
			rx_request(spi);
	}

	spin_unlock_irqrestore(&spi->lock, flags);

	return 0;
}

static int transfer_wait_finished(struct lantiq_spi *spi)
{
	unsigned long timeout;

	/* wait for completion by interrupt */
	timeout = wait_for_completion_timeout(&spi->xfer_complete,
			msecs_to_jiffies(spi->timeout));
	if (!timeout)
		return -EIO;

	/* make sure that HW is idle */
	timeout = jiffies + msecs_to_jiffies(spi->timeout);
	do {
		if (!hw_is_busy(spi))
			return 0;

		cond_resched();
	} while (!time_after_eq(jiffies, timeout));

	/* flush FIFOs on timeout */
	rx_fifo_flush(spi);
	tx_fifo_flush(spi);

	return -EIO;
}

static int lantiq_spi_transfer_one_message(struct spi_master *master,
						struct spi_message *msg)
{
	struct lantiq_spi *spi = spi_master_get_devdata(master);
	struct spi_device *spidev = msg->spi;
	struct spi_transfer *t;
	int status;
	unsigned int cs_change = 1;

	hw_setup_message(spi, spidev);

	list_for_each_entry(t, &msg->transfers, transfer_list) {
		INIT_COMPLETION(spi->xfer_complete);

		hw_setup_transfer(spi, spidev, t);

		if (cs_change)
			chipselect_enable(spidev);
		cs_change = t->cs_change;

		status = transfer_start(spi, spidev, t);
		if (status) {
			dev_err(spi->dev, "failed to start transfer\n");
			goto done;
		}

		status = transfer_wait_finished(spi);
		if (status) {
			dev_err(spi->dev, "transfer timeout\n");
			goto done;
		}

		status = spi->status;
		if (status) {
			dev_err(spi->dev, "transfer failed\n");
			goto done;
		}

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

static int lantiq_spi_prepare_transfer(struct spi_master *master)
{
	struct lantiq_spi *spi = spi_master_get_devdata(master);

	pm_runtime_get_sync(spi->dev);

	return 0;
}

static int lantiq_spi_unprepare_transfer(struct spi_master *master)
{
	struct lantiq_spi *spi = spi_master_get_devdata(master);

	pm_runtime_put(spi->dev);

	return 0;
}

static const struct lantiq_spi_hwcfg spi_xway = {
	.num_chipselect = 3,
	.irnen_r = SPI_IRNEN_R_XWAY,
	.irnen_t = SPI_IRNEN_T_XWAY,
};

static const struct lantiq_spi_hwcfg spi_xrx = {
	.num_chipselect = 6,
	.irnen_r = SPI_IRNEN_R_XRX,
	.irnen_t = SPI_IRNEN_T_XRX,
};

static const struct of_device_id lantiq_spi_match[] = {
	{ .compatible = "lantiq,xway-spi", .data = &spi_xway, },
	{ .compatible = "lantiq,xrx200-spi", .data = &spi_xrx, },
	{ .compatible = "lantiq,xrx300-spi", .data = &spi_xrx, },
	{ .compatible = "lantiq,xrx330-spi", .data = &spi_xrx, },
	{},
};
MODULE_DEVICE_TABLE(of, lantiq_spi_match);

static int lantiq_spi_probe(struct platform_device *pdev)
{
	struct spi_master *master;
	struct resource *res;
	struct lantiq_spi *spi;
	const struct lantiq_spi_hwcfg *hwcfg;
	const struct of_device_id *match;
	int err, rx_irq, tx_irq, err_irq;

	match = of_match_device(lantiq_spi_match, &pdev->dev);
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

	rx_irq = platform_get_irq_byname(pdev, SPI_RX_IRQ_NAME);
	if (rx_irq < 0) {
		dev_err(&pdev->dev, "failed to get %s\n", SPI_RX_IRQ_NAME);
		return -ENXIO;
	}

	tx_irq = platform_get_irq_byname(pdev, SPI_TX_IRQ_NAME);
	if (tx_irq < 0) {
		dev_err(&pdev->dev, "failed to get %s\n", SPI_TX_IRQ_NAME);
		return -ENXIO;
	}

	err_irq = platform_get_irq_byname(pdev, SPI_ERR_IRQ_NAME);
	if (err_irq < 0) {
		dev_err(&pdev->dev, "failed to get %s\n", SPI_ERR_IRQ_NAME);
		return -ENXIO;
	}

	master = spi_alloc_master(&pdev->dev, sizeof(struct lantiq_spi));
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

	err = devm_request_irq(&pdev->dev, rx_irq, lantiq_spi_xmit_interrupt, 0,
		SPI_RX_IRQ_NAME, spi);
	if (err)
		goto err_master_put;

	err = devm_request_irq(&pdev->dev, tx_irq, lantiq_spi_xmit_interrupt, 0,
		SPI_TX_IRQ_NAME, spi);
	if (err)
		goto err_master_put;

	err = devm_request_irq(&pdev->dev, err_irq, lantiq_spi_err_interrupt, 0,
		SPI_ERR_IRQ_NAME, spi);
	if (err)
		goto err_master_put;

	spi->spi_clk = clk_get(&pdev->dev, NULL);
	if (IS_ERR(spi->spi_clk)) {
		err = PTR_ERR(spi->spi_clk);
		goto err_master_put;
	}
	clk_prepare_enable(spi->spi_clk);

	spi->fpi_clk = clk_get_sys("fpi", "fpi");
	if (IS_ERR(spi->fpi_clk)) {
		err = PTR_ERR(spi->fpi_clk);
		goto err_clk_disable;
	}

	init_completion(&spi->xfer_complete);
	spin_lock_init(&spi->lock);
	spi->timeout = 2000;
	spi->cs_delay = 100;
	spi->bits_per_word = 8;
	spi->speed_hz = 0;

	master->dev.of_node = pdev->dev.of_node;
	master->bus_num = 0;
	master->num_chipselect = hwcfg->num_chipselect;
	master->setup = lantiq_spi_setup;
	master->cleanup = lantiq_spi_cleanup;
	master->prepare_transfer_hardware = lantiq_spi_prepare_transfer;
	master->transfer_one_message = lantiq_spi_transfer_one_message;
	master->unprepare_transfer_hardware = lantiq_spi_unprepare_transfer;
	master->mode_bits = SPI_CPOL | SPI_CPHA | SPI_LSB_FIRST | SPI_CS_HIGH |
				SPI_LOOP;

	lantiq_spi_hw_init(spi);

	err = spi_register_master(master);
	if (err) {
		dev_err(&pdev->dev, "failed to register spi_master\n");
		goto err_clk_put;
	}

	dev_info(&pdev->dev,
		"Lantiq SPI controller (TXFS %u, RXFS %u, DMA %u)\n",
		tx_fifo_size(spi), rx_fifo_size(spi), supports_dma(spi));

	return 0;

err_clk_put:
	clk_put(spi->fpi_clk);
err_clk_disable:
	clk_disable_unprepare(spi->spi_clk);
	clk_put(spi->spi_clk);
err_master_put:
	platform_set_drvdata(pdev, NULL);
	spi_master_put(master);

	return err;
}

static int lantiq_spi_remove(struct platform_device *pdev)
{
	struct lantiq_spi *spi = platform_get_drvdata(pdev);
	struct spi_master *master = spi->master;

	spi_unregister_master(master);

	lantiq_spi_writel(spi, 0, SPI_IRNEN);
	rx_fifo_flush(spi);
	tx_fifo_flush(spi);
	hw_enter_config_mode(spi);

	clk_disable_unprepare(spi->spi_clk);
	clk_put(spi->spi_clk);
	clk_put(spi->fpi_clk);

	platform_set_drvdata(pdev, NULL);
	spi_master_put(master);

	return 0;
}

static struct platform_driver lantiq_spi_driver = {
	.probe = lantiq_spi_probe,
	.remove = lantiq_spi_remove,
	.driver = {
		.name = "spi-lantiq",
		.owner = THIS_MODULE,
		.of_match_table = lantiq_spi_match,
	},
};
module_platform_driver(lantiq_spi_driver);

MODULE_DESCRIPTION("Lantiq SPI controller driver");
MODULE_AUTHOR("Daniel Schwierzeck <daniel.schwierzeck@gmail.com>");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:spi-lantiq");
