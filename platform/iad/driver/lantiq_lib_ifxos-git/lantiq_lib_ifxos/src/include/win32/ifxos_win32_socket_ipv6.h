#ifndef _IFXOS_WIN32_SOCKET_IPV6_H
#define _IFXOS_WIN32_SOCKET_IPV6_H
/******************************************************************************

                               Copyright (c) 2014
                            Lantiq Deutschland GmbH

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#ifdef WIN32

/** \file
   This file contains Win32 definitions for Socket IP V6.
*/

/** \defgroup IFXOS_SOCKET_WIN32_IPV6 Socket IP V6 (Win32)

   This Group contains the Win32 Socket definitions and function.

\ingroup IFXOS_LAYER_WIN32
*/

#ifdef __cplusplus
   extern "C" {
#endif

/* ============================================================================
   IFX Win32 adaptation - Includes
   ========================================================================= */
#include "ifx_types.h"
#include <Winsock2.h>
#include <Ws2ipdef.h>

/* get IP V4 defines */
#if ( !defined(IFXOS_FLAT_HIRACHY) || (IFXOS_FLAT_HIRACHY == 0) )
#	include "win32/ifxos_win32_socket.h"
#else
#	include "ifxos_win32_socket.h"
#endif

/* ============================================================================
   IFX Win32 adaptation - supported features
   ========================================================================= */

#if defined(HAVE_IFXOS_IPV6_SUPPORT) && (HAVE_IFXOS_IPV6_SUPPORT == 1)
#	error "Win32: Socket - IP V6 not supported / tested"
#endif

#if defined(HAVE_IFXOS_IPV6_SUPPORT)
#	undef HAVE_IFXOS_IPV6_SUPPORT
#endif
#define HAVE_IFXOS_IPV6_SUPPORT			0


/* ============================================================================
   IFX Win32 adaptation - types and defines
   ========================================================================= */

/** Wrap IP V6 address family define AF_INET6 */
#define IFXOS_SOC_AF_INET6	AF_INET6
/** Wrap the IP V6 socket address type */
typedef struct sockaddr_in6	IFXOS_sockAddr6_t;

#ifdef __cplusplus
}
#endif
#endif      /* #ifdef WIN32 */
#endif      /* #ifndef _IFXOS_WIN32_SOCKET_IPV6_H */

