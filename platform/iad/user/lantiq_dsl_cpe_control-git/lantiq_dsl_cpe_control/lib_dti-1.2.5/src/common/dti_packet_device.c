/******************************************************************************

                              Copyright (c) 2009
                            Lantiq Deutschland GmbH
                     Am Campeon 3; 85579 Neubiberg, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/** \file
   Debug and Trace Interface - Basic Protocol Packet Handling.
*/

/* ============================================================================
   Includes
   ========================================================================= */
#include "dti_osmap.h"


#include "ifx_dti_protocol.h"
#include "ifx_dti_protocol_device.h"
#include "dti_protocol_interface.h"

#include "dti_device.h"
#include "dti_packet_device.h"


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


/* ============================================================================
   Local Function Declaration
   ========================================================================= */

DTI_STATIC IFX_int_t DTI_packet_DeviceReset(
                        DTI_DeviceCtx_t         *pDtiDevCtx,
                        const DTI_Packet_t      *pDtiPacketIn,
                        DTI_Packet_t            *pDtiPacketOut,
                        IFX_uint32_t            bufferOutSize);

DTI_STATIC IFX_int_t DTI_packet_DeviceDownload(
                        DTI_DeviceCtx_t         *pDtiDevCtx,
                        DTI_ProtocolServerCtx_t *pDtiProtServerCtx,
                        const DTI_Packet_t      *pDtiPacketIn,
                        DTI_Packet_t            *pDtiPacketOut,
                        IFX_uint32_t            bufferOutSize);

DTI_STATIC IFX_int_t DTI_packet_DeviceOpen(
                        DTI_DeviceCtx_t         *pDtiDevCtx,
                        const DTI_Packet_t      *pDtiPacketIn,
                        DTI_Packet_t            *pDtiPacketOut,
                        IFX_uint32_t            bufferOutSize);

DTI_STATIC IFX_int_t DTI_packet_DeviceClose(
                        DTI_DeviceCtx_t         *pDtiDevCtx,
                        const DTI_Packet_t      *pDtiPacketIn,
                        DTI_Packet_t            *pDtiPacketOut,
                        IFX_uint32_t            bufferOutSize);

DTI_STATIC IFX_int_t DTI_packet_RegisterLock(
                        DTI_DeviceCtx_t         *pDtiDevCtx,
                        const DTI_Packet_t      *pDtiPacketIn,
                        DTI_Packet_t            *pDtiPacketOut,
                        IFX_uint32_t            bufferOutSize);

DTI_STATIC IFX_int_t DTI_packet_RegisterGet(
                        DTI_DeviceCtx_t         *pDtiDevCtx,
                        const DTI_Packet_t      *pDtiPacketIn,
                        DTI_Packet_t            *pDtiPacketOut,
                        IFX_uint32_t            bufferOutSize);

DTI_STATIC IFX_int_t DTI_packet_RegisterSet(
                        DTI_DeviceCtx_t         *pDtiDevCtx,
                        const DTI_Packet_t      *pDtiPacketIn,
                        DTI_Packet_t            *pDtiPacketOut,
                        IFX_uint32_t            bufferOutSize);

DTI_STATIC IFX_int_t DTI_packet_ConfigSet(
                        DTI_DeviceCtx_t         *pDtiDevCtx,
                        const DTI_Packet_t      *pDtiPacketIn,
                        DTI_Packet_t            *pDtiPacketOut,
                        IFX_uint32_t            bufferOutSize);

DTI_STATIC IFX_int_t DTI_packet_ConfigGet(
                        DTI_DeviceCtx_t         *pDtiDevCtx,
                        const DTI_Packet_t      *pDtiPacketIn,
                        DTI_Packet_t            *pDtiPacketOut,
                        IFX_uint32_t            bufferOutSize);

DTI_STATIC IFX_int_t DTI_packet_MessageSend(
                        DTI_DeviceCtx_t         *pDtiDevCtx,
                        const DTI_Packet_t      *pDtiPacketIn,
                        DTI_Packet_t            *pDtiPacketOut,
                        IFX_uint32_t            bufferOutSize);

DTI_STATIC IFX_int_t DTI_packet_TraceBufferConfigSet(
                        DTI_DeviceCtx_t         *pDtiDevCtx,
                        const DTI_Packet_t      *pDtiPacketIn,
                        DTI_Packet_t            *pDtiPacketOut,
                        IFX_uint32_t            bufferOutSize);

DTI_STATIC IFX_int_t DTI_packet_TraceBufferReset(
                        DTI_DeviceCtx_t         *pDtiDevCtx,
                        const DTI_Packet_t      *pDtiPacketIn,
                        DTI_Packet_t            *pDtiPacketOut,
                        IFX_uint32_t            bufferOutSize);

DTI_STATIC IFX_int_t DTI_packet_TraceBufferStatusGet(
                        DTI_DeviceCtx_t         *pDtiDevCtx,
                        const DTI_Packet_t      *pDtiPacketIn,
                        DTI_Packet_t            *pDtiPacketOut,
                        IFX_uint32_t            bufferOutSize);

DTI_STATIC IFX_int_t DTI_packet_TraceBufferGet(
                        DTI_DeviceCtx_t         *pDtiDevCtx,
                        const DTI_Packet_t      *pDtiPacketIn,
                        DTI_Packet_t            **ppDtiPacketOut,
                        IFX_uint32_t            *pBufferOutSize);

DTI_STATIC IFX_int_t DTI_packet_DebugGet(
                        DTI_DeviceCtx_t         *pDtiDevCtx,
                        const DTI_Packet_t      *pDtiPacketIn,
                        DTI_Packet_t            *pDtiPacketOut,
                        IFX_uint32_t            bufferOutSize);

DTI_STATIC IFX_int_t DTI_packet_DebugSet(
                        DTI_DeviceCtx_t         *pDtiDevCtx,
                        const DTI_Packet_t      *pDtiPacketIn,
                        DTI_Packet_t            *pDtiPacketOut,
                        IFX_uint32_t            bufferOutSize);

DTI_STATIC IFX_int_t DTI_packet_WinEasyCiAccess(
                        DTI_DeviceCtx_t         *pDtiDevCtx,
                        const DTI_Packet_t      *pDtiPacketIn,
                        DTI_Packet_t            *pDtiPacketOut,
                        IFX_uint32_t            bufferOutSize);

/* ============================================================================
   Variables
   ========================================================================= */
IFXOS_PRN_USR_MODULE_CREATE(DTI_DEV, DTI_PRN_LEVEL_HIGH);

/* ============================================================================
   Local Functions - packet processing
   ========================================================================= */

/**
   Process the packet "DTI_PacketType_eDeviceReset"

\param
   pDtiDevCtx        - points to the Device context.
\param
   pDtiPacketIn      - points to the IN packet.
\param
   pDtiPacketOut     - points to the OUT packet.
\param
   bufferOutSize     - size of the out buffer.

\return
   IFX_SUCCESS reset done and the responce packet is set.
   IFX_ERROR   currently no (only in case of non-DTI related errors).
*/
DTI_STATIC IFX_int_t DTI_packet_DeviceReset(
                        DTI_DeviceCtx_t         *pDtiDevCtx,
                        const DTI_Packet_t      *pDtiPacketIn,
                        DTI_Packet_t            *pDtiPacketOut,
                        IFX_uint32_t            bufferOutSize)
{
   DTI_PacketError_t       packetError = DTI_eNoError;
   DTI_H2D_DeviceReset_t   *pDataIn;
   DTI_D2H_DeviceReset_t   *pDataOut;
   DTI_PTR_U               uPayload;
   DTI_DeviceAccessFct_t   *pDeviceAccessFct = pDtiDevCtx->pDeviceAccessFct;

   if (pDeviceAccessFct->pResetFct != IFX_NULL)
   {
      uPayload.pUInt8 = (IFX_uint8_t *)pDtiPacketIn->payload;
      pDataIn = (DTI_H2D_DeviceReset_t *)DTI_PTR_CAST_GET_ULONG(uPayload);

      uPayload.pUInt8 = pDtiPacketOut->payload;
      pDataOut = (DTI_D2H_DeviceReset_t *)DTI_PTR_CAST_GET_ULONG(uPayload);

      if ((pDataIn == IFX_NULL) || (pDataOut == IFX_NULL))
      {
         DTI_PRN_USR_ERR_NL(DTI_DEV, DTI_PRN_LEVEL_ERR,
            ("Error: DTI Packet Dev Reset - missaligned payload."DTI_CRLF));

         packetError = DTI_eErrMalformedPacket;
      }
      else
      {
         (void)pDeviceAccessFct->pResetFct(
                  pDtiDevCtx, pDataIn, pDataOut,
                  (IFX_int_t)pDtiPacketIn->header.payloadSize / sizeof(IFX_uint32_t),
                  &packetError);
      }
   }
   else
   {
      DTI_PRN_USR_ERR_NL(DTI_DEV, DTI_PRN_LEVEL_ERR,
         ("Error: DTI Packet Dev[%s] Reset - not implemented."DTI_CRLF,
           (pDeviceAccessFct->pDevIfName) ? pDeviceAccessFct->pDevIfName : "unknown"));

      packetError = DTI_eErrConfiguration;
   }

   (void)DTI_packetResponceSet(
            pDtiPacketIn, pDtiPacketOut,
            packetError, pDtiPacketIn->header.payloadSize,
            bufferOutSize, IFX_FALSE);

   return IFX_SUCCESS;
}

