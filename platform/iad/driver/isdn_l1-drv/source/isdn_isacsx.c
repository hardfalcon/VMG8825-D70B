/*
 * (C) Copyright 2005-2017 Sphairon GmbH (a ZyXEL company)
 *
 * SPDX-License-Identifier: GPL-2.0
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/uaccess.h>
#include <linux/delay.h>

#include "isdn_device.h"
#include "isdn_user.h"
#include "isdn_isacsx.h"
#include "isdn_l1.h"
#include "pcm_clk_gen.h"

#define ISDN_TRACE_REG_RW	0
#define ISDN_TRACE_MESSAGES	0
#define ISDN_TRACE_INTERRUPTS	0

/* Isac SPI Command header definitions */
#define ISAC_SPI_HDR_RANDOM	0x40 /* alternating read/write non-interleaved */
#define ISAC_SPI_HDR_RANDOMI	0x48 /* alternating read/write interleaved */
#define ISAC_SPI_HDR_RWONLY	0x43 /* read/write only constant address (auto increment) */
#define ISAC_SPI_HDR_RFWO	0x41 /* read followed by write-only non-interleaved */
#define ISAC_SPI_HDR_RFWOI	0x49 /* read followed by write-only interleaved */

#define ISAC_SPI_HI_ADDR	0x02 /* Bit to be set in Header Command for upper address range 80-FF */

bool isacsx_has_power_src1(const struct isdnl1 *isdnl1)
{
	u8 arx;

	if (!test_bit(ISDNL1_MODE_TE, &isdnl1->flags))
		return true;

	if (test_bit(ISDNL1_NO_PWR_SRC1, &isdnl1->flags))
		return true;

	arx = isacsx_reg_read(isdnl1, ISACSX_ARX);

	if (!(arx & (1 << isdnl1->pwr_src_pin)))
		return true;

	return false;
}

void isacsx_disable_irq(struct isdnl1 *isdnl1)
{
	/* Disable interrupts */
	isacsx_reg_write(isdnl1, ISACSX_MASK, 0xFF);
}

void isacsx_reg_write(const struct isdnl1 *isdnl1, u8 offset, u8 value)
{
	struct spi_message m;
	struct spi_transfer t;
	u8 cmd[3];
	int ret;

	if (ISDN_TRACE_REG_RW)
		dev_dbg(&isdnl1->sdev->dev, "%s: offset 0x%02x, value 0x%02x\n",
			__func__, offset, value);

	cmd[0] = ISAC_SPI_HDR_RANDOM | ((offset & 0x80) ? ISAC_SPI_HI_ADDR : 0);
	cmd[1] = offset & ~0x80;
	cmd[2] = value;

	spi_message_init(&m);
	memset(&t, 0, sizeof(t));

	t.tx_buf = cmd;
	t.len = sizeof(cmd);
	spi_message_add_tail(&t, &m);

	ret = spi_sync(isdnl1->sdev, &m);
}

u8 isacsx_reg_read(const struct isdnl1 *isdnl1, u8 offset)
{
	struct spi_message m;
	struct spi_transfer t[2];
	u8 cmd[2];
	u8 result;
	int ret;

	if (ISDN_TRACE_REG_RW)
		dev_dbg(&isdnl1->sdev->dev, "%s: offset 0x%02x\n",
			__func__, offset);

	cmd[0] = ISAC_SPI_HDR_RANDOM | ((offset & 0x80) ? ISAC_SPI_HI_ADDR : 0);
	/* set msb to indicate a read */
	cmd[1] = offset | 0x80;

	spi_message_init(&m);
	memset(t, 0, sizeof(t));

	t[0].tx_buf = cmd;
	t[0].len = sizeof(cmd);
	spi_message_add_tail(&t[0], &m);

	t[1].rx_buf = &result;
	t[1].len = sizeof(result);
	spi_message_add_tail(&t[1], &m);

	ret = spi_sync(isdnl1->sdev, &m);

	if (ISDN_TRACE_REG_RW)
		dev_dbg(&isdnl1->sdev->dev, "%s: got value 0x%02x,0x%02x, result 0x%02x\n",
			__func__, cmd[0], cmd[1], result);

	return result;
}

void isacsx_reg_mask(const struct isdnl1 *isdnl1, u8 offset, u8 set, u8 clr)
{
	u8 val = isacsx_reg_read(isdnl1, offset);
	val &= ~clr;
	val |= set;
	isacsx_reg_write(isdnl1, offset, val);
}

static void isacsx_init_spi(struct isdnl1 *isdnl1)
{
	/* Configure SPI interface to push/pull */
	isacsx_reg_write(isdnl1, ISACSX_MODE2, ISACSX_MODE2_PPSDX);
}

/*
 * Transceiver block 0x30-0x3a
 * ISACSX_TR_CONF0              (default 0x01)
 * ISACSX_TR_CONF1              (default 0x0x)
 * ISACSX_TR_CONF2              (default 0x80)
 * ISACSX_TR_STA                (default 0x00)
 * ISACSX_TR_CMD                (default 0x08)
 * ISACSX_SQRR1/ISACSX_SQXR1    (default 0x40/0x4f)
 * ISACSX_SQRR2/ISACSX_SQXR2    (default 0x00/0x00)
 * ISACSX_SQRR3/ISACSX_SQXR3    (default 0x00/0x00)
 * ISACSX_ISTATR                (default 0x00)
 * ISACSX_MASKTR                (default 0xff)
 * ISACSX_TR_MODE               (default 0x0x)
 */
