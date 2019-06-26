/******************************************************************************

                              Copyright (c) 2009
                            Lantiq Deutschland GmbH
                     Am Campeon 3; 85579 Neubiberg, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/** \file
   This file contains the functions for the DTI Command Line Interface handling
*/

/* ============================================================================
   Includes
   ========================================================================= */

#include "dti_osmap.h"

#include "ifx_dti_protocol.h"
#include "ifx_dti_protocol_cli.h"
#include "dti_protocol_interface.h"

#include "dti_agent_interface.h"
#include "dti_control.h"

#include "dti_cli_interface.h"
#include "dti_cli.h"


/* ============================================================================
   Local defines
   ========================================================================= */
#ifdef DTI_STATIC
#undef DTI_STATIC
#endif

#ifdef DTI_DEBUG
#define DTI_STATIC
#else
#define DTI_STATIC   static
#endif

/* ============================================================================
   Local variables
   ========================================================================= */
#if defined(INCLUDE_CLI_SUPPORT) && (INCLUDE_CLI_SUPPORT == 1)

/* Create device debug module - user part */
IFXOS_PRN_USR_MODULE_CREATE(DTI_CLI, DTI_PRN_LEVEL_HIGH);

#endif

/* ============================================================================
   Local Function declarations
   ========================================================================= */
#if defined(INCLUDE_CLI_SUPPORT) && (INCLUDE_CLI_SUPPORT == 1)

DTI_STATIC IFX_int_t DTI_CLI_DefaultLoop_Send(
                        IFX_void_t        *pCallbackContext,
                        const IFX_char_t  *pCmdIn,
                        IFX_char_t        *pResultOut,
                        IFX_int_t         *pResultBufSize_byte,
                        IFX_int_t         *pResultCode);

DTI_STATIC IFX_int_t DTI_CLI_singleInterface_Stop(
                        DTI_cli_control_t *pCliControl);

DTI_STATIC IFX_int_t DTI_CLI_workerEventControl_create(
                        DTI_CliEventCtx_t    *pCliEventCtx,
                        IFX_uint_t           eventBufferSize_byte,
                        IFX_uint_t           maxEventSize_byte);

DTI_STATIC IFX_int_t DTI_CLI_workerEventControl_delete(
                        DTI_AgentCtx_t       *pAgentCtx,
                        IFX_int_t            workerNum);

DTI_STATIC IFX_int_t  DTI_CLI_ConfigGet_Buffer(
                        DTI_AgentCtx_t          *pAgentCtx,
                        DTI_cli_control_t       *pCliControl,
                        DTI_ProtocolServerCtx_t *pDtiProtServerCtx,
                        DTI_CliEventCtx_t       *pCliEventCtx,
                        DTI_PacketError_t       *pPacketError,
                        IFX_uint_t              bufferIdx);

DTI_STATIC IFX_int_t DTI_packet_CliString(
                        DTI_AgentCtx_t          *pAgentCtx,
                        DTI_ProtocolServerCtx_t *pDtiProtServerCtx,
                        const DTI_Packet_t      *pDtiPacketIn,
                        IFX_uint_t              dtiBufInLen,
                        DTI_Packet_t            **ppDtiPacketOut,
                        IFX_uint32_t            bufferOutSize);

DTI_STATIC IFX_int_t DTI_packet_CliInfoGet(
                        DTI_AgentCtx_t          *pAgentCtx,
                        DTI_ProtocolServerCtx_t *pDtiProtServerCtx,
                        const DTI_Packet_t      *pDtiPacketIn,
                        DTI_Packet_t            *pDtiPacketOut,
                        IFX_uint32_t            bufferOutSize);

DTI_STATIC IFX_int_t DTI_CLI_eventWrite(
                        DTI_AgentCtx_t    *pAgentCtx,
                        DTI_WorkerCtx_t   *pWorkerCtx,
                        DTI_cli_control_t *pCliControl,
                        IFX_char_t        *pEventOut,
                        IFX_uint_t        eventOutSize_byte);

DTI_STATIC IFX_int_t DTI_CLI_SendFunctionRegisterCommon(
                                 IFX_void_t                           *pDtiAgentContext,
                                 IFX_void_t                           *pCallbackContext,
                                 const IFX_char_t                     *pIfName,
                                 DTI_CliSendCommandFunc_t             pSendFct,
                                 DTI_CliSendFragmentedCommandFunc_t   pSendFragFct,
                                 IFX_uint_t                           responceBufferSize);

#endif      /* #if defined(INCLUDE_CLI_SUPPORT) && (INCLUDE_CLI_SUPPORT == 1) */

/* ============================================================================
   Local Function definitons
   ========================================================================= */
#if defined(INCLUDE_CLI_SUPPORT) && (INCLUDE_CLI_SUPPORT == 1)

/**
   Local loop CLI function
*/
DTI_STATIC IFX_int_t DTI_CLI_DefaultLoop_Send(
                        IFX_void_t        *pCallbackContext,
                        const IFX_char_t  *pCmdIn,
                        IFX_char_t        *pResultOut,
                        IFX_int_t         *pResultBufSize_byte,
                        IFX_int_t         *pResultCode)
{
   IFX_int_t   resultCode = DTI_SUCCESS, writtenBytes = 0;
   IFX_char_t  cliBuf[64];
   if (pCmdIn)
   {
      DTI_PRN_USR_DBG_NL(DTI_CLI, DTI_PRN_LEVEL_NORMAL,
         ("DTI CLI Default Loop - IN <%s>" DTI_CRLF, pCmdIn));

      (void)DTI_StrNCpy(cliBuf, pCmdIn, 63);
   }
   else
   {
      DTI_PRN_USR_ERR_NL(DTI_CLI, DTI_PRN_LEVEL_ERR,
         ("DTI CLI Default Loop - IN <missing cli command>" DTI_CRLF));

      (void)DTI_snprintf(cliBuf, 63,
               "missing cli command");

      resultCode = IFX_ERROR;
   }

   cliBuf[63] = '\0';   /* terminate */

   if (pResultOut && pResultBufSize_byte)
   {
      writtenBytes = DTI_snprintf(pResultOut, *pResultBufSize_byte,
         "DTI CLI Default Loop - RESULT <%s>", cliBuf);
   }
   else
   {
      DTI_PRN_USR_DBG_NL(DTI_CLI, DTI_PRN_LEVEL_HIGH,
         ("DTI CLI Default Loop - RESULT <missing return buffer> <%s>" DTI_CRLF,
           cliBuf));

      resultCode = IFX_ERROR;
   }

   if (pResultBufSize_byte)
   {
      *pResultBufSize_byte = writtenBytes + 1;
   }

   if (pResultCode)
   {
      *pResultCode = resultCode;
   }

   return resultCode;
}

/**
   Stops and release the given CLI interface.

\param
   pCliControl - points to the CLI interface context.

return
   IFX_SUCCESS stop done.
   IFX_ERROR   stop failed.

*/
DTI_STATIC IFX_int_t DTI_CLI_singleInterface_Stop(
                        DTI_cli_control_t *pCliControl)
{
   if (pCliControl != IFX_NULL)
   {
      if (IFXOS_LOCK_INIT_VALID(&pCliControl->cliResponceBufLock))
         {DTI_LockGet(&pCliControl->cliResponceBufLock);}

      if (pCliControl->bRdyForCliSend == IFX_FALSE)
      {
         DTI_PRN_USR_ERR_NL(DTI_CLI, DTI_PRN_LEVEL_WRN,
            ("WARNING: CLI IF STOP - IF[%d] already disabled" DTI_CRLF,
              pCliControl->cliIfNum));
      }
      else
      {
         DTI_PRN_USR_DBG_NL(DTI_CLI, DTI_PRN_LEVEL_HIGH,
            ("CLI IF STOP - CLI IF[%d] name = %s" DTI_CRLF,
              pCliControl->cliIfNum, pCliControl->cliIfName));
      }

      pCliControl->bRdyForCliSend   = IFX_FALSE;
      pCliControl->pSendFunct       = IFX_NULL;
      pCliControl->pSendFragFunct   = IFX_NULL;
      pCliControl->pCallbackContext = IFX_NULL;

      (void)DTI_snprintf(pCliControl->cliIfName, DTI_CLI_MAX_NAME_LEN,
                         "not_used");

      if (pCliControl->pCliResponceBuffer != IFX_NULL)
      {
         DTI_Free(pCliControl->pCliResponceBuffer);
         pCliControl->pCliResponceBuffer = IFX_NULL;
      }
      pCliControl->responceBufferSize = 0;

      if (IFXOS_LOCK_INIT_VALID(&pCliControl->cliResponceBufLock))
         {DTI_LockRelease(&pCliControl->cliResponceBufLock);}
   }
   else
   {
      DTI_PRN_USR_ERR_NL(DTI_CLI, DTI_PRN_LEVEL_ERR,
         ("ERROR: CLI IF STOP - missing CLI IF context" DTI_CRLF));

      return IFX_ERROR;
   }

   return IFX_SUCCESS;
}

/**
   Create a worker thread CLI Event control struct
   - allocate fifo buffer
   - setup fifo.
*/
DTI_STATIC IFX_int_t DTI_CLI_workerEventControl_create(
                        DTI_CliEventCtx_t    *pCliEventCtx,
                        IFX_uint_t           eventBufferSize_byte,
                        IFX_uint_t           maxEventSize_byte)
{
   IFX_uint_t  eventBufferSize_ulong;
   DTI_PTR_U   uEvtBuffer;
   if (pCliEventCtx->pEventFifoBuffer != IFX_NULL)
   {
      DTI_PRN_USR_ERR_NL(DTI_CLI, DTI_PRN_LEVEL_WRN,
         ("WARNING: CLI Worker Event Control Create - already exist."DTI_CRLF ));

      return IFX_SUCCESS;
   }

   pCliEventCtx->bAutoCliMsgActive = IFX_FALSE;

   eventBufferSize_ulong = eventBufferSize_byte / sizeof(IFX_ulong_t) +
                           ((eventBufferSize_byte % sizeof(IFX_ulong_t)) ? 1 : 0);

   pCliEventCtx->eventFifoBufferSize_byte = (eventBufferSize_ulong * sizeof(IFX_ulong_t));

   if ( (pCliEventCtx->eventFifoBufferSize_byte == 0) ||
        (pCliEventCtx->eventFifoBufferSize_byte >= DTI_CLI_EVENT_MAX_BUFFER) )
   {
      DTI_PRN_USR_ERR_NL(DTI_CLI, DTI_PRN_LEVEL_ERR,
         ("Error: CLI Worker Event Control Create - invalid event buffer size %d."DTI_CRLF,
           pCliEventCtx->eventFifoBufferSize_byte ));

      pCliEventCtx->eventFifoBufferSize_byte = 0;
      pCliEventCtx->maxEventSize_byte        = 0;

      return IFX_ERROR;
   }

   uEvtBuffer.pUInt8 = DTI_Malloc(pCliEventCtx->eventFifoBufferSize_byte);
   if ( (pCliEventCtx->pEventFifoBuffer = DTI_PTR_CAST_GET_ULONG(uEvtBuffer)) == IFX_NULL)
   {
      DTI_PRN_USR_ERR_NL(DTI_CLI, DTI_PRN_LEVEL_ERR,
         ("Error: CLI Worker Event Control Create - event buffer allocation (size %d), %s "DTI_CRLF,
           pCliEventCtx->eventFifoBufferSize_byte,
           (uEvtBuffer.pUInt8 == IFX_NULL) ? "failed" : "missaligned" ));

      if (uEvtBuffer.pUInt8 != IFX_NULL)
      {
         DTI_Free(uEvtBuffer.pUInt8);
      }
      pCliEventCtx->eventFifoBufferSize_byte = 0;
      pCliEventCtx->maxEventSize_byte        = 0;

      return IFX_ERROR;
   }

   if (DTI_Var_Fifo_Init(
               &pCliEventCtx->eventFifo,
               &uEvtBuffer.pULong[0], &uEvtBuffer.pULong[eventBufferSize_ulong],
               (maxEventSize_byte + sizeof(DTI_PacketHeader_t))) != IFX_SUCCESS)
   {

      DTI_PRN_USR_ERR_NL(DTI_CLI, DTI_PRN_LEVEL_ERR,
         ("Error: CLI Worker Event Control Create - fifo init"DTI_CRLF ));

      DTI_Free(uEvtBuffer.pUInt8);
      pCliEventCtx->eventFifoBufferSize_byte = 0;
      pCliEventCtx->maxEventSize_byte        = 0;
      pCliEventCtx->pEventFifoBuffer         = IFX_NULL;

      return IFX_ERROR;
   }
   pCliEventCtx->maxEventSize_byte = maxEventSize_byte;

   DTI_PRN_USR_DBG_NL(DTI_CLI, DTI_PRN_LEVEL_HIGH,
      ("CLI Worker Event Control Create - done"DTI_CRLF ));

   return IFX_SUCCESS;
}

