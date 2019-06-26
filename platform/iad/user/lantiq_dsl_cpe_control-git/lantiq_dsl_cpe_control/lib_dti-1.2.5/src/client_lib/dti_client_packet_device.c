/******************************************************************************

                              Copyright (c) 2009
                            Lantiq Deutschland GmbH
                     Am Campeon 3; 85579 Neubiberg, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/** \file
   Debug and Trace Interface - Client Device Packet Handling.
*/

/* ============================================================================
   Includes
   ========================================================================= */
#include "dti_osmap.h"

#include "ifx_dti_protocol.h"
#include "ifx_dti_protocol_device.h"
#include "dti_protocol_interface.h"

#include "dti_client_packet_protocol.h"
#include "dti_client_packet_device.h"

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




/* ============================================================================
   Variables
   ========================================================================= */


/* ============================================================================
   Local Functions - packet processing
   ========================================================================= */


/*
   Setup Device Packet "DTI_PacketType_eDeviceReset"

   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dti_client_packet_devcie.h'
*/
IFX_int_t DTI_CLIENT_packetDeviceReset_Set(
                        DTI_Packet_t   *pDtiPacketSend,
                        IFX_uint_t     paylBufferSize,
                        IFX_uint32_t   mode,
                        IFX_uint32_t   mask)
{
   DTI_H2D_DeviceReset_t   *pPayloadSend;
   DTI_PTR_U               uPayload;

   if (pDtiPacketSend == IFX_NULL)
   {
      DTI_PRN_USR_ERR_NL(DTI_CLIENT, DTI_PRN_LEVEL_ERR,
         ("Error: Client Device Packet Reset - missing arg."DTI_CRLF));

      return IFX_ERROR;
   }

   uPayload.pUInt8 = (IFX_uint8_t *)pDtiPacketSend->payload;
   pPayloadSend    = (DTI_H2D_DeviceReset_t *)DTI_PTR_CAST_GET_ULONG(uPayload);
   if (pPayloadSend == IFX_NULL)
   {
      DTI_PRN_USR_ERR_NL(DTI_CLIENT, DTI_PRN_LEVEL_ERR,
         ("Error: Client Device Packet Reset - missaligned payload."DTI_CRLF));

      return IFX_ERROR;
   }

   (void)DTI_headerPacketTypeSet(
                        pDtiPacketSend,
                        (IFX_uint32_t)DTI_PacketType_eDeviceReset,
                        (IFX_uint32_t)DTI_e32Bit,
                        sizeof(DTI_H2D_DeviceReset_t));

   pPayloadSend->mode    = mode;
   pPayloadSend->mask[0] = mask;

   return IFX_SUCCESS;
}

/*
   Setup Device Packet "DTI_PacketType_eDeviceDownload"

   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dti_client_packet_devcie.h'
*/
IFX_int_t DTI_CLIENT_packetDeviceDownload_Set(
                        DTI_Packet_t   *pDtiPacketSend,
                        IFX_uint_t     paylBufferSize,
                        IFX_uint32_t   imageNum,
                        IFX_uint32_t   mode,
                        IFX_uint32_t   resetMask,
                        IFX_uint32_t   loadMask)
{
   DTI_H2D_DeviceDownload_t   *pPayloadSend;
   DTI_PTR_U                  uPayload;

   if (pDtiPacketSend == IFX_NULL)
   {
      DTI_PRN_USR_ERR_NL(DTI_CLIENT, DTI_PRN_LEVEL_ERR,
         ("Error: Client Device Packet Download - missing arg."DTI_CRLF));

      return IFX_ERROR;
   }

   uPayload.pUInt8 = (IFX_uint8_t *)pDtiPacketSend->payload;
   pPayloadSend    = (DTI_H2D_DeviceDownload_t *)DTI_PTR_CAST_GET_ULONG(uPayload);

   if (pPayloadSend == IFX_NULL)
   {
      DTI_PRN_USR_ERR_NL(DTI_CLIENT, DTI_PRN_LEVEL_ERR,
         ("Error: Client Device Packet Download - missaligned payload."DTI_CRLF));

      return IFX_ERROR;
   }

   (void)DTI_headerPacketTypeSet(
                        pDtiPacketSend,
                        (IFX_uint32_t)DTI_PacketType_eDeviceDownload,
                        (IFX_uint32_t)DTI_e32Bit,
                        sizeof(DTI_H2D_DeviceDownload_t));

   pPayloadSend->imageNum  = imageNum;
   pPayloadSend->mode      = mode;
   pPayloadSend->resetMask = resetMask;
   pPayloadSend->loadMask  = loadMask;

   return IFX_SUCCESS;
}

