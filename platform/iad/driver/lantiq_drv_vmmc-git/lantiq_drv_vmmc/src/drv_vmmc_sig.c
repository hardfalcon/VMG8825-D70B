/******************************************************************************

                              Copyright (c) 2012
                            Lantiq Deutschland GmbH
                             http://www.lantiq.com

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/*
   \file drv_vmmc_sig.c
   This file contains the implementation of the functions for the SIGnalling
   module.
*/

/* ============================= */
/* Includes                      */
/* ============================= */

#include "drv_api.h"
#include "drv_vmmc_sig_priv.h"
#include "drv_vmmc_sig.h"
#include "drv_vmmc_api.h"
#include "drv_vmmc_sig_cid.h"
#include "drv_vmmc_con.h"
#include "drv_vmmc_cod.h"

/* ============================= */
/* Local Macros & Definitions    */
/* ============================= */
/** Requested gap for tone holding to maximum 7000 ms sec (value / 10)*/
#define TONEHOLDING_RGAP (7000 / 10)
/* Requested gap for CED detection 120 ms sec (value / 10)*/
/* #define CED_RGAP         (120 / 10) */
/* DIS detection Number of Requested Repetition of the Preamble Sequence */
/* #define DIS_REPET        0x8 */
/* DIS detection minimum signal level */
/* #define DIS_LEVEL         0xCF */
/* CED detection minimum phase shift for the phase reversal  */
/* #define CED_PHASE        0x23 */
/* Toneholding detection minimum signal level */
/* #define TONEHOLDING_LEVEL 0x83 */
/* CED detection minimum signal level */
/* #define CED_LEVEL         0xCF */


/* ============================= */
/* Local Variables               */
/* ============================= */


/* ============================= */
/*  Local function declarations  */
/* ============================= */

static IFX_int32_t VMMC_TAPI_LL_SIG_DetectEnable (IFX_TAPI_LL_CH_t *pLLChannel,
                                                  IFX_TAPI_SIG_DETECTION_t const *pSig);

static IFX_int32_t VMMC_TAPI_LL_SIG_DetectDisable (IFX_TAPI_LL_CH_t *pLLChannel,
                                                   IFX_TAPI_SIG_DETECTION_t const *pSig);

/* ============================= */
/* Global function definitions   */
/* ============================= */


/**
   Function that fills in the SIG module function pointers in the driver
   context structure which is passed to HL TAPI during registration.

   \param  pSig         Pointer to the SIG part in the driver context struct.
*/
IFX_void_t VMMC_SIG_Func_Register (IFX_TAPI_DRV_CTX_SIG_t *pSig)
{
   /* Fill the function pointers of the signalling module ll interface */
   pSig->UTG_Start = VMMC_TAPI_LL_SIG_UTG_Start;
   pSig->UTG_Stop = VMMC_TAPI_LL_SIG_UTG_Stop;
   pSig->UTG_Count_Get = VMMC_TAPI_LL_SIG_UTG_Count_Get;
   pSig->UTG_Event_Deactivated = VMMC_TAPI_LL_UTG_Event_Deactivated;

   pSig->DTMFG_Cfg = VMMC_TAPI_LL_SIG_DTMFG_Cfg;
   pSig->DTMFG_Start = VMMC_TAPI_LL_SIG_DTMFG_Start;
   pSig->DTMFG_Stop = VMMC_TAPI_LL_SIG_DTMFG_Stop;

   pSig->DTMFD_Enable = VMMC_TAPI_LL_SIG_DTMFD_Start;
   pSig->DTMFD_Disable = VMMC_TAPI_LL_SIG_DTMFD_Stop;
   pSig->DTMFD_OOB = VMMC_TAPI_LL_SIG_DTMFD_OOB;
   pSig->DTMFD_Override = VMMC_TAPI_LL_SIG_DTMFD_Override;
   pSig->DTMF_RxCoeff = VMMC_TAPI_LL_SIG_DTMF_RX_CFG;

   pSig->CPTD_Start = VMMC_TAPI_LL_SIG_CPTD_Start;
   pSig->CPTD_Stop = VMMC_TAPI_LL_SIG_CPTD_Stop;

   pSig->MFTD_OOB = VMMC_TAPI_LL_SIG_MFTD_OOB;

#ifdef TAPI_CID
   pSig->CID_TX_Start = VMMC_TAPI_LL_SIG_CID_TX_Start;
   pSig->CID_TX_Stop = VMMC_TAPI_LL_SIG_CID_TX_Stop;
   pSig->CID_RX_Start = VMMC_TAPI_LL_SIG_CID_RX_Start;
   pSig->CID_RX_Stop = VMMC_TAPI_LL_SIG_CID_RX_Stop;
#endif /* TAPI_CID */

   pSig->MFTD_Enable = VMMC_TAPI_LL_SIG_DetectEnable;
   pSig->MFTD_Disable = VMMC_TAPI_LL_SIG_DetectDisable;
}


