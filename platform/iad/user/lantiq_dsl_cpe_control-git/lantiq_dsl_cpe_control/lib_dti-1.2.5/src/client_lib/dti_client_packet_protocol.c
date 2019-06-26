/******************************************************************************

                              Copyright (c) 2009
                            Lantiq Deutschland GmbH
                     Am Campeon 3; 85579 Neubiberg, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/** \file
   Debug and Trace Interface - Client Protocol Packet Handling.
*/

/* ============================================================================
   Includes
   ========================================================================= */

#include "dti_osmap.h"

#include "ifx_dti_protocol.h"
#include "ifx_dti_protocol_cli.h"
#include "dti_protocol_interface.h"

#include "dti_client_packet_protocol.h"

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

IFXOS_PRN_USR_MODULE_CREATE(DTI_CLIENT, IFXOS_PRN_LEVEL_HIGH);


/* ============================================================================
   Local Function defintions
   ========================================================================= */

/* ============================================================================
   Global Function defintions
   ========================================================================= */

/*
   Setup Protocol Packet Info with the given settings.

   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dti_client_packet_protocol.h'
*/
IFX_int_t DTI_CLIENT_packetProt_Set(
                        DTI_Packet_t      *pDtiPacketSend,
                        IFX_uint_t        packetBufferSize,
                        DTI_PacketType_t  packetType,
                        DTI_PayloadType_t payloadType,
                        IFX_uint8_t       *pPacketPayload,
                        IFX_uint_t        payloadSize_byte)
{
   DTI_PTR_U            uPayload, uPayloadSend;

   if (pDtiPacketSend == IFX_NULL)
   {
      DTI_PRN_USR_ERR_NL(DTI_CLIENT, DTI_PRN_LEVEL_ERR,
         ("Error: Client Prot Packet Set - missing arg."DTI_CRLF));

      return IFX_ERROR;
   }

   if (packetBufferSize < (sizeof(DTI_PacketHeader_t) + payloadSize_byte) )
   {
      DTI_PRN_USR_ERR_NL(DTI_CLIENT, DTI_PRN_LEVEL_ERR,
         ("Error: Client Prot Packet Set - out buffer to small."DTI_CRLF));

      return IFX_ERROR;
   }

   if ( (pPacketPayload != IFX_NULL) && (payloadSize_byte > 0) )
   {
      uPayload.pUInt8 = (IFX_uint8_t *)pDtiPacketSend->payload;
      switch(payloadType)
      {
         case DTI_e16Bit:
            uPayloadSend.pUInt16 = (IFX_uint16_t *)DTI_PTR_CAST_GET_UINT16(uPayload);
            break;
         case DTI_e32Bit:
            uPayloadSend.pUInt32 = (IFX_uint32_t *)DTI_PTR_CAST_GET_UINT32(uPayload);
            break;
         case DTI_e8Bit:
         case DTI_eMixed:
         default:
            uPayloadSend.pUInt8  = (IFX_uint8_t  *)DTI_PTR_CAST_GET_UINT8(uPayload);
      }

      if (uPayloadSend.pUInt8 == IFX_NULL)
      {
         DTI_PRN_USR_ERR_NL(DTI_CLIENT, DTI_PRN_LEVEL_ERR,
            ("Error: Client Prot Packet Set - missaligned payload."DTI_CRLF));

         return IFX_ERROR;
      }

      DTI_MemCpy(uPayloadSend.pUInt8, pPacketPayload, (IFX_int_t)payloadSize_byte);
   }

   (void)DTI_headerPacketTypeSet(
                        pDtiPacketSend,
                        (IFX_uint32_t)packetType,
                        (IFX_uint32_t)payloadType,
                        payloadSize_byte );

   return IFX_SUCCESS;
}

