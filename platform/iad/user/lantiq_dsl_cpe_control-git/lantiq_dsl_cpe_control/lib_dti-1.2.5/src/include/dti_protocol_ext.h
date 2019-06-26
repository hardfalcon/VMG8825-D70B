#ifndef _DTI_PROTOCOL_EXT_H
#define _DTI_PROTOCOL_EXT_H
/******************************************************************************

                              Copyright (c) 2009
                            Lantiq Deutschland GmbH
                     Am Campeon 3; 85579 Neubiberg, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/** \file
   DTI Protocol library API.
*/

#ifdef __cplusplus
   extern "C" {
#endif

/* ============================================================================
   Includes
   ========================================================================= */

#include "ifx_types.h"
#include "ifxos_common.h"
#include "ifxos_debug.h"

#include "ifx_dti_protocol.h"

#include "dti_connection_interface.h"
#include "dti_protocol_interface.h"

/* ============================================================================
   Macro Defines
   ========================================================================= */


/* Packet Group Mask */
#define DTI_PACKET_GROUP_MASK                   0xFFFF0000


/* Packet Group: 0x0000xxxx - Control Level Access */
#define DTI_GROUP_CONTROL_ACCESS                0x00000000
/** Packet Group: 0x0001xxxx - Low Level HW Access */
#define DTI_GROUP_LOW_LEVEL_HW_ACCESS           0x00010000
/* Packet Group: 0x0002xxxx - Control and Message Interface */
#define DTI_GROUP_CNTRL_MSG_ACCESS              0x00020000
/* Packet Group: 0x0003xxxx - Trace Buffer Access */
#define DTI_GROUP_TRACE_BUFFER_ACCESS           0x00030000
/* Packet Group: 0x0004xxxx - Debug Register Access */
#define DTI_GROUP_DEBUG_REG_ACCESS              0x00040000
/* Packet Group: 0x0005xxxx - Command Line Interface (CLI) */
#define DTI_GROUP_CLI_ACCESS                    0x00050000
/* Packet Group: 0x0006xxxx -  */
#define DTI_GROUP_WINEASY_ACCESS                0x00060000

/* ============================================================================
   Type Defines
   ========================================================================= */

/**
   Function type to write the system info buffer.

   On a responce of the System Info request, the DTI Protocol Lib writes the
   the DTI Protocol specific informations at first.
   If a user specific function is set, this function is also called to
   fill the answer with the user context specific informations.

1. Arg:
   - points to the user given context (argument of the packet handler)
2. Arg:
   - points to the buffer to write the info
3. Arg:
   - size of the buffer

return
   The function has to return the number of written bytes.

*/
typedef IFX_int_t (*DTI_systemInfoGetFunction_t)(IFX_void_t *, IFX_char_t *, IFX_int_t);


/**
   This structure stores the info about downloaded image
*/
typedef struct
{
   /** image ID */
   IFX_int_t      imageId;
   /** max allowed image size */
   IFX_uint_t     maxImageSize;
   /** max tranfere chunk size */
   IFX_uint_t     maxChunkSize;

   /** number if users (in use and locked if != 0) */
   IFX_int_t      useCount;
   /** current already loaded bytes */
   IFX_uint_t     loadedSize;

   /** image size */
   IFX_uint_t     imageSize;
   /** image data */
   IFX_uint8_t    *pData;
} DTI_ImageControl_t;

/**
   Function type to get a free image control struct to save a image.

   On a "image load start" request the DTI Library reqires a control struct
   from the user context to save the image.

1. Arg:
   - points to the user given context (argument of the packet handler)
2. Arg:
   - image id of the requested image.
3. Arg:
   - return pointer, returns the free image control struct from the user context.
4. Arg:
   - return pointer, returns the DTI packet error.

return
   On Success, the function returns IFX_SUCCESS and sets the return pointer else
   IFX_ERROR an a NULL pointer is returned.

*/
typedef IFX_int_t (*DTI_imageCntrlGetFunction_t)(
                     IFX_void_t *, IFX_int_t, DTI_ImageControl_t **, DTI_PacketError_t *);


/**
   Function type to lock a image control struct to save a image.

   On a "image operation" request the user agent reqires the control struct
   of the requested image from the user context.

1. Arg:
   - points to the user given context (argument of the packet handler)

2. Arg:
   - image id of the requested image.
3. Arg:
   - lock / unlock the image.
4. Arg:
   - Lock: return pointer, returns the free image control struct from the user context.
   - Unlock: not used
5. Arg:
   - return pointer, returns the DTI packet error.

return
   lockOnOff = 1 (lock):
   On Success, this function returns IFX_SUCCESS, sets the return pointer and
   locks the image for use
   else IFX_ERROR an a NULL pointer is returned.

   lockOnOff = 0 (unlock):
   On Success, this function returns IFX_SUCCESS and unlocks the image from use
   else IFX_ERROR.
*/
typedef IFX_int_t (*DTI_imageCntrlLockFunction_t)(
                     IFX_void_t *, IFX_int_t, IFX_int_t, DTI_ImageControl_t **, DTI_PacketError_t *);



/**
   DTI Protocol Server Context (Agent side)
*/
typedef struct {
   /** remote connection */
   DTI_Connection_t        dtiCon;

   /** remote connection */
   DTI_ConnectionCntrl_t   dtiConCntrl;

   /** callback function: system info get */
   DTI_systemInfoGetFunction_t   pCbFctSysInfoGet;

   /** callback function: image control get */
   DTI_imageCntrlGetFunction_t   pCbFctImageCntrlGet;

   /** callback function: image control lock / unlock */
   DTI_imageCntrlLockFunction_t  pCbFctImageCntrlLock;

   /** callback function context (points to the DTI Agent struct) */
   IFX_void_t           *pCbCtxAgent;
   /** current image for file upload */
   DTI_ImageControl_t   *pCtxImageCntrl;

   /* set if the sync has been lost */
   IFX_boolean_t        bResync;

   /** IN Packet buffer */
   DTI_packetBuffer_t   packetIn;
   /** OUT Packet buffer */
   DTI_packetBuffer_t   packetOut;

} DTI_ProtocolServerCtx_t;


/* ============================================================================
   Exports
   ========================================================================= */

extern DTI_systemInfoGetFunction_t pDTI_systemInfoGetFunction;


extern IFX_int_t DTI_imageRelease(
                        DTI_ProtocolServerCtx_t    *pDtiProtServerCtx,
                        DTI_H2D_ImageRelease_t     *pDataIn,
                        DTI_PacketError_t          *pPacketError);

extern IFX_int_t DTI_imageWriteToFile(
                        DTI_ProtocolServerCtx_t    *pDtiProtServerCtx,
                        DTI_H2D_ImageWriteToFile_t *pDataIn,
                        const IFX_char_t           *pFullPathName,
                        DTI_D2H_ImageWriteToFile_t *pDataOut,
                        DTI_PacketError_t          *pPacketError);

extern IFX_int_t DTI_packetSystemInfoSet(
                        DTI_ProtocolServerCtx_t *pDtiProtServerCtx,
                        const DTI_Packet_t      *pDtiPacketIn,
                        DTI_Packet_t            *pDtiPacketOut,
                        IFX_uint32_t            bufferOutSize);

extern IFX_int_t DTI_packetImageLoadStartSet(
                        DTI_ProtocolServerCtx_t *pDtiProtServerCtx,
                        const DTI_Packet_t      *pDtiPacketIn,
                        DTI_Packet_t            *pDtiPacketOut,
                        IFX_uint32_t            bufferOutSize);

extern IFX_int_t DTI_packetImageDownloadStartSet(
                        DTI_ProtocolServerCtx_t *pDtiProtServerCtx,
                        const DTI_Packet_t      *pDtiPacketIn,
                        DTI_Packet_t            *pDtiPacketOut,
                        IFX_uint32_t            bufferOutSize);

extern IFX_int_t DTI_packetImageLoadChunkSet(
                        DTI_ProtocolServerCtx_t *pDtiProtServerCtx,
                        const DTI_Packet_t      *pDtiPacketIn,
                        DTI_Packet_t            *pDtiPacketOut,
                        IFX_uint32_t            bufferOutSize);

extern IFX_int_t DTI_packetImageDownloadChunkSet(
                        DTI_ProtocolServerCtx_t *pDtiProtServerCtx,
                        const DTI_Packet_t      *pDtiPacketIn,
                        DTI_Packet_t            *pDtiPacketOut,
                        IFX_uint32_t            bufferOutSize);

extern IFX_int_t DTI_packetImageReleaseSet(
                        DTI_ProtocolServerCtx_t *pDtiProtServerCtx,
                        const DTI_Packet_t      *pDtiPacketIn,
                        DTI_Packet_t            *pDtiPacketOut,
                        IFX_uint32_t            bufferOutSize);

extern IFX_int_t DTI_packetImageWriteToFileSet(
                        DTI_ProtocolServerCtx_t *pDtiProtServerCtx,
                        const DTI_Packet_t      *pDtiPacketIn,
                        DTI_Packet_t            *pDtiPacketOut,
                        IFX_uint32_t            bufferOutSize);

extern IFX_int_t DTI_packetHandler_Standard(
                        DTI_ProtocolServerCtx_t *pDtiProtServerCtx,
                        const DTI_Packet_t      *pDtiPacketIn,
                        DTI_Packet_t            *pDtiPacketOut,
                        IFX_uint_t              dtiBufOutLen);

#ifdef __cplusplus
}
#endif

#endif /* _DTI_PROTOCOL_EXT_H */
