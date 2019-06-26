/******************************************************************************

                              Copyright (c) 2012
                            Lantiq Deutschland GmbH
                             http://www.lantiq.com

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/**
   \file drv_vmmc_sig_dtmfg.c Implementation of the SIG - DTMF module.
*/

/* ============================= */
/* Includes                      */
/* ============================= */
#include "drv_api.h"
#include "drv_vmmc_sig_priv.h"
#include "drv_vmmc_api.h"
#include "drv_vmmc_cod.h"
#include "drv_vmmc_sig.h"

/* ============================= */
/* Local Macros & Definitions    */
/* ============================= */

/**
   Store status in channel structure and call according callback function if
   present

   \param  status   DTMF status.
*/
#define DTMF_STATUS(status)\
do {\
   pDtmf->state = (status);\
   if (pDtmf->stateCb != IFX_NULL)\
      pDtmf->stateCb(pCh);\
} while (0)

/* max number of DTMF words per message */
#define DTMF_MAX_MSG_WORDS       10

/**
   If define is present the driver will not make use of the auto-deactivation
   feature of the DTMF/AT generator.
   When DTMF_NO_AUTODEACT is not defned, the AD bit will be set with the last
   portion of DTMF data and the generator will be deactivated by firmware after
   transmission.
   When DTMF_NO_AUTODEACT is defined, the AD bit is not set and the driver
   deactivates the generator on reception of the implirf BUF underrun interrupt.
\remarks
   DTMF_NO_AUTODEACT should be set to guarantee immediate deactivation of the
   DTMF/AT generator on hook-events.
*/
#undef DTMF_NO_AUTODEACT

/**
   Represents the minimal allowed for programming the DTMF Receiver Level
*/
#define VMMC_DTMF_RX_LEVEL_MIN         -96

/**
   Represents the maximal allowed value for programming the DTMF Receiver Level
*/
#define VMMC_DTMF_RX_LEVEL_MAX         -1

/**
   Represents the minimal value represented by table VMMC_Gaintable[]
   which can be used to program the DTMF Receiver Gain
*/
#define VMMC_DTMF_RX_GAIN_MIN          VMMC_VOLUME_GAIN_MIN

/**
   Represents the maximum value represented by table VMMC_Gaintable[]
   which can be used to program the DTMF Receiver Gain
*/
#define VMMC_DTMF_RX_GAIN_MAX          VMMC_VOLUME_GAIN_MAX

/**
   Represents the minimal value represented by table VMMC_DtmfRxTwist[]
   which can be used to program the DTMF Receiver Twist
*/
#define VMMC_DTMF_RX_TWIST_MIN         0

/**
   Represents the maximum value represented by table VMMC_DtmfRxTwist[]
   which can be used to program the DTMF Receiver Gain
*/
#define VMMC_DTMF_RX_TWIST_MAX         12

/**
   Represents the maximum value for the voice path delay.
*/
#define VMMC_DTMF_RX_DELAY_MAX         20

/**
   Table with calculated DTMF Receiver(detector) Twist coefficients used by
   the SIG module. It represents values between 0dB and 12dB, in steps of 1dB
   calculated with the formula below.

   TWIST = 256 * 10**(-(TWIST[dB] + 0.5)/10)
   { TWIST[dB] [-0.01, 12], TWIST [E4h, 0Eh] }
*/
const IFX_uint8_t VMMC_DtmfRxTwist[] =
{
   0xe4, 0xb5, 0x8f, 0x72,       /*  0dB   1dB   2dB   3dB  */
   0x5a, 0x48, 0x39, 0x2d,       /*  4dB   5dB   6dB   7dB  */
   0x24, 0x1c, 0x16, 0x12,       /*  8dB   9dB  10dB  11dB  */
   0x0e                          /* 12dB                    */
};


/* ============================= */
/* Function definitions          */
/* ============================= */

/**
   Initalize the DTMF module and the cached firmware messages.

  \param  pCh           Pointer to the VMMC channel structure.
*/
IFX_void_t VMMC_SIG_DTMF_InitCh (VMMC_CHANNEL *pCh)
{
   VMMC_DEVICE   *pDev = pCh->pParent;
   VMMC_SIGCH_t  *pSig = pCh->pSIG;
   IFX_uint8_t   ch = pCh->nChannel - 1;

   /* DTMF generator control */
   memset(&pSig->fw_sig_dtmfgen, 0, sizeof(pSig->fw_sig_dtmfgen));
   pSig->fw_sig_dtmfgen.CMD  = CMD_EOP;
   pSig->fw_sig_dtmfgen.CHAN = ch;
   pSig->fw_sig_dtmfgen.MOD = MOD_SIGNALING;
   pSig->fw_sig_dtmfgen.ECMD = SIG_DTMFATG_CTRL_ECMD;
   pSig->fw_sig_dtmfgen.GENNR  = ch;
   if (pDev->caps.bEventMailboxSupported)
   {
      pSig->fw_sig_dtmfgen.EVM = SIG_DTMFATG_CTRL_EVM_READY |
                                 SIG_DTMFATG_CTRL_EVM_BUF_REQ |
                                 SIG_DTMFATG_CTRL_EVM_BUF_UNDERFLOW;
   }

   /* DTMF generator coefficients */
   memset(&pSig->fw_sig_dtmfgen_coef, 0, sizeof(pSig->fw_sig_dtmfgen_coef));
   pSig->fw_sig_dtmfgen_coef.CMD  = CMD_EOP;
   pSig->fw_sig_dtmfgen_coef.CHAN = ch;
   pSig->fw_sig_dtmfgen_coef.MOD  = MOD_RESOURCE;
   pSig->fw_sig_dtmfgen_coef.ECMD = RES_DTMFATG_COEF_ECMD;

   /* DTMF generator data */
   memset(&pSig->fw_sig_dtmfatg_data, 0, sizeof(pSig->fw_sig_dtmfatg_data));
   pSig->fw_sig_dtmfatg_data.CMD  = CMD_EOP;
   pSig->fw_sig_dtmfatg_data.CHAN = ch;
   pSig->fw_sig_dtmfatg_data.MOD  = MOD_RESOURCE;
   pSig->fw_sig_dtmfatg_data.ECMD = SIG_DTMFATG_DATA_ECMD;

   /* DTMF receiver control */
   memset(&pSig->fw_sig_dtmfrcv, 0, sizeof(pSig->fw_sig_dtmfrcv));
   pSig->fw_sig_dtmfrcv.CMD  = CMD_EOP;
   pSig->fw_sig_dtmfrcv.CHAN = ch;
   pSig->fw_sig_dtmfrcv.MOD  = MOD_SIGNALING;
   pSig->fw_sig_dtmfrcv.ECMD = SIG_DTMFR_CTRL_ECMD;
   pSig->fw_sig_dtmfrcv.DTRNR  = ch;
   pSig->fw_sig_dtmfrcv.FUNC = 1;
   if (pDev->caps.bEventMailboxSupported)
   {
      pSig->fw_sig_dtmfrcv.ES = SIG_DTMFR_CTRL_ES_EN;
   }

   /* DTMF receiver override control */
   memset(&pSig->fw_sig_dtmfrcv_override, 0,
          sizeof(pSig->fw_sig_dtmfrcv_override));
   pSig->fw_sig_dtmfrcv_override.CMD  = CMD_EOP;
   pSig->fw_sig_dtmfrcv_override.CHAN = ch;
   pSig->fw_sig_dtmfrcv_override.MOD  = MOD_SIGNALING;
   pSig->fw_sig_dtmfrcv_override.ECMD = SIG_DTMFR_CTRL_ECMD;
   pSig->fw_sig_dtmfrcv_override.AS = 1;
   pSig->fw_sig_dtmfrcv_override.DTRNR  = ch;
   if (pDev->caps.bEventMailboxSupported)
   {
      pSig->fw_sig_dtmfrcv_override.ES = SIG_DTMFR_CTRL_ES_EN;
   }

   pSig->bDtmfdOverride = IFX_FALSE;
   pSig->bDtmfdCoeffModified = IFX_FALSE;

   /* DTMF receiver coefficients */
   memset(&pSig->fw_sig_dtmfrcv_coef, 0, sizeof(pSig->fw_sig_dtmfrcv_coef));
   pSig->fw_sig_dtmfrcv_coef.CMD  = CMD_EOP;
   pSig->fw_sig_dtmfrcv_coef.CHAN = ch;
   pSig->fw_sig_dtmfrcv_coef.MOD  = MOD_RESOURCE;
   pSig->fw_sig_dtmfrcv_coef.ECMD = RES_DTMFR_COEF_ECMD;

   /* DTMF generator default timing configuration (times in ms) */
   pSig->nDtmfInterDigitTime = 100;
   pSig->nDtmfDigitPlayTime  = 100;
}