/*
   Setup Protocol Loopback Packet Infof with the given settings.

   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dti_client_packet_protocol.h'
*/
IFX_int_t DTI_CLIENT_packetProtLoopBack_Set(
                        DTI_Packet_t      *pDtiPacketSend,
                        IFX_uint_t        packetBufferSize,
                        DTI_PayloadType_t packetPayloadType,
                        IFX_uint8_t       *pPacketPayload,
                        IFX_uint_t        payloadSize_byte)
{
   if (DTI_CLIENT_packetProt_Set(
                     pDtiPacketSend, packetBufferSize,
                     DTI_PacketType_eLoopback, packetPayloadType,
                     pPacketPayload, payloadSize_byte) != IFX_SUCCESS)
   {
      DTI_PRN_USR_ERR_NL(DTI_CLIENT, DTI_PRN_LEVEL_ERR,
         ("Error: Client Prot LOOPBACK Packet Set - setup."DTI_CRLF));

      return IFX_ERROR;
   }

   return IFX_SUCCESS;
}

/**
   Setup Protocol Error Packet with the given settings.

   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dti_client_packet_protocol.h'
*/
IFX_int_t DTI_CLIENT_packetProtError_Set(
                        DTI_Packet_t      *pDtiPacketSend,
                        IFX_uint_t        packetBufferSize,
                        DTI_PacketError_t packetError)
{
   if (pDtiPacketSend == IFX_NULL)
   {
      DTI_PRN_USR_ERR_NL(DTI_CLIENT, DTI_PRN_LEVEL_ERR,
         ("Error: Client Prot Error Packet Set - missing arg."DTI_CRLF));

      return IFX_ERROR;
   }

   (void)DTI_headerPacketTypeSet(
                        pDtiPacketSend,
                        (IFX_uint32_t)DTI_PacketType_eError,
                        (IFX_uint32_t)DTI_e8Bit, 0 );

   pDtiPacketSend->header.error = (IFX_uint32_t)packetError;

   return IFX_SUCCESS;
}

/*
   Setup Protocol System Info Get Packet.

   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dti_client_packet_protocol.h'
*/
IFX_int_t DTI_CLIENT_packetProtSysInfo_Set(
                        DTI_Packet_t      *pDtiPacketSend,
                        IFX_uint_t        packetBufferSize)
{
   if (DTI_CLIENT_packetProt_Set(
                     pDtiPacketSend, packetBufferSize,
                     DTI_PacketType_eSytemInfo, DTI_e8Bit,
                     IFX_NULL, 0) != IFX_SUCCESS)
   {
      DTI_PRN_USR_ERR_NL(DTI_CLIENT, DTI_PRN_LEVEL_ERR,
         ("Error: Client Prot SYS INFO Packet Set - setup."DTI_CRLF));

      return IFX_ERROR;
   }

   return IFX_SUCCESS;
}

/*
   Setup Protocol "start image load" Packet with the given settings.

   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dti_client_packet_protocol.h'
*/
IFX_int_t DTI_CLIENT_packetProtImageLoadStart_Set(
                        DTI_Packet_t      *pDtiPacketSend,
                        IFX_uint_t        packetBufferSize,
                        IFX_uint_t        imageNum,
                        IFX_uint_t        imageSize_byte)
{
   DTI_H2D_ImageLoadStart_t   *pPayloadSend;
   DTI_PTR_U                  uPayload;

   if (pDtiPacketSend == IFX_NULL)
   {
      DTI_PRN_USR_ERR_NL(DTI_CLIENT, DTI_PRN_LEVEL_ERR,
         ("Error: Client Prot Image Load Start Set - missing arg."DTI_CRLF));

      return IFX_ERROR;
   }

   if (packetBufferSize < (sizeof(DTI_PacketHeader_t) + sizeof(DTI_H2D_ImageLoadStart_t)) )
   {
      DTI_PRN_USR_ERR_NL(DTI_CLIENT, DTI_PRN_LEVEL_ERR,
         ("Error: Client Prot Image Load Start Set - out buffer to small."DTI_CRLF));

      return IFX_ERROR;
   }

   uPayload.pUInt8 = (IFX_uint8_t *)pDtiPacketSend->payload;
   pPayloadSend    = (DTI_H2D_ImageLoadStart_t *)DTI_PTR_CAST_GET_ULONG(uPayload);
   if (pPayloadSend == IFX_NULL)
   {
      DTI_PRN_USR_ERR_NL(DTI_CLIENT, DTI_PRN_LEVEL_ERR,
         ("Error: Client Prot Image Load Start Set - missaligned payload."DTI_CRLF));

      return IFX_ERROR;
   }

   (void)DTI_headerPacketTypeSet(
                        pDtiPacketSend,
                        (IFX_uint32_t)DTI_PacketType_eImageLoadStart,
                        (IFX_uint32_t)DTI_e32Bit,
                        sizeof(DTI_H2D_ImageLoadStart_t) );

   pPayloadSend->imageNum  = (IFX_uint32_t)imageNum;
   pPayloadSend->imageSize = (IFX_uint32_t)imageSize_byte;

   return IFX_SUCCESS;
}

