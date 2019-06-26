/******************************************************************************

                               Copyright (c) 2014
                            Lantiq Deutschland GmbH

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#ifdef NUCLEUS_PLUS

/** \file
   This file contains the IFXOS Layer implemantation for Nucleus User
   IP V6 Socket.
*/

/* ============================================================================
   IFX Nucleus adaptation - Global Includes
   ========================================================================= */
#include <nucleus.h>

#include "ifx_types.h"
#include "ifxos_rt_if_check.h"
#include "ifxos_socket.h"
#include "ifxos_socket_ipv6.h"
#include "ifxos_debug.h"


/* ============================================================================
   IFX Nucleus adaptation - User Space, Socket
   ========================================================================= */

/** \addtogroup IFXOS_SOCKET_NUCLEUS_IPV6
@{ */

#if ( defined(IFXOS_HAVE_SOCKET) && (IFXOS_HAVE_SOCKET == 1) )

/* ToDo: Feature still not implemented tested */
#define IFXOS_NUCLEUS_IPV6_TODO		1

/**
   Nucleus - This function creates a IP V6 socket.

\par Implementation
   - Create a AF_INET6 socket, no specified protocol.

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
	IFXOS_socket_t     *pSocketFd)
{
#if defined(HAVE_IFXOS_IPV6_SUPPORT) && (HAVE_IFXOS_IPV6_SUPPORT == 1)
#	if (IFXOS_NUCLEUS_IPV6_TODO == 0)
	/* arg3 = 0: do not specifiy the protocol */
	if((*pSocketFd = socket(AF_INET6, socType, 0)) == -1)
		{return IFX_ERROR;}

	return IFX_SUCCESS;
#	else
	/* not implemented / tested */
	return IFX_ERROR;
#	endif
#else
	/* not built-in */
	return IFX_ERROR;
#endif
}

/**
   Nucleus - Receives data from a UDP IP V6 socket.

\par Implementation
   -  via "recv_from"

\param
   socFd          specifies the socket. Value has to be greater or equal zero
\param
   pBuffer        specifies the pointer to a buffer where the data will be copied
\param
   bufSize_byte   specifies the size in byte of the buffer 'pBuffer'
\param
   pSocAddr6    specifies a pointer to the IFXOS_sockAddr6_t structure

\return
   Returns the number of received bytes. Returns a negative value if an error
   occured
*/
IFX_int_t IFXOS_SocketRecvFromIpV6(
	IFXOS_socket_t socFd,
	IFX_char_t *pBuffer,
	IFX_int_t bufSize_byte,
	IFXOS_sockAddr6_t *pSocAddr6)
{
#if defined(HAVE_IFXOS_IPV6_SUPPORT) && (HAVE_IFXOS_IPV6_SUPPORT == 1)
#	if (IFXOS_NUCLEUS_IPV6_TODO == 0)
	int ret;
	int pFromlen = sizeof(IFXOS_sockAddr6_t);

	IFXOS_RETURN_IF_POINTER_NULL(pBuffer, IFX_ERROR);
	IFXOS_RETURN_IF_ARG_LE_ZERO(bufSize_byte, IFX_ERROR);

	ret = (IFX_int_t)recvfrom(
		(int)socFd, (char*)pBuffer, (int)bufSize_byte,
		0, (struct sockaddr *)pSocAddr6, &pFromlen);

	return ret;
#	else
	/* not implemented / tested */
	return IFX_ERROR;
#	endif
#else
	/* not built-in */
	return IFX_ERROR;
#endif
}

/**
   Nucleus Application - Sends data to UDP IP V6 socket.

\par Implementation
   -  via "send"

\param
   socFd          specifies the socket. Value has to be greater or equal zero
\param
   pBuffer        specifies the pointer to a buffer where the data will be copied
\param
   bufSize_byte   specifies the size in byte of the buffer 'pBuffer'
\param
   pSocAddr6    specifies a pointer to the IFXOS_sockAddr6_t structure

\return
   Returns the number of received bytes. Returns a negative value if an error
   occured
*/
IFX_int_t IFXOS_SocketSendToIpV6(
	IFXOS_socket_t socFd,
	IFX_char_t *pBuffer,
	IFX_int_t bufSize_byte,
	IFXOS_sockAddr6_t *pSocAddr6)
{
#if defined(HAVE_IFXOS_IPV6_SUPPORT) && (HAVE_IFXOS_IPV6_SUPPORT == 1)
#	if (IFXOS_NUCLEUS_IPV6_TODO == 0)
	int ret;

	IFXOS_RETURN_IF_POINTER_NULL(pBuffer, IFX_ERROR);
	IFXOS_RETURN_IF_ARG_LE_ZERO(bufSize_byte, IFX_ERROR);

	ret = (IFX_int_t)sendto(
		(int)socFd, (const char*)pBuffer,
		(int)bufSize_byte, 0, pSocAddr6, sizeof(IFXOS_sockAddr6_t));

	return ret;
#	else
	/* not implemented / tested */
	return IFX_ERROR;
#	endif
#else
	/* not built-in */
	return IFX_ERROR;
#endif
}

/**
   Nucleus - Assignes a local address to a TCP/IP, UDP/IP or raw socket.

\par Implementation
   -  via "bind"

\param
   socFd       specifies the socket should be bind to the address
               Value has to be greater or equal zero
\param
   pSocAddr6    specifies a pointer to the DSL_SockAddr_t structure

\return
   - IFX_SUCCESS in case of success
   - IFX_ERROR if operation failed
*/
IFX_int_t IFXOS_SocketBindIpV6(
	IFXOS_socket_t    socFd,
	IFXOS_sockAddr6_t  *pSocAddr6)
{
#if defined(HAVE_IFXOS_IPV6_SUPPORT) && (HAVE_IFXOS_IPV6_SUPPORT == 1)
#	if (IFXOS_NUCLEUS_IPV6_TODO == 0)
	IFX_int_t ret;

	IFXOS_RETURN_IF_POINTER_NULL(pSocAddr6, IFX_ERROR);

	ret = bind((int)socFd, (struct sockaddr*)pSocAddr6, sizeof(IFXOS_sockAddr6_t));
	if (ret != 0)
		{return IFX_ERROR;}

	return IFX_SUCCESS;
#	else
	/* not implemented / tested */
	return IFX_ERROR;
#	endif
#else
	/* not built-in */
	return IFX_ERROR;
#endif
}

#endif   /* #if ( defined(IFXOS_HAVE_SOCKET) && (IFXOS_HAVE_SOCKET == 1) ) */

/** @} */

#endif      /* #ifdef NUCLEUS_PLUS */



