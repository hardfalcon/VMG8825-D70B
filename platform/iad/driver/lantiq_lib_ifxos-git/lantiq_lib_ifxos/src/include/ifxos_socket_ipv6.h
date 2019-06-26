#ifndef _IFXOS_SOCKET_IPV6_H
#define _IFXOS_SOCKET_IPV6_H
/******************************************************************************

                               Copyright (c) 2014
                            Lantiq Deutschland GmbH

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/** \file
   This file contains definitions for IP V6 socket access.
   For the IP V6 socket access the current IP V4 interface is still required.
*/

/** \defgroup IFXOS_SOCKET_IPV6 Socket

   This Group contains the Socket definitions and function.

\ingroup IFXOS_INTERFACE
*/

#ifdef __cplusplus
   extern "C" {
#endif

/* ============================================================================
   IFX OS adaptation - Includes
   ========================================================================= */
#if ( !defined(IFXOS_FLAT_HIRACHY) || (IFXOS_FLAT_HIRACHY == 0) )
#	if defined(LINUX)
#		include "linux/ifxos_linux_socket_ipv6.h"
#	elif defined(VXWORKS)
#		include "vxworks/ifxos_vxworks_socket_ipv6.h"
#	elif defined(ECOS)
#		include "ecos/ifxos_ecos_socket_ipv6.h"
#	elif defined(NUCLEUS_PLUS)
#		include "nucleus/ifxos_nucleus_socket_ipv6.h"
#	elif defined(WIN32)
#		include "win32/ifxos_win32_socket_ipv6.h"
#	elif defined(RTEMS)
#		include "rtems/ifxos_rtems_socket_ipv6.h"
#	elif defined(GENERIC_OS)
#		include "generic_os/ifxos_generic_os_socket_ipv6.h"
#	else
#		error "Socket Adaptation - Please define your OS"
#	endif
#else
#	if defined(LINUX)
#		include "ifxos_linux_socket_ipv6.h"
#	elif defined(VXWORKS)
#		include "ifxos_vxworks_socket_ipv6.h"
#	elif defined(ECOS)
#		include "ifxos_ecos_socket_ipv6.h"
#	elif defined(NUCLEUS_PLUS)
#		include "ifxos_nucleus_socket_ipv6.h"
#	elif defined(WIN32)
#		include "ifxos_win32_socket_ipv6.h"
#	elif defined(RTEMS)
#		include "ifxos_rtems_socket_ipv6.h"
#	elif defined(GENERIC_OS)
#		include "ifxos_generic_os_socket_ipv6.h"
#	else
#		error "Socket Adaptation - Please define your OS"
#	endif
#endif


/* ============================================================================
   IFX OS adaptation - Socket
   ========================================================================= */

/** \addtogroup IFXOS_SOCKET
@{ */

/**
   This function creates a IPv6 socket.

\param
   socType     specifies the type of the socket
               - IFXOS_SOC_TYPE_STREAM: TCP/IP socket
               - IFXOS_SOC_TYPE_DGRAM:  UDP/IP socket
\param
   pSocketFd   specifies the pointer where the value of the socket should be
               set. Value will be greater or equal zero

\return
   - IFX_SUCCESS in case of success
   - IFX_ERROR   if operation failed
*/
IFX_int_t IFXOS_SocketCreateIpV6(
	IFXOS_socketType_t socType,
	IFXOS_socket_t *pSocketFd);

/**
   Receives data from a IP V6 UDP socket.

\param
   socFd         specifies the socket. Value has to be greater or equal zero
\param
   pBuffer       specifies the pointer to a buffer where the data will be copied
\param
   bufSize_byte  specifies the size in byte of the buffer 'pBuffer'
\param
   pSocAddr6     specifies a pointer to the IFXOS_sockAddr6_t structure

\return
   Returns the number of received bytes. Returns a negative value if an error
   occured
*/
IFX_int_t IFXOS_SocketRecvFromIpV6(
	IFXOS_socket_t socFd,
	IFX_char_t *pBuffer,
	IFX_int_t bufSize_byte,
	IFXOS_sockAddr6_t *pSocAddr6);

/**
   Sends data to IP V6 UDP socket.

\param
   socFd          specifies the socket. Value has to be greater or equal zero
\param
   pBuffer        specifies the pointer to a buffer where the data will be copied
\param
   bufSize_byte   specifies the size in byte of the buffer 'pBuffer'
\param
   pSocAddr6      specifies a pointer to the IFXOS_sockAddr6_t structure

\return
   Returns the number of sent bytes. Returns a negative value if an error
   occured
*/
IFX_int_t IFXOS_SocketSendToIpV6(
	IFXOS_socket_t socFd,
	IFX_char_t *pBuffer,
	IFX_int_t bufSize_byte,
	IFXOS_sockAddr6_t *pSocAddr6);

/**
   Assignes a local address to a IP V6 socket.

\param
   socFd       specifies the socket should be bind to the address
               Value has to be greater or equal zero
\param
   pSocAddr6   specifies a pointer to the IFXOS_sockAddr6_t structure

\return
   - IFX_SUCCESS in case of success
   - IFX_ERROR if operation failed
*/
IFX_int_t IFXOS_SocketBindIpV6(
	IFXOS_socket_t socFd,
	IFXOS_sockAddr6_t *pSocAddr6);

/** @} */

#ifdef __cplusplus
}
#endif

#endif      /* #ifndef _IFXOS_SOCKET_IPV6_H */

