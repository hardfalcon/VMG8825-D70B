/******************************************************************************

  Copyright (c) 2012 Lantiq Deutschland GmbH
  Copyright 2016, Intel Corporation.

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

*******************************************************************************
   Module      : drv_vmmc_dect.c
   Description : This file implements the DECT module
******************************************************************************/

#ifdef HAVE_CONFIG_H
#include <drv_config.h>
#endif

#ifdef DECT_SUPPORT

/** \file drv_vmmc_dect.c
    VMMC DECT module.
    This module provides the DECT channel control functionality.
    \remarks DECT is available only in special FW versions. */

/** \defgroup DECT_IMPLEMENTATION DECT Implementation
    Implementation of the services provided by the DECT channels. */
/* @{ */

/* ============================= */
/* Includes                      */
/* ============================= */
#include "drv_api.h"
#include "drv_vmmc_api.h"
#include "drv_vmmc_dect_priv.h"
#include "drv_vmmc_dect.h"
#include "drv_vmmc_sig.h"
#include "drv_vmmc_con.h"


/* ============================= */
/* Local Macros & Definitions    */
/* ============================= */

/* Translation table for coder values   TAPI enum -> FW value */
static IFX_uint8_t TranslateCoderTable[] =
{
   COD_CHAN_SPEECH_ENC_NO,             /*  0: No coder */
   DECT_CHAN_SPEECH_ENC_G711_ALAW,     /*  1: G.711 A-Law 64 kbit/s */
   DECT_CHAN_SPEECH_ENC_G711_MLAW,     /*  2: G.711 u-Law 64 kbit/s */
   DECT_CHAN_SPEECH_ENC_G726_32,       /*  3: G.726 32 kbit/s */
   DECT_CHAN_SPEECH_ENC_G722_64        /*  4: G.722 64 kbit/s (wideband) */
};

/* Translation table for frame length values   TAPI enum -> FW value */
static IFX_uint8_t TranslateFrameLengthTable[] =
{
   DECT_ENC_PTE_2_5MS,                 /*  1: 2.5 ms packetization length. */
   DECT_ENC_PTE_5MS,                   /*  2: 5 ms packetization length.   */
   DECT_ENC_PTE_10MS                   /*  3: 10 ms packetization length.  */
};


/* ============================= */
/* Local function declaration    */
/* ============================= */

static IFX_uint8_t vmmc_dect_trans_enc_tapi2fw (IFX_TAPI_DECT_ENC_TYPE_t nCoder);
static IFX_uint8_t vmmc_dect_trans_fl_tapi2fw (IFX_TAPI_DECT_ENC_LENGTH_t nFL);
static IFX_boolean_t vmmc_dect_codec_supported (VMMC_DEVICE *pDev,
                                                IFX_uint8_t nCodec);
static IFX_int32_t vmmc_dect_UTG_Start (VMMC_CHANNEL *pLLChannel,
                                     IFX_TAPI_TONE_SIMPLE_t const *pSimpleTone);
static IFX_int32_t vmmc_dect_UTG_Stop (VMMC_CHANNEL *pCh);

static IFX_int32_t VMMC_TAPI_LL_DECT_CH_Cfg (IFX_TAPI_LL_CH_t *pLLChannel,
                                             IFX_uint8_t nEncDelay,
                                             IFX_uint8_t nDecDelay);

static IFX_int32_t VMMC_TAPI_LL_DECT_CH_ENC_Set (IFX_TAPI_LL_CH_t *pLLChannel,
                                       IFX_TAPI_DECT_ENC_TYPE_t nCoder,
                                       IFX_TAPI_DECT_ENC_LENGTH_t nFrameLength);

static IFX_int32_t VMMC_TAPI_LL_DECT_CH_Enable (IFX_TAPI_LL_CH_t *pLLChannel,
                                                IFX_uint8_t nEnable);

static IFX_int32_t VMMC_TAPI_LL_DECT_EC_Cfg (IFX_TAPI_LL_CH_t *pLLChannel,
                                              IFX_TAPI_DECT_EC_CFG_t const *pEC_Cfg);

static IFX_int32_t VMMC_TAPI_LL_DECT_CH_Gain_Set (IFX_TAPI_LL_CH_t *pLLChannel,
                                             IFX_TAPI_PKT_VOLUME_t const *pVol);

static IFX_int32_t VMMC_TAPI_LL_DECT_CH_Statistic (IFX_TAPI_LL_CH_t *pLLCh,
                                       IFX_TAPI_DECT_STATISTICS_t *pStatistic);

static IFX_int32_t VMMC_TAPI_LL_DECT_UTG_Start (IFX_TAPI_LL_CH_t *pLLChannel,
                                     IFX_TAPI_TONE_SIMPLE_t const *pSimpleTone,
                                     TAPI_TONE_DST dst,
                                     IFX_uint8_t res);

static IFX_int32_t VMMC_TAPI_LL_DECT_UTG_Stop (IFX_TAPI_LL_CH_t *pLLChannel,
                                               IFX_uint8_t res);



/* ============================= */
/* Function definitions          */
/* ============================= */

/**
   Function to translate coder value from TAPI enum to FW encoding.

   \param nCoder    Coder as TAPI enum value
   \return
      ENC value or COD_CHAN_SPEECH_ENC_NO
*/
static IFX_uint8_t vmmc_dect_trans_enc_tapi2fw (IFX_TAPI_DECT_ENC_TYPE_t nCoder)
{
   /* range check for safety */
   /* sizeof is sufficient because data type in array is uint8 */
   if( (size_t)nCoder >= sizeof(TranslateCoderTable) )
   {
      TRACE(VMMC, DBG_LEVEL_HIGH,
            ("ERROR: Parameter coder value %d out of range [0 to %d]\n",
             nCoder, sizeof(TranslateCoderTable)));
      return COD_CHAN_SPEECH_ENC_NO;
   }

   return TranslateCoderTable[nCoder];
}


/**
   Function to translate frame length value from TAPI enum to FW encoding.

   \param nFL    Frame length as TAPI enum value
   \return
      Frame length value or COD_CHAN_SPEECH_PTE_NO
*/
static IFX_uint8_t vmmc_dect_trans_fl_tapi2fw (IFX_TAPI_DECT_ENC_LENGTH_t nFL)
{
   /* range check for safety */
   /* sizeof is sufficient because data type in array is uint8 */
   if( (size_t)nFL > sizeof(TranslateFrameLengthTable) || (size_t)nFL == 0)
   {
      TRACE(VMMC, DBG_LEVEL_HIGH,
            ("ERROR: Parameter framelength value %d out of range [1 to %d]\n",
             nFL, sizeof(TranslateFrameLengthTable)));
      return COD_CHAN_SPEECH_PTE_NO;
   }

   /* -1 because the array starts with 0 but first enum value is 1 */
   return TranslateFrameLengthTable[nFL - 1];
}