/**
   Process the packet "DTI_PacketType_eDeviceDownload"

\param
   pDtiDevCtx        - points to the Device context.
\param
   pDtiProtServerCtx - points to the DTI protocol server context.
\param
   pDtiPacketIn      - points to the IN packet.
\param
   pDtiPacketOut     - points to the OUT packet.
\param
   bufferOutSize     - size of the out buffer.

\return
   IFX_SUCCESS reset done and the responce packet is set.
   IFX_ERROR   currently no (only in case of non-DTI related errors).
*/
DTI_STATIC IFX_int_t DTI_packet_DeviceDownload(
                        DTI_DeviceCtx_t         *pDtiDevCtx,
                        DTI_ProtocolServerCtx_t *pDtiProtServerCtx,
                        const DTI_Packet_t      *pDtiPacketIn,
                        DTI_Packet_t            *pDtiPacketOut,
                        IFX_uint32_t            bufferOutSize)
{
   DTI_PacketError_t          packetError = DTI_eNoError;
   DTI_H2D_DeviceDownload_t   *pDataIn;
   DTI_D2H_DeviceDownload_t   *pDataOut;
   DTI_PTR_U                  uPayload;
   DTI_DeviceAccessFct_t      *pDeviceAccessFct = pDtiDevCtx->pDeviceAccessFct;

   if (pDeviceAccessFct->pDownloadFct != IFX_NULL)
   {
      uPayload.pUInt8 = (IFX_uint8_t *)pDtiPacketIn->payload;
      pDataIn = (DTI_H2D_DeviceDownload_t *)DTI_PTR_CAST_GET_ULONG(uPayload);

      uPayload.pUInt8 = pDtiPacketOut->payload;
      pDataOut = (DTI_D2H_DeviceDownload_t *)DTI_PTR_CAST_GET_ULONG(uPayload);

      if ((pDataIn == IFX_NULL) || (pDataOut == IFX_NULL))
      {
         DTI_PRN_USR_ERR_NL(DTI_DEV, DTI_PRN_LEVEL_ERR,
            ("Error: DTI Packet Dev Download - missaligned payload."DTI_CRLF));

         packetError = DTI_eErrMalformedPacket;
      }
      else
      {
         (void)pDeviceAccessFct->pDownloadFct(
                                    pDtiDevCtx, pDtiProtServerCtx,
                                    pDataIn, pDataOut, &packetError);
      }
   }
   else
   {
      DTI_PRN_USR_ERR_NL(DTI_DEV, DTI_PRN_LEVEL_ERR,
         ("Error: DTI Packet Dev[%s] Download - not implemented."DTI_CRLF,
           (pDeviceAccessFct->pDevIfName) ? pDeviceAccessFct->pDevIfName : "unknown"));

      packetError = DTI_eErrConfiguration;
   }

   (void)DTI_packetResponceSet(
            pDtiPacketIn, pDtiPacketOut,
            packetError, sizeof(DTI_D2H_DeviceDownload_t),
            bufferOutSize, IFX_FALSE);

   return IFX_SUCCESS;
}

/**
   Process the packet "DTI_PacketType_eDeviceOpen"

\param
   pDtiDevCtx        - points to the Device context.
\param
   pDtiPacketIn      - points to the IN packet.
\param
   pDtiPacketOut     - points to the OUT packet.
\param
   bufferOutSize     - size of the out buffer.

\return
   IFX_SUCCESS open done.
   IFX_ERROR   error while open given device.
*/
DTI_STATIC IFX_int_t DTI_packet_DeviceOpen(
                        DTI_DeviceCtx_t         *pDtiDevCtx,
                        const DTI_Packet_t      *pDtiPacketIn,
                        DTI_Packet_t            *pDtiPacketOut,
                        IFX_uint32_t            bufferOutSize)
{
   DTI_PacketError_t       packetError = DTI_eNoError;
   DTI_DeviceAccessFct_t   *pDeviceAccessFct = pDtiDevCtx->pDeviceAccessFct;

   if (pDeviceAccessFct->pDeviceOpenFct != IFX_NULL)
   {
      (void)pDeviceAccessFct->pDeviceOpenFct(
               pDtiDevCtx,
               (IFX_int_t)DTI_HDR_PORT_PORT_NUM_GET(pDtiPacketIn->header.port),
               &packetError);
   }
   else
   {
      DTI_PRN_USR_ERR_NL(DTI_DEV, DTI_PRN_LEVEL_ERR,
         ("Error: DTI Packet Dev[%s] Open - not implemented."DTI_CRLF,
           (pDeviceAccessFct->pDevIfName) ? pDeviceAccessFct->pDevIfName : "unknown"));

      packetError = DTI_eErrConfiguration;
   }

   (void)DTI_packetResponceSet(
            pDtiPacketIn, pDtiPacketOut,
            packetError, 0,
            bufferOutSize, IFX_FALSE);

   return IFX_SUCCESS;
}

/**
   Process the packet "DTI_PacketType_eDeviceClose"

\param
   pDtiDevCtx        - points to the Device context.
\param
   pDtiPacketIn      - points to the IN packet.
\param
   pDtiPacketOut     - points to the OUT packet.
\param
   bufferOutSize     - size of the out buffer.

\return
   IFX_SUCCESS open done.
   IFX_ERROR   error while open given device.
*/
DTI_STATIC IFX_int_t DTI_packet_DeviceClose(
                        DTI_DeviceCtx_t         *pDtiDevCtx,
                        const DTI_Packet_t      *pDtiPacketIn,
                        DTI_Packet_t            *pDtiPacketOut,
                        IFX_uint32_t            bufferOutSize)
{
   DTI_PacketError_t       packetError = DTI_eNoError;
   DTI_DeviceAccessFct_t   *pDeviceAccessFct = pDtiDevCtx->pDeviceAccessFct;

   if (pDeviceAccessFct->pDeviceCloseFct != IFX_NULL)
   {
      (void)pDeviceAccessFct->pDeviceCloseFct(
               pDtiDevCtx,
               (IFX_int_t)DTI_HDR_PORT_PORT_NUM_GET(pDtiPacketIn->header.port),
               &packetError);
   }
   else
   {
      DTI_PRN_USR_ERR_NL(DTI_DEV, DTI_PRN_LEVEL_ERR,
         ("Error: DTI Packet Dev[%s] Close - not implemented."DTI_CRLF,
           (pDeviceAccessFct->pDevIfName) ? pDeviceAccessFct->pDevIfName : "unknown"));

      packetError = DTI_eErrConfiguration;
   }

   (void)DTI_packetResponceSet(
            pDtiPacketIn, pDtiPacketOut,
            packetError, 0,
            bufferOutSize, IFX_FALSE);

   return IFX_SUCCESS;
}

/**
   Process the packet "DTI_PacketType_eRegisterLock"

\param
   pDtiDevCtx        - points to the Device context.
\param
   pDtiPacketIn      - points to the IN packet.
\param
   pDtiPacketOut     - points to the OUT packet.
\param
   bufferOutSize     - size of the out buffer.

\return
   IFX_SUCCESS reset done and the responce packet is set.
   IFX_ERROR   currently no (only in case of non-DTI related errors).
*/
DTI_STATIC IFX_int_t DTI_packet_RegisterLock(
                        DTI_DeviceCtx_t         *pDtiDevCtx,
                        const DTI_Packet_t      *pDtiPacketIn,
                        DTI_Packet_t            *pDtiPacketOut,
                        IFX_uint32_t            bufferOutSize)
{
   DTI_PacketError_t       packetError = DTI_eNoError;
   DTI_H2D_DeviceLock_t    *pDataIn;
   DTI_D2H_DeviceLock_t    *pDataOut;
   DTI_PTR_U               uPayload;
   DTI_DeviceAccessFct_t   *pDeviceAccessFct = pDtiDevCtx->pDeviceAccessFct;

   if (pDeviceAccessFct->pRegisterLockFct != IFX_NULL)
   {
      uPayload.pUInt8 = (IFX_uint8_t *)pDtiPacketIn->payload;
      pDataIn = (DTI_H2D_DeviceLock_t *)DTI_PTR_CAST_GET_ULONG(uPayload);

      uPayload.pUInt8 = pDtiPacketOut->payload;
      pDataOut = (DTI_D2H_DeviceLock_t *)DTI_PTR_CAST_GET_ULONG(uPayload);

      if ((pDataIn == IFX_NULL) || (pDataOut == IFX_NULL))
      {
         DTI_PRN_USR_ERR_NL(DTI_DEV, DTI_PRN_LEVEL_ERR,
            ("Error: DTI Packet Dev Lock - missaligned payload."DTI_CRLF));

         packetError = DTI_eErrMalformedPacket;
      }
      else
      {
         (void)pDeviceAccessFct->pRegisterLockFct(
                  pDtiDevCtx, pDataIn, pDataOut,
                  (IFX_int_t)DTI_HDR_PORT_PORT_NUM_GET(pDtiPacketIn->header.port),
                  &packetError);
      }
   }
   else
   {
      DTI_PRN_USR_ERR_NL(DTI_DEV, DTI_PRN_LEVEL_ERR,
         ("Error: DTI Packet Dev[%s] Lock - not implemented."DTI_CRLF,
           (pDeviceAccessFct->pDevIfName) ? pDeviceAccessFct->pDevIfName : "unknown"));

      packetError = DTI_eErrConfiguration;
   }

   (void)DTI_packetResponceSet(
            pDtiPacketIn, pDtiPacketOut,
            packetError, sizeof(DTI_D2H_DeviceLock_t),
            bufferOutSize, IFX_FALSE);

   return IFX_SUCCESS;
}