/**
   Enables or disables event transmission of the DTMF receiver and the MFTD.

   Called after COD is enabled to generate RTP event packets or just before the
   RTP packet generation is disabled in order to turn event transmission in
   the tone detectors on or off. The generation of RTP events in the COD
   depends on the encoder and decoder running both at the same time.

   \param  pCh    Pointer to VMMC channel structure.
   \param  bCod   IFX_TRUE if RTP support in COD has just been switched on.
                  IFX_FALSE if RTP support in COD is about to be switched off.
   \return
   VMMC_statusOk if successful, else error code
*/
IFX_int32_t VMMC_SIG_UpdateEventTrans (VMMC_CHANNEL *pCh, IFX_boolean_t bCod)
{
   IFX_uint32_t *pCmd;
   IFX_boolean_t bDoWrite = IFX_FALSE;
   IFX_int32_t ret = VMMC_statusOk;

   /* enable event support in SIG channel */
   if (bCod == IFX_TRUE && pCh->pSIG->fw_sig_ch.ES != 1)
   {
      pCh->pSIG->fw_sig_ch.ES = 1;
      ret = CmdWrite (pCh->pParent,
                      (IFX_uint32_t *) &pCh->pSIG->fw_sig_ch, SIG_CHAN_LEN);
      if (!VMMC_SUCCESS(ret))
      {
         RETURN_STATUS (ret);
      }
   }

   if (bCod)
   {
      if (pCh->pSIG->et_stat.flag.dtmf_rec != pCh->pSIG->fw_sig_dtmfrcv.ET)
      {
         pCh->pSIG->fw_sig_dtmfrcv.ET = pCh->pSIG->et_stat.flag.dtmf_rec;
         bDoWrite = IFX_TRUE;
      }
   }
   else
   {
      pCh->pSIG->fw_sig_dtmfrcv.ET = 0;
      bDoWrite = IFX_TRUE;
   }

   if (bDoWrite == IFX_TRUE)
   {
      pCmd = (IFX_uint32_t *) &pCh->pSIG->fw_sig_dtmfrcv;

      ret = CmdWrite (pCh->pParent, pCmd, SIG_DTMFR_CTRL_LEN);
   }

   if (VMMC_SUCCESS (ret))
   {
      /* release lock set in DEC_Start, DEC_Stop, ENC_start, ENC_stop
         because MFTD_EvtTransSet locks it again */
      VMMC_OS_MutexRelease (&pCh->chAcc);
      if (bCod)
      {
         /* set event transmission according to the stored state */
         ret = VMMC_SIG_MFTD_EvtTransSet (pCh,
                  pCh->pSIG->et_stat.flag.mftd ? IFX_TRUE : IFX_FALSE);
      }
      else
      {
         /* turn of event transmission */
         ret = VMMC_SIG_MFTD_EvtTransSet (pCh, IFX_FALSE);
      }
      VMMC_OS_MutexGet (&pCh->chAcc);
   }

   /* disable event support in SIG channel */
   if (bCod != IFX_TRUE && pCh->pSIG->fw_sig_ch.ES != 0)
   {
      pCh->pSIG->fw_sig_ch.ES = 0;
      ret = CmdWrite (pCh->pParent,
                      (IFX_uint32_t *) &pCh->pSIG->fw_sig_ch, SIG_CHAN_LEN);
   }

   RETURN_STATUS (ret);
}


/**
   Stops the signalling channel if no algorithm is running.

   This function is called to minimize the DSP usage after deactivating
   resources. If no more resource is used in the signalling channel the channel
   is shut down.

   \param  pCh          Pointer to VMMC channel structure.
   \param  bOn Target operation status on or maybe off
            - IFX_TRUE it must be switched on. No check if needed is done
            - IFX_FALSE a resource has been deactivated, thus check if signalling
              channel is still needed.
   \return
       VMMC_statusOk if no error, otherwise error code
 */
IFX_int32_t VMMC_SIG_AutoChStop (VMMC_CHANNEL *pCh, IFX_boolean_t bOn)
{
   IFX_uint32_t *pCmd, nCount;
   IFX_int32_t  ret = VMMC_statusOk;

   if (bOn == IFX_FALSE)
   {
      /* Request to turn off the SIG channel */

      if (pCh->pSIG->fw_sig_dtmfrcv.EN == 0 &&
          pCh->pSIG->fw_sig_dtmfrcv_override.EN == 0 &&
          pCh->pSIG->fw_sig_dtmfgen.EN == 0 &&
          pCh->pSIG->fw_utg[0].EN == 0 &&
          pCh->pSIG->fw_utg[0].SM == 0 &&
          pCh->pSIG->fw_utg[1].EN == 0 &&
          pCh->pSIG->fw_utg[1].SM == 0 &&
          pCh->pSIG->fw_sig_mftd.EN == 0 &&
          pCh->pSIG->fw_sig_cidrcv.EN == 0 &&
          pCh->pSIG->fw_sig_cidsend.EN == 0 &&
          pCh->pSIG->fw_sig_cptd.CPTD[0].EN == 0 &&
          pCh->pSIG->fw_sig_cptd.CPTD[1].EN == 0 &&
          pCh->pSIG->fw_sig_cptd.CPTD[2].EN == 0 &&
          VMMC_COD_ChStatusGet (pCh) == IFX_DISABLE )
      {
         /* SIG is not needed and will be switched off now if it is running */
         if (pCh->pSIG->fw_sig_ch.EN == SIG_CHAN_ENABLE)
         {
            /* switch the signalling channel off */
            pCh->pSIG->fw_sig_ch.EN = SIG_CHAN_DISABLE;

            pCmd = (IFX_uint32_t *) &pCh->pSIG->fw_sig_ch;
            nCount = sizeof (pCh->pSIG->fw_sig_ch) - CMD_HDR_CNT;
            ret = CmdWrite (pCh->pParent, pCmd, nCount);
         }
      }
      else
      {
         /* leave the state activated, but additional check is done */
         if (pCh->pSIG->fw_sig_ch.EN == SIG_CHAN_DISABLE)
         {
            /* This must never happen! Channel must be active when at least
               one generator or detector is activated. */
            /* errmsg: Invalid state */
            RETURN_STATUS (VMMC_statusInvalSigState);
         }
      }
   }
   else
   {
      /* Request to turn on the SIG channel */

      if (pCh->pSIG->fw_sig_ch.EN  == SIG_CHAN_DISABLE)
      {
         /* switch the signalling channel on */
         pCh->pSIG->fw_sig_ch.EN = SIG_CHAN_ENABLE;

         pCmd = (IFX_uint32_t *)  &pCh->pSIG->fw_sig_ch;
         nCount = sizeof (pCh->pSIG->fw_sig_ch) - CMD_HDR_CNT;
         ret = CmdWrite (pCh->pParent, pCmd, nCount);
      }
   }

   RETURN_STATUS (ret);
}


