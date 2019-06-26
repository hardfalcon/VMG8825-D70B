/******************************************************************************

                              Copyright (c) 2009
                            Lantiq Deutschland GmbH
                     Am Campeon 3; 85579 Neubiberg, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/** \file
   Debug and Trace Interface - Generic Device Interface (empty).
*/

/* ============================================================================
   Includes
   ========================================================================= */
#include "dti_osmap.h"

#if defined(DEVICE_GENERIC)

#include "ifx_dti_protocol.h"
#include "ifx_dti_protocol_device.h"

#include "dti_device.h"
#include "dti_device_generic_ctx.h"


/* ============================================================================
   Defines
   ========================================================================= */

#ifdef DTI_STATIC
#  undef DTI_STATIC
#endif

#ifdef DTI_DEBUG
#  define DTI_STATIC
#else
#  define DTI_STATIC       static
#endif

#define DTI_DEVICE_IF_NAME_GENERIC                    "GENERIC"

#define DTI_BOARD_NAME_STR_GENERIC                    "GENERIC Board"

/** base device name for generic devices */
#define DTI_DEVICE_NAME_PREFIX_GENERIC                "/dev/generic"

/** default GENERIC number of devices */
#define DTI_DEV_GENERIC_DEFAULT_NUM_OF_DEVS           1
/** default GENERIC ports per device */
#define DTI_DEV_GENERIC_DEFAULT_PORTS_PER_DEV         1




/* ============================================================================
   Local Function Declaration
   ========================================================================= */


/* ============================================================================
   Device Access Functions
   ========================================================================= */

DTI_STATIC IFX_int_t DTI_GEN_PrintoutLevelSet(
                        IFX_int_t            newDbgLevel);

DTI_STATIC IFX_int_t DTI_GEN_ModuleSetup(
                        DTI_DeviceSysInfo_t  *pDeviceSystemInfo,
                        DTI_DeviceCtx_t      **ppDtiDevCtx);

DTI_STATIC IFX_int_t DTI_GEN_ModuleDelete(
                        DTI_DeviceSysInfo_t  *pDeviceSystemInfo,
                        DTI_DeviceCtx_t      **ppDtiDevCtx);

DTI_STATIC IFX_int_t DTI_GEN_SystemInfoWrite(
                        DTI_DeviceSysInfo_t  *pDeviceSystemInfo,
                        IFX_char_t           *pSysInfoBuffer,
                        IFX_int_t            bufferSize);

DTI_STATIC IFX_int_t DTI_GEN_Reset(
                        DTI_DeviceCtx_t         *pDtiDevCtx,
                        DTI_H2D_DeviceReset_t   *pInDevReset,
                        DTI_D2H_DeviceReset_t   *pOutDevReset,
                        IFX_int_t               rstMaskSize_32,
                        DTI_PacketError_t       *pPacketError);

DTI_STATIC IFX_int_t DTI_GEN_Download(
                        DTI_DeviceCtx_t            *pDtiDevCtx,
                        DTI_ProtocolServerCtx_t    *pDtiProtServerCtx,
                        DTI_H2D_DeviceDownload_t   *pInDevDownload,
                        DTI_D2H_DeviceDownload_t   *pOutDevDownload,
                        DTI_PacketError_t          *pPacketError);

DTI_STATIC IFX_int_t DTI_GEN_DeviceOpen(
                        DTI_DeviceCtx_t      *pDtiDevCtx,
                        IFX_int_t            lineNum,
                        DTI_PacketError_t    *pPacketError);

DTI_STATIC IFX_int_t DTI_GEN_DeviceClose(
                        DTI_DeviceCtx_t      *pDtiDevCtx,
                        IFX_int_t            lineNum,
                        DTI_PacketError_t    *pPacketError);

DTI_STATIC IFX_int_t DTI_GEN_RegisterLock(
                        DTI_DeviceCtx_t      *pDtiDevCtx,
                        DTI_H2D_DeviceLock_t *pInLock,
                        DTI_D2H_DeviceLock_t *pOutLock,
                        IFX_int_t            lineNum,
                        DTI_PacketError_t    *pPacketError);

DTI_STATIC IFX_int_t DTI_GEN_RegisterGet(
                        DTI_DeviceCtx_t         *pDtiDevCtx,
                        DTI_H2D_RegisterGet_t   *pInRegGet,
                        DTI_D2H_RegisterGet_t   *pOutRegGet,
                        IFX_int_t               lineNum,
                        IFX_uint32_t            *pOutPaylSize_byte,
                        DTI_PacketError_t       *pPacketError);

DTI_STATIC IFX_int_t DTI_GEN_RegisterSet(
                        DTI_DeviceCtx_t         *pDtiDevCtx,
                        DTI_H2D_RegisterSet_t   *pInRegSet,
                        IFX_int_t               inRegSetSize_Byte,
                        IFX_int_t               lineNum,
                        DTI_PacketError_t       *pPacketError);

DTI_STATIC IFX_int_t DTI_GEN_ConfigSet(
                        DTI_DeviceCtx_t            *pDtiDevCtx,
                        DTI_H2D_DeviceConfigSet_t  *pInCfgSet,
                        DTI_D2H_DeviceConfigSet_t  *pOutCfgSet,
                        IFX_int_t                  lineNum,
                        DTI_PacketError_t          *pPacketError);

DTI_STATIC IFX_int_t DTI_GEN_ConfigGet(
                        DTI_DeviceCtx_t            *pDtiDevCtx,
                        DTI_H2D_DeviceConfigGet_t  *pInCfgGet,
                        DTI_D2H_DeviceConfigGet_t  *pOutCfgGet,
                        IFX_int_t                  lineNum,
                        DTI_PacketError_t          *pPacketError);

DTI_STATIC IFX_int_t DTI_GEN_Message8Send(
                        DTI_DeviceCtx_t      *pDtiDevCtx,
                        DTI_H2D_Message8_u   *pInMsg8Send,
                        DTI_D2H_Message8_u   *pOutMsg8Send,
                        IFX_int_t            lineNum,
                        IFX_int_t            inPaylSize_byte,
                        IFX_int_t            *pOutPaylSize_byte,
                        DTI_PacketError_t    *pPacketError);

