#ifndef _DRV_VMMC_STREAM_H
#define _DRV_VMMC_STREAM_H
/******************************************************************************

                              Copyright (c) 2012
                            Lantiq Deutschland GmbH
                             http://www.lantiq.com

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/**
   \file drv_vmmc_stream.h
   This file contains the definition of the packet streaming.

   \remarks
   Only downstream packet handling is done here. The upstream packet handling
   is done directly in the interrupt handler in drv_vmmc_int.c.
*/

/* ============================= */
/* Includes                      */
/* ============================= */
#include "drv_api.h"
#include "drv_vmmc_api.h"
#include "drv_mps_vmmc.h"

/* ============================= */
/* Global Defines                */
/* ============================= */

/* ============================= */
/* Global Structures             */
/* ============================= */

/* ============================= */
/* Global function declaration   */
/* ============================= */
extern IFX_int32_t VMMC_LL_Open  (IFX_TAPI_LL_CH_t *pLLChDev);
extern IFX_int32_t VMMC_LL_Close (IFX_TAPI_LL_CH_t *pLLChDev);
extern IFX_int32_t VMMC_LL_Write (IFX_TAPI_LL_CH_t *pLLCh, const char *buf,
                                  IFX_int32_t count, IFX_int32_t* ppos,
                                  IFX_TAPI_STREAM_t stream);
extern IFX_int32_t VMMC_MPS_Write (VMMC_CHANNEL *pCh, IFX_TAPI_STREAM_t stream,
                                   mps_message *pMsg);

#endif /* _DRV_VMMC_STREAM_H */