/**
   Function that checks in the capability list that the FW reports supports the
   requested codec.

   \param pDev          Pointer to VMMC device data.
   \param nCodec        Encoder identifier.
   \return
      - \ref IFX_TRUE  if encoder is supported
      - \ref IFX_FALSE if encoder is not supported
*/
static IFX_boolean_t vmmc_dect_codec_supported (VMMC_DEVICE *pDev,
                                                IFX_uint8_t nCodec)
{
   IFX_uint8_t nMask = 0x00;

   switch (nCodec)
   {
      case COD_CHAN_SPEECH_ENC_NO:
         /* no coder: doing nothing is never a problem */
         return IFX_TRUE;
      case DECT_CHAN_SPEECH_ENC_G711_MLAW:
         /* fallthrough */
      case DECT_CHAN_SPEECH_ENC_G711_ALAW:
         nMask = 0x04;
         break;
      case DECT_CHAN_SPEECH_ENC_G726_32:
         nMask = 0x01;
         break;
      case DECT_CHAN_SPEECH_ENC_G722_64:
         nMask = 0x02;
         break;
      default:
         /* unknown coder - this should not happen */
         break;
   }

   if (pDev->caps.DECT_CODECS & nMask)
      return IFX_TRUE;

   return IFX_FALSE;
}


/**
   Do low level UTG (Universal Tone Generator) configuration and activation.

   \param pLLChannel    Pointer to the VMMC channel structure.
   \param pSimpleTone   Internal simple tone table entry.
   \return Return value according to \ref IFX_return_t
      - \ref IFX_ERROR if an error occured
      - \ref IFX_SUCCESS if successful
   \remarks
      It is assumed that the UTG is not active before calling this function
      which programs the UTG coefficients.
*/
static
IFX_int32_t vmmc_dect_UTG_Start (VMMC_CHANNEL *pLLChannel,
                                 IFX_TAPI_TONE_SIMPLE_t const *pSimpleTone)
{
   IFX_int32_t          ret = VMMC_statusOk;
   VMMC_CHANNEL        *pCh  = (VMMC_CHANNEL *)pLLChannel;
   VMMC_DEVICE         *pDev = pCh->pParent;
   DECT_UTG_CTRL_t     *pUtgCmd;
   RES_UTG_COEF_t      *pUtgCoefCmd;

   /* calling function ensures valid parameter */
   VMMC_ASSERT(pSimpleTone != IFX_NULL);

   /* setup the UTG coefficients */
   pUtgCoefCmd = &pCh->pDECT->fw_dect_utg_coef;
   VMMC_SIG_UTG_SetCoeff (pSimpleTone, pUtgCoefCmd);

   ret = CmdWrite (pDev, (IFX_uint32_t *)pUtgCoefCmd,
                   sizeof(*pUtgCoefCmd) - CMD_HDR_CNT);

   if (!VMMC_SUCCESS (ret))
   {
      /* Report a failure writing the UTG coefficients */
      RETURN_STATUS (ret);
   }

   /* pointer to cached UTG control message */
   pUtgCmd = &pCh->pDECT->fw_dect_utg_ctrl;

   /* activate the UTG */
   pUtgCmd->SM = SIG_UTG_CTRL_SM_STOP;
   pUtgCmd->EN = SIG_UTG_CTRL_ENABLE;

   ret = CmdWrite (pDev, (IFX_uint32_t *)pUtgCmd,
                   sizeof(*pUtgCmd) - CMD_HDR_CNT);

   if (VMMC_SUCCESS (ret))
   {
      if (pSimpleTone->loop > 0 || pSimpleTone->pause > 0)
      {
         /* auto stop after loop or after each generation step */
         pUtgCmd->SM = SIG_UTG_CTRL_SM_CONTINUE;
         pUtgCmd->EN = SIG_UTG_CTRL_DISABLE;

         ret = CmdWrite (pDev, (IFX_uint32_t *)pUtgCmd,
                         sizeof(*pUtgCmd) - CMD_HDR_CNT);
      }
   }

   RETURN_STATUS (ret);
}


/**
   Stops the UTG

   \param pCh           Pointer to the VMMC channel structure.
   \return Return value according to \ref IFX_return_t
      - \ref IFX_ERROR if an error occured
      - \ref IFX_SUCCESS if successful
*/
static
IFX_int32_t vmmc_dect_UTG_Stop (VMMC_CHANNEL *pCh)
{
   IFX_int32_t          ret = VMMC_statusOk;
   VMMC_DEVICE         *pDev = pCh->pParent;
   DECT_UTG_CTRL_t     *pUtgCmd;

   /* pointer to cached UTG control message */
   pUtgCmd = &pCh->pDECT->fw_dect_utg_ctrl;

   if (pUtgCmd->EN == SIG_UTG_CTRL_ENABLE ||
       pUtgCmd->SM == SIG_UTG_CTRL_SM_CONTINUE)
   {
      pUtgCmd->EN = SIG_UTG_CTRL_DISABLE;
      pUtgCmd->SM = SIG_UTG_CTRL_SM_STOP;

      ret = CmdWrite (pDev, (IFX_uint32_t *)pUtgCmd,
                      sizeof(*pUtgCmd) - CMD_HDR_CNT);
   }

   RETURN_STATUS (ret);
}



/* ============================= */
/* Global function definition    */
/* ============================= */


