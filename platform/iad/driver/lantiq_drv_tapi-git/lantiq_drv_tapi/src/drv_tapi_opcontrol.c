/******************************************************************************

                            Copyright (c) 2006-2015
                        Lantiq Beteiligungs-GmbH & Co.KG
                             http://www.lantiq.com

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/**
   \file drv_tapi_opcontrol.c
   TAPI Operation Control Services.

   \remarks
   All operations done by functions in this module are phone
   related and assumes a phone channel file descriptor.
   Caller of anyone of the functions must make sure that a phone
   channel is used. In case data channel functions are invoked here,
   an instance of the data channel must be passed.
*/

/* ============================= */
/* Includes                      */
/* ============================= */

#include "drv_tapi.h"
#include "drv_tapi_errno.h"
#include "drv_tapi_ppd.h"

/* ============================= */
/* Local Macros & Definitions    */
/* ============================= */

/* ============================= */
/* Local variable definition     */
/* ============================= */

/* ============================= */
/* Local function declaration    */
/* ============================= */

/* ============================= */
/* Global function definition    */
/* ============================= */

/**
   Reads the hook status from the device

   \param pChannel   - handle to TAPI_CHANNEL structure
   \param pHookMode  - storage variable for hook state

   \return TAPI_statusOk

   \remarks
      pHookMode require different memory size for different driver modes.
      For single device node driver mode require handle to
      the \ref IFX_TAPI_LINE_HOOK_STATUS_GET_t structure.
      For multiple device node driver mode require handle to
      the \ref IFX_int32_t.
*/
IFX_int32_t TAPI_Phone_HookstateGet (TAPI_CHANNEL *pChannel, IFX_TAPI_LINE_HOOK_STATUS_GET_t *pHookMode)
{
   if (pChannel->TapiOpControlData.bHookState)
   {
#ifdef TAPI_ONE_DEVNODE
      pHookMode->hookMode = IFX_TAPI_LINE_OFFHOOK;
#else
      *pHookMode = IFX_TRUE;
#endif /* TAPI_ONE_DEVNODE*/
   }
   else
   {
#ifdef TAPI_ONE_DEVNODE
      pHookMode->hookMode = IFX_TAPI_LINE_ONHOOK;
#else
      *pHookMode = IFX_FALSE;
#endif /* TAPI_ONE_DEVNODE*/
   }

   return (TAPI_statusOk);
}

