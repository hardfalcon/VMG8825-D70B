/******************************************************************************

                            Copyright (c) 2014, 2016
                        Lantiq Beteiligungs-GmbH & Co.KG

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

*******************************************************************************/

/**
   \file drv_tapi_fxo.c
   TAPI FXO implementation
   based on a system specific DAA abstraction (GPIO handling) as in drv_daa
*/

#include "drv_tapi.h"

#ifdef TAPI_FEAT_FXO
#include "drv_tapi_errno.h"

/** maximum number of DAA channels TAPI can handle,
    (size of the DAA to TAPI_CH lookuptable) */
#define IFX_TAPI_FXO_MAX_DAA_CH             16


static IFX_TAPI_DRV_CTX_DAA_t *gpDaaCtx = IFX_NULL;
static TAPI_CHANNEL* daaChLookUpTable[IFX_TAPI_FXO_MAX_DAA_CH];

static TAPI_CHANNEL* lookup_pTAPICh (IFX_int32_t nDAA);

#endif /* TAPI_FEAT_FXO */

/**
   Update the SmartSLIC FXO boolean flag.

   The flag is required to indicate systems which have
   SmartSLIC and fused FXO line(s).
   For such systems a special handling of FXO ioctls is required.

   \param  pTapiDev     Pointer to TAPI device structure.
   \param  bVal         new flag value.

   \return
      none

   \remarks
      This function can be called from LL driver.
*/
IFX_void_t IFX_TAPI_Update_SlicFxo (TAPI_DEV *pTapiDev, IFX_boolean_t bVal)
{
#ifdef TAPI_FEAT_FXO
   pTapiDev->bSmartSlicFxo = bVal;
#else
   IFX_UNUSED (pTapiDev);
   IFX_UNUSED (bVal);
#endif /* TAPI_FEAT_FXO */
}

#ifdef TAPI_FEAT_FXO

/**
   lookup the TAPI_CHANNEL context belonging to a DAA number
   (required for DAA events)
   \param nDAA - DAA number which raised the event, starting at 0
   \return     - pointer to a TAPI_CHANNEL context,
                 IFX_NULL in case of unknown
*/
static TAPI_CHANNEL* lookup_pTAPICh (IFX_int32_t nDAA)
{
   /* sanity check */
   if (nDAA >= IFX_TAPI_FXO_MAX_DAA_CH)
      return (TAPI_CHANNEL*) IFX_NULL;
   return daaChLookUpTable[nDAA];
}


/**
   Register a DAA channel in the lookup table
   \param pChannel      Pointer to a TAPI_CHANNEL context.
   \param nDAA          DAA number (starting at 0).
   \return IFX_SUCCESS or IFX_ERROR in case of failure
*/
IFX_int32_t TAPI_FXO_Register_DAA (TAPI_CHANNEL *pChannel, IFX_int32_t nDAA)
{
   /* protection could be added as the lookup table is accessed from
      the DAA's interrupt context - which can be any interreupt context,
      currently we expect that the the linetype configuration is finished
      before the first event is reported, i.e. the interrupts of the
      DAA are unmasked(!) */
   /* sanity check */
   if (nDAA >= IFX_TAPI_FXO_MAX_DAA_CH)
      return(IFX_ERROR);
   /* store channel context */
   daaChLookUpTable[nDAA] = pChannel;
   return IFX_SUCCESS;
}


/**
   initialize a daa channel after registration, i.e. configure the GPIOs
   and internal data structures of the daa driver
   \param pChannel  - pointer to a TAPI_CHANNEL context
   \return IFX_SUCCESS or IFX_ERROR in case of failure
*/
IFX_int32_t TAPI_FXO_Init_DAA (TAPI_CHANNEL *pChannel)
{
   IFX_int32_t ret;

   if (gpDaaCtx == IFX_NULL)
   {
      return IFX_ERROR;
   }

   ret = gpDaaCtx->Init(pChannel->TapiOpControlData.nDAA);
   if (ret == IFX_SUCCESS)
   {
      ret = gpDaaCtx->hookSet(pChannel->TapiOpControlData.nDAA,
                              IFX_TAPI_FXO_HOOK_ONHOOK);
   }

   return ret;
}


