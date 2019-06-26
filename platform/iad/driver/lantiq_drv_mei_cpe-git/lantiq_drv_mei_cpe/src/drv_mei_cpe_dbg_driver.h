#ifndef _DRV_MEI_CPE_DBG_DRIVER_H
#define _DRV_MEI_CPE_DBG_DRIVER_H
/******************************************************************************

                          Copyright (c) 2007-2015
                     Lantiq Beteiligungs-GmbH & Co. KG

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/* ==========================================================================
   Description : This file contains OS specific defines for the VRX driver.

   Remarks     : Please use the compiler switches here if you have
                 more than one OS.
   ========================================================================== */


/* ============================================================================
   Global Includes
   ========================================================================= */
#include "ifx_types.h"
#include "drv_mei_cpe_config.h"
#include "drv_mei_cpe_api.h"

#if (MEI_MSG_DUMP_ENABLE == 1)
#include "cmv_message_format.h"
#endif

/* ============================================================================
   Global Driver Debug Macros
   ========================================================================= */

#if ( (MEI_MSG_DUMP_ENABLE == 1) && (MEI_DEBUG_PRINT == 1) )
#define MEI_DBG_MSG_LOG_DUMP(pMeiDev, pCmvMsg) \
                        MEI_DbgMsgDumpCntrl(pMeiDev, pCmvMsg, IFX_TRUE)
#define MEI_DBG_MSG_TRC_DUMP(pMeiDev, pCmvMsg) \
                        MEI_DbgMsgDumpCntrl(pMeiDev, pCmvMsg, IFX_FALSE)
#else
#define MEI_DBG_MSG_LOG_DUMP(pMeiDev, pCmvMsg)
#define MEI_DBG_MSG_TRC_DUMP(pMeiDev, pCmvMsg)
#endif      /* #if ((MEI_MSG_DUMP_ENABLE == 1) && (MEI_DEBUG_PRINT == 1) ) */


/* ============================================================================
   Global Driver Debug Variables
   ========================================================================= */

#if ( (MEI_MSG_DUMP_ENABLE == 1) && (MEI_DEBUG_PRINT == 1) )
/* VRX-Driver: Msg Dump debug module - declare print level variable */
MEI_DRV_PRN_USR_MODULE_DECL(MEI_MSG_DUMP);
MEI_DRV_PRN_INT_MODULE_DECL(MEI_MSG_DUMP);

extern IFX_uint32_t  MEI_msgDumpEnable;
extern IFX_uint32_t  MEI_msgDumpOutCntrl;
extern IFX_boolean_t MEI_msgDumpSetLabel;
extern IFX_uint16_t  MEI_msgDumpLine;
extern IFX_uint16_t  MEI_msgDumpId;
#endif   /* #if ((MEI_MSG_DUMP_ENABLE == 1) && (MEI_DEBUG_PRINT == 1) ) */



/* ============================================================================
   Global Driver Debug Functions
   ========================================================================= */

#if ( (MEI_MSG_DUMP_ENABLE == 1) && (MEI_DEBUG_PRINT == 1) )
extern IFX_void_t MEI_DbgMsgDumpCntrl(
                              MEI_DEV_T       *pMeiDev,
                              CMV_STD_MESSAGE_T *pCmvMsg,
                              IFX_boolean_t     bLog);
#endif



#if MEI_TRACE_DRV_STATE_CHANGES == 1
extern IFX_void_t MEI_DrvStateSet_Trace(
                              MEI_DEV_T *pMeiDev,
                              MEI_DRV_STATE_E newState);

extern IFX_void_t MEI_ModemStateSet_Trace(
                              MEI_DEV_T *pMeiDev,
                              IFX_uint32_t newState );
#endif

#if MEI_TRACE_MB_STATE_CHANGES == 1
extern IFX_void_t MEI_DrvMailBoxStateSet_Trace(
                              MEI_DEV_T *pMeiDev,
                              MEI_MB_STATE_E newState );
#endif

#if (MEI_SUPPORT_DRV_NFC_DEBUG == 1)
extern IFX_int32_t MEI_ShowNfcData(
                              MEI_DEV_T *pMeiDev,
                              IFX_uint8_t *pBuf,
                              IFX_int32_t bufSize);
#endif

#if (MEI_MISC_TEST == 1)
extern IFX_void_t MEI_StructureCheck(IFX_void_t);
#endif



#endif      /* #ifndef _DRV_MEI_CPE_DBG_DRIVER_H */

