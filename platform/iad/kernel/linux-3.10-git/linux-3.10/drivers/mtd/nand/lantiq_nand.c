/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Copyright (C) 2012 John Crispin <blogic@openwrt.org>
 * Copyright (C) 2015 Daniel Schwierzeck <daniel.schwierzeck@gmail.com>
 */
#define DEBUG
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/of_platform.h>
#include <linux/of_device.h>
#include <linux/of_mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/partitions.h>
#include <linux/dma-mapping.h>

#include <lantiq_soc.h>
#include <lantiq_dma.h>

/* EBU registers */
#define EBU_CON			0x10
#define EBU_ADDR_SEL_0		0x20
#define EBU_ADDR_SEL_1		0x24
#define EBU_CON_0		0x60
#define EBU_CON_1		0x64

/* NAND registers */
#define NAND_CON		0xb0
#define NAND_WAIT		0xb4
#define NAND_ECC0		0xb8
#define NAND_ECC_AC		0xbc
#define NAND_ECC_CR		0xc0

/* HSNAND registers */
#define HSNAND_NDAC_CTL1	0x10
#define HSNAND_NDAC_CTL2	0x14
#define HSNAND_INT_MASK_CTL	0x24
#define HSNAND_INT_STA		0x28
#define HSNAND_MD_CTL		0x30
#define HSNAND_ND_PARA0		0x3c
#define HSNAND_ND_ODD_ECC	0x40
#define HSNAND_ND_ODD_ECC1	0x44
#define HSNAND_ND_EVEN_ECC	0x48
#define HSNAND_ND_EVEN_ECC1	0x4c

#define EBU_ADDR_SEL_MASK(x)	(x << 4)
#define EBU_ADDR_SEL_REGEN	BIT(0)
#define EBU_CON_SETUP		(1 << 22)
#define EBU_CON_ALEC		(3 << 14)
#define EBU_CON_BCGEN_RES	(3 << 12)
#define EBU_CON_WAITWRC2	(2 << 8)
#define EBU_CON_WAITRDC2	(2 << 6)
#define EBU_CON_HOLDC1		(1 << 4)
#define EBU_CON_RECOVC1		(1 << 2)
#define EBU_CON_CMULT4		1

#define NAND_CON_ECC_ON		BIT(31)
#define NAND_CON_LATCH_PRE	BIT(23)
#define NAND_CON_LATCH_WP	BIT(22)
#define NAND_CON_LATCH_SE	BIT(21)
#define NAND_CON_LATCH_CS	BIT(20)
#define NAND_CON_LATCH_CLE	BIT(19)
#define NAND_CON_LATCH_ALE	BIT(18)
#define NAND_CON_OUT_CS1	BIT(10)
#define NAND_CON_IN_CS1		BIT(8)
#define NAND_CON_PRE_P		BIT(7)
#define NAND_CON_WP_P		BIT(6)
#define NAND_CON_SE_P		BIT(5)
#define NAND_CON_CS_P		BIT(4)
#define NAND_CON_CLE_P		BIT(3)
#define NAND_CON_ALE_P		BIT(2)
#define NAND_CON_CSMUX		BIT(1)
#define NAND_CON_NANDM		BIT(0)
#define NAND_CON_LATCH_ALL	(NAND_CON_LATCH_PRE | NAND_CON_LATCH_WP | \
				NAND_CON_LATCH_SE | NAND_CON_LATCH_CS)

#define NAND_WAIT_WR_C		BIT(3)
#define NAND_WAIT_RD_E		BIT(2)
#define NAND_WAIT_BY_E		BIT(1)
#define NAND_WAIT_RDBY		BIT(0)

#define NAND_CMD_ALE		BIT(2)
#define NAND_CMD_CLE		BIT(3)
#define NAND_CMD_CS		BIT(4)
#define NAND_CMD_SE		BIT(5)
#define NAND_CMD_WP		BIT(6)
#define NAND_CMD_PRE		BIT(7)

