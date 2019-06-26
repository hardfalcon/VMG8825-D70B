/******************************************************************************

  Copyright (c) 2006-2015 Lantiq Deutschland GmbH
  Copyright (c) 2015 Lantiq Beteiligungs-GmbH & Co.KG
  Copyright 2016, Intel Corporation.

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/**
   \file drv_vmmc_pcm.c
   This file implements the PCM functionality.
*/

#include "drv_api.h"

#ifdef VMMC_FEAT_PCM

/* ============================= */
/* Includes                      */
/* ============================= */
#include "drv_vmmc_res.h"
#include "drv_vmmc_pcm_priv.h"
#include "drv_vmmc_pcm.h"
#include "drv_vmmc_api.h"
#include "drv_vmmc_con.h"

/* ============================= */
/* Global function prototypes    */
/* ============================= */
#ifdef VMMC_FEAT_LINUX_PLATFORM_DRIVER
extern int vmmc_pcm_pin_config(unsigned int mode);
#else
extern IFX_int32_t VMMC_XRX100_PcmGpioReserve(
                        IFX_TAPI_PCM_IF_MODE_t mode,
                        IFX_boolean_t GPIOreserved);
#endif

#if defined(SYSTEM_XRX300)
extern void ifx_mps_pcm_if_war(void);
#else
static inline void ifx_mps_pcm_if_war(void)
{
}
#endif

/* ============================= */
/* Local Macros & Definitions    */
/* ============================= */

/* ============================= */
/* Local variables and types     */
/* ============================= */

/** Represent timeslot direction */
typedef enum {
   /* timeslot has no direction */
   VMMC_PCM_TS_DIR_NONE = 0,
   /* timeslot used for transmission */
   VMMC_PCM_TS_DIR_TX,
   /* timeslot used for receiving */
   VMMC_PCM_TS_DIR_RX,
   /* timeslot used for transmission and receiving */
   VMMC_PCM_TS_DIR_BOTH
} IFX_PCM_TS_Direction_t;

static IFX_TAPI_PCM_IF_CFG_t     ifx_tapi_pcm_if_cfg_defaults;

/* ============================= */
/* Local function declaration    */
/* ============================= */
static IFX_int32_t VMMC_TAPI_LL_PCM_CH_Cfg (
                        IFX_TAPI_LL_CH_t *pLLChannel,
                        IFX_TAPI_PCM_CFG_t const *pPCMConfig);

static IFX_int32_t VMMC_TAPI_LL_PCM_CH_Enable (
                        IFX_TAPI_LL_CH_t *pLLChannel,
                        IFX_uint32_t nMode,
                        IFX_TAPI_PCM_CFG_t *pPcmCfg);

static IFX_int32_t VMMC_TAPI_LL_PCM_Volume_Set (
                        IFX_TAPI_LL_CH_t *pLLChannel,
                        IFX_TAPI_LINE_VOLUME_t const *pVol);

static IFX_int32_t VMMC_TAPI_LL_PCM_LEC_Set (
                        IFX_TAPI_LL_CH_t *pLLChannel,
                        TAPI_LEC_DATA_t const *pLecConf);

static IFX_int32_t VMMC_TAPI_LL_PCM_DEC_HP_Set (
                        IFX_TAPI_LL_CH_t *pLLChannel,
                        IFX_boolean_t bHp);

static IFX_int32_t VMMC_TAPI_LL_PCM_IF_Cfg (
                        IFX_TAPI_LL_DEV_t *pLLDev,
                        const IFX_TAPI_PCM_IF_CFG_t *pCfg);

static IFX_int32_t VMMC_PCM_IF_Cfg (
                        VMMC_DEVICE *pDev,
                        const IFX_TAPI_PCM_IF_CFG_t *pCfg);

/* ============================= */
/* Function definitions          */
/* ============================= */

/**
   Release PCM timeslots.

   \param   pDev        VMMC device handle
   \param   nDir        Timeslot direction see \ref IFX_PCM_TS_Direction_t
   \param   nTimeslot   Timeslot number
   \param   nCount      Number of consecutive timeslots
   \param   nSplit      Flag indicating split timeslot usage.
                        0: consecutive timeslots; !0: split timeslots
                        For split timeslots nCount must be larger than one.
*/
static IFX_void_t VMMC_PCM_TimeSlotRelease (VMMC_DEVICE *pDev,
                                            IFX_PCM_TS_Direction_t nDir,
                                            IFX_uint32_t nTimeslot,
                                            IFX_uint32_t nCount,
                                            IFX_uint32_t nSplit)
{
   IFX_uint32_t   (*pcmTS)[PCM_TS_ARRAY] = IFX_NULL, /* pointer to an array */
                  i           = 0,
                  j           = 0,
                  Idx         = 0,
                  BitValue    = 0;
   IFX_uint16_t   nHighway    = 0; /* one highway only */

   if (nSplit && (nCount <= 1))
   {
      return;
   }

   if (nSplit)
   {
      /* The offset between 1st and 2nd group is half the number of
         maximum timeslots. */
      nSplit = pDev->nMaxTimeslot >> 1;
   }

   if ((nCount < 1) || ((nTimeslot + nCount + nSplit) > PCM_MAX_TS))
   {
      return;
   }

   if (nDir == VMMC_PCM_TS_DIR_BOTH)
   {
      /* Recursive call occurred only once for each direction */
      VMMC_PCM_TimeSlotRelease (pDev, VMMC_PCM_TS_DIR_RX,
                                nTimeslot, nCount, nSplit);
      VMMC_PCM_TimeSlotRelease (pDev, VMMC_PCM_TS_DIR_TX,
                                nTimeslot, nCount, nSplit);
      return;
   }

   switch (nDir)
   {
   case VMMC_PCM_TS_DIR_RX:
      pcmTS = pDev->PcmRxTs;
      break;
   case VMMC_PCM_TS_DIR_TX:
      pcmTS = pDev->PcmTxTs;
      break;
   default:
      return;
   }

   /* the timeslot map held in the variables PcmRxTs, PcmTxTs is global
      for all channels and must be protected against concurrent access */
   VMMC_OS_MutexGet (&pDev->mtxMemberAcc);

   /* free the timeslots in the map */
   for (j = 0;  j < nCount; j++)
   {
      /* Translate the loop parameter so that for split timeslots
         the index of the 2nd group includes the offset of half the
         possible timeslots. For consecutive timeslots i=j is used. */
      i = ((nSplit != 0) && (j >= (nCount/2))) ? (j - (nCount/2) + nSplit) : j;

      /* get the index in the time slot array. Each bit reflects one time
      slot. Modulo 32 separate the index of the array and the bit inside
      the field. */
      Idx = (nTimeslot + i) >> 5;
      BitValue = 1 << ((nTimeslot + i) & 0x1F);

      pcmTS[nHighway][Idx] &= ~BitValue;
   }

   /* unlock */
   VMMC_OS_MutexRelease (&pDev->mtxMemberAcc);
}

/**
   Allocate PCM timeslots.

   \param   pDev        VMMC device handle
   \param   nDir        Timeslot direction see \ref IFX_PCM_TS_Direction_t
   \param   nTimeslot   Timeslot number
   \param   nCount      Number of consecutive timeslots
   \param   nSplit      Flag indicating split timeslot usage.
                        0: consecutive timeslots; !0: split timeslots
                        For split timeslots nCount must be larger than one.

   \return
   - VMMC_statusPcmNoTx Tx timeslot for PCM channel activation not available
   - VMMC_statusPcmNoRx Rx timeslot for PCM channel activation not available
   - VMMC_statusOk if successful

*/
static IFX_int32_t VMMC_PCM_TimeSlotAllocate (VMMC_DEVICE *pDev,
                                            IFX_PCM_TS_Direction_t nDir,
                                            IFX_uint32_t nTimeslot,
                                            IFX_uint32_t nCount,
                                            IFX_uint32_t nSplit)
{
   IFX_int32_t    ret         = VMMC_statusOk;
   IFX_uint32_t  (*pcmTS)[PCM_TS_ARRAY] = IFX_NULL, /* pointer to an array */
                  i           = 0,
                  j           = 0,
                  Idx         = 0,
                  BitValue    = 0;
   IFX_uint16_t   nHighway    = 0; /* one highway only */

   if (nSplit && (nCount <= 1))
   {
      RETURN_DEVSTATUS (VMMC_statusParam);
   }

   if (nSplit)
   {
      /* The offset between 1st and 2nd group is half the number of
         maximum timeslots. */
      nSplit = pDev->nMaxTimeslot >> 1;
   }

   if ((nCount < 1) || ((nTimeslot + nCount + nSplit) > PCM_MAX_TS))
   {
      RETURN_DEVSTATUS (VMMC_statusParam);
   }

   if (nDir == VMMC_PCM_TS_DIR_BOTH)
   {
      /* Recursive call occurred only once for each direction */
      ret = VMMC_PCM_TimeSlotAllocate (pDev, VMMC_PCM_TS_DIR_RX,
         nTimeslot, nCount, nSplit);

      if (VMMC_SUCCESS(ret))
      {
         /* Recursive call occurred only once for each direction */
         ret = VMMC_PCM_TimeSlotAllocate (pDev, VMMC_PCM_TS_DIR_TX,
            nTimeslot, nCount, nSplit);

         if (!VMMC_SUCCESS(ret))
         {
            VMMC_PCM_TimeSlotRelease (pDev, VMMC_PCM_TS_DIR_RX,
               nTimeslot, nCount, nSplit);
         }
      }

      RETURN_DEVSTATUS (ret);
   }

   switch (nDir)
   {
   case VMMC_PCM_TS_DIR_RX:
      pcmTS = pDev->PcmRxTs;
      break;
   case VMMC_PCM_TS_DIR_TX:
      pcmTS = pDev->PcmTxTs;
      break;
   default:
      RETURN_DEVSTATUS (VMMC_statusParam);
   }

   /* the timeslot map held in the variables PcmRxTs, PcmTxTs is global
      for all channels and must be protected against concurrent access */
   VMMC_OS_MutexGet (&pDev->mtxMemberAcc);

   /* First check if all timeslots are available. */
   for (j = 0;  j < nCount; j++)
   {
      /* Translate the loop parameter so that for split timeslots
         the index of the 2nd group includes the offset of half the
         possible timeslots. For consecutive timeslots i=j is used. */
      i = ((nSplit != 0) && (j >= (nCount/2))) ? (j - (nCount/2) + nSplit) : j;

      /* get the index in the time slot array. Each bit reflects one time
      slot. Modulo 32 separate the index of the array and the bit inside
      the field. */
      Idx = (nTimeslot + i) >> 5;
      BitValue = 1 << ((nTimeslot + i) & 0x1F);

      if (pcmTS[nHighway][Idx] & BitValue)
      {
         switch (nDir)
         {
         case VMMC_PCM_TS_DIR_RX:
            /** Rx timeslot for PCM channel activation not available */
            ret = VMMC_statusPcmNoRx;
            break;
         case VMMC_PCM_TS_DIR_TX:
            /** Tx timeslot for PCM channel activation not available */
            ret = VMMC_statusPcmNoTx;
            break;
         default:
            break;
         }
      }
   }

   if (VMMC_SUCCESS(ret))
   {
      /* Second allocate all timeslots in the map that are needed */
      for (j = 0;  j < nCount; j++)
      {
         /* please see description in the block above */
         i = ((nSplit != 0) && (j >= (nCount/2))) ? (j - (nCount/2) + nSplit) : j;
         Idx = (nTimeslot + i) >> 5;
         BitValue = 1 << ((nTimeslot + i) & 0x1F);

         pcmTS[nHighway][Idx] |= BitValue;
      }
   }

   /* unlock */
   VMMC_OS_MutexRelease (&pDev->mtxMemberAcc);

   RETURN_DEVSTATUS (ret);
}

