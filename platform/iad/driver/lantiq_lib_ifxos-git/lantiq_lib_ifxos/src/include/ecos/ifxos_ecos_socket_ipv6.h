#ifndef _IFXOS_ECOS_SOCKET_IPV6_H
#define _IFXOS_ECOS_SOCKET_IPV6_H
/******************************************************************************

                               Copyright (c) 2014
                            Lantiq Deutschland GmbH

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/


#ifdef ECOS

/** \file
   This file contains eCos definitions for Socket IP V6.
*/

/** \defgroup IFXOS_SOCKET_ECOS_IPV6 Socket IP V6 (eCos)

   This Group contains the eCos Socket definitions and function.

\ingroup IFXOS_LAYER_ECOS
*/

/* ============================================================================
   IFX eCos adaptation - Includes
   ========================================================================= */

#if defined(CYGPKG_NET) || defined(CYGPKG_NET_LWIP)
#  ifdef CYGPKG_NET_LWIP
#     include <time.h>
#  endif
#  include <network.h>
#endif

/* get IP V4 defines */
#if ( !defined(IFXOS_FLAT_HIRACHY) || (IFXOS_FLAT_HIRACHY == 0) )
#	include "ecos/ifxos_ecos_socket.h"
#else
#	include "ifxos_ecos_socket.h"
#endif

#ifdef __cplusplus
   extern "C" {
#endif

/* ============================================================================
   IFX eCos adaptation - supported features
   ========================================================================= */

#if defined(HAVE_IFXOS_IPV6_SUPPORT) && (HAVE_IFXOS_IPV6_SUPPORT == 1)
#	error "eCos: Socket - IP V6 not supported / tested"
#endif

#if defined(HAVE_IFXOS_IPV6_SUPPORT)
#	undef HAVE_IFXOS_IPV6_SUPPORT
#endif
#define HAVE_IFXOS_IPV6_SUPPORT			0

/* ============================================================================
   IFX eCos adaptation - types and defines
   ========================================================================= */

/** Wrap IP V6 address family define AF_INET6 */
#define IFXOS_SOC_AF_INET6	AF_INET6
/** Wrap the IP V6 socket address type */
typedef struct sockaddr_in6	IFXOS_sockAddr6_t;

#ifdef __cplusplus
}
#endif

#endif      /* #ifdef ECOS */
#endif      /* #ifndef _IFXOS_ECOS_SOCKET_IPV6_H */