#define NDAC_CTL1_ADDR3(x)		(((x) & 0xff) << 24)
#define NDAC_CTL1_ADDR2(x)		(((x) & 0xff) << 16)
#define NDAC_CTL1_ADDR1(x)		(((x) & 0xff) << 8)
#define NDAC_CTL1_CMD(x)		((x) & 0xff)
#define NDAC_CTL1_CMD_READ		0x00
#define NDAC_CTL1_CMD_READ_SAMSUNG	0x60
#define NDAC_CTL1_CMD_WRITE		0x80
#define NDAC_CTL2_CMD2_READ_ONFI	0x00
#define NDAC_CTL2_CMD2_READ_SAMSUNG	0x60
#define NDAC_CTL2_CMD2_WRITE_ONFI	0x80
#define NDAC_CTL2_CMD2_WRITE_SAMSUNG	0x81
#define NDAC_CTL2_CMD_2(x)		(((x) & 0xff) << 19)
#define NDAC_CTL2_ADDR_CYCLE(x)		(((x) - 3) << 16)
#define NDAC_CTL2_ADDR5(x)		(((x) & 0xff) << 8)
#define NDAC_CTL2_ADDR4(x)		((x) & 0xff)
#define INT_AECC_EV			BIT(6)
#define INT_AECC_OD			BIT(5)
#define INT_WR_C			BIT(4)
#define MD_CTL_WRITE			BIT(10)
#define MD_CTL_CE_SHIFT			3
#define MD_CTL_CE_MASK			(0x7 << MD_CTL_CE_SHIFT)
#define MD_CTL_CE_DIS			(0x0 << MD_CTL_CE_SHIFT)
#define MD_CTL_CE_CS0			(0x1 << MD_CTL_CE_SHIFT)
#define MD_CTL_CE_CS1			(0x2 << MD_CTL_CE_SHIFT)
#define MD_CTL_GO			BIT(2)
#define MD_CTL_MODE_NO_ECC		BIT(1)
#define MD_CTL_DEFAULT			BIT(0)
#define ND_PARA0_TYPE_ONFI		BIT(18)
#define ND_PARA0_PCOUNT(x)		((((x) - 1) & 0xff) << 10)
#define ND_PARA0_PMODE_2PLANE		BIT(9)
#define ND_PARA0_ECC_4BYTE		BIT(8)
#define ND_PARA0_ECC_NO_OOB		BIT(7)
#define ND_PARA0_ECC_ADVANCED		BIT(6)
#define ND_PARA0_PIB_32			(0x0 << 4)
#define ND_PARA0_PIB_64			(0x1 << 4)
#define ND_PARA0_PIB_128		(0x2 << 4)
#define ND_PARA0_PIB_256		(0x3 << 4)
#define ND_PARA0_PAGE_512		0x0
#define ND_PARA0_PAGE_2048		0x1
#define ND_PARA0_PAGE_4096		0x2
#define ND_PARA0_PAGE_8192		0x3

#define DMA_ALIGN			32

extern __iomem void *ltq_dmanand_membase;
#define ltq_hsnand_w32(x, y)	ltq_w32((x), ltq_dmanand_membase + (y))
#define ltq_hsnand_r32(x)	ltq_r32(ltq_dmanand_membase + (x))

struct lantiq_nand;

struct lantiq_nand_hw {
	unsigned hw_ecc_hamming:1;
	unsigned hw_ecc_reed_solomon:1;
	void (*hw_init)(struct lantiq_nand *);
};

struct lantiq_nand {
	struct device *dev;
	const struct lantiq_nand_hw *hw;
	struct nand_chip chip;
	struct mtd_info mtd;
	struct completion comp;
	struct nand_ecclayout ecc_layout;
	struct dma_device_info *dma_dev;
	void __iomem *io_base;
	void *dma_buf;
	dma_addr_t dma_addr;
	size_t dma_buf_size;
	unsigned int addr_cycles;
	unsigned int pages_per_block;
	unsigned int hw_page;
	unsigned int hw_cmd;
	unsigned int hw_cmd_2;
	unsigned big_endian:1;
	unsigned is_onfi:1;
	unsigned plane2:1;
	unsigned ecc_4byte:1;
	unsigned ecc_no_oob:1;
	unsigned ecc_off:1;
	unsigned ecc_advanced:1;
};

static void lantiq_nand_latch_ale(bool state)
{
	if (state)
		ltq_ebu_w32_mask(0, NAND_CON_LATCH_ALE, NAND_CON);
	else
		ltq_ebu_w32_mask(NAND_CON_LATCH_ALE, 0, NAND_CON);
}

static void lantiq_nand_latch_cle(bool state)
{
	if (state)
		ltq_ebu_w32_mask(0, NAND_CON_LATCH_CLE, NAND_CON);
	else
		ltq_ebu_w32_mask(NAND_CON_LATCH_CLE, 0, NAND_CON);
}

static void lantiq_nand_wait(void)
{
	unsigned long timeout = jiffies + msecs_to_jiffies(50);

	do {
		if (ltq_ebu_r32(NAND_WAIT) & NAND_WAIT_WR_C)
			return;
		cond_resched();
	} while (!time_after_eq(jiffies, timeout));
}

static int lantiq_nand_dev_ready(struct mtd_info *mtd)
{
	return ltq_ebu_r32(NAND_WAIT) & NAND_WAIT_RDBY;
}

static void lantiq_nand_select_chip(struct mtd_info *mtd, int chipnum)
{
	if (chipnum < 0)
		ltq_ebu_w32_mask(NAND_CON_LATCH_ALL, 0, NAND_CON);
	else
		ltq_ebu_w32_mask(0, NAND_CON_LATCH_ALL, NAND_CON);
}

