#ifndef _IFXOS_NUCLEUS_SOCKET_IPV6_H
#define _IFXOS_NUCLEUS_SOCKET_IPV6_H
/******************************************************************************

                               Copyright (c) 2014
                            Lantiq Deutschland GmbH

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#ifdef NUCLEUS_PLUS

/** \file
   This file contains Nucleus definitions for Socket IP V6.
*/

/** \defgroup IFXOS_SOCKET_NUCLEUS_IPV6 Socket IP V6 (Nucleus)

   This Group contains the Nucleus Socket definitions and function.

\ingroup IFXOS_LAYER_NUCLEUS
*/

/* ============================================================================
   IFX Nucleus adaptation - Includes
   ========================================================================= */
#include <nucleus.h>
#include "ifx_types.h"

/* get IP V4 defines */
#if ( !defined(IFXOS_FLAT_HIRACHY) || (IFXOS_FLAT_HIRACHY == 0) )
#	include "nucleus/ifxos_nucleus_os_socket.h"
#else
#	include "ifxos_nucleus_socket.h"
#endif

#ifdef __cplusplus
   extern "C" {
#endif

/* ============================================================================
   IFX Nucleus adaptation - supported features
   ========================================================================= */

#if defined(HAVE_IFXOS_IPV6_SUPPORT) && (HAVE_IFXOS_IPV6_SUPPORT == 1)
#	error "Nucleus: Socket - IP V6 not supported / tested"
#endif

#if defined(HAVE_IFXOS_IPV6_SUPPORT)
#	undef HAVE_IFXOS_IPV6_SUPPORT
#endif
#define HAVE_IFXOS_IPV6_SUPPORT			0

/* ============================================================================
   IFX Nucleus adaptation - types and defines
   ========================================================================= */

/** Wrap IP V6 address family define AF_INET6 */
#define IFXOS_SOC_AF_INET6	AF_INET6
/** Wrap the IP V6 socket address type */
typedef struct sockaddr_in6	IFXOS_sockAddr6_t;

#ifdef __cplusplus
}
#endif

#endif      /* #ifdef NUCLEUS_PLUS */
#endif      /* #ifndef _IFXOS_NUCLEUS_SOCKET_IPV6_H */


