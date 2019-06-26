/*
 * (C) Copyright 2005-2017 Sphairon GmbH (a ZyXEL company)
 *
 * SPDX-License-Identifier: GPL-2.0
 */

#include <linux/kernel.h>

#include "isdn_device.h"
#include "isdn_l1.h"
#include "isdn_isacsx.h"

#define LTS_TIMER1_VALUE	5000
#define LTS_TIMER2_VALUE	100
#define TE_TIMER3_VALUE		7000
#define TE_TIMER_SENSE_VALUE	2000

static struct Fsm l1fsm_lts;
static struct Fsm l1fsm_te;

/* Layer 1 States for LT-S mode */
enum lts_state {
	ST_LTS_RESET,
	ST_LTS_G1_DEACTIVATED,
	ST_LTS_G2_PENDING_ACTIVATION,
	ST_LTS_G2_LOST_FRAMING,
	ST_LTS_G3_ACTIVATED,
	ST_LTS_G4_PENDING_DEACTIVATION,
	ST_LTS_TEST_MODE_SSP,
	ST_LTS_TEST_MODE_SCP,
	ST_LTS_COUNT,
};

/* Layer 1 State Names for LT-S mode */
static const char *lts_state_names[] = {
	"RESET",
	"G1_DEACTIVATED",
	"G2_PENDING_ACTIVATION",
	"G2_LOST_FRAMING",
	"G3_ACTIVATED",
	"G4_PENDING_DEACTIVATION",
	"TEST_MODE_SSP",
	"TEST_MODE_SCP",
};

/* Layer 1 States for TE mode */
enum te_state {
	ST_TE_RESET,
	ST_TE_F3_DEACTIVATED,
	ST_TE_F3_POWER_UP,
	ST_TE_F3_PENDING_DEACTIVATION,
	ST_TE_F4_PENDING_ACTIVATION,
	ST_TE_F5_UNSYNCHRONIZED,
	ST_TE_F6_SYNCHRONIZED,
	ST_TE_F7_ACTIVATED,
	ST_TE_F8_LOST_FRAMING,
	ST_TE_TEST_MODE_SSP,
	ST_TE_TEST_MODE_SCP,
	ST_TE_LOOP_A_CLOSED,
	ST_TE_LOOP_A_ACTIVATED,
	ST_TE_COUNT,
};

/* Layer 1 State Names for TE mode */
static const char *te_state_names[] = {
	"RESET",
	"F3_DEACTIVATED",
	"F3_POWER_UP",
	"F3_PENDING_DEACTIVATION",
	"F4_PENDING_ACTIVATION",
	"F5_UNSYNCHRONIZED",
	"F6_SYNCHRONIZED",
	"F7_ACTIVATED",
	"F8_LOST_FRAMING",
	"TEST_MODE_SSP",
	"TEST_MODE_SCP",
	"LOOP_A_CLOSED",
	"LOOP_A_ACTIVATED",
};

static const char *l1_event_names[] = {
	"MPH_RESET_REQ",
	"PH_ACT_REQ",
	"MPH_DEACT_REQ",
	"MPH_TEST_SSP",
	"MPH_TEST_SCP",
	"HW_IND_LTS_TIM",
	"HW_IND_LTS_RES",
	"HW_IND_LTS_RSY",
	"HW_IND_LTS_AR",
	"HW_IND_LTS_CVR",
	"HW_IND_LTS_AI",
	"HW_IND_LTS_DI",
	"HW_IND_TE_DR",
	"HW_IND_TE_RES",
	"HW_IND_TE_TMA",
	"HW_IND_TE_SLD",
	"HW_IND_TE_RSY",
	"HW_IND_TE_DR6",
	"HW_IND_TE_PU",
	"HW_IND_TE_AR",
	"HW_IND_TE_ARL",
	"HW_IND_TE_CVR",
	"HW_IND_TE_AIL",
	"HW_IND_TE_AI8",
	"HW_IND_TE_AI10",
	"HW_IND_TE_DC",
	"EXP_TIMER1",
	"EXP_TIMER3",
};

void l1_event(struct isdnl1 *isdnl1, unsigned int event)
{
	FsmEvent(&isdnl1->l1_fsm, event, NULL);
}

