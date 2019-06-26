#ifndef _drv_MEI_CPE_download_h
#define _drv_MEI_CPE_download_h
/******************************************************************************

                          Copyright (c) 2007-2015
                     Lantiq Beteiligungs-GmbH & Co. KG

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/* ==========================================================================
   Description : Common functions used for FW download via the VRX Driver
   ========================================================================== */

#ifdef __cplusplus
extern "C"
{
#endif

/* ============================================================================
   Inlcudes
   ========================================================================= */
/* get at first the driver configuration */
#include "drv_mei_cpe_config.h"
#include "ifx_types.h"
#include "drv_mei_cpe_os.h"
#include "drv_mei_cpe_interface.h"
#include "drv_mei_cpe_download_vrx_common.h"

/* ==========================================================================
   Global Variable Definitions
   ========================================================================== */

extern MEI_DRVOS_sema_t    pFwDlCntrlLock;

/* ============================================================================
   Macros
   ========================================================================= */

#if (MEI_DRV_OS_BYTE_ORDER == MEI_DRV_OS_BIG_ENDIAN)
   /** Swap 32 bit value - DMA wide byte order swap  */
   #define SWAP32_DMA_WIDE_BYTE_ORDER(x)  ( (((x)&0xFF000000)>>16) \
                                          | (((x)&0x00FF0000)>>16) \
                                          | (((x)&0x0000FF00)<<16) \
                                          | (((x)&0x000000FF)<<16) )

#else
   #define SWAP32_DMA_WIDE_BYTE_ORDER
#endif /* (MEI_DRV_OS_BYTE_ORDER == MEI_DRV_OS_BIG_ENDIAN)*/

#define MEI_BOOT_FLAG 0x80000000
#define MEI_BOOTLOADER_SIZE_PAGE 10


/* ============================================================================
   Global Firmware Download functions
   ========================================================================= */

extern IFX_int32_t MEI_IoctlFirmwareDownload(
                                 MEI_DYN_CNTRL_T        *pMeiDynCntrl,
                                 IOCTL_MEI_fwDownLoad_t *pArgFwDl,
                                 IFX_boolean_t            bInternCall);

extern IFX_int32_t MEI_IoctlOptFirmwareDownload(
                                 MEI_DYN_CNTRL_T           *pMeiDynCntrl,
                                 IOCTL_MEI_fwOptDownLoad_t *pArgFwDl,
                                 IFX_boolean_t             bInternCall);

#ifdef __cplusplus
/* extern "C" */
}
#endif

#endif   /* #ifndef _drv_MEI_CPE_download_h */


