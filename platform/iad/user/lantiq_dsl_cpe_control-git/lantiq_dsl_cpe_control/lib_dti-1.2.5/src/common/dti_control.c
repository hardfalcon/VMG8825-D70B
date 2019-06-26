/******************************************************************************

                              Copyright (c) 2009
                            Lantiq Deutschland GmbH
                     Am Campeon 3; 85579 Neubiberg, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/** \file
   Debug and Trace Interface - Server/Worker Thread Handling.
*/

/* ============================================================================
   Includes
   ========================================================================= */

#include "dti_osmap.h"

#include "ifx_dti_protocol.h"
#include "ifx_dti_protocol_device.h"
#include "ifx_dti_protocol_cli.h"

#include "dti_protocol_interface.h"
#include "dti_protocol_ext.h"

#include "dti_device.h"
#include "dti_packet_device.h"

#include "dti_agent_interface.h"
#include "dti_statistic.h"
#include "dti_control.h"

/* ============================================================================
   Defines
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
   Local Function Declaration
   ========================================================================= */

DTI_STATIC IFX_int_t DTI_imageCntrlGetFunction(
                        IFX_void_t           *pUserCtx,
                        IFX_int_t            imageId,
                        DTI_ImageControl_t   **ppImageGntrl,
                        DTI_PacketError_t    *pPacketError);

DTI_STATIC IFX_int_t DTI_imageCntrlLockFunction(
                        IFX_void_t           *pUserCtx,
                        IFX_int_t            imageId,
                        IFX_int_t            lockOnOff,
                        DTI_ImageControl_t   **ppImageGntrl,
                        DTI_PacketError_t    *pPacketError);

DTI_STATIC IFX_int_t DTI_packetRecvErrorHandler(
                        DTI_AgentCtx_t          *pAgentCtx,
                        DTI_ProtocolServerCtx_t *pProtocolServerCtx,
                        DTI_Packet_t            *pPacketIn,
                        DTI_PacketError_t       packetError);

DTI_STATIC IFX_uint_t DTI_cliControlSearchKey(
                        DTI_cliConfigKeyword_t  *pKeyWords,
                        const IFX_char_t        **ppInCliControlStr,
                        IFX_uint_t              *pRemainLen);

DTI_STATIC IFX_int_t DTI_cliControlHelpPrint(
                        DTI_cliConfigKeyword_t *pGroupEntry,
                        IFX_char_t             *pDescript,
                        DTI_cliConfigKeyword_t *pKeyWords,
                        IFX_char_t             **ppResponseBuffer,
                        IFX_uint32_t           *pResponseBufferSize_byte);

DTI_STATIC IFX_int_t DTI_cliControlGrConfigProcess(
                        DTI_AgentCtx_t          *pAgentCtx,
                        DTI_WorkerCtx_t         *pWorkerCtx,
                        const IFX_char_t        *pArgs,
                        IFX_uint_t              argLen,
                        IFX_int_t               commandNum,
                        IFX_char_t              *pResponseBuffer,
                        IFX_uint32_t            responseBufferSize_byte);

DTI_STATIC IFX_int_t DTI_cliControlGrDeviceProcess(
                        DTI_AgentCtx_t          *pAgentCtx,
                        DTI_WorkerCtx_t         *pWorkerCtx,
                        const IFX_char_t        *pArgs,
                        IFX_uint_t              argLen,
                        IFX_int_t               commandNum,
                        IFX_char_t              *pResponseBuffer,
                        IFX_uint32_t            responseBufferSize_byte);

DTI_STATIC IFX_int_t DTI_cliControlProcess(
                        DTI_AgentCtx_t          *pAgentCtx,
                        DTI_WorkerCtx_t         *pWorkerCtx,
                        const DTI_Packet_t      *pDtiPacketIn,
                        DTI_Packet_t            *pDtiPacketOut,
                        IFX_uint32_t            bufferOutSize);

DTI_STATIC IFX_int_t DTI_packetHandler_cliControl(
                        DTI_AgentCtx_t          *pAgentCtx,
                        DTI_WorkerCtx_t         *pWorkerCtx,
                        DTI_Packet_t            *pDtiPacketIn,
                        IFX_uint_t              dtiBufInLen,
                        DTI_Packet_t            *pDtiPacketOut,
                        IFX_uint_t              dtiBufOutLen);

DTI_STATIC IFX_int_t DTI_packetGroupHandler(
                        DTI_AgentCtx_t          *pAgentCtx,
                        DTI_WorkerCtx_t         *pWorkerCtx,
                        DTI_Packet_t            *pPacketIn,
                        IFX_uint_t              dtiBufInLen,
                        DTI_Packet_t            *pPacketOut,
                        IFX_uint_t              dtiBufOutLen);

DTI_STATIC IFX_int_t DTI_WorkerContextInit(
                        DTI_AgentCtx_t          *pAgentCtx,
                        DTI_Connection_t        *pNewDtiCon,
                        IFX_int_t               workerNum,
                        DTI_WorkerCtx_t         **ppWorkerCtx);

DTI_STATIC IFX_int_t DTI_WorkerContextRelease(
                        DTI_AgentCtx_t          *pAgentCtx,
                        DTI_WorkerCtx_t         *pWorkerCtx);

DTI_STATIC IFX_int_t DTI_Worker(
                        DTI_AgentCtx_t          *pAgentCtx,
                        DTI_WorkerCtx_t         *pWorkerCtx,
                        volatile IFX_boolean_t  *pBShutdown);

DTI_STATIC IFX_int_t DTI_WorkerThreadFct(
                        IFXOS_ThreadParams_t    *pThreadParams);

DTI_STATIC IFX_int_t DTI_WorkerThreadGet(
                        DTI_AgentCtx_t          *pAgentCtx);

DTI_STATIC IFX_int_t DTI_WorkerThreadStartup(
                        DTI_AgentCtx_t          *pAgentCtx,
                        DTI_Connection_t        *pNewDtiCon);

DTI_STATIC IFX_int_t DTI_WorkerThreadStopAll(
                        DTI_AgentCtx_t          *pAgentCtx);

DTI_STATIC IFX_int_t DTI_ListenerThread (
                        IFXOS_ThreadParams_t *pThreadParams);

/* ============================================================================
   Variables
   ========================================================================= */

/* Create tcp debug module - user part */
IFXOS_PRN_USR_MODULE_CREATE(DTI_DBG, IFXOS_PRN_LEVEL_NORMAL);

/* Default values */
IFX_boolean_t  DTI_configLocked = IFX_FALSE;



/*
   CLI control - list of group keywords
*/
DTI_cliConfigKeyword_t DTI_cliControlGroupEntries[] =
{
   DTI_CLI_CNTRL_GROUP_CONFIG,
   DTI_CLI_CNTRL_GROUP_DEVICE,
   DTI_CLI_CNTRL_GROUP_CLI,
   DTI_CLI_CNTRL_GROUP_HELP,
   {0xFF, IFX_NULL, IFX_NULL}
};

/*
   CLI control - Config Group keywords
*/
DTI_cliConfigKeyword_t DTI_cliControlGrConfigEntries[] =
{
   DTI_CLI_CNTRL_GR_CONFIG_COMMANDS,
   {0xFF, IFX_NULL, IFX_NULL}
};

/*
   CLI control - Config Image keywords
*/
DTI_cliConfigKeyword_t DTI_cliControlConfigImageEntries[] =
{
   DTI_CLI_CNTRL_GR_CONFIG_IMAGE_COMMANDS,
   {0xFF, IFX_NULL, IFX_NULL}
};


/*
   CLI control - Device Group keywords
*/
DTI_cliConfigKeyword_t DTI_cliControlGrDeviceEntries[] =
{
   DTI_CLI_CNTRL_GR_DEVICE_COMMANDS,
   {0xFF, IFX_NULL, IFX_NULL}
};

/*
   CLI control - CLI Group keywords
*/
DTI_cliConfigKeyword_t DTI_cliControlGrCliEntries[] =
{
#if defined(INCLUDE_CLI_SUPPORT) && (INCLUDE_CLI_SUPPORT == 1)
   DTI_CLI_CNTRL_GR_CLI_COMMANDS,
#endif
   {0xFF, IFX_NULL, IFX_NULL}
};

/* ============================================================================
   Local Functions
   ========================================================================= */

/**
   Function type to get a free image control struct to save a image.

   On a "image load start" request the DTI Library reqires a control struct
   from the user context to save the image.

1. Arg:
   - points to the user given context (argument of the packet handler)
2. Arg:
   - return pointer, returns the free image control struct from the user context.

return
   On Success, the function returns IFX_SUCCESS and sets the return pointer else
   IFX_ERROR an a NULL pointer is returned.

*/

/**
   Check and Return the requested image control struct from the given user context.

\remarks
   This function is called as callback out form the connection context on
   the processing of a corresponding packet.

\param
   pUserCtx       - points to the user given context (argument of the packet handler)
\param
   imageId        - image id of the requested image.
\param
   ppImageGntrl   - return pointer,
                    Lock:   returns the free image control struct from the user context.
                    Unlock: not used
\param
   pPacketError   - return pointer, returns the DTI packet error.

\return
   IFX_SUCCESS:
      sets the return pointer and locks the image for use
   IFX_ERROR:
      NULL pointer is returned.
*/
DTI_STATIC IFX_int_t DTI_imageCntrlGetFunction(
                        IFX_void_t           *pUserCtx,
                        IFX_int_t            imageId,
                        DTI_ImageControl_t   **ppImageGntrl,
                        DTI_PacketError_t    *pPacketError)
{
   IFX_int_t      retVal = IFX_SUCCESS;
   DTI_AgentCtx_t *pAgentCtx = (DTI_AgentCtx_t *)pUserCtx;

   if ( (pAgentCtx == IFX_NULL) || (pPacketError == IFX_NULL) )
   {
      DTI_PRN_USR_ERR_NL(DTI_DBG, DTI_PRN_LEVEL_ERR,
         ("ERROR: Callback Image Cntrl Get - missing return ptr."DTI_CRLF));

      if (ppImageGntrl)
      {
         *ppImageGntrl = IFX_NULL;
      }

      if (pPacketError)
      {
         *pPacketError = DTI_eErrConfiguration;
      }

      return IFX_ERROR;
   }

   if (imageId >= DTI_MAX_NUM_OF_IMAGES)
   {
      DTI_PRN_USR_ERR_NL(DTI_DBG, DTI_PRN_LEVEL_ERR,
         ("ERROR: Callback Image Lock - invalid image ID = %d >= max %d."DTI_CRLF,
           imageId, DTI_MAX_NUM_OF_IMAGES));

      *ppImageGntrl = IFX_NULL;
      *pPacketError = DTI_eErrPortOutOfRange;

      return IFX_ERROR;
   }

   *pPacketError = DTI_eNoError;

   if (DTI_LockGet(&pAgentCtx->dataLock) != IFX_SUCCESS)
   {
      DTI_PRN_USR_ERR_NL(DTI_DBG, DTI_PRN_LEVEL_ERR,
         ("ERROR: Callback Image Cntrl Get - semaphore."DTI_CRLF));

      if (ppImageGntrl)
      {
         *ppImageGntrl = IFX_NULL;
      }
      *pPacketError = DTI_eErrPortOperation;

      return IFX_ERROR;
   }

   if (pAgentCtx->imageCntrl[imageId].useCount != 0)
   {
      DTI_PRN_USR_ERR_NL(DTI_DBG, DTI_PRN_LEVEL_ERR,
         ("ERROR: Callback Image Lock - image already in use (count = %d)"DTI_CRLF,
         pAgentCtx->imageCntrl[imageId].useCount));

      *ppImageGntrl = IFX_NULL;
      *pPacketError = DTI_eErrPortOutOfRange;

      retVal = IFX_ERROR;
   }
   else
   {
      /* mark the image for changes */
      DTI_PRN_USR_DBG_NL(DTI_DBG, DTI_PRN_LEVEL_HIGH,
         ("Callback Image Lock - start image update (image ID %d)"DTI_CRLF,
           imageId));

      pAgentCtx->imageCntrl[imageId].useCount = -1;
      *ppImageGntrl = &pAgentCtx->imageCntrl[imageId];
   }

   DTI_LockRelease(&pAgentCtx->dataLock);

   return retVal;
}