static void lts_cmd_res(struct FsmInst *fi, int event, void *arg)
{
	struct isdnl1 *isdnl1 = fi->userdata;
	l1_cmd(isdnl1, HW_CMD_RES);
}

static void lts_cmd_ar(struct FsmInst *fi, int event, void *arg)
{
	struct isdnl1 *isdnl1 = fi->userdata;
	l1_cmd(isdnl1, HW_CMD_AR8);
}

static void lts_cmd_dr(struct FsmInst *fi, int event, void *arg)
{
	struct isdnl1 *isdnl1 = fi->userdata;
	l1_cmd(isdnl1, HW_CMD_DR);
}

static void lts_go_reset(struct FsmInst *fi, int event, void *arg)
{
	struct isdnl1 *isdnl1 = fi->userdata;
	FsmDelTimer(&isdnl1->l1_timer, 0);
	FsmChangeState(fi, ST_LTS_RESET);
	l1_cmd(isdnl1, HW_CMD_RES);
	clear_bit(ISDNL1_ACTIVATED, &isdnl1->flags);
}

static void lts_go_reset_ph(struct FsmInst *fi, int event, void *arg)
{
	struct isdnl1 *isdnl1 = fi->userdata;
	ph_event(isdnl1, PH_DEACT_IND);
	lts_go_reset(fi, event, arg);
}

static void lts_go_g1(struct FsmInst *fi, int event, void *arg)
{
	struct isdnl1 *isdnl1 = fi->userdata;
	FsmChangeState(fi, ST_LTS_G1_DEACTIVATED);
	l1_cmd(isdnl1, HW_CMD_DC);
}

static void lts_go_g2(struct FsmInst *fi, int event, void *arg)
{
	struct isdnl1 *isdnl1 = fi->userdata;
	FsmRestartTimer(&isdnl1->l1_timer, LTS_TIMER1_VALUE, EXP_TIMER1, isdnl1, 0);
	FsmChangeState(fi, ST_LTS_G2_PENDING_ACTIVATION);
	l1_cmd(isdnl1, HW_CMD_AR8);
}

static void lts_go_g2_lost_framing(struct FsmInst *fi, int event, void *arg)
{
	struct isdnl1 *isdnl1 = fi->userdata;
	FsmRestartTimer(&isdnl1->l1_timer, LTS_TIMER1_VALUE, EXP_TIMER1, isdnl1, 0);
	FsmChangeState(fi, ST_LTS_G2_LOST_FRAMING);
	l1_cmd(isdnl1, HW_CMD_AR8);
	clear_bit(ISDNL1_ACTIVATED, &isdnl1->flags);
}

static void lts_go_g3(struct FsmInst *fi, int event, void *arg)
{
	struct isdnl1 *isdnl1 = fi->userdata;
	FsmDelTimer(&isdnl1->l1_timer, 0);
	FsmChangeState(fi, ST_LTS_G3_ACTIVATED);
	l1_cmd(isdnl1, HW_CMD_AR8);
	set_bit(ISDNL1_ACTIVATED, &isdnl1->flags);
}

static void lts_go_g3_ph(struct FsmInst *fi, int event, void *arg)
{
	struct isdnl1 *isdnl1 = fi->userdata;
	lts_go_g3(fi, event, arg);
	ph_event(isdnl1, PH_ACT_IND);
}

static void lts_go_g4(struct FsmInst *fi, int event, void *arg)
{
	struct isdnl1 *isdnl1 = fi->userdata;
	FsmDelTimer(&isdnl1->l1_timer, 0);
	FsmChangeState(fi, ST_LTS_G4_PENDING_DEACTIVATION);

	/*
	 * L1 command must be queued if this function is called in soft-irq
	 * context due to a timer expired event.
	 */
	if (arg == isdnl1)
		l1_cmd_queued(isdnl1, HW_CMD_DR);
	else
		l1_cmd(isdnl1, HW_CMD_DR);

	clear_bit(ISDNL1_ACTIVATED, &isdnl1->flags);
}

static void lts_go_g4_ph(struct FsmInst *fi, int event, void *arg)
{
	struct isdnl1 *isdnl1 = fi->userdata;
	ph_event(isdnl1, PH_DEACT_IND);
	lts_go_g4(fi, event, arg);
}