/*
   Setup Device Packet "DTI_PacketType_eDeviceOpen"

   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dti_client_packet_devcie.h'
*/
IFX_int_t DTI_CLIENT_packetDeviceOpen_Set(
                        DTI_Packet_t   *pDtiPacketSend,
                        IFX_uint_t     paylBufferSize)
{
   if (pDtiPacketSend == IFX_NULL)
   {
      DTI_PRN_USR_ERR_NL(DTI_CLIENT, DTI_PRN_LEVEL_ERR,
         ("Error: Client Device Packet DevOpen - missing arg."DTI_CRLF));

      return IFX_ERROR;
   }

   (void)DTI_headerPacketTypeSet(
                        pDtiPacketSend,
                        (IFX_uint32_t)DTI_PacketType_eDeviceOpen,
                        (IFX_uint32_t)DTI_e32Bit,
                        0);

   return IFX_SUCCESS;
}

/*
   Setup Device Packet "DTI_PacketType_eDeviceClose"

   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dti_client_packet_devcie.h'
*/
IFX_int_t DTI_CLIENT_packetDeviceClose_Set(
                        DTI_Packet_t   *pDtiPacketSend,
                        IFX_uint_t     paylBufferSize)
{
   if (pDtiPacketSend == IFX_NULL)
   {
      DTI_PRN_USR_ERR_NL(DTI_CLIENT, DTI_PRN_LEVEL_ERR,
         ("Error: Client Device Packet DevClose - missing arg."DTI_CRLF));

      return IFX_ERROR;
   }

   (void)DTI_headerPacketTypeSet(
                        pDtiPacketSend,
                        (IFX_uint32_t)DTI_PacketType_eDeviceClose,
                        (IFX_uint32_t)DTI_e32Bit,
                        0);

   return IFX_SUCCESS;
}


/*
   Setup Device Packet "DTI_PacketType_eRegisterLock"

\param
   pDtiPacketSend   - points to the IN packet.
\param
   paylBufferSize    - payload size of the IN packet.
\param
   lockOnOff         - 1: lock the driver, 0: unlock the driver.

\return
   IFX_SUCCESS setup done.
   IFX_ERROR   missing args or missaligned payload.
*/
IFX_int_t DTI_CLIENT_packetLock_Set(
                        DTI_Packet_t   *pDtiPacketSend,
                        IFX_uint_t     paylBufferSize,
                        IFX_uint_t     lockOnOff)
{
   DTI_H2D_DeviceLock_t *pPayloadSend;
   DTI_PTR_U            uPayload;

   if (pDtiPacketSend == IFX_NULL)
   {
      DTI_PRN_USR_ERR_NL(DTI_CLIENT, DTI_PRN_LEVEL_ERR,
         ("Error: Client Device Packet Lock - missing arg."DTI_CRLF));

      return IFX_ERROR;
   }

   uPayload.pUInt8 = (IFX_uint8_t *)pDtiPacketSend->payload;
   pPayloadSend    = (DTI_H2D_DeviceLock_t *)DTI_PTR_CAST_GET_ULONG(uPayload);

   if (pPayloadSend == IFX_NULL)
   {
      DTI_PRN_USR_ERR_NL(DTI_CLIENT, DTI_PRN_LEVEL_ERR,
         ("Error: Client Device Packet Lock - missaligned payload."DTI_CRLF));

      return IFX_ERROR;
   }

   (void)DTI_headerPacketTypeSet(
                        pDtiPacketSend,
                        (IFX_uint32_t)DTI_PacketType_eRegisterLock,
                        (IFX_uint32_t)DTI_e32Bit,
                        sizeof(DTI_H2D_DeviceLock_t));

   pPayloadSend->lock  = (lockOnOff) ? 1 : 0;

   return IFX_SUCCESS;
}