/**
   Basic DTMF Module configuration.

   \param  pCh          Pointer to the VMMC channel structure.

   \return VMMC_statusOk if no error, otherwise error code.
*/
IFX_int32_t VMMC_SIG_DTMF_BaseConf (VMMC_CHANNEL *pCh)
{
   VMMC_DEVICE  *pDev    = pCh->pParent;
   VMMC_SIGCH_t *pSIG    = pCh->pSIG;
   IFX_int32_t   ret;

   /* DTMF Generator ******************************************************** */

   /* DTMF generator control */
   /* EN = 1, AD = 1, MOD = 0, FGMOD = 1 */
   /* Note : when ET = 1 : MOD = 0 , FG = 1 (ref FW Spec). If done another
      way, it leads to CERR in FW. */
   pSIG->fw_sig_dtmfgen.EN = 0;
   pSIG->fw_sig_dtmfgen.AD = 1;
   pSIG->fw_sig_dtmfgen.MD = 0;
   pSIG->fw_sig_dtmfgen.FG = 1;
   pSIG->fw_sig_dtmfgen.ADD_1 = 0;
   pSIG->fw_sig_dtmfgen.ADD_2 = 1;

   ret = CmdWrite (pDev, (IFX_uint32_t *) &pSIG->fw_sig_dtmfgen,
                   sizeof (pSIG->fw_sig_dtmfgen) - CMD_HDR_CNT);
   if (ret != VMMC_statusOk)
      RETURN_STATUS (ret);

   /* DTMF generator coefficients */
   /* Preset the DTMF generator levels to result in a 2dB twist.
      level(f1) = -11 dB (0x24), level(f2) = -9 dB (0x2D).
      Leave all other values at FW defaults. */
   pSIG->fw_sig_dtmfgen_coef.LEVEL1 = 0x24;
   pSIG->fw_sig_dtmfgen_coef.LEVEL2 = 0x2D;
   pSIG->fw_sig_dtmfgen_coef.TIMT = 0x5A;
   pSIG->fw_sig_dtmfgen_coef.TIMP = 0x5A;

   ret = CmdWrite (pDev, (IFX_uint32_t *) &pSIG->fw_sig_dtmfgen_coef,
                   sizeof(pSIG->fw_sig_dtmfgen_coef) - CMD_HDR_CNT);
   if (ret != VMMC_statusOk)
      RETURN_STATUS (ret);
   if (pDev->err == VMMC_ERR_CERR)
      RETURN_STATUS (VMMC_statusCerr);

   /* DTMF Receiver ********************************************************* */

   if (pSIG->fw_sig_dtmfrcv.EN)
   {
      /* Disable the DTMF receiver before programming the coefficients */
      pSIG->fw_sig_dtmfrcv.EN   = SIG_DTMFR_CTRL_DISABLE;

      ret = CmdWrite (pDev, (IFX_uint32_t *) &pSIG->fw_sig_dtmfrcv,
                      sizeof (pSIG->fw_sig_dtmfrcv) - CMD_HDR_CNT);
      if (ret != VMMC_statusOk)
         RETURN_STATUS (ret);
   }

   /* Write the DTMF receiver coefficients */
   /* Set the default level of the DTMF receiver to -30dB. The firmware default
      is set to -56dB. It is recommended not to use a value below -30dB.
      The "TWIST" value stays unchanged */
   /* Level= -30dB */
   pSIG->fw_sig_dtmfrcv_coef.LEVEL = 0xE2;
   /* Twist= 9.1dB */
   pSIG->fw_sig_dtmfrcv_coef.TWIST = 0x1C;
   /* Gain adjustment: 0dB */
   pSIG->fw_sig_dtmfrcv_coef.GAIN = VMMC_GAIN_0DB;
   pSIG->fw_sig_dtmfrcv_coef.TCMIN = 0x1C;
   pSIG->fw_sig_dtmfrcv_coef.TBMAX = 0x07;
   pSIG->fw_sig_dtmfrcv_coef.TPMIN = 0x1E;
   pSIG->fw_sig_dtmfrcv_coef.FINELEVEL = 0x7F;
   /* Only write the first word with the change an is always supported. */
   ret = CmdWrite (pDev, (IFX_uint32_t *) &pSIG->fw_sig_dtmfrcv_coef,
                   RES_DTMFR_COEF_LEN);
   if (ret != VMMC_statusOk)
      RETURN_STATUS (ret);

   /* Enable the DTMF receiver */
   /* EN = 1, ET = 0, AS = 1 */
   pSIG->fw_sig_dtmfrcv.EN   = SIG_DTMFR_CTRL_ENABLE;
   pSIG->fw_sig_dtmfrcv.ET   = 0;
   pSIG->fw_sig_dtmfrcv.AS   = pCh->pSIG->bAutoSuppression = IFX_TRUE;

   ret = CmdWrite (pDev, (IFX_uint32_t *) &pSIG->fw_sig_dtmfrcv,
                   sizeof (pSIG->fw_sig_dtmfrcv) - CMD_HDR_CNT);
   if (ret != VMMC_statusOk)
      RETURN_STATUS (ret);
   if (pDev->err == VMMC_ERR_CERR)
      RETURN_STATUS (VMMC_statusCerr);

   return VMMC_statusOk;
}


/**
   Configure the upstream signal transmission behaviour of tones detected by
   the DTMF.

   The DTMF signal may be sent out-of-band (OOB), in-band or both.

   If the coder is running and the mode is to be switched to out of band,
   the OOB setting is applied directly. Otherwise it is just stored
   and applied when the encoder or decoder is switched on.

   No message will be sent and no error returned, when the setting has
   not been changed. The driver checks for the cached values.

   \param  pLLChannel Handle to VMMC_CHANNEL structure.
   \param  nOobMode  Mode of the inband and out of band transmission of
                     RFC2883 event packets:
          - IFX_TAPI_PKT_EV_OOB_ALL, DTMF is sent in-band and out-ofband
          - IFX_TAPI_PKT_EV_OOB_ONLY, DTMF is sent only out of band
          - IFX_TAPI_PKT_EV_OOB_NO, DTMF is sent only inband
          - IFX_TAPI_PKT_EV_OOB_DEFAULT, DTMF is sent only out of band
          - IFX_TAPI_PKT_EV_OOB_BLOCK, Block event transmission: neither
            in-band nor out-of-band

   \return
   - VMMC_statusCmdWr Writing the command failed
   - VMMC_statusWrongEvPT If OOB event transmission should be enabled, the
      EventPT must be != 0
   - VMMC_statusFuncParm Wrong parameters passed. This code is returned
     when OOB mode is invalid.
   - VMMC_statusSigNotAct Signaling module is not activated. Internal error.
   - VMMC_statusOk if successful
*/
IFX_int32_t VMMC_TAPI_LL_SIG_DTMFD_OOB (IFX_TAPI_LL_CH_t *pLLChannel,
                                        IFX_TAPI_PKT_EV_OOB_t nOobMode)
{
   VMMC_CHANNEL *pCh  = (VMMC_CHANNEL *) pLLChannel;
   VMMC_DEVICE  *pDev = pCh->pParent;
   SIG_DTMFR_CTRL_t sigDtmfr;
   IFX_int32_t   ret  = IFX_SUCCESS;
   IFX_uint32_t *pCmd, nCount;

   memset (&sigDtmfr, 0, sizeof (sigDtmfr));
   sigDtmfr = pCh->pSIG->fw_sig_dtmfrcv;

   /* Read Signaling Module status */
   if (!(pDev->nDevState & DS_SIG_EN))
   {
      /* errmsg: Signaling module not enabled */
      RETURN_STATUS (VMMC_statusSigModNotEn);
   }

   /* if oob event transmission should be enabled, the EventPT must be != 0 */
   if (nOobMode != IFX_TAPI_PKT_EV_OOB_NO && pCh->nEvtPT == 0)
   {
      /* errmsg: Event payload type mismatch */
      RETURN_STATUS (VMMC_statusWrongEvpt);
   }

   VMMC_OS_MutexGet (&pDev->mtxMemberAcc);

   switch (nOobMode)
   {
      case IFX_TAPI_PKT_EV_OOB_ALL:
         /* enable in-band and out-band transmission */
         pCh->pSIG->bAutoSuppression      = IFX_FALSE;
         pCh->pSIG->et_stat.flag.dtmf_rec = IFX_TRUE;
         break;
      case IFX_TAPI_PKT_EV_OOB_ONLY:
      case IFX_TAPI_PKT_EV_OOB_DEFAULT:
         /* disable in-band and enable out-band transmission */
         pCh->pSIG->bAutoSuppression      = IFX_TRUE;
         pCh->pSIG->et_stat.flag.dtmf_rec = IFX_TRUE;
         break;
      case IFX_TAPI_PKT_EV_OOB_NO:
         /* enable in-band and disable out-band transmission */
         pCh->pSIG->bAutoSuppression      = IFX_FALSE;
         pCh->pSIG->et_stat.flag.dtmf_rec = IFX_FALSE;
         break;
      case IFX_TAPI_PKT_EV_OOB_BLOCK:
         /* disable  in-band and out-band transmission */
         pCh->pSIG->bAutoSuppression      = IFX_TRUE;
         pCh->pSIG->et_stat.flag.dtmf_rec = IFX_FALSE;
         break;
      default:
         VMMC_OS_MutexRelease (&pDev->mtxMemberAcc);
         /* errmsg: At least one parameter is wrong */
         RETURN_STATUS (VMMC_statusParam);
   }

   /* Update ET right here if event support is enabled in the SIG channel.
      In any case the state is kept for the update that happens when the
      SIG channel event support changes. */
   if (pCh->pSIG->fw_sig_ch.ES == 1)
   {
      pCh->pSIG->fw_sig_dtmfrcv.ET = pCh->pSIG->et_stat.flag.dtmf_rec;
   }

   /* auto suppression may only be enabled for local side input */
   if (pCh->pSIG->fw_sig_dtmfrcv.IS == SIG_DTMFR_CTRL_IS_SIGINA)
   {
      pCh->pSIG->fw_sig_dtmfrcv.AS = pCh->pSIG->bAutoSuppression;
   }

   /* if settings have changed - write the fw command */
   if (((sigDtmfr.AS != pCh->pSIG->fw_sig_dtmfrcv.AS) ||
        (sigDtmfr.ET != pCh->pSIG->fw_sig_dtmfrcv.ET)) &&
        (pCh->pSIG->bDtmfdOverride == IFX_FALSE))
   {
      /* write only when AS or ET has changed */
      pCmd = (IFX_uint32_t *) &(pCh->pSIG->fw_sig_dtmfrcv);
      nCount = sizeof (pCh->pSIG->fw_sig_dtmfrcv) - CMD_HDR_CNT;

      ret = CmdWrite (pCh->pParent, pCmd, nCount);
   }

   VMMC_OS_MutexRelease (&pDev->mtxMemberAcc);

   return ret;
}


