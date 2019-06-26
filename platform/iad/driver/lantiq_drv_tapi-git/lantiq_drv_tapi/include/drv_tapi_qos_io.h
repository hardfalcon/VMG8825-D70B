#ifndef _DRV_TAPI_QOS_IO_H
#define _DRV_TAPI_QOS_IO_H
/******************************************************************************

                              Copyright (c) 2014
                            Lantiq Deutschland GmbH

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

*******************************************************************************/

/**
   \file drv_tapi_qos_io.h  TAPI interface to the QOS driver.

   \remarks Define QOS_SUPPORT to compile this service.
*/

/** \addtogroup TAPI_INTERFACE */
/*@{*/

/** \defgroup TAPI_INTERFACE_QOS QOS Services
    The QOS service is a direct connection from the KPI interface to the
    network stack of the operating system. All QOS services apply to channel
    file descriptors. */
/*@{*/


/* ========================================================================== */
/*                     TAPI Interface Ioctl Commands                          */
/* ========================================================================== */

/* magic number for QOS ioctls */
#define QOS_IOC_MAGIC 's'

/* include automatically generated TAPI QOS ioctl command indexes */
#include "drv_tapi_qos_io_indexes.h"

/* ======================================================================== */
/* TAPI QOS Services, ioctl commands (Group TAPI_INTERFACE_QOS)             */
/* ======================================================================== */

/** This service creates a new session on a channel.

   \param Pointer to a \ref QOS_INIT_SESSION structure.

   \return Returns value as follows:
      - IFX_SUCCESS: if successful
      - IFX_ERROR: in case of an error
*/
#define  FIO_QOS_START                       _IOW (QOS_IOC_MAGIC, FIO_QOS_START_IDX, QOS_INIT_SESSION)

/** This service creates a new session on a channel.
    The application layer has to create and bind the socket file descriptor
    to a local address and connect it to a remote address before providing
    it to FIO_QOS_ON_SOCKET_START. The application must not use the socket
    for packet reception and transmission after FIO_QOS_ON_SOCKET_START.
    The application must call FIO_QOS_STOP when the call is terminated and
    only after this is allowed to release the socket file descriptor.

   \param QOS_INIT_SESSION_ON_SOCKET* The parameter points to a
          \ref QOS_INIT_SESSION_ON_SOCKET structure.

   \return Returns value as follows:
      - IFX_SUCCESS: if successful
      - IFX_ERROR: in case of an error
*/
#define  FIO_QOS_ON_SOCKET_START             _IOW(QOS_IOC_MAGIC, FIO_QOS_ON_SOCKET_START_IDX, QOS_INIT_SESSION_ON_SOCKET)

/*  The FIO_QOS_ACTIVATE IOCTL is obsolete and included only for compatibility reasons. It always return IFX_SUCCESS.
    FIO_QOS_START now implicitly includes this functionality.

   \param int This interface expects no parameter. It should be set to 0.

   \return Returns value as follows:
      - IFX_SUCCESS: in all cases
*/
#define  FIO_QOS_ACTIVATE                    _IO (QOS_IOC_MAGIC, FIO_QOS_ACTIVATE_IDX)

/** This service deactivates and deletes a session on a channel.
    The parameter is ignored, the port number is no longer needed to
    stop a session.

   \param int This interface expects no parameter. It should be set to 0.

   \return Returns value as follows:
      - IFX_SUCCESS: if successful
      - IFX_ERROR: in case of an error
*/
#ifdef TAPI_ONE_DEVNODE
   #define  FIO_QOS_STOP                     _IOW (QOS_IOC_MAGIC, FIO_QOS_STOP_IDX, QOS_STOP_SESSION)
#else /* TAPI_ONE_DEVNODE */
   #define  FIO_QOS_STOP                     _IO (QOS_IOC_MAGIC, FIO_QOS_STOP_IDX)
#endif /* TAPI_ONE_DEVNODE */

