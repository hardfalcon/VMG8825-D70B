/*
 * (C) Copyright 2005-2017 Sphairon GmbH (a ZyXEL company)
 *
 * SPDX-License-Identifier: GPL-2.0
 */

#ifndef __ISDN_L1_H__
#define __ISDN_L1_H__

enum l1_event {
	MPH_RESET_REQ,
	PH_ACT_REQ,
	MPH_DEACT_REQ,
	MPH_TEST_SSP,
	MPH_TEST_SCP,
	HW_IND_LTS_TIM,
	HW_IND_LTS_RES,
	HW_IND_LTS_RSY,
	HW_IND_LTS_AR,
	HW_IND_LTS_CVR,
	HW_IND_LTS_AI,
	HW_IND_LTS_DI,
	HW_IND_TE_DR,
	HW_IND_TE_RES,
	HW_IND_TE_TMA,
	HW_IND_TE_SLD,
	HW_IND_TE_RSY,
	HW_IND_TE_DR6,
	HW_IND_TE_PU,
	HW_IND_TE_AR,
	HW_IND_TE_ARL,
	HW_IND_TE_CVR,
	HW_IND_TE_AIL,
	HW_IND_TE_AI8,
	HW_IND_TE_AI10,
	HW_IND_TE_DC,
	EXP_TIMER1,
	EXP_TIMER3,
	EVENT_COUNT,
};

enum l1_cmd {
	HW_CMD_TEST_SSP,
	HW_CMD_TEST_SCP,
	HW_CMD_DR,
	HW_CMD_RES,
	HW_CMD_AR8,
	HW_CMD_AR10,
	HW_CMD_ARL,
	HW_CMD_AIL,
	HW_CMD_DC,
	HW_CMD_DI,
	HW_CMD_TIM,
};

struct FsmInst;
struct FsmTimer;
struct isdnl1;

int l1_init(void);
void l1_exit(void);
int l1_setup(struct FsmInst *fsm, struct FsmTimer *timer, struct isdnl1 *isdnl1);
void l1_event(struct isdnl1 *isdnl1, unsigned int event);

#endif /* __ISDN_L1_H__ */
