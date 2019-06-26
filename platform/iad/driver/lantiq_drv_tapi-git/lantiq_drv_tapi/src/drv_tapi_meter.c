/******************************************************************************

                              Copyright (c) 2014
                            Lantiq Deutschland GmbH

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/**
   \file drv_tapi_meter.c
   TAPI Metering Service.
*/

/* ============================= */
/* Includes                      */
/* ============================= */

#include "drv_tapi.h"

#ifdef TAPI_FEAT_METERING

#include "drv_tapi_errno.h"

/* ============================= */
/* Type declarations             */
/* ============================= */

struct TAPI_METER_DATA
{
   /* is metering currently active? */
   IFX_boolean_t      bMeterActive;
   /* timer id for metering */
   Timer_ID           MeterTimerID;
   /* metering coniguration data */
   IFX_TAPI_METER_CFG_t  MeterConfig;
   /* distance between the metering bursts in ms */
   IFX_uint32_t       nPulseDist;
   /* number of bursts */
   IFX_uint32_t       nPulses;
   /* last linemode befor metering start */
   IFX_uint8_t        nLastLineMode;
   /* Number of bursts that are transmitted since the last statistic call */
   IFX_uint32_t       nBurstTransmitted;
   /* Number of pulses that are requested by the periodic timer or application
      intermediate burst and which are not generated yet.
      They will be generated by the firwmare. */
   IFX_uint32_t       nPulseOutstanding;
};

/* ============================= */
/* Local function declaration    */
/* ============================= */

static IFX_void_t  ifx_tapi_Meter_OnTimer(Timer_ID Timer, IFX_int32_t nArg);

/* ============================= */
/* Local function definitions    */
/* ============================= */

/**
   Function called by timer which switches the meter status.

   \param Timer         TimerID of timer that exipres.
   \param nArg          Argument of timer including the TAPI_CHANNEL structure
                        (cast to an integer)
*/
static IFX_void_t ifx_tapi_Meter_OnTimer(Timer_ID Timer, IFX_int32_t nArg)
{
   TAPI_CHANNEL *pChannel = (TAPI_CHANNEL *) nArg;
   TAPI_METER_DATA_t *pMeter = pChannel->pTapiMeterData;
   IFX_TAPI_DRV_CTX_t* pDrvCtx = pChannel->pTapiDevice->pDevDrvCtx;

   /* Metering burst */
   TAPI_SetTime_Timer(Timer, pMeter->nPulseDist, IFX_FALSE, IFX_FALSE);

   /* send first pulse */
   if (IFX_TAPI_PtrChk (pDrvCtx->ALM.Metering_Start))
   {
      (IFX_void_t)pDrvCtx->ALM.Metering_Start (pChannel->pLLChannel,
                                   pMeter->nPulses);
      pMeter->nBurstTransmitted++;
      pMeter->nPulseOutstanding += pMeter->nPulses;
   }
   return;
}

/* ============================= */
/* Global function definitions   */
/* ============================= */

/**
   Initialise metering on the given channel.

   Initialise the data structures and resources needed for metering.

   \param   pChannel    Pointer to TAPI_CHANNEL structure.

   \return
   TAPI_statusOk or error code

   \remarks This function is not protected. The global protection is done
   in the calling function with TAPI_OS_MutexGet (&pChannel->semTapiChDataLock)
*/
IFX_int32_t IFX_TAPI_Meter_Initialise_Unprot (TAPI_CHANNEL *pChannel)
{
   TAPI_METER_DATA_t *pTapiMeterData = pChannel->pTapiMeterData;

   /* check if channel has the required analog module */
   if (pChannel->nChannel >=  pChannel->pTapiDevice->nResource.AlmCount)
   {
      /* no analog module -> nothing to do  --  this is not an error */
      return TAPI_statusOk;
   }

   if (pTapiMeterData != IFX_NULL)
   {
      /* already initialised - do nothing */
      return TAPI_statusOk;
   }

   /* allocate data storage on the channel */
   pTapiMeterData = TAPI_OS_Malloc (sizeof (*pTapiMeterData));
   if (pTapiMeterData == IFX_NULL)
   {
      RETURN_STATUS (TAPI_statusNoMem, 0);
   }

   /* Store pointer to data in the channel or we lose it on exit. */
   pChannel->pTapiMeterData = pTapiMeterData;
   memset (pTapiMeterData, 0x00, sizeof (*pTapiMeterData));

   /* initialize (create) metering timer */
   pTapiMeterData->MeterTimerID =
      TAPI_Create_Timer((TIMER_ENTRY)ifx_tapi_Meter_OnTimer, (IFX_uintptr_t)pChannel);
   if(pTapiMeterData->MeterTimerID == IFX_NULL)
   {
      TAPI_OS_Free (pTapiMeterData);
      RETURN_STATUS (TAPI_statusNoMem, 0);
   }

   /* every thing is ok -- this is the regular exit to this function */
   return TAPI_statusOk;
}