static void lts_go_test_ssp(struct FsmInst *fi, int event, void *arg)
{
	struct isdnl1 *isdnl1 = fi->userdata;
	ph_event(isdnl1, PH_DEACT_IND);
	FsmDelTimer(&isdnl1->l1_timer, 0);
	FsmChangeState(fi, ST_LTS_TEST_MODE_SSP);
	l1_cmd(isdnl1, HW_CMD_TEST_SSP);
	clear_bit(ISDNL1_ACTIVATED, &isdnl1->flags);
}

static void lts_go_test_scp(struct FsmInst *fi, int event, void *arg)
{
	struct isdnl1 *isdnl1 = fi->userdata;
	ph_event(isdnl1, PH_DEACT_IND);
	FsmDelTimer(&isdnl1->l1_timer, 0);
	FsmChangeState(fi, ST_LTS_TEST_MODE_SCP);
	l1_cmd(isdnl1, HW_CMD_TEST_SCP);
	clear_bit(ISDNL1_ACTIVATED, &isdnl1->flags);
}

static const struct FsmNode lts_nodes[] = {
	{ ST_LTS_RESET, MPH_RESET_REQ, lts_cmd_res },
	{ ST_LTS_RESET, MPH_TEST_SSP, lts_go_test_ssp },
	{ ST_LTS_RESET, MPH_TEST_SCP, lts_go_test_scp },
	{ ST_LTS_RESET, HW_IND_LTS_DI, lts_go_g1 },
	{ ST_LTS_RESET, HW_IND_LTS_TIM, lts_go_g1 },

	{ ST_LTS_G4_PENDING_DEACTIVATION, MPH_RESET_REQ, lts_go_reset },
	{ ST_LTS_G4_PENDING_DEACTIVATION, MPH_TEST_SSP, lts_go_test_ssp },
	{ ST_LTS_G4_PENDING_DEACTIVATION, MPH_TEST_SCP, lts_go_test_scp },
	{ ST_LTS_G4_PENDING_DEACTIVATION, PH_ACT_REQ, lts_go_g2 },
	{ ST_LTS_G4_PENDING_DEACTIVATION, HW_IND_LTS_DI, lts_go_g1 },
	{ ST_LTS_G4_PENDING_DEACTIVATION, HW_IND_LTS_AR, lts_go_g2 },

	{ ST_LTS_G1_DEACTIVATED, MPH_RESET_REQ, lts_go_reset },
	{ ST_LTS_G1_DEACTIVATED, MPH_TEST_SSP, lts_go_test_ssp },
	{ ST_LTS_G1_DEACTIVATED, MPH_TEST_SCP, lts_go_test_scp },
	{ ST_LTS_G1_DEACTIVATED, PH_ACT_REQ, lts_cmd_ar },
	{ ST_LTS_G1_DEACTIVATED, MPH_DEACT_REQ, lts_go_g4 },
	{ ST_LTS_G1_DEACTIVATED, HW_IND_LTS_AR, lts_go_g2 },
	/* allow direct transitions if we miss some C/I state changes/interrupts */
	{ ST_LTS_G1_DEACTIVATED, HW_IND_LTS_AI, lts_go_g3_ph },
	{ ST_LTS_G1_DEACTIVATED, HW_IND_LTS_RSY, lts_go_g4 },

	{ ST_LTS_G2_PENDING_ACTIVATION, MPH_RESET_REQ, lts_go_reset },
	{ ST_LTS_G2_PENDING_ACTIVATION, MPH_TEST_SSP, lts_go_test_ssp },
	{ ST_LTS_G2_PENDING_ACTIVATION, MPH_TEST_SCP, lts_go_test_scp },
	{ ST_LTS_G2_PENDING_ACTIVATION, MPH_DEACT_REQ, lts_cmd_dr },
	{ ST_LTS_G2_PENDING_ACTIVATION, HW_IND_LTS_AI, lts_go_g3_ph },
	{ ST_LTS_G2_PENDING_ACTIVATION, EXP_TIMER1, lts_go_g4 },

	{ ST_LTS_G3_ACTIVATED, MPH_RESET_REQ, lts_go_reset_ph },
	{ ST_LTS_G3_ACTIVATED, MPH_TEST_SSP, lts_go_test_ssp },
	{ ST_LTS_G3_ACTIVATED, MPH_TEST_SCP, lts_go_test_scp },
	{ ST_LTS_G3_ACTIVATED, MPH_DEACT_REQ, lts_cmd_dr },
	{ ST_LTS_G3_ACTIVATED, HW_IND_LTS_TIM, lts_go_g4_ph },
	{ ST_LTS_G3_ACTIVATED, HW_IND_LTS_RSY, lts_go_g2_lost_framing },

	{ ST_LTS_G2_LOST_FRAMING, MPH_RESET_REQ, lts_go_reset_ph },
	{ ST_LTS_G2_LOST_FRAMING, MPH_TEST_SSP, lts_go_test_ssp },
	{ ST_LTS_G2_LOST_FRAMING, MPH_TEST_SCP, lts_go_test_scp },
	{ ST_LTS_G2_LOST_FRAMING, MPH_DEACT_REQ, lts_cmd_dr },
	{ ST_LTS_G2_LOST_FRAMING, HW_IND_LTS_AI, lts_go_g3 },
	{ ST_LTS_G2_LOST_FRAMING, EXP_TIMER1, lts_go_g4_ph },

	{ ST_LTS_TEST_MODE_SSP, MPH_RESET_REQ, lts_go_reset },
	{ ST_LTS_TEST_MODE_SCP, MPH_RESET_REQ, lts_go_reset },
};

