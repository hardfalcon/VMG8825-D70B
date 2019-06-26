/****************************************************************************

                          Copyright (c) 2007-2015
                     Lantiq Beteiligungs-GmbH & Co. KG

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

*****************************************************************************/
/** \file
   Debug Streams: common defines for the MEI Debug stream handling.
*/

/* ==========================================================================
   includes
   ========================================================================== */
#include "drv_mei_cpe_config.h"

#include "drv_mei_cpe_dbg_streams.h"

#if (MEI_SUPPORT_DEBUG_STREAMS == 1)

#include "ifx_types.h"
#include "drv_mei_cpe_os.h"

#include "drv_mei_cpe_dbg.h"
#include "drv_mei_cpe_dbg_driver.h"

/** get interface and configuration */
#include "drv_mei_cpe_api.h"
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

/* ==========================================================================
   Debug Streams - Macro defs
   ========================================================================== */

#define MEI_DBG_STREAM_DUMP_USR_PRN(name,level,message) \
         /*lint -e{750,717} */ \
         do { \
            if(level >= MEI_PrnUsrModule_##name) \
            { \
               (void)MEI_OS_PRINT_USR_DBG_RAW message ; \
            } \
         } while(0)

/* offset of msg parameter 2 within the data array */
#define MEI_DBG_STREAM_PARAM2_OFF       0

#ifndef MEI_OS_ADDR_ALIGNED_SIZE
#  define MEI_OS_ADDR_ALIGNED_SIZE(esz)    \
         ( (((esz) / sizeof(IFX_addr_t)) + (((esz) % sizeof(IFX_addr_t) > 0) ? 1 : 0)) * sizeof(IFX_addr_t) )
#endif

/* Default buffer size */
#define MEI_DBG_STREAM_DEFAULT_BUFFER_SIZE       100000

/* ==========================================================================
   Debug Streams - Local Function Declarations
   ========================================================================== */

MEI_STATIC IFX_int_t MEI_DbgStreamControlCreate(
                              MEI_DYN_CNTRL_T         *pMeiDynCntrl,
                              IFX_uint_t               bufferSize,
                              MEI_DYN_DBG_STRM_DATA_T **ppInstDynDebugStream);

MEI_STATIC IFX_int_t MEI_DbgStreamControlDelete(
                              MEI_DYN_CNTRL_T    *pMeiDynCntrl);

MEI_STATIC IFX_int_t MEI_DbgStreamOpModeChange(
                              MEI_DYN_CNTRL_T             *pMeiDynCntrl,
                              MEI_DBG_STREAM_BUF_OPMODE_E  eNewOpMode);

MEI_STATIC IFX_int_t MEI_DbgStreamDataGet(
                              MEI_DYN_CNTRL_T                *pMeiDynCntrl,
                              IOCTL_MEI_DEBUG_STREAM_data_t  *pDbgStreamRead,
                              IFX_boolean_t                   bInternCall);

MEI_STATIC IFX_int_t MEI_DbgStreamStartStopFilter(
                              MEI_DEV_T                 *pMeiDev,
                              MEI_DYN_DBG_STRM_DATA_T   *pInstDynDebugStream,
                              CMV_STD_MESSAGE_T          *pMsg,
                              IFX_uint_t                  msgElementCount_16);

MEI_STATIC IFX_int_t MEI_DbgStreamSnapshotMode(
                              MEI_DEV_T                 *pMeiDev,
                              MEI_DYN_DBG_STRM_DATA_T   *pInstDynDebugStream,
                              CMV_STD_MESSAGE_T          *pMsg,
                              IFX_uint_t                  msgElementCount_16);

MEI_STATIC IFX_int_t MEI_DbgStreamSendErrorIndication(
                              MEI_DYN_DBG_STRM_DATA_T    *pInstDynDebugStream);
                              
MEI_STATIC IFX_boolean_t MEI_DbgStreamAddToDevList( 
                                                      MEI_DYN_CNTRL_T         *pMeiDynCntrl,
                                                      MEI_DYN_DBG_STRM_DATA_T *pVrxDynDbgStrm);

MEI_STATIC IFX_boolean_t MEI_DbgStreamRemoveFromDevList( 
                                                       MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                                                       MEI_DYN_DBG_STRM_DATA_T **ppVrxDynDbgStrm);
                                                       
MEI_STATIC IFX_int_t MEI_DbgStreamControl(
                              MEI_DYN_CNTRL_T                         *pMeiDynCntrl,
                              MEI_DYN_DBG_STRM_DATA_T                 *pInstDynDebugStream,
                              IFX_boolean_t                            bEnable);                                                       
                              
MEI_STATIC IFX_int_t MEI_DbgStreamDefaultConfigSet(
                              MEI_DYN_CNTRL_T    *pMeiDynCntrl);                              
                              
MEI_STATIC IFX_uint32_t MEI_DbgStreamSendNotify(
                              MEI_DEV_T          *pMeiDev,
                              MEI_DYN_NFC_DATA_T *pNfcInstance,
                              CMV_STD_MESSAGE_T   *pNotifyMsg);

#if (MEI_DEBUG_PRINT == 1)
MEI_STATIC IFX_void_t MEI_DbgStreamEventDump(
                              MEI_DEV_T                  *pMeiDev,
                              MEI_DYN_DBG_STRM_DATA_T    *pInstDynDebugStream,
                              CMV_STD_MESSAGE_HEADER_T   *pModemMsgHdr,
                              IFX_uint16_t               *pPayloadData,
                              IFX_uint_t                 paylCount_16);
#endif

/* If the IFXOS is not included use internal MEI driver fifo implemantation */
#if (MEI_DRV_IFXOS_ENABLE == 0)

#  define IFXOS_SUPPORTS_FIFO_PEEK 1
#  define TO_ULONG_SIZE(esz)    \
      ((esz)/sizeof(IFX_ulong_t) + ((esz)%sizeof(IFX_ulong_t) > 0))
     
/* Element size header is 1 long int. */
#  define SIZE_HEADER          1
/* Element trailer is 1 long int */
#  ifndef SIZE_TRAILER
#     define SIZE_TRAILER       1
#  endif
#  define SIZE_TRAILER_VALUE   0xDEADBEEF

MEI_STATIC IFX_return_t MEI_DRVOS_Var_Fifo_Init (
                           MEI_DRVOS_VFIFO* pFifo, 
                           IFX_ulong_t* pStart,
                           IFX_ulong_t* pEnd, 
                           IFX_uint32_t size);
MEI_STATIC IFX_void_t   MEI_DRVOS_Var_Fifo_Clear (
                           MEI_DRVOS_VFIFO *pFifo);
MEI_STATIC IFX_ulong_t* MEI_DRVOS_Var_Fifo_readElement (
                           MEI_DRVOS_VFIFO *pFifo, 
                           IFX_uint32_t *elSizeB);
MEI_STATIC IFX_ulong_t* MEI_DRVOS_Var_Fifo_peekElement (
                           MEI_DRVOS_VFIFO *pFifo, 
                           IFX_uint32_t *elSizeB);
MEI_STATIC IFX_ulong_t* MEI_DRVOS_Var_Fifo_writeElement (
                           MEI_DRVOS_VFIFO *pFifo, 
                           IFX_uint32_t elSizeB);
MEI_STATIC IFX_uint32_t MEI_DRVOS_Var_Fifo_getCount (
                           MEI_DRVOS_VFIFO *pFifo);
MEI_STATIC IFX_uint32_t MEI_DRVOS_Var_Fifo_getRoom (
                           MEI_DRVOS_VFIFO *pFifo);
#endif /* (MEI_DRV_IFXOS_ENABLE == 0) */
                              
/* ==========================================================================
   Debug Streams - Global Variables.
   ========================================================================== */
IFX_char_t  *MEI_DbgStreamsOpmodeNames[] =
   {"Default", "FILL", "RING", "FIFO", "Unknown"};

MEI_DRV_PRN_USR_MODULE_CREATE(MEI_DBG_STREAM, MEI_DRV_PRN_LEVEL_HIGH);
MEI_DRV_PRN_INT_MODULE_CREATE(MEI_DBG_STREAM, MEI_DRV_PRN_LEVEL_HIGH);
MEI_DRV_PRN_INT_MODULE_CREATE(MEI_DBG_STREAM_DUMP, MEI_DRV_PRN_LEVEL_OFF);


/* Semaphore to protect:
   - config changes (debug streams) */
MEI_DRVOS_sema_t  MEI_dbgStreamCntrlLock;

/* mark the devices which have debug streams enabled */
IFX_uint8_t   MEI_DRV_dbgStreamsOn[MEI_MAX_SUPPORTED_DFEX_ENTITIES];

/* under Linux, if set try to alloc the fifo buffer as continous buffer (mem block) */
IFX_uint_t    MEI_dbgStreamBlockAlloc = 0;

/* under Linux, if set try to alloc the fifo buffer as continous buffer (mem block) */
IFX_uint_t    MEI_dbgStreamMaxBuffer  = MEI_DBG_STREAMS_MAX_BUFFER;

/**
   Get and process a incoming debug stream event message.

*/
IFX_void_t MEI_DBG_STREAM_EventRecv(
                              MEI_DEV_T               *pMeiDev,
                              MEI_DYN_DBG_STRM_DATA_T *pDbgStrmRootInstance,
                              CMV_STD_MESSAGE_T       *pMsg)
{
   IFX_uint_t                        modemMsgPaylSize_byte = 0;
   MEI_DYN_DBG_STRM_DATA_T           *pDbgStrmInstance_next = IFX_NULL;
   MEI_OS_BUFFER_PTR_T               uFifoBufferWrite, uFifoBufferRead;
#define MEI_DEBUG_STREAM_MESSAGE_HEADER_SIZE_BYTES (sizeof(IFX_uint16_t) * 3)
#define MEI_DEBUG_STREAM_MAX_BUFFER_CAPACITY_IN_MSGS (pDbgStrmInstance_next->dbgStreamFifoBufferSize / \
                                                     (sizeof(EVT_DBG_DebugStream_t) + sizeof(IFX_uint16_t)))
#define MEI_DEBUG_STREAM_BUFFER_NOTIFY_FILL_FACTOR_PERCENTS 35

   /* There is a version dependent CRC in the data => check for the current version */
   modemMsgPaylSize_byte = (pMsg->header.length << 1) + MEI_DEBUG_STREAM_MESSAGE_HEADER_SIZE_BYTES;

   pDbgStrmInstance_next = pDbgStrmRootInstance;
   while(pDbgStrmInstance_next)
   {
      /* Check for the debug streams are enabled */
      if (pDbgStrmInstance_next->control != 1)
      {
         /* If the debug stream is not enabled, don't process the messages. */
         goto MEI_DBG_STREAM_MAILBOX_EVT_RECV_EXIT;
      }

      if ( (modemMsgPaylSize_byte == 0)    ||
           (modemMsgPaylSize_byte &  0x01) ||
           (modemMsgPaylSize_byte > (CMV_USED_PAYLOAD_8BIT_SIZE + MEI_DEBUG_STREAM_MESSAGE_HEADER_SIZE_BYTES)) )
      {
         /* invalid payload */
         PRN_ERR_INT_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
                         ("MEI_DBG_STREAM[%02d]: ERROR - evt recv, invalid payload (size = %d)!" MEI_DRV_CRLF,
                          MEI_DRV_LINENUM_GET(pMeiDev), modemMsgPaylSize_byte));

         MEI_DBG_STREAMS_STAT_MSG_CORRUPTED_CNT_INC(pDbgStrmInstance_next);

         goto MEI_DBG_STREAM_MAILBOX_EVT_RECV_EXIT;
      }

      /* Debug stream message filter */
      if (pDbgStrmInstance_next->eFilterMode == e_MEI_DBG_STREAM_START_STOP)
      {
         if (MEI_DbgStreamStartStopFilter(
                                          pMeiDev,
                                          pDbgStrmInstance_next,
                                          pMsg,
                                          modemMsgPaylSize_byte / sizeof(IFX_uint16_t)) != IFX_SUCCESS)
         {
            PRN_ERR_INT_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
                            ("MEI_DBG_STREAM[%02d]: WARNING - evt recv, debug stream "
                             "event start/stop filter error!" MEI_DRV_CRLF,
                             MEI_DRV_LINENUM_GET(pMeiDev)));
            goto MEI_DBG_STREAM_MAILBOX_EVT_RECV_EXIT;
         }
      }
      else
      {
         if (MEI_DbgStreamSnapshotMode(
                                       pMeiDev,
                                       pDbgStrmInstance_next,
                                       pMsg,
                                       modemMsgPaylSize_byte / sizeof(IFX_uint16_t)) != IFX_SUCCESS)
         {
            PRN_ERR_INT_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
                            ("MEI_DBG_STREAM[%02d]: WARNING - evt recv, debug stream "
                             "event snapshot filter error!" MEI_DRV_CRLF,
                             MEI_DRV_LINENUM_GET(pMeiDev)));
            goto MEI_DBG_STREAM_MAILBOX_EVT_RECV_EXIT;
         }
      }

      if (pDbgStrmInstance_next->eFilterState ==
          eMEI_DbgStreamFilterState_SkipThis)
      {
         /* Skip the filtered message */
         goto MEI_DBG_STREAM_MAILBOX_EVT_RECV_EXIT;
      }
      else if (pDbgStrmInstance_next->eFilterState ==
               eMEI_DbgStreamFilterState_SkipNext)
      {
         /* Save the last non-skipped messages and set state to 'skip'
            to skip the next messages  */
         pDbgStrmInstance_next->eFilterState =
            eMEI_DbgStreamFilterState_SkipThis;
      }
      else
      {
         /* Save the messages to the buffer */
      }

      MEI_DBG_STREAMS_STAT_MSG_D2H_CNT_INC(pDbgStrmInstance_next);
      MEI_DBG_STREAMS_STAT_DATA_D2H_SIZE_ADD(pDbgStrmInstance_next, modemMsgPaylSize_byte);

      if ( (pDbgStrmInstance_next->eOpMode           == e_MEI_DBG_STREAM_FIFO) &&
           (pDbgStrmInstance_next->bOverflowOccurred == eMEI_DbgStreamOverflowState_Pending) )
      {
         /*
            Overflow occurred - try to send error indication
            */
         if (MEI_DbgStreamSendErrorIndication(pDbgStrmInstance_next) != IFX_SUCCESS)
         {

            MEI_DBG_STREAMS_STAT_MSG_BFULL_DISCARD_CNT_INC(pDbgStrmInstance_next);
            MEI_DBG_STREAMS_STAT_DATA_BFULL_DISCARD_SIZE_ADD(pDbgStrmInstance_next, modemMsgPaylSize_byte);

            goto MEI_DBG_STREAM_MAILBOX_EVT_RECV_EXIT;
         }
      }

      uFifoBufferWrite.pULong = MEI_DRVOS_Var_Fifo_writeElement(
                                                                &pDbgStrmInstance_next->dbgStreamFifo,
                                                                MEI_DBG_STREAMS_CAST(IFX_uint32_t, modemMsgPaylSize_byte) );

      if (uFifoBufferWrite.pVoid == IFX_NULL)
      {
         IFX_uint_t  readSize = 0, readSize_all = 0;

         if ( (pDbgStrmInstance_next->bReadPending == IFX_FALSE) &&
              (pDbgStrmInstance_next->eOpMode      == e_MEI_DBG_STREAM_USER_RING) )
         {
            while(readSize_all < modemMsgPaylSize_byte)
            {
               /* ring buffer operation mode - read msg's to free FIFO space */
               uFifoBufferRead.pULong = MEI_DRVOS_Var_Fifo_readElement(&pDbgStrmInstance_next->dbgStreamFifo, &readSize);

               if ( (uFifoBufferRead.pVoid == IFX_NULL) || (readSize == 0) )
               {
                  /* ERROR - cannot read from FIFO while data write fails */
                  PRN_ERR_INT_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
                                  ("MEI_DBG_STREAM[%02d]: ERROR - evt recv, free ring buffer failes!" MEI_DRV_CRLF,
                                   MEI_DRV_LINENUM_GET(pMeiDev) ));

                  break;
               }

               MEI_DBG_STREAMS_STAT_MSG_RBUFFER_OVERWR_CNT_INC(pDbgStrmInstance_next);
               MEI_DBG_STREAMS_STAT_DATA_RBUFFER_OVERWR_SIZE_ADD(pDbgStrmInstance_next, readSize);

               readSize_all += readSize;
            }

            if (readSize_all >= modemMsgPaylSize_byte)
            {
               uFifoBufferWrite.pULong = MEI_DRVOS_Var_Fifo_writeElement(
                                                                         &pDbgStrmInstance_next->dbgStreamFifo,
                                                                         MEI_DBG_STREAMS_CAST(IFX_uint32_t, modemMsgPaylSize_byte) );
            }
            else
            {
               /* ERROR - cannot read from FIFO while data write fails */
               MEI_DBG_STREAMS_STAT_MSG_ERR_DISCARD_CNT_INC(pDbgStrmInstance_next);
               MEI_DBG_STREAMS_STAT_DATA_ERR_DISCARD_SIZE_ADD(pDbgStrmInstance_next, modemMsgPaylSize_byte);

               goto MEI_DBG_STREAM_MAILBOX_EVT_RECV_EXIT;
            }
         }
      }

      if (uFifoBufferWrite.pULong == IFX_NULL)
      {
         if ( (pDbgStrmInstance_next->eOpMode == e_MEI_DBG_STREAM_FIFO) &&
              (pDbgStrmInstance_next->bOverflowOccurred == eMEI_DbgStreamOverflowState_Clear) )
         {
            /*
               Signal overflow - if FIFO mode is used, send error indication
               */
            pDbgStrmInstance_next->bOverflowOccurred = eMEI_DbgStreamOverflowState_Pending;
            pDbgStrmInstance_next->overflowTime_ms   = MEI_DRVOS_GetElapsedTime_ms(pDbgStrmInstance_next->startTime_ms);
         }

         /* no space */
         MEI_DBG_STREAMS_STAT_MSG_BFULL_DISCARD_CNT_INC(pDbgStrmInstance_next);
         MEI_DBG_STREAMS_STAT_DATA_BFULL_DISCARD_SIZE_ADD(pDbgStrmInstance_next, modemMsgPaylSize_byte);

         goto MEI_DBG_STREAM_MAILBOX_EVT_RECV_EXIT;
      }
      else
      {
         /*
            able to get the stream - clear overflow indication
            */
         pDbgStrmInstance_next->bOverflowOccurred = eMEI_DbgStreamOverflowState_Clear;

         /* Copy the debug stream data payload to the buffer */
         memcpy(uFifoBufferWrite.pULong, &pMsg->header.MessageID, modemMsgPaylSize_byte);

         MEI_DBG_STREAMS_STAT_MSG_H2BUF_CNT_INC(pDbgStrmInstance_next);
         MEI_DBG_STREAMS_STAT_DATA_H2BUF_SIZE_ADD(pDbgStrmInstance_next, modemMsgPaylSize_byte);
#if (MEI_DEBUG_PRINT == 1)
         if (MEI_PrnIntModule_MEI_DBG_STREAM_DUMP < MEI_DRV_PRN_LEVEL_OFF)
         {
            MEI_DbgStreamEventDump(pMeiDev, pDbgStrmInstance_next, &pMsg->header,
                                   uFifoBufferWrite.pUInt16, modemMsgPaylSize_byte >> 1);
         }
#endif
      }

      /* Notify data availability if the notification is enabled 
         within configuration and it is neeeded this time */
      if ((pDbgStrmInstance_next->pDynNfcDbgEvtRecieved != IFX_NULL) &&
          (pDbgStrmInstance_next->eFilterMode == e_MEI_DBG_STREAM_START_STOP))
      {
         if (pDbgStrmInstance_next->bNotify == IFX_TRUE)
         {
            MEI_DbgStreamSendNotify(
                                    pMeiDev,
                                    (MEI_DYN_NFC_DATA_T*)
                                    pDbgStrmInstance_next->pDynNfcDbgEvtRecieved,
                                    pMsg);
         }

         if (MEI_DRVOS_Var_Fifo_getCount(
                                         &pDbgStrmInstance_next->dbgStreamFifo) >=
             ((MEI_DEBUG_STREAM_MAX_BUFFER_CAPACITY_IN_MSGS *
               MEI_DEBUG_STREAM_BUFFER_NOTIFY_FILL_FACTOR_PERCENTS) /100))
         {
            pDbgStrmInstance_next->bNotify = IFX_TRUE;
         }
         else
         {
            pDbgStrmInstance_next->bNotify = IFX_FALSE;
         }
      }

      /* get next */
      pDbgStrmInstance_next = pDbgStrmInstance_next->pNext;
   }
   
