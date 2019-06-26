#ifndef _DRV_VMMC_ALM_PRIV_H
#define _DRV_VMMC_ALM_PRIV_H
/******************************************************************************

                              Copyright (c) 2013
                            Lantiq Deutschland GmbH
                             http://www.lantiq.com

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************
   Module      : drv_vmmc_alm_priv.h
   Description : This file contains the defines, the structures declarations
                 for ALM module.
*******************************************************************************/

/* includes */
#include "drv_tapi_ll_interface.h"
#include "drv_vmmc_fw_commands_voip.h"
#include "drv_vmmc_fw_commands_sdd.h"
#include "drv_vmmc_res.h"


/******************************************************************************
** Firmware Analog Line Interface Channel Cmd
**
*******************************************************************************/

#ifdef VMMC_FEAT_CAP_MEASURE
typedef enum
{
  /* C-measurement is not active (initial) */
  VMMC_CapMeasState_Inactive,
  /* C-measurement is started */
  VMMC_CapMeasState_Started
} VMMC_ALM_CAP_MEAS_STATE_t;
#endif /* VMMC_FEAT_CAP_MEASURE */

/** Structure for the analog line module channel
    including firmware message cache */
struct VMMC_ALMCH
{
   /* Current line operating mode - reflects the actual opmode.
      Written in interrupt context by the firmware event,
      read by opmode change requesting thread to ensure that the
      opmode transition is complete */
   volatile IFX_uint8_t          curr_opmode;
   IFX_uint8_t                   curr_polarity;
   IFX_uint8_t                   curr_howler;
   IFX_uint8_t                   curr_mwi_lamp;
   volatile IFX_boolean_t        bOpmodeChangePending;
   /* SDD event */
   VMMC_OS_event_t               sdd_event;
   /* Analog Line Interface channel command cache */
   ALI_CHAN_t                    fw_ali_ch;
   /* Cached FW message for calibration */
   VMMC_SDD_Calibrate_t          fw_sdd_calibrate,
   /* Cache for the last calibration results */
                                 calibrationLastResults;
   /* the current NB/WB mode */
   OPMODE_SMPL                   module_mode;
   /* line type configured to FXS == IFX_TRUE or FXO == IFX_FALSE */
   IFX_boolean_t                 line_type_fxs;
   /* Type of DC/DC converter */
   enum VMMC_DCDC_TYPE           nDcDcType;
   /* the storage of battery, apoh, polarity and ring status for FXO line */
   volatile IFX_uint32_t         fxo_flags;
   /* The echo suppressor resource id */
   VMMC_RES_ID_t                 nEsResId;
   /* The line echo canceller resource id */
   VMMC_RES_ID_t                 nLecResId;
#if defined(VMMC_FEAT_GR909) || defined(VMMC_FEAT_CAP_MEASURE)
   /* Cached FW message for SDD_GR909Config */
   VMMC_SDD_GR909Config_t        fw_sdd_GR909Config;
#endif
#ifdef VMMC_FEAT_GR909
   /* IFX_TRUE if ring config stored for this channel */
   IFX_boolean_t                 b_ring_cfg;
   /* Previous ring frequency */
   IFX_uint16_t                  ring_freq_prev;
   /* Ring configuration */
   VMMC_SDD_RingConfig_t         ring_cfg;
   /* IFX_TRUE: GR909 limits are set / IFX_FALSE: GR909 limits
      aren't set, so set defaults */
   IFX_boolean_t                 b_GR909_limits;
   /* Cached FW message for SDD_GR909ResultsRead */
   VMMC_SDD_GR909ResultsRead_t   fw_sdd_GR909Results;
#endif /* VMMC_FEAT_GR909 */
   /* State of the calibration */
   IFX_TAPI_CALIBRATION_STATE_t  nCalibrationState;
   /* Indicates the current state of the FW */
   IFX_boolean_t                 bCalibrationRunning;
   /* Indicates that calibration was called driver internal. */
   IFX_boolean_t                 bCalibrationInternal;
   /* Indicates that calibration is needed after BBD download. */
   IFX_boolean_t                 bCalibrationNeeded;
   /* Semaphore to defer until calibration has completed. */
   VMMC_OS_mutex_t               mtxCalibrationWait;

