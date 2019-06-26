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
#include "dti_protocol_ext.h"

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

#ifndef DTI_FileLoad
#  define DTI_FileLoad                             IFXOS_FileLoad
#endif

#define MAX_PATH_LENGTH 255

/* ============================================================================
   Local Function Declaration
   ========================================================================= */

DTI_STATIC IFX_int_t DTI_systemInfoWrite_protocol(
                        IFX_char_t  *pSysInfoBuffer,
                        IFX_int_t   bufferSize);

DTI_STATIC IFX_int_t DTI_imageLoadStart(
                        DTI_ProtocolServerCtx_t    *pDtiProtServerCtx,
                        DTI_H2D_ImageLoadStart_t   *pDataIn,
                        DTI_D2H_ImageLoadStart_t   *pDataOut,
                        DTI_PacketError_t          *pPacketError);

DTI_STATIC IFX_int_t DTI_imageDownloadStart(
                        DTI_ProtocolServerCtx_t        *pDtiProtServerCtx,
                        DTI_H2D_ImageDownloadStart_t   *pDataIn,
                        DTI_D2H_ImageDownloadStart_t   *pDataOut,
                        DTI_PacketError_t              *pPacketError);

DTI_STATIC IFX_int_t DTI_imageLoadChunk(
                        DTI_ProtocolServerCtx_t    *pDtiProtServerCtx,
                        DTI_H2D_ImageLoadChunk_t   *pDataIn,
                        DTI_PacketError_t          *pPacketError);

DTI_STATIC IFX_int_t DTI_imageDownloadChunk(
                        DTI_ProtocolServerCtx_t        *pDtiProtServerCtx,
                        DTI_H2D_ImageDownloadChunk_t   *pDataIn,
                        DTI_D2H_ImageDownloadChunk_t   *pDataOut,
                        IFX_uint8_t                    *outChunk,
                        DTI_PacketError_t              *pPacketError);

#ifdef LINUX
DTI_STATIC IFX_int_t DTI_executeShellScript(
                        DTI_ProtocolServerCtx_t             *pDtiProtServerCtx,
                        DTI_H2D_ExecuteRemoteShScript_t     *pDataIn,
                        DTI_PacketError_t                   *pPacketError);
#endif
/* ============================================================================
   Variables
   ========================================================================= */

DTI_systemInfoGetFunction_t pDTI_systemInfoGetFunction = IFX_NULL;

/* ============================================================================
   Local Function defintions
   ========================================================================= */
/**
   Write the Protocol System Info to a given buffer.
*/
DTI_STATIC IFX_int_t DTI_systemInfoWrite_protocol(
                        IFX_char_t  *pSysInfoBuffer,
                        IFX_int_t   bufferSize)
{
   IFX_int_t writtenLen = 0;

   (void)DTI_snprintf(&pSysInfoBuffer[writtenLen], bufferSize - writtenLen,
      "ProtocolRevision=%s", DTI_PROTOCOL_VER_STR);

   writtenLen = (IFX_int_t)DTI_StrLen(&pSysInfoBuffer[writtenLen]) + 1;

   return writtenLen;
}

/**
   Image Download Start.

\param
   pDtiProtServerCtx - points to the DTI protocol server context.
\param
   pDataIn           - points to the "ImageLoadStart" payload IN data.
\param
   pDataOut          - points to the "ImageLoadStart" payload OUT data.
\param
   pPacketError      - return pointer, return DTI packet error.

\return
   IFX_SUCCESS if setup was successful, else
   ERROR in case of missing args.
*/

DTI_STATIC IFX_int_t DTI_imageDownloadStart(
                        DTI_ProtocolServerCtx_t        *pDtiProtServerCtx,
                        DTI_H2D_ImageDownloadStart_t   *pDataIn,
                        DTI_D2H_ImageDownloadStart_t   *pDataOut,
                        DTI_PacketError_t              *pPacketError)
{
#if defined(IFXOS_HAVE_FILE_ACCESS) && (IFXOS_HAVE_FILE_ACCESS == 1)
    DTI_PacketError_t     unlockPacketError = DTI_eNoError;
    DTI_ImageControl_t   *pImageCntrl = IFX_NULL;
    IFX_uint8_t          *pImage;
    IFX_size_t            imageSize_byte;
#endif /* defined(IFXOS_HAVE_FILE_ACCESS) && (IFXOS_HAVE_FILE_ACCESS == 1) */

    if ( (pDtiProtServerCtx->pCbCtxAgent          == IFX_NULL) ||
        (pDtiProtServerCtx->pCbFctImageCntrlGet  == IFX_NULL) ||
        (pDtiProtServerCtx->pCbFctImageCntrlLock == IFX_NULL) )
    {
       DTI_PRN_USR_ERR_NL(DTI_PROTOCOL, DTI_PRN_LEVEL_ERR,
          ("Error: Image Download Start - missing callback."DTI_CRLF));

       *pPacketError = DTI_eErrConfiguration;
       return IFX_ERROR;
    }

    if (pDtiProtServerCtx->pCtxImageCntrl != IFX_NULL)
    {
       DTI_PRN_USR_ERR_NL(DTI_PROTOCOL, DTI_PRN_LEVEL_ERR,
          ("Error: Image Download Start - operation pending."DTI_CRLF));

       *pPacketError = DTI_eErrPortOperation;
       return IFX_ERROR;
    }

#if !defined(IFXOS_HAVE_FILE_ACCESS) || (IFXOS_HAVE_FILE_ACCESS != 1)
    DTI_PRN_USR_ERR_NL(DTI_PROTOCOL, DTI_PRN_LEVEL_ERR,
       ("ERROR: Server Operation - file download, load from file system not supported."DTI_CRLF));

    *pPacketError = DTI_eErrConfiguration;
    return IFX_ERROR;
#else

    if (DTI_FileLoad (pDataIn->fileName, &pImage, &imageSize_byte) != 0)
    {
        DTI_PRN_USR_ERR_NL(DTI_PROTOCOL, DTI_PRN_LEVEL_ERR,
            ("ERROR: Server Operation - file download, open <%s>."DTI_CRLF,
            pDataIn->fileName));

        *pPacketError = DTI_eErrInvalidParameters;
        return IFX_ERROR;
    }


    /* get the image control */
    if (pDtiProtServerCtx->pCbFctImageCntrlGet(
                              pDtiProtServerCtx->pCbCtxAgent,               /* dti agent context */
                              (IFX_int_t)pDataIn->imageNum,
                              &pImageCntrl,
                              pPacketError) != IFX_SUCCESS)
    {
       DTI_PRN_USR_ERR_NL(DTI_PROTOCOL, DTI_PRN_LEVEL_ERR,
          ("Error: Image Download Start - operation failed."DTI_CRLF));

       *pPacketError = DTI_eErrPortOperation;
       return IFX_ERROR;
    }
    *pPacketError = DTI_eNoError;

    if (pImageCntrl->maxImageSize < (IFX_uint_t)imageSize_byte )
    {
       DTI_PRN_USR_ERR_NL(DTI_PROTOCOL, DTI_PRN_LEVEL_ERR,
         ("Error: Image Download Start - invalid size."DTI_CRLF));

       *pPacketError = DTI_eErrInvalidParameters;
       DTI_Free(pImage);

       (void)pDtiProtServerCtx->pCbFctImageCntrlLock(
                              pDtiProtServerCtx->pCbCtxAgent,
                              pImageCntrl->imageId, 0,
                              IFX_NULL, &unlockPacketError);

       return IFX_ERROR;
   }

   if (pImageCntrl->pData != IFX_NULL)
   {
      DTI_Free(pImageCntrl->pData);
      pImageCntrl->pData      = IFX_NULL;
      pImageCntrl->imageSize  = 0;
      pImageCntrl->loadedSize = 0;
   }

   if (imageSize_byte > 0)
   {
       pImageCntrl->imageSize  = imageSize_byte;
       pImageCntrl->pData      = pImage;
       pDataOut->imageSize     = imageSize_byte;
       pDataOut->maxChunkSize  = (IFX_uint32_t)pImageCntrl->maxChunkSize;
       pDtiProtServerCtx->pCtxImageCntrl = pImageCntrl;
   }
   else
   {
      /* 0 Size request to free the space and release */
      DTI_PRN_USR_DBG_NL(DTI_PROTOCOL, DTI_PRN_LEVEL_HIGH,
         ("Image Download Start - release image ID %d."DTI_CRLF, pImageCntrl->imageId));

      (void)pDtiProtServerCtx->pCbFctImageCntrlLock(
                              pDtiProtServerCtx->pCbCtxAgent,
                              pImageCntrl->imageId, 0,
                              IFX_NULL, &unlockPacketError);

      pDataOut->maxChunkSize = 0;
      pDataOut->imageSize = 0;
   }

   return (*pPacketError == DTI_eNoError) ? IFX_SUCCESS : IFX_ERROR;
#endif /* !defined(IFXOS_HAVE_FILE_ACCESS) || (IFXOS_HAVE_FILE_ACCESS != 1) */
}