MEI_DBG_STREAM_MAILBOX_EVT_RECV_EXIT:

   return;
}

/**
   Detect the debug stream message corresponding to the filter configuration.
*/
MEI_STATIC IFX_int_t MEI_DbgStreamStartStopFilter(
                              MEI_DEV_T                 *pMeiDev,
                              MEI_DYN_DBG_STRM_DATA_T   *pInstDynDebugStream,
                              CMV_STD_MESSAGE_T         *pMsg,
                              IFX_uint_t                 msgElementCount_16)
{
   IFX_uint16_t *pStreamId = IFX_NULL;
   IFX_uint16_t *pData16 = IFX_NULL;
   IFX_uint16_t i = 0;
   IFX_uint8_t nOffset = 0;
   IFX_boolean_t bStartPatternCheck = IFX_FALSE,
                 bStopPatternCheck = IFX_FALSE,
                 bPatternConformity = IFX_FALSE;
   MEI_DbgStreamFilterState_e ePrevState;

   if (pMeiDev == IFX_NULL)
   {
      PRN_ERR_INT_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DBG_STREAM: ERROR - device is not defined!"
            MEI_DRV_CRLF));

      return IFX_ERROR;
   }

   if (pMsg != IFX_NULL)
   {
      pStreamId = &pMsg->payload.params_16Bit[1];
   }
   else
   {
      PRN_ERR_INT_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DBG_STREAM[%02d]: ERROR - message is not defined!"
            MEI_DRV_CRLF, MEI_DRV_LINENUM_GET(pMeiDev)));

      return IFX_ERROR;
   }

   if (pInstDynDebugStream != IFX_NULL)
   {
      ePrevState = pInstDynDebugStream->eFilterState;
   }
   else
   {
      PRN_ERR_INT_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DBG_STREAM[%02d]: ERROR - instance is not defined!"
            MEI_DRV_CRLF, MEI_DRV_LINENUM_GET(pMeiDev)));

      return IFX_ERROR;
   }

   /*
         Debug Stream Data Fromat Example:
  Protocol version number
      |        StreamStatus TxSymCount StreamCrc Data
      4002     C051         DB00 0004  0000      D502 04DA 0400 0011 ....
       | |                                         ||
     Stream ID                                   Event ID
   */

   /* There is a version dependent CRC in the data => check for the current version */
   nOffset = (((*pStreamId & 0xF000) >> 12) < 3) ? 4 : 5;
   
   if (msgElementCount_16 < nOffset)
   {
      PRN_ERR_INT_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DBG_STREAM[%02d]: ERROR - evt filter, Wrong payload length!" MEI_DRV_CRLF,
              MEI_DRV_LINENUM_GET(pMeiDev)));

      return IFX_ERROR;   
   }

   pData16 = &pMsg->payload.params_16Bit[nOffset + 1];

   /* Skip the data pattern checks if the start mask = 0 */
   for (i = 0; i < MEI_DBG_STREAM_MAX_COMPARE_SIZE_IN_WORDS; i++)
   {
      if (pInstDynDebugStream->startMask[i] != 0)
      {
         bStartPatternCheck = IFX_TRUE;
         break;
      }
   }
    
   /* Skip the data pattern checks if the stop mask = 0 */
   for (i = 0; i < MEI_DBG_STREAM_MAX_COMPARE_SIZE_IN_WORDS; i++)
   {                  
      if (pInstDynDebugStream->stopMask[i] != 0)
      {
         bStopPatternCheck = IFX_TRUE;
         break;
      }
   }
   
   if ((bStartPatternCheck == IFX_FALSE) && (bStopPatternCheck == IFX_FALSE) &&
       (pInstDynDebugStream->startEventId == 0) && (pInstDynDebugStream->stopEventId == 0) &&
       (pInstDynDebugStream->startStreamId == 0) && (pInstDynDebugStream->stopStreamId == 0))      
   {
      /* Skip the filtering procedure and process all events if the filter is disabled */
      pInstDynDebugStream->eFilterState = eMEI_DbgStreamFilterState_Store;
      return IFX_SUCCESS;
   }   

   /* EventId handling */
   if ((*pStreamId & 0x0FFF) == 2)
   {
      /*
         StreamId == 2 => EventId handling:
         StreamId 2 is a special case. The FW collects multiple state change events into one DbgStream
         message of type 2. Those information is stored into the payload data of the message and needs 
         to be analyzed for a match of EventId. EventId is only of relevance with StreamId = 2, otherwise 
         it can be ignored. It does not make much sense to combine EventId and DataPattern, but it shall 
         not exclude each other. If StreamId = 2 and EventId != 0 then analyze the whole messages of type 
         2 for a match of the EventId.   
      */

      /* 0xFF are fill bytes and ignored */
      if ((pData16[0] & 0x00FF) != 0x00FF)
      {
         /* The first byte of the debug data (GHS/SOC message) is the LS Byte of the first 16-bit word.
            StreamIds in 0..127 are possible. The highest Bit of the event id indicates an Overflow in the Eventbuffer
            => apply 0x007F mask to get LSB => EventId */
         if (pInstDynDebugStream->eFilterState ==
             eMEI_DbgStreamFilterState_SkipThis)
         {
            if ((pInstDynDebugStream->startEventId != 0) &&
                 (pInstDynDebugStream->startEventId == (pData16[0] & 0x007F)))
            {
               pInstDynDebugStream->eFilterState =
                  eMEI_DbgStreamFilterState_Store;
               goto MEI_DBG_STREAM_FILTER_EXIT;
            }
         }
         else
         {
            if ((pInstDynDebugStream->stopEventId != 0) &&
               (pInstDynDebugStream->stopEventId == (pData16[0] & 0x007F)))
            {
               pInstDynDebugStream->eFilterState =
                  eMEI_DbgStreamFilterState_SkipNext;
               goto MEI_DBG_STREAM_FILTER_EXIT;
            }
         }         
      }
   }

   if (pInstDynDebugStream->eFilterState == eMEI_DbgStreamFilterState_SkipThis)
   {
      if ((*pStreamId & 0x0FFF) == pInstDynDebugStream->startStreamId)
      {
         /* start debug stream event storing (Stream Id criterion conformance) */
         pInstDynDebugStream->eFilterState = eMEI_DbgStreamFilterState_Store;
         goto MEI_DBG_STREAM_FILTER_EXIT;
      }
   }
   else
   {
      if ((*pStreamId & 0x0FFF) == pInstDynDebugStream->stopStreamId)
      {
         /* stop debug stream event storing (Stream Id criterion conformance) */
         pInstDynDebugStream->eFilterState = eMEI_DbgStreamFilterState_SkipNext;
         goto MEI_DBG_STREAM_FILTER_EXIT;
      }
   }
   
   if (pInstDynDebugStream->eFilterState == eMEI_DbgStreamFilterState_SkipThis)
   {
      if (bStartPatternCheck == IFX_TRUE)
      {
         /* Check for the data payload conformance criterion to start message storing */
         for (i = 0; (i < (msgElementCount_16 - nOffset)) && 
                     (i < MEI_DBG_STREAM_MAX_COMPARE_SIZE_IN_WORDS); i++)
         {
            /* Check for the first words of the payload conforms 
               to the filter start criterion  */
            if ((pInstDynDebugStream->startMask[i] != 0) &&
                ((pInstDynDebugStream->startMask[i] & pData16[i]) !=
                 (pInstDynDebugStream->startMask[i] & pInstDynDebugStream->startPattern[i])))
            {
               /* keep skipping the messages */
               bPatternConformity = IFX_FALSE;
               break;
            }
            else if (pInstDynDebugStream->startMask[i] == 0)
            {
               /* pattern conformity state is not changed */
            }
            else
            {
               bPatternConformity = IFX_TRUE;
            }
         }
         if (bPatternConformity == IFX_TRUE)
         {
            /* stop skipping the messages */
            pInstDynDebugStream->eFilterState = eMEI_DbgStreamFilterState_Store;
            goto MEI_DBG_STREAM_FILTER_EXIT;
         }
      }
   }
   else
   {
      if (bStopPatternCheck == IFX_TRUE)
      {
         /* Check for the data payload conformance criterion to stop message storing */
         for (i = 0; (i < (msgElementCount_16 - nOffset)) && 
                     (i < MEI_DBG_STREAM_MAX_COMPARE_SIZE_IN_WORDS); i++)
         {
            /* Check for the first words of the payload conforms 
               to the filter stop criterion  */
            if ((pInstDynDebugStream->stopMask[i] != 0) &&
                ((pInstDynDebugStream->stopMask[i] & pData16[i]) !=
                 (pInstDynDebugStream->stopMask[i] & pInstDynDebugStream->stopPattern[i])))
            {
               /* stop skipping the messages */
               bPatternConformity = IFX_FALSE;
               break;
            }
            else if (pInstDynDebugStream->stopMask[i] == 0)
            {
               /* pattern conformity state is not changed */
            }
            else
            {
               bPatternConformity = IFX_TRUE;
            }
         }
         if (bPatternConformity == IFX_TRUE)
         {
            /* start skipping the messages */
            pInstDynDebugStream->eFilterState = eMEI_DbgStreamFilterState_SkipNext;
            goto MEI_DBG_STREAM_FILTER_EXIT;
         }
      }
   }

MEI_DBG_STREAM_FILTER_EXIT:

   if ((ePrevState != pInstDynDebugStream->eFilterState) &&
       (ePrevState == eMEI_DbgStreamFilterState_Store))
   {
      /* Notify the upper layer about the stop trigger is hit */
      pInstDynDebugStream->bNotify = IFX_TRUE;
   }

   return IFX_SUCCESS;
}