/**
   Create a worker thread CLI Event control struct
   - allocate fifo buffer
   - setup fifo.
*/
DTI_STATIC IFX_int_t DTI_CLI_workerEventControl_delete(
                        DTI_AgentCtx_t       *pAgentCtx,
                        IFX_int_t            workerNum)
{
   IFX_int_t            cliIfIdx;
   DTI_cli_control_t    *pCliIf;
   DTI_CliEventCtx_t    *pCliEventCtx = &pAgentCtx->pWorker[workerNum]->cliEventCtx;
   /* disable this worker on all CLI's */
   for (cliIfIdx = 0; cliIfIdx < DTI_CLI_MAX_CLI_INTERFACES; cliIfIdx++)
   {
      pCliIf = &pAgentCtx->cliControl[cliIfIdx];
      pCliIf->bAcceptEvents[workerNum] = IFX_FALSE;
   }

   pCliEventCtx->bAutoCliMsgActive = IFX_FALSE;

   if (pCliEventCtx->pEventFifoBuffer == IFX_NULL)
   {
      DTI_PRN_USR_ERR_NL(DTI_CLI, DTI_PRN_LEVEL_WRN,
         ("WARNING: CLI Worker Event Control Delete - DTI Worker[%d] FIFO not exist."DTI_CRLF,
           workerNum));

      return IFX_SUCCESS;
   }

   DTI_PRN_USR_DBG_NL(DTI_CLI, DTI_PRN_LEVEL_HIGH,
      ("CLI Worker Event Control Delete - ."DTI_CRLF ));

   DTI_Var_Fifo_Clear(&pCliEventCtx->eventFifo);
   DTI_MemSet(&pCliEventCtx->eventFifo, 0x00, sizeof(IFX_VFIFO));

   DTI_Free(pCliEventCtx->pEventFifoBuffer);
   pCliEventCtx->pEventFifoBuffer         = IFX_NULL;
   pCliEventCtx->eventFifoBufferSize_byte = 0;
   pCliEventCtx->maxEventSize_byte        = 0;

   return IFX_SUCCESS;
}

/**
 This function registers a corresponding send function within the DTI.
 */
DTI_STATIC IFX_int_t DTI_CLI_SendFunctionRegisterCommon(
                                 IFX_void_t                           *pDtiAgentContext,
                                 IFX_void_t                           *pCallbackContext,
                                 const IFX_char_t                     *pIfName,
                                 DTI_CliSendCommandFunc_t             pSendFct,
                                 DTI_CliSendFragmentedCommandFunc_t   pSendFragFct,
                                 IFX_uint_t                           responceBufferSize)
{
#if defined(INCLUDE_CLI_SUPPORT) && (INCLUDE_CLI_SUPPORT == 1)
   IFX_int_t         ifIdx;
   DTI_AgentCtx_t    *pAgentCtx = (DTI_AgentCtx_t *)pDtiAgentContext;
   DTI_cli_control_t *pCliControl = IFX_NULL;
   if (!pAgentCtx)
   {
      DTI_PRN_USR_ERR_NL(DTI_CLI, DTI_PRN_LEVEL_ERR,
         ("Error: CLI Send Fct Register - missing agent context" DTI_CRLF));

      return DTI_ERR_NULL_PTR_ARG;
   }

   if (!pIfName || (!pSendFct && !pSendFragFct))
   {
      DTI_PRN_USR_ERR_NL(DTI_CLI, DTI_PRN_LEVEL_ERR,
         ("Error: CLI Send Fct Register - missing args" DTI_CRLF));

      return DTI_ERR_NULL_PTR_ARG;
   }

   if (pAgentCtx->bListenRun != IFX_TRUE)
   {
      DTI_PRN_USR_ERR_NL(DTI_CLI, DTI_PRN_LEVEL_WRN,
         ("WARNING: CLI Send Fct Register - agent not running" DTI_CRLF));
   }

   pCliControl = &pAgentCtx->cliControl[0];

   for (ifIdx = 0; ifIdx < DTI_CLI_MAX_CLI_INTERFACES; ifIdx++)
   {
      DTI_PRN_USR_DBG_NL(DTI_CLI, DTI_PRN_LEVEL_HIGH,
         ("Send Fct Reg[%02d] - name: %s, avail: %s" DTI_CRLF,
         ifIdx, pCliControl[ifIdx].cliIfName,
         (pCliControl[ifIdx].bRdyForCliSend == IFX_TRUE) ? "Yes" : "No"));
   }

   for (ifIdx = 0; ifIdx < DTI_CLI_MAX_CLI_INTERFACES; ifIdx++)
   {
      if (IFXOS_LOCK_INIT_VALID(&pCliControl[ifIdx].cliResponceBufLock))
         {DTI_LockGet(&pCliControl[ifIdx].cliResponceBufLock);}

      if (pCliControl[ifIdx].bRdyForCliSend == IFX_FALSE)
      {
         if (responceBufferSize > 0)
         {
            pCliControl[ifIdx].pCliResponceBuffer = DTI_Malloc(responceBufferSize);
            if (pCliControl[ifIdx].pCliResponceBuffer == IFX_NULL)
            {
               pCliControl[ifIdx].responceBufferSize = 0;
               if (IFXOS_LOCK_INIT_VALID(&pCliControl->cliResponceBufLock))
                  {DTI_LockRelease(&pCliControl[ifIdx].cliResponceBufLock);}

               DTI_PRN_USR_ERR_NL(DTI_CLI, DTI_PRN_LEVEL_ERR,
                  ("Error: CLI Send Fct Register - CLI name = <%s>, buffer alloc" DTI_CRLF,
                    ifIdx, pIfName));

               return IFX_ERROR;
            }
            pCliControl[ifIdx].responceBufferSize = responceBufferSize;
         }
         else
         {
            /* use internal buffer */
            pCliControl[ifIdx].responceBufferSize = 0;
            pCliControl[ifIdx].pCliResponceBuffer = IFX_NULL;
         }

         DTI_PRN_USR_DBG_NL(DTI_CLI, DTI_PRN_LEVEL_HIGH,
            ("DTI CLI - sendFct register IF: %d name: %s" DTI_CRLF,
              ifIdx, pIfName));

         (void)DTI_snprintf(pCliControl[ifIdx].cliIfName, DTI_CLI_MAX_NAME_LEN,
                  "%s", pIfName);

         pCliControl[ifIdx].pCallbackContext = pCallbackContext;
         if( pSendFct != IFX_NULL)
         {
            pCliControl[ifIdx].pSendFunct       = pSendFct;
         }
         else if( pSendFragFct != IFX_NULL)
         {
            pCliControl[ifIdx].pSendFragFunct       = pSendFragFct;
         }
         pCliControl[ifIdx].bRdyForCliSend   = IFX_TRUE;

         if (IFXOS_LOCK_INIT_VALID(&pCliControl->cliResponceBufLock))
            {DTI_LockRelease(&pCliControl[ifIdx].cliResponceBufLock);}

         break;
      }

      if (IFXOS_LOCK_INIT_VALID(&pCliControl->cliResponceBufLock))
         {DTI_LockRelease(&pCliControl[ifIdx].cliResponceBufLock);}
   }

   if (ifIdx < DTI_CLI_MAX_CLI_INTERFACES)
   {
      DTI_PRN_USR_DBG_NL(DTI_CLI, DTI_PRN_LEVEL_HIGH,
         ("DTI CLI - sendFct register, IF = %d, CLI name = <%s>" DTI_CRLF,
           ifIdx, pIfName));

      return ifIdx;
   }

   DTI_PRN_USR_ERR_NL(DTI_CLI, DTI_PRN_LEVEL_ERR,
      ("Error: CLI Send Fct Register - CLI name = <%s>, no free interface" DTI_CRLF,
        pIfName));

   return IFX_ERROR;

#else
   DTI_PRN_USR_ERR_NL(DTI_CLI, DTI_PRN_LEVEL_ERR,
      ("Error: CLI Send Fct Register - CLI NOT ENABLED" DTI_CRLF));

   return DTI_ERR_CONFIGURATION;
#endif      /* #if defined(INCLUDE_CLI_SUPPORT) && (INCLUDE_CLI_SUPPORT == 1) */
}

/* ============================================================================
   CLI Packet Function definitons
   ========================================================================= */

DTI_STATIC IFX_int_t  DTI_CLI_ConfigGet_Buffer(
                        DTI_AgentCtx_t          *pAgentCtx,
                        DTI_cli_control_t       *pCliControl,
                        DTI_ProtocolServerCtx_t *pDtiProtServerCtx,
                        DTI_CliEventCtx_t       *pCliEventCtx,
                        DTI_PacketError_t       *pPacketError,
                        IFX_uint_t              bufferIdx)
{
   IFX_uint_t bufferSize = 0;
   /* "@DTI# bufferSize <0/1/2> (show tx/rx/evt buffer size)" */
   switch (bufferIdx)
   {
      case 0:
         if (pCliControl->bRdyForCliSend != IFX_TRUE)
         {
            bufferSize = (IFX_uint_t)sizeof(pDtiProtServerCtx->packetIn.buffer);
         }
         else
         {
            bufferSize = 0;
         }
         break;

      case 1:
         if (pCliControl->bRdyForCliSend != IFX_TRUE)
         {
            bufferSize = (pCliControl->responceBufferSize) ?
                          (IFX_uint_t)pCliControl->responceBufferSize : DTI_OUT_BUFFER_SIZE;
         }
         else
         {
            bufferSize = 0;
         }
         break;

      case 2:
         if (pCliEventCtx != IFX_NULL)
         {
            if (pCliEventCtx->bAutoCliMsgActive == IFX_TRUE)
            {
               bufferSize = (IFX_uint_t)pCliEventCtx->maxEventSize_byte;
            }
            else
            {
               bufferSize = 0;
            }
         }
         else
         {
            bufferSize = 0;
            *pPacketError = DTI_eErrConfiguration;
         }
         break;

      default:
         bufferSize = ~0;
         *pPacketError = DTI_eErrPortOutOfRange;
   }

   return (IFX_int_t)bufferSize;
}