/**
   Process the packet "DTI_PacketType_eRegisterGet"

\param
   pDtiDevCtx        - points to the Device context.
\param
   pDtiPacketIn      - points to the IN packet.
\param
   pDtiPacketOut     - points to the OUT packet.
\param
   bufferOutSize     - size of the out buffer.

\return
   IFX_SUCCESS reset done and the responce packet is set.
   IFX_ERROR   currently no (only in case of non-DTI related errors).
*/
DTI_STATIC IFX_int_t DTI_packet_RegisterGet(
                        DTI_DeviceCtx_t         *pDtiDevCtx,
                        const DTI_Packet_t      *pDtiPacketIn,
                        DTI_Packet_t            *pDtiPacketOut,
                        IFX_uint32_t            bufferOutSize)
{
   IFX_uint32_t            outPaylSize_byte = 0;
   DTI_PacketError_t       packetError = DTI_eNoError;
   DTI_H2D_RegisterGet_t   *pDataIn;
   DTI_D2H_RegisterGet_t   *pDataOut;
   DTI_PTR_U               uPayload;
   DTI_DeviceAccessFct_t   *pDeviceAccessFct = pDtiDevCtx->pDeviceAccessFct;

   if (pDeviceAccessFct->pRegisterGetFct != IFX_NULL)
   {
      uPayload.pUInt8 = (IFX_uint8_t *)pDtiPacketIn->payload;
      pDataIn = (DTI_H2D_RegisterGet_t *)DTI_PTR_CAST_GET_ULONG(uPayload);

      uPayload.pUInt8 = pDtiPacketOut->payload;
      pDataOut = (DTI_D2H_RegisterGet_t *)DTI_PTR_CAST_GET_ULONG(uPayload);

      if ((pDataIn == IFX_NULL) || (pDataOut == IFX_NULL))
      {
         DTI_PRN_USR_ERR_NL(DTI_DEV, DTI_PRN_LEVEL_ERR,
            ("Error: DTI Packet RegisterGet - missaligned payload."DTI_CRLF));

         packetError = DTI_eErrMalformedPacket;
      }
      else
      {
         if ( (sizeof(DTI_PacketHeader_t) + (pDataIn->count * sizeof(IFX_uint32_t))) > bufferOutSize)
         {
            DTI_PRN_USR_ERR_NL(DTI_DEV, DTI_PRN_LEVEL_ERR,
               ("Error: DTI Packet RegisterGet - count = %d > max = %d (buffer size)."DTI_CRLF,
                 pDataIn->count, (bufferOutSize - sizeof(DTI_PacketHeader_t)) / sizeof(IFX_uint32_t) ));

            packetError = DTI_eErrConfiguration;
         }
         else
         {
            (void)pDeviceAccessFct->pRegisterGetFct(
                     pDtiDevCtx, pDataIn, pDataOut,
                     (IFX_int_t)DTI_HDR_PORT_PORT_NUM_GET(pDtiPacketIn->header.port),
                     &outPaylSize_byte, &packetError);
         }
      }
   }
   else
   {
      DTI_PRN_USR_ERR_NL(DTI_DEV, DTI_PRN_LEVEL_ERR,
         ("Error: DTI Packet Dev[%s] RegisterGet - not implemented."DTI_CRLF,
           (pDeviceAccessFct->pDevIfName) ? pDeviceAccessFct->pDevIfName : "unknown"));

      packetError = DTI_eErrConfiguration;
   }

   (void)DTI_packetResponceSet(
            pDtiPacketIn, pDtiPacketOut, packetError, outPaylSize_byte, bufferOutSize, IFX_FALSE);

   return IFX_SUCCESS;
}