/**
   Basic SIGNALLING Module configuration.

   Use this function where needed to set the base configuration
   of Signalling Module.
   This function configures:
      - SIGNALLING Channels
      - CID, DTMF gen & recv
      - ATDs, UTDs or MFTDs

   \param  pCh          Pointer to the VMMC channel structure.

   \return VMMC_statusOk if no error, otherwise error code.
*/
IFX_int32_t VMMC_SIG_Base_Conf (VMMC_CHANNEL *pCh)
{
   VMMC_DEVICE  *pDev    = pCh->pParent;
   VMMC_SIGCH_t *pSIG    = pCh->pSIG;
   IFX_uint32_t *pCmd    = IFX_NULL;
   IFX_int32_t   ret;
   IFX_uint8_t   ch      = (pCh->nChannel - 1);
   IFX_uint8_t   i;

   /* RTP support configuration ********************************************* */
   /* sig rtp support configuration, enable local playout of all RFC2833 evts */
   /* event playout to local side (A2). To be done before channel activation. */
   pSIG->fw_sig_rtp_sup.A1       = SIG_RTP_SUP_NOTONE;
   pSIG->fw_sig_rtp_sup.A2       = SIG_RTP_SUP_TONEMUTE;
   pSIG->fw_sig_rtp_sup.EVTOG    = 0x07;
   pSIG->fw_sig_rtp_sup.EMT      = 0x7F;
   pSIG->fw_sig_rtp_sup.EVT_PT   = DEFAULT_EVTPT;

   ret = CmdWrite (pDev, (IFX_uint32_t *) &pSIG->fw_sig_rtp_sup,
                   sizeof (pSIG->fw_sig_rtp_sup) - CMD_HDR_CNT);
   if (ret != VMMC_statusOk)
      RETURN_STATUS (ret);
   if (pDev->err == VMMC_ERR_CERR)
      RETURN_STATUS (VMMC_statusCerr);

   /* Activate the signalling channel **************************************** */
   pSIG->fw_sig_ch.EN = SIG_CHAN_ENABLE;
   /* disable event support */
   pSIG->fw_sig_ch.ES = 0;

   /* Set the inputs of the module and write cached message to fw */
   ret = VMMC_SIG_Set_Inputs(pCh);

   if (ret != VMMC_statusOk)
      RETURN_STATUS (ret);

#ifdef TAPI_CID
   /* CID Generator and CID Detector **************************************** */
   ret = VMMC_SIG_CID_BaseConf(pCh);
   if (ret != VMMC_statusOk)
      RETURN_STATUS (ret);
#endif /* TAPI_CID */

   /* DTMF Generator and DTMF Detector ************************************** */
   ret = VMMC_SIG_DTMF_BaseConf(pCh);
   if (ret != VMMC_statusOk)
      RETURN_STATUS (ret);

   /* UTG ******************************************************************* */
   if (ch >= pDev->caps.nUTG)
   {
      /** errmsg: No tone resource available for this channel */
      RETURN_STATUS (VMMC_statusNoToneRes);
   }
   else
   {
      /* configure all UTGs of this channel */
      for (i = 0; (i < pDev->caps.nUtgPerCh) && (i < LL_TAPI_TONE_MAXRES); i++)
      {
         pSIG->fw_utg[i].EN = SIG_UTG_CTRL_DISABLE;
         pSIG->fw_utg[i].A1 = SIG_UTG_CTRL_A1_ON;
         pSIG->fw_utg[i].A2 = SIG_UTG_CTRL_A2_ON;

         pCmd = (IFX_uint32_t *)&pSIG->fw_utg[i];
         ret = CmdWrite (pDev, pCmd, sizeof(SIG_UTG_CTRL_t) - CMD_HDR_CNT);
         if (ret != VMMC_statusOk)
            RETURN_STATUS (ret);
         if (pDev->err == VMMC_ERR_CERR)
            RETURN_STATUS (VMMC_statusCerr);
      }
   }

   /* MFTD ****************************************************************** */
   /*lint -e(774) */
   if ((ret == IFX_SUCCESS) && (pDev->caps.nMFTD > 0 && ch == 0))
   {
      /* hardcode MFTD gap holdtime for transp. fax transmissions to 7 sec,
         all other parameters are FW defaults, only once for ch = 0 (broadcast)
       */
      IFX_uint32_t fw_sig_mftd_ansd_coef [5] = {0};

      fw_sig_mftd_ansd_coef[0] = 0x2600CD07;
      /* LEVELS=-35dB, AM-MOD=0.18, PHASE=150DEG */
      fw_sig_mftd_ansd_coef[1] = 0x01232E6A;
      /* SNR=5dB, AGAP=100ms, RTIME=210ms */
      fw_sig_mftd_ansd_coef[2] = 0x287A0A15;
      /* ABREAK=100ms, RGAP=120ms, LEVELHOLD=-35dB */
      fw_sig_mftd_ansd_coef[3] = 0x0A0C0000;
      /* RGAPHOLD=7000ms */
      fw_sig_mftd_ansd_coef[4] = 0x02460000 | TONEHOLDING_RGAP;
      ret = CmdWrite (pDev, fw_sig_mftd_ansd_coef, 16);

      if (ret != VMMC_statusOk)
         RETURN_STATUS (ret);
      if (pDev->err == VMMC_ERR_CERR)
         RETURN_STATUS (VMMC_statusCerr);
   }

   return VMMC_statusOk;
}


