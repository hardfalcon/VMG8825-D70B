/******************************************************************************

                          Copyright (c) 2007-2015
                     Lantiq Beteiligungs-GmbH & Co. KG

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/* ==========================================================================
   Description : VRX Driver Debug Functions
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

#include "drv_mei_cpe_api.h"

#if ((MEI_MSG_DUMP_ENABLE == 1) || (MEI_MISC_TEST == 1))
#include "cmv_message_format.h"
#endif

#if (MEI_MISC_TEST == 1)
#include "drv_mei_cpe_mailbox.h"
#endif


/* ============================================================================
   Local Macro declaration
   ========================================================================= */

#ifdef MEI_STATIC
#undef MEI_STATIC
#endif

#ifdef MEI_DEBUG
#define MEI_STATIC
#else
#define MEI_STATIC   static
#endif

#if ( (MEI_MSG_DUMP_ENABLE == 1) && (MEI_DEBUG_PRINT == 1) )

#define DUMP_USR_PRN(name,level,message) do {if(level >= MEI_PrnUsrModule_##name) \
      { MEI_PRINT_INT_RAW message ; } } while(0)
#define DUMP_INT_PRN(name,level,message) do {if(level >= MEI_PrnIntModule_##name) \
      { MEI_PRINT_INT_RAW message ; } } while(0)


/* LOG macro - no LF or CR */
#define MEI_MSG_DUMP_LOG_16(data16)\
               temp16.d16 = MEI_SWAP_TO_LITTLE_S(data16); \
               DUMP_INT_PRN(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_HIGH, ("%02X %02X ", temp16.d8[0], temp16.d8[1]))

#define MEI_MSG_DUMP_LOG_32(data32)\
               temp32.d32 = MEI_SWAP_TO_LITTLE_L(data32); \
               DUMP_INT_PRN(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_HIGH, ("%02X %02X %02X %02X ", \
                        temp32.d8[0], temp32.d8[1], temp32.d8[2], temp32.d8[3]))

/* LOG macro - no LF or CR */
#define MEI_MSG_DUMP_TRC_16(data16)\
               temp16.d16 = MEI_SWAP_TO_LITTLE_S(data16); \
               DUMP_USR_PRN(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_HIGH, ("%02X %02X ", temp16.d8[0], temp16.d8[1]))

#define MEI_MSG_DUMP_TRC_32(data32)\
               temp32.d32 = MEI_SWAP_TO_LITTLE_L(data32); \
               DUMP_USR_PRN(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_HIGH, ("%02X %02X %02X %02X ", \
                   temp32.d8[0], temp32.d8[1], temp32.d8[2], temp32.d8[3]))

#endif      /* #if ( (MEI_MSG_DUMP_ENABLE == 1) && (MEI_DEBUG_PRINT == 1) ) */

/* ============================================================================
   Local Function declaration
   ========================================================================= */

#if ( (MEI_MSG_DUMP_ENABLE == 1) && (MEI_DEBUG_PRINT == 1) )
MEI_STATIC IFX_void_t MEI_DbgMsgDumpLogLabel(
                              CMV_STD_MESSAGE_T *pCmvMsg,
                              IFX_uint16_t      fctOpCode);
MEI_STATIC IFX_void_t MEI_DbgMsgDumpTraceLabel(
                              CMV_STD_MESSAGE_T *pCmvMsg,
                              IFX_uint16_t      fctOpCode);

MEI_STATIC IFX_void_t MEI_DbgMsgDumpLogWE(
                              MEI_DEV_T       *pMeiDev,
                              CMV_STD_MESSAGE_T *pCmvMsg,
                              IFX_uint16_t      fctOpCode);
MEI_STATIC IFX_void_t MEI_DbgMsgDumpTrcWE(
                              MEI_DEV_T       *pMeiDev,
                              CMV_STD_MESSAGE_T *pCmvMsg,
                              IFX_uint16_t      fctOpCode);
MEI_STATIC IFX_void_t MEI_DbgMsgDumpTrcWH(
                              MEI_DEV_T       *pMeiDev,
                              CMV_STD_MESSAGE_T *pCmvMsg,
                              IFX_uint16_t      fctOpCode);
MEI_STATIC IFX_void_t MEI_DbgMsgDumpLogWH(
                              MEI_DEV_T       *pMeiDev,
                              CMV_STD_MESSAGE_T *pCmvMsg,
                              IFX_uint16_t      fctOpCode);
MEI_STATIC IFX_void_t MEI_DbgMsgDumpLogDrv(
                              MEI_DEV_T       *pMeiDev,
                              CMV_STD_MESSAGE_T *pCmvMsg,
                              IFX_uint16_t      fctOpCode);
MEI_STATIC IFX_void_t MEI_DbgMsgDumpTraceDrv(
                              MEI_DEV_T       *pMeiDev,
                              CMV_STD_MESSAGE_T *pCmvMsg,
                              IFX_uint16_t      fctOpCode);
#endif


#if (MEI_SUPPORT_MEI_DEBUG == 1)
static IFX_int32_t MEI_GetNfcDataPerInstance(
                     MEI_DEV_T    *pMeiDev,
                     MEI_DYN_NFC_DATA_T *pNfcData,
                     IFX_uint8_t    nfcListNum,
                     IFX_uint8_t    *pBuf,
                     IFX_int32_t    bufSize);
#endif      /* MEI_SUPPORT_MEI_DEBUG */


/* ============================================================================
   VRX Driver Debug Varibales
   ========================================================================= */


#if ( (MEI_MSG_DUMP_ENABLE == 1) && (MEI_DEBUG_PRINT == 1) )
/* VRX-Driver: Msg Dump module - create print level variable */
MEI_DRV_PRN_USR_MODULE_CREATE(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_LOW);
MEI_DRV_PRN_INT_MODULE_CREATE(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_LOW);

IFX_uint32_t  MEI_msgDumpEnable   = 0;
IFX_uint32_t  MEI_msgDumpOutCntrl = 0;
IFX_boolean_t MEI_msgDumpSetLabel = IFX_FALSE;
IFX_uint16_t  MEI_msgDumpLine     = 0;
/* VDSL2 VRX project */
IFX_uint16_t  MEI_msgDumpId = 0x0086;
#endif      /* #if (MEI_MSG_DUMP_ENABLE == 1) */