/**
   Detect the debug stream message corresponding to the snapshot mode
   configuration.
*/
MEI_STATIC IFX_int_t MEI_DbgStreamSnapshotMode(
                              MEI_DEV_T                 *pMeiDev,
                              MEI_DYN_DBG_STRM_DATA_T   *pInstDynDebugStream,
                              CMV_STD_MESSAGE_T         *pMsg,
                              IFX_uint_t                 msgElementCount_16)
{
   IFX_int_t ret = IFX_SUCCESS;
   IFX_uint16_t *pStreamId = &pMsg->payload.params_16Bit[1];
   IFX_uint16_t *pData16 = IFX_NULL;
   IFX_uint16_t i = 0;
   IFX_uint8_t nOffset = 0;

   /*
         Debug Stream Data Fromat Example:
  Protocol version number
      |        StreamStatus TxSymCount StreamCrc Data
      4002     C051         DB00 0004  0000      D502 04DA 0400 0011 ....
       | |                                         ||
     Stream ID                                   Event ID
   */

   /* There is a version dependent CRC in the data => check for the current version */
   nOffset = (((*pStreamId & 0xF000) >> 12) < 3) ? 4 : 5;
   
   if (msgElementCount_16 < nOffset)
   {
      PRN_ERR_INT_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DBG_STREAM[%02d]: ERROR - evt filter, Wrong payload length!" MEI_DRV_CRLF,
              MEI_DRV_LINENUM_GET(pMeiDev)));

      return IFX_ERROR;   
   }

   pData16 = &pMsg->payload.params_16Bit[nOffset + 1];

   if ((pInstDynDebugStream->startStreamId != 0) &&
       (pInstDynDebugStream->startStreamId != (*pStreamId & 0x0FFF)))
   {
      goto MEI_DBG_STREAM_SNAPSHOT_EXIT;
   }

   if ((pInstDynDebugStream->startEventId != 0) &&
      (pInstDynDebugStream->startEventId != (pData16[0] & 0x007F)))
   {
      goto MEI_DBG_STREAM_SNAPSHOT_EXIT;
   }   
   
   for (i = 0; (i < (msgElementCount_16 - nOffset)) && 
               (i < MEI_DBG_STREAM_MAX_COMPARE_SIZE_IN_WORDS); i++)
   {
      /* Check for the first words of the payload conforms
         to the filter start criterion */
      if ((pInstDynDebugStream->startMask[i] != 0) &&
          ((pInstDynDebugStream->startMask[i] &
            pData16[i]) !=
           (pInstDynDebugStream->startMask[i] &
            pInstDynDebugStream->startPattern[i])))
      {
         goto MEI_DBG_STREAM_SNAPSHOT_EXIT;
      }
   }

   /* Notify the upper layer about the snapshot is ready */
   MEI_DbgStreamSendNotify(
      pMeiDev,
      (MEI_DYN_NFC_DATA_T*)pInstDynDebugStream->pDynNfcDbgEvtRecieved,
      pMsg);

   /* Skip all the next messages if the necessary message is found */
   pInstDynDebugStream->eFilterState = eMEI_DbgStreamFilterState_SkipNext;

MEI_DBG_STREAM_SNAPSHOT_EXIT:

   return ret;
}

/**
   Try to send an error indication after an overflow has been occured.
*/
MEI_STATIC IFX_int_t MEI_DbgStreamSendErrorIndication(
                              MEI_DYN_DBG_STRM_DATA_T    *pInstDynDebugStream)
{
   IFX_uint32_t  errMsgSize_byte = sizeof(IOCTL_MEI_DBGSTREAM_error_message_t);
   IOCTL_MEI_DBGSTREAM_error_message_t *pErrMsg;
   MEI_OS_BUFFER_PTR_T                  uFifoBufferWrite;

   uFifoBufferWrite.pULong = MEI_DRVOS_Var_Fifo_writeElement(&pInstDynDebugStream->dbgStreamFifo, errMsgSize_byte);

   pErrMsg = (IOCTL_MEI_DBGSTREAM_error_message_t *)MEI_DBG_STREAMS_PTR_CAST_GET_ULONG(uFifoBufferWrite);
   if (pErrMsg == IFX_NULL)
   {
      /* still no space */
      return IFX_ERROR;
   }

   pErrMsg->msgId = MEI_DBG_STREAMS_CAST(unsigned short, MEI_DBG_STREAM_FIFO_MODE_ERROR_MSGID);

   /* payload data - currently only a overwrite indication */
   pErrMsg->msgSize_byte = 4;
   pErrMsg->data[0]      = MEI_DBG_STREAMS_CAST(unsigned int, pInstDynDebugStream->overflowTime_ms);

   MEI_DBG_STREAMS_STAT_BUF_OVERFLOW_CNT_INC(pInstDynDebugStream);

   pInstDynDebugStream->overflowTime_ms = 0;
   pInstDynDebugStream->bOverflowOccurred = eMEI_DbgStreamOverflowState_Processed;

   return IFX_SUCCESS;
}

#if (MEI_DEBUG_PRINT == 1)
MEI_STATIC IFX_void_t MEI_DbgStreamEventDump(
                              MEI_DEV_T                  *pMeiDev,
                              MEI_DYN_DBG_STRM_DATA_T    *pInstDynDebugStream,
                              CMV_STD_MESSAGE_HEADER_T   *pModemMsgHdr,
                              IFX_uint16_t               *pPayloadData,
                              IFX_uint_t                  paylCount_16)

{
   IFX_uint_t i;

   if (paylCount_16 > CMV_USED_PAYLOAD_16BIT_SIZE)
   {
      paylCount_16 = CMV_USED_PAYLOAD_16BIT_SIZE;
   }

   PRN_ERR_INT_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_HIGH,
         ("DBG_STR[%02d], Cnt = %03d: %04X - %04X %04X   |  %04X ",
            MEI_DRV_LINENUM_GET(pMeiDev), paylCount_16,
            pModemMsgHdr->MessageID, pModemMsgHdr->index, pModemMsgHdr->length,
            pPayloadData[0] ));

   for (i=1; i < paylCount_16; i++)
   {
      PRN_ERR_INT_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL,      
         ("%04X ", pPayloadData[i]));
   }

   PRN_ERR_INT_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_HIGH,
         (MEI_DRV_CRLF));
   return;
}
#endif

/* ==========================================================================
   Debug Streams - Local Functions.
   ========================================================================== */

/**
   Allocate resources and create context for the debug stream handling.

\param
   pMeiDynCntrl     - points to the MEI driver port struct.
\param
   bufferSize     - size of the internal stream buffer.
\param
   ppInstDynDebugStream - return pointer, returns generated stream buffer control sturct.

\return
   IFX_SUCCESS and returns the created stream buffer context, else
   IFX_ERROR or an error code (< 0).

*/
MEI_STATIC IFX_int_t MEI_DbgStreamControlCreate(
                              MEI_DYN_CNTRL_T         *pMeiDynCntrl,
                              IFX_uint_t               bufferSize,
                              MEI_DYN_DBG_STRM_DATA_T **ppInstDynDebugStream)
{
   IFX_int_t               retVal = IFX_SUCCESS;
   IFX_uint_t              dbgStreamSize_byte;
   IFX_ulong_t             *pFifoBuffer;
   MEI_DYN_DBG_STRM_DATA_T  *pInstDynDebugStream = pMeiDynCntrl->pInstDynDebugStream;
   MEI_OS_BUFFER_PTR_T     uMemCtx, uMemBuffer;

   if (bufferSize == 0)
   {
      PRN_ERR_USR_NL( MEI_DBG_STREAM, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DBG_STREAM[%02d]: ERROR - ControlCreate, zero buffer defined!"
            MEI_DRV_CRLF, MEI_DRV_LINENUM_GET(pMeiDynCntrl->pMeiDev)));

      return -e_MEI_ERR_INVAL_CONFIG;
   }

   if (pInstDynDebugStream != IFX_NULL)
   {
      PRN_ERR_USR_NL( MEI_DBG_STREAM, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DBG_STREAM[%02d]: ERROR - ControlCreate, already exists!" MEI_DRV_CRLF,
              MEI_DRV_LINENUM_GET(pMeiDynCntrl->pMeiDev)));

      return -e_MEI_ERR_ALREADY_DONE;
   }

   if ((MEI_OS_ADDR_ALIGNED_SIZE(bufferSize)) > MEI_dbgStreamMaxBuffer)
   {
      PRN_ERR_USR_NL( MEI_DBG_STREAM, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DBG_STREAM[%02d]: ERROR - ControlCreate, invalid buffer size %d (max %d)!" MEI_DRV_CRLF,
              MEI_DRV_LINENUM_GET(pMeiDynCntrl->pMeiDev),
              MEI_OS_ADDR_ALIGNED_SIZE(bufferSize), MEI_dbgStreamMaxBuffer));

      return -e_MEI_ERR_NO_MEM;
   }

   dbgStreamSize_byte =
      MEI_OS_ADDR_ALIGNED_SIZE(sizeof(MEI_DYN_DBG_STRM_DATA_T)) + MEI_OS_ADDR_ALIGNED_SIZE(bufferSize);

   uMemCtx.pUInt8 = MEI_DRVOS_VirtMalloc(dbgStreamSize_byte);

   if (uMemCtx.pUInt8 == IFX_NULL)
   {
      PRN_ERR_USR_NL( MEI_DBG_STREAM, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DBG_STREAM[%02d]: ERROR - ControlCreate, mem alloc (struct = %d + buffer = %d [byte])!" MEI_DRV_CRLF,
              MEI_DRV_LINENUM_GET(pMeiDynCntrl->pMeiDev),
              sizeof(MEI_DYN_DBG_STRM_DATA_T), bufferSize));

      return -e_MEI_ERR_NO_MEM;
   }

   memset(uMemCtx.pVoid, 0x00, MEI_DBG_STREAMS_CAST(IFX_int_t, dbgStreamSize_byte));

   /* context */
   if ((pInstDynDebugStream = (MEI_DYN_DBG_STRM_DATA_T *)MEI_DBG_STREAMS_PTR_CAST_GET_ULONG(uMemCtx)) == IFX_NULL)
   {
      PRN_ERR_USR_NL( MEI_DBG_STREAM, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DBG_STREAM[%02d]: ERROR - ControlCreate, struct missaligned!" MEI_DRV_CRLF,
              MEI_DRV_LINENUM_GET(pMeiDynCntrl->pMeiDev)));

      retVal = -e_MEI_ERR_OP_FAILED;
      goto MEI_DBGSTREAM_CONTROL_CREATE_ERROR;
   }

   /* fifo setup */
   uMemBuffer.pAddr =
      uMemCtx.pAddr + (MEI_OS_ADDR_ALIGNED_SIZE(sizeof(MEI_DYN_DBG_STRM_DATA_T)) / sizeof(IFX_int_t));

   if ((pFifoBuffer = MEI_DBG_STREAMS_PTR_CAST_GET_ULONG(uMemBuffer)) == IFX_NULL)
   {
      PRN_ERR_USR_NL( MEI_DBG_STREAM, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DBG_STREAM[%02d]: ERROR - ControlCreate, buffer missaligned!" MEI_DRV_CRLF,
              MEI_DRV_LINENUM_GET(pMeiDynCntrl->pMeiDev)));

      retVal = -e_MEI_ERR_OP_FAILED;
      goto MEI_DBGSTREAM_CONTROL_CREATE_ERROR;
   }

   uMemBuffer.pAddr += (MEI_OS_ADDR_ALIGNED_SIZE(bufferSize) / sizeof(IFX_addr_t));
   if (MEI_DRVOS_Var_Fifo_Init(
               &pInstDynDebugStream->dbgStreamFifo,
               pFifoBuffer, uMemBuffer.pAddr,
               sizeof(EVT_DBG_DebugStream_t)) != IFX_SUCCESS)
   {
      PRN_ERR_USR_NL( MEI_DBG_STREAM, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DBG_STREAM[%02d]: ERROR - ControlCreate, FIFO Init!" MEI_DRV_CRLF,
              MEI_DRV_LINENUM_GET(pMeiDynCntrl->pMeiDev)));

      retVal = -e_MEI_ERR_OP_FAILED;
      goto MEI_DBGSTREAM_CONTROL_CREATE_ERROR;
   }

   pInstDynDebugStream->dbgStreamFifoBufferSize = MEI_OS_ADDR_ALIGNED_SIZE(bufferSize);
   pInstDynDebugStream->eFilterState = eMEI_DbgStreamFilterState_Store;
   pInstDynDebugStream->bNotify      = IFX_FALSE;   
   *ppInstDynDebugStream = pInstDynDebugStream;

   /* first: protect against other users */
   MEI_DRV_GET_UNIQUE_ACCESS(pMeiDynCntrl->pMeiDev);

   MEI_DbgStreamAddToDevList(pMeiDynCntrl, pInstDynDebugStream);

   MEI_DRV_RELEASE_UNIQUE_ACCESS(pMeiDynCntrl->pMeiDev);
   

   return IFX_SUCCESS;

MEI_DBGSTREAM_CONTROL_CREATE_ERROR:

   MEI_DRVOS_VirtFree(uMemCtx.pUInt8);
   *ppInstDynDebugStream = IFX_NULL;

   return retVal;
}

/**
   Stop the debug streams handling for this port and release resources.

   Therefore the access is protected against
   - other tasks and
   - device access (interrupt handling)


*/
MEI_STATIC IFX_int_t MEI_DbgStreamControlDelete(
                              MEI_DYN_CNTRL_T    *pMeiDynCntrl)
{
   MEI_DYN_DBG_STRM_DATA_T *pInstDynDebugStream = IFX_NULL;
   MEI_DEV_T  *pMeiDev = pMeiDynCntrl->pMeiDev;   

   /* get unique access */

   MEI_DRV_GET_UNIQUE_MAILBOX_ACCESS(pMeiDev);

   if (pMeiDynCntrl->pInstDynDebugStream != IFX_NULL)
   {
      MEI_DbgStreamRemoveFromDevList(pMeiDynCntrl, &pInstDynDebugStream);

      MEI_DRVOS_Var_Fifo_Clear(&pInstDynDebugStream->dbgStreamFifo);
      MEI_DRVOS_VirtFree(pInstDynDebugStream);
   }

#if (MEI_DBG_STREAMS_SUPPORT_STATISTICS == 1)
   memset(&pInstDynDebugStream->dbgStreamStatistics, 0x00, sizeof(MEI_DbgStreamStatistic_t));
#endif

   MEI_DRV_RELEASE_UNIQUE_MAILBOX_ACCESS(pMeiDev);

   return IFX_SUCCESS;
}


