#ifndef _IFXOS_VXWORKS_SOCKET_IPV6_H
#define _IFXOS_VXWORKS_SOCKET_IPV6_H
/******************************************************************************

                               Copyright (c) 2014
                            Lantiq Deutschland GmbH

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#ifdef VXWORKS

/** \file
   This file contains VxWorks definitions for Device Access.
*/

/** \defgroup IFXOS_SOCKET_VXWORKS_IPV6 Socket IP V6 (VxWorks)

   This Group contains the VxWorks Socket definitions and function.


\ingroup IFXOS_LAYER_VXWORKS
*/

#ifdef __cplusplus
   extern "C" {
#endif

/* ============================================================================
   IFX VxWorks adaptation - Includes
   ========================================================================= */
#include <vxWorks.h>
#include <sockLib.h>
#include <inetLib.h>
#include <selectLib.h>

#include "ifx_types.h"

/* get IP V4 defines */
#if ( !defined(IFXOS_FLAT_HIRACHY) || (IFXOS_FLAT_HIRACHY == 0) )
#	include "vxworks/ifxos_vxworks_socket.h"
#else
#	include "ifxos_vxworks_socket.h"
#endif

/* ============================================================================
   IFX VxWorks adaptation - supported features
   ========================================================================= */

#if defined(HAVE_IFXOS_IPV6_SUPPORT) && (HAVE_IFXOS_IPV6_SUPPORT == 1)
#	error "VxWorks: Socket - IP V6 not supported / tested"
#endif

#if defined(HAVE_IFXOS_IPV6_SUPPORT)
#	undef HAVE_IFXOS_IPV6_SUPPORT
#endif
#define HAVE_IFXOS_IPV6_SUPPORT			0

/* ============================================================================
   IFX VxWorks adaptation - types and defines
   ========================================================================= */

/** Wrap IP V6 address family define AF_INET6 */
#define IFXOS_SOC_AF_INET6	AF_INET6
/** Wrap the IP V6 socket address type */
typedef struct sockaddr_in6	IFXOS_sockAddr6_t;

#ifdef __cplusplus
}
#endif
#endif      /* #ifdef VXWORKS */
#endif      /* #ifndef _IFXOS_VXWORKS_SOCKET_IPV6_H */