/**
   Cleanup metering on the given channel.

   Free the resources needed for the metering.

   \param   pChannel    Pointer to TAPI_CHANNEL structure.
*/
IFX_void_t IFX_TAPI_Meter_Cleanup(TAPI_CHANNEL *pChannel)
{
   TAPI_OS_MutexGet (&pChannel->semTapiChDataLock);

   if (pChannel->pTapiMeterData != IFX_NULL)
   {
      /* unconditionally destruct the metering timer if existing */
      if (pChannel->pTapiMeterData->MeterTimerID != (Timer_ID)0)
      {
         TAPI_Delete_Timer (pChannel->pTapiMeterData->MeterTimerID);
      }

      /* free the data storage on the channel */
      TAPI_OS_Free (pChannel->pTapiMeterData);
      pChannel->pTapiMeterData = IFX_NULL;
   }

   TAPI_OS_MutexRelease (&pChannel->semTapiChDataLock);
}


/**
   Return the status of the metering module.

   \param   pChannel    Pointer to TAPI_CHANNEL structure.

   \return
     - \ref IFX_TRUE:  if metering is active
     - \ref IFX_FALSE: if metering is not active or not installed on channel
*/
IFX_boolean_t TAPI_Phone_Meter_IsActive (TAPI_CHANNEL *pChannel)
{
   if (pChannel->pTapiMeterData != IFX_NULL)
   {
      return pChannel->pTapiMeterData->bMeterActive;
   }
   else
   {
      return IFX_FALSE;
   }
}

/**
   Starts the metering.

    Check if all pre-conditions are ok then start the timer for metering.

   \param pChannel      Pointer to TAPI_CHANNEL structure.
   \param pMeterStart  Contains the start params (IFX_TAPI_METER_START_t)

   \return
   TAPI_statusOk or error code
*/
IFX_int32_t TAPI_Phone_Meter_Start (TAPI_CHANNEL *pChannel,
                                    IFX_TAPI_METER_START_t const *pMeterStart)
{
   TAPI_METER_DATA_t *pTapiMeterData = pChannel->pTapiMeterData;
   IFX_TAPI_DRV_CTX_t* pDrvCtx = pChannel->pTapiDevice->pDevDrvCtx;
   IFX_int32_t ret = TAPI_statusOk;

   if (pTapiMeterData == IFX_NULL)
   {
      /* metering not initialised on this channel */
      RETURN_STATUS (TAPI_statusInitMeterFail, 0);
   }

#ifdef TAPI_FEAT_RINGENGINE
   /* check if telephone isn't ringing */
   if (IFX_TAPI_Ring_IsActive(pChannel) == IFX_TRUE)
   {
      /* channel is just ringing */
      RETURN_STATUS (TAPI_statusRingAct, 0);
   }
#endif /* TAPI_FEAT_RINGENGINE */

   /* check if line is in active state */
   if (!(pChannel->TapiOpControlData.nLineMode == IFX_TAPI_LINE_FEED_ACTIVE ||
       pChannel->TapiOpControlData.nLineMode == IFX_TAPI_LINE_FEED_ACTIVE_REV ||
       pChannel->TapiOpControlData.nLineMode == IFX_TAPI_LINE_FEED_NORMAL_AUTO ||
       pChannel->TapiOpControlData.nLineMode == IFX_TAPI_LINE_FEED_REVERSED_AUTO))
   {
      RETURN_STATUS (TAPI_statusLineNotAct, 0);
   }

   /* make sure metering is not already activated */
   if (TAPI_Phone_Meter_IsActive (pChannel) == IFX_TRUE)
   {
      /* metering is already active */
      RETURN_STATUS (TAPI_statusMeterAct, 0);
   }

   /* check if meter characteristic ist set correctly */
   if ((pMeterStart->nPulseDist == 0) !=
       (pMeterStart->nPulses == 0)) /*lint !e731 */
   {
      /* bad params */
      RETURN_STATUS (TAPI_statusParam, 0);
   }

   if (pMeterStart->nPulses)
   {
      /* check if burst time less than pause between pulses */
      if (((pTapiMeterData->MeterConfig.nPulseLen +
            pTapiMeterData->MeterConfig.nPauseLen) *
            pMeterStart->nPulses -
            pTapiMeterData->MeterConfig.nPauseLen) >
         (pMeterStart->nPulseDist * 1000) )
      {
         /* bad params */
         RETURN_STATUS (TAPI_statusParam, 0);
      }
   }

   pTapiMeterData->nPulseDist = pMeterStart->nPulseDist * 1000;
   pTapiMeterData->nPulses = pMeterStart->nPulses;

   /* save line mode */
   pTapiMeterData->nLastLineMode = pChannel->TapiOpControlData.nLineMode;

   /* send first pulse */
   if (IFX_TAPI_PtrChk (pDrvCtx->ALM.Metering_Start))
   {
      ret = pDrvCtx->ALM.Metering_Start (pChannel->pLLChannel,
                                         pTapiMeterData->nPulses);
      pTapiMeterData->nBurstTransmitted++;
      pTapiMeterData->nPulseOutstanding += pTapiMeterData->nPulses;
   }
   else
   {
      /* metering is already active */
      RETURN_STATUS (TAPI_statusNotSupported, 0);
   }

   if (pMeterStart->nPulseDist > 0)
   {
      /* start the timer for metering */
      TAPI_SetTime_Timer(pTapiMeterData->MeterTimerID,
                           pTapiMeterData->nPulseDist,
                           IFX_FALSE, IFX_FALSE);
   }

   /* Metering is active now */
   pTapiMeterData->bMeterActive = IFX_TRUE;

   return ret;
}