/**
   Check and lock the requested image from the given user context.

\remarks
   This function is called as callback out form the connection context on
   the processing of a corresponding packet.

\param
   pUserCtx       - points to the user given context (argument of the packet handler)
\param
   imageId        - image id of the requested image.
\param
   lockOnOff      - lock / unlock the image.
\param
   ppImageGntrl   - return pointer,
                    Lock:   returns the free image control struct from the user context.
                    Unlock: not used
\param
   pPacketError   - return pointer, returns the DTI packet error.

\return
   IFX_SUCCESS:
      lockOnOff = 1 (lock):   sets the return pointer and locks the image for use
      lockOnOff = 0 (unlock): unlocks the image from use

   IFX_ERROR:
      lockOnOff = 1 (lock):   NULL pointer is returned.
      lockOnOff = 0 (unlock): return pointer not used.
*/
DTI_STATIC IFX_int_t DTI_imageCntrlLockFunction(
                        IFX_void_t           *pUserCtx,
                        IFX_int_t            imageId,
                        IFX_int_t            lockOnOff,
                        DTI_ImageControl_t   **ppImageGntrl,
                        DTI_PacketError_t    *pPacketError)
{
   IFX_int_t      retVal = IFX_SUCCESS;
   DTI_AgentCtx_t *pAgentCtx = (DTI_AgentCtx_t *)pUserCtx;

   if ( (pAgentCtx == IFX_NULL) || (pPacketError == IFX_NULL) )
   {
      DTI_PRN_USR_ERR_NL(DTI_DBG, DTI_PRN_LEVEL_ERR,
         ("ERROR: Callback Image Lock - missing return ptr."DTI_CRLF));

      if (ppImageGntrl)
      {
         *ppImageGntrl = IFX_NULL;
      }

      if (pPacketError)
      {
         *pPacketError = DTI_eErrConfiguration;
      }

      return IFX_ERROR;
   }

   *pPacketError = DTI_eNoError;

   if (DTI_LockGet(&pAgentCtx->dataLock) != IFX_SUCCESS)
   {
      DTI_PRN_USR_ERR_NL(DTI_DBG, DTI_PRN_LEVEL_ERR,
         ("ERROR: Callback Image Lock - semaphore."DTI_CRLF));

      if (ppImageGntrl)
      {
         *ppImageGntrl = IFX_NULL;
      }
      *pPacketError = DTI_eErrConfiguration;

      return IFX_ERROR;
   }

   if ( (imageId >= DTI_MAX_NUM_OF_IMAGES) ||
        (lockOnOff && (ppImageGntrl == IFX_NULL)) )
   {
      DTI_PRN_USR_ERR_NL(DTI_DBG, DTI_PRN_LEVEL_ERR,
         ("ERROR: Callback Image Lock - args, image ID = %d, retPtr = %s"DTI_CRLF,
         imageId, (ppImageGntrl == IFX_NULL) ? "invalid" : "valid"));

      if (ppImageGntrl)
      {
         *ppImageGntrl = IFX_NULL;
         *pPacketError = DTI_eErrPortOutOfRange;
      }
      else
      {
         *pPacketError = DTI_eErrConfiguration;
      }
      retVal = IFX_ERROR;
   }
   else
   {
      if (lockOnOff)
      {
         if (ppImageGntrl)
         {
            if (pAgentCtx->imageCntrl[imageId].useCount != -1)
            {
               *ppImageGntrl = &pAgentCtx->imageCntrl[imageId];
               pAgentCtx->imageCntrl[imageId].useCount++;
            }
            else
            {
               DTI_PRN_USR_ERR_NL(DTI_DBG, DTI_PRN_LEVEL_ERR,
                  ("ERROR: Callback Image Lock - image blocked for config."DTI_CRLF));

               *ppImageGntrl = IFX_NULL;
               *pPacketError = DTI_eErrPortOperation;
            }
         }
         else
         {
            DTI_PRN_USR_ERR_NL(DTI_DBG, DTI_PRN_LEVEL_ERR,
               ("ERROR: Callback Image Lock - missing imageCntrl ptr."DTI_CRLF));

            *pPacketError = DTI_eErrConfiguration;
            retVal = IFX_ERROR;
         }
      }
      else
      {
         if (pAgentCtx->imageCntrl[imageId].useCount > 0)
         {
            pAgentCtx->imageCntrl[imageId].useCount--;
         }
         else
         {
            pAgentCtx->imageCntrl[imageId].useCount = 0;
         }
      }
   }

   DTI_LockRelease(&pAgentCtx->dataLock);

   return retVal;
}


/*
   DTI_eErrUnknown;
   DTI_eErrNetwork;
   DTI_eErrMalformedPacket
   DTI_eErrInvalidPayloadSize
*/
DTI_STATIC IFX_int_t DTI_packetRecvErrorHandler(
                        DTI_AgentCtx_t          *pAgentCtx,
                        DTI_ProtocolServerCtx_t *pProtocolServerCtx,
                        DTI_Packet_t            *pPacketIn,
                        DTI_PacketError_t       packetError)
{
   IFX_int_t            retVal = IFX_SUCCESS;

   switch(packetError)
   {
      case DTI_eErrNetwork:
         /* network problem, connection lost --> sutdown */
         retVal = IFX_ERROR;
         break;

      case DTI_eNoError:
         /* BUG: call error handler with no error --> sutdown */
         retVal = IFX_ERROR;
         break;

      case DTI_eErrUnknown:
      case DTI_eErrMalformedPacket:
      case DTI_eErrInvalidPayloadSize:
      case DTI_eErrTimeout:
      case DTI_eErrUnknownMsg:
      case DTI_eErrInvalidParameters:
      case DTI_eErrInvalidPacketType:
      case DTI_eErrConfiguration:
      case DTI_eErrPortOutOfRange:
      case DTI_eErrPortOpen:
      case DTI_eErrPortOperation:
      case DTI_eErrPortAutoOperation:
      default:
         /* Send Error Packet */
         retVal = DTI_packetErrorSet(
                        pPacketIn,
                        pProtocolServerCtx->packetOut.pPacket,
                        sizeof(pProtocolServerCtx->packetOut.buffer),
                        packetError);

         break;
   }

   if (retVal == IFX_SUCCESS)
   {
      /* valid error packet for responce */
      DTI_packetShow (
         pProtocolServerCtx->packetOut.pPacket, IFX_TRUE, IFX_FALSE, "error send",
         (pProtocolServerCtx->packetOut.pPacket->header.error == DTI_eNoError) ?
                IFXOS_PRN_LEVEL_LOW : IFXOS_PRN_LEVEL_HIGH);

      if (DTI_packetSend( &pProtocolServerCtx->dtiCon,
                          pProtocolServerCtx->packetOut.pPacket) != IFX_SUCCESS)
      {
         DTI_PRN_USR_ERR_NL(DTI_DBG, IFXOS_PRN_LEVEL_ERR,
            ("Error: Packet Recv Error Handler - send packet." DTI_CRLF));

         return IFX_ERROR;
      }
   }

   return IFX_SUCCESS;
}


/**
   Search a keyword within the given table.
*/
DTI_STATIC IFX_uint_t DTI_cliControlSearchKey(
                        DTI_cliConfigKeyword_t  *pKeyWords,
                        const IFX_char_t        **ppInCliControlStr,
                        IFX_uint_t              *pRemainLen)
{
   IFX_uint_t        i, remainLen = *pRemainLen;
   const IFX_char_t  *pInCliControlStrCurr = *ppInCliControlStr;

   /* remove leading blanks */
   while ( (DTI_IsSpace((IFX_int_t)*pInCliControlStrCurr)) && (remainLen) )
   {
      pInCliControlStrCurr++;
      remainLen--;
   }

   for (i = 0; pKeyWords[i].pKeywordName != IFX_NULL; i++)
   {
      if (remainLen >= (IFX_uint_t)DTI_StrLen(pKeyWords[i].pKeywordName))
      {
         if (DTI_StrNCmp( pKeyWords[i].pKeywordName,
                          pInCliControlStrCurr,
                          DTI_StrLen(pKeyWords[i].pKeywordName)) == 0)
         {
            if (  ((DTI_StrLen(pInCliControlStrCurr) > DTI_StrLen(pKeyWords[i].pKeywordName))&&
                   (DTI_IsSpace((IFX_int_t)pInCliControlStrCurr[DTI_StrLen(pKeyWords[i].pKeywordName)])))
                 ||(DTI_StrLen(pInCliControlStrCurr) == DTI_StrLen(pKeyWords[i].pKeywordName)) )
               /* found --> break */
               break;
         }
      }
   }

   if (pKeyWords[i].pKeywordName == IFX_NULL)
   {
      return i;
   }

   /* set to next word and remove leading blanks */
   pInCliControlStrCurr += DTI_StrLen(pKeyWords[i].pKeywordName);
   remainLen            -= (IFX_uint_t)DTI_StrLen(pKeyWords[i].pKeywordName);

   while ( (DTI_IsSpace((IFX_int_t)*pInCliControlStrCurr)) && (remainLen) )
   {
      pInCliControlStrCurr++;
      remainLen--;
   }

   *pRemainLen        = remainLen;
   *ppInCliControlStr = pInCliControlStrCurr;

   return i;
}

/**
   CLI Control - process config group
*/
DTI_STATIC IFX_int_t DTI_cliControlHelpPrint(
                        DTI_cliConfigKeyword_t *pGroupEntry,
                        IFX_char_t             *pDescript,
                        DTI_cliConfigKeyword_t *pKeyWords,
                        IFX_char_t             **ppResponseBuffer,
                        IFX_uint32_t           *pResponseBufferSize_byte)
{
   IFX_int_t      i, responceLen, respAll = 0;
   IFX_char_t     *pResponseBuffer        = *ppResponseBuffer;
   IFX_uint32_t   responseBufferSize_byte = *pResponseBufferSize_byte;

   if (pGroupEntry)
   {
      responceLen = DTI_snprintf(pResponseBuffer, (IFX_int_t)responseBufferSize_byte,
         DTI_CRLF "DTI CLI Control: [%d] \"%s\"\t%s" DTI_CRLF,
          pGroupEntry->keywordIdx,
          (pGroupEntry->pKeywordName != IFX_NULL) ? pGroupEntry->pKeywordName : "invalid",
          (pGroupEntry->pKeywordHelp != IFX_NULL) ? pGroupEntry->pKeywordHelp : "no help");
   }
   else
   {
      responceLen = DTI_snprintf(pResponseBuffer, (IFX_int_t)responseBufferSize_byte,
         DTI_CRLF "DTI CLI Control: %s" DTI_CRLF,
            pDescript ? pDescript : "help");
   }

   responseBufferSize_byte -= (IFX_uint32_t)responceLen;
   pResponseBuffer         += responceLen;
   respAll                 += responceLen;

   for (i = 0; pKeyWords[i].pKeywordName != IFX_NULL; i++)
   {
      if (responseBufferSize_byte > 0)
      {
         responceLen = DTI_snprintf(pResponseBuffer, (IFX_int_t)responseBufferSize_byte,
            "\t[%d] \"%s\"\t%s" DTI_CRLF,
             i,
             (pKeyWords[i].pKeywordName != IFX_NULL) ?
               pKeyWords[i].pKeywordName : "invalid",
             (pKeyWords[i].pKeywordHelp != IFX_NULL) ?
               pKeyWords[i].pKeywordHelp : "no help" );

         responseBufferSize_byte -= (IFX_uint32_t)responceLen;
         pResponseBuffer         += responceLen;
         respAll                 += responceLen;
      }
   }

   *ppResponseBuffer         = pResponseBuffer;
   *pResponseBufferSize_byte = responseBufferSize_byte;

   return respAll;
}

/**
   CLI Control - generate help.
*/
DTI_STATIC IFX_int_t DTI_cliControlGrConfigHelp(
                        DTI_AgentCtx_t          *pAgentCtx,
                        DTI_WorkerCtx_t         *pWorkerCtx,
                        IFX_int_t               groupNum,
                        IFX_char_t              *pResponseBuffer,
                        IFX_uint32_t            responseBufferSize_byte)
{
   IFX_int_t   i, currGroup, respLen = 0;

   for (i = 0; DTI_cliControlGroupEntries[i].pKeywordName != IFX_NULL; i++)
   {
      if ( (DTI_cliControlGroupEntries[groupNum].pKeywordName != IFX_NULL) &&
           (groupNum != DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GROUP_HELP)) )
      {
         currGroup = groupNum;
      }
      else
      {
         currGroup = i;
      }

      switch (currGroup)
      {
         case DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GROUP_CONFIG):
            respLen += DTI_cliControlHelpPrint(
                           &DTI_cliControlGroupEntries[DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GROUP_CONFIG)],
                           IFX_NULL,
                           DTI_cliControlGrConfigEntries,
                           &pResponseBuffer, &responseBufferSize_byte);
            break;

         case DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GROUP_DEVICE):
            respLen += DTI_cliControlHelpPrint(
                           &DTI_cliControlGroupEntries[DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GROUP_DEVICE)],
                           IFX_NULL,
                           DTI_cliControlGrDeviceEntries,
                           &pResponseBuffer, &responseBufferSize_byte);
            break;

         case DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GROUP_CLI):
            respLen += DTI_cliControlHelpPrint(
                           &DTI_cliControlGroupEntries[DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GROUP_CLI)],
                           IFX_NULL,
                           DTI_cliControlGrCliEntries,
                           &pResponseBuffer, &responseBufferSize_byte);
            break;

         case DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GROUP_HELP):
         default:
            break;
      }

      if ( (DTI_cliControlGroupEntries[groupNum].pKeywordName != IFX_NULL) &&
           (groupNum != DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GROUP_HELP)) )
      {
         break;
      }

   }

   return respLen;
}

