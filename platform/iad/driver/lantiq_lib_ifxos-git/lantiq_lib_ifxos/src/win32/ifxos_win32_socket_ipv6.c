/******************************************************************************

                               Copyright (c) 2014
                            Lantiq Deutschland GmbH

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#if defined(WIN32) && !defined(NUCLEUS_PLUS)

/** \file
   This file contains the IFXOS Layer implementation for Win32 User
   Socket IP V6.
*/

/* ============================================================================
   IFX Win32 adaptation - Global Includes
   ========================================================================= */
/*
#include <winsock2.h>
*/
#include "ifx_types.h"
#include "ifxos_rt_if_check.h"
#include "ifxos_socket.h"
#include "ifxos_socket_ipv6.h"
#include "ifxos_debug.h"


/* ============================================================================
   IFX Win32 adaptation - User Space, Socket
   ========================================================================= */

/** \addtogroup IFXOS_SOCKET_WIN32_IPV6
@{ */

#if ( defined(IFXOS_HAVE_SOCKET) && (IFXOS_HAVE_SOCKET == 1) )

/**
   Win32 - This function creates a IP V6 socket.

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
	IFXOS_socket_t *pSocketFd)
{
#if defined(HAVE_IFXOS_IPV6_SUPPORT) && (HAVE_IFXOS_IPV6_SUPPORT == 1)
	IFXOS_RETURN_IF_POINTER_NULL(pSocketFd, IFX_ERROR);

	/* arg3 = 0: do not specifiy the protocol */
	if((*pSocketFd = socket(AF_INET6, socType, 0)) == INVALID_SOCKET)
		{return IFX_ERROR;}

	return IFX_SUCCESS;
#else
	/* not built-in */
	return IFX_ERROR;
#endif
}

/**
   Win32 - Receives data from a datagramm socket IP V6.

\par Implementation
   -  via "recv_from"

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
	IFXOS_sockAddr6_t *pSocAddr6)
{
#if defined(HAVE_IFXOS_IPV6_SUPPORT) && (HAVE_IFXOS_IPV6_SUPPORT == 1)
	int ret;
	int pFromlen = sizeof(IFXOS_sockAddr6_t);

	IFXOS_RETURN_IF_POINTER_NULL(pBuffer, IFX_ERROR);
	IFXOS_RETURN_IF_ARG_LE_ZERO(bufSize_byte, IFX_ERROR);

	ret = (IFX_int_t)recvfrom(
		(SOCKET)socFd, (char*)pBuffer, (int)bufSize_byte,
		0, (struct sockaddr *)pSocAddr6, &pFromlen);

	return ret;
#else
	/* not built-in */
	return IFX_ERROR;
#endif
}

/**
   Win32 - Sends data to UDP socket IP V6.

\par Implementation
   -  via "send"

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
IFX_int_t IFXOS_SocketSendToIpV6(
	IFXOS_socket_t socFd,
	IFX_char_t *pBuffer,
	IFX_int_t bufSize_byte,
	IFXOS_sockAddr6_t *pSocAddr6)
{
#if defined(HAVE_IFXOS_IPV6_SUPPORT) && (HAVE_IFXOS_IPV6_SUPPORT == 1)
	int ret;

	IFXOS_RETURN_IF_POINTER_NULL(pBuffer, IFX_ERROR);
	IFXOS_RETURN_IF_ARG_LE_ZERO(bufSize_byte, IFX_ERROR);

	ret = (IFX_int_t)sendto(
		(SOCKET)socFd, (const char*)pBuffer, (int)bufSize_byte,
		0, (const struct sockaddr*)pSocAddr6, sizeof(IFXOS_sockAddr6_t));

	return ret;
#else
	/* not built-in */
	return IFX_ERROR;
#endif
}

/**
   Win32 - Assignes a local address to a socket IP V6.

\par Implementation
   -  via "bind"

\param
   socFd      specifies the socket should be bind to the address
              Value has to be greater or equal zero
\param
   pSocAddr6  specifies a pointer to the IFXOS_sockAddr_t structure

\return
   - IFX_SUCCESS in case of success
   - IFX_ERROR if operation failed
*/
IFX_int_t IFXOS_SocketBindIpV6(
	IFXOS_socket_t    socFd,
	IFXOS_sockAddr6_t  *pSocAddr6)
{
#if defined(HAVE_IFXOS_IPV6_SUPPORT) && (HAVE_IFXOS_IPV6_SUPPORT == 1)
	IFX_int_t ret;

	IFXOS_RETURN_IF_POINTER_NULL(pSocAddr6, IFX_ERROR);

	ret = bind((SOCKET)socFd, (struct sockaddr*)pSocAddr6, sizeof(IFXOS_sockAddr6_t));
	if (ret != 0)
	{
		ret = WSAGetLastError();
		if (ret == WSAEADDRINUSE)
			{return IFX_SUCCESS;}

		return IFX_ERROR;
	}

	return IFX_SUCCESS;
#else
	/* not built-in */
	return IFX_ERROR;
#endif
}

#endif      /* #if ( defined(IFXOS_HAVE_SOCKET) && (IFXOS_HAVE_SOCKET == 1) ) */

/** @} */

#endif      /* #ifdef WIN32 */

