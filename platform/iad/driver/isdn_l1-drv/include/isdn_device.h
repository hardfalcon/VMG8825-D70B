/*
 * (C) Copyright 2005-2017 Sphairon GmbH (a ZyXEL company)
 *
 * SPDX-License-Identifier: GPL-2.0
 */

#ifndef __ISDN_DEVICE_H__
#define __ISDN_DEVICE_H__

#include <linux/spi/spi.h>
#include <linux/cdev.h>
#include <linux/workqueue.h>
#include <linux/bitops.h>

#include "isdn_user.h"
#include "fsm.h"
#include "kfifo.h"

// Driver Version number (can be queried via ioctl TSMINT_GET_INFO)
#define SPH_ISDN_DRV_VERSION     0x00040000	// Hi-Word = Major, Lo-Word = Minor
#define SPH_ISDN_MAJ_VER         ((unsigned short)(SPH_ISDN_DRV_VERSION >> 16))
#define SPH_ISDN_MIN_VER         ((unsigned short)(SPH_ISDN_DRV_VERSION & 0x0000FFFF))

enum isdnl1_chipid {
	CHIP_NONE,
	CHIP_PEB3086,
	CHIP_PSB21150,
};

enum isdnl1_flags {
	ISDNL1_MODE_TE,
	ISDNL1_MODE_LTT,
	ISDNL1_MODE_NT,
	ISDNL1_MODE_LTS,
	ISDNL1_BUS_SHORT,
	ISDNL1_TX_BUSY,
	ISDNL1_TX_ACTIVE_B1,
	ISDNL1_TX_ACTIVE_B2,
	ISDNL1_ACTIVATED,
	ISDNL1_NO_PWR_SRC1,
	ISDNL1_DPS,
	ISDNL1_SKIP_SRES,
};

enum ph_event {
	PH_ACT_IND,
	PH_DEACT_IND,
	PH_DATA_IND,
	PH_DATA_CNF,
};

struct isdnl1;
struct notifier_block;

typedef void (ph_event_cb)(struct isdnl1 *, enum ph_event);

struct isdnl1 {
	struct mutex mtx;
	struct spi_device *sdev;
	struct work_struct l1_cmd_work;
	struct notifier_block notifier;
	struct cdev cdev;
	int cdev_minor;
	atomic_t use_cnt;
	wait_queue_head_t rx_wait;
	wait_queue_head_t tx_wait;
	struct FsmInst l1_fsm;
	struct FsmTimer l1_timer;
	unsigned long flags;
	unsigned int tx_cnt;
	unsigned int iom2_channel;
	unsigned int pwr_src_pin;
	bool tx_enable_b1;
	bool tx_enable_b2;
	DECLARE_KFIFO(tx_msg, L1MSG, 64);
	DECLARE_KFIFO(rx_msg, L1MSG, 64);
	DECLARE_KFIFO(l1_cmd_queue, unsigned int, 4);
	L1MSG tx_frame;
	L1MSG rx_frame;
	ph_event_cb *ph_event_cb;
};

void l1_cmd(struct isdnl1 *isdnl1, unsigned int cmd);
void l1_cmd_queued(struct isdnl1 *isdnl1, unsigned int cmd);

static inline void ph_event(struct isdnl1 *isdnl1, enum ph_event ev)
{
	if (isdnl1->ph_event_cb)
		isdnl1->ph_event_cb(isdnl1, ev);
}

#endif /* __ISDN_DEVICE_H__ */