/**
   CLI Control - process config group
*/
DTI_STATIC IFX_int_t DTI_cliControlGrConfigProcess(
                        DTI_AgentCtx_t          *pAgentCtx,
                        DTI_WorkerCtx_t         *pWorkerCtx,
                        const IFX_char_t        *pArgs,
                        IFX_uint_t              argLen,
                        IFX_int_t               commandNum,
                        IFX_char_t              *pResponseBuffer,
                        IFX_uint32_t            responseBufferSize_byte)
{
   IFX_int_t   responceLen = 0;
   IFX_uint_t  remain = argLen;
   IFX_char_t  *pCurrArg = (IFX_char_t *)pArgs;

   switch (commandNum)
   {
      /*
         set debug level
      */
      case DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GR_CONFIG_DBGLEVEL):
         {
            /* @DTI# config dbglevel <new level> */
            IFX_uint_t  ulValDebugLevel = ~0;

            if ( DTI_cliControlGetNextDigitVal(
                        &pCurrArg, &remain, &ulValDebugLevel, 0) == IFX_SUCCESS)
            {
               DTI_DebugLevelSet(ulValDebugLevel);

               DTI_PRN_USR_DBG_NL(DTI_DBG, DTI_PRN_LEVEL_HIGH,
                  ("CLI Control: Group[%02d <%s>] Opt[%02d <%s>] - <%s>." DTI_CRLF,
                    DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GROUP_CONFIG),
                    DTI_cliControlGroupEntries[DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GROUP_CONFIG)].pKeywordName,
                    DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GR_CONFIG_DBGLEVEL),
                    DTI_cliControlGrConfigEntries[DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GR_CONFIG_DBGLEVEL)].pKeywordName,
                    pArgs ));
            }

            responceLen = DTI_snprintf(pResponseBuffer, (IFX_int_t)responseBufferSize_byte,
               "%s nReturn=0 <%s %s = %d>",
               (IFX_char_t *)DTI_CLI_CONTROL_PREFIX,
               DTI_cliControlGroupEntries[DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GROUP_CONFIG)].pKeywordName,
               DTI_cliControlGrConfigEntries[DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GR_CONFIG_DBGLEVEL)].pKeywordName,
               (IFX_int_t)IFXOS_PRN_USR_LEVEL_GET(DTI_DBG));
         }
         break;

      case DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GR_CONFIG_IMAGE):
         {
            /* @DTI# config image <imageId> <write [name] | release | show> */
            IFX_int_t   retVal = IFX_ERROR;
            IFX_uint_t  i, imageId = ~0;

            i = DTI_cliControlSearchKey(
                                 DTI_cliControlConfigImageEntries,
                                 (const IFX_char_t **)&pCurrArg, &remain);

            if (DTI_cliControlConfigImageEntries[i].pKeywordName != IFX_NULL)
            {
               retVal = DTI_cliControlGetNextDigitVal(&pCurrArg, &remain, &imageId, 0);
            }

            switch(i)
            {
               case DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GR_CONFIG_IMAGE_SHOW):
                  {
                     IFX_uint_t  imageCnt = 0, idx, tmpLen = 0;

                     for (idx = 0; idx < DTI_MAX_NUM_OF_IMAGES; idx++)
                     {
                        if (pAgentCtx->imageCntrl[idx].pData)
                        {
                           imageCnt++;
                        }
                     }

                     tmpLen = DTI_snprintf(pResponseBuffer, (IFX_int_t)responseBufferSize_byte,
                        "%s nReturn=0 <%s %s %s> num of images = %d>" DTI_CRLF,
                        (IFX_char_t *)DTI_CLI_CONTROL_PREFIX,
                        DTI_cliControlGroupEntries[DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GROUP_CONFIG)].pKeywordName,
                        DTI_cliControlGrConfigEntries[DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GR_CONFIG_IMAGE)].pKeywordName,
                        DTI_cliControlConfigImageEntries[DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GR_CONFIG_IMAGE_SHOW)].pKeywordName,
                        (IFX_int_t)imageCnt);

                     responseBufferSize_byte -= (IFX_uint32_t)tmpLen;
                     pResponseBuffer         += tmpLen;
                     responceLen             += tmpLen;

                     if ( (retVal == IFX_SUCCESS) &&
                          (imageId < DTI_MAX_NUM_OF_IMAGES) )
                     {
                        if (pAgentCtx->imageCntrl[imageId].pData)
                        {
                           tmpLen = DTI_snprintf(pResponseBuffer, (IFX_int_t)responseBufferSize_byte,
                              "\tImage[%d]: useCnt = %3d, size = %d" DTI_CRLF,
                              imageId,
                              (IFX_int_t)pAgentCtx->imageCntrl[imageId].useCount,
                              (IFX_int_t)pAgentCtx->imageCntrl[imageId].imageSize);
                        }
                        else
                        {
                           tmpLen = DTI_snprintf(pResponseBuffer, (IFX_int_t)responseBufferSize_byte,
                              "\tImage[%d]: no data" DTI_CRLF,
                              imageId);
                        }
                        responseBufferSize_byte -= (IFX_uint32_t)tmpLen;
                        pResponseBuffer         += tmpLen;
                        responceLen             += tmpLen;
                     }
                     else
                     {
                        for (idx = 0; idx < DTI_MAX_NUM_OF_IMAGES; idx++)
                        {
                           if (pAgentCtx->imageCntrl[idx].pData)
                           {
                              tmpLen = DTI_snprintf(pResponseBuffer, (IFX_int_t)responseBufferSize_byte,
                                 "\tImage[%d]: useCnt = %3d, size = %d" DTI_CRLF,
                                 idx,
                                 (IFX_int_t)pAgentCtx->imageCntrl[idx].useCount,
                                 (IFX_int_t)pAgentCtx->imageCntrl[idx].imageSize);

                              responseBufferSize_byte -= (IFX_uint32_t)tmpLen;
                              pResponseBuffer         += tmpLen;
                              responceLen             += tmpLen;
                           }
                        }
                     }
                  }
                  break;

               case DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GR_CONFIG_IMAGE_RELEASE):
                  {
                     DTI_PacketError_t      packetError = DTI_eNoError;
                     DTI_H2D_ImageRelease_t imageRelease;

                     if ( (retVal == IFX_SUCCESS) &&
                          (imageId < DTI_MAX_NUM_OF_IMAGES) )
                     {
                        if (pAgentCtx->imageCntrl[imageId].pData)
                        {
                           imageRelease.imageNum = imageId;
                           retVal = DTI_imageRelease(
                                             &pWorkerCtx->dtiProtocolServerCtx,
                                             &imageRelease, &packetError);
                           if ( (retVal == IFX_SUCCESS) && (packetError == DTI_eNoError) )
                           {
                              responceLen = DTI_snprintf(pResponseBuffer, (IFX_int_t)responseBufferSize_byte,
                                 "%s nReturn=0 <%s %s %s> image[%d]: done" DTI_CRLF,
                                 (IFX_char_t *)DTI_CLI_CONTROL_PREFIX,
                                 DTI_cliControlGroupEntries[DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GROUP_CONFIG)].pKeywordName,
                                 DTI_cliControlGrConfigEntries[DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GR_CONFIG_IMAGE)].pKeywordName,
                                 DTI_cliControlConfigImageEntries[DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GR_CONFIG_IMAGE_RELEASE)].pKeywordName,
                                 (IFX_int_t)imageId);
                           }
                           else
                           {
                              responceLen = DTI_snprintf(pResponseBuffer, (IFX_int_t)responseBufferSize_byte,
                                 "%s nReturn=0 <%s %s %s> image[%d]: failed" DTI_CRLF,
                                 (IFX_char_t *)DTI_CLI_CONTROL_PREFIX,
                                 DTI_cliControlGroupEntries[DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GROUP_CONFIG)].pKeywordName,
                                 DTI_cliControlGrConfigEntries[DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GR_CONFIG_IMAGE)].pKeywordName,
                                 DTI_cliControlConfigImageEntries[DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GR_CONFIG_IMAGE_RELEASE)].pKeywordName,
                                 (IFX_int_t)imageId);
                           }
                        }
                        else
                        {
                           responceLen = DTI_snprintf(pResponseBuffer, (IFX_int_t)responseBufferSize_byte,
                              "%s nReturn=0 <%s %s %s> image[%d]: no data" DTI_CRLF,
                              (IFX_char_t *)DTI_CLI_CONTROL_PREFIX,
                              DTI_cliControlGroupEntries[DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GROUP_CONFIG)].pKeywordName,
                              DTI_cliControlGrConfigEntries[DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GR_CONFIG_IMAGE)].pKeywordName,
                              DTI_cliControlConfigImageEntries[DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GR_CONFIG_IMAGE_RELEASE)].pKeywordName,
                              (IFX_int_t)imageId);
                        }
                     }
                     else
                     {
                        responceLen = DTI_snprintf(pResponseBuffer, (IFX_int_t)responseBufferSize_byte,
                           "%s nReturn=0 <%s %s %s> invalid imageId %d" DTI_CRLF,
                           (IFX_char_t *)DTI_CLI_CONTROL_PREFIX,
                           DTI_cliControlGroupEntries[DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GROUP_CONFIG)].pKeywordName,
                           DTI_cliControlGrConfigEntries[DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GR_CONFIG_IMAGE)].pKeywordName,
                           DTI_cliControlConfigImageEntries[DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GR_CONFIG_IMAGE_RELEASE)].pKeywordName,
                           (IFX_int_t)imageId);
                     }
                  }
                  break;

               case DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GR_CONFIG_IMAGE_WRITE):
                  {
                     DTI_PacketError_t          packetError = DTI_eNoError;
                     DTI_H2D_ImageWriteToFile_t h2dImageWriteToFile;
                     DTI_D2H_ImageWriteToFile_t d2hImageWriteToFile;

                     if ( (retVal == IFX_SUCCESS) &&
                          (imageId < DTI_MAX_NUM_OF_IMAGES) )
                     {
                        if (pAgentCtx->imageCntrl[imageId].pData)
                        {
                           h2dImageWriteToFile.imageNum    = imageId;
                           h2dImageWriteToFile.cntrlOption = 0;


                           retVal = DTI_imageWriteToFile(
                                             &pWorkerCtx->dtiProtocolServerCtx,
                                             &h2dImageWriteToFile,
                                             pCurrArg,            /* remaining string is the filename */
                                             &d2hImageWriteToFile,
                                             &packetError);
                           if ( (retVal == IFX_SUCCESS) && (packetError == DTI_eNoError) )
                           {
                              responceLen = DTI_snprintf(pResponseBuffer, (IFX_int_t)responseBufferSize_byte,
                                 "%s nReturn=0 <%s %s %s> image[%d]: done" DTI_CRLF,
                                 (IFX_char_t *)DTI_CLI_CONTROL_PREFIX,
                                 DTI_cliControlGroupEntries[DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GROUP_CONFIG)].pKeywordName,
                                 DTI_cliControlGrConfigEntries[DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GR_CONFIG_IMAGE)].pKeywordName,
                                 DTI_cliControlConfigImageEntries[DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GR_CONFIG_IMAGE_WRITE)].pKeywordName,
                                 (IFX_int_t)imageId);
                           }
                           else
                           {
                              responceLen = DTI_snprintf(pResponseBuffer, (IFX_int_t)responseBufferSize_byte,
                                 "%s nReturn=0 <%s %s %s> image[%d]: failed" DTI_CRLF,
                                 (IFX_char_t *)DTI_CLI_CONTROL_PREFIX,
                                 DTI_cliControlGroupEntries[DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GROUP_CONFIG)].pKeywordName,
                                 DTI_cliControlGrConfigEntries[DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GR_CONFIG_IMAGE)].pKeywordName,
                                 DTI_cliControlConfigImageEntries[DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GR_CONFIG_IMAGE_WRITE)].pKeywordName,
                                 (IFX_int_t)imageId);
                           }


                        }
                        else
                        {
                           responceLen = DTI_snprintf(pResponseBuffer, (IFX_int_t)responseBufferSize_byte,
                              "%s nReturn=0 <%s %s %s> image[%d]: no data" DTI_CRLF,
                              (IFX_char_t *)DTI_CLI_CONTROL_PREFIX,
                              DTI_cliControlGroupEntries[DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GROUP_CONFIG)].pKeywordName,
                              DTI_cliControlGrConfigEntries[DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GR_CONFIG_IMAGE)].pKeywordName,
                              DTI_cliControlConfigImageEntries[DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GR_CONFIG_IMAGE_WRITE)].pKeywordName,
                              (IFX_int_t)imageId);
                        }
                     }
                     else
                     {
                        responceLen = DTI_snprintf(pResponseBuffer, (IFX_int_t)responseBufferSize_byte,
                           "%s nReturn=0 <%s %s %s> invalid imageId %d" DTI_CRLF,
                           (IFX_char_t *)DTI_CLI_CONTROL_PREFIX,
                           DTI_cliControlGroupEntries[DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GROUP_CONFIG)].pKeywordName,
                           DTI_cliControlGrConfigEntries[DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GR_CONFIG_IMAGE)].pKeywordName,
                           DTI_cliControlConfigImageEntries[DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GR_CONFIG_IMAGE_WRITE)].pKeywordName,
                           (IFX_int_t)imageId);
                     }
                  }
                  break;

               default:
                  responceLen = DTI_cliControlHelpPrint(
                                 IFX_NULL,
                                 "config image commands",
                                 DTI_cliControlConfigImageEntries,
                                 &pResponseBuffer, &responseBufferSize_byte);
            }
         }
         break;

      default:
         break;
   }

   return responceLen;
}

