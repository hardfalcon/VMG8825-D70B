#ifndef _IFXOS_GENERIC_OS_SOCKET_IPV6_H
#define _IFXOS_GENERIC_OS_SOCKET_IPV6_H
/******************************************************************************

                               Copyright (c) 2014
                            Lantiq Deutschland GmbH

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#ifdef GENERIC_OS

/** \file
   This file contains Generic OS definitions for Socket IP V6.
*/

/** \defgroup IFXOS_SOCKET_GENERIC_OS_IPV6 Socket IP V6 (Generic OS)

   This Group contains the Generic OS Socket definitions and function.

\ingroup IFXOS_LAYER_GENERIC_OS
*/

/* ============================================================================
   IFX Generic OS adaptation - Includes
   ========================================================================= */
#include "ifx_types.h"

/* get IP V4 defines */
#if ( !defined(IFXOS_FLAT_HIRACHY) || (IFXOS_FLAT_HIRACHY == 0) )
#	include "generic_os/ifxos_generic_os_socket.h"
#else
#	include "ifxos_generic_os_socket.h"
#endif

#ifdef __cplusplus
   extern "C" {
#endif

/* ============================================================================
   IFX Generic OS adaptation - supported features
   ========================================================================= */

#if defined(HAVE_IFXOS_IPV6_SUPPORT) && (HAVE_IFXOS_IPV6_SUPPORT == 1)
#	error "Generic OS: Socket - IP V6 not supported / tested"
#endif

#if defined(HAVE_IFXOS_IPV6_SUPPORT)
#	undef HAVE_IFXOS_IPV6_SUPPORT
#endif
#define HAVE_IFXOS_IPV6_SUPPORT			0

/* ============================================================================
   IFX Generic OS adaptation - types and defines
   ========================================================================= */

/** Wrap IP V6 address family define AF_INET6 */
#define IFXOS_SOC_AF_INET6	AF_INET6
/** Wrap the IP V6 socket address type */
typedef struct sockaddr_in6	IFXOS_sockAddr6_t;

#ifdef __cplusplus
}
#endif

#endif      /* #ifdef GENERIC_OS */
#endif      /* #ifndef _IFXOS_GENERIC_OS_SOCKET_IPV6_H */


