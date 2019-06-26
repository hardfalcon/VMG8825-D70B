/******************************************************************************

  Copyright (c) 2006-2013 Lantiq Deutschland GmbH
  Copyright (c) 2015 Lantiq Beteiligungs-GmbH & Co.KG
  Copyright 2016, Intel Corporation.

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/**
   \file drv_vmmc_cod.c Implementation of the CODer module.
*/

/* ============================= */
/* Includes                      */
/* ============================= */
#include "drv_api.h"
#include "drv_vmmc_cod_priv.h"
#include "drv_vmmc_api.h"
#include "drv_vmmc_sig.h"
#include "drv_vmmc_sig_priv.h"
#include "drv_vmmc_con.h"
#include "drv_vmmc_cod.h"
#include "drv_vmmc_errno.h"
#include "drv_vmmc_announcements.h"

/* ============================= */
/* Local Macros & Definitions    */
/* ============================= */

/* Translation table for coder values   TAPI enum -> FW value */
static IFX_uint8_t TranslateCoderTable[] =
{
   COD_CHAN_SPEECH_ENC_NO,             /*  0: No coder */
   COD_CHAN_SPEECH_ENC_G7231_6_3,      /*  1: G723, 6.3 kBit/s */
   COD_CHAN_SPEECH_ENC_G7231_5_3,      /*  2: G723, 5.3 kBit/s */
   COD_CHAN_SPEECH_ENC_NO,             /*  3: No coder */
   COD_CHAN_SPEECH_ENC_NO,             /*  4: No coder */
   COD_CHAN_SPEECH_ENC_NO,             /*  5: No coder */
   COD_CHAN_SPEECH_ENC_G728_16,        /*  6: G728, 16 kBit/s */
   COD_CHAN_SPEECH_ENC_G729AB_8,       /*  7: G729 A and B, 8 kBit/s */
   COD_CHAN_SPEECH_ENC_G711_MLAW,      /*  8: G711 u-Law, 64 kBit/s */
   COD_CHAN_SPEECH_ENC_G711_ALAW,      /*  9: G711 A-Law, 64 kBit/s */
   COD_CHAN_SPEECH_ENC_G711_MLAW_VBD,  /* 10: G711 u-law VBD, 64 kBit/s */
   COD_CHAN_SPEECH_ENC_G711_ALAW_VBD,  /* 11: G711 A-law VBD, 64 kBit/s */
   COD_CHAN_SPEECH_ENC_G726_16,        /* 12: G726, 16 kBit/s */
   COD_CHAN_SPEECH_ENC_G726_24,        /* 13: G726, 24 kBit/s */
   COD_CHAN_SPEECH_ENC_G726_32,        /* 14: G726, 32 kBit/s */
   COD_CHAN_SPEECH_ENC_G726_40,        /* 15: G726, 40 kBit/s */
   COD_CHAN_SPEECH_ENC_G729E_11_8,     /* 16: G729 E, 11.8 kBit/s */
   COD_CHAN_SPEECH_ENC_ILBC_13_3,      /* 17: iLBC, 13.3 kBit/s */
   COD_CHAN_SPEECH_ENC_ILBC_15_2,      /* 18: iLBC, 15.2 kBit/s */
   COD_CHAN_SPEECH_ENC_LIN16_8KHZ,     /* 19: Linear Codec, 16 bit, 8 kHz */
   COD_CHAN_SPEECH_ENC_LIN16_16KHZ,    /* 20: Linear Codec, 16 bit, 16 kHz */
   COD_CHAN_SPEECH_ENC_AMR_4_75,       /* 21: AMR, 4.75 kBit/s */
   COD_CHAN_SPEECH_ENC_AMR_5_15,       /* 22: AMR, 5.15 kBit/s */
   COD_CHAN_SPEECH_ENC_AMR_5_9,        /* 23: AMR, 5.9 kBit/s */
   COD_CHAN_SPEECH_ENC_AMR_6_7,        /* 24: AMR, 6.7 kBit/s */
   COD_CHAN_SPEECH_ENC_AMR_7_4,        /* 25: AMR, 7.4 kBit/s */
   COD_CHAN_SPEECH_ENC_AMR_7_95,       /* 26: AMR, 7.95 kBit/s */
   COD_CHAN_SPEECH_ENC_AMR_10_2,       /* 27: AMR, 10.2 kBit/s */
   COD_CHAN_SPEECH_ENC_AMR_12_2,       /* 28: AMR, 12.2 kBit/s */
   COD_CHAN_SPEECH_ENC_NO,             /* 29: No coder */
   COD_CHAN_SPEECH_ENC_NO,             /* 30: No coder */
   COD_CHAN_SPEECH_ENC_G722_64,        /* 31: G.722 (wideband), 64 kBit/s */
   COD_CHAN_SPEECH_ENC_G7221_24,       /* 32: G.722.1 (wideband), 24 kBit/s */
   COD_CHAN_SPEECH_ENC_G7221_32,       /* 33: G.722.1 (wideband), 32 kBit/s */
   COD_CHAN_SPEECH_ENC_NO,             /* 34: AMR-NB many bitrates */
   COD_CHAN_SPEECH_ENC_NO              /* 35: AMR-WB (G.722.2) many bitrates */
};

/* Translation table for frame length values   TAPI enum -> FW value */
static IFX_uint8_t TranslateFrameLengthTable[] =
{
   COD_CHAN_SPEECH_PTE_NO,             /*  0: Not supported. */
   COD_CHAN_SPEECH_PTE_NO,             /*  1: 2.5 ms packetization length. */
   COD_CHAN_SPEECH_PTE_NO,             /*  2: 5 ms packetization length.   */
   COD_CHAN_SPEECH_PTE_NO,             /*  3: 5.5 ms packetization length. */
   COD_CHAN_SPEECH_PTE_10,             /*  4: 10 ms packetization length. */
   COD_CHAN_SPEECH_PTE_NO,             /*  5: 11 ms packetization length. */
   COD_CHAN_SPEECH_PTE_20,             /*  6: 20 ms packetization length. */
   COD_CHAN_SPEECH_PTE_30,             /*  7: 30 ms packetization length. */
   COD_CHAN_SPEECH_PTE_40,             /*  8: 40 ms packetization length. */
   COD_CHAN_SPEECH_PTE_NO,             /*  9: 50 ms packetization length. */
   COD_CHAN_SPEECH_PTE_60              /* 10: 60 ms packetization length. */
};

/* Translation table from AMR bit rate to codec type TAPI enum -> TAPI enum */
static IFX_TAPI_COD_TYPE_t TranslateAmrNB_BitRateTable[] =
{
   IFX_TAPI_COD_TYPE_AMR_4_75,         /*  0: AMR, 4.75 kbit/s. */
   IFX_TAPI_COD_TYPE_AMR_5_15,         /*  1: AMR, 5.15 kbit/s. */
   IFX_TAPI_COD_TYPE_AMR_5_9,          /*  2: AMR, 5.9 kbit/s. */
   IFX_TAPI_COD_TYPE_AMR_6_7,          /*  3: AMR, 6.7 kbit/s */
   IFX_TAPI_COD_TYPE_AMR_7_4,          /*  4: AMR, 7.4 kbit/s. */
   IFX_TAPI_COD_TYPE_AMR_7_95,         /*  5: AMR, 7.95 kbit/s. */
   IFX_TAPI_COD_TYPE_AMR_10_2,         /*  6: AMR, 10.2 kbit/s. */
   IFX_TAPI_COD_TYPE_AMR_12_2          /*  7: AMR, 12.2 kbit/s. */
};

/* Jitter Buffer default configuration */
#define JB_CONFIG_TYPE            IFX_TAPI_JB_TYPE_ADAPTIVE
#define JB_CONFIG_PKT_ADAPT       IFX_TAPI_JB_PKT_ADAPT_VOICE
#define JB_CONFIG_SCALING         0x16    /* Scaling factor */
#define JB_CONFIG_INITIAL_SIZE    0x0050  /* Inital JB size 10 ms */
#define JB_CONFIG_MINIMUM_SIZE    0x0050  /* Min.   JB size 10 ms */
#define JB_CONFIG_MAXIMUM_SIZE    0x0C80  /* Max.   JB size 400 ms */
                                          /* sizes in multiples of 125 us */
/* Enhanced Jitter Buffer configuration default values */
#define COD_JB_CONF_ADAP_FACTOR   0x4D
#define COD_JB_CONF_MIN_MARGIN    0x28


/* ---------------------------------------------------------------------------
                        AGC related variables - BEGIN
 */

/** Parameter limits for AGC (Automated Gain Control). */

/** "Compare Level", this is the target level in 'dB', MAX is 0dB. */
#define AGC_CONFIG_COM_MAX  0

/** "Compare Level", this is the target level in 'dB', MIN is -50dB. */
#define AGC_CONFIG_COM_MIN -50

/** Used to get right table index for dB -> HEX conversion. */
#define AGC_CONFIG_COM_OFFSET 50

/** "Maximum Gain", maximum gain that we'll be applied to the signal in
    'dB', MAX is 48dB. */
#define AGC_CONFIG_GAIN_MAX 48

/** "Maximum Gain", maximum gain that we'll be applied to the signal in
    'dB', MIN is 0dB. */
#define AGC_CONFIG_GAIN_MIN 0

/** Used to get right table index for dB -> HEX conversion. */
#define AGC_CONFIG_GAIN_OFFSET 0

/** "Maximum Attenuation for AGC", maximum attenuation that we'll be applied
    to the signal in 'dB', MAX is 0dB. */
#define AGC_CONFIG_ATT_MAX 0

/** "Maximum Attenuation for AGC", maximum attenuation that we'll be applied
    to the signal in 'dB', MIN is -42dB. */
#define AGC_CONFIG_ATT_MIN -42

/** Used to get right table index for dB -> HEX conversion. */
#define AGC_CONFIG_ATT_OFFSET 42

/** "Minimum Input Level", signals below this threshold won't be processed
    by AGC in 'dB', MAX is -25 dB. */
#define AGC_CONFIG_LIM_MAX -25

/** "Minimum Input Level", signals below this threshold won't be processed
    by AGC in 'dB', MIN is -60 dB. */
#define AGC_CONFIG_LIM_MIN -60

/** Used to get right table index for dB -> HEX conversion. */
#define AGC_CONFIG_LIM_OFFSET 60

#ifdef VMMC_FEAT_FAX_T38_FW
/* default value 3.14 dB for gain1 */
#define VMMC_FAX_GAIN1_DEFAULT 0x2DEF

/* default value -3.14 dB for gain2 */
#define VMMC_FAX_GAIN2_DEFAULT 0x164A

/* Max bitrate range definition */
#define VMMC_MAXBITRATE_MIN    2400
#define VMMC_MAXBITRATE_MAX    14400

/* Modulation buffer size in units of 0.625 ms;
   min:1 (0.625 ms), max:320 (200 ms) */
#define VMMC_MOBSZ_MIN         1
#define VMMC_MOBSZ_MAX         320

/* Required modulation buffer fill level before modulation start;
   in units of 0.625 ms; min:1 (0.625 ms), max:320 (200 ms) */
#define VMMC_MOBSM_MIN         1
#define VMMC_MOBSM_MAX         320

/* Required modulation buffer fill level before modulator's request
   for more data; in units of 0.625 ms; min:1 (0.625 ms), max:320 (200 ms) */
#define VMMC_MOBRD_MIN         1
#define VMMC_MOBRD_MAX         320

/* Required demodulation buffer level before demodulator sends upstream data;
   in units of 0.625 ms; min:1 (0.625 ms), max:144 (90 ms) */
#define VMMC_DMBSD_MIN         1
#define VMMC_DMBSD_MAX         144
#endif /* VMMC_FEAT_FAX_T38_FW */

/* Maximum redundancy level for AMR codec */
#define VMMC_AMR_FEC_LEVEL_MAX 2
#define VMMC_AMR_CMR_MAX       7

/** Conversion table from dB to HEX values. */
static IFX_uint8_t AgcConfig_Reg_COM [AGC_CONFIG_COM_MAX -
                                     (AGC_CONFIG_COM_MIN - 1)] =
{
   /* conversion table from 'dB# to HEX register values */
   /* -50   -49   -48   -47   -46   -45   -44   -43   -42   -41   -40   */
      132,  133,  133,  134,  134,  135,  136,  137,  138,  140,  141,
   /* -39   -38   -37   -36   -35   -34   -33   -32   -31   -30   -29   */
      142,  144,  146,  148,  151,  154,  157,  160,  164,  169,  174,
   /* -28   -27   -26   -25   -24   -23   -22   -21   -20   -19   -18   */
      179,  186,  193,  7,    8,    9,    10,   11,   13,   14,   16,
   /* -17   -16   -15   -14   -13   -12   -11   -10   -9    -8    -7    */
      18,   20,   23,   26,   29,   32,   36,   40,   45,   51,   57,
   /* -6    -5    -4    -3    -2    -1    0  */
      64,   72,   81,   91,   102,  114,  128
};

/** Conversion table from dB to HEX values. */
static IFX_uint8_t AgcConfig_Reg_GAIN [AGC_CONFIG_GAIN_MAX -
                                      (AGC_CONFIG_GAIN_MIN - 1)] =
{
   /* 0     1     2     3     4     5     6     7     8     9     10    */
      136,  137,  138,  139,  141,  142,  144,  146,  148,  151,  153,
   /* 11    12    13    14    15    16    17    18    19    20    21    */
      156,  160,  164,  168,  173,  178,  185,  4,    199,  208,  218,
   /* 22    23    24    25    26    27    28    29    30    31    32    */
      229,  241,  32,   36,   40,   45,   50,   56,   63,   71,   80,
   /* 33    34    35    36    37    38    39    40    41    42    43    */
      89,   100,  112,  126,  142,  159,  178,  200,  224,  252,  255,
   /* 44    45    46    47    48    */
      255,  255,  255,  255,  255
};

/** Conversion table from dB to HEX values. */
static IFX_uint8_t AgcConfig_Reg_ATT[AGC_CONFIG_ATT_MAX -
                                     (AGC_CONFIG_ATT_MIN - 1)] =
{
   /* -42   -41   -40   -39   -38   -37   -36   -35   -34   -33   -32   */
      1,    1,    1,    1,    2,    2,    2,    2,    3,    3,    3,
   /* -31   -30   -29   -28   -27   -26   -25   -24   -23   -22   -21   */
      4,    4,    5,    5,    6,    6,    7,    8,    9,    10,   11,
   /* -20   -19   -18   -17   -16   -15   -14   -13   -12   -11   -10   */
      13,   14,   16,   18,   20,   23,   26,   29,   32,   36,   40,
   /* -9    -8    -7    -6    -5    -4    -3    -2    -1    0  */
      45,   51,   57,   64,   72,   81,   91,   102,  114,  127
};

/** Conversion table from dB to HEX values. */
static IFX_uint8_t AgcConfig_Reg_LIM [AGC_CONFIG_LIM_MAX -
                                      (AGC_CONFIG_LIM_MIN - 1)] =
{
   /* -60   -59   -58   -57   -56   -55   -54   -53   -52   -51   -50   */
      161,  165,  169,  174,  180,  186,  193,  201,  210,  220,  232,
   /* -49   -48   -47   -46   -45   -44   -43   -42   -41   -40   -39   */
      7,    8,    9,    10,   12,   13,   14,   16,   18,   20,   23,
   /* -38   -37   -36   -35   -34   -33   -32   -31   -30   -29   -28   */
      26,   29,   32,   36,   41,   46,   51,   58,   65,   73,   81,
   /* -27   -26   -25   */
      91,   103,  115
};

/* ---------------------------------------------------------------------------
                        AGC related variables - END
 */

/* ============================= */
/* Local function declaration    */
/* ============================= */
static IFX_uint8_t VMMC_COD_trans_bitrate_tapi2fw (
                        IFX_TAPI_COD_TYPE_t nCoder,
                        IFX_TAPI_COD_BITRATE_t nBitRate);

static IFX_int32_t vmmc_cod_ENCnPTE_Check (
                        IFX_uint8_t nFrameLength,
                        IFX_uint8_t nCodec,
                        IFX_uint8_t bAMRE);

static IFX_uint8_t vmmc_cod_trans_fl_tapi2fw (IFX_TAPI_COD_LENGTH_t nFL);

static IFX_boolean_t vmmc_codec_support_check (
                        VMMC_DEVICE *pDev, IFX_uint8_t nCodec);

static IFX_int32_t vmmc_cod_Voice_Enable (
                        VMMC_CHANNEL *pCh,
                        IFX_uint8_t nMode);

static IFX_int32_t vmmc_cod_RTP_Cfg (
                        VMMC_CHANNEL *pCh,
                        IFX_TAPI_PKT_RTP_CFG_t const *pRtpConf);

static IFX_int32_t vmmc_cod_RTP_PayloadTableSet (
                        VMMC_CHANNEL *pCh,
                        IFX_boolean_t bUp,
                        IFX_uint8_t *pPTvalues);

static IFX_void_t  vmmc_cod_WidebandCodecCheck (VMMC_CHANNEL *pCh);

static IFX_int32_t VMMC_TAPI_LL_COD_ENC_Cfg_Set (
                        IFX_TAPI_LL_CH_t *pLLChannel,
                        IFX_TAPI_ENC_CFG_SET_t const *pEncCfg);

static IFX_int32_t VMMC_TAPI_LL_COD_VAD_Cfg (
                        IFX_TAPI_LL_CH_t *pLLChannel,
                        IFX_int32_t nVAD);

static IFX_int32_t VMMC_TAPI_LL_COD_AGC_Cfg (
                        IFX_TAPI_LL_CH_t *pLLChannel,
                        IFX_TAPI_ENC_AGC_CFG_t const *pParam);

static IFX_int32_t vmmc_cod_AGC_Enable (
                        VMMC_CHANNEL *pCh,
                        IFX_TAPI_ENC_AGC_MODE_t agcMode);

static IFX_int32_t VMMC_TAPI_LL_COD_AGC_Enable (
                        IFX_TAPI_LL_CH_t *pLLChannel,
                        IFX_TAPI_ENC_AGC_MODE_t Param);

static IFX_int32_t VMMC_TAPI_LL_COD_DEC_Start (
                        IFX_TAPI_LL_CH_t *pLLChannel);

static IFX_int32_t VMMC_TAPI_LL_COD_DEC_Stop (
                        IFX_TAPI_LL_CH_t *pLLChannel);

static IFX_int32_t VMMC_TAPI_LL_COD_ENC_Start (
                        IFX_TAPI_LL_CH_t *pLLChannel);

static IFX_int32_t VMMC_TAPI_LL_COD_ENC_Stop (
                        IFX_TAPI_LL_CH_t *pLLChannel);

static IFX_int32_t VMMC_TAPI_LL_COD_ENC_Hold (
                        IFX_TAPI_LL_CH_t *pLLChannel,
                        IFX_operation_t nOnHold);

static IFX_int32_t VMMC_TAPI_LL_COD_RTP_Cfg (
                        IFX_TAPI_LL_CH_t *pLLChannel,
                        IFX_TAPI_PKT_RTP_CFG_t const *pRtpConf);

static IFX_int32_t VMMC_TAPI_LL_COD_RTP_PayloadTable_Cfg (
                        IFX_TAPI_LL_CH_t *pLLChannel,
                        IFX_TAPI_PKT_RTP_PT_CFG_t const *pRtpPTConf);

static IFX_int32_t VMMC_TAPI_LL_COD_RTP_EventGenerate (
                        IFX_TAPI_LL_CH_t *pLLChannel,
                        IFX_uint8_t nEvent,
                        IFX_boolean_t bStart,
                        IFX_uint8_t nDuration,
                        IFX_int8_t nVolume);

static IFX_int32_t VMMC_TAPI_LL_COD_RTCP_Reset (
                        IFX_TAPI_LL_CH_t *pLLChannel);

static IFX_int32_t VMMC_TAPI_LL_COD_RTCP_Get (
                        IFX_TAPI_LL_CH_t *pLLChannel,
                        IFX_TAPI_PKT_RTCP_STATISTICS_t *pRTCP);

static IFX_int32_t VMMC_TAPI_LL_COD_JB_Cfg (
                        IFX_TAPI_LL_CH_t *pLLChannel,
                        IFX_TAPI_JB_CFG_t const *pJbConf);

static IFX_int32_t VMMC_TAPI_LL_COD_JB_Stat_Get (
                        IFX_TAPI_LL_CH_t *pLLChannel,
                        IFX_TAPI_JB_STATISTICS_t *pJbData);

static IFX_int32_t VMMC_TAPI_LL_COD_JB_Stat_Reset (
                        IFX_TAPI_LL_CH_t *pLLChannel);

static IFX_int32_t VMMC_TAPI_LL_COD_Volume_Set (
                        IFX_TAPI_LL_CH_t *pLLChannel,
                        IFX_TAPI_PKT_VOLUME_t const *pVol);

static IFX_int32_t VMMC_TAPI_LL_COD_DEC_Cfg_Set (
                        IFX_TAPI_LL_CH_t *pLLChannel,
                        IFX_TAPI_DEC_CFG_t const *pTapiDecCfg);

static IFX_int32_t VMMC_TAPI_LL_COD_DEC_HP_Set (
                        IFX_TAPI_LL_CH_t *pLLChannel,
                        IFX_boolean_t bHp);

static IFX_int32_t VMMC_TAPI_LL_COD_DEC_ChgDetailReq (
                        IFX_TAPI_LL_CH_t *pLLCh,
                        IFX_TAPI_DEC_DETAILS_t *pDec);

static IFX_int32_t VMMC_TAPI_LL_COD_MOS_Cfg (
                        IFX_TAPI_LL_CH_t *pLLCh,
                        IFX_TAPI_COD_MOS_t const *pMosCfg);

static IFX_int32_t VMMC_TAPI_LL_COD_MOS_Result_Get (
                        IFX_TAPI_LL_CH_t *pLLCh,
                        IFX_TAPI_COD_MOS_t *pMos);

#ifdef VMMC_FEAT_FAX_T38
/* VMMC_FAX_T38 related functions */

static IFX_int32_t vmmc_cod_DP_DemodSet (
                        VMMC_CHANNEL *pCh, IFX_uint8_t nSt1,
                        IFX_uint8_t nSt2, IFX_uint8_t nEq,
                        IFX_uint8_t nTr);

static IFX_int32_t vmmc_cod_DP_ModSet (
                        VMMC_CHANNEL *pCh, IFX_uint8_t nSt,
                        IFX_uint16_t nLen, IFX_uint8_t nDbm,
                        IFX_uint8_t nTEP, IFX_uint8_t nTr);

static IFX_int32_t vmmc_cod_DP_Set (
                        VMMC_CHANNEL *pCh, IFX_boolean_t bEn,
                        IFX_boolean_t bMod, IFX_int16_t gain,
                        IFX_uint16_t mod_start,
                        IFX_uint16_t mod_req,
                        IFX_uint16_t demod_send);

/* Configures the Datapump for Modulation */
static IFX_int32_t VMMC_TAPI_LL_COD_T38_Mod_Enable (
                        IFX_TAPI_LL_CH_t *pLLChannel,
                        IFX_TAPI_T38_MOD_DATA_t const *pFaxMod);

/* Configures the Datapump for Demodulation */
static IFX_int32_t VMMC_TAPI_LL_COD_T38_DeMod_Enable (
                        IFX_TAPI_LL_CH_t *pLLChannel,
                        IFX_TAPI_T38_DEMOD_DATA_t const *pFaxDemod);

/*  disables the Fax datapump */
static IFX_int32_t VMMC_TAPI_LL_COD_T38_Datapump_Disable (
                        IFX_TAPI_LL_CH_t *pLLChannel);

/* query Fax Status */
static IFX_int32_t VMMC_TAPI_LL_COD_T38_Status_Get (
                        IFX_TAPI_LL_CH_t *pLLChannel,
                        IFX_TAPI_T38_STATUS_t *pFaxStatus);

/* Set Fax Status */
static IFX_int32_t VMMC_TAPI_LL_COD_T38_Status_Set (
                        IFX_TAPI_LL_CH_t *pLLChannel,
                        unsigned char status);

/* Set Fax Error status */
static IFX_int32_t VMMC_TAPI_LL_COD_T38_Error_Set (
                        IFX_TAPI_LL_CH_t *pLLChannel,
                        unsigned char error);
#endif /* VMMC_FEAT_FAX_T38 */

#ifdef VMMC_FEAT_FAX_T38_FW
IFX_int32_t VMMC_TAPI_LL_COD_FAX_Cap_Get (
                        IFX_TAPI_LL_DEV_t *pLLDev,
                        IFX_TAPI_T38_CAP_t *pCap);

IFX_int32_t VMMC_TAPI_LL_COD_FAX_Cfg_Get (
                        IFX_TAPI_LL_CH_t *pLLCh,
                        IFX_TAPI_T38_FAX_CFG_t *pCfg);

IFX_int32_t VMMC_TAPI_LL_COD_FAX_Cfg_Set (
                        IFX_TAPI_LL_CH_t *pLLCh,
                        IFX_TAPI_T38_FAX_CFG_t const *pCfg);

IFX_int32_t VMMC_TAPI_LL_COD_FAX_FDP_Cfg_Get (
                        IFX_TAPI_LL_CH_t *pLLCh,
                        IFX_TAPI_T38_FDP_CFG_t *pFDPCfg);

IFX_int32_t VMMC_TAPI_LL_COD_FAX_FDP_Cfg_Set (
                        IFX_TAPI_LL_CH_t *pLLCh,
                        IFX_TAPI_T38_FDP_CFG_t const *pFDPCfg);

IFX_int32_t VMMC_TAPI_LL_COD_FAX_Start (
                        IFX_TAPI_LL_CH_t *pLLCh,
                        IFX_TAPI_T38_SESS_CFG_t const *pT38Cfg);

IFX_int32_t VMMC_TAPI_LL_COD_FAX_Stat_Get (
                        IFX_TAPI_LL_CH_t *pLLCh,
                        IFX_TAPI_T38_SESS_STATISTICS_t *pStat);

IFX_int32_t VMMC_TAPI_LL_COD_FAX_Stop (
                        IFX_TAPI_LL_CH_t *pLLCh);

IFX_int32_t VMMC_TAPI_LL_COD_FAX_Trace (
                        IFX_TAPI_LL_CH_t *pLLCh,
                        IFX_TAPI_T38_TRACE_CFG_t const *pTrace);

static IFX_int16_t VMMC_FAX_HEX2dB(IFX_uint16_t Hex);
#endif /* VMMC_FEAT_FAX_T38_FW */

/* ============================= */
/* Function definitions          */
/* ============================= */

/**
   decode the frame length coefficient to the IFX_TAPI value
   \param nPTE frame length (packetisation time) coefficients
   \return IFX_TAPI value for this packetisation time
*/
static IFX_TAPI_COD_LENGTH_t vmmc_cod_trans_fl_fw2tapi(IFX_int16_t nPTE)
{
   switch (nPTE)
   {
   case 10:
      return (IFX_TAPI_COD_LENGTH_5);
   case 11:
      return (IFX_TAPI_COD_LENGTH_5_5);
   case 20:
      return (IFX_TAPI_COD_LENGTH_10);
   case 40:
      return (IFX_TAPI_COD_LENGTH_20);
   case 60:
      return (IFX_TAPI_COD_LENGTH_30);
   case 22:
      return (IFX_TAPI_COD_LENGTH_11);
   case 80:
      return (IFX_TAPI_COD_LENGTH_40);
   case 120:
      return (IFX_TAPI_COD_LENGTH_60);
   default:
      return IFX_TAPI_COD_LENGTH_ZERO;
   }
}

/**
   Function to translate coder value from TAPI enum to FW encoding.

   \param  nCoder       TAPI coder value.
   \param  nBitRate     TAPI bitrate value.
   \param  bAMRE        if 0 bitrate is coded in codec type for AMR-NB codec,
                        otherwise bitrate is coded with separate bitfield.

   \return
   ENC value or COD_CHAN_SPEECH_ENC_NO.
*/
IFX_uint8_t VMMC_COD_trans_cod_tapi2fw (IFX_TAPI_COD_TYPE_t nCoder,
                                        IFX_TAPI_COD_BITRATE_t nBitRate,
                                        IFX_uint8_t bAMRE)
{
   if (bAMRE)
   {
      switch (nCoder)
      {
         /* All AMR-NB codecs with different bitrates
            have now one FW ENC number */
         case IFX_TAPI_COD_TYPE_AMR_4_75:
         case IFX_TAPI_COD_TYPE_AMR_5_15:
         case IFX_TAPI_COD_TYPE_AMR_5_9:
         case IFX_TAPI_COD_TYPE_AMR_6_7:
         case IFX_TAPI_COD_TYPE_AMR_7_4:
         case IFX_TAPI_COD_TYPE_AMR_7_95:
         case IFX_TAPI_COD_TYPE_AMR_10_2:
         case IFX_TAPI_COD_TYPE_AMR_12_2:
         case IFX_TAPI_COD_TYPE_AMR_NB:
            return COD_CHAN_SPEECH_ENC_AMR_NB;
         /* All AMR-NB codecs with different bitartes
            have now one FW ENC number */
         case IFX_TAPI_COD_TYPE_AMR_WB:
            return COD_CHAN_SPEECH_ENC_AMR_WB;
         default:
            break;
      }
   }
   /* if bitrate not transmitted with separate bitfield BITR in FW message,
      then bitrate for AMR-NB must be decoded to ENC number */
   else if (IFX_TAPI_COD_TYPE_AMR_NB == nCoder)
   {
      /* range check neccessary because this function is called from extern */
      if( nBitRate >=
          sizeof(TranslateAmrNB_BitRateTable)/sizeof(IFX_TAPI_COD_TYPE_t) )
      {
         TRACE(VMMC, DBG_LEVEL_HIGH,
               ("\nDRV_ERROR: Parameter bitrate value %d out of range [0 to %d]\n",
                nBitRate, sizeof(TranslateAmrNB_BitRateTable)));
         return COD_CHAN_SPEECH_ENC_NO;
      }
      nCoder = TranslateAmrNB_BitRateTable[nBitRate];
   }
   /* range check neccessary because this function is called from extern */
   /* sizeof is sufficient because data type in array is uint8 */
   if( nCoder >= sizeof(TranslateCoderTable) )
   {
      TRACE(VMMC, DBG_LEVEL_HIGH,
            ("\nDRV_ERROR: Parameter coder value %d out of range [0 to %d]\n",
             nCoder, sizeof(TranslateCoderTable)));
      return COD_CHAN_SPEECH_ENC_NO;
   }

   return TranslateCoderTable[nCoder];
}


