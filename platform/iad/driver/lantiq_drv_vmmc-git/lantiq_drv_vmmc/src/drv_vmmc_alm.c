/******************************************************************************

  Copyright (c) 2013 Lantiq Deutschland GmbH
  Copyright 2016, Intel Corporation.

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/**
   \file drv_vmmc_alm.c Implementation of the ALM module.
*/

/* ============================= */
/* Includes                      */
/* ============================= */
#include "drv_api.h"
#include "drv_vmmc_api.h"
#include "drv_vmmc_alm_priv.h"
#include "drv_vmmc_alm.h"
#include "drv_vmmc_con.h"
#include "drv_mps_vmmc.h"
#ifdef VMMC_FEAT_GR909
#include "drv_vmmc_gr909.h"
#endif

#include "ifxos_time.h"

/* ============================= */
/* Global variables              */
/* ============================= */

/* default line types for ALM module */
static VMMC_ALM_LINE_TYPE_t vmmc_alm_line[MAX_ALM_NUM] =
   {VMMC_ALM_LINE_FXS, VMMC_ALM_LINE_FXS, VMMC_ALM_LINE_FXO};

/* ============================= */
/* Local Macros & Definitions    */
/* ============================= */

/* Limits for useful values returned by the calibration process. */
#define VMMC_SDD_CALIBRATE_TXOFFSET_MAX (+983)
#define VMMC_SDD_CALIBRATE_TXOFFSET_MIN (-983)
#define VMMC_SDD_CALIBRATE_MEOFFSET_MAX (+1311)
#define VMMC_SDD_CALIBRATE_MEOFFSET_MIN (-1311)
#define VMMC_SDD_CALIBRATE_ULIMOFFSET30_MAX (+683)
#define VMMC_SDD_CALIBRATE_ULIMOFFSET30_MIN (-683)
#define VMMC_SDD_CALIBRATE_ULIMOFFSET60_MAX (+683)
#define VMMC_SDD_CALIBRATE_ULIMOFFSET60_MIN (-683)
#define VMMC_SDD_CALIBRATE_IDACGAIN_MAX (+32183)
#define VMMC_SDD_CALIBRATE_IDACGAIN_MIN (+26331)
#define VMMC_SDD_CALIBRATE_VDCDC_MIN (+13926)
#define VMMC_SDD_CALIBRATE_VDCDC_MAX (+18842)

/* ============================= */
/* Local variables and types     */
/* ============================= */

/* ============================= */
/* Local function declaration    */
/* ============================= */
#ifdef VMMC_FEAT_CALIBRATION
static IFX_int32_t vmmc_alm_Calibration_Get (VMMC_CHANNEL *pCh,
                        IFX_TAPI_CALIBRATION_CFG_t *pClbConfig);
#if 0
static IFX_int32_t vmmc_alm_Calibration_Set (VMMC_CHANNEL *pCh,
                        IFX_TAPI_CALIBRATION_CFG_t *pClbConfig);
#endif
#endif /* VMMC_FEAT_CALIBRATION */

static IFX_int32_t vmmc_alm_Calibration_Start (
                        VMMC_CHANNEL *pCh,
                        IFX_boolean_t bInternal);

#ifdef VMMC_FEAT_CALIBRATION
static IFX_int32_t vmmc_alm_Calibration_Stop (VMMC_CHANNEL *pCh);

static IFX_int32_t VMMC_TAPI_LL_ALM_Calibration_Results (
                        IFX_TAPI_LL_CH_t *pLLChannel,
                        IFX_TAPI_CALIBRATION_CFG_t *pClbConfig);

static IFX_int32_t VMMC_TAPI_LL_ALM_Calibration_Get (
                        IFX_TAPI_LL_CH_t *pLLChannel,
                        IFX_TAPI_CALIBRATION_CFG_t *pClbConfig);

static IFX_int32_t VMMC_TAPI_LL_ALM_Calibration_Start (
                        IFX_TAPI_LL_CH_t *pLLChannel);

static IFX_int32_t VMMC_TAPI_LL_ALM_Calibration_Stop (
                        IFX_TAPI_LL_CH_t *pLLChannel);
#endif /* VMMC_FEAT_CALIBRATION */

static IFX_int32_t VMMC_TAPI_LL_ALM_Calibration_Finish (
                        IFX_TAPI_LL_CH_t *pLLChannel);

static IFX_int32_t VMMC_TAPI_LL_ALM_Line_Mode_Set (
                        IFX_TAPI_LL_CH_t *pLLChannel,
                        IFX_int32_t nMode,
                        IFX_uint8_t nTapiLineMode);

static IFX_int32_t VMMC_TAPI_LL_ALM_Line_Mode_Get (
                        IFX_TAPI_LL_CH_t *pLLChannel,
                        IFX_TAPI_LINE_FEED_t *pFeed);

static IFX_int32_t VMMC_TAPI_LL_ALM_Line_Type_Set (
                        IFX_TAPI_LL_CH_t *pLLChannel,
                        IFX_TAPI_LINE_TYPE_t nType);

static IFX_int32_t VMMC_TAPI_LL_ALM_VMMC_Test_HookGen (
                        IFX_TAPI_LL_CH_t *pLLChannel,
                        IFX_boolean_t bHook);

static IFX_int32_t VMMC_TAPI_LL_ALM_VMMC_Test_Loop (
                        IFX_TAPI_LL_CH_t *pLLChannel,
                        IFX_TAPI_TEST_LOOP_t const* pLoop);

static IFX_int32_t VMMC_TAPI_LL_ALM_Volume_Set (
                        IFX_TAPI_LL_CH_t *pLLChannel,
                        IFX_TAPI_LINE_VOLUME_t const *pVol);

static IFX_int32_t VMMC_TAPI_LL_ALM_Volume_High_Level (
                        IFX_TAPI_LL_CH_t *pLLCh,
                        IFX_int32_t bEnable);

static IFX_int32_t VMMC_TAPI_LL_ALM_LEC_Set (
                        IFX_TAPI_LL_CH_t *pLLChannel,
                        TAPI_LEC_DATA_t *pLecConf);

static IFX_int32_t vmmc_alm_Echo_Suppressor_Set (
                        IFX_TAPI_LL_CH_t *pLLCh,
                        IFX_enDis_t nEnable,
                        IFX_enDis_t nActiveNLP);

#ifdef VMMC_FEAT_CONT_MEASUREMENT
static IFX_int32_t VMMC_TAPI_LL_ALM_ContMeas_Req (
                        IFX_TAPI_LL_CH_t *pLLChannel);

static IFX_int32_t VMMC_TAPI_LL_ALM_ContMeas_Reset(
                        IFX_TAPI_LL_CH_t *pLLChannel);

static IFX_int32_t VMMC_TAPI_LL_ALM_ContMeas_Get(
                        IFX_TAPI_LL_CH_t *pLLChannel,
                        IFX_TAPI_CONTMEASUREMENT_GET_t *pContMeas);
#endif /* VMMC_FEAT_CONT_MEASUREMENT */

#ifdef VMMC_FEAT_CAP_MEASURE
static IFX_int32_t VMMC_TAPI_LL_ALM_CapMeasStart(
                        IFX_TAPI_LL_CH_t *pLLChannel,
                        IFX_boolean_t bTip2RingOnly);

static IFX_int32_t VMMC_TAPI_LL_ALM_CapMeasStop(
                        IFX_TAPI_LL_CH_t *pLLChannel);

static IFX_int32_t VMMC_TAPI_LL_ALM_CapMeasResult(
                        IFX_TAPI_LL_CH_t *pLLChannel);
#endif /* VMMC_FEAT_CAP_MEASURE */

#ifdef VMMC_FEAT_NLT
static IFX_int32_t VMMC_TAPI_LL_ALM_NLT_Cap_Result(
                        IFX_TAPI_LL_CH_t *pLLChannel,
                        IFX_TAPI_NLT_CAPACITANCE_RESULT_t *pResult);

static IFX_int32_t VMMC_TAPI_LL_ALM_NLT_OLConfig_Set(
                        IFX_TAPI_LL_CH_t *pLLChannel,
                        IFX_TAPI_NLT_CONFIGURATION_OL_t *pConfig);

static IFX_int32_t VMMC_TAPI_LL_ALM_NLT_OLConfig_Get(
                        IFX_TAPI_LL_CH_t *pLLChannel,
                        IFX_TAPI_NLT_CONFIGURATION_OL_t *pConfig);
#endif /* (VMMC_FEAT_NLT) */

static IFX_void_t  vmmc_alm_HookWindow_OnTimer(
                        Timer_ID Timer,
                        IFX_ulong_t arg);

static IFX_int32_t VMMC_TAPI_LL_ALM_CfgDupTimer_Override (
                        IFX_TAPI_LL_CH_t *pLLChannel,
                        IFX_uint8_t nStandbyTime);

#ifdef VMMC_FEAT_SLIC
static IFX_int32_t VMMC_TAPI_LL_ALM_SlicRecovery (
                        IFX_TAPI_LL_DEV_t *pLLDev);

static IFX_int32_t VMMC_TAPI_LL_ALM_SSICrashHandler (
                        IFX_TAPI_LL_DEV_t *pLLDev,
                        struct IFX_TAPI_DBG_SSI_CRASH *pData);
#endif /* VMMC_FEAT_SLIC */

extern IFX_int32_t VMMC_WaitForSddOpmodeChEvt(
                        VMMC_CHANNEL *pCh);

extern IFX_uint32_t VMMC_OS_GetTimeDelta(
                        IFX_uint32_t last_timestamp,
                        IFX_uint32_t *current_timestamp);


/* ============================= */
/* Global function declaration   */
/* ============================= */

/* ============================= */
/* Function definitions          */
/* ============================= */

/**
   Disables the line in case of an emergency.

   This function is used directly from interrupt context to disable the line
   feeding in case of ground fault or SLIC overtemperature. It is also used
   during recovery after an SSI crash to get out of the emergency shutdown.

   \param  pCh             Pointer to the VMMC channel structure.

   \remarks
   This function is called from interrupt context and is thus not protected.
*/
IFX_void_t irq_VMMC_ALM_LineDisable (VMMC_CHANNEL *pCh)
{
   VMMC_SDD_Opmode_t *pOpmod = &pCh->pALM->fw_sdd_opmode;

   pOpmod->OperatingMode = VMMC_SDD_OPMODE_DISABLED;
   pOpmod->Polarity = VMMC_OPMOD_POL_NORMAL;
   pOpmod->Howler = VMMC_OPMOD_HOWLER_OFF;
   pOpmod->MwiLamp = VMMC_OPMOD_MWI_LAMP_OFF;
   pOpmod->TtxBurst = VMMC_OPMOD_TTX_BURST_OFF;

   CmdWriteIsr (pCh->pParent, (IFX_uint32_t *)pOpmod, SDD_Opmode_LEN);

   pCh->pALM->curr_opmode = pOpmod->OperatingMode;
   pCh->pALM->curr_polarity = pOpmod->Polarity;
   pCh->pALM->curr_howler = pOpmod->Howler;
   pCh->pALM->curr_mwi_lamp = pOpmod->MwiLamp;
}

/**
   Updates current line operating mode.

   This function is used directly from interrupt context to update
   the current line operating mode of an analog channel and wake up
   the thread waiting on sdd_event. The sdd_event informs the waiting
   thread of complete operating mode transition.

   \param  pCh             Pointer to the VMMC channel structure.
   \param  lm              Current line mode reported by firmware.
*/
IFX_void_t irq_VMMC_ALM_UpdateOpModeAndWakeUp (VMMC_CHANNEL *pCh,
                                               IFX_uint8_t lm)
{
   VMMC_ASSERT ((pCh != IFX_NULL) && (pCh->pALM != IFX_NULL));

   if (lm != OPMODE_IGNORED)
   {
      pCh->pALM->curr_opmode = lm;

#ifdef VMMC_FEAT_CLOCK_SCALING
      if ((pCh->pALM->curr_opmode == VMMC_SDD_OPMODE_DISABLED) ||
          (pCh->pALM->curr_opmode == VMMC_SDD_OPMODE_STANDBY) ||
          ((pCh->pALM->curr_opmode == VMMC_SDD_OPMODE_FXO) &&
           (pCh->pALM->fw_sdd_fxoHookSwitch.HF == SDD_FxoHookSwitch_HF_ON)))
      {
         pCh->pALM->bDartCanSleep = IFX_TRUE;
      }
#endif /* VMMC_FEAT_CLOCK_SCALING */
   }

   pCh->pALM->bOpmodeChangePending = IFX_FALSE;
   VMMC_OS_EventWakeUp (&pCh->pALM->sdd_event);
}

#ifdef VMMC_FEAT_CLOCK_SCALING
/**
   Returns if the DART is active or not.

   \param  pCh             Pointer to the VMMC channel structure.
*/
IFX_boolean_t irq_VMMC_ALM_DartCanSleep (VMMC_CHANNEL *pCh)
{
   /* For non initialised channels / ALM modules return true because the
      going to sleep must not be blocked for not existing resources. */
   if ((pCh == IFX_NULL) || (pCh->pALM == IFX_NULL))
      return IFX_TRUE;

   return pCh->pALM->bDartCanSleep;
}
#endif /* VMMC_FEAT_CLOCK_SCALING */

/**
   Configure or check ALM module for given sampling mode.

   \param  pCh             Pointer to the VMMC channel structure.
   \param  action          Action to be executed (set or check).
   \param  sigarray_mode   Signalling array operation mode (16kHz or 8 kHz).
   \param  module_mode     Operation mode of the module (16kHz or 8 kHz).

   \return
   If action is SM_SET: IFX_SUCCESS or IFX_ERROR.
   If action is SM_CHECK: IFX_TRUE when module would do a switch or IFX_FALSE
                          if nothing needs to be done.

*/
IFX_int32_t  VMMC_ALM_SamplingMode (VMMC_CHANNEL *pCh,
                                    SM_ACTION action,
                                    OPMODE_SMPL sigarray_mode,
                                    OPMODE_SMPL module_mode)
{
   VMMC_DEVICE       *pDev = pCh->pParent;
   ALI_CHAN_t        *p_fw_ali_ch;
   IFX_uint32_t      new_ISR, new_UD;
   IFX_int32_t       ret = IFX_SUCCESS;

   /* calling function should ensure valid parameters */
   VMMC_ASSERT(pCh != IFX_NULL);

   /* Signal array 8 kHz AND module 16 kHz is disallowed */
   VMMC_ASSERT((sigarray_mode != NB_8_KHZ) || (module_mode != WB_16_KHZ));
   /* Determine the desired value of the ISR bit from the parameters */
   new_ISR = (sigarray_mode == WB_16_KHZ) ? 1 : 0;
   /* Determine the desired value of the up-/down-sampling bit */
   /* UD is set when signal array runs in other mode than the module */
   new_UD  = (sigarray_mode != module_mode) ? 1 : 0;

   /* protect fw msg */
   VMMC_OS_MutexGet (&pCh->chAcc);

   /* get pointer to cached fw message */
   p_fw_ali_ch = &pCh->pALM->fw_ali_ch;

   /* check if the ALM module already operates in requested mode */
   if( (p_fw_ali_ch->ISR != new_ISR) ||
       (p_fw_ali_ch->UD  != new_UD) )
   {
      /* If action is execute do changes otherwise report need for execution. */
      if (action == SM_SET)
      {
         /* change the ISR bit of the ALM module */
         /* before ISR bit can be changed the ALM module must be deactivated */
         /* before the ALM module can be deactivated both LEC and ES must be
            deactivated */

         /* LEC deactivation is always needed when LEC is enabled */
         /* Coding ensures that LEC is enabled when the resource id is valid. */
         if (VMMC_RES_ID_VALID (pCh->pALM->nLecResId))
         {
            ret = VMMC_RES_LEC_Enable (pCh->pALM->nLecResId, IFX_DISABLE);
         }
         /* Echo Suppressor deactivation is mandatory when enabled */
         /* Coding ensures that ES is enabled when the resource id is valid. */
         if ((ret == VMMC_statusOk) && VMMC_RES_ID_VALID (pCh->pALM->nEsResId))
         {
            ret = VMMC_RES_ES_Enable (pCh->pALM->nEsResId, IFX_DISABLE);
         }
         /* ALM module deactivation is needed if the module is enabled */
         if ((p_fw_ali_ch->EN != ALI_CHAN_DISABLE) && (ret == IFX_SUCCESS))
         {
            /* deactivate the ALM module */
            p_fw_ali_ch->EN = ALI_CHAN_DISABLE;
            /* write ALM channel command */
            ret = CmdWrite(pDev, (IFX_uint32_t*)p_fw_ali_ch,
                           sizeof(ALI_CHAN_t)- CMD_HDR_CNT);
            /* state when entering here was enabled so restore the setting */
            p_fw_ali_ch->EN = ALI_CHAN_ENABLE;
         }

         /* Here the LEC and ALM module are turned off - change settings now */

         if (ret == IFX_SUCCESS)
         {
            /* Set the ISR bit in the cached message to the new value */
            p_fw_ali_ch->ISR = new_ISR;
            /* Set the UD bit in the cached message to the new value */
            p_fw_ali_ch->UD = new_UD;

            TRACE(VMMC, DBG_LEVEL_LOW,
                  ("Set ALM channel %u ISR = %d UD = %d\n",
                   pCh->nChannel - 1, p_fw_ali_ch->ISR, p_fw_ali_ch->UD));
         }

         if ((ret == IFX_SUCCESS) && (module_mode != pCh->pALM->module_mode))
         {
            if ((ret == VMMC_statusOk) &&
                VMMC_RES_ID_VALID (pCh->pALM->nLecResId))
            {
               /* set the LEC to the current sampling rate (mode) */
               VMMC_RES_LEC_SamplingModeSet (pCh->pALM->nLecResId, module_mode);
            }
         }
         /* write cached ALM channel command to the chip if enabled */
         if ((p_fw_ali_ch->EN != ALI_CHAN_DISABLE)  && (ret == IFX_SUCCESS))
         {
            /* write ALM channel command */
            ret = CmdWrite(pDev, (IFX_uint32_t*)p_fw_ali_ch,
                           sizeof(ALI_CHAN_t)- CMD_HDR_CNT);
         }
         /* enable the ES if it was running before */
         if ((ret == VMMC_statusOk) && VMMC_RES_ID_VALID (pCh->pALM->nEsResId))
         {
            ret = VMMC_RES_ES_Enable (pCh->pALM->nEsResId, IFX_ENABLE);
         }
         /* enable the LEC if it was running before */
         if ((ret == VMMC_statusOk) && VMMC_RES_ID_VALID (pCh->pALM->nLecResId))
         {
            ret = VMMC_RES_LEC_Enable (pCh->pALM->nLecResId, IFX_ENABLE);

#ifdef VMMC_FEAT_RTCP_XR
            /* Set association between LEC and COD. */
            if (VMMC_SUCCESS(ret))
            {
               ret = VMMC_RES_LEC_AssociatedCodSet(
                        pCh->pALM->nLecResId,
                        VMMC_CON_SingleDataChannelCodFind(pCh, VMMCDSP_MT_ALM));
            }
#endif /* VMMC_FEAT_RTCP_XR */
         }
         /* remember the module_mode so that we can avoid loading ALM coeffs
            if the module modes does not change from this one */
         pCh->pALM->module_mode = module_mode;
      }
      else
      {
         /* action is check: return that this module would do a switch */
         ret = IFX_TRUE;
      }
   }
   else
   {
      if (action == SM_SET)
      {
         TRACE(VMMC, DBG_LEVEL_LOW,
               ("Sampling rate of ALM on channel %u already matching\n",
                pCh->nChannel - 1));

#ifdef VMMC_FEAT_RTCP_XR
         /* If there is a LEC set association between LEC and COD. */
         if (VMMC_RES_ID_VALID (pCh->pALM->nLecResId))
         {
            ret = VMMC_RES_LEC_AssociatedCodSet(
                     pCh->pALM->nLecResId,
                     VMMC_CON_SingleDataChannelCodFind(pCh, VMMCDSP_MT_ALM));
         }
#endif /* VMMC_FEAT_RTCP_XR */
      }
      else
      {
         /* action is check: return that this module does not need a switch */
         ret = IFX_FALSE;
      }
   }

   VMMC_OS_MutexRelease (&pCh->chAcc);

   return ret;
}

/**
   base ALM configuration

   Use this function where needed to set the base configuration
   of Analog Line Module.

   \param  pCh             Pointer to the VMMC channel structure.

   \return
   - VMMC_statusOk         If successful
   - VMMC_statusCmdWr      Writing the command has failed
   - VMMC_statusAlmInit    Analog line intialization failed
*/
IFX_int32_t VMMC_ALM_baseConf (VMMC_CHANNEL *pCh)
{

   VMMC_DEVICE      *pDev  = pCh->pParent;
   IFX_int32_t       ret = VMMC_statusOk;

   /* configure ALI channel */
   pCh->pALM->fw_ali_ch.EN  = ALI_CHAN_ENABLE;

   pCh->pALM->fw_ali_ch.DG1 = VMMC_GAIN_0DB;
   pCh->pALM->fw_ali_ch.DG2 = VMMC_GAIN_0DB;

   if (pDev->caps.bEventMailboxSupported)
   {
      /* Enable reporting of hook events */
      pCh->pALM->fw_ali_ch.EH = ALI_CHAN_EH_ON;
      /* Enable reporting of opmode change events */
      pCh->pALM->fw_ali_ch.EO = ALI_CHAN_EO_ON;
   }

   /* Set the inputs in the cached message and write it */
   ret = VMMC_ALM_Set_Inputs(pCh);

   if (!VMMC_SUCCESS(ret))
   {
      /* errmsg: Analog line intialization failed, due to command write error */
      RETURN_STATUS (VMMC_statusAlmInit);
   }

   /* FXO configuration */
   if (pCh->pALM->line_type_fxs == IFX_FALSE)
   {
      /* Write FXO config to allow ringing signals with 50 Hz. Ring pulses
         shorter than DEB1 [ms] will be ignored. To allow 50Hz ringing the
         value must be below 20ms. */
      pCh->pALM->fw_sdd_fxoConfig.TACT   = 10;  /* [ms] */
      pCh->pALM->fw_sdd_fxoConfig.DEB2   = 70;  /* [ms] */
      pCh->pALM->fw_sdd_fxoConfig.OSIT   = 100; /* [ms] */
      pCh->pALM->fw_sdd_fxoConfig.FLASHT = 50;  /* [ms] */
      pCh->pALM->fw_sdd_fxoConfig.DEB1   = 16;  /* [ms] */

      ret = CmdWrite(pDev, (IFX_uint32_t*)&pCh->pALM->fw_sdd_fxoConfig,
                     SDD_FxoConfig_LEN);

      /* activate FXO channel in firmware */
      if (VMMC_SUCCESS(ret) &&
          (pCh->pALM->fxo_flags & (1U << FXO_LINE_MODE)))
      {
         ret = VMMC_ALM_OpmodeModeSet(pCh, VMMC_SDD_OPMODE_DISABLED);
         if (VMMC_SUCCESS(ret))
         {
            ret = VMMC_ALM_OpmodeModeSet(pCh, VMMC_SDD_OPMODE_FXO);
         }
      }
   }

   RETURN_STATUS (ret);
}