/**
   Sets the encoder and decoder start delay.

   \param pLLChannel    Pointer to the VMMC channel structure.
   \param nEncDelay     Delay from the start of the decoder to the start of
                        the encoder in steps of 2.5ms. Range 0ms - 10ms.
   \param nDecDelay     Delay from the arrival of the first packet to the start
                        of the decoder in steps of 2.5ms. Range 0ms - 10ms.
   \return
   - VMMC_statusOk         If successful
   - VMMC_statusInvalCh    No DECT resource on the addressed channel.
   - VMMC_statusDectEncDelayOutOfRange    DECT encoder delay out of range.
   - VMMC_statusDectDecDelayOutOfRange    DECT decoder delay out of range.
   - VMMC_statusDectCfgWhileActive        DECT codec delays cannot be set
                                          while channel is active.
   \remarks Configuration may only be done when channel is deactivated.
*/
IFX_int32_t VMMC_TAPI_LL_DECT_CH_Cfg (IFX_TAPI_LL_CH_t *pLLChannel,
                                      IFX_uint8_t nEncDelay,
                                      IFX_uint8_t nDecDelay)
{
   VMMC_CHANNEL        *pCh = (VMMC_CHANNEL *) pLLChannel;
   DECT_CHAN_SPEECH_t  *p_fw_dect_ch_speech;

   /* Work only on channels where DECT is initialised */
   if (pCh->pDECT == IFX_NULL)
   {
      /* errmsg: Resource not valid. Channel number out of range  */
      RETURN_STATUS (VMMC_statusInvalCh);
   }
   p_fw_dect_ch_speech = &pCh->pDECT->fw_dect_ch_speech;

   /* Check for invalid parameter values */
   if (nEncDelay > 4)
   {
      /* errmsg: DECT encoder delay out of range */
      RETURN_STATUS (VMMC_statusDectEncDelayOutOfRange);
   }
   if (nDecDelay > 4)
   {
      /* errmsg: DECT decoder delay out of range */
      RETURN_STATUS (VMMC_statusDectDecDelayOutOfRange);
   }

   /* Configuration may only be done when channel is deactivated */
   if (p_fw_dect_ch_speech->EN != COD_CHAN_SPEECH_DISABLE)
   {
      /* errmsg: DECT codec delays cannot be set while channel is active */
      RETURN_STATUS (VMMC_statusDectCfgWhileActive);
   }

   /* protect fwmsg against concurrent tasks */
   VMMC_OS_MutexGet (&pCh->chAcc);

   p_fw_dect_ch_speech->EncDelay = nEncDelay;
   p_fw_dect_ch_speech->DecDelay = nDecDelay;

   /* Writing of the FW message is not needed because of the precondition that
      the channel must be in the stopped state. */

   /* unlock */
   VMMC_OS_MutexRelease (&pCh->chAcc);

   return VMMC_statusOk;
}


/**
   Sets the encoder type and frame length for the encoding path.

   \param pLLChannel    Pointer to the VMMC channel structure.
   \param nCoder        Encoder packet type to be set.
   \param nFrameLength  Length of packets to be generated by the coder.
   \return
   - VMMC_statusOk         If successful
   - VMMC_statusInvalCh    No DECT resource on the addressed channel.

   \remarks The frame length cannot be changed while the channel is running.
*/
IFX_int32_t VMMC_TAPI_LL_DECT_CH_ENC_Set (IFX_TAPI_LL_CH_t *pLLChannel,
                                          IFX_TAPI_DECT_ENC_TYPE_t nCoder,
                                          IFX_TAPI_DECT_ENC_LENGTH_t nFrameLength)
{
   VMMC_CHANNEL        *pCh = (VMMC_CHANNEL *) pLLChannel;
   VMMC_DEVICE         *pDev = pCh->pParent;
   DECT_CHAN_SPEECH_t  *p_fw_dect_ch_speech;
   IFX_boolean_t        bUpdate = IFX_FALSE,
                        bSRchange = IFX_FALSE;
   IFX_int32_t          ret = VMMC_statusOk;
   IFX_uint8_t          nCod, nPte;

   /* Work only on channels where DECT is initialised */
   if (pCh->pDECT == IFX_NULL)
   {
      /* errmsg: Resource not valid. Channel number out of range  */
      RETURN_STATUS (VMMC_statusInvalCh);
   }
   p_fw_dect_ch_speech = &pCh->pDECT->fw_dect_ch_speech;

   /* Translate parameters from TAPI to FW values */
   nCod = vmmc_dect_trans_enc_tapi2fw (nCoder);
   nPte = vmmc_dect_trans_fl_tapi2fw(nFrameLength);

   /* Check for invalid parameter values. */
   if (nPte == COD_CHAN_SPEECH_PTE_NO)
   {
      /* errmsg: DECT invalid frame length parameter */
      RETURN_STATUS (VMMC_statusDectInvalidFrameLength);
   }
   if (nCod == COD_CHAN_SPEECH_ENC_NO)
   {
      /* errmsg: DECT invalid coder parameter */
      RETURN_STATUS (VMMC_statusDectInvalidCoder);
   }

   /* Check if the FW supports the selected coder. */
   if (vmmc_dect_codec_supported (pDev, nCod) != IFX_TRUE)
   {
      /* errmsg: DECT selected coder not supported by firmware */
      RETURN_STATUS (VMMC_statusDectCoderNotInFw);
   }

   /* All parameter combinations are supported no special checks needed */

   VMMC_OS_MutexGet (&pCh->chAcc);

   if (p_fw_dect_ch_speech->ENC != nCod)
   {
      /* set flag if the mode changes between NB and WB */
      if ((p_fw_dect_ch_speech->ENC == IFX_TAPI_DECT_ENC_TYPE_G722_64) ||
          (nCod == IFX_TAPI_DECT_ENC_TYPE_G722_64))
            {
               DECT_UTG_CTRL_t *pUtgCmd = &pCh->pDECT->fw_dect_utg_ctrl;

               if (pUtgCmd->EN == SIG_UTG_CTRL_ENABLE ||
                   pUtgCmd->SM == SIG_UTG_CTRL_SM_CONTINUE)
               {
                  VMMC_OS_MutexRelease (&pCh->chAcc);
                  RETURN_STATUS(VMMC_statusDectCodSetFailedUtg);
               }

               bSRchange = IFX_TRUE;
            }
      /* store the coder value in the cached fw-message */
      p_fw_dect_ch_speech->ENC = nCod;
      bUpdate = IFX_TRUE;
   }

   if (p_fw_dect_ch_speech->PTE != nPte)
   {
      if (p_fw_dect_ch_speech->EN != COD_CHAN_SPEECH_DISABLE)
      {
         /* changing the PTE is not allowed while the channel is running */
         /* do not change the pte value in the cached fw-message */
         /* if only the frame length was changed return error but if
            also the coder is changed the return is determined by the
            result of CmdWrite() below. */
         /* errmsg: DECT frame length cannot be changed while the channel is
                    active */
         ret = VMMC_statusDectFrameLengthWhileActive;
      }
      else
      {
         /* store the pte value in the cached fw-message */
         p_fw_dect_ch_speech->PTE = nPte;
         bUpdate = IFX_TRUE;
      }
   }

   if ((bUpdate != IFX_FALSE) &&
       (p_fw_dect_ch_speech->EN != COD_CHAN_SPEECH_DISABLE))
   {
      /* channel is enabled */

      /* if the sampling mode changed also an update of the sampling
         mode in the attached conference is needed */
      if (bSRchange != IFX_FALSE)
      {
         /* first disable the DECT channel */
         p_fw_dect_ch_speech->EN = COD_CHAN_SPEECH_DISABLE;
         ret = CmdWrite (pDev, (IFX_uint32_t *)p_fw_dect_ch_speech,
                         sizeof (DECT_CHAN_SPEECH_t) - CMD_HDR_CNT);
         VMMC_OS_MutexRelease (&pCh->chAcc);

         /* set module to NB or WB depending on the encoder */
         if (p_fw_dect_ch_speech->ENC == IFX_TAPI_DECT_ENC_TYPE_G722_64)
         {
            VMMC_CON_ModuleSamplingModeSet (pCh, VMMCDSP_MT_DECT,
                                            VMMC_CON_SMPL_WB);
         }
         else
         {
            VMMC_CON_ModuleSamplingModeSet (pCh, VMMCDSP_MT_DECT,
                                            VMMC_CON_SMPL_NB);
         }
         /* reevaluate the conference that this module belongs to */
         VMMC_CON_MatchConfSmplRate(pCh, VMMCDSP_MT_DECT);

         VMMC_OS_MutexGet (&pCh->chAcc);
         /* finally enable the DECT channel again */
         p_fw_dect_ch_speech->EN = COD_CHAN_SPEECH_ENABLE;
         ret = CmdWrite (pDev, (IFX_uint32_t *)p_fw_dect_ch_speech,
                         sizeof (DECT_CHAN_SPEECH_t) - CMD_HDR_CNT);
      }
      else
      {
         /* write the updated msg with the new encoder to the device */
         ret = CmdWrite (pDev, (IFX_uint32_t *)p_fw_dect_ch_speech,
                         sizeof (DECT_CHAN_SPEECH_t) - CMD_HDR_CNT);
      }
   }

   VMMC_OS_MutexRelease (&pCh->chAcc);

   RETURN_STATUS (ret);
}


