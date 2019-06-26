/*******************************************************************************

                              Copyright (c) 2014
                            Lantiq Deutschland GmbH

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

*******************************************************************************/

/**
   \file drv_tapi_kpi.c
   This file contains the implementation of the SRTP.
*/

/* ========================================================================== */
/*                                 Includes                                   */
/* ========================================================================== */
#include "drv_tapi.h"

#ifdef TAPI_FEAT_SRTP

#ifdef TAPI_VERSION3
#include "rtp.h"
#endif /* TAPI_VERSION3 */

/* ========================================================================== */
/*                             Macro definitions                              */
/* ========================================================================== */

/* ========================================================================== */
/*                             Type definitions                               */
/* ========================================================================== */


/* ========================================================================== */
/*                             Global variables                               */
/* ========================================================================== */

#ifdef TAPI_VERSION3
IFX_boolean_t bSrtpLibInit = IFX_FALSE;
extern IFX_TAPI_HL_DRV_CTX_t gHLDrvCtx [TAPI_MAX_LL_DRIVERS];


/* ========================================================================== */
/*                           Function prototypes                              */
/* ========================================================================== */

/* ========================================================================== */
/*                         Function implementation                            */
/* ========================================================================== */

/**
   Event Call-back function.
   This function will accept SRTP lib events

   \param  pChannel     Handle to TAPI_CHANNEL structure.
   \param  pSrtpCfg     Configuration.

   \return TAPI_statusOk or error code
*/
IFX_void_t IFX_TAPI_SRTP_event_reporter (srtp_event_data_t *data)
{
   IFX_uint32_t iDrv, iCh, iDev;
   TAPI_CHANNEL *pChannel = IFX_NULL;
   IFX_TAPI_EVENT_t  tapiEvent;
   IFX_boolean_t bRx;

   /* Select channel and direction */
   for (iDrv=0; iDrv < TAPI_MAX_LL_DRIVERS; iDrv++)
   {
      if (IFX_NULL == gHLDrvCtx[iDrv].pDrvCtx)
         continue;

      if (IFX_NULL == gHLDrvCtx[iDrv].pDrvCtx->pTapiDev)
         continue;

      for (iDev=0; iDev < gHLDrvCtx[iDrv].pDrvCtx->maxDevs; iDev++)
      {
         for (iCh = 0; iCh < gHLDrvCtx[iDrv].pDrvCtx->pTapiDev[iDev].nMaxChannel; iCh++)
         {
            if( gHLDrvCtx[iDrv].pDrvCtx->pTapiDev[iDev].pChannel[iCh].pSrtp->rx_session == data->session)
            {
               pChannel = &gHLDrvCtx[iDrv].pDrvCtx->pTapiDev[iDev].pChannel[iCh];
               bRx = IFX_TRUE;
               break;
            }
            if( gHLDrvCtx[iDrv].pDrvCtx->pTapiDev[iDev].pChannel[iCh].pSrtp->tx_session == data->session)
            {
               bRx = IFX_FALSE;
               return;
            }
         }
      }
   }

   /* Report event */
   if (pChannel != IFX_NULL)
   {
      memset(&tapiEvent, 0, sizeof(IFX_TAPI_EVENT_t));
      tapiEvent.module = IFX_TAPI_MODULE_TYPE_NONE;
      switch (data->event)
      {
         case event_ssrc_collision:
            /* SSRC collision event not realized in TAPI interface */
            return;
         case event_key_soft_limit:
            tapiEvent.id = IFX_TAPI_EVENT_SRTP_LIFE_TIME;
            if (bRx == IFX_TRUE)
               tapiEvent.data.srtp_life_time.bSRTP_ThresRx = IFX_TRUE;
            else
               tapiEvent.data.srtp_life_time.bSRTP_ThresTx = IFX_TRUE;
           break;
         case event_key_hard_limit:
            tapiEvent.id = IFX_TAPI_EVENT_SRTP_LIFE_TIME;
            if (bRx == IFX_TRUE)
               tapiEvent.data.srtp_life_time.bSRTP_ElapsedRx = IFX_TRUE;
            else
               tapiEvent.data.srtp_life_time.bSRTP_ElapsedTx = IFX_TRUE;
           break;
         case event_packet_index_limit:
            /* This defined but not reported in library */
         default:
            return;
      }
      if (pChannel->pSrtp->bEvLifetimeEn == IFX_TRUE)
      {
         IFX_TAPI_Event_Dispatch(pChannel, &tapiEvent);
         pChannel->pSrtp->bEvLifetimeEn = IFX_FALSE;
      }

   }
}

