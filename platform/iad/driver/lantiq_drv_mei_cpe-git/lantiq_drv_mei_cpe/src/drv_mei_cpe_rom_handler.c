/******************************************************************************

                          Copyright (c) 2007-2015
                     Lantiq Beteiligungs-GmbH & Co. KG

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/* ==========================================================================
   Description : VRX Firmware Download function (ROM START).
   ========================================================================== */

/* ============================================================================
   Inlcudes
   ========================================================================= */

/* get at first the driver configuration */
#include "drv_mei_cpe_config.h"
#include "ifx_types.h"
#include "drv_mei_cpe_os.h"
#include "drv_mei_cpe_dbg.h"

#include "drv_mei_cpe_dbg_driver.h"

/* ==========================================================================
   Support firmware download (via Boot ROM Code)
   ========================================================================== */
#if (MEI_SUPPORT_ROM_CODE == 1)

/* get the device struct */
#include "drv_mei_cpe_interface.h"
#include "drv_mei_cpe_api.h"
#include "drv_mei_cpe_device_cntrl.h"
#include "drv_mei_cpe_msg_process.h"

/* get ROM handler definitions */
#include "drv_mei_cpe_rom_handler_if.h"
#include "drv_mei_cpe_rom_handler.h"

#if (MEI_SUPPORT_DL_DMA_CS == 1)
#include "drv_mei_cpe_download.h"
#endif

/* device boot code messages */
#include "user_if_vdsl2_boot_messages.h"

/* get the MEI interface */
#include "drv_mei_cpe_mei_interface.h"

/* ==========================================================================
   Macros
   ========================================================================== */

#if 1
#define MEI_TRACE_BOOT_MSG(pbootmsg, pdescr, dbglevel)\
                            MEI_TraceBootMsg(pbootmsg, pdescr, dbglevel)
#else
#define MEI_TRACE_BOOT_MSG(pbootmsg, pdescr dbglevel)
#endif

#if 1
#define MEI_LOG_BOOT_MSG(pbootmsg, pdescr, dbglevel)\
                            MEI_LogBootMsg(pbootmsg, pdescr, dbglevel)
#else
#define MEI_LOG_BOOT_MSG(pbootmsg, pdescr dbglevel)
#endif

/* ==========================================================================
   Global variables
   ========================================================================== */

/* VRX-Driver: ROM Code module - create print level variable */
MEI_DRV_PRN_USR_MODULE_CREATE(MEI_ROM, MEI_DRV_PRN_LEVEL_LOW);
MEI_DRV_PRN_INT_MODULE_CREATE(MEI_ROM, MEI_DRV_PRN_LEVEL_LOW);


/* Max time Wait: "VRX Event Init Done" */
IFX_int32_t MEI_MaxWaitInitDone_ms = 1000;

/* ==========================================================================
   Local variables
   ========================================================================== */


/* ==========================================================================
   Local function declarations
   ========================================================================== */

static IFX_void_t MEI_TraceBootMsg( MEI_Mailbox_t *pBootMsg,
                                          const char *pDescr,
                                          IFX_uint32_t dbgLevel);


static IFX_void_t MEI_LogBootMsg( MEI_Mailbox_t *pBootMsg,
                                        const char *pDescr,
                                        IFX_uint32_t dbgLevel);


static IFX_int32_t MEI_FDOnEvt_INITDONE( MEI_DEV_T *pMeiDev,
                                               MEI_Mailbox_t *pBootMsg);

#if (MEI_SUPPORT_DFE_GPA_ACCESS == 1)
static IFX_int32_t MEI_FDOnAck_GpaAccess( MEI_DEV_T *pMeiDev,
                                                MEI_Mailbox_t *pBootMsg);
#endif

static IFX_int32_t MEI_RomBootMsgHandler( MEI_DEV_T *pMeiDev,
                                                MEI_Mailbox_t *pBootMsg);

/* ==========================================================================
   Local function definitions
   ========================================================================== */

