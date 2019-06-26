/******************************************************************************

                          Copyright (c) 2007-2015
                     Lantiq Beteiligungs-GmbH & Co. KG

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/
#ifndef _DRV_MEI_CPE_DSM_H
#define _DRV_MEI_CPE_DSM_H

/* ==========================================================================
   Description : DSM Vectoring definitions.
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

extern MEI_DRVOS_sema_t pCallBackFuncAccessLock;

extern int32_t mei_dsm_cb_func(uint32_t *p_error_vector);

extern IFX_int32_t MEI_VRX_DSM_ErbAlloc(
                  MEI_DEV_T    *pMeiDev,
                  IFX_uint32_t erb_buf_size);

extern IFX_void_t MEI_VRX_DSM_ErbFree(
                  MEI_DEV_T *pMeiDev);

extern IFX_void_t MEI_VRX_DSM_DataInit(
                  MEI_DEV_T *pMeiDev);

extern IFX_int32_t MEI_VRX_DSM_ControlSet(
                  MEI_DYN_CNTRL_T *pMeiDynCntrl,
                  IOCTL_MEI_dsmConfig_t *pDsmConfig);

extern IFX_int32_t MEI_VRX_DSM_StatusGet(
                  MEI_DYN_CNTRL_T *pMeiDynCntrl,
                  IOCTL_MEI_dsmStatus_t *pDsmStatus);

extern IFX_int32_t MEI_VRX_DSM_MacConfigSet(
                  MEI_DYN_CNTRL_T *pMeiDynCntrl,
                  IOCTL_MEI_MacConfig_t *pMacConfig);

extern IFX_int32_t MEI_VRX_DSM_StatsGet(
                  MEI_DYN_CNTRL_T *pMeiDynCntrl,
                  IOCTL_MEI_dsmStatistics_t *pDsmStatistics);

extern IFX_void_t MEI_VRX_DSM_FwStatsCheck(
                  MEI_DYN_CNTRL_T *pMeiDynCntrl);

extern IFX_void_t MEI_VRX_DSM_FwStatsUpdate(
                  MEI_DYN_CNTRL_T *pMeiDynCntrl,
                  IOCTL_MEI_dsmStatistics_t *pDsmStatistics);

extern IFX_int32_t MEI_VRX_DSM_FwConfigSet(
                  MEI_DYN_CNTRL_T *pMeiDynCntrl);

#if MEI_SUPPORT_DEVICE_VR11 != 1
extern IFX_void_t MEI_VRX_DSM_EvtErbHandler(
                  MEI_DEV_T *pMeiDev,
                  EVT_DSM_ErrorVectorReady_t *pDsmErbParams);
#endif /* (MEI_SUPPORT_DEVICE_VR11 != 1) */

#if (MEI_DBG_DSM_PROFILING == 1)
extern IFX_uint32_t MEI_Count0_read(MEI_DEV_T *pMeiDev);

extern IFX_void_t MEI_VRX_DSM_DbgTestProfiling(MEI_DEV_T *pMeiDev);

extern IFX_void_t MEI_VRX_DSM_DbgPrintProfiling(MEI_DEV_T *pMeiDev);

extern IFX_void_t MEI_VRX_DSM_DbgUpdateProfiling(
                  MEI_DEV_T *pMeiDev,
                  u64 count_start,
                  u64 count_end);
#endif


#ifdef __cplusplus
/* extern "C" */
}
#endif

#endif   /* #ifndef _DRV_MEI_CPE_DSM_H */