DTI_STATIC IFX_int_t DTI_GEN_Message16Send(
                        DTI_DeviceCtx_t      *pDtiDevCtx,
                        DTI_H2D_Message16_u  *pInMsg16Send,
                        DTI_D2H_Message16_u  *pOutMsg16Send,
                        IFX_int_t            lineNum,
                        IFX_int_t            inPaylSize_byte,
                        IFX_int_t            *pOutPaylSize_byte,
                        DTI_PacketError_t    *pPacketError);

DTI_STATIC IFX_int_t DTI_GEN_Message32Send(
                        DTI_DeviceCtx_t      *pDtiDevCtx,
                        DTI_H2D_Message32_u  *pInMsg32Send,
                        DTI_D2H_Message32_u  *pOutMsg32Send,
                        IFX_int_t            lineNum,
                        IFX_int_t            inPaylSize_byte,
                        IFX_int_t            *pOutPaylSize_byte,
                        DTI_PacketError_t    *pPacketError);

DTI_STATIC IFX_int_t DTI_GEN_TraceBufferConfigSet(
                        DTI_DeviceCtx_t            *pDtiDevCtx,
                        DTI_H2D_TraceConfigSet_t   *pInTraceConfigSet,
                        DTI_D2H_TraceConfigSet_t   *pOutTraceConfigSet,
                        IFX_int_t                  lineNum,
                        DTI_PacketError_t          *pPacketError);

DTI_STATIC IFX_int_t DTI_GEN_TraceBufferReset(
                        DTI_DeviceCtx_t      *pDtiDevCtx,
                        IFX_int_t            lineNum,
                        DTI_PacketError_t    *pPacketError);

DTI_STATIC IFX_int_t DTI_GEN_TraceBufferStatusGet(
                        DTI_DeviceCtx_t            *pDtiDevCtx,
                        DTI_D2H_TraceStatusGet_t   *pOutTraceStatusGet,
                        IFX_int_t                  lineNum,
                        DTI_PacketError_t          *pPacketError);

DTI_STATIC IFX_int_t DTI_GEN_TraceBufferGet(
                        DTI_DeviceCtx_t            *pDtiDevCtx,
                        DTI_H2D_TraceBufferGet_t   *pInTraceBufferGet,
                        DTI_Packet_t               **ppUsedDtiPacketOut,
                        IFX_int_t                  *pUsedBufferOutSize,
                        IFX_int_t                  lineNum,
                        IFX_int_t                  *pTrBufReadSize_byte,
                        DTI_PacketError_t          *pPacketError);

DTI_STATIC IFX_int_t DTI_GEN_DebugRead(
                        DTI_DeviceCtx_t      *pDtiDevCtx,
                        DTI_H2D_DebugRead_t  *pInDbgGet,
                        DTI_D2H_DebugRead_t  *pOutDbgGet,
                        IFX_int_t            lineNum,
                        IFX_int_t            *pDbgReadCount,
                        DTI_PacketError_t    *pPacketError);

DTI_STATIC IFX_int_t DTI_GEN_DebugWrite(
                        DTI_DeviceCtx_t      *pDtiDevCtx,
                        DTI_H2D_DebugWrite_t *pInDbgSet,
                        IFX_uint32_t         *pOutDbgGet_nU,
                        IFX_int_t            lineNum,
                        IFX_int_t            *pDbgWriteCount,
                        DTI_PacketError_t    *pPacketError);

DTI_STATIC IFX_int_t DTI_GEN_AutoMsgProcess(
                        DTI_DeviceCtx_t         *pDtiDevCtx,
                        const DTI_Connection_t  *pDtiCon,
                        IFX_uint32_t            devSelectWait_ms,
                        IFX_char_t              *pOutBuffer,
                        IFX_int_t               outBufferSize_byte);

DTI_STATIC IFX_int_t DTI_GEN_WinEasyCiAccess(
                        DTI_DeviceCtx_t      *pDtiDevCtx,
                        IFX_int_t            lineNum,
                        const IFX_uint8_t    *pDataIn,
                        const IFX_uint32_t   sizeIn,
                        IFX_uint8_t          *pDataOut,
                        const IFX_uint32_t   sizeOut,
                        DTI_PacketError_t    *pPacketError);

/* ============================================================================
   Variables
   ========================================================================= */

/* Create device debug module - user part */
IFXOS_PRN_USR_MODULE_CREATE(DTI_GEN, DTI_PRN_LEVEL_HIGH);

#if (DTI_DEVICE_INTERFACE_VERSION != 0x00010200)
#error "wrong DTI_DEVICE_INTERFACE_VERSION!"
#endif

/**
   Collection of all available device access functions.
*/
DTI_DeviceAccessFct_t DTI_DeviceAccessFct_GENERIC =
{
   sizeof(DTI_DeviceAccessFct_t),         /* structure check */
   DTI_DEVICE_INTERFACE_VERSION,          /* device interface version */
   DTI_DEVICE_IF_NAME_GENERIC,            /* device name */

   DTI_GEN_PrintoutLevelSet,
   DTI_GEN_ModuleSetup,
   DTI_GEN_ModuleDelete,
   DTI_GEN_SystemInfoWrite,

   DTI_GEN_Reset,
   DTI_GEN_Download,
   DTI_GEN_DeviceOpen,
   DTI_GEN_DeviceClose,
   DTI_GEN_RegisterLock,
   DTI_GEN_RegisterGet,
   DTI_GEN_RegisterSet,
   DTI_GEN_ConfigSet,
   DTI_GEN_ConfigGet,
   DTI_GEN_Message16Send,
   DTI_GEN_Message32Send,
   DTI_GEN_TraceBufferConfigSet,
   DTI_GEN_TraceBufferReset,
   DTI_GEN_TraceBufferStatusGet,
   DTI_GEN_TraceBufferGet,
   DTI_GEN_DebugRead,
   DTI_GEN_DebugWrite,
   DTI_GEN_AutoMsgProcess,
   DTI_GEN_WinEasyCiAccess,

#if (DTI_DEVICE_INTERFACE_VERSION > 0x00000100)
   DTI_GEN_Message8Send,
#endif
};


/* ============================================================================
   Local Function
   ========================================================================= */


/* ============================================================================
   Device Access Functions
   ========================================================================= */

/**
   Set a new debug level.
*/
DTI_STATIC IFX_int_t DTI_GEN_PrintoutLevelSet(
                        IFX_int_t            newDbgLevel)
{

   IFXOS_PRN_USR_LEVEL_SET(DTI_GEN, newDbgLevel);

   return IFX_SUCCESS;
}