/**
   Image Load Start.

\param
   pDtiProtServerCtx - points to the DTI protocol server context.
\param
   pDataIn           - points to the "ImageLoadStart" payload IN data.
\param
   pDataOut          - points to the "ImageLoadStart" payload OUT data.
\param
   pPacketError      - return pointer, return DTI packet error.

\return
   IFX_SUCCESS if setup was successful, else
   ERROR in case of missing args.
*/
DTI_STATIC IFX_int_t DTI_imageLoadStart(
                        DTI_ProtocolServerCtx_t    *pDtiProtServerCtx,
                        DTI_H2D_ImageLoadStart_t   *pDataIn,
                        DTI_D2H_ImageLoadStart_t   *pDataOut,
                        DTI_PacketError_t          *pPacketError)
{
   DTI_PacketError_t    unlockPacketError = DTI_eNoError;
   DTI_ImageControl_t   *pImageCntrl = IFX_NULL;

   if ( (pDtiProtServerCtx->pCbCtxAgent          == IFX_NULL) ||
        (pDtiProtServerCtx->pCbFctImageCntrlGet  == IFX_NULL) ||
        (pDtiProtServerCtx->pCbFctImageCntrlLock == IFX_NULL) )
   {
      DTI_PRN_USR_ERR_NL(DTI_PROTOCOL, DTI_PRN_LEVEL_ERR,
         ("Error: Image Load Start - missing callback."DTI_CRLF));

      *pPacketError = DTI_eErrConfiguration;
      return IFX_ERROR;
   }

   if (pDtiProtServerCtx->pCtxImageCntrl != IFX_NULL)
   {
      DTI_PRN_USR_ERR_NL(DTI_PROTOCOL, DTI_PRN_LEVEL_ERR,
         ("Error: Image Load Start - operation pending."DTI_CRLF));

      *pPacketError = DTI_eErrPortOperation;
      return IFX_ERROR;
   }

   /* get the image control */
   if (pDtiProtServerCtx->pCbFctImageCntrlGet(
                              pDtiProtServerCtx->pCbCtxAgent,               /* dti agent context */
                              (IFX_int_t)pDataIn->imageNum,
                              &pImageCntrl,
                              pPacketError) != IFX_SUCCESS)
   {
      DTI_PRN_USR_ERR_NL(DTI_PROTOCOL, DTI_PRN_LEVEL_ERR,
         ("Error: Image Load Start - operation failed."DTI_CRLF));

      *pPacketError = DTI_eErrPortOperation;
      return IFX_ERROR;
   }

   *pPacketError = DTI_eNoError;

   if (pImageCntrl->maxImageSize < (IFX_uint_t)pDataIn->imageSize )
   {
      DTI_PRN_USR_ERR_NL(DTI_PROTOCOL, DTI_PRN_LEVEL_ERR,
         ("Error: Image Load Start - invalid size."DTI_CRLF));

      *pPacketError = DTI_eErrInvalidParameters;
      goto DTI_IMAGE_LOAD_START_EXIT;
   }

   if (pImageCntrl->pData != IFX_NULL)
   {
      DTI_Free(pImageCntrl->pData);
      pImageCntrl->pData      = IFX_NULL;
      pImageCntrl->imageSize  = 0;
      pImageCntrl->loadedSize = 0;
   }

   if (pDataIn->imageSize > 0)
   {
      pImageCntrl->loadedSize = 0;
      pImageCntrl->imageSize  = 0;
      pImageCntrl->pData      = (IFX_uint8_t *)DTI_Malloc(pDataIn->imageSize);

      if (pImageCntrl->pData == IFX_NULL)
      {
         DTI_PRN_USR_ERR_NL(DTI_PROTOCOL, DTI_PRN_LEVEL_ERR,
            ("Error: Image Load Start - memory."DTI_CRLF));

         *pPacketError = DTI_eErrPortOperation;
         goto DTI_IMAGE_LOAD_START_EXIT;
      }
      else
      {
         pImageCntrl->imageSize            = (IFX_uint_t)pDataIn->imageSize;
         pDtiProtServerCtx->pCtxImageCntrl = pImageCntrl;
         pDataOut->maxChunkSize            = (IFX_uint32_t)pImageCntrl->maxChunkSize;

         DTI_PRN_USR_DBG_NL(DTI_PROTOCOL, DTI_PRN_LEVEL_HIGH,
            ("Image Load Start - image: ID %d, size %d, max chunk %d."DTI_CRLF,
            pImageCntrl->imageId, pImageCntrl->imageSize, pImageCntrl->maxChunkSize));

      }
   }
   else
   {
      /* 0 Size request to free the space and release */

      DTI_PRN_USR_DBG_NL(DTI_PROTOCOL, DTI_PRN_LEVEL_HIGH,
         ("Image Load Start - release image ID %d."DTI_CRLF, pImageCntrl->imageId));

      (void)pDtiProtServerCtx->pCbFctImageCntrlLock(
                              pDtiProtServerCtx->pCbCtxAgent,
                              pImageCntrl->imageId, 0,
                              IFX_NULL, &unlockPacketError);

      pDataOut->maxChunkSize = 0;
   }

DTI_IMAGE_LOAD_START_EXIT:

   if (*pPacketError != DTI_eNoError)
   {
      DTI_PRN_USR_ERR_NL(DTI_PROTOCOL, DTI_PRN_LEVEL_ERR,
         ("ERROR: Image Load Start - unlock and free."DTI_CRLF));

      if (pImageCntrl->pData != IFX_NULL)
      {
         DTI_Free(pImageCntrl->pData);
      }
      pImageCntrl->pData      = IFX_NULL;
      pImageCntrl->imageSize  = 0;
      pImageCntrl->loadedSize = 0;

      (void)pDtiProtServerCtx->pCbFctImageCntrlLock(
                              pDtiProtServerCtx->pCbCtxAgent,
                              pImageCntrl->imageId, 0, IFX_NULL, &unlockPacketError);
   }

   return (*pPacketError == DTI_eNoError) ? IFX_SUCCESS : IFX_ERROR;
}