static void isacsx_init_transceiver(struct isdnl1 *isdnl1)
{
	u8 tr_mode = 0x0;

	if (test_bit(ISDNL1_MODE_LTS, &isdnl1->flags))
		tr_mode = 0x3;

	if (test_bit(ISDNL1_MODE_LTT, &isdnl1->flags))
		tr_mode = 0x1;

	isacsx_reg_write(isdnl1, ISACSX_TR_MODE, tr_mode);
}

static void isacsx_reset(struct isdnl1 *isdnl1)
{
	u8 sres;

	sres = ISACSX_SRES_RES_CI | ISACSX_SRES_RES_BCH | ISACSX_SRES_RES_MON |
		ISACSX_SRES_RES_DCH | ISACSX_SRES_RES_IOM | ISACSX_SRES_RES_TR;

	isacsx_reg_write(isdnl1, ISACSX_CMDRD, CMDRD_XRES);
	isacsx_reg_write(isdnl1, ISACSX_CMDRD, CMDRD_RRES);

	if (!test_bit(ISDNL1_SKIP_SRES, &isdnl1->flags)) {
		isacsx_reg_write(isdnl1, ISACSX_SRES, sres);

		/*
		 * SRES is finished after four BCL cycles. Only wait for reset done
		 * if external PCM clocks are applied or pcm_clk_gen is disabled.
		 * pcm_clk_start() returns false in that case. Otherwise pcm_clk_gen
		 * generates 8 DCL cycles and returns true.
		 *
		 * Slowest DCL rate is 1536 kHz in TE mode -> BCL rate is 768 kHz.
		 * 4 * 1 / 768 kHz = 5.2 msec
		 */
		if (!pcm_clk_start())
			msleep(10);
	}
}

static void isacsx_write_fifo(struct isdnl1 *isdnl1,
	const void *buf, unsigned int len)
{
	const u8 *data = buf;

	for (; len > 0; len--) {
		isacsx_reg_write(isdnl1, ISACSX_XFIFOD, *data);
		data++;
	}
}

static void isacsx_read_fifo(struct isdnl1 *isdnl1,
	void *buf, unsigned int len)
{
	u8 *data = buf;

	/* ISAC-SX maintains an internal index, so we just read from offset 0 */
	for (; len > 0; len--) {
		*data = isacsx_reg_read(isdnl1, ISACSX_RFIFOD);
		data++;
	}
}

static void isacsx_empty_fifo(struct isdnl1 *isdnl1,
	unsigned int len)
{
	void *data = &isdnl1->rx_frame.Data[isdnl1->rx_frame.dwCount];

	isdnl1->rx_frame.dwCount += len;
	isacsx_read_fifo(isdnl1, data, len);
	isacsx_reg_write(isdnl1, ISACSX_CMDRD, CMDRD_RMC);
}

void isacsx_fill_fifo(struct isdnl1 *isdnl1)
{
	void *data;
	u8 cmd = CMDRD_XTF;
	unsigned int len;

	len = isdnl1->tx_frame.dwCount - isdnl1->tx_cnt;
	if (len == 0)
		return;

	if (len > ISACSX_XFIFOD_SIZE)
		len = ISACSX_XFIFOD_SIZE;
	else
		cmd |= CMDRD_XME;

	data = &isdnl1->tx_frame.Data[isdnl1->tx_cnt];
	isdnl1->tx_cnt += len;

	isacsx_write_fifo(isdnl1, data, len);
	isacsx_reg_write(isdnl1, ISACSX_CMDRD, cmd);
}

void isacsx_send_tx_frame(struct isdnl1 *isdnl1)
{
	unsigned int len;

	len = kfifo_get(&isdnl1->tx_msg, &isdnl1->tx_frame);
	WARN_ON(len != 1);

	if (ISDN_TRACE_MESSAGES) {
		dev_dbg(& isdnl1->sdev->dev, "sending L2 message with len %lu, remaining %u\n",
			isdnl1->tx_frame.dwCount, kfifo_len(&isdnl1->tx_msg));
		print_hex_dump_bytes("isdnl1: ", DUMP_PREFIX_OFFSET,
			isdnl1->tx_frame.Data, isdnl1->tx_frame.dwCount);
	}

	isacsx_fill_fifo(isdnl1);
}

static void isacsx_next_tx_frame(struct isdnl1 *isdnl1)
{
	unsigned int len;

	isdnl1->tx_cnt = 0;

	if (!kfifo_is_empty(&isdnl1->tx_msg)) {
		len = kfifo_get(&isdnl1->tx_msg, &isdnl1->tx_frame);
		WARN_ON(len != 1);

		if (ISDN_TRACE_MESSAGES) {
			dev_dbg(& isdnl1->sdev->dev, "sending L2 message with len %lu, remaining %u\n",
				isdnl1->tx_frame.dwCount, kfifo_len(&isdnl1->tx_msg));
			print_hex_dump_bytes("isdnl1: ", DUMP_PREFIX_OFFSET,
				isdnl1->tx_frame.Data, isdnl1->tx_frame.dwCount);
		}

		isacsx_fill_fifo(isdnl1);
	} else {
		memset(&isdnl1->tx_frame, 0, sizeof(L1MSG));
		clear_bit(ISDNL1_TX_BUSY, &isdnl1->flags);
		isdnl1->ph_event_cb(isdnl1, PH_DATA_CNF);
	}
}

