/******************************************************************************

  Copyright (c) 2013 Lantiq Deutschland GmbH
  Copyright 2016, Intel Corporation.

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/**
   \file drv_vmmc_gr909.c
   This file contains the implementation of functions for GR909 linetesting.
*/

/** +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   \todo
   - protect running GR909, keep status information
   - check how to set different thresholds for pass/fail decision
*/


/* ========================================================================== */
/*                                 Includes                                   */
/* ========================================================================== */
#include "drv_vmmc_api.h"
#ifdef VMMC_FEAT_GR909
#include "drv_vmmc_alm.h"
#include "drv_vmmc_alm_priv.h"
#include "drv_vmmc_errno.h"

#include "ifxos_time.h"

/* ========================================================================== */
/*                             Macro definitions                              */
/* ========================================================================== */
#define HPT_W2G_AC_LIM_DEFAULT  9454  /* 0x24ee */
#define HPT_W2W_AC_LIM_DEFAULT  9454  /* 0x24ee */
#define HPT_W2G_DC_LIM_DEFAULT  18232 /* 0x4738 */
#define HPT_W2W_DC_LIM_DEFAULT  18232 /* 0x4738 */
#define FEMF_W2G_AC_LIM_DEFAULT 1890  /* 0x0762 */
#define FEMF_W2W_AC_LIM_DEFAULT 1890  /* 0x0762 */
#define FEMF_W2G_DC_LIM_DEFAULT 810   /* 0x032a */
#define FEMF_W2W_DC_LIM_DEFAULT 810   /* 0x032a */
#define RFT_RES_LIM_DEFAULT     7669  /* 0x1df5 */
#define ROH_LIN_LIM_DEFAULT     15    /* 0x000f */
#define RIT_LOW_LIM_DEFAULT     71    /* 0x0047 */
#define RIT_HIGH_LIM_DEFAULT    2045  /* 0x07fd */

/* ========================================================================== */
/*                             Type definitions                               */
/* ========================================================================== */

/* ========================================================================== */
/*                             Global variables                               */
/* ========================================================================== */

/* ========================================================================== */
/*                           Function prototypes                              */
/* ========================================================================== */

/* ========================================================================== */
/*                         Function implementation                            */
/* ========================================================================== */