/**
   Sets the LEC configuration on the PCM.

   \param  pLLChannel   Pointer to VMMC channel structure.
   \param  pLecConf     Handle to IFX_TAPI_LEC_CFG_t structure.

   \return
   - VMMC_statusInvalCh Resource not valid. No PCM on this channel.
   - VMMC_statusPcmChNotEn Echo cancelling needs the PCM channel to be
     activated which is not the case.
   - VMMC_statusFuncParm Wrong parameters passed. This code is returned
     when the nOpMode parameter has an invalid value.
   - VMMC_statusNotSupported Requested action is not supported. This code
     is returned when a WLEC is requested but not supported by the VoFW.
   - VMMC_statusNoRes No free LEC resources is available
   - VMMC_statusCmdWr Writing the command has failed
   - VMMC_statusOk if successful
*/
IFX_int32_t VMMC_TAPI_LL_PCM_LEC_Set (IFX_TAPI_LL_CH_t *pLLChannel,
                                     TAPI_LEC_DATA_t const *pLecConf)
{
   VMMC_CHANNEL *pCh = (VMMC_CHANNEL *) pLLChannel;
   IFX_int32_t   ret  = VMMC_statusOk;
   IFX_enDis_t   bEnES,
                 bEnNLP = IFX_DISABLE;
   TAPI_LEC_DATA_t lecConf;

   /* parameter check */
   if (pCh == IFX_NULL)
   {
      /* Cannot log this error because pCh is NULL. */
      return VMMC_statusInvalCh;
   }
   if (pCh->pPCM == IFX_NULL)
   {
      /* errmsg: Resource not valid. Channel number out of range */
      RETURN_STATUS (VMMC_statusInvalCh);
   }
   /* Make sure that the PCM channel is activated */
   if (pCh->pPCM->fw_pcm_ch.EN != PCM_CHAN_ENABLE)
   {
      /* errmsg: Configuration not possible. PCM channel not activated */
      RETURN_STATUS (VMMC_statusPcmChNotEn);
   }

   /* copy configuration in to the local buffer - the window size parameters
      may be modified below in VMMC_RES_LEC_CoefWinValidate(). */
   lecConf = *pLecConf;

   /* Needed for LEC and ES activation. */
   bEnES = ((lecConf.nOpMode == IFX_TAPI_WLEC_TYPE_ES) ||
            (lecConf.nOpMode == IFX_TAPI_WLEC_TYPE_NE_ES) ||
            (lecConf.nOpMode == IFX_TAPI_WLEC_TYPE_NFE_ES)) ?
           IFX_ENABLE : IFX_DISABLE;

   /* protect channel variables */
   VMMC_OS_MutexGet (&pCh->chAcc);

   if ((lecConf.nOpMode == IFX_TAPI_WLEC_TYPE_OFF) ||
       (lecConf.nOpMode == IFX_TAPI_WLEC_TYPE_ES)    )
   {
      /* Disable the line echo canceller. */

      if (VMMC_RES_ID_VALID(pCh->pPCM->nLecResId))
      {
         /* Disable the LEC, free resource and forget the resource ID. */
         ret = VMMC_RES_LEC_Enable (pCh->pPCM->nLecResId, IFX_DISABLE);
         if (VMMC_SUCCESS (ret))
         {
            ret = VMMC_RES_LEC_Release (pCh->pPCM->nLecResId);
         }
         if (VMMC_SUCCESS (ret))
         {
            pCh->pPCM->nLecResId = VMMC_RES_ID_NULL;
         }
      }
   }
   else
   {
      /* Enable the line echo canceller. */

      VMMC_RES_LEC_MODE_t nOperatingMode;
      OPMODE_SMPL nSamplingMode;

      /* Translate the operating mode parameter */
      switch (lecConf.nOpMode)
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

      /* Determine the current sampling mode of this module */
      if (pCh->pPCM->fw_pcm_ch.ISR == pCh->pPCM->fw_pcm_ch.UD)
      {
         nSamplingMode = NB_8_KHZ;
      }
      else
      {
         nSamplingMode = WB_16_KHZ;
      }

      /* Translate the NLP parameter */
      bEnNLP = (lecConf.bNlp == IFX_TAPI_WLEC_NLP_OFF) ?
               IFX_DISABLE : IFX_ENABLE;

      /* Do parameter checks and corrections on the window size parameters */
      ret = VMMC_RES_LEC_CoefWinValidate (VMMC_RES_MOD_PCM,
                                          nOperatingMode, &lecConf);
      if (!VMMC_SUCCESS (ret))
      {
         VMMC_OS_MutexRelease (&pCh->chAcc);
         RETURN_STATUS (ret);
      }

      if (!VMMC_RES_ID_VALID(pCh->pPCM->nLecResId))
      {
         /* allocate a LEC resource */
         pCh->pPCM->nLecResId = VMMC_RES_LEC_Allocate (pCh, VMMC_RES_MOD_PCM);

         if (!VMMC_RES_ID_VALID(pCh->pPCM->nLecResId))
         {
            VMMC_OS_MutexRelease (&pCh->chAcc);
            RETURN_STATUS (VMMC_statusNoRes);
         }

         /* set the current sampling mode */
         VMMC_RES_LEC_SamplingModeSet (pCh->pPCM->nLecResId, nSamplingMode);
      }

      VMMC_RES_LEC_OperatingModeSet (pCh->pPCM->nLecResId,
                                     nOperatingMode, bEnNLP);

      /* Set window sizes for 8kHz sampling mode */
      switch (nOperatingMode)
      {
         default:
         case VMMC_RES_LEC_MODE_NLEC:
            /* NLEC operating mode */
            VMMC_RES_LEC_CoefWinSet (pCh->pPCM->nLecResId,
                                     NB_8_KHZ, nOperatingMode,
                                     lecConf.nNBNEwindow, 0);
            break;
         case VMMC_RES_LEC_MODE_WLEC:
            /* WLEC operating mode */ /*lint -e{656} */
            VMMC_RES_LEC_CoefWinSet (pCh->pPCM->nLecResId,
                                     NB_8_KHZ, nOperatingMode,
                                     lecConf.nNBNEwindow +
                                     lecConf.nNBFEwindow,
                                     lecConf.nNBFEwindow);
            break;
      }

      /* Set window sizes for 16kHz sampling mode */
      VMMC_RES_LEC_CoefWinSet (pCh->pPCM->nLecResId,
                               WB_16_KHZ, nOperatingMode,
                               lecConf.nWBNEwindow, 0);

      /* Set information if the ES is also working on this channel */
      VMMC_RES_LEC_ParameterSelect (pCh->pPCM->nLecResId, bEnES);

      /* Enable the echo suppressor resource */
      ret = VMMC_RES_LEC_Enable (pCh->pPCM->nLecResId, IFX_ENABLE);

#ifdef VMMC_FEAT_RTCP_XR
      /* Set association between LEC and COD. */
      if (VMMC_SUCCESS(ret))
      {
         ret = VMMC_RES_LEC_AssociatedCodSet(
                     pCh->pPCM->nLecResId,
                     VMMC_CON_SingleDataChannelCodFind(pCh, VMMCDSP_MT_PCM));
      }
#endif /* VMMC_FEAT_RTCP_XR */
   }

   if (VMMC_SUCCESS(ret))
   {
      /* Turn the echo suppressor on or off as requested. */
      if (bEnES == IFX_ENABLE)
      {
         /* Enable the echo suppressor. */

         if ((pCh->pParent->caps.bESonPCM != 0) &&
             !VMMC_RES_ID_VALID(pCh->pPCM->nEsResId))
         {
            /* allocate an ES resource */
            pCh->pPCM->nEsResId = VMMC_RES_ES_Allocate (pCh, VMMC_RES_MOD_PCM);
         }

         if (VMMC_RES_ID_VALID(pCh->pPCM->nEsResId))
         {
            ret = VMMC_RES_ES_ParameterSelect (pCh->pPCM->nEsResId, bEnNLP);

            if (VMMC_SUCCESS (ret))
            {
               ret = VMMC_RES_ES_Enable (pCh->pPCM->nEsResId, IFX_ENABLE);
            }
         }
         else
         {
            VMMC_OS_MutexRelease (&pCh->chAcc);
            RETURN_STATUS (VMMC_statusNoRes);
         }

      }
      else
      {
         /* Disable the echo suppressor. */

         if (VMMC_RES_ID_VALID(pCh->pPCM->nEsResId))
         {
            /* Disable the ES, free resource and forget the resource ID. */
            ret = VMMC_RES_ES_Enable (pCh->pPCM->nEsResId, IFX_DISABLE);
            if (VMMC_SUCCESS (ret))
            {
               ret = VMMC_RES_ES_Release (pCh->pPCM->nEsResId);
            }
            if (VMMC_SUCCESS (ret))
            {
               pCh->pPCM->nEsResId = VMMC_RES_ID_NULL;
            }
         }
      }
   }

   VMMC_OS_MutexRelease (&pCh->chAcc);

   RETURN_STATUS (ret);
}

#ifdef VMMC_FEAT_HDLC

/**
   Write HDLC data to the PCM channel.

   \param  pCh       Pointer to the VMMC channel structure.
   \param  pBuf      Pointer to a buffer with the data to be sent.
   \param  nLen      Data length in bytes.

   \return
   - VMMC_statusInvalCh Resource not valid. No PCM on this channel.
   - VMMC_statusNoRes D-channel resource for PCM channel not available.
   - or error codes of VMMC_RES_HDLC_Write()
   - VMMC_statusOk if successful
*/
IFX_int32_t VMMC_PCM_HDLC_Write (VMMC_CHANNEL *pCh, const IFX_uint8_t *pBuf,
                                 IFX_int32_t nLen)
{
   /* parameter check */
   if (pCh == IFX_NULL)
   {
      /* Cannot log this error because pCh is NULL. */
      return VMMC_statusInvalCh;
   }

   if (pCh->pPCM == IFX_NULL)
   {
      /* errmsg: Resource not valid. Channel number out of range */
      return VMMC_statusInvalCh;
   }

   if (!VMMC_RES_ID_VALID(pCh->pPCM->nHdlcResId))
   {
      IFX_TAPI_EVENT_t tapiEvent;

      memset(&tapiEvent, 0, sizeof(IFX_TAPI_EVENT_t));
      tapiEvent.ch = pCh->nChannel - 1;
      tapiEvent.id = IFX_TAPI_EVENT_FAULT_HDLC_DISABLED;

      IFX_TAPI_Event_Dispatch(pCh->pTapiCh, &tapiEvent);

      /** D-channel resource for PCM channel not available */
      return VMMC_statusNoRes;
   }

   return VMMC_RES_HDLC_Write (pCh->pPCM->nHdlcResId, pBuf, nLen);
}

/**
   Mark HDLC buffer of PCM channel as ready for new data.

   See \ref irq_VMMC_RES_HDLC_BufferReadySet

   \param  pCh    Pointer to the VMMC channel structure.

   \return
      None
*/
IFX_void_t irq_VMMC_PCM_HDLC_BufferReadySet (VMMC_CHANNEL *pCh)
{
   /* parameter check */
   if (pCh == IFX_NULL)
      return;

   if (pCh->pPCM == IFX_NULL)
   {
      /* errmsg: Resource not valid. Channel number out of range */
      return;
   }

   if (!VMMC_RES_ID_VALID(pCh->pPCM->nHdlcResId))
   {
      VMMC_ASSERT(0); /* should never occur */

      /** HDLC resource for PCM channel not available */
      return;
   }

   irq_VMMC_RES_HDLC_BufferReadySet(pCh->pPCM->nHdlcResId);
}

