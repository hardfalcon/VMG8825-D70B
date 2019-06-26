#ifndef _DRV_VMMC_PCM_H
#define _DRV_VMMC_PCM_H
/******************************************************************************

                              Copyright (c) 2012
                            Lantiq Deutschland GmbH
                             http://www.lantiq.com

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/**
   \file drv_vmmc_pcm.h
   This file contains the defines and the global functions declarations of
   the PCM module.
*/


#include "drv_vmmc_api.h"
#include "drv_tapi_ll_interface.h"

/* PCM services */
extern IFX_void_t  VMMC_PCM_Func_Register      (IFX_TAPI_DRV_CTX_PCM_t *pPCM);
extern IFX_int32_t VMMC_PCM_Allocate_Ch_Structures (VMMC_CHANNEL *pCh);
extern IFX_void_t  VMMC_PCM_Free_Ch_Structures (VMMC_CHANNEL *pCh);
extern IFX_void_t  VMMC_PCM_InitCh             (VMMC_CHANNEL *pCh, IFX_uint8_t pcmCh);
extern IFX_int32_t VMMC_PCM_Set_Inputs         (VMMC_CHANNEL *pCh);
extern IFX_int32_t VMMC_PCM_ChStop             (VMMC_CHANNEL *pCh);
extern IFX_int32_t VMMC_PCM_IF_Stop            (VMMC_DEVICE *pDev);

extern IFX_int32_t VMMC_PCM_SamplingMode       (VMMC_CHANNEL *pCh,
                                                SM_ACTION action,
                                                OPMODE_SMPL sigarray_mode,
                                                OPMODE_SMPL module_mode);
#ifdef VMMC_FEAT_HDLC
extern IFX_void_t irq_VMMC_PCM_HDLC_BufferReadySet (VMMC_CHANNEL *pCh);
extern IFX_int32_t VMMC_PCM_HDLC_Write          (VMMC_CHANNEL *pCh,
                                                 const IFX_uint8_t *pBuf,
                                                 IFX_int32_t nLen);
#endif /* VMMC_FEAT_HDLC */
#endif
