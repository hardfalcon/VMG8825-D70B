/******************************************************************************

                          Copyright (c) 2007-2015
                     Lantiq Beteiligungs-GmbH & Co. KG

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/* ==========================================================================
   Description : VRX device specific insert / extract functions.
   ========================================================================== */

/* ============================================================================
   Includes
   ========================================================================= */

/* get at first the driver configuration */
#include "drv_mei_cpe_config.h"

#include "ifx_types.h"
#include "drv_mei_cpe_os.h"

#include "drv_mei_cpe_api.h"
#include "drv_mei_cpe_device_cntrl.h"

#if (MEI_SUPPORT_VDSL2_ADSL_SWAP == 1)

#include "drv_mei_cpe_dbg.h"
#include "drv_mei_cpe_dbg_driver.h"

#include "drv_mei_cpe_interface.h"

#include "cmv_message_format.h"

#endif      /* #if (MEI_SUPPORT_VDSL2_ADSL_SWAP == 1) */

#include "drv_mei_cpe_msg_process.h"

#if (MEI_SUPPORT_VDSL2_ADSL_SWAP == 1)
#include "drv_mei_cpe_mei_interface.h"
#endif


#if (MEI_DRV_ATM_OAM_ENABLE == 1)
#include "drv_mei_cpe_atmoam.h"
#include "drv_if_vdsl2_atm_oam.h"
#endif

#if (MEI_DRV_CLEAR_EOC_ENABLE == 1)
#include "drv_mei_cpe_clear_eoc.h"
#include "drv_if_vdsl2_clear_eoc.h"
#endif

/* ============================================================================
   Global Macro Definitions
   ========================================================================= */

#ifdef MEI_STATIC
#undef MEI_STATIC
#endif

#ifdef MEI_DEBUG
#define MEI_STATIC
#else
#define MEI_STATIC   static
#endif


/* ============================================================================
   Common used functions for message handling
   ========================================================================= */

/**
   This function checks the availablility of the given message for the
   current driver and modem state.
*/
IFX_int32_t MEI_DevCntlMsgAvailable(
                              MEI_DYN_CNTRL_T *pMeiDynCntrl,
                              IFX_uint32_t      drvMsgId)
{
   if ( MEI_DRV_MSG_AVAILABLE( pMeiDynCntrl->pMeiDev,
                                 drvMsgId) != IFX_TRUE )
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_WRN,
            ("MEI_DRV[%02d - %02d]: Warning - cannot Write 0x%04X, invalid modem state (%d!=%d)" MEI_DRV_CRLF
             , MEI_DRV_DYN_LINENUM_GET(pMeiDynCntrl), pMeiDynCntrl->openInstance
             , MEI_DRV_MODEM_MSG_ID_GET(drvMsgId)
             , MEI_DRV_MODEM_STATE_GET(pMeiDynCntrl->pMeiDev)
             , MEI_DRV_MODEM_AVAIL_STATE_GET(drvMsgId) ));

      return IFX_ERROR;
   }

   return IFX_SUCCESS;
}


#if (MEI_DRV_POWERSAVING_ENABLED == 1)
/* ============================================================================
   VRX Device Init - setup for power saving
   ========================================================================= */


/**
   Init the VRX device - for power saving.
   Therefore the function will be called on interrupt level while the
   processing of the GP1 interrupt or the "EventInitDone ROM msg.

\remarks
   The GP1 interrupt is signaled if the ROM code is entered. This can occurr
   while startup (power up) or reset. After basic ROM code initialization the
   EventInitDone ROM message will be send.


\param
   pMeiDev   private device data [I]

\return
   0 (IFX_SUCCESS) if success
   negative value in case of error

*/
IFX_int32_t MEI_DevSetup_PowerSave(
                              MEI_DEV_T *pMeiDev)
{
   IFX_uint32_t regVal = 0, regValWr = 0, regVal1 = 0;

   /*============================================================
      update/patch OnChip Memory Control register - pull up
      Off and On Chip:
         - Mem ChipSelect
         - Bus Reset
         - Write / Output Enable
         - Address/Data Bus
   */

   MEI_ReadDma32Bit( &pMeiDev->meiDrvCntrl,
                     MEI_DEV_REG_ADDR_OC_MEM_CTRL,
                     &regVal, 1);

   regVal = ((regVal & 0x0000FFFF) | 0x7F7F0000);

   MEI_WriteDma32Bit( &pMeiDev->meiDrvCntrl,
                      MEI_DEV_REG_ADDR_OC_MEM_CTRL,
                      &regVal, 1, 0);

   MEI_ReadDma32Bit( &pMeiDev->meiDrvCntrl,
                     MEI_DEV_REG_ADDR_OC_MEM_CTRL,
                     &regVal1, 1);

   PRN_DBG_INT( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL, MEI_DRV_LINENUM_GET(pMeiDev),
        ("MEI_DRV[%02d]: PowerS - OC_MEM_CTRL 0x%08X = 0x%08X --> 0x%08X" MEI_DRV_CRLF,
           MEI_DRV_LINENUM_GET(pMeiDev)
         , MEI_DEV_REG_ADDR_OC_MEM_CTRL
         , regVal
         , regVal1 ));

   /*============================================================*/


   /*============================================================
      update/patch Alternate Function Select Register - pull up
      Disbale Clock functionality
         - AFCLKO1, AFCLKO2
   */

   MEI_ReadDma32Bit( &pMeiDev->meiDrvCntrl,
                     MEI_DEV_REG_ADDR_AFSEL, &regVal, 1);

   if (MEI_DFE_INSTANCE_PER_ENTITY == 1)
   {
      /* single device - switch clock off */
      regValWr = (regVal & ~0x00000018);
   }
   else
   {
      /* multi device - check instance (first/second) */
      if (pMeiDev->modemData.chipId & 0x01)
      {
         /* second instance in the package - switch clock off on both*/
         regValWr = (regVal & ~0x00000018);
      }
      else
      {
         /* first instance in the package - switch clock on  on VRX 0
                                          - switch clock off on VRX 1 */
         regValWr = (regVal & ~0x00000018) | 0x00000008;
      }
   }

   MEI_WriteDma32Bit( &pMeiDev->meiDrvCntrl,
                      MEI_DEV_REG_ADDR_AFSEL,
                      &regValWr, 1, 0);

   MEI_ReadDma32Bit( &pMeiDev->meiDrvCntrl,
                     MEI_DEV_REG_ADDR_AFSEL,
                     &regVal1, 1);

   PRN_DBG_INT( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL, MEI_DRV_LINENUM_GET(pMeiDev),
        ("MEI_DRV[%02d]: PowerS - AFSEL 0x%08X = 0x%08X --> 0x%08X" MEI_DRV_CRLF,
           MEI_DRV_LINENUM_GET(pMeiDev)
         , MEI_DEV_REG_ADDR_AFSEL
         , regVal
         , regVal1));
   /*============================================================*/

   return IFX_SUCCESS;
}
#endif      /* #if (MEI_DRV_POWERSAVING_ENABLED == 1) */



#if (MEI_SUPPORT_DFE_GPA_ACCESS == 1)
/* ==========================================================================
   VRX Device - Debug messages (General Purpose Access GPA).
   ========================================================================== */

/**
   Write to the target via General Purpose Access.

   Online code:
   - CMD_DBG_MemMapWrite      --> ACK_DBG_MemMapWirte
   - CMD_DBG_AuxRegisterWrite --> ACK_DBG_AuxRegisterWrite

\param
   pMeiDev:    Points to the VRX device struct.
\param
   drvState:   Current state (Online or ROM Handler alive)
\param
   aux:     Destination area on the target: MEM/AUX
\param
   addr:    Destination address on the target (mem addr / aux register offset)
\param
   val:     Value to set.

\return
   IFX_SUCCESS
   IFX_ERROR
*/
IFX_int32_t MEI_OnlineGpaWr(
                        MEI_DYN_CNTRL_T *pMeiDynCntrl,
                        IFX_boolean_t aux,
                        IFX_uint32_t addr, IFX_uint32_t val)
{
   IFX_uint32_t      msg_size;
   unsigned short    msgId;
   MEI_DEV_T   *pMeiDev = pMeiDynCntrl->pMeiDev;
   CMV_STD_MESSAGE_T gpaMsg;


   PRN_DBG_USR( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL, MEI_DRV_LINENUM_GET(pMeiDev),
          ("MEI_DRV[%02d]: GPA Msg (OnL) Write, %s = 0x%08X, val = 0x%08X" MEI_DRV_CRLF,
           MEI_DRV_LINENUM_GET(pMeiDev), ((aux) ? "aux" : "mem"), addr, val ));

   /*
      setup the GPA message
   */
   memset(&gpaMsg, 0x00, sizeof(gpaMsg));
   gpaMsg.header.mbxCode =  MEI_MBOX_CODE_MSG_WRITE;

   /* msgId2cmv: setup CMV header fields */
   msgId = (aux) ? CMD_DBG_AUX_REGISTER_WRITE : CMD_DBG_MEM_MAP_WRITE;
   MEI_MsgId2CmvHeader( pMeiDev, &gpaMsg, msgId);

   /*
      set payload fields - always 4 (16 bit units)
   */
   CMV_MSGHDR_PAYLOAD_SIZE_SET(gpaMsg, 4);

   gpaMsg.payload.params_32Bit[0] = addr;
   gpaMsg.payload.params_32Bit[1] = val;

   /* mailbox message size: header + payload */
   msg_size = CMV_HEADER_8BIT_SIZE + sizeof(unsigned int) * 2;

   /* keep msgId for check ACK */
   pMeiDev->gpaBuf.MessageID = (IFX_uint32_t)msgId;

   if ( (MEI_WriteMailbox( pMeiDynCntrl, NULL,
                             (MEI_MEI_MAILBOX_T *)&gpaMsg,
                             msg_size )) == 0 )
   {
      /* error while send */
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DRV[%02d]: ERROR: send GPA msg (OnL) <%s>" MEI_DRV_CRLF,
              MEI_DRV_LINENUM_GET(pMeiDev),
              (aux) ? "CMD_DBG_AuxRegisterWrite" : "CMD_DBG_MemMapWrite"));

      pMeiDev->gpaBuf.MessageID = 0;
      return IFX_ERROR;
   }

   /* setup timeout counter for ACK */
   MEI_SET_TIMEOUT_CNT( pMeiDev,
                  MEI_MaxWaitDfeResponce_ms / MEI_MIN_MAILBOX_POLL_TIME_MS);

   /* poll for response */
   while(1)
   {
      /* wait: for an response from the VRX boot loader */
      MEI_PollIntPerVrxLine(pMeiDev, e_MEI_DEV_ACCESS_MODE_PASSIV_POLL);

      if (MEI_DRV_MAILBOX_STATE_GET(pMeiDev) != e_MEI_MB_FREE)
      {
         if ( MEI_WaitForMailbox(pMeiDev) != IFX_SUCCESS)
         {
            /* TIMEOUT - clear the GPA message ID */
            pMeiDev->gpaBuf.MessageID = 0;
            break;
         }
      }
      else
         break;
   }

   /* check result */
   if ( (IFX_uint16_t)pMeiDev->gpaBuf.MessageID != msgId )
   {
      /* error */
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
             ("MEI_DRV[%02d]: Write GPA (OnL), timeout for msgId 0x%04X (recv 0x%04X)" MEI_DRV_CRLF,
              MEI_DRV_LINENUM_GET(pMeiDev), msgId, pMeiDev->gpaBuf.MessageID));

      pMeiDev->gpaBuf.MessageID = 0;
      return IFX_ERROR;
   }

   pMeiDev->gpaBuf.MessageID = 0;
   return IFX_SUCCESS;
}