/*
   Setup Protocol "start image download" Packet with the given settings.

   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dti_client_packet_protocol.h'
*/
IFX_int_t DTI_CLIENT_packetProtImageDownloadStart_Set(
                        DTI_Packet_t      *pDtiPacketSend,
                        IFX_uint_t        packetBufferSize,
                        IFX_uint_t        imageNum,
                        const IFX_char_t *fileName)
{
    IFX_uint_t                     paylSize;
    DTI_H2D_ImageDownloadStart_t   *pPayloadSend;
    DTI_PTR_U                      uPayload;
    IFX_uint_t                     fileNameSize;

    if (pDtiPacketSend == IFX_NULL)
    {
       DTI_PRN_USR_ERR_NL(DTI_CLIENT, DTI_PRN_LEVEL_ERR,
          ("Error: Client Prot Image Download Start Set - missing arg."DTI_CRLF));

       return IFX_ERROR;
    }

    if (packetBufferSize < (sizeof(DTI_PacketHeader_t) + sizeof(DTI_H2D_ImageDownloadStart_t)) )
    {
       DTI_PRN_USR_ERR_NL(DTI_CLIENT, DTI_PRN_LEVEL_ERR,
          ("Error: Client Prot Image Download Start Set - out buffer to small."DTI_CRLF));

       return IFX_ERROR;
    }

    fileNameSize = strlen(fileName);

    /* ImageDownloadStart Hdr (- header struct payl element) + sizeof fileName */
    paylSize = (sizeof(DTI_H2D_ImageDownloadStart_t) - sizeof(IFX_uint8_t)) + fileNameSize;

    uPayload.pUInt8 = (IFX_uint8_t *)pDtiPacketSend->payload;
    pPayloadSend    = (DTI_H2D_ImageDownloadStart_t *)DTI_PTR_CAST_GET_ULONG(uPayload);
    if (pPayloadSend == IFX_NULL)
    {
       DTI_PRN_USR_ERR_NL(DTI_CLIENT, DTI_PRN_LEVEL_ERR,
          ("Error: Client Prot Image Download Start Set - missaligned payload."DTI_CRLF));

       return IFX_ERROR;
    }

    (void)DTI_headerPacketTypeSet(
                        pDtiPacketSend,
                        (IFX_uint32_t)DTI_PacketType_eImageDownloadStart,
                        (IFX_uint32_t)DTI_eMixed,
                        (IFX_uint32_t)paylSize );

    pPayloadSend->imageNum  = (IFX_uint32_t)DTI_ntohl(imageNum);
    DTI_MemCpy(pPayloadSend->fileName, fileName, fileNameSize);

    return IFX_SUCCESS;
}

