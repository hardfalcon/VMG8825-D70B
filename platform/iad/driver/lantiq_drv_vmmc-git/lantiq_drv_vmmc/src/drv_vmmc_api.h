#ifndef _DRV_VMMCAPI_H
#define _DRV_VMMCAPI_H
/******************************************************************************

                            Copyright (c) 2006-2015
                        Lantiq Beteiligungs-GmbH & Co.KG
                             http://www.lantiq.com

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/**
   \file drv_vmmc_api.h
   This file contains the defines, the structures declarations,
   the tables declarations and the global functions declarations.
*/

/* includes */
#include "drv_api.h"
#include "drv_tapi_io.h"
#include "vmmc_io.h"
#include "drv_vmmc_hostapi.h"
#include "drv_tapi_ll_interface.h"
/* error information from outside, to make this file more clean */
#include "drv_vmmc_errno.h"

#include "drv_vmmc_fw_commands_voip.h"
#include "drv_vmmc_fw_data.h"

/* ============================= */
/* Trace groups                  */
/* ============================= */
/* Trace groups */
DECLARE_TRACE_GROUP(VMMC);

/* ============================= */
/* Global Defines                */
/* ============================= */

/** Dev type used for the cap list */
#define VMMC_DEV_TYPE         IFX_TAPI_DEV_TYPE_VOICE_MACRO

/** Maximal Command/Data Words */
#define MAX_CMD_WORD          32
/** Maximal Packet Words */
#define MAX_PACKET_WORD       256
/* fifo threshold for voice packet */
#define POBX_BUFFER_THRESHOLD 20
/* read timeout for mailbox in ms */
#define CMD_MBX_RD_TIMEOUT_MS 200

/* default payload type for RTP event packets */
#define DEFAULT_EVTPT         0x60

/* PCM timeslot allocation management definitions */
#define PCM_MAX_TS            127
#define PCM_TS_ARRAY          ((PCM_MAX_TS + 3) / 32)

/* max CID Data size in word to send on one go */
#define MAX_CID_LOOP_DATA     20

/* max tone resources per SIG module */
#define LL_TAPI_TONE_MAXRES   2

/** Number of PCM highways (busses) */
#define PCM_HIGHWAY                    1

/** count of PCM shortcut channel resources */
#define VMMC_PCM_S_CH_RES_CNT          2

/* ============================= */
/* VMMC Common defines        */
/* ============================= */

#define VMMC_MINOR_BASE       10
#define DEV_NAME              "vmmc"
#define VMMC_MAJOR            122

/* ============================= */
/* VMMC flag settings         */
/* ============================= */

/* this value is used for variables which should have a defined
   content. Matches for bytes and replaces -1 to use variable space. */
#define NOTVALID              0xff

/* ============================= */
/* Global Macros                 */
/* ============================= */

#ifdef DEBUG
#define VCH_MAGIC        0x81f56d4e
#define VDEV_MAGIC       0x81f56d4e

#define CHK_DEV_MAGIC VMMC_ASSERT(pDev->magic == VDEV_MAGIC)
#define CHK_CH_MAGIC  VMMC_ASSERT(pCh->magic  == VCH_MAGIC)
#else
#define CHK_DEV_MAGIC
#define CHK_CH_MAGIC
#endif /* DEBUG */

#define VMMC_SUCCESS TAPI_SUCCESS


#define RETURN_STATUS(code)                                                \
   do{                                                                     \
      if (((IFX_return_t)(code) == IFX_ERROR) ||                           \
          ((TAPI_statusClass_t)(code) > TAPI_statusClassWarn))             \
      {                                                                    \
         VMMC_ChErrorEvent (pCh, (IFX_uint16_t)(code),                     \
                            __LINE__, __FILE__,IFX_NULL, 0);               \
      }                                                                    \
      return (code);                                                       \
   }while(0)

#define RETURN_INFO_STATUS(code,info,size)                                 \
   /*lint -e{506, 774} */                                                  \
   do{                                                                     \
      if (((IFX_return_t)(code) == IFX_ERROR) ||                           \
          ((TAPI_statusClass_t)(code) > TAPI_statusClassWarn))             \
      {                                                                    \
         VMMC_ChErrorEvent (pCh, (IFX_uint16_t)code, __LINE__, __FILE__,   \
                            (void*)(info), (IFX_uint32_t)(size));          \
      }                                                                    \
      return (code);                                                       \
   }while(0)

