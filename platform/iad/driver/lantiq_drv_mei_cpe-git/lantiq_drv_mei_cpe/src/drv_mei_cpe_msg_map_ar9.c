/******************************************************************************

                          Copyright (c) 2007-2015
                     Lantiq Beteiligungs-GmbH & Co. KG

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/* ==========================================================================
   Description : Message Handling between the driver and the AR9 device.
   ========================================================================== */

/* ============================================================================
   Includes
   ========================================================================= */

/* get at first the driver configuration */
#include "drv_mei_cpe_config.h"

#if (MEI_SUPPORT_DEVICE_AR9 == 1)

#include "ifx_types.h"

/* add VRX OS Layer */
#include "drv_mei_cpe_os.h"
/* add VRX debug/printout part */
#include "drv_mei_cpe_dbg.h"

#include "drv_mei_cpe_interface.h"
#include "drv_mei_cpe_mei_interface.h"
#include "drv_mei_cpe_api.h"

#include "drv_mei_cpe_msg_process.h"
#include "drv_mei_cpe_driver_msg.h"

#include "drv_mei_cpe_dbg_driver.h"

#include "cmv_message_format_ar9.h"

#include "drv_mei_cpe_msg_map_ar9.h"

/* ============================================================================
   Local Macros & Definitions
   ========================================================================= */

#ifdef MEI_STATIC
#undef MEI_STATIC
#endif

#ifdef MEI_DEBUG
#define MEI_STATIC
#else
#define MEI_STATIC   static
#endif


extern IFX_int32_t MEI_AR9_CmvMsgSend(
                              MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                              IOCTL_MEI_messageSend_t  *pUserMsgs);


MEI_STATIC IFX_int32_t MEI_AR9_CMD_ModemFSM_StateGet(
                              MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                              IOCTL_MEI_messageSend_t  *pUserMsgs,
                              IFX_boolean_t             bInternCall);

MEI_STATIC IFX_int32_t MEI_AR9_CMD_ModemFSM_StateSet(
                              MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                              IOCTL_MEI_messageSend_t  *pUserMsgs,
                              IFX_boolean_t             bInternCall);

MEI_STATIC IFX_int32_t MEI_AR9_CMD_XTSE_Configure(
                              MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                              IOCTL_MEI_messageSend_t  *pUserMsgs,
                              IFX_boolean_t             bInternCall);

MEI_STATIC IFX_int32_t MEI_AR9_CMD_XTSE_StatusGet(
                              MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                              IOCTL_MEI_messageSend_t  *pUserMsgs,
                              IFX_boolean_t             bInternCall);

MEI_STATIC IFX_int32_t MEI_AR9_CMD_BearerChsDS_Get(
                              MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                              IOCTL_MEI_messageSend_t  *pUserMsgs,
                              IFX_boolean_t             bInternCall);

MEI_STATIC IFX_int32_t MEI_AR9_CMD_BearerChsUS_Get(
                              MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                              IOCTL_MEI_messageSend_t  *pUserMsgs,
                              IFX_boolean_t             bInternCall);

MEI_STATIC IFX_int32_t MEI_AR9_CMD_VersionInfoGet(
                              MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                              IOCTL_MEI_messageSend_t  *pUserMsgs,
                              IFX_boolean_t             bInternCall);

MEI_STATIC IFX_int32_t MEI_AR9_CMD_ADSL_FrameDataDS_LP0Get(
                              MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                              IOCTL_MEI_messageSend_t  *pUserMsgs,
                              IFX_boolean_t             bInternCall);

MEI_STATIC IFX_int32_t MEI_AR9_CMD_ADSL_FrameDataDS_LP1Get(
                              MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                              IOCTL_MEI_messageSend_t  *pUserMsgs,
                              IFX_boolean_t             bInternCall);

MEI_STATIC IFX_int32_t MEI_AR9_CMD_ADSL_FrameDataUS_LP0Get(
                              MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                              IOCTL_MEI_messageSend_t  *pUserMsgs,
                              IFX_boolean_t             bInternCall);

MEI_STATIC IFX_int32_t MEI_AR9_CMD_ADSL_FrameDataUS_LP1Get(
                              MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                              IOCTL_MEI_messageSend_t  *pUserMsgs,
                              IFX_boolean_t             bInternCall);

MEI_STATIC IFX_int32_t MEI_AR9_CMD_ADSL_BAT_US_Get(
                              MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                              IOCTL_MEI_messageSend_t  *pUserMsgs,
                              IFX_boolean_t             bInternCall);

MEI_STATIC IFX_int32_t MEI_AR9_CMD_ADSL_BAT_DS_Get(
                              MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                              IOCTL_MEI_messageSend_t  *pUserMsgs,
                              IFX_boolean_t             bInternCall);

MEI_STATIC IFX_int32_t MEI_AR9_CMD_SNR_NE_TableEntriesGet(
                              MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                              IOCTL_MEI_messageSend_t  *pUserMsgs,
                              IFX_boolean_t             bInternCall);

MEI_STATIC IFX_int32_t MEI_AR9_CMD_ADSL_GainTableUS_Get(
                              MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                              IOCTL_MEI_messageSend_t  *pUserMsgs,
                              IFX_boolean_t             bInternCall);

MEI_STATIC IFX_int32_t MEI_AR9_CMD_ADSL_GainTableDS_Get(
                              MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                              IOCTL_MEI_messageSend_t  *pUserMsgs,
                              IFX_boolean_t             bInternCall);

MEI_STATIC IFX_int32_t MEI_AR9_CMD_LineStatusDS_Get(
                              MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                              IOCTL_MEI_messageSend_t  *pUserMsgs,
                              IFX_boolean_t             bInternCall);

MEI_STATIC IFX_int32_t MEI_AR9_CMD_LineStatusUS_Get(
                              MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                              IOCTL_MEI_messageSend_t  *pUserMsgs,
                              IFX_boolean_t             bInternCall);

MEI_STATIC IFX_int32_t MEI_AR9_CMD_CRC_StatsNE_NoTR1Get(
                              MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                              IOCTL_MEI_messageSend_t  *pUserMsgs,
                              IFX_boolean_t             bInternCall);

MEI_STATIC IFX_int32_t MEI_AR9_CMD_CRC_StatsFE_NoTR1Get(
                              MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                              IOCTL_MEI_messageSend_t  *pUserMsgs,
                              IFX_boolean_t             bInternCall);

MEI_STATIC IFX_int32_t MEI_AR9_CMD_FEC_StatsNE_NoTR1Get(
                              MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                              IOCTL_MEI_messageSend_t  *pUserMsgs,
                              IFX_boolean_t             bInternCall);

MEI_STATIC IFX_int32_t MEI_AR9_CMD_FEC_StatsFE_NoTR1Get(
                              MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                              IOCTL_MEI_messageSend_t  *pUserMsgs,
                              IFX_boolean_t             bInternCall);

MEI_STATIC IFX_int32_t MEI_AR9_CMD_CRC_StatsNE_NoTR1Set(
                              MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                              IOCTL_MEI_messageSend_t  *pUserMsgs,
                              IFX_boolean_t             bInternCall);

MEI_STATIC IFX_int32_t MEI_AR9_CMD_FEC_StatsNE_NoTR1Set(
                              MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                              IOCTL_MEI_messageSend_t  *pUserMsgs,
                              IFX_boolean_t             bInternCall);

MEI_STATIC IFX_int32_t MEI_AR9_CMD_HW_ConfigSet(
                              MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                              IOCTL_MEI_messageSend_t  *pUserMsgs,
                              IFX_boolean_t             bInternCall);

MEI_STATIC IFX_int32_t MEI_AR9_CMD_G997_ATM_StatsGet(
                              MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                              IOCTL_MEI_messageSend_t  *pUserMsgs,
                              IFX_boolean_t             bInternCall);

MEI_STATIC IFX_int32_t MEI_AR9_CMD_G997_ATM_StatsSet(
                              MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                              IOCTL_MEI_messageSend_t  *pUserMsgs,
                              IFX_boolean_t             bInternCall);

MEI_STATIC IFX_int32_t MEI_AR9_CMD_SysVendorID_O_Get(
                              MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                              IOCTL_MEI_messageSend_t  *pUserMsgs,
                              IFX_boolean_t             bInternCall);

MEI_STATIC IFX_int32_t MEI_AR9_CMD_SysVendorVersionNumO_Get(
                              MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                              IOCTL_MEI_messageSend_t  *pUserMsgs,
                              IFX_boolean_t             bInternCall);

MEI_STATIC IFX_int32_t MEI_AR9_CMD_SysVendorSerialNum_O_Get(
                              MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                              IOCTL_MEI_messageSend_t  *pUserMsgs,
                              IFX_boolean_t             bInternCall);

MEI_STATIC IFX_int32_t MEI_AR9_CMD_HS_StandardInfoFE_SPAR1Get(
                              MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                              IOCTL_MEI_messageSend_t  *pUserMsgs,
                              IFX_boolean_t             bInternCall);

MEI_STATIC IFX_int32_t MEI_AR9_CMD_LinePerfCountNE_NoTR1Get(
                              MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                              IOCTL_MEI_messageSend_t  *pUserMsgs,
                              IFX_boolean_t             bInternCall);

MEI_STATIC IFX_int32_t MEI_AR9_CMD_LinePerfCountNE_NoTR1Set(
                              MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                              IOCTL_MEI_messageSend_t  *pUserMsgs,
                              IFX_boolean_t             bInternCall);

MEI_STATIC IFX_int32_t MEI_AR9_CMD_LinePerfCountFE_NoTR1Get(
                              MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                              IOCTL_MEI_messageSend_t  *pUserMsgs,
                              IFX_boolean_t             bInternCall);

MEI_STATIC IFX_int32_t MEI_AR9_HDLC_MsgHandle(
                              MEI_DYN_CNTRL_T *pMeiDynCntrl,
                              IFX_uint8_t *pTxHdlcPkt,
                              IFX_int32_t nTxHdlcPktLen,
                              IFX_uint8_t *pRxHdlcPkt,
                              IFX_int32_t nRxHdlcPktLen);

MEI_STATIC IFX_int32_t MEI_AR9_AdslModeStatusGet(
                              MEI_DYN_CNTRL_T *pMeiDynCntrl,
                              IFX_boolean_t   *pAdsl1,
                              IFX_boolean_t   *pAdsl2p);

MEI_AR9_CmdEntry_t g_CmdTable[]={
   MEI_AR9_CMD_ADD(CMD_MODEMFSM_STATEGET, MEI_AR9_CMD_ModemFSM_StateGet),
   MEI_AR9_CMD_ADD(CMD_MODEMFSM_STATESET, MEI_AR9_CMD_ModemFSM_StateSet),
   MEI_AR9_CMD_ADD(CMD_XTSE_CONFIGURE,    MEI_AR9_CMD_XTSE_Configure),
   MEI_AR9_CMD_ADD(CMD_BEARERCHSDS_GET,   MEI_AR9_CMD_BearerChsDS_Get),
   MEI_AR9_CMD_ADD(CMD_BEARERCHSUS_GET,   MEI_AR9_CMD_BearerChsUS_Get),
   MEI_AR9_CMD_ADD(CMD_VERSIONINFOGET,    MEI_AR9_CMD_VersionInfoGet),
   MEI_AR9_CMD_ADD(CMD_XTSE_STATUSGET,    MEI_AR9_CMD_XTSE_StatusGet),
   MEI_AR9_CMD_ADD(CMD_ADSL_FRAMEDATADS_LP0GET, MEI_AR9_CMD_ADSL_FrameDataDS_LP0Get),
   MEI_AR9_CMD_ADD(CMD_ADSL_FRAMEDATADS_LP1GET, MEI_AR9_CMD_ADSL_FrameDataDS_LP1Get),
   MEI_AR9_CMD_ADD(CMD_ADSL_FRAMEDATAUS_LP0GET, MEI_AR9_CMD_ADSL_FrameDataUS_LP0Get),
   MEI_AR9_CMD_ADD(CMD_ADSL_FRAMEDATAUS_LP1GET, MEI_AR9_CMD_ADSL_FrameDataUS_LP1Get),
   MEI_AR9_CMD_ADD(CMD_ADSL_BAT_US_GET, MEI_AR9_CMD_ADSL_BAT_US_Get),
   MEI_AR9_CMD_ADD(CMD_ADSL_BAT_DS_GET, MEI_AR9_CMD_ADSL_BAT_DS_Get),
   MEI_AR9_CMD_ADD(CMD_SNR_NE_TABLEENTRIESGET, MEI_AR9_CMD_SNR_NE_TableEntriesGet),
   MEI_AR9_CMD_ADD(CMD_ADSL_GAINTABLEUS_GET, MEI_AR9_CMD_ADSL_GainTableUS_Get),
   MEI_AR9_CMD_ADD(CMD_ADSL_GAINTABLEDS_GET, MEI_AR9_CMD_ADSL_GainTableDS_Get),
   MEI_AR9_CMD_ADD(CMD_LINESTATUSDS_GET, MEI_AR9_CMD_LineStatusDS_Get),
   MEI_AR9_CMD_ADD(CMD_LINESTATUSUS_GET, MEI_AR9_CMD_LineStatusUS_Get),
   MEI_AR9_CMD_ADD(CMD_HW_CONFIGSET, MEI_AR9_CMD_HW_ConfigSet),
   MEI_AR9_CMD_ADD(CMD_CRC_STATSNE_NOTR1GET, MEI_AR9_CMD_CRC_StatsNE_NoTR1Get),
   MEI_AR9_CMD_ADD(CMD_CRC_STATSFE_NOTR1GET, MEI_AR9_CMD_CRC_StatsFE_NoTR1Get),
   MEI_AR9_CMD_ADD(CMD_FEC_STATSNE_NOTR1GET, MEI_AR9_CMD_FEC_StatsNE_NoTR1Get),
   MEI_AR9_CMD_ADD(CMD_FEC_STATSFE_NOTR1GET, MEI_AR9_CMD_FEC_StatsFE_NoTR1Get),
   MEI_AR9_CMD_ADD(CMD_CRC_STATSNE_NOTR1SET, MEI_AR9_CMD_CRC_StatsNE_NoTR1Set),
   MEI_AR9_CMD_ADD(CMD_FEC_STATSNE_NOTR1SET, MEI_AR9_CMD_FEC_StatsNE_NoTR1Set),
   MEI_AR9_CMD_ADD(CMD_G997_ATM_STATSGET, MEI_AR9_CMD_G997_ATM_StatsGet),
   MEI_AR9_CMD_ADD(CMD_G997_ATM_STATSSET, MEI_AR9_CMD_G997_ATM_StatsSet),
   MEI_AR9_CMD_ADD(CMD_SYSVENDORID_O_GET, MEI_AR9_CMD_SysVendorID_O_Get),
   MEI_AR9_CMD_ADD(CMD_SYSVENDORVERSIONNUMO_GET, MEI_AR9_CMD_SysVendorVersionNumO_Get),
   MEI_AR9_CMD_ADD(CMD_SYSVENDORSERIALNUM_O_GET, MEI_AR9_CMD_SysVendorSerialNum_O_Get),
   MEI_AR9_CMD_ADD(CMD_HS_STANDARDINFOFE_SPAR1GET, MEI_AR9_CMD_HS_StandardInfoFE_SPAR1Get),
   MEI_AR9_CMD_ADD(CMD_LINEPERFCOUNTNE_NOTR1GET, MEI_AR9_CMD_LinePerfCountNE_NoTR1Get),
   MEI_AR9_CMD_ADD(CMD_LINEPERFCOUNTNE_NOTR1SET, MEI_AR9_CMD_LinePerfCountNE_NoTR1Set),
   MEI_AR9_CMD_ADD(CMD_LINEPERFCOUNTFE_NOTR1GET, MEI_AR9_CMD_LinePerfCountFE_NoTR1Get),

   /* Delimeter*/
   MEI_AR9_CMD_ADD(0xFFFF, IFX_NULL)
};

MEI_STATIC IFX_void_t MEI_AR9_CmvMsgMake(
                              IFX_uint8_t  cmv_group,
                              IFX_uint16_t cmv_address,
                              IFX_uint16_t cmv_index,
                              IFX_uint8_t  cmv_sz16,
                              IFX_boolean_t bRead,
                              IFX_uint16_t *pCmvPayload,
                              AR9_CMV_STD_MESSAGE_T   *pCmvTx,
                              AR9_CMV_STD_MESSAGE_T   *pCmvRx,
                              IOCTL_MEI_messageSend_t *pMsg)
{
   memset(pCmvTx, 0x0, sizeof(AR9_CMV_STD_MESSAGE_T));
   memset(pCmvRx, 0x0, sizeof(AR9_CMV_STD_MESSAGE_T));

   /* Set Function Opcode field*/
   P_AR9_CMV_MSGHDR_FCT_OPCODE_SET(pCmvTx, (bRead ? H2D_CMV_READ : H2D_CMV_WRITE));

   /* Set Bit Size field*/
   P_AR9_CMV_MSGHDR_BIT_SIZE_SET(pCmvTx, AR9_CMV_MSG_BIT_SIZE_16BIT);

   /* Set CMV Size field*/
   P_AR9_CMV_MSGHDR_PAYLOAD_SIZE_SET(pCmvTx, cmv_sz16);

   /* Set CMV Group field*/
   P_AR9_CMV_MSGHDR_GROUP_SET(pCmvTx, cmv_group);

   /* Set CMV Access Type field*/
   P_AR9_CMV_MSGHDR_ACCESS_TYPE_SET(pCmvTx,
      (cmv_index == 0 ? AR9_CMV_MSGHDR_ACCESS_TYPE_DIRECT : AR9_CMV_MSGHDR_ACCESS_TYPE_INDIRECT));

   /* Set CMV Address field*/
   P_AR9_CMV_MSGHDR_ADDRESS_SET(pCmvTx, cmv_address);

   /* Set CMV Index field*/
   P_AR9_CMV_MSGHDR_INDEX_SET(pCmvTx, cmv_index);

   if (!bRead && pCmvPayload)
   {
      memcpy(pCmvTx->payload.params_16Bit, pCmvPayload,
         cmv_sz16 > AR9_CMV_MAX_PAYLOAD_16BIT_SIZE ?
         AR9_CMV_MAX_PAYLOAD_8BIT_SIZE : cmv_sz16 * sizeof(IFX_uint16_t));
   }

   memset(pMsg, 0x0, sizeof(IOCTL_MEI_messageSend_t));

   pMsg->write_msg.pPayload = (unsigned char*)pCmvTx;
   pMsg->ack_msg.pPayload   = (unsigned char*)pCmvRx;

   pMsg->write_msg.paylSize_byte = AR9_CMV_HEADER_8BIT_SIZE +
      (bRead ? 0 : P_AR9_CMV_MSGHDR_PAYLOAD_SIZE_GET(pCmvTx) * sizeof(IFX_uint16_t));

   pMsg->ack_msg.paylSize_byte   = AR9_CMV_HEADER_8BIT_SIZE +
      (bRead ? P_AR9_CMV_MSGHDR_PAYLOAD_SIZE_GET(pCmvTx) * sizeof(IFX_uint16_t) : 0);

   return;

}

