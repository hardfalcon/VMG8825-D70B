#ifndef _DRV_VMMC_INT_EVT_H_
#define _DRV_VMMC_INT_EVT_H_
/******************************************************************************

                              Copyright (c) 2013
                            Lantiq Deutschland GmbH
                             http://www.lantiq.com

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/
/*
   \file drv_vmmc_int_evt.h
   This file contains the definitions of the voice-FW event message interface.
*/

#ifndef __PACKED__
   #if defined (__GNUC__) || defined (__GNUG__)
      /* GNU C or C++ compiler */
      #define __PACKED__ __attribute__ ((packed))
   #elif !defined (__PACKED__)
      #define __PACKED__      /* nothing */
   #endif
#endif


/* ----- Common Event Header ----- */
#define CMD_EVT 9

/* Subcommand coding for events */
#define EVT_MOD_PCM 0
#define EVT_MOD_ALI 1
#define EVT_MOD_SIG 2
#define EVT_MOD_COD 3
#define EVT_MOD_VOICEREC 5

#define EVT_HEAD_BE           \
   /** Reserved */            \
   IFX_uint32_t ResHead00 : 3;\
   /** Command type */        \
   IFX_uint32_t CMD : 5;      \
   /** Reserved */            \
   IFX_uint32_t ResHead01 : 4;\
   /** Channel */             \
   IFX_uint32_t CHAN : 4;     \
   /** Mode of subcommand */  \
   IFX_uint32_t MOD : 3;      \
   /** Subcommand type */     \
   IFX_uint32_t ECMD : 5;     \
   /** Length */              \
   IFX_uint32_t LENGTH : 8


/* ----- Definition of event messages ----- */

/** Over Temperature */
typedef struct EVT_ALI_OVT
{
   EVT_HEAD_BE;
} __PACKED__ EVT_ALI_OVT_t;

#define EVT_ALI_OVT_ECMD 0
#define EVT_ALI_OVT_LEN 0


/** Hook Event */
typedef struct EVT_ALI_RAW_HOOK
{
   EVT_HEAD_BE;
   /** Timestamp */
   IFX_uint32_t TIME_STAMP : 16;
   /** Reserved */
   IFX_uint32_t Res02 : 15;
   /** Raw hook event */
   IFX_uint32_t RON : 1;
} __PACKED__ EVT_ALI_RAW_HOOK_t;

#define EVT_ALI_RAW_HOOK_ECMD 1
#define EVT_ALI_RAW_HOOK_RON_ONHOOK 0
#define EVT_ALI_RAW_HOOK_RON_OFFHOOK 1


/** Line Testing Finished */
typedef struct EVT_ALI_LT_END
{
   EVT_HEAD_BE;
} __PACKED__ EVT_ALI_LT_END_t;

#define EVT_ALI_LT_END_ECMD 3


/**  ARX DCCONTROL Event */
typedef struct EVT_ALI_AR9DCC
{
   EVT_HEAD_BE;
   /** Reserved */
   IFX_uint32_t Res02 : 20;
   /** Analog line operating mode */
   IFX_uint32_t OPMODE : 4;
   /** Reserved */
   IFX_uint32_t Res03 : 3;
   /** Event code */
   IFX_uint32_t EVT : 5;
} __PACKED__ EVT_ALI_AR9DCC_t;

#define EVT_ALI_AR9DCC_ECMD 4
#define EVT_ALI_AR9DCC_LEN 4
#define EVT_ALI_AR9DCC_EVT_GF 3
#define EVT_ALI_AR9DCC_EVT_GK 4
#define EVT_ALI_AR9DCC_EVT_OPC 5
#define EVT_ALI_AR9DCC_EVT_CORCE 6
#define EVT_ALI_AR9DCC_EVT_COEFE 7
#define EVT_ALI_AR9DCC_EVT_TTXF  8
#define EVT_ALI_AR9DCC_EVT_GF_FIN 12
#define EVT_ALI_AR9DCC_EVT_GK_FIN 13
#define EVT_ALI_AR9DCC_EVT_OTEMP_FIN 14
#define EVT_ALI_AR9DCC_EVT_SSI_CRASH 15
#define EVT_ALI_AR9DCC_EVT_SSI_CRASH_FIN 16
#define EVT_ALI_AR9DCC_EVT_OMI 17
#define EVT_ALI_AR9DCC_EVT_DART_IN_SLEEP 18
#define EVT_ALI_AR9DCC_EVT_DART_WAKEUP_REQ 19