#define RETURN_DEVSTATUS(code)                                             \
   /*lint -e{506, 774} */                                                  \
   do{                                                                     \
      if (((IFX_return_t)(code) == IFX_ERROR) ||                           \
          ((TAPI_statusClass_t)(code) > TAPI_statusClassWarn))             \
      {                                                                    \
         VMMC_DevErrorEvent (pDev, (code), __LINE__, __FILE__,             \
                             IFX_NULL, 0);                                 \
      }                                                                    \
      return (code);                                                       \
   }while(0)

#define RETURN_INFO_DEVSTATUS(code,info,size)                              \
   /*lint -e{506, 774} */                                                  \
   do{                                                                     \
      if (((IFX_return_t)(code) == IFX_ERROR) ||                           \
          ((TAPI_statusClass_t)(code) > TAPI_statusClassWarn))             \
         VMMC_DevErrorEvent (pDev, (code), __LINE__, __FILE__,             \
                             (void*)(info), (IFX_uint32_t)(size));         \
      return code;                                                         \
   }while(0)

/*******************************************************************************
Description:
   Converts a value to one byte per masking
Arguments:
   val  : any value
Return:
   Low Byte
*******************************************************************************/
#define LOWBYTE(val)               (IFX_uint8_t)((val) & 0xFF)

/*******************************************************************************
Description:
   Extracts the high byte from a IFX_uint16_t value
Arguments:
   val  : any IFX_uint16_t value
Return:
   High Byte
*******************************************************************************/
#define HIGHBYTE(val)              (IFX_uint8_t)(((val)>>8) & 0xFF)


/* ================================ */
/* Macros related to irq handling   */
/* ================================ */

/*******************************************************************************
Description:
   Disables the interrupt line
Arguments
   irq -  irq line number
Remarks
   This macro is system specific. In case the Operating system methods can not
   be used for this purpose, define the correct macro in your user configuration
   file.
*******************************************************************************/
#ifndef VMMC_DISABLE_IRQLINE
#define VMMC_DISABLE_IRQLINE(irq)       VMMC_OS_IRQ_DISABLE(irq)
#endif

/*******************************************************************************
Description:
   Enables the interrupt line
Arguments
   irq -  irq line number
Remarks
   This macro is system specific. In case the Operating system methods can not
   be used for this purpose, define the correct macro in your user configuration
   file.
*******************************************************************************/
#ifndef VMMC_ENABLE_IRQLINE
#define VMMC_ENABLE_IRQLINE(irq)        VMMC_OS_IRQ_ENABLE(irq)
#endif


/*******************************************************************************
Description:
   Disables the global interrupt
Arguments
   var -  local mask that has to be stored for VMMC_ENABLE_IRQGLOBAL
Remarks
   This macro is system specific. In case the Operating system methods can not
   be used for this purpose, define the correct macro in your user configuration
   file.
   This macro is used when no device specific interrupt handler or interrupt
   line is configured (nIrg < 0).
*******************************************************************************/
#ifndef VMMC_DISABLE_IRQGLOBAL
#define VMMC_DISABLE_IRQGLOBAL(var)       VMMC_OS_LOCKINT(var)
#endif

/*******************************************************************************
Description:
   Enables the global interrupt
Arguments
   var -  local mask that was stored by  VMMC_DISABLE_IRQGLOBAL
Remarks
   This macro is system specific. In case the Operating system methods can not
   be used for this purpose, define the correct macro in your user configuration
   file.
   This macro is used when no device specific interrupt handler or interrupt
   line is configured (nIrg < 0).
*******************************************************************************/
#ifndef VMMC_ENABLE_IRQGLOBAL
#define VMMC_ENABLE_IRQGLOBAL(var)       VMMC_OS_UNLOCKINT(var)
#endif

/** mark variable as unused, to suppress compilation warrnings

   \remarks: (ANSI X3.159-1989)
      void is used, in any context where the value of an expression
      is to be discarded, to indicate explicitly that a value is
      ignored by writing the cast (void).
*/
#define VMMC_UNUSED(var) ((IFX_void_t)(var))