/**
   Initalize the analog module and the cached firmware messages.

   \param  pCh             Pointer to the VMMC channel structure.
*/
IFX_void_t VMMC_ALM_InitCh (VMMC_CHANNEL *pCh)
{
   VMMC_ALMCH_t            *pALM = pCh->pALM;
   ALI_CHAN_t              *pAliCh  = &pALM->fw_ali_ch;
   VMMC_SDD_Opmode_t       *pOpmode = &pALM->fw_sdd_opmode;
   VMMC_SDD_BasicConfig_t  *pSDD_BasicConfig = &pALM->fw_sdd_basic_config;
   IFX_uint8_t             ch = pCh->nChannel - 1;

   /* create timer to supervise elapsed time since last hook event */
   pALM->nHookWindowTimerId =
      TAPI_Create_Timer((TIMER_ENTRY)vmmc_alm_HookWindow_OnTimer,
                        (IFX_ulong_t)pCh);

   /* begin with disabled line feeding */
   pALM->curr_opmode = VMMC_SDD_OPMODE_DISABLED;
   pALM->curr_polarity = VMMC_OPMOD_POL_NORMAL;
   pALM->curr_howler = VMMC_OPMOD_HOWLER_OFF;
#ifdef VMMC_FEAT_CLOCK_SCALING
   /* when disabled the DART is inactive */
   pALM->bDartCanSleep = IFX_TRUE;
#endif /* VMMC_FEAT_CLOCK_SCALING */
   pALM->bOpmodeChangePending = IFX_FALSE;

   /* initialize SDD event */
   VMMC_OS_EventInit (&pALM->sdd_event);

   /* ALI ch message */
   memset (pAliCh, 0, sizeof (*pAliCh));
   pAliCh->CHAN  = ch;
   pAliCh->CMD   = CMD_EOP;
   pAliCh->MOD   = MOD_ALI;
   pAliCh->ECMD  = ALI_CHAN_ECMD;

   /* SDD_Opmode */
   memset (pOpmode, 0, sizeof (*pOpmode));
   pOpmode->CHAN = ch;
   pOpmode->CMD  = CMD_SDD;
   pOpmode->MOD  = MOD_SDD;
   pOpmode->ECMD = SDD_Opmode_ECMD;

   /* setup basic configuration command (temporary storage for MWL) */
   memset (&pALM->sdd_basic_config_tmp_mwl, 0, sizeof (VMMC_SDD_BasicConfig_t));
   pALM->sdd_basic_config_tmp_mwl.CHAN   = ch;
   pALM->sdd_basic_config_tmp_mwl.CMD    = CMD_SDD;
   pALM->sdd_basic_config_tmp_mwl.ECMD   = SDD_BasicConfig_ECMD;

   /* setup ring configuration command (temporary storage for MWL) */
   memset(&pALM->sdd_ring_config_tmp_mwl, 0, sizeof (VMMC_SDD_RingConfig_t));
   pALM->sdd_ring_config_tmp_mwl.CHAN   = ch;
   pALM->sdd_ring_config_tmp_mwl.CMD    = CMD_SDD;
   pALM->sdd_ring_config_tmp_mwl.ECMD   = SDD_RingConfig_ECMD;

   /* Reading calibration data */
   memset(&pALM->fw_sdd_calibrate, 0, sizeof(pALM->fw_sdd_calibrate));
   pALM->fw_sdd_calibrate.CHAN = ch;
   pALM->fw_sdd_calibrate.CMD  = CMD_SDD;
   pALM->fw_sdd_calibrate.MOD  = MOD_SDD;
   pALM->fw_sdd_calibrate.ECMD = SDD_Calibrate_ECMD;

   /* Initialise the semaphore to wait for calibration to finish. */
   VMMC_OS_MutexInit(&pCh->pALM->mtxCalibrationWait);
   VMMC_OS_MutexGet(&pCh->pALM->mtxCalibrationWait);
   /* no calibration was done yet */
   pALM->nCalibrationState = IFX_TAPI_CALIBRATION_STATE_NO;
   /* currently calibration is not running */
   pALM->bCalibrationRunning = IFX_FALSE;
   /* No calibration is needed - flag will be set during BBD download. */
   pALM->bCalibrationNeeded = IFX_FALSE;

   /* Clear the cache for the last calibration results */
   memset(&pALM->calibrationLastResults, 0,
          sizeof(pALM->calibrationLastResults));

   /* setup linetype */
   pALM->line_type_fxs = (vmmc_alm_line[ch] == VMMC_ALM_LINE_FXS) ?
                                     IFX_TRUE : IFX_FALSE;
   /* initialize FXO bits - all 0, polarity 1 (normal polarity) */
   pALM->fxo_flags = 0 | (1 << FXO_POLARITY);
#ifdef VMMC_ACT_FXO
   /* initialize default FXO line mode */
   pALM->fxo_flags |= (1U << FXO_LINE_MODE);
#endif /*VMMC_ACT_FXO*/

   /* start in narrowband operation mode */
   pALM->module_mode = NB_8_KHZ;

   /* echo suppressor resource id */
   pALM->nEsResId = VMMC_RES_ID_NULL;

   /* line echo canceller resource id */
   pALM->nLecResId = VMMC_RES_ID_NULL;

   /* default DC/DC type at startup */
   pALM->nDcDcType = VMMC_DCDC_TYPE_DEFAULT_IBB;

   /* SDD_BasicConfig */
   memset (pSDD_BasicConfig, 0, sizeof(VMMC_SDD_BasicConfig_t));
   pSDD_BasicConfig->CMD = CMD_SDD;
   pSDD_BasicConfig->CHAN = ch;
   pSDD_BasicConfig->ECMD = SDD_BasicConfig_ECMD;
   pALM->bIdleExtNeedRestore = IFX_FALSE;
   pALM->nStdbyDupTimeOverride = 0;

#ifdef VMMC_FEAT_CONT_MEASUREMENT
   /* Read continuous measurement from SDD */
   memset(&pALM->fw_sdd_contMeasRead, 0, sizeof(pALM->fw_sdd_contMeasRead));
   pALM->fw_sdd_contMeasRead.CHAN = ch;
   pALM->fw_sdd_contMeasRead.CMD  = CMD_SDD;
   pALM->fw_sdd_contMeasRead.ECMD = SDD_ContMeasRead_ECMD;

   /* Clear continuous measurement values in SDD */
   memset(&pALM->fw_sdd_contMearClear, 0, sizeof(pALM->fw_sdd_contMearClear));
   pALM->fw_sdd_contMearClear.CHAN = ch;
   pALM->fw_sdd_contMearClear.CMD  = CMD_SDD;
   pALM->fw_sdd_contMearClear.ECMD = SDD_ContMeasClear_ECMD;
#endif /* VMMC_FEAT_CONT_MEASUREMENT */

   /* SDD_FxoHookSwitch */
   memset(&pALM->fw_sdd_fxoHookSwitch, 0, sizeof(pALM->fw_sdd_fxoHookSwitch));
   pALM->fw_sdd_fxoHookSwitch.CHAN = ch;
   pALM->fw_sdd_fxoHookSwitch.CMD  = CMD_SDD;
   pALM->fw_sdd_fxoHookSwitch.ECMD = SDD_FxoHookSwitch_ECMD;

   /* SDD_FxoConfig */
   memset(&pALM->fw_sdd_fxoConfig, 0, sizeof(pALM->fw_sdd_fxoConfig));
   pALM->fw_sdd_fxoConfig.CHAN   = ch;
   pALM->fw_sdd_fxoConfig.CMD    = CMD_SDD;
   pALM->fw_sdd_fxoConfig.ECMD   = SDD_FxoConfig_ECMD;

#ifdef VMMC_FEAT_CAP_MEASURE
   /* SDD_GR909Config */
   memset(&pALM->fw_sdd_GR909Config, 0, sizeof(pALM->fw_sdd_GR909Config));
   pALM->fw_sdd_GR909Config.CMD     = CMD_SDD;
   pALM->fw_sdd_GR909Config.CHAN    = ch;
   pALM->fw_sdd_GR909Config.MOD     = MOD_SDD;
   pALM->fw_sdd_GR909Config.ECMD    = SDD_GR909Config_ECMD;
   pALM->fw_sdd_GR909Config.LENGTH  = SDD_GR909Config_LEN;
   pALM->fw_sdd_GR909Config.HptW2gAcLim  = 0x24EE;
   pALM->fw_sdd_GR909Config.HptW2wAcLim  = 0x24EE;
   pALM->fw_sdd_GR909Config.HptW2gDcLim  = 0x4738;
   pALM->fw_sdd_GR909Config.HptW2wDcLim  = 0x4738;
   pALM->fw_sdd_GR909Config.FemfW2gAcLim = 0x0762;
   pALM->fw_sdd_GR909Config.FemfW2wAcLim = 0x0762;
   pALM->fw_sdd_GR909Config.FemfW2gDcLim = 0x032A;
   pALM->fw_sdd_GR909Config.FemfW2wDcLim = 0x032A;
   pALM->fw_sdd_GR909Config.RftResLim    = 0x1DF5;
   pALM->fw_sdd_GR909Config.RohLinLim    = 0x000F;
   pALM->fw_sdd_GR909Config.RitLowLim    = 0x0047;
   pALM->fw_sdd_GR909Config.RitHighLim   = 0x07FD;

   /* SDD_GR909PhoneDetection */
   memset(&pALM->fw_sdd_phone_detection, 0,
          sizeof(pALM->fw_sdd_phone_detection));
   pALM->fw_sdd_phone_detection.CMD  = CMD_SDD;
   pALM->fw_sdd_phone_detection.CHAN = ch;
   pALM->fw_sdd_phone_detection.MOD  = MOD_SDD1;
   pALM->fw_sdd_phone_detection.ECMD = SDD_GR909PhoneDetection_ECMD;

   /* Initial state of capacitance measurement */
   pALM->eCapMeasState = VMMC_CapMeasState_Inactive;
#endif /* VMMC_FEAT_CAP_MEASURE */

#ifdef VMMC_FEAT_NLT
   /* Set default values for capacitance open loop calibration factors. */
   pALM->nlt_CapacitanceConfig.fOlCapTip2Ring = 7.5;
   pALM->nlt_CapacitanceConfig.fOlCapTip2Gnd = 0;
   pALM->nlt_CapacitanceConfig.fOlCapRing2Gnd = 0;
   /* Set default values for resistance open loop calibration factors. */
   pALM->nlt_ResistanceConfig.fOlResTip2Ring = 0;
   pALM->nlt_ResistanceConfig.fOlResTip2Gnd = 0;
   pALM->nlt_ResistanceConfig.fOlResRing2Gnd = 0;
   pCh->pALM->nRmeas = IFX_TAPI_NLT_RMEAS_1_5MOHM;
#endif /* VMMC_CFG_FEATURES & VMMC_FEAT_GR909 */

   /* SDD_MwiConfig default */
   memset(&pALM->fw_sdd_mwi_config, 0, sizeof(pALM->fw_sdd_mwi_config));
   pALM->fw_sdd_mwi_config.CHAN       = ch;
   pALM->fw_sdd_mwi_config.CMD        = CMD_SDD;
   pALM->fw_sdd_mwi_config.ECMD       = SDD_MwiConfig_ECMD;
   pALM->fw_sdd_mwi_config.MwiVoltage = SDD_MwiVoltageDefault;
   pALM->fw_sdd_mwi_config.MwiThresh  = SDD_MwiThreshDefault;
   pALM->fw_sdd_mwi_config.MwiSlope   = SDD_MwiSlopeDefault;
   pALM->fw_sdd_mwi_config.MwiOnTime  = SDD_MwiOnTimeDefault;
   pALM->fw_sdd_mwi_config.MwiOffTime = SDD_MwiOffTimeDefault;

   /* initialise the structures used for connecting modules */
   VMMC_CON_Init_AlmCh (pCh);
}


/**
   Set the signal inputs of the cached fw message for the given channel.

   \param  pCh             Pointer to the VMMC channel structure.

  \return
   - VMMC_statusCmdWr Writing the command has failed
   - VMMC_statusOk if successful
*/
IFX_int32_t VMMC_ALM_Set_Inputs (VMMC_CHANNEL *pCh)
{
   ALI_CHAN_t        *p_fw_ali_ch;
   IFX_int32_t       ret;

   /* update the signal inputs of this cached msg */
   p_fw_ali_ch = &pCh->pALM->fw_ali_ch;

   VMMC_OS_MutexGet (&pCh->chAcc);
   p_fw_ali_ch->I1 = VMMC_CON_Get_ALM_SignalInput (pCh, 0);
   p_fw_ali_ch->I2 = VMMC_CON_Get_ALM_SignalInput (pCh, 1);
   p_fw_ali_ch->I3 = VMMC_CON_Get_ALM_SignalInput (pCh, 2);
   p_fw_ali_ch->I4 = VMMC_CON_Get_ALM_SignalInput (pCh, 3);
   p_fw_ali_ch->I5 = VMMC_CON_Get_ALM_SignalInput (pCh, 4);

   /* Turn on the ALM channel although it should already run by default */
   p_fw_ali_ch->EN = ALI_CHAN_ENABLE;

   /* Write the updated cached message to fw. */
   ret = CmdWrite (pCh->pParent, (IFX_uint32_t *)p_fw_ali_ch,
                   sizeof(ALI_CHAN_t) - CMD_HDR_CNT);

   VMMC_OS_MutexRelease (&pCh->chAcc);

   RETURN_STATUS(ret);
}


/**
   Stop ALM on this channel

   \param  pCh             Pointer to the VMMC channel structure.

   \return
   - VMMC_statusOk         If successful
   - VMMC_statusCmdWr      Writing the command has failed
*/
IFX_int32_t VMMC_ALM_ChStop (VMMC_CHANNEL *pCh)
{
   ALI_CHAN_t       *p_fw_ali_ch;
   VMMC_DEVICE      *pDev  = pCh->pParent;
   IFX_int32_t       ret   = VMMC_statusOk;

   /* calling function should ensure valid parameters */
   VMMC_ASSERT(pCh != IFX_NULL);

   if (pCh->pALM != IFX_NULL)
   {
      if (pCh->pALM->nHookWindowTimerId != 0)
      {
         TAPI_Delete_Timer (pCh->pALM->nHookWindowTimerId);
         pCh->pALM->nHookWindowTimerId = 0;
      }

      /* get pointer to cached fw message */
      p_fw_ali_ch = &pCh->pALM->fw_ali_ch;

      /* protect fw msg */
      VMMC_OS_MutexGet (&pCh->chAcc);

      if (pDev->caps.bEventMailboxSupported)
      {
         /* After stop we do not want to get any events any more. */
         p_fw_ali_ch->EH = ALI_CHAN_EH_OFF;
         p_fw_ali_ch->EO = ALI_CHAN_EO_OFF;
      }

      /* set the line to disabled (this implicitly writes the fw message) */
      ret = VMMC_TAPI_LL_ALM_Line_Mode_Set (pCh, IFX_TAPI_LINE_FEED_DISABLED, 0);

      /* before the ALM module can be deactivated both LEC and ES must be
         deactivated */

      /* LEC deactivation is always needed when LEC is enabled */
      /* Coding ensures that LEC is enabled when the resource id is valid. */
      if ((ret == VMMC_statusOk) && VMMC_RES_ID_VALID (pCh->pALM->nLecResId))
      {
         ret = VMMC_RES_LEC_Enable (pCh->pALM->nLecResId, IFX_DISABLE);
      }

      /* Echo Suppressor deactivation is mandatory when enabled */
      /* Coding ensures that ES is enabled when the resource id is valid. */
      if ((ret == VMMC_statusOk) && VMMC_RES_ID_VALID (pCh->pALM->nEsResId))
      {
         ret = VMMC_RES_ES_Enable (pCh->pALM->nEsResId, IFX_DISABLE);
      }

      /* ALM module deactivation is done if the module is enabled */
      if ((ret == VMMC_statusOk) && (p_fw_ali_ch->EN != ALI_CHAN_DISABLE))
      {
         /* deactivate the ALM module */
         p_fw_ali_ch->EN = ALI_CHAN_DISABLE;
         /* write ALM channel command */
         ret = CmdWrite(pDev, (IFX_uint32_t *)p_fw_ali_ch,
                        sizeof(*p_fw_ali_ch)- CMD_HDR_CNT);
      }

      /* Reset of the DC/DC type to default allows to download the BBD again. */
      pCh->pALM->nDcDcType = VMMC_DCDC_TYPE_DEFAULT_IBB;

      VMMC_OS_MutexRelease (&pCh->chAcc);

      /* Delete the semaphore to wait for calibration to finish. */
      VMMC_OS_MutexRelease(&pCh->pALM->mtxCalibrationWait);
      VMMC_OS_MutexDelete(&pCh->pALM->mtxCalibrationWait);
   }

   RETURN_STATUS(ret);
}


/**
   Allocate data structure of the ALM module in the given channel.

   \param  pCh             Pointer to the VMMC channel structure.

   \return
      - VMMC_statusOk if ok
      - VMMC_statusNoMem Memory allocation failed -> struct not created

   \remarks The channel parameter is no longer checked because the calling
   function assures correct values.
*/
IFX_int32_t VMMC_ALM_Allocate_Ch_Structures (VMMC_CHANNEL *pCh)
{
   VMMC_ALM_Free_Ch_Structures (pCh);

   pCh->pALM = VMMC_OS_Malloc (sizeof(VMMC_ALMCH_t));
   if (pCh->pALM == IFX_NULL)
   {
      /* errmsg: Memory allocation failed */
      RETURN_STATUS (VMMC_statusNoMem);
   }
   memset(pCh->pALM, 0, sizeof(VMMC_ALMCH_t));
#ifdef EVALUATION
   if (VMMC_Eval_ALM_Allocate_Ch_Structures (pCh) != VMMC_statusOk)
   {
      RETURN_STATUS(VMMC_statusDevInitFail);
   }
#endif /* #ifdef EVALUATION */

   return VMMC_statusOk;
}


/**
   Free data structure of the ALM module in the given channel.

   \param  pCh             Pointer to the VMMC channel structure.
*/
IFX_void_t VMMC_ALM_Free_Ch_Structures (VMMC_CHANNEL *pCh)
{
   if (pCh->pALM != IFX_NULL)
   {
#ifdef EVALUATION
      VMMC_Eval_ALM_Free_Ch_Structures (pCh);
#endif /* #ifdef EVALUATION */
      VMMC_OS_Free (pCh->pALM);
      pCh->pALM = IFX_NULL;
   }
}


#ifdef VMMC_FEAT_SLIC
/**
   Check whether SmartSLIC is connected

   \param  pDev         Pointer to the VMMC device structure.

   \return
   - IFX_TRUE           SmartSLIC is connected
   - IFX_FALSE          SmartSLIC is NOT connected
*/
IFX_boolean_t VMMC_ALM_SmartSLIC_IsConnected(VMMC_DEVICE *pDev)
{
   VMMC_SYS_SLIC_TEST_t   sysSlic;
   IFX_int32_t            ret;

   memset((IFX_void_t *) &sysSlic, 0, sizeof(VMMC_SYS_SLIC_TEST_t));

   sysSlic.CMD    = CMD_EOP;
   sysSlic.MOD    = MOD_SYSTEM;
   sysSlic.ECMD   = SYS_SLIC_TEST_ECMD;
   ret = CmdRead(pDev, (IFX_uint32_t *) &sysSlic,
                       (IFX_uint32_t *) &sysSlic,
                        SYS_SLIC_TEST_LENGTH);
   if (VMMC_SUCCESS(ret))
   {
      if (sysSlic.SmartSLICStatus & SYS_SLIC_TEST_CONNECTED)
      {
         return IFX_TRUE;
      }
   }

   return IFX_FALSE;
}
#endif /* VMMC_FEAT_SLIC */


#ifdef VMMC_FEAT_SLIC
/**
   Read the number of channels on the SmartSLIC.

   \param  pDev          Pointer to the VMMC device structure.
   \param  nChannels     Returns the number of analog channels.
                         Only valid if return is VMMC_statusOk.
   \param  nFXOChannels  Returns the number of natively supported FXO channels.
                         Only valid if return is VMMC_statusOk.

   \return
   - VMMC_statusOk      if successful
   - VMMC_statusErr     if no SmartSLIC is connected or revision read failed or
                        total ALM channel count for FW is greater than MAX_ALM_NUM
*/
IFX_int32_t VMMC_ALM_SmartSLIC_ChGet (VMMC_DEVICE *pDev,
                                      IFX_uint8_t *nChannels,
                                      IFX_uint8_t *nFXOChannels)
{
   VMMC_SDD_REVISION_READ_t SDDVersCmd;
   IFX_int32_t ret = VMMC_statusErr;

   if (VMMC_ALM_SmartSLIC_IsConnected(pDev))
   {
      IFX_uint8_t ch;

      memset((IFX_void_t *)&SDDVersCmd, 0, sizeof(SDDVersCmd));
      SDDVersCmd.CMD = CMD_SDD;
      SDDVersCmd.MOD = MOD_SDD;
      SDDVersCmd.ECMD = SDD_REVISION_READ_ECMD;
      ret = CmdRead(pDev, (IFX_uint32_t *)&SDDVersCmd,
                          (IFX_uint32_t *)&SDDVersCmd, 8);

      if (VMMC_SUCCESS(ret))
      {
         /* sanity check */
         if (SDDVersCmd.SmartSLIC_CH > MAX_ALM_NUM)
         {
            TRACE (VMMC, DBG_LEVEL_HIGH,
            ("VMMC: ALM count in FW is greater than supported by VMMC (%d>%d)\n",
                                         SDDVersCmd.SmartSLIC_CH, MAX_ALM_NUM));
            return VMMC_statusErr;
         }
         *nChannels = SDDVersCmd.SmartSLIC_CH;
         TRACE (VMMC, DBG_LEVEL_LOW,
                  ("VMMC: SDD version %d.%d.%d\n",
                  SDDVersCmd.DCCtrlVers, SDDVersCmd.DCCtrlVersStep,
                  SDDVersCmd.DCCtrlHotFixRevision));
         /* update FXO settings */
         vmmc_alm_line[0] = !SDDVersCmd.SmartSLIC_FXO0 ?
                                          VMMC_ALM_LINE_FXS : VMMC_ALM_LINE_FXO;
         vmmc_alm_line[1] = !SDDVersCmd.SmartSLIC_FXO1 ?
                                          VMMC_ALM_LINE_FXS : VMMC_ALM_LINE_FXO;
         /* return the count of FXO channels to calling function */
         for (ch=0, *nFXOChannels=0; ch<SDDVersCmd.SmartSLIC_CH; ch++)
            if (vmmc_alm_line[ch] == VMMC_ALM_LINE_FXO)
               (*nFXOChannels)++;
         /* update HL TAPI SmartSLIC flag - required for Clare FXO support on AR9 */
         if (*nFXOChannels &&
             ((SDDVersCmd.DCCtrlVers==1 && SDDVersCmd.DCCtrlVersStep>=37) ||
              SDDVersCmd.DCCtrlVers>1))
         {
            TAPI_DEV *pTapiDev = (pDev->pChannel[0].pTapiCh)->pTapiDevice;

            IFX_TAPI_Update_SlicFxo(pTapiDev, IFX_TRUE);
         }
      }
   }

   return ret;
}
#endif /* VMMC_FEAT_SLIC */


#ifdef VMMC_FEAT_SLIC
/**
   Read the version of the SDD and store it in the device structure.

   \param  pDev          Pointer to the VMMC device structure.

   \return
   - VMMC_statusOk      if successful
   - Errors from CmdRead() otherwise
*/
IFX_int32_t VMMC_SDD_VersionRead (
                        VMMC_DEVICE *pDev)
{
   VMMC_SDD_REVISION_READ_t SddVersCmd;
   IFX_int32_t ret;

   memset((IFX_void_t *)&SddVersCmd, 0, sizeof(SddVersCmd));
   SddVersCmd.CMD = CMD_SDD;
   SddVersCmd.MOD = MOD_SDD;
   SddVersCmd.ECMD = SDD_REVISION_READ_ECMD;

   ret = CmdRead(pDev, (IFX_uint32_t *)&SddVersCmd,
                       (IFX_uint32_t *)&SddVersCmd, SDD_REVISION_READ_LEN);
   if (!VMMC_SUCCESS(ret))
   {
      return ret;
   }

   /* SmartSLIC revision */
   pDev->sdd.nSlicDevId = SddVersCmd.SmartSLIC_DevID;
   pDev->sdd.nSlicRevision = SddVersCmd.SmartSLIC_HW_Rev;
   /* Firmware (DCCtrl) version */
   pDev->sdd.nFirmwareVers = SddVersCmd.DCCtrlVers;
   pDev->sdd.nFirmwareStep = SddVersCmd.DCCtrlVersStep;
   pDev->sdd.nFirmwareHotFix = SddVersCmd.DCCtrlHotFixRevision;
   /* ASDSP version */
   pDev->sdd.nASDSPVers = SddVersCmd.ASDSPVers;
   pDev->sdd.nASDSPStep = SddVersCmd.ASDSPStep;

   /* DC/DC converter type indicated on the SLIC GPIOs. */
   pDev->sdd.nAllowedDcDcType = (enum VMMC_DCDC_TYPE)
      (SddVersCmd.DcDcType + VMMC_DCDC_TYPE_IBB);
   /* Quick and easy check if the DC/DC converter is a dedicated type. */
   pDev->sdd.bDcDcHwCombined =
      (SddVersCmd.DcDcType & SDD_REVISION_READ_DCTYPE_COMBINED) ?
      IFX_TRUE : IFX_FALSE;

   TRACE (VMMC, DBG_LEVEL_NORMAL, (
          "VMMC: SDD %d.%d.%d, ASDSP %d.%d, SmartSLIC ID 0x%02x Rev %d, "
          "connected DC/DC Type %d\n",
          pDev->sdd.nFirmwareVers,
          pDev->sdd.nFirmwareStep,
          pDev->sdd.nFirmwareHotFix,
          pDev->sdd.nASDSPVers,
          pDev->sdd.nASDSPStep,
          pDev->sdd.nSlicDevId,
          pDev->sdd.nSlicRevision,
          SddVersCmd.DcDcType /* show untranslated value from HW */
         ));

   return VMMC_statusOk;
}
#endif /* VMMC_FEAT_SLIC */