/**
   Set the signal inputs of the cached fw message for the given channel.

  \param  pCh           Pointer to the VMMC channel structure.
  \return  IFX_SUCCESS or IFX_ERROR
*/
IFX_int32_t VMMC_SIG_Set_Inputs (VMMC_CHANNEL *pCh)
{
   SIG_CHAN_t        *p_fw_sig_ch;
   int               ret = IFX_SUCCESS;

   /* update the signal inputs of this cached msg */
   p_fw_sig_ch = &pCh->pSIG->fw_sig_ch;

   VMMC_OS_MutexGet (&pCh->chAcc);
   p_fw_sig_ch->I1 = VMMC_CON_Get_SIG_SignalInput (pCh, 0);
   p_fw_sig_ch->I2 = VMMC_CON_Get_SIG_SignalInput (pCh, 1);

   /* Write the updated cached message to fw only if channel is running */
   if (p_fw_sig_ch->EN)
   {
      ret = CmdWrite (pCh->pParent, (IFX_uint32_t *)p_fw_sig_ch,
                      sizeof(SIG_CHAN_t) - CMD_HDR_CNT);
   }
   VMMC_OS_MutexRelease (&pCh->chAcc);

   if (ret != VMMC_statusOk)
   {
      VMMC_DEVICE *pDev = pCh->pParent;
      RETURN_DEVSTATUS (ret);
   }

   return VMMC_statusOk;
}


/**
   Configure rt(c)p for a new connection

   \param  pCh             Pointer to the VMMC channel structure.
   \param  pRtpConf        Pointer to IFX_TAPI_PKT_RTP_CFG_t structure.

   \return Return value as follows:
   - VMMC_statusWrongEvpt Event payload type mismatch
   - VMMC_statusParam At least one parameter is wrong
   - VMMC_statusOk if successful

\remarks
   The field 'nEventPlayPT' is only used when the field 'nPlayEvents' is set
   to IFX_TAPI_PKT_EV_OOBPLAY_APT_PLAY.
*/
IFX_int32_t VMMC_SIG_RTP_OOB_Cfg (VMMC_CHANNEL *pCh,
                                   IFX_TAPI_PKT_RTP_CFG_t const *pRtpConf)
{
   VMMC_DEVICE      *pDev = pCh->pParent;
   IFX_uint32_t     *pCmd, nCount;
   IFX_uint8_t       nEventPT,
                     nEventPlayPT;
   IFX_int32_t       ret;

   /* cut payload-type parameters to the allowed range */
   nEventPT     = pRtpConf->nEventPT     & 0x7F;
   nEventPlayPT = pRtpConf->nEventPlayPT & 0x7F;

   /* check payload-type parameters for valid and exclusive values */
   if (nEventPT == 0)
   {
      /* errmsg: Event payload type mismatch */
      RETURN_STATUS (VMMC_statusWrongEvpt);
   }
   if ((pRtpConf->nPlayEvents == IFX_TAPI_PKT_EV_OOBPLAY_APT_PLAY) &&
       ((nEventPlayPT == 0) ||
        (nEventPlayPT == nEventPT)))
   {
      /* errmsg: Event payload type mismatch */
      RETURN_STATUS (VMMC_statusWrongEvpt);
   }

   /* protect fw messages */
   VMMC_OS_MutexGet (&pDev->mtxMemberAcc);

   /* event transmission settings */
   switch (pRtpConf->nEvents)
   {
      case IFX_TAPI_PKT_EV_OOB_NO:
      case IFX_TAPI_PKT_EV_OOB_BLOCK:
         /* No event transmission support */
         pCh->pSIG->et_stat.value = 0;
         break;
      case IFX_TAPI_PKT_EV_OOB_DEFAULT:
      case IFX_TAPI_PKT_EV_OOB_ONLY:
      case IFX_TAPI_PKT_EV_OOB_ALL:
         /* event transmission support */
         pCh->pSIG->et_stat.value = 0xFFFF;
         break;
      default:
         VMMC_OS_MutexRelease (&pDev->mtxMemberAcc);
         /* errmsg: At least one parameter is wrong */
         RETURN_STATUS (VMMC_statusParam);
   }

   /* configure the event play out */
   switch (pRtpConf->nPlayEvents)
   {
      case IFX_TAPI_PKT_EV_OOBPLAY_DEFAULT:
      case IFX_TAPI_PKT_EV_OOBPLAY_PLAY:
      case IFX_TAPI_PKT_EV_OOBPLAY_APT_PLAY:
         /* enable the playout of all event tones the EPOU can play */
         pCh->pSIG->fw_sig_rtp_sup.EVTOG = 0x07;
         break;
      case IFX_TAPI_PKT_EV_OOBPLAY_MUTE:
         /* disable the playout of all event tones the EPOU can play */
         pCh->pSIG->fw_sig_rtp_sup.EVTOG = 0x0;
         break;
      default:
         VMMC_OS_MutexRelease (&pDev->mtxMemberAcc);
         /* errmsg: At least one parameter is wrong */
         RETURN_STATUS (VMMC_statusParam);
   }

   /* set SSRC for the OOB transmission */
   pCh->pSIG->fw_sig_rtp_sup.SSRC =
      (IFX_uint32_t)(pRtpConf->nSsrc & 0xFFFFFFFFL);

   /* set payload type */
   pCh->pSIG->fw_sig_rtp_sup.EVT_PT = nEventPT;
   pCh->pSIG->fw_sig_rtp_sup.EVT_PTDS = nEventPlayPT;
   /* use different pt for upstream and downstream direction? */
   if(pRtpConf->nPlayEvents == IFX_TAPI_PKT_EV_OOBPLAY_APT_PLAY)
   {
      /* different payload types for upstream and downstream direction */
      pCh->pSIG->fw_sig_rtp_sup.EN_EPTD = 1;
   }
   else
   {
      /* same payload types for upstream and downstream direction */
      pCh->pSIG->fw_sig_rtp_sup.EN_EPTD = 0;
   }

   /* EPOU Trigger is always enabled; event filtering is done on TAPI level */

   /* set event pt, ssrc, tone playout direction for signalling channel.
      This has to be done with inactive signalling channel */
   pCmd   = (IFX_uint32_t *) &pCh->pSIG->fw_sig_rtp_sup;
   nCount = sizeof (pCh->pSIG->fw_sig_rtp_sup) - CMD_HDR_CNT;
   ret    = CmdWrite (pDev, pCmd, nCount);

   if (VMMC_SUCCESS (ret))
   {
      /* Store the payload type for downstream packet classification. This
         is used in the downstream packet path to add the correct header to
         the event packets that are put into the downstream packet mailbox. */
      if (pRtpConf->nPlayEvents == IFX_TAPI_PKT_EV_OOBPLAY_APT_PLAY)
      {
         pCh->nEvtPT = nEventPlayPT;
      }
      else
      {
         pCh->nEvtPT = nEventPT;
      }
   }

   VMMC_OS_MutexRelease (&pDev->mtxMemberAcc);

   if (VMMC_SUCCESS (ret))
   {
      /* event transmission settings for the DTMF receiver */
      ret = VMMC_TAPI_LL_SIG_DTMFD_OOB (pCh,
                                        (IFX_TAPI_PKT_EV_OOB_t)pRtpConf->nEvents);
   }
   if (VMMC_SUCCESS (ret))
   {
      /* event transmission settings for the MFTD */
      ret = VMMC_TAPI_LL_SIG_MFTD_OOB (pCh,
                                       (IFX_TAPI_PKT_EV_OOB_t)pRtpConf->nEvents);
   }

   RETURN_STATUS (ret);
}


