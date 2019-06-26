#ifndef _DRV_VMMC_SIG_H
#define _DRV_VMMC_SIG_H
/******************************************************************************

                              Copyright (c) 2012
                            Lantiq Deutschland GmbH
                             http://www.lantiq.com

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/*
   \file drv_vmmc_sig.h
   This file contains the declaration of the functions for the SIGnalling
   module.
*/

/* ============================= */
/* Includes                      */
/* ============================= */

#include "ifx_types.h"
#include "drv_vmmc_api.h"
#include "drv_vmmc_int_evt.h"
#include "drv_tapi_ll_interface.h"

/* ============================= */
/* Global Defines                */
/* ============================= */

/* ============================= */
/* Local Macros & Definitions    */
/* ============================= */

/* ============================= */
/* Global Structures             */
/* ============================= */

/* ============================= */
/* Global function declaration   */
/* ============================= */

extern IFX_void_t   VMMC_SIG_Func_Register (IFX_TAPI_DRV_CTX_SIG_t *pSig);

extern IFX_int32_t  VMMC_SIG_Allocate_Ch_Structures (VMMC_CHANNEL *pCh);
extern IFX_void_t   VMMC_SIG_Free_Ch_Structures (VMMC_CHANNEL *pCh);

extern IFX_void_t   VMMC_SIG_InitCh (VMMC_CHANNEL *pCh);
extern IFX_int32_t  VMMC_SIG_Base_Conf (VMMC_CHANNEL *pCh);
extern IFX_int32_t  VMMC_SIG_Set_Inputs (VMMC_CHANNEL *pCh);
extern IFX_int32_t  VMMC_SIG_ChStop (VMMC_CHANNEL *pCh);

extern IFX_int32_t  VMMC_SIG_SamplingMode (VMMC_CHANNEL *pCh,
                                           SM_ACTION action,
                                           OPMODE_SMPL mode);

extern IFX_int32_t  VMMC_SIG_RTP_OOB_Cfg (VMMC_CHANNEL *pCh,
                                   IFX_TAPI_PKT_RTP_CFG_t const *pRtpConf);

extern IFX_int32_t  VMMC_SIG_Event_Stat_Get (VMMC_CHANNEL *pCh,
                                             IFX_uint32_t *ReceivedBytesLow,
                                             IFX_uint32_t *ReceivedBytesHigh);

extern IFX_int32_t  VMMC_SIG_Event_Stat_Reset (VMMC_CHANNEL *pCh);


extern IFX_int32_t  VMMC_SIG_UpdateEventTrans (VMMC_CHANNEL *pCh,
                                               IFX_boolean_t bCod);

extern IFX_int32_t VMMC_SIG_UTG_SetCoeff  (IFX_TAPI_TONE_SIMPLE_t const *pTone,
                                           RES_UTG_COEF_t *pCmd);

/* Functions to serve interrupt events */

extern IFX_int32_t  irq_VMMC_SIG_DtmfOnReady    (VMMC_CHANNEL *pCh);
extern IFX_int32_t  irq_VMMC_SIG_DtmfOnRequest  (VMMC_CHANNEL *pCh);
extern IFX_int32_t  irq_VMMC_SIG_DtmfOnUnderrun (VMMC_CHANNEL *pCh);
extern IFX_int32_t  irq_VMMC_SIG_DtmfStop       (VMMC_CHANNEL *pCh);
extern void         irq_VMMC_SIG_MFTD_Event     (VMMC_CHANNEL *pCh,
                                                 IFX_uint8_t nVal,
                                                 IFX_boolean_t bRx);
extern IFX_uint8_t  irq_VMMC_SIG_DTMF_encode_fw2tapi (IFX_uint8_t fwDtmfCode);
extern IFX_char_t   irq_VMMC_SIG_DTMF_encode_fw2ascii (IFX_uint8_t fwDtmfCode);

/* Prototypes for functions which are exported via the function
   VINETIC_SIG_Func_Register(). The prototype for the function pointers are
   defined in drv_tapi_ll_interface.h and must be identical to the prototypes
   below. */

extern IFX_int32_t VMMC_TAPI_LL_SIG_UTG_Start  (IFX_TAPI_LL_CH_t *pLLChannel,
                                               IFX_TAPI_TONE_SIMPLE_t const *pSimpleTone,
                                               TAPI_TONE_DST dst,
                                               IFX_uint8_t res);

extern IFX_int32_t VMMC_TAPI_LL_SIG_UTG_Stop  (IFX_TAPI_LL_CH_t *pLLChannel,
                                              IFX_uint8_t res);

extern IFX_uint8_t VMMC_TAPI_LL_SIG_UTG_Count_Get (IFX_TAPI_LL_CH_t *pLLChannel);

extern IFX_void_t VMMC_TAPI_LL_UTG_Event_Deactivated (IFX_TAPI_LL_CH_t *pLLChannel,
                                                     IFX_uint8_t utgNum);

extern IFX_uint8_t VMMC_SIG_CPTD_ToneFromEvtGet(
   VMMC_CHANNEL *pCh,
   EVT_SIG_CPTD_t *pEvtCPTD);

extern IFX_int32_t VMMC_TAPI_LL_SIG_CPTD_Start (
   IFX_TAPI_LL_CH_t *pLLChannel,
   IFX_TAPI_TONE_SIMPLE_t const *pTone,
   IFX_TAPI_TONE_CPTD_t const *pSignal);

extern IFX_int32_t VMMC_TAPI_LL_SIG_CPTD_Stop  (
   IFX_TAPI_LL_CH_t *pLLChannel,
   IFX_TAPI_TONE_CPTD_t const *pSignal);

#endif /* _DRV_VMMC_SIG_H */