/**
   Stops the metering.

   \param pChannel      Pointer to TAPI_CHANNEL structure.

   \return
   TAPI_statusOk or error code
*/
IFX_int32_t TAPI_Phone_Meter_Stop (TAPI_CHANNEL *pChannel)
{
   IFX_int32_t ret = TAPI_statusOk;

   if (pChannel->pTapiMeterData == IFX_NULL)
   {
      /* metering not initialised on this channel */
      RETURN_STATUS (TAPI_statusInitMeterFail, 0);
   }

   if (TAPI_Phone_Meter_IsActive (pChannel) == IFX_TRUE)
   {
      TAPI_Stop_Timer(pChannel->pTapiMeterData->MeterTimerID);
      /* Metering is deactivated */
      pChannel->pTapiMeterData->bMeterActive = IFX_FALSE;
#ifdef TAPI_VERSION3
      /* recover the last line mode */
      ret = TAPI_Phone_Set_Linefeed (pChannel,
                                     pChannel->pTapiMeterData->nLastLineMode);
#endif /* TAPI_VERSION3 */
   }

   return ret;
}


/**
   Sets the characteristic for the metering service.

   \param pChannel      Pointer to TAPI_CHANNEL structure.
   \param pMeterConfig  Contains the metering settings (IFX_TAPI_METER_CFG_t)

   \return
   TAPI_statusOk or error code
*/
IFX_int32_t TAPI_Phone_Meter_Config (TAPI_CHANNEL *pChannel,
                                     IFX_TAPI_METER_CFG_t const *pMeterConfig)
{
   IFX_TAPI_DRV_CTX_t* pDrvCtx = pChannel->pTapiDevice->pDevDrvCtx;
   IFX_int32_t ret = TAPI_statusOk;

   if (pChannel->pTapiMeterData == IFX_NULL)
   {
      RETURN_STATUS (TAPI_statusInitMeterFail, 0);
   }
   if (pMeterConfig == IFX_NULL)
   {
      RETURN_STATUS (TAPI_statusParam, 0);
   }

   pChannel->pTapiMeterData->MeterConfig.bMode      = pMeterConfig->bMode;
   pChannel->pTapiMeterData->MeterConfig.nPulseLen  = pMeterConfig->nPulseLen;
   pChannel->pTapiMeterData->MeterConfig.nPauseLen  = pMeterConfig->nPauseLen;

   if (IFX_TAPI_PtrChk (pDrvCtx->ALM.Metering_Cfg))
      ret = pDrvCtx->ALM.Metering_Cfg (pChannel->pLLChannel,
                                       (IFX_TAPI_METER_CFG_t*)pMeterConfig);
   return ret;
}