/**
   register a daa abstraction driver to drv_tapi
   \param pDaaCtx - reference to a daa driver context
   \return        - IFX_SUCCESS or IFX_ERROR in case of failure
   \remarks
   function is exported to be called by the DAA abstraction driver
*/
IFX_int32_t IFX_TAPI_Register_DAA_Drv (IFX_TAPI_DRV_CTX_DAA_t *pDaaCtx)
{
   /* add version check before assigning the pointer */
   gpDaaCtx = pDaaCtx;
   return IFX_SUCCESS;
}


/**
   FXO event dispatcher wrapper function
   (including the mapping from DAA numbers to TAPI channels)
   \param nDAA   - DAA number which raised the event, starting at 0
   \param pEvent - TAPI event structure containing the event details
   \return       - IFX_SUCCESS or IFX_ERROR in case of failure
   \remarks
      called from daa driver's interrupt context
*/
IFX_int32_t IFX_TAPI_FXO_Event_Dispatch (
   IFX_int32_t nDAA,
   IFX_TAPI_EVENT_t *pEvent)
{
   TAPI_CHANNEL *pTAPICh = lookup_pTAPICh(nDAA);

   /* sanity check */
   if (pTAPICh == IFX_NULL)
      return IFX_ERROR;

   /* forward event to TAPI Event Dispatcher */
   return IFX_TAPI_Event_Dispatch (pTAPICh, pEvent);
}

/** Configuration for DTMF dialing. Mainly used on FXO lines but can be also
    used on analog lines.

   \param pChannel      Handle to the TAPI channel.
   \param p_DialCfg     Handle to the \ref IFX_TAPI_FXO_DIAL_CFG_t structure.

   \return
      TAPI_statusOk if successful else error code.
*/
IFX_int32_t IFX_TAPI_FXO_Dial_Cfg_Set (
   TAPI_CHANNEL *pChannel,
   IFX_TAPI_FXO_DIAL_CFG_t const *p_DialCfg)
{
   IFX_TAPI_DRV_CTX_t *pDrvCtx = pChannel->pTapiDevice->pDevDrvCtx;
   IFX_int32_t retLL;

   if (!IFX_TAPI_PtrChk (pDrvCtx->SIG.DTMFG_Cfg))
      RETURN_STATUS (TAPI_statusLLNotSupp, 0);

   retLL = pDrvCtx->SIG.DTMFG_Cfg(pChannel->pLLChannel,
      p_DialCfg->nInterDigitTime, p_DialCfg->nDigitPlayTime);

   if (!TAPI_SUCCESS (retLL))
      RETURN_STATUS (TAPI_statusLLFailed, retLL);

   return TAPI_statusOk;
}

/** Configuration of the fxo hook.

   \param pChannel   Handle to the TAPI channel.
   \param p_fhCfg    Handle to the \ref IFX_TAPI_FXO_FLASH_CFG_t structure.

   \return
      TAPI_statusOk if successful else error code.
*/
IFX_int32_t IFX_TAPI_FXO_Flash_Cfg_Set (
   TAPI_CHANNEL *pChannel,
   IFX_TAPI_FXO_FLASH_CFG_t const *p_fhCfg)
{
   TAPI_ASSERT (pChannel);

   if (IFX_TRUE == pChannel->pTapiDevice->bSmartSlicFxo)
   {
      IFX_TAPI_DRV_CTX_t *pDrvCtx = pChannel->pTapiDevice->pDevDrvCtx;
      IFX_int32_t retLL;

      /* sanity check for low-level support */
      if ((IFX_NULL == pDrvCtx) || (IFX_NULL == pDrvCtx->ALM.FXO_FlashCfg))
         RETURN_STATUS (TAPI_statusLLNotSupp, 0);

      retLL = pDrvCtx->ALM.FXO_FlashCfg(pChannel->pLLChannel, p_fhCfg);

      if (!TAPI_SUCCESS (retLL))
         RETURN_STATUS (TAPI_statusLLFailed, retLL);
   }
   else
   {
      /* configuration is allowed also on non FXO channels, sanity check */
      if ((gpDaaCtx == IFX_NULL) || (gpDaaCtx->fhCfg == IFX_NULL))
         RETURN_STATUS (TAPI_statusNotSupported, 0);

      return gpDaaCtx->fhCfg(p_fhCfg->nFlashTime);
   }

   return TAPI_statusOk;
}