/**
   Start selected subset (or all) GR909 tests

   \param  pLLChannel   Pointer to LL channel context.
   \param  p_start      Pointer to IFX_TAPI_GR909_START_t structure.

   \return
   - VMMC_statusOk      On success.
   - VMMC_statusInvalCh When there is no SDD on the addressed channel.
   - VMMC_statusFuncParm On parameter error.
   - VMMC_statusGR909LineNotDisabled When the line is not disabled on start.
   - VMMC_statusBlockedDcDcTypeMissmatch DC/DC type not set
   - VMMC_statusGR909NeighbourLineNotDisabled

   \remarks
   GR909 tests can be started only on disabled lines.
*/
IFX_int32_t VMMC_TAPI_LL_ALM_GR909_Start (IFX_TAPI_LL_CH_t *pLLChannel,
                                        IFX_TAPI_GR909_START_t const *p_start)
{
   VMMC_CHANNEL *pCh  = (VMMC_CHANNEL *) pLLChannel;
   VMMC_CHANNEL *pOtherCh = IFX_NULL;
   VMMC_DEVICE  *pDev;
   VMMC_SDD_GR909Config_t *pGR909ctl;
   IFX_uint8_t curr_opmode, other_curr_opmode;
   IFX_int32_t ret;

   /* sanity check */
   if (pCh == IFX_NULL)
   {
      /* RETURN_STATUS needs a valid pointer in pCh to send a TAPI event. */
      return VMMC_statusInvalCh;
   }
   if (pCh->pALM == IFX_NULL)
   {
      /* errmsg: Resource not valid. Channel number out of range */
      RETURN_STATUS(VMMC_statusInvalCh);
   }

   pDev = pCh->pParent;
   pGR909ctl = &pCh->pALM->fw_sdd_GR909Config;

   /* For DC/DC converters other than default a BBD download is required. */
   if ((pCh->pALM->nDcDcType == VMMC_DCDC_TYPE_DEFAULT_IBB) &&
       (pCh->pParent->sdd.nAllowedDcDcType != VMMC_DCDC_TYPE_IBB))
   {
      /* errmsg: Operation blocked until a BBD with DC/DC configuration
                 matching the connected DC/DC hardware type is downloaded */
      RETURN_STATUS (VMMC_statusBlockedDcDcTypeMissmatch);
   }

   /* Nothing to test ? */
   if (p_start->test_mask == 0)
   {
      /* errmsg: Parameter is out of range */
      RETURN_STATUS(VMMC_statusFuncParm);
   }

   /* protect channel against concurrent tasks */
   VMMC_OS_MutexGet(&pCh->chAcc);

   /* Get the current opmode - wait while a opmode change is ongoing. */
   ret = VMMC_ALM_OpmodeGet (pCh, &curr_opmode);
   /* Only exit when interrupted by a signal. When waiting was aborted by the
      timeout assume that no linemode change is pending and try to continue
      with the opmode change that was requested. */
   if (ret == VMMC_statusSddEvtWaitInterrupt)
   {
      VMMC_OS_MutexRelease(&pCh->chAcc);
      RETURN_STATUS (ret);
   }

   /* report error if line is not in "disabled" state */
   if (curr_opmode != VMMC_SDD_OPMODE_DISABLED)
   {
      VMMC_OS_MutexRelease(&pCh->chAcc);
      RETURN_STATUS(VMMC_statusGR909LineNotDisabled);
   }

   /* For combined DC/DC the other channel must be disabled when starting. */
   if (pCh->pParent->sdd.bDcDcHwCombined == IFX_TRUE)
   {
      /* The combined DC/DC flag is only set on 2 channel systems. So there
         is always a neighbouring channel. */
      pOtherCh = VMMC_ALM_FxsNeighbourChGet(pCh);
      if (pOtherCh == IFX_NULL)
      {
         VMMC_OS_MutexRelease(&pCh->chAcc);
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
         VMMC_OS_MutexRelease(&pCh->chAcc);
         RETURN_STATUS (ret);
      }

      if (other_curr_opmode != VMMC_SDD_OPMODE_DISABLED)
      {
         /* release protection */
         VMMC_OS_MutexRelease (&pOtherCh->chAcc);
         VMMC_OS_MutexRelease(&pCh->chAcc);
         /* errmsg: Neighbour line mode is not DISABLED */
         RETURN_STATUS (VMMC_statusGR909NeighbourLineNotDisabled);
      }
   }

   pGR909ctl->Pdf     = 0; /* no capacitance test */
   pGR909ctl->Hpt     = ((p_start->test_mask & IFX_TAPI_GR909_HPT) ? 1 : 0);
   pGR909ctl->Femf    = ((p_start->test_mask & IFX_TAPI_GR909_FEMF)? 1 : 0);
   pGR909ctl->Rft     = ((p_start->test_mask & IFX_TAPI_GR909_RFT) ? 1 : 0);
   pGR909ctl->Roh     = ((p_start->test_mask & IFX_TAPI_GR909_ROH) ? 1 : 0);
   pGR909ctl->Rit     = ((p_start->test_mask & IFX_TAPI_GR909_RIT) ? 1 : 0);
   pGR909ctl->HptW2gAcLim   = HPT_W2G_AC_LIM_DEFAULT;
   pGR909ctl->HptW2wAcLim   = HPT_W2W_AC_LIM_DEFAULT;
   pGR909ctl->HptW2gDcLim   = HPT_W2G_DC_LIM_DEFAULT;
   pGR909ctl->HptW2wDcLim   = HPT_W2W_DC_LIM_DEFAULT;
   pGR909ctl->FemfW2gAcLim  = FEMF_W2G_AC_LIM_DEFAULT;
   pGR909ctl->FemfW2wAcLim  = FEMF_W2W_AC_LIM_DEFAULT;
   pGR909ctl->FemfW2gDcLim  = FEMF_W2G_DC_LIM_DEFAULT;
   pGR909ctl->FemfW2wDcLim  = FEMF_W2W_DC_LIM_DEFAULT;
   pGR909ctl->RftResLim     = RFT_RES_LIM_DEFAULT;
   pGR909ctl->RohLinLim     = ROH_LIN_LIM_DEFAULT;
   pGR909ctl->RitLowLim     = RIT_LOW_LIM_DEFAULT;
   pGR909ctl->RitHighLim    = RIT_HIGH_LIM_DEFAULT;

   /* If one of these measurements
      is started, both bits should be set because the DC firmware can't handle
      the ROH measurement separately at the moment */
   if ((pGR909ctl->Rft == 1) || (pGR909ctl->Roh == 1))
   {
     pGR909ctl->Rft = 1;
     pGR909ctl->Roh = 1;
   }

   ret = CmdWrite(pDev, (IFX_uint32_t *)((IFX_void_t *)pGR909ctl),
                  pGR909ctl->LENGTH);

   if (!VMMC_SUCCESS(ret))
   {
      if (pOtherCh != IFX_NULL)
      {
         /* release protection */
         VMMC_OS_MutexRelease (&pOtherCh->chAcc);
      }
      VMMC_OS_MutexRelease(&pCh->chAcc);
      RETURN_STATUS(ret);
   }

   /* start GR909 measurement */
   ret = VMMC_ALM_OpmodeModeSet(pCh, VMMC_SDD_OPMODE_GR909);

   if (pOtherCh != IFX_NULL)
   {
      /* release protection */
      VMMC_OS_MutexRelease (&pOtherCh->chAcc);
   }
   VMMC_OS_MutexRelease(&pCh->chAcc);

   RETURN_STATUS(ret);
}