/**
   Image Load Chunk.

\param
   pDtiProtServerCtx - points to the DTI protocol server context.
\param
   pDataIn           - points to the "ImageLoadChunk" payload IN data.
\param
   pPacketError      - return pointer, return DTI packet error.

\return
   IFX_SUCCESS if setup was successful, else
   ERROR in case of missing args.
*/
DTI_STATIC IFX_int_t DTI_imageLoadChunk(
                        DTI_ProtocolServerCtx_t    *pDtiProtServerCtx,
                        DTI_H2D_ImageLoadChunk_t   *pDataIn,
                        DTI_PacketError_t          *pPacketError)
{
   DTI_PacketError_t    unlockPacketError = DTI_eNoError;
   DTI_ImageControl_t   *pImageCntrl = IFX_NULL;

   if ( (pDtiProtServerCtx->pCbCtxAgent          == IFX_NULL ) ||
        (pDtiProtServerCtx->pCbFctImageCntrlLock == IFX_NULL) )
   {
      DTI_PRN_USR_ERR_NL(DTI_PROTOCOL, DTI_PRN_LEVEL_ERR,
         ("Error: Image Load Chunk - missing callback."DTI_CRLF));

      *pPacketError = DTI_eErrConfiguration;
      return IFX_ERROR;
   }

   if ((pImageCntrl = pDtiProtServerCtx->pCtxImageCntrl) == IFX_NULL)
   {
      DTI_PRN_USR_ERR_NL(DTI_PROTOCOL, DTI_PRN_LEVEL_ERR,
         ("Error: Image Load Chunk - missing start."DTI_CRLF));

      *pPacketError = DTI_eErrPortOperation;
      return IFX_ERROR;
   }

   DTI_PRN_USR_DBG_NL(DTI_PROTOCOL, DTI_PRN_LEVEL_NORMAL,
      ("Image Load Chunk - image ID %d (loaded %d / size %d), offset %d, chunk size %d."DTI_CRLF,
       pImageCntrl->imageId, pImageCntrl->loadedSize, pImageCntrl->imageSize,
       pDataIn->offset, pDataIn->chunkSize));


   if ( (pDataIn->chunkSize > pImageCntrl->maxChunkSize) ||
        ((pImageCntrl->loadedSize + pDataIn->chunkSize) > pImageCntrl->imageSize) )
   {
      DTI_PRN_USR_ERR_NL(DTI_PROTOCOL, DTI_PRN_LEVEL_ERR,
         ("Error: Image Load Chunk - size overflow, Chunk %d (max %d) + %d > Size %d."DTI_CRLF,
          pDataIn->chunkSize, pImageCntrl->maxChunkSize, pImageCntrl->loadedSize,
          pImageCntrl->imageSize));

      *pPacketError = DTI_eErrInvalidPayloadSize;
      goto DTI_IMAGE_LOAD_CHUNK_EXIT;
   }

   if ( (pDataIn->offset + pDataIn->chunkSize) > pImageCntrl->imageSize)
   {
      DTI_PRN_USR_ERR_NL(DTI_PROTOCOL, DTI_PRN_LEVEL_ERR,
         ("Error: Image Load Chunk - offset overflow, loaded: %d + %d > Size %d."DTI_CRLF,
          pImageCntrl->loadedSize, pDataIn->offset, pImageCntrl->imageSize));

      *pPacketError = DTI_eErrMalformedPacket;
      goto DTI_IMAGE_LOAD_CHUNK_EXIT;
   }

   DTI_MemCpy( (IFX_void_t *)&pImageCntrl->pData[pDataIn->offset],
               (IFX_void_t *)pDataIn->data,
               (IFX_int_t)pDataIn->chunkSize);

   pImageCntrl->loadedSize += pDataIn->chunkSize;
   *pPacketError = DTI_eNoError;

DTI_IMAGE_LOAD_CHUNK_EXIT:

   if (*pPacketError != DTI_eNoError)
   {
      DTI_PRN_USR_ERR_NL(DTI_PROTOCOL, DTI_PRN_LEVEL_ERR,
         ("ERROR: Image Load Chunk - unlock and free Image %d."DTI_CRLF,
          pImageCntrl->imageId));

      if (pImageCntrl->pData != IFX_NULL)
      {
         DTI_Free(pImageCntrl->pData);
      }
      pImageCntrl->pData      = IFX_NULL;
      pImageCntrl->imageSize  = 0;
      pImageCntrl->loadedSize = 0;

      (void)pDtiProtServerCtx->pCbFctImageCntrlLock(
                           pDtiProtServerCtx->pCbCtxAgent,
                           pImageCntrl->imageId, 0, IFX_NULL, &unlockPacketError);

      pDtiProtServerCtx->pCtxImageCntrl = IFX_NULL;
   }
   else
   {
      if (pImageCntrl->loadedSize == pImageCntrl->imageSize)
      {
         DTI_PRN_USR_DBG_NL(DTI_PROTOCOL, DTI_PRN_LEVEL_HIGH,
            ("Image Load Chunk - upload complete, ImageId %d, Size %d."DTI_CRLF,
             pImageCntrl->imageId, pImageCntrl->imageSize));

         pDtiProtServerCtx->pCtxImageCntrl = IFX_NULL;
      }

      (void)pDtiProtServerCtx->pCbFctImageCntrlLock(
                              pDtiProtServerCtx->pCbCtxAgent,
                              pImageCntrl->imageId, 0, IFX_NULL, &unlockPacketError);

      if (unlockPacketError != DTI_eNoError)
      {
         *pPacketError = unlockPacketError;
      }
   }

   return (*pPacketError == DTI_eNoError) ? IFX_SUCCESS : IFX_ERROR;
}

#ifdef LINUX
DTI_STATIC IFX_int_t DTI_executeShellScript(
                        DTI_ProtocolServerCtx_t              *pDtiProtServerCtx,
                        DTI_H2D_ExecuteRemoteShScript_t      *pDataIn,
                        DTI_PacketError_t                    *pPacketError)
{
    int systemReturn;
    DTI_PRN_USR_DBG_NL(DTI_PROTOCOL, DTI_PRN_LEVEL_NORMAL,
     ("Execute Shell Script - script to execute: %s ."DTI_CRLF,
      pDataIn->scriptToExecute));

    systemReturn = system(pDataIn->scriptToExecute);

    DTI_MemSet(pDataIn->scriptToExecute, 0, strlen(pDataIn->scriptToExecute));

    if (systemReturn == -1 )
    {
        DTI_PRN_USR_ERR_NL(DTI_PROTOCOL, DTI_PRN_LEVEL_ERR,
         ("ERROR: Execute Shell Script - system fork failed ."DTI_CRLF));
        *pPacketError = DTI_eErrInvalidParameters;
        return IFX_ERROR;
    }

    if (WEXITSTATUS(systemReturn) != 0)
    {
        DTI_PRN_USR_ERR_NL(DTI_PROTOCOL, DTI_PRN_LEVEL_ERR,
         ("ERROR: Execute Shell Script - script execution failed ."DTI_CRLF));
        *pPacketError = DTI_eErrInvalidParameters;
        return IFX_ERROR;
    }

    DTI_PRN_USR_DBG_NL(DTI_PROTOCOL, DTI_PRN_LEVEL_NORMAL,
       ("Execute Shell Script - script was successfully executed."DTI_CRLF));

    return  IFX_SUCCESS;
}
#endif

