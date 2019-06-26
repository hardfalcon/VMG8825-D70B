/******************************************************************************

                              Copyright (c) 2009
                            Lantiq Deutschland GmbH
                     Am Campeon 3; 85579 Neubiberg, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#ifdef GENERIC_OS

/** \file
   This file contains the IFXOS Layer implementation for GENERIC_OS User
   Socket.
*/

/* ============================================================================
   IFX GENERIC_OS adaptation - Global Includes
   ========================================================================= */
/*
   Customer-ToDo:
   Include your customer OS header required for the implementation.
*/

#include "ifx_types.h"
#include "ifxos_rt_if_check.h"
#include "ifxos_socket.h"
#include "ifxos_socket_ipv6.h"
#include "ifxos_debug.h"


/* ============================================================================
   IFX GENERIC_OS adaptation - User Space, Socket
   ========================================================================= */

/** \addtogroup IFXOS_SOCKET_GENERIC_OS_IPV6
@{ */

#if ( defined(IFXOS_HAVE_SOCKET) && (IFXOS_HAVE_SOCKET == 1) )

/**
   GENERIC_OS - This function creates a IP V6 socket.

\par Implementation
   - Create a AF_INET socket, no specified protocol.

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
	/*
	   Customer-ToDo:
	   Fill with your customer OS implementation - like
	   *pSocketFd = socket(AF_INET6, socType, 0);
	*/
	IFXOS_RETURN_IF_POINTER_NULL(pSocketFd, IFX_ERROR);

	return IFX_SUCCESS;
}

/**
   GENERIC_OS - Receives data from a UDP IP V6 socket.

\par Implementation
   -  via "recv_from"

\param
   socFd          specifies the socket. Value has to be greater or equal zero
\param
   pBuffer        specifies the pointer to a buffer where the data will be
                  copied
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
	IFX_int_t ret = 0;
	/*
	   Customer-ToDo:
	   Fill with your customer OS implementation - like

	int pFromlen = sizeof(IFXOS_sockAddr6_t);
	ret = (IFX_int_t)recvfrom((SOCKET)socFd, (char*)pBuffer, (int)bufSize_byte, 0, pSocAddr6, &pFromlen);
	*/

	IFXOS_RETURN_IF_POINTER_NULL(pBuffer, IFX_ERROR);
	IFXOS_RETURN_IF_ARG_LE_ZERO(bufSize_byte, IFX_ERROR);

	return ret;
}

/**
   GENERIC_OS Application - Sends data to UDP IP V6 socket.

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
	IFX_char_t     *pBuffer,
	IFX_int_t      bufSize_byte,
	IFXOS_sockAddr6_t  *pSocAddr6)
{
	IFX_int_t ret = 0;

	/*
	   Customer-ToDo:
	   Fill with your customer OS implementation - like
	   ret = (IFX_int_t)sendto(
	           (SOCKET)socFd, (const char*)pBuffer, (int)bufSize_byte,
	           0, pSocAddr6, sizeof(IFXOS_sockAddr6_t));
	*/
	IFXOS_RETURN_IF_POINTER_NULL(pBuffer, IFX_ERROR);
	IFXOS_RETURN_IF_ARG_LE_ZERO(bufSize_byte, IFX_ERROR);

	return ret;
}

/**
   GENERIC_OS - Assignes a local address to a IP V6 socket.

\par Implementation
   -  via "bind"

\param
   socFd       specifies the socket should be bind to the address
               Value has to be greater or equal zero
\param
   pSocAddr6    specifies a pointer to the IFXOS_sockAddr6_t structure

\return
   - IFX_SUCCESS in case of success
   - IFX_ERROR if operation failed
*/
IFX_int_t IFXOS_SocketBindIpV6(
	IFXOS_socket_t socFd,
	IFXOS_sockAddr6_t *pSocAddr6)
{
	IFX_int_t ret = 0;

	/*
	   Customer-ToDo:
	   Fill with your customer OS implementation - like
	   ret = bind(socFd, pSocAddr6, sizeof(struct sockaddr_in));
	*/
	IFXOS_RETURN_IF_POINTER_NULL(pSocAddr6, IFX_ERROR);

	return ret;
}

#endif   /* #if ( defined(IFXOS_HAVE_SOCKET) && (IFXOS_HAVE_SOCKET == 1) ) */


/** @} */

#endif      /* #ifdef GENERIC_OS */