/**
   Only display the boot messages (trace)
\param
   pBootMsg: points to the bootmsg which has to be displayed
\param
   pDescr:   points to the additional description string.
\param

\attention
   - For VxWorks: do not use on int-level
*/
static IFX_void_t MEI_TraceBootMsg( MEI_Mailbox_t *pBootMsg,
                                          const char *pDescr,
                                          IFX_uint32_t dbgLevel)
{
   IFX_int32_t i;

   PRN_DBG_USR_NL( MEI_ROM, dbgLevel,
         (MEI_DRV_CRLF "--- Boot Message: %s --------------------" MEI_DRV_CRLF, pDescr) );

   PRN_DBG_USR_NL( MEI_ROM, dbgLevel,
         ("Hdr:  MB Id = 0x%04X, FctCode = 0x%04X, size = 0x%04X, msgId = 0x%04X" MEI_DRV_CRLF,
         pBootMsg->MbxCode, pBootMsg->FctCode, pBootMsg->Size, pBootMsg->MessageID) );


   PRN_DBG_USR_NL( MEI_ROM, dbgLevel, ("DATA: " MEI_DRV_CRLF));

   for (i=0; i < pBootMsg->Size; i++)
   {
      PRN_DBG_USR_NL( MEI_ROM, dbgLevel,
            ("Param[%d]: 0x%08X " MEI_DRV_CRLF, i, pBootMsg->Params[i]));

      if (i>7)
         break;
   }

   PRN_DBG_USR_NL( MEI_ROM, dbgLevel,
         ("---------------------------------------------------" MEI_DRV_CRLF MEI_DRV_CRLF) );

   return;
}

/**
   Only display the boot messages
\param
   pBootMsg: points to the bootmsg which has to be displayed
\param
   pDescr:   points to the additional description string.
\param

\attention
   - Called on int-level
*/
static IFX_void_t MEI_LogBootMsg( MEI_Mailbox_t *pBootMsg,
                                 const char *pDescr,
                                 IFX_uint32_t dbgLevel)
{
   IFX_int32_t i;

   PRN_DBG_INT_NL( MEI_ROM, dbgLevel,
       (MEI_DRV_CRLF "--- Boot Message: %s --------------------" MEI_DRV_CRLF, pDescr) );

   PRN_DBG_INT_NL( MEI_ROM, dbgLevel,
       ("Hdr:  MB Id = 0x%04X, FctCode = 0x%04X, size = 0x%04X, msgId = 0x%04X" MEI_DRV_CRLF,
         pBootMsg->MbxCode, pBootMsg->FctCode, pBootMsg->Size, pBootMsg->MessageID) );


   PRN_DBG_INT_NL( MEI_ROM, dbgLevel,
         ("DATA: " MEI_DRV_CRLF));

   for (i=0; i < pBootMsg->Size; i++)
   {
      PRN_DBG_INT_NL( MEI_ROM, dbgLevel,
            ("Param[%d]: 0x%08X " MEI_DRV_CRLF, i, pBootMsg->Params[i]));

      if (i>7)
         break;
   }

   PRN_DBG_INT_NL( MEI_ROM, dbgLevel,
       ("---------------------------------------------------" MEI_DRV_CRLF MEI_DRV_CRLF) );

   return;
}