static void lantiq_nand_cmd_ctrl(struct mtd_info *mtd, int cmd, unsigned int ctrl)
{
	struct nand_chip *chip = mtd->priv;
	unsigned long addr_w = (unsigned long) chip->IO_ADDR_W;
	unsigned long addr_r = (unsigned long) chip->IO_ADDR_R;
	unsigned long flags;

	if (ctrl & NAND_CTRL_CHANGE) {
		if (ctrl & NAND_NCE) {
			addr_w |= NAND_CMD_CS;
			addr_r |= NAND_CMD_CS;
		} else {
			addr_w &= ~NAND_CMD_CS;
			addr_r &= ~NAND_CMD_CS;
		}

		if (ctrl & NAND_ALE) {
			lantiq_nand_latch_ale(true);
			addr_w |= NAND_CMD_ALE;
		} else {
			lantiq_nand_latch_ale(false);
			addr_w &= ~NAND_CMD_ALE;
		}

		if (ctrl & NAND_CLE) {
			lantiq_nand_latch_cle(true);
			addr_w |= NAND_CMD_CLE;
		} else {
			lantiq_nand_latch_cle(false);
			addr_w &= ~NAND_CMD_CLE;
		}

		chip->IO_ADDR_W = (void __iomem *) addr_w;
		chip->IO_ADDR_R = (void __iomem *) addr_r;
	}

	if (cmd != NAND_CMD_NONE) {
		spin_lock_irqsave(&ebu_lock, flags);
		writeb(cmd, chip->IO_ADDR_W);
		spin_unlock_irqrestore(&ebu_lock, flags);
		lantiq_nand_wait();
	}
}

static uint8_t lantiq_nand_read_byte(struct mtd_info *mtd)
{
	struct nand_chip *chip = mtd->priv;
	unsigned long flags;
	uint8_t val;

	spin_lock_irqsave(&ebu_lock, flags);
	val = readb(chip->IO_ADDR_R);
	spin_unlock_irqrestore(&ebu_lock, flags);

	return val;
}

static void lantiq_nand_read_buf(struct mtd_info *mtd, uint8_t *buf, int len)
{
	struct nand_chip *chip = mtd->priv;
	unsigned long flags;

	spin_lock_irqsave(&ebu_lock, flags);
	ioread8_rep(chip->IO_ADDR_R, buf, len);
	spin_unlock_irqrestore(&ebu_lock, flags);
}

static void lantiq_nand_write_buf(struct mtd_info *mtd, const uint8_t *buf,
					int len)
{
	struct nand_chip *chip = mtd->priv;
	unsigned long flags;

	spin_lock_irqsave(&ebu_lock, flags);
	iowrite8_rep(chip->IO_ADDR_W, buf, len);
	spin_unlock_irqrestore(&ebu_lock, flags);
	lantiq_nand_wait();
}

static void lantiq_nand_hw_cmd_addr(const struct mtd_info *mtd)
{
	struct nand_chip *chip = mtd->priv;
	struct lantiq_nand *nand = chip->priv;
	u32 ndac_ctl1, ndac_ctl2;

	ndac_ctl1 = NDAC_CTL1_ADDR1(0) | NDAC_CTL1_CMD(nand->hw_cmd);
	ndac_ctl2 = NDAC_CTL2_ADDR_CYCLE(nand->addr_cycles) |
		NDAC_CTL2_CMD_2(nand->hw_cmd_2);

	if (mtd->writesize > 512) {
		ndac_ctl1 |= NDAC_CTL1_ADDR2(0);
		ndac_ctl1 |= NDAC_CTL1_ADDR3(nand->hw_page);
		ndac_ctl2 |= NDAC_CTL2_ADDR4(nand->hw_page >> 8);
		if (nand->addr_cycles == 5)
			ndac_ctl2 |= NDAC_CTL2_ADDR5(nand->hw_page >> 16);
	} else {
		ndac_ctl1 |= NDAC_CTL1_ADDR2(nand->hw_page);
		ndac_ctl1 |= NDAC_CTL1_ADDR3(nand->hw_page >> 8);
		if (nand->addr_cycles == 4)
			ndac_ctl2 |= NDAC_CTL2_ADDR4(nand->hw_page >> 16);
	}

	ltq_hsnand_w32(ndac_ctl1, HSNAND_NDAC_CTL1);
	ltq_hsnand_w32(ndac_ctl2, HSNAND_NDAC_CTL2);
}

static void lantiq_nand_hw_ctl(bool do_write, bool do_ecc)
{
	u32 md_ctl = MD_CTL_DEFAULT | MD_CTL_CE_CS1;

	if (do_write)
		md_ctl |= MD_CTL_WRITE;

	if (!do_ecc)
		md_ctl |= MD_CTL_MODE_NO_ECC;

	ltq_hsnand_w32(md_ctl, HSNAND_MD_CTL);
}

