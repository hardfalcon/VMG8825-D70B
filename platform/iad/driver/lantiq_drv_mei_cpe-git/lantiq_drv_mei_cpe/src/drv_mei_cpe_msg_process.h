#ifndef _DRV_MEI_CPE_MSG_PROCESS_H
#define _DRV_MEI_CPE_MSG_PROCESS_H
/******************************************************************************

                          Copyright (c) 2007-2015
                     Lantiq Beteiligungs-GmbH & Co. KG

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/* ==========================================================================
   Description : Message Handling between the driver and the VRX device.
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

#include "drv_mei_cpe_api.h"
#include "drv_mei_cpe_mailbox.h"

#if (MEI_SUPPORT_DEVICE_AR9 == 1)
#include "drv_mei_cpe_msg_process_ar9.h"
#endif /* #if (MEI_SUPPORT_DEVICE_AR9 == 1)*/

/* ============================================================================
   Macros
   ========================================================================= */

#if 0
#define MEI_LOG_CMV_MSG(pDFE_DEV, pCMV_MSG, pINFO, DBG_LEVEL)
#else
#define MEI_LOG_CMV_MSG(pDFE_DEV, pCMV_MSG, pINFO, DBG_LEVEL) \
                         MEI_LogCmvMsg( pDFE_DEV, pCMV_MSG, pINFO, DBG_LEVEL )
#endif

#if 0
#define MEI_TRACE_CMV_MSG(pDFE_DEV, pCMV_MSG, pINFO, DBG_LEVEL)
#else
/** Log a CMV message */
#define MEI_TRACE_CMV_MSG(pDFE_DEV, pCMV_MSG, pINFO, DBG_LEVEL) \
                         MEI_TraceCmvMsg( pDFE_DEV, pCMV_MSG, pINFO, DBG_LEVEL )
#endif


/** Control Mask for Autonomous Messages (default) */
#define MEI_MSG_CNTRL_MODEM_MSG_MASK_DEFAULT     0x0000FFFF

/*
   COMPILE check agianst the interface file - the settings must match
*/



/* ============================================================================
   Global Message functions
   ========================================================================= */

extern IFX_void_t MEI_LogCmvMsg( MEI_DEV_T *pMeiDev,
                                   CMV_STD_MESSAGE_T *pCmvMsg,
                                   const char *pDescr, IFX_uint32_t dbgLevel);

extern IFX_void_t MEI_TraceCmvMsg(
                              MEI_DEV_T *pMeiDev,
                              CMV_STD_MESSAGE_T *pCmvMsg,
                              const char *pDescr, IFX_uint32_t dbgLevel);

extern IFX_int32_t MEI_MsgPayl32Swap( MEI_DEV_T *pMeiDev,
                                        CMV_STD_MESSAGE_T *pMsg,
                                        IFX_int32_t msgSize);

#if (MEI_SUPPORT_DRV_LOOPS == 1)
extern IFX_boolean_t MEI_MailboxLoop( MEI_DEV_T *pMeiDev,
                                        IFX_boolean_t loopOn);
#endif

extern IFX_int32_t MEI_WaitForMailbox(
                              MEI_DEV_T *pMeiDev);

extern IFX_int32_t MEI_SetCmvHeader(
                              MEI_DEV_T           *pMeiDev,
                              CMV_STD_MESSAGE_T     *pMsg,
                              IOCTL_MEI_message_t *pUsrMsgWr);

extern IFX_void_t MEI_MsgId2CmvHeader(
                              MEI_DEV_T       *pMeiDev,
                              CMV_STD_MESSAGE_T *pMsg,
                              IFX_uint16_t      msgId);

extern IFX_int32_t MEI_WaitForInstance(
                              MEI_DYN_CNTRL_T    *pMeiDynCntrl,
                              MEI_DEV_T          *pMeiDev,
                              MEI_DYN_CMD_DATA_T *pDynCmd);

extern IFX_int32_t MEI_WriteMailbox(
                              MEI_DYN_CNTRL_T    *pMeiDynCntrl,
                              MEI_DYN_CMD_DATA_T *pDynCmd,
                              MEI_MEI_MAILBOX_T  *pMBMsg,
                              IFX_int32_t          mbMsgSize);

extern IFX_void_t  MEI_ReadMailbox(
                              MEI_DEV_T *pMeiDev);

extern IFX_int32_t MEI_CheckAck(
                              MEI_DYN_CNTRL_T *pMeiDynCntrl);

