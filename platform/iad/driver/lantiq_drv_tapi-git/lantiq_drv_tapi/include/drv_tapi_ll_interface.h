#ifndef _DRV_TAPI_LL_INTERFACE_H
#define _DRV_TAPI_LL_INTERFACE_H

/******************************************************************************

                           Copyright (c) 2014, 2016
                        Lantiq Beteiligungs-GmbH & Co.KG
                             http://www.lantiq.com

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/
/**
   \file drv_tapi_ll_interface.h
   Contains the structures which are shared between the high-level TAPI and
    low-level TAPI drivers.
   These structures mainly contains the function pointers which will be exported
   by the low level driver.

   This file is divided in different sections depending on the firmware modules:
   - INTERRUPT_AND_PROTECTION_MODULE
   - CODER_MODULE
   - PCM_MODULE
   - SIG_MODULE
   - ALM_MODULE
   - CON_MODULE
   - MISC_MODULE

*/
/* ============================= */
/* Includes                      */
/* ============================= */

#include <ifx_types.h>
#include "drv_tapi_io.h"
#include "drv_tapi_kpi_io.h"

/* ============================= */
/* Local Macros  Definitions    */
/* ============================= */

/* Channel 255 indicates the control device */
#define IFX_TAPI_DEVICE_CH_NUMBER    255

/** MFTD Answering tone detector phase reversal threshold
    Defines on which occurence of a phase reversal the signal is sent when
    enabled. Allowed range: 1-3. */
#define IFX_TAPI_SIG_MFTD_PR_THRESHOLD      (2)

/** FXO event status bit numbers */
#define FXO_BATTERY  0
#define FXO_APOH     1
#define FXO_POLARITY 2
#define FXO_RING     3
/** FXO line mode bit */
#define FXO_LINE_MODE 31

/* Used as argument to CfgDupTimer_Override() to indicate override off. */
#define DUP_OVERRIDE_OFF   0

/* ================================ */
/* Enumerations                     */
/* ================================ */

/** Hook validation time array offsets for IFX_TAPI_LL_ALM_HookVt */
enum HOOKVT_IDX
{
   HOOKVT_IDX_HOOK_OFF = 0,
   HOOKVT_IDX_HOOK_ON,
   HOOKVT_IDX_HOOK_FLASH,
   HOOKVT_IDX_HOOK_FLASHMAKE,
   HOOKVT_IDX_DIGIT_LOW,
   HOOKVT_IDX_DIGIT_HIGH,
   HOOKVT_IDX_INTERDIGIT
};

/** Definition of different data streams */
typedef enum
{
   IFX_TAPI_STREAM_COD = 0,
   IFX_TAPI_STREAM_DECT = 1,
   IFX_TAPI_STREAM_HDLC = 3,
   IFX_TAPI_STREAM_MAX
} IFX_TAPI_STREAM_t;

/** Counters for packet statistics */
typedef enum
{
   /** total number of packets delivered without errors */
   TAPI_STAT_COUNTER_EGRESS_DELIVERED,
   /** total number of packets discarded */
   TAPI_STAT_COUNTER_EGRESS_DISCARDED,
   /** number of packets discarded due to congestion */
   TAPI_STAT_COUNTER_EGRESS_CONGESTED,
#if 0
   /** minimum latency of all packets transported */
   TAPI_STAT_COUNTER_EGRESS_LATENCY_MIN,
   /** maximum latency of all packets transported */
   TAPI_STAT_COUNTER_EGRESS_LATENCY_MAX,
   /** sum of the latency */
   TAPI_STAT_COUNTER_EGRESS_LATENCY_SUM,
#endif
   /** total number of packets delivered without errors */
   TAPI_STAT_COUNTER_INGRESS_DELIVERED,
   /** total number of packets discarded */
   TAPI_STAT_COUNTER_INGRESS_DISCARDED,
   /** number of packets discarded due to congestion */
   TAPI_STAT_COUNTER_INGRESS_CONGESTED,
#if 0
   /** minimum latency of all packets transported */
   TAPI_STAT_COUNTER_INGRESS_LATENCY_MIN,
   /** maximum latency of all packets transported */
   TAPI_STAT_COUNTER_INGRESS_LATENCY_MAX,
   /** sum of the latency */
   TAPI_STAT_COUNTER_INGRESS_LATENCY_SUM,
#endif
   /* maximum value of this enum */
   TAPI_STAT_COUNTER_MAX
} TAPI_STAT_COUNTER_t;

typedef enum
{
   IFX_TAPI_LL_TONE_DIR_NONE = 0,
   IFX_TAPI_LL_TONE_EXTERNAL = 0x1,
   IFX_TAPI_LL_TONE_INTERNAL = 0x2,
   IFX_TAPI_LL_TONE_BOTH = 0x3
} IFX_TAPI_LL_TONE_DIR_t;

/** Directions for DTMF dtetector activation and deactivation  */
typedef enum
{
   IFX_TAPI_LL_DTMFD_DIR_NONE = 0,
   IFX_TAPI_LL_DTMFD_DIR_EXTERNAL = 1,
   IFX_TAPI_LL_DTMFD_DIR_INTERNAL = 2,
   IFX_TAPI_LL_DTMFD_DIR_BOTH = 3
} IFX_TAPI_LL_DTMFD_DIR_t;

typedef struct {
   /* Describes the module type where the tone will be detected. */
   IFX_TAPI_MODULE_TYPE_t nMod;
   /* Direction from which to detect the DTMF signal. */
   IFX_TAPI_LL_DTMFD_DIR_t direction;
   /* Generate event when DTMF ends? */
   IFX_boolean_t bEndEvent;
} IFX_TAPI_LL_DTMFD_CFG_t;

typedef struct {
   /* Describes the module type where the tone will be detected. */
   IFX_TAPI_MODULE_TYPE_t nMod;
   /* Direction from which to detect the DTMF signal. */
   IFX_TAPI_LL_DTMFD_DIR_t direction;
   /* IFX_TRUE: override, IFX_FALSE: normal operation. */
   IFX_boolean_t bOverride;
   /* Enable or disable the DTMF detector. */
   IFX_enDis_t nOperation;
} IFX_TAPI_LL_DTMFD_OVERRIDE_t;

/** Directions for MF R2 dtetector activation and deactivation  */
typedef enum
{
   IFX_TAPI_LL_MF_R2_DIR_EXTERNAL = 0,
   IFX_TAPI_LL_MF_R2_DIR_INTERNAL = 1,
   IFX_TAPI_LL_MF_R2_DIR_BOTH = 2
} IFX_TAPI_LL_MF_R2_DIR_t;

typedef IFX_void_t      IFX_TAPI_LL_DEV_t;
typedef IFX_void_t      IFX_TAPI_LL_CH_t;
typedef IFX_uint16_t    IFX_TAPI_LL_ERR_t;

#ifndef DRV_TAPI_H
typedef struct _TAPI_DEV      TAPI_DEV;
typedef struct _TAPI_CHANNEL  TAPI_CHANNEL;
#endif /* DRV_TAPI_H */

typedef struct
{
   IFX_TAPI_COD_TYPE_t   dec_type;
   IFX_TAPI_COD_LENGTH_t dec_framelength;
} IFX_TAPI_DEC_DETAILS_t;


/** \defgroup TAPI_LL_INTERFACE TAPI Low-Level driver interface
   Lists all the functions which are registered by the low level TAPI driver */
/*@{*/

/** \defgroup INTERRUPT_AND_PROTECTION_MODULE Protection service
   This service is used to lock and unlock the interrupts for protecting the
   access to shared data structures.   */

/** \defgroup CODER_MODULE  Coder service
   This service is used to access the coder module functionalities like encoding
   and decoding, RTP, RTCP etc., */

/** \defgroup PCM_MODULE PCM module services
   The PCM - */

/** \defgroup SIG_MODULE Signaling module service
   This service includes the functionalities like DTMF receiver, Tone detection/
   Generation, CID receiver and sender */

/** \defgroup ALM_MODULE Analong Line interface Module service
   Contains the functionalities of ALM module */

/** \defgroup DECT_MODULE DECT module service
   This services provides access to the special coders/decoders that are used
   in conjunction with DECT packet streams. Also a special tone generator is
   provided for DECT channels. */

/** \defgroup CON_MODULE Connection Module
  This module provides functions to connect different DSP modules. It is used
  for conferencing, but also for basic dynamic connections */

/*@}*/

/* ============================= */
/* Structure for LEC data        */
/*                               */
/* ============================= */

typedef struct
{
   /* LEC operating mode */
   IFX_TAPI_WLEC_TYPE_t nOpMode;
   /* Non Linear Processing on or off */
   IFX_TAPI_WLEC_NLP_t bNlp;
   /* Gain for input or LEC off */
   IFX_int8_t nGainIn;
   /* Gain for ouput or LEC off */
   IFX_int8_t nGainOut;
   /** LEC tail length - unused only a storage needed for get function */
   IFX_int8_t nLen;
   /** Size of the near-end window in narrowband sampling mode. */
   IFX_TAPI_WLEC_WIN_SIZE_t   nNBNEwindow;
   /** Size of the far-end window in narrowband sampling mode.
       Note: this is used only if nOpMode is set to IFX_TAPI_LEC_TYPE_NFE */
   IFX_TAPI_WLEC_WIN_SIZE_t   nNBFEwindow;
   /** Size of the near-end window in wideband sampling mode. */
   IFX_TAPI_WLEC_WIN_SIZE_t   nWBNEwindow;
}TAPI_LEC_DATA_t;

/** tone play destinations */
typedef enum
{
   TAPI_TONE_DST_DEFAULT  = 0x0,
   TAPI_TONE_DST_LOCAL    = 0x1,
   TAPI_TONE_DST_NET      = 0x2,
   TAPI_TONE_DST_NETLOCAL = 0x3
}TAPI_TONE_DST;


/** Specifies the capability of the tone generator regarding tone
    sequence support. */
typedef enum
{
   /** Plays out a frequency or silence. No tone sequence with
      cadences are supported */
   IFX_TAPI_TONE_RESSEQ_FREQ = 0x0,
   /** Plays out a full simple tone including cadences and loops */
   IFX_TAPI_TONE_RESSEQ_SIMPLE = 0x1
}IFX_TAPI_TONE_RESSEQ_t;

/** Tone resource information. */
typedef struct
{
   /** Resource ID or number of the generator. Used as index in the tone status array and
   must be a number between 0 and TAPI_TONE_MAXRES */
   IFX_uint8_t nResID;
   /** Number of maximum supported frequencies at one time */
   IFX_uint8_t nFreq;
   /** Specifies the capability of the tone generator regarding tone
       sequence support. See \ref IFX_TAPI_TONE_RESSEQ_t for details. */
   IFX_TAPI_TONE_RESSEQ_t sequenceCap;
} IFX_TAPI_TONE_RES_t;

/* =============================== */
/* Defines for complex tone states */
/* =============================== */
typedef enum
{
   /** Initialization state */
   TAPI_CT_IDLE = 0,
   /** UTG tone sequence is not active, but the tone is in pause state */
   TAPI_CT_ACTIVE_PAUSE,
   /** Tone is currently playing out on the tone generator */
   TAPI_CT_ACTIVE,
   /* UTG tone sequence is deactived by the LL driver automatically. Afterwards a
   next step can be programmed or a new tone can be started. */
   TAPI_CT_DEACTIVATED

}TAPI_CMPLX_TONE_STATE_t;

typedef enum
{
   /** RTP protocol */
   IFX_TAPI_PRT_TYPE_RTP,
   /** AAL protocol */
   IFX_TAPI_PRT_TYPE_AAL
}IFX_TAPI_PRT_TYPE_t;

/* ============================= */
/* Structure for CID data        */
/* ============================= */

/** Caller ID transmission data types. */
typedef enum
{
   IFX_TAPI_CID_DATA_TYPE_FSK_BEL202,
   IFX_TAPI_CID_DATA_TYPE_FSK_V23,
   IFX_TAPI_CID_DATA_TYPE_DTMF,
} IFX_TAPI_CID_DATA_TYPE_t;

/** Structure used for starting the CID transmitter in the LL-driver. */
struct TAPI_CID_DATA;
typedef struct TAPI_CID_DATA TAPI_CID_DATA_t;