/**
   Read from the target via General Purpose Access.

   Online code:
   - CMD_DBG_MemMapRead       --> ACK_DBG_MemMapRead
   - CMD_DBG_AuxRegisterRead  --> ACK_DBG_AuxRegisterRead


\param
   pMeiDev: Points to the VRX device struct.
\param
   aux:     Destination area on the target: MEM/AUX
\param
   addr:    Destination address on the target (mem addr / aux register offset)
\param
   val:     points to the destination to return the value.

\return
   IFX_SUCCESS
   IFX_ERROR
*/
IFX_int32_t MEI_OnlineGpaRd(
                        MEI_DYN_CNTRL_T *pMeiDynCntrl,
                        IFX_boolean_t     aux,
                        IFX_uint32_t      addr,
                        IFX_uint32_t      *val)
{
   IFX_uint32_t      msg_size;
   unsigned short    msgId;
   MEI_DEV_T   *pMeiDev = pMeiDynCntrl->pMeiDev;
   CMV_STD_MESSAGE_T gpaMsg;

   PRN_DBG_USR( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL, MEI_DRV_LINENUM_GET(pMeiDev),
          ("MEI_DRV[%02d]: GPA Msg (OnL) Read, %s = 0x%08X" MEI_DRV_CRLF,
           MEI_DRV_LINENUM_GET(pMeiDev), ((aux) ? "aux" : "mem"), addr ));

   /*
      setup the GPA message
   */
   memset(&gpaMsg, 0x00, sizeof(gpaMsg));
   gpaMsg.header.mbxCode =  MEI_MBOX_CODE_MSG_WRITE;

   /* msgId2cmv: setup CMV header fields */
   msgId = (aux) ? CMD_DBG_AUX_REGISTER_READ : CMD_DBG_MEM_MAP_READ;
   MEI_MsgId2CmvHeader( pMeiDev, &gpaMsg, msgId);

   /*
      set payload fields - always 2 (16 bit unit)
   */
   CMV_MSGHDR_PAYLOAD_SIZE_SET(gpaMsg, 2);

   gpaMsg.payload.params_32Bit[0] = addr;

   /* mailbox message size: header + payload */
   msg_size = CMV_HEADER_8BIT_SIZE + sizeof(unsigned int) * 1;

   /* keep msgId for check ACK */
   pMeiDev->gpaBuf.MessageID = (IFX_uint32_t)msgId;

   if ( (MEI_WriteMailbox( pMeiDynCntrl, NULL,
                             (MEI_MEI_MAILBOX_T *)&gpaMsg,
                             msg_size )) == 0 )
   {
      /* error while send */
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DRV[%02d]: ERROR: send GPA msg (OnL) <%s>" MEI_DRV_CRLF,
              MEI_DRV_LINENUM_GET(pMeiDev),
              (aux) ? "CMD_DBG_AuxRegisterRead" : "CMD_DBG_MemMapRead"));

      pMeiDev->gpaBuf.MessageID = 0;
      return IFX_ERROR;
   }

   PRN_DBG_USR( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL, MEI_DRV_LINENUM_GET(pMeiDev),
          ("MEI_DRV[%02d]: GPA Msg (OnL) Read - wait for response" MEI_DRV_CRLF,
           MEI_DRV_LINENUM_GET(pMeiDev)));

   /* setup timeout counter for ACK */
   MEI_SET_TIMEOUT_CNT( pMeiDev,
                  MEI_MaxWaitDfeResponce_ms / MEI_MIN_MAILBOX_POLL_TIME_MS);

   /* poll for response */
   while(1)
   {
      /* wait: for an response from the VRX boot loader */
      MEI_PollIntPerVrxLine(pMeiDev, e_MEI_DEV_ACCESS_MODE_PASSIV_POLL);

      if (MEI_DRV_MAILBOX_STATE_GET(pMeiDev) != e_MEI_MB_FREE)
      {
         if ( MEI_WaitForMailbox(pMeiDev) != IFX_SUCCESS)
            break;
      }
      else
         break;
   }


   /* check result */
   if ( (IFX_uint16_t)pMeiDev->gpaBuf.MessageID != msgId )
   {
      /* error */
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
             ("MEI_DRV[%02d]: Read GPA (OnL), timeout for msgId 0x%04X (recv 0x%04X)" MEI_DRV_CRLF,
              MEI_DRV_LINENUM_GET(pMeiDev), msgId, gpaMsg.header.MessageID ));

      pMeiDev->gpaBuf.MessageID = 0;
      return IFX_ERROR;
   }
   else
   {
      pMeiDev->gpaBuf.MessageID = 0;
   }

   /* SUCCESS --> return value */
   switch (gpaMsg.header.MessageID)
   {
      case CMD_DBG_MEM_MAP_READ:
      case CMD_DBG_AUX_REGISTER_READ:
         *val = pMeiDev->gpaBuf.GpaBuffer[1];
         break;
      default:
         /* shit should never happen */
         return IFX_ERROR;
   }

   PRN_DBG_USR( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL, MEI_DRV_LINENUM_GET(pMeiDev),
          ("MEI_DRV[%02d]: GPA Msg (OnL) Read[0x%04X] - val = 0x%08X" MEI_DRV_CRLF,
           MEI_DRV_LINENUM_GET(pMeiDev), msgId, pMeiDev->gpaBuf.GpaBuffer[1]));

   return IFX_SUCCESS;
}



/**
   ACK for the following General Purpose Access messages
   - CMD_MEMORYMAPPEDREAD  --> ACK_MEMORYMAPPEDREAD
   - CMD_MEMORYMAPPEDWRITE --> ACK_MEMORYMAPPEDWRITE
   - CMD_AUXREGISTERWRITE  --> ACK_AUXREGISTERWRITE
   - CMD_AUXREGISTERREAD   --> ACK_AUXREGISTERREAD

\param
   pMeiDev  points to the device structure
\param
   pBootMsg points to the received boot code message.

\attention
   - Called on int-level
*/
IFX_int32_t MEI_OnlineOnGpaAckRecv(
                        MEI_DEV_T *pMeiDev,
                        CMV_STD_MESSAGE_T *pGpaMsg )
{

   /* check recv ack against outstanding cmd */
   if ( (IFX_uint16_t)(pMeiDev->gpaBuf.MessageID) != pGpaMsg->header.MessageID)
   {
      PRN_ERR_INT_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
           ("MEI_DRV[%02d]: mismatch msgId - GPA CMD = 0x%04X ACK = 0x%04X" MEI_DRV_CRLF,
             MEI_DRV_LINENUM_GET(pMeiDev), (IFX_uint16_t)pMeiDev->gpaBuf.MessageID, pGpaMsg->header.MessageID));

      MEI_LOG_CMV_MSG( pMeiDev, pGpaMsg,
                             "Recv mismatch ACK GPA (OL)", MEI_DRV_PRN_LEVEL_HIGH);

      return IFX_SUCCESS;
   }

   /* return data */
   switch(pGpaMsg->header.MessageID)
   {
      case ACK_DBG_MEM_MAP_WIRTE:
      case ACK_DBG_AUX_REGISTER_WRITE:
         /* no action necessary */
         break;

      case ACK_DBG_MEM_MAP_READ:
      case ACK_DBG_AUX_REGISTER_READ:
         /* copy data */
         pMeiDev->gpaBuf.GpaBuffer[0] = pGpaMsg->payload.params_32Bit[0];
         pMeiDev->gpaBuf.GpaBuffer[1] = pGpaMsg->payload.params_32Bit[1];
         break;

      default:
         /* shit */
         PRN_ERR_INT_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_WRN,
             ("MEI_DRV[%02d]: invalid GPA (OL) message msgId = 0x%04X" MEI_DRV_CRLF,
               MEI_DRV_LINENUM_GET(pMeiDev), pGpaMsg->header.MessageID));
         return IFX_SUCCESS;
   }

   /* return OpCode */
   pMeiDev->gpaBuf.MessageID = (IFX_uint32_t)pGpaMsg->header.MessageID;

   return IFX_SUCCESS;
}
#endif   /* #if (MEI_SUPPORT_DFE_GPA_ACCESS == 1) */


/* ============================================================================
   SWAP VDSL2 - ADSL firmware
   ========================================================================= */
#if (MEI_SUPPORT_VDSL2_ADSL_SWAP == 1)

/**
   Restart the FW on the chip - startup from the beginning.

\param
   pMeiDynCntrl points to the dynamic control struct. [I]

\return
   0 (IFX_SUCCESS) if success
   negative value in case of error

*/
IFX_int32_t MEI_CMD_DSL_ModeModify(
                              MEI_DYN_CNTRL_T    *pMeiDynCntrl,
                              MEI_DEV_CFG_FW_MODE_E eFwMode)
{
   IFX_int32_t          modemMsgSize, modemPaylSize;
   MEI_DYN_CMD_DATA_T *pDynCmd   = pMeiDynCntrl->pInstDynCmd;
   CMV_STD_MESSAGE_T    *pModemMsg = (CMV_STD_MESSAGE_T *)pDynCmd->cmdWrBuf.pBuffer;

   /* lock the the current instance */
   MEI_DRVOS_SemaphoreLock(&pMeiDynCntrl->pInstanceRWlock);

   /* ===============================================
      check if the instance specific buffer is free
      =============================================== */
   MEI_WaitForInstance(pMeiDynCntrl, pMeiDynCntrl->pMeiDev, pDynCmd);

   /* ===============================================
      Setup MODEM_CONTROL message header
      =============================================== */
   modemPaylSize = 1 * sizeof(IFX_uint16_t);
   modemMsgSize  = CMV_HEADER_8BIT_SIZE + modemPaylSize;
   memset(pModemMsg, 0x00, modemMsgSize);

   pModemMsg->header.mbxCode =  MEI_MBOX_CODE_MSG_WRITE;
   MEI_MsgId2CmvHeader( pMeiDynCntrl->pMeiDev, pModemMsg,
                          (IFX_uint16_t)MEI_DEV_CMD_DSL_MODE_MODIFY );

    /* size field contains number of 16 bit payload elements of the message */
   P_CMV_MSGHDR_PAYLOAD_SIZE_SET( pModemMsg,
                                  ((IFX_uint32_t)modemPaylSize >> CMV_MSG_BIT_SIZE_16BIT) );

   /* ===============================================
      Setup MODEM_CONTROL message payload
      =============================================== */
   pModemMsg->header.index  = 0;
   pModemMsg->header.length = modemPaylSize >> CMV_MSG_BIT_SIZE_16BIT;

      /* set the restart command for swap */
   switch (eFwMode)
   {
      case e_MEI_DEV_CFG_MODE_VDSL2:
      pModemMsg->payload.params_16Bit[0] = 0;
      break;

      case e_MEI_DEV_CFG_MODE_ADSL:
      pModemMsg->payload.params_16Bit[0] = 1;
      break;
   }


   /* ===============================================
      Send MODEM_CONTROL
      =============================================== */
   if ( (MEI_WriteMsgAndCheck( pMeiDynCntrl, pDynCmd,
                           (MEI_MEI_MAILBOX_T *)pModemMsg,
                           modemMsgSize) ) != IFX_SUCCESS )
   {
      MEI_DRVOS_SemaphoreUnlock(&pMeiDynCntrl->pInstanceRWlock);

      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DRV[%02d - %02d]: ERROR - FW Restart!" MEI_DRV_CRLF
             , MEI_DRV_DYN_LINENUM_GET(pMeiDynCntrl), pMeiDynCntrl->openInstance));

      return IFX_ERROR;
   }

   MEI_DRVOS_SemaphoreUnlock(&pMeiDynCntrl->pInstanceRWlock);

   return IFX_SUCCESS;
}


/**
   Select the firmware mode (VDSL2 or ADSL) for startup or swap.

\param
   pMeiDev   private device data [I]
\param
   eFwMode     selects the mode (VDSL2 or ADSL)

\return
   0 (IFX_SUCCESS) if success
   negative value in case of error

\remark
   Called on int and user level.
*/
IFX_int32_t MEI_DevCfgFwModeSelect(
                              MEI_DEV_T             *pMeiDev,
                              MEI_DEV_CFG_FW_MODE_E eFwMode)
{
   IFX_uint32_t regVal = 0;

   /* get current value and clear bit */
   MEI_ReadDma32Bit( &pMeiDev->meiDrvCntrl,
                     MEI_DEV_CFG_FW_MODE_SELECT_REG,
                     &regVal, 1);

   switch (eFwMode)
   {
      case e_MEI_DEV_CFG_MODE_VDSL2:
         /* VDSL mode - clear bit[29] */
         regVal &= ~(0x00000001 << MEI_DEV_CFG_FW_MODE_SELECT_BIT_POS);

         /* set the VDSL2 mode */
         PRN_DBG_INT( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL, MEI_DRV_LINENUM_GET(pMeiDev),
               ("MEI_DRV[%02d]: Device Cfg - select FW mode = <VDSL2> (0x%08X = 0x%08X)" MEI_DRV_CRLF
                , MEI_DRV_LINENUM_GET(pMeiDev)
                , MEI_DEV_CFG_FW_MODE_SELECT_REG
                , regVal));
         break;
      case e_MEI_DEV_CFG_MODE_ADSL:
         /* ADSL mode - set bit[29] */
         regVal |= (0x00000001 << MEI_DEV_CFG_FW_MODE_SELECT_BIT_POS);

         /* set the ADSL mode */
         PRN_DBG_INT( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL, MEI_DRV_LINENUM_GET(pMeiDev),
               ("MEI_DRV[%02d]: Device Cfg - select FW mode = <ADSL> (0x%08X = 0x%08X)" MEI_DRV_CRLF
                , MEI_DRV_LINENUM_GET(pMeiDev)
                , MEI_DEV_CFG_FW_MODE_SELECT_REG
                , regVal));
         break;
      default:
         PRN_DBG_INT( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR, MEI_DRV_LINENUM_GET(pMeiDev),
               ("MEI_DRV[%02d]: Device Cfg - FW mode invalid mode = %d" MEI_DRV_CRLF
                , MEI_DRV_LINENUM_GET(pMeiDev), (IFX_int32_t)eFwMode));
         return IFX_ERROR;
   }

   MEI_WriteDma32Bit( &pMeiDev->meiDrvCntrl,
                      MEI_DEV_CFG_FW_MODE_SELECT_REG,
                      &regVal, 1, 0);

   return IFX_SUCCESS;
}