/**
   Generates a metering burst.

   \param pChannel      Pointer to TAPI_CHANNEL structure.
   \param pMeterBurst   Contains the burst settings (IFX_TAPI_METER_BURST_t)

   \return
   TAPI_statusOk or error code
*/
IFX_int32_t TAPI_Phone_Meter_Burst (TAPI_CHANNEL *pChannel,
                                    IFX_TAPI_METER_BURST_t const *pMeterBurst)
{
   TAPI_METER_DATA_t *pTapiMeterData = pChannel->pTapiMeterData;
   IFX_TAPI_DRV_CTX_t* pDrvCtx = pChannel->pTapiDevice->pDevDrvCtx;
   IFX_int32_t retLL = TAPI_statusOk;

   if (pTapiMeterData == IFX_NULL)
   {
      /* metering not initialised on this channel */
      RETURN_STATUS (TAPI_statusInitMeterFail, 0);
   }

#ifdef TAPI_FEAT_RINGENGINE
   /* check if telephone isn't ringing */
   if (IFX_TAPI_Ring_IsActive (pChannel) == IFX_TRUE)
   {
      /* channel is just ringing */
      RETURN_STATUS (TAPI_statusRingAct, 0);
   }
#endif /* TAPI_FEAT_RINGENGINE */

   /* check if line is in active state */
   if (! (pChannel->TapiOpControlData.nLineMode == IFX_TAPI_LINE_FEED_ACTIVE ||
       pChannel->TapiOpControlData.nLineMode == IFX_TAPI_LINE_FEED_ACTIVE_REV ||
       pChannel->TapiOpControlData.nLineMode == IFX_TAPI_LINE_FEED_NORMAL_AUTO ||
       pChannel->TapiOpControlData.nLineMode == IFX_TAPI_LINE_FEED_REVERSED_AUTO))
   {
      RETURN_STATUS (TAPI_statusLineNotAct, 0);
   }

   /* save line mode */
   pTapiMeterData->nLastLineMode = pChannel->TapiOpControlData.nLineMode;

   /* send first pulse */
   if (IFX_TAPI_PtrChk (pDrvCtx->ALM.Metering_Start))
   {
      retLL = pDrvCtx->ALM.Metering_Start (pChannel->pLLChannel,
                                         pMeterBurst->nPulses);
      if (TAPI_SUCCESS (retLL))
      {
#ifdef TAPI_VERSION3
         pTapiMeterData->bMeterActive = IFX_TRUE;
#endif /* TAPI_VERSION3 */
         pTapiMeterData->nBurstTransmitted++;
         pTapiMeterData->nPulseOutstanding += pTapiMeterData->nPulses;
      }
      else
      {
         /*errmsg: Failed to start metering */
         RETURN_STATUS (TAPI_statusMeterBurstFail, retLL);
      }
   }
   else
   {
      /* metering is already active */
      RETURN_STATUS (TAPI_statusNotSupported, 0);
   }

   RETURN_STATUS (TAPI_statusOk, 0);
}

/**
   Returns metering statistics.

   \param pChannel      Pointer to TAPI_CHANNEL structure.
   \param pMeterBurst   Statistics (IFX_TAPI_METER_STATISTICS_t)

   \return
   TAPI_statusOk or erro code
*/
IFX_int32_t TAPI_Phone_Meter_Stat (TAPI_CHANNEL *pChannel,
                                   IFX_TAPI_METER_STATISTICS_t *pMeterStat)
{
   TAPI_METER_DATA_t *pTapiMeterData = pChannel->pTapiMeterData;

   if (pTapiMeterData == IFX_NULL)
   {
      /* metering not initialised on this channel */
      RETURN_STATUS (TAPI_statusInitMeterFail, 0);
   }
   pMeterStat->nPulseOutstanding = pTapiMeterData->nPulseOutstanding;
   pMeterStat->nBurstTransmitted = pTapiMeterData->nBurstTransmitted;
   pTapiMeterData->nBurstTransmitted = 0;
   return TAPI_statusOk;
}

/**
   Process meter end event.
   Clear Pulses counter for statistics

   \param   pChannel    Pointer to TAPI_CHANNEL structure.
*/
IFX_void_t IFX_TAPI_Meter_EventServe (TAPI_CHANNEL *pChannel)
{
   TAPI_METER_DATA_t *pTapiMeterData = pChannel->pTapiMeterData;

   if (pTapiMeterData != IFX_NULL)
   {
      pTapiMeterData->nPulseOutstanding = 0;
   }
   return;
}

#endif /* TAPI_FEAT_METERING */