/*
   Setup Protocol "execute remote shell script" Packet with the given settings.

   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dti_client_packet_protocol.h'
*/
IFX_int_t DTI_CLIENT_packetProtExecuteRemoteShScript_Set( DTI_Packet_t     *pDtiPacketSend,
                                                          IFX_uint_t        packetBufferSize,
                                                          const IFX_char_t *remoteScript)
{
    IFX_uint_t                        paylSize = 0;
    DTI_H2D_ExecuteRemoteShScript_t   *pPayloadSend;
    IFX_uint_t                        scriptSize;
    DTI_PTR_U                         uPayload;

    if ( (pDtiPacketSend == IFX_NULL) )
    {
       DTI_PRN_USR_ERR_NL(DTI_CLIENT, DTI_PRN_LEVEL_ERR,
          ("Error: Client Prot Execute Remote Sh Script Set - missing arg."DTI_CRLF));

       return IFX_ERROR;
    }

    if (packetBufferSize < (sizeof(DTI_PacketHeader_t) + sizeof(DTI_H2D_ExecuteRemoteShScript_t)) )
    {
       DTI_PRN_USR_ERR_NL(DTI_CLIENT, DTI_PRN_LEVEL_ERR,
          ("Error: Client Prot Execute Remote Sh Script Set - out buffer to small."DTI_CRLF));

       return IFX_ERROR;
    }

    scriptSize = strlen(remoteScript);

    paylSize = (sizeof(DTI_H2D_ExecuteRemoteShScript_t) - sizeof(IFX_uint8_t)) + scriptSize;

    uPayload.pUInt8 = (IFX_uint8_t *)pDtiPacketSend->payload;
    pPayloadSend    = (DTI_H2D_ExecuteRemoteShScript_t *)DTI_PTR_CAST_GET_ULONG(uPayload);
    if (pPayloadSend == IFX_NULL)
    {
        DTI_PRN_USR_ERR_NL(DTI_CLIENT, DTI_PRN_LEVEL_ERR,
            ("Error:  Client Prot Execute Remote Sh Script Set - missaligned payload."DTI_CRLF));

        return IFX_ERROR;
    }

    (void)DTI_headerPacketTypeSet(
                        pDtiPacketSend,
                        (IFX_uint32_t)DTI_PacketType_eExecuteRemoteShScript,
                        (IFX_uint32_t)DTI_eMixed, paylSize);

    DTI_MemCpy(pPayloadSend->scriptToExecute, remoteScript, scriptSize);

    return IFX_SUCCESS;
}

/**
   Setup Protocol "image load chunk" Packet with the given settings.

   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dti_client_packet_protocol.h'
*/
IFX_int_t DTI_CLIENT_packetProtImageLoadChunk_Set(
                        DTI_Packet_t      *pDtiPacketSend,
                        IFX_uint_t        packetBufferSize,
                        IFX_uint_t        chunkSize_byte,
                        IFX_uint_t        offset_byte,
                        IFX_uint8_t       *pImage)
{
   IFX_uint_t                 paylSize;
   DTI_H2D_ImageLoadChunk_t   *pPayloadSend;
   DTI_PTR_U                  uPayload;

   if ( (pDtiPacketSend == IFX_NULL) || (pImage == IFX_NULL) )
   {
      DTI_PRN_USR_ERR_NL(DTI_CLIENT, DTI_PRN_LEVEL_ERR,
         ("Error: Client Prot Image Load Chunk Set - missing arg."DTI_CRLF));

      return IFX_ERROR;
   }

   /* ImageLoadChunk Hdr (- header struct payl element) + chunk size */
   paylSize =  (sizeof(DTI_H2D_ImageLoadChunk_t) - sizeof(IFX_uint8_t))  + chunkSize_byte;

   if (packetBufferSize < (sizeof(DTI_PacketHeader_t) + paylSize))
   {
      DTI_PRN_USR_ERR_NL(DTI_CLIENT, DTI_PRN_LEVEL_ERR,
         ("Error: Client Prot Image Load Chunk Set - out buffer to small."DTI_CRLF));

      return IFX_ERROR;
   }

   uPayload.pUInt8 = (IFX_uint8_t *)pDtiPacketSend->payload;
   pPayloadSend    = (DTI_H2D_ImageLoadChunk_t *)DTI_PTR_CAST_GET_ULONG(uPayload);
   if (pPayloadSend == IFX_NULL)
   {
      DTI_PRN_USR_ERR_NL(DTI_CLIENT, DTI_PRN_LEVEL_ERR,
         ("Error: Client Prot Image Load Chunk Set - missaligned payload."DTI_CRLF));

      return IFX_ERROR;
   }

   (void)DTI_headerPacketTypeSet(
                        pDtiPacketSend,
                        (IFX_uint32_t)DTI_PacketType_eImageLoadChunk,
                        (IFX_uint32_t)DTI_eMixed,
                        (IFX_uint32_t)paylSize );

   /* mixed mode - convert already to net order */
   pPayloadSend->offset    = (IFX_uint32_t)DTI_ntohl(offset_byte);
   pPayloadSend->chunkSize = (IFX_uint32_t)DTI_ntohl(chunkSize_byte);

   DTI_MemCpy(pPayloadSend->data, &pImage[offset_byte], (IFX_int_t)chunkSize_byte);

   return IFX_SUCCESS;
}