/** round the parameter */
#define ROUND(n)        ((( (n) * 10) + 5 ) / 10)
/** macro to divide the parameter by 10 and round accordingly */
#define ROUND_DIV10(n)  ( ( (n) +  5) / 10 )
/** macro to divide the parameter by 100 and round accordingly */
#define ROUND_DIV100(n) ( ( (n) + 50) / 100 )
/** macro to divide the parameter by 1000 and round accordingly */
#define ROUND_DIV1000(n)( ( (n) + 500)/ 1000 )

/** constant 2^2 */
#define C2_2            4
/** constant 2^6 */
#define C2_6            64
/** constant 2^8 */
#define C2_8            256
/** constant 2^13 */
#define C2_13           8192
/** constant 2^14 */
#define C2_14           16384
/** constant 2^15 */
#define C2_15           32768
/** constant 2^31 */
#define C2_31           2147483648UL
/** constant 2^32 */
#define C2_32           4294967295UL

/* ================================ */
/* Enumerations                     */
/* ================================ */

/* device states */
/* firmware successfully downloaded */
#define   DS_FW_DLD                    0x00000001
/* phi downloaded successfully, no further download will be allowed */
#define   DS_PHI_DLD                   0x00000002
/* CRAM Downloaded */
#define   DS_CRAM_DLD                  0x00000004
/* Basic Init done */
#define   DS_BASIC_INIT                0x00000010
/** GPIOs for PCM reserved */
#define   DS_GPIO_RESERVED             0x00000020
/** Device successfully initialized */
#define   DS_DEV_INIT                  0x00000040
/** COD module enabled  */
#define   DS_COD_EN                    0x00000100
/** ALM module enabled  */
#define   DS_ALM_EN                    0x00000200
/** SIG module enabled  */
#define   DS_SIG_EN                    0x00000400
/** PCM interface enabled  */
#define   DS_PCM_EN                    0x00000800

/* channel states */
/* Init done */
#define   CS_INIT                      0x000020


typedef enum {
   DSP_CPTD_FL_16    = 0,
   DSP_CPTD_FL_32    = 1,
   DSP_CPTD_FL_64    = 2
} DSP_CPTD_FL_t;

typedef enum
{
   DSP_CPTD_TP_10_3400_HZ  = 0,
   DSP_CPTD_TP_250_3400_HZ = 1
} DSP_CPTD_TP_t;

typedef enum
{
   DSP_CPTD_WS_HAMMING     = 0,
   DSP_CPTD_WS_BLACKMAN    = 1
} DSP_CPTD_WS_t;

/** Enumeration for narrowband / wideband selection */
typedef enum
{
   NB_8_KHZ,  /* Narrowband Mode, 8 KHz Sampling Rate */
   WB_16_KHZ  /* Wideband Mode, 16 KHz Sampling Rate */
} OPMODE_SMPL;

/** Enumeration for action to be executed in sampling mode functions */
typedef enum
{
   SM_SET,
   SM_CHECK
} SM_ACTION;

/** Type of DC/DC converter */
enum VMMC_DCDC_TYPE
{
   /** Default: Dedicated Inverting Buck-Boost DC/DC Converter. */
   VMMC_DCDC_TYPE_DEFAULT_IBB = 0,
   /** Dedicated Inverting Buck-Boost DC/DC Converter. */
   VMMC_DCDC_TYPE_IBB = 1,
   /** Combined Inverting Buck-Boost DC/DC Converter. */
   VMMC_DCDC_TYPE_CIBB = 2
};

/* ============================= */
/* Global Structures             */
/* ============================= */

struct VMMC_ALMCH;
struct VMMC_CODCH;
struct VMMC_SIGCH;
struct VMMC_PCMCH;
struct VMMC_CON;
struct VMMC_DECT;
struct VMMC_LINCH;
struct VMMC_RES_ES;
struct VMMC_RES_LEC;
union VMMC_PMC_CHANNEL;

typedef struct VMMC_ALMCH VMMC_ALMCH_t;
typedef struct VMMC_CODCH VMMC_CODCH_t;
typedef struct VMMC_SIGCH VMMC_SIGCH_t;
typedef struct VMMC_PCMCH VMMC_PCMCH_t;
typedef struct VMMC_CON VMMC_CON_t;
typedef struct VMMC_DECTCH VMMC_DECTCH_t;
typedef struct VMMC_LINCH VMMC_LINCH_t;
typedef struct VMMC_RES_ES VMMC_RES_ES_t;
typedef struct VMMC_RES_LEC VMMC_RES_LEC_t;
typedef struct VMMC_RES_CPTD VMMC_RES_CPTD_t;
typedef union  VMMC_PMC_CHANNEL VMMC_PMC_CHANNEL_t;
typedef struct VMMC_RES_HDLC VMMC_RES_HDLC_t;
typedef struct VMMC_ANN_s VMMC_ANN_t;