/**
   Initalize the signalling module and the cached firmware messages.

  \param  pCh           Pointer to the VMMC channel structure.
*/
IFX_void_t VMMC_SIG_InitCh (VMMC_CHANNEL *pCh)
{
   VMMC_DEVICE   *pDev = pCh->pParent;
   VMMC_SIGCH_t  *pSig = pCh->pSIG;
   IFX_uint8_t   ch = pCh->nChannel - 1;

   VMMC_CON_Init_SigCh (pCh);

   /* SIG channel control */
   memset(&pSig->fw_sig_ch, 0, sizeof(pSig->fw_sig_ch));
   pSig->fw_sig_ch.CMD  = CMD_EOP;
   pSig->fw_sig_ch.CHAN = ch;
   pSig->fw_sig_ch.MOD  = MOD_SIGNALING;
   pSig->fw_sig_ch.ECMD = SIG_CHAN_ECMD;

   /* UTG control */
   memset(&pSig->fw_utg[0], 0 ,sizeof(pSig->fw_utg[0]));
   pSig->fw_utg[0].CMD  = CMD_EOP;
   pSig->fw_utg[0].CHAN = ch;
   pSig->fw_utg[0].MOD  = MOD_SIGNALING;
   pSig->fw_utg[0].ECMD = SIG_UTG_CTRL_ECMD;

   memset(&pSig->fw_utg[1], 0 ,sizeof(pSig->fw_utg[1]));
   pSig->fw_utg[1].CMD  = CMD_EOP;
   pSig->fw_utg[1].CHAN = ch;
   pSig->fw_utg[1].MOD  = MOD_SIGNALING;
   pSig->fw_utg[1].ECMD = SIG_UTG2_CTRL_ECMD;

   if (pDev->caps.bEventMailboxSupported)
   {
      pSig->fw_utg[0].EU = SIG_UTG_CTRL_EU_ON;
      pSig->fw_utg[1].EU = SIG_UTG_CTRL_EU_ON;
   }

   /* UTG coefficient (used for both UTGs - ch will be modified then) */
   memset(&pSig->fw_sig_utg_coef, 0, sizeof(pSig->fw_sig_utg_coef));
   pSig->fw_sig_utg_coef.CMD    = CMD_EOP;
   pSig->fw_sig_utg_coef.CHAN   = ch;
   pSig->fw_sig_utg_coef.ECMD   = RES_UTG_COEF_ECMD;
   pSig->fw_sig_utg_coef.MOD    = MOD_RESOURCE;

   /* MFTD control */
   memset(&pSig->fw_sig_mftd, 0, sizeof(pSig->fw_sig_mftd));
   pSig->fw_sig_mftd.CMD   = CMD_EOP;
   pSig->fw_sig_mftd.CHAN  = ch;
   pSig->fw_sig_mftd.MOD   = MOD_SIGNALING;
   pSig->fw_sig_mftd.ECMD  = SIG_MFTD_CTRL_ECMD;
   pSig->fw_sig_mftd.MFTDNR = ch;
   if (pDev->caps.bEventMailboxSupported)
   {
      pSig->fw_sig_mftd.EV = 1;
   }

   VMMC_SIG_DTMF_InitCh(pCh);

#ifdef TAPI_CID
   VMMC_SIG_CID_InitCh (pCh);
#endif /* TAPI_CID */

   /* SIG channel RTP support configuration */
   memset(&pSig->fw_sig_rtp_sup, 0, sizeof(pSig->fw_sig_rtp_sup));
   pSig->fw_sig_rtp_sup.CMD  = CMD_EOP;
   pSig->fw_sig_rtp_sup.CHAN = ch;
   pSig->fw_sig_rtp_sup.MOD  = MOD_SIGNALING;
   pSig->fw_sig_rtp_sup.ECMD = SIG_RTP_SUP_ECMD;
   /* important to note - assumption is that COD-SIG are always connected
                          straight! */
   pSig->fw_sig_rtp_sup.CC   = ch;

   /* CPTD control */
   memset(&pSig->fw_sig_cptd, 0, sizeof(pSig->fw_sig_cptd));
   pSig->fw_sig_cptd.CMD  = CMD_EOP;
   pSig->fw_sig_cptd.CHAN = ch;
   pSig->fw_sig_cptd.MOD  = MOD_SIGNALING;
   pSig->fw_sig_cptd.ECMD = SIG_CPTD_CTRL_ECMD;

   pSig->nCPTD_Cnt = 0;
   memset(pSig->nCPTD_ToneIndex, 0, sizeof(pSig->nCPTD_ToneIndex));

   /* RTP event generation statistics */
   memset (&pSig->fw_sig_rtp_evt, 0, sizeof (pSig->fw_sig_rtp_evt));
   pSig->fw_sig_rtp_evt.CMD    = CMD_EOP;
   pSig->fw_sig_rtp_evt.CHAN   = ch;
   pSig->fw_sig_rtp_evt.MOD    = MOD_SIGNALING;
   pSig->fw_sig_rtp_evt.ECMD   = SIG_RTP_EVT_STAT_ECMD;
}