static void te_cmd_res(struct FsmInst *fi, int event, void *arg)
{
	struct isdnl1 *isdnl1 = fi->userdata;
	l1_cmd(isdnl1, HW_CMD_RES);
}

static void te_cmd_ar(struct FsmInst *fi, int event, void *arg)
{
	struct isdnl1 *isdnl1 = fi->userdata;
	l1_cmd(isdnl1, HW_CMD_AR8);
}

static void te_cmd_di(struct FsmInst *fi, int event, void *arg)
{
	struct isdnl1 *isdnl1 = fi->userdata;
	l1_cmd_queued(isdnl1, HW_CMD_DI);
}

static void te_go_reset(struct FsmInst *fi, int event, void *arg)
{
	struct isdnl1 *isdnl1 = fi->userdata;
	FsmDelTimer(&isdnl1->l1_timer, 0);
	FsmChangeState(fi, ST_TE_RESET);
	l1_cmd(isdnl1, HW_CMD_DI);
	clear_bit(ISDNL1_ACTIVATED, &isdnl1->flags);
}

static void te_go_reset_ph(struct FsmInst *fi, int event, void *arg)
{
	struct isdnl1 *isdnl1 = fi->userdata;
	ph_event(isdnl1, PH_DEACT_IND);
	te_go_reset(fi, event, arg);
}

static void te_go_f3_power_up(struct FsmInst *fi, int event, void *arg)
{
	struct isdnl1 *isdnl1 = fi->userdata;
	FsmChangeState(fi, ST_TE_F3_POWER_UP);
	clear_bit(ISDNL1_ACTIVATED, &isdnl1->flags);
}

static void te_go_f3_power_up_ph(struct FsmInst *fi, int event, void *arg)
{
	struct isdnl1 *isdnl1 = fi->userdata;
	ph_event(isdnl1, PH_DEACT_IND);
	te_go_f3_power_up(fi, event, arg);
}

static void te_go_f3_deactivated(struct FsmInst *fi, int event, void *arg)
{
	struct isdnl1 *isdnl1 = fi->userdata;
	FsmChangeState(fi, ST_TE_F3_DEACTIVATED);
	l1_cmd(isdnl1, HW_CMD_DI);
	clear_bit(ISDNL1_ACTIVATED, &isdnl1->flags);
}

static void te_go_f3_deactivated_ph(struct FsmInst *fi, int event, void *arg)
{
	struct isdnl1 *isdnl1 = fi->userdata;
	ph_event(isdnl1, PH_DEACT_IND);
	te_go_f3_deactivated(fi, event, arg);
}

static void te_go_f3_pending_deactivation(struct FsmInst *fi, int event, void *arg)
{
	struct isdnl1 *isdnl1 = fi->userdata;
	ph_event(isdnl1, PH_DEACT_IND);
	FsmChangeState(fi, ST_TE_F3_PENDING_DEACTIVATION);
	l1_cmd(isdnl1, HW_CMD_DI);
	clear_bit(ISDNL1_ACTIVATED, &isdnl1->flags);
}