/**
   Setup Protocol "image download chunk" Packet with the given
   settings.

   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dti_client_packet_protocol.h'
*/
IFX_int_t DTI_CLIENT_packetProtImageDownloadChunk_Set(
                        DTI_Packet_t      *pDtiPacketSend,
                        IFX_uint_t        packetBufferSize,
                        IFX_uint_t        chunkSize_byte,
                        IFX_uint_t        offset_byte)
{
    IFX_uint_t                     paylSize;
    DTI_H2D_ImageDownloadChunk_t   *pPayloadSend;
    DTI_PTR_U                      uPayload;

    if ( (pDtiPacketSend == IFX_NULL) )
    {
       DTI_PRN_USR_ERR_NL(DTI_CLIENT, DTI_PRN_LEVEL_ERR,
          ("Error: Client Prot Image Download Chunk Set - missing arg."DTI_CRLF));

       return IFX_ERROR;
    }

    /* ImageLoadChunk Hdr (- header struct payl element) */
    paylSize =  sizeof(DTI_H2D_ImageDownloadChunk_t) - sizeof(IFX_uint8_t);

    if (packetBufferSize < (sizeof(DTI_PacketHeader_t) + paylSize))
    {
       DTI_PRN_USR_ERR_NL(DTI_CLIENT, DTI_PRN_LEVEL_ERR,
          ("Error: Client Prot Image Dwonload Chunk Set - out buffer to small."DTI_CRLF));

       return IFX_ERROR;
    }

    uPayload.pUInt8 = (IFX_uint8_t *)pDtiPacketSend->payload;
    pPayloadSend    = (DTI_H2D_ImageDownloadChunk_t *)DTI_PTR_CAST_GET_ULONG(uPayload);
    if (pPayloadSend == IFX_NULL)
    {
       DTI_PRN_USR_ERR_NL(DTI_CLIENT, DTI_PRN_LEVEL_ERR,
          ("Error: Client Prot Image Download Chunk Set - missaligned payload."DTI_CRLF));

       return IFX_ERROR;
    }

    (void)DTI_headerPacketTypeSet(
                        pDtiPacketSend,
                        (IFX_uint32_t)DTI_PacketType_eImageDownloadChunk,
                        (IFX_uint32_t)DTI_eMixed,
                        (IFX_uint32_t)8 );

    pPayloadSend->offset    = (IFX_uint32_t)DTI_ntohl(offset_byte);
    pPayloadSend->chunkSize = (IFX_uint32_t)DTI_ntohl(chunkSize_byte);

    return IFX_SUCCESS;
}