/**
   Enables or Disables the DECT channel speech compression.

   \param pLLChannel    Pointer to the VMMC channel structure.
   \param nEnable       0: off, !0: on.
   \return
   - VMMC_statusOk         If successful
   - VMMC_statusInvalCh    No DECT resource on the addressed channel.
   - VMMC_statusDectStopFailedEcActive Cannot stop channel while EC is active.
   \remarks
      We use the global enable flag for turning the DECT channel on or off.
      The decoder is always configured to run and the encoder can be configured
      with the codec selected. With NO_ENC the encoder will not start.
*/
IFX_int32_t VMMC_TAPI_LL_DECT_CH_Enable (IFX_TAPI_LL_CH_t *pLLChannel,
                                         IFX_uint8_t nEnable)
{
   VMMC_CHANNEL        *pCh = (VMMC_CHANNEL *) pLLChannel;
   VMMC_DEVICE         *pDev = pCh->pParent;
   DECT_CHAN_SPEECH_t  *p_fw_dect_ch_speech;
   IFX_int32_t          ret = VMMC_statusOk;

   /* Work only on channels where DECT is initialised */
   if (pCh->pDECT == IFX_NULL)
   {
      /* errmsg: Resource not valid. Channel number out of range  */
      RETURN_STATUS (VMMC_statusInvalCh);
   }
   p_fw_dect_ch_speech = &pCh->pDECT->fw_dect_ch_speech;

   /* switch on */
   if ((nEnable) && (p_fw_dect_ch_speech->EN == COD_CHAN_SPEECH_DISABLE))
   {
      /* Before activation set module to NB or WB depending on the encoder */
      if (p_fw_dect_ch_speech->ENC == IFX_TAPI_DECT_ENC_TYPE_G722_64)
      {
         VMMC_CON_ModuleSamplingModeSet (pCh, VMMCDSP_MT_DECT,
                                         VMMC_CON_SMPL_WB);
      }
      else
      {
         VMMC_CON_ModuleSamplingModeSet (pCh, VMMCDSP_MT_DECT,
                                         VMMC_CON_SMPL_NB);
      }
      /* reevaluate the conference that this module belongs to */
      VMMC_CON_MatchConfSmplRate(pCh, VMMCDSP_MT_DECT);

      /* Enable the DECT channel (protected against concurrent access) */
      VMMC_OS_MutexGet (&pCh->chAcc);
      p_fw_dect_ch_speech->EN = COD_CHAN_SPEECH_ENABLE;
      ret = CmdWrite (pDev, (IFX_uint32_t *)p_fw_dect_ch_speech,
                      sizeof (DECT_CHAN_SPEECH_t) - CMD_HDR_CNT);
      VMMC_OS_MutexRelease (&pCh->chAcc);
   }
   else
   /* switch off */
   if ((!nEnable) && (p_fw_dect_ch_speech->EN != COD_CHAN_SPEECH_DISABLE))
   {
      /* check if DECT echo canceller is running */
      if (VMMC_RES_ID_VALID(pCh->pDECT->nEsResId))
      {
         RETURN_STATUS (VMMC_statusDectStopFailedEcActive);
      }
      /* Disable the DECT channel (protected against concurrent access) */
      VMMC_OS_MutexGet (&pCh->chAcc);
         /* stop UTG first, if running */
         ret = vmmc_dect_UTG_Stop(pCh);
         if (VMMC_SUCCESS(ret))
         {
            p_fw_dect_ch_speech->EN = COD_CHAN_SPEECH_DISABLE;
            ret = CmdWrite (pDev, (IFX_uint32_t *)p_fw_dect_ch_speech,
                      sizeof (DECT_CHAN_SPEECH_t) - CMD_HDR_CNT);
         }
      VMMC_OS_MutexRelease (&pCh->chAcc);

      /* After deactivation set the sampling mode to off */
      VMMC_CON_ModuleSamplingModeSet (pCh, VMMCDSP_MT_DECT, VMMC_CON_SMPL_OFF);
      /* reevaluate the conference that this module belongs to */
      VMMC_CON_MatchConfSmplRate(pCh, VMMCDSP_MT_DECT);
   }

   RETURN_STATUS (ret);
}