/*
   Setup Device Packet "DTI_PacketType_eRegisterGet"

   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dti_client_packet_devcie.h'
*/
IFX_int_t DTI_CLIENT_packetRegisterGet_Set(
                        DTI_Packet_t   *pDtiPacketSend,
                        IFX_uint_t     paylBufferSize,
                        IFX_uint32_t   regAddr,
                        IFX_uint_t     regCount)
{
   DTI_H2D_RegisterGet_t   *pPayloadSend;
   DTI_PTR_U               uPayload;

   if (pDtiPacketSend == IFX_NULL)
   {
      DTI_PRN_USR_ERR_NL(DTI_CLIENT, DTI_PRN_LEVEL_ERR,
         ("Error: Client Device Packet RegisterGet - missing arg."DTI_CRLF));

      return IFX_ERROR;
   }

   uPayload.pUInt8 = (IFX_uint8_t *)pDtiPacketSend->payload;
   pPayloadSend    = (DTI_H2D_RegisterGet_t *)DTI_PTR_CAST_GET_ULONG(uPayload);

   if (pPayloadSend == IFX_NULL)
   {
      DTI_PRN_USR_ERR_NL(DTI_CLIENT, DTI_PRN_LEVEL_ERR,
         ("Error: Client Device Packet RegisterGet - missaligned payload."DTI_CRLF));

      return IFX_ERROR;
   }

   (void)DTI_headerPacketTypeSet(
                        pDtiPacketSend,
                        (IFX_uint32_t)DTI_PacketType_eRegisterGet,
                        (IFX_uint32_t)DTI_e32Bit,
                        sizeof(DTI_H2D_RegisterGet_t));

   pPayloadSend->address = regAddr;
   pPayloadSend->count   = (IFX_uint32_t)regCount;

   return IFX_SUCCESS;
}

/**
   Setup Device Packet "DTI_PacketType_eRegisterSet"

   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dti_client_packet_devcie.h'
*/
IFX_int_t DTI_CLIENT_packetRegisterSet_Set(
                        DTI_Packet_t   *pDtiPacketSend,
                        IFX_uint_t     paylBufferSize,
                        IFX_uint32_t   regAddr,
                        IFX_uint32_t   *pRegData,
                        IFX_uint_t     regCount_32)
{
   IFX_uint_t              i;
   DTI_H2D_RegisterSet_t   *pPayloadSend;
   DTI_PTR_U               uPayload;

   if (pDtiPacketSend == IFX_NULL)
   {
      DTI_PRN_USR_ERR_NL(DTI_CLIENT, DTI_PRN_LEVEL_ERR,
         ("Error: Client Device Packet RegisterSet - missing arg."DTI_CRLF));

      return IFX_ERROR;
   }

   uPayload.pUInt8 = (IFX_uint8_t *)pDtiPacketSend->payload;
   pPayloadSend    = (DTI_H2D_RegisterSet_t *)DTI_PTR_CAST_GET_ULONG(uPayload);

   if (pPayloadSend == IFX_NULL)
   {
      DTI_PRN_USR_ERR_NL(DTI_CLIENT, DTI_PRN_LEVEL_ERR,
         ("Error: Client Device Packet RegisterSet - missaligned payload."DTI_CRLF));

      return IFX_ERROR;
   }

   regCount_32 = 1;

   (void)DTI_headerPacketTypeSet(
                        pDtiPacketSend,
                        (IFX_uint32_t)DTI_PacketType_eRegisterSet,
                        (IFX_uint32_t)DTI_e32Bit,
                        (sizeof(DTI_H2D_RegisterGet_t) + ((regCount_32 - 1) * sizeof(IFX_uint32_t))) );

   pPayloadSend->address = regAddr;
   for (i = 0; i < regCount_32; i++)
   {
      pPayloadSend->data[i] = pRegData[i];
   }

   return IFX_SUCCESS;
}