/** Configuration of OSI timing.

   \param pChannel   Handle to the TAPI channel.
   \param p_osiCfg   Handle to the \ref IFX_TAPI_FXO_OSI_CFG_t structure.

   \return
      TAPI_statusOk if successful else error code.
*/
IFX_int32_t IFX_TAPI_FXO_OSI_Cfg_Set (
   TAPI_CHANNEL *pChannel,
   IFX_TAPI_FXO_OSI_CFG_t const *p_osiCfg)
{
   TAPI_ASSERT (pChannel);

   if (IFX_TRUE == pChannel->pTapiDevice->bSmartSlicFxo)
   {
      IFX_TAPI_DRV_CTX_t *pDrvCtx = pChannel->pTapiDevice->pDevDrvCtx;
      IFX_int32_t retLL;

      /* sanity check for low-level support */
      if ((IFX_NULL == pDrvCtx) || (IFX_NULL == pDrvCtx->ALM.FXO_OsiCfg))
         RETURN_STATUS (TAPI_statusLLNotSupp, 0);

      retLL = pDrvCtx->ALM.FXO_OsiCfg (pChannel->pLLChannel, p_osiCfg);

      if (!TAPI_SUCCESS (retLL))
         RETURN_STATUS (TAPI_statusLLFailed, retLL);
   }
   else
   {
      /* configuration is allowed also on non FXO channels */
      if ((gpDaaCtx == IFX_NULL) || (gpDaaCtx->osiCfg == IFX_NULL))
         RETURN_STATUS (TAPI_statusNotSupported, 0);

      return gpDaaCtx->osiCfg(p_osiCfg->nOSIMax);
   }

   return TAPI_statusOk;
}

/** Dials DTMF digits.

   \param pChannel      Handle to the TAPI channel
   \param p_DialData    Handle to the \ref IFX_TAPI_FXO_DIAL_t structure.

   \return
      TAPI_statusOk if successful else error code.
*/
IFX_int32_t IFX_TAPI_FXO_Dial_Start (TAPI_CHANNEL *pChannel,
   IFX_TAPI_FXO_DIAL_t const *p_DialData)
{
   IFX_TAPI_DRV_CTX_t *pDrvCtx = pChannel->pTapiDevice->pDevDrvCtx;
   IFX_int32_t retLL;

   if (!IFX_TAPI_PtrChk (pDrvCtx->SIG.DTMFG_Start))
      RETURN_STATUS (TAPI_statusLLNotSupp, 0);

   if ( p_DialData->nDigits > IFX_TAPI_FXO_DIAL_DIGITS )
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
           ("TAPI: FXO dial-length value exceeds buffer size\n"));
      RETURN_STATUS (TAPI_statusParam, 0);
   }

   retLL = pDrvCtx->SIG.DTMFG_Start(pChannel->pLLChannel,
      p_DialData->nDigits, p_DialData->data);

   if (!TAPI_SUCCESS (retLL))
      RETURN_STATUS (TAPI_statusLLFailed, retLL);

   return TAPI_statusOk;
}