/**
   Process a CLI string packet.
   Therefore the port number is interpreted as CLI interface number and if the
   interface is available the CLI command will be forwarded.

\param
   pAgentCtx         - points to the DTI Agent Context.
\param
   pDtiProtServerCtx - points to the worker DTI protocol context.
\param
   pDtiPacketIn      - points to the IN packet.
\param
   pDtiPacketOut     - points to the OUT packet.
\param
   bufferOutSize     - size of the out buffer.

\return
   IFX_SUCCESS if setup was successful, else
   ERROR in case of missing args.
*/
DTI_STATIC IFX_int_t DTI_packet_CliString(
                        DTI_AgentCtx_t          *pAgentCtx,
                        DTI_ProtocolServerCtx_t *pDtiProtServerCtx,
                        const DTI_Packet_t      *pDtiPacketIn,
                        IFX_uint_t              dtiBufInLen,
                        DTI_Packet_t            **ppDtiPacketOut,
                        IFX_uint32_t            bufferOutSize)
{
   IFX_int_t         ifIdx, responceLen = 0;
   IFX_uint_t        availBufferOutSize = 0;
   IFX_int_t         cliRetVal = IFX_SUCCESS, cliOpRetVal = IFX_SUCCESS;
   DTI_PacketError_t packetError = DTI_eNoError;
   DTI_PTR_U         uDtiPacketOut;
   DTI_Packet_t      *pDtiPacketOut;
   IFX_char_t        *pDataIn;
   IFX_char_t        *pDataOut;
   DTI_cli_control_t *pIfControl = IFX_NULL;

   ifIdx   = (IFX_int_t)DTI_HDR_PORT_PORT_NUM_GET(pDtiPacketIn->header.port);
   pDataIn = (IFX_char_t *)pDtiPacketIn->payload;

   if ((ifIdx < 0) || (ifIdx >= DTI_CLI_MAX_CLI_INTERFACES))
   {
      /* CLI IfNum invalid - use given buffer for responce */
      packetError        = DTI_eErrPortOutOfRange;
      pDtiPacketOut      = *ppDtiPacketOut;
      pDataOut           = (IFX_char_t *)pDtiPacketOut->payload;
      availBufferOutSize = (IFX_uint_t)bufferOutSize;

      if (availBufferOutSize > sizeof(DTI_PacketHeader_t))
      {
         responceLen = DTI_snprintf(pDataOut,
            ((IFX_int_t)availBufferOutSize - sizeof(DTI_PacketHeader_t)),
            "Error: CLI Packet String - tan %d, interface[%d] out of range",
            pDtiPacketIn->header.tan, ifIdx);
      }
      else
      {
         DTI_PRN_USR_ERR_NL(DTI_CLI, DTI_PRN_LEVEL_ERR,
            ("Error: CLI Packet String - tan %d, interface[%d] out of range" DTI_CRLF,
            pDtiPacketIn->header.tan, ifIdx));
      }
   }
   else
   {
      pIfControl = &pAgentCtx->cliControl[ifIdx];
      if ((pIfControl->bRdyForCliSend == IFX_TRUE) && (pIfControl->pSendFunct != IFX_NULL || pIfControl->pSendFragFunct != IFX_NULL))
      {
         if ( (pIfControl->responceBufferSize == 0) || (pIfControl->pCliResponceBuffer == IFX_NULL) )
         {
            /* use given responce buffer */
            pDtiPacketOut      = *ppDtiPacketOut;
            pDataOut           = (IFX_char_t *)pDtiPacketOut->payload;
            availBufferOutSize = (IFX_uint_t)bufferOutSize;
         }
         else
         {
            /* user responce buffer set */
            uDtiPacketOut.pUInt8 = pIfControl->pCliResponceBuffer;
            pDtiPacketOut        = (DTI_Packet_t *)DTI_PTR_CAST_GET_ULONG(uDtiPacketOut);
            if (pDtiPacketOut != IFX_NULL )
            {
               availBufferOutSize = pIfControl->responceBufferSize;
               pDataOut           = (IFX_char_t *)pDtiPacketOut->payload;
            }
            else
            {
               /* CLI Response Buffer missalingend - use given buffer for responce */
               packetError        = DTI_eErrConfiguration;
               pDtiPacketOut      = *ppDtiPacketOut;
               pDataOut           = (IFX_char_t *)pDtiPacketOut->payload;
               availBufferOutSize = (IFX_uint_t)bufferOutSize;
               if (availBufferOutSize > sizeof(DTI_PacketHeader_t))
               {
                  responceLen = DTI_snprintf(pDataOut,
                     ((IFX_int_t)availBufferOutSize - sizeof(DTI_PacketHeader_t)),
                     "Error: CLI Packet String - user buffer missaligned, CLI IF %d",
                     ifIdx);
               }
               else
               {
                  DTI_PRN_USR_ERR_NL(DTI_CLI, DTI_PRN_LEVEL_ERR,
                     ("Error: CLI Packet String - user buffer missaligned, CLI IF %d" DTI_CRLF,
                     ifIdx));
               }
            }
         }
         if (packetError == DTI_eNoError)
         {
            *pDataOut = '\0';
            if (pDtiPacketIn->header.payloadSize == 0)
            {
               responceLen = DTI_snprintf(pDataOut,
                  ((IFX_int_t)availBufferOutSize - sizeof(DTI_PacketHeader_t)),
                  "Error: CLI Packet String - no IN payload, packet TAN = %d, CLI IF %d",
                  pDtiPacketIn->header.tan, ifIdx);

               packetError = DTI_eErrMalformedPacket;
            }
            else
            {
               /* terminate input string */
               if ( pDtiPacketIn->header.payloadSize <
                     (IFX_uint32_t)(dtiBufInLen - sizeof(DTI_PacketHeader_t)) )
               {
                  pDataIn[pDtiPacketIn->header.payloadSize] = '\0';
               }
               else
               {
                  pDataIn[pDtiPacketIn->header.payloadSize - 1] = '\0';
               }

               if (pIfControl->bCmdMsgTrace & DTI_CLI_TRACE_CMD_WR_SEND_FLAG)
               {
                  DTI_Printf("%s" DTI_CRLF, pDataIn);
               }

               /* ready to send cli string to the registered CLI interface */
               responceLen = (IFX_int_t)availBufferOutSize - sizeof(DTI_PacketHeader_t);

               if( pIfControl->pSendFunct != IFX_NULL)
               {
                  cliRetVal = pIfControl->pSendFunct(
                                          pIfControl->pCallbackContext,
                                          pDataIn, pDataOut,
                                          &responceLen, &cliOpRetVal);
               }
               else if( pIfControl->pSendFragFunct != IFX_NULL)
               {
                  DTI_PartialResponceContext_t *pFragCallBackContext = IFX_NULL;
                  pFragCallBackContext = DTI_Malloc( sizeof( DTI_PartialResponceContext_t));

                  pFragCallBackContext->pPacketIn = (DTI_Packet_t *)pDtiPacketIn;
                  pFragCallBackContext->pPacketOut = (DTI_Packet_t *)pDtiPacketOut;
                  pFragCallBackContext->iBufferOutLen = responceLen;
                  pFragCallBackContext->pDtiProtServerCtx = pDtiProtServerCtx;

                  cliRetVal = pIfControl->pSendFragFunct(
                                          pIfControl->pCallbackContext,
                                          DTI_handlePartialResponse,
                                          pFragCallBackContext,
                                          pDataIn, pDataOut,
                                          &responceLen, &cliOpRetVal);

                  DTI_Free( pFragCallBackContext );
               }

               if (cliRetVal != IFX_SUCCESS)
               {
                  DTI_PRN_USR_ERR_NL(DTI_CLI, DTI_PRN_LEVEL_ERR,
                     ("Error: CLI Packet String - tan %d, IF[%d], callback failed, respLen %d, opRet %d",
                     pDtiPacketIn->header.tan, ifIdx, responceLen, cliOpRetVal));


                  packetError = DTI_eErrPortOperation;
               }

               if (responceLen > 0)
               {
                  if (responceLen > ((IFX_int_t)availBufferOutSize - (IFX_int_t)sizeof(DTI_PacketHeader_t)) )
                  {
                     DTI_PRN_USR_ERR_NL(DTI_CLI, DTI_PRN_LEVEL_ERR,
                        ("Error: CLI Packet String - tan %d, IF[%d], callback buffer overflow, respLen %d (avail %d)" DTI_CRLF,
                        pDtiPacketIn->header.tan, ifIdx, responceLen, availBufferOutSize));

                     responceLen = (IFX_int_t)availBufferOutSize;
                     pDataOut[responceLen - (sizeof(DTI_PacketHeader_t) + 1)] = '\0';
                  }
                  else
                  {
                     DTI_PRN_USR_DBG_NL(DTI_CLI, DTI_PRN_LEVEL_LOW,
                        ("CLI Packet String - tan %d, IF[%d], callback responce len %d (+ Hdr %d), opRet %d" DTI_CRLF,
                        pDtiPacketIn->header.tan, ifIdx, responceLen, sizeof(DTI_PacketHeader_t), cliOpRetVal));

                     if ( responceLen < (IFX_int_t)(availBufferOutSize - sizeof(DTI_PacketHeader_t)) )
                     {
                        pDataOut[responceLen] = '\0';
                        responceLen++;
                     }
                     else
                     {
                        pDataOut[responceLen - 1] = '\0';
                     }
                  }
               }

               if (pIfControl->bCmdMsgTrace & DTI_CLI_TRACE_CMD_RD_SEND_FLAG)
               {
                  DTI_Printf("%s" DTI_CRLF, pDataOut);
               }

               if (pIfControl->bTestDistEvt == IFX_TRUE)
               {
                  (void)DTI_CLI_InterfaceEventSend(
                        pAgentCtx, pIfControl->cliIfNum, pDataOut, (IFX_uint_t)responceLen);
               }
            }        /* if (pDtiPacketIn->header.payloadSize == 0) */
         }        /* if (packetError == DTI_eNoError) */
      }
      else
      {
         /* CLI not ready - use given buffer for responce */
         packetError        = DTI_eErrPortOpen;
         pDtiPacketOut      = *ppDtiPacketOut;
         pDataOut           = (IFX_char_t *)pDtiPacketOut->payload;
         availBufferOutSize = (IFX_uint_t)bufferOutSize;

         if (availBufferOutSize > sizeof(DTI_PacketHeader_t))
         {
            responceLen = DTI_snprintf(pDataOut,
               ((IFX_int_t)availBufferOutSize - sizeof(DTI_PacketHeader_t)),
               "Error: CLI Packet String - tan %d, interface[%d] invalid (%s)",
               pDtiPacketIn->header.tan, ifIdx, (pIfControl->bRdyForCliSend) ? "Fct missing" : "OFF");
         }
         else
         {
            DTI_PRN_USR_ERR_NL(DTI_CLI, DTI_PRN_LEVEL_ERR,
               ("Error: CLI Packet String - tan %d, interface[%d] invalid (%s)" DTI_CRLF,
               pDtiPacketIn->header.tan, ifIdx, (pIfControl->bRdyForCliSend) ? "Fct missing" : "OFF"));
         }
      }     /* if ((pIfControl->bRdyForCliSend == IFX_TRUE) && (pIfControl->pSendFunct != IFX_NULL)) */
   }
   if ( (packetError != DTI_eNoError) && (responceLen > 0))
   {
      DTI_PRN_USR_ERR_NL(DTI_CLI, DTI_PRN_LEVEL_ERR,
         ("%s" DTI_CRLF, pDataOut));
   }

   (void)DTI_packetResponceSet(
            pDtiPacketIn, pDtiPacketOut,
            packetError, (IFX_uint32_t)responceLen,
            availBufferOutSize, IFX_FALSE);

   *ppDtiPacketOut = pDtiPacketOut;

   return IFX_SUCCESS;
}

IFX_uint32_t DTI_handlePartialResponse(IFX_void_t              *pPartRespContext,
                                       IFX_void_t              *pBuffer,
                                       IFX_uint32_t            len)
{
   DTI_PartialResponceContext_t *pPartRespCont;
   IFX_uint_t        availBufferOutSize;

   pPartRespCont = (DTI_PartialResponceContext_t*) pPartRespContext;
   availBufferOutSize = pPartRespCont->iBufferOutLen + sizeof(DTI_PacketHeader_t);

   (void)DTI_packetResponceSet(
                   pPartRespCont->pPacketIn, pPartRespCont->pPacketOut,
               /*packetError*/ DTI_eNoError, (IFX_uint32_t)len,
               availBufferOutSize, IFX_FALSE);



   pPartRespCont->pPacketOut->header.packetType = DTI_PacketType_eCliPartialString;

   /* Output partial packet header */
   /*
   printf( "Partial packet header:\n" );
   printf( "  magic          0x%08X\n", pPartRespCont->pPacketOut->header.magic );
   printf( "  packetType     0x%08X\n", pPartRespCont->pPacketOut->header.packetType );
   printf( "  packetOptions  0x%08X\n", pPartRespCont->pPacketOut->header.packetOptions );
   printf( "  port           0x%08X\n", pPartRespCont->pPacketOut->header.port );
   printf( "  tan            0x%08X\n", pPartRespCont->pPacketOut->header.tan );
   printf( "  error          0x%08X\n", pPartRespCont->pPacketOut->header.error );
   printf( "  payloadSize    %d\n", pPartRespCont->pPacketOut->header.payloadSize );
   */
   if (DTI_packetSend(&pPartRespCont->pDtiProtServerCtx->dtiCon, pPartRespCont->pPacketOut) != IFX_SUCCESS)
   {
      DTI_PRN_USR_ERR_NL(DTI_CLI, DTI_PRN_LEVEL_ERR,
         ("Error: Device Packet Handler - send packet."DTI_CRLF));

      return 0;
   }

   return len;
}

