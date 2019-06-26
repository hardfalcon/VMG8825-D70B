/******************************************************************************

  Copyright (c) 2012 Lantiq Deutschland GmbH
  Copyright 2016, Intel Corporation.

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/**
   \file drv_vmmc_sig_utg.c
   This file implements the functions for the Universal Tone Generator (UTG).
*/

#include "drv_api.h"
#include "drv_vmmc_sig_priv.h"
#include "drv_vmmc_api.h"
#include "drv_vmmc_sig.h"

/* array of partial values used to approach the function 10^X */
const IFX_uint32_t  tens  [28][2] =
{
   {    1,  100231},
   {    2,  100462},
   {    3,  100693},
   {    4,  100925},
   {    5,  101158},
   {    6,  101391},
   {    7,  101625},
   {    8,  101859},
   {    9,  102094},
   {   10,  102329},
   {   20,  104713},
   {   30,  107152},
   {   40,  109648},
   {   50,  112202},
   {   60,  114815},
   {   70,  117490},
   {   80,  120226},
   {   90,  123027},
   {  100,  125893},
   {  200,  158489},
   {  300,  199526},
   {  400,  251189},
   {  500,  316228},
   {  600,  398107},
   {  700,  501187},
   {  800,  630957},
   {  900,  794328},
   { 1000, 1000000}
};


/* ============================= */
/* Local function declaration    */
/* ============================= */

static IFX_int16_t vmmc_sig_UTG_CalcLevel (IFX_int32_t level);
static IFX_int32_t vmmc_sig_UTG_Start     (VMMC_CHANNEL *pCh,
                                           IFX_TAPI_TONE_SIMPLE_t const *,
                                           TAPI_TONE_DST,
                                           IFX_uint8_t);
static IFX_int32_t vmmc_sig_UTG_Stop      (VMMC_CHANNEL *pCh,
                                           IFX_uint8_t nUtg);

/* ============================= */
/* Local function definition     */
/* ============================= */

/**
   function to calculate the utg level coefficient

   \param level - level in [0.1 dB]
   \return
      utg level coefficient
   \remarks
      none
*/
static IFX_int16_t vmmc_sig_UTG_CalcLevel (IFX_int32_t level)
{
   long lvl;
   IFX_uint32_t  exp, tenExp;
   IFX_uint32_t  shift;
   IFX_int32_t   i;

   if (level == 0)
      return 0xFF;
   /* calclulate the desired level in mdB and the exponent */
   lvl = level * 100;
   exp = (IFX_uint32_t)((-lvl) / 20);

   /* set the initial shift factor according to the level */
   if (lvl <= -20000)
      shift  = 1000;
   else if (lvl <= -10000)
      shift  = 10000;
   else
      shift  = 10000;

   /* the initial value of the result (tenExp) is the shift factor
      which will be compensated in the last step (calculating the
      coefficient)
      return ((xxxxx * shift) / tenExp); */
   tenExp = shift;

   /* go over allelements in the tens array, starting with the
      largest entry and ... */
   for (i=27;i>=0;i--)
   {
      /* ... loop while the current tens[i][0] is part of the remaining exp */
      while (exp >= tens[i][0])
      {

         /* calulate part of the result for tens[i][0], check which accuracy
            of the tens[i][1] can be used to multiply tenExp without overflow */
         if ((C2_31 / tenExp) > tens[i][1])
         {
            /* use the unscaled tens[i][1] value to calculate
               the tenExp share */
            tenExp *= tens[i][1];
            tenExp /= 100000;
         }
         else if ( (C2_31 / tenExp) > ROUND_DIV10(tens[i][1]))
         {
            /* scale the tens[i][1] value by 10
               to calculate the tenExp share */
            tenExp *= ROUND_DIV10(tens[i][1]);
            tenExp /= 10000;
         }
         else if ( (C2_31 / tenExp) > ROUND_DIV100(tens[i][1]))
         {
            /* scale the tens[i][1] value by 100
               to calculate the tenExp share */
            tenExp *= ROUND_DIV100(tens[i][1]);
            tenExp /= 1000;
         }
         else
         {
            /* scale the tens[i][1] value by 1000
               to calculate the tenExp share */
            tenExp *= ROUND_DIV1000(tens[i][1]);
            tenExp /= 100;
         }

         /* calculate the remaining exp, i.e. subtract that part of exponent
            that has been calculated in this step */
         exp -= tens[i][0];
      }
   }

   /* calculate the coefficient according to the specification... */
   return (IFX_int16_t) ROUND (((C2_8*shift)/(tenExp)));
}