/** This service stops all QOS support. Regardless on which channel file
    descriptor it is given, it deactivates and deletes all sessions on all
    channels.

   \param int This interface expects no parameter. It should be set to 0.

   \return Returns value as follows:
      - IFX_SUCCESS: if successful
      - IFX_ERROR: in case of an error
*/
#define  FIO_QOS_CLEAN                       _IO (QOS_IOC_MAGIC, FIO_QOS_CLEAN_IDX)


/* ========================================================================= */
/*                      TAPI Interface Enumerations                          */
/* ========================================================================= */

/* ======================================================================== */
/* TAPI QOS Services, enumerations (Group TAPI_INTERFACE_QOS)               */
/* ======================================================================== */

/* This define is now obsolete. It is kept for backward compatibility reasons only. */
#define QOS_PORT_CLEAN     0xFFFF /* or 65535 */


/* ========================================================================== */
/*                      TAPI Interface Structures                             */
/* ========================================================================== */

/* ======================================================================== */
/* TAPI QOS Services, structures (Group TAPI_INTERFACE_QOS)                 */
/* ======================================================================== */

/** TAPI QOS session creation structure used for \ref FIO_QOS_START. */
typedef struct
{
#ifdef TAPI_ONE_DEVNODE
   /** Device index */
   IFX_uint16_t dev;
   /** Channel 'module' index */
   IFX_uint16_t ch;
#endif /* TAPI_ONE_DEVNODE */
   /* AF_INET or AF_INET6*/
   IFX_uint16_t family;
   union
   {
       struct
       {
           /** Source (local) ip address. */
           IFX_uint32_t srcAddr;
           /** Destination (remore) ip address. */
           IFX_uint32_t destAddr;
       } ipv4;
       struct
       {
           /** Source (local) ip address. */
           IFX_uint8_t srcAddr[16];
           /** Destination (remore) ip address. */
           IFX_uint8_t destAddr[16];
       }ipv6;
   };
   /** Source (local) port number. */
   IFX_uint16_t srcPort;
   /** Destination (remote) port number. */
   IFX_uint16_t destPort;
   IFX_boolean_t do_srtp;
   IFX_uint32_t ssrc;

   IFX_TAPI_ALGS_ENCR_t eSRTP;
   IFX_TAPI_ALGS_AUTH_t eSRTP_Auth;
   IFX_uint32_t nSRTP_AuthFieldLength;

   IFX_uint8_t localKey[IFX_TAPI_SRTP_MKI_KEY_MAX];
   IFX_uint8_t localSalt[IFX_TAPI_SRTP_MKI_SALT_MAX];
   IFX_uint8_t remoteKey[IFX_TAPI_SRTP_MKI_KEY_MAX];
   IFX_uint8_t remoteSalt[IFX_TAPI_SRTP_MKI_SALT_MAX];
} QOS_INIT_SESSION;

/** TAPI QOS session creation structure used for \ref FIO_QOS_ON_SOCKET_START.*/
typedef struct {
#ifdef TAPI_ONE_DEVNODE
   /** Device index */
   IFX_uint16_t dev;
   /** Channel 'module' index */
   IFX_uint16_t ch;
#endif /* TAPI_ONE_DEVNODE */
   /** File descriptor of the socket to be used for sending data.
       The socket must already be bound to a local address, connected to a
       remote address and ready for sending before using it in the IOCTL. */
   int fd;
} QOS_INIT_SESSION_ON_SOCKET;

#ifdef TAPI_ONE_DEVNODE
/** TAPI QOS session termination structure used for \ref FIO_QOS_STOP. */
typedef struct {
   /** Device index */
   IFX_uint16_t dev;
   /** Channel 'module' index */
   IFX_uint16_t ch;
} QOS_STOP_SESSION;
#endif /* TAPI_ONE_DEVNODE */


/*@}*/ /* TAPI_INTERFACE_QOS */
/*@}*/ /* TAPI_INTERFACE */

#endif /* _DRV_TAPI_QOS_IO_H */