/**
   Parse the CLI interfaces and return the informations.

\param
   pAgentCtx         - points to the DTI Agent Context.
\param
   pDtiProtServerCtx - points to the worker DTI protocol context.
\param
   pDtiPacketIn      - points to the IN packet.
\param
   pDtiPacketOut     - points to the OUT packet.
\param
   bufferOutSize     - size of the out buffer.

\return
   IFX_SUCCESS if setup was successful, else
   ERROR in case of missing args.
*/
DTI_STATIC IFX_int_t DTI_packet_CliInfoGet(
                        DTI_AgentCtx_t          *pAgentCtx,
                        DTI_ProtocolServerCtx_t *pDtiProtServerCtx,
                        const DTI_Packet_t      *pDtiPacketIn,
                        DTI_Packet_t            *pDtiPacketOut,
                        IFX_uint32_t            bufferOutSize)
{
   IFX_int_t         responceLen = 0;
   DTI_PacketError_t packetError = DTI_eNoError;
   IFX_char_t        *pDataOut;
   pDataOut = (IFX_char_t *)pDtiPacketOut->payload;

   if (bufferOutSize > sizeof(DTI_PacketHeader_t))
   {
      *pDataOut = '\0';

      responceLen =  DTI_CLI_configStrWrite(
                           pAgentCtx, pDataOut,
                           ((IFX_int_t)bufferOutSize - sizeof(DTI_PacketHeader_t)));

      if (responceLen < 0)
      {
         packetError = DTI_eErrPortOperation;
         responceLen = 0;
      }
   }
   else
   {
      DTI_PRN_USR_ERR_NL(DTI_CLI, DTI_PRN_LEVEL_ERR,
         ("Error: CLI Packet Config Get - no/less OUT payload."DTI_CRLF));

      packetError = DTI_eErrMalformedPacket;
   }

   if ( (packetError != DTI_eNoError) && (responceLen > 0))
   {
      DTI_PRN_USR_ERR_NL(DTI_CLI, DTI_PRN_LEVEL_ERR,
         ("ERROR InfoGet:" DTI_CRLF "%s" DTI_CRLF, pDataOut));
   }

   (void)DTI_packetResponceSet(
            pDtiPacketIn, pDtiPacketOut,
            packetError, (IFX_uint32_t)responceLen,
            bufferOutSize, IFX_FALSE);

   return IFX_SUCCESS;
}

/**
   Write a CLI Event to the given worker for further processing.
*/
DTI_STATIC IFX_int_t DTI_CLI_eventWrite(
                        DTI_AgentCtx_t    *pAgentCtx,
                        DTI_WorkerCtx_t   *pWorkerCtx,
                        DTI_cli_control_t *pCliControl,
                        IFX_char_t        *pEventOut,
                        IFX_uint_t        eventOutSize_byte)

{
   IFX_boolean_t     bDropped      = IFX_FALSE;
   IFX_uint_t        fifoBufferSize_byte = 0;
   DTI_PTR_U         uFifoBuffer;
   DTI_Packet_t      *pEventPacket;
   DTI_CliEventCtx_t *pCliEventCtx = &pWorkerCtx->cliEventCtx;
   if (pCliEventCtx->bAutoCliMsgActive == IFX_TRUE)
   {
      fifoBufferSize_byte = sizeof(DTI_PacketHeader_t) + eventOutSize_byte;
      if (pEventOut[eventOutSize_byte - 1] != '\0')
      {
         fifoBufferSize_byte++;
      }

      if (pCliEventCtx->maxEventSize_byte < eventOutSize_byte)
      {
         DTI_PRN_USR_ERR_NL(DTI_CLI, DTI_PRN_LEVEL_ERR,
            ("Error: CLI Event Write - <overflow> drop on CLI IF[%d], event size %d (max %d)" DTI_CRLF,
              pCliControl->cliIfNum, eventOutSize_byte, pCliEventCtx->maxEventSize_byte));

         bDropped = IFX_TRUE;
         goto DTI_CLI_EVENT_WRITE_EXIT;
      }

      if (DTI_LockGet(&pCliEventCtx->eventFifoLock) != IFX_SUCCESS)
      {
         DTI_PRN_USR_ERR_NL(DTI_CLI, DTI_PRN_LEVEL_ERR,
            ("Error: CLI Event Write - <lock get> drop on CLI IF[%d]" DTI_CRLF,
              pCliControl->cliIfNum));

         bDropped = IFX_TRUE;
         goto DTI_CLI_EVENT_WRITE_EXIT;
      }
      uFifoBuffer.pULong = DTI_Var_Fifo_writeElement(
                              &pCliEventCtx->eventFifo,
                              (IFX_uint32_t)fifoBufferSize_byte );

      pEventPacket = (DTI_Packet_t *)DTI_PTR_CAST_GET_ULONG(uFifoBuffer);

      if (pEventPacket != IFX_NULL)
      {
         DTI_MemCpy(pEventPacket->payload, pEventOut, (IFX_int_t)eventOutSize_byte);
         if (pEventOut[eventOutSize_byte - 1] != '\0')
         {
            pEventPacket->payload[eventOutSize_byte] = '\0';
         }

         (void)DTI_headerPacketTypeSet(
                        pEventPacket,
                        (IFX_uint32_t)DTI_PacketType_eCliString,
                        (IFX_uint32_t)DTI_e8Bit,
                        (IFX_uint32_t)(fifoBufferSize_byte - sizeof(DTI_PacketHeader_t)) );

         /* complete header setup */
         pEventPacket->header.port   = (IFX_uint32_t)
               DTI_HDR_PORT_PORT_NUM_SET(pEventPacket->header.port, pCliControl->cliIfNum);
         pEventPacket->header.tan    = (IFX_uint32_t)0;
         pEventPacket->header.error  = (IFX_uint32_t)DTI_eNoError;
      }
      else
      {
         DTI_PRN_USR_ERR_NL(DTI_CLI, DTI_PRN_LEVEL_ERR,
            ("Error: CLI Event Write - <FIFO Wr> drop on CLI IF[%d], event size %d" DTI_CRLF,
              pCliControl->cliIfNum, eventOutSize_byte));

         bDropped = IFX_TRUE;
      }
      DTI_LockRelease(&pCliEventCtx->eventFifoLock);
   }

DTI_CLI_EVENT_WRITE_EXIT:

   if (bDropped == IFX_TRUE)
   {
      pEventOut[eventOutSize_byte - 1] = '\0';

      DTI_PRN_USR_ERR_NL(DTI_CLI, DTI_PRN_LEVEL_WRN,
         ("WARNING: Dropped CLI Event" DTI_CRLF "<<%s>>" DTI_CRLF, pEventOut));

      return IFX_ERROR;
   }

   return IFX_SUCCESS;
}


#endif /* #if defined(INCLUDE_CLI_SUPPORT) && (INCLUDE_CLI_SUPPORT == 1) */

/* ============================================================================
   Global Function definitons
   ========================================================================= */

/**
   Create a config string of the available CLI interfaces.

*/
IFX_int_t DTI_CLI_configStrWrite(
                        DTI_AgentCtx_t *pAgentCtx,
                        IFX_char_t     *pSysInfoBuffer,
                        IFX_int_t      bufferSize)
{
#if defined(INCLUDE_CLI_SUPPORT) && (INCLUDE_CLI_SUPPORT == 1)
   IFX_int_t  ifIdx, written = 0;
   if (pAgentCtx == IFX_NULL)
   {
      DTI_PRN_USR_ERR_NL(DTI_CLI, DTI_PRN_LEVEL_ERR,
         ("Error: CLI Config Str Create - missing agent context" DTI_CRLF));

      return 0;
   }

   if (pSysInfoBuffer == IFX_NULL)
   {
      DTI_PRN_USR_ERR_NL(DTI_CLI, DTI_PRN_LEVEL_ERR,
         ("Error: CLI Config Str Create - missing args" DTI_CRLF));

      return 0;
   }

   (void)DTI_snprintf(&pSysInfoBuffer[written], bufferSize - written,
      "NUM_CLI_IF=%d", DTI_CLI_MAX_CLI_INTERFACES);
   written += (IFX_int_t)DTI_StrLen(&pSysInfoBuffer[written]) + 1;

   if (written >= bufferSize)
   {
      DTI_PRN_USR_ERR_NL(DTI_CLI, DTI_PRN_LEVEL_ERR,
         ("Error: CLI Config Str Create - buffer overflow (I)" DTI_CRLF));

      return bufferSize;
   }

   for (ifIdx = 0; ifIdx < DTI_CLI_MAX_CLI_INTERFACES; ifIdx++)
   {
      if (written < bufferSize)
      {
         (void)DTI_snprintf(&pSysInfoBuffer[written], bufferSize - written,
            "CLI_IF_%d_NAME=%s \tretBufferSize=%d",
            ifIdx, pAgentCtx->cliControl[ifIdx].cliIfName,
            (pAgentCtx->cliControl[ifIdx].pCliResponceBuffer) ?
               pAgentCtx->cliControl[ifIdx].responceBufferSize : DTI_OUT_BUFFER_SIZE );

         written += DTI_StrLen(&pSysInfoBuffer[written]) + 1;
      }
      else
      {
         break;
      }
   }

   if (written >= bufferSize)
   {
      DTI_PRN_USR_ERR_NL(DTI_CLI, DTI_PRN_LEVEL_ERR,
         ("Error: CLI Config Str Create - buffer overflow (II)" DTI_CRLF));

      return bufferSize;
   }

#else
   IFX_int_t  written = 0;

   (void)DTI_snprintf(&pSysInfoBuffer[written], bufferSize - written,
                      "WARNING - no DSL devices supported");
   written = (IFX_uint_t)DTI_StrLen(&pSysInfoBuffer[written]) + 1;

#endif   /* #if defined(INCLUDE_CLI_SUPPORT) && (INCLUDE_CLI_SUPPORT == 1) */

   return written;
}

/**
   DTI Command Line Interface (CLI) Packet Handler.

\param
   pAgentCtx,        - points to the DTI Agent context.
\param
   pDtiProtServerCtx - points to the current DTI protocol context.
\param
   pDtiPacketIn      - points to the IN packet.
\param
   pDtiPacketOut     - points to the OUT packet.
\param
   dtiBufOutLen      - size of the out buffer.

\return
   IFX_SUCCESS if setup was successful, else
   ERROR in case of missing args.
*/
IFX_int_t DTI_packetHandler_cli(
                        DTI_AgentCtx_t          *pAgentCtx,
                        DTI_ProtocolServerCtx_t *pDtiProtServerCtx,
                        IFX_int_t               workerNum,
                        const DTI_Packet_t      *pDtiPacketIn,
                        IFX_uint_t              dtiBufInLen,
                        DTI_Packet_t            *pDtiPacketOut,
                        IFX_uint_t              dtiBufOutLen)
{
   IFX_int_t            retVal = IFX_SUCCESS;
   IFX_uint_t           bufOutLen;
   const DTI_Packet_t   *pPacketIn;
   DTI_Packet_t         *pPacketOut;
   DTI_PTR_U            uPayload;
   DTI_cli_control_t    *pIfControl = IFX_NULL;
   if ((pDtiProtServerCtx == IFX_NULL) || (pAgentCtx == IFX_NULL))
   {
      DTI_PRN_USR_ERR_NL(DTI_CLI, DTI_PRN_LEVEL_ERR,
         ("Error: CLI Packet Handler - NULL ptr args."DTI_CRLF));

      return IFX_ERROR;
   }

   if (pDtiPacketIn)
   {
      pPacketIn  = pDtiPacketIn;
   }
   else
   {
      uPayload.pUInt8 = (IFX_uint8_t *)pDtiProtServerCtx->packetIn.buffer;
      pPacketIn       = (DTI_Packet_t *)DTI_PTR_CAST_GET_ULONG(uPayload);
      dtiBufInLen     = sizeof(pDtiProtServerCtx->packetIn.buffer);
   }

   if (pDtiPacketOut)
   {
      pPacketOut = pDtiPacketOut;
      bufOutLen  = dtiBufOutLen;
   }
   else
   {
      uPayload.pUInt8 = (IFX_uint8_t *)pDtiProtServerCtx->packetOut.buffer;
      pPacketOut      = (DTI_Packet_t *)DTI_PTR_CAST_GET_ULONG(uPayload);
      bufOutLen       = sizeof(pDtiProtServerCtx->packetOut.buffer);
   }

   if ((pPacketIn == IFX_NULL) || (pPacketOut == IFX_NULL))
   {
      DTI_PRN_USR_ERR_NL(DTI_CLI, DTI_PRN_LEVEL_ERR,
         ("Error: Device Packet Handler - %s packet missaligend."DTI_CRLF,
         (pPacketIn == IFX_NULL) ? "IN" : "OUT" ));

      return IFX_ERROR;
   }

   if (pPacketIn->header.magic != DTI_MAGIC)
   {
      DTI_PRN_USR_ERR_NL(DTI_CLI, DTI_PRN_LEVEL_ERR,
         ("Error: CLI Packet Handler - invalid IN packet."DTI_CRLF));

      return IFX_SUCCESS;
   }

   switch(pPacketIn->header.packetType)
   {
#if defined(INCLUDE_CLI_SUPPORT) && (INCLUDE_CLI_SUPPORT == 1)
      case DTI_PacketType_eCliString:
         {
            IFX_int_t ifIdx = (IFX_int_t)DTI_HDR_PORT_PORT_NUM_GET(pPacketIn->header.port);
            if ((ifIdx >= 0) && (ifIdx < DTI_CLI_MAX_CLI_INTERFACES))
            {
               pIfControl = &pAgentCtx->cliControl[ifIdx];
               if (IFXOS_LOCK_INIT_VALID(&pIfControl->cliResponceBufLock))
                  {DTI_LockGet(&pIfControl->cliResponceBufLock);}
            }
            retVal = DTI_packet_CliString(
                        pAgentCtx, pDtiProtServerCtx,
                        pPacketIn, dtiBufInLen, &pPacketOut, bufOutLen);
         }
         break;

      case DTI_PacketType_eCliInfoGet:
         retVal = DTI_packet_CliInfoGet(
                        pAgentCtx, pDtiProtServerCtx,
                        pPacketIn, pPacketOut, bufOutLen);
         break;
#endif

      default:
         {
            retVal = DTI_packetUnknownSet(pPacketIn, pPacketOut, bufOutLen);
         }
         break;
   }

   if (retVal == IFX_SUCCESS)
   {
      DTI_packetShow (
         pPacketOut, IFX_TRUE, IFX_FALSE, "CLI Send",
         (pPacketOut->header.error == DTI_eNoError) ? IFXOS_PRN_LEVEL_LOW : DTI_PRN_LEVEL_HIGH);

      if (DTI_packetSend(&pDtiProtServerCtx->dtiCon, pPacketOut) != IFX_SUCCESS)
      {
         DTI_PRN_USR_ERR_NL(DTI_CLI, DTI_PRN_LEVEL_ERR,
            ("Error: Device Packet Handler - send packet."DTI_CRLF));

         retVal = IFX_ERROR;
      }
   }

   if (pIfControl != IFX_NULL)
   {
      if (IFXOS_LOCK_INIT_VALID(&pIfControl->cliResponceBufLock))
         {DTI_LockRelease(&pIfControl->cliResponceBufLock);}
   }

   return retVal;
}


