#ifndef _DTI_CLIENT_PACKET_PROTOCOL_H
#define _DTI_CLIENT_PACKET_PROTOCOL_H
/******************************************************************************

                              Copyright (c) 2009
                            Lantiq Deutschland GmbH
                     Am Campeon 3; 85579 Neubiberg, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/** \file
   DTI Client Protocol library API.
*/

/** \page  DTI_CLIENT_OVERVIEW_PAGE DTI Client Overview

   For the implementation of a DTI Client the basic functions of the DTI Library
   concerning connection handling and packet handling can be used.

   But for the setup of a DTI packet on client side there are different functions
   necessary. The following shows the available client functions which are
   seperated into the corresponding groups:

   - Basic protocol specific funtions
   - device specific functions
   - CLI specific functions.

   - \subpage DTI_CLIENT_PROTOCOL_PAGE
   - \subpage DTI_CLIENT_DEVICEL_PAGE
   - \subpage DTI_CLIENT_CLI_PAGE
*/

/** \page  DTI_CLIENT_PROTOCOL_PAGE DTI Client Protocol Module

   This module provides functions to setup, manage and send a DTI packet on client
   side. On client side the operations are different to the job on agent side and depend
   on the specific packet to send.

   Therefore the module provides functions for
   - setup protocol specific settings.
   - setup a loopback packet.
   - setup a "system info" request packet.
   - setup packets to upload a file.
   - send a DTI packet
   - receive a DTI packet (with timeout)-

   For more information see \ref DTI_CLIENT_PROTOCOL_IF
*/

/** \page  DTI_CLIENT_DEVICEL_PAGE DTI Client Device Module
   This module provides functions to setup a DTI packet on client side for a
   supported device.

   Therefore the module provides functions to setup DTI packets on client side
   for:
   - device control and management (like reset, FW download, configuration)
   - Device Register access
   - Message handling
   - Device Debug access
   - Trace buffer handling (VINAX only)

   For more information see \ref DTI_CLIENT_DEVICE_IF
*/

/** \page  DTI_CLIENT_CLI_PAGE DTI Client CLI Module
   This module provides functions to setup a DTI packet on client side for the
   Command Line Interface (CLI) handling

   For a command line interface handling, the client has to setup a DTI packet
   which sends a command in string format to the agent where the line / port
   number for such a packet selects one of the connected CLI interfaces on
   the agent side.

   For more information see \ref DTI_CLIENT_CLI_IF
*/

/** \defgroup DTI_CLIENT_PROTOCOL_IF DTI Client Protocol Interface

   This Group contains the definitions and functions of the Debug and Trace
   protocol Layer used on client side.

   To fullfill the protocol, basic functions are provided to setup and manage
   DTI packets. This module provides routines for setup and send a DTI packet
   corresponding to the DTI protocol.
*/