typedef struct _VMMC_CHANNEL VMMC_CHANNEL;
typedef struct _VMMC_DEVICE  VMMC_DEVICE;
typedef struct vmmc_interrupt_s vmmc_interrupt_t;
typedef struct _VMMC_CAPABILITIES VMMC_CAPABILITIES_t;

#ifdef EVALUATION
#include "drv_vmmc_eval.h"
#else
#define VMMC_EVAL_EVENT(channelNumber,tapiEvent)
#define VMMC_EVAL_UNHANDLED_EVENT(channel,eventId)
#define VMMC_EVAL_MOL(pDev)
#endif /* #ifdef EVALUATION */

/** Mailbox Data packet structure */
typedef struct
{
   /** control and payload data */
   IFX_uint16_t pData[MAX_PACKET_WORD];
} PACKET;

#if 0
/** tone resource structure as time argument */
typedef struct
{
   /* timer id for voice path establisment */
   Timer_ID                Tone_Timer;
   VMMC_CHANNEL *pCh;
   /** Resource number of the tone play unit */
   IFX_uint32_t            nRes;
} VMMC_TONERES;
#endif /* 0 */

struct vmmc_interrupt_s
{
   /** OS/architecture specific IRQ number */
   VMMC_OS_INTSTAT         nIrq;
   /** "registered in OS" flag */
   IFX_boolean_t           bRegistered;
   /** interrupt enabled flag : IFX_TRUE=Enabled / IFX_FALSE=Disabled */
   volatile IFX_boolean_t  bIntEnabled;
#ifdef DEBUG_INT
   /* this is used to debug nested calls of
      Vmmc_IrqLockDevice/Vmmc_IrqUnlockDevice */
   IFX_int32_t             nCount;
#endif /* DEBUG */
   /** to lock against multiple enable/disable */
   VMMC_OS_mutex_t         intAcc;
   /* pointer to first vmmc device, registered for this irq */
   VMMC_DEVICE          *pdev_head;
   /** pointer to next different irq */
   vmmc_interrupt_t     *next_irq;
};


/* capabilities structure for internal usage */
struct _VMMC_CAPABILITIES
{
   /* maximum EDSP resources available */
   IFX_uint8_t   nMaxRes;
   /* Number of UTG resources per channel (== SIG module), either 1 or 2 */
   IFX_uint32_t  nUtgPerCh : 8;
   /* Number of CPTDs per channel */
   IFX_uint32_t  nCptdPerCh : 4;
   /** Support for extended jitter buffer statistics 0=no 1=yes */
   IFX_uint32_t  bExtendedJBsupported : 1;
   /** Support of jitter buffer enhancements 0=no 1=yes */
   IFX_uint32_t  bEnhancedJB : 1;
   /** Support for two UTDs per SIG module */
   IFX_uint32_t  bUtd2supported : 1;
   /** RTP Protocol support: 0 = no / 1 = yes */
   IFX_uint32_t  bProtocolRTP : 1;
   /** AAL Protocol support: 0 = no / 1 = yes */
   IFX_uint32_t  bProtocolAAL : 1;
   /** Event Mailbox support: 0 = no / 1 = yes */
   IFX_uint32_t  bEventMailboxSupported : 1;
   /** Echo suppressor support on ALM */
   IFX_uint32_t  bESonALM : 1;
   /** Echo suppressor support on PCM */
   IFX_uint32_t  bESonPCM : 1;
   /** Echo suppressor support on DECT */
   IFX_uint32_t  bESonDECT : 1;
   /** T.38 stack is implemented in firmware */
   IFX_uint32_t  bT38FW : 1;
   /** DTMF receiver enhancements: 0=no 1=yes */
   IFX_uint32_t  bDT1 : 1;
   /** DTMF receiver enhancements, step 1: 0=no 1=yes */
   IFX_uint32_t  bDT2 : 1;
   /** Support of RFC 4040 clearmode: 0=no 1=yes */
   IFX_uint32_t  bRFC4040supported : 1;
   /** Support of Echo Suppressor enhancements: 0=no 1=yes */
   IFX_uint32_t  bEnhancedES : 1;
   /** Support of the idle pattern in the D-Channel:
          0=only flags, 1=both flags and 0xFF */
   IFX_uint32_t  bDIP : 1;
   /** Support of jitter adaptation during silence: 0=no 1=yes */
   IFX_uint32_t  bJAS : 1;
   /** Echo canceler additions 1: 0=no 1=yes
       EC supports parameter freeze and automatic parameter freeze. */
   IFX_uint32_t  bECA1 : 1;
   /** EMOS calculation: 0=no 1=yes */
   IFX_uint32_t  bEMOS : 1;
   /** Message Waiting Lamp supported by SDD: 0=no 1=yes */
   IFX_uint32_t  bMWI : 1;
   /** FW version with timestamp extension supported 0=no 1=yes */
   IFX_uint32_t  bVerExtTs : 1;
   /** The handshake mechanism for Clock Scaling is supported 0=no 1=yes */
   IFX_uint32_t  bCLKHS : 1;
   /** RTCP XR support 0=no 1=yes */
   IFX_uint32_t  bRtcpXR : 1;
   /** AMR Encoder
       0 - old style,
       1 - bit rate can be set in a separate bitfield */
   IFX_uint32_t  bAMRE : 1;
   /* combined DC/DC operation supported */
   IFX_uint32_t  bfw_combDcDc : 1;
   /** FW supports PCM split timeslots 0=no 1=yes */
   IFX_uint32_t  bPcmSplitTs : 1;
   /* Hook-event timestamp supported by FW: 0=no 1=yes */
   IFX_uint32_t  bHTS : 1;
   /* FW support for burst number extension in RTCP XR VoIP Metrics Report.
      0=no 1=yes */
   IFX_uint32_t  bXR_BN : 1;