/**
   Initialize the CLI Event control struct.
   - init lock
*/
IFX_int_t DTI_CLI_workerEventCtxInit(
                        DTI_AgentCtx_t       *pAgentCtx,
                        IFX_int_t            workerNum)
{
#if    defined(INCLUDE_CLI_SUPPORT) && (INCLUDE_CLI_SUPPORT == 1) \
    && defined(DTI_CLI_EVENT_SUPPORT) && (DTI_CLI_EVENT_SUPPORT == 1)

   if ( (pAgentCtx == IFX_NULL) ||
        (workerNum >= DTI_NUM_OF_USED_WORKER_GET(pAgentCtx)) )
   {
      DTI_PRN_USR_ERR_NL(DTI_CLI, DTI_PRN_LEVEL_ERR,
         ("Error: CLI Worker Event Ctx Create - arg <%s>."DTI_CRLF,
           (workerNum >= DTI_NUM_OF_USED_WORKER_GET(pAgentCtx)) ? "workerNum" : "null ptr" ));

      return IFX_ERROR;
   }

   if (pAgentCtx->pWorker[workerNum] == IFX_NULL )
   {
      DTI_PRN_USR_ERR_NL(DTI_CLI, DTI_PRN_LEVEL_ERR,
         ("Error: CLI Worker Event Ctx Create - DTI Worker[%d] not running."DTI_CRLF,
           workerNum));

      return IFX_ERROR;
   }

   DTI_MemSet(&pAgentCtx->pWorker[workerNum]->cliEventCtx, 0x00, sizeof(DTI_CliEventCtx_t));

   DTI_LockInit(&pAgentCtx->pWorker[workerNum]->cliEventCtx.eventFifoLock);
#endif

   return IFX_SUCCESS;
}

/**
   Reset the CLI Event control struct.
   - Clear this worker from all CLI Interfaces.
   - Disable event handling.
   - Delete lock.
*/
IFX_int_t DTI_CLI_workerEventCtxDelete(
                        DTI_AgentCtx_t       *pAgentCtx,
                        IFX_int_t            workerNum)
{
#if    defined(INCLUDE_CLI_SUPPORT) && (INCLUDE_CLI_SUPPORT == 1) \
    && defined(DTI_CLI_EVENT_SUPPORT) && (DTI_CLI_EVENT_SUPPORT == 1)

   if ( (pAgentCtx == IFX_NULL) ||
        (workerNum >= DTI_NUM_OF_USED_WORKER_GET(pAgentCtx)) )
   {
      DTI_PRN_USR_ERR_NL(DTI_CLI, DTI_PRN_LEVEL_ERR,
         ("Error: CLI Worker Event Ctx Delete - arg <%s>."DTI_CRLF,
           (workerNum >= DTI_NUM_OF_USED_WORKER_GET(pAgentCtx)) ? "workerNum" : "null ptr" ));

      return IFX_ERROR;
   }

   if (pAgentCtx->pWorker[workerNum] == IFX_NULL )
   {
      DTI_PRN_USR_ERR_NL(DTI_CLI, DTI_PRN_LEVEL_ERR,
         ("Error: CLI Worker Event Ctx Delete - DTI Worker[%d] not running."DTI_CRLF,
           workerNum));

      return IFX_ERROR;
   }

   if (IFXOS_LOCK_INIT_VALID(&pAgentCtx->pWorker[workerNum]->cliEventCtx.eventFifoLock) == IFX_TRUE)
   {
      (void)DTI_CLI_workerEventRelease(
               pAgentCtx, workerNum);

      DTI_LockDelete(&pAgentCtx->pWorker[workerNum]->cliEventCtx.eventFifoLock);
   }
   else
   {
      DTI_PRN_USR_ERR_NL(DTI_CLI, DTI_PRN_LEVEL_ERR,
         ("Error: CLI Worker Event Ctx Delete - not initialized."DTI_CRLF ));

      return IFX_ERROR;
   }

#endif

   return IFX_SUCCESS;
}

/**
   Enable / Disable the CLI Event Control.

\param
   pCliEventCtx   - points to the corresponding CLI Event context.
\param
   eventBufferSize_byte - buffer size for event handling.
\param
   maxEventSize_byte    - max event size.

\return

*/
IFX_int_t DTI_CLI_workerEventCreate(
                        DTI_AgentCtx_t       *pAgentCtx,
                        IFX_int_t            workerNum,
                        IFX_uint_t           eventBufferSize_byte,
                        IFX_uint_t           maxEventSize_byte)
{
#if    defined(INCLUDE_CLI_SUPPORT) && (INCLUDE_CLI_SUPPORT == 1) \
    && defined(DTI_CLI_EVENT_SUPPORT) && (DTI_CLI_EVENT_SUPPORT == 1)

   DTI_CliEventCtx_t *pCliEventCtx;

   if ( (pAgentCtx == IFX_NULL) ||
        (workerNum >= DTI_NUM_OF_USED_WORKER_GET(pAgentCtx)) )
   {
      DTI_PRN_USR_ERR_NL(DTI_CLI, DTI_PRN_LEVEL_ERR,
         ("Error: CLI Worker Event Create - arg <%s>."DTI_CRLF,
           (workerNum >= DTI_NUM_OF_USED_WORKER_GET(pAgentCtx)) ? "workerNum" : "null ptr" ));

      return IFX_ERROR;
   }

   if (pAgentCtx->pWorker[workerNum] == IFX_NULL )
   {
      DTI_PRN_USR_ERR_NL(DTI_CLI, DTI_PRN_LEVEL_ERR,
         ("Error: CLI Worker Event Create - DTI Worker[%d] not running."DTI_CRLF,
           workerNum));

      return IFX_ERROR;
   }
   pCliEventCtx = &pAgentCtx->pWorker[workerNum]->cliEventCtx;

   if (DTI_LockGet(&pCliEventCtx->eventFifoLock) != IFX_SUCCESS)
   {
      DTI_PRN_USR_ERR_NL(DTI_CLI, DTI_PRN_LEVEL_ERR,
         ("Error: CLI Worker Event Create - lock get."DTI_CRLF ));

      return IFX_ERROR;
   }

   if (pCliEventCtx->pEventFifoBuffer != IFX_NULL)
   {
      DTI_PRN_USR_ERR_NL(DTI_CLI, DTI_PRN_LEVEL_ERR,
         ("Error: CLI Worker Event Create - already exist (buffer size = %d, max event size = %d)."DTI_CRLF,
           pCliEventCtx->eventFifoBufferSize_byte, pCliEventCtx->maxEventSize_byte));

      DTI_LockRelease(&pCliEventCtx->eventFifoLock);
      return IFX_ERROR;
   }


   if ( (DTI_CLI_workerEventControl_create(
            pCliEventCtx, eventBufferSize_byte, maxEventSize_byte)) != IFX_SUCCESS)
   {
      DTI_LockRelease(&pCliEventCtx->eventFifoLock);
      return IFX_ERROR;
   }

   pCliEventCtx->bAutoCliMsgActive = IFX_TRUE;

   DTI_LockRelease(&pCliEventCtx->eventFifoLock);

#endif   /* #if defined(INCLUDE_CLI_SUPPORT) && (INCLUDE_CLI_SUPPORT == 1) */

   return IFX_SUCCESS;
}

/**
   Close the CLI Event Control of this worker.
   - Clear this worker from all CLI Interfaces.
   - Disable and release event fifo buffer.
   -

*/
IFX_int_t DTI_CLI_workerEventRelease(
                        DTI_AgentCtx_t       *pAgentCtx,
                        IFX_int_t            workerNum)
{
#if    defined(INCLUDE_CLI_SUPPORT) && (INCLUDE_CLI_SUPPORT == 1) \
    && defined(DTI_CLI_EVENT_SUPPORT) && (DTI_CLI_EVENT_SUPPORT == 1)

   if ( (pAgentCtx == IFX_NULL) ||
        (workerNum >= DTI_NUM_OF_USED_WORKER_GET(pAgentCtx)) )
   {
      DTI_PRN_USR_ERR_NL(DTI_CLI, DTI_PRN_LEVEL_ERR,
         ("Error: CLI Worker Event Release - arg <%s>."DTI_CRLF,
           (workerNum >= DTI_NUM_OF_USED_WORKER_GET(pAgentCtx)) ? "workerNum" : "null ptr" ));

      return IFX_ERROR;
   }

   if (pAgentCtx->pWorker[workerNum] == IFX_NULL )
   {
      DTI_PRN_USR_ERR_NL(DTI_CLI, DTI_PRN_LEVEL_ERR,
         ("Error: CLI Worker Event Release - DTI Worker[%d] not running."DTI_CRLF,
           workerNum));

      return IFX_ERROR;
   }

   if (DTI_LockGet(&pAgentCtx->pWorker[workerNum]->cliEventCtx.eventFifoLock) != IFX_SUCCESS)
   {
      DTI_PRN_USR_ERR_NL(DTI_CLI, DTI_PRN_LEVEL_ERR,
         ("Error: CLI Worker Event Release - lock get."DTI_CRLF ));

      return IFX_ERROR;
   }

   DTI_PRN_USR_DBG_NL(DTI_CLI, DTI_PRN_LEVEL_HIGH,
      ("CLI Worker Event Release - worker[%d]."DTI_CRLF,
        workerNum));

   (void)DTI_CLI_workerEventControl_delete(
                     pAgentCtx, workerNum);

   DTI_LockRelease(&pAgentCtx->pWorker[workerNum]->cliEventCtx.eventFifoLock);

#endif
   return IFX_SUCCESS;
}