static void isacsx_finish_rx(struct isdnl1 *isdnl1)
{
	if (ISDN_TRACE_MESSAGES) {
		dev_dbg(&isdnl1->sdev->dev, "received L2 message with len %lu\n",
			isdnl1->rx_frame.dwCount);
		print_hex_dump_bytes("isdnl1: ", DUMP_PREFIX_OFFSET,
			isdnl1->rx_frame.Data, isdnl1->rx_frame.dwCount);
	}

	if (atomic_read(&isdnl1->use_cnt)) {
		isdnl1->rx_frame.dwStatus = PH_DATA | L1_INDICATION;
		kfifo_put(&isdnl1->rx_msg, isdnl1->rx_frame);
		wake_up_interruptible(&isdnl1->rx_wait);
	}

	memset(&isdnl1->rx_frame, 0, sizeof(L1MSG));
}

static void isacsx_retransmit(struct isdnl1 *isdnl1)
{
	dev_dbg(&isdnl1->sdev->dev, "retransmit L2 message\n");

	if (test_bit(ISDNL1_TX_BUSY, &isdnl1->flags)) {
		isdnl1->tx_cnt = 0;
		isacsx_fill_fifo(isdnl1);
	}
}

static void isacsx_cic_irq_handler(struct isdnl1 *isdnl1)
{
	u8 cir0 = isacsx_reg_read(isdnl1, ISACSX_CIR0);
	u8 codr0 = cir0 >> 4;

	if (!(cir0 & CIR0_CIC0))
		return;

	if (ISDN_TRACE_INTERRUPTS)
		dev_dbg(&isdnl1->sdev->dev, "C/I code change 0x%x", codr0);

	if (test_bit(ISDNL1_MODE_LTS, &isdnl1->flags)) {
		switch (codr0) {
		case 0x0:
			l1_event(isdnl1, HW_IND_LTS_TIM);
			break;
		case 0x1:
			l1_event(isdnl1, HW_IND_LTS_RES);
			break;
		case 0x4:
			l1_event(isdnl1, HW_IND_LTS_RSY);
			break;
		case 0x8:
			l1_event(isdnl1, HW_IND_LTS_AR);
			break;
		case 0xb:
			l1_event(isdnl1, HW_IND_LTS_CVR);
			break;
		case 0xc:
			l1_event(isdnl1, HW_IND_LTS_AI);
			break;
		case 0xf:
			l1_event(isdnl1, HW_IND_LTS_DI);
			break;
		default:
			break;
		}
	}

	if (test_bit(ISDNL1_MODE_TE, &isdnl1->flags) ||
	    test_bit(ISDNL1_MODE_LTT, &isdnl1->flags)) {
		switch (codr0) {
		case 0x0:
			l1_event(isdnl1, HW_IND_TE_DR);
			break;
		case 0x1:
			l1_event(isdnl1, HW_IND_TE_RES);
			break;
		case 0x2:
			l1_event(isdnl1, HW_IND_TE_TMA);
			break;
		case 0x3:
			l1_event(isdnl1, HW_IND_TE_SLD);
			break;
		case 0x4:
			l1_event(isdnl1, HW_IND_TE_RSY);
			break;
		case 0x5:
			l1_event(isdnl1, HW_IND_TE_DR6);
			break;
		case 0x7:
			l1_event(isdnl1, HW_IND_TE_PU);
			break;
		case 0x8:
			l1_event(isdnl1, HW_IND_TE_AR);
			break;
		case 0xa:
			l1_event(isdnl1, HW_IND_TE_ARL);
			break;
		case 0xb:
			l1_event(isdnl1, HW_IND_TE_CVR);
			break;
		case 0xc:
			l1_event(isdnl1, HW_IND_TE_AI8);
			break;
		case 0xd:
			l1_event(isdnl1, HW_IND_TE_AI10);
			break;
		case 0xe:
			l1_event(isdnl1, HW_IND_TE_AIL);
			break;
		case 0xf:
			l1_event(isdnl1, HW_IND_TE_DC);
			break;
		default:
			break;
		}
	}
}

static void isacsx_rme_irq_handler(struct isdnl1 *isdnl1)
{
	unsigned int len;
	u8 rstad;

	len = (isacsx_reg_read(isdnl1, ISACSX_RBCHD) & 0x0F) << 8;
	len |= isacsx_reg_read(isdnl1, ISACSX_RBCLD);

	if (len < isdnl1->rx_frame.dwCount) {
		dev_err(&isdnl1->sdev->dev, "ignored invalid RX frame len %u\n", len);
		memset(&isdnl1->rx_frame, 0, sizeof(L1MSG));
		return;
	}

	len -= isdnl1->rx_frame.dwCount;
	isacsx_empty_fifo(isdnl1, len);

	rstad = isdnl1->rx_frame.Data[isdnl1->rx_frame.dwCount - 1];
	isdnl1->rx_frame.dwCount--;

	if ((rstad & RSTAD_VALID_MASK) != (RSTAD_VFR | RSTAD_CRC)) {
		dev_err(&isdnl1->sdev->dev, "ignored invalid RX frame (RSTAD %02x)\n", rstad);
		memset(&isdnl1->rx_frame, 0, sizeof(L1MSG));
		return;
	}

	isacsx_finish_rx(isdnl1);
}

static void isacsx_rpf_irq_handler(struct isdnl1 *isdnl1)
{
	isacsx_empty_fifo(isdnl1, ISACSX_RFIFOD_SIZE);
}

