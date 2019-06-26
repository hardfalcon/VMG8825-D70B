/*
 * (C) Copyright 2005-2017 Sphairon GmbH (a ZyXEL company)
 *
 * SPDX-License-Identifier: GPL-2.0
 */

#ifndef __PCM_CLK_GEN_H__
#define __PCM_CLK_GEN_H__

int pcm_clk_gen_init(bool enable);
void pcm_clk_gen_exit(void);

bool pcm_clk_start(void);

enum pcm_clk_event {
	PCM_CLK_EVENT_SYNC,
	PCM_CLK_EVENT_LOST,
};
typedef void (*pcm_clk_cb)(enum pcm_clk_event event, void *priv);
int pcm_clk_register_cb(struct notifier_block *nb);
int pcm_clk_unregister_cb(struct notifier_block *nb);

#endif /* __PCM_CLK_GEN_H__ */