/** Decoder Change Event */
typedef struct EVT_COD_DEC_CHANGE
{
   EVT_HEAD_BE;
   /** Reserved */
   IFX_uint32_t Res02 : 9;
   /** Decoder Change */
   IFX_uint32_t DC : 1;
   /** Codec mode request  */
   IFX_uint32_t CMR : 1;
   /** Type of Decoder */
   IFX_uint32_t DEC : 5;
   /** Reserved */
   IFX_uint32_t Res03 : 12;
   /** Codec mode request code */
   IFX_uint32_t CMRC : 4;
} __PACKED__ EVT_COD_DEC_CHANGE_t;

#define EVT_COD_DEC_CHANGE_ECMD 0
#define EVT_COD_DEC_CHANGE_LEN 4
#define EVT_COD_DEC_CHANGE_DC_NO 0
#define EVT_COD_DEC_CHANGE_DC_YES 1
#define EVT_COD_DEC_CHANGE_CMRC_NO 0
#define EVT_COD_DEC_CHANGE_CMRC_YES 1


/** Voice Playout Unit Event */
typedef struct EVT_COD_VPOU_LIMIT
{
   EVT_HEAD_BE;
   /** Reserved */
   IFX_uint32_t Res02 : 30;
   /** EVENT */
   IFX_uint32_t EV : 2;
} __PACKED__ EVT_COD_VPOU_LIMIT_t;

#define EVT_COD_VPOU_LIMIT_ECMD 1
#define EVT_COD_VPOU_LIMIT_EV_BACK 0
#define EVT_COD_VPOU_LIMIT_EV_LOW 1
#define EVT_COD_VPOU_LIMIT_EV_HIGH 3


/** Voice Playout Unit Statistics Change */
typedef struct EVT_COD_VPOU_STAT
{
   EVT_HEAD_BE;
} __PACKED__ EVT_COD_VPOU_STAT_t;

#define EVT_COD_VPOU_STAT_ECMD 2


/** Linear Channel Data Request Event */
typedef struct EVT_COD_LIN_REQ
{
   EVT_HEAD_BE;
} __PACKED__ EVT_COD_LIN_REQ_t;

#define EVT_COD_LIN_REQ_ECMD 7
#define EVT_COD_LIN_REQ_LEN 0


/** FAX Data Pump Request */
typedef struct EVT_COD_FAX_REQ
{
   EVT_HEAD_BE;
   /** Reserved */
   IFX_uint32_t Res02 : 16;
   /** Event type */
   IFX_uint32_t FDP_EVT : 16;
} __PACKED__ EVT_COD_FAX_REQ_t;

#define EVT_COD_FAX_REQ_ECMD 12
#define EVT_COD_FAX_REQ_LEN 4
#define EVT_COD_FAX_REQ_FDP_EVT_FDP_REQ 0
#define EVT_COD_FAX_REQ_FDP_EVT_MBSU 1
#define EVT_COD_FAX_REQ_FDP_EVT_DBSO 2
#define EVT_COD_FAX_REQ_FDP_EVT_MBDO 3
#define EVT_COD_FAX_REQ_FDP_EVT_MBDU 4
#define EVT_COD_FAX_REQ_FDP_EVT_DBDO 5


/** RFC 2833 Event */
typedef struct EVT_SIG_RFCDET
{
   EVT_HEAD_BE;
   /** Event */
   IFX_uint32_t EVT : 8;
   /** Reserved */
   IFX_uint32_t Res02 : 11;
   /** Phase reversal change */
   IFX_uint32_t PR_CHG : 1;
   /** DTMF tone  */
   IFX_uint32_t DTMF : 4;
   /** Reserved */
   IFX_uint32_t Res03 : 1;
   /** Tone which was received as an event */
   IFX_uint32_t TONE : 7;
} __PACKED__ EVT_SIG_RFCDET_t;