/* ============================================================================
   VRX Driver Debug Function - Message Dump
   ========================================================================= */
#if ( (MEI_MSG_DUMP_ENABLE == 1) && (MEI_DEBUG_PRINT == 1) )

/**
   Dump start label (LOG).
*/
MEI_STATIC IFX_void_t MEI_DbgMsgDumpLogLabel(
                              CMV_STD_MESSAGE_T *pCmvMsg,
                              IFX_uint16_t      fctOpCode)
{

   if (MEI_msgDumpSetLabel)
   {
      DUMP_INT_PRN(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_NORMAL, ("(%04X/%04X) ",
         P_CMV_MSGHDR_FCT_OPCODE_GET(pCmvMsg), fctOpCode) );

      switch(P_CMV_MSGHDR_FCT_OPCODE_GET(pCmvMsg))
      {
         case H2D_CMV_READ:
            DUMP_INT_PRN(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_HIGH, ("--> CmdRd" MEI_DRV_CRLF) );
            break;
         case H2D_CMV_WRITE:
            DUMP_INT_PRN(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_HIGH, ("--> CmdWr" MEI_DRV_CRLF) );
            break;
         case H2D_DEBUG_READ_DM:
            DUMP_INT_PRN(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_HIGH, ("--> DbgRdD\n\r") );
            break;
         case H2D_DEBUG_WRITE_DM:
            DUMP_INT_PRN(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_HIGH, ("--> DbgWrD\n\r") );
            break;
         case H2D_DEBUG_READ_PM:
            DUMP_INT_PRN(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_HIGH, ("--> DbgRdP\n\r") );
            break;
         case H2D_DEBUG_WRITE_PM:
            DUMP_INT_PRN(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_HIGH, ("--> DbgWrP\n\r") );
            break;
/*
#define H2D_DEBUG_READ_DM           0x02
#define H2D_DEBUG_READ_PM           0x06
#define H2D_DEBUG_WRITE_DM          0x0A
#define H2D_DEBUG_WRITE_PM          0x0E
*/
         case D2H_CMV_READ_REPLY:
            DUMP_INT_PRN(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_HIGH, ("<-- AckRd" MEI_DRV_CRLF) );
            break;
         case D2H_CMV_WRITE_REPLY:
            DUMP_INT_PRN(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_HIGH, ("<-- AckWr" MEI_DRV_CRLF) );
            break;
         case D2H_DEBUG_READ_DM_REPLY:
            DUMP_INT_PRN(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_HIGH, ("<-- AckDbgRdD\n\r") );
            break;
         case D2H_DEBUG_WRITE_DM_REPLY:
            DUMP_INT_PRN(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_HIGH, ("<-- AckDbgWrD\n\r") );
            break;
         case D2H_DEBUG_READ_PM_REPLY:
            DUMP_INT_PRN(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_HIGH, ("<-- AckDbgRdP\n\r") );
            break;
         case D2H_DEBUG_WRITE_PM_REPLY:
            DUMP_INT_PRN(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_HIGH, ("<-- AckDbgWrP\n\r") );
            break;
/*
#define D2H_DEBUG_READ_DM_REPLY     0x03
#define D2H_DEBUG_READ_PM_REPLY     0x07
#define D2H_DEBUG_WRITE_DM_REPLY    0x0B
#define D2H_DEBUG_WRITE_PM_REPLY    0x0F
*/
         case D2H_CMV_INDICATE:
            DUMP_INT_PRN(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_HIGH, ("<-- NFC" MEI_DRV_CRLF) );
            break;
         default:
            DUMP_INT_PRN(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_HIGH, ("<-> unknown 0x%02X\n\r",
               P_CMV_MSGHDR_FCT_OPCODE_GET(pCmvMsg)) );
            break;
      }
   }
   return;
}

/**
   Dump start label (Trace).
*/
MEI_STATIC IFX_void_t MEI_DbgMsgDumpTraceLabel(
                              CMV_STD_MESSAGE_T *pCmvMsg,
                              IFX_uint16_t      fctOpCode)
{
   if (MEI_msgDumpSetLabel)
   {
      DUMP_USR_PRN(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_NORMAL, ("(%04X/%04X) ",
         P_CMV_MSGHDR_FCT_OPCODE_GET(pCmvMsg), fctOpCode) );

      switch(P_CMV_MSGHDR_FCT_OPCODE_GET(pCmvMsg))
      {
         case H2D_CMV_READ:
            DUMP_USR_PRN(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_HIGH, ("--> CmdRd" MEI_DRV_CRLF) );
            break;
         case H2D_CMV_WRITE:
            DUMP_USR_PRN(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_HIGH, ("--> CmdWr" MEI_DRV_CRLF) );
            break;
         case H2D_DEBUG_READ_DM:
            DUMP_USR_PRN(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_HIGH, ("--> DbgRdD\n\r") );
            break;
         case H2D_DEBUG_WRITE_DM:
            DUMP_USR_PRN(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_HIGH, ("--> DbgWrD\n\r") );
            break;
         case H2D_DEBUG_READ_PM:
            DUMP_USR_PRN(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_HIGH, ("--> DbgRdP\n\r") );
            break;
         case H2D_DEBUG_WRITE_PM:
            DUMP_USR_PRN(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_HIGH, ("--> DbgWrP\n\r") );
            break;
         case D2H_CMV_READ_REPLY:
            DUMP_USR_PRN(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_HIGH, ("<-- AckRd" MEI_DRV_CRLF) );
            break;
         case D2H_CMV_WRITE_REPLY:
            DUMP_USR_PRN(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_HIGH, ("<-- AckWr" MEI_DRV_CRLF) );
            break;
         case D2H_DEBUG_READ_DM_REPLY:
            DUMP_USR_PRN(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_HIGH, ("<-- AckDbgRdD\n\r") );
            break;
         case D2H_DEBUG_WRITE_DM_REPLY:
            DUMP_USR_PRN(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_HIGH, ("<-- AckDbgWrD\n\r") );
            break;
         case D2H_DEBUG_READ_PM_REPLY:
            DUMP_USR_PRN(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_HIGH, ("<-- AckDbgRdP\n\r") );
            break;
         case D2H_DEBUG_WRITE_PM_REPLY:
            DUMP_USR_PRN(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_HIGH, ("<-- AckDbgWrP\n\r") );
            break;
         case D2H_CMV_INDICATE:
            DUMP_USR_PRN(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_HIGH, ("<-- NFC" MEI_DRV_CRLF) );
            break;
         default:
            DUMP_USR_PRN(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_HIGH, ("<-> unknown 0x%02X\n\r",
               P_CMV_MSGHDR_FCT_OPCODE_GET(pCmvMsg)) );
            break;
      }
   }

   return;
}