/**
   Configure SRTP events on selected TAPI Channel.

   \param  pChannel     Handle to TAPI_CHANNEL structure.
   \param  pEvent       Event.
   \param  value        Value.

   \return TAPI_statusOk or error code
*/
IFX_int32_t IFX_TAPI_SRTP_EventsCfg (TAPI_CHANNEL *pChannel, IFX_TAPI_EVENT_t *pEvent,
                                     IFX_uint32_t const value)
{
   if (pChannel->pSrtp == IFX_NULL)
   {
      /* SRTP not initialized on this channel */
      return TAPI_statusOk;
   }

   switch(pEvent->id)
   {
   case IFX_TAPI_EVENT_SRTP_LIFE_TIME:
      if (value == 0)
         pChannel->pSrtp->bEvLifetimeEn = IFX_TRUE;
      else
         pChannel->pSrtp->bEvLifetimeEn = IFX_FALSE;
      break;
   case IFX_TAPI_EVENT_SRTP_DISCARD:
      if (value == 0)
         pChannel->pSrtp->bEvDiscardEn = IFX_TRUE;
      else
         pChannel->pSrtp->bEvDiscardEn = IFX_FALSE;
      break;
   case IFX_TAPI_EVENT_SRTP_KEY_DERIVED:
   case IFX_TAPI_EVENT_SRTP_MASTER_KEY_CHANGE:
   default:
      break;
   }
   return TAPI_statusOk;
}
#endif /* TAPI_VERSION3 */