/**
   Configure and activate the UTG (Universal Tone Generator)

   For the UTG, the coefficients have to be programmed with a separate command
   before the UTG can be activated. The UTG must be inactive when programming
   the coefficients.

   \param  pLLChannel   Pointer to VMMC_CHANNEL structure.
   \param  pSimpleTone  Pointer to internal simple tone table entry.
   \param  nDst         Destination (direction) where to play the tone.
   \param  nUtg         UTG submodule to play the tone on.

   \return
   IFX_SUCCESS/IFX_ERROR

   \remarks
   The selected signalling channel has to be activated before this command can
   be sent.
   It is assumed that the UTG is not active before calling this function which
   programs the UTG coefficients.
   This function is completely protected from concurrent member access.
*/
static
IFX_int32_t vmmc_sig_UTG_Start (VMMC_CHANNEL *pLLChannel,
                                IFX_TAPI_TONE_SIMPLE_t const *pSimpleTone,
                                TAPI_TONE_DST nDst,
                                IFX_uint8_t nUtg)
{
   IFX_int32_t      ret = IFX_SUCCESS;
   VMMC_CHANNEL     *pCh  = (VMMC_CHANNEL *)pLLChannel;
   VMMC_DEVICE      *pDev = pCh->pParent;
   SIG_UTG_CTRL_t   *pUtgCmd;
   RES_UTG_COEF_t   *pUtgCoefCmd;
   IFX_uint32_t     *pCmd, nCount;
   IFX_uint8_t       nCh = pCh->nChannel - 1;
   IFX_uint8_t       nUtgResNr;

   /* calling function ensures valid parameter */
   VMMC_ASSERT(pSimpleTone != IFX_NULL);
   VMMC_ASSERT(nUtg < pDev->caps.nUtgPerCh);

   /* Use this UTG resource number. */
   nUtgResNr = nCh + nUtg * pDev->caps.nSIG;

   /* setup the UTG coefficients */
   pUtgCoefCmd = &pCh->pSIG->fw_sig_utg_coef;

   /* Rewrite the channel in the command header and set the coefficients. */
   pUtgCoefCmd->CHAN = nUtgResNr;
   VMMC_SIG_UTG_SetCoeff (pSimpleTone, pUtgCoefCmd);

   pCmd = (IFX_uint32_t *)pUtgCoefCmd;
   ret = CmdWrite (pDev, pCmd, RES_UTG_COEF_LEN);
   if (!VMMC_SUCCESS(ret))
      RETURN_STATUS (ret);

   /* pointer to cached UTG control message */
   pUtgCmd = &pCh->pSIG->fw_utg[nUtg];

   /* Set the UTG resource number. */
   pUtgCmd->UTGNRL = nUtgResNr & 0x0F;
   pUtgCmd->UTGNRH = (nUtgResNr >> 4) & 0x0F;

   /* Tone signal injection and muting the voice signal
     into adder 1 and/or adder 2 */
   switch (nDst)
   {
      case  TAPI_TONE_DST_NET:
         /* put it to adder 1 to network */
         pUtgCmd->A1 = SIG_UTG_CTRL_A1_ON;
         pUtgCmd->A2 = SIG_UTG_CTRL_A2_OFF;
         break;
      case  TAPI_TONE_DST_NETLOCAL:
         /* put it to adder 1 to network and adder 2 for local */
         pUtgCmd->A1 = SIG_UTG_CTRL_A1_ON;
         pUtgCmd->A2 = SIG_UTG_CTRL_A2_ON;
         break;
      case  TAPI_TONE_DST_LOCAL:
      case  TAPI_TONE_DST_DEFAULT:
      default:
         /* play it locally: take adder 2 */
         pUtgCmd->A1 = SIG_UTG_CTRL_A1_OFF;
         pUtgCmd->A2 = SIG_UTG_CTRL_A2_ON;
         break;
   }

   /* activate the UTG with the tone action previously programmed */
   pUtgCmd->SM = SIG_UTG_CTRL_SM_STOP;
   pUtgCmd->EN = SIG_UTG_CTRL_ENABLE;
   pUtgCmd->EU = SIG_UTG_CTRL_EU_ON;

   pCmd = (IFX_uint32_t *)pUtgCmd;
   ret = CmdWrite (pDev, pCmd, SIG_UTG_CTRL_LEN);

   if (VMMC_SUCCESS(ret))
   {
      if (pSimpleTone->loop > 0 || pSimpleTone->pause > 0)
      {
         /* auto stop after loop or after each generation step */
         pUtgCmd->SM = SIG_UTG_CTRL_SM_CONTINUE;
         pUtgCmd->EN = SIG_UTG_CTRL_DISABLE;

         pCmd = (IFX_uint32_t *) &pCh->pSIG->fw_utg[nUtg];
         nCount = sizeof (pCh->pSIG->fw_utg[nUtg]) - CMD_HDR_CNT;
         ret = CmdWrite (pDev, pCmd, nCount);
      }
   }
   if (!VMMC_SUCCESS(ret))
      RETURN_STATUS (ret);
   return VMMC_statusOk;
}


