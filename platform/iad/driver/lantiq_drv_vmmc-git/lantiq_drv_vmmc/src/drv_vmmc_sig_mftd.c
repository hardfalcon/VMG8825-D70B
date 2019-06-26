/******************************************************************************

                              Copyright (c) 2012
                            Lantiq Deutschland GmbH
                             http://www.lantiq.com

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/** \file drv_vmmc_sig_mftd.c
    This files implements the SIG - MFTD module.
    Function definitions of Modem and Fax Tone Detection module. */

/* ============================= */
/* Includes                      */
/* ============================= */

#include "drv_api.h"
#include "drv_vmmc_sig_priv.h"
#include "drv_vmmc_api.h"
#include "drv_vmmc_cod.h"

/* ============================= */
/* Local Macros  Definitions    */
/* ============================= */

static IFX_TAPI_EVENT_ID_t pMftdMap[] =
{
   /* index 00 : reserved (intermediate value between two states) */
   IFX_TAPI_EVENT_FAXMODEM_NONE,
   /* index 01 : holding level detection - only when nothing else is detected */
   IFX_TAPI_EVENT_FAXMODEM_NONE,
   /* index 02 : V.21 DIS preamble detection */
   IFX_TAPI_EVENT_FAXMODEM_DIS,
   /* index 03 : Voice modem discriminator */
   IFX_TAPI_EVENT_FAXMODEM_VMD,
   /* index 04 : ANS detected */
   IFX_TAPI_EVENT_FAXMODEM_CED,
   /* index 05 : ANS with one phase reversal detected */
#if (IFX_TAPI_SIG_MFTD_PR_THRESHOLD == 1)
    IFX_TAPI_EVENT_FAXMODEM_PR,
#else
   IFX_TAPI_EVENT_FAXMODEM_NONE,
#endif
   /* index 06 : ANS with two phase reversals detected */
#if (IFX_TAPI_SIG_MFTD_PR_THRESHOLD == 2)
    IFX_TAPI_EVENT_FAXMODEM_PR,
#else
   IFX_TAPI_EVENT_FAXMODEM_NONE,
#endif
   /* index 07 : ANS with three phase reversals detected */
#if (IFX_TAPI_SIG_MFTD_PR_THRESHOLD == 3)
    IFX_TAPI_EVENT_FAXMODEM_PR,
#else
   IFX_TAPI_EVENT_FAXMODEM_NONE,
#endif
   /* index 08 : ANSam detected */
   IFX_TAPI_EVENT_FAXMODEM_AM,
   /* index 09 : ANSam with one phase reversal detected */
#if (IFX_TAPI_SIG_MFTD_PR_THRESHOLD == 1)
    IFX_TAPI_EVENT_FAXMODEM_PR,
#else
   IFX_TAPI_EVENT_FAXMODEM_NONE,
#endif
   /* index 10 : ANSam with two phase reversals detected */
#if (IFX_TAPI_SIG_MFTD_PR_THRESHOLD == 2)
    IFX_TAPI_EVENT_FAXMODEM_PR,
#else
   IFX_TAPI_EVENT_FAXMODEM_NONE,
#endif
   /* index 11 : ANSam with three phase reversals detected */
#if (IFX_TAPI_SIG_MFTD_PR_THRESHOLD == 3)
    IFX_TAPI_EVENT_FAXMODEM_PR,
#else
   IFX_TAPI_EVENT_FAXMODEM_NONE,
#endif
   /* index 12 :  980 Hz single tone detected (V.21L mark sequence) */
   IFX_TAPI_EVENT_FAXMODEM_V21L,
   /* index 13 : 1400 Hz single tone detected (V.18A mark sequence) */
   IFX_TAPI_EVENT_FAXMODEM_V18A,
   /* index 14 : 1800 Hz single tone detected (V.27, V.32 carrier signal) */
   IFX_TAPI_EVENT_FAXMODEM_V27,
   /* index 15 : 1300 Hz single tone detected (Modem calling tone) */
   IFX_TAPI_EVENT_FAXMODEM_CNGMOD,
   /* index 16 : 1100 Hz single tone detected (FAX calling tone) */
   IFX_TAPI_EVENT_FAXMODEM_CNGFAX,
   /* index 17 : 2225 Hz single tone detected (Bell answering tone) */
   IFX_TAPI_EVENT_FAXMODEM_BELL,
   /* index 18 : 2250 Hz single tone detected (V.22 unscrambled binary ones */
   IFX_TAPI_EVENT_FAXMODEM_V22,
   /* index 19 : 1650 Hz single tone (V.21H mark sequence) */
   IFX_TAPI_EVENT_FAXMODEM_V21H,
   /* index 20 : reserved */
   IFX_TAPI_EVENT_FAXMODEM_NONE,
   /* index 21 : 2225 Hz single tone or 2250 Hz single tone detected */
   IFX_TAPI_EVENT_FAXMODEM_V22ORBELL,
   /* index 22 : reserved */
   IFX_TAPI_EVENT_FAXMODEM_NONE,
   /* index 23 : reserved */
   IFX_TAPI_EVENT_FAXMODEM_NONE,
   /* index 24 : 600 Hz + 3000 Hz dual tone detected (V.32 AC signal) */
   IFX_TAPI_EVENT_FAXMODEM_V32AC,
   /* index 25 : 1375 Hz + 2002 Hz dual tone detected (V.8bis init segment) */
   IFX_TAPI_EVENT_FAXMODEM_V8BIS,
   /* index 26 : 2130 Hz + 2750 Hz dual tone (Bell Caller ID Type 2 Alert Tone)*/
   IFX_TAPI_EVENT_FAXMODEM_CAS_BELL,
   /* index 27 : reserved */
   IFX_TAPI_EVENT_FAXMODEM_NONE,
   /* index 28 : reserved */
   IFX_TAPI_EVENT_FAXMODEM_NONE,
   /* index 29 : reserved */
   IFX_TAPI_EVENT_FAXMODEM_NONE,
   /* index 30 : reserved */
   IFX_TAPI_EVENT_FAXMODEM_NONE,
   /* index 31 : no detection -> tone holding end */
   IFX_TAPI_EVENT_FAXMODEM_HOLDEND
};