/**
   Debug Message Dump Log (WinEasy)

   Format:

   | -- control header ---| |  data .....
   <msgId> <dbgId> <entity> <00> <01> ....

*/
MEI_STATIC IFX_void_t MEI_DbgMsgDumpLogWE(
                              MEI_DEV_T       *pMeiDev,
                              CMV_STD_MESSAGE_T *pCmvMsg,
                              IFX_uint16_t      fctOpCode)
{
   IFX_int32_t i;
   unsigned short paylSize;
   union {IFX_uint8_t  d8[2]; IFX_uint16_t d16;} temp16;
   union {IFX_uint8_t  d8[4]; IFX_uint16_t d32;} temp32;

   MEI_DbgMsgDumpLogLabel(pCmvMsg, fctOpCode);

   /* dump msg - control header
      0401 003c 0000 13 06 00 00 00 00 00
   */
   DUMP_INT_PRN(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_HIGH,
            ("%04X %04X %04X "
            , ( P_CMV_MSGHDR_MSGID_GET(pCmvMsg) |
              ((P_CMV_MSGHDR_FCT_OPCODE_GET(pCmvMsg) & 0x04) ? MEI_MSGID_BIT_IND_WR_CMD : 0x0000) )
            , MEI_msgDumpId, MEI_DRV_LINENUM_GET(pMeiDev) ));

   /* dump msg - data (CMV index length first)
      0401 003c 0000 13 06 00 00 00 00 00
   */
   MEI_MSG_DUMP_LOG_16(pCmvMsg->header.index);
   MEI_MSG_DUMP_LOG_16(pCmvMsg->header.length);

   paylSize = P_CMV_MSGHDR_PAYLOAD_SIZE_GET(pCmvMsg);
   paylSize = (paylSize > CMV_USED_PAYLOAD_16BIT_SIZE) ? CMV_USED_PAYLOAD_16BIT_SIZE : paylSize;

   if (P_CMV_MSGHDR_BIT_SIZE_GET(pCmvMsg) == (unsigned short)CMV_MSG_BIT_SIZE_32BIT)
   {
      paylSize = paylSize >> 1;
      for (i=0; i < paylSize; i++) {
         MEI_MSG_DUMP_LOG_32(pCmvMsg->payload.params_32Bit[i]); }
   }
   else
   {
      for (i=0; i < paylSize; i++) {
         MEI_MSG_DUMP_LOG_16(pCmvMsg->payload.params_16Bit[i]); }
   }

   if (MEI_msgDumpSetLabel) {
      DUMP_INT_PRN(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_HIGH, (MEI_DRV_CRLF "---" MEI_DRV_CRLF) ); }
   else {
      DUMP_INT_PRN(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_HIGH, (MEI_DRV_CRLF) ); }

   return;
}

/**
   Debug Message Dump Trace WE

   Format:

   | -- control header ---| |  data .....
   <msgId> <dbgId> <entity> <00> <01> ....

*/
MEI_STATIC IFX_void_t MEI_DbgMsgDumpTrcWE(
                              MEI_DEV_T       *pMeiDev,
                              CMV_STD_MESSAGE_T *pCmvMsg,
                              IFX_uint16_t      fctOpCode)
{
   IFX_int32_t i;
   unsigned short paylSize;
   union {IFX_uint8_t  d8[2]; IFX_uint16_t d16;} temp16;
   union {IFX_uint8_t  d8[4]; IFX_uint16_t d32;} temp32;

   MEI_DbgMsgDumpTraceLabel(pCmvMsg, fctOpCode);

   /* dump msg - control header
      0401 003c 0000 13 06 00 00 00 00 00
   */
   DUMP_USR_PRN(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_HIGH,
         ("%04X %04X %04X "
          , ( P_CMV_MSGHDR_MSGID_GET(pCmvMsg) |
              ((P_CMV_MSGHDR_FCT_OPCODE_GET(pCmvMsg) & 0x04) ? MEI_MSGID_BIT_IND_WR_CMD : 0x0000) )
          , MEI_msgDumpId, MEI_DRV_LINENUM_GET(pMeiDev)) );

   /* dump msg - data (CMV index length first)
      0401 003c 0000 13 06 00 00 00 00 00
   */
   MEI_MSG_DUMP_TRC_16(pCmvMsg->header.index);
   MEI_MSG_DUMP_TRC_16(pCmvMsg->header.length);

   paylSize = P_CMV_MSGHDR_PAYLOAD_SIZE_GET(pCmvMsg);
   paylSize = (paylSize > CMV_USED_PAYLOAD_16BIT_SIZE) ? CMV_USED_PAYLOAD_16BIT_SIZE : paylSize;

   if (P_CMV_MSGHDR_BIT_SIZE_GET(pCmvMsg) == (unsigned short)CMV_MSG_BIT_SIZE_32BIT)
   {
      paylSize = paylSize >> 1;
      for (i=0; i < paylSize; i++) {
         MEI_MSG_DUMP_TRC_32(pCmvMsg->payload.params_32Bit[i]); }
   }
   else
   {
      for (i=0; i < paylSize; i++) {
         MEI_MSG_DUMP_TRC_16(pCmvMsg->payload.params_16Bit[i]); }
   }

   if (MEI_msgDumpSetLabel) {
      DUMP_USR_PRN(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_HIGH, (MEI_DRV_CRLF "---" MEI_DRV_CRLF) ); }
   else {
      DUMP_USR_PRN(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_HIGH, (MEI_DRV_CRLF) ); }

   return;
}