/**
   Change the operation mode for the debug streams handling on this port.
*/
MEI_STATIC IFX_int_t MEI_DbgStreamOpModeChange(
                              MEI_DYN_CNTRL_T             *pMeiDynCntrl,
                              MEI_DBG_STREAM_BUF_OPMODE_E  eNewOpMode)
{
#if (MEI_DEBUG_PRINT == 1)
   MEI_DBG_STREAM_BUF_OPMODE_E eOldOpMode = e_MEI_DBG_STREAM_LAST;
#endif
   MEI_DEV_T  *pMeiDev = pMeiDynCntrl->pMeiDev;
   MEI_DYN_DBG_STRM_DATA_T *pInstDynDebugStream = pMeiDynCntrl->pInstDynDebugStream;

   if (pInstDynDebugStream == IFX_NULL)
   {
      PRN_ERR_USR_NL( MEI_DBG_STREAM, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DBG_STREAM[%02d]: ERROR - OpModeChange, not enabled!" MEI_DRV_CRLF,
              MEI_DRV_LINENUM_GET(pMeiDev)));

      return IFX_ERROR;
   }

   if (eNewOpMode >= e_MEI_DBG_STREAM_LAST)
   {
      PRN_ERR_USR_NL( MEI_DBG_STREAM, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DBG_STREAM[%02d]: ERROR - OpModeChange, unknown new opmode %d"
            MEI_DRV_CRLF, MEI_DRV_LINENUM_GET(pMeiDev), eNewOpMode));

      return IFX_ERROR;
   }

   if (pInstDynDebugStream->eOpMode >= e_MEI_DBG_STREAM_LAST)
   {
      PRN_ERR_USR_NL( MEI_DBG_STREAM, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DBG_STREAM[%02d]: WARNING - OpModeChange, unknown current "
            "opmode %d was changed to default" MEI_DRV_CRLF,
            MEI_DRV_LINENUM_GET(pMeiDev), pInstDynDebugStream->eOpMode));

      pInstDynDebugStream->eOpMode = e_MEI_DBG_STREAM_DEFAULT_RING;
   }

   if (pInstDynDebugStream->eOpMode != eNewOpMode)
   {
#if (MEI_DEBUG_PRINT == 1)
      eOldOpMode = pInstDynDebugStream->eOpMode;
#endif
      /* protect against device operations */
      MEI_DRV_GET_UNIQUE_MAILBOX_ACCESS(pMeiDev);

      /* reset the FIFO */
      MEI_DRVOS_Var_Fifo_Clear(&pInstDynDebugStream->dbgStreamFifo);
      pInstDynDebugStream->eOpMode = eNewOpMode;

      MEI_DRV_RELEASE_UNIQUE_MAILBOX_ACCESS(pMeiDev);
#if (MEI_DEBUG_PRINT == 1)
      PRN_DBG_USR_NL( MEI_DBG_STREAM, MEI_DRV_PRN_LEVEL_HIGH,
                ("MEI_DBG_STREAM[%02d]: OpModeChange, %s (%d) --> %s (%d)!" MEI_DRV_CRLF,
                 MEI_DRV_LINENUM_GET(pMeiDev),
                 MEI_DbgStreamsOpmodeNames[eOldOpMode], eOldOpMode,
                 MEI_DbgStreamsOpmodeNames[eNewOpMode], eNewOpMode ));
#endif
   }

   return IFX_SUCCESS;
}

MEI_STATIC IFX_int_t MEI_DbgStreamControl(
                              MEI_DYN_CNTRL_T                         *pMeiDynCntrl,
                              MEI_DYN_DBG_STRM_DATA_T                 *pInstDynDebugStream,
                              IFX_boolean_t                            bEnable)
{
   IFX_int_t               retVal = IFX_SUCCESS;
   MEI_DEV_T  *pMeiDev = pMeiDynCntrl->pMeiDev;
   CMD_DBG_DebugStreamControl_t         ctrlCmd;
   ACK_DBG_DebugStreamControl_t         ctrlAck;      

   MEI_DBG_STREAMS_CTX_ACCESS_GET(pMeiDynCntrl);

   if (pInstDynDebugStream == IFX_NULL)
   {
      MEI_DBG_STREAMS_CTX_ACCESS_RELEASE(pMeiDynCntrl);

      PRN_ERR_USR_NL( MEI_DBG_STREAM, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DBG_STREAM[%02d - %02d]: ERROR - IOCTL DbgStreamControl, debug streams must "
             "be configured at first!" MEI_DRV_CRLF, MEI_DRV_LINENUM_GET(pMeiDev), pMeiDynCntrl->openInstance));

      return IFX_ERROR;
   }

   memset(&ctrlCmd, 0x00, sizeof(ctrlCmd));
   memset(&ctrlAck, 0x00, sizeof(ctrlAck));

   ctrlCmd.Length = 1;
   ctrlCmd.Parameter0 = (IFX_uint16_t)bEnable;

   retVal = MEI_InternalSendMessage(pMeiDynCntrl, CMD_DBG_DEBUG_STREAM_CONTROL,
                                 sizeof(ctrlCmd), (unsigned char *)&ctrlCmd,
                                 sizeof(ctrlAck), (unsigned char *)&ctrlAck);

   MEI_DBG_STREAMS_CTX_ACCESS_RELEASE(pMeiDynCntrl);

   return retVal;
}

MEI_STATIC IFX_int_t MEI_DbgStreamDefaultConfigSet(
                              MEI_DYN_CNTRL_T    *pMeiDynCntrl)
{
   IFX_int_t               retVal = IFX_ERROR;
   MEI_DYN_DBG_STRM_DATA_T *pInstDynDebugStream = IFX_NULL;
   MEI_DEV_T  *pMeiDev = pMeiDynCntrl->pMeiDev;
   IFX_uint16_t i = 0;

   MEI_DBG_STREAMS_CTX_ACCESS_GET(pMeiDynCntrl);

   if (pMeiDynCntrl->pInstDynDebugStream != IFX_NULL)
   {
      MEI_DBG_STREAMS_CTX_ACCESS_RELEASE(pMeiDynCntrl);

      PRN_ERR_USR_NL( MEI_DBG_STREAM, MEI_DRV_PRN_LEVEL_WRN,
            ("MEI_DBG_STREAM[%02d - %02d]: ERROR - previous debug stream "
             "configuration exists!" MEI_DRV_CRLF,
              MEI_DRV_LINENUM_GET(pMeiDev), pMeiDynCntrl->openInstance));
              
      return IFX_SUCCESS;
   }

   if ( (retVal = MEI_DbgStreamControlCreate(
                                    pMeiDynCntrl,
                                    MEI_DBG_STREAM_DEFAULT_BUFFER_SIZE,
                                    &pMeiDynCntrl->pInstDynDebugStream)) != IFX_SUCCESS)
   {
      MEI_DBG_STREAMS_CTX_ACCESS_RELEASE(pMeiDynCntrl);

      PRN_ERR_USR_NL( MEI_DBG_STREAM, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DBG_STREAM[%02d - %02d]: ERROR - default config set, create failed!" MEI_DRV_CRLF,
              MEI_DRV_LINENUM_GET(pMeiDev), pMeiDynCntrl->openInstance));

      return retVal;
   }
   
   pInstDynDebugStream = pMeiDynCntrl->pInstDynDebugStream;
   
   if (pInstDynDebugStream != IFX_NULL)
   {
      pInstDynDebugStream->eOpMode       = e_MEI_DBG_STREAM_DEFAULT_RING;
      pInstDynDebugStream->control       = 1;
      pInstDynDebugStream->startTime_ms  = MEI_DRVOS_GetElapsedTime_ms(0);

      pInstDynDebugStream->startStreamId = 0;
      pInstDynDebugStream->startEventId  = 0; 
      pInstDynDebugStream->stopStreamId  = 0; 
      pInstDynDebugStream->stopEventId   = 0;
      for (i = 0; i < MEI_DBG_STREAM_MAX_COMPARE_SIZE_IN_WORDS; i++)
      {
         pInstDynDebugStream->startMask[i]    = 0;  
         pInstDynDebugStream->startPattern[i] = 0;
         pInstDynDebugStream->stopMask[i]     = 0;
         pInstDynDebugStream->stopPattern[i]  = 0;  
      }   
      pInstDynDebugStream->pDynNfcDbgEvtRecieved = IFX_NULL;
   }
   else
   {
      PRN_ERR_USR_NL( MEI_DBG_STREAM, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DBG_STREAM[%02d - %02d]: ERROR - default config set, instance config create failed!" 
             MEI_DRV_CRLF, MEI_DRV_LINENUM_GET(pMeiDev), pMeiDynCntrl->openInstance));   
   }

   MEI_DBG_STREAMS_GLOBAL_CFG_ACCESS_GET;
   MEI_DBG_STREAMS_GLOBAL_CFG_ADD_DEV(pMeiDynCntrl->pDfeX->entityNum);
   MEI_DBG_STREAMS_GLOBAL_CFG_ACCESS_RELEASE;

   MEI_DBG_STREAMS_CTX_ACCESS_RELEASE(pMeiDynCntrl);

   PRN_DBG_USR_NL( MEI_DBG_STREAM, MEI_DRV_PRN_LEVEL_NORMAL,
         ("MEI_DBG_STREAM[%02d - %02d]: default config set - done!" MEI_DRV_CRLF,
           MEI_DRV_LINENUM_GET(pMeiDev), pMeiDynCntrl->openInstance));

   return IFX_SUCCESS;
}

/* ==========================================================================
   Debug Streams - Global Functions.
   ========================================================================== */

/**
   Do the basic startup initialisation (driver module init).
*/
IFX_int_t MEI_DBG_STREAM_ModuleInit(void)
{
   memset(MEI_DRV_dbgStreamsOn,    0x00, sizeof(MEI_DRV_dbgStreamsOn));
   memset(&MEI_dbgStreamCntrlLock, 0x00, sizeof(MEI_DRVOS_sema_t));

   if (MEI_DRVOS_SemaphoreInit(&MEI_dbgStreamCntrlLock) != IFX_SUCCESS)
   {
      return IFX_ERROR;
   }

   return IFX_SUCCESS;
}

/**
   Delete the basic startup initialisation (driver module exit).
*/
IFX_int_t MEI_DBG_STREAM_ModuleDelete(void)
{
   MEI_DRVOS_SemaphoreDelete(&MEI_dbgStreamCntrlLock);

   return IFX_SUCCESS;
}


/**
   Release and free the debug streams resources.

\remark
   Used for "module delete", therefore the access is protected against
   - other tasks.

   The devices are already down - no interrupt protection necessary.
*/
IFX_int_t MEI_DBG_STREAM_ControlRelease(
                              MEI_DYN_CNTRL_T  *pMeiDynCntrl)
{
   MEI_DYN_DBG_STRM_DATA_T *pInstDynDebugStream = IFX_NULL;

   MEI_DBG_STREAMS_CTX_ACCESS_GET(pMeiDynCntrl);

   if (pMeiDynCntrl->pInstDynDebugStream != IFX_NULL)
   {
      MEI_DbgStreamRemoveFromDevList(pMeiDynCntrl, &pInstDynDebugStream);

      MEI_DRVOS_Var_Fifo_Clear(&pInstDynDebugStream->dbgStreamFifo);
      MEI_DRVOS_VirtFree(pInstDynDebugStream);
   }

   MEI_DBG_STREAMS_CTX_ACCESS_RELEASE(pMeiDynCntrl);

   return IFX_SUCCESS;
}

/**
   Reset the debug streams resources.

\remark
   Used for "module delete", therefore the access is protected against
   - other tasks.

   The devices are already down - no interrupt protection necessary.
*/
IFX_int_t MEI_DBG_STREAM_ControlReset(
                              MEI_DYN_CNTRL_T  *pMeiDynCntrl)
{
   MEI_DYN_DBG_STRM_DATA_T *pInstDynDebugStream = IFX_NULL;

   MEI_DBG_STREAMS_CTX_ACCESS_GET(pMeiDynCntrl);

   if (pMeiDynCntrl->pInstDynDebugStream != IFX_NULL)
   {
      pInstDynDebugStream = pMeiDynCntrl->pInstDynDebugStream;
      pMeiDynCntrl->pInstDynDebugStream    = IFX_NULL;

      pInstDynDebugStream->bReadPending    = IFX_FALSE;
      pInstDynDebugStream->control         = 0;
      pInstDynDebugStream->eFilterState    = eMEI_DbgStreamFilterState_SkipThis;
      pInstDynDebugStream->bNotify         = IFX_FALSE;

      pInstDynDebugStream->bOverflowOccurred = eMEI_DbgStreamOverflowState_Clear;
      MEI_DRVOS_Var_Fifo_Clear(&pInstDynDebugStream->dbgStreamFifo);

      pMeiDynCntrl->pInstDynDebugStream = pInstDynDebugStream;
   }

   MEI_DBG_STREAMS_CTX_ACCESS_RELEASE(pMeiDynCntrl);

   return IFX_SUCCESS;
}

/**
   Set necessary fw confirure options for debug streams

\param
   pMeiDynCntrl   - private dynamic device data (per open instance)

\return
   IFX_SUCCESS    Success
   IFX_ERROR      in case of error
*/
IFX_int_t MEI_DBG_STREAM_FwConfigSet(MEI_DYN_CNTRL_T *pMeiDynCntrl)
{
   IFX_int32_t                        ret = 0;
   MEI_DYN_DBG_STRM_DATA_T           *pDbgStrmInstance_next = 
                                      pMeiDynCntrl->pMeiDev->pRootDbgStrmRecvFirst;

   while(pDbgStrmInstance_next)
   {
      if (pDbgStrmInstance_next != IFX_NULL)
      {
         ret = MEI_DbgStreamControl(pMeiDynCntrl, pDbgStrmInstance_next, pDbgStrmInstance_next->control);
         
      }
         
      /* get next */
      pDbgStrmInstance_next = pDbgStrmInstance_next->pNext;
   }

   return ret;
}

/* ==========================================================================
   Debug Streams - IOCTL Functions.
   ========================================================================== */