/**
   Get a pointer to the neighbour channel struct of the given VMMC ALM
   FXS channel.

   The VMMC only has two ALM FXS channels by HW design and so if channel 0
   is passed channel 1 is returned and vice versa.

   \param  pCh          Pointer to the channel.

   \return
   Pointer to the VMMC ALM FXS channel struct of the neighbour channel.
*/
VMMC_CHANNEL *VMMC_ALM_FxsNeighbourChGet (VMMC_CHANNEL *pCh)
{
   IFX_uint8_t nCh;
   VMMC_DEVICE *pDev = pCh->pParent;
   VMMC_CHANNEL *pChNeighbour;

   if (pCh->pALM == IFX_NULL)
   {
      /* can get neighbour only for FXS ALM channel */
      return IFX_NULL;
   }

   if (pCh->pALM->line_type_fxs != IFX_TRUE)
   {
      /* can get neighbour only for FXS ALM channel */
      return IFX_NULL;
   }
   /* Note that the channel number stored in the channel is index + 1. */
   nCh = ((pCh->nChannel - 1) == 0) ? 1 : 0;
   pChNeighbour = &pDev->pChannel[nCh];
   if ((pChNeighbour->pALM != IFX_NULL) &&
       (IFX_TRUE == pChNeighbour->pALM->line_type_fxs))
   {
      /* return only another ALM channel */
      return pChNeighbour;
   }
   return IFX_NULL;
}


#ifdef VMMC_FEAT_CALIBRATION
/**
   Retrieve calibration data

   \param  pCh          Pointer to the VMMC channel structure.
   \param  pClbConfig   Result as current calibration data.

   \return
   - VMMC_statusOk                   if successful
   - VMMC_statusNoRes                no resource on given channel
   - VMMC_statusCalInProgress        Calibration in progress
   - VMMC_statusOpModeWrErr          Writing the command has failed
*/
static IFX_int32_t vmmc_alm_Calibration_Get (VMMC_CHANNEL *pCh,
                        IFX_TAPI_CALIBRATION_CFG_t *pClbConfig)
{
   VMMC_DEVICE *pDev = pCh->pParent;
   VMMC_SDD_Calibrate_t *pCalibrate = &pCh->pALM->fw_sdd_calibrate;
   IFX_uint8_t  curr_opmode;
   IFX_int32_t  ret = VMMC_statusOk;

   if (pCh->pALM->line_type_fxs == IFX_FALSE)
   {
      /* errmsg: Protection from calling on FXO line */
      RETURN_STATUS (VMMC_statusFXSCallOnFXO);
   }

   /* Sanity check */
   if (pClbConfig == IFX_NULL)
   {
      RETURN_STATUS (VMMC_statusParam);
   }

   /* Set return structure to defined values. */
   memset(pClbConfig, 0x00, sizeof(*pClbConfig));
   pClbConfig->dev = pDev->nDevNr;
   pClbConfig->ch = pCh->nChannel - 1;

   /* Get the current opmode - wait while a opmode change is ongoing. */
   ret = VMMC_ALM_OpmodeGet (pCh, &curr_opmode);
   /* Only exit when interrupted by a signal. When waiting was aborted by the
      timeout assume that no linemode change is pending and try to continue
      with the opmode change that was requested. */
   if (ret == VMMC_statusSddEvtWaitInterrupt)
   {
      RETURN_STATUS (ret);
   }

   /* While calibration is running no values can be read - return with error */
   if (curr_opmode == VMMC_SDD_OPMODE_CALIBRATE)
   {
      /* All returned values are invalid */
      pClbConfig->nState = IFX_TAPI_CALIBRATION_STATE_NO;
      /* errmsg: Current line mode is CALIBRATE */
      RETURN_STATUS (VMMC_statusCalInProgress);
   }

   /* Read calibration values from FW */
   ret = CmdRead (pDev, (IFX_uint32_t *)pCalibrate,
                  (IFX_uint32_t *)pCalibrate, SDD_Calibrate_LEN);

   if (VMMC_SUCCESS (ret))
   {
      pClbConfig->nState = pCh->pALM->nCalibrationState;
      pClbConfig->nITransOffset = (IFX_int16_t)(((IFX_int32_t)((IFX_int16_t)pCalibrate->TxOffset) * 5000) / 32768);
      pClbConfig->nMeOffset     = (IFX_int16_t)(((IFX_int32_t)((IFX_int16_t)pCalibrate->MeOffset) * 25000) / 32768);
      pClbConfig->nUlimOffset30 = (IFX_int16_t)(((IFX_int32_t)((IFX_int16_t)pCalibrate->UlimOffset30) * 7200) / 32768);
      pClbConfig->nUlimOffset60 = (IFX_int16_t)(((IFX_int32_t)((IFX_int16_t)pCalibrate->UlimOffset60) * 14400) / 32768);
      pClbConfig->nIdacGain     = (IFX_int16_t)((((IFX_int32_t)((IFX_int16_t)pCalibrate->IdacGain) * 1000) / 29257) - 1000);
      pClbConfig->nVdcdc        = (IFX_int16_t)((((IFX_int32_t)((IFX_int16_t)pCalibrate->Vdcdc) * 1000) / 16384) - 1000);
      /* In dedicated DC/DC mode the Vdcdc value is invalid - mark as unused. */
      if (pDev->sdd.bDcDcHwCombined == IFX_FALSE)
         pClbConfig->nVdcdc = 0;
   }
   else
   {
      /* All returned values are invalid */
      pClbConfig->nState = IFX_TAPI_CALIBRATION_STATE_NO;
   }

   RETURN_STATUS(ret);
}


#if 0
/**
   Set the calibration data

   \param  pCh          Pointer to the VMMC channel structure.
   \param  pClbConfig   New calibration data.

   \return
   - VMMC_statusOk                   if successful
   - VMMC_statusCalInProgress        Calibration in progress
   - VMMC_statusOpModeWrErr          Writing the command has failed
*/
static IFX_int32_t vmmc_alm_Calibration_Set (VMMC_CHANNEL *pCh,
                        IFX_TAPI_CALIBRATION_CFG_t *pClbConfig)
{
   VMMC_DEVICE *pDev = pCh->pParent;
   VMMC_SDD_Calibrate_t *pCalibrate = &pCh->pALM->fw_sdd_calibrate;
   OPMODE_CMD_t *pOpmode = &pCh->pALM->dc_opmode;
   IFX_int32_t ret;

   /* check if transition is valid, prepare command */
   if (pOpmode->OP_MODE == VMMC_OPMOD_CALIBRATE)
   {
      /* errmsg: Current line mode is CALIBRATE */
      RETURN_STATUS (VMMC_statusCalInProgress);
   }

   /* prepare command */
   /* todo: calculation not yet done */
   pCalibrate->TxOffset       = pClbConfig->nITransOffset;
   pCalibrate->MeOffset       = pClbConfig->nMeOffset;
   pCalibrate->UlimOffset30   = pClbConfig->nUlimOffset30;
   pCalibrate->UlimOffset60   = pClbConfig->nUlimOffset60;
   pCalibrate->IdacGain       = pClbConfig->nIdacGain;
   ret = CmdWrite (pDev, (IFX_uint32_t *)pCalibrate, SDD_Calibrate_LEN);

   RETURN_STATUS (ret);
}
#endif
#endif /* VMMC_FEAT_CALIBRATION */


/**
   Start calibration process for analog channel.

   \param  pCh          Pointer to the VMMC channel structure.
   \param  bInternal    Flag indicating call from external TAPI API (IFX_FALSE)
                        or driver internal (IFX_TRUE).

   \return
   - VMMC_statusOk                   if successful
   - VMMC_statusNoRes                no resource on given channel
   - VMMC_statusCalLineNotDisabled   Invalid current line mode
   - VMMC_statusOpModeWrErr          Writing the command has failed
*/
static IFX_int32_t vmmc_alm_Calibration_Start (VMMC_CHANNEL *pCh,
                                               IFX_boolean_t bInternal)
{
   VMMC_CHANNEL *pOtherCh = IFX_NULL;
   IFX_uint8_t curr_opmode, other_curr_opmode;
   IFX_int32_t ret = VMMC_statusOk;

   if (pCh->pALM->line_type_fxs == IFX_FALSE)
   {
      /* errmsg: Protection from calling on FXO line */
      RETURN_STATUS (VMMC_statusFXSCallOnFXO);
   }
   /* For DC/DC converters other than default a BBD download is required. */
   if ((pCh->pALM->nDcDcType == VMMC_DCDC_TYPE_DEFAULT_IBB) &&
       (pCh->pParent->sdd.nAllowedDcDcType != VMMC_DCDC_TYPE_IBB))
   {
      /* errmsg: Operation blocked until a BBD with DC/DC configuration
                 matching the connected DC/DC hardware type is downloaded */
      RETURN_STATUS (VMMC_statusBlockedDcDcTypeMissmatch);
   }

   /* Get the current opmode - wait while a opmode change is ongoing. */
   ret = VMMC_ALM_OpmodeGet (pCh, &curr_opmode);
   /* Only exit when interrupted by a signal. When waiting was aborted by the
      timeout assume that no linemode change is pending and try to continue
      with the opmode change that was requested. */
   if (ret == VMMC_statusSddEvtWaitInterrupt)
   {
      RETURN_STATUS (ret);
   }

   /* If Calibration is already active - do nothing and return immediately. */
   if (curr_opmode == VMMC_SDD_OPMODE_CALIBRATE)
   {
      return VMMC_statusOk;
   }
   /* Check if transition is valid */
   if (curr_opmode != VMMC_SDD_OPMODE_DISABLED)
   {
      /* errmsg: Current line mode is not DISABLED */
      RETURN_STATUS (VMMC_statusCalLineNotDisabled);
   }

   /* For combined DC/DC the other channel must be disabled when starting. */
   if (pCh->pParent->sdd.bDcDcHwCombined == IFX_TRUE)
   {
      /* The combined DC/DC flag is only set on 2 channel systems. So there
         is always a neighbouring channel. */
      pOtherCh = VMMC_ALM_FxsNeighbourChGet(pCh);
      if (pOtherCh == IFX_NULL)
      {
         /* errmsg: Resource not valid. Channel number out of range */
         RETURN_STATUS (VMMC_statusInvalCh);
      }

      /* protect other channel from mutual access */
      VMMC_OS_MutexGet (&pOtherCh->chAcc);

      /* Get the current opmode - wait while a opmode change is ongoing. */
      ret = VMMC_ALM_OpmodeGet (pOtherCh, &other_curr_opmode);
      /* Only exit when interrupted by a signal. When waiting was aborted by
         timeout assume that no linemode change is pending and try to continue
         with the opmode change that was requested. */
      if (ret == VMMC_statusSddEvtWaitInterrupt)
      {
         /* release protection */
         VMMC_OS_MutexRelease (&pOtherCh->chAcc);
         RETURN_STATUS (ret);
      }

      if (other_curr_opmode != VMMC_SDD_OPMODE_DISABLED)
      {
         /* release protection */
         VMMC_OS_MutexRelease (&pOtherCh->chAcc);
         /* errmsg: Neighbour line mode is not DISABLED */
         RETURN_STATUS (VMMC_statusCalNeighbourLineNotDisabled);
      }
   }

   /* Remember if started from API or driver internal. */
   pCh->pALM->bCalibrationInternal = bInternal;

   /* Start calibration */

   /* Changing the operating mode does the actual start. */
   ret = VMMC_ALM_OpmodeModeSet (pCh, VMMC_SDD_OPMODE_CALIBRATE);

   if (pOtherCh != IFX_NULL)
   {
      /* release protection */
      VMMC_OS_MutexRelease (&pOtherCh->chAcc);
   }

   RETURN_STATUS(ret);
}


#ifdef VMMC_FEAT_CALIBRATION
/**
   Stop calibration process for analog channel.

   \param  pCh          Pointer to the VMMC channel structure.

   \return
   - VMMC_statusOk                   if successful
   - VMMC_statusCalLineNotCalibrating Invalid current line mode
   - VMMC_statusOpModeWrErr          Writing the command has failed
*/
static IFX_int32_t vmmc_alm_Calibration_Stop (VMMC_CHANNEL *pCh)
{
   IFX_uint8_t curr_opmode;
   IFX_int32_t ret;

   if (pCh->pALM->line_type_fxs == IFX_FALSE)
   {
      /* errmsg: Protection from calling on FXO line */
      RETURN_STATUS (VMMC_statusFXSCallOnFXO);
   }

   /* Get the current opmode - wait while a opmode change is ongoing. */
   ret = VMMC_ALM_OpmodeGet (pCh, &curr_opmode);
   /* Only exit when interrupted by a signal. When waiting was aborted by the
      timeout assume that no linemode change is pending and try to continue. */
   if (ret == VMMC_statusSddEvtWaitInterrupt)
   {
      RETURN_STATUS (ret);
   }

   /* check if transition is valid */
   if (curr_opmode != VMMC_SDD_OPMODE_CALIBRATE)
   {
      /* We are not in calibration state. So stopping is not needed.
         This is what was intended by the user and so not an error. */
      return VMMC_statusOk;
   }

   /* Stop calibration by changing operating mode to PDH */
   ret = VMMC_ALM_OpmodeModeSet (pCh, VMMC_SDD_OPMODE_DISABLED);

   RETURN_STATUS (ret);
}


/**
   Retrieve the results of the last calibration process

   \param  pLLChannel   Pointer to the VMMC channel structure.
   \param  pClbConfig   Result as current calibration data.

   \return
   - VMMC_statusOk             if successful
   - VMMC_statusInvalCh        Channel number out of range
*/
IFX_int32_t VMMC_TAPI_LL_ALM_Calibration_Results (IFX_TAPI_LL_CH_t *pLLChannel,
                                         IFX_TAPI_CALIBRATION_CFG_t *pClbConfig)
{
   VMMC_CHANNEL *pCh = (VMMC_CHANNEL *) pLLChannel;
   VMMC_SDD_Calibrate_t *pLastResults = &pCh->pALM->calibrationLastResults;

   /* Sanity checks */
   VMMC_ASSERT(pCh != IFX_NULL);
   if (pCh->pALM == IFX_NULL)
   {
      /* errmsg: Resource not valid. Channel number out of range */
      RETURN_STATUS (VMMC_statusInvalCh);
   }
   if (pClbConfig == IFX_NULL)
   {
      RETURN_STATUS (VMMC_statusParam);
   }

   if (pCh->pALM->line_type_fxs == IFX_FALSE)
   {
      /* errmsg: Protection from calling on FXO line */
      RETURN_STATUS (VMMC_statusFXSCallOnFXO);
   }

   /* protect channel from mutual access */
   VMMC_OS_MutexGet (&pCh->chAcc);

   /* Set return structure to defined values. */
   memset(pClbConfig, 0x00, sizeof(*pClbConfig));
   pClbConfig->dev = pCh->pParent->nDevNr;
   pClbConfig->ch = pCh->nChannel - 1;
   pClbConfig->nState = pCh->pALM->nCalibrationState;
   pClbConfig->nITransOffset = (IFX_int16_t)(((IFX_int32_t)((IFX_int16_t)pLastResults->TxOffset) * 5000) / 32768);
   pClbConfig->nMeOffset     = (IFX_int16_t)(((IFX_int32_t)((IFX_int16_t)pLastResults->MeOffset) * 25000) / 32768);
   pClbConfig->nUlimOffset30 = (IFX_int16_t)(((IFX_int32_t)((IFX_int16_t)pLastResults->UlimOffset30) * 7200) / 32768);
   pClbConfig->nUlimOffset60 = (IFX_int16_t)(((IFX_int32_t)((IFX_int16_t)pLastResults->UlimOffset60) * 14400) / 32768);
   pClbConfig->nIdacGain     = (IFX_int16_t)((((IFX_int32_t)((IFX_int16_t)pLastResults->IdacGain) * 1000) / 29257) - 1000);
   pClbConfig->nVdcdc        = (IFX_int16_t)((((IFX_int32_t)((IFX_int16_t)pLastResults->Vdcdc) * 1000) / 16384) - 1000);
   /* In dedicated DC/DC mode the Vdcdc value is invalid - mark as unused. */
   if (pCh->pParent->sdd.bDcDcHwCombined == IFX_FALSE)
      pClbConfig->nVdcdc = 0;

   /* release protection */
   VMMC_OS_MutexRelease (&pCh->chAcc);

   return VMMC_statusOk;
}


/**
   Retrieve calibration data from device

   \param  pLLChannel   Pointer to the VMMC channel structure.
   \param  pClbConfig   Result as current calibration data.

   \return
   - VMMC_statusOk             if successful
   - VMMC_statusInvalCh        Channel number out of range
*/
IFX_int32_t VMMC_TAPI_LL_ALM_Calibration_Get (IFX_TAPI_LL_CH_t *pLLChannel,
                                   IFX_TAPI_CALIBRATION_CFG_t *pClbConfig)
{
   VMMC_CHANNEL *pCh = (VMMC_CHANNEL *) pLLChannel;
   IFX_int32_t ret;

   /* sanity check */
   VMMC_ASSERT(pClbConfig != IFX_NULL);
   VMMC_ASSERT(pCh != IFX_NULL);
   if (pCh->pALM == IFX_NULL)
   {
      /* errmsg: Resource not valid. Channel number out of range */
      RETURN_STATUS (VMMC_statusInvalCh);
   }

   /* protect channel from mutual access */
   VMMC_OS_MutexGet (&pCh->chAcc);

   ret = vmmc_alm_Calibration_Get (pCh, pClbConfig);

   /* release protection */
   VMMC_OS_MutexRelease (&pCh->chAcc);

   RETURN_STATUS (ret);
}


/** Set the calibration data.

   \param  pLLChannel   Pointer to the VMMC channel structure.
   \param  pClbConfig   New calibration data.

   \return
   - VMMC_statusOk             if successful
   - VMMC_statusInvalCh        Channel number out of range

   \todo Adapt code to AR9
*/
#if 0
IFX_int32_t VMMC_TAPI_LL_ALM_Calibration_Set (IFX_TAPI_LL_CH_t *pLLChannel,
                                   IFX_TAPI_CALIBRATION_CFG_t *pClbConfig)
{
   VMMC_CHANNEL *pCh = (VMMC_CHANNEL *) pLLChannel;
   VMMC_DEVICE *pDev = pCh->pParent;
   IFX_int32_t ret;


   /* sanity check */
   VMMC_ASSERT(pClbConfig != IFX_NULL);
   VMMC_ASSERT(pCh != IFX_NULL);
   if (pCh->pALM == IFX_NULL)
   {
      /* errmsg: Resource not valid. Channel number out of range */
      RETURN_STATUS (VMMC_statusInvalCh);
   }

   /* protect channel from mutual access */
   VMMC_OS_MutexGet (&pCh->chAcc);

   ret = vmmc_alm_Calibration_Set (pCh, pClbConfig);

   /* release protection */
   VMMC_OS_MutexRelease (&pCh->chAcc);

   RETURN_STATUS (ret);
}
#endif


/**
   Start calibration on the given analog line.

   Wrapper to protect the actual start function that runs calibration on
   the analog line. Calibration takes about a second and ends automatically.
   The end is signalled with the event IFX_TAPI_EVENT_CALIBRATION_END.

   \param  pLLChannel   Pointer to the VMMC channel structure.

   \return
   - VMMC_statusOk                   if successful
   - VMMC_statusInvalCh              Channel number out of range
*/
IFX_int32_t VMMC_TAPI_LL_ALM_Calibration_Start (IFX_TAPI_LL_CH_t *pLLChannel)
{
   VMMC_CHANNEL *pCh = (VMMC_CHANNEL *) pLLChannel;
   IFX_int32_t ret;

   /* sanity check */
   VMMC_ASSERT(pCh != IFX_NULL);
   if (pCh->pALM == IFX_NULL)
   {
      /* errmsg: Resource not valid. Channel number out of range */
      RETURN_STATUS (VMMC_statusInvalCh);
   }

   /* protect channel from mutual access */
   VMMC_OS_MutexGet (&pCh->chAcc);

   ret = vmmc_alm_Calibration_Start (pCh, IFX_FALSE);

   /* release protection */
   VMMC_OS_MutexRelease (&pCh->chAcc);

   RETURN_STATUS (ret);
}


/**
   Stop a started calibration immediately.

   \param  pLLChannel   Pointer to the VMMC channel structure.

   \return
   - VMMC_statusOk                   if successful
   - VMMC_statusInvalCh              Channel number out of range
*/
IFX_int32_t VMMC_TAPI_LL_ALM_Calibration_Stop (IFX_TAPI_LL_CH_t *pLLChannel)
{
   VMMC_CHANNEL *pCh = (VMMC_CHANNEL *) pLLChannel;
   IFX_int32_t ret;

   /* sanity check */
   VMMC_ASSERT(pCh != IFX_NULL);
   if (pCh->pALM == IFX_NULL)
   {
      /* errmsg: Resource not valid. Channel number out of range */
      RETURN_STATUS (VMMC_statusInvalCh);
   }

   /* protect channel from mutual access */
   VMMC_OS_MutexGet (&pCh->chAcc);

   ret = vmmc_alm_Calibration_Stop (pCh);

   /* release protection */
   VMMC_OS_MutexRelease (&pCh->chAcc);

   RETURN_STATUS (ret);
}
#endif /* VMMC_FEAT_CALIBRATION */