/*
   Setup Device Packet "DTI_PacketType_eConfigSet"

   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dti_client_packet_devcie.h'
*/
IFX_int_t DTI_CLIENT_packetConfigSet_Set(
                        DTI_Packet_t   *pDtiPacketSend,
                        IFX_uint_t     paylBufferSize,
                        IFX_uint32_t   key,
                        IFX_uint32_t   keyValue)
{
   DTI_H2D_DeviceConfigSet_t  *pPayloadSend;
   DTI_PTR_U                  uPayload;

   if (pDtiPacketSend == IFX_NULL)
   {
      DTI_PRN_USR_ERR_NL(DTI_CLIENT, DTI_PRN_LEVEL_ERR,
         ("Error: Client Device Packet ConfigSet - missing arg."DTI_CRLF));

      return IFX_ERROR;
   }

   uPayload.pUInt8 = (IFX_uint8_t *)pDtiPacketSend->payload;
   pPayloadSend    = (DTI_H2D_DeviceConfigSet_t *)DTI_PTR_CAST_GET_ULONG(uPayload);

   if (pPayloadSend == IFX_NULL)
   {
      DTI_PRN_USR_ERR_NL(DTI_CLIENT, DTI_PRN_LEVEL_ERR,
         ("Error: Client Device Packet ConfigSet - missaligned payload."DTI_CRLF));

      return IFX_ERROR;
   }

   (void)DTI_headerPacketTypeSet(
                        pDtiPacketSend,
                        (IFX_uint32_t)DTI_PacketType_eConfigSet,
                        (IFX_uint32_t)DTI_e32Bit,
                        sizeof(DTI_H2D_DeviceConfigSet_t));

   pPayloadSend->key    = key;
   pPayloadSend->value  = keyValue;

   return IFX_SUCCESS;
}


/*
   Setup Device Packet "DTI_PacketType_eConfigGet"

   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dti_client_packet_devcie.h'
*/
IFX_int_t DTI_CLIENT_packetConfigGet_Set(
                        DTI_Packet_t   *pDtiPacketSend,
                        IFX_uint_t     paylBufferSize,
                        IFX_uint32_t   key)
{
   DTI_H2D_DeviceConfigGet_t  *pPayloadSend;
   DTI_PTR_U                  uPayload;

   if (pDtiPacketSend == IFX_NULL)
   {
      DTI_PRN_USR_ERR_NL(DTI_CLIENT, DTI_PRN_LEVEL_ERR,
         ("Error: Client Device Packet ConfigSet - missing arg."DTI_CRLF));

      return IFX_ERROR;
   }

   uPayload.pUInt8 = (IFX_uint8_t *)pDtiPacketSend->payload;
   pPayloadSend    = (DTI_H2D_DeviceConfigGet_t *)DTI_PTR_CAST_GET_ULONG(uPayload);

   if (pPayloadSend == IFX_NULL)
   {
      DTI_PRN_USR_ERR_NL(DTI_CLIENT, DTI_PRN_LEVEL_ERR,
         ("Error: Client Device Packet ConfigGet - missaligned payload."DTI_CRLF));

      return IFX_ERROR;
   }

   (void)DTI_headerPacketTypeSet(
                        pDtiPacketSend,
                        (IFX_uint32_t)DTI_PacketType_eConfigGet,
                        (IFX_uint32_t)DTI_e32Bit,
                        sizeof(DTI_H2D_DeviceConfigGet_t));

   pPayloadSend->key    = key;

   return IFX_SUCCESS;
}

