#ifndef _DRV_VMMC_ANNOUNCEMENTS_H
#define _DRV_VMMC_ANNOUNCEMENTS_H
/******************************************************************************

                              Copyright (c) 2012
                            Lantiq Deutschland GmbH
                             http://www.lantiq.com

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

*******************************************************************************/

/**
   \file drv_vmmc_announcements.h
   Interface of the Low-level TAPI announcements implementation.
   This file contains the declaration of the functions for the announcements
   module.
*/

#ifdef VMMC_FEAT_ANNOUNCEMENTS

/* ============================= */
/* Global Defines                */
/* ============================= */

/* maximum number of announcements */
#define VMMC_ANNOUNCEMENTS_MAX      256

/* maximum size of one announcement (64 kB) */
#define VMMC_ANNOUNCEMENTS_SIZE_MAX 65536

/* ============================= */
/* Global Types                  */
/* ============================= */

/* ============================= */
/* Global function declaration   */
/* ============================= */

extern IFX_int32_t VMMC_Ann_Init (VMMC_DEVICE* pDev);
extern IFX_void_t VMMC_Ann_Cleanup (VMMC_DEVICE* pDev,
                                    IFX_boolean_t bChipAccess);

extern IFX_void_t VMMC_AnnEndEventServe(VMMC_CHANNEL* pCh);

extern IFX_int32_t IFX_TAPI_LL_Ann_Cfg (
   IFX_TAPI_LL_CH_t *pLLCh,
   IFX_TAPI_COD_ANNOUNCE_CFG_t const *pCfg);

extern IFX_int32_t IFX_TAPI_LL_Ann_Start (
   IFX_TAPI_LL_CH_t *pLLCh,
   IFX_TAPI_COD_ANNOUNCE_START_t const *pStart);

extern IFX_int32_t IFX_TAPI_LL_Ann_Stop (IFX_TAPI_LL_CH_t *pLLCh);

extern IFX_int32_t IFX_TAPI_LL_Ann_Free (
   IFX_TAPI_LL_CH_t *pLLCh,
   IFX_TAPI_COD_ANNOUNCE_BUFFER_FREE_t const *pFree);

#endif /* VMMC_FEAT_ANNOUNCEMENTS */

#endif /* _DRV_VMMC_ANNOUNCEMENTS_H */