/**
   Select a FW mode and restart the firmware (swap).

\param
   pMeiDynCntrl points to the dynamic control struct.
\param
   eFwMode     selects the mode (VDSL2 or ADSL)

\return
   0 (IFX_SUCCESS) if success
   negative value in case of error

*/
IFX_int32_t MEI_DevCfgFwModeSwap(
                              MEI_DYN_CNTRL_T       *pMeiDynCntrl,
                              MEI_DEV_CFG_FW_MODE_E eFwMode)
{
   PRN_DBG_USR( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL, MEI_DRV_DYN_LINENUM_GET(pMeiDynCntrl),
         ("MEI_DRV[%02d - %02d]: Device Cfg, swap FW mode %d" MEI_DRV_CRLF
          , MEI_DRV_DYN_LINENUM_GET(pMeiDynCntrl), pMeiDynCntrl->openInstance
          , (IFX_int32_t)eFwMode));

   /* set the given mode (VDSL2 or ADSL) */
   if (MEI_DevCfgFwModeSelect( pMeiDynCntrl->pMeiDev, eFwMode ) != IFX_SUCCESS)
   {
      return IFX_ERROR;
   }

   /* restart the FW for swap */
   if ( MEI_CMD_DSL_ModeModify( pMeiDynCntrl, eFwMode ) != IFX_SUCCESS )
   {
      return IFX_ERROR;
   }

   return IFX_SUCCESS;
}

#endif      /* #if (MEI_SUPPORT_VDSL2_ADSL_SWAP == 1) */

/* ============================================================================
   ATM OAM part
   ========================================================================= */
#if (MEI_DRV_ATM_OAM_ENABLE == 1)

/**
   Write the VRX CMD_ATMINSERTEXTRACT_CONTROL command.
   Enable / Disable the ATM OAM feature and configure the alarm handling.

\param
   pMeiDynCntrl    points to the dynamic control struct.
\param
   pAtmOamDevCntrl   points to the ATM OAM control struct.

\return
   0 (IFX_SUCCESS) if success
   negative value in case of error
*/
IFX_int32_t MEI_ATMOAM_CMD_InsExtControl(
                              MEI_DYN_CNTRL_T        *pMeiDynCntrl,
                              MEI_ATMOAM_DEV_CNTRL_T *pAtmOamDevCntrl)
{
   IFX_int32_t          modemMsgSize, modemPaylSize;
   MEI_DYN_CMD_DATA_T *pDynCmd   = pMeiDynCntrl->pInstDynCmd;
   CMV_STD_MESSAGE_T    *pModemMsg = (CMV_STD_MESSAGE_T *)pDynCmd->cmdWrBuf.pBuffer;

   CMD_AtmInsertExtract_Control_t *pIfxMsg = (CMD_AtmInsertExtract_Control_t *)&pModemMsg->header.index;

   if ( MEI_DevCntlMsgAvailable( pMeiDynCntrl,
                                   (IFX_uint32_t)MEI_DRV_CMD_ATMINSERTEXTRACT_CONTROL)
        != IFX_SUCCESS)
   {
      return -e_MEI_ERR_REM_INVAL_LINE_STATE;
   }

   PRN_DBG_USR( MEI_ATMOAM, MEI_DRV_PRN_LEVEL_NORMAL, MEI_DRV_DYN_LINENUM_GET(pMeiDynCntrl),
         ("MEI_ATM[%02d - %02d]: device ATM OAM control" MEI_DRV_CRLF
          , MEI_DRV_DYN_LINENUM_GET(pMeiDynCntrl), pMeiDynCntrl->openInstance));

   /* lock the the current instance */
   MEI_DRVOS_SemaphoreLock(&pMeiDynCntrl->pInstanceRWlock);

   /* ===============================================
      check if the instance specific buffer is free
      =============================================== */
   MEI_WaitForInstance(pMeiDynCntrl, pMeiDynCntrl->pMeiDev, pDynCmd);

   /* ===============================================
      Setup CMD_ATMINSERTEXTRACT_CONTROL header
      =============================================== */
   modemPaylSize = sizeof(CMD_AtmInsertExtract_Control_t) - (2 * sizeof(IFX_uint16_t));
   modemMsgSize  = CMV_HEADER_8BIT_SIZE + modemPaylSize;
   memset(pModemMsg, 0x00, modemMsgSize);

   pModemMsg->header.mbxCode =  MEI_MBOX_CODE_MSG_WRITE;
   MEI_MsgId2CmvHeader( pMeiDynCntrl->pMeiDev, pModemMsg,
                          MEI_DRV_MODEM_MSG_ID_GET(MEI_DRV_CMD_ATMINSERTEXTRACT_CONTROL) );
    /* size field contains number of 16 bit payload elements of the message */
   P_CMV_MSGHDR_PAYLOAD_SIZE_SET( pModemMsg,
                                  ((IFX_uint32_t)modemPaylSize >> CMV_MSG_BIT_SIZE_16BIT) );

   /* ===============================================
      Setup CMD_ATMINSERTEXTRACT_CONTROL payload
      =============================================== */
   pIfxMsg->Index            = 0;
   pIfxMsg->Length           = modemPaylSize >> CMV_MSG_BIT_SIZE_32BIT;
   pIfxMsg->LinkNo           = pAtmOamDevCntrl->linkNo;
   pIfxMsg->direction        = pAtmOamDevCntrl->dir;
   pIfxMsg->Control          = (pAtmOamDevCntrl->bEnInsExt) ? 1 : 0;
   pIfxMsg->failMsgControl   = (pAtmOamDevCntrl->bEnAlmOnExtract) ? 1 : 0;
   pIfxMsg->insertEVT_Status = (pAtmOamDevCntrl->bEnEvtOnInsert) ? 1 : 0;

   /* ===============================================
      Send CMD_ATMINSERTEXTRACT_CONTROL
      =============================================== */
   if ( (MEI_WriteMsgAndCheck(
               pMeiDynCntrl, pDynCmd,
               (MEI_MEI_MAILBOX_T *)pModemMsg,
               modemMsgSize) ) != IFX_SUCCESS )
   {
      MEI_DRVOS_SemaphoreUnlock(&pMeiDynCntrl->pInstanceRWlock);

      PRN_ERR_USR_NL( MEI_ATMOAM, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_ATM[%02d - %02d]: ERROR - set ATM OAM control failed!" MEI_DRV_CRLF,
             MEI_DRV_DYN_LINENUM_GET(pMeiDynCntrl), pMeiDynCntrl->openInstance));

      return IFX_ERROR;
   }

   MEI_DRVOS_SemaphoreUnlock(&pMeiDynCntrl->pInstanceRWlock);

   return IFX_SUCCESS;
}



/**
   Write the VRX CMD_ATMINSERTEXTRACTSTATSGET command.
   Request the ATM OAM insert / extract status infos.

\param
   pMeiDynCntrl    points to the dynamic control struct.
\param
   pAtmOamDevCntrl   points to the ATM OAM control struct.
\param
   pAtmOamStats      points to the counter struct.

\return
   0 (IFX_SUCCESS) if success
   negative value in case of error
*/
IFX_int32_t MEI_ATMOAM_CMD_InsExtStatsGet(
                              MEI_DYN_CNTRL_T             *pMeiDynCntrl,
                              MEI_ATMOAM_DEV_CNTRL_T      *pAtmOamDevCntrl,
                              IOCTL_MEI_ATMOAM_counter_t  *pAtmOamStats)
{
   IFX_int32_t          modemMsgSize, modemPaylSize;
   MEI_DYN_CMD_DATA_T *pDynCmd   = pMeiDynCntrl->pInstDynCmd;
   CMV_STD_MESSAGE_T    *pModemMsg = (CMV_STD_MESSAGE_T *)pDynCmd->cmdWrBuf.pBuffer;
   CMV_STD_MESSAGE_T    *pModemAck = (CMV_STD_MESSAGE_T *)pDynCmd->cmdAckCntrl.recvDataBuf_s.pBuffer;

   CMD_AtmInsertExtractStatsGet_t *pIfxMsg = (CMD_AtmInsertExtractStatsGet_t *)&pModemMsg->header.index;
   ACK_AtmInsertExtractStatsGet_t *pIfxAck = (ACK_AtmInsertExtractStatsGet_t *)&pModemAck->header.index;

   if ( MEI_DevCntlMsgAvailable( pMeiDynCntrl,
                                   (IFX_uint32_t)MEI_DRV_CMD_ATMINSERTEXTRACTSTATSGET)
        != IFX_SUCCESS)
   {
      return -e_MEI_ERR_REM_INVAL_LINE_STATE;
   }

   PRN_DBG_USR( MEI_ATMOAM, MEI_DRV_PRN_LEVEL_NORMAL, MEI_DRV_DYN_LINENUM_GET(pMeiDynCntrl),
         ("MEI_ATM[%02d - %02d]: ATM OAM stats get" MEI_DRV_CRLF
          , MEI_DRV_DYN_LINENUM_GET(pMeiDynCntrl), pMeiDynCntrl->openInstance));

   /* lock the the current instance */
   MEI_DRVOS_SemaphoreLock(&pMeiDynCntrl->pInstanceRWlock);

   /* ===============================================
      check if the instance specific buffer is free
      =============================================== */
   MEI_WaitForInstance(pMeiDynCntrl, pMeiDynCntrl->pMeiDev, pDynCmd);

   /* ===============================================
      Setup CMD_ATMINSERTEXTRACTSTATSGET header
      =============================================== */
   modemPaylSize = sizeof(CMD_AtmInsertExtractStatsGet_t) - (2 * sizeof(IFX_uint16_t));
   modemMsgSize  = CMV_HEADER_8BIT_SIZE + modemPaylSize;
   memset(pModemMsg, 0x00, modemMsgSize);

   pModemMsg->header.mbxCode =  MEI_MBOX_CODE_MSG_WRITE;
   MEI_MsgId2CmvHeader( pMeiDynCntrl->pMeiDev, pModemMsg,
                          MEI_DRV_MODEM_MSG_ID_GET(MEI_DRV_CMD_ATMINSERTEXTRACTSTATSGET) );
    /* size field contains number of 16 bit payload elements of the message */
   P_CMV_MSGHDR_PAYLOAD_SIZE_SET( pModemMsg,
                                  ((IFX_uint32_t)modemPaylSize >> CMV_MSG_BIT_SIZE_16BIT) );

   /* ===============================================
      Setup CMD_ATMINSERTEXTRACTSTATSGET payload
      =============================================== */
   pIfxMsg->Index          = 0;
   pIfxMsg->Length         = modemPaylSize >> CMV_MSG_BIT_SIZE_32BIT;
   pIfxMsg->LinkNo         = pAtmOamDevCntrl->linkNo;
   pIfxMsg->direction      = pAtmOamDevCntrl->dir;

   /* ===============================================
      Send CMD_ATMINSERTEXTRACTSTATSGET
      =============================================== */
   if ( (MEI_WriteMsgAndCheck(
               pMeiDynCntrl, pDynCmd,
               (MEI_MEI_MAILBOX_T *)pModemMsg,
               modemMsgSize) ) != IFX_SUCCESS )
   {
      MEI_DRVOS_SemaphoreUnlock(&pMeiDynCntrl->pInstanceRWlock);

      PRN_ERR_USR_NL( MEI_ATMOAM, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_ATM[%02d - %02d]: ERROR - ATM OAM stats get failed!" MEI_DRV_CRLF
             , MEI_DRV_DYN_LINENUM_GET(pMeiDynCntrl), pMeiDynCntrl->openInstance));

      return IFX_ERROR;
   }

   /* ===============================================
      Check the response CMD_ATMINSERTEXTRACTSTATSGET
      =============================================== */
   if (pAtmOamStats)
   {
      pAtmOamStats->extrCellCnt       = (unsigned int)pIfxAck->extractedCells;
      pAtmOamStats->extrCellFailedCnt = (unsigned int)pIfxAck->failExtractCells;
      pAtmOamStats->forwardCellCnt    = 0;
      pAtmOamStats->insCellCnt        = (unsigned int)pIfxAck->insertedCells;
   }

   MEI_DRVOS_SemaphoreUnlock(&pMeiDynCntrl->pInstanceRWlock);

   return IFX_SUCCESS;
}