/*
   Setup Protocol "write to file" Packet with the given settings.

   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dti_client_packet_protocol.h'
*/
IFX_int_t DTI_CLIENT_packetProtWriteToFile_Set(
                        DTI_Packet_t      *pDtiPacketSend,
                        IFX_uint_t        packetBufferSize,
                        IFX_uint_t        imageNum,
                        IFX_uint_t        cntrlOpt,
                        const IFX_char_t  *pDestFilename)
{
   IFX_int_t   packetSize = 0;
   DTI_H2D_ImageWriteToFile_t *pPayloadSend;
   DTI_PTR_U                  uPayload;

   if ( (pDtiPacketSend == IFX_NULL) || (pDestFilename == IFX_NULL) )
   {
      DTI_PRN_USR_ERR_NL(DTI_CLIENT, DTI_PRN_LEVEL_ERR,
         ("Error: Client Prot Write To File Set - missing arg."DTI_CRLF));

      return IFX_ERROR;
   }

   packetSize = (IFX_int_t)DTI_StrLen(pDestFilename);
   if (packetSize <= 0)
   {
      DTI_PRN_USR_ERR_NL(DTI_CLIENT, DTI_PRN_LEVEL_ERR,
         ("Error: Client Prot Write To File Set - empty filename."DTI_CRLF));

      return IFX_ERROR;
   }

   /* size: (DTI Packet -1) + (strlen + termination) */
   packetSize += (IFX_int_t)sizeof(DTI_H2D_ImageWriteToFile_t);

   if (packetBufferSize < (sizeof(DTI_PacketHeader_t) + packetSize) )
   {
      DTI_PRN_USR_ERR_NL(DTI_CLIENT, DTI_PRN_LEVEL_ERR,
         ("Error: Client Prot Write To File Set - out buffer to small."DTI_CRLF));

      return IFX_ERROR;
   }

   uPayload.pUInt8 = (IFX_uint8_t *)pDtiPacketSend->payload;
   pPayloadSend    = (DTI_H2D_ImageWriteToFile_t *)DTI_PTR_CAST_GET_ULONG(uPayload);
   if (pPayloadSend == IFX_NULL)
   {
      DTI_PRN_USR_ERR_NL(DTI_CLIENT, DTI_PRN_LEVEL_ERR,
         ("Error: Client Prot Write To File Set - missaligned payload."DTI_CRLF));

      return IFX_ERROR;
   }

   (void)DTI_headerPacketTypeSet(
                        pDtiPacketSend,
                        (IFX_uint32_t)DTI_PacketType_eImageWriteToFile,
                        (IFX_uint32_t)DTI_eMixed,
                        packetSize );

   pPayloadSend->imageNum    = (IFX_uint32_t)imageNum;
   pPayloadSend->cntrlOption = (IFX_uint32_t)cntrlOpt;
   DTI_StrCpy(pPayloadSend->fullPathName, pDestFilename);

   return IFX_SUCCESS;
}


/*
   Setup Protocol "Image Release" Packet with the given settings.

   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dti_client_packet_protocol.h'
*/
IFX_int_t DTI_CLIENT_packetProtImageRelease_Set(
                        DTI_Packet_t      *pDtiPacketSend,
                        IFX_uint_t        packetBufferSize,
                        IFX_uint_t        imageNum)
{
   DTI_H2D_ImageRelease_t *pPayloadSend;
   DTI_PTR_U              uPayload;

   if (pDtiPacketSend == IFX_NULL)
   {
      DTI_PRN_USR_ERR_NL(DTI_CLIENT, DTI_PRN_LEVEL_ERR,
         ("Error: Client Prot Image Release Set - missing arg."DTI_CRLF));

      return IFX_ERROR;
   }

   if (packetBufferSize < (sizeof(DTI_PacketHeader_t) + sizeof(DTI_H2D_ImageRelease_t)) )
   {
      DTI_PRN_USR_ERR_NL(DTI_CLIENT, DTI_PRN_LEVEL_ERR,
         ("Error: Client Prot Image Release Set - out buffer to small."DTI_CRLF));

      return IFX_ERROR;
   }

   uPayload.pUInt8 = (IFX_uint8_t *)pDtiPacketSend->payload;
   pPayloadSend    = (DTI_H2D_ImageRelease_t *)DTI_PTR_CAST_GET_ULONG(uPayload);
   if (pPayloadSend == IFX_NULL)
   {
      DTI_PRN_USR_ERR_NL(DTI_CLIENT, DTI_PRN_LEVEL_ERR,
         ("Error: Client Prot Image Release Set - missaligned payload."DTI_CRLF));

      return IFX_ERROR;
   }

   (void)DTI_headerPacketTypeSet(
                        pDtiPacketSend,
                        (IFX_uint32_t)DTI_PacketType_eImageRelease,
                        (IFX_uint32_t)DTI_e32Bit,
                        sizeof(DTI_H2D_ImageRelease_t) );

   pPayloadSend->imageNum    = (IFX_uint32_t)imageNum;

   return IFX_SUCCESS;
}


