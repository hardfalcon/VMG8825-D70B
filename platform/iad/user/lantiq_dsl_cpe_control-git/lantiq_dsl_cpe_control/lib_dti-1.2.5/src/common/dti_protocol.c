/******************************************************************************

                              Copyright (c) 2009
                            Lantiq Deutschland GmbH
                     Am Campeon 3; 85579 Neubiberg, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/** \file
   Debug and Trace Interface - Protocol.
*/

/* ============================================================================
   Includes
   ========================================================================= */

#include "dti_osmap.h"
#include "ifx_dti_protocol.h"
#include "dti_protocol_interface.h"

/* ============================================================================
   Defines
   ========================================================================= */
#ifdef DTI_STATIC
#  undef DTI_STATIC
#endif

#ifdef DTI_DEBUG
#  define DTI_STATIC
#else
#  define DTI_STATIC   static
#endif

/* ============================================================================
   Local Function Declaration
   ========================================================================= */



/* ============================================================================
   Variables
   ========================================================================= */

IFXOS_PRN_USR_MODULE_CREATE(DTI_PROTOCOL, IFXOS_PRN_LEVEL_HIGH);

/* ============================================================================
   Local Function defintions
   ========================================================================= */



/* ============================================================================
   Global Function defintions
   ========================================================================= */

/**
   This function takes over the info from the IN header into the OUT header.

   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dti_protocol_interface.h'

*/
IFX_int_t DTI_headerResponceSet(
                        const DTI_PacketHeader_t   *pHdrIn,
                        DTI_PacketHeader_t         *pHdrOut,
                        IFX_uint32_t               nPayloadSize,
                        DTI_PacketError_t          packetError)
{
   if (!pHdrIn || !pHdrOut)
   {
      IFXOS_PRN_USR_ERR_NL(DTI_PROTOCOL, IFXOS_PRN_LEVEL_ERR,
         ("Error: Packet Responce Hdr set - NULL ptr args."IFXOS_CRLF));

      return IFX_ERROR;
   }

   /* take over the IN data */
   pHdrOut->packetType    = pHdrIn->packetType;
   pHdrOut->port          = pHdrIn->port;
   pHdrOut->tan           = pHdrIn->tan;
   pHdrOut->packetOptions = pHdrIn->packetOptions;

   /* set fixed values */
   pHdrOut->magic         = DTI_MAGIC;

   /* set variable values */
   pHdrOut->payloadSize   = nPayloadSize;
   pHdrOut->error         = (IFX_uint32_t)packetError;

   return IFX_SUCCESS;
}

/**
   This function setup a responce of a given packet

   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dti_protocol_interface.h'
*/
IFX_int_t DTI_packetResponceSet(
                        const DTI_Packet_t   *pDtiPacketIn,
                        DTI_Packet_t         *pDtiPacketOut,
                        DTI_PacketError_t    packetError,
                        IFX_uint32_t         payloadSize,
                        IFX_uint32_t         bufferOutSize,
                        IFX_boolean_t        bCpyPayload)
{
   if (!pDtiPacketOut || !pDtiPacketIn)
   {
      IFXOS_PRN_USR_ERR_NL(DTI_PROTOCOL, IFXOS_PRN_LEVEL_ERR,
         ("Error: Packet Responce set - NULL ptr args."IFXOS_CRLF));

      return IFX_ERROR;
   }

   (void)DTI_headerResponceSet(
            &pDtiPacketIn->header, &pDtiPacketOut->header,
            payloadSize,
            packetError);

   if ((payloadSize + sizeof(DTI_PacketHeader_t)) > bufferOutSize)
   {
      IFXOS_PRN_USR_ERR_NL(DTI_PROTOCOL, IFXOS_PRN_LEVEL_WRN,
         ("WARNING: Packet Responce set - cut payload."IFXOS_CRLF));

      pDtiPacketOut->header.payloadSize = (bufferOutSize - sizeof(DTI_PacketHeader_t)) & ~0x3;
   }

   /* setup payload */
   if (bCpyPayload == IFX_TRUE)
   {
      if (pDtiPacketOut->header.payloadSize > 0)
      {
         DTI_MemCpy(
               (IFX_char_t *)pDtiPacketOut->payload,
               (IFX_char_t *)pDtiPacketIn->payload,
               (IFX_int_t)pDtiPacketOut->header.payloadSize );
      }
   }

   return IFX_SUCCESS;
}