/**
   Write the VRX CMD_AtmCellLineInsertStatusGet command.
   Request the status of the previous ATM OAM insert operation.

\param
   pMeiDynCntrl    points to the dynamic control struct.
\param
   pAtmOamDevCntrl   points to the ATM OAM control struct.
\param
   pInsStatus        points to the insert status.

\return
   0 (IFX_SUCCESS) if success
   negative value in case of error
*/
IFX_int32_t MEI_ATMOAM_CMD_InsStatusGet(
                              MEI_DYN_CNTRL_T             *pMeiDynCntrl,
                              MEI_ATMOAM_DEV_CNTRL_T      *pAtmOamDevCntrl,
                              IFX_uint32_t                  *pInsStatus)
{
   IFX_int32_t          modemMsgSize, modemPaylSize;
   MEI_DYN_CMD_DATA_T *pDynCmd   = pMeiDynCntrl->pInstDynCmd;
   CMV_STD_MESSAGE_T    *pModemMsg = (CMV_STD_MESSAGE_T *)pDynCmd->cmdWrBuf.pBuffer;
   CMV_STD_MESSAGE_T    *pModemAck = (CMV_STD_MESSAGE_T *)pDynCmd->cmdAckCntrl.recvDataBuf_s.pBuffer;

   CMD_AtmCellLineInsertStatusGet_t *pIfxMsg = (CMD_AtmCellLineInsertStatusGet_t *)&pModemMsg->header.index;
   ACK_AtmCellLineInsertStatusGet_t *pIfxAck = (ACK_AtmCellLineInsertStatusGet_t *)&pModemAck->header.index;

   if ( MEI_DevCntlMsgAvailable( pMeiDynCntrl,
                                   (IFX_uint32_t)MEI_DRV_CMD_ATMCELLLINEINSERTSTATUSGET)
        != IFX_SUCCESS)
   {
      return -e_MEI_ERR_REM_INVAL_LINE_STATE;
   }

   PRN_DBG_USR( MEI_ATMOAM, MEI_DRV_PRN_LEVEL_NORMAL, MEI_DRV_DYN_LINENUM_GET(pMeiDynCntrl),
         ("MEI_ATM[%02d - %02d]: ATM OAM ins line status get" MEI_DRV_CRLF
          , MEI_DRV_DYN_LINENUM_GET(pMeiDynCntrl), pMeiDynCntrl->openInstance));

   /* lock the the current instance */
   MEI_DRVOS_SemaphoreLock(&pMeiDynCntrl->pInstanceRWlock);

   /* ===============================================
      check if the instance specific buffer is free
      =============================================== */
   MEI_WaitForInstance(pMeiDynCntrl, pMeiDynCntrl->pMeiDev, pDynCmd);

   /* ===============================================
      Setup CMD_AtmCellLineInsertStatusGet header
      =============================================== */
   modemPaylSize = sizeof(CMD_AtmCellLineInsertStatusGet_t) - (2 * sizeof(IFX_uint16_t));
   modemMsgSize  = CMV_HEADER_8BIT_SIZE + modemPaylSize;
   memset(pModemMsg, 0x00, modemMsgSize);

   pModemMsg->header.mbxCode =  MEI_MBOX_CODE_MSG_WRITE;
   MEI_MsgId2CmvHeader( pMeiDynCntrl->pMeiDev, pModemMsg,
                          MEI_DRV_MODEM_MSG_ID_GET(MEI_DRV_CMD_ATMCELLLINEINSERTSTATUSGET) );
    /* size field contains number of 16 bit payload elements of the message */
   P_CMV_MSGHDR_PAYLOAD_SIZE_SET( pModemMsg,
                                  ((IFX_uint32_t)modemPaylSize >> CMV_MSG_BIT_SIZE_16BIT) );

   /* ===============================================
      Setup CMD_AtmCellLineInsertStatusGet payload
      =============================================== */
   pIfxMsg->Index          = 0;
   pIfxMsg->Length         = modemPaylSize >> CMV_MSG_BIT_SIZE_32BIT;
   pIfxMsg->LinkNo         = pAtmOamDevCntrl->linkNo;

   /* ===============================================
      Send CMD_AtmCellLineInsertStatusGet
      =============================================== */
   if ( (MEI_WriteMsgAndCheck(
               pMeiDynCntrl, pDynCmd,
               (MEI_MEI_MAILBOX_T *)pModemMsg,
               modemMsgSize) ) != IFX_SUCCESS )
   {
      MEI_DRVOS_SemaphoreUnlock(&pMeiDynCntrl->pInstanceRWlock);

      PRN_ERR_USR_NL( MEI_ATMOAM, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_ATM[%02d - %02d]: ERROR - ATM OAM ins line status get failed!" MEI_DRV_CRLF
             , MEI_DRV_DYN_LINENUM_GET(pMeiDynCntrl), pMeiDynCntrl->openInstance));

      return IFX_ERROR;
   }

   /* ===============================================
      Check the response CMD_AtmCellLineInsertStatusGet
      =============================================== */
   switch (pIfxAck->insertStatus)
   {
      case ACK_ATMCELLLINEINSERTSTATUSGET_INSERT_PROG:
         MEI_ATMOAM_TXBUF_STATE_SET(pAtmOamDevCntrl, eMEI_ATMOAM_OP_TX_BUF_BUSY);
         break;
      case ACK_ATMCELLLINEINSERTSTATUSGET_INSERT_DONE:
         MEI_ATMOAM_TXBUF_STATE_SET(pAtmOamDevCntrl, eMEI_ATMOAM_OP_TX_BUF_FREE);
         break;
      case ACK_ATMCELLLINEINSERTSTATUSGET_INSERT_ERR:
         MEI_ATMOAM_TXBUF_STATE_SET(pAtmOamDevCntrl, eMEI_ATMOAM_OP_TX_BUF_ERROR);
         break;
      default:
         MEI_ATMOAM_TXBUF_STATE_SET(pAtmOamDevCntrl, eMEI_ATMOAM_OP_TX_BUF_ERROR);
         PRN_ERR_USR_NL( MEI_ATMOAM, MEI_DRV_PRN_LEVEL_ERR,
               ("MEI_ATM[%02d - %02d]: ERROR - ATM OAM ins line status, invalid state 0x%X!" MEI_DRV_CRLF,
                MEI_DRV_DYN_LINENUM_GET(pMeiDynCntrl), pMeiDynCntrl->openInstance,
                pIfxAck->insertStatus));
   }

   if (pInsStatus)
   {
      *pInsStatus = pIfxAck->insertStatus;
   }

   MEI_DRVOS_SemaphoreUnlock(&pMeiDynCntrl->pInstanceRWlock);

   return IFX_SUCCESS;
}

/**
   Write the VRX CMD_ATMCELLLINEINSERT command.
   Insert a ATM OAM cell block to the line.

\param
   pMeiDynCntrl    points to the dynamic control struct.
\param
   pAtmOamDevCntrl   points to the ATM OAM control struct.
\param
   pRawAtmCellBlock  points to the ATM cell block.
\param
   cellCnt           number of ATM cells

\return
   0 (IFX_SUCCESS) if success
   negative value in case of error
*/
IFX_int32_t MEI_ATMOAM_CMD_AtmCellLineInsert(
                              MEI_DYN_CNTRL_T             *pMeiDynCntrl,
                              MEI_ATMOAM_DEV_CNTRL_T      *pAtmOamDevCntrl,
                              IOCTL_MEI_ATMOAM_rawCell_t  *pRawAtmCellBlock,
                              IFX_uint32_t                  cellCnt)
{
   IFX_int32_t          modemMsgSize, modemPaylSize;
   IFX_uint32_t         idx;
   MEI_DYN_CMD_DATA_T *pDynCmd    = pMeiDynCntrl->pInstDynCmd;
   CMV_STD_MESSAGE_T    *pModemMsg  = (CMV_STD_MESSAGE_T *)pDynCmd->cmdWrBuf.pBuffer;
   CMD_AtmCellLineInsert_t *pIfxMsg = (CMD_AtmCellLineInsert_t *)&pModemMsg->header.index;

   if ( MEI_DevCntlMsgAvailable( pMeiDynCntrl,
                                   (IFX_uint32_t)MEI_DRV_CMD_ATMCELLLINEINSERT)
        != IFX_SUCCESS)
   {
      return -e_MEI_ERR_REM_INVAL_LINE_STATE;
   }

   PRN_DBG_USR( MEI_ATMOAM, MEI_DRV_PRN_LEVEL_NORMAL, MEI_DRV_DYN_LINENUM_GET(pMeiDynCntrl),
         ("MEI_ATM[%02d - %02d]: ATM OAM insert" MEI_DRV_CRLF
          , MEI_DRV_DYN_LINENUM_GET(pMeiDynCntrl), pMeiDynCntrl->openInstance));

   /* lock the the current instance */
   MEI_DRVOS_SemaphoreLock(&pMeiDynCntrl->pInstanceRWlock);

   /* ===============================================
      check if the instance specific buffer is free
      =============================================== */
   MEI_WaitForInstance(pMeiDynCntrl, pMeiDynCntrl->pMeiDev, pDynCmd);

   /* ===============================================
      Setup CMD_ATMINSERTEXTRACT_CONTROL header
      =============================================== */
   modemPaylSize = sizeof(IFX_uint32_t) + (cellCnt * sizeof(MEI_ATMcell_t));
   modemMsgSize  = CMV_HEADER_8BIT_SIZE + modemPaylSize;
   memset(pModemMsg, 0x00, modemMsgSize);

   pModemMsg->header.mbxCode =  MEI_MBOX_CODE_MSG_WRITE;
   MEI_MsgId2CmvHeader( pMeiDynCntrl->pMeiDev, pModemMsg,
                          MEI_DRV_MODEM_MSG_ID_GET(MEI_DRV_CMD_ATMCELLLINEINSERT) );
    /* size field contains number of 16 bit payload elements of the message */
   P_CMV_MSGHDR_PAYLOAD_SIZE_SET( pModemMsg,
                                  ((IFX_uint32_t)modemPaylSize >> CMV_MSG_BIT_SIZE_16BIT) );

   /* ===============================================
      Setup CMD_ATMINSERTEXTRACT_CONTROL payload
      =============================================== */
   pIfxMsg->Index          = 0;
   pIfxMsg->Length         = modemPaylSize >> CMV_MSG_BIT_SIZE_32BIT;
   pIfxMsg->LinkNo         = pAtmOamDevCntrl->linkNo;

   /* copy the given ATM cells to the message !! keep in mind the order !! */
   for (idx = 0; idx < cellCnt; idx++)
   {
      memcpy( &pIfxMsg->ATMCells[idx],
              &pRawAtmCellBlock[idx],
              (sizeof(MEI_ATMcell_t)));
   }

   /* ===============================================
      Send CMD_ATMINSERTEXTRACT_CONTROL
      =============================================== */
   if ( !(MEI_ATMOAM_CHECK_TRANSMODE_EVT_TX(pAtmOamDevCntrl)) )
   {
      MEI_ATMOAM_TXBUF_STATE_SET(pAtmOamDevCntrl, eMEI_ATMOAM_OP_TX_BUF_BUSY);
   }

   if ( (MEI_WriteMsgAndCheck(
               pMeiDynCntrl, pDynCmd,
               (MEI_MEI_MAILBOX_T *)pModemMsg,
               modemMsgSize) ) != IFX_SUCCESS )
   {
      MEI_ATMOAM_STAT_INC_INST_MSG_ERR_CNT(pAtmOamDevCntrl);
      MEI_ATMOAM_TXBUF_STATE_SET(pAtmOamDevCntrl, eMEI_ATMOAM_OP_TX_BUF_FREE);
      MEI_DRVOS_SemaphoreUnlock(&pMeiDynCntrl->pInstanceRWlock);

      PRN_ERR_USR_NL( MEI_ATMOAM, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_ATM[%02d - %02d]: ERROR - ATM OAM insert failed!" MEI_DRV_CRLF
             , MEI_DRV_DYN_LINENUM_GET(pMeiDynCntrl), pMeiDynCntrl->openInstance));

      return IFX_ERROR;
   }

   MEI_ATMOAM_STAT_INC_INST_MSG_CNT(pAtmOamDevCntrl);
   MEI_ATMOAM_STAT_ADD_INST_CELL_CNT(pAtmOamDevCntrl, cellCnt);
   MEI_DRVOS_SemaphoreUnlock(&pMeiDynCntrl->pInstanceRWlock);

   return IFX_SUCCESS;
}


