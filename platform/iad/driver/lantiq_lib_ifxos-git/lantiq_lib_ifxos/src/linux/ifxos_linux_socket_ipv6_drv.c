/******************************************************************************

                               Copyright (c) 2014
                            Lantiq Deutschland GmbH

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/
#ifdef LINUX
#ifdef __KERNEL__

/** \file
   This file contains the IFXOS Layer implementation for LINUX Kernel
   Socket IP V6.
*/

/* ============================================================================
   IFX Linux adaptation - Global Includes - Kernel
   ========================================================================= */
#include <linux/kernel.h>
#ifdef MODULE
   #include <linux/module.h>
#endif
#include <linux/version.h>
#include <linux/in.h>
#include <linux/net.h>
#include <asm/uaccess.h>

#include "ifx_types.h"
#include "ifxos_rt_if_check.h"
#include "ifxos_socket.h"
#include "ifxos_socket_ipv6.h"
#include "ifxos_debug.h"

/* ============================================================================
   IFX Linux adaptation - Kernel Space, Socket
   ========================================================================= */

/** \addtogroup IFXOS_SOCKET_LINUX_IPV6
@{ */

#if (defined(IFXOS_HAVE_SOCKET) && (IFXOS_HAVE_SOCKET == 1))

/**
   LINUX Kernel - This function creates a TCP/IP, UDP/IP or raw IPv6 socket.

\par Implementation
   - Create a PF_INET6 socket, no specified protocol.

\param
   socType    specifies the type of the socket
              - IFXOS_SOC_TYPE_STREAM: TCP/IP socket
              - IFXOS_SOC_TYPE_DGRAM:  UDP/IP socket
\param
   pSocketFd  specifies the pointer where the value of the socket should be
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
   if (sock_create(PF_INET6, socType, 0, (struct socket **)pSocketFd) == -1)
      {return IFX_ERROR;}

   return IFX_SUCCESS;
#else
   return IFX_ERROR;
#endif
}

/**
   LINUX Kernel - Receives data from a UDP IP V6 socket.

\attention
   Still not implemented

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
   struct msghdr msg;
   struct iovec iov;
   mm_segment_t old_fs;
   int ret;

   IFXOS_RETURN_IF_POINTER_NULL(pBuffer, IFX_ERROR);
   IFXOS_RETURN_IF_ARG_LE_ZERO(bufSize_byte, IFX_ERROR);

   iov.iov_base = pBuffer;
   iov.iov_len = (unsigned int) bufSize_byte;

   msg.msg_name = (void *) pSocAddr6;
   msg.msg_namelen = sizeof(IFXOS_sockAddr6_t);
   msg.msg_control = IFX_NULL;
   msg.msg_controllen = 0;
   msg.msg_flags = 0;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,19,0))
   msg.msg_iov = &iov;
   msg.msg_iovlen = 1;
#else
   iov_iter_init(&msg.msg_iter, READ, &iov, 1, bufSize_byte);
#endif

   /* Modify address limitation which is used if user space is calling
      kernel space, otherwise sock_recvmsg() will fail.*/
   old_fs = get_fs();
   set_fs(KERNEL_DS);

   ret = sock_recvmsg ((struct socket *) socFd, &msg, bufSize_byte, 0);
   set_fs(old_fs);

   return ret;
#else
   return IFX_ERROR;
#endif
}

/**
   LINUX Kernel - Sends data to UDP IP V6 socket.

\par Implementation
   -  via "sock_sendmsg"

\param
   socFd        specifies the socket. Value has to be greater or equal zero
\param
   pBuffer      specifies the pointer to a buffer where the data will be copied
\param
   bufSize_byte specifies the size in byte of the buffer 'pBuffer'
\param
   pSocAddr     specifies a pointer to the IFXOS_sockAddr_t structure

\return
   Returns the number of sent bytes. Returns a negative value if an error
   occured
*/
IFX_int_t IFXOS_SocketSendToIpV6(
   IFXOS_socket_t socFd,
   IFX_char_t *pBuffer,
   IFX_int_t bufSize_byte,
   IFXOS_sockAddr6_t *pSocAddr)
   {
#if defined(HAVE_IFXOS_IPV6_SUPPORT) && (HAVE_IFXOS_IPV6_SUPPORT == 1)
   struct msghdr msg;
   struct iovec iov;
   mm_segment_t old_fs;
   int ret;

   IFXOS_RETURN_IF_POINTER_NULL(pBuffer, IFX_ERROR);
   IFXOS_RETURN_IF_ARG_LE_ZERO(bufSize_byte, IFX_ERROR);

   iov.iov_base = pBuffer;
   iov.iov_len = (unsigned int) bufSize_byte;

   msg.msg_name = (void *) pSocAddr;
   msg.msg_namelen = sizeof(IFXOS_sockAddr6_t);
   msg.msg_control = IFX_NULL;
   msg.msg_controllen = 0;
   msg.msg_flags = MSG_DONTWAIT;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,19,0))
   msg.msg_iov = &iov;
   msg.msg_iovlen = 1;
#else
   iov_iter_init(&msg.msg_iter, WRITE, &iov, 1, bufSize_byte);
#endif

   /* Modify address limitation which is used if user space is calling
   kernel space, otherwise sock_sendmsg() will fail.*/
   old_fs = get_fs();
   set_fs(KERNEL_DS);
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,19,0)
   ret = sock_sendmsg((struct socket *) socFd, &msg, bufSize_byte);
#else
   ret = sock_sendmsg((struct socket *) socFd, &msg);
#endif
   set_fs(old_fs);

   return ret;
#else
   return IFX_ERROR;
#endif
}

/**
   LINUX Kernel - Assignes a local address to a TCP/IPv6, UDP/IPv6 or raw socket.

\par Implementation
   -  via "bind"

\param
   socFd     specifies the socket should be bind to the address
             Value has to be greater or equal zero
\param
   pSocAddr  specifies a pointer to the IFXOS_sockAddr6_t structure

\return
   - IFX_SUCCESS in case of success
   - IFX_ERROR if operation failed
*/
IFX_int_t IFXOS_SocketBindIpV6(
   IFXOS_socket_t socFd,
   IFXOS_sockAddr6_t *pSocAddr)
{
#if defined(HAVE_IFXOS_IPV6_SUPPORT) && (HAVE_IFXOS_IPV6_SUPPORT == 1)
   IFX_int_t ret;

   IFXOS_RETURN_IF_POINTER_NULL(pSocAddr, IFX_ERROR);

   ret = ((struct socket *)socFd)->ops->bind(
         (struct socket *)socFd,
         (struct sockaddr*) pSocAddr,
         sizeof(IFXOS_sockAddr6_t));
   if (ret != 0)
      {return IFX_ERROR;}

   return IFX_SUCCESS;
#else
   return IFX_ERROR;
#endif
}

#endif	/* #if (defined(IFXOS_HAVE_SOCKET) && (IFXOS_HAVE_SOCKET == 1)) */

/** @} */

#ifdef MODULE
#ifdef HAVE_IFXOS_IPV6_SUPPORT
EXPORT_SYMBOL(IFXOS_SocketCreateIpV6);
EXPORT_SYMBOL(IFXOS_SocketSendToIpV6);
EXPORT_SYMBOL(IFXOS_SocketRecvFromIpV6);
EXPORT_SYMBOL(IFXOS_SocketBindIpV6);
#endif
#endif

#endif      /* #ifdef __KERNEL__ */
#endif      /* #ifdef LINUX */