/**
   EVENT - EVT_INITDONE has been received.
   NOTE:
      This is the first message send by the VRX ROM Handler to signal that
      the VRX is alive (and initialisation has been done)

\param
   pMeiDev  points to the device structure
\param
   pBootMsg points to the received boot code message.

\return
   IFX_SUCCESS: valid boot message.
   IFX_ERROR:   corrupted or valid boot message.

\remark
   IF the VRX sends this message it depends on the boot mode (only in
   boot modes with host interaction the VRX will send this message)

\attention
   - Called on int-level
*/
static IFX_int32_t MEI_FDOnEvt_INITDONE( MEI_DEV_T *pMeiDev,
                                               MEI_Mailbox_t *pBootMsg)
{
   EVT_InitDone_t *pInitDone = (EVT_InitDone_t *)&pBootMsg->Params;
   MEI_DRV_STATE_E tempState;

#if (MEI_DRV_POWERSAVING_ENABLED == 1)
   /* do device initialisation - power saving */
   MEI_DevSetup_PowerSave(pMeiDev);
#endif

#if (MEI_SUPPORT_VDSL2_ADSL_SWAP == 1)
   /* set the default FW mode (VDSL2 / ADLS) */
   if ( MEI_DevCfgFwModeSelect( pMeiDev,
                                  e_MEI_DEV_CFG_MODE_VDSL2)
         != IFX_SUCCESS )
   {
      return IFX_ERROR;
   }
#endif

   /*
      check current Driver state - still waiting for alive
      - set next state VRX alive
   */
   if (( MEI_DRV_STATE_GET(pMeiDev) == e_MEI_DRV_STATE_SW_INIT_DONE ) ||
       ( MEI_DRV_STATE_GET(pMeiDev) == e_MEI_DRV_STATE_BOOT_WAIT_ROM_ALIVE ))
   {
      if ( (IFX_uint8_t)pInitDone->BTCFG != pMeiDev->modemData.devBootMode )
      {
         /* boot mode mismatch --> warning */
        PRN_ERR_INT_NL( MEI_ROM, MEI_DRV_PRN_LEVEL_WRN,
              ("MEI_DRV[%02d]: EVT_INITDONE - bootmode mismatch cfg: %d, msg: %d" MEI_DRV_CRLF,
                MEI_DRV_LINENUM_GET(pMeiDev), pMeiDev->modemData.devBootMode,
                (IFX_uint8_t)pInitDone->BTCFG ));

      }

      /* set next state: ROM Handler alive */
      MEI_DRV_STATE_SET(pMeiDev, e_MEI_DRV_STATE_BOOT_ROM_ALIVE);

   }
   else
   {
      /* discard message and reset the driver state --> ROM handler alive */
      tempState = MEI_DRV_STATE_GET(pMeiDev);
      MEI_DRV_STATE_SET(pMeiDev, e_MEI_DRV_STATE_BOOT_ROM_ALIVE);

      PRN_ERR_INT_NL( MEI_ROM, MEI_DRV_PRN_LEVEL_ERR,
           ("MEI_DRV[%02d]: <EVT_INITDONE> recv - reset state curr %d --> new %d" MEI_DRV_CRLF,
             MEI_DRV_LINENUM_GET(pMeiDev), tempState, MEI_DRV_STATE_GET(pMeiDev) ));
      return IFX_ERROR;
   }

   PRN_DBG_INT( MEI_ROM, MEI_DRV_PRN_LEVEL_LOW, MEI_DRV_LINENUM_GET(pMeiDev),
        ("MEI_DRV[%02d]: <EVT_INITDONE> - received" MEI_DRV_CRLF,
          MEI_DRV_LINENUM_GET(pMeiDev)));

   return IFX_SUCCESS;
}

#if (MEI_SUPPORT_DFE_GPA_ACCESS == 1)
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

\return
   Always IFX_SUCCESS.

\attention
   - Called on int-level
