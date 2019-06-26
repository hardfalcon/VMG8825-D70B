#ifndef _DRV_VMMC_SIG_PRIV_H
#define _DRV_VMMC_SIG_PRIV_H
/******************************************************************************

                              Copyright (c) 2012
                            Lantiq Deutschland GmbH
                             http://www.lantiq.com

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/*
   \file drv_vmmc_sig_priv.h
   This file contains the private declarations of the SIGnalling module.
*/

/* ============================= */
/* Includes                      */
/* ============================= */
#include "ifx_types.h"
#include "drv_vmmc_api.h"
#include "drv_vmmc_fw_commands_voip.h"


/* ============================= */
/* Global Defines                */
/* ============================= */
#define VMMC_SIG_TX 1
#define VMMC_SIG_RX 2


/* ============================= */
/* Global Types                  */
/* ============================= */
/** DTMF transmission status
Note: The states ABORT and ERROR indicate the status of the /last/ DTMF
      transmission - a new transmission can be started right away without
      restriction. */
typedef enum
{
   /* DTMF generator idle */
   DTMF_READY,
   /* DTMF generator activated, waiting on first REQ */
   DTMF_START,
   /* DTMF generator transmitting */
   DTMF_TRANSMIT,
   /* DTMF transmission aborted (either by user or underrun) */
   DTMF_ABORT
} VMMC_DTMF_STATE;

/** FSK transmission status */
typedef enum
{
   VMMC_CID_STATE_SETUP     = 0,
   VMMC_CID_STATE_TRANSMIT  = 1
} VMMC_CID_STATE;

/* this structure is used to store status information of
   all signaling modules. It is used to store the event transmission
   status in the sigch structure and to switch off and on the modules */
typedef union
{
   IFX_uint16_t value;
   struct {
      unsigned dtmf_rec                      : 1;
      unsigned dtmfgen                       : 1;
      unsigned mftd                          : 1;
   } flag;
} SIG_MOD_STATE;

/* DTMF transmission structure */
typedef struct
{
   /* Prevent multiple use */
   IFX_vint32_t       useCnt;
   /* DTMF transmission state */
   VMMC_DTMF_STATE state;
   /* Callback function called on DTMF status changes */
   IFX_void_t         (*stateCb)(VMMC_CHANNEL *pCh);
   /* Pointer to DTMF data to send */
   IFX_uint16_t       *pData;
   /* Number of DTMF words to send*/
   IFX_uint16_t  nWords;
   /* Number of DTMF word currently in transmission */
   IFX_uint16_t  nCurr;
   /* Number of DTMF words sent */
   IFX_uint16_t  nSent;
   /* Simplified mode (expect 8bit DTMF words in pDtmfData,
      only support digits 0-D, only supports high-level
      frequency generation mode) if set */
   IFX_boolean_t bByteMode;
} VMMC_DTMF;

/** CID sending structure */
typedef struct
{
   IFX_uint8_t               pCid [258];
   IFX_TAPI_CID_DATA_TYPE_t  nCidDataType;
   IFX_vuint8_t              nCidCnt;
   IFX_vuint8_t              nPos;
   VMMC_CID_STATE            nState;
} VMMC_CID;

/** Structure for the signaling channel
   including firmware message cache */
struct VMMC_SIGCH
{
   SIG_CHAN_t         fw_sig_ch;
   SIG_DTMFATG_CTRL_t fw_sig_dtmfgen;
   RES_DTMFATG_COEF_t fw_sig_dtmfgen_coef;
   RES_DTMFATG_DATA_t fw_sig_dtmfatg_data;
   SIG_DTMFR_CTRL_t   fw_sig_dtmfrcv;
   SIG_DTMFR_CTRL_t   fw_sig_dtmfrcv_override;
   RES_DTMFR_COEF_t   fw_sig_dtmfrcv_coef;
   SIG_CPTD_CTRL_t    fw_sig_cptd;
   SIG_UTG_CTRL_t     fw_utg[LL_TAPI_TONE_MAXRES];
   RES_UTG_COEF_t     fw_sig_utg_coef;
   SIG_CIDR_CTRL_t    fw_sig_cidrcv;
   RES_CIDR_COEF_t    fw_sig_cidr_coef;
   SIG_CIDS_CTRL_t    fw_sig_cidsend;
   RES_CIDS_COEF_t    fw_sig_cids_coef;
   RES_CIDS_DATA_t    fw_sig_cids_data;
   SIG_RTP_SUP_t      fw_sig_rtp_sup;
   SIG_RTP_EVT_STAT_t fw_sig_rtp_evt;
   SIG_MFTD_CTRL_t    fw_sig_mftd;
   /* Event transmission status to be programmed on coder start */
   SIG_MOD_STATE      et_stat;
   /** Currently enabled line signals, zero means enabled.
      The current signal mask is set by the interface enable/disable signal */
   IFX_uint32_t       sigMask;
   IFX_uint32_t       sigMaskExt;
   /** stores the last detected tone of the MFTD for tone end detection */
   IFX_uint16_t       lastMftd1ToneIdx;
   IFX_uint16_t       lastMftd2ToneIdx;
   /* DTMF generator configuration */
   IFX_uint16_t       nDtmfInterDigitTime;
   IFX_uint16_t       nDtmfDigitPlayTime;
   VMMC_DTMF          dtmfSend;
   /* CID sender structure, need protection  */
   VMMC_CID           cidSend;
   /* FSK receiver data */
   IFX_uint16_t       nRxCount;
   /* Last DTMF sign needed for DTMF end reporting */
   IFX_uint8_t        nLastDtmfSign;
   /** Number of active CPTDs */
   IFX_uint8_t        nCPTD_Cnt;
   /** Index of tone detected by CPTD */
   IFX_uint8_t        nCPTD_ToneIndex[SIG_CPTD_CTRL_DATA_MAX];
   /* Indicates that DTMFD control is in override mode. */
   IFX_boolean_t      bDtmfdOverride;
   /* Store the current auto suppression state set by the OOB command. */
   IFX_boolean_t      bAutoSuppression;
   /* Indicates that DTMFD coefficients were modified. */
   IFX_boolean_t      bDtmfdCoeffModified;
   /* Cache for last read RTP statistic data */
   IFX_uint32_t       nRecBytesH;
   IFX_uint32_t       nRecBytesL;

#ifdef EVALUATION
   IFX_void_t         *pEval;
#endif /* #ifdef EVALUATION */
};