/*
   Setup Device Packet "DTI_PacketType_eMessageSend" (VINAX)

   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dti_client_packet_devcie.h'
*/
IFX_int_t DTI_CLIENT_packetVnxMessageSend_Set(
                        DTI_Packet_t   *pDtiPacketSend,
                        IFX_uint_t     paylBufferSize,
                        IFX_uint16_t   msgId,
                        IFX_uint16_t   msgIndex,
                        IFX_uint16_t   msgLength,
                        IFX_uint8_t    *pUsrMsgData,
                        IFX_uint_t     usrMsgDataSize_byte)
{
   union {
      DTI_H2D_vnxMessage32_t  *pMsg32;
      DTI_H2D_vnxMessage16_t  *pMsg16;
   } uPayloadSend;
   DTI_PTR_U        uPayload;

   if (pDtiPacketSend == IFX_NULL)
   {
      DTI_PRN_USR_ERR_NL(DTI_CLIENT, DTI_PRN_LEVEL_ERR,
         ("Error: Client Device Packet VNX MessageSend - missing arg."DTI_CRLF));

      return IFX_ERROR;
   }

   uPayload.pUInt8      = (IFX_uint8_t *)pDtiPacketSend->payload;
   uPayloadSend.pMsg32  = (DTI_H2D_vnxMessage32_t *)DTI_PTR_CAST_GET_ULONG(uPayload);

   if (uPayloadSend.pMsg32 == IFX_NULL)
   {
      DTI_PRN_USR_ERR_NL(DTI_CLIENT, DTI_PRN_LEVEL_ERR,
         ("Error: Client Device Packet VNX MessageSend - missaligned packet payload."DTI_CRLF));

      return IFX_ERROR;
   }

   if (msgId & DTI_VINAX_IFX_MSG_INDICATION)
   {
      uPayload.pUInt32  = (IFX_uint32_t *)DTI_PTR_CAST_GET_ULONG(uPayload);
      if (uPayload.pUInt32 == IFX_NULL)
      {
         DTI_PRN_USR_ERR_NL(DTI_CLIENT, DTI_PRN_LEVEL_ERR,
            ("Error: Client Device Packet VNX MessageSend - missaligned 32 bit user payload."DTI_CRLF));

         return IFX_ERROR;
      }

      (void)DTI_headerPacketTypeSet(
                     pDtiPacketSend,
                     (IFX_uint32_t)DTI_PacketType_eMessageSend,
                     (IFX_uint32_t)DTI_e32Bit,
                     ((sizeof(DTI_H2D_vnxMessage32_t) - sizeof(IFX_uint32_t)) + usrMsgDataSize_byte) );

      uPayloadSend.pMsg32->msgid  = (IFX_uint32_t)msgId;
      uPayloadSend.pMsg32->index  = (IFX_uint32_t)msgIndex;
      uPayloadSend.pMsg32->length = (IFX_uint32_t)msgLength;

      if (usrMsgDataSize_byte > 0)
      {
         DTI_MemCpy( uPayloadSend.pMsg32->data, pUsrMsgData, (IFX_int_t)usrMsgDataSize_byte);
      }
   }
   else
   {
      (void)DTI_headerPacketTypeSet(
                     pDtiPacketSend,
                     (IFX_uint32_t)DTI_PacketType_eMessageSend,
                     (IFX_uint32_t)DTI_e16Bit,
                     ((sizeof(DTI_H2D_vnxMessage16_t) - sizeof(IFX_uint16_t)) + usrMsgDataSize_byte) );

      uPayloadSend.pMsg16->msgid  = (IFX_uint16_t)msgId;
      uPayloadSend.pMsg16->index  = (IFX_uint16_t)msgIndex;
      uPayloadSend.pMsg16->length = (IFX_uint16_t)msgLength;

      DTI_MemCpy( uPayloadSend.pMsg16->data, pUsrMsgData, (IFX_int_t)usrMsgDataSize_byte);
   }

   return IFX_SUCCESS;
}

/*
   Setup Device Packet "DTI_PacketType_eMessageSend"

   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dti_client_packet_devcie.h'
*/
IFX_int_t DTI_CLIENT_packetGmxMessageSend_Set(
                        DTI_Packet_t   *pDtiPacketSend,
                        IFX_uint_t     paylBufferSize,
                        IFX_uint16_t   cmdWord,
                        IFX_uint16_t   lenField,
                        IFX_uint8_t    *pUsrMsgData,
                        IFX_uint_t     usrMsgDataSize_byte)
{
   union {
      DTI_H2D_gmxMessage16_t  *pMsg16;
   } uPayloadSend;
   DTI_PTR_U        uPayload;

   if (pDtiPacketSend == IFX_NULL)
   {
      DTI_PRN_USR_ERR_NL(DTI_CLIENT, DTI_PRN_LEVEL_ERR,
         ("Error: Client Device Packet GMX MessageSend - missing arg."DTI_CRLF));

      return IFX_ERROR;
   }

   uPayload.pUInt8      = (IFX_uint8_t *)pDtiPacketSend->payload;
   uPayloadSend.pMsg16  = (DTI_H2D_gmxMessage16_t *)DTI_PTR_CAST_GET_ULONG(uPayload);
   if (uPayloadSend.pMsg16 == IFX_NULL)
   {
      DTI_PRN_USR_ERR_NL(DTI_CLIENT, DTI_PRN_LEVEL_ERR,
         ("Error: Client Device Packet GMX MessageSend - missaligned packet payload."DTI_CRLF));

      return IFX_ERROR;
   }

   (void)DTI_headerPacketTypeSet(
                  pDtiPacketSend,
                  (IFX_uint32_t)DTI_PacketType_eMessageSend,
                  (IFX_uint32_t)DTI_e16Bit,
                  ((sizeof(DTI_H2D_gmxMessage16_t) - sizeof(IFX_uint16_t)) + usrMsgDataSize_byte) );

   uPayloadSend.pMsg16->cmdWord  = cmdWord;
   uPayloadSend.pMsg16->lenField = lenField;

   if (usrMsgDataSize_byte)
   {
      DTI_MemCpy( uPayloadSend.pMsg16->data, pUsrMsgData, (IFX_int_t)usrMsgDataSize_byte);
   }

   return IFX_SUCCESS;
}