/**
   Debug Message Dump Log (WH)

   Format:

   <wh-cmd> msg id, idx, payload ...
*/
MEI_STATIC IFX_void_t MEI_DbgMsgDumpLogWH(
                              MEI_DEV_T       *pMeiDev,
                              CMV_STD_MESSAGE_T *pCmvMsg,
                              IFX_uint16_t      fctOpCode)
{
   IFX_int32_t i;
   IFX_boolean_t  ifxMsg;
   unsigned short paylSize;
   union {IFX_uint8_t  d8[2]; IFX_uint16_t d16;} temp16;

   MEI_DbgMsgDumpLogLabel(pCmvMsg, fctOpCode);

   ifxMsg =  ((P_CMV_MSGHDR_MSGID_GET(pCmvMsg)) & MEI_MSGID_BIT_IND_IFX) ?
               IFX_TRUE : IFX_FALSE;

   switch(fctOpCode)
   {
      case H2D_CMV_READ:
         DUMP_INT_PRN(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_HIGH, ("[%02d]%s ", MEI_DRV_LINENUM_GET(pMeiDev), ifxMsg ? "crd":"cr") );
         break;
      case H2D_CMV_WRITE:
         DUMP_INT_PRN(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_HIGH, ("[%02d]%s ",  MEI_DRV_LINENUM_GET(pMeiDev), ifxMsg ? "cwd":"cw") );
         break;
      case D2H_CMV_READ_REPLY:
         DUMP_INT_PRN(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_HIGH, ("[%02d]%s ",  MEI_DRV_LINENUM_GET(pMeiDev), ifxMsg ? "re_crd":"re_cr") );
         break;
      case D2H_CMV_WRITE_REPLY:
         DUMP_INT_PRN(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_HIGH, ("[%02d]%s ",  MEI_DRV_LINENUM_GET(pMeiDev), ifxMsg ? "re_cwd":"re_cw") );
         break;
      case D2H_CMV_INDICATE:
         DUMP_INT_PRN(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_HIGH, ("[%02d]%s ",  MEI_DRV_LINENUM_GET(pMeiDev), ifxMsg ? "ind":"ind") );
         break;

      case H2D_DEBUG_READ_DM:
      case H2D_DEBUG_READ_PM:
         DUMP_INT_PRN(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_HIGH, ("[%02d]PeRd ",  MEI_DRV_LINENUM_GET(pMeiDev)) );
         break;

      case D2H_DEBUG_READ_DM_REPLY:
      case D2H_DEBUG_READ_PM_REPLY:
         DUMP_INT_PRN(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_HIGH, ("[%02d]re_PeRd ",  MEI_DRV_LINENUM_GET(pMeiDev)) );
         break;

      case H2D_DEBUG_WRITE_DM:
      case H2D_DEBUG_WRITE_PM:
         DUMP_INT_PRN(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_HIGH, ("[%02d]PeWr ",  MEI_DRV_LINENUM_GET(pMeiDev)) );
         break;

      case D2H_DEBUG_WRITE_DM_REPLY:
      case D2H_DEBUG_WRITE_PM_REPLY:
         DUMP_INT_PRN(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_HIGH, ("[%02d]re_PeWr ",  MEI_DRV_LINENUM_GET(pMeiDev)) );
         break;

      default:
         DUMP_INT_PRN(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_HIGH, ("[%02d]??? ", MEI_DRV_LINENUM_GET(pMeiDev)) );
         break;
   }

   /* msg ID byte order (little endian) */
   temp16.d16 = MEI_SWAP_TO_LITTLE_S( (P_CMV_MSGHDR_MSGID_GET(pCmvMsg)  |
                  ((P_CMV_MSGHDR_FCT_OPCODE_GET(pCmvMsg) & 0x04) ? MEI_MSGID_BIT_IND_WR_CMD : 0x0000)) );
   DUMP_INT_PRN(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_HIGH,
            ("0x%02X 0x%02X ", temp16.d8[0], temp16.d8[1]));

   /* index */
   DUMP_INT_PRN(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_HIGH, ("0x%04X ", P_CMV_MSGHDR_INDEX_GET(pCmvMsg)));

   paylSize = P_CMV_MSGHDR_PAYLOAD_SIZE_GET(pCmvMsg);
   paylSize = (paylSize > CMV_USED_PAYLOAD_16BIT_SIZE) ? CMV_USED_PAYLOAD_16BIT_SIZE : paylSize;

   if (P_CMV_MSGHDR_BIT_SIZE_GET(pCmvMsg) == (unsigned short)CMV_MSG_BIT_SIZE_32BIT)
   {
      paylSize = paylSize >> 1;
      for (i=0; i < paylSize; i++) {
         DUMP_INT_PRN(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_HIGH,
                  ("0x%X ", pCmvMsg->payload.params_32Bit[i])); }
   }
   else
   {
      for (i=0; i < paylSize; i++) {
         DUMP_INT_PRN(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_HIGH,
                  ("0x%X ", pCmvMsg->payload.params_16Bit[i])); }
   }

   if (MEI_msgDumpSetLabel) {
      DUMP_INT_PRN(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_HIGH, (MEI_DRV_CRLF "---" MEI_DRV_CRLF) ); }
   else {
      DUMP_INT_PRN(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_HIGH, (MEI_DRV_CRLF) ); }

   return;
}