/**
   Finish calibration process on the analog line.

   This function is called after the firmware indicated that calibration has
   finished. It reads the calibration values and does a plausibility check.
   If values are out of range a reset to defaults is done. In the end the
   line is made available again an a TAPI event is sent to the application
   to inform about the result of the calibration process.

   \param  pLLChannel   Pointer to the VMMC channel structure.

   \return
   - VMMC_statusOk                   if successful
   - VMMC_statusInvalCh              Channel number out of range
*/
IFX_int32_t VMMC_TAPI_LL_ALM_Calibration_Finish (IFX_TAPI_LL_CH_t *pLLChannel)
{
   VMMC_CHANNEL *pCh = (VMMC_CHANNEL *) pLLChannel;
   VMMC_DEVICE  *pDev = pCh->pParent;
   VMMC_SDD_Calibrate_t *pCalibrate = &pCh->pALM->fw_sdd_calibrate,
                        *pLastResults = &pCh->pALM->calibrationLastResults;

   IFX_TAPI_EVENT_t tapiEvent;
   IFX_int32_t ret;

   /* sanity check */
   VMMC_ASSERT(pCh != IFX_NULL);
   if (pCh->pALM == IFX_NULL)
   {
      /* This should actually never occur since this function is only called
         within TAPI HL where the channel is already checked. */
      /* errmsg: Resource not valid. Channel number out of range */
      RETURN_STATUS (VMMC_statusInvalCh);
   }

   /* protect fw messages from mutual access */
   if (!pCh->pALM->bCalibrationInternal)
      VMMC_OS_MutexGet (&pCh->chAcc);

   /* Read the calibrated values for verification below. */
   ret = CmdRead (pDev, (IFX_uint32_t *)pCalibrate,
                  (IFX_uint32_t *)pCalibrate, SDD_Calibrate_LEN);

   if (VMMC_SUCCESS (ret))
   {
      /* Remember the just read results for later reading. */
      *pLastResults = *pCalibrate;

      TRACE(VMMC, DBG_LEVEL_LOW,
            ("Calibration results on channel %d:\n"
             "  TX path offset %i\n"
             "  Measurement equipment offset %i\n"
             "  RX DC path offset %i\n"
             "  RX DC path offset %i\n"
             "  IDAC gain correction %i\n",
             pCh->nChannel - 1,
             (IFX_int16_t)pCalibrate->TxOffset,
             (IFX_int16_t)pCalibrate->MeOffset,
             (IFX_int16_t)pCalibrate->UlimOffset30,
             (IFX_int16_t)pCalibrate->UlimOffset60,
             (IFX_int16_t)pCalibrate->IdacGain));
      if (pDev->sdd.bDcDcHwCombined == IFX_TRUE)
      {
         TRACE(VMMC, DBG_LEVEL_LOW,
               ("  Combined DC/DC voltage correction %i\n",
                (IFX_int16_t)pCalibrate->Vdcdc));
      }
   }

   /* If reading the values failed or range check results that values are
      out of range reset to zero offset values to prevent that implausible
      values are used. */
   if (!VMMC_SUCCESS(ret) ||
       ((IFX_int16_t)pCalibrate->TxOffset > VMMC_SDD_CALIBRATE_TXOFFSET_MAX) ||
       ((IFX_int16_t)pCalibrate->TxOffset < VMMC_SDD_CALIBRATE_TXOFFSET_MIN) ||
       ((IFX_int16_t)pCalibrate->MeOffset > VMMC_SDD_CALIBRATE_MEOFFSET_MAX) ||
       ((IFX_int16_t)pCalibrate->MeOffset < VMMC_SDD_CALIBRATE_MEOFFSET_MIN) ||
       ((IFX_int16_t)pCalibrate->UlimOffset30 > VMMC_SDD_CALIBRATE_ULIMOFFSET30_MAX) ||
       ((IFX_int16_t)pCalibrate->UlimOffset30 < VMMC_SDD_CALIBRATE_ULIMOFFSET30_MIN) ||
       ((IFX_int16_t)pCalibrate->UlimOffset60 > VMMC_SDD_CALIBRATE_ULIMOFFSET60_MAX) ||
       ((IFX_int16_t)pCalibrate->UlimOffset60 < VMMC_SDD_CALIBRATE_ULIMOFFSET60_MIN) ||
       ((IFX_int16_t)pCalibrate->IdacGain > VMMC_SDD_CALIBRATE_IDACGAIN_MAX) ||
       ((IFX_int16_t)pCalibrate->IdacGain < VMMC_SDD_CALIBRATE_IDACGAIN_MIN) ||
       ((pDev->sdd.bDcDcHwCombined == IFX_TRUE) &&
        (((IFX_int16_t)pCalibrate->Vdcdc < VMMC_SDD_CALIBRATE_VDCDC_MIN) ||
         ((IFX_int16_t)pCalibrate->Vdcdc > VMMC_SDD_CALIBRATE_VDCDC_MAX))  )
      )
   {
      /* At least one of the values is out of range -> reset to defaults. */
      pCalibrate->TxOffset       = 0;
      pCalibrate->MeOffset       = 0;
      pCalibrate->UlimOffset30   = 0;
      pCalibrate->UlimOffset60   = 0;
      pCalibrate->IdacGain       = 0;
      pCalibrate->Vdcdc          = 0;
      ret = CmdWrite (pDev, (IFX_uint32_t *)pCalibrate, SDD_Calibrate_LEN);

      /* Set flag that calibration failed */
      pCh->pALM->nCalibrationState = IFX_TAPI_CALIBRATION_STATE_FAILED;
   }
   else
   {
      pCh->pALM->nCalibrationState = IFX_TAPI_CALIBRATION_STATE_DONE;
   }

   pCh->pALM->bCalibrationNeeded = IFX_FALSE;

   if (pCh->pALM->bCalibrationInternal)
   {
      /* Wake the waiting call in ALM_Calibration below. */
      VMMC_OS_MutexRelease(&pCh->pALM->mtxCalibrationWait);
   }
   else
   {
      /* release protection */
      VMMC_OS_MutexRelease (&pCh->chAcc);

      /* Send the calibration-end event for the application */
      memset(&tapiEvent, 0, sizeof(IFX_TAPI_EVENT_t));
      tapiEvent.ch = pCh->nChannel - 1;
      tapiEvent.id = IFX_TAPI_EVENT_CALIBRATION_END;
      tapiEvent.module = IFX_TAPI_MODULE_TYPE_ALM;
      if (pCh->pALM->nCalibrationState == IFX_TAPI_CALIBRATION_STATE_DONE)
      {
         tapiEvent.data.calibration = IFX_TAPI_EVENT_CALIBRATION_SUCCESS;
      }
      else
      {
         tapiEvent.data.calibration = IFX_TAPI_EVENT_CALIBRATION_ERROR_RANGE;
      }
      IFX_TAPI_Event_Dispatch(pCh->pTapiCh, &tapiEvent);
   }

   RETURN_STATUS(ret);
}


/**
   Used internal to start calibration on the given analog line.

   This function is called after a BBD download was done to calibrate the
   analog channel where the download has just been done. This function blocks
   until the finished event has been received and the validation of the
   calibration was done. This function passes a parameter to prevent the
   sending of the TAPI event IFX_TAPI_EVENT_CALIBRATION_END and at the
   same time initiate the wakeup from the blocking wait.

   \param  pCh          Pointer to the VMMC channel structure.

   \return
   - VMMC_statusOk                   if successful
   - VMMC_statusInvalCh              Channel number out of range
*/
IFX_int32_t VMMC_ALM_Calibration (VMMC_CHANNEL *pCh)
{
   IFX_int32_t ret;

   /* sanity check */
   VMMC_ASSERT(pCh != IFX_NULL);
   VMMC_ASSERT(pCh->pALM != IFX_NULL);

   /* protect channel from mutual access */
   VMMC_OS_MutexGet (&pCh->chAcc);

   ret = vmmc_alm_Calibration_Start (pCh, IFX_TRUE);

   if (VMMC_SUCCESS (ret))
   {
      /* Block until the semaphore is given in the calibration finished
         function or a signal interrupts. */
      ret = VMMC_OS_MutexLockInterruptible(&pCh->pALM->mtxCalibrationWait);
      if (ret != 0)
      {
         ret = VMMC_statusErr;
      }
   }

   /* release protection */
   VMMC_OS_MutexRelease (&pCh->chAcc);

   RETURN_STATUS (ret);
}


/**
   Prepare parameters and call the target configuration function to switch the
   line mode.

   A check is done, whether the channel supports a analog line. If not
   VMMC_statusInvalCh is returned.
   This function is blocking and must not be called from timer or interrupt
   context.

   \param  pLLChannel      Pointer to the VMMC channel structure.
   \param  nMode           Linefeed mode.
   \param  nTapiLineMode   The currently set line mode. Usage in discussion.

   \return
   - VMMC_statusInvalCh the channel does not support a analog line
   - VMMC_statusParam The parameters are wrong
   - VMMC_statusCmdWr Writing the command has failed
   - VMMC_statusOk if successful
*/
/*lint -esym(715, nTapiLineMode) */
IFX_int32_t VMMC_TAPI_LL_ALM_Line_Mode_Set (IFX_TAPI_LL_CH_t *pLLChannel,
                                           IFX_int32_t nMode,
                                           IFX_uint8_t nTapiLineMode)
{
   VMMC_CHANNEL *pCh  = (VMMC_CHANNEL *) pLLChannel;
   VMMC_CHANNEL *pOtherCh = IFX_NULL;
   IFX_int32_t ret = VMMC_statusOk;
   VMMC_SDD_Opmode_t *pOpmod;
   IFX_uint8_t curr_opmode, other_curr_opmode;
   VMMC_UNUSED(nTapiLineMode);

   /* sanity check */
   VMMC_ASSERT(pCh != IFX_NULL);
   if (pCh->pALM == IFX_NULL)
   {
      /* errmsg: Resource not valid. Channel number out of range */
      RETURN_STATUS (VMMC_statusInvalCh);
   }

   if (pCh->pALM->line_type_fxs == IFX_FALSE &&
          nMode != IFX_TAPI_LINE_FEED_DISABLED)
   {
      /* errmsg: Protection from calling on FXO line;
         however PDH is allowed, e.g. for IFX_TAPI_DEV_STOP */
      RETURN_STATUS (VMMC_statusFXSCallOnFXO);
   }

   /* For DC/DC converters other than default a BBD download is required. */
   if ((pCh->pALM->line_type_fxs == IFX_TRUE) &&
       (pCh->pALM->nDcDcType == VMMC_DCDC_TYPE_DEFAULT_IBB) &&
       (pCh->pParent->sdd.nAllowedDcDcType != VMMC_DCDC_TYPE_IBB))
   {
      /* errmsg: Operation blocked until a BBD with DC/DC configuration
                 matching the connected DC/DC hardware type is downloaded */
      RETURN_STATUS (VMMC_statusBlockedDcDcTypeMissmatch);
   }

#ifdef VMMC_FEAT_CAP_MEASURE
   if ((nMode != IFX_TAPI_LINE_FEED_DISABLED) &&
       (pCh->pALM->eCapMeasState != VMMC_CapMeasState_Inactive))
   {
      /* errmsg: Action not possible when
         capacitance measurement is active */
      RETURN_STATUS (VMMC_statusFailCapMeasActive);
   }
#endif /* VMMC_FEAT_CAP_MEASURE */

   pOpmod = &pCh->pALM->fw_sdd_opmode;

   /* TODO: implicitly turn off the MWL on any line mode change */

   /* Get the current opmode - wait while a opmode change is ongoing. */
   ret = VMMC_ALM_OpmodeGet (pCh, &curr_opmode);
   /* Only exit when interrupted by a signal. When waiting was aborted by the
      timeout assume that no linemode change is pending and try to continue
      with the opmode change that was requested. */
   if (ret == VMMC_statusSddEvtWaitInterrupt)
   {
      RETURN_STATUS (ret);
   }

   /* check if transition is valid, prepare command */
   switch (nMode)
   {
      case IFX_TAPI_LINE_FEED_ACTIVE:
      case IFX_TAPI_LINE_FEED_RING_PAUSE:
      /* for backward compatibility only */
      case IFX_TAPI_LINE_FEED_NORMAL_AUTO:
      case IFX_TAPI_LINE_FEED_ACTIVE_LOW:
      case IFX_TAPI_LINE_FEED_ACTIVE_BOOSTED:
         switch (curr_opmode)
         {
            case VMMC_SDD_OPMODE_DISABLED:
            case VMMC_SDD_OPMODE_ACTIVE:
            case VMMC_SDD_OPMODE_RING_BURST:
            case VMMC_SDD_OPMODE_STANDBY:
               pOpmod->OperatingMode = VMMC_SDD_OPMODE_ACTIVE;
               /* in case of ring pause, leave reverse
                  polarity bit as is */
               if (nMode != IFX_TAPI_LINE_FEED_RING_PAUSE)
                  pOpmod->Polarity = VMMC_OPMOD_POL_NORMAL;
               break;
            default:
               /* errmsg: Line mode switch is invalid. Not every transition
                  is valid. */
               RETURN_INFO_STATUS (VMMC_statusInvalLMSwitch,
                                   &nMode, sizeof(nMode));
         }
         break;
      case IFX_TAPI_LINE_FEED_ACTIVE_REV:
      /* for backward compatibility only */
      case IFX_TAPI_LINE_FEED_REVERSED_AUTO:
         switch (curr_opmode)
         {
            case VMMC_SDD_OPMODE_ACTIVE:
            case VMMC_SDD_OPMODE_RING_BURST:
            case VMMC_SDD_OPMODE_STANDBY:
               pOpmod->OperatingMode = VMMC_SDD_OPMODE_ACTIVE;
               pOpmod->Polarity = VMMC_OPMOD_POL_REVERSE;
               break;
            default:
               /** Line mode switch is invalid. Not every transition is valid. */
               RETURN_INFO_STATUS (VMMC_statusInvalLMSwitch,
                                   &nMode, sizeof(nMode));
         }
         break;
      case IFX_TAPI_LINE_FEED_PARKED_REVERSED:
      case IFX_TAPI_LINE_FEED_STANDBY:
         switch (curr_opmode)
         {
            case VMMC_SDD_OPMODE_ACTIVE:
            case VMMC_SDD_OPMODE_RING_BURST:
            case VMMC_SDD_OPMODE_STANDBY:
            case VMMC_SDD_OPMODE_DISABLED:
               pOpmod->OperatingMode = VMMC_SDD_OPMODE_STANDBY;
               pOpmod->Polarity = VMMC_OPMOD_POL_NORMAL;
               break;
            default:
               /** Line mode switch is invalid. Not every transition is valid. */
               RETURN_INFO_STATUS (VMMC_statusInvalLMSwitch,
                                   &nMode, sizeof(nMode));
         }
         break;
      case IFX_TAPI_LINE_FEED_DISABLED:
         pOpmod->OperatingMode = VMMC_SDD_OPMODE_DISABLED;
         pOpmod->Polarity = VMMC_OPMOD_POL_NORMAL;
         break;
      case IFX_TAPI_LINE_FEED_RING_BURST:
         switch (curr_opmode)
         {
            case VMMC_SDD_OPMODE_ACTIVE:         /* check for hook in case of active? */
            case VMMC_SDD_OPMODE_RING_BURST:
            case VMMC_SDD_OPMODE_STANDBY:
               /* leave reversal bit as is */
               pOpmod->OperatingMode = VMMC_SDD_OPMODE_RING_BURST;
               break;
            default:
               /* Line mode switch is invalid. Not every transition is valid. */
               RETURN_INFO_STATUS (VMMC_statusInvalLMSwitch,
                                   &nMode, sizeof(nMode));
         }
         break;
      /* unsupported linemodes */
      case IFX_TAPI_LINE_FEED_HIGH_IMPEDANCE:
      case IFX_TAPI_LINE_FEED_METER:
      case IFX_TAPI_LINE_FEED_ACT_TEST:
      case IFX_TAPI_LINE_FEED_ACT_TESTIN:
      case IFX_TAPI_LINE_FEED_DISABLED_RESISTIVE_SWITCH:
      default:
         RETURN_INFO_STATUS (VMMC_statusNotSupported,
                             &nMode, sizeof(nMode));
   }


   /* In combined DC/DC mode mode changing the linemode to anything other
      than disabled is only allowed if the neighbour channel is not doing
      calibration or linetesting */
   if ((pCh->pALM->line_type_fxs == IFX_TRUE) &&
       (pCh->pParent->sdd.bDcDcHwCombined == IFX_TRUE) &&
       (nMode != IFX_TAPI_LINE_FEED_DISABLED))
   {
      /* The combined DC/DC flag is only set on 2 channel systems. So there
         is always a neighbouring channel. */
      pOtherCh = VMMC_ALM_FxsNeighbourChGet(pCh);
      if (pOtherCh == IFX_NULL)
      {
         /* errmsg: Resource not valid. Channel number out of range */
         RETURN_STATUS (VMMC_statusInvalCh);
      }
      /* protect other channel from mutual access */
      VMMC_OS_MutexGet (&pOtherCh->chAcc);

      /* Get the current opmode - wait while a opmode change is ongoing. */
      ret = VMMC_ALM_OpmodeGet (pOtherCh, &other_curr_opmode);
      /* Only exit when interrupted by a signal. When waiting was aborted by
         timeout assume that no linemode change is pending and try to continue
         with the opmode change that was requested. */
      if (ret == VMMC_statusSddEvtWaitInterrupt)
      {
         /* release protection */
         VMMC_OS_MutexRelease (&pOtherCh->chAcc);
         RETURN_STATUS (ret);
      }

      if ((other_curr_opmode == VMMC_SDD_OPMODE_GR909) ||
          (other_curr_opmode == VMMC_SDD_OPMODE_CALIBRATE))
      {
         /* release protection */
         VMMC_OS_MutexRelease (&pOtherCh->chAcc);
         /* errmsg: Linefeed change not allowed while neighbour line is doing
                    linetesting. */
         RETURN_STATUS (VMMC_statusNeighbourLineBlocksLMSwitch);
      }
   }

   ret = VMMC_ALM_OpmodeSet (pCh);

   if (pOtherCh != IFX_NULL)
   {
      /* release protection */
      VMMC_OS_MutexRelease (&pOtherCh->chAcc);
   }

   return ret;
}
/*lint +esym(715, nTapiLineMode) */

/**
   Read the current operating mode and report to TAPI.
   Reports IFX_TAPI_LINE_FEED_DISABLED if called during PDH,
   FXO, Calibration or GR909.

   \param  pLLChannel      Pointer to the VMMC channel structure.
   \param  pFeed           Pointer to TAPI IFX_TAPI_LINE_FEED_t variable.

   \return
   - VMMC_statusOk         always
*/
IFX_int32_t VMMC_TAPI_LL_ALM_Line_Mode_Get (IFX_TAPI_LL_CH_t *pLLChannel,
                                            IFX_TAPI_LINE_FEED_t *pFeed)
{
   VMMC_CHANNEL *pCh  = (VMMC_CHANNEL *) pLLChannel;
   IFX_TAPI_LINE_MODE_t nMode;
   IFX_uint8_t curr_opmode;
   IFX_int32_t ret;

   /* parameter check */
   VMMC_ASSERT(pCh != IFX_NULL);
   if (pCh->pALM == IFX_NULL)
   {
      /* errmsg: Resource not valid. Channel number out of range */
      RETURN_STATUS (VMMC_statusInvalCh);
   }

   /* Get the current opmode - wait while a opmode change is ongoing. */
   ret = VMMC_ALM_OpmodeGet (pCh, &curr_opmode);
   /* Only exit when interrupted by a signal. When waiting was aborted by the
      timeout assume that no linemode change is pending and try to continue. */
   if (ret == VMMC_statusSddEvtWaitInterrupt)
   {
      RETURN_STATUS (ret);
   }

   switch (curr_opmode)
   {
      case VMMC_SDD_OPMODE_ACTIVE:
         if (pCh->pALM->curr_polarity == VMMC_OPMOD_POL_NORMAL)
            nMode = IFX_TAPI_LINE_FEED_ACTIVE;
         else
            nMode = IFX_TAPI_LINE_FEED_ACTIVE_REV;
         break;

      case VMMC_SDD_OPMODE_STANDBY:
         nMode = IFX_TAPI_LINE_FEED_STANDBY;
         break;

      case VMMC_SDD_OPMODE_RING_BURST:
         nMode = IFX_TAPI_LINE_FEED_RING_BURST;
         break;

      /* report disabled status in case of PDH,
         FXO, Calibration and GR909 */
      default:
         nMode = IFX_TAPI_LINE_FEED_DISABLED;
         break;
   }

#ifdef TAPI_ONE_DEVNODE
   pFeed->lineMode = nMode;
#else  /* TAPI_ONE_DEVNODE */
   *pFeed = nMode;
#endif /* TAPI_ONE_DEVNODE */

   return VMMC_statusOk;
}

/**
   Set line type to FXS or FXO and the sampling mode operation.

   In case switching to FXS line type, the line mode used is Power Down High
   Impedance (PDH). For FXO the line type FXO is set.
   The sampling rate for FXS can be narrowband, wideband or automatic switching.
   For FXO the sampling rate is always narrowband.
   A check is done, whether the channel number is not higher then the supported
   analog lines. If not VMMC_statusInvalCh is returned.

   \param  pLLChannel      Pointer to the VMMC channel structure.
   \param  nType           New line type, can be IFX_TAPI_LINE_TYPE_FXS or
                           IFX_TAPI_LINE_TYPE_FXO.

   \return
   - VMMC_statusInvalCh    Channel number out of range
   - VMMC_statusParam      The parameters are wrong
   - VMMC_statusCmdWr      Writing the command has failed
   - VMMC_statusOk         If successful
*/
IFX_int32_t VMMC_TAPI_LL_ALM_Line_Type_Set(IFX_TAPI_LL_CH_t *pLLChannel,
                                           IFX_TAPI_LINE_TYPE_t nType)
{
   VMMC_CHANNEL *pCh  = (VMMC_CHANNEL *) pLLChannel;
   VMMC_DEVICE  *pDev;
   IFX_int32_t   ret = VMMC_statusOk;

   /* parameter check */
   VMMC_ASSERT(pCh != IFX_NULL);
   if (pCh->pALM == IFX_NULL)
   {
      /* errmsg: Resource not valid. Channel number out of range */
      RETURN_STATUS (VMMC_statusInvalCh);
   }

   pDev   = pCh->pParent;

   /* check if setting is valid, set opmode upon changes between FXS and FXO */
   switch (nType)
   {
      case IFX_TAPI_LINE_TYPE_FXS_NB:
      case IFX_TAPI_LINE_TYPE_FXS_WB:
      case IFX_TAPI_LINE_TYPE_FXS_AUTO:
         if (pCh->pALM->line_type_fxs == IFX_FALSE)
         {
            /* SmartSLIC has fixed channel types.
               errmsg: Line type change not allowed */
            RETURN_STATUS (VMMC_statusLineTypChNotAll);
         }
         /* Change to FXS only if not already in FXS mode.
            This prevents switch to PDH when already in FXS mode. */
         if (pCh->pALM->line_type_fxs != IFX_TRUE)
         {
            pCh->pALM->line_type_fxs = IFX_TRUE;
            TRACE(VMMC, DBG_LEVEL_NORMAL,
                  ("INFO: VMMC_TAPI_LL_ALM_Line_Type_Set: Dev%d,Ch%d: "
                   "switch to FXS line type\n",
                   pDev->nDevNr, pCh->nChannel - 1));

            ret = VMMC_ALM_OpmodeModeSet (pCh, VMMC_SDD_OPMODE_DISABLED);
         }
         break;

      case IFX_TAPI_LINE_TYPE_FXO:
         if (pCh->pALM->line_type_fxs != IFX_FALSE)
         {
            /* SmartSLIC has fixed channel types.
               errmsg: Line type change not allowed */
            RETURN_STATUS (VMMC_statusLineTypChNotAll);
         }
         pCh->pALM->line_type_fxs = IFX_FALSE;
         TRACE(VMMC, DBG_LEVEL_NORMAL,
                  ("INFO: VMMC_TAPI_LL_ALM_Line_Type_Set: Dev%d,Ch%d: "
                   "switch to FXO line type\n",
                   pDev->nDevNr, pCh->nChannel - 1));

         /* switching to linemode FXO requires to switch to PDH first... */
         ret = VMMC_ALM_OpmodeModeSet(pCh, VMMC_SDD_OPMODE_DISABLED);
         ret = VMMC_ALM_OpmodeModeSet(pCh, VMMC_SDD_OPMODE_FXO);
         break;

      default:
         TRACE(VMMC, DBG_LEVEL_NORMAL,
               ("Invalid line type / sampling mode %d for analog line\n", nType));
         /* errmsg: Invalid line type or sampling mode for analog line */
         RETURN_STATUS (VMMC_statusInvalAlmTypeSmpl);
   }

   /* Set the module's ability to switch between narrowband and wideband
      operation mode. */
   switch (nType)
   {
      default:
         /* just to make the compiler happy; case is already blocked above */
      case IFX_TAPI_LINE_TYPE_FXO:
      case IFX_TAPI_LINE_TYPE_FXS_NB:
         VMMC_CON_ModuleSamplingModeSet (pCh, VMMCDSP_MT_ALM, VMMC_CON_SMPL_NB);
         break;
      case IFX_TAPI_LINE_TYPE_FXS_WB:
         VMMC_CON_ModuleSamplingModeSet (pCh, VMMCDSP_MT_ALM, VMMC_CON_SMPL_WB);
         break;
      case IFX_TAPI_LINE_TYPE_FXS_AUTO:
         VMMC_CON_ModuleSamplingModeSet (pCh, VMMCDSP_MT_ALM, VMMC_CON_SMPL_AUTO);
         break;
   }

   if (VMMC_SUCCESS (ret))
   {
      /* reevaluate sampling in the conference that this module belongs to */
      ret = VMMC_CON_MatchConfSmplRate(pCh, VMMCDSP_MT_ALM);
   }

   return ret;
}