/**
   Firmware DTMF generator configuration

   Disables DTMF generator during configuration and leave values set to 0xFF
   untouched. See description of DTMF/AT Generator, Coefficients message for
   more details on parameters.

   \param  pCh             Pointer to VMMC channel structure.
   \param  nTIMT           DTMF tone on time,
                           TIM=TIM[ms]*2 with 0ms < TIM < 127ms
   \param  nTIMP           DTMF tone pause,
                           TIM=TIM[ms]*2 with 0ms < TIM < 127ms

   \return
   IFX_SUCCESS or IFX_ERROR
*/
IFX_int32_t VMMC_SIG_DTMFG_CoeffSet(VMMC_CHANNEL *pCh, IFX_uint8_t nTIMT,
                                    IFX_uint8_t nTIMP)
{
   IFX_int32_t       ret, enable;
   VMMC_DEVICE       *pDev = pCh->pParent;

   enable = pCh->pSIG->fw_sig_dtmfgen.EN;

   if (enable)
   {
      /* The DTMF/AT generator needs to be disabled while setting its coeffs. */
      pCh->pSIG->fw_sig_dtmfgen.EN = SIG_DTMFATG_CTRL_DISABLE;
      ret = CmdWrite (pDev, (IFX_uint32_t *) &(pCh->pSIG->fw_sig_dtmfgen),
                      sizeof (pCh->pSIG->fw_sig_dtmfgen) - CMD_HDR_CNT);
      if (!VMMC_SUCCESS(ret))
      {
         RETURN_STATUS(ret);
      }
   }

   /* Only modify time values that are not set to 0xFF */
   if (nTIMT != 0xFF)
      pCh->pSIG->fw_sig_dtmfgen_coef.TIMT = nTIMT;
   if (nTIMP != 0xFF)
      pCh->pSIG->fw_sig_dtmfgen_coef.TIMP = nTIMP;

   ret = CmdWrite(pDev, (IFX_uint32_t *) &pCh->pSIG->fw_sig_dtmfgen_coef,
                  sizeof(pCh->pSIG->fw_sig_dtmfgen_coef) - CMD_HDR_CNT);
   if (!VMMC_SUCCESS(ret))
   {
      RETURN_STATUS(ret);
   }

   if (enable)
   {
      /* Activate generator again if it was enabled when entering here. */
      pCh->pSIG->fw_sig_dtmfgen.EN = SIG_DTMFATG_CTRL_ENABLE;
      ret = CmdWrite (pDev, (IFX_uint32_t *) &pCh->pSIG->fw_sig_dtmfgen,
                      sizeof (pCh->pSIG->fw_sig_dtmfgen) - CMD_HDR_CNT);
   }

   RETURN_STATUS(ret);
}


/**
   Start DTMF generator

   \param  pCh             Pointer to VMMC channel structure.
   \param  pDtmfData       Pointer to the DTMF data to send.
   \param  nDtmfWords      Number of DTMF words to send.
   \param  nFG             Frequency generation mode (0 = low, 1 = high).
   \param  cbDtmfStatus    Callback function on DTMF status change
                           (set to IFX_NULL if unused).
   \param  bByteMode       Format of pDtmfData (0 = 16bit, 1 = 8bit).

   \return
   IFX_SUCCESS/IFX_ERROR

   \remarks
   After triggering the DTMF transmission by calling VMMC_DtmfStart, the
   transmission will be handled automatically. The DTMF data will be sent on
   interrupt request and stopped on end of transmission, error or hook event.
   The callback cbDtmfStatus can be used to track the status of the DTMF
   transmission.

   \remarks
   If bByteMode is set, the driver will convert the IFX_char_t data to
   DTMF words. This Mode only supports restricted DTMF signs 0 to D but
   no alert tones or pause.

   \remarks
   Only supports DTMF generator high level timing mode.

   \remarks
   Note : when ET = 1 : MOD = 0 , FG = 1 (ref FW Spec).
          If done another way, it leads to CERR in FW.
*/
IFX_int32_t VMMC_SIG_DTMFG_Start(VMMC_CHANNEL *pCh, IFX_uint16_t *pDtmfData,
                                 IFX_uint16_t nDtmfWords, IFX_uint32_t nFG,
                                 IFX_void_t (*cbDtmfStatus)(VMMC_CHANNEL *pCh),
                                 IFX_boolean_t bByteMode)
{

   IFX_int32_t    ret = IFX_SUCCESS;
   VMMC_DEVICE    *pDev  = pCh->pParent;
   VMMC_DTMF      *pDtmf = &pCh->pSIG->dtmfSend;
   IFX_uint32_t   *pCmd, nCount;
   SIG_DTMFATG_CTRL_t *pDtmfAtgCmd = &pCh->pSIG->fw_sig_dtmfgen;
   IFX_uint16_t   dataLength;

   /* Prevent simultaneous usage of generator resource */
   if (++pDtmf->useCnt != 1)
   {
      pDtmf->useCnt--;
      return IFX_ERROR;
   }

   /* Store DTMF data in channel structure for non-blocking sending */
   pDtmf->nSent      = 0;
   pDtmf->nCurr      = 0;
   pDtmf->nWords     = nDtmfWords;
   pDtmf->stateCb    = cbDtmfStatus;
   pDtmf->bByteMode  = bByteMode;
   dataLength        = bByteMode ? nDtmfWords : nDtmfWords << 1;

   if (pDtmf->pData != IFX_NULL)
   {
      VMMC_OS_Free (pDtmf->pData);
   }
   if ((pDtmf->pData = VMMC_OS_Malloc (dataLength)) != IFX_NULL)
   {
      memcpy (pDtmf->pData, pDtmfData, dataLength);
   }

   ret = VMMC_SIG_AutoChStop (pCh, IFX_TRUE);

   if (ret == IFX_SUCCESS)
   {
      DTMF_STATUS(DTMF_START);

      if (pDtmfAtgCmd->EN)
      {
         /* Disable the DTMFATG first before modifying the MD and FG bit */
         pDtmfAtgCmd->EN = SIG_DTMFATG_CTRL_DISABLE;

         pCmd = (IFX_uint32_t *) pDtmfAtgCmd;
         nCount = sizeof (SIG_DTMFATG_CTRL_t) - CMD_HDR_CNT;

         ret = CmdWrite(pDev, pCmd, nCount);
      }

      /* Re-enable DTMFATG and modify FG bit */
      pDtmfAtgCmd->EN = SIG_DTMFATG_CTRL_ENABLE;
      pDtmfAtgCmd->AD = 1;
      pDtmfAtgCmd->MD = 1;
      pDtmfAtgCmd->FG = nFG;
      pDtmfAtgCmd->ADD_1 = 1;
      pDtmfAtgCmd->ADD_2 = 1;

      pCmd = (IFX_uint32_t *) pDtmfAtgCmd;
      nCount = sizeof (SIG_DTMFATG_CTRL_t) - CMD_HDR_CNT;

      ret = CmdWrite(pDev, pCmd, nCount);
   }

   return ret;
}