/**
   Autonomous message - notification for TX cell done

\param
   pMeiDynCntrl          points to the dynamic control struct.
\param
   pAtmOamDevCntrl         points to the ATM OAM control struct.
\param
   pNFC_AtmCellLineInsert  points to the received NFC message.

\return
   0 (IFX_SUCCESS) if success
   negative value in case of error

\remark
   Called in int-level
*/
IFX_int32_t MEI_ATMOAM_doEVT_AtmCellLineInsertStatus(
                              MEI_DEV_T                *pMeiDev,
                              MEI_ATMOAM_DEV_CNTRL_T   *pAtmOamDevCntrl,
                              CMV_STD_MESSAGE_T          *pModemMsg)
{
   EVT_AtmCellLineInsertStatusGet_t *pEVT_AtmCellLineInsert = IFX_NULL;
   pEVT_AtmCellLineInsert = (EVT_AtmCellLineInsertStatusGet_t *)&pModemMsg->header.index;

   if (MEI_ATMOAM_TXBUF_STATE_GET(pAtmOamDevCntrl) != eMEI_ATMOAM_OP_TX_BUF_BUSY)
   {
      PRN_ERR_INT_NL( MEI_ATMOAM, MEI_DRV_PRN_LEVEL_WRN,
            ("MEI_ATM[%02d]: WRN - TX NFC while buffer not busy!" MEI_DRV_CRLF
             , MEI_DRV_LINENUM_GET(pMeiDev)));
   }

   MEI_ATMOAM_STAT_INC_INST_MSG_NFC_CNT(pAtmOamDevCntrl);

   /* Message check */
   if (pEVT_AtmCellLineInsert->insertStatus == EVT_ATMCELLLINEINSERTSTATUSGET_INSERT_DONE)
   {
      MEI_ATMOAM_TXBUF_STATE_SET(pAtmOamDevCntrl, eMEI_ATMOAM_OP_TX_BUF_FREE);
   }
   else
   {
      MEI_ATMOAM_TXBUF_STATE_SET(pAtmOamDevCntrl, eMEI_ATMOAM_OP_TX_BUF_ERROR);
   }

   if (pAtmOamDevCntrl->bAtmOamInstDoneNeedWakeUp)
   {
      pAtmOamDevCntrl->bAtmOamInstDoneNeedWakeUp = IFX_FALSE;
      MEI_DRVOS_EventWakeUp(&pAtmOamDevCntrl->eventAtmOamInstDone);
   }

   return IFX_SUCCESS;
}


/**
   Autonomous message - alarm cell extract failed.

\param
   pMeiDynCntrl             points to the dynamic control struct.
\param
   pAtmOamDevCntrl            points to the ATM OAM control struct.
\param
   pALM_AtmCellExtractFailed  points to the received ALM message.

\return
   0 (IFX_SUCCESS) if success
   negative value in case of error

\remark
   Called in int-level
*/
IFX_int32_t MEI_ATMOAM_doALM_AtmCellExtractFailed(
                              MEI_DEV_T                *pMeiDev,
                              MEI_ATMOAM_DEV_CNTRL_T   *pAtmOamDevCntrl,
                              CMV_STD_MESSAGE_T          *pModemMsg,
                              IFX_uint32_t               *pLinkNo,
                              IFX_uint32_t               *pFailCount)
{
   ALM_AtmCellExtractFailed_t *pALM_AtmCellExtractFailed = (ALM_AtmCellExtractFailed_t *)&pModemMsg->header.index;

   /* take over the new statistic counter */
   MEI_ATMOAM_STAT_INC_ALM_MSG_CNT(pAtmOamDevCntrl);
   MEI_ATMOAM_STAT_SET_EXTR_FAILED_CELL_CNT( pAtmOamDevCntrl,
                                               (IFX_uint32_t)pALM_AtmCellExtractFailed->FailCount);

   if (pLinkNo)
      *pLinkNo = pALM_AtmCellExtractFailed->LinkNo;

   if (pFailCount)
      *pFailCount = pALM_AtmCellExtractFailed->FailCount;

   return IFX_SUCCESS;
}


/**
   Autonomous message - ATM cell line extract.

\param
   pMeiDynCntrl             points to the dynamic control struct.
\param
   pAtmOamDevCntrl            points to the ATM OAM control struct.
\param
   pEVT_AtmCellLineExtract    points to the received EVT message.

\return
   number of ATM cells or
   negative value in case of error

\remark
   Called in int-level
*/
IFX_int32_t MEI_ATMOAM_doEVT_AtmCellExtract(
                              MEI_DEV_T                   *pMeiDev,
                              MEI_ATMOAM_DEV_CNTRL_T      *pAtmOamDevCntrl,
                              CMV_STD_MESSAGE_T             *pModemMsg,
                              MEI_ATMOAM_CELL_BUFFER_T    *pRxAtmCells,
                              IFX_uint8_t                   bufferSize_Cells)
{
   IFX_int32_t  retVal = IFX_SUCCESS;
   IFX_uint32_t idx, cellCnt = 0;
   EVT_AtmCellLineExtract_t *pEVT_AtmCellLineExtract = (EVT_AtmCellLineExtract_t *)&pModemMsg->header.index;

   /* get cell count -->
      ((number of 32 bit params - LinkNo) * sizeof(32bit params)) / sizeof(ATM struct)
   */
   if (pEVT_AtmCellLineExtract->Length)
   {
      cellCnt = (IFX_uint32_t)( ((pEVT_AtmCellLineExtract->Length -1)
                                  * sizeof(IFX_uint32_t)) / sizeof(MEI_ATMcell_t) );

#if 0
      /* plausi check */
      if ( (((cellCnt * sizeof(MEI_ATMcell_t)) / sizeof(IFX_uint32_t)) + 1)
           != pEVT_AtmCellLineExtract->Length )
      {
         retVal = 11;
         goto MEI_ATMOAM_DOEVT_ATMCELLEXTRACT_ERR;
      }
#endif

      /* check range for RX */
      if ((cellCnt < 1) && (cellCnt > MEI_ATMOAM_MAX_RX_CELL_CNT))
      {
         retVal = 12;
         goto MEI_ATMOAM_DOEVT_ATMCELLEXTRACT_ERR;
      }

      /* check destination size */
      if (bufferSize_Cells < cellCnt)
      {
         retVal = 13;
         goto MEI_ATMOAM_DOEVT_ATMCELLEXTRACT_ERR;
      }
   }
   else
   {
         retVal = 10;
         goto MEI_ATMOAM_DOEVT_ATMCELLEXTRACT_ERR;
   }

   /*
      Takeover the received cells
   */
   for (idx = 0; idx < cellCnt; idx++)
   {
      /* keep in mind the ordering */
      memcpy( &(pRxAtmCells->atmCells[idx]),
              &(pEVT_AtmCellLineExtract->ATMcells[idx]),
              (sizeof(IOCTL_MEI_ATMOAM_rawCell_t)));
   }

   pRxAtmCells->cellCnt  = cellCnt;
   pRxAtmCells->atmOamId = MEI_DRV_MSG_CTRL_IF_MODEM_ATMOAM_CELL_ON;

   MEI_ATMOAM_STAT_INC_EXTR_MSG_CNT(pAtmOamDevCntrl);
   MEI_ATMOAM_STAT_ADD_EXTR_CELL_CNT(pAtmOamDevCntrl, (IFX_uint32_t)cellCnt);

   return IFX_SUCCESS;

MEI_ATMOAM_DOEVT_ATMCELLEXTRACT_ERR:
   MEI_ATMOAM_STAT_INC_EXTR_MSG_ERR_CNT(pAtmOamDevCntrl);

   PRN_ERR_INT_NL( MEI_ATMOAM, MEI_DRV_PRN_LEVEL_WRN,
         ("MEI_ATM[%02d]: ERROR - Extract Msg invalid, cellCnt = %d (- %d -)" MEI_DRV_CRLF
          , MEI_DRV_LINENUM_GET(pMeiDev), cellCnt, retVal));

   pRxAtmCells->cellCnt = 0;
   return IFX_ERROR;
}

#endif      /* #if (MEI_DRV_ATM_OAM_ENABLE == 1) */


/* ============================================================================
   Clear EOC Insert/Extract part
   ========================================================================= */
#if (MEI_DRV_CLEAR_EOC_ENABLE == 1)


MEI_STATIC IFX_int32_t MEI_CEOC_DataGet(
                              MEI_DEV_T               *pMeiDev,
                              MEI_CEOC_DEV_CNTRL_T    *pCEocDevCntrl,
                              MEI_CEOC_FRAME_BUFFER_T *pCEocFrameBuf,
                              EVT_ClearEOC_Read_t       *pCatMsgEvt);


/**
   Interprete and get the recieved data form the message to the internal buffer
   - ACK_CLEAREOC_READ message
   - EVT_CLEAREOC_READ message

\param
   pMeiDynCntrl    points to the dynamic control struct.
\param
   pCEocDevCntrl     points to the Clear EOC control struct.
\param
   pCEocFrameBuf     points to the internal EOC frame buffer (destination)
\param
   pCatMsgEvt        points to the recieved data.

\return
   0 (IFX_SUCCESS) if success
   negative value in case of error
*/
MEI_STATIC IFX_int32_t MEI_CEOC_DataGet(
                              MEI_DEV_T               *pMeiDev,
                              MEI_CEOC_DEV_CNTRL_T    *pCEocDevCntrl,
                              MEI_CEOC_FRAME_BUFFER_T *pCEocFrameBuf,
                              EVT_ClearEOC_Read_t       *pCatMsgEvt)
{
   IFX_int32_t                nextTransfer_bytes;
   IFX_uint8_t                *pEocDataSrc, *pEocDataDest;
   MEI_CEOC_MEI_EOC_FRAME_T *pVrxEocFrame = &pCEocFrameBuf->vrxEocFrame;


   if (pCatMsgEvt->Index == 0)
   {

      if (pCatMsgEvt->Length < 3)
      {
         /* Error - start segment while buffer not empty */
         PRN_ERR_USR_NL( MEI_CEOC, MEI_DRV_PRN_LEVEL_WRN,
               ("MEI_CEOC[%02d]: WRN - recv C-EOC Read - invalid msg!" MEI_DRV_CRLF,
                MEI_DRV_LINENUM_GET(pMeiDev)));

         pVrxEocFrame->length_byte = 0;
         pCEocFrameBuf->transferedSize_byte = 0;
         return IFX_ERROR;
      }

      /* first segement - setup buffer */
      if (pCEocFrameBuf->transferedSize_byte != 0)
      {
         /* Error - start segment while buffer not empty */
         PRN_ERR_USR_NL( MEI_CEOC, MEI_DRV_PRN_LEVEL_WRN,
               ("MEI_CEOC[%02d]: WRN - recv C-EOC Read - overwrite frame!" MEI_DRV_CRLF,
                MEI_DRV_LINENUM_GET(pMeiDev)));
      }

      pVrxEocFrame->length_byte = pCatMsgEvt->Data[0];
      pVrxEocFrame->protIdent   = pCatMsgEvt->Data[1];

      if (pVrxEocFrame->length_byte > (MEI_CEOC_RAW_EOC_DATA_SIZE_BYTE + sizeof(pVrxEocFrame->protIdent)) )
      {
         PRN_ERR_USR_NL( MEI_CEOC, MEI_DRV_PRN_LEVEL_ERR,
               ("MEI_CEOC[%02d]: ERROR - recv C-EOC Read - invalid size %d (max %d + 2)!" MEI_DRV_CRLF,
                MEI_DRV_LINENUM_GET(pMeiDev),
                pVrxEocFrame->length_byte, MEI_CEOC_RAW_EOC_DATA_SIZE_BYTE));
         pVrxEocFrame->length_byte = 0;
         pCEocFrameBuf->transferedSize_byte = 0;
         return IFX_ERROR;
      }

      pCEocFrameBuf->transferedSize_byte =
            sizeof(pVrxEocFrame->length_byte) + sizeof(pVrxEocFrame->protIdent);

      pCEocFrameBuf->frameSize_byte      = pVrxEocFrame->length_byte
                                           + sizeof(pVrxEocFrame->length_byte);
      /* padding */
      if (pCEocFrameBuf->frameSize_byte & 0x1)
         pCEocFrameBuf->frameSize_byte++;

      pEocDataSrc        = (IFX_uint8_t *)&pCatMsgEvt->Data[2];
      nextTransfer_bytes = ( (pCatMsgEvt->Length << CMV_MSG_BIT_SIZE_16BIT)
                             - (2 * sizeof(unsigned short)) );
   }
   else
   {
      /* Second segement */
      if (pCEocFrameBuf->transferedSize_byte == 0)
      {
         PRN_ERR_USR_NL( MEI_CEOC, MEI_DRV_PRN_LEVEL_ERR,
               ("MEI_CEOC[%02d]: ERR - recv C-EOC Read - discard, missing first seg !" MEI_DRV_CRLF,
                MEI_DRV_LINENUM_GET(pMeiDev)));

         return IFX_ERROR;
      }

      if (pCEocFrameBuf->transferedSize_byte != ((IFX_uint32_t)pCatMsgEvt->Index << CMV_MSG_BIT_SIZE_16BIT))
      {
         PRN_DBG_USR( MEI_CEOC, MEI_DRV_PRN_LEVEL_HIGH, MEI_DRV_LINENUM_GET(pMeiDev),
               ("MEI_CEOC[%02d]: ERR - recv C-EOC Read - discard, data lost !" MEI_DRV_CRLF,
                MEI_DRV_LINENUM_GET(pMeiDev)));

         return IFX_ERROR;
      }

      pEocDataSrc        = (IFX_uint8_t *)&pCatMsgEvt->Data[0];
      nextTransfer_bytes = (pCatMsgEvt->Length << CMV_MSG_BIT_SIZE_16BIT);
   }

   pEocDataDest  = (IFX_uint8_t *)pVrxEocFrame->cEocRawData.d_8;
   pEocDataDest += (pCEocFrameBuf->transferedSize_byte -
                     (sizeof(pVrxEocFrame->length_byte) + sizeof(pVrxEocFrame->protIdent)));

   if ( (pCEocFrameBuf->transferedSize_byte + nextTransfer_bytes) > pCEocFrameBuf->frameSize_byte )
   {
      PRN_ERR_USR_NL( MEI_CEOC, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_CEOC[%02d]: ERR - recv C-EOC Read - buffer overflow !" MEI_DRV_CRLF,
             MEI_DRV_LINENUM_GET(pMeiDev)));

      return IFX_ERROR;
   }

   memcpy(pEocDataDest, pEocDataSrc, nextTransfer_bytes);
   pCEocFrameBuf->transferedSize_byte += nextTransfer_bytes;

   return IFX_SUCCESS;
}