/**
   CLI Control - process device group
*/
DTI_STATIC IFX_int_t DTI_cliControlGrDeviceProcess(
                        DTI_AgentCtx_t          *pAgentCtx,
                        DTI_WorkerCtx_t         *pWorkerCtx,
                        const IFX_char_t        *pArgs,
                        IFX_uint_t              argLen,
                        IFX_int_t               commandNum,
                        IFX_char_t              *pResponseBuffer,
                        IFX_uint32_t            responseBufferSize_byte)
{
   IFX_int_t            responceLen = 0, devIdx;
   IFX_uint_t           lineNum = ~0, remain = argLen;
   DTI_PacketError_t    packetError;
   union {
      DTI_H2D_DeviceConfigGet_t  cfgGet;
      DTI_H2D_DeviceConfigSet_t  cfgSet;
   } in;

   union {
      DTI_D2H_DeviceConfigGet_t  cfgGet;
      DTI_D2H_DeviceConfigSet_t  cfgSet;
   } out;

   IFX_char_t        *pCurrArg = (IFX_char_t *)pArgs;
   DTI_DeviceCtx_t   *pDtiDevCtx         = IFX_NULL;


   /* get line */
   (void)DTI_cliControlGetNextDigitVal(&pCurrArg, &remain, &lineNum, 0);

   if ( lineNum == (IFX_uint_t) (~0))
   {
      return 0;
   }
   devIdx = (lineNum >> 24) & 0x000000FF;
   lineNum &= 0x00FFFFFF;

   if (devIdx < DTI_MAX_DEVICE_INTERFACES)
   {
      pDtiDevCtx = pWorkerCtx->pDtiDevCtx[devIdx];
   }

   if (pDtiDevCtx == IFX_NULL)
   {
      DTI_PRN_USR_DBG_NL(DTI_DBG, DTI_PRN_LEVEL_HIGH,
         ("CLI Control: Group[%02d <%s>] Opt[%02d <%s>] - <%s>."DTI_CRLF,
           DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GROUP_DEVICE),
           DTI_cliControlGroupEntries[DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GROUP_DEVICE)].pKeywordName,
           commandNum,
           DTI_cliControlGrDeviceEntries[commandNum].pKeywordName,
           pArgs ));

      responceLen = DTI_snprintf(pResponseBuffer, (IFX_int_t)responseBufferSize_byte,
         "%s nReturn=0 <%s %s> = <invalid device type>",
          (IFX_char_t *)DTI_CLI_CONTROL_PREFIX,
          DTI_cliControlGroupEntries[DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GROUP_DEVICE)].pKeywordName,
          DTI_cliControlGrDeviceEntries[commandNum].pKeywordName );

      return responceLen;
   }

   DTI_MemSet(&in,  0x00, sizeof(in));
   DTI_MemSet(&out, 0x00, sizeof(out));

   switch (commandNum)
   {
      case DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GR_DEV_CFGSIZE):
         {
            /* "@DTI# device cfgsize <lineNum> <0/1/2> (MBox / Reg / Dbg)" */
            IFX_uint_t key = ~0;

            (void)DTI_cliControlGetNextDigitVal(
                        &pCurrArg, &remain, &key, 0);

            if (key != (IFX_uint_t) (~0))
            {
               in.cfgGet.key = (IFX_uint32_t)key;
               packetError   = DTI_eNoError;

               (void)DTI_device_configGet(
                     pDtiDevCtx, &in.cfgGet, &out.cfgGet,
                     (IFX_int_t)lineNum, &packetError);
            }
            else
            {
               packetError  = DTI_eErrInvalidParameters;
            }

            if (packetError == DTI_eNoError)
            {
               DTI_PRN_USR_DBG_NL(DTI_DBG, DTI_PRN_LEVEL_HIGH,
                  ("CLI Control: %s <%s %s> - Args: <%s> - DONE."DTI_CRLF,
                    (IFX_char_t *)DTI_CLI_CONTROL_PREFIX,
                    DTI_cliControlGroupEntries[DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GROUP_DEVICE)].pKeywordName,
                    DTI_cliControlGrDeviceEntries[commandNum].pKeywordName,
                    pArgs ));

               responceLen = DTI_snprintf(pResponseBuffer, (IFX_int_t)responseBufferSize_byte,
                  "%s nReturn=0 <%s %s> line = %d, key = %d, value = %d",
                  (IFX_char_t *)DTI_CLI_CONTROL_PREFIX,
                  DTI_cliControlGroupEntries[DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GROUP_DEVICE)].pKeywordName,
                  DTI_cliControlGrDeviceEntries[commandNum].pKeywordName,
                  lineNum, out.cfgGet.key, out.cfgGet.value);
            }
            else
            {
               DTI_PRN_USR_ERR_NL(DTI_DBG, DTI_PRN_LEVEL_ERR,
                  ("CLI Control: %s <%s %s> - Args: <%s> - with errors."DTI_CRLF,
                    (IFX_char_t *)DTI_CLI_CONTROL_PREFIX,
                    DTI_cliControlGroupEntries[DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GROUP_DEVICE)].pKeywordName,
                    DTI_cliControlGrDeviceEntries[commandNum].pKeywordName,
                    pArgs ));

               responceLen = DTI_snprintf(pResponseBuffer, (IFX_int_t)responseBufferSize_byte,
                  "%s nReturn=-1 <%s %s> - Args: <%s>",
                  (IFX_char_t *)DTI_CLI_CONTROL_PREFIX,
                  DTI_cliControlGroupEntries[DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GROUP_DEVICE)].pKeywordName,
                  DTI_cliControlGrDeviceEntries[commandNum].pKeywordName,
                  pArgs);
            }
         }
         break;

      case DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GR_DEV_NFC):
         {
            /* "@DTI# device nfc <lineNum> [<0/1> (NFC OFF/ON)]" */
            IFX_uint_t value = ~0;

            (void)DTI_cliControlGetNextDigitVal(
                        &pCurrArg, (IFX_uint_t *)&remain, &value, 0);

            packetError   = DTI_eNoError;

            if (value == (IFX_uint_t) (~0))
            {
               in.cfgGet.key = (IFX_uint32_t)DTI_eAutonousMessages;

               (void)DTI_device_configGet(
                     pDtiDevCtx, &in.cfgGet, &out.cfgGet,
                     (IFX_int_t)lineNum, &packetError);
            }
            else
            {
               in.cfgSet.key   = (IFX_uint32_t)DTI_eAutonousMessages;
               in.cfgSet.value = (value == 0) ? 0 : 1;

               (void)DTI_device_configSet(
                     pDtiDevCtx, &in.cfgSet, &out.cfgSet,
                     (IFX_int_t)lineNum, &packetError);
            }

            if (packetError == DTI_eNoError)
            {
               DTI_PRN_USR_DBG_NL(DTI_DBG, DTI_PRN_LEVEL_HIGH,
                  ("CLI Control: %s <%s %s> - Args: <%s> - DONE."DTI_CRLF,
                    (IFX_char_t *)DTI_CLI_CONTROL_PREFIX,
                    DTI_cliControlGroupEntries[DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GROUP_DEVICE)].pKeywordName,
                    DTI_cliControlGrDeviceEntries[commandNum].pKeywordName,
                    pArgs ));

               if (value == (IFX_uint_t) (~0))
               {
                  responceLen = DTI_snprintf(pResponseBuffer, (IFX_int_t)responseBufferSize_byte,
                     "%s nReturn=0 <%s %s> Get: line = %d, key = %d, value = %d",
                     (IFX_char_t *)DTI_CLI_CONTROL_PREFIX,
                     DTI_cliControlGroupEntries[DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GROUP_DEVICE)].pKeywordName,
                     DTI_cliControlGrDeviceEntries[commandNum].pKeywordName,
                     lineNum, out.cfgGet.key, out.cfgGet.value);
               }
               else
               {
                  responceLen = DTI_snprintf(pResponseBuffer, (IFX_int_t)responseBufferSize_byte,
                     "%s nReturn=0 <%s %s> Set: line = %d, key = %d, value = %d",
                     (IFX_char_t *)DTI_CLI_CONTROL_PREFIX,
                     DTI_cliControlGroupEntries[DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GROUP_DEVICE)].pKeywordName,
                     DTI_cliControlGrDeviceEntries[commandNum].pKeywordName,
                     lineNum, out.cfgSet.key, out.cfgSet.value);
               }
            }
            else
            {
               DTI_PRN_USR_ERR_NL(DTI_DBG, DTI_PRN_LEVEL_ERR,
                  ("CLI Control: %s <%s %s> - Args: <%s> - with errors."DTI_CRLF,
                    (IFX_char_t *)DTI_CLI_CONTROL_PREFIX,
                    DTI_cliControlGroupEntries[DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GROUP_DEVICE)].pKeywordName,
                    DTI_cliControlGrDeviceEntries[commandNum].pKeywordName,
                    pArgs ));

               responceLen = DTI_snprintf(pResponseBuffer, (IFX_int_t)responseBufferSize_byte,
                  "%s nReturn=-1 <%s %s> - Args: <%s>",
                  (IFX_char_t *)DTI_CLI_CONTROL_PREFIX,
                  DTI_cliControlGroupEntries[DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GROUP_DEVICE)].pKeywordName,
                  DTI_cliControlGrDeviceEntries[commandNum].pKeywordName,
                  pArgs);
            }
         }
         break;

      case DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GR_DEV_LINE_CLOSE):
         {
            packetError   = DTI_eNoError;

            (void)DTI_device_devClose(pDtiDevCtx, (IFX_int_t)lineNum, &packetError);

            if (packetError == DTI_eNoError)
            {
               DTI_PRN_USR_DBG_NL(DTI_DBG, DTI_PRN_LEVEL_HIGH,
                  ("CLI Control: %s <%s %s> - Args: <%s> - DONE."DTI_CRLF,
                    (IFX_char_t *)DTI_CLI_CONTROL_PREFIX,
                    DTI_cliControlGroupEntries[DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GROUP_DEVICE)].pKeywordName,
                    DTI_cliControlGrDeviceEntries[commandNum].pKeywordName,
                    pArgs ));

               responceLen = DTI_snprintf(pResponseBuffer, (IFX_int_t)responseBufferSize_byte,
                  "%s nReturn=0 <%s %s> line = %d Line Close DONE",
                  (IFX_char_t *)DTI_CLI_CONTROL_PREFIX,
                  DTI_cliControlGroupEntries[DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GROUP_DEVICE)].pKeywordName,
                  DTI_cliControlGrDeviceEntries[commandNum].pKeywordName,
                  lineNum);
            }
            else
            {
               DTI_PRN_USR_ERR_NL(DTI_DBG, DTI_PRN_LEVEL_ERR,
                  ("CLI Control: %s <%s %s> - Args: <%s> - with errors."DTI_CRLF,
                    (IFX_char_t *)DTI_CLI_CONTROL_PREFIX,
                    DTI_cliControlGroupEntries[DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GROUP_DEVICE)].pKeywordName,
                    DTI_cliControlGrDeviceEntries[commandNum].pKeywordName,
                    pArgs ));

               responceLen = DTI_snprintf(pResponseBuffer, (IFX_int_t)responseBufferSize_byte,
                  "%s nReturn=-1 <%s %s> - Args: <%s>",
                  (IFX_char_t *)DTI_CLI_CONTROL_PREFIX,
                  DTI_cliControlGroupEntries[DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GROUP_DEVICE)].pKeywordName,
                  DTI_cliControlGrDeviceEntries[commandNum].pKeywordName,
                  pArgs);
            }
         }
         break;

      case DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GR_DEV_LINE_OPEN):
         {
            packetError   = DTI_eNoError;

            (void)DTI_device_devOpen(pDtiDevCtx, (IFX_int_t)lineNum, &packetError);

            if (packetError == DTI_eNoError)
            {
               DTI_PRN_USR_DBG_NL(DTI_DBG, DTI_PRN_LEVEL_HIGH,
                  ("CLI Control: %s <%s %s> - Args: <%s> - DONE."DTI_CRLF,
                    (IFX_char_t *)DTI_CLI_CONTROL_PREFIX,
                    DTI_cliControlGroupEntries[DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GROUP_DEVICE)].pKeywordName,
                    DTI_cliControlGrDeviceEntries[commandNum].pKeywordName,
                    pArgs ));

               responceLen = DTI_snprintf(pResponseBuffer, (IFX_int_t)responseBufferSize_byte,
                  "%s nReturn=0 <%s %s> line = %d Line Open DONE",
                  (IFX_char_t *)DTI_CLI_CONTROL_PREFIX,
                  DTI_cliControlGroupEntries[DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GROUP_DEVICE)].pKeywordName,
                  DTI_cliControlGrDeviceEntries[commandNum].pKeywordName,
                  lineNum);
            }
            else
            {
               DTI_PRN_USR_ERR_NL(DTI_DBG, DTI_PRN_LEVEL_ERR,
                  ("CLI Control: %s <%s %s> - Args: <%s> - with errors."DTI_CRLF,
                    (IFX_char_t *)DTI_CLI_CONTROL_PREFIX,
                    DTI_cliControlGroupEntries[DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GROUP_DEVICE)].pKeywordName,
                    DTI_cliControlGrDeviceEntries[commandNum].pKeywordName,
                    pArgs ));

               responceLen = DTI_snprintf(pResponseBuffer, (IFX_int_t)responseBufferSize_byte,
                  "%s nReturn=-1 <%s %s> - Args: <%s>",
                  (IFX_char_t *)DTI_CLI_CONTROL_PREFIX,
                  DTI_cliControlGroupEntries[DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GROUP_DEVICE)].pKeywordName,
                  DTI_cliControlGrDeviceEntries[commandNum].pKeywordName,
                  pArgs);
            }
         }
         break;

      default:
         DTI_PRN_USR_DBG_NL(DTI_DBG, DTI_PRN_LEVEL_HIGH,
            ("CLI Control: Group[%02d <%s>] Opt[%02d <%s>] - <%s>."DTI_CRLF,
              DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GROUP_DEVICE),
              DTI_cliControlGroupEntries[DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GROUP_DEVICE)].pKeywordName,
              commandNum,
              DTI_cliControlGrDeviceEntries[commandNum].pKeywordName,
              pArgs ));

         responceLen = DTI_snprintf(pResponseBuffer, (IFX_int_t)responseBufferSize_byte,
            "%s nReturn=0 <%s %s = <not supported>",
             (IFX_char_t *)DTI_CLI_CONTROL_PREFIX,
             DTI_cliControlGroupEntries[DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GROUP_DEVICE)].pKeywordName,
             DTI_cliControlGrDeviceEntries[commandNum].pKeywordName );

         break;
   }

   return responceLen;
}