*/
static IFX_int32_t MEI_FDOnAck_GpaAccess( MEI_DEV_T *pMeiDev,
                                                MEI_Mailbox_t *pBootMsg )
{
   if ( MEI_DRV_MAILBOX_STATE_GET(pMeiDev) == e_MEI_MB_FREE)
   {
      /* receive unexpected boot message */
      PRN_ERR_INT_NL( MEI_ROM, MEI_DRV_PRN_LEVEL_ERR,
           ("MEI_DRV[%02d]: unexpected <ACK> GPA - msgId = 0x%04X" MEI_DRV_CRLF,
             MEI_DRV_LINENUM_GET(pMeiDev), pBootMsg->MessageID));
      MEI_LOG_BOOT_MSG(pBootMsg, "Recv unexp ACK GPA", MEI_DRV_PRN_LEVEL_HIGH);

      return IFX_SUCCESS;
   }

   /*
      ACK expected - free the mailbox status
   */
   MEI_DRV_MAILBOX_STATE_SET(pMeiDev, e_MEI_MB_FREE);

   /* check recev ack against outstanding cmd */
   if ( (IFX_uint16_t)(pMeiDev->gpaBuf.MessageID | 0x200) != pBootMsg->MessageID)
   {
      PRN_ERR_INT_NL( MEI_ROM, MEI_DRV_PRN_LEVEL_ERR,
           ("MEI_DRV[%02d]: mismatch msgId - CMD = 0x%04X ACK = 0x%04X" MEI_DRV_CRLF,
             MEI_DRV_LINENUM_GET(pMeiDev), (IFX_uint16_t)pMeiDev->gpaBuf.MessageID, pBootMsg->MessageID));
      MEI_LOG_BOOT_MSG(pBootMsg, "Recv mismatch ACK GPA", MEI_DRV_PRN_LEVEL_HIGH);

      return IFX_SUCCESS;
   }


   /* return data */
   switch(pBootMsg->MessageID)
   {
      case ACK_MEMORYMAPPEDWRITE:
      case ACK_AUXREGISTERWRITE:
         /* no action necessary */
         break;
      case ACK_MEMORYMAPPEDREAD:
         {
            ACK_MemoryMappedRead_t *pAckMemMapRd =
                              (ACK_MemoryMappedRead_t *)&pBootMsg->Params;
            /* copy data */
            pMeiDev->gpaBuf.GpaBuffer[0] = pAckMemMapRd->Address;
            pMeiDev->gpaBuf.GpaBuffer[1] = pAckMemMapRd->Value;
         }
         break;
      case ACK_AUXREGISTERREAD:
         {
            ACK_AuxRegisterRead_t *pAckAuxRegRd =
                              (ACK_AuxRegisterRead_t *)&pBootMsg->Params;
            /* copy data */
            pMeiDev->gpaBuf.GpaBuffer[0] = pAckAuxRegRd->RegNo;
            pMeiDev->gpaBuf.GpaBuffer[1] = pAckAuxRegRd->Value;
         }
         break;
      default:
         /* shit */
         PRN_ERR_INT_NL( MEI_ROM, MEI_DRV_PRN_LEVEL_ERR,
              ("MEI_DRV[%02d]: invalid GPA message msgId = 0x%04X" MEI_DRV_CRLF,
                MEI_DRV_LINENUM_GET(pMeiDev), pBootMsg->MessageID));
         return IFX_SUCCESS;
   }

   /* return OpCode */
   pMeiDev->gpaBuf.MessageID = (IFX_uint32_t)pBootMsg->MessageID;

   return IFX_SUCCESS;
}
#endif   /* #if (MEI_SUPPORT_DFE_GPA_ACCESS == 1) */


#define ON_BOOT_MSG_ID(boot_msg_id, boot_msg_funct, p_boot_msg) \
               case boot_msg_id: \
                  ret = boot_msg_funct(pMeiDev, p_boot_msg); \
                  break

/**
   ROM Boot message handler - process messages while boot phase.

\param
   pMeiDev: Points to the VRX device struct.
\param
   pBootMsg: Points to the previous received boot message.

\return
   Always IFX_SUCCESS

\attention
   - Called on int-level
*/
static IFX_int32_t MEI_RomBootMsgHandler( MEI_DEV_T *pMeiDev,
                                                MEI_Mailbox_t *pBootMsg)
{
   IFX_int32_t ret = 0;

   MEI_LOG_BOOT_MSG(pBootMsg, "Recv Boot Msg", MEI_DRV_PRN_LEVEL_NORMAL);

   /* process the boot message */
   switch(pBootMsg->MessageID)
   {
      #if (MEI_SUPPORT_DL_DMA_CS == 1)
      /* download via DMA and CodeSwap */
      ON_BOOT_MSG_ID( ACK_ONLINESTATEACTIVATE, MEI_FDOnAck_ONLINESTATEACTIVATE, pBootMsg);
      #endif

      ON_BOOT_MSG_ID( EVT_INITDONE, MEI_FDOnEvt_INITDONE, pBootMsg);

      #if (MEI_SUPPORT_DFE_GPA_ACCESS == 1)
      ON_BOOT_MSG_ID( ACK_MEMORYMAPPEDREAD,  MEI_FDOnAck_GpaAccess, pBootMsg);
      ON_BOOT_MSG_ID( ACK_MEMORYMAPPEDWRITE, MEI_FDOnAck_GpaAccess, pBootMsg);
      ON_BOOT_MSG_ID( ACK_AUXREGISTERWRITE,  MEI_FDOnAck_GpaAccess, pBootMsg);
      ON_BOOT_MSG_ID( ACK_AUXREGISTERREAD,   MEI_FDOnAck_GpaAccess, pBootMsg);
      #endif

      default:
         PRN_DBG_INT( MEI_ROM, MEI_DRV_PRN_LEVEL_LOW, MEI_DRV_LINENUM_GET(pMeiDev),
              ("MEI_DRV[%02d]: ERROR - unknown boot msgId 0x%04X" MEI_DRV_CRLF,
                MEI_DRV_LINENUM_GET(pMeiDev), pBootMsg->MessageID));

         break;
         /* unknown message */
   }

   return IFX_SUCCESS;
}