/**
   Enable / Disable the CLI Event Control.

\param
   pCliControl    - points to the corresponding CLI control struct.
\param
   pCliEventCtx   - points to the corresponding CLI Event context.
\param
   workerNum      - worker number
\param
   bEnable        - enable / disable the CLI Events for this interface.

\return

*/
IFX_int_t DTI_CLI_workerEventControl(
                        DTI_AgentCtx_t       *pAgentCtx,
                        IFX_int_t            workerNum,
                        IFX_uint_t           cliIfNum,
                        IFX_boolean_t        bEnable)
{
#if    defined(INCLUDE_CLI_SUPPORT) && (INCLUDE_CLI_SUPPORT == 1) \
    && defined(DTI_CLI_EVENT_SUPPORT) && (DTI_CLI_EVENT_SUPPORT == 1)

   DTI_cli_control_t *pCliControl;
   DTI_CliEventCtx_t *pCliEventCtx;

   if ( (pAgentCtx == IFX_NULL) ||
        (workerNum >= DTI_NUM_OF_USED_WORKER_GET(pAgentCtx)) ||
        (cliIfNum >= DTI_CLI_MAX_CLI_INTERFACES) )
   {
      DTI_PRN_USR_ERR_NL(DTI_CLI, DTI_PRN_LEVEL_ERR,
         ("Error: CLI Worker Event Control - arg <%s>."DTI_CRLF,
           (pAgentCtx == IFX_NULL) ? "null ptr" :
               ((workerNum >= DTI_NUM_OF_USED_WORKER_GET(pAgentCtx)) ? "workerNum" : "cliIfNum") ));

      return IFX_ERROR;
   }

   if (pAgentCtx->pWorker[workerNum] == IFX_NULL )
   {
      DTI_PRN_USR_ERR_NL(DTI_CLI, DTI_PRN_LEVEL_ERR,
         ("Error: CLI Worker Event Create - DTI Worker[%d] not running."DTI_CRLF,
           workerNum));

      return IFX_ERROR;
   }
   pCliEventCtx = &pAgentCtx->pWorker[workerNum]->cliEventCtx;
   pCliControl  = &pAgentCtx->cliControl[cliIfNum];

   if (DTI_LockGet(&pCliEventCtx->eventFifoLock) != IFX_SUCCESS)
   {
      DTI_PRN_USR_ERR_NL(DTI_CLI, DTI_PRN_LEVEL_ERR,
         ("Error: CLI Worker Event Control - lock get."DTI_CRLF ));

      return IFX_ERROR;
   }

   if (pCliEventCtx->bAutoCliMsgActive != IFX_TRUE)
   {
      DTI_PRN_USR_ERR_NL(DTI_CLI, DTI_PRN_LEVEL_ERR,
         ("Error: CLI Worker Event Control - worker event not activ."DTI_CRLF ));

      DTI_LockRelease(&pCliEventCtx->eventFifoLock);
      return IFX_ERROR;
   }

   if (bEnable == IFX_TRUE)
   {
      pCliControl->bAcceptEvents[workerNum] = 1;
   }
   else
   {
      pCliControl->bAcceptEvents[workerNum] = 0;
   }

   DTI_LockRelease(&pCliEventCtx->eventFifoLock);

   DTI_PRN_USR_DBG_NL(DTI_CLI, DTI_PRN_LEVEL_HIGH,
      ("CLI Worker Event Control: CLI IF[%d], DTI Worker[%d] - %s."DTI_CRLF,
        pCliControl->cliIfNum, workerNum,
        (bEnable == IFX_TRUE) ? "ENABLE" : "DISABLE"));

#endif   /* #if defined(INCLUDE_CLI_SUPPORT) && (INCLUDE_CLI_SUPPORT == 1) */

   return IFX_SUCCESS;
}

/**
   Process the CLI Event FIFO of the given worker thread
*/
IFX_int_t DTI_CLI_autoMsgProcess(
                        DTI_AgentCtx_t          *pAgentCtx,
                        IFX_int_t               workerNum,
                        DTI_ProtocolServerCtx_t *pDtiProtServerCtx)
{
   IFX_int_t         retVal = IFX_SUCCESS;
   IFX_uint32_t      fifoElementSize_byte = 0;
   DTI_CliEventCtx_t *pCliEventCtx;
   DTI_Packet_t      *pEventPacket;
   DTI_PTR_U         uFifoBuffer;
   if ( (pAgentCtx == IFX_NULL) ||
        (workerNum >= DTI_NUM_OF_USED_WORKER_GET(pAgentCtx)) )
   {
      DTI_PRN_USR_ERR_NL(DTI_CLI, DTI_PRN_LEVEL_ERR,
         ("Error: CLI Auto Msg - arg <%s>."DTI_CRLF,
           (workerNum >= DTI_NUM_OF_USED_WORKER_GET(pAgentCtx)) ? "workerNum" : "null ptr" ));

      return IFX_ERROR;
   }

   if (pAgentCtx->pWorker[workerNum] == IFX_NULL )
   {
      DTI_PRN_USR_ERR_NL(DTI_CLI, DTI_PRN_LEVEL_ERR,
         ("Error: CLI Auto Msg - DTI Worker[%d] not running."DTI_CRLF,
           workerNum));

      return IFX_ERROR;
   }

   pCliEventCtx = &pAgentCtx->pWorker[workerNum]->cliEventCtx;

   if (pCliEventCtx->bAutoCliMsgActive == IFX_TRUE)
   {
      if (DTI_Var_Fifo_isEmpty(&pCliEventCtx->eventFifo) == IFX_TRUE)
      {
         /* nothing to do */
         return IFX_SUCCESS;
      }

/**
   PEEK: take the buffer and process the available message
         --> the FIFO is not blocked while processing.
*/
#if (IFXOS_SUPPORTS_FIFO_PEEK == 1)

      if (DTI_LockGet(&pCliEventCtx->eventFifoLock) != IFX_SUCCESS)
      {
         DTI_PRN_USR_ERR_NL(DTI_CLI, DTI_PRN_LEVEL_ERR,
            ("Error: CLI Auto Msg (P) - lock get."DTI_CRLF ));

         return IFX_ERROR;
      }
      /* cli msg available - peek the msg */
      uFifoBuffer.pULong = DTI_Var_Fifo_peekElement(
                              &pCliEventCtx->eventFifo, &fifoElementSize_byte);

      DTI_LockRelease(&pCliEventCtx->eventFifoLock);

      pEventPacket = (DTI_Packet_t *)DTI_PTR_CAST_GET_ULONG(uFifoBuffer);

      if (pEventPacket == IFX_NULL)
      {
         DTI_PRN_USR_ERR_NL(DTI_CLI, DTI_PRN_LEVEL_ERR,
            ("ERROR: CLI Auto Msg (P) - peek packet, %s."DTI_CRLF,
            (uFifoBuffer.pULong == IFX_NULL) ? "peek failed" : "msg missaligend" ));

         return IFX_ERROR;
      }

      if (pEventPacket->header.payloadSize != fifoElementSize_byte - sizeof(DTI_PacketHeader_t))
      {
         /* error */
         DTI_PRN_USR_ERR_NL(DTI_CLI, DTI_PRN_LEVEL_ERR,
            ("ERROR: CLI Auto Msg (P) - invalid payload size."DTI_CRLF));
         pEventPacket->header.error  = (IFX_uint32_t)DTI_eErrInvalidPayloadSize;
         DTI_packetShow(pEventPacket, IFX_TRUE, IFX_FALSE, "CLI AutoMsg (P)", DTI_PRN_LEVEL_HIGH);
      }
      else
      {
         DTI_packetShow(pEventPacket, IFX_TRUE, IFX_FALSE, "CLI AutoMsg (P)", IFXOS_PRN_LEVEL_LOW);
      }

      if (DTI_packetSend(&pDtiProtServerCtx->dtiCon, pEventPacket) != IFX_SUCCESS)
      {
         DTI_CONN_PACKET_OUT_CLI_EVT_DISC_INC(pAgentCtx, workerNum);
         DTI_CONN_PACKET_OUT_ERR_INC(pAgentCtx, workerNum);
         DTI_PRN_USR_ERR_NL(DTI_CLI, DTI_PRN_LEVEL_ERR,
            ("ERROR: CLI Auto Msg (P) - send packet."DTI_CRLF));

         retVal = IFX_ERROR;
      }
      else
      {
         DTI_CONN_PACKET_OUT_CLI_EVT_INC(pAgentCtx, workerNum);
      }

      if (DTI_LockGet(&pCliEventCtx->eventFifoLock) != IFX_SUCCESS)
      {
         DTI_PRN_USR_ERR_NL(DTI_CLI, DTI_PRN_LEVEL_ERR,
            ("Error: CLI Auto Msg (P) - lock get."DTI_CRLF ));

         return IFX_ERROR;
      }

      /* simple read to free FIFO */
      (void)DTI_Var_Fifo_readElement(&pCliEventCtx->eventFifo, &fifoElementSize_byte);

      DTI_LockRelease(&pCliEventCtx->eventFifoLock);

#else   /* #if (IFXOS_SUPPORTS_FIFO_PEEK == 1) */

      if (DTI_LockGet(&pCliEventCtx->eventFifoLock) != IFX_SUCCESS)
      {
         DTI_PRN_USR_ERR_NL(DTI_CLI, DTI_PRN_LEVEL_ERR,
            ("Error: CLI Auto Msg (R) - lock get."DTI_CRLF ));

         return IFX_ERROR;
      }

      /* cli msg available - peek the msg */
      uFifoBuffer.pULong = DTI_Var_Fifo_readElement(
                              &pCliEventCtx->eventFifo, &fifoElementSize_byte);

      pEventPacket = (DTI_Packet_t *)DTI_PTR_CAST_GET_ULONG(uFifoBuffer);

      if (pEventPacket->header.payloadSize != fifoElementSize_byte - sizeof(DTI_PacketHeader_t))
      {
         /* error */
         pEventPacket->header.error  = (IFX_uint32_t)DTI_eErrInvalidPayloadSize;
      }

      DTI_packetShow (
         pEventPacket, IFX_TRUE, IFX_FALSE, "CLI AutoMsg (R)",
         (pEventPacket->header.error == DTI_eNoError) ? IFXOS_PRN_LEVEL_LOW : DTI_PRN_LEVEL_HIGH);


      if (DTI_packetSend(&pDtiProtServerCtx->dtiCon, pEventPacket) != IFX_SUCCESS)
      {
         DTI_CONN_PACKET_OUT_CLI_EVT_DISC_INC(pAgentCtx, workerNum);
         DTI_CONN_PACKET_OUT_ERR_INC(pAgentCtx, workerNum);
         DTI_PRN_USR_ERR_NL(DTI_CLI, DTI_PRN_LEVEL_ERR,
            ("ERROR: CLI Auto Msg (R) - send packet."DTI_CRLF));

         retVal = IFX_ERROR;
      }
      else
      {
         DTI_CONN_PACKET_OUT_CLI_EVT_INC(pAgentCtx, workerNum);
      }


      DTI_LockRelease(&pCliEventCtx->eventFifoLock);

#endif   /* #if (IFXOS_SUPPORTS_FIFO_PEEK == 1) */
   }

   return retVal;
}


/**
   Initialize the CLI interface control struct.
   - reset the data
   - set the local loop cli function.
*/
IFX_int_t DTI_CLI_moduleControlInit(
                        DTI_AgentCtx_t    *pAgentCtx)
{
#if defined(INCLUDE_CLI_SUPPORT) && (INCLUDE_CLI_SUPPORT == 1)
   IFX_int_t  ifIdx;
   DTI_cli_control_t *pCliControl = &pAgentCtx->cliControl[0];

   DTI_MemSet(pCliControl, 0x00, sizeof(DTI_cli_control_t) * DTI_CLI_MAX_CLI_INTERFACES);

   for (ifIdx = 0; ifIdx < DTI_CLI_MAX_CLI_INTERFACES - 1; ifIdx++)
   {
      DTI_LockInit(&pCliControl[ifIdx].cliResponceBufLock);

      pCliControl[ifIdx].cliIfNum = (IFX_uint8_t)ifIdx;
      (void)DTI_snprintf(pCliControl[ifIdx].cliIfName, DTI_CLI_MAX_NAME_LEN,
                         "not_used");
   }

   /* set the last interface for default loopback */
   pCliControl[DTI_CLI_MAX_CLI_INTERFACES - 1].cliIfNum         = DTI_CLI_MAX_CLI_INTERFACES - 1;
   pCliControl[DTI_CLI_MAX_CLI_INTERFACES - 1].pSendFunct       = DTI_CLI_DefaultLoop_Send;
   pCliControl[DTI_CLI_MAX_CLI_INTERFACES - 1].pCallbackContext = (IFX_void_t *)pAgentCtx;

   (void)DTI_snprintf(
            pCliControl[DTI_CLI_MAX_CLI_INTERFACES - 1].cliIfName,
            DTI_CLI_MAX_NAME_LEN,
            "local_loop");

   pCliControl[DTI_CLI_MAX_CLI_INTERFACES - 1].bRdyForCliSend = IFX_TRUE;

   for (ifIdx = 0; ifIdx < DTI_CLI_MAX_CLI_INTERFACES; ifIdx++)
   {
      DTI_PRN_USR_DBG_NL(DTI_CLI, DTI_PRN_LEVEL_NORMAL,
         ("DTI CLI: DefaultInit Reg[%02d] - name: %s, avail: %s" DTI_CRLF,
         ifIdx, pCliControl[ifIdx].cliIfName,
         (pCliControl[ifIdx].bRdyForCliSend == IFX_TRUE) ? "Yes" : "No"));
   }
#endif   /* #if defined(INCLUDE_CLI_SUPPORT) && (INCLUDE_CLI_SUPPORT == 1) */

   return IFX_SUCCESS;
}