/**
   Debug Message Dump TRACE (WH)

   Format:

   <wh-cmd> msg id, idx, payload ...
*/
MEI_STATIC IFX_void_t MEI_DbgMsgDumpTrcWH(
                              MEI_DEV_T       *pMeiDev,
                              CMV_STD_MESSAGE_T *pCmvMsg,
                              IFX_uint16_t      fctOpCode)
{
   IFX_int32_t i;
   IFX_boolean_t  ifxMsg;
   unsigned short paylSize;
   union {IFX_uint8_t  d8[2]; IFX_uint16_t d16;} temp16;


   MEI_DbgMsgDumpTraceLabel(pCmvMsg, fctOpCode);

   ifxMsg =  ((P_CMV_MSGHDR_MSGID_GET(pCmvMsg)) & MEI_MSGID_BIT_IND_IFX) ?
               IFX_TRUE : IFX_FALSE;

   switch(fctOpCode)
   {
      case H2D_CMV_READ:
         DUMP_USR_PRN(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_HIGH, ("[%02d]%s ", MEI_DRV_LINENUM_GET(pMeiDev), ifxMsg ? "crd":"cr") );
         break;
      case H2D_CMV_WRITE:
         DUMP_USR_PRN(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_HIGH, ("[%02d]%s ", MEI_DRV_LINENUM_GET(pMeiDev), ifxMsg ? "cwd":"cw") );
         break;
      case D2H_CMV_READ_REPLY:
         DUMP_USR_PRN(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_HIGH, ("[%02d]%s ", MEI_DRV_LINENUM_GET(pMeiDev), ifxMsg ? "re_crd":"re_cr") );
         break;
      case D2H_CMV_WRITE_REPLY:
         DUMP_USR_PRN(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_HIGH, ("[%02d]%s ", MEI_DRV_LINENUM_GET(pMeiDev), ifxMsg ? "re_cwd":"re_cw") );
         break;
      case D2H_CMV_INDICATE:
         DUMP_USR_PRN(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_HIGH, ("[%02d]%s ", MEI_DRV_LINENUM_GET(pMeiDev), ifxMsg ? "ind":"ind") );
         break;
      default:
         DUMP_USR_PRN(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_HIGH, ("[%02d]??? ", MEI_DRV_LINENUM_GET(pMeiDev)) );

         break;
   }

   temp16.d16 = MEI_SWAP_TO_LITTLE_S( (P_CMV_MSGHDR_MSGID_GET(pCmvMsg)  |
                  ((P_CMV_MSGHDR_FCT_OPCODE_GET(pCmvMsg) & 0x04) ? MEI_MSGID_BIT_IND_WR_CMD : 0x0000)) );
   DUMP_USR_PRN(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_HIGH, ("0x%02X 0x%02X ", temp16.d8[0], temp16.d8[1]));

   /* index */
   DUMP_USR_PRN(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_HIGH, ("0x%04X ", P_CMV_MSGHDR_INDEX_GET(pCmvMsg)));

   paylSize = P_CMV_MSGHDR_PAYLOAD_SIZE_GET(pCmvMsg);
   paylSize = (paylSize > CMV_USED_PAYLOAD_16BIT_SIZE) ? CMV_USED_PAYLOAD_16BIT_SIZE : paylSize;

   if (P_CMV_MSGHDR_BIT_SIZE_GET(pCmvMsg) == (unsigned short)CMV_MSG_BIT_SIZE_32BIT)
   {
      paylSize = paylSize >> 1;
      for (i=0; i < paylSize; i++) {
         DUMP_USR_PRN(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_HIGH,
                    ("0x%X ", pCmvMsg->payload.params_32Bit[i])); }
   }
   else
   {
      for (i=0; i < paylSize; i++) {
         DUMP_USR_PRN(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_HIGH,
                    ("0x%X ", pCmvMsg->payload.params_16Bit[i])); }
   }

   if (MEI_msgDumpSetLabel) {
      DUMP_USR_PRN(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_HIGH, (MEI_DRV_CRLF "---" MEI_DRV_CRLF) ); }
   else {
      DUMP_USR_PRN(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_HIGH, (MEI_DRV_CRLF) ); }

   return;
}


/**
   Debug Message Dump Log (IFX format)

   Format:

   <cmd> msg id, idx, length, payload ...
*/
MEI_STATIC IFX_void_t MEI_DbgMsgDumpLogDrv(
                              MEI_DEV_T       *pMeiDev,
                              CMV_STD_MESSAGE_T *pCmvMsg,
                              IFX_uint16_t      fctOpCode)
{
   IFX_int32_t i;
   IFX_boolean_t  ifxMsg;
   unsigned short paylSize;

   ifxMsg =  ((P_CMV_MSGHDR_MSGID_GET(pCmvMsg)) & MEI_MSGID_BIT_IND_IFX) ?
               IFX_TRUE : IFX_FALSE;

   MEI_DbgMsgDumpLogLabel(pCmvMsg, fctOpCode);

   switch(fctOpCode)
   {
      case H2D_CMV_READ:
         DUMP_INT_PRN(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_HIGH, ("Cmd_Rd[%02d]: ", MEI_DRV_LINENUM_GET(pMeiDev)));
         break;
      case H2D_CMV_WRITE:
         DUMP_INT_PRN(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_HIGH, ("Cmd_Wr[%02d]: ", MEI_DRV_LINENUM_GET(pMeiDev)));
         break;
      case D2H_CMV_READ_REPLY:
         DUMP_INT_PRN(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_HIGH, ("ACK_Rd[%02d]: ", MEI_DRV_LINENUM_GET(pMeiDev)));
         break;
      case D2H_CMV_WRITE_REPLY:
         DUMP_INT_PRN(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_HIGH, ("ACK_Wr[%02d]: ", MEI_DRV_LINENUM_GET(pMeiDev)));
         break;
      case D2H_CMV_INDICATE:
         DUMP_INT_PRN(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_HIGH, ("NFC   [%02d]: ", MEI_DRV_LINENUM_GET(pMeiDev)));
         break;
      default:
         DUMP_INT_PRN(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_HIGH, ("u 0x%2X [%02d]: ", fctOpCode, MEI_DRV_LINENUM_GET(pMeiDev)));
         break;
   }

   DUMP_INT_PRN(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_HIGH,
       ("%04X - %04X %04X   |  "
         , ( P_CMV_MSGHDR_MSGID_GET(pCmvMsg) |
             ((P_CMV_MSGHDR_FCT_OPCODE_GET(pCmvMsg) & 0x04) ? MEI_MSGID_BIT_IND_WR_CMD : 0x0000) )
         , P_CMV_MSGHDR_INDEX_GET(pCmvMsg)
         , P_CMV_MSGHDR_LENGTH_GET(pCmvMsg) ));


   paylSize = P_CMV_MSGHDR_PAYLOAD_SIZE_GET(pCmvMsg);
   paylSize = (paylSize > CMV_USED_PAYLOAD_16BIT_SIZE) ? CMV_USED_PAYLOAD_16BIT_SIZE : paylSize;

   if (P_CMV_MSGHDR_BIT_SIZE_GET(pCmvMsg) == (unsigned short)CMV_MSG_BIT_SIZE_32BIT)
   {
      paylSize = paylSize >> 1;
      for (i=0; i < paylSize; i++)
      {
         DUMP_INT_PRN(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_HIGH,("%08X ", pCmvMsg->payload.params_32Bit[i]));
      }
   }
   else
   {
      for (i=0; i < paylSize; i++)
      {
         DUMP_INT_PRN(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_HIGH,("%04X ", pCmvMsg->payload.params_16Bit[i]));
      }
   }

   if (MEI_msgDumpSetLabel) {
      DUMP_INT_PRN(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_HIGH, (MEI_DRV_CRLF "---" MEI_DRV_CRLF) ); }
   else {
      DUMP_INT_PRN(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_HIGH, (MEI_DRV_CRLF) ); }

   return;
}