MEI_STATIC IFX_int32_t MEI_AR9_CmvAckParam16BitGet(
                              IOCTL_MEI_message_t  *pAckMsg,
                              IFX_uint16_t         *pParam,
                              IFX_uint16_t         nParamIndex,
                              IFX_uint16_t         nParamCount)
{
   IFX_int32_t ret = 0;
   IFX_uint16_t nPayloadParamByteCount;

   /* Check pointers*/
   if (!pAckMsg || !pParam)
   {
      return -1;
   }

   /* Check for available CMV Parameters in the Payload*/
   if ((pAckMsg->paylSize_byte == 0) || (pAckMsg->paylSize_byte <= AR9_CMV_HEADER_8BIT_SIZE))
   {
      return -1;
   }

   /* Get CMV Payload Parameters count*/
   nPayloadParamByteCount = pAckMsg->paylSize_byte - AR9_CMV_HEADER_8BIT_SIZE;

   /* Check for enougth CMV Payload Parameters*/
   if ( ((nParamIndex + 1) * nParamCount) * sizeof(IFX_uint16_t) >  nPayloadParamByteCount)
   {
      return -1;
   }

   memcpy(
      pParam, pAckMsg->pPayload + AR9_CMV_HEADER_8BIT_SIZE +(nParamIndex * sizeof(IFX_uint16_t)),
      nParamCount * sizeof(IFX_uint16_t));

   return ret;
}

MEI_STATIC IFX_int32_t MEI_AR9_UserCmdHeaderGet(
                              IOCTL_MEI_message_t *pUserCmdMsg,
                              IFX_uint8_t         *pCmd,
                              IFX_boolean_t        bInternCall)
{
   IFX_int32_t ret = 0;

   if (pUserCmdMsg->paylSize_byte < sizeof(MEI_UserMsgHeader_t))
   {
      return -1;
   }

   /* Get Index and Length*/
   if (bInternCall)
   {
      memcpy(pCmd, pUserCmdMsg->pPayload, sizeof(MEI_UserMsgHeader_t));
   }
   else
   {
      if ( (MEI_DRVOS_CpyFromUser( pCmd, pUserCmdMsg->pPayload,
                                   sizeof(MEI_UserMsgHeader_t))) == IFX_NULL )

      {
         ret = -e_MEI_ERR_RETURN_ARG;
      }
   }

   return ret;
}

MEI_STATIC IFX_int32_t MEI_AR9_UserAckHeaderSet(
                              IOCTL_MEI_message_t  *pUserCmdMsg,
                              IFX_uint8_t          *pAck,
                              IFX_boolean_t        bInternCall)
{
   IFX_int32_t ret = 0, nAckSyzeByte = sizeof(MEI_UserMsgHeader_t);

   if (sizeof(MEI_UserMsgHeader_t) > pUserCmdMsg->paylSize_byte)
   {
      nAckSyzeByte = pUserCmdMsg->paylSize_byte;
      return -1;
   }

   /* Get Index and Length*/
   if (bInternCall)
   {
      memcpy(pUserCmdMsg->pPayload, pAck, nAckSyzeByte);
   }
   else
   {
      if ( (MEI_DRVOS_CpyToUser( pUserCmdMsg->pPayload, pAck,
                                 nAckSyzeByte)) == IFX_NULL )

      {
         ret = -e_MEI_ERR_RETURN_ARG;
      }
   }

   return ret;
}

MEI_STATIC IFX_int32_t MEI_AR9_UserCmdGet(
                              IOCTL_MEI_message_t *pUserCmdMsg,
                              IFX_uint8_t         *pCmd,
                              IFX_uint16_t         nCmdSyzeByte,
                              IFX_boolean_t        bInternCall)
{
   IFX_int32_t ret = 0;

   if (pUserCmdMsg->paylSize_byte > nCmdSyzeByte)
   {
      return -1;
   }

   /* Get Index and Length*/
   if (bInternCall)
   {
      memcpy(pCmd, pUserCmdMsg->pPayload, pUserCmdMsg->paylSize_byte);
   }
   else
   {
      if ( (MEI_DRVOS_CpyFromUser( pCmd, pUserCmdMsg->pPayload,
                                   pUserCmdMsg->paylSize_byte)) == IFX_NULL )

      {
         ret = -e_MEI_ERR_RETURN_ARG;
      }
   }

   return ret;
}

MEI_STATIC IFX_int32_t MEI_AR9_UserAckSet(
                              IOCTL_MEI_message_t  *pUserCmdMsg,
                              IFX_uint8_t          *pAck,
                              IFX_uint16_t         nAckSyzeByte,
                              IFX_boolean_t        bInternCall)
{
   IFX_int32_t ret = 0;

   if (nAckSyzeByte > pUserCmdMsg->paylSize_byte)
   {
      nAckSyzeByte = pUserCmdMsg->paylSize_byte;
      return -1;
   }

   /* Get Index and Length*/
   if (bInternCall)
   {
      memcpy(pUserCmdMsg->pPayload, pAck, nAckSyzeByte);
   }
   else
   {
      if ( (MEI_DRVOS_CpyToUser( pUserCmdMsg->pPayload, pAck,
                                 nAckSyzeByte)) == IFX_NULL )

      {
         ret = -e_MEI_ERR_RETURN_ARG;
      }
   }

   return ret;
}

MEI_STATIC IFX_int32_t MEI_AR9_CMD_Unhandled(
                              MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                              IOCTL_MEI_messageSend_t  *pUserMsgs,
                              IFX_boolean_t             bInternCall)
{
   IFX_int32_t ret = 0;
   MEI_UserMsgHeader_t cmdHdr, ackHdr;

   /* Get User Cmd */
   ret = MEI_AR9_UserCmdHeaderGet(&pUserMsgs->write_msg,
                                 (IFX_uint8_t*)&cmdHdr, bInternCall);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   /* Set ACK Index*/
   ackHdr.nIndex  = cmdHdr.nIndex;
   /* Set ACK Length*/
   ackHdr.nLength = cmdHdr.nLength;

   /* Set User Ack*/
   ret = MEI_AR9_UserAckHeaderSet(&pUserMsgs->ack_msg,
                                 (IFX_uint8_t*) &ackHdr, bInternCall);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   pUserMsgs->ictl.retCode = ret;

   return ret;
}

/**
   CMD_VersionInfoGet firmware message handler.
   Internal mapping only!
*/
MEI_STATIC IFX_int32_t MEI_AR9_CMD_VersionInfoGet(
                              MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                              IOCTL_MEI_messageSend_t  *pUserMsgs,
                              IFX_boolean_t             bInternCall)
{
   IFX_int32_t ret = 0;
   CMD_VersionInfoGet_t cmd = {0};
   ACK_VersionInfoGet_t ack = {0};

   /* Get User Cmd */
   ret = MEI_AR9_UserCmdGet(&pUserMsgs->write_msg, (IFX_uint8_t*)&cmd,
                            sizeof(CMD_VersionInfoGet_t), bInternCall);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   /* Set ACK Index*/
   ack.Index  = cmd.Index;
   /* Set ACK Length*/
   ack.Length = cmd.Length;

   /* Set HW version: DFE HW Version Number for VRX V1.3, Shared reticle*/
   ack.HW_Version = 0x3;
   /* Set FW version: v9.6.1.2.0.5*/
   ack.FW_Version = (0x9 << 24) | (0x6 << 16) | (0x1 << 12) | (0x2 << 8) | (0x0 << 6) | (5 << 0);

   /* Set User Ack*/
   ret = MEI_AR9_UserAckSet(&pUserMsgs->ack_msg, (IFX_uint8_t*) &ack,
                            sizeof(ACK_VersionInfoGet_t), bInternCall);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   pUserMsgs->ack_msg.paylSize_byte = sizeof(ACK_VersionInfoGet_t);

   pUserMsgs->ictl.retCode = ret;

   return ret;
}

/**
   CMD_XTSE_StatusGet firmware message handler.
*/
MEI_STATIC IFX_int32_t MEI_AR9_CMD_XTSE_StatusGet(
                              MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                              IOCTL_MEI_messageSend_t  *pUserMsgs,
                              IFX_boolean_t             bInternCall)
{
   IFX_int32_t ret = 0;
   IFX_uint16_t nParam16;
   CMD_XTSE_StatusGet_t cmd = {0};
   ACK_XTSE_StatusGet_t ack = {0};
   AR9_CMV_STD_MESSAGE_T cmv_tx, cmv_rx;
   IOCTL_MEI_messageSend_t msg;

   /* Get User Cmd */
   ret = MEI_AR9_UserCmdGet(&pUserMsgs->write_msg, (IFX_uint8_t*)&cmd,
                            sizeof(CMD_XTSE_StatusGet_t), bInternCall);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   /* SOC: STAT 0x1 0x0 0x1*/
   MEI_AR9_CmvMsgMake(CMV_GROUP_STAT, 0x1, 0x0, 0x1, IFX_TRUE, IFX_NULL, &cmv_tx, &cmv_rx, &msg);

   /* Send Socrates Cmv Message and get CMV response*/
   ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = msg.ictl.retCode;
      return ret;
   }


   /* Get CMV Ack Message Parameter*/
   ret = MEI_AR9_CmvAckParam16BitGet(&msg.ack_msg, &nParam16, 0x0, 0x1);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   ack.A15 = MEI_AR9_BIT_GET(nParam16, 15);
   ack.A14 = MEI_AR9_BIT_GET(nParam16, 14);
   ack.A13 = MEI_AR9_BIT_GET(nParam16, 13);
   ack.A9  = MEI_AR9_BIT_GET(nParam16,  9);
   ack.A8  = MEI_AR9_BIT_GET(nParam16,  8);
   ack.A5  = MEI_AR9_BIT_GET(nParam16, 11);
   ack.A4  = MEI_AR9_BIT_GET(nParam16, 10);
   ack.A3  = MEI_AR9_BIT_GET(nParam16,  3);
   ack.A2  = MEI_AR9_BIT_GET(nParam16,  2);
   ack.A0  = MEI_AR9_BIT_GET(nParam16,  0);

   /* Select Annex L Mask1/Mask2 */
   if (nParam16 == 0x1100)
   {
      /* SOC: STAT 0x4 0x0 0x1*/
      MEI_AR9_CmvMsgMake(CMV_GROUP_STAT, 0x4, 0x0, 0x1, IFX_TRUE, IFX_NULL, &cmv_tx, &cmv_rx, &msg);

      /* Send Socrates Cmv Message and get CMV response*/
      ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
      if (ret < 0)
      {
         pUserMsgs->ictl.retCode = msg.ictl.retCode;
         return ret;
      }


      /* Get CMV Ack Message Parameter*/
      ret = MEI_AR9_CmvAckParam16BitGet(&msg.ack_msg, &nParam16, 0x0, 0x1);
      if (ret < 0)
      {
         pUserMsgs->ictl.retCode = ret;
         return ret;
      }

      ack.A11 = MEI_AR9_BIT_GET(nParam16, 12);
      ack.A12 = MEI_AR9_BIT_GET(nParam16, 13);
   }

   /* SOC: STAT 17 0x0 0x1*/
   MEI_AR9_CmvMsgMake(CMV_GROUP_STAT, 17, 0x0, 0x1, IFX_TRUE, IFX_NULL, &cmv_tx, &cmv_rx, &msg);

   /* Send Socrates Cmv Message and get CMV response*/
   ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = msg.ictl.retCode;
      return ret;
   }


   /* Get CMV Ack Message Parameter*/
   ret = MEI_AR9_CmvAckParam16BitGet(&msg.ack_msg, &nParam16, 0x0, 0x1);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   ack.A6  = MEI_AR9_BIT_GET(nParam16, 0);
   ack.A7  = MEI_AR9_BIT_GET(nParam16, 1);
   ack.A1  = MEI_AR9_BIT_GET(nParam16, 2);

   /* Set ACK Index*/
   ack.Index  = cmd.Index;
   /* Set ACK Length*/
   ack.Length = cmd.Length;

   /* Set User Ack*/
   ret = MEI_AR9_UserAckSet(&pUserMsgs->ack_msg, (IFX_uint8_t*) &ack,
                            sizeof(ACK_XTSE_StatusGet_t), bInternCall);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   pUserMsgs->ack_msg.paylSize_byte = sizeof(ACK_XTSE_StatusGet_t);

   pUserMsgs->ictl.retCode = ret;

   return ret;
}

/**
   CMD_ModemFSM_StateGet firmware message handler.
*/
MEI_STATIC IFX_int32_t MEI_AR9_CMD_ModemFSM_StateGet(
                              MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                              IOCTL_MEI_messageSend_t  *pUserMsgs,
                              IFX_boolean_t             bInternCall)
{
   IFX_int32_t ret = 0;
   IFX_uint16_t nParam16;
   CMD_ModemFSM_StateGet_t cmd = {0};
   ACK_ModemFSM_StateGet_t ack = {0};
   AR9_CMV_STD_MESSAGE_T cmv_tx, cmv_rx;
   IOCTL_MEI_messageSend_t msg;

   /* Get User Cmd */
   ret = MEI_AR9_UserCmdGet(&pUserMsgs->write_msg, (IFX_uint8_t*)&cmd,
                            sizeof(CMD_ModemFSM_StateGet_t), bInternCall);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   /* SOC: STAT 0x0 0x0 0x1*/
   MEI_AR9_CmvMsgMake(CMV_GROUP_STAT, 0x0, 0x0, 0x1, IFX_TRUE, IFX_NULL, &cmv_tx, &cmv_rx, &msg);

   /* Send Socrates Cmv Message and get CMV response*/
   ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = msg.ictl.retCode;
      return ret;
   }


   /* Get CMV Ack Message Parameter*/
   ret = MEI_AR9_CmvAckParam16BitGet(&msg.ack_msg, &nParam16, 0x0, 0x1);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   /* Set ACK Index*/
   ack.Index  = cmd.Index;
   /* Set ACK Length*/
   ack.Length = cmd.Length;
   /* Set ModemState*/
   ack.ModemState = nParam16;


   /* Set User Ack*/
   ret = MEI_AR9_UserAckSet(&pUserMsgs->ack_msg, (IFX_uint8_t*) &ack,
                            sizeof(ACK_ModemFSM_StateGet_t), bInternCall);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   pUserMsgs->ack_msg.paylSize_byte = sizeof(MEI_UserMsgHeader_t) + sizeof(nParam16);

   pUserMsgs->ictl.retCode = ret;

   return ret;
}

/**
   CMD_ModemFSM_StateSet firmware message handler.
*/
MEI_STATIC IFX_int32_t MEI_AR9_CMD_ModemFSM_StateSet(
                              MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                              IOCTL_MEI_messageSend_t  *pUserMsgs,
                              IFX_boolean_t             bInternCall)
{
   IFX_int32_t ret = 0;
   IFX_uint16_t nParam16;
   CMD_ModemFSM_StateSet_t cmd = {0};
   ACK_ModemFSM_StateSet_t ack = {0};
   AR9_CMV_STD_MESSAGE_T cmv_tx, cmv_rx;
   IOCTL_MEI_messageSend_t msg;

   /* Get User Cmd */
   ret = MEI_AR9_UserCmdGet(&pUserMsgs->write_msg, (IFX_uint8_t*)&cmd,
                            sizeof(CMD_ModemFSM_StateSet_t), bInternCall);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   /* Get LinkControl*/
   nParam16 = (IFX_uint16_t)cmd.LinkControl;

   /* SOC: CNTL 0x0 0x0 0x1*/
   MEI_AR9_CmvMsgMake(CMV_GROUP_CNTL, 0x0, 0x0, 0x1, IFX_FALSE, &nParam16, &cmv_tx, &cmv_rx, &msg);

   /* Send Socrates Cmv Message and get CMV response*/
   ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = msg.ictl.retCode;
      return ret;
   }

   /* Set ACK Index*/
   ack.Index  = cmd.Index;
   /* Set ACK Length*/
   ack.Length = cmd.Length;

   /* Set User Ack*/
   ret = MEI_AR9_UserAckSet(&pUserMsgs->ack_msg, (IFX_uint8_t*) &ack,
                            sizeof(ACK_ModemFSM_StateSet_t), bInternCall);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   pUserMsgs->ack_msg.paylSize_byte = sizeof(MEI_UserMsgHeader_t);

   pUserMsgs->ictl.retCode = ret;

   return ret;
}

/**
   CMD_XTSE_Configure firmware message handler.
*/
MEI_STATIC IFX_int32_t MEI_AR9_CMD_XTSE_Configure(
                              MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                              IOCTL_MEI_messageSend_t  *pUserMsgs,
                              IFX_boolean_t             bInternCall)
{
   IFX_int32_t ret = 0;
   IFX_uint16_t nParam16 = 0;
   CMD_XTSE_Configure_t cmd = {0};
   ACK_XTSE_Configure_t ack = {0};
   AR9_CMV_STD_MESSAGE_T cmv_tx, cmv_rx;
   IOCTL_MEI_messageSend_t msg;

   /* Get User Cmd */
   ret = MEI_AR9_UserCmdGet(&pUserMsgs->write_msg, (IFX_uint8_t*)&cmd,
                            sizeof(CMD_XTSE_Configure_t), bInternCall);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   /* T1.413*/
   nParam16 |= cmd.A0;
   /* G.992.1, Annex A*/
   nParam16 |= (cmd.A2 << 2);
   /* G.992.1, Annex B*/
   nParam16 |= (cmd.A3 << 3);
   /* G.992.3, Annex A*/
   nParam16 |= (cmd.A8 << 8);
   /* G.992.3, Annex B*/
   nParam16 |= (cmd.A9 << 9);
   /* G.992.3, Annex I*/
   nParam16 |= (cmd.A4 << 10);
   /* G.992.3, Annex J*/
   nParam16 |= (cmd.A5 << 11);
   /* G.992.3, Annex L*/
   nParam16 |= (cmd.A11 << 12);
   nParam16 |= (cmd.A12 << 12);
   /* G.992.3, Annex M*/
   nParam16 |= (cmd.A13 << 13);
   /* G.992.5, Annex B*/
   nParam16 |= (cmd.A14 << 14);
   /* G.992.5, Annex A*/
   nParam16 |= (cmd.A15 << 15);

   /* SOC: OPTN 0x0 0x0 0x1*/
   MEI_AR9_CmvMsgMake(CMV_GROUP_OPTN, 0x0, 0x0, 0x1, IFX_FALSE, &nParam16, &cmv_tx, &cmv_rx, &msg);

   /* Send Socrates Cmv Message and get CMV response*/
   ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = msg.ictl.retCode;
      return ret;
   }

   nParam16 = 0;
   /* G.992.5, Annex I*/
   nParam16 |= cmd.A6;
   /* G.992.5, Annex J*/
   nParam16 |= (cmd.A7 << 1);
   /* G.992.5, Annex M*/
   nParam16 |= (cmd.A1 << 2);

   /* SOC: OPTN 0x7 0x0 0x1*/
   MEI_AR9_CmvMsgMake(CMV_GROUP_OPTN, 0x7, 0x0, 0x1, IFX_FALSE, &nParam16, &cmv_tx, &cmv_rx, &msg);

   /* Send Socrates Cmv Message and get CMV response*/
   ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = msg.ictl.retCode;
      return ret;
   }

   /* Set ACK Index*/
   ack.Index  = cmd.Index;
   /* Set ACK Length*/
   ack.Length = cmd.Length;

   /* Set User Ack*/
   ret = MEI_AR9_UserAckSet(&pUserMsgs->ack_msg, (IFX_uint8_t*) &ack,
                            sizeof(ACK_ModemFSM_StateSet_t), bInternCall);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   pUserMsgs->ack_msg.paylSize_byte = sizeof(MEI_UserMsgHeader_t);

   pUserMsgs->ictl.retCode = ret;

   return ret;
}