static void te_go_f4(struct FsmInst *fi, int event, void *arg)
{
	struct isdnl1 *isdnl1 = fi->userdata;
	FsmRestartTimer(&isdnl1->l1_timer, TE_TIMER3_VALUE, EXP_TIMER3, NULL, 0);
	FsmChangeState(fi, ST_TE_F4_PENDING_ACTIVATION);
	l1_cmd(isdnl1, HW_CMD_AR8);
}

static void te_go_f5(struct FsmInst *fi, int event, void *arg)
{
	FsmChangeState(fi, ST_TE_F5_UNSYNCHRONIZED);
}

static void te_go_f6(struct FsmInst *fi, int event, void *arg)
{
	FsmChangeState(fi, ST_TE_F6_SYNCHRONIZED);
}

static void te_go_f7_8(struct FsmInst *fi, int event, void *arg)
{
	struct isdnl1 *isdnl1 = fi->userdata;
	FsmDelTimer(&isdnl1->l1_timer, 0);
	FsmChangeState(fi, ST_TE_F7_ACTIVATED);
	l1_cmd(isdnl1, HW_CMD_AR8);
	set_bit(ISDNL1_ACTIVATED, &isdnl1->flags);
	ph_event(isdnl1, PH_ACT_IND);
}

static void te_go_f7_10(struct FsmInst *fi, int event, void *arg)
{
	struct isdnl1 *isdnl1 = fi->userdata;
	FsmDelTimer(&isdnl1->l1_timer, 0);
	FsmChangeState(fi, ST_TE_F7_ACTIVATED);
	l1_cmd(isdnl1, HW_CMD_AR10);
	set_bit(ISDNL1_ACTIVATED, &isdnl1->flags);
	ph_event(isdnl1, PH_ACT_IND);
}

static void te_go_f8(struct FsmInst *fi, int event, void *arg)
{
	FsmChangeState(fi, ST_TE_F8_LOST_FRAMING);
}