/**
   IOCTL Command - debug streams setup.
*/
IFX_int_t MEI_IoctlDbgStreamConfigGet(
                              MEI_DYN_CNTRL_T                      *pMeiDynCntrl,
                              IOCTL_MEI_DEBUG_STREAM_configGet_t   *pDbgStreamInitGet)
{
   MEI_DYN_DBG_STRM_DATA_T *pInstDynDebugStream = IFX_NULL;
   MEI_DEV_T  *pMeiDev = pMeiDynCntrl->pMeiDev;   
   IFX_uint16_t i = 0;

   MEI_DBG_STREAMS_CTX_ACCESS_GET(pMeiDynCntrl);

   if (pMeiDynCntrl->pInstDynDebugStream == IFX_NULL)
   {
      PRN_ERR_USR_NL( MEI_DBG_STREAM, MEI_DRV_PRN_LEVEL_WRN,
               ("MEI_DBG_STREAM[%02d - %02d]: ERROR - IOCTL DbgStreamConfigGet, debug streams configuration "
                "is not set!" MEI_DRV_CRLF, MEI_DRV_LINENUM_GET(pMeiDev), pMeiDynCntrl->openInstance));      

      return IFX_ERROR;
   }

   pInstDynDebugStream = pMeiDynCntrl->pInstDynDebugStream;
   pDbgStreamInitGet->bufferType            = 0;
   pDbgStreamInitGet->bufferSize            = MEI_DBG_STREAMS_CAST(unsigned int, pInstDynDebugStream->dbgStreamFifoBufferSize);
   pDbgStreamInitGet->onOff                 = MEI_DBG_STREAMS_CAST(unsigned int, pInstDynDebugStream->control);
   if (pInstDynDebugStream->pDynNfcDbgEvtRecieved != IFX_NULL)
   {
      pDbgStreamInitGet->notificationEnabled = 1;
   }
   pDbgStreamInitGet->operationMode         = pInstDynDebugStream->eOpMode;
   pDbgStreamInitGet->filterMode            = pInstDynDebugStream->eFilterMode;
   pDbgStreamInitGet->startStreamId         = MEI_DBG_STREAMS_CAST(unsigned short, pInstDynDebugStream->startStreamId);
   pDbgStreamInitGet->startEventId          = MEI_DBG_STREAMS_CAST(unsigned short, pInstDynDebugStream->startEventId);
   pDbgStreamInitGet->stopStreamId          = MEI_DBG_STREAMS_CAST(unsigned short, pInstDynDebugStream->stopStreamId);
   pDbgStreamInitGet->stopEventId           = MEI_DBG_STREAMS_CAST(unsigned short, pInstDynDebugStream->stopEventId);
   for (i = 0; i < MEI_DBG_STREAM_MAX_COMPARE_SIZE_IN_WORDS; i++)
   {
      pDbgStreamInitGet->startMask[i]    = MEI_DBG_STREAMS_CAST(unsigned short, pInstDynDebugStream->startMask[i]);     
      pDbgStreamInitGet->startPattern[i] = MEI_DBG_STREAMS_CAST(unsigned short, pInstDynDebugStream->startPattern[i]);
      pDbgStreamInitGet->stopMask[i]     = MEI_DBG_STREAMS_CAST(unsigned short, pInstDynDebugStream->stopMask[i]);
      pDbgStreamInitGet->stopPattern[i]  = MEI_DBG_STREAMS_CAST(unsigned short, pInstDynDebugStream->stopPattern[i]);
   }

   MEI_DBG_STREAMS_CTX_ACCESS_RELEASE(pMeiDynCntrl);

   PRN_DBG_USR_NL( MEI_DBG_STREAM, MEI_DRV_PRN_LEVEL_LOW,
         ("MEI_DBG_STREAM[%02d - %02d]: - IOCTL Init Get: OnOff = %d, Notify = %d, Mode = %d, FilterMode = %d, BType = %d, BSize = %d" MEI_DRV_CRLF,
           MEI_DRV_LINENUM_GET(pMeiDev),
           pMeiDynCntrl->openInstance,
           pDbgStreamInitGet->onOff, 
           pDbgStreamInitGet->notificationEnabled,
           pDbgStreamInitGet->operationMode,
           pDbgStreamInitGet->filterMode,
           pDbgStreamInitGet->bufferType,
           pDbgStreamInitGet->bufferSize));


   return IFX_SUCCESS;
}


/**
   IOCTL Command - debug streams setup.
*/
IFX_int_t MEI_IoctlDbgStreamConfigSet(
                              MEI_DYN_CNTRL_T                            *pMeiDynCntrl,
                              const IOCTL_MEI_DEBUG_STREAM_configSet_t   *pDbgStreamInitSet)
{
   IFX_int_t               retVal = IFX_ERROR;
   MEI_DYN_DBG_STRM_DATA_T *pInstDynDebugStream = IFX_NULL;
   MEI_DEV_T  *pMeiDev = pMeiDynCntrl->pMeiDev;
   IFX_uint16_t i = 0;

   MEI_DBG_STREAMS_CTX_ACCESS_GET(pMeiDynCntrl);

   if (pMeiDynCntrl->pInstDynDebugStream == IFX_NULL)
   {
      if ( (retVal = MEI_DbgStreamControlCreate(
                                       pMeiDynCntrl,
                                       MEI_DBG_STREAMS_CAST(IFX_uint_t, pDbgStreamInitSet->bufferSize),
                                       &pInstDynDebugStream)) != IFX_SUCCESS)
      {
         MEI_DBG_STREAMS_CTX_ACCESS_RELEASE(pMeiDynCntrl);

         PRN_ERR_USR_NL( MEI_DBG_STREAM, MEI_DRV_PRN_LEVEL_ERR,
               ("MEI_DBG_STREAM[%02d - %02d]: ERROR - IOCTL DbgStreamConfigSet, create failed!" MEI_DRV_CRLF,
                 MEI_DRV_LINENUM_GET(pMeiDev), pMeiDynCntrl->openInstance));

         return retVal;
      }

      pMeiDynCntrl->pInstDynDebugStream = pInstDynDebugStream;
   }

   if (pInstDynDebugStream != IFX_NULL)
   {
      /* The stream data should not be filtered by default */
      pInstDynDebugStream->eFilterState  = eMEI_DbgStreamFilterState_Store;
      pInstDynDebugStream->eOpMode       = pDbgStreamInitSet->operationMode;
      pInstDynDebugStream->control       = pDbgStreamInitSet->onOff;
      if ((pDbgStreamInitSet->filterMode != e_MEI_DBG_STREAM_START_STOP) &&
          (pDbgStreamInitSet->filterMode != e_MEI_DBG_STREAM_SNAPSHOT))
      {
         PRN_ERR_USR_NL( MEI_DBG_STREAM, MEI_DRV_PRN_LEVEL_ERR,
                  ("MEI_DBG_STREAM[%02d - %02d]: ERROR - IOCTL "
                  "DbgStreamConfigSet, unknown  filter mode %d!"
                  "Default value is used!"
                   MEI_DRV_CRLF, MEI_DRV_LINENUM_GET(pMeiDev),
                   pMeiDynCntrl->openInstance, pDbgStreamInitSet->filterMode));

         pInstDynDebugStream->eFilterMode = e_MEI_DBG_STREAM_START_STOP;
      }
      else
      {
         pInstDynDebugStream->eFilterMode = pDbgStreamInitSet->filterMode;
      }
      if (pDbgStreamInitSet->notificationEnabled == 1)
      {
         pInstDynDebugStream->pDynNfcDbgEvtRecieved = (IFX_void_t*)pMeiDynCntrl->pInstDynNfc;
         if (pInstDynDebugStream->pDynNfcDbgEvtRecieved == IFX_NULL)
         {
            PRN_ERR_USR_NL( MEI_DBG_STREAM, MEI_DRV_PRN_LEVEL_ERR,
                     ("MEI_DBG_STREAM[%02d - %02d]: ERROR - IOCTL DbgStreamConfigSet, NFC messages are disabled!"
                      MEI_DRV_CRLF, MEI_DRV_LINENUM_GET(pMeiDev), pMeiDynCntrl->openInstance));
         }
      }
      pInstDynDebugStream->startTime_ms  = MEI_DRVOS_GetElapsedTime_ms(0);
      pInstDynDebugStream->startStreamId = pDbgStreamInitSet->startStreamId;
      pInstDynDebugStream->startEventId  = pDbgStreamInitSet->startEventId;
      pInstDynDebugStream->stopStreamId  = pDbgStreamInitSet->stopStreamId;
      pInstDynDebugStream->stopEventId   = pDbgStreamInitSet->stopEventId;

      if (pInstDynDebugStream->eFilterMode == e_MEI_DBG_STREAM_START_STOP)
      {
         /* Check if the start stream id criteria was defined
            filter the stream data until the criteria will be reached */
         if (pInstDynDebugStream->startStreamId != 0)
         {
            pInstDynDebugStream->eFilterState = eMEI_DbgStreamFilterState_SkipThis;
         }

         /* Check if the start event id criteria was defined
            filter the stream data untill the criteria will be reached */
         if (pInstDynDebugStream->startEventId != 0)
         {
            pInstDynDebugStream->eFilterState =
               eMEI_DbgStreamFilterState_SkipThis;
         }
      }
      else
      {
         pInstDynDebugStream->eFilterState = eMEI_DbgStreamFilterState_Store;
      }

      for (i = 0; i < MEI_DBG_STREAM_MAX_COMPARE_SIZE_IN_WORDS; i++)
      {
         pInstDynDebugStream->startMask[i]    = pDbgStreamInitSet->startMask[i];
         pInstDynDebugStream->startPattern[i] = pDbgStreamInitSet->startPattern[i];
         pInstDynDebugStream->stopMask[i]     = pDbgStreamInitSet->stopMask[i];
         pInstDynDebugStream->stopPattern[i]  = pDbgStreamInitSet->stopPattern[i];

         if (pInstDynDebugStream->eFilterMode == e_MEI_DBG_STREAM_START_STOP)
         {
            /* Check if the start mask criteria was defined
               filter the stream data untill the criteria will be reached */
            if (pInstDynDebugStream->startMask[i] != 0)
            {
               pInstDynDebugStream->eFilterState =
                  eMEI_DbgStreamFilterState_SkipThis;
            }
         }
      }
   }

   MEI_DBG_STREAMS_GLOBAL_CFG_ACCESS_GET;
   MEI_DBG_STREAMS_GLOBAL_CFG_ADD_DEV(pMeiDynCntrl->pDfeX->entityNum);
   MEI_DBG_STREAMS_GLOBAL_CFG_ACCESS_RELEASE;

   MEI_DBG_STREAMS_CTX_ACCESS_RELEASE(pMeiDynCntrl);

   PRN_DBG_USR_NL( MEI_DBG_STREAM, MEI_DRV_PRN_LEVEL_NORMAL,
         ("MEI_DBG_STREAM[%02d - %02d]: ioctl init - done!" MEI_DRV_CRLF,
           MEI_DRV_LINENUM_GET(pMeiDev), pMeiDynCntrl->openInstance));

   return IFX_SUCCESS;
}

/**
   Release the debug stream (free resources)
*/
IFX_int_t MEI_IoctlDbgStreamRelease(
                              MEI_DYN_CNTRL_T                         *pMeiDynCntrl,
                              const IOCTL_MEI_DEBUG_STREAM_release_t  *param)
{
   MEI_DYN_DBG_STRM_DATA_T *pInstDynDebugStream = IFX_NULL;
   MEI_DEV_T  *pMeiDev = pMeiDynCntrl->pMeiDev;         

   MEI_DBG_STREAMS_CTX_ACCESS_GET(pMeiDynCntrl);

   if (pMeiDynCntrl->pInstDynDebugStream == IFX_NULL)
   {
      PRN_ERR_USR_NL( MEI_DBG_STREAM, MEI_DRV_PRN_LEVEL_WRN,
               ("MEI_DBG_STREAM[%02d - %02d]: ERROR - IOCTL DbgStreamRelease, debug streams already disabled!"
                MEI_DRV_CRLF, MEI_DRV_LINENUM_GET(pMeiDev), pMeiDynCntrl->openInstance));      

      MEI_DBG_STREAMS_CTX_ACCESS_RELEASE(pMeiDynCntrl);
      return IFX_SUCCESS;
   }   

   switch(param->releaseMode)
   {
      case e_MEI_DBG_STREAM_RELEASE_COMPLETELY:
         (void)MEI_DbgStreamControlDelete(pMeiDynCntrl);
         MEI_DBG_STREAMS_GLOBAL_CFG_ACCESS_GET;
         MEI_DBG_STREAMS_GLOBAL_CFG_REMOVE_DEV(pMeiDynCntrl->pDfeX->entityNum);
         MEI_DBG_STREAMS_GLOBAL_CFG_ACCESS_RELEASE;
         break;
      case e_MEI_DBG_STREAM_DELETE_DATA_AND_STATISTIC:
         pInstDynDebugStream = pMeiDynCntrl->pInstDynDebugStream;
         MEI_DRVOS_Var_Fifo_Clear(&pInstDynDebugStream->dbgStreamFifo);
#if (MEI_DBG_STREAMS_SUPPORT_STATISTICS == 1)
         memset(&pInstDynDebugStream->dbgStreamStatistics, 0, sizeof(pInstDynDebugStream->dbgStreamStatistics));
#endif         
         break;
      default:
         PRN_DBG_USR_NL( MEI_DBG_STREAM, MEI_DRV_PRN_LEVEL_HIGH,
               ("MEI_DBG_STREAM[%02d - %02d]: ioctl release - FAILED, unknown mode(%d)!" MEI_DRV_CRLF,
                 MEI_DRV_LINENUM_GET(pMeiDev),
                 pMeiDynCntrl->openInstance, param->releaseMode));
         MEI_DBG_STREAMS_CTX_ACCESS_RELEASE(pMeiDynCntrl);
         return IFX_ERROR;
   }
   MEI_DBG_STREAMS_CTX_ACCESS_RELEASE(pMeiDynCntrl);

   PRN_DBG_USR_NL( MEI_DBG_STREAM, MEI_DRV_PRN_LEVEL_NORMAL,
         ("MEI_DBG_STREAM[%02d - %02d]: ioctl release - done!" MEI_DRV_CRLF,
           MEI_DRV_LINENUM_GET(pMeiDev), pMeiDynCntrl->openInstance));

   return IFX_SUCCESS;
}


IFX_int_t MEI_IoctlDbgStreamMaskSet(
                              MEI_DYN_CNTRL_T                    *pMeiDynCntrl,
                              IOCTL_MEI_DEBUG_STREAM_mask_set_t  *pDbgStreamMask)
{
   IFX_int_t               retVal = IFX_ERROR;
#if (MEI_DEBUG_PRINT == 1)
   MEI_DEV_T  *pMeiDev = pMeiDynCntrl->pMeiDev;
#endif
   CMD_DBG_DebugStreamConfigure_t cfgCmd;
   ACK_DBG_DebugStreamConfigure_t cfgAck;  

   MEI_DBG_STREAMS_CTX_ACCESS_GET(pMeiDynCntrl);

   if (pDbgStreamMask != IFX_NULL)
   {
      memset(&cfgCmd, 0x00, sizeof(cfgCmd));
      memset(&cfgAck, 0x00, sizeof(cfgAck));

      cfgCmd.Length = 10;
      cfgCmd.Mask1 = pDbgStreamMask->mask1;
      cfgCmd.Mask2 = pDbgStreamMask->mask2;
      cfgCmd.Mask3 = pDbgStreamMask->mask3;
      cfgCmd.Mask4 = pDbgStreamMask->mask4;
      cfgCmd.Mask5 = pDbgStreamMask->mask5;

      retVal = MEI_InternalSendMessage(pMeiDynCntrl, CMD_DBG_DEBUG_STREAM_CONFIGURE,
                                    sizeof(cfgCmd), (unsigned char *)&cfgCmd,
                                    sizeof(cfgAck), (unsigned char *)&cfgAck);

      PRN_DBG_USR_NL( MEI_DBG_STREAM, MEI_DRV_PRN_LEVEL_NORMAL,
            ("MEI_DBG_STREAM[%02d - %02d]: ioctl mask set - done!" MEI_DRV_CRLF,
              MEI_DRV_LINENUM_GET(pMeiDev), pMeiDynCntrl->openInstance));
   }

   MEI_DBG_STREAMS_CTX_ACCESS_RELEASE(pMeiDynCntrl);

   return IFX_SUCCESS;
}