/**
   Stop and free the CLI interfaces.

\return
   If success IFX_SUCCESS, the context is freed and the ptr is set to IFX_NULL.
   else IFX_ERROR.

*/
IFX_int_t DTI_CLI_moduleStop(
                        DTI_AgentCtx_t    *pAgentCtx)
{
#if defined(INCLUDE_CLI_SUPPORT) && (INCLUDE_CLI_SUPPORT == 1)
   IFX_int_t  ifIdx;

   for (ifIdx = 0; ifIdx < DTI_CLI_MAX_CLI_INTERFACES - 1; ifIdx++)
   {
      (void)DTI_CLI_singleInterface_Stop(&pAgentCtx->cliControl[ifIdx]);
      if (IFXOS_LOCK_INIT_VALID(&pAgentCtx->cliControl[ifIdx].cliResponceBufLock))
      {
         DTI_LockDelete(&pAgentCtx->cliControl[ifIdx].cliResponceBufLock);
      }
   }

#endif

   return IFX_SUCCESS;
}


/* ============================================================================
   CLI Interface Functions
   ========================================================================= */

/**
   This function registers a corresponding send function within the DTI.
   The function is used to send incoming command strings to the caller
   command line interface (CLI).
   The send function must have the format of \ref DTI_CliSendCommandFunc_t

\param
   pDtiAgentContext  - points to the DTI Agent context.
\param
   pCallbackContext  - points to the callback context, this context contains
                       user data used by the callback function. The callback
                       function is called with this pointer.
\param
   pIfName           - CLI name, via this name the CLI will be identified from
                       the remote client.
\param
   pSendFct          - function pointer to the send function to send a command
                       on this CLI.
                       Provide a NULL pointer to reserve the CLI interface and
                       to get a interface number (for example, only to register
                       an event call back).
\param
   responceBufferSize - the buffer size required by the callback function.

\return
   In case of success, the CLI interface is marked as reserved and the
   interface number is returned (>= 0)
   In case of error IFX_ERROR will be return.

\remarks
   The last available interface is the default loop back interface.

*/

IFX_int_t DTI_CLI_SendFunctionRegister(
                                 IFX_void_t                 *pDtiAgentContext,
                                 IFX_void_t                 *pCallbackContext,
                                 const IFX_char_t           *pIfName,
                                 DTI_CliSendCommandFunc_t   pSendFct,
                                 IFX_uint_t                 responceBufferSize)
{
   return( DTI_CLI_SendFunctionRegisterCommon( pDtiAgentContext, pCallbackContext, 
            pIfName, pSendFct, IFX_NULL, responceBufferSize));
}

/**
   This function registers a corresponding send function within the DTI.
   The function is used to send incoming command strings to the caller
   command line interface (CLI) with fragmented response support.
   The send function must have the format of \ref DTI_CliSendFragmentedCommandFunc_t

\param
   pDtiAgentContext  - points to the DTI Agent context.
\param
   pCallbackContext  - points to the callback context, this context contains
                       user data used by the callback function. The callback
                       function is called with this pointer.
\param
   pIfName           - CLI name, via this name the CLI will be identified from
                       the remote client.
\param
   pSendFragFct      - function pointer to the send function to send a command                     on this CLI.
                       Provide a NULL pointer to reserve the CLI interface and
                       to get a interface number (for example, only to register
                       an event call back).
\param
   responceBufferSize - the buffer size required by the callback function.

\return
   In case of success, the CLI interface is marked as reserved and the
   interface number is returned (>= 0)
   In case of error IFX_ERROR will be return.

\remarks
   The last available interface is the default loop back interface.

*/
IFX_int_t DTI_CLI_SendFragmentedFunctionRegister(
                                 IFX_void_t                           *pDtiAgentContext,
                                 IFX_void_t                           *pCallbackContext,
                                 const IFX_char_t                     *pIfName,
                                 DTI_CliSendFragmentedCommandFunc_t   pSendFragFct,
                                 IFX_uint_t                           responceBufferSize)
{
   return( DTI_CLI_SendFunctionRegisterCommon( pDtiAgentContext, 
            pCallbackContext, pIfName, IFX_NULL, pSendFragFct, responceBufferSize));
}


/**
   This function stops the given DTI CLI interface within the DTI Agent.
   - unregisters the send function.
   - disable the event callback.
   - free allocated memory.

\param
   pDtiAgentContext  - points to the DTI Agent context.
\param
   pIfNumber         - CLI interface number.

\return
   In case of success, the CLI Send Fct is unregistered.
   In case of error IFX_ERROR or a negative value will be return.

\remarks
   The last available interface is the default loop back interface.

*/
IFX_int_t DTI_CLI_InterfaceStop(
                                 IFX_void_t  *pDtiAgentContext,
                                 IFX_int_t   ifNumber)
{
#if defined(INCLUDE_CLI_SUPPORT) && (INCLUDE_CLI_SUPPORT == 1)
   IFX_int_t         retVal = IFX_SUCCESS;
   DTI_AgentCtx_t    *pAgentCtx = (DTI_AgentCtx_t *)pDtiAgentContext;

   if (!pAgentCtx)
   {
      DTI_PRN_USR_ERR_NL(DTI_CLI, DTI_PRN_LEVEL_ERR,
         ("Error: CLI IF STOP - IF[%d], missing agent context" DTI_CRLF,
          ifNumber));

      return DTI_ERR_NULL_PTR_ARG;
   }

   if ( (ifNumber < 0) || (ifNumber >= DTI_CLI_MAX_CLI_INTERFACES) )
   {
      DTI_PRN_USR_ERR_NL(DTI_CLI, DTI_PRN_LEVEL_ERR,
         ("Error: CLI IF STOP - invalid IF number %d (max %d -1)" DTI_CRLF,
           ifNumber, DTI_CLI_MAX_CLI_INTERFACES));

      return DTI_ERR_CONFIGURATION;
   }

   if (pAgentCtx->bListenRun != IFX_TRUE)
   {
      DTI_PRN_USR_ERR_NL(DTI_CLI, DTI_PRN_LEVEL_WRN,
         ("WARNING: CLI IF STOP - IF[%d], agent not running" DTI_CRLF, ifNumber));
   }

   retVal = DTI_CLI_singleInterface_Stop(&pAgentCtx->cliControl[ifNumber]);

   return retVal;

#else
   DTI_PRN_USR_ERR_NL(DTI_CLI, DTI_PRN_LEVEL_ERR,
      ("Error: CLI Fct Unegister - CLI NOT ENABLED" DTI_CRLF));

   return DTI_ERR_CONFIGURATION;
#endif      /* #if defined(INCLUDE_CLI_SUPPORT) && (INCLUDE_CLI_SUPPORT == 1) */
}



/**
   This function sends a CLI Event to the waiting remote clients.

\param
   pDtiAgentContext     - points to the DTI Agent context.
\param
   pIfNumber            - CLI interface number.
\param
   pEventOut            - points to the CLI Event string to send.
\param
   pEventOutSize_byte   - sizeof the Event String to send.

\return
   In case of success, the CLI Event Send has been done.
   In case of error IFX_ERROR or a negative value will be return.

\remarks
   This function checks all running worker threads if they accepts CLI events
   and distributes the string.
*/
IFX_int_t DTI_CLI_InterfaceEventSend(
                                 IFX_void_t  *pDtiAgentContext,
                                 IFX_int_t   ifNumber,
                                 IFX_char_t  *pEventOut,
                                 IFX_uint_t  eventOutSize_byte)
{
#if defined(INCLUDE_CLI_SUPPORT) && (INCLUDE_CLI_SUPPORT == 1)
   IFX_int_t         workerIdx, retVal = IFX_SUCCESS;
   DTI_AgentCtx_t    *pAgentCtx = (DTI_AgentCtx_t *)pDtiAgentContext;
   DTI_cli_control_t *pCliControl = IFX_NULL;

   if (!pAgentCtx)
   {
      DTI_PRN_USR_ERR_NL(DTI_CLI, DTI_PRN_LEVEL_ERR,
         ("Error: CLI IF Event - IF[%d], missing agent context" DTI_CRLF,
          ifNumber));

      return DTI_ERR_NULL_PTR_ARG;
   }

   if ( (pAgentCtx->bListenRun != IFX_TRUE) || (pAgentCtx->bControlAutoCliMsgSupport != IFX_TRUE) )
   {
      DTI_PRN_USR_ERR_NL(DTI_CLI, DTI_PRN_LEVEL_WRN,
         ("WARNING: CLI IF Event - IF[%d], agent %s" DTI_CRLF,
           ifNumber,
           (pAgentCtx->bListenRun != IFX_TRUE) ? "not running" : "events not supported" ));

      return DTI_ERR_CONFIGURATION;
   }

   if ( (ifNumber < 0) || (ifNumber >= DTI_CLI_MAX_CLI_INTERFACES) )
   {
      DTI_PRN_USR_ERR_NL(DTI_CLI, DTI_PRN_LEVEL_ERR,
         ("Error: CLI IF Event - invalid IF number %d (max %d -1)" DTI_CRLF,
           ifNumber, DTI_CLI_MAX_CLI_INTERFACES));

      return DTI_ERR_CONFIGURATION;
   }

   pCliControl = &pAgentCtx->cliControl[ifNumber];

   for (workerIdx = 0; workerIdx < DTI_NUM_OF_USED_WORKER_GET(pAgentCtx); workerIdx++)
   {
      if (pCliControl->bAcceptEvents[workerIdx] == 1)
      {
         if (pAgentCtx->pWorker[workerIdx] != IFX_NULL)
         {
            (void)DTI_CLI_eventWrite(
                     pAgentCtx, pAgentCtx->pWorker[workerIdx], pCliControl,
                     pEventOut, eventOutSize_byte);
         }
      }
   }


   return retVal;

#else
   DTI_PRN_USR_ERR_NL(DTI_CLI, DTI_PRN_LEVEL_ERR,
      ("Error: CLI Fct Unegister - CLI NOT ENABLED" DTI_CRLF));

   return DTI_ERR_CONFIGURATION;
#endif      /* #if defined(INCLUDE_CLI_SUPPORT) && (INCLUDE_CLI_SUPPORT == 1) */
}




