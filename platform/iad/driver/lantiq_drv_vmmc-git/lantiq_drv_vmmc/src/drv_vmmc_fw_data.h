/*******************************************************************************

                              Copyright (c) 2012
                            Lantiq Deutschland GmbH
                             http://www.lantiq.com

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/


#ifndef _VMMC_FW_SPEC_DATA_H_
#define _VMMC_FW_SPEC_DATA_H_

/**
   \file drv_vmmc_fw_data.h
   This file contains the definitions of the voice-FW data interface.
   To reduce copying, data between the control CPU and the voice CPU are
   exchanged not directly, but by exchanging pointers to SDRAM areas, where
   the actual data are stored.
*/

#ifndef __PACKED__
   #if defined (__GNUC__) || defined (__GNUG__)
      /* GNU C or C++ compiler */
      #define __PACKED__ __attribute__ ((packed))
   #elif !defined (__PACKED__)
      #define __PACKED__      /* nothing */
   #endif
#endif


/** @defgroup _VMMC_FW_SPEC_DATA_ Data Messages
 *  @{
 */

#ifdef __cplusplus
   extern "C" {
#endif

/* ----- Include section ----- */
/* ----- Include section (End) ----- */

/* ----- Define section ----- */
/* ----- Define section (End) ----- */

#define  DAT_PAYL_PTR_MSG_VR_STATUS_PACKET  0
#define  DAT_PAYL_PTR_MSG_VR_DATA_PACKET  1
#define  DAT_PAYL_PTR_MSG_VOICE_PACKET  4
#define  DAT_PAYL_PTR_MSG_EVENT_PACKET  5
#define  DAT_PAYL_PTR_MSG_DECT_PACKET  7
#define  DAT_PAYL_PTR_MSG_HDLC_PACKET  9
#define  DAT_PAYL_PTR_MSG_FAX_DATA_PACKET  16
#define  DAT_PAYL_PTR_MSG_FAX_STATUS_PACKET  17
#define  DAT_PAYL_PTR_MSG_FAX_T38_PACKET   17
#define  DAT_PAYL_PTR_MSG_CID_DATA_PACKET  20
#define  DAT_PAYL_PTR_MSG_TEST_DATA_PACKET 30
#define  DAT_RTP_VOICE_US_NO  0x0
#define  DAT_RTP_VOICE_US_YES  0x1
#define  DAT_RTP_SID_US_NO  0x0
#define  DAT_RTP_EVENT_DS_DTMF_DIG_0  0
#define  DAT_RTP_EVENT_DS_DTMF_DIG_1  1
#define  DAT_RTP_EVENT_DS_DTMF_DIG_2  2
#define  DAT_RTP_EVENT_DS_DTMF_DIG_3  3
#define  DAT_RTP_EVENT_DS_DTMF_DIG_4  4
#define  DAT_RTP_EVENT_DS_DTMF_DIG_5  5
#define  DAT_RTP_EVENT_DS_DTMF_DIG_6  6
#define  DAT_RTP_EVENT_DS_DTMF_DIG_7  7
#define  DAT_RTP_EVENT_DS_DTMF_DIG_8  8
#define  DAT_RTP_EVENT_DS_DTMF_DIG_9  9
#define  DAT_RTP_EVENT_DS_DTMF_DIG_STAR  10
#define  DAT_RTP_EVENT_DS_DTMF_DIG_HASH  11
#define  DAT_RTP_EVENT_DS_DTMF_DIG_A  12
#define  DAT_RTP_EVENT_DS_DTMF_DIG_B  13
#define  DAT_RTP_EVENT_DS_DTMF_DIG_C  14
#define  DAT_RTP_EVENT_DS_DTMF_DIG_D  15
#define  DAT_RTP_EVENT_DS_ANS_2100_15  32
#define  DAT_RTP_EVENT_DS_ANS_2100_15_450_25  33
#define  DAT_RTP_EVENT_DS_ANSAM_2100_1_15_01  34
#define  DAT_RTP_EVENT_DS_ANSAM_2100_1_450_25_15_01  35
#define  DAT_RTP_EVENT_DS_CNG_1100  36
#define  DAT_RTP_EVENT_DS_DIS_V21  54
#define  DAT_RTP_EVENT_US_P_NO  0x0
#define  DAT_RTP_EVENT_US_NO  0x0
#define  DAT_RTP_EVENT_US_YES  0x1
#define  DAT_RTP_EVENT_US_DTMF_DIG_0  0x00
#define  DAT_RTP_EVENT_US_DTMF_DIG_1  0x01
#define  DAT_RTP_EVENT_US_DTMF_DIG_2  0x02
#define  DAT_RTP_EVENT_US_DTMF_DIG_3  0x03
#define  DAT_RTP_EVENT_US_DTMF_DIG_4  0x04
#define  DAT_RTP_EVENT_US_DTMF_DIG_5  0x05
#define  DAT_RTP_EVENT_US_DTMF_DIG_6  0x06
#define  DAT_RTP_EVENT_US_DTMF_DIG_7  0x07
#define  DAT_RTP_EVENT_US_DTMF_DIG_8  0x08
#define  DAT_RTP_EVENT_US_DTMF_DIG_9  0x09
#define  DAT_RTP_EVENT_US_DTMF_DIG_STAR  0x0A
#define  DAT_RTP_EVENT_US_DTMF_DIG_HASH  0x0B
#define  DAT_RTP_EVENT_US_DTMF_DIG_A  0x0C
#define  DAT_RTP_EVENT_US_DTMF_DIG_B  0x0D
#define  DAT_RTP_EVENT_US_DTMF_DIG_C  0x0E
#define  DAT_RTP_EVENT_US_DTMF_DIG_D  0x0F
#define  DAT_RTP_EVENT_US_ANS_2100_15  0x20
#define  DAT_RTP_EVENT_US_ANS_2100_15_450_25  0x21
#define  DAT_RTP_EVENT_US_ANSAM_2100_1_15_01  0x22
#define  DAT_RTP_EVENT_US_ANSAM_2100_1_450_25_15_01  0x23
#define  DAT_RTP_EVENT_US_CNG_1100  36
#define  DAT_RTP_EVENT_US_DIS_V21  54
#define  DAT_FAX_STAT_US_FINISHED  0x1
#define  DAT_FAX_STAT_US_LS_UNKNOWN  0x00000
#define  DAT_FAX_STAT_US_LS_V21  0x00001
#define  DAT_FAX_STAT_US_LS_V27_2400  0x00010
#define  DAT_FAX_STAT_US_LS_27_4800  0x00011
#define  DAT_FAX_STAT_US_LS_V29_7200  0x00100
#define  DAT_FAX_STAT_US_LS_V29_9600  0x00101
#define  DAT_FAX_STAT_US_LS_V17_7200  0x00110
#define  DAT_FAX_STAT_US_LS_V17_9600  0x00111
#define  DAT_FAX_STAT_US_LS_V17_12000  0x01000
#define  DAT_FAX_STAT_US_LS_V17_14400  0x01001
#define  DAT_FAX_STAT_US_LS_CNG  0x01010
#define  DAT_FAX_STAT_US_LS_CED  0x01011
#define  DAT_FAX_STAT_US_LS_TEP  0x01100
#define  DAT_FAX_STAT_US_LS_NO  0x11111
#define  DAT_FAX_DATA_DS_MOD_FINISHED  0x1
#define  DAT_FAX_DATA_US_FINISHED  0x1
#define  DAT_VR_STAT_US_YES  0x1

typedef struct DAT_PAYL_PTR_MSG DAT_PAYL_PTR_MSG_t;
typedef struct DAT_ADR_PTR_MSG DAT_ADR_PTR_MSG_t;
typedef struct DAT_RTP_VOICE_DS DAT_RTP_VOICE_DS_t;
typedef struct DAT_RTP_VOICE_US DAT_RTP_VOICE_US_t;
typedef struct DAT_RTP_SID_DS DAT_RTP_SID_DS_t;
typedef struct DAT_RTP_SID_US DAT_RTP_SID_US_t;
typedef struct DAT_RTP_EVENT_DS DAT_RTP_EVENT_DS_t;
typedef struct DAT_RTP_EVENT_US DAT_RTP_EVENT_US_t;
typedef struct DAT_FAX_STAT_US DAT_FAX_STAT_US_t;
typedef struct DAT_FAX_DATA_DS DAT_FAX_DATA_DS_t;
typedef struct DAT_FAX_DATA_US DAT_FAX_DATA_US_t;
typedef struct DAT_CIDR_DATA_US DAT_CIDR_DATA_US_t;
typedef struct DAT_VR_STAT_US DAT_VR_STAT_US_t;
typedef struct DAT_VR_DATA_DS DAT_VR_DATA_DS_t;
typedef struct DAT_VR_DATA_US DAT_VR_DATA_US_t;

/**
   This message is used to send or receive pointers to data packets. Upstream messages
   are read from the data upstream mailbox, downstream messages are written to the data
   downstream mailbox.
*/
struct DAT_PAYL_PTR_MSG
{
   /** Reserved, write zeros, read as dont care. */
   IFX_uint32_t Res00 : 3;
   /** Data Type */
   IFX_uint32_t CMD : 5;
   /** Reserved */
   IFX_uint32_t Res01 : 4;
   /** Channel */
   IFX_uint32_t CHAN : 4;
   /** Reserved */
   IFX_uint32_t Res02 : 8;
   /** Length of Instruction Part in Byte */
   IFX_uint8_t LENGTH;
   /** Address */
   IFX_uint32_t ADR;
   /** Length */
   IFX_uint32_t LEN;
} __PACKED__ ;


/**
   This message is used to send pointers to empty memory regions from the control CPU
   to the voice CPU. The voice CPU fills these memory regions with data and sends the
   pointers upstream by a Data Pointer Message. In upstream direction, this message
   is used to release memory regions to the control CPU, which contained downstream
   data.
*/
struct DAT_ADR_PTR_MSG
{
   /** Reserved */
   IFX_uint32_t Res00 : 3;
   /** Command */
   IFX_uint32_t CMD : 5;
   /** Reserved */
   IFX_uint32_t Res01 : 4;
   /** Channel */
   IFX_uint32_t CHAN : 4;
   /** Reserved */
   IFX_uint32_t Res02 : 8;
   /** Length of Instruction Part in Byte */
   IFX_uint8_t LENGTH;
   /** Address (1..n */
   IFX_uint32_t ADR;
   /** Length */
   IFX_uint32_t LEN;
} __PACKED__ ;


/**
   This format holds voice data packets which are received from the network and are
   formatted according to RFC 3550.
*/
struct DAT_RTP_VOICE_DS
{
   /** RTP version */
   IFX_uint32_t V : 2;
   /** Padding */
   IFX_uint32_t P : 1;
   /** Extension */
   IFX_uint32_t X : 1;
   /** CSRC Count */
   IFX_uint32_t CC : 4;
   /** Marker Bit */
   IFX_uint32_t M : 1;
   /** Payload Type */
   IFX_uint32_t PT : 7;
   /** Sequence Number */
   IFX_uint16_t SEQ_NR;
   /** Timestamp */
   IFX_uint32_t TS;
   /** Synchronization Source Identifier */
   IFX_uint32_t SSRC;
   /** Contributing Source Identifier (optional) */
   IFX_uint32_t CSRC;
   /** Header Extension High Word (optional) */
   IFX_uint16_t HEXT_HIGH;
   /** Header Extension Low Word (optional) */
   IFX_uint16_t HEXT_LOW;
   /** Payload Data */
   IFX_uint32_t DATA;
} __PACKED__ ;


/**
   This describes the payload format generated by the firmware according to RFC 3550.
*/
struct DAT_RTP_VOICE_US
{
   /** RTP version */
   IFX_uint32_t V : 2;
   /** Padding */
   IFX_uint32_t P : 1;
   /** Extension */
   IFX_uint32_t X : 1;
   /** CSRC Count */
   IFX_uint32_t CC : 4;
   /** Marker Bit */
   IFX_uint32_t M : 1;
   /** Payload Type */
   IFX_uint32_t PT : 7;
   /** Sequence Number */
   IFX_uint16_t SEQ_NR;
   /** Timestamp */
   IFX_uint32_t TS;
   /** Synchronization Source Identifier */
   IFX_uint32_t SSRC;
   /** Contributing Source Identifier (optional) */
   IFX_uint32_t CSRC;
   /** Payload Data */
   IFX_uint32_t DATA;
} __PACKED__ ;


/**
   The following format is a proprietary format for silence indication (SID) packets
   for G.711 and G.726. For G.723.1 and G.729A/B the SID packet format is defined
   within the standard.
*/
struct DAT_RTP_SID_DS
{
   /** RTP Version */
   IFX_uint32_t V : 2;
   /** Padding */
   IFX_uint32_t P : 1;
   /** Extension */
   IFX_uint32_t X : 1;
   /** CSRC Count */
   IFX_uint32_t CC : 4;
   /** Marker */
   IFX_uint32_t M : 1;
   /** Payload Type */
   IFX_uint32_t PT : 7;
   /** Sequence Number */
   IFX_uint16_t SEQ_NR;
   /** Timestamp */
   IFX_uint32_t TIME_STAMP;
   /** Synchronization Source Identifier */
   IFX_uint32_t SSRC;
   /** Contributing Source Identifiers */
   IFX_uint32_t CSRC;
   /** Header Extension, High Word */
   IFX_uint16_t HDR_EXT_HW;
   /** Header Extension, Low Word */
   IFX_uint16_t HDR_EXT_LW;
   /** Noise Level */
   IFX_uint32_t LEV : 8;
   /** Quantized Reflection Coefficient 1 */
   IFX_uint32_t N1 : 8;
   /** Quantized Reflection Coefficient 2 */
   IFX_uint32_t N2 : 8;
   /** Quantized Reflection Coefficient 3 */
   IFX_uint32_t N3 : 8;
   /** Quantized Reflection Coefficient 4 */
   IFX_uint32_t N4 : 8;
   /** Quantized Reflection Coefficient 5 */
   IFX_uint32_t N5 : 8;
   /** Quantized Reflection Coefficient 6 */
   IFX_uint32_t N6 : 8;
   /** Quantized Reflection Coefficient 7 */
   IFX_uint32_t N7 : 8;
   /** Quantized Reflection Coefficient 8 */
   IFX_uint32_t N8 : 8;
   /** Quantized Reflection Coefficient 9 */
   IFX_uint32_t N9 : 8;
   /** Quantized Reflection Coefficient 10 */
   IFX_uint32_t N10 : 8;
   /** Reserved */
   IFX_uint32_t Res00 : 8;
} __PACKED__ ;


/**
   The following format is a proprietary format for the SID packets used for G.711 and
   G.726. For G.723.1 and G.729A/B, the packet format is defined in the standard.
*/
struct DAT_RTP_SID_US
{
   /** RTP Version */
   IFX_uint32_t V : 2;
   /** Padding */
   IFX_uint32_t P : 1;
   /** Header Extension */
   IFX_uint32_t X : 1;
   /** CSRC Count */
   IFX_uint32_t CC : 4;
   /** Marker Bit */
   IFX_uint32_t M : 1;
   /** Payload Type */
   IFX_uint32_t PT : 7;
   /** Sequence Number */
   IFX_uint16_t SEQ_NR;
   /** Timestamp */
   IFX_uint32_t TIME_STAMP;
   /** Synchronization Source Identifier */
   IFX_uint32_t SSRC;
   /** Contributing Source Identifier */
   IFX_uint32_t CSRC;
   /** Noise Level */
   IFX_uint32_t LEV : 8;
   /** Quantized Reflection Coefficient 1 */
   IFX_uint32_t N1 : 8;
   /** Quantized Reflection Coefficient 2 */
   IFX_uint32_t N2 : 8;
   /** Quantized Reflection Coefficient 3 */
   IFX_uint32_t N3 : 8;
   /** Quantized Reflection Coefficient 4 */
   IFX_uint32_t N4 : 8;
   /** Quantized Reflection Coefficient 5 */
   IFX_uint32_t N5 : 8;
   /** Quantized Reflection Coefficient 6 */
   IFX_uint32_t N6 : 8;
   /** Quantized Reflection Coefficient 7 */
   IFX_uint32_t N7 : 8;
   /** Quantized Reflection Coefficient 8 */
   IFX_uint32_t N8 : 8;
   /** Quantized Reflection Coefficient 9 */
   IFX_uint32_t N9 : 8;
   /** Quantized Reflection Coefficient 10 */
   IFX_uint32_t N10 : 8;
   /** Reserved */
   IFX_uint32_t Res00 : 8;
} __PACKED__ ;


/**
   This format holds RTP event packets according to RFC 2833. Supported feature is the
   DTMF generation.
*/
struct DAT_RTP_EVENT_DS
{
   /** RTP Version */
   IFX_uint32_t V : 2;
   /** Padding */
   IFX_uint32_t P : 1;
   /** Extension */
   IFX_uint32_t X : 1;
   /** CSRC Count */
   IFX_uint32_t CC : 4;
   /** Marker Bit */
   IFX_uint32_t M : 1;
   /** Payload Type */
   IFX_uint32_t PT : 7;
   /** Sequence Number */
   IFX_uint16_t SEQ_NR;
   /** Timestamp, High Word */
   IFX_uint32_t TIME_STAMP;
   /** Synchronization Source */
   IFX_uint32_t SSRC;
   /** Contributing Source Identifier */
   IFX_uint32_t CSRC;
   /** Header Extension, High Word */
   IFX_uint16_t HDR_EXT_HW;
   /** Header Extension, Low Word */
   IFX_uint16_t HDR_EXT_LW;
   /** Event */
   IFX_uint8_t EVENT1;
   /** End Bit */
   IFX_uint32_t E1 : 1;
   /** Reserved */
   IFX_uint32_t R1 : 1;
   /** Power Level for the Event */
   IFX_uint32_t VOLUME1 : 6;
   /** Duration */
   IFX_uint16_t DURATION1;
   /** Event 2 */
   IFX_uint8_t EVENT2;
   /** End Bit */
   IFX_uint32_t E2 : 1;
   /** Reserved */
   IFX_uint32_t R2 : 1;
   /** Power Level for Event 2 */
   IFX_uint32_t VOLUME2 : 6;
   /** Duration2 */
   IFX_uint16_t DURATION2;
} __PACKED__ ;


/**
   The format holds DTMF and ATD event payload according to RFC 2833.
*/
struct DAT_RTP_EVENT_US
{
   /** RTP Version */
   IFX_uint32_t V : 2;
   /** Padding */
   IFX_uint32_t P : 1;
   /** Header Extension */
   IFX_uint32_t X : 1;
   /** CSRC Count */
   IFX_uint32_t CC : 4;
   /** Marker Bit */
   IFX_uint32_t M : 1;
   /** Payload Type */
   IFX_uint32_t PT : 7;
   /** Sequence Number */
   IFX_uint16_t SEQ_NR;
   /** Timestamp, High Word */
   IFX_uint32_t TIME_STAMP;
   /** Synchronization Source, High Word */
   IFX_uint32_t SSRC;
   /** Event */
   IFX_uint8_t EVENT;
   /** End Bit */
   IFX_uint32_t E : 1;
   /** Reserved */
   IFX_uint32_t Res00 : 1;
   /** Power Level for DTMF, ATD and DIS Events */
   IFX_uint32_t VOLUME : 6;
   /** Duration */
   IFX_uint16_t DURATION;
} __PACKED__ ;


/**
   This commands describes the structure of a FAX Relay status payload.
*/
struct DAT_FAX_STAT_US
{
   /** End */
   IFX_uint32_t END : 1;
   /** Reserved */
   IFX_uint32_t RES00 : 10;
   /** Line Status */
   IFX_uint32_t LS : 5;
} __PACKED__ ;


/**
   This chapter describes the payload format for downstream FAX Relay data packets.
*/
struct DAT_FAX_DATA_DS
{
   /** End */
   IFX_uint32_t END : 1;
   /** Reserved */
   IFX_uint32_t Res00 : 15;
   /** Data Byte */
   IFX_uint32_t Data_Byte0 : 8;
   /** Data Byte */
   IFX_uint32_t Data_Byte1 : 8;
} __PACKED__ ;


/**
   This chapter describes the payload format for an upstream FAX relay data packet.
*/
struct DAT_FAX_DATA_US
{
   /** End */
   IFX_uint32_t END : 1;
   /** Reserved */
   IFX_uint32_t Res00 : 15;
   /** Data Byte */
   IFX_uint32_t Data_Byte0 : 8;
   /** Data Byte */
   IFX_uint32_t Data_Byte1 : 8;
} __PACKED__ ;


/**
   This chapter describes the command for an upstream CID receiver packet.
*/
struct DAT_CIDR_DATA_US
{
   /** Carrier Detected */
   IFX_uint32_t CD : 1;
   /** Reserved */
   IFX_uint32_t RES00 : 15;
   /** Received Data Byte N */
   IFX_uint8_t DATA_N;
   /** Received Data Byte N+1 */
   IFX_uint8_t DATA_N_PLUS1;
} __PACKED__ ;


/**
   This chapter describes the speech recognition status payload format. The payload
   contains the result of the recognition process.
   The length of the payload is variable and can be 2, 4, 6, 8, 10 or 12 byte for
   recognition mode and 2 or 4 byte for training mode.
*/
struct DAT_VR_STAT_US
{
   /** Reserved */
   IFX_uint32_t Res00 : 1;
   /** Training Mode */
   IFX_uint32_t TR : 1;
   /** Reserved */
   IFX_uint32_t Res01 : 6;
   /** Status */
   IFX_uint32_t STAT : 2;
   /** Reserved */
   IFX_uint32_t Res02 : 3;
   /** Number of Hits */
   IFX_uint32_t NR_HITS : 3;
   /** Score */
   IFX_uint8_t SCORE0;
   /** Reference Number */
   IFX_uint8_t REF_NR0;
   /** Score */
   IFX_uint8_t SCORE1;
   /** Reference Number */
   IFX_uint8_t REF_NR1;
   /** Score */
   IFX_uint8_t SCORE2;
   /** Reference Number */
   IFX_uint8_t REF_NR2;
   /** Score */
   IFX_uint8_t SCORE3;
   /** Reference Number */
   IFX_uint8_t REF_NR3;
   /** Score */
   IFX_uint8_t SCORE4;
   /** Reference Number */
   IFX_uint8_t REF_NR4;
} __PACKED__ ;


/**
   This section describes the speech recognition data payload format in downstream
   direction. The payload contains the reference vector(s) of a word in the dictionary.
   These reference vectors have to be provided by the control CPU to the voice CPU for
   comparison with the DTW (dynamic time warping) algorithm in training and recognition
   mode.
*/
struct DAT_VR_DATA_DS
{
   /** Reserved */
   IFX_uint32_t Res00 : 8;
   /** Reference Number */
   IFX_uint32_t REF_NR : 8;
   /** Data */
   IFX_uint16_t DATA0;
   /** Data */
   IFX_uint32_t DATA1;
   /** Data */
   IFX_uint16_t DATA2;
} __PACKED__ ;


/**
   This section describes the speech recognition data payload format in upstream
   direction. The payload contains the reference vector(s) of a trained or updated
   word.
*/
struct DAT_VR_DATA_US
{
   /** Reserved */
   IFX_uint32_t Res00 : 8;
   /** Reference Number */
   IFX_uint32_t REF_NR : 8;
   /** Data */
   IFX_uint16_t DATA0;
   /** Data */
   IFX_uint32_t DATA1;
   /** Data */
   IFX_uint16_t DATA2;
} __PACKED__ ;


#ifdef __cplusplus
}
#endif

/** @} */
#endif