#define EVT_SIG_RFCDET_ECMD 6
#define EVT_SIG_RFCDET_TONE_DTMF 0x1
#define EVT_SIG_RFCDET_TONE_ANS 0x2
#define EVT_SIG_RFCDET_TONE__ANS 0x4
#define EVT_SIG_RFCDET_TONE_ANSAM 0x8
#define EVT_SIG_RFCDET_TONE__ANSAM 0x10
#define EVT_SIG_RFCDET_TONE_CNG 0x20
#define EVT_SIG_RFCDET_TONE_DIS 0x40


/** RFC 2833 Statistics Change Event */
typedef struct EVT_SIG_RFCSTAT
{
   EVT_HEAD_BE;
} __PACKED__ EVT_SIG_RFCSTAT_t;

#define EVT_SIG_RFCSTAT_ECMD 4
#define EVT_SIG_RFCSTAT_LEN 0


/** Universal Tone Generator 1 Event */
typedef struct EVT_SIG_UTG1
{
   EVT_HEAD_BE;
} __PACKED__ EVT_SIG_UTG1_t;

#define EVT_SIG_UTG1_ECMD 7
#define EVT_SIG_UTG1_LEN 0


/** Universal Tone Generator 2 Event */
typedef struct EVT_SIG_UTG2
{
   EVT_HEAD_BE;
} __PACKED__ EVT_SIG_UTG2_t;

#define EVT_SIG_UTG2_ECMD 8


/** DTMF Generator Event */
typedef struct EVT_SIG_DTMFG
{
   EVT_HEAD_BE;
   /** Reserved */
   IFX_uint32_t Res02 : 29;
   /** Event */
   IFX_uint32_t EVENT : 3;
} __PACKED__ EVT_SIG_DTMFG_t;

#define EVT_SIG_DTMFG_ECMD 2
#define EVT_SIG_DTMFG_EVENT_READY 0x1
#define EVT_SIG_DTMFG_EVENT_BUF_REQUEST 0x2
#define EVT_SIG_DTMFG_EVENT_BUF_UNDERFLOW 0x4


/** Caller ID Sender Event */
typedef struct EVT_SIG_CIDS
{
   EVT_HEAD_BE;
   /** Reserved */
   IFX_uint32_t Res02 : 29;
   /** Event */
   IFX_uint32_t EVENT : 3;
} __PACKED__ EVT_SIG_CIDS_t;

#define EVT_SIG_CIDS_ECMD 3
#define EVT_SIG_CIDS_EVENT_READY 0x1
#define EVT_SIG_CIDS_EVENT_BUF_REQUEST 0x2
#define EVT_SIG_CIDS_EVENT_BUF_UNDERFLOW 0x4


/** DTMF Receiver Event */
typedef struct EVT_SIG_DTMFD
{
   EVT_HEAD_BE;
   /** Reserved */
   IFX_uint32_t Res02 : 26;
   /** Signal Path */
   IFX_uint32_t I : 1;
   /** Event */
   IFX_uint32_t EVT : 1;
   /** DTMF Code */
   IFX_uint32_t DTMF : 4;
} __PACKED__ EVT_SIG_DTMFD_t;

#define EVT_SIG_DTMFD_ECMD 0
#define EVT_SIG_DTMFD_I_I1 0
#define EVT_SIG_DTMFD_I_I2 1
#define EVT_SIG_DTMFD_EVT_DTMF_END 0
#define EVT_SIG_DTMFD_EVT_DTMF_START 1


/** Call Progress Tone Detector Event */
typedef struct EVT_SIG_CPTD
{
   EVT_HEAD_BE;
   /** Reserved */
   IFX_uint32_t Res02 : 28;
   /* CPTD structure number in SIG_CPTD_CTRL_t that detected tone */
   IFX_uint32_t CP : 2;
   /** Signal Path */
   IFX_uint32_t I : 2;
} __PACKED__ EVT_SIG_CPTD_t;