/* Modem tone holding signal stopped mask */
#define VMMC_SIG_MASK_TONEHOLDING_END  (IFX_TAPI_SIG_TONEHOLDING_END  |  \
                                        IFX_TAPI_SIG_TONEHOLDING_ENDRX |  \
                                        IFX_TAPI_SIG_TONEHOLDING_ENDTX)

/* ============================= */
/* Global variable definition    */
/* ============================= */

/* ============================= */
/* Global function declaration   */
/* ============================= */

/* ============================= */
/* Local function declaration    */
/* ============================= */

/* ============================= */
/* Local variable definition     */
/* ============================= */

/* ============================= */
/* Local function definition     */
/* ============================= */

/* ============================= */
/* Global function definition    */
/* ============================= */

/**
   Sets signal detection on the Modem Fax Tone Detector (MFTD)

   Sets the MFTD to detect exactly the tones given in the bitfields signal and
   signalExt.

   \param  pCh          Pointer to VMMC channel structure.
   \param  nSignal      Signal definition bitfield.
   \param  nSignalExt   Extended signal definition bitfield.

   \return Return value as follows:
      - VMMC_statusCmdWr Writing the command failed
      - VMMC_statusOk if successful
*/
IFX_int32_t VMMC_SIG_MFTD_Set (VMMC_CHANNEL *pCh, IFX_uint32_t nSignal,
                               IFX_uint32_t nSignalExt)
{
   VMMC_DEVICE *pDev = pCh->pParent;
   IFX_int32_t ret = VMMC_statusOk;
   SIG_MFTD_CTRL_t *pMftd = &pCh->pSIG->fw_sig_mftd;

   /* reprogram the MFTD only if something has changed */
   if ((nSignal    != pCh->pSIG->sigMask) ||
       (nSignalExt != pCh->pSIG->sigMaskExt))
   {
      /* changing anything but the MH or VMD bit requires to stop the MFTD first
         but stop only if the MFTD has previously been enabled
         also stop if only MH or VMD are active and now get both deactivated */
      if ((pMftd->EN == 1) &&
          (
            (((nSignalExt ^ pCh->pSIG->sigMaskExt) & ~IFX_TAPI_SIG_EXT_VMD) ||
             ((nSignal ^ pCh->pSIG->sigMask) & ~VMMC_SIG_MASK_TONEHOLDING_END))
            ||
            ((((nSignalExt ^ pCh->pSIG->sigMaskExt) & IFX_TAPI_SIG_EXT_VMD) ||
              ((nSignal ^ pCh->pSIG->sigMask) & VMMC_SIG_MASK_TONEHOLDING_END))
             &&
             (!(nSignalExt & IFX_TAPI_SIG_EXT_VMD) &&
              !(nSignal & VMMC_SIG_MASK_TONEHOLDING_END)))
          )
         )
      {
         pMftd->EN = 0;
         ret = CmdWrite (pDev, (IFX_uint32_t*)pMftd,
                         sizeof(SIG_MFTD_CTRL_t) - CMD_HDR_CNT);
         if (!VMMC_SUCCESS(ret))
            RETURN_STATUS (ret);
      }

      /* first clear the data fields of the cached command that we set here */
      pMftd->MH = 0;
      pMftd->VMD = 0;
      /* clear single, dual, atd and dis */
      ((IFX_uint32_t*)pMftd)[2] = 0x0000;

      /* Modem holding characteristic is independent of the direction */
      if (nSignal & VMMC_SIG_MASK_TONEHOLDING_END)
         pMftd->MH = 1;

      /* Voice / Modem discriminator is independent of the direction*/
      if (nSignalExt & IFX_TAPI_SIG_EXT_VMD)
         pMftd->VMD = 1;

      /* Next the single tone detectors are activated as required */

      /* V.21 mark sequence */
      if (nSignalExt & (IFX_TAPI_SIG_EXT_V21L | IFX_TAPI_SIG_EXT_V21LRX))
         pMftd->SINGLE2 |=  SIG_MFTD_SINGLE_V21L;

      if (nSignalExt & (IFX_TAPI_SIG_EXT_V21L | IFX_TAPI_SIG_EXT_V21LTX))
         pMftd->SINGLE1 |=  SIG_MFTD_SINGLE_V21L;

      /* V.18A mark sequence */
      if (nSignalExt & (IFX_TAPI_SIG_EXT_V18A | IFX_TAPI_SIG_EXT_V18ARX))
         pMftd->SINGLE2 |=  SIG_MFTD_SINGLE_V18A;

      if (nSignalExt & (IFX_TAPI_SIG_EXT_V18A | IFX_TAPI_SIG_EXT_V18ATX))
         pMftd->SINGLE1 |=  SIG_MFTD_SINGLE_V18A;

      /* V.27, V.32 carrier */
      if (nSignalExt & (IFX_TAPI_SIG_EXT_V27 | IFX_TAPI_SIG_EXT_V27RX))
         pMftd->SINGLE2 |= SIG_MFTD_SINGLE_V27;

      if (nSignalExt & (IFX_TAPI_SIG_EXT_V27 | IFX_TAPI_SIG_EXT_V27TX))
         pMftd->SINGLE1 |= SIG_MFTD_SINGLE_V27;

      /* CNG Modem Calling Tone */
      if (nSignal & (IFX_TAPI_SIG_CNGMOD | IFX_TAPI_SIG_CNGMODRX))
         pMftd->SINGLE2 |= SIG_MFTD_SINGLE_CNGMOD;

      if (nSignal & (IFX_TAPI_SIG_CNGMOD | IFX_TAPI_SIG_CNGMODTX))
         pMftd->SINGLE1 |= SIG_MFTD_SINGLE_CNGMOD;

      /* CNG Fax Calling Tone */
      if (nSignal & (IFX_TAPI_SIG_CNGFAX | IFX_TAPI_SIG_CNGFAXRX))
         pMftd->SINGLE2 |= SIG_MFTD_SINGLE_CNGFAX;

      if (nSignal & (IFX_TAPI_SIG_CNGFAX | IFX_TAPI_SIG_CNGFAXTX))
         pMftd->SINGLE1 |= SIG_MFTD_SINGLE_CNGFAX;

      /* Bell answering tone */
      if (nSignalExt & (IFX_TAPI_SIG_EXT_BELL | IFX_TAPI_SIG_EXT_BELLRX))
         pMftd->SINGLE2 |= SIG_MFTD_SINGLE_BELL;

      if (nSignalExt & (IFX_TAPI_SIG_EXT_BELL | IFX_TAPI_SIG_EXT_BELLTX))
         pMftd->SINGLE1 |= SIG_MFTD_SINGLE_BELL;

      /* V.22 unsrambled binary ones */
      if (nSignalExt & (IFX_TAPI_SIG_EXT_V22 | IFX_TAPI_SIG_EXT_V22RX))
         pMftd->SINGLE2 |= SIG_MFTD_SINGLE_V22;

      if (nSignalExt & (IFX_TAPI_SIG_EXT_V22 | IFX_TAPI_SIG_EXT_V22TX))
         pMftd->SINGLE1 |= SIG_MFTD_SINGLE_V22;

      /* V.21H mark sequence */
      if (nSignalExt & (IFX_TAPI_SIG_EXT_V21HRX))
         pMftd->SINGLE2 |= SIG_MFTD_SINGLE_V21H;

      if (nSignalExt & (IFX_TAPI_SIG_EXT_V21HTX))
         pMftd->SINGLE1 |= SIG_MFTD_SINGLE_V21H;

      /* Next the dual tone detectors are activated as required */

      /* V.32 AC signal */
      if (nSignalExt & (IFX_TAPI_SIG_EXT_V32AC | IFX_TAPI_SIG_EXT_V32ACRX))
         pMftd->DUAL2 |= SIG_MFTD_DUAL_V32AC;

      if (nSignalExt & (IFX_TAPI_SIG_EXT_V32AC | IFX_TAPI_SIG_EXT_V32ACTX))
         pMftd->DUAL1 |= SIG_MFTD_DUAL_V32AC;

      /* V8bis signal */
      if (nSignal & (IFX_TAPI_SIG_V8BISRX))
         pMftd->DUAL2 |= SIG_MFTD_DUAL_V8bis;

      if (nSignal & (IFX_TAPI_SIG_V8BISTX))
         pMftd->DUAL1 |= SIG_MFTD_DUAL_V8bis;

      /* BELL CAS (Caller ID Type 2 Alert Tone) */
      if (nSignalExt & (IFX_TAPI_SIG_EXT_CASBELLRX))
         pMftd->DUAL2 |= SIG_MFTD_DUAL_CASBELL;

      if (nSignalExt & (IFX_TAPI_SIG_EXT_CASBELLTX))
         pMftd->DUAL1 |= SIG_MFTD_DUAL_CASBELL;

      /* Next the answering tone detectors are activated as required */
      /* ATD can either be disabled, enabled to detect signal and phase
         reversals or enabled to detect signal with phase reversals and AM.
         So when enabled the phase reversal detection is always active.
         Enabeling detection of AM implicitly activates the detector. */

      if (nSignal & (IFX_TAPI_SIG_CED | IFX_TAPI_SIG_CEDRX |
                     IFX_TAPI_SIG_PHASEREV | IFX_TAPI_SIG_PHASEREVRX))
         pMftd->ATD2 = SIG_MFTD_ATD_EN;

      if (nSignal & (IFX_TAPI_SIG_CED | IFX_TAPI_SIG_CEDTX |
                     IFX_TAPI_SIG_PHASEREV | IFX_TAPI_SIG_PHASEREVTX))
         pMftd->ATD1 = SIG_MFTD_ATD_EN;

      if (nSignal & (IFX_TAPI_SIG_AM  | IFX_TAPI_SIG_AMRX))
         pMftd->ATD2 = SIG_MFTD_ATD_AM_EN;

      if (nSignal & (IFX_TAPI_SIG_AM  | IFX_TAPI_SIG_AMTX))
         pMftd->ATD1 = SIG_MFTD_ATD_AM_EN;

      /* Next the DIS tone detector is enabled if required  */
      if (nSignal & (IFX_TAPI_SIG_DIS | IFX_TAPI_SIG_DISRX))
         pMftd->DIS2 = 1;

      if (nSignal & (IFX_TAPI_SIG_DIS | IFX_TAPI_SIG_DISTX))
         pMftd->DIS1 = 1;

      /* Finally enable the entire MFTD if any detectors are enabled */
      if (nSignalExt != 0 ||
          (nSignal & ~(IFX_TAPI_SIG_DTMFTX | IFX_TAPI_SIG_DTMFRX |
                       IFX_TAPI_SIG_CPTD | IFX_TAPI_SIG_CIDENDTX)) != 0)
      {
         pMftd->EN = 1;
         ret = CmdWrite (pDev, (IFX_uint32_t*)pMftd,
                         sizeof(SIG_MFTD_CTRL_t) - CMD_HDR_CNT);
      }
   }

   /* store the current setting for finding differences during the next call */
   if (VMMC_SUCCESS(ret))
   {
      pCh->pSIG->sigMask    = nSignal;
      pCh->pSIG->sigMaskExt = nSignalExt;
   }

   RETURN_STATUS (ret);
}