/**
   Sets the linefeeding mode of the device

   \param pChannel      Pointer to TAPI_CHANNEL structure.
   \param nMode         Line mode.

   \return
   IFX_SUCCESS or IFX_ERROR

   \remarks
   Line mode is ALWAYS set, also if it was set before.
*/
IFX_int32_t TAPI_Phone_Set_Linefeed(TAPI_CHANNEL *pChannel,
                                    IFX_int32_t nMode)
{
   IFX_TAPI_DRV_CTX_t* pDrvCtx = pChannel->pTapiDevice->pDevDrvCtx;
   IFX_boolean_t       bBatSw  = IFX_FALSE;
   IFX_boolean_t       bPol    = IFX_FALSE;
   IFX_int32_t         ret     = TAPI_statusOk;
   IFX_uint8_t         tmp_nLineMode = pChannel->TapiOpControlData.nLineMode;
   IFX_uint8_t         tmp_nBatterySw = pChannel->TapiOpControlData.nBatterySw;
   IFX_uint8_t         tmp_nPolarity = pChannel->TapiOpControlData.nPolarity;
   IFX_uint8_t         nLineMode = (IFX_uint8_t)nMode;

   /* Disallow Standby and Ringing when phone is off-hook. When the last line
      feeding was "disabled" the hook state is undefined so skip this check. */
   if ((tmp_nLineMode != IFX_TAPI_LINE_FEED_DISABLED) &&
       (pChannel->TapiOpControlData.bHookState == IFX_TRUE))
   {
      switch (nLineMode)
      {
         case IFX_TAPI_LINE_FEED_STANDBY:
         case IFX_TAPI_LINE_FEED_RING_BURST:
         case IFX_TAPI_LINE_FEED_RING_PAUSE:
            /* Unsuitable line mode while phone is off-hook */
            RETURN_STATUS (TAPI_statusPhoneOffHook, 0);
         default:
            break;
      }
   }

   /* In emergency shutdown mode only setting of disabled is allowed. */
   if (pChannel->TapiOpControlData.bEmergencyShutdown == IFX_TRUE)
   {
      if (nLineMode == IFX_TAPI_LINE_FEED_DISABLED)
      {
         pChannel->TapiOpControlData.bEmergencyShutdown = IFX_FALSE;
      }
      else
      {
         /* errmsg: Cannot set linefeeding different than disabled while a line
            fault is present. */
         RETURN_STATUS (TAPI_statusLineFault, 0);
      }
   }


#ifdef TAPI_VERSION3
#ifdef TAPI_FEAT_METERING
   if (TAPI_Phone_Meter_IsActive (pChannel))
   {
      /* NOTE: Recursive call
               _Meter_Stop will call _Set_Linefeed to restore
               the line mode before metering */

      /* stop metering before line mode changing */
      TAPI_Phone_Meter_Stop (pChannel);
   }
#endif /* TAPI_FEAT_METERING */
#endif /* TAPI_VERSION3 */

#ifndef TAPI_FEAT_CID
#ifdef TAPI_FEAT_RINGENGINE
   /* This code causes a recursive call ending in a lockup when TAPI CID
      tries to use OSI alerting. */
   if (nLineMode == IFX_TAPI_LINE_FEED_DISABLED)
   {
      /* NOTE:
           Possible recursive calls during linefeed restore procedure.
           No any conflicts expected, because of DISABLED mode never used.
      */
      IFX_TAPI_Ring_Stop (pChannel);
   }
#endif /* TAPI_FEAT_RINGENGINE */
#endif /* TAPI_FEAT_CID */

   /* check if auto battery switch have to enabled */
   if ((nLineMode == IFX_TAPI_LINE_FEED_NORMAL_AUTO    ||
        nLineMode == IFX_TAPI_LINE_FEED_REVERSED_AUTO) &&
       !pChannel->TapiOpControlData.nBatterySw)
   {
      bBatSw = IFX_TRUE;
      pChannel->TapiOpControlData.nBatterySw = 0x01;
   }

   /* check if auto battery switch have to disabled */
   if (!(nLineMode == IFX_TAPI_LINE_FEED_NORMAL_AUTO    ||
         nLineMode == IFX_TAPI_LINE_FEED_REVERSED_AUTO) &&
       pChannel->TapiOpControlData.nBatterySw)
   {
      bBatSw = IFX_TRUE;
      pChannel->TapiOpControlData.nBatterySw = 0x00;
   }

   /* check if polarity has to change */
   if ((nLineMode == IFX_TAPI_LINE_FEED_ACTIVE_REV        ||
        nLineMode == IFX_TAPI_LINE_FEED_REVERSED_AUTO     ||
        nLineMode == IFX_TAPI_LINE_FEED_REVERSED_LOW      ||
        nLineMode == IFX_TAPI_LINE_FEED_ACTIVE_RES_REVERSED) &&
       !pChannel->TapiOpControlData.nPolarity)
   {
      bPol = IFX_TRUE;
      pChannel->TapiOpControlData.nPolarity = 0x01;
   }
   if (!(nLineMode == IFX_TAPI_LINE_FEED_ACTIVE_REV       ||
         nLineMode == IFX_TAPI_LINE_FEED_REVERSED_AUTO    ||
         nLineMode == IFX_TAPI_LINE_FEED_REVERSED_LOW     ||
         nLineMode == IFX_TAPI_LINE_FEED_ACTIVE_RES_REVERSED) &&
       pChannel->TapiOpControlData.nPolarity)
   {
      bPol = IFX_TRUE;
      pChannel->TapiOpControlData.nPolarity = 0x00;
   }

#ifdef TAPI_FEAT_DIAL
   if ((tmp_nLineMode == IFX_TAPI_LINE_FEED_DISABLED) &&
       (nLineMode != IFX_TAPI_LINE_FEED_DISABLED))
   {
      IFX_TAPI_Dial_CfgApply (pChannel);
   }
#endif /* TAPI_FEAT_DIAL */

#ifdef TAPI_FEAT_PHONE_DETECTION
   ret = IFX_TAPI_PPD_HandleLineFeeding(pChannel, &nLineMode);
   if (nLineMode != IFX_TAPI_LINE_FEED_PHONE_DETECT)
   {
      /* call low level function to change operation mode */
      if (IFX_TAPI_PtrChk (pDrvCtx->ALM.Line_Mode_Set))
      {
          ret = pDrvCtx->ALM.Line_Mode_Set(pChannel->pLLChannel,
                                 nLineMode, tmp_nLineMode);
      }
   }
#else
   if (nLineMode == IFX_TAPI_LINE_FEED_PHONE_DETECT)
   {
      nLineMode = IFX_TAPI_LINE_FEED_STANDBY;
      TRACE(TAPI_DRV, DBG_LEVEL_LOW,
           ("\nDRV_WARNING: Phone Detection is not supported. "
            "STANDBY used instead of DETECT (ch %d).\n",
            pChannel->nChannel));
   }

   /* call low level function to change operation mode */
   if (IFX_TAPI_PtrChk (pDrvCtx->ALM.Line_Mode_Set))
   {
       ret = pDrvCtx->ALM.Line_Mode_Set(pChannel->pLLChannel,
                              nLineMode, tmp_nLineMode);
   }
#endif /* TAPI_FEAT_PHONE_DETECTION */

   if (!TAPI_SUCCESS(ret))
   {
      /* restore the old value, because the configuration on the driver failed */
      pChannel->TapiOpControlData.nLineMode = tmp_nLineMode;
#ifdef TAPI_FEAT_PHONE_DETECTION
      (void)IFX_TAPI_PPD_HandleLineFeeding(pChannel, &tmp_nLineMode);
#endif /* TAPI_FEAT_PHONE_DETECTION */
      RETURN_STATUS (TAPI_statusLineModeFail, ret);
   }

   /* save current linemode in tapi structure */
   pChannel->TapiOpControlData.nLineMode = nLineMode;

   /* switch polarity only when it is necessary */
   if (bPol && (ret == TAPI_statusOk))
   {
      if (IFX_TAPI_PtrChk (pDrvCtx->ALM.Line_Polarity_Set))
         ret = pDrvCtx->ALM.Line_Polarity_Set(pChannel->pLLChannel);

      if (!TAPI_SUCCESS(ret))
      {
         /* restore the old value, because the configuration on the driver failed */
         pChannel->TapiOpControlData.nPolarity = tmp_nPolarity;
      }
   }

   /* enable / disable automatic battery switch */
   if (bBatSw && (ret == TAPI_statusOk))
   {
      if (IFX_TAPI_PtrChk (pDrvCtx->ALM.AutoBatterySwitch))
         ret = pDrvCtx->ALM.AutoBatterySwitch(pChannel->pLLChannel);

      if (!TAPI_SUCCESS(ret))
      {
         /* restore the old value, because the configuration on the driver failed */
         pChannel->TapiOpControlData.nBatterySw = tmp_nBatterySw;
      }
   }
   return ret;
}