/**
   Configure SRTP on selected TAPI Channel.
   This function will redirect call if SRTP supported in LL driver

   \param  pChannel     Handle to TAPI_CHANNEL structure.
   \param  pSrtpCfg     Configuration.

   \return TAPI_statusOk or error code
*/
IFX_int32_t IFX_TAPI_SRTP_CfgSet (TAPI_CHANNEL *pChannel, IFX_TAPI_PKT_SRTP_CFG_t const *pSrtpCfg)
{
   IFX_int32_t ret;
#ifdef TAPI_VERSION3
   err_status_t SrtpLibRet;
   IFX_boolean_t bSrtpEn = IFX_FALSE;
#endif /* TAPI_VERSION3 */
   IFX_TAPI_DRV_CTX_t *pDrvCtx = pChannel->pTapiDevice->pDevDrvCtx;

   if (pDrvCtx->COD.SRTP_CfgSet != IFX_NULL)
   {
      ret = pDrvCtx->COD.SRTP_CfgSet (pChannel->pLLChannel, pSrtpCfg);
      if (!TAPI_SUCCESS(ret))
         RETURN_STATUS (TAPI_statusLLFailed, ret);
      else
      {
         /* Return immediately, software realization skipped */
         RETURN_STATUS (TAPI_statusOk, 0);
      }
   }

#ifdef TAPI_VERSION3
   if (pChannel->pSrtp == IFX_NULL)
   {
      /* SRTP not initialized on this channel */
      RETURN_STATUS (TAPI_statusInitSrtpFail, 0);
   }
   /* Initialize SRTP library */
   if (bSrtpLibInit != IFX_TRUE)
   {
      SrtpLibRet = srtp_init();
      if (SrtpLibRet != err_status_ok)
      {
         TRACE (TAPI_DRV, DBG_LEVEL_HIGH,
               ("\nERR: SRTP library initialization failed with error code %d\n", SrtpLibRet));
         RETURN_STATUS (TAPI_statusSrtpLibErr, SrtpLibRet);
      }
      /* redirect events to TAPI function */
      srtp_install_event_handler (IFX_TAPI_SRTP_event_reporter);
      bSrtpLibInit = IFX_TRUE;
   }
   /* Disable enabled streams, in case of reconfiguration or deactivation */
   if (pChannel->pSrtp->bSrtpEnabled == IFX_TRUE)
   {
      pChannel->pSrtp->bSrtpEnabled = IFX_FALSE;
      SrtpLibRet = srtp_dealloc(pChannel->pSrtp->tx_session);
      if (SrtpLibRet != err_status_ok)
         RETURN_STATUS (TAPI_statusSrtpLibErr, SrtpLibRet);
      SrtpLibRet = srtp_dealloc(pChannel->pSrtp->rx_session);
      if (SrtpLibRet != err_status_ok)
         RETURN_STATUS (TAPI_statusSrtpLibErr, SrtpLibRet);
   }
   /* Set initial key indexes */
   if (pSrtpCfg->nMKI_RtpInitialRx >= TAPI_SRTP_MAX_KEYS ||
       /*pSrtpCfg->nMKI_RtcpInitialRx >= TAPI_SRTP_MAX_KEYS ||*/
       pSrtpCfg->nMKI_RtpInitialTx >= TAPI_SRTP_MAX_KEYS /*||
       pSrtpCfg->nMKI_RtcpInitialTx >= TAPI_SRTP_MAX_KEYS*/)
   {
      RETURN_STATUS (TAPI_statusParam, 0);
   }
   /* Check that master keys was set for all selected indexes */
   if ((pChannel->pSrtp->MKI_Data[pSrtpCfg->nMKI_RtpInitialRx].bKeyConfigured != IFX_TRUE) ||
       (pChannel->pSrtp->MKI_Data[pSrtpCfg->nMKI_RtpInitialTx].bKeyConfigured != IFX_TRUE) /* ||
       (pChannel->pSrtp->MKI_Data[pSrtpCfg->nMKI_RtcpInitialRx].bKeyConfigured != IFX_TRUE) ||
       (pChannel->pSrtp->MKI_Data[pSrtpCfg->nMKI_RtcpInitialTx].bKeyConfigured != IFX_TRUE)*/)
   {
      RETURN_STATUS (TAPI_statusBadMasterKey, 0);
   }
   pChannel->pSrtp->nMKI_RtpRx = pSrtpCfg->nMKI_RtpInitialRx;
   /* pChannel->pSrtp->nMKI_RtcpRx = pSrtpCfg->nMKI_RtcpInitialRx;*/
   pChannel->pSrtp->nMKI_RtpTx = pSrtpCfg->nMKI_RtpInitialTx;
   /*pChannel->pSrtp->nMKI_RtcpTx = pSrtpCfg->nMKI_RtcpInitialTx;*/

   /* Encryption type */
   switch (pSrtpCfg->eSRTP)
   {
      case IFX_TAPI_ENCR_AES_CTR:
         pChannel->pSrtp->policy_rx.rtp.cipher_type = AES_128_ICM;
         if (pChannel->pSrtp->MKI_Data[pSrtpCfg->nMKI_RtpInitialRx].bSaltConfigured == IFX_TRUE)
         {
            /* default 128 bits per RFC 3711 */
            pChannel->pSrtp->policy_rx.rtp.cipher_key_len = IFX_TAPI_SRTP_MKI_KEY_MAX + IFX_TAPI_SRTP_MKI_SALT_MAX;
         }
         else
         {
            pChannel->pSrtp->policy_rx.rtp.cipher_key_len = IFX_TAPI_SRTP_MKI_KEY_MAX;
         }
         /* set key */
         pChannel->pSrtp->policy_rx.key = pChannel->pSrtp->MKI_Data[pSrtpCfg->nMKI_RtpInitialRx].key;

         pChannel->pSrtp->policy_tx.rtp.cipher_type = AES_128_ICM;
         if (pChannel->pSrtp->MKI_Data[pSrtpCfg->nMKI_RtpInitialTx].bSaltConfigured == IFX_TRUE)
         {
            /* default 128 bits per RFC 3711 */
            pChannel->pSrtp->policy_tx.rtp.cipher_key_len = IFX_TAPI_SRTP_MKI_KEY_MAX + IFX_TAPI_SRTP_MKI_SALT_MAX;
         }
         else
         {
            pChannel->pSrtp->policy_tx.rtp.cipher_key_len = IFX_TAPI_SRTP_MKI_KEY_MAX;
         }
         /* Set key */
         pChannel->pSrtp->policy_tx.key = pChannel->pSrtp->MKI_Data[pSrtpCfg->nMKI_RtpInitialTx].key;
         /* Enable SRTP */
         bSrtpEn = IFX_TRUE;
         break;
      case IFX_TAPI_ENCR_NONE:
         /* TODO: check if possible authentification only ( sec_serv_auth mode )
          * if yes, bSrtpEnabled should be enabled also on IFX_TAPI_AUTH_HMAC_SHA1 */
         bSrtpEn = IFX_FALSE;
         break;
      default:
         RETURN_STATUS (TAPI_statusParam, 0);
   }

   /* SRTCP not supported */
   switch (pSrtpCfg->eSRTCP)
   {
      case IFX_TAPI_ENCR_AES_CTR:
         pChannel->pSrtp->bSrtcpEnabled = IFX_TRUE;
         RETURN_STATUS (TAPI_statusNotSupported, 0);
         break;
      case IFX_TAPI_ENCR_NONE:
         pChannel->pSrtp->bSrtcpEnabled = IFX_FALSE;
         break;
      default:
         RETURN_STATUS (TAPI_statusParam, 0);
   }

   /* Authentication type */
   switch (pSrtpCfg->eSRTP_Auth)
   {
      case IFX_TAPI_AUTH_HMAC_SHA1:
         if (pSrtpCfg->nSRTP_AuthFieldLength == 0)
         {
            RETURN_STATUS (TAPI_statusNotSupported, 0);
         }
         pChannel->pSrtp->policy_rx.rtp.auth_type = HMAC_SHA1;
         /* TODO: check, looks like we need to add this parameter */
         pChannel->pSrtp->policy_rx.rtp.auth_key_len =
               TAPI_SRTP_AUT_KEY_LEN_DEFAULT;
         pChannel->pSrtp->policy_rx.rtp.auth_tag_len =
               pSrtpCfg->nSRTP_AuthFieldLength;
         if (pSrtpCfg->eSRTP == IFX_TAPI_ENCR_AES_CTR)
            pChannel->pSrtp->policy_rx.rtp.sec_serv = sec_serv_conf_and_auth;
         else
            pChannel->pSrtp->policy_rx.rtp.sec_serv = sec_serv_auth;

         pChannel->pSrtp->policy_tx.rtp.auth_type = HMAC_SHA1;
         pChannel->pSrtp->policy_tx.rtp.auth_key_len =
               TAPI_SRTP_AUT_KEY_LEN_DEFAULT;
         pChannel->pSrtp->policy_tx.rtp.auth_tag_len =
               pSrtpCfg->nSRTP_AuthFieldLength;
         if (pSrtpCfg->eSRTP == IFX_TAPI_ENCR_AES_CTR)
            pChannel->pSrtp->policy_tx.rtp.sec_serv = sec_serv_conf_and_auth;
         else
            pChannel->pSrtp->policy_tx.rtp.sec_serv = sec_serv_auth;
         break;
      case IFX_TAPI_AUTH_NONE:
         pChannel->pSrtp->policy_rx.rtp.auth_type = NULL_AUTH;
         pChannel->pSrtp->policy_rx.rtp.auth_key_len = 0;
         if (pSrtpCfg->eSRTP == IFX_TAPI_ENCR_AES_CTR)
            pChannel->pSrtp->policy_rx.rtp.sec_serv = sec_serv_conf;
         else
            pChannel->pSrtp->policy_rx.rtp.sec_serv = sec_serv_none;

         pChannel->pSrtp->policy_tx.rtp.auth_type = NULL_AUTH;
         pChannel->pSrtp->policy_tx.rtp.auth_key_len = 0;
         if (pSrtpCfg->eSRTP == IFX_TAPI_ENCR_AES_CTR)
            pChannel->pSrtp->policy_tx.rtp.sec_serv = sec_serv_conf;
         else
            pChannel->pSrtp->policy_tx.rtp.sec_serv = sec_serv_none;

         break;
      default:
         RETURN_STATUS (TAPI_statusParam, 0);
   }

   /* SRTCP disabled */
   switch (pSrtpCfg->eSRTCP_Auth)
   {
      case IFX_TAPI_AUTH_HMAC_SHA1:
         if (pSrtpCfg->nSRTCP_AuthFieldLength == 0)
         {
            RETURN_STATUS (TAPI_statusParam, 0);
         }
         break;
      case IFX_TAPI_AUTH_NONE:
         break;
      default:
         RETURN_STATUS (TAPI_statusParam, 0);
   }

   /* Enable stream with selected configuration if required */
   if (bSrtpEn == IFX_TRUE)
   {
      crypto_policy_set_rtcp_default(&pChannel->pSrtp->policy_rx.rtcp);
      crypto_policy_set_rtcp_default(&pChannel->pSrtp->policy_tx.rtcp);

      pChannel->pSrtp->policy_rx.ssrc.type           = ssrc_any_outbound;
      pChannel->pSrtp->policy_rx.ssrc.value          = 0;
      pChannel->pSrtp->policy_rx.next               =  NULL;
      pChannel->pSrtp->policy_tx.ssrc.type           = ssrc_any_inbound;
      pChannel->pSrtp->policy_tx.ssrc.value          = 0;
      pChannel->pSrtp->policy_tx.next               =  NULL;
      /* Add configured streams */
      SrtpLibRet = srtp_create(&pChannel->pSrtp->rx_session, &pChannel->pSrtp->policy_rx);
      if (SrtpLibRet != err_status_ok)
         RETURN_STATUS (TAPI_statusSrtpLibErr, SrtpLibRet);

      SrtpLibRet = srtp_create(&pChannel->pSrtp->tx_session, &pChannel->pSrtp->policy_tx);
      if (SrtpLibRet != err_status_ok)
         RETURN_STATUS (TAPI_statusSrtpLibErr, SrtpLibRet);

      /* Enable SRTP, now packets in stream ready for encryption and decryption */
      pChannel->pSrtp->bSrtpEnabled = IFX_TRUE;
   }

   RETURN_STATUS (TAPI_statusOk, 0);
#else /* TAPI_VERSION3 */
   RETURN_STATUS (TAPI_statusNotSupported, 0);
#endif /* TAPI_VERSION3 */
}