/* ============================= */
/* Global Variables              */
/* ============================= */
extern const IFX_uint32_t  tens [][2];

/* ============================= */
/* Global function declaration   */
/* ============================= */

extern IFX_int32_t VMMC_SIG_AutoChStop (
                        VMMC_CHANNEL *pCh,
                        IFX_boolean_t bOn);

extern IFX_void_t  VMMC_SIG_DTMF_InitCh (
                        VMMC_CHANNEL *pCh);

extern IFX_int32_t VMMC_SIG_DTMF_BaseConf (
                        VMMC_CHANNEL *pCh);

extern IFX_int32_t VMMC_SIG_DTMFD_Set (
                        VMMC_CHANNEL const *pCh,
                        IFX_boolean_t bEn,
                        IFX_uint8_t dir);

extern IFX_int32_t VMMC_SIG_DTMFG_CoeffSet (
                        VMMC_CHANNEL *pCh,
                        IFX_uint8_t nTIMT,
                        IFX_uint8_t nTIMP);

extern IFX_int32_t VMMC_SIG_DTMFG_Start (
                        VMMC_CHANNEL *pCh,
                        IFX_uint16_t *pDtmfData,
                        IFX_uint16_t nDtmfWords,
                        IFX_uint32_t nFG,
                        IFX_void_t (*cbDtmfStatus)(VMMC_CHANNEL *pCh),
                        IFX_boolean_t bByteMode);

extern IFX_return_t VMMC_SIG_DTMF_encode_ascii2fw (
                        IFX_char_t nChar,
                        IFX_uint8_t *pDtmfCode);

extern IFX_int32_t VMMC_SIG_MFTD_Set (
                        VMMC_CHANNEL *pCh,
                        IFX_uint32_t nSignal,
                        IFX_uint32_t nSignalExt);

extern IFX_int32_t VMMC_SIG_MFTD_EvtTransSet (
                        VMMC_CHANNEL *pCh,
                        IFX_boolean_t bEnable);

extern IFX_int32_t VMMC_TAPI_LL_SIG_MFTD_OOB (
                        IFX_TAPI_LL_CH_t *pLLChannel,
                        IFX_TAPI_PKT_EV_OOB_t nOobMode);

extern IFX_int32_t  VMMC_TAPI_LL_SIG_DTMFD_Start (
                        IFX_TAPI_LL_CH_t *pLLChannel,
                        IFX_TAPI_LL_DTMFD_CFG_t const *pCfg);

extern IFX_int32_t  VMMC_TAPI_LL_SIG_DTMFD_Stop (
                        IFX_TAPI_LL_CH_t *pLLChannel,
                        IFX_TAPI_LL_DTMFD_CFG_t const *pCfg);

extern IFX_int32_t  VMMC_TAPI_LL_SIG_DTMFG_Cfg (
                        IFX_TAPI_LL_CH_t *pLLChannel,
                        IFX_uint16_t nInterDigitTime,
                        IFX_uint16_t nDigitPlayTime);

extern IFX_int32_t  VMMC_TAPI_LL_SIG_DTMFG_Start (
                        IFX_TAPI_LL_CH_t *pLLChannel,
                        IFX_uint8_t nDigits,
                        IFX_char_t const *data);

extern IFX_int32_t  VMMC_TAPI_LL_SIG_DTMFG_Stop (
                        IFX_TAPI_LL_CH_t *pLLChannel);

extern IFX_return_t VMMC_TAPI_LL_SIG_DTMF_RX_CFG (
                        IFX_TAPI_LL_CH_t *pLLChannel,
                        IFX_boolean_t bRW,
                        IFX_TAPI_DTMF_RX_CFG_t *pDtmfRxCoeff);

extern IFX_int32_t VMMC_TAPI_LL_SIG_DTMFD_OOB (
                        IFX_TAPI_LL_CH_t *pLLChannel,
                        IFX_TAPI_PKT_EV_OOB_t nOobMode);

extern IFX_int32_t  VMMC_TAPI_LL_SIG_DTMFD_Override (
                        IFX_TAPI_LL_CH_t *pLLChannel,
                        IFX_TAPI_LL_DTMFD_OVERRIDE_t const *pCfg);

extern IFX_void_t   VMMC_SIG_CID_InitCh (
                        VMMC_CHANNEL *pCh);

extern IFX_int32_t  VMMC_SIG_CID_BaseConf (
                        VMMC_CHANNEL *pCh);

#endif  /* _DRV_VMMC_SIG_PRIV_H */
