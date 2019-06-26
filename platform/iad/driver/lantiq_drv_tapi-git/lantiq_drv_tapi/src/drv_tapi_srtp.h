#ifndef _DRV_TAPI_SRTP_H
#define _DRV_TAPI_SRTP_H
/******************************************************************************

                              Copyright (c) 2014
                            Lantiq Deutschland GmbH

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/**
   \file drv_tapi_srtp.h
   This file contains the declaration of the SRTP.
*/

#ifdef TAPI_FEAT_SRTP
/* ========================================================================== */
/*                               Includes                                     */
/* ========================================================================== */
#ifdef TAPI_VERSION3
#include "srtp.h"
#endif

/* ========================================================================== */
/*                               Configuration                                */
/* ========================================================================== */


/* ========================================================================== */
/*                             Macro definitions                              */
/* ========================================================================== */


/* ========================================================================== */
/*                             Type definitions                               */
/* ========================================================================== */

#ifdef TAPI_VERSION3

#define TAPI_SRTP_MAX_KEY_LEN 64
#define TAPI_SRTP_MAX_KEYS 4
/* Allowed minimum lifetime */
#define TAPI_SRTP_LIFETIME_MIN 1024
/* Derivation packets number thresholds */
#define TAPI_SRTP_DERIVATION_MAX 24
#define TAPI_SRTP_DERIVATION_MIN 10

/* default 160 bits per RFC 3711 */
#define TAPI_SRTP_AUT_KEY_LEN_DEFAULT 20

/** SRTP Keys configuration.  */
typedef struct
{
   /* Key with salt */
   IFX_uint8_t key[TAPI_SRTP_MAX_KEY_LEN];
   /* IFX_TRUE if key was configured */
   IFX_boolean_t bKeyConfigured;
   /* IFX_TRUE if salt was configured */
   IFX_boolean_t bSaltConfigured;
} IFX_TAPI_SRTP_DATA_KEY_t;

/** used in the TAPI_CHANNEL for SRTP configuration.  */
typedef struct
{
   /* SRTP library session context for stream from COD to Ethernet */
   srtp_t rx_session;
   /* SRTP library session context for stream from Ethernet to COD */
   srtp_t tx_session;
   /* SRTP library Keys/session configuration */
   srtp_policy_t policy_rx;
   srtp_policy_t policy_tx;
   /* IFX_TRUE means SRTP is enabled and
    * packets will be encrypted and decrypted */
   IFX_boolean_t bSrtpEnabled;
   IFX_boolean_t bSrtcpEnabled;
   /* Current master key index selector for RTP downstream. */
   IFX_uint32_t  nMKI_RtpRx;
   IFX_uint32_t  nMKI_RtcpRx;
   /* Current master key index selector for RTP upstream.*/
   IFX_uint32_t  nMKI_RtpTx;
   IFX_uint32_t  nMKI_RtcpTx;
   /* Stored keys configuration */
   IFX_TAPI_SRTP_DATA_KEY_t MKI_Data[TAPI_SRTP_MAX_KEYS];
   /* IFX_TRUE if event IFX_TAPI_EVENT_SRTP_DISCARD was enabled and acknowledged */
   IFX_boolean_t bEvDiscardEn;
   /* IFX_TRUE if event IFX_TAPI_EVENT_SRTP_LIFE_TIME was enabled and acknowledged */
   IFX_boolean_t bEvLifetimeEn;
   /* Statistics data */
   IFX_TAPI_PKT_SRTP_STATISTICS_GET_t  TapiSrtpData;
} IFX_TAPI_SRTP_DATA_t;

IFX_int32_t IFX_TAPI_SRTP_EventsCfg (TAPI_CHANNEL *pChannel, IFX_TAPI_EVENT_t *pEvent,
                                     IFX_uint32_t const value);
#endif /* TAPI_VERSION3 */
#endif /* TAPI_FEAT_SRTP */
#endif /* _DRV_TAPI_SRTP_H */