/**
   Stop SIG on this channel

   \param  pCh             Pointer to the VMMC channel structure.

   \return
   - VMMC_statusOk         If successful
   - VMMC_statusCmdWr      Writing the command has failed
*/
IFX_int32_t VMMC_SIG_ChStop (VMMC_CHANNEL *pCh)
{
   IFX_int32_t       ret   = VMMC_statusOk;
   IFX_TAPI_TONE_CPTD_t sigCptd;

   /* calling function should ensure valid parameters */
   VMMC_ASSERT(pCh != IFX_NULL);

   if (pCh->pSIG != IFX_NULL)
   {
      /* protect fw msg */
      VMMC_OS_MutexGet (&pCh->chAcc);

      /* before the SIG module can be deactivated all algorithms must be
         deactivated */

      VMMC_SIG_MFTD_Set (pCh, 0, 0);
      VMMC_SIG_DTMFD_Set (pCh, IFX_FALSE, 0);
      VMMC_TAPI_LL_SIG_DTMFG_Stop ((IFX_TAPI_LL_CH_t *) pCh);
      VMMC_TAPI_LL_SIG_UTG_Stop ((IFX_TAPI_LL_CH_t *) pCh, 0);
      VMMC_TAPI_LL_SIG_UTG_Stop ((IFX_TAPI_LL_CH_t *) pCh, 1);
#ifdef TAPI_CID
      VMMC_TAPI_LL_SIG_CID_RX_Stop ((IFX_TAPI_LL_CH_t *) pCh, IFX_NULL);
      VMMC_TAPI_LL_SIG_CID_TX_Stop ((IFX_TAPI_LL_CH_t *) pCh);
#endif /* TAPI_CID */

      memset(&sigCptd, 0, sizeof(sigCptd));
      /* to disable all CPTDs */
      sigCptd.tone = 0;
      VMMC_TAPI_LL_SIG_CPTD_Stop ((IFX_TAPI_LL_CH_t *) pCh, &sigCptd);

      /* The call to AutoChStop() in the last function already took care of
         deactivating the channel. */

      VMMC_OS_MutexRelease (&pCh->chAcc);
   }

   RETURN_STATUS(ret);
}