typedef enum
{
   TAPI_CID_ALERT_NONE,
   /** first ring burst */
   TAPI_CID_ALERT_FR,
   /** DTAS */
   TAPI_CID_ALERT_DTAS,
   /** Line Reversal with DTAS */
   TAPI_CID_ALERT_LR_DTAS,
   /** Ring Pulse */
   TAPI_CID_ALERT_RP,
   /** Open Switch Interval */
   TAPI_CID_ALERT_OSI,
   /** CPE Alert Signal */
   TAPI_CID_ALERT_CAS,
   /** Alert Signal (NTT) */
   TAPI_CID_ALERT_AS_NTT,
   /** CAR Signal (NTT) */
   TAPI_CID_ALERT_CAR_NTT,
   /** Line Reversal */
   TAPI_CID_ALERT_LR
} IFX_TAPI_CID_ALERT_TYPE_t;

typedef struct
{
   /** The configured CID standard */
   IFX_TAPI_CID_STD_t      nStandard;

   /** Type of ETSI Alert of onhook services associated to ringing.
      Default IFX_TAPI_CID_ALERT_ETSI_FR */
   IFX_TAPI_CID_ALERT_ETSI_t   nETSIAlertRing;
   /** Type of ETSI Alert of onhook services not associated to ringing.
      Default IFX_TAPI_CID_ALERT_ETSI_RP. */
   IFX_TAPI_CID_ALERT_ETSI_t   nETSIAlertNoRing;
   /** Tone table index for the alert tone to be used.
      Required for automatic CID/MWI generation. Default XXXXXd. */
   IFX_uint32_t            nAlertToneOnhook;
   IFX_uint32_t            nAlertToneOffhook;
   /* Time needed to play the alert tone. */
   IFX_uint32_t            nAlertToneTime;
   /** Ring Pulse on time interval. */
   IFX_uint32_t            ringPulseOnTime;
   /** Ring Pulse off time interval. */
   IFX_uint32_t            ringPulseOffTime;
   /** Ring Pulse Loops. */
   IFX_uint32_t            ringPulseLoop;
   /** DTMF ACK after CAS, used for offhook transmission. Default DTMF 'D'. */
   IFX_char_t              ackTone;
   /** Usage of OSI for offhook transmission. Default "no use" */
   IFX_uint32_t            OSIoffhook;
   /** Lenght of the OSI signal in ms. Default 200 ms. */
   IFX_uint32_t            OSItime;
   /** First ring cadence time, used for TAPI_CID_ALERT_FR */
   IFX_uint32_t            cadenceRingBurst;
   /** Cadence ring pause time for CID transmission, used for TAPI_CID_ALERT_FR */
   IFX_uint32_t            cadenceRingPause;
   /** Reception of 2nd ack signal timeout (after data transmission) (NTT)*/
   IFX_uint32_t            ack2Timeout;
   /** Duration of the Subscriber Alerting Signal (SAS) tone. */
   IFX_uint32_t            nSasToneTime;
   /** Time to wait before a SAS tone, in ms, for offhook services. */
   IFX_uint32_t            beforeSAStime;
   /** Time to wait between generation of SAS and CAS tone, in ms. */
   IFX_uint32_t            SAS2CAStime;
   /** Tone table index for the SAS tone to be used. */
   IFX_uint32_t            nSAStone;
   /** Do not transfer Caller ID */
   IFX_boolean_t           bRingOnly;

   IFX_TAPI_CID_ABS_REASON_t   TapiCidDtmfAbsCli;
   IFX_TAPI_CID_TIMING_t       TapiCidTiming;
   IFX_TAPI_CID_FSK_CFG_t      TapiCidFskConf;
   IFX_TAPI_CID_DTMF_CFG_t     TapiCidDtmfConf;
} IFX_TAPI_CID_CONF_t;

/** Struct used for starting the CID sequence in the LL-driver. */
typedef struct
{
   /* Select CID type 1 (ON-) or type 2 (OFF-HOOK) */
   IFX_TAPI_CID_HOOK_MODE_t   txHookMode;
   /* CID data to be sent */
   IFX_uint8_t                *pCidParam;
   /* number of CID data octets */
   IFX_uint16_t               nCidParamLen;
   /* alert type for sequence */
   IFX_TAPI_CID_ALERT_TYPE_t  nAlertType;
   /* flag for starting periodical ringing */
   IFX_boolean_t              bRingStart;
   /* caller id config */
   IFX_TAPI_CID_CONF_t        *pConfData;
   /* pointer to the cadence we currently use */
   IFX_uint8_t                *pCurrentCadence;
   /* copy of the number of bits in the cadence we currently use */
   IFX_uint16_t               BitsInCurrentCadence;
   /* pointer to the cadence we currently use */
   IFX_uint8_t                *pPeriodicCadence;
   /* copy of the number of bits in the cadence we currently use */
   IFX_uint16_t               BitsInPeriodicCadence;
   /* ringing will stop automatically after the maximum ring was reached */
   IFX_uint32_t               nMaxRings;
} IFX_TAPI_CID_SEQ_CONF_t;

typedef struct
{
   /* Select CID type 1 (ON-) or type 2 (OFF-HOOK) */
   IFX_TAPI_CID_HOOK_MODE_t   txHookMode;
   /* The type of CID data to be sent */
   IFX_TAPI_CID_DATA_TYPE_t   cidDataType;
   /* CID data to be sent */
   IFX_uint8_t                *pCidParam;
   /* number of CID data octets */
   IFX_uint16_t               nCidParamLen;
   /* FSK transmitter/receiver specific settings */
   IFX_TAPI_CID_FSK_CFG_t     *pFskConf;
   /* DTMF transmitter specific settings */
   IFX_TAPI_CID_DTMF_CFG_t    *pDtmfConf;
} IFX_TAPI_CID_TX_t;

/** Struct used for starting the CID receiver in the LL-driver. */
typedef struct
{
   /* Module type */
   IFX_TAPI_MODULE_TYPE_t module;
   /* The configured CID standard */
   IFX_TAPI_CID_STD_t         nStandard;
   /* Select CID type 1 (ON-) or type 2 (OFF-HOOK) */
   IFX_TAPI_CID_HOOK_MODE_t   txHookMode;
   /* FSK transmitter/receiver specific settings */
   IFX_TAPI_CID_FSK_CFG_t     *pFskConf;
} IFX_TAPI_CID_RX_t;

/* =============================== */
/* Defines for error reporting     */
/* =============================== */

/** Classify error when reported via event dispatcher. For internal use */
typedef enum
{
   /** Report TAPI channel specific error */
   IFX_TAPI_ERRSRC_TAPI_CH    = 0,
   /** Report TAPI device or global error */
   IFX_TAPI_ERRSRC_TAPI_DEV   = 0x1000,
   /** Report low level global error */
   IFX_TAPI_ERRSRC_LL_DEV     = 0x2000,
   /** Report low level channel specific error */
   IFX_TAPI_ERRSRC_LL_CH      = 0x4000,
   /** Bit mask for modification of low level driver error codes. This bit
       is set for low level driver error codes */
   IFX_TAPI_ERRSRC_LL         = 0x8000,
   /** Maks of error sources used for clearing */
   IFX_TAPI_ERRSRC_MASK       = (IFX_TAPI_ERRSRC_LL |
                                 IFX_TAPI_ERRSRC_LL_CH |
                                 IFX_TAPI_ERRSRC_LL_DEV |
                                 IFX_TAPI_ERRSRC_TAPI_DEV |
                                 IFX_TAPI_ERRSRC_TAPI_CH)
}IFX_TAPI_ERRSRC;


/* ============================= */
/* Defines for resource counts   */
/* ============================= */

/** Struct used for reporting resources from LL to HL. */
typedef struct
{
   /** Number of ALM modules or analog lines */
   IFX_uint16_t            AlmCount;
   /** Number of SIG modules */
   IFX_uint16_t            SigCount;
   /** Number of COD modules */
   IFX_uint16_t            CodCount;
   /** Number of PCM modules */
   IFX_uint16_t            PcmCount;
   /** Number of DECT coder modules */
   IFX_uint16_t            DectCount;
   /** Number of DTMF generators */
   IFX_uint16_t            DTMFGCount;
   /** Number of DTMF receivers */
   IFX_uint16_t            DTMFRCount;
   /** Number of FSK generators */
   IFX_uint16_t            FSKGCount;
   /** Number of FSK receivers */
   IFX_uint16_t            FSKRCount;
   /** Number of UGT/TG capable channels */
   IFX_uint16_t            ToneCount;
   /** Number of HDLC channel resources */
   IFX_uint16_t            HdlcCount;
   /** Number of MF R2 receivers */
   IFX_uint16_t            MF_R2Count;
   /* additional resources can be added here
      initialise them in ifx_tapi_Prepare_Dev() */
} IFX_TAPI_RESOURCE;

/* ============================= */
/* Structure for CERR reporting  */
/* ============================= */

/** Report the reason and details of a cmd error to drv_tapi. */
typedef struct _IFX_TAPI_DBG_CERR {
   /** reason code */
   IFX_uint32_t      cause;
   /** cmd header */
   IFX_uint32_t      cmd;
} IFX_TAPI_DBG_CERR_t;

/** Get debug information of an SSI crash event. */
struct IFX_TAPI_DBG_SSI_CRASH {
   /** debug information */
   IFX_uint32_t      cause[3];
};

/* ============================================= */
/* Structure for Capacitance measurement result  */
/* ============================================= */

/** Tip to Ring Capacitance measurement result. */
typedef struct _IFX_TAPI_NLT_T2R_CAPACITANCE_RESULT {
   /** Validity of measurement result tip to ring. */
   IFX_boolean_t  bValidTip2Ring;
   /** Measured capacitance tip to ring [nF]. */
   IFX_uint32_t   nCapTip2Ring;
} IFX_TAPI_NLT_T2R_CAPACITANCE_RESULT_t;

/** Line to Ground Capacitance measurement result. */
typedef struct _IFX_TAPI_NLT_L2GND_CAPACITANCE_RESULT {
   /** Validity of measurement results tip to ground and ring to ground.*/
   IFX_boolean_t  bValidLine2Gnd;
   /** Measured capacitance tip to ground [nF]. */
   IFX_uint32_t   nCapTip2Gnd;
   /** Measured capacitance ring to ground [nF]. */
   IFX_uint32_t   nCapRing2Gnd;
} IFX_TAPI_NLT_L2GND_CAPACITANCE_RESULT_t;

/* ============================================= */
/* Structure for open loop configuration         */
/* ============================================= */
/** Structure used for storing the open loop capacitance.
    The float values in here are correction factors that are just stored in
    the driver and will be returned together with the associated measurement
    results. */
typedef struct _IFX_TAPI_NLT_OL_CAPACITANCE_CONF {
   /** Open loop capacitance tip to ring [nF] (in). */
   IFX_float_t   fOlCapTip2Ring;
   /** Open loop capacitance tip to ground [nF] (in). */
   IFX_float_t   fOlCapTip2Gnd;
   /** Open loop capacitance ring to ground [nF] (in). */
   IFX_float_t   fOlCapRing2Gnd;
} IFX_TAPI_NLT_OL_CAPACITANCE_CONF_t;

/** Structure used for storing the open loop resistance.
    The float values in here are correction factors that are just stored in
    the driver and will be returned together with the associated measurement
    results. */
typedef struct _IFX_TAPI_NLT_OL_RESISTANCE_CONF {
   /** Open loop resistance tip to ring [Ohm] (in). */
   IFX_float_t   fOlResTip2Ring;
   /** Open loop resistance tip to ground [Ohm] (in). */
   IFX_float_t   fOlResTip2Gnd;
   /** Open loop resistance ring to ground [Ohm] (in). */
   IFX_float_t   fOlResRing2Gnd;
} IFX_TAPI_NLT_OL_RESISTANCE_CONF_t;

/* ============================= */
/* Defines for Driver Context    */
/* ============================= */

