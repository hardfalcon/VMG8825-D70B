/******************************************************************************

                              Copyright (c) 2012
                            Lantiq Deutschland GmbH
                             http://www.lantiq.com

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/**
   \file drv_vmmc_sig_cptd.c
   Definitions of the functions for Call Progress Tone Detection module.
*/

#include "drv_api.h"
#include "drv_vmmc_sig.h"
#include "drv_vmmc_sig_priv.h"
#include "drv_vmmc_api.h"
#include "drv_vmmc_res_priv.h"

/* ============================= */
/* Local Macros & Definitions    */
/* ============================= */

/* default detection level tolerance, -20dB */
#define CPTD_DETECTION_LEVEL_TOLERANCE  200
/* allow -42dB total power in a pause step */
#define CPTCOEFF_POW_PAUSE       0x0002
/* allow -10dB maximum frequency power to total power ratio for valid tone
   detection */
#define CPTCOEFF_FP_TP_R_DEFAULT 0x075D

/*lint --esym( 750, CONST_CPT_HAMMING, CONST_CPT_BLACKMAN)
   One of both remains unused. */
/** hamming constant for cpt level calculation */
#define CONST_CPT_HAMMING        (  610)
/** blackman constant for cpt level calculation */
#define CONST_CPT_BLACKMAN       (-1580)
/** define which constant is taken for cpt level calculation */
#define CONST_CPT                CONST_CPT_BLACKMAN
/* return the maximum of two scalar values */
#define MAX(x,y) ((x) > (y) ? (x) : (y))
/* return the minimum of two scalar values */
#define MIN(x,y) ((x) < (y) ? (x) : (y))

/** CPTD resource not set */
#define CPTD_RES_NOT_SET            0xFF

/** no tone index available */
#define VMMC_SIG_CPTD_NO_TONE_INDEX       0

/* ============================= */
/* Local function declaration    */
/* ============================= */

static IFX_int32_t vmmc_sig_CPTD_ConfEnable (VMMC_CHANNEL *pCh,
                                             IFX_TAPI_TONE_SIMPLE_t const *pTone,
                                             IFX_TAPI_TONE_CPTD_t const *pSignal,
                                             SIG_CPTD_CTRL_t *pCmd,
                                             DSP_CPTD_FL_t frameLength,
                                             IFX_uint8_t nResNr);
static IFX_int32_t vmmc_sig_CPTD_ConfDisable (VMMC_CHANNEL *pCh,
                                              SIG_CPTD_CTRL_t *pCmd,
                                              IFX_uint8_t nResNr,
                                              IFX_boolean_t bResFree);
static IFX_int32_t vmmc_sig_CPTD_SetCoeff (IFX_TAPI_TONE_SIMPLE_t const *pTone,
                                           RES_CPTD_COEF_t *pCmd,
                                           DSP_CPTD_FL_t *pFrameLength);

static IFX_int16_t vmmc_sig_CPTD_CalcFreq (IFX_uint32_t f);
static IFX_int16_t vmmc_sig_CPTD_CalcLevel (IFX_int32_t level);

/* ============================= */
/* Local function definitions    */
/* ============================= */