/**
   Process a CLI control string.
*/
DTI_STATIC IFX_int_t DTI_cliControlProcess(
                        DTI_AgentCtx_t          *pAgentCtx,
                        DTI_WorkerCtx_t         *pWorkerCtx,
                        const DTI_Packet_t      *pDtiPacketIn,
                        DTI_Packet_t            *pDtiPacketOut,
                        IFX_uint32_t            bufferOutSize)
{
   IFX_int_t         cliCntrlGroup = -1, cliCntrlOption = -1, responceLen = 0;
   IFX_uint_t        i, remainLen;
   const IFX_char_t  *pInCliControlStrCurr;
   DTI_cliConfigKeyword_t *pKeyWords = IFX_NULL;

   remainLen            = pDtiPacketIn->header.payloadSize - (IFX_uint32_t)DTI_StrLen(DTI_CLI_CONTROL_PREFIX);
   /*lint -e{416} */
   pInCliControlStrCurr = (const IFX_char_t *)&pDtiPacketIn->payload[DTI_StrLen(DTI_CLI_CONTROL_PREFIX)];


   /*
      DTI Control Command:
         "@DTI# <group> <command> [<command params>]"

      <group>
         - config : dgblevel
         - device : cfgsize, line open / close, nfc
         - cli    : buffer size, event handling, ...
   */

   /* search the group */
   i = DTI_cliControlSearchKey(
                        DTI_cliControlGroupEntries,
                        &pInCliControlStrCurr, &remainLen);

   if (DTI_cliControlGroupEntries[i].pKeywordName == IFX_NULL)
   {
      return 0;
   }

   /* group found */
   cliCntrlGroup = (IFX_int_t)i;

   switch (cliCntrlGroup)
   {
      case DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GROUP_CONFIG):
         pKeyWords = DTI_cliControlGrConfigEntries;
         break;

      case DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GROUP_DEVICE):
         pKeyWords = DTI_cliControlGrDeviceEntries;
         break;

      case DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GROUP_CLI):
         pKeyWords = DTI_cliControlGrCliEntries;
         break;

      case DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GROUP_HELP):
         /* search again within the group for specific help */
         pKeyWords = DTI_cliControlGroupEntries;
         break;

      default:
         return 0;
   }

   /* search the keyword (command) */
   i = DTI_cliControlSearchKey(
                        pKeyWords,
                        &pInCliControlStrCurr, &remainLen);

   if (cliCntrlGroup == DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GROUP_HELP) )
   {
      if ((remainLen > 0) && (*pInCliControlStrCurr != '\0') )
      {
         /* remaining infos available --> no help request */
         return 0;
      }

      responceLen = DTI_cliControlGrConfigHelp(
                           pAgentCtx, pWorkerCtx,
                           (IFX_int_t)i,
                           (IFX_char_t *)pDtiPacketOut->payload,
                           bufferOutSize - sizeof(DTI_PacketHeader_t));
   }
   else
   {
      if (pKeyWords[i].pKeywordName == IFX_NULL)
      {
         /* no control command key */
         return 0;
      }

      /* option found */
      cliCntrlOption = (IFX_int_t)i;

      switch (cliCntrlGroup)
      {
         case DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GROUP_CONFIG):
            responceLen = DTI_cliControlGrConfigProcess(
                              pAgentCtx, pWorkerCtx,
                              pInCliControlStrCurr, remainLen, cliCntrlOption,
                              (IFX_char_t *)pDtiPacketOut->payload,
                              bufferOutSize - sizeof(DTI_PacketHeader_t));
            break;

         case DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GROUP_DEVICE):
            responceLen = DTI_cliControlGrDeviceProcess(
                              pAgentCtx, pWorkerCtx,
                              pInCliControlStrCurr, remainLen, cliCntrlOption,
                              (IFX_char_t *)pDtiPacketOut->payload,
                              bufferOutSize - sizeof(DTI_PacketHeader_t));
            break;

         case DTI_CLI_CNTRL_NUM(DTI_CLI_CNTRL_GROUP_CLI):
            responceLen = DTI_CLI_cliControlGrCliProcess(
                              pAgentCtx, pWorkerCtx->workerNum,
                              pInCliControlStrCurr, remainLen, cliCntrlOption,
                              (IFX_char_t *)pDtiPacketOut->payload,
                              bufferOutSize - sizeof(DTI_PacketHeader_t));
            break;

         default:
            responceLen = 0;
            break;
      }
   }

   if (responceLen > 0)
   {
      (void)DTI_packetResponceSet(
               pDtiPacketIn, pDtiPacketOut,
               DTI_eNoError,
               (IFX_uint32_t)responceLen, bufferOutSize, IFX_FALSE);
   }

   return responceLen;
}


/**
   Checks an incoming CLI string if this is a internal control command.

\remark
   A internal CLI control string starts with "@DTI# "

*/
DTI_STATIC IFX_int_t DTI_packetHandler_cliControl(
                        DTI_AgentCtx_t          *pAgentCtx,
                        DTI_WorkerCtx_t         *pWorkerCtx,
                        DTI_Packet_t            *pDtiPacketIn,
                        IFX_uint_t              dtiBufInLen,
                        DTI_Packet_t            *pDtiPacketOut,
                        IFX_uint_t              dtiBufOutLen)
{
   IFX_int_t            cliControlDone = 0;
   IFX_uint_t           bufOutLen = 0;
   const DTI_Packet_t   *pPacketIn;
   DTI_Packet_t         *pPacketOut = IFX_NULL;
   DTI_PTR_U            uPayload;
   const IFX_char_t     *pCliControlKey = DTI_CLI_CONTROL_PREFIX;

   if ((pWorkerCtx == IFX_NULL) || (pAgentCtx == IFX_NULL))
   {
      DTI_PRN_USR_ERR_NL(DTI_DBG, DTI_PRN_LEVEL_ERR,
         ("Error: Packet Handler CliControl - NULL ptr args."DTI_CRLF));

      return IFX_ERROR;
   }

   if (pDtiPacketIn)
   {
      pPacketIn  = pDtiPacketIn;
   }
   else
   {
      uPayload.pUInt8 = (IFX_uint8_t *)pWorkerCtx->dtiProtocolServerCtx.packetIn.buffer;
      pPacketIn       = (DTI_Packet_t *)DTI_PTR_CAST_GET_ULONG(uPayload);
      dtiBufInLen     = sizeof(pWorkerCtx->dtiProtocolServerCtx.packetIn.buffer);

      if (pPacketIn == IFX_NULL)
      {
         DTI_PRN_USR_ERR_NL(DTI_DBG, DTI_PRN_LEVEL_ERR,
            ("Error: Packet Handler CliControl - IN packet missaligned."DTI_CRLF));

         return IFX_ERROR;
      }
   }

   if (pPacketIn->header.magic != DTI_MAGIC)
   {
      DTI_PRN_USR_ERR_NL(DTI_DBG, DTI_PRN_LEVEL_ERR,
         ("Error: Packet Handler CliControl - invalid IN packet."DTI_CRLF));

      return IFX_ERROR;
   }

   switch(pPacketIn->header.packetType)
   {
      case DTI_PacketType_eCliString:
         if (    ((IFX_char_t)pPacketIn->payload[0] == pCliControlKey[0])
              && (pPacketIn->header.payloadSize > (IFX_uint32_t)DTI_StrLen(pCliControlKey) ) )
         {
            if (DTI_StrNCmp(
                     (const IFX_char_t *)pPacketIn->payload,
                     pCliControlKey,
                     DTI_StrLen(pCliControlKey)) == 0)
            {
               /* "@DTI# ..." found process, setup and send response */
               if (pDtiPacketOut)
               {
                  pPacketOut = pDtiPacketOut;
                  bufOutLen  = dtiBufOutLen;
               }
               else
               {
                  uPayload.pUInt8 = (IFX_uint8_t *)pWorkerCtx->dtiProtocolServerCtx.packetOut.buffer;
                  pPacketOut      = (DTI_Packet_t *)DTI_PTR_CAST_GET_ULONG(uPayload);
                  bufOutLen       = sizeof(pWorkerCtx->dtiProtocolServerCtx.packetOut.buffer);
               }

               DTI_CONN_PACKET_IN_CLI_CNTRL_INC(pAgentCtx, pWorkerCtx->workerNum);
               /* process a "@DTI# ..." control command */
               cliControlDone = DTI_cliControlProcess(
                                       pAgentCtx, pWorkerCtx,
                                       pPacketIn, pPacketOut, bufOutLen);
            }
         }
         break;

      default:
         break;
   }

   if ( (cliControlDone > 0) && (pPacketOut != IFX_NULL) )
   {
      DTI_packetShow (
         pPacketOut, IFX_TRUE, IFX_FALSE, "CLI Control Send",
         (pPacketOut->header.error == DTI_eNoError) ? IFXOS_PRN_LEVEL_LOW : DTI_PRN_LEVEL_HIGH);

      if (DTI_packetSend(&pWorkerCtx->dtiProtocolServerCtx.dtiCon, pPacketOut) != IFX_SUCCESS)
      {
         DTI_CONN_PACKET_OUT_ERR_INC(pAgentCtx, pWorkerCtx->workerNum);
         DTI_PRN_USR_ERR_NL(DTI_DBG, DTI_PRN_LEVEL_ERR,
            ("Error: Packet Handler CliControl - send packet."DTI_CRLF));

         return IFX_ERROR;
      }
      DTI_CONN_PACKET_OUT_CLI_CNTRL_INC(pAgentCtx, pWorkerCtx->workerNum);
      DTI_CONN_PACKET_OUT_INC(pAgentCtx, pWorkerCtx->workerNum);
   }

   return cliControlDone;
}

/**
   Call the corresponding packet handler depending on the group.

*/
DTI_STATIC IFX_int_t DTI_packetGroupHandler(
                        DTI_AgentCtx_t          *pAgentCtx,
                        DTI_WorkerCtx_t         *pWorkerCtx,
                        DTI_Packet_t            *pPacketIn,
                        IFX_uint_t              dtiBufInLen,
                        DTI_Packet_t            *pPacketOut,
                        IFX_uint_t              dtiBufOutLen)
{
   IFX_int_t         retVal = IFX_SUCCESS, devIdx;
   IFX_uint32_t      groupPacketType;
   DTI_DeviceCtx_t   *pDtiDevCtx = IFX_NULL;

   if ( (pAgentCtx == IFX_NULL) || (pWorkerCtx == IFX_NULL) || (pPacketIn == IFX_NULL))
   {
      DTI_PRN_USR_ERR_NL(DTI_DBG, DTI_PRN_LEVEL_ERR,
         ("Error: packet handler - NULL ptr arg."DTI_CRLF));

      return IFX_ERROR;
   }

   groupPacketType = pPacketIn->header.packetType & DTI_PACKET_GROUP_MASK;

   switch (groupPacketType)
   {
   case DTI_GROUP_CONTROL_ACCESS:
         DTI_CONN_PACKET_IN_CNTRL_INC(pAgentCtx, pWorkerCtx->workerNum);
         if ((retVal = DTI_packetHandler_Standard(
                           &pWorkerCtx->dtiProtocolServerCtx,
                           pPacketIn, pPacketOut, dtiBufOutLen)) == IFX_SUCCESS)
         {
            DTI_CONN_PACKET_OUT_CNTRL_INC(pAgentCtx, pWorkerCtx->workerNum);
            DTI_CONN_PACKET_OUT_INC(pAgentCtx, pWorkerCtx->workerNum);
         }
         else
         {
            DTI_CONN_PACKET_OUT_DISC_INC(pAgentCtx, pWorkerCtx->workerNum);
         }
         break;

      case DTI_GROUP_LOW_LEVEL_HW_ACCESS:
      case DTI_GROUP_CNTRL_MSG_ACCESS:
      case DTI_GROUP_TRACE_BUFFER_ACCESS:
      case DTI_GROUP_DEBUG_REG_ACCESS:
      case DTI_GROUP_WINEASY_ACCESS:
         {
            DTI_CONN_PACKET_IN_DEV_INC(pAgentCtx, pWorkerCtx->workerNum);
            devIdx = DTI_HDR_PORT_DEV_TYPE_NUM_GET(pPacketIn->header.port);
            if (devIdx < DTI_MAX_DEVICE_INTERFACES)
            {
               pDtiDevCtx = pWorkerCtx->pDtiDevCtx[devIdx];

               if ((retVal = DTI_packetHandler_device(
                              pDtiDevCtx,
                              &pWorkerCtx->dtiProtocolServerCtx,
                              pPacketIn, pPacketOut, dtiBufOutLen)) == IFX_SUCCESS)
               {
                  DTI_CONN_PACKET_OUT_DEV_INC(pAgentCtx, pWorkerCtx->workerNum);
                  DTI_CONN_PACKET_OUT_INC(pAgentCtx, pWorkerCtx->workerNum);
               }
               else
               {
                  DTI_CONN_PACKET_OUT_DISC_INC(pAgentCtx, pWorkerCtx->workerNum);
               }
            }
            else
            {
               if ( (retVal = DTI_packetRecvErrorHandler(
                              pAgentCtx, &pWorkerCtx->dtiProtocolServerCtx,
                              pPacketIn, DTI_eErrDeviceTypeOutOfRange)) != IFX_SUCCESS)
               {
                  DTI_PRN_USR_ERR_NL(DTI_DBG, DTI_PRN_LEVEL_ERR,
                     ("Error: Packet Handler - send error packet (devType)."DTI_CRLF));
               }
            }
         }
         break;

      case DTI_GROUP_CLI_ACCESS:
         /* check for a control CLI command
            Prefix: "@DTI# ..."
         */
         if (DTI_packetHandler_cliControl(
                     pAgentCtx, pWorkerCtx,
                     pPacketIn, dtiBufInLen, pPacketOut, dtiBufOutLen) == 0)
         {
#if defined(INCLUDE_CLI_SUPPORT) && (INCLUDE_CLI_SUPPORT == 1)
            DTI_CONN_PACKET_IN_CLI_INC(pAgentCtx, pWorkerCtx->workerNum);
            if ((retVal = DTI_packetHandler_cli(
                           pAgentCtx,
                           &pWorkerCtx->dtiProtocolServerCtx,
                           pWorkerCtx->workerNum,
                           pPacketIn, dtiBufInLen, pPacketOut, dtiBufOutLen)) == IFX_SUCCESS)
            {
               DTI_CONN_PACKET_OUT_CLI_INC(pAgentCtx, pWorkerCtx->workerNum);
               DTI_CONN_PACKET_OUT_INC(pAgentCtx, pWorkerCtx->workerNum);
            }
            else
            {
               DTI_CONN_PACKET_OUT_DISC_INC(pAgentCtx, pWorkerCtx->workerNum);
            }

#else
            DTI_CONN_PACKET_IN_UNKNOWN_INC(pAgentCtx, pWorkerCtx->workerNum);
            /* call standard handler to generate and send a unknown responce */
            if ((retVal = DTI_packetHandler_Standard(
                           &pWorkerCtx->dtiProtocolServerCtx,
                           pPacketIn, pPacketOut, dtiBufOutLen)) == IFX_SUCCESS)
            {
               DTI_CONN_PACKET_OUT_UNKNOWN_INC(pAgentCtx, pWorkerCtx->workerNum);
               DTI_CONN_PACKET_OUT_INC(pAgentCtx, pWorkerCtx->workerNum);
            }
            else
            {
               DTI_CONN_PACKET_OUT_DISC_INC(pAgentCtx, pWorkerCtx->workerNum);
            }
#endif
         }
         break;


      default:
         /* call standard handler to generate and send a unknown responce */
         DTI_CONN_PACKET_IN_UNKNOWN_INC(pAgentCtx, pWorkerCtx->workerNum);
         if ((retVal = DTI_packetHandler_Standard(
                           &pWorkerCtx->dtiProtocolServerCtx,
                           pPacketIn, pPacketOut, dtiBufOutLen)) == IFX_SUCCESS)
         {
            DTI_CONN_PACKET_OUT_UNKNOWN_INC(pAgentCtx, pWorkerCtx->workerNum);
            DTI_CONN_PACKET_OUT_INC(pAgentCtx, pWorkerCtx->workerNum);
         }
         else
         {
            DTI_CONN_PACKET_OUT_DISC_INC(pAgentCtx, pWorkerCtx->workerNum);
         }
         break;
   }

   return retVal;
}