   /* Derived data fields
      (not available as direct firmware values but calculated by the driver) */
   /** Number of SLIC native FXS channels */
   IFX_uint32_t  nFXS : 8;
   /** Number of SLIC native FXO channels */
   IFX_uint32_t  nFXO : 8;

   /* Fields below are a direct copy from the firmware capability message */
   /** Number of PCM Channels */
   IFX_uint32_t  nPCM : 8;
   /** Number of Analog Line Channels */
   IFX_uint32_t  nALI : 8;
   /** Number of Audio Channels */
   IFX_uint32_t  nAudioCnt : 8;
   /** Number of Signaling Channels */
   IFX_uint32_t  nSIG : 8;
   /** Number of Coder Channels */
   IFX_uint32_t  nCOD : 8;
   /** Number of AGCs */
   IFX_uint32_t  nAGC : 8;
   /** Number of Equalizers */
   IFX_uint32_t  nEQ : 8;
   /** Number of Near-End LECs */
   IFX_uint32_t  nNLEC : 8;
   /** Number of Combined Near-End/Far-End LECs */
   IFX_uint32_t  nWLEC : 8;
   /** Number of Near-End Wideband LECs */
   IFX_uint32_t  nNWLEC : 8;
   /** Number of Combined Near-End/Far-End Wideband LECs */
   IFX_uint32_t  nWWLEC : 8;
   /** Number of Universal Tone Generators */
   IFX_uint32_t  nUTG : 8;
   /** Number of DTMF Generators */
   IFX_uint32_t  nDTMFG : 8;
   /** Number of Caller ID Senders */
   IFX_uint32_t  nCIDS : 8;
   /** Number of Caller ID Receivers */
   IFX_uint32_t  nCIDR : 8;
   /** Number of Call Progress Tone Detectors */
   IFX_uint32_t  nCPTD : 8;
   /** Number of Modem and Fax Tone Discriminators (MFTDs) */
   IFX_uint32_t  nMFTD : 8;
   /** Number of FAX Channels with FAX Relay (T.38) Support */
   IFX_uint32_t  nFAX : 8;
   /** Number of DTMF Detectors */
   IFX_uint32_t  nDTMFD : 8;
   /** Number of PCM codec resources (G.726 and G.722) */
   IFX_uint32_t  nPCMCOD : 5;
   /** Number of "PCM shortcuts" */
   IFX_uint32_t  nPCMS : 2;
   /** Number of HDLC framers (e.g. for D-channel access) */
   IFX_uint32_t  nHDLC : 2;
   /** Codecs */
   IFX_uint32_t  CODECS : 16;
   /** Maximum Number of Low Complexity Coders for the Coder Channel */
   IFX_uint32_t  CLOW : 8;
   /** Maximum Number of Mid Complexity Coders for the Coder Channel */
   IFX_uint32_t  CMID : 8;
   /** Maximum Number of High Complexity Coders for the Coder Channel*/
   IFX_uint32_t  CMAX : 8;
   /** PCM Channel Coders */
   IFX_uint32_t  PCOD : 8;
   /** MFTD Version */
   IFX_uint32_t  MFTDV : 4;
   /** Number of DECT Channels */
   IFX_uint32_t  nDECT : 8;
   /** DECT codecs */
   IFX_uint32_t  DECT_CODECS;
   /** Echo Suppressor in analog line channel */
   IFX_uint32_t  ES : 1;
   /** Tone Detection Capabilities */
   IFX_uint32_t  TONES : 16;
   /** Features */
   IFX_uint32_t  FEAT : 8;
   /** Overlays */
   IFX_uint32_t  OVL : 8;
   /** Event Playout Capabilities */
   IFX_uint32_t  EPC : 8;
   /** Event Transmission Capabilities */
   IFX_uint32_t  ETC : 8;
   /** Number of echo suppressor ressources */
   IFX_uint32_t  nES : 8;
   /** Number of Linear Channels */
   IFX_uint32_t  nLIN : 8;
};