/**
   function to calculate the cpt frequency coefficients

   \param f - frequency in [Hz]
   \return
      cpt frequency coefficient
   \remarks
      none
*/
static IFX_int16_t vmmc_sig_CPTD_CalcFreq (IFX_uint32_t f)
{
   IFX_int16_t coef = 0;

   /*
      The formula to calculate the cpt frequency is f_coef = 2 * cos (
      2*pi*f[Hz] / 8000) * 2^14 = 2^15 * cos (pi*f[Hz] / 4000)

      Symmetrie: cos (phi) = sin (pi/2 - phi) --> we will use the taylor
      approximation for sin in the interval [pi/4 .. 3/4pi]

      with X = pi*f/4000

      Approximation with Taylor for cos in [-pi/4 .. +pi/4] = 2^15 * ( 1 - (X^2
      /2) + (X^4 /24) ) = 2^15 - (2^15 * X^2 / 2) + (2^15 * X^4 /24) = 2^15 -
      (2^15 * X^2 / 2) + A

      to ensure that f^4 does not overflow, use f/10 A = 2^15 * X^4 /24 = 2^15
      * (pi*f /4000)^4 /24 = 2^15 * (pi/4000)^4 * f^4 /24 = 2^15 * (pi/400)^4
      *(f/10)^4 /24

      Approximation with Taylor for sin in [-pi/4 .. +pi/4] = 2^15 * ( X - (X^3
      /6) + (X^5 /120) ) = (2^15 * X) - (2^15 * X^3 /6) + (2^15 * X^5 /120) =
      (2^15 * X) - (2^15 * X^3 /6) + B

      to ensure that f^5 does not overflow, use f/20 B = 2^15 * X^5 /120 = 2^15
      * (pi*f /4000)^5 / 120 = 2^15 * (pi/4000)^5 * f^5 / 120 = 2^15 *
      (pi*20/4000)^5 * (f/20)^5 /120 = 2^15 * (pi/200)^5 * (f/20)^5 /120 */

   if (f <= 1000)
   {
      /* cos approximation using the taylor algorithm */
      /* this formula covers the cos from 0 to pi/4 */
      coef =
         (IFX_int16_t) ((C2_15 - ((f * f * 1011) / 100000)) +
                        ((((f / 10) * (f / 10) * (f / 10) * (f / 10)) /
                          192487) - 1));
   }
   else if (f <= 2000)
   {
      /* sin approximation using the taylor algorithm */
      /* this formula covers the cos from pi/4 to pi/2 */
      f = 2000 - f;
      coef =
         (IFX_int16_t) (((25736 * f) / 1000 - (f * f * f / 377948)) +
                        ((f / 20) * (f / 20) * (f / 20) * (f / 20) * (f / 20) /
                         3829411));
   }
   else if (f <= 3000)
   {
      /* sin approximation using the taylor algorithm */
      /* this formula covers the cos from pi/2 to 3/4pi */
      f = f - 2000;
      coef =
         -(IFX_int16_t) (((((25736 * f) / 1000) -
                           (f * f * f / 377948)) +
                          ((f / 20) * (f / 20) * (f / 20) * (f / 20) *
                           (f / 20) / 3829411)));
   }
   else if (f <= 4000)
   {
      /* cos approximation using the taylor algorithm */
      /* this formula covers the cos from 3/4 pi to pi */
      f = 4000 - f;
      coef =
         -(IFX_int16_t) (((C2_15 - ((f * f * 1011) / 100000)) +
                          ((f / 10) * (f / 10) * (f / 10) * (f / 10) /
                           192487)));
   }
   else
   {
      VMMC_ASSERT (IFX_FALSE);
   }

   return coef;
}

/**
   function to calculate the cpt level coefficient

   \param level - level in [0.1 dB]
   \return
      cpt level coefficient
   \remarks
      CONST_CPT has to be set by define to
      CONST_CPT_HAMMING or CONST_CPT_BLACKMAN
*/
static IFX_int16_t vmmc_sig_CPTD_CalcLevel (IFX_int32_t level)
{
   long lvl;
   IFX_uint32_t tenExp;
   IFX_uint32_t shift;
   IFX_int32_t exp, i;

   /* calculate the desired level in mdB and the exponent */
   lvl = level * 100;
   exp = (IFX_int32_t) (-lvl - CONST_CPT) / 10;

   /* set the initial shift factor according to the level */
   if (lvl <= -20000)
      shift = 1000;
   else if (lvl <= -10000)
      shift = 10000;
   else
      shift = 10000;

   /* the initial value of the result (tenExp) is the shift factor which will
      be compensated in the last step (calculating the coefficient) return
      ((xxxxx * shift) / tenExp); */
   tenExp = shift;

   /* go over all elements in the tens array, starting with the largest entry
      and ... */
   for (i = 27; i >= 0; i--)
   {
      /* ... loop while the current tens[i][0] is part of the remaining exp */
      while (exp >= ((IFX_int32_t) tens[i][0]))
      {
         /* calculate part of the result for tens[i][0], check which accuracy
            of the tens[i][1] can be used to multiply tenExp without overflow */
         if ((C2_31 / tenExp) > tens[i][1])
         {
            /* use the unscaled tens[i][1] value to calculate the tenExp share */
            tenExp *= tens[i][1];
            tenExp /= 100000;
         }
         else if ((C2_31 / tenExp) > ROUND_DIV10 (tens[i][1]))
         {
            /* scale the tens[i][1] value by 10 to calculate the tenExp share */
            tenExp *= ROUND_DIV10 (tens[i][1]);
            tenExp /= 10000;
         }
         else if ((C2_31 / tenExp) > ROUND_DIV100 (tens[i][1]))
         {
            /* scale the tens[i][1] value by 100 to calculate the tenExp share */
            tenExp *= ROUND_DIV100 (tens[i][1]);
            tenExp /= 1000;
         }
         else
         {
            /* scale the tens[i][1] value by 1000 to calculate the tenExp share
             */
            tenExp *= ROUND_DIV1000 (tens[i][1]);
            tenExp /= 100;
         }

         /* calculate the remaining exp, i.e. subtract that part of exponent
            that has been calculated in this step */
         exp -= ((IFX_int32_t) tens[i][0]);
      }
   }

   /* calculate the coefficient according to the specification... */
   return (IFX_int16_t) ROUND (((C2_14 * shift) / (tenExp)));
}