extern IFX_int32_t MEI_NfcCallBackSet(
                              MEI_DYN_CNTRL_T *pMeiDynCntrl,
                              MEI_InternalMsgRecvCallBack pCallBackFunc,
                              IFX_void_t                    *pNfcCallBackData);

extern IFX_uint32_t MEI_DistributeAutoMsg(
                              MEI_DEV_T          *pMeiDev,
                              MEI_DYN_NFC_DATA_T *pNfcRootInstance,
                              IFX_uint8_t          *pMsg,
                              IFX_int32_t          msgSize,
                              IFX_uint32_t         msgType );

extern IFX_boolean_t MEI_AddNfcToDevList(
                              MEI_DYN_CNTRL_T    *pMeiDynCntrl,
                              MEI_DYN_NFC_DATA_T *pVrxDynNfc);

extern IFX_boolean_t MEI_RemoveNfcFromDevList(
                              MEI_DYN_CNTRL_T    *pMeiDynCntrl,
                              MEI_DYN_NFC_DATA_T **ppVrxDynNfc);

extern IFX_int32_t MEI_WriteMsgAndCheck(
                              MEI_DYN_CNTRL_T    *pMeiDynCntrl,
                              MEI_DYN_CMD_DATA_T *pDynCmd,
                              MEI_MEI_MAILBOX_T  *pMbMsg,
                              IFX_int32_t          modemMsgSize);


extern IFX_int32_t MEI_IoctlNfcEnable(
                              MEI_DYN_CNTRL_T *pMeiDynCntrl,
                              IFX_uint32_t      nfcBufPerInst,
                              IFX_uint32_t      nfcBufSize);

extern IFX_int32_t MEI_IoctlNfcDisable(
                              MEI_DYN_CNTRL_T *pMeiDynCntrl);

extern IFX_int32_t MEI_IoctlAutoMsgCtlSet(
                              MEI_DYN_CNTRL_T         *pMeiDynCntrl,
                              IOCTL_MEI_autoMsgCtrl_t *pArgAutoMsgCtrl);

extern IFX_int32_t MEI_IoctlAutoMsgCtlGet(
                              MEI_DYN_CNTRL_T         *pMeiDynCntrl,
                              IOCTL_MEI_autoMsgCtrl_t *pArgAutoMsgCtrl);

extern IFX_int32_t MEI_IoctlCmdMsgWrite(
                              MEI_DYN_CNTRL_T     *pMeiDynCntrl,
                              IOCTL_MEI_message_t *pUserMsg,
                              IFX_boolean_t         bInternCall);

extern IFX_int32_t MEI_IoctlAckMsgRead(
                              MEI_DYN_CNTRL_T     *pMeiDynCntrl,
                              IOCTL_MEI_message_t *pUserMsg,
                              IFX_boolean_t         bInternCall);

extern IFX_int32_t MEI_IoctlMsgSend(
                              MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                              IOCTL_MEI_messageSend_t  *pUserMsgs,
                              IFX_boolean_t              bInternCall);

extern IFX_int32_t MEI_IoctlNfcMsgRead(
                              MEI_DYN_CNTRL_T       *pMeiDynCntrl,
                              IOCTL_MEI_message_t   *pUserMsg,
                              IFX_boolean_t           bInternCall);

#if (MEI_SUPPORT_RAW_MSG == 1)
extern IFX_int32_t MEI_IoctlRawMsgWrite(
                              MEI_DYN_CNTRL_T *pMeiDynCntrl,
                              IFX_uint16_t      *pMsg,
                              IFX_int32_t       msgCount_16Bit);
extern IFX_int32_t MEI_IoctlRawAckRead(
                              MEI_DYN_CNTRL_T *pMeiDynCntrl,
                              IFX_uint16_t      *pMsgAck,
                              IFX_int32_t       msgCount_16Bit);
extern IFX_int32_t MEI_IoctlRawMsgSend(
                              MEI_DYN_CNTRL_T       *pMeiDynCntrl,
                              IOCTL_MEI_mboxSend_t  *pMBoxSend);
extern IFX_int32_t MEI_IoctlRawNfcRead(
                              MEI_DYN_CNTRL_T *pMeiDynCntrl,
                              IFX_uint16_t      *pMBoxNfc,
                              IFX_int32_t       msgCount_16Bit);
#endif


#ifdef __cplusplus
/* extern "C" */
}
#endif

#endif      /* _DRV_MEI_CPE_MSG_PROCESS_H */