/**
   Changes the linefeeding mode of the device

   \param pChannel      Pointer to TAPI_CHANNEL structure.
   \param nMode         Line mode.

*/
IFX_void_t TAPI_Phone_Change_Linefeed(TAPI_CHANNEL *pChannel,
                                      IFX_int32_t nMode)
{
   switch (pChannel->TapiOpControlData.nLineMode)
   {
      case IFX_TAPI_LINE_FEED_ACTIVE:
      case IFX_TAPI_LINE_FEED_RING_PAUSE:
      case IFX_TAPI_LINE_FEED_NORMAL_AUTO:
      case IFX_TAPI_LINE_FEED_ACTIVE_LOW:
      case IFX_TAPI_LINE_FEED_ACTIVE_BOOSTED:
      case IFX_TAPI_LINE_FEED_ACTIVE_REV:
      case IFX_TAPI_LINE_FEED_REVERSED_AUTO:
         if (nMode != IFX_TAPI_LINE_FEED_ACTIVE)
         {
            pChannel->TapiOpControlData.nLineMode = nMode;
         }
         break;
      case IFX_TAPI_LINE_FEED_STANDBY:
      case IFX_TAPI_LINE_FEED_PARKED_REVERSED:
         if (nMode != IFX_TAPI_LINE_FEED_STANDBY)
         {
            pChannel->TapiOpControlData.nLineMode = nMode;
         }
         break;
      case IFX_TAPI_LINE_FEED_DISABLED:
         pChannel->TapiOpControlData.nLineMode = nMode;
         break;
      case IFX_TAPI_LINE_FEED_RING_BURST:
         pChannel->TapiOpControlData.nLineMode = nMode;
         break;
      case IFX_TAPI_LINE_FEED_NLT:
         pChannel->TapiOpControlData.nLineMode = nMode;
         break;
      default:
         break;
   }
}