/**
   Debug Message Dump Trace (IFX format)

   Format:

   <cmd> msg id, idx, length, payload ...
*/
MEI_STATIC IFX_void_t MEI_DbgMsgDumpTraceDrv(
                              MEI_DEV_T       *pMeiDev,
                              CMV_STD_MESSAGE_T *pCmvMsg,
                              IFX_uint16_t      fctOpCode)
{
   IFX_int32_t i;
   IFX_boolean_t  ifxMsg;
   unsigned short paylSize;

   ifxMsg =  ((P_CMV_MSGHDR_MSGID_GET(pCmvMsg)) & MEI_MSGID_BIT_IND_IFX) ?
               IFX_TRUE : IFX_FALSE;

   MEI_DbgMsgDumpTraceLabel(pCmvMsg, fctOpCode);

   switch(fctOpCode)
   {
      case H2D_CMV_READ:
         DUMP_USR_PRN(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_HIGH, ("Cmd_Rd[%02d]: ", MEI_DRV_LINENUM_GET(pMeiDev)));
         break;
      case H2D_CMV_WRITE:
         DUMP_USR_PRN(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_HIGH, ("Cmd_Wr[%02d]: ", MEI_DRV_LINENUM_GET(pMeiDev)));
         break;
      case D2H_CMV_READ_REPLY:
         DUMP_USR_PRN(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_HIGH, ("ACK_Rd[%02d]: ", MEI_DRV_LINENUM_GET(pMeiDev)));
         break;
      case D2H_CMV_WRITE_REPLY:
         DUMP_USR_PRN(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_HIGH, ("ACK_Wr[%02d]: ", MEI_DRV_LINENUM_GET(pMeiDev)));
         break;
      case D2H_CMV_INDICATE:
         DUMP_USR_PRN(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_HIGH, ("NFC   [%02d]: ", MEI_DRV_LINENUM_GET(pMeiDev)));
         break;
      default:
         DUMP_USR_PRN(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_HIGH, ("u 0x%02X [%02d]: ", fctOpCode, MEI_DRV_LINENUM_GET(pMeiDev)));
         break;
   }

   DUMP_USR_PRN(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_HIGH,
       ("%04X - %04X %04X   |  "
         , ( P_CMV_MSGHDR_MSGID_GET(pCmvMsg) |
             ((P_CMV_MSGHDR_FCT_OPCODE_GET(pCmvMsg) & 0x04) ? MEI_MSGID_BIT_IND_WR_CMD : 0x0000) )
         , P_CMV_MSGHDR_INDEX_GET(pCmvMsg)
         , P_CMV_MSGHDR_LENGTH_GET(pCmvMsg) ));


   paylSize = P_CMV_MSGHDR_PAYLOAD_SIZE_GET(pCmvMsg);
   paylSize = (paylSize > CMV_USED_PAYLOAD_16BIT_SIZE) ? CMV_USED_PAYLOAD_16BIT_SIZE : paylSize;

   if (P_CMV_MSGHDR_BIT_SIZE_GET(pCmvMsg) == (unsigned short)CMV_MSG_BIT_SIZE_32BIT)
   {
      paylSize = paylSize >> 1;
      for (i=0; i < paylSize; i++)
      {
         DUMP_USR_PRN(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_HIGH,("%08X ", pCmvMsg->payload.params_32Bit[i]));
      }
   }
   else
   {
      for (i=0; i < paylSize; i++)
      {
         DUMP_USR_PRN(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_HIGH,("%04X ", pCmvMsg->payload.params_16Bit[i]));
      }
   }

   if (MEI_msgDumpSetLabel) {
      DUMP_USR_PRN(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_HIGH, (MEI_DRV_CRLF "---" MEI_DRV_CRLF) ); }
   else {
      DUMP_USR_PRN(MEI_MSG_DUMP, MEI_DRV_PRN_LEVEL_HIGH, (MEI_DRV_CRLF) ); }

   return;
}