/** Interrupt and Protection Module */
/** \addtogroup INTERRUPT_AND_PROTECTION_MODULE */
/** Used for data protection by higher layer */
/*@{*/
typedef struct
{
   /** This function disables the irq line if the driver is in interrupt mode
      \param pLLDev     Handle to low-level device
   */
   IFX_void_t (*LockDevice) (IFX_TAPI_LL_DEV_t *pLLDev);
   /** This function enables the irq line if the driver is in interrupt mode
      \param pLLDev     Handle to low-level device
   */
   IFX_void_t (*UnlockDevice) (IFX_TAPI_LL_DEV_t *pLLDev);
   /** This function enables the irq line if the driver is in interrupt mode
      \param pLLDev     Handle to low-level device
   */
   IFX_void_t (*IrqEnable) (IFX_TAPI_LL_DEV_t *pLLDev);
   /** This function disables the irq line if the driver is in interrupt mode
      \param pLLDev     Handle to low-level device
   */
   IFX_void_t (*IrqDisable) (IFX_TAPI_LL_DEV_t *pLLDev);
} IFX_TAPI_DRV_CTX_IRQ_t;

/*@}*/ /* INTERRUPT_AND_PROTECTION_MODULE */


/** CODer Module */  /* ***************************************************** */
/** \addtogroup CODER_MODULE */
/** Used for Coder services higher layer */
/*@{*/
typedef struct
{
   /** Starts the coder in Upstream / record data
      \param pLLCh         Handle to low-level channel
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*ENC_Start) (IFX_TAPI_LL_CH_t *pLLCh);

   /** Stops the coder in Upstream / record data
      \param pLLCh         Handle to low-level channel
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*ENC_Stop) (IFX_TAPI_LL_CH_t *pLLCh);

   /** Put encoder into hold or unhold state.
      \param pLLCh         Handle to low-level channel
      \param nOnHold     Hold state (IFX_ENABLE - hold, IFX_DISABLE - unhold)
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*ENC_Hold) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_operation_t nOnHold);

   /** Sets the Recording codec and frame length
      \param pLLCh         Handle to low-level channel
      \param pEncCfg       Handle to ENC configuration

      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*ENC_Cfg) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_ENC_CFG_SET_t const *pEncCfg);

   /** Sets Decoder specific parameters
      \param pLLCh         Handle to low-level channel
      \param pTapiDecCfg   Pointer to DEC configuration
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*DEC_Cfg) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_DEC_CFG_t const *pTapiDecCfg);

   /** Turns the room noise detection mode on or off
      \param pLLCh            Handle to low-level channel
      \param bEnable          IFX_TRUE to enable or IFX_FALSE to disable
      \param nThreshold       detection level in minus dB
      \param nVoicePktCnt     count of consecutive voice packets required
                              for event
      \param nSilencePktCnt   count of consecutive silence packets required
                              for event
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*ENC_RoomNoise) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_boolean_t bEnable,
      IFX_uint32_t nThreshold,
      IFX_uint8_t nVoicePktCnt,
      IFX_uint8_t nSilencePktCnt);

   /** Starts the playing.
      \param pLLCh      Handle to low-level channel
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*DEC_Start) (IFX_TAPI_LL_CH_t *pLLCh);

   /** Stops the playing.
      \param pLLCh      Handle to low-level channel
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*DEC_Stop) (IFX_TAPI_LL_CH_t *pLLCh);

   /** Sets the Voice Activity Detection mode
      \param pLLCh      Handle to low-level channel
      \param nVAD       Switch On or Off
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*VAD_Cfg) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_int32_t nVAD);

   /** Enable/Disable the AGC resource inside the device
      \param pLLCh      Handle to low-level channel
      \param agcMode    AGC mode to be configured (enable/disable)
                        \ref IFX_TAPI_ENC_AGC_MODE_t
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*AGC_Enable) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_uint32_t agcMode);

   /** Enable/Disable the AGC resource inside the device
      \param pLLCh      Handle to low-level channel
      \param pAGC_Cfg   New AGC parameters
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*AGC_Cfg) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_ENC_AGC_CFG_t const *pAGC_Cfg);

   /** Configures the jitter buffer
      \param pLLCh      Handle to low-level channel
      \param pJbConf    Pointer to the Jitter buffer configuration
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*JB_Cfg) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_JB_CFG_t const *pJbConf);

   /** Query the Jitter buffer statistics
      \param pLLCh      Handle to low-level channel
      \param pJbData    Pointer to the Jitter buffer data
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*JB_Stat_Get) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_JB_STATISTICS_t *pJbData);

   /** Reset the Jitter buffer statistics
      \param pLLCh      Handle to low-level channel
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*JB_Stat_Reset) (IFX_TAPI_LL_CH_t *pLLCh);

   /** Configure RTP and RTCP for a connection
      \param pLLCh         Handle to low-level channel
      \param pRtpConf      Pointer to IFX_TAPI_PKT_RTP_CFG_t, RTP configuraton
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*RTP_Cfg) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_PKT_RTP_CFG_t const *pRtpConf);

   /** Configure a new payload type
      \param pLLCh         Handle to low-level channel
      \param pRtpPTConf    Pointer to RTP payload configuraton
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*RTP_PayloadTable_Cfg) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_PKT_RTP_PT_CFG_t const *pRtpPTConf);

   /** Configure the session packet header information
      \param pLLCh         Handle to low-level channel
      \param pNET_Conf     Pointer to session packet header configuration
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*NET_Cfg) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_PKT_NET_CFG_t const *pNET_Conf);

   /** Start or stop generation of RTP event packets
      \param pLLCh         Handle to low-level channel
      \param nEvent        Event code as defined in RFC2833
      \param nStart        Start (true) or stop (false)
      \param nDuration     Duration of event in units of 10 ms (0 = forever)
      \param nVolume       Volume of event. Value range [0-63] corresponding
                           to 0 dB to -63 dB. Values outside of the allowed
                           range will activate firmware defaults.
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*RTP_EV_Generate) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_uint8_t nEvent,
      IFX_boolean_t bStart,
      IFX_uint8_t nDuration,
      IFX_int8_t nVolume);

   /** Configuration for generation of ABCD RTP event packets
      \param pLLCh         Handle to low-level channel
      \param pCfg          Handle to configuration structure
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*RTP_EV_GenerateAbcdCfg) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_PKT_EV_GENERATE_ABCD_CFG_t const *pCfg);

   /** Gets the RTCP statistic information for the addressed channel
      \param pLLCh      Handle to low-level channel
      \param pRTCP      Pointer to RTCP Statistics structure
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*RTCP_Get) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_PKT_RTCP_STATISTICS_t *pRTCP);

   /**  Resets  RTCP statistics
      \param pLLCh      Handle to low-level channel
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*RTCP_Reset) (IFX_TAPI_LL_CH_t *pLLCh);

   /** Gets the block of RTCP XR statistic information for the addressed channel
      \param pLLCh      Handle to low-level channel
      \param pRTCP      Pointer to RTCP XR BLOCK structure
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*RTCP_XR_Get) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_PKT_RTCP_XR_BLOCK_GET_t *pRTCP);

   /**  Retrive and reset extended RTP statistics
      \param pLLCh      Handle to low-level channel
      \param pStat      Pointer to Extended RTP statistics structure
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*RTP_Ext_Stat_Get) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_PKT_RTP_EXT_STATISTICS_GET_t *pStat);

   /**  Configure extended RTP statistics
      \param pLLCh      Handle to low-level channel
      \param pCfg       Pointer to Extended RTP statistics
                        configuration structure
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*RTP_Ext_Stat_Cfg) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_PKT_RTP_EXT_STATISTICS_CFG_t const *pStat);

   /* CURRENTLY NOT SUPPORTED
      the following pair of function pointers can be used to retrieve ("gather") the RTCP statistics non blocking,
      therefore RTCP_Prepare must be called within a noninterruptible context
    */
   IFX_int32_t (*RTCP_Prepare_Unprot) (IFX_TAPI_LL_CH_t *pLLCh);
   IFX_int32_t (*RTCP_Prepared_Get) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_PKT_RTCP_STATISTICS_t *pRTCP);

   /* obsolete / reserved */
   IFX_int32_t (*AAL_Cfg) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_PCK_AAL_CFG_t const *pAalConf);

   /* obsolete / reserved */
   IFX_int32_t (*AAL_Profile_Cfg) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_PCK_AAL_PROFILE_t const *pProfile);

   /** Configures the Datapump for Modulation
      \param pLLCh      Handle to low-level channel
      \param pFaxMod    Pointer to the IFX_TAPI_T38_MOD_DATA_t structure
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*T38_Mod_Enable) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_T38_MOD_DATA_t const *pFaxMod);

   /** Configures the Datapump for Demodulation
      \param pLLCh      Handle to low-level channel
      \param pFaxDemod  Pointer to the IFX_TAPI_T38_DEMOD_DATA_t structure
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*T38_DeMod_Enable) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_T38_DEMOD_DATA_t const *pFaxDemod);

   /** Disables the Fax datapump
      \param pLLCh         Handle to low-level channel
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*T38_Datapump_Disable) (IFX_TAPI_LL_CH_t *pLLCh);

   /** Query the Fax Status
      \param pLLCh         Handle to low-level channel
      \param pFaxStatus    Pointer to T38 status structure where
                           the status will be copied
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*T38_Status_Get) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_T38_STATUS_t *pFaxStatus);

   /** Set the Fax Status
      \param pLLCh   Handle to low-level channel
      \param status  The status which to be set
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*T38_Status_Set) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_uint8_t status);

   /** Set the Fax Error Status
      \param pLLCh   Handle to low-level channel
      \param error   The error status which to be set
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*T38_Error_Set) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_uint8_t error);

   /** Decoder Change event reporting request new decoder details
      \param pLLCh         Handle to low-level channel
      \param pDecDetails   Pointer to a decoder details structure
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*DEC_Chg_Evt_Detail_Req) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_DEC_DETAILS_t *pDec);

   /** Switches on/off HP filter of decoder path
      \param pLLCh   Handle to low-level channel
      \param bHp     IFX_FALSE to switch HP off, IFX_TRUE to switch HP on
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*DEC_HP_Set) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_boolean_t bHp);

   /** Sets the COD interface volume
      \param pLLCh   Handle to low-level channel
      \param pVol            Pointer to the IFX_TAPI_PKT_VOLUME_t structure
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*Volume_Set) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_PKT_VOLUME_t const *pVol);

   /** Gets the supported T.38 fax channel implementation capabilities
      \param pLLDev  Handle to low-level device
      \param pCap    Pointer to the IFX_TAPI_T38_CAP_t structure
      \return
         IFX_SUCCESS if successful else device specific return code.
    */
   IFX_int32_t (*FAX_Cap_Get) (
      IFX_TAPI_LL_DEV_t *pLLDev,
      IFX_TAPI_T38_CAP_t *pCap);

   /** Gets the parameters of T.38 fax channel
      \param pLLCh   Handle to low-level channel
      \param pCfg    Pointer to the IFX_TAPI_T38_FAX_CFG_t structure
      \return
         IFX_SUCCESS if successful else device specific return code.
    */
   IFX_int32_t (*FAX_Cfg_Get) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_T38_FAX_CFG_t *pCfg);

   /** Sets the parameters of T.38 fax channel
      \param pLLCh   Handle to low-level channel
      \param pCfg    Pointer to the IFX_TAPI_T38_FAX_CFG_t structure
      \return
         IFX_SUCCESS if successful else device specific return code.
    */
   IFX_int32_t (*FAX_Cfg_Set) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_T38_FAX_CFG_t const *pCfg);

   /** Gets the current configuration of the FAX Data Pump parameters
      \param pLLCh      Handle to low-level channel
      \param pFDPCfg    Pointer to the IFX_TAPI_T38_FDP_CFG_t structure
      \return
         IFX_SUCCESS if successful else device specific return code.
    */
   IFX_int32_t (*FAX_FDP_Cfg_Get) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_T38_FDP_CFG_t *pFDPCfg);

   /** Sets the FAX Data Pump parameters
      \param pLLCh      Handle to low-level channel
      \param pFDPCfg    Pointer to the IFX_TAPI_T38_FDP_CFG_t structure
      \return
      IFX_SUCCESS if successful else device specific return code.
    */
   IFX_int32_t (*FAX_FDP_Cfg_Set) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_T38_FDP_CFG_t const *pFDPCfg);

   /** starts a T.38 fax session on the given channel
      \param pLLCh      Handle to low-level channel
      \param pT38Cfg    Pointer to the IFX_TAPI_T38_SESS_CFG_t structure
      \return
         IFX_SUCCESS if successful else device specific return code.
    */
   IFX_int32_t (*FAX_Start) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_T38_SESS_CFG_t const *pT38Cfg);

   /** Gets a T.38 fax session statistics
      \param pLLCh   Handle to low-level channel
      \param pStat   Pointer to IFX_TAPI_T38_SESS_STATISTICS_t structure
      \return
         IFX_SUCCESS if successful else device specific return code.
    */
   IFX_int32_t (*FAX_Stat_Get) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_T38_SESS_STATISTICS_t *pStat);

   /** stops currently running T.38 fax session on the given channel
      \param pLLCh   Handle to low-level channel
      \return
         IFX_SUCCESS if successful else device specific return code.
    */
   IFX_int32_t (*FAX_Stop) (IFX_TAPI_LL_CH_t *pLLCh);

   /** used to enable/disable a trace of FAX events
      \param pLLCh   Handle to low-level channel
      \param pTrace  Pointer to the IFX_TAPI_T38_TRACE_CFG_t structure
      \return
         IFX_SUCCESS if successful else device specific return code.
    */
   IFX_int32_t (*FAX_TraceSet) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_T38_TRACE_CFG_t const *pTrace);

   /** Adds (or downloads) an announcement file to the TAPI low-level
      (or firmware) memory
      \param pLLDev  Handle to low-level device
      \param pCfg    Pointer to \ref IFX_TAPI_COD_ANNOUNCE_CFG_t structure
      \return
         IFX_SUCCESS if successful else device specific return code.
    */
   IFX_int32_t (*Ann_Cfg) (
      IFX_TAPI_LL_DEV_t *pLLDev,
      IFX_TAPI_COD_ANNOUNCE_CFG_t const *pCfg);

   /** Release the buffer, which was allocated for an announcement
      \param pLLDev  Handle to low-level device
      \param pFree   Pointer to \ref IFX_TAPI_COD_ANNOUNCE_BUFFER_FREE_t structure
      \return
         IFX_SUCCESS if successful else device specific return code.
    */
   IFX_int32_t (*Ann_Free) (
      IFX_TAPI_LL_DEV_t *pLLDev,
      IFX_TAPI_COD_ANNOUNCE_BUFFER_FREE_t const *pFree);

   /** Starts playing announcement
      \param pLLCh   Handle to low-level channel
      \param pStart  Pointer to \ref IFX_TAPI_COD_ANNOUNCE_START_t structure
      \return
         IFX_SUCCESS if successful else device specific return code.
    */
   IFX_int32_t (*Ann_Start) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_COD_ANNOUNCE_START_t const *pStart);

   /** Stops playing announcment
      \param pLLCh   Handle to low-level channel
      \return
         IFX_SUCCESS if successful else device specific return code.
    */
   IFX_int32_t (*Ann_Stop) (IFX_TAPI_LL_CH_t *pLLCh);

   /** Configures the MOS calculation.
      \param pLLCh           Pointer to Low-level channel structure
      \param pMosCfg         Pointer to the IFX_TAPI_COD_MOS_CFG_t structure
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*MOS_Cfg) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_COD_MOS_t const *pMosCfg);

   /** Reads out the current MOS value.
      \param pLLCh           Pointer to Low-level channel structure
      \param pMos            Pointer to the IFX_TAPI_COD_MOS_t structure
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*MOS_Result_Get) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_COD_MOS_t *pMos);

   /** Interface is obsolete, do not use. */
   IFX_int32_t (*AMR_Set) (IFX_TAPI_LL_CH_t *pLLCh,
           IFX_TAPI_COD_AMR_t const *pAMR);

   /** Get the AMR codec specific parameters.
      \param pLLCh           Pointer to Low-level channel structure
      \param pAMR            Pointer to the IFX_TAPI_COD_MOS_t structure
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*AMR_Get) (IFX_TAPI_LL_CH_t *pLLCh,
           IFX_TAPI_COD_AMR_t *pAMR);

   IFX_int32_t (*SRTP_CfgSet) (
         IFX_TAPI_LL_CH_t *pLLCh,
         IFX_TAPI_PKT_SRTP_CFG_t const *pSrtpCfg);
   IFX_int32_t (*SRTP_MKI_Set) (
         IFX_TAPI_LL_CH_t *pLLCh,
         IFX_TAPI_PKT_SRTP_MKI_t const *pMki);
   IFX_int32_t (*SRTP_MKI_CfgSet) (
         IFX_TAPI_LL_CH_t *pLLCh,
         IFX_TAPI_PKT_SRTP_MKI_CFG_t const *pMkiCfg);
   IFX_int32_t (*SRTP_CapGet) (
         IFX_TAPI_LL_DEV_t *pLLDev,
         IFX_TAPI_PKT_SRTP_CAP_GET_t *pCap);
   IFX_int32_t (*SRTP_StatGet) (
         IFX_TAPI_LL_CH_t *pLLCh,
         IFX_TAPI_PKT_SRTP_STATISTICS_GET_t *pStat);
   IFX_int32_t (*SRTP_StatReset) (
         IFX_TAPI_LL_CH_t *pLLCh,
         IFX_TAPI_PKT_SRTP_STATISTICS_RESET_t const *pStatReset);
   IFX_int32_t (*SRTP_PckIdxSet) (
         IFX_TAPI_LL_CH_t *pLLCh,
         IFX_TAPI_PKT_SRTP_PACKET_INDEX_SET_t const *pPckIdxSet);
   IFX_int32_t (*SRTP_PckIdxGet) (
         IFX_TAPI_LL_CH_t *pLLCh,
         IFX_TAPI_PKT_SRTP_PACKET_INDEX_GET_t *pPckIdxGet);
   IFX_int32_t (*SRTP_EventCfg) (
         IFX_TAPI_LL_CH_t *pLLCh,
         IFX_TAPI_EVENT_t *pEvent,
         IFX_uint32_t const value);
} IFX_TAPI_DRV_CTX_COD_t;

/*@}*/ /* CODER_MODULE*/