/**
   Handle DTMF generator underrun

   Stop the DTMF/AT generator with immediate effect on underrun. In case
   compiled with DTMF_NO_AUTODEACT a wanted underrun will occur on end of
   transmission.

   \param  pCh             Pointer to VMMC channel structure.

   \return
   IFX_ERROR/IFX_SUCCESS
*/
IFX_int32_t irq_VMMC_SIG_DtmfOnUnderrun(VMMC_CHANNEL *pCh)
{
   IFX_int32_t ret;

   ret = irq_VMMC_SIG_DtmfStop(pCh);
   return ret;
}


/**
   Stops the DTMF/AT generator with immediate effect

   \param  pCh             Pointer to VMMC channel structure.

   \return
   IFX_ERROR/IFX_SUCCESS

   \remarks
   This function does not deactivate the global signaling channel, because
   otherwise the switching function VMMC_SIG_AutoChStop must be protected
   against interrupts. Switching of signaling channel is not a must, but
   only a recommendation. In a typical scenario more sig resources would be
   used anyway, that the switching is not mandatory.
*/
IFX_int32_t irq_VMMC_SIG_DtmfStop(VMMC_CHANNEL *pCh)
{
   IFX_int32_t        ret     = IFX_SUCCESS;
   VMMC_DEVICE       *pDev    = pCh->pParent;
   IFX_uint32_t      *pCmd, nCount;
   SIG_DTMFATG_CTRL_t    *pDtmfAtgCmd = &pCh->pSIG->fw_sig_dtmfgen;

   /* Disable the DTMF generator with immediate effect */
   pDtmfAtgCmd->EN = 0;
   pDtmfAtgCmd->AD = 0;

   pCmd = (IFX_uint32_t *) pDtmfAtgCmd;
   nCount = sizeof (SIG_DTMFATG_CTRL_t) - CMD_HDR_CNT;
   ret = CmdWriteIsr (pDev, pCmd, nCount);

   return ret;
}


/**
   To be called when the DTMF generator has finished.

   \param  pCh             Pointer to VMMC channel structure.

   \return
   IFX_ERROR/IFX_SUCCESS
*/
IFX_int32_t irq_VMMC_SIG_DtmfOnReady(VMMC_CHANNEL *pCh)
{
   IFX_int32_t        ret     = IFX_SUCCESS;
   VMMC_DTMF_STATE    status  = DTMF_READY;
   VMMC_DTMF         *pDtmf   = &pCh->pSIG->dtmfSend;

   status = (pDtmf->nSent == pDtmf->nWords) ? DTMF_READY : DTMF_ABORT;

   if (pDtmf->pData != IFX_NULL)
   {
      /* Free DTMF string */
      VMMC_OS_Free (pDtmf->pData);
      pDtmf->pData  = IFX_NULL;
   }

   pDtmf->nWords = 0;
   pDtmf->nSent  = 0;
   pDtmf->nCurr  = 0;
   pDtmf->useCnt = 0;

   DTMF_STATUS(status);

   return ret;
}


/**
   Send data to DTMF generator

   DTMF data is taken from channel specific structure. Octets are expected to
   contain either DTC of frequency values acc. to DTMF generator mode. See
   description of DTMF/AT Generator, Data message for more details.

   \param  pCh             Pointer to VMMC channel structure.

   \return
   IFX_ERROR/IFX_SUCCESS

   \remarks
   Function to be called from interrupt mode (DTMF/AT generator request) only.
*/
IFX_int32_t irq_VMMC_SIG_DtmfOnRequest(VMMC_CHANNEL *pCh)
{
   IFX_uint16_t    nSend, nLength;
   IFX_uint16_t    *pAtgCmd;
   IFX_int32_t     ret       = IFX_SUCCESS;
   IFX_uint32_t     *pCmd;
   VMMC_DEVICE    *pDev     = pCh->pParent;
   VMMC_DTMF      *pDtmf    = &pCh->pSIG->dtmfSend;
   RES_DTMFATG_DATA_t *pDtmfAtgData = &pCh->pSIG->fw_sig_dtmfatg_data;
   SIG_DTMFATG_CTRL_t *pDtmfAtgCmd = &pCh->pSIG->fw_sig_dtmfgen;
   IFX_uint8_t i;

#ifndef DTMF_NO_AUTODEACT
   IFX_boolean_t   deact    = IFX_FALSE;
#endif /* DTMF_NO_AUTODEACT */

   if (pDtmf->state != DTMF_TRANSMIT)
   {
      DTMF_STATUS(DTMF_TRANSMIT);
   }

   /* Update the number of sent DTMF signs */
   pDtmf->nSent += pDtmf->nCurr;
   pDtmf->nCurr = 0;

#ifdef DTMF_NO_AUTODEACT
   if (pDtmf->nSent == pDtmf->nWords)
   {
      /* We can get one request too much without auto-deactivation... */
      return ret;
   }
#endif /* DTMF_NO_AUTODEACT */

   /* Determine max. number of signs to transmit depending on generator mode */
   if (pDtmfAtgCmd->MD & SIG_DTMFATG_CTRL_MOD_HIGH )
   {
      /* High level timing mode, up to 5 signs in low level frequency mode or
         up to 10 in high level frequency mode */
      if ((pDtmf->nWords - pDtmf->nSent) > DTMF_MAX_MSG_WORDS)
      {
         nSend = DTMF_MAX_MSG_WORDS;
      }
      else
      {
         /* Last data portion... */
         nSend = (pDtmf->nWords - pDtmf->nSent);
#ifndef DTMF_NO_AUTODEACT
         deact = IFX_TRUE;
#endif /* DTMF_NO_AUTODEACT */
      }
   }
   else
   {
      /* Low level timing mode, only supports one sign per command */
      nSend = 1;
   }

   if (nSend)
   {
      /* Get a pointer to the data area which is behind the header of the cmd */
      pAtgCmd = &pDtmfAtgData->FREQ11_DTC1;

      /* Wipe the data area in the command. The size of this area is
         command size - header size. */
      /*lint -e(419) */
      memset (pAtgCmd, 0x00, sizeof(*pDtmfAtgData) - CMD_HDR_CNT);

      /* Fill the data area */
      if (pDtmf->bByteMode == IFX_TRUE)
      {
         for (i = 0; i < nSend; i++)
         {
            pAtgCmd[i] = (IFX_uint16_t)
                        *(((IFX_char_t*)pDtmf->pData) + pDtmf->nSent + i);
         }
      }
      else
      {
        memcpy(pAtgCmd, (pDtmf->pData + pDtmf->nSent), nSend*2);
      }

      /* Length is given in bytes - that is words multiplied by two */
      nLength = nSend << 1;

      pCmd = (IFX_uint32_t*) pDtmfAtgData;

      ret = CmdWriteIsr(pDev, pCmd, nLength);
   }

#ifndef DTMF_NO_AUTODEACT
   if ((ret == IFX_SUCCESS) && (deact == IFX_TRUE))
   {
      pDtmfAtgCmd->EN = SIG_DTMFATG_CTRL_DISABLE;
      pDtmfAtgCmd->AD = 1;

      pCmd = (IFX_uint32_t*)pDtmfAtgCmd;
      ret = CmdWrite(pDev, pCmd, SIG_DTMFATG_CTRL_LEN);
   }
#endif /* DTMF_NO_AUTODEACT */

   if (ret == IFX_SUCCESS)
   {
      pDtmf->nCurr = nSend;
   }
   else
   {
      irq_VMMC_SIG_DtmfStop(pCh);
   }

   return ret;
}


