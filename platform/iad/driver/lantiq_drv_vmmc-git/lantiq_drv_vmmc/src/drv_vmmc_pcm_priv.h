#ifndef _DRV_VMMC_PCM_PRIV_H
#define _DRV_VMMC_PCM_PRIV_H
/******************************************************************************

                              Copyright (c) 2012
                            Lantiq Deutschland GmbH
                             http://www.lantiq.com

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************
   Module      : drv_vmmc_pcm_priv.h
   Description : This file contains the defines, the structures declarations
                  for PCM module.
*******************************************************************************/

/* includes */
#include "drv_vmmc_fw_commands_voip.h"
#include "drv_vmmc_res.h"

/** Targets of PCM channel usage */
typedef enum {
   /* channel not used */
   VMMC_PCM_TARGET_NONE = 0,
   /* channel used for voice */
   VMMC_PCM_TARGET_VOICE,
   /* channel used for shortcut (loop) */
   VMMC_PCM_TARGET_SHORTCUT,
   /* channel used for HDLC data */
   VMMC_PCM_TARGET_HDLC
} VMMC_PCM_TARGET_t;

/** Reserve PCM channel for some target */
#define VMMC_PCM_RESERVE(pPcmCh, nTarget) (pPcmCh)->nInUse = (nTarget)

/** Release PCM channel of some target */
#define VMMC_PCM_RELEASE(pPcmCh)          (pPcmCh)->nInUse = VMMC_PCM_TARGET_NONE

/** Check PCM channel availability for some targets */
#define VMMC_PCM_AVAIL(pPcmCh, nTarget) \
   (((pPcmCh)->nInUse == VMMC_PCM_TARGET_NONE) || \
    ((pPcmCh)->nInUse == (nTarget)))

/** Structure for the PCM channel
   including firmware message cache */
struct VMMC_PCMCH
{
   PCM_CHAN_t             fw_pcm_ch;
   PCM_SCHAN_t            fw_pcm_sch;
   /* The echo suppressor resource id */
   VMMC_RES_ID_t          nEsResId;
   /* The line echo canceller resource id */
   VMMC_RES_ID_t          nLecResId;
#ifdef VMMC_FEAT_HDLC
   /* The HDLC resource id */
   VMMC_RES_ID_t          nHdlcResId;
#endif /* VMMC_FEAT_HDLC */
   /* represent target of PCM channel usage */
   VMMC_PCM_TARGET_t      nInUse;
};

#endif /* VMMC_PCM_PRIV_H */