/**
   CMD_BearerChsDS_Get firmware message handler.
*/
MEI_STATIC IFX_int32_t MEI_AR9_CMD_BearerChsDS_Get(
                              MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                              IOCTL_MEI_messageSend_t  *pUserMsgs,
                              IFX_boolean_t             bInternCall)
{
   IFX_int32_t ret = 0;
   IFX_uint16_t nParam16[4] = {0};
   CMD_BearerChsDS_Get_t cmd = {0};
   ACK_BearerChsDS_Get_t ack = {0};
   AR9_CMV_STD_MESSAGE_T cmv_tx, cmv_rx;
   IOCTL_MEI_messageSend_t msg;

   /* Get User Cmd */
   ret = MEI_AR9_UserCmdGet(&pUserMsgs->write_msg, (IFX_uint8_t*)&cmd,
                            sizeof(CMD_BearerChsDS_Get_t), bInternCall);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   /* SOC: RATE 0x1 0x0 0x4*/
   MEI_AR9_CmvMsgMake(CMV_GROUP_RATE, 0x1, 0x0, 0x4, IFX_TRUE, IFX_NULL, &cmv_tx, &cmv_rx, &msg);

   /* Send Socrates Cmv Message and get CMV response*/
   ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = msg.ictl.retCode;
      return ret;
   }

   /* Send Socrates Cmv Message and get CMV response*/
   ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = msg.ictl.retCode;
      return ret;
   }

   /* Get CMV Ack Message Parameter*/
   ret = MEI_AR9_CmvAckParam16BitGet(&msg.ack_msg, &nParam16[0], 0x0, 0x4);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   /* Fill ACK*/
   ack.DRdsLP0_LSW = nParam16[0];
   ack.DRdsLP0_MSW = nParam16[1];
   ack.DRdsLP1_LSW = nParam16[2];
   ack.DRdsLP1_MSW = nParam16[3];

   /* SOC: INFO 92 0x0 0x1*/
   MEI_AR9_CmvMsgMake(CMV_GROUP_INFO, 92, 0x0, 0x1, IFX_TRUE, IFX_NULL, &cmv_tx, &cmv_rx, &msg);

   /* Send Socrates Cmv Message and get CMV response*/
   ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = msg.ictl.retCode;
      return ret;
   }

   /* Get CMV Ack Message Parameter*/
   ret = MEI_AR9_CmvAckParam16BitGet(&msg.ack_msg, &nParam16[0], 0x0, 0x1);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   /* SOC: INFO 92 0x1 0x1*/
   MEI_AR9_CmvMsgMake(CMV_GROUP_INFO, 92, 0x1, 0x1, IFX_TRUE, IFX_NULL, &cmv_tx, &cmv_rx, &msg);

   /* Send Socrates Cmv Message and get CMV response*/
   ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = msg.ictl.retCode;
      return ret;
   }

   /* Get CMV Ack Message Parameter*/
   ret = MEI_AR9_CmvAckParam16BitGet(&msg.ack_msg, &nParam16[1], 0x0, 0x1);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   ack.delay_dsLP0 = nParam16[1]/4;

   /* SOC: INFO 92 (INFO 92 0)+1 0x1*/
   MEI_AR9_CmvMsgMake(CMV_GROUP_INFO, 92, nParam16[0]+1, 0x1, IFX_TRUE, IFX_NULL, &cmv_tx, &cmv_rx, &msg);

   /* Send Socrates Cmv Message and get CMV response*/
   ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = msg.ictl.retCode;
      return ret;
   }

   /* Get CMV Ack Message Parameter*/
   ret = MEI_AR9_CmvAckParam16BitGet(&msg.ack_msg, &nParam16[2], 0x0, 0x1);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   ack.delay_dsLP1 = nParam16[2]/4;

   /* SOC: STAT 4 0x0 0x1*/
   MEI_AR9_CmvMsgMake(CMV_GROUP_STAT, 4, 0x0, 0x1, IFX_TRUE, IFX_NULL, &cmv_tx, &cmv_rx, &msg);

   /* Send Socrates Cmv Message and get CMV response*/
   ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = msg.ictl.retCode;
      return ret;
   }

   /* Get CMV Ack Message Parameter*/
   ret = MEI_AR9_CmvAckParam16BitGet(&msg.ack_msg, &nParam16[0], 0x0, 0x1);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   ack.TCMstatus_dsLP0 = (nParam16[0] >> 4) & 0x1;

   /* Set ACK Index*/
   ack.Index  = cmd.Index;
   /* Set ACK Length*/
   ack.Length = cmd.Length;

   /* Set User Ack*/
   ret = MEI_AR9_UserAckSet(&pUserMsgs->ack_msg, (IFX_uint8_t*) &ack,
                            sizeof(ACK_BearerChsDS_Get_t), bInternCall);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   pUserMsgs->ack_msg.paylSize_byte = sizeof(ACK_BearerChsDS_Get_t);

   pUserMsgs->ictl.retCode = ret;

   return ret;
}

/**
   CMD_BearerChsUS_Get firmware message handler.
*/
MEI_STATIC IFX_int32_t MEI_AR9_CMD_BearerChsUS_Get(
                              MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                              IOCTL_MEI_messageSend_t  *pUserMsgs,
                              IFX_boolean_t             bInternCall)
{
   IFX_int32_t ret = 0;
   IFX_uint16_t nParam16[4] = {0};
   CMD_BearerChsUS_Get_t cmd = {0};
   ACK_BearerChsUS_Get_t ack = {0};
   AR9_CMV_STD_MESSAGE_T cmv_tx, cmv_rx;
   IOCTL_MEI_messageSend_t msg;

   /* Get User Cmd */
   ret = MEI_AR9_UserCmdGet(&pUserMsgs->write_msg, (IFX_uint8_t*)&cmd,
                            sizeof(CMD_BearerChsUS_Get_t), bInternCall);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   /* SOC: RATE 0x0 0x0 0x4*/
   MEI_AR9_CmvMsgMake(CMV_GROUP_RATE, 0x0, 0x0, 0x4, IFX_TRUE, IFX_NULL, &cmv_tx, &cmv_rx, &msg);

   /* Send Socrates Cmv Message and get CMV response*/
   ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = msg.ictl.retCode;
      return ret;
   }

   /* Send Socrates Cmv Message and get CMV response*/
   ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = msg.ictl.retCode;
      return ret;
   }

   /* Get CMV Ack Message Parameter*/
   ret = MEI_AR9_CmvAckParam16BitGet(&msg.ack_msg, &nParam16[0], 0x0, 0x4);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   /* Fill ACK*/
   ack.DRusLP0_LSW = nParam16[0];
   ack.DRusLP0_MSW = nParam16[1];
   ack.DRusLP1_LSW = nParam16[2];
   ack.DRusLP1_MSW = nParam16[3];

   /* SOC: INFO 93 0x0 0x1*/
   MEI_AR9_CmvMsgMake(CMV_GROUP_INFO, 93, 0x0, 0x1, IFX_TRUE, IFX_NULL, &cmv_tx, &cmv_rx, &msg);

   /* Send Socrates Cmv Message and get CMV response*/
   ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = msg.ictl.retCode;
      return ret;
   }

   /* Get CMV Ack Message Parameter*/
   ret = MEI_AR9_CmvAckParam16BitGet(&msg.ack_msg, &nParam16[0], 0x0, 0x1);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   /* SOC: INFO 93 0x1 0x1*/
   MEI_AR9_CmvMsgMake(CMV_GROUP_INFO, 93, 0x1, 0x1, IFX_TRUE, IFX_NULL, &cmv_tx, &cmv_rx, &msg);

   /* Send Socrates Cmv Message and get CMV response*/
   ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = msg.ictl.retCode;
      return ret;
   }

   /* Get CMV Ack Message Parameter*/
   ret = MEI_AR9_CmvAckParam16BitGet(&msg.ack_msg, &nParam16[1], 0x0, 0x1);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   ack.delay_usLP0 = nParam16[1]/4;

   /* SOC: INFO 93 (INFO 93 0)+1 0x1*/
   MEI_AR9_CmvMsgMake(CMV_GROUP_INFO, 93, nParam16[0]+1, 0x1, IFX_TRUE, IFX_NULL, &cmv_tx, &cmv_rx, &msg);

   /* Send Socrates Cmv Message and get CMV response*/
   ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = msg.ictl.retCode;
      return ret;
   }

   /* Get CMV Ack Message Parameter*/
   ret = MEI_AR9_CmvAckParam16BitGet(&msg.ack_msg, &nParam16[2], 0x0, 0x1);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   ack.delay_usLP1 = nParam16[2]/4;

   /* SOC: STAT 4 0x0 0x1*/
   MEI_AR9_CmvMsgMake(CMV_GROUP_STAT, 4, 0x0, 0x1, IFX_TRUE, IFX_NULL, &cmv_tx, &cmv_rx, &msg);

   /* Send Socrates Cmv Message and get CMV response*/
   ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = msg.ictl.retCode;
      return ret;
   }

   /* Get CMV Ack Message Parameter*/
   ret = MEI_AR9_CmvAckParam16BitGet(&msg.ack_msg, &nParam16[0], 0x0, 0x1);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   ack.TCMstatus_usLP0 = (nParam16[0] >> 7) & 0x1;

   /* Set ACK Index*/
   ack.Index  = cmd.Index;
   /* Set ACK Length*/
   ack.Length = cmd.Length;

   /* Set User Ack*/
   ret = MEI_AR9_UserAckSet(&pUserMsgs->ack_msg, (IFX_uint8_t*) &ack,
                            sizeof(ACK_BearerChsUS_Get_t), bInternCall);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   pUserMsgs->ack_msg.paylSize_byte = sizeof(ACK_BearerChsUS_Get_t);

   pUserMsgs->ictl.retCode = ret;

   return ret;
}

/**
   CMD_ADSL_FrameDataDS_LP0Get firmware message handler.
*/
MEI_STATIC IFX_int32_t MEI_AR9_CMD_ADSL_FrameDataDS_LP0Get(
                              MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                              IOCTL_MEI_messageSend_t  *pUserMsgs,
                              IFX_boolean_t             bInternCall)
{
   IFX_int32_t ret = 0;
   IFX_uint16_t nParam16;
   CMD_ADSL_FrameDataDS_LP0Get_t cmd = {0};
   ACK_ADSL_FrameDataDS_LP0Get_t ack = {0};
   AR9_CMV_STD_MESSAGE_T cmv_tx, cmv_rx;
   IOCTL_MEI_messageSend_t msg;

   /* Get User Cmd */
   ret = MEI_AR9_UserCmdGet(&pUserMsgs->write_msg, (IFX_uint8_t*)&cmd,
                            sizeof(CMD_ADSL_FrameDataDS_LP0Get_t), bInternCall);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   /* SOC: CNFG 23 0x0 0x1*/
   MEI_AR9_CmvMsgMake(CMV_GROUP_CNFG, 23, 0x0, 0x1, IFX_TRUE, IFX_NULL, &cmv_tx, &cmv_rx, &msg);

   /* Send Socrates Cmv Message and get CMV response*/
   ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = msg.ictl.retCode;
      return ret;
   }

   /* Get CMV Ack Message Parameter*/
   ret = MEI_AR9_CmvAckParam16BitGet(&msg.ack_msg, &nParam16, 0x0, 0x1);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   /* Set Ack Rp value*/
   ack.Rp = nParam16;

   /* SOC: CNFG 27 0x0 0x1*/
   MEI_AR9_CmvMsgMake(CMV_GROUP_CNFG, 27, 0x0, 0x1, IFX_TRUE, IFX_NULL, &cmv_tx, &cmv_rx, &msg);

   /* Send Socrates Cmv Message and get CMV response*/
   ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = msg.ictl.retCode;
      return ret;
   }

   /* Get CMV Ack Message Parameter*/
   ret = MEI_AR9_CmvAckParam16BitGet(&msg.ack_msg, &nParam16, 0x0, 0x1);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   /* Set Ack Dp value*/
   ack.Dp = nParam16;

   /* SOC: CNFG 28 0x0 0x1*/
   MEI_AR9_CmvMsgMake(CMV_GROUP_CNFG, 28, 0x0, 0x1, IFX_TRUE, IFX_NULL, &cmv_tx, &cmv_rx, &msg);

   /* Send Socrates Cmv Message and get CMV response*/
   ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = msg.ictl.retCode;
      return ret;
   }

   /* Get CMV Ack Message Parameter*/
   ret = MEI_AR9_CmvAckParam16BitGet(&msg.ack_msg, &nParam16, 0x0, 0x1);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   /* Set Ack Kp value*/
   ack.Kp = nParam16;

   /* SOC: CNFG 24 0x0 0x1*/
   MEI_AR9_CmvMsgMake(CMV_GROUP_CNFG, 24, 0x0, 0x1, IFX_TRUE, IFX_NULL, &cmv_tx, &cmv_rx, &msg);

   /* Send Socrates Cmv Message and get CMV response*/
   ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = msg.ictl.retCode;
      return ret;
   }

   /* Get CMV Ack Message Parameter*/
   ret = MEI_AR9_CmvAckParam16BitGet(&msg.ack_msg, &nParam16, 0x0, 0x1);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   /* Set Ack Mp value*/
   ack.Mp = nParam16;

   /* SOC: CNFG 25 0x0 0x1*/
   MEI_AR9_CmvMsgMake(CMV_GROUP_CNFG, 25, 0x0, 0x1, IFX_TRUE, IFX_NULL, &cmv_tx, &cmv_rx, &msg);

   /* Send Socrates Cmv Message and get CMV response*/
   ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = msg.ictl.retCode;
      return ret;
   }

   /* Get CMV Ack Message Parameter*/
   ret = MEI_AR9_CmvAckParam16BitGet(&msg.ack_msg, &nParam16, 0x0, 0x1);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   /* Set Ack Lp value*/
   ack.Lp = nParam16;

   /* SOC: CNFG 26 0x0 0x1*/
   MEI_AR9_CmvMsgMake(CMV_GROUP_CNFG, 26, 0x0, 0x1, IFX_TRUE, IFX_NULL, &cmv_tx, &cmv_rx, &msg);

   /* Send Socrates Cmv Message and get CMV response*/
   ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = msg.ictl.retCode;
      return ret;
   }

   /* Get CMV Ack Message Parameter*/
   ret = MEI_AR9_CmvAckParam16BitGet(&msg.ack_msg, &nParam16, 0x0, 0x1);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   /* Set Ack Tp value*/
   ack.Tp = nParam16;

   /* SOC: CNFG 31 0x0 0x1*/
   MEI_AR9_CmvMsgMake(CMV_GROUP_CNFG, 31, 0x0, 0x1, IFX_TRUE, IFX_NULL, &cmv_tx, &cmv_rx, &msg);

   /* Send Socrates Cmv Message and get CMV response*/
   ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = msg.ictl.retCode;
      return ret;
   }

   /* Get CMV Ack Message Parameter*/
   ret = MEI_AR9_CmvAckParam16BitGet(&msg.ack_msg, &nParam16, 0x0, 0x1);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   /* Set Ack MSGc value*/
   ack.MSGc = nParam16;

   /* Set ACK Index*/
   ack.Index  = cmd.Index;
   /* Set ACK Length*/
   ack.Length = cmd.Length;

   /* Set User Ack*/
   ret = MEI_AR9_UserAckSet(&pUserMsgs->ack_msg, (IFX_uint8_t*) &ack,
                            sizeof(ACK_ADSL_FrameDataDS_LP0Get_t), bInternCall);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   pUserMsgs->ack_msg.paylSize_byte = sizeof(ACK_ADSL_FrameDataDS_LP0Get_t);

   pUserMsgs->ictl.retCode = ret;

   return ret;
}

/**
   CMD_ADSL_FrameDataDS_LP1Get firmware message handler.
*/
MEI_STATIC IFX_int32_t MEI_AR9_CMD_ADSL_FrameDataDS_LP1Get(
                              MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                              IOCTL_MEI_messageSend_t  *pUserMsgs,
                              IFX_boolean_t             bInternCall)
{
   IFX_int32_t ret = 0;
   IFX_uint16_t nParam16;
   CMD_ADSL_FrameDataDS_LP1Get_t cmd = {0};
   ACK_ADSL_FrameDataDS_LP1Get_t ack = {0};
   AR9_CMV_STD_MESSAGE_T cmv_tx, cmv_rx;
   IOCTL_MEI_messageSend_t msg;

   /* Get User Cmd */
   ret = MEI_AR9_UserCmdGet(&pUserMsgs->write_msg, (IFX_uint8_t*)&cmd,
                            sizeof(CMD_ADSL_FrameDataDS_LP1Get_t), bInternCall);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   /* SOC: CNFG 23 0x1 0x1*/
   MEI_AR9_CmvMsgMake(CMV_GROUP_CNFG, 23, 0x1, 0x1, IFX_TRUE, IFX_NULL, &cmv_tx, &cmv_rx, &msg);

   /* Send Socrates Cmv Message and get CMV response*/
   ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = msg.ictl.retCode;
      return ret;
   }

   /* Get CMV Ack Message Parameter*/
   ret = MEI_AR9_CmvAckParam16BitGet(&msg.ack_msg, &nParam16, 0x0, 0x1);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   /* Set Ack Rp value*/
   ack.Rp = nParam16;

   /* SOC: CNFG 27 0x1 0x1*/
   MEI_AR9_CmvMsgMake(CMV_GROUP_CNFG, 27, 0x1, 0x1, IFX_TRUE, IFX_NULL, &cmv_tx, &cmv_rx, &msg);

   /* Send Socrates Cmv Message and get CMV response*/
   ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = msg.ictl.retCode;
      return ret;
   }

   /* Get CMV Ack Message Parameter*/
   ret = MEI_AR9_CmvAckParam16BitGet(&msg.ack_msg, &nParam16, 0x0, 0x1);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   /* Set Ack Dp value*/
   ack.Dp = nParam16;

   /* SOC: CNFG 28 0x0 0x1*/
   MEI_AR9_CmvMsgMake(CMV_GROUP_CNFG, 28, 0x7, 0x1, IFX_TRUE, IFX_NULL, &cmv_tx, &cmv_rx, &msg);

   /* Send Socrates Cmv Message and get CMV response*/
   ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = msg.ictl.retCode;
      return ret;
   }

   /* Get CMV Ack Message Parameter*/
   ret = MEI_AR9_CmvAckParam16BitGet(&msg.ack_msg, &nParam16, 0x0, 0x1);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   /* Set Ack Kp value*/
   ack.Kp = nParam16;

   /* SOC: CNFG 24 0x1 0x1*/
   MEI_AR9_CmvMsgMake(CMV_GROUP_CNFG, 24, 0x1, 0x1, IFX_TRUE, IFX_NULL, &cmv_tx, &cmv_rx, &msg);

   /* Send Socrates Cmv Message and get CMV response*/
   ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = msg.ictl.retCode;
      return ret;
   }

   /* Get CMV Ack Message Parameter*/
   ret = MEI_AR9_CmvAckParam16BitGet(&msg.ack_msg, &nParam16, 0x0, 0x1);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   /* Set Ack Mp value*/
   ack.Mp = nParam16;

   /* SOC: CNFG 25 0x1 0x1*/
   MEI_AR9_CmvMsgMake(CMV_GROUP_CNFG, 25, 0x1, 0x1, IFX_TRUE, IFX_NULL, &cmv_tx, &cmv_rx, &msg);

   /* Send Socrates Cmv Message and get CMV response*/
   ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = msg.ictl.retCode;
      return ret;
   }

   /* Get CMV Ack Message Parameter*/
   ret = MEI_AR9_CmvAckParam16BitGet(&msg.ack_msg, &nParam16, 0x0, 0x1);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   /* Set Ack Lp value*/
   ack.Lp = nParam16;

   /* SOC: CNFG 26 0x0 0x1*/
   MEI_AR9_CmvMsgMake(CMV_GROUP_CNFG, 26, 0x1, 0x1, IFX_TRUE, IFX_NULL, &cmv_tx, &cmv_rx, &msg);

   /* Send Socrates Cmv Message and get CMV response*/
   ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = msg.ictl.retCode;
      return ret;
   }

   /* Get CMV Ack Message Parameter*/
   ret = MEI_AR9_CmvAckParam16BitGet(&msg.ack_msg, &nParam16, 0x0, 0x1);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   /* Set Ack Tp value*/
   ack.Tp = nParam16;

   /* SOC: CNFG 31 0x0 0x1*/
   MEI_AR9_CmvMsgMake(CMV_GROUP_CNFG, 31, 0x0, 0x1, IFX_TRUE, IFX_NULL, &cmv_tx, &cmv_rx, &msg);

   /* Send Socrates Cmv Message and get CMV response*/
   ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = msg.ictl.retCode;
      return ret;
   }

   /* Get CMV Ack Message Parameter*/
   ret = MEI_AR9_CmvAckParam16BitGet(&msg.ack_msg, &nParam16, 0x0, 0x1);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   /* Set Ack MSGc value*/
   ack.MSGc = nParam16;

   /* Set ACK Index*/
   ack.Index  = cmd.Index;
   /* Set ACK Length*/
   ack.Length = cmd.Length;

   /* Set User Ack*/
   ret = MEI_AR9_UserAckSet(&pUserMsgs->ack_msg, (IFX_uint8_t*) &ack,
                            sizeof(ACK_ADSL_FrameDataDS_LP1Get_t), bInternCall);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   pUserMsgs->ack_msg.paylSize_byte = sizeof(ACK_ADSL_FrameDataDS_LP1Get_t);

   pUserMsgs->ictl.retCode = ret;

   return ret;
}