/**
   Stops the UTG

   \param  pCh          Pointer to VMMC channel structure.
   \param  nUtg         UTG submodule to play the tone on.

   \return
   VMMC_statusOk or error code
*/
static
IFX_int32_t vmmc_sig_UTG_Stop (VMMC_CHANNEL *pCh, IFX_uint8_t nUtg)
{
   IFX_int32_t     ret       = VMMC_statusOk;
   VMMC_DEVICE     *pDev    = pCh->pParent;
   SIG_UTG_CTRL_t  *pUtgCmd;
   IFX_uint32_t    *pCmd, nCount;

   /* calling function ensures valid parameter */
   VMMC_ASSERT(nUtg < pDev->caps.nUtgPerCh);

   /* pointer to cached UTG control message */
   pUtgCmd = &pCh->pSIG->fw_utg[nUtg];

   /* INFO: No need to enable the UTG before disabling with SM bit set to zero
            as required for Vinetic-CPE. */
   if (pUtgCmd->EN == SIG_UTG_CTRL_ENABLE ||
       pUtgCmd->SM == SIG_UTG_CTRL_SM_CONTINUE)
   {
      pUtgCmd->EN = SIG_UTG_CTRL_DISABLE;
      pUtgCmd->SM = SIG_UTG_CTRL_SM_STOP;
      /* make sure no further event is generated on manual stop */
      pUtgCmd->EU = SIG_UTG_CTRL_EU_OFF;

      pCmd = (IFX_uint32_t *)pUtgCmd;
      nCount = sizeof(SIG_UTG_CTRL_t) - CMD_HDR_CNT;

      ret = CmdWrite (pDev, pCmd, nCount);
   }
   if (!VMMC_SUCCESS(ret))
      RETURN_STATUS (ret);
   return VMMC_statusOk;
}


/* ============================= */
/* Global function definitions   */
/* ============================= */