/**
   Setup GENERIC Module

\param
   pDeviceSystemInfo - points to the system infos
\param
   DTI_DeviceCtx_t   - pointer to return the allocated Modul struct.

\return
   If success IFX_SUCCESS, the context is returned via the ppDtiDevCtx param
   else IFX_ERROR.

*/
DTI_STATIC IFX_int_t DTI_GEN_ModuleSetup(
                        DTI_DeviceSysInfo_t  *pDeviceSystemInfo,
                        DTI_DeviceCtx_t      **ppDtiDevCtx)
{
   IFX_int_t   ctxSize = 0;
   DTI_DeviceCtx_t            *pDtiDevCtx = IFX_NULL;
   DTI_DEV_GenericDriverCtx_t *pGenDevCtx = IFX_NULL;
   DTI_PTR_U                  uDtiDevCtx;

   if ( (ppDtiDevCtx == IFX_NULL) || (pDeviceSystemInfo == IFX_NULL))
   {
      DTI_PRN_USR_ERR_NL(DTI_GEN, DTI_PRN_LEVEL_ERR,
         ("ERROR: Generic Dev Module Setup - NULL ptr args."DTI_CRLF));

      return IFX_ERROR;
   }

   if (*ppDtiDevCtx != IFX_NULL)
   {
      pDtiDevCtx = *ppDtiDevCtx;

      DTI_PRN_USR_ERR_NL(DTI_GEN, IFXOS_PRN_LEVEL_WRN,
         ("WARNING: Generic Dev Module Setup - already done (devs = %d, ports = %d)."DTI_CRLF,
         pGenDevCtx->numOfDevs, pGenDevCtx->numOfPorts));

      return IFX_ERROR;
   }


   DTI_PRN_USR_DBG_NL(DTI_GEN, DTI_PRN_LEVEL_HIGH,
      ("Generic Dev Module Setup - devs = %d, ports = %d."DTI_CRLF,
        pDeviceSystemInfo->numOfDevs, pDeviceSystemInfo->portsPerDev));

   ctxSize = sizeof(DTI_DeviceCtx_t) + sizeof(DTI_DEV_GenericDriverCtx_t);

   uDtiDevCtx.pUInt8 = (IFX_uint8_t *)DTI_Malloc((IFX_uint_t)ctxSize);
   if (uDtiDevCtx.pUInt8 == IFX_NULL)
   {
      DTI_PRN_USR_ERR_NL(DTI_GEN, DTI_PRN_LEVEL_ERR,
         ("ERROR: Generic Dev Module Setup - Dev Struct alloc."DTI_CRLF));

      return IFX_ERROR;
   }
   DTI_MemSet(uDtiDevCtx.pUInt8, 0x0, ctxSize);

   /* set context pointer */
   pDtiDevCtx = (DTI_DeviceCtx_t *)DTI_PTR_CAST_GET_ULONG(uDtiDevCtx);
   if (pDtiDevCtx == IFX_NULL)
   {
      DTI_Free(uDtiDevCtx.pUInt8);

      DTI_PRN_USR_ERR_NL(DTI_GEN, DTI_PRN_LEVEL_ERR,
         ("ERROR: Generic Dev Module Setup - Dev Struct miss-aligned."DTI_CRLF));

      return IFX_ERROR;
   }

   /* set pDevice pointer*/
   uDtiDevCtx.pUInt8  += sizeof(DTI_DeviceCtx_t);
   pGenDevCtx = (DTI_DEV_GenericDriverCtx_t*)DTI_PTR_CAST_GET_UINT32(uDtiDevCtx);

   if (pGenDevCtx == IFX_NULL)
   {
      DTI_Free(pDtiDevCtx);

      DTI_PRN_USR_ERR_NL(DTI_GEN, DTI_PRN_LEVEL_ERR,
         ("ERROR: Generic Dev Module Setup - Generic pDevice miss-aligned."DTI_CRLF));

      return IFX_ERROR;
   }


   pDtiDevCtx->bAutoDevMsgActive  = IFX_FALSE;

   pGenDevCtx->numOfDevs          = pDeviceSystemInfo->numOfDevs;
   pGenDevCtx->portsPerDev        = pDeviceSystemInfo->portsPerDev;
   pGenDevCtx->numOfPorts         = pDeviceSystemInfo->numOfPorts;

   pDtiDevCtx->pDevice = (IFX_void_t *)pGenDevCtx;
   *ppDtiDevCtx = pDtiDevCtx;

   return IFX_SUCCESS;
}


/**
   Delete a GENERIC Module.
   - close all open devices
   - free memory

\param
   ppDtiDevCtx - contains the modul context.

\return
   If success IFX_SUCCESS, the context is freed and the ptr is set to IFX_NULL.
   else IFX_ERROR.

*/
DTI_STATIC IFX_int_t DTI_GEN_ModuleDelete(
                        DTI_DeviceSysInfo_t  *pDeviceSystemInfo,
                        DTI_DeviceCtx_t      **ppDtiDevCtx)
{
   DTI_DeviceCtx_t            *pDtiDevCtx = IFX_NULL;

   if (ppDtiDevCtx == IFX_NULL)
   {
      DTI_PRN_USR_ERR_NL(DTI_GEN, DTI_PRN_LEVEL_ERR,
         ("ERROR: Generic Dev Module Delete - NULL ptr args."DTI_CRLF));

      return IFX_ERROR;
   }

   if (*ppDtiDevCtx == IFX_NULL)
   {
      DTI_PRN_USR_ERR_NL(DTI_GEN, DTI_PRN_LEVEL_ERR,
         ("ERROR: Generic Dev Module Delete - NULL ptr modul ctx."DTI_CRLF));

      return IFX_ERROR;
   }

   pDtiDevCtx = *ppDtiDevCtx;
   *ppDtiDevCtx = IFX_NULL;

   /*
      Release Device specific resources
   */

   DTI_Free(pDtiDevCtx);

   return IFX_SUCCESS;
}