/**
   CMD_ADSL_FrameDataUS_LP0Get firmware message handler.
*/
MEI_STATIC IFX_int32_t MEI_AR9_CMD_ADSL_FrameDataUS_LP0Get(
                              MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                              IOCTL_MEI_messageSend_t  *pUserMsgs,
                              IFX_boolean_t             bInternCall)
{
   IFX_int32_t ret = 0;
   IFX_uint16_t nParam16;
   CMD_ADSL_FrameDataUS_LP0Get_t cmd = {0};
   ACK_ADSL_FrameDataUS_LP0Get_t ack = {0};
   AR9_CMV_STD_MESSAGE_T cmv_tx, cmv_rx;
   IOCTL_MEI_messageSend_t msg;

   /* Get User Cmd */
   ret = MEI_AR9_UserCmdGet(&pUserMsgs->write_msg, (IFX_uint8_t*)&cmd,
                            sizeof(CMD_ADSL_FrameDataUS_LP0Get_t), bInternCall);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   /* SOC: CNFG 12 0x0 0x1*/
   MEI_AR9_CmvMsgMake(CMV_GROUP_CNFG, 12, 0x0, 0x1, IFX_TRUE, IFX_NULL, &cmv_tx, &cmv_rx, &msg);

   /* Send Socrates Cmv Message and get CMV response*/
   ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = msg.ictl.retCode;
      return ret;
   }

   /* Get CMV Ack Message Parameter*/
   ret = MEI_AR9_CmvAckParam16BitGet(&msg.ack_msg, &nParam16, 0x0, 0x1);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   /* Set Ack Rp value*/
   ack.Rp_LP0us = nParam16;

   /* SOC: CNFG 16 0x0 0x1*/
   MEI_AR9_CmvMsgMake(CMV_GROUP_CNFG, 16, 0x0, 0x1, IFX_TRUE, IFX_NULL, &cmv_tx, &cmv_rx, &msg);

   /* Send Socrates Cmv Message and get CMV response*/
   ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = msg.ictl.retCode;
      return ret;
   }

   /* Get CMV Ack Message Parameter*/
   ret = MEI_AR9_CmvAckParam16BitGet(&msg.ack_msg, &nParam16, 0x0, 0x1);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   /* Set Ack Dp value*/
   ack.Dp = nParam16;

   /* SOC: CNFG 17 0x0 0x1*/
   MEI_AR9_CmvMsgMake(CMV_GROUP_CNFG, 17, 0x0, 0x1, IFX_TRUE, IFX_NULL, &cmv_tx, &cmv_rx, &msg);

   /* Send Socrates Cmv Message and get CMV response*/
   ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = msg.ictl.retCode;
      return ret;
   }

   /* Get CMV Ack Message Parameter*/
   ret = MEI_AR9_CmvAckParam16BitGet(&msg.ack_msg, &nParam16, 0x0, 0x1);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   /* Set Ack Kp value*/
   ack.Kp = nParam16;

   /* SOC: CNFG 13 0x0 0x1*/
   MEI_AR9_CmvMsgMake(CMV_GROUP_CNFG, 13, 0x0, 0x1, IFX_TRUE, IFX_NULL, &cmv_tx, &cmv_rx, &msg);

   /* Send Socrates Cmv Message and get CMV response*/
   ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = msg.ictl.retCode;
      return ret;
   }

   /* Get CMV Ack Message Parameter*/
   ret = MEI_AR9_CmvAckParam16BitGet(&msg.ack_msg, &nParam16, 0x0, 0x1);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   /* Set Ack Mp value*/
   ack.Mp = nParam16;

   /* SOC: CNFG 14 0x0 0x1*/
   MEI_AR9_CmvMsgMake(CMV_GROUP_CNFG, 14, 0x0, 0x1, IFX_TRUE, IFX_NULL, &cmv_tx, &cmv_rx, &msg);

   /* Send Socrates Cmv Message and get CMV response*/
   ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = msg.ictl.retCode;
      return ret;
   }

   /* Get CMV Ack Message Parameter*/
   ret = MEI_AR9_CmvAckParam16BitGet(&msg.ack_msg, &nParam16, 0x0, 0x1);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   /* Set Ack Lp value*/
   ack.Lp = nParam16;

   /* SOC: CNFG 15 0x0 0x1*/
   MEI_AR9_CmvMsgMake(CMV_GROUP_CNFG, 15, 0x0, 0x1, IFX_TRUE, IFX_NULL, &cmv_tx, &cmv_rx, &msg);

   /* Send Socrates Cmv Message and get CMV response*/
   ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = msg.ictl.retCode;
      return ret;
   }

   /* Get CMV Ack Message Parameter*/
   ret = MEI_AR9_CmvAckParam16BitGet(&msg.ack_msg, &nParam16, 0x0, 0x1);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   /* Set Ack Tp value*/
   ack.Tp = nParam16;

   /* SOC: CNFG 20 0x0 0x1*/
   MEI_AR9_CmvMsgMake(CMV_GROUP_CNFG, 20, 0x0, 0x1, IFX_TRUE, IFX_NULL, &cmv_tx, &cmv_rx, &msg);

   /* Send Socrates Cmv Message and get CMV response*/
   ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = msg.ictl.retCode;
      return ret;
   }

   /* Get CMV Ack Message Parameter*/
   ret = MEI_AR9_CmvAckParam16BitGet(&msg.ack_msg, &nParam16, 0x0, 0x1);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   /* Set Ack MSGc value*/
   ack.MSGc = nParam16;

   /* Set ACK Index*/
   ack.Index  = cmd.Index;
   /* Set ACK Length*/
   ack.Length = cmd.Length;

   /* Set User Ack*/
   ret = MEI_AR9_UserAckSet(&pUserMsgs->ack_msg, (IFX_uint8_t*) &ack,
                            sizeof(ACK_ADSL_FrameDataUS_LP0Get_t), bInternCall);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   pUserMsgs->ack_msg.paylSize_byte = sizeof(ACK_ADSL_FrameDataUS_LP0Get_t);

   pUserMsgs->ictl.retCode = ret;

   return ret;
}

/**
   CMD_ADSL_FrameDataUS_LP1Get firmware message handler.
*/
MEI_STATIC IFX_int32_t MEI_AR9_CMD_ADSL_FrameDataUS_LP1Get(
                              MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                              IOCTL_MEI_messageSend_t  *pUserMsgs,
                              IFX_boolean_t             bInternCall)
{
   IFX_int32_t ret = 0;
   IFX_uint16_t nParam16;
   CMD_ADSL_FrameDataUS_LP1Get_t cmd = {0};
   ACK_ADSL_FrameDataUS_LP1Get_t ack = {0};
   AR9_CMV_STD_MESSAGE_T cmv_tx, cmv_rx;
   IOCTL_MEI_messageSend_t msg;

   /* Get User Cmd */
   ret = MEI_AR9_UserCmdGet(&pUserMsgs->write_msg, (IFX_uint8_t*)&cmd,
                            sizeof(CMD_ADSL_FrameDataUS_LP1Get_t), bInternCall);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   /* SOC: CNFG 12 0x1 0x1*/
   MEI_AR9_CmvMsgMake(CMV_GROUP_CNFG, 12, 0x1, 0x1, IFX_TRUE, IFX_NULL, &cmv_tx, &cmv_rx, &msg);

   /* Send Socrates Cmv Message and get CMV response*/
   ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = msg.ictl.retCode;
      return ret;
   }

   /* Get CMV Ack Message Parameter*/
   ret = MEI_AR9_CmvAckParam16BitGet(&msg.ack_msg, &nParam16, 0x0, 0x1);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   /* Set Ack Rp value*/
   ack.Rp_LP0us = nParam16;

   /* SOC: CNFG 16 0x1 0x1*/
   MEI_AR9_CmvMsgMake(CMV_GROUP_CNFG, 16, 0x1, 0x1, IFX_TRUE, IFX_NULL, &cmv_tx, &cmv_rx, &msg);

   /* Send Socrates Cmv Message and get CMV response*/
   ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = msg.ictl.retCode;
      return ret;
   }

   /* Get CMV Ack Message Parameter*/
   ret = MEI_AR9_CmvAckParam16BitGet(&msg.ack_msg, &nParam16, 0x0, 0x1);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   /* Set Ack Dp value*/
   ack.Dp = nParam16;

   /* SOC: CNFG 17 0x2 0x1*/
   MEI_AR9_CmvMsgMake(CMV_GROUP_CNFG, 17, 0x2, 0x1, IFX_TRUE, IFX_NULL, &cmv_tx, &cmv_rx, &msg);

   /* Send Socrates Cmv Message and get CMV response*/
   ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = msg.ictl.retCode;
      return ret;
   }

   /* Get CMV Ack Message Parameter*/
   ret = MEI_AR9_CmvAckParam16BitGet(&msg.ack_msg, &nParam16, 0x0, 0x1);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   /* Set Ack Kp value*/
   ack.Kp = nParam16;

   /* SOC: CNFG 13 0x1 0x1*/
   MEI_AR9_CmvMsgMake(CMV_GROUP_CNFG, 13, 0x1, 0x1, IFX_TRUE, IFX_NULL, &cmv_tx, &cmv_rx, &msg);

   /* Send Socrates Cmv Message and get CMV response*/
   ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = msg.ictl.retCode;
      return ret;
   }

   /* Get CMV Ack Message Parameter*/
   ret = MEI_AR9_CmvAckParam16BitGet(&msg.ack_msg, &nParam16, 0x0, 0x1);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   /* Set Ack Mp value*/
   ack.Mp = nParam16;

   /* SOC: CNFG 14 0x1 0x1*/
   MEI_AR9_CmvMsgMake(CMV_GROUP_CNFG, 14, 0x1, 0x1, IFX_TRUE, IFX_NULL, &cmv_tx, &cmv_rx, &msg);

   /* Send Socrates Cmv Message and get CMV response*/
   ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = msg.ictl.retCode;
      return ret;
   }

   /* Get CMV Ack Message Parameter*/
   ret = MEI_AR9_CmvAckParam16BitGet(&msg.ack_msg, &nParam16, 0x0, 0x1);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   /* Set Ack Lp value*/
   ack.Lp = nParam16;

   /* SOC: CNFG 15 0x1 0x1*/
   MEI_AR9_CmvMsgMake(CMV_GROUP_CNFG, 15, 0x1, 0x1, IFX_TRUE, IFX_NULL, &cmv_tx, &cmv_rx, &msg);

   /* Send Socrates Cmv Message and get CMV response*/
   ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = msg.ictl.retCode;
      return ret;
   }

   /* Get CMV Ack Message Parameter*/
   ret = MEI_AR9_CmvAckParam16BitGet(&msg.ack_msg, &nParam16, 0x0, 0x1);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   /* Set Ack Tp value*/
   ack.Tp = nParam16;

   /* SOC: CNFG 20 0x0 0x1*/
   MEI_AR9_CmvMsgMake(CMV_GROUP_CNFG, 20, 0x0, 0x1, IFX_TRUE, IFX_NULL, &cmv_tx, &cmv_rx, &msg);

   /* Send Socrates Cmv Message and get CMV response*/
   ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = msg.ictl.retCode;
      return ret;
   }

   /* Get CMV Ack Message Parameter*/
   ret = MEI_AR9_CmvAckParam16BitGet(&msg.ack_msg, &nParam16, 0x0, 0x1);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   /* Set Ack MSGc value*/
   ack.MSGc = nParam16;

   /* Set ACK Index*/
   ack.Index  = cmd.Index;
   /* Set ACK Length*/
   ack.Length = cmd.Length;

   /* Set User Ack*/
   ret = MEI_AR9_UserAckSet(&pUserMsgs->ack_msg, (IFX_uint8_t*) &ack,
                            sizeof(ACK_ADSL_FrameDataUS_LP1Get_t), bInternCall);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   pUserMsgs->ack_msg.paylSize_byte = sizeof(ACK_ADSL_FrameDataUS_LP1Get_t);

   pUserMsgs->ictl.retCode = ret;

   return ret;
}

/**
   CMD_ADSL_BAT_US_Get firmware message handler.
*/
MEI_STATIC IFX_int32_t MEI_AR9_CMD_ADSL_BAT_US_Get(
                              MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                              IOCTL_MEI_messageSend_t  *pUserMsgs,
                              IFX_boolean_t             bInternCall)
{
   IFX_int32_t ret = 0;
   IFX_uint16_t nParam16, i, idx = 0;
   CMD_ADSL_BAT_US_Get_t cmd = {0};
   ACK_ADSL_BAT_US_Get_t ack = {0};
   AR9_CMV_STD_MESSAGE_T cmv_tx, cmv_rx;
   IOCTL_MEI_messageSend_t msg;

   /* Get User Cmd */
   ret = MEI_AR9_UserCmdGet(&pUserMsgs->write_msg, (IFX_uint8_t*)&cmd,
                            sizeof(CMD_ADSL_BAT_US_Get_t), bInternCall);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   if (cmd.Index > 31 || cmd.Length > 32)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   idx = cmd.Index;

   for (i = 0; i < cmd.Length; i++)
   {
      MEI_AR9_CmvMsgMake(CMV_GROUP_INFO, 22, idx, 0x1, IFX_TRUE, IFX_NULL, &cmv_tx, &cmv_rx, &msg);

      /* Send Socrates Cmv Message and get CMV response*/
      ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
      if (ret < 0)
      {
         pUserMsgs->ictl.retCode = msg.ictl.retCode;
         return ret;
      }

      /* Get CMV Ack Message Parameter*/
      ret = MEI_AR9_CmvAckParam16BitGet(&msg.ack_msg, &nParam16, 0x0, 0x1);
      if (ret < 0)
      {
         pUserMsgs->ictl.retCode = ret;
         return ret;
      }

      ack.BAT[i].data_01 = (IFX_uint8_t)((nParam16 >> 8) & 0xFF);
      ack.BAT[i].data_00 = (IFX_uint8_t)((nParam16) & 0xFF);

      idx++;
   }

   /* Set ACK Index*/
   ack.Index  = cmd.Index;
   /* Set ACK Length*/
   ack.Length = cmd.Length;


   /* Set User Ack*/
   ret = MEI_AR9_UserAckSet(&pUserMsgs->ack_msg, (IFX_uint8_t*) &ack,
                            sizeof(ACK_ADSL_BAT_US_Get_t), bInternCall);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   pUserMsgs->ack_msg.paylSize_byte = sizeof(MEI_UserMsgHeader_t) + ack.Length * sizeof(IFX_uint16_t);

   pUserMsgs->ictl.retCode = ret;

   return ret;
}

/**
   CMD_ADSL_BAT_DS_Get firmware message handler.
*/
MEI_STATIC IFX_int32_t MEI_AR9_CMD_ADSL_BAT_DS_Get(
                              MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                              IOCTL_MEI_messageSend_t  *pUserMsgs,
                              IFX_boolean_t             bInternCall)
{
   IFX_int32_t ret = 0;
   IFX_uint16_t nParam16, i, idx = 0;
   CMD_ADSL_BAT_DS_Get_t cmd = {0};
   ACK_ADSL_BAT_DS_Get_t ack = {0};
   AR9_CMV_STD_MESSAGE_T cmv_tx, cmv_rx;
   IOCTL_MEI_messageSend_t msg;

   /* Get User Cmd */
   ret = MEI_AR9_UserCmdGet(&pUserMsgs->write_msg, (IFX_uint8_t*)&cmd,
                            sizeof(CMD_ADSL_BAT_DS_Get_t), bInternCall);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   if (cmd.Index > 255 || cmd.Length > 128)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   idx = cmd.Index;

   for (i = 0; i < cmd.Length; i++)
   {
      MEI_AR9_CmvMsgMake(CMV_GROUP_INFO, 23, idx, 0x1, IFX_TRUE, IFX_NULL, &cmv_tx, &cmv_rx, &msg);

      /* Send Socrates Cmv Message and get CMV response*/
      ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
      if (ret < 0)
      {
         pUserMsgs->ictl.retCode = msg.ictl.retCode;
         return ret;
      }

      /* Get CMV Ack Message Parameter*/
      ret = MEI_AR9_CmvAckParam16BitGet(&msg.ack_msg, &nParam16, 0x0, 0x1);
      if (ret < 0)
      {
         pUserMsgs->ictl.retCode = ret;
         return ret;
      }

      ack.BAT[i].data_01 = (IFX_uint8_t)((nParam16 >> 8) & 0xFF);
      ack.BAT[i].data_00 = (IFX_uint8_t)((nParam16) & 0xFF);

      idx++;
   }

   /* Set ACK Index*/
   ack.Index  = cmd.Index;
   /* Set ACK Length*/
   ack.Length = cmd.Length;


   /* Set User Ack*/
   ret = MEI_AR9_UserAckSet(&pUserMsgs->ack_msg, (IFX_uint8_t*) &ack,
                            sizeof(ACK_ADSL_BAT_DS_Get_t), bInternCall);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   pUserMsgs->ack_msg.paylSize_byte = sizeof(MEI_UserMsgHeader_t) + ack.Length * sizeof(IFX_uint16_t);

   pUserMsgs->ictl.retCode = ret;

   return ret;
}

/**
   CMD_SNR_NE_TableEntriesGet firmware message handler.
*/
MEI_STATIC IFX_int32_t MEI_AR9_CMD_SNR_NE_TableEntriesGet(
                              MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                              IOCTL_MEI_messageSend_t  *pUserMsgs,
                              IFX_boolean_t             bInternCall)
{
   IFX_int32_t ret = 0;
   IFX_uint16_t nParam16, i, idx = 0;
   CMD_SNR_NE_TableEntriesGet_t cmd = {0};
   ACK_SNR_NE_TableEntriesGet_t ack = {0};
   AR9_CMV_STD_MESSAGE_T cmv_tx, cmv_rx;
   IOCTL_MEI_messageSend_t msg;

   /* Get User Cmd */
   ret = MEI_AR9_UserCmdGet(&pUserMsgs->write_msg, (IFX_uint8_t*)&cmd,
                            sizeof(CMD_SNR_NE_TableEntriesGet_t), bInternCall);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   if (cmd.Index > 511 || cmd.Length > 128)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   idx = cmd.Index;

   for (i = 0; i < cmd.Length; i++)
   {
      MEI_AR9_CmvMsgMake(CMV_GROUP_INFO, 11, idx, 0x1, IFX_TRUE, IFX_NULL, &cmv_tx, &cmv_rx, &msg);

      /* Send Socrates Cmv Message and get CMV response*/
      ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
      if (ret < 0)
      {
         pUserMsgs->ictl.retCode = msg.ictl.retCode;
         return ret;
      }

      /* Get CMV Ack Message Parameter*/
      ret = MEI_AR9_CmvAckParam16BitGet(&msg.ack_msg, &nParam16, 0x0, 0x1);
      if (ret < 0)
      {
         pUserMsgs->ictl.retCode = ret;
         return ret;
      }

      ack.SNRps[i] = nParam16;

      idx++;
   }

   /* Set ACK Index*/
   ack.Index  = cmd.Index;
   /* Set ACK Length*/
   ack.Length = cmd.Length;


   /* Set User Ack*/
   ret = MEI_AR9_UserAckSet(&pUserMsgs->ack_msg, (IFX_uint8_t*) &ack,
                            sizeof(ACK_SNR_NE_TableEntriesGet_t), bInternCall);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   pUserMsgs->ack_msg.paylSize_byte = sizeof(MEI_UserMsgHeader_t) + ack.Length * sizeof(IFX_uint16_t);

   pUserMsgs->ictl.retCode = ret;

   return ret;
}