/**
   Configure the upstream signal transmission behaviour of tones detected by
   the MFTD.

   Controls the transmission when a tone event is detected. The tone can be
   transmitted inband and/or out-of-band with the help of RFC2833 event packets.

   \param pLLChannel    Pointer to the VMMC channel structure.
   \param nOobMode      Value from IFX_TAPI_PKT_EV_OOB_t structure.

   \return Return value as follows:
      - VMMC_statusCmdWr Writing the command failed
      - VMMC_statusOk if successful
*/
IFX_int32_t VMMC_TAPI_LL_SIG_MFTD_OOB (IFX_TAPI_LL_CH_t *pLLChannel,
                                      IFX_TAPI_PKT_EV_OOB_t nOobMode)
{
   VMMC_CHANNEL     *pCh  = (VMMC_CHANNEL *) pLLChannel;
   IFX_boolean_t     bOutband = IFX_FALSE;
   IFX_int32_t       ret = VMMC_statusOk;

   /* no outband transmission except in these modes */
   if ((nOobMode == IFX_TAPI_PKT_EV_OOB_ALL) ||
       (nOobMode == IFX_TAPI_PKT_EV_OOB_ONLY) ||
       (nOobMode == IFX_TAPI_PKT_EV_OOB_DEFAULT) )
   {
      bOutband = IFX_TRUE;
   }

   /* Update ET right here if event support is enabled in the SIG channel.
      In any case the state is kept for the update that happens when the
      SIG channel event support changes. */
   if (pCh->pSIG->fw_sig_ch.ES == 1)
   {
      /* configure the MFTD for OOB transmission */
      ret = VMMC_SIG_MFTD_EvtTransSet (pCh, bOutband);
   }

   if (VMMC_SUCCESS (ret))
   {
      /* save the setting for UpdateEventTrans() */
      pCh->pSIG->et_stat.flag.mftd = bOutband ? 1 : 0;
   }

   RETURN_STATUS (ret);
}