DTI_STATIC IFX_int_t DTI_imageDownloadChunk(
                        DTI_ProtocolServerCtx_t        *pDtiProtServerCtx,
                        DTI_H2D_ImageDownloadChunk_t   *pDataIn,
                        DTI_D2H_ImageDownloadChunk_t   *pDataOut,
                        IFX_uint8_t                    *outChunk,
                        DTI_PacketError_t              *pPacketError)
{
   DTI_PacketError_t    unlockPacketError = DTI_eNoError;
   DTI_ImageControl_t   *pImageCntrl = IFX_NULL;

   if ( (pDtiProtServerCtx->pCbCtxAgent          == IFX_NULL ) ||
        (pDtiProtServerCtx->pCbFctImageCntrlLock == IFX_NULL) )
   {
      DTI_PRN_USR_ERR_NL(DTI_PROTOCOL, DTI_PRN_LEVEL_ERR,
         ("Error: Image Download Chunk - missing callback."DTI_CRLF));

      *pPacketError = DTI_eErrConfiguration;
      return IFX_ERROR;
   }

   if ((pImageCntrl = pDtiProtServerCtx->pCtxImageCntrl) == IFX_NULL)
   {
      DTI_PRN_USR_ERR_NL(DTI_PROTOCOL, DTI_PRN_LEVEL_ERR,
         ("Error: Image Download Chunk - missing start."DTI_CRLF));


      *pPacketError = DTI_eErrPortOperation;
      return IFX_ERROR;
   }

   if ( (pDataIn->chunkSize > pImageCntrl->maxChunkSize) ||
        ((pImageCntrl->loadedSize + pDataIn->chunkSize) > pImageCntrl->imageSize) )
   {
      DTI_PRN_USR_ERR_NL(DTI_PROTOCOL, DTI_PRN_LEVEL_ERR,
         ("Error: Image Download Chunk - size overflow, Chunk %d (max %d) + %d > Size %d."DTI_CRLF,
          pDataIn->chunkSize, pImageCntrl->maxChunkSize, pImageCntrl->loadedSize,
          pImageCntrl->imageSize));

      *pPacketError = DTI_eErrInvalidPayloadSize;
      goto DTI_IMAGE_DOWNLOAD_CHUNK_EXIT;
   }

   if ( (pDataIn->offset + pDataIn->chunkSize) > pImageCntrl->imageSize)
   {
      DTI_PRN_USR_ERR_NL(DTI_PROTOCOL, DTI_PRN_LEVEL_ERR,
         ("Error: Image Download Chunk - offset overflow, loaded: %d + %d > Size %d."DTI_CRLF,
          pImageCntrl->loadedSize, pDataIn->offset, pImageCntrl->imageSize));


      *pPacketError = DTI_eErrMalformedPacket;
      goto DTI_IMAGE_DOWNLOAD_CHUNK_EXIT;
   }

   pDataOut->chunkSize = pDataIn->chunkSize;
   pDataOut->offset = pDataIn->offset;

   DTI_MemCpy( (IFX_void_t *)pDataOut->data,
               (IFX_void_t *)&pImageCntrl->pData[pDataIn->offset],
               (IFX_int_t)pDataOut->chunkSize);

   pImageCntrl->loadedSize += pDataIn->chunkSize;
   *pPacketError = DTI_eNoError;

DTI_IMAGE_DOWNLOAD_CHUNK_EXIT:

   if (*pPacketError != DTI_eNoError)
   {
      DTI_PRN_USR_ERR_NL(DTI_PROTOCOL, DTI_PRN_LEVEL_ERR,
         ("ERROR: Image Download Chunk - unlock and free Image %d."DTI_CRLF,
          pImageCntrl->imageId));

      if (pImageCntrl->pData != IFX_NULL)
      {
         DTI_Free(pImageCntrl->pData);
      }
      pImageCntrl->pData      = IFX_NULL;
      pImageCntrl->imageSize  = 0;
      pImageCntrl->loadedSize = 0;

      (void)pDtiProtServerCtx->pCbFctImageCntrlLock(
                           pDtiProtServerCtx->pCbCtxAgent,
                           pImageCntrl->imageId, 0, IFX_NULL, &unlockPacketError);

      pDtiProtServerCtx->pCtxImageCntrl = IFX_NULL;
   }
   else
   {
      if (pImageCntrl->loadedSize == pImageCntrl->imageSize)
      {
         DTI_PRN_USR_DBG_NL(DTI_PROTOCOL, DTI_PRN_LEVEL_HIGH,
            ("Image Download Chunk - download complete, ImageId %d, Size %d."DTI_CRLF,
             pImageCntrl->imageId, pImageCntrl->imageSize));

         pDtiProtServerCtx->pCtxImageCntrl = IFX_NULL;
      }

      (void)pDtiProtServerCtx->pCbFctImageCntrlLock(
                              pDtiProtServerCtx->pCbCtxAgent,
                              pImageCntrl->imageId, 0, IFX_NULL, &unlockPacketError);

      if (unlockPacketError != DTI_eNoError)
      {
         *pPacketError = unlockPacketError;
      }
   }

   return (*pPacketError == DTI_eNoError) ? IFX_SUCCESS : IFX_ERROR;
}

/**
   Image Release.

\param
   pDtiProtServerCtx - points to the DTI protocol server context.
\param
   pDataIn           - points to the "ImageRelease" payload IN data.
\param
   pPacketError      - return pointer, return DTI packet error.

\return
   IFX_SUCCESS if setup was successful, else
   ERROR in case of missing args.
*/
IFX_int_t DTI_imageRelease(
                        DTI_ProtocolServerCtx_t    *pDtiProtServerCtx,
                        DTI_H2D_ImageRelease_t     *pDataIn,
                        DTI_PacketError_t          *pPacketError)
{
   DTI_ImageControl_t   *pImageCntrl = IFX_NULL;

   if (pDtiProtServerCtx == IFX_NULL)
   {
      DTI_PRN_USR_ERR_NL(DTI_PROTOCOL, DTI_PRN_LEVEL_ERR,
         ("Error: Image Release - context."DTI_CRLF));

      *pPacketError = DTI_eErrConfiguration;
      return IFX_ERROR;
   }

   if ( (pDtiProtServerCtx->pCbCtxAgent          == IFX_NULL) ||
        (pDtiProtServerCtx->pCbFctImageCntrlGet  == IFX_NULL) ||
        (pDtiProtServerCtx->pCbFctImageCntrlLock == IFX_NULL) )
   {
      DTI_PRN_USR_ERR_NL(DTI_PROTOCOL, DTI_PRN_LEVEL_ERR,
         ("Error: Image Release - missing callback."DTI_CRLF));

      *pPacketError = DTI_eErrConfiguration;
      return IFX_ERROR;
   }
   *pPacketError = DTI_eNoError;

   /* get the image control for config change */
   if (pDtiProtServerCtx->pCbFctImageCntrlGet(
                              pDtiProtServerCtx->pCbCtxAgent,               /* dti agent context */
                              (IFX_int_t)pDataIn->imageNum,
                              &pImageCntrl,
                              pPacketError) != IFX_SUCCESS)
   {
      DTI_PRN_USR_ERR_NL(DTI_PROTOCOL, DTI_PRN_LEVEL_ERR,
         ("Error: Image Release - get image control %d."DTI_CRLF,
           pDataIn->imageNum));

      *pPacketError = DTI_eErrPortOperation;
      return IFX_ERROR;
   }

   DTI_PRN_USR_DBG_NL(DTI_PROTOCOL, DTI_PRN_LEVEL_HIGH,
      ("Image Release - release image ID %d."DTI_CRLF, pImageCntrl->imageId));

   if (pImageCntrl->pData != IFX_NULL)
   {
      DTI_Free(pImageCntrl->pData);
   }
   pImageCntrl->pData      = IFX_NULL;
   pImageCntrl->imageSize  = 0;
   pImageCntrl->loadedSize = 0;

   (void)pDtiProtServerCtx->pCbFctImageCntrlLock(
                           pDtiProtServerCtx->pCbCtxAgent,
                           pImageCntrl->imageId, 0,
                           IFX_NULL, pPacketError);

   return (*pPacketError == DTI_eNoError) ? IFX_SUCCESS : IFX_ERROR;
}