/**
   This function translates the given FW specific DTMF code to the corresponding
   TAPI DTMF coding.

   \param  fwDtmfCode      Digit to encode in FW specific encoding.

   \return Returns the translated DTMF digit in TAPI specific encoding
*/
IFX_uint8_t irq_VMMC_SIG_DTMF_encode_fw2tapi (IFX_uint8_t fwDtmfCode)
{
   if (fwDtmfCode <= 0x0F)
   {
      switch (fwDtmfCode)
      {
      case 0x00:
         return 0x0B;
      case 0x0B:
         return 0x0C;
      default:
         if (fwDtmfCode <= 0x0A)
            return fwDtmfCode;
         else if ((fwDtmfCode >= 0x0C) && (fwDtmfCode <= 0x0F))
            return fwDtmfCode + 0x10;
      }
   }
   return 0x00;
}


/**
   This function converts the given FW specific DTMF code to the corresponding
   ASCII character coding.

   \param  fwDtmfCode      Digit to encode in FW specific encoding.

   \return Returns the translated DTMF digit in ASCII encoding.

   \remarks
   For the VMMC firmware, following caracters are coded as follows:
       '*' = 0x2A (ASCII) = 0x0A (VMMC)
       '#' = 0x23 (ASCII) = 0x0B (VMMC)
       'A' = 0x41 (ASCII) = 0x0C (VMMC)
       'B' = 0x42 (ASCII) = 0x0D (VMMC)
       'C' = 0x43 (ASCII) = 0x0E (VMMC)
       'D' = 0x44 (ASCII) = 0x0F (VMMC)
       '0' - '9'          = 0x00 - 0x09
*/
IFX_char_t irq_VMMC_SIG_DTMF_encode_fw2ascii (IFX_uint8_t fwDtmfCode)
{
   if (fwDtmfCode <= 0x0F)
   {
      switch (fwDtmfCode)
      {
      case 0x0A:
         return '*';
      case 0x0B:
         return '#';
      default:
         if (fwDtmfCode <= 0x09)
            return fwDtmfCode + '0';
         else if ((fwDtmfCode >= 0x0C) && (fwDtmfCode <= 0x0F))
            return (fwDtmfCode - 0x0C) + 'A';
      }
   }
   return '0';
}


/**
   This function converts the given TAPI DTMF code to the device DTMF code
   This function is used by the DTMF CID implementation to store the device
   DTMF codes in the playout buffer.

   For the VoFW following caracters are coded as follows:
       '*' = 0x2A (ASCII) = 0x0A
       '#' = 0x23 (ASCII) = 0x0B
       'A' = 0x41 (ASCII) = 0x0C
       'B' = 0x42 (ASCII) = 0x0D
       'C' = 0x43 (ASCII) = 0x0E
       'D' = 0x44 (ASCII) = 0x0F
       '0' ..  '9' to appropriate values 0 .. 9

   \param  nChar        Digit to encode.
   \param  pDtmfCode    Pointer to return DTMF code.

   \return
   - VMMC_statusCmdWr Writing the command failed
   - VMMC_statusFuncParm Wrong parameter passed. This code is returned
     when the given character is out of range. See remarks for valid
     parameters.
   - VMMC_statusOk if successful
*/
IFX_return_t VMMC_SIG_DTMF_encode_ascii2fw (IFX_char_t nChar,
                                            IFX_uint8_t *pDtmfCode)
{
   IFX_return_t ret;

   if (pDtmfCode == IFX_NULL)
      return IFX_ERROR;

   ret = IFX_SUCCESS;

   switch (nChar)
   {
   case '*':
      *pDtmfCode = 0x0A;
      break;
   case '#':
      *pDtmfCode = 0x0B;
      break;
   case 'A':
      *pDtmfCode = 0x0C;
      break;
   case 'B':
      *pDtmfCode = 0x0D;
      break;
   case 'C':
      *pDtmfCode = 0x0E;
      break;
   case 'D':
      *pDtmfCode = 0x0F;
      break;
   default:
      if ((nChar >= '0') && (nChar <= '9'))
         *pDtmfCode = (IFX_uint8_t)nChar - '0';
      else
         ret = IFX_ERROR;
      break;
   }

   return ret;
}


/**
   Disable or Enable the DTMF receiver

   \param  pCh             Pointer to VMMC channel structure.
   \param  bEn             - IFX_TRUE : enable
                           - IFX_FALSE : disable.
   \param  dir             - VMMC_SIG_TX: local tx
                           - VMMC_SIG_RX: remote rx.
   \param  enableEndEvent  - IFX_TRUE : enable sending of DTMF end event
                           - IFX_FALSE : disable sending of DTMF end event.

   \return
      IFX_SUCCESS or IFX_ERROR
*/
IFX_int32_t vmmc_sig_DTMFD_Set (VMMC_CHANNEL const *pCh, IFX_boolean_t bEn,
                                IFX_uint8_t dir, IFX_boolean_t enableEndEvent)
{
   IFX_int32_t       ret = IFX_SUCCESS;
   VMMC_DEVICE      *pDev = pCh->pParent;
   SIG_DTMFR_CTRL_t *pDtmfrCtrl = &pCh->pSIG->fw_sig_dtmfrcv;
   IFX_uint32_t     *pCmd, nCount;
   IFX_uint8_t       bOldEn = pDtmfrCtrl->EN;
   IFX_uint8_t       bOldIs = pDtmfrCtrl->IS;
   IFX_uint8_t       bOldEE = pDtmfrCtrl->EE;
   IFX_uint32_t      nNewIS;

   nNewIS = (dir == VMMC_SIG_TX) ?
               SIG_DTMFR_CTRL_IS_SIGINA : SIG_DTMFR_CTRL_IS_SIGINB;

   /* disable the detector if the configuration changes the IS bit */
   if ((pDtmfrCtrl->EN == 1) && (bEn == IFX_TRUE) &&
       (pDtmfrCtrl->IS !=  nNewIS) && (pCh->pSIG->bDtmfdOverride == IFX_FALSE))
   {
      pDtmfrCtrl->EN = SIG_DTMFR_CTRL_DISABLE;
      pCmd = (IFX_uint32_t *) pDtmfrCtrl;
      nCount = sizeof (SIG_DTMFR_CTRL_t) - CMD_HDR_CNT;
      ret = CmdWrite (pDev, pCmd, nCount);
   }

   /* Set enable bit according to parameter */
   pDtmfrCtrl->EN = bEn ? SIG_DTMFR_CTRL_ENABLE : SIG_DTMFR_CTRL_DISABLE;

   if (bEn)
   {
      /* set these parameters only if enabling the detector */

      if (pDev->caps.bEventMailboxSupported)
      {
         /* Set event end bit according to parameter */
         pDtmfrCtrl->EE = (enableEndEvent == IFX_TRUE) ? 1 : 0;
      }

      /* Set input select bit according to parameter */
      pDtmfrCtrl->IS = nNewIS;

      /* Activate the voice path delay if a value is given. */
      pDtmfrCtrl->DEL = (pDtmfrCtrl->DELAY > 0) ? 1 : 0;

      /* Auto suppression is cached in the channel because in the FW message
         it can be overwritten depending on the signal input selected. */
      pDtmfrCtrl->AS = pCh->pSIG->bAutoSuppression;

      /* for network side signal the DTMF detector does not support
         auto suppression or delay of the voice path signal. */
      if (nNewIS == SIG_DTMFR_CTRL_IS_SIGINB)
      {
         pDtmfrCtrl->AS = 0;
         pDtmfrCtrl->DEL = 0;
      }
   }

   /* write DTMF configuration if EN, IS or EE has changed */
   if (((bOldEn != pDtmfrCtrl->EN) ||
        (bOldIs != pDtmfrCtrl->IS) ||
        (bOldEE != pDtmfrCtrl->EE)) &&
        (pCh->pSIG->bDtmfdOverride == IFX_FALSE))
   {
      pCmd = (IFX_uint32_t *) pDtmfrCtrl;
      nCount = sizeof (SIG_DTMFR_CTRL_t) - CMD_HDR_CNT;
      ret = CmdWrite (pDev, pCmd, nCount);
   }

   return ret;
}