/**
   Function to translate coder value from FW encoding to TAPI enum.

   \param  nCoder       FW coder value.
   \param  bAMRE        if 0 bitrate is coded in codec type for AMR-NB codec,
                        otherwise bitrate is coded with separate bitfield.

   \return
   IFX_TAPI value for this coder type or IFX_TAPI_COD_TYPE_UNKNOWN.
*/
IFX_TAPI_COD_TYPE_t VMMC_COD_trans_cod_fw2tapi (IFX_int8_t nCoder,
                                                IFX_uint8_t bAMRE)
{
   IFX_TAPI_COD_TYPE_t  i;

   if (bAMRE)
   {
      /* When AMRE feature available some COD_CHAN_SPEECH_ENC values
         are different and not all are supported */
      switch (nCoder)
      {
         case COD_CHAN_SPEECH_ENC_AMR_NB:
            return IFX_TAPI_COD_TYPE_AMR_NB;
         case COD_CHAN_SPEECH_ENC_AMR_WB:
            return IFX_TAPI_COD_TYPE_AMR_WB;
         /* case COD_CHAN_SPEECH_ENC_AMR_4_75:
            same value as COD_CHAN_SPEECH_ENC_AMR_NB */
         case COD_CHAN_SPEECH_ENC_AMR_5_15:
         /* case COD_CHAN_SPEECH_ENC_AMR_6_7:
            same value as COD_CHAN_SPEECH_ENC_AMR_WB */
         case COD_CHAN_SPEECH_ENC_AMR_7_4:
         case COD_CHAN_SPEECH_ENC_AMR_7_95:
         case COD_CHAN_SPEECH_ENC_AMR_10_2:
         case COD_CHAN_SPEECH_ENC_AMR_12_2:
            /* not all values are supported */
            return IFX_TAPI_COD_TYPE_UNKNOWN;
         default:
            break;
      }
   }
   /* Lookup FW value in the translation table. Index in this table is the
      enum used by tapi. Index 0 is coder type unknown and is left out of
      the check. If FW value cannot be found leave and exit below. */
   for (i=IFX_TAPI_COD_TYPE_UNKNOWN; i < sizeof(TranslateCoderTable); i++)
   {
      if (TranslateCoderTable[i] == nCoder)
      {
         return i;
      }
   }

   return IFX_TAPI_COD_TYPE_UNKNOWN;
}

/**
   Decode FW BITR value from TAPI enums.

   \param  nCoder       TAPI coder value.
   \param  nBitRate     TAPI bitrate value.

   \return BITR (bitrate) value for cmd COD_CHAN_SPEECH.
*/
IFX_uint8_t VMMC_COD_trans_bitrate_tapi2fw (
                        IFX_TAPI_COD_TYPE_t nCoder,
                        IFX_TAPI_COD_BITRATE_t nBitRate)
{
   IFX_uint8_t i;

   for (i=0;
        i<sizeof(TranslateAmrNB_BitRateTable)/sizeof(IFX_TAPI_COD_TYPE_t); i++)
   {
      if (nCoder == TranslateAmrNB_BitRateTable[i])
      {
         return i;
      }
   }
   switch (nCoder)
   {
      case IFX_TAPI_COD_TYPE_AMR_NB:
         if (COD_CHAN_SPEECH_BITR_AMR_NB_MAX < nBitRate )
         {
            return COD_CHAN_SPEECH_BITR_INVALID;
         }
         return nBitRate;
      case IFX_TAPI_COD_TYPE_AMR_WB:
         if (COD_CHAN_SPEECH_BITR_AMR_WB_MAX < nBitRate )
         {
            return COD_CHAN_SPEECH_BITR_INVALID;
         }
         return nBitRate;
      default:
         return 0;
   }
}

/**
   Function to perform check whether the requested codec is supported by
   the firmware version in use.

   \param pDev       Pointer to VMMC device data.
   \param nCodec     Encoder identifier.

   \return
   IFX_TRUE for supported encoder, else IFX_FALSE
*/
static IFX_boolean_t vmmc_codec_support_check (VMMC_DEVICE *pDev,
                                               IFX_uint8_t nCodec)
{
   IFX_boolean_t bRet = IFX_FALSE;

   /*lint -save -e725 */
   switch (nCodec)
   {
      /* inactive encoder */
      case COD_CHAN_SPEECH_ENC_NO:
         bRet = IFX_TRUE;
         break;
      /* L16 16 bit linear, 8kHz sampling rate */
      case COD_CHAN_SPEECH_ENC_LIN16_8KHZ:
         if ((pDev->caps.CODECS & CODEC_L16))
         bRet = IFX_TRUE;
         break;
      /* L16 16 bit linear, 16kHz sampling rate */
      case COD_CHAN_SPEECH_ENC_LIN16_16KHZ:
         if ((pDev->caps.CODECS & CODEC_L16_16))
         bRet = IFX_TRUE;
         break;
      /* G.711, alaw and mlaw */
      case COD_CHAN_SPEECH_ENC_G711_ALAW:
      case COD_CHAN_SPEECH_ENC_G711_ALAW_VBD:
      case COD_CHAN_SPEECH_ENC_G711_MLAW:
      case COD_CHAN_SPEECH_ENC_G711_MLAW_VBD:
         if ((pDev->caps.CODECS & CODEC_G711))
         bRet = IFX_TRUE;
         break;
      /* G.726, 16, 24, 32, and 40 kbit/s */
      case COD_CHAN_SPEECH_ENC_G726_16:
      case COD_CHAN_SPEECH_ENC_G726_24:
      case COD_CHAN_SPEECH_ENC_G726_32:
      case COD_CHAN_SPEECH_ENC_G726_40:
         if ((pDev->caps.CODECS & CODEC_G726))
         bRet = IFX_TRUE;
         break;
      /* AMR with all data rates.
         NB: 4.75, 5.15, 5.9, 6.7, 7.4, 7.95, 10.2, 12.2 bit/s
         WB: 6.60, 8.85, 12.65, 14.25, 15.85, 18.25, 19.85, 23.05, 23.85
         Note that for the old API the bitrates were separate encoder values
         while with new interface that is signalled with the AMRE bit only
         AMR-NB and AMR-WB are left. The others are reserved. */
      case COD_CHAN_SPEECH_ENC_AMR_NB:
           /* alias: COD_CHAN_SPEECH_ENC_AMR_4_75 */
         if (pDev->caps.CODECS & CODEC_AMR_NB)
            bRet = IFX_TRUE;
         break;
      case COD_CHAN_SPEECH_ENC_AMR_WB:
           /* alias: COD_CHAN_SPEECH_ENC_AMR_5_9 */
         if ((pDev->caps.bAMRE) && (pDev->caps.CODECS & CODEC_AMR_WB))
            bRet = IFX_TRUE;
         if ((!pDev->caps.bAMRE) && (pDev->caps.CODECS & CODEC_AMR_NB))
            bRet = IFX_TRUE;
         break;
      case COD_CHAN_SPEECH_ENC_AMR_5_15:
      case COD_CHAN_SPEECH_ENC_AMR_6_7:
      case COD_CHAN_SPEECH_ENC_AMR_7_4:
      case COD_CHAN_SPEECH_ENC_AMR_7_95:
      case COD_CHAN_SPEECH_ENC_AMR_10_2:
      case COD_CHAN_SPEECH_ENC_AMR_12_2:
         if ((!pDev->caps.bAMRE) && (pDev->caps.CODECS & CODEC_AMR_NB))
            bRet = IFX_TRUE;
         break;
      /* G.728, 16 kbit/s */
      case COD_CHAN_SPEECH_ENC_G728_16:
         if ((pDev->caps.CODECS & CODEC_G728))
         bRet = IFX_TRUE;
         break;
      /* G.729A/B, 8 and 11.8 kbit/s */
      case COD_CHAN_SPEECH_ENC_G729AB_8:
         if ((pDev->caps.CODECS & CODEC_G729AB))
         bRet = IFX_TRUE;
         break;
      /* G.722, 64 kbit/s */
      case COD_CHAN_SPEECH_ENC_G722_64:
         if ((pDev->caps.CODECS & CODEC_G722))
         bRet = IFX_TRUE;
         break;
      /* G.722.1, 24 and 32 kbit/s */
      case COD_CHAN_SPEECH_ENC_G7221_24:
      case COD_CHAN_SPEECH_ENC_G7221_32:
         if ((pDev->caps.CODECS & CODEC_G722_1))
         bRet = IFX_TRUE;
         break;
      /* ILBC, 13.3 and 15.2 kbit/s */
      case COD_CHAN_SPEECH_ENC_ILBC_13_3:
      case COD_CHAN_SPEECH_ENC_ILBC_15_2:
         if ((pDev->caps.CODECS & CODEC_ILBC))
         bRet = IFX_TRUE;
         break;
      /* G723.1, 5.3 and 6.3 kbit/s */
      case COD_CHAN_SPEECH_ENC_G7231_5_3:
      case COD_CHAN_SPEECH_ENC_G7231_6_3:
         if ((pDev->caps.CODECS & CODEC_G723_1))
         bRet = IFX_TRUE;
         break;
      /* G.729E, 11.8 kbit/s */
      case COD_CHAN_SPEECH_ENC_G729E_11_8:
         if ((pDev->caps.CODECS & CODEC_G729E))
         bRet = IFX_TRUE;
         break;
      default:
         bRet = IFX_FALSE;
   }
   /*lint -restore*/

   return bRet;
}


/**
   Function to translate frame length value from TAPI enum to FW encoding.

   \param  nFL          TAPI frame length value.

   \return
   FW frame length value or COD_CHAN_SPEECH_PTE_NO.
*/
static IFX_uint8_t vmmc_cod_trans_fl_tapi2fw (IFX_TAPI_COD_LENGTH_t nFL)
{
   /* range check not neccessary as long as enum and array stay in sync
      and nobody casts any out of range values to the parameter */
   return TranslateFrameLengthTable[nFL];
}


/**
   Function called to check the validity of the co-existence of the requested
   PTE value and encoder algorithm value.

   Check is needed because not every encoder packet time is allowed for each
   encoder algorithm.

   \param  nFrameLength Encoder packet time (PTE) value.
   \param  nCodec       Encoder algorithm value.
   \param  bAMRE        if 0 bitrate is coded in codec type for AMR-NB codec,
                        otherwise bitrate is coded with separate bitfield.

   \return
   IFX_SUCCESS or IFX_ERROR
*/
static IFX_int32_t vmmc_cod_ENCnPTE_Check (IFX_uint8_t nFrameLength,
                                           IFX_uint8_t nCodec,
                                           IFX_uint8_t bAMRE)
{
   IFX_int32_t ret = IFX_SUCCESS;

   switch (nFrameLength)
   {
      /* 10ms, not possible for G.723x, G.722.1, ILBCx and AMR */
      case COD_CHAN_SPEECH_PTE_10 :
         if ((nCodec == COD_CHAN_SPEECH_ENC_G7231_5_3) ||
             (nCodec == COD_CHAN_SPEECH_ENC_G7231_6_3) ||
             (nCodec == COD_CHAN_SPEECH_ENC_ILBC_15_2) ||
             (nCodec == COD_CHAN_SPEECH_ENC_ILBC_13_3) ||
             (nCodec == COD_CHAN_SPEECH_ENC_G7221_24)  ||
             (nCodec == COD_CHAN_SPEECH_ENC_G7221_32)  ||
             AMR_CODEC(nCodec, bAMRE) )
         {
            ret = IFX_ERROR;
         }
         break;
      /* 20ms, not possible for G.723x, ILBC 13.3 and LIN_16kHz  */
      case COD_CHAN_SPEECH_PTE_20 :
         if ((nCodec == COD_CHAN_SPEECH_ENC_G7231_5_3) ||
             (nCodec == COD_CHAN_SPEECH_ENC_G7231_6_3) ||
             (nCodec == COD_CHAN_SPEECH_ENC_ILBC_13_3) ||
             (nCodec == COD_CHAN_SPEECH_ENC_LIN16_16KHZ))
         {
            ret = IFX_ERROR;
         }
         break;
      /* 30ms, not possible for ILBC 15.2, AMR, G.722.1 and LIN_16kHz */
      case COD_CHAN_SPEECH_PTE_30:
         if ((nCodec == COD_CHAN_SPEECH_ENC_ILBC_15_2) ||
             AMR_CODEC(nCodec, bAMRE)                  ||
             (nCodec == COD_CHAN_SPEECH_ENC_G7221_24)  ||
             (nCodec == COD_CHAN_SPEECH_ENC_G7221_32)  ||
             (nCodec == COD_CHAN_SPEECH_ENC_LIN16_16KHZ))
         {
            ret = IFX_ERROR;
         }
         break;
      /* 40ms,  not possible for ILBC 13.3 and G.723x, LIN_8kHz and LIN_16kHz */
      case COD_CHAN_SPEECH_PTE_40 :
         if ((nCodec == COD_CHAN_SPEECH_ENC_G7231_5_3) ||
             (nCodec == COD_CHAN_SPEECH_ENC_G7231_6_3) ||
             (nCodec == COD_CHAN_SPEECH_ENC_ILBC_13_3) ||
             (nCodec == COD_CHAN_SPEECH_ENC_LIN16_8KHZ) ||
             (nCodec == COD_CHAN_SPEECH_ENC_LIN16_16KHZ))
         {
            ret = IFX_ERROR;
         }
         break;
      /* 60ms,  not possible for LIN_8kHz and LIN_16kHz */
      case COD_CHAN_SPEECH_PTE_60 :
         if ((nCodec == COD_CHAN_SPEECH_ENC_LIN16_8KHZ) ||
             (nCodec == COD_CHAN_SPEECH_ENC_LIN16_16KHZ))
         {
            ret = IFX_ERROR;
         }
         break;
      default:
         break;
   }

   return ret;
}


/**
   Sets Coder payload table (RTP) in the streaming direction given.

   \param  pCh          Pointer to the VMMC channel structure.
   \param  bUp          IFX_TRUE : upstream / IFX_FALSE : downstream.
   \param pPTvalues     Pointer to array with PT values.

   \return
      VMMC_statusOk or VMMC_statusErr
   \remark
      This function assumes that the array pPTvalues is at least
      IFX_TAPI_ENC_TYPE_MAX big and supports all coders specified in
      IFX_TAPI_ENC_TYPE_t which is used as index.
*/
static IFX_int32_t vmmc_cod_RTP_PayloadTableSet (VMMC_CHANNEL *pCh,
                                                 IFX_boolean_t bUp,
                                                 IFX_uint8_t *pPTvalues)
{
   IFX_int32_t ret = VMMC_statusErr;
   VMMC_DEVICE   *pDev = pCh->pParent;
   IFX_uint32_t  *pCmd;
   IFX_uint16_t  nCount, i;

   COD_CHAN_RTP_SUP_CFG_US_t *pCodRtpConfUs;
   COD_CHAN_RTP_SUP_CFG_DS_t *pCodRtpConfDs;

   pCodRtpConfUs = &pCh->pCOD->fw_cod_rtp_us_conf;
   pCodRtpConfDs = &pCh->pCOD->fw_cod_rtp_ds_conf;

   /* range check and check for conflicts with the event PT */
   for (i=0 ; i<IFX_TAPI_ENC_TYPE_MAX; i++)
   {
      /* FW accepts payload types 0x00 - 0x7F,
         bit 0x80 is reserved for the SID bit */
      if (pPTvalues[i] & 0x80)
      {
         TRACE(VMMC, DBG_LEVEL_HIGH,
         ("ERROR RTP payload type (0x%X) out of range 0x00-0x7F\n",
           pPTvalues[i]));
          /* errmsg: payloadtype out of range (0x00-0x7F) */
         RETURN_STATUS (VMMC_statusRtpPtOutOfRange);
      }

      /* check for conflicts with the event payload type */
      if (pPTvalues[i] == pCh->nEvtPT)
      {
         TRACE(VMMC, DBG_LEVEL_HIGH,
              ("ERROR conflict in RTP PT table,"
               "redefinition of the event PT (0x%X)\n", pCh->nEvtPT));
         /* errmsg: event payloadtype redefinition */
         RETURN_STATUS (VMMC_statusRtpEvtPtRedefinition);
      }
   }

   /* upstream direction */
   if (bUp == IFX_TRUE)
   {
      /* set commands data words using cached value to avoid reading
         the coder channel configuration */
      pCodRtpConfUs->SID2   = COD_CHAN_RTP_SUP_CFG_US_SID_RTP_PT;
      pCodRtpConfUs->PT2    = pPTvalues [IFX_TAPI_ENC_TYPE_ALAW];

      pCodRtpConfUs->SID3   = COD_CHAN_RTP_SUP_CFG_US_SID_RTP_PT;
      pCodRtpConfUs->PT3    = pPTvalues [IFX_TAPI_ENC_TYPE_MLAW];

      pCodRtpConfUs->SID4   = COD_CHAN_RTP_SUP_CFG_US_SID_RTP_PT;
      pCodRtpConfUs->PT4    = pPTvalues [IFX_TAPI_ENC_TYPE_G726_16];

      pCodRtpConfUs->SID5   = COD_CHAN_RTP_SUP_CFG_US_SID_RTP_PT;
      pCodRtpConfUs->PT5    = pPTvalues [IFX_TAPI_ENC_TYPE_G726_24];

      pCodRtpConfUs->SID6   = COD_CHAN_RTP_SUP_CFG_US_SID_RTP_PT;
      pCodRtpConfUs->PT6    = pPTvalues [IFX_TAPI_ENC_TYPE_G726_32];

      pCodRtpConfUs->SID7   = COD_CHAN_RTP_SUP_CFG_US_SID_RTP_PT;
      pCodRtpConfUs->PT7    = pPTvalues [IFX_TAPI_ENC_TYPE_G726_40];

      if (pDev->caps.bAMRE)
      {
         pCodRtpConfUs->SID8   = COD_CHAN_RTP_SUP_CFG_US_SID_SAME_PT;
         pCodRtpConfUs->PT8    = (0 != pPTvalues [IFX_TAPI_ENC_TYPE_AMR_NB]) ?
            pPTvalues [IFX_TAPI_ENC_TYPE_AMR_NB] :
            pPTvalues [IFX_TAPI_ENC_TYPE_AMR_4_75];

         pCodRtpConfUs->SID9   = COD_CHAN_RTP_SUP_CFG_US_SID_SAME_PT;
         pCodRtpConfUs->PT9    = 0x7f;

         pCodRtpConfUs->SID10  = COD_CHAN_RTP_SUP_CFG_US_SID_SAME_PT;
         pCodRtpConfUs->PT10   = pPTvalues [IFX_TAPI_ENC_TYPE_AMR_WB];

         pCodRtpConfUs->SID11  = COD_CHAN_RTP_SUP_CFG_US_SID_SAME_PT;
         pCodRtpConfUs->PT11   = 0x7f;

         pCodRtpConfUs->SID12  = COD_CHAN_RTP_SUP_CFG_US_SID_SAME_PT;
         pCodRtpConfUs->PT12   = 0x7f;

         pCodRtpConfUs->SID13  = COD_CHAN_RTP_SUP_CFG_US_SID_SAME_PT;
         pCodRtpConfUs->PT13   = 0x7f;

         pCodRtpConfUs->SID14  = COD_CHAN_RTP_SUP_CFG_US_SID_SAME_PT;
         pCodRtpConfUs->PT14   = 0x7f;

         pCodRtpConfUs->SID15  = COD_CHAN_RTP_SUP_CFG_US_SID_SAME_PT;
         pCodRtpConfUs->PT15   = 0x7f;
      }
      else
      {
         pCodRtpConfUs->SID8   = COD_CHAN_RTP_SUP_CFG_US_SID_SAME_PT;
         pCodRtpConfUs->PT8    = pPTvalues [IFX_TAPI_ENC_TYPE_AMR_4_75];

         pCodRtpConfUs->SID9   = COD_CHAN_RTP_SUP_CFG_US_SID_SAME_PT;
         pCodRtpConfUs->PT9    = pPTvalues [IFX_TAPI_ENC_TYPE_AMR_5_15];

         pCodRtpConfUs->SID10  = COD_CHAN_RTP_SUP_CFG_US_SID_SAME_PT;
         pCodRtpConfUs->PT10   = pPTvalues [IFX_TAPI_ENC_TYPE_AMR_5_9];

         pCodRtpConfUs->SID11  = COD_CHAN_RTP_SUP_CFG_US_SID_SAME_PT;
         pCodRtpConfUs->PT11   = pPTvalues [IFX_TAPI_ENC_TYPE_AMR_6_7];

         pCodRtpConfUs->SID12  = COD_CHAN_RTP_SUP_CFG_US_SID_SAME_PT;
         pCodRtpConfUs->PT12   = pPTvalues [IFX_TAPI_ENC_TYPE_AMR_7_4];

         pCodRtpConfUs->SID13  = COD_CHAN_RTP_SUP_CFG_US_SID_SAME_PT;
         pCodRtpConfUs->PT13   = pPTvalues [IFX_TAPI_ENC_TYPE_AMR_7_95];

         pCodRtpConfUs->SID14  = COD_CHAN_RTP_SUP_CFG_US_SID_SAME_PT;
         pCodRtpConfUs->PT14   = pPTvalues [IFX_TAPI_ENC_TYPE_AMR_10_2];

         pCodRtpConfUs->SID15  = COD_CHAN_RTP_SUP_CFG_US_SID_SAME_PT;
         pCodRtpConfUs->PT15   = pPTvalues [IFX_TAPI_ENC_TYPE_AMR_12_2];
      }

      pCodRtpConfUs->SID16  = COD_CHAN_RTP_SUP_CFG_US_SID_SAME_PT;
      pCodRtpConfUs->PT16   = pPTvalues [IFX_TAPI_ENC_TYPE_G728];

      pCodRtpConfUs->SID17  = COD_CHAN_RTP_SUP_CFG_US_SID_SAME_PT;
      pCodRtpConfUs->PT17   = 0x7f;

      pCodRtpConfUs->SID18  = COD_CHAN_RTP_SUP_CFG_US_SID_SAME_PT;
      pCodRtpConfUs->PT18   = pPTvalues [IFX_TAPI_ENC_TYPE_G729];

      pCodRtpConfUs->SID19  = COD_CHAN_RTP_SUP_CFG_US_SID_SAME_PT;
      pCodRtpConfUs->PT19   = pPTvalues [IFX_TAPI_ENC_TYPE_G729_E];

      pCodRtpConfUs->SID20  = COD_CHAN_RTP_SUP_CFG_US_SID_SAME_PT;
      pCodRtpConfUs->PT20   = pPTvalues [IFX_TAPI_ENC_TYPE_G7221_24];

      pCodRtpConfUs->SID21  = COD_CHAN_RTP_SUP_CFG_US_SID_SAME_PT;
      pCodRtpConfUs->PT21   = pPTvalues [IFX_TAPI_ENC_TYPE_G7221_32];

      pCodRtpConfUs->SID22  = COD_CHAN_RTP_SUP_CFG_US_SID_RTP_PT;
      pCodRtpConfUs->PT22   = pPTvalues [IFX_TAPI_ENC_TYPE_G722_64];

      pCodRtpConfUs->SID23  = COD_CHAN_RTP_SUP_CFG_US_SID_SAME_PT;
      pCodRtpConfUs->PT23   = 0x7f;

      pCodRtpConfUs->SID24  = COD_CHAN_RTP_SUP_CFG_US_SID_RTP_PT;
      pCodRtpConfUs->PT24   = pPTvalues[IFX_TAPI_ENC_TYPE_LIN16_8];

      pCodRtpConfUs->SID25  = COD_CHAN_RTP_SUP_CFG_US_SID_RTP_PT;
      pCodRtpConfUs->PT25   = pPTvalues[IFX_TAPI_ENC_TYPE_LIN16_16];

      pCodRtpConfUs->SID26  = COD_CHAN_RTP_SUP_CFG_US_SID_SAME_PT;
      pCodRtpConfUs->PT26   = pPTvalues [IFX_TAPI_ENC_TYPE_ILBC_152];

      pCodRtpConfUs->SID27  = COD_CHAN_RTP_SUP_CFG_US_SID_SAME_PT;
      pCodRtpConfUs->PT27   = pPTvalues [IFX_TAPI_ENC_TYPE_ILBC_133];

      pCodRtpConfUs->SID28  = COD_CHAN_RTP_SUP_CFG_US_SID_SAME_PT;
      pCodRtpConfUs->PT28   = pPTvalues [IFX_TAPI_ENC_TYPE_G723_53];

      pCodRtpConfUs->SID29  = COD_CHAN_RTP_SUP_CFG_US_SID_SAME_PT;
      pCodRtpConfUs->PT29   = pPTvalues [IFX_TAPI_ENC_TYPE_G723_63];

      pCodRtpConfUs->SID30  = COD_CHAN_RTP_SUP_CFG_US_SID_SAME_PT;
      pCodRtpConfUs->PT30   = pPTvalues [IFX_TAPI_ENC_TYPE_ALAW_VBD];

      pCodRtpConfUs->SID31  = COD_CHAN_RTP_SUP_CFG_US_SID_SAME_PT;
      pCodRtpConfUs->PT31   = pPTvalues [IFX_TAPI_ENC_TYPE_MLAW_VBD];

      pCmd = (IFX_uint32_t *) pCodRtpConfUs;
      nCount = sizeof (COD_CHAN_RTP_SUP_CFG_US_t) - CMD_HDR_CNT;
   }
   else /* downstream direction */
   {
      pCodRtpConfDs->PT2   = pPTvalues [IFX_TAPI_ENC_TYPE_ALAW];
      pCodRtpConfDs->PT3   = pPTvalues [IFX_TAPI_ENC_TYPE_MLAW];
      pCodRtpConfDs->PT4   = pPTvalues [IFX_TAPI_ENC_TYPE_G726_16];
      pCodRtpConfDs->PT5   = pPTvalues [IFX_TAPI_ENC_TYPE_G726_24];
      pCodRtpConfDs->PT6   = pPTvalues [IFX_TAPI_ENC_TYPE_G726_32];
      pCodRtpConfDs->PT7   = pPTvalues [IFX_TAPI_ENC_TYPE_G726_40];
      if (pDev->caps.bAMRE)
      {
         pCodRtpConfDs->PT8   = (0 != pPTvalues [IFX_TAPI_ENC_TYPE_AMR_NB]) ?
            pPTvalues [IFX_TAPI_ENC_TYPE_AMR_NB] :
            pPTvalues [IFX_TAPI_ENC_TYPE_AMR_4_75];
         pCodRtpConfDs->PT9   = 0x7f;
         pCodRtpConfDs->PT10  = pPTvalues [IFX_TAPI_ENC_TYPE_AMR_WB];
         pCodRtpConfDs->PT11  = 0x7f;
         pCodRtpConfDs->PT12  = 0x7f;
         pCodRtpConfDs->PT13  = 0x7f;
         pCodRtpConfDs->PT14  = 0x7f;
         pCodRtpConfDs->PT15  = 0x7f;
      }
      else
      {
         pCodRtpConfDs->PT8   = pPTvalues [IFX_TAPI_ENC_TYPE_AMR_4_75];
         pCodRtpConfDs->PT9   = pPTvalues [IFX_TAPI_ENC_TYPE_AMR_5_15];
         pCodRtpConfDs->PT10  = pPTvalues [IFX_TAPI_ENC_TYPE_AMR_5_9];
         pCodRtpConfDs->PT11  = pPTvalues [IFX_TAPI_ENC_TYPE_AMR_6_7];
         pCodRtpConfDs->PT12  = pPTvalues [IFX_TAPI_ENC_TYPE_AMR_7_4];
         pCodRtpConfDs->PT13  = pPTvalues [IFX_TAPI_ENC_TYPE_AMR_7_95];
         pCodRtpConfDs->PT14  = pPTvalues [IFX_TAPI_ENC_TYPE_AMR_10_2];
         pCodRtpConfDs->PT15  = pPTvalues [IFX_TAPI_ENC_TYPE_AMR_12_2];
      }
      pCodRtpConfDs->PT16  = pPTvalues [IFX_TAPI_ENC_TYPE_G728];
      pCodRtpConfDs->PT17  = 0x7f;
      pCodRtpConfDs->PT18  = pPTvalues [IFX_TAPI_ENC_TYPE_G729];
      pCodRtpConfDs->PT19  = pPTvalues [IFX_TAPI_ENC_TYPE_G729_E];
      pCodRtpConfDs->PT20  = pPTvalues [IFX_TAPI_ENC_TYPE_G7221_24];
      pCodRtpConfDs->PT21  = pPTvalues [IFX_TAPI_ENC_TYPE_G7221_32];
      pCodRtpConfDs->PT22  = pPTvalues [IFX_TAPI_ENC_TYPE_G722_64];
      pCodRtpConfDs->PT23  = 0x7f;
      pCodRtpConfDs->PT24  = pPTvalues [IFX_TAPI_ENC_TYPE_LIN16_8];
      pCodRtpConfDs->PT25  = pPTvalues [IFX_TAPI_ENC_TYPE_LIN16_16];
      pCodRtpConfDs->PT26  = pPTvalues [IFX_TAPI_ENC_TYPE_ILBC_152];
      pCodRtpConfDs->PT27  = pPTvalues [IFX_TAPI_ENC_TYPE_ILBC_133];
      pCodRtpConfDs->PT28  = pPTvalues [IFX_TAPI_ENC_TYPE_G723_53];
      pCodRtpConfDs->PT29  = pPTvalues [IFX_TAPI_ENC_TYPE_G723_63];
      pCodRtpConfDs->PT30  = pPTvalues [IFX_TAPI_ENC_TYPE_ALAW_VBD];
      pCodRtpConfDs->PT31  = pPTvalues [IFX_TAPI_ENC_TYPE_MLAW_VBD];

      pCmd = (IFX_uint32_t *) pCodRtpConfDs;
      nCount = sizeof (COD_CHAN_RTP_SUP_CFG_DS_t) - CMD_HDR_CNT;
   }

   ret = CmdWrite (pDev, pCmd, nCount);

   return ret;
}