static void lantiq_nand_hw_setup(struct lantiq_nand *nand)
{
	u32 nd_para0 = ND_PARA0_PCOUNT(1);

	if (nand->is_onfi)
		nd_para0 |= ND_PARA0_TYPE_ONFI;

	if (nand->plane2)
		nd_para0 |= ND_PARA0_PMODE_2PLANE;

	if (nand->ecc_4byte)
		nd_para0 |= ND_PARA0_ECC_4BYTE;

	if (nand->ecc_advanced)
		nd_para0 |= ND_PARA0_ECC_ADVANCED;

	switch (nand->pages_per_block) {
	case 32:
		nd_para0 |= ND_PARA0_PIB_32;
		break;
	case 64:
		nd_para0 |= ND_PARA0_PIB_64;
		break;
	case 128:
		nd_para0 |= ND_PARA0_PIB_128;
		break;
	case 256:
		nd_para0 |= ND_PARA0_PIB_256;
		break;
	default:
		break;
	}

	switch (nand->mtd.writesize) {
	case 512:
		nd_para0 |= ND_PARA0_PAGE_512;
		break;
	case 2048:
		nd_para0 |= ND_PARA0_PAGE_2048;
		break;
	case 4096:
		nd_para0 |= ND_PARA0_PAGE_4096;
		break;
	case 8192:
		nd_para0 |= ND_PARA0_PAGE_8192;
		break;
	default:
		break;
	}

	ltq_hsnand_w32(nd_para0, HSNAND_ND_PARA0);
	ltq_hsnand_w32(INT_AECC_EV | INT_AECC_OD | INT_WR_C, HSNAND_INT_STA);
	ltq_hsnand_w32(INT_AECC_EV | INT_AECC_OD | INT_WR_C, HSNAND_INT_MASK_CTL);
}

static void lantiq_nand_hw_go(void)
{
	u32 md_ctl = ltq_hsnand_r32(HSNAND_MD_CTL);
	md_ctl |= MD_CTL_GO;
	ltq_hsnand_w32(md_ctl, HSNAND_MD_CTL);
}

static int lantiq_nand_wr_c(void)
{
	unsigned long timeout = jiffies + msecs_to_jiffies(1000);
	u32 int_sta;

	do {
		int_sta = ltq_hsnand_r32(HSNAND_INT_STA);
		if (int_sta & INT_WR_C) {
			ltq_hsnand_w32(int_sta, HSNAND_INT_STA);
			return 0;
		}
		cond_resched();
	} while (!time_after_eq(jiffies, timeout));

	return -ETIMEDOUT;
}

static void lantiq_nand_cmdfunc(struct mtd_info *mtd, unsigned int command,
				int column, int page_addr)
{
	struct nand_chip *chip = mtd->priv;
	struct lantiq_nand *nand = chip->priv;

	nand->hw_page = page_addr;

	if (command == NAND_CMD_SEQIN && column < nand->mtd.writesize) {
		nand->hw_cmd = NDAC_CTL1_CMD_WRITE;
		nand->hw_cmd_2 = 0;
		lantiq_nand_hw_cmd_addr(mtd);
		return;
	}

	if (command == NAND_CMD_READ0 && column < nand->mtd.writesize) {
		nand->hw_cmd = NDAC_CTL1_CMD_READ;
		nand->hw_cmd_2 = 0;
		lantiq_nand_hw_cmd_addr(mtd);
		return;
	}

	if (nand->mtd.writesize > 512)
		nand_command_lp(mtd, command, column, page_addr);
	else
		nand_command(mtd, command, column, page_addr);
}

static int lantiq_nand_check_ecc_chunk(void *buf1, void *buf2, int len,
				       int bitflips_threshold)
{
	const unsigned char *bitmap1 = buf1;
	const unsigned char *bitmap2 = buf2;
	int bitflips = 0;

	for (; len && ((uintptr_t)bitmap1) % sizeof(long);
	     len--, bitmap1++, bitmap2++) {
		bitflips += hweight8(*bitmap1 ^ *bitmap2);
		if (unlikely(bitflips > bitflips_threshold))
			return -EBADMSG;
	}

	for (; len >= sizeof(long);
	     len -= sizeof(long), bitmap1 += sizeof(long),
	     bitmap2 += sizeof(long)) {
		bitflips += hweight32(*((unsigned long *)bitmap1) ^
				*((unsigned long *)bitmap2));
		if (unlikely(bitflips > bitflips_threshold))
			return -EBADMSG;
	}

	for (; len > 0; len--, bitmap1++, bitmap2++) {
		bitflips += hweight8(*bitmap1 ^ *bitmap2);
		if (unlikely(bitflips > bitflips_threshold))
			return -EBADMSG;
	}

	return bitflips;
}

static bool lantiq_nand_read_error(struct lantiq_nand *nand)
{
	u32 int_sta;

	/* HW only shows uncorrectable errors per ECC block */
	int_sta = ltq_hsnand_r32(HSNAND_INT_STA);
	if (int_sta & (INT_AECC_EV | INT_AECC_OD)) {
		/* clear ECC interrupts */
		ltq_hsnand_w32(int_sta, HSNAND_INT_STA);
	}

	return false;
}