/**
   Unprotected activation or deactivation of PCM HDLC-channel.

   Resource availability check is done. The function returns with error when
   the resource or the timeslot is not available.

   No message will be send and no error returned, when the setting has
   not been changed. The driver checks for the cached values.

   \param  pCh          Pointer to VMMC channel structure.
   \param  pHdlcCfg     Pointer to the current PCM HDLC configuration

   \return
   - VMMC_statusPcmInUse Resource not available
   - VMMC_statusCmdWr Writing the command failed
   - VMMC_statusNoRes The requested resource is not available. The channel does
      not support PCM.
   - VMMC_statusPcmNoTx Tx timeslot for PCM channel activation not available
   - VMMC_statusPcmNoRx Rx timeslot for PCM channel activation not available
   - VMMC_statusOk if successful
*/
static IFX_int32_t VMMC_PCM_HDLC_Cfg (VMMC_CHANNEL *pCh,
                                      IFX_TAPI_PCM_HDLC_CFG_t const *pHdlcCfg)
{
   VMMC_DEVICE    *pDev          = pCh->pParent;
   IFX_int32_t    ret            = VMMC_statusOk;
   IFX_uint32_t   nTimeslot      = 0,
                  nTimeslotCount = 1; /* narrow band only */

   if (!VMMC_PCM_AVAIL (pCh->pPCM, VMMC_PCM_TARGET_HDLC))
   {
      /* errmsg: Resource not available. Channel already in use for another tasks */
      RETURN_STATUS (VMMC_statusPcmInUse);
   }

   nTimeslot = pHdlcCfg->nTimeslot;

   if ((IFX_DISABLE == pHdlcCfg->nEnable) &&
       (VMMC_RES_ID_VALID(pCh->pPCM->nHdlcResId)))
   {
      /* Deactivate the PCM channel */

      VMMC_RES_HDLC_TimeslotGet (pCh->pPCM->nHdlcResId, &nTimeslot);

      /* Disable the HDLC, free resource and forget the resource ID. */
      ret = VMMC_RES_HDLC_Enable (pCh->pPCM->nHdlcResId, IFX_DISABLE);
      if (!VMMC_SUCCESS (ret))
         RETURN_STATUS (ret);

      ret = VMMC_RES_HDLC_Release (pCh->pPCM->nHdlcResId);
      if (!VMMC_SUCCESS (ret))
         RETURN_STATUS (ret);

      pCh->pPCM->nHdlcResId = VMMC_RES_ID_NULL;

      /* markup PCM channel as unused by HDLC */
      VMMC_PCM_RELEASE (pCh->pPCM);

      VMMC_PCM_TimeSlotRelease (pDev, VMMC_PCM_TS_DIR_BOTH,
                                nTimeslot, nTimeslotCount, 0);
   }
   else if ((IFX_ENABLE == pHdlcCfg->nEnable) &&
            (!VMMC_RES_ID_VALID(pCh->pPCM->nHdlcResId)))
   {
      /* Activate the PCM channel */

      /* allocate a HDLC resource */
      pCh->pPCM->nHdlcResId = VMMC_RES_HDLC_Allocate (pCh);

      if (!VMMC_RES_ID_VALID(pCh->pPCM->nHdlcResId))
      {
         /** D-channel resource for PCM channel not available */
         RETURN_STATUS (VMMC_statusNoRes);
      }

      ret = VMMC_PCM_TimeSlotAllocate (pDev, VMMC_PCM_TS_DIR_BOTH,
         nTimeslot, nTimeslotCount, 0);

      if (!VMMC_SUCCESS (ret))
      {
         VMMC_RES_HDLC_Release (pCh->pPCM->nHdlcResId);
         pCh->pPCM->nHdlcResId = VMMC_RES_ID_NULL;

         RETURN_STATUS (ret);
      }

      /* set the comunication timeslot */
      VMMC_RES_HDLC_TimeslotSet (pCh->pPCM->nHdlcResId, nTimeslot);

      ret = VMMC_RES_HDLC_Enable (pCh->pPCM->nHdlcResId, IFX_ENABLE);

      if (!VMMC_SUCCESS (ret))
      {
         /* restore previous state */
         VMMC_PCM_TimeSlotRelease (pDev, VMMC_PCM_TS_DIR_BOTH,
                                   nTimeslot, nTimeslotCount, 0);

         VMMC_RES_HDLC_Release (pCh->pPCM->nHdlcResId);
         pCh->pPCM->nHdlcResId = VMMC_RES_ID_NULL;

         RETURN_STATUS (ret);
      }

      /* markup PCM channel as used by HDLC */
      VMMC_PCM_RESERVE (pCh->pPCM, VMMC_PCM_TARGET_HDLC);
   }

   RETURN_STATUS (ret);
}

/**
   Activate or deactivate a PCM HDLC channel.

   Resource availability check is done. The function returns with error when
   the resource or the timeslot is not available.

   No message will be send and no error returned, when the setting has
   not been changed. The driver checks for the cached values.

   \param  pLLChannel   Pointer to VMMC channel structure.
   \param  pHdlcCfg     Pointer to the current PCM HDLC channel configuration

   \return
   - VMMC_statusCmdWr Writing the command failed
   - VMMC_statusNoRes The requested resource is not available. The channel does
      not support PCM.
   - VMMC_statusPcmNoTx Tx timeslot for PCM channel activation not available
   - VMMC_statusPcmNoRx Rx timeslot for PCM channel activation not available
   - VMMC_statusOk if successful
*/
IFX_int32_t VMMC_TAPI_LL_PCM_HDLC_Cfg (IFX_TAPI_LL_CH_t *pLLChannel,
                                       IFX_TAPI_PCM_HDLC_CFG_t const *pHdlcCfg)
{
   VMMC_CHANNEL   *pCh           = (VMMC_CHANNEL *) pLLChannel;
   VMMC_DEVICE    *pDev          = IFX_NULL;
   IFX_int32_t    ret            = VMMC_statusOk;

   /* parameter check */
   if (pCh == IFX_NULL)
   {
      /* Cannot log this error because pCh is NULL. */
      return VMMC_statusInvalCh;
   }

   if (pHdlcCfg == IFX_NULL)
   {
      RETURN_STATUS (VMMC_statusParam);
   }

   if (pCh->pPCM == IFX_NULL)
   {
      /* errmsg: Resource not valid. Channel number out of range */
      RETURN_STATUS (VMMC_statusInvalCh);
   }

   /* Set pDev after validating pCh. */
   pDev = pCh->pParent;

   /* check if interface needs to be enabled (with defaults) */
   if (!(pDev->nDevState & DS_PCM_EN))
   {
      ret = VMMC_PCM_IF_Cfg (pDev, &ifx_tapi_pcm_if_cfg_defaults);
      /* nDevState DS_PCM_EN has been set inside (if successful) */
      if (ret != VMMC_statusOk)
      {
         RETURN_STATUS (ret);
      }
   }

   /* protect channel against concurrent tasks */
   VMMC_OS_MutexGet (&pCh->chAcc);

   ret = VMMC_PCM_HDLC_Cfg (pCh, pHdlcCfg);

   /* unlock */
   VMMC_OS_MutexRelease (&pCh->chAcc);

   RETURN_STATUS (ret);
}
#endif /* VMMC_FEAT_HDLC */

/**
   Unprotected activation or deactivation of PCM channel loop.

   Resource availability check is done. The function returns with error when
   the resource or the timeslot is not available.

   No message will be send and no error returned, when the setting has
   not been changed. The driver checks for the cached values.

   \param  pCh          Pointer to first VMMC channel structure.
   \param  pCh2         Pointer to second VMMC channel structure.
   \param  pLoopCfg     Pointer to the current PCM channel loop configuration

   \return
   - VMMC_statusPcmInUse Resource not available
   - VMMC_statusCmdWr Writing the command failed
   - VMMC_statusNoRes The requested resource is not available. The channel does
      not support PCM.
   - VMMC_statusPcmNoTx Tx timeslot for PCM channel activation not available
   - VMMC_statusPcmNoRx Rx timeslot for PCM channel activation not available
   - VMMC_statusOk if successful
*/
IFX_int32_t VMMC_PCM_LOOP_Cfg (VMMC_CHANNEL *pCh,
                               VMMC_CHANNEL *pCh2,
                               IFX_TAPI_PCM_LOOP_CFG_t const *pLoopCfg)
{
   VMMC_DEVICE    *pDev          = pCh->pParent;
   PCM_SCHAN_t    *pPcmSCh       = IFX_NULL,
                  *pPcmSCh2      = IFX_NULL;
   IFX_uint32_t   nTimeslotCount = 1; /* narrow band only */
   IFX_uint16_t   nSChResId      = 0;
   IFX_int32_t    ret            = VMMC_statusOk;

   if (!VMMC_PCM_AVAIL (pCh->pPCM, VMMC_PCM_TARGET_SHORTCUT))
   {
      /* errmsg: Resource not available. Channel already in use for another tasks */
      RETURN_STATUS (VMMC_statusPcmInUse);
   }

   if (!VMMC_PCM_AVAIL (pCh2->pPCM, VMMC_PCM_TARGET_SHORTCUT))
   {
      /* errmsg: Resource not available. Channel already in use for another tasks */
      RETURN_STATUS (VMMC_statusPcmInUse);
   }

   pPcmSCh  = &pCh->pPCM->fw_pcm_sch;
   pPcmSCh2 = &pCh2->pPCM->fw_pcm_sch;

   /* Check that channels can be manipulated. Two channels have to be
      in the same enabling state. Only channels which connected
      each to other can be disabled. */
   if ((pPcmSCh->EN != pPcmSCh2->EN) ||
      ((pPcmSCh->EN == PCM_SCHAN_ENABLE) &&
       (pPcmSCh->SCHAN != pPcmSCh2->CHAN))
      )
   {
      /* That channels connected to the another channels */
      RETURN_STATUS (VMMC_statusInvalCh);
   }

   if ((IFX_DISABLE == pLoopCfg->nEnable) &&
       (PCM_SCHAN_ENABLE == pPcmSCh->EN))
   {
      /* Deactivate the PCM loop */

      /* disable PCM channel: This will deactivate the timeslots */
      pPcmSCh->EN = PCM_SCHAN_DISABLE;
      ret = CmdWrite (pDev, (IFX_uint32_t *) pPcmSCh, PCM_SCHAN_LEN);
      if (!VMMC_SUCCESS (ret))
      {
         /* restore previous state */
         pPcmSCh->EN = PCM_SCHAN_ENABLE;
         RETURN_STATUS (ret);
      }

      pPcmSCh2->EN = PCM_SCHAN_DISABLE;

      /* markup PCM channel as unused as shortcut */
      VMMC_PCM_RELEASE (pCh->pPCM);
      VMMC_PCM_RELEASE (pCh2->pPCM);

      VMMC_PCM_TimeSlotRelease (pDev, VMMC_PCM_TS_DIR_BOTH,
         pPcmSCh->TS, nTimeslotCount, 0);
      VMMC_PCM_TimeSlotRelease (pDev, VMMC_PCM_TS_DIR_BOTH,
         pPcmSCh->STS, nTimeslotCount, 0);

      /* device members must be protected against concurrent access */
      VMMC_OS_MutexGet (&pDev->mtxMemberAcc);
      /* release PCM channel loop resource */
      pDev->PcmSchRes[pPcmSCh->PCMSR] = IFX_LOW;
      /* unlock */
      VMMC_OS_MutexRelease (&pDev->mtxMemberAcc);
   }
   else if ((IFX_ENABLE == pLoopCfg->nEnable) &&
            (PCM_SCHAN_DISABLE == pPcmSCh->EN))
   {
      /* Activate the PCM loop */

      /* find free PCM channel loop resource */
      for (nSChResId = 0;
          ((nSChResId < VMMC_PCM_S_CH_RES_CNT) &&
           (pDev->PcmSchRes[nSChResId] == IFX_HIGH));
          nSChResId++);

      if (nSChResId == VMMC_PCM_S_CH_RES_CNT)
      {
         /** PCM channel loop resource not available */
         RETURN_STATUS (VMMC_statusNoRes);
      }

      ret = VMMC_PCM_TimeSlotAllocate (pDev, VMMC_PCM_TS_DIR_BOTH,
         pLoopCfg->nTimeslot1, nTimeslotCount, 0);
      if (!VMMC_SUCCESS (ret))
         RETURN_STATUS (ret);

      ret = VMMC_PCM_TimeSlotAllocate (pDev, VMMC_PCM_TS_DIR_BOTH,
         pLoopCfg->nTimeslot2, nTimeslotCount, 0);
      if (!VMMC_SUCCESS (ret))
      {
         VMMC_PCM_TimeSlotRelease (pDev, VMMC_PCM_TS_DIR_BOTH,
            pLoopCfg->nTimeslot1, nTimeslotCount, 0);

         RETURN_STATUS (ret);
      }

      /* Set the timeslots and enable the channel */
      pPcmSCh->TS    = pLoopCfg->nTimeslot1;
      pPcmSCh->SCHAN = pPcmSCh2->CHAN;
      pPcmSCh->STS   = pLoopCfg->nTimeslot2;
      pPcmSCh->PCMSR = nSChResId;
      pPcmSCh->EN    = PCM_SCHAN_ENABLE;

      ret = CmdWrite (pDev, (IFX_uint32_t *) pPcmSCh, PCM_SCHAN_LEN);
      if (!VMMC_SUCCESS (ret))
      {
         /* restore previous state */
         pPcmSCh->EN = PCM_SCHAN_DISABLE;

         VMMC_PCM_TimeSlotRelease (pDev, VMMC_PCM_TS_DIR_BOTH,
            pLoopCfg->nTimeslot1, nTimeslotCount, 0);
         VMMC_PCM_TimeSlotRelease (pDev, VMMC_PCM_TS_DIR_BOTH,
            pLoopCfg->nTimeslot2, nTimeslotCount, 0);

         RETURN_STATUS (ret);
      }

      pPcmSCh2->TS    = pPcmSCh->STS;
      pPcmSCh2->SCHAN = pPcmSCh->CHAN;
      pPcmSCh2->STS   = pPcmSCh->TS;
      pPcmSCh2->PCMSR = pPcmSCh->PCMSR;
      pPcmSCh2->EN    = PCM_SCHAN_ENABLE;

      /* markup PCM channel as used as shortcut */
      VMMC_PCM_RESERVE (pCh->pPCM, VMMC_PCM_TARGET_SHORTCUT);
      VMMC_PCM_RESERVE (pCh2->pPCM, VMMC_PCM_TARGET_SHORTCUT);

      /* device members must be protected against concurrent access */
      VMMC_OS_MutexGet (&pDev->mtxMemberAcc);
      /* reserve PCM D-channel resource */
      pDev->PcmSchRes[nSChResId] = IFX_HIGH;
      /* unlock */
      VMMC_OS_MutexRelease (&pDev->mtxMemberAcc);
   }

   RETURN_STATUS (ret);
}