/**
   Checks whether the new coder and decoder algorithm selected is a wideband
   capable codec or not and store the state in the CON module.

   \param  pCh    - pointer to VMMC channel structure

*/
static IFX_void_t vmmc_cod_WidebandCodecCheck( VMMC_CHANNEL *pCh )
{
   if ((pCh->pCOD->fw_cod_ch_speech.EN == COD_CHAN_SPEECH_DISABLE) &&
       (pCh->pCOD->enc_running == IFX_FALSE))
   {
      VMMC_CON_ModuleSamplingModeSet (pCh, VMMCDSP_MT_COD, VMMC_CON_SMPL_OFF);
   }
   else
   if((pCh->pCOD->curr_dec == COD_CHAN_SPEECH_ENC_G722_64) ||
      (pCh->pCOD->curr_dec == COD_CHAN_SPEECH_ENC_G7221_24) ||
      (pCh->pCOD->curr_dec == COD_CHAN_SPEECH_ENC_G7221_32) ||
      (pCh->pCOD->curr_dec == COD_CHAN_SPEECH_ENC_LIN16_16KHZ) ||
      /* Decoding of encoder depends on AMRE field. */
      (pCh->pCOD->curr_dec == COD_CHAN_SPEECH_ENC_AMR_WB &&
       pCh->pParent->caps.bAMRE) ||
      (pCh->pCOD->enc_conf == COD_CHAN_SPEECH_ENC_G722_64) ||
      (pCh->pCOD->enc_conf == COD_CHAN_SPEECH_ENC_G7221_24) ||
      (pCh->pCOD->enc_conf == COD_CHAN_SPEECH_ENC_G7221_32) ||
      (pCh->pCOD->enc_conf == COD_CHAN_SPEECH_ENC_LIN16_16KHZ) ||
      /* Decoding of encoder depends on AMRE field. */
      (pCh->pCOD->enc_conf == COD_CHAN_SPEECH_ENC_AMR_WB &&
       pCh->pParent->caps.bAMRE) )
   {
      VMMC_CON_ModuleSamplingModeSet (pCh, VMMCDSP_MT_COD, VMMC_CON_SMPL_WB);
   }
   else
   {
      VMMC_CON_ModuleSamplingModeSet (pCh, VMMCDSP_MT_COD, VMMC_CON_SMPL_NB);
   }
}


/**
   Enables or Disables voice coder channel according to nMode

   \param  pCh          Pointer to the VMMC channel structure.
   \param  nMode        0: off, 1: on.

   \return
      - VMMC_statusOk if successful

   \remarks
   To succesfully modify the coder channel settings, at least the two first
   command Words should be programed.
   Function should protect access to fw messages
   After activating the coder activate stored ET bits: DTMF receiver and
   generator, ATD, UTD. For FW_ETU Sig-Ch and no generator is touched.
   Remember all ET bits and switch ET bits to zero: DTMF receiver and
   generator, ATD, UTD. For ETU Sig-Ch and no generator.
*/
static IFX_int32_t vmmc_cod_Voice_Enable (VMMC_CHANNEL *pCh, IFX_uint8_t nMode)
{
   VMMC_DEVICE       *pDev   = (VMMC_DEVICE*) (pCh->pParent);
   COD_CHAN_SPEECH_t *pCodCh = &pCh->pCOD->fw_cod_ch_speech;
   IFX_int32_t ret = VMMC_statusOk, bSet = IFX_FALSE;

   /* switch on */
   if ((nMode == 1) && !(pCodCh->EN))
   {
      pCodCh->EN = COD_CHAN_SPEECH_ENABLE;
      bSet = IFX_TRUE;

      /* SIG and COD are the data-channel so both must be enabled together. */
      ret = VMMC_SIG_AutoChStop (pCh, IFX_TRUE);
   }
   /* switch off */
   else if ((nMode == 0) && (pCodCh->EN))
   {
      pCodCh->EN = COD_CHAN_SPEECH_DISABLE;
      bSet = IFX_TRUE;
      ret = VMMC_SIG_UpdateEventTrans(pCh, IFX_FALSE);

      /* Disable SIG if possible; COD enable bit is already cleared. */
      if (ret == IFX_SUCCESS)
      {
         ret = VMMC_SIG_AutoChStop (pCh, IFX_FALSE);
      }

      /* Disable AGC before turning off the coder channel. */
      if (ret == IFX_SUCCESS)
      {
         /* This calls the internal function to avoid a deadlock due to
            protection because here we are already in an protected area. */
         ret = vmmc_cod_AGC_Enable (pCh, IFX_TAPI_ENC_AGC_MODE_DISABLE);
      }

#ifdef VMMC_FEAT_ANNOUNCEMENTS
      /* Disable Announcement before turning off the coder channel. */
      if (ret == IFX_SUCCESS)
      {
         ret = IFX_TAPI_LL_Ann_Stop ((IFX_TAPI_LL_CH_t*)pCh);
      }
#endif /* VMMC_FEAT_ANNOUNCEMENTS */
   }

   if (ret == VMMC_statusOk && bSet)
   {
      ret = CmdWrite (pDev, (IFX_uint32_t *) pCodCh,
                      sizeof (COD_CHAN_SPEECH_t) - CMD_HDR_CNT);
   }

   return ret;
}


/* ============================= */
/* Global function definition    */
/* ============================= */

/**
   Sets the Voice Activation Detection mode.

   This function sets the SIC bit (Silence Compression) of Coder Channel's
   cofiguration if VAD is on. Otherwise, it sets it to zero. Recording Codec
   G.729E (11.8kbps) and ILBC do not support VAD.
   In Case of VAD with comfort noise generation, CNG bit will be set only if
   Codec is G711.
   SIC maybe modified in G.711 and G.723  on the fly.
   The SIC bit is don't care for G728, G729E, ILBC. No error is returned.
   For G.729 A/B coder must be stopped and started (ENC =0), which resets
   the statistics. It is highly recommend to configure VAD before coder start.

\param pLLChannel    Handle to TAPI low level channel structure
\param nVAD          IFX_TRUE: VAD switched on
                     IFX_FALSE: VAD is off
\return
   - VMMC_statusParam The parameters are wrong
   - VMMC_statusCmdWr Writing the command has failed
   - VMMC_statusCodRun Change of SC is not possible while coder G.729AB is
         running. According to the specification of the coder the silence
         compression bit may not be changed.
   - VMMC_statusOk if successful
*/
IFX_int32_t VMMC_TAPI_LL_COD_VAD_Cfg (IFX_TAPI_LL_CH_t *pLLChannel, IFX_int32_t nVAD)
{
   VMMC_CHANNEL      *pCh  = (VMMC_CHANNEL *)pLLChannel;
   VMMC_DEVICE       *pDev = pCh->pParent;
   IFX_int32_t ret = VMMC_statusOk;
   IFX_uint8_t  sic_bit, cng_bit, ns_bit;

   switch (nVAD)
   {
   case  IFX_TAPI_ENC_VAD_NOVAD:
      /* deactivate silence compression, deactivate comfort noise generation
         (SIC=0, NS=0), CNG=0 */
      TRACE (VMMC, DBG_LEVEL_LOW,
            ("INFO: VMMC_TAPI_LL_COD_VAD_Cfg: VMMC%d,Ch%d: "
             "IFX_TAPI_ENC_VAD_NOVAD\n", pDev->nDevNr, pCh->nChannel - 1));
      sic_bit = 0;
      ns_bit = 0;
      cng_bit = 0;
      break;
   case IFX_TAPI_ENC_VAD_CNG_ONLY:
      /* deactivate silence compression, activate comfort noise generation
         (SIC=0, NS=0), CNG=1 */
      TRACE (VMMC, DBG_LEVEL_LOW,
            ("INFO: VMMC_TAPI_LL_COD_VAD_Cfg: VMMC%d,Ch%d: "
             "IFX_TAPI_ENC_VAD_CNG_ONLY\n", pDev->nDevNr, pCh->nChannel - 1));
      sic_bit = 0;
      ns_bit = 0;
      /* Comfort noise generation is switched on in the
         case of a G.711 SID packet */
      cng_bit = 1;
      break;
   case IFX_TAPI_ENC_VAD_SC_ONLY:
      /* activate silence compression, deactivate comfort noise generation
         (SIC=1, NS=0), CNG=0 */
      TRACE (VMMC, DBG_LEVEL_LOW,
            ("INFO: VMMC_TAPI_LL_COD_VAD_Cfg: VMMC%d,Ch%d: "
             "IFX_TAPI_ENC_VAD_SC_ONLY\n", pDev->nDevNr, pCh->nChannel - 1));
      sic_bit = 1;
      ns_bit = 0;
      cng_bit = 0;
      break;
   case IFX_TAPI_ENC_VAD_ON:
      /* active silence compression, activate comfort noise generation
         (background noise level as well as the spectral coefficients
          are used) */
      TRACE (VMMC, DBG_LEVEL_LOW,
            ("INFO: VMMC_TAPI_LL_COD_VAD_Cfg: VMMC%d,Ch%d: "
             "IFX_TAPI_ENC_VAD_ON\n", pDev->nDevNr, pCh->nChannel - 1));
      sic_bit = 1;
      cng_bit = 1;
      ns_bit  = 1;
      break;
   case IFX_TAPI_ENC_VAD_G711:
      /* active silence compression, activate comfort noise generation
         (background noise level used only) */
      TRACE (VMMC, DBG_LEVEL_LOW,
            ("INFO: VMMC_TAPI_LL_COD_VAD_Cfg: VMMC%d,Ch%d: "
             "IFX_TAPI_ENC_VAD_G711\n", pDev->nDevNr, pCh->nChannel - 1));
      sic_bit = 1;
      cng_bit = 1;
      ns_bit = 0;
      break;
   default:
      /* errmsg: Invalid VAD parameter specified */
      RETURN_STATUS (VMMC_statusCodInvalVad);
   }

   /* protect fw messages */
   VMMC_OS_MutexGet (&pCh->chAcc);

   /* In case of coders other than G.722, G.726 and G.711 bit NS is don't care,
    * however here we set it to 0, as precaution.
    */
   if (!((pCh->pCOD->fw_cod_ch_speech.ENC ==  COD_CHAN_SPEECH_ENC_G726_24) ||
         (pCh->pCOD->fw_cod_ch_speech.ENC ==  COD_CHAN_SPEECH_ENC_G726_32) ||
         (pCh->pCOD->fw_cod_ch_speech.ENC ==  COD_CHAN_SPEECH_ENC_G726_40) ||
         (pCh->pCOD->fw_cod_ch_speech.ENC ==  COD_CHAN_SPEECH_ENC_G711_ALAW) ||
         (pCh->pCOD->fw_cod_ch_speech.ENC ==  COD_CHAN_SPEECH_ENC_G711_MLAW) ||
         (pCh->pCOD->fw_cod_ch_speech.ENC ==  COD_CHAN_SPEECH_ENC_G722_64)))
   {
      ns_bit = 0;
   }

   /* In case of G.728, G.729E, and iLBC the silence compression is not
      configurable(SIC must be set 0), so here we silently suppress any
      attempt to set bit SIC */
   if ((sic_bit == 1) &&
       ((pCh->pCOD->fw_cod_ch_speech.ENC == COD_CHAN_SPEECH_ENC_G728_16) ||
        (pCh->pCOD->fw_cod_ch_speech.ENC == COD_CHAN_SPEECH_ENC_G729E_11_8) ||
        (pCh->pCOD->fw_cod_ch_speech.ENC == COD_CHAN_SPEECH_ENC_ILBC_15_2) ||
        (pCh->pCOD->fw_cod_ch_speech.ENC == COD_CHAN_SPEECH_ENC_ILBC_13_3) ||
        (pCh->pCOD->fw_cod_ch_speech.ENC == COD_CHAN_SPEECH_ENC_G711_ALAW_VBD) ||
        (pCh->pCOD->fw_cod_ch_speech.ENC == COD_CHAN_SPEECH_ENC_G711_MLAW_VBD)))
   {
      TRACE (VMMC, DBG_LEVEL_LOW,
            ("INFO: VMMC_TAPI_LL_COD_VAD_Cfg: VMMC%d,Ch%d: silence "
             "compression is not configurable for the running coder.\n",
             pDev->nDevNr, pCh->nChannel - 1));
      sic_bit = 0;
      ns_bit = 0;
   }

   /* check if there is a need to send an update firmware message.
      only send a message if at least one bit has changed */
   if (( ns_bit != pCh->pCOD->fw_cod_ch_speech.NS) ||
       (cng_bit != pCh->pCOD->fw_cod_ch_speech.CNG) ||
       (sic_bit != pCh->pCOD->fw_cod_ch_speech.SIC))
   {
      /* write the modified bits to the firmware message
         and send it to the command mailbox */
      pCh->pCOD->fw_cod_ch_speech.NS  = ns_bit;
      pCh->pCOD->fw_cod_ch_speech.CNG = cng_bit;
      pCh->pCOD->fw_cod_ch_speech.SIC = pCh->pCOD->sc_bit = sic_bit;

      /*  set the requested VAD mode */
      ret = CmdWrite (pDev, (IFX_uint32_t *)&pCh->pCOD->fw_cod_ch_speech,
                      COD_CHAN_SPEECH_LEN);
   }

   /* release protection */
   VMMC_OS_MutexRelease (&pCh->chAcc);

   return ret;
}


/**
   Configures a set of parameters related to the AGC (Automatic Gain Control)
   block, available in the Encoder direction only. AGC functional block is part
   of the Coder module. Not all configurable parameters can be configured with
   this interface function. See below for a list of parameters which retain their
   reset values.

   The parameters passed in IFX_TAPI_ENC_AGC_CFG_t data structure are in dB,
   so mapping tables are used to convert them into hex value suitable for
   programming.

\param pLLChannel Handle to TAPI low level channel structure
\param pAGC_Cfg   Enable (IFX_TAPI_ENC_AGC_MODE_ENABLE)
                  or Disable (IFX_TAPI_ENC_AGC_MODE_DISABLE) AGC
\return
   - VMMC_statusParam The parameters are wrong, e.g. when exeeding the limits
   - VMMC_statusCmdWr Writing the command has failed
   - VMMC_statusOk if successful
*/
IFX_int32_t VMMC_TAPI_LL_COD_AGC_Cfg (
   IFX_TAPI_LL_CH_t *pLLChannel,
   IFX_TAPI_ENC_AGC_CFG_t const *pAGC_Cfg)
{
   VMMC_CHANNEL *pCh = (VMMC_CHANNEL *)pLLChannel;
   VMMC_DEVICE *pDev = pCh->pParent;
   RES_AGC_COEF_t *pAgcCoeff;
   IFX_int32_t ret;

   /* Check if given parameter are in range */
   if ((pAGC_Cfg->com > AGC_CONFIG_COM_MAX)    ||
       (pAGC_Cfg->com < AGC_CONFIG_COM_MIN)    ||
       (pAGC_Cfg->gain > AGC_CONFIG_GAIN_MAX)  ||
       (pAGC_Cfg->gain < AGC_CONFIG_GAIN_MIN)  ||
       (pAGC_Cfg->att > AGC_CONFIG_ATT_MAX)    ||
       (pAGC_Cfg->att < AGC_CONFIG_ATT_MIN)    ||
       (pAGC_Cfg->lim > AGC_CONFIG_LIM_MAX)    ||
       (pAGC_Cfg->lim < AGC_CONFIG_LIM_MIN))
   {
      RETURN_STATUS (VMMC_statusParam);
   }

   /* Check number of available resources */
   if ((pCh->pCOD == IFX_NULL) ||
       ((pCh->nChannel-1) >= (IFX_uint8_t)pDev->caps.nAGC))
   {
      /* errmsg: The requested resource is not available. */
      RETURN_STATUS (VMMC_statusNoRes);
   }

   /* protect fw messages */
   VMMC_OS_MutexGet (&pCh->chAcc);

   /* Get pointer to fw message now that we know that the struct exists. */
   pAgcCoeff = &pCh->pCOD->fw_res_agc_coeff;

   /* Get the register values from the 'dB' values */
   pAgcCoeff->COM   = AgcConfig_Reg_COM[pAGC_Cfg->com + AGC_CONFIG_COM_OFFSET];
   pAgcCoeff->GAIN  = AgcConfig_Reg_GAIN[pAGC_Cfg->gain + AGC_CONFIG_GAIN_OFFSET];
   pAgcCoeff->ATT   = AgcConfig_Reg_ATT[pAGC_Cfg->att + AGC_CONFIG_ATT_OFFSET];
   pAgcCoeff->LIM   = AgcConfig_Reg_LIM[pAGC_Cfg->lim + AGC_CONFIG_LIM_OFFSET];
   /* parameter INIGAIN keeps its reset value of 0dB
      (initial AGC gain/attenuation) */
   /* parameter SPEEDL keeps its reset value of 0.0078
      (Adaptation Speed of the Gain towards lower Values) */
   /* parameter SPEEDH keeps its reset value of 0.0625
      (Adaptation Speed of the Gain towards higher Values) */
   /* parameter DEC keeps its reset value of 8.26 ms
      (Time Constant of the Peak Detector) */
   /* parameter LP keeps its reset value of 0.125 ms
      (Low Pass Time Constant) */

   /* Write the newly configured AGC coefficients */
   ret = CmdWrite(pDev, (IFX_uint32_t *)pAgcCoeff, RES_AGC_COEF_LEN);

   /* release protection */
   VMMC_OS_MutexRelease (&pCh->chAcc);

   RETURN_STATUS (ret);
}


/**
   Enable/Disable the AGC resource inside the device.

   This implementation assumes that the index of the AGC resource is fixed
   assigned to the related index of the Coder-Module.
   The AGC resource is enable or disable, it depending on 'agcMode'.

   \param  pCh          Pointer to the VMMC channel structure.
   \param  agcMode      IFX_TAPI_ENC_AGC_MODE_t enumeration.

   \return
   - VMMC_statusCmdWr Writing the command failed
   - VMMC_statusOk if successful
*/
static IFX_int32_t vmmc_cod_AGC_Enable (VMMC_CHANNEL *pCh,
                                        IFX_TAPI_ENC_AGC_MODE_t agcMode)
{
   VMMC_DEVICE *pDev = pCh->pParent;
   COD_AGC_CTRL_t *pAgcCtrl = &pCh->pCOD->fw_cod_agc_ctrl;
   IFX_int32_t ret = VMMC_statusOk;

   switch (agcMode)
   {
      case IFX_TAPI_ENC_AGC_MODE_DISABLE:
         /* Valid AGC control action, disable */
         break;
      case IFX_TAPI_ENC_AGC_MODE_ENABLE:
      {
         COD_CHAN_SPEECH_t *pCodCh = &pCh->pCOD->fw_cod_ch_speech;

         /* Valid AGC control action, enable.
            However enabling is only allowed if the coder channel is started
            using command CMD_COD_CHAN_SPEECH */
         if (pCodCh->EN == COD_CHAN_SPEECH_DISABLE)
         {
            /* errmsg: Enabling AGC on the coder is only allowed if the coder
               channel is activated */
            RETURN_STATUS (VMMC_statusCodAgc);
         }
         break;
      }

      default:
         RETURN_STATUS (VMMC_statusParam);
   }

   /* Write message only if mode needs an update. */
   if (pAgcCtrl->EN != agcMode)
   {
      pAgcCtrl->EN = agcMode;
      ret = CmdWrite (pDev, (IFX_uint32_t *)pAgcCtrl, COD_AGC_CTRL_LEN);
   }

   RETURN_STATUS (ret);
}


/**
   Enable/Disable the AGC resource inside the device.

   This is just the wrapper for external calls which checks the resource
   and protects against concurrent access.

   \param  pLLChannel   Pointer to the VMMC channel structure.
   \param  agcMode      IFX_TAPI_ENC_AGC_MODE_t enumeration.

   \return
   - VMMC_statusCmdWr Writing the command failed
   - VMMC_statusNoRes The requested resource is not available.
   - VMMC_statusOk if successful
*/
IFX_int32_t VMMC_TAPI_LL_COD_AGC_Enable (IFX_TAPI_LL_CH_t *pLLChannel,
                                        IFX_TAPI_ENC_AGC_MODE_t agcMode)
{
   VMMC_CHANNEL *pCh = (VMMC_CHANNEL *) pLLChannel;
   IFX_int32_t ret;

   /* Work only on channels where COD is initialised */
   if (pCh->pCOD == IFX_NULL)
   {
      /* errmsg: The requested resource is not available. */
      RETURN_STATUS (VMMC_statusNoRes);
   }

   /* protect fwmsg access against concurrent tasks */
   VMMC_OS_MutexGet (&pCh->chAcc);

   ret = vmmc_cod_AGC_Enable (pCh, agcMode);

   /* release protection */
   VMMC_OS_MutexRelease (&pCh->chAcc);

   return ret;
}


/**
   Starts the decoder (downstream).

   If the decoder is already running no further action is performed
   and VMMC_statusOk is returned.

   If the encoder is running only the DEC bit is set which enables the
   decoder.
   If the encoder is not running this function enables the coder channel
   with the existing configuration.
   After activating the coder previously configured ET bits are activated:
   DTMF receiver and MFTD.

   No wideband checking is done in this function. Wideband is checked as soon
   as the decoder change event reports which packets are actually decoded.

   \param  pLLChannel   Pointer to the VMMC channel structure.

   \return
   - VMMC_statusCmdWr Writing the command failed
   - VMMC_statusT38Run Coder activation is not possible while the T.38 data
      pump is running. Firt IFX_TAPI_T38_STOP must be called.
   - VMMC_statusNoRes if called on a channel where there is no COD ressource
   - VMMC_statusOk if successful or not needed
*/
IFX_int32_t VMMC_TAPI_LL_COD_DEC_Start (IFX_TAPI_LL_CH_t *pLLChannel)
{
   VMMC_CHANNEL   *pCh   = (VMMC_CHANNEL *) pLLChannel;
   VMMC_DEVICE    *pDev  = (VMMC_DEVICE*) (pCh->pParent);
   IFX_int32_t     ret   = VMMC_statusErr;

   /* Work only on channels where COD is initialised */
   if (pCh->pCOD == IFX_NULL)
   {
      /* errmsg: The requested resource is not available. */
      RETURN_STATUS (VMMC_statusNoRes);
   }

   VMMC_OS_MutexGet (&pCh->chAcc);

   if (pCh->pCOD->fw_cod_ch_speech.DEC == COD_CHAN_SPEECH_DEC_INACTIVE)
   {
#ifdef VMMC_FEAT_FAX_T38
      if (pCh->pCOD->fw_cod_fax_ctrl.EN == 1)
      {
         VMMC_OS_MutexRelease (&pCh->chAcc);
         RETURN_STATUS (VMMC_statusT38Run);
      }
#endif /* VMMC_FEAT_FAX_T38 */

      /* update fw-msg while protected against concurrent tasks */
      pCh->pCOD->fw_cod_ch_speech.DEC = COD_CHAN_SPEECH_DEC_ACTIVE;

      if (pCh->pCOD->fw_cod_ch_speech.EN == COD_CHAN_SPEECH_DISABLE)
      {
         /* The 'en' bit has to be set too. function 'vmmc_cod_Voice_Enable'
            writes the firmware message to the device. */

         ret = vmmc_cod_Voice_Enable (pCh, 1);

         if (ret != VMMC_statusOk)
         {
            /* enabling the coder-module failed, mark this in the 'dec'
               parameter */
             pCh->pCOD->fw_cod_ch_speech.DEC = COD_CHAN_SPEECH_DEC_INACTIVE;
         }
      }
      else
      {
         /* The 'en' bit was already set in the firmware message but the
            parameter 'dec' has changed. Only if the 'dec' bit is zero, the
            firmware message has to be sent to the device. */
         ret = CmdWrite (pDev, (IFX_uint32_t *) &pCh->pCOD->fw_cod_ch_speech,
                         COD_CHAN_SPEECH_LEN);
      }

      /* Enable the event transmission if configured.
         It is mandatory that encoder must be running as well. */
      if (ret == VMMC_statusOk &&
          pCh->pCOD->fw_cod_ch_speech.ENC != COD_CHAN_SPEECH_ENC_NO)
      {
         ret = VMMC_SIG_UpdateEventTrans(pCh, IFX_TRUE);
      }
   }
   else
   {
      ret = VMMC_statusOk;
   }

   VMMC_OS_MutexRelease (&pCh->chAcc);

   return ret;
}


/**
   Stops the decoder (downstream).

   If the decoder is already switched off no further action is performed
   and VMMC_statusOk is returned.

   If the encoder is running only the DEC bit is set to zero, which
   disables the decoder.

   If no encoder is running the coder channel is deactivated.
   Also all ET bits are deactivated.
   Therefore all ET bits are cached and the ET bits are switched to zero.
   Then it resets the signaling channel.
   This is neccessary to initialze the event playout unit. Otherwise the
   sequence number may not match for the new connection and events could
   be played out.

   \param  pLLChannel   Pointer to the VMMC channel structure.

   \return
   - VMMC_statusCmdWr Writing the command failed
   - VMMC_statusNoRes if called on a channel where there is no COD ressource
   - VMMC_statusOk if successful
*/
IFX_int32_t VMMC_TAPI_LL_COD_DEC_Stop (IFX_TAPI_LL_CH_t *pLLChannel)
{
   VMMC_CHANNEL *pCh  = (VMMC_CHANNEL *) pLLChannel;
   VMMC_DEVICE  *pDev = pCh->pParent;
   IFX_int32_t ret = VMMC_statusErr;

   /* Work only on channels where COD is initialised */
   if (pCh->pCOD == IFX_NULL)
   {
      /* errmsg: The requested resource is not available. */
      RETURN_STATUS (VMMC_statusNoRes);
   }

   /* protect fwmsg against concurrent tasks */
   VMMC_OS_MutexGet (&pCh->chAcc);
   /* Note: please take special care of the unlocking in this code. */

   if (pCh->pCOD->fw_cod_ch_speech.DEC)
   {
#ifdef VMMC_FEAT_ANNOUNCEMENTS
      /* Disable Announcement before turning off the decoder. */
      IFX_TAPI_LL_Ann_Stop ((IFX_TAPI_LL_CH_t*)pCh);
#endif /* VMMC_FEAT_ANNOUNCEMENTS */

      /* decoder has to be updated */
      pCh->pCOD->fw_cod_ch_speech.DEC = COD_CHAN_SPEECH_DEC_INACTIVE;
      if (pCh->pCOD->fw_cod_ch_speech.ENC == COD_CHAN_SPEECH_DISABLE)
      {
         /* encoder is not running -> disable the whole coder-module */
         ret = vmmc_cod_Voice_Enable (pCh, 0);
      }
      else
      {
         /* RTP event support requires both encoder and decoder running.
            Disable the RTP event support before stopping the decoder */
         ret = VMMC_SIG_UpdateEventTrans(pCh, IFX_FALSE);

         if (ret == VMMC_statusOk)
         {
            /* encoder is still running -> do not disable the whole coder-module
               but send the updated firmware message to the device */
            ret = CmdWrite (pDev, (IFX_uint32_t *) &pCh->pCOD->fw_cod_ch_speech,
                         COD_CHAN_SPEECH_LEN);
         }
      }

      if (ret == VMMC_statusOk)
      {
         /* after decoder has been stopped check for necessary WB/NB mode
            changes and configure new sampling mode if needed */
         pCh->pCOD->curr_dec = COD_CHAN_SPEECH_ENC_NO;

         VMMC_OS_MutexRelease (&pCh->chAcc);

         /* Update the wideband status */
         vmmc_cod_WidebandCodecCheck(pCh);
         /* reevaluate the conference that this module belongs to */
         VMMC_CON_MatchConfSmplRate(pCh, VMMCDSP_MT_COD);
      }
      else
      {
         VMMC_OS_MutexRelease (&pCh->chAcc);
      }
   }
   else
   {
      VMMC_OS_MutexRelease (&pCh->chAcc);
      ret = VMMC_statusOk;
   }

   RETURN_STATUS (ret);
}


/**
   Switches on/off the HP filter in the decoder path of the COD module.

   \param  pLLChannel   Pointer to the VMMC channel structure.
   \param  bHp          IFX_FALSE to switch HP filter off
                        IFX_TRUE  to switch HP filter on

   \return
   - VMMC_statusCmdWr Writing the command failed
   - VMMC_statusNoRes if called on a channel where there is no COD ressource
   - VMMC_statusOk if successful
*/
IFX_int32_t VMMC_TAPI_LL_COD_DEC_HP_Set (IFX_TAPI_LL_CH_t *pLLChannel,
                                         IFX_boolean_t bHp)
{
   IFX_int32_t ret = VMMC_statusOk;
   VMMC_CHANNEL *pCh  = (VMMC_CHANNEL *) pLLChannel;
   VMMC_DEVICE  *pDev = pCh->pParent;
   COD_CHAN_SPEECH_t *pCodCh;
   IFX_uint32_t newHP = (bHp == IFX_TRUE) ? 1 : 0;

   /* Work only on channels where COD is initialised */
   if (pCh->pCOD == IFX_NULL)
   {
      /* errmsg: The requested resource is not available. */
      RETURN_STATUS (VMMC_statusNoRes);
   }

   /* protect fwmsg access against concurrent tasks */
   VMMC_OS_MutexGet (&pCh->chAcc);

   /* Get pointer to fw message now that we know that the struct exists. */
   pCodCh = &pCh->pCOD->fw_cod_ch_speech;

   if (pCodCh->HP != newHP)
   {
      /* store new HP setting */
      pCodCh->HP = newHP;

      /* if decoder is currently running write new setting to fw */
      if ((pCodCh->EN != COD_CHAN_SPEECH_DISABLE) &&
          (pCodCh->DEC != COD_CHAN_SPEECH_DEC_INACTIVE))
      {
         ret = CmdWrite (pDev, (IFX_uint32_t *)pCodCh,
                         sizeof(COD_CHAN_SPEECH_t) - CMD_HDR_CNT);
      }
   }

   /* unlock protection */
   VMMC_OS_MutexRelease (&pCh->chAcc);

   return ret;
}