static void lantiq_nand_control_rx_dma(struct lantiq_nand *nand, bool state)
{
	struct dma_device_info *dma_dev = nand->dma_dev;
	struct dma_channel_info *rx_chan;

	rx_chan = dma_dev->rx_chan[dma_dev->current_rx_chan];
	if (state)
		rx_chan->open(rx_chan);
	else
		rx_chan->close(rx_chan);
}

static void lantiq_nand_control_tx_dma(struct lantiq_nand *nand, bool state)
{
	struct dma_device_info *dma_dev = nand->dma_dev;
	struct dma_channel_info *tx_chan;

	tx_chan = dma_dev->tx_chan[dma_dev->current_tx_chan];
	if (state && tx_chan->control == IFX_DMA_CH_ON)
		tx_chan->enable_irq(tx_chan);
	else
		tx_chan->disable_irq(tx_chan);
}

static void lantiq_nand_read_page_dma(struct mtd_info *mtd,
				     struct nand_chip *chip, int oob_required)
{
	struct lantiq_nand *nand = chip->priv;

	dma_device_desc_setup(nand->dma_dev, nand->dma_buf,
		nand->dma_buf_size);
	lantiq_nand_control_rx_dma(nand, true);

	lantiq_nand_cmd_ctrl(mtd, NAND_CMD_NONE, NAND_NCE | NAND_ALE |
		NAND_CTRL_CHANGE);

	init_completion(&nand->comp);
	lantiq_nand_hw_go();
	wait_for_completion(&nand->comp);

	if (oob_required)
		chip->read_buf(mtd, chip->oob_poi, mtd->oobsize);

	lantiq_nand_cmd_ctrl(mtd, NAND_CMD_NONE, NAND_NCE | NAND_CTRL_CHANGE);
}

static int lantiq_nand_read_page_raw(struct mtd_info *mtd,
					struct nand_chip *chip, uint8_t *buf,
					int oob_required, int page)
{
	struct lantiq_nand *nand = chip->priv;

	lantiq_nand_hw_ctl(false, false);
	lantiq_nand_read_page_dma(mtd, chip, oob_required);
	memcpy(buf, nand->dma_buf, nand->dma_buf_size);

	return 0;
}

static int lantiq_nand_read_page_ecc(struct mtd_info *mtd,
					struct nand_chip *chip, uint8_t *buf,
					int oob_required, int page)
{
	struct lantiq_nand *nand = chip->priv;
	int i, stat;
	int eccbytes = chip->ecc.bytes;
	int eccsize = chip->ecc.size;
	int eccsteps = chip->ecc.steps;
	int eccstrength = chip->ecc.strength;
	int max_bitflips = 0;
	uint8_t *data = buf;
	uint8_t *data_raw = nand->dma_buf;
	uint8_t *oob_poi = chip->oob_poi;
	bool read_error;

	/* Read via DMA with HW ECC enabled into page buffer */
	lantiq_nand_hw_ctl(false, true);
	lantiq_nand_read_page_dma(mtd, chip, 0);
	memcpy(buf, nand->dma_buf, nand->dma_buf_size);

	/* Check if HW ECC engine indicated uncorrectable errors */
	read_error = lantiq_nand_read_error(nand);

	/*
	 * Read again via DMA but now with HW ECC disabled into temporary
	 * page buffer. Now we also read OOB to get the ECC bytes. We need
	 * this information to check for empty pages or to determine bit flips.
	 */
	lantiq_nand_hw_ctl(false, false);
	lantiq_nand_read_page_dma(mtd, chip, 1);

	for (i = 0; eccsteps; eccsteps--, i += eccbytes, data += eccsize,
	     data_raw += eccsize) {
		if (read_error) {
			/* Check for empty pages with bitflips */
			stat = nand_check_erased_ecc_chunk(data_raw, eccsize,
				&oob_poi[i], eccbytes, NULL, 0, eccstrength);
		} else {
			/*
			 * Check for bitflips between corrected and uncorrected
			 * page data.
			 */
			stat = lantiq_nand_check_ecc_chunk(data, data_raw,
				eccsize, eccstrength);
		}

		if (stat < 0) {
			mtd->ecc_stats.failed++;
		} else {
			mtd->ecc_stats.corrected += stat;
			max_bitflips = max_t(unsigned int, max_bitflips, stat);
		}
	}

	return max_bitflips;
}

static int lantiq_nand_write_page_dma(struct mtd_info *mtd,
				      struct nand_chip *chip,
				      const uint8_t *buf)
{
	struct lantiq_nand *nand = chip->priv;
	int err = 0;

	memcpy(nand->dma_buf, buf, nand->dma_buf_size);

	lantiq_nand_cmd_ctrl(mtd, NAND_CMD_NONE, NAND_NCE | NAND_ALE |
		NAND_CTRL_CHANGE);

	dma_device_write(nand->dma_dev, nand->dma_buf, nand->dma_buf_size,
		NULL);

	init_completion(&nand->comp);
	lantiq_nand_hw_go();
	wait_for_completion(&nand->comp);

	err = lantiq_nand_wr_c();

	lantiq_nand_cmd_ctrl(mtd, NAND_CMD_NONE, NAND_NCE | NAND_CTRL_CHANGE);

	return err;
}