/** PCM Module */  /* ***************************************************** */
/** \addtogroup PCM_MODULE */
/** Used for PCM services higher layer */
/*@{*/
typedef struct
{
   /** Configure and enable the PCM interface
      \param pLLDev     Handle to low-level device
      \param pCfg       Pointer to the configuration structure
      \return
         IFX_SUCCESS if successful else device specific return code.
    */
   IFX_int32_t (*ifCfg) (
      IFX_TAPI_LL_DEV_t *pLLDev,
      IFX_TAPI_PCM_IF_CFG_t const *pCfg);

   /** Prepare parameters and call the target function to activate PCM module
      \param pLLCh      Handle to low-level channel
      \param nMode      Activation mode
      \param pPcmCfg    Pointer to the PCM configuration structure
      \return
         IFX_SUCCESS if successful else device specific return code.
    */
   IFX_int32_t (*Enable) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_uint32_t nMode,
      IFX_TAPI_PCM_CFG_t *);

   /** Prepare parameters and call the target function to Configure the PCM module
      \param pLLCh      Handle to low-level channel
      \param pPcmCfg    Pointer to the PCM configuration structure
      \return
         IFX_SUCCESS if successful else device specific return code.
    */
   IFX_int32_t (*Cfg) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_PCM_CFG_t const *pPCMConfig);

   /** Sets the LEC configuration on the PCM
      \param pLLCh      Handle to low-level channel
      \param pLecConf   Pointer to the LEC configuration structure
      \return
         IFX_SUCCESS if successful else device specific return code.
    */
   IFX_int32_t (*Lec_Cfg) (
      IFX_TAPI_LL_CH_t *pLLCh,
      TAPI_LEC_DATA_t const *pLecConf);

   /** Sets the PCM interface volume
      \param pLLCh      Handle to low-level channel
      \param pVol       Pointer to the IFX_TAPI_LINE_VOLUME_t structure
      \return
         IFX_SUCCESS if successful else device specific return code.
    */
   IFX_int32_t (*Volume_Set) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_LINE_VOLUME_t const *pVol);

   /** Switches on/off HP filter of decoder path
      \param pLLCh      Handle to low-level channel
      \param bHp        IFX_FALSE to switch HP off, IFX_TRUE to switch HP on
      \return
         IFX_SUCCESS if successful else device specific return code.
    */
   IFX_int32_t (*DEC_HP_Set) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_boolean_t bHp);

   /** Configure and activate the PCM channel with HDLC support
      \param pLLCh      Handle to low-level channel
      \param pHdlcCfg   Pointer to the HDLC channel configuration structure
      \return
         IFX_SUCCESS if successful else device specific return code.
    */
   IFX_int32_t (*HDLC_Cfg) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_PCM_HDLC_CFG_t const *pHdlcCfg);

   /** Configure and activate loop for PCM channels
      \param pLLCh      Handle to low-level channel
      \param pLoopCfg   Pointer to the PCM Loop configuration structure
      \return
         IFX_SUCCESS if successful else device specific return code.
    */
   IFX_int32_t (*Loop) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_PCM_LOOP_CFG_t const *pLoopCfg);

   /** Muting for PCM channels
   \param pLLCh           Pointer to Low-level channel structure
   \param pMuteCfg        Pointer to the PCM Mute structure
   \return
   IFX_SUCCESS if successful
   IFX_ERROR if an error occured */
   IFX_int32_t (*Mute) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_PCM_MUTE_CFG_t const *pMuteCfg);

} IFX_TAPI_DRV_CTX_PCM_t;
/*@}*/ /* PCM_MODULE*/