/**
   CMD_ADSL_GainTableUS_Get firmware message handler.
*/
MEI_STATIC IFX_int32_t MEI_AR9_CMD_ADSL_GainTableUS_Get(
                              MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                              IOCTL_MEI_messageSend_t  *pUserMsgs,
                              IFX_boolean_t             bInternCall)
{
   IFX_int32_t ret = 0;
   IFX_uint16_t nParam16, i, idx = 0;
   CMD_ADSL_GainTableUS_Get_t cmd = {0};
   ACK_ADSL_GainTableUS_Get_t ack = {0};
   AR9_CMV_STD_MESSAGE_T cmv_tx, cmv_rx;
   IOCTL_MEI_messageSend_t msg;

   /* Get User Cmd */
   ret = MEI_AR9_UserCmdGet(&pUserMsgs->write_msg, (IFX_uint8_t*)&cmd,
                            sizeof(CMD_ADSL_GainTableUS_Get_t), bInternCall);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   if (cmd.Index > 63 || cmd.Length > 64)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   idx = cmd.Index;

   for (i = 0; i < cmd.Length; i++)
   {
      MEI_AR9_CmvMsgMake(CMV_GROUP_INFO, 24, idx, 0x1, IFX_TRUE, IFX_NULL, &cmv_tx, &cmv_rx, &msg);

      /* Send Socrates Cmv Message and get CMV response*/
      ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
      if (ret < 0)
      {
         pUserMsgs->ictl.retCode = msg.ictl.retCode;
         return ret;
      }

      /* Get CMV Ack Message Parameter*/
      ret = MEI_AR9_CmvAckParam16BitGet(&msg.ack_msg, &nParam16, 0x0, 0x1);
      if (ret < 0)
      {
         pUserMsgs->ictl.retCode = ret;
         return ret;
      }

      ack.Gains[i] = nParam16;

      idx++;
   }

   /* Set ACK Index*/
   ack.Index  = cmd.Index;
   /* Set ACK Length*/
   ack.Length = cmd.Length;


   /* Set User Ack*/
   ret = MEI_AR9_UserAckSet(&pUserMsgs->ack_msg, (IFX_uint8_t*) &ack,
                            sizeof(ACK_ADSL_GainTableUS_Get_t), bInternCall);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   pUserMsgs->ack_msg.paylSize_byte = sizeof(MEI_UserMsgHeader_t) + ack.Length * sizeof(IFX_uint16_t);

   pUserMsgs->ictl.retCode = ret;

   return ret;
}

/**
   CMD_ADSL_GainTableDS_Get firmware message handler.
*/
MEI_STATIC IFX_int32_t MEI_AR9_CMD_ADSL_GainTableDS_Get(
                              MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                              IOCTL_MEI_messageSend_t  *pUserMsgs,
                              IFX_boolean_t             bInternCall)
{
   IFX_int32_t ret = 0;
   IFX_uint16_t nParam16, i, idx = 0;
   CMD_ADSL_GainTableDS_Get_t cmd = {0};
   ACK_ADSL_GainTableDS_Get_t ack = {0};
   AR9_CMV_STD_MESSAGE_T cmv_tx, cmv_rx;
   IOCTL_MEI_messageSend_t msg;

   /* Get User Cmd */
   ret = MEI_AR9_UserCmdGet(&pUserMsgs->write_msg, (IFX_uint8_t*)&cmd,
                            sizeof(CMD_ADSL_GainTableDS_Get_t), bInternCall);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   if (cmd.Index > 511 || cmd.Length > 128)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   idx = cmd.Index;

   for (i = 0; i < cmd.Length; i++)
   {
      MEI_AR9_CmvMsgMake(CMV_GROUP_INFO, 25, idx, 0x1, IFX_TRUE, IFX_NULL, &cmv_tx, &cmv_rx, &msg);

      /* Send Socrates Cmv Message and get CMV response*/
      ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
      if (ret < 0)
      {
         pUserMsgs->ictl.retCode = msg.ictl.retCode;
         return ret;
      }

      /* Get CMV Ack Message Parameter*/
      ret = MEI_AR9_CmvAckParam16BitGet(&msg.ack_msg, &nParam16, 0x0, 0x1);
      if (ret < 0)
      {
         pUserMsgs->ictl.retCode = ret;
         return ret;
      }

      ack.Gains[i] = nParam16;

      idx++;
   }

   /* Set ACK Index*/
   ack.Index  = cmd.Index;
   /* Set ACK Length*/
   ack.Length = cmd.Length;


   /* Set User Ack*/
   ret = MEI_AR9_UserAckSet(&pUserMsgs->ack_msg, (IFX_uint8_t*) &ack,
                            sizeof(ACK_ADSL_GainTableDS_Get_t), bInternCall);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   pUserMsgs->ack_msg.paylSize_byte = sizeof(MEI_UserMsgHeader_t) + ack.Length * sizeof(IFX_uint16_t);

   pUserMsgs->ictl.retCode = ret;

   return ret;
}

/**
   CMD_LineStatusDS_Get firmware message handler.
*/
MEI_STATIC IFX_int32_t MEI_AR9_CMD_LineStatusDS_Get(
                              MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                              IOCTL_MEI_messageSend_t  *pUserMsgs,
                              IFX_boolean_t             bInternCall)
{
   IFX_int32_t ret = 0;
   IFX_uint16_t nParam16[7];
   CMD_LineStatusDS_Get_t cmd = {0};
   ACK_LineStatusDS_Get_t ack = {0};
   AR9_CMV_STD_MESSAGE_T cmv_tx, cmv_rx;
   IOCTL_MEI_messageSend_t msg;

   /* Get User Cmd */
   ret = MEI_AR9_UserCmdGet(&pUserMsgs->write_msg, (IFX_uint8_t*)&cmd,
                            sizeof(CMD_LineStatusDS_Get_t), bInternCall);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   /* SOC: INFO 68 0x0 0x7*/
   MEI_AR9_CmvMsgMake(CMV_GROUP_INFO, 68, 0x0, 0x7, IFX_TRUE, IFX_NULL, &cmv_tx, &cmv_rx, &msg);

   /* Send Socrates Cmv Message and get CMV response*/
   ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = msg.ictl.retCode;
      return ret;
   }

   /* Get CMV Ack Message Parameter*/
   ret = MEI_AR9_CmvAckParam16BitGet(&msg.ack_msg, &nParam16[0], 0x0, 0x7);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   ack.LATNds       = nParam16[1];
   ack.SATNds       = nParam16[2];
   ack.ATTNDRds_LSW = nParam16[4];
   ack.ATTNDRds_MSW = nParam16[5];

   /* SOC: PLAM 45 0x0 0x1*/
   MEI_AR9_CmvMsgMake(CMV_GROUP_PLAM, 45, 0x0, 0x1, IFX_TRUE, IFX_NULL, &cmv_tx, &cmv_rx, &msg);

   /* Send Socrates Cmv Message and get CMV response*/
   ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = msg.ictl.retCode;
      return ret;
   }

   /* Get CMV Ack Message Parameter*/
   ret = MEI_AR9_CmvAckParam16BitGet(&msg.ack_msg, &nParam16[0], 0x0, 0x1);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   ack.SNRMds = nParam16[0];

   /* Set ACK Index*/
   ack.Index  = cmd.Index;
   /* Set ACK Length*/
   ack.Length = cmd.Length;

   /* Set User Ack*/
   ret = MEI_AR9_UserAckSet(&pUserMsgs->ack_msg, (IFX_uint8_t*) &ack,
                            sizeof(ACK_LineStatusDS_Get_t), bInternCall);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   pUserMsgs->ack_msg.paylSize_byte = sizeof(ACK_LineStatusDS_Get_t) - 6;

   pUserMsgs->ictl.retCode = ret;

   return ret;
}

/**
   CMD_LineStatusUS_Get firmware message handler.
*/
MEI_STATIC IFX_int32_t MEI_AR9_CMD_LineStatusUS_Get(
                              MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                              IOCTL_MEI_messageSend_t  *pUserMsgs,
                              IFX_boolean_t             bInternCall)
{
   IFX_int32_t ret = 0;
   IFX_uint16_t nParam16[7];
   CMD_LineStatusUS_Get_t cmd = {0};
   ACK_LineStatusUS_Get_t ack = {0};
   AR9_CMV_STD_MESSAGE_T cmv_tx, cmv_rx;
   IOCTL_MEI_messageSend_t msg;

   /* Get User Cmd */
   ret = MEI_AR9_UserCmdGet(&pUserMsgs->write_msg, (IFX_uint8_t*)&cmd,
                            sizeof(CMD_LineStatusUS_Get_t), bInternCall);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   /* SOC: INFO 69 0x0 0x7*/
   MEI_AR9_CmvMsgMake(CMV_GROUP_INFO, 69, 0x0, 0x7, IFX_TRUE, IFX_NULL, &cmv_tx, &cmv_rx, &msg);

   /* Send Socrates Cmv Message and get CMV response*/
   ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = msg.ictl.retCode;
      return ret;
   }

   /* Get CMV Ack Message Parameter*/
   ret = MEI_AR9_CmvAckParam16BitGet(&msg.ack_msg, &nParam16[0], 0x0, 0x7);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   ack.LATNus       = nParam16[1];
   ack.SATNus       = nParam16[2];
   ack.ATTNDRus_LSW = nParam16[4];
   ack.ATTNDRus_MSW = nParam16[5];
   ack.SNRMus       = nParam16[3];

   /* Set ACK Index*/
   ack.Index  = cmd.Index;
   /* Set ACK Length*/
   ack.Length = cmd.Length;

   /* Set User Ack*/
   ret = MEI_AR9_UserAckSet(&pUserMsgs->ack_msg, (IFX_uint8_t*) &ack,
                            sizeof(ACK_LineStatusUS_Get_t), bInternCall);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   pUserMsgs->ack_msg.paylSize_byte = sizeof(ACK_LineStatusUS_Get_t) - 6;

   pUserMsgs->ictl.retCode = ret;

   return ret;
}

/**
   CMD_CRC_StatsNE_NoTR1Get firmware message handler.
*/
MEI_STATIC IFX_int32_t MEI_AR9_CMD_CRC_StatsNE_NoTR1Get(
                              MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                              IOCTL_MEI_messageSend_t  *pUserMsgs,
                              IFX_boolean_t             bInternCall)
{
   IFX_int32_t ret = 0;
   IFX_uint16_t nParam16;
   CMD_CRC_StatsNE_NoTR1Get_t cmd = {0};
   ACK_CRC_StatsNE_NoTR1Get_t ack = {0};
   AR9_CMV_STD_MESSAGE_T cmv_tx, cmv_rx;
   IOCTL_MEI_messageSend_t msg;

   /* Get User Cmd */
   ret = MEI_AR9_UserCmdGet(&pUserMsgs->write_msg, (IFX_uint8_t*)&cmd,
                            sizeof(CMD_CRC_StatsNE_NoTR1Get_t), bInternCall);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   /* SOC: PLAM 2 0x0 0x1*/
   MEI_AR9_CmvMsgMake(CMV_GROUP_PLAM, 2, 0x0, 0x1, IFX_TRUE, IFX_NULL, &cmv_tx, &cmv_rx, &msg);

   /* Send Socrates Cmv Message and get CMV response*/
   ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = msg.ictl.retCode;
      return ret;
   }

   /* Get CMV Ack Message Parameter*/
   ret = MEI_AR9_CmvAckParam16BitGet(&msg.ack_msg, &nParam16, 0x0, 0x1);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }
   ack.cntCVI_LSW = nParam16;

   /* SOC: PLAM 4 0x0 0x1*/
   MEI_AR9_CmvMsgMake(CMV_GROUP_PLAM, 4, 0x0, 0x1, IFX_TRUE, IFX_NULL, &cmv_tx, &cmv_rx, &msg);

   /* Send Socrates Cmv Message and get CMV response*/
   ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = msg.ictl.retCode;
      return ret;
   }

   /* Get CMV Ack Message Parameter*/
   ret = MEI_AR9_CmvAckParam16BitGet(&msg.ack_msg, &nParam16, 0x0, 0x1);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   ack.cntCVI_MSW = nParam16;

   /* Set ACK Index*/
   ack.Index  = cmd.Index;
   /* Set ACK Length*/
   ack.Length = cmd.Length;

   /* Set User Ack*/
   ret = MEI_AR9_UserAckSet(&pUserMsgs->ack_msg, (IFX_uint8_t*) &ack,
                            sizeof(ACK_CRC_StatsNE_NoTR1Get_t), bInternCall);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   pUserMsgs->ack_msg.paylSize_byte = sizeof(ACK_CRC_StatsNE_NoTR1Get_t);

   pUserMsgs->ictl.retCode = ret;

   return ret;
}

/**
   CMD_CRC_StatsNE_NoTR1Set firmware message handler.
*/
MEI_STATIC IFX_int32_t MEI_AR9_CMD_CRC_StatsNE_NoTR1Set(
                              MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                              IOCTL_MEI_messageSend_t  *pUserMsgs,
                              IFX_boolean_t             bInternCall)
{
   IFX_int32_t ret = 0;
   IFX_uint16_t nParam16;
   CMD_CRC_StatsNE_NoTR1Set_t cmd = {0};
   ACK_CRC_StatsNE_NoTR1Set_t ack = {0};
   AR9_CMV_STD_MESSAGE_T cmv_tx, cmv_rx;
   IOCTL_MEI_messageSend_t msg;

   /* Get User Cmd */
   ret = MEI_AR9_UserCmdGet(&pUserMsgs->write_msg, (IFX_uint8_t*)&cmd,
                            sizeof(CMD_CRC_StatsNE_NoTR1Set_t), bInternCall);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   nParam16 = cmd.cntCVI_LSW;

   /* SOC: PLAM 2 0x0 0x1*/
   MEI_AR9_CmvMsgMake(CMV_GROUP_PLAM, 2, 0x0, 0x1, IFX_FALSE, &nParam16, &cmv_tx, &cmv_rx, &msg);

   /* Send Socrates Cmv Message and get CMV response*/
   ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = msg.ictl.retCode;
      return ret;
   }

   nParam16 = cmd.cntCVI_MSW;

   /* SOC: PLAM 4 0x0 0x1*/
   MEI_AR9_CmvMsgMake(CMV_GROUP_PLAM, 4, 0x0, 0x1, IFX_FALSE, &nParam16, &cmv_tx, &cmv_rx, &msg);

   /* Send Socrates Cmv Message and get CMV response*/
   ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = msg.ictl.retCode;
      return ret;
   }

   /* Set ACK Index*/
   ack.Index  = cmd.Index;
   /* Set ACK Length*/
   ack.Length = cmd.Length;

   /* Set User Ack*/
   ret = MEI_AR9_UserAckSet(&pUserMsgs->ack_msg, (IFX_uint8_t*) &ack,
                            sizeof(ACK_CRC_StatsNE_NoTR1Set_t), bInternCall);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   pUserMsgs->ack_msg.paylSize_byte = sizeof(ACK_CRC_StatsNE_NoTR1Set_t);

   pUserMsgs->ictl.retCode = ret;

   return ret;
}

/**
   CMD_CRC_StatsFE_NoTR1Get firmware message handler.
*/
MEI_STATIC IFX_int32_t MEI_AR9_CMD_CRC_StatsFE_NoTR1Get(
                              MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                              IOCTL_MEI_messageSend_t  *pUserMsgs,
                              IFX_boolean_t             bInternCall)
{
   IFX_int32_t ret = 0;
   IFX_uint16_t nParam16;
   CMD_CRC_StatsFE_NoTR1Get_t cmd = {0};
   ACK_CRC_StatsFE_NoTR1Get_t ack = {0};
   AR9_CMV_STD_MESSAGE_T cmv_tx, cmv_rx;
   IOCTL_MEI_messageSend_t msg;

   IFX_boolean_t bAdsl1 = IFX_FALSE, bAdsl2p = IFX_FALSE;
   IFX_uint16_t hdlcCmd[2] = {0};
   IFX_uint16_t hdlcRxBuffer[29] = {0};

   /* Get User Cmd */
   ret = MEI_AR9_UserCmdGet(&pUserMsgs->write_msg, (IFX_uint8_t*)&cmd,
                            sizeof(CMD_CRC_StatsFE_NoTR1Get_t), bInternCall);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   /* Check for the ADSL1 mode*/
   ret = MEI_AR9_AdslModeStatusGet(pMeiDynCntrl, &bAdsl1, &bAdsl2p);
   if (ret < 0)
   {
      return -1;
   }

   if (bAdsl1)
   {
      /* SOC: PLAM 24 0x0 0x1*/
      MEI_AR9_CmvMsgMake(CMV_GROUP_PLAM, 24, 0x0, 0x1, IFX_TRUE, IFX_NULL, &cmv_tx, &cmv_rx, &msg);

      /* Send Socrates Cmv Message and get CMV response*/
      ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
      if (ret < 0)
      {
         pUserMsgs->ictl.retCode = msg.ictl.retCode;
         return ret;
      }

      /* Get CMV Ack Message Parameter*/
      ret = MEI_AR9_CmvAckParam16BitGet(&msg.ack_msg, &nParam16, 0x0, 0x1);
      if (ret < 0)
      {
         pUserMsgs->ictl.retCode = ret;
         return ret;
      }

      ack.cntCVI_LSW = nParam16;
      ack.cntCVI_MSW = 0;
   }
   else
   {
      hdlcCmd[0] = 0x105;

      ret = MEI_AR9_HDLC_MsgHandle(
                           pMeiDynCntrl,
                           (IFX_uint8_t*) hdlcCmd, 2,
                           (IFX_uint8_t*) hdlcRxBuffer, sizeof(hdlcRxBuffer));
      if (ret < 0)
      {
         pUserMsgs->ictl.retCode = -1;
         return ret;
      }

      ack.cntCVI_LSW = SWAP16_BYTE_ORDER(hdlcRxBuffer[2]);
      ack.cntCVI_MSW = SWAP16_BYTE_ORDER(hdlcRxBuffer[1]);
   }

   /* Set ACK Index*/
   ack.Index  = cmd.Index;
   /* Set ACK Length*/
   ack.Length = cmd.Length;

   /* Set User Ack*/
   ret = MEI_AR9_UserAckSet(&pUserMsgs->ack_msg, (IFX_uint8_t*) &ack,
                            sizeof(ACK_CRC_StatsFE_NoTR1Get_t), bInternCall);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   pUserMsgs->ack_msg.paylSize_byte = sizeof(ACK_CRC_StatsFE_NoTR1Get_t);

   pUserMsgs->ictl.retCode = ret;

   return ret;
}