static void isacsx_xpr_irq_handler(struct isdnl1 *isdnl1)
{
	if (!test_bit(ISDNL1_TX_BUSY, &isdnl1->flags))
		return;

	if (isdnl1->tx_cnt < isdnl1->tx_frame.dwCount)
		isacsx_fill_fifo(isdnl1);
	else
		isacsx_next_tx_frame(isdnl1);
}

static void isacsx_icd_irq_handler(struct isdnl1 *isdnl1)
{
	u8 val;

	val = isacsx_reg_read(isdnl1, ISACSX_ISTAD);

	/* Handle Transmit Data Underrun */
	if (val & ISTAD_XDU) {
		if (ISDN_TRACE_INTERRUPTS)
			dev_dbg(&isdnl1->sdev->dev, "ISTAD XDU\n");
		isacsx_retransmit(isdnl1);
	}

	/* Handle Transmit Message Repeat */
	if (val & ISTAD_XMR) {
		if (ISDN_TRACE_INTERRUPTS)
			dev_dbg(&isdnl1->sdev->dev, "ISTAD XMR\n");
		isacsx_retransmit(isdnl1);
	}

	/* Handle Transmit Pool Ready */
	if (val & ISTAD_XPR) {
		if (ISDN_TRACE_INTERRUPTS)
			dev_dbg(&isdnl1->sdev->dev, "ISTAD XPR\n");
		isacsx_xpr_irq_handler(isdnl1);
	}

	/* Handle Receive Frame Overflow */
	if (val & ISTAD_RFO) {
		if (ISDN_TRACE_INTERRUPTS)
			dev_dbg(&isdnl1->sdev->dev, "ISTAD RFO\n");
		isacsx_reg_write(isdnl1, ISACSX_CMDRD, CMDRD_RMC);
	}

	/* Handle Receive Message End */
	if (val & ISTAD_RME) {
		if (ISDN_TRACE_INTERRUPTS)
			dev_dbg(&isdnl1->sdev->dev, "ISTAD RME\n");
		isacsx_rme_irq_handler(isdnl1);
	}

	/* Handle Receive Pool Full (indicates partial message in receive fifo) */
	if (val & ISTAD_RPF) {
		if (ISDN_TRACE_INTERRUPTS)
			dev_dbg(&isdnl1->sdev->dev, "ISTAD RPF\n");
		isacsx_rpf_irq_handler(isdnl1);
	}
}

int isacsx_irq_dispatch(struct isdnl1 *isdnl1)
{
	u8 ista = isacsx_reg_read(isdnl1, ISACSX_ISTA);
	unsigned int max_loop = 3;

	while ((ista & (ISACSX_ISTA_ICD | ISACSX_ISTA_CIC)) && max_loop--) {
		if (ISDN_TRACE_INTERRUPTS)
			dev_dbg(&isdnl1->sdev->dev, "ISTA %02x, max_loop %u\n",
				ista, max_loop);

		if (ista & ISACSX_ISTA_ICD)
			isacsx_icd_irq_handler(isdnl1);

		if (ista & ISACSX_ISTA_CIC)
			isacsx_cic_irq_handler(isdnl1);

		ista = isacsx_reg_read(isdnl1, ISACSX_ISTA);
	}

	return 0;
}

int isacsx_pcm_clk_cb(struct notifier_block *nb, unsigned long ev,
				void *data)
{
	struct isdnl1 *isdnl1 = container_of(nb, struct isdnl1, notifier);

	dev_dbg(&isdnl1->sdev->dev, "%s: event %ld\n", __func__, ev);

	if (!test_bit(ISDNL1_MODE_LTS, &isdnl1->flags))
		return NOTIFY_OK;

	/*
	* Without PCM clock the S0 interface is not useable. Therefore
	* indicate a L1 deactivation to upper ISDN layers.
	*/
	if (ev == PCM_CLK_EVENT_LOST) {
		ph_event(isdnl1, PH_DEACT_IND);
		clear_bit(ISDNL1_ACTIVATED, &isdnl1->flags);
	}

	/*
	* After applying PCM clocks force a restart of L1 activation to always
	* cleanly resynchronize the S0 interface.
	*/
	if (ev == PCM_CLK_EVENT_SYNC)
		l1_event(isdnl1, MPH_DEACT_REQ);

	return NOTIFY_OK;
}

int isacsx_ioctl_get_info(struct isdnl1 *isdnl1, unsigned long arg)
{
	void __user *argp = (void __user *)arg;
	L1DEV_INFO info;
	unsigned int tss_base = 4 * isdnl1->iom2_channel;

	info.dwVersion = ISACSX_DRV_VERSION;
	info.dwDevNum = isdnl1->cdev_minor;
	info.dwDevCount = 1;
	info.dwTimeslotA = tss_base;
	info.dwTimeslotB = tss_base + 1;
	info.dwTimeslotD = tss_base + 3;
	info.dwFlags = 0;

	if (test_bit(ISDNL1_MODE_TE, &isdnl1->flags))
		info.dwFlags |= IXF_TE_MODE;

	if (test_bit(ISDNL1_MODE_LTS, &isdnl1->flags))
		info.dwFlags |= IXF_LTS_MODE;

	if (test_bit(ISDNL1_MODE_LTT, &isdnl1->flags))
		info.dwFlags |= IXF_LTT_MODE;

	if (test_bit(ISDNL1_DPS, &isdnl1->flags))
		info.dwFlags |= IXF_SWAP_TRBC;

	return copy_to_user(argp, &info, sizeof(info));
}

