#ifndef _DRV_VMMC_BBD_H
#define _DRV_VMMC_BBD_H
/******************************************************************************

                              Copyright (c) 2012
                            Lantiq Deutschland GmbH
                             http://www.lantiq.com

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

*******************************************************************************/

/**
   \file drv_vmmc_bbd.h
*/

/* ============================= */
/* Includes                      */
/* ============================= */

#include "lib_bbd.h"
#include "drv_vmmc_fw_commands_sdd.h"

/* ============================= */
/* Global Defines                */
/* ============================= */

/* ============================= */
/* Global Structures             */
/* ============================= */


/* ============================= */
/* Global function declaration   */
/* ============================= */

extern IFX_int32_t VMMC_TAPI_LL_BBD_Dnld(
                        IFX_TAPI_LL_DEV_t *pLLDev,
                                         IFX_void_t const *pProc);

extern IFX_int32_t VMMC_BBD_Download(
                        VMMC_CHANNEL *pCh,
                        bbd_format_t *pBBD);

#endif /* _DRV_VMMC_BBD_H */