/*
   Allocate / init a worker context data struct.
*/
DTI_STATIC IFX_int_t DTI_WorkerContextInit(
                        DTI_AgentCtx_t          *pAgentCtx,
                        DTI_Connection_t        *pNewDtiCon,
                        IFX_int_t               workerNum,
                        DTI_WorkerCtx_t         **ppWorkerCtx)
{
   IFX_boolean_t           bNewCtx = IFX_FALSE;
   DTI_WorkerCtx_t         *pWorkerCtx = IFX_NULL;
   DTI_ProtocolServerCtx_t *pProtocolServerCtx = IFX_NULL;
   DTI_PTR_U       uPtr;

   if ( (pAgentCtx == IFX_NULL) || (ppWorkerCtx == IFX_NULL) )
   {
      DTI_PRN_USR_ERR_NL(DTI_DBG, DTI_PRN_LEVEL_ERR,
         ("ERROR: worker context init - NULL ptr args."DTI_CRLF));

      return IFX_ERROR;
   }

   pWorkerCtx = *ppWorkerCtx;

   if (pWorkerCtx == IFX_NULL)
   {
      bNewCtx = IFX_TRUE;

      uPtr.pUInt8 = (IFX_uint8_t *)DTI_Malloc(sizeof(DTI_WorkerCtx_t));
      if (uPtr.pUInt8 == IFX_NULL)
      {
         DTI_PRN_USR_ERR_NL(DTI_DBG, DTI_PRN_LEVEL_ERR,
               ("ERROR: worker context init - ctx no memory."DTI_CRLF));

         goto DTI_WORKER_CONTEXT_INIT_ERROR;
      }
      DTI_MemSet(uPtr.pUInt8, 0x00, sizeof(DTI_WorkerCtx_t));

      if ( (pWorkerCtx = (DTI_WorkerCtx_t *)DTI_PTR_CAST_GET_ULONG(uPtr)) == IFX_NULL)
      {
         DTI_PRN_USR_ERR_NL(DTI_DBG, DTI_PRN_LEVEL_ERR,
               ("ERROR: worker context init - workerCtx missaligned."DTI_CRLF));

         DTI_Free(uPtr.pUInt8);
         goto DTI_WORKER_CONTEXT_INIT_ERROR;
      }
   }

   /*
      Init DTI protocol part
   */
   uPtr.pUInt8        = (IFX_uint8_t *)&pWorkerCtx->dtiProtocolServerCtx;
   pProtocolServerCtx = (DTI_ProtocolServerCtx_t *)DTI_PTR_CAST_GET_ULONG(uPtr);
   if (pProtocolServerCtx == IFX_NULL)
   {
      DTI_PRN_USR_ERR_NL(DTI_DBG, DTI_PRN_LEVEL_ERR,
            ("ERROR: worker context init - DTI ProtServerCtx missaligned."DTI_CRLF));

      goto DTI_WORKER_CONTEXT_INIT_ERROR;
   }

   uPtr.pUInt8                           = (IFX_uint8_t *)pProtocolServerCtx->packetIn.buffer;
   pProtocolServerCtx->packetIn.pPacket  = (DTI_Packet_t *)DTI_PTR_CAST_GET_ULONG(uPtr);

   uPtr.pUInt8                           = (IFX_uint8_t *)pProtocolServerCtx->packetOut.buffer;
   pProtocolServerCtx->packetOut.pPacket = (DTI_Packet_t *)DTI_PTR_CAST_GET_ULONG(uPtr);

   if ( (pProtocolServerCtx->packetIn.pPacket  == IFX_NULL) ||
        (pProtocolServerCtx->packetOut.pPacket == IFX_NULL))
   {
      DTI_PRN_USR_ERR_NL(DTI_DBG, DTI_PRN_LEVEL_ERR,
            ("ERROR: worker context init - IN/OUT buffer missaligned."DTI_CRLF));

      goto DTI_WORKER_CONTEXT_INIT_ERROR;
   }

   /*
      setup callbacks
   */
   pProtocolServerCtx->pCbCtxAgent          = (IFX_void_t *)pAgentCtx;
   pProtocolServerCtx->pCbFctImageCntrlGet  = DTI_imageCntrlGetFunction;
   pProtocolServerCtx->pCbFctImageCntrlLock = DTI_imageCntrlLockFunction;

#if    defined(INCLUDE_CLI_SUPPORT) && (INCLUDE_CLI_SUPPORT == 1) \
    && defined(DTI_CLI_EVENT_SUPPORT) && (DTI_CLI_EVENT_SUPPORT == 1)
   if (bNewCtx)
   {
      DTI_MemSet(&pWorkerCtx->cliEventCtx, 0x00, sizeof(DTI_CliEventCtx_t));
      DTI_LockInit(&pWorkerCtx->cliEventCtx.eventFifoLock);
   }
#endif

   /*
      setup new connection
   */
   DTI_conStructInit(&pProtocolServerCtx->dtiCon);
   DTI_conCntrlStructInit(&pProtocolServerCtx->dtiConCntrl);
   if (pNewDtiCon != IFX_NULL)
   {
      DTI_conConSet(&pProtocolServerCtx->dtiCon, pNewDtiCon);
      DTI_conCntrlConSet( &pProtocolServerCtx->dtiCon,
                          &pProtocolServerCtx->dtiConCntrl);
      DTI_conAddForRecvWait( &pProtocolServerCtx->dtiCon,
                             &pProtocolServerCtx->dtiConCntrl);
   }

   /*
      setup device module part (todo)
   */


   /*
      init done
   */

   pWorkerCtx->workerNum = workerNum;
   *ppWorkerCtx          = pWorkerCtx;

   return IFX_SUCCESS;

DTI_WORKER_CONTEXT_INIT_ERROR:

   if (pNewDtiCon != IFX_NULL)
   {
      if (pWorkerCtx != IFX_NULL)
      {
         DTI_Free(pWorkerCtx);
      }
   }

   *ppWorkerCtx = IFX_NULL;

   return IFX_ERROR;
}

/*
   Allocate / init a worker context data struct.
*/
DTI_STATIC IFX_int_t DTI_WorkerContextRelease(
                        DTI_AgentCtx_t          *pAgentCtx,
                        DTI_WorkerCtx_t         *pWorkerCtx)
{
   IFX_int_t devIdx;
   DTI_DeviceCtx_t         *pDtiDevCtx         = IFX_NULL;
   DTI_deviceInterface_t   *pCurrDevInterface  = IFX_NULL;
   DTI_DeviceAccessFct_t   *pDeviceAccessFct   = IFX_NULL;

   if (pAgentCtx == IFX_NULL)
   {
      DTI_PRN_USR_ERR_NL(DTI_DBG, DTI_PRN_LEVEL_ERR,
         ("Error: worker context release - NULL ptr arg."DTI_CRLF));

      return IFX_ERROR;
   }

   if (pWorkerCtx != IFX_NULL)
   {
      (void)DTI_conClose(&pWorkerCtx->dtiProtocolServerCtx.dtiCon);

      (void)DTI_CLI_workerEventCtxDelete(
            pAgentCtx, pWorkerCtx->workerNum);

      for (devIdx = 0; devIdx < DTI_MAX_DEVICE_INTERFACES; devIdx++)
      {
         pCurrDevInterface = pWorkerCtx->pUsedDevInterfaces[devIdx];
         pDtiDevCtx        = pWorkerCtx->pDtiDevCtx[devIdx];
         pWorkerCtx->pDtiDevCtx[devIdx] = IFX_NULL;
         if (pDtiDevCtx != IFX_NULL)
         {
            pDeviceAccessFct = pDtiDevCtx->pDeviceAccessFct;
            pDtiDevCtx->pDeviceAccessFct = IFX_NULL;
            (void)DTI_device_moduleStop(
                     pDeviceAccessFct,
                     &pCurrDevInterface[devIdx].deviceSysInfo,
                     &pDtiDevCtx);
         }
      }

      for (devIdx = 0; devIdx < DTI_MAX_DEVICE_INTERFACES; devIdx++)
      {
         pCurrDevInterface = pWorkerCtx->pUsedDevInterfaces[devIdx];
         pWorkerCtx->pUsedDevInterfaces[devIdx] = IFX_NULL;
         if (pCurrDevInterface != IFX_NULL)
         {
            DTI_Free(pCurrDevInterface);
         }
      }
   }

   return IFX_SUCCESS;
}