/**
   Get resource number for given tone from FW command.

   \param  pCh          Pointer to VMMC channel structure.
   \param  p_fw_sig_cptd   Pointer to the CPTD configuration for channel.
   \param  nTone        Tone index number.

   \return
   - resource number if found
   - CPTD_RES_NOT_SET if resource not found
*/
IFX_uint8_t vmmc_sig_CPTD_GetResOfTone (VMMC_CHANNEL *pCh,
                                        SIG_CPTD_CTRL_t *p_fw_sig_cptd,
                                        IFX_int32_t nTone)
{
   VMMC_DEVICE *pDev = pCh->pParent;
   IFX_int32_t i;

   for (i = 0; i < MIN(pDev->caps.nCptdPerCh, SIG_CPTD_CTRL_DATA_MAX); i++)
   {
      /* checking only enabled CPTDs */
      if (p_fw_sig_cptd->CPTD[i].EN != 0)
      {
         if (nTone == pCh->pSIG->nCPTD_ToneIndex[i])
         {
            /* this is the searched resource number */
            return p_fw_sig_cptd->CPTD[i].CPTNR;
         }
      } /* checking only enabled CPTDs */
   }
   /* not found */
   return CPTD_RES_NOT_SET;
}

/**
   Disable CPTD for given tone, if tone not enabled return success.

   \param  pCh          Pointer to VMMC channel structure.
   \param  p_fw_sig_cptd   Pointer to the CPTD configuration for channel.
   \param  pResNr       Pointer to resource number.
   \param  nTone        Tone index number.

   \return
   - VMMC_statusOk if everything OK or tone not enabled
   - CmdWrite status
*/
IFX_int32_t vmmc_sig_CPTD_DisableTone (VMMC_CHANNEL *pCh,
                                       SIG_CPTD_CTRL_t *p_fw_sig_cptd,
                                       IFX_uint8_t *pResNr,
                                       IFX_int32_t nTone)
{
   VMMC_DEVICE *pDev = pCh->pParent;
   IFX_int32_t ret;

   /* get resource number */
   *pResNr = vmmc_sig_CPTD_GetResOfTone(pCh, p_fw_sig_cptd, nTone);

   if (*pResNr == CPTD_RES_NOT_SET)
   {
      /* if no resource available then return success,
         this tone was not enabled */
      RETURN_STATUS(VMMC_statusOk);
   }

   /* configure CPTD control structure */
   ret = vmmc_sig_CPTD_ConfDisable(pCh, p_fw_sig_cptd, *pResNr, IFX_FALSE);

   if (VMMC_SUCCESS(ret))
   {
      ret = CmdWrite (pDev, (IFX_uint32_t *)p_fw_sig_cptd,
                      p_fw_sig_cptd->LENGTH);
   }

   RETURN_STATUS(ret);
}