/**
   Starts the coder in upstream / record data

   If the encoder is already running no further action is performed
   and VMMC_statusOk is returned.

   If the decoder is running only the ENC bit is set which enables the
   encoder.
   If the decoder is not running this function enables the coder channel
   with the existing configuration.
   After activating the coder previously configured ET bits are activated:
   DTMF receiver and MFTD.

   \param  pLLChannel   Pointer to the VMMC channel structure.

   \return
   - VMMC_statusCmdWr Writing the command failed
   - VMMC_statusNoRes if called on a channel where there is no COD ressource
   - VMMC_statusT38Run Coder activation is not possible while the T.38 data
      pump is running. First IFX_TAPI_T38_STOP must be called.
   - VMMC_statusOk if successful
*/
IFX_int32_t VMMC_TAPI_LL_COD_ENC_Start (IFX_TAPI_LL_CH_t *pLLChannel)
{
   VMMC_CHANNEL *pCh    = (VMMC_CHANNEL *) pLLChannel;
   VMMC_CODCH_t *pCodCh = pCh->pCOD;
   IFX_int32_t ret = VMMC_statusOk;

   /* Work only on channels where COD is initialised */
   if (pCh->pCOD == IFX_NULL)
   {
      /* errmsg: The requested resource is not available. */
      RETURN_STATUS (VMMC_statusNoRes);
   }

   if (pCodCh->fw_cod_ch_speech.ENC != pCodCh->enc_conf)
   {
#ifdef VMMC_FEAT_FAX_T38
      if (pCh->pCOD->fw_cod_fax_ctrl.EN == 1)
      {
         RETURN_STATUS (VMMC_statusT38Run);
      }
#endif /* VMMC_FEAT_FAX_T38 */

      /* Keep this line before the VMMC_CON_MatchConfSmplRate() call */
      pCh->pCOD->enc_running = IFX_TRUE;
      /* Update the wideband status with the last configuration. */
      vmmc_cod_WidebandCodecCheck(pCh);
      /* Reevaluate the conference that this module belongs to.
         This also sets the configured encoding algorithm and pte.
         This also writes the cached fw-message if the channel is enabled. */
      VMMC_CON_MatchConfSmplRate(pCh, VMMCDSP_MT_COD);

      /* Finally enable the coder channel if not already running */
      VMMC_OS_MutexGet (&pCh->chAcc);
      if (pCodCh->fw_cod_ch_speech.EN == COD_CHAN_SPEECH_DISABLE)
      {
         /* The 'en' bit has to be set too. function
          * 'vmmc_cod_Voice_Enable'
            writes the firmware message to the device */
         ret = vmmc_cod_Voice_Enable (pCh, 1);

         if (ret != VMMC_statusOk)
         {
            /* enabling the coder-module failed, mark this in the 'enc'
               parameter */
            pCodCh->fw_cod_ch_speech.ENC = COD_CHAN_SPEECH_ENC_NO;
            pCh->pCOD->enc_running = IFX_FALSE;
         }
      }

      /* Enable the event transmission if configured.
         It is mandatory that decoder must be running as well. */
      if (ret == VMMC_statusOk &&
          pCodCh->fw_cod_ch_speech.DEC != COD_CHAN_SPEECH_DEC_INACTIVE)
      {
         ret = VMMC_SIG_UpdateEventTrans(pCh, IFX_TRUE);
      }
      VMMC_OS_MutexRelease (&pCh->chAcc);
   }
#if 0
   else
   {
      /* nothing to do. Encoder is already running */
   }
#endif
   return ret;
}


/**
   Stops the encoder (upstream).

   If the encoder is already switched off no further action is performed
   and VMMC_statusOk is returned.

   If the decoder is running only the ENC bit is set to zero, which
   disables the encoder.

   If no decoder is running the coder channel is deactivated.
   Also all ET bits are deactivated.
   Therefore all ET bits are cached and the ET bits are switched to zero.
   Then it resets the signaling channel.
   This is neccessary to initialze the event playout unit. Otherwise the
   sequence number may not match for the new connection and events could
   be played out.

   \param  pLLChannel   Pointer to the VMMC channel structure.

   \return
   - VMMC_statusCmdWr Writing the command failed
   - VMMC_statusNoRes if called on a channel where there is no COD ressource
   - VMMC_statusOk if successful
*/
IFX_int32_t VMMC_TAPI_LL_COD_ENC_Stop (IFX_TAPI_LL_CH_t *pLLChannel)
{
   VMMC_CHANNEL *pCh = (VMMC_CHANNEL *) pLLChannel;
   IFX_int32_t ret;

   /* Work only on channels where COD is initialised */
   if (pCh->pCOD == IFX_NULL)
   {
      /* errmsg: The requested resource is not available. */
      RETURN_STATUS (VMMC_statusNoRes);
   }

   VMMC_OS_MutexGet (&pCh->chAcc);
   if (pCh->pCOD->fw_cod_ch_speech.ENC)
   {
      /* encoder is running and needs to be stopped */

      /* Keep this line before the VMMC_CON_MatchConfSmplRate() call */
      pCh->pCOD->enc_running = IFX_FALSE;

      if (pCh->pCOD->fw_cod_ch_speech.DEC == COD_CHAN_SPEECH_DEC_INACTIVE)
      {
         /* decoder is not running -> disable the whole coder-module */
         ret = vmmc_cod_Voice_Enable (pCh, 0);
         if (ret != VMMC_statusOk)
         {
            VMMC_OS_MutexRelease (&pCh->chAcc);
            return ret;
         }
      }

      VMMC_OS_MutexRelease (&pCh->chAcc);
      /* Update the wideband status */
      vmmc_cod_WidebandCodecCheck(pCh);
      /* Reevaluate the conference that this module belongs to.
         This also clears the configured encoding algorithm in the fw-message.
         This also writes the cached fw-message. */
      ret = VMMC_CON_MatchConfSmplRate(pCh, VMMCDSP_MT_COD);
      VMMC_OS_MutexGet (&pCh->chAcc);
   }
   else
   {
      ret = VMMC_statusOk;
   }

   /* do not try to wakeup or reset the fifos here (!) */

   VMMC_OS_MutexRelease (&pCh->chAcc);

   return ret;
}


/**
   Mute or unmute the coder channel.

   \param  pCh          Pointer to the VMMC channel structure.
   \param  nOnHold      IFX_ENABLE - encoder is put to hold
                        IFX_DISABLE - encoder is put to unhold

   \return
   - VMMC_statusCmdWr Writing the command failed
   - VMMC_statusOk if successful
*/
IFX_int32_t  VMMC_COD_ENC_Hold(VMMC_CHANNEL     *pCh,
                               IFX_operation_t  nOnHold)
{
   COD_CHAN_SPEECH_t *pCodCh = &(pCh->pCOD->fw_cod_ch_speech);
   IFX_int32_t ret = VMMC_statusOk;

   switch (nOnHold)
   {
      default:
      case IFX_DISABLE:
         pCodCh->EM = 0;
         break;
      case IFX_ENABLE:
         pCodCh->EM = 1;
         break;
   }
   /* need to write the full length because of AAL2 bitpacking */
   if((pCodCh->EN) && (pCodCh->ENC))
   {
      ret = CmdWrite(pCh->pParent, (IFX_uint32_t *)pCodCh,
                     sizeof(COD_CHAN_SPEECH_t) - CMD_HDR_CNT);
   }
   return ret;
}


/**
   Put encoder in hold/unhold state.

   This function temporarily stop and restart packet encoding, for example to
   hold/unhold the remote VoIP party.

   \param  pLLChannel   Pointer to the VMMC channel structure.
   \param  nOnHold      IFX_ENABLE - encoder is put to hold
                        IFX_DISABLE - encoder is put to unhold
   \return
   - VMMC_statusCmdWr Writing the command failed
   - VMMC_statusOk if successful
*/
IFX_int32_t VMMC_TAPI_LL_COD_ENC_Hold(IFX_TAPI_LL_CH_t *pLLChannel,
                                      IFX_operation_t nOnHold)
{
   IFX_int32_t ret;
   VMMC_CHANNEL *pCh = (VMMC_CHANNEL *)pLLChannel;

   /* Work only on channels where COD is initialised */
   if (pCh->pCOD == IFX_NULL)
   {
      /* errmsg: The requested resource is not available. */
      RETURN_STATUS (VMMC_statusNoRes);
   }

   VMMC_OS_MutexGet (&pCh->chAcc);
   ret = VMMC_COD_ENC_Hold( pCh, nOnHold );
   VMMC_OS_MutexRelease (&pCh->chAcc);

   return ret;
}


/**
   Sets the Recording Codec and packet length

   \param  pLLChannel   Pointer to the VMMC channel structure.
   \param  pEncCfg      Pointer to the encoder configuration.

   \return
   - VMMC_statusErr if an error occured
   - VMMC_statusOk if successful
*/
IFX_int32_t VMMC_TAPI_LL_COD_ENC_Cfg_Set (IFX_TAPI_LL_CH_t *pLLChannel,
                                         IFX_TAPI_ENC_CFG_SET_t const *pEncCfg)
{
   VMMC_CHANNEL *pCh = (VMMC_CHANNEL *) pLLChannel;
   VMMC_DEVICE *pDev = pCh->pParent;
   VMMC_CODCH_t *pCodCh = pCh->pCOD;
   IFX_uint8_t  nCod, nPte, nBitRateFW = 0, nFec = 0;
   IFX_TAPI_COD_TYPE_t nCoder = pEncCfg->nEncType;
   IFX_TAPI_COD_LENGTH_t nFrameLength = pEncCfg->nFrameLen;
   IFX_TAPI_COD_AAL2_BITPACK_t nBitPack = pEncCfg->AAL2BitPack;
   IFX_TAPI_COD_BITRATE_t nBitRate = pEncCfg->nBitRate;
   IFX_TAPI_COD_REDUNDANCY_t nRedundancy = pEncCfg->nRedundancy;

   /* Work only on channels where COD is initialised */
   if (pCh->pCOD == IFX_NULL)
   {
      /* errmsg: The requested resource is not available. */
      RETURN_STATUS (VMMC_statusNoRes);
   }
   /* Translate codec TAPI parameter to FW value */
   nCod = VMMC_COD_trans_cod_tapi2fw (nCoder, nBitRate, pDev->caps.bAMRE);
   if (nCod == COD_CHAN_SPEECH_ENC_NO)
   {
      /* Unknown encoder type */
      RETURN_INFO_STATUS (VMMC_statusCodconfNotValid, &nCoder, sizeof(nCoder));
   }
   /* Translate framelength TAPI parameter to FW value */
   nPte = vmmc_cod_trans_fl_tapi2fw(nFrameLength);
   if (nPte == COD_CHAN_SPEECH_PTE_NO)
   {
      RETURN_STATUS (VMMC_statusCodTime);
   }
   /* Check if requested frame length is supported by the requested codec */
   if (vmmc_cod_ENCnPTE_Check(nPte, nCod, pDev->caps.bAMRE) != IFX_SUCCESS)
   {
      RETURN_STATUS (VMMC_statusCodTime);
   }
   /* Check whether the requested encoder type is supported by firmware */
   if (vmmc_codec_support_check(pDev, nCod) == IFX_FALSE)
   {
      /* Requested encoder type not supported by this firmware */
      RETURN_INFO_STATUS (VMMC_statusCodconfNotValid, &nCod, sizeof(nCod));
   }
   if (pDev->caps.bAMRE)
   {
      /* Get value of bitrate parameter. */
      nBitRateFW = VMMC_COD_trans_bitrate_tapi2fw(nCoder, nBitRate);
      if (COD_CHAN_SPEECH_BITR_INVALID == nBitRateFW)
      {
         /* Invalid bitrate */
         RETURN_INFO_STATUS (VMMC_statusCodconfNotValid,
                             &nBitRate, sizeof(nBitRate));
      }
   }

   /* Translate redundancy TAPI parameter to FW value. Silently ignore values
      which are not applicable for the selected encoder. */
   nFec = 0;
   if (AMR_CODEC(nCod, pDev->caps.bAMRE))
   {
      switch (nRedundancy)
      {
      case IFX_TAPI_COD_REDUNDANCY_NONE:
         /* nFec = 0; */
         break;
      case IFX_TAPI_COD_REDUNDANCY_RFC3267_SINGLE:
         if ((nPte != COD_CHAN_SPEECH_PTE_20) &&
             (nPte != COD_CHAN_SPEECH_PTE_40))
         {
            /* errmsg: Coder redundancy not supported for the selected
               packetization length. */
            RETURN_STATUS (VMMC_statusCodRedUnsupportedFrameLength);
         }
         nFec = 1;
         break;
      case IFX_TAPI_COD_REDUNDANCY_RFC3267_DOUBLE:
         if ((nPte != COD_CHAN_SPEECH_PTE_20))
         {
            /* errmsg: Coder redundancy not supported for the selected
               packetization length. */
            RETURN_STATUS (VMMC_statusCodRedUnsupportedFrameLength);
         }
         nFec = 2;
         break;
      default:
         /* errmsg: Coder redundancy value invalid. */
         RETURN_STATUS (VMMC_statusCodRedundancyInvalid);
      }
   }

   VMMC_OS_MutexGet (&pCh->chAcc);

   /* bit packing configuration */
   if (!((nBitPack == IFX_TAPI_COD_AAL2_BITPACK) &&
        ((nCoder == IFX_TAPI_COD_TYPE_G726_16) ||
         (nCoder == IFX_TAPI_COD_TYPE_G726_24) ||
         (nCoder == IFX_TAPI_COD_TYPE_G726_32) ||
         (nCoder == IFX_TAPI_COD_TYPE_G726_40))))
   {
      pCodCh->fw_cod_ch_speech.EE = 0;
   }
   else
   {
      pCodCh->fw_cod_ch_speech.EE = 1;
   }

   /* Store the values in cache variables. The values will be written
      to the fw-message cache when we match the sampling mode of the
      entire conference below. This prevents an inconsistent state in the
      cached fw-message which is used by other functions in between here
      and the actual write done in VMMC_COD_SamplingModeSet(). */
   pCodCh->enc_conf = nCod;
   pCodCh->pte_conf = nPte;
   pCodCh->bitr_conf = nBitRateFW;
   pCodCh->fec_conf = nFec;

   /* If the encoder is already running update the wideband status.
      Running state is determined by the enc field of the fw message
      because the enable bit also controls the decoder which might be
      running independently of the encoder. */
   if (pCodCh->fw_cod_ch_speech.ENC)
   {
      /* Encoder is enabled */

      /* Update the wideband status */
      vmmc_cod_WidebandCodecCheck(pCh);
      /* Actual write is done implicitly in here, lock is acquired internally
         again... avoid deadlocks and release the mutex before */
      VMMC_OS_MutexRelease (&pCh->chAcc);
      VMMC_CON_MatchConfSmplRate(pCh, VMMCDSP_MT_COD);
   }
   else
   {
      VMMC_OS_MutexRelease (&pCh->chAcc);
   }

   return VMMC_statusOk;
}


/**
   Configures the Decoder

   \param pLLChannel    Pointer to the VMMC channel structure.
   \param pTapiDecCfg   Pointer to DEC configuration structure.
   \return
   - VMMC_statusCmdWr Writing the command failed
   - VMMC_statusOk if successful
*/
IFX_int32_t VMMC_TAPI_LL_COD_DEC_Cfg_Set (
   IFX_TAPI_LL_CH_t *pLLChannel,
   IFX_TAPI_DEC_CFG_t const *pTapiDecCfg)
{
   VMMC_CHANNEL *pCh    = (VMMC_CHANNEL *) pLLChannel;
   VMMC_DEVICE  *pDev   = pCh->pParent;
   VMMC_CODCH_t *pCodCh = pCh->pCOD;
   IFX_int32_t    ret   = VMMC_statusOk;

   /* Work only on channels where COD is initialised */
   if (pCh->pCOD == IFX_NULL)
   {
      /* errmsg: The requested resource is not available. */
      RETURN_STATUS (VMMC_statusNoRes);
   }

   /* ARGSUSED */
   VMMC_OS_MutexGet (&pCh->chAcc);
   switch (pTapiDecCfg->AAL2BitPack)
   {
      case IFX_TAPI_COD_RTP_BITPACK:
         pCodCh->fw_cod_ch_speech.DE = 0;
         break;
      case IFX_TAPI_COD_AAL2_BITPACK:
         pCodCh->fw_cod_ch_speech.DE = 1;
         break;
   }

   switch (pTapiDecCfg->Plc)
   {
      default:
      case IFX_TAPI_DEC_PLC_CODEC:
         /* Default, if applicable use codec specific PLC.
            Best setting in case of voice calls. */
         pCodCh->fw_cod_ch_speech.BFI = 1;
         pCodCh->fw_cod_ch_speech.PLC = 0;
         break;
      case IFX_TAPI_DEC_PLC_ZERO:
         /* Zero insertion: play out zero. It may be used
            in case of voice band data transmission. */
         pCodCh->fw_cod_ch_speech.BFI = 0;
         pCodCh->fw_cod_ch_speech.PLC = 0;
         break;
      case IFX_TAPI_DEC_PLC_ONE:
         /* One insertion (linear), corresponding to 0x848 G.711.
            It is required for G.722 sample over G.711 ALaw. */
         pCodCh->fw_cod_ch_speech.BFI = 0;
         pCodCh->fw_cod_ch_speech.PLC = 1;
         break;
   }

   /* if the decoder is running send the message to fw
      running state is determined by the dec field of the fw message */
   if (pCodCh->fw_cod_ch_speech.DEC)
   {
      ret = CmdWrite (pDev, (IFX_uint32_t *) &pCodCh->fw_cod_ch_speech,
                      sizeof(COD_CHAN_SPEECH_t) - CMD_HDR_CNT);
   }
   VMMC_OS_MutexRelease (&pCh->chAcc);

   return ret;
}


/**
   Sets the COD interface volume.

   Gain Parameter are given in 'dB'. The range is -24dB ... +12dB.

   \param  pLLChannel   Pointer to the VMMC channel structure.
   \param  pVol         Pointer to IFX_TAPI_LINE_VOLUME_t structure.

   \return
   - VMMC_statusCmdWr Writing the command failed
   - VMMC_statusFuncParm Wrong parameters passed. This code is returned
     when any gain parameter is lower than -24 dB or higher than +12 dB
   - VMMC_statusNoRes if called on a channel where there is no COD resource
   - VMMC_statusOk if successful
*/
IFX_int32_t VMMC_TAPI_LL_COD_Volume_Set (IFX_TAPI_LL_CH_t *pLLChannel,
                                         IFX_TAPI_PKT_VOLUME_t const *pVol)
{
   VMMC_CHANNEL *pCh  = (VMMC_CHANNEL *) pLLChannel;
   VMMC_DEVICE  *pDev = pCh->pParent;
   IFX_int32_t  ret = VMMC_statusOk;

   /* Work only on channels where COD is initialised */
   if (pCh->pCOD != IFX_NULL)
   {
      COD_CHAN_SPEECH_t *pCodCh = &pCh->pCOD->fw_cod_ch_speech;

      /* range check, because gain var is integer */
      if ((pVol->nEnc < VMMC_VOLUME_GAIN_MIN) ||
          (pVol->nEnc > VMMC_VOLUME_GAIN_MAX) ||
          (pVol->nDec < VMMC_VOLUME_GAIN_MIN) ||
          (pVol->nDec > VMMC_VOLUME_GAIN_MAX))
      {
         /* parameter out of supported range */
         RETURN_STATUS (VMMC_statusFuncParm);
      }

      /* protect fw msg */
      VMMC_OS_MutexGet (&pCh->chAcc);

      /* store actual settings into message cache */
      pCodCh->GAIN1 = VMMC_Gaintable[pVol->nDec + (-VMMC_VOLUME_GAIN_MIN)];
      pCodCh->GAIN2 = VMMC_Gaintable[pVol->nEnc + (-VMMC_VOLUME_GAIN_MIN)];

      /* if channel is enabled write to fw */
      if (pCodCh->EN == COD_CHAN_SPEECH_ENABLE)
      {
         /* channel is enabled, write the updated msg to the device */
         ret = CmdWrite (pDev, (IFX_uint32_t *)pCodCh,
                         sizeof(COD_CHAN_SPEECH_t) - CMD_HDR_CNT);
      }

      /* release lock */
      VMMC_OS_MutexRelease (&pCh->chAcc);
   }
   else
   {
      /* errmsg: The requested resource is not available. */
      RETURN_STATUS (VMMC_statusNoRes);
   }

   return ret;
}


/**
   Configures the jitter buffer

   \param  pLLChannel   Pointer to the VMMC channel structure.
   \param  pJbConf      Pointer to IFX_TAPI_JB_CFG_t structure.

   \return
   - VMMC_statusInvalCh No coder resource on the given channel.
   - VMMC_statusCmdWr Writing the command failed
   - VMMC_statusOk if successful
*/
IFX_int32_t VMMC_TAPI_LL_COD_JB_Cfg (IFX_TAPI_LL_CH_t *pLLChannel,
                                     IFX_TAPI_JB_CFG_t const *pJbConf)
{
   VMMC_CHANNEL  *pCh = (VMMC_CHANNEL *) pLLChannel;
   VMMC_DEVICE  *pDev = pCh->pParent;
   COD_JB_CONF_t *pCodJbConf;
   IFX_int32_t    ret;

   /* abort if channel has no COD resource */
   if (pCh->pCOD == IFX_NULL)
   {
      /* errmsg: Resource not valid. Channel number out of range  */
      RETURN_STATUS (VMMC_statusInvalCh);
   }

   /* get a local handle for the fw message */
   pCodJbConf = &pCh->pCOD->fw_cod_jb_conf;

   /* protect fw msg */
   VMMC_OS_MutexGet (&pCh->chAcc);

   /* do JB settings */
   switch (pJbConf->nJbType)
   {
      case IFX_TAPI_JB_TYPE_FIXED:
         pCodJbConf->ADAP = COD_JB_CONF_ADAP_FIXED;
         break;
      case IFX_TAPI_JB_TYPE_ADAPTIVE:
         /* set adaptive type */
         pCodJbConf->ADAP = COD_JB_CONF_ADAP_EN;
         break;
      default:
         RETURN_STATUS (VMMC_statusParam);
   }

   switch (pJbConf->nPckAdpt)
   {
      case  IFX_TAPI_JB_PKT_ADAPT_VOICE:
         /* Packet adaption is optimized for voice. Reduced adjustment speed and
            packet repetition is off */
         /* ADAP = 1, ->see above PJE = 1 SF = default ->see above */
         pCodJbConf->PJE = COD_JB_CONF_PJE_ON;
         pCodJbConf->RAD = COD_JB_CONF_RAD_OFF;
         pCodJbConf->PRP = COD_JB_CONF_PRP_OFF;
         pCodJbConf->DVF = COD_JB_CONF_DVF_ON;
         pCodJbConf->MODE = COD_JB_CONF_MODE_NORMAL;
         pCodJbConf->SAD = COD_JB_CONF_SAD_ON;
         break;
      case  IFX_TAPI_JB_PKT_ADAPT_DATA:
         /* Packet adaption is optimized for data */
         pCodJbConf->PJE = COD_JB_CONF_PJE_OFF;
         pCodJbConf->RAD = COD_JB_CONF_RAD_ON;
         pCodJbConf->PRP = COD_JB_CONF_PRP_ON;
         pCodJbConf->DVF = COD_JB_CONF_DVF_OFF;
         pCodJbConf->MODE = COD_JB_CONF_MODE_RESET;
         pCodJbConf->SAD = COD_JB_CONF_SAD_OFF;
         break;
      case IFX_TAPI_JB_PKT_ADAPT_DATA_NO_REP:
         /** Packet adaption optimized for data but without doing packet
             repetition if packets should get lost. */
         pCodJbConf->PJE = COD_JB_CONF_PJE_OFF;
         pCodJbConf->RAD = COD_JB_CONF_RAD_ON;
         pCodJbConf->PRP = COD_JB_CONF_PRP_OFF;
         pCodJbConf->DVF = COD_JB_CONF_DVF_OFF;
         pCodJbConf->MODE = COD_JB_CONF_MODE_RESET;
         pCodJbConf->SAD = COD_JB_CONF_SAD_OFF;
         break;
      /* 0.1 are reserved - currently not supported */
      default:
         RETURN_STATUS (VMMC_statusParam);
   }

   /* Scaling factor */
   pCodJbConf->SF   = (IFX_uint8_t)pJbConf->nScaling;
   /* Initial Size of JB */
   pCodJbConf->INIT_JOB_POD = pJbConf->nInitialSize;
   /* minimum Size of JB */
   pCodJbConf->MIN_JB_POD   = pJbConf->nMinSize;
   /* maximum Size of JB  */
   pCodJbConf->MAX_JB_POD   = pJbConf->nMaxSize;
   /* non adaptive mode */
   pCodJbConf->NAM          = COD_JB_CONF_NAM_ADAPTIVE;

   /** Adaptation factor */
   pCodJbConf->ADAP_FACTOR = COD_JB_CONF_ADAP_FACTOR;
   /** Minimum margin */
   pCodJbConf->MIN_MARGIN = COD_JB_CONF_MIN_MARGIN;
   /** Jitter adaptation during silence */
   pCodJbConf->LOC =
      (pDev->caps.bJAS == 1 &&
         (pJbConf->nLocalAdpt == IFX_TAPI_JB_LOCAL_ADAPT_ON ||
          pJbConf->nLocalAdpt == IFX_TAPI_JB_LOCAL_ADAPT_DEFAULT)) ? 1 : 0;

   ret = CmdWrite (pDev, (IFX_uint32_t *) pCodJbConf,
                   pDev->caps.bEnhancedJB == 0 ?
                   COD_JB_CONF_LEN : COD_JB_CONF_ENH_LEN);

   /* release lock */
   VMMC_OS_MutexRelease (&pCh->chAcc);

   return ret;
}


/**
   Query jitter buffer statistics

   The implementation for this service should be aligned with RTCP statistics.
   Like the RTCP statistics they are prepared and signaled when ready.
   Otherwise this function would block too long ( > 10 ms).

   This function just ready previously requested jitter buffer statistics.

   When jitter buffer statistics arrive they are cached inside the channel
   structure. Then a flag is set, that new data has arrived.

   \param pLLChannel      Handle to VMMC channel structure
   \param pJbData         Handle to IFX_TAPI_JB_STATISTICS_t structure

   \return
      - VMMC_statusInvalCh No coder resource on the given channel.
      - VMMC_statusCodNotActiveOnJbRead  Coder Channel must be active to read
           JB statistics
      - VMMC_statusCmdWr Writing the command failed
      - VMMC_statusOk if successful

   \remarks The statistics can only be read while the coder is active.
*/
IFX_int32_t VMMC_TAPI_LL_COD_JB_Stat_Get (IFX_TAPI_LL_CH_t *pLLChannel,
                                          IFX_TAPI_JB_STATISTICS_t *pJbData)
{
   VMMC_CHANNEL  *pCh = (VMMC_CHANNEL *) pLLChannel;
   VMMC_DEVICE   *pDev = pCh->pParent;
   COD_JB_STAT_t *pJBStat;
   IFX_int32_t    ret;

   /* abort if channel has no COD resource */
   if (pCh->pCOD == IFX_NULL)
   {
      /* errmsg: Resource not valid. Channel number out of range  */
      RETURN_STATUS (VMMC_statusInvalCh);
   }

   /* The statistics can only be read while the coder is active. */
   if (pCh->pCOD->fw_cod_ch_speech.EN != COD_CHAN_SPEECH_ENABLE)
   {
      /* errmsg: Coder Channel must be active to read JB statistics */
      RETURN_STATUS (VMMC_statusCodNotActiveOnJbRead);
   }

   /* get type of configured jitter buffer */
   switch (pCh->pCOD->fw_cod_jb_conf.ADAP & COD_JB_CONF_ADAP_ADAPTIVE)
   {
      case COD_JB_CONF_ADAP_FIXED:
         /* jitter buffer type */
         pJbData->nType = IFX_TAPI_JB_TYPE_FIXED;
         break;
      case COD_JB_CONF_ADAP_EN:
      default:
         /* jitter buffer type */
         pJbData->nType = IFX_TAPI_JB_TYPE_ADAPTIVE;
         break;
   }

   /* read Coder Channel JB Statistics */
   pJBStat           = &pCh->pCOD->fw_cod_jb_stat;

   ret = CmdRead (pDev, (IFX_uint32_t *) pJBStat,
                  (IFX_uint32_t*) pJBStat, COD_JB_STAT_LEN);

   /* assign the JB statistics values to the corresponding members of the
      IFX_TAPI_JB_STATISTICS_t structure */
   if (VMMC_SUCCESS(ret))
   {
      /* incoming time, not supported anymore */
      pJbData->nInTime           = 0;
      /* Comfort Noise Generation, not supported anymore */
      pJbData->nCNG              = 0;
      /* Bad Frame Interpolation, not supported anymore */
      pJbData->nBFI              = 0;
      /* max packet delay, not supported anymore */
      pJbData->nMaxDelay         = 0;
      /* minimum packet delay, not supported anymore */
      pJbData->nMinDelay         = 0;
      /* network jitter value, not supported anymore */
      pJbData->nNwJitter         = 0;
      /* play out delay */
      pJbData->nPODelay          = pJBStat->PACKET_POD;
      /* max play out delay */
      pJbData->nMaxPODelay       = pJBStat->MAX_PACKET_POD;
      /* min play out delay */
      pJbData->nMinPODelay       = pJBStat->MIN_PACKET_POD;
      /* current jitter buffer size */
      pJbData->nBufSize          = pJBStat->JITTER;
      pJbData->nMaxBufSize       = pJBStat->MAX_JITTER;
      pJbData->nMinBufSize       = pJBStat->MIN_JITTER;
      /* received packet number  */
      pJbData->nPackets          = pJBStat->PACKETS;
      /* lost packets number, not supported anymore */
      pJbData->nLost             = 0;
      /* invalid packet number (discarded) */
      pJbData->nInvalid          = pJBStat->DISCARDED_PACKETS;
      /* duplication packet number, not supported anymore */
      pJbData->nDuplicate        = pJBStat->DISCARDED_PACKETS;
      /* late packets number */
      pJbData->nLate             = pJBStat->LATE_PACKETS;
      /* early packets number */
      pJbData->nEarly            = pJBStat->EARLY_PACKETS;
      /* resynchronizations number */
      pJbData->nResync           = pJBStat->RESNC;
      /* new support */
      pJbData->nIsUnderflow      = pJBStat->IS_UNDERFLOW;
      pJbData->nIsNoUnderflow    = pJBStat->IS_NO_UNDERFLOW;
      pJbData->nIsIncrement      = pJBStat->IS_INCREMENT;
      pJbData->nSkDecrement      = pJBStat->SK_DECREMENT;
      pJbData->nDsDecrement      = pJBStat->DS_DECREMENT;
      pJbData->nDsOverflow       = pJBStat->DS_OVERFLOW;
      pJbData->nSid              = pJBStat->SID_HW;

      /* Because the firmware does not provide the high DWORD part
         the driver has to handle the overflow. This is done by comparing the
         new and the old value. If the new value is smaller, there is
         an overflow and high is increased */
      if (pJBStat->RECEIVED_BYTES < pCh->pCOD->nRecBytesL)
      {
         pCh->pCOD->nRecBytesH++;
      }
      pCh->pCOD->nRecBytesL = pJBStat->RECEIVED_BYTES;

      /* Set the absolute values in the struct with the results. */
      pJbData->nRecBytesL = pCh->pCOD->nRecBytesL;
      pJbData->nRecBytesH = pCh->pCOD->nRecBytesH;
   }

   if (VMMC_SUCCESS(ret))
   {
      /* Add the statistics of the packets generated in the SIG module. */
      IFX_uint32_t ReceivedBytesLow;
      IFX_uint32_t ReceivedBytesHigh;

      /* Get the absolute values from the SIG module. */
      ret = VMMC_SIG_Event_Stat_Get (pCh,
                                     &ReceivedBytesLow, &ReceivedBytesHigh);

      if (VMMC_SUCCESS(ret))
      {
         IFX_uint32_t OldCounter = pJbData->nRecBytesL;

         /* Add the absolute values to the struct with the results. */
         pJbData->nRecBytesL += ReceivedBytesLow;
         pJbData->nRecBytesH += ReceivedBytesHigh;

         /* carry on an overflow of the low-word */
         if (pJbData->nRecBytesL < OldCounter)
         {
            pJbData->nRecBytesH++;
         }
      }
   }

#ifdef VMMC_FEAT_RTCP_XR
   if (VMMC_SUCCESS(ret) &&
       (pDev->caps.bRtcpXR == 1) && (pDev->caps.bXR_BN == 1))
   {
      /* The burst_number is appended to the RTCP XR VoIP metrics report. */

      /* protect fwmsg access against concurrent tasks */
      VMMC_OS_MutexGet (&pCh->chAcc);

      ret = CmdRead (pDev,
                     (IFX_uint32_t *) &pCh->pCOD->fw_cod_rtcp_xr_voip_met,
                     (IFX_uint32_t *) &pCh->pCOD->fw_cod_rtcp_xr_voip_met,
                     COD_RTCP_XR_VOIP_MET_EXT_LEN);
      if (VMMC_SUCCESS(ret))
      {
         pJbData->nBurstNumber =
            pCh->pCOD->fw_cod_rtcp_xr_voip_met.burst_number;
      }

      /* release protection */
      VMMC_OS_MutexRelease (&pCh->chAcc);
   }
#endif /* VMMC_FEAT_RTCP_XR */

   return ret;
}