/*
   Setup Device Packet "DTI_PacketType_eMessageSend"

   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dti_client_packet_devcie.h'
*/
IFX_int_t DTI_CLIENT_packetMessageSend_Set(
                        DTI_Packet_t   *pDtiPacketSend,
                        IFX_uint_t     paylBufferSize,
                        IFX_uint_t     payloadType,
                        IFX_uint8_t    *pUsrMsgData,
                        IFX_uint_t     usrMsgDataSize_byte)
{
   union {
      DTI_H2D_rawMessage8_t   *pMsg8;
      DTI_H2D_rawMessage16_t  *pMsg16;
      DTI_H2D_rawMessage32_t  *pMsg32;
   } uPayloadSend;
   DTI_PTR_U        uPayload;

   if (pDtiPacketSend == IFX_NULL)
   {
      DTI_PRN_USR_ERR_NL(DTI_CLIENT, DTI_PRN_LEVEL_ERR,
         ("Error: Client Packet MessageSend - missing arg."DTI_CRLF));

      return IFX_ERROR;
   }

   uPayload.pUInt8      = (IFX_uint8_t *)pDtiPacketSend->payload;
   uPayloadSend.pMsg32  = (DTI_H2D_rawMessage32_t *)DTI_PTR_CAST_GET_ULONG(uPayload);

   if (uPayloadSend.pMsg32 == IFX_NULL)
   {
      DTI_PRN_USR_ERR_NL(DTI_CLIENT, DTI_PRN_LEVEL_ERR,
         ("Error: Client Device Packet MessageSend - missaligned packet payload."DTI_CRLF));

      return IFX_ERROR;
   }


   switch(payloadType)
   {
      case DTI_eMixed:
      case DTI_e8Bit:
         break;

      case DTI_e16Bit:
         if (usrMsgDataSize_byte & 0x1)
         {
            DTI_PRN_USR_ERR_NL(DTI_CLIENT, DTI_PRN_LEVEL_ERR,
               ("Error: Client Packet MessageSend - invalid 16 bit user size = %d."DTI_CRLF,
                 usrMsgDataSize_byte));
            return IFX_ERROR;
         }
         break;

      case DTI_e32Bit:
         if (usrMsgDataSize_byte & 0x3)
         {
            DTI_PRN_USR_ERR_NL(DTI_CLIENT, DTI_PRN_LEVEL_ERR,
               ("Error: Client Packet MessageSend - invalid 32 bit user size = %d."DTI_CRLF,
                 usrMsgDataSize_byte));
            return IFX_ERROR;
         }
         break;

      default:
         DTI_PRN_USR_ERR_NL(DTI_CLIENT, DTI_PRN_LEVEL_ERR,
            ("Error: Client Packet MessageSend - invalid payload type (8, 16, 32 bit)."DTI_CRLF));

         return IFX_ERROR;
   }


   (void)DTI_headerPacketTypeSet(
                  pDtiPacketSend,
                  (IFX_uint32_t)DTI_PacketType_eMessageSend,
                  (IFX_uint32_t)payloadType,
                  usrMsgDataSize_byte );

   if (usrMsgDataSize_byte)
   {
      DTI_MemCpy( uPayloadSend.pMsg8->data, pUsrMsgData, (IFX_int_t)usrMsgDataSize_byte);
   }

   return IFX_SUCCESS;
}

