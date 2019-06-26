#ifndef _DRV_VMMC_PMC_H
#define _DRV_VMMC_PMC_H
/******************************************************************************

                              Copyright (c) 2012
                            Lantiq Deutschland GmbH
                             http://www.lantiq.com

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/**
   \file drv_vmmc_pmc.h
   Declares the interface towards the system power management control unit.
*/

/* ============================= */
/* Includes                      */
/* ============================= */
#include "drv_vmmc_api.h"
#include "drv_mps_vmmc.h"

/* ============================= */
/* Global Defines                */
/* ============================= */

/* ============================= */
/* Global Types                  */
/* ============================= */
typedef struct VMMC_PMC_CHANNEL_BITS
{
   IFX_uint32_t   pcm_ch : 1;
   IFX_uint32_t   pcm_lec : 1;
   IFX_uint32_t   pcm_es : 1;
   IFX_uint32_t   pcm_lb : 1;
   IFX_uint32_t   hdlc_ch : 1;
   IFX_uint32_t   alm_lec : 1;
   IFX_uint32_t   alm_es : 1;
   IFX_uint32_t   cod_ch : 1;
   IFX_uint32_t   cod_agc : 1;
   IFX_uint32_t   cod_fdp : 1;
   IFX_uint32_t   sig_ch : 1;
   IFX_uint32_t   sig_dtmfd : 1;
   IFX_uint32_t   sig_dtmfg : 1;
   IFX_uint32_t   sig_fskd : 1;
   IFX_uint32_t   sig_fskg : 1;
   IFX_uint32_t   sig_mftd : 1;
   IFX_uint32_t   sig_utg1 : 1;
   IFX_uint32_t   sig_utg2 : 1;
   IFX_uint32_t   sig_cptd : 1;
   IFX_uint32_t   dect_ch : 1;
   IFX_uint32_t   dect_utg : 1;
} VMMC_PMC_CHANNEL_BITS_t;

union VMMC_PMC_CHANNEL
{
   IFX_uint32_t  value;
   VMMC_PMC_CHANNEL_BITS_t  bits;
};

/* ============================= */
/* Global variable declaration   */
/* ============================= */

/* ============================= */
/* Global function declaration   */
/* ============================= */
extern IFX_int32_t VMMC_PMC_OnDriverStart(
                        void);

extern IFX_int32_t VMMC_PMC_OnDriverStop(
                        void);

extern IFX_int32_t VMMC_PMC_Init(
                        VMMC_DEVICE *pDev);

extern IFX_void_t  VMMC_PMC_Exit(
                        VMMC_DEVICE *pDev);

extern IFX_int32_t VMMC_PMC_baseConf(
                        VMMC_DEVICE *pDev);

extern IFX_int32_t VMMC_PMC_Write(
                        VMMC_DEVICE *pDev,
                        mps_message *pMsg);

extern IFX_int32_t VMMC_PMC_FwBoot(
                        VMMC_DEVICE *pDev,
                        IFX_boolean_t bBoot);

#ifdef VMMC_FEAT_SLIC
extern IFX_void_t  irq_VMMC_PMC_DartWakeupReqEvent(
                        VMMC_DEVICE *pDev);

extern IFX_void_t  irq_VMMC_PMC_DartInSleepEvent(
                        VMMC_DEVICE *pDev);
#endif /* VMMC_FEAT_SLIC */

extern IFX_void_t  VMMC_PMC_ClockStateGet(
                        IFX_int32_t *pClockState,
                        IFX_int32_t *pRequestedClockState);

#endif /* _DRV_VMMC_PMC_H */