/**
   Restore the linefeeding mode of the device to the last requested mode

   \param pChannel      Pointer to TAPI_CHANNEL structure.
*/
IFX_void_t TAPI_Phone_Linefeed_Restore(TAPI_CHANNEL *pChannel)
{
   IFX_TAPI_DRV_CTX_t* pDrvCtx = pChannel->pTapiDevice->pDevDrvCtx;
   IFX_int32_t         ret     = TAPI_statusOk;

   IFX_uint8_t         nLineMode = pChannel->TapiOpControlData.nLineMode;

   /* While in emergency shutdown do not try to restore the line feed. */
   if (pChannel->TapiOpControlData.bEmergencyShutdown == IFX_TRUE)
      return;

#ifdef TAPI_FEAT_PHONE_DETECTION
   ret = IFX_TAPI_PPD_HandleLineFeeding(pChannel, &nLineMode);
   if (nLineMode != IFX_TAPI_LINE_FEED_PHONE_DETECT)
#endif /* TAPI_FEAT_PHONE_DETECTION */
   {
      /* call low level function to change operation mode */
      if (IFX_TAPI_PtrChk (pDrvCtx->ALM.Line_Mode_Set))
      {
          ret = pDrvCtx->ALM.Line_Mode_Set(pChannel->pLLChannel,
                                           nLineMode, nLineMode);
      }
   }

   if (!TAPI_SUCCESS(ret))
   {
      TRACE(TAPI_DRV, DBG_LEVEL_LOW,
            ("TAPI_ERROR: Linefeed restore failed %X\n", ret));
   }
}


/**
   Gets the linefeeding mode of the device

   \param pChannel      Pointer to TAPI_CHANNEL structure.
   \param nMode         Line mode.
*/
IFX_void_t TAPI_Phone_Linefeed_Get(TAPI_CHANNEL *pChannel,
                                   IFX_uint8_t *nLineMode)
{
   *nLineMode = pChannel->TapiOpControlData.nLineMode;
}


/**
   Sets the line type mode of the specific analog channel

   \param pChannel      Pointer to TAPI_CHANNEL structure.
   \param pCfg          Pointer to struct IFX_TAPI_LINE_TYPE_CFG_t.

   \return TAPI_statusOk or TAPI_statusErr

   \remarks Line mode is ALWAYS set, also if it was set before. By default
           FXS type is used.
*/
IFX_int32_t TAPI_Phone_Set_LineType(
      TAPI_CHANNEL *pChannel,
      IFX_TAPI_LINE_TYPE_CFG_t const *pCfg
   )
{
   IFX_TAPI_DRV_CTX_t *pDrvCtx = pChannel->pTapiDevice->pDevDrvCtx;
   IFX_int32_t ret = TAPI_statusOk,
               retLL = TAPI_statusOk;

   /* call low level function to change operation mode */
   if (IFX_TAPI_PtrChk (pDrvCtx->ALM.Line_Type_Set))
   {
       retLL = pDrvCtx->ALM.Line_Type_Set(pChannel->pLLChannel, pCfg->lineType);
   }

   if (TAPI_SUCCESS(retLL))
   {
      /* Save new type and DAA number */
      pChannel->TapiOpControlData.nLineType = pCfg->lineType;
      pChannel->TapiOpControlData.nDAA      = pCfg->nDaaCh;
#ifdef TAPI_FEAT_FXO
      /* initialize DAA only for non-SmartSLIC based systems */
      if (pCfg->lineType == IFX_TAPI_LINE_TYPE_FXO &&
          pChannel->pTapiDevice->bSmartSlicFxo != IFX_TRUE)
      {
         ret = TAPI_FXO_Register_DAA(pChannel, pChannel->TapiOpControlData.nDAA);
         /* init the DAA channel */
         if ((TAPI_SUCCESS(ret)) &&
             (! pChannel->TapiOpControlData.bDaaInitialized))
         {
            ret = TAPI_FXO_Init_DAA (pChannel);
            if (TAPI_SUCCESS(ret))
            {
               pChannel->TapiOpControlData.bDaaInitialized = IFX_TRUE;
            }
         }
      }
      else
      {
         ret = TAPI_FXO_Register_DAA (IFX_NULL, pChannel->TapiOpControlData.nDAA);
      }
#ifdef TAPI_FEAT_PHONE_DETECTION
      if (pCfg->lineType == IFX_TAPI_LINE_TYPE_FXO)
      {
         /* Remove the FXS Phone Detection state machine for this channel
            if it was started. */
         IFX_TAPI_PPD_Cleanup(pChannel);
      }
      else if(!((pCfg->lineType == IFX_TAPI_LINE_TYPE_FXS_NB) ||
              (pCfg->lineType == IFX_TAPI_LINE_TYPE_FXS_WB) ||
              (pCfg->lineType == IFX_TAPI_LINE_TYPE_FXS_AUTO)
              ))
      {
         /* Init the FXS Phone Detection state machine for this
            channel. */
         ret = IFX_TAPI_PPD_Initialise_Unprot(pChannel);
      }
#endif /* TAPI_FEAT_PHONE_DETECTION */
#else
      RETURN_STATUS (TAPI_statusNotSupported, 0);
#endif /* TAPI_FEAT_FXO */
   }
   else
   {
      /* errmsg: LL driver returned an error */
      ret = TAPI_statusLLFailed;
   }

   RETURN_STATUS (ret, retLL);
}