/**
   Image Load Start.

\param
   pDtiProtServerCtx - points to the DTI protocol server context.
\param
   pDataIn           - points to the "ImageWriteToFile" payload IN data.
\param
   pDataOut          - points to the "ImageWriteToFile" payload OUT data.
\param
   pPacketError      - return pointer, return DTI packet error.

\return
   IFX_SUCCESS if setup was successful, else
   ERROR in case of missing args.
*/
IFX_int_t DTI_imageWriteToFile(
                        DTI_ProtocolServerCtx_t    *pDtiProtServerCtx,
                        DTI_H2D_ImageWriteToFile_t *pDataIn,
                        const IFX_char_t           *pFullPathName,
                        DTI_D2H_ImageWriteToFile_t *pDataOut,
                        DTI_PacketError_t          *pPacketError)
{
#if (defined(IFXOS_HAVE_FILE_ACCESS) && (IFXOS_HAVE_FILE_ACCESS == 1))
   const IFX_char_t     *pUsedFullPathName = IFX_NULL;
   DTI_ImageControl_t   *pImageCntrl = IFX_NULL;
   DTI_File_t           *pFileFd = IFX_NULL;

   if ( (pDtiProtServerCtx->pCbCtxAgent          == IFX_NULL) ||
        (pDtiProtServerCtx->pCbFctImageCntrlGet  == IFX_NULL) ||
        (pDtiProtServerCtx->pCbFctImageCntrlLock == IFX_NULL) )
   {
      DTI_PRN_USR_ERR_NL(DTI_PROTOCOL, DTI_PRN_LEVEL_ERR,
         ("Error: Image Write To File - missing callback."DTI_CRLF));

      pDataOut->dtiCode  = -1;
      pDataOut->osCode   = 0;
      *pPacketError = DTI_eErrConfiguration;

      return IFX_ERROR;
   }

   if (pFullPathName)
   {
      pUsedFullPathName = pFullPathName;
   }
   else
   {
      pUsedFullPathName = (const IFX_char_t *)pDataIn->fullPathName;
   }

   if ( DTI_StrLen(pUsedFullPathName) <= 0)
   {
      DTI_PRN_USR_ERR_NL(DTI_PROTOCOL, DTI_PRN_LEVEL_ERR,
         ("Error: Image Write To File - missing full path name."DTI_CRLF));

      pDataOut->dtiCode  = 0;
      pDataOut->osCode   = 0;
      *pPacketError = DTI_eErrInvalidParameters;

      return IFX_ERROR;
   }

   *pPacketError = DTI_eNoError;
   if (pDataIn->cntrlOption & DTI_WRITE_TO_FILE_CNTRL_RELEASE)
   {
      /* get the image control for config change */
      if (pDtiProtServerCtx->pCbFctImageCntrlGet(
                                 pDtiProtServerCtx->pCbCtxAgent,               /* dti agent context */
                                 (IFX_int_t)pDataIn->imageNum,
                                 &pImageCntrl,
                                 pPacketError) != IFX_SUCCESS)
      {
         DTI_PRN_USR_ERR_NL(DTI_PROTOCOL, DTI_PRN_LEVEL_ERR,
            ("Error: Image Write To File - get image control %d."DTI_CRLF,
              pDataIn->imageNum));

         pDataOut->dtiCode  = -1;
         pDataOut->osCode   = 0;
         *pPacketError = DTI_eErrPortOperation;
         return IFX_ERROR;
      }
   }
   else
   {
      /* lock the image */
      if (pDtiProtServerCtx->pCbFctImageCntrlLock(
                                 pDtiProtServerCtx->pCbCtxAgent,               /* dti agent context */
                                 (IFX_int_t)pDataIn->imageNum,
                                 1, &pImageCntrl, pPacketError) != IFX_SUCCESS)
      {
         DTI_PRN_USR_ERR_NL(DTI_PROTOCOL, DTI_PRN_LEVEL_ERR,
            ("Error: Image Write To File - lock image %d."DTI_CRLF,
              pDataIn->imageNum));

         pDataOut->dtiCode  = -1;
         pDataOut->osCode   = 0;
         *pPacketError = DTI_eErrPortOperation;
         return IFX_ERROR;
      }
   }

   if ((pImageCntrl->pData == IFX_NULL) || (pImageCntrl->imageSize == 0))
   {
      DTI_PRN_USR_ERR_NL(DTI_PROTOCOL, DTI_PRN_LEVEL_ERR,
         ("ERROR: Image Write To File - image %d empty."DTI_CRLF,
           pDataIn->imageNum));

      pDataOut->dtiCode  = -1;
      pDataOut->osCode   = 0;
      *pPacketError = DTI_eErrPortOperation;

      goto DTI_IMAGE_WRITE_TO_FILE_EXIT;
   }

   pFileFd = DTI_FOpen(pUsedFullPathName, IFXOS_OPEN_MODE_WRITE_BIN);
   if (pFileFd == IFX_NULL)
   {
      DTI_PRN_USR_ERR_NL(DTI_PROTOCOL, DTI_PRN_LEVEL_ERR,
         ("ERROR: Image Write To File - open <%s> failed."DTI_CRLF,
           pUsedFullPathName));

      pDataOut->dtiCode  = 0;
      pDataOut->osCode   = errno;

      *pPacketError = DTI_eErrPortOperation;
      goto DTI_IMAGE_WRITE_TO_FILE_EXIT;
   }

   if ( DTI_FWrite(pImageCntrl->pData, 1, pImageCntrl->imageSize, pFileFd) !=
            (IFX_size_t)pImageCntrl->imageSize )
   {
      DTI_PRN_USR_ERR_NL(DTI_PROTOCOL, DTI_PRN_LEVEL_ERR,
         ("ERROR: Image Write To File - write <%s> size %d failed."DTI_CRLF,
           pUsedFullPathName, pImageCntrl->imageSize));

      pDataOut->dtiCode  = 0;
      pDataOut->osCode   = errno;

      *pPacketError = DTI_eErrPortOperation;
      goto DTI_IMAGE_WRITE_TO_FILE_EXIT;
   }

   if (pDataIn->cntrlOption & DTI_WRITE_TO_FILE_CNTRL_RELEASE)
   {
      if (pImageCntrl->pData != IFX_NULL)
      {
         DTI_Free(pImageCntrl->pData);
      }
      pImageCntrl->pData      = IFX_NULL;
      pImageCntrl->imageSize  = 0;
      pImageCntrl->loadedSize = 0;
   }

DTI_IMAGE_WRITE_TO_FILE_EXIT:

   (void)pDtiProtServerCtx->pCbFctImageCntrlLock(
                           pDtiProtServerCtx->pCbCtxAgent,
                           pImageCntrl->imageId, 0,
                           IFX_NULL, pPacketError);

   if (pFileFd != IFX_NULL)
   {
      DTI_FClose(pFileFd);
   }

   return (*pPacketError == DTI_eNoError) ? IFX_SUCCESS : IFX_ERROR;

#else
   pDataOut->imageNum = pDataIn->imageNum;
   pDataOut->dtiCode  = -1;
   pDataOut->osCode   = -1;
   *pPacketError = DTI_eErrConfiguration;
   return IFX_ERROR;
#endif   /* #if (defined(IFXOS_HAVE_FILE_ACCESS) && (IFXOS_HAVE_FILE_ACCESS == 1)) */
}