/**
   CLI Control - process cli group
*/
IFX_int_t DTI_CLI_cliControlGrCliProcess(
                        DTI_AgentCtx_t          *pAgentCtx,
                        IFX_int_t               workerNum,
                        const IFX_char_t        *pArgs,
                        IFX_uint_t              argLen,
                        IFX_int_t               commandNum,
                        IFX_char_t              *pResponseBuffer,
                        IFX_uint32_t            responseBufferSize_byte)
{
#if defined(INCLUDE_CLI_SUPPORT) && (INCLUDE_CLI_SUPPORT == 1)


   IFX_int_t         responceLen = 0;
   IFX_uint_t        remain = argLen;
   DTI_PacketError_t packetError = DTI_eNoError;
   IFX_char_t        *pCurrArg = (IFX_char_t *)pArgs;
   DTI_WorkerCtx_t   *pWorkerCtx = IFX_NULL;

   if (pAgentCtx == IFX_NULL)
   {
      DTI_PRN_USR_ERR_NL(DTI_CLI, DTI_PRN_LEVEL_ERR,
         ("Error: CLI Control - agent null ptr." DTI_CRLF));

      return IFX_ERROR;
   }

   if (workerNum >= DTI_NUM_OF_USED_WORKER_GET(pAgentCtx))
   {
      DTI_PRN_USR_DBG_NL(DTI_CLI, DTI_PRN_LEVEL_HIGH,
         ("CLI Control: Group[%02d <%s>] - invalid DTI Worker Thread Number."DTI_CRLF,
           DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GROUP_CLI),
           DTI_cliControlGroupEntries[DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GROUP_CLI)].pKeywordName));

      packetError = DTI_eErrConfiguration;
   }
   else
   {
      pWorkerCtx = pAgentCtx->pWorker[workerNum];
   }

   if ( pWorkerCtx == IFX_NULL)
   {
      responceLen = DTI_snprintf(pResponseBuffer, (IFX_int_t)responseBufferSize_byte,
         "%s nReturn=-1 %s <cmd> <%s> - invalid config",
          (IFX_char_t *)DTI_CLI_CONTROL_PREFIX,
          DTI_cliControlGroupEntries[DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GROUP_CLI)].pKeywordName,
          pArgs );

      return responceLen;
   }

   switch (commandNum)
   {
      case DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GR_CLI_BUFSIZE):
         {
            IFX_uint_t bufferIdx = ~0, cliIfNum = ~0, bufferSize = 0;

            /* "@DTI# bufferSize <IF Num> <0/1/2> (show tx/rx/evt buffer size)" */
            if (DTI_cliControlGetNextDigitVal(
                        &pCurrArg, &remain, &cliIfNum, 0) == IFX_SUCCESS)
            {
               (void)DTI_cliControlGetNextDigitVal(
                           &pCurrArg, &remain, &bufferIdx, 0);
            }

            if ( (cliIfNum < DTI_CLI_MAX_CLI_INTERFACES) &&
                 (bufferIdx != (IFX_uint_t)(~0)) )
            {
               bufferSize = (IFX_uint_t)
                              DTI_CLI_ConfigGet_Buffer(
                                       pAgentCtx,
                                       &pAgentCtx->cliControl[cliIfNum],
                                       &pWorkerCtx->dtiProtocolServerCtx,
                                       &pWorkerCtx->cliEventCtx,
                                       &packetError,
                                       bufferIdx);
            }
            else
            {
               packetError = DTI_eErrPortOutOfRange;
            }

            if (packetError == DTI_eNoError)
            {
               responceLen = DTI_snprintf(pResponseBuffer, (IFX_int_t)responseBufferSize_byte,
                  "%s nReturn=0 <%s %s> CLI IF NUM = %d, buffer = %d, size = %d",
                  (IFX_char_t *)DTI_CLI_CONTROL_PREFIX,
                  DTI_cliControlGroupEntries[DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GROUP_CLI)].pKeywordName,
                  DTI_cliControlGrCliEntries[commandNum].pKeywordName,
                  cliIfNum, bufferIdx, bufferSize);
            }
         }
         break;

      case DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GR_CLI_EVT_CREATE):
         {
            IFX_uint_t bufferSize = ~0, maxEventSize = ~0;

            /* "@DTI# eventCreate <evt buffersize> <max evt size> (create event handling)" */
            if (DTI_cliControlGetNextDigitVal(
                        &pCurrArg, &remain, &bufferSize, 0) == IFX_SUCCESS)
            {
               (void)DTI_cliControlGetNextDigitVal(
                           &pCurrArg, &remain, &maxEventSize, 0);
            }

            if ( (bufferSize < DTI_CLI_EVENT_MAX_BUFFER) &&
                 (maxEventSize < DTI_CLI_EVENT_MAX_BUFFER) )
            {
               if (DTI_CLI_workerEventCreate(
                        pAgentCtx, workerNum,
                        bufferSize, maxEventSize) != IFX_SUCCESS)
               {
                  packetError = DTI_eErrPortOperation;
               }
            }
            else
            {
               packetError = DTI_eErrPortOutOfRange;
            }

            if (packetError == DTI_eNoError)
            {
               responceLen = DTI_snprintf(pResponseBuffer, (IFX_int_t)responseBufferSize_byte,
                  "%s nReturn=0 <%s %s %s> - DONE",
                  (IFX_char_t *)DTI_CLI_CONTROL_PREFIX,
                  DTI_cliControlGroupEntries[DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GROUP_CLI)].pKeywordName,
                  DTI_cliControlGrCliEntries[commandNum].pKeywordName,
                  pArgs);
            }
         }
         break;

      case DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GR_CLI_EVT_CLOSE):
         {
            /* "@DTI# eventClose (close event handling)" */
            if (DTI_CLI_workerEventRelease(
                     pAgentCtx,
                     workerNum) != IFX_SUCCESS)
            {
               packetError = DTI_eErrPortOperation;
            }
         }
         break;

      case DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GR_CLI_EVENT):
         {
            /* "@DTI# event <IFNum> <0/1> (switch events ON/OFF)" */
            IFX_uint_t cliIfNum = ~0, cmdValue = ~0;

            if (DTI_cliControlGetNextDigitVal(
                        &pCurrArg, &remain, &cliIfNum, 0) == IFX_SUCCESS)
            {
               (void)DTI_cliControlGetNextDigitVal(
                           &pCurrArg, &remain, &cmdValue, 0);
            }

            if ( (cliIfNum <= DTI_CLI_MAX_CLI_INTERFACES) && (cmdValue != (IFX_uint_t)(~0)) )
            {
               if ( DTI_CLI_workerEventControl(
                        pAgentCtx, workerNum,
                        cliIfNum, (cmdValue == 1) ? IFX_TRUE : IFX_FALSE) != IFX_SUCCESS)
               {
                  packetError = DTI_eErrPortOperation;
               }
            }
            else
            {
               packetError = DTI_eErrPortOutOfRange;
            }

            if (packetError == DTI_eNoError)
            {
               responceLen = DTI_snprintf(pResponseBuffer, (IFX_int_t)responseBufferSize_byte,
                  "%s nReturn=0 <%s %s> - CLI IF[%d] AutoMsg %s DONE",
                  (IFX_char_t *)DTI_CLI_CONTROL_PREFIX,
                  DTI_cliControlGroupEntries[DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GROUP_CLI)].pKeywordName,
                  DTI_cliControlGrCliEntries[commandNum].pKeywordName,
                  cliIfNum, (cmdValue == 1) ? "ENABLE" : "DISABLE");
            }
         }
         break;

      case DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GR_CLI_TR_CMD):
         {
            /* "@DTI# traceCmd <IFNum> <0/1> (cmd msg log OFF/ON)" */
            IFX_uint_t cliIfNum = ~0, cmdValue = ~0;

            if (DTI_cliControlGetNextDigitVal(
                        &pCurrArg, &remain, &cliIfNum, 0) == IFX_SUCCESS)
            {
               (void)DTI_cliControlGetNextDigitVal(
                           &pCurrArg, &remain, &cmdValue, 0);
            }

            if ( (cliIfNum < DTI_CLI_MAX_CLI_INTERFACES) && (cmdValue != (IFX_uint_t)(~0)) )
            {
               pAgentCtx->cliControl[cliIfNum].bCmdMsgTrace = cmdValue & (DTI_CLI_TRACE_CMD_SEND_FLAGS);
            }
            else
            {
               packetError = DTI_eErrPortOutOfRange;
            }

            if (packetError == DTI_eNoError)
            {
               responceLen = DTI_snprintf(pResponseBuffer, (IFX_int_t)responseBufferSize_byte,
                  "%s nReturn=0 <%s %s> - CLI IF[%d] traceCmd %s DONE",
                  (IFX_char_t *)DTI_CLI_CONTROL_PREFIX,
                  DTI_cliControlGroupEntries[DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GROUP_CLI)].pKeywordName,
                  DTI_cliControlGrCliEntries[commandNum].pKeywordName,
                  cliIfNum, (cmdValue == 1) ? "ENABLE" : "DISABLE");
            }
         }
         break;

      case DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GR_CLI_TR_EVT):
         {
            /* "@DTI# traceEvent <IFNum> <0/1> (event msg log OFF/ON)" */
            IFX_uint_t cliIfNum = ~0, cmdValue = ~0;

            if (DTI_cliControlGetNextDigitVal(
                        &pCurrArg, &remain, &cliIfNum, 0) == IFX_SUCCESS)
            {
               (void)DTI_cliControlGetNextDigitVal(
                           &pCurrArg, &remain, &cmdValue, 0);
            }

            if ( (cliIfNum < DTI_CLI_MAX_CLI_INTERFACES) && (cmdValue != (IFX_uint_t)(~0)) )
            {
               pAgentCtx->cliControl[cliIfNum].bEvtMsgTrace = (cmdValue == 1) ? IFX_TRUE : IFX_FALSE;
            }
            else
            {
               packetError = DTI_eErrPortOutOfRange;
            }

            if (packetError == DTI_eNoError)
            {
               responceLen = DTI_snprintf(pResponseBuffer, (IFX_int_t)responseBufferSize_byte,
                  "%s nReturn=0 <%s %s> - CLI IF[%d] traceEvent %s DONE",
                  (IFX_char_t *)DTI_CLI_CONTROL_PREFIX,
                  DTI_cliControlGroupEntries[DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GROUP_CLI)].pKeywordName,
                  DTI_cliControlGrCliEntries[commandNum].pKeywordName,
                  cliIfNum, (cmdValue == 1) ? "ENABLE" : "DISABLE");
            }
         }
         break;

      case DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GR_CLI_DIST_EVT):
         {
            /* "@DTI# distEvent <IFNum> <0/1> (event msg log OFF/ON)" */
            IFX_uint_t cliIfNum = ~0, cmdValue = ~0;

            if (DTI_cliControlGetNextDigitVal(
                        &pCurrArg, &remain, &cliIfNum, 0) == IFX_SUCCESS)
            {
               (void)DTI_cliControlGetNextDigitVal(
                           &pCurrArg, &remain, &cmdValue, 0);
            }

            if ( (cliIfNum < DTI_CLI_MAX_CLI_INTERFACES) && (cmdValue != (IFX_uint_t)(~0)) )
            {
               pAgentCtx->cliControl[cliIfNum].bTestDistEvt = (cmdValue == 1) ? IFX_TRUE : IFX_FALSE;
            }
            else
            {
               packetError = DTI_eErrPortOutOfRange;
            }

            if (packetError == DTI_eNoError)
            {
               responceLen = DTI_snprintf(pResponseBuffer, (IFX_int_t)responseBufferSize_byte,
                  "%s nReturn=0 <%s %s> - CLI IF[%d] distEvent %s DONE",
                  (IFX_char_t *)DTI_CLI_CONTROL_PREFIX,
                  DTI_cliControlGroupEntries[DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GROUP_CLI)].pKeywordName,
                  DTI_cliControlGrCliEntries[commandNum].pKeywordName,
                  cliIfNum, (cmdValue == 1) ? "ENABLE" : "DISABLE");
            }
         }
         break;

      default:
            DTI_PRN_USR_DBG_NL(DTI_CLI, DTI_PRN_LEVEL_HIGH,
               ("CLI Control: Group[%02d <%s>] Opt[%02d <%s>] - <%s>."DTI_CRLF,
                 DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GROUP_CLI),
                 DTI_cliControlGroupEntries[DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GROUP_CLI)].pKeywordName,
                 commandNum,
                 DTI_cliControlGrCliEntries[commandNum].pKeywordName,
                 pArgs ));

            responceLen = DTI_snprintf(pResponseBuffer, (IFX_int_t)responseBufferSize_byte,
               "%s nReturn=0 <%s %s = <not supported>",
                (IFX_char_t *)DTI_CLI_CONTROL_PREFIX,
                DTI_cliControlGroupEntries[DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GROUP_CLI)].pKeywordName,
                DTI_cliControlGrCliEntries[commandNum].pKeywordName );
         break;
   }

   if ( (packetError != DTI_eNoError) && (responceLen == 0) )
   {
      responceLen = DTI_snprintf(pResponseBuffer, (IFX_int_t)responseBufferSize_byte,
         "%s nReturn=-1 %s %s <%s> - failed (DTI Error = %d)",
          (IFX_char_t *)DTI_CLI_CONTROL_PREFIX,
          DTI_cliControlGroupEntries[DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GROUP_CLI)].pKeywordName,
          DTI_cliControlGrCliEntries[commandNum].pKeywordName,
          pArgs, (IFX_int_t)packetError );
   }

   return responceLen;
#else
   IFX_int_t   responceLen = 0;

   responceLen = DTI_snprintf(pResponseBuffer, (IFX_int_t)responseBufferSize_byte,
      "%s nReturn=-1 <%s %s = <not supported>",
       (IFX_char_t *)DTI_CLI_CONTROL_PREFIX,
       DTI_cliControlGroupEntries[DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GROUP_CLI)].pKeywordName,
       DTI_cliControlGrCliEntries[commandNum].pKeywordName );

   return responceLen;
#endif
}