/** SIG Module */ /* ********************************************************/
/** \addtogroup SIG_MODULE */
/** Signalling module services*/
/*@{*/
typedef struct
{
   /** Do low level UTG configuration and activation
      \param pLLCh         Handle to low-level channel
      \param pSimpleTone   Internal simple tone table entry
      \param dst           Destination
      \param nResID        Low level resource ID queried before by
                           ToneGen_ResIdGet. Otherwise TAPI defaults
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*UTG_Start) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_TONE_SIMPLE_t const *pSimpleTone,
      TAPI_TONE_DST dst,
      IFX_uint8_t nResID);

   IFX_int32_t (*SUTG) (void);

   /** Stop playing the tone with the given tone definition
      \param pLLCh      Handle to low-level channel
      \param nResID     Low level resource ID queried before by
                        ToneGen_ResIdGet. Otherwise TAPI defaults
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*UTG_Stop) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_uint8_t nResID);

   /** Sets the tone level of tone currently played
      \param pLLCh      Handle to low-level channel
      \param pToneLevel Struct with the tone level specification.
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*UTG_Level_Set) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_PREDEF_TONE_LEVEL_t const *pToneLevel);

   /** Returns the total number of UTGs per channel
      \param pLLCh      Handle to low-level channel
      \return
         Returns the total number of UTGs per channel
   */
   IFX_uint8_t  (*UTG_Count_Get) (IFX_TAPI_LL_CH_t *pLLCh);

   /** Called when tone is deactivated
      \param pLLCh      Handle to low-level channel
      \param utgNum     UTG resource number.
   */
   IFX_void_t   (*UTG_Event_Deactivated) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_uint8_t utgNum);

   /** Do low level UTG configuration and activation
       Allocate a tone generation resource on a specific module "modType".

      \param pLLCh      Handle to low-level channel
      \param nResID     The resouce identifier returned
      \param genType    Describes the module type where to play the tone.
      \param genDir     Tone generation direction on the module.
      \return
         IFX_SUCCESS if successful else device specific return code.

      \remarks
         nResID is returned and stored inside LL driver if necessary.
         It is used during tone event report to TAPI HL. TAPI HL uses nResID
         as an index for event dispatch functionality.
         This index is used for IFX_TAPI_LL_ToneStart, IFX_TAPI_LL_ToneStop
         and IFX_TAPI_LL_ToneGenResRelease.
         This API only supports to allocate a resource with genDir set to
         IFX_TAPI_LL_TONE_EXTERNAL or IFX_TAPI_LL_TONE_INTERNAL. It does not
         support to allocate a resource for both directions
         IFX_TAPI_LL_TONE_BOTH.
         TAPI LL disables all tone generator events in case the return value
         is not success.
   */
   IFX_int32_t (*ToneGen_ResIdGet) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_MODULE_TYPE_t genType,
      IFX_TAPI_LL_TONE_DIR_t genDir,
      IFX_TAPI_TONE_RES_t *pRes);

   /** Request Details about the last RFC2833 event packet received for
       playout.
      \param pLLCh      Handle to low-level channel
      \param nEvent     Pointer where to store the event code
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*EPOU_Trig_Evt_Detail_Req) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_uint16_t *nEvent);

   /** Configure the DTMF tone generator
      \param pLLCh            Handle to low-level channel
      \param nInterDigitTime  Inter-digit-time in ms
      \param nDigitPlayTime   Active digit-play-time in ms
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*DTMFG_Cfg) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_uint16_t nInterDigitTime,
      IFX_uint16_t nDigitPlayTime);

   /** Start the DTMF tone generator
      \param pLLCh      Handle to low-level channel
      \param nDigits    Number of digits in the data string to be sent
      \param *data      String with the digits (ascii 0-9 A-D) to be sent
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*DTMFG_Start) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_uint8_t nDigits,
      IFX_char_t const *data);

   /** Stop the DTMF tone generator
      \param pLLCh   Handle to low-level channel
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*DTMFG_Stop) (IFX_TAPI_LL_CH_t *pLLCh);

   /** Start the DTMF tone detector
      \param pLLCh      Handle to low-level channel
      \param pCfg       Pointer to the DTMF detector configuration
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*DTMFD_Enable) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_LL_DTMFD_CFG_t const *pCfg);

   /** Stop the DTMF tone detector
      \param pLLCh      Handle to low-level channel
      \param pCfg       Pointer to the DTMF detector configuration
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*DTMFD_Disable) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_LL_DTMFD_CFG_t const *pCfg);

   /** Controls the DTMF sending mode.
      \param pLLCh      Handle to low-level channel
      \param nOobMode   Mode of DTMFD (Inband or Out of band transmission
                        of RFC2833 event packets)
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*DTMFD_OOB) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_PKT_EV_OOB_t nOobMode);

   /** Override control of the DTMF tone detector
      \param pLLCh   Handle to low-level channel
      \param pCfg    Pointer to the DTMF detector configuration
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*DTMFD_Override) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_LL_DTMFD_OVERRIDE_t const *pCfg);

   /** Sets/Gets DTMF receiver coefficients.
      \param pLLCh         Handle to low-level channel
      \param bRW           IFX_FALSE to write, IFX_TRUE to read settings
      \param pDtmfRxCoeff  Pointer to DTMF Rx coefficients settings
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*DTMF_RxCoeff) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_boolean_t bRW,
      IFX_TAPI_DTMF_RX_CFG_t *pDtmfRxCoeff);

   /** Starts Call Progress Tone Detection
      \param pLLCh      Handle to low-level channel
      \param pTone      Pointer to simple tone structure
      \param pSignal    Pointer to the signal description
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*CPTD_Start) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_TONE_SIMPLE_t const *pTone,
      IFX_TAPI_TONE_CPTD_t const *pSignal);

   /** Stops the Call Progress Tone Detection
      \param pLLCh      Handle to low-level channel
      \param pSignal    Pointer to the signal description
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*CPTD_Stop) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_TONE_CPTD_t const *pSignal);

   /** Enables signal detection
      \param pLLCh   Handle to low-level channel
      \param pSig    Pointer to IFX_TAPI_SIG_DETECTION_t structure
      \param nMod    Module type
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*MFTD_Enable) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_SIG_DETECTION_t const *pSig);

   /** Disables signal detection
      \param pLLCh   Handle to low-level channel
      \param pSig    Pointer to IFX_TAPI_SIG_DETECTION_t structure
      \param nMod    Module type
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*MFTD_Disable) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_SIG_DETECTION_t const *pSig);

   /** Parse detected event
      \param pLLCh         Handle to low-level channel
      \param nCh           Signalling channel number
      \param pTapiEvent    Pointer to IFX_TAPI_EVENT_t structure to store event
      \param nVal          Detected MFTD tone value
      \param bRx           Direction
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*MFTD_Event) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_uint8_t nCh,
      IFX_TAPI_EVENT_t *pTapiEvent,
      IFX_uint8_t nVal, IFX_boolean_t bRx);

   /** Configures the transmission of tone signals detected by MFTD. Tones can
       be transmitted inband or out-of-band with RFC2833 RTP event packets.
      \param pLLCh      Handle to low-level channel
      \param nOobMode   Mode of signal transmission. (Inband or Out-of-band
                        transmission with RFC2833 event packets)
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*MFTD_OOB) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_PKT_EV_OOB_t nOobMode);

   /** Start CID data transmission
      \param pLLCh      Handle to low-level channel
      \param pCidData   Pointer to the CID transmition configuration and data
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*CID_TX_Start) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_CID_TX_t const *pCidData);

   /** Stop CID data transmission
      \param pLLCh      Handle to low-level channel
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*CID_TX_Stop) (IFX_TAPI_LL_CH_t *pLLCh);

   /** Start the CID receiver
      \param pLLCh   Handle to low-level channel
      \param pCid    Contains the CID configuration and data to be sent
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*CID_RX_Start) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_CID_RX_t const *pCidData);

   /** Stop the CID receiver
      \param pLLCh   Handle to low-level channel
      \param pCid    CID stop configuration
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*CID_RX_Stop) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_CID_RX_CFG_t const *pCid);

   /** Start the internal CID State Machine
      \param pLLCh      Handle to low-level channel
      \param pCidData   Pointer to the CID transmission configuration and data
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*CIDSM_Start) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_CID_SEQ_CONF_t const *pCidData);

   /** Stop the internal CID State Machine
      \param pLLCh   Handle to low-level channel
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*CIDSM_Stop) (IFX_TAPI_LL_CH_t *pLLCh);

   /** Start the tone level peak detector
      \param pLLCh      Handle to low-level channel
      \param pPeakdCfg  Handle to Tone Level Peak Detector Configuration
                        structure
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*PEAKD_Start) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_PEAK_DETECT_CFG_t const *pPeakdCfg);

   /** Stop the tone level peak detector
      \param pLLCh      Handle to low-level channel
      \param pPeakdCfg  Handle to Tone Level Peak Detector Configuration
                        structure
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*PEAKD_Stop) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_PEAK_DETECT_CFG_t const *pPeakdCfg);

   /** Reads out the maximum value of the peak detector
      \param pLLCh         Handle to low-level channel
      \param pPeakdResult  Handle to data structure to retrieve the
                           measured value of the peak detector
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*PEAKD_Result) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_PEAK_DETECT_RESULT_GET_t *pPeakdResult);

   /** Start the MF R2 tone detector
      \param pLLCh         Handle to low-level channel
      \param pMF_R2_Cfg    Handle to MF R2 tone Detector Configuration structure
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*MF_R2_Start) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_TONE_MF_R2_CFG_t const *pMF_R2_Cfg);

   /** Stop the MF R2 tone detector
      \param pLLCh      Handle to low-level channel
      \param pPeakdCfg  Handle to MF R2 tone Detector Configuration structure
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*MF_R2_Stop) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_TONE_MF_R2_CFG_t const *pMF_R2_Cfg);

} IFX_TAPI_DRV_CTX_SIG_t;

/** ALM Module */ /* ********************************************************/
/** \addtogroup ALM_MODULE */
/** Analog line module services*/
/*@{*/
typedef struct
{
   /** Set line type and sampling operation mode of the analog line.
      \param pLLCh   Handle to low-level channel
      \param nType   Line type and sampling mode to be set
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*Line_Type_Set) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_LINE_TYPE_t nType);

   /** Set the line mode of the analog line
      \param pLLCh            Handle to low-level channel
      \param nMode
      \param nTapiLineMode
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*Line_Mode_Set) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_int32_t nMode,
      IFX_uint8_t nTapiLineMode);

   /** Read back the line mode of the analog line
      \param pLLCh            Handle to low-level channel
      \param pMode
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*Line_Mode_Get) (
      IFX_TAPI_LL_CH_t *pLLCh, IFX_TAPI_LINE_FEED_t *pMode);

   /** Switch Line Polarity
      \param pLLCh   Handle to low-level channel
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*Line_Polarity_Set) (IFX_TAPI_LL_CH_t *pLLCh);

   /** Set the phone volume
      \param pLLCh      Handle to low-level channel
      \param pVol       Pointer to IFX_TAPI_LINE_VOLUME_t structure
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*Volume_Set) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_LINE_VOLUME_t const *pVol);

   /** This service enables or disables a high level path of a phone channel.
      \param pLLCh      Handle to low-level channel
      \param bEnable    Enable or disable
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*Volume_High_Level) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_int32_t bEnable);

   /** Ring configuration set
      \param pLLCh         Handle to low-level channel
      \param pRingConfig   Pointer to ring config structure.
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*Ring_Cfg_Set) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_RING_CFG_t const *pRingConfig);

   /** Ring configuration get
      \param pLLCh         Handle to low-level channel
      \param pRingConfig   Pointer to ring config structure which is to be
                           filled with data.
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*Ring_Cfg_Get) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_RING_CFG_t *pRingConfig);

   /** Enable/Disalbe Auto battery switch
      \param pLLCh         Handle to low-level channel
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*AutoBatterySwitch) (IFX_TAPI_LL_CH_t *pLLCh);

   /** Configure metering mode of chip
      \param pLLCh         Handle to low-level channel
      \param nMode         TTX (0) or reverse polarity (1)
      \param nPulseLen     Duration of pulse
      \param nPauseLen     Duration of pause
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*Metering_Cfg) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_METER_CFG_t *pCfg);

   /** Send metering burst
      \param pLLCh      Handle to low-level channel
      \param nPulseNum  Number of pulses
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*Metering_Start) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_uint32_t nPulseNum);

   /** Restores the line state back after fault
      \param pLLCh      Handle to low-level channel
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*FaultLine_Restore) (IFX_TAPI_LL_CH_t *pLLCh);

   /** Sets the LEC configuration on the ALM
      \param pLLCh      Handle to low-level channel
      \param pLecConf   Pointer to LEC configuration structure
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*Lec_Cfg) (
      IFX_TAPI_LL_CH_t *pLLCh,
      TAPI_LEC_DATA_t *pLecConf);

   /** Starts playing out a tone on the ALM tone generator.
      \param pLLCh         Handle to low-level channel
      \param res           Resource number used for playing the tone.
      \param pToneSimple   Pointer to the tone definition to play
      \param dst           Destination where to play the tone
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*TG_Play) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_uint8_t res,
      IFX_TAPI_TONE_SIMPLE_t const *pToneSimple,
      TAPI_TONE_DST dst);

   /** Stop playing the tone with the given tone definition
      \param pLLCh   Handle to low-level channel
      \param res     Resource number used for playing the tone
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*TG_Stop) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_uint8_t res);

   /** Starts playing out the next tone of the simple tone definition.
      \param pLLCh      Handle to low-level channel
      \param pTone      Pointer to the current simple tone definition
      \param res        Resource number used for playing the tone
      \param nToneStep  Identifies the next tone step of the simple tone
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*TG_ToneStep) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_TONE_SIMPLE_t const *pTone,
      IFX_uint8_t res, IFX_uint8_t *nToneStep);

   /** Gets the activation status of the message waiting lamp.
      \param pLLCh         Handle to low-level channel
      \param pActivation   Handle to \ref IFX_TAPI_MWL_ACTIVATION_t structure
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*MWL_Activation_Get) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_MWL_ACTIVATION_t *pActivation);

   /** Activate/deactivates the message waiting lamp.
      \param pLLCh         Handle to low-level channel
      \param pActivation   Handle to \ref IFX_TAPI_MWL_ACTIVATION_t structure
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*MWL_Activation_Set) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_MWL_ACTIVATION_t const *pActivation);

   /** Simulate Hook generation (for debug use only)
      \param pLLCh   Handle to low-level channel
      \param bHook   Hook state
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*TestHookGen) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_boolean_t bHook);

   /** ALM 8kHz test loop switch (for debug use only)
      \param pLLCh   Handle to low-level channel
      \param pLoop    Handle to \ref IFX_TAPI_TEST_LOOP_t structure
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*TestLoop) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_TEST_LOOP_t const *pLoop);

   /** Configure the hook validation timers.

      This function maybe called multiple times with different values.
      If the device is not yet initialized with IFX_TAPI_CH_INIT, no message is
      sent. The data is only cached in a firmare message strucutre.

      After IFX_TAPI_CH_INIT the message CMD_ALI_HOOK_DSC is always send
      to the device, and always disables and enables the hook statemachine.
      This behaviour could lead to problems in detecting hooks, digits or
      flash hook. Therefore it is recommended to call IFX_TAPI_LINE_HOOK_VT_SET
      before issuing an channel init.

      \param pLLCh      Handle to low-level channel
      \param pHookVt    Handle to IFX_TAPI_LINE_HOOK_VT_t structure array
                        containing the timing of HookOffTime, HookOnTime,
                        HookFlashTime, HookFlashMakeTime, DigitLowTime,
                        DigitHighTime and InterDigitTime in that order.
      \param len        Length of the array in elements of
                        IFX_TAPI_LINE_HOOK_VT_t. If the length is 1,
                        the type must be evaluated with the field nType.
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*HookVt_Set) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_LINE_HOOK_VT_t const *pHookVt,
      IFX_uint8_t len);

   /** Start calibration process for analog channel
      \param pLLCh   Handle to low-level channel
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*Calibration_Start) (IFX_TAPI_LL_CH_t *pLLCh);

   /** Stop calibration process for analog channel
      \param pLLCh   Handle to low-level channel
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*Calibration_Stop) (IFX_TAPI_LL_CH_t *pLLCh);

   /** Finish the calibration process on an analog channel
      \param pLLCh   Handle to low-level channel
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*Calibration_Finish) (IFX_TAPI_LL_CH_t *pLLCh);

   /** Read out the current calibration values of the analog channel
      \param pLLCh         Handle to low-level channel
      \param pClbConfig    Handle to \ref IFX_TAPI_CALIBRATION_CFG_t structure
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*Calibration_Get) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_CALIBRATION_CFG_t *pClbConfig);

   /** Writes the calibration values of the analog channel
      \param pLLCh         Handle to low-level channel
      \param pClbConfig    Handle to \ref IFX_TAPI_CALIBRATION_CFG_t structure
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*Calibration_Set) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_CALIBRATION_CFG_t const *pClbConfig);

   /** Read out the calibration results of the analog channel
      \param pLLCh         Handle to low-level channel
      \param pClbConfig    Handle to \ref IFX_TAPI_CALIBRATION_CFG_t structure
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*Calibration_Results_Get) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_CALIBRATION_CFG_t *pClbConfig);

   /** Start the COMTEL signal
      \param pLLCh         Handle to low-level channel
      \param pComTelConf   Handle to \ref IFX_TAPI_COMTEL_t structure
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*ComTel_Start) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_COMTEL_t const *pComTelConf);

   /** Start selected subset (or all) GR909 tests
      \param pLLCh         Handle to low-level channel
      \param pGR909Start   Handle to \ref IFX_TAPI_GR909_START_t structure
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*GR909_Start) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_GR909_START_t const *pGR909Start);

   /** Stop GR909 tests
      \param pLLCh   Handle to low-level channel
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*GR909_Stop) (IFX_TAPI_LL_CH_t *pLLCh);

   /** Read GR909 results
      \param pLLCh         Handle to low-level channel
      \param pGR909Result  Handle to \ref IFX_TAPI_GR909_RESULT_t structure
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*GR909_Result) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_GR909_RESULT_t *pGR909Result);

   /** Read out the analog line battery voltage configuration.
      \param pLLCh   Handle to low-level channel
      \param pVBat   Handle to the memory for result
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*FeedingVoltageGet) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_int16_t* pVbat);

   /** Write the analog line battery voltage configuration.
      \param pLLCh   Handle to low-level channel
      \param nVBat   Battery voltage value
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*FeedingVoltageSet) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_int16_t nVbat);

   /** Raw event detection control
      \param pLLCh      Handle to low-level channel
      \param nAction    Enable if IFX_TRUE else disable raw event detection
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*RawHookCtrl) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_boolean_t nAction);

   /** Retrieves the relative gain configuration.
      \param pLLCh      Handle to low-level channel
      \param pTapiGain  Handle to \ref IFX_TAPI_GAIN_t structure
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*TxRxGain_Get) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_GAIN_t *pTapiGain);

   /** Sets the relative gain of the analog line module.
      \param pLLCh      Handle to low-level channel
      \param pTapiGain  Handle to \ref IFX_TAPI_GAIN_t structure
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*TxRxGain_Set) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_GAIN_t const *pTapiGain);

   /** Retrieve the version string of the currently version.
      \param pLLCh            Handle to low-level channel
      \param pVersionChEntry  Handle to \ref IFX_TAPI_VERSION_CH_ENTRY_t
                              structure
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*VersionChEntry_Get) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_VERSION_CH_ENTRY_t *pVersionChEntry);

   /** Request continuous measurement results
      \param pLLCh   Handle to low-level channel
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*ContMeasReq)  (IFX_TAPI_LL_CH_t *pLLCh);

   /** Reset continous measurement results
      \param pLLCh   Handle to low-level channel
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*ContMeasReset)(IFX_TAPI_LL_CH_t *pLLCh);

   /** Return the stored continuous measurement results
      \param pLLCh      Handle to low-level channel
      \param pContMeas  Handle to \ref IFX_TAPI_CONTMEASUREMENT_GET_t structure
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*ContMeasGet) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_CONTMEASUREMENT_GET_t *pContMeas);

   /** Set the new hook state on FXO line
      \param pLLCh   Handle to low-level channel
      \param hook    hook state
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*FXO_HookSet) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_FXO_HOOK_t hook);

   /** Get current hook state for FXO line
      \param pLLCh   Handle to low-level channel
      \param pHook   Handle to hook state result storage
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*FXO_HookGet) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_FXO_HOOK_t *pHook);

   /** Issue a hook flash on FXO line
      \param pLLCh   Handle to low-level channel
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*FXO_FlashSet) (IFX_TAPI_LL_CH_t *pLLCh);

   /** Configure a hook flash time
      \param pLLCh      Handle to low-level channel
      \param p_fhCfg    Handle to \ref IFX_TAPI_FXO_FLASH_CFG_t structure
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*FXO_FlashCfg) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_FXO_FLASH_CFG_t const *p_fhCfg);

   /** Configure maximum detection time for OSI
      \param pLLCh      Handle to low-level channel
      \param p_osiCfg   Handle to \ref IFX_TAPI_FXO_OSI_CFG_t structure
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*FXO_OsiCfg) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_FXO_OSI_CFG_t const *p_osiCfg);

   /** Get FXO event status from LL driver
      \param pLLCh      Handle to low-level channel
      \param bit_pos    Can be one of FXO_BATTERY, FXO_APOH, FXO_POLARITY,
                        FXO_RING.
      \param enable     Pointer to IFX_enDis_t; output value.
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*FXO_StatGet) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_uint32_t bit_pos,
      IFX_enDis_t *enable);

   /** Set FXO line mode
      \param pLLCh   Handle to low-level channel
      \param pMode   Handle to \ref IFX_TAPI_FXO_LINE_MODE_t structure
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*FXO_LineModeSet) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_FXO_LINE_MODE_t const *pMode);

   /** Start an analog line capacitance measurement session
      \param pLLCh      Handle to low-level channel
      \param bTip2RingOnly   Measure only tip to ring capacitance.
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*CapMeasStart) (IFX_TAPI_LL_CH_t *pLLCh,
                                IFX_boolean_t bTip2RingOnly);

   /* obsolete / reserved */
   IFX_int32_t (*CapMeasStartInt) (IFX_TAPI_LL_CH_t *pLLCh);

   /** Stop any running analog line capacitance measurement session
      \param pLLCh      Handle to low-level channel
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*CapMeasStop) (IFX_TAPI_LL_CH_t *pLLCh);

   /** Get capacitance measurement result
      \param pLLCh      Handle to low-level channel
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*CapMeasResult) (IFX_TAPI_LL_CH_t *pLLCh);

   /** Check cpacitance measurement support.
      \param pLLCh       Handle to low-level channel
      \param pSupported  Returns information whether capacitance measurement
                         is supported (IFX_TRUE) or not (IFX_FALSE).
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*CheckCapMeasSup) (IFX_TAPI_LL_CH_t *pLLCh,
                                   IFX_boolean_t *pSupported);

   /**Configures the open loop calibration factors of the measurement path
       for line testing
      \param pLLCh   Handle to low-level channel
      \param pArg    Pointer to IFX_TAPI_NLT_CONFIGURATION_OL_t structure
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*NLT_OLConfigSet) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_NLT_CONFIGURATION_OL_t *pArg);

   /**Gets the open loop calibration factors of the measurement path
       for line testing
      \param pLLCh   Handle to low-level channel
      \param pArg    Pointer to IFX_TAPI_NLT_CONFIGURATION_OL_t structure
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*NLT_OLConfigGet) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_NLT_CONFIGURATION_OL_t *pArg);

   /**Configures the measurement path for line testing according to
      Rmes resitor .
      \param pLLCh   Handle to low-level channel
      \param pArg    Pointer to IFX_TAPI_NLT_CONFIGURATION_RMES_t structure
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*NLT_RmesConfigSet) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_NLT_CONFIGURATION_RMES_t *pArg);

   /** Reads the results of the capacitance measurement.
      \param pLLCh    Handle to low-level channel
      \param pResult  Pointer to IFX_TAPI_NLT_CAPACITANCE_RESULT_t structure
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*NLT_capacitance_result_get) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_NLT_CAPACITANCE_RESULT_t *pResult);

   /** Override control of the DUP timer configuration
      \param pLLCh   Handle to low-level channel
      \param nStandbyTime DUP_OVERRIDE_OFF to clear override or time in ms
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*CfgDupTimer_Override) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_uint8_t nStandbyTime);

   /** Bring the SLIC back into service after a failure.
      \param  pLLDev    Pointer to low-level device.
      \return
      IFX_SUCCESS if successful else device specific return code. */
   IFX_int32_t (*SlicRecovery) (
                        IFX_TAPI_LL_DEV_t *pDev);

   /** Debugging support. Handle an SSI-crash event. Retrieve the cause code
       for debugging purposes.
      \param pLLDev     Handle to low-level device
      \param pData      Pointer to struct IFX_TAPI_DBG_SSI_CRASH where details
                        will be returned on success.
      \return
      IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*Dbg_SSICrash_Handler) (
      IFX_TAPI_LL_DEV_t *pLLDev,
      struct IFX_TAPI_DBG_SSI_CRASH *pData);
} IFX_TAPI_DRV_CTX_ALM_t;
/*@}*/

/** DECT Module */  /* **************************************************** */
/** \addtogroup DECT_MODULE */
/** Used for DECT services higher layer */
/*@{*/
typedef struct
{
   /** Sets the encoder and decoder start delay
      \param pLLCh      Handle to low-level channel
      \param nEncDelay  Delay from the start of the decoder to the start of
                        the encoder in steps of 2.5ms. Range 0ms - 10ms.
      \param nDecDelay  Delay from the arrival of the first packet to the start
                        of the decoder in steps of 2.5ms. Range 0ms - 10ms.
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*Ch_Cfg) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_uint8_t nEncDelay,
      IFX_uint8_t nDecDelay);

   /** Sets the coder type and frame length for the DECT encoding path
      \param pLLCh      Handle to low-level channel
      \param nCoder     Selected coder type
      \param nFrameLength  length of packets to be generated by the coder
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*ENC_Cfg) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_DECT_ENC_TYPE_t nCoder,
      IFX_TAPI_DECT_ENC_LENGTH_t nFrameLength);

   /** Prepare parameters and call the target function to activate DECT module
      \param pLLCh      Handle to low-level channel
      \param nEnable    Enable or disable the module
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*Enable) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_uint8_t nEnable);

   /** Sets the gains for the DECT en-/decoding path.
      \param pLLCh      Handle to low-level channel
      \param pVol       Pointer to IFX_TAPI_LINE_VOLUME_t structure
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*Gain_Set) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_PKT_VOLUME_t const *pVol);

   /** Get the statistic data from the DECT coder channel
      \param pLLCh      Handle to low-level channel
      \param pStatistic pointer to struct where to store the statistic data
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*Statistic) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_DECT_STATISTICS_t *pStatistic);

   /** Do low level UTG configuration and activation
      \param pLLCh         Handle to low-level channel
      \param pSimpleTone   Internal simple tone table entry
      \param dst           Destination (unused)
      \param utgNum        UTG number  (always 0)
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*UTG_Start) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_TONE_SIMPLE_t const *pSimpleTone,
      TAPI_TONE_DST dst,
      IFX_uint8_t res);

   /** Stop playing the tone with the given tone definition
      \param pLLCh      Handle to low-level channel
      \param res        Resource number used for playing the tone (always 0)
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*UTG_Stop) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_uint8_t res);

   /** Do low level DECT echo canceller configuration
      \param pLLCh      Handle to low-level channel
      \param pEC_cfg    Pointer to \ref IFX_TAPI_DECT_EC_CFG_t structure
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*EC_Cfg) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_DECT_EC_CFG_t const *pEC_cfg);
} IFX_TAPI_DRV_CTX_DECT_t;
/*@}*/ /* DECT_MODULE*/


/** CONnection Module */
/** \addtogroup CON_MODULE */
/** Connection Module services*/
/*@{*/
typedef struct
{
   /** Adds a connection between a data channel and any of these modules:
       PCM, ALM, DECT.
      \param pLLCh      Handle to low-level channel
      \param pMap       Pointer to IFX_TAPI_MAP_DATA_t structure
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*Data_Channel_Add) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_MAP_DATA_t const *pMap);

   /** Removes a connection between a data channel and any of these modules:
       PCM, ALM, DECT.
      \param pLLCh      Handle to low-level channel
      \param pMap       Pointer to IFX_TAPI_MAP_DATA_t structure
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*Data_Channel_Remove) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_MAP_DATA_t const *pMap);

   /** Adds a connection between any two of the following modules:
       PCM, ALM, DECT.
      \param pLLCh      Handle to low-level channel
      \param nType1     Module type in the first channel
      \param nCh2       Channel number of second channel
      \param nType2     Module type in the second channel
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*Module_Connect) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_MAP_TYPE_t nType1,
      IFX_uint8_t nCh2,
      IFX_TAPI_MAP_TYPE_t nType2);

   /** Removes a connection between any two of the following modules:
       PCM, ALM, DECT.
      \param pLLCh      Handle to low-level channel
      \param nType1     Module type in the first channel
      \param nCh2       Channel number of second channel
      \param nType2     Module type in the second channel
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*Module_Disconnect) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_MAP_TYPE_t nType1,
      IFX_uint8_t nCh2,
      IFX_TAPI_MAP_TYPE_t nType2);

   /** Mute/Unmute all connections to modules which are attached to the given
      data channel except the first module connected to the local side.
      \param pLLCh      Handle to low-level channel
      \param nMute      IFX_TRUE: mute / IFX_FALSE: unmute
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*Data_Channel_Mute) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_boolean_t nMute);

   /** Adds a monitoring connection between any modules.
      \param pLLCh      Handle to low-level channel
      \param pMap       Pointer to IFX_TAPI_MAP_MON_CFG_t structure
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*Mon_Channel_Add) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_MAP_MON_CFG_t const *pMap);

   /** Removes a connection between a PCM channel and another PCM channel or
       an analog phone.
      \param pLLCh      Handle to low-level channel
      \param pMap       Pointer to IFX_TAPI_MAP_MON_CFG_t structure
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*Mon_Channel_Remove) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_MAP_MON_CFG_t const *pMap);

   /** Find the module which gets the input from the data channel and provides
       it's input to the data channel.
      \param pLLCh      Handle to low-level channel
      \param pTapiCh    Returns pointer to the TAPI channel the found module
                        belongs to or IFX_NULL if no module is connected.
      \param pModType   Returns of which type the found module is.
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*Data_Channel_Find_Connected_Module) (
      IFX_TAPI_LL_CH_t *pLLCh,
      TAPI_CHANNEL **pTapiCh,
      IFX_TAPI_MAP_TYPE_t *pModType);

   /** Find the data channel which gets it's main input from a given module
       and which provides it's input to the given module.

       There can be more than one data channel connected to a module.

      \param pLLCh      Handle to low-level channel
      \param pModType   Returns of which type the found module is.
      \param pTapiCh    On input specifies which was the last channel found
                        by this function. For the first call use IFX_NULL.
                        For iteration calls use the last output.
                        Returns pointer to the TAPI channel the found module
                        belongs to or IFX_NULL if no module is connected.
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*Module_Find_Connected_Data_Channel) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_MAP_TYPE_t pModType,
      TAPI_CHANNEL **pTapiCh);

   /** Mute module incoming connections exclude signaling channel.
      \param pLLCh       Handle to IFX_TAPI_LL_CH_t structure
      \param nMod        Module type to mute
      \param nMute       IFX_TRUE: mute / IFX_FALSE: unmute
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*Channel_Mute) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_MODULE_TYPE_t nMod,
      IFX_boolean_t nMute);

} IFX_TAPI_DRV_CTX_CON_t;
/*@}*/

/** Polling Interface */ /* ****************************************************/
/** \addtogroup POLL_INTERFACE */
/** List of required low-level polling services */
/*@{*/
typedef struct
{
   /** Read voice/fax/event packets from the device
      \param pLLDev     Handle to low-level device
      \param ppPkts     Array of free buffer pointers
      \param pPktsNum   On entry identifies the number of packets to be read,
                        on return it contains the number of packets read
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*rdPkts) (
      IFX_TAPI_LL_DEV_t *pLLDev,
      IFX_void_t **ppPkts,
      IFX_int32_t *pPktsNum,
      IFX_int32_t nDevID);

   /** Write voice/fax/event packets available
      \param pLLDev     Handle to low-level device
      \param ppPkts     Array of packet buffer pointers to be written
      \param pPktsNum   On entry identifies the number of packets to be written,
                        on return it contains the number of packets successfully
                        written. On success (IFX_SUCCESS) all packets have been
                        successfully written.
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*wrPkts) (
      IFX_TAPI_LL_DEV_t *pLLDev,
      IFX_int32_t *pPktsNum);

   /** Updates the low-level TAPI device status by reading the hardware status
       registers and taking the appropriate actions upon status change.
       Typically this function executes the device's ISR.

      \param pLLDev     Handle to low-level device
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*pollEvents) (IFX_TAPI_LL_DEV_t *pLLDev);

   /** Used to control the packet-generation related interrupts. In case a
       device is registerred for packet polling it is necessary to disable
       the related interrupts so as to prohibit any unwanted overhead of
       switching to interrupt context.

      \param pLLDev     Handle to low-level device
      \param bEnable    IFX_TRUE to enable, IFX_FALSE to disable the
                        related interrupts
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*pktsIRQCtrl) (
      IFX_TAPI_LL_DEV_t *pLLDev,
      IFX_boolean_t bEnable);

   /** Used to control the TAPI event-generation related interrupts. In case a
       device is registerred for events polling it is necessary to disable
       the related interrupts so as to prohibit any unwanted overhead of
       switching to interrupt context.

      \param pLLDev     Handle to low-level device
      \param bEnable    IFX_TRUE to enable, IFX_FALSE to disable the
                        related interrupts
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*evtsIRQCtrl) (
      IFX_TAPI_LL_DEV_t *pLLDev,
      IFX_boolean_t bEnable);

} IFX_TAPI_DRV_CTX_POLL_t;
/*@}*/

/** Network Linetesting (NLT) Interface */ /* *********************************/
/** \addtogroup NLT_INTERFACE */
/** List of required low-level NLT services */
/*@{*/
typedef struct
{
   /** Starts the specified NLT test on the specified channel.
      \param pLLCh   Handle to low-level channel
      \param pArg            Pointer to IFX_TAPI_NLT_TEST_START_t structure
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*NLT_test_start) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_NLT_TEST_START_t *pArg);

   /** Gets results of the specified NLT test on the specified channel.
      \param pLLCh   Handle to low-level channel
      \param pArg    Pointer to IFX_TAPI_NLT_RESULT_GET_t structure
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*NLT_result_get) (
      IFX_TAPI_LL_CH_t *pLLCh,
      IFX_TAPI_NLT_RESULT_GET_t *pArg);

} IFX_TAPI_DRV_CTX_NLT_t;
/*@}*/

/* driver context data structure */
/** \addtogroup TAPI_LL_INTERFACE */
/** Interface between High-Level TAPI and Low-Level TAPI */
/*@{*/
typedef struct
{
   /** high-level and low-level interface API version, keep as first element */
   IFX_char_t                              *hlLLInterfaceVersion;
   /** device nodes prefix (if DEVFS is used) /dev/\<devNodeName\>\<number\> */
   IFX_char_t                              *devNodeName;
   /** driverName */
   IFX_char_t                              *drvName;
   /** driverVersion */
   IFX_char_t                              *drvVersion;

   IFX_uint16_t                             majorNumber;
   IFX_uint16_t                             minorBase;
   IFX_uint16_t                             maxDevs;
   IFX_uint16_t                             maxChannels;

   /** To get events from multiple devices in round robin manner remember
       which device to service next. */
   IFX_uint16_t                             nLastEventDevice;

   /* The following two prepare functions receive a pointer to the
      HL device and channel structures and return pointer to the
      LL device and channel structures(!)
   */

   /** Prepare the low-level device struct.
      \param pTapiDev   Pointer to the high-level device struct.
      \param devNum     Device number.
      \return
         Pointer to low-level device struct.
   */
   IFX_TAPI_LL_DEV_t* (*Prepare_Dev) (
      TAPI_DEV *pTapiDev,
      IFX_uint32_t devNum);

   /** Initialise the low-level device struct.
      \param pDev       Pointer to low-level device struct.
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*Init_Dev) (IFX_TAPI_LL_DEV_t *pDev);

   /** Stop the low-level device and free all allocated resources.
      \param pDev       Pointer to low-level device struct.
      \param bChipAccess Allow or deny chip access in this function.
   */
   IFX_void_t (*Exit_Dev) (
      IFX_TAPI_LL_DEV_t *pDev,
      IFX_boolean_t bChipAccess);

   /** Prepare the low-level channel struct.
      \param pTapiCh    Pointer to high-level channel struct.
      \param pLLDev     Pointer to low-level device struct.
      \param chNum        Channel number.
      \return
         Pointer to low-level channel struct.
   */
   IFX_TAPI_LL_CH_t* (*Prepare_Ch) (
      TAPI_CHANNEL *pTapiCh,
      IFX_TAPI_LL_DEV_t *pLLDev,
      IFX_uint32_t chNum);

   /** Initialise the low level channel struct.
      \param pCh        Pointer to low-level channel struct.
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*Init_Ch) (IFX_TAPI_LL_CH_t *pCh);

   /** Start the firmware
      \param pLLDev     Pointer to low-level device struct.
      \param pProc      Pointer to low-level device initialization structure.
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*FW_Start) (
      IFX_TAPI_LL_DEV_t *pLLDev,
      IFX_void_t const *pProc);

   /** Initialise the firmware
      \param pLLDev     Pointer to low-level device struct.
      \param nMode      Enum from IFX_TAPI_INIT_MODE_t specifying the setup.
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*FW_Init) (
      IFX_TAPI_LL_DEV_t *pLLDev,
      IFX_uint8_t nMode);

   /** Download a BBD file.
      \param pCh        Pointer to low-level channel/device struct.
      \param pProc      Pointer to low-level device initialization structure.
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*BBD_Dnld) (
      IFX_TAPI_LL_CH_t *pCh,
      IFX_void_t const *pProc);

   /* obsolete / reserved */
   IFX_int32_t (*Pwr_Save_Dev) (IFX_TAPI_LL_DEV_t *pDev);

   /** Returns the number of entries in the capability list.
      \param pDev       Pointer to low-level device struct.
      \return
         The number of capability entries.
   */
   IFX_int32_t (*CAP_Number_Get) (IFX_TAPI_LL_DEV_t *pDev);

   /** Return the low-level device capability list.
      \param pDev       Pointer to low-level device struct.
      \param pCapList   Pointer to IFX_TAPI_CAP_LIST_t structure with details
                        where to copy the data to. No more capabilities than
                        specified in the element nCap will be copied into the
                        memory given in this structure.
      \return
         IFX_SUCCESS Always successful.
   */
   IFX_int32_t (*CAP_List_Get) (
      IFX_TAPI_LL_DEV_t *pDev,
      IFX_TAPI_CAP_LIST_t *pCapList);

   /** Checks in the capability list if a specific capability is supported.
      \param pDev       Pointer to low-level device struct.
      \param pCapList   Pointer to IFX_TAPI_CAP_t structure.
      \return
         Support status of the capability
         - 0 if not supported
         - 1 if supported
   */
   IFX_int32_t (*CAP_Check) (
      IFX_TAPI_LL_DEV_t *pDev,
      IFX_TAPI_CAP_t *pCapList);

   /** Returns free command mailbox inbox space.
      \param pDev       Pointer to low-level device struct.
      \param cmdmbx_size Pointer to variable where to return the command inbox
                        size. No check is done on this parameter!
      \return
         IFX_SUCCESS or IFX_ERROR.
   */
   IFX_int32_t (*GetCmdMbxSize) (
      IFX_TAPI_LL_DEV_t *pDev,
      IFX_uint8_t *cmdmbx_size);

   /** Called when TAPI channel is opened.
      \param pLLCh      Pointer to low-level channel struct.
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*Open) (IFX_TAPI_LL_CH_t *pLLCh);
   /** Called when TAPI channel is closed.
      \param pLLCh      Pointer to low-level channel struct.
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*Release) (IFX_TAPI_LL_CH_t *pLLCh);

   /** Forward IOCTL to low level driver.
      \param pCh        Pointer to either device or channel struct.
      \param nCmd       IOCTL identifier.
      \param ioarg      IOCTL argument.
      \return
         IFX_SUCCESS if successful else device specific return code.
   */
   IFX_int32_t (*Ioctl) (
      IFX_TAPI_LL_CH_t *pCh,
      IFX_uint32_t nCmd,
      IFX_ulong_t ioarg);

   /** Write a packet downstream
      \param pCh        Handle to low-level channel.
      \param buf        Pointer to a buffer with the data to be sent.
      \param count      Data length in bytes.
      \param ppos       unused
      \param stream     Tag that identifies the packet contents.
      \return
         - IFX_ERROR    on failure
         - nLength      length of handled data
   */
   IFX_int32_t (*Write) (
      IFX_TAPI_LL_CH_t *pCh,
      const IFX_char_t *buf,
      IFX_int32_t count,
      IFX_int32_t* ppos,
      IFX_TAPI_STREAM_t stream);

   /** amount of bytes HL must reserve before the data in downstream packets */
   IFX_uint32_t                             pktBufPrependSpace;
   /** shows that LL has a packet read routine and needs an upstream fifo */
   IFX_boolean_t                            bProvidePktRead;
   /** for each stream the number of channels that provide packet read */
   IFX_uint16_t                             readChannels[IFX_TAPI_STREAM_MAX];

   /* Coder related functions for the HL TAPI */
   IFX_TAPI_DRV_CTX_COD_t                   COD;

   /* PCM related functions for the HL TAPI */
   IFX_TAPI_DRV_CTX_PCM_t                   PCM;

   /* Signalling Module related functions for HL TAPI */
   IFX_TAPI_DRV_CTX_SIG_t                   SIG;

   /* Analog Line Module related functions for HL TAPI */
   IFX_TAPI_DRV_CTX_ALM_t                   ALM;

   /* DECT Module related functions for HL TAPI */
   IFX_TAPI_DRV_CTX_DECT_t                  DECT;

   /* Connection related functions for the HL TAPI */
   IFX_TAPI_DRV_CTX_CON_t                   CON;

   /* Protection andn Interrupt module functions for the HL TAPI */
   IFX_TAPI_DRV_CTX_IRQ_t                   IRQ;

   /* Polling interface related LL routines to be used by HL TAPI */
   IFX_TAPI_DRV_CTX_POLL_t                  POLL;

   /* Network Linetesting (NLT) related LL routines to be used by HL TAPI */
   IFX_TAPI_DRV_CTX_NLT_t                   NLT;

   /** array of pTapiDev pointers associated with this driver context */
   TAPI_DEV                                *pTapiDev;

   /** Debug only - handle a CmdError and read out the reason.
      \param pLLDev     Pointer to low-level device struct.
      \param pData      Pointer to cause and header of the the command.
      \return
      - VMMC_statusCmdWr Writing the command failed
      - VMMC_statusOk if successful
   */
   IFX_int32_t (*Dbg_CErr_Handler) (
      IFX_TAPI_LL_DEV_t *pLLDev,
      IFX_TAPI_DBG_CERR_t *pData);

   /** Retrieve the firmware version string of the currently downloaded firmware
       version.
      \param pLLDev     Pointer to low-level device struct.
      \param pVersionDevEntry Pointer to struct where to return the details.
      \return
         IFX_SUCCESS if successful else device specific return code.
    */
   IFX_int32_t (*VersionDevEntryGet) (
      IFX_TAPI_LL_DEV_t *pDev,
      IFX_TAPI_VERSION_DEV_ENTRY_t *pVersionDevEntry);

   /** Confirmation for MIPS overload event.
      \param pLLDev     Handle to low-level device
      \return
         IFX_SUCCESS if successful else device specific return code.
    */
   IFX_int32_t (*MipsOvldEnable) (IFX_TAPI_LL_DEV_t *pDev);

   /** Confirmation for IP v6 header error.
      \param pLLDev     Handle to low-level device
      \param value               IFX_TRUE - Enable
                                 IFX_FALSE - Disable

      \return
         IFX_SUCCESS if successful else device specific return code.
    */
   IFX_int32_t (*Ipv6HeaderError) (IFX_TAPI_LL_DEV_t *pDev, IFX_boolean_t bEnable);

   /** Enable firmware events redirection to high level driver with
    * IFX_TAPI_EVENT_DEBUG_FW_EVENTS event.
      \param pLLDev     Handle to low-level device

      \return
         IFX_SUCCESS if successful else device specific return code.
    */
   IFX_int32_t (*DbgFwEvents) (IFX_TAPI_LL_DEV_t *pLLDev);
} IFX_TAPI_DRV_CTX_t;
/*@}*/

#ifndef DRV_TAPI_H
struct _TAPI_DEV
{
   /* channel number IFX_TAPI_DEVICE_CH_NUMBER indicates the control device */
   /* ATTENTION, nChannel must be the first element */
   IFX_uint8_t               nChannel;
   /* pointer to LL device structure */
   IFX_TAPI_LL_DEV_t        *pLLDev;
};

struct _TAPI_CHANNEL
{
   /* channel number */
   /* ATTENTION, nChannel must be the first element */
   IFX_uint8_t                   nChannel;
   /* pointer to the Low level driver channel */
   IFX_TAPI_LL_CH_t             *pLLChannel;
   /* pointer to the tapi device structure */
   TAPI_DEV                     *pTapiDevice;

   /* \note:
      This is an incomplete declaration of the TAPI channel structure which is
      used only in the LL driver.
      For the complete implementation see in drv_tapi.h.
   */
};
#endif /* DRV_TAPI_H */


typedef IFX_void_t* Timer_ID;
typedef IFX_void_t (*TIMER_ENTRY)(Timer_ID timer_id, IFX_ulong_t arg);

typedef struct IFX_TAPI_DRV_CTX_t TAPI_LOW_LEVEL_DRV_CTX_t;

/* Registration function for the Low Level TAPI driver */
extern IFX_int32_t  IFX_TAPI_Register_LL_Drv    (IFX_TAPI_DRV_CTX_t*);
extern IFX_int32_t  IFX_TAPI_Unregister_LL_Drv  (IFX_int32_t majorNumber);
extern IFX_void_t   IFX_TAPI_DeviceReset        (TAPI_DEV *pTapiDev);
extern IFX_void_t   IFX_TAPI_ReportResources    (TAPI_DEV *pTapiDev,
                                                 IFX_TAPI_RESOURCE *pResources);

extern Timer_ID      TAPI_Create_Timer          (TIMER_ENTRY pTimerEntry,
                                                 IFX_ulong_t nArgument);
extern IFX_boolean_t TAPI_SetTime_Timer         (Timer_ID Timer,
                                                 IFX_uint32_t nTime,
                                                 IFX_boolean_t bPeriodically,
                                                 IFX_boolean_t bRestart);
extern IFX_boolean_t TAPI_Delete_Timer          (Timer_ID Timer);
extern IFX_boolean_t TAPI_Stop_Timer            (Timer_ID Timer);

extern IFX_void_t*   IFX_TAPI_VoiceBufferGet    (void);
extern IFX_int32_t   IFX_TAPI_VoiceBufferPut    (IFX_void_t *pData);
extern IFX_int32_t   IFX_TAPI_UpStreamFifo_Reset(TAPI_CHANNEL* pChannel,
                                                 IFX_TAPI_STREAM_t nStream);
extern IFX_int32_t   IFX_TAPI_UpStreamFifo_Put  (TAPI_CHANNEL* pTapiCh,
                                                 IFX_TAPI_STREAM_t nStream,
                                                 const IFX_void_t * const pData,
                                                 const IFX_uint32_t nLength,
                                                 const IFX_uint32_t nOffset);
extern IFX_void_t*   IFX_TAPI_DownStreamFifo_Handle_Get(TAPI_DEV* pTapiDev);

extern void    IFX_TAPI_DownStream_RequestData(
                        TAPI_CHANNEL* pChannel,
                        IFX_boolean_t bRequest);

extern IFX_int32_t IFX_TAPI_Ring_Stop_Ext       (TAPI_CHANNEL *pChannel, IFX_boolean_t bRestoreLineFeed);

#define IFX_TAPI_Event_ImmediateDispatch(pCh,pEvent) \
      IFX_TAPI_Event_DispatchExt (pCh,pEvent, IFX_FALSE)
#define IFX_TAPI_Event_DeferredDispatch(pCh,pEvent) \
      IFX_TAPI_Event_DispatchExt (pCh,pEvent, IFX_TRUE)

extern IFX_int32_t IFX_TAPI_Event_Dispatch (TAPI_CHANNEL *pChannel,
   IFX_TAPI_EVENT_t *pTapiEvent);

extern IFX_int32_t IFX_TAPI_Event_DispatchExt (TAPI_CHANNEL *pChannel,
   IFX_TAPI_EVENT_t *pTapiEvent, IFX_boolean_t bDefer);

#ifdef TAPI_VERSION3
extern IFX_void_t  TAPI_Tone_Set_Source         (TAPI_CHANNEL *,
                                                 IFX_uint8_t res,
                                                 IFX_TAPI_TONE_RESSEQ_t src);
#endif /* TAPI_VERSION3 */

extern IFX_void_t TAPI_Cid_Abort                (TAPI_CHANNEL *pChannel);
extern IFX_boolean_t TAPI_Cid_IsActive          (TAPI_CHANNEL *pChannel);

extern IFX_TAPI_CID_RX_DATA_t *TAPI_Phone_GetCidRxBuf (TAPI_CHANNEL *pChannel, IFX_uint32_t nLen);
extern TAPI_CMPLX_TONE_STATE_t TAPI_ToneState   (TAPI_CHANNEL *pChannel,
                                                 IFX_uint8_t res);

/** Put packet received in the irq handler into the egress fifo */
extern IFX_int32_t irq_IFX_TAPI_KPI_PutToEgress (TAPI_CHANNEL *pChannel,
                                                 IFX_TAPI_KPI_STREAM_t stream,
                                                 IFX_void_t *pPacket,
                                                 IFX_uint32_t nPacketLength);

/** Retrieve the KPI Channel number of a given stream on a given TAPI Channel */
extern IFX_TAPI_KPI_CH_t IFX_TAPI_KPI_ChGet     (TAPI_CHANNEL *pChannel,
                                                 IFX_TAPI_KPI_STREAM_t stream);

/** Raise handler for KPI ingress direction */
extern IFX_void_t IFX_TAPI_KPI_ScheduleIngressHandling (void);

/* Packet path statistic */
extern IFX_void_t  IFX_TAPI_Stat_Add            (TAPI_CHANNEL *pChannel,
                                                 IFX_TAPI_STREAM_t stream,
                                                 TAPI_STAT_COUNTER_t counter,
                                                 IFX_uint32_t value);

/* Update the SLIC FXO flag in TAPI HL device context;
   the flag is required for special handling of FXO ioctls on systems
   with 3-channel SLIC and Clare LITELINK PLI */
extern IFX_void_t IFX_TAPI_Update_SlicFxo       (TAPI_DEV *pTapiDev,
                                                 IFX_boolean_t bVal);
#endif /* _DRV_TAPI_LL_INTERFACE_H */