#define EVT_SIG_CPTD_ECMD 1
#define EVT_SIG_CPTD_I_I1 0
#define EVT_SIG_CPTD_I_I2 1
#define EVT_SIG_CPTD_I_I12 2


/** MFTD Detector Event */
typedef struct EVT_SIG_MFTD
{
   EVT_HEAD_BE;
   /** Reserved */
   IFX_uint32_t Res02 : 20;
   /** MFTD tone change detected in channel 1 */
   IFX_uint32_t I1 : 1;
   /** MFTD tone change detected in channel 2 */
   IFX_uint32_t I2 : 1;
   /** Detected MFTD tone in channel 1 */
   IFX_uint32_t MFTD1 : 5;
   /** Detected MFTD tone in channel 2 */
   IFX_uint32_t MFTD2 : 5;
} __PACKED__ EVT_SIG_MFTD_t;

#define EVT_SIG_MFTD_ECMD 5
#define EVT_SIG_MFTD_I1_NO 0
#define EVT_SIG_MFTD_I1_YES 1
#define EVT_SIG_MFTD_I2_NO 0
#define EVT_SIG_MFTD_I2_YES 1


/** Voice Recognition Event  */
typedef struct EVT_VR_STAT
{
   EVT_HEAD_BE;
   /** Reserved */
   IFX_uint32_t Res02 : 29;
   /** Error */
   IFX_uint32_t ERR : 1;
   /** Invalid word */
   IFX_uint32_t IVW : 1;
   /** Request */
   IFX_uint32_t REQ : 1;
} __PACKED__ EVT_VR_STAT_t;

#define EVT_VR_STAT_ECMD 0
#define EVT_VR_STAT_IVW_TOO_SHORT 0
#define EVT_VR_STAT_IVW_TOO_LONG 0


/** T.38 FAX State Change Event */
typedef struct EVT_COD_FAX_STATE
{
   EVT_HEAD_BE;
   /** Reserved */
   IFX_uint32_t Res01 : 16;
   /** State of the facsimile transmission */
   IFX_uint32_t T38_FAX_STATE : 16;
} __PACKED__ EVT_COD_FAX_STATE_t;

#define EVT_COD_FAX_STATE_ECMD 10
#define EVT_COD_FAX_STATE_LEN 4
#define EVT_COD_FAX_STATE_ST_NEG 0x3
#define EVT_COD_FAX_STATE_ST_MOD 0x4
#define EVT_COD_FAX_STATE_ST_DEM 0x5
#define EVT_COD_FAX_STATE_ST_TRANS 0x6
#define EVT_COD_FAX_STATE_ST_PP 0x7
#define EVT_COD_FAX_STATE_ST_INT 0x8
#define EVT_COD_FAX_STATE_ST_DCN 0x9


/** T.38 FAX Channel Event */
typedef struct EVT_COD_FAX_ERR
{
   EVT_HEAD_BE;
   /** Reserved */
   IFX_uint32_t Res01 : 16;
   /** Error or debug type (debug types start from 128) */
   IFX_uint32_t FOIP_ERR : 16;
} __PACKED__ EVT_COD_FAX_ERR_t;

/* modulator signal buffer underrun */
#define EVT_COD_FAX_ERR_MBSU   1
/* demodulator signal buffer overflow */
#define EVT_COD_FAX_ERR_DBSO   2
/* modulator data buffer overflow */
#define EVT_COD_FAX_ERR_MBDO   3
/* modulator data buffer underrun */
#define EVT_COD_FAX_ERR_MBDU   4
/* demodulator data buffer overflow */
#define EVT_COD_FAX_ERR_DBDO   5
/* wrong packet passed from IP */
#define EVT_COD_FAX_ERR_WPIP   6
/* command buffer overflow */
#define EVT_COD_FAX_ERR_CBO    7
/* data buffer overflow */
#define EVT_COD_FAX_ERR_DBO    8
/* wrong command in command buffer */
#define EVT_COD_FAX_ERR_WRC    9
/* T.38 activation failed */
#define EVT_COD_FAX_ERR_AF1   10
/* T.38 deactivation failed */
#define EVT_COD_FAX_ERR_AF2   11
/* T.38 trace flush finished */
#define EVT_COD_FAX_ERR_TFF  128