int isacsx_ioctl_set_config(struct isdnl1 *isdnl1, unsigned long arg)
{
	void __user *argp = (void __user *)arg;
	L1DEV_CONFIG cfg;
	int err;

	err = copy_from_user(&cfg, argp, sizeof(cfg));
	if (err)
		return err;

	if (!(cfg.dwFlags & ICF_TX_ENABLE))
		return 0;

	if (!test_bit(ISDNL1_MODE_TE, &isdnl1->flags))
		return 0;

	if (cfg.bEnableTX) {
		if (cfg.dwBChannel & 0x01) {
			set_bit(ISDNL1_TX_ACTIVE_B2, &isdnl1->flags);
			isacsx_reg_mask(isdnl1, ISACSX_TR_CR, TR_CR_EN_B2X, 0);
		} else {
			set_bit(ISDNL1_TX_ACTIVE_B1, &isdnl1->flags);
			isacsx_reg_mask(isdnl1, ISACSX_TR_CR, TR_CR_EN_B1X, 0);
		}
	} else {
		if (cfg.dwBChannel & 0x01) {
			clear_bit(ISDNL1_TX_ACTIVE_B2, &isdnl1->flags);
			isacsx_reg_mask(isdnl1, ISACSX_TR_CR, 0, TR_CR_EN_B2X);
		} else {
			clear_bit(ISDNL1_TX_ACTIVE_B1, &isdnl1->flags);
			isacsx_reg_mask(isdnl1, ISACSX_TR_CR, 0, TR_CR_EN_B1X);
		}
	}

	return 0;
}

int isacsx_ioctl_get_config(struct isdnl1 *isdnl1, unsigned long arg)
{
	void __user *argp = (void __user *)arg;
	L1DEV_CONFIG cfg;
	int err;

	err = copy_from_user(&cfg, argp, sizeof(cfg));
	if (err)
		return err;

	if (cfg.dwBChannel & 0x01)
		cfg.bEnableTX = test_bit(ISDNL1_TX_ACTIVE_B2, &isdnl1->flags);
	else
		cfg.bEnableTX = test_bit(ISDNL1_TX_ACTIVE_B1, &isdnl1->flags);

	return copy_to_user(argp, &cfg, sizeof(cfg));
}

int isacsx_ioctl_set_l1state(struct isdnl1 *isdnl1, unsigned long arg)
{
	bool __user *argp = (bool __user *)arg;
	bool state;
	int err;

	err = get_user(state, argp);
	if (err)
		return err;

	if (state)
		l1_event(isdnl1, PH_ACT_REQ);
	else
		l1_event(isdnl1, MPH_DEACT_REQ);

	return 0;
}

int isacsx_ioctl_get_l1state(struct isdnl1 *isdnl1, unsigned long arg)
{
	bool __user *argp = (bool __user *)arg;
	bool state = !!test_bit(ISDNL1_ACTIVATED, &isdnl1->flags);

	return put_user(state, argp);
}

static void isacsx_setbusmode(struct isdnl1 *isdnl1, bool state)
{
	unsigned char tr_conf0;

	dev_dbg(&isdnl1->sdev->dev, "Setting %s mode\n",
		state ? "bus" : "P2P");

	tr_conf0 = isacsx_reg_read(isdnl1, ISACSX_TR_CONF0);

	if (state) {
		tr_conf0 |= TR_CONF0_BUS;
		set_bit(ISDNL1_BUS_SHORT, &isdnl1->flags);
	} else {
		tr_conf0 &= ~TR_CONF0_BUS;
		clear_bit(ISDNL1_BUS_SHORT, &isdnl1->flags);
	}

	isacsx_reg_write(isdnl1, ISACSX_TR_CONF0, tr_conf0);
}

int isacsx_ioctl_set_busmode(struct isdnl1 *isdnl1, unsigned long arg)
{
	bool __user *argp = (bool __user *)arg;
	bool busmode;
	int err;

	err = get_user(busmode, argp);
	if (err)
		return err;

	if (!test_bit(ISDNL1_MODE_LTS, &isdnl1->flags))
		return 0;

	isacsx_setbusmode(isdnl1, !busmode);

	return 0;
}

int isacsx_ioctl_get_status(struct isdnl1 *isdnl1, unsigned long arg)
{
	void __user *argp = (void __user *)arg;
	L1DEV_STATUS status;
	int err;

	err = copy_from_user(&status, argp, sizeof(status));
	if (err)
		return err;

	status.dwStatus = 0;

	if (status.dwMask & ISF_ONLINE) {
		if (isacsx_has_power_src1(isdnl1))
			status.dwStatus |= ISF_ONLINE;
	}

	if (status.dwMask & ISF_P2P) {
		if (test_bit(ISDNL1_BUS_SHORT, &isdnl1->flags))
			status.dwStatus &= ~ISF_P2P;
		else
			status.dwStatus |= ISF_P2P;
	}

	return copy_to_user(argp, &status, sizeof(status));
}