/**
   Send a DTI packet via the DTI connection.

   For a detailed description of the function, its arguments and return value
   please refer to the description in the header file 'dti_client_packet_protocol.h'
*/
IFX_int_t DTI_CLIENT_packetSend(
                        const DTI_Connection_t  *pDtiConnection,
                        DTI_Packet_t            *pDtiPacketSend,
                        IFX_uint32_t            dtiPort,
                        IFX_uint32_t            dtiTan)
{
   if ( (pDtiConnection == IFX_NULL) || (pDtiPacketSend == IFX_NULL) )
   {
      DTI_PRN_USR_ERR_NL(DTI_CLIENT, DTI_PRN_LEVEL_ERR,
         ("Error: Client Packet Send - missing arg."DTI_CRLF));

      return IFX_ERROR;
   }

   /* update header */
   pDtiPacketSend->header.port = dtiPort;
   pDtiPacketSend->header.tan  = dtiTan;

   if (DTI_packetSend(pDtiConnection, pDtiPacketSend) != IFX_SUCCESS)
   {
      DTI_PRN_USR_ERR_NL(DTI_CLIENT, DTI_PRN_LEVEL_ERR,
         ("Error: Client Packet Send - send failed."DTI_CRLF));

      return IFX_ERROR;
   }

   return IFX_SUCCESS;
}