/**
   Enables or Disables the DECT channel echo canceller.

   \param pLLChannel    Pointer to the VMMC channel structure.
   \param pEC_Cfg       Pointer to IFX_TAPI_DECT_EC_CFG_t structure
   \return
   - VMMC_statusOk         If successful
   - VMMC_statusInvalCh    No DECT resource on the addressed channel.
   - VMMC_statusEsNotSupported Echo suppressor is not supported by FW.
   - VMMC_statusNoRes      Allocating an ES resource failed.
   - VMMC_statusDectEcStartFailedChNotActive  Cannot start DECT
                           Echo Canceller when DECT channel is not active.
*/
IFX_int32_t VMMC_TAPI_LL_DECT_EC_Cfg (IFX_TAPI_LL_CH_t *pLLChannel,
                                      IFX_TAPI_DECT_EC_CFG_t const *pEC_Cfg)
{
   VMMC_CHANNEL        *pCh = (VMMC_CHANNEL *) pLLChannel;
   VMMC_DEVICE         *pDev;
   IFX_int32_t          ret = VMMC_statusOk;

   /* sanity check */
   VMMC_ASSERT(pCh != IFX_NULL);
   if (pCh->pDECT == IFX_NULL)
   {
      /* errmsg: Resource not valid. Channel number out of range */
      RETURN_STATUS (VMMC_statusInvalCh);
   }

   pDev = pCh->pParent;
   if (!pDev->caps.bESonDECT)
   {
      /* errmsg: Echo suppressor is not supported by FW */
      RETURN_STATUS (VMMC_statusEsNotSupported);
   }

   if (pEC_Cfg->nType == IFX_TAPI_EC_TYPE_OFF)
   {
      /* Disable the echo suppressor. */

      if (VMMC_RES_ID_VALID(pCh->pDECT->nEsResId))
      {
         /* Disable the ES, free resource and forget the resource ID. */
         ret = VMMC_RES_ES_Enable (pCh->pDECT->nEsResId, IFX_DISABLE);
         if (VMMC_SUCCESS (ret))
         {
            ret = VMMC_RES_ES_Release (pCh->pDECT->nEsResId);
         }
         if (VMMC_SUCCESS (ret))
         {
            pCh->pDECT->nEsResId = VMMC_RES_ID_NULL;
         }
      }
   }
   else if (pEC_Cfg->nType == IFX_TAPI_EC_TYPE_ES)
   {
      /* Enable the echo suppressor. */

      DECT_CHAN_SPEECH_t  *p_fw_dect_ch_speech = &pCh->pDECT->fw_dect_ch_speech;

      if (p_fw_dect_ch_speech->EN == COD_CHAN_SPEECH_DISABLE)
      {
         /* errmsg: Cannot start DECT Echo Canceller
             when DECT channel is not active */
         RETURN_STATUS (VMMC_statusDectEcStartFailedChNotActive);
      }

      if (!VMMC_RES_ID_VALID(pCh->pDECT->nEsResId))
      {
         /* allocate an ES resource */
         pCh->pDECT->nEsResId = VMMC_RES_ES_Allocate (pCh, VMMC_RES_MOD_DECT);
      }

      if (!VMMC_RES_ID_VALID(pCh->pDECT->nEsResId))
      {
         RETURN_STATUS (VMMC_statusNoRes);
      }

      /* no LEC+NLP on DECT channel */
      ret = VMMC_RES_ES_ParameterSelect (pCh->pDECT->nEsResId, IFX_DISABLE);

      if (VMMC_SUCCESS (ret))
      {
         ret = VMMC_RES_ES_Enable (pCh->pDECT->nEsResId, IFX_ENABLE);
      }
   }
   else
   {
      /* errmsg: wrong parameter */
      ret = VMMC_statusParam;
   }

   RETURN_STATUS (ret);
}


/**
   Sets the DECT channel gains.

   Gain Parameter are given in 'dB'. The range is -24dB ... +12dB.

   \param pLLChannel    Pointer to the VMMC channel structure.
   \param pVol          Pointer to IFX_TAPI_LINE_VOLUME_t structure.
   \return
   - VMMC_statusCmdWr Writing the command failed
   - VMMC_statusFuncParm Wrong parameters passed. This code is returned
     when any gain parameter is lower than -24 dB or higher than +12 dB
   - VMMC_statusInvalCh if called on a channel where there is no DECT ressource
   - VMMC_statusOk if successful
*/
IFX_int32_t VMMC_TAPI_LL_DECT_CH_Gain_Set (IFX_TAPI_LL_CH_t *pLLChannel,
                                           IFX_TAPI_PKT_VOLUME_t const *pVol)
{
   VMMC_CHANNEL        *pCh = (VMMC_CHANNEL *) pLLChannel;
   VMMC_DEVICE         *pDev = pCh->pParent;
   DECT_CHAN_SPEECH_t  *p_fw_dect_ch_speech;
   IFX_int32_t          ret = VMMC_statusOk;

   /* Work only on channels where DECT is initialised */
   if (pCh->pDECT == IFX_NULL)
   {
      /* errmsg: Resource not valid. Channel number out of range  */
      RETURN_STATUS (VMMC_statusInvalCh);
   }

   /* range check */
   if ((pVol->nEnc < VMMC_VOLUME_GAIN_MIN) ||
       (pVol->nEnc > VMMC_VOLUME_GAIN_MAX) ||
       (pVol->nDec < VMMC_VOLUME_GAIN_MIN) ||
       (pVol->nDec > VMMC_VOLUME_GAIN_MAX))
   {
      /* errmsg: DECT gain out of supported range */
      RETURN_STATUS (VMMC_statusDectGainOutOfRange);
   }

   /* protect fw msg */
   VMMC_OS_MutexGet (&pCh->chAcc);

   p_fw_dect_ch_speech = &pCh->pDECT->fw_dect_ch_speech;

   /* store actual settings into message cache */
   p_fw_dect_ch_speech->GAIN1 =
      VMMC_Gaintable[pVol->nDec - VMMC_VOLUME_GAIN_MIN];
   p_fw_dect_ch_speech->GAIN2 =
      VMMC_Gaintable[pVol->nEnc - VMMC_VOLUME_GAIN_MIN];

   /* if channel is enabled write to fw */
   if (p_fw_dect_ch_speech->EN == COD_CHAN_SPEECH_ENABLE)
   {
      /* channel is enabled, write the updated msg to the device */
      ret = CmdWrite (pDev, (IFX_uint32_t *)p_fw_dect_ch_speech,
                      sizeof (DECT_CHAN_SPEECH_t) - CMD_HDR_CNT);
   }

   /* release lock */
   VMMC_OS_MutexRelease (&pCh->chAcc);

   RETURN_STATUS (ret);
}


