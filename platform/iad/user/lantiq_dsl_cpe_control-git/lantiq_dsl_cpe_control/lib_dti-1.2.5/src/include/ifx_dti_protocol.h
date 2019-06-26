#ifndef _IFX_DTI_PROTOCOL_H
#define _IFX_DTI_PROTOCOL_H
/******************************************************************************

                              Copyright (c) 2009
                            Lantiq Deutschland GmbH
                     Am Campeon 3; 85579 Neubiberg, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/** \file
   Debug and Trace Interface - Protocol definitions and declarations.
*/


/** \defgroup DTI_PROTOCOL Debug and Trace Interface Protocol
   The Debug and Trace Interface (DTI) is a communication interface to
   access and communicate with target systems (embedded) for debug and
   test purposals.
*/

/** \defgroup DTI_PROTOCOL_COMMON DTI Protocol (Common Part)

   This Group contains basic definitions of the common part of the Debug and
   Trace Interface (DTI) Protocol.

   Wihtin this header the device independant definitions are defined, this are:
   - DTI Protocol version.
   - Basic DTI packet definitions, like: header, basic packets (system info, etc).
   - Basic defines for the DTI packet type, options, payload types.
   - DTI Error Codes.

\ingroup DTI_PROTOCOL

*/


#ifdef __cplusplus
   extern "C" {
#endif

/* ============================================================================
   Includes
   ========================================================================= */
#include "ifx_types.h"

/* ============================================================================
   Interface defines
   ========================================================================= */

/** \addtogroup DTI_PROTOCOL_COMMON
@{ */


#ifndef _MKSTR_1
#define _MKSTR_1(x)    #x
#define _MKSTR(x)      _MKSTR_1(x)
#endif

/** DTI version, major number */
#define DTI_PROTOCOL_VER_MAJOR     0
/** DTI version, minor number */
#define DTI_PROTOCOL_VER_MINOR     0
/** DTI version, build number */
#define DTI_PROTOCOL_VER_STEP      7
/** DTI version string */
#define DTI_PROTOCOL_VER_STR       _MKSTR(DTI_PROTOCOL_VER_MAJOR)"."_MKSTR(DTI_PROTOCOL_VER_MINOR)"."_MKSTR(DTI_PROTOCOL_VER_STEP)
/** DTI what string */
#define DTI_PROTOCOL_WHAT_STR      "@(#)Debug and Trace Interface, Protocol Version " DTI_PROTOCOL_VER_STR

/** DTI Protocol Version - hex format */
#define DTI_PROTOCOL_VER \
		(  (DTI_PROTOCOL_VER_MAJOR << 24) \
		 | (DTI_PROTOCOL_VER_MINOR << 16) \
		 | (DTI_PROTOCOL_VER_STEP  << 8) )

/**
   Version Check - equal with the given version

   Usage:
\code
   #if (!DTI_VERSION_CHECK_EQ(0,0,2))
   #   error "DTI_VERSION_CHECK_EQ: requiere at least DTI Protocol version 0.0.2"
   #endif
\endcode
*/
#define DTI_VERSION_CHECK_EQ(major, minor, step) \
               ((DTI_PROTOCOL_VER_MAJOR == major) && (DTI_PROTOCOL_VER_MINOR == minor) && (DTI_PROTOCOL_VER_STEP == step) )

/* ============================================================================
   Packet Type: 0x0000xxxx - Control Level Access
   ========================================================================= */

/** DTI packet - magic key */
#define DTI_MAGIC                               0xdeadbeef

/** DTI Header - Port Field: used bits for the port number */
#define DTI_HDR_PORT_PORT_MASK                  0x00FFFFFF
/** DTI Header - Port Field: used bits for the device number */
#define DTI_HDR_PORT_DEV_TYPE_MASK              0xFF000000

/** DTI Header - Option Field: used bits for the payload type */
#define DTI_HDR_OPTION_MASK_PAYL_TYPE           0x000000FF

#define DTI_HDR_PORT_PORT_NUM_GET(hdr_port_field) \
                  ( (IFX_uint32_t)(hdr_port_field) & DTI_HDR_PORT_PORT_MASK )

#define DTI_HDR_PORT_PORT_NUM_SET(hdr_port_field, port_num) \
                  ( ((IFX_uint32_t)(hdr_port_field) & ~DTI_HDR_PORT_PORT_MASK) | \
                    ((IFX_uint32_t)(port_num) & DTI_HDR_PORT_PORT_MASK) )

#define DTI_HDR_PORT_DEV_TYPE_NUM_GET(hdr_port_field) \
                  (((IFX_uint32_t)(hdr_port_field) & DTI_HDR_PORT_DEV_TYPE_MASK) >> 24)

#define DTI_HDR_PORT_DEV_TYPE_NUM_SET(hdr_port_field, port_num) \
                  ( ((IFX_uint32_t)(hdr_port_field) & ~DTI_HDR_PORT_DEV_TYPE_MASK) | \
                    (((IFX_uint32_t)(port_num) << 24) & DTI_HDR_PORT_PORT_MASK) )

/**
   Packet Type: 0x0000xxxx - Control Level Access
*/
typedef enum
{
   /** Error Packet Type */
   DTI_PacketType_eError                = 0x00000001,
   /** Loopback Packet */
   DTI_PacketType_eLoopback             = 0x00000002,
   /** System Information Retrieval */
   DTI_PacketType_eSytemInfo            = 0x00000003,
   /** Prepare Binary Load */
   DTI_PacketType_eImageLoadStart       = 0x00000010,
   /** Send Image Chunk */
   DTI_PacketType_eImageLoadChunk       = 0x00000011,
   /** Release a stored image */
   DTI_PacketType_eImageRelease         = 0x00000012,
   /** Write a image to the local file system */
   DTI_PacketType_eImageWriteToFile     = 0x00000013,
   /** Prepare Binary Download */
   DTI_PacketType_eImageDownloadStart   = 0x00000014,
   /** Download Image Chunk */
   DTI_PacketType_eImageDownloadChunk   = 0x00000015,
   /** Execute Remote Shell Script */
   DTI_PacketType_eExecuteRemoteShScript= 0x00000016

} DTI_PacketType_t;

/**
    Possible payload data types
*/
typedef enum
{
   /** payload data is of mixed data*/
   DTI_eMixed = 0
   /** payload data is 8 bit type */
   ,DTI_e8Bit  = 1
   /** payload data is 16 bit type */
   ,DTI_e16Bit = 2
   /** payload data is 32 bit type */
   ,DTI_e32Bit = 3

} DTI_PayloadType_t;

/**
    Possible packet error codes
*/
typedef enum
{
   /* no error */
   DTI_eNoError = 0
   /** packet type is unknown or not supported*/
   ,DTI_eErrUnknown = 1
   /** some non DTI API related network error*/
   ,DTI_eErrNetwork = 2
   /** response was not seen within max pending time*/
   ,DTI_eErrTimeout = 3
   /** unknown mailbox message*/
   ,DTI_eErrUnknownMsg = 4
   /** Packet is in incorrect format */
   ,DTI_eErrMalformedPacket
   /** Packet is in incorrect format */
   ,DTI_eErrInvalidParameters
   /** Packet type is not known or supported */
   ,DTI_eErrInvalidPacketType
   /** Payload size is not supported */
   ,DTI_eErrInvalidPayloadSize

   /** invalid / inclomplete configuration */
   ,DTI_eErrConfiguration
   /** port / device / interface num out of range */
   ,DTI_eErrPortOutOfRange
   /** port / interface open error */
   ,DTI_eErrPortOpen
   /** port / device / interface operation error */
   ,DTI_eErrPortOperation
   /** Autonomous port / device / interface operation error */
   ,DTI_eErrPortAutoOperation
   /** unknown / not configured device interface */
   ,DTI_eErrDeviceTypeOutOfRange

} DTI_PacketError_t;

/**
   The DTI paket header structure.
*/
typedef struct {
   /** A constant value of 0xdeadbeef used for resynchronization and debugging */
   IFX_uint32_t magic;
   /** The protocol packet type. The upper 16 bits are used to select a functional
       group (e.g. host interface, debug interface, host controller). The lower 16
       bits are the command itself.(Definitions see DTI_PacketType_t)*/
   IFX_uint32_t packetType;
   /** The packet options. The upper 24 bits are currently reserved and should be set to zero.
       The lower 8 bits identify the payload type. The payload type is the bit size of the payload items.
       0 = Mixed, 1 = 8 bit,  2 = 16bit, 3 = 32 bit. (Definitions see DTI_PayloadType_t)*/
   IFX_uint32_t packetOptions;
   /** The port / channel number. If a packet is channel independent this value will be ignored. */
    IFX_uint32_t port;
   /** The transaction number.
       This must be an auto-incrementing non zero number. The packet(s) to acknowledge the initiating
       message will use the same number. The value 0 is reserved for autonomous / asynchronous messages.
       On the device side the Tan is simply copied into the acknowledging packet. No check for proper
       incrementing is performed. Even a Tan of zero will just be mirrored, but no error condition is generated.*/
   IFX_uint32_t tan;
   /** This field contains the error code. In case the packet type/direction does not have
       an error code the field is reserved and has to be set to zero. An error code of zero means success. */
   IFX_uint32_t error;
   /** The payload size in number of bytes. */
   IFX_uint32_t payloadSize;
} DTI_PacketHeader_t;


/** The DTI paket definition */
typedef struct {
   /** paket header */
   DTI_PacketHeader_t header;
   /** payload */
   IFX_uint8_t  payload[1];
} DTI_Packet_t;


/** Loopback Packet.

   Packet type: \ref DTI_PacketType_eLoopback .

   This packet can be used for debugging and testing of the connection.
   It just mirrors the complete packet data received from the PC side.

*/
typedef struct {
   IFX_uint8_t data[1];
} DTI_H2D_Loopback_t;

/** Loopback Packet.

   Packet type: \ref  DTI_PacketType_eLoopback .

   This packet can be used for debugging and testing of the connection.
   It just mirrors the complete packet data received from the PC side.
*/
typedef DTI_H2D_Loopback_t DTI_D2H_Loopback_t;

/** System Information Retrieval.

   Packet type: \ref  DTI_PacketType_eSytemInfo .

   This packet provides general information about the board. This information is organized
   as zero terminated strings with a tag and a value. The format of such a string has to be
   Tag=Value. Numbers are coded with ASCII digits.

   \remarks
   Predefined Tags for system information:
   - ProtocolRevision   A version number which identifies the protocol implementation. This might be used to support future expansion.
   - VendorName         Short string with an abbreviation of the vendor name.
   - BoardName          Short string with the board/product name.
   - BoardRevision
   - MaxChannel         Number of channels / ports accessible by this interface.
   - PacketQueueSize    Number of pending requests the target can buffer

*/
typedef struct {
   IFX_char_t  option[1];
} DTI_D2H_SysInfo_t;

/**
   Image load - transfere an binary form the host (PC) to the target system.

   The host informs the target about the file size and identifier.

*/
typedef struct {
   /** Identify the image for later use */
   IFX_uint32_t imageNum;
   /** Size of the image to download in number of bytes */
   IFX_uint32_t imageSize;
} DTI_H2D_ImageLoadStart_t;

/**
   Image download - transfere an binary the target system(board) to the host(PC)  .

   The host informs the target about the file name to download and identifier.
*/
typedef struct {
    /** Identify the image for later use */
    IFX_uint32_t imageNum;
    /** Path to image */
    IFX_char_t  fileName[1];
} DTI_H2D_ImageDownloadStart_t;

/**
   Execute remote script - execute remote shell script  .
*/
typedef struct
{
    /** A string with a script for remote execution */
    IFX_char_t scriptToExecute[1];
} DTI_H2D_ExecuteRemoteShScript_t;

/**
   Image download - transfere an binary the target system(board) to the host(PC).

   The target responce to a previous \ref  DTI_H2D_ImageDownloadStart_t command
   with the max binary chunk size which can be processed.
*/
typedef struct {
   /** Image size to download */
   IFX_uint32_t imageSize;
   /** Maximum size in number of bytes for image chunks to send */
   IFX_uint32_t maxChunkSize;
} DTI_D2H_ImageDownloadStart_t;

/**
   Image load - transfere an binary form the host (PC) to the target system.

   The target responce to a previous \ref  DTI_H2D_ImageLoadStart_t command
   with the max binary chunk size which can be processed.

*/
typedef struct {
   /** Maximum size in number of bytes for image chunks to send */
   IFX_uint32_t maxChunkSize;

} DTI_D2H_ImageLoadStart_t;

/**
   Image load - Data Chunk Upload.

   Packet type: \ref  DTI_PacketType_eImageLoadChunk .

   With this packet the image is send in chunks to the board. This ensures a
   response in a certain amount of time even with a slow connection.
   The host side can suggest a maximum number of bytes which are transferred
   in one packet, the target should accept this size.
   But the PC side can even send less than that per packet.

\remarks The implementation on the board has to check whether the written
   data fits to the allocated image size.
   A error indication should be generated otherwise.

*/
typedef struct {
   /** The byte offset into the image of this chunk. */
   IFX_uint32_t offset;
   /** Size in number of bytes for this chunk. */
   IFX_uint32_t chunkSize;
   /** Data chunk */
   IFX_uint8_t  data[1];

} DTI_H2D_ImageLoadChunk_t;

/**
   Image download - Data Chunk Download.

   Packet type: \ref  DTI_PacketType_eImageDownloadChunk .

   With this packet the image is send in chunks form a board to a host PC. This ensures a
   response in a certain amount of time even with a slow connection.
*/
typedef struct
{
   /** The byte offset into the image of this chunk. */
   IFX_uint32_t offset;
   /** Size in number of bytes for this chunk. */
   IFX_uint32_t chunkSize;

} DTI_H2D_ImageDownloadChunk_t;

/**
   Image download - Data Chunk Download.

   Packet type: \ref  DTI_PacketType_eImageDownloadChunk .

   Response from board to pc with a chunk data.
*/
typedef struct
{
    /** The byte offset into the image of this chunk. */
    IFX_uint32_t offset;
    /** Size in number of bytes for this chunk. */
    IFX_uint32_t chunkSize;
    /** Data chunk */
    IFX_uint8_t  data[1];

} DTI_D2H_ImageDownloadChunk_t;

/**
   Image release - release the selected image for the target sytem.
*/
typedef struct {
   /** Identify the image for later use */
   IFX_uint32_t   imageNum;
} DTI_H2D_ImageRelease_t;


/** Packet Write To File - cntrlOption: release the image after write */
#define DTI_WRITE_TO_FILE_CNTRL_RELEASE         0x00000001

/**
   Image write - write the selected image to the target file system.

\remarks
   Therefore the target system must support a OS specific file system.
*/
typedef struct {
   /** Identify the image for later use */
   IFX_uint32_t   imageNum;
   /** additional control options
         Bit[0]: release the file after write */
   IFX_uint32_t   cntrlOption;
   /** full path name (must be a zero-terminated string) */
   IFX_char_t     fullPathName[1];
} DTI_H2D_ImageWriteToFile_t;

/**
   Image write - write the selected image to the target file system.

\remarks
   Therefore the target system must support a OS specific file system.
*/
typedef struct {
   /** Identify the image for later use */
   IFX_uint32_t   imageNum;
   /** return code of the DTI Agent related operation */
   IFX_int32_t    dtiCode;
   /** OS specific return code of the write operation */
   IFX_int32_t    osCode;
} DTI_D2H_ImageWriteToFile_t;

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* _IFX_DTI_PROTOCOL_H */
