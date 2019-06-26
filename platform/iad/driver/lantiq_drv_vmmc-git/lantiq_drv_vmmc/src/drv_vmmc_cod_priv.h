#ifndef _DRV_VMMC_COD_PRIV_H
#define _DRV_VMMC_COD_PRIV_H
/******************************************************************************

                              Copyright (c) 2013
                            Lantiq Deutschland GmbH
                             http://www.lantiq.com

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/**
   \file drv_vmmc_cod_priv.h Private structure of the CODer module.
*/

/* ============================= */
/* Includes                      */
/* ============================= */
#include "drv_vmmc_fw_commands_voip.h"

/* ============================= */
/* Global Defines                */
/* ============================= */

/* ============================= */
/* Global Types                  */
/* ============================= */

/** Structure for the coder channel including firmware message cache */
struct VMMC_CODCH
{
   /** firmware message cache */
   COD_CHAN_SPEECH_t            fw_cod_ch_speech;
   COD_FAX_CTRL_t               fw_cod_fax_ctrl;
   COD_FAX_ACT_t                fw_cod_fax_act;
   COD_FAX_CONF_t               fw_cod_fax_conf;
   COD_FAX_FDP_PARAMS_t         fw_cod_fax_fdp_params;
   COD_FAX_STAT_t               fw_cod_fax_stat;
   COD_FAX_TRACE_t              fw_cod_fax_trace;
#if 0
   COD_FAX_MOD_CTRL_t           fw_cod_fax_mod;
   COD_FAX_DEMOD_CTRL_t         fw_cod_fax_demod;
#endif /* 0 */
   COD_CHAN_RTP_SUP_CFG_US_t    fw_cod_rtp_us_conf;
   COD_CHAN_RTP_SUP_CFG_DS_t    fw_cod_rtp_ds_conf;
   COD_JB_CONF_t                fw_cod_jb_conf;
   COD_JB_STAT_t                fw_cod_jb_stat;
   COD_RTP_SUP_CFG_t            fw_cod_rtp;
   COD_EVT_GEN_t                fw_cod_evt_gen;
   COD_RTCP_SUP_CTRL_t          fw_cod_rtcp_stat;
#ifdef VMMC_FEAT_RTCP_XR
   COD_RTCP_XR_STAT_SUM_t       fw_cod_rtcp_xr_stat_sum;
   COD_RTCP_XR_VOIP_MET_t       fw_cod_rtcp_xr_voip_met;
   COD_ASSOCIATED_LECNR_t       fw_cod_associated_lecnr;
#endif /* VMMC_FEAT_RTCP_XR */
   COD_DEC_STAT_t               fw_cod_dec_stat;
   COD_AGC_CTRL_t               fw_cod_agc_ctrl;
   RES_AGC_COEF_t               fw_res_agc_coeff;
#ifdef VMMC_FEAT_ANNOUNCEMENTS
   COD_ANN_CTRL_t               fw_cod_ann_ctrl;
#endif /* VMMC_FEAT_ANNOUNCEMENTS */
   CMD_COD_CFG_STAT_MOS_t       fw_cod_cfg_stat_mos;
   CMD_COD_READ_STAT_MOS_t      fw_cod_read_stat_mos;

   /* AMR codec specific data */
   IFX_uint8_t                  AMR_DecCMR;

   /** local storage for the configured encoder algorithm and encoder packet
       time. Stored outside the fw message so that decoder can be started
       while the encoder is stopped. The value is set when the sampling mode
       is set.*/
   IFX_uint8_t              enc_conf;
   IFX_uint8_t              pte_conf;
   IFX_uint8_t              bitr_conf;
   IFX_uint8_t              fec_conf;
   /* local storage for the silence compression bit. This bit has to be
      suppressed for certain coders so we store it outside the fw-message.
      It is set together with the encoder that is stored in enc_conf. */
   IFX_uint8_t              sc_bit;
   /** flag showing if the encoder is to be enabled. */
   IFX_boolean_t            enc_running;
   /** local storage for the current decoder. This is used for wideband to
       know if the decoder is currently decoding a wideband encoded stream. */
   IFX_uint8_t              curr_dec;
   /** local storage for RTP statistics */
   IFX_uint32_t             rtcp[7];
   IFX_boolean_t            rtcp_update;
   /** store currently configured SSRC */
   IFX_uint32_t             nSsrc;
   IFX_uint32_t             nRecBytesH;
   IFX_uint32_t             nRecBytesL;

#ifdef EVALUATION
   IFX_void_t               *pEval;
#endif /* #ifdef EVALUATION */
};

/* ============================= */
/* Global Variables              */
/* ============================= */

/* ============================= */
/* Global function declaration   */
/* ============================= */

#endif /* _DRV_VMMC_COD_PRIV_H */