#ifdef TAPI_FEAT_SSLIC_RECOVERY
/**
   Handle SmartSLIC fault and recovery.

   \param pChannel      Pointer to TAPI_CHANNEL structure.
   \param bCrashed      Current SSI bus status.
                        IFX_TRUE = crashed, IFX_FALSE = recovered.
*/
IFX_void_t TAPI_Phone_SlicFaultHandling(
                        TAPI_DEV *pTapiDev,
                        IFX_boolean_t bCrashed)
{
   if (bCrashed == IFX_TRUE)
   {
      /* SSI crash occured */
      TAPI_SetTime_Timer (pTapiDev->SlicFaultTimerID,
         TAPI_SLIC_FAULT_TIMEOUT, IFX_FALSE, IFX_FALSE);
   }
   else
   {
      /* Recovered from SSI crash */
      IFX_TAPI_DRV_CTX_t* pDrvCtx = pTapiDev->pDevDrvCtx;

      /* Call low level function to bring the SLIC back into operation.
         This acknowledges the error and restores the linefeed to the
         previous state. If the function does not exist the timer is
         kept running to finally report an SSI crash to the application. */
      if (IFX_TAPI_PtrChk (pDrvCtx->ALM.SlicRecovery))
      {
         TAPI_Stop_Timer (pTapiDev->SlicFaultTimerID);
         pDrvCtx->ALM.SlicRecovery(pTapiDev->pLLDev);
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
               ("SSI-bus crash recovered\n"));
      }
   }
}
#endif /* TAPI_FEAT_SSLIC_RECOVERY */


#ifdef TAPI_FEAT_SSLIC_RECOVERY
/**
   SmartSLIC fault supervision timer callback function.

   When a SLIC fault was not recovered during the timeout report this with
   a TAPI event to the application. The application should then do the next
   level of recovery by clearing all the calls on this device and restarting
   it.

   \param  Timer        TimerID of timer that exipres.
   \param  nArg         Argument of timer. This argument is a pointer to the
                        TAPI_DEV structure.
*/
IFX_void_t TAPI_Phone_SlicFaultOnTimer(
                        Timer_ID Timer,
                        IFX_ulong_t nArg)
{
   TAPI_DEV *pTapiDev = (TAPI_DEV *)nArg;
   IFX_TAPI_DRV_CTX_t *pDrvCtx = pTapiDev->pDevDrvCtx;
   IFX_TAPI_EVENT_t tapiEvent;
   IFX_int32_t retLL = TAPI_statusOk;

   IFX_UNUSED(Timer);

   if (IFX_TAPI_PtrChk (pDrvCtx->ALM.Dbg_SSICrash_Handler))
   {
      struct IFX_TAPI_DBG_SSI_CRASH data;
      memset(&data, 0, sizeof(data));

      retLL = pDrvCtx->ALM.Dbg_SSICrash_Handler(pTapiDev->pLLDev, &data);
      if (retLL == TAPI_statusOk)
      {
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
               ("!!! SSI-bus crash dump 0x%08X 0x%08X 0x%08X\n",
               data.cause[0], data.cause[1], data.cause[2]));
      }
      else
      {
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
               ("!!! Reading SSI-bus crash-cause failed: %d\n", retLL));
      }
   }

   /* Report SSI crash as TAPI event to the application.
      The event is device global and always sent on channel 0. */
   memset(&tapiEvent, 0, sizeof(tapiEvent));
   tapiEvent.id = IFX_TAPI_EVENT_FAULT_HW_SSI_ERR;
   IFX_TAPI_Event_Dispatch(pTapiDev->pChannel, &tapiEvent);

   return;
}
#endif /* TAPI_FEAT_SSLIC_RECOVERY */