/**
   Resets jitter buffer statistics

   Sends the firmware message Coder Channel JB Statistics with length zero
   to reset the jitter buffer.

   \param  pLLChannel   Pointer to the VMMC channel structure.

   \return
      - VMMC_statusInvalCh No coder resource on the given channel.
      - VMMC_statusCodNotActiveOnJbReset  Coder Channel must be active to reset
           JB statistics
      - VMMC_statusCmdWr Writing the command failed
      - VMMC_statusOk if successful

   \remarks The statistics can only be reset while the coder is active.
*/
IFX_int32_t VMMC_TAPI_LL_COD_JB_Stat_Reset (IFX_TAPI_LL_CH_t *pLLChannel)
{
   VMMC_CHANNEL *pCh = (VMMC_CHANNEL *) pLLChannel;
   VMMC_DEVICE  *pDev = pCh->pParent;
   IFX_int32_t   ret;

   /* abort if channel has no COD resource */
   if (pCh->pCOD == IFX_NULL)
   {
      /* errmsg: Resource not valid. Channel number out of range  */
      RETURN_STATUS (VMMC_statusInvalCh);
   }

   /* The statistics can only be reset while the coder is active. */
   if (pCh->pCOD->fw_cod_ch_speech.EN != COD_CHAN_SPEECH_ENABLE)
   {
      /* errmsg: Coder Channel must be active to reset JB statistics */
      RETURN_STATUS (VMMC_statusCodNotActiveOnJbReset);
   }

   /* reset JB Statistics by sending a Coder Channel JB Statistics write
      command of length 0 */
   ret = CmdWrite (pDev, (IFX_uint32_t *) &pCh->pCOD->fw_cod_jb_stat, 0);

   if (VMMC_SUCCESS(ret))
   {
      /* Reset the cached absolute values of the received bytes */
      pCh->pCOD->nRecBytesL = pCh->pCOD->nRecBytesH = 0L;

      /* Reset also the statistic values in the SIG channel */
      VMMC_SIG_Event_Stat_Reset(pCh);
   }

   RETURN_STATUS (ret);
}


/**
   Configure RTP for a new connection

   A check on the availability is done for coder and signaling channels.
   If the corresponding resource is not available no configurtation is done.
   For the coder channel this function configures the SSRC, sequence nr and
   the timestamp. For the signaling channel is configures SSRC, OOB signalling
   and the event payload type.

   If the coder is running and the mode is to be switched to out of band,
   the OOB setting is applied directely. Otherwise it is just stored
   and applied when the encoder or decoder is switched on.
   No message will be send and no error returned, when the setting has
   not been changed. The driver checks for the cached values.

   \param  pLLChannel   Pointer to the VMMC channel structure.
   \param  pRtpConf     Pointer to IFX_TAPI_PKT_RTP_CFG_t structure.

   \return
   - VMMC_statusCmdWr Writing the command failed
   - VMMC_statusFuncParm Wrong parameters passed. This code is returned
     when OOB mode is invalid.
   - VMMC_statusWrongEvPT If OOB event transmission should be enabled, the
      EventPT must be != 0
   - VMMC_statusOk if successful
*/
IFX_int32_t VMMC_TAPI_LL_COD_RTP_Cfg (IFX_TAPI_LL_CH_t *pLLChannel,
                                      IFX_TAPI_PKT_RTP_CFG_t const *pRtpConf)
{
   VMMC_CHANNEL *pCh = (VMMC_CHANNEL *) pLLChannel;
   IFX_int32_t ret = VMMC_statusOk;

   if (pCh->pCOD != IFX_NULL)
   {
      /* set SSRC, Sequence Nr and Timestamp for coder channel */
      ret = vmmc_cod_RTP_Cfg (pCh, pRtpConf);
   }

   /* RTP events can only be generated by a SIG module. But it requries a COD
      channel to transport them. So both modules must exist on this channel. */
   if (VMMC_SUCCESS(ret) && (pCh->pSIG != IFX_NULL) && (pCh->pCOD != IFX_NULL))
   {
      /* set SSRC, PTs for event playout/reception and Event generation trigger
         for the signalling channel */
      ret = VMMC_SIG_RTP_OOB_Cfg (pCh, pRtpConf);
   }

   RETURN_STATUS (ret);
}


/**
   Configure a new payload type table

   Unused payload type entries (PT17, PT23, PT24 , PT25 and PT30)  are set
   to 0x7F (reset value for firmware message). In this case the SID
   setting is not applied.
   The SID packets are configured to use the same payload type except
   for u-Law and a-Law.

   \param  pLLChannel   Pointer to the VMMC channel structure.
   \param  pRtpPTConf   Pointer to IFX_TAPI_PKT_RTP_PT_CFG_t structure.

   \return
   - VMMC_statusInvalCh No coder resource on the given channel.
   - VMMC_statusCmdWr Writing the command failed
   - VMMC_statusOk if successful
*/
IFX_int32_t VMMC_TAPI_LL_COD_RTP_PayloadTable_Cfg (IFX_TAPI_LL_CH_t *pLLChannel,
                                                   IFX_TAPI_PKT_RTP_PT_CFG_t const *pRtpPTConf)
{
   IFX_int32_t ret = VMMC_statusOk;
   VMMC_CHANNEL *pCh = (VMMC_CHANNEL *)pLLChannel;

   if (pCh->pCOD != IFX_NULL)
   {
      /* Set payload types in upstream direction */
      ret = vmmc_cod_RTP_PayloadTableSet (pCh, IFX_TRUE,
                                          (IFX_uint8_t*)pRtpPTConf->nPTup);

      /* Set payload types in downstream direction */
      if (VMMC_SUCCESS(ret))
      {
         ret = vmmc_cod_RTP_PayloadTableSet (pCh, IFX_FALSE,
                                             (IFX_uint8_t*)pRtpPTConf->nPTdown);
      }
   }
   else
   {
      /* errmsg: Resource not valid. Channel number out of range  */
      RETURN_STATUS (VMMC_statusInvalCh);
   }

   return ret;
}


/**
   Start or stop generation of RTP event packets

   \param  pLLChannel   Pointer to the VMMC channel structure.
   \param  nEvent       Event code as defined in RFC2833
   \param  bStart       Start (true) or stop (false)
   \param  nDuration    Duration of event in units of 10 ms (0 = forever)
   \param  nVolume      Volume of event. Value range [0-63] corresponding
                        to 0 dB to -63 dB. Values outside of the allowed
                        range will activate firmware defaults.
   \return
   - VMMC_statusCmdWr Writing the command failed
   - VMMC_statusOk if successful

   \remarks
   Even though the FW message belongs to the SIG module it is sent from here
   because the RTP generation is originally a function of the COD module.
*/
IFX_int32_t VMMC_TAPI_LL_COD_RTP_EventGenerate (IFX_TAPI_LL_CH_t *pLLChannel,
                                                IFX_uint8_t nEvent,
                                                IFX_boolean_t bStart,
                                                IFX_uint8_t nDuration,
                                                IFX_int8_t nVolume)
{
   VMMC_CHANNEL  *pCh  = (VMMC_CHANNEL *) pLLChannel;
   VMMC_DEVICE   *pDev = pCh->pParent;
   IFX_int32_t   ret   = IFX_ERROR;
   COD_EVT_GEN_t *pCodEvGen;

   /* abort if channel has no COD resource */
   if (pCh->pCOD == IFX_NULL)
   {
      /* errmsg: Resource not valid. Channel number out of range  */
      RETURN_STATUS (VMMC_statusInvalCh);
   }

   /* ARGSUSED */
   if( pCh->pSIG->et_stat.value == 0xFFFF )
   {
      /* protect fwmsg access against concurrent tasks */
      VMMC_OS_MutexGet (&pCh->chAcc);

      pCodEvGen = &pCh->pCOD->fw_cod_evt_gen;

      /* valid values for duration are 0x05 - 0xFF (50ms - 2550ms) */
      if ( (nDuration != 0) && (nDuration < 0x05) )
      {
         TRACE (VMMC, DBG_LEVEL_HIGH,
             ("\nDRV_ERROR: Duration for event generation "
              "increased to 50ms minimum\n"));
         nDuration = 0x05;
      }

      if ( (nVolume >= 0) && (nVolume <= 63) )
      {
         /* volume value within valid range -> set to message */
         pCodEvGen->UV  = 1;
         pCodEvGen->VOL = (IFX_uint8_t)nVolume;
      }
      else
      {
         /* volume value out of valid range -> use FW defaults */
         pCodEvGen->UV  = 0;
         pCodEvGen->VOL = 0;
      }

      pCodEvGen->EVENT = nEvent;
      pCodEvGen->ED    = nDuration & 0xFF;
      pCodEvGen->ST    = (bStart == IFX_TRUE) ? 1 : 0;
      ret              = CmdWrite(pDev, (IFX_uint32_t *)pCodEvGen,
                                  sizeof(COD_EVT_GEN_t)- CMD_HDR_CNT);

      VMMC_OS_MutexRelease (&pCh->chAcc);
   }

   return ret;
}


/**
   Gets the RTCP statistic information for the addressed channel.

   RTCP statistics for the specified channel are not reset after the completion
   of the read request.

   \param  pLLChannel   Pointer to the VMMC channel structure.
   \param  pRTCP        Pointer to IFX_TAPI_PKT_RTCP_STATISTICS_t structure.

   \return
      - VMMC_statusInvalCh No coder resource on the given channel.
      - VMMC_statusCodNotActiveOnRtcpRead  Coder Channel must be active to read
           RTCP statistics.
      - VMMC_statusParam Return parameter may not be NULL.
      - VMMC_statusCmdWr Writing the command failed
      - VMMC_statusOk if successful

   \remarks The statistics can only be read while the coder is active.
*/
IFX_int32_t VMMC_TAPI_LL_COD_RTCP_Get (IFX_TAPI_LL_CH_t *pLLChannel,
                                       IFX_TAPI_PKT_RTCP_STATISTICS_t *pRTCP)
{
   VMMC_CHANNEL *pCh  = (VMMC_CHANNEL *) pLLChannel;
   VMMC_DEVICE  *pDev = pCh->pParent;
   COD_RTCP_SUP_CTRL_t *p_fw_cod_rtcp_stat;
   IFX_int32_t ret;

   /* abort if channel has no COD resource */
   if (pCh->pCOD == IFX_NULL)
   {
      /* errmsg: Resource not valid. Channel number out of range  */
      RETURN_STATUS (VMMC_statusInvalCh);
   }

   /* The statistics can only be read while the coder is active. */
   if (pCh->pCOD->fw_cod_ch_speech.EN != COD_CHAN_SPEECH_ENABLE)
   {
      /* errmsg: Coder Channel must be active to read RTCP statistics */
      RETURN_STATUS (VMMC_statusCodNotActiveOnRtcpRead);
   }

   if (pRTCP == IFX_NULL)
   {
      /* Return parameter may not be NULL. */
      RETURN_STATUS (VMMC_statusParam);
   }

   /* protect fwmsg access against concurrent tasks */
   VMMC_OS_MutexGet (&pCh->chAcc);

   /* Get pointer to fw message now that we know that the struct exists. */
   p_fw_cod_rtcp_stat = &pCh->pCOD->fw_cod_rtcp_stat;

   /* Read Coder Channel Statistics */
   ret = CmdRead (pDev, (IFX_uint32_t *) p_fw_cod_rtcp_stat,
                        (IFX_uint32_t *) p_fw_cod_rtcp_stat,
                        COD_RTCP_SUP_CTRL_LEN);

   if (VMMC_SUCCESS(ret))
   {
      /* Clear output structure only if we are going to fill it with results. */
      memset(pRTCP, 0, sizeof(*pRTCP));

      /* assign the RTCP values to the corresponding members of the
         IFX_TAPI_PKT_RTCP_STATISTICS_t structure */
      pRTCP->rtp_ts        = p_fw_cod_rtcp_stat->TIME_STAMP;
      pRTCP->psent         = p_fw_cod_rtcp_stat->S_PKT_CNT;
      pRTCP->osent         = p_fw_cod_rtcp_stat->S_OCT_CNT;
      pRTCP->rssrc         = p_fw_cod_rtcp_stat->R_SSRC;
      pRTCP->fraction      = p_fw_cod_rtcp_stat->R_FRACT_LOST;
      pRTCP->lost          = (IFX_int32_t)
                                ((p_fw_cod_rtcp_stat->R_PKTS_LOST_HB << 16) |
                                  p_fw_cod_rtcp_stat->R_PKTS_LOST_LW);
      pRTCP->last_seq      = p_fw_cod_rtcp_stat->R_EXT_HSNR;
      pRTCP->jitter        = p_fw_cod_rtcp_stat->R_INT_JIT;
      /* return stored SSRC */
      pRTCP->ssrc          = pCh->pCOD->nSsrc;
      /* The calling control task will set the parameters lsr and dlsr */
   }

   /* release protection */
   VMMC_OS_MutexRelease (&pCh->chAcc);

   RETURN_STATUS (ret);
}


/**
   Resets  RTCP statistics

   Sends the firmware message Coder Channel RTCP Statistics with length zero
   to reset the RTCP statistics.

   \param  pLLChannel   Pointer to the VMMC channel structure.

   \return
      - VMMC_statusInvalCh No coder resource on the given channel.
      - VMMC_statusCodNotActiveOnRtcpReset  Coder Channel must be active to
           reset RTCP statistics
      - VMMC_statusCmdWr Writing the command failed
      - VMMC_statusOk if successful
*/
IFX_int32_t VMMC_TAPI_LL_COD_RTCP_Reset (IFX_TAPI_LL_CH_t *pLLChannel)
{
   VMMC_CHANNEL *pCh  = (VMMC_CHANNEL *)pLLChannel;
   VMMC_DEVICE  *pDev = pCh->pParent;

   /* abort if channel has no COD resource */
   if (pCh->pCOD == IFX_NULL)
   {
      /* errmsg: Resource not valid. Channel number out of range  */
      RETURN_STATUS (VMMC_statusInvalCh);
   }

   /* The statistics can only be reset while the coder is active. */
   if (pCh->pCOD->fw_cod_ch_speech.EN != COD_CHAN_SPEECH_ENABLE)
   {
      /* errmsg: Coder Channel must be active to reset RTCP statistics */
      RETURN_STATUS (VMMC_statusCodNotActiveOnRtcpReset);
   }

   /* For reset we write a length of 0. */
   return CmdWrite (pDev, (IFX_uint32_t *)&pCh->pCOD->fw_cod_rtcp_stat, 0);
}

#ifdef VMMC_FEAT_RTCP_XR
/**
   Gets the block of RTCP XR statistic information for the addressed channel.

   \param  pLLChannel   Pointer to the VMMC channel structure.
   \param  pRTCP_XR     Pointer to IFX_TAPI_PKT_RTCP_XR_BLOCK_GET_t structure.

   \return
      - VMMC_statusInvalCh No coder resource on the given channel.
      - VMMC_statusCodNotActiveOnRtcpRead  Coder Channel must be active to read
           RTCP statistics.
      - VMMC_statusParam Return parameter may not be NULL
           or invalid block type number.
      - return value of  CmdRead() if CmdRead() call fails
      - VMMC_statusOk if successful

   \remarks The statistics block can only be read while the coder is active.
*/
IFX_int32_t VMMC_TAPI_LL_COD_RTCP_XR_Get (IFX_TAPI_LL_CH_t *pLLChannel,
                                          IFX_TAPI_PKT_RTCP_XR_BLOCK_GET_t *pRTCP_XR)
{
   VMMC_CHANNEL *pCh  = (VMMC_CHANNEL *) pLLChannel;
   VMMC_DEVICE  *pDev = pCh->pParent;
   IFX_int32_t ret = VMMC_statusParam;

   /* check if feature is supported */
   if (!pDev->caps.bRtcpXR)
   {
      RETURN_STATUS (VMMC_statusNotSupported);
   }
   /* abort if channel has no COD resource */
   if (pCh->pCOD == IFX_NULL)
   {
      /* errmsg: Resource not valid. Channel number out of range  */
      RETURN_STATUS (VMMC_statusInvalCh);
   }

   /* The statistics can only be read while the coder is active. */
   if (pCh->pCOD->fw_cod_ch_speech.EN != COD_CHAN_SPEECH_ENABLE)
   {
      /* errmsg: Coder Channel must be active to read RTCP statistics */
      RETURN_STATUS (VMMC_statusCodNotActiveOnRtcpRead);
   }

   if (pRTCP_XR == IFX_NULL)
   {
      /* Return parameter may not be NULL. */
      RETURN_STATUS (VMMC_statusParam);
   }

   /* protect fwmsg access against concurrent tasks */
   VMMC_OS_MutexGet (&pCh->chAcc);

   switch(pRTCP_XR->nBlockType)
   {
      case IFX_TAPI_PKT_RTCP_XR_BLOCK_TYPE_STATISTICS_SUMMARY_REPORT:
         /* Read Coder Channel Statistics */
         ret = CmdRead (pDev,
                        (IFX_uint32_t *) &pCh->pCOD->fw_cod_rtcp_xr_stat_sum,
                        (IFX_uint32_t *) &pCh->pCOD->fw_cod_rtcp_xr_stat_sum,
                        COD_RTCP_XR_STAT_SUM_LEN);
         if (VMMC_SUCCESS(ret))
         { /* copy data block */
            pRTCP_XR->sBlockData.StatisticsSummaryReport =
               pCh->pCOD->fw_cod_rtcp_xr_stat_sum.StatisticsSummaryReport;
         }
         break;
      case IFX_TAPI_PKT_RTCP_XR_BLOCK_TYPE_VOIP_METRICS_REPORT:
         /* Read Coder Channel Statistics */
         /* In order to get the metric field regarding echo return loss (RERL)
            the controller has to tell the FW which, if any, LEC resource (LECNR)
            is assigned to which coder channel. This is done via the Associated
            LEC Resource Number command. */
         ret = CmdRead (pDev,
                        (IFX_uint32_t *) &pCh->pCOD->fw_cod_rtcp_xr_voip_met,
                        (IFX_uint32_t *) &pCh->pCOD->fw_cod_rtcp_xr_voip_met,
                        COD_RTCP_XR_VOIP_MET_LEN);
         if (VMMC_SUCCESS(ret))
         { /* copy data block */
            pRTCP_XR->sBlockData.VoIPMetricsReport =
               pCh->pCOD->fw_cod_rtcp_xr_voip_met.VoIPMetricsReport;
         }
         break;
      default:
         /* Invalid/unhandled block type number. */
         ret = VMMC_statusParam;
         break;
   }
   /* release protection */
   VMMC_OS_MutexRelease (&pCh->chAcc);

   RETURN_STATUS (ret);
}
#endif /* VMMC_FEAT_RTCP_XR */

/**
   Callback function used by the TAPI event dispatcher in case of a
   decoder change event to retrieve details (decoder type and packetisation
   time) of the new decoder.

   The call of the function is important as reading of the decoder details
   acknowledges the decoder change interrupt in firmware, i.e. no further
   decoder change will be reported.

   \param  pLLCh        Pointer to the VMMC channel structure.
   \param  pDec         Pointer to a structure to retrieve the decoder details.

   \return IFX_SUCCESS or IFX_ERROR
*/
static IFX_int32_t VMMC_TAPI_LL_COD_DEC_ChgDetailReq (IFX_TAPI_LL_CH_t *pLLCh,
                                                      IFX_TAPI_DEC_DETAILS_t *pDec)
{
   VMMC_CHANNEL *pCh = (VMMC_CHANNEL *) pLLCh;
   IFX_int32_t ret;

   VMMC_OS_MutexGet (&pCh->chAcc);

   ret = CmdRead (pCh->pParent,
                  (IFX_uint32_t *) &pCh->pCOD->fw_cod_dec_stat,
                  (IFX_uint32_t *) &pCh->pCOD->fw_cod_dec_stat, 4);

   if (VMMC_SUCCESS(ret))
   {
      pDec->dec_type =
         VMMC_COD_trans_cod_fw2tapi(pCh->pCOD->fw_cod_dec_stat.DEC,
                                    pCh->pParent->caps.bAMRE);
      pDec->dec_framelength =
         vmmc_cod_trans_fl_fw2tapi(pCh->pCOD->fw_cod_dec_stat.PTD);

      TRACE(VMMC, DBG_LEVEL_LOW,
            (" decoder change -> decoder=%d, frame length=%d\n",
             pCh->pCOD->fw_cod_dec_stat.DEC, pCh->pCOD->fw_cod_dec_stat.PTD));

      /* store the current decoder so that we can check it for wideband at
         any time */
      pCh->pCOD->curr_dec = pCh->pCOD->fw_cod_dec_stat.DEC;

      VMMC_OS_MutexRelease (&pCh->chAcc);

      /* Update the wideband status */
      vmmc_cod_WidebandCodecCheck(pCh);
      /* Reevaluate the conference that this module belongs to */
      VMMC_CON_MatchConfSmplRate(pCh, VMMCDSP_MT_COD);
   }
   else
   {
      VMMC_OS_MutexRelease (&pCh->chAcc);
   }

   return ret;
}


/**
   Configures the MOS calculation.

   \param  pLLCh        Pointer to the VMMC channel structure.
   \param  pMosCfg      Pointer to the IFX_TAPI_COD_MOS_CFG_t structure.

   \return
   - VMMC_statusOk if successful
   - VMMC_statusNoRes if called on a channel where there is no COD resource
   - VMMC_statusNotSupported Feature not supported by FW
   - VMMC_statusCodActiveMosCfgFailed MOS configuration failed because
         coder channel is active
*/
static IFX_int32_t VMMC_TAPI_LL_COD_MOS_Cfg (
                     IFX_TAPI_LL_CH_t *pLLCh,
                     IFX_TAPI_COD_MOS_t const *pMosCfg)
{
   VMMC_CHANNEL *pCh  = (VMMC_CHANNEL *) pLLCh;
   VMMC_DEVICE  *pDev = pCh->pParent;
   IFX_int32_t  ret = VMMC_statusOk;

   /* Does the FW support the feature? */
   if (pDev->caps.bEMOS == 0)
   {
      /* errmsg: Feature or combination not supported */
      RETURN_STATUS (VMMC_statusNotSupported);
   }

   /* Work only on channels where COD is initialised */
   if (pCh->pCOD == IFX_NULL)
   {
      /* errmsg: The requested resource is not available. */
      RETURN_STATUS (VMMC_statusNoRes);
   }

   /* protect fw msg access */
   VMMC_OS_MutexGet (&pCh->chAcc);

   if (pCh->pCOD->fw_cod_ch_speech.EN == COD_CHAN_SPEECH_DISABLE)
   {
      CMD_COD_CFG_STAT_MOS_t *pCodChMOS_Cfg = &pCh->pCOD->fw_cod_cfg_stat_mos;

      pCodChMOS_Cfg->R_DEF      = pMosCfg->mos.nR;
      pCodChMOS_Cfg->CTI        = pMosCfg->mos.nCTI;
      pCodChMOS_Cfg->A_FACT     = pMosCfg->mos.nAdvantage;
      /* FW uses for MOS_CQE a basis of 1/256. The TAPI API uses a basis
         of 1/10. Convert the values here. */
      pCodChMOS_Cfg->MOS_CQE_TH = (pMosCfg->mos.nMOS * 256) / 10;

      ret = CmdWrite (pDev, (IFX_uint32_t *)pCodChMOS_Cfg,
                      CMD_COD_CFG_STAT_MOS_LEN);
   }
   else
   {
      /* errmsg: MOS configuration failed because coder channel is active */
      ret = VMMC_statusCodActiveMosCfgFailed;
   }

   /* release lock */
   VMMC_OS_MutexRelease (&pCh->chAcc);

   RETURN_STATUS (ret);
}


/**
   Reads out the current MOS value.

   \param  pLLCh        Pointer to the VMMC channel structure.
   \param  pMos         Pointer to the IFX_TAPI_COD_MOS_t structure.

   \return
   - VMMC_statusOk if successful
   - VMMC_statusNoRes if called on a channel where there is no COD resource
   - VMMC_statusNotSupported Feature not supported by FW
   - VMMC_statusCodMosGetFailedCodNotActive  MOS result get failed because
        coder channel is not active
*/
static IFX_int32_t VMMC_TAPI_LL_COD_MOS_Result_Get (
                     IFX_TAPI_LL_CH_t *pLLCh,
                     IFX_TAPI_COD_MOS_t *pMos)
{
   VMMC_CHANNEL *pCh  = (VMMC_CHANNEL *) pLLCh;
   VMMC_DEVICE  *pDev = pCh->pParent;
   IFX_int32_t  ret = VMMC_statusOk;

   /* Initalise the result field with 0. */
   memset(&pMos->mos, 0, sizeof(pMos->mos));

   /* Does the FW support the feature? */
   if (pDev->caps.bEMOS == 0)
   {
      /* errmsg: Feature or combination not supported */
      RETURN_STATUS (VMMC_statusNotSupported);
   }

   /* Work only on channels where COD is initialised */
   if (pCh->pCOD == IFX_NULL)
   {
      /* errmsg: The requested resource is not available. */
      RETURN_STATUS (VMMC_statusNoRes);
   }

   if ((pCh->pCOD->fw_cod_ch_speech.EN != COD_CHAN_SPEECH_DISABLE) &&
       (pCh->pCOD->fw_cod_ch_speech.DEC != COD_CHAN_SPEECH_DEC_INACTIVE))
   {
      CMD_COD_CFG_STAT_MOS_t *pCodChMOS_Cfg;
      CMD_COD_READ_STAT_MOS_t *pCodChMOS_Status;

      pCodChMOS_Cfg = &pCh->pCOD->fw_cod_cfg_stat_mos;
      pCodChMOS_Status = &pCh->pCOD->fw_cod_read_stat_mos;

      /* protect fw msg access */
      VMMC_OS_MutexGet (&pCh->chAcc);

      ret = CmdRead (pDev,
                     (IFX_uint32_t *)pCodChMOS_Status,
                     (IFX_uint32_t *)pCodChMOS_Status,
                     CMD_COD_READ_STAT_MOS_LEN);
      if (VMMC_SUCCESS(ret))
      {
         pMos->mos.nR         = pCodChMOS_Cfg->R_DEF;
         pMos->mos.nCTI       = pCodChMOS_Cfg->CTI;
         pMos->mos.nAdvantage = pCodChMOS_Cfg->A_FACT;
         /* FW uses for MOS_CQE a basis of 1/256. The TAPI API uses a basis
            of 1/10. Convert the values here. */
         pMos->mos.nMOS       = (pCodChMOS_Status->MOS_CQE * 10) / 256;
      }

      /* release lock */
      VMMC_OS_MutexRelease (&pCh->chAcc);
   }
   else
   {
      /* errmsg: MOS result get failed because decoder is not active */
      ret = VMMC_statusCodMosGetFailedDecNotActive;
   }

   RETURN_STATUS (ret);
}


#ifdef VMMC_FEAT_FAX_T38_FW
/**
   Gets the supported T.38 fax channel implementation capabilities
   \param pLLDev          Pointer to Low-level device structure
          pCap            Pointer to the IFX_TAPI_T38_CAP_t structure
   \return
      VMMC_statusOk if successful else device specific return code.
*/
IFX_int32_t VMMC_TAPI_LL_COD_FAX_Cap_Get(IFX_TAPI_LL_DEV_t *pLLDev,
                                             IFX_TAPI_T38_CAP_t *pCap)
{
   VMMC_DEVICE *pDev = (VMMC_DEVICE* )pLLDev;
   COD_FAX_READ_CAP_t Fax_Read_Cap, *pFWCap = &Fax_Read_Cap;
   IFX_int32_t ret;

   /* sanity check */
   if (!pDev->caps.bT38FW)
   {
      return VMMC_statusNotSupported;
   }
   memset (pFWCap, 0, sizeof(COD_FAX_READ_CAP_t));
   pFWCap->CMD       = CMD_EOP;
   pFWCap->MOD       = MOD_CODER;
   pFWCap->ECMD      = COD_FAX_READ_CAP_ECMD;
   ret = CmdRead (pDev, (IFX_uint32_t *) pFWCap,
                        (IFX_uint32_t *) pFWCap, COD_FAX_READ_CAP_LEN);
   if (ret == VMMC_statusOk)
   {
      memset (pCap, 0, sizeof(IFX_TAPI_T38_CAP_t));
      /** T.38 transport protocol. */
      switch (pFWCap->TPORT)
      {
         case COD_FAX_READ_CAP_TPORT_TCP:
            pCap->Protocol = IFX_TAPI_T38_TCP;
            break;
         case COD_FAX_READ_CAP_TPORT_UDP:
            pCap->Protocol = IFX_TAPI_T38_UDP;
            break;
         default:
            if(pFWCap->TPORT ==
               (COD_FAX_READ_CAP_TPORT_TCP | COD_FAX_READ_CAP_TPORT_UDP))
            {
               pCap->Protocol = IFX_TAPI_T38_TCP_UDP;
            }
            break;
      }
      /** UDP error correction method. */
      switch (pFWCap->UDPEC)
      {
         case COD_FAX_READ_CAP_UDPEC_RED:
            pCap->UDPErrCorr = IFX_TAPI_T38_CAP_RED;
            break;
         case COD_FAX_READ_CAP_UDPEC_FEC:
            pCap->UDPErrCorr = IFX_TAPI_T38_CAP_FEC;
            break;
         default:
            if (pFWCap->UDPEC ==
                (COD_FAX_READ_CAP_UDPEC_RED | COD_FAX_READ_CAP_UDPEC_FEC))
            {
               pCap->UDPErrCorr = IFX_TAPI_T38_CAP_RED_FEC;
            }
            break;
      }
      /** T.38 UDP rate management method. */
      switch (pFWCap->UDPRMM)
      {
         case COD_FAX_READ_CAP_RMM_LOC_TCF:
            pCap->nUDPRateManagement = IFX_TAPI_T38_LOC_TCF;
            break;
         case COD_FAX_READ_CAP_RMM_TRANS_TCF:
            pCap->nUDPRateManagement = IFX_TAPI_T38_TRANS_TCF;
            break;
         default:
            if (pFWCap->UDPRMM ==
                (COD_FAX_READ_CAP_RMM_LOC_TCF | COD_FAX_READ_CAP_RMM_TRANS_TCF))
            {
               pCap->nUDPRateManagement = IFX_TAPI_T38_LOC_TRANS_TCF;
            }
            break;
      }
      /** Bit options for T.38 */
      pCap->FacConvOpt = (IFX_TAPI_T38_FACSIMILE_CNVT_t)0;
      if (pFWCap->BITOPT & COD_FAX_READ_CAP_BITOPT_FBMR)
         pCap->FacConvOpt |= IFX_TAPI_T38_HFBMR;
      if (pFWCap->BITOPT & COD_FAX_READ_CAP_BITOPT_TMMR)
         pCap->FacConvOpt |= IFX_TAPI_T38_HTMMR;
      if (pFWCap->BITOPT & COD_FAX_READ_CAP_BITOPT_TJBIG)
         pCap->FacConvOpt |= IFX_TAPI_T38_TJBIG;
      /** Maximum bit rate used by FAX data pump
         (kbit/s)*10 convert to bit/s */
      pCap->nBitRateMax = (IFX_uint32_t)pFWCap->MAXBITRATE * 100;
      /** UDP maximum buffer size */
      pCap->nUDPBuffSizeMax = (IFX_uint16_t)pFWCap->UDPMBS;
      /** UDP maximum datagram size */
      pCap->nUDPDatagramSizeMax = (IFX_uint16_t)pFWCap->UDPMDS;
      /** T.38 ASN.1 version */
      pCap->nT38Ver = (IFX_uint8_t)pFWCap->T38VER;
   }
   return ret;
}