/**
   This test service generates hook events.

   \param  pLLChannel      Pointer to the VMMC channel structure.
   \param  bHook           IFX_TRUE generate off-hook,
                           IFX_FALSE generates on-hook.

   \return
   - VMMC_statusInvalCh the channel does not support a analog line
   - VMMC_statusOk if successful

   \remarks
   The hook event is triggered by sending the same TAPI event from this function
   that is normally sent by he interrupt handler when the HW reports an hook.
   The hook event then gets to the hook state machine for validation. Depending
   on the timing of calling this interface also hook flash and pulse dialing
   can be verified.
*/
IFX_int32_t VMMC_TAPI_LL_ALM_VMMC_Test_HookGen(IFX_TAPI_LL_CH_t *pLLChannel,
                                               IFX_boolean_t bHook)
{
   VMMC_CHANNEL *pCh = (VMMC_CHANNEL *) pLLChannel;
   IFX_TAPI_EVENT_t tapiEvent;

   VMMC_ASSERT(pCh != IFX_NULL);
   if (pCh->pALM == IFX_NULL)
   {
      /* errmsg: Resource not valid. Channel number out of range */
      RETURN_STATUS (VMMC_statusInvalCh);
   }

   /* Fill event structure. */
   memset(&tapiEvent, 0, sizeof(IFX_TAPI_EVENT_t));
   tapiEvent.module = IFX_TAPI_MODULE_TYPE_ALM;

   if(bHook == IFX_TRUE)
      tapiEvent.id = IFX_TAPI_EVENT_FXS_OFFHOOK_INT;
   else
      tapiEvent.id = IFX_TAPI_EVENT_FXS_ONHOOK_INT;

   IFX_TAPI_Event_Dispatch(pCh->pTapiCh, &tapiEvent);

   return VMMC_statusOk;
}


/**
   This service controls an 8 kHz loop in the device or the driver for testing.
   Additionally the signals will pass through to the line undisturbed.

   \param  pLLChannel      Pointer to the VMMC channel structure.
   \param  pLoop           if IFX_TRUE a loop is switched on
                           if IFX_FALSE a loop is switched off

   \return
   - VMMC_statusInvalCh    Channel number out of range
   - VMMC_statusParam      The parameters are wrong
   - VMMC_statusCmdWr      Writing the command has failed
   - VMMC_statusOk         If successful
*/
IFX_int32_t VMMC_TAPI_LL_ALM_VMMC_Test_Loop(IFX_TAPI_LL_CH_t *pLLChannel,
                                            IFX_TAPI_TEST_LOOP_t const* pLoop)
{
   VMMC_CHANNEL *pCh = (VMMC_CHANNEL *) pLLChannel;
   VMMC_DEVICE *pDev = pCh->pParent;
   IFX_int32_t ret;

   VMMC_ASSERT(pCh != IFX_NULL);
   if (pCh->pALM == IFX_NULL)
   {
      /* errmsg: Resource not valid. Channel number out of range */
      RETURN_STATUS (VMMC_statusInvalCh);
   }

   {
      IFX_uint32_t dcctrlLoop[2];
      IFX_uint32_t ch = (IFX_uint32_t)(pCh->nChannel - 1);

      memset (&dcctrlLoop, 0, sizeof (dcctrlLoop));
      dcctrlLoop[0] = 0x04001E04L | (ch & 0x0000000FL) << 16;

      /* depending on the request enable or disable testloop */
      if (pLoop->bAnalog)
      {
         /* enable debug operation */
         dcctrlLoop[1] = 0x00000001L;
      }
      else
      {
         /* enable normal operation */
         dcctrlLoop[1] = 0x00000000L;
      }

      /* write the message */
      ret = CmdWrite (pDev, dcctrlLoop, 4);
   }

   RETURN_STATUS (ret);
}


/**
   Set the phone volume

   Gain Parameter are given in 'dB'. The range is -24dB ... 12dB.
   This function enables the ALM in firmware. There will be
   no error reported if it is not enabled before (IFX_TAPI_CH_INIT done).

   \param  pLLChannel      Pointer to the VMMC channel structure.
   \param  pVol            Pointer to IFX_TAPI_LINE_VOLUME_t structure.

   \return
   - VMMC_statusInvalCh    Channel number out of range
   - VMMC_statusCmdWr Writing the command has failed
   - VMMC_statusParamRange Wrong parameters passed. This code is returned
     when any gain parameter is lower than -24 dB or higher than 12 dB
   - VMMC_statusOk if successful
*/
IFX_int32_t VMMC_TAPI_LL_ALM_Volume_Set (IFX_TAPI_LL_CH_t *pLLChannel,
                                         IFX_TAPI_LINE_VOLUME_t const *pVol)
{
   VMMC_CHANNEL   *pCh = (VMMC_CHANNEL *) pLLChannel;
   IFX_int32_t     ret;

   /* sanity check */
   VMMC_ASSERT(pCh != IFX_NULL);
   if (pCh->pALM == IFX_NULL)
   {
      /* errmsg: Resource not valid. Channel number out of range */
      RETURN_STATUS (VMMC_statusInvalCh);
   }

   /* range check, because gain var is integer */
   if ((pVol->nGainTx < VMMC_VOLUME_GAIN_MIN) ||
       (pVol->nGainTx > VMMC_VOLUME_GAIN_MAX) ||
       (pVol->nGainRx < VMMC_VOLUME_GAIN_MIN) ||
       (pVol->nGainRx > VMMC_VOLUME_GAIN_MAX))
   {
      /* errmsg: parameters are out of the supported range */
      RETURN_STATUS (VMMC_statusParamRange);
   }

   /* protect fw msg */
   VMMC_OS_MutexGet (&pCh->chAcc);

   pCh->pALM->fw_ali_ch.DG1 = VMMC_Gaintable[pVol->nGainTx +
                                             (-VMMC_VOLUME_GAIN_MIN)];
   pCh->pALM->fw_ali_ch.DG2 = VMMC_Gaintable[pVol->nGainRx +
                                             (-VMMC_VOLUME_GAIN_MIN)];
   pCh->pALM->fw_ali_ch.EN  = ALI_CHAN_ENABLE;

   ret = CmdWrite (pCh->pParent,
                   (IFX_uint32_t *) &(pCh->pALM->fw_ali_ch), ALI_CHAN_LEN);

   /* release fw protection */
   VMMC_OS_MutexRelease (&pCh->chAcc);

   return ret;
}


/**
   This service enables or disables a high level path of a phone channel.

   It is intended for phone channels only and must be used in
   combination with IFX_TAPI_PHONE_VOLUME_SET or IFX_TAPI_PCM_VOLUME_SET
   to set the max. level (IFX_TAPI_LINE_VOLUME_HIGH) or to restore level

   \param  pLLChannel      Pointer to the VMMC channel structure.
   \param  bEnable         The parameter represent a boolean value of
                           IFX_TAPI_LINE_LEVEL_t.
                           - 0: IFX_TAPI_LINE_LEVEL_DISABLE,
                                disable the high level path.
                           - 1: IFX_TAPI_LINE_LEVEL_ENABLE,
                                enable the high level path.

   \return
   - VMMC_statusInvalCh    Channel number out of range
   - VMMC_statusCmdWr Writing the command failed
   - VMMC_statusOk if successful

*/
IFX_int32_t VMMC_TAPI_LL_ALM_Volume_High_Level  (IFX_TAPI_LL_CH_t *pLLChannel,
                                                 IFX_int32_t bEnable)
{
   VMMC_CHANNEL   *pCh = (VMMC_CHANNEL *) pLLChannel;
   VMMC_SDD_Opmode_t *pOpmod;
   IFX_int32_t    ret;

   /* sanity check */
   VMMC_ASSERT(pCh != IFX_NULL);
   if (pCh->pALM == IFX_NULL)
   {
      /* errmsg: Resource not valid. Channel number out of range */
      RETURN_STATUS (VMMC_statusInvalCh);
   }

   pOpmod = &pCh->pALM->fw_sdd_opmode;

   /* Update the cached message with the latest values from the FW because
      the FW may have autonomously changed the opmode due to an off-hook. */
   ret = CmdRead(pCh->pParent, (IFX_uint32_t *)pOpmod,
                               (IFX_uint32_t *)pOpmod, SDD_Opmode_LEN);

   if (VMMC_SUCCESS (ret))
   {
      pOpmod->Howler = bEnable ? 1 : 0;

      ret = VMMC_ALM_OpmodeSet (pCh);
   }

   return ret;
}


/**
   Sets the LEC configuration on the analog line.

   \param  pLLChannel   Handle to TAPI low level channel structure.
   \param  pLecConf     Handle to IFX_TAPI_LEC_CFG_t structure.

   \return
   - VMMC_statusInvalCh    Channel number out of range
   - VMMC_statusFuncParm Wrong parameters passed. This code is returned
     when the nOpMode parameter has an invalid value.
   - VMMC_statusNotSupported Requested action is not supported. This code
     is returned when a WLEC is requested but not supported by the VoFW.
   - VMMC_statusNoRes No free LEC resources is available
   - VMMC_statusCmdWr Writing the command has failed
   - VMMC_statusOk if successful
*/
IFX_int32_t VMMC_TAPI_LL_ALM_LEC_Set (IFX_TAPI_LL_CH_t *pLLChannel,
                                     TAPI_LEC_DATA_t *pLecConf)
{
   VMMC_CHANNEL *pCh  = (VMMC_CHANNEL *) pLLChannel;
   IFX_int32_t   ret  = VMMC_statusOk;
   IFX_enDis_t   bEnES,
                 bEnNLP = IFX_DISABLE;

   /* sanity check */
   VMMC_ASSERT(pCh != IFX_NULL);
   if (pCh->pALM == IFX_NULL)
   {
      /* errmsg: Resource not valid. Channel number out of range */
      RETURN_STATUS (VMMC_statusInvalCh);
   }

   /* Needed for LEC and ES activation. */
   bEnES = ((pLecConf->nOpMode == IFX_TAPI_WLEC_TYPE_ES) ||
            (pLecConf->nOpMode == IFX_TAPI_WLEC_TYPE_NE_ES) ||
            (pLecConf->nOpMode == IFX_TAPI_WLEC_TYPE_NFE_ES)) ?
           IFX_ENABLE : IFX_DISABLE;

   /* protect channel variables */
   VMMC_OS_MutexGet (&pCh->chAcc);

   if ((pLecConf->nOpMode == IFX_TAPI_WLEC_TYPE_OFF) ||
       (pLecConf->nOpMode == IFX_TAPI_WLEC_TYPE_ES)    )
   {
      /* Disable the line echo canceller. */

      if (VMMC_RES_ID_VALID(pCh->pALM->nLecResId))
      {
         /* Disable the LEC, free resource and forget the resource ID. */
         ret = VMMC_RES_LEC_Enable (pCh->pALM->nLecResId, IFX_DISABLE);
         if (VMMC_SUCCESS (ret))
         {
            ret = VMMC_RES_LEC_Release (pCh->pALM->nLecResId);
         }
         if (VMMC_SUCCESS (ret))
         {
            pCh->pALM->nLecResId = VMMC_RES_ID_NULL;
         }
      }
   }
   else
   {
      /* Enable the line echo canceller. */

      VMMC_RES_LEC_MODE_t nOperatingMode;

      /* Note: ALM is always enabled so we do not need to check for this and
         can start right away to enable the LEC and/or the ES. */

      /* Translate the operating mode parameter */
      switch (pLecConf->nOpMode)
      {
         case  IFX_TAPI_WLEC_TYPE_NE:
         case  IFX_TAPI_WLEC_TYPE_NE_ES:
            nOperatingMode = VMMC_RES_LEC_MODE_NLEC;
            break;
         case  IFX_TAPI_WLEC_TYPE_NFE:
         case  IFX_TAPI_WLEC_TYPE_NFE_ES:
            nOperatingMode = VMMC_RES_LEC_MODE_WLEC;
            break;
         default:
            VMMC_OS_MutexRelease (&pCh->chAcc);
            RETURN_STATUS (VMMC_statusParam);
      }

      /* Translate the NLP parameter */
      bEnNLP = (pLecConf->bNlp == IFX_TAPI_WLEC_NLP_OFF) ?
               IFX_DISABLE : IFX_ENABLE;

      /* Do parameter checks and corrections on the window size parameters */
      ret = VMMC_RES_LEC_CoefWinValidate (VMMC_RES_MOD_ALM,
                                          nOperatingMode, pLecConf);
      if (!VMMC_SUCCESS (ret))
      {
         VMMC_OS_MutexRelease (&pCh->chAcc);
         RETURN_STATUS (ret);
      }

      if (!VMMC_RES_ID_VALID(pCh->pALM->nLecResId))
      {
         /* allocate a LEC resource */
         pCh->pALM->nLecResId = VMMC_RES_LEC_Allocate (pCh, VMMC_RES_MOD_ALM);

         if (!VMMC_RES_ID_VALID(pCh->pALM->nLecResId))
         {
            VMMC_OS_MutexRelease (&pCh->chAcc);
            RETURN_STATUS (VMMC_statusNoRes);
         }

         /* set the current sampling mode */
         VMMC_RES_LEC_SamplingModeSet (pCh->pALM->nLecResId,
                                       pCh->pALM->module_mode);
      }

      VMMC_RES_LEC_OperatingModeSet (pCh->pALM->nLecResId,
                                     nOperatingMode, bEnNLP);

      /* Set window sizes for 8kHz sampling mode */
      switch (nOperatingMode)
      {
         default:
         case VMMC_RES_LEC_MODE_NLEC:
            /* NLEC operating mode */
            VMMC_RES_LEC_CoefWinSet (pCh->pALM->nLecResId,
                                     NB_8_KHZ, nOperatingMode,
                                     (IFX_uint8_t)pLecConf->nNBNEwindow, 0);
            break;
         case VMMC_RES_LEC_MODE_WLEC:
            /* WLEC operating mode */
            VMMC_RES_LEC_CoefWinSet (pCh->pALM->nLecResId,
                                     NB_8_KHZ, nOperatingMode,
                                     (IFX_uint8_t)pLecConf->nNBNEwindow +
                                     (IFX_uint8_t)pLecConf->nNBFEwindow,
                                     (IFX_uint8_t)pLecConf->nNBFEwindow);
            break;
      }

      /* Set window sizes for 16kHz sampling mode */
      VMMC_RES_LEC_CoefWinSet (pCh->pALM->nLecResId,
                               WB_16_KHZ, nOperatingMode,
                               (IFX_uint8_t)pLecConf->nWBNEwindow, 0);

      /* Set information if the ES is also working on this channel */
      VMMC_RES_LEC_ParameterSelect (pCh->pALM->nLecResId, bEnES);

      /* Enable the echo suppressor resource */
      ret = VMMC_RES_LEC_Enable (pCh->pALM->nLecResId, IFX_ENABLE);

#ifdef VMMC_FEAT_RTCP_XR
      /* Set association between LEC and COD. */
      if (VMMC_SUCCESS(ret))
      {
         ret = VMMC_RES_LEC_AssociatedCodSet(
                     pCh->pALM->nLecResId,
                     VMMC_CON_SingleDataChannelCodFind(pCh, VMMCDSP_MT_ALM));
      }
#endif /* VMMC_FEAT_RTCP_XR */
   }

   VMMC_OS_MutexRelease (&pCh->chAcc);

   if (VMMC_SUCCESS(ret))
   {
      /* Turn the echo suppressor on or off as requested. */
      if (bEnES == IFX_ENABLE)
      {
         ret = vmmc_alm_Echo_Suppressor_Set (pLLChannel, IFX_ENABLE, bEnNLP);
      }
      else
      {
         ret = vmmc_alm_Echo_Suppressor_Set (pLLChannel, IFX_DISABLE, bEnNLP);
      }
   }

   RETURN_STATUS (ret);
}


/**
   Enable or disable the echo suppressor.

   \param  pLLChannel   Handle to TAPI low level channel structure.
   \param  nEnable      Enable (<>0) or disable (=0) the echo suppressor.
   \param  nActiveNLP   Flag that indicates if LEC+NLP is active. This is used
                        to select a different parameter set.

   \return
      - VMMC_statusInvalCh Channel number is out of range
      - VMMC_statusNoRes No echo suppressor resource available
      - VMMC_statusCmdWr Writing the command has failed
      - VMMC_statusOk if successful
*/
static IFX_int32_t vmmc_alm_Echo_Suppressor_Set (IFX_TAPI_LL_CH_t *pLLChannel,
                                                 IFX_enDis_t nEnable,
                                                 IFX_enDis_t nActiveNLP)
{
   VMMC_CHANNEL *pCh  = (VMMC_CHANNEL *) pLLChannel;
   IFX_int32_t   ret  = VMMC_statusOk;

   /* sanity check */
   VMMC_ASSERT(pCh != IFX_NULL);
   if (pCh->pALM == IFX_NULL)
   {
      /* errmsg: Resource not valid. Channel number out of range */
      RETURN_STATUS (VMMC_statusInvalCh);
   }

   if (pCh->pALM->line_type_fxs == IFX_FALSE)
   {
      /* errmsg: Protection from calling on FXO line */
      RETURN_STATUS (VMMC_statusFXSCallOnFXO);
   }

   if (nEnable == IFX_DISABLE)
   {
      /* Disable the echo suppressor. */

      if (VMMC_RES_ID_VALID(pCh->pALM->nEsResId))
      {
         /* Disable the ES, free resource and forget the resource ID. */
         ret = VMMC_RES_ES_Enable (pCh->pALM->nEsResId, IFX_DISABLE);
         if (VMMC_SUCCESS (ret))
         {
            ret = VMMC_RES_ES_Release (pCh->pALM->nEsResId);
         }
         if (VMMC_SUCCESS (ret))
         {
            pCh->pALM->nEsResId = VMMC_RES_ID_NULL;
         }
      }
   }
   else
   {
      /* Enable the echo suppressor. */

      if (!VMMC_RES_ID_VALID(pCh->pALM->nEsResId))
      {
         /* allocate an ES resource */
         pCh->pALM->nEsResId = VMMC_RES_ES_Allocate (pCh, VMMC_RES_MOD_ALM);
      }

      if (!VMMC_RES_ID_VALID(pCh->pALM->nEsResId))
      {
         RETURN_STATUS (VMMC_statusNoRes);
      }

      /* In the ES_Enable() we need to know if the LEC+NLP is active on this
         channel. */
      ret = VMMC_RES_ES_ParameterSelect (pCh->pALM->nEsResId, nActiveNLP);

      if (VMMC_SUCCESS (ret))
      {
         ret = VMMC_RES_ES_Enable (pCh->pALM->nEsResId, IFX_ENABLE);
      }
   }

   RETURN_STATUS (ret);
}


#ifdef VMMC_FEAT_CONT_MEASUREMENT
/**
   Request continuous measurement results.

   \param  pLLChannel   Handle to TAPI low level channel structure.

   \return
      - VMMC_statusInvalCh
      - VMMC_statusFXSCallOnFXO
      - VMMC_statusOk

   \remarks
      This function does nothing on firmware interface. It spawns an
      event IFX_TAPI_EVENT_CONTMEASUREMENT in a task context. Added for
      compatibility with TAPI_V4 interface.
*/
static IFX_int32_t VMMC_TAPI_LL_ALM_ContMeas_Req(IFX_TAPI_LL_CH_t *pLLChannel)
{
   VMMC_CHANNEL *pCh  = (VMMC_CHANNEL *) pLLChannel;
   TAPI_CHANNEL    *pChannel = pCh->pTapiCh;
   IFX_TAPI_EVENT_t tapiEvent;

   /* sanity check */
   VMMC_ASSERT(pCh != IFX_NULL);
   if (pCh->pALM == IFX_NULL)
   {
      /* errmsg: Resource not valid. Channel number out of range */
      RETURN_STATUS (VMMC_statusInvalCh);
   }

   if (pCh->pALM->line_type_fxs == IFX_FALSE)
   {
      /* errmsg: Protection from calling on FXO line */
      RETURN_STATUS (VMMC_statusFXSCallOnFXO);
   }

   memset (&tapiEvent, 0, sizeof(IFX_TAPI_EVENT_t));
   tapiEvent.id = IFX_TAPI_EVENT_CONTMEASUREMENT;
   tapiEvent.module = IFX_TAPI_MODULE_TYPE_ALM;
   IFX_TAPI_Event_Dispatch(pChannel, &tapiEvent);

   RETURN_STATUS (VMMC_statusOk);
}

/**
   Reset continuous measurement results.

   \param  pLLChannel   Handle to TAPI low level channel structure.

   \return
      - VMMC_statusInvalCh
      - VMMC_statusFXSCallOnFXO
      - VMMC_statusCmdWr Writing the command has failed
      - VMMC_statusOk if successful
*/
static IFX_int32_t VMMC_TAPI_LL_ALM_ContMeas_Reset(IFX_TAPI_LL_CH_t *pLLChannel)
{
   VMMC_CHANNEL *pCh  = (VMMC_CHANNEL *) pLLChannel;
   VMMC_DEVICE  *pDev = pCh->pParent;
   VMMC_SDD_ContMeasClear_t *pContMeasClear = &pCh->pALM->fw_sdd_contMearClear;
   IFX_int32_t   ret;

   /* sanity check */
   VMMC_ASSERT(pCh != IFX_NULL);
   if (pCh->pALM == IFX_NULL)
   {
      /* errmsg: Resource not valid. Channel number out of range */
      RETURN_STATUS (VMMC_statusInvalCh);
   }

   if (pCh->pALM->line_type_fxs == IFX_FALSE)
   {
      /* errmsg: Protection from calling on FXO line */
      RETURN_STATUS (VMMC_statusFXSCallOnFXO);
   }

   /* protect channel from mutual access */
   VMMC_OS_MutexGet (&pCh->chAcc);

   pContMeasClear->ClearCMD = 1;
   ret = CmdWrite(pDev, (IFX_uint32_t*)pContMeasClear, SDD_ContMeasClear_LEN);

   VMMC_OS_MutexRelease (&pCh->chAcc);

   RETURN_STATUS (ret);
}

/**
   Return the stored continuous measure results.

   \param  pLLChannel   Handle to TAPI low level channel structure.

   \return
      - VMMC_statusInvalCh
      - VMMC_statusFXSCallOnFXO
      - IFX_ERROR CmdRead error
      - VMMC_statusOk if successful
*/
static IFX_int32_t VMMC_TAPI_LL_ALM_ContMeas_Get(IFX_TAPI_LL_CH_t *pLLChannel,
                                      IFX_TAPI_CONTMEASUREMENT_GET_t *pContMeas)
{
   VMMC_CHANNEL *pCh  = (VMMC_CHANNEL *) pLLChannel;
   VMMC_DEVICE  *pDev = pCh->pParent;
   VMMC_SDD_ContMeasRead_t *pContMeasRead = &pCh->pALM->fw_sdd_contMeasRead;
   IFX_int32_t   ret;

   /* sanity check */
   VMMC_ASSERT(pCh != IFX_NULL);
   if (pCh->pALM == IFX_NULL)
   {
      /* errmsg: Resource not valid. Channel number out of range */
      RETURN_STATUS (VMMC_statusInvalCh);
   }

   if (pCh->pALM->line_type_fxs == IFX_FALSE)
   {
      /* errmsg: Protection from calling on FXO line */
      RETURN_STATUS (VMMC_statusFXSCallOnFXO);
   }

   /* protect channel from mutual access */
   VMMC_OS_MutexGet (&pCh->chAcc);

   ret = CmdRead(pDev, (IFX_uint32_t *)pContMeasRead,
                       (IFX_uint32_t *)pContMeasRead, SDD_ContMeasRead_LEN);

   if (VMMC_SUCCESS(ret))
   {
      /* typecasts are mandatory because of signed values */
      pContMeas->nVLineWireRing =
                        ((IFX_int16_t)pContMeasRead->VlineRing * 14400) / 32768;
      pContMeas->nVLineWireTip =
                         ((IFX_int16_t)pContMeasRead->VlineTip * 14400) / 32768;
      pContMeas->nVLineDesired = (pContMeasRead->Vlim * 14400) >> 15;
      pContMeas->nILine = ((IFX_int16_t)pContMeasRead->Itrans * 10000) / 32768;
      pContMeas->nILineLong =
                            ((IFX_int16_t)pContMeasRead->Ilong * 10000) / 32768;
      pContMeas->nILineRingPeak = (pContMeasRead->Iring * 10000) >> 15;
      pContMeas->nVLineRingPeak = (pContMeasRead->Vring * 14400) >> 15;
      pContMeas->nTtxMeterReal = pContMeasRead->TtxReal;
      pContMeas->nTtxMeterImag = pContMeasRead->TtxImag;
      pContMeas->nTtxMeterLen = pContMeasRead->TtxLen;
      pContMeas->nITtxMeter = pContMeasRead->Ittx;
      pContMeas->nVTtxMeter = pContMeasRead->Vttx;
      pContMeas->nVBat = ((IFX_int16_t)pContMeasRead->Vbat * 14400) / 32768;
      pContMeas->nSlicTemp = (IFX_int16_t)pContMeasRead->SlicTemp;
   }

   VMMC_OS_MutexRelease (&pCh->chAcc);

   RETURN_STATUS (ret);
}
#endif /* VMMC_FEAT_CONT_MEASUREMENT */