/** Issues on-/off-hook in the fxo interface.

   \param pChannel   Handle to the TAPI channel.
   \param pHook      Handle to the \ref IFX_TAPI_FXO_HOOK_t structure.

   \return
      TAPI_statusOk if successful else error code.
*/
IFX_int32_t IFX_TAPI_FXO_Hook_Set (
   TAPI_CHANNEL *pChannel,
   IFX_TAPI_FXO_HOOK_t nHook)
{
   TAPI_ASSERT (pChannel);

   if (IFX_TRUE == pChannel->pTapiDevice->bSmartSlicFxo)
   {
      IFX_TAPI_DRV_CTX_t *pDrvCtx = pChannel->pTapiDevice->pDevDrvCtx;
      IFX_int32_t retLL;

      /* sanity check for low-level support */
      if ((IFX_NULL == pDrvCtx) || (IFX_NULL == pDrvCtx->ALM.FXO_HookSet))
         RETURN_STATUS (TAPI_statusLLNotSupp, 0);

      retLL = pDrvCtx->ALM.FXO_HookSet (pChannel->pLLChannel, nHook);

      if (!TAPI_SUCCESS (retLL))
         RETURN_STATUS (TAPI_statusLLFailed, retLL);
   }
   else
   {
      /* sanity check - ioctl can only be applied to FXO channels */
      if (pChannel->TapiOpControlData.nLineType != IFX_TAPI_LINE_TYPE_FXO)
         RETURN_STATUS (TAPI_statusInvalidCh, 0);

      if (! pChannel->TapiOpControlData.bDaaInitialized)
      {
         /* errmsg: Daa was not initialized */
         RETURN_STATUS (TAPI_statusInitDaa, 0);
      }

      if ((IFX_NULL == gpDaaCtx) || (IFX_NULL == gpDaaCtx->hookSet))
         RETURN_STATUS (TAPI_statusNotSupported, 0);

      return gpDaaCtx->hookSet(pChannel->TapiOpControlData.nDAA, nHook);
   }

   return TAPI_statusOk;
}

/** Issues flash-hook in the FXO interface.

   \param pChannel   Handle to the TAPI channel.

   \return
      TAPI_statusOk if successful else error code.
*/
IFX_int32_t IFX_TAPI_FXO_Flash_Set (TAPI_CHANNEL *pChannel)
{
   TAPI_ASSERT (pChannel);

   if (IFX_TRUE == pChannel->pTapiDevice->bSmartSlicFxo)
   {
      IFX_TAPI_DRV_CTX_t *pDrvCtx = pChannel->pTapiDevice->pDevDrvCtx;
      IFX_int32_t retLL;

      /* sanity check for low-level support */
      if ((IFX_NULL == pDrvCtx) || (IFX_NULL == pDrvCtx->ALM.FXO_FlashSet))
         RETURN_STATUS (TAPI_statusLLNotSupp, 0);

      retLL = pDrvCtx->ALM.FXO_FlashSet (pChannel->pLLChannel);

      if (!TAPI_SUCCESS (retLL))
         RETURN_STATUS (TAPI_statusLLFailed, retLL);
   }
   else
   {
      /* sanity check - ioctl can only be applied to FXO channels */
      if (pChannel->TapiOpControlData.nLineType != IFX_TAPI_LINE_TYPE_FXO)
         RETURN_STATUS (TAPI_statusInvalidCh, 0);

      if (! pChannel->TapiOpControlData.bDaaInitialized)
      {
         /* errmsg: Daa was not initialized */
         RETURN_STATUS (TAPI_statusInitDaa, 0);
      }

      if ((IFX_NULL == gpDaaCtx) || (IFX_NULL == gpDaaCtx->fhSet))
         RETURN_STATUS (TAPI_statusNotSupported, 0);

      return gpDaaCtx->fhSet(pChannel->TapiOpControlData.nDAA);
   }

   return TAPI_statusOk;
}