/**
   SRTP MKI selection.
   This function will redirect call if SRTP supported in LL driver

   \param  pChannel     Handle to TAPI_CHANNEL structure.
   \param  pData        Configuration.

   \return TAPI_statusOk or error code
*/
IFX_int32_t IFX_TAPI_SRTP_MKI_Set (TAPI_CHANNEL *pChannel, IFX_TAPI_PKT_SRTP_MKI_t const *pData)
{
   IFX_int32_t ret = TAPI_statusOk;
   IFX_TAPI_DRV_CTX_t *pDrvCtx = pChannel->pTapiDevice->pDevDrvCtx;

   /* Switch to hardware realization if it is available */
   if (pDrvCtx->COD.SRTP_MKI_Set != IFX_NULL)
   {
      ret = pDrvCtx->COD.SRTP_MKI_Set (pChannel->pLLChannel, pData);
      if (!TAPI_SUCCESS(ret))
         RETURN_STATUS (TAPI_statusLLFailed, ret);
      else
      {
         /* Return immediately, software realization skipped */
         RETURN_STATUS (TAPI_statusOk, 0);
      }
   }

   /* IOCTL not supported in software realization */
   RETURN_STATUS (TAPI_statusNotSupported, ret);
}

/**
   SRTP MKI configuration.
   This function will redirect call if SRTP supported in LL driver

   \param  pChannel     Handle to TAPI_CHANNEL structure.
   \param  pData        Configuration.

   \return TAPI_statusOk or error code
*/
IFX_int32_t IFX_TAPI_SRTP_MKI_CfgSet (TAPI_CHANNEL *pChannel, IFX_TAPI_PKT_SRTP_MKI_CFG_t const *pData)
{
   IFX_TAPI_DRV_CTX_t *pDrvCtx = pChannel->pTapiDevice->pDevDrvCtx;
   IFX_int32_t ret = TAPI_statusOk;

   /* Switch to hardware realization if it is available */
   if (pDrvCtx->COD.SRTP_MKI_CfgSet != IFX_NULL)
   {
      ret = pDrvCtx->COD.SRTP_MKI_CfgSet (pChannel->pLLChannel, pData);
      if (!TAPI_SUCCESS(ret))
         RETURN_STATUS (TAPI_statusLLFailed, ret);
      else
      {
         /* Return immediately, software realization skipped */
         RETURN_STATUS (TAPI_statusOk, 0);
      }
   }

#ifdef TAPI_VERSION3
   if (pChannel->pSrtp == IFX_NULL)
   {
      /* SRTP not initialized on this channel */
      RETURN_STATUS (TAPI_statusInitSrtpFail, 0);
   }

   if ((pChannel->pSrtp->bSrtpEnabled == IFX_TRUE) &&
       ((pData->nIndex == pChannel->pSrtp->nMKI_RtpRx) ||
        (pData->nIndex == pChannel->pSrtp->nMKI_RtpTx)))
   {
      RETURN_STATUS (TAPI_statusSrtpActive, 0);
   }
   switch (pData->keyType)
   {
   case MasterKey:
      memcpy(pChannel->pSrtp->MKI_Data[pData->nIndex].key,
            pData->KeyDetails.key, IFX_TAPI_SRTP_MKI_KEY_MAX);
      pChannel->pSrtp->MKI_Data[pData->nIndex].bKeyConfigured = IFX_TRUE;
      break;
   case MasterSalt:
      append_salt_to_key ((uint8_t *)pChannel->pSrtp->MKI_Data[pData->nIndex].key, IFX_TAPI_SRTP_MKI_KEY_MAX,
                          (uint8_t *)pData->KeyDetails.key, IFX_TAPI_SRTP_MKI_SALT_MAX);
      pChannel->pSrtp->MKI_Data[pData->nIndex].bSaltConfigured = IFX_TRUE;
      break;
   case MkiRx:
   case MkiTx:
   case MasterInfo:
      /*
      if ((pData->KeyDetails.info.nLifetime <= TAPI_SRTP_LIFETIME_MIN) ||
          (pData->KeyDetails.info.nLifetime < pData->KeyDetails.info.nLifetimeThresh) )
      {
         RETURN_STATUS (TAPI_statusNotSupported, 0);
      }*/
      RETURN_STATUS (TAPI_statusNotSupported, 0);
   default:
      RETURN_STATUS (TAPI_statusParam, 0);
   }
   RETURN_STATUS (ret, 0);
#else /* TAPI_VERSION3 */
   RETURN_STATUS (TAPI_statusNotSupported, 0);
#endif /* TAPI_VERSION3 */
}