/**
   Control the Debug Message Dump

   MDMP_ENABLE:
      0x000000FF
      0x------7F     ---->  enable: 0 off, 1 driver, 2 WE, 3 WH
      0x------8-     ---->  set label on/off
      0x----FF--     ---->  out control, block
                              - 0x01  H2D_CMV_READ
                              - 0x02  D2H_CMV_READ_REPLY
                              - 0x04  H2D_CMV_WRITE
                              - 0x08  D2H_CMV_WRITE_REPLY
                              - 0x10  D2H_CMV_INDICATE
                              - 0x20
      0xFFFF----     ---->  Line filter - 0x8000 filter on
*/
IFX_void_t MEI_DbgMsgDumpCntrl(
                              MEI_DEV_T       *pMeiDev,
                              CMV_STD_MESSAGE_T *pCmvMsg,
                              IFX_boolean_t     bLog)
{
   IFX_uint16_t fctOpCode;

   /* check if the dump is enabled */
   if (!MEI_msgDumpEnable)
      return;

   /* check if we have to trace only this line */
   if (MEI_msgDumpLine & 0x8000)
   {
      if ( (MEI_msgDumpLine & 0x00FF) != MEI_DRV_LINENUM_GET(pMeiDev))
         return;
   }
   else
   {
      /* check for line print enable */
      if ( MEI_DbgCheckForLine((IFX_uint8_t)MEI_DRV_LINENUM_GET(pMeiDev)) != IFX_TRUE)
      {
         return;
      }
   }

   fctOpCode = P_CMV_MSGHDR_FCT_OPCODE_GET(pCmvMsg);

   /* update the functional opcode for further process, thanks to AW */
   switch (pCmvMsg->header.mbxCode)
   {
        /* MEI_MBOX_CODE_MSG_ACK:   */
      case MEI_MBOX_CODE_MSG_WRITE:
         break;

      case MEI_MBOX_CODE_NFC_REQ:
      case MEI_MBOX_CODE_EVT_REQ:
      case MEI_MBOX_CODE_ALM_REQ:
         fctOpCode = D2H_CMV_INDICATE;
         break;
      default:
         break;
   }

   /* check type */
   switch(fctOpCode)
   {
      case H2D_CMV_READ:
         if (MEI_msgDumpOutCntrl & 0x00000001)
         {
            return;
         }
         break;
      case D2H_CMV_READ_REPLY:
         if (MEI_msgDumpOutCntrl & 0x00000002)
         {
            return;
         }

         break;
      case H2D_CMV_WRITE:
         if (MEI_msgDumpOutCntrl & 0x00000004)
         {
            return;
         }
         break;

      case D2H_CMV_WRITE_REPLY:
         if (MEI_msgDumpOutCntrl & 0x00000008)
         {
            return;
         }
         break;
      case D2H_CMV_INDICATE:
         if (MEI_msgDumpOutCntrl & 0x00000010)
         {
            return;
         }

         break;
      default:
         if (MEI_msgDumpOutCntrl & 0x00000020)
         {
            return;
         }
         break;
   }

   switch (MEI_msgDumpEnable)
   {
      case 1:
         /* driver modem message format */
         if (bLog)
            MEI_DbgMsgDumpLogDrv(pMeiDev, pCmvMsg, fctOpCode);
         else
            MEI_DbgMsgDumpTraceDrv(pMeiDev, pCmvMsg, fctOpCode);

         break;
      case 2:
         /* wineasy message format */
         if (bLog)
            MEI_DbgMsgDumpLogWE(pMeiDev, pCmvMsg, fctOpCode);
         else
            MEI_DbgMsgDumpTrcWE(pMeiDev, pCmvMsg, fctOpCode);

         break;
      case 3:
         /* wh message format */
         if (bLog)
            MEI_DbgMsgDumpLogWH(pMeiDev, pCmvMsg, fctOpCode);
         else
            MEI_DbgMsgDumpTrcWH(pMeiDev, pCmvMsg, fctOpCode);

         break;
      default:
         break;
   }

   return;
}

#endif      /* #if ( (MEI_MSG_DUMP_ENABLE == 1) && (MEI_DEBUG_PRINT == 1) ) */


/* ============================================================================
   VRX Driver Debug function - State Changes
   ========================================================================= */

#if MEI_TRACE_DRV_STATE_CHANGES == 1
/**
   Display state changes of the driver.

\remarks
   Only for testing.
*/
IFX_void_t MEI_DrvStateSet_Trace(
                              MEI_DEV_T *pMeiDev,
                              MEI_DRV_STATE_E newState )
{
   PRN_DBG_INT( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL, MEI_DRV_LINENUM_GET(pMeiDev),
        ("MEI_DRV[%d]: --> state change: %d --> %d" MEI_DRV_CRLF,
          MEI_DRV_LINENUM_GET(pMeiDev), pMeiDev->eDevDrvState, newState ));

   pMeiDev->eDevDrvState = newState;

   return;
}     /*  IFX_void_t MEI_DrvStateSet_Trace(...) */


/**
   Display state changes of the modem.

\remarks
   Only for testing.
*/
IFX_void_t MEI_ModemStateSet_Trace(
                              MEI_DEV_T *pMeiDev,
                              IFX_uint32_t  newState )
{
   PRN_DBG_INT( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL, MEI_DRV_LINENUM_GET(pMeiDev),
        ("MEI_DRV[%d]: --> modem state change: %d --> %d" MEI_DRV_CRLF,
          MEI_DRV_LINENUM_GET(pMeiDev), pMeiDev->modemData.modemState, newState ));

   pMeiDev->modemData.modemState = newState;

   return;
}     /*  IFX_void_t MEI_ModemStateSet_Trace(...) */

#endif /* #if MEI_TRACE_DRV_STATE_CHANGES == 1 */

#if MEI_TRACE_MB_STATE_CHANGES == 1
/**
   Display state changes of the Mailbox.

\remarks
   Only for testing.
*/
IFX_void_t MEI_DrvMailBoxStateSet_Trace(
                              MEI_DEV_T *pMeiDev,
                              MEI_MB_STATE_E newState )
{
   PRN_DBG_INT( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL, MEI_DRV_LINENUM_GET(pMeiDev),
        ("MEI_DRV[%d]: --> MB state change: %d --> %d" MEI_DRV_CRLF,
          MEI_DRV_LINENUM_GET(pMeiDev), pMeiDev->modemData.mBoxState, newState ));

   pMeiDev->modemData.mBoxState = newState;

   return;
}
#endif


/* ============================================================================
   VRX Driver Debug function - NFC Handling
   ========================================================================= */
#if (MEI_SUPPORT_DRV_NFC_DEBUG == 1)

/**
   Fill display buffer.
*/
static IFX_int32_t MEI_GetNfcDataPerInstance(
                              MEI_DEV_T    *pMeiDev,
                              MEI_DYN_NFC_DATA_T *pNfcData,
                              IFX_uint8_t    nfcListNum,
                              IFX_uint8_t    *pBuf,
                              IFX_int32_t    bufSize)
{
   IFX_int32_t idx, len = 0;

   if (pBuf)
   {
      if ((len + 90) < bufSize)
      {
         len += sprintf(pBuf + len,
                        "MEI_DRV[%02d]: NFC List[%02d] - rdIdx = %2d, wrIdx = %2d t = %lu\n\r",
                        MEI_DRV_LINENUM_GET(pMeiDev), nfcListNum,
                        pNfcData->rdIdxRd, pNfcData->rdIdxWr,
                        MEI_DRVOS_GetElapsedTime_ms(0));
      }

      else
      {
         goto ERR_MEI_SHOWNFCDATAPERINSTANCE;
      }

      if ((len + 50) < bufSize)
      {
         len += sprintf(pBuf + len, "\tNFC Buffer Length: - max NFC buffers: %d" MEI_DRV_CRLF,
                        pNfcData->numOfBuf);
      }
      else
      {
         goto ERR_MEI_SHOWNFCDATAPERINSTANCE;
      }

      for (idx = 0; idx < pNfcData->numOfBuf; idx ++)
      {
         if ((len + 110) < bufSize)
         {
            len += sprintf(pBuf + len,
                     "\tNfcBuf[%02d] cntrl = 0x%08X len = %3d t = %8d -  "\
                     "MSG: 0x%04X 0x%04X 0x%04X 0x%04X 0x%04X 0x%04X\n\r",
                     idx, pNfcData->pRecvDataCntrl[idx].bufCtrl, pNfcData->pRecvDataCntrl[idx].msgLen,
                     pNfcData->pRecvDataCntrl[idx].recvTime,
                     pNfcData->pRecvDataCntrl[idx].recvDataBuf_s.pBuffer[0],
                     pNfcData->pRecvDataCntrl[idx].recvDataBuf_s.pBuffer[1],
                     pNfcData->pRecvDataCntrl[idx].recvDataBuf_s.pBuffer[2],
                     pNfcData->pRecvDataCntrl[idx].recvDataBuf_s.pBuffer[3],
                     pNfcData->pRecvDataCntrl[idx].recvDataBuf_s.pBuffer[4],
                     pNfcData->pRecvDataCntrl[idx].recvDataBuf_s.pBuffer[6]);
         }
         else
         {
            PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_WRN,
                  ("MEI_DRV[%02d]: NFC List[%02d] - WARNING to small display buffer[%d], written %d" MEI_DRV_CRLF,
                  MEI_DRV_LINENUM_GET(pMeiDev), nfcListNum, bufSize, len) );
            break;
         }
      }

      len += sprintf(pBuf + len, MEI_DRV_CRLF);
   }

   return len;

