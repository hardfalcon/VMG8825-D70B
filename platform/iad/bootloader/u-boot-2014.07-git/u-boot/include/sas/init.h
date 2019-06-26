/*
 * Copyright (C) 2011-2015 Sphairon GmbH (a ZyXEL company)
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __SAS_INIT_H__
#define __SAS_INIT_H__

int sas_init_f(void);
int sas_early_init_r(void);
int sas_late_init_r(void);

int sas_board_init_f(void);
int sas_board_early_init_r(void);
int sas_board_late_init_r(void);

#endif /* __SAS_INIT_H__ */