static int lantiq_nand_write_page_raw(struct mtd_info *mtd,
					struct nand_chip *chip,
					const uint8_t *buf, int oob_required)
{
	struct lantiq_nand *nand = chip->priv;
	int err;

	lantiq_nand_hw_ctl(true, false);
	err = lantiq_nand_write_page_dma(mtd, chip, buf);

	if (!err && oob_required)
		chip->ecc.write_oob_raw(mtd, chip, nand->hw_page);

	return err ? -EIO : 0;
}

static int lantiq_nand_write_page_ecc(struct mtd_info *mtd,
					struct nand_chip *chip,
					const uint8_t *buf, int oob_required)
{
	struct lantiq_nand *nand = chip->priv;
	int err;

	lantiq_nand_hw_ctl(true, true);
	err = lantiq_nand_write_page_dma(mtd, chip, buf);

	if (!err && oob_required)
		chip->ecc.write_oob_raw(mtd, chip, nand->hw_page);

	return err ? -EIO : 0;
}

static int lantiq_nand_dma_intr_handler(struct dma_device_info *dma_dev,
					int status)
{
	struct lantiq_nand *nand = dma_dev->priv;

	switch (status) {
	case RCV_INT:
		lantiq_nand_control_rx_dma(nand, false);
		complete(&nand->comp);
		break;

	case TX_BUF_FULL_INT:
		lantiq_nand_control_tx_dma(nand, true);
		break;

	case TRANSMIT_CPT_INT:
		lantiq_nand_control_tx_dma(nand, false);
		complete(&nand->comp);
		break;
	}

	return 0;
}

static u8 *lantiq_nand_dma_buffer_alloc(int len, int *byte_offset, void **opt)
{
	return NULL;
}

static int lantiq_nand_dma_buffer_free(u8 *dataptr, void *opt)
{
	return 0;
}

static int lantiq_nand_dma_init(struct lantiq_nand *nand)
{
	struct dma_device_info *dma_dev;
	int err;

	dma_dev = dma_device_reserve("HSNAND");
	if (!dma_dev) {
		dev_err(nand->dev, "failed to reserve HSNAND DMA\n");
		return -ENODEV;
	}

	dma_dev->intr_handler = lantiq_nand_dma_intr_handler;
	dma_dev->buffer_alloc = lantiq_nand_dma_buffer_alloc;
	dma_dev->buffer_free = lantiq_nand_dma_buffer_free;
	dma_dev->tx_endianness_mode = IFX_DMA_ENDIAN_TYPE3;
	dma_dev->rx_endianness_mode = IFX_DMA_ENDIAN_TYPE3;
	dma_dev->tx_burst_len = DMA_BURSTL_8DW;
	dma_dev->rx_burst_len = DMA_BURSTL_8DW;
	dma_dev->num_rx_chan = 1;
	dma_dev->num_tx_chan = 1;
	dma_dev->current_tx_chan = 0;
	dma_dev->current_rx_chan = 0;
	dma_dev->tx_chan[0]->desc_len = 1;
	dma_dev->tx_chan[0]->byte_offset = 0;
	dma_dev->tx_chan[0]->control = IFX_DMA_CH_ON;
	dma_dev->rx_chan[0]->desc_len = 1;
	dma_dev->rx_chan[0]->byte_offset = 0;
	dma_dev->rx_chan[0]->control = IFX_DMA_CH_ON;
	dma_dev->priv = nand;

	err = dma_device_register(dma_dev);
	if (err) {
		dev_err(nand->dev, "failed to register HSNAND DMA\n");
		dma_device_release(dma_dev);
		return err;
	}

	nand->dma_dev = dma_dev;

	return 0;
}

static void lantiq_nand_dma_exit(struct lantiq_nand *nand)
{
	if (!nand->dma_dev)
		return;

	dma_device_unregister(nand->dma_dev);
	dma_device_release(nand->dma_dev);
	nand->dma_dev = NULL;
}

static int lantiq_nand_init(struct lantiq_nand *nand)
{
	const unsigned int hsnand_info = ltq_get_hsnand_info();

	if (nand->mtd.writesize > 512) {
		if (nand->chip.chipsize > (128 << 20))
			nand->addr_cycles = 5;
		else
			nand->addr_cycles = 4;
	} else {
		if (nand->chip.chipsize > (32 << 20))
			nand->addr_cycles = 4;
		else
			nand->addr_cycles = 3;
	}

	if (nand->chip.onfi_version)
		nand->is_onfi = 1;

	nand->pages_per_block = nand->mtd.erasesize / nand->mtd.writesize;

	if (hsnand_info & HSNAND_ECC_OFF)
		nand->ecc_off = 1;

	if (hsnand_info & HSNAND_ECC_4BYTE)
		nand->ecc_4byte = 1;

	if (hsnand_info & HSNAND_ECC_NO_OOB)
		nand->ecc_no_oob = 1;

	if (hsnand_info & HSNAND_BIG_ENDIAN)
		nand->big_endian = 1;

	dev_dbg(nand->dev, "byte per page %u, pages_per_block %u, addr_cycles %u\n",
		nand->mtd.writesize, nand->pages_per_block, nand->addr_cycles);
	dev_dbg(nand->dev, "ECC %s %dB %s\n", nand->ecc_off ? "off" : "on",
		nand->ecc_4byte ? 4 : 3, nand->ecc_no_oob ? "IB" : "OOB");

	return 0;
}