/**
   Set the new hook state on FXO line.

   \param  pLLChannel   Handle to TAPI low level channel structure.
   \param  hook         Enum type for TAPI FXO hook

   \return
      - VMMC_statusInvalCh if no ALM resource
      - VMMC_statusParam if hook parameter is ambiguous
      - VMMC_statusLineNotFXO if line is not FXO
      - IFX_ERROR CmdWrite error
      - VMMC_statusOk if successful
*/
static IFX_int32_t VMMC_TAPI_LL_ALM_FxoHookSet(IFX_TAPI_LL_CH_t *pLLChannel,
                                               IFX_TAPI_FXO_HOOK_t hook)
{
   VMMC_CHANNEL *pCh  = (VMMC_CHANNEL *) pLLChannel;
   VMMC_DEVICE  *pDev;
   VMMC_SDD_FxoHookSwitch_t *pFxoHookSwitch;
   IFX_int32_t   ret;

   /* sanity check */
   VMMC_ASSERT(pCh != IFX_NULL);
   if (pCh->pALM == IFX_NULL)
   {
      /* errmsg: Resource not valid. Channel number out of range */
      RETURN_STATUS (VMMC_statusInvalCh);
   }

   if (pCh->pALM->line_type_fxs != IFX_FALSE)
   {
      /* errmsg: Protection from calling on non-FXO line */
      RETURN_STATUS (VMMC_statusLineNotFXO);
   }

   if (!(pCh->pALM->fxo_flags & (1U << FXO_LINE_MODE)))
   {
      /* errmsg: Protection from calling on disabled FXO line */
      RETURN_STATUS (VMMC_statusFXOLineDisabled);
   }
   pDev = pCh->pParent;
   pFxoHookSwitch = &pCh->pALM->fw_sdd_fxoHookSwitch;

   /* protect channel from mutual access */
   VMMC_OS_MutexGet (&pCh->chAcc);
   switch (hook)
   {
      case IFX_TAPI_FXO_HOOK_ONHOOK:
         pFxoHookSwitch->HF = SDD_FxoHookSwitch_HF_ON;
         break;
      case IFX_TAPI_FXO_HOOK_OFFHOOK:
         pFxoHookSwitch->HF = SDD_FxoHookSwitch_HF_OFF;
#ifdef VMMC_FEAT_CLOCK_SCALING
         pCh->pALM->bDartCanSleep = IFX_FALSE;
#endif /* VMMC_FEAT_CLOCK_SCALING */
         break;
      default:
         ret = VMMC_statusParam;
         goto error;
   }

   ret = CmdWrite(pDev, (IFX_uint32_t*)pFxoHookSwitch, SDD_FxoHookSwitch_LEN);

#ifdef VMMC_FEAT_CLOCK_SCALING
   if ((ret == VMMC_statusOk) &&
       (hook == IFX_TAPI_FXO_HOOK_ONHOOK))
   {
      pCh->pALM->bDartCanSleep = IFX_TRUE;
   }
#endif /* VMMC_FEAT_CLOCK_SCALING */

error:
   VMMC_OS_MutexRelease (&pCh->chAcc);

   RETURN_STATUS (ret);
}

/**
   Issue the hook flash on FXO line.

   \param  pLLChannel   Handle to TAPI low level channel structure.

   \return
      - VMMC_statusInvalCh if no ALM resource
      - VMMC_statusLineNotFXO if line is not FXO
      - IFX_ERROR CmdWrite error
      - VMMC_statusOk if successful
*/
static IFX_int32_t VMMC_TAPI_LL_ALM_FxoFlashSet(IFX_TAPI_LL_CH_t *pLLChannel)
{
   VMMC_CHANNEL *pCh  = (VMMC_CHANNEL *) pLLChannel;
   VMMC_DEVICE  *pDev;
   VMMC_SDD_FxoHookSwitch_t *pFxoHookSwitch;
   IFX_int32_t   ret;

   /* sanity check */
   VMMC_ASSERT(pCh != IFX_NULL);
   if (pCh->pALM == IFX_NULL)
   {
      /* errmsg: Resource not valid. Channel number out of range */
      RETURN_STATUS (VMMC_statusInvalCh);
   }

   if (pCh->pALM->line_type_fxs != IFX_FALSE)
   {
      /* errmsg: Protection from calling on non-FXO line */
      RETURN_STATUS (VMMC_statusLineNotFXO);
   }

   if (!(pCh->pALM->fxo_flags & (1U << FXO_LINE_MODE)))
   {
      /* errmsg: Protection from calling on disabled FXO line */
      RETURN_STATUS (VMMC_statusFXOLineDisabled);
   }
   pDev = pCh->pParent;
   pFxoHookSwitch = &pCh->pALM->fw_sdd_fxoHookSwitch;

   /* protect channel from mutual access */
   VMMC_OS_MutexGet (&pCh->chAcc);

   pFxoHookSwitch->HF = SDD_FxoHookSwitch_HF_FLASH;
   ret = CmdWrite(pDev, (IFX_uint32_t*)pFxoHookSwitch, SDD_FxoHookSwitch_LEN);

   VMMC_OS_MutexRelease (&pCh->chAcc);

   RETURN_STATUS (ret);
}

/**
   Configure the hook flash time.

   \param  pLLChannel   Handle to TAPI low level channel structure.
   \param  p_fhCfg      Pointer to IFX_TAPI_FXO_FLASH_CFG_t structure.

   \return
      - VMMC_statusInvalCh if no ALM resource
      - IFX_ERROR CmdWrite error
      - VMMC_statusOk if successful

   \remarks
      No protection from calling on non-FXO line or disabled FXO line
*/
static IFX_int32_t VMMC_TAPI_LL_ALM_FxoFlashCfg(IFX_TAPI_LL_CH_t *pLLChannel,
                                             IFX_TAPI_FXO_FLASH_CFG_t const *p_fhCfg)
{
   VMMC_CHANNEL *pCh  = (VMMC_CHANNEL *) pLLChannel;
   VMMC_DEVICE  *pDev;
   VMMC_SDD_FxoConfig_t *pFxoConfig;
   IFX_int32_t   ret;

   /* sanity check */
   VMMC_ASSERT(pCh != IFX_NULL);
   if (pCh->pALM == IFX_NULL)
   {
      /* errmsg: Resource not valid. Channel number out of range */
      RETURN_STATUS (VMMC_statusInvalCh);
   }
   pDev = pCh->pParent;
   pFxoConfig = &pCh->pALM->fw_sdd_fxoConfig;

   /* protect channel from mutual access */
   VMMC_OS_MutexGet (&pCh->chAcc);
   pFxoConfig->FLASHT = p_fhCfg->nFlashTime >> 1;
   ret = CmdWrite(pDev, (IFX_uint32_t*)pFxoConfig, SDD_FxoConfig_LEN);
   VMMC_OS_MutexRelease (&pCh->chAcc);

   RETURN_STATUS (ret);
}

/**
   Configure the maximum detection time for an open switching interval (OSI).

   \param  pLLChannel   Handle to TAPI low level channel structure.
   \param  p_osiCfg     Pointer to IFX_TAPI_FXO_OSI_CFG_t structure.

   \return
      - VMMC_statusInvalCh if no ALM resource
      - VMMC_statusParam if OSI max time <= 70 ms
      - IFX_ERROR CmdWrite error
      - VMMC_statusOk if successful

   \remarks
      No protection from calling on non-FXO line or disabled FXO line
*/
static IFX_int32_t VMMC_TAPI_LL_ALM_FxoOsiCfg(IFX_TAPI_LL_CH_t *pLLChannel,
                                              IFX_TAPI_FXO_OSI_CFG_t const *p_osiCfg)
{
   VMMC_CHANNEL *pCh  = (VMMC_CHANNEL *) pLLChannel;
   VMMC_DEVICE  *pDev;
   VMMC_SDD_FxoConfig_t *pFxoConfig;
   IFX_int32_t   ret;

   /* sanity check */
   VMMC_ASSERT(pCh != IFX_NULL);
   if (pCh->pALM == IFX_NULL)
   {
      /* errmsg: Resource not valid. Channel number out of range */
      RETURN_STATUS (VMMC_statusInvalCh);
   }

   /* time must be > 70 ms */
   if (p_osiCfg->nOSIMax <= 70)
   {
      RETURN_STATUS (VMMC_statusParam);
   }
   pDev = pCh->pParent;
   pFxoConfig = &pCh->pALM->fw_sdd_fxoConfig;

   /* protect channel from mutual access */
   VMMC_OS_MutexGet (&pCh->chAcc);
   pFxoConfig->OSIT = p_osiCfg->nOSIMax >> 1;
   ret = CmdWrite(pDev, (IFX_uint32_t*)pFxoConfig, SDD_FxoConfig_LEN);
   VMMC_OS_MutexRelease (&pCh->chAcc);
   RETURN_STATUS (ret);
}

/**
   Get current FXO hook state.

   \param  pLLChannel   Handle to TAPI low level channel structure.
   \param  p_hook       Pointer to IFX_TAPI_FXO_HOOK_t enum.

   \return
      - VMMC_statusInvalCh if no ALM resource
      - VMMC_statusLineNotFXO if line is not FXO
      - IFX_SUCCESS

   \remarks
      The hook status cannot be read from firmware as SDD_FxoHookSwitch
      command is write-only. The hook state is obtained from SDD_FxoHookSwitch
      command cache in ALM context.
*/
static IFX_int32_t VMMC_TAPI_LL_ALM_FxoHookGet(IFX_TAPI_LL_CH_t *pLLChannel,
                                               IFX_TAPI_FXO_HOOK_t *p_hook)
{
   VMMC_CHANNEL *pCh  = (VMMC_CHANNEL *) pLLChannel;
   VMMC_SDD_FxoHookSwitch_t *pFxoHookSwitch;

   /* sanity check */
   VMMC_ASSERT(pCh != IFX_NULL);
   if (pCh->pALM == IFX_NULL)
   {
      /* errmsg: Resource not valid. Channel number out of range */
      RETURN_STATUS (VMMC_statusInvalCh);
   }

   if (pCh->pALM->line_type_fxs != IFX_FALSE)
   {
      /* errmsg: Protection from calling on non-FXO line */
      RETURN_STATUS (VMMC_statusLineNotFXO);
   }

   if (!(pCh->pALM->fxo_flags & (1U << FXO_LINE_MODE)))
   {
      /* errmsg: Protection from calling on disabled FXO line */
      RETURN_STATUS (VMMC_statusFXOLineDisabled);
   }
   pFxoHookSwitch = &pCh->pALM->fw_sdd_fxoHookSwitch;

   switch (pFxoHookSwitch->HF)
   {
      case SDD_FxoHookSwitch_HF_OFF:
         *p_hook = IFX_TAPI_FXO_HOOK_OFFHOOK;
         break;
      default:
         *p_hook = IFX_TAPI_FXO_HOOK_ONHOOK;
         break;
   }
   return IFX_SUCCESS;
}

/**
   Get FXO status bit.

   \param  pLLChannel   Handle to TAPI low level channel structure.
   \param  bit_pos      Bit position defined in drv_tapi_ll_interface.h.
                        Can be one of FXO_BATTERY,FXO_APOH,FXO_POLARITY,
                        FXO_RING.
   \param  enable       Pointer to IFX_enDis_t; output value.

   \return
      - IFX_SUCCESS

   \remarks
      Just a trivial converter from bit value of fxo_flags to
      IFX_enDis_t enum.
*/
static IFX_int32_t VMMC_TAPI_LL_ALM_FxoStatGet(IFX_TAPI_LL_CH_t *pLLChannel,
                                               IFX_uint32_t bit_pos,
                                               IFX_enDis_t *enable)
{
   VMMC_CHANNEL *pCh  = (VMMC_CHANNEL *) pLLChannel;

   /* sanity check */
   VMMC_ASSERT(pCh != IFX_NULL);
   if (pCh->pALM == IFX_NULL)
   {
      /* errmsg: Resource not valid. Channel number out of range */
      RETURN_STATUS (VMMC_statusInvalCh);
   }

   if (pCh->pALM->line_type_fxs != IFX_FALSE)
   {
      /* errmsg: Protection from calling on non-FXO line */
      RETURN_STATUS (VMMC_statusLineNotFXO);
   }

   if (!(pCh->pALM->fxo_flags & (1U << FXO_LINE_MODE)))
   {
      /* errmsg: Protection from calling on disabled FXO line */
      RETURN_STATUS (VMMC_statusFXOLineDisabled);
   }

   *enable = (pCh->pALM->fxo_flags >> bit_pos & 1) ? IFX_ENABLE : IFX_DISABLE;

   return IFX_SUCCESS;
}

/**
   FXO line mode set.

   \param  pLLChannel   Handle to TAPI low level channel structure.
   \param  pMode        Pointer to IFX_TAPI_FXO_LINE_MODE_t type

   \return

*/
static IFX_int32_t VMMC_TAPI_LL_ALM_FxoLineModeSet(IFX_TAPI_LL_CH_t *pLLChannel,
                                                IFX_TAPI_FXO_LINE_MODE_t const *pMode)
{
   VMMC_CHANNEL *pCh  = (VMMC_CHANNEL *) pLLChannel;
   IFX_uint8_t  curr_opmode;
   IFX_int32_t  ret = IFX_SUCCESS;

   /* sanity check */
   VMMC_ASSERT(pCh != IFX_NULL);
   if (pCh->pALM == IFX_NULL)
   {
      /* errmsg: Resource not valid. Channel number out of range */
      RETURN_STATUS (VMMC_statusInvalCh);
   }

   if (pCh->pALM->line_type_fxs != IFX_FALSE)
   {
      /* errmsg: Protection from calling on non-FXO line */
      RETURN_STATUS (VMMC_statusLineNotFXO);
   }

   VMMC_OS_MutexGet (&pCh->chAcc);

   switch (pMode->mode)
   {
      case IFX_TAPI_FXO_LINE_MODE_DISABLED:
         {
            /* if already disabled - do nothing and exit */
            if (!(pCh->pALM->fxo_flags & (1U << FXO_LINE_MODE)))
               goto exit;

            ret = VMMC_ALM_OpmodeModeSet (pCh, VMMC_SDD_OPMODE_DISABLED);

            if (VMMC_SUCCESS(ret))
            {
               /* update FXO status bit */
               pCh->pALM->fxo_flags &= ~(1U << FXO_LINE_MODE);
            }
         }
         break;

      case IFX_TAPI_FXO_LINE_MODE_ACTIVE:
         {
            /* if already active - do nothing and exit */
            if (pCh->pALM->fxo_flags & (1U << FXO_LINE_MODE))
               goto exit;

            /* Get the current opmode - wait while a opmode change is ongoing.*/
            ret = VMMC_ALM_OpmodeGet (pCh, &curr_opmode);
            /* Only exit when interrupted by a signal. When waiting was aborted
               by the timeout assume that no linemode change is pending and try
               to continue. */
            if (ret == VMMC_statusSddEvtWaitInterrupt)
            {
               goto exit;
            }

            /* switching to linemode FXO requires to switch to PDH first;
               if current mode is not PDH - switch to PDH */
            if (curr_opmode != VMMC_SDD_OPMODE_DISABLED)
            {
               ret = VMMC_ALM_OpmodeModeSet(pCh, VMMC_SDD_OPMODE_DISABLED);
               if (!VMMC_SUCCESS(ret))
               {
                  goto exit;
               }
            }

            ret = VMMC_ALM_OpmodeModeSet (pCh, VMMC_SDD_OPMODE_FXO);

            if (VMMC_SUCCESS(ret))
            {
               /* update FXO status bit */
               pCh->pALM->fxo_flags |= (1U << FXO_LINE_MODE);
            }
         }
         break;

      default:
         {
            RETURN_STATUS (VMMC_statusParam);
         }
   } /* switch */

exit:
   VMMC_OS_MutexRelease (&pCh->chAcc);
   RETURN_STATUS(ret);
}


#ifdef VMMC_FEAT_MWL
/** Gets the activation status of the message waiting lamp.

   \param pCh           Handle to TAPI low level channel structure

   \param pActivation   Handle to \ref IFX_TAPI_MWL_ACTIVATION_t structure

   \return
   - VMMC_statusOk          if successful
   - VMMC_statusNotSupported
   - VMMC_statusReadErr
*/
static IFX_int32_t vmmc_alm_MWL_Activation_Get(VMMC_CHANNEL *pCh,
                               IFX_TAPI_MWL_ACTIVATION_t *pActivation)
{
   IFX_int32_t        ret        = VMMC_statusOk;
   VMMC_SDD_Opmode_t  *pOpmod    = &pCh->pALM->fw_sdd_opmode;
   IFX_uint8_t        curr_opmode;

   /* Get the current opmode - wait while a opmode change is ongoing. */
   ret = VMMC_ALM_OpmodeGet (pCh, &curr_opmode);
   /* Only exit when interrupted by a signal. When waiting was aborted by the
      timeout assume that no linemode change is pending and try to continue. */
   if (ret == VMMC_statusSddEvtWaitInterrupt)
   {
      RETURN_STATUS (ret);
   }

   if ((VMMC_SDD_OPMODE_RING_BURST == curr_opmode) &&
      (1 == pOpmod->MwiLamp))
   {
      pActivation->nActivation = IFX_ENABLE;
   }
   else
   {
      pActivation->nActivation = IFX_DISABLE;
   }

   RETURN_STATUS (ret);
}

/** Gets the activation status of the message waiting lamp.

   \param pLLChannel    Handle to TAPI low level channel structure

   \param pActivation   Handle to \ref IFX_TAPI_MWL_ACTIVATION_t structure

   \return
   - VMMC_statusOk          if successful
   - VMMC_statusInvalCh
   - VMMC_statusReadErr
*/
static IFX_int32_t VMMC_TAPI_LL_ALM_MWL_Activation_Get(
                   IFX_TAPI_LL_CH_t *pLLChannel,
                   IFX_TAPI_MWL_ACTIVATION_t *pActivation)
{
   IFX_int32_t        ret     = VMMC_statusOk;
   VMMC_CHANNEL       *pCh     = (VMMC_CHANNEL *) pLLChannel;
   VMMC_DEVICE        *pDev    = pCh->pParent;

   VMMC_ASSERT(IFX_NULL != pActivation);

   /* sanity check */
   if ((IFX_uint8_t)(pCh->nChannel - 1) >= pDev->caps.nALI)
   {
      RETURN_STATUS (VMMC_statusInvalCh);
   }

   /* protect channel from mutual access */
   VMMC_OS_MutexGet (&pCh->chAcc);

   ret = vmmc_alm_MWL_Activation_Get(pCh, pActivation);

   /* release channel */
   VMMC_OS_MutexRelease (&pCh->chAcc);

   RETURN_STATUS (ret);
}

/** Activate/deactivates the message waiting lamp.

   \param pCh           Handle to TAPI low level channel structure

   \param pActivation   Handle to \ref IFX_TAPI_MWL_ACTIVATION_t structure

   \return
   - VMMC_statusOk             if successful
   - VMMC_statusNotSupported
   - VMMC_statusParam
   - VMMC_statusMwlNotStandby
   - VMMC_statusOpModeWrErr
*/
static IFX_int32_t vmmc_alm_MWL_Activation_Set(
                   VMMC_CHANNEL *pCh,
                   IFX_TAPI_MWL_ACTIVATION_t const *pActivation)
{
   IFX_int32_t             ret       = VMMC_statusOk;
   VMMC_DEVICE             *pDev     = pCh->pParent;
   VMMC_SDD_Opmode_t       *pOpmod   = &pCh->pALM->fw_sdd_opmode;
   VMMC_SDD_RingConfig_t   *pRingCfg = &pCh->pALM->sdd_ring_config_tmp_mwl,
                                       sdd_ring_config_mwi;
   VMMC_SDD_BasicConfig_t  *pBasicCfg = &pCh->pALM->sdd_basic_config_tmp_mwl,
                                        sdd_basic_config_mwi;

   /* For DC/DC converters other than default a BBD download is required. */
   if ((pCh->pALM->nDcDcType == VMMC_DCDC_TYPE_DEFAULT_IBB) &&
       (pCh->pParent->sdd.nAllowedDcDcType != VMMC_DCDC_TYPE_IBB))
   {
      /* errmsg: Operation blocked until a BBD with DC/DC configuration
                 matching the connected DC/DC hardware type is downloaded */
      RETURN_STATUS (VMMC_statusBlockedDcDcTypeMissmatch);
   }

   /* MWL and combined DC/DC together is not possible. */
   if (pDev->sdd.bDcDcHwCombined == IFX_TRUE)
   {
      /* errmsg: MWL not possible on combined DC/DC hardware */
      RETURN_STATUS (VMMC_statusNoMwlAndCombinedDcDc);
   }

   if (IFX_ENABLE == pActivation->nActivation)
   {
      IFX_uint8_t             curr_opmode;

      /* Get the current opmode - wait while a opmode change is ongoing. */
      ret = VMMC_ALM_OpmodeGet (pCh, &curr_opmode);
      /* Only exit when interrupted by a signal. When waiting was aborted by
         timeout assume that no linemode change is pending and try to continue.*/
      if (ret == VMMC_statusSddEvtWaitInterrupt)
      {
         RETURN_STATUS (ret);
      }

      if (VMMC_SDD_OPMODE_STANDBY != curr_opmode)
      {
         /* errmsg: MWL might be started only in linemode standby */
         RETURN_STATUS (VMMC_statusMwlNotStandby);
      }

      /* read back the basic configuration */
      ret = CmdRead (pDev, (IFX_uint32_t*) pBasicCfg, (IFX_uint32_t*) pBasicCfg,
                     SDD_BasicConfig_LEN);
      if (!VMMC_SUCCESS (ret))
      {
         /* errmsg: Reading the basic configuration failed */
         RETURN_STATUS (VMMC_statusBasicCfgRdErr);
      }

      /* read back the ring configuration */
      ret = CmdRead (pDev, (IFX_uint32_t*) pRingCfg, (IFX_uint32_t*) pRingCfg,
                     SDD_RingConfig_LEN);
      if (!VMMC_SUCCESS (ret))
      {
         /* errmsg: Reading the ring configuration failed  */
         RETURN_STATUS (VMMC_statusRingCfgRdErr);
      }

      /* MWL operation requires clearing of GsEn bit of SDD_BasicConfig */
      sdd_basic_config_mwi = *pBasicCfg;
      if (sdd_basic_config_mwi.GsEn != SDD_BasicConfig_DISABLE)
      {
         sdd_basic_config_mwi.GsEn = SDD_BasicConfig_DISABLE;
         ret = CmdWrite(pDev, (IFX_uint32_t*) &sdd_basic_config_mwi,
                        sdd_basic_config_mwi.LENGTH);
         if (!VMMC_SUCCESS (ret))
         {
            /* errmsg: Writing the basic configuration failed */
            RETURN_STATUS (VMMC_statusBasicCfgWrErr);
         }
      }

      /* MWL operation requires special ring configuration */
      sdd_ring_config_mwi = *pRingCfg;
      if (sdd_ring_config_mwi.RingMode != 0 ||
          sdd_ring_config_mwi.RingSignal != 0 ||
          sdd_ring_config_mwi.RingCrestFact != 4)
      {
         sdd_ring_config_mwi.RingMode      = 0; /* always for MWL */
         sdd_ring_config_mwi.RingSignal    = 0; /* sinewave ringing signal */
         sdd_ring_config_mwi.RingCrestFact = 4; /* near sinewave ring */
         ret = CmdWrite(pDev, (IFX_uint32_t*) &sdd_ring_config_mwi,
                        sdd_ring_config_mwi.LENGTH);
         if (!VMMC_SUCCESS (ret))
         {
            /* errmsg: Writing the ring configuration failed */
            RETURN_STATUS (VMMC_statusRingCfgWrErr);
         }
      }

      pOpmod->OperatingMode = VMMC_SDD_OPMODE_RING_BURST;
      pOpmod->MwiLamp = 1;
   }
   else if (IFX_DISABLE == pActivation->nActivation)
   {
      if (pOpmod->MwiLamp == 0)
      {
         return VMMC_statusOk;
      }

      pOpmod->OperatingMode = VMMC_SDD_OPMODE_STANDBY;
      pOpmod->MwiLamp = 0;
   }
   else
   {
      RETURN_STATUS (VMMC_statusParam);
   }

   ret = VMMC_ALM_OpmodeSet (pCh);
   if (!VMMC_SUCCESS (ret))
   {
      /* errmsg: Writing the operation mode failed. */
      RETURN_STATUS (VMMC_statusOpModeWrErr);
   }

   if (IFX_DISABLE == pActivation->nActivation)
   {
      /* restore basic configuration */
      ret = CmdWrite(pDev, (IFX_uint32_t*) pBasicCfg, SDD_BasicConfig_LEN);
      if (!VMMC_SUCCESS (ret))
      {
         RETURN_STATUS (VMMC_statusBasicCfgWrErr);
      }

      /* restore ring configuration */
      ret = CmdWrite(pDev, (IFX_uint32_t*) pRingCfg, SDD_RingConfig_LEN);
      if (!VMMC_SUCCESS (ret))
      {
         RETURN_STATUS (VMMC_statusRingCfgWrErr);
      }
   }

   RETURN_STATUS (ret);
}