/**
   Write the VRX CMD_CLEAREOC_CONFIGURE command.
   Enable / Disable the Clear EOC feature and configure the indication handling.

\param
   pMeiDynCntrl    points to the dynamic control struct.
\param
   pCEocDevCntrl     points to the Clear EOC control struct.

\return
   0 (IFX_SUCCESS) if success
   negative value in case of error
*/
IFX_int32_t MEI_CEOC_CMD_ClearEOC_Configure(
                              MEI_DYN_CNTRL_T      *pMeiDynCntrl,
                              MEI_CEOC_DEV_CNTRL_T *pCEocDevCntrl)
{
   IFX_int32_t          modemMsgSize, modemPaylSize;
   MEI_DYN_CMD_DATA_T     *pDynCmd   = pMeiDynCntrl->pInstDynCmd;
   CMV_STD_MESSAGE_T        *pModemMsg = (CMV_STD_MESSAGE_T *)pDynCmd->cmdWrBuf.pBuffer;
   CMD_ClearEOC_Configure_t *pCatMsg   = (CMD_ClearEOC_Configure_t *)&pModemMsg->header.index;

   if ( MEI_DevCntlMsgAvailable( pMeiDynCntrl,
                                   (IFX_uint32_t)MEI_DRV_CMD_CLEAREOC_CONFIGURE)
        != IFX_SUCCESS)
   {
      return -e_MEI_ERR_REM_INVAL_LINE_STATE;
   }

   PRN_DBG_USR( MEI_CEOC, MEI_DRV_PRN_LEVEL_NORMAL, MEI_DRV_DYN_LINENUM_GET(pMeiDynCntrl),
         ("MEI_CEOC[%02d - %02d]: device C-EOC control" MEI_DRV_CRLF
          , MEI_DRV_DYN_LINENUM_GET(pMeiDynCntrl), pMeiDynCntrl->openInstance));

   /* lock the the current instance */
   MEI_DRVOS_SemaphoreLock(&pMeiDynCntrl->pInstanceRWlock);

   /* ===============================================
      check if the instance specific buffer is free
      =============================================== */
   MEI_WaitForInstance(pMeiDynCntrl, pMeiDynCntrl->pMeiDev, pDynCmd);

   /* ===============================================
      Setup CMD_CLEAREOC_CONFIGURE header
      =============================================== */
   modemPaylSize = sizeof(CMD_ClearEOC_Configure_t) - (2 * sizeof(IFX_uint16_t));
   modemMsgSize  = CMV_HEADER_8BIT_SIZE + modemPaylSize;
   memset(pModemMsg, 0x00, modemMsgSize);

   pModemMsg->header.mbxCode =  MEI_MBOX_CODE_MSG_WRITE;
   MEI_MsgId2CmvHeader( pMeiDynCntrl->pMeiDev, pModemMsg,
                          MEI_DRV_MODEM_MSG_ID_GET(MEI_DRV_CMD_CLEAREOC_CONFIGURE) );
    /* size field contains number of 16 bit payload elements of the message */
   P_CMV_MSGHDR_PAYLOAD_SIZE_SET( pModemMsg,
                                  ((IFX_uint32_t)modemPaylSize >> CMV_MSG_BIT_SIZE_16BIT) );

   /* ===============================================
      Setup CMD_CLEAREOC_CONFIGURE payload
      =============================================== */
   pCatMsg->Index          = 0;
   pCatMsg->Length         = modemPaylSize >> CMV_MSG_BIT_SIZE_16BIT;

   if (pCEocDevCntrl->bEnEvtRxData)
      pCatMsg->P02_MsgControl |= CMD_CLEAREOC_CONFIGURE_P02_RX_EVT_DATA;

   if (pCEocDevCntrl->bEnEvtRxStatus)
      pCatMsg->P02_MsgControl |= CMD_CLEAREOC_CONFIGURE_P02_RX_EVT_STAT;

   if (pCEocDevCntrl->bEnEvtTxStatus)
      pCatMsg->P02_MsgControl |= CMD_CLEAREOC_CONFIGURE_P02_TX_EVT_STAT;

   pCatMsg->P02_MsgControl &= ~(CMD_CLEAREOC_CONFIGURE_P02_RES00);

   /* ===============================================
      Send CMD_CLEAREOC_CONFIGURE
      =============================================== */
   if ( (MEI_WriteMsgAndCheck(
               pMeiDynCntrl, pDynCmd,
               (MEI_MEI_MAILBOX_T *)pModemMsg,
               modemMsgSize) ) != IFX_SUCCESS )
   {
      MEI_DRVOS_SemaphoreUnlock(&pMeiDynCntrl->pInstanceRWlock);

      PRN_ERR_USR_NL( MEI_CEOC, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_CEOC[%02d - %02d]: ERROR - set C-EOC control failed!" MEI_DRV_CRLF
             , MEI_DRV_DYN_LINENUM_GET(pMeiDynCntrl), pMeiDynCntrl->openInstance));

      return IFX_ERROR;
   }

   MEI_DRVOS_SemaphoreUnlock(&pMeiDynCntrl->pInstanceRWlock);

   return IFX_SUCCESS;
}



/**
   Write the VRX CMD_CLEAREOCSTATUSGET command.
   Request the Clear EOC status form the device.

\param
   pMeiDynCntrl    points to the dynamic control struct.
\param
   pCEocDevCntrl     points to the Clear EOC control struct.
\param
   pTxOpStatus       returns the new TX operation state out form the response.
                     TX Op = idle     --> OP_NONE
                           = progress --> OP_IN_PROG
                     else             --> OP_ERR
\param
   pRxOpStatus      returns the new RX state out form the response.
                     RX Op = idle     --> OP_NONE
                           = progress --> OP_IN_PROG
                           = done     --> OP_DONE
                     else             --> OP_ERR
\return
   0 (IFX_SUCCESS) if success
   negative value in case of error
*/
IFX_int32_t MEI_CEOC_CMD_ClearEOCStatusGet(
                              MEI_DYN_CNTRL_T       *pMeiDynCntrl,
                              MEI_CEOC_DEV_CNTRL_T  *pCEocDevCntrl)
{
   IFX_int32_t             modemMsgSize, modemPaylSize;
   MEI_DYN_CMD_DATA_T    *pDynCmd    = pMeiDynCntrl->pInstDynCmd;
   CMV_STD_MESSAGE_T       *pModemMsg  = (CMV_STD_MESSAGE_T *)pDynCmd->cmdWrBuf.pBuffer;
   CMV_STD_MESSAGE_T       *pModemAck  = (CMV_STD_MESSAGE_T *)pDynCmd->cmdAckCntrl.recvDataBuf_s.pBuffer;
   CMD_ClearEOCStatusGet_t *pCatMsgCmd = (CMD_ClearEOCStatusGet_t *)&pModemMsg->header.index;
   ACK_ClearEOCStatusGet_t *pCatMsgAck = (ACK_ClearEOCStatusGet_t *)&pModemAck->header.index;

   if ( MEI_DevCntlMsgAvailable( pMeiDynCntrl,
                                   (IFX_uint32_t)MEI_DRV_CMD_CLEAREOCSTATUSGET)
        != IFX_SUCCESS)
   {
      return -e_MEI_ERR_REM_INVAL_LINE_STATE;
   }

   PRN_DBG_USR( MEI_CEOC, MEI_DRV_PRN_LEVEL_NORMAL, MEI_DRV_DYN_LINENUM_GET(pMeiDynCntrl),
         ("MEI_CEOC[%02d - %02d]: send C-EOC Stat Get" MEI_DRV_CRLF
          , MEI_DRV_DYN_LINENUM_GET(pMeiDynCntrl), pMeiDynCntrl->openInstance));

   modemPaylSize = sizeof(CMD_ClearEOCStatusGet_t) - (2 * sizeof(IFX_uint16_t));
   modemMsgSize  = CMV_HEADER_8BIT_SIZE + modemPaylSize;

   /* ===============================================
      lock and check if the instance specific buffer is free
      =============================================== */
   MEI_DRVOS_SemaphoreLock(&pMeiDynCntrl->pInstanceRWlock);
   MEI_WaitForInstance(pMeiDynCntrl, pMeiDynCntrl->pMeiDev, pDynCmd);

   /* ===============================================
      Setup CMD_CLEAREOCSTATUSGET message
      =============================================== */
   memset(pModemMsg, 0x00, modemMsgSize);
   pModemMsg->header.mbxCode =  MEI_MBOX_CODE_MSG_WRITE;
   MEI_MsgId2CmvHeader( pMeiDynCntrl->pMeiDev, pModemMsg,
                          MEI_DRV_MODEM_MSG_ID_GET(MEI_DRV_CMD_CLEAREOCSTATUSGET) );

   /* size field contains number of 16 bit payload elements of the message */
   P_CMV_MSGHDR_PAYLOAD_SIZE_SET( pModemMsg,
                                  ((IFX_uint32_t)modemPaylSize >> CMV_MSG_BIT_SIZE_16BIT) );

   pCatMsgCmd->Index  = 0;
   pCatMsgCmd->Length = (sizeof(ACK_ClearEOCStatusGet_t) - (2 * sizeof(IFX_uint16_t))) >> CMV_MSG_BIT_SIZE_16BIT;

   /* ===============================================
      Send CMD_CLEAREOCSTATUSGET
      =============================================== */
   if ( (MEI_WriteMsgAndCheck(
               pMeiDynCntrl, pDynCmd,
               (MEI_MEI_MAILBOX_T *)pModemMsg,
               modemMsgSize) ) != IFX_SUCCESS )
   {
      MEI_DRVOS_SemaphoreUnlock(&pMeiDynCntrl->pInstanceRWlock);

      PRN_ERR_USR_NL( MEI_CEOC, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_CEOC[%02d - %02d]: ERROR - send C-EOC Stat Get failed!" MEI_DRV_CRLF
             , MEI_DRV_DYN_LINENUM_GET(pMeiDynCntrl), pMeiDynCntrl->openInstance));

      return IFX_ERROR;
   }

   /* ===============================================
      Success CMD_CLEAREOCSTATUSGET - get the data
      =============================================== */

   PRN_DBG_USR( MEI_CEOC, MEI_DRV_PRN_LEVEL_NORMAL, MEI_DRV_DYN_LINENUM_GET(pMeiDynCntrl),
         ("MEI_CEOC[%02d - %02d]: ACK C-EOC Stat Get - TX Stat = 0x%X, RX Stat = 0x%X" MEI_DRV_CRLF
          , MEI_DRV_DYN_LINENUM_GET(pMeiDynCntrl), pMeiDynCntrl->openInstance
          , pCatMsgAck->P02_TxStat, pCatMsgAck->P03_RxStat));

   if (!pCEocDevCntrl->bEnEvtTxStatus)
   {
      switch (pCatMsgAck->P02_TxStat)
      {
         case ACK_ClearEOCStatusGet_IDLE:
            MEI_CEOC_TX_DEV_BUF_STATE_SET(pCEocDevCntrl, eMEI_CEOC_TX_DEV_BUF_STATE_IDLE);
            break;
         case ACK_ClearEOCStatusGet_TXPROG:
            MEI_CEOC_TX_DEV_BUF_STATE_SET(pCEocDevCntrl, eMEI_CEOC_TX_DEV_BUF_STATE_IN_PROGRESS);
            break;
         case ACK_ClearEOCStatusGet_Reserved:
         case ACK_ClearEOCStatusGet_TXERR:
         default:
            MEI_CEOC_TX_DEV_BUF_STATE_SET(pCEocDevCntrl, eMEI_CEOC_TX_DEV_BUF_STATE_ERROR);
            break;
      }
   }

   if (!pCEocDevCntrl->bEnEvtRxStatus)
   {
      switch (pCatMsgAck->P03_RxStat)
      {
         case ACK_ClearEOCStatusGet_IDLE:
            MEI_CEOC_RX_DEV_BUF_STATE_SET(pCEocDevCntrl, eMEI_CEOC_RX_DEV_BUF_STATE_IDLE);
            break;
         case ACK_ClearEOCStatusGet_RXPROG:
            MEI_CEOC_RX_DEV_BUF_STATE_SET(pCEocDevCntrl, eMEI_CEOC_RX_DEV_BUF_STATE_IN_PROGRESS);
            break;
         case ACK_ClearEOCStatusGet_RXDONE:
            MEI_CEOC_RX_DEV_BUF_STATE_SET(pCEocDevCntrl, eMEI_CEOC_RX_DEV_BUF_STATE_DONE);
            break;
         case ACK_ClearEOCStatusGet_RXERR:
         default:
            MEI_CEOC_RX_DEV_BUF_STATE_SET(pCEocDevCntrl, eMEI_CEOC_RX_DEV_BUF_STATE_ERROR);
            break;
      }
   }

   MEI_DRVOS_SemaphoreUnlock(&pMeiDynCntrl->pInstanceRWlock);

   return IFX_SUCCESS;
}