/* ==========================================================================
   Global function definitions
   ========================================================================== */


/**
   Write a BOOT mailbox message to the VRX.

\param
   pMeiDev: Points to the VRX device struct.
\param
   pMsg:    Points to the message.
\param
   msgSize: Size [byte] of the message.

\return
   Number of transfered bytes.
   Negativ value in case of error.
*/
IFX_int32_t MEI_WriteRomBootMsg( MEI_DEV_T *pMeiDev,
                                       MEI_Mailbox_t *pBootMsg,
                                       IFX_int32_t  msgSize )
{
   IFX_int32_t ret, payloadCount, i;

   PRN_DBG_USR( MEI_ROM, MEI_DRV_PRN_LEVEL_LOW, MEI_DRV_LINENUM_GET(pMeiDev),
          ("MEI_DRV[%02d]: Write MBox boot msgId(0x%02X)) params[%d]" MEI_DRV_CRLF,
           MEI_DRV_LINENUM_GET(pMeiDev), pBootMsg->MessageID, pBootMsg->Size));

   MEI_TRACE_BOOT_MSG(pBootMsg, "Write Boot Msg", MEI_DRV_PRN_LEVEL_NORMAL);

   /* swap 32bit payload */
   payloadCount = (pBootMsg->Size > 8) ? 8 : pBootMsg->Size;

   for (i = 0; i < payloadCount; i++)
   {
      pBootMsg->Params[i] = SWAP32_DMA_WIDTH_ORDER(pBootMsg->Params[i]);
   }

   MEI_TRACE_BOOT_MSG(pBootMsg, "Write Boot Msg (SWAPPED)", MEI_DRV_PRN_LEVEL_LOW);

   /* protect against interrupt driven MailBox read */
   MEI_DRV_GET_UNIQUE_ACCESS(pMeiDev);

   if ( (ret = MEI_WriteMailBox( &pMeiDev->meiDrvCntrl,
                                 pMeiDev->modemData.mBoxDescr.addrMe2Arc,
                                 (MEI_MEI_MAILBOX_T *)pBootMsg,
                                 msgSize ))
         != (msgSize / (IFX_int32_t)sizeof(IFX_uint16_t)) )
   {
      PRN_ERR_USR_NL( MEI_ROM, MEI_DRV_PRN_LEVEL_ERR,
             ("MEI_DRV[%02d]: ERROR - write MBox boot msg" MEI_DRV_CRLF,
              MEI_DRV_LINENUM_GET(pMeiDev)));
   }
   else
   {
      /* set VRX device mailbox status */
      MEI_DRV_MAILBOX_STATE_SET(pMeiDev, e_MEI_MB_PENDING_ACK_1);
      ret *= sizeof(IFX_uint16_t);

      PRN_DBG_USR( MEI_ROM, MEI_DRV_PRN_LEVEL_LOW, MEI_DRV_LINENUM_GET(pMeiDev),
             ("MEI_DRV[%02d]: MBox boot msg written - size = %d" MEI_DRV_CRLF,
              MEI_DRV_LINENUM_GET(pMeiDev), ret));
   }

   MEI_DRV_RELEASE_UNIQUE_ACCESS(pMeiDev);

   return ret;

}