/**
   SRTP capabilities.
   This function will redirect call if SRTP supported in LL driver

   \param  pTapiDev     Handle to TAPI_DEV structure.
   \param  pData        Configuration.

   \return TAPI_statusOk or error code
*/
IFX_int32_t IFX_TAPI_SRTP_CapGet (TAPI_DEV *pTapiDev, IFX_TAPI_PKT_SRTP_CAP_GET_t *pData)
{
   IFX_TAPI_DRV_CTX_t *pDrvCtx = pTapiDev->pDevDrvCtx;
   IFX_int32_t ret = TAPI_statusOk;

   /* Switch to hardware realization if it is available */
   if (pDrvCtx->COD.SRTP_CapGet != IFX_NULL)
   {
      ret = pDrvCtx->COD.SRTP_CapGet (pTapiDev->pLLDev, pData);
      if (!TAPI_SUCCESS(ret))
         RETURN_DEVSTATUS (TAPI_statusLLFailed, ret);
      else
      {
         /* Return immediately, software realization skipped */
         RETURN_DEVSTATUS (TAPI_statusOk, 0);
      }
   }

#ifdef TAPI_VERSION3
   /** Number of supported master key and master key index per channel. */
   pData->nMKI = TAPI_SRTP_MAX_KEYS;
   pData->nCapAlgEncr = pData->nCapAlgAuth = 0;
   /** Bit field listing the supported authentication algorithm. */
   pData->nCapAlgAuth = IFX_TAPI_AUTH_HMAC_SHA1;
   /** Bit field listing the supported encryption algorithm. */
   pData->nCapAlgEncr = IFX_TAPI_ENCR_AES_CTR;
   /** Maximum support master key length in bytes. */
   pData->nMasterKeyLength = IFX_TAPI_SRTP_MKI_KEY_MAX;
   /** Maximum support master salt length in bytes. */
   pData->nMasterSaltLength = IFX_TAPI_SRTP_MKI_SALT_MAX;
   /** Maximum support MKI field length. */
   pData->nMKI_Length = IFX_TAPI_SRTP_MKI_KEY_MAX;
   /** Minimal session key derivation rate. Amount of packets before a new session
       key is generated from the master key. */
   pData->nKeyDerivationRate = 0;
   /** RTP Replay protection window size in number of packets. */
   pData->nSRTP_ReplayWindowSize = 0;
   /** RTCP Replay protection window size in number of packets. */
   pData->nSRTCP_ReplayWindowSize = 0;
   /** Maximum support authentication field length. */
   pData->nAuthFieldLength = 0;

   RETURN_DEVSTATUS (ret, 0);
#else /* TAPI_VERSION3 */
   RETURN_DEVSTATUS (TAPI_statusNotSupported, 0);
#endif /* TAPI_VERSION3 */
}