/**
   Sets event transmission of Modem Fax Tone Detector (MFTD) signals.

   \param  pCh          Pointer to VMMC channel structure.
   \param  bEnable      Enable

   \return Return value as follows:
      - VMMC_statusCmdWr Writing the command failed
      - VMMC_statusOk if successful
*/
IFX_int32_t VMMC_SIG_MFTD_EvtTransSet (VMMC_CHANNEL *pCh,
                                       IFX_boolean_t bEnable)
{
   VMMC_DEVICE *pDev = pCh->pParent;
   IFX_int32_t ret = VMMC_statusOk;
   SIG_MFTD_CTRL_t *pMftd = &pCh->pSIG->fw_sig_mftd;

   VMMC_OS_MutexGet (&pCh->chAcc);

   /* Check if anything needs to be done. Note that because all flags are set
      in parallel to the same value we just check one at the moment. This
      needs to be changed as soon as the flags get set individually. */
   if ( ((bEnable != IFX_FALSE) && (pMftd->ETA == 1)) ||
        ((bEnable == IFX_FALSE) && (pMftd->ETA == 0))    )
   {
      /* state is already set as requested */
      VMMC_OS_MutexRelease (&pCh->chAcc);
      return VMMC_statusOk;
   }

   /* changing these bits requires to stop the MFTD first if it is running .*/
   if (pMftd->EN == 1)
   {
      pMftd->EN = 0;
      ret = CmdWrite (pDev, (IFX_uint32_t*)pMftd,
                      sizeof(SIG_MFTD_CTRL_t) - CMD_HDR_CNT);
      if (!VMMC_SUCCESS(ret))
      {
         VMMC_OS_MutexRelease (&pCh->chAcc);
         RETURN_STATUS (ret);
      }

      /*set this as a flag for the write at the end of this function */
      pMftd->EN = 1;
   }

   if (bEnable != IFX_FALSE)
   {
      /* Enable ATD event transmission */
      pMftd->ETA = 1;
      /* Enable the CNG event transmission */
      pMftd->ETC = 1;
      /* Enable DIS event transmission */
      pMftd->ETD = 1;
   }
   else
   {
      /* Disable ATD event transmission */
      pMftd->ETA = 0;
      /* Disable the CNG event transmission */
      pMftd->ETC = 0;
      /* Disable DIS event transmission */
      pMftd->ETD = 0;
   }

   /* restart the MFTD if it was running before */
   if (pMftd->EN == 1)
   {
      ret = CmdWrite (pDev, (IFX_uint32_t*)pMftd,
                      sizeof(SIG_MFTD_CTRL_t) - CMD_HDR_CNT);
      /* ret is directly fed into the final return statement below */
   }

   VMMC_OS_MutexRelease (&pCh->chAcc);

   RETURN_STATUS (ret);
}