/**
   DTI worker function
*/
DTI_STATIC IFX_int_t DTI_Worker(
                        DTI_AgentCtx_t          *pAgentCtx,
                        DTI_WorkerCtx_t         *pWorkerCtx,
                        volatile IFX_boolean_t  *pBShutdown)
{
   IFX_int_t               nIfxRet = IFX_SUCCESS, recvBytes, devIdx;
   IFX_int_t               devSelectWait_ms  = 10, sockSelectWait_ms = DTI_WORKER_THREAD_WAIT_MS;
   IFX_uint_t              msgIdleCount = 0, thrSleep_ms = 0;
   IFX_boolean_t           bConClosed = IFX_FALSE;
   IFX_boolean_t           bAutoDevMsgActive = IFX_FALSE, bControlAutoCliMsgSupport = IFX_FALSE;
   DTI_PacketError_t       packetError         = DTI_eNoError;
   DTI_ProtocolServerCtx_t *pProtocolServerCtx = IFX_NULL;
   DTI_DeviceCtx_t         *pDtiDevCtx         = IFX_NULL;
   DTI_deviceInterface_t   *pCurrDevInterface  = IFX_NULL;
   DTI_DeviceAccessFct_t   *pDeviceAccessFct   = IFX_NULL;

   if ( (pAgentCtx == IFX_NULL) || (pWorkerCtx == IFX_NULL) || (pBShutdown == IFX_NULL) )
   {
      DTI_PRN_USR_ERR_NL(DTI_DBG, DTI_PRN_LEVEL_ERR,
         ("Error: worker thr - NULL ptr arg."DTI_CRLF));

      nIfxRet = IFX_ERROR;
      goto DTI_WORKER_THREAD_EXIT;
   }

   DTI_WORKER_THR_START_REQ_INC(pAgentCtx, pWorkerCtx->workerNum);
   DTI_PRN_USR_DBG_NL(DTI_DBG, DTI_PRN_LEVEL_HIGH,
      ("worker thr[%d] - enter."DTI_CRLF, pWorkerCtx->workerNum));

   pProtocolServerCtx = &pWorkerCtx->dtiProtocolServerCtx;

   /**
      Lock: get corresponding interface from the agent context,
            --> protect agains device interface changes.
   */
   if (DTI_LockGet(&pAgentCtx->dataLock) != IFX_SUCCESS)
   {
      DTI_WORKER_THR_START_FAIL_INC(pAgentCtx, pWorkerCtx->workerNum);
      DTI_PRN_USR_ERR_NL(DTI_DBG, DTI_PRN_LEVEL_ERR,
         ("ERROR: worker thr[%d] - lock interface setup."DTI_CRLF, pWorkerCtx->workerNum));

      return IFX_ERROR;
   }
   for (devIdx = 0; devIdx < DTI_MAX_DEVICE_INTERFACES; devIdx++)
   {
      if ( (pAgentCtx->deviceInterface[devIdx].bConfigured == IFX_TRUE) &&
           (pAgentCtx->deviceInterface[devIdx].pDeviceAccessFct != IFX_NULL) )
      {
         pCurrDevInterface = DTI_Malloc(sizeof(DTI_deviceInterface_t));
         if (pCurrDevInterface == IFX_NULL)
         {
            DTI_PRN_USR_ERR_NL(DTI_DBG, DTI_PRN_LEVEL_ERR,
               ("Error: worker thr[%d] - alloc devIF."DTI_CRLF, pWorkerCtx->workerNum));

            nIfxRet = IFX_ERROR;
         }
         else
         {
            DTI_MemCpy(pCurrDevInterface, &pAgentCtx->deviceInterface[devIdx],
                                   sizeof(DTI_deviceInterface_t));

            pDeviceAccessFct = pCurrDevInterface->pDeviceAccessFct;
            if (DTI_device_moduleStart(
                             pDeviceAccessFct,
                             &pCurrDevInterface->deviceSysInfo,
                             &pDtiDevCtx) != IFX_SUCCESS)
            {
               DTI_PRN_USR_ERR_NL(DTI_DBG, DTI_PRN_LEVEL_ERR,
                  ("Error: worker thr[%d] - device module <%s> init."DTI_CRLF,
                  pWorkerCtx->workerNum, pCurrDevInterface->ifName));

               nIfxRet = IFX_ERROR;
            }
            else
            {
               pDtiDevCtx->devIfNum           = devIdx;
               pDtiDevCtx->pDeviceAccessFct   = pDeviceAccessFct;
               pWorkerCtx->pUsedDevInterfaces[devIdx] = pCurrDevInterface;
               pWorkerCtx->pDtiDevCtx[devIdx]         = pDtiDevCtx;
            }
         }
      }
   }
   DTI_LockRelease(&pAgentCtx->dataLock);

   if (nIfxRet != IFX_SUCCESS)
   {
      DTI_WORKER_THR_START_FAIL_INC(pAgentCtx, pWorkerCtx->workerNum);
      DTI_PRN_USR_ERR_NL(DTI_DBG, DTI_PRN_LEVEL_ERR,
         ("Error: worker thr[%d] - while device module init."DTI_CRLF, pWorkerCtx->workerNum));

      goto DTI_WORKER_THREAD_EXIT;
   }

   DTI_WORKER_THR_START_DONE_INC(pAgentCtx, pWorkerCtx->workerNum);
   recvBytes = 0;
   pProtocolServerCtx->packetIn.pPacket = IFX_NULL;

   /*
      thread loop
   */
   while( (pAgentCtx->bListenRun == IFX_TRUE) && (*pBShutdown == 0) && (bConClosed == IFX_FALSE) )
   {

      nIfxRet = DTI_packetRead (
                        &pProtocolServerCtx->dtiCon,
                        &pProtocolServerCtx->dtiConCntrl,
                        &pProtocolServerCtx->bResync,
                        sockSelectWait_ms,
                        sizeof(pProtocolServerCtx->packetIn.buffer),
                        pProtocolServerCtx->packetIn.buffer,
                        &recvBytes, &packetError, &pProtocolServerCtx->packetIn.pPacket);

      switch(nIfxRet)
      {
      case DTI_CON_RECV_STATUS_DONE:
            DTI_CONN_PACKET_IN_INC(pAgentCtx, pWorkerCtx->workerNum);
            if (packetError == DTI_eNoError)
            {
               /* success packet handling */
               if (DTI_packetGroupHandler(
                     pAgentCtx, pWorkerCtx,
                     pProtocolServerCtx->packetIn.pPacket,
                     sizeof(pProtocolServerCtx->packetIn.buffer),
                     IFX_NULL, 0) != IFX_SUCCESS)
               {
                  DTI_PRN_USR_ERR_NL(DTI_DBG, DTI_PRN_LEVEL_ERR,
                     ("Error: worker thr - EXIT on send error."DTI_CRLF));
               }
            }
            else
            {
               if (DTI_packetRecvErrorHandler(
                     pAgentCtx, pProtocolServerCtx,
                     pProtocolServerCtx->packetIn.pPacket, packetError) != IFX_SUCCESS)
               {
                  DTI_PRN_USR_ERR_NL(DTI_DBG, DTI_PRN_LEVEL_ERR,
                     ("Error: worker thr - EXIT on recv error handling."DTI_CRLF));
               }
            }
            recvBytes = 0;
            pProtocolServerCtx->packetIn.pPacket = IFX_NULL;
            msgIdleCount = 0;
            break;

         case DTI_CON_RECV_STATUS_PENDING:
            break;
         case DTI_CON_RECV_STATUS_ERROR:
            break;
         case DTI_CON_RECV_STATUS_STREAM_RESYNC:
            DTI_CONN_RESYNC_TRIGGER_INC(pAgentCtx, pWorkerCtx->workerNum);
            break;

         case DTI_CON_RECV_STATUS_CON_CLOSED:
            DTI_CONN_CLOSE_TRIGGER_INC(pAgentCtx, pWorkerCtx->workerNum);
            bConClosed = IFX_TRUE;
            break;

         default:
            break;
      }

      bAutoDevMsgActive = IFX_FALSE;
      for (devIdx = 0; devIdx < DTI_MAX_DEVICE_INTERFACES; devIdx++)
      {
         pDtiDevCtx = pWorkerCtx->pDtiDevCtx[devIdx];
         if (pDtiDevCtx != IFX_NULL)
         {
            if ( (pDtiDevCtx->bAutoDevMsgActive == IFX_TRUE) && (bConClosed == IFX_FALSE) )
            {
               bAutoDevMsgActive = IFX_TRUE;
               (void)DTI_device_autoMsgProcess(
                              pDtiDevCtx, &pProtocolServerCtx->dtiCon,
                              (IFX_uint32_t)devSelectWait_ms,
                              pProtocolServerCtx->packetOut.buffer,
                              sizeof(pProtocolServerCtx->packetOut.buffer));
            }
         }
      }

      bControlAutoCliMsgSupport = IFX_FALSE;
#     if defined(INCLUDE_CLI_SUPPORT) && (INCLUDE_CLI_SUPPORT == 1)
      if ( (pAgentCtx->bControlAutoCliMsgSupport == IFX_TRUE) && (bConClosed == IFX_FALSE) )
      {
         bControlAutoCliMsgSupport = IFX_TRUE;
         (void)DTI_CLI_autoMsgProcess(
                  pAgentCtx, pWorkerCtx->workerNum, pProtocolServerCtx);
      }
#     endif

      if (bConClosed == IFX_TRUE)
      {
         break;
      }

      if (   (bAutoDevMsgActive         == IFX_TRUE)
          || (bControlAutoCliMsgSupport == IFX_TRUE)
         )
      {
         /* DTI connection and device handling --> poll both */
         sockSelectWait_ms = 10;
         thrSleep_ms = 10 * msgIdleCount;

         if (msgIdleCount < 5)
         {
            msgIdleCount++;
         }

         DTI_MSecSleep(thrSleep_ms);
      }
      else
      {
         sockSelectWait_ms = DTI_WORKER_THREAD_WAIT_MS;
      }
   }     /* thread loop */

DTI_WORKER_THREAD_EXIT:
   DTI_WORKER_THR_CLOSE_INC(pAgentCtx, pWorkerCtx->workerNum);

   DTI_PRN_USR_DBG_NL(DTI_DBG, DTI_PRN_LEVEL_HIGH,
      ("worker thr[%d] - leave." DTI_CRLF,
         (pWorkerCtx != IFX_NULL) ? pWorkerCtx->workerNum : 99 ));

   if ( (pAgentCtx != IFX_NULL) && (pWorkerCtx != IFX_NULL) )
   {
      (void)DTI_CLI_workerEventRelease(
               pAgentCtx, pWorkerCtx->workerNum);

      for (devIdx = 0; devIdx < DTI_MAX_DEVICE_INTERFACES; devIdx++)
      {
         pCurrDevInterface = pWorkerCtx->pUsedDevInterfaces[devIdx];
         pDtiDevCtx        = pWorkerCtx->pDtiDevCtx[devIdx];
         pWorkerCtx->pDtiDevCtx[devIdx] = IFX_NULL;

         if (pDtiDevCtx != IFX_NULL)
         {
            pDeviceAccessFct = pDtiDevCtx->pDeviceAccessFct;
            pDtiDevCtx->pDeviceAccessFct = IFX_NULL;
            (void)DTI_device_moduleStop(
                     pDeviceAccessFct,
                     &pCurrDevInterface->deviceSysInfo,
                     &pDtiDevCtx);
         }
      }

      for (devIdx = 0; devIdx < DTI_MAX_DEVICE_INTERFACES; devIdx++)
      {
         pCurrDevInterface = pWorkerCtx->pUsedDevInterfaces[devIdx];
         pWorkerCtx->pUsedDevInterfaces[devIdx] = IFX_NULL;
         if (pCurrDevInterface != IFX_NULL)
         {
            DTI_Free(pCurrDevInterface);
         }
      }

      if (pProtocolServerCtx != IFX_NULL)
      {
         (void)DTI_conClose(&pProtocolServerCtx->dtiCon);
      }
   }

   return nIfxRet;
}

/**
   DTI worker thread function, this is a wrapper to keep the thread running.
*/
DTI_STATIC IFX_int_t DTI_WorkerThreadFct(
                        IFXOS_ThreadParams_t    *pThreadParams)
{
   IFX_int_t       retVal = IFX_SUCCESS;
   DTI_AgentCtx_t  *pAgentCtx = (DTI_AgentCtx_t*)pThreadParams->nArg1;
   DTI_WorkerCtx_t *pWorkerCtx   = (DTI_WorkerCtx_t*)pThreadParams->nArg2;

   pWorkerCtx->thrResumeState = DTI_WORKER_THREAD_RESUME_IN_USE;

   while (pThreadParams->bShutDown == IFX_FALSE)
   {
      if (pWorkerCtx->thrResumeState == DTI_WORKER_THREAD_RESUME_IN_USE)
      {
         /* enter the thread function */
         retVal = DTI_Worker(pAgentCtx, pWorkerCtx, &pThreadParams->bShutDown);
         pWorkerCtx->thrResumeState = DTI_WORKER_THREAD_RESUME_FREE;
      }
      else
      {
         /* wait for resume */
         DTI_MSecSleep(100);
      }
   }

   return retVal;
}


/**
   Search for a free struct and return the index
*/
DTI_STATIC IFX_int_t DTI_WorkerThreadGet(
                        DTI_AgentCtx_t       *pAgentCtx)
{
   IFX_int_t i;
   DTI_WorkerCtx_t *pWorkerCtx = IFX_NULL;

   if (pAgentCtx->bSingleThreadedMode == IFX_TRUE)
   {
      /* single threaded mode: take the first worker one */
      i = 0;
   }
   else
   {
      for (i = 0; i < DTI_NUM_OF_USED_WORKER_GET(pAgentCtx); i++)
      {
         if ((pWorkerCtx = pAgentCtx->pWorker[i]) == IFX_NULL)
         {
            /* found - still empty entry */
            break;
         }
         else
         {
            if (IFXOS_THREAD_INIT_VALID(&pWorkerCtx->thr) == IFX_TRUE)
            {
               if ( (pWorkerCtx->thr.thrParams.bRunning == IFX_TRUE) &&
                    (pWorkerCtx->thrResumeState == DTI_WORKER_THREAD_RESUME_FREE) )
               {
                  /* found: thread already running, reuse it */
                  break;
               }
            }
            else
            {
               /* found: thread not running, unused entry */
               break;
            }
         }
      }
   }

   return (i < DTI_NUM_OF_USED_WORKER_GET(pAgentCtx)) ? i : IFX_ERROR;
}

/* DTI worker Startup */
DTI_STATIC IFX_int_t DTI_WorkerThreadStartup(
                        DTI_AgentCtx_t          *pAgentCtx,
                        DTI_Connection_t        *pNewDtiCon)
{
   IFX_int_t               retVal, workerId = -1;
   DTI_WorkerCtx_t         *pWorkerCtx         = IFX_NULL;

   /*
      Create / resume new worker.
   */
   if (DTI_LockGet(&pAgentCtx->dataLock) != IFX_SUCCESS)
   {
      (void)DTI_conClose(pNewDtiCon);

      DTI_PRN_USR_ERR_NL(DTI_DBG, DTI_PRN_LEVEL_ERR,
         ("ERROR: worker thr startup - semaphore."DTI_CRLF));

      return IFX_ERROR;
   }

   if ( (workerId = DTI_WorkerThreadGet(pAgentCtx)) == IFX_ERROR)
   {
      DTI_LockRelease(&pAgentCtx->dataLock);
      (void)DTI_conClose(pNewDtiCon);

      DTI_PRN_USR_ERR_NL(DTI_DBG, DTI_PRN_LEVEL_ERR,
         ("ERROR: worker thr startup - no free struct."DTI_CRLF));

      return IFX_ERROR;
   }

   pWorkerCtx = pAgentCtx->pWorker[workerId];
   if (DTI_WorkerContextInit(pAgentCtx, pNewDtiCon, workerId, &pWorkerCtx) != IFX_SUCCESS)
   {
      DTI_CONN_REQ_LISTEN_REQ_FAIL_INC(pAgentCtx);
      DTI_LockRelease(&pAgentCtx->dataLock);
      (void)DTI_conClose(pNewDtiCon);

      DTI_PRN_USR_ERR_NL(DTI_DBG, DTI_PRN_LEVEL_ERR,
         ("ERROR: worker thr startup - init worker ctx."DTI_CRLF));

      return IFX_ERROR;
   }
   DTI_CONN_REQ_LISTEN_REQ_DONE_INC(pAgentCtx);
   pAgentCtx->pWorker[workerId] = pWorkerCtx;

   /*
      let the worker work
   */
   if (pAgentCtx->bSingleThreadedMode == IFX_TRUE)
   {
      pAgentCtx->nWorkers = 1;
      DTI_LockRelease(&pAgentCtx->dataLock);

      DTI_PRN_USR_DBG_NL(DTI_DBG, DTI_PRN_LEVEL_ERR,
            ("INFO: worker thr startup - single threaded mode, worker %d."DTI_CRLF, workerId));

      retVal = DTI_Worker(pAgentCtx, pWorkerCtx, &pAgentCtx->listenerThr.thrParams.bShutDown);
   }
   else
   {
      if (IFXOS_THREAD_INIT_VALID(&pWorkerCtx->thr) != IFX_TRUE)
      {
         retVal = DTI_ThreadInit(
                       &pWorkerCtx->thr, "DTIWorker", DTI_WorkerThreadFct,
                       DTI_WORKER_THREAD_STACK_SIZE, DTI_WORKER_THREAD_PRIO,
                       (IFX_ulong_t)pAgentCtx,
                       (IFX_ulong_t)pWorkerCtx);
         if ( retVal != IFX_SUCCESS)
         {
            DTI_WORKER_THR_START_FAIL_INC(pAgentCtx, workerId);
            DTI_LockRelease(&pAgentCtx->dataLock);
            (void)DTI_conClose(pNewDtiCon);

            DTI_PRN_USR_ERR_NL(DTI_DBG, DTI_PRN_LEVEL_ERR,
                  ("ERROR: worker thr startup - start new thr %d."DTI_CRLF, workerId));

            return IFX_ERROR;
         }
         else
         {
            DTI_PRN_USR_DBG_NL(DTI_DBG, DTI_PRN_LEVEL_ERR,
                  ("INFO: worker thr startup - start worker thr %d."DTI_CRLF, workerId));
         }
      }
      else
      {
         DTI_PRN_USR_DBG_NL(DTI_DBG, DTI_PRN_LEVEL_ERR,
               ("INFO: worker thr startup - reuse worker thr %d."DTI_CRLF, workerId));

         pWorkerCtx->thrResumeState = DTI_WORKER_THREAD_RESUME_IN_USE;
      }

      pAgentCtx->nWorkers++;
      DTI_LockRelease(&pAgentCtx->dataLock);
   }


   return IFX_SUCCESS;
}