/**
   Set the coefficients for the UTG coefficient command.

   This function sets only the coefficient part of the command but leaves
   the command header part untouched.

   \param  pTone        Internal simple tone table entry.
   \param  pUtgCoef     Pointer to struct to be filled with coefficients.

   \return
   VMMC_statusOk
*/
IFX_int32_t VMMC_SIG_UTG_SetCoeff (IFX_TAPI_TONE_SIMPLE_t const *pTone,
                                   RES_UTG_COEF_t *pUtgCoef)
{
   IFX_int32_t modulation_factor;

   /* skip the command header and clear the rest */
   memset ((void *)((IFX_uint32_t)pUtgCoef + CMD_HDR_CNT),
           0, sizeof(*pUtgCoef) - CMD_HDR_CNT);

   pUtgCoef->FD_IN_ATT = 0x47FB;
   pUtgCoef->FD_IN_SP  = 0x4000;
   pUtgCoef->FD_OT_SP  = 0x199A;
   pUtgCoef->FD_OT_TIM = 0xFF;

   /* set frequency A for F_1 */
   pUtgCoef->F1 =  (IFX_uint16_t)((8192 * pTone->freqA) / 1000);
   pUtgCoef->F2 =  (IFX_uint16_t)((8192 * pTone->freqB) / 1000);
   pUtgCoef->F3 =  (IFX_uint16_t)((8192 * pTone->freqC) / 1000);
   pUtgCoef->F4 =  (IFX_uint16_t)((8192 * pTone->freqD) / 1000);

   /* set power level for LEV_1, LEV_2 and LEV_3 */
   pUtgCoef->LEV_1 = (IFX_uint8_t)vmmc_sig_UTG_CalcLevel(pTone->levelA);
   pUtgCoef->LEV_2 = (IFX_uint8_t)vmmc_sig_UTG_CalcLevel(pTone->levelB);
   pUtgCoef->LEV_3 = (IFX_uint8_t)vmmc_sig_UTG_CalcLevel(pTone->levelC);
   pUtgCoef->LEV_4 = (IFX_uint8_t)vmmc_sig_UTG_CalcLevel(pTone->levelD);

   /* set modulation factor */
   if (pTone->modulation_factor == IFX_TAPI_TONE_MODULATION_FACTOR_100)
      modulation_factor = 100;
   else
      modulation_factor = pTone->modulation_factor;

   pUtgCoef->MOD_12 = (IFX_uint8_t)((modulation_factor * (-1280)) / 1000);

   /* set step times:  T_x */
   if (pTone->cadence[0] != 0)
   {
      /* the firmware uses a timebase of 0.5 ms */
     pUtgCoef->T_1 = (IFX_uint16_t)(2 * pTone->cadence[0]);
      /* set mask for MSK_i  */
      pUtgCoef->MSK1_F1_ON =
          !(!(pTone->frequencies[0] & IFX_TAPI_TONE_FREQA));

      pUtgCoef->MSK1_F2_ON =
          !(!(pTone->frequencies[0] & IFX_TAPI_TONE_FREQB));
      pUtgCoef->MSK1_F3_ON =
          !(!(pTone->frequencies[0] & IFX_TAPI_TONE_FREQC));
      pUtgCoef->MSK1_F4_ON =
          !(!(pTone->frequencies[0] & IFX_TAPI_TONE_FREQD));

      if (pTone->modulation[0] & IFX_TAPI_TONE_MODULATION_ON)
         pUtgCoef->MSK1_M12 = RES_UTG_COEF_M12_ON;

      pUtgCoef->MSK1_NXT =  1;
   }
   else
   {
      /* ToneAPI tone's cadence can not be zero */
      return VMMC_statusSigUtgBadCadence;
   }

   if (pTone->cadence[1] != 0)
   {
      /* the firmware uses a timebase of 0.5 ms */
     pUtgCoef->T_2 = (IFX_uint16_t)(2 * pTone->cadence[1]);
       /* set mask for MSK_i  */
      pUtgCoef->MSK2_F1_ON =
          !(!(pTone->frequencies[1] & IFX_TAPI_TONE_FREQA));

      pUtgCoef->MSK2_F2_ON =
          !(!(pTone->frequencies[1] & IFX_TAPI_TONE_FREQB));
      pUtgCoef->MSK2_F3_ON =
          !(!(pTone->frequencies[1] & IFX_TAPI_TONE_FREQC));
      pUtgCoef->MSK2_F4_ON =
          !(!(pTone->frequencies[1] & IFX_TAPI_TONE_FREQD));

      if (pTone->modulation[1] & IFX_TAPI_TONE_MODULATION_ON)
         pUtgCoef->MSK2_M12 = RES_UTG_COEF_M12_ON;

      pUtgCoef->MSK2_NXT =  2;

   }
   else
   {
      /* special case: continous tone for the one and only cadence step.
         In that case a value FFFF must be programmed to the firmware */
      if (pTone->pause == 0 && pTone->loop == 0)
      {
         pUtgCoef->T_1 = 0xFFFF;
      }

      pUtgCoef->MSK1_NXT =  0;

      if (pTone->pause == 0 && pTone->loop)
      {
              /* The deactivation of the tone
                 generator is delayed until the current tone generation step
                 has been completely executed inclusive of the requested
                 repetitions */
             pUtgCoef->MSK1_REP = (IFX_uint16_t)(pTone->loop - 1);
              pUtgCoef->MSK1_SA  = RES_UTG_COEF_SA_DELAYED_REP;
       }
       else
       {
              /* The deactivation of the tone
                 generator is delayed until the current tone generation step
                 has been completely executed. The deactivation is made
                 immediately after the execution of the current tone
                 generation step. */
              pUtgCoef->MSK1_SA  = RES_UTG_COEF_SA_DELAYED_EXECUTED;
       }

       goto utg_coeff_end;
   }

   if (pTone->cadence[2] != 0)
   {
      /* the firmware uses a timebase of 0.5 ms */
     pUtgCoef->T_3 = (IFX_uint16_t)(2 * pTone->cadence[2]);
       /* set mask for MSK_i  */
      pUtgCoef->MSK3_F1_ON =
          !(!(pTone->frequencies[2] & IFX_TAPI_TONE_FREQA));

      pUtgCoef->MSK3_F2_ON =
          !(!(pTone->frequencies[2] & IFX_TAPI_TONE_FREQB));
      pUtgCoef->MSK3_F3_ON =
          !(!(pTone->frequencies[2] & IFX_TAPI_TONE_FREQC));
      pUtgCoef->MSK3_F4_ON =
          !(!(pTone->frequencies[2] & IFX_TAPI_TONE_FREQD));

      if (pTone->modulation[2] & IFX_TAPI_TONE_MODULATION_ON)
         pUtgCoef->MSK3_M12 = RES_UTG_COEF_M12_ON;

      pUtgCoef->MSK3_NXT =  3;

   }
   else
   {
      pUtgCoef->MSK2_NXT = 0;

      if (pTone->pause == 0 && pTone->loop)
      {
              /* The deactivation of the tone
                 generator is delayed until the current tone generation step
                 has been completely executed inclusive of the requested
                 repetitions */
             pUtgCoef->MSK2_REP = (IFX_uint16_t)(pTone->loop - 1);
              pUtgCoef->MSK2_SA  = RES_UTG_COEF_SA_DELAYED_REP;
       }
       else
       {
              /* The deactivation of the tone
                 generator is delayed until the current tone generation step
                 has been completely executed. The deactivation is made
                 immediately after the execution of the current tone
                 generation step. */
              pUtgCoef->MSK2_SA  = RES_UTG_COEF_SA_DELAYED_EXECUTED;
       }

       goto utg_coeff_end;
   }

   if (pTone->cadence[3] != 0)
   {
      /* the firmware uses a timebase of 0.5 ms */
     pUtgCoef->T_4 = (IFX_uint16_t)(2 * pTone->cadence[3]);
        /* set mask for MSK_i  */
      pUtgCoef->MSK4_F1_ON =
          !(!(pTone->frequencies[3] & IFX_TAPI_TONE_FREQA));

      pUtgCoef->MSK4_F2_ON =
          !(!(pTone->frequencies[3] & IFX_TAPI_TONE_FREQB));
      pUtgCoef->MSK4_F3_ON =
          !(!(pTone->frequencies[3] & IFX_TAPI_TONE_FREQC));
      pUtgCoef->MSK4_F4_ON =
          !(!(pTone->frequencies[3] & IFX_TAPI_TONE_FREQD));

      if (pTone->modulation[3] & IFX_TAPI_TONE_MODULATION_ON)
         pUtgCoef->MSK4_M12 = RES_UTG_COEF_M12_ON;

      pUtgCoef->MSK4_NXT =  4;

   }
   else
   {
      pUtgCoef->MSK3_NXT = 0;
      if (pTone->pause == 0 && pTone->loop)
      {
              /* The deactivation of the tone
                 generator is delayed until the current tone generation step
                 has been completely executed inclusive of the requested
                 repetitions */
             pUtgCoef->MSK3_REP = (IFX_uint16_t)(pTone->loop - 1);
             pUtgCoef->MSK3_SA  = RES_UTG_COEF_SA_DELAYED_REP;
       }
       else
       {
              /* The deactivation of the tone
                 generator is delayed until the current tone generation step
                 has been completely executed. The deactivation is made
                 immediately after the execution of the current tone
                 generation step. */
              pUtgCoef->MSK3_SA  = RES_UTG_COEF_SA_DELAYED_EXECUTED;
       }

       goto utg_coeff_end;
   }

   if (pTone->cadence[4] != 0)
   {
      /* the firmware uses a timebase of 0.5 ms */
     pUtgCoef->T_5 = (IFX_uint16_t)(2 * pTone->cadence[4]);
         /* set mask for MSK_i  */
      pUtgCoef->MSK5_F1_ON =
          !(!(pTone->frequencies[4] & IFX_TAPI_TONE_FREQA));

      pUtgCoef->MSK5_F2_ON =
          !(!(pTone->frequencies[4] & IFX_TAPI_TONE_FREQB));
      pUtgCoef->MSK5_F3_ON =
          !(!(pTone->frequencies[4] & IFX_TAPI_TONE_FREQC));
      pUtgCoef->MSK5_F4_ON =
          !(!(pTone->frequencies[4] & IFX_TAPI_TONE_FREQD));

      if (pTone->modulation[4] & IFX_TAPI_TONE_MODULATION_ON)
         pUtgCoef->MSK5_M12 = RES_UTG_COEF_M12_ON;

      pUtgCoef->MSK5_NXT =  5;

   }
   else
   {
      pUtgCoef->MSK4_NXT =  0;

      if (pTone->pause == 0 && pTone->loop)
      {
              /* The deactivation of the tone
                 generator is delayed until the current tone generation step
                 has been completely executed inclusive of the requested
                 repetitions */
             pUtgCoef->MSK4_REP = (IFX_uint16_t)(pTone->loop - 1);
             pUtgCoef->MSK4_SA  = RES_UTG_COEF_SA_DELAYED_REP;
       }
       else
       {
              /* The deactivation of the tone
                 generator is delayed until the current tone generation step
                 has been completely executed. The deactivation is made
                 immediately after the execution of the current tone
                 generation step. */
              pUtgCoef->MSK4_SA  = RES_UTG_COEF_SA_DELAYED_EXECUTED;
       }

       goto utg_coeff_end;
   }
   if (pTone->cadence[5] != 0)
   {
      /* the firmware uses a timebase of 0.5 ms */
     pUtgCoef->T_6 = (IFX_uint16_t)(2 * pTone->cadence[5]);
          /* set mask for MSK_i  */
      pUtgCoef->MSK6_F1_ON =
          !(!(pTone->frequencies[5] & IFX_TAPI_TONE_FREQA));

      pUtgCoef->MSK6_F2_ON =
          !(!(pTone->frequencies[5] & IFX_TAPI_TONE_FREQB));
      pUtgCoef->MSK6_F3_ON =
          !(!(pTone->frequencies[5] & IFX_TAPI_TONE_FREQC));
      pUtgCoef->MSK6_F4_ON =
          !(!(pTone->frequencies[5] & IFX_TAPI_TONE_FREQD));

      if (pTone->modulation[5] & IFX_TAPI_TONE_MODULATION_ON)
         pUtgCoef->MSK6_M12 = RES_UTG_COEF_M12_ON;

      pUtgCoef->MSK6_NXT =  0;

      if (pTone->pause == 0 && pTone->loop)
      {
              /* The deactivation of the tone
                 generator is delayed until the current tone generation step
                 has been completely executed inclusive of the requested
                 repetitions */
             pUtgCoef->MSK6_REP = (IFX_uint16_t)(pTone->loop - 1);
             pUtgCoef->MSK6_SA  = RES_UTG_COEF_SA_DELAYED_REP;
       }
       else
       {
              /* The deactivation of the tone
                 generator is delayed until the current tone generation step
                 has been completely executed. The deactivation is made
                 immediately after the execution of the current tone
                 generation step. */
              pUtgCoef->MSK6_SA  = RES_UTG_COEF_SA_DELAYED_EXECUTED;
       }

       goto utg_coeff_end;

   }
   else
   {
      pUtgCoef->MSK5_NXT =  0;

      if (pTone->pause == 0 && pTone->loop)
      {
              /* The deactivation of the tone
                 generator is delayed until the current tone generation step
                 has been completely executed inclusive of the requested
                 repetitions */
             pUtgCoef->MSK5_REP = (IFX_uint16_t)(pTone->loop - 1);
             pUtgCoef->MSK5_SA  = RES_UTG_COEF_SA_DELAYED_REP;
       }
       else
       {
              /* The deactivation of the tone
                 generator is delayed until the current tone generation step
                 has been completely executed. The deactivation is made
                 immediately after the execution of the current tone
                 generation step. */
              pUtgCoef->MSK5_SA  = RES_UTG_COEF_SA_DELAYED_EXECUTED;
       }

       goto utg_coeff_end;
   }

utg_coeff_end:
   /* Always set the GO_1 and GO_2 coefficients to 0 dB (0x8000), i.e.
      tone level is defined only via the level settings above.
      If multiple tones are played out simultaneously the application has
      to adapt the levels to prevent clipping. */
   /* pUtgCoef->GO = 0x8000;*/
   pUtgCoef->GO = 0x7FFF;

   return VMMC_statusOk;
}