/**
   Process the packet "DTI_PacketType_eRegisterSet"

\param
   pDtiDevCtx        - points to the Device context.
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
DTI_STATIC IFX_int_t DTI_packet_RegisterSet(
                        DTI_DeviceCtx_t         *pDtiDevCtx,
                        const DTI_Packet_t      *pDtiPacketIn,
                        DTI_Packet_t            *pDtiPacketOut,
                        IFX_uint32_t            bufferOutSize)
{
   DTI_PacketError_t       packetError = DTI_eNoError;
   DTI_H2D_RegisterSet_t   *pDataIn;
   DTI_PTR_U               uPayload;
   DTI_DeviceAccessFct_t   *pDeviceAccessFct = pDtiDevCtx->pDeviceAccessFct;

   if (pDeviceAccessFct->pRegisterSetFct != IFX_NULL)
   {
      uPayload.pUInt8 = (IFX_uint8_t *)pDtiPacketIn->payload;
      pDataIn = (DTI_H2D_RegisterSet_t *)DTI_PTR_CAST_GET_ULONG(uPayload);
      if (pDataIn == IFX_NULL)
      {
         DTI_PRN_USR_ERR_NL(DTI_DEV, DTI_PRN_LEVEL_ERR,
            ("Error: DTI Packet RegisterSet - %s payload."DTI_CRLF,
              (pDataIn == IFX_NULL) ? "missaligned" : "missing"));

         packetError = DTI_eErrMalformedPacket;
      }
      else
      {
         (void)pDeviceAccessFct->pRegisterSetFct(
                  pDtiDevCtx, pDataIn,
                  (IFX_int_t)pDtiPacketIn->header.payloadSize,
                  (IFX_int_t)DTI_HDR_PORT_PORT_NUM_GET(pDtiPacketIn->header.port),
                  &packetError);
      }
   }
   else
   {
      DTI_PRN_USR_ERR_NL(DTI_DEV, DTI_PRN_LEVEL_ERR,
         ("Error: DTI Packet Dev[%s] RegisterSet - not implemented."DTI_CRLF,
           (pDeviceAccessFct->pDevIfName) ? pDeviceAccessFct->pDevIfName : "unknown"));

      packetError = DTI_eErrConfiguration;
   }

   (void)DTI_packetResponceSet(
            pDtiPacketIn, pDtiPacketOut,
            packetError, 0,
            bufferOutSize, IFX_FALSE);


   return IFX_SUCCESS;
}

/**
   Process the packet "DTI_PacketType_eConfigSet"

\param
   pDtiDevCtx        - points to the Device context.
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
DTI_STATIC IFX_int_t DTI_packet_ConfigSet(
                        DTI_DeviceCtx_t         *pDtiDevCtx,
                        const DTI_Packet_t      *pDtiPacketIn,
                        DTI_Packet_t            *pDtiPacketOut,
                        IFX_uint32_t            bufferOutSize)
{
   DTI_PacketError_t          packetError = DTI_eNoError;
   DTI_H2D_DeviceConfigSet_t  *pDataIn;
   DTI_D2H_DeviceConfigSet_t  *pDataOut;
   DTI_PTR_U                  uPayload;
   DTI_DeviceAccessFct_t      *pDeviceAccessFct = pDtiDevCtx->pDeviceAccessFct;

   if (pDeviceAccessFct->pConfigSetFct != IFX_NULL)
   {
      uPayload.pUInt8 = (IFX_uint8_t *)pDtiPacketIn->payload;
      pDataIn = (DTI_H2D_DeviceConfigSet_t *)DTI_PTR_CAST_GET_ULONG(uPayload);

      uPayload.pUInt8 = pDtiPacketOut->payload;
      pDataOut = (DTI_D2H_DeviceConfigSet_t *)DTI_PTR_CAST_GET_ULONG(uPayload);

      if ((pDataIn == IFX_NULL) || (pDataOut == IFX_NULL))
      {
         DTI_PRN_USR_ERR_NL(DTI_DEV, DTI_PRN_LEVEL_ERR,
            ("Error: DTI Packet Dev Config Set - missaligned payload."DTI_CRLF));

         packetError = DTI_eErrMalformedPacket;
      }
      else
      {
         (void)pDeviceAccessFct->pConfigSetFct(
                  pDtiDevCtx, pDataIn, pDataOut,
                  (IFX_int_t)DTI_HDR_PORT_PORT_NUM_GET(pDtiPacketIn->header.port),
                  &packetError);
      }
   }
   else
   {
      DTI_PRN_USR_ERR_NL(DTI_DEV, DTI_PRN_LEVEL_ERR,
         ("Error: DTI Packet Dev[%s] Config Set - not implemented."DTI_CRLF,
           (pDeviceAccessFct->pDevIfName) ? pDeviceAccessFct->pDevIfName : "unknown"));

      packetError = DTI_eErrConfiguration;
   }

   (void)DTI_packetResponceSet(
            pDtiPacketIn, pDtiPacketOut,
            packetError, sizeof(DTI_D2H_DeviceConfigSet_t),
            bufferOutSize, IFX_FALSE);


   return IFX_SUCCESS;
}


/**
   Process the packet "DTI_PacketType_eConfigGet"

\param
   pDtiDevCtx        - points to the Device context.
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
DTI_STATIC IFX_int_t DTI_packet_ConfigGet(
                        DTI_DeviceCtx_t         *pDtiDevCtx,
                        const DTI_Packet_t      *pDtiPacketIn,
                        DTI_Packet_t            *pDtiPacketOut,
                        IFX_uint32_t            bufferOutSize)
{
   DTI_PacketError_t          packetError = DTI_eNoError;
   DTI_H2D_DeviceConfigGet_t  *pDataIn;
   DTI_D2H_DeviceConfigGet_t  *pDataOut;
   DTI_PTR_U                  uPayload;
   DTI_DeviceAccessFct_t      *pDeviceAccessFct = pDtiDevCtx->pDeviceAccessFct;

   if (pDeviceAccessFct->pConfigGetFct != IFX_NULL)
   {
      uPayload.pUInt8 = (IFX_uint8_t *)pDtiPacketIn->payload;
      pDataIn = (DTI_H2D_DeviceConfigGet_t *)DTI_PTR_CAST_GET_ULONG(uPayload);

      uPayload.pUInt8 = pDtiPacketOut->payload;
      pDataOut = (DTI_D2H_DeviceConfigGet_t *)DTI_PTR_CAST_GET_ULONG(uPayload);

      if ((pDataIn == IFX_NULL) || (pDataOut == IFX_NULL))
      {
         DTI_PRN_USR_ERR_NL(DTI_DEV, DTI_PRN_LEVEL_ERR,
            ("Error: DTI Packet Dev Config Get - missaligned payload."DTI_CRLF));

         packetError = DTI_eErrMalformedPacket;
      }
      else
      {
         (void)pDeviceAccessFct->pConfigGetFct(
                  pDtiDevCtx, pDataIn, pDataOut,
                  (IFX_int_t)DTI_HDR_PORT_PORT_NUM_GET(pDtiPacketIn->header.port),
                  &packetError);
      }
   }
   else
   {
      DTI_PRN_USR_ERR_NL(DTI_DEV, DTI_PRN_LEVEL_ERR,
         ("Error: DTI Packet Dev[%s] Config Get - not implemented."DTI_CRLF,
           (pDeviceAccessFct->pDevIfName) ? pDeviceAccessFct->pDevIfName : "unknown"));

      packetError = DTI_eErrConfiguration;
   }

   (void)DTI_packetResponceSet(
            pDtiPacketIn, pDtiPacketOut,
            packetError, sizeof(DTI_D2H_DeviceConfigGet_t),
            bufferOutSize, IFX_FALSE);

   return IFX_SUCCESS;
}


/**
   Process the packet "DTI_PacketType_eMessageSend"

\param
   pDtiDevCtx        - points to the Device context.
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
DTI_STATIC IFX_int_t DTI_packet_MessageSend(
                        DTI_DeviceCtx_t         *pDtiDevCtx,
                        const DTI_Packet_t      *pDtiPacketIn,
                        DTI_Packet_t            *pDtiPacketOut,
                        IFX_uint32_t            bufferOutSize)
{
   IFX_int_t               outPayloadSize = (IFX_int_t)bufferOutSize - sizeof(DTI_PacketHeader_t);
   DTI_PacketError_t       packetError = DTI_eNoError;
   DTI_PTR_U               uPayload;
   DTI_DeviceAccessFct_t   *pDeviceAccessFct = pDtiDevCtx->pDeviceAccessFct;

   switch(pDtiPacketIn->header.packetOptions & DTI_HDR_OPTION_MASK_PAYL_TYPE)
   {
      case DTI_e8Bit:
         {
            DTI_H2D_Message8_u *pDataIn8;
            DTI_D2H_Message8_u *pDataOut8;

            if (pDeviceAccessFct->pMessage8SendFct != IFX_NULL)
            {
               uPayload.pUInt8 = (IFX_uint8_t *)pDtiPacketIn->payload;
               pDataIn8        = (DTI_H2D_Message8_u *)DTI_PTR_CAST_GET_ULONG(uPayload);
               uPayload.pUInt8 = pDtiPacketOut->payload;
               pDataOut8       = (DTI_D2H_Message8_u *)DTI_PTR_CAST_GET_ULONG(uPayload);

               if ((pDataIn8 == IFX_NULL) || (pDataOut8 == IFX_NULL))
               {
                  DTI_PRN_USR_ERR_NL(DTI_DEV, DTI_PRN_LEVEL_ERR,
                     ("Error: DTI Packet Message8 Send - missaligned payload."DTI_CRLF));

                  packetError = DTI_eErrMalformedPacket;
               }
               else
               {
                  (void)pDeviceAccessFct->pMessage8SendFct(
                           pDtiDevCtx, pDataIn8, pDataOut8,
                           (IFX_int_t)DTI_HDR_PORT_PORT_NUM_GET(pDtiPacketIn->header.port),
                           (IFX_int_t)pDtiPacketIn->header.payloadSize,
                           &outPayloadSize, &packetError);
               }
            }
            else
            {
               DTI_PRN_USR_ERR_NL(DTI_DEV, DTI_PRN_LEVEL_ERR,
                  ("Error: DTI Packet Dev[%s] Message8 Send - not implemented."DTI_CRLF,
                    (pDeviceAccessFct->pDevIfName) ? pDeviceAccessFct->pDevIfName : "unknown"));

               packetError = DTI_eErrConfiguration;
            }
         }
         break;

         case DTI_e16Bit:
         {
            DTI_H2D_Message16_u *pDataIn16;
            DTI_D2H_Message16_u *pDataOut16;

            if (pDeviceAccessFct->pMessage16SendFct != IFX_NULL)
            {
               uPayload.pUInt8 = (IFX_uint8_t *)pDtiPacketIn->payload;
               pDataIn16       = (DTI_H2D_Message16_u *)DTI_PTR_CAST_GET_ULONG(uPayload);
               uPayload.pUInt8 = pDtiPacketOut->payload;
               pDataOut16      = (DTI_D2H_Message16_u *)DTI_PTR_CAST_GET_ULONG(uPayload);

               if ((pDataIn16 == IFX_NULL) || (pDataOut16 == IFX_NULL))
               {
                  DTI_PRN_USR_ERR_NL(DTI_DEV, DTI_PRN_LEVEL_ERR,
                     ("Error: DTI Packet Message16 Send - missaligned payload."DTI_CRLF));

                  packetError = DTI_eErrMalformedPacket;
               }
               else
               {
                  (void)pDeviceAccessFct->pMessage16SendFct(
                           pDtiDevCtx, pDataIn16, pDataOut16,
                           (IFX_int_t)DTI_HDR_PORT_PORT_NUM_GET(pDtiPacketIn->header.port),
                           (IFX_int_t)pDtiPacketIn->header.payloadSize,
                           &outPayloadSize, &packetError);
               }
            }
            else
            {
               DTI_PRN_USR_ERR_NL(DTI_DEV, DTI_PRN_LEVEL_ERR,
                  ("Error: DTI Packet Dev[%s] Message16 Send - not implemented."DTI_CRLF,
                    (pDeviceAccessFct->pDevIfName) ? pDeviceAccessFct->pDevIfName : "unknown"));

               packetError = DTI_eErrConfiguration;
            }
         }
         break;

      case DTI_e32Bit:
         {
            DTI_H2D_Message32_u *pDataIn32;
            DTI_D2H_Message32_u *pDataOut32;

            if (pDeviceAccessFct->pMessage32SendFct != IFX_NULL)
            {
               uPayload.pUInt8 = (IFX_uint8_t *)pDtiPacketIn->payload;
               pDataIn32       = (DTI_H2D_Message32_u *)DTI_PTR_CAST_GET_ULONG(uPayload);
               uPayload.pUInt8 = pDtiPacketOut->payload;
               pDataOut32      = (DTI_D2H_Message32_u *)DTI_PTR_CAST_GET_ULONG(uPayload);

               if ((pDataIn32 == IFX_NULL) || (pDataOut32 == IFX_NULL))
               {
                  DTI_PRN_USR_ERR_NL(DTI_DEV, DTI_PRN_LEVEL_ERR,
                     ("Error: DTI Packet Message32 Send - missaligned payload."DTI_CRLF));

                  packetError = DTI_eErrMalformedPacket;
               }
               else
               {
                  (void)pDeviceAccessFct->pMessage32SendFct(
                           pDtiDevCtx, pDataIn32, pDataOut32,
                           (IFX_int_t)DTI_HDR_PORT_PORT_NUM_GET(pDtiPacketIn->header.port),
                           (IFX_int_t)pDtiPacketIn->header.payloadSize,
                           &outPayloadSize, &packetError);
               }
            }
            else
            {
               DTI_PRN_USR_ERR_NL(DTI_DEV, DTI_PRN_LEVEL_ERR,
                  ("Error: DTI Packet Dev[%s] Message32 Send - not implemented."DTI_CRLF,
                    (pDeviceAccessFct->pDevIfName) ? pDeviceAccessFct->pDevIfName : "unknown"));

               packetError = DTI_eErrConfiguration;
            }
         }
         break;

      default:
         /* not supported option */
         DTI_PRN_USR_ERR_NL(DTI_DEV, DTI_PRN_LEVEL_ERR,
            ("Error: DTI Packet Message Send - invalid option %d."DTI_CRLF,
            pDtiPacketIn->header.packetOptions ));

         packetError = DTI_eErrInvalidParameters;
   }

   (void)DTI_packetResponceSet(
            pDtiPacketIn, pDtiPacketOut,
            packetError, (IFX_uint32_t)outPayloadSize,
            bufferOutSize, IFX_FALSE);

   return IFX_SUCCESS;
}