/**
   Get the statistic data from the DECT coder channel.

   \param pLLChannel    Pointer to the VMMC channel structure.
   \param pStatistic    Pointer to struct where to store the statistic data.

   \return
   - VMMC_statusOk         If successful
   - VMMC_statusInvalCh    No DECT resource on the addressed channel.
*/
IFX_int32_t VMMC_TAPI_LL_DECT_CH_Statistic (IFX_TAPI_LL_CH_t *pLLChannel,
                                         IFX_TAPI_DECT_STATISTICS_t *pStatistic)
{
   VMMC_CHANNEL        *pCh = (VMMC_CHANNEL *) pLLChannel;
   VMMC_DEVICE         *pDev = pCh->pParent;
   DECT_CODER_STAT_t   *p_fw_dect_coder_stat;
   IFX_int32_t          ret = VMMC_statusOk;

   /* Work only on channels where DECT is initialised */
   if (pCh->pDECT == IFX_NULL)
   {
      /* errmsg: Resource not valid. Channel number out of range  */
      RETURN_STATUS (VMMC_statusInvalCh);
   }

   /* protect fwmsg against concurrent tasks */
   VMMC_OS_MutexGet (&pCh->chAcc);

   p_fw_dect_coder_stat = &pCh->pDECT->fw_dect_coder_stat;

   ret = CmdRead (pDev, (IFX_uint32_t *)p_fw_dect_coder_stat,
                        (IFX_uint32_t *)p_fw_dect_coder_stat,
                        sizeof(DECT_CODER_STAT_t)-CMD_HDR_CNT);

   if (VMMC_SUCCESS (ret))
   {
      /* copy the data from fw-msg into the ioctl parameters */
      pStatistic->nPktUp      = p_fw_dect_coder_stat->H_FP_PKTS_CNT;
      pStatistic->nPktDown    = p_fw_dect_coder_stat->FP_H_PKTS_CNT;
      pStatistic->nSid        = p_fw_dect_coder_stat->FP_H_SID_PKTS_CNT;
      pStatistic->nPlc        = p_fw_dect_coder_stat->FP_H_PLC_PKTS_CNT;
      pStatistic->nOverflows  = p_fw_dect_coder_stat->FP_H_OVFL_CNT;
      pStatistic->nUnderflows = p_fw_dect_coder_stat->FP_H_UNFL_CNT;
      pStatistic->nInvalid    = p_fw_dect_coder_stat->FP_H_INVA_PKTS_CNT;
   }

   if (VMMC_SUCCESS (ret) &&
       (pStatistic->nReset != 0))
   {
      /* reset the statistic counters */
      ret = CmdWrite (pDev, (IFX_uint32_t *)p_fw_dect_coder_stat, 0);
   }

   /* unlock */
   VMMC_OS_MutexRelease (&pCh->chAcc);

   RETURN_STATUS (ret);
}


/**
   Do low level UTG (Universal Tone Generator) configuration and activation.

   This function handles all necessary steps to play out a full simple tone
   on the UTG. It returns immediately.

   If the resource number is not 0 this function returns an error code of
   VMMC_statusDectUtgBadRes.
   First the coefficients are programmed, before the UTG is be activated.
   The UTG must be inactive when programming the coefficients. It is assumed
   that the UTG is not active before calling this function which programs
   the UTG coefficients.

   \param pLLChannel    Pointer to the VMMC channel structure.
   \param pSimpleTone   Pointer to the tone definition to play. May not be NULL.
   \param res           Resource number which is used for playing the tone.
   \param dst           Destination where to play the tone: local or network.
                        Unused.

   \return
      - VMMC_statusInvalCh    No DECT resource on the addressed channel.
      - VMMC_statusDectUtgFailedChNotActive  Cannot start DECT UTG when
           DECT channel is not active.
      - VMMC_statusDectUtgBadRes  DECT UTG resource parameter invalid.
      - VMMC_statusDectUtgBadTone  DECT UTG simple-tone parameter invalid.
      - VMMC_statusCmdWr Writing the command failed
      - VMMC_statusNoRes No free resources to play the tone. The reason
           is that res is larger then the number of available tone resources.
      - VMMC_statusFuncParm Wrong parameter passed. This code is returned
           when the given cadence is invalid, for example the first cadence step
           is zero. This is an internal check, because the tone configuration
           function verifies the correct cadence.
      - VMMC_statusOk if successful
*/
IFX_int32_t VMMC_TAPI_LL_DECT_UTG_Start (IFX_TAPI_LL_CH_t *pLLChannel,
                                        IFX_TAPI_TONE_SIMPLE_t const *pSimpleTone,
                                        TAPI_TONE_DST dst,
                                        IFX_uint8_t res)
{
   VMMC_CHANNEL *pCh = (VMMC_CHANNEL *) pLLChannel;
   IFX_int32_t ret = VMMC_statusInvalCh;

   /* Work only on channels where DECT is initialised */
   if (pCh->pDECT != IFX_NULL)
   {
      /* abort if the DECT channel is not active */
      if (pCh->pDECT->fw_dect_ch_speech.EN == COD_CHAN_SPEECH_DISABLE)
      {
         /* errmsg: Cannot start DECT UTG when DECT channel is not active */
         RETURN_STATUS (VMMC_statusDectUtgFailedChNotActive);
      }
      /* check validity of the parameters */
      if (res != 0)
      {
         /* errmsg: DECT UTG resource parameter invalid */
         RETURN_STATUS (VMMC_statusDectUtgBadRes);
      }
      if (pSimpleTone == IFX_NULL)
      {
         /* errmsg: DECT UTG tone parameter invalid */
         RETURN_STATUS (VMMC_statusDectUtgBadTone);
      }
      /* parameter dst is not used for DECT */
      dst = dst;

      TAPI_Tone_Set_Source (pCh->pTapiCh, res, IFX_TAPI_TONE_RESSEQ_SIMPLE);

      /* Activate the universal tone generator */
      VMMC_OS_MutexGet (&pCh->chAcc);
      ret = vmmc_dect_UTG_Start(pCh, pSimpleTone);
      VMMC_OS_MutexRelease (&pCh->chAcc);
   }

   RETURN_STATUS (ret);
}


/**
   Stop playing a tone immediately.

   \param pLLChannel    Pointer to the VMMC channel structure.
   \param res           Resource number which is used for playing the tone.
   \return
      - VMMC_statusInvalCh    No DECT resource on the addressed channel.
      - VMMC_statusCmdWr Writing the command failed
      - VMMC_statusNoRes No free resources specified in res. The reason
         is that res is larger then the number of available tone resources.
      - VMMC_statusOk if successful
   \remarks
      This function does nothing when the tone playing resource is already
      deactivated.
*/
IFX_int32_t  VMMC_TAPI_LL_DECT_UTG_Stop (IFX_TAPI_LL_CH_t *pLLChannel,
                                        IFX_uint8_t res)
{
   VMMC_CHANNEL *pCh = (VMMC_CHANNEL *) pLLChannel;
   IFX_int32_t ret = VMMC_statusInvalCh;

   /* Work only on channels where DECT is initialised */
   if (pCh->pDECT != IFX_NULL)
   {
      /* check for valid resource number */
      if (res != 0)
      {
         /* errmsg: DECT UTG resource parameter invalid */
         RETURN_STATUS (VMMC_statusDectUtgBadRes);
      }

      /* Deactivate the universal tone generator */
      VMMC_OS_MutexGet (&pCh->chAcc);
      ret = vmmc_dect_UTG_Stop (pCh);
      VMMC_OS_MutexRelease (&pCh->chAcc);
   }

   RETURN_STATUS (ret);
}