/**
   Stop GR909 tests

   \param  pLLChannel   Pointer to LL channel context.

   \return
   - VMMC_statusOk      On success.
   - VMMC_statusInvalCh When there is no SDD on the addressed channel.
   - VMMC_statusBlockedDcDcTypeMissmatch
*/
IFX_int32_t VMMC_TAPI_LL_ALM_GR909_Stop (IFX_TAPI_LL_CH_t *pLLChannel)
{
   VMMC_CHANNEL *pCh  = (VMMC_CHANNEL *) pLLChannel;
   IFX_int32_t  ret;

   /* sanity check */
   if (pCh == IFX_NULL)
   {
      /* RETURN_STATUS needs a valid pointer in pCh to send a TAPI event. */
      return VMMC_statusInvalCh;
   }
   if (pCh->pALM == IFX_NULL)
   {
      /* errmsg: Resource not valid. Channel number out of range */
      RETURN_STATUS(VMMC_statusInvalCh);
   }

   /* For DC/DC converters other than default a BBD download is required. */
   if ((pCh->pALM->nDcDcType == VMMC_DCDC_TYPE_DEFAULT_IBB) &&
       (pCh->pParent->sdd.nAllowedDcDcType != VMMC_DCDC_TYPE_IBB))
   {
      /* errmsg: Operation blocked until a BBD with DC/DC configuration
                 matching the connected DC/DC hardware type is downloaded */
      RETURN_STATUS (VMMC_statusBlockedDcDcTypeMissmatch);
   }

   /* protect channel against concurrent tasks */
   VMMC_OS_MutexGet(&pCh->chAcc);

   ret = VMMC_ALM_OpmodeModeSet(pCh, VMMC_SDD_OPMODE_DISABLED);

   VMMC_OS_MutexRelease(&pCh->chAcc);

   RETURN_STATUS(ret);
}