/**
   Write the Sytem Info of the current DTI Agent instance to the given buffer.

\param
   pDeviceSystemInfo     - points to Sytem Info struct of the current agent instance.
\param
   pSysInfoBuffer    - points to the Sytem Info char buffer.
\param
   bufferSize        - buffer size.

\return
   Number of written bytes.
*/
DTI_STATIC IFX_int_t DTI_GEN_SystemInfoWrite(
                        DTI_DeviceSysInfo_t  *pDeviceSystemInfo,
                        IFX_char_t           *pSysInfoBuffer,
                        IFX_int_t            bufferSize)
{
   IFX_int_t writtenLen = 0;
   const IFX_char_t *pBoardName = IFX_NULL;

   (void)DTI_snprintf(&pSysInfoBuffer[writtenLen], bufferSize - writtenLen,
      "VendorName=%s", DTI_VENDOR_NAME_STR);
   writtenLen = (IFX_int_t)DTI_StrLen(&pSysInfoBuffer[writtenLen]) + 1;

   if (pDeviceSystemInfo->bValid == IFX_TRUE)
   {
      pBoardName = DTI_BOARD_NAME_STR_GENERIC;
   }

   (void)DTI_snprintf(&pSysInfoBuffer[writtenLen], bufferSize - writtenLen,
      "BoardName=%s", (pBoardName != IFX_NULL) ? pBoardName : "<unknown>");
   writtenLen += (IFX_int_t)DTI_StrLen(&pSysInfoBuffer[writtenLen]) + 1;

   (void)DTI_snprintf(&pSysInfoBuffer[writtenLen], bufferSize - writtenLen,
      "BoardRevision=0.0");
   writtenLen += (IFX_int_t)DTI_StrLen(&pSysInfoBuffer[writtenLen]) + 1;

   if (pDeviceSystemInfo->bValid == IFX_TRUE)
   {
      (void)DTI_snprintf(&pSysInfoBuffer[writtenLen], bufferSize - writtenLen,
         "NumOfDevices=%d", pDeviceSystemInfo->numOfDevs);
      writtenLen += (IFX_int_t)DTI_StrLen(&pSysInfoBuffer[writtenLen]) + 1;

      (void)DTI_snprintf(&pSysInfoBuffer[writtenLen], bufferSize - writtenLen,
         "MaxChannel=%d", pDeviceSystemInfo->numOfPorts);
      writtenLen += (IFX_int_t)DTI_StrLen(&pSysInfoBuffer[writtenLen]) + 1;
   }
   else
   {
      (void)DTI_snprintf(&pSysInfoBuffer[writtenLen], bufferSize - writtenLen,
         "NumOfDevices=<na>");
      writtenLen += (IFX_int_t)DTI_StrLen(&pSysInfoBuffer[writtenLen]) + 1;

      (void)DTI_snprintf(&pSysInfoBuffer[writtenLen], bufferSize - writtenLen,
         "MaxChannel=<na>");
      writtenLen += (IFX_int_t)DTI_StrLen(&pSysInfoBuffer[writtenLen]) + 1;
   }

   return writtenLen;
}

/**
   Do an device reset on the selected device.

\param
   pDtiDevCtx     - points to the GENERIC Device context.
\param
   pInDevReset    - points to the DTI Host2Dev Reset struct.
\param
   pOutDevReset   - points to the DTI Dev2Host Reset struct.
\param
   rstMaskSize_32 - number of mask elements (32 Bit).
\param
   pPacketError   - return ptr, return the DTI Packet Error.

\return
   IFX_SUCCESS and returns the DTI Packet Error code.
   IFX_ERROR   currently no (only in case of non-DTI related errors).
*/
DTI_STATIC IFX_int_t DTI_GEN_Reset(
                        DTI_DeviceCtx_t         *pDtiDevCtx,
                        DTI_H2D_DeviceReset_t   *pInDevReset,
                        DTI_D2H_DeviceReset_t   *pOutDevReset,
                        IFX_int_t               rstMaskSize_32,
                        DTI_PacketError_t       *pPacketError)
{
   DTI_PRN_USR_ERR_NL(DTI_GEN, DTI_PRN_LEVEL_ERR,
      ("WARNING: Generic Dev Reset - not implemented."DTI_CRLF));

   /* take over mask for responce */
   pOutDevReset->mask[0] = pInDevReset->mask[0];
   *pPacketError = DTI_eErrUnknown;

   return IFX_SUCCESS;
}

/**
   Do a FW download on the selected devices

\param
   pDtiDevCtx        - points to the GENERIC Device context.
\param
   pDtiProtServerCtx - points to the DTI protocol server context.
\param
   pInDevDownload    - points to the DTI Host2Dev Download struct.
\param
   pOutDevDownload   - points to the DTI Dev2Host Download struct.
\param
   pPacketError      - return ptr, return the DTI Packet Error.

\return
   IFX_SUCCESS and returns the DTI Packet Error code.
   IFX_ERROR   currently no (only in case of non-DTI related errors).
*/
IFX_int_t DTI_GEN_Download(
                        DTI_DeviceCtx_t            *pDtiDevCtx,
                        DTI_ProtocolServerCtx_t    *pDtiProtServerCtx,
                        DTI_H2D_DeviceDownload_t   *pInDevDownload,
                        DTI_D2H_DeviceDownload_t   *pOutDevDownload,
                        DTI_PacketError_t          *pPacketError)
{
   DTI_PRN_USR_DBG_NL(DTI_GEN, DTI_PRN_LEVEL_HIGH,
      ("WARNING: Generic Dev Download - not implemented."DTI_CRLF));

   pOutDevDownload->errorMask = 0xFFFFFFFF;
   *pPacketError = DTI_eErrUnknown;

   return IFX_SUCCESS;
}


/**
   Open a given line device.

\param
   pDtiDevCtx     - points to the GENERIC Device context.
\param
   lineNum        - line number.
\param
   pPacketError   - return ptr, return the DTI Packet Error.

\return
   IFX_SUCCESS and returns the DTI Packet Error code.
   IFX_ERROR   currently no (only in case of non-DTI related errors).
*/
DTI_STATIC IFX_int_t DTI_GEN_DeviceOpen(
                        DTI_DeviceCtx_t      *pDtiDevCtx,
                        IFX_int_t            lineNum,
                        DTI_PacketError_t    *pPacketError)
{
   DTI_PRN_USR_DBG_NL(DTI_GEN, DTI_PRN_LEVEL_HIGH,
      ("WARNING: Generic Dev Open - not implemented."DTI_CRLF));

   *pPacketError = DTI_eErrUnknown;

   return IFX_SUCCESS;
}