/** Activate/deactivates the message waiting lamp.

   \param pLLChannel    Handle to TAPI low level channel structure

   \param pActivation   Handle to \ref IFX_TAPI_MWL_ACTIVATION_t structure

   \return
   - VMMC_statusOk             if successful
   - VMMC_statusInvalCh
   - VMMC_statusParam
   - VMMC_statusMwlNotStandby
   - VMMC_statusOpModeWrErr
*/
static IFX_int32_t VMMC_TAPI_LL_ALM_MWL_Activation_Set(
                   IFX_TAPI_LL_CH_t *pLLChannel,
                   IFX_TAPI_MWL_ACTIVATION_t const *pActivation)
{
   IFX_int32_t       ret      = VMMC_statusOk;
   VMMC_CHANNEL     *pCh     = (VMMC_CHANNEL *) pLLChannel;
   VMMC_DEVICE      *pDev    = pCh->pParent;

   VMMC_ASSERT(IFX_NULL != pActivation);

   /* sanity check */
   if ((IFX_uint8_t)(pCh->nChannel - 1) >= pDev->caps.nALI)
   {
      RETURN_STATUS (VMMC_statusInvalCh);
   }

   /* protect channel from mutual access */
   VMMC_OS_MutexGet (&pCh->chAcc);

   ret = vmmc_alm_MWL_Activation_Set(pCh, pActivation);

   /* release channel */
   VMMC_OS_MutexRelease (&pCh->chAcc);

   RETURN_STATUS (ret);
}
#endif /* VMMC_FEAT_MWL */

/**
   Set MWL configuration.

   \param  pCh pointer to VMMC_CHANNEL structure

   \return
*/
IFX_int32_t VMMC_ALM_MWL_DefaultConf (VMMC_CHANNEL *pCh)
{
   VMMC_DEVICE             *pDev = pCh->pParent;
   VMMC_ALMCH_t            *pALM = pCh->pALM;
   VMMC_SDD_MwiConfig_t    *pSDD_MwiConf = &pALM->fw_sdd_mwi_config;
   IFX_int32_t             ret = IFX_SUCCESS;

   if (pALM->line_type_fxs == IFX_TRUE && pDev->caps.bMWI)
   {
      ret = CmdWrite(pDev, (IFX_uint32_t *)pSDD_MwiConf, SDD_MwiConfig_LEN);
   }

   return ret;
}

#ifdef VMMC_FEAT_CAP_MEASURE
/**
   Check for capacitance measurement support

   \param  pLLChannel   Pointer to TAPI low level channel structure.
   \param  pSupported   Returns information whether capacitance measurement
                        is supported (IFX_TRUE) or not (IFX_FALSE).

   \return
   - VMMC_statusOk always
*/
IFX_int32_t VMMC_TAPI_LL_ALM_LT_CheckCapMeasSup (IFX_TAPI_LL_CH_t *pLLChannel,
                                                 IFX_boolean_t *pSupported)
{
   VMMC_CHANNEL * pCh  = (VMMC_CHANNEL*) pLLChannel;
   VMMC_DEVICE  * pDev = pCh->pParent;

   /* Using the combined DC/DC operation the capacitance measurement requires
      that the other channel is disabled. In this case report that there is
      no capacitance measurement. */
   *pSupported = (pDev->sdd.bDcDcHwCombined != IFX_TRUE) ? IFX_TRUE : IFX_FALSE;
   return VMMC_statusOk;
}

/**
   Start an analog line capacitance measurement.

   \param  pCh pointer to VMMC_CHANNEL structure

   \return

   \remarks
      This function configures GR909 for phone detection
      and starts the GR909 measurement in FW.
*/
static IFX_int32_t vmmc_alm_Capacitance_Measurement_Start(VMMC_CHANNEL *pCh)
{
   VMMC_DEVICE            *pDev = pCh->pParent;
   VMMC_SDD_GR909Config_t *pSddGR909Config = &pCh->pALM->fw_sdd_GR909Config;
   IFX_int32_t            ret;

#ifdef VMMC_FEAT_NLT
   /* clear previous results */
   memset (&(pCh->pALM->t2r_cap_result), 0, sizeof(pCh->pALM->t2r_cap_result));
   memset (&(pCh->pALM->l2g_cap_result), 0, sizeof(pCh->pALM->l2g_cap_result));
#endif /* VMMC_FEAT_NLT */
   /* configure SDD to start line capacitance measurement */
   pSddGR909Config->Pdf = 1;
   /* clear all GR909 tests */
   pSddGR909Config->Hpt =
   pSddGR909Config->Femf =
   pSddGR909Config->Rft =
   pSddGR909Config->Roh =
   pSddGR909Config->Rit = 0;
   ret = CmdWrite(pDev, (IFX_uint32_t *)pSddGR909Config,
                                        SDD_GR909Config_LEN);
   if (VMMC_SUCCESS(ret))
   {
      ret = VMMC_ALM_OpmodeModeSet(pCh, VMMC_SDD_OPMODE_GR909);
   }

   return ret;
}

/**
   Start an analog line capacitance measurement session.

   \param  pLLChannel   Handle to TAPI low level channel structure.
   \param  bTip2RingOnly  not used.

   \return

   \remarks
      This function enables Opmode change events from FW,
      then sets the line Opmode to PDH. If the line mode already
      was PDH, the function starts the capacitance measurement.
      Otherwise the capacitance measurement start will be
      triggered by Opmode change event (PDH) from FW.
      The reason for this solution is that GR909 measurement
      can only be started in PDH line mode, so the driver has
      to be sure that FW has completed the switch to PDH.
*/
static IFX_int32_t VMMC_TAPI_LL_ALM_CapMeasStart(IFX_TAPI_LL_CH_t *pLLChannel,
                                                 IFX_boolean_t bTip2RingOnly)
{
   VMMC_CHANNEL *pCh  = (VMMC_CHANNEL *) pLLChannel;
   IFX_uint8_t  curr_opmode;
   IFX_int32_t  ret = VMMC_statusOk;

   VMMC_UNUSED(bTip2RingOnly);

   /* sanity check */
   VMMC_ASSERT(pCh != IFX_NULL);
   if (pCh->pALM == IFX_NULL)
   {
      /* errmsg: Resource not valid. Channel number out of range */
      RETURN_STATUS (VMMC_statusInvalCh);
   }

   if (pCh->pALM->line_type_fxs == IFX_FALSE)
   {
      /* errmsg: Protection from calling on FXO line */
      RETURN_STATUS (VMMC_statusFXSCallOnFXO);
   }

   /* For DC/DC converters other than default a BBD download is required. */
   if ((pCh->pALM->nDcDcType == VMMC_DCDC_TYPE_DEFAULT_IBB) &&
       (pCh->pParent->sdd.nAllowedDcDcType != VMMC_DCDC_TYPE_IBB))
   {
      /* errmsg: Operation blocked until a BBD with DC/DC configuration
                 matching the connected DC/DC hardware type is downloaded */
      RETURN_STATUS (VMMC_statusBlockedDcDcTypeMissmatch);
   }

   if (pCh->pALM->eCapMeasState != VMMC_CapMeasState_Inactive)
   {
      /* errmsg: Capacitance measurement already in progress */
      RETURN_STATUS (VMMC_statusCapMeasStartWhileActive);
   }

   VMMC_OS_MutexGet (&pCh->chAcc);

   pCh->pALM->eCapMeasState = VMMC_CapMeasState_Started;

   /* Get the current opmode - wait while a opmode change is ongoing. */
   ret = VMMC_ALM_OpmodeGet (pCh, &curr_opmode);
   /* Only exit when interrupted by a signal. When waiting was aborted by
      timeout assume that no linemode change is pending and try to continue. */
   if (ret != VMMC_statusSddEvtWaitInterrupt)
   {
      if (curr_opmode != VMMC_SDD_OPMODE_DISABLED)
      {
         /* set PDH line mode */
         ret = VMMC_ALM_OpmodeModeSet(pCh, VMMC_SDD_OPMODE_DISABLED);
      }

      if (VMMC_SUCCESS(ret))
      {
         ret = vmmc_alm_Capacitance_Measurement_Start(pCh);
      }
   }

   VMMC_OS_MutexRelease (&pCh->chAcc);
   RETURN_STATUS(ret);
}

/**
   Stop any running analog line capacitance measurement session.

   \param  pLLChannel   Handle to TAPI low level channel structure.

   \return

*/
static IFX_int32_t VMMC_TAPI_LL_ALM_CapMeasStop(IFX_TAPI_LL_CH_t *pLLChannel)
{
   VMMC_CHANNEL *pCh  = (VMMC_CHANNEL *) pLLChannel;
   IFX_int32_t  ret = IFX_SUCCESS;

   /* sanity check */
   VMMC_ASSERT(pCh != IFX_NULL);
   if (pCh->pALM == IFX_NULL)
   {
      /* errmsg: Resource not valid. Channel number out of range */
      RETURN_STATUS (VMMC_statusInvalCh);
   }

   if (pCh->pALM->line_type_fxs == IFX_FALSE)
   {
      /* errmsg: Protection from calling on FXO line */
      RETURN_STATUS (VMMC_statusFXSCallOnFXO);
   }

   VMMC_OS_MutexGet (&pCh->chAcc);
   ret = VMMC_ALM_OpmodeModeSet(pCh, VMMC_SDD_OPMODE_DISABLED);
   if (VMMC_SUCCESS(ret))
   {
      pCh->pALM->eCapMeasState = VMMC_CapMeasState_Inactive;
   }
   VMMC_OS_MutexRelease (&pCh->chAcc);

   RETURN_STATUS(ret);
}

/**
   Read capacitance measurement result.

   \param  pLLChannel   Handle to TAPI low level channel structure.

   \return

*/
static IFX_int32_t VMMC_TAPI_LL_ALM_CapMeasResult(IFX_TAPI_LL_CH_t *pLLChannel)
{
   VMMC_CHANNEL *pCh  = (VMMC_CHANNEL *) pLLChannel;
   VMMC_DEVICE  *pDev;
   VMMC_SDD_GR909PhoneDetection_t *pGR909PhoneDetection;
   IFX_TAPI_EVENT_t tapiEvent;
   IFX_int32_t  ret;

   /* next three sanity checks are paranoid, as this function is driver
      internal API and thus the channel resources should be correct.
      The checks may be hit only in case if FW reports wrong channel in
      a capacitance measurement ready event. */
   VMMC_ASSERT(pCh != IFX_NULL);
   if (pCh->pALM == IFX_NULL)
   {
      /* errmsg: Resource not valid. Channel number out of range */
      RETURN_STATUS (VMMC_statusInvalCh);
   }

   if (pCh->pALM->line_type_fxs == IFX_FALSE)
   {
      /* errmsg: Protection from calling on FXO line */
      RETURN_STATUS (VMMC_statusFXSCallOnFXO);
   }

   pDev = pCh->pParent;
   pGR909PhoneDetection = &pCh->pALM->fw_sdd_phone_detection;
   memset(&tapiEvent, 0, sizeof(tapiEvent));

   VMMC_OS_MutexGet (&pCh->chAcc);
   tapiEvent.ch = pCh->nChannel - 1;
   tapiEvent.id = IFX_TAPI_EVENT_LINE_MEASURE_CAPACITANCE_RDY;
   tapiEvent.module = IFX_TAPI_MODULE_TYPE_ALM;

   ret = CmdRead(pDev, (IFX_uint32_t *) pGR909PhoneDetection,
                       (IFX_uint32_t *) pGR909PhoneDetection,
                       SDD_GR909PhoneDetection_LEN);
   VMMC_OS_MutexRelease (&pCh->chAcc);
   if (VMMC_SUCCESS(ret))
   {
      /* zero capacitance is an error */
      tapiEvent.data.lcap.nReturnCode =
                (pGR909PhoneDetection->Capacitance == 0) ? IFX_ERROR : ret;
      tapiEvent.data.lcap.nCapacitance = pGR909PhoneDetection->Capacitance;
#ifdef VMMC_FEAT_NLT
      /* Store result from measurement. Needed for
            IFX_TAPI_NLT_CAPACITANCE_RESULT_GET */
      pCh->pALM->t2r_cap_result.bValidTip2Ring =
                (pGR909PhoneDetection->Capacitance == 0) ? IFX_FALSE : IFX_TRUE;
      pCh->pALM->t2r_cap_result.nCapTip2Ring = pGR909PhoneDetection->Capacitance;
#endif /* VMMC_FEAT_NLT */

      /* clear progress indicator */
      pCh->pALM->eCapMeasState = VMMC_CapMeasState_Inactive;

      IFX_TAPI_Event_Dispatch(pCh->pTapiCh, &tapiEvent);

      if (pCh->pALM->bCapMeasTip2RingOnly == IFX_FALSE)
      {
         /* Capacitance measurement was started by user.
            Report that capacitance measurement is finished and results can be
            read by the IFX_TAPI_NLT_CAPACITANCE_RESULT_GET */
         tapiEvent.id = IFX_TAPI_EVENT_NLT_END;
         tapiEvent.module = IFX_TAPI_MODULE_TYPE_ALM;
         IFX_TAPI_Event_Dispatch(pCh->pTapiCh, &tapiEvent);
       }
   }

   RETURN_STATUS(ret);
}
#endif /* VMMC_FEAT_CAP_MEASURE */

#ifdef VMMC_FEAT_NLT
/**
   Reads the results of the capacitance measurement. Used by the
   IFX_TAPI_NLT_CAPACITANCE_RESULT_GET command.

    \param  pLLChannel   Pointer to the TAPI LL channel structure.
    \param  pResult      Pointer to the IFX_TAPI_NLT_CAPACITANCE_RESULT_t.

    \return
    - VMMC_statusOk or status error
 */
static IFX_int32_t VMMC_TAPI_LL_ALM_NLT_Cap_Result(IFX_TAPI_LL_CH_t *pLLChannel,
                                     IFX_TAPI_NLT_CAPACITANCE_RESULT_t *pResult)
{
   VMMC_CHANNEL                      *pCh  = (VMMC_CHANNEL *) pLLChannel;

   /* calling function should ensure valid parameters */
   VMMC_ASSERT(pCh != IFX_NULL);
   VMMC_ASSERT(pResult != IFX_NULL);

   /* protect channel from mutual access */
   VMMC_OS_MutexGet (&pCh->chAcc);

   /* Copy measurement results */
   pResult->bValidTip2Ring = pCh->pALM->t2r_cap_result.bValidTip2Ring;
   pResult->nCapTip2Ring = pCh->pALM->t2r_cap_result.nCapTip2Ring;
   pResult->bValidLine2Gnd = pCh->pALM->l2g_cap_result.bValidLine2Gnd ;
   pResult->nCapTip2Gnd = pCh->pALM->l2g_cap_result.nCapTip2Gnd;
   pResult->nCapRing2Gnd = pCh->pALM->l2g_cap_result.nCapRing2Gnd;
   pResult->fOlCapTip2Ring = pCh->pALM->nlt_CapacitanceConfig.fOlCapTip2Ring;
   pResult->fOlCapTip2Gnd = pCh->pALM->nlt_CapacitanceConfig.fOlCapTip2Gnd;
   pResult->fOlCapRing2Gnd = pCh->pALM->nlt_CapacitanceConfig.fOlCapRing2Gnd;

   /* release channel */
   VMMC_OS_MutexRelease (&pCh->chAcc);
   RETURN_STATUS (VMMC_statusOk);
}

/**
   This function is used to configure the open loop calibration factors
   of the measurement path for line testing. Used by the
   IFX_TAPI_NLT_CONFIGURATION_OL_SET command.

    \param  pLLChannel   Pointer to the TAPI LL channel structure.
    \param  pConfig      Pointer to the IFX_TAPI_NLT_CONFIGURATION_OL_t.

    \return
    - VMMC_statusOk or status error
 */
static IFX_int32_t VMMC_TAPI_LL_ALM_NLT_OLConfig_Set(
                                       IFX_TAPI_LL_CH_t *pLLChannel,
                                       IFX_TAPI_NLT_CONFIGURATION_OL_t *pConfig)
{
   VMMC_CHANNEL                       *pCh  = (VMMC_CHANNEL *) pLLChannel;
   IFX_int32_t                        ret = VMMC_statusOk;

   /* calling function should ensure valid parameters */
   VMMC_ASSERT(pCh != IFX_NULL);
   VMMC_ASSERT(pConfig != IFX_NULL);

   /* protect channel from mutual access */
   VMMC_OS_MutexGet (&pCh->chAcc);

   /* Store configuration */
   pCh->pALM->nlt_ResistanceConfig.fOlResTip2Ring = pConfig->fOlResTip2Ring;
   pCh->pALM->nlt_ResistanceConfig.fOlResTip2Gnd = pConfig->fOlResTip2Gnd;
   pCh->pALM->nlt_ResistanceConfig.fOlResRing2Gnd = pConfig->fOlResRing2Gnd;
   pCh->pALM->nlt_CapacitanceConfig.fOlCapTip2Ring = pConfig->fOlCapTip2Ring;
   pCh->pALM->nlt_CapacitanceConfig.fOlCapTip2Gnd = pConfig->fOlCapTip2Gnd;
   pCh->pALM->nlt_CapacitanceConfig.fOlCapRing2Gnd = pConfig->fOlCapRing2Gnd;

   /* release channel */
   VMMC_OS_MutexRelease (&pCh->chAcc);
   RETURN_STATUS (ret);
}

/**
   This function is used to get the open loop calibration factors
   of the measurement path for line testing. Used by the
   IFX_TAPI_NLT_CONFIGURATION_OL_GET command.

    \param  pLLChannel   Pointer to the TAPI LL channel structure.
    \param  pConfig      Pointer to the IFX_TAPI_NLT_CONFIGURATION_OL_t.

    \return
    - VMMC_statusOk or status error
 */
static IFX_int32_t VMMC_TAPI_LL_ALM_NLT_OLConfig_Get(
                                       IFX_TAPI_LL_CH_t *pLLChannel,
                                       IFX_TAPI_NLT_CONFIGURATION_OL_t *pConfig)
{
   VMMC_CHANNEL                       *pCh  = (VMMC_CHANNEL *) pLLChannel;
   IFX_int32_t                        ret = VMMC_statusOk;

   /* calling function should ensure valid parameters */
   VMMC_ASSERT(pCh != IFX_NULL);
   VMMC_ASSERT(pConfig != IFX_NULL);

   /* protect channel from mutual access */
   VMMC_OS_MutexGet (&pCh->chAcc);

   /* Store configuration */
   pConfig->fOlResTip2Ring = pCh->pALM->nlt_ResistanceConfig.fOlResTip2Ring;
   pConfig->fOlResTip2Gnd = pCh->pALM->nlt_ResistanceConfig.fOlResTip2Gnd;
   pConfig->fOlResRing2Gnd = pCh->pALM->nlt_ResistanceConfig.fOlResRing2Gnd;
   pConfig->fOlCapTip2Ring = pCh->pALM->nlt_CapacitanceConfig.fOlCapTip2Ring;
   pConfig->fOlCapTip2Gnd = pCh->pALM->nlt_CapacitanceConfig.fOlCapTip2Gnd;
   pConfig->fOlCapRing2Gnd = pCh->pALM->nlt_CapacitanceConfig.fOlCapRing2Gnd;
   pConfig->dev_type = IFX_TAPI_GR909_DEV_VMMC;

   /* release channel */
   VMMC_OS_MutexRelease (&pCh->chAcc);
   RETURN_STATUS (ret);
}
#endif /* VMMC_FEAT_NLT */

#ifdef VMMC_FEAT_METERING
/**
   Sends one metering burst

   \param pCh           Pointer to the VMMC channel structure.
   \param nPulseNum     Number of pulses in burst

   \return
      - VMMC_statusParam  Wrong parameters passed. This code is returned
                          when the nPulseNum parameter has an invalid or
                          unsupported value.
      - IFX_ERROR         Writing the command has failed
      - VMMC_statusOk     if successful
*/
static IFX_int32_t vmmc_alm_MeteringBurst (VMMC_CHANNEL *pCh,
                                             IFX_uint32_t nPulseNum)
{
   VMMC_SDD_Opmode_t *pSDD_Opmod = &pCh->pALM->fw_sdd_opmode;
   IFX_int32_t       ret = VMMC_statusOk;

   /* For DC/DC converters other than default a BBD download is required. */
   if ((pCh->pALM->nDcDcType == VMMC_DCDC_TYPE_DEFAULT_IBB) &&
       (pCh->pParent->sdd.nAllowedDcDcType != VMMC_DCDC_TYPE_IBB))
   {
      /* errmsg: Operation blocked until a BBD with DC/DC configuration
                 matching the connected DC/DC hardware type is downloaded */
      RETURN_STATUS (VMMC_statusBlockedDcDcTypeMissmatch);
   }

   /* only single pulse supported */
   if (nPulseNum != 1)
      return VMMC_statusParam;

   pSDD_Opmod->OperatingMode = VMMC_SDD_OPMODE_ACTIVE;
   pSDD_Opmod->TtxBurst = SDD_Opmode_TTX_BURST;
   ret = VMMC_ALM_OpmodeSet (pCh);

   return ret;
}