/** PCM HDLC Channel Event */
typedef struct EVT_PCM_HDLC
{
   EVT_HEAD_BE;
   /** TX buffer empty */
   IFX_uint32_t TE : 1;
   /** TX buffer overflow */
   IFX_uint32_t TO : 1;
   /** Reserved */
   IFX_uint32_t Res02 : 30;
} __PACKED__ EVT_PCM_HDLC_t;

#define EVT_PCM_HDLC_ECMD 4
#define EVT_PCM_HDLC_LEN 4


/** FXO Event */
typedef struct EVT_FXO
{
   EVT_HEAD_BE;
   /** Reserved */
   IFX_uint32_t Res02 : 28;
   /** Event */
   IFX_uint32_t EVENT : 4;
} __PACKED__ EVT_FXO_t;

#define EVT_FXO_RING_ON           0
#define EVT_FXO_RING_OFF          1
#define EVT_FXO_BATT_ON           2
#define EVT_FXO_BATT_OFF          3
#define EVT_FXO_OSI_END           4
#define EVT_FXO_APOH_ON           5
#define EVT_FXO_APOH_OFF          6
#define EVT_FXO_POLARITY_REVERSED 7
#define EVT_FXO_POLARITY_NORMAL   8


/** Announcement Event */
typedef struct EVT_COD_ANN_END
{
   EVT_HEAD_BE;
   /** Reserved */
   IFX_uint32_t Res02 : 24;
   /** ID of the announcement, which has terminated */
   IFX_uint32_t ANNID : 8;
} __PACKED__ EVT_COD_ANN_END_t;

#define EVT_COD_ANN_END_ECMD 9
#define EVT_COD_ANN_END_LEN 4


/** MOS Event */
typedef struct EVT_CPD_STAT_MOS
{
   EVT_HEAD_BE;
   /** Reserved */
   IFX_uint32_t Res02 : 16;
   /** MOS_CQE value that triggered event generation */
   IFX_uint32_t MOS_CQE : 16;
} __PACKED__ EVT_CPD_STAT_MOS_t;

#define EVT_CPD_STAT_MOS_ECMD 20
#define EVT_CPD_STAT_MOS_LEN 4


typedef union VMMC_EvtMsg
{
   IFX_uint32_t val[4];
   EVT_ALI_OVT_t evt_ali_ovt;
   EVT_ALI_RAW_HOOK_t evt_ali_raw_hook;
   EVT_ALI_LT_END_t evt_ali_lt_end;
   EVT_ALI_AR9DCC_t evt_ali_ar9dcc;
   EVT_COD_DEC_CHANGE_t evt_cod_dec_change;
   EVT_COD_VPOU_LIMIT_t evt_cod_vpou_limit;
   EVT_COD_VPOU_STAT_t evt_cod_vpou_stat;
   EVT_COD_LIN_REQ_t evt_cod_lin_req;
   EVT_COD_ANN_END_t evt_cod_ann_end;
   EVT_COD_FAX_REQ_t evt_cod_fax_req;
   EVT_CPD_STAT_MOS_t evt_cpd_stat_mos;
   EVT_SIG_RFCDET_t evt_sig_rfcdet;
   EVT_SIG_RFCSTAT_t evt_sig_rfcstat;
   EVT_SIG_UTG1_t evt_sig_utg1;
   EVT_SIG_UTG2_t evt_sig_utg2;
   EVT_SIG_DTMFG_t evt_sig_dtmfg;
   EVT_SIG_CIDS_t evt_sig_cids;
   EVT_SIG_DTMFD_t evt_sig_dtmfd;
   EVT_SIG_CPTD_t evt_sig_cptd;
   EVT_SIG_MFTD_t evt_sig_mftd;
   EVT_VR_STAT_t evt_vr_stat;
   EVT_COD_FAX_STATE_t evt_cod_fax_state;
   EVT_COD_FAX_ERR_t evt_cod_fax_err;
   EVT_PCM_HDLC_t evt_pcm_hdlc;
   EVT_FXO_t evt_fxo;
} VMMC_EvtMsg_t;