/* SDD specific attributes which are device global. */
struct SDD_DEVICE
{
   /* SmartSLIC revision */
   IFX_uint8_t          nSlicDevId;
   IFX_uint8_t          nSlicRevision;
   /* Firmware (DCCtrl) version */
   IFX_uint8_t          nFirmwareVers;
   IFX_uint8_t          nFirmwareStep;
   IFX_uint8_t          nFirmwareHotFix;
   /* ASDSP version */
   IFX_uint8_t          nASDSPVers;
   IFX_uint8_t          nASDSPStep;
   /* Indicates combined DC/DC (true) or dedicated DC/DC (false, default) */
   IFX_boolean_t        bDcDcHwCombined;
   /* DC/DC converter type indicated on the GPIOs of the SLIC. */
   enum VMMC_DCDC_TYPE  nAllowedDcDcType;
};


/* Channel structure */
struct _VMMC_CHANNEL
{
   /* common fields with VMMC_DEVICE structure */
#ifdef DEBUG
   IFX_uint32_t             magic;
#endif /* DEBUG */
   /* Actual  vmmc channel, starting with 1, a channel number 0 indicates
      the control device structure VMMC_DEVICE */
   IFX_uint8_t              nChannel;
   /* tracking in use of the device */
   IFX_uint16_t             nInUse;
   /* Status of channel, see states defines. */
   IFX_uint32_t             nState;

   /* ptr to actual device */
   VMMC_DEVICE             *pParent;
   /* overall channel protection ( read/write/ioctl level)
      PS: Avoid nested locking of this mutex. It can lead to a deadlock */
   VMMC_OS_mutex_t          chAcc;

   /* ALM Channel management structure */
   VMMC_ALMCH_t            *pALM;
   /* CODER channel management structure */
   VMMC_CODCH_t            *pCOD;
   /* SIGNALLING channel management structure */
   VMMC_SIGCH_t            *pSIG;
   /* PCM Channel management structure*/
   VMMC_PCMCH_t            *pPCM;
   /* DECT channel management structure */
   VMMC_DECTCH_t           *pDECT;
   /* Connection module structure */
   VMMC_CON_t              *pCON;

   TAPI_CHANNEL            *pTapiCh;

#if 0
   VMMC_TONERES            *pToneRes;
#endif
   IFX_uint8_t              nEvtPT;
#ifdef VMMC_FEAT_FAX_T38
   /* set flag when fax data are requested
      on downstream direction */
   IFX_TAPI_T38_STATUS_t         TapiFaxStatus;
   /**\todo just for T.38 testing, maybe removed soon */
   IFX_uint32_t   nFdpReq;
   IFX_uint32_t   nFdpReqServiced;
#endif /* VMMC_FEAT_FAX_T38 */
#ifdef VMMC_FEAT_FAX_T38_FW
   IFX_boolean_t   bTapiT38Status;
#endif /* VMMC_FEAT_FAX_T38_FW */
   /* store count of access to the channel without KPI path */
   IFX_uint32_t   nNoKpiPathError;
#ifdef VMMC_FEAT_ANNOUNCEMENTS
   /* store handle to the announcement resource */
   VMMC_ANN_t              *pAnn;
#endif /* VMMC_FEAT_ANNOUNCEMENTS */
};