/**
   Disable or Enable the DTMF receiver

   \param  pCh             Pointer to VMMC channel structure.
   \param  bEn             - IFX_TRUE : enable
                           - IFX_FALSE : disable.
   \param  dir             - VMMC_SIG_TX: local tx
                           - VMMC_SIG_RX: remote rx.

   \return
      IFX_SUCCESS or IFX_ERROR
*/
IFX_int32_t VMMC_SIG_DTMFD_Set (VMMC_CHANNEL const *pCh,
                                IFX_boolean_t bEn, IFX_uint8_t dir)
{
   SIG_DTMFR_CTRL_t *pDtmfrCtrl = &pCh->pSIG->fw_sig_dtmfrcv;


   return vmmc_sig_DTMFD_Set (pCh, bEn, dir,
                              (pDtmfrCtrl->EE == 1) ? IFX_TRUE : IFX_FALSE);
}


/** Start the DTMF tone detector

   \param pLLChannel Pointer to the VMMC channel structure.
   \param pCfg       Pointer to the DTMF detector configuration

   \return
   - VMMC_statusInvalCh No DTMF tone detector on this channel
   - VMMC_statusCmdWr Writing the command failed
   - VMMC_statusOk if successful
*/
IFX_int32_t  VMMC_TAPI_LL_SIG_DTMFD_Start (IFX_TAPI_LL_CH_t *pLLChannel,
                                           IFX_TAPI_LL_DTMFD_CFG_t const *pCfg)
{
   VMMC_CHANNEL *pCh  = (VMMC_CHANNEL *) pLLChannel;
   IFX_int32_t ret;
   IFX_int32_t dir;

   /* abort if channel contains no DTMF detector resource */
   if (pCh->nChannel > pCh->pParent->caps.nDTMFD)
   {
      /* errmsg: Resource not valid. Channel number out of range */
      RETURN_STATUS (VMMC_statusInvalCh);
   }

   switch (pCfg->direction)
   {
      case IFX_TAPI_LL_DTMFD_DIR_EXTERNAL:
         switch (pCfg->nMod)
         {
            case IFX_TAPI_MODULE_TYPE_COD:
               dir = VMMC_SIG_RX /* network */;
               break;
            case IFX_TAPI_MODULE_TYPE_ALM:
               dir = VMMC_SIG_TX /* local */;
               break;
            default:
               RETURN_STATUS (VMMC_statusSigDtmfBadInputSelect);
         }
         break;
      case IFX_TAPI_LL_DTMFD_DIR_INTERNAL:
         switch (pCfg->nMod)
         {
            case IFX_TAPI_MODULE_TYPE_COD:
               dir = VMMC_SIG_TX /* local */;
               break;
            case IFX_TAPI_MODULE_TYPE_ALM:
               dir = VMMC_SIG_RX /* network */;
               break;
            default:
               RETURN_STATUS (VMMC_statusSigDtmfBadInputSelect);
         }
         break;
      default:
         RETURN_STATUS (VMMC_statusSigDtmfBadInputSelect);
   }

   ret = VMMC_SIG_AutoChStop (pCh, IFX_TRUE);
   if (VMMC_SUCCESS(ret))
   {
      ret = vmmc_sig_DTMFD_Set (pCh, IFX_TRUE, dir, pCfg->bEndEvent);
   }

   RETURN_STATUS (ret);
}


/** Stop the DTMF tone detector

   \param pLLChannel Pointer to the VMMC channel structure.
   \param pCfg       Pointer to the DTMF detector configuration

   \return
   - VMMC_statusInvalCh No DTMF tone detector on this channel
   - VMMC_statusCmdWr Writing the command failed
   - VMMC_statusOk if successful
*/
IFX_int32_t  VMMC_TAPI_LL_SIG_DTMFD_Stop (IFX_TAPI_LL_CH_t *pLLChannel,
                                          IFX_TAPI_LL_DTMFD_CFG_t const *pCfg)
{
   VMMC_CHANNEL *pCh  = (VMMC_CHANNEL *) pLLChannel;
   IFX_int32_t  ret;

   VMMC_UNUSED(pCfg);

   /* abort if channel contains no DTMF detector resource */
   if (pCh->nChannel > pCh->pParent->caps.nDTMFD)
   {
      /* errmsg: Resource not valid. Channel number out of range */
      RETURN_STATUS (VMMC_statusInvalCh);
   }

   ret = VMMC_SIG_DTMFD_Set (pCh, IFX_FALSE, 0);

   if (VMMC_SUCCESS(ret))
   {
      ret = VMMC_SIG_AutoChStop (pCh, IFX_FALSE);
   }

   RETURN_STATUS (ret);
}


/** Override control of the DTMF tone detector

   \param pLLChannel    Pointer to the VMMC channel structure.
   \param pCfg          Pointer to the DTMF detector configuration

   \return
   - VMMC_statusInvalCh No DTMF tone detector on this channel
   - VMMC_statusCmdWr Writing the command failed
   - VMMC_statusOk if successful
*/
IFX_int32_t  VMMC_TAPI_LL_SIG_DTMFD_Override (IFX_TAPI_LL_CH_t *pLLChannel,
   IFX_TAPI_LL_DTMFD_OVERRIDE_t const *pCfg)
{
   VMMC_CHANNEL *pCh  = (VMMC_CHANNEL *) pLLChannel;
   VMMC_DEVICE  *pDev = pCh->pParent;
   IFX_int32_t   ret = VMMC_statusOk;

   /* abort if channel contains no DTMF detector resource */
   if (pCh->nChannel > pCh->pParent->caps.nDTMFD)
   {
      return VMMC_statusInvalCh;
   }

   /* do nothing if state is already set */
   if (pCh->pSIG->bDtmfdOverride == pCfg->bOverride)
   {
      return VMMC_statusOk;
   }

   if (pCfg->bOverride == IFX_FALSE)
   {
      /* Normal mode */
      SIG_DTMFR_CTRL_t *pDtmfrCtrl = &pCh->pSIG->fw_sig_dtmfrcv;
      SIG_DTMFR_CTRL_t *pDtmfrCtrlOverride
                                   = &pCh->pSIG->fw_sig_dtmfrcv_override;

      /* If during override mode the DTMF rx coeffs were modified return to
         normal coefficients now. */
      if (pCh->pSIG->bDtmfdCoeffModified != IFX_FALSE)
      {
         RES_DTMFR_COEF_t *pDtmfRxCoeff = &pCh->pSIG->fw_sig_dtmfrcv_coef;

         /* Having written the coefficients means that also the DTMF receiver
            was activated afterwards. It must be stopped before coefficients
            can be written. */
         pDtmfrCtrlOverride->EN = SIG_DTMFR_CTRL_DISABLE;
         ret = CmdWrite (pDev, (IFX_uint32_t *) pDtmfrCtrlOverride,
                         sizeof (*pDtmfrCtrlOverride) - CMD_HDR_CNT);

         if (ret == VMMC_statusOk)
         {
            ret = CmdWrite (pDev, (IFX_uint32_t *) pDtmfRxCoeff,
                            sizeof (*pDtmfRxCoeff) - CMD_HDR_CNT);
            pCh->pSIG->bDtmfdCoeffModified = IFX_FALSE;
         }
      }

      if (ret == VMMC_statusOk)
      {
         ret = CmdWrite (pDev, (IFX_uint32_t *) pDtmfrCtrl,
                         sizeof (*pDtmfrCtrl) - CMD_HDR_CNT);
      }
   }
   else
   {
      /* Override mode */
      SIG_DTMFR_CTRL_t *pDtmfrCtrlNormal = &pCh->pSIG->fw_sig_dtmfrcv;
      SIG_DTMFR_CTRL_t *pDtmfrCtrl = &pCh->pSIG->fw_sig_dtmfrcv_override;

      IFX_int32_t nInput = SIG_DTMFR_CTRL_IS_SIGINA /* local */;
      if (IFX_TAPI_MODULE_TYPE_COD == pCfg->nMod)
      {
         if (IFX_TAPI_LL_DTMFD_DIR_EXTERNAL == pCfg->direction)
         {
            nInput = SIG_DTMFR_CTRL_IS_SIGINB /* network */;
         }
      }
      else if (IFX_TAPI_MODULE_TYPE_ALM == pCfg->nMod)
      {
         if (IFX_TAPI_LL_DTMFD_DIR_INTERNAL == pCfg->direction)
         {
            nInput = SIG_DTMFR_CTRL_IS_SIGINB /* network */;
         }
      }
      else
      {
         RETURN_STATUS (VMMC_statusSigDtmfBadInputSelect);
      }

      /* Set enable bit according to parameter */
      pDtmfrCtrl->EN = (IFX_ENABLE == pCfg->nOperation) ?
                       SIG_DTMFR_CTRL_ENABLE : SIG_DTMFR_CTRL_DISABLE;

      /* Set input select bit according to parameter */
      pDtmfrCtrl->IS = nInput;

      if (pDtmfrCtrl->EN)
      {
         /* DTMF should be turned on -> make sure the SIG module is active */
         ret = VMMC_SIG_AutoChStop (pCh, IFX_TRUE);

         /* Before turning on modify TCMIN in the DTMF RX coefficients.
            But only if the parameter is supported by this firmware. */
         if (pDev->caps.bDT2 == 1)
         {
            if ((ret == VMMC_statusOk) && (pDtmfrCtrlNormal->EN == 1))
            {
               /* In normal mode the receiver was running. It must be
                  deactivated before the coefficients can be written. */
               pDtmfrCtrlNormal->EN = SIG_DTMFR_CTRL_DISABLE;
               ret = CmdWrite (pDev, (IFX_uint32_t *) pDtmfrCtrlNormal,
                               sizeof (*pDtmfrCtrlNormal) - CMD_HDR_CNT);
               pDtmfrCtrlNormal->EN = SIG_DTMFR_CTRL_ENABLE;
            }

            if (ret == VMMC_statusOk)
            {
               RES_DTMFR_COEF_t *pDtmfRxCoeff = &pCh->pSIG->fw_sig_dtmfrcv_coef;

               /* reduce minimum burst duration by 3 ms */
               pDtmfRxCoeff->TCMIN -= 3 /* ms */;
               ret = CmdWrite (pDev, (IFX_uint32_t *) pDtmfRxCoeff,
                               sizeof (*pDtmfRxCoeff) - CMD_HDR_CNT);
               /* return to the previous value */
               pDtmfRxCoeff->TCMIN += 3 /* ms */;
               /* Remember to reprogram coefficients in normal mode. */
               pCh->pSIG->bDtmfdCoeffModified = IFX_TRUE;
            }
         }
      }

      if (ret == VMMC_statusOk)
      {
         /* Write override command */
         ret = CmdWrite (pDev, (IFX_uint32_t *) pDtmfrCtrl,
                         sizeof (*pDtmfrCtrl) - CMD_HDR_CNT);
      }
   }

   /* Remember the state for some checks. */
   pCh->pSIG->bDtmfdOverride = pCfg->bOverride;

   RETURN_STATUS (ret);
}