int isacsx_ioctl_enable(struct isdnl1 *isdnl1)
{
	unsigned int dps = 0;
	unsigned int tss_base = 4 * isdnl1->iom2_channel;

	dev_info(&isdnl1->sdev->dev, "enabling chip\n");

	l1_event(isdnl1, MPH_DEACT_REQ);
	msleep(100);

	isacsx_init_spi(isdnl1);
	isacsx_disable_irq(isdnl1);
	isacsx_reset(isdnl1);
	isacsx_init_transceiver(isdnl1);

	isacsx_reg_write(isdnl1, ISACSX_SQXR1, SQXR1_SQX);
	isacsx_reg_write(isdnl1, ISACSX_EXMD1, 0x00);
	isacsx_reg_write(isdnl1, ISACSX_MODE1, 0x00);

	/* Enable data port switch if requested by device-tree */
	if (test_bit(ISDNL1_DPS, &isdnl1->flags))
		dps = 0x80;
	else
		dps = 0x00;

	/*
	 * IOM-2 interface
	 * - Software Power Up -> disabled
	 * - Asynchronous Awake -> enabled
	 * - C/I channel selection -> configure C/I channel only
	 * - TIC Bus -> disabled
	 * - BCL/SCLK -> enabled (TODO: check if pin is connected at all)
	 * - Clock mode -> DCL is double bit clock
	 * - Open drain -> DU/DD are push/pull drivers
	 * - IOM interface -> enabled
	 * - Serial Data Strobe signals -> disabled
	 */
	isacsx_reg_write(isdnl1, ISACSX_IOM_CR, ISACSX_IOM_CR_EN_BCL |
		ISACSX_IOM_CR_TIC_DIS | ISACSX_IOM_CR_DIS_OD);
	isacsx_reg_write(isdnl1, ISACSX_SDS1_CR, 0);
	isacsx_reg_write(isdnl1, ISACSX_SDS2_CR, 0);

	/*
	 * Control monitor data
	 * see errata sheet for "Data Corruption Caused by MONITOR Handler"
	 * keep default value after reset 0x40
	 * - DPS -> 0
	 * - Enable output -> enabled
	 * - Channel selection -> 0
	*/
	#if 0
	isacsx_reg_write(isdnl1, ISACSX_MON_CR,
		dps | MON_CR_EN_MON | (isdnl1->iom2_channel & MON_CR_CS_MASK));
	#endif

	/*
	 * Control C/I data and HDLC D-channel data
	 * - C/I1 handler data -> disabled
	 * - D timeslot for D-channel controller -> enabled
	 * - B1 timeslot for D-channel controller -> disabled
	 * - B2 timeslot for D-channel controller -> disabled
	 * - IOM channel select -> channel number from device-tree
	 */
	isacsx_reg_write(isdnl1, ISACSX_DCI_CR,
		DCI_CR_D_EN_D | (isdnl1->iom2_channel & DCI_CR_CS_MASK));
	isacsx_reg_write(isdnl1, ISACSX_BCHA_CR,
		dps | BCH_CR_EN_D | (isdnl1->iom2_channel & BCH_CR_CS_MASK));
	isacsx_reg_write(isdnl1, ISACSX_BCHB_CR,
		isdnl1->iom2_channel & BCH_CR_CS_MASK);

	/* Control transceiver data access */
	isacsx_reg_write(isdnl1, ISACSX_TR_TSDP_BC1,
		dps | (tss_base & TSDP_TSS_MASK));
	isacsx_reg_write(isdnl1, ISACSX_TR_TSDP_BC2,
		dps | ((tss_base + 1) & TSDP_TSS_MASK));
	isacsx_reg_write(isdnl1, ISACSX_TR_CR, TR_CR_EN_D | TR_CR_EN_B2R |
		TR_CR_EN_B1R | TR_CR_EN_B2X | TR_CR_EN_B1X |
		(isdnl1->iom2_channel & TR_CR_CS_MASK));

	/*
	 * Controller data access
	 * - setup CDA1y for loops from DU to DD (IOM-side)
	 * - setup CDA2y for loops from DD to DU (TR-side)
	 */
	isacsx_reg_write(isdnl1, ISACSX_CDA1_CR, 0);
	isacsx_reg_write(isdnl1, ISACSX_CDA2_CR, 0);
	isacsx_reg_write(isdnl1, ISACSX_CDA_TSDP10,
		dps | (tss_base & TSDP_TSS_MASK));
	isacsx_reg_write(isdnl1, ISACSX_CDA_TSDP11,
		dps | ((tss_base + 1) & TSDP_TSS_MASK));
	isacsx_reg_write(isdnl1, ISACSX_CDA_TSDP20,
		(dps ^ 0x80) | (tss_base & TSDP_TSS_MASK));
	isacsx_reg_write(isdnl1, ISACSX_CDA_TSDP21,
		(dps ^ 0x80) | ((tss_base + 1) & TSDP_TSS_MASK));

	isacsx_reg_write(isdnl1, ISACSX_MODED,
		MODED_MDS_TRANS0 | MODED_RAC | MODED_DIM_COLLOFF |
		MODED_DIM1);

	isacsx_reg_write(isdnl1, ISACSX_TR_CONF0, TR_CONF0_LDD);
	isacsx_reg_write(isdnl1, ISACSX_TR_CONF2, 0x00);
	isacsx_reg_write(isdnl1, ISACSX_MASKD, 0x03);
	isacsx_reg_write(isdnl1, ISACSX_MASK, ~(ISACSX_MASK_ICD | ISACSX_MASK_CIC));

	return 0;
}

int isacsx_ioctl_disable(struct isdnl1 *isdnl1)
{
	dev_info(&isdnl1->sdev->dev, "disabling chip\n");

	isacsx_init_spi(isdnl1);
	isacsx_disable_irq(isdnl1);
	isacsx_reset(isdnl1);

	/* keep the transceiver and IOM enabled in TE mode */
	if (!test_bit(ISDNL1_MODE_TE, &isdnl1->flags))
	{
		/* disable IOM */
		isacsx_reg_write(isdnl1, ISACSX_IOM_CR, ISACSX_IOM_CR_DIS_IOM);

		/* disable transciever */
		isacsx_reg_write(isdnl1, ISACSX_TR_CONF0, TR_CONF0_DIS_TR);
		isacsx_reg_write(isdnl1, ISACSX_TR_CONF2, TR_CONF2_DIS_TX);
	}

	return 0;
}