/**
   SRTP statistics.
   This function will redirect call if SRTP supported in LL driver

   \param  pChannel     Handle to TAPI_CHANNEL structure.
   \param  pData        Configuration.

   \return TAPI_statusOk or error code
*/
IFX_int32_t IFX_TAPI_SRTP_StatGet (TAPI_CHANNEL *pChannel, IFX_TAPI_PKT_SRTP_STATISTICS_GET_t *pData)
{
   IFX_int32_t ret = TAPI_statusOk;
   IFX_TAPI_DRV_CTX_t *pDrvCtx = pChannel->pTapiDevice->pDevDrvCtx;

   /* Switch to hardware realization if it is available */
   if (pDrvCtx->COD.SRTP_StatGet != IFX_NULL)
   {
      ret = pDrvCtx->COD.SRTP_StatGet (pChannel->pLLChannel, pData);
      if (!TAPI_SUCCESS(ret))
         RETURN_STATUS (TAPI_statusLLFailed, ret);
      else
      {
         /* Return immediately, software realization skipped */
         RETURN_STATUS (TAPI_statusOk, 0);
      }
   }

#ifdef TAPI_VERSION3
   if (pChannel->pSrtp == IFX_NULL)
   {
      /* SRTP not initialized on this channel */
      RETURN_STATUS (TAPI_statusInitSrtpFail, 0);
   }

   pChannel->pSrtp->TapiSrtpData.dev = pData->dev;
   pChannel->pSrtp->TapiSrtpData.ch = pData->ch;
   memcpy(pData, &pChannel->pSrtp->TapiSrtpData, sizeof(*pData));
   RETURN_STATUS (TAPI_statusOk, 0);
#else
   RETURN_STATUS (TAPI_statusNotSupported, 0);
#endif
}