/**
   Configures the call progress tone detector structure to enable it for given
   resource number.

   \param  pCh          Pointer to VMMC channel structure.
   \param  pTone        pointer to the simple tone to detect.
   \param  pSignal      Pointer to the signal description.
   \param  pCmd         Pointer to the command structure to fill.
   \param  frameLength  Selects the frame length.
   \param  nResNr       CPTD resource number.

   \return
   - VMMC_statusOk if everything OK
   - VMMC_statusParam if wrong parameter
   - VMMC_statusSigCptdMaxRes cannot enabled
*/
static IFX_int32_t vmmc_sig_CPTD_ConfEnable (VMMC_CHANNEL *pCh,
                                             IFX_TAPI_TONE_SIMPLE_t const *pTone,
                                             IFX_TAPI_TONE_CPTD_t const *pSignal,
                                             SIG_CPTD_CTRL_t *pCmd,
                                             DSP_CPTD_FL_t frameLength,
                                             IFX_uint8_t nResNr)
{
   VMMC_DEVICE *pDev = pCh->pParent;
   IFX_uint32_t nCNT = 0, nIS = 0, i;

   /* check for continuos tone */
   if (pTone->cadence[1] == 0)
   {
      nCNT = 1;
   }

   switch (pSignal->signal)
   {
   case IFX_TAPI_TONE_CPTD_DIRECTION_RX:
      /* for receive path input I2 is used */
      nIS = SIG_CPTD_CTRL_IS_I2;
      break;
   case IFX_TAPI_TONE_CPTD_DIRECTION_TX:
      nIS = SIG_CPTD_CTRL_IS_I1;
      break;
   default:
      RETURN_STATUS(VMMC_statusParam);
   }
   /* for all avilable CPTDs */
   for (i = 0; i < MIN(pDev->caps.nCptdPerCh, SIG_CPTD_CTRL_DATA_MAX); i++)
   {
      if (pCmd->CPTD[i].EN == 0)
      {
         /* enable CPTD on first available structure */
         memset(&pCmd->CPTD[i], 0, sizeof (SIG_CPTD_CTRL_DATA_t));
         /* enable CPTD */
         pCmd->CPTD[i].EN = 1;
         /* increase number of used CPTDs */
         pCh->pSIG->nCPTD_Cnt++;
         /* remember tone index for CPTD stop and event handling */
         pCh->pSIG->nCPTD_ToneIndex[i] = pSignal->tone;
         /* Any Tone Detection inactive */
         pCmd->CPTD[i].AT = 0;
         /* set total power */
         pCmd->CPTD[i].TP = DSP_CPTD_TP_250_3400_HZ;
         /* set continuos tone */
         pCmd->CPTD[i].CNT = nCNT;
         /* set frame length */
         pCmd->CPTD[i].FL = frameLength;
         /* select window type */
         pCmd->CPTD[i].WS = DSP_CPTD_WS_HAMMING;
         /* set Input Signal */
         pCmd->CPTD[i].IS = nIS;
         /* set resource number */
         pCmd->CPTD[i].CPTNR = nResNr;
         break;
      }
   } /* for all avilable CPTDs */

   /* if did not enable any CPTD return err */
   if (i == MIN(pDev->caps.nCptdPerCh, SIG_CPTD_CTRL_DATA_MAX))
   {
      RETURN_STATUS(VMMC_statusSigCptdMaxRes);
   }

   /* set length of command data */
   for (i = 0; i < MIN(pDev->caps.nCptdPerCh, SIG_CPTD_CTRL_DATA_MAX); i++)
   {
      /* set length to hold furthest enabled CPTD */
      if (pCmd->CPTD[i].EN != 0)
      {
         pCmd->LENGTH = (i + 1) * SIG_CPTD_CTRL_LEN;
      }
   }

   RETURN_STATUS(VMMC_statusOk);
}

/**
   Configures the call progress tone detector structure to disable for given
   resource number. Free the resources if bResFree is set to IFX_TRUE.

   \param  pCh          Pointer to VMMC channel structure.
   \param  pCmd         Pointer to the command structure to fill.
   \param  nResNr       CPTD resource number, CPTD_RES_NOT_SET if all CPTDs
                        should be disabled.
   \param  bResFree     Set to IFX_TRUE if function should release resources.

   \return
   VMMC_statusOk - all is OK
   VMMC_statusNoChg - no CPTD disabled
   return value of VMMC_RES_CPTD_Release() if failed to release resource
*/
static IFX_int32_t vmmc_sig_CPTD_ConfDisable (VMMC_CHANNEL *pCh,
                                              SIG_CPTD_CTRL_t *pCmd,
                                              IFX_uint8_t nResNr,
                                              IFX_boolean_t bResFree)
{
   IFX_uint32_t i, nCPTD_CntPre = pCh->pSIG->nCPTD_Cnt;
   IFX_int32_t ret;
   VMMC_DEVICE *pDev = pCh->pParent;


   /* once again go through all CPTDs structures */
   for (i = 0; i < MIN(pDev->caps.nCptdPerCh, SIG_CPTD_CTRL_DATA_MAX); i++)
   {
      if (pCmd->CPTD[i].EN != 0)
      {
         /* set length to hold furthest enabled CPTD */
         pCmd->LENGTH = (i + 1) * SIG_CPTD_CTRL_LEN;

         /* disable if given resource number or resource number is not set*/
         if (pCmd->CPTD[i].CPTNR == nResNr ||
             CPTD_RES_NOT_SET == nResNr)
         {
            /* disable and clean up */
            pCmd->CPTD[i].EN = 0;
            pCh->pSIG->nCPTD_Cnt--;
            pCh->pSIG->nCPTD_ToneIndex[i]=0;

            /* check if releasing resources */
            if (IFX_TRUE == bResFree)
            {
               ret = VMMC_RES_CPTD_Release(pCh, pCmd->CPTD[i].CPTNR);
               if (!VMMC_SUCCESS(ret))
                  RETURN_STATUS(ret);
            } /* check if releasing resources */

         } /* disable if given resource number or resource number is not set*/
      }
   } /* once again go through all CPTDs structures */

   /* report error if no CPTD was disabled */
   if (pCh->pSIG->nCPTD_Cnt == nCPTD_CntPre)
   {
      RETURN_STATUS(VMMC_statusNoChg);
   }
   RETURN_STATUS(VMMC_statusOk);
}

