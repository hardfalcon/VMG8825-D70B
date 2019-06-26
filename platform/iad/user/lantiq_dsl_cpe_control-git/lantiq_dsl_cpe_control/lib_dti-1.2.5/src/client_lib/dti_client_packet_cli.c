/******************************************************************************

                              Copyright (c) 2009
                            Lantiq Deutschland GmbH
                     Am Campeon 3; 85579 Neubiberg, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/** \file
   Debug and Trace Interface - Client Command Line Interface (CLI) Packet Handling.
*/

/* ============================================================================
   Includes
   ========================================================================= */
#include "dti_osmap.h"

#include "ifx_dti_protocol.h"
#include "ifx_dti_protocol_cli.h"
#include "dti_protocol_interface.h"

#include "dti_client_packet_protocol.h"
#include "dti_client_packet_cli.h"

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
   Setup Device Packet "DTI_PacketType_eCliString"

   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dti_client_packet_cli.h'
*/
IFX_int_t DTI_CLIENT_packetCliString_Set(
                        DTI_Packet_t   *pDtiPacketSend,
                        IFX_uint_t     paylBufferSize,
                        IFX_char_t     *pCliString,
                        IFX_uint_t     userPayloadSize)
{
   IFX_uint_t  payloadSize, addStrTermination = 0;
   DTI_PTR_U   uPayload;

   if ((pDtiPacketSend == IFX_NULL) || (pCliString == IFX_NULL))
   {
      DTI_PRN_USR_ERR_NL(DTI_CLIENT, DTI_PRN_LEVEL_ERR,
         ("Error: Client Cli Packet String - missing arg, <%s>."DTI_CRLF,
           (pDtiPacketSend) ? "str arg" : "send packet" ));

      return IFX_ERROR;
   }

   if (userPayloadSize)
   {
      payloadSize = userPayloadSize;
      if (pCliString[userPayloadSize - 1] != '\0')
      {
         addStrTermination = 1;
      }
   }
   else
   {
      payloadSize = (IFX_uint_t)DTI_StrLen(pCliString) + 1;
   }

   if ( (payloadSize + addStrTermination) > (paylBufferSize - sizeof(DTI_PacketHeader_t)) )
   {
      DTI_PRN_USR_ERR_NL(DTI_CLIENT, DTI_PRN_LEVEL_ERR,
         ("Error: Client Cli Packet String - buffer overflow, str = %d > buffer = %d."DTI_CRLF,
           (payloadSize + addStrTermination), (paylBufferSize - sizeof(DTI_PacketHeader_t)) ));

      return IFX_ERROR;
   }

   uPayload.pUInt8 = (IFX_uint8_t *)pDtiPacketSend->payload;

   (void)DTI_headerPacketTypeSet(
                        pDtiPacketSend,
                        (IFX_uint32_t)DTI_PacketType_eCliString,
                        (IFX_uint32_t)DTI_e8Bit,
                        (payloadSize + addStrTermination));

   DTI_MemCpy(uPayload.pUInt8, pCliString, (IFX_int_t)payloadSize);
   if (addStrTermination == 1)
   {
      uPayload.pUInt8[payloadSize] = '\0';
   }

   return IFX_SUCCESS;
}

/*
   Setup Device Packet "DTI_PacketType_eCliInfoGet" to request the CLI info
   from the DTI Agent.

   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dti_client_packet_cli.h'
*/
IFX_int_t DTI_CLIENT_packetCliInfoGet_Set(
                        DTI_Packet_t   *pDtiPacketSend,
                        IFX_uint_t     paylBufferSize)
{
   if (pDtiPacketSend == IFX_NULL)
   {
      DTI_PRN_USR_ERR_NL(DTI_CLIENT, DTI_PRN_LEVEL_ERR,
         ("Error: Client CLI Packet INFO GET - missing arg."DTI_CRLF));

      return IFX_ERROR;
   }

   (void)DTI_headerPacketTypeSet(
                        pDtiPacketSend,
                        (IFX_uint32_t)DTI_PacketType_eCliInfoGet,
                        (IFX_uint32_t)DTI_e8Bit, 0);

   return IFX_SUCCESS;

}