/**
   Control the debug stream feature
*/
IFX_int_t MEI_IoctlDbgStreamControl(
                              MEI_DYN_CNTRL_T                         *pMeiDynCntrl,
                              const IOCTL_MEI_DEBUG_STREAM_control_t  *pDbgStreamCntrl)
{
   IFX_int_t               retVal = IFX_SUCCESS;
   MEI_DYN_DBG_STRM_DATA_T *pInstDynDebugStream = pMeiDynCntrl->pInstDynDebugStream;
   MEI_DEV_T  *pMeiDev = pMeiDynCntrl->pMeiDev;

   MEI_DBG_STREAMS_CTX_ACCESS_GET(pMeiDynCntrl);

   if (pInstDynDebugStream == IFX_NULL)
   {
      retVal = MEI_DbgStreamDefaultConfigSet(pMeiDynCntrl);
      if (retVal != IFX_SUCCESS)
      {    
         MEI_DBG_STREAMS_CTX_ACCESS_RELEASE(pMeiDynCntrl);

         PRN_ERR_USR_NL( MEI_DBG_STREAM, MEI_DRV_PRN_LEVEL_ERR,
               ("MEI_DBG_STREAM[%02d - %02d]: ERROR - IOCTL DbgStreamControl, debug streams default "
                "configuration setting failed!" MEI_DRV_CRLF, MEI_DRV_LINENUM_GET(pMeiDev), pMeiDynCntrl->openInstance));

         return retVal;
      }

      PRN_ERR_USR_NL( MEI_DBG_STREAM, MEI_DRV_PRN_LEVEL_WRN,
               ("MEI_DBG_STREAM[%02d - %02d]: ERROR - IOCTL DbgStreamControl, debug streams configuration "
                "is not set, use default configuration!" MEI_DRV_CRLF, MEI_DRV_LINENUM_GET(pMeiDev), pMeiDynCntrl->openInstance));      
   }
   
   if (pDbgStreamCntrl != IFX_NULL)
   {
      retVal = MEI_DbgStreamOpModeChange(pMeiDynCntrl, pDbgStreamCntrl->operationMode);

      if ( retVal != IFX_SUCCESS)
      {
         MEI_DBG_STREAMS_CTX_ACCESS_RELEASE(pMeiDynCntrl);

         PRN_ERR_USR_NL( MEI_DBG_STREAM, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DBG_STREAM[%02d - %02d]: ERROR - IOCTL DbgStreamControl, OP "
            "mode change" MEI_DRV_CRLF, MEI_DRV_LINENUM_GET(pMeiDev),
            pMeiDynCntrl->openInstance));

         return retVal;
      }
   }

   if ((pDbgStreamCntrl != IFX_NULL ) && (pInstDynDebugStream != IFX_NULL))
   {
      if (pDbgStreamCntrl->onOff == 1)
      {
         pInstDynDebugStream->control = 1;
   
         if (pInstDynDebugStream->eFilterMode == e_MEI_DBG_STREAM_SNAPSHOT)
         {
            pInstDynDebugStream->eFilterState = eMEI_DbgStreamFilterState_Store;
         }
      }
      else
      {
         pInstDynDebugStream->control = 0;
      }
   }

   MEI_DBG_STREAMS_CTX_ACCESS_RELEASE(pMeiDynCntrl);

   return retVal;
}

/**
   Read out the collected driver statistics.
*/
IFX_int_t MEI_IoctlDbgStreamStatisticGet(
                              MEI_DYN_CNTRL_T                      *pMeiDynCntrl,
                              IOCTL_MEI_DEBUG_STREAM_statistic_t   *pDbgStreamStatistics)
{
   MEI_DEV_T  *pMeiDev = pMeiDynCntrl->pMeiDev;
#if (MEI_DBG_STREAMS_SUPPORT_STATISTICS == 1)
   MEI_DbgStreamStatistic_t  *pDbgStreamCtxStatistic = IFX_NULL;

   memset(pDbgStreamStatistics, 0x00, sizeof(IOCTL_MEI_DEBUG_STREAM_statistic_t));

   MEI_DBG_STREAMS_CTX_ACCESS_GET(pMeiDynCntrl);

   if (pMeiDynCntrl->pInstDynDebugStream == IFX_NULL)
   {
      MEI_DBG_STREAMS_CTX_ACCESS_RELEASE(pMeiDynCntrl);

      PRN_ERR_USR_NL( MEI_DBG_STREAM, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DBG_STREAM[%02d - %02d]: ERROR - IOCTL DbgStreamStatisticGet, debug streams must "
             "be configured at first!" MEI_DRV_CRLF, MEI_DRV_LINENUM_GET(pMeiDev), pMeiDynCntrl->openInstance));

      return IFX_ERROR;
   }

   pDbgStreamCtxStatistic = &pMeiDynCntrl->pInstDynDebugStream->dbgStreamStatistics;

   pDbgStreamStatistics->msgD2HCnt =
      MEI_DBG_STREAMS_CAST(unsigned int, pDbgStreamCtxStatistic->msgD2HCnt);
   pDbgStreamStatistics->dataD2HSize_byte =
      MEI_DBG_STREAMS_CAST(unsigned int, pDbgStreamCtxStatistic->dataD2HSize_byte);
   pDbgStreamStatistics->msgH2BufCnt =
      MEI_DBG_STREAMS_CAST(unsigned int, pDbgStreamCtxStatistic->msgH2BufCnt);
   pDbgStreamStatistics->dataH2BufSize_byte =
      MEI_DBG_STREAMS_CAST(unsigned int, pDbgStreamCtxStatistic->dataH2BufSize_byte);
   pDbgStreamStatistics->msgBuf2UsrCnt =
      MEI_DBG_STREAMS_CAST(unsigned int, pDbgStreamCtxStatistic->msgBuf2UsrCnt);
   pDbgStreamStatistics->dataBuf2UsrSize_byte =
      MEI_DBG_STREAMS_CAST(unsigned int, pDbgStreamCtxStatistic->dataBuf2UsrSize_byte);
   pDbgStreamStatistics->msgRingBufferOverwriteCnt =
      MEI_DBG_STREAMS_CAST(unsigned int, pDbgStreamCtxStatistic->msgRingBufferOverwriteCnt);
   pDbgStreamStatistics->dataRingBufferOverwriteSize_byte =
      MEI_DBG_STREAMS_CAST(unsigned int, pDbgStreamCtxStatistic->dataRingBufferOverwriteSize_byte);
   pDbgStreamStatistics->msgBufferFullDiscardCnt =
      MEI_DBG_STREAMS_CAST(unsigned int, pDbgStreamCtxStatistic->msgBufferFullDiscardCnt);
   pDbgStreamStatistics->dataBufferFullDiscardSize_byte =
      MEI_DBG_STREAMS_CAST(unsigned int, pDbgStreamCtxStatistic->dataBufferFullDiscardSize_byte);
   pDbgStreamStatistics->msgProcessErrDiscardCnt =
      MEI_DBG_STREAMS_CAST(unsigned int, pDbgStreamCtxStatistic->msgProcessErrDiscardCnt);
   pDbgStreamStatistics->dataProcessErrDiscardSize_byte =
      MEI_DBG_STREAMS_CAST(unsigned int, pDbgStreamCtxStatistic->dataProcessErrDiscardSize_byte);
   pDbgStreamStatistics->msgCfgDiscardCnt =
      MEI_DBG_STREAMS_CAST(unsigned int, pDbgStreamCtxStatistic->msgCfgDiscardCnt);
   pDbgStreamStatistics->dataCfgDiscardSize_byte =
      MEI_DBG_STREAMS_CAST(unsigned int, pDbgStreamCtxStatistic->dataCfgDiscardSize_byte);
   pDbgStreamStatistics->msgCorruptedCnt =
      MEI_DBG_STREAMS_CAST(unsigned int, pDbgStreamCtxStatistic->msgCorruptedCnt);

   pDbgStreamStatistics->deviceOverflowCnt =
      MEI_DBG_STREAMS_CAST(unsigned int, pDbgStreamCtxStatistic->deviceOverflowCnt);

   pDbgStreamStatistics->bufferOverflowCnt =
      MEI_DBG_STREAMS_CAST(unsigned int, pDbgStreamCtxStatistic->bufferOverflowCnt);

   pDbgStreamStatistics->streamD2HCnt =
      MEI_DBG_STREAMS_CAST(unsigned int, pDbgStreamCtxStatistic->streamD2HCnt);

   pDbgStreamStatistics->runTime_ms =
      MEI_DBG_STREAMS_CAST(unsigned int, (pDbgStreamCtxStatistic->startTime - pDbgStreamCtxStatistic->currTime));

   MEI_DBG_STREAMS_CTX_ACCESS_RELEASE(pMeiDynCntrl);

   return IFX_SUCCESS;

#else

   memset(pDbgStreamStatistics, 0x00, sizeof(IOCTL_MEI_DEBUG_STREAM_statistic_t));

   PRN_ERR_USR_NL( MEI_DBG_STREAM, MEI_DRV_PRN_LEVEL_ERR,
         ("MEI_DBG_STREAM[%02d - %02d]: ERROR - IOCTL Statistics, statistics not enabled!" MEI_DRV_CRLF,
           MEI_DRV_LINENUM_GET(pMeiDev), pMeiDynCntrl->openInstance));

   return IFX_ERROR;

#endif
}

/**
   Read out the collected debug stream data.
*/
IFX_int_t MEI_IoctlDbgStreamDataGet(
                              MEI_DYN_CNTRL_T                *pMeiDynCntrl,
                              IOCTL_MEI_DEBUG_STREAM_data_t  *pDbgStreamRead)
{
   return MEI_DbgStreamDataGet(pMeiDynCntrl, pDbgStreamRead, IFX_FALSE);
}

/**
   Read out the collected debug stream data.
*/
MEI_STATIC IFX_int_t MEI_DbgStreamDataGet(
                              MEI_DYN_CNTRL_T                *pMeiDynCntrl,
                              IOCTL_MEI_DEBUG_STREAM_data_t  *pDbgStreamRead,
                              IFX_boolean_t                   bInternCall)
{

   IFX_int_t                retVal = IFX_SUCCESS;
   IFX_uint_t               waitCnt, maxReads, readSize_byte = 0, readElemSizeAll_byte = 0, readElemCnt = 0;
   MEI_DYN_DBG_STRM_DATA_T  *pInstDynDebugStream = IFX_NULL;
   MEI_OS_BUFFER_PTR_T      uFifoBufferRead, uUserBufferWrite;
   MEI_DEV_T                *pMeiDev = pMeiDynCntrl->pMeiDev;

   if ( (pDbgStreamRead->pData == IFX_NULL) || (pDbgStreamRead->dataBufferSize_byte == 0) )
   {
      /* ERROR invalid arguments - missing user buffer */
      PRN_ERR_USR_NL( MEI_DBG_STREAM, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DBG_STREAM[%02d]: ERROR - IOCTL DataRead, missing user buffer!" MEI_DRV_CRLF,
              MEI_DRV_LINENUM_GET(pMeiDev)));

      pDbgStreamRead->maxStreamEntries    = 0;
      pDbgStreamRead->dataBufferSize_byte = 0;

      return IFX_ERROR;
   }

   uUserBufferWrite.pUInt8 = pDbgStreamRead->pData;

   MEI_DBG_STREAMS_CTX_ACCESS_GET(pMeiDynCntrl);

   if (pMeiDynCntrl->pInstDynDebugStream == IFX_NULL)
   {
      MEI_DBG_STREAMS_CTX_ACCESS_RELEASE(pMeiDynCntrl);

      PRN_ERR_USR_NL( MEI_DBG_STREAM, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DBG_STREAM[%02d]: ERROR - IOCTL DbgStreamDataGet, debug streams must "
             "be configured at first!" MEI_DRV_CRLF, MEI_DRV_LINENUM_GET(pMeiDev)));

      pDbgStreamRead->maxStreamEntries    = 0;
      pDbgStreamRead->dataBufferSize_byte = 0;

      return IFX_ERROR;
   }

   pInstDynDebugStream = pMeiDynCntrl->pInstDynDebugStream;

   maxReads = (pDbgStreamRead->maxStreamEntries != 0) ? pDbgStreamRead->maxStreamEntries : 1;
   if (pDbgStreamRead->timeout_ms == MEI_DBG_STREAMS_CAST(unsigned int, ~0))
   {
      waitCnt = ~0;
   }
   else
   {
      waitCnt  = MEI_DBG_STREAMS_CAST(IFX_uint_t, (pDbgStreamRead->timeout_ms / MEI_DBG_STREAMS_WAIT_POLL_TIME_MS));
   }

#if (IFXOS_SUPPORTS_FIFO_PEEK == 1)
   MEI_DRV_GET_UNIQUE_MAILBOX_ACCESS(pMeiDev);
   pInstDynDebugStream->bReadPending = IFX_TRUE;
   MEI_DRV_RELEASE_UNIQUE_MAILBOX_ACCESS(pMeiDev);

   while (maxReads)
   {
      uFifoBufferRead.pULong = MEI_DRVOS_Var_Fifo_peekElement(&pInstDynDebugStream->dbgStreamFifo, &readSize_byte);

      if ( (uFifoBufferRead.pVoid == IFX_NULL) || (readSize_byte == 0) )
      {
         /* FIFO empty */
         if (waitCnt == 0)
         {
            /* no timeout or timeout reached */
            break;
         }

         MEI_DRVOS_Wait_ms(MEI_DBG_STREAMS_WAIT_POLL_TIME_MS);
         if (MEI_DBG_STREAMS_CAST(int, waitCnt) != ~0)
         {
            waitCnt--;
         }
         continue;
      }

      if ( (readElemSizeAll_byte + readSize_byte) > pDbgStreamRead->dataBufferSize_byte)
      {
         /* user buffer full */
         break;
      }

#if (MEI_DRV_OS_BYTE_ORDER != MEI_DRV_OS_BIG_ENDIAN)
      /* Swap debug stream data to device order */
      {
         IFX_uint_t i;
         IFX_uint16_t *pData16 = uFifoBufferRead.pUInt16;

         for (i = 0; i < (readSize_byte / sizeof(IFX_uint16_t)) ; i++)
         {
            pData16[i] = SWAP16_BYTE_ORDER(pData16[i]);
         }
      }
#endif

      if (bInternCall == IFX_FALSE)
      {
         /* copy data to user space */
         if ( (MEI_DRVOS_CpyToUser(
                        uUserBufferWrite.pVoid,
                        uFifoBufferRead.pUInt8,
                        MEI_DBG_STREAMS_CAST(IFX_uint32_t, readSize_byte))) == IFX_NULL )
         {
            PRN_ERR_USR_NL( MEI_DBG_STREAM, MEI_DRV_PRN_LEVEL_ERR,
                  ("MEI_DBG_STREAM[%02d]: ERROR - IOCTL DataRead, cpy2usr!" MEI_DRV_CRLF,
                    MEI_DRV_LINENUM_GET(pMeiDev)));

            retVal = IFX_ERROR;
            break;
         }
      }
      else
      {
         memcpy(uUserBufferWrite.pUInt8,
                uFifoBufferRead.pUInt8,
                MEI_DBG_STREAMS_CAST(IFX_uint32_t, readSize_byte));
      }
      uUserBufferWrite.pUInt8 += readSize_byte;
      readElemSizeAll_byte    += readSize_byte;
      readElemCnt++;

      /* FIFO peek - free element */
      MEI_DRV_GET_UNIQUE_MAILBOX_ACCESS(pMeiDev);

      uFifoBufferRead.pULong = MEI_DRVOS_Var_Fifo_readElement(&pInstDynDebugStream->dbgStreamFifo, &readSize_byte);

      MEI_DBG_STREAMS_STAT_MSG_BUF2USR_CNT_INC(pInstDynDebugStream);
      MEI_DBG_STREAMS_STAT_DATA_BUF2USR_SIZE_ADD(pInstDynDebugStream, readSize_byte);

      MEI_DRV_RELEASE_UNIQUE_MAILBOX_ACCESS(pMeiDev);

      if (pDbgStreamRead->maxStreamEntries != 0)
      {
         maxReads--;
      }
   }

   MEI_DRV_GET_UNIQUE_MAILBOX_ACCESS(pMeiDev);
   pInstDynDebugStream->bReadPending = IFX_FALSE;
   MEI_DRV_RELEASE_UNIQUE_MAILBOX_ACCESS(pMeiDev);