/**
   Stop all Worker Threads
*/
DTI_STATIC IFX_int_t DTI_WorkerThreadStopAll(
                        DTI_AgentCtx_t *pAgentCtx)
{
   IFX_int_t i;
   DTI_WorkerCtx_t *pWorker = IFX_NULL;

   if (pAgentCtx == IFX_NULL)
   {
      DTI_PRN_USR_ERR_NL(DTI_DBG, DTI_PRN_LEVEL_ERR,
         ("Error: worker thr stop all - NULL ptr arg."DTI_CRLF));

      return IFX_ERROR;
   }

   if (DTI_LockGet(&pAgentCtx->dataLock) != IFX_SUCCESS)
   {
      DTI_PRN_USR_ERR_NL(DTI_DBG, DTI_PRN_LEVEL_ERR,
         ("ERROR: worker thr stop all - semaphore."DTI_CRLF));

      return IFX_ERROR;
   }

   /*lint -e{794} */
   for (i = 0; i < DTI_NUM_OF_USED_WORKER_GET(pAgentCtx); i++)
   {
      /*lint -e{794} */
      if ( (pWorker = pAgentCtx->pWorker[i]) != IFX_NULL)
      {
         if (IFXOS_THREAD_INIT_VALID(&pWorker->thr) == IFX_TRUE)
         {
            if (DTI_ThreadShutdown(&pWorker->thr, DTI_WORKER_THREAD_WAIT_MS * 2) == IFX_SUCCESS)
            {
               DTI_MemSet(&pWorker->thr, 0x00, sizeof(IFXOS_ThreadCtrl_t));
               pAgentCtx->nWorkers--;
            }
         }
      }
   }

   /*lint -e{794} */
   for (i = 0; i < DTI_NUM_OF_USED_WORKER_GET(pAgentCtx); i++)
   {
      /*lint -e{794} */
      if ( (pWorker = pAgentCtx->pWorker[i]) != IFX_NULL)
      {
         if (IFXOS_THREAD_INIT_VALID(&pWorker->thr) == IFX_TRUE)
         {
            DTI_ThreadDelete(&pWorker->thr, DTI_WORKER_THREAD_WAIT_MS * 4);
            DTI_MemSet(&pWorker->thr, 0x00, sizeof(IFXOS_ThreadCtrl_t));

            pAgentCtx->nWorkers--;
         }
      }
   }

   /*lint -e{794} */
   for (i = 0; i < DTI_NUM_OF_USED_WORKER_GET(pAgentCtx); i++)
   {
      /*lint -e{794} */
      if ( (pWorker = pAgentCtx->pWorker[i]) != IFX_NULL)
      {
         pAgentCtx->pWorker[i] = IFX_NULL;

         (void)DTI_WorkerContextRelease(pAgentCtx, pWorker);
         DTI_Free(pWorker);

      }
   }

   /*lint -e{794} */
   DTI_LockRelease(&pAgentCtx->dataLock);

   return IFX_SUCCESS;
}

/* DTI Listener */
DTI_STATIC IFX_int_t DTI_ListenerThread (
                        IFXOS_ThreadParams_t *pThreadParams)
{
   IFX_int_t         imgNum, nIfxRet = IFX_SUCCESS;
   DTI_AgentCtx_t    *pAgentCtx = (DTI_AgentCtx_t*)pThreadParams->nArg1;
   DTI_Connection_t  newDtiConnection;

   DTI_PRN_USR_DBG_NL(DTI_DBG, DTI_PRN_LEVEL_HIGH,
      ("listen thr - enter."DTI_CRLF));

   if (pAgentCtx == IFX_NULL)
   {
      DTI_PRN_USR_ERR_NL(DTI_DBG, DTI_PRN_LEVEL_ERR,
         ("Error: listen thr - NULL ptr arg."DTI_CRLF));

      nIfxRet = IFX_ERROR;
      goto DTI_LISTENER_THREAD_EXIT;
   }

   DTI_MemSet(&pAgentCtx->listenCon,   0x00, sizeof(pAgentCtx->listenCon));

   nIfxRet = DTI_conOpen(
                        &pAgentCtx->listenCon, IFX_FALSE,
                        pAgentCtx->listenConCntrl.sockPort,
                        pAgentCtx->listenConCntrl.ipAddr);

   if (nIfxRet != IFX_SUCCESS)
   {
      DTI_PRN_USR_ERR_NL(DTI_DBG, DTI_PRN_LEVEL_ERR,
         ("ERROR: listen thr - DTI Connection Open."DTI_CRLF));

      nIfxRet = IFX_ERROR;
      goto DTI_LISTENER_THREAD_EXIT;
   }

   DTI_conAddForRecvWait(&pAgentCtx->listenCon, &pAgentCtx->listenConCntrl);

   pAgentCtx->bListenRun = IFX_TRUE;

   /*
      Thread Loop
   */
   while(pAgentCtx->bListenRun == IFX_TRUE && pThreadParams->bShutDown == 0)
   {
      if (DTI_conNewConnectGet(
                        &pAgentCtx->listenCon,
                        &pAgentCtx->listenConCntrl,
                        DTI_LISTENER_ACCEPT_WAIT_MS,
                        &newDtiConnection) == 1)
      {
         DTI_CONN_REQ_LISTEN_REQ_INC(pAgentCtx);
         nIfxRet = DTI_WorkerThreadStartup(pAgentCtx, &newDtiConnection);
      }
   }     /* thread loop */

   pAgentCtx->bListenRun = IFX_FALSE;

DTI_LISTENER_THREAD_EXIT:

   DTI_PRN_USR_DBG_NL(DTI_DBG, DTI_PRN_LEVEL_HIGH,
      ("listen thr - leave."DTI_CRLF));

   if (pAgentCtx != IFX_NULL)
   {
      /* stop all worker */
      (void)DTI_WorkerThreadStopAll(pAgentCtx);

      /* stop CLI interface */
      (void)DTI_CLI_moduleStop(pAgentCtx);

      /* free loaded files */
      for (imgNum = 0; imgNum < DTI_MAX_NUM_OF_IMAGES; imgNum++)
      {
         if (pAgentCtx->imageCntrl[imgNum].pData != IFX_NULL)
         {
            if (pAgentCtx->imageCntrl[imgNum].useCount)
            {
               DTI_PRN_USR_ERR_NL(DTI_DBG, DTI_PRN_LEVEL_WRN,
                  ("WARNING: listen thr - leave, image %d still in use."DTI_CRLF,
                    imgNum));
            }

            pAgentCtx->imageCntrl[imgNum].imageSize  = 0;
            pAgentCtx->imageCntrl[imgNum].loadedSize = 0;

            if (pAgentCtx->imageCntrl[imgNum].pData != IFX_NULL)
            {
               DTI_Free(pAgentCtx->imageCntrl[imgNum].pData);
            }
            pAgentCtx->imageCntrl[imgNum].pData      = IFX_NULL;
            pAgentCtx->imageCntrl[imgNum].useCount   = 0;
         }

         pAgentCtx->imageCntrl[imgNum].imageId = imgNum;
         pAgentCtx->imageCntrl[imgNum].maxChunkSize = DTI_MAX_IMAGE_CHUNK_SIZE;
         pAgentCtx->imageCntrl[imgNum].maxImageSize = DTI_MAX_IMAGE_SIZE;
      }
   }

   DTI_conClose(&pAgentCtx->listenCon);

   return IFX_SUCCESS;
}


/* ============================================================================
   Global Functions
   ========================================================================= */

/**
   DTI Listener startup
*/
IFX_int_t DTI_ListenerThread_Startup (
                        DTI_AgentCtx_t *pAgentCtx)
{

   if (pAgentCtx == IFX_NULL)
   {
      DTI_PRN_USR_ERR_NL(DTI_DBG, DTI_PRN_LEVEL_ERR,
         ("Error: listen thr startup - NULL ptr arg."DTI_CRLF));

      return DTI_ERR_NULL_PTR_ARG;
   }

   if (pAgentCtx->bListenRun == IFX_TRUE)
   {
      DTI_PRN_USR_ERR_NL(DTI_DBG, DTI_PRN_LEVEL_ERR,
         ("Error: listen thr startup - already running."DTI_CRLF));

      return DTI_ERR_CONFIGURATION;
   }

   /*
      start thread
   */
   DTI_MemSet(&pAgentCtx->listenerThr,  0x00, sizeof(pAgentCtx->listenerThr));

   if (DTI_ThreadInit(
            &pAgentCtx->listenerThr, "DTIListen", DTI_ListenerThread,
            DTI_LISTENER_THREAD_STACK_SIZE, DTI_LISTENER_THREAD_PRIO,
            (IFX_ulong_t)pAgentCtx, 0) != IFX_SUCCESS)
   {
      DTI_PRN_USR_ERR_NL(DTI_DBG, DTI_PRN_LEVEL_ERR,
         ("Error: listen thr startup - start thread."DTI_CRLF));

      return DTI_ERR_INTERNAL;
   }

   /* wait for listener run */


   return IFX_SUCCESS;
}

/**
   DTI Listener Stop
*/
IFX_int_t DTI_ListenerThread_Stop (
                        DTI_AgentCtx_t *pAgentCtx)
{
   IFX_int_t retVal = IFX_SUCCESS;

   DTI_PRN_USR_DBG_NL(DTI_DBG, DTI_PRN_LEVEL_HIGH,
      ("WARNING: listener thr stop - stop all worker."DTI_CRLF));

   if (pAgentCtx->bSingleThreadedMode == IFX_TRUE)
   {
      if ( DTI_ThreadShutdown(&pAgentCtx->listenerThr, DTI_LISTENER_THREAD_WAIT_MS * 4) != IFX_SUCCESS)
      {
         DTI_ThreadDelete(&pAgentCtx->listenerThr, DTI_LISTENER_THREAD_WAIT_MS * 4);
      }
   }
   else
   {
      retVal = DTI_WorkerThreadStopAll(pAgentCtx);

      if ( DTI_ThreadShutdown(&pAgentCtx->listenerThr, DTI_LISTENER_THREAD_WAIT_MS * 2) != IFX_SUCCESS)
      {
         DTI_ThreadDelete(&pAgentCtx->listenerThr, DTI_LISTENER_THREAD_WAIT_MS * 4);
      }
   }

   return retVal;
}


/**
   Converts the first string word and return the pointer to the next word.

\param
   ppArgStr    - return pointer, contains ptr to the arg string
\param
   pArgLen     - points to the current arg length and returns remaining len
\param
   pULVal      - returns the converted value
\param
   base        - convertion base (0 default, 10 dec, 16 hex).


*/
IFX_int_t DTI_cliControlGetNextDigitVal(
                        IFX_char_t  **ppArgStr,
                        IFX_uint_t  *pArgLen,
                        IFX_uint_t  *pULVal,
                        IFX_uint_t  base)
{
   IFX_ulong_t ulVal  = ~0;
   IFX_uint_t  argLen = *pArgLen;
   IFX_char_t  *pCurrArg = *ppArgStr, *pNextArg;
   IFX_char_t  argBuffer[32];

   pNextArg = &argBuffer[0];

   /* remove leading blanks */
   while ( (DTI_IsSpace((IFX_int_t)*pCurrArg)) && (argLen) )
   {
      pCurrArg++;
      argLen--;
   }

   /* take a copy for convertion */
   while ( argLen &&  (*pCurrArg != '\0') &&
           !(DTI_IsSpace((IFX_int_t)*pCurrArg)) &&
           ((*pArgLen - argLen) < 30) )
   {
      *pNextArg = *pCurrArg;
      pCurrArg++;
      pNextArg++;
      argLen--;
   }
   *pNextArg = '\0';

   if (DTI_convertStrToUl(argBuffer, (IFX_int_t)base, &ulVal) != IFX_SUCCESS)
   {
      DTI_PRN_USR_ERR_NL(DTI_DBG, DTI_PRN_LEVEL_ERR,
         ("Error: Next Digit Val - convertion failed." DTI_CRLF));

      return IFX_ERROR;
   }

   /* remove leading blanks from the next string value */
   while ( (DTI_IsSpace((IFX_int_t)*pCurrArg)) && (argLen) )
   {
      pCurrArg++;
      argLen--;
   }

   *ppArgStr = pCurrArg;
   *pArgLen  = argLen;
   *pULVal   = (IFX_uint_t)ulVal;

   return IFX_SUCCESS;
}