/**
   Get tone number from EVT_SIG_CPTD_t.

   \param  pCh          Pointer to VMMC channel structure.
   \param  pEvtCPTD     Pointer to event data structure.

   \return
      - VMMC_statusFuncParm The parameters are wrong
      - VMMC_SIG_CPTD_NO_TONE_INDEX if unable to get tone number
      - tone Index
*/
IFX_uint8_t VMMC_SIG_CPTD_ToneFromEvtGet(VMMC_CHANNEL *pCh,
                                         EVT_SIG_CPTD_t *pEvtCPTD)
{
   VMMC_DEVICE *pDev = pCh->pParent;
   SIG_CPTD_CTRL_t *p_fw_sig_cptd;

   IFX_uint8_t nResNr;

   p_fw_sig_cptd = &pCh->pSIG->fw_sig_cptd;

   if (pEvtCPTD->CP >= MIN(pDev->caps.nCptdPerCh, SIG_CPTD_CTRL_DATA_MAX))
   {
      TRACE(VMMC, DBG_LEVEL_HIGH,
            ("Invalid CPTD number %u\n", pEvtCPTD->CP));
      return VMMC_SIG_CPTD_NO_TONE_INDEX;
   }

   if (p_fw_sig_cptd->CPTD[pEvtCPTD->CP].EN == 0)
   {
      TRACE(VMMC, DBG_LEVEL_HIGH,
            ("CPTD disabled for given number %u\n",
             pEvtCPTD->CP));
      return VMMC_SIG_CPTD_NO_TONE_INDEX;
   }
   nResNr = p_fw_sig_cptd->CPTD[pEvtCPTD->CP].CPTNR;
   /* validate resource number */
   if (nResNr >= pDev->caps.nCPTD)
   {
      TRACE(VMMC, DBG_LEVEL_HIGH,
            ("Invalid CPTD resource number %u\n", nResNr));
      return VMMC_SIG_CPTD_NO_TONE_INDEX;
   }

   /* return tone index */
   return pCh->pSIG->nCPTD_ToneIndex[pEvtCPTD->CP];
}