/**
   Gets the parameters of T.38 fax channel
   \param pLLCh           Pointer to Low-level channel structure
          pCfg            Pointer to the IFX_TAPI_T38_FAX_CFG_t structure
   \return
      VMMC_statusOk if successful else device specific return code.
*/
IFX_int32_t VMMC_TAPI_LL_COD_FAX_Cfg_Get(IFX_TAPI_LL_CH_t *pLLCh,
                                           IFX_TAPI_T38_FAX_CFG_t *pCfg)
{
   VMMC_CHANNEL *pCh = (VMMC_CHANNEL *)pLLCh;
   VMMC_DEVICE *pDev = (VMMC_DEVICE *)pCh->pParent;
   IFX_int32_t ret;
   COD_FAX_CONF_t *pConf = &pCh->pCOD->fw_cod_fax_conf;

   /* sanity check */
   if (!pDev->caps.bT38FW)
   {
      RETURN_STATUS (VMMC_statusNotSupported);
   }
   VMMC_OS_MutexGet (&pCh->chAcc);
   ret = CmdRead (pDev, (IFX_uint32_t*)pConf, (IFX_uint32_t*)pConf,
                                                             COD_FAX_CONF_LEN);
   if (ret == VMMC_statusOk)
   {
      /** T.38 protocol feature mask. */
      pCfg->OptionMask = (IFX_TAPI_T38_FEAT_MASK_t)0;
      if (pConf->T38OPT & COD_FAX_CONF_T38OPT_NON)
         pCfg->OptionMask |= IFX_TAPI_T38_FEAT_NON;
      if (pConf->T38OPT & COD_FAX_CONF_T38OPT_LONG)
         pCfg->OptionMask |= IFX_TAPI_T38_FEAT_LONG;
      if (pConf->T38OPT & COD_FAX_CONF_T38OPT_ASN1)
         pCfg->OptionMask |= IFX_TAPI_T38_FEAT_ASN1;
      if (pConf->T38OPT & COD_FAX_CONF_T38OPT_ECM)
         pCfg->OptionMask |= IFX_TAPI_T38_FEAT_ECM;

      /** Data pump demodulation gain (in dB) */
      pCfg->nGainRx = VMMC_FAX_HEX2dB ((IFX_uint16_t)pConf->GAIN1);

      /** Data pump modulation gain (in dB) */
      pCfg->nGainTx = VMMC_FAX_HEX2dB ((IFX_uint16_t)pConf->GAIN2);

      /** IFP packets send interval (in ms) */
      pCfg->nIFPSI = (IFX_TAPI_T38_IFPSI_t)pConf->IFPSI;

      /** Number of packets to calculate FEC */
      pCfg->nPktFec = (IFX_uint16_t)pConf->FEC;

      /** Data wait time ms/10 convert to ms */
      pCfg->nDWT = (IFX_uint16_t)pConf->DWT * 10;

      /** Timeout for start of T.38 modulation (in ms) */
      pCfg->nModAutoStartTime = (IFX_uint16_t)pConf->ASWT;

      /** Time to insert spoofing during automatic modulation
          ms/10 convert to ms */
      pCfg->nSpoofAutoInsTime = (IFX_uint16_t)pConf->AST * 10;

      /** Desired output power level (in -dBm)*/
      pCfg->nDbm = (IFX_uint8_t)pConf->DBM;

      /** Number of additional recovery data packets sent on high-speed
          FAX transmissions */
      pCfg->nPktRecovHiSpeed = (IFX_uint8_t)pConf->HRED;

      /** Number of additional recovery data packets sent on lowspeed
          FAX transmissions */
      pCfg->nPktRecovLoSpeed = (IFX_uint8_t)pConf->LRED;

      /** Number of additional recovery T30_INDICATOR packets */
      pCfg->nPktRecovInd = (IFX_uint8_t)pConf->IRED;

      /** Length (bytes) of valid data in the 'aNsx' field.
          Value can not be bigger than \ref IFX_TAPI_T38_NSXLEN. */
      pCfg->nNsxLen = (IFX_uint8_t)pConf->NSXLEN;
      if (pCfg->nNsxLen > IFX_TAPI_T38_NSXLEN)
         pCfg->nNsxLen = IFX_TAPI_T38_NSXLEN;

      /** Data bytes of NSX field */
      pCfg->aNsx[0] = (IFX_uint8_t)pConf->NSX1;
      pCfg->aNsx[1] = (IFX_uint8_t)pConf->NSX2;
      pCfg->aNsx[2] = (IFX_uint8_t)pConf->NXS3;
      pCfg->aNsx[3] = (IFX_uint8_t)pConf->NSX4;
      pCfg->aNsx[4] = (IFX_uint8_t)pConf->NSX5;
      pCfg->aNsx[5] = (IFX_uint8_t)pConf->NSX6;
      pCfg->aNsx[6] = (IFX_uint8_t)pConf->NXS7;
   }
   VMMC_OS_MutexRelease (&pCh->chAcc);
   return ret;
}

/**
   Sets the parameters of T.38 fax channel
   \param pLLCh           Pointer to Low-level channel structure
          pCfg            Pointer to the IFX_TAPI_T38_FAX_CFG_t structure
   \return
          VMMC_statusOk if successful else device specific return code.
*/
IFX_int32_t VMMC_TAPI_LL_COD_FAX_Cfg_Set(IFX_TAPI_LL_CH_t *pLLCh,
                                           IFX_TAPI_T38_FAX_CFG_t const *pCfg)
{
   VMMC_CHANNEL *pCh = (VMMC_CHANNEL *)pLLCh;
   VMMC_DEVICE *pDev = (VMMC_DEVICE *)pCh->pParent;
   IFX_int32_t ret;
   COD_FAX_CONF_t *pConf = &pCh->pCOD->fw_cod_fax_conf;

   /* sanity check */
   if (!pDev->caps.bT38FW)
   {
      RETURN_STATUS (VMMC_statusNotSupported);
   }
   VMMC_OS_MutexGet (&pCh->chAcc);
   /** T.38 protocol feature mask. */
   pConf->T38OPT = 0;
   if (pCfg->OptionMask & IFX_TAPI_T38_FEAT_NON)
      pConf->T38OPT |= COD_FAX_CONF_T38OPT_NON;
   if (pCfg->OptionMask & IFX_TAPI_T38_FEAT_LONG)
      pConf->T38OPT |= COD_FAX_CONF_T38OPT_LONG;
   if (pCfg->OptionMask & IFX_TAPI_T38_FEAT_ASN1)
      pConf->T38OPT |= COD_FAX_CONF_T38OPT_ASN1;
   if (pCfg->OptionMask & IFX_TAPI_T38_FEAT_ECM)
      pConf->T38OPT |= COD_FAX_CONF_T38OPT_ECM;

   /** Gain 1 */
   if (pCfg->nGainRx == IFX_TAPI_T38_CFG_DEFAULT_GAIN)
   {
      /* default gain1 */
      pConf->GAIN1 = VMMC_FAX_GAIN1_DEFAULT;
   }
   else if ((pCfg->nGainRx < VMMC_VOLUME_GAIN_MIN) ||
            (pCfg->nGainRx > VMMC_VOLUME_GAIN_MAX))
   {
      VMMC_OS_MutexRelease (&pCh->chAcc);
      /* parameter out of supported range */
      RETURN_STATUS (VMMC_statusParam);
   }
   else
   {
      /* dB values from -24 to 12 are precalculated in table */
      pConf->GAIN1 = VMMC_Gaintable[pCfg->nGainRx + (-VMMC_VOLUME_GAIN_MIN)];
   }

   /** Gain 2 */
   if (pCfg->nGainTx == IFX_TAPI_T38_CFG_DEFAULT_GAIN)
   {
      /* default gain2 */
      pConf->GAIN2 = VMMC_FAX_GAIN2_DEFAULT;
   }
   else if ((pCfg->nGainTx < VMMC_VOLUME_GAIN_MIN) ||
            (pCfg->nGainTx > VMMC_VOLUME_GAIN_MAX))
   {
      VMMC_OS_MutexRelease (&pCh->chAcc);
      /* parameter out of supported range */
      RETURN_STATUS (VMMC_statusParam);
   }
   else
   {
      /* dB values from -24 to 12 are precalculated in table */
      pConf->GAIN2 = VMMC_Gaintable[pCfg->nGainTx + (-VMMC_VOLUME_GAIN_MIN)];
   }

   /** IFP packets send interval (in ms) */
   switch (pCfg->nIFPSI)
   {
      case IFX_TAPI_T38_IFPSI_MS5:
         pConf->IFPSI = IFX_TAPI_T38_IFPSI_MS5;
         break;
      case IFX_TAPI_T38_IFPSI_MS10:
         pConf->IFPSI = IFX_TAPI_T38_IFPSI_MS10;
         break;
      case IFX_TAPI_T38_IFPSI_MS15:
         pConf->IFPSI = IFX_TAPI_T38_IFPSI_MS15;
         break;
      case IFX_TAPI_T38_IFPSI_MS20:
         pConf->IFPSI = IFX_TAPI_T38_IFPSI_MS20;
         break;
      default:
         VMMC_OS_MutexRelease (&pCh->chAcc);
         /* errmsg: At least one parameter is wrong */
         RETURN_STATUS (VMMC_statusParam);
   }

   /** Number of packets to calculate FEC */
   pConf->FEC = pCfg->nPktFec;

   /** Data wait time ms convert to ms/10 */
   pConf->DWT = pCfg->nDWT / 10;

   /** Timeout for start of T.38 modulation (in ms) */
   pConf->ASWT = pCfg->nModAutoStartTime;

   /** Time to insert spoofing during automatic modulation
       convert ms to ms/10 */
   pConf->AST = pCfg->nSpoofAutoInsTime / 10;

   /** Desired output power level (in -dBm)*/
   pConf->DBM = pCfg->nDbm;

   /** Number of additional recovery data packets sent on high-speed
       FAX transmissions */
   pConf->HRED = pCfg->nPktRecovHiSpeed;

   /** Number of additional recovery data packets sent on lowspeed
       FAX transmissions */
   pConf->LRED = pCfg->nPktRecovLoSpeed;

   /** Number of additional recovery T30_INDICATOR packets */
   pConf->IRED = pCfg->nPktRecovInd;

   /** Length (bytes) of valid data in the 'aNsx' field.
       Value can not be bigger than \ref IFX_TAPI_T38_NSXLEN. */
   if (pCfg->nNsxLen > IFX_TAPI_T38_NSXLEN)
   {
      VMMC_OS_MutexRelease (&pCh->chAcc);
      /* errmsg: At least one parameter is wrong */
      RETURN_STATUS (VMMC_statusParam);
   }
   pConf->NSXLEN = pCfg->nNsxLen;

   /** Data bytes of NSX field */
   pConf->NSX1 = pCfg->aNsx[0];
   pConf->NSX2 = pCfg->aNsx[1];
   pConf->NXS3 = pCfg->aNsx[2];
   pConf->NSX4 = pCfg->aNsx[3];
   pConf->NSX5 = pCfg->aNsx[4];
   pConf->NSX6 = pCfg->aNsx[5];
   pConf->NXS7 = pCfg->aNsx[6];

   ret = CmdWrite (pDev, (IFX_uint32_t*)pConf, COD_FAX_CONF_LEN);
   VMMC_OS_MutexRelease (&pCh->chAcc);
   return ret;
}


/**
   Gets the current configuration of the FAX Data Pump parameters
   \param pLLCh           Pointer to Low-level channel structure
          pFDPCfg         Pointer to the IFX_TAPI_T38_FDP_CFG_t structure
   \return
          VMMC_statusOk if successful else device specific return code.
*/
IFX_int32_t VMMC_TAPI_LL_COD_FAX_FDP_Cfg_Get(IFX_TAPI_LL_CH_t *pLLCh,
                                        IFX_TAPI_T38_FDP_CFG_t *pFDPCfg)
{
   VMMC_CHANNEL *pCh = (VMMC_CHANNEL *)pLLCh;
   VMMC_DEVICE *pDev = (VMMC_DEVICE *)pCh->pParent;
   IFX_int32_t ret;
   COD_FAX_FDP_PARAMS_t *pFDPConf = &pCh->pCOD->fw_cod_fax_fdp_params;

   /* sanity check */
   if (!pDev->caps.bT38FW)
   {
      RETURN_STATUS (VMMC_statusNotSupported);
   }
   VMMC_OS_MutexGet (&pCh->chAcc);
   ret = CmdRead (pDev, (IFX_uint32_t*)pFDPConf, (IFX_uint32_t*)pFDPConf,
                                                       COD_FAX_FDP_PARAMS_LEN);
   if (ret == VMMC_statusOk)
   {
      /** Modulation buffer size (in units of 0.625ms) */
      pFDPCfg->nMobsz = (IFX_uint16_t)pFDPConf->MOBSZ;
      /** Required modulation buffer fill level (in units of 0.625ms)
          before for modulation starts */
      pFDPCfg->nMobsm = (IFX_uint16_t)pFDPConf->MOBSM;
      /** Required modulation buffer fill level (in units of 0.625ms)
          before the modulation requests more data */
      pFDPCfg->nMobrd = (IFX_uint16_t)pFDPConf->MOBRD;
      /** Required demodulation buffer level (in units of 0.625ms)
          before the demodulator sends data */
      pFDPCfg->nDmbsd = (IFX_uint16_t)pFDPConf->DMBSD;
   }
   VMMC_OS_MutexRelease (&pCh->chAcc);
   return ret;
}

/**
   Sets the FAX Data Pump parameters
   \param pLLCh           Pointer to Low-level channel structure
          pFDPCfg         Pointer to the IFX_TAPI_T38_FDP_CFG_t structure
   \return
          VMMC_statusOk if successful else device specific return code.
*/
IFX_int32_t VMMC_TAPI_LL_COD_FAX_FDP_Cfg_Set(IFX_TAPI_LL_CH_t *pLLCh,
                                        IFX_TAPI_T38_FDP_CFG_t const *pFDPCfg)
{
   VMMC_CHANNEL *pCh = (VMMC_CHANNEL *)pLLCh;
   VMMC_DEVICE *pDev = (VMMC_DEVICE *)pCh->pParent;
   IFX_int32_t ret;
   COD_FAX_FDP_PARAMS_t *pFDPConf = &pCh->pCOD->fw_cod_fax_fdp_params;

   /* sanity check */
   if (!pDev->caps.bT38FW)
   {
      RETURN_STATUS (VMMC_statusNotSupported);
   }
   /* parameter range check */
   if ((pFDPCfg->nMobsz < VMMC_MOBSZ_MIN || pFDPCfg->nMobsz > VMMC_MOBSZ_MAX) ||
       (pFDPCfg->nMobsm < VMMC_MOBSM_MIN || pFDPCfg->nMobsm > VMMC_MOBSM_MAX) ||
       (pFDPCfg->nMobrd < VMMC_MOBRD_MIN || pFDPCfg->nMobrd > VMMC_MOBRD_MAX) ||
       (pFDPCfg->nDmbsd < VMMC_DMBSD_MIN || pFDPCfg->nDmbsd > VMMC_DMBSD_MAX))
   {
      /* errmsg: At least one parameter is wrong  */
      RETURN_STATUS (VMMC_statusParam);
   }
   VMMC_OS_MutexGet (&pCh->chAcc);
   /** Modulation buffer size (in units of 0.625ms) */
   pFDPConf->MOBSZ = pFDPCfg->nMobsz;
   /** Required modulation buffer fill level (in units of 0.625ms)
       before for modulation starts */
   pFDPConf->MOBSM = pFDPCfg->nMobsm;
   /** Required modulation buffer fill level (in units of 0.625ms)
       before the modulation requests more data */
   pFDPConf->MOBRD = pFDPCfg->nMobrd;
   /** Required demodulation buffer level (in units of 0.625ms)
       before the demodulator sends data */
   pFDPConf->DMBSD = pFDPCfg->nDmbsd;
   ret = CmdWrite (pDev, (IFX_uint32_t*)pFDPConf, COD_FAX_FDP_PARAMS_LEN);
   VMMC_OS_MutexRelease (&pCh->chAcc);
   return ret;
}

/**
   starts a T.38 fax session on the given channel
   \param pLLCh           Pointer to Low-level channel structure
          pT38Cfg         Pointer to the IFX_TAPI_T38_SESS_CFG_t structure
   \return
          VMMC_statusOk if successful else device specific return code.
*/
IFX_int32_t VMMC_TAPI_LL_COD_FAX_Start(IFX_TAPI_LL_CH_t *pLLCh,
                                       IFX_TAPI_T38_SESS_CFG_t const *pT38Cfg)
{
   VMMC_CHANNEL *pCh = (VMMC_CHANNEL *) pLLCh;
   VMMC_DEVICE  *pDev = pCh->pParent;
   IFX_int32_t ret = VMMC_statusOk;
   COD_FAX_ACT_t *pFaxCrtl = &pCh->pCOD->fw_cod_fax_act;

   /* sanity check */
   if (!pDev->caps.bT38FW)
   {
      RETURN_STATUS (VMMC_statusNotSupported);
   }
   if (pFaxCrtl->EN == COD_FAX_ACT_ON)
   {
      /* errmsg: Action not possible when fax channel already running */
      RETURN_STATUS (VMMC_statusT38Restart);
   }
   if (pCh->pCOD->fw_cod_ch_speech.EN != COD_CHAN_SPEECH_DISABLE)
   {
      /* errmsg: Action not possible when coder is running */
      RETURN_STATUS (VMMC_statusCodRun);
   }

   VMMC_OS_MutexGet (&pCh->chAcc);
   /* Same sampling rate as for coder */
   pFaxCrtl->ISR = pCh->pCOD->fw_cod_ch_speech.ISR;

   /* use same resource number as channel number */
   pFaxCrtl->T38R = pFaxCrtl->CHAN;

   /* Use current coder channel input,
      T38 channel is part of Coder channel all connections setuped in coder */
   pFaxCrtl->I1 = pCh->pCOD->fw_cod_ch_speech.I1;
   if (pFaxCrtl->I1 == ECMD_IX_EMPTY)
   {
      VMMC_OS_MutexRelease (&pCh->chAcc);
      /* errmsg: Action not possible when coder channel not mapped */
      RETURN_STATUS (VMMC_statusT38NotMapped);
   }

   /* Maximum bit rate (in bit/s). The maximum bit rate can be configured to
      restrict the facsimile page transfer rate */
   if ((pT38Cfg->nBitRateMax > VMMC_MAXBITRATE_MAX) ||
       (pT38Cfg->nBitRateMax < VMMC_MAXBITRATE_MIN))
   {
      VMMC_OS_MutexRelease (&pCh->chAcc);
      /* errmsg: At least one parameter is wrong */
      RETURN_STATUS (VMMC_statusParam);
   }
   pFaxCrtl->MAXBITRATE = pT38Cfg->nBitRateMax /100;

   /* Supported transport protocol */
   if (pT38Cfg->nProtocol != IFX_TAPI_T38_UDP)
   {
      VMMC_OS_MutexRelease (&pCh->chAcc);
      /* errmsg: At least one parameter is wrong */
      RETURN_STATUS (VMMC_statusParam);
   }
   pFaxCrtl->TRPR = COD_FAX_ACT_TRPR_UDP;

   /* Rate management method */
   switch (pT38Cfg->nRateManagement)
   {
      case IFX_TAPI_T38_LOC_TCF:
         pFaxCrtl->RMM = COD_FAX_ACT_RMM_LOC_TCF;
         break;
      case IFX_TAPI_T38_TRANS_TCF:
         pFaxCrtl->RMM = COD_FAX_ACT_RMM_TRANS_TCF;
         break;
      default:
         VMMC_OS_MutexRelease (&pCh->chAcc);
         /* errmsg: At least one parameter is wrong */
         RETURN_STATUS (VMMC_statusParam);
   }

   /* T.38 ASN.1 version */
   if (pT38Cfg->nT38Ver != COD_FAX_ACT_T38VER_VERS0)
   {
      VMMC_OS_MutexRelease (&pCh->chAcc);
      /* errmsg: At least one parameter is wrong */
      RETURN_STATUS (VMMC_statusParam);
   }
   pFaxCrtl->T38VER = pT38Cfg->nT38Ver;

   /* Facsimile image conversion options */
   pFaxCrtl->BITOPT = pT38Cfg->FacConvOpt;

   if (!pT38Cfg->nUDPBuffSizeMax || !pT38Cfg->nUDPDatagramSizeMax)
   {
      VMMC_OS_MutexRelease (&pCh->chAcc);
      /* errmsg: At least one parameter is wrong */
      RETURN_STATUS (VMMC_statusParam);
   }

   /* UDP maximum buffer size */
   pFaxCrtl->UDPMBS = pT38Cfg->nUDPBuffSizeMax;

   /* UDP maximum datagram size */
   pFaxCrtl->UDPMDS = pT38Cfg->nUDPDatagramSizeMax;

   /* UDP error correction method */
   if (pT38Cfg->nUDPErrCorr == IFX_TAPI_T38_RED)
   {
      pFaxCrtl->UDPEC = COD_FAX_ACT_UDPFEC_RED;
   }
   else if (pT38Cfg->nUDPErrCorr == IFX_TAPI_T38_FEC)
   {
      pFaxCrtl->UDPEC = COD_FAX_ACT_UDPFEC_FEC;
   }
   else
   {
      VMMC_OS_MutexRelease (&pCh->chAcc);
      /* errmsg: At least one parameter is wrong */
      RETURN_STATUS (VMMC_statusParam);
   }

   pFaxCrtl->EN = COD_FAX_ACT_ON;
   ret = CmdWrite (pDev, (IFX_uint32_t*)pFaxCrtl, COD_FAX_ACT_LEN);
   if (ret == VMMC_statusOk)
      pCh->bTapiT38Status = IFX_TRUE;
   VMMC_OS_MutexRelease (&pCh->chAcc);
   return ret;
}

/**
   Gets a T.38 fax session statistics
   \param pLLCh           Pointer to Low-level channel structure
          pStat           Pointer to IFX_TAPI_T38_SESS_STATISTICS_t structure
   \return
          VMMC_statusOk if successful else device specific return code.
*/
IFX_int32_t VMMC_TAPI_LL_COD_FAX_Stat_Get(IFX_TAPI_LL_CH_t *pLLCh,
                                  IFX_TAPI_T38_SESS_STATISTICS_t *pStat)
{
   VMMC_CHANNEL *pCh = (VMMC_CHANNEL *)pLLCh;
   VMMC_DEVICE *pDev = (VMMC_DEVICE *)pCh->pParent;
   IFX_int32_t ret;
   COD_FAX_STAT_t *pFaxStat = &pCh->pCOD->fw_cod_fax_stat;

   /* sanity check */
   if (!pDev->caps.bT38FW)
   {
      RETURN_STATUS (VMMC_statusNotSupported);
   }
   if (pCh->pCOD->fw_cod_fax_act.EN != COD_FAX_ACT_ON)
   {
      /* errmsg: Action not possible when fax channel not active */
      RETURN_STATUS (VMMC_statusT38NotActive);
   }
   VMMC_OS_MutexGet (&pCh->chAcc);
   ret = CmdRead (pDev, (IFX_uint32_t*)pFaxStat, (IFX_uint32_t*)pFaxStat,
                                                             COD_FAX_STAT_LEN);
   if (ret == VMMC_statusOk)
   {
      /** T.38 session flags. Selects the standard used for Fax T.38 */
      pStat->SessInfo = (IFX_TAPI_T38_SESS_FLAGS_t)0;
      if (pFaxStat->T38SESSFL & COD_FAX_STAT_T38SESSFL_FEC)
         pStat->SessInfo |= IFX_TAPI_T38_SESS_FEC;
      if (pFaxStat->T38SESSFL & COD_FAX_STAT_T38SESSFL_RED)
         pStat->SessInfo |= IFX_TAPI_T38_SESS_RED;
      if (pFaxStat->T38SESSFL & COD_FAX_STAT_T38SESSFL_ECM)
         pStat->SessInfo |= IFX_TAPI_T38_SESS_ECM;
      if (pFaxStat->T38SESSFL & COD_FAX_STAT_T38SESSFL_T30COMPL)
         pStat->SessInfo |= IFX_TAPI_T38_SESS_T30COMPL;

      /** FAX data pump standards used during session */
      pStat->nFdpStand = (IFX_TAPI_T38_FDPSTD_t)0;
      if (pFaxStat->FDPSTAND & COD_FAX_STAT_FDPSTAND_V27_2400)
         pStat->nFdpStand |= IFX_TAPI_T38_FDPSTD_V27_2400;
      if (pFaxStat->FDPSTAND & COD_FAX_STAT_FDPSTAND_V27_4800)
         pStat->nFdpStand |= IFX_TAPI_T38_FDPSTD_V27_4800;
      if (pFaxStat->FDPSTAND & COD_FAX_STAT_FDPSTAND_V29_7200)
         pStat->nFdpStand |= IFX_TAPI_T38_FDPSTD_V29_7200;
      if (pFaxStat->FDPSTAND & COD_FAX_STAT_FDPSTAND_V29_9600)
         pStat->nFdpStand |= IFX_TAPI_T38_FDPSTD_V29_9600;
      if (pFaxStat->FDPSTAND & COD_FAX_STAT_FDPSTAND_V17_7200)
         pStat->nFdpStand |= IFX_TAPI_T38_FDPSTD_V17_7200;
      if (pFaxStat->FDPSTAND & COD_FAX_STAT_FDPSTAND_V17_9600)
         pStat->nFdpStand |= IFX_TAPI_T38_FDPSTD_V17_9600;
      if (pFaxStat->FDPSTAND & COD_FAX_STAT_FDPSTAND_V17_12000)
         pStat->nFdpStand |= IFX_TAPI_T38_FDPSTD_V17_12000;
      if (pFaxStat->FDPSTAND & COD_FAX_STAT_FDPSTAND_V17_14400)
         pStat->nFdpStand |= IFX_TAPI_T38_FDPSTD_V17_14400;

      /** Number of lost packets */
      pStat->nPktLost = (IFX_uint32_t)pFaxStat->T38_PKTS_LOST;

      /** Number of recovered packets by usage of error correction
          mechanism (Redundancy or FEC) */
      pStat->nPktRecov = (IFX_uint32_t)pFaxStat->T38_PKTS_REC;

      /** Maximum number of consecutively lost packets */
      pStat->nPktGroupLost = (IFX_uint32_t)pFaxStat->T38_PKTS_LOSTGROUP;

      /** State of facsimile transmission */
      pStat->nFaxSessState =
          (IFX_TAPI_T38_SESS_STATE_t)pFaxStat->T38_PKTS_FAX_STATE;

      /** Number of FTT responses */
      pStat->nFttRsp = (IFX_uint16_t)pFaxStat->FTTNUM;
      /** Number of transmitted pages */
      pStat->nPagesTx = (IFX_uint16_t)pFaxStat->TXPAGES;
      /** Number of scan line breaks during modulation */
      pStat->nLineBreak = (IFX_uint32_t)pFaxStat->LINEBREAK;
      /** Number of facsimile control frame line breaks during modulation */
      pStat->nV21FrmBreak = (IFX_uint16_t)pFaxStat->V21FRM_BREAK;
      /** Number of ECM frame breaks during modulation */
      pStat->nEcmFrmBreak = (IFX_uint16_t)pFaxStat->ECMFRM_BREAK;
      /** Major version of T.38 implementation */
      pStat->nT38VerMajor = (IFX_uint16_t)pFaxStat->T38_VER_MAJ;
      /** Minor version of T.38 implementation */
      pStat->nT38VerMin = (IFX_uint16_t)pFaxStat->T38_VER_MIN;
   }
   VMMC_OS_MutexRelease (&pCh->chAcc);
   return ret;
}

/**
   stops currently running T.38 fax session on the given channel
   \param pLLCh           Pointer to Low-level channel structure
   \return
          VMMC_statusOk if successful else device specific return code.
*/
IFX_int32_t VMMC_TAPI_LL_COD_FAX_Stop(IFX_TAPI_LL_CH_t *pLLCh)
{
   VMMC_CHANNEL *pCh = (VMMC_CHANNEL *) pLLCh;
   VMMC_DEVICE  *pDev = pCh->pParent;
   IFX_int32_t ret = VMMC_statusOk;
   COD_FAX_ACT_t *pFaxCrtl = &pCh->pCOD->fw_cod_fax_act;

   /* sanity check */
   if (!pDev->caps.bT38FW)
   {
      RETURN_STATUS (VMMC_statusNotSupported);
   }
   if (pFaxCrtl->EN != COD_FAX_ACT_ON)
   {
      /* already stopped */
      return ret;
   }
   VMMC_OS_MutexGet (&pCh->chAcc);
   /* stop fax channel */
   pFaxCrtl->EN = COD_FAX_ACT_OFF;
   ret = CmdWrite (pDev, (IFX_uint32_t*)pFaxCrtl, COD_FAX_ACT_LEN);
   if (ret == VMMC_statusOk)
      pCh->bTapiT38Status = IFX_FALSE;
   VMMC_OS_MutexRelease (&pCh->chAcc);
   return ret;
}