/**
   Activate or deactivate a PCM channel loop.

   Resource availability check is done. The function returns with error when
   the resource or the timeslot is not available.

   No message will be send and no error returned, when the setting has
   not been changed. The driver checks for the cached values.

   \param  pLLChannel   Pointer to VMMC channel structure.
   \param  pLoopCfg     Pointer to the current PCM channel loop configuration

   \return
   - VMMC_statusCmdWr Writing the command failed
   - VMMC_statusNoRes The requested resource is not available. The channel does
      not support PCM.
   - VMMC_statusPcmNoTx Tx timeslot for PCM channel activation not available
   - VMMC_statusPcmNoRx Rx timeslot for PCM channel activation not available
   - VMMC_statusOk if successful

   \remarks
   Two PCM channels are used: the PCM channel addressed by the fd and
   the one in the next channel fd. That means if a shortcut is programmed
   for PCM channel 2, PCM channel 3 is also used for this shortcut.
*/
IFX_int32_t VMMC_TAPI_LL_PCM_LOOP_Cfg (IFX_TAPI_LL_CH_t *pLLChannel,
                                       IFX_TAPI_PCM_LOOP_CFG_t const *pLoopCfg)
{
   VMMC_CHANNEL   *pCh           = (VMMC_CHANNEL *) pLLChannel,
                  *pCh2          = IFX_NULL;
   VMMC_DEVICE    *pDev          = IFX_NULL;
   IFX_int32_t    ret            = VMMC_statusOk;

   /* parameter check */
   if (pCh == IFX_NULL)
   {
      /* Cannot log this error because pCh is NULL. */
      return VMMC_statusInvalCh;
   }

   if ((pLoopCfg == IFX_NULL) ||
       ((pLoopCfg->nEnable == IFX_ENABLE) &&
        (pLoopCfg->nTimeslot1 == pLoopCfg->nTimeslot2)))
   {
      RETURN_STATUS (VMMC_statusParam);
   }

   if (pCh->pPCM == IFX_NULL)
   {
      /* errmsg: Resource not valid. Channel number out of range */
      RETURN_STATUS (VMMC_statusInvalCh);
   }

   /* Set pDev after validating pCh. */
   pDev = pCh->pParent;

   /* Two PCM channels are used: the PCM channel addressed by the fd and
   the one in the next channel fd. */
   if (pCh->nChannel < pCh->pParent->caps.nPCM)
   {
      pCh2 = pDev->pChannel + pCh->nChannel; /* note: nChannel range [1:x] */
   }
   else if (pCh->nChannel == pCh->pParent->caps.nPCM)
   {
      pCh2 = pDev->pChannel + 0;
   }
   else
   {
      RETURN_STATUS (VMMC_statusInvalCh);
   }

   if (pCh2->pPCM == IFX_NULL)
   {
      /* errmsg: Resource not valid. Channel number out of range */
      RETURN_STATUS (VMMC_statusInvalCh);
   }

   /* check if interface needs to be enabled (with defaults) */
   if (!(pDev->nDevState & DS_PCM_EN))
   {
      ret = VMMC_PCM_IF_Cfg (pDev, &ifx_tapi_pcm_if_cfg_defaults);
      /* nDevState DS_PCM_EN has been set inside (if successful) */
      if (VMMC_SUCCESS (ret))
         RETURN_STATUS (ret);
   }

   /* protect channel against concurrent tasks */
   VMMC_OS_MutexGet (&pCh->chAcc);
   VMMC_OS_MutexGet (&pCh2->chAcc);

   ret = VMMC_PCM_LOOP_Cfg (pCh, pCh2, pLoopCfg);

   /* unlock */
   VMMC_OS_MutexRelease (&pCh2->chAcc);
   VMMC_OS_MutexRelease (&pCh->chAcc);

   RETURN_STATUS (ret);
}

/**
   Prepare and check PCM channel parameters and call the target configuration
   function to configure the PCM interface.

   \param  pLLChannel   Pointer to VMMC channel structure.
   \param  pPCMConfig   Contains the new configuration for PCM interface.

   \return
   - VMMC_statusInvalCh Resource not valid. No PCM on this channel.
   - VMMC_statusPcmChNotEn Setting configuration needs the PCM channel to be
     deactivated which is not the case.
   - VMMC_statusCmdWr Writing the command failed
   - VMMC_statusNotSupported Wrong parameters passed. This code is returned
     when the nResolution has an invalid value, one or both time slots are
     larger then 127 or the specified highway is invalid.
   - VMMC_statusPcmTsInvalid PCM timeslot given out of range
   - VMMC_statusPcmHwInvalid Number of PCM Highway out of range
   - VMMC_statusOk if successful

   \remarks
   Performs error checking according to the underlying device capability.
   This function just checks the configuration. No firmware message is sent.
*/
IFX_int32_t VMMC_TAPI_LL_PCM_CH_Cfg (IFX_TAPI_LL_CH_t *pLLChannel,
                                     IFX_TAPI_PCM_CFG_t const *pPCMConfig)
{
   VMMC_CHANNEL *pCh = (VMMC_CHANNEL *) pLLChannel;
   VMMC_DEVICE  *pDev;
   IFX_uint32_t nTimeslotCount = 1,
                nSplit = 0;

   /* parameter check */
   if (pCh == IFX_NULL)
   {
      /* Cannot log this error because pCh is NULL. */
      return VMMC_statusInvalCh;
   }

   if (pCh->pPCM == IFX_NULL)
   {
      /* errmsg: Resource not valid. Channel number out of range */
      RETURN_STATUS (VMMC_statusInvalCh);
   }

   /* protect channel against concurrent tasks */
   VMMC_OS_MutexGet (&pCh->chAcc);

   if (!VMMC_PCM_AVAIL (pCh->pPCM, VMMC_PCM_TARGET_VOICE))
   {
      /* unlock */
      VMMC_OS_MutexRelease (&pCh->chAcc);

      /* errmsg: Resource not available. Channel already in use for another tasks */
      RETURN_STATUS (VMMC_statusPcmInUse);
   }

   /* Set pDev after validating pCh. */
   pDev = pCh->pParent;

   /* configuration changes are allowed only while the PCM ch is disabled */
   if (pCh->pPCM->fw_pcm_ch.EN)
   {
      /* unlock */
      VMMC_OS_MutexRelease (&pCh->chAcc);

      /* errmsg: PCM channel cfg cannot be changed while channel is active */
      RETURN_STATUS(VMMC_statusPcmChEn);
   }

   /* check if G.722 codec is available */
   if(pPCMConfig->nResolution == IFX_TAPI_PCM_RES_WB_G722)
   {
      if (!(pDev->caps.PCOD & EDSP_CAP_PCMCOD_PG722) ||
                                         (pCh->nChannel > pDev->caps.nPCMCOD))
      {
         /* unlock */
         VMMC_OS_MutexRelease (&pCh->chAcc);
         /* errmsg: PCM channel codec is not available */
         RETURN_STATUS(VMMC_statusPcmChCoderNotAvail);
      }
   }

   /* check if G.726 codec is available */
   if((pPCMConfig->nResolution >= IFX_TAPI_PCM_RES_NB_G726_16) &&
      (pPCMConfig->nResolution <= IFX_TAPI_PCM_RES_NB_G726_40))
   {
      if (!(pDev->caps.PCOD & EDSP_CAP_PCMCOD_PG726) ||
           (pCh->nChannel > pDev->caps.nPCMCOD))
      {
         /* unlock */
         VMMC_OS_MutexRelease (&pCh->chAcc);
         /* errmsg: PCM channel codec is not available */
         RETURN_STATUS(VMMC_statusPcmChCoderNotAvail);
      }
   }

   /* check PCM resolution for supported values and set the number of
      timeslots required for this setting to be used in the check below. */
   switch (pPCMConfig->nResolution)
   {
      case IFX_TAPI_PCM_RES_NB_ALAW_8BIT:
      case IFX_TAPI_PCM_RES_NB_ULAW_8BIT:
      case IFX_TAPI_PCM_RES_WB_G722:
      case IFX_TAPI_PCM_RES_NB_G726_16:
      case IFX_TAPI_PCM_RES_NB_G726_24:
      case IFX_TAPI_PCM_RES_NB_G726_32:
      case IFX_TAPI_PCM_RES_NB_G726_40:
         nTimeslotCount = 1;
         break;
      case IFX_TAPI_PCM_RES_NB_LINEAR_16BIT:
      case IFX_TAPI_PCM_RES_WB_ALAW_8BIT:
      case IFX_TAPI_PCM_RES_WB_ULAW_8BIT:
         nTimeslotCount = 2;
         break;
      case IFX_TAPI_PCM_RES_WB_LINEAR_16BIT:
         nTimeslotCount = 4;
         break;
      case IFX_TAPI_PCM_RES_WB_LINEAR_SPLIT_16BIT:
         if (pDev->caps.bPcmSplitTs != 1)
         {
            /* unlock */
            VMMC_OS_MutexRelease (&pCh->chAcc);
            /* FW does not support split timeslots for WB mode. */
            /* errmsg: Requested PCM resolution not supported. */
            RETURN_STATUS(VMMC_statusPcmResolutionNotSupported);
         }
         nTimeslotCount = 2; /* 2nd group has just 2 timeslots */
         nSplit = pDev->nMaxTimeslot >> 1; /* 2nd group offset from 1st group */
         break;
      default:
         /* unlock */
         VMMC_OS_MutexRelease (&pCh->chAcc);

         RETURN_STATUS (VMMC_statusNotSupported);
   }

   /* Here we check that the PCM sample has enough space on the given timeslot.
      Because we have also coders that use 2 and 4 consecutive timeslots we
      set above the number of timeslots used by one sample and then check here
      that the maximum timeslot is not exceeded.
      Note: Timeslots start counting from 0 */
   if (((pPCMConfig->nTimeslotRX + nTimeslotCount + nSplit) > pDev->nMaxTimeslot) ||
       ((pPCMConfig->nTimeslotTX + nTimeslotCount + nSplit) > pDev->nMaxTimeslot))
   {
      /* unlock */
      VMMC_OS_MutexRelease (&pCh->chAcc);

      /* errmsg:  PCM timeslot given out of range */
      RETURN_STATUS (VMMC_statusPcmTsInvalid);
   }

   if (pPCMConfig->nHighway >= PCM_HIGHWAY)
   {
      /* unlock */
      VMMC_OS_MutexRelease (&pCh->chAcc);

      /* errmsg:  Number of PCM Highway out of range */
      RETURN_STATUS (VMMC_statusPcmHwInvalid);
   }

   /* unlock */
   VMMC_OS_MutexRelease (&pCh->chAcc);

   return VMMC_statusOk;
}