/**
   Write the VRX CMD_CLEAREOC_TXTRIGGER command.
   Trigger the underlaying FW to insert the previous written EOC frame to the line.

\param
   pMeiDynCntrl    points to the dynamic control struct.
\param
   pCEocDevCntrl     points to the Clear EOC control struct.

\return
   0 (IFX_SUCCESS) if success
   negative value in case of error
*/
IFX_int32_t MEI_CEOC_CMD_ClearEOC_TxTrigger(
                              MEI_DYN_CNTRL_T      *pMeiDynCntrl,
                              MEI_CEOC_DEV_CNTRL_T *pCEocDevCntrl)
{
   IFX_int32_t              modemMsgSize, modemPaylSize;
   MEI_DYN_CMD_DATA_T     *pDynCmd   = pMeiDynCntrl->pInstDynCmd;
   CMV_STD_MESSAGE_T        *pModemMsg = (CMV_STD_MESSAGE_T *)pDynCmd->cmdWrBuf.pBuffer;
   CMD_ClearEOC_TxTrigger_t *pCatMsg   = (CMD_ClearEOC_TxTrigger_t *)&pModemMsg->header.index;

   if ( MEI_DevCntlMsgAvailable( pMeiDynCntrl,
                                   (IFX_uint32_t)MEI_DRV_CMD_CLEAREOC_TXTRIGGER)
        != IFX_SUCCESS)
   {
      return -e_MEI_ERR_REM_INVAL_LINE_STATE;
   }

   PRN_DBG_USR( MEI_CEOC, MEI_DRV_PRN_LEVEL_NORMAL, MEI_DRV_DYN_LINENUM_GET(pMeiDynCntrl),
         ("MEI_CEOC[%02d - %02d]: send C-EOC trigger" MEI_DRV_CRLF
          , MEI_DRV_DYN_LINENUM_GET(pMeiDynCntrl), pMeiDynCntrl->openInstance));

   /* lock the the current instance */
   MEI_DRVOS_SemaphoreLock(&pMeiDynCntrl->pInstanceRWlock);

   /* ===============================================
      check if the instance specific buffer is free
      =============================================== */
   MEI_WaitForInstance(pMeiDynCntrl, pMeiDynCntrl->pMeiDev, pDynCmd);

   /* ===============================================
      Setup CMD_CLEAREOC_TXTRIGGER header
      =============================================== */
   modemPaylSize = sizeof(CMD_ClearEOC_TxTrigger_t) - (2 * sizeof(IFX_uint16_t));
   modemMsgSize  = CMV_HEADER_8BIT_SIZE + modemPaylSize;
   memset(pModemMsg, 0x00, modemMsgSize);

   pModemMsg->header.mbxCode =  MEI_MBOX_CODE_MSG_WRITE;
   MEI_MsgId2CmvHeader( pMeiDynCntrl->pMeiDev, pModemMsg,
                          MEI_DRV_MODEM_MSG_ID_GET(MEI_DRV_CMD_CLEAREOC_TXTRIGGER) );
    /* size field contains number of 16 bit payload elements of the message */
   P_CMV_MSGHDR_PAYLOAD_SIZE_SET( pModemMsg,
                                  ((IFX_uint32_t)modemPaylSize >> CMV_MSG_BIT_SIZE_16BIT) );

   /* ===============================================
      Setup CMD_CLEAREOC_TXTRIGGER payload
      =============================================== */
   pCatMsg->Index          = 0;
   pCatMsg->Length         = modemPaylSize >> CMV_MSG_BIT_SIZE_16BIT;
   pCatMsg->P02_TxTrigger  = CMD_CLEAREOC_TXTRIGGER_P02_TX_TRIGGER;

   /* ===============================================
      Send CMD_CLEAREOC_TXTRIGGER
      =============================================== */
   if ( (MEI_WriteMsgAndCheck(
               pMeiDynCntrl, pDynCmd,
               (MEI_MEI_MAILBOX_T *)pModemMsg,
               modemMsgSize) ) != IFX_SUCCESS )
   {
      MEI_DRVOS_SemaphoreUnlock(&pMeiDynCntrl->pInstanceRWlock);

      PRN_ERR_USR_NL( MEI_CEOC, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_CEOC[%02d - %02d]: ERROR - set C-EOC trigger failed!" MEI_DRV_CRLF
             , MEI_DRV_DYN_LINENUM_GET(pMeiDynCntrl), pMeiDynCntrl->openInstance));

      return IFX_ERROR;
   }

   MEI_CEOC_STAT_INC_SEND_TX_TRIGGER(pCEocDevCntrl);

   MEI_DRVOS_SemaphoreUnlock(&pMeiDynCntrl->pInstanceRWlock);

   return IFX_SUCCESS;
}

/**
   Write the VRX CMD_CLEAREOC_WRITE command.
   Write a EOC frame segment to the device.

\param
   pMeiDynCntrl    points to the dynamic control struct.
\param
   pCEocDevCntrl     points to the Clear EOC control struct.

\return
   0 (IFX_SUCCESS) if success
   negative value in case of error
*/
IFX_int32_t MEI_CEOC_CMD_ClearEOC_Write(
                              MEI_DYN_CNTRL_T         *pMeiDynCntrl,
                              MEI_CEOC_DEV_CNTRL_T    *pCEocDevCntrl,
                              MEI_CEOC_FRAME_BUFFER_T *pCEocFrameBuf)
{
   IFX_int32_t          modemMsgSize, modemPaylSize, nextTransfer_bytes;
   MEI_DYN_CMD_DATA_T *pDynCmd   = pMeiDynCntrl->pInstDynCmd;
   CMV_STD_MESSAGE_T    *pModemMsg = (CMV_STD_MESSAGE_T *)pDynCmd->cmdWrBuf.pBuffer;
   CMD_ClearEOC_Write_t *pCatMsg   = (CMD_ClearEOC_Write_t *)&pModemMsg->header.index;
   IFX_uint8_t          *pEocDataSrc, *pEocDataDest;
   MEI_CEOC_MEI_EOC_FRAME_T *pVrxEocFrame = &pCEocFrameBuf->vrxEocFrame;

   if ( MEI_DevCntlMsgAvailable( pMeiDynCntrl,
                                   (IFX_uint32_t)MEI_DRV_CMD_CLEAREOC_WRITE)
        != IFX_SUCCESS)
   {
      return -e_MEI_ERR_REM_INVAL_LINE_STATE;
   }

   PRN_DBG_USR( MEI_CEOC, MEI_DRV_PRN_LEVEL_NORMAL, MEI_DRV_DYN_LINENUM_GET(pMeiDynCntrl),
         ("MEI_CEOC[%02d - %02d]: send C-EOC Write" MEI_DRV_CRLF
          , MEI_DRV_DYN_LINENUM_GET(pMeiDynCntrl), pMeiDynCntrl->openInstance));


   /* ===============================================
      check the given VRX frame and buffer
      =============================================== */
   if (pCEocFrameBuf->frameSize_byte == 0)
   {
      PRN_ERR_USR_NL( MEI_CEOC, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_CEOC[%02d - %02d]: Error send C-EOC Write - "
             "no data" MEI_DRV_CRLF,
             MEI_DRV_DYN_LINENUM_GET(pMeiDynCntrl), pMeiDynCntrl->openInstance));

      return IFX_ERROR;
   }

   if (pCEocFrameBuf->transferedSize_byte >= pCEocFrameBuf->frameSize_byte)
   {
      PRN_ERR_USR_NL( MEI_CEOC, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_CEOC[%02d - %02d]: Error send C-EOC Write - "
             "transfered bytes %d invalid" MEI_DRV_CRLF,
             MEI_DRV_DYN_LINENUM_GET(pMeiDynCntrl), pMeiDynCntrl->openInstance,
             pCEocFrameBuf->transferedSize_byte));

      return IFX_ERROR;
   }

   modemPaylSize      = sizeof(CMD_ClearEOC_Write_t) - (2 * sizeof(IFX_uint16_t));
   nextTransfer_bytes = pCEocFrameBuf->frameSize_byte - pCEocFrameBuf->transferedSize_byte;
   nextTransfer_bytes = (nextTransfer_bytes > modemPaylSize) ? modemPaylSize : nextTransfer_bytes;
   modemPaylSize      = nextTransfer_bytes;
   modemMsgSize       = CMV_HEADER_8BIT_SIZE + modemPaylSize;

   /* lock the the current instance */
   MEI_DRVOS_SemaphoreLock(&pMeiDynCntrl->pInstanceRWlock);

   /* ===============================================
      check if the instance specific buffer is free
      =============================================== */
   MEI_WaitForInstance(pMeiDynCntrl, pMeiDynCntrl->pMeiDev, pDynCmd);

   /* ===============================================
      Setup CMD_CLEAREOC_WRITE message
      =============================================== */
   memset(pModemMsg, 0x00, modemMsgSize);
   pModemMsg->header.mbxCode =  MEI_MBOX_CODE_MSG_WRITE;
   MEI_MsgId2CmvHeader( pMeiDynCntrl->pMeiDev, pModemMsg,
                          MEI_DRV_MODEM_MSG_ID_GET(MEI_DRV_CMD_CLEAREOC_WRITE) );
   /* size field contains number of 16 bit payload elements of the message */
   P_CMV_MSGHDR_PAYLOAD_SIZE_SET( pModemMsg,
                                  ((IFX_uint32_t)modemPaylSize >> CMV_MSG_BIT_SIZE_16BIT) );

   if (pCEocFrameBuf->transferedSize_byte == 0)
   {
      /* first segment */
      pCatMsg->Index          = 0;
      pCatMsg->Length         = modemPaylSize >> CMV_MSG_BIT_SIZE_16BIT;

      /* set Vrx EOC frame - length */
      pModemMsg->payload.params_16Bit[0] = (unsigned short)pVrxEocFrame->length_byte;
      /* set Vrx EOC frame - protocol ID */
      pModemMsg->payload.params_16Bit[1] = (unsigned short)pVrxEocFrame->protIdent;

      nextTransfer_bytes -= (2 * sizeof(unsigned short));
      pEocDataDest = (IFX_uint8_t *)&pCatMsg->Data[2 * sizeof(unsigned short)];
      pEocDataSrc  = pVrxEocFrame->cEocRawData.d_8;

   }
   else
   {
      /* second segment */
      pCatMsg->Index          = pCEocFrameBuf->transferedSize_byte >> CMV_MSG_BIT_SIZE_16BIT;
      pCatMsg->Length         = modemPaylSize >> CMV_MSG_BIT_SIZE_16BIT;

      pEocDataDest = (IFX_uint8_t *)pCatMsg->Data;
      pEocDataSrc  = &pVrxEocFrame->cEocRawData.d_8[(pCEocFrameBuf->transferedSize_byte) - (2 * sizeof(unsigned short))];
   }

   /* set the EOC data */
   memcpy(pEocDataDest, pEocDataSrc, nextTransfer_bytes);

   /* ===============================================
      Send CMD_CLEAREOC_WRITE
      =============================================== */
   if ( (MEI_WriteMsgAndCheck(
               pMeiDynCntrl, pDynCmd,
               (MEI_MEI_MAILBOX_T *)pModemMsg,
               modemMsgSize) ) != IFX_SUCCESS )
   {
      MEI_CEOC_STAT_INC_WR_MSG_ERR_CNT(pCEocDevCntrl);
      MEI_DRVOS_SemaphoreUnlock(&pMeiDynCntrl->pInstanceRWlock);

      PRN_ERR_USR_NL( MEI_CEOC, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_CEOC[%02d - %02d]: ERROR - send C-EOC Write failed!" MEI_DRV_CRLF
             , MEI_DRV_DYN_LINENUM_GET(pMeiDynCntrl), pMeiDynCntrl->openInstance));

      return IFX_ERROR;
   }

   MEI_CEOC_STAT_INC_WR_MSG_CNT(pCEocDevCntrl);
   MEI_DRVOS_SemaphoreUnlock(&pMeiDynCntrl->pInstanceRWlock);

   /* ===============================================
      Success CMD_CLEAREOC_WRITE update buffer info
      =============================================== */
   pCEocFrameBuf->transferedSize_byte += modemPaylSize;

   return IFX_SUCCESS;
}