/* VMMC global Device Stucture */
struct _VMMC_DEVICE
{
#ifdef DEBUG
   IFX_uint32_t             magic;
#endif /* DEBUG */
   /* Vmmc channel. This value is always zero for the device structure.
      Different channel device structure are from type VMMC_CHANNEL.
      The first field is also a nChannel. This gurantees that the channel
      field is always available, although the cast was wrong. But ensure
      the correct cast after reading the channel field */
   IFX_uint8_t              nChannel;
   /* tracking in use of the device */
   IFX_uint16_t             nInUse;
   /* device error status, never reset. read out for detail
      information if return value of function is not IFX_SUCCESS */
   IFX_int32_t              err;
   /* absolute index of the device, when multiple devices are existing
      For the first device the value is set to zero                        */
   IFX_int32_t               nDevNr;
   /* Status of device, see states defines .
      Must be protected against interrupts and concurrents tasks           */
   IFX_vuint32_t             nDevState;
   /* local storage for the interrupt flags that are returned during disabling
      the global interrupts. This value is used to enable the interrupts
      again */
   VMMC_OS_INTSTAT           nIrqMask;
   /* chip revision, read from REVISION register */
   IFX_uint8_t               nChipRev;
   /* chip major revision, set depending on REVISION register */
   IFX_uint8_t               nChipMajorRev;
   /* chip type, read from REVISION register */
   IFX_uint8_t               nChipType;
   /* EDSP version number (major, minor, hotfix), platform and variant */
   IFX_uint8_t               nEdspVers[5];

   /** Capability list for the device  -  Interface to application-level */
   IFX_TAPI_CAP_t          *CapList;
   IFX_uint8_t               nMaxCaps;
   /** Capability list for the device  -  Internal data for driver */
   VMMC_CAPABILITIES_t       caps;
   IFX_boolean_t             bCapsRead;

   /* SDD specific attributes */
   struct SDD_DEVICE         sdd;

   /* array of line echo canceller resources */
   VMMC_RES_LEC_t           *pResLec;
   /* array of CPTD resources */
   VMMC_RES_CPTD_t          *pResCptd;
   /* array of echo supressor resources */
   VMMC_RES_ES_t            *pResEs;

   /* VMMC cmd mbx concurent access protection mutex.
      PS: Avoid nested locking of this mutex. It can lead to a deadlock */
   VMMC_OS_mutex_t           mtxCmdMbxAcc;
   /* VMMC share variables concurent access protection mutex.
      PS: Avoid nested locking of this mutex. It can lead to a deadlock */
   VMMC_OS_mutex_t           mtxMemberAcc;
   /* mutex protecting CmdReadAccess */
   VMMC_OS_mutex_t           mtxCmdReadAcc;
   /* mutex protecting the VoiceDataMailbox */
   VMMC_OS_mutex_t           mtxDataMbxAcc;
   /* channel structures */
   VMMC_CHANNEL              pChannel[VMMC_MAX_CH_NR];

#ifdef VMMC_FEAT_PCM
   /** PCM RX timeslot allocation management. Each bit represents a timeslot. */
   IFX_uint32_t              PcmRxTs[PCM_HIGHWAY][PCM_TS_ARRAY];
   /** PCM TX timeslot allocation management. Each bit represents a timeslot. */
   IFX_uint32_t              PcmTxTs[PCM_HIGHWAY][PCM_TS_ARRAY];
   /** Actual number of PCM timeslots resulting from the DCL clock setting. */
   IFX_uint16_t              nMaxTimeslot;
#endif /* VMMC_FEAT_PCM */

   /** array of PCM shortcut resources */
   IFX_highLow_t             PcmSchRes[VMMC_PCM_S_CH_RES_CNT];

#ifdef VMMC_FEAT_HDLC
   /** array of HDLC resources */
   VMMC_RES_HDLC_t           *pResHdlc;
#endif /* VMMC_FEAT_HDLC */

