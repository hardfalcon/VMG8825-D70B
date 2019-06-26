#ifndef _DRV_VMMC_GR909_H
#define _DRV_VMMC_GR909_H
/******************************************************************************

                              Copyright (c) 2012
                            Lantiq Deutschland GmbH
                             http://www.lantiq.com

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/**
   \file drv_vmmc_gr909.h
*/

/* ========================================================================== */
/*                                 Includes                                   */
/* ========================================================================== */

/* ========================================================================== */
/*                             Macro definitions                              */
/* ========================================================================== */

/* ========================================================================== */
/*                             Type definitions                               */
/* ========================================================================== */

/* ========================================================================== */
/*                           Function prototypes                              */
/* ========================================================================== */
extern IFX_int32_t VMMC_TAPI_LL_ALM_GR909_Start (IFX_TAPI_LL_CH_t *pLLChannel,
                                        IFX_TAPI_GR909_START_t const *p_start);
extern IFX_int32_t VMMC_TAPI_LL_ALM_GR909_Stop (IFX_TAPI_LL_CH_t *pLLChannel);
extern IFX_int32_t VMMC_TAPI_LL_ALM_GR909_Result (IFX_TAPI_LL_CH_t *pLLChannel,
                                        IFX_TAPI_GR909_RESULT_t *pResults);
extern IFX_int32_t VMMC_TAPI_LL_ALM_NLT_RmesConfig_Set(
                                    IFX_TAPI_LL_CH_t *pLLChannel,
                                    IFX_TAPI_NLT_CONFIGURATION_RMES_t *pConfig);
#endif /* _DRV_VMMC_GR909_H */