/**
   Start the call progress tone detector (CPTD)

   \param pLLChannel Handle to TAPI low level channel structure
   \param pTone      Handle to the simple tone structure
   \param pSignal    Pointer to the signal description


   \return
   - VMMC_statusCmdWr Writing the command failed
   - VMMC_statusParamRange parameter not in range.
   - VMMC_statusSigCptdMaxRes max number of CPTDs reached.

   - VMMC_statusOk if successful

   \remarks
   If the CPTD is already running for given tone then it is stopped,
   configured and then started again.
*/
IFX_int32_t VMMC_TAPI_LL_SIG_CPTD_Start (
   IFX_TAPI_LL_CH_t *pLLChannel,
   IFX_TAPI_TONE_SIMPLE_t const *pTone,
   IFX_TAPI_TONE_CPTD_t const *pSignal)
{
   VMMC_CHANNEL *pCh = (VMMC_CHANNEL *) pLLChannel;
   VMMC_DEVICE *pDev;
   SIG_CPTD_CTRL_t *p_fw_sig_cptd;
   IFX_int32_t ret = IFX_ERROR;
   RES_CPTD_COEF_t *pCoefCmd;
   DSP_CPTD_FL_t frameLength = DSP_CPTD_FL_16;
   IFX_uint8_t nResNr = CPTD_RES_NOT_SET;

   if ((IFX_NULL == pCh) || (IFX_NULL == pSignal) ||
       (IFX_NULL == pCh->pParent->pResCptd))
      return VMMC_statusParam;

   pDev = pCh->pParent;
   p_fw_sig_cptd = &pCh->pSIG->fw_sig_cptd;

   /* tone index == 0 can be used only for stop CPTD */
   if (0 == pSignal->tone)
      RETURN_STATUS(VMMC_statusParamRange);

   /* if CPTD detector is running stop it first before configuring it,
      this will also set resource number if already allocated for this tone */
   vmmc_sig_CPTD_DisableTone(pCh, p_fw_sig_cptd, &nResNr, pSignal->tone);

   /* check number of enabled CPTDs */
   if (pCh->pSIG->nCPTD_Cnt >= MIN(pDev->caps.nCptdPerCh, SIG_CPTD_CTRL_DATA_MAX))
   {
      RETURN_STATUS(VMMC_statusSigCptdMaxRes);
   }

   if (nResNr == CPTD_RES_NOT_SET)
   {
      /* getting resources */
      ret = VMMC_RES_CPTD_Allocate(pCh, &nResNr);

      if (!VMMC_SUCCESS(ret))
         RETURN_STATUS (ret);
   }

   /* nResNr must be already valid if no error occured */
   pCoefCmd = &pDev->pResCptd[nResNr].fw_cptdCoef;

   ret = VMMC_SIG_AutoChStop (pCh, IFX_TRUE);

   if (VMMC_SUCCESS(ret))
   {
   /* set CPTD coefficients ************************************************* */
      /* nFrameSize is depending on the tone cadence and will be decided
         inside vmmc_sig_CPTD_SetCoeff */
      vmmc_sig_CPTD_SetCoeff (pTone, pCoefCmd, &frameLength);
      /* write CPTD coefficients */
      ret = CmdWrite (pDev, (IFX_uint32_t *)pCoefCmd, RES_CPTD_COEF_LEN);
   }

   /* activate CPTD ********************************************************* */
   if (VMMC_SUCCESS(ret))
   {
      /* on activation of the CPTD also nFrameSize is configured as
         determined above... */
      /*lint -e{644} to get here also the code above must have been executed
                     that sets the frameLength variable. */
      ret = vmmc_sig_CPTD_ConfEnable (pCh, pTone, pSignal,
                                      p_fw_sig_cptd, frameLength, nResNr);
   }

   if (VMMC_SUCCESS(ret))
   {
      /* enable CPTD */
      ret = CmdWrite (pDev, (IFX_uint32_t *)p_fw_sig_cptd,
                      p_fw_sig_cptd->LENGTH);

      if (!VMMC_SUCCESS(ret))
         vmmc_sig_CPTD_ConfDisable(pCh, p_fw_sig_cptd, nResNr, IFX_FALSE);
   }

   if (!VMMC_SUCCESS(ret))
   {
      /* release resources and stop SIG channel */
      if (nResNr != CPTD_RES_NOT_SET)
      {
         VMMC_RES_CPTD_Release(pCh, nResNr);
      }
      VMMC_SIG_AutoChStop (pCh, IFX_FALSE);
   }

   RETURN_STATUS(ret);
}

/**
   Stop the call progress tone detector (CPTD)

   \param pLLChannel Handle to TAPI low level channel structure
   \param pSignal    Pointer to the signal description

   \return
   - VMMC_statusCmdWr Writing the command failed
   - VMMC_statusOk if successful
   - VMMC_statusSigCptdNoRes if CPTD not enabled
*/
IFX_int32_t VMMC_TAPI_LL_SIG_CPTD_Stop (
   IFX_TAPI_LL_CH_t *pLLChannel,
   IFX_TAPI_TONE_CPTD_t const *pSignal)
{
   VMMC_CHANNEL *pCh = (VMMC_CHANNEL *) pLLChannel;
   VMMC_DEVICE *pDev;
   SIG_CPTD_CTRL_t *p_fw_sig_cptd;
   IFX_int32_t ret = VMMC_statusOk;
   IFX_uint8_t nResNr = CPTD_RES_NOT_SET;

   if ((IFX_NULL == pCh) || (IFX_NULL == pSignal))
      return VMMC_statusParam;

   p_fw_sig_cptd = &pCh->pSIG->fw_sig_cptd;
   pDev = pCh->pParent;

   /* for tone set to 0 disable all CPTDs */
   if (pSignal->tone == 0)
   {
      if (pCh->pSIG->nCPTD_Cnt == 0)
      {
         /* this is OK, but trace info */
         TRACE(VMMC, DBG_LEVEL_LOW, ("No CPTD to disable\n"));
         RETURN_STATUS(VMMC_statusOk);
      }
   }
   else
   {
      if (pCh->pSIG->nCPTD_Cnt == 0)
      {
         TRACE(VMMC, DBG_LEVEL_LOW,
               ("No CPTD to disable tone %d\n", pSignal->tone));
         RETURN_STATUS(VMMC_statusOk);
      }
      nResNr = vmmc_sig_CPTD_GetResOfTone(pCh, p_fw_sig_cptd, pSignal->tone);
      if (CPTD_RES_NOT_SET == nResNr)
      {
         TRACE(VMMC, DBG_LEVEL_LOW,
               ("No CPTD for tone %d to disable\n", pSignal->tone));
         RETURN_STATUS(VMMC_statusOk);
      }
   }

   /* disable CPTD and release CPTD resources */
   ret = vmmc_sig_CPTD_ConfDisable(pCh, p_fw_sig_cptd, nResNr, IFX_TRUE);

   /* Write command if all is OK */
   if (VMMC_SUCCESS(ret))
   {
      ret = CmdWrite (pDev, (IFX_uint32_t *)p_fw_sig_cptd, p_fw_sig_cptd->LENGTH);

      if (VMMC_SUCCESS(ret))
      {
         ret = VMMC_SIG_AutoChStop (pCh, IFX_FALSE);
      }
   }

   RETURN_STATUS(ret);
}


