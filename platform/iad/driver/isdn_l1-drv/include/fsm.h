/*
 *
 * Author       Karsten Keil <kkeil@novell.com>
 *
 * Thanks to    Jan den Ouden
 *              Fritz Elfert
 * Copyright 2008  by Karsten Keil <kkeil@novell.com>
 *
 * SPDX-License-Identifier: GPL-2.0
 */

#ifndef _MISDN_FSM_H
#define _MISDN_FSM_H

#include <linux/timer.h>

/* Statemachine */

struct FsmInst;

typedef void (*FSMFNPTR)(struct FsmInst *, int, void *);

struct Fsm {
	FSMFNPTR *jumpmatrix;
	int state_count, event_count;
	const char **strEvent, **strState;
};

struct FsmInst {
	struct Fsm *fsm;
	int state;
	int debug;
	void *userdata;
	int userint;
	void (*printdebug) (struct FsmInst *, char *, ...);
};

struct FsmNode {
	int state, event;
	void (*routine) (struct FsmInst *, int, void *);
};

struct FsmTimer {
	struct FsmInst *fi;
	struct timer_list tl;
	int event;
	void *arg;
};

int FsmNew(struct Fsm *, const struct FsmNode *, int);
void FsmFree(struct Fsm *);
int FsmEvent(struct FsmInst *, int , void *);
void FsmChangeState(struct FsmInst *, int);
void FsmInitTimer(struct FsmInst *, struct FsmTimer *);
int FsmAddTimer(struct FsmTimer *, int, int, void *, int);
void FsmRestartTimer(struct FsmTimer *, int, int, void *, int);
void FsmDelTimer(struct FsmTimer *, int);

#endif