static const struct FsmNode te_nodes[] = {
	{ ST_TE_RESET, MPH_RESET_REQ, te_cmd_res },
	{ ST_TE_RESET, HW_IND_TE_RES, te_go_reset },
	{ ST_TE_RESET, HW_IND_TE_DC, te_go_f3_deactivated },
	{ ST_TE_RESET, HW_IND_TE_PU, te_go_f3_power_up },
	{ ST_TE_RESET, HW_IND_TE_AI8, te_go_f7_8 },
	{ ST_TE_RESET, HW_IND_TE_AI10, te_go_f7_10 },

	{ ST_TE_F3_DEACTIVATED, MPH_RESET_REQ, te_cmd_res },
	{ ST_TE_F3_DEACTIVATED, HW_IND_TE_RES, te_go_reset },
	{ ST_TE_F3_DEACTIVATED, PH_ACT_REQ, te_cmd_ar },
	{ ST_TE_F3_DEACTIVATED, HW_IND_TE_PU, te_go_f4 },
	{ ST_TE_F3_DEACTIVATED, HW_IND_TE_AR, te_go_f6 },
	{ ST_TE_F3_DEACTIVATED, HW_IND_TE_AI8, te_go_f7_8 },
	{ ST_TE_F3_DEACTIVATED, HW_IND_TE_AI10, te_go_f7_10 },

	{ ST_TE_F3_PENDING_DEACTIVATION, MPH_RESET_REQ, te_cmd_res },
	{ ST_TE_F3_PENDING_DEACTIVATION, HW_IND_TE_RES, te_go_reset },
	{ ST_TE_F3_PENDING_DEACTIVATION, HW_IND_TE_DC, te_go_f3_deactivated },
	{ ST_TE_F3_PENDING_DEACTIVATION, HW_IND_TE_PU, te_go_f3_power_up },
	{ ST_TE_F3_PENDING_DEACTIVATION, HW_IND_TE_AR, te_go_f6 },
	{ ST_TE_F3_PENDING_DEACTIVATION, HW_IND_TE_AI8, te_go_f7_8 },
	{ ST_TE_F3_PENDING_DEACTIVATION, HW_IND_TE_AI10, te_go_f7_10 },

	{ ST_TE_F3_POWER_UP, MPH_RESET_REQ, te_cmd_res },
	{ ST_TE_F3_POWER_UP, HW_IND_TE_RES, te_go_reset },
	{ ST_TE_F3_POWER_UP, PH_ACT_REQ, te_cmd_ar },
	{ ST_TE_F3_POWER_UP, HW_IND_TE_DC, te_go_f3_deactivated },
	{ ST_TE_F3_POWER_UP, HW_IND_TE_AI8, te_go_f7_8 },
	{ ST_TE_F3_POWER_UP, HW_IND_TE_AI10, te_go_f7_10 },

	{ ST_TE_F4_PENDING_ACTIVATION, MPH_RESET_REQ, te_cmd_res },
	{ ST_TE_F4_PENDING_ACTIVATION, HW_IND_TE_RES, te_go_reset_ph },
	{ ST_TE_F4_PENDING_ACTIVATION, HW_IND_TE_DC, te_go_f3_deactivated_ph },
	{ ST_TE_F4_PENDING_ACTIVATION, HW_IND_TE_PU, te_go_f3_power_up_ph },
	{ ST_TE_F4_PENDING_ACTIVATION, HW_IND_TE_RSY, te_go_f5 },
	{ ST_TE_F4_PENDING_ACTIVATION, EXP_TIMER3, te_cmd_di },

	{ ST_TE_F5_UNSYNCHRONIZED, MPH_RESET_REQ, te_cmd_res },
	{ ST_TE_F5_UNSYNCHRONIZED, HW_IND_TE_RES, te_go_reset_ph },
	{ ST_TE_F5_UNSYNCHRONIZED, HW_IND_TE_DC, te_go_f3_deactivated_ph },
	{ ST_TE_F5_UNSYNCHRONIZED, HW_IND_TE_PU, te_go_f3_power_up_ph },
	{ ST_TE_F5_UNSYNCHRONIZED, HW_IND_TE_AR, te_go_f6 },
	{ ST_TE_F5_UNSYNCHRONIZED, HW_IND_TE_AI8, te_go_f7_8 },
	{ ST_TE_F5_UNSYNCHRONIZED, HW_IND_TE_AI10, te_go_f7_10 },
	{ ST_TE_F5_UNSYNCHRONIZED, EXP_TIMER3, te_cmd_di },

	{ ST_TE_F6_SYNCHRONIZED, MPH_RESET_REQ, te_cmd_res },
	{ ST_TE_F6_SYNCHRONIZED, HW_IND_TE_RES, te_go_reset_ph },
	{ ST_TE_F6_SYNCHRONIZED, HW_IND_TE_DC, te_go_f3_deactivated_ph },
	{ ST_TE_F6_SYNCHRONIZED, HW_IND_TE_DR6, te_go_f3_pending_deactivation },
	{ ST_TE_F6_SYNCHRONIZED, HW_IND_TE_AI8, te_go_f7_8 },
	{ ST_TE_F6_SYNCHRONIZED, HW_IND_TE_AI10, te_go_f7_10 },
	{ ST_TE_F6_SYNCHRONIZED, HW_IND_TE_RSY, te_go_f8 },
	{ ST_TE_F6_SYNCHRONIZED, EXP_TIMER3, te_cmd_di },

	{ ST_TE_F7_ACTIVATED, MPH_RESET_REQ, te_cmd_res },
	{ ST_TE_F7_ACTIVATED, HW_IND_TE_RES, te_go_reset_ph },
	{ ST_TE_F7_ACTIVATED, HW_IND_TE_DC, te_go_f3_deactivated_ph },
	{ ST_TE_F7_ACTIVATED, HW_IND_TE_DR, te_go_f3_pending_deactivation },
	{ ST_TE_F7_ACTIVATED, HW_IND_TE_AR, te_go_f6 },
	{ ST_TE_F7_ACTIVATED, HW_IND_TE_RSY, te_go_f8 },

	{ ST_TE_F8_LOST_FRAMING, MPH_RESET_REQ, te_cmd_res },
	{ ST_TE_F8_LOST_FRAMING, HW_IND_TE_RES, te_go_reset },
	{ ST_TE_F8_LOST_FRAMING, HW_IND_TE_DR, te_go_f3_pending_deactivation },
	{ ST_TE_F8_LOST_FRAMING, HW_IND_TE_DC, te_go_f3_deactivated_ph },
	{ ST_TE_F8_LOST_FRAMING, HW_IND_TE_PU, te_go_f3_power_up_ph },
	{ ST_TE_F8_LOST_FRAMING, HW_IND_TE_AR, te_go_f6 },
	{ ST_TE_F8_LOST_FRAMING, HW_IND_TE_AI8, te_go_f7_8 },
	{ ST_TE_F8_LOST_FRAMING, HW_IND_TE_AI10, te_go_f7_10 },
	{ ST_TE_F8_LOST_FRAMING, EXP_TIMER3, te_cmd_di },

	{ ST_TE_TEST_MODE_SSP, MPH_RESET_REQ, te_cmd_res },
	{ ST_TE_TEST_MODE_SSP, HW_IND_TE_RES, te_go_reset },
	{ ST_TE_TEST_MODE_SSP, HW_IND_TE_DC, te_go_f3_deactivated },

	{ ST_TE_TEST_MODE_SCP, MPH_RESET_REQ, te_cmd_res },
	{ ST_TE_TEST_MODE_SCP, HW_IND_TE_RES, te_go_reset },
	{ ST_TE_TEST_MODE_SCP, HW_IND_TE_DC, te_go_f3_deactivated },

	{ ST_TE_LOOP_A_ACTIVATED, MPH_RESET_REQ, te_cmd_res },
	{ ST_TE_LOOP_A_ACTIVATED, HW_IND_TE_RES, te_go_reset },
	{ ST_TE_LOOP_A_ACTIVATED, HW_IND_TE_DC, te_go_f3_deactivated },

	{ ST_TE_LOOP_A_CLOSED, MPH_RESET_REQ, te_cmd_res },
	{ ST_TE_LOOP_A_CLOSED, HW_IND_TE_RES, te_go_reset },
	{ ST_TE_LOOP_A_CLOSED, HW_IND_TE_DC, te_go_f3_deactivated },
};