/**
   Configures call progress tone detection coefficients.

   \param  pTone        Pointer to internal simple tone table entry.
   \param  pCmd         Coefficient command to fill.
   \param  pFrameLength Returns which frame length was selected.

   \return
   IFX_SUCCESS
*/
static IFX_int32_t vmmc_sig_CPTD_SetCoeff (IFX_TAPI_TONE_SIMPLE_t const *pTone,
                                           RES_CPTD_COEF_t *pCmd,
                                           DSP_CPTD_FL_t *pFrameLength)
{
   IFX_uint8_t nr = 0;
   IFX_uint8_t i;
   IFX_uint16_t val;
   unsigned int nTenPercentTimeTolerance,
                nMaxTenPercentTimeTolerance = 0,
                nAbsoluteTimeTolerance;

   /* set frame length, in case no step is <= 200ms we use a frame lenth of
      32ms, otherwise 16ms */
   *pFrameLength = DSP_CPTD_FL_32;
   for (i = 0; i < IFX_TAPI_TONE_STEPS_MAX; ++i)
   {
      /* check for last cadence step */
      if (pTone->cadence[i] == 0)
         break;
      /* if one of the steps is shorter or eaqual to 200ms we reduce the frame
         length to 16 ms */
      if (pTone->cadence[i] <= 200)
         *pFrameLength = DSP_CPTD_FL_16;
   }

   /* set nAbsoluteTimeTolerance to roughly 1.2 times the frame length */
   switch (*pFrameLength)
   {
      case DSP_CPTD_FL_16:
         nAbsoluteTimeTolerance =  40;
         break;
      case DSP_CPTD_FL_64:
         nAbsoluteTimeTolerance = 160;
         break;
      case DSP_CPTD_FL_32:
      default:
         nAbsoluteTimeTolerance =  80;
         break;
   }

   /* default settings */
   /* program allowed twist to 6 dB */
   pCmd->TWIST_12 = 0x20;
   pCmd->TWIST_34 = 0x20;
   pCmd->POW_PAUSE = CPTCOEFF_POW_PAUSE;
   pCmd->FP_TP_R = CPTCOEFF_FP_TP_R_DEFAULT;

   /* set frequency and level A for F_1. Freq A is always set. Tone API assures
      it, the detection level is always below the defined tone. */
   pCmd->GOE_1 = (IFX_uint16_t) vmmc_sig_CPTD_CalcFreq (pTone->freqA);
   pCmd->LEV_1 =
      (IFX_uint16_t) vmmc_sig_CPTD_CalcLevel (pTone->levelA -
                                              CPTD_DETECTION_LEVEL_TOLERANCE);
   /* set frequency and level B for F_2 */
   if (pTone->freqB)
   {
      pCmd->GOE_2 = (IFX_uint16_t) vmmc_sig_CPTD_CalcFreq (pTone->freqB);
      pCmd->LEV_2 =
         (IFX_uint16_t) vmmc_sig_CPTD_CalcLevel (pTone->levelB -
                                                 CPTD_DETECTION_LEVEL_TOLERANCE);
   }
   else
   {
      pCmd->GOE_2 = RES_CPTD_COEF_FX_0HZ;
      pCmd->LEV_2 = RES_CPTD_COEF_LEVEL_0DB;
   }
   /* set frequency and level C for F_3 */
   if (pTone->freqC)
   {
      pCmd->GOE_3 = (IFX_uint16_t) vmmc_sig_CPTD_CalcFreq (pTone->freqC);
      pCmd->LEV_3 =
         (IFX_uint16_t) vmmc_sig_CPTD_CalcLevel (pTone->levelC -
                                                 CPTD_DETECTION_LEVEL_TOLERANCE);
   }
   else
   {
      pCmd->GOE_3 = RES_CPTD_COEF_FX_0HZ;
      pCmd->LEV_3 = RES_CPTD_COEF_LEVEL_0DB;
   }
   /* set frequency and level D for F_4 */
   if (pTone->freqD)
   {
      pCmd->GOE_4 = (IFX_uint16_t) vmmc_sig_CPTD_CalcFreq (pTone->freqD);
      pCmd->LEV_4 =
         (IFX_uint16_t) vmmc_sig_CPTD_CalcLevel (pTone->levelD -
                                                 CPTD_DETECTION_LEVEL_TOLERANCE);
   }
   else
   {
      pCmd->GOE_4 = RES_CPTD_COEF_FX_0HZ;
      pCmd->LEV_4 = RES_CPTD_COEF_LEVEL_0DB;
   }

   /* set step times: T_x */
   for (i = 0; i < 4; ++i)
   {
      /* check for last cadence step */
      if (pTone->cadence[i] == 0)
         break;

      /* to allow +/- 10% deviation in the time, we'll reduce each time by 10%
         and program +TIM_TOL to 2 * MAX ( cadence [i] / 10 ). In addition we
         check if the tolerance for each step as well as +TIM_TOL is smaller
         than nAbsoluteTimeTolerance, if so we use the latter one. */
      nTenPercentTimeTolerance = pTone->cadence[i] / 10;
      /* limit the time tolerance to 8 bit / 2 */
      if (nTenPercentTimeTolerance > 127)
         nTenPercentTimeTolerance = 127;
      if (nTenPercentTimeTolerance > nMaxTenPercentTimeTolerance)
         nMaxTenPercentTimeTolerance = nTenPercentTimeTolerance;

      val = (IFX_uint16_t) (pTone->cadence[i] - MAX(nAbsoluteTimeTolerance,
                                                    nTenPercentTimeTolerance));
      switch (i)
      {
         case 0:
            pCmd->T_1 = val;
            break;
         case 1:
            pCmd->T_2 = val;
            break;
         case 2:
            pCmd->T_3 = val;
            break;
         case 3:
            pCmd->T_4 = val;
            break;
         default:
            /* to make the compiler happy -- do nothing */
            break;
      }

      /* if no freq is selected in this step, activate pause detection */
      if ((pTone->frequencies[i] & IFX_TAPI_TONE_FREQALL) == 0)
      {
         val = RES_CPTD_COEF_P;
      }
      else
      {
         /* set mask for MSK_i - use frequency A. Initialize the field */
         if (pTone->frequencies[i] & IFX_TAPI_TONE_FREQA)
            val = RES_CPTD_COEF_F1;
         else
            val = 0;
         /* set mask for MSK_i - use frequency B */
         if (pTone->frequencies[i] & IFX_TAPI_TONE_FREQB)
            val |= RES_CPTD_COEF_F2;
         /* set mask for MSK_i - use frequency C */
         if (pTone->frequencies[i] & IFX_TAPI_TONE_FREQC)
            val |= RES_CPTD_COEF_F3;
         /* set mask for MSK_i - use frequency D */
         if (pTone->frequencies[i] & IFX_TAPI_TONE_FREQD)
            val |= RES_CPTD_COEF_F4;
      }
      switch (i)
      {
         case 0:
            pCmd->MSK_1 = val;
            break;
         case 1:
            pCmd->MSK_2 = val;
            break;
         case 2:
            pCmd->MSK_3 = val;
            break;
         case 3:
            pCmd->MSK_4 = val;
            break;
         default:
            /* to make the compiler happy -- do nothing */
            break;
      }
      nr++;
   }
   if (pTone->cadence[1] == 0 && pTone->pause == 0)
   {
      /* set tolerance +TIM_TOL */
      pCmd->TIM_TOL =
                (2 * MAX (nAbsoluteTimeTolerance, nMaxTenPercentTimeTolerance));
      /* return because nothing more to do */
      return IFX_SUCCESS;
   }
   /* set end of steps */
   switch (i)
   {
      case 1:
         pCmd->MSK_1 |= RES_CPTD_COEF_E;
         break;
      case 2:
         pCmd->MSK_2 |= RES_CPTD_COEF_E;
         break;
      case 3:
         pCmd->MSK_3 |= RES_CPTD_COEF_E;
         break;
      case 4:
         pCmd->MSK_4 |= RES_CPTD_COEF_E;
         break;
      default:
         /* to make the compiler happy -- do nothing */
         break;
   }

   /* set tolerance +TIM_TOL and NR of successfully fulfilled timing
      requirement steps required to pass */
   pCmd->TIM_TOL =
                (2 * MAX (nAbsoluteTimeTolerance, nMaxTenPercentTimeTolerance));
   pCmd->NR = nr;

   return IFX_SUCCESS;
}