#if 0
table[][] = {
   0x9002000,"EVT_ALI_OVT",
   0x9002100,"EVT_ALI_RAW_HOOK",
   0x9002300,"EVT_ALI_LT_END",
   0X9002404,"EVT_ALI_AR9DCC",
   0x9006004,"EVT_COD_DEC_CHANGE",
   0x9006100,"EVT_COD_VPOU_LIMIT",
   0x9006200,"EVT_COD_VPOU_STAT",
   0x9006700,"EVT_COD_LIN_REQ",
   0x9006c04,"EVT_COD_FAX_REQ",
   0x9004600,"EVT_SIG_RFCDET",
   0x9004400,"EVT_SIG_RFCSTAT",
   0x9004700,"EVT_SIG_UTG1",
   0x9004800,"EVT_SIG_UTG2",
   0x9004200,"EVT_SIG_DTMFG",
   0x9004300,"EVT_SIG_CIDS",
   0x9004000,"EVT_SIG_DTMFD",
   0x9004100,"EVT_SIG_CPTD",
   0x9004500,"EVT_SIG_MFTD",
   0x900a000,"EVT_VR_STAT",
   0x9000400,"EVT_PCM_DCHAN"
};
#endif


#define VMMC_EVT_ID_MASK   0x1F00FF00
typedef enum
{
   VMMC_EVT_ID_ALI_OVT            = 0x09002000,
   VMMC_EVT_ID_ALI_RAW_HOOK       = 0x09002100,
   VMMC_EVT_ID_ALI_LT_END         = 0x09002300,
   VMMC_EVT_ID_ALI_AR9DCC         = 0x09002400,
   VMMC_EVT_ID_COD_DEC_CHANGE     = 0x09006000,
   VMMC_EVT_ID_COD_VPOU_LIMIT     = 0x09006100,
   VMMC_EVT_ID_COD_VPOU_STAT      = 0x09006200,
   VMMC_EVT_ID_COD_LIN_REQ        = 0x09006700,
   VMMC_EVT_ID_COD_LIN_UNDERFLOW  = 0x09006800,
   VMMC_EVT_ID_COD_ANN_END        = 0x09006900,
   VMMC_EVT_ID_COD_FAX_STATE      = 0x09006a00,
   VMMC_EVT_ID_COD_FAX_ERR_EVT    = 0x09006b00,
   VMMC_EVT_ID_COD_FAX_REQ        = 0x09006c00,
   VMMC_EVT_ID_CPD_STAT_MOS       = 0x09007400,
   VMMC_EVT_ID_DECT_UTG           = 0x09007800,
   VMMC_EVT_ID_SIG_RFC2833DET     = 0x09004600,
   VMMC_EVT_ID_SIG_RFC2833STAT    = 0x09004400,
   VMMC_EVT_ID_SIG_UTG1           = 0x09004700,
   VMMC_EVT_ID_SIG_UTG2           = 0x09004800,
   VMMC_EVT_ID_SIG_DTMFG          = 0x09004200,
   VMMC_EVT_ID_SIG_CIDS           = 0x09004300,
   VMMC_EVT_ID_SIG_DTMFD          = 0x09004000,
   VMMC_EVT_ID_SIG_CPTD           = 0x09004100,
   VMMC_EVT_ID_SIG_MFTD           = 0x09004500,
   VMMC_EVT_ID_SYS_INT_ERR        = 0x0900eb00,
   VMMC_EVT_ID_PCM_HDLC_RDY       = 0x09000400,
   VMMC_EVT_ID_FXO                = 0x09002500
} VMMC_EVT_ID_t;

#endif /* _DRV_VMMC_INT_EVT_H_ */