/**
   Process the packet "DTI_PacketType_eTraceBufferConfigSet"

\param
   pDtiDevCtx        - points to the Device context.
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
DTI_STATIC IFX_int_t DTI_packet_TraceBufferConfigSet(
                        DTI_DeviceCtx_t         *pDtiDevCtx,
                        const DTI_Packet_t      *pDtiPacketIn,
                        DTI_Packet_t            *pDtiPacketOut,
                        IFX_uint32_t            bufferOutSize)
{
   DTI_PacketError_t          packetError = DTI_eNoError;
   DTI_H2D_TraceConfigSet_t   *pDataIn;
   DTI_D2H_TraceConfigSet_t   *pDataOut;
   DTI_PTR_U                  uPayload;
   DTI_DeviceAccessFct_t      *pDeviceAccessFct = pDtiDevCtx->pDeviceAccessFct;

   if (pDeviceAccessFct->pTraceBufferConfigSetFct != IFX_NULL)
   {
      uPayload.pUInt8 = (IFX_uint8_t *)pDtiPacketIn->payload;
      pDataIn = (DTI_H2D_TraceConfigSet_t *)DTI_PTR_CAST_GET_ULONG(uPayload);

      uPayload.pUInt8 = pDtiPacketOut->payload;
      pDataOut = (DTI_D2H_TraceConfigSet_t *)DTI_PTR_CAST_GET_ULONG(uPayload);

      if ((pDataIn == IFX_NULL) || (pDataOut == IFX_NULL))
      {
         DTI_PRN_USR_ERR_NL(DTI_DEV, DTI_PRN_LEVEL_ERR,
            ("Error: DTI Packet Trace Buffer Config Set - missaligned payload."DTI_CRLF));

         packetError = DTI_eErrMalformedPacket;
      }
      else
      {
         (void)pDeviceAccessFct->pTraceBufferConfigSetFct(
                     pDtiDevCtx, pDataIn, pDataOut,
                     (IFX_int_t)DTI_HDR_PORT_PORT_NUM_GET(pDtiPacketIn->header.port),
                     &packetError);
      }
   }
   else
   {
      DTI_PRN_USR_ERR_NL(DTI_DEV, DTI_PRN_LEVEL_ERR,
         ("Error: DTI Packet Dev[%s] Trace Buffer Config Set - not implemented."DTI_CRLF,
           (pDeviceAccessFct->pDevIfName) ? pDeviceAccessFct->pDevIfName : "unknown"));

      packetError = DTI_eErrConfiguration;
   }

   (void)DTI_packetResponceSet(
            pDtiPacketIn, pDtiPacketOut,
            packetError, sizeof(DTI_D2H_TraceConfigSet_t),
            bufferOutSize, IFX_FALSE);

   return IFX_SUCCESS;
}

/**
   Process the packet "DTI_PacketType_eTraceBufferReset"

\param
   pDtiDevCtx        - points to the Device context.
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
DTI_STATIC IFX_int_t DTI_packet_TraceBufferReset(
                        DTI_DeviceCtx_t         *pDtiDevCtx,
                        const DTI_Packet_t      *pDtiPacketIn,
                        DTI_Packet_t            *pDtiPacketOut,
                        IFX_uint32_t            bufferOutSize)
{
   DTI_PacketError_t       packetError = DTI_eNoError;
   DTI_DeviceAccessFct_t   *pDeviceAccessFct = pDtiDevCtx->pDeviceAccessFct;

   if (pDeviceAccessFct->pTraceBufferResetFct != IFX_NULL)
   {
      (void)pDeviceAccessFct->pTraceBufferResetFct(
                  pDtiDevCtx,
                  (IFX_int_t)DTI_HDR_PORT_PORT_NUM_GET(pDtiPacketIn->header.port),
                  &packetError);
   }
   else
   {
      DTI_PRN_USR_ERR_NL(DTI_DEV, DTI_PRN_LEVEL_ERR,
         ("Error: DTI Packet Dev[%s] Trace Buffer Reset - not implemented."DTI_CRLF,
           (pDeviceAccessFct->pDevIfName) ? pDeviceAccessFct->pDevIfName : "unknown"));

      packetError = DTI_eErrConfiguration;
   }

   (void)DTI_packetResponceSet(
            pDtiPacketIn, pDtiPacketOut,
            packetError, 0,
            bufferOutSize, IFX_FALSE);

   return IFX_SUCCESS;
}


/**
   Process the packet "DTI_PacketType_eTraceBufferStatusGet"

\param
   pDtiDevCtx        - points to the Device context.
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
DTI_STATIC IFX_int_t DTI_packet_TraceBufferStatusGet(
                        DTI_DeviceCtx_t         *pDtiDevCtx,
                        const DTI_Packet_t      *pDtiPacketIn,
                        DTI_Packet_t            *pDtiPacketOut,
                        IFX_uint32_t            bufferOutSize)
{
   DTI_PacketError_t          packetError = DTI_eNoError;
   DTI_D2H_TraceStatusGet_t   *pDataOut;
   DTI_PTR_U                  uPayload;
   DTI_DeviceAccessFct_t      *pDeviceAccessFct = pDtiDevCtx->pDeviceAccessFct;

   if (pDeviceAccessFct->pTraceBufferStatusGetFct != IFX_NULL)
   {
      uPayload.pUInt8 = pDtiPacketOut->payload;
      pDataOut = (DTI_D2H_TraceStatusGet_t *)DTI_PTR_CAST_GET_ULONG(uPayload);

      if (pDataOut == IFX_NULL)
      {
         DTI_PRN_USR_ERR_NL(DTI_DEV, DTI_PRN_LEVEL_ERR,
            ("Error: DTI Packet Trace Buffer Status Get - missaligned payload."DTI_CRLF));

         packetError = DTI_eErrMalformedPacket;
      }
      else
      {
         (void)pDeviceAccessFct->pTraceBufferStatusGetFct(
                     pDtiDevCtx, pDataOut,
                     (IFX_int_t)DTI_HDR_PORT_PORT_NUM_GET(pDtiPacketIn->header.port),
                     &packetError);
      }
   }
   else
   {
      DTI_PRN_USR_ERR_NL(DTI_DEV, DTI_PRN_LEVEL_ERR,
         ("Error: DTI Packet Dev[%s] Trace Buffer Status Get - not implemented."DTI_CRLF,
           (pDeviceAccessFct->pDevIfName) ? pDeviceAccessFct->pDevIfName : "unknown"));

      packetError = DTI_eErrConfiguration;
   }

   (void)DTI_packetResponceSet(
            pDtiPacketIn, pDtiPacketOut,
            packetError, sizeof(DTI_D2H_TraceStatusGet_t),
            bufferOutSize, IFX_FALSE);


   return IFX_SUCCESS;
}


/**
   Process the packet "DTI_PacketType_eTraceBufferGet"

\param
   pDtiDevCtx        - points to the Device context.
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
DTI_STATIC IFX_int_t DTI_packet_TraceBufferGet(
                        DTI_DeviceCtx_t         *pDtiDevCtx,
                        const DTI_Packet_t      *pDtiPacketIn,
                        DTI_Packet_t            **ppDtiPacketOut,
                        IFX_uint32_t            *pBufferOutSize)
{
   IFX_int_t                  trBufReadSize_byte = 0;
   IFX_uint32_t               usedBufferOutSize  = 0;
   DTI_PacketError_t          packetError = DTI_eNoError;
   DTI_PTR_U                  uPayload;

   DTI_H2D_TraceBufferGet_t   *pDataIn;
   DTI_Packet_t               *pUsedDtiPacketOut = IFX_NULL;
   DTI_DeviceAccessFct_t      *pDeviceAccessFct = pDtiDevCtx->pDeviceAccessFct;

   if (pDeviceAccessFct->pTraceBufferGetFct != IFX_NULL)
   {
      uPayload.pUInt8 = (IFX_uint8_t *)pDtiPacketIn->payload;
      pDataIn = (DTI_H2D_TraceBufferGet_t *)DTI_PTR_CAST_GET_ULONG(uPayload);

      usedBufferOutSize = *pBufferOutSize;
      pUsedDtiPacketOut = *ppDtiPacketOut;

      if (pDataIn == IFX_NULL)
      {
         DTI_PRN_USR_ERR_NL(DTI_DEV, DTI_PRN_LEVEL_ERR,
            ("Error: DTI Packet Trace Buffer Get - missaligned payload."DTI_CRLF));

         packetError = DTI_eErrMalformedPacket;
      }
      else
      {
         /* Note:
               if the requested data size does not fit into the given out packet
               this function will allocate an larger buffer and will use this one.
               The new buffer (packet out) is returned via the return ptr arguments. */
         (void)pDeviceAccessFct->pTraceBufferGetFct(
                     pDtiDevCtx, pDataIn,
                     &pUsedDtiPacketOut, (IFX_int_t *)&usedBufferOutSize,
                     (IFX_int_t)DTI_HDR_PORT_PORT_NUM_GET(pDtiPacketIn->header.port),
                     &trBufReadSize_byte, &packetError);
      }
   }
   else
   {
      DTI_PRN_USR_ERR_NL(DTI_DEV, DTI_PRN_LEVEL_ERR,
         ("Error: DTI Packet Dev[%s] Trace Buffer Get - not implemented."DTI_CRLF,
           (pDeviceAccessFct->pDevIfName) ? pDeviceAccessFct->pDevIfName : "unknown"));

      packetError = DTI_eErrConfiguration;
   }

   (void)DTI_packetResponceSet(
            pDtiPacketIn, pUsedDtiPacketOut, packetError,
            (IFX_uint32_t)trBufReadSize_byte,
            (IFX_uint32_t)usedBufferOutSize,
            IFX_FALSE);

   pUsedDtiPacketOut->header.packetOptions = DTI_e8Bit;

   *pBufferOutSize = usedBufferOutSize;
   *ppDtiPacketOut = pUsedDtiPacketOut;

   return IFX_SUCCESS;
}