/*
   Setup Device Packet "DTI_PacketType_eTraceBufferConfigSet"

   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dti_client_packet_devcie.h'
*/
IFX_int_t DTI_CLIENT_packetTraceBufferCfgSet_Set(
                        DTI_Packet_t   *pDtiPacketSend,
                        IFX_uint_t     paylBufferSize,
                        IFX_uint32_t   operationMode,
                        IFX_uint32_t   bufferSize_byte,
                        IFX_uint32_t   nfcThreshold)
{
   DTI_H2D_TraceConfigSet_t   *pPayloadSend;
   DTI_PTR_U                  uPayload;

   if (pDtiPacketSend == IFX_NULL)
   {
      DTI_PRN_USR_ERR_NL(DTI_CLIENT, DTI_PRN_LEVEL_ERR,
         ("Error: Client Device Packet TraceBufferCfgSet - missing arg."DTI_CRLF));

      return IFX_ERROR;
   }

   uPayload.pUInt8 = (IFX_uint8_t *)pDtiPacketSend->payload;
   pPayloadSend    = (DTI_H2D_TraceConfigSet_t *)DTI_PTR_CAST_GET_ULONG(uPayload);

   if (pPayloadSend == IFX_NULL)
   {
      DTI_PRN_USR_ERR_NL(DTI_CLIENT, DTI_PRN_LEVEL_ERR,
         ("Error: Client Device Packet TraceBufferCfgSet - missaligned payload."DTI_CRLF));

      return IFX_ERROR;
   }

   (void)DTI_headerPacketTypeSet(
                        pDtiPacketSend,
                        (IFX_uint32_t)DTI_PacketType_eTraceBufferConfigSet,
                        (IFX_uint32_t)DTI_e32Bit,
                        sizeof(DTI_H2D_TraceConfigSet_t));

   pPayloadSend->mode         = operationMode;
   pPayloadSend->size         = bufferSize_byte;
   pPayloadSend->nfcThreshold = nfcThreshold;

   return IFX_SUCCESS;
}

/*
   Setup Device Packet "DTI_PacketType_eTraceBufferReset"

   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dti_client_packet_devcie.h'
*/
IFX_int_t DTI_CLIENT_packetTraceBufferReset_Set(
                        DTI_Packet_t   *pDtiPacketSend,
                        IFX_uint_t     paylBufferSize)
{
   if (pDtiPacketSend == IFX_NULL)
   {
      DTI_PRN_USR_ERR_NL(DTI_CLIENT, DTI_PRN_LEVEL_ERR,
         ("Error: Client Device Packet TraceBufferReset - missing arg."DTI_CRLF));

      return IFX_ERROR;
   }

   (void)DTI_headerPacketTypeSet(
                        pDtiPacketSend,
                        (IFX_uint32_t)DTI_PacketType_eTraceBufferReset,
                        (IFX_uint32_t)DTI_e8Bit, 0);

   return IFX_SUCCESS;
}

/*
   Setup Device Packet "DTI_PacketType_eTraceBufferStatusGet"

   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dti_client_packet_devcie.h'
*/
IFX_int_t DTI_CLIENT_packetTraceBufferStatGet_Set(
                        DTI_Packet_t   *pDtiPacketSend,
                        IFX_uint_t     paylBufferSize)
{
   if (pDtiPacketSend == IFX_NULL)
   {
      DTI_PRN_USR_ERR_NL(DTI_CLIENT, DTI_PRN_LEVEL_ERR,
         ("Error: Client Device Packet TraceBufferStatGet - missing arg."DTI_CRLF));

      return IFX_ERROR;
   }

   (void)DTI_headerPacketTypeSet(
                        pDtiPacketSend,
                        (IFX_uint32_t)DTI_PacketType_eTraceBufferStatusGet,
                        (IFX_uint32_t)DTI_e32Bit, 0);

   return IFX_SUCCESS;
}

/**
   Setup Device Packet "DTI_PacketType_eTraceBufferGet"

   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dti_client_packet_devcie.h'
*/
IFX_int_t DTI_CLIENT_packetTraceBufferGet_Set(
                        DTI_Packet_t   *pDtiPacketSend,
                        IFX_uint_t     paylBufferSize,
                        IFX_uint32_t   readSize_byte)
{
   DTI_H2D_TraceBufferGet_t   *pPayloadSend;
   DTI_PTR_U                  uPayload;

   if (pDtiPacketSend == IFX_NULL)
   {
      DTI_PRN_USR_ERR_NL(DTI_CLIENT, DTI_PRN_LEVEL_ERR,
         ("Error: Client Device Packet TraceBufferGet - missing arg."DTI_CRLF));

      return IFX_ERROR;
   }

   uPayload.pUInt8 = (IFX_uint8_t *)pDtiPacketSend->payload;
   pPayloadSend    = (DTI_H2D_TraceBufferGet_t *)DTI_PTR_CAST_GET_ULONG(uPayload);

   if (pPayloadSend == IFX_NULL)
   {
      DTI_PRN_USR_ERR_NL(DTI_CLIENT, DTI_PRN_LEVEL_ERR,
         ("Error: Client Device Packet TraceBufferGet - missaligned payload."DTI_CRLF));

      return IFX_ERROR;
   }

   (void)DTI_headerPacketTypeSet(
                        pDtiPacketSend,
                        (IFX_uint32_t)DTI_PacketType_eTraceBufferGet,
                        (IFX_uint32_t)DTI_e32Bit,
                        sizeof(DTI_H2D_TraceBufferGet_t));

   pPayloadSend->size = readSize_byte;

   return IFX_SUCCESS;
}

