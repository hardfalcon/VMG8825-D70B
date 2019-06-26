#ifndef _DTI_CLIENT_PACKET_CLI_H
#define _DTI_CLIENT_PACKET_CLI_H
/******************************************************************************

                              Copyright (c) 2009
                            Lantiq Deutschland GmbH
                     Am Campeon 3; 85579 Neubiberg, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/** \file
   DTI Client CLI Protocol library API.
*/

/** \defgroup DTI_CLIENT_CLI_IF DTI Client Command Line Interface (CLI)

   This Group contains the definitions and functions of the DTI Command Line
   Interface Layer used on client side.

   On agent side an application can register its own CLI interface within
   the DTI Agent. From now on the DTI Agent is able to forward incoming
   CLI commands to such a registered interface.

   For the CLI access this module provides functions to setup the
   corresponding DTI packet for
   - request the current CLI configuration and setup from the DTI Agent
   - setup a DTI String packet containing the command in string format
*/


#ifdef __cplusplus
   extern "C" {
#endif

/* ============================================================================
   Includes
   ========================================================================= */

#include "ifx_types.h"
#include "ifx_dti_protocol.h"

/** \addtogroup DTI_CLIENT_CLI_IF
   *{ */
/* ============================================================================
   Macro Defines
   ========================================================================= */

/* ============================================================================
   Type Defines
   ========================================================================= */

/* ============================================================================
   Exports
   ========================================================================= */


/**
   Setup Device Packet "DTI_PacketType_eCliString" to send a CLI command to the
   DTI Agent

\param
   pDtiPacketSend    - points to the IN packet.
\param
   paylBufferSize    - size of the IN packet payload buffer.
\param
   pCliString        - points to the user CLI command string.
\param
   userPayloadSize   - Size of the user payload to send, if not set the command
                       string length + termination will be used.

\return
   IFX_SUCCESS setup done.
   IFX_ERROR   missing args or missaligned payload.
*/
extern IFX_int_t DTI_CLIENT_packetCliString_Set(
                        DTI_Packet_t   *pDtiPacketSend,
                        IFX_uint_t     paylBufferSize,
                        IFX_char_t     *pCliString,
                        IFX_uint_t     userPayloadSize);

/**
   Setup Device Packet "DTI_PacketType_eCliInfoGet" to request the CLI info
   from the DTI Agent.

\param
   pDtiPacketSend   - points to the IN packet.
\param
   paylBufferSize    - payload size of the IN packet.

\return
   IFX_SUCCESS setup done.
   IFX_ERROR   missing args or missaligned payload.
*/
extern IFX_int_t DTI_CLIENT_packetCliInfoGet_Set(
                        DTI_Packet_t   *pDtiPacketSend,
                        IFX_uint_t     paylBufferSize);


/** *} */

#ifdef __cplusplus
}
#endif

#endif /* _DTI_CLIENT_PACKET_DEVICE_H */