/**
   CMD_FEC_StatsNE_NoTR1Get firmware message handler.
*/
MEI_STATIC IFX_int32_t MEI_AR9_CMD_FEC_StatsNE_NoTR1Get(
                              MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                              IOCTL_MEI_messageSend_t  *pUserMsgs,
                              IFX_boolean_t             bInternCall)
{
   IFX_int32_t ret = 0;
   IFX_uint16_t nParam16;
   CMD_FEC_StatsNE_NoTR1Get_t cmd = {0};
   ACK_FEC_StatsNE_NoTR1Get_t ack = {0};
   AR9_CMV_STD_MESSAGE_T cmv_tx, cmv_rx;
   IOCTL_MEI_messageSend_t msg;

   /* Get User Cmd */
   ret = MEI_AR9_UserCmdGet(&pUserMsgs->write_msg, (IFX_uint8_t*)&cmd,
                            sizeof(CMD_FEC_StatsNE_NoTR1Get_t), bInternCall);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   /* SOC: PLAM 3 0x0 0x1*/
   MEI_AR9_CmvMsgMake(CMV_GROUP_PLAM, 3, 0x0, 0x1, IFX_TRUE, IFX_NULL, &cmv_tx, &cmv_rx, &msg);

   /* Send Socrates Cmv Message and get CMV response*/
   ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = msg.ictl.retCode;
      return ret;
   }

   /* Get CMV Ack Message Parameter*/
   ret = MEI_AR9_CmvAckParam16BitGet(&msg.ack_msg, &nParam16, 0x0, 0x1);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }
   ack.cntECI_LSW = nParam16;

   /* SOC: PLAM 5 0x0 0x1*/
   MEI_AR9_CmvMsgMake(CMV_GROUP_PLAM, 5, 0x0, 0x1, IFX_TRUE, IFX_NULL, &cmv_tx, &cmv_rx, &msg);

   /* Send Socrates Cmv Message and get CMV response*/
   ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = msg.ictl.retCode;
      return ret;
   }

   /* Get CMV Ack Message Parameter*/
   ret = MEI_AR9_CmvAckParam16BitGet(&msg.ack_msg, &nParam16, 0x0, 0x1);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   ack.cntECI_MSW = nParam16;

   /* Set ACK Index*/
   ack.Index  = cmd.Index;
   /* Set ACK Length*/
   ack.Length = cmd.Length;

   /* Set User Ack*/
   ret = MEI_AR9_UserAckSet(&pUserMsgs->ack_msg, (IFX_uint8_t*) &ack,
                            sizeof(ACK_FEC_StatsNE_NoTR1Get_t), bInternCall);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   pUserMsgs->ack_msg.paylSize_byte = sizeof(ACK_FEC_StatsNE_NoTR1Get_t);

   pUserMsgs->ictl.retCode = ret;

   return ret;
}

/**
   CMD_FEC_StatsNE_NoTR1Set firmware message handler.
*/
MEI_STATIC IFX_int32_t MEI_AR9_CMD_FEC_StatsNE_NoTR1Set(
                              MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                              IOCTL_MEI_messageSend_t  *pUserMsgs,
                              IFX_boolean_t             bInternCall)
{
   IFX_int32_t ret = 0;
   IFX_uint16_t nParam16;
   CMD_FEC_StatsNE_NoTR1Set_t cmd = {0};
   ACK_FEC_StatsNE_NoTR1Set_t ack = {0};
   AR9_CMV_STD_MESSAGE_T cmv_tx, cmv_rx;
   IOCTL_MEI_messageSend_t msg;

   /* Get User Cmd */
   ret = MEI_AR9_UserCmdGet(&pUserMsgs->write_msg, (IFX_uint8_t*)&cmd,
                            sizeof(CMD_FEC_StatsNE_NoTR1Set_t), bInternCall);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   nParam16 = cmd.cntECI_LSW;

   /* SOC: PLAM 3 0x0 0x1*/
   MEI_AR9_CmvMsgMake(CMV_GROUP_PLAM, 3, 0x0, 0x1, IFX_FALSE, &nParam16, &cmv_tx, &cmv_rx, &msg);

   /* Send Socrates Cmv Message and get CMV response*/
   ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = msg.ictl.retCode;
      return ret;
   }

   nParam16 = cmd.cntECI_MSW;

   /* SOC: PLAM 5 0x0 0x1*/
   MEI_AR9_CmvMsgMake(CMV_GROUP_PLAM, 5, 0x0, 0x1, IFX_FALSE, &nParam16, &cmv_tx, &cmv_rx, &msg);

   /* Send Socrates Cmv Message and get CMV response*/
   ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = msg.ictl.retCode;
      return ret;
   }

   /* Set ACK Index*/
   ack.Index  = cmd.Index;
   /* Set ACK Length*/
   ack.Length = cmd.Length;

   /* Set User Ack*/
   ret = MEI_AR9_UserAckSet(&pUserMsgs->ack_msg, (IFX_uint8_t*) &ack,
                            sizeof(ACK_FEC_StatsNE_NoTR1Set_t), bInternCall);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   pUserMsgs->ack_msg.paylSize_byte = sizeof(ACK_FEC_StatsNE_NoTR1Set_t);

   pUserMsgs->ictl.retCode = ret;

   return ret;
}

/**
   CMD_FEC_StatsFE_NoTR1Get firmware message handler.
*/
MEI_STATIC IFX_int32_t MEI_AR9_CMD_FEC_StatsFE_NoTR1Get(
                              MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                              IOCTL_MEI_messageSend_t  *pUserMsgs,
                              IFX_boolean_t             bInternCall)
{
   IFX_int32_t ret = 0;
   IFX_uint16_t nParam16;
   CMD_FEC_StatsFE_NoTR1Get_t cmd = {0};
   ACK_FEC_StatsFE_NoTR1Get_t ack = {0};
   AR9_CMV_STD_MESSAGE_T cmv_tx, cmv_rx;
   IOCTL_MEI_messageSend_t msg;

   IFX_boolean_t bAdsl1 = IFX_FALSE, bAdsl2p = IFX_FALSE;
   IFX_uint16_t hdlcCmd[2] = {0};
   IFX_uint16_t hdlcRxBuffer[29] = {0};

   /* Get User Cmd */
   ret = MEI_AR9_UserCmdGet(&pUserMsgs->write_msg, (IFX_uint8_t*)&cmd,
                            sizeof(CMD_FEC_StatsFE_NoTR1Get_t), bInternCall);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   /* Check for the ADSL1 mode*/
   ret = MEI_AR9_AdslModeStatusGet(pMeiDynCntrl, &bAdsl1, &bAdsl2p);
   if (ret < 0)
   {
      return -1;
   }

   if (bAdsl1)
   {
      /* SOC: PLAM 24 0x0 0x1*/
      MEI_AR9_CmvMsgMake(CMV_GROUP_PLAM, 28, 0x0, 0x1, IFX_TRUE, IFX_NULL, &cmv_tx, &cmv_rx, &msg);

      /* Send Socrates Cmv Message and get CMV response*/
      ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
      if (ret < 0)
      {
         pUserMsgs->ictl.retCode = msg.ictl.retCode;
         return ret;
      }

      /* Get CMV Ack Message Parameter*/
      ret = MEI_AR9_CmvAckParam16BitGet(&msg.ack_msg, &nParam16, 0x0, 0x1);
      if (ret < 0)
      {
         pUserMsgs->ictl.retCode = ret;
         return ret;
      }

      ack.cntECI_LSW = nParam16;
      ack.cntECI_MSW = 0;
   }
   else
   {
      hdlcCmd[0] = 0x105;

      ret = MEI_AR9_HDLC_MsgHandle(
                           pMeiDynCntrl,
                           (IFX_uint8_t*) hdlcCmd, 2,
                           (IFX_uint8_t*) hdlcRxBuffer, sizeof(hdlcRxBuffer));
      if (ret < 0)
      {
         pUserMsgs->ictl.retCode = -1;
         return ret;
      }

      ack.cntECI_LSW = SWAP16_BYTE_ORDER(hdlcRxBuffer[4]);
      ack.cntECI_MSW = SWAP16_BYTE_ORDER(hdlcRxBuffer[3]);
   }

   /* Set ACK Index*/
   ack.Index  = cmd.Index;
   /* Set ACK Length*/
   ack.Length = cmd.Length;

   /* Set User Ack*/
   ret = MEI_AR9_UserAckSet(&pUserMsgs->ack_msg, (IFX_uint8_t*) &ack,
                            sizeof(ACK_FEC_StatsFE_NoTR1Get_t), bInternCall);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   pUserMsgs->ack_msg.paylSize_byte = sizeof(ACK_FEC_StatsFE_NoTR1Get_t);

   pUserMsgs->ictl.retCode = ret;

   return ret;
}

/**
   CMD_G997_ATM_StatsGet firmware message handler.
*/
MEI_STATIC IFX_int32_t MEI_AR9_CMD_G997_ATM_StatsGet(
                              MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                              IOCTL_MEI_messageSend_t  *pUserMsgs,
                              IFX_boolean_t             bInternCall)
{
   IFX_int32_t ret = 0;
   IFX_uint16_t nParam16[2] = {0};
   CMD_G997_ATM_StatsGet_t cmd = {0};
   ACK_G997_ATM_StatsGet_t ack = {0};
   AR9_CMV_STD_MESSAGE_T cmv_tx, cmv_rx;
   IOCTL_MEI_messageSend_t msg;

   /* Get User Cmd */
   ret = MEI_AR9_UserCmdGet(&pUserMsgs->write_msg, (IFX_uint8_t*)&cmd,
                            sizeof(CMD_G997_ATM_StatsGet_t), bInternCall);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   /* SOC: PLAM 11 0x0 0x2*/
   MEI_AR9_CmvMsgMake(CMV_GROUP_PLAM, 11, 0x0, 0x2, IFX_TRUE, IFX_NULL, &cmv_tx, &cmv_rx, &msg);

   /* Send Socrates Cmv Message and get CMV response*/
   ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = msg.ictl.retCode;
      return ret;
   }

   /* Get CMV Ack Message Parameter*/
   ret = MEI_AR9_CmvAckParam16BitGet(&msg.ack_msg, &nParam16[0], 0x0, 0x2);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   ack.hecp = (nParam16[1] << 16) | nParam16[0];

   /* SOC: PLAM 16 0x0 0x2*/
   MEI_AR9_CmvMsgMake(CMV_GROUP_PLAM, 16, 0x0, 0x2, IFX_TRUE, IFX_NULL, &cmv_tx, &cmv_rx, &msg);

   /* Send Socrates Cmv Message and get CMV response*/
   ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = msg.ictl.retCode;
      return ret;
   }

   /* Get CMV Ack Message Parameter*/
   ret = MEI_AR9_CmvAckParam16BitGet(&msg.ack_msg, &nParam16[0], 0x0, 0x2);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   ack.cdp = (nParam16[1] << 16) | nParam16[0];

   /* SOC: PLAM 18 0x0 0x2*/
   MEI_AR9_CmvMsgMake(CMV_GROUP_PLAM, 18, 0x0, 0x2, IFX_TRUE, IFX_NULL, &cmv_tx, &cmv_rx, &msg);

   /* Send Socrates Cmv Message and get CMV response*/
   ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = msg.ictl.retCode;
      return ret;
   }

   /* Get CMV Ack Message Parameter*/
   ret = MEI_AR9_CmvAckParam16BitGet(&msg.ack_msg, &nParam16[0], 0x0, 0x2);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   ack.cup = (nParam16[1] << 16) | nParam16[0];

   /* SOC: PLAM 19 0x0 0x2*/
   MEI_AR9_CmvMsgMake(CMV_GROUP_PLAM, 19, 0x0, 0x2, IFX_TRUE, IFX_NULL, &cmv_tx, &cmv_rx, &msg);

   /* Send Socrates Cmv Message and get CMV response*/
   ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = msg.ictl.retCode;
      return ret;
   }

   /* Get CMV Ack Message Parameter*/
   ret = MEI_AR9_CmvAckParam16BitGet(&msg.ack_msg, &nParam16[0], 0x0, 0x2);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   ack.ibep = (nParam16[1] << 16) | nParam16[0];

   /* Set ACK Index*/
   ack.Index  = cmd.Index;
   /* Set ACK Length*/
   ack.Length = cmd.Length;
   /* Set ACK LinkNo*/
   ack.LinkNo = cmd.LinkNo;

   /* Set User Ack*/
   ret = MEI_AR9_UserAckSet(&pUserMsgs->ack_msg, (IFX_uint8_t*) &ack,
                            sizeof(ACK_G997_ATM_StatsGet_t), bInternCall);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   pUserMsgs->ack_msg.paylSize_byte = sizeof(ACK_G997_ATM_StatsGet_t);

   pUserMsgs->ictl.retCode = ret;

   return ret;
}

/**
   CMD_G997_ATM_StatsSet firmware message handler.
*/
MEI_STATIC IFX_int32_t MEI_AR9_CMD_G997_ATM_StatsSet(
                              MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                              IOCTL_MEI_messageSend_t  *pUserMsgs,
                              IFX_boolean_t             bInternCall)
{
   IFX_int32_t ret = 0;
   IFX_uint16_t nParam16[2] = {0};
   CMD_G997_ATM_StatsSet_t cmd = {0};
   ACK_G997_ATM_StatsSet_t ack = {0};
   AR9_CMV_STD_MESSAGE_T cmv_tx, cmv_rx;
   IOCTL_MEI_messageSend_t msg;

   /* Get User Cmd */
   ret = MEI_AR9_UserCmdGet(&pUserMsgs->write_msg, (IFX_uint8_t*)&cmd,
                            sizeof(CMD_G997_ATM_StatsSet_t), bInternCall);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   nParam16[0] = (IFX_uint16_t)(cmd.hecp & 0xFFFF);
   nParam16[1] = (IFX_uint16_t)((cmd.hecp >> 16) & 0xFFFF);

   /* SOC: PLAM 11 0x0 0x2*/
   MEI_AR9_CmvMsgMake(CMV_GROUP_PLAM, 11, 0x0, 0x2, IFX_FALSE, &nParam16[0], &cmv_tx, &cmv_rx, &msg);

   /* Send Socrates Cmv Message and get CMV response*/
   ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = msg.ictl.retCode;
      return ret;
   }

   nParam16[0] = (IFX_uint16_t)(cmd.cdp & 0xFFFF);
   nParam16[1] = (IFX_uint16_t)((cmd.cdp >> 16) & 0xFFFF);

   /* SOC: PLAM 16 0x0 0x2*/
   MEI_AR9_CmvMsgMake(CMV_GROUP_PLAM, 16, 0x0, 0x2, IFX_FALSE, &nParam16[0], &cmv_tx, &cmv_rx, &msg);

   /* Send Socrates Cmv Message and get CMV response*/
   ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = msg.ictl.retCode;
      return ret;
   }

   nParam16[0] = (IFX_uint16_t)(cmd.cup & 0xFFFF);
   nParam16[1] = (IFX_uint16_t)((cmd.cup >> 16) & 0xFFFF);

   /* SOC: PLAM 18 0x0 0x2*/
   MEI_AR9_CmvMsgMake(CMV_GROUP_PLAM, 18, 0x0, 0x2, IFX_FALSE, &nParam16[0], &cmv_tx, &cmv_rx, &msg);

   /* Send Socrates Cmv Message and get CMV response*/
   ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = msg.ictl.retCode;
      return ret;
   }

   nParam16[0] = (IFX_uint16_t)(cmd.ibep & 0xFFFF);
   nParam16[1] = (IFX_uint16_t)((cmd.ibep >> 16) & 0xFFFF);

   /* SOC: PLAM 19 0x0 0x2*/
   MEI_AR9_CmvMsgMake(CMV_GROUP_PLAM, 19, 0x0, 0x2, IFX_FALSE, &nParam16[0], &cmv_tx, &cmv_rx, &msg);

   /* Send Socrates Cmv Message and get CMV response*/
   ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = msg.ictl.retCode;
      return ret;
   }


   /* Set ACK Index*/
   ack.Index  = cmd.Index;
   /* Set ACK Length*/
   ack.Length = cmd.Length;

   /* Set User Ack*/
   ret = MEI_AR9_UserAckSet(&pUserMsgs->ack_msg, (IFX_uint8_t*) &ack,
                            sizeof(ACK_G997_ATM_StatsSet_t), bInternCall);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   pUserMsgs->ack_msg.paylSize_byte = sizeof(ACK_G997_ATM_StatsSet_t);

   pUserMsgs->ictl.retCode = ret;

   return ret;
}

/**
   CMD_HW_ConfigSet firmware message handler.
   Just a dummy mapping to provide the required functionality for the
   Channel counters!
   Sending this message will switch off PM Channel counters reset in the AR9 FW.
*/
MEI_STATIC IFX_int32_t MEI_AR9_CMD_HW_ConfigSet(
                              MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                              IOCTL_MEI_messageSend_t  *pUserMsgs,
                              IFX_boolean_t             bInternCall)
{
   IFX_int32_t ret = 0;
   IFX_uint16_t nParam16;
   CMD_HW_ConfigSet_t cmd = {0};
   ACK_HW_ConfigSet_t ack = {0};
   AR9_CMV_STD_MESSAGE_T cmv_tx, cmv_rx;
   IOCTL_MEI_messageSend_t msg;

   /* Get User Cmd */
   ret = MEI_AR9_UserCmdGet(&pUserMsgs->write_msg, (IFX_uint8_t*)&cmd,
                            sizeof(CMD_HW_ConfigSet_t), bInternCall);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   /* SOC: INFO 103 0x1 0x1*/
   MEI_AR9_CmvMsgMake(CMV_GROUP_INFO, 103, 0x1, 0x1, IFX_TRUE, IFX_NULL, &cmv_tx, &cmv_rx, &msg);

   /* Send Socrates Cmv Message and get CMV response*/
   ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = msg.ictl.retCode;
      return ret;
   }

   /* Get CMV Ack Message Parameter*/
   ret = MEI_AR9_CmvAckParam16BitGet(&msg.ack_msg, &nParam16, 0x0, 0x1);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   nParam16 |= 0x20;

   /* SOC: INFO 103 0x0 0x1*/
   MEI_AR9_CmvMsgMake(CMV_GROUP_INFO, 103, 0x1, 0x1, IFX_FALSE, &nParam16, &cmv_tx, &cmv_rx, &msg);

   /* Send Socrates Cmv Message and get CMV response*/
   ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = msg.ictl.retCode;
      return ret;
   }

   /* Set ACK Index*/
   ack.Index  = cmd.Index;
   /* Set ACK Length*/
   ack.Length = cmd.Length;

   /* Set User Ack*/
   ret = MEI_AR9_UserAckSet(&pUserMsgs->ack_msg, (IFX_uint8_t*) &ack,
                            sizeof(ACK_HW_ConfigSet_t), bInternCall);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   pUserMsgs->ack_msg.paylSize_byte = sizeof(ACK_HW_ConfigSet_t);

   pUserMsgs->ictl.retCode = ret;

   return ret;
}

