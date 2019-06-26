#ifndef _DRV_TAPI_QOS_LL_INTERFACE_H
#define _DRV_TAPI_QOS_LL_INTERFACE_H
/******************************************************************************

                              Copyright (c) 2014
                            Lantiq Deutschland GmbH

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

*******************************************************************************/

/**
   \file drv_tapi_qos_ll_interface.h
   TAPI LL interface to the UDP redirect driver which enables QOS.
*/

#include <ifx_types.h>
#include "drv_tapi_kpi_io.h"


/** Interface version number (major, minor, build number, package type) */
#define DRV_QOS_INTERFACE_VER_STR  "1.3.1.0"


typedef struct {
   /** interface version, keep as first element */
   IFX_char_t                 *InterfaceVersion;
   /** driver name */
   IFX_char_t                 *drvName;

   /**
   Function that creates a UDP socket and attaches it to the given KPI channel.

   \param  kpiCh        Channel number.
   \param  srcAddr      Source (local) IP address.
   \param  srcPort      Source (local) port number.
   \param  dstAddr      Destination (remote) IP address.
   \param  dstPort      Destination (remote) port number.

   \return
      - CALL_MK_SESSION_ERR
      - NO_ERROR
   */
   IFX_int32_t  (*start)      (IFX_TAPI_KPI_CH_t kpiCh,
                               IFX_uint16_t family,
                               IFX_void_t* srcAddr, IFX_uint16_t srcPort,
                               IFX_void_t* dstAddr, IFX_uint16_t dstPort,
                               IFX_boolean_t do_srtp,
                               IFX_uint32_t ssrc,
                               IFX_TAPI_ALGS_ENCR_t eSRTP,
                               IFX_TAPI_ALGS_AUTH_t eSRTP_Auth,
                               IFX_uint32_t nSRTP_AuthFieldLength,
                               const IFX_uint8_t * localKey,
                               const IFX_uint8_t * localSalt,
                               const IFX_uint8_t * remoteKey,
                               const IFX_uint8_t * remoteSalt);

   /**
   Function that attaches a given UDP socket to the given KPI channel.

   \param  kpiCh        Channel number.
   \param  fd           Socket file descriptor.

   \return
      - CALL_MK_SESSION_ERR
      - NO_ERROR
   */
   IFX_int32_t  (*start_fd)   (IFX_TAPI_KPI_CH_t kpiCh,
                               int fd);

   /**
   Function that deletes the one session on the given channel.
   Deleting a session means deactivating it and deleting the socket.

   \param  channel      Channel number.

   \return
      - CHANNEL_NO_ERR
      - NO_ERROR
   */
   IFX_int32_t  (*stop)       (IFX_TAPI_KPI_CH_t kpiCh);

   /**
   Function that stops the entire qos support. It deactivates and deletes all
   sessions on all channels.

   \return
      - NO_ERROR
   */
   IFX_int32_t  (*clean)      (void);

   /**
   Find a free channel and return it's index

   \return
      - index of free channel found
      - -1 if no free channel found
   */
   IFX_int16_t  (*getFreeChannel) (void);

   /**
   Pointer to an egress struct tasklet_struct (Linux), optional
   */
   IFX_void_t                 *pQosEgressTasklet;
} IFX_TAPI_DRV_CTX_QOS_t;


extern IFX_return_t IFX_TAPI_QOS_DrvRegister(IFX_TAPI_DRV_CTX_QOS_t *pQosCtx);

#endif /* _DRV_TAPI_QOS_LL_INTERFACE_H */
