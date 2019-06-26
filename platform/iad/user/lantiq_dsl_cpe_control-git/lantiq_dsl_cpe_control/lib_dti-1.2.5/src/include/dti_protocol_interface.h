#ifndef _DTI_PROTOCOL_INTERFACE_H
#define _DTI_PROTOCOL_INTERFACE_H
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

/** \defgroup DTI_PROTOCOL_IF DTI Protocol Interface

   This Group contains the definitions and functions of the Debug and Trace
   protocol Layer.

   To fullfill the protocol, basic functions are provided to setup and manage
   DTI packets. This module provides routines for setup, update and change the
   DTI packet header and packet corresponding to the DTI protocol.

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

/* ============================================================================
   Macro Defines
   ========================================================================= */

/** \addtogroup DTI_PROTOCOL_IF
@{ */

/** receive buffer size */
#define DTI_IN_BUFFER_SIZE                            4096
/** send buffer size */
#define DTI_OUT_BUFFER_SIZE                           4096

/* ============================================================================
   Type Defines
   ========================================================================= */
/**
   DTI Packet buffer control struct
*/
typedef struct {
   /** points to the Packet (buffer) */
   DTI_Packet_t         *pPacket;
   /** DTI packet buffer for incoming / outgoing packets */
   IFX_char_t           buffer[DTI_IN_BUFFER_SIZE];
} DTI_packetBuffer_t;


/* ============================================================================
   Exports
   ========================================================================= */

IFXOS_PRN_USR_MODULE_DECL(DTI_PROTOCOL);

/**
   This function takes over the info from the IN header into the OUT header.

\param
   pHdrIn         - points to the received packet header.
\param
   pHdrOut        - points to the responce packet header.
\param
   nPayloadSize   - payload size which has to be set in the responce package.
\param
   packetError    - error code which has to be set in the responce package.

\return
   IFX_SUCCESS if setup was successful, else
   ERROR in case of missing args.
*/
extern IFX_int_t DTI_headerResponceSet(
                        const DTI_PacketHeader_t   *pHdrIn,
                        DTI_PacketHeader_t         *pHdrOut,
                        IFX_uint32_t               nPayloadSize,
                        DTI_PacketError_t          packetError);

/**
   This function setup a responce of a given packet

\param
   pDtiPacketIn   - points to the IN packet.
\param
   pDtiPacketOut  - points to the OUT packet.
\param
   packetError    - error code which has to be set in the responce package.
\param
   payloadSize    - payload size which has to be set in the responce package.
\param
   bufferOutSize  - buffer size of the OUT packet.
\param
   bCpyPayload    - flag to indicate if the payload will be copied.

\return
   IFX_SUCCESS if setup was successful, else
   ERROR in case of missing args.
*/
extern IFX_int_t DTI_packetResponceSet(
                        const DTI_Packet_t   *pDtiPacketIn,
                        DTI_Packet_t         *pDtiPacketOut,
                        DTI_PacketError_t    packetError,
                        IFX_uint32_t         payloadSize,
                        IFX_uint32_t         bufferOutSize,
                        IFX_boolean_t        bCpyPayload);

/**
   This function setup an "unknown packet"

\param
   pDtiPacketIn   - points to the IN packet.
\param
   pDtiPacketOut  - points to the OUT packet.
\param
   bufferOutSize  - size of the output buffer.

\return
   IFX_SUCCESS if setup was successful, else
   ERROR in case of missing args.
*/
extern IFX_int_t DTI_packetUnknownSet(
                        const DTI_Packet_t   *pDtiPacketIn,
                        DTI_Packet_t         *pDtiPacketOut,
                        IFX_uint32_t         bufferOutSize);

/**
   This function setup an "error packet"

\param
   pDtiPacketIn   - points to the IN packet.
\param
   pDtiPacketOut  - points to the OUT packet.
\param
   bufferOutSize  - size of the output buffer.
\param
   packetError    - error code which has to be set in the responce package.

\return
   IFX_SUCCESS if setup was successful, else
   ERROR in case of missing args.
*/
extern IFX_int_t DTI_packetErrorSet(
                        const DTI_Packet_t   *pDtiPacketIn,
                        DTI_Packet_t         *pDtiPacketOut,
                        IFX_uint32_t         bufferOutSize,
                        DTI_PacketError_t    packetError);

/**
   This function setup an "loopback packet"

\param
   pDtiPacketIn   - points to the IN packet.
\param
   pDtiPacketOut  - points to the OUT packet.
\param
   bufferOutSize  - size of the output buffer.

\return
   IFX_SUCCESS if setup was successful, else
   ERROR in case of missing args.
*/
extern IFX_int_t DTI_packetLoopBackSet(
                        const DTI_Packet_t   *pDtiPacketIn,
                        DTI_Packet_t         *pDtiPacketOut,
                        IFX_uint32_t         bufferOutSize);

/**
   Setup the fixed and packet type specific fields

\param
   pDtiPacketIn   - points to the IN packet.
\param
   packetType     - the used packet type to be set.
\param
   packetOptions  - the used packet options to be set.
\param
   payloadSize    - payload size which has to be set in the responce package.

\return
   IFX_SUCCESS
*/
extern IFX_int_t DTI_headerPacketTypeSet(
                        DTI_Packet_t   *pDtiPacketIn,
                        IFX_uint32_t   packetType,
                        IFX_uint32_t   packetOptions,
                        IFX_uint32_t   payloadSize);

/**
   Converts a given ASCII string to a unsigned long value.
\param
   pIn      - points to the input str. Depending on the base the
              corresponding characters are allowed.
\param
   base     - used base for convertion, if base is 0
               base 16 will be used if the string starts with "0x", else
               base 10 (dec) will be used.
\param
   pRetVal  - return pointer, points to the location to store the converted value.

\return
   IFX_SUCCESS convertion done, and the value is returned via the "pRetVal" arg
   IFX_ERROR   error in the convertion occurred.

\remark
   Shift to IFXOS ??
*/
extern IFX_int_t DTI_convertStrToUl(
                        IFX_char_t     *pIn,
                        IFX_int_t      base,
                        IFX_ulong_t    *pRetVal);


/** @} */


#ifdef __cplusplus
}
#endif

#endif /* _DTI_PROTOCOL_INTERFACE_H */