/**
   CMD_SysVendorID_O_Get firmware message handler.
*/
MEI_STATIC IFX_int32_t MEI_AR9_CMD_SysVendorID_O_Get(
                              MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                              IOCTL_MEI_messageSend_t  *pUserMsgs,
                              IFX_boolean_t             bInternCall)
{
   IFX_int32_t ret = 0, i = 0;
   CMD_SysVendorID_O_Get_t cmd = {0};
   ACK_SysVendorID_O_Get_t ack = {0};

   IFX_uint16_t hdlcCmd[2];
   IFX_uint16_t hdlcRxBuffer[32] = {0};

   /* Get User Cmd */
   ret = MEI_AR9_UserCmdGet(&pUserMsgs->write_msg, (IFX_uint8_t*)&cmd,
                            sizeof(CMD_SysVendorID_O_Get_t), bInternCall);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   /* Fill HDLC command*/
   hdlcCmd[0] = 0x0143;
   hdlcCmd[1] = 0x0;

   ret = MEI_AR9_HDLC_MsgHandle(
                        pMeiDynCntrl,
                        (IFX_uint8_t*) hdlcCmd, 4,
                        (IFX_uint8_t*) hdlcRxBuffer, 32 * 2);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = -1;
      return ret;
   }

   /* Set ACK Index*/
   ack.Index  = cmd.Index;
   /* Set ACK Length*/
   ack.Length = cmd.Length;

   for (i = 0; i < 4; i++)
   {
      ack.sysVendorID[i] = hdlcRxBuffer[i + 1];
   }

   /* Set User Ack*/
   ret = MEI_AR9_UserAckSet(&pUserMsgs->ack_msg, (IFX_uint8_t*) &ack,
                            sizeof(ACK_SysVendorID_O_Get_t), bInternCall);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   pUserMsgs->ack_msg.paylSize_byte = sizeof(ACK_SysVendorID_O_Get_t);;

   pUserMsgs->ictl.retCode = ret;

   return ret;
}

/**
   CMD_SysVendorVersionNumO_Get firmware message handler.
*/
MEI_STATIC IFX_int32_t MEI_AR9_CMD_SysVendorVersionNumO_Get(
                              MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                              IOCTL_MEI_messageSend_t  *pUserMsgs,
                              IFX_boolean_t             bInternCall)
{
   IFX_int32_t ret = 0, i = 0;
   CMD_SysVendorVersionNumO_Get_t  cmd = {0};
   ACK_SysVendorVersionNum_O_Get_t ack = {0};

   IFX_uint16_t hdlcCmd[2];
   IFX_uint16_t hdlcRxBuffer[32] = {0};

   /* Get User Cmd */
   ret = MEI_AR9_UserCmdGet(&pUserMsgs->write_msg, (IFX_uint8_t*)&cmd,
                            sizeof(CMD_SysVendorVersionNumO_Get_t), bInternCall);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   /* Fill HDLC command*/
   hdlcCmd[0] = 0x0143;
   hdlcCmd[1] = 0x0;

   ret = MEI_AR9_HDLC_MsgHandle(
                        pMeiDynCntrl,
                        (IFX_uint8_t*) hdlcCmd, 4,
                        (IFX_uint8_t*) hdlcRxBuffer, 32 * 2);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = -1;
      return ret;
   }

   /* Set ACK Index*/
   ack.Index  = cmd.Index;
   /* Set ACK Length*/
   ack.Length = cmd.Length;

   for (i = 0; i < 8; i++)
   {
      ack.versionNum[i] = hdlcRxBuffer[i + 5];
   }

   /* Set User Ack*/
   ret = MEI_AR9_UserAckSet(&pUserMsgs->ack_msg, (IFX_uint8_t*) &ack,
                            sizeof(ACK_SysVendorVersionNum_O_Get_t), bInternCall);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   pUserMsgs->ack_msg.paylSize_byte = sizeof(ACK_SysVendorVersionNum_O_Get_t);;

   pUserMsgs->ictl.retCode = ret;

   return ret;
}

/**
   CMD_SysVendorSerialNum_O_Get firmware message handler.
*/
MEI_STATIC IFX_int32_t MEI_AR9_CMD_SysVendorSerialNum_O_Get(
                              MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                              IOCTL_MEI_messageSend_t  *pUserMsgs,
                              IFX_boolean_t             bInternCall)
{
   IFX_int32_t ret = 0, i = 0;
   CMD_SysVendorSerialNum_O_Get_t  cmd = {0};
   ACK_SysVendorSerialNum_O_Get_t ack = {0};

   IFX_uint16_t hdlcCmd[2];
   IFX_uint16_t hdlcRxBuffer[32] = {0};

   /* Get User Cmd */
   ret = MEI_AR9_UserCmdGet(&pUserMsgs->write_msg, (IFX_uint8_t*)&cmd,
                            sizeof(CMD_SysVendorSerialNum_O_Get_t), bInternCall);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   /* Fill HDLC command*/
   hdlcCmd[0] = 0x0143;
   hdlcCmd[1] = 0x0;

   ret = MEI_AR9_HDLC_MsgHandle(
                        pMeiDynCntrl,
                        (IFX_uint8_t*) hdlcCmd, 4,
                        (IFX_uint8_t*) hdlcRxBuffer, 32 * 2);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = -1;
      return ret;
   }

   /* Set ACK Index*/
   ack.Index  = cmd.Index;
   /* Set ACK Length*/
   ack.Length = cmd.Length;

   for (i = 0; i < 16; i++)
   {
      ack.serialNum[i] = hdlcRxBuffer[i + 13];
   }

   /* Set User Ack*/
   ret = MEI_AR9_UserAckSet(&pUserMsgs->ack_msg, (IFX_uint8_t*) &ack,
                            sizeof(ACK_SysVendorSerialNum_O_Get_t), bInternCall);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   pUserMsgs->ack_msg.paylSize_byte = sizeof(ACK_SysVendorSerialNum_O_Get_t);;

   pUserMsgs->ictl.retCode = ret;

   return ret;
}

/**
   CMD_HS_StandardInfoFE_SPAR1Get firmware message handler.
*/
MEI_STATIC IFX_int32_t MEI_AR9_CMD_HS_StandardInfoFE_SPAR1Get(
                              MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                              IOCTL_MEI_messageSend_t  *pUserMsgs,
                              IFX_boolean_t             bInternCall)
{
   IFX_int32_t ret = 0;
   IFX_uint16_t nParam16[3];
   CMD_HS_StandardInfoFE_SPAR1Get_t cmd = {0};
   ACK_HS_StandardInfoFE_SPAR1Get_t ack = {0};
   IFX_uint16_t *pAck = (IFX_uint16_t*)&ack;
   AR9_CMV_STD_MESSAGE_T cmv_tx, cmv_rx;
   IOCTL_MEI_messageSend_t msg;

   /* Get User Cmd */
   ret = MEI_AR9_UserCmdGet(&pUserMsgs->write_msg, (IFX_uint8_t*)&cmd,
                            sizeof(CMD_HS_StandardInfoFE_SPAR1Get_t), bInternCall);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   /* SOC: INFO 67 0x0 3*/
   MEI_AR9_CmvMsgMake(CMV_GROUP_INFO, 67, 0x0, 0x3, IFX_TRUE, IFX_NULL, &cmv_tx, &cmv_rx, &msg);

   /* Send Socrates Cmv Message and get CMV response*/
   ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = msg.ictl.retCode;
      return ret;
   }

   /* Get CMV Ack Message Parameter*/
   ret = MEI_AR9_CmvAckParam16BitGet(&msg.ack_msg, &nParam16[0], 0x0, 0x3);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   /* Set ACK Index*/
   ack.Index  = cmd.Index;
   /* Set ACK Length*/
   ack.Length = cmd.Length;

   *(pAck + 2) = nParam16[0];
   *(pAck + 3) = nParam16[1];
   *(pAck + 4) = nParam16[2];

   /* Set User Ack*/
   ret = MEI_AR9_UserAckSet(&pUserMsgs->ack_msg, (IFX_uint8_t*) &ack,
                            sizeof(ACK_HS_StandardInfoFE_SPAR1Get_t), bInternCall);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   pUserMsgs->ack_msg.paylSize_byte = sizeof(ACK_HS_StandardInfoFE_SPAR1Get_t);

   pUserMsgs->ictl.retCode = ret;

   return ret;
}

/**
   CMD_LinePerfCountNE_NoTR1Get firmware message handler.
*/
MEI_STATIC IFX_int32_t MEI_AR9_CMD_LinePerfCountNE_NoTR1Get(
                              MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                              IOCTL_MEI_messageSend_t  *pUserMsgs,
                              IFX_boolean_t             bInternCall)
{
   IFX_int32_t ret = 0;
   IFX_uint16_t nParam16;
   CMD_LinePerfCountNE_NoTR1Get_t cmd = {0};
   ACK_LinePerfCountNE_NoTR1Get_t ack = {0};
   AR9_CMV_STD_MESSAGE_T cmv_tx, cmv_rx;
   IOCTL_MEI_messageSend_t msg;

   /* Get User Cmd */
   ret = MEI_AR9_UserCmdGet(&pUserMsgs->write_msg, (IFX_uint8_t*)&cmd,
                            sizeof(CMD_LinePerfCountNE_NoTR1Get_t), bInternCall);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   /* SOC: PLAM 7 0x0 0x1*/
   MEI_AR9_CmvMsgMake(CMV_GROUP_PLAM, 7, 0x0, 0x1, IFX_TRUE, IFX_NULL, &cmv_tx, &cmv_rx, &msg);

   /* Send Socrates Cmv Message and get CMV response*/
   ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = msg.ictl.retCode;
      return ret;
   }

   /* Get CMV Ack Message Parameter*/
   ret = MEI_AR9_CmvAckParam16BitGet(&msg.ack_msg, &nParam16, 0x0, 0x1);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }
   ack.cntES_LSW = nParam16;
   ack.cntES_MSW = 0; /* no mapping found*/

   /* SOC: PLAM 8 0x0 0x1*/
   MEI_AR9_CmvMsgMake(CMV_GROUP_PLAM, 8, 0x0, 0x1, IFX_TRUE, IFX_NULL, &cmv_tx, &cmv_rx, &msg);

   /* Send Socrates Cmv Message and get CMV response*/
   ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = msg.ictl.retCode;
      return ret;
   }

   /* Get CMV Ack Message Parameter*/
   ret = MEI_AR9_CmvAckParam16BitGet(&msg.ack_msg, &nParam16, 0x0, 0x1);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   ack.cntSES_LSW = nParam16;
   ack.cntSES_MSW = 0; /* no mapping found*/

   /* SOC: PLAM 9 0x0 0x1*/
   MEI_AR9_CmvMsgMake(CMV_GROUP_PLAM, 9, 0x0, 0x1, IFX_TRUE, IFX_NULL, &cmv_tx, &cmv_rx, &msg);

   /* Send Socrates Cmv Message and get CMV response*/
   ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = msg.ictl.retCode;
      return ret;
   }

   /* Get CMV Ack Message Parameter*/
   ret = MEI_AR9_CmvAckParam16BitGet(&msg.ack_msg, &nParam16, 0x0, 0x1);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   ack.cntLOSS_LSW = nParam16;
   ack.cntLOSS_MSW = 0; /* no mapping found*/

   ack.cntFECS_LSW = 0; /* not used*/
   ack.cntFECS_MSW = 0; /* not used*/

   /* Set ACK Index*/
   ack.Index  = cmd.Index;
   /* Set ACK Length*/
   ack.Length = cmd.Length;

   /* Set User Ack*/
   ret = MEI_AR9_UserAckSet(&pUserMsgs->ack_msg, (IFX_uint8_t*) &ack,
                            sizeof(ACK_LinePerfCountNE_NoTR1Get_t), bInternCall);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   pUserMsgs->ack_msg.paylSize_byte = sizeof(ACK_LinePerfCountNE_NoTR1Get_t);

   pUserMsgs->ictl.retCode = ret;

   return ret;
}

/**
   CMD_LinePerfCountNE_NoTR1Set firmware message handler.
*/
MEI_STATIC IFX_int32_t MEI_AR9_CMD_LinePerfCountNE_NoTR1Set(
                              MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                              IOCTL_MEI_messageSend_t  *pUserMsgs,
                              IFX_boolean_t             bInternCall)
{
   IFX_int32_t ret = 0;
   IFX_uint16_t nParam16;
   CMD_LinePerfCountNE_NoTR1Set_t cmd = {0};
   ACK_LinePerfCountNE_NoTR1Set_t ack = {0};
   AR9_CMV_STD_MESSAGE_T cmv_tx, cmv_rx;
   IOCTL_MEI_messageSend_t msg;

   /* Get User Cmd */
   ret = MEI_AR9_UserCmdGet(&pUserMsgs->write_msg, (IFX_uint8_t*)&cmd,
                            sizeof(CMD_LinePerfCountNE_NoTR1Set_t), bInternCall);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   nParam16 = cmd.cntES_LSW;

   /* SOC: PLAM 7 0x0 0x1*/
   MEI_AR9_CmvMsgMake(CMV_GROUP_PLAM, 7, 0x0, 0x1, IFX_FALSE, &nParam16, &cmv_tx, &cmv_rx, &msg);

   /* Send Socrates Cmv Message and get CMV response*/
   ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = msg.ictl.retCode;
      return ret;
   }

   nParam16 = cmd.cntSES_LSW;

   /* SOC: PLAM 8 0x0 0x1*/
   MEI_AR9_CmvMsgMake(CMV_GROUP_PLAM, 8, 0x0, 0x1, IFX_FALSE, &nParam16, &cmv_tx, &cmv_rx, &msg);

   /* Send Socrates Cmv Message and get CMV response*/
   ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = msg.ictl.retCode;
      return ret;
   }

   nParam16 = cmd.cntLOSS_LSW;

   /* SOC: PLAM 9 0x0 0x1*/
   MEI_AR9_CmvMsgMake(CMV_GROUP_PLAM, 9, 0x0, 0x1, IFX_FALSE, &nParam16, &cmv_tx, &cmv_rx, &msg);

   /* Send Socrates Cmv Message and get CMV response*/
   ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = msg.ictl.retCode;
      return ret;
   }

   nParam16 = cmd.cntUAS_LSW;

   /* SOC: PLAM 10 0x0 0x1*/
   MEI_AR9_CmvMsgMake(CMV_GROUP_PLAM, 10, 0x0, 0x1, IFX_FALSE, &nParam16, &cmv_tx, &cmv_rx, &msg);

   /* Send Socrates Cmv Message and get CMV response*/
   ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = msg.ictl.retCode;
      return ret;
   }

   /* Set ACK Index*/
   ack.Index  = cmd.Index;
   /* Set ACK Length*/
   ack.Length = cmd.Length;

   /* Set User Ack*/
   ret = MEI_AR9_UserAckSet(&pUserMsgs->ack_msg, (IFX_uint8_t*) &ack,
                            sizeof(ACK_LinePerfCountNE_NoTR1Set_t), bInternCall);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   pUserMsgs->ack_msg.paylSize_byte = sizeof(ACK_LinePerfCountNE_NoTR1Set_t);

   pUserMsgs->ictl.retCode = ret;

   return ret;
}

/**
   CMD_LinePerfCountFE_NoTR1Get firmware message handler.
*/
MEI_STATIC IFX_int32_t MEI_AR9_CMD_LinePerfCountFE_NoTR1Get(
                              MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                              IOCTL_MEI_messageSend_t  *pUserMsgs,
                              IFX_boolean_t             bInternCall)
{
   IFX_int32_t ret = 0;
   IFX_uint16_t nParam16;
   CMD_LinePerfCountFE_NoTR1Get_t cmd = {0};
   ACK_LinePerfCountFE_NoTR1Get_t ack = {0};
   AR9_CMV_STD_MESSAGE_T cmv_tx, cmv_rx;
   IOCTL_MEI_messageSend_t msg;

   IFX_boolean_t bAdsl1 = IFX_FALSE, bAdsl2p = IFX_FALSE;
   IFX_uint16_t hdlcCmd[2] = {0};
   IFX_uint16_t hdlcRxBuffer[29] = {0};

   /* Get User Cmd */
   ret = MEI_AR9_UserCmdGet(&pUserMsgs->write_msg, (IFX_uint8_t*)&cmd,
                            sizeof(CMD_LinePerfCountFE_NoTR1Get_t), bInternCall);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   /* Check for the ADSL1 mode*/
   ret = MEI_AR9_AdslModeStatusGet(pMeiDynCntrl, &bAdsl1, &bAdsl2p);
   if (ret < 0)
   {
      return -1;
   }

   if (bAdsl1)
   {
      /* SOC: PLAM 33 0x0 0x1*/
      MEI_AR9_CmvMsgMake(CMV_GROUP_PLAM, 33, 0x0, 0x1, IFX_TRUE, IFX_NULL, &cmv_tx, &cmv_rx, &msg);

      /* Send Socrates Cmv Message and get CMV response*/
      ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
      if (ret < 0)
      {
         pUserMsgs->ictl.retCode = msg.ictl.retCode;
         return ret;
      }

      /* Get CMV Ack Message Parameter*/
      ret = MEI_AR9_CmvAckParam16BitGet(&msg.ack_msg, &nParam16, 0x0, 0x1);
      if (ret < 0)
      {
         pUserMsgs->ictl.retCode = ret;
         return ret;
      }

      ack.cntES_LSW  = nParam16;
      ack.cntES_MSW = 0;

      /* SOC: PLAM 34 0x0 0x1*/
      MEI_AR9_CmvMsgMake(CMV_GROUP_PLAM, 34, 0x0, 0x1, IFX_TRUE, IFX_NULL, &cmv_tx, &cmv_rx, &msg);

      /* Send Socrates Cmv Message and get CMV response*/
      ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
      if (ret < 0)
      {
         pUserMsgs->ictl.retCode = msg.ictl.retCode;
         return ret;
      }

      /* Get CMV Ack Message Parameter*/
      ret = MEI_AR9_CmvAckParam16BitGet(&msg.ack_msg, &nParam16, 0x0, 0x1);
      if (ret < 0)
      {
         pUserMsgs->ictl.retCode = ret;
         return ret;
      }

      ack.cntSES_LSW  = nParam16;
      ack.cntSES_MSW = 0;

      /* SOC: PLAM 35 0x0 0x1*/
      MEI_AR9_CmvMsgMake(CMV_GROUP_PLAM, 35, 0x0, 0x1, IFX_TRUE, IFX_NULL, &cmv_tx, &cmv_rx, &msg);

      /* Send Socrates Cmv Message and get CMV response*/
      ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
      if (ret < 0)
      {
         pUserMsgs->ictl.retCode = msg.ictl.retCode;
         return ret;
      }

      /* Get CMV Ack Message Parameter*/
      ret = MEI_AR9_CmvAckParam16BitGet(&msg.ack_msg, &nParam16, 0x0, 0x1);
      if (ret < 0)
      {
         pUserMsgs->ictl.retCode = ret;
         return ret;
      }

      ack.cntLOSS_LSW  = nParam16;
      ack.cntLOSS_MSW = 0;

      ack.cntFECS_LSW = 0; /* not used*/
      ack.cntFECS_MSW = 0; /* not used*/
   }
   else
   {
      hdlcCmd[0] = 0x105;

      ret = MEI_AR9_HDLC_MsgHandle(
                           pMeiDynCntrl,
                           (IFX_uint8_t*) hdlcCmd, 2,
                           (IFX_uint8_t*) hdlcRxBuffer, sizeof(hdlcRxBuffer));
      if (ret < 0)
      {
         pUserMsgs->ictl.retCode = -1;
         return ret;
      }

      ack.cntES_LSW = SWAP16_BYTE_ORDER(hdlcRxBuffer[8]);
      ack.cntES_MSW = SWAP16_BYTE_ORDER(hdlcRxBuffer[7]);

      ack.cntSES_LSW = SWAP16_BYTE_ORDER(hdlcRxBuffer[10]);
      ack.cntSES_MSW = SWAP16_BYTE_ORDER(hdlcRxBuffer[9]);

      ack.cntLOSS_LSW = SWAP16_BYTE_ORDER(hdlcRxBuffer[12]);
      ack.cntLOSS_MSW = SWAP16_BYTE_ORDER(hdlcRxBuffer[11]);

      ack.cntFECS_LSW = 0; /* not used*/
      ack.cntFECS_MSW = 0; /* not used*/
   }

   /* Set ACK Index*/
   ack.Index  = cmd.Index;
   /* Set ACK Length*/
   ack.Length = cmd.Length;

   /* Set User Ack*/
   ret = MEI_AR9_UserAckSet(&pUserMsgs->ack_msg, (IFX_uint8_t*) &ack,
                            sizeof(ACK_LinePerfCountFE_NoTR1Get_t), bInternCall);
   if (ret < 0)
   {
      pUserMsgs->ictl.retCode = ret;
      return ret;
   }

   pUserMsgs->ack_msg.paylSize_byte = sizeof(ACK_LinePerfCountFE_NoTR1Get_t);

   pUserMsgs->ictl.retCode = ret;

   return ret;
}