/**
   Read GR909 results

   Used by the application to read the GR909 results after the reception of
   the IFX_TAPI_GR909_RDY event.

   \param  pLLChannel   Pointer to LL channel context.
   \param  pResults     Pointer to TAPI result structure.

   \return
   - VMMC_statusOk      On success.
   - VMMC_statusInvalCh When there is no SDD on the addressed channel.
*/
IFX_int32_t VMMC_TAPI_LL_ALM_GR909_Result (IFX_TAPI_LL_CH_t *pLLChannel,
                                        IFX_TAPI_GR909_RESULT_t *pResults)
{
   VMMC_CHANNEL *pCh  = (VMMC_CHANNEL *) pLLChannel;
   VMMC_DEVICE  *pDev;
   VMMC_SDD_GR909ResultsRead_t *pPassFail;
   IFX_int32_t  ret;

   /* sanity check */
   if (pCh == IFX_NULL)
   {
      /* RETURN_STATUS needs a valid pointer in pCh to send a TAPI event. */
      return VMMC_statusInvalCh;
   }
   if (pCh->pALM == IFX_NULL)
   {
      /* errmsg: Resource not valid. Channel number out of range */
      RETURN_STATUS(VMMC_statusInvalCh);
   }

   pDev = pCh->pParent;
   pPassFail = &pCh->pALM->fw_sdd_GR909Results;

   pPassFail->CMD  = CMD_SDD;
   pPassFail->CHAN = pCh->nChannel - 1;
   pPassFail->MOD  = MOD_SDD;
   pPassFail->ECMD = SDD_GR909ResultsRead_ECMD;
   pPassFail->LENGTH = SDD_GR909ResultsRead_LEN;

   ret = CmdRead(pDev, (IFX_uint32_t *)((IFX_void_t *)pPassFail),
                       (IFX_uint32_t *)((IFX_void_t *)pPassFail),
                       pPassFail->LENGTH);

   if (!VMMC_SUCCESS(ret))
      RETURN_STATUS(ret);

   /* \todo check that tests are really finished (status registers?) */

   /* fill in validity information in the Results structure */
   pResults->valid  = pPassFail->HptValid ? IFX_TAPI_GR909_HPT : 0;
   pResults->valid |= pPassFail->FemfValid ? IFX_TAPI_GR909_FEMF : 0;
   pResults->valid |= pPassFail->RftValid ? IFX_TAPI_GR909_RFT : 0;
   pResults->valid |= pPassFail->RohValid ? IFX_TAPI_GR909_ROH : 0;
   pResults->valid |= pPassFail->RitValid ? IFX_TAPI_GR909_RIT : 0;
   /* fill in pass/fail information in the Results structure */
   pResults->passed  = pPassFail->HptPass ? IFX_TAPI_GR909_HPT : 0;
   pResults->passed |= pPassFail->FemfPass ? IFX_TAPI_GR909_FEMF : 0;
   pResults->passed |= pPassFail->RftPass ? IFX_TAPI_GR909_RFT : 0;
   pResults->passed |= pPassFail->RohPass ? IFX_TAPI_GR909_ROH : 0;
   pResults->passed |= pPassFail->RitPass ? IFX_TAPI_GR909_RIT : 0;

   /* read the test results */
   if (pPassFail->HptValid && pPassFail->HptPass)
   {
      pResults->HPT_AC_R2G = pPassFail->HptAcR2g;
      pResults->HPT_AC_T2G = pPassFail->HptAcT2g;
      pResults->HPT_AC_T2R = pPassFail->HptAcT2r;
      pResults->HPT_DC_R2G = pPassFail->HptDcR2g;
      pResults->HPT_DC_T2G = pPassFail->HptDcT2g;
      pResults->HPT_DC_T2R = pPassFail->HptDcT2r;
   }
   if (pPassFail->FemfValid && pPassFail->FemfPass)
   {
      /* FEMF results have the same placeholder as HPT results
         in a FW command */
      pResults->FEMF_AC_R2G = pPassFail->HptAcR2g;
      pResults->FEMF_AC_T2G = pPassFail->HptAcT2g;
      pResults->FEMF_AC_T2R = pPassFail->HptAcT2r;
      pResults->FEMF_DC_R2G = pPassFail->HptDcR2g;
      pResults->FEMF_DC_T2G = pPassFail->HptDcT2g;
      pResults->FEMF_DC_T2R = pPassFail->HptDcT2r;
   }
   if (pPassFail->RftValid && pPassFail->RftPass)
   {
      pResults->RFT_R2G = pPassFail->RftR2g;
      pResults->RFT_T2G = pPassFail->RftT2g;
      pResults->RFT_T2R = pPassFail->RftT2r;
   }
   if (pPassFail->RohValid && pPassFail->RohPass)
   {
      pResults->ROH_T2R_L = pPassFail->RohLow;
      pResults->ROH_T2R_H = pPassFail->RohHigh;
   }
   if (pPassFail->RitValid && pPassFail->RitPass)
   {
      pResults->RIT_RES = pPassFail->RitRes;
   }

   pResults->dev_type = IFX_TAPI_GR909_DEV_VMMC;
   pResults->OLR_T2R = pCh->pALM->nlt_ResistanceConfig.fOlResTip2Ring;
   pResults->OLR_T2G = pCh->pALM->nlt_ResistanceConfig.fOlResTip2Gnd;
   pResults->OLR_R2G = pCh->pALM->nlt_ResistanceConfig.fOlResRing2Gnd;

   RETURN_STATUS(ret);
}

/**
   This function is used to configure the measurement path for line testing
   according to Rmes resitor. Used by the IFX_TAPI_NLT_CONFIGURATION_RMES_SET
   command.

   \param  pLLChannel   Pointer to the TAPI LL channel structure.
   \param  pConfig      Pointer to the IFX_TAPI_NLT_CONFIGURATION_RMES_t.

   \return
   - VMMC_statusOk or status error
 */
IFX_int32_t VMMC_TAPI_LL_ALM_NLT_RmesConfig_Set(
                        IFX_TAPI_LL_CH_t *pLLChannel,
                        IFX_TAPI_NLT_CONFIGURATION_RMES_t *pConfig)
{
   VMMC_CHANNEL                       *pCh  = (VMMC_CHANNEL *) pLLChannel;
   IFX_int32_t                        ret = VMMC_statusOk;

   /* calling function should ensure valid parameters */
   VMMC_ASSERT(pCh != IFX_NULL);
   VMMC_ASSERT(pConfig != IFX_NULL);

   /* protect channel from mutual access */
   VMMC_OS_MutexGet(&pCh->chAcc);

   if ((pConfig->nRmeas == IFX_TAPI_NLT_RMEAS_1_5MOHM) ||
       (pConfig->nRmeas == IFX_TAPI_NLT_RMEAS_DEFAULT))
   {
      pCh->pALM->nRmeas = pConfig->nRmeas;
   }
   else
   {
      /** errmsg: Invalid Rmes value */
      ret = VMMC_statusInvalidRmes;
   }

   /* release channel */
   VMMC_OS_MutexRelease(&pCh->chAcc);
   RETURN_STATUS(ret);
}
#endif /* VMMC_FEAT_GR909 */
