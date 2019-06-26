#ifndef _DRV_MEI_CPE_CLEAR_EOC_COMMON_H
#define _DRV_MEI_CPE_CLEAR_EOC_COMMON_H
/******************************************************************************

                          Copyright (c) 2007-2015
                     Lantiq Beteiligungs-GmbH & Co. KG

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/* ==========================================================================
   Description : common interface functions and definition for ATM OAM.
   ========================================================================== */

/* ==========================================================================
   includes
   ========================================================================== */

/* get at first the driver configuration */
#include "drv_mei_cpe_config.h"

#if (MEI_DRV_CLEAR_EOC_ENABLE == 1)

#include "ifx_types.h"
#include "drv_mei_cpe_os.h"

#include "drv_mei_cpe_interface.h"

#include "drv_mei_cpe_clear_eoc.h"
#include "drv_mei_cpe_api.h"
#include "cmv_message_format.h"


/* ==========================================================================
   Gloabl functions: Clear EOC
   ========================================================================== */

extern IFX_int32_t MEI_CEOC_ReleaseDevCntrl(
                              MEI_DEV_T       *pMeiDev);

extern IFX_int32_t MEI_CEOC_ResetControl(
                              MEI_DEV_T       *pMeiDev);

extern IFX_boolean_t MEI_CEOC_CheckForWork(
                              MEI_DEV_T            *pMeiDev,
                              MEI_CEOC_DEV_CNTRL_T *pCEocDevCntrl,
                              IFX_uint16_t           msgId);

extern IFX_int32_t MEI_CEOC_AutoMsgHandler(
                              MEI_DEV_T       *pMeiDev,
                              CMV_STD_MESSAGE_T *pModemMsg);

extern IFX_int32_t MEI_CEOC_IoctlDrvInit(
                              MEI_DYN_CNTRL_T       *pMeiDynCntrl,
                              IOCTL_MEI_CEOC_init_t *pIoctlCEocInit);

extern IFX_int32_t MEI_CEOC_IoctlCntrl(
                              MEI_DYN_CNTRL_T        *pMeiDynCntrl,
                              IOCTL_MEI_CEOC_cntrl_t *pIoctlCEocCntrl);

extern IFX_int32_t MEI_CEOC_IoctlStatusGet(
                              MEI_DYN_CNTRL_T            *pMeiDynCntrl,
                              IOCTL_MEI_CEOC_statistic_t *pIoctlCEocStats);

extern IFX_int32_t MEI_CEOC_IoctlFrameWrite(
                              MEI_DYN_CNTRL_T        *pMeiDynCntrl,
                              IOCTL_MEI_CEOC_frame_t *pIoctlCEocFrame,
                              IFX_boolean_t            bInternCall);

extern IFX_int32_t MEI_CEOC_IoctlFrameRead(
                              MEI_DYN_CNTRL_T        *pMeiDynCntrl,
                              IOCTL_MEI_CEOC_frame_t *pIoctlCEocFrame,
                              IFX_boolean_t            bInternCall);


#endif      /* #if (MEI_DRV_CLEAR_EOC_ENABLE == 1) */

#endif      /* #ifndef _DRV_MEI_CPE_CLEAR_EOC_COMMON_H */