static void lantiq_nand_create_ecc_layout(struct lantiq_nand *nand)
{
	int i, ecc_steps, ecc_total;

	ecc_steps = nand->mtd.writesize / nand->chip.ecc.size;
	ecc_total = ecc_steps * nand->chip.ecc.bytes;

	nand->ecc_layout.eccbytes = ecc_total;

	for (i = 0; i < ecc_total; i++)
		nand->ecc_layout.eccpos[i] = i;

	if (ecc_total < nand->mtd.oobsize) {
		nand->ecc_layout.oobfree[0].offset = ecc_total;
		nand->ecc_layout.oobfree[0].length =
			nand->mtd.oobsize - ecc_total;
	}

	nand->chip.ecc.layout = &nand->ecc_layout;
}

static void lantiq_nand_ecc_init_reed_solomon(struct lantiq_nand *nand)
{
	nand->chip.cmdfunc = lantiq_nand_cmdfunc;
	nand->chip.ecc.read_page_raw = lantiq_nand_read_page_raw;
	nand->chip.ecc.read_page = lantiq_nand_read_page_ecc;
	nand->chip.ecc.write_page_raw = lantiq_nand_write_page_raw;
	nand->chip.ecc.write_page = lantiq_nand_write_page_ecc;
	nand->chip.ecc.size = 128;

	if (nand->ecc_4byte) {
		nand->chip.ecc.bytes = 4;
		nand->chip.ecc.strength = 2;
	} else {
		nand->chip.ecc.bytes = 3;
		nand->chip.ecc.strength = 1;
	}

	if (nand->ecc_advanced)
		nand->chip.ecc.strength += 1;

	lantiq_nand_create_ecc_layout(nand);

	nand->chip.options |= NAND_NO_SUBPAGE_WRITE;
	nand->chip.bbt_options |= NAND_BBT_NO_OOB_BBM | NAND_BBT_NO_OOB;
}

static void lantiq_nand_ecc_init_none(struct lantiq_nand *nand)
{
	nand->chip.cmdfunc = lantiq_nand_cmdfunc;
	nand->chip.ecc.read_page_raw = lantiq_nand_read_page_raw;
	nand->chip.ecc.write_page_raw = lantiq_nand_write_page_raw;
	nand->chip.options |= NAND_NO_SUBPAGE_WRITE;
}

static int lantiq_nand_ecc_init(struct lantiq_nand *nand)
{
	if (nand->ecc_off)
		nand->chip.ecc.mode = NAND_ECC_NONE;

	dev_dbg(nand->dev, "ECC mode %d, ecc_step_ds %u, ecc_strength_ds %u\n",
		nand->chip.ecc.mode, nand->chip.ecc_step_ds,
		nand->chip.ecc_strength_ds);

	switch (nand->chip.ecc.mode) {
	case NAND_ECC_HW:
		lantiq_nand_hw_setup(nand);
		lantiq_nand_ecc_init_reed_solomon(nand);
		break;
	case NAND_ECC_NONE:
		lantiq_nand_hw_setup(nand);
		lantiq_nand_ecc_init_none(nand);
	default:
		break;
	}

	return 0;
}

static void lantiq_nand_hw_init_xrx330(struct lantiq_nand *nand)
{
	unsigned long io_base = (unsigned long)nand->io_base;

	ltq_ebu_w32(NAND_CON_OUT_CS1 | NAND_CON_IN_CS1 | NAND_CON_PRE_P |
		NAND_CON_WP_P | NAND_CON_SE_P | NAND_CON_CS_P |
		NAND_CON_CSMUX | NAND_CON_NANDM, NAND_CON);

	ltq_ebu_w32(CPHYSADDR(io_base) | EBU_ADDR_SEL_MASK(2) |
		EBU_ADDR_SEL_REGEN, EBU_ADDR_SEL_1);

	ltq_ebu_w32(EBU_CON_SETUP | EBU_CON_ALEC | EBU_CON_BCGEN_RES |
		EBU_CON_WAITWRC2 | EBU_CON_WAITRDC2 | EBU_CON_HOLDC1 |
		EBU_CON_RECOVC1 | EBU_CON_CMULT4, EBU_CON_1);
}