/* ============================================================================
   Global Function defintions
   ========================================================================= */

/**
   This function setup an "system info packet"

\param
   pDtiPacketIn   - points to the IN packet.
\param
   pDtiPacketOut  - points to the OUT packet.
\param
   bufferOutSize  - size of the out buffer.

\return
   IFX_SUCCESS if setup was successful, else
   ERROR in case of missing args.
*/
IFX_int_t DTI_packetSystemInfoSet(
                        DTI_ProtocolServerCtx_t *pDtiProtServerCtx,
                        const DTI_Packet_t      *pDtiPacketIn,
                        DTI_Packet_t            *pDtiPacketOut,
                        IFX_uint32_t            bufferOutSize)
{
   DTI_PacketError_t packetErr = DTI_eNoError;
   IFX_int_t         currWr, writtenBytes, remainBytes;
   IFX_char_t        *pCurrPos;

   if (!pDtiPacketOut || !pDtiPacketIn)
   {
      DTI_PRN_USR_ERR_NL(DTI_PROTOCOL, IFXOS_PRN_LEVEL_ERR,
         ("Error: Packet System Info set - NULL ptr args."IFXOS_CRLF));

      return IFX_ERROR;
   }

   /* setup payload */
   writtenBytes = 0;
   remainBytes  = (IFX_int_t)bufferOutSize - sizeof(DTI_PacketHeader_t);
   pCurrPos     = (IFX_char_t *)pDtiPacketOut->payload;

   currWr = DTI_systemInfoWrite_protocol(pCurrPos, remainBytes);
   if (currWr < 0)
   {
      packetErr = DTI_eErrUnknown;
      writtenBytes = 0;
      goto DTI_PACKET_SYSTEM_INFO_SET_DONE;
   }

   remainBytes  -= currWr;
   writtenBytes += currWr;
   pCurrPos     += currWr;

   if (pDTI_systemInfoGetFunction)
   {
      currWr = pDTI_systemInfoGetFunction(
                     pDtiProtServerCtx->pCbCtxAgent, pCurrPos, remainBytes);
      if (currWr < 0)
      {
         packetErr = DTI_eErrUnknown;
         writtenBytes = 0;

         goto DTI_PACKET_SYSTEM_INFO_SET_DONE;
      }

      remainBytes  -= currWr;
      writtenBytes += currWr;
      pCurrPos     += currWr;
   }

DTI_PACKET_SYSTEM_INFO_SET_DONE:

   (void)DTI_headerResponceSet(
            &pDtiPacketIn->header, &pDtiPacketOut->header,
            (IFX_uint32_t)writtenBytes, packetErr);

   return IFX_SUCCESS;
}


/**
   This function setup an "start image load"

\param
   pDtiProtServerCtx - points to the DTI protocol server context.
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
IFX_int_t DTI_packetImageLoadStartSet(
                        DTI_ProtocolServerCtx_t *pDtiProtServerCtx,
                        const DTI_Packet_t      *pDtiPacketIn,
                        DTI_Packet_t            *pDtiPacketOut,
                        IFX_uint32_t            bufferOutSize)
{
   DTI_PacketError_t          packetError = DTI_eNoError;
   DTI_H2D_ImageLoadStart_t   *pDataIn;
   DTI_D2H_ImageLoadStart_t   *pDataOut;
   DTI_PTR_U                  uPayload;

   uPayload.pUInt8 = (IFX_uint8_t *)pDtiPacketIn->payload;
   pDataIn = (DTI_H2D_ImageLoadStart_t *)DTI_PTR_CAST_GET_ULONG(uPayload);

   uPayload.pUInt8 = pDtiPacketOut->payload;
   pDataOut = (DTI_D2H_ImageLoadStart_t *)DTI_PTR_CAST_GET_ULONG(uPayload);

   if ((pDataIn == IFX_NULL) || (pDataOut == IFX_NULL))
   {
      DTI_PRN_USR_ERR_NL(DTI_PROTOCOL, DTI_PRN_LEVEL_ERR,
         ("Error: Packet Image Load Start - missaligned payload."DTI_CRLF));

      packetError = DTI_eErrMalformedPacket;
   }
   else
   {
      (void)DTI_imageLoadStart(pDtiProtServerCtx, pDataIn, pDataOut, &packetError);
   }

   (void)DTI_packetResponceSet(
         pDtiPacketIn, pDtiPacketOut,
         packetError, sizeof(DTI_D2H_ImageLoadStart_t),
         bufferOutSize, IFX_FALSE);

   return IFX_SUCCESS;
}

IFX_int_t DTI_packetImageDownloadStartSet(
                       DTI_ProtocolServerCtx_t *pDtiProtServerCtx,
                       const DTI_Packet_t      *pDtiPacketIn,
                       DTI_Packet_t            *pDtiPacketOut,
                       IFX_uint32_t            bufferOutSize)
{
   DTI_PacketError_t              packetError = DTI_eNoError;
   DTI_H2D_ImageDownloadStart_t   *pDataIn;
   DTI_D2H_ImageDownloadStart_t   *pDataOut;
   DTI_PTR_U                      uPayload;

   uPayload.pUInt8 = (IFX_uint8_t *)pDtiPacketIn->payload;
   pDataIn = (DTI_H2D_ImageDownloadStart_t *)DTI_PTR_CAST_GET_ULONG(uPayload);

   uPayload.pUInt8 = pDtiPacketOut->payload;
   pDataOut = (DTI_D2H_ImageDownloadStart_t *)DTI_PTR_CAST_GET_ULONG(uPayload);

   if ((pDataIn == IFX_NULL) || (pDataOut == IFX_NULL))
   {
      DTI_PRN_USR_ERR_NL(DTI_PROTOCOL, DTI_PRN_LEVEL_ERR,
         ("Error: Packet Image Download Start - missaligned payload."DTI_CRLF));

      packetError = DTI_eErrMalformedPacket;
   }
   else
   {
      pDataIn->imageNum = (IFX_uint32_t)DTI_ntohl(pDataIn->imageNum);
      (void)DTI_imageDownloadStart(pDtiProtServerCtx, pDataIn, pDataOut, &packetError);
   }

   (void)DTI_packetResponceSet(
         pDtiPacketIn, pDtiPacketOut,
         packetError, sizeof(DTI_D2H_ImageDownloadStart_t),
         bufferOutSize, IFX_FALSE);

   return IFX_SUCCESS;
}


IFX_int_t DTI_packetExecuteShellScript(
                        DTI_ProtocolServerCtx_t *pDtiProtServerCtx,
                        const DTI_Packet_t      *pPacketIn,
                        DTI_Packet_t            *pPacketOut,
                        IFX_uint32_t            bufferOutSize)
{
    DTI_PacketError_t               packetError = DTI_eNoError;
    DTI_H2D_ExecuteRemoteShScript_t *pDataIn;
    DTI_PTR_U                       uPayload;

    uPayload.pUInt8 = (IFX_uint8_t *)pPacketIn->payload;
    pDataIn = (DTI_H2D_ExecuteRemoteShScript_t *)DTI_PTR_CAST_GET_ULONG(uPayload);

    if (pDataIn == IFX_NULL)
    {
       DTI_PRN_USR_ERR_NL(DTI_PROTOCOL, DTI_PRN_LEVEL_ERR,
       ("Error: Packet Execute remote shell script - missaligned payload."DTI_CRLF));

       packetError = DTI_eErrMalformedPacket;
    }
    else
    {
#ifdef LINUX
        (void)DTI_executeShellScript(pDtiProtServerCtx, pDataIn, &packetError);
#else
        DTI_PRN_USR_ERR_NL(DTI_PROTOCOL, DTI_PRN_LEVEL_ERR,
        ("Error: Packet Execute remote shell script - is not supported by OS."DTI_CRLF));
                packetError = DTI_eErrConfiguration;
#endif
    }

    (void)DTI_packetResponceSet(
         pPacketIn, pPacketOut,
         packetError, 0,
         bufferOutSize, IFX_FALSE);

    return IFX_SUCCESS;
}


/**
   This function setup an "chunk image load"

\param
   pDtiProtServerCtx - points to the DTI protocol server context.
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
IFX_int_t DTI_packetImageLoadChunkSet(
                        DTI_ProtocolServerCtx_t *pDtiProtServerCtx,
                        const DTI_Packet_t      *pDtiPacketIn,
                        DTI_Packet_t            *pDtiPacketOut,
                        IFX_uint32_t            bufferOutSize)
{
   DTI_PacketError_t          packetError = DTI_eNoError;
   DTI_H2D_ImageLoadChunk_t   *pDataIn;
   DTI_PTR_U                  uPayload;

   uPayload.pUInt8 = (IFX_uint8_t *)pDtiPacketIn->payload;
   pDataIn = (DTI_H2D_ImageLoadChunk_t *)DTI_PTR_CAST_GET_ULONG(uPayload);
   if (pDataIn == IFX_NULL)
   {
      DTI_PRN_USR_ERR_NL(DTI_PROTOCOL, DTI_PRN_LEVEL_ERR,
         ("Error: Packet Image Load Chunk - missaligned payload."DTI_CRLF));

      packetError = DTI_eErrMalformedPacket;
   }
   else
   {
      /* packet is "mixed" - reorder */
      pDataIn->chunkSize = (IFX_uint32_t)DTI_ntohl(pDataIn->chunkSize);
      pDataIn->offset    = (IFX_uint32_t)DTI_ntohl(pDataIn->offset);

      (void)DTI_imageLoadChunk(pDtiProtServerCtx, pDataIn, &packetError);
   }

   (void)DTI_packetResponceSet(
         pDtiPacketIn, pDtiPacketOut,
         packetError, 0, bufferOutSize, IFX_FALSE);

   return IFX_SUCCESS;
}