/**
   Get and process an incoming BOOT ROM mailbox message.

\param
   pMeiDev: Points to the VRX device struct.

\return
   NONE - all statistics will be set within the device struct.

\attention
   - Called on int-level
*/
IFX_void_t MEI_RecvRomBootMsg(MEI_DEV_T *pMeiDev )
{
   IFX_uint32_t count, payloadCount, i;
   MEI_Mailbox_t  bootMsg;

   /* get the message */
   count = MEI_GetMailBoxBootMsg( &pMeiDev->meiDrvCntrl,
                                  &bootMsg,
                                  sizeof(MEI_Mailbox_t) / sizeof(IFX_uint16_t),
                                  IFX_FALSE );

   /* process the Firmware Download Handler */
   if (count > 0)
   {
      /* swap 32bit payload */
      payloadCount = (bootMsg.Size > 8) ? 8 : bootMsg.Size;

      for (i = 0; i < payloadCount; i++)
      {
         bootMsg.Params[i] = SWAP32_DMA_WIDTH_ORDER(bootMsg.Params[i]);
      }

      switch(MEI_DRV_STATE_GET(pMeiDev))
      {
         case e_MEI_DRV_STATE_SW_INIT_DONE:
            if (bootMsg.MessageID == EVT_INITDONE)
               MEI_RomBootMsgHandler(pMeiDev, &bootMsg);

            break;
         case e_MEI_DRV_STATE_BOOT_WAIT_ROM_ALIVE:
         case e_MEI_DRV_STATE_BOOT_ROM_ALIVE:
         #if (MEI_SUPPORT_DL_DMA_CS == 1)
         case e_MEI_DRV_STATE_BOOT_ROM_DL_PENDING:
         #endif
            MEI_RomBootMsgHandler(pMeiDev, &bootMsg);

            PRN_DBG_INT( MEI_ROM, MEI_DRV_PRN_LEVEL_NORMAL, MEI_DRV_LINENUM_GET(pMeiDev),
                ("MEI_DRV[%02d]: Boot Message processed" MEI_DRV_CRLF,
                  MEI_DRV_LINENUM_GET(pMeiDev)));

            break;
         default:
            /* error unexpected message */
            PRN_ERR_INT_NL( MEI_ROM, MEI_DRV_PRN_LEVEL_ERR,
                 ( "MEI_DRV[%02d]: DISCARD - "
                   "unexpected boot message[0x%04X] (invalid state %d)" MEI_DRV_CRLF,
                   MEI_DRV_LINENUM_GET(pMeiDev),
                   bootMsg.MessageID, MEI_DRV_STATE_GET(pMeiDev) ));

            MEI_LOG_BOOT_MSG(&bootMsg, "Recv - invalid", MEI_DRV_PRN_LEVEL_HIGH);
      }
   }

   /* after mailbox processing --> release the mailbox */
   MEI_ReleaseMailboxMsg(&pMeiDev->meiDrvCntrl);

   return;
}

