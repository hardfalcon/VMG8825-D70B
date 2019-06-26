#ifndef _IFXOS_LINUX_SOCKET_IPV6_H
#define _IFXOS_LINUX_SOCKET_IPV6_H
/******************************************************************************

                               Copyright (c) 2014
                            Lantiq Deutschland GmbH

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#ifdef LINUX

/** \file
   This file contains LINUX definitions for Socket IP V6.
*/

/** \defgroup IFXOS_SOCKET_LINUX_IPV6 Socket IP V6 (Linux User Space)

   This Group contains the LINUX Socket definitions and function.


\ingroup IFXOS_LAYER_LINUX
*/

#ifdef __cplusplus
   extern "C" {
#endif

/* ============================================================================
   IFX LINUX adaptation - Includes
   ========================================================================= */
#ifdef __KERNEL__
#	include <linux/net.h>
#	include <linux/in.h>
#	include <linux/in6.h>
#else
#	include <sys/types.h>
#	include <sys/socket.h>
#	include <sys/select.h>
#	include <netinet/in.h>
#	include <arpa/inet.h>
/*
	ToDo: add IP V6 user space includes
*/
#endif

#include "ifx_types.h"

/* get IP V4 defines */
#if ( !defined(IFXOS_FLAT_HIRACHY) || (IFXOS_FLAT_HIRACHY == 0) )
#	include "linux/ifxos_linux_socket.h"
#else
#	include "ifxos_linux_socket.h"
#endif

/* ============================================================================
   IFX LINUX adaptation - supported features
   ========================================================================= */
#ifdef __KERNEL__
	/** IFX LINUX adaptation - support IP V6 (Linux Kernel Space) */
#	ifndef HAVE_IFXOS_IPV6_SUPPORT
#		define HAVE_IFXOS_IPV6_SUPPORT		0
#	endif
#else
#	if defined(HAVE_IFXOS_IPV6_SUPPORT) && (HAVE_IFXOS_IPV6_SUPPORT == 1)
#		error "Linux User Space: Socket - IP V6 not supported / tested"
#	endif
#	if defined(HAVE_IFXOS_IPV6_SUPPORT)
#		undef HAVE_IFXOS_IPV6_SUPPORT
#	endif
#	define HAVE_IFXOS_IPV6_SUPPORT			0
#endif

/* ============================================================================
   IFX LINUX adaptation - types and defines
   ========================================================================= */

#ifdef __KERNEL__
/** Wrap IP V6 address family define AF_INET6 */
#define IFXOS_SOC_AF_INET6	AF_INET6

/** Wrap the IP V6 socket address type */
typedef struct sockaddr_in6	IFXOS_sockAddr6_t;

#else	/* #ifdef __KERNEL__ ... #else ... */

/** Wrap IP V6 address family define AF_INET6 */
#define IFXOS_SOC_AF_INET6	AF_INET6

/** Wrap the IP V6 socket address type */
typedef struct sockaddr_in6	IFXOS_sockAddr6_t;

#endif	/* #ifdef __KERNEL__ */

#ifdef __cplusplus
}
#endif
#endif      /* #ifdef LINUX */
#endif      /* #ifndef _IFXOS_LINUX_SOCKET_IPV6_H */