/**
   Close a given line device.

\param
   pDtiDevCtx     - points to the Generic Device context.
\param
   lineNum        - line number.
\param
   pPacketError   - return ptr, return the DTI Packet Error.

\return
   IFX_SUCCESS and returns the DTI Packet Error code.
   IFX_ERROR   currently no (only in case of non-DTI related errors).
*/
DTI_STATIC IFX_int_t DTI_GEN_DeviceClose(
                        DTI_DeviceCtx_t      *pDtiDevCtx,
                        IFX_int_t            lineNum,
                        DTI_PacketError_t    *pPacketError)
{
   DTI_PRN_USR_DBG_NL(DTI_GEN, DTI_PRN_LEVEL_HIGH,
      ("WARNING: Generic Dev Close - not implemented."DTI_CRLF));

   *pPacketError = DTI_eErrUnknown;

   return IFX_SUCCESS;
}



/**
   Get / release the device lock.

\param
   pDtiDevCtx     - points to the GENERIC Device context.
\param
   pInLock        - points to the DTI Host2Dev Lock struct.
\param
   pOutLock       - points to the DTI Dev2Host Lock struct.
\param
   lineNum        - line number.
\param
   pPacketError   - return ptr, return the DTI Packet Error.

\return
   IFX_SUCCESS and returns the DTI Packet Error code.
   IFX_ERROR   currently no (only in case of non-DTI related errors).
*/
DTI_STATIC IFX_int_t DTI_GEN_RegisterLock(
                        DTI_DeviceCtx_t      *pDtiDevCtx,
                        DTI_H2D_DeviceLock_t *pInLock,
                        DTI_D2H_DeviceLock_t *pOutLock,
                        IFX_int_t            lineNum,
                        DTI_PacketError_t    *pPacketError)
{
   DTI_PRN_USR_DBG_NL(DTI_GEN, DTI_PRN_LEVEL_HIGH,
      ("WARNING: Generic Dev RegisterLock - not implemented."DTI_CRLF));

   pOutLock->lock = 0;
   *pPacketError = DTI_eErrUnknown;

   return IFX_SUCCESS;
}

/**
   Get a device register (MEI interface).

\param
   pDtiDevCtx     - points to the GENERIC Device context.
\param
   pInRegGet      - points to the DTI Host2Dev RegisterGet struct.
\param
   pOutRegGet     - points to the DTI Dev2Host RegisterGet struct.
\param
   lineNum        - line number.
\param
   pOutPaylSize_byte - return ptr, return the size of the read registers.
\param
   pPacketError   - return ptr, return the DTI Packet Error.

\return
   IFX_SUCCESS and returns the DTI Packet Error code.
   IFX_ERROR   currently no (only in case of non-DTI related errors).
*/
DTI_STATIC IFX_int_t DTI_GEN_RegisterGet(
                        DTI_DeviceCtx_t         *pDtiDevCtx,
                        DTI_H2D_RegisterGet_t   *pInRegGet,
                        DTI_D2H_RegisterGet_t   *pOutRegGet,
                        IFX_int_t               lineNum,
                        IFX_uint32_t            *pOutPaylSize_byte,
                        DTI_PacketError_t       *pPacketError)
{
   DTI_PRN_USR_DBG_NL(DTI_GEN, DTI_PRN_LEVEL_HIGH,
      ("WARNING: Generic Dev RegisterGet - not implemented."DTI_CRLF));

   *pPacketError = DTI_eErrUnknown;

   return IFX_SUCCESS;
}


/**
   Get a device register (MEI interface).

\param
   pDtiDevCtx        - points to the GENERIC Device context.
\param
   pInRegSet         - points to the DTI Host2Dev RegisterSet struct.
\param
   inRegSetSize_Byte - size of the given IN data [byte].
\param
   lineNum           - line number.
\param
   pPacketError      - return ptr, return the DTI Packet Error.

\return
   IFX_SUCCESS DTI Packet Error code is "no error".
   IFX_ERROR   if the register read operation fails.
*/
DTI_STATIC IFX_int_t DTI_GEN_RegisterSet(
                        DTI_DeviceCtx_t         *pDtiDevCtx,
                        DTI_H2D_RegisterSet_t   *pInRegSet,
                        IFX_int_t               inRegSetSize_Byte,
                        IFX_int_t               lineNum,
                        DTI_PacketError_t       *pPacketError)
{
   DTI_PRN_USR_DBG_NL(DTI_GEN, DTI_PRN_LEVEL_HIGH,
      ("WARNING: Generic Dev RegisterGet - not implemented."DTI_CRLF));

   *pPacketError = DTI_eErrUnknown;

   return IFX_SUCCESS;
}


/**
   Set a device configuration.

\param
   pDtiConCtx     - points to the DTI connection setup.
\param
   pDtiDevCtx     - points to the GENERIC Device context.
\param
   pInCfgSet      - points to the DTI Host2Dev ConfigSet struct.
\param
   pOutCfgSet     - points to the DTI Dev2Host ConfigSet struct.
\param
   lineNum        - line number.
\param
   pPacketError   - return ptr, return the DTI Packet Error.

\return
   IFX_SUCCESS and returns the DTI Packet Error code.
   IFX_ERROR   currently no (only in case of non-DTI related errors).

\remark
   Changes of the autonomous message handling may have influence to the
   connection settings (select wait time)
*/
DTI_STATIC IFX_int_t DTI_GEN_ConfigSet(
                        DTI_DeviceCtx_t            *pDtiDevCtx,
                        DTI_H2D_DeviceConfigSet_t  *pInCfgSet,
                        DTI_D2H_DeviceConfigSet_t  *pOutCfgSet,
                        IFX_int_t                  lineNum,
                        DTI_PacketError_t          *pPacketError)
{
   DTI_MemSet(pOutCfgSet, 0x00, sizeof(DTI_D2H_DeviceConfigSet_t));
   pOutCfgSet->key = pInCfgSet->key;

   switch (pInCfgSet->key)
   {
      case DTI_eTimeout:
         DTI_PRN_USR_DBG_NL(DTI_GEN, DTI_PRN_LEVEL_HIGH,
            ("WARNING: Generic Dev ConfigSet Timeout - not implemented."DTI_CRLF));

         pOutCfgSet->value = 0;
         *pPacketError = DTI_eErrUnknown;
         break;

      case DTI_eAutonousMessages:
         pDtiDevCtx->bAutoDevMsgActive = IFX_TRUE;

         DTI_PRN_USR_DBG_NL(DTI_GEN, DTI_PRN_LEVEL_HIGH,
            ("WARNING: Generic Dev ConfigSet AutoMsg - not implemented."DTI_CRLF));

         pOutCfgSet->value = 0;
         *pPacketError = DTI_eErrUnknown;
         break;

      case DTI_eMaxRegAccess:
         DTI_PRN_USR_DBG_NL(DTI_GEN, DTI_PRN_LEVEL_HIGH,
            ("WARNING: Generic Dev ConfigSet MaxRegAccess - not implemented."DTI_CRLF));

         pOutCfgSet->value = 0;
         *pPacketError = DTI_eErrUnknown;
         break;

      case DTI_eMaxDebugAccess:
         DTI_PRN_USR_DBG_NL(DTI_GEN, DTI_PRN_LEVEL_HIGH,
            ("WARNING: Generic Dev ConfigSet MaxDbgAccess - not implemented."DTI_CRLF));

         pOutCfgSet->value = 0;
         *pPacketError = DTI_eErrUnknown;
         break;


      /* R/O */
      case DTI_eMailboxSize:
      default:
         pOutCfgSet->value = 0;
         *pPacketError = DTI_eErrUnknown;
         break;
   }

   return IFX_SUCCESS;
}


