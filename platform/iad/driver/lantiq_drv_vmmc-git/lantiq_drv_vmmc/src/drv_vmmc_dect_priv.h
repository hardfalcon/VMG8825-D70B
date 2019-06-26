#ifndef _DRV_VMMC_DECT_PRIV_H
#define _DRV_VMMC_DECT_PRIV_H
/******************************************************************************

                              Copyright (c) 2012
                            Lantiq Deutschland GmbH
                             http://www.lantiq.com

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************
   Module      : drv_vmmc_dect_priv.h
   Description : This file contains the defines and structures declarations
                 of DECT module
*******************************************************************************/

/* ============================= */
/* Includes                      */
/* ============================= */
#include "drv_vmmc_fw_commands_voip.h"
#include "drv_vmmc_res.h"

/* ============================= */
/* Global Defines                */
/* ============================= */

/* ============================= */
/* Global Types                  */
/* ============================= */

/** Structure for the DECT channel including firmware message cache */
struct VMMC_DECTCH
{
   /** firmware message cache */
   DECT_CHAN_SPEECH_t           fw_dect_ch_speech;
   DECT_CODER_STAT_t            fw_dect_coder_stat;
   DECT_UTG_CTRL_t              fw_dect_utg_ctrl;
   RES_UTG_COEF_t               fw_dect_utg_coef;
   /* The echo suppressor resource id */
   VMMC_RES_ID_t                nEsResId;
};

/* ============================= */
/* Global Variables              */
/* ============================= */

/* ============================= */
/* Global function declaration   */
/* ============================= */

#endif /* _DRV_VMMC_DECT_PRIV_H */