/*
   HDLC block
*/

MEI_STATIC IFX_int32_t MEI_AR9_AdslModeStatusGet(
                        MEI_DYN_CNTRL_T *pMeiDynCntrl,
                        IFX_boolean_t   *pAdsl1,
                        IFX_boolean_t   *pAdsl2p)
{
   IFX_int32_t ret = 0;
   AR9_CMV_STD_MESSAGE_T cmv_tx, cmv_rx;
   IOCTL_MEI_messageSend_t msg;
   IFX_uint16_t nAdslMode = 0, nAdslMode1 = 0;

   *pAdsl1  = IFX_FALSE;
   *pAdsl2p = IFX_FALSE;

   /* SOC: STAT 0x1 0x0 0x1*/
   MEI_AR9_CmvMsgMake(CMV_GROUP_STAT, 1, 0x0, 0x1, IFX_TRUE, IFX_NULL, &cmv_tx, &cmv_rx, &msg);

   /* Send Socrates Cmv Message and get CMV response*/
   ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
   if (ret < 0)
   {
      return ret;
   }

   /* Get CMV Ack Message Parameter*/
   ret = MEI_AR9_CmvAckParam16BitGet(&msg.ack_msg, &nAdslMode, 0x0, 0x1);
   if (ret < 0)
   {
      return ret;
   }

   if (nAdslMode == 0)
   {
      /* SOC: STAT 0x1 0x0 0x1*/
      MEI_AR9_CmvMsgMake(CMV_GROUP_STAT, 17, 0x0, 0x1, IFX_TRUE, IFX_NULL, &cmv_tx, &cmv_rx, &msg);

      /* Send Socrates Cmv Message and get CMV response*/
      ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
      if (ret < 0)
      {
         return ret;
      }

      /* Get CMV Ack Message Parameter*/
      ret = MEI_AR9_CmvAckParam16BitGet(&msg.ack_msg, &nAdslMode1, 0x0, 0x1);
      if (ret < 0)
      {
         return ret;
      }
   }

   if (!nAdslMode && !nAdslMode1)
   {
      return -1;
   }


   /* Check for the ADSL1 mode*/
   if (!((nAdslMode >= 0x100) || (nAdslMode1 != 0 && nAdslMode1 < 8)))
   {
      /* Set ADSL1 mode indication*/
      *pAdsl1  = IFX_TRUE;
   }

   /* Check for the ADSL2+ mode*/
   if ((nAdslMode >= 0x4000) || (nAdslMode1 != 0 && nAdslMode1 < 8))
   {
      /* Set ADSL2+ mode indication*/
      *pAdsl2p = IFX_TRUE;
   }

   return ret;
}

MEI_STATIC IFX_int32_t MEI_AR9_HDLC_StatusGet(
                        MEI_DYN_CNTRL_T *pMeiDynCntrl,
                        IFX_uint16_t *pnStatus)
{
   IFX_int32_t ret = 0;
   AR9_CMV_STD_MESSAGE_T cmv_tx, cmv_rx;
   IOCTL_MEI_messageSend_t msg;

   /* SOC: STAT 0x1 0x0 0x1*/
   MEI_AR9_CmvMsgMake(CMV_GROUP_STAT, 14, 0x0, 0x1, IFX_TRUE, IFX_NULL, &cmv_tx, &cmv_rx, &msg);

   /* Send Socrates Cmv Message and get CMV response*/
   ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
   if (ret < 0)
   {
      return ret;
   }

   /* Get CMV Ack Message Parameter*/
   ret = MEI_AR9_CmvAckParam16BitGet(&msg.ack_msg, pnStatus, 0x0, 0x1);
   if (ret < 0)
   {
      return ret;
   }

   return ret;
}

MEI_STATIC IFX_int32_t MEI_AR9_HDLC_ResolvedGet(
   MEI_DYN_CNTRL_T *pMeiDynCntrl,
   IFX_uint16_t nStatus)
{
   if (nStatus == CMV_ME_HDLC_MSG_QUEUED || nStatus == CMV_ME_HDLC_MSG_SENT ||
       nStatus == CMV_ME_HDLC_MSG_NOT_SUPPORTED)
   {
      return CMV_ME_HDLC_UNRESOLVED;
   }

   if (nStatus == CMV_ME_HDLC_IDLE)
   {
      return CMV_ME_HDLC_RESOLVED;
   }

   return CMV_ME_HDLC_RESOLVED;
}

MEI_STATIC IFX_int32_t MEI_AR9_HDLC_Send(
   MEI_DYN_CNTRL_T *pMeiDynCntrl,
   IFX_uint8_t *pHdlcPkt,
   IFX_int32_t nPktLen,
   IFX_int32_t nMaxLength)
{
   IFX_int32_t ret = 0;
   AR9_CMV_STD_MESSAGE_T cmv_tx, cmv_rx;
   IOCTL_MEI_messageSend_t msg;

   IFX_uint16_t nData = 0;
   IFX_uint16_t nLen = 0;
   IFX_uint16_t nRxLength = 0;
   IFX_int32_t nWriteSize = 0;

   if (nPktLen > nMaxLength)
   {
      /* SOC: INFO 85 2 0x1*/
      MEI_AR9_CmvMsgMake(CMV_GROUP_INFO, 85, 2, 0x1, IFX_TRUE, IFX_NULL, &cmv_tx, &cmv_rx, &msg);

      /* Send Socrates Cmv Message and get CMV response*/
      ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
      if (ret < 0)
      {
         return ret;
      }

      /* Get CMV Ack Message Parameter*/
      ret = MEI_AR9_CmvAckParam16BitGet(&msg.ack_msg, &nRxLength, 0x0, 0x1);
      if (ret < 0)
      {
         return ret;
      }

      if (nRxLength + nMaxLength < nPktLen)
      {
         return -1;
      }
   }

   while (nLen < nPktLen)
   {
      nWriteSize = nPktLen - nLen;
      if (nWriteSize > 24)
      {
         nWriteSize = 24;
      }

      MEI_AR9_CmvMsgMake(CMV_GROUP_INFO, 81, nLen/2, (nWriteSize + 1) / 2,
         IFX_FALSE, (IFX_uint16_t*)(pHdlcPkt + nLen), &cmv_tx, &cmv_rx, &msg);

      /* Send Socrates Cmv Message and get CMV response*/
      ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
      if (ret < 0)
      {
         return ret;
      }

      nLen += ((IFX_uint16_t)nWriteSize);
   }

   /* Update tx message length */
   MEI_AR9_CmvMsgMake(CMV_GROUP_INFO, 83, 0x2, 0x1, IFX_FALSE, &nLen, &cmv_tx, &cmv_rx, &msg);

   /* Send Socrates Cmv Message and get CMV response*/
   ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
   if (ret < 0)
   {
      return ret;
   }

   /* Start to send */
   nData = (1 << 0);
   MEI_AR9_CmvMsgMake(CMV_GROUP_CNTL, 2, 0x0, 0x1, IFX_FALSE, &nData, &cmv_tx, &cmv_rx, &msg);

   /* Send Socrates Cmv Message and get CMV response*/
   ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
   if (ret < 0)
   {
      return ret;
   }

   return ret;
}

MEI_STATIC IFX_int32_t MEI_AR9_HDLC_Write (
   MEI_DYN_CNTRL_T *pMeiDynCntrl,
   IFX_uint8_t *pHdlcPkt,
   IFX_int32_t nHdlcPktLen)
{
   IFX_int32_t ret = 0;
   AR9_CMV_STD_MESSAGE_T cmv_tx, cmv_rx;
   IOCTL_MEI_messageSend_t msg;
   IFX_boolean_t bAdsl1 = IFX_FALSE, bAdsl2p = IFX_FALSE;
   IFX_uint16_t nHdlcStatus = 0;
   IFX_uint16_t nMaxHdlcTxLength = 0;
   IFX_uint16_t nRetry = 0;
   IFX_int32_t nSendBusyCounter = 0;
   IFX_boolean_t bSendRetry = IFX_FALSE;
   IFX_uint16_t nParam16 = 0;

   /* SOC: STAT 0x0 0x0 0x1*/
   MEI_AR9_CmvMsgMake(CMV_GROUP_STAT, 0x0, 0x0, 0x1, IFX_TRUE, IFX_NULL, &cmv_tx, &cmv_rx, &msg);

   /* Send Socrates Cmv Message and get CMV response*/
   ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
   if (ret < 0)
   {
      return ret;
   }

   /* Get CMV Ack Message Parameter*/
   ret = MEI_AR9_CmvAckParam16BitGet(&msg.ack_msg, &nParam16, 0x0, 0x1);
   if (ret < 0)
   {
      return ret;
   }

   /* Check for the SHOWTIME state*/
   if (nParam16 != CMV_STAT_MODEM_STATUS_SHOWTIME)
   {
      return -1;
   }

   /* Check for the ADSL1 mode*/
   ret = MEI_AR9_AdslModeStatusGet(pMeiDynCntrl, &bAdsl1, &bAdsl2p);
   if ((ret < 0) || bAdsl1)
   {
      return -1;
   }

   for (;;)
   {
      /* retry 1000 times (1 sec) */
      while (nRetry < 1000)
      {
         /* Get HDLC status*/
         ret = MEI_AR9_HDLC_StatusGet(pMeiDynCntrl, &nHdlcStatus);
         if (ret < 0)
         {
            break;
         }

         /* arc ready to send HDLC message */
         if (MEI_AR9_HDLC_ResolvedGet(pMeiDynCntrl, nHdlcStatus) == CMV_ME_HDLC_RESOLVED)
         {
            /* SOC: INFO 83 0x0 0x1*/
            MEI_AR9_CmvMsgMake(CMV_GROUP_INFO, 83, 0x0, 0x1, IFX_TRUE, IFX_NULL, &cmv_tx, &cmv_rx, &msg);

            /* Send Socrates Cmv Message and get CMV response*/
            ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
            if (ret < 0)
            {
               return ret;
            }

            /* Get CMV Ack Message Parameter*/
            ret = MEI_AR9_CmvAckParam16BitGet(&msg.ack_msg, &nMaxHdlcTxLength, 0x0, 0x1);
            if (ret < 0)
            {
               return ret;
            }

            /* Send HDLC Packet*/
            return MEI_AR9_HDLC_Send (pMeiDynCntrl, pHdlcPkt, nHdlcPktLen, nMaxHdlcTxLength);
         }
         else
         {
            if ((nHdlcStatus == CMV_ME_HDLC_MSG_SENT) ||
                (nHdlcStatus == CMV_ME_HDLC_MSG_QUEUED))
            {
               nSendBusyCounter++;
            }
            else if (nHdlcStatus == CMV_ME_HDLC_MSG_NOT_SUPPORTED)
            {
               ret = -1;
               break;
            }
            else
            {
               ret = -1;
               break;
            }
         }
         nRetry++;
         MEI_DRVOS_Wait_ms(1);
      }

      if (ret < 0)
      {
         break;
      }

      /* wait 10 seconds and FW still report busy -> reset FW HDLC status */
      if (nSendBusyCounter > 950 && bSendRetry == IFX_FALSE)
      {
         bSendRetry = IFX_TRUE;
         nRetry = 0;

         nSendBusyCounter = 0;
         nParam16 = 0x2;

         /* SOC: CNTL 0x2 0x0 0x1*/
         MEI_AR9_CmvMsgMake(CMV_GROUP_CNTL, 0x2, 0x0, 0x1, IFX_FALSE, &nParam16, &cmv_tx, &cmv_rx, &msg);

         /* Send Socrates Cmv Message and get CMV response*/
         ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
         if (ret < 0)
         {
            break;
         }
         continue;
      }
      else
      {
         ret = -1;
         break;
      }
   }

   return ret;
}

MEI_STATIC IFX_int32_t MEI_AR9_HDLC_Read (
   MEI_DYN_CNTRL_T *pMeiDynCntrl,
   IFX_uint8_t *pHdlcPkt,
   IFX_int32_t nMaxHdlcPktLen,
   IFX_int32_t *pnRead)
{
   IFX_int32_t ret = 0;
   AR9_CMV_STD_MESSAGE_T cmv_tx, cmv_rx;
   IOCTL_MEI_messageSend_t msg;

   IFX_boolean_t bAdsl1 = IFX_FALSE, bAdsl2p = IFX_FALSE;
   IFX_int32_t nMsgReadLen, nRetry = 0, nCurrentSize = 0;
   IFX_uint16_t nHdlcStatus = 0, nPktLen = 0;
   IFX_uint16_t buf[16], nParam16 = 0;

   if (!pHdlcPkt || !pnRead)
   {
      return -1;
   }

   *pnRead = 0;

   /* SOC: STAT 0x0 0x0 0x1*/
   MEI_AR9_CmvMsgMake(CMV_GROUP_STAT, 0x0, 0x0, 0x1, IFX_TRUE, IFX_NULL, &cmv_tx, &cmv_rx, &msg);

   /* Send Socrates Cmv Message and get CMV response*/
   ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
   if (ret < 0)
   {
      return ret;
   }

   /* Get CMV Ack Message Parameter*/
   ret = MEI_AR9_CmvAckParam16BitGet(&msg.ack_msg, &nParam16, 0x0, 0x1);
   if (ret < 0)
   {
      return ret;
   }

   /* Check for the SHOWTIME state*/
   if (nParam16 != CMV_STAT_MODEM_STATUS_SHOWTIME)
   {
      return -1;
   }

   /* Check for the ADSL1 mode*/
   ret = MEI_AR9_AdslModeStatusGet(pMeiDynCntrl, &bAdsl1, &bAdsl2p);
   if ((ret < 0) || bAdsl1)
   {
      return -1;
   }

   /* Wait max 4 sec*/
   while (nRetry < 40)
   {
      ret = MEI_AR9_HDLC_StatusGet(pMeiDynCntrl, &nHdlcStatus);
      if (ret < 0)
      {
         break;
      }

      if (nHdlcStatus == CMV_ME_HDLC_RESP_RCVD)
      {
         /* SOC: INFO 83 3 1*/
         MEI_AR9_CmvMsgMake(CMV_GROUP_INFO, 83, 3, 0x1, IFX_TRUE, IFX_NULL, &cmv_tx, &cmv_rx, &msg);

         /* Send Socrates Cmv Message and get CMV response*/
         ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
         if (ret < 0)
         {
            return ret;
         }

         /* Get CMV Ack Message Parameter*/
         ret = MEI_AR9_CmvAckParam16BitGet(&msg.ack_msg, &nPktLen, 0x0, 0x1);
         if (ret < 0)
         {
            return ret;
         }

         if (nPktLen > nMaxHdlcPktLen)
         {
            return -1;
         }
         else
         {
            while (nCurrentSize < nPktLen)
            {
               if (nPktLen - nCurrentSize > (MEI_MAX_CMV_MSG_LENGTH * 2 - 8))
               {
                  nMsgReadLen = MEI_MAX_CMV_MSG_LENGTH * 2 - 8;
               }
               else
               {
                  nMsgReadLen = nPktLen - nCurrentSize;
               }

               MEI_AR9_CmvMsgMake(CMV_GROUP_INFO, 82,
                  (IFX_uint16_t)(0 + (nCurrentSize / 2)),
                  (nMsgReadLen + 1) / 2, IFX_TRUE, IFX_NULL, &cmv_tx, &cmv_rx, &msg);

               /* Send Socrates Cmv Message and get CMV response*/
               ret = MEI_AR9_CmvMsgSend(pMeiDynCntrl, &msg);
               if (ret < 0)
               {
                  return ret;
               }

               /* Get CMV Ack Message Parameter*/
               ret = MEI_AR9_CmvAckParam16BitGet(&msg.ack_msg, buf, 0x0, (nMsgReadLen + 1) / 2);
               if (ret < 0)
               {
                  return ret;
               }

               memcpy (pHdlcPkt + nCurrentSize, buf, nMsgReadLen);
               nCurrentSize += nMsgReadLen;
            }

            *pnRead = nCurrentSize;
         }
         break;
      }
      else if (nHdlcStatus == CMV_ME_HDLC_RESP_TIMEOUT)
      {
         return -1;
      }
      else
      {
         ret = -1;
      }

      nRetry++;

      MEI_DRVOS_Wait_ms(100);
   }

   return ret;
}

MEI_STATIC IFX_int32_t MEI_AR9_HDLC_MsgHandle(
                        MEI_DYN_CNTRL_T *pMeiDynCntrl,
                        IFX_uint8_t *pTxHdlcPkt,
                        IFX_int32_t nTxHdlcPktLen,
                        IFX_uint8_t *pRxHdlcPkt,
                        IFX_int32_t nRxHdlcPktLen )
{
   IFX_int32_t ret = 0;
   IFX_int32_t nHdlcRxLen = 0;

   if (!pTxHdlcPkt || !pRxHdlcPkt || !nTxHdlcPktLen || !nRxHdlcPktLen)
   {
      return -1;
   }

   ret = MEI_AR9_HDLC_Write (pMeiDynCntrl, pTxHdlcPkt, nTxHdlcPktLen);
   if (ret < 0)
   {
      return ret;
   }

   MEI_DRVOS_Wait_ms(1);

   ret = MEI_AR9_HDLC_Read (pMeiDynCntrl, pRxHdlcPkt, nRxHdlcPktLen, &nHdlcRxLen);

   if (ret < 0 || nHdlcRxLen <=0)
   {
      return -1;
   }

   return ret;
}

/**
   Map and Write message to the AR9 device and wait for the corresponding ACK.

\param
   pMeiDynCntrl - private dynamic device data (per open instance)
\param
   pUserMsgs      - points to the user message information data
\param
   bInternCall    - indicates if the call is form the internal interface
                    (image and data already in kernel space)

\return
   0 if success
   negative value in case of error.

*/
IFX_int32_t MEI_AR9_MapMsgSend(
                              MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                              IOCTL_MEI_messageSend_t  *pUserMsgs,
                              IFX_boolean_t             bInternCall)
{
   /* Set default ret code, ignore unknown MsgId*/
   IFX_int32_t ret = 0;
   MEI_AR9_CmdEntry_t *pCmdTable = g_CmdTable;
   IFX_boolean_t bFound = IFX_FALSE;

   /* Find CMD handler according to the specified MsgId*/
   while(pCmdTable->pFunc != IFX_NULL)
   {
      if (pCmdTable->msgId != pUserMsgs->write_msg.msgId)
      {
         pCmdTable++;
         continue;
      }

      /* Call CMD handler*/
      ret = pCmdTable->pFunc(pMeiDynCntrl, pUserMsgs, bInternCall);

      bFound = IFX_TRUE;

      break;
   }

   /* Handling of not mapped CMDs*/
   if (!bFound)
   {
      ret = MEI_AR9_CMD_Unhandled(pMeiDynCntrl, pUserMsgs, bInternCall);
   }

   /* TODO: Crosscheck if it is really necessary to handle these values*/
   pUserMsgs->ack_msg.msgClassifier = pUserMsgs->write_msg.msgClassifier;
   pUserMsgs->ack_msg.msgCtrl       = pUserMsgs->write_msg.msgCtrl;
   pUserMsgs->ack_msg.msgId         = pUserMsgs->write_msg.msgId;

   return ret;
}


#endif /* #if (MEI_SUPPORT_DEVICE_AR9 == 1)*/