#if (MEI_SUPPORT_DFE_GPA_ACCESS == 1)
/**
   Write to the target via General Purpose Access.

   - CMD_MEMORYMAPPEDWRITE --> ACK_MEMORYMAPPEDWRITE
   - CMD_AUXREGISTERWRITE  --> ACK_AUXREGISTERWRITE

\param
   pMeiDev: Points to the VRX device struct.
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
IFX_int32_t MEI_RomHandlerWrGpa( MEI_DEV_T *pMeiDev,
                                       IFX_boolean_t aux,
                                       IFX_uint32_t addr, IFX_uint32_t val)
{
   IFX_uint32_t      msg_size;
   MEI_Mailbox_t   bootMsg;

   /*
      setup the boot message
   */
   bootMsg.MbxCode = MEI_MBOX_CODE_BOOT;
   bootMsg.FctCode = 0x8000;

   switch(aux)
   {
      case IFX_FALSE:
         PRN_DBG_USR( MEI_ROM, MEI_DRV_PRN_LEVEL_NORMAL, MEI_DRV_LINENUM_GET(pMeiDev),
                ("MEI_DRV[%02d]: ROM - Write GPA, mem = 0x%08X, val = 0x%08X" MEI_DRV_CRLF,
                 MEI_DRV_LINENUM_GET(pMeiDev), addr, val ));

         bootMsg.MessageID = CMD_MEMORYMAPPEDWRITE;
         bootMsg.Size = 2;
         ((CMD_MemoryMappedWrite_t *)&bootMsg.Params)->Address = addr;
         ((CMD_MemoryMappedWrite_t *)&bootMsg.Params)->Value = val;

         /* mailbox message size: payload + header */
         msg_size = sizeof(CMD_MemoryMappedWrite_t) +
                        MEI_PARAM_COUNT_16_TO_8(MEI_BOOT_HEADER_16BIT_SIZE);
         break;
      case IFX_TRUE:
         PRN_DBG_USR( MEI_ROM, MEI_DRV_PRN_LEVEL_NORMAL, MEI_DRV_LINENUM_GET(pMeiDev),
                ("MEI_DRV[%02d]: ROM - Write GPA, AUX = 0x%X, 0x%08X" MEI_DRV_CRLF,
                 MEI_DRV_LINENUM_GET(pMeiDev), addr, val ));

         bootMsg.MessageID = CMD_AUXREGISTERWRITE;
         bootMsg.Size = 2;
         ((CMD_AuxRegisterWrite_t *)&bootMsg.Params)->RegNo = addr;
         ((CMD_AuxRegisterWrite_t *)&bootMsg.Params)->Value = val;

         /* mailbox message size: payload + header */
         msg_size = sizeof(CMD_AuxRegisterWrite_t) +
                        MEI_PARAM_COUNT_16_TO_8(MEI_BOOT_HEADER_16BIT_SIZE);

         break;
      default:
         PRN_ERR_USR_NL( MEI_ROM, MEI_DRV_PRN_LEVEL_WRN,
                ("MEI_DRV[%02d]: ROM - Write GPA, unknown dest = 0x%X" MEI_DRV_CRLF,
                 MEI_DRV_LINENUM_GET(pMeiDev), aux ));

         return -e_MEI_ERR_INVAL_ARG;
   }


   /* keep opcode for check ACK */
   pMeiDev->gpaBuf.MessageID = (IFX_uint32_t)bootMsg.MessageID;

   if ( (MEI_WriteRomBootMsg(pMeiDev, &bootMsg, msg_size)) == 0 )
   {
      /* error while send */
      PRN_ERR_USR_NL( MEI_ROM, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DRV[%02d]: ERROR - send boot msg <%s>" MEI_DRV_CRLF,
              MEI_DRV_LINENUM_GET(pMeiDev),
              (aux) ? "CMD_AUXREGISTERWRITE" : "CMD_MEMORYMAPPEDWRITE"));

      pMeiDev->gpaBuf.MessageID = 0;

      return -e_MEI_ERR_OP_FAILED;
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
            break;
      }
      else
         break;
   }

   /* check result */
   if ( (IFX_uint16_t)pMeiDev->gpaBuf.MessageID != (bootMsg.MessageID | 0x200) )
   {
      /* Error */
      PRN_ERR_USR_NL( MEI_ROM, MEI_DRV_PRN_LEVEL_ERR,
             ("MEI_DRV[%02d]: ROM - Write GPA, timeout for msgId 0x%04X" MEI_DRV_CRLF,
              MEI_DRV_LINENUM_GET(pMeiDev), bootMsg.MessageID ));

      pMeiDev->gpaBuf.MessageID = 0;
      return -e_MEI_ERR_DEV_TIMEOUT;
   }

   /* clear the GPA msg id */
   pMeiDev->gpaBuf.MessageID = 0;
   return IFX_SUCCESS;
}