#ifdef TAPI_FEAT_ALM_LEC
/**
   Sets the LEC operating mode

   \param pChannel      Pointer to TAPI_CHANNEL structure.
   \param pLecConf      Pointer to IFX_TAPI_WLEC_CFG_t structure.

   \return
   TAPI_statusOk or TAPI_statusErr
*/
IFX_int32_t TAPI_Phone_LecMode_Alm_Set (TAPI_CHANNEL *pChannel,
                                        IFX_TAPI_WLEC_CFG_t const *pLecConf)
{
   IFX_TAPI_DRV_CTX_t *pDrvCtx = pChannel->pTapiDevice->pDevDrvCtx;
   TAPI_LEC_DATA_t oLecData;
   IFX_int32_t     retLL;

   /* do not touch anything if LL does not support LEC */
   if (!IFX_TAPI_PtrChk (pDrvCtx->ALM.Lec_Cfg))
   {
      /* LEC not supported by LL driver */
      RETURN_STATUS (TAPI_statusLLNotSupp, 0);
   }

   /* make a copy of the current configuration */
   memcpy(&oLecData, &pChannel->TapiLecAlmData, sizeof(TAPI_LEC_DATA_t));

   /* assemble configuration data structure */
   oLecData.nOpMode     = pLecConf->nType;
   oLecData.bNlp        = pLecConf->bNlp;
   oLecData.nNBNEwindow = pLecConf->nNBNEwindow;
   oLecData.nNBFEwindow = pLecConf->nNBFEwindow;
   oLecData.nWBNEwindow = pLecConf->nWBNEwindow;

   /* call low level for settings */
   retLL = pDrvCtx->ALM.Lec_Cfg (pChannel->pLLChannel, &oLecData);
   if (!TAPI_SUCCESS (retLL))
   {
      /* errmsg: LL driver returned an error */
      RETURN_STATUS (TAPI_statusLLFailed, retLL);
   }

   /* begin of protected area */
   TAPI_OS_MutexGet (&pChannel->semTapiChDataLock);
   /* save configuration */
   memcpy(&pChannel->TapiLecAlmData, &oLecData, sizeof(TAPI_LEC_DATA_t));
   /* end of protected area */
   TAPI_OS_MutexRelease (&pChannel->semTapiChDataLock);

   return TAPI_statusOk;
}
#endif /* TAPI_FEAT_ALM_LEC */


#ifdef TAPI_FEAT_ALM_LEC
/**
   Get the current LEC operating mode

   \param pChannel      Pointer to TAPI_CHANNEL structure.
   \param pWLecConf     Pointer to IFX_TAPI_WLEC_CFG_t structure.

   \return
   TAPI_statusOk or TAPI_statusErr
*/
IFX_int32_t TAPI_Phone_LecMode_Alm_Get (TAPI_CHANNEL *pChannel,
                                        IFX_TAPI_WLEC_CFG_t *pWLecConf)
{
   TAPI_LEC_DATA_t *pLecData = &pChannel->TapiLecAlmData;

   /* begin of protected area */
   TAPI_OS_MutexGet (&pChannel->semTapiChDataLock);

   /* read config from channel structure */
   pWLecConf->nType = pLecData->nOpMode;
   pWLecConf->bNlp  = (IFX_TAPI_WLEC_NLP_t)pLecData->bNlp;
   pWLecConf->nNBNEwindow = pLecData->nNBNEwindow;
   pWLecConf->nNBFEwindow = pLecData->nNBFEwindow;
   pWLecConf->nWBNEwindow = pLecData->nWBNEwindow;

   /* end of protected area */
   TAPI_OS_MutexRelease (&pChannel->semTapiChDataLock);

   return TAPI_statusOk;
}
#endif /* TAPI_FEAT_ALM_LEC */


