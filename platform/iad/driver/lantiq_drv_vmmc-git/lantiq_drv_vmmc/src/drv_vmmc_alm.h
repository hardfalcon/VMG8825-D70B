#ifndef _DRV_VMMC_ALM_H
#define _DRV_VMMC_ALM_H
/****************************************************************************

                              Copyright (c) 2013
                            Lantiq Deutschland GmbH
                             http://www.lantiq.com

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/*
   \file drv_vmmc_alm.h
   This file contains the external interface of the ALM module.
*/

/* ============================= */
/* Includes                      */
/* ============================= */

/* ============================= */
/* Global Defines                */
/* ============================= */
#define MAX_ALM_NUM 3
#define SDD_EVT_TIMEOUT_MS 100
#define OPMODE_IGNORED 255

/* ============================= */
/* Local Macros & Definitions    */
/* ============================= */

/* ============================= */
/* Global Structures             */
/* ============================= */
/* ALM line type */
typedef enum
{
   VMMC_ALM_LINE_FXS = 0,
   VMMC_ALM_LINE_FXO
} VMMC_ALM_LINE_TYPE_t;

/* ============================= */
/* Global function declaration   */
/* ============================= */

extern IFX_void_t  VMMC_ALM_Func_Register (
                        IFX_TAPI_DRV_CTX_ALM_t *pAlm);

extern IFX_int32_t VMMC_ALM_Allocate_Ch_Structures (
                        VMMC_CHANNEL *pCh);

extern IFX_void_t  VMMC_ALM_Free_Ch_Structures (
                        VMMC_CHANNEL *pCh);

extern IFX_void_t  VMMC_ALM_InitCh (
                        VMMC_CHANNEL *pCh);

extern IFX_int32_t VMMC_ALM_Set_Inputs (
                        VMMC_CHANNEL *pCh);

extern IFX_int32_t VMMC_ALM_ChStop (
                        VMMC_CHANNEL *pCh);

extern IFX_int32_t VMMC_ALM_baseConf (
                        VMMC_CHANNEL *pCh);

extern IFX_int32_t VMMC_ALM_SamplingMode (
                        VMMC_CHANNEL *pCh,
                        SM_ACTION action,
                        OPMODE_SMPL sigarray_mode,
                        OPMODE_SMPL module_mode);

extern IFX_void_t  irq_VMMC_ALM_LineDisable (
                        VMMC_CHANNEL *pCh);

#ifdef VMMC_FEAT_SLIC
extern IFX_boolean_t VMMC_ALM_SmartSLIC_IsConnected (
                        VMMC_DEVICE *pDev);

extern IFX_int32_t VMMC_ALM_SmartSLIC_ChGet (
                        VMMC_DEVICE *pDev,
                        IFX_uint8_t *nChannels,
                        IFX_uint8_t *nFXOChannels);

extern IFX_int32_t VMMC_SDD_VersionRead (
                        VMMC_DEVICE *pDev);
#endif /* VMMC_FEAT_SLIC */

extern VMMC_CHANNEL *VMMC_ALM_FxsNeighbourChGet (
                        VMMC_CHANNEL *pCh);

extern IFX_int32_t VMMC_ALM_MWL_DefaultConf (
                        VMMC_CHANNEL *pCh);

extern IFX_void_t irq_VMMC_ALM_UpdateOpModeAndWakeUp (
                        VMMC_CHANNEL *pCh,
                        IFX_uint8_t lm);

extern IFX_int32_t VMMC_ALM_OpmodeSet (
                        VMMC_CHANNEL *pCh);

extern IFX_int32_t VMMC_ALM_OpmodeModeSet (
                        VMMC_CHANNEL *pCh,
                        IFX_uint32_t nOperatingMode);

extern IFX_int32_t VMMC_ALM_OpmodeGet (
                        VMMC_CHANNEL *pCh,
                        IFX_uint8_t *pCurrentOpmode);

#ifdef VMMC_FEAT_CLOCK_SCALING
extern IFX_boolean_t irq_VMMC_ALM_DartCanSleep (
                        VMMC_CHANNEL *pCh);
#endif /* VMMC_FEAT_CLOCK_SCALING */

extern IFX_int32_t VMMC_ALM_Calibration (
                        VMMC_CHANNEL *pCh);

#endif /* _DRV_VMMC_ALM_H */
