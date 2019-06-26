#ifndef _DRV_VMMC_INIT_H
#define _DRV_VMMC_INIT_H
/******************************************************************************

                              Copyright (c) 2014
                            Lantiq Deutschland GmbH
                             http://www.lantiq.com

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************
   Module      : drv_vmmc_init.h
*******************************************************************************/

/* ============================= */
/* Includes                      */
/* ============================= */
#include "drv_vmmc_api.h"
#ifdef VMMC_FEAT_LINUX_PLATFORM_DRIVER
#include <linux/platform_device.h>
#endif /* VMMC_FEAT_LINUX_PLATFORM_DRIVER */

/* ============================= */
/* Global Defines                */
/* ============================= */

/* ============================= */
/* Global Types                  */
/* ============================= */

/* ============================= */
/* Global variable declaration   */
/* ============================= */
extern const IFX_char_t DRV_VMMC_WHATVERSION[];
#ifdef HAVE_CONFIG_H
extern const IFX_char_t DRV_VMMC_WHICHCONFIG[];
#endif /* HAVE_CONFIG_H */

/* ============================= */
/* Global function declaration   */
/* ============================= */

extern IFX_int32_t   VMMC_ChipAccessInit (VMMC_DEVICE *pDev);

extern IFX_int32_t  TAPI_LL_Phone_Get_Capabilities (IFX_TAPI_LL_DEV_t *pLLDev);
extern IFX_return_t  TAPI_LL_Phone_Get_Capability_List (
   IFX_TAPI_LL_DEV_t *pLLDev,
   IFX_TAPI_CAP_LIST_t *pCapList);

extern IFX_int32_t   TAPI_LL_Phone_Check_Capability (IFX_TAPI_LL_DEV_t *pLLDev,
                                                     IFX_TAPI_CAP_t *pCapList);
extern IFX_int32_t   VMMC_AddCaps (VMMC_DEVICE *pDev);

extern IFX_int32_t   VMMC_GetDevice (IFX_uint16_t nr, VMMC_DEVICE** pDev);

extern IFX_return_t  VMMC_Get_FwCap (VMMC_DEVICE *pDev);

extern IFX_int32_t   VMMC_DeviceDriverStart(void);

extern IFX_void_t    VMMC_DeviceDriverStop(void);

#endif /* _DRV_VMMC_INIT_H */