/** Receives battery status from the FXO interface.

   \param pChannel   Handle to the TAPI channel.
   \param pBat       Handle to the \ref IFX_TAPI_FXO_BAT_t structure.

   \return
      TAPI_statusOk if successful else error code.
*/
IFX_int32_t IFX_TAPI_FXO_Bat_Get (
   TAPI_CHANNEL *pChannel,
   IFX_TAPI_FXO_BAT_t *pBat)
{
   IFX_enDis_t *pMode;

   TAPI_ASSERT (pChannel);

#if defined (TAPI_ONE_DEVNODE)
   pMode = &pBat->mode;
#else
   pMode = pBat;
#endif /* defined (TAPI_ONE_DEVNODE) */

   if (IFX_TRUE == pChannel->pTapiDevice->bSmartSlicFxo)
   {
      IFX_TAPI_DRV_CTX_t *pDrvCtx = pChannel->pTapiDevice->pDevDrvCtx;
      IFX_int32_t retLL;

      /* sanity check for low-level support */
      if ((IFX_NULL == pDrvCtx) || (IFX_NULL == pDrvCtx->ALM.FXO_StatGet))
         RETURN_STATUS (TAPI_statusLLNotSupp, 0);

      retLL = pDrvCtx->ALM.FXO_StatGet (pChannel->pLLChannel,
         FXO_BATTERY, pMode);

      if (!TAPI_SUCCESS (retLL))
         RETURN_STATUS (TAPI_statusLLFailed, retLL);
   }
   else
   {
      /* sanity check - ioctl can only be applied to FXO channels */
      if (pChannel->TapiOpControlData.nLineType != IFX_TAPI_LINE_TYPE_FXO)
         RETURN_STATUS (TAPI_statusInvalidCh, 0);

      if (! pChannel->TapiOpControlData.bDaaInitialized)
      {
         /* errmsg: Daa was not initialized */
         RETURN_STATUS (TAPI_statusInitDaa, 0);
      }

      if ((IFX_NULL == gpDaaCtx) || (IFX_NULL == gpDaaCtx->hookGet))
         RETURN_STATUS (TAPI_statusNotSupported, 0);

      return gpDaaCtx->batGet (pChannel->TapiOpControlData.nDAA, pMode);
   }

   return TAPI_statusOk;
}

/** Retrieves the current hook state on a FXO channel (set by the
    application itself).

   \param pChannel   Handle to the TAPI channel.
   \param pHook      Handle to the \ref IFX_TAPI_FXO_HOOK_CFG_t structure.

   \return
      TAPI_statusOk if successful else error code.
*/
IFX_int32_t IFX_TAPI_FXO_Hook_Get (
   TAPI_CHANNEL *pChannel,
   IFX_TAPI_FXO_HOOK_CFG_t *pHook)
{
   IFX_TAPI_FXO_HOOK_t *pHookMode;

   TAPI_ASSERT (pChannel);

#if defined (TAPI_ONE_DEVNODE)
   pHookMode = &pHook->hookMode;
#else
   pHookMode = pHook;
#endif /* defined (TAPI_ONE_DEVNODE) */

   if (IFX_TRUE == pChannel->pTapiDevice->bSmartSlicFxo)
   {
      IFX_TAPI_DRV_CTX_t *pDrvCtx = pChannel->pTapiDevice->pDevDrvCtx;
      IFX_int32_t retLL;

      /* sanity check for low-level support */
      if ((IFX_NULL == pDrvCtx) || (IFX_NULL == pDrvCtx->ALM.FXO_HookGet))
         RETURN_STATUS (TAPI_statusLLNotSupp, 0);

      retLL = pDrvCtx->ALM.FXO_HookGet (pChannel->pLLChannel, pHookMode);

      if (!TAPI_SUCCESS (retLL))
         RETURN_STATUS (TAPI_statusLLFailed, retLL);
   }
   else
   {
      /* sanity check - ioctl can only be applied to FXO channels */
      if (pChannel->TapiOpControlData.nLineType != IFX_TAPI_LINE_TYPE_FXO)
         RETURN_STATUS (TAPI_statusInvalidCh, 0);

      if (! pChannel->TapiOpControlData.bDaaInitialized)
      {
         /* errmsg: Daa was not initialized */
         RETURN_STATUS (TAPI_statusInitDaa, 0);
      }

      if ((IFX_NULL == gpDaaCtx) || (IFX_NULL == gpDaaCtx->hookGet))
         RETURN_STATUS (TAPI_statusNotSupported, 0);

      return gpDaaCtx->hookGet (pChannel->TapiOpControlData.nDAA, pHookMode);
   }

   return TAPI_statusOk;
}