/**
   used to enable/disable a trace of FAX events
   \param pLLCh           Pointer to Low-level channel structure
   \param pTrace          Pointer to the IFX_TAPI_T38_TRACE_CFG_t structure
   \return
          VMMC_statusOk if successful else device specific return code.
*/
IFX_int32_t VMMC_TAPI_LL_COD_FAX_Trace(
   IFX_TAPI_LL_CH_t *pLLCh,
   IFX_TAPI_T38_TRACE_CFG_t const *pTrace)
{
   VMMC_CHANNEL *pCh = (VMMC_CHANNEL *)pLLCh;
   VMMC_DEVICE *pDev = (VMMC_DEVICE *)pCh->pParent;
   IFX_int32_t ret = VMMC_statusOk;
   COD_FAX_TRACE_t *pTraceCtrl = &pCh->pCOD->fw_cod_fax_trace;

   /* sanity check */
   if (!pDev->caps.bT38FW)
   {
      RETURN_STATUS (VMMC_statusNotSupported);
   }
   VMMC_OS_MutexGet (&pCh->chAcc);
   if (pTrace->DbgMask == 0)
   {
      pTraceCtrl->EN = COD_FAX_TRACE_OFF;
   }
   else
   {
      pTraceCtrl->EN = COD_FAX_TRACE_ON;
   }
   pTraceCtrl->FEATURE = COD_FAX_TRACE_FEATURE_T38TRACE;
   pTraceCtrl->DBGMASK = pTrace->DbgMask;
   ret = CmdWrite (pDev, (IFX_uint32_t*)pTraceCtrl, COD_FAX_TRACE_LEN);
   VMMC_OS_MutexRelease (&pCh->chAcc);
   return ret;
}

/**
   Convertation from HEX to dB
   \param
      Hex       - gain value in firmware HEX format
   \return
      dB value
*/
static IFX_int16_t VMMC_FAX_HEX2dB (IFX_uint16_t Hex)
{
   IFX_int16_t i;

   /* Check Hex value for each dB starting from 12 to -84 */
   for (i = (-VMMC_VOLUME_GAIN_MIN) + VMMC_VOLUME_GAIN_MAX; i >= 0; i--)
   {
      if (Hex >= VMMC_Gaintable[i])
         return i - (-VMMC_VOLUME_GAIN_MIN);
   }
   return ((VMMC_VOLUME_GAIN_MIN) - 1);
}
#endif /* VMMC_FEAT_FAX_T38_FW */

#ifdef VMMC_FEAT_FAX_T38
/**
   Configures the fax datapump demodulator

   \param  pCh        - pointer to VMMC channel structure
   \param  nSt1       - standard 1
   \param  nSt2       - Standard 2
   \param  nEq        - Equalizer
   \param  nTr        - Training

   \return
      IFX_SUCCESS or IFX_ERROR
*/
static IFX_int32_t vmmc_cod_DP_DemodSet (VMMC_CHANNEL *pCh, IFX_uint8_t nSt1,
                                   IFX_uint8_t nSt2, IFX_uint8_t nEq,
                                   IFX_uint8_t nTr)
{
   VMMC_DEVICE          *pDev = pCh->pParent;
   COD_FAX_DEMOD_CTRL_t CodDemod;

   memset (&CodDemod, 0, sizeof(COD_FAX_DEMOD_CTRL_t));
   CodDemod.CMD  = CMD_EOP;
   CodDemod.CHAN = pCh->nChannel - 1;
   CodDemod.ECMD = COD_FAX_DEMOD_CTRL_ECMD;
   CodDemod.MOD  = MOD_CODER;

   CodDemod.STD1 = (nSt1 & COD_FAX_DEMOD_CTRL_STD1);
   CodDemod.STD2 = (nSt2 & COD_FAX_DEMOD_CTRL_STD2);
   CodDemod.EQ   = (nEq  & COD_FAX_DEMOD_CTRL_EQ_REUSE);
   CodDemod.TRN  = (nTr  & COD_FAX_DEMOD_CTRL_TRN_LONG);

   return CmdWrite (pDev, (IFX_uint32_t *) &CodDemod,
                    sizeof(COD_FAX_DEMOD_CTRL_t) - CMD_HDR_CNT);
}


/**
   Configures the Datapump modulator

   \param   pCh        - pointer to VMMC channel structure
   \param   nSt        - standard
   \param   nLen       - signal length
   \param   nDbm       - level
   \param   nTEP       - TEP
   \param   nTr        - Training

   \return
      IFX_SUCCESS or IFX_ERROR
*/
static IFX_int32_t vmmc_cod_DP_ModSet (VMMC_CHANNEL *pCh, IFX_uint8_t nSt,
                                       IFX_uint16_t nLen, IFX_uint8_t nDbm,
                                       IFX_uint8_t nTEP, IFX_uint8_t nTr)
{
   VMMC_DEVICE          *pDev = pCh->pParent;
   COD_FAX_MOD_CTRL_t CodMod;

   memset (&CodMod, 0, sizeof(COD_FAX_DEMOD_CTRL_t));
   CodMod.CMD  = CMD_EOP;
   CodMod.CHAN = pCh->nChannel - 1;
   CodMod.ECMD = COD_FAX_MOD_CTRL_ECMD;
   CodMod.MOD  = MOD_CODER;

   CodMod.STD = nSt;
   CodMod.DBM = nDbm;
   CodMod.TEP = nTEP;
   CodMod.TRN = nTr;

   if (nLen <= COD_FAX_MOD_CTRL_MAX_SGLEN)
      CodMod.SGLEN = (nLen & COD_FAX_MOD_CTRL_SGLEN);
   else
      CodMod.SGLEN = COD_FAX_MOD_CTRL_MAX_SGLEN;

   return CmdWrite (pDev, (IFX_uint32_t*)&CodMod,
                    sizeof (COD_FAX_MOD_CTRL_t) - CMD_HDR_CNT);
}


/**
   Set data pump according to input parameters

   This function disables datapump if bEn is IFX_FALSE. Otherwise it enables
   the datapump and set modulator or demodulator according to bMod.

   \param   pCh        - pointer to VMMC channel structure
   \param   bEn        - IFX_FALSE : disable / IFX_TRUE : enable datapump
   \param   bMod       - 0 = Modulator / 1 = Demodulator
   \param   gain       - gain to be applied, signed value in steps of 1dB
   \param   mod_start  - level for start modulation
   \param   mod_req    - level for request more data
   \param   demod_send - level for send data

   \return
      - IFX_SUCCESS or IFX_ERROR
      - VMMC_statusFuncParm  Gain parameter out of the supported range.
*/
static IFX_int32_t vmmc_cod_DP_Set (VMMC_CHANNEL *pCh, IFX_boolean_t bEn,
                                    IFX_boolean_t bMod, IFX_int16_t gain,
                                    IFX_uint16_t mod_start,
                                    IFX_uint16_t mod_req,
                                    IFX_uint16_t demod_send)
{
   VMMC_DEVICE    *pDev  = pCh->pParent;
   IFX_uint8_t    ch     = (pCh->nChannel - 1);
   IFX_uint32_t   nCount = 4;
   COD_FAX_CTRL_t *pCodFaxCtrl;

   pCodFaxCtrl = &pCh->pCOD->fw_cod_fax_ctrl;
   /* Use the same input configuration as the coder does */
   pCodFaxCtrl->I1 = VMMC_CON_Get_COD_SignalInput (pCh, 0);
   pCodFaxCtrl->CHAN = ch;
   pCodFaxCtrl->DPNR = ch;

   /* Disable Datapump if  EN = 0 */
   if (bEn == IFX_FALSE)
   {
      pCodFaxCtrl->EN = 0;
   }
   else
   {
      IFX_uint16_t fw_gain;

      nCount = 12;
      pCodFaxCtrl->EN = 1;

      /* Calculate the FW gain value.
         Two ranges of gain values are supported.
         - The new range is from VMMC_VOLUME_GAIN_MIN (-24 dB) to
           VMMC_VOLUME_GAIN_MAX (+12 dB). It uses a lookup table to translate
           the dB-values into precalculated values.
         - The old range for compatibility with the 2CPE device starts with
           VMMC_VOLUME_GAIN_MAX + 1 (= 13 resulting in -17 dB) and ranges
           to 0x7FFF / 85 (= 385 resulting in +12 dB). The value of 0x60 is
           translated into the exaxt 0 dB equivalent. The use of this range
           is deprecated. */
      if ((gain < VMMC_VOLUME_GAIN_MIN) || (gain > (0x7FFF / 85)))
      {
         /* parameter out of supported range */
         RETURN_STATUS (VMMC_statusFuncParm);
      }
      if (gain <= VMMC_VOLUME_GAIN_MAX)
      {
         /* Lookup FW gain value from precalculated table. */
         fw_gain = VMMC_Gaintable[gain + (-VMMC_VOLUME_GAIN_MIN)];
      }
      else
      {
         /* 2CPE compatiblity mode. Use rough aproximation of gain formula */
         fw_gain = (gain == 0x60) ? VMMC_GAIN_0DB : 85 * gain;
      }

      switch (bMod)
      {
         /* configure data for modulation */
         case IFX_FALSE:
            pCodFaxCtrl->MD = COD_FAX_CTRL_MD_MODULATION;
            /* Modulation gain */
            pCodFaxCtrl->GAIN2 = fw_gain;
            /* MOBSM: level for start modulation */
            pCodFaxCtrl->MOBSM = mod_start;
            /* MOBRD: level for request more data */
            pCodFaxCtrl->MOBRD = mod_req;
            break;
         /* configure data for demodulation */
         case IFX_TRUE:
            /* MD = 1, */
            pCodFaxCtrl->MD = COD_FAX_CTRL_MD_DEMODULATION;
            /* Demodulation gain */
            pCodFaxCtrl->GAIN1 = fw_gain;
            /* DMBSD: send data level */
            pCodFaxCtrl->DMBSD = demod_send;
            break;
      }
   }

   return (CmdWrite (pDev, (IFX_uint32_t *) pCodFaxCtrl, nCount));
}


/**
   Configures the Datapump for Modulation

   \param  pLLChannel   Pointer to the VMMC channel structure.
   \param  pFaxMod      Pointer to IFX_TAPI_T38_MOD_DATA_t structure.

   \return
   - VMMC_statusCmdWr Writing the command failed
   - VMMC_statusCodRun Modulator may not be activated when the coder is running
   - VMMC_statusFuncParm Wrong parameter passed.
   - VMMC_statusOk if successful

   \remarks
   The configuration of the Modulator requires that the datapump is disabled.
   So the following is done here :
      - Disable datapump if enabled
      - Set Modulator
      - Activate datapump for Modulation and
        set other datapump parameters related to Modulation
*/
IFX_int32_t VMMC_TAPI_LL_COD_T38_Mod_Enable (IFX_TAPI_LL_CH_t *pLLChannel,
                                         IFX_TAPI_T38_MOD_DATA_t const *pFaxMod)
{
   VMMC_CHANNEL *pCh = (VMMC_CHANNEL *)pLLChannel;
   IFX_int32_t ret;
   IFX_TAPI_EVENT_t tapiEvent;

   if (pCh->pCOD->fw_cod_ch_speech.EN)
   {
      /* errmsg: Action not possible when coder is running */
      RETURN_STATUS (VMMC_statusCodRun);
   }

   /* disable datapump */
   ret = vmmc_cod_DP_Set (pCh, IFX_FALSE, IFX_FALSE, 0, 0, 0, 0);

   /* set command to configure the modulator */
   if (VMMC_SUCCESS(ret))
   {
      ret = vmmc_cod_DP_ModSet (pCh, pFaxMod->nStandard,
                                pFaxMod->nSigLen, pFaxMod->nDbm,
                                pFaxMod->nTEP, pFaxMod->nTraining);
   }
   else
   {
      pCh->TapiFaxStatus.nStatus &= ~ (IFX_TAPI_FAX_T38_TX_ON |
                                            IFX_TAPI_FAX_T38_DP_ON);
      /* set datapump failed, no further action required */
      return ret;
   }

   if (VMMC_SUCCESS(ret))
   {
      /* set fax state as running */
      pCh->TapiFaxStatus.nStatus |= IFX_TAPI_FAX_T38_TX_ON;
      /* configure and start the datapump for modulation */
      ret = vmmc_cod_DP_Set (pCh, IFX_TRUE, IFX_FALSE, pFaxMod->nGainTx,
                             pFaxMod->nMobsm, pFaxMod->nMobrd, 0);
   }

   /* handle error case: Continue with voice */
   if (!VMMC_SUCCESS(ret))
   {
      /**\todo Here vmmc_cod_DP_Set or vmmc_cod_DP_ModSet generated an error.
       Does this make sence to call it again?
       Anyway if vmmc_cod_DP_ModSet failed no DP is enabled */
      ret = vmmc_cod_DP_Set (pCh, IFX_FALSE, IFX_FALSE, 0, 0, 0, 0);
      pCh->TapiFaxStatus.nStatus &= ~IFX_TAPI_FAX_T38_TX_ON;
      /* set error and issue tapi exception */
      memset(&tapiEvent, 0, sizeof(IFX_TAPI_EVENT_t));
      tapiEvent.id = IFX_TAPI_EVENT_T38_ERROR_SETUP;
      tapiEvent.module = IFX_TAPI_MODULE_TYPE_COD;
      tapiEvent.data.value = IFX_TAPI_EVENT_T38_ERROR_SETUP_MODON;
      IFX_TAPI_Event_Dispatch(pCh->pTapiCh,&tapiEvent);
   }
   else
   {
      pCh->TapiFaxStatus.nStatus |= IFX_TAPI_FAX_T38_DP_ON;
   }

   return ret;
}


/**
   Configures the Datapump for Demodulation

   \param  pLLChannel   Pointer to the VMMC channel structure.
   \param  pFaxDemod    Pointer to IFX_TAPI_T38_DEMOD_DATA_t structure.

   \return
   - VMMC_statusCmdWr Writing the command failed
   - VMMC_statusCodRun Modulator may not be activated when the coder is running
   - VMMC_statusFuncParm Wrong parameter passed.
   - VMMC_statusOk if successful

   \remarks
   The configuration of the Demodulator requires that the datapump is
   disabled.
   So the followings are done here :
      - Disable datapump if enable
      - Set Demodulator
      - Activate datapump for Demodulation and
        set other datapump parameters related to Demodulation
*/
IFX_int32_t VMMC_TAPI_LL_COD_T38_DeMod_Enable (IFX_TAPI_LL_CH_t *pLLChannel,
                                               IFX_TAPI_T38_DEMOD_DATA_t const *pFaxDemod)
{
   VMMC_CHANNEL *pCh = (VMMC_CHANNEL *) pLLChannel;
   VMMC_DEVICE *pDev = pCh->pParent;
   IFX_int32_t ret;
   IFX_TAPI_EVENT_t tapiEvent;

   if (pCh->pCOD->fw_cod_ch_speech.EN)
   {
      /* errmsg: Action not possible when coder is running */
      RETURN_STATUS (VMMC_statusCodRun);
   }

   /* Disable Channel Fax datapump */
   ret = vmmc_cod_DP_Set (pCh, IFX_FALSE, IFX_FALSE, 0, 0, 0, 0);

   /* set command to configure the demodulator */
   if (VMMC_SUCCESS(ret))
   {
      ret = vmmc_cod_DP_DemodSet (pCh, pFaxDemod->nStandard1,
                                  pFaxDemod->nStandard2,
                                  pFaxDemod->nEqualizer, pFaxDemod->nTraining);
   }
   else
   {
      /* set datapump failed, no further action required */
      return ret;
   }

   if (VMMC_SUCCESS(ret))
   {
      /* Clear fifo for this channel to record valid fax data */
      Vmmc_IrqLockDevice (pDev);
      /* set fax state as running */
      pCh->TapiFaxStatus.nStatus |= IFX_TAPI_FAX_T38_TX_ON;
      Vmmc_IrqUnlockDevice (pDev);
      /* configure the datapump for demodulation */
      ret = vmmc_cod_DP_Set (pCh, IFX_TRUE, IFX_TRUE, pFaxDemod->nGainRx,
                             0, 0, pFaxDemod->nDmbsd);
   }
   else
   {
      pCh->TapiFaxStatus.nStatus &= ~ (IFX_TAPI_FAX_T38_TX_ON |
                                            IFX_TAPI_FAX_T38_DP_ON);
      /* SetDPDemod failed, no further action required */
      return ret;
   }

   /* configure the datapump for demodulation */
   if (!VMMC_SUCCESS(ret))
   {
      /* handle error case: continue with voice */
      ret = vmmc_cod_DP_Set (pCh, IFX_FALSE, IFX_FALSE, 0, 0, 0, 0);
      Vmmc_IrqLockDevice (pDev);
      /* from now on, fax is running */
      pCh->TapiFaxStatus.nStatus = 0;
      Vmmc_IrqUnlockDevice (pDev);
      /* set error and issue tapi exception */
      /* Fill event structure. */
      memset(&tapiEvent, 0, sizeof(IFX_TAPI_EVENT_t));
      tapiEvent.id = IFX_TAPI_EVENT_T38_ERROR_SETUP;
      tapiEvent.module = IFX_TAPI_MODULE_TYPE_COD;
      tapiEvent.data.value = IFX_TAPI_EVENT_T38_ERROR_SETUP_DEMODON;
      IFX_TAPI_Event_Dispatch(pCh->pTapiCh,&tapiEvent);
   }
   else
   {
      pCh->TapiFaxStatus.nStatus |= IFX_TAPI_FAX_T38_DP_ON;
   }

   return ret;
}


/**
   Disables the Fax datapump

   \param  pLLChannel   Pointer to the VMMC channel structure.

   \return
   - VMMC_statusCmdWr Writing the command failed
   - VMMC_statusOk if successful

   \remarks
   Sends the VoFW message Code Channel Fax Control to disable it.
*/
IFX_int32_t VMMC_TAPI_LL_COD_T38_Datapump_Disable (IFX_TAPI_LL_CH_t *pLLChannel)
{
   VMMC_CHANNEL  *pCh = (VMMC_CHANNEL *)pLLChannel;
   IFX_int32_t ret;
   IFX_TAPI_EVENT_t tapiEvent;

   /* Disable Channel Fax datapump */
   ret = vmmc_cod_DP_Set (pCh, IFX_FALSE, IFX_FALSE, 0, 0, 0, 0);
   /* activate voice path and make driver ready for a next fax connection */
   if (VMMC_SUCCESS(ret))
   {
      /* reset status /error flags */
      pCh->TapiFaxStatus.nStatus = 0;
      pCh->TapiFaxStatus.nError = 0;
   }
   else
   {
      /* set error and issue tapi exception */
      /* Fill event structure. */
      memset(&tapiEvent, 0, sizeof(IFX_TAPI_EVENT_t));
      tapiEvent.id = IFX_TAPI_EVENT_T38_ERROR_SETUP;
      tapiEvent.module = IFX_TAPI_MODULE_TYPE_COD;
      tapiEvent.data.value = IFX_TAPI_EVENT_T38_ERROR_SETUP_DPOFF;
      IFX_TAPI_Event_Dispatch(pCh->pTapiCh,&tapiEvent);
      TRACE (VMMC, DBG_LEVEL_HIGH,
             ("DRV_ERR: Disable Datapump" " failed\n"));
   }

   pCh->nFdpReq = 0;
   pCh->nFdpReqServiced = 0;

   return ret;
}


/**
   Query T.38 Fax Status

   \param  pLLChannel   Pointer to the VMMC channel structure.
   \param  pFaxStatus   Pointer to IFX_TAPI_T38_STATUS_t structure.

   \return
   - VMMC_statusOk in all cases
*/
IFX_int32_t VMMC_TAPI_LL_COD_T38_Status_Get (IFX_TAPI_LL_CH_t *pLLChannel,
                                      IFX_TAPI_T38_STATUS_t *pFaxStatus)
{
   VMMC_CHANNEL *pCh = (VMMC_CHANNEL *)pLLChannel;

   /* read status from cache structure */
   *pFaxStatus = pCh->TapiFaxStatus;

   return VMMC_statusOk;
}


/**
  Set Fax Error Status in the LL channel structure.

   \param  pLLChannel   Pointer to the VMMC channel structure.
   \param  error        Error value.

   \return
   - VMMC_statusOk in all cases
*/
IFX_int32_t VMMC_TAPI_LL_COD_T38_Error_Set (IFX_TAPI_LL_CH_t *pLLChannel,
                                      unsigned char error)
{
   VMMC_CHANNEL *pCh = (VMMC_CHANNEL *)pLLChannel;

   /* set status to cache structure */
   pCh->TapiFaxStatus.nError = error;

   return VMMC_statusOk;
}


/**
   Store current T.38 Fax status in the LL channel structure.

   \param  pLLChannel   Pointer to the VMMC channel structure.
   \param  status       Status value.

   \return
   - VMMC_statusOk in all cases
*/
IFX_int32_t VMMC_TAPI_LL_COD_T38_Status_Set (IFX_TAPI_LL_CH_t *pLLChannel,
                                      unsigned char status)
{
   VMMC_CHANNEL *pCh = (VMMC_CHANNEL *)pLLChannel;

   /* set status to cache structure */
   pCh->TapiFaxStatus.nStatus = status;

   return VMMC_statusOk;
}
#endif /* VMMC_FEAT_FAX_T38 */

/**
   Get AMR codec parameters: fill in the pAMR structure.

   \param  pLLCh        Pointer to the VMMC channel structure.
   \param  pAMR         Pointer to IFX_TAPI_COD_AMR_t structure.

   \return
   - VMMC_statusNotSupported if AMR is not supported by voice FW
   - VMMC_statusOk in other cases
*/
IFX_int32_t VMMC_TAPI_LL_COD_AMR_Get (IFX_TAPI_LL_CH_t *pLLCh,
                                      IFX_TAPI_COD_AMR_t *pAMR)
{
   VMMC_CHANNEL *pCh    = (VMMC_CHANNEL *)pLLCh;
   VMMC_DEVICE  *pDev   = pCh->pParent;

   if (!(pDev->caps.CODECS & CODEC_AMR_NB))
   {
      RETURN_STATUS (VMMC_statusNotSupported);
   }
   pAMR->DecCMR = pCh->pCOD->AMR_DecCMR;

   RETURN_STATUS(VMMC_statusOk);
}

/**
   Update AMR decoded CMR field.
   This function is called from interrupt handler.

   \param  pCh          Pointer to the VMMC_CHANNEL structure.
   \param  CmrDec       Value from FW message.

*/
IFX_void_t VMMC_COD_CmrDec_Update (VMMC_CHANNEL *pCh,
                                   IFX_uint8_t CmrDec)
{
   pCh->pCOD->AMR_DecCMR = CmrDec;
}

/**
   Basic CODER Module configuration

   Use this function where needed to set the base configuration
   of the Coder Module. This function isn't an IOCTL function
   This function configures:
      - CODER Module
      - all CODER Channels
      - all CODER Jitter Buffers
      - Coder Channel decoder status and Coder Channel profiles in case of AAL

   \param  pCh          Pointer to the VMMC channel structure.

   \return
    VMMC_statusOk if no error, otherwise VMMC_statusErr
*/
IFX_int32_t VMMC_COD_baseConf (VMMC_CHANNEL *pCh)
{
   IFX_int32_t ret = VMMC_statusOk;
   VMMC_DEVICE      *pDev   = pCh->pParent;
   IFX_TAPI_JB_CFG_t jbCfg;

   /* Coder Channel control configuration */
   /* set all data fields/bits in the fw message */
   pCh->pCOD->fw_cod_ch_speech.EN    = 0;
   pCh->pCOD->fw_cod_ch_speech.DEC   = 0;
   pCh->pCOD->fw_cod_ch_speech.ENC   = COD_CHAN_SPEECH_ENC_NO;
   pCh->pCOD->fw_cod_ch_speech.GAIN1 = VMMC_GAIN_0DB;
   pCh->pCOD->fw_cod_ch_speech.GAIN2 = VMMC_GAIN_0DB;
   pCh->pCOD->fw_cod_ch_speech.BFI   = 1;
   pCh->pCOD->fw_cod_ch_speech.CNG   = 1;
   pCh->pCOD->fw_cod_ch_speech.HP    = 1;
   pCh->pCOD->fw_cod_ch_speech.PF    = 1;
   pCh->pCOD->fw_cod_ch_speech.IM    = 1;
   pCh->pCOD->fw_cod_ch_speech.PST   = 1;
   pCh->pCOD->fw_cod_ch_speech.NS    = 1;
   pCh->pCOD->fw_cod_ch_speech.SIC   = 1;
   pCh->pCOD->fw_cod_ch_speech.PTE   = COD_CHAN_SPEECH_PTE_10;
   pCh->pCOD->fw_cod_ch_speech.FEC   = 0;
   pCh->pCOD->fw_cod_ch_speech.RED   = 0;
   pCh->pCOD->fw_cod_ch_speech.EE    = 0;
   pCh->pCOD->fw_cod_ch_speech.DE    = 0;
   pCh->pCOD->fw_cod_ch_speech.PLC   = 0;
   pCh->pCOD->fw_cod_ch_speech.TSF   = 0;
   /* store the encoder till the coder-module gets enabled and unmuted */
   pCh->pCOD->enc_conf               = COD_CHAN_SPEECH_ENC_G711_MLAW;
   pCh->pCOD->pte_conf               = COD_CHAN_SPEECH_PTE_10;
   pCh->pCOD->bitr_conf              = 0;
   pCh->pCOD->fec_conf               = 0;
   pCh->pCOD->sc_bit                 = 1;
   pCh->pCOD->enc_running            = IFX_FALSE;
   pCh->pCOD->curr_dec               = COD_CHAN_SPEECH_ENC_NO;

   /* Set the inputs in the cached message */
   VMMC_COD_Set_Inputs (pCh);
   /* VMMC_COD_Set_Inputs does not write because EN is 0 so write here */
   ret = CmdWrite (pDev, (IFX_uint32_t *) &(pCh->pCOD->fw_cod_ch_speech),
                   COD_CHAN_SPEECH_LEN);

   /* Configure coder channel decoder status to issue interrupts on change of
      Decoder (DC) or Packet Time (PTC) */
   if (VMMC_SUCCESS(ret))
   {
      pCh->pCOD->fw_cod_dec_stat.DC     = 1;
      pCh->pCOD->fw_cod_dec_stat.PTC    = 1;
      pCh->pCOD->fw_cod_dec_stat.CMRC   = 1;
      ret = CmdWrite (pDev, (IFX_uint32_t *) &(pCh->pCOD->fw_cod_dec_stat),
                      COD_DEC_STAT_LEN);
   }

   /* Jitter Buffer Default configuration (adaptive/voice) */
   memset (&jbCfg, 0, sizeof(IFX_TAPI_JB_CFG_t));
   jbCfg.nJbType      = JB_CONFIG_TYPE;
   jbCfg.nPckAdpt     = JB_CONFIG_PKT_ADAPT;
   jbCfg.nScaling     = JB_CONFIG_SCALING;
   jbCfg.nInitialSize = JB_CONFIG_INITIAL_SIZE;
   jbCfg.nMinSize     = JB_CONFIG_MINIMUM_SIZE;
   jbCfg.nMaxSize     = JB_CONFIG_MAXIMUM_SIZE;
   VMMC_TAPI_LL_COD_JB_Cfg(pCh, &jbCfg);

#ifdef VMMC_FEAT_FAX_T38
   /* FAX control */
   pCh->pCOD->fw_cod_fax_ctrl.GAIN1 = VMMC_GAIN_0DB;
   pCh->pCOD->fw_cod_fax_ctrl.GAIN2 = VMMC_GAIN_0DB;
   /* all to reset value of 2 ms */
   pCh->pCOD->fw_cod_fax_ctrl.MOBRD = 3;
   pCh->pCOD->fw_cod_fax_ctrl.MOBSM = 3;
   pCh->pCOD->fw_cod_fax_ctrl.DMBSD = 3;
#endif /* VMMC_FEAT_FAX_T38 */

   return ret;
}