/**
   This function setup an "chunk image download"

\param
   pDtiProtServerCtx - points to the DTI protocol server context.
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
IFX_int_t DTI_packetImageDownloadChunkSet(
                        DTI_ProtocolServerCtx_t *pDtiProtServerCtx,
                        const DTI_Packet_t      *pDtiPacketIn,
                        DTI_Packet_t            *pDtiPacketOut,
                        IFX_uint32_t            bufferOutSize)
{
   DTI_PacketError_t              packetError = DTI_eNoError;
   DTI_H2D_ImageDownloadChunk_t   *pDataIn;
   DTI_D2H_ImageDownloadChunk_t   *pDataOut;
   IFX_uint8_t                    *outChunk = IFX_NULL;
   DTI_PTR_U                      uPayload;

   uPayload.pUInt8 = (IFX_uint8_t *)pDtiPacketIn->payload;
   pDataIn = (DTI_H2D_ImageDownloadChunk_t *)DTI_PTR_CAST_GET_ULONG(uPayload);

   uPayload.pUInt8 = (IFX_uint8_t *)pDtiPacketOut->payload;
   pDataOut = (DTI_D2H_ImageDownloadChunk_t *)DTI_PTR_CAST_GET_ULONG(uPayload);

   if (pDataIn == IFX_NULL)
   {
      DTI_PRN_USR_ERR_NL(DTI_PROTOCOL, DTI_PRN_LEVEL_ERR,
         ("Error: Packet Image Download Chunk - missaligned payload."DTI_CRLF));

      packetError = DTI_eErrMalformedPacket;
   }
   else if (pDataOut == IFX_NULL)
   {
      DTI_PRN_USR_ERR_NL(DTI_PROTOCOL, DTI_PRN_LEVEL_ERR,
         ("Error: Packet Image Download Chunk - missaligned payload."DTI_CRLF));

      packetError = DTI_eErrMalformedPacket;
   }
   else
   {
      /* packet is "mixed" - reorder */
      pDataIn->chunkSize = DTI_ntohl(pDataIn->chunkSize);
      pDataIn->offset    = DTI_ntohl(pDataIn->offset);

      (void)DTI_imageDownloadChunk(pDtiProtServerCtx, pDataIn, pDataOut, outChunk,  &packetError);
   }

   (void)DTI_packetResponceSet(
         pDtiPacketIn, pDtiPacketOut,
         packetError, sizeof(DTI_D2H_ImageDownloadChunk_t) +  pDataIn->chunkSize, bufferOutSize, IFX_FALSE);


   return IFX_SUCCESS;
}