/**
   Process the packet "DTI_PacketType_eDebugGet"

\param
   pDtiDevCtx        - points to the Device context.
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
DTI_STATIC IFX_int_t DTI_packet_DebugGet(
                        DTI_DeviceCtx_t         *pDtiDevCtx,
                        const DTI_Packet_t      *pDtiPacketIn,
                        DTI_Packet_t            *pDtiPacketOut,
                        IFX_uint32_t            bufferOutSize)
{
   IFX_int_t               dbgReadCount = 0;
   DTI_PacketError_t       packetError = DTI_eNoError;
   DTI_H2D_DebugRead_t     *pDataIn;
   DTI_D2H_DebugRead_t     *pDataOut;
   DTI_PTR_U               uPayload;
   DTI_DeviceAccessFct_t   *pDeviceAccessFct = pDtiDevCtx->pDeviceAccessFct;

   if (pDeviceAccessFct->pDebugReadFct != IFX_NULL)
   {
      uPayload.pUInt8 = (IFX_uint8_t *)pDtiPacketIn->payload;
      pDataIn = (DTI_H2D_DebugRead_t *)DTI_PTR_CAST_GET_ULONG(uPayload);

      uPayload.pUInt8 = pDtiPacketOut->payload;
      pDataOut = (DTI_D2H_DebugRead_t *)DTI_PTR_CAST_GET_ULONG(uPayload);

      if ((pDataIn == IFX_NULL) || (pDataOut == IFX_NULL))
      {
         DTI_PRN_USR_ERR_NL(DTI_DEV, DTI_PRN_LEVEL_ERR,
            ("Error: DTI Packet DebugGet - missaligned payload."DTI_CRLF));

         packetError = DTI_eErrMalformedPacket;
      }
      else
      {
         if ( (sizeof(DTI_PacketHeader_t) + (pDataIn->count * sizeof(IFX_uint32_t))) > bufferOutSize)
         {
            DTI_PRN_USR_ERR_NL(DTI_DEV, DTI_PRN_LEVEL_ERR,
               ("Error: DTI Packet DebugGet - count = %d > max = %d (buffer size)."DTI_CRLF,
                 pDataIn->count, (bufferOutSize - sizeof(DTI_PacketHeader_t)) / sizeof(IFX_uint32_t) ));

            packetError = DTI_eErrConfiguration;
         }
         else
         {
            (void)pDeviceAccessFct->pDebugReadFct(
                     pDtiDevCtx, pDataIn, pDataOut,
                     (IFX_int_t)DTI_HDR_PORT_PORT_NUM_GET(pDtiPacketIn->header.port),
                     &dbgReadCount, &packetError);
         }
      }
   }
   else
   {
      DTI_PRN_USR_ERR_NL(DTI_DEV, DTI_PRN_LEVEL_ERR,
         ("Error: DTI Packet Dev[%s] DebugGet - not implemented."DTI_CRLF,
           (pDeviceAccessFct->pDevIfName) ? pDeviceAccessFct->pDevIfName : "unknown"));

      packetError = DTI_eErrConfiguration;
   }

   (void)DTI_packetResponceSet(
            pDtiPacketIn, pDtiPacketOut,
            packetError,
            (IFX_uint32_t)(dbgReadCount * sizeof(IFX_uint32_t)),
            bufferOutSize, IFX_FALSE);

   return IFX_SUCCESS;
}


/**
   Process the packet "DTI_PacketType_eDebugSet"

\param
   pDtiDevCtx        - points to the Device context.
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
DTI_STATIC IFX_int_t DTI_packet_DebugSet(
                        DTI_DeviceCtx_t         *pDtiDevCtx,
                        const DTI_Packet_t      *pDtiPacketIn,
                        DTI_Packet_t            *pDtiPacketOut,
                        IFX_uint32_t            bufferOutSize)
{

   IFX_int_t               dbgWriteCount = 0;
   DTI_PacketError_t       packetError = DTI_eNoError;
   DTI_H2D_DebugWrite_t    *pDataIn;
   DTI_PTR_U               uPayload;
   DTI_DeviceAccessFct_t   *pDeviceAccessFct = pDtiDevCtx->pDeviceAccessFct;

   if (pDeviceAccessFct->pDebugWriteFct != IFX_NULL)
   {
      uPayload.pUInt8 = (IFX_uint8_t *)pDtiPacketIn->payload;
      pDataIn = (DTI_H2D_DebugWrite_t *)DTI_PTR_CAST_GET_ULONG(uPayload);

      if (pDataIn == IFX_NULL)
      {
         DTI_PRN_USR_ERR_NL(DTI_DEV, DTI_PRN_LEVEL_ERR,
            ("Error: DTI Packet DebugSet - missaligned payload."DTI_CRLF));

         packetError = DTI_eErrMalformedPacket;
      }
      else
      {
         dbgWriteCount = ((IFX_int_t)pDtiPacketIn->header.payloadSize
                          - (sizeof(pDataIn->type) + sizeof(pDataIn->offset))) / sizeof(IFX_uint32_t);

         (void)pDeviceAccessFct->pDebugWriteFct(
                  pDtiDevCtx, pDataIn, IFX_NULL,
                  (IFX_int_t)DTI_HDR_PORT_PORT_NUM_GET(pDtiPacketIn->header.port),
                  &dbgWriteCount, &packetError);
      }
   }
   else
   {
      DTI_PRN_USR_ERR_NL(DTI_DEV, DTI_PRN_LEVEL_ERR,
         ("Error: DTI Packet Dev[%s] DebugSet - not implemented."DTI_CRLF,
           (pDeviceAccessFct->pDevIfName) ? pDeviceAccessFct->pDevIfName : "unknown"));

      packetError = DTI_eErrConfiguration;
   }

   (void)DTI_packetResponceSet(
            pDtiPacketIn, pDtiPacketOut,
            packetError, 0,
            bufferOutSize, IFX_FALSE);


   return IFX_SUCCESS;
}

/**
   Process the packet "DTI_PacketType_eWinEasyCiAccess"

\param
   pDtiDevCtx        - points to the Device context.
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
DTI_STATIC IFX_int_t DTI_packet_WinEasyCiAccess(
                        DTI_DeviceCtx_t         *pDtiDevCtx,
                        const DTI_Packet_t      *pDtiPacketIn,
                        DTI_Packet_t            *pDtiPacketOut,
                        IFX_uint32_t            bufferOutSize)
{
   DTI_PacketError_t       packetError = DTI_eNoError;
   IFX_int_t               len = 0;
   DTI_DeviceAccessFct_t   *pDeviceAccessFct = pDtiDevCtx->pDeviceAccessFct;

   if (pDeviceAccessFct->pWinEasyCiAccessFct != IFX_NULL)
   {
      len = pDeviceAccessFct->pWinEasyCiAccessFct(
                                    pDtiDevCtx,
                                    DTI_HDR_PORT_PORT_NUM_GET(pDtiPacketIn->header.port),
                                    pDtiPacketIn->payload,
                                    pDtiPacketIn->header.payloadSize,
                                    pDtiPacketOut->payload,
                                    bufferOutSize,
                                    &packetError);
   }
   else
   {
      DTI_PRN_USR_ERR_NL(DTI_DEV, DTI_PRN_LEVEL_ERR,
         ("Error: DTI Packet Dev[%s] WinEasyCiAccess - not implemented."DTI_CRLF,
           (pDeviceAccessFct->pDevIfName) ? pDeviceAccessFct->pDevIfName : "unknown"));

      packetError = DTI_eErrConfiguration;
      len = -1;
   }

   if(len >= 0)
   {
      (void)DTI_packetResponceSet(pDtiPacketIn,
                                  pDtiPacketOut,
                                  packetError,
                                  len,
                                  bufferOutSize,
                                  IFX_FALSE);
      return IFX_SUCCESS;
   }

   return IFX_ERROR;
}


/* ============================================================================
   Global Function
   ========================================================================= */

/**
   DTI Standard Call Packet Handler.
*/