ERR_MEI_SHOWNFCDATAPERINSTANCE:

   len += sprintf(pBuf + len, MEI_DRV_CRLF);
   PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_WRN,
         ("MEI_DRV[%02d]: NFC List[%02d] - WARNING to small display buffer[%d], written %d" MEI_DRV_CRLF,
         MEI_DRV_LINENUM_GET(pMeiDev), nfcListNum, bufSize, len) );

   return len;
}


/**
   Debug/Test function to verify the NFC structure.

\remark
   To ensure consistent data, the interrupt will be disabled and the
   channel will be locked.

*/
IFX_int32_t MEI_ShowNfcData(
                              MEI_DEV_T *pMeiDev,
                              IFX_uint8_t *pBuf,
                              IFX_int32_t bufSize)
{
   MEI_DYN_NFC_DATA_T *pNfcData = pMeiDev->pRootNfcRecvFirst;
   IFX_int32_t len = 0;
   IFX_uint8_t nfcListNum = 0;

   if (MEI_DRV_STATE_GET(pMeiDev) == e_MEI_DRV_STATE_NOT_INIT)
   {
      return 0;
   }

   MEI_DRV_GET_UNIQUE_ACCESS(pMeiDev);
   /* step through the list of all enabled NFC's */
   while (pNfcData != NULL)
   {
      len += MEI_GetNfcDataPerInstance( pMeiDev, pNfcData, nfcListNum,
                                          &pBuf[len], bufSize - len);
      pNfcData = pNfcData->pNext;
      nfcListNum++;
   }

   MEI_DRV_RELEASE_UNIQUE_ACCESS(pMeiDev);

   return len;
}
#endif      /* MEI_SUPPORT_DRV_NFC_DEBUG */


#if (MEI_MISC_TEST == 1)
/**
   Display the used structure sizes and check it against the defines.

\remarks
   Only for testing.
*/
IFX_void_t MEI_StructureCheck(IFX_void_t)
{
   PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_HIGH,
          ("MEI_DRV: ALL - MEI_MEI_MAILBOX_T     %d (0x%X)" MEI_DRV_CRLF,
            sizeof(MEI_MEI_MAILBOX_T), sizeof(MEI_MEI_MAILBOX_T) ));

   PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_HIGH,
          ("MEI_DRV: MB  - MEI_MEI_MAILBOX_RAW_T   %d (0x%X)" MEI_DRV_CRLF,
            sizeof(MEI_MEI_MAILBOX_RAW_T), sizeof(MEI_MEI_MAILBOX_RAW_T) ));
   PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_HIGH,
          ("MEI_DRV: MB  - MEI_CMV_MAILBOX_T       %d (0x%X)" MEI_DRV_CRLF MEI_DRV_CRLF,
            sizeof(MEI_CMV_MAILBOX_T), sizeof(MEI_CMV_MAILBOX_T) ));

   PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_HIGH,
          ("MEI_DRV: MSG - CMV_STD_MESSAGE_T         %d (0x%X)" MEI_DRV_CRLF,
            sizeof(CMV_STD_MESSAGE_T), sizeof(CMV_STD_MESSAGE_T) ));
   PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_HIGH,
          ("MEI_DRV: MSG - CMV_MESSAGE_MODEM_RDY_T   %d (0x%X)" MEI_DRV_CRLF,
            sizeof(CMV_MESSAGE_MODEM_RDY_T), sizeof(CMV_MESSAGE_MODEM_RDY_T) ));
   PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_HIGH,
          ("MEI_DRV: MSG - CMV_MESSAGE_CS_T          %d (0x%X)" MEI_DRV_CRLF,
            sizeof(CMV_MESSAGE_CS_T), sizeof(CMV_MESSAGE_CS_T) ));
   PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_HIGH,
          ("MEI_DRV: MSG - CMV_MESSAGE_FAST_RD_T     %d (0x%X)" MEI_DRV_CRLF MEI_DRV_CRLF,
            sizeof(CMV_MESSAGE_FAST_RD_T), sizeof(CMV_MESSAGE_FAST_RD_T) ));

   PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_HIGH,
          ("MEI_DRV: STD - CMV_STD_MESSAGE_HEADER_T  %d (0x%X)" MEI_DRV_CRLF,
            CMV_HEADER_8BIT_SIZE, CMV_HEADER_8BIT_SIZE ));

   PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_HIGH,
          ("MEI_DRV: STD - CMV_STD_MESSAGE_PAYLOAD_T %d (0x%X)" MEI_DRV_CRLF MEI_DRV_CRLF,
            sizeof(CMV_STD_MESSAGE_PAYLOAD_T), sizeof(CMV_STD_MESSAGE_PAYLOAD_T) ));
}     /*  MEI_STATIC IFX_void_t MEI_StructureCheck(...) */
#endif   /* MEI_MISC_TEST */