static const struct lantiq_nand_hw lantiq_nand_xrx330 = {
	.hw_ecc_hamming = 1,
	.hw_ecc_reed_solomon = 1,
	.hw_init = lantiq_nand_hw_init_xrx330,
};

static struct of_device_id lantiq_nand_match[] = {
	{ .compatible = "lantiq,xrx330-nand", .data = &lantiq_nand_xrx330 },
	{},
};
MODULE_DEVICE_TABLE(of, lantiq_nand_match);

static int lantiq_nand_probe(struct platform_device *pdev)
{
	const struct of_device_id *match;
	struct lantiq_nand *nand;
	struct resource *res;
	void __iomem *base;
	const char *of_mtd_name = NULL;
	struct mtd_part_parser_data ppdata;
	int err, ecc_mode, val;

	match = of_match_device(lantiq_nand_match, &pdev->dev);
	if (!match) {
		dev_err(&pdev->dev, "no device match\n");
		return -EINVAL;
	}

	of_property_read_string(pdev->dev.of_node, "linux,mtd-name",
		&of_mtd_name);

	ecc_mode = of_get_nand_ecc_mode(pdev->dev.of_node);
	if (ecc_mode < 0) {
		dev_err(&pdev->dev, "no ECC mode specified\n");
		return -EINVAL;
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res)
		return -ENXIO;

	base = devm_request_and_ioremap(&pdev->dev, res);
	if (!base)
		return -EADDRNOTAVAIL;

	nand = devm_kzalloc(&pdev->dev, sizeof(*nand), GFP_KERNEL);
	if (!nand)
		return -ENOMEM;

	nand->dev = &pdev->dev;
	platform_set_drvdata(pdev, nand);

	nand->hw = match->data;
	nand->io_base = base;

	if (of_get_property(pdev->dev.of_node, "bbt-use-flash", &val))
		nand->chip.bbt_options |= NAND_BBT_USE_FLASH;

	nand->chip.priv = nand;
	nand->chip.IO_ADDR_R = base;
	nand->chip.IO_ADDR_W = base;
	nand->chip.bbt_options |= NAND_BBT_CREATE_EMPTY;
	nand->chip.chip_delay = 30;
	nand->chip.dev_ready = lantiq_nand_dev_ready;
	nand->chip.select_chip = lantiq_nand_select_chip;
	nand->chip.cmd_ctrl = lantiq_nand_cmd_ctrl;
	nand->chip.read_byte = lantiq_nand_read_byte;
	nand->chip.read_buf = lantiq_nand_read_buf;
	nand->chip.write_buf = lantiq_nand_write_buf;
	nand->chip.ecc.mode = ecc_mode;
	nand->mtd.priv = &nand->chip;
	nand->mtd.owner = THIS_MODULE;
	nand->mtd.dev.parent = &pdev->dev;

	if (of_mtd_name)
		nand->mtd.name = of_mtd_name;
	else
		dev_name(&pdev->dev);

	err = nand_scan_ident(&nand->mtd, 1, NULL);
	if (err)
		return err;

	nand->dma_buf_size = nand->mtd.writesize;
	nand->dma_buf = dma_alloc_coherent(&pdev->dev, nand->dma_buf_size,
				&nand->dma_addr, GFP_KERNEL);
	if (!nand->dma_buf)
		return -ENOMEM;

	err = lantiq_nand_dma_init(nand);
	if (err)
		goto err_free;

	err = lantiq_nand_init(nand);
	if (err)
		goto err_dma;

	err = lantiq_nand_ecc_init(nand);
	if (err)
		goto err_dma;

	err = nand_scan_tail(&nand->mtd);
	if (err)
		goto err_dma;

	ppdata.of_node = pdev->dev.of_node;
	err = mtd_device_parse_register(&nand->mtd, NULL, &ppdata,
		NULL, 0);
	if (err)
		goto err_nand;

	return 0;

err_nand:
	nand_release(&nand->mtd);
err_dma:
	lantiq_nand_dma_exit(nand);
err_free:
	dma_free_coherent(&pdev->dev, nand->dma_buf_size, nand->dma_buf,
		nand->dma_addr);
	return err;
}

static int lantiq_nand_remove(struct platform_device *pdev)
{
	struct lantiq_nand *nand = platform_get_drvdata(pdev);

	nand_release(&nand->mtd);
	lantiq_nand_dma_exit(nand);
	dma_free_coherent(nand->dev, nand->dma_buf_size, nand->dma_buf,
		nand->dma_addr);
	platform_set_drvdata(pdev, NULL);

	return 0;
}

static struct platform_driver lantiq_nand_driver = {
	.driver = {
		.name = "lantiq-nand",
		.owner = THIS_MODULE,
		.of_match_table = lantiq_nand_match,
	},
	.probe = lantiq_nand_probe,
	.remove = lantiq_nand_remove,
};
module_platform_driver(lantiq_nand_driver);

MODULE_AUTHOR("Daniel Schwierzeck <daniel.schwierzeck@gmail.com>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Lantiq NAND driver");