/**
   Activate or deactivate a PCM channel.

   The configuration must be set previously with the low level function
   VMMC_TAPI_LL_PCM_Ch_Cfg.
   Resource availability check is done. The function returns with error when
   the resource or the timeslot is not available.

   No message will be send and no error returned, when the setting has
   not been changed. The driver checks for the cached values.

   When deactivating the channel a possibly running LEC and ES are automatically
   deactivated and released. No error is reported in this case.

   \param  pLLChannel   Pointer to VMMC channel structure.
   \param  nMode        Activation mode
                        - 1: timeslot activated
                        - 0: timeslot deactivated
   \param  pPcmCfg      Pointer to the current PCM configuration

   \return
   - VMMC_statusCmdWr Writing the command failed
   - VMMC_statusNoRes The requested resource is not available. The channel does
      not support PCM.
   - VMMC_statusPcmNoTx Tx timeslot for PCM channel activation not available
   - VMMC_statusPcmNoRx Rx timeslot for PCM channel activation not available
   - VMMC_statusOk if successful

   \note
   Open: resource check for 16 bit and 8 bit necessary? Is on the fly
   switching without resource limitation possible?
*/
IFX_int32_t VMMC_TAPI_LL_PCM_CH_Enable (IFX_TAPI_LL_CH_t *pLLChannel,
                                        IFX_uint32_t nMode,
                                        IFX_TAPI_PCM_CFG_t *pPcmCfg)
{
   VMMC_CHANNEL *pCh = (VMMC_CHANNEL *) pLLChannel;
   VMMC_DEVICE  *pDev;
   PCM_CHAN_t   *pPcmCh;
   IFX_uint32_t  nTimeslotCount = 1,
                 nSplit = 0;
   IFX_int32_t   ret = VMMC_statusOk;

   /* parameter check */
   if (pCh == IFX_NULL)
   {
      /* Cannot log this error because pCh is NULL. */
      return VMMC_statusInvalCh;
   }

   if (pCh->pPCM == IFX_NULL)
   {
      /* errmsg: Resource not valid. Channel number out of range */
      RETURN_STATUS (VMMC_statusInvalCh);
   }

   /* Set pDev after validating pCh. */
   pDev = pCh->pParent;

   /* check if interface needs to be enabled (with defaults) */
   if (!(pDev->nDevState & DS_PCM_EN))
   {
      ret = VMMC_PCM_IF_Cfg (pDev, &ifx_tapi_pcm_if_cfg_defaults);
      /* nDevState DS_PCM_EN has been set inside (if successful) */
      if (ret != VMMC_statusOk)
      {
         RETURN_STATUS (ret);
      }
   }

   /* protect channel against concurrent tasks */
   VMMC_OS_MutexGet (&pCh->chAcc);

   if (!VMMC_PCM_AVAIL (pCh->pPCM, VMMC_PCM_TARGET_VOICE))
   {
      /* unlock */
      VMMC_OS_MutexRelease (&pCh->chAcc);

      /* errmsg: Resource not available. Channel already in use for another tasks */
      RETURN_STATUS (VMMC_statusPcmInUse);
   }

   pPcmCh = &pCh->pPCM->fw_pcm_ch;

   if ((nMode == 0) && (pPcmCh->EN != PCM_CHAN_DISABLE))
   {
      /* Deactivate the PCM channel */

      /* determine the number of timeslots required for this coder */
      switch (pPcmCh->COD)
      {
      case PCM_CHAN_COD_LIN:
         nTimeslotCount = ((pPcmCh->ISR == 1) && (pPcmCh->UD == 0)) ? 4 : 2;
         break;
      case PCM_CHAN_COD_G711_ALAW:
      case PCM_CHAN_COD_G711_MLAW:
         nTimeslotCount = ((pPcmCh->ISR == 1) && (pPcmCh->UD == 0)) ? 2 : 1;
         break;
      case PCM_CHAN_COD_G722:
      case PCM_CHAN_COD_G726_16:
      case PCM_CHAN_COD_G726_24:
      case PCM_CHAN_COD_G726_32:
      case PCM_CHAN_COD_G726_40:
         nTimeslotCount = 1;
         break;
      default:
         /* default was only added to make the compiler happy */
         VMMC_ASSERT(0); /* unknown coder field value */
         break;
      }

      /* free the timeslots in the map */
      VMMC_PCM_TimeSlotRelease (pDev, VMMC_PCM_TS_DIR_RX,
         pPcmCh->RTS, nTimeslotCount, (pPcmCh->STS == 1));
      VMMC_PCM_TimeSlotRelease (pDev, VMMC_PCM_TS_DIR_TX,
         pPcmCh->XTS, nTimeslotCount, (pPcmCh->STS == 1));

      /* Before disabling the channel the LEC and ES need to be disabled.
         Because we cannot activate them again we also free the resource. */

      /* Disable the echo suppressor. */
      if (VMMC_RES_ID_VALID(pCh->pPCM->nEsResId))
      {
         /* Disable the ES, free resource and forget the resource ID. */
         ret = VMMC_RES_ES_Enable (pCh->pPCM->nEsResId, IFX_DISABLE);
         if (VMMC_SUCCESS (ret))
         {
            ret = VMMC_RES_ES_Release (pCh->pPCM->nEsResId);
         }
         if (VMMC_SUCCESS (ret))
         {
            pCh->pPCM->nEsResId = VMMC_RES_ID_NULL;
         }
      }

      /* Disable the line echo canceller. */
      if (VMMC_RES_ID_VALID(pCh->pPCM->nLecResId))
      {
         /* Disable the LEC, free resource and forget the resource ID. */
         ret = VMMC_RES_LEC_Enable (pCh->pPCM->nLecResId, IFX_DISABLE);
         if (VMMC_SUCCESS (ret))
         {
            ret = VMMC_RES_LEC_Release (pCh->pPCM->nLecResId);
         }
         if (VMMC_SUCCESS (ret))
         {
            pCh->pPCM->nLecResId = VMMC_RES_ID_NULL;
         }
      }

      /* disable PCM channel: This will deactivate the timeslots */
      pPcmCh->EN = PCM_CHAN_DISABLE;

      ret = CmdWrite (pDev, (IFX_uint32_t *) pPcmCh, PCM_CHAN_LEN);

      /* markup PCM channel as unused */
      VMMC_PCM_RELEASE (pCh->pPCM);

      /* After deactivation set the sampling mode to off */
      VMMC_CON_ModuleSamplingModeSet (pCh, VMMCDSP_MT_PCM, VMMC_CON_SMPL_OFF);

      /* release channel for sample rate configuration */
      VMMC_OS_MutexRelease (&pCh->chAcc);
      /* reevaluate the conference that this module belongs to */
      VMMC_CON_MatchConfSmplRate(pCh, VMMCDSP_MT_PCM);
      /* restore lock */
      VMMC_OS_MutexGet (&pCh->chAcc);
   }
   else if ((nMode == 1) && (pPcmCh->EN == PCM_CHAN_DISABLE))
   {
      /* Activate the PCM channel */

      /* determine the number of timeslots required for this coder */
      switch (pPcmCfg->nResolution)
      {
      case IFX_TAPI_PCM_RES_NB_ALAW_8BIT:
      case IFX_TAPI_PCM_RES_NB_ULAW_8BIT:
      case IFX_TAPI_PCM_RES_WB_G722:
      case IFX_TAPI_PCM_RES_NB_G726_16:
      case IFX_TAPI_PCM_RES_NB_G726_24:
      case IFX_TAPI_PCM_RES_NB_G726_32:
      case IFX_TAPI_PCM_RES_NB_G726_40:
         nTimeslotCount = 1;
         break;
      case IFX_TAPI_PCM_RES_NB_LINEAR_16BIT:
      case IFX_TAPI_PCM_RES_WB_ALAW_8BIT:
      case IFX_TAPI_PCM_RES_WB_ULAW_8BIT:
         nTimeslotCount = 2;
         break;
      case IFX_TAPI_PCM_RES_WB_LINEAR_16BIT:
         nTimeslotCount = 4;
         break;
      case IFX_TAPI_PCM_RES_WB_LINEAR_SPLIT_16BIT:
         nTimeslotCount = 4;
         nSplit = 1; /* flag that split timeslots should be used */
         break;
      default:
         /* no error case needed - already checked by VMMC_TAPI_LL_PCM_CH_Cfg() */
         /* default was only added to make the compiler happy */
         VMMC_ASSERT(0); /* unknown coder field value */
         break;
      }

      /* allocate the timeslots in the map */
      ret = VMMC_PCM_TimeSlotAllocate (pDev, VMMC_PCM_TS_DIR_RX,
         pPcmCfg->nTimeslotRX, nTimeslotCount, nSplit);
      if (!(VMMC_SUCCESS(ret)))
      {
         /* unlock */
         VMMC_OS_MutexRelease (&pCh->chAcc);
         RETURN_STATUS (ret);
      }

      ret = VMMC_PCM_TimeSlotAllocate (pDev, VMMC_PCM_TS_DIR_TX,
         pPcmCfg->nTimeslotTX, nTimeslotCount, nSplit);

      if (!(VMMC_SUCCESS(ret)))
      {
         VMMC_PCM_TimeSlotRelease (pDev, VMMC_PCM_TS_DIR_RX,
            pPcmCfg->nTimeslotRX, nTimeslotCount, nSplit);

         /* unlock */
         VMMC_OS_MutexRelease (&pCh->chAcc);
         RETURN_STATUS (ret);
      }

      /* Set the coder field in the cached message */
      pPcmCh->STS = 0;
      switch (pPcmCfg->nResolution)
      {
      case IFX_TAPI_PCM_RES_NB_ALAW_8BIT:
      case IFX_TAPI_PCM_RES_WB_ALAW_8BIT:
         pPcmCh->COD = PCM_CHAN_COD_G711_ALAW;
         break;
      case IFX_TAPI_PCM_RES_NB_ULAW_8BIT:
      case IFX_TAPI_PCM_RES_WB_ULAW_8BIT:
         pPcmCh->COD = PCM_CHAN_COD_G711_MLAW;
         break;
      case IFX_TAPI_PCM_RES_WB_G722:
         pPcmCh->COD = PCM_CHAN_COD_G722;
         break;
      case IFX_TAPI_PCM_RES_NB_G726_16:
         pPcmCh->COD = PCM_CHAN_COD_G726_16;
         break;
      case IFX_TAPI_PCM_RES_NB_G726_24:
         pPcmCh->COD = PCM_CHAN_COD_G726_24;
         break;
      case IFX_TAPI_PCM_RES_NB_G726_32:
         pPcmCh->COD = PCM_CHAN_COD_G726_32;
         break;
      case IFX_TAPI_PCM_RES_NB_G726_40:
         pPcmCh->COD = PCM_CHAN_COD_G726_40;
         break;
      case IFX_TAPI_PCM_RES_WB_LINEAR_SPLIT_16BIT:
         pPcmCh->COD = PCM_CHAN_COD_LIN;
         pPcmCh->STS = 1;
         break;
      case IFX_TAPI_PCM_RES_NB_LINEAR_16BIT:
      case IFX_TAPI_PCM_RES_WB_LINEAR_16BIT:
      default:
         pPcmCh->COD = PCM_CHAN_COD_LIN;
         break;
      }

      /* Set wideband or narrowband operation mode */
      switch (pPcmCfg->nResolution)
      {
      case IFX_TAPI_PCM_RES_WB_ALAW_8BIT:
      case IFX_TAPI_PCM_RES_WB_ULAW_8BIT:
      case IFX_TAPI_PCM_RES_WB_LINEAR_16BIT:
      case IFX_TAPI_PCM_RES_WB_LINEAR_SPLIT_16BIT:
      case IFX_TAPI_PCM_RES_WB_G722:
         /* Wideband mode */
         VMMC_CON_ModuleSamplingModeSet (pCh, VMMCDSP_MT_PCM,
                                         VMMC_CON_SMPL_WB);
         break;
      default:
         /* Narrowband mode */
         VMMC_CON_ModuleSamplingModeSet (pCh, VMMCDSP_MT_PCM,
                                         VMMC_CON_SMPL_NB);
         break;
      }

      /* release channel for sample rate configuration */
      VMMC_OS_MutexRelease (&pCh->chAcc);
      /* reevaluate the conference that this module belongs to */
      VMMC_CON_MatchConfSmplRate(pCh, VMMCDSP_MT_PCM);
      /* restore lock */
      VMMC_OS_MutexGet (&pCh->chAcc);

      /* markup PCM channel as used */
      VMMC_PCM_RESERVE (pCh->pPCM, VMMC_PCM_TARGET_VOICE);

      /* ADPCM bitpacking selection */
      pPcmCh->BP   = (pPcmCfg->nBitPacking == IFX_TAPI_PCM_BITPACK_LSB) ? 0 : 1;
      /* PCM sample swapping */
      pPcmCh->SWP  = (pPcmCfg->nSampleSwap == IFX_TAPI_PCM_SAMPLE_SWAP_DISABLED)
                      ? 0 : 1;

      /* Set the timeslots and enable the channel */
      pPcmCh->XTS  = pPcmCfg->nTimeslotTX;
      pPcmCh->RTS  = pPcmCfg->nTimeslotRX;
      pPcmCh->EN   = PCM_CHAN_ENABLE;

      ret = CmdWrite (pDev, (IFX_uint32_t *) pPcmCh, PCM_CHAN_LEN);
   }

   /* unlock */
   VMMC_OS_MutexRelease (&pCh->chAcc);

   RETURN_STATUS (ret);
}