/** Retrieves APOH (another phone off-hook) status of the fxo interface.

   \param pChannel   Handle to the TAPI channel.
   \param pApoh      Handle to the \ref IFX_TAPI_FXO_APOH_t structure.

   \return
      TAPI_statusOk if successful else error code.
*/
IFX_int32_t IFX_TAPI_FXO_Apoh_Get (
   TAPI_CHANNEL *pChannel,
   IFX_TAPI_FXO_APOH_t *pApoh)
{
   IFX_enDis_t *pMode;

   TAPI_ASSERT (pChannel);

#if defined (TAPI_ONE_DEVNODE)
   pMode = &pApoh->mode;
#else
   pMode = pApoh;
#endif /* defined (TAPI_ONE_DEVNODE) */

   if (IFX_TRUE == pChannel->pTapiDevice->bSmartSlicFxo)
   {
      IFX_TAPI_DRV_CTX_t *pDrvCtx = pChannel->pTapiDevice->pDevDrvCtx;
      IFX_int32_t retLL;

      /* sanity check for low-level support */
      if ((IFX_NULL == pDrvCtx) || (IFX_NULL == pDrvCtx->ALM.FXO_StatGet))
         RETURN_STATUS (TAPI_statusLLNotSupp, 0);

      retLL = pDrvCtx->ALM.FXO_StatGet (pChannel->pLLChannel,
         FXO_APOH, pMode);

      if (!TAPI_SUCCESS (retLL))
         RETURN_STATUS (TAPI_statusLLFailed, retLL);
   }
   else
   {
      /* sanity check - ioctl can only be applied to FXO channels */
      if (pChannel->TapiOpControlData.nLineType != IFX_TAPI_LINE_TYPE_FXO)
         RETURN_STATUS (TAPI_statusInvalidCh, 0);

      if (! pChannel->TapiOpControlData.bDaaInitialized)
      {
         /* errmsg: Daa was not initialized */
         RETURN_STATUS (TAPI_statusInitDaa, 0);
      }

      if ((IFX_NULL == gpDaaCtx) || (IFX_NULL == gpDaaCtx->hookGet))
         RETURN_STATUS (TAPI_statusNotSupported, 0);

      return gpDaaCtx->apohGet (pChannel->TapiOpControlData.nDAA, pMode);
   }

   return TAPI_statusOk;
}

