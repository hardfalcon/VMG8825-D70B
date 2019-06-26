#ifndef _IFX_DTI_PROTOCOL_CLI_H
#define _IFX_DTI_PROTOCOL_CLI_H
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

/** \defgroup DTI_PROTOCOL_CLI DTI Protocol (CLI Part)

   This Group contains definitions of the Command Line Interface (CLI) specific
   part of the Debug and Trace Interface (DTI) Protocol.

   Wihtin this header the CLI specific definitions are defined, this are:
   - DTI CLI control  specific message types and options.
   - DTI CLI send packet structures.

\ingroup DTI_PROTOCOL
*/

#ifdef __cplusplus
   extern "C" {
#endif

/* ============================================================================
   Includes
   ========================================================================= */
#include "ifx_types.h"
#include "ifx_dti_protocol.h"


/** \addtogroup DTI_PROTOCOL_CLI
   *{ */

/* ============================================================================
   Packet Type: 0x0005xxxx - Command Line Interface (CLI)
   ========================================================================= */

/* ============================================================================
   Defines
   ========================================================================= */

/** DTI Command Line Interface - max length of a CLI name */
#define DTI_CLI_MAX_NAME_LEN                    16
/** DTI Command Line Interface - default loop back interface name */
#define DTI_CLI_DEFAULT_LOOPBACK_IF_NAME        "cli_loop"


/* ============================================================================
   Types
   ========================================================================= */

/**
   Possible CLI packet types
*/
typedef enum {
/*
   Packet Type: 0x0005xxxx - Command Line Interface (CLI)
*/
/** CLI - information get */
   DTI_PacketType_eCliInfoGet          = 0x00050000,
/** CLI - Single string.
   Complete CLI string or final packet of a sequence of 
   fragments of packet type DTI_PacketType_eCliPartialString. */
   DTI_PacketType_eCliString           = 0x00050001,
/** CLI - Partial response string.
   Big CLI contents may be fragmented into several Packets of this type.
   The CLI content is terminated by a Packet of type DTI_PacketType_eCliString. */
   DTI_PacketType_eCliPartialString    = 0x00050002
} DTI_PacketTypeCli_t;

/* ============================================================================
   Packet Group: 0x0005xxxx - Command Line Interface.
   ========================================================================= */

/** *} */

#ifdef __cplusplus
}
#endif

#endif /* _IFX_DTI_PROTOCOL_CLI_H */