/**
   Sends one metering burst

   \param pLLChannel Handle to TAPI low level channel structure
   \param nPulseNum  Number of pulses in burst

   \return
      - VMMC_statusInvalCh Channel not valid or channel number out of range
      - VMMC_statusParam   Wrong parameters passed. This code is returned
                           when the nPulseNum parameter has an invalid or
                           unsupported value.
      - IFX_ERROR          Writing the command has failed
      - VMMC_statusOk      if successful
*/
static IFX_int32_t VMMC_TAPI_LL_ALM_MeteringStart (IFX_TAPI_LL_CH_t *pLLChannel,
                                                   IFX_uint32_t nPulseNum)
{
   VMMC_CHANNEL *pCh  = (VMMC_CHANNEL *) pLLChannel;
   IFX_int32_t ret;

   VMMC_ASSERT(pCh);

   /* sanity checks */
   VMMC_ASSERT(pCh != IFX_NULL);
   if (pCh->pALM == IFX_NULL)
   {
      /* errmsg: Resource not valid. Channel number out of range */
      RETURN_STATUS (VMMC_statusInvalCh);
   }

   if (pCh->pALM->line_type_fxs == IFX_FALSE)
   {
      /* errmsg: Protection from calling on FXO line */
      RETURN_STATUS (VMMC_statusFXSCallOnFXO);
   }

   VMMC_OS_MutexGet (&pCh->chAcc);

   ret = vmmc_alm_MeteringBurst (pCh, nPulseNum);

   VMMC_OS_MutexRelease (&pCh->chAcc);

   RETURN_STATUS (ret);
}
#endif /* VMMC_FEAT_METERING */

/**
   Write the SDD_Opmode command.

   Defers when called while another opmode change is still not complete until
   this is done. A possible timeout error from the deferring is shaddowed by
   errors from the CmdWrite.
   The opmode "disabled" is always written even if already in status "disabled".

   \param  pCh             Pointer to the VMMC channel structure.

   \return
   - VMMC_statusCmdWr               writing the command has failed
   - VMMC_statusSddEvtWaitTmout     timeout waiting for an event from SDD
   - VMMC_statusSddEvtWaitInterrupt waiting for event from SDD is interrupted
                                    by a signal
   - VMMC_statusOk                  if successful
*/
IFX_int32_t VMMC_ALM_OpmodeSet (VMMC_CHANNEL *pCh)
{
   VMMC_SDD_Opmode_t *pOpmod = &pCh->pALM->fw_sdd_opmode;
   IFX_int32_t ret;
   IFX_uint8_t curr_opmode;

   /* Get the current opmode - wait while a opmode change is ongoing. */
   ret = VMMC_ALM_OpmodeGet (pCh, &curr_opmode);
   /* Only exit when interrupted by a signal. When waiting was aborted by the
      timeout assume that no linemode change is pending and try to continue
      with the opmode change that was requested. */
   if (ret == VMMC_statusSddEvtWaitInterrupt)
   {
      return ret;
   }

   /* do nothing if parameters didn't change */
   if (pOpmod->OperatingMode != VMMC_SDD_OPMODE_DISABLED &&
       pOpmod->OperatingMode == curr_opmode &&
       pCh->pALM->curr_polarity == pOpmod->Polarity &&
       pCh->pALM->curr_howler == pOpmod->Howler &&
       VMMC_OPMOD_TTX_BURST_OFF == pOpmod->TtxBurst &&
       pCh->pALM->curr_mwi_lamp == pOpmod->MwiLamp)
   {
      return VMMC_statusOk;
   }

   /* Only when the opmode changes wait for the opmode change event. */
   if (pOpmod->OperatingMode != curr_opmode)
   {
      pCh->pALM->bOpmodeChangePending = IFX_TRUE;
   }
   else
   {
      pCh->pALM->bOpmodeChangePending = IFX_FALSE;
   }

   if (pOpmod->OperatingMode == VMMC_SDD_OPMODE_DISABLED)
   {
      /* clear other fields of SDD_Opmode */
      pOpmod->Polarity = VMMC_OPMOD_POL_NORMAL;
      pOpmod->Howler = VMMC_OPMOD_HOWLER_OFF;
      /* pOpmod->TtxBurst = VMMC_OPMOD_TTX_BURST_OFF; handled below */
   }

   if (pOpmod->OperatingMode != VMMC_SDD_OPMODE_RING_BURST)
   {
      /* Allow MWI only with linefeed ringing. */
      pOpmod->MwiLamp = VMMC_OPMOD_MWI_LAMP_OFF;
   }

   if (pOpmod->OperatingMode != VMMC_SDD_OPMODE_ACTIVE)
   {
      /* Allow TTX only with linefeed active. */
      pOpmod->TtxBurst = VMMC_OPMOD_TTX_BURST_OFF;
   }

#ifdef VMMC_FEAT_CLOCK_SCALING
   pCh->pALM->bDartCanSleep = IFX_FALSE;
#endif /* VMMC_FEAT_CLOCK_SCALING */

   ret = CmdWrite(pCh->pParent, (IFX_uint32_t *)pOpmod, SDD_Opmode_LEN);

   /* Automatically reset the TTX flag. */
   pOpmod->TtxBurst = VMMC_OPMOD_TTX_BURST_OFF;

   if (VMMC_SUCCESS(ret))
   {
      /* store current status - pCh->pALM->curr_opmode is updated by ISR */
      pCh->pALM->curr_polarity = pOpmod->Polarity;
      pCh->pALM->curr_howler = pOpmod->Howler;
      pCh->pALM->curr_mwi_lamp = pOpmod->MwiLamp;
   }

   return ret;
}

/**
   Wrapper for the above function that sets the given opmode.

   \param  pCh             Pointer to the VMMC channel structure.
   \param  nOperatingMode  Operating mode.

   \return
   Returnvalue from \ref VMMC_ALM_OpModeSet().
*/
IFX_int32_t VMMC_ALM_OpmodeModeSet (
                        VMMC_CHANNEL *pCh,
                        IFX_uint32_t nOperatingMode)
{
   VMMC_SDD_Opmode_t *pOpmod = &pCh->pALM->fw_sdd_opmode;

   pOpmod->OperatingMode = nOperatingMode;
   return VMMC_ALM_OpmodeSet (pCh);
}

/**
   Get the current opmode but defer while a opmode change is in progress.

   \param  pCh             Pointer to the VMMC channel structure.
   \param  pCurrOpmode     Pointer to variable returning the current opmode.

   \return
   - VMMC_statusOk                  if successful
   - VMMC_statusSddEvtWaitTmout     timeout waiting for an event from SDD
   - VMMC_statusSddEvtWaitInterrupt waiting for event from SDD is interrupted
                                    by a signal
*/
IFX_int32_t VMMC_ALM_OpmodeGet (VMMC_CHANNEL *pCh,
                                IFX_uint8_t *pCurrentOpmode)
{
   IFX_int32_t timeout;

   /* Wait while a linemode change is pending to avoid returning an outdated
      linemode. When the waiting was aborted by a signal or a timeout occured
      while waiting inform about this in the return code. */
   timeout = VMMC_WaitForSddOpmodeChEvt(pCh);

   /* Return the current opmode regardless of the result of the waiting. */
   *pCurrentOpmode = pCh->pALM->curr_opmode;

   if (timeout > 0)
   {
      /* Positive values indicate the wait ended because the expected event
         occured. */
      return VMMC_statusOk;
   }
   else if (timeout == 0)
   {
      /* A return value of zero indicates that the timeout expired. */
      /* errmsg: Timeout waiting for an event from SDD. */
      return VMMC_statusSddEvtWaitTmout;
   }

   /* All negative values indicate that the waiting was interrupted by a
      signal from the OS. */
   /* errmsg: Waiting for an event from SDD interrupted by a signal. */
   return VMMC_statusSddEvtWaitInterrupt;
}


/**
   Timer return function to supervise the hook event window.

   To prevent calculation after a wraparound this timer resets the HookWindow
   flag.

   \param  timer_id     timer context
   \param  arg          pointer to a VMMC channel structure
*/
static IFX_void_t vmmc_alm_HookWindow_OnTimer (Timer_ID Timer,
                                               IFX_ulong_t arg)
{
   VMMC_CHANNEL *pCh = (VMMC_CHANNEL *)arg;
   VMMC_UNUSED (Timer);

   if ((pCh != IFX_NULL) && (pCh->pALM != IFX_NULL))
   {
      pCh->pALM->bHookWindow = IFX_FALSE;
   }
}


/**
   Calculate the elapsed time since the last hook event.

   This function is to be called with each hook event. It calculates the
   elapsed time since the last hook event.

   On the VMMC platforms there is no timestamp in the FW event so the MIPS core
   count register is used instead. A timer is used to detect the wraparound.

   \param  pCh          Pointer to VMMC channel structure.
   \param  timestamp    Current timestamp value from the FW event.

   \return
   Elapsed time since the last hook event in 1/8 ms steps.
   If the last event happened too long ago 0xFFFF is retuned.
*/
IFX_uint16_t VMMC_ALM_ElapsedTimeSinceLastHook (
                        VMMC_CHANNEL *pCh,
                        IFX_uint16_t timestamp)
{
   IFX_uint16_t   elapsedTime = 0xFFFF;

   if (pCh->pParent->caps.bHTS)
   {
      /* FW supports the timestamp in hook events so use it. */

      /* The wraparound of this timer happens every 8.192 seconds. */
      if (pCh->pALM->bHookWindow)
      {
         /* Calculate elapsed time since the saved timestamp. In case the
            16-bit counter had a wraparound since the saved timestamp the
            calculation will still be correct because the calculation is
            done with 16-bit unsigned data type. This will always result in
            a positive distance between the two timestamps. */
         elapsedTime = timestamp -
                       (IFX_uint16_t)pCh->pALM->nLastHookEvtTimestamp;
      }

      /* store the timestamp from this event */
      pCh->pALM->nLastHookEvtTimestamp = timestamp;
   }
   else
   {
      IFX_uint32_t   current_timestamp, time_difference;

      /* FW does not support the timetamp in hook events.
         Use the MIPS core count register to get a timestamp. */
      time_difference = VMMC_OS_GetTimeDelta(
                           pCh->pALM->nLastHookEvtTimestamp,
                           &current_timestamp);

      /* The wraparound of this timer depends on the current system frequency.
         With 8 seconds time window up to 1GHz will work.*/
      if (pCh->pALM->bHookWindow)
      {
         if (time_difference < 0xFFFF)
         {
            elapsedTime = (IFX_uint16_t)time_difference;
         }
      }

      /* store the timestamp */
      pCh->pALM->nLastHookEvtTimestamp = current_timestamp;
   }

   /* Start timer, length of timer = 8000 ms. This time needs to be below
      the wraparound of the timestamp counter. */
   if (pCh->pALM->nHookWindowTimerId != 0)
   {
      TAPI_SetTime_Timer (pCh->pALM->nHookWindowTimerId,
                          7000, IFX_FALSE, IFX_TRUE);
      pCh->pALM->bHookWindow = IFX_TRUE;
   }

   return elapsedTime;
}


/**
   Change the DUP timer configuration between normal and override mode.

   \param  pLLChannel   Pointer to VMMC channel structure.
   \param  nStandbyTime DUP_OVERRIDE_OFF to clear override or time in ms

   \return
   - VMMC_statusOk or status error

   \remarks
   Do not use time values below 2ms. There the hook detection behaves instable.
*/
static IFX_int32_t VMMC_TAPI_LL_ALM_CfgDupTimer_Override (
                        IFX_TAPI_LL_CH_t *pLLChannel,
                        IFX_uint8_t nStandbyTime)
{
   VMMC_CHANNEL *pCh  = (VMMC_CHANNEL *)pLLChannel;
   VMMC_DEVICE *pDev = pCh->pParent;
   VMMC_SDD_BasicConfig_t *pSDD_BasicConfig, OverrideSddBasicConfig;
   IFX_int32_t  ret = VMMC_statusOk;

   /* calling function should ensure valid parameters */
   VMMC_ASSERT(pCh != IFX_NULL);
   VMMC_ASSERT(pCh->pALM != IFX_NULL);
   VMMC_ASSERT(nStandbyTime >= 2);    /* DUP times below 2ms behave instable. */
   VMMC_ASSERT(nStandbyTime <= 15);
   VMMC_ASSERT(pDev != IFX_NULL);

   /* do nothing if state is already set */
   if (pCh->pALM->nStdbyDupTimeOverride == nStandbyTime)
   {
      return VMMC_statusOk;
   }

   pSDD_BasicConfig = &pCh->pALM->fw_sdd_basic_config;

   if (nStandbyTime == DUP_OVERRIDE_OFF)
   {
      /* Normal mode */
      VMMC_OS_MutexGet (&pCh->chAcc);
      ret = CmdWrite (pDev,
                      (IFX_uint32_t *)((IFX_void_t *)pSDD_BasicConfig),
                      SDD_BasicConfig_LEN);
      VMMC_OS_MutexRelease (&pCh->chAcc);
   }
   else
   {
      /* Override mode */
      vmmc_BBD_UpdateBasicConfigCache(pCh);

      VMMC_OS_MutexGet (&pCh->chAcc);
      OverrideSddBasicConfig = pCh->pALM->fw_sdd_basic_config;
      OverrideSddBasicConfig.OnhookDup = (IFX_uint32_t)(nStandbyTime & 0xF);
      ret = CmdWrite (pDev,
                      (IFX_uint32_t *)((IFX_void_t *)&OverrideSddBasicConfig),
                      SDD_BasicConfig_LEN);
      VMMC_OS_MutexRelease (&pCh->chAcc);
   }

   /* Remember the value for the entrance check. */
   pCh->pALM->nStdbyDupTimeOverride = nStandbyTime;

   RETURN_STATUS (ret);
}


#ifdef VMMC_FEAT_SLIC
/**
   Bring SLIC back into operation after fault end was reported.

   The SDD puts the SLIC in emergency shutdown mode once a fault (SSI crash)
   was detected. To exit the emergency shutdown mode linefeed 'disabled'
   needs to be set on all channels. After this the last known linefeed is
   programmed back into the line. After each programming do a delay of at
   least 2ms in order not to overwrite the previous linefeed request before
   the SDD can process it. Because the OPC event is not sent after disabled
   in emergency shutdown mode the delay is the only way to handle this.

   \param  pLLDev       Pointer to VMMC device structure.

   \return
   - always VMMC_statusOk
*/
static IFX_int32_t VMMC_TAPI_LL_ALM_SlicRecovery (
                        IFX_TAPI_LL_DEV_t *pLLDev)
{
   VMMC_DEVICE *pDev = (VMMC_DEVICE* )pLLDev;
   VMMC_ALMCH_t *pALM;
   IFX_uint16_t i;

   /* Set linefeed disabled on all channels of the device in order to get
      out of the emergency shutdown mode. */
   for (i = 0; i < VMMC_MAX_CH_NR; i++)
   {
      pALM = pDev->pChannel[i].pALM;

      if (pALM != IFX_NULL)
      {
         /* Backup the FW message before setting disabled.
            The last linemode which was confirmed by the FW is in curr_opmode.
            Update the command with it but when an opmode change is pending
            repeat the last set opmode command as it is. */
         pALM->fw_last_sdd_opmode = pALM->fw_sdd_opmode;
         if (!pALM->bOpmodeChangePending)
         {
            pALM->fw_last_sdd_opmode.OperatingMode = pALM->curr_opmode;
         }
         /* Exit the emergency shutdown with linefeed disabled. */
         irq_VMMC_ALM_LineDisable(&pDev->pChannel[i]);
      }
   }

   /* Allow the SDD to process the disabled commands on each of the channels. */
   IFXOS_MSecSleep(5);

   /* Restore the previous linefeeding on each of the channels of the device. */
   for (i = 0; i < VMMC_MAX_CH_NR; i++)
   {
      pALM = pDev->pChannel[i].pALM;

      if (pALM != IFX_NULL)
      {
         VMMC_SDD_Opmode_t *pOpmode = &pALM->fw_sdd_opmode;

         /* Restore the last opmode command. */
         *pOpmode = pALM->fw_last_sdd_opmode;

         CmdWriteIsr (pDev, (IFX_uint32_t *)pOpmode, SDD_Opmode_LEN);

         pALM->curr_opmode = pOpmode->OperatingMode;
         pALM->curr_polarity = pOpmode->Polarity;
         pALM->curr_howler = pOpmode->Howler;
      }
   }

   /* Allow the SDD to process the opmode commands on each of the channels. */
   IFXOS_MSecSleep(5);

   pDev->nSlicRecoveryCnt++;

   return VMMC_statusOk;
}
#endif /* VMMC_FEAT_SLIC */


#ifdef VMMC_FEAT_SLIC
/**
   Debugging support: SSI crash handler to read the cause code from the SDD.

   \param  pLLDev       Pointer to VMMC device structure.
   \param  pData        Pointer to struct IFX_TAPI_DBG_SSI_CRASH where details
                        will be returned on success.

   \return
   - VMMC_statusOk or status error
*/
static IFX_int32_t VMMC_TAPI_LL_ALM_SSICrashHandler (
                        IFX_TAPI_LL_DEV_t *pLLDev,
                        struct IFX_TAPI_DBG_SSI_CRASH *pData)
{
   VMMC_DEVICE *pDev = (VMMC_DEVICE* )pLLDev;
   struct VMMC_SDD_DartDebug msgSddDartDebug;
   IFX_int32_t  ret = VMMC_statusOk;

   /* Calling function must ensure valid parameters. */
   VMMC_ASSERT(pDev != IFX_NULL);

   /* Prepare the message to read the cause from SDD. */
   memset (&msgSddDartDebug, 0, sizeof (msgSddDartDebug));
   msgSddDartDebug.CHAN = 0;
   msgSddDartDebug.CMD  = CMD_SDD;
   msgSddDartDebug.MOD  = MOD_SDD;
   msgSddDartDebug.ECMD = SDD_DartDebug_ECMD;
   msgSddDartDebug.DebugType = SDD_DartDebug_DebugType_SSI;

   /* To read the data first a write with RWD set to write is needed.
      In the second write the read of the data is requested by clearing RWD
      and finally the CmdRead can be done to retrieve the data. */
   msgSddDartDebug.RWD = SDD_DartDebug_RWD_WRITE;
   ret = CmdWrite(pDev,
                  (IFX_uint32_t *)((IFX_void_t *)&msgSddDartDebug),
                  SDD_DartDebug_LEN);
   if (VMMC_SUCCESS(ret))
   {
      msgSddDartDebug.RWD = SDD_DartDebug_RWD_READ;
      ret = CmdWrite(pDev,
                     (IFX_uint32_t *)((IFX_void_t *)&msgSddDartDebug),
                     SDD_DartDebug_LEN);
   }
   if (VMMC_SUCCESS(ret))
   {
      ret = CmdRead(pDev,
                    (IFX_uint32_t *)((IFX_void_t *)&msgSddDartDebug),
                    (IFX_uint32_t *)((IFX_void_t *)&msgSddDartDebug),
                    SDD_DartDebug_LEN);
   }
   if (VMMC_SUCCESS(ret))
   {
      pData->cause[0] = ((IFX_uint32_t *)(&msgSddDartDebug))[1];
      pData->cause[1] = ((IFX_uint32_t *)(&msgSddDartDebug))[2];
      pData->cause[2] = ((IFX_uint32_t *)(&msgSddDartDebug))[3];
   }

   return ret;
}
#endif /* VMMC_FEAT_SLIC */


/**
   Function that fills in the ALM module function pointers in the driver
   context structure which is passed to HL TAPI during registration.

   \param  pAlm         Pointer to the ALM part in the driver context struct.
*/
IFX_void_t VMMC_ALM_Func_Register (IFX_TAPI_DRV_CTX_ALM_t *pAlm)
{
   /* Fill the function pointers with the services provided */
   pAlm->Lec_Cfg           = VMMC_TAPI_LL_ALM_LEC_Set;
   pAlm->Line_Mode_Set     = VMMC_TAPI_LL_ALM_Line_Mode_Set;
   pAlm->Line_Mode_Get     = VMMC_TAPI_LL_ALM_Line_Mode_Get;
   pAlm->Line_Type_Set     = VMMC_TAPI_LL_ALM_Line_Type_Set;
   pAlm->Volume_Set        = VMMC_TAPI_LL_ALM_Volume_Set;
   pAlm->Volume_High_Level = VMMC_TAPI_LL_ALM_Volume_High_Level;
   pAlm->TestHookGen       = VMMC_TAPI_LL_ALM_VMMC_Test_HookGen;
   pAlm->TestLoop          = VMMC_TAPI_LL_ALM_VMMC_Test_Loop;

#ifdef VMMC_FEAT_CALIBRATION
   pAlm->Calibration_Get   = VMMC_TAPI_LL_ALM_Calibration_Get;
#if 0
   pAlm->Calibration_Set   = VMMC_TAPI_LL_ALM_Calibration_Set;
#endif
   pAlm->Calibration_Start = VMMC_TAPI_LL_ALM_Calibration_Start;
   pAlm->Calibration_Stop  = VMMC_TAPI_LL_ALM_Calibration_Stop;
   pAlm->Calibration_Results_Get = VMMC_TAPI_LL_ALM_Calibration_Results;
#endif /* VMMC_FEAT_CALIBRATION */
   pAlm->Calibration_Finish = VMMC_TAPI_LL_ALM_Calibration_Finish;

#ifdef VMMC_FEAT_GR909
   pAlm->GR909_Start  = VMMC_TAPI_LL_ALM_GR909_Start;
   pAlm->GR909_Stop   = VMMC_TAPI_LL_ALM_GR909_Stop;
   pAlm->GR909_Result = VMMC_TAPI_LL_ALM_GR909_Result;
#endif /* VMMC_FEAT_GR909 */
#ifdef VMMC_FEAT_CONT_MEASUREMENT
   pAlm->ContMeasReq   = VMMC_TAPI_LL_ALM_ContMeas_Req;
   pAlm->ContMeasReset = VMMC_TAPI_LL_ALM_ContMeas_Reset;
   pAlm->ContMeasGet   = VMMC_TAPI_LL_ALM_ContMeas_Get;
#endif /* VMMC_FEAT_CONT_MEASUREMENT */
   pAlm->FXO_HookSet   = VMMC_TAPI_LL_ALM_FxoHookSet;
   pAlm->FXO_HookGet   = VMMC_TAPI_LL_ALM_FxoHookGet;
   pAlm->FXO_FlashSet  = VMMC_TAPI_LL_ALM_FxoFlashSet;
   pAlm->FXO_FlashCfg  = VMMC_TAPI_LL_ALM_FxoFlashCfg;
   pAlm->FXO_OsiCfg    = VMMC_TAPI_LL_ALM_FxoOsiCfg;
   pAlm->FXO_StatGet   = VMMC_TAPI_LL_ALM_FxoStatGet;
   pAlm->FXO_LineModeSet = VMMC_TAPI_LL_ALM_FxoLineModeSet;
#ifdef VMMC_FEAT_CAP_MEASURE
   pAlm->CheckCapMeasSup = VMMC_TAPI_LL_ALM_LT_CheckCapMeasSup;
   pAlm->CapMeasStart  = VMMC_TAPI_LL_ALM_CapMeasStart;
   pAlm->CapMeasStop   = VMMC_TAPI_LL_ALM_CapMeasStop;
   pAlm->CapMeasResult = VMMC_TAPI_LL_ALM_CapMeasResult;
#endif /* VMMC_FEAT_CAP_MEASURE */
#ifdef VMMC_FEAT_NLT
   pAlm->NLT_capacitance_result_get = VMMC_TAPI_LL_ALM_NLT_Cap_Result;
   pAlm->NLT_OLConfigSet      = VMMC_TAPI_LL_ALM_NLT_OLConfig_Set;
   pAlm->NLT_OLConfigGet      = VMMC_TAPI_LL_ALM_NLT_OLConfig_Get;
#endif /* VMMC_FEAT_NLT */
#ifdef VMMC_FEAT_MWL
   pAlm->MWL_Activation_Get   = VMMC_TAPI_LL_ALM_MWL_Activation_Get;
   pAlm->MWL_Activation_Set   = VMMC_TAPI_LL_ALM_MWL_Activation_Set;
#endif /* VMMC_FEAT_MWL */
#ifdef VMMC_FEAT_METERING
   pAlm->Metering_Start       = VMMC_TAPI_LL_ALM_MeteringStart;
#endif /* VMMC_FEAT_METERING */
   pAlm->CfgDupTimer_Override = VMMC_TAPI_LL_ALM_CfgDupTimer_Override;
#ifdef VMMC_FEAT_SLIC
   pAlm->SlicRecovery         = VMMC_TAPI_LL_ALM_SlicRecovery;
   /* Debugging support: SSI crash handler. */
   pAlm->Dbg_SSICrash_Handler = VMMC_TAPI_LL_ALM_SSICrashHandler;
#endif /* VMMC_FEAT_SLIC */
}
