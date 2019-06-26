#ifndef _DRV_MEI_CPE_ATMOAM_COMMON_H
#define _DRV_MEI_CPE_ATMOAM_COMMON_H
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

#if (MEI_DRV_ATM_OAM_ENABLE == 1)

#include "ifx_types.h"
#include "drv_mei_cpe_os.h"

#include "drv_mei_cpe_interface.h"

#include "drv_mei_cpe_atmoam.h"
#include "drv_mei_cpe_api.h"
#include "cmv_message_format.h"


/* ==========================================================================
   Gloabl functions: ATM OAM
   ========================================================================== */

extern IFX_int32_t MEI_AtmOamReleaseDevCntrl(
                              MEI_DEV_T       *pMeiDev);

extern IFX_int32_t MEI_AtmOamControlEnable(
                              MEI_DYN_CNTRL_T *pMeiDynCntrl,
                              MEI_DEV_T       *pMeiDev);

extern IFX_int32_t MEI_ATMOAM_IoctlDrvInit(
                              MEI_DYN_CNTRL_T         *pMeiDynCntrl,
                              IOCTL_MEI_ATMOAM_init_t *pAtmOamInit);

extern IFX_int32_t MEI_ATMOAM_IoctlCntrl(
                              MEI_DYN_CNTRL_T           *pMeiDynCntrl,
                              IOCTL_MEI_ATMOAM_cntrl_t  *pAtmOamCntrl);

extern IFX_int32_t MEI_ATMOAM_IoctlCounterGet(
                              MEI_DYN_CNTRL_T            *pMeiDynCntrl,
                              IOCTL_MEI_ATMOAM_counter_t *pAtmOamStats);

extern IFX_int32_t MEI_ATMOAM_IoctlStatusGet(
                              MEI_DYN_CNTRL_T           *pMeiDynCntrl,
                              IOCTL_MEI_ATMOAM_status_t *pAtmOamStatus );

extern IFX_int32_t MEI_ATMOAM_IoctlCellInsert(
                              MEI_DYN_CNTRL_T                *pMeiDynCntrl,
                              IOCTL_MEI_ATMOAM_drvAtmCells_t *pAtmOamCells);

extern IFX_boolean_t MEI_ATMOAM_CheckForWork(
                              MEI_DEV_T              *pMeiDev,
                              MEI_ATMOAM_DEV_CNTRL_T *pAtmOamDevCntrl,
                              IFX_uint16_t             msgId);

extern IFX_int32_t MEI_ATMOAM_AutoMsgHandler(
                              MEI_DEV_T       *pMeiDev,
                              IFX_uint16_t      msgId,
                              CMV_STD_MESSAGE_T *pModemMsg);


extern IFX_int32_t MEI_ATMOAM_ResetControl(
                              MEI_DEV_T       *pMeiDev);

#endif      /* #if (MEI_DRV_ATM_OAM_ENABLE == 1) */

#endif      /* #ifndef _DRV_MEI_CPE_ATMOAM_COMMON_H */