/**
   Configure the DTMF tone generator

   \param  pLLChannel      Pointer to the VMMC channel structure.
   \param  nInterDigitTime Inter-digit-time in ms.
   \param  nDigitPlayTime  Active digit-play-time in ms.

   \return
   - VMMC_statusFuncParm Wrong parameters passed. This is either
      nInterDigitTime or nDigitPlayTime is greater than 127
   - VMMC_statusOk if successful
*/
IFX_int32_t VMMC_TAPI_LL_SIG_DTMFG_Cfg (IFX_TAPI_LL_CH_t *pLLChannel,
                                       IFX_uint16_t nInterDigitTime,
                                       IFX_uint16_t nDigitPlayTime)
{
   VMMC_CHANNEL *pCh  = (VMMC_CHANNEL *) pLLChannel;

   if (nInterDigitTime > 127 || nDigitPlayTime > 127 )
   {
      TRACE (VMMC, DBG_LEVEL_HIGH,
             ("DTMF generator timing coefficients too big (max 127ms allowed)\n"));
      /* errmsg: Parameter is out of range */
      RETURN_STATUS (VMMC_statusFuncParm);
   }

   pCh->pSIG->nDtmfInterDigitTime = nInterDigitTime;
   pCh->pSIG->nDtmfDigitPlayTime  = nDigitPlayTime;

   return VMMC_statusOk;
}


/**
   Start the DTMF tone generator

   \param  pLLChannel   Pointer to the VMMC channel structure.
   \param  nDigits      Number of digits in the data string to be sent.
   \param  pData        String with the digits (ascii 0-9 A-D) to be sent.

   \return
   - VMMC_statusCmdWr Writing the command failed
   - VMMC_statusOk if successful
*/
IFX_int32_t VMMC_TAPI_LL_SIG_DTMFG_Start (IFX_TAPI_LL_CH_t *pLLChannel,
                                          IFX_uint8_t nDigits,
                                          IFX_char_t const *pData)
{
   VMMC_CHANNEL *pCh  = (VMMC_CHANNEL *) pLLChannel;
   IFX_int32_t ret;
   int i;
   IFX_uint8_t *data = IFX_NULL;

   /* abort if channel contains no DTMF generator resource */
   if (pCh->nChannel > pCh->pParent->caps.nDTMFG)
      return IFX_ERROR;

   /* prevent starting the generator while it is already running */
   if (pCh->pSIG->dtmfSend.useCnt != 0)
      /**\todo Error handling here  */
      return IFX_ERROR;

   if (nDigits <= 0)
      /**\todo Error handling here  */
      return IFX_ERROR;

   /* allocate memory for local data transformation */
   data = VMMC_OS_Malloc (sizeof (IFX_uint8_t) * nDigits);
   memset (data, 0, sizeof (IFX_uint8_t) * nDigits);

   /* arguments to this function are in half ms steps so multiply values by 2 */
   ret = VMMC_SIG_DTMFG_CoeffSet(pCh, pCh->pSIG->nDtmfDigitPlayTime << 1,
                                      pCh->pSIG->nDtmfInterDigitTime << 1);

   /* transcode Characters A-D, # and * to FW specific setting */
   /* errors may occur if input string contains invalid characters */
   for (i=0; ret == IFX_SUCCESS && i<nDigits; i++)
      ret = VMMC_SIG_DTMF_encode_ascii2fw(pData[i], data+i);

   /* arguments to the DTMF generator is a byte string with ASCII encoded
      digits and special characters (high level frequency generation) */
   if ( ret == IFX_SUCCESS )
   {
      /*lint -e{826} yes casting of nDigits is correct */
      ret = VMMC_SIG_DTMFG_Start(pCh, (IFX_uint16_t *)data,
                                      (IFX_uint16_t)nDigits, 1, NULL, IFX_TRUE);
   }

   /* cleanup local memory */
   VMMC_OS_Free (data);

   RETURN_STATUS (ret);
}


/**
   Stop the DTMF tone generator

   \param  pLLChannel   Pointer to the VMMC channel structure.

   \return
   - VMMC_statusCmdWr Writing the command failed
   - VMMC_statusOk if successful
*/
IFX_int32_t VMMC_TAPI_LL_SIG_DTMFG_Stop (IFX_TAPI_LL_CH_t *pLLChannel)
{
   VMMC_CHANNEL *pCh  = (VMMC_CHANNEL *) pLLChannel;

   /* abort if channel contains no DTMF generator resource */
   if (pCh->nChannel > pCh->pParent->caps.nDTMFG)
      return IFX_ERROR;

   return irq_VMMC_SIG_DtmfStop(pCh);
}