#ifdef TAPI_FEAT_PCM_LEC
/**
   Sets the LEC operating mode

   \param pChannel      Pointer to TAPI_CHANNEL structure.
   \param pLecConf      Pointer to IFX_TAPI_WLEC_CFG_t structure.

   \return
   TAPI_statusOk or TAPI_statusErr
*/
IFX_int32_t TAPI_Phone_LecMode_Pcm_Set (TAPI_CHANNEL *pChannel,
                                        IFX_TAPI_WLEC_CFG_t const *pLecConf)
{
   IFX_TAPI_DRV_CTX_t *pDrvCtx = pChannel->pTapiDevice->pDevDrvCtx;
   TAPI_LEC_DATA_t oLecData;
   IFX_int32_t     retLL;

   /* do not touch anything if LL does not support LEC */
   if (!IFX_TAPI_PtrChk (pDrvCtx->PCM.Lec_Cfg))
   {
      /* LEC not supported by LL driver */
      RETURN_STATUS (TAPI_statusLLNotSupp, 0);
   }

   /* make a copy of the current configuration */
   memcpy(&oLecData, &pChannel->TapiLecPcmData, sizeof(TAPI_LEC_DATA_t));

   /* assemble configuration data structure */
   oLecData.nOpMode     = pLecConf->nType;
   oLecData.bNlp        = pLecConf->bNlp;
   oLecData.nNBNEwindow = pLecConf->nNBNEwindow;
   oLecData.nNBFEwindow = pLecConf->nNBFEwindow;
   oLecData.nWBNEwindow = pLecConf->nWBNEwindow;

   /* call low level for settings */
   retLL = pDrvCtx->PCM.Lec_Cfg (pChannel->pLLChannel, &oLecData);

   if (!TAPI_SUCCESS (retLL))
   {
      /* errmsg: LL driver returned an error */
      RETURN_STATUS (TAPI_statusLLFailed, retLL);
   }

   /* begin of protected area */
   TAPI_OS_MutexGet (&pChannel->semTapiChDataLock);
   /* save configuration */
   memcpy(&pChannel->TapiLecPcmData, &oLecData, sizeof(TAPI_LEC_DATA_t));
   /* end of protected area */
   TAPI_OS_MutexRelease (&pChannel->semTapiChDataLock);

   return TAPI_statusOk;
}
#endif /* TAPI_FEAT_PCM_LEC */


#ifdef TAPI_FEAT_PCM_LEC
/**
   Get the current LEC operating mode

   \param pChannel      Pointer to TAPI_CHANNEL structure.
   \param pLecConf      Pointer to IFX_TAPI_WLEC_CFG_t structure.

   \return
   TAPI_statusOk or TAPI_statusErr
*/
IFX_int32_t TAPI_Phone_LecMode_Pcm_Get (TAPI_CHANNEL *pChannel,
                                        IFX_TAPI_WLEC_CFG_t *pLecConf)
{
   TAPI_LEC_DATA_t *pLecData = &pChannel->TapiLecPcmData;

   /* begin of protected area */
   TAPI_OS_MutexGet (&pChannel->semTapiChDataLock);

   /* read config from channel structure */
   pLecConf->bNlp  = (IFX_TAPI_WLEC_NLP_t)pLecData->bNlp;
   pLecConf->nType = pLecData->nOpMode;
   pLecConf->nNBNEwindow = pLecData->nNBNEwindow;
   pLecConf->nNBFEwindow = pLecData->nNBFEwindow;
   pLecConf->nWBNEwindow = pLecData->nWBNEwindow;

   /* end of protected area */
   TAPI_OS_MutexRelease (&pChannel->semTapiChDataLock);

   return TAPI_statusOk;
}
#endif /* TAPI_FEAT_PCM_LEC */


#ifdef TAPI_FEAT_DTMF
/**
   Gets the DTMF Receiver Coefficients

   \param pChannel        - handle to TAPI_CHANNEL structure
   \param pCoeff          - handle to IFX_TAPI_DTMF_RX_CFG_t structure

   \return
   TAPI_statusOk or TAPI_statusErr
*/
IFX_int32_t TAPI_Phone_DTMFR_Cfg_Get (TAPI_CHANNEL *pChannel,
   IFX_TAPI_DTMF_RX_CFG_t *pCoeff)
{
   IFX_TAPI_DRV_CTX_t *pDrvCtx = pChannel->pTapiDevice->pDevDrvCtx;
   IFX_int32_t retLL;

   /* do not touch anything if LL device DTMF Rx coefficients
      not configurable */
   if (!IFX_TAPI_PtrChk (pDrvCtx->SIG.DTMF_RxCoeff))
   {
      /* DTMF Rx coefficients not configurable on LL device */
      RETURN_STATUS (TAPI_statusLLNotSupp, 0);
   }

   /* call low level routine to read the settings */
   retLL = pDrvCtx->SIG.DTMF_RxCoeff (pChannel->pLLChannel, IFX_TRUE, pCoeff);
   if (!(TAPI_SUCCESS (retLL)))
   {
      RETURN_STATUS (TAPI_statusDtmfRxCfg, retLL);
   }

   return TAPI_statusOk;
}
#endif /* TAPI_FEAT_DTMF */