/**
   DTI Device Packet Handler.

\param
   pDtiDevCtx        - points to the Device context.
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
IFX_int_t DTI_packetHandler_device(
                        DTI_DeviceCtx_t         *pDtiDevCtx,
                        DTI_ProtocolServerCtx_t *pDtiProtServerCtx,
                        const DTI_Packet_t      *pDtiPacketIn,
                        DTI_Packet_t            *pDtiPacketOut,
                        IFX_uint_t              dtiBufOutLen)
{
   IFX_int_t            retVal = IFX_SUCCESS;
   IFX_uint_t           bufOutLen;
   const DTI_Packet_t   *pPacketIn;
   DTI_Packet_t         *pPacketOut;
   DTI_PTR_U            uPayload;

   if ((pDtiProtServerCtx == IFX_NULL) || (pDtiDevCtx == IFX_NULL))
   {
      DTI_PRN_USR_ERR_NL(DTI_DEV, DTI_PRN_LEVEL_ERR,
         ("Error: Device Packet Handler - NULL ptr args."DTI_CRLF));

      return IFX_ERROR;
   }

   if (pDtiDevCtx->pDeviceAccessFct == IFX_NULL)
   {
      DTI_PRN_USR_ERR_NL(DTI_DEV, DTI_PRN_LEVEL_ERR,
         ("Error: Device Packet Handler - missing device handler."DTI_CRLF));

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

   if ( (pPacketIn == IFX_NULL) || (pPacketOut == IFX_NULL) )
   {
      DTI_PRN_USR_ERR_NL(DTI_DEV, DTI_PRN_LEVEL_ERR,
         ("Error: Device Packet Handler - %s packet missaligend."DTI_CRLF,
         (pPacketIn == IFX_NULL) ? "IN" : "OUT" ));

      return IFX_ERROR;
   }

   if (pPacketIn->header.magic != DTI_MAGIC)
   {
      DTI_PRN_USR_ERR_NL(DTI_DEV, DTI_PRN_LEVEL_ERR,
         ("Error: Device Packet Handler - invalid IN packet."DTI_CRLF));

      return IFX_ERROR;
   }

   /* save pPacketIn and pPacketOut reference into Dev Ctx */
   pDtiDevCtx->pPacketIn = (DTI_Packet_t*) pPacketIn;
   pDtiDevCtx->pPacketOut = (DTI_Packet_t*) pPacketOut;

   switch(pPacketIn->header.packetType)
   {
      /*
         Packet Type: 0x0001xxxx - Low Level HW Access
      */
      case DTI_PacketType_eDeviceReset:
         retVal = DTI_packet_DeviceReset(
                        pDtiDevCtx, pPacketIn, pPacketOut, bufOutLen);
         break;

      case DTI_PacketType_eDeviceDownload:
         retVal = DTI_packet_DeviceDownload(
                        pDtiDevCtx, pDtiProtServerCtx, pPacketIn, pPacketOut, bufOutLen);
         break;

      case DTI_PacketType_eDeviceOpen:
         retVal = DTI_packet_DeviceOpen(
                        pDtiDevCtx, pPacketIn, pPacketOut, bufOutLen);
         break;

      case DTI_PacketType_eDeviceClose:
         retVal = DTI_packet_DeviceClose(
                        pDtiDevCtx, pPacketIn, pPacketOut, bufOutLen);
         break;

      case DTI_PacketType_eRegisterLock:
         retVal = DTI_packet_RegisterLock(
                        pDtiDevCtx, pPacketIn, pPacketOut, bufOutLen);
         break;

      case DTI_PacketType_eRegisterGet:
         retVal = DTI_packet_RegisterGet(
                        pDtiDevCtx, pPacketIn, pPacketOut, bufOutLen);
         break;

      case DTI_PacketType_eRegisterSet:
         retVal = DTI_packet_RegisterSet(
                        pDtiDevCtx, pPacketIn, pPacketOut, bufOutLen);
         break;

      /*
         Packet Type: 0x0002xxxx - Control and Message Interface
      */
      case DTI_PacketType_eConfigSet:
         retVal = DTI_packet_ConfigSet(
                        pDtiDevCtx, pPacketIn, pPacketOut, bufOutLen);
         break;

      case DTI_PacketType_eConfigGet:
         retVal = DTI_packet_ConfigGet(
                        pDtiDevCtx, pPacketIn, pPacketOut, bufOutLen);
         break;

      case DTI_PacketType_eMessageSend:
         retVal = DTI_packet_MessageSend(
                        pDtiDevCtx, pPacketIn, pPacketOut, bufOutLen);
         break;

      case DTI_PacketType_eMessageError:
         retVal = DTI_packetUnknownSet(
                        pPacketIn, pPacketOut, bufOutLen);
         break;

      /*
         Packet Type: 0x0003xxxx - Trace Buffer Access
      */
      case DTI_PacketType_eTraceBufferConfigSet:
         retVal = DTI_packet_TraceBufferConfigSet(
                        pDtiDevCtx, pPacketIn, pPacketOut, bufOutLen);
         break;

      case DTI_PacketType_eTraceBufferReset:
         retVal = DTI_packet_TraceBufferReset(
                        pDtiDevCtx, pPacketIn, pPacketOut, bufOutLen);
         break;

      case DTI_PacketType_eTraceBufferStatusGet:
         retVal = DTI_packet_TraceBufferStatusGet(
                        pDtiDevCtx, pPacketIn, pPacketOut, bufOutLen);
         break;

      case DTI_PacketType_eTraceBufferGet:
          retVal = DTI_packet_TraceBufferGet(
                        pDtiDevCtx, pPacketIn, &pPacketOut, &bufOutLen);
         break;

      /*
         Packet Type: 0x0004xxxx - Debug Register Access
      */
      case DTI_PacketType_eDebugGet:
         retVal = DTI_packet_DebugGet(
                        pDtiDevCtx, pPacketIn, pPacketOut, bufOutLen);
         break;

      case DTI_PacketType_eDebugSet:
         retVal = DTI_packet_DebugSet(
                        pDtiDevCtx, pPacketIn, pPacketOut, bufOutLen);
         break;

      /*
         Packet Type: 0x0006xxxx - WinEasy C/I Access
      */
      case DTI_PacketType_eWinEasyCiAccess:
         retVal = DTI_packet_WinEasyCiAccess(
                        pDtiDevCtx, pPacketIn, pPacketOut, bufOutLen);
         break;

      default:
         {
            retVal = DTI_packetUnknownSet(
                           pPacketIn, pPacketOut, bufOutLen);
         }
         break;
   }

   /* destroy pPacketIn and pPacketOut reference of Dev Ctx */
   pDtiDevCtx->pPacketIn = NULL;
   pDtiDevCtx->pPacketOut = NULL;


   if (retVal == IFX_SUCCESS)
   {
      DTI_packetShow (
         pPacketOut, IFX_TRUE, IFX_FALSE, "Dev Send",
         (pPacketOut->header.error == DTI_eNoError) ? IFXOS_PRN_LEVEL_LOW : DTI_PRN_LEVEL_HIGH);

      if (DTI_packetSend(&pDtiProtServerCtx->dtiCon, pPacketOut) != IFX_SUCCESS)
      {
         DTI_PRN_USR_ERR_NL(DTI_DEV, DTI_PRN_LEVEL_ERR,
            ("Error: Device Packet Handler - send packet."DTI_CRLF));

         return IFX_ERROR;
      }
   }

   return IFX_SUCCESS;
}

/**
   Set the Device config

\remark
   Simple a warpper for the device specific function.
*/
IFX_int_t DTI_device_configSet(
                        DTI_DeviceCtx_t            *pDtiDevCtx,
                        DTI_H2D_DeviceConfigSet_t  *pInCfgSet,
                        DTI_D2H_DeviceConfigSet_t  *pOutCfgSet,
                        IFX_int_t                  lineNum,
                        DTI_PacketError_t          *pPacketError)
{
   DTI_DeviceAccessFct_t   *pDeviceAccessFct = IFX_NULL;

   if (pDtiDevCtx == IFX_NULL)
   {
      DTI_PRN_USR_ERR_NL(DTI_DEV, DTI_PRN_LEVEL_ERR,
         ("Error: Device ConfigSet - NULL ptr args."DTI_CRLF));

      return IFX_ERROR;
   }

   if (pDtiDevCtx->pDeviceAccessFct == IFX_NULL)
   {
      DTI_PRN_USR_ERR_NL(DTI_DEV, DTI_PRN_LEVEL_ERR,
         ("Error: Device ConfigSet - missing device handler."DTI_CRLF));

      return IFX_ERROR;
   }
   pDeviceAccessFct = pDtiDevCtx->pDeviceAccessFct;

   if (pDeviceAccessFct->pConfigSetFct != IFX_NULL)
   {
      return pDeviceAccessFct->pConfigSetFct(
                                    pDtiDevCtx,
                                    pInCfgSet, pOutCfgSet,
                                    lineNum, pPacketError);
   }
   else
   {
      DTI_PRN_USR_ERR_NL(DTI_DEV, DTI_PRN_LEVEL_ERR,
         ("Error: Device ConfigSet - Dev[%s] ConfigSet - not implemented."DTI_CRLF,
           (pDeviceAccessFct->pDevIfName) ? pDeviceAccessFct->pDevIfName : "unknown"));

      *pPacketError = DTI_eErrConfiguration;
   }

   return IFX_ERROR;
}


/**
   Get the Device config

\remark
   Simple a warpper for the device specific function.
*/
IFX_int_t DTI_device_configGet(
                        DTI_DeviceCtx_t            *pDtiDevCtx,
                        DTI_H2D_DeviceConfigGet_t  *pInCfgGet,
                        DTI_D2H_DeviceConfigGet_t  *pOutCfgGet,
                        IFX_int_t                  lineNum,
                        DTI_PacketError_t          *pPacketError)
{
   DTI_DeviceAccessFct_t   *pDeviceAccessFct = IFX_NULL;

   if (pDtiDevCtx == IFX_NULL)
   {
      DTI_PRN_USR_ERR_NL(DTI_DEV, DTI_PRN_LEVEL_ERR,
         ("Error: Device ConfigSet - NULL ptr args."DTI_CRLF));

      return IFX_ERROR;
   }

   if (pDtiDevCtx->pDeviceAccessFct == IFX_NULL)
   {
      DTI_PRN_USR_ERR_NL(DTI_DEV, DTI_PRN_LEVEL_ERR,
         ("Error: Device ConfigGet - missing device handler."DTI_CRLF));

      return IFX_ERROR;
   }
   pDeviceAccessFct = pDtiDevCtx->pDeviceAccessFct;

   if (pDeviceAccessFct->pConfigGetFct != IFX_NULL)
   {
      return pDeviceAccessFct->pConfigGetFct(
                                    pDtiDevCtx,
                                    pInCfgGet, pOutCfgGet,
                                    lineNum, pPacketError);
   }
   else
   {
      DTI_PRN_USR_ERR_NL(DTI_DEV, DTI_PRN_LEVEL_ERR,
         ("Error: Device ConfigGet - Dev[%s] ConfigGet - not implemented."DTI_CRLF,
           (pDeviceAccessFct->pDevIfName) ? pDeviceAccessFct->pDevIfName : "unknown"));

      *pPacketError = DTI_eErrConfiguration;
   }

   return IFX_ERROR;
}

/**
   Open a given device.

\remark
   Simple a warpper for the device specific function.
*/
IFX_int_t DTI_device_devOpen(
                        DTI_DeviceCtx_t   *pDtiDevCtx,
                        IFX_int_t         lineNum,
                        DTI_PacketError_t *pPacketError)
{
   DTI_DeviceAccessFct_t   *pDeviceAccessFct = IFX_NULL;

   if (pDtiDevCtx == IFX_NULL)
   {
      DTI_PRN_USR_ERR_NL(DTI_DEV, DTI_PRN_LEVEL_ERR,
         ("Error: Device Open - NULL ptr args."DTI_CRLF));

      return IFX_ERROR;
   }

   if (pDtiDevCtx->pDeviceAccessFct == IFX_NULL)
   {
      DTI_PRN_USR_ERR_NL(DTI_DEV, DTI_PRN_LEVEL_ERR,
         ("Error: Device Open - missing device handler."DTI_CRLF));

      return IFX_ERROR;
   }
   pDeviceAccessFct = pDtiDevCtx->pDeviceAccessFct;

   if (pDeviceAccessFct->pDeviceOpenFct != IFX_NULL)
   {
      return pDeviceAccessFct->pDeviceOpenFct(
                                    pDtiDevCtx, lineNum, pPacketError);
   }
   else
   {
      DTI_PRN_USR_ERR_NL(DTI_DEV, DTI_PRN_LEVEL_ERR,
         ("Error: Device Open - Dev[%s] Open - not implemented."DTI_CRLF,
           (pDeviceAccessFct->pDevIfName) ? pDeviceAccessFct->pDevIfName : "unknown"));

      *pPacketError = DTI_eErrConfiguration;
   }

   return IFX_ERROR;
}