/**
   This function setup an "unknown packet"

   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dti_protocol_interface.h'
*/
IFX_int_t DTI_packetUnknownSet(
                        const DTI_Packet_t   *pDtiPacketIn,
                        DTI_Packet_t         *pDtiPacketOut,
                        IFX_uint32_t         bufferOutSize)
{
   if (!pDtiPacketOut || !pDtiPacketIn)
   {
      IFXOS_PRN_USR_ERR_NL(DTI_PROTOCOL, IFXOS_PRN_LEVEL_ERR,
         ("Error: Packet Unknown set - NULL ptr args."IFXOS_CRLF));

      return IFX_ERROR;
   }

   (void)DTI_headerResponceSet(
            &pDtiPacketIn->header, &pDtiPacketOut->header,
            pDtiPacketIn->header.payloadSize, DTI_eErrInvalidPacketType);

   /* setup payload */
   if ((pDtiPacketIn->header.payloadSize + sizeof(DTI_PacketHeader_t)) > bufferOutSize)
   {
      IFXOS_PRN_USR_ERR_NL(DTI_PROTOCOL, IFXOS_PRN_LEVEL_WRN,
         ("Error: Packet Unknown set - cut payload."IFXOS_CRLF));

      pDtiPacketOut->header.payloadSize = (bufferOutSize - sizeof(DTI_PacketHeader_t)) & ~0x3;
   }

   if (pDtiPacketOut->header.payloadSize > 0)
   {
      DTI_MemCpy(
            (IFX_char_t *)pDtiPacketOut->payload,
            (IFX_char_t *)pDtiPacketIn->payload,
            (IFX_int_t)pDtiPacketOut->header.payloadSize );
   }

   return IFX_SUCCESS;
}


/**
   This function setup an "error packet"

   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dti_protocol_interface.h'
*/
IFX_int_t DTI_packetErrorSet(
                        const DTI_Packet_t   *pDtiPacketIn,
                        DTI_Packet_t         *pDtiPacketOut,
                        IFX_uint32_t         bufferOutSize,
                        DTI_PacketError_t    packetError)
{
   if (!pDtiPacketOut || !pDtiPacketIn)
   {
      IFXOS_PRN_USR_ERR_NL(DTI_PROTOCOL, IFXOS_PRN_LEVEL_ERR,
         ("Error: Packet Unknown set - NULL ptr args."IFXOS_CRLF));

      return IFX_ERROR;
   }

   /* take over some the IN data */
   pDtiPacketOut->header.port          = pDtiPacketIn->header.port;
   pDtiPacketOut->header.tan           = pDtiPacketIn->header.tan;
   pDtiPacketOut->header.packetOptions = pDtiPacketIn->header.packetOptions;

   /* set fixed values */
   pDtiPacketOut->header.magic         = DTI_MAGIC;
   pDtiPacketOut->header.payloadSize   = 0;

   pDtiPacketOut->header.error         = (IFX_uint32_t)packetError;
   pDtiPacketOut->header.packetType    = (IFX_uint32_t)DTI_PacketType_eError;

   /* no payload */

   return IFX_SUCCESS;
}