#ifdef TAPI_FEAT_DTMF
/**
   Sets the DTMF Receiver Coefficients

   \param pChannel        - handle to TAPI_CHANNEL structure
   \param pCoeff          - handle to IFX_TAPI_DTMF_RX_CFG_t structure

   \return
   TAPI_statusOk or TAPI_statusErr
*/
IFX_int32_t TAPI_Phone_DTMFR_Cfg_Set (TAPI_CHANNEL *pChannel,
   IFX_TAPI_DTMF_RX_CFG_t const *pCoeff)
{
   IFX_TAPI_DRV_CTX_t *pDrvCtx = pChannel->pTapiDevice->pDevDrvCtx;
   IFX_int32_t ret;
   IFX_TAPI_DTMF_RX_CFG_t coeff;

   /* do not touch anything if LL device DTMF Rx coefficients
      not configurable */
   if (!IFX_TAPI_PtrChk (pDrvCtx->SIG.DTMF_RxCoeff))
   {
      /* DTMF Rx coefficients not configurable on LL device */
      RETURN_STATUS (TAPI_statusLLNotSupp, 0);
   }

   memcpy (&coeff, pCoeff, sizeof (coeff));

   /* call low level routine to write the settings */
   ret = pDrvCtx->SIG.DTMF_RxCoeff (pChannel->pLLChannel, IFX_FALSE, &coeff);
   if (!(TAPI_SUCCESS (ret)))
   {
      RETURN_STATUS (TAPI_statusDtmfRxCfg, ret);
   }

   return ret;
}
#endif /* TAPI_FEAT_DTMF */


/**
   Read out the analog line battery voltage configuration.

   \param   pChannel Handle to the TAPI channel
   \param   pVBat    Handle to the memory for result

   \return
      TAPI_statusOk - success
      TAPI_statusLLNotSupp - service not supported by low-level driver
      TAPI_statusLLFailed - failed low-level call

   \remarks
      pVBat require different memory size for different driver modes.
      For single device node driver mode require handle to
      the \ref IFX_TAPI_LINE_BATTERY_VOLTAGE_t structure.
      For multiple device node driver mode require handle to
      the \ref IFX_int32_t.
*/
IFX_int32_t TAPI_LineBatteryVoltageGet (TAPI_CHANNEL *pChannel,
   IFX_TAPI_LINE_BATTERY_VOLTAGE_t *pVBat)
{
   IFX_TAPI_DRV_CTX_t *pDrvCtx = pChannel->pTapiDevice->pDevDrvCtx;
   IFX_int32_t retLL;
   IFX_int16_t nVBat;

   /* Check for LL driver functionality */
   if (!IFX_TAPI_PtrChk (pDrvCtx->ALM.FeedingVoltageGet))
   {
      RETURN_STATUS (TAPI_statusLLNotSupp, 0);
   }

   retLL = pDrvCtx->ALM.FeedingVoltageGet (pChannel->pLLChannel, &nVBat);
   if (!TAPI_SUCCESS(retLL))
   {
      RETURN_STATUS (TAPI_statusLLFailed, retLL);
   }

#ifdef TAPI_ONE_DEVNODE
   pVBat->nVBat = nVBat;
#else
   *pVBat = nVBat;
#endif /* TAPI_ONE_DEVNODE */

   return TAPI_statusOk;
}

/**
   Write the analog line battery voltage configuration.

   \param   pChannel Handle to the TAPI channel
   \param   nVBat    Battery voltage value

   \return
      TAPI_statusOk - success
      TAPI_statusLLNotSupp - service not supported by low-level driver
      TAPI_statusLLFailed - failed low-level call
*/
IFX_int32_t TAPI_LineBatteryVoltageSet (TAPI_CHANNEL *pChannel,
   IFX_int32_t nVBat)
{
   IFX_TAPI_DRV_CTX_t *pDrvCtx = pChannel->pTapiDevice->pDevDrvCtx;
   IFX_int32_t retLL;

   /* Check for LL driver functionality */
   if (!IFX_TAPI_PtrChk (pDrvCtx->ALM.FeedingVoltageSet))
   {
      RETURN_STATUS (TAPI_statusLLNotSupp, 0);
   }

   retLL = pDrvCtx->ALM.FeedingVoltageSet (pChannel->pLLChannel,
      (IFX_int16_t)nVBat);
   if (!TAPI_SUCCESS(retLL))
   {
      RETURN_STATUS (TAPI_statusLLFailed, retLL);
   }

   return TAPI_statusOk;
}