static void isacsx_set_loop(struct isdnl1 *isdnl1, unsigned long flags)
{
	u8 cdax_cr = 0;
	const u8 tr_cr_mask = TR_CR_EN_D | TR_CR_EN_B2R | TR_CR_EN_B1R |
		TR_CR_EN_B2X | TR_CR_EN_B1X;

	if (flags & HWTEST_LOOP_TYPE_NONE) {
		dev_info(&isdnl1->sdev->dev, "loop: type none\n");
		isacsx_reg_write(isdnl1, ISACSX_CDA1_CR, 0);
		isacsx_reg_write(isdnl1, ISACSX_CDA2_CR, 0);
		isacsx_reg_mask(isdnl1, ISACSX_TR_CR, tr_cr_mask, 0);
		isacsx_reg_mask(isdnl1, ISACSX_IOM_CR, 0, ISACSX_IOM_CR_DIS_IOM);
		return;
	}

	if (flags & HWTEST_LOOP_TYPE_B1) {
		dev_info(&isdnl1->sdev->dev, "loop: type B1\n");
		cdax_cr |= ISACSX_CDAx_CR_EN_I0 | ISACSX_CDAx_CR_EN_O0;
	}

	if (flags & HWTEST_LOOP_TYPE_B2) {
		dev_info(&isdnl1->sdev->dev, "loop: type B2\n");
		cdax_cr |= ISACSX_CDAx_CR_EN_I1 | ISACSX_CDAx_CR_EN_O1;
	}

	if (flags & HWTEST_LOOP_TYPE_B1B2) {
		dev_info(&isdnl1->sdev->dev, "loop: type B1B2\n");
		cdax_cr |= ISACSX_CDAx_CR_EN_I1 | ISACSX_CDAx_CR_EN_I0 |
			ISACSX_CDAx_CR_EN_O1 | ISACSX_CDAx_CR_EN_O0 |
			ISACSX_CDAx_CR_SWAP;
	}

	if (flags & HWTEST_LOOP_SIDE_NONE) {
		/* Enable TR loop (DD -> DU), keep IOM enabled */
		dev_info(&isdnl1->sdev->dev, "loop: side none\n");
		isacsx_reg_mask(isdnl1, ISACSX_TR_CR, tr_cr_mask, 0);
		isacsx_reg_mask(isdnl1, ISACSX_IOM_CR, 0, ISACSX_IOM_CR_DIS_IOM);
		isacsx_reg_write(isdnl1, ISACSX_CDA1_CR, 0);
		isacsx_reg_mask(isdnl1, ISACSX_CDA2_CR, cdax_cr, 0);
	}

	if (flags & HWTEST_LOOP_SIDE_TR) {
		/* Enable TR loop (DD -> DU), disable IOM */
		dev_info(&isdnl1->sdev->dev, "loop: side TR\n");
		isacsx_reg_mask(isdnl1, ISACSX_TR_CR, tr_cr_mask, 0);
		isacsx_reg_mask(isdnl1, ISACSX_IOM_CR, ISACSX_IOM_CR_DIS_IOM, 0);
		isacsx_reg_write(isdnl1, ISACSX_CDA1_CR, 0);
		isacsx_reg_mask(isdnl1, ISACSX_CDA2_CR, cdax_cr, 0);
	}

	if (flags & HWTEST_LOOP_SIDE_IOM) {
		/* Disable TR, enable IOM loop (DU -> DD) */
		dev_info(&isdnl1->sdev->dev, "loop: side IOM\n");
		isacsx_reg_mask(isdnl1, ISACSX_TR_CR, 0, tr_cr_mask);
		isacsx_reg_mask(isdnl1, ISACSX_IOM_CR, 0, ISACSX_IOM_CR_DIS_IOM);
		isacsx_reg_mask(isdnl1, ISACSX_CDA1_CR, cdax_cr, 0);
		isacsx_reg_write(isdnl1, ISACSX_CDA2_CR, 0);
	}
}