/**
   Sets the PCM interface volume.

   Gain Parameter are given in 'dB'. The range is -24dB ... 12dB. The message
   is always written even if the values are the same already written into FW.

   \param  pLLChannel   Pointer to VMMC channel structure.
   \param  pVol         Pointer to IFX_TAPI_LINE_VOLUME_t structure.

   \return
      - VMMC_statusCmdWr Writing the command failed.
      - VMMC_statusFuncParm Wrong parameters passed. This code is returned
        when any gain parameter is lower than -24 dB or higher than 12 dB.
      - VMMC_statusOk if successful.
*/
IFX_int32_t VMMC_TAPI_LL_PCM_Volume_Set (IFX_TAPI_LL_CH_t *pLLChannel,
                                         IFX_TAPI_LINE_VOLUME_t const *pVol)
{
   VMMC_CHANNEL *pCh = (VMMC_CHANNEL *) pLLChannel;
   VMMC_DEVICE  *pDev;
   PCM_CHAN_t   *pPcmCh;
   IFX_int32_t   ret = VMMC_statusOk;

   /* parameter check */
   if (pCh == IFX_NULL)
   {
      /* Cannot log this error because pCh is NULL. */
      return VMMC_statusInvalCh;
   }
   if (pCh->pPCM == IFX_NULL)
   {
      /* errmsg: Resource not valid. Channel number out of range */
      RETURN_STATUS (VMMC_statusInvalCh);
   }

   /* Set pDev after validating pCh. */
   pDev = pCh->pParent;

   /* range check, because gain var is integer */
   if ((pVol->nGainTx < VMMC_VOLUME_GAIN_MIN) ||
       (pVol->nGainTx > VMMC_VOLUME_GAIN_MAX) ||
       (pVol->nGainRx < VMMC_VOLUME_GAIN_MIN) ||
       (pVol->nGainRx > VMMC_VOLUME_GAIN_MAX))
   {
      /* parameters are out of the supported range */
      RETURN_STATUS (VMMC_statusParam);
   }

   /* protect fw msg */
   VMMC_OS_MutexGet (&pCh->chAcc);

   pPcmCh = &pCh->pPCM->fw_pcm_ch;

   /* set gains in the cached fw message no matter if enabled or disabled */
   pPcmCh->GAIN1 = VMMC_Gaintable[pVol->nGainTx + (-VMMC_VOLUME_GAIN_MIN)];
   pPcmCh->GAIN2 = VMMC_Gaintable[pVol->nGainRx + (-VMMC_VOLUME_GAIN_MIN)];

   if (pPcmCh->EN != PCM_CHAN_DISABLE)
   {
      ret = CmdWrite (pDev, (IFX_uint32_t *)pPcmCh, PCM_CHAN_LEN);
   }

   /* release lock */
   VMMC_OS_MutexRelease (&pCh->chAcc);

   RETURN_STATUS (ret);
}


/**
   Switches on/off the HP filter in the decoder path of the PCM module

   The message is always written even if the values are the same already
   written into FW.

   \param  pLLChannel   Pointer to VMMC channel structure.
   \param  bHp          IFX_FALSE to switch HP filter off,
                        IFX_TRUE  to switch HP filter on.
   \return
      - VMMC_statusCmdWr Writing the command has failed
      - VMMC_statusOk if successful
*/
IFX_int32_t VMMC_TAPI_LL_PCM_DEC_HP_Set (IFX_TAPI_LL_CH_t *pLLChannel,
                                         IFX_boolean_t bHp)
{
   VMMC_CHANNEL *pCh  = (VMMC_CHANNEL *) pLLChannel;
   VMMC_DEVICE  *pDev;
   PCM_CHAN_t   *pPcmCh;
   IFX_int32_t   ret  = VMMC_statusOk;

   /* parameter check */
   if (pCh == IFX_NULL)
   {
      /* Cannot log this error because pCh is NULL. */
      return VMMC_statusInvalCh;
   }
   if (pCh->pPCM == IFX_NULL)
   {
      /* errmsg: Resource not valid. Channel number out of range */
      RETURN_STATUS (VMMC_statusInvalCh);
   }

   /* Set pDev after validating pCh. */
   pDev = pCh->pParent;

   /* protect fw msg */
   VMMC_OS_MutexGet (&pCh->chAcc);

   pPcmCh = &pCh->pPCM->fw_pcm_ch;

   /* set HP flag no matter if the channel is enabled or disabled */
   pPcmCh->HP = (bHp == IFX_TRUE) ? PCM_CHAN_HP_ON : PCM_CHAN_HP_OFF;

   if (pPcmCh->EN != PCM_CHAN_DISABLE)
   {
      ret = CmdWrite (pDev, (IFX_uint32_t *)pPcmCh, PCM_CHAN_LEN);
   }

   /* release lock */
   VMMC_OS_MutexRelease (&pCh->chAcc);

   RETURN_STATUS (ret);
}


/**
   Configure and enable PCM interface / wrapper function for drv_tapi

   \param  pLLDev       Pointer to the device structure.
   \param  pCfg         Pointer to the coefficient structure.

   \return
   - VMMC_statusCmdWr Writing the command has failed
   - VMMC_statusOk if successful
*/
IFX_int32_t VMMC_TAPI_LL_PCM_IF_Cfg (IFX_TAPI_LL_DEV_t *pLLDev,
                                     const IFX_TAPI_PCM_IF_CFG_t *pCfg)
{
   return VMMC_PCM_IF_Cfg ((VMMC_DEVICE*) pLLDev, pCfg);
}