/**
   SRTP statistics reset.
   This function will redirect call if SRTP supported in LL driver

   \param  pChannel     Handle to TAPI_CHANNEL structure.
   \param  pData        Configuration.

   \return TAPI_statusOk or error code
*/
IFX_int32_t IFX_TAPI_SRTP_StatReset (TAPI_CHANNEL *pChannel, IFX_TAPI_PKT_SRTP_STATISTICS_RESET_t const *pData)
{
   IFX_int32_t ret = TAPI_statusOk;
   IFX_TAPI_DRV_CTX_t *pDrvCtx = pChannel->pTapiDevice->pDevDrvCtx;

   /* Switch to hardware realization if it is available */
   if (pDrvCtx->COD.SRTP_StatReset != IFX_NULL)
   {
      ret = pDrvCtx->COD.SRTP_StatReset (pChannel->pLLChannel, pData);
      if (!TAPI_SUCCESS(ret))
         RETURN_STATUS (TAPI_statusLLFailed, ret);
      else
      {
         /* Return immediately, software realization skipped */
         RETURN_STATUS (TAPI_statusOk, 0);
      }
   }

#ifdef TAPI_VERSION3
   if (pChannel->pSrtp == IFX_NULL)
   {
      /* SRTP not initialized on this channel */
      RETURN_STATUS (TAPI_statusInitSrtpFail, 0);
   }

   memset(&pChannel->pSrtp->TapiSrtpData, 0, sizeof(pChannel->pSrtp->TapiSrtpData));
   RETURN_STATUS (TAPI_statusOk, 0);
#else
   RETURN_STATUS (TAPI_statusNotSupported, 0);
#endif
}