static void l1m_debug(struct FsmInst *fi, char *fmt, ...)
{
	struct isdnl1 *isdnl1 = fi->userdata;
	va_list args;
	char buf[256];

	va_start(args, fmt);
	vsprintf(buf, fmt, args);
	va_end(args);

	dev_info(&isdnl1->sdev->dev, "L1: %s\n", buf);
}

int l1_init()
{
	int err;

	l1fsm_lts.state_count = ST_LTS_COUNT;
	l1fsm_lts.event_count = EVENT_COUNT;
	l1fsm_lts.strState = lts_state_names;
	l1fsm_lts.strEvent = l1_event_names;

	l1fsm_te.state_count = ST_TE_COUNT;
	l1fsm_te.event_count = EVENT_COUNT;
	l1fsm_te.strState = te_state_names;
	l1fsm_te.strEvent = l1_event_names;

	err = FsmNew(&l1fsm_lts, lts_nodes, ARRAY_SIZE(lts_nodes));
	if (err)
		return err;

	err = FsmNew(&l1fsm_te, te_nodes, ARRAY_SIZE(te_nodes));
	if (err)
		goto err_free_lts;

	return 0;

err_free_lts:
	FsmFree(&l1fsm_lts);

	return err;
}

void l1_exit()
{
	FsmFree(&l1fsm_lts);
	FsmFree(&l1fsm_te);
}

int l1_setup(struct FsmInst *fsm, struct FsmTimer *timer, struct isdnl1 *isdnl1)
{
	if (test_bit(ISDNL1_MODE_LTS, &isdnl1->flags)) {
		fsm->fsm = &l1fsm_lts;
		fsm->state = ST_LTS_RESET;
	}

	if (test_bit(ISDNL1_MODE_TE, &isdnl1->flags) ||
	    test_bit(ISDNL1_MODE_LTT, &isdnl1->flags)) {
		fsm->fsm = &l1fsm_te;
		fsm->state = ST_TE_RESET;
	}

	fsm->debug = 1;
	fsm->userdata = isdnl1;
	fsm->printdebug = l1m_debug;
	FsmInitTimer(fsm, timer);

	return 0;
}