/**
   Configure and enable PCM interface

   \param  pDev         Pointer to the device structure.
   \param  pCfg         Pointer to the coefficient structure.

   \return
   - VMMC_statusCmdWr Writing the command has failed
   - VMMC_statusOk if successful
*/
static IFX_int32_t VMMC_PCM_IF_Cfg (VMMC_DEVICE *pDev,
                                    const IFX_TAPI_PCM_IF_CFG_t *pCfg)
{
   PCM_CTRL_t   PcmCtrlCmd;
   PCM_CTRL_t  *pPcmCtrlCmd = &PcmCtrlCmd;
   IFX_uint8_t  i;
   IFX_int32_t  ret = VMMC_statusOk;

   /* sanity check - refuse slave mode with MCTS != 0 (HW Bug) */
   if ((pCfg->nOpMode != IFX_TAPI_PCM_IF_MODE_MASTER) && pCfg->nMCTS)
   {
      /* errmsg: Invalid slave mode with MCTS = 0 */
      RETURN_DEVSTATUS (VMMC_statusPcmSlaveCfg);
   }

   /* Gather the status of all PCM channels, if any is active we cannot
      reconfigure the PCM if-mode. */
   for (i = 0; i < VMMC_MAX_CH_NR; i++)
   {
      if ((pDev->pChannel[i].pPCM != IFX_NULL) &&
          (pDev->pChannel[i].pPCM->fw_pcm_ch.EN != PCM_CHAN_DISABLE))
      {
         /* errmsg: PCM interface cannot be configured while any PCM channel
                    is active */
         RETURN_DEVSTATUS(VMMC_statusPcmIfCfgWhileActive);
      }
   }

   /* sanity checks */
   if (pCfg->nDoubleClk == IFX_DISABLE)
   {  /* single clocking */
      if (pCfg->nShift == IFX_ENABLE)
      {
         /* errmsg: PCM interface configuration invalid,
                    Shift Edge for Double clocking only */
         RETURN_DEVSTATUS(VMMC_statusPcmIfCfgInvalidShift);
      }
   }
   else
   {  /* double clocking */
      if (pCfg->nDrive == IFX_TAPI_PCM_IF_DRIVE_HALF)
      {
         /* errmsg: PCM interface configuration invalid,
                    Driving Mode for Single clocking only */
         RETURN_DEVSTATUS(VMMC_statusPcmIfCfgInvalidDrvHalf);
      }
   }

#if !defined(SYSTEM_FALCON)
#ifdef VMMC_FEAT_LINUX_PLATFORM_DRIVER
   ret = vmmc_pcm_pin_config(pCfg->nOpMode);
#else
   /* GPIO pins are reserved only once - if DS_GPIO_RESERVED bit isn't set */
   {
      IFX_boolean_t do_gpio_reserve;

      do_gpio_reserve = (pDev->nDevState & DS_GPIO_RESERVED) ?
                        IFX_TRUE : IFX_FALSE;

      ret = VMMC_XRX100_PcmGpioReserve(pCfg->nOpMode, do_gpio_reserve);
   }
#endif
   if (ret != IFX_SUCCESS)
   {
      /* errmsg: PCM interface configuration failed to configure the GPIOs */
      RETURN_DEVSTATUS(VMMC_statusPcmIfCfgGpioFailed);
   }
   pDev->nDevState |= DS_GPIO_RESERVED;
#endif /* !SYSTEM_FALCON */

   TRACE(VMMC, DBG_LEVEL_LOW, ("PCM if cfg\n"));
   TRACE(VMMC, DBG_LEVEL_LOW, ("PCM mode %d\n", pCfg->nOpMode));
   TRACE(VMMC, DBG_LEVEL_LOW, ("PCM clk  %d (double %d)\n",
         pCfg->nDCLFreq, pCfg->nDoubleClk));
   TRACE(VMMC, DBG_LEVEL_LOW, ("PCM slope  tx %s\n",
        (pCfg->nSlopeTX==IFX_TAPI_PCM_IF_SLOPE_RISE)?"rising":"falling"));
   TRACE(VMMC, DBG_LEVEL_LOW, ("PCM slope  rx %s\n",
        (pCfg->nSlopeRX==IFX_TAPI_PCM_IF_SLOPE_RISE)?"rising":"falling"));
   TRACE(VMMC, DBG_LEVEL_LOW, ("PCM offset tx %d\n", pCfg->nOffsetTX));
   TRACE(VMMC, DBG_LEVEL_LOW, ("PCM offset rx %d\n", pCfg->nOffsetRX));
   TRACE(VMMC, DBG_LEVEL_LOW, ("PCM MCTS 0x%04x\n", pCfg->nMCTS));

   /* Enable the PCM Interface */
   memset (pPcmCtrlCmd, 0, sizeof (PCM_CTRL_t));
   pPcmCtrlCmd->CMD        = CMD_EOP;
   pPcmCtrlCmd->MOD        = MOD_PCM;
   pPcmCtrlCmd->ECMD       = PCM_CTRL_ECMD;

   /* reconfiguration? then switch off first */
   if (pDev->nDevState & DS_PCM_EN)
   {
      /* Disable the PCM Interface */
      pPcmCtrlCmd->EN = IFX_DISABLE;
      ret = CmdWrite (pDev, (IFX_uint32_t*) pPcmCtrlCmd, PCM_CTRL_LEN);
      if (!(VMMC_SUCCESS (ret)))
         return ret;
   }

   pPcmCtrlCmd->EN         = IFX_ENABLE;
   pPcmCtrlCmd->SM = (pCfg->nOpMode == IFX_TAPI_PCM_IF_MODE_MASTER) ? 0 : 1;

   /* limit to 8 bits and mask reserved bit*/
   pPcmCtrlCmd->MCTS       = pCfg->nMCTS & 0xEF;

   /* activate clock tracking for all slave modes or
      master modes with MCTS != 0 (ignoring the LSB (PCM_FDu)) */
   pPcmCtrlCmd->CT         = (pPcmCtrlCmd->SM ||
                   ((!pPcmCtrlCmd->SM) && (pPcmCtrlCmd->MCTS & 0xFE))) ? 1:0;
   pPcmCtrlCmd->DBLCLK     = pCfg->nDoubleClk;
   /* attention coding of rising and falling edge differs for rx/tx slope! */
   pPcmCtrlCmd->X_SLOPE    = (pCfg->nSlopeTX == IFX_TAPI_PCM_IF_SLOPE_RISE) ?
                              0 : 1;
   pPcmCtrlCmd->R_SLOPE    = (pCfg->nSlopeRX == IFX_TAPI_PCM_IF_SLOPE_RISE) ?
                              1 : 0;
   pPcmCtrlCmd->PCMXO      = pCfg->nOffsetTX;
   pPcmCtrlCmd->PCMRO      = pCfg->nOffsetRX;
   pPcmCtrlCmd->DRIVE_0    = pCfg->nDrive;
   pPcmCtrlCmd->SHIFT      = pCfg->nShift;
   pPcmCtrlCmd->DS         = 1;

   /* 1024 and 16384 kHz are not supported by Danube */
   if (!((pCfg->nDCLFreq == IFX_TAPI_PCM_IF_DCLFREQ_1024) ||
         (pCfg->nDCLFreq == IFX_TAPI_PCM_IF_DCLFREQ_16384)))
   {
      pPcmCtrlCmd->DCLFREQ = pCfg->nDCLFreq;
   }
   else
   {
      pPcmCtrlCmd->DCLFREQ = IFX_TAPI_PCM_IF_DCLFREQ_2048;
   }
   /* update the maximum number of timeslots */
   switch (pCfg->nDCLFreq)
   {
      case IFX_TAPI_PCM_IF_DCLFREQ_512:
         pDev->nMaxTimeslot = 8;
         break;
      case IFX_TAPI_PCM_IF_DCLFREQ_1536:
         pDev->nMaxTimeslot = 24;
         break;
      default:
      case IFX_TAPI_PCM_IF_DCLFREQ_2048:
         pDev->nMaxTimeslot = 32;
         break;
      case IFX_TAPI_PCM_IF_DCLFREQ_4096:
         pDev->nMaxTimeslot = 64;
         break;
      case IFX_TAPI_PCM_IF_DCLFREQ_8192:
         pDev->nMaxTimeslot = 128;
         break;
   }
   /* double clocking offers only half the timeslots */
   if (pPcmCtrlCmd->DBLCLK)
   {
      pDev->nMaxTimeslot /= 2;
   }

   ret = CmdWrite (pDev, (IFX_uint32_t *)pPcmCtrlCmd, PCM_CTRL_LEN);

   pDev->nDevState |= DS_PCM_EN;

   ifx_mps_pcm_if_war();

   return ret;
}


/**
   Stop the PCM interface

   \param  pDev         Pointer to the device structure.

   \return
   - VMMC_statusOk         If successful
   - VMMC_statusCmdWr      Writing the command has failed
*/
IFX_int32_t VMMC_PCM_IF_Stop (VMMC_DEVICE *pDev)
{
   PCM_CTRL_t   PcmCtrlCmd;
   PCM_CTRL_t  *pPcmCtrlCmd = &PcmCtrlCmd;
   IFX_uint8_t  i;
   IFX_int32_t  ret = VMMC_statusOk;

   /* calling function should ensure valid parameters */
   VMMC_ASSERT(pDev != IFX_NULL);

   /* Gather the status of all PCM channels, if any is active we cannot
      stop the PCM interface. */
   for (i = 0; i < VMMC_MAX_CH_NR; i++)
   {
      if ((pDev->pChannel[i].pPCM != IFX_NULL) &&
          (pDev->pChannel[i].pPCM->fw_pcm_ch.EN != PCM_CHAN_DISABLE))
      {
         /* errmsg: PCM interface cannot be stopped while any PCM channel
                    is active */
         RETURN_DEVSTATUS(VMMC_statusPcmIfStopWhileActive);
      }
   }

   if (pDev->nDevState & DS_PCM_EN)
   {
      /* Disable the PCM Interface */
      memset (pPcmCtrlCmd, 0, sizeof (PCM_CTRL_t));
      pPcmCtrlCmd->CMD  = CMD_EOP;
      pPcmCtrlCmd->MOD  = MOD_PCM;
      pPcmCtrlCmd->ECMD = PCM_CTRL_ECMD;
      pPcmCtrlCmd->EN   = IFX_DISABLE;
      ret = CmdWrite (pDev, (IFX_uint32_t*) pPcmCtrlCmd, PCM_CTRL_LEN);
   }

   RETURN_DEVSTATUS(ret);
}


/**
   Initalize the PCM module and the cached firmware messages

   \param  pCh             Pointer to the VMMC channel structure.
   \param  pcmCh           The PCM channel resource to use.
*/
IFX_void_t VMMC_PCM_InitCh (VMMC_CHANNEL *pCh, IFX_uint8_t pcmCh)
{
   VMMC_PCMCH_t        *pPCM        = pCh->pPCM;
   PCM_CHAN_t          *pPcmCh      = &pPCM->fw_pcm_ch;
   PCM_SCHAN_t         *pPcmSCh     = &pPCM->fw_pcm_sch;
   IFX_uint8_t         ch = pCh->nChannel - 1;

   /* PCM ch message */
   memset (pPcmCh, 0, sizeof (PCM_CHAN_t));
   pPcmCh->CMD         = CMD_EOP;
   pPcmCh->CHAN        = pcmCh;
   pPcmCh->MOD         = MOD_PCM;
   pPcmCh->ECMD        = PCM_CHAN_ECMD;
   pPcmCh->EN          = PCM_CHAN_DISABLE;
   pPcmCh->COD         = PCM_CHAN_COD_G711_ALAW;
   pPcmCh->CODNR       = ch;
   pPcmCh->GAIN1       = VMMC_GAIN_0DB;
   pPcmCh->GAIN2       = VMMC_GAIN_0DB;
   pPcmCh->I1          = ECMD_IX_EMPTY;
   pPcmCh->I2          = ECMD_IX_EMPTY;
   pPcmCh->I3          = ECMD_IX_EMPTY;
   pPcmCh->I4          = ECMD_IX_EMPTY;
   pPcmCh->I5          = ECMD_IX_EMPTY;
   pPcmCh->HP          = PCM_CHAN_HP_ON;

   /* PCM Shortcut Command */
   memset (pPcmSCh, 0, sizeof (PCM_SCHAN_t));
   pPcmSCh->CMD         = CMD_EOP;
   pPcmSCh->CHAN        = pcmCh;
   pPcmSCh->MOD         = MOD_PCM;
   pPcmSCh->ECMD        = PCM_SCHAN_ECMD;
   pPcmSCh->EN          = PCM_SCHAN_DISABLE;

   /* set static defaults for PCM i/f */
   ifx_tapi_pcm_if_cfg_defaults.nOpMode    = IFX_TAPI_PCM_IF_MODE_SLAVE;
   ifx_tapi_pcm_if_cfg_defaults.nDCLFreq   = IFX_TAPI_PCM_IF_DCLFREQ_2048;
   ifx_tapi_pcm_if_cfg_defaults.nDoubleClk = IFX_DISABLE;
   ifx_tapi_pcm_if_cfg_defaults.nSlopeTX   = IFX_TAPI_PCM_IF_SLOPE_RISE;
   ifx_tapi_pcm_if_cfg_defaults.nSlopeRX   = IFX_TAPI_PCM_IF_SLOPE_FALL;
   ifx_tapi_pcm_if_cfg_defaults.nOffsetTX  = IFX_TAPI_PCM_IF_OFFSET_NONE;
   ifx_tapi_pcm_if_cfg_defaults.nOffsetRX  = IFX_TAPI_PCM_IF_OFFSET_NONE;
   ifx_tapi_pcm_if_cfg_defaults.nDrive     = IFX_TAPI_PCM_IF_DRIVE_ENTIRE;
   ifx_tapi_pcm_if_cfg_defaults.nShift     = IFX_DISABLE;
   ifx_tapi_pcm_if_cfg_defaults.nMCTS      = 0x0000;

   /* DCL Freq (2048) / 64 = 32 Timeslots */
   pCh->pParent->nMaxTimeslot = 32;

   /* echo suppressor resource id */
   pPCM->nEsResId = VMMC_RES_ID_NULL;

   /* line echo canceller resource id */
   pPCM->nLecResId = VMMC_RES_ID_NULL;

   /* initialise the structures used for connecting modules */
   VMMC_CON_Init_PcmCh (pCh, pcmCh);
}


