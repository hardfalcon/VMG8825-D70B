#ifndef _IFXOS_RTEMS_SOCKET_IPV6_H
#define _IFXOS_RTEMS_SOCKET_IPV6_H
/******************************************************************************

                              Copyright (c) 2009
                            Lantiq Deutschland GmbH
                     Am Campeon 3; 85579 Neubiberg, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/


#ifdef RTEMS

/** \file
   This file contains RTEMS definitions for Socket IP V6.
*/

/** \defgroup IFXOS_SOCKET_RTEMS_IPV6 Socket IP V6 (Generic OS)

   This Group contains the RTEMS Socket definitions and function.

\ingroup IFXOS_LAYER_RTEMS
*/

/* ============================================================================
   RTEMS adaptation - Includes
   ========================================================================= */
#include "ifx_types.h"

/* get IP V4 defines */
#if ( !defined(IFXOS_FLAT_HIRACHY) || (IFXOS_FLAT_HIRACHY == 0) )
#	include "rtems/ifxos_rtems_socket.h"
#else
#	include "ifxos_rtems_socket.h"
#endif

#ifdef __cplusplus
   extern "C" {
#endif

/* ============================================================================
   RTEMS adaptation - supported features
   ========================================================================= */

#if defined(HAVE_IFXOS_IPV6_SUPPORT) && (HAVE_IFXOS_IPV6_SUPPORT == 1)
#	error "RTEMS: Socket - IP V6 not supported / tested"
#endif

#if defined(HAVE_IFXOS_IPV6_SUPPORT)
#	undef HAVE_IFXOS_IPV6_SUPPORT
#endif
#define HAVE_IFXOS_IPV6_SUPPORT			0

/* ============================================================================
   RTEMS adaptation - types and defines
   ========================================================================= */

/** Wrap IP V6 address family define AF_INET6 */
#define IFXOS_SOC_AF_INET6	AF_INET6
/** Wrap the IP V6 socket address type */
typedef struct sockaddr_in6	IFXOS_sockAddr6_t;

#ifdef __cplusplus
}
#endif

#endif      /* #ifdef RTEMS */
#endif      /* #ifndef _IFXOS_RTEMS_SOCKET_IPV6_H */