/*
   Setup Device Packet "DTI_PacketType_eDebugGet"

   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dti_client_packet_devcie.h'
*/
IFX_int_t DTI_CLIENT_packetDebugGet_Set(
                        DTI_Packet_t   *pDtiPacketSend,
                        IFX_uint_t     paylBufferSize,
                        IFX_uint32_t   offset,
                        IFX_uint_t     destType,
                        IFX_uint_t     count_32)
{
   DTI_H2D_DebugRead_t  *pPayloadSend;
   DTI_PTR_U            uPayload;

   if (pDtiPacketSend == IFX_NULL)
   {
      DTI_PRN_USR_ERR_NL(DTI_CLIENT, DTI_PRN_LEVEL_ERR,
         ("Error: Client Device Packet DebugGet - missing arg."DTI_CRLF));

      return IFX_ERROR;
   }

   uPayload.pUInt8 = (IFX_uint8_t *)pDtiPacketSend->payload;
   pPayloadSend    = (DTI_H2D_DebugRead_t *)DTI_PTR_CAST_GET_ULONG(uPayload);

   if (pPayloadSend == IFX_NULL)
   {
      DTI_PRN_USR_ERR_NL(DTI_CLIENT, DTI_PRN_LEVEL_ERR,
         ("Error: Client Device Packet DebugGet - missaligned payload."DTI_CRLF));

      return IFX_ERROR;
   }

   (void)DTI_headerPacketTypeSet(
                        pDtiPacketSend,
                        (IFX_uint32_t)DTI_PacketType_eDebugGet,
                        (IFX_uint32_t)DTI_e32Bit,
                        sizeof(DTI_H2D_DebugRead_t));

   pPayloadSend->offset = offset;
   pPayloadSend->type   = (IFX_uint32_t)destType;
   pPayloadSend->count  = (IFX_uint32_t)count_32;

   return IFX_SUCCESS;
}

/*
   Setup Device Packet "DTI_PacketType_eDebugSet"

   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dti_client_packet_devcie.h'
*/
IFX_int_t DTI_CLIENT_packetDebugSet_Set(
                        DTI_Packet_t   *pDtiPacketSend,
                        IFX_uint_t     paylBufferSize,
                        IFX_uint32_t   offset,
                        IFX_uint_t     destType,
                        IFX_uint32_t   *pDbgData,
                        IFX_uint_t     count_32)
{
   DTI_H2D_DebugWrite_t *pPayloadSend;
   DTI_PTR_U            uPayload;

   if (pDtiPacketSend == IFX_NULL)
   {
      DTI_PRN_USR_ERR_NL(DTI_CLIENT, DTI_PRN_LEVEL_ERR,
         ("Error: Client Device Packet DebugSet - missing arg."DTI_CRLF));

      return IFX_ERROR;
   }

   uPayload.pUInt8 = (IFX_uint8_t *)pDtiPacketSend->payload;
   pPayloadSend    = (DTI_H2D_DebugWrite_t *)DTI_PTR_CAST_GET_ULONG(uPayload);

   if (pPayloadSend == IFX_NULL)
   {
      DTI_PRN_USR_ERR_NL(DTI_CLIENT, DTI_PRN_LEVEL_ERR,
         ("Error: Client Device Packet DebugSet - missaligned payload."DTI_CRLF));

      return IFX_ERROR;
   }

   (void)DTI_headerPacketTypeSet(
                        pDtiPacketSend,
                        (IFX_uint32_t)DTI_PacketType_eDebugSet,
                        (IFX_uint32_t)DTI_e32Bit,
                        sizeof(DTI_H2D_DebugWrite_t) + ((count_32 - 1) * sizeof(IFX_uint32_t)));

   pPayloadSend->offset = offset;
   pPayloadSend->type   = (IFX_uint32_t)destType;

   DTI_MemCpy(pPayloadSend->data, pDbgData, (IFX_int_t)(count_32 * sizeof(IFX_uint32_t)) );

   return IFX_SUCCESS;
}