/**
   Get the device configuration of the given line.

\param
   pDtiDevCtx     - points to the GENERIC Device context.
\param
   pInCfgSet      - points to the DTI Host2Dev ConfigSet struct.
\param
   pOutCfgSet     - points to the DTI Dev2Host ConfigSet struct.
\param
   lineNum        - line number.
\param
   pPacketError   - return ptr, return the DTI Packet Error.

\return
   IFX_SUCCESS and returns the DTI Packet Error code.
   IFX_ERROR   currently no (only in case of non-DTI related errors).
*/
DTI_STATIC IFX_int_t DTI_GEN_ConfigGet(
                        DTI_DeviceCtx_t            *pDtiDevCtx,
                        DTI_H2D_DeviceConfigGet_t  *pInCfgGet,
                        DTI_D2H_DeviceConfigGet_t  *pOutCfgGet,
                        IFX_int_t                  lineNum,
                        DTI_PacketError_t          *pPacketError)
{
   DTI_MemSet(pOutCfgGet, 0x00, sizeof(DTI_D2H_DeviceConfigGet_t));
   pOutCfgGet->key = pInCfgGet->key;

   switch (pInCfgGet->key)
   {
      case DTI_eTimeout:
         DTI_PRN_USR_DBG_NL(DTI_GEN, DTI_PRN_LEVEL_HIGH,
            ("WARNING: Generic Dev ConfigGet Timeout - not implemented."DTI_CRLF));

         *pPacketError = DTI_eErrUnknown;
         break;

      case DTI_eAutonousMessages:
         DTI_PRN_USR_DBG_NL(DTI_GEN, DTI_PRN_LEVEL_HIGH,
            ("WARNING: Generic Dev ConfigGet AutoMsg - not implemented."DTI_CRLF));

         *pPacketError = DTI_eErrUnknown;
         break;

      case DTI_eMailboxSize:
         DTI_PRN_USR_DBG_NL(DTI_GEN, DTI_PRN_LEVEL_HIGH,
            ("WARNING: Generic Dev ConfigGet MBoxSize - not implemented."DTI_CRLF));

         *pPacketError = DTI_eErrUnknown;
         break;

      case DTI_eMaxRegAccess:
         DTI_PRN_USR_DBG_NL(DTI_GEN, DTI_PRN_LEVEL_HIGH,
            ("WARNING: Generic Dev ConfigGet MaxRegAccess - not implemented."DTI_CRLF));

         *pPacketError = DTI_eErrUnknown;
         break;

      case DTI_eMaxDebugAccess:
         DTI_PRN_USR_DBG_NL(DTI_GEN, DTI_PRN_LEVEL_HIGH,
            ("WARNING: Generic Dev ConfigGet MaxDbgAccess - not implemented."DTI_CRLF));

         *pPacketError = DTI_eErrUnknown;
         break;

      default:
         *pPacketError = DTI_eErrUnknown;
         break;
   }

   return IFX_SUCCESS;
}

/**
   Send a 8 Bit message to the device and wait for the responce.

\param
   pDtiDevCtx        - points to the GENERIC Device context.
\param
   pInMsg8Send      - points to the DTI 8 bit message send struct.
\param
   pOutMsg8Send     - points to the DTI 8 bit message responce struct.
\param
   lineNum           - line number.
\param
   inPaylSize_byte   - payload size of the DTI packet [byte].
\param
   pOutPaylSize_byte - return ptr, return the OUT payload size [byte].
\param
   pPacketError      - return ptr, return the DTI Packet Error.

\return
   IFX_SUCCESS and returns the DTI Packet Error code.
   IFX_ERROR   currently no (only in case of non-DTI related errors).
*/
DTI_STATIC IFX_int_t DTI_GEN_Message8Send(
                        DTI_DeviceCtx_t      *pDtiDevCtx,
                        DTI_H2D_Message8_u   *pInMsg8Send,
                        DTI_D2H_Message8_u   *pOutMsg8Send,
                        IFX_int_t            lineNum,
                        IFX_int_t            inPaylSize_byte,
                        IFX_int_t            *pOutPaylSize_byte,
                        DTI_PacketError_t    *pPacketError)
{
   DTI_PRN_USR_DBG_NL(DTI_GEN, DTI_PRN_LEVEL_HIGH,
      ("WARNING: Generic Dev Message8Send - not implemented."DTI_CRLF));

   *pPacketError = DTI_eErrUnknown;

   return IFX_SUCCESS;
}



/**
   Send a 16 Bit message to the device and wait for the responce.

\param
   pDtiDevCtx        - points to the GENERIC Device context.
\param
   pInMsg16Send      - points to the DTI 16 bit message send struct.
\param
   pOutMsg16Send     - points to the DTI 16 bit message responce struct.
\param
   lineNum           - line number.
\param
   inPaylSize_byte   - payload size of the DTI packet [byte].
\param
   pOutPaylSize_byte - return ptr, return the OUT payload size [byte].
\param
   pPacketError      - return ptr, return the DTI Packet Error.

\return
   IFX_SUCCESS and returns the DTI Packet Error code.
   IFX_ERROR   currently no (only in case of non-DTI related errors).
*/
DTI_STATIC IFX_int_t DTI_GEN_Message16Send(
                        DTI_DeviceCtx_t      *pDtiDevCtx,
                        DTI_H2D_Message16_u  *pInMsg16Send,
                        DTI_D2H_Message16_u  *pOutMsg16Send,
                        IFX_int_t            lineNum,
                        IFX_int_t            inPaylSize_byte,
                        IFX_int_t            *pOutPaylSize_byte,
                        DTI_PacketError_t    *pPacketError)
{
   DTI_PRN_USR_DBG_NL(DTI_GEN, DTI_PRN_LEVEL_HIGH,
      ("WARNING: Generic Dev Message16Send - not implemented."DTI_CRLF));

   *pPacketError = DTI_eErrUnknown;

   return IFX_SUCCESS;
}