#ifdef __cplusplus
   extern "C" {
#endif

/* ============================================================================
   Includes
   ========================================================================= */

#include "ifx_types.h"

#include "ifx_dti_protocol.h"
#include "dti_connection_interface.h"
#include "dti_protocol_interface.h"

/* ============================================================================
   Macro Defines
   ========================================================================= */
/** \addtogroup DTI_CLIENT_PROTOCOL_IF
*{ */


/* ============================================================================
   Type Defines
   ========================================================================= */


/**
   DTI Protocol Client Context
*/
typedef struct {
   /** remote connection */
   DTI_Connection_t     dtiClientCon;

   /** points to the IN Packet (buffer IN) */
   DTI_packetBuffer_t   packetIn;
   /** points to the OUT Packet (buffer OUT) */
   DTI_packetBuffer_t   packetOut;

} DTI_CLIENT_ProtocolCtx_t;


/* ============================================================================
   Exports
   ========================================================================= */

IFXOS_PRN_USR_MODULE_DECL(DTI_CLIENT);

/**
   Setup Protocol Packet Info with the given settings.

\param
   pDtiPacketSend    - points to the packet to send.
\param
   packetBufferSize  - buffer size of the send packet.
\param
   packetType        - packet type to set
\param
   payloadType       - payload type (8, 16, 32 Bit, mixed mode)
\param
   pPacketPayload    - points to the packet payload
\param
   payloadSize_byte  - size of the given payload data [byte]

\return
   IFX_SUCCESS setup done.
   IFX_ERROR   missing args or missaligned payload.
*/
extern IFX_int_t DTI_CLIENT_packetProt_Set(
                        DTI_Packet_t      *pDtiPacketSend,
                        IFX_uint_t        packetBufferSize,
                        DTI_PacketType_t  packetType,
                        DTI_PayloadType_t payloadType,
                        IFX_uint8_t       *pPacketPayload,
                        IFX_uint_t        payloadSize_byte);

/**
   Setup Protocol Loopback Packet Infof with the given settings.

\param
   pDtiPacketSend    - points to the packet to send.
\param
   packetBufferSize  - buffer size of the send packet.
\param
   payloadType       - payload type of the packet to send (8, 16, 32 Bit, mixed mode)
\param
   pPacketPayload    - points to the packet payload
\param
   payloadSize_byte  - size of send payload [byte].

\return
   IFX_SUCCESS setup done.
   IFX_ERROR   missing args or missaligned payload.
*/
extern IFX_int_t DTI_CLIENT_packetProtLoopBack_Set(
                        DTI_Packet_t      *pDtiPacketSend,
                        IFX_uint_t        packetBufferSize,
                        DTI_PayloadType_t payloadType,
                        IFX_uint8_t       *pPacketPayload,
                        IFX_uint_t        payloadSize_byte);

/**
   Setup Protocol Error Packet with the given settings.

\param
   pDtiPacketSend    - points to the packet to send.
\param
   packetBufferSize  - buffer size of the send packet.
\param
   packetError       - packet error information

\return
   IFX_SUCCESS setup done.
   IFX_ERROR   missing args or missaligned payload.
*/
extern IFX_int_t DTI_CLIENT_packetProtError_Set(
                        DTI_Packet_t      *pDtiPacketSend,
                        IFX_uint_t        packetBufferSize,
                        DTI_PacketError_t packetError);

/**
   Setup Protocol System Info Get Packet.

\param
   pDtiPacketSend    - points to the packet to send.
\param
   packetBufferSize  - buffer size of the send packet.

\return
   IFX_SUCCESS setup done.
   IFX_ERROR   missing args or missaligned payload.
*/
extern IFX_int_t DTI_CLIENT_packetProtSysInfo_Set(
                        DTI_Packet_t      *pDtiPacketSend,
                        IFX_uint_t        packetBufferSize);

/**
   Setup Protocol "start image load" Packet with the given settings.

\param
   pDtiPacketSend - points to the IN packet.
\param
   packetBufferSize - payload size of the IN packet.
\param
   imageNum       - image ID to identify the binary
\param
   imageSize_byte - size of the image.

\return
   IFX_SUCCESS setup done.
   IFX_ERROR   missing args or missaligned payload.
*/
extern IFX_int_t DTI_CLIENT_packetProtImageLoadStart_Set(
                        DTI_Packet_t      *pDtiPacketSend,
                        IFX_uint_t        packetBufferSize,
                        IFX_uint_t        imageNum,
                        IFX_uint_t        imageSize_byte);

/**
   Setup Protocol "start image download" Packet with the given settings.

\param
   pDtiPacketSend - points to the IN packet.
\param
   packetBufferSize - payload size of the IN packet.
\param
   imageNum       - image ID to identify the binary
\param
   imageSize_byte - size of the image.

\return
   IFX_SUCCESS setup done.
   IFX_ERROR   missing args or missaligned payload.
*/
extern IFX_int_t DTI_CLIENT_packetProtImageDownloadStart_Set(
                        DTI_Packet_t      *pDtiPacketSend,
                        IFX_uint_t         packetBufferSize,
                        IFX_uint_t         imageNum,
                        const IFX_char_t  *fileName );


/**
   Setup Protocol "image load chunk" Packet with the given settings.

\param
   pDtiPacketSend    - points to the packet to send.
\param
   packetBufferSize  - buffer size of the send packet.
\param
   chunkSize_byte    - size of the chunk.
\param
   offset_byte       - offset within the image.
\param
   pImage           - points to the image to upload.

\return
   IFX_SUCCESS setup done.
   IFX_ERROR   missing args or missaligned payload.
*/
extern IFX_int_t DTI_CLIENT_packetProtImageLoadChunk_Set(
                        DTI_Packet_t      *pDtiPacketSend,
                        IFX_uint_t        packetBufferSize,
                        IFX_uint_t        chunkSize_byte,
                        IFX_uint_t        offset_byte,
                        IFX_uint8_t       *pImage);

/**
   Setup Protocol "execute remote shell script" Packet with the given settings.

\param
   pDtiPacketSend    - points to the packet to send.
\param
   packetBufferSize  - buffer size of the send packet.
\param
   remoteScript      - remote script to execute.


\return
   IFX_SUCCESS setup done.
   IFX_ERROR   missing args or missaligned payload.
*/

extern IFX_int_t DTI_CLIENT_packetProtExecuteRemoteShScript_Set( DTI_Packet_t      *pDtiPacketSend,
                                                                 IFX_uint_t        packetBufferSize,
                                                                 const IFX_char_t* remoteScript);
/**
   Setup Protocol "image download chunk" Packet with the given settings.

\param
   pDtiPacketSend    - points to the packet to send.
\param
   packetBufferSize  - buffer size of the send packet.
\param
   chunkSize_byte    - size of the chunk.
\param
   offset_byte       - offset within the image.
\param
   pImage           - points to the image to download to.

\return
   IFX_SUCCESS setup done.
   IFX_ERROR   missing args or missaligned payload.
*/
extern IFX_int_t DTI_CLIENT_packetProtImageDownloadChunk_Set(
                            DTI_Packet_t      *pDtiPacketSend,
                            IFX_uint_t        packetBufferSize,
                            IFX_uint_t        chunkSize_byte,
                            IFX_uint_t        offset_byte);

/**
   Setup Protocol "Write To File" Packet with the given settings.
   This message will trigger the DTI Agent on the target system to write
   the selected image to the target file system.

\param
   pDtiPacketSend    - points to the packet to send.
\param
   packetBufferSize  - buffer size of the send packet.
\param
   imageNum          - image ID to identify the binary
\param
   cntrlOpt          - additonal control options
                        0x1: release image from DTI context after write
\param
   pDestFilename     - file name used on the destination target.

\return
   IFX_SUCCESS setup done.
   IFX_ERROR   missing args or missaligned payload.
*/
extern IFX_int_t DTI_CLIENT_packetProtWriteToFile_Set(
                        DTI_Packet_t      *pDtiPacketSend,
                        IFX_uint_t        packetBufferSize,
                        IFX_uint_t        imageNum,
                        IFX_uint_t        cntrlOpt,
                        const IFX_char_t  *pDestFilename);

/**
   Setup Protocol "Release Image" Packet with the given settings.
   This message will trigger the DTI Agent to release the selected image 
   from the DTI Agent context.

\param
   pDtiPacketSend    - points to the packet to send.
\param
   packetBufferSize  - buffer size of the send packet.
\param
   imageNum          - image ID to identify the binary

\return
   IFX_SUCCESS setup done.
   IFX_ERROR   missing args or missaligned payload.
*/
extern IFX_int_t DTI_CLIENT_packetProtImageRelease_Set(
                        DTI_Packet_t      *pDtiPacketSend,
                        IFX_uint_t        packetBufferSize,
                        IFX_uint_t        imageNum);

/**
   Send a DTI packet via the DTI connection.

\param
   pDtiConnection    - points to the DTI connection.
\param
   pDtiPacketSend    - points to the packet to send.
\param
   dtiPort           - line / device / interface number.
\param
   dtiTan            - trans action number.

\return
   IFX_SUCCESS send done.
   IFX_ERROR
*/
extern IFX_int_t DTI_CLIENT_packetSend(
                        const DTI_Connection_t  *pDtiConnection,
                        DTI_Packet_t            *pDtiPacketSend,
                        IFX_uint32_t            dtiPort,
                        IFX_uint32_t            dtiTan);

/**
   Read from  a client DTI connection

\param
   pDtiConnection - points to the DTI client connection data.
\param
   pDtiConCntrl   - points to the DTI client connection control structure.
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
extern IFX_int_t DTI_CLIENT_packetTimedRead(
                        DTI_Connection_t        *pDtiConnection,
                        DTI_ConnectionCntrl_t   *pDtiConCntrl,
                        IFX_int_t               timeOut_ms,
                        IFX_char_t              *pRecvBuffer,
                        IFX_int_t               recvBufferSize_byte);

/** *} */

#ifdef __cplusplus
}
#endif

#endif /* _DTI_CLIENT_PACKET_PROTOCOL_H */