/**
   This function setup an "loopback packet"

   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dti_protocol_interface.h'
*/
IFX_int_t DTI_packetLoopBackSet(
                        const DTI_Packet_t   *pDtiPacketIn,
                        DTI_Packet_t         *pDtiPacketOut,
                        IFX_uint32_t         bufferOutSize)
{
   if (!pDtiPacketOut || !pDtiPacketIn)
   {
      IFXOS_PRN_USR_ERR_NL(DTI_PROTOCOL, IFXOS_PRN_LEVEL_ERR,
         ("Error: Packet LoopBack set - NULL ptr args."IFXOS_CRLF));

      return IFX_ERROR;
   }

   (void)DTI_headerResponceSet(
            &pDtiPacketIn->header, &pDtiPacketOut->header,
            pDtiPacketIn->header.payloadSize, DTI_eNoError);


   /* setup payload */
   if ((pDtiPacketIn->header.payloadSize + sizeof(DTI_PacketHeader_t)) > bufferOutSize)
   {
      IFXOS_PRN_USR_ERR_NL(DTI_PROTOCOL, IFXOS_PRN_LEVEL_WRN,
         ("WARNING: Packet LoopBack set - cut payload."IFXOS_CRLF));

      pDtiPacketOut->header.payloadSize = (bufferOutSize - sizeof(DTI_PacketHeader_t)) & ~0x3;
   }

   if (pDtiPacketOut->header.payloadSize > 0)
   {
      DTI_MemCpy(
            (IFX_char_t *)pDtiPacketOut->payload,
            (IFX_char_t *)pDtiPacketIn->payload,
            (IFX_int_t)pDtiPacketOut->header.payloadSize );
   }

   return IFX_SUCCESS;
}


/**
   Setup the fixed and packet type specific fields

   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dti_protocol_interface.h'
*/
IFX_int_t DTI_headerPacketTypeSet(
                        DTI_Packet_t   *pDtiPacketIn,
                        IFX_uint32_t   packetType,
                        IFX_uint32_t   packetOptions,
                        IFX_uint32_t   payloadSize)
{
   DTI_MemSet(pDtiPacketIn, 0x00, sizeof(DTI_PacketHeader_t));

   pDtiPacketIn->header.magic          = DTI_MAGIC;
   pDtiPacketIn->header.packetType     = packetType;
   pDtiPacketIn->header.packetOptions  = packetOptions;
   pDtiPacketIn->header.payloadSize    = payloadSize;

   return IFX_SUCCESS;
}


/* ============================================================================
   Common functions, may shift to IFXOS
   ========================================================================= */

/**
   Converts a given ASCII string to a unsigned long value.

   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dti_protocol_interface.h'

\remark
   Shift to IFXOS ??
*/
IFX_int_t DTI_convertStrToUl(
                        IFX_char_t     *pIn,
                        IFX_int_t      base,
                        IFX_ulong_t    *pRetVal)
{
   IFX_char_t  *pEnd;
   IFX_ulong_t ulVal;

   if ( (pIn == IFX_NULL) || (pRetVal == IFX_NULL) )
   {
      IFXOS_PRN_USR_ERR_NL(DTI_PROTOCOL, IFXOS_PRN_LEVEL_ERR,
         ("Error: DTI StrToUl - NULL ptr."IFXOS_CRLF));

      return IFX_ERROR;
   }

   if (DTI_StrLen(pIn) > 0)
   {
      errno = 0;
      ulVal = (IFX_ulong_t)DTI_StrToUl(pIn, &pEnd, (IFX_int_t)base);

      if (errno)
      {
         IFXOS_PRN_USR_ERR_NL(DTI_PROTOCOL, IFXOS_PRN_LEVEL_ERR,
            ("Error: DTI StrToUl - convertion failed, <%s>, base = %d,  errno = %d."IFXOS_CRLF,
              pIn, base, errno));

         return IFX_ERROR;
      }
   }
   else
   {
      IFXOS_PRN_USR_ERR_NL(DTI_PROTOCOL, IFXOS_PRN_LEVEL_ERR,
         ("Error: DTI StrToUl - zero len str."IFXOS_CRLF));

      return IFX_ERROR;
   }

   *pRetVal = ulVal;

   return IFX_SUCCESS;
}