/**
   Do low level UTG (Universal Tone Generator) configuration and activation

   This function handles all necessary steps to play out a full simple tone
   on the UTG. It returns immediately.
   Per device there may be multiple resources to play a tone. If the resource
   number exceeds the amount of available resources this function returns an
   error code VMMC_statusNoRes.

   The selected signalling channel will be automatically activated when this
   function is executed.
   First the coefficients are programmed, before the UTG is be activated.
   The UTG must be inactive when programming the coefficients.
   Also here is no check done.
   It is assumed that the UTG is not active before calling this function which
   programs the UTG coefficients.

   The tone signal is injection and muting the voice signal into adder 1 for
   network and/or adder 2 for local tones .

   \param  pLLChannel   Handle to TAPI low level channel structure.
   \param  pSimpleTone  Pointer to a simple tone to be played.
   \param  dst          Destination where to play the tone: local or network
   \param  res          Resource number which is used for playing the tone.
                        The available resources are device dependend.

   \return
   - VMMC_statusCmdWr Writing the command failed
   - VMMC_statusSigUtgBadRes No free resources to play the tone. The reason
      is that res is larger then the number of available tone resources.
   - VMMC_statusFuncParm Wrong parameter passed. This code is returned
     when the given cadence is invalid, for example the first cadence step is
     zero. This is an internal check, because the tone configuration function
     verifies the correct cadence.
   - VMMC_statusInvalCh No tone resource on this channel.
   - VMMC_statusOk if successful
*/
IFX_int32_t VMMC_TAPI_LL_SIG_UTG_Start (IFX_TAPI_LL_CH_t *pLLChannel,
                                       IFX_TAPI_TONE_SIMPLE_t const *pSimpleTone,
                                       TAPI_TONE_DST dst,
                                       IFX_uint8_t res)
{
   VMMC_CHANNEL *pCh = (VMMC_CHANNEL *) pLLChannel;
   VMMC_DEVICE *pDev = (VMMC_DEVICE *) pCh->pParent;
   IFX_int32_t ret = VMMC_statusOk;

   /* Work only on channels where SIG is initialised */
   if (pCh->pSIG == IFX_NULL)
   {
      /* errmsg: Resource not valid. Channel number out of range */
      RETURN_STATUS (VMMC_statusInvalCh);
   }
   /* Does the channel have the requested resource number? */
   if ((res >= pDev->caps.nUtgPerCh) || (res >= LL_TAPI_TONE_MAXRES))
   {
      /*errmsg: UTG bad resource number */
      RETURN_STATUS (VMMC_statusSigUtgBadRes);
   }
   /* Tone parameter is required */
   if (pSimpleTone == IFX_NULL)
   {
      RETURN_STATUS (VMMC_statusParam);
   }

   TAPI_Tone_Set_Source (pCh->pTapiCh, res, IFX_TAPI_TONE_RESSEQ_SIMPLE);

   VMMC_OS_MutexGet (&pDev->mtxMemberAcc);
   /* If not already running activate the signalling module in this channel */
   ret = VMMC_SIG_AutoChStop (pCh, IFX_TRUE);
   /* Activate the universal tone generator, disable the DTMF generator
      and program the simple tone sequence */
   if (VMMC_SUCCESS (ret))
   {
      ret = vmmc_sig_UTG_Start(pCh, pSimpleTone, dst, res);
   }
   VMMC_OS_MutexRelease (&pDev->mtxMemberAcc);

   RETURN_STATUS (ret);
}