/**
   Sets/Gets DTMF Receiver Coefficients

   \param pLLChannel    Handle to TAPI low level channel structure.
   \param bRW           IFX_FALSE to read, IFX_TRUE to write coefficients.
   \param pCoeff        Pointer to DTMF Rx coefficients data structure,
                        to read from or write to.
   \return
   - IFX_SUCCESS on successful read/write coefficients
   - IFX_ERROR on error

   \remarks
   Setting of the DTMF coefficients is only allowed while the DTMF receiver
   is disabled. As a result, if setting of the coefficients is attempted while
   the DTMF receiver is enabled, it will be disabled temporarily in order to
   write the coefficients, and reenabled again.
*/
IFX_return_t VMMC_TAPI_LL_SIG_DTMF_RX_CFG (IFX_TAPI_LL_CH_t *pLLChannel,
                                           IFX_boolean_t bRW,
                                           IFX_TAPI_DTMF_RX_CFG_t *pCoeff)
{
   IFX_int32_t ret      = IFX_SUCCESS;
   VMMC_CHANNEL *pCh    = (VMMC_CHANNEL *) pLLChannel;
   VMMC_DEVICE  *pDev   = pCh->pParent;
   SIG_DTMFR_CTRL_t *pDtmfRxCtrl;
   RES_DTMFR_COEF_t *pDtmfRxCoeff;
   IFX_uint32_t nCount;

   /* abort if channel contains no DTMF receiver resource */
   if (pCh->nChannel > pDev->caps.nDTMFD)
      return IFX_ERROR;

   pDtmfRxCtrl =  &pCh->pSIG->fw_sig_dtmfrcv;
   pDtmfRxCoeff = &pCh->pSIG->fw_sig_dtmfrcv_coef;

   if (bRW == IFX_FALSE)
   {
      /* Write DTMF receiver coefficients */

      /* Enforce valid range of level parameter */
      if (pCoeff->nLevel < VMMC_DTMF_RX_LEVEL_MIN)
      {
         pCoeff->nLevel = VMMC_DTMF_RX_LEVEL_MIN;
         TRACE (VMMC, DBG_LEVEL_HIGH, ("\nDRV_WARN: Limited the "
                "DTMF receiver level to %d dB\n",VMMC_DTMF_RX_LEVEL_MIN));
      }
      if (pCoeff->nLevel > VMMC_DTMF_RX_LEVEL_MAX)
      {
         pCoeff->nLevel = VMMC_DTMF_RX_LEVEL_MAX;
         TRACE (VMMC, DBG_LEVEL_HIGH, ("\nDRV_WARN: Limited the "
                "DTMF receiver level to %d dB\n",VMMC_DTMF_RX_LEVEL_MAX));
      }

      /* Enforce valid range of twist parameter */
      if (pCoeff->nTwist < VMMC_DTMF_RX_TWIST_MIN)
      {
         pCoeff->nTwist = VMMC_DTMF_RX_TWIST_MIN;
         TRACE (VMMC, DBG_LEVEL_HIGH, ("\nDRV_WARN: Limited the "
                "DTMF receiver twist to %d dB\n",VMMC_DTMF_RX_TWIST_MIN));
      }
      if (pCoeff->nTwist > VMMC_DTMF_RX_TWIST_MAX)
      {
         pCoeff->nTwist = VMMC_DTMF_RX_TWIST_MAX;
         TRACE (VMMC, DBG_LEVEL_HIGH, ("\nDRV_WARN: Limited the "
                "DTMF receiver twist to %d dB\n",VMMC_DTMF_RX_TWIST_MAX));
      }

      /* Enforce valid range of gain parameter */
      if (pCoeff->nGain < VMMC_DTMF_RX_GAIN_MIN)
      {
         pCoeff->nGain = VMMC_DTMF_RX_GAIN_MIN;
         TRACE (VMMC, DBG_LEVEL_HIGH, ("\nDRV_WARN: Limited the "
                "DTMF receiver gain to %d dB\n", VMMC_DTMF_RX_GAIN_MIN));
      }
      if (pCoeff->nGain > VMMC_DTMF_RX_GAIN_MAX)
      {
         pCoeff->nGain = VMMC_DTMF_RX_GAIN_MAX;
         TRACE (VMMC, DBG_LEVEL_HIGH, ("\nDRV_WARN: Limited the "
                "DTMF receiver gain to %d dB\n", VMMC_DTMF_RX_GAIN_MAX));
      }

      /* Enforce valid range of delay parameter */
      if (pCoeff->nVoicePathDelay > VMMC_DTMF_RX_DELAY_MAX)
      {
         pCoeff->nVoicePathDelay = VMMC_DTMF_RX_DELAY_MAX;
         TRACE (VMMC, DBG_LEVEL_HIGH, ("\nDRV_WARN: Limited the "
                "DTMF voice path delay to %d ms\n", VMMC_DTMF_RX_DELAY_MAX));
      }

      if (pDtmfRxCtrl->EN)
      {
         /* DTMF Receiver is enabled - however, it must be disabled before
            writing the DTMF Receiver Coefficients */
         pDtmfRxCtrl->EN = SIG_DTMFR_CTRL_DISABLE;

         nCount = sizeof(SIG_DTMFR_CTRL_t) - CMD_HDR_CNT;
         ret = CmdWrite (pDev, (IFX_uint32_t *)pDtmfRxCtrl, nCount);
         if (ret != IFX_SUCCESS)
         {
            goto error;
         }

         pDtmfRxCtrl->EN = SIG_DTMFR_CTRL_ENABLE;
      }

      /* Convert LEVEL value to 2's complement */
      pDtmfRxCoeff->LEVEL = (IFX_uint8_t)pCoeff->nLevel;

      /* Lookup TWIST coefficient value */
      pDtmfRxCoeff->TWIST = VMMC_DtmfRxTwist[pCoeff->nTwist + /*lint --e(778)*/
                                             (-VMMC_DTMF_RX_TWIST_MIN)];

      /* Lookup GAIN coefficient value */
      pDtmfRxCoeff->GAIN  = VMMC_Gaintable[pCoeff->nGain +
                                           (-VMMC_DTMF_RX_GAIN_MIN)];

      /* Length covers only the basic values */
      nCount = RES_DTMFR_COEF_LEN;
      /* Write the DTMF Receiver Coefficients */
      ret = CmdWrite (pDev, (IFX_uint32_t *)pDtmfRxCoeff, nCount);
      if (ret != IFX_SUCCESS)
      {
         goto error;
      }

      /* Set the voice path delay value in the control message */
      pDtmfRxCtrl->DELAY = pCoeff->nVoicePathDelay;
      /* Activate the delay if a value is given and local side input is
         selected. For network side signal the DTMF detector does not support
         the delay of the voice path signal. */
      if ( (pCoeff->nVoicePathDelay > 0) &&
           (pDtmfRxCtrl->IS == SIG_DTMFR_CTRL_IS_SIGINA) )
      {
         pDtmfRxCtrl->DEL = 1;
      }
      else
      {
         pDtmfRxCtrl->DEL = 0;
      }

      if (pDtmfRxCtrl->EN)
      {
         /* Reenable the DTMF Receiver, as it was initially enabled */
         nCount = sizeof(SIG_DTMFR_CTRL_t) - CMD_HDR_CNT;

         ret = CmdWrite (pDev, (IFX_uint32_t *)pDtmfRxCtrl, nCount);
         if (ret != IFX_SUCCESS)
         {
            goto error;
         }
      }
   }
   else
   {
      IFX_uint32_t i, nTableLen;

      /* Read DTMF receiver coefficients */

      /* Length covers only the basic values */
      nCount = RES_DTMFR_COEF_LEN;

      ret = CmdRead (pDev, (IFX_uint32_t *)pDtmfRxCoeff,
                           (IFX_uint32_t *)pDtmfRxCoeff, nCount);

      if (ret != IFX_SUCCESS)
      {
         goto error;
      }

      /* convert LEVEL to signed number */
      pCoeff->nLevel = (IFX_int8_t)pDtmfRxCoeff->LEVEL;

      /*
         Lookup TWIST dB value
      */
      nTableLen = sizeof(VMMC_DtmfRxTwist) / sizeof(IFX_uint8_t);
      for (i = 0; i < nTableLen; i++)
      {
         if ((i == 0) &&
             (pDtmfRxCoeff->TWIST > VMMC_DtmfRxTwist[0]))
            break;
         if ((pDtmfRxCoeff->TWIST == VMMC_DtmfRxTwist[i]) ||
             ((pDtmfRxCoeff->TWIST < VMMC_DtmfRxTwist[i]) &&
             (i < nTableLen - 1) &&
             (pDtmfRxCoeff->TWIST > VMMC_DtmfRxTwist[i+1])))
            break;
      }
      pCoeff->nTwist = VMMC_DTMF_RX_TWIST_MIN + i;

      /*
         Lookup GAIN dB value
      */
      nTableLen = sizeof(VMMC_Gaintable) / sizeof(IFX_uint16_t);
      for (i = 0; i < nTableLen; i++)
      {
         if (pDtmfRxCoeff->GAIN == VMMC_Gaintable[i])
         {
            break;
         }
      }
      if ( i >= nTableLen)
      {
         /* we did not find the exact value - return error and max value */
         i = nTableLen-1;
         ret = IFX_ERROR;
      }
      pCoeff->nGain = VMMC_DTMF_RX_GAIN_MIN + (IFX_int32_t)i;

      /* Get the voice path delay from the cached control message */
      pCoeff->nVoicePathDelay = pDtmfRxCtrl->DELAY;
   }

error:
   TRACE (VMMC, DBG_LEVEL_NORMAL,
          ("DTMF receiver coefficients %s (%s): "
           "LEVEL=%02x(%ddB), TWIST=%02x(%ddB), GAIN=0x%04x(%ddB)\n",
          bRW == IFX_FALSE ? "written" : "read",
          ret == IFX_SUCCESS ? "success" : "error",
          pDtmfRxCoeff->LEVEL, pCoeff->nLevel,
          pDtmfRxCoeff->TWIST, pCoeff->nTwist,
          pDtmfRxCoeff->GAIN,  pCoeff->nGain));

   return  (IFX_SUCCESS == ret) ? IFX_SUCCESS : IFX_ERROR;
}