/**
   Write the VRX CMD_CLEAREOC_READ command.
   Read a EOC frame segment from the device.

\param
   pMeiDynCntrl    points to the dynamic control struct.
\param
   pCEocDevCntrl     points to the Clear EOC control struct.

\return
   0 (IFX_SUCCESS) if success
   negative value in case of error
*/
IFX_int32_t MEI_CEOC_CMD_ClearEOC_Read(
                              MEI_DYN_CNTRL_T         *pMeiDynCntrl,
                              MEI_CEOC_DEV_CNTRL_T    *pCEocDevCntrl,
                              MEI_CEOC_FRAME_BUFFER_T *pCEocFrameBuf)
{
   IFX_int32_t          modemMsgSize, modemPaylSize;
   MEI_DYN_CMD_DATA_T *pDynCmd    = pMeiDynCntrl->pInstDynCmd;
   CMV_STD_MESSAGE_T    *pModemMsg  = (CMV_STD_MESSAGE_T *)pDynCmd->cmdWrBuf.pBuffer;
   CMV_STD_MESSAGE_T    *pModemAck  = (CMV_STD_MESSAGE_T *)pDynCmd->cmdAckCntrl.recvDataBuf_s.pBuffer;
   CMD_ClearEOC_Read_t  *pCatMsgCmd = (CMD_ClearEOC_Read_t *)&pModemMsg->header.index;
   ACK_ClearEOC_Read_t  *pCatMsgAck = (ACK_ClearEOC_Read_t *)&pModemAck->header.index;

   if ( MEI_DevCntlMsgAvailable( pMeiDynCntrl,
                                   (IFX_uint32_t)MEI_DRV_CMD_CLEAREOC_READ)
        != IFX_SUCCESS)
   {
      return -e_MEI_ERR_REM_INVAL_LINE_STATE;
   }

   PRN_DBG_USR( MEI_CEOC, MEI_DRV_PRN_LEVEL_NORMAL, MEI_DRV_DYN_LINENUM_GET(pMeiDynCntrl),
         ("MEI_CEOC[%02d - %02d]: send C-EOC Read" MEI_DRV_CRLF
          , MEI_DRV_DYN_LINENUM_GET(pMeiDynCntrl), pMeiDynCntrl->openInstance));


   /* ===============================================
      check the given VRX frame and buffer
      =============================================== */
   if (     (pCEocFrameBuf->frameSize_byte == 0)
         || (pCEocFrameBuf->transferedSize_byte >= pCEocFrameBuf->frameSize_byte) )
   {
      /* frame size set and frame already received */
      PRN_ERR_USR_NL( MEI_CEOC, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_CEOC[%02d - %02d]: Error send C-EOC Read - no buffer space" MEI_DRV_CRLF,
              MEI_DRV_DYN_LINENUM_GET(pMeiDynCntrl), pMeiDynCntrl->openInstance));

      return IFX_ERROR;
   }

   modemPaylSize = sizeof(CMD_ClearEOC_Read_t) - (2 * sizeof(IFX_uint16_t));
   modemMsgSize  = CMV_HEADER_8BIT_SIZE + modemPaylSize;

   /* ===============================================
      lock and check if the instance specific buffer is free
      =============================================== */
   MEI_DRVOS_SemaphoreLock(&pMeiDynCntrl->pInstanceRWlock);
   MEI_WaitForInstance(pMeiDynCntrl, pMeiDynCntrl->pMeiDev, pDynCmd);

   /* ===============================================
      Setup CMD_CLEAREOC_READ message
      =============================================== */
   memset(pModemMsg, 0x00, modemMsgSize);
   pModemMsg->header.mbxCode =  MEI_MBOX_CODE_MSG_WRITE;
   MEI_MsgId2CmvHeader( pMeiDynCntrl->pMeiDev, pModemMsg,
                          MEI_DRV_MODEM_MSG_ID_GET(MEI_DRV_CMD_CLEAREOC_READ) );

   /* size field contains number of 16 bit payload elements of the message */
   P_CMV_MSGHDR_PAYLOAD_SIZE_SET( pModemMsg,
                                  ((IFX_uint32_t)modemPaylSize >> CMV_MSG_BIT_SIZE_16BIT) );

   if (pCEocFrameBuf->transferedSize_byte == 0)
   {
      /* first frame segment */
      pCatMsgCmd->Index  = 0;
      /* request max frame segment size - thanks for the great concept */
      pCatMsgCmd->Length = ((sizeof(ACK_ClearEOC_Read_t) - (2 * sizeof(IFX_uint16_t))) >> CMV_MSG_BIT_SIZE_16BIT);
   }
   else
   {
      pCatMsgCmd->Index  = (pCEocFrameBuf->transferedSize_byte >> CMV_MSG_BIT_SIZE_16BIT);
      pCatMsgCmd->Length = (pCEocFrameBuf->frameSize_byte - pCEocFrameBuf->transferedSize_byte) >> CMV_MSG_BIT_SIZE_16BIT;
   }

   /* ===============================================
      Send CMD_CLEAREOC_READ
      =============================================== */
   if ( (MEI_WriteMsgAndCheck(
               pMeiDynCntrl, pDynCmd,
               (MEI_MEI_MAILBOX_T *)pModemMsg,
               modemMsgSize) ) != IFX_SUCCESS )
   {
      MEI_CEOC_STAT_INC_RD_MSG_ERR_CNT(pCEocDevCntrl);
      MEI_DRVOS_SemaphoreUnlock(&pMeiDynCntrl->pInstanceRWlock);

      PRN_ERR_USR_NL( MEI_CEOC, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_CEOC[%02d - %02d]: ERROR - send C-EOC Read failed!" MEI_DRV_CRLF
             , MEI_DRV_DYN_LINENUM_GET(pMeiDynCntrl), pMeiDynCntrl->openInstance));

      return IFX_ERROR;
   }

   /* ===============================================
      Success CMD_CLEAREOC_READ - get the data
      =============================================== */
   MEI_CEOC_STAT_INC_RD_MSG_CNT(pCEocDevCntrl);

   if ( MEI_CEOC_DataGet(
            pMeiDynCntrl->pMeiDev, pCEocDevCntrl,
            pCEocFrameBuf, (EVT_ClearEOC_Read_t *)pCatMsgAck) != IFX_SUCCESS)
   {
      MEI_CEOC_STAT_INC_RD_MSG_ERR_CNT(pCEocDevCntrl);
      MEI_DRVOS_SemaphoreUnlock(&pMeiDynCntrl->pInstanceRWlock);

      return IFX_ERROR;
   }

   MEI_DRVOS_SemaphoreUnlock(&pMeiDynCntrl->pInstanceRWlock);
   return IFX_SUCCESS;
}

/**
   Autonomous message -  EVT_CLEAREOC_READ received.
   Handle a upcoming Clear EOC data message form the device.

\param
   pMeiDynCntrl    points to the dynamic control struct.
\param
   pCEocDevCntrl     points to the Clear EOC control struct.

\return
   0 (IFX_SUCCESS) if success
   negative value in case of error
*/
IFX_int32_t MEI_CEOC_doEVT_ClearEOC_Read(
                              MEI_DEV_T                *pMeiDev,
                              MEI_CEOC_DEV_CNTRL_T     *pCEocDevCntrl,
                              CMV_STD_MESSAGE_T          *pModemMsg,
                              MEI_CEOC_FRAME_BUFFER_T  *pCEocFrameBuf)
{
   EVT_ClearEOC_Read_t  *pCatMsgEvt = (EVT_ClearEOC_Read_t *)&pModemMsg->header.index;

   MEI_CEOC_STAT_INC_EVT_RD_MSG_CNT(pCEocDevCntrl);

   if ( MEI_CEOC_DataGet(
            pMeiDev, pCEocDevCntrl, pCEocFrameBuf, pCatMsgEvt) != IFX_SUCCESS)
   {
      MEI_CEOC_STAT_INC_EVT_RD_MSG_ERR_CNT(pCEocDevCntrl);

      return IFX_ERROR;
   }

   return IFX_SUCCESS;
}

/**
   Autonomous message -  EVT_CLEAREOCSTATUSGET received.
   Handle a upcoming Clear EOC status message form the device.

\param
   pMeiDynCntrl    points to the dynamic control struct.
\param
   pCEocDevCntrl     points to the Clear EOC control struct.

\return
   0 (IFX_SUCCESS) if success
   negative value in case of error
*/
IFX_int32_t MEI_CEOC_doEVT_ClearEOCStatusGet(
                              MEI_DEV_T                *pMeiDev,
                              MEI_CEOC_DEV_CNTRL_T     *pCEocDevCntrl,
                              CMV_STD_MESSAGE_T          *pModemMsg)
{
   IFX_int32_t             retVal = IFX_SUCCESS;
   EVT_ClearEOCStatusGet_t *pCatMsgEvt = (EVT_ClearEOCStatusGet_t *)&pModemMsg->header.index;

   if (pCEocDevCntrl->bEnEvtTxStatus)
   {
      switch (pCatMsgEvt->P02_TxStat)
      {
         case EVT_ClearEOCStatusGet_IDLE:
            MEI_CEOC_TX_DEV_BUF_STATE_SET(pCEocDevCntrl, eMEI_CEOC_TX_DEV_BUF_STATE_IDLE);
            break;
         case EVT_ClearEOCStatusGet_TXPROG:
            MEI_CEOC_TX_DEV_BUF_STATE_SET(pCEocDevCntrl, eMEI_CEOC_TX_DEV_BUF_STATE_IN_PROGRESS);
            break;
         case EVT_ClearEOCStatusGet_TXDONE:
            /* transmit done --> set buffer to idle */
            MEI_CEOC_TX_DEV_BUF_STATE_SET(pCEocDevCntrl, eMEI_CEOC_TX_DEV_BUF_STATE_IDLE);
            break;
         case EVT_ClearEOCStatusGet_TXERR:
         default:
            MEI_CEOC_TX_DEV_BUF_STATE_SET(pCEocDevCntrl, eMEI_CEOC_TX_DEV_BUF_STATE_ERROR);
            MEI_CEOC_STAT_INC_TX_STAT_ERR(pCEocDevCntrl);
            break;
      }
   }

   if (pCEocDevCntrl->bEnEvtRxStatus)
   {
      switch (pCatMsgEvt->P03_RxStat)
      {
         case EVT_ClearEOCStatusGet_IDLE:
            MEI_CEOC_RX_DEV_BUF_STATE_SET(pCEocDevCntrl, eMEI_CEOC_RX_DEV_BUF_STATE_IDLE);
            break;
         case EVT_ClearEOCStatusGet_RXPROG:
            MEI_CEOC_RX_DEV_BUF_STATE_SET(pCEocDevCntrl, eMEI_CEOC_RX_DEV_BUF_STATE_IN_PROGRESS);
            break;
         case EVT_ClearEOCStatusGet_RXDONE:
            MEI_CEOC_RX_DEV_BUF_STATE_SET(pCEocDevCntrl, eMEI_CEOC_RX_DEV_BUF_STATE_DONE);
            break;
         case EVT_ClearEOCStatusGet_RXERR:
         default:
            MEI_CEOC_RX_DEV_BUF_STATE_SET(pCEocDevCntrl, eMEI_CEOC_RX_DEV_BUF_STATE_ERROR);
            MEI_CEOC_STAT_INC_RX_STAT_ERR(pCEocDevCntrl);
            break;
      }
   }

   return retVal;
}

#endif   /* #if (MEI_DRV_CLEAR_EOC_ENABLE == 1) */