/**
   Stop playing a tone of given definition immediately

   This function does nothing when the tone playing resource is already
   deactivated.

   \param  pLLChannel   Handle to TAPI low level channel structure.
   \param  res          Resource number which was used for playing the tone.
                        The available resources are device dependend.
                        Usually res is set to zero.

   \return
   - VMMC_statusInvalCh No tone resource on this channel.
   - VMMC_statusCmdWr Writing the command failed
   - VMMC_statusNoRes No free resources specified in res. The reason
      is that res is larger then the number of available tone resources.
   - VMMC_statusOk if successful

   \remarks
   Stops the internal timer in any case.
*/
IFX_int32_t  VMMC_TAPI_LL_SIG_UTG_Stop (IFX_TAPI_LL_CH_t *pLLChannel,
                                       IFX_uint8_t res)
{
   VMMC_CHANNEL *pCh = (VMMC_CHANNEL *) pLLChannel;
   VMMC_DEVICE *pDev = (VMMC_DEVICE *) pCh->pParent;
   IFX_int32_t ret = VMMC_statusOk;

   /* Work only on channels where SIG is initialised */
   if (pCh->pSIG == IFX_NULL)
   {
      /* errmsg: Resource not valid. Channel number out of range */
      RETURN_STATUS (VMMC_statusInvalCh);
   }
   /* check for valid resource number */
   if ((res >= pDev->caps.nUtgPerCh) || (res >= LL_TAPI_TONE_MAXRES))
   {
      RETURN_STATUS (VMMC_statusSigUtgBadRes);
   }

   VMMC_OS_MutexGet (&pDev->mtxMemberAcc);
   ret = vmmc_sig_UTG_Stop (pCh, res);
   if (VMMC_SUCCESS (ret))
      ret = VMMC_SIG_AutoChStop (pCh, IFX_FALSE);
   VMMC_OS_MutexRelease (&pDev->mtxMemberAcc);

   RETURN_STATUS (ret);
}