/**
   SRTP set packet index.
   This function will redirect call if SRTP supported in LL driver

   \param  pChannel     Handle to TAPI_CHANNEL structure.
   \param  pData        Configuration.

   \return TAPI_statusOk or error code
*/
IFX_int32_t IFX_TAPI_SRTP_PckIdxSet (TAPI_CHANNEL *pChannel, IFX_TAPI_PKT_SRTP_PACKET_INDEX_SET_t const *pData)
{
   IFX_int32_t ret = TAPI_statusOk;
   IFX_TAPI_DRV_CTX_t *pDrvCtx = pChannel->pTapiDevice->pDevDrvCtx;

   /* Switch to hardware realization if it is available */
   if (pDrvCtx->COD.SRTP_PckIdxSet != IFX_NULL)
   {
      ret = pDrvCtx->COD.SRTP_PckIdxSet (pChannel->pLLChannel, pData);
      if (!TAPI_SUCCESS(ret))
         RETURN_STATUS (TAPI_statusLLFailed, ret);
      else
      {
         /* Return immediately, software realization skipped */
         RETURN_STATUS (TAPI_statusOk, 0);
      }
   }

   /* IOCTL not supported in software realization */
   RETURN_STATUS (TAPI_statusNotSupported, ret);
}

/**
   SRTP get packet index
   This function will redirect call if SRTP supported in LL driver

   \param  pChannel     Handle to TAPI_CHANNEL structure.
   \param  pData        Configuration.

   \return TAPI_statusOk or error code
*/
IFX_int32_t IFX_TAPI_SRTP_PckIdxGet (TAPI_CHANNEL *pChannel, IFX_TAPI_PKT_SRTP_PACKET_INDEX_GET_t *pData)
{
   IFX_int32_t ret = TAPI_statusOk;
   IFX_TAPI_DRV_CTX_t *pDrvCtx = pChannel->pTapiDevice->pDevDrvCtx;

   /* Switch to hardware realization if it is available */
   if (pDrvCtx->COD.SRTP_PckIdxGet != IFX_NULL)
   {
      ret = pDrvCtx->COD.SRTP_PckIdxGet (pChannel->pLLChannel, pData);
      if (!TAPI_SUCCESS(ret))
         RETURN_STATUS (TAPI_statusLLFailed, ret);
      else
      {
         /* Return immediately, software realization skipped */
         RETURN_STATUS (TAPI_statusOk, 0);
      }
   }

   /* IOCTL not supported in software realization */
   RETURN_STATUS (TAPI_statusNotSupported, ret);
}

#ifdef TAPI_VERSION3
/**
   Initialize SRTP on the given channel.

   Initialize the data structures and resources needed for SRTP.

   \param   pChannel    Pointer to TAPI_CHANNEL structure.

   \return
   TAPI_statusOk or error code

   \remarks This function is not protected. The global protection is done
   in the calling function with TAPI_OS_MutexGet (&pChannel->semTapiChDataLock)
*/
IFX_int32_t IFX_TAPI_SRTP_Initialise_Unprot (TAPI_CHANNEL *pChannel)
{
   IFX_TAPI_SRTP_DATA_t *pSrtp = pChannel->pSrtp;

   /* check if channel has the required analog module */
   if (pChannel->nChannel >=  pChannel->pTapiDevice->nResource.CodCount)
   {
      /* no cod module -> nothing to do  --  this is not an error */
      return TAPI_statusOk;
   }

   if (pSrtp != IFX_NULL)
   {
      /* already initialised - do nothing */
      return TAPI_statusOk;
   }

   /* allocate data storage on the channel */
   pSrtp = TAPI_OS_Malloc (sizeof (*pSrtp));
   if (pSrtp == IFX_NULL)
   {
      RETURN_STATUS (TAPI_statusNoMem, 0);
   }

   /* Store pointer to data in the channel or we lose it on exit. */
   pChannel->pSrtp = pSrtp;
   memset (pSrtp, 0x00, sizeof (*pSrtp));

   /* every thing is ok -- this is the regular exit to this function */
   return TAPI_statusOk;
}


/**
   Cleanup SRTP on the given channel.

   Free the resources needed for the metering.

   \param   pChannel    Pointer to TAPI_CHANNEL structure.
*/
IFX_void_t IFX_TAPI_SRTP_Cleanup(TAPI_CHANNEL *pChannel)
{
   TAPI_OS_MutexGet (&pChannel->semTapiChDataLock);

   if (pChannel->pSrtp != IFX_NULL)
   {
      /* free the data storage on the channel */
      TAPI_OS_Free (pChannel->pSrtp);
      pChannel->pSrtp = IFX_NULL;
   }

   TAPI_OS_MutexRelease (&pChannel->semTapiChDataLock);
}
#endif /* TAPI_VERSION3 */
#endif /* TAPI_FEAT_SRTP */
