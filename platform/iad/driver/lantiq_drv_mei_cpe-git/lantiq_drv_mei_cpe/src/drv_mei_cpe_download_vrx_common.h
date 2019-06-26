/******************************************************************************

                          Copyright (c) 2007-2015
                     Lantiq Beteiligungs-GmbH & Co. KG

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/
#ifndef _DRV_MEI_CPE_DOWNLOAD_VRX_COMMON_H
#define _DRV_MEI_CPE_DOWNLOAD_VRX_COMMON_H

/* ==========================================================================
   Description : VR9/AR9 Firmware Download definitions .
   ========================================================================== */

#ifdef __cplusplus
extern "C"
{
#endif

/* get config */
#include "drv_mei_cpe_config.h"

/* ============================================================================
   Inlcudes
   ========================================================================= */

#include "ifx_types.h"
#include "drv_mei_cpe_api.h"


/* ============================================================================
   Macros
   ========================================================================= */

/* ============================================================================
   Exports
   ========================================================================= */

extern IFX_void_t MEI_DEV_FirmwareDownloadResourcesRelease(
                                 MEI_DEV_T       *pMeiDev);

extern IFX_int32_t MEI_DEV_IoctlFirmwareDownload(
                                 MEI_DYN_CNTRL_T        *pMeiDynCntrl,
                                 IOCTL_MEI_fwDownLoad_t *pArgFwDl,
                                 IFX_boolean_t            bInternCall);

extern IFX_int32_t MEI_IoctlFwModeCtrlSet(
                                 MEI_DYN_CNTRL_T           *pMeiDynCntrl,
                                 IOCTL_MEI_FwModeCtrlSet_t *pArgFwModeCtrl);

extern IFX_int32_t MEI_IoctlFwModeStatGet(
                                 MEI_DYN_CNTRL_T           *pMeiDynCntrl,
                                 IOCTL_MEI_FwModeStatGet_t *pArgFwModeStat);

#ifdef __cplusplus
/* extern "C" */
}
#endif

#endif   /* #ifndef _DRV_MEI_CPE_DOWNLOAD_VRX_COMMON_H */