   VMMC_OS_event_t           mpsCmdWakeUp;

   /* variables for shared interrupt handling
      (see implementatiuon in drv_vmmc_vxworks.c) */
   vmmc_interrupt_t     *pIrq;
   VMMC_DEVICE          *pInt_NextDev;

   /* entry to Tapi device, needs protection  */
   TAPI_DEV             *pTapiDev;

   /* store status of pkt mailbox (downstream) congestion to avoid flooding
      of TAPI events to the application */
   IFX_boolean_t        bPktMbxCongested;
   /* error flag for cmd read - avoid all kinds of compiler optimization */
   volatile IFX_boolean_t bCmdReadError;
   /* command outbox data availability - avoid all kinds of optimizations */
   volatile IFX_boolean_t bCmdOutBoxData;

#ifdef VMMC_FEAT_CLOCK_SCALING
   /* Power management control data */
   VMMC_PMC_CHANNEL_t       *pPMC;
   /* Current status of the DART. IFX_TRUE = active.*/
   volatile IFX_boolean_t    bDartActive;
   /* Current status of the VoIP FW. IFX_TRUE = active.  */
   volatile IFX_boolean_t    bFwActive;
   /* Flag for task context that clock high is needed. */
   volatile IFX_boolean_t    bNeedSysClkHigh;
   /* Flag that is set while the FW is started. */
   volatile IFX_boolean_t    bFwStartup;
#endif /* VMMC_FEAT_CLOCK_SCALING */

   /* Counter for MIPS overload occurence */
   IFX_uint32_t              nMipsOl;
   /* Flag indicating an SSI crash */
   IFX_boolean_t             bSSIcrash;
   /* Debug counter for the number of successful SLIC recoveries */
   IFX_uint32_t              nSlicRecoveryCnt;

#ifdef VMMC_FEAT_ANNOUNCEMENTS
   /* announcement resources storage */
   VMMC_ANN_t               *pAnn;
#endif /* VMMC_FEAT_ANNOUNCEMENTS */
};

/* ============================= */
/* Global variable declaration   */
/* ============================= */

/* ============================= */
/* Global function declaration   */
/* ============================= */

/* IOCTL specific functions, implemented in drv_vmmc_ioctl.c */
IFX_int32_t VMMC_Dev_Spec_Ioctl  (IFX_TAPI_LL_CH_t *pLLDummyCh,
                                  IFX_uint32_t iocmd, IFX_ulong_t ioarg);
extern IFX_void_t  VMMC_ChErrorEvent    (VMMC_CHANNEL *pCh, IFX_uint16_t err,
                                  IFX_uint32_t nLine, const IFX_char_t *sFile,
                                 void* info, IFX_uint32_t len);
extern IFX_void_t  VMMC_DevErrorEvent   (VMMC_DEVICE *pCh, IFX_uint16_t err,
                                  IFX_uint32_t nLine, const IFX_char_t* sFile,
                                 void* info, IFX_uint32_t len);

IFX_int32_t VMMC_CmdErr_Handler  (IFX_TAPI_LL_DEV_t *pDev,
                                  IFX_TAPI_DBG_CERR_t *pData);

/* VMMC OS specific functions, implemented in drv_vmmc_<os>.c */
IFX_void_t Vmmc_IrqLockDevice          (IFX_TAPI_LL_DEV_t *pLLDev);
IFX_void_t Vmmc_IrqUnlockDevice        (IFX_TAPI_LL_DEV_t *pLLDev);
IFX_void_t Vmmc_IrqUnlockGlobal        (VMMC_DEVICE *pDev);

/* VMMC API functions, implemented in drv_vmmc_api.c  */
IFX_int32_t CmdWrite       (VMMC_DEVICE *pDev, IFX_uint32_t *pCmd,
                            IFX_uint16_t count);
IFX_int32_t CmdWriteIsr    (VMMC_DEVICE *pDev, IFX_uint32_t *pCmd,
                            IFX_uint16_t count);
IFX_int32_t CmdRead        (VMMC_DEVICE *pDev, IFX_uint32_t *pCmd,
                            IFX_uint32_t *pData, IFX_uint16_t count);

extern void cpb2w (IFX_uint16_t *pWbuf, IFX_uint8_t *pBbuf, IFX_uint32_t nB);
#endif /* _DRV_VMMCAPI_H */