/**
   Read from  a client DTI connection

\param
   pDtiConnection - points to the DTI client connection data.
\param
   pReadFdSet     - fd set struct for recv handling.
\param
   timeOut_ms     - timeout for read operation
                      0xFFFFFFFF: wait forever
\param
   pRecvBuffer          - points to the receive buffer.
\param
   recvBufferSize_byte  - receive buffer size.

\return
   0:    no data received
   -1:   error occured (connection closed ?).
   else: number of received bytes.
*/
IFX_int_t DTI_CLIENT_packetTimedRead(
                        DTI_Connection_t        *pDtiConnection,
                        DTI_ConnectionCntrl_t   *pDtiConCntrl,
                        IFX_int_t               timeOut_ms,
                        IFX_char_t              *pRecvBuffer,
                        IFX_int_t               recvBufferSize_byte)
{
   IFX_int_t         retVal, recvBytes = 0;
   IFX_boolean_t     bResync = IFX_FALSE;
   DTI_PacketError_t packetError = DTI_eNoError;
   DTI_Packet_t      *pDtiPacket = IFX_NULL;
   
   IFX_char_t *pBuff = pRecvBuffer;
   IFX_boolean_t bPartialRespose = IFX_FALSE;
   IFX_int_t         iBytesInBuffer = 0;

   if ( (pDtiConnection == IFX_NULL) || (pDtiConCntrl == IFX_NULL) || (pRecvBuffer == IFX_NULL) )
   {
      DTI_PRN_USR_ERR_NL(DTI_CLIENT, DTI_PRN_LEVEL_ERR,
         ("Error: Client Packet TRead - missing arg."DTI_CRLF));

      return IFX_ERROR;
   }

   retVal = DTI_CON_RECV_STATUS_PENDING;
   while (retVal == DTI_CON_RECV_STATUS_PENDING || retVal == DTI_CON_RECV_STATUS_PARTIAL_DONE)
   {
      if( retVal == DTI_CON_RECV_STATUS_PARTIAL_DONE )
      {
         recvBytes = 0;
      }
      retVal = DTI_packetRead (
                        pDtiConnection, pDtiConCntrl, &bResync, timeOut_ms,
                        recvBufferSize_byte, pBuff,
                        &recvBytes, &packetError, &pDtiPacket);
      if( retVal == DTI_CON_RECV_STATUS_ERROR )
      {
         break;
      }
      
      if (retVal == DTI_CON_RECV_STATUS_TIMEOUT )
      {
         /* time limited: --> break also if receive pending */
         break;
      }
            
      if( retVal == DTI_CON_RECV_STATUS_PENDING )
      {
         continue;
      }
      
      if( pDtiPacket != IFX_NULL )
	   {
	      if( pDtiPacket->header.packetType == DTI_PacketType_eCliPartialString ) /* Partial packet */
         {
            if( bPartialRespose == IFX_FALSE )
            {
               pBuff = DTI_Malloc( sizeof( IFX_char_t) * DTI_IN_BUFFER_SIZE);
               bPartialRespose = IFX_TRUE;
               iBytesInBuffer += recvBytes;
            }
            else
            {
               if( iBytesInBuffer + recvBytes - sizeof(DTI_PacketHeader_t) >= recvBufferSize_byte )
               {
                  DTI_PRN_USR_ERR_NL(DTI_CLIENT, DTI_PRN_LEVEL_ERR,
                     ("Error: Not enough space in buffer." DTI_CRLF));
                  if( pBuff != pRecvBuffer ) /* Additional buffer was used for partial response */
                  {
                     DTI_Free( pBuff );
                  }
                  return iBytesInBuffer;
               }
               else
               {
                  DTI_MemCpy(pRecvBuffer + iBytesInBuffer, pDtiPacket->payload, pDtiPacket->header.payloadSize);
                  iBytesInBuffer += pDtiPacket->header.payloadSize;
               }
            }
            retVal = DTI_CON_RECV_STATUS_PARTIAL_DONE;
         }
         else if( pDtiPacket->header.packetType == DTI_PacketType_eCliString ) /* Full or Last packet */
         {
            if( bPartialRespose == IFX_TRUE ) /* Last packet */
            {
               if( iBytesInBuffer + recvBytes - sizeof(DTI_PacketHeader_t) >= recvBufferSize_byte )
               {
                  DTI_PRN_USR_ERR_NL(DTI_CLIENT, DTI_PRN_LEVEL_ERR,
                     ("Error: Not enough space in buffer." DTI_CRLF));
                  if( pBuff != pRecvBuffer ) /* Additional buffer was used for partial response */
                  {
                     DTI_Free( pBuff );
                  }
                  return iBytesInBuffer;
               }
               else
               {
                  DTI_MemCpy(pRecvBuffer + iBytesInBuffer, pDtiPacket->payload, pDtiPacket->header.payloadSize);
                  iBytesInBuffer += pDtiPacket->header.payloadSize;
               }
            }
            else /* Full packet */
            {
               iBytesInBuffer += recvBytes;
            }
         }
         else
         {
            iBytesInBuffer += recvBytes;
         }
	   }
   }

   if( pBuff != pRecvBuffer ) /* Additional buffer was used for partial response */
   {
      DTI_Free( pBuff );
   }
   
   if (retVal == DTI_CON_RECV_STATUS_DONE)
   {
      if( pBuff != pRecvBuffer )
      {
         pDtiPacket = (DTI_Packet_t *) pRecvBuffer;
         pDtiPacket->header.payloadSize = iBytesInBuffer;
         pDtiPacket->header.packetType = DTI_PacketType_eCliString;
      }
      
      return iBytesInBuffer;
   }

   if (recvBytes != 0)
   {
      DTI_PRN_USR_ERR_NL(DTI_CLIENT, DTI_PRN_LEVEL_ERR,
         ("Error: Client Packet TRead - receive failed, data pending."DTI_CRLF));
   }

   return IFX_ERROR;
}