/**
   Send a 32 Bit message to the device and wait for the responce.

\param
   pDtiDevCtx        - points to the GENERIC Device context.
\param
   pInMsg32Send      - points to the DTI 32 bit message send struct.
\param
   pOutMsg32Send     - points to the DTI 32 bit message responce struct.
\param
   lineNum           - line number.
\param
   inPaylSize_byte   - payload size of the DTI packet [byte].
\param
   pOutPaylSize_byte - return ptr, return the OUT payload size.
\param
   pPacketError      - return ptr, return the DTI Packet Error.

\return
   IFX_SUCCESS and returns the DTI Packet Error code.
   IFX_ERROR   currently no (only in case of non-DTI related errors).
*/
DTI_STATIC IFX_int_t DTI_GEN_Message32Send(
                        DTI_DeviceCtx_t      *pDtiDevCtx,
                        DTI_H2D_Message32_u  *pInMsg32Send,
                        DTI_D2H_Message32_u  *pOutMsg32Send,
                        IFX_int_t            lineNum,
                        IFX_int_t            inPaylSize_byte,
                        IFX_int_t            *pOutPaylSize_byte,
                        DTI_PacketError_t    *pPacketError)
{
   DTI_PRN_USR_DBG_NL(DTI_GEN, DTI_PRN_LEVEL_HIGH,
      ("WARNING: Generic Dev Message32Send - not implemented."DTI_CRLF));

   *pPacketError = DTI_eErrUnknown;

   return IFX_SUCCESS;
}

/**
   Setup the trace buffer configuration (GENERIC Rev2 debug streams).

\remark
   This function releases current configuration if the Debug Streams are
   already configured before the new config is set.

\param
   pDtiDevCtx           - points to the GENERIC Device context.
\param
   pInTraceConfigSet    - points to the DTI Host2Dev TraceConfigSet struct.
\param
   pOutTraceConfigSet   - points to the DTI Dev2Host TraceConfigSet struct.
\param
   lineNum              - line number.
\param
   pPacketError         - return ptr, return the DTI Packet Error.

\return
   IFX_SUCCESS and returns the DTI Packet Error code.
   IFX_ERROR   debug stream setup failed.
*/
DTI_STATIC IFX_int_t DTI_GEN_TraceBufferConfigSet(
                        DTI_DeviceCtx_t            *pDtiDevCtx,
                        DTI_H2D_TraceConfigSet_t   *pInTraceConfigSet,
                        DTI_D2H_TraceConfigSet_t   *pOutTraceConfigSet,
                        IFX_int_t                  lineNum,
                        DTI_PacketError_t          *pPacketError)
{
   DTI_PRN_USR_DBG_NL(DTI_GEN, DTI_PRN_LEVEL_HIGH,
      ("WARNING: Generic Dev TraceBufferConfigSet - not implemented."DTI_CRLF));

   pOutTraceConfigSet->size = 0;
   *pPacketError = DTI_eErrUnknown;

   return IFX_SUCCESS;
}


/**
   Reset the current trace buffer (GENERIC Rev2 debug streams).

\param
   pDtiDevCtx     - points to the GENERIC Device context.
\param
   lineNum        - line number.
\param
   pPacketError   - return ptr, return the DTI Packet Error.

\return
   IFX_SUCCESS and returns the DTI Packet Error code.
   IFX_ERROR   debug stream setup failed.
*/
DTI_STATIC IFX_int_t DTI_GEN_TraceBufferReset(
                        DTI_DeviceCtx_t      *pDtiDevCtx,
                        IFX_int_t            lineNum,
                        DTI_PacketError_t    *pPacketError)
{
   DTI_PRN_USR_DBG_NL(DTI_GEN, DTI_PRN_LEVEL_HIGH,
      ("WARNING: Generic Dev TraceBufferReset - not implemented."DTI_CRLF));

   *pPacketError = DTI_eErrUnknown;

   return IFX_SUCCESS;
}

/**
   Setup the trace buffer configuration (GENERIC Rev2 debug streams).

\param
   pDtiDevCtx           - points to the GENERIC Device context.
\param
   pOutTraceStatusGet   - points to the DTI Dev2Host TraceStatusGet struct.
\param
   lineNum              - line number.
\param
   pPacketError         - return ptr, return the DTI Packet Error.

\return
   IFX_SUCCESS and returns the DTI Packet Error code.
   IFX_ERROR   debug stream config request failed.
*/
DTI_STATIC IFX_int_t DTI_GEN_TraceBufferStatusGet(
                        DTI_DeviceCtx_t            *pDtiDevCtx,
                        DTI_D2H_TraceStatusGet_t   *pOutTraceStatusGet,
                        IFX_int_t                  lineNum,
                        DTI_PacketError_t          *pPacketError)
{
   DTI_PRN_USR_DBG_NL(DTI_GEN, DTI_PRN_LEVEL_HIGH,
      ("WARNING: Generic Dev TraceBufferStatusGet - not implemented."DTI_CRLF));

   pOutTraceStatusGet->mode = 0;
   pOutTraceStatusGet->size = 0;
   pOutTraceStatusGet->fill = 0;

   *pPacketError = DTI_eErrUnknown;

   return IFX_SUCCESS;
}

