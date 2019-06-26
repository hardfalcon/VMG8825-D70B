#ifndef DRV_TAPI_FXO_H
#define DRV_TAPI_FXO_H
/******************************************************************************

                            Copyright (c) 2014, 2016
                        Lantiq Beteiligungs-GmbH & Co.KG

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/**
   \file drv_tapi_fxo.h
   This file contains the declaration for the FXO feature.
*/

#ifdef TAPI_FEAT_FXO
/* ========================================================================== */
/*                           Function prototypes                              */
/* ========================================================================== */

extern IFX_int32_t TAPI_FXO_Register_DAA (
                        TAPI_CHANNEL *pChannel,
                        IFX_int32_t nDAA);

extern IFX_int32_t TAPI_FXO_Init_DAA (
                        TAPI_CHANNEL *pChannel);

extern IFX_int32_t IFX_TAPI_FXO_Dial_Cfg_Set (
                        TAPI_CHANNEL *pChannel,
                        IFX_TAPI_FXO_DIAL_CFG_t const *p_DialCfg);

extern IFX_int32_t IFX_TAPI_FXO_Flash_Cfg_Set (
                        TAPI_CHANNEL *pChannel,
                        IFX_TAPI_FXO_FLASH_CFG_t const *p_fhCfg);

extern IFX_int32_t IFX_TAPI_FXO_OSI_Cfg_Set (
                        TAPI_CHANNEL *pChannel,
                        IFX_TAPI_FXO_OSI_CFG_t const *p_osiCfg);

extern IFX_int32_t IFX_TAPI_FXO_Dial_Start (
                        TAPI_CHANNEL *pChannel,
                        IFX_TAPI_FXO_DIAL_t const *p_DialData);

extern IFX_int32_t IFX_TAPI_FXO_Hook_Set (
                        TAPI_CHANNEL *pChannel,
                        IFX_TAPI_FXO_HOOK_t nHook);

extern IFX_int32_t IFX_TAPI_FXO_Flash_Set (
                        TAPI_CHANNEL *pChannel);

extern IFX_int32_t IFX_TAPI_FXO_Bat_Get (
                        TAPI_CHANNEL *pChannel,
                        IFX_TAPI_FXO_BAT_t *pBat);

extern IFX_int32_t IFX_TAPI_FXO_Hook_Get (
                        TAPI_CHANNEL *pChannel,
                        IFX_TAPI_FXO_HOOK_CFG_t *pHook);

extern IFX_int32_t IFX_TAPI_FXO_Apoh_Get (
                        TAPI_CHANNEL *pChannel,
                        IFX_TAPI_FXO_APOH_t *pApoh);

extern IFX_int32_t IFX_TAPI_FXO_Polarity_Get (
                        TAPI_CHANNEL *pChannel,
                        IFX_TAPI_FXO_POLARITY_t *pPol);

extern IFX_int32_t IFX_TAPI_FXO_Ring_Get (
                        TAPI_CHANNEL *pChannel,
                        IFX_TAPI_FXO_RING_STATUS_t *pRing);
#endif /* TAPI_FEAT_FXO */

#endif /* DRV_TAPI_FXO_H */