/**
   Get the Device config

\remark
   Simple a warpper for the device specific function.
*/
IFX_int_t DTI_device_devClose(
                        DTI_DeviceCtx_t            *pDtiDevCtx,
                        IFX_int_t                  lineNum,
                        DTI_PacketError_t          *pPacketError)
{
   DTI_DeviceAccessFct_t   *pDeviceAccessFct = IFX_NULL;

   if (pDtiDevCtx == IFX_NULL)
   {
      DTI_PRN_USR_ERR_NL(DTI_DEV, DTI_PRN_LEVEL_ERR,
         ("Error: Device Close - NULL ptr args."DTI_CRLF));

      return IFX_ERROR;
   }

   if (pDtiDevCtx->pDeviceAccessFct == IFX_NULL)
   {
      DTI_PRN_USR_ERR_NL(DTI_DEV, DTI_PRN_LEVEL_ERR,
         ("Error: Device Close - missing device handler."DTI_CRLF));

      return IFX_ERROR;
   }
   pDeviceAccessFct = pDtiDevCtx->pDeviceAccessFct;

   if (pDeviceAccessFct->pDeviceCloseFct != IFX_NULL)
   {
      return pDeviceAccessFct->pDeviceCloseFct(
                                    pDtiDevCtx, lineNum, pPacketError);
   }
   else
   {
      DTI_PRN_USR_ERR_NL(DTI_DEV, DTI_PRN_LEVEL_ERR,
         ("Error: Device Close - Dev[%s] Close - not implemented."DTI_CRLF,
           (pDeviceAccessFct->pDevIfName) ? pDeviceAccessFct->pDevIfName : "unknown"));

      *pPacketError = DTI_eErrConfiguration;
   }

   return IFX_ERROR;
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
IFX_int_t DTI_device_autoMsgProcess(
                        DTI_DeviceCtx_t         *pDtiDevCtx,
                        const DTI_Connection_t  *pDtiCon,
                        IFX_uint32_t            devSelectWait_ms,
                        IFX_char_t              *pOutBuffer,
                        IFX_int_t               outBufferSize_byte)
{
   IFX_int_t               retVal = IFX_SUCCESS;
   DTI_DeviceAccessFct_t   *pDeviceAccessFct = IFX_NULL;

   if (pDtiDevCtx == IFX_NULL)
   {
      DTI_PRN_USR_ERR_NL(DTI_DEV, DTI_PRN_LEVEL_ERR,
         ("Error: Device AutoMsgProcess - NULL ptr args."DTI_CRLF));

      return IFX_ERROR;
   }

   if (pDtiDevCtx->pDeviceAccessFct == IFX_NULL)
   {
      DTI_PRN_USR_ERR_NL(DTI_DEV, DTI_PRN_LEVEL_ERR,
         ("Error: Device AutoMsgProcess - missing device handler."DTI_CRLF));

      return IFX_ERROR;
   }
   pDeviceAccessFct = pDtiDevCtx->pDeviceAccessFct;

   if (pDeviceAccessFct->pAutoMsgProcessFct != IFX_NULL)
   {
      if ( (retVal = pDeviceAccessFct->pAutoMsgProcessFct(
                           pDtiDevCtx, pDtiCon, devSelectWait_ms,
                           pOutBuffer, outBufferSize_byte)) != IFX_SUCCESS)
      {
         DTI_PRN_USR_ERR_NL(DTI_DEV, DTI_PRN_LEVEL_ERR,
            ("Error: Device Auto Msg Process - with errors."DTI_CRLF));
      }
   }
   else
   {
      DTI_PRN_USR_ERR_NL(DTI_DEV, DTI_PRN_LEVEL_ERR,
         ("Error: Device AutoMsgProcess - Dev[%s] AutoMsgProcess - not implemented."DTI_CRLF,
           (pDeviceAccessFct->pDevIfName) ? pDeviceAccessFct->pDevIfName : "unknown"));

      return IFX_ERROR;
   }

   return retVal;
}


/**
   Setup and start the underlaying DSL device

\param
   pAgentSysInfo  - points to the DTI Agent System Info.
\param
   ppDtiDevCtx    - return pointer, return the created device context.

\return
   IFX_SUCCESS if setup was successful, else IFX_ERROR.

*/
IFX_int_t DTI_device_moduleStart(
                        DTI_DeviceAccessFct_t   *pDeviceAccessFct,
                        DTI_DeviceSysInfo_t     *pDeviceSystemInfo,
                        DTI_DeviceCtx_t         **ppDtiDevCtx)
{
   IFX_int_t         retVal = IFX_SUCCESS;
   DTI_DeviceCtx_t   *pDtiDevCtx = IFX_NULL;

   if (pDeviceAccessFct == IFX_NULL)
   {
      DTI_PRN_USR_ERR_NL(DTI_DEV, DTI_PRN_LEVEL_ERR,
         ("Error: Device Module Start - missing device handler."DTI_CRLF));

      return IFX_ERROR;
   }

   if (pDeviceAccessFct->pModuleSetupFct != IFX_NULL)
   {
      retVal = pDeviceAccessFct->pModuleSetupFct(pDeviceSystemInfo, &pDtiDevCtx);

      if (retVal == IFX_SUCCESS)
      {
         /* set the given Device Access Fct */
         pDtiDevCtx->pDeviceAccessFct = pDeviceAccessFct;

         *ppDtiDevCtx = pDtiDevCtx;
      }
   }
   else
   {
      DTI_PRN_USR_ERR_NL(DTI_DEV, DTI_PRN_LEVEL_ERR,
         ("Error: Module Start - Dev[%s] ModuleSetup not implemented."DTI_CRLF,
           (pDeviceAccessFct->pDevIfName) ? pDeviceAccessFct->pDevIfName : "unknown"));

      return IFX_ERROR;
   }

   return retVal;
}


/**
   Stop and cleanup the underlaying DSL device

\param
   ppDtiDevCtx    - return pointer, contains the device context.

\return
   IFX_SUCCESS if stop was successful and the return pointer is cleared,
   else IFX_ERROR.

*/
IFX_int_t DTI_device_moduleStop(
                        DTI_DeviceAccessFct_t   *pDeviceAccessFct,
                        DTI_DeviceSysInfo_t     *pDeviceSystemInfo,
                        DTI_DeviceCtx_t         **ppDtiDevCtx)
{
   IFX_int_t retVal = IFX_SUCCESS;

   if (pDeviceAccessFct == IFX_NULL)
   {
      DTI_PRN_USR_ERR_NL(DTI_DEV, DTI_PRN_LEVEL_ERR,
         ("Error: Device Module Stop - missing device handler."DTI_CRLF));

      return IFX_ERROR;
   }

   if (pDeviceAccessFct->pModuleDeleteFct != IFX_NULL)
   {
      retVal = pDeviceAccessFct->pModuleDeleteFct(pDeviceSystemInfo, ppDtiDevCtx);
   }
   else
   {
      DTI_PRN_USR_ERR_NL(DTI_DEV, DTI_PRN_LEVEL_ERR,
         ("Error: Module Stop - Dev[%s] ModuleDelete not implemented."DTI_CRLF,
           (pDeviceAccessFct->pDevIfName) ? pDeviceAccessFct->pDevIfName : "unknown"));

      return IFX_ERROR;
   }

   return retVal;
}

/**
   Write the Sytem Info of the current DTI Agent instance to the given buffer.

\param
   pDeviceSystemInfo - points to Sytem Info struct of the current agent instance.
\param
   pSysInfoBuffer    - points to the Sytem Info char buffer.
\param
   bufferSize        - buffer size.

\return
   Number of written bytes.
*/
IFX_int_t DTI_device_systemInfoWrite(
                        DTI_DeviceAccessFct_t   *pDeviceAccessFct,
                        DTI_DeviceSysInfo_t     *pDeviceSystemInfo,
                        IFX_char_t              *pSysInfoBuffer,
                        IFX_int_t               bufferSize)
{
   IFX_int_t       writtenLen = 0;

   if (pDeviceAccessFct == IFX_NULL)
   {
      DTI_PRN_USR_ERR_NL(DTI_DEV, DTI_PRN_LEVEL_ERR,
         ("Error: Device SysInfoWrite - missing device handler."DTI_CRLF));

      return IFX_ERROR;
   }

   if (pDeviceAccessFct->pSystemInfoWrite_deviceFct != IFX_NULL)
   {
      if (pDeviceSystemInfo)
      {
         writtenLen = pDeviceAccessFct->pSystemInfoWrite_deviceFct(
                     pDeviceSystemInfo, pSysInfoBuffer, bufferSize);
      }
   }
   else
   {
      (void)DTI_snprintf(&pSysInfoBuffer[writtenLen], bufferSize - writtenLen,
         "VendorName=%s", DTI_VENDOR_NAME_STR);
      writtenLen = (IFX_int_t)DTI_StrLen(&pSysInfoBuffer[writtenLen]) + 1;

      (void)DTI_snprintf(&pSysInfoBuffer[writtenLen], bufferSize - writtenLen,
         "BoardName=<na>");
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

   return writtenLen;
}