/**
   Interrupt handler for MFTD events

   \param  pCh          Pointer to VMMC channel structure.
   \param  nVal         Value given in irq status register.
   \param  bRx          <>0 receive or ==0 transmit.
*/
void irq_VMMC_SIG_MFTD_Event (VMMC_CHANNEL *pCh,
                              IFX_uint8_t nVal,
                              IFX_boolean_t bRx)
{
   IFX_TAPI_EVENT_t tapiEvent;

   /* Fill event structure. */
   memset(&tapiEvent, 0, sizeof(IFX_TAPI_EVENT_t));
   tapiEvent.data.fax_sig.signal = nVal;

   if (bRx)
   {
      /* MFTD2 listens on the downstream direction - receive path */
      TRACE (VMMC, DBG_LEVEL_LOW, ("MFTD Rx Event %X\n", nVal));
      tapiEvent.data.fax_sig.network = 1;

      /* handle CED end signalling */
      if (((pCh->pSIG->lastMftd2ToneIdx >= 4) &&
           (pCh->pSIG->lastMftd2ToneIdx <= 11)) &&
          ((nVal < 4) || (nVal > 11)))
      {
         tapiEvent.id = IFX_TAPI_EVENT_FAXMODEM_CEDEND;
         IFX_TAPI_Event_Dispatch(pCh->pTapiCh, &tapiEvent);
      }
      /* decode the tone index into signals using a lookup table and
         dispatch signals */
      tapiEvent.id = pMftdMap[nVal];
      /* change "holding end" event into "none" event when holding tone detector
         is not enabled. the MH working independent of the direction so take
         also care of desired direction for reporting */
      if ((tapiEvent.id == IFX_TAPI_EVENT_FAXMODEM_HOLDEND) &&
          !(pCh->pSIG->sigMask & (IFX_TAPI_SIG_TONEHOLDING_END |
                                  IFX_TAPI_SIG_TONEHOLDING_ENDRX)))
      {
         tapiEvent.id = IFX_TAPI_EVENT_FAXMODEM_NONE;
      }
      /* remember the tone index for the next time "CED end" handling */
      pCh->pSIG->lastMftd2ToneIdx = nVal;
   }
   else
   {
      /* MFTD1 listens on the upstream direction - transmit path */
      TRACE (VMMC, DBG_LEVEL_LOW, ("MFTD Tx Event %X\n", nVal));
      tapiEvent.data.fax_sig.local   = 1;
      /* handle CED end signalling */
      if (((pCh->pSIG->lastMftd1ToneIdx >= 4) &&
           (pCh->pSIG->lastMftd1ToneIdx <= 11)) &&
          ((nVal < 4) || (nVal > 11)))
      {
         tapiEvent.id = IFX_TAPI_EVENT_FAXMODEM_CEDEND;
         IFX_TAPI_Event_Dispatch(pCh->pTapiCh, &tapiEvent);
      }
      /* decode the tone index into signals using a lookup table and
         dispatch signals */
      tapiEvent.id = pMftdMap[nVal];
      /* change "holding end" event into "none" event when holding tone detector
         is disabled */
      /* change "holding end" event into "none" event when holding tone detector
         is not enabled. the MH working independent of the direction so take
         also care of desired direction for reporting */
      if ((tapiEvent.id == IFX_TAPI_EVENT_FAXMODEM_HOLDEND) &&
          !(pCh->pSIG->sigMask & (IFX_TAPI_SIG_TONEHOLDING_END |
                                  IFX_TAPI_SIG_TONEHOLDING_ENDTX)))
      {
         tapiEvent.id = IFX_TAPI_EVENT_FAXMODEM_NONE;
      }
      /* remember the tone index for the next time "CED end" handling */
      pCh->pSIG->lastMftd1ToneIdx = nVal;
   }
   if (tapiEvent.id != IFX_TAPI_EVENT_FAXMODEM_NONE)
   {
      IFX_TAPI_Event_Dispatch(pCh->pTapiCh, &tapiEvent);
   }
}
