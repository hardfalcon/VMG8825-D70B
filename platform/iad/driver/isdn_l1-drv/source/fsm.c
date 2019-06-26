/*
 * finite state machine implementation
 *
 * Author       Karsten Keil <kkeil@novell.com>
 *
 * Thanks to    Jan den Ouden
 *              Fritz Elfert
 * Copyright 2008  by Karsten Keil <kkeil@novell.com>
 *
 * SPDX-License-Identifier: GPL-2.0
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/string.h>
#include "fsm.h"

#if 0
#define FSM_TIMER_DEBUG 0
#endif

int FsmNew(struct Fsm *fsm, const struct FsmNode *fnlist, int fncount)
{
	int i;

	fsm->jumpmatrix = kzalloc(sizeof(FSMFNPTR) * fsm->state_count *
		fsm->event_count, GFP_KERNEL);

	if (!fsm->jumpmatrix)
		return -ENOMEM;

	for (i = 0; i < fncount; i++) {
		if ((fnlist[i].state >= fsm->state_count) ||
		    (fnlist[i].event >= fsm->event_count)) {
			pr_err("FsmNew Error: %d st(%ld/%ld) ev(%ld/%ld)\n",
			       i, (long)fnlist[i].state, (long)fsm->state_count,
			       (long)fnlist[i].event, (long)fsm->event_count);
		} else {
			fsm->jumpmatrix[fsm->state_count * fnlist[i].event +
				fnlist[i].state] = (FSMFNPTR) fnlist[i].routine;
		}
	}

	return 0;
}

void FsmFree(struct Fsm *fsm)
{
	kfree((void *)fsm->jumpmatrix);
}

int FsmEvent(struct FsmInst *fi, int event, void *arg)
{
	FSMFNPTR r;

	if ((fi->state >= fi->fsm->state_count) ||
	    (event >= fi->fsm->event_count)) {
		pr_err("FsmEvent Error st(%ld/%ld) ev(%d/%ld)\n",
		       (long)fi->state, (long)fi->fsm->state_count, event,
		       (long)fi->fsm->event_count);
		return 1;
	}
	r = fi->fsm->jumpmatrix[fi->fsm->state_count * event + fi->state];
	if (r) {
		if (fi->debug)
			fi->printdebug(fi, "State %s Event %s",
				       fi->fsm->strState[fi->state],
				       fi->fsm->strEvent[event]);
		r(fi, event, arg);
		return 0;
	} else {
		if (fi->debug)
			fi->printdebug(fi, "State %s Event %s no action",
				       fi->fsm->strState[fi->state],
				       fi->fsm->strEvent[event]);
		return 1;
	}
}

void FsmChangeState(struct FsmInst *fi, int newstate)
{
	fi->state = newstate;
	if (fi->debug)
		fi->printdebug(fi, "ChangeState %s",
			       fi->fsm->strState[newstate]);
}

static void FsmExpireTimer(struct FsmTimer *ft)
{
#ifdef FSM_TIMER_DEBUG
	if (ft->fi->debug)
		ft->fi->printdebug(ft->fi, "FsmExpireTimer %lx", (long)ft);
#endif
	FsmEvent(ft->fi, ft->event, ft->arg);
}

void FsmInitTimer(struct FsmInst *fi, struct FsmTimer *ft)
{
	ft->fi = fi;
	ft->tl.function = (void *)FsmExpireTimer;
	ft->tl.data = (long)ft;
#ifdef FSM_TIMER_DEBUG
	if (ft->fi->debug)
		ft->fi->printdebug(ft->fi, "FsmInitTimer %lx", (long)ft);
#endif
	init_timer(&ft->tl);
}

void FsmDelTimer(struct FsmTimer *ft, int where)
{
#ifdef FSM_TIMER_DEBUG
	if (ft->fi->debug)
		ft->fi->printdebug(ft->fi, "FsmDelTimer %lx %d", (long)ft,
				   where);
#endif

	del_timer_sync(&ft->tl);
}

int FsmAddTimer(struct FsmTimer *ft, int millisec, int event, void *arg,
		int where)
{
#ifdef FSM_TIMER_DEBUG
	if (ft->fi->debug)
		ft->fi->printdebug(ft->fi, "FsmAddTimer %lx %d %d",
				   (long)ft, millisec, where);
#endif
	if (timer_pending(&ft->tl)) {
		pr_warn("FsmAddTimer: timer already active!\n");
		ft->fi->printdebug(ft->fi, "FsmAddTimer already active!");
		return -1;
	}
	init_timer(&ft->tl);
	ft->event = event;
	ft->arg = arg;
	ft->tl.expires = jiffies + (millisec * HZ) / 1000;
	add_timer(&ft->tl);
	return 0;
}

void FsmRestartTimer(struct FsmTimer *ft, int millisec, int event, void *arg,
		     int where)
{
#ifdef FSM_TIMER_DEBUG
	if (ft->fi->debug)
		ft->fi->printdebug(ft->fi, "FsmRestartTimer %lx %d %d",
				   (long)ft, millisec, where);
#endif
	if (timer_pending(&ft->tl))
		del_timer_sync(&ft->tl);
	init_timer(&ft->tl);
	ft->event = event;
	ft->arg = arg;
	ft->tl.expires = jiffies + (millisec * HZ) / 1000;
	add_timer(&ft->tl);
}