/**
   Initalize the coder module and the cached firmware messages.

   \param pCh           Pointer to the VMMC channel structure.
   \return
      - VMMC_statusCmdWr Writing the command failed
      - VMMC_statusOk if successful
   \remarks
      To allocate memory call \ref VMMC_DECT_Allocate_Ch_Structures() first.
*/
IFX_return_t VMMC_DECT_InitCh (VMMC_CHANNEL *pCh)
{
   VMMC_DEVICE         *pDev = pCh->pParent;
   VMMC_DECTCH_t       *pDECT = pCh->pDECT;
   IFX_uint8_t         ch = pCh->nChannel - 1;

   /* Initialise the CON structure corresponding to this channel */
   VMMC_CON_Init_DectCh (pCh);

   /* Command: DECT channel speech compression  */
   memset(&pDECT->fw_dect_ch_speech, 0, sizeof(pDECT->fw_dect_ch_speech));
   /* command header */
   pDECT->fw_dect_ch_speech.CMD    = CMD_DECT;
   pDECT->fw_dect_ch_speech.CHAN   = ch;
   pDECT->fw_dect_ch_speech.MOD    = MOD_CODER;
   pDECT->fw_dect_ch_speech.ECMD   = COD_CHAN_SPEECH_ECMD;
   /* command data */
   pDECT->fw_dect_ch_speech.EN     = 0;
   pDECT->fw_dect_ch_speech.ISR    = 0;
   pDECT->fw_dect_ch_speech.NS     = 0;
   pDECT->fw_dect_ch_speech.PTE    = DECT_ENC_PTE_10MS;
   pDECT->fw_dect_ch_speech.HP     = 1;
   pDECT->fw_dect_ch_speech.CNG    = 0;
   pDECT->fw_dect_ch_speech.BFI    = 1;
   pDECT->fw_dect_ch_speech.DEC    = 1;
   pDECT->fw_dect_ch_speech.PST    = 0;
   pDECT->fw_dect_ch_speech.SIC    = 0;
   pDECT->fw_dect_ch_speech.ADD    = DECT_CHAN_SPEECH_ADD_SIGNAL_UTG_ON;
   pDECT->fw_dect_ch_speech.ENC    = DECT_CHAN_SPEECH_ENC_G726_32;
   pDECT->fw_dect_ch_speech.GAIN1  = VMMC_GAIN_0DB;
   pDECT->fw_dect_ch_speech.GAIN2  = VMMC_GAIN_0DB;
#ifdef VMMC_DECT_NIBBLE_SWAP
   /* activate DECT nibble swap for COSIC modem (default is 1) */
   pDECT->fw_dect_ch_speech.NSWAP  = VMMC_DECT_NIBBLE_SWAP;
#else
   #warning VMMC_DECT_NIBBLE_SWAP not defined, using default (1)
   pDECT->fw_dect_ch_speech.NSWAP  = 1;
#endif /* VMMC_DECT_NIBBLE_SWAP */

   /* Command: DECT coder channel statistics */
   memset(&pDECT->fw_dect_coder_stat, 0, sizeof(pDECT->fw_dect_coder_stat));
   /* command header */
   pDECT->fw_dect_coder_stat.CMD   = CMD_DECT;
   pDECT->fw_dect_coder_stat.CHAN  = ch;
   pDECT->fw_dect_coder_stat.MOD   = MOD_CODER;
   pDECT->fw_dect_coder_stat.ECMD  = DECT_CODER_STAT_ECMD;

   /* Command: universal tone generator coefficients */
   memset(&pDECT->fw_dect_utg_coef, 0, sizeof(pDECT->fw_dect_utg_coef));
   pDECT->fw_dect_utg_coef.CMD     = CMD_EOP;
   pDECT->fw_dect_utg_coef.CHAN    = ch + (pDev->caps.nSIG *
                                           pDev->caps.nUtgPerCh);
   pDECT->fw_dect_utg_coef.MOD     = MOD_RESOURCE;
   pDECT->fw_dect_utg_coef.ECMD    = RES_UTG_COEF_ECMD;

   /* Command: DECT channel universal tone generator control */
   memset(&pDECT->fw_dect_utg_ctrl, 0, sizeof(pDECT->fw_dect_utg_ctrl));
   pDECT->fw_dect_utg_ctrl.CMD     = CMD_DECT;
   pDECT->fw_dect_utg_ctrl.CHAN    = ch;
   pDECT->fw_dect_utg_ctrl.MOD     = MOD_CODER;
   pDECT->fw_dect_utg_ctrl.ECMD    = DECT_UTG_CTRL_ECMD;
   pDECT->fw_dect_utg_ctrl.UTGNR   = ch + (pDev->caps.nSIG *
                                           pDev->caps.nUtgPerCh);
   if (pDev->caps.bEventMailboxSupported)
   {
      pDECT->fw_dect_utg_ctrl.EU   = 1;
   }

   return IFX_SUCCESS;
}


/**
   Set the signal inputs of the cached fw message for the given channel.

   \param pCh           Pointer to the VMMC channel structure.
   \return  IFX_SUCCESS or IFX_ERROR
   \remarks Writing to FW is only done if the adressed channel is running.
*/
IFX_return_t VMMC_DECT_Set_Inputs (VMMC_CHANNEL *pCh)
{
   DECT_CHAN_SPEECH_t  *p_fw_dect_ch_speech;
   int                  ret = IFX_SUCCESS;

   /* update the signal inputs of this cached msg */
   p_fw_dect_ch_speech = &pCh->pDECT->fw_dect_ch_speech;

   VMMC_OS_MutexGet (&pCh->chAcc);
   p_fw_dect_ch_speech->I1 = VMMC_CON_Get_DECT_SignalInput (pCh, 0);
   p_fw_dect_ch_speech->I2 = VMMC_CON_Get_DECT_SignalInput (pCh, 1);
   p_fw_dect_ch_speech->I3 = VMMC_CON_Get_DECT_SignalInput (pCh, 2);
   p_fw_dect_ch_speech->I4 = VMMC_CON_Get_DECT_SignalInput (pCh, 3);
   p_fw_dect_ch_speech->I5 = VMMC_CON_Get_DECT_SignalInput (pCh, 4);

   /* Write the updated cached message to fw only if channel is running */
   if (p_fw_dect_ch_speech->EN)
   {
      ret = CmdWrite (pCh->pParent, (IFX_uint32_t *)p_fw_dect_ch_speech,
                      sizeof(DECT_CHAN_SPEECH_t) - CMD_HDR_CNT);
   }
   VMMC_OS_MutexRelease (&pCh->chAcc);

   return (VMMC_statusOk == ret) ? IFX_SUCCESS : IFX_ERROR;
}