/**
   Initalize the coder module and the cached firmware messages

   \param  pCh          Pointer to the VMMC channel structure.

   \return
      - VMMC_statusCmdWr Writing the command failed
      - VMMC_statusOk if successful
   \remarks memset is done in VMMC_COD_Allocate_Ch_Structures already
*/
IFX_int32_t VMMC_COD_InitCh (VMMC_CHANNEL *pCh)
{
   VMMC_CODCH_t *pCod = pCh->pCOD;
   IFX_uint8_t  ch = pCh->nChannel - 1;
   int ret = VMMC_statusOk;

   /* Coder Channel control */
   memset(&pCod->fw_cod_ch_speech, 0, sizeof(pCod->fw_cod_ch_speech));
   pCod->fw_cod_ch_speech.CMD    = CMD_EOP;
   pCod->fw_cod_ch_speech.CHAN   = ch;
   pCod->fw_cod_ch_speech.MOD    = MOD_CODER;
   pCod->fw_cod_ch_speech.ECMD   = COD_CHAN_SPEECH_ECMD;
   pCod->fw_cod_ch_speech.CODNR  = ch;

   /* Coder RTP Timestamp configuration */
   memset(&pCod->fw_cod_rtp, 0, sizeof(pCod->fw_cod_rtp));
   pCod->fw_cod_rtp.CMD          = CMD_EOP;
   pCod->fw_cod_rtp.CHAN         = ch;
   pCod->fw_cod_rtp.MOD          = MOD_CODER;
   pCod->fw_cod_rtp.ECMD         = COD_CHAN_RTP_TIMESTAMP_ECMD;

   /* Coder RTP Support Upstream configuration */
   memset(&pCod->fw_cod_rtp_us_conf, 0, sizeof(pCod->fw_cod_rtp_us_conf));
   pCod->fw_cod_rtp_us_conf.CMD  = CMD_EOP;
   pCod->fw_cod_rtp_us_conf.CHAN = ch;
   pCod->fw_cod_rtp_us_conf.MOD  = MOD_CODER;
   pCod->fw_cod_rtp_us_conf.ECMD = COD_CHAN_RTP_SUP_CFG_US_ECMD;

   /* Coder RTP Support Downstream configuration */
   memset(&pCod->fw_cod_rtp_ds_conf, 0, sizeof(pCod->fw_cod_rtp_ds_conf));
   pCod->fw_cod_rtp_ds_conf.CMD  = CMD_EOP;
   pCod->fw_cod_rtp_ds_conf.CHAN = ch;
   pCod->fw_cod_rtp_ds_conf.MOD  = MOD_CODER;
   pCod->fw_cod_rtp_ds_conf.ECMD = COD_CHAN_RTP_SUP_CFG_DS_ECMD;

   /* Coder Decoder Status */
   memset(&pCod->fw_cod_dec_stat, 0, sizeof(pCod->fw_cod_dec_stat));
   pCod->fw_cod_dec_stat.CMD     = CMD_EOP;
   pCod->fw_cod_dec_stat.CHAN    = ch;
   pCod->fw_cod_dec_stat.MOD     = MOD_CODER;
   pCod->fw_cod_dec_stat.ECMD    = COD_DEC_STAT_ECMD;

   /* RTP Event Generation */
   memset(&pCod->fw_cod_evt_gen, 0, sizeof(pCod->fw_cod_evt_gen));
   pCod->fw_cod_evt_gen.CMD      = CMD_EOP;
   pCod->fw_cod_evt_gen.CHAN     = ch;
   pCod->fw_cod_evt_gen.MOD      = MOD_SIGNALING;
   pCod->fw_cod_evt_gen.ECMD     = COD_EVT_GEN_ECMD;

   /* RTCP Statistics */
   memset(&pCod->fw_cod_rtcp_stat, 0, sizeof(pCod->fw_cod_rtcp_stat));
   pCod->fw_cod_rtcp_stat.CMD    = CMD_EOP;
   pCod->fw_cod_rtcp_stat.CHAN   = ch;
   pCod->fw_cod_rtcp_stat.MOD    = MOD_CODER;
   pCod->fw_cod_rtcp_stat.ECMD   = COD_RTCP_SUP_CTRL_ECMD;

#ifdef VMMC_FEAT_RTCP_XR
   /* RTCP XR Statistics Summary Report Block */
   memset(&pCod->fw_cod_rtcp_xr_stat_sum, 0, sizeof(pCod->fw_cod_rtcp_xr_stat_sum));
   pCod->fw_cod_rtcp_xr_stat_sum.CMD    = CMD_RTCP_XR;
   pCod->fw_cod_rtcp_xr_stat_sum.CHAN   = ch;
   pCod->fw_cod_rtcp_xr_stat_sum.MOD    = MOD_CODER;
   pCod->fw_cod_rtcp_xr_stat_sum.ECMD   = COD_RTCP_XR_STAT_SUM_ECMD;

   /* RTCP XR VoIP Metrics Report Block */
   memset(&pCod->fw_cod_rtcp_xr_voip_met, 0, sizeof(pCod->fw_cod_rtcp_xr_voip_met));
   pCod->fw_cod_rtcp_xr_voip_met.CMD    = CMD_RTCP_XR;
   pCod->fw_cod_rtcp_xr_voip_met.CHAN   = ch;
   pCod->fw_cod_rtcp_xr_voip_met.MOD    = MOD_CODER;
   pCod->fw_cod_rtcp_xr_voip_met.ECMD   = COD_RTCP_XR_VOIP_MET_ECMD;

   /* RTCP XR Associated LEC Resource Number */
   memset(&pCod->fw_cod_associated_lecnr, 0, sizeof(pCod->fw_cod_associated_lecnr));
   pCod->fw_cod_associated_lecnr.CMD    = CMD_RTCP_XR;
   pCod->fw_cod_associated_lecnr.CHAN   = ch;
   pCod->fw_cod_associated_lecnr.MOD    = MOD_CODER;
   pCod->fw_cod_associated_lecnr.ECMD   = COD_ASSOCIATED_LECNR_ECMD;
#endif /* VMMC_FEAT_RTCP_XR */

   /* AGC control */
   memset(&pCod->fw_cod_agc_ctrl, 0, sizeof(pCod->fw_cod_agc_ctrl));
   pCod->fw_cod_agc_ctrl.CMD     = CMD_EOP;
   pCod->fw_cod_agc_ctrl.CHAN    = ch;
   pCod->fw_cod_agc_ctrl.MOD     = MOD_CODER;
   pCod->fw_cod_agc_ctrl.ECMD    = COD_AGC_CTRL_ECMD;
   pCod->fw_cod_agc_ctrl.AGCNR   = ch;

   /* AGC coefficients*/
   memset(&pCod->fw_res_agc_coeff, 0, sizeof(pCod->fw_res_agc_coeff));
   pCod->fw_res_agc_coeff.CMD    = CMD_EOP;
   pCod->fw_res_agc_coeff.CHAN   = ch;
   pCod->fw_res_agc_coeff.MOD    = MOD_RESOURCE;
   pCod->fw_res_agc_coeff.ECMD   = RES_AGC_COEF_ECMD;

   /* JB configuration */
   memset(&pCod->fw_cod_jb_conf, 0, sizeof(pCod->fw_cod_jb_conf));
   pCod->fw_cod_jb_conf.CMD      = CMD_EOP;
   pCod->fw_cod_jb_conf.CHAN     = ch;
   pCod->fw_cod_jb_conf.MOD      = MOD_CODER;
   pCod->fw_cod_jb_conf.ECMD     = COD_JB_CONF_ECMD;

   /* JB statistics */
   memset(&pCod->fw_cod_jb_stat, 0, sizeof(pCod->fw_cod_jb_stat));
   pCod->fw_cod_jb_stat.CMD      = CMD_EOP;
   pCod->fw_cod_jb_stat.CHAN     = ch;
   pCod->fw_cod_jb_stat.MOD      = MOD_CODER;
   pCod->fw_cod_jb_stat.ECMD     = COD_JB_STAT_ECMD;

   /* Read the current set of AGC coefficients. On first configuration the
      default set of coefficient is read back. */
   if (ch < pCh->pParent->caps.nAGC)
   {
      ret = CmdRead(pCh->pParent,
                    (IFX_uint32_t *)&pCod->fw_res_agc_coeff,
                    (IFX_uint32_t *)&pCod->fw_res_agc_coeff,
                     RES_AGC_COEF_LEN);
   }

#ifdef VMMC_FEAT_FAX_T38
   /* FAX control */
   memset(&pCod->fw_cod_fax_ctrl, 0, sizeof(pCod->fw_cod_fax_ctrl));
   pCod->fw_cod_fax_ctrl.CMD     = CMD_EOP;
   pCod->fw_cod_fax_ctrl.CHAN    = ch;
   pCod->fw_cod_fax_ctrl.MOD     = MOD_CODER;
   pCod->fw_cod_fax_ctrl.ECMD    = COD_FAX_CTRL_ECMD;
#endif /* VMMC_FEAT_FAX_T38 */

#ifdef VMMC_FEAT_FAX_T38_FW
   /* T.38 channel control */
   memset (&pCod->fw_cod_fax_act, 0x00, sizeof (pCod->fw_cod_fax_act));
   pCod->fw_cod_fax_act.CMD      = CMD_EOP;
   pCod->fw_cod_fax_act.CHAN     = ch;
   pCod->fw_cod_fax_act.MOD      = MOD_CODER;
   pCod->fw_cod_fax_act.ECMD     = COD_FAX_ACT_ECMD;
   pCod->fw_cod_fax_act.T38R     = ch;

   /* T.38 FAX configuration */
   memset (&pCod->fw_cod_fax_conf, 0x00, sizeof (pCod->fw_cod_fax_conf));
   pCod->fw_cod_fax_conf.CMD     = CMD_EOP;
   pCod->fw_cod_fax_conf.CHAN    = ch;
   pCod->fw_cod_fax_conf.MOD     = MOD_CODER;
   pCod->fw_cod_fax_conf.ECMD    = COD_FAX_CONF_ECMD;

   /* T.38 FDP configuration */
   memset (&pCod->fw_cod_fax_fdp_params, 0x00,
           sizeof (pCod->fw_cod_fax_fdp_params));
   pCod->fw_cod_fax_fdp_params.CMD     = CMD_EOP;
   pCod->fw_cod_fax_fdp_params.CHAN    = ch;
   pCod->fw_cod_fax_fdp_params.MOD     = MOD_CODER;
   pCod->fw_cod_fax_fdp_params.ECMD    = COD_FAX_FDP_PARAMS_ECMD;

   /* T.38 session statistics */
   memset (&pCod->fw_cod_fax_stat, 0x00, sizeof (pCod->fw_cod_fax_stat));
   pCod->fw_cod_fax_stat.CMD     = CMD_EOP;
   pCod->fw_cod_fax_stat.CHAN    = ch;
   pCod->fw_cod_fax_stat.MOD     = MOD_CODER;
   pCod->fw_cod_fax_stat.ECMD    = COD_FAX_STAT_ECMD;

   /* T.38 session trace */
   memset (&pCod->fw_cod_fax_trace, 0x00, sizeof (pCod->fw_cod_fax_trace));
   pCod->fw_cod_fax_trace.CMD    = CMD_EOP;
   pCod->fw_cod_fax_trace.CHAN   = ch;
   pCod->fw_cod_fax_trace.MOD    = MOD_CODER;
   pCod->fw_cod_fax_trace.ECMD   = COD_FAX_TRACE_ECMD;
#endif /* VMMC_FEAT_FAX_T38_FW */

#ifdef VMMC_FEAT_ANNOUNCEMENTS
   memset(&pCod->fw_cod_ann_ctrl, 0, sizeof(pCod->fw_cod_ann_ctrl));
   pCod->fw_cod_ann_ctrl.CMD     = CMD_EOP;
   pCod->fw_cod_ann_ctrl.CHAN    = ch;
   pCod->fw_cod_ann_ctrl.MOD     = MOD_CODER;
   pCod->fw_cod_ann_ctrl.ECMD    = COD_ANN_CTRL_ECMD;
   pCod->fw_cod_ann_ctrl.E       = COD_ANN_CTRL_E_STOP;
#endif /* VMMC_FEAT_ANNOUNCEMENTS */

   /* Coder Channel MOS_CQE Support Configuration */
   memset(&pCod->fw_cod_cfg_stat_mos, 0, sizeof(pCod->fw_cod_cfg_stat_mos));
   pCod->fw_cod_cfg_stat_mos.CMD    = CMD_EOP;
   pCod->fw_cod_cfg_stat_mos.CHAN   = ch;
   pCod->fw_cod_cfg_stat_mos.MOD    = MOD_CODER;
   pCod->fw_cod_cfg_stat_mos.ECMD   = CMD_COD_CFG_STAT_MOS_ECMD;

   memset(&pCod->fw_cod_read_stat_mos, 0, sizeof(pCod->fw_cod_read_stat_mos));
   pCod->fw_cod_read_stat_mos.CMD   = CMD_EOP;
   pCod->fw_cod_read_stat_mos.CHAN  = ch;
   pCod->fw_cod_read_stat_mos.MOD   = MOD_CODER;
   pCod->fw_cod_read_stat_mos.ECMD  = CMD_COD_READ_STAT_MOS_ECMD;

   VMMC_CON_Init_CodCh (pCh);

   RETURN_STATUS (ret);
}


/**
   Set the signal inputs of the cached fw message for the given channel
  \param  pCh          Pointer to the VMMC channel structure.
  \return  IFX_SUCCESS or IFX_ERROR
*/
IFX_int32_t VMMC_COD_Set_Inputs (VMMC_CHANNEL *pCh)
{
   COD_CHAN_SPEECH_t *p_fw_cod_ch_speech;
   int               ret = VMMC_statusOk;

   /* update the signal inputs of this cached msg */
   p_fw_cod_ch_speech = &pCh->pCOD->fw_cod_ch_speech;

   VMMC_OS_MutexGet (&pCh->chAcc);
   p_fw_cod_ch_speech->I1 = VMMC_CON_Get_COD_SignalInput (pCh, 0);
   p_fw_cod_ch_speech->I2 = VMMC_CON_Get_COD_SignalInput (pCh, 1);
   p_fw_cod_ch_speech->I3 = VMMC_CON_Get_COD_SignalInput (pCh, 2);
   p_fw_cod_ch_speech->I4 = VMMC_CON_Get_COD_SignalInput (pCh, 3);
   p_fw_cod_ch_speech->I5 = VMMC_CON_Get_COD_SignalInput (pCh, 4);

   /* Write the updated cached message to fw only if channel is running */
   if (p_fw_cod_ch_speech->EN)
   {
      ret = CmdWrite (pCh->pParent, (IFX_uint32_t *)p_fw_cod_ch_speech,
                      COD_CHAN_SPEECH_LEN);
   }

   VMMC_OS_MutexRelease (&pCh->chAcc);

   return ret;
}


/**
   Stop COD on this channel

   \param  pCh             Pointer to the VMMC channel structure.

   \return
   - VMMC_statusOk         If successful
   - VMMC_statusCmdWr      Writing the command has failed
*/
IFX_int32_t VMMC_COD_ChStop (VMMC_CHANNEL *pCh)
{
   IFX_int32_t       ret   = VMMC_statusOk;

   /* calling function should ensure valid parameters */
   VMMC_ASSERT(pCh != IFX_NULL);

   if (pCh->pCOD != IFX_NULL)
   {
      VMMC_OS_MutexGet (&pCh->chAcc);
      ret = vmmc_cod_Voice_Enable (pCh, 0);
      VMMC_OS_MutexRelease (&pCh->chAcc);
   }

   RETURN_STATUS(ret);
}


/**
   RTP settings for Coder channel

   \param  pCh          Pointer to the VMMC channel structure.
   \param  pRtpConf     RTP configuration data passed from TAPI.
*/
IFX_int32_t vmmc_cod_RTP_Cfg (VMMC_CHANNEL *pCh,
                              IFX_TAPI_PKT_RTP_CFG_t const *pRtpConf)
{
   VMMC_DEVICE *pDev = pCh->pParent;
   IFX_int32_t ret = VMMC_statusOk;

   /* Set SSRC and Sequence Number (RTP Upstream Configuration) in COD channel.
      This is set unconditionally because the sequence number is increased
      autonomously in the FW and reading before setting it just wastes time. */
   pCh->pCOD->fw_cod_rtp_us_conf.SSRC   = pRtpConf->nSsrc;
   pCh->pCOD->fw_cod_rtp_us_conf.SEQ_NR = pRtpConf->nSeqNr;
   ret = CmdWrite (pDev, (IFX_uint32_t *)&pCh->pCOD->fw_cod_rtp_us_conf, 8);

   if (VMMC_SUCCESS(ret))
   {
      /* store the SSRC for use in RTCP statistic get */
      pCh->pCOD->nSsrc  = pRtpConf->nSsrc;
   }

   /* set time stamp (RTP Timestamp Configuration)
      The timestamp value is continously increased when the channel is
      enabled. So it is very unlikely that the cached value is still the
      one in the cache variable. So we ignore the cache and unconditionally
      write the timestamp every time we get here. */
   if (VMMC_SUCCESS(ret))
   {
      pCh->pCOD->fw_cod_rtp.TIME_STAMP     = pRtpConf->nTimestamp;
      ret = CmdWrite (pDev, (IFX_uint32_t *) &pCh->pCOD->fw_cod_rtp,
                      sizeof(pCh->pCOD->fw_cod_rtp) - CMD_HDR_CNT);
   }

   return ret;
}


/**
  Get the Coder channel status information.

  \param  pCh          Pointer to the VMMC channel structure.

  \return
  -IFX_ENABLE if coder is enabled
  -IFX_DISABLE if the coder is disabled
*/
IFX_enDis_t VMMC_COD_ChStatusGet (VMMC_CHANNEL *pCh)
{
   if ((pCh->pCOD != IFX_NULL) &&
       (pCh->pCOD->fw_cod_ch_speech.EN == COD_CHAN_SPEECH_ENABLE))
   {
      return IFX_ENABLE;
   }

   return IFX_DISABLE;
}


/**
   Allocate data structure of the COD module in the given channel

   \param  pCh          Pointer to the VMMC channel structure.

   \return
      - VMMC_statusOk if ok
      - VMMC_statusNoMem Memory allocation failed -> struct not created

   \remarks The channel parameter is no longer checked because the calling
   function assures correct values.
*/
IFX_int32_t VMMC_COD_Allocate_Ch_Structures (VMMC_CHANNEL *pCh)
{
   VMMC_COD_Free_Ch_Structures (pCh);

   pCh->pCOD = VMMC_OS_Malloc (sizeof(VMMC_CODCH_t));
   if (pCh->pCOD == NULL)
   {
      /* errmsg: Memory allocation failed */
      RETURN_STATUS (VMMC_statusNoMem);
   }
   memset(pCh->pCOD, 0, sizeof(VMMC_CODCH_t));
#ifdef EVALUATION
   if (VMMC_Eval_COD_Allocate_Ch_Structures (pCh) != VMMC_statusOk)
   {
      RETURN_STATUS(VMMC_statusDevInitFail);
   }
#endif /* #ifdef EVALUATION */

   return VMMC_statusOk;
}


/**
   Free data structure of the COD module in the given channel.

   \param  pCh             Pointer to the VMMC channel structure.
*/
IFX_void_t VMMC_COD_Free_Ch_Structures (VMMC_CHANNEL *pCh)
{
   if (pCh->pCOD != IFX_NULL)
   {
#ifdef EVALUATION
      VMMC_Eval_COD_Free_Ch_Structures (pCh);
#endif /* #ifdef EVALUATION */
      VMMC_OS_Free (pCh->pCOD);
      pCh->pCOD = IFX_NULL;
   }
}


/**
   Configure or check COD module for given sampling mode.

   \param  pCh             Pointer to the VMMC channel structure.
   \param  action          Action to be executed (set or check).
   \param  mode            Signalling array operation mode (16kHz or 8 kHz).

   \return
   If action is SM_SET: IFX_SUCCESS or IFX_ERROR.
   If action is SM_CHECK: IFX_TRUE when module would do a switch or IFX_FALSE
                          if nothing needs to be done.

   \remarks
   Coder channel should be muted before NB/WB configuration.
*/
IFX_int32_t VMMC_COD_SamplingMode (VMMC_CHANNEL *pCh,
                                   SM_ACTION action,
                                   OPMODE_SMPL mode)
{
   VMMC_DEVICE       *pDev = pCh->pParent;
   VMMC_CODCH_t      *pCodCh;
   COD_CHAN_SPEECH_t *p_fw_cod_ch_speech;
   IFX_boolean_t     bUpdate = IFX_FALSE;
   IFX_int32_t       change_isr;
   IFX_int32_t       ret = VMMC_statusOk;
   COD_AGC_CTRL_t    *pAgcCtrl;
   IFX_boolean_t     bRestoreAGC = IFX_FALSE;

   /* calling function should ensure valid parameters */
   VMMC_ASSERT(pCh != NULL);

   pCodCh = pCh->pCOD;
   p_fw_cod_ch_speech = &(pCodCh->fw_cod_ch_speech);
   pAgcCtrl = &pCh->pCOD->fw_cod_agc_ctrl;

   /* determine if COD module already operates in requested sampling mode */
   change_isr = ((mode == WB_16_KHZ) && (p_fw_cod_ch_speech->ISR == 0)) ||
                ((mode == NB_8_KHZ)  && (p_fw_cod_ch_speech->ISR == 1));

   /* If action is check return if module needs a switch */
   if (action != SM_SET)
   {
      return change_isr;
   }

   /* Action is set */

   /* protect fw msg */
   VMMC_OS_MutexGet (&pCh->chAcc);

   /* Update also the ENC and PTE field if the configuration has changed them.
      This can be done only here where also the ISR is set because we may
      never set a WB coder algorithm while ISR is 0. Because also other
      calls use the cached fw message and write them we keep the setting
      in separate variables and write them only in this place. */
   if (pCodCh->enc_running == IFX_TRUE)
   {
      if ((p_fw_cod_ch_speech->ENC != pCodCh->enc_conf) ||
          (p_fw_cod_ch_speech->PTE != pCodCh->pte_conf) ||
          (p_fw_cod_ch_speech->BITR != pCodCh->bitr_conf) ||
          (p_fw_cod_ch_speech->FEC != pCodCh->fec_conf))
      {
         /* Force an update because at least ENC or PTE changed. */
         bUpdate = IFX_TRUE;
      }

      if (IFX_TRUE == bUpdate)
      {
         IFX_uint32_t      nNewTSF;

         /* Set TSF (time stamp frequency) according to the selected encoder.
            TSF = 1 for all wideband encoders except G.722.
            TSF = 0 for all narrowband encoders and G.722.
            ITU-T standard G.722 defines TSF=0 which is technically an error in
            this standard. Anyhow we set TSF as stipulated in this standard. */
         if ((pCodCh->enc_conf == COD_CHAN_SPEECH_ENC_G7221_24) ||
             (pCodCh->enc_conf == COD_CHAN_SPEECH_ENC_G7221_32) ||
             (pCodCh->enc_conf == COD_CHAN_SPEECH_ENC_LIN16_16KHZ) ||
             /* Decoding of encoder depends on AMRE field. */
             ((pCodCh->enc_conf == COD_CHAN_SPEECH_ENC_AMR_WB) && pDev->caps.bAMRE))
         {
            nNewTSF = 1;
         }
         else
         {
            nNewTSF = 0;
         }

         if (p_fw_cod_ch_speech->TSF != nNewTSF)
         {
            /* To change the timestamp frequency the channel needs to be
               deactivated. */
            bRestoreAGC = pAgcCtrl->EN ? IFX_TRUE : IFX_FALSE;
            ret =  vmmc_cod_Voice_Enable (pCh, 0);
            p_fw_cod_ch_speech->EN = COD_CHAN_SPEECH_ENABLE;
            p_fw_cod_ch_speech->TSF = nNewTSF;
         }

         /* Set the ENC and PTE after a possible deactivate done just above. */
         p_fw_cod_ch_speech->ENC = pCodCh->enc_conf;
         p_fw_cod_ch_speech->PTE = pCodCh->pte_conf;
         p_fw_cod_ch_speech->BITR = pCodCh->bitr_conf;
         p_fw_cod_ch_speech->FEC = pCodCh->fec_conf;

         /* The SIC bit must be set to 0 in case of G.728, G.729E, and iLBC.
            Also G.711 VBD does not allow silence compression. */
         if ((p_fw_cod_ch_speech->ENC == COD_CHAN_SPEECH_ENC_G728_16) ||
             (p_fw_cod_ch_speech->ENC == COD_CHAN_SPEECH_ENC_G729E_11_8) ||
             (p_fw_cod_ch_speech->ENC == COD_CHAN_SPEECH_ENC_ILBC_13_3) ||
             (p_fw_cod_ch_speech->ENC == COD_CHAN_SPEECH_ENC_ILBC_15_2) ||
             (p_fw_cod_ch_speech->ENC == COD_CHAN_SPEECH_ENC_G711_ALAW_VBD) ||
             (p_fw_cod_ch_speech->ENC == COD_CHAN_SPEECH_ENC_G711_MLAW_VBD) )
         {
            p_fw_cod_ch_speech->SIC = 0;
         }
         else
         {
            p_fw_cod_ch_speech->SIC = pCodCh->sc_bit;
         }
      }
   }
   else
   {
      if (p_fw_cod_ch_speech->ENC != COD_CHAN_SPEECH_ENC_NO)
      {
         p_fw_cod_ch_speech->ENC = COD_CHAN_SPEECH_ENC_NO;
         bUpdate = IFX_TRUE;
         /* RTP event support requires both encoder and decoder running.
            Turn off the event transmission */
         ret = VMMC_SIG_UpdateEventTrans(pCh, IFX_FALSE);
      }
   }

   /* Now change the ISR if needed. */
   if( change_isr )
   {
      /* Switch coder channel's ISR bit according to the requested
         sampling operation mode (8 or 16 kHz) */
      p_fw_cod_ch_speech->ISR = !(p_fw_cod_ch_speech->ISR);
      TRACE(VMMC, DBG_LEVEL_LOW,
            ("Set COD channel %u ISR = %d\n",
            pCh->nChannel - 1, p_fw_cod_ch_speech->ISR));
      bUpdate = IFX_TRUE;
   }
   else
   {
      TRACE(VMMC, DBG_LEVEL_LOW,
            ("Sampling rate of COD on channel %u already matching\n",
            pCh->nChannel - 1));
   }

   /* Write only to FW if COD is enabled and something has changed */
   if (p_fw_cod_ch_speech->EN && bUpdate &&
       (ret == /*lint --e(774) */ VMMC_statusOk))
   {
      ret = CmdWrite(pDev, (IFX_uint32_t*)p_fw_cod_ch_speech ,
                     sizeof(COD_CHAN_SPEECH_t)- CMD_HDR_CNT);

      /* restore event support in SIG channel */
      if (ret == VMMC_statusOk &&
          pCh->pCOD->fw_cod_ch_speech.ENC != COD_CHAN_SPEECH_ENC_NO &&
          pCh->pCOD->fw_cod_ch_speech.DEC != COD_CHAN_SPEECH_DEC_INACTIVE)
      {
         ret = VMMC_SIG_UpdateEventTrans(pCh, IFX_TRUE);
      }

      /* restore AGC */
      if (ret == VMMC_statusOk && bRestoreAGC != IFX_FALSE)
      {
         ret = vmmc_cod_AGC_Enable (pCh, IFX_TAPI_ENC_AGC_MODE_ENABLE);
      }
   }

   VMMC_OS_MutexRelease (&pCh->chAcc);

   return ret;
}


/**
   Function that fills in the COD module function pointers in the driver
   context structure which is passed to HL TAPI during registration.

   \param  pCOD         Pointer to the COD part in the driver context struct.
*/
IFX_void_t VMMC_COD_Func_Register (IFX_TAPI_DRV_CTX_COD_t *pCOD)
{
   pCOD->ENC_Start               = VMMC_TAPI_LL_COD_ENC_Start;
   pCOD->ENC_Stop                = VMMC_TAPI_LL_COD_ENC_Stop;
   pCOD->ENC_Hold                = VMMC_TAPI_LL_COD_ENC_Hold;
   pCOD->ENC_Cfg                 = VMMC_TAPI_LL_COD_ENC_Cfg_Set;

   pCOD->DEC_Start               = VMMC_TAPI_LL_COD_DEC_Start;
   pCOD->DEC_Stop                = VMMC_TAPI_LL_COD_DEC_Stop;
   pCOD->DEC_Cfg                 = VMMC_TAPI_LL_COD_DEC_Cfg_Set;
   pCOD->DEC_HP_Set              = VMMC_TAPI_LL_COD_DEC_HP_Set;
   pCOD->DEC_Chg_Evt_Detail_Req  = VMMC_TAPI_LL_COD_DEC_ChgDetailReq;

   pCOD->Volume_Set              = VMMC_TAPI_LL_COD_Volume_Set;

   pCOD->VAD_Cfg                 = VMMC_TAPI_LL_COD_VAD_Cfg;

   pCOD->AGC_Cfg                 = VMMC_TAPI_LL_COD_AGC_Cfg;
   pCOD->AGC_Enable              = VMMC_TAPI_LL_COD_AGC_Enable;

   pCOD->JB_Cfg                  = VMMC_TAPI_LL_COD_JB_Cfg;
   pCOD->JB_Stat_Reset           = VMMC_TAPI_LL_COD_JB_Stat_Reset;
   pCOD->JB_Stat_Get             = VMMC_TAPI_LL_COD_JB_Stat_Get;

   pCOD->RTCP_Get                = VMMC_TAPI_LL_COD_RTCP_Get;
   pCOD->RTCP_Reset              = VMMC_TAPI_LL_COD_RTCP_Reset;

#ifdef VMMC_FEAT_RTCP_XR
   pCOD->RTCP_XR_Get             = VMMC_TAPI_LL_COD_RTCP_XR_Get;
#endif /* VMMC_FEAT_RTCP_XR */

   pCOD->RTP_PayloadTable_Cfg    = VMMC_TAPI_LL_COD_RTP_PayloadTable_Cfg;
   pCOD->RTP_Cfg                 = VMMC_TAPI_LL_COD_RTP_Cfg;
   pCOD->RTP_EV_Generate         = VMMC_TAPI_LL_COD_RTP_EventGenerate;

   pCOD->MOS_Cfg                 = VMMC_TAPI_LL_COD_MOS_Cfg;
   pCOD->MOS_Result_Get          = VMMC_TAPI_LL_COD_MOS_Result_Get;

#ifdef VMMC_FEAT_FAX_T38
   pCOD->T38_Mod_Enable          = VMMC_TAPI_LL_COD_T38_Mod_Enable;
   pCOD->T38_DeMod_Enable        = VMMC_TAPI_LL_COD_T38_DeMod_Enable;
   pCOD->T38_Datapump_Disable    = VMMC_TAPI_LL_COD_T38_Datapump_Disable;
   pCOD->T38_Status_Get          = VMMC_TAPI_LL_COD_T38_Status_Get;
   pCOD->T38_Status_Set          = VMMC_TAPI_LL_COD_T38_Status_Set;
   pCOD->T38_Error_Set           = VMMC_TAPI_LL_COD_T38_Error_Set;
#endif

#ifdef VMMC_FEAT_FAX_T38_FW
   pCOD->FAX_Cap_Get             = VMMC_TAPI_LL_COD_FAX_Cap_Get;
   pCOD->FAX_Cfg_Get             = VMMC_TAPI_LL_COD_FAX_Cfg_Get;
   pCOD->FAX_Cfg_Set             = VMMC_TAPI_LL_COD_FAX_Cfg_Set;
   pCOD->FAX_FDP_Cfg_Get         = VMMC_TAPI_LL_COD_FAX_FDP_Cfg_Get;
   pCOD->FAX_FDP_Cfg_Set         = VMMC_TAPI_LL_COD_FAX_FDP_Cfg_Set;
   pCOD->FAX_Start               = VMMC_TAPI_LL_COD_FAX_Start;
   pCOD->FAX_Stat_Get            = VMMC_TAPI_LL_COD_FAX_Stat_Get;
   pCOD->FAX_Stop                = VMMC_TAPI_LL_COD_FAX_Stop;
   pCOD->FAX_TraceSet            = VMMC_TAPI_LL_COD_FAX_Trace;
#endif /* VMMC_FEAT_FAX_T38_FW */

#ifdef VMMC_FEAT_ANNOUNCEMENTS
   pCOD->Ann_Cfg                 = IFX_TAPI_LL_Ann_Cfg;
   pCOD->Ann_Free                = IFX_TAPI_LL_Ann_Free;
   pCOD->Ann_Start               = IFX_TAPI_LL_Ann_Start;
   pCOD->Ann_Stop                = IFX_TAPI_LL_Ann_Stop;
#endif /* VMMC_FEAT_ANNOUNCEMENTS */

   pCOD->AMR_Get                 = VMMC_TAPI_LL_COD_AMR_Get;
}