/**
   Set the signal inputs of the cached fw message for the given channel

   \param  pCh             Pointer to the VMMC channel structure.

   \return
   - VMMC_statusOk if ok
   - VMMC_statusCmdWr If writing command failed
*/
IFX_int32_t VMMC_PCM_Set_Inputs (VMMC_CHANNEL *pCh)
{
   PCM_CHAN_t        *p_fw_pcm_ch;
   IFX_int32_t ret = VMMC_statusOk;

   /* update the signal inputs of this cached msg */
   p_fw_pcm_ch = &pCh->pPCM->fw_pcm_ch;

   VMMC_OS_MutexGet (&pCh->chAcc);
   p_fw_pcm_ch->I1 = VMMC_CON_Get_PCM_SignalInput (pCh, 0);
   p_fw_pcm_ch->I2 = VMMC_CON_Get_PCM_SignalInput (pCh, 1);
   p_fw_pcm_ch->I3 = VMMC_CON_Get_PCM_SignalInput (pCh, 2);
   p_fw_pcm_ch->I4 = VMMC_CON_Get_PCM_SignalInput (pCh, 3);
   p_fw_pcm_ch->I5 = VMMC_CON_Get_PCM_SignalInput (pCh, 4);

   /* Write the updated cached message to fw only if channel is running */
   if (p_fw_pcm_ch->EN)
   {
      ret = CmdWrite (pCh->pParent, (IFX_uint32_t *)p_fw_pcm_ch,
                      PCM_CHAN_LEN);
   }
   VMMC_OS_MutexRelease (&pCh->chAcc);

   return ret;
}


/**
   Stop PCM on this channel

   \param  pCh             Pointer to the VMMC channel structure.

   \return
   - VMMC_statusOk         If successful
   - VMMC_statusCmdWr      Writing the command has failed
*/
IFX_int32_t VMMC_PCM_ChStop (VMMC_CHANNEL *pCh)
{
   PCM_CHAN_t       *p_fw_pcm_ch;
   VMMC_DEVICE      *pDev  = pCh->pParent;
   IFX_int32_t       ret   = VMMC_statusOk;

   /* calling function should ensure valid parameters */
   VMMC_ASSERT(pCh != IFX_NULL);

   if (pCh->pPCM != IFX_NULL)
   {
      /* protect fw msg */
      VMMC_OS_MutexGet (&pCh->chAcc);

      /* before the module can be deactivated both LEC and ES must be
         deactivated */

      /* LEC deactivation is always needed when LEC is enabled */
      /* Coding ensures that LEC is enabled when the resource id is valid. */
      if (VMMC_RES_ID_VALID (pCh->pPCM->nLecResId))
      {
         ret = VMMC_RES_LEC_Enable (pCh->pPCM->nLecResId, IFX_DISABLE);
      }

      /* Echo Suppressor deactivation is mandatory when enabled */
      /* Coding ensures that ES is enabled when the resource id is valid. */
      if ((ret == VMMC_statusOk) && VMMC_RES_ID_VALID (pCh->pPCM->nEsResId))
      {
         ret = VMMC_RES_ES_Enable (pCh->pPCM->nEsResId, IFX_DISABLE);
      }

      /* get pointer to cached fw message */
      p_fw_pcm_ch = &pCh->pPCM->fw_pcm_ch;

      /* PCM module deactivation is needed if the module is enabled */
      if ((ret == VMMC_statusOk) && (p_fw_pcm_ch->EN != PCM_CHAN_DISABLE))
      {
         /* deactivate the PCM module */
         p_fw_pcm_ch->EN = PCM_CHAN_DISABLE;
         /* write PCM channel command */
         ret = CmdWrite(pDev, (IFX_uint32_t*)p_fw_pcm_ch,
                        sizeof(PCM_CHAN_t)- CMD_HDR_CNT);
      }

      VMMC_OS_MutexRelease (&pCh->chAcc);
   }

   RETURN_STATUS(ret);
}


/**
   Allocate data structure of the PCM module in the given channel.

   \param  pCh             Pointer to the VMMC channel structure.

   \return
      - VMMC_statusOk if ok
      - VMMC_statusNoMem Memory allocation failed -> struct not created

   \remarks The channel parameter is no longer checked because the calling
   function assures correct values.
*/
IFX_int32_t VMMC_PCM_Allocate_Ch_Structures (VMMC_CHANNEL *pCh)
{
   VMMC_PCM_Free_Ch_Structures (pCh);

   pCh->pPCM = VMMC_OS_Malloc (sizeof(VMMC_PCMCH_t));
   if (pCh->pPCM == NULL)
   {
      /* errmsg: Memory allocation failed */
      RETURN_STATUS (VMMC_statusNoMem);
   }
   memset(pCh->pPCM, 0, sizeof(VMMC_PCMCH_t));

   return VMMC_statusOk;
}


/**
   Free data structure of the PCM module in the given channel.

   \param  pCh             Pointer to the VMMC channel structure.
*/
IFX_void_t VMMC_PCM_Free_Ch_Structures (VMMC_CHANNEL *pCh)
{
   if (pCh->pPCM != IFX_NULL)
   {
      VMMC_OS_Free (pCh->pPCM);
      pCh->pPCM = IFX_NULL;
   }
}


/**
   Configure or check PCM module for given sampling mode.

   \param  pCh             Pointer to the VMMC channel structure.
   \param  action          Action to be executed (set or check).
   \param  sigarray_mode   Signalling array operation mode (16kHz or 8 kHz).
   \param  module_mode     Indicates whether the module is set fixed to NB mode
                           or not.

   \return
   If action is SM_SET: IFX_SUCCESS or IFX_ERROR.
   If action is SM_CHECK: IFX_TRUE when module would do a switch or IFX_FALSE
                          if nothing needs to be done.

*/
IFX_int32_t  VMMC_PCM_SamplingMode (VMMC_CHANNEL *pCh,
                                    SM_ACTION action,
                                    OPMODE_SMPL sigarray_mode,
                                    OPMODE_SMPL module_mode)
{
   VMMC_DEVICE       *pDev = pCh->pParent;
   PCM_CHAN_t        *p_fw_pcm_ch;
   IFX_int32_t       ret = IFX_SUCCESS;
   IFX_uint32_t      new_ISR, new_UD;

   /* calling function should ensure valid parameters */
   VMMC_ASSERT(pCh != NULL);

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
   p_fw_pcm_ch = &pCh->pPCM->fw_pcm_ch;

   /* check if the PCM module already operates in requested mode */
   if( (p_fw_pcm_ch->ISR != new_ISR) ||
       (p_fw_pcm_ch->UD != new_UD) )
   {
      /* If action is execute do changes otherwise report need for execution. */
      if (action == SM_SET)
      {
         /* change the ISR bit of the PCM module */
         /* before ISR bit can be changed the PCM module must be deactivated */
         /* before the module can be deactivated both LEC and ES must be
            deactivated */

         /* LEC deactivation is always needed when LEC is enabled */
         /* Coding ensures that LEC is enabled when the resource id is valid. */
         if (VMMC_RES_ID_VALID (pCh->pPCM->nLecResId))
         {
            ret = VMMC_RES_LEC_Enable (pCh->pPCM->nLecResId, IFX_DISABLE);
         }
         /* Echo Suppressor deactivation is mandatory when enabled */
         /* Coding ensures that ES is enabled when the resource id is valid. */
         if ((ret == VMMC_statusOk) && VMMC_RES_ID_VALID (pCh->pPCM->nEsResId))
         {
            ret = VMMC_RES_ES_Enable (pCh->pPCM->nEsResId, IFX_DISABLE);
         }
         /* PCM module deactivation is needed if the module is enabled */
         if ((p_fw_pcm_ch->EN != PCM_CHAN_DISABLE) && (ret == IFX_SUCCESS))
         {
            /* deactivate the PCM module */
            p_fw_pcm_ch->EN = PCM_CHAN_DISABLE;
            /* write PCM channel command */
            ret = CmdWrite(pDev, (IFX_uint32_t*)p_fw_pcm_ch,
                           sizeof(PCM_CHAN_t)- CMD_HDR_CNT);
            /* state when entering here was enabled so restore the setting */
            p_fw_pcm_ch->EN = PCM_CHAN_ENABLE;
         }

         /* Here the PCM module is turned off - change settings now */

         if (ret == VMMC_statusOk)
         {
            /* Set the ISR bit in the cached message to the new value */
            p_fw_pcm_ch->ISR = new_ISR;
            /* Set the UD bit in the cached message to the new value */
            p_fw_pcm_ch->UD = new_UD;

            TRACE(VMMC, DBG_LEVEL_LOW,
                  ("Set PCM channel %u ISR = %d UD = %d\n",
                   pCh->nChannel - 1, p_fw_pcm_ch->ISR, p_fw_pcm_ch->UD));
         }

         /* write cached PCM channel command to the chip if enabled */
         if ((p_fw_pcm_ch->EN != PCM_CHAN_DISABLE)  && (ret == IFX_SUCCESS))
         {
            /* write PCM channel command */
            ret = CmdWrite(pDev, (IFX_uint32_t*)p_fw_pcm_ch,
                           sizeof(PCM_CHAN_t)- CMD_HDR_CNT);
         }
         /* enable the ES if it was running before */
         if ((ret == VMMC_statusOk) && VMMC_RES_ID_VALID (pCh->pPCM->nEsResId))
         {
            ret = VMMC_RES_ES_Enable (pCh->pPCM->nEsResId, IFX_ENABLE);
         }
         /* enable the LEC if it was running before */
         if ((ret == VMMC_statusOk) && VMMC_RES_ID_VALID (pCh->pPCM->nLecResId))
         {
            /* set the LEC to the current sampling rate (mode) */
            VMMC_RES_LEC_SamplingModeSet (pCh->pPCM->nLecResId, module_mode);
            ret = VMMC_RES_LEC_Enable (pCh->pPCM->nLecResId, IFX_ENABLE);

#ifdef VMMC_FEAT_RTCP_XR
            /* Set association between LEC and COD. */
            if (VMMC_SUCCESS(ret))
            {
               ret = VMMC_RES_LEC_AssociatedCodSet(
                        pCh->pPCM->nLecResId,
                        VMMC_CON_SingleDataChannelCodFind(pCh, VMMCDSP_MT_PCM));
            }
#endif /* VMMC_FEAT_RTCP_XR */
         }
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
               ("Sampling rate of PCM on channel %u already matching\n",
               pCh->nChannel - 1));

#ifdef VMMC_FEAT_RTCP_XR
         /* If there is a LEC set association between LEC and COD. */
         if (VMMC_RES_ID_VALID (pCh->pPCM->nLecResId))
         {
            ret = VMMC_RES_LEC_AssociatedCodSet(
                     pCh->pPCM->nLecResId,
                     VMMC_CON_SingleDataChannelCodFind(pCh, VMMCDSP_MT_PCM));
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

   RETURN_STATUS (ret);
}


/**
   Function that fills in the PCM module function pointers in the driver
   context structure which is passed to HL TAPI during registration.

   \param  pPCM         Pointer to the PCM part in the driver context struct.
*/
IFX_void_t VMMC_PCM_Func_Register (IFX_TAPI_DRV_CTX_PCM_t *pPCM)
{
   pPCM->ifCfg          = VMMC_TAPI_LL_PCM_IF_Cfg;
   pPCM->Cfg            = VMMC_TAPI_LL_PCM_CH_Cfg;
   pPCM->Enable         = VMMC_TAPI_LL_PCM_CH_Enable;
   pPCM->Lec_Cfg        = VMMC_TAPI_LL_PCM_LEC_Set;
#ifdef VMMC_FEAT_HDLC
   pPCM->HDLC_Cfg        = VMMC_TAPI_LL_PCM_HDLC_Cfg;
#endif /* VMMC_FEAT_HDLC */
   pPCM->Loop           = VMMC_TAPI_LL_PCM_LOOP_Cfg;
   pPCM->Volume_Set     = VMMC_TAPI_LL_PCM_Volume_Set;
   pPCM->DEC_HP_Set     = VMMC_TAPI_LL_PCM_DEC_HP_Set;
}

#endif /* VMMC_FEAT_PCM */