/**
   Stop DECT on this channel

   \param  pCh             Pointer to the VMMC channel structure.

   \return
   - VMMC_statusOk         If successful
   - VMMC_statusCmdWr      Writing the command has failed
*/
IFX_int32_t VMMC_DECT_ChStop (VMMC_CHANNEL *pCh)
{
   DECT_CHAN_SPEECH_t  *p_fw_dect_ch_speech;
   VMMC_DEVICE      *pDev  = pCh->pParent;
   IFX_int32_t       ret   = VMMC_statusOk;

   /* calling function should ensure valid parameters */
   VMMC_ASSERT(pCh != IFX_NULL);

   if (pCh->pDECT != IFX_NULL)
   {
      if (VMMC_RES_ID_VALID(pCh->pDECT->nEsResId))
      {
         /* Disable the ES, free resource and forget the resource ID. */
         ret = VMMC_RES_ES_Enable (pCh->pDECT->nEsResId, IFX_DISABLE);
         if (VMMC_SUCCESS (ret))
         {
            ret = VMMC_RES_ES_Release (pCh->pDECT->nEsResId);
         }
         if (VMMC_SUCCESS (ret))
         {
            pCh->pDECT->nEsResId = VMMC_RES_ID_NULL;
         }
      }

      /* protect fw msg */
      VMMC_OS_MutexGet (&pCh->chAcc);

      /* get pointer to cached fw message */
      p_fw_dect_ch_speech = &pCh->pDECT->fw_dect_ch_speech;

      /* DECT module deactivation is done if the module is enabled */
      if (p_fw_dect_ch_speech->EN != COD_CHAN_SPEECH_DISABLE)
      {
         /* deactivate the DECT module */
         p_fw_dect_ch_speech->EN = COD_CHAN_SPEECH_DISABLE;
         /* write DECT channel command */
         ret = CmdWrite(pDev, (IFX_uint32_t*)p_fw_dect_ch_speech ,
                        sizeof(DECT_CHAN_SPEECH_t)- CMD_HDR_CNT);
      }

      VMMC_OS_MutexRelease (&pCh->chAcc);
   }

   RETURN_STATUS(ret);
}


/**
   Allocate data structure of the DECT module in the given channel.

   \param  pCh             Pointer to the VMMC channel structure.

   \return
      - VMMC_statusOk if ok
      - VMMC_statusNoMem Memory allocation failed -> struct not created

   \remarks The channel parameter is no longer checked because the calling
   function assures correct values.
*/
IFX_int32_t VMMC_DECT_Allocate_Ch_Structures (VMMC_CHANNEL *pCh)
{
   VMMC_DECT_Free_Ch_Structures (pCh);

   pCh->pDECT = VMMC_OS_Malloc (sizeof(VMMC_DECTCH_t));
   if (pCh->pDECT == NULL)
   {
      /* errmsg: Memory allocation failed */
      RETURN_STATUS (VMMC_statusNoMem);
   }
   memset(pCh->pDECT, 0, sizeof(VMMC_DECTCH_t));

   return VMMC_statusOk;
}


/**
   Free data structure of the DECT module in the given channel.

   \param  pCh             Pointer to the VMMC channel structure.
*/
IFX_void_t VMMC_DECT_Free_Ch_Structures (VMMC_CHANNEL *pCh)
{
   if (pCh->pDECT != IFX_NULL)
   {
      VMMC_OS_Free (pCh->pDECT);
      pCh->pDECT = IFX_NULL;
   }
}


/**
   Configure or check DECT module for given sampling mode.

   \param  pCh             Pointer to the VMMC channel structure.
   \param  action          Action to be executed (set or check).
   \param  mode            Signalling array operation mode (16kHz or 8 kHz).

   \return
   If action is SM_SET: IFX_SUCCESS or IFX_ERROR.
   If action is SM_CHECK: IFX_TRUE when module would do a switch or IFX_FALSE
                          if nothing needs to be done.
*/
IFX_int32_t  VMMC_DECT_SamplingMode (VMMC_CHANNEL *pCh,
                                     SM_ACTION action,
                                     OPMODE_SMPL mode)
{
   VMMC_DEVICE         *pDev = pCh->pParent;
   DECT_CHAN_SPEECH_t  *p_fw_dect_ch_speech;
   IFX_int32_t         ret = IFX_SUCCESS;

   /* calling function should ensure valid parameters */
   VMMC_ASSERT(pCh != NULL);

   /* protect fw msg */
   VMMC_OS_MutexGet (&pCh->chAcc);

   /* get pointer to cached fw message */
   p_fw_dect_ch_speech = &pCh->pDECT->fw_dect_ch_speech;

   /* check if the ALM module already operates in requested mode */
   if( ((mode == WB_16_KHZ) && (p_fw_dect_ch_speech->ISR == 0)) ||
       ((mode == NB_8_KHZ)  && (p_fw_dect_ch_speech->ISR == 1)) )
   {
      /* If action is execute do changes otherwise report need for execution. */
      if (action == SM_SET)
      {
         /* change the DECT channel's ISR bit */
         p_fw_dect_ch_speech->ISR = !(p_fw_dect_ch_speech->ISR);
         TRACE(VMMC, DBG_LEVEL_LOW,
               ("Set DECT channel %u ISR = %d\n",
               pCh->nChannel - 1, p_fw_dect_ch_speech->ISR));

         /* write only to FW if DECT is enabled */
         if (p_fw_dect_ch_speech->EN)
         {
            ret = CmdWrite(pDev, (IFX_uint32_t*)p_fw_dect_ch_speech ,
                           sizeof(DECT_CHAN_SPEECH_t)- CMD_HDR_CNT);
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
              ("Sampling rate of DECT on channel %u already matching\n",
              pCh->nChannel - 1));
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
   Function that fills in the DECT module function pointers in the driver
   context structure which is passed to HL TAPI during registration.

   \param  pDECT        Pointer to the DECT part in the driver context struct.
*/
IFX_void_t VMMC_DECT_Func_Register (IFX_TAPI_DRV_CTX_DECT_t *pDECT)
{
   pDECT->Ch_Cfg    = VMMC_TAPI_LL_DECT_CH_Cfg;
   pDECT->ENC_Cfg   = VMMC_TAPI_LL_DECT_CH_ENC_Set;
   pDECT->Enable    = VMMC_TAPI_LL_DECT_CH_Enable;
   pDECT->Gain_Set  = VMMC_TAPI_LL_DECT_CH_Gain_Set;
   pDECT->Statistic = VMMC_TAPI_LL_DECT_CH_Statistic;
   pDECT->UTG_Start = VMMC_TAPI_LL_DECT_UTG_Start;
   pDECT->UTG_Stop  = VMMC_TAPI_LL_DECT_UTG_Stop;
   pDECT->EC_Cfg    = VMMC_TAPI_LL_DECT_EC_Cfg;
}

/* @} */

#endif /* DECT_SUPPORT */