/**
   Read from the target via General Purpose Access.

   - CMD_MEMORYMAPPEDREAD  --> ACK_MEMORYMAPPEDREAD
   - CMD_AUXREGISTERREAD        --> ACK_AUXREGISTERREAD

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
IFX_int32_t MEI_RomHandlerRdGpa( MEI_DEV_T *pMeiDev,
                                       IFX_boolean_t aux,
                                       IFX_uint32_t addr, IFX_uint32_t *val)
{
   IFX_uint32_t      msg_size;
   MEI_Mailbox_t   bootMsg;

   /* setup the boot message */
   bootMsg.MbxCode = MEI_MBOX_CODE_BOOT;

   bootMsg.FctCode = 0x8000;

   switch(aux)
   {
      case IFX_FALSE:
         bootMsg.MessageID = CMD_MEMORYMAPPEDREAD;
         bootMsg.Size = sizeof(CMD_MemoryMappedRead_t) / sizeof(IFX_uint32_t);
         ((CMD_MemoryMappedRead_t *)&bootMsg.Params)->Address = addr;

         /* mailbox message size: payload + header */
         msg_size = sizeof(CMD_MemoryMappedRead_t) +
                        MEI_PARAM_COUNT_16_TO_8(MEI_BOOT_HEADER_16BIT_SIZE);

         PRN_DBG_USR( MEI_ROM, MEI_DRV_PRN_LEVEL_NORMAL, MEI_DRV_LINENUM_GET(pMeiDev),
                ("MEI_DRV[%02d]: ROM Read GPA (mem) - msgID = 0x%04X, size = %d, addr = 0x%08X" MEI_DRV_CRLF,
                 MEI_DRV_LINENUM_GET(pMeiDev), bootMsg.MessageID, bootMsg.Size, bootMsg.Params[0] ));

         break;
      case IFX_TRUE:
         bootMsg.MessageID = CMD_AUXREGISTERREAD;
         bootMsg.Size = sizeof(CMD_AuxRegisterRead_t) / sizeof(IFX_uint32_t);;
         ((CMD_AuxRegisterRead_t *)&bootMsg.Params)->RegNo = addr;

         /* mailbox message size: payload + header */
         msg_size = sizeof(CMD_AuxRegisterRead_t) +
                        MEI_PARAM_COUNT_16_TO_8(MEI_BOOT_HEADER_16BIT_SIZE);

         PRN_DBG_USR( MEI_ROM, MEI_DRV_PRN_LEVEL_NORMAL, MEI_DRV_LINENUM_GET(pMeiDev),
                ("MEI_DRV[%02d]: ROM Read GPA (aux) - msgID = 0x%04X, size = %d, regNo = 0x%08X" MEI_DRV_CRLF,
                 MEI_DRV_LINENUM_GET(pMeiDev), bootMsg.MessageID, bootMsg.Size, bootMsg.Params[0] ));
         break;
      default:
         PRN_ERR_USR_NL( MEI_ROM, MEI_DRV_PRN_LEVEL_WRN,
                ("MEI_DRV[%02d]: ROM - Read GPA, unknown dest = 0x%X" MEI_DRV_CRLF,
                 MEI_DRV_LINENUM_GET(pMeiDev), aux ));

         return -e_MEI_ERR_INVAL_ARG;
   }


   /* keep opcode for check ACK */
   pMeiDev->gpaBuf.MessageID = (IFX_uint32_t)bootMsg.MessageID;

   if ( (MEI_WriteRomBootMsg(pMeiDev, &bootMsg, msg_size)) == 0 )
   {
      /* error while send */
      PRN_ERR_USR_NL( MEI_ROM, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DRV[%02d]: ERROR - send boot msg <%s>" MEI_DRV_CRLF,
              MEI_DRV_LINENUM_GET(pMeiDev),
              (aux) ? "CMD_AUXREGREAD" : "CMD_MEMORYMAPPEDREAD"));

      pMeiDev->gpaBuf.MessageID = 0;
      return -e_MEI_ERR_OP_FAILED;
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
            break;
      }
      else
         break;
   }

   /* check result */
   if ( (IFX_uint16_t)pMeiDev->gpaBuf.MessageID != (bootMsg.MessageID | 0x200) )
   {
      /* success */
      PRN_ERR_USR_NL( MEI_ROM, MEI_DRV_PRN_LEVEL_ERR,
             ("MEI_DRV[%02d]: ROM - Read GPA, timeout for msgId 0x%04X" MEI_DRV_CRLF,
              MEI_DRV_LINENUM_GET(pMeiDev), bootMsg.MessageID ));

      pMeiDev->gpaBuf.MessageID = 0;
      return -e_MEI_ERR_DEV_TIMEOUT;
   }
   else
   {
      /* clear the GPA msg id */
      pMeiDev->gpaBuf.MessageID = 0;
   }

   /* SUCCESS --> return value */
   switch (bootMsg.MessageID)
   {
      case CMD_MEMORYMAPPEDREAD:
         *val = ((ACK_MemoryMappedRead_t *)&pMeiDev->gpaBuf.GpaBuffer)->Value ;
         break;
      case CMD_AUXREGISTERREAD:
         *val = ((ACK_AuxRegisterRead_t *)&pMeiDev->gpaBuf.GpaBuffer)->Value ;
         break;
      default:
         /* shit should never happen */
         return IFX_ERROR;
   }

   return IFX_SUCCESS;
}
#endif      /* #if (MEI_SUPPORT_DFE_GPA_ACCESS == 1) */

#endif      /* MEI_SUPPORT_ROM_CODE */