/** Receives polarity status from the FXO interface.

   \param pChannel   Handle to the TAPI channel.
   \param pPol       Handle to the \ref IFX_TAPI_FXO_POLARITY_t structure.

   \return
      TAPI_statusOk if successful else error code.
*/
IFX_int32_t IFX_TAPI_FXO_Polarity_Get (
   TAPI_CHANNEL *pChannel,
   IFX_TAPI_FXO_POLARITY_t *pPol)
{
   IFX_enDis_t *pMode = IFX_NULL;

   TAPI_ASSERT (pChannel);

#if defined (TAPI_ONE_DEVNODE)
   pMode = &pPol->polarityMode;
#else
   pMode = pPol;
#endif /* defined (TAPI_ONE_DEVNODE) */

   if (IFX_TRUE == pChannel->pTapiDevice->bSmartSlicFxo)
   {
      IFX_TAPI_DRV_CTX_t *pDrvCtx = pChannel->pTapiDevice->pDevDrvCtx;
      IFX_int32_t retLL;

      /* sanity check for low-level support */
      if ((IFX_NULL == pDrvCtx) || (IFX_NULL == pDrvCtx->ALM.FXO_StatGet))
         RETURN_STATUS (TAPI_statusLLNotSupp, 0);

      retLL = pDrvCtx->ALM.FXO_StatGet (pChannel->pLLChannel,
         FXO_POLARITY, pMode);

      if (!TAPI_SUCCESS (retLL))
         RETURN_STATUS (TAPI_statusLLFailed, retLL);
   }
   else
   {
      /* sanity check - ioctl can only be applied to FXO channels */
      if (pChannel->TapiOpControlData.nLineType != IFX_TAPI_LINE_TYPE_FXO)
         RETURN_STATUS (TAPI_statusInvalidCh, 0);

      if (! pChannel->TapiOpControlData.bDaaInitialized)
      {
         /* errmsg: Daa was not initialized */
         RETURN_STATUS (TAPI_statusInitDaa, 0);
      }

      if ((IFX_NULL == gpDaaCtx) || (IFX_NULL == gpDaaCtx->hookGet))
         RETURN_STATUS (TAPI_statusNotSupported, 0);

      return gpDaaCtx->polGet (pChannel->TapiOpControlData.nDAA, pMode);
   }

   return TAPI_statusOk;
}

/** Receives ring status from the FXO interface.

   \param pChannel   Handle to the TAPI channel.
   \param pRing      Handle to the \ref IFX_TAPI_FXO_RING_STATUS_t structure.

   \return
      TAPI_statusOk if successful else error code.
*/
IFX_int32_t IFX_TAPI_FXO_Ring_Get (
   TAPI_CHANNEL *pChannel,
   IFX_TAPI_FXO_RING_STATUS_t *pRing)
{
   IFX_enDis_t *pMode = IFX_NULL;

   TAPI_ASSERT (pChannel);

#if defined (TAPI_ONE_DEVNODE)
   pMode = &pRing->ringMode;
#else
   pMode = pRing;
#endif /* defined (TAPI_ONE_DEVNODE) */

   if (IFX_TRUE == pChannel->pTapiDevice->bSmartSlicFxo)
   {
      IFX_TAPI_DRV_CTX_t *pDrvCtx = pChannel->pTapiDevice->pDevDrvCtx;
      IFX_int32_t retLL;

      /* sanity check for low-level support */
      if ((IFX_NULL == pDrvCtx) || (IFX_NULL == pDrvCtx->ALM.FXO_StatGet))
         RETURN_STATUS (TAPI_statusLLNotSupp, 0);

      retLL = pDrvCtx->ALM.FXO_StatGet (pChannel->pLLChannel,
         FXO_RING, pMode);

      if (!TAPI_SUCCESS (retLL))
         RETURN_STATUS (TAPI_statusLLFailed, retLL);
   }
   else
   {
      /* sanity check - ioctl can only be applied to FXO channels */
      if (pChannel->TapiOpControlData.nLineType != IFX_TAPI_LINE_TYPE_FXO)
         RETURN_STATUS (TAPI_statusInvalidCh, 0);

      if (! pChannel->TapiOpControlData.bDaaInitialized)
      {
         /* errmsg: Daa was not initialized */
         RETURN_STATUS (TAPI_statusInitDaa, 0);
      }

      if ((IFX_NULL == gpDaaCtx) || (IFX_NULL == gpDaaCtx->hookGet))
         RETURN_STATUS (TAPI_statusNotSupported, 0);

      return gpDaaCtx->ringGet (pChannel->TapiOpControlData.nDAA, pMode);
   }

   return TAPI_statusOk;
}

#endif /* TAPI_FEAT_FXO */