/**
   Configure or check SIG module for given sampling mode.

   \param  pCh             Pointer to the VMMC channel structure.
   \param  action          Action to be executed (set or check).
   \param  mode            Signalling array operation mode (16kHz or 8 kHz).

   \return
   If action is SM_SET: IFX_SUCCESS or IFX_ERROR.
   If action is SM_CHECK: IFX_TRUE when module would do a switch or IFX_FALSE
                          if nothing needs to be done.

*/
IFX_int32_t  VMMC_SIG_SamplingMode (VMMC_CHANNEL *pCh,
                                    SM_ACTION action,
                                    OPMODE_SMPL mode)
{
   VMMC_DEVICE      *pDev = pCh->pParent;
   SIG_CHAN_t       *pSigCh;
   IFX_uint32_t     *pCmd;
   IFX_uint32_t     nCount = (sizeof(SIG_CHAN_t)- CMD_HDR_CNT);
   IFX_int32_t      ret = IFX_SUCCESS;

   /* calling function should ensure valid parameters */
   VMMC_ASSERT(pCh != NULL);

   pSigCh = &pCh->pSIG->fw_sig_ch;

   /* protect fw msg */
   VMMC_OS_MutexGet (&pCh->chAcc);

   /* check if passed FW module already operates in requested mode */
   if( ((mode == WB_16_KHZ) && (pSigCh->ISR == 0)) ||
       ((mode == NB_8_KHZ)  && (pSigCh->ISR == 1)) )
   {
      /* If action is execute do changes otherwise report need for execution. */
      if (action == SM_SET)
      {
         pCmd = (IFX_uint32_t*)pSigCh;

         /* configure signalling channel with changed ISR bit */
         pSigCh->ISR = !(pSigCh->ISR);
         TRACE(VMMC, DBG_LEVEL_LOW,
               ("Set SIG channel %u ISR = %d\n", pCh->nChannel-1, pSigCh->ISR));

         if((ret = CmdWrite(pDev, pCmd, nCount)) != IFX_SUCCESS)
         {
            TRACE(VMMC, DBG_LEVEL_HIGH,
               ("failed to change ISR for signalling channel\n"));
            return ret;
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
               ("Sampling rate of SIG on channel %u already matching\n",
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
   Allocate data structure of the SIG module in the given channel.

   \param  pCh             Pointer to the VMMC channel structure.

   \return
      - VMMC_statusOk if ok
      - VMMC_statusNoMem Memory allocation failed -> struct not created

   \remarks The channel parameter is no longer checked because the calling
   function assures correct values.
*/
IFX_int32_t VMMC_SIG_Allocate_Ch_Structures (VMMC_CHANNEL *pCh)
{
   VMMC_SIG_Free_Ch_Structures (pCh);

   pCh->pSIG = VMMC_OS_Malloc (sizeof(VMMC_SIGCH_t));
   if (pCh->pSIG == NULL)
   {
      /* errmsg: Memory allocation failed */
      RETURN_STATUS (VMMC_statusNoMem);
   }
   memset(pCh->pSIG, 0, sizeof(VMMC_SIGCH_t));
#ifdef EVALUATION
   if (VMMC_Eval_SIG_Allocate_Ch_Structures (pCh) != VMMC_statusOk)
   {
      RETURN_STATUS(VMMC_statusDevInitFail);
   }
#endif /* #ifdef EVALUATION */

   return VMMC_statusOk;
}


/**
   Free data structure of the SIG module in the given channel.

   \param  pCh             Pointer to the VMMC channel structure.
*/
IFX_void_t VMMC_SIG_Free_Ch_Structures (VMMC_CHANNEL *pCh)
{
   if (pCh->pSIG != IFX_NULL)
   {
#ifdef EVALUATION
      VMMC_Eval_SIG_Free_Ch_Structures (pCh);
#endif /* #ifdef EVALUATION */
      VMMC_OS_Free (pCh->pSIG);
      pCh->pSIG = IFX_NULL;
   }
}


/**
   Read out Signalling channel event information

   \param  pCh          Pointer to VMMC channel structure.
   \param  ReceivedBytesLow   Pointer to uint32 which returns the lower 32-bit
                              of the received bytes counter.
   \param  ReceivedBytesHigh  Pointer to uint32 which returns the upper 32-bit
                              of the received bytes counter.

   \return  VMMC_statusOk or ret code in case the stucture could not be created.
*/
IFX_int32_t VMMC_SIG_Event_Stat_Get (VMMC_CHANNEL *pCh,
                                     IFX_uint32_t *ReceivedBytesLow,
                                     IFX_uint32_t *ReceivedBytesHigh)
{
   SIG_RTP_EVT_STAT_t sigRtpEvt;
   IFX_int32_t ret;

   memset ((void *)&sigRtpEvt, 0, sizeof (SIG_RTP_EVT_STAT_t));

   ret = CmdRead (pCh->pParent,
                  (IFX_uint32_t *)&pCh->pSIG->fw_sig_rtp_evt,
                  (IFX_uint32_t *)&sigRtpEvt,
                  sizeof(SIG_RTP_EVT_STAT_t)-CMD_HDR_CNT);

   if (VMMC_SUCCESS (ret))
   {
      /* Generate a high 32-bit value by checking the overflow of the lower
         32-bit reported by the firmware. */
      if (sigRtpEvt.RECEIVED_BYTES < pCh->pSIG->nRecBytesL)
      {
         pCh->pSIG->nRecBytesH++;
      }
      pCh->pSIG->nRecBytesL = sigRtpEvt.RECEIVED_BYTES;

      /* return the counters in these parameters */
      *ReceivedBytesLow =  pCh->pSIG->nRecBytesL;
      *ReceivedBytesHigh = pCh->pSIG->nRecBytesH;
   }

   RETURN_STATUS (ret);
}


/**
   Reset the Signalling channel event information

   \param  pCh          Pointer to VMMC channel structure.

   \return
   - VMMC_statusOk if successful
   - VMMC_statusCmdWr Writing the command failed
*/
IFX_int32_t VMMC_SIG_Event_Stat_Reset (VMMC_CHANNEL *pCh)
{
   VMMC_DEVICE *pDev = pCh->pParent;
   IFX_int32_t ret;

   /* Reset statistics fw counters by writing command of length 0 */
   ret = CmdWrite (pDev, (IFX_uint32_t *) &pCh->pSIG->fw_sig_rtp_evt, 0);

   if (VMMC_SUCCESS(ret))
   {
      /* Reset the cached absolute values of the received bytes */
      pCh->pSIG->nRecBytesL = pCh->pSIG->nRecBytesH = 0L;
   }

   RETURN_STATUS (ret);
}


/**
   Enables signal detection

   This functions assumes when a signaling resource is available that
   DTMF receiver and MFTD is supported and available wihtout limitation.
   Calling this function without changes (e.g. pSig is empty) results in a
   return code VMMC_statusOk and no action on the device.

   ATD can either be disabled, enabled to detect signal and phase
   reversals or enabled to detect signal with phase reversals and AM.
   So when enabled the phase reversal detection is always active.
   Enabeling detection of AM implicitly activates the detector.

   This function sends the DTMF receiver and/or the MFTD VoFW message.

   \param  pLLChannel   Pointer to the VMMC channel structure.
   \param  pSig         Handle to IFX_TAPI_SIG_DETECTION_t structure.
   \return
      - VMMC_statusCmdWr Writing the command failed
      - VMMC_statusNoRes No free resources to detect a tone.
      - VMMC_statusInvalCh No SIG module on the adressed channel.
      - VMMC_statusOk if successful
*/
IFX_int32_t VMMC_TAPI_LL_SIG_DetectEnable (IFX_TAPI_LL_CH_t *pLLChannel,
                                           IFX_TAPI_SIG_DETECTION_t const *pSig)
{
   VMMC_CHANNEL *pCh = (VMMC_CHANNEL *) pLLChannel;
   IFX_int32_t  ret  = IFX_SUCCESS;

   if (pCh->pSIG == IFX_NULL)
   {
      /* errmsg: Resource not valid. Channel number out of range */
      RETURN_STATUS (VMMC_statusInvalCh);
   }

   /* The DTMF receiver has only one input. So setting it to both directions
      at the same time is just as impossible as activating it in one direction
      while it is still active in the other direction. */
   if ( ((pSig->sig & (IFX_TAPI_SIG_DTMFTX | IFX_TAPI_SIG_DTMFRX)) ==
         (IFX_TAPI_SIG_DTMFTX | IFX_TAPI_SIG_DTMFRX)) ||
        ((pSig->sig & IFX_TAPI_SIG_DTMFTX) &&
         (pCh->pSIG->sigMask & IFX_TAPI_SIG_DTMFRX)) ||
        ((pSig->sig & IFX_TAPI_SIG_DTMFRX) &&
         (pCh->pSIG->sigMask & IFX_TAPI_SIG_DTMFTX)) )
   {
      /* errmsg: DTMF detector bad input direction selection */
      RETURN_STATUS (VMMC_statusSigDtmfBadInputSelect);
   }

   /* protect fw messages */
   VMMC_OS_MutexGet (&pCh->chAcc);

   ret = VMMC_SIG_AutoChStop (pCh, IFX_TRUE);

   if (VMMC_SUCCESS(ret))
   {
      ret = VMMC_SIG_MFTD_Set (pCh, pCh->pSIG->sigMask | pSig->sig,
                               pCh->pSIG->sigMaskExt | pSig->sig_ext);
   }

   if (VMMC_SUCCESS(ret) &&
       (pSig->sig & (IFX_TAPI_SIG_DTMFTX | IFX_TAPI_SIG_DTMFRX)))
   {
      if (pSig->sig & IFX_TAPI_SIG_DTMFTX)
      {
         /* enable the DTMF receiver in TX direction (local) */
         ret = VMMC_SIG_DTMFD_Set (pCh, IFX_TRUE, VMMC_SIG_TX);
         if (VMMC_SUCCESS(ret))
         {
            pCh->pSIG->sigMask |= IFX_TAPI_SIG_DTMFTX;
         }
      }
      else
      {
         /* enable the DTMF receiver in RX direction (from network side) */
         ret = VMMC_SIG_DTMFD_Set (pCh, IFX_TRUE, VMMC_SIG_RX);
         if (VMMC_SUCCESS(ret))
         {
            pCh->pSIG->sigMask |= IFX_TAPI_SIG_DTMFRX;
         }
      }
   }

   VMMC_OS_MutexRelease (&pCh->chAcc);

   RETURN_STATUS (ret);
}


/**
   Disables signal detection

   Disable the detectors that are specified in the bitfields of the
   IFX_TAPI_SIG_DETECTION_t structure. If none of the specified detectors
   is enabled nothing will be done.

   \param  pLLChannel   Pointer to the VMMC channel structure.
   \param  pSig         Pointer to IFX_TAPI_SIG_DETECTION_t structure.
   \return
      - VMMC_statusInvalCh No SIG module on the adressed channel.
      - VMMC_statusCmdWr Writing the command failed
      - VMMC_statusOk if successful
*/
IFX_int32_t VMMC_TAPI_LL_SIG_DetectDisable (IFX_TAPI_LL_CH_t *pLLChannel,
                                            IFX_TAPI_SIG_DETECTION_t const *pSig)
{
   VMMC_CHANNEL *pCh = (VMMC_CHANNEL *) pLLChannel;
   IFX_int32_t  ret  = VMMC_statusOk;

   if (pCh->pSIG == IFX_NULL)
   {
      /* errmsg: Resource not valid. Channel number out of range */
      RETURN_STATUS (VMMC_statusInvalCh);
   }

   /* protect fw messages */
   VMMC_OS_MutexGet (&pCh->chAcc);

   ret = VMMC_SIG_MFTD_Set (pCh, pCh->pSIG->sigMask & ~pSig->sig,
                            pCh->pSIG->sigMaskExt & ~pSig->sig_ext);

   if ((VMMC_SUCCESS(ret)) &&
       (pSig->sig & (IFX_TAPI_SIG_DTMFTX | IFX_TAPI_SIG_DTMFRX)))
   {
      /* disable the DTMF receiver in both directions */
      ret = VMMC_SIG_DTMFD_Set (pCh, IFX_FALSE, 0);
      if (VMMC_SUCCESS(ret))
      {
         pCh->pSIG->sigMask &= ~(IFX_TAPI_SIG_DTMFTX | IFX_TAPI_SIG_DTMFRX);
      }
   }

   if (VMMC_SUCCESS(ret))
   {
      ret = VMMC_SIG_AutoChStop (pCh, IFX_FALSE);
   }

   VMMC_OS_MutexRelease (&pCh->chAcc);

   RETURN_STATUS (ret);
}