/**
   Returns the total number of UTGs per channel

   \param  pLLChannel   Handle to TAPI low level channel structure.

   \return
   Returns the total number of UTGs per channel - function always succeeds.
*/
IFX_uint8_t VMMC_TAPI_LL_SIG_UTG_Count_Get (IFX_TAPI_LL_CH_t *pLLChannel)
{
   VMMC_CHANNEL *pCh = (VMMC_CHANNEL *) pLLChannel;
   VMMC_DEVICE *pDev = (VMMC_DEVICE *) pCh->pParent;
   return pDev->caps.nUtgPerCh;
}


/**
   Deactivate UTG

   Called upon event to update the state information.

   \param  pLLChannel   Handle to TAPI low level channel structure.
   \param  utgNum       UTG resource number.
*/
IFX_void_t VMMC_TAPI_LL_UTG_Event_Deactivated (IFX_TAPI_LL_CH_t *pLLChannel,
                                              IFX_uint8_t utgNum)
{
   VMMC_CHANNEL     *pCh     = (VMMC_CHANNEL *)pLLChannel;
   SIG_UTG_CTRL_t   *pUtgCmd;

   /* we are called upon event with already verified parameters */
   VMMC_ASSERT(pCh->pSIG != IFX_NULL);
   VMMC_ASSERT(utgNum < ((VMMC_DEVICE *)pCh->pParent)->caps.nUtgPerCh);
   /* after asserting valid resource number get the cached message */
   pUtgCmd = &pCh->pSIG->fw_utg[utgNum];

   /* reset the cache state */
   pUtgCmd->SM = SIG_UTG_CTRL_SM_STOP;
   pUtgCmd->EN = SIG_UTG_CTRL_DISABLE;
}