/**
   This function release an image from the agent

\param
   pDtiProtServerCtx - points to the DTI protocol server context.
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
IFX_int_t DTI_packetImageReleaseSet(
                        DTI_ProtocolServerCtx_t *pDtiProtServerCtx,
                        const DTI_Packet_t      *pDtiPacketIn,
                        DTI_Packet_t            *pDtiPacketOut,
                        IFX_uint32_t            bufferOutSize)
{
   DTI_PacketError_t       packetError = DTI_eNoError;
   DTI_H2D_ImageRelease_t  *pDataIn;
   DTI_PTR_U               uPayload;

   uPayload.pUInt8 = (IFX_uint8_t *)pDtiPacketIn->payload;
   pDataIn = (DTI_H2D_ImageRelease_t *)DTI_PTR_CAST_GET_ULONG(uPayload);
   if (pDataIn == IFX_NULL)
   {
      DTI_PRN_USR_ERR_NL(DTI_PROTOCOL, DTI_PRN_LEVEL_ERR,
         ("Error: Packet Image Release - missaligned payload."DTI_CRLF));

      packetError = DTI_eErrMalformedPacket;
   }
   else
   {
      (void)DTI_imageRelease(pDtiProtServerCtx, pDataIn, &packetError);
   }

   (void)DTI_packetResponceSet(
         pDtiPacketIn, pDtiPacketOut,
         packetError, 0, bufferOutSize, IFX_FALSE);

   return IFX_SUCCESS;
}

/**
   This function setup an "write to file" operation.

\param
   pDtiProtServerCtx - points to the DTI protocol server context.
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
IFX_int_t DTI_packetImageWriteToFileSet(
                        DTI_ProtocolServerCtx_t *pDtiProtServerCtx,
                        const DTI_Packet_t      *pDtiPacketIn,
                        DTI_Packet_t            *pDtiPacketOut,
                        IFX_uint32_t            bufferOutSize)
{
   DTI_PacketError_t          packetError = DTI_eNoError;
   DTI_H2D_ImageWriteToFile_t *pDataIn;
   DTI_D2H_ImageWriteToFile_t *pDataOut;
   DTI_PTR_U                  uPayload;

   uPayload.pUInt8 = (IFX_uint8_t *)pDtiPacketIn->payload;
   pDataIn = (DTI_H2D_ImageWriteToFile_t *)DTI_PTR_CAST_GET_ULONG(uPayload);

   uPayload.pUInt8 = pDtiPacketOut->payload;
   pDataOut = (DTI_D2H_ImageWriteToFile_t *)DTI_PTR_CAST_GET_ULONG(uPayload);

   if ((pDataIn == IFX_NULL) || (pDataOut == IFX_NULL))
   {
      DTI_PRN_USR_ERR_NL(DTI_PROTOCOL, DTI_PRN_LEVEL_ERR,
         ("Error: Packet Image Write To File - missaligned payload."DTI_CRLF));

      packetError = DTI_eErrMalformedPacket;
   }
   else
   {
      /* packet is "mixed" - reorder */
      pDataIn->imageNum    = (IFX_uint32_t)DTI_ntohl(pDataIn->imageNum);
      pDataIn->cntrlOption = (IFX_uint32_t)DTI_ntohl(pDataIn->cntrlOption);

      (void)DTI_imageWriteToFile(pDtiProtServerCtx, pDataIn, IFX_NULL, pDataOut, &packetError);
   }

   (void)DTI_packetResponceSet(
         pDtiPacketIn, pDtiPacketOut,
         packetError, sizeof(DTI_D2H_ImageWriteToFile_t),
         bufferOutSize, IFX_FALSE);

   /* IN / OUT packet have different payload types */
   pDtiPacketOut->header.packetOptions = (IFX_uint32_t)DTI_e32Bit;

   return IFX_SUCCESS;
}


/**
   DTI Standard Call Packet Handler.
\param
   pDtiProtServerCtx - points to the DTI protocol server context.
*/
IFX_int_t DTI_packetHandler_Standard(
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

   if (pDtiProtServerCtx == IFX_NULL)
   {
      DTI_PRN_USR_ERR_NL(DTI_PROTOCOL, IFXOS_PRN_LEVEL_ERR,
         ("Error: Packet Prot Handler - NULL ptr args."IFXOS_CRLF));

      return IFX_ERROR;
   }

   if (pDtiPacketIn != IFX_NULL)
   {
      pPacketIn  = pDtiPacketIn;
   }
   else
   {
      uPayload.pUInt8 = (IFX_uint8_t *)pDtiProtServerCtx->packetIn.buffer;
      pPacketIn = (DTI_Packet_t *)DTI_PTR_CAST_GET_ULONG(uPayload);
   }

   if (pDtiPacketOut != IFX_NULL)
   {
      pPacketOut = pDtiPacketOut;
      bufOutLen  = dtiBufOutLen;
   }
   else
   {
      uPayload.pUInt8 = (IFX_uint8_t *)pDtiProtServerCtx->packetOut.buffer;
      pPacketOut = (DTI_Packet_t *)DTI_PTR_CAST_GET_ULONG(uPayload);
      bufOutLen  = sizeof(pDtiProtServerCtx->packetOut.buffer);
   }

   if ( (pPacketIn == IFX_NULL) || (pPacketOut == IFX_NULL) )
   {
      DTI_PRN_USR_ERR_NL(DTI_PROTOCOL, IFXOS_PRN_LEVEL_ERR,
         ("Error: Packet Prot Handler - invalid payload assignment (%s)."IFXOS_CRLF,
          (pPacketIn == IFX_NULL) ? "IN" : "OUT"));

      return IFX_ERROR;
   }

   if (pPacketIn->header.magic != DTI_MAGIC)
   {
      DTI_PRN_USR_ERR_NL(DTI_PROTOCOL, IFXOS_PRN_LEVEL_ERR,
         ("Error: Packet Prot Handler - invalid IN packet."IFXOS_CRLF));

      return IFX_ERROR;
   }

   switch(pPacketIn->header.packetType)
   {
      case DTI_PacketType_eLoopback:
         {
            retVal = DTI_packetLoopBackSet(pPacketIn, pPacketOut, bufOutLen);
         }
         break;
      case DTI_PacketType_eSytemInfo:
         {
            retVal = DTI_packetSystemInfoSet(
                              pDtiProtServerCtx, pPacketIn, pPacketOut, bufOutLen);
         }
         break;
      case DTI_PacketType_eImageLoadStart:
         {
            retVal = DTI_packetImageLoadStartSet(
                              pDtiProtServerCtx, pPacketIn, pPacketOut, bufOutLen);
         }
         break;
      case DTI_PacketType_eImageDownloadStart:
         {
            retVal = DTI_packetImageDownloadStartSet(
                              pDtiProtServerCtx, pPacketIn, pPacketOut, bufOutLen);
         }
         break;
      case DTI_PacketType_eExecuteRemoteShScript:
         {
                    retVal = DTI_packetExecuteShellScript(
                              pDtiProtServerCtx, pPacketIn, pPacketOut, bufOutLen);
                 }
         break;

      case DTI_PacketType_eImageDownloadChunk:
         {
            retVal = DTI_packetImageDownloadChunkSet(
                              pDtiProtServerCtx, pPacketIn, pPacketOut, bufOutLen);
         }
         break;

      case DTI_PacketType_eImageLoadChunk:
         {
            retVal = DTI_packetImageLoadChunkSet(
                              pDtiProtServerCtx, pPacketIn, pPacketOut, bufOutLen);
         }
         break;

      case DTI_PacketType_eImageRelease:
         {
            retVal = DTI_packetImageReleaseSet(
                              pDtiProtServerCtx, pPacketIn, pPacketOut, bufOutLen);
         }
         break;

      case DTI_PacketType_eImageWriteToFile:
         {
            retVal = DTI_packetImageWriteToFileSet(
                              pDtiProtServerCtx, pPacketIn, pPacketOut, bufOutLen);
         }
         break;

      default:
         {
            retVal = DTI_packetUnknownSet(pPacketIn, pPacketOut, bufOutLen);
         }
         break;
   }

   if (retVal == IFX_SUCCESS)
   {
      DTI_packetShow (
         pPacketOut, IFX_TRUE, IFX_FALSE, "send",
         (pPacketOut->header.error == DTI_eNoError) ? IFXOS_PRN_LEVEL_LOW : IFXOS_PRN_LEVEL_HIGH);

      if (DTI_packetSend(&pDtiProtServerCtx->dtiCon, pPacketOut) != IFX_SUCCESS)
      {
         DTI_PRN_USR_ERR_NL(DTI_PROTOCOL, IFXOS_PRN_LEVEL_ERR,
            ("Error: Packet Prot Handler - send packet."IFXOS_CRLF));

         return IFX_ERROR;
      }
   }

   return IFX_SUCCESS;
}