/**
   Read data from the device via Debug Access (MEI Debug).

\param
   pDtiDevCtx           - points to the GENERIC Device context.
\param
   pInTraceBufferGet    - points to the DTI Host2Dev RegisterGet struct.
\param
   ppUsedDtiPacketOut   - return ptr, points to the DTI Out packet.
\param
   pUsedBufferOutSize   - return value, points to the DTI Out packet size.
\param
   lineNum              - line number.
\param
   pTrBufReadSize_byte  - return ptr, return the number of read registers.
\param
   pPacketError         - return ptr, return the DTI Packet Error.

\remark
   The function is called with the standard out packet. If the requested size
   does not fit into this buffer, a corresponding buffer is allocated and used
   for the ioctl call. The pointer to this out packet will be returned.

\return
   IFX_SUCCESS  debug stream data read successful.
      - returns the DTI Packet Error code.
      - pointer to the used out package.
      - size of the used out package.
   IFX_ERROR   data read failed.
*/
DTI_STATIC IFX_int_t DTI_GEN_TraceBufferGet(
                        DTI_DeviceCtx_t            *pDtiDevCtx,
                        DTI_H2D_TraceBufferGet_t   *pInTraceBufferGet,
                        DTI_Packet_t               **ppUsedDtiPacketOut,
                        IFX_int_t                  *pUsedBufferOutSize,
                        IFX_int_t                  lineNum,
                        IFX_int_t                  *pTrBufReadSize_byte,
                        DTI_PacketError_t          *pPacketError)
{
   DTI_PRN_USR_DBG_NL(DTI_GEN, DTI_PRN_LEVEL_HIGH,
      ("WARNING: Generic Dev TraceBufferGet - not implemented."DTI_CRLF));

   *pTrBufReadSize_byte = 0;
   *pPacketError = DTI_eErrUnknown;

   return IFX_SUCCESS;
}

/**
   Read data from the device via Debug Access (MEI Debug).

\param
   pDtiDevCtx     - points to the GENERIC Device context.
\param
   pInDbgGet      - points to the DTI Host2Dev RegisterGet struct.
\param
   pOutDbgGet     - points to the DTI Dev2Host RegisterGet struct.
\param
   lineNum        - line number.
\param
   pDbgReadCount  - return ptr, return the number of read registers.
\param
   pPacketError   - return ptr, return the DTI Packet Error.

\return
   IFX_SUCCESS and returns the DTI Packet Error code.
   IFX_ERROR   currently no (only in case of non-DTI related errors).
*/
DTI_STATIC IFX_int_t DTI_GEN_DebugRead(
                        DTI_DeviceCtx_t      *pDtiDevCtx,
                        DTI_H2D_DebugRead_t  *pInDbgGet,
                        DTI_D2H_DebugRead_t  *pOutDbgGet,
                        IFX_int_t            lineNum,
                        IFX_int_t            *pDbgReadCount,
                        DTI_PacketError_t    *pPacketError)
{
   DTI_PRN_USR_DBG_NL(DTI_GEN, DTI_PRN_LEVEL_HIGH,
      ("WARNING: Generic Dev DebugRead - not implemented."DTI_CRLF));

   *pDbgReadCount = 0;
   *pPacketError = DTI_eErrUnknown;

   return IFX_SUCCESS;
}


/**
   Write data to the device via Debug Access (MEI Debug).

\param
   pDtiDevCtx     - points to the GENERIC Device context.
\param
   pInDbgSet      - points to the DTI Host2Dev RegisterGet struct.
\param
   pOutDbgGet_nU  - not used.
\param
   lineNum        - line number.
\param
   pDbgWriteCount - return ptr, return the number of read registers.
\param
   pPacketError   - return ptr, return the DTI Packet Error.

\return
   IFX_SUCCESS and returns the DTI Packet Error code.
   IFX_ERROR   currently no (only in case of non-DTI related errors).
*/
DTI_STATIC IFX_int_t DTI_GEN_DebugWrite(
                        DTI_DeviceCtx_t      *pDtiDevCtx,
                        DTI_H2D_DebugWrite_t *pInDbgSet,
                        IFX_uint32_t         *pOutDbgGet_nU,
                        IFX_int_t            lineNum,
                        IFX_int_t            *pDbgWriteCount,
                        DTI_PacketError_t    *pPacketError)
{
   DTI_PRN_USR_DBG_NL(DTI_GEN, DTI_PRN_LEVEL_HIGH,
      ("WARNING: Generic Dev DebugRead - not implemented."DTI_CRLF));

   *pDbgWriteCount = 0;
   *pPacketError = DTI_eErrUnknown;

   return IFX_SUCCESS;
}


/**
   Check and process for Device Autonomous Messages.
   The Auto Msg is read form the device (driver) and a corresponding
   DTI packet is created and sent to the upper DTI Client

\param
   pDtiDevCtx           - points to the Device context.
\param
   pDtiCon              - points to the established DTI Connection data.
\param
   devSelectWait_ms     - wait time [ms] for the device select operation.
\param
   pOutBuffer           - points to the DTI packet out buffer to create a DTI packet.
\param
   outBufferSize_byte   - DTI out buffer size [byte]

\return
   IFX_SUCCESS if the DTI packet has been sent.
   IFX_ERROR   DTI packet has not been sent.

\remarks
   At first the device msg will be read out to free the internal driver buffer.
   This is done independant on the established connection.
*/
DTI_STATIC IFX_int_t DTI_GEN_AutoMsgProcess(
                        DTI_DeviceCtx_t         *pDtiDevCtx,
                        const DTI_Connection_t  *pDtiCon,
                        IFX_uint32_t            devSelectWait_ms,
                        IFX_char_t              *pOutBuffer,
                        IFX_int_t               outBufferSize_byte)
{
   DTI_PRN_USR_DBG_NL(DTI_GEN, DTI_PRN_LEVEL_HIGH,
      ("WARNING: Generic Dev AutoMsgProcess - not implemented."DTI_CRLF));

   return IFX_SUCCESS;
}

DTI_STATIC IFX_int_t DTI_GEN_WinEasyCiAccess(
                        DTI_DeviceCtx_t      *pDtiDevCtx,
                        IFX_int_t            lineNum,
                        const IFX_uint8_t    *pDataIn,
                        const IFX_uint32_t   sizeIn,
                        IFX_uint8_t          *pDataOut,
                        const IFX_uint32_t   sizeOut,
                        DTI_PacketError_t    *pPacketError)
{
   DTI_PRN_USR_DBG_NL(DTI_GEN, DTI_PRN_LEVEL_HIGH,
      ("WARNING: Generic Dev WinEasyCiAccess - not implemented."DTI_CRLF));

   *pPacketError = DTI_eErrUnknown;
   return IFX_SUCCESS;
}



#endif /* #if defined(DEVICE_GENERIC) */

