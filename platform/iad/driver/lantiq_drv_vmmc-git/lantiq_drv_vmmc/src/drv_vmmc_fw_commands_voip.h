#ifndef _DRV_VMMC_FW_COMMANDS_VOIP_H
#define _DRV_VMMC_FW_COMMANDS_VOIP_H
/******************************************************************************

                            Copyright (c) 2006-2015
                        Lantiq Beteiligungs-GmbH & Co.KG
                             http://www.lantiq.com

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/**
   \file drv_vmmc_fw_commands_voip.h
   This file contains the definitions of the voice-FW command interface.
*/

/* ============================= */
/* Includes                      */
/* ============================= */
#include "drv_tapi_io.h"
#include "drv_vmmc_fw_commands.h"

/* ============================= */
/* Global Defines                */
/* ============================= */

/* ============================= */
/* Global Types                  */
/* ============================= */

/** @defgroup _VMMC_FW_SPEC_COMMANDS_ Command Messages
 *  @{
 */

#ifdef __cplusplus
   extern "C" {
#endif

/* ----- Signal Array ----- */

enum /* ECMD_IX_SIG */
{
   ECMD_IX_EMPTY          = 0x00,

   ECMD_IX_COD_OUT6       = 0x01,
   ECMD_IX_COD_OUT7       = 0x02,
   ECMD_IX_COD_OUT8       = 0x03,

   ECMD_IX_ALM_OUT0       = 0x04,
   ECMD_IX_ALM_OUT1       = 0x05,
   ECMD_IX_ALM_OUT2       = 0x06,

   ECMD_IX_COD_OUT9       = 0x07,

   ECMD_IX_PCM_OUT0       = 0x08,
   ECMD_IX_PCM_OUT1       = 0x09,
   ECMD_IX_PCM_OUT2       = 0x0A,
   ECMD_IX_PCM_OUT3       = 0x0B,
   ECMD_IX_PCM_OUT4       = 0x0C,
   ECMD_IX_PCM_OUT5       = 0x0D,
   ECMD_IX_PCM_OUT6       = 0x0E,
   ECMD_IX_PCM_OUT7       = 0x0F,
   ECMD_IX_PCM_OUT8       = 0x10,
   ECMD_IX_PCM_OUT9       = 0x11,
   ECMD_IX_PCM_OUT10      = 0x12,
   ECMD_IX_PCM_OUT11      = 0x13,
   ECMD_IX_PCM_OUT12      = 0x14,
   ECMD_IX_PCM_OUT13      = 0x15,
   ECMD_IX_PCM_OUT14      = 0x16,
   ECMD_IX_PCM_OUT15      = 0x17,

   ECMD_IX_COD_OUT0       = 0x18,
   ECMD_IX_COD_OUT1       = 0x19,
   ECMD_IX_COD_OUT2       = 0x1A,
   ECMD_IX_COD_OUT3       = 0x1B,
   ECMD_IX_COD_OUT4       = 0x1C,
   ECMD_IX_COD_OUT5       = 0x1D,

   ECMD_IX_SIG_OUTA4      = 0x1E,
   ECMD_IX_SIG_OUTB4      = 0x1F,
   ECMD_IX_SIG_OUTA0      = 0x20,
   ECMD_IX_SIG_OUTB0      = 0x21,
   ECMD_IX_SIG_OUTA1      = 0x22,
   ECMD_IX_SIG_OUTB1      = 0x23,
   ECMD_IX_SIG_OUTA2      = 0x24,
   ECMD_IX_SIG_OUTB2      = 0x25,
   ECMD_IX_SIG_OUTA3      = 0x26,
   ECMD_IX_SIG_OUTB3      = 0x27,

   ECMD_IX_SIG_OUTA5      = 0x2A,
   ECMD_IX_SIG_OUTB5      = 0x2B,
   ECMD_IX_SIG_OUTA6      = 0x2C,
   ECMD_IX_SIG_OUTB6      = 0x2D,
   ECMD_IX_SIG_OUTA7      = 0x2E,
   ECMD_IX_SIG_OUTB7      = 0x2F,

   ECMD_IX_COD_OUT10      = 0x30,
   ECMD_IX_COD_OUT11      = 0x31,
   ECMD_IX_COD_OUT12      = 0x32,   /* overlay with DECT0 */
   ECMD_IX_COD_OUT13      = 0x33,   /* overlay with DECT1 */

   ECMD_IX_SIG_OUTA12     = 0x34,   /* overlay with DECT2 */
   ECMD_IX_SIG_OUTB12     = 0x35,   /* overlay with DECT3 */
   ECMD_IX_SIG_OUTA13     = 0x36,   /* overlay with DECT4 */
   ECMD_IX_SIG_OUTB13     = 0x37,   /* overlay with DECT5 */

   ECMD_IX_DECT_OUT0      = 0x32,   /* overlay with COD12 */
   ECMD_IX_DECT_OUT1      = 0x33,   /* overlay with COD13 */
   ECMD_IX_DECT_OUT2      = 0x34,   /* overlay with SIG12-A */
   ECMD_IX_DECT_OUT3      = 0x35,   /* overlay with SIG12-B */
   ECMD_IX_DECT_OUT4      = 0x36,   /* overlay with SIG13-A */
   ECMD_IX_DECT_OUT5      = 0x37,   /* overlay with SIG14-B */

   ECMD_IX_SIG_OUTA8      = 0x38,
   ECMD_IX_SIG_OUTB8      = 0x39,
   ECMD_IX_SIG_OUTA9      = 0x3A,
   ECMD_IX_SIG_OUTB9      = 0x3B,
   ECMD_IX_SIG_OUTA10     = 0x3C,
   ECMD_IX_SIG_OUTB10     = 0x3D,
   ECMD_IX_SIG_OUTA11     = 0x3E,
   ECMD_IX_SIG_OUTB11     = 0x3F
};

/* ----- typedefs ----- */

typedef struct PCM_CTRL PCM_CTRL_t;
typedef struct PCM_CHAN PCM_CHAN_t;
typedef struct PCM_SCHAN PCM_SCHAN_t;
typedef struct PCM_DCHAN PCM_DCHAN_t;
typedef struct PCM_LEC PCM_LEC_t;

typedef struct ALI_CHAN ALI_CHAN_t;
typedef struct ALI_LEC ALI_LEC_t;
typedef struct ALI_ES ALI_ES_t;

typedef struct COD_CHAN_SPEECH COD_CHAN_SPEECH_t;
typedef struct COD_AGC_CTRL COD_AGC_CTRL_t;
typedef struct COD_CHAN_RTP_SUP_CFG_US COD_CHAN_RTP_SUP_CFG_US_t;
typedef struct COD_CHAN_RTP_SUP_CFG_DS COD_CHAN_RTP_SUP_CFG_DS_t;
typedef struct COD_JB_CONF COD_JB_CONF_t;
typedef struct COD_JB_STAT COD_JB_STAT_t;
typedef struct COD_DEC_STAT COD_DEC_STAT_t;
typedef struct COD_RTP_SUP_CFG COD_RTP_SUP_CFG_t;
typedef struct COD_RTP_CONF_SUPP COD_RTP_CONF_SUPP_t;
typedef struct COD_RTP_CONF_STAT COD_RTP_CONF_STAT_t;
typedef struct COD_RTCP_SUP_CTRL COD_RTCP_SUP_CTRL_t;
typedef struct COD_RTCP_XR_STAT_SUM COD_RTCP_XR_STAT_SUM_t;
typedef struct COD_RTCP_XR_VOIP_MET COD_RTCP_XR_VOIP_MET_t;
typedef struct COD_ASSOCIATED_LECNR COD_ASSOCIATED_LECNR_t;
typedef struct COD_EVT_GEN COD_EVT_GEN_t;

typedef struct COD_FAX_CTRL COD_FAX_CTRL_t;
typedef struct COD_FAX_MOD_CTRL COD_FAX_MOD_CTRL_t;
typedef struct COD_FAX_DEMOD_CTRL COD_FAX_DEMOD_CTRL_t;
typedef struct COD_FAX_READ_CAP COD_FAX_READ_CAP_t;
typedef struct COD_FAX_ACT COD_FAX_ACT_t;
typedef struct COD_FAX_CONF COD_FAX_CONF_t;
typedef struct COD_FAX_FDP_PARAMS COD_FAX_FDP_PARAMS_t;
typedef struct COD_FAX_STAT COD_FAX_STAT_t;
typedef struct COD_FAX_TRACE COD_FAX_TRACE_t;

typedef struct DECT_CHAN_SPEECH DECT_CHAN_SPEECH_t;
typedef struct DECT_CODER_STAT DECT_CODER_STAT_t;
typedef struct DECT_UTG_CTRL DECT_UTG_CTRL_t;

typedef struct SIG_CHAN SIG_CHAN_t;
typedef struct SIG_CIDS_CTRL SIG_CIDS_CTRL_t;
typedef struct SIG_DTMFATG_CTRL SIG_DTMFATG_CTRL_t;
typedef struct SIG_DTMFR_CTRL SIG_DTMFR_CTRL_t;
typedef struct SIG_CPTD_CTRL SIG_CPTD_CTRL_t;
typedef struct SIG_UTG_CTRL SIG_UTG_CTRL_t;
typedef struct SIG_MFTD_CTRL SIG_MFTD_CTRL_t;
typedef struct SIG_CIDR_CTRL SIG_CIDR_CTRL_t;
typedef struct SIG_RTP_SUP SIG_RTP_SUP_t;
typedef struct SIG_RTP_EVT_STAT SIG_RTP_EVT_STAT_t;

typedef struct RES_LEC_COEF RES_LEC_COEF_t;
typedef struct RES_LEC_NLP_COEF RES_LEC_NLP_COEF_t;
typedef struct RES_CIDR_COEF RES_CIDR_COEF_t;
typedef struct RES_CIDS_COEF RES_CIDS_COEF_t;
typedef struct RES_CIDS_DATA RES_CIDS_DATA_t;
typedef struct RES_DTMFATG_COEF RES_DTMFATG_COEF_t;
typedef struct RES_DTMFATG_DATA RES_DTMFATG_DATA_t;
typedef struct RES_DTMFR_COEF RES_DTMFR_COEF_t;
typedef struct RES_AGC_COEF RES_AGC_COEF_t;
typedef struct RES_CPTD_COEF RES_CPTD_COEF_t;
typedef struct RES_UTG_COEF RES_UTG_COEF_t;
typedef struct RES_ES_COEF RES_ES_COEF_t;

typedef struct SYS_CERR_ACK SYS_CERR_ACK_t;
typedef struct SYS_OLOAD_ACK SYS_OLOAD_ACK_t;
typedef struct SYS_VER SYS_VER_t;
typedef struct SYS_CAP SYS_CAP_t;
typedef struct SYS_CERR_GET SYS_CERR_GET_t;

/* ----- definition of voice-FW commands ----- */

/* Number of CPTDs in command struct */
#define  SIG_CPTD_CTRL_DATA_MAX      3


/**
   This command activates the PCM interface. Activation of the PCM interface is
   required before a PCM channel can be activated.
*/
struct PCM_CTRL
{
   CMD_HEAD_BE;
   /** Enable */
   IFX_uint32_t EN : 1;
   /** Data Streaming */
   IFX_uint32_t DS : 1;
   /** Clock Tracking */
   IFX_uint32_t CT : 1;
   /** Reserved */
   IFX_uint32_t Res01 : 2;
   /** PCM Transmit Offset */
   IFX_uint32_t PCMXO : 3;
   /** Double Clock */
   IFX_uint32_t DBLCLK : 1;
   /** Transmit Slope */
   IFX_uint32_t X_SLOPE : 1;
   /** Receive Slope */
   IFX_uint32_t R_SLOPE : 1;
   /** Driving Mode for Bit 0 */
   IFX_uint32_t DRIVE_0 : 1;
   /** Shift access edge */
   IFX_uint32_t SHIFT : 1;
   /** PCM Receive Offset */
   IFX_uint32_t PCMRO : 3;
   /** MCTS */
   IFX_uint32_t MCTS : 8;
   /** Reserved */
   IFX_uint32_t Res02 : 2;
   /** Slave mode */
   IFX_uint32_t SM : 1;
   /** DCL frequency */
   IFX_uint32_t DCLFREQ : 5;
} __PACKED__ ;

#define  PCM_CTRL_ECMD  0
#define  PCM_CTRL_LEN  4
#define  PCM_CTRL_DISABLE  0x0
#define  PCM_CTRL_ENABLE  0x1
#define  PCM_CTRL_DS_ON  0x1
#define  PCM_CTRL_PCMXO_0  0
#define  PCM_CTRL_PCMXO_1  1
#define  PCM_CTRL_PCMXO_2  2
#define  PCM_CTRL_PCMXO_3  3
#define  PCM_CTRL_PCMXO_4  4
#define  PCM_CTRL_PCMXO_5  5
#define  PCM_CTRL_PCMXO_6  6
#define  PCM_CTRL_PCMXO_7  7
#define  PCM_CTRL_DBLCLK_OFF  0x0
#define  PCM_CTRL_DBLCLK_ON  0x1
#define  PCM_CTRL_X_SLOPE_FALL  0x1
#define  PCM_CTRL_R_SLOPE_RISE  0x1
#define  PCM_CTRL_DRIVE_0_ENTIRE  0x0
#define  PCM_CTRL_DRIVE_0_HALF  0x1
#define  PCM_CTRL_OFF  0x0
#define  PCM_CTRL_ON  0x1
#define  PCM_CTRL_PCMRO_0  0
#define  PCM_CTRL_PCMRO_1  1
#define  PCM_CTRL_PCMRO_2  2
#define  PCM_CTRL_PCMRO_3  3
#define  PCM_CTRL_PCMRO_4  4
#define  PCM_CTRL_PCMRO_5  5
#define  PCM_CTRL_PCMRO_6  6
#define  PCM_CTRL_PCMRO_7  7
#define  PCM_CTRL_DCLFREQ_512  0
#define  PCM_CTRL_DCLFREQ_1024  1
#define  PCM_CTRL_DCLFREQ_1536  2
#define  PCM_CTRL_DCLFREQ_2048  3
#define  PCM_CTRL_DCLFREQ_4096  4
#define  PCM_CTRL_DCLFREQ_8192  5
#define  PCM_CTRL_DCLFREQ_16384  6


/**
   This command configures a PCM channel. Up to 16 channels are supported, depending on
   the firmware variant. The PCM-Interface has to be activated with the command PCM
   Interface Control on Page3 before this command can be sent. The Line Echo Canceller
   has to be activated with a separate command.
   Each PCM channel can be used in wideband mode or in narrowband mode. In addititon,
   the PCM interface itself can be used to transport narrowband data while the
   interface to the signal array transports wideband data. For this case, an
   interpolator and a decimator are included in the signal flow. Table6 gives an
   overview of the operating modes.
   Operating Mode of the PCM Channel
   For wideband mode (ISR=1 and UD=0), only the codecs G.711 a-law, G.711 m-law and 16
   bit linear are supported. For G.711 two, for 16 bit linear mode, 4 consecutive time
   slots are used for the transmission.
*/
struct PCM_CHAN
{
   CMD_HEAD_BE;
   /** Enable */
   IFX_uint32_t EN : 1;
   /** Coder */
   IFX_uint32_t COD : 3;
   /** Coder Resource Number */
   IFX_uint32_t CODNR : 4;
   /** High Pass */
   IFX_uint32_t HP : 1;
   /** Bit Packing */
   IFX_uint32_t BP : 1;
   /** Input 1 Address */
   IFX_uint32_t I1 : 6;
   /** Internal Sampling Rate */
   IFX_uint32_t ISR : 1;
   /** Transmit Highway Time slot */
   IFX_uint32_t XTS : 7;
   /** Upsampling / Downsampling */
   IFX_uint32_t UD : 1;
   /** Receive Highway Time slot */
   IFX_uint32_t RTS : 7;
   /** Gain PCM Transmit */
   IFX_uint32_t GAIN1 : 16;
   /** Gain PCM Receive */
   IFX_uint32_t GAIN2 : 16;
   /** Reserved */
   IFX_uint32_t Res01 : 2;
   /** Input 2 Address */
   IFX_uint32_t I2 : 6;
   /** Sample Swap */
   IFX_uint32_t SWP : 1;
   /** Split TimeSlots */
   IFX_uint32_t STS : 1;
   /** Input 3 Address */
   IFX_uint32_t I3 : 6;
   /** Reserved */
   IFX_uint32_t Res03 : 2;
   /** Input 4 Address */
   IFX_uint32_t I4 : 6;
   /** Reserved */
   IFX_uint32_t Res04 : 2;
   /** Input 5 Address */
   IFX_uint32_t I5 : 6;
} __PACKED__ ;

#define  PCM_CHAN_ECMD  1
#define  PCM_CHAN_LEN  12
#define  PCM_CHAN_DISABLE  0x0
#define  PCM_CHAN_ENABLE  0x1
#define  PCM_CHAN_COD_LIN        0
#define  PCM_CHAN_COD_G722       1
#define  PCM_CHAN_COD_G711_ALAW  2
#define  PCM_CHAN_COD_G711_MLAW  3
#define  PCM_CHAN_COD_G726_16    4
#define  PCM_CHAN_COD_G726_24    5
#define  PCM_CHAN_COD_G726_32    6
#define  PCM_CHAN_COD_G726_40    7
#define  PCM_CHAN_HP_OFF  0x0
#define  PCM_CHAN_HP_ON  0x1
#define  PCM_CHAN_BP_LSB  0x0
#define  PCM_CHAN_BP_MSB  0x1
#define  PCM_CHAN_8KHZ  0x0
#define  PCM_CHAN_16KHZ  0x1
#define  PCM_CHAN_UD_OFF  0x0
#define  PCM_CHAN_UD_ON  0x1


/**
   This command creates a shortcut between PCM channel CHAN and SCHAN.
   The data are copied in the 8 kHz interrupt from TS:DD to STS:DD and
   from STS:DU to TS:DU.

   Neither the PCM channel CHAN nor the PCM channel SCHAN may already be in use.
   If they are already used, a command error will be issued. Also the resource
   number PCMSR must not be in use. The same resource number and channel
   numbers, which have been used for activation must be used for deactivation.
*/
struct PCM_SCHAN
{
   CMD_HEAD_BE;
   /** Enable */
   IFX_uint32_t EN : 1;
   /** Time slot of PCM channel */
   IFX_uint32_t TS : 7;
   /** Reserved */
   IFX_uint32_t Res02 : 4;
   /** Resource number of the PCM shortcut */
   IFX_uint32_t PCMSR : 4;
   /** Reserved */
   IFX_uint32_t Res03 : 4;
   /** PCM Shortcut channel number */
   IFX_uint32_t SCHAN : 4;
   /** Reserved */
   IFX_uint32_t Res04 : 1;
   /** Time slot of the PCM shortcut channel */
   IFX_uint32_t STS : 7;
} __PACKED__ ;

#define  PCM_SCHAN_ECMD 5
#define  PCM_SCHAN_LEN 4
#define  PCM_SCHAN_DISABLE 0x0
#define  PCM_SCHAN_ENABLE 0x1
#define  PCM_SCHAN_PCMSR_CNT 0x2


/**
   This command configures a PCM D-channel.
   The same channel number must not be used for activating a PCM channel
   or a PCM D-channel. Also the the same time slot must not be used by a
   PCM channel and a PCM D-channel.
*/
struct PCM_DCHAN
{
   CMD_HEAD_BE;
   /** Enable */
   IFX_uint32_t EN : 1;
   /** Time slot to be activated */
   IFX_uint32_t TS : 7;
   /** Inter frame time fill pattern */
   IFX_uint32_t ITF : 1;
   /** Reserved */
   IFX_uint32_t Res02 : 3;
   /** D-Channel resource number */
   IFX_uint32_t DCR : 4;
   /** Reserved */
   IFX_uint32_t Res03 : 16;
} __PACKED__ ;

#define  PCM_DCHAN_ECMD 4
#define  PCM_DCHAN_LEN 4
#define  PCM_DCHAN_DISABLE 0x0
#define  PCM_DCHAN_ENABLE 0x1
#define  PCM_DCHAN_DCR_CNT 0x2
#define  PCM_DCHAN_ITF_IDLE  0
#define  PCM_DCHAN_ITF_FLAGS 1


/**
   This command activates the line echo canceller (LEC) in the addressed PCM channel.
   The LEC has near end and far end echo cancellation capabilities. If the LEC runs in
   narrowband or wideband mode is determined by the bits ISR and UDt in PCM Interface
   Channel Control on Page7. The selected PCM channel (CHAN) has to be activated
   before this command can be sent. The coefficients for the LEC and NLP have to be
   programmed with separate commands prior to the activation of the function.
*/
struct PCM_LEC
{
   CMD_HEAD_BE;
   /** Enable */
   IFX_uint32_t EN : 1;
   /** Reserved */
   IFX_uint32_t Res01 : 2;
   /** Open echo path */
   IFX_uint32_t OEP : 1;
   /** Moving Window Mode */
   IFX_uint32_t MW : 1;
   /** Decorrelation Filter Mode */
   IFX_uint32_t DCF : 1;
   /** Disable Turbo Mode */
   IFX_uint32_t DTM : 1;
   /** Reserved */
   IFX_uint32_t Res02 : 1;
   /** Adaptation Stop Bit */
   IFX_uint32_t AS : 1;
   /** Non Linear Processor (NLP) */
   IFX_uint32_t NLP : 1;
   /** NLP Mode */
   IFX_uint32_t NLPM : 2;
   /** LEC Resource Number */
   IFX_uint32_t LECNR : 4;
   /**  Parameter freeze */
   IFX_uint32_t PF : 1;
   /**  Automatic parameter freeze */
   IFX_uint32_t APF : 1;
   /**  Reset Parameters */
   IFX_uint32_t RP : 1;
   /** Reserved */
   IFX_uint32_t Res03 : 13;
} __PACKED__ ;

#define  PCM_LEC_ECMD  2
#define  PCM_LEC_LEN  4
#define  PCM_LEC_DISABLE  0x0
#define  PCM_LEC_ENABLE   0x1
#define  PCM_LEC_ON  0x1
#define  PCM_LEC_MW_FIX  0x0
#define  PCM_LEC_MW_ADAPTIVE  0x1
#define  PCM_LEC_DCF_EN   0x1
#define  PCM_LEC_DTM_OFF  0x1
#define  PCM_LEC_AS_STOP  0x1
#define  PCM_LEC_NLP_OFF  0
#define  PCM_LEC_NLP_ON   1
#define  PCM_LEC_NLPM_RES  1
#define  PCM_LEC_NLPM_SIGN_NOISE  2
#define  PCM_LEC_NLPM_WHITE_NOISE  3
#define  PCM_LEC_PF_OFF  0
#define  PCM_LEC_PF_ON   1
#define  PCM_LEC_APF_OFF 0
#define  PCM_LEC_APF_ON  1
#define  PCM_LEC_RP_OFF  1
#define  PCM_LEC_RP_ON   0


/**
   This command configures an Analog Line Interface channel. The Line Echo Canceller
   has to be activated with a separate command.
*/
struct ALI_CHAN
{
   CMD_HEAD_BE;
   /** Enable */
   IFX_uint32_t EN : 1;
   /** Internal Sampling Rate */
   IFX_uint32_t ISR : 1;
   /** Upsampling / Downsampling */
   IFX_uint32_t UD : 1;
   /* BP reserved */
   IFX_uint32_t BP : 1;
   /* EH enable hook event generation */
   IFX_uint32_t EH : 1;
   /* EO OPC event generation */
   IFX_uint32_t EO : 1;
   /** Reserved */
   IFX_uint32_t Res01 : 4;
   /** Signal array address of input signal 1 */
   IFX_uint32_t I1 : 6;
   /** Gain ALM Transmit */
   IFX_uint32_t DG1 : 16;
   /** Gain ALM Receive */
   IFX_uint32_t DG2 : 16;
   /** Reserved */
   IFX_uint32_t Res02 : 2;
   /** Signal Array Address of Input Signal 2 */
   IFX_uint32_t I2 : 6;
   /** Reserved */
   IFX_uint32_t Res03 : 2;
   /** Signal Array Address of Input Signal 3 */
   IFX_uint32_t I3 : 6;
   /** Reserved */
   IFX_uint32_t Res04 : 2;
   /** Signal Array Address of Input Signal 4 */
   IFX_uint32_t I4 : 6;
   /** Reserved */
   IFX_uint32_t Res05 : 2;
   /** Signal Array Address of Input Signal 5 */
   IFX_uint32_t I5 : 6;
   /** Reserved */
   IFX_uint32_t Res06 : 16;
} __PACKED__ ;

#define  ALI_CHAN_ECMD  1
#define  ALI_CHAN_LEN  12
#define  ALI_CHAN_ENABLE  0x1
#define  ALI_CHAN_DISABLE  0x0
#define  ALI_CHAN_16KHZ  0x1
#define  ALI_CHAN_OFF  0x0
#define  ALI_CHAN_ON  0x1
#define  ALI_CHAN_EH_ON 0x1
#define  ALI_CHAN_EH_OFF 0x0
#define  ALI_CHAN_EO_ON 0x1
#define  ALI_CHAN_EO_OFF 0x0


/**
   This command activates the line echo canceller (LEC) in the addressed ALI channel.
   The LEC has near end and far end echo cancellation capabilities. If the LEC runs in
   narrowband or wideband mode is determined by the ISR bit in Analog Line Interface
   Channel on Page30. The selected ALI channel (CHAN) has to be activated before this
   command can be sent. The coefficients for the LEC and NLP have to be programmed with
   separate commands prior to the activation of the algorithm.
*/
struct ALI_LEC
{
   CMD_HEAD_BE;
   /** Enable */
   IFX_uint32_t EN : 1;
   /** Reserved */
   IFX_uint32_t Res01 : 2;
   /** Open echo path */
   IFX_uint32_t OEP : 1;
   /** Moving Window Mode */
   IFX_uint32_t MW : 1;
   /** Decorrelation Filter Mode */
   IFX_uint32_t DCF : 1;
   /** Disable Turbo Mode */
   IFX_uint32_t DTM : 1;
   /** Reserved */
   IFX_uint32_t Res02 : 1;
   /** Adaptation Stop Bit */
   IFX_uint32_t AS : 1;
   /** Non Linear Processor (NLP) */
   IFX_uint32_t NLP : 1;
   /** NLP Mode */
   IFX_uint32_t NLPM : 2;
   /** LEC Resource Number */
   IFX_uint32_t LECNR : 4;
   /**  Parameter freeze */
   IFX_uint32_t PF : 1;
   /**  Automatic parameter freeze */
   IFX_uint32_t APF : 1;
   /**  Reset Parameters */
   IFX_uint32_t RP : 1;
   /** Reserved */
   IFX_uint32_t Res03 : 13;
} __PACKED__ ;

#define  ALI_LEC_ECMD  2
#define  ALI_LEC_LEN  4
#define  ALI_LEC_DISABLE  0x0
#define  ALI_LEC_ENABLE  0x1
#define  ALI_LEC_ON  0x1
#define  ALI_LEC_MW_FIX  0x0
#define  ALI_LEC_MW_ADAPTIVE  0x1
#define  ALI_LEC_DTM_OFF  0x1
#define  ALI_LEC_AS_STOP  0x1
#define  ALI_LEC_NLPM_RES  1
#define  ALI_LEC_NLPM_SIGN_NOISE  2
#define  ALI_LEC_NLPM_WHITE_NOISE  3
#define  ALI_LEC_NLP_ON  1
#define  ALI_LEC_NLP_OFF 0
#define  ALI_LEC_DCF_EN  0x1
#define  ALI_LEC_PF_OFF 0
#define  ALI_LEC_PF_ON  1
#define  ALI_LEC_APF_OFF 0
#define  ALI_LEC_APF_ON  1
#define  ALI_LEC_RP_OFF 1
#define  ALI_LEC_RP_ON  0


/**
   This command activates the line echo canceller echo suppressor in the
   addressed ALI/PCM/DECT channel.
*/
struct ALI_ES
{
   CMD_HEAD_BE;
   /** Enable */
   IFX_uint32_t EN : 1;
   /** Reserved */
   IFX_uint32_t Res01 : 11;
   /** ES Resource Number */
   IFX_uint32_t ESNR : 4;
   /** Reserved */
   IFX_uint32_t Res02 : 16;
} __PACKED__ ;

#define  ALI_ES_ECMD  3
#define  PCM_ES_ECMD  3
#define  DECT_ES_ECMD  4
#define  ALI_ES_LEN  4


/**
   This command activates a coder channel in speech compression mode.
   The coder can only be a narrowband coder, if the bit ISR is set to 0. If the bit ISR
   is set to 1, the coder can be a narrowband or a wideband coder. If for ISR=1, a
   narrowband decoder is active, the samples are automatically interpolated to 16 kHz
   before they are passed to the signal array. If for ISR=1, a narrowband encoder is
   chosen, the samples from the signal array are automatically decimated to 8 kHz
   before they are passed to the encoder.
   The bit ISR may only be changed, when the coder channel is disabled.
*/
struct COD_CHAN_SPEECH
{
   CMD_HEAD_BE;
   /** Enable */
   IFX_uint32_t EN : 1;
   /** Internal Sampling Rate */
   IFX_uint32_t ISR : 1;
   /** Noise Level */
   IFX_uint32_t NS : 1;
   /** Encoder Packet Time */
   IFX_uint32_t PTE : 3;
   /** Coder Resource Number */
   IFX_uint32_t CODNR : 4;
   /** Signal Array Address of Input Signal 1 */
   IFX_uint32_t I1 : 6;
   /** DC-High Pass */
   IFX_uint32_t HP : 1;
   /** Decoder Post Filter */
   IFX_uint32_t PF : 1;
   /** Comfort Noise Generation (Decoder) */
   IFX_uint32_t CNG : 1;
   /** Bad Frame Interpolation (Decoder) */
   IFX_uint32_t BFI : 1;
   /** Decoder Path Status */
   IFX_uint32_t DEC : 1;
   /** Interrupt Mode */
   IFX_uint32_t IM : 1;
   /** Periodical SID Transmission */
   IFX_uint32_t PST : 1;
   /** Silence Compression of the Encoder */
   IFX_uint32_t SIC : 1;
   /** Encoder Mute */
   IFX_uint32_t EM : 1;
   /** FEC for the AMR codec according to RFC 3267 */
   IFX_uint32_t FEC : 2;
   /** Encoder Algorithm */
   IFX_uint32_t ENC : 5;
   /** Gain Transmit */
   IFX_uint32_t GAIN1 : 16;
   /** Gain Receive */
   IFX_uint32_t GAIN2 : 16;
   /** Decoder Endianess */
   IFX_uint32_t DE : 1;
   /** Encoder Endianess */
   IFX_uint32_t EE : 1;
   /** Input 2 */
   IFX_uint32_t I2 : 6;
   /** Redundancy level */
   IFX_uint32_t RED : 2;
   /** Input 3 */
   IFX_uint32_t I3 : 6;
   /** Packet Loss Code */
   IFX_uint32_t PLC : 1;
   /** Reserved */
   IFX_uint32_t Res04 : 1;
   /** Input 4 */
   IFX_uint32_t I4 : 6;
   /** Reserved */
   IFX_uint32_t Res05 : 2;
   /** Input 5 */
   IFX_uint32_t I5 : 6;
   /** Time stamp frequency */
   IFX_uint32_t TSF : 1;
   /** Bit Rate */
   IFX_uint32_t BITR : 7;
   /** Reserved */
   IFX_uint32_t Res06 : 8;
   /** Reserved */
   IFX_uint32_t Res07 : 16;
} __PACKED__ ;

#define  COD_CHAN_SPEECH_ECMD  1
#define  COD_CHAN_SPEECH_LEN  16
#define  COD_CHAN_SPEECH_DISABLE  0x0
#define  COD_CHAN_SPEECH_ENABLE  0x1
#define  COD_CHAN_SPEECH_8KHZ  0x0
#define  COD_CHAN_SPEECH_16KHZ  0x1
#define  COD_CHAN_SPEECH_ACTIVE  0x1
#define  COD_CHAN_SPEECH_ON  0x1
#define  COD_CHAN_SPEECH_DEC_ACTIVE 0x1
#define  COD_CHAN_SPEECH_DEC_INACTIVE 0x0
#define  COD_CHAN_SPEECH_PTE_NO       0
#define  COD_CHAN_SPEECH_PTE_10       1
#define  COD_CHAN_SPEECH_PTE_20       2
#define  COD_CHAN_SPEECH_PTE_30       3
#define  COD_CHAN_SPEECH_PTE_40       6
#define  COD_CHAN_SPEECH_PTE_60       7
#define  COD_CHAN_SPEECH_ENC_NO  0
#define  COD_CHAN_SPEECH_ENC_G711_ALAW  2
#define  COD_CHAN_SPEECH_ENC_G711_MLAW  3
#define  COD_CHAN_SPEECH_ENC_G726_16  4
#define  COD_CHAN_SPEECH_ENC_G726_24  5
#define  COD_CHAN_SPEECH_ENC_G726_32  6
#define  COD_CHAN_SPEECH_ENC_G726_40  7
/* AMR with bitrate codec number can be used only,
   when AMRE is not set in SYS_CAP. */
#define  COD_CHAN_SPEECH_ENC_AMR_4_75  8
#define  COD_CHAN_SPEECH_ENC_AMR_5_15  9
#define  COD_CHAN_SPEECH_ENC_AMR_5_9  10
#define  COD_CHAN_SPEECH_ENC_AMR_6_7  11
#define  COD_CHAN_SPEECH_ENC_AMR_7_4  12
#define  COD_CHAN_SPEECH_ENC_AMR_7_95  13
#define  COD_CHAN_SPEECH_ENC_AMR_10_2  14
#define  COD_CHAN_SPEECH_ENC_AMR_12_2  15
#define  COD_CHAN_SPEECH_ENC_G728_16  16
#define  COD_CHAN_SPEECH_ENC_G729AB_8  18
#define  COD_CHAN_SPEECH_ENC_G729E_11_8  19
#define  COD_CHAN_SPEECH_ENC_G7221_24  20
#define  COD_CHAN_SPEECH_ENC_G7221_32  21
#define  COD_CHAN_SPEECH_ENC_G722_64  22
#define  COD_CHAN_SPEECH_ENC_LIN16_8KHZ   24
#define  COD_CHAN_SPEECH_ENC_LIN16_16KHZ  25
#define  COD_CHAN_SPEECH_ENC_ILBC_15_2  26
#define  COD_CHAN_SPEECH_ENC_ILBC_13_3  27
#define  COD_CHAN_SPEECH_ENC_G7231_5_3  28
#define  COD_CHAN_SPEECH_ENC_G7231_6_3  29
#define  COD_CHAN_SPEECH_ENC_G711_ALAW_VBD  30
#define  COD_CHAN_SPEECH_ENC_G711_MLAW_VBD  31

/* Encoder Algorithm number without bitrate,
   used when AMRE is set in SYS_CAP. */
/** AMR-NB - used when AMRE is set */
#define  COD_CHAN_SPEECH_ENC_AMR_NB  8
/* Max number of bitrate for AMR-NB */
#define  COD_CHAN_SPEECH_BITR_AMR_NB_MAX  7

/** G722.2, AMR-WB - used when AMRE is set */
#define  COD_CHAN_SPEECH_ENC_AMR_WB  10
/* Max number of bitrate for AMR-WB */
#define  COD_CHAN_SPEECH_BITR_AMR_WB_MAX  8

#define  COD_CHAN_SPEECH_BITR_INVALID  0xFF

/**
   This command activates the AGC in the addressed coder channel. Whether the AGC runs
   with 8 or 16 kHz sampling rate is determined by the ISR bit in the command Coder
   Channel Speech Compression on Page68.
   The selected coder channel (CHAN) has to be activated before this command can be
   sent. If AGC coefficients, which differ from the default values, are required, they
   have to be programmed with a separate command before the AGC is activated.
*/
struct COD_AGC_CTRL
{
   CMD_HEAD_BE;
   /** Enable */
   IFX_uint32_t EN : 1;
   /** Reserved */
   IFX_uint32_t Res01 : 11;
   /** AGC Resource Number */
   IFX_uint32_t AGCNR : 4;
   /** Reserved */
   IFX_uint32_t Res02 : 16;
} __PACKED__ ;

#define  COD_AGC_CTRL_ECMD  3
#define  COD_AGC_CTRL_LEN  4
#define  COD_AGC_CTRL_ENABLE  0x1
#define  COD_AGC_CTRL_DISABLE 0x0


/**
   This command allows to set or read the value for the timestamp. The timestamp is set
   for each channel individually.
*/
struct COD_RTP_SUP_CFG
{
   CMD_HEAD_BE;
   /** 32-Bit Timestamp */
   IFX_uint32_t TIME_STAMP;
} __PACKED__ ;

#define  COD_CHAN_RTP_TIMESTAMP_ECMD   16
#define  COD_CHAN_RTP_TIMESTAMP_LEN 4


/**
   This command allows the setting of the payload type for each of the supported codecs
   in upstream direction. The payload types have to be set before the channel is
   activated.
   For G.723.1, iLBC and AMR, the same payload type may be used for each bit rate of
   the corresponding codec.
*/
struct COD_CHAN_RTP_SUP_CFG_US
{
   CMD_HEAD_BE;
   /** Synchronization Source */
   IFX_uint32_t SSRC;

/* Instruction 2 */
   /** Sequence Number */
   IFX_uint32_t SEQ_NR : 16;
   /** Silence Insertion Descriptor for Encoder 2 */
   IFX_uint32_t SID2 : 1;
   /** Payload Type for Encoder 2 */
   IFX_uint32_t PT2 : 7;
   /** Silence Insertion Descriptor for Encoder 3 */
   IFX_uint32_t SID3 : 1;
   /** Payload Type for corresponding Encoder 3 */
   IFX_uint32_t PT3 : 7;

/* Instruction 3 */
   /** Silence Insertion Descriptor for Encoder 4 */
   IFX_uint32_t SID4 : 1;
   /** Payload Type for corresponding Encoder 4 */
   IFX_uint32_t PT4 : 7;
   /** Silence Insertion Descriptor for Encoder 5 */
   IFX_uint32_t SID5 : 1;
   /** Payload Type for corresponding Encoder 5 */
   IFX_uint32_t PT5 : 7;
   /** Silence Insertion Descriptor for Encoder 6 */
   IFX_uint32_t SID6 : 1;
   /** Payload Type for corresponding Encoder 6 */
   IFX_uint32_t PT6 : 7;
   /** Silence Insertion Descriptor for Encoder 7 */
   IFX_uint32_t SID7 : 1;
   /** Payload Type for corresponding Encoder 7 */
   IFX_uint32_t PT7 : 7;

/* Instruction 4 */
   /** Silence Insertion Descriptor for Encoder 8 */
   IFX_uint32_t SID8 : 1;
   /** Payload Type for corresponding Encoder 8 */
   IFX_uint32_t PT8 : 7;
   /** Silence Insertion Descriptor for Encoder 9 */
   IFX_uint32_t SID9 : 1;
   /** Payload Type for corresponding Encoder 9 */
   IFX_uint32_t PT9 : 7;
   /** Silence Insertion Descriptor for Encoder 10 */
   IFX_uint32_t SID10 : 1;
   /** Payload Type for corresponding Encoder 10 */
   IFX_uint32_t PT10 : 7;
   /** Silence Insertion Descriptor for Encoder 11 */
   IFX_uint32_t SID11 : 1;
   /** Payload Type for corresponding Encoder 11 */
   IFX_uint32_t PT11 : 7;

/* Instruction 5 */
   /** Silence Insertion Descriptor for Encoder 12 */
   IFX_uint32_t SID12 : 1;
   /** Payload Type for corresponding Encoder 12 */
   IFX_uint32_t PT12 : 7;
   /** Silence Insertion Descriptor for Encoder 13 */
   IFX_uint32_t SID13 : 1;
   /** Payload Type for corresponding Encoder 13 */
   IFX_uint32_t PT13 : 7;
   /** Silence Insertion Descriptor for Encoder 14 */
   IFX_uint32_t SID14 : 1;
   /** Payload Type for corresponding Encoder 14 */
   IFX_uint32_t PT14 : 7;
   /** Silence Insertion Descriptor for Encoder 15 */
   IFX_uint32_t SID15 : 1;
   /** Payload Type for corresponding Encoder 15 */
   IFX_uint32_t PT15 : 7;

/* Instruction 6 */
   /** Silence Insertion Descriptor for Encoder 16 */
   IFX_uint32_t SID16 : 1;
   /** Payload Type for corresponding Encoder 16 */
   IFX_uint32_t PT16 : 7;
   /** Silence Insertion Descriptor for Encoder 17 */
   IFX_uint32_t SID17 : 1;
   /** Payload Type for corresponding Encoder 17 */
   IFX_uint32_t PT17 : 7;
   /** Silence Insertion Descriptor for Encoder 18 */
   IFX_uint32_t SID18 : 1;
   /** Payload Type for corresponding Encoder 18 */
   IFX_uint32_t PT18 : 7;
   /** Silence Insertion Descriptor for Encoder 19 */
   IFX_uint32_t SID19 : 1;
   /** Payload Type for corresponding Encoder 19 */
   IFX_uint32_t PT19 : 7;

/* Instruction 7 */
   /** Silence Insertion Descriptor for Encoder 20 */
   IFX_uint32_t SID20 : 1;
   /** Payload Type for corresponding Encoder 20 */
   IFX_uint32_t PT20 : 7;
   /** Silence Insertion Descriptor for Encoder 21 */
   IFX_uint32_t SID21 : 1;
   /** Payload Type for corresponding Encoder 21 */
   IFX_uint32_t PT21 : 7;
   /** Silence Insertion Descriptor for Encoder 22 */
   IFX_uint32_t SID22 : 1;
   /** Payload Type for corresponding Encoder 22 */
   IFX_uint32_t PT22 : 7;
   /** Silence Insertion Descriptor for Encoder 23 */
   IFX_uint32_t SID23 : 1;
   /** Payload Type for corresponding Encoder 23 */
   IFX_uint32_t PT23 : 7;

/* Instruction 8 */
   /** Silence Insertion Descriptor for Encoder 24 */
   IFX_uint32_t SID24 : 1;
   /** Payload Type for corresponding Encoder 24 */
   IFX_uint32_t PT24 : 7;
   /** Silence Insertion Descriptor for Encoder 25 */
   IFX_uint32_t SID25 : 1;
   /** Payload Type for corresponding Encoder 25 */
   IFX_uint32_t PT25 : 7;
   /** Silence Insertion Descriptor for Encoder 26 */
   IFX_uint32_t SID26 : 1;
   /** Payload Type for corresponding Encoder 26 */
   IFX_uint32_t PT26 : 7;
   /** Silence Insertion Descriptor for Encoder 27 */
   IFX_uint32_t SID27 : 1;
   /** Payload Type for corresponding Encoder 27 */
   IFX_uint32_t PT27 : 7;

/* Instruction 9 */
   /** Silence Insertion Descriptor for Encoder 28 */
   IFX_uint32_t SID28 : 1;
   /** Payload Type for corresponding Encoder 28 */
   IFX_uint32_t PT28 : 7;
   /** Silence Insertion Descriptor for Encoder 29 */
   IFX_uint32_t SID29 : 1;
   /** Payload Type for corresponding Encoder 29 */
   IFX_uint32_t PT29 : 7;
   /** Silence Insertion Descriptor for Encoder 30 */
   IFX_uint32_t SID30 : 1;
   /** Payload Type for corresponding Encoder 30 */
   IFX_uint32_t PT30 : 7;
   /** Silence Insertion Descriptor for Encoder 31 */
   IFX_uint32_t SID31 : 1;
   /** Payload Type for corresponding Encoder 31 */
   IFX_uint32_t PT31 : 7;
} __PACKED__ ;

#define  COD_CHAN_RTP_SUP_CFG_US_ECMD   17
#define  COD_CHAN_RTP_SUP_CFG_US_SID_RTP_PT  0x1
#define  COD_CHAN_RTP_SUP_CFG_US_SID_SAME_PT  0x0


/**
   This command allow the setting of the payload type for each of the supported codecs
   ifor the downstream direction. The payload types have to be set before the channel
   is activated. There are separate commands avialable for the setting of the payload
   types for upstream and downstream direction, because some applications have to use
   different payload types for the same codec in upstream and downstream.
   For G.723.1, iLBC and AMR, the same payload type may be used for each bit rate of
   the corresponding codec.
*/
struct COD_CHAN_RTP_SUP_CFG_DS
{
   CMD_HEAD_BE;

/* Instruction 1 */
   /** Reserved */
   IFX_uint32_t Res02 : 1;
   /** Payload Type for Encoder 2 */
   IFX_uint32_t PT2 : 7;
   /** Reserved */
   IFX_uint32_t Res03 : 1;
   /** Payload Type for corresponding Encoder 3 */
   IFX_uint32_t PT3 : 7;
   /** Reserved */
   IFX_uint32_t Res04 : 1;
   /** Payload Type for corresponding Encoder 4 */
   IFX_uint32_t PT4 : 7;
   /** Reserved */
   IFX_uint32_t Res05 : 1;
   /** Payload Type for corresponding Encoder 5 */
   IFX_uint32_t PT5 : 7;

/* Instruction 2 */
   /** Reserved */
   IFX_uint32_t Res06 : 1;
   /** Payload Type for Encoder 6 */
   IFX_uint32_t PT6 : 7;
   /** Reserved */
   IFX_uint32_t Res07 : 1;
   /** Payload Type for corresponding Encoder 7 */
   IFX_uint32_t PT7 : 7;
   /** Reserved */
   IFX_uint32_t Res08 : 1;
   /** Payload Type for corresponding Encoder 8 */
   IFX_uint32_t PT8 : 7;
   /** Reserved */
   IFX_uint32_t Res09 : 1;
   /** Payload Type for corresponding Encoder 9 */
   IFX_uint32_t PT9 : 7;

/* Instruction 3 */
   /** Reserved */
   IFX_uint32_t Res10 : 1;
   /** Payload Type for Encoder 10 */
   IFX_uint32_t PT10 : 7;
   /** Reserved */
   IFX_uint32_t Res11 : 1;
   /** Payload Type for corresponding Encoder 11 */
   IFX_uint32_t PT11 : 7;
   /** Reserved */
   IFX_uint32_t Res12 : 1;
   /** Payload Type for corresponding Encoder 12 */
   IFX_uint32_t PT12 : 7;
   /** Reserved */
   IFX_uint32_t Res13 : 1;
   /** Payload Type for corresponding Encoder 13 */
   IFX_uint32_t PT13 : 7;

/* Instruction 4 */
   /** Reserved */
   IFX_uint32_t Res14 : 1;
   /** Payload Type for Encoder 14 */
   IFX_uint32_t PT14 : 7;
   /** Reserved */
   IFX_uint32_t Res15 : 1;
   /** Payload Type for corresponding Encoder 15 */
   IFX_uint32_t PT15 : 7;
   /** Reserved */
   IFX_uint32_t Res16 : 1;
   /** Payload Type for corresponding Encoder 16 */
   IFX_uint32_t PT16 : 7;
   /** Reserved */
   IFX_uint32_t Res17 : 1;
   /** Payload Type for corresponding Encoder 17 */
   IFX_uint32_t PT17 : 7;

/* Instruction 5 */
   /** Reserved */
   IFX_uint32_t Res18 : 1;
   /** Payload Type for Encoder 18 */
   IFX_uint32_t PT18 : 7;
   /** Reserved */
   IFX_uint32_t Res19 : 1;
   /** Payload Type for corresponding Encoder 19 */
   IFX_uint32_t PT19 : 7;
   /** Reserved */
   IFX_uint32_t Res20 : 1;
   /** Payload Type for corresponding Encoder 20 */
   IFX_uint32_t PT20 : 7;
   /** Reserved */
   IFX_uint32_t Res21 : 1;
   /** Payload Type for corresponding Encoder 21 */
   IFX_uint32_t PT21 : 7;

/* Instruction 6 */
   /** Reserved */
   IFX_uint32_t Res22 : 1;
   /** Payload Type for Encoder 22 */
   IFX_uint32_t PT22 : 7;
   /** Reserved */
   IFX_uint32_t Res23 : 1;
   /** Payload Type for corresponding Encoder 23 */
   IFX_uint32_t PT23 : 7;
   /** Reserved */
   IFX_uint32_t Res24 : 1;
   /** Payload Type for corresponding Encoder 24 */
   IFX_uint32_t PT24 : 7;
   /** Reserved */
   IFX_uint32_t Res25 : 1;
   /** Payload Type for corresponding Encoder 25 */
   IFX_uint32_t PT25 : 7;

/* Instruction 7 */
   /** Reserved */
   IFX_uint32_t Res26 : 1;
   /** Payload Type for Encoder 26 */
   IFX_uint32_t PT26 : 7;
   /** Reserved */
   IFX_uint32_t Res27 : 1;
   /** Payload Type for corresponding Encoder 27 */
   IFX_uint32_t PT27 : 7;
   /** Reserved */
   IFX_uint32_t Res28 : 1;
   /** Payload Type for corresponding Encoder 28 */
   IFX_uint32_t PT28 : 7;
   /** Reserved */
   IFX_uint32_t Res29 : 1;
   /** Payload Type for corresponding Encoder 29 */
   IFX_uint32_t PT29 : 7;

/* Instruction 8 */
   /** Reserved */
   IFX_uint32_t Res30 : 1;
   /** Payload Type for Encoder 30 */
   IFX_uint32_t PT30 : 7;
   /** Reserved */
   IFX_uint32_t Res31 : 1;
   /** Payload Type for corresponding Encoder 31 */
   IFX_uint32_t PT31 : 7;
   /** Reserved */
   IFX_uint32_t Res32 : 16;
} __PACKED__ ;

#define  COD_CHAN_RTP_SUP_CFG_DS_ECMD   25


/**
   This command configures the jitter buffer of the corresponding coder channel. The
   jitter buffer has to be configured before the channel is activated.
*/
struct COD_JB_CONF
{
   CMD_HEAD_BE;
   /** Scaling Factor */
   IFX_uint32_t SF : 8;
   /** Packet Repetition */
   IFX_uint32_t PRP : 1;
   /** Peak Jitter Estimation */
   IFX_uint32_t PJE : 1;
   /** Discard Voice Frames */
   IFX_uint32_t DVF : 1;
   /** Non Adaptive Mode */
   IFX_uint32_t NAM : 1;
   /** Reserved */
   IFX_uint32_t Res01 : 2;
   /** Reduced Adjustment Speed */
   IFX_uint32_t RAD : 1;
   /** Adaptive Jitter Buffer Mode Enable */
   IFX_uint32_t ADAP : 1;
   /** Initial Jitter Buffer Play Out Delay */
   IFX_uint32_t INIT_JOB_POD : 16;
   /** Minimum Jitter Buffer Play Out Delay */
   IFX_uint32_t MIN_JB_POD :16;
   /** Maximum Jitter Buffer Play Out Delay */
   IFX_uint32_t MAX_JB_POD : 16;
   /** Reserved */
   IFX_uint32_t Res02 : 1;
   /** Adaptation factor */
   IFX_uint32_t ADAP_FACTOR : 7;
   /** Minimum margin */
   IFX_uint32_t MIN_MARGIN : 8;
   /** Reserved */
   IFX_uint32_t Res03 : 13;
   /** Smart adaptation */
   IFX_uint32_t SAD : 1;
   /** Jitter adaptation during silence */
   IFX_uint32_t LOC : 1;
   /** Mode - Behavior in case of over- and underflow */
   IFX_uint32_t MODE : 1;
} __PACKED__ ;

#define  COD_JB_CONF_ECMD 18
#define  COD_JB_CONF_LEN 8
#define  COD_JB_CONF_ENH_LEN 12
#define  COD_JB_CONF_OFF  0x1
#define  COD_JB_CONF_ON  0x1
#define  COD_JB_CONF_DVF_ON  0x0
#define  COD_JB_CONF_DVF_OFF  0x1
#define  COD_JB_CONF_NAM_ADAPTIVE  0x0
#define  COD_JB_CONF_NAM_INHIBITED  0x1
#define  COD_JB_CONF_ADAP_FIXED  0x0
#define  COD_JB_CONF_PJE_ON  0x1
#define  COD_JB_CONF_PJE_OFF 0x0
#define  COD_JB_CONF_ADAP_EN 0x1
#define  COD_JB_CONF_ADAP_ADAPTIVE   0x0001
#define  COD_JB_CONF_MODE_NORMAL 0x0
#define  COD_JB_CONF_MODE_RESET  0x1
#define  COD_JB_CONF_SAD_ON 0x1
#define  COD_JB_CONF_SAD_OFF 0x0
#define  COD_JB_CONF_PRP_ON  0x1
#define  COD_JB_CONF_PRP_OFF  0x0
#define  COD_JB_CONF_RAD_ON  0x1
#define  COD_JB_CONF_RAD_OFF  0x0


/**
   This command allows the CPU to read out information, required for RTCP packet
   assembly according to RFC 3550. A write command will reset the complete statistic.If
   the controller requests the RTCP statistic before the first packet has been received
   after the channel activation or after a re synchronization (due to a SSRC switch or
   sequence number and/or timestamp jump) a zero statistic is delivered for the
   receiver statistic. This means that the receivers SSRC, fraction lost, packets lost,
   extended highest sequence number and the inter arrival jitter are set to 0.On a host
   requests of a RTCP statistic a statistic is returned in any case, even if no
   additional packets have been received since the last RTCP statistic request. Under
   these circumstances the fraction lost would be zero, the SSRC, packets lost, the
   extended highest sequence number and the inter arrival jitter would be the same as
   within the previous sent statistic.According to RFC 3550, the host must not send the
   receivers report if the extended highest sequence number is zero or is the same as
   the sequence number of the last delivered receiver statistic.
   A write command resets the whole statistics.
   TBD: Automatic generation, after N seconds.
*/
struct COD_RTCP_SUP_CTRL
{
   CMD_HEAD_BE;
   /** Timestamp */
   IFX_uint32_t TIME_STAMP;
   /** Sender's Packet Count */
   IFX_uint32_t S_PKT_CNT;
   /** Sender's Octet Count */
   IFX_uint32_t S_OCT_CNT;
   /** Receiver's Synchronization Source Value, High Word */
   IFX_uint32_t R_SSRC;
   /** Receiver's Fraction Lost */
   IFX_uint8_t R_FRACT_LOST;
   /** Receiver's Packets Lost - High Byte */
   IFX_uint8_t R_PKTS_LOST_HB;
   /** Receiver's Packets Lost - Low Word */
   IFX_uint16_t R_PKTS_LOST_LW;
   /** Receiver's Extended Highest Sequence Number */
   IFX_uint32_t R_EXT_HSNR;
   /** Receiver's Inter arrival Jitter */
   IFX_uint32_t R_INT_JIT;
} __PACKED__ ;

#define  COD_RTCP_SUP_CTRL_ECMD 19
#define  COD_RTCP_SUP_CTRL_LEN 28

/**
   This command allows the CPU to read out information that is required for
   RTCP-XR Statistics Summary Report Block assembly according to RFC3611. If the
   controller requests the RTCP-XR Statistics Summary Report Block before the
   first packet has been received after the channel activation or after a jitter
   buffer re-synchronization (due to SSRC switch or sequence number and/or
   timestamp jump) a zero statistic is delivered for the statistics summary.
   This means that SSRC, beg_seq, end_seq, lost_packets, duplicate_packets,
   min_jitter, max_jitter, mean_jitter and dev_jitter are all set to zero.
   The Statistic Summary Report Block command may only be read when the
   corresponding coder channel is active.
 */
struct COD_RTCP_XR_STAT_SUM
{
   CMD_HEAD_BE;
   /** RTCP XR Statistics Summary Report Block as defined by RFC3611. */
   IFX_TAPI_PKT_RTCP_XR_BLOCK_STATISTICS_SUMMARY_REPORT  StatisticsSummaryReport;
} __PACKED__ ;

#define  COD_RTCP_XR_STAT_SUM_ECMD 0
#define  COD_RTCP_XR_STAT_SUM_LEN 40

/**
   This command allows the CPU to read out information that is required for
   RTCP-XR VOIP Metrics Report Block assembly according to RFC3611. If the
   controller requests the VOIP Metrics Report Block before the first packet has
   been received after the channel activation or after a jitter buffer
   re-synchronization (due to SSRC switch or sequence number and/or timestamp
   jump) a statistic is delivered for the VOIP metric fields that contains the
   VOIP Metrics Report Block Command reset values, see below.
   The VOIP Metrics Report Block command may only be read when the corresponding
   coder channel is active. Note in order to obtain proper signal level and noise
   level estimates a speech/noise discriminator is needed in the coder channel
   decoder. A speech detector is available in FW versions supporting the so called
   "Jitter Buffer Local Adaptation" feature and this speech detector will be used
   for the speech/noise discrimination for the signal and noise level VOIP Metrics
   Report Block fields. In order to get the metric field regarding echo return
   loss (RERL) the controller has to tell the FW which, if any, LEC resource
   (LECNR) is assigned to which coder channel. This is done via the Associated
   LEC Resource Number command.
 */
struct COD_RTCP_XR_VOIP_MET
{
   CMD_HEAD_BE;
   /** RTCP XR VoIP Metrics Report Block as defined by RFC3611. */
   IFX_TAPI_PKT_RTCP_XR_BLOCK_VOIP_METRICS_REPORT VoIPMetricsReport;
   /** Counter for the number of bursts. Burst definition as stipulated in
       RFC 3611 (RTCP-XR). */
   IFX_uint32_t burst_number;
} __PACKED__ ;

#define  COD_RTCP_XR_VOIP_MET_ECMD 1
#define  COD_RTCP_XR_VOIP_MET_LEN 36
#define  COD_RTCP_XR_VOIP_MET_EXT_LEN 40 /* if XR-BN is set to 1 */

/**
   The Associated LEC Resource Number (LECNR) Command allows the controller
   to tell the FW which, if any, LECNR has been assigned to which coder channel.
   This is necessary to be able to calculate the proper residual echo return
   loss value (RERL). The command is a read/write command.
   If a LEC (ALI/PCM channel) will be associated with a Coder Channel in a call.
   The SW (Tapi) has to issue an Associated LEC Resource Number command with C=1
   and the proper LECNR in order to obtain a valid RELR value for the coder
   channel. If no LEC is associated with the call a command with C=0 has to be
   sent so that a value of "127" (unavailable) is reported in the Voip Metric.
   It is the responsibility of the SW (Tapi) to program the associations correctly,
   FW does not check LECNR configuration correctness.
   Note: Due to timing issues regarding LEC activation, Coder Channel activation
   and Associated LECNR it can happen that the reported RERL value for the first
   few received packets is not 100% correct.
   Furthermore, for third party conferences with one Voip and two FXS participants,
   the Tapi has to set C=0, so that a RERL value of "127" is returned
   in the Voip Metric Summary.
 */
struct COD_ASSOCIATED_LECNR
{
   CMD_HEAD_BE;
   /** Reserved */
   IFX_uint32_t Res01 : 23;
   /** Set if LEC number LECNR is associated with coder channel CHAN. */
   IFX_uint32_t C : 1;
   /* LEC Resource Number of the LEC (ALI or PCM) associated
      with coder channel CHAN. */
   IFX_uint32_t LECNR : 8;
} __PACKED__ ;

#define  COD_ASSOCIATED_LECNR_ECMD 2
#define  COD_ASSOCIATED_LECNR_LEN 4
#define  COD_ASSOCIATED_LECNR_C_CLEAR 0
#define  COD_ASSOCIATED_LECNR_C_SET 1

/**
   This command delivers additional statistic information of the Voice Play Out Unit. A
   read or write access clears the status bit VPOU_STAT. A write command (with length =
   0) will reset the whole statistic.
*/
struct COD_JB_STAT
{
   CMD_HEAD_BE;
   /** Packet Play Out Delay */
   IFX_uint32_t PACKET_POD : 16;
   /** Maximum Packet Play Out Delay */
   IFX_uint32_t MAX_PACKET_POD : 16;
   /** Minimum Packet Play Out Delay */
   IFX_uint32_t MIN_PACKET_POD : 16;
   /** Estimated Jitter Buffer Size */
   IFX_uint32_t JITTER : 16;
   /** Maximum Estimated Jitter Buffer Size */
   IFX_uint32_t MAX_JITTER : 16;
   /** Minimum Estimated Jitter Buffer Size */
   IFX_uint32_t MIN_JITTER : 16;
   /** Total number of Received Packets */
   IFX_uint32_t PACKETS;
   /** Number of Discarded Packets */
   IFX_uint32_t DISCARDED_PACKETS : 16;
   /** Number of Late Packets */
   IFX_uint32_t LATE_PACKETS : 16;
   /** Number of Early Packets */
   IFX_uint32_t EARLY_PACKETS : 16;
   /** Number of Re synchronization */
   IFX_uint32_t RESNC : 16;
   /** Injected Samples */
   IFX_uint32_t IS_UNDERFLOW;
   /** Total Number of Injected Samples */
   IFX_uint32_t IS_NO_UNDERFLOW;
   /** Injected Samples Increment */
   IFX_uint32_t IS_INCREMENT;
   /** Skipped Lost Samples */
   IFX_uint32_t SK_DECREMENT;
   /** Dropped Samples */
   IFX_uint32_t DS_DECREMENT;
   /** Dropped Samples Overflow */
   IFX_uint32_t DS_OVERFLOW;
   /** Comfort Noise Samples */
   IFX_uint32_t SID_HW;
   /* Received Bytes */
   IFX_uint32_t RECEIVED_BYTES;
} __PACKED__ ;

#define  COD_JB_STAT_ECMD 20
#define  COD_JB_STAT_LEN 56


/**
   This command reflects the current decoder status, which means the currently used
   decoder and the packet time. A read access will clear the status bit DEC_CHG.
   The bits PTC and DC can be changed via a write command. The bits PTD and DEC are
   read only.
   The packet time monitoring is disabled during silence periods to avoid that a
   transition from voice to silence and vice versa could be interpreted as packet time
   change. Therefore the packet size PTD is equal to the packet size of the last
   received voice packet. Until the first voice packet has been received, PTD is set to
   0.After a coder switch or after the channel activation the DEC bits might indicate
   the wrong G.711 coding if the first packet, which is send by the G.711 encoder at
   the far end side, is a G.711 SID packet. In the case of G.711 SID packets the G.711
   coding is set to the G.711 coding of the last received G.711 voice packet. If no
   voice packet is available at that time, it is assumed that the G.711 coding is
   A-Law.If silence suppression is used for the G.726 coder, G.711 SID packets are sent
   within silence periods. If a G.711 SID packet is received, the decoder can not
   distinguish between a coder switch to G.711 and the beginning of a G.726 silence
   period. Therefore a kind of coder switch is detected for the transitions G.726 voice
   packet to G.711 SID packet and G.711 SID packet to G.726 voice packet in any case.
   During the silence period the decoder coding is set to G.711 A-Law.
*/
struct COD_DEC_STAT
{
   CMD_HEAD_BE;
   /** Packet Time Duration */
   IFX_uint8_t PTD;
   /** Packet Time Change */
   IFX_uint32_t PTC : 1;
   /** Decoder Change */
   IFX_uint32_t DC : 1;
   /** Codec Mode Request Change */
   IFX_uint32_t CMRC : 1;
   /** Type of Decoder */
   IFX_uint32_t DEC : 5;
   /** Reserved */
   IFX_uint32_t Res02 : 12;
   /** Codec Mode Request */
   IFX_uint32_t CMR : 4;
} __PACKED__ ;

#define  COD_DEC_STAT_ECMD 21
#define  COD_DEC_STAT_LEN 4
#define  COD_DEC_STAT_ON  0x1
#define  COD_DEC_STAT_PTC_ON 1
#define  COD_DEC_STAT_PTC_OFF 0
#define  COD_DEC_STAT_DC_ON 1
#define  COD_DEC_STAT_DC_OFF 0


/**
   This command defines a set of contributing sources as conferencing information for
   upstream traffic. A contributing source can be given as a channel selection
   (corresponding bit in the channel bit mask must be set to 1) or immediately as a 32
   bit CSRC.
*/
struct COD_RTP_CONF_SUPP
{
   CMD_HEAD_BE;
   /** Bit mask of Conferencing Channels */
   IFX_uint16_t CHAN_MASK;
   /** Reserved */
   IFX_uint32_t Res : 12;
   /** Counter of Contributing Sources */
   IFX_uint32_t CC : 4;
   /** Contributing Source 1 */
   IFX_uint32_t CSRC1;
   /** Contributing Source 2 */
   IFX_uint32_t CSRC2;
   /** Contributing Source 3 */
   IFX_uint32_t CSRC3;
} __PACKED__ ;


/**
   This command lists 32-bit SSRC and 32-bit CSRCs of a channel as conferencing
   information from downstream traffic. The number of stored Contributing Sources
   (CSRCs) is limited to 2.
*/
struct COD_RTP_CONF_STAT
{
   CMD_HEAD_BE;
   /** Reserved */
   IFX_uint32_t Res : 12;
   /** Number of Contributing Sources */
   IFX_uint32_t NR_CC_SC : 4;
   /** Reserved */
   IFX_uint32_t Res01 : 16;
   /** Synchronization Source */
   IFX_uint32_t SSRC;
   /** Contributing Source 1 */
   IFX_uint32_t CSRC1;
   /** Contributing Source 2 */
   IFX_uint32_t CSRC2;
} __PACKED__ ;


/**
   This command allows the generation of RTP Events according to RFC 4733
   (obsolete RFC 2833). By the use of this command, the control CPU can cause
   the coder channel to insert events in the RTP stream.
*/
struct COD_EVT_GEN
{
   CMD_HEAD_BE;
   /** Start Stop */
   IFX_uint32_t ST : 1;
   /** Use volume field */
   IFX_uint32_t UV : 1;
   /** Volume */
   IFX_uint32_t VOL : 6;
   /** Event Duration */
   IFX_uint32_t ED : 8;
   /** Reserved */
   IFX_uint32_t Res01 : 8;
   /** EVENT */
   IFX_uint32_t EVENT : 8;
} __PACKED__ ;

#define  COD_EVT_GEN_ECMD 25
#define  COD_EVT_GEN_LEN 4
#define  COD_EVT_GEN_START  0x1
#define  COD_EVT_GEN_ON  0x1


/**
   This command activates one FAX data pump coder channel and determines if the
   modulator or demodulator is active. Before this command can be sent the coder
   channel has to be activated.
   MOBSM and MOBRD can be chosen in any way, that means that MOBSM can be equal,
   greater, or less than MOBRD. MOBSM determines the start of the modulator and MOBRD
   is the level where the modulator requests new data.
   MOBSM, MOBRD, and DMBSD are defined in time units to make the buffering independent
   from the bit rate. The data pump automatically calculates the correspondence buffer
   size in bytes after the bit rate has been detected by the demodulator or within the
   initialization in the case of modulation.
*/
struct COD_FAX_CTRL
{
   CMD_HEAD_BE;
   /** Enable */
   IFX_uint32_t EN : 1;
   /** Mode */
   IFX_uint32_t MD : 1;
   /** Reserved */
   IFX_uint32_t Res01 : 2;
   /** Data Pump Resource Number */
   IFX_uint32_t DPNR : 4;
   /** Reserved */
   IFX_uint32_t Res02 : 2;
   /** Input Signal */
   IFX_uint32_t I1 : 6;
   /** Gain for Upstream (Demodulation) Direction */
   IFX_uint32_t GAIN1 : 16;
   /** Gain for Downstream (Modulation) Direction */
   IFX_uint32_t GAIN2 : 16;
   /** Modulation Buffer, Level for Start Modulation */
   IFX_uint16_t MOBSM;
   /** Modulation Buffer, Request More Data */
   IFX_uint16_t MOBRD;
   /** Demodulation Buffer, Send Data Level */
   IFX_uint16_t DMBSD;
} __PACKED__ ;

#define  COD_FAX_CTRL_ECMD  8
#define  COD_FAX_CTRL_LEN  12
#define  COD_FAX_CTRL_ACTIVE  0x1
#define  COD_FAX_CTRL_MD_MODULATION  0x0
#define  COD_FAX_CTRL_MD_DEMODULATION  0x1


/**
   This command has to be sent before the Modulator is activated.
*/
struct COD_FAX_MOD_CTRL
{
   CMD_HEAD_BE;
   /** Reserved */
   IFX_uint32_t Res01 : 2;
   /** Desired Output Signal Power */
   IFX_uint32_t DBM : 6;
   /** Reserved */
   IFX_uint32_t Res02 : 1;
   /** Talker Echo Protection Signal */
   IFX_uint32_t TEP : 1;
   /** Training Sequence */
   IFX_uint32_t TRN : 1;
   /** Standard */
   IFX_uint32_t STD : 5;
   /** Reserved */
   IFX_uint32_t Res03 : 1;
   /** Signal Duration [in ms] */
   IFX_uint32_t SGLEN : 15;
} __PACKED__ ;

#define  COD_FAX_MOD_CTRL_ECMD  26
#define  COD_FAX_MOD_CTRL_NO  0x0
#define  COD_FAX_MOD_CTRL_YES  0x1
#define  COD_FAX_MOD_CTRL_TRN_SHORT  0x0
#define  COD_FAX_MOD_CTRL_TRN_LONG  0x1
#define  COD_FAX_MOD_CTRL_STD_SILENCE  0
#define  COD_FAX_MOD_CTRL_STD_V21  1
#define  COD_FAX_MOD_CTRL_STD_V27_2400  2
#define  COD_FAX_MOD_CTRL_STD_V27_4800  3
#define  COD_FAX_MOD_CTRL_STD_V29_7200  4
#define  COD_FAX_MOD_CTRL_STD_V29_9600  5
#define  COD_FAX_MOD_CTRL_STD_V17_7200  6
#define  COD_FAX_MOD_CTRL_STD_V17_9600  7
#define  COD_FAX_MOD_CTRL_STD_V17_12000  8
#define  COD_FAX_MOD_CTRL_STD_V17_14400  9
#define  COD_FAX_MOD_CTRL_STD_CNG  10
#define  COD_FAX_MOD_CTRL_STD_CED  11
#define  COD_FAX_MOD_CTRL_MAX_SGLEN     4000
#define  COD_FAX_MOD_CTRL_SGLEN         0x7FFF


/**
   This command has to be sent before the demodulator is activated.
   Two alternative standards were introduced to resolve ambiguity in the FAX protocol.
   Sometimes the receiving side does not know the next standard for the demodulation.
   In this case the upper layer software must supply two alternatives to the Fax Data
   Pump. In general STD1 is one of the page standards and STD2 is V.21. On the other
   side the alternative standard is undesirable in some situations. Therefore the STD2
   can be disabled via the coding 11111.
*/
struct COD_FAX_DEMOD_CTRL
{
   CMD_HEAD_BE;
   /** Training Sequence */
   IFX_uint32_t TRN : 1;
   /** Equalizer */
   IFX_uint32_t EQ : 1;
   /** Reserved */
   IFX_uint32_t Res01 : 4;
   /** Alternative Desired Standard */
   IFX_uint32_t STD2 : 5;
   /** Desired Standard */
   IFX_uint32_t STD1 : 5;
   /** Reserved */
   IFX_uint32_t Res02 : 16;
} __PACKED__ ;

#define  COD_FAX_DEMOD_CTRL_ECMD  27
#define  COD_FAX_DEMOD_CTRL_TRN_SHORT  0x0
#define  COD_FAX_DEMOD_CTRL_TRN_LONG  0x1
#define  COD_FAX_DEMOD_CTRL_EQ_REUSE  0x1
#define  COD_FAX_DEMOD_CTRL_STD_V27_2400  2
#define  COD_FAX_DEMOD_CTRL_STD_V27_4800  3
#define  COD_FAX_DEMOD_CTRL_STD_V29_7200  4
#define  COD_FAX_DEMOD_CTRL_STD_V29_9600  5
#define  COD_FAX_DEMOD_CTRL_STD_V17_7200  6
#define  COD_FAX_DEMOD_CTRL_STD_V17_9600  7
#define  COD_FAX_DEMOD_CTRL_STD_V17_12000  8
#define  COD_FAX_DEMOD_CTRL_STD_V17_14400  9
#define  COD_FAX_DEMOD_CTRL_STD_USE_STD1  32
#define  COD_FAX_DEMOD_CTRL_STD1_V27_2400  2
#define  COD_FAX_DEMOD_CTRL_STD1_V27_4800  3
#define  COD_FAX_DEMOD_CTRL_STD1_V29_7200  4
#define  COD_FAX_DEMOD_CTRL_STD1_V29_9600  5
#define  COD_FAX_DEMOD_CTRL_STD1_V17_7200  6
#define  COD_FAX_DEMOD_CTRL_STD1_V17_9600  7
#define  COD_FAX_DEMOD_CTRL_STD1_V17_12000  8
#define  COD_FAX_DEMOD_CTRL_STD1_V17_14400  9
#define  COD_FAX_DEMOD_CTRL_STD1 0x001F
#define  COD_FAX_DEMOD_CTRL_STD2 0x001F


/*
   The command provides the functionality to read out supported T.38
   implementation capabilities, which should be used by the signaling protocols
   like H.323, SIP/SDP or MDC/MGCP for negotiation of T.38 capabilities during
   call establishment. Capabilities for TCP and UDP may be obtained by this
   command. Note that only UDP is supported by the current implementation.
   The command may be issued any time, but is intended to be called during
   system initialization as T.38 capabilities are fixed and correspond to
   implemented features.
*/
struct COD_FAX_READ_CAP
{
   CMD_HEAD_BE;
   /** Reserved */
   IFX_uint32_t Res01 : 4;
   /** Supported transport protocol */
   IFX_uint32_t TPORT : 4;
   /** UDP error correction method */
   IFX_uint32_t UDPEC : 8;
   /** Maximum bit rate used by FAX data pump */
   IFX_uint32_t MAXBITRATE : 16;
   /** Reserved */
   IFX_uint32_t Res02 : 4;
   /** Rate management method for UDP */
   IFX_uint32_t UDPRMM : 4;
   /** T.38 ASN.1 version */
   IFX_uint32_t T38VER : 8;
   /** Bit options for T.38 */
   IFX_uint32_t BITOPT : 16;
   /** UDP maximum buffer size */
   IFX_uint32_t UDPMBS : 16;
   /** UDP maximum datagram size */
   IFX_uint32_t UDPMDS : 16;
} __PACKED__ ;

#define COD_FAX_READ_CAP_ECMD 7
#define COD_FAX_READ_CAP_LEN 12
#define COD_FAX_READ_CAP_TPORT_TCP 0x1
#define COD_FAX_READ_CAP_TPORT_UDP 0x2
#define COD_FAX_READ_CAP_UDPEC_RED 0x1
#define COD_FAX_READ_CAP_UDPEC_FEC 0x2
#define COD_FAX_READ_CAP_RMM_LOC_TCF 0x1
#define COD_FAX_READ_CAP_RMM_TRANS_TCF 0x2
#define COD_FAX_READ_CAP_BITOPT_FBMR 0x0001
#define COD_FAX_READ_CAP_BITOPT_TMMR 0x0002
#define COD_FAX_READ_CAP_BITOPT_TJBIG 0x0004


/** FAX Channel Activation */
struct COD_FAX_ACT
{
   CMD_HEAD_BE;
   /** Enable */
   IFX_uint32_t EN : 1;
   /** Internal sampling rate */
   IFX_uint32_t ISR : 1;
   /** Reserved */
   IFX_uint32_t Res02 : 2;
   /** T.38 resource number */
   IFX_uint32_t T38R : 4;
   /** Input signal */
   IFX_uint32_t I1 : 8;
   /** Maximum bit rate */
   IFX_uint32_t MAXBITRATE : 16;
   /** Transport protocol */
   IFX_uint32_t TRPR : 4;
   /** Rate management method */
   IFX_uint32_t RMM : 4;
   /** T.38 ASN.1 version */
   IFX_uint32_t T38VER : 8;
   /** Bit options for T.38 */
   IFX_uint32_t BITOPT : 16;
   /** UDP maximum buffer size */
   IFX_uint32_t UDPMBS : 16;
   /** UDP maximum datagram size */
   IFX_uint32_t UDPMDS : 16;
   /** UDP error correction method */
   IFX_uint32_t UDPEC : 8;
   /** Reserved */
   IFX_uint32_t Res03 : 24;
} __PACKED__ ;

#define COD_FAX_ACT_ECMD 5
#define COD_FAX_ACT_LEN 16
#define COD_FAX_ACT_OFF 0
#define COD_FAX_ACT_ON 1
#define COD_FAX_ACT_ISR_8KHZ 0
#define COD_FAX_ACT_ISR_16KHZ 1
#define COD_FAX_ACT_TRPR_UDP 1
#define COD_FAX_ACT_RMM_LOC_TCF 1
#define COD_FAX_ACT_RMM_TRANS_TCF 2
#define COD_FAX_ACT_T38VER_VERS0 0
#define COD_FAX_ACT_BITOPT_FBMR 0x0001
#define COD_FAX_ACT_BITOPT_TMMR 0x0002
#define COD_FAX_ACT_BITOPT_TJBIG 0x0004
#define COD_FAX_ACT_UDPFEC_RED 1
#define COD_FAX_ACT_UDPFEC_FEC 2


/** FAX Configuration Command */
struct COD_FAX_CONF
{
   CMD_HEAD_BE;
   /** Reserved */
   IFX_uint32_t Res01 : 16;
   /** Gain for upstream (demodulation) direction */
   IFX_uint32_t GAIN1 : 16;
   /** Gain for downstream (modulation) direction */
   IFX_uint32_t GAIN2 : 16;
   /** IFP packets send interval */
   IFX_uint32_t IFPSI : 16;
   /** T.38 options */
   IFX_uint32_t T38OPT : 16;
   /** Desired output power level */
   IFX_uint32_t DBM : 6;
   /** Number of additional recovery data packets sent on high-speed FAX transmissions */
   IFX_uint32_t HRED : 3;
   /** Number of additional recovery data packets sent on low-speed FAX transmissions */
   IFX_uint32_t LRED : 3;
   /** Number of packets to calculate FEC */
   IFX_uint32_t FEC : 4;
   /** Data wait time */
   IFX_uint32_t DWT : 7;
   /** Number of additional recovery T30_INDICATOR packets */
   IFX_uint32_t IRED : 3;
   /** Timeout for start of T.38 modulation */
   IFX_uint32_t ASWT : 12;
   /** Time to insert spoofing during automatic modulation1) */
   IFX_uint32_t AST : 10;
   /** Number of data bytes of NSX field2) */
   IFX_uint32_t NSXLEN : 8;
   /** First byte of NSX */
   IFX_uint32_t NSX1 : 8;
   /** Second byte of NSX2) */
   IFX_uint32_t NSX2 : 8;
   /** Third byte of NSX2) */
   IFX_uint32_t NXS3 : 8;
   /** Fourth byte of NSX2) */
   IFX_uint32_t NSX4 : 8;
   /** Fifth byte of NSX2) */
   IFX_uint32_t NSX5 : 8;
   /** Sixth byte of NSX2) */
   IFX_uint32_t NSX6 : 8;
   /** Seventh byte of NSX2) */
   IFX_uint32_t NXS7 : 8;
} __PACKED__ ;

#define COD_FAX_CONF_ECMD 13
#define COD_FAX_CONF_LEN 24
#define COD_FAX_CONF_T38OPT_NON 0x02
#define COD_FAX_CONF_T38OPT_ASN1 0x04
#define COD_FAX_CONF_T38OPT_LONG 0x08
#define COD_FAX_CONF_T38OPT_ECM 0x10
#define COD_FAX_CONF_T38OPT_ALL 0x40


/** FAX Data Pump Parameters */
struct COD_FAX_FDP_PARAMS
{
   CMD_HEAD_BE;
   /** Modulation buffer size */
   IFX_uint32_t MOBSZ : 16;
   /** Modulation buffer fill level for modulation start */
   IFX_uint32_t MOBSM : 16;
   /** Modulation buffer fill level for generation of a data request */
   IFX_uint32_t MOBRD : 16;
   /** Demodulation buffer send data level */
   IFX_uint32_t DMBSD : 16;
} __PACKED__ ;

#define COD_FAX_FDP_PARAMS_ECMD 15
#define COD_FAX_FDP_PARAMS_LEN 8


/** FAX Session Statistics */
struct COD_FAX_STAT
{
   CMD_HEAD_BE;
   /** T.38 session flags */
   IFX_uint32_t T38SESSFL : 16;
   /** FAX data pump standards used during session */
   IFX_uint32_t FDPSTAND : 16;
   /** Number of lost packets */
   IFX_uint32_t T38_PKTS_LOST : 32;
   /** Number of received packets */
   IFX_uint32_t T38_PKTS_REC : 32;
   /** Maximum number of consecutively lost packets */
   IFX_uint32_t T38_PKTS_LOSTGROUP : 16;
   /** State of facsimile transmission */
   IFX_uint32_t T38_PKTS_FAX_STATE : 16;
   /** Number of FTT responses */
   IFX_uint32_t FTTNUM : 16;
   /** Number of transmitted pages */
   IFX_uint32_t TXPAGES : 16;
   /** Number of scan line breaks during modulation */
   IFX_uint32_t LINEBREAK : 32;
   /** Number of facsimile control frame line breaks during modulation */
   IFX_uint32_t V21FRM_BREAK : 16;
   /** Number of ECN frame breaks during modulation */
   IFX_uint32_t ECMFRM_BREAK : 16;
   /** Major version of T.38 implementation */
   IFX_uint32_t T38_VER_MAJ : 16;
   /** Minor version of T.38 implementation */
   IFX_uint32_t T38_VER_MIN : 16;
} __PACKED__ ;

#define COD_FAX_STAT_ECMD 6
#define COD_FAX_STAT_LEN 32
#define COD_FAX_STAT_T38SESSFL_FEC 0x1
#define COD_FAX_STAT_T38SESSFL_RED 0x2
#define COD_FAX_STAT_T38SESSFL_ECM 0x4
#define COD_FAX_STAT_T38SESSFL_T30COMPL 0x8
#define COD_FAX_STAT_FDPSTAND_V27_2400 0x01
#define COD_FAX_STAT_FDPSTAND_V27_4800 0x02
#define COD_FAX_STAT_FDPSTAND_V29_7200 0x04
#define COD_FAX_STAT_FDPSTAND_V29_9600 0x08
#define COD_FAX_STAT_FDPSTAND_V17_7200 0x10
#define COD_FAX_STAT_FDPSTAND_V17_9600 0x20
#define COD_FAX_STAT_FDPSTAND_V17_12000 0x40
#define COD_FAX_STAT_FDPSTAND_V17_14400 0x80


/** FAX Trace Command */
struct COD_FAX_TRACE
{
   CMD_HEAD_BE;
   /** Feature */
   IFX_uint32_t FEATURE : 8;
   /** Enable trace mode */
   IFX_uint32_t EN : 1;
   /** Feature */
   IFX_uint32_t Res01 : 15;
   /** Payload type of T.38 trace packet */
   IFX_uint32_t TRC_PT : 8;
   /** Debug mask */
   IFX_uint32_t DBGMASK : 32;
} __PACKED__ ;

#define COD_FAX_TRACE_ECMD 14
#define COD_FAX_TRACE_LEN 8
#define COD_FAX_TRACE_FEATURE_T38TRACE 7
#define COD_FAX_TRACE_OFF 0
#define COD_FAX_TRACE_ON 1
#define COD_FAX_TRACE_Res01_T38TRACE 7


/**
   This command activates a DECT channel in speech compression mode.
   The coder can only be a narrowband coder, if the bit ISR is set to 0. If the bit ISR
   is set to 1, the coder can be a narrowband or a wideband coder. If for ISR=1, a
   narrowband decoder is active, the samples are automatically interpolated to 16 kHz
   before they are passed to the signal array. If for ISR=1, a narrowband encoder is
   chosen, the samples from the signal array are automatically decimated to 8 kHz
   before they are passed to the encoder.
   The bit ISR must not be changed when the coder channel is active.
*/
struct DECT_CHAN_SPEECH
{
   CMD_HEAD_BE;
   /** Enable */
   IFX_uint32_t EN : 1;
   /** Internal Sampling Rate */
   IFX_uint32_t ISR : 1;
   /** Noise Level */
   IFX_uint32_t NS : 1;
   /** Encoder Packet Time */
   IFX_uint32_t PTE : 2;
   /** Reserved */
   IFX_uint32_t Res01 : 5;
   /** Signal Array Address of Input Signal 1 */
   IFX_uint32_t I1 : 6;
   /** DC-High Pass */
   IFX_uint32_t HP : 1;
   /** Reserved */
   IFX_uint32_t Res02 : 1;
   /** Comfort Noise Generation (Decoder) */
   IFX_uint32_t CNG : 1;
   /** Bad Frame Interpolation (Decoder) */
   IFX_uint32_t BFI : 1;
   /** Decoder Path Status */
   IFX_uint32_t DEC : 1;
   /** Reserved */
   IFX_uint32_t Res03 : 1;
   /** Periodical SID Transmission */
   IFX_uint32_t PST : 1;
   /** Silence Compression of the Encoder */
   IFX_uint32_t SIC : 1;
   /** Reserved */
   IFX_uint32_t Res04 : 1;
   /** Add Signal of the UTG to Signal Path */
   IFX_uint32_t ADD : 1;
   /** Nibble Swap */
   IFX_uint32_t NSWAP :1;
   /** Encoder Algorithm */
   IFX_uint32_t ENC : 5;
   /** Gain Transmit */
   IFX_uint32_t GAIN1 : 16;
   /** Gain Receive */
   IFX_uint32_t GAIN2 : 16;
   /** Reserved */
   IFX_uint32_t Res05 : 2;
   /** Input 2 */
   IFX_uint32_t I2 : 6;
   /** Reserved */
   IFX_uint32_t Res06 : 2;
   /** Input 3 */
   IFX_uint32_t I3 : 6;
   /** Reserved */
   IFX_uint32_t Res07 : 2;
   /** Input 4 */
   IFX_uint32_t I4 : 6;
   /** Reserved */
   IFX_uint32_t Res08 : 2;
   /** Input 5 */
   IFX_uint32_t I5 : 6;
   /** Reserved */
   IFX_uint32_t Res09 : 16;
   /** Reserved */
   IFX_uint32_t Res10 : 8;
   /** Encoder Start Delay */
   IFX_uint32_t EncDelay : 4;
   /** Decoder Start Delay */
   IFX_uint32_t DecDelay : 4;
} __PACKED__ ;

#define  DECT_ENC_PTE_2_5MS     0
#define  DECT_ENC_PTE_5MS       1
#define  DECT_ENC_PTE_10MS      2

/** Mute the signal from the far end,
    when generating a tone with the DECT UTG */
#define  DECT_CHAN_SPEECH_ADD_SIGNAL_UTG_OFF    0
/** Add the signal of the far end to the tone,
    when generating a tone with the DECT UTG
    Note: If UTG is not active ADD flag is ignored. */
#define  DECT_CHAN_SPEECH_ADD_SIGNAL_UTG_ON     1

#define  DECT_CHAN_SPEECH_ENC_G726_32    1
#define  DECT_CHAN_SPEECH_ENC_G711_ALAW  2
#define  DECT_CHAN_SPEECH_ENC_G711_MLAW  3
#define  DECT_CHAN_SPEECH_ENC_G722_64    4


/**
   This command delivers statistic information of the DECT Coder Channel.
   A read or write access clears the status bit VPOU_STAT. A write command
   (with length = 0) will reset the whole statistic.
*/
struct DECT_CODER_STAT
{
   CMD_HEAD_BE;
   /* Host to FP Packet Count. The total number of upstream DECT data packets
      (host to FP ) transmitted since start of transmission (voice packets). */
   IFX_uint32_t H_FP_PKTS_CNT : 32;
   /* FP to Host Packet Count. The total number of downstream DECT data packets
     (FP to Host) received since starting transmission (voice packets). */
   IFX_uint32_t FP_H_PKTS_CNT : 32;
   /* Number of FP to Host SID Packets. Number of SID packets received from FP. */
   IFX_uint32_t FP_H_SID_PKTS_CNT : 32;
   /* Number of FP to Host PLC Packets. Number of PLC packets received from FP. */
   IFX_uint32_t FP_H_PLC_PKTS_CNT : 32;
   /* Number of FP to Host Buffer Overflows. Number of packets that have to be
      discarded due to overflow. */
   IFX_uint32_t FP_H_OVFL_CNT : 32;
   /* Number of FP to Host Buffer Underflows. Number of decoder buffer
     underflows every 2.5 ms. The decoder runs on 2.5 ms packets. */
   IFX_uint32_t FP_H_UNFL_CNT : 32;
   /* Number of FP to Host Invalid Packets. Number of invalid packets that
      arrive downstream from handset. */
   IFX_uint32_t FP_H_INVA_PKTS_CNT : 32;
} __PACKED__ ;

#define  DECT_CODER_STAT_ECMD  2


/**
   This command activates the DECT universal tone generator in the addressed
   DECT channel.
   The DECT channel has to be activated before this command can be sent.
   Before the UTG can be activated, the coefficients have to be programmed.
*/
struct DECT_UTG_CTRL
{
   CMD_HEAD_BE;
   /** Enable */
   IFX_uint32_t EN : 1;
   /** Stop Mask */
   IFX_uint32_t SM : 1;
   /** Square Wave Select */
   IFX_uint32_t SQ : 1;
   /** Fade-In/Out Logarithmic Select */
   /* LOG is a macro so this is named _LOG */
   IFX_uint32_t _LOG : 1;
   /** Immediate stop */
   IFX_uint32_t IMS : 1;
   /** Enable UTG status event */
   IFX_uint32_t EU : 1;
   /** Reserved */
   IFX_uint32_t Res01 : 2;
   /** UTG Resource Number */
   IFX_uint32_t UTGNR : 8;
   /** Reserved */
   IFX_uint32_t Res02 : 16;
} __PACKED__ ;

#define  DECT_UTG_CTRL_ECMD  3


/**
   This command activates or deactivates one signaling channel. The detectors and
   generator in the signaling channel always run in narrowband mode (8 kHz sampling
   rate). optional interpolators and decimators are provided to process wideband
   samples.
*/
struct SIG_CHAN
{
   CMD_HEAD_BE;
   /** Enable */
   IFX_uint32_t EN : 1;
   /** Enable Event Support */
   IFX_uint32_t ES : 1;
   /** Input Signal 1 */
   IFX_uint32_t I1 : 6;
   /** Internal Sampling Rate */
   IFX_uint32_t ISR : 1;
   /** Reserved */
   IFX_uint32_t Res01 : 1;
   /** Input Signal 2 */
   IFX_uint32_t I2 : 6;
   /** Mute Signal Path 1 */
   IFX_uint32_t MUTE1 : 1;
   /** Mute Signal Path 2 */
   IFX_uint32_t MUTE2 : 1;
   /** Reserved */
   IFX_uint32_t Res02 : 14;
} __PACKED__ ;

#define  SIG_CHAN_ECMD  1
#define  SIG_CHAN_LEN  4
#define  SIG_CHAN_ENABLE  0x1
#define  SIG_CHAN_DISABLE  0x0
#define  SIG_CHAN_8KHZ  0x0
#define  SIG_CHAN_16KHZ  0x1


/**
   This command activates one of the CID Senders in the addressed signaling channel.
   The selected signaling channel has to be activated before this command can be
   sent.If the desired CID Sender coefficients differ from the provided default values,
   they coefficients have to be programmed with the command Caller ID Sender
   Coefficients on Page143 prior to the activation of the CID.
   After the activation of the CID Sender, the sender is waiting for the first data
   bytes. Once the host has sent the minimum required data bytes (coefficient BRS), the
   CID Sender starts the transmission.
   If the auto ring mode is off, the CID Sender is waiting for the first data bytes
   after the activation of the CID Sender. Once the host has sent the minimum required
   data bytes (at least RBS, see Caller ID Sender Coefficients on Page143, number of
   bytes have to be sent at the beginning see coefficient RBS), the CID Sender starts
   the transmission automatically. If the CID Sender requests additional data, the
   status bit CID_REQ is set. After the host has sent all available data bytes, it can
   deactivate the CID Sender with Auto Deactivation on (AD=1). The host can deactivate
   the CID Sender with Auto Deactivation off (AD=0) at any time. In this case the CID
   Sender stops the transmission immediately even if there are still data available in
   the internal buffer.

   If the auto ring mode is on, the host has to send at first the minimum required data
   bytes (see bit field BRS in command "Caller ID Sender Coefficients"). Then the CID
   Sender waits until the auto ring function allows the Sender to transmit the CID.
   If the CID transmission has been started the CID Sender requests new data in the same
   way as in normal operation. If the host has no additional data, the host should
   deactivate the CID Sender with Auto Deactivation on (AD=1). It is not allowed that
   the host deactivates the CID Sender with Auto Deactivation on (AD=1) before the
   CID Sender has requested additional bytes. Of course the host can deactivate the
   CID Sender with Auto Deactivation off (AD=0) at any time. In this case the CID Sender
   stops the transmission immediately.
   Originally the FSK sender is equipped with a band pass filter, which filters the
   FSK signal and restricts it better to its intended bandwidth. Nevertheless experience
   showed, that some phones do not receive the FSK Caller ID correctly, when the filter
   is enabled. Therefore the option is provided to switch the band pass filter off or
   on with bit EB (enable band pass filter). EB=0 is the recommended setting, because
   it enables Caller ID reception also on those phones, which do not fully comply to
   the standard.
   Restrictions
   The signaling channel has to be activated before the Caller ID sender is switched on.
   The Caller ID sender has to be switched off before deactivation of the signaling channel.
   The Caller ID sender has to be deactivated with the same resource number, that was
   used for activation. If Caller ID sender coefficients, which differ from the default
   values, are required, they should be programmed with the command "Caller ID Sender
   Coefficients", before the generator is activated.
   The bits HLEV, V23, AR and CISNR must not be changed, when the Caller ID Sender is active.
*/
struct SIG_CIDS_CTRL
{
   CMD_HEAD_BE;
   /** Enable */
   IFX_uint32_t EN : 1;
   /** Enable bandpass filter */
   IFX_uint32_t EB : 1;
   /** Auto Deactivation */
   IFX_uint32_t AD : 1;
   /** High Level CID Generation Mode */
   IFX_uint32_t HLEV : 1;
   /** CID Specification */
   IFX_uint32_t V23 : 1;
   /** Auto Ring Mode */
   IFX_uint32_t AR : 1;
   /** Analog Channel */
   IFX_uint32_t AC : 1;
   /** Reserved */
   IFX_uint32_t Res02 : 1;
   /** Add Signal to Signal Path 1 */
   IFX_uint32_t ADD_1 : 2;
   /** Add Signal to Signal Path 2 */
   IFX_uint32_t ADD_2 : 2;
   /** CID Sender Resource Number */
   IFX_uint32_t CISNR : 4;
   /** Event Mask */
   IFX_uint32_t EVM : 3;
   /** Reserved */
   IFX_uint32_t Res03 : 13;
} __PACKED__ ;

#define  SIG_CIDS_CTRL_ECMD  2
#define  SIG_CIDS_CTRL_LEN  4
#define  SIG_CIDS_CTRL_DISABLE  0x0
#define  SIG_CIDS_CTRL_ENABLE  0x1
#define  SIG_CIDS_CTRL_ON  0x1
#define  SIG_CIDS_CTRL_HLEV_HIGH  0x1
#define  SIG_CIDS_CTRL_V23_ITU_T  0x1
#define  SIG_CIDS_CTRL_Channel1  0x1
#define  SIG_CIDS_CTRL_AD_ON     0x1
#define  SIG_CIDS_CTRL_ADD1_ON   0x1
#define  SIG_CIDS_CTRL_ADD2_ON   0x1
#define  SIG_CIDS_CTRL_EVM_RDY   1
#define  SIG_CIDS_CTRL_EVM_BUF_REQ 2
#define  SIG_CIDS_CTRL_EVM_BUF_UNDERFLOW 4


/**
   This command activates one of the DTMF/AT Generators in the addressed signaling
   channel. The selected signaling channel has to be activated before this command can
   be sent.If DTMF/AT Generator coefficients, which differ from the default values, are
   required, they have to be programmed with a separate command before the DTMF/AT
   Generator is activated.
*/
struct SIG_DTMFATG_CTRL
{
   CMD_HEAD_BE;
   /** Status of DTMF/AT Generator */
   IFX_uint32_t EN : 1;
   /** Event Mask */
   IFX_uint32_t EVM : 3;
   /** Auto Deactivation */
   IFX_uint32_t AD : 1;
   /** Timing Control Mode */
   IFX_uint32_t MD : 1;
   /** Frequency Generation */
   IFX_uint32_t FG : 1;
   /** Reserved */
   IFX_uint32_t Res02 : 1;
   /** Add Signal to Signal Path 1 */
   IFX_uint32_t ADD_1 : 2;
   /** Add Signal to Signal Path 2 */
   IFX_uint32_t ADD_2 : 2;
   /** DTMF/AT Generator Resource Number */
   IFX_uint32_t GENNR : 4;
   /** Reserved */
   IFX_uint32_t Res03 : 16;
} __PACKED__ ;

#define  SIG_DTMFATG_CTRL_ECMD  3
#define  SIG_DTMFATG_CTRL_LEN  4
#define  SIG_DTMFATG_CTRL_ENABLE  0x1
#define  SIG_DTMFATG_CTRL_DISABLE  0x0
#define  SIG_DTMFATG_CTRL_ON  0x1
#define  SIG_DTMFATG_CTRL_MOD_HIGH  0x1
#define  SIG_DTMFATG_CTRL_MOD_LOW  0x0
#define  SIG_DTMFATG_CTRL_FG_HIGH  0x1
#define  SIG_DTMFATG_CTRL_FG_LOW  0x0
#define  SIG_DTMFATG_CTRL_ADD1_ON  1
#define  SIG_DTMFATG_CTRL_ADD1_OFF 0
#define  SIG_DTMFATG_CTRL_ADD2_ON  1
#define  SIG_DTMFATG_CTRL_ADD2_OFF 0
#define  SIG_DTMFATG_CTRL_ET  1
#define  SIG_DTMFATG_CTRL_EVM_READY  1
#define  SIG_DTMFATG_CTRL_EVM_BUF_REQ 2
#define  SIG_DTMFATG_CTRL_EVM_BUF_UNDERFLOW 4


/**
   This command activates the DTMF Receiver in the addressed channel. The selected
   signaling channel has to be activated before this command can be sent.If DTMF
   Receiver coefficients, which differ from the default values, are required, they have
   to be programmed with a separate command before the DTMF Receiver is activated.
*/
struct SIG_DTMFR_CTRL
{
   CMD_HEAD_BE;
   /** Enable */
   IFX_uint32_t EN : 1;
   /** Event Transmission Support */
   IFX_uint32_t ET : 1;
   /** Functionality of status bit */
   IFX_uint32_t FUNC : 1;
   /** Event generation DTMF_START */
   IFX_uint32_t ES : 1;
   /** Event generation DTMF_END */
   IFX_uint32_t EE : 1;
   /** Reserved */
   IFX_uint32_t Res01 : 4;
   /** Delay on/off */
   IFX_uint32_t DEL : 1;
   /** Input Signal */
   IFX_uint32_t IS : 1;
   /** Auto Suppression */
   IFX_uint32_t AS : 1;
   /** DTMF/AT Receiver Resource Number */
   IFX_uint32_t DTRNR : 4;
   /** Reserved */
   IFX_uint32_t Res02 : 11;
   /** Delay of the speech signal */
   IFX_uint32_t DELAY : 5;
} __PACKED__ ;

#define  SIG_DTMFR_CTRL_ECMD 4
#define  SIG_DTMFR_CTRL_LEN 4
#define  SIG_DTMFR_CTRL_ENABLE  0x1
#define  SIG_DTMFR_CTRL_DISABLE  0x0
#define  SIG_DTMFR_CTRL_ACTIVE  0x1
#define  SIG_DTMFR_CTRL_IS_SIGINB  0x1
#define  SIG_DTMFR_CTRL_IS_SIGINA  0x0
#define  SIG_DTMFR_CTRL_ON  0x1
#define  SIG_DTMFR_CTRL_ES_EN  1


/**
   This is a part of a command that activates the Call Progress Tone
   Detection (CPTD). This structure enables/disables a single CPTD Resource.
*/
typedef struct SIG_CPTD_CTRL_DATA
{
   /** Enable the CPT Detector */
   IFX_uint32_t EN : 1;
   /** Any Tone Detection */
   IFX_uint32_t AT : 1;
   /** Any Tone Status */
   IFX_uint32_t ATS : 2;
   /** Total Power */
   IFX_uint32_t TP : 1;
   /** Continuous Tone Detect */
   IFX_uint32_t CNT : 1;
   /** Frame Length */
   IFX_uint32_t FL : 2;
   /** Window Select */
   IFX_uint32_t WS : 1;
   /** Reserved */
   IFX_uint32_t Res01 : 1;
   /** Input Signal */
   IFX_uint32_t IS : 2;
   /** CPTD Resource Number */
   IFX_uint32_t CPTNR : 4;
   /** Reserved */
   IFX_uint32_t Res02 : 16;
} SIG_CPTD_CTRL_DATA_t;

/**
   This command activates the Call Progress Tone Detection (CPTD) in the selected
   signalling channel. The signaling channel (CHAN) has to be activated before this
   command can be sent. Before the CPT can be activated, the coefficients have to be
   programmed.
*/
struct SIG_CPTD_CTRL
{
   CMD_HEAD_BE;
   /** Enable/disable CPTD resource.
       Structure is repeated multiple times in this FW command.
       Each single element controls separate CPTD resource. */
   SIG_CPTD_CTRL_DATA_t CPTD[SIG_CPTD_CTRL_DATA_MAX];
} __PACKED__ ;

#define  SIG_CPTD_CTRL_ECMD 9
#define  SIG_CPTD_CTRL_LEN 4
#define  SIG_CPTD_CTRL_ENABLE  0x1
#define  SIG_CPTD_CTRL_ACTIVE  0x1
#define  SIG_CPTD_CTRL_ATS_DETECTED  2
#define  SIG_CPTD_CTRL_ATS_SEEN  3
#define  SIG_CPTD_CTRL_TP_250_3400  0x1
#define  SIG_CPTD_CTRL_CNT_CONTINUOUS  0x1
#define  SIG_CPTD_CTRL_FL_128_16MS  0
#define  SIG_CPTD_CTRL_FL_256_32MS  1
#define  SIG_CPTD_CTRL_FL_512_64MS  2
#define  SIG_CPTD_CTRL_WS_BLACKMAN  0x1
#define  SIG_CPTD_CTRL_IS_I1  0
#define  SIG_CPTD_CTRL_IS_I2  1
#define  SIG_CPTD_CTRL_IS_I1_I2  2


/**
   This command activates the universal tone generator in the addressed signaling
   channel.
   The signaling channel has to be activated before this command can be sent. Before
   the UTG can be activated, the coefficients have to be programmed.
*/
struct SIG_UTG_CTRL
{
   CMD_HEAD_BE;
   /** Enable */
   IFX_uint32_t EN : 1;
   /** Stop Mask */
   IFX_uint32_t SM : 1;
   /** Square Wave Select */
   IFX_uint32_t SQ : 1;
   /** Fade-In/Out Logarithmic Select */
   /* LOG is a macro so this is named _LOG */
   IFX_uint32_t _LOG : 1;
   /** Immediate Stop Select */
   IFX_uint32_t IMS : 1;
   /** EU enable end of tone event generation */
   IFX_uint32_t EU : 1;
   /** Reserved */
   IFX_uint32_t Res01 : 2;
   /** Add Signal to Signal Path 1 */
   IFX_uint32_t A1 : 2;
   /** Add Signal to Signal Path 2 */
   IFX_uint32_t A2 : 2;
   /** UTG Resource Number low-part */
   IFX_uint32_t UTGNRL : 4;
   /** Reserved */
   IFX_uint32_t Res02 : 12;
   /** UTG Resource Number high-part */
   IFX_uint32_t UTGNRH : 4;
} __PACKED__ ;

#define  SIG_UTG_CTRL_ECMD 10
#define  SIG_UTG2_CTRL_ECMD 6
#define  SIG_UTG_CTRL_LEN  4
#define  SIG_UTG_CTRL_DISABLE  0x0
#define  SIG_UTG_CTRL_ENABLE  0x1
#define  SIG_UTG_CTRL_SM_CONTINUE  0x1
#define  SIG_UTG_CTRL_SM_STOP      0x0
#define  SIG_UTG_CTRL_SQ_SQUARE  0x1
#define  SIG_UTG_CTRL_FADE_LOGARITHMIC  0x1
#define  SIG_UTG_CTRL_ON  1
#define  SIG_UTG_CTRL_A1_ON 1
#define  SIG_UTG_CTRL_A1_OFF 0
#define  SIG_UTG_CTRL_A2_ON 1
#define  SIG_UTG_CTRL_A2_OFF 0
#define  SIG_UTG_CTRL_EU_OFF 0
#define  SIG_UTG_CTRL_EU_ON 1


/**
   This command activates the modem and fax tone discriminator (MFTD).
*/
struct SIG_MFTD_CTRL
{
   CMD_HEAD_BE;
   /** Enable */
   IFX_uint32_t EN : 1;
   IFX_uint32_t ETA : 1;
   IFX_uint32_t ETD : 1;
   IFX_uint32_t ETC : 1;
   /** Enable MFTD status events */
   IFX_uint32_t EV : 1;
   /** Reserved */
   IFX_uint32_t Res02 : 2;
   IFX_uint32_t VMD : 1;
   IFX_uint32_t MH : 1;
   /** Reserved */
   IFX_uint32_t Res03 : 3;
   IFX_uint32_t MFTDNR : 4;
   /** Reserved */
   IFX_uint32_t Res04 : 16;
   /** Reserved */
   IFX_uint32_t Res05 : 1;
   IFX_uint32_t DUAL1 : 3;
   IFX_uint32_t ATD1 : 2;
   IFX_uint32_t DIS1 : 1;
   IFX_uint32_t Res06 : 1;
   IFX_uint32_t SINGLE1 : 8;
   /** Reserved */
   IFX_uint32_t Res07 : 1;
   IFX_uint32_t DUAL2 : 3;
   IFX_uint32_t ATD2 : 2;
   IFX_uint32_t DIS2 : 1;
   IFX_uint32_t Res08 : 1;
   IFX_uint32_t SINGLE2 : 8;
} __PACKED__ ;

#define  SIG_MFTD_CTRL_ECMD  5
#define  SIG_MFTD_CTRL_LEN   8
#define  SIG_MFTD_SINGLE_V21L  0x0001
#define  SIG_MFTD_SINGLE_V18A  0x0002
#define  SIG_MFTD_SINGLE_V27   0x0004
#define  SIG_MFTD_SINGLE_CNGMOD 0x0008
#define  SIG_MFTD_SINGLE_CNGFAX 0x0010
#define  SIG_MFTD_SINGLE_BELL  0x0020
#define  SIG_MFTD_SINGLE_V22   0x0040
#define  SIG_MFTD_SINGLE_V21H  0x0080
#define  SIG_MFTD_DUAL_V32AC   0x0001
#define  SIG_MFTD_DUAL_V8bis   0x0002
#define  SIG_MFTD_DUAL_CASBELL 0x0004
#define  SIG_MFTD_ATD_EN       0x0002
#define  SIG_MFTD_ATD_AM_EN    0x0003
#define  SIG_MFTD_VAD_EN       0x0001


/**
   This command activates one of the CID Receivers in the addressed channel.
   The selected signaling channel has to be activated before this command can be sent.
   Before the CID Receiver can be activated, the coefficients have to be programmed
   with a separate command.
*/
struct SIG_CIDR_CTRL
{
   CMD_HEAD_BE;
   /** Enable */
   IFX_uint32_t EN : 1;
   /** Reserved */
   IFX_uint32_t Res01 : 1;
   /** Drop Out within Seizure and Mark */
   IFX_uint32_t DO : 1;
   /** CID-Receiver Mode */
   IFX_uint32_t CM : 1;
   /** Reserved */
   IFX_uint32_t Res02 : 6;
   /** Input Signal */
   IFX_uint32_t IS : 1;
   /** Reserved */
   IFX_uint32_t Res03 : 1;
   /** CID Receiver Resource Number */
   IFX_uint32_t CIDRNR : 4;
   /** Reserved */
   IFX_uint32_t Res04 : 16;
} __PACKED__ ;

#define  SIG_CIDR_CTRL_ECMD 11
#define  SIG_CIDR_CTRL_LEN 4
#define  SIG_CIDR_CTRL_DISABLE  0x0
#define  SIG_CIDR_CTRL_ENABLE  0x1
#define  SIG_CIDR_CTRL_DO_DISABLE  0x1
#define  SIG_CIDR_CTRL_CM_TELCORDIA  0x1
#define  SIG_CIDR_CTRL_CM_V23 0x0
#define  SIG_CIDR_CTRL_IS_I2  0x1


/**
   This command allows the control CPU to configure parameters required for RTP event
   tranmsission as well as for event playout. The bitfields SSRC, EVT_PT, VB, EMT allow
   the configuration of the event transmission (generation of RTP event packets) while
   the bitfields EVTOG, A1 and A2 control the playout of received events. CC is
   relevant for event transmission as well as for event playout.
   This command has to be sent before the corresponding channel will be activated.
   Otherwise the default or last used configuration will be used until the command is
   received.
*/
struct SIG_RTP_SUP
{
   CMD_HEAD_BE;
   /** Synchronization Source Value for the Events */
   IFX_uint32_t SSRC;
   /** Reserved */
   IFX_uint32_t Res01 : 16;
   IFX_uint32_t EN_EPTD : 1;
   IFX_uint32_t EVT_PTDS : 7;
   IFX_uint32_t PR_MODE : 1;
   /** Event Payload Type */
   IFX_uint32_t EVT_PT : 7;
   /** Voice Block */
   IFX_uint32_t VB : 1;
   /** Reserved */
   IFX_uint32_t Res02 : 4;
   /** Event to be Played by the internal Tone Generator */
   IFX_uint32_t EVTOG : 3;
   /** Coder Channel */
   IFX_uint32_t CC : 4;
   /** Tone Generator Mode for Adder 1 */
   IFX_uint32_t A1 : 2;
   /** Tone Generator Mode for Adder 2 */
   IFX_uint32_t A2 : 2;
   /** Reserved */
   IFX_uint32_t Res03 : 9;
   /** Event Mask Trigger */
   IFX_uint32_t EMT : 7;
} __PACKED__ ;

#define  SIG_RTP_SUP_ECMD 16
#define  SIG_RTP_SUP_LEN 12
#define  SIG_RTP_SUP_MUTE_ON  2
#define  SIG_RTP_SUP_MUTE_OFF 1
#define  SIG_RTP_SUP_YES  0x1
#define  SIG_RTP_SUP_ANS  2
#define  SIG_RTP_SUP_CNG  4
#define  SIG_RTP_SUP_NOTONE  0
#define  SIG_RTP_SUP_TONE  1
#define  SIG_RTP_SUP_TONEMUTE  2
#define  SIG_RTP_SUP_DTMF  0x1


/**
   This command delivers some statistic information for the event play out and event
   transmit unit. A read or write access clears the status bit EPOU_STAT.
   A write command will reset the whole statistic
*/
struct SIG_RTP_EVT_STAT
{
   CMD_HEAD_BE;
   /** Sent Events */
   IFX_uint32_t SENT_EVENTS;
   /** Received Events */
   IFX_uint32_t RECEIVED_EVENTS;
   /** Discarded Events */
   IFX_uint16_t DISCARDED_EVENTS;
   /** Late Events */
   IFX_uint16_t LATE_EVENTS;
   /** Early Events */
   IFX_uint16_t EARLY_EVENTS;
   /** Re synchronization */
   IFX_uint16_t RESYNC;
   /** Received Bytes */
   IFX_uint32_t RECEIVED_BYTES;
} __PACKED__ ;

#define  SIG_RTP_EVT_STAT_ECMD 28


/**
   This command determines the coefficients for the PCM as well as for the analog line
   interface line echo canceller (LEC).
*/
struct RES_LEC_COEF
{
   CMD_HEAD_BE;
   /** Length */
   IFX_uint8_t LEN;
   /** Power Detection Level Receive */
   IFX_uint8_t POWR;
   /** Delta Power */
   IFX_uint8_t DELTA_P;
   /** Delta Quality */
   IFX_uint8_t DELTA_Q;
   /** Gain Transmit In */
   IFX_uint16_t GAIN_XI;
   /** Gain Transmit Out */
   IFX_uint16_t GAIN_XO;
   /** Gain Adjustment Receive Path */
   IFX_uint16_t GAIN_RI;
   /** Length of the fix Window */
   IFX_uint8_t LEN_FIX_WIN;
   /** Power Limit */
   IFX_uint8_t PMW_POWR;
   /** Delta Power */
   IFX_uint8_t PMW_DELTAP;
   /** Delta Quality */
   IFX_uint8_t PMW_DELTAQ;
   /** Reserved */
   IFX_uint32_t Res01 : 16;
} __PACKED__ ;

#define  RES_LEC_COEF_ECMD  0
#define  RES_LEC_COEF_LEN  16
#define  RES_LEC_COEF_YES  0x1


/**
   This command allows the configuration of the coefficients for the NLP of the Line
   echo canceller.
*/
struct RES_LEC_NLP_COEF
{
   CMD_HEAD_BE;
   /** Power estimation, Increment */
   IFX_uint8_t C_POW_INC;
   /** Power estimation, Decrement */
   IFX_uint8_t C_POW_DEC;
   /** Background Noise Estimation Level for X-In/X-Lout */
   IFX_uint8_t C_BN_LEV_X;
   /** Background Noise Estimation Level for R-In */
   IFX_uint8_t C_BN_LEV_R;
   /** Background Noise Estimation Increment */
   IFX_uint8_t C_BN_INC;
   /** Background Noise Estimation Decrement */
   IFX_uint8_t C_BN_DEC;
   /** Background Noise Estimation Maximum Noise Value */
   IFX_uint8_t C_BN_MAX;
   /** Background Noise Estimation Adjustment */
   IFX_uint8_t C_BN_ADJ;
   /** Residual Echo Minimum ERL LEC+Line */
   IFX_uint8_t C_RE_MIN_ERLL;
   /** Residual Echo Estimated ERL LEC+Line */
   IFX_uint8_t C_RE_EST_ERLL;
   /** Speech Detection Level for X-Lout */
   IFX_uint8_t C_SD_LEV_X;
   /** Speech Detection Level for R-In */
   IFX_uint8_t C_SD_LEV_R;
   /** Speech Detection Level for BN */
   IFX_uint8_t C_SD_LEV_BN;
   /** Speech Detection Level for Residual Echo */
   IFX_uint8_t C_SD_LEV_RE;
   /** Speech Detection Overhang Time Double Talk */
   IFX_uint8_t C_SD_OT_DT;
   /** Echo Return Loss (ERL) Estimation, Fast Time Constant */
   IFX_uint8_t C_ERL_LPF;
   /** Echo Return Loss (ERL) Estimation, Slow Time Constant */
   IFX_uint8_t C_ERL_LPS;
   /** NLP Control Level for Residual Echo */
   IFX_uint8_t C_CT_LEV_RE;
   /** Reserved */
   IFX_uint32_t Res01 : 16;
} __PACKED__ ;

#define  RES_LEC_NLP_COEF_ECMD   3
#define  RES_LEC_NLP_COEF_LEN  20
#define  RES_LEC_NLP_COEF_YES  0x1


/**
   This command allow the control CPU to set the coefficients for the CID Receiver.
*/
struct RES_CIDR_COEF
{
   CMD_HEAD_BE;
   /** Reserved */
   IFX_uint32_t Res01 : 1;
   /** Minimum CID level */
   IFX_uint32_t LEVEL : 15;
   /** Reserved */
   IFX_uint32_t Res02 : 1;
   /** Number of Seizure Bits */
   IFX_uint32_t SEIZURE : 15;
   /** Reserved */
   IFX_uint32_t Res03 : 1;
   /** Number of Mark */
   IFX_uint32_t MARK : 15;
   /** Message Format 0 */
   IFX_uint8_t MESSAGE_FORMAT_0;
   /** Message Format 1 */
   IFX_uint8_t MESSAGE_FORMAT_1;
   /** Message Format 2 */
   IFX_uint8_t MESSAGE_FORMAT_2;
   /** Message Format 3 */
   IFX_uint8_t MESSAGE_FORMAT_3;
   /** Message Format 4 */
   IFX_uint8_t MESSAGE_FORMAT_4;
   /** Message Format 5 */
   IFX_uint8_t MESSAGE_FORMAT_5;
} __PACKED__ ;

#define  RES_CIDR_COEF_ECMD 7
#define  RES_CIDR_COEF_LEN  12
#define  RES_CIDR_COEF_YES  0x1
#define  RES_CIDR_COEF_DEFAULT_LEVEL   0x00b0
#define  RES_CIDR_COEF_DEFAULT_SEIZURE 0x003c
#define  RES_CIDR_COEF_DEFAULT_MARK    0x0032


/**
   This command allows the control CPU to configure the coefficients of the
   Caller ID sender.
*/
struct RES_CIDS_COEF
{
   CMD_HEAD_BE;
   /** Reserved */
   IFX_uint32_t Res01 : 1;
   /** CID Transmit Level */
   IFX_uint32_t LEVEL : 15;
   /** Reserved */
   IFX_uint32_t Res02 : 1;
   /** Number of Seizure Bits */
   IFX_uint32_t SEIZURE : 15;
   /** Reserved */
   IFX_uint32_t Res03 : 1;
   /** Number of Mark Bits */
   IFX_uint32_t MARK : 15;
   /** Reserved */
   IFX_uint32_t Res04 : 4;
   /** Number of Additional Stop Bits */
   IFX_uint32_t STOP : 4;
   /** Buffer Request Size in Byte */
   IFX_uint8_t BRS;
} __PACKED__ ;

#define  RES_CIDS_COEF_ECMD  8
#define  RES_CIDS_COEF_LEN  8
#define  RES_CIDS_COEF_YES   0x1


/**
   This command sends new data to the CID Sender.
   Each data word contains 2 data bytes for the high level mode or 16bits for the
   low-level mode. It is only allowed to send new data when the status bit CIS_REQ has
   been set. Otherwise the command is discarded and the command error bit, CERR, will
   be set. The bits CIS_REQ and CIS_BUF have to be reset by the control CPU.If the
   request bit CIS_REQ has been set to 1, the host must send at least BRS number of
   bytes. Only if there are no additional bytes to send, it is allowed to send less
   than BRS number of bytes. In the this case the host may not get a new request.
*/
struct RES_CIDS_DATA
{
   CMD_HEAD_BE;
   /** 16-bit Data Word or high byte and low byte */
   IFX_uint16_t DATA[10];
} __PACKED__ ;

#define  RES_CIDS_DATA_ECMD 9
#define  RES_CIDS_DATA_LEN 20


/**
   This command allows configuration of the DTMF/AT generator coefficients.
   If the event support is active (ET=1), the coefficients TIMT and TIMP are not needed
   by the generator and therefore TIMT and TIMP are don't care. The coefficients
   LEVEL1 and LEVEL2 are used in any mode and determine the twist. The coefficients
   ATT_AA and ATT_AB are added to the signal level which is received by the events.
   Thus the total attenuation is equal for both output signal, and results from the
   addition of the signal level and the corresponding coefficient.
*/
struct RES_DTMFATG_COEF
{
   CMD_HEAD_BE;
   /** Level for Frequency 1 (Lower Frequency) */
   IFX_uint8_t LEVEL1;
   /** Level for Frequency 2 (Higher Frequency) */
   IFX_uint8_t LEVEL2;
   /** Time for DTMF Tone */
   IFX_uint8_t TIMT;
   /** Time Between 2 DTMF Tones and after the last Tone */
   IFX_uint8_t TIMP;
   /** Output Attenuation for Adder 1 */
   IFX_uint8_t ATT_A1;
   /** Output Attenuation for Adder 2 */
   IFX_uint8_t ATT_A2;
   /** Reserved */
   IFX_uint32_t Res01 : 16;
} __PACKED__ ;

#define  RES_DTMFATG_COEF_ECMD 10
#define  RES_DTMFATG_COEF_LEN 8
#define  RES_DTMFATG_COEF_YES  0x1


/**
   This command sends new data to the DTMF/AT Generator.
   In the low-level mode (MD = 0), the host can send only one frequency pair per
   command. Additional frequency pairs will be discarded (see command DTMF and
   Answering Tone Generator Control on Page113).In the high-level mode (MD = 1), the
   host can send more than one frequency pair. Up to 5 in low-level frequency mode (FG
   = 0) and up to 10 in high-level frequency mode (FG = 1).In the low-level mode (FG =
   0) for the frequency generation, the host has to send two frequency words for each
   frequency pair. In high level mode (FG = 1), each word contains a short coding for a
   predefined frequency pairs.
   This command must not be used if the event transmission support is activated. In
   this case the host has to send the events as event packets via the packet in-box.The
   host must not send this command when the status bits DTMFG_REQ and DTMFG_BUF in the
   status register are zero. In this case the command will be discarded.In case of that
   the request bit has been set and the host wants to send additional DTFM digits, the
   host has to deliver at least 1 DTMF digit within 80 ms.
   Coding FREQxx
   If one of the frequencies (FREQ1, FREQ2) is FFFFH, the generator sets the
   corresponding signal to 0. In this case, the generator sends only one frequency.
   This feature is available in the low-level mode (MD=0) as well as in the high level
   mode (MD=1).
   If both frequencies are FFFFH or DTC is 11 1111B in the low-level mode (MD=0), the
   generator sends a zero signal (Pause). In the high level mode (MD=1) the generator
   inserts an additional pause with the length TIMP.
*/
struct RES_DTMFATG_DATA
{
   CMD_HEAD_BE;
   /** Frequency 1 1st Tone or Dual Tone Control Word 1 */
   IFX_uint16_t FREQ11_DTC1;
   /** Frequency 2 1st Tone or Dual Tone Control Word 2 */
   IFX_uint16_t FREQ21_DTC2;
   /** Frequency 1 2nd Tone */
   IFX_uint16_t FREQ12_DTC3;
   /** Frequency 2 2nd Tone */
   IFX_uint16_t FREQ22_DTC4;
   /** Frequency 1 3rd Tone */
   IFX_uint16_t FREQ13_DTC5;
   /** Frequency 2 3nd Tone */
   IFX_uint16_t FREQ23_DTC6;
   /** Frequency 1 4th Tone */
   IFX_uint16_t FREQ14_DTC7;
   /** Frequency 2 4th Tone */
   IFX_uint16_t FREQ24_DTC8;
   /** Frequency 1 5th Tone */
   IFX_uint16_t FREQ15_DTC9;
   /** Frequency 2 5th Tone */
   IFX_uint16_t FREQ25_DTC10;
} __PACKED__ ;

#define  SIG_DTMFATG_DATA_ECMD  11


/**
   This command determines the coefficients for the DTMF Receiver.
*/
struct RES_DTMFR_COEF
{
   CMD_HEAD_BE;
   /** Minimum Signal Level */
   IFX_uint32_t LEVEL : 8;
   /** Maximal Allowed Signal Twist */
   IFX_uint32_t TWIST : 8;
   /** Gain Adjustment of the DTMF Input Signal */
   IFX_uint32_t GAIN : 16;
   /** Minimum Burst Duration Time */
   IFX_uint32_t TCMIN : 8;
   /** Maximum Burst Interruption Time */
   IFX_uint32_t TBMAX : 8;
   /** Minimum Pause Duration Time */
   IFX_uint32_t TPMIN : 8;
   /** Fine Adjustment of the Minimum Signal Level */
   IFX_uint32_t FINELEVEL : 8;
} __PACKED__ ;

#define  RES_DTMFR_COEF_ECMD  12
#define  RES_DTMFR_COEF_LEN  4


/**
   This command allows the control CPU to configure the coefficients for the automatic
   gain control (AGC).
*/
struct RES_AGC_COEF
{
   CMD_HEAD_BE;
   /** Target Level of the AGC Output */
   IFX_uint32_t COM : 8;
   /** Initial Gain */
   IFX_uint32_t INIGAIN : 8;
   /** Adaptation Speed of the Gain towards lower Values */
   IFX_uint32_t SEEDL : 8;
   /** Adaptation Speed of the Gain towards higher Values */
   IFX_uint32_t SPPEDH : 8;
   /** Maximum Gain of the AGC */
   IFX_uint32_t GAIN : 8;
   /** Minimum Gain of the AGC */
   IFX_uint32_t ATT : 8;
   /** Time Constant of the Peak Detector */
   IFX_uint32_t DEC : 8;
   /** Minimum Signal Level for the Adaptation of the AGC */
   IFX_uint32_t LIM : 8;
   /** Reserved */
   IFX_uint32_t Res01 : 8;
   /** One */
   IFX_uint32_t ONE : 1;
   /** Low Pass Time Constant */
   IFX_uint32_t LP : 7;
   /** Reserved */
   IFX_uint32_t Res02 : 16;
} __PACKED__ ;

#define  RES_AGC_COEF_ECMD 15
#define  RES_AGC_COEF_LEN 12
#define  RES_AGC_COEF_YES  0x1


/**
   This command allows the control CPU to set the coefficients for the Call Progress
   Tone Detector.
   T_1, T_2, T_3 and T_4 are internally rounded up to multiples of frame length/2, as
   the timing and frequency analysis is only executed once per frame length/2 samples.
   In general the actual start and end of bursts and pauses will not be synchronized to
   the execution of the timing and frequency analysis of the CPT.

*/
struct RES_CPTD_COEF
{
   CMD_HEAD_BE;
   /** Coefficient for the Goertzel Algorithm 1 */
   IFX_uint16_t GOE_1;
   /** Coefficient for the Goertzel Algorithm 2 */
   IFX_uint16_t GOE_2;
   /** Coefficient for the Goertzel Algorithm 3 */
   IFX_uint16_t GOE_3;
   /** Coefficient for the Goertzel Algorithm 4 */
   IFX_uint16_t GOE_4;
   /** Level for Corresponding Frequency 1 */
   IFX_uint16_t LEV_1;
   /** Level for Corresponding Frequency 2 */
   IFX_uint16_t LEV_2;
   /** Level for Corresponding Frequency 3 */
   IFX_uint16_t LEV_3;
   /** Level for Corresponding Frequency 4 */
   IFX_uint16_t LEV_4;
   /** TWIST 12 */
   IFX_uint8_t TWIST_12;
   /** TWIST 34 */
   IFX_uint8_t TWIST_34;
   /** Time */
   IFX_uint16_t T_1;
#if 0 /* This actually is MSK_1 - seems to be extended in Spec for some reason */
   /** Reserved */
   IFX_uint32_t Res01 : 1;
   /** Pause Detection */
   IFX_uint32_t P : 1;
   /** ENABLE */
   IFX_uint32_t E : 1;
   /** Frequencies 1 and 2 or Frequencies 3 and 4 */
   IFX_uint32_t F12or34 : 1;
   /** Frequency 3 or Frequency 4 */
   IFX_uint32_t F3xor4 : 1;
   /** Frequency 1or Frequency 2 */
   IFX_uint32_t F1xor2 : 1;
   /** Twist 34 */
   IFX_uint32_t TW34 : 1;
   /** Twist 12 */
   IFX_uint32_t TW12 : 1;
   /** Frequency 4 */
   IFX_uint32_t F4 : 2;
   /** Frequency 3 */
   IFX_uint32_t F3 : 2;
   /** Frequency 2 */
   IFX_uint32_t F2 : 2;
   /** Frequency 1 */
   IFX_uint32_t F1 : 2;
#else
   /** Mask Coefficients for Timing 1 */
   IFX_uint16_t MSK_1;
#endif
   /** Time */
   IFX_uint16_t T_2;
   /** Mask Coefficients for Timing 2 */
   IFX_uint16_t MSK_2;
   /** Time */
   IFX_uint16_t T_3;
   /** Mask Coefficients for Timing 3 */
   IFX_uint16_t MSK_3;
   /** Time */
   IFX_uint16_t T_4;
   /** Mask Coefficients for Timing 4 */
   IFX_uint16_t MSK_4;
   /** Timing Tolerance */
   IFX_uint8_t TIM_TOL;
   /** Number of Successive Fulfilled Timing Requirements */
   IFX_uint8_t NR;
   /** Maximum Allowed Total Power Within Requested Pause */
   IFX_uint16_t POW_PAUSE;
   /** Maximum Allowed Frequency to Total Power */
   IFX_uint16_t FP_TP_R;
   /** Any Tone Detection, Total Power */
   IFX_uint16_t AT_POWER;
   /** Any Tone Detection, Duration */
   IFX_uint8_t AT_DUR;
   /** Any Tone Detection, Gap Time */
   IFX_uint8_t AT_GAP;
} __PACKED__ ;

#define  RES_CPTD_COEF_ECMD 16
#define  RES_CPTD_COEF_LEN 44
#define  RES_CPTD_COEF_YES  0x1
#define  RES_CPTD_COEF_ON  0x1
#define  RES_CPTD_COEF_CV_E_T_1_MSK_1  0x1
#define  RES_CPTD_COEF_OR  0x1
#define  RES_CPTD_COEF_BELOW_34  0x1
#define  RES_CPTD_COEF_BELOW  0x1
#define  RES_CPTD_COEF_F_GREATER  1
#define  RES_CPTD_COEF_F_LOWER  2
#define  RES_CPTD_COEF_F1 0x0001
#define  RES_CPTD_COEF_F2 0x0004
#define  RES_CPTD_COEF_F3 0x0010
#define  RES_CPTD_COEF_F4 0x0040
#define  RES_CPTD_COEF_E  (1 << 13)
#define  RES_CPTD_COEF_P  (1 << 14)
#define  RES_CPTD_COEF_FX_0HZ 0x8000
#define  RES_CPTD_COEF_LEVEL_0DB 0x2C7B


/**
   This command allow the control CPU to set the coefficients for the universal tone
   generator (UTG).
*/
struct RES_UTG_COEF
{
   CMD_HEAD_BE;
   /** Fade In Attenuation */
   IFX_uint32_t FD_IN_ATT : 16;
   /** Fade In Step */
   IFX_uint32_t FD_IN_SP : 16;
   /** Fade In Time */
   IFX_uint32_t FD_IN_TIM : 16;
   /** Fade Out Step */
   IFX_uint32_t FD_OT_SP : 16;
   /** Fade Out Time */
   IFX_uint32_t FD_OT_TIM : 8;
   /** Modulation Factor */
   IFX_uint32_t MOD_12 : 8;
   /** Frequency 1 */
   IFX_uint32_t F1 : 16;
   /** Frequency 2 */
   IFX_uint32_t F2 : 16;
   /** Frequency 3 */
   IFX_uint32_t F3 : 16;
   /** Frequency 4 */
   IFX_uint32_t F4 : 16;
   /** Level 1 for the Corresponding Frequency F1 */
   IFX_uint32_t LEV_1 : 8;
   /** Level 2 for the Corresponding Frequency F2 */
   IFX_uint32_t LEV_2 : 8;
   /** Level 3 for the Corresponding Frequency F3 */
   IFX_uint32_t LEV_3 : 8;
   /** Level 4 for the Corresponding Frequency F4 */
   IFX_uint32_t LEV_4 : 8;
   /** Time Step Generation (Instruction 12,14,16,18,20,22,n 1..6) */
   IFX_uint32_t T_1 : 16;
   /** Next Mask */
   IFX_uint32_t MSK1_NXT : 4;
   /** Repetition Counter */
   IFX_uint32_t MSK1_REP : 3;
   /** Fade In Enable */
   IFX_uint32_t MSK1_FI : 1;
   /** Frequency 1 */
   IFX_uint32_t MSK1_F1_ON : 1;
   /** Frequency 2 */
   IFX_uint32_t MSK1_F2_ON : 1;
   /** Frequency 3 */
   IFX_uint32_t MSK1_F3_ON : 1;
   /** Frequency 4 */
   IFX_uint32_t MSK1_F4_ON : 1;
   /** Modulation of Frequency 1 */
   IFX_uint32_t MSK1_M12 : 1;
   /** Fade Out Enable */
   IFX_uint32_t MSK1_FO : 1;
   /** Stop Allowed */
   IFX_uint32_t MSK1_SA : 2;
   /** Time Step 2 Generation Description see T_1 */
   IFX_uint32_t T_2 : 16;
   /** Mask Coefficients for the Tone Generation Steps: */
   /** Next Mask */
   IFX_uint32_t MSK2_NXT : 4;
   /** Repetition Counter */
   IFX_uint32_t MSK2_REP : 3;
   /** Fade In Enable */
   IFX_uint32_t MSK2_FI : 1;
   /** Frequency 1 */
   IFX_uint32_t MSK2_F1_ON : 1;
   /** Frequency 2 */
   IFX_uint32_t MSK2_F2_ON : 1;
   /** Frequency 3 */
   IFX_uint32_t MSK2_F3_ON : 1;
   /** Frequency 4 */
   IFX_uint32_t MSK2_F4_ON : 1;
   /** Modulation of Frequency 1 */
   IFX_uint32_t MSK2_M12 : 1;
   /** Fade Out Enable */
   IFX_uint32_t MSK2_FO : 1;
   /** Stop Allowed */
   IFX_uint32_t MSK2_SA : 2;

   /** Time Step 3 Generation Description see T_1 */
   IFX_uint32_t T_3 : 16;
   /** Mask Coefficients for the Tone Generation Steps */
   /** Next Mask */
   IFX_uint32_t MSK3_NXT : 4;
   /** Repetition Counter */
   IFX_uint32_t MSK3_REP : 3;
   /** Fade In Enable */
   IFX_uint32_t MSK3_FI : 1;
   /** Frequency 1 */
   IFX_uint32_t MSK3_F1_ON : 1;
   /** Frequency 2 */
   IFX_uint32_t MSK3_F2_ON : 1;
   /** Frequency 3 */
   IFX_uint32_t MSK3_F3_ON : 1;
   /** Frequency 4 */
   IFX_uint32_t MSK3_F4_ON : 1;
   /** Modulation of Frequency 1 */
   IFX_uint32_t MSK3_M12 : 1;
   /** Fade Out Enable */
   IFX_uint32_t MSK3_FO : 1;
   /** Stop Allowed */
   IFX_uint32_t MSK3_SA : 2;

   /** Time Step 4 Generation Description see T_1 */
   IFX_uint32_t T_4 : 16;
   /** Mask Coefficients for the Tone Generation Steps */
   /** Next Mask */
   IFX_uint32_t MSK4_NXT : 4;
   /** Repetition Counter */
   IFX_uint32_t MSK4_REP : 3;
   /** Fade In Enable */
   IFX_uint32_t MSK4_FI : 1;
   /** Frequency 1 */
   IFX_uint32_t MSK4_F1_ON : 1;
   /** Frequency 2 */
   IFX_uint32_t MSK4_F2_ON : 1;
   /** Frequency 3 */
   IFX_uint32_t MSK4_F3_ON : 1;
   /** Frequency 4 */
   IFX_uint32_t MSK4_F4_ON : 1;
   /** Modulation of Frequency 1 */
   IFX_uint32_t MSK4_M12 : 1;
   /** Fade Out Enable */
   IFX_uint32_t MSK4_FO : 1;
   /** Stop Allowed */
   IFX_uint32_t MSK4_SA : 2;

   /** Time Step 5 Generation Description see T_1 */
   IFX_uint32_t T_5 : 16;
   /** Mask Coefficients for the Tone Generation Steps: */
   /** Next Mask */
   IFX_uint32_t MSK5_NXT : 4;
   /** Repetition Counter */
   IFX_uint32_t MSK5_REP : 3;
   /** Fade In Enable */
   IFX_uint32_t MSK5_FI : 1;
   /** Frequency 1 */
   IFX_uint32_t MSK5_F1_ON : 1;
   /** Frequency 2 */
   IFX_uint32_t MSK5_F2_ON : 1;
   /** Frequency 3 */
   IFX_uint32_t MSK5_F3_ON : 1;
   /** Frequency 4 */
   IFX_uint32_t MSK5_F4_ON : 1;
   /** Modulation of Frequency 1 */
   IFX_uint32_t MSK5_M12 : 1;
   /** Fade Out Enable */
   IFX_uint32_t MSK5_FO : 1;
   /** Stop Allowed */
   IFX_uint32_t MSK5_SA : 2;

   /** Time Step 6 Generation Description see T_1 */
   IFX_uint32_t T_6 : 16;
   /** Mask Coefficients for the Tone Generation Steps */
   /** Next Mask */
   IFX_uint32_t MSK6_NXT : 4;
   /** Repetition Counter */
   IFX_uint32_t MSK6_REP : 3;
   /** Fade In Enable */
   IFX_uint32_t MSK6_FI : 1;
   /** Frequency 1 */
   IFX_uint32_t MSK6_F1_ON : 1;
   /** Frequency 2 */
   IFX_uint32_t MSK6_F2_ON : 1;
   /** Frequency 3 */
   IFX_uint32_t MSK6_F3_ON : 1;
   /** Frequency 4 */
   IFX_uint32_t MSK6_F4_ON : 1;
   /** Modulation of Frequency 1 */
   IFX_uint32_t MSK6_M12 : 1;
   /** Fade Out Enable */
   IFX_uint32_t MSK6_FO : 1;
   /** Stop Allowed */
   IFX_uint32_t MSK6_SA : 2;

   /** Output Gain */
   IFX_uint32_t GO : 16;
} __PACKED__ ;

#define  RES_UTG_COEF_ECMD  17
#define  RES_UTG_COEF_LEN  48
#define  RES_UTG_COEF_YES  0x1
#define  RES_UTG_COEF_NXT_MSB 0x8
#define  RES_UTG_COEF_NXT 0x7
#define  RES_UTG_COEF_FI_ON  0x1
#define  RES_UTG_COEF_F1_ON  0x1
#define  RES_UTG_COEF_F2_ON  0x1
#define  RES_UTG_COEF_F3_ON  0x1
#define  RES_UTG_COEF_F4_ON  0x1
#define  RES_UTG_COEF_M12_ON  0x1
#define  RES_UTG_COEF_FO_ON  0x1
#define  RES_UTG_COEF_SA_NO  0x00
#define  RES_UTG_COEF_SA_DELAYED_REP  0x01
#define  RES_UTG_COEF_SA_DELAYED_EXECUTED  0x02
#define  RES_UTG_COEF_SA_IMMEDIATE  0x03


/**
   This command allows setting of the line echo canceller echo suppressor
   coefficients.
*/
struct RES_ES_COEF
{
   CMD_HEAD_BE;
   /** Time constant for low pass filter 1 */
   IFX_uint32_t LP1R : 8;
   /** Offset for discrimator */
   IFX_uint32_t OFFR : 8;
   /** Limit for low pass filter output 2 */
   IFX_uint32_t LP2LR : 8;
   /** Limit for LOG output */
   IFX_uint32_t LIMR : 8;
   /** Time constant for peak detector speech case */
   IFX_uint32_t PDSR : 8;
   /** Time constant for low pass filter 2 speech case */
   IFX_uint32_t LP2SR : 8;
   /** Time constant for peak detector noise case */
   IFX_uint32_t PDNR : 8;
   /** Time constant for low pass filter 2 noise case */
   IFX_uint32_t LP2NR : 8;
   /** Time for switching on attenuation */
   IFX_uint32_t TAT : 8;
   /** Time for switching on attenuation */
   IFX_uint32_t T0 : 8;
   /** Attenuation echo suppressor */
   IFX_uint32_t ATT_ES : 8;
   /** Echo time */
   IFX_uint32_t ECHOT : 8;
   /** Time constant for low pass (acoustic side) double talk detection */
   IFX_uint32_t LPS : 8;
   /** Limit for disabling echo suppressor in case of double-talk (low pass filtered acoustic side signal > LIM_DS */
   IFX_uint32_t LIM_DS : 8;
   /* Maximum Attenuation */
   IFX_uint32_t MAX_ATT_ES : 16;
   /* ERL Threshold */
   IFX_uint32_t ERL_THRESH  : 16;
   /* Background Noise Estimation Adjustment */
   IFX_uint32_t BN_ADJ : 16;
   /* Background Noise Estimation Increment */
   IFX_uint32_t BN_INC : 16;
   /* Background Noise Estimation Decrement */
   IFX_uint32_t BN_DEC : 16;
   /* Background Noise Estimation Maximum Noise Value */
   IFX_uint32_t BN_MAX : 16;
   /* Background Noise Estimation Level for X-In/X-Lout */
   IFX_uint32_t BN_LEV_X : 16;
   /* Background Noise Estimation Level for R-In */
   IFX_uint32_t BN_LEV_R : 16;
   /** Reserved */
   IFX_uint32_t Res01 : 16;
} __PACKED__ ;

#define  RES_ES_COEF_ECMD     29
#define  RES_ES_COEF_LEN      32


/**
   This command has to be send by the control CPU in order to acknowledge a
   command error condition.
*/
struct SYS_CERR_ACK
{
   CMD_HEAD_BE;
} __PACKED__ ;

#define  SYS_CERR_ACK_ECMD 0


/**
   If the status bit MIPS_OL is set, the control CPU has to acknowledge the
   error condition with this command, otherwise the status bit will be set
   again from the voice CPU. The control CPU also has to clear the status bit
   in the status register.
*/
struct SYS_OLOAD_ACK
{
   CMD_HEAD_BE;
   /** Reserved */
   IFX_uint32_t Res01 : 15;
   /** EDSP MIPS Overload */
   IFX_uint32_t MIPS_OL : 1;
   /** Reserved */
   IFX_uint32_t Res02 : 16;
} __PACKED__ ;

#define  SYS_OLOAD_ACK_ECMD 2
#define  SYS_OLOAD_ACK_LEN 4
#define  SYS_OLOAD_MIPS_OL_ACK  0x1


/**
   This command reads out the reason and the corresponding header of a
   wrong cmd in case of a cmd error.
   Before this command can be used, a SYS_CERR_ACK is required.
*/
struct SYS_CERR_GET
{
   CMD_HEAD_BE;
   /** Cmd Error Cause */
   IFX_uint32_t cause;
   /** Command */
   IFX_uint32_t cmd;
} __PACKED__ ;

#define  SYS_CERR_GET_ECMD 4


/**
   This command allows the control CPU to read out the version and the variant
   of the firmware. The firmware version is formatted as "major.minor" and
   gives information about the development state of the firmware. The variant
   characterizes a specific feature set.
*/
struct SYS_VER
{
   CMD_HEAD_BE;
   /** Major */
   IFX_uint32_t MAJ : 8;
   /** Minor */
   IFX_uint32_t MIN : 8;
   /** Hotfix */
   IFX_uint32_t HF : 4;
   /** Platform */
   IFX_uint32_t PLA : 4;
   /** Variant */
   IFX_uint32_t VAR : 8;
   /**  Date of the firmware build in Linux time format */
   IFX_uint32_t LXDATE;
} __PACKED__ ;

#define  SYS_VER_ECMD 6
#define  SYS_VER_LEN_BASIC 4
#define  SYS_VER_LEN_WITH_TIME 8


/**
   This command reads out the capabilities of the firmware variant. The command
   provides information about the features,  which  are  implemented.  It does
   not  give  information  about  the  resource  requirements  regarding
   processing power (MIPs), which are consumed by each feature.
   Future extensions to this command can either be made by using the reserved
   bits in the command or by appending new words at the end of the command. It
   is important, to keep the command structure compatible to older versions, so
   higher layer software components can be made interoperable with different
   capability command versions.
   The version and the length of the capability message can be read out by
   reading the first 32-bit word of the command.
*/
struct SYS_CAP
{
   CMD_HEAD_BE;
   /** Version */
   IFX_uint32_t VERS : 8;
   /** Length of the Capability Command */
   IFX_uint32_t BLEN : 8;
   /** Number of PCM Channels */
   IFX_uint32_t NPCM : 8;
   /** Number of Analog Line Channels */
   IFX_uint32_t NALI : 8;
   /** Number of Signaling Channels */
   IFX_uint32_t NSIG : 8;
   /** Number of Coder Channels */
   IFX_uint32_t NCOD : 8;
   /** Number of AGCs */
   IFX_uint32_t NAGC : 8;
   /** Number of Equalizers */
   IFX_uint32_t NEQ : 8;
   /** Number of Near-End LECs */
   IFX_uint32_t NNLEC : 8;
   /** Number of Combined Near-End/Far-End LECs */
   IFX_uint32_t NWLEC : 8;
   /** Number of Near-End Wideband LECs */
   IFX_uint32_t NNWLEC : 8;
   /** Number of Combined Near-End/Far-End Wideband LECs */
   IFX_uint32_t NWWLEC : 8;
   /** Number of Universal Tone Generators */
   IFX_uint32_t NUTG : 8;
   /** Number of DTMF Generators */
   IFX_uint32_t NDTMFG : 8;
   /** Number of Caller ID Senders */
   IFX_uint32_t NCIDS : 8;
   /** Number of Caller ID Receivers */
   IFX_uint32_t NCIDR : 8;
   /** Number of Number of Call Progress Tone Detectors */
   IFX_uint32_t NCPTD : 8;
   /** Number of Number of Modem and Fax Tone Discriminators (MFTDs) */
   IFX_uint32_t NMFTD : 8;
   /** Number of Number of FAX Channels with FAX Relay (T.38) Support */
   IFX_uint32_t NFAX : 8;
   /** Number of DTMF Detectors */
   IFX_uint32_t NDTMFD : 8;
   /** Jitter buffer enhancements */
   IFX_uint32_t JB1 : 1;
   /** DTMF receiver enhancements */
   IFX_uint32_t DT1 : 1;
   /** T.38 stack is implemented in firmware */
   IFX_uint32_t T38FW : 1;
   /** DTMF receiver enhancements, step 1 */
   IFX_uint32_t DT2 : 1;
   /** RFC 4040 clearmode is supported */
   IFX_uint32_t RFC4040 : 1;
   /** Enhanced Echo Suppressor */
   IFX_uint32_t ESE : 1;
   /** FXO functionality with the XWAY-SLIC and external DAA */
   IFX_uint32_t FXODAA : 1;
   /** Number of PCM codec resources */
   IFX_uint32_t PCMCOD : 5;
   /** Number of "PCM shortcuts" */
   IFX_uint32_t PCMS : 2;
   /** Number of HDLC framers for D-channel access */
   IFX_uint32_t DCHAN : 2;
   /** Codecs */
   IFX_uint32_t CODECS : 16;
   /** Maximum Number of Low Complexity Coders for the Coder Channel */
   IFX_uint32_t CLOW : 8;
   /** Maximum Number of Mid Complexity Coders for the Coder Channel */
   IFX_uint32_t CMID : 8;
   /** Maximum Number of High Complexity Coders for the Coder Channel*/
   IFX_uint32_t CMAX : 8;
   /** PCM Channel Coders */
   IFX_uint32_t PCOD : 8;
   /** MFTD Version */
   IFX_uint32_t MFTDV : 4;
   /** Number of DECT Channels */
   IFX_uint32_t NDECT : 4;
   /** DECT Codecs */
   IFX_uint32_t DECT_CODECS : 4;
   /** Echo Suppressor in analog line channel */
   IFX_uint32_t ES : 1;
   /** Reserved */
   IFX_uint32_t PDD : 1;
   /** Redundancy (RFC 2198) */
   IFX_uint32_t RED : 1;
   /** Three CPTDs per signaling channel are supported */
   IFX_uint32_t CPTD3 : 1;
   /** Tone Detection Capabilities */
   IFX_uint32_t TONES : 16;
   /** Features */
   IFX_uint32_t FEAT : 8;
   /** Overlays */
   IFX_uint32_t OV : 8;
   /** Event Playout Capabilities */
   IFX_uint32_t EPC : 8;
   /** Event Transmission Capabilities */
   IFX_uint32_t ETC : 8;
   /** Number of linear test channels */
   IFX_uint32_t NLIN : 8;
   /** Announcement playback supported */
   IFX_uint32_t Ann : 1;
   /** Support of the DECT channel Echo Suppressor */
   IFX_uint32_t DES : 1;
   /** Support of the idle pattern in the D-Channel */
   IFX_uint32_t DIP : 1;
   /** LEC version */
   IFX_uint32_t LECV : 3;
   /** Reserved */
   IFX_uint32_t FXOS : 1;
   /** Jitter adaptation during silence */
   IFX_uint32_t JAS : 1;
   /** Reserved */
   IFX_uint32_t NBF : 1;
   /** Reserved */
   IFX_uint32_t FT : 1;
   /** Echo canceler additions 1:
       EC supports parameter freeze and automatic parameter freeze */
   IFX_uint32_t ECA1 : 1;
   /** Echo canceler additions 2 */
   IFX_uint32_t ECA2 : 1;
   /** Echo canceler additions 3 */
   IFX_uint32_t ECA3 : 1;
   /** EMOS calculation */
   IFX_uint32_t EMOS : 1;
   /** Jitter Buffer "Smart Adaptation" feature is supported */
   IFX_uint32_t JBSA : 1;
   /** Message Waiting Lamp supported by SDD */
   IFX_uint32_t MWI : 1;
   /** Number of echo suppressor ressources */
   IFX_uint32_t NES : 8;
   /** Extended Firmware Version and Variant is supported */
   IFX_uint32_t EVER : 1;
   /** The handshake mechanism for Clock Scaling mode is supported */
   IFX_uint32_t CLKHS : 1;
   /** RTCP-XR support */
   IFX_uint32_t XR : 1;
   /** AMR Encoder
       0 - old style,
       1 - bit rate can be set in a separate bitfield */
   IFX_uint32_t AMRE : 1;
   /** Hook Timestamp
       0 - Timestamp is absent in raw hook events.
       1 - Timestamp is present in raw hook events. */
   IFX_uint32_t HTS : 1;
   /** PCM Split TimeSlots */
   IFX_uint32_t PSTS : 1;
   /** Burst number added to RTCP-XR message. */
   IFX_uint32_t XR_BN : 1;
   /** Reserved */
   IFX_uint32_t Res07 : 25;
} __PACKED__ ;

#define  SYS_CAP_ECMD 7
/* Available COD channel coders (caps.CODECS) */
#define CODEC_L16                    0x0001
#define CODEC_L16_16                 0x0002
#define CODEC_G711                   0x0004
#define CODEC_G726                   0x0008
#define CODEC_AMR_NB                 0x0010
#define CODEC_G728                   0x0020
#define CODEC_G729AB                 0x0040
#define CODEC_G722                   0x0080
#define CODEC_G722_1                 0x0100
#define CODEC_ILBC                   0x0200
#define CODEC_G723_1                 0x0400
#define CODEC_G729E                  0x0800
#define CODEC_G729EV                 0x1000
#define CODEC_AMR_WB                 0x2000
/*#define CODEC_ISAC                 0x4000*/
/*#define CODEC_G729E                0x8000*/
/** EDSP FW feature encoding (caps.FEAT) */
#define EDSP_CAP_FEAT_UTGUD            0x01
#define EDSP_CAP_FEAT_HDX              0x02
#define EDSP_CAP_FEAT_FDX              0x04
#define EDSP_CAP_FEAT_ISR16            0x08
#define EDSP_CAP_FEAT_CHAUD            0x10
#define EDSP_CAP_FEAT_CHVR             0x20
/** Available PCM channel coders (caps.PCOD) */
#define EDSP_CAP_PCMCOD_PL16           0x01
#define EDSP_CAP_PCMCOD_PL16_16        0x02
#define EDSP_CAP_PCMCOD_PG711          0x04
#define EDSP_CAP_PCMCOD_PG726          0x08
#define EDSP_CAP_PCMCOD_PG722          0x10


/**
   The command Smart SLIC Status (SDD specific) allows to find out, if a
   Smart SLIC is attached.
*/
typedef struct VMMC_SYS_SLIC_TEST
{
   CMD_HEAD_BE;
   /** reserved */
   IFX_uint32_t reserved : 24;
   /** SmartSLIC status */
   IFX_uint32_t SmartSLICStatus : 8;
} __PACKED__ VMMC_SYS_SLIC_TEST_t;

#define  SYS_SLIC_TEST_ECMD            14
#define  SYS_SLIC_TEST_LENGTH           4
#define  SYS_SLIC_TEST_DISCONNECTED   0x0
#define  SYS_SLIC_TEST_CONNECTED      0x1


/**
   This command is sent by the control CPU in two cases:
   - To start the handshake mechanism for clock scaling.
   - To acknowledge that the request to set high system clock frequency was
     executed and clock is now high enough for the DART to wake up.
*/
struct SYS_CLOCK_HIGH
{
   CMD_HEAD_BE;
} __PACKED__ ;

#define  SYS_CLOCK_HIGH_ECMD 17
#define  SYS_CLOCK_HIGH_LEN 0


/**
 * This command allows to start and stop the announcement. All bit fields,
 * especially ADDR may be overwritten. That means, an announcement may be
 * restarted or a new announcement may be started while a playback is active.
 */
typedef struct COD_ANN_CTRL
{
   CMD_HEAD_BE;
   /** Start playback */
   IFX_uint32_t E : 1;
   /** Loop the announcement */
   IFX_uint32_t L : 1;
   /** Address */
   IFX_uint32_t ADDR : 30;
} __PACKED__ COD_ANN_CTRL_t;

#define COD_ANN_CTRL_ECMD 29
#define COD_ANN_CTRL_LEN 4
#define COD_ANN_CTRL_E_STOP 0
#define COD_ANN_CTRL_E_START 1
#define COD_ANN_CTRL_L_OFF 0
#define COD_ANN_CTRL_L_ON 1


/** Coder Channel MOS_CQE Support Configuration */
typedef struct CMD_COD_CFG_STAT_MOS
{
    CMD_HEAD_BE;
    /** Default R value */
    IFX_uint32_t R_DEF : 16;
    /** Calcualtion Time Interval */
    IFX_uint32_t CTI : 16;
    /** MOS threshold for reporting event to controller */
    IFX_uint32_t MOS_CQE_TH : 16;
    /** Advantage Factor */
    IFX_uint32_t A_FACT : 16;
} __PACKED__ CMD_COD_CFG_STAT_MOS_t;

#define CMD_COD_CFG_STAT_MOS_ECMD 24
#define CMD_COD_CFG_STAT_MOS_LEN 8


/** Coder Channel MOS_CQE Support */
typedef struct CMD_COD_READ_STAT_MOS
{
    CMD_HEAD_BE;
    /** Reserved */
    IFX_uint32_t Res01 : 16;
    /** Calculated MOS_CQE value from latest CTI (calculation time
    interval ) */
    IFX_uint32_t MOS_CQE : 16;
} __PACKED__ CMD_COD_READ_STAT_MOS_t;

#define CMD_COD_READ_STAT_MOS_ECMD 28
#define CMD_COD_READ_STAT_MOS_LEN 4


typedef struct VMMC_CMD
{
   CMD_HEAD_BE;
} __PACKED__ VMMC_CMD_t;


typedef union VMMC_Msg
{
   IFX_uint32_t val[30];
   VMMC_CMD_t cmd;
   RES_LEC_COEF_t res_LecCoef;
} VMMC_Msg_t;


/* Gains to be used in various FW messages of all FW modules */

/** Define for a gain of 0 dB */
#define VMMC_GAIN_0DB                0x2000

/** Minimum volume gain: -24dB
    (practically -inf is the minimum gain supported) */
#define VMMC_VOLUME_GAIN_MIN        -24
/** Maximum volume gain: +12dB
    (practically 12dB is the maximum gain supported) */
#define VMMC_VOLUME_GAIN_MAX         12
/** Calculated table to convert the gain in 'dB' into the FW values
    in steps of 1dB. */
extern const IFX_uint16_t VMMC_Gaintable [49];

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* _DRV_VMMC_FW_COMMANDS_VOIP_H */