#else

   /* FIFO read mode - at least we need space to save one dbg stream element --> break */
   if ( pDbgStreamRead->dataBufferSize_byte < sizeof(EVT_DBG_DebugStream_t))
   {
      /* ERROR invalid arguments - not enough user buffer */
      PRN_ERR_USR_NL( MEI_DBG_STREAM, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DBG_STREAM[%02d]: ERROR - IOCTL DataRead, not enough user buffer size %d (requ %d)!" MEI_DRV_CRLF,
              MEI_DRV_LINENUM_GET(pMeiDev),
              pDbgStreamRead->dataBufferSize_byte, sizeof(EVT_DBG_DebugStream_t)));

      pDbgStreamRead->maxStreamEntries    = 0;
      pDbgStreamRead->dataBufferSize_byte = 0;

      return IFX_ERROR;
   }

   while (maxReads)
   {
      MEI_DRV_GET_UNIQUE_MAILBOX_ACCESS(pMeiDev);
      pInstDynDebugStream->bReadPending = IFX_TRUE;

      /* FIFO read mode - at least we need space to save one dbg stream element --> break */
      if ( (pDbgStreamRead->dataBufferSize_byte - readElemSizeAll_byte) < sizeof(EVT_DBG_DebugStream_t))
      {
         pInstDynDebugStream->bReadPending = IFX_FALSE;
         MEI_DRV_RELEASE_UNIQUE_MAILBOX_ACCESS(pMeiDev);

         break;
      }

      uFifoBufferRead.pULong = MEI_DRVOS_Var_Fifo_readElement(&pInstDynDebugStream->dbgStreamFifo, &readSize_byte);

      if ( (uFifoBufferRead.pVoid == IFX_NULL) || (readSize_byte == 0) )
      {
         /* FIFO empty */
         if (waitCnt == 0)
         {
            /* no timeout or timeout reached */
            pInstDynDebugStream->bReadPending = IFX_FALSE;
            MEI_DRV_RELEASE_UNIQUE_MAILBOX_ACCESS(pMeiDev);

            break;
         }

         MEI_DRVOS_Wait_ms(MEI_DBG_STREAMS_WAIT_POLL_TIME_MS);
         if (waitCnt != ~0)
         {
            waitCnt--;
         }
         pInstDynDebugStream->bReadPending = IFX_FALSE;
         MEI_DRV_RELEASE_UNIQUE_MAILBOX_ACCESS(pMeiDev);
         continue;
      }

      if ( (readElemSizeAll_byte + readSize_byte) > pDbgStreamRead->dataBufferSize_byte )
      {
         /* user buffer full - config error ? normally it should be enought (see check above) */
         MEI_DBG_STREAMS_STAT_MSG_ERR_DISCARD_CNT_INC(pInstDynDebugStream);
         MEI_DBG_STREAMS_STAT_DATA_ERR_DISCARD_SIZE_ADD(readSize_byte);

         pInstDynDebugStream->bReadPending = IFX_FALSE;
         MEI_DRV_RELEASE_UNIQUE_MAILBOX_ACCESS(pMeiDev);

         retVal = IFX_ERROR;
         break;
      }

#if (MEI_DRV_OS_BYTE_ORDER != MEI_DRV_OS_BIG_ENDIAN)
      /* Swap debug stream data to device order */
      {
         IFX_uint_t i;
         IFX_uint16_t *pData16 = uFifoBufferRead.pUInt16;

         for (i = 0; i < (readSize_byte / sizeof(IFX_uint16_t)) ; i++)
         {
            pData16[i] = SWAP16_BYTE_ORDER(pData16[i]);
         }
      }
#endif
      if (bInternCall == IFX_FALSE)
      {
         /* copy data to user space */
         if ( (MEI_DRVOS_CpyToUser(
                        uUserBufferWrite.pVoid,
                        uFifoBufferRead.pUInt8,
                        MEI_DBG_STREAMS_CAST(IFX_uint32_t, readSize_byte))) == IFX_NULL )
         {
            PRN_ERR_USR_NL( MEI_DBG_STREAM, MEI_DRV_PRN_LEVEL_ERR,
                  ("MEI_DBG_STREAM[%02d]: ERROR - IOCTL DataRead, cpy2usr!" MEI_DRV_CRLF,
                    MEI_DRV_LINENUM_GET(pMeiDev)));

            pInstDynDebugStream->bReadPending = IFX_FALSE;
            MEI_DRV_RELEASE_UNIQUE_MAILBOX_ACCESS(pMeiDev);

            retVal = IFX_ERROR;
            break;
         }
      }
      else
      {
         /* copy data to kernel space */
         memcpy(uUserBufferWrite.pUInt8,
                uFifoBufferRead.pUInt8,
                MEI_DBG_STREAMS_CAST(IFX_uint32_t, readSize_byte));
      }
      MEI_DBG_STREAMS_STAT_MSG_BUF2USR_CNT_INC(pInstDynDebugStream);
      MEI_DBG_STREAMS_STAT_DATA_BUF2USR_SIZE_ADD(pInstDynDebugStream, readSize_byte);

      uUserBufferWrite.pUInt8 += readSize_byte;
      readElemSizeAll_byte    += readSize_byte;
      readElemCnt++;

      pInstDynDebugStream->bReadPending = IFX_FALSE;
      MEI_DRV_RELEASE_UNIQUE_MAILBOX_ACCESS(pMeiDev);


      /* max entries set and exceeded --> break */
      if ( (pDbgStreamRead->maxStreamEntries != 0) && (readElemCnt >= pDbgStreamRead->maxStreamEntries) )
      {
         break;
      }

      if (pDbgStreamRead->maxStreamEntries != 0)
      {
         maxReads--;
      }
   }
#endif

   MEI_DBG_STREAMS_CTX_ACCESS_RELEASE(pMeiDynCntrl);

   pDbgStreamRead->maxStreamEntries    = readElemCnt;
   pDbgStreamRead->dataBufferSize_byte = readElemSizeAll_byte;

   return retVal;
}

/**
   Add a new dynamic debug stream receive struct to the device specific list.

\remarks
   To allow receiving notifications from the VRX device the current
   driver instance must be able to handle and store the incoming messages.
   The required buffers must be set in a "per open instance" way.
   Therefore the device instance manages a list of all open instances which are
   enabled to receive incoming debug streams

\param
   pMeiDynCntrl: Points to the VRX dynamic device control struct.
\param
   pVrxDynDbgStrm:   Points to the new dynamic debug stream handling struct.

\return
   - TRUE:  debug stream struct added to the device.
   - FALSE: ERROR - not able to add debug stream struct to the device.
*/
MEI_STATIC IFX_boolean_t MEI_DbgStreamAddToDevList( MEI_DYN_CNTRL_T         *pMeiDynCntrl,
                                                    MEI_DYN_DBG_STRM_DATA_T *pVrxDynDbgStrm)
{
   MEI_DEV_T          *pMeiDev;

   /*
      check params
   */
   if ( !pMeiDynCntrl || !pVrxDynDbgStrm )
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
             ("MEI_DRV [??]: MEI_DbgStreamAddToDevList - invalid params" MEI_DRV_CRLF) );
      return IFX_FALSE;
   }

   /* get VRX device struct */
   pMeiDev = pMeiDynCntrl->pMeiDev;

   PRN_DBG_USR( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL, MEI_DRV_LINENUM_GET(pMeiDev),
          ("MEI_DRV [%02d-%02d]: add the debug stream struct to the device list" MEI_DRV_CRLF,
           MEI_DRV_LINENUM_GET(pMeiDev), pMeiDynCntrl->openInstance) );

   /*
      Add the debug stream element to the end of the list.
   */
   pVrxDynDbgStrm->pNext = NULL;

   if (pMeiDev->pRootDbgStrmRecvLast)
   {
      /* list not empty */
      pVrxDynDbgStrm->pPrev                = pMeiDev->pRootDbgStrmRecvLast;
      pMeiDev->pRootDbgStrmRecvLast->pNext = pVrxDynDbgStrm;
      pMeiDev->pRootDbgStrmRecvLast        = pVrxDynDbgStrm;
   }
   else
   {
      /* list empty */
      pVrxDynDbgStrm->pPrev          = NULL;
      pMeiDev->pRootDbgStrmRecvLast  = pVrxDynDbgStrm;
      pMeiDev->pRootDbgStrmRecvFirst = pVrxDynDbgStrm;
   }

   return IFX_TRUE;
}

/**
   Remove a dynamic debug stream receive struct from the device specific list.

\remarks
   To allow receiving notifications from the VRX device the current
   driver instance must be able to handle and store the incoming messages.
   The required buffers must be set in a "per open instance" way.
   Therefore the device instance manages a list of all open instances which are
   enabled to receive incoming debug stream's

\param
   pMeiDynCntrl: Points to the VRX dynamic device control struct.
\param
   pVrxDynDbgStrm:   Points to the new dynamic debug stream handling struct.

\return
   - TRUE:  debug stream struct added to the device.
   - FALSE: ERROR - not able to add debug stream struct to the device.
*/
MEI_STATIC IFX_boolean_t MEI_DbgStreamRemoveFromDevList( MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                                                         MEI_DYN_DBG_STRM_DATA_T **ppVrxDynDbgStrm)
{
   MEI_DEV_T               *pMeiDev;
   MEI_DYN_DBG_STRM_DATA_T *pDynDbgStrm;

   /*
      check params, check debug stream enabled
   */
   if ( !pMeiDynCntrl || !ppVrxDynDbgStrm )
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
             ("MEI_DRV [??]: RemoveDbgStrmFromDevList - invalid params" MEI_DRV_CRLF) );

      return IFX_FALSE;
   }

   /* get VRX device struct */
   pMeiDev = pMeiDynCntrl->pMeiDev;

   if ( (pDynDbgStrm = pMeiDynCntrl->pInstDynDebugStream) == NULL )
   {
      /* debug stream struct not available (not enabled) */
      PRN_DBG_USR( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL, MEI_DRV_LINENUM_GET(pMeiDev),
             ("MEI_DRV [%02d-%02d]: RemoveDbgStrmFromDevList - no debug stream struct" MEI_DRV_CRLF,
              MEI_DRV_LINENUM_GET(pMeiDev), pMeiDynCntrl->openInstance) );

      return IFX_FALSE;
   }

   PRN_DBG_USR( MEI_DRV, MEI_DRV_PRN_LEVEL_LOW, MEI_DRV_LINENUM_GET(pMeiDev),
          ("MEI_DRV [%02d-%02d]: remove the debug stream struct from the device list" MEI_DRV_CRLF,
           MEI_DRV_LINENUM_GET(pMeiDev), pMeiDynCntrl->openInstance) );

   /*
      Dechain the debug stream element from the list.
   */
   if (pDynDbgStrm->pPrev != NULL)
   {
      pDynDbgStrm->pPrev->pNext = pDynDbgStrm->pNext;
   }
   else
   {
      /* begin of list */
      pMeiDev->pRootDbgStrmRecvFirst = pDynDbgStrm->pNext;
   }

   if (pDynDbgStrm->pNext != NULL)
   {
      pDynDbgStrm->pNext->pPrev = pDynDbgStrm->pPrev;
   }
   else
   {
      /* end of list */
      pMeiDev->pRootDbgStrmRecvLast = pDynDbgStrm->pPrev;
   }

   /* remove it form dynamic control struct and return it back */
   pMeiDynCntrl->pInstDynDebugStream = NULL;
   *ppVrxDynDbgStrm = pDynDbgStrm;
   
   return IFX_TRUE;
}

/**
   Distribute a message over all open instances.

\param
   pMeiDev: Points to the VRX device struct.
\param
   pNfcRootInstance: points to the root of the open instance list.
\param
   pMsg:    Points to the "Driver" message.
\param
   msgSize: size of the "Driver" message.
\param:
   msgType: type of the driver message.

\return
   Number of distributed messages.
*/
MEI_STATIC IFX_uint32_t MEI_DbgStreamSendNotify(
                              MEI_DEV_T          *pMeiDev,
                              MEI_DYN_NFC_DATA_T *pNfcInstance,
                              CMV_STD_MESSAGE_T  *pNotifyMsg)
{
   IFX_uint32_t distCount = 0, recvTime, processCtrl;
   
   recvTime = MEI_DRVOS_GetElapsedTime_ms(0);
   
   /* Restrict lenght of the message to cut the payload data */
   pNotifyMsg->header.length = 0;
   pNotifyMsg->header.paylCntrl &= ~CMV_MSGHDR_PAYLOAD_SIZE_MASK;

   processCtrl = (((IFX_uint32_t)(((MEI_CMV_MAILBOX_T *)pNotifyMsg)->cmv.header.mbxCode)) & 0x0000FFFF);
   
   /* check if this autonomous message is enabled */
   if (pNfcInstance->msgProcessCtrl & processCtrl)
   {
      if (pNfcInstance->pRecvDataCntrl[pNfcInstance->rdIdxWr].bufCtrl == MEI_RECV_BUF_CTRL_FREE)
      {
         memcpy( pNfcInstance->pRecvDataCntrl[pNfcInstance->rdIdxWr].recvDataBuf_s.pBuffer, 
                 pNotifyMsg, 
                 sizeof(CMV_STD_MESSAGE_HEADER_T));
         pNfcInstance->pRecvDataCntrl[pNfcInstance->rdIdxWr].msgLen   = sizeof(CMV_STD_MESSAGE_HEADER_T);
         pNfcInstance->pRecvDataCntrl[pNfcInstance->rdIdxWr].bufCtrl  = MEI_RECV_BUF_CTRL_MODEM_NFC_MSG;
         pNfcInstance->pRecvDataCntrl[pNfcInstance->rdIdxWr].recvTime = recvTime;

         /* increase write index of buffer */
         pNfcInstance->rdIdxWr =
            (pNfcInstance->rdIdxWr < (pNfcInstance->numOfBuf -1)) ? (pNfcInstance->rdIdxWr+1) : 0;

         /* inform sleeping processes */
         if (pMeiDev->bNfcNeedWakeUp)
         {
            MEI_DRVOS_SelectQueueWakeUp(
                  &pMeiDev->selNfcWakeupList,
                  MEI_DRVOS_SEL_WAKEUP_TYPE_RD);

            pMeiDev->bNfcNeedWakeUp = IFX_FALSE;

            PRN_DBG_INT( MEI_DRV, MEI_DRV_PRN_LEVEL_LOW, MEI_DRV_LINENUM_GET(pMeiDev),
                 ("MEI_DRV[%02d]: WakeUp processes - <msgType 0x%08X>" MEI_DRV_CRLF,
                 MEI_DRV_LINENUM_GET(pMeiDev), MEI_RECV_BUF_CTRL_DRIVER_MSG));
         }
         else
         {
            PRN_DBG_INT( MEI_DRV, MEI_DRV_PRN_LEVEL_LOW, MEI_DRV_LINENUM_GET(pMeiDev),
                   ("MEI_DRV[%02d]: skip WakeUp processes - <msgType 0x%08X>" MEI_DRV_CRLF,
                   MEI_DRV_LINENUM_GET(pMeiDev), MEI_RECV_BUF_CTRL_DRIVER_MSG));
         }
      }
   }

   return distCount;
}