int isacsx_ioctl_set_hwtest(struct isdnl1 *isdnl1, unsigned long arg)
{
	void __user *argp = (void __user *)arg;
	L1DEV_HWTEST hwtest;
	int err;

	err = copy_from_user(&hwtest, argp, sizeof(hwtest));
	if (err)
		return err;

	switch (hwtest.dwCmd) {
	case HWTEST_CMD_LOOP:
		isacsx_set_loop(isdnl1, hwtest.dwFlags);
		break;
	case HWTEST_CMD_SCP:
		l1_event(isdnl1, MPH_TEST_SCP);
		break;
	case HWTEST_CMD_SSP:
		l1_event(isdnl1, MPH_TEST_SSP);
		break;
	case HWTEST_CMD_RESET:
		l1_event(isdnl1, MPH_RESET_REQ);
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

int isacsx_ioctl_set_ledstatus(struct isdnl1 *isdnl1, unsigned long arg)
{
	void __user *argp = (void __user *)arg;
	L1DEV_SETLED setled;
	u8 reg_acfg1 = 0, reg_atx = 0, reg_aoe = 0;
	int err;

	err = copy_from_user(&setled, argp, sizeof(setled));
	if (err)
		return err;

	reg_acfg1 = isacsx_reg_read(isdnl1, ISACSX_ACFG1);
	reg_atx = isacsx_reg_read(isdnl1, ISACSX_ATX);
	reg_aoe = isacsx_reg_read(isdnl1, ISACSX_AOE);

	switch (setled.dwState) {
	case HW_LED_ON:
		dev_dbg(&isdnl1->sdev->dev, "LED state: HW_LED_ON\n");
		reg_acfg1 |= AOE_AUX6 | AOE_AUX7;
		reg_atx &= ~(AOE_AUX6 | AOE_AUX7);
		reg_aoe &= ~(AOE_AUX6 | AOE_AUX7);
		break;
	case HW_LED_B1_ON:
		dev_dbg(&isdnl1->sdev->dev, "LED state: HW_LED_B1_ON\n");
		reg_acfg1 |= AOE_AUX7;
		reg_atx &= ~AOE_AUX7;
		reg_aoe &= ~AOE_AUX7;
		break;
	case HW_LED_B2_ON:
		dev_dbg(&isdnl1->sdev->dev, "LED state: HW_LED_B2_ON\n");
		reg_acfg1 |= AOE_AUX6;
		reg_atx &= ~AOE_AUX6;
		reg_aoe &= ~AOE_AUX6;
		break;
	case HW_LED_OFF:
		dev_dbg(&isdnl1->sdev->dev, "LED state: HW_LED_OFF\n");
		reg_atx |= AOE_AUX6 | AOE_AUX7;
		reg_aoe &= ~(AOE_AUX6 | AOE_AUX7);
		break;
	case HW_LED_B1_OFF:
		dev_dbg(&isdnl1->sdev->dev, "LED state: HW_LED_B1_OFF\n");
		reg_atx |= AOE_AUX7;
		reg_aoe &= ~AOE_AUX7;
		break;
	case HW_LED_B2_OFF:
		dev_dbg(&isdnl1->sdev->dev, "LED state: HW_LED_B2_OFF\n");
		reg_atx |= AOE_AUX6;
		reg_aoe &= ~AOE_AUX6;
		break;
	}

	isacsx_reg_write(isdnl1, ISACSX_ACFG1, reg_acfg1);
	isacsx_reg_write(isdnl1, ISACSX_ATX, reg_atx);
	isacsx_reg_write(isdnl1, ISACSX_AOE, reg_aoe);

	return 0;
}

int isacsx_ioctl_read_reg(struct isdnl1 *isdnl1, unsigned long arg)
{
	void __user *argp = (void __user *)arg;
	L1DEV_REG reg;
	int err;

	err = copy_from_user(&reg, argp, sizeof(reg));
	if (err)
		return err;

	reg.dwRegVal = isacsx_reg_read(isdnl1, reg.dwRegOfs);

	return copy_to_user(argp, &reg, sizeof(reg));
}

int isacsx_ioctl_write_reg(struct isdnl1 *isdnl1, unsigned long arg)
{
	void __user *argp = (void __user *)arg;
	L1DEV_REG reg;
	int err;

	err = copy_from_user(&reg, argp, sizeof(reg));
	if (err)
		return err;

	isacsx_reg_write(isdnl1, reg.dwRegOfs, reg.dwRegVal);

	return 0;
}

void isacsx_hwcmd(struct isdnl1 *isdnl1, unsigned int cmd)
{
	u8 cix0 = 0;

	if (test_bit(ISDNL1_MODE_LTS, &isdnl1->flags)) {
		switch (cmd) {
		case HW_CMD_DR:
			cix0 = 0x0;
			break;
		case HW_CMD_RES:
			cix0 = 0x1;
			break;
		case HW_CMD_TEST_SSP:
			cix0 = 0x2;
			break;
		case HW_CMD_TEST_SCP:
			cix0 = 0x3;
			break;
		case HW_CMD_AR8:
			cix0 = 0x8;
			break;
		case HW_CMD_ARL:
			cix0 = 0xa;
			break;
		case HW_CMD_AIL:
			cix0 = 0xe;
			break;
		case HW_CMD_DC:
			cix0 = 0xf;
			break;
		default:
			return;
		}
	}

	if (test_bit(ISDNL1_MODE_TE, &isdnl1->flags) ||
	    test_bit(ISDNL1_MODE_LTT, &isdnl1->flags)) {
		switch (cmd) {
		case HW_CMD_TIM:
			cix0 = 0x0;
			break;
		case HW_CMD_RES:
			cix0 = 0x1;
			break;
		case HW_CMD_TEST_SSP:
			cix0 = 0x2;
			break;
		case HW_CMD_TEST_SCP:
			cix0 = 0x3;
			break;
		case HW_CMD_AR8:
			cix0 = 0x8;
			break;
		case HW_CMD_AR10:
			cix0 = 0x9;
			break;
		case HW_CMD_ARL:
			cix0 = 0xa;
			break;
		case HW_CMD_DI:
			cix0 = 0xf;
			break;
		default:
			return;
		}
	}

	dev_dbg(&isdnl1->sdev->dev, "HW cmd: %02x\n", (cix0 << 4));

	isacsx_reg_write(isdnl1, ISACSX_CIX0, (cix0 << 4) | 0xe);
}

void isacsx_hwinit(struct isdnl1 *isdnl1)
{
	isacsx_ioctl_disable(isdnl1);
	l1_event(isdnl1, MPH_RESET_REQ);
}