   /* Cached FW message for SDD_BasicConfig */
   VMMC_SDD_BasicConfig_t        fw_sdd_basic_config;
   /* IdleExt bit of SDD_BasicConfig restoration indicator */
   IFX_boolean_t                 bIdleExtNeedRestore;
#ifdef VMMC_FEAT_CONT_MEASUREMENT
   /* Cached FW message for SDD_ContMeasRead */
   VMMC_SDD_ContMeasRead_t       fw_sdd_contMeasRead;
   /* Cached FW message for SDD_ContMeasClear */
   VMMC_SDD_ContMeasClear_t      fw_sdd_contMearClear;
#endif /* VMMC_FEAT_CONT_MEASUREMENT */
   /* Cached FW message for SDD_FxoConfig */
   VMMC_SDD_FxoConfig_t          fw_sdd_fxoConfig;
   /* Cached FW message for SDD_FxoHookSwitch */
   VMMC_SDD_FxoHookSwitch_t      fw_sdd_fxoHookSwitch;
#ifdef VMMC_FEAT_CAP_MEASURE
   /* Cached FW message for SDD_GR909PhoneDetection */
   VMMC_SDD_GR909PhoneDetection_t fw_sdd_phone_detection;
   /* State of capacitance measurement */
   volatile VMMC_ALM_CAP_MEAS_STATE_t eCapMeasState;
   /* IFX_TRUE - only Tip to Ring capacitance is measured. */
   IFX_boolean_t                 bCapMeasTip2RingOnly;
#endif /* VMMC_FEAT_CAP_MEASURE */
#ifdef VMMC_FEAT_NLT
   /* Tip to Ring Capacitance measurement result. */
   IFX_TAPI_NLT_T2R_CAPACITANCE_RESULT_t    t2r_cap_result;
   /* Line to Ground Capacitance measurement result. */
   IFX_TAPI_NLT_L2GND_CAPACITANCE_RESULT_t  l2g_cap_result;
   /* open loop capacitance configuration. */
   IFX_TAPI_NLT_OL_CAPACITANCE_CONF_t nlt_CapacitanceConfig;
   /* open loop resistance configuration. */
   IFX_TAPI_NLT_OL_RESISTANCE_CONF_t  nlt_ResistanceConfig;
   /* Configuration which Rmeas variant is connected to the chip. */
   IFX_TAPI_NLT_RMEAS_CFG_t           nRmeas;
#endif /* (VMMC_CFG_FEATURES & VMMC_CFG_ADD_FEAT_NLT) */
   /* Cached FW message for SDD_Opmode */
   VMMC_SDD_Opmode_t             fw_sdd_opmode,
                                 fw_last_sdd_opmode;
   /* Cached FW message for SDD_MwiConfig */
   VMMC_SDD_MwiConfig_t          fw_sdd_mwi_config;
   /* temporary storage for basic configuration (required for MWL feature) */
   VMMC_SDD_BasicConfig_t        sdd_basic_config_tmp_mwl;
   /* temporary storage for ring configuration (required for MWL feature) */
   VMMC_SDD_RingConfig_t         sdd_ring_config_tmp_mwl;
   /* elapsed time since last hook event */
   IFX_uint32_t                  nLastHookEvtTimestamp;
   Timer_ID                      nHookWindowTimerId;
   volatile IFX_boolean_t        bHookWindow;
#ifdef VMMC_FEAT_CLOCK_SCALING
   /* IFX_FALSE indicates that the DART channel is active. IFX_TRUE indicates
   that linemode allows the DART to sleep. */
   volatile IFX_boolean_t        bDartCanSleep;
#endif /* VMMC_FEAT_CLOCK_SCALING */
#ifdef EVALUATION
   IFX_void_t                    *pEval;
#endif /* #ifdef EVALUATION */
   /* Indicates override of the Standby DUP time configuration. */
   IFX_uint8_t                   nStdbyDupTimeOverride;
};


IFX_int32_t vmmc_BBD_UpdateBasicConfigCache (VMMC_CHANNEL *pCh);

#endif /* _DRV_VMMC_ALM_PRIV_H */