#if (MEI_DRV_IFXOS_ENABLE == 0)
/**
   Initializes the variable-sized fifo structure
   \param *pFifo - Pointer to the MEI_DRVOS_VFIFO structure
   \param *pStart - Pointer to the fifo first element (IFX_ulong_t aligned)
   \param *pEnd - Pointer to the first address beyond the last fifo element (IFX_ulong_t aligned)
   \param maxElSize - maximum allowed size of an element in bytes
   \return
   Always zero, otherwise error
*/
MEI_STATIC IFX_return_t MEI_DRVOS_Var_Fifo_Init (MEI_DRVOS_VFIFO* pFifo, IFX_ulong_t* pStart,
                          IFX_ulong_t* pEnd, IFX_uint32_t maxElSize)
{
   if (pFifo == IFX_NULL)
      return IFX_ERROR;

   /* Check if pStart and pEnd are word aligned pointers */
   {
      if ((IFX_ulong_t)pStart % sizeof(IFX_ulong_t) != 0)
      {
         PRN_ERR_USR_NL(MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
               ("MEI_DRV(FIFO): ERROR - pStart is not unsigned long aligned (0x%lX)!!" MEI_DRV_CRLF,
               (IFX_ulong_t)pStart));
         return IFX_ERROR;
      }
      if ((IFX_ulong_t)pEnd % sizeof(IFX_ulong_t) != 0)
      {
         PRN_ERR_USR_NL(MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
               ("MEI_DRV(FIFO): ERROR - pEnd is not unsigned long aligned (0x%lX)!!" MEI_DRV_CRLF,
               (IFX_ulong_t)pEnd));
         return IFX_ERROR;
      }
   }

   pFifo->pEnd   = pEnd;
   pFifo->pStart = pStart;
   pFifo->pRead  = pStart;
   pFifo->pWrite = pStart;
   pFifo->size   = TO_ULONG_SIZE(maxElSize);
   pFifo->count  = 0;
   pFifo->max_size = 0;

   return IFX_SUCCESS;
}

/**
   Clears the variable-sized fifo
   \param *pFifo - Pointer to the Fifo structure
*/
MEI_STATIC IFX_void_t MEI_DRVOS_Var_Fifo_Clear (MEI_DRVOS_VFIFO *pFifo)
{
   pFifo->pRead  = pFifo->pStart;
   pFifo->pWrite = pFifo->pStart;
   pFifo->count = 0;
}

/**
   Get the next element to read from variable-sized fifo
   \param *pFifo - Pointer to the Fifo structure
   \return Returns the element address (IFX_ulong_t aligned) to read from,
           or IFX_NULL if no element available or an error occured
   \remark Error occurs if fifo is empty
*/
MEI_STATIC IFX_ulong_t* MEI_DRVOS_Var_Fifo_readElement (MEI_DRVOS_VFIFO *pFifo, IFX_uint32_t *elSizeB)
{
   IFX_ulong_t *ret = IFX_NULL;
   IFX_ulong_t elSizeUL;

   if (elSizeB)
      *elSizeB = 0;

   if ( (pFifo->pRead < pFifo->pStart) || (pFifo->pRead > pFifo->pEnd))
   {
      PRN_ERR_USR_NL(MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DRV(FIFO): ERROR - enter var read: pRead: 0x%lX out of range pStart: 0x%lX, pEnd: 0x%lX, !!" MEI_DRV_CRLF,
            (IFX_ulong_t)pFifo->pRead, (IFX_ulong_t)pFifo->pStart, (IFX_ulong_t)pFifo->pEnd ));

      return IFX_NULL;
   }

   if (pFifo->count == 0)
   {
      return IFX_NULL;
   }

   if ((pFifo->pRead[0] == (IFX_ulong_t)~0) ||
       ((pFifo->pEnd - pFifo->pRead) <= (SIZE_HEADER + SIZE_TRAILER)))
   {
      pFifo->pRead = pFifo->pStart;
   }

   elSizeUL = TO_ULONG_SIZE(pFifo->pRead[0]);

   if ( (pFifo->pRead + (SIZE_HEADER + SIZE_TRAILER) + elSizeUL) > pFifo->pEnd)
   {
      PRN_ERR_USR_NL(MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DRV(FIFO): ERROR - var read: overflow, pRead: 0x%lX, pEnd: 0x%lX, elSize: 0x%08lX (+ 0x%X) !!" MEI_DRV_CRLF,
            (IFX_ulong_t)pFifo->pRead, (IFX_ulong_t)pFifo->pEnd, elSizeUL, (SIZE_HEADER + SIZE_TRAILER)));

      return IFX_NULL;
   }

#if (SIZE_TRAILER == 1)
   if (pFifo->pRead[elSizeUL + SIZE_HEADER] != SIZE_TRAILER_VALUE)
   {
      PRN_ERR_USR_NL(MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DRV(FIFO): ERROR - var read: overwrite occured: 0x%lX, pEnd: 0x%lX, elSize: 0x%08lX (+ 0x%X) !!" MEI_DRV_CRLF,
            (IFX_ulong_t)pFifo->pRead, (IFX_ulong_t)pFifo->pEnd, elSizeUL, (SIZE_HEADER + SIZE_TRAILER)));

      pFifo->pRead[elSizeUL + SIZE_HEADER] = (IFX_ulong_t)~0;
   }
#endif

   if (elSizeB)
      *elSizeB = (IFX_uint32_t)(pFifo->pRead[0]);

   ret = pFifo->pRead + SIZE_HEADER;
   pFifo->pRead += (SIZE_HEADER + SIZE_TRAILER) + elSizeUL;

   pFifo->count--;

   if ( (pFifo->pRead < pFifo->pStart) || (pFifo->pRead > pFifo->pEnd))
   {
      PRN_ERR_USR_NL(MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DRV(FIFO): ERROR - leave var read: pRead: 0x%lX out of range pStart: 0x%lX, pEnd: 0x%lX, !!" MEI_DRV_CRLF,
            (IFX_ulong_t)pFifo->pRead, (IFX_ulong_t)pFifo->pStart, (IFX_ulong_t)pFifo->pEnd ));

      return IFX_NULL;
   }

   return ret;
}

/**
   Peek the next element to read from variable-sized fifo
   \remark To free the element use the corresponding read function.

   \param *pFifo - Pointer to the Fifo structure
   \return Returns the element address (IFX_ulong_t aligned) to read from,
           or IFX_NULL if no element available or an error occured
   \remark Error occurs if fifo is empty
*/
MEI_STATIC IFX_ulong_t* MEI_DRVOS_Var_Fifo_peekElement (MEI_DRVOS_VFIFO *pFifo, IFX_uint32_t *elSizeB)
{
   IFX_ulong_t *ret = IFX_NULL, *pPeekRead = pFifo->pRead;
   IFX_ulong_t elSizeUL;

   if (elSizeB)
      *elSizeB = 0;

   if ( (pFifo->pRead < pFifo->pStart) || (pFifo->pRead > pFifo->pEnd))
   {
      PRN_ERR_USR_NL(MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DRV(FIFO): ERROR - enter var peek: pRead: 0x%lX out of range pStart: 0x%lX, pEnd: 0x%lX, !!" MEI_DRV_CRLF,
            (IFX_ulong_t)pFifo->pRead, (IFX_ulong_t)pFifo->pStart, (IFX_ulong_t)pFifo->pEnd ));

      return IFX_NULL;
   }

   if (pFifo->count == 0)
   {
      return IFX_NULL;
   }

   if ((pPeekRead[0] == (IFX_ulong_t)~0) ||
       ((pFifo->pEnd - pPeekRead) <= (SIZE_HEADER + SIZE_TRAILER)))
   {
      pPeekRead = pFifo->pStart;
   }

   elSizeUL = TO_ULONG_SIZE(pPeekRead[0]);

   if ( (pPeekRead + (SIZE_HEADER + SIZE_TRAILER) + elSizeUL) > pFifo->pEnd)
   {
      PRN_ERR_USR_NL(MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DRV(FIFO): ERROR - var peek: overflow, pRead: 0x%lX, pEnd: 0x%lX, elSize: 0x%08lX (+ 0x%X) !!" MEI_DRV_CRLF,
            (IFX_ulong_t)pPeekRead, (IFX_ulong_t)pFifo->pEnd, elSizeUL, (SIZE_HEADER + SIZE_TRAILER)));

      return IFX_NULL;
   }

#if (SIZE_TRAILER == 1)
   if (pPeekRead[elSizeUL + SIZE_HEADER] != SIZE_TRAILER_VALUE)
   {
      PRN_ERR_USR_NL(MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DRV(FIFO): ERROR - var peek: overwrite occured: 0x%lX, pEnd: 0x%lX, elSize: 0x%08lX (+ 0x%X) !!" MEI_DRV_CRLF,
            (IFX_ulong_t)pPeekRead, (IFX_ulong_t)pFifo->pEnd, elSizeUL, (SIZE_HEADER + SIZE_TRAILER)));

      pPeekRead[elSizeUL + SIZE_HEADER] = (IFX_ulong_t)~0;
   }
#endif

   if (elSizeB)
      *elSizeB = (IFX_uint32_t)(pPeekRead[0]);

   ret = pPeekRead + SIZE_HEADER;

   if ( (pFifo->pRead < pFifo->pStart) || (pFifo->pRead > pFifo->pEnd))
   {
      PRN_ERR_USR_NL(MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DRV(FIFO): ERROR - leave var peek: pRead: 0x%lX out of range pStart: 0x%lX, pEnd: 0x%lX, !!" MEI_DRV_CRLF,
            (IFX_ulong_t)pFifo->pRead, (IFX_ulong_t)pFifo->pStart, (IFX_ulong_t)pFifo->pEnd ));

      return IFX_NULL;
   }

   return ret;
}

/**
   Get the next element to write to
   \param *pFifo - Pointer to the Fifo structure
   \param size - Size of a new element to be written in bytes
   \return
   Returns the element address to write to (IFX_ulong_t aligned), or IFX_NULL in case of error
   \remark
   Error occurs if the write pointer reaches the read pointer, meaning
   the fifo if full and would otherwise overwrite the next element.
*/
MEI_STATIC IFX_ulong_t* MEI_DRVOS_Var_Fifo_writeElement (MEI_DRVOS_VFIFO *pFifo, IFX_uint32_t elSizeB)
{
   IFX_ulong_t* ret = IFX_NULL;
   IFX_ulong_t elSizeUL = TO_ULONG_SIZE (elSizeB);

   if ( (pFifo->pWrite < pFifo->pStart) || (pFifo->pWrite >= pFifo->pEnd))
   {
      PRN_ERR_USR_NL(MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DRV(FIFO): ERROR - enter var write: pWrite: 0x%lX out of range pStart: 0x%lX, pEnd: 0x%lX, !!" MEI_DRV_CRLF,
            (IFX_ulong_t)pFifo->pWrite, (IFX_ulong_t)pFifo->pStart, (IFX_ulong_t)pFifo->pEnd ));

      return IFX_NULL;
   }

   if (elSizeUL > MEI_DRVOS_Var_Fifo_getRoom (pFifo) || elSizeUL > pFifo->size)
   {
      return IFX_NULL;
   }
   else
   {
      if (pFifo->pWrite >= pFifo->pRead &&
         (elSizeUL + SIZE_HEADER + SIZE_TRAILER) > (IFX_ulong_t)(pFifo->pEnd - pFifo->pWrite))
      {
         /* check if we have enough room from the head */
         if ((elSizeUL + SIZE_HEADER + SIZE_TRAILER) > (IFX_ulong_t)(pFifo->pRead - pFifo->pStart))
         {
            /* if FIFO is empty */
            if (pFifo->pWrite == pFifo->pRead)
            {
               /* ... sync to FIFO start */
               pFifo->pRead = pFifo->pStart;
            }
            else
            {
               return IFX_NULL;
            }
         }
         if ((pFifo->pEnd - pFifo->pWrite) >= SIZE_HEADER)
         {
            pFifo->pWrite[0] = (IFX_ulong_t)~0;
         }
         pFifo->pWrite = pFifo->pStart;
      }

      pFifo->pWrite[0] = elSizeB;
#if (SIZE_TRAILER == 1)
      pFifo->pWrite[elSizeUL + SIZE_HEADER] = SIZE_TRAILER_VALUE;
#endif

      ret              = pFifo->pWrite + SIZE_HEADER;

      pFifo->pWrite   += elSizeUL + (SIZE_HEADER + SIZE_TRAILER);
      if (pFifo->pWrite == pFifo->pEnd)
         pFifo->pWrite = pFifo->pStart;


      if ( (pFifo->pWrite < pFifo->pStart) || (pFifo->pWrite >= pFifo->pEnd))
      {
         PRN_ERR_USR_NL(MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
               ("MEI_DRV(FIFO): ERROR - leave var write: pWrite: 0x%lX out of range pStart: 0x%lX, pEnd: 0x%lX, !!" MEI_DRV_CRLF,
               (IFX_ulong_t)pFifo->pWrite, (IFX_ulong_t)pFifo->pStart, (IFX_ulong_t)pFifo->pEnd ));

         return IFX_NULL;
      }

      pFifo->count++;
      return ret;
   }
}

/**
   Get the number of stored elements
   \param *pFifo - Pointer to the Fifo structure
   \return
   Number of containing elements
*/
MEI_STATIC IFX_uint32_t MEI_DRVOS_Var_Fifo_getCount(MEI_DRVOS_VFIFO *pFifo)
{
   return pFifo->count;
}

/**
 * Returns size of free room in the variable-sized IFX_FIFO.
 *    \param *pFifo - Pointer to the Fifo structure
 *    \return
 *    The size of free room in the IFX_FIFO, in IFX_ulong_t integers.
 */
MEI_STATIC IFX_uint32_t MEI_DRVOS_Var_Fifo_getRoom (MEI_DRVOS_VFIFO *pFifo)
{
   IFX_long_t diff = pFifo->pWrite - pFifo->pRead;
   IFX_long_t headRoom, tailRoom;

   if (diff == 0)
   {
      if (pFifo->count)
         return 0;
      else
      {
         return (pFifo->pEnd - pFifo->pStart - (SIZE_HEADER + SIZE_TRAILER));
      }
   }
   if (diff > 0)
   {
      tailRoom = pFifo->pEnd  - pFifo->pWrite - (SIZE_HEADER + SIZE_TRAILER);
      headRoom = pFifo->pRead - pFifo->pStart - (SIZE_HEADER + SIZE_TRAILER);

      return (headRoom > tailRoom) ? headRoom : tailRoom;
   }
   else
   {
      if ((-diff) <= (SIZE_HEADER + SIZE_TRAILER) )
         return 0;
      else
         return ((-diff) - (SIZE_HEADER + SIZE_TRAILER) );
   }
}

#endif /* (MEI_DRV_IFXOS_ENABLE == 0) */

#endif      /* #if (MEI_SUPPORT_DEBUG_STREAMS == 1) */


