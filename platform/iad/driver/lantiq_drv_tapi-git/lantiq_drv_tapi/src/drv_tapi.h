#ifndef DRV_TAPI_H
#define DRV_TAPI_H

/******************************************************************************

                            Copyright (c) 2006-2016
                        Lantiq Beteiligungs-GmbH & Co.KG
                             http://www.lantiq.com

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/**
   \file drv_tapi.h
   Contains TAPI functions declaration und structures.
*/


/* ============================= */
/* Includes                      */
/* ============================= */
#include "drv_tapi_config.h"
#include "lib_fifo.h"
#include "lib_bufferpool.h"
#include "drv_tapi_io.h"
#include "drv_tapi_kio.h"
#include "drv_tapi_errno.h"
#include "drv_tapi_osmap.h"

#ifdef TAPI_FEAT_QOS
   #include "drv_tapi_qos_io.h"
#endif /* TAPI_FEAT_QOS */

#if defined(LINUX) && defined (__KERNEL__)
   #if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
      #include <linux/cdev.h>
   #endif /* LINUX_VERSION_CODE >= Linux 2.6.0 */
#endif

/* ============================= */
/* Global defines                */
/* ============================= */

/* simple usage of ptr_chk */
#define IFX_TAPI_PtrChk(ptr) ptr_chk((ptr), #ptr)

#define VMMC_IOC_MAGIC 'M'
#define VINETIC_IOC_MAGIC 'V'
#define SVIP_IOC_MAGIC 'S'
#define DUS_IOC_MAGIC 'D'
#define VXT_IOC_MAGIC 'X'

#define TAPI_MIN_FLASH                80
#define TAPI_MAX_FLASH               400
#define TAPI_MIN_FLASH_MAKE          200
#define TAPI_MIN_DIGIT_LOW            30
#define TAPI_MAX_DIGIT_LOW            80
#define TAPI_MIN_DIGIT_HIGH           30
#define TAPI_MAX_DIGIT_HIGH           80
#define TAPI_MIN_OFF_HOOK             40
#define TAPI_MIN_ON_HOOK             400
#define TAPI_MIN_INTERDIGIT          300

#ifndef TAPI_SLIC_FAULT_TIMEOUT
   /** Maximum time in milliseconds to wait for a SLIC recovery before
       reporting a SLIC fault.  */
   #define TAPI_SLIC_FAULT_TIMEOUT   250 /* [ms] */
#endif /* IFX_TAPI_EVENT_FIFO_SIZE */

#ifndef TAPI_RING_CADENCE_GRANULARITY
   /* Duration of one bit of the ring cadence in milliseconds. Please note
      that should the timer service have a granularity this should be a
      multiple of this granularity. */
   #define TAPI_RING_CADENCE_GRANULARITY 50 /* ms */
#endif /* TAPI_RING_CADENCE_GRANULARITY */

/** Maximum number of allowed LL drivers */
#ifndef TAPI_MAX_LL_DRIVERS
#define TAPI_MAX_LL_DRIVERS            5
#endif /* TAPI_MAX_LL_DRIVERS */

#ifndef TAPI_TONE_MAXRES
   #ifdef TAPI_VERSION3
        /** Maximum number of tone generators on one channel of any CPE.
            2 on SIG and 1 on DECT */
        #define TAPI_TONE_MAXRES 3
    #else /* !TAPI_VERSION3 */
        /** Maximum tone generators that can run in parallel: 3 modules ALM,
            PCM, COD and CONF and 2 tone generators */
        #define TAPI_TONE_MAXRES 8
    #endif /* TAPI_VERSION3*/
#endif

#ifndef IFX_TAPI_EVENT_POOL_INITIAL_SIZE
/** Initial number of events structures allocated by the driver. If more
    event structures are needed the pool grows automatically in steps of
    IFX_TAPI_EVENT_POOL_GROW_SIZE */
   #define IFX_TAPI_EVENT_POOL_INITIAL_SIZE 70
#endif

#ifndef IFX_TAPI_EVENT_POOL_GROW_SIZE
/** Number of events that the event structure pool grows every time it gets
    depleted. */
   #define IFX_TAPI_EVENT_POOL_GROW_SIZE 70
#endif

#ifndef IFX_TAPI_EVENT_FIFO_SIZE
/** Event Fifo Size */
   #define IFX_TAPI_EVENT_FIFO_SIZE               10
#endif /* IFX_TAPI_EVENT_FIFO_SIZE */

#ifndef IFX_TAPI_EVENT_POOL_GROW_LIMIT
   #ifdef TAPI_VERSION3
      #define IFX_TAPI_EVENT_POOL_GROW_LIMIT           490
   #else /* non TAPI_VERSION3 */
      #define IFX_TAPI_EVENT_POOL_GROW_LIMIT \
         (TAPI_MAX_LL_DRIVERS * IFX_TAPI_EVENT_FIFO_SIZE * 16/* max channels per device */)
   #endif /* TAPI_VERSION3 */
#endif

/* Number of tones for internal driver purposes */
#define TAPI_MAX_RESERVED_TONES       3
/* Maximum number of tones codes, comprising the maximum user tones and the
   maximum internal reserved tones. */
#define TAPI_MAX_TONE_CODE           (1 /*index 0*/ + \
                                      IFX_TAPI_TONE_INDEX_MAX + \
                                      TAPI_MAX_RESERVED_TONES)

/* Used as parameter to IFX_TAPI_Dial_OffhookTime_Override(). */
#define IFX_TAPI_DIAL_TIME_NORMAL   0

/* =================================== */
/* Utility macros                      */
/* =================================== */

/** mark variable as unused, to suppress compilation warrnings

   \remarks: (ANSI X3.159-1989)
      void is used, in any context where the value of an expression
      is to be discarded, to indicate explicitly that a value is
      ignored by writing the cast (void).
*/
#define IFX_UNUSED(var) ((IFX_void_t)(var))

#ifndef ARRAY_SIZE
   #define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif /* ARRAY_SIZE */

/* =================================== */
/* Global typedef forward declarations */
/* =================================== */

typedef struct _TAPI_DEV      TAPI_DEV;
typedef struct _TAPI_CHANNEL  TAPI_CHANNEL;

/* =================================== */
/* Error handling macros               */
/* =================================== */
#define ERROR_CLASS_MASK           (TAPI_statusClassErr |                  \
                                    TAPI_statusClassWarn |                 \
                                    TAPI_statusClassCritical)

/* Note on both RETURN_STATUS() and RETURN_DEVSTATUS():
   "code" may contain an already combined HL and LL error code. In this case
   the "llcode" field is ignored. In all other cases the error code is the
   combination of the HL and LL error code. The "code" field may also contain
   an old style error code (IFX_ERRROR) which is also treated as an error.
   The error is put on the stack if either "code" or "llcode" contain an
   error code. */

#define RETURN_STATUS(code, llcode)                                        \
   /*lint -save -e{506, 572, 774, 778} */                                  \
   do{                                                                     \
      IFX_uint32_t   HLcode, LLcode;                                       \
      if (((IFX_uint32_t)(code)) & 0xFFFF0000)                             \
      {                                                                    \
         HLcode = (((IFX_uint32_t)(code)) & 0xFFFF0000) >> 16;             \
         LLcode = (((IFX_uint32_t)(code)) & 0x0000FFFF);                   \
      }                                                                    \
      else                                                                 \
      {                                                                    \
         HLcode = (((IFX_uint32_t)(code)) & 0x0000FFFF);                   \
         LLcode = (((IFX_uint32_t)(llcode)) & 0x0000FFFF);                 \
      }                                                                    \
      if (((IFX_return_t)(HLcode) == IFX_ERROR) ||                         \
          ((IFX_return_t)(LLcode) == IFX_ERROR) ||                         \
          ((TAPI_statusClass_t)(HLcode) & ERROR_CLASS_MASK) ||             \
          ((TAPI_statusClass_t)(LLcode) & ERROR_CLASS_MASK)   )            \
      {                                                                    \
         TAPI_ErrorStatus (pChannel->pTapiDevice,                          \
                           (TAPI_Status_t)(HLcode), (LLcode),              \
                           __LINE__, __FILE__);                            \
      }                                                                    \
      return   (((IFX_uint32_t)(HLcode) & 0x0000FFFF) << 16) |             \
                ((IFX_uint32_t)(LLcode) & 0x0000FFFF);                     \
   }while(0) /*lint -restore */

#define RETURN_DEVSTATUS(code, llcode)                                     \
   /*lint -save -e{506, 572, 774, 778} */                                  \
   do{                                                                     \
      IFX_uint32_t   HLcode, LLcode;                                       \
      if (((IFX_uint32_t)(code)) & 0xFFFF0000)                             \
      {                                                                    \
         HLcode = (((IFX_uint32_t)(code)) & 0xFFFF0000) >> 16;             \
         LLcode = (((IFX_uint32_t)(code)) & 0x0000FFFF);                   \
      }                                                                    \
      else                                                                 \
      {                                                                    \
         HLcode = (((IFX_uint32_t)(code)) & 0x0000FFFF);                   \
         LLcode = (((IFX_uint32_t)(llcode)) & 0x0000FFFF);                 \
      }                                                                    \
      if (((IFX_return_t)(HLcode) == IFX_ERROR) ||                         \
          ((IFX_return_t)(LLcode) == IFX_ERROR) ||                         \
          ((TAPI_statusClass_t)(HLcode) & ERROR_CLASS_MASK) ||             \
          ((TAPI_statusClass_t)(LLcode) & ERROR_CLASS_MASK)   )            \
      {                                                                    \
         TAPI_ErrorStatus (pTapiDev,                                       \
                           (TAPI_Status_t)(HLcode), (LLcode),              \
                           __LINE__, __FILE__);                            \
      }                                                                    \
      return   (((IFX_uint32_t)(HLcode) & 0x0000FFFF) << 16) |             \
                ((IFX_uint32_t)(LLcode) & 0x0000FFFF);                     \
   }while(0) /*lint -restore */

/* =================================== */
/* Defines for tapi device init state  */
/* =================================== */
/** Device status: nothing initialised */
#define   TAPI_INITSTATUS_UNINITIALISED            0x00000000
/** Device status: PPD procfs entry for the device created. */
#define   TAPI_INITSTATUS_PPD_PROCFS_CREATED       0x00000001

/* ================================= */
/* Enums                             */
/* ================================= */

#ifdef TAPI_FEAT_POLL
/** Defines for tapi operating modes  */
/* Bit 0 is used for packets and bit 1 for events. */
typedef enum _TAPI_WORKING_MODE
{
   /** Interrupt mode for packets */
   TAPI_INTERRUPT_MODE_PACKETS = 0,
   /** Interrupt mode for events */
   TAPI_INTERRUPT_MODE_EVENTS = 0,
   /** Polling mode for packets */
   TAPI_POLLING_MODE_PACKETS = 1,
   /** Polling mode for events */
   TAPI_POLLING_MODE_EVENTS = 2
} TAPI_WORKING_MODE;
#endif /* TAPI_FEAT_POLL */

/* Channel filedescriptor flags */
enum CH_FLAGS
{
   /** indicates a non-blocking read driver's function */
   CF_NONBLOCK             = 0x00000008,
   /** Indicates that a task is pending via select on this device */
   /* Only used in vxWorks */
   CF_NEED_WAKEUP          = 0x00100000,
};

/* =============================== */
/* Further includes                */
/* =============================== */

/* include debugging interface */
#include "drv_tapi_debug.h"

/* include definitions of the low level driver interface which require
   some of the definitions above */
#include "drv_tapi_ll_interface.h"

#ifdef TAPI_FEAT_QOS
   #include "drv_tapi_qos_ll_interface.h"
#endif /* TAPI_FEAT_QOS */

#ifdef TAPI_FEAT_FXO
   #include "drv_tapi_fxo_ll_interface.h"
#endif /* TAPI_FEAT_FXO */

/* Event handling */
#include "drv_tapi_event.h"

/* include for KPI requires the TAPI_CHANNEL so include it here */
#include "drv_tapi_kpi.h"

/* SRTP */
#include "drv_tapi_srtp.h"

/* FXO services */
#include "drv_tapi_fxo.h"

#include "drv_tapi_ppd.h"

#define TAPI_MAX_DEVFS_HANDLES 36

typedef struct _IFX_TAPI_HL_DRV_CTX
{
   /* Pointer to the driver context provided by the LL driver. A value of
      NULL indicates that this struct is unused. */
   IFX_TAPI_DRV_CTX_t *pDrvCtx;
#if defined(LINUX) && defined (__KERNEL__)
   /* buffer to store the registered device driver name, Linux references to
      this string, e.g. on cat /proc/devices - please don't remove */
   IFX_char_t          registeredDrvName[20];
   /* Count of contiguous device numbers requested. */
   unsigned int   nDevNrCount;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,18))
   struct class_device *pLL_Device[TAPI_MAX_DEVFS_HANDLES];
#else
   struct device *pLL_Device[TAPI_MAX_DEVFS_HANDLES];
#endif /* < Linux 2.6.18 */
#endif /* >= Linux 2.6.0 */
#endif /* LINUX */
} IFX_TAPI_HL_DRV_CTX_t;

/* ============================= */
/** Structure for operation control
    \internal */
/* ============================= */
typedef struct
{
   /* interrupt routine save hookstatus here */
   IFX_uint8_t               bHookState;
   /* polarity status from the line; 1 = reversed; 0 = normal */
   IFX_uint8_t               nPolarity;
   /* automatic battery switch; 1 = automatic; 0 = normal */
   IFX_uint8_t               nBatterySw;
   /* last line feed mode; is set after ringing stops */
   IFX_uint8_t               nLineMode;
   /* Last line type; changed when new line type changed with success. */
   IFX_TAPI_LINE_TYPE_t      nLineType;
   /** number of the assigned daa instance (LineType FXO),
       handled by the daa abstraction driver */
   IFX_uint16_t              nDAA;
   /** status of DAA channel initialisation
       (GPIO reservation, etc is done only once) */
   IFX_boolean_t             bDaaInitialized;
   /* set when the fault condition occurs of the device. It will be reseted
      when the line mode is modified */
   IFX_boolean_t             bFaulCond;
   /* Shutdown because of overtemp or ground fault event. */
   IFX_boolean_t             bEmergencyShutdown;
} TAPI_OPCONTROL_DATA_t;

/* ============================= */
/* Structure for ring data       */
/* ============================= */
struct TAPI_RING_DATA;
typedef struct TAPI_RING_DATA TAPI_RING_DATA_t;

/* ============================= */
/* Structure for pcm data        */
/* ============================= */
typedef struct
{
   /* configuration data for pcm services */
   IFX_TAPI_PCM_CFG_t   PCMConfig;
   /* save activation status */
   IFX_boolean_t        bTimeSlotActive;
   /* save configuration status */
   IFX_boolean_t        bCfgSuccess;
}TAPI_PCM_DATA_t;

/* ============================= */
/* Structure for dial data       */
/* ============================= */
struct TAPI_DIAL_DATA;
typedef struct TAPI_DIAL_DATA TAPI_DIAL_DATA_t;

/* ============================= */
/* Structure for PPD data        */
/* ============================= */
struct TAPI_PPD_DATA;
typedef struct TAPI_PPD_DATA TAPI_PPD_DATA_t;

/* ============================= */
/* Structure for metering data   */
/*                               */
/* ============================= */
struct TAPI_METER_DATA;
typedef struct TAPI_METER_DATA TAPI_METER_DATA_t;

/* ============================= */
/* Structure for complex tone    */
/* data                          */
/* ============================= */
struct TAPI_TONE_DATA;
typedef struct TAPI_TONE_DATA TAPI_TONE_DATA_t;

struct TAPI_TONE_RES;
typedef struct TAPI_TONE_RES TAPI_TONE_RES_t;

/* ============================== */
/* Structure for statistic data   */
/*                                */
/* ============================== */
struct TAPI_STAT_DATA;
typedef struct TAPI_STAT_DATA TAPI_STAT_DATA_t;

/* ============================= */
/* channel specific structure    */
/* ============================= */
struct _TAPI_CHANNEL
{
   /* channel number */
   /* ATTENTION, nChannel must be the first element */
   IFX_uint8_t                   nChannel;
   /* pointer to the Low level driver channel */
   IFX_TAPI_LL_CH_t             *pLLChannel;
   /* pointer to the tapi device structure */
   TAPI_DEV                     *pTapiDevice;
   /* semaphore used only in blocking read access,
      in this case given from interrupt context */
   TAPI_OS_event_t               semReadBlock;
   /* wakeup queue for select on read */
   TAPI_OS_drvSelectQueue_t      wqRead;
   /* wakeup queue for select on write */
   TAPI_OS_drvSelectQueue_t      wqWrite;
   /* stores the current fax status */
   volatile IFX_boolean_t        bFaxDataRequest;
   /* flags for different purposes, see CH_FLAGS */
   volatile IFX_uint32_t         nFlags;

   /* In Use counter */
   IFX_uint16_t                  nInUse;

   /* channel is initialized */
   IFX_boolean_t                 bInitialized;

   /* locking semaphore for protecting data */
   TAPI_OS_mutex_t               semTapiChDataLock;

   /* overall channel protection ( read/write/ioctl level)
   PS: Avoid nested locking of this mutex. It can lead to a deadlock */
   TAPI_OS_mutex_t               semTapiChSingleIoctlAccess;

   /* data structures for services */
   TAPI_OPCONTROL_DATA_t         TapiOpControlData;
   IFX_TAPI_EVENT_HANDLER_DATA_t *pEventHandler;
#ifdef TAPI_FEAT_ALM_LEC
   TAPI_LEC_DATA_t               TapiLecAlmData;
#endif /* TAPI_FEAT_ALM_LEC */
#ifdef TAPI_FEAT_PCM_LEC
   TAPI_LEC_DATA_t               TapiLecPcmData;
#endif /* TAPI_FEAT_PCM_LEC */
#ifdef TAPI_FEAT_METERING
   TAPI_METER_DATA_t             *pTapiMeterData;
#endif /* TAPI_FEAT_METERING */
#ifdef TAPI_FEAT_RINGENGINE
   TAPI_RING_DATA_t              *pTapiRingData;
#endif /* TAPI_FEAT_RINGENGINE */
#ifdef TAPI_FEAT_PCM
   TAPI_PCM_DATA_t               TapiPCMData;
#endif /* TAPI_FEAT_PCM */
#ifdef TAPI_FEAT_DIAL
   TAPI_DIAL_DATA_t              *pTapiDialData;
#endif /* TAPI_FEAT_DIAL */
#ifdef TAPI_FEAT_TONEENGINE
   TAPI_TONE_RES_t               *pToneRes;
   /* complex tone data */
   TAPI_TONE_DATA_t              *TapiComplexToneData;
#endif /* TAPI_FEAT_TONEENGINE */
#ifdef TAPI_FEAT_CID
   TAPI_CID_DATA_t               *pTapiCidData;
#endif /* TAPI_FEAT_CID */
#ifdef TAPI_FEAT_PHONE_DETECTION
   TAPI_PPD_DATA_t               *pTapiPpdData;
#endif /* TAPI_FEAT_PHONE_DETECTION */
#ifdef TAPI_FEAT_PACKET
   TAPI_OS_spin_lock_s           sl_up_stream_fifo[IFX_TAPI_STREAM_MAX];
   FIFO_ID*                      pUpStreamFifo[IFX_TAPI_STREAM_MAX];
#endif /* TAPI_FEAT_PACKET */
#ifdef TAPI_FEAT_KPI
   /** Control structure for the Kernel Packet Interface (KPI) */
   IFX_TAPI_KPI_STREAM_SWITCH    pKpiStream[IFX_TAPI_KPI_STREAM_MAX];
#if defined (TAPI_FEAT_SRTP) && defined (TAPI_VERSION3)
   IFX_TAPI_SRTP_DATA_t          *pSrtp;
#endif /* TAPI_FEAT_SRTP && TAPI_VERSION3 */
#endif /* TAPI_FEAT_KPI */
#ifdef EVENT_COUNTER
   /* debugging feature; per channel event sequence number */
   IFX_uint32_t                  nEventSeqNo;
#endif
#ifdef TAPI_FEAT_STATISTICS
   TAPI_STAT_DATA_t              *pTapiStatData;
#endif /* TAPI_FEAT_STATISTICS */
};

/* ============================= */
/* tapi structure                */
/* ============================= */
struct _TAPI_DEV
{
   /* channel number IFX_TAPI_DEVICE_CH_NUMBER indicates the control device */
   /* ATTENTION, nChannel must be the first element */
   IFX_uint8_t               nChannel;
   /* pointer to LL device structure */
   IFX_TAPI_LL_DEV_t        *pLLDev;
   /* link to the device driver context */
   IFX_TAPI_DRV_CTX_t       *pDevDrvCtx;

   /* Number of channels for which memory is allocated in the array below.
     This does not reflect the number of analog, signaling, pcm or coder
     channels. They are reported in the nResource struct below. */
   IFX_uint8_t               nMaxChannel;
   /* array of tapi channel structures */
   TAPI_CHANNEL             *pChannel;
   /* struct with counters how many resources are available and initialised */
   IFX_TAPI_RESOURCE        nResource;

   /** Device ID (unique only for devices of the same device context) */
   IFX_uint32_t             nDev;
   /** Globally unique TAPI device ID [0,1,...] (even across device context) */
   IFX_uint32_t             nDevID;
   /* usage counter, counts the number of open fds */
   IFX_uint16_t              nInUse;
   /* already opened or not */
   IFX_boolean_t             bInitialized;
   /* Flags to remember which parts are initialised. */
   IFX_uint32_t            nInitStatusFlags;

#if defined(LINUX) && defined (__KERNEL__)
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
   /** Linux char device struct */
   struct cdev          cdev;
#endif /* >= Linux 2.6.0 */
#endif /* LINUX */

   /* Event wakeup queue for select, this one is used to report all kind of
      device and channel events to the application. Its something like VxWorks
      version of select() call. */
   TAPI_OS_drvSelectQueue_t  wqEvent;
   /* Additional flag for vxWorks if a real wakeup is required */
   volatile IFX_boolean_t    bNeedWakeup;

   /* overall channel protection (ioctl level)
   PS: Avoid nested locking of this mutex. It can lead to a deadlock */
   TAPI_OS_mutex_t           semTapiDevSingleIoctlAccess;

#ifdef TAPI_FEAT_POLL
   IFX_TAPI_POLL_CONFIG_t *pPollCfg;
   /* Flag for operating mode of tapi (interrupt - default or polling). */
   TAPI_WORKING_MODE             fWorkingMode;
#ifdef TAPI_FEAT_PACKET
   TAPI_OS_spin_lock_s       sl_down_stream_fifo;
   /* Device specific fifo for downstream direction. */
   FIFO_ID*                  pDownStreamFifo;
#endif /* TAPI_FEAT_PACKET */
#endif /* TAPI_FEAT_POLL */

#ifdef TAPI_FEAT_FXO
   /* flag for SmartSLIC - required for special handling of FXO ioctls */
   IFX_boolean_t            bSmartSlicFxo;
#endif /* TAPI_FEAT_FXO */

#ifdef TAPI_FEAT_SSLIC_RECOVERY
   /** timer for SmartSLIC fault supervision */
   Timer_ID                  SlicFaultTimerID;
#endif /* TAPI_FEAT_SSLIC_RECOVERY */

   /** last error code and error stack */
   IFX_TAPI_Error_t         error;

#ifdef EVENT_COUNTER
   /* debugging feature; per device event sequence number */
   IFX_uint32_t             nEventSeqNo;
#endif

   /** Last channel where an event was reported. */
   IFX_uint8_t               nLastEventChannel;
   /* IFX_TRUE if enabled transparent firmware events  */
   IFX_boolean_t             bFwEventsEnabled;
};

/* ============================= */
/* Global variables declaration  */
/* ============================= */
/* Declarations for debug interface */
DECLARE_TRACE_GROUP  (TAPI_DRV);

/* global high level driver context used in system interface only */
extern struct _IFX_TAPI_HL_DRV_CTX gHLDrvCtx [TAPI_MAX_LL_DRIVERS];

extern const IFX_char_t TAPI_WHATVERSION[];
#ifdef HAVE_CONFIG_H
extern const IFX_char_t DRV_TAPI_WHICHCONFIG[];
#endif /* HAVE_CONFIG_H */


/* ======================================== */
/**  Driver identification                  */
/* ======================================== */
extern IFX_int32_t TAPI_Phone_Get_Version (
                        IFX_char_t *pVer);

extern IFX_int32_t TAPI_Phone_Check_Version (
                        IFX_TAPI_VERSION_t const *vers);

extern IFX_int32_t IFX_TAPI_Cap_Nr_Get (
                        TAPI_DEV *pTapiDev,
                        IFX_TAPI_CAP_NR_t *pCap);

/* ======================================== */
/**  Error reporting                        */
/* ======================================== */
extern IFX_int32_t TAPI_Last_Err_Get (
                        TAPI_DEV *pTapiDev,
                        IFX_TAPI_Error_t *pErr);

extern void TAPI_ErrorStatus (
                        TAPI_DEV *pTapiDevice,
                        TAPI_Status_t nHlCode,
                        IFX_int32_t nLlCode,
                        IFX_uint32_t nLine,
                        const IFX_char_t* sFile);

/* ======================================== */
/**  TAPI Initialisation                    */
/* ======================================== */
#ifdef EVENT_LOGGER_DEBUG
extern TAPI_DEV *TAPI_DeviceGetByID(IFX_uint32_t nDevID);
#endif /* EVENT_LOGGER_DEBUG */

extern IFX_int32_t IFX_TAPI_Driver_Start (void);
extern IFX_void_t  IFX_TAPI_Driver_Stop (void);

extern IFX_int32_t IFX_TAPI_Event_On_Driver_Start (void);
extern IFX_void_t  IFX_TAPI_Event_On_Driver_Stop  (void);

extern IFX_return_t TAPI_OS_RegisterLLDrv (
                        IFX_TAPI_DRV_CTX_t *pLLDrvCtx,
                        IFX_TAPI_HL_DRV_CTX_t *pHLDrvCtx);

extern IFX_return_t TAPI_OS_UnregisterLLDrv (
                        IFX_TAPI_DRV_CTX_t *pLLDrvCtx,
                        IFX_TAPI_HL_DRV_CTX_t *pHLDrvCtx);

extern IFX_TAPI_DRV_CTX_t* IFX_TAPI_DeviceDriverContextGet (
                        IFX_int32_t Major);

extern IFX_int32_t IFX_TAPI_DeviceStart (
                        TAPI_DEV *pTapiDev,
                        IFX_TAPI_DEV_START_CFG_t const *pDevStartCfg);

extern IFX_int32_t IFX_TAPI_DeviceStop (
                        TAPI_DEV *pTapiDev);

/* Legacy support for driver initialisation */
extern IFX_int32_t IFX_TAPI_Phone_Init (
                        TAPI_DEV *pTapiDev,
                        IFX_TAPI_CH_INIT_t const *pInit);

/* ======================================== */
/* Analog line services                     */
/* ======================================== */

extern IFX_int32_t TAPI_Phone_Set_LineType (
                        TAPI_CHANNEL *pChannel,
                        IFX_TAPI_LINE_TYPE_CFG_t const *pCfg);

extern IFX_int32_t TAPI_Phone_Set_Linefeed (
                        TAPI_CHANNEL *pChannel,
                        IFX_int32_t nMode);

extern IFX_void_t  TAPI_Phone_Linefeed_Get (
                        TAPI_CHANNEL *pChannel,
                        IFX_uint8_t *nLineMode);

extern IFX_void_t  TAPI_Phone_Change_Linefeed (
                        TAPI_CHANNEL *pChannel,
                        IFX_int32_t nMode);

extern IFX_void_t  TAPI_Phone_Linefeed_Restore (
                        TAPI_CHANNEL *pChannel);

extern IFX_int32_t TAPI_LineBatteryVoltageGet (
                        TAPI_CHANNEL *pChannel,
                        IFX_TAPI_LINE_BATTERY_VOLTAGE_t *pVBat);

extern IFX_int32_t TAPI_LineBatteryVoltageSet (
                        TAPI_CHANNEL *pChannel,
                        IFX_int32_t nVbat);

#ifdef TAPI_FEAT_SSLIC_RECOVERY
extern IFX_void_t  TAPI_Phone_SlicFaultHandling(
                        TAPI_DEV *pTapiDev,
                        IFX_boolean_t bCrashed);

extern IFX_void_t  TAPI_Phone_SlicFaultOnTimer(
                        Timer_ID Timer,
                        IFX_ulong_t nArg);
#endif /* TAPI_FEAT_SSLIC_RECOVERY */

extern IFX_int32_t TAPI_Phone_HookstateGet (
                        TAPI_CHANNEL *pChannel,
                        IFX_TAPI_LINE_HOOK_STATUS_GET_t *pHookMode);

/* ======================================== */
/* Ringing services                         */
/* ======================================== */
#ifdef TAPI_FEAT_RINGENGINE
extern IFX_int32_t IFX_TAPI_Ring_Initialise_Unprot (
                        TAPI_CHANNEL *pChannel);

extern IFX_void_t  IFX_TAPI_Ring_Cleanup (
                        TAPI_CHANNEL *pChannel);

extern IFX_int32_t IFX_TAPI_Ring_Prepare (
                        TAPI_CHANNEL *pChannel);

extern IFX_int32_t IFX_TAPI_Ring_Start (
                        TAPI_CHANNEL *pChannel);

extern IFX_int32_t IFX_TAPI_Ring_Stop (
                        TAPI_CHANNEL *pChannel);

extern IFX_int32_t IFX_TAPI_Ring_DoBlocking (
                        TAPI_CHANNEL *pChannel);

extern IFX_int32_t IFX_TAPI_Ring_IsActive (
                        TAPI_CHANNEL *pChannel);

extern IFX_int32_t IFX_TAPI_Ring_SetCadence (
                        TAPI_CHANNEL *pChannel,
                        IFX_uint32_t nCadence);

extern IFX_int32_t IFX_TAPI_Ring_SetCadenceHighRes (
                        TAPI_CHANNEL *pChannel,
                        IFX_TAPI_RING_CADENCE_t const *pCadence);

extern IFX_int32_t IFX_TAPI_Ring_SetConfig (
                        TAPI_CHANNEL *pChannel,
                        IFX_TAPI_RING_CFG_t const *pRingConfig);

extern IFX_int32_t IFX_TAPI_Ring_GetConfig (
                        TAPI_CHANNEL *pChannel,
                        IFX_TAPI_RING_CFG_t *pRingConfig);

extern IFX_int32_t IFX_TAPI_Ring_SetMaxRings (
                        TAPI_CHANNEL *pChannel,
                        IFX_uint32_t nMaxRings);

extern IFX_int32_t IFX_TAPI_Ring_Engine_Start (
                        TAPI_CHANNEL *pChannel,
                        IFX_boolean_t bStartWithInitial);
#endif /* TAPI_FEAT_RINGENGINE */

#ifdef TAPI_FEAT_CID
extern IFX_int32_t IFX_TAPI_Ring_CalculateRingTiming (
                        TAPI_CHANNEL *pChannel,
                        IFX_uint32_t *pCadenceRingBurst,
                        IFX_uint32_t *pCadenceRingPause);

extern IFX_void_t  IFX_TAPI_Ring_CidCadencePrepare (
                        TAPI_CHANNEL *pChannel,
                        IFX_TAPI_CID_SEQ_CONF_t *pCidData);
#endif /* TAPI_FEAT_CID */

/* ======================================== */
/* Line Echo Cancellation                   */
/* ======================================== */
#ifdef TAPI_FEAT_ALM_LEC
extern IFX_int32_t TAPI_Phone_LecMode_Alm_Set (
                        TAPI_CHANNEL *pChannel,
                        IFX_TAPI_WLEC_CFG_t const *pLecConf);

extern IFX_int32_t TAPI_Phone_LecMode_Alm_Get (
                        TAPI_CHANNEL *pChannel,
                        IFX_TAPI_WLEC_CFG_t *pWLecConf);
#endif /* TAPI_FEAT_ALM_LEC */

#ifdef TAPI_FEAT_PCM_LEC
extern IFX_int32_t TAPI_Phone_LecMode_Pcm_Set (
                        TAPI_CHANNEL *pChannel,
                        IFX_TAPI_WLEC_CFG_t const *pLecConf);

extern IFX_int32_t TAPI_Phone_LecMode_Pcm_Get (
                        TAPI_CHANNEL *pChannel,
                        IFX_TAPI_WLEC_CFG_t *pLecConf);
#endif /* TAPI_FEAT_PCM_LEC */

/* ======================================== */
/* DTMF services                            */
/* ======================================== */
#ifdef TAPI_FEAT_DTMF
extern IFX_int32_t TAPI_Phone_DTMFR_Cfg_Get (
                        TAPI_CHANNEL *pChannel,
                        IFX_TAPI_DTMF_RX_CFG_t *pDtmfRxCoeff);

extern IFX_int32_t TAPI_Phone_DTMFR_Cfg_Set (
                        TAPI_CHANNEL *pChannel,
                        IFX_TAPI_DTMF_RX_CFG_t const *pDtmfRxCoeff);
#endif /* TAPI_FEAT_DTMF */

/* ======================================== */
/* Tone services                            */
/* ======================================== */
#ifdef TAPI_FEAT_TONETABLE
extern IFX_int32_t TAPI_Phone_Tone_Predef_Config (
                        void);

extern IFX_int32_t TAPI_Phone_Tone_TableConf (
                        IFX_TAPI_TONE_t const *pTone);

extern IFX_int32_t TAPI_Phone_Add_SimpleTone (
                        IFX_TAPI_TONE_SIMPLE_t const *pSimpleTone);

extern IFX_int32_t TAPI_Phone_Add_ComposedTone (
                        IFX_TAPI_TONE_COMPOSED_t const *pComposedTone);

extern IFX_uint32_t IFX_TAPI_Tone_DurationGet (
                        IFX_uint32_t nToneIndex);
#endif /* TAPI_FEAT_TONETABLE */

#ifdef TAPI_FEAT_TONEENGINE
extern IFX_int32_t IFX_TAPI_Tone_Initialise_Unprot (
                        TAPI_CHANNEL *pChannel);

extern IFX_void_t  IFX_TAPI_Tone_Cleanup (
                        TAPI_CHANNEL *pChannel);

extern IFX_int32_t TAPI_Phone_Tone_Play (
                        TAPI_CHANNEL *pChannel,
                        IFX_int32_t nToneIndex,
                        TAPI_TONE_DST dst);

extern IFX_int32_t TAPI_Phone_Tone_Play_Unprot (
                        TAPI_CHANNEL *pChannel,
                        IFX_TAPI_TONE_PLAY_t *pTone,
                        IFX_boolean_t bSendEndEvent);

extern IFX_int32_t TAPI_Phone_Tone_Stop (
                        TAPI_CHANNEL *pChannel,
                        IFX_TAPI_TONE_PLAY_t *pTone,
                        TAPI_TONE_DST nDirection);

extern IFX_void_t TAPI_Tone_Step_Completed(
                        TAPI_CHANNEL* pChannel,
                        IFX_uint8_t utgNum);
#endif /* TAPI_FEAT_TONEENGINE */

#ifdef TAPI_FEAT_TONEGEN
extern IFX_int32_t TAPI_Target_Tone_Play (
                        TAPI_CHANNEL *pChannel,
                        IFX_TAPI_TONE_PLAY_t *pTone);

extern IFX_int32_t TAPI_Phone_Tone_Local_Play (
                        TAPI_CHANNEL *pChannel,
                        IFX_int32_t nToneIndex);

extern IFX_int32_t TAPI_Phone_Tone_Local_Stop (
                        TAPI_CHANNEL *pChannel,
                        IFX_int32_t nToneIndex);

extern IFX_int32_t TAPI_Phone_Tone_Net_Play (
                        TAPI_CHANNEL *pChannel,
                        IFX_int32_t nToneIndex);

extern IFX_int32_t TAPI_Phone_Tone_Net_Stop (
                        TAPI_CHANNEL *pChannel,
                        IFX_int32_t nToneIndex);

extern IFX_int32_t TAPI_Phone_Tone_Def_Stop (
                        TAPI_CHANNEL *pChannel,
                        IFX_TAPI_TONE_PLAY_t *pTone);

extern IFX_int32_t TAPI_Phone_Tone_Ringback (
                        TAPI_CHANNEL *pChannel);

extern IFX_int32_t TAPI_Phone_Tone_Busy (
                        TAPI_CHANNEL *pChannel);

extern IFX_int32_t TAPI_Phone_Tone_Dial (
                        TAPI_CHANNEL *pChannel);

extern IFX_int32_t TAPI_Phone_Tone_Set_Level (
                        TAPI_CHANNEL *pChannel,
                        IFX_TAPI_PREDEF_TONE_LEVEL_t const *pToneLevel);
#endif /* TAPI_FEAT_TONEGEN */

#ifdef TAPI_FEAT_DECT
extern IFX_int32_t TAPI_DECT_Tone_Play (
                        TAPI_CHANNEL *pChannel,
                        IFX_int32_t nToneIndex);

extern IFX_int32_t TAPI_DECT_Tone_Stop (
                        TAPI_CHANNEL *pChannel);
#endif /* TAPI_FEAT_DECT */

#ifdef TAPI_FEAT_CPTD
extern IFX_int32_t TAPI_Phone_DetectToneStart (
                        TAPI_CHANNEL *pChannel,
                        IFX_TAPI_TONE_CPTD_t const *signal);

extern IFX_int32_t TAPI_Phone_DetectToneStop (
                        TAPI_CHANNEL *pChannel,
                        IFX_TAPI_TONE_CPTD_t const *signal);
#endif /* TAPI_FEAT_CPTD */

/* ======================================== */
/* Connection services                      */
/* ======================================== */
#ifdef TAPI_FEAT_PCM
extern IFX_int32_t TAPI_Phone_PCM_MapAdd (
                        TAPI_CHANNEL *pChannel,
                        IFX_TAPI_MAP_PCM_t const *pMap);

extern IFX_int32_t TAPI_Phone_PCM_MapRemove (
                        TAPI_CHANNEL *pChannel,
                        IFX_TAPI_MAP_PCM_t const *pMap);
#endif /* TAPI_FEAT_PCM */

#ifdef TAPI_FEAT_VOICE
extern IFX_int32_t TAPI_Data_Channel_Add (
                        TAPI_CHANNEL *pChannel,
                        IFX_TAPI_MAP_DATA_t const *pMap);

extern IFX_int32_t TAPI_Data_Channel_Remove (
                        TAPI_CHANNEL *pChannel,
                        IFX_TAPI_MAP_DATA_t const *pMap);

extern IFX_int32_t TAPI_Phone_MapAdd (
                        TAPI_CHANNEL *pChannel,
                        IFX_TAPI_MAP_PHONE_t const *pMap);

extern IFX_int32_t TAPI_Phone_MapRemove (
                        TAPI_CHANNEL *pChannel,
                        IFX_TAPI_MAP_PHONE_t const *pMap);
#endif /* TAPI_FEAT_VOICE */

#ifdef TAPI_FEAT_CID
extern IFX_int32_t IFX_TAPI_Module_Find_Connected_Data_Channel (
                        TAPI_CHANNEL *pChannel,
                        IFX_TAPI_MAP_TYPE_t nModType,
                        TAPI_CHANNEL **pTapiCh);
#endif /* TAPI_FEAT_CID */

#ifdef TAPI_FEAT_DECT
extern IFX_int32_t TAPI_Phone_DECT_MapAdd (
                        TAPI_CHANNEL *pChannel,
                        IFX_TAPI_MAP_DECT_t const *pMap);

extern IFX_int32_t TAPI_Phone_DECT_MapRemove (
                        TAPI_CHANNEL *pChannel,
                        IFX_TAPI_MAP_DECT_t const *pMap);
#endif /* TAPI_FEAT_DECT */

/* ======================================== */
/* PCM services                             */
/* ======================================== */
#ifdef TAPI_FEAT_PCM
extern IFX_int32_t TAPI_Phone_PCM_IF_Set_Config (
                        TAPI_DEV *pTAPIDev,
                        IFX_TAPI_PCM_IF_CFG_t const *pPCMif);

extern IFX_int32_t TAPI_Phone_PCM_Set_Config (
                        TAPI_CHANNEL *pChannel,
                        IFX_TAPI_PCM_CFG_t const *pPCMConfig);

extern IFX_int32_t TAPI_Phone_PCM_Get_Config (
                        TAPI_CHANNEL *pChannel,
                        IFX_TAPI_PCM_CFG_t *pPCMConfig);

extern IFX_int32_t TAPI_Phone_PCM_Set_Activation(
                        TAPI_CHANNEL *pChannel,
                        IFX_uint32_t nActive);

extern IFX_int32_t TAPI_Phone_PCM_Get_Activation(
                        TAPI_CHANNEL *pChannel,
                        IFX_TAPI_PCM_ACTIVATION_t *pActive);
#ifdef TAPI_FEAT_HDLC
extern IFX_int32_t TAPI_Phone_PCM_HDLC_Set (
                        TAPI_CHANNEL *pChannel,
                        IFX_TAPI_PCM_HDLC_CFG_t const *pHdlcCfg);
#endif /* TAPI_FEAT_HDLC */
extern IFX_int32_t TAPI_Phone_PCM_Loop_Set (
                        TAPI_CHANNEL *pChannel,
                        IFX_TAPI_PCM_LOOP_CFG_t const *pLoopCfg);

extern IFX_int32_t TAPI_Phone_PCM_DEC_HP_Set (
                        TAPI_CHANNEL *pChannel,
                        IFX_uint32_t nHp);
#endif /* TAPI_FEAT_PCM */

/* ======================================== */
/* Pulse dial detection services            */
/* ======================================== */
#ifdef TAPI_FEAT_DIAL
extern IFX_int32_t IFX_TAPI_Dial_Initialise_Unprot (
                        TAPI_CHANNEL *pChannel);

extern IFX_void_t  IFX_TAPI_Dial_Cleanup (
                        TAPI_CHANNEL *pChannel);

extern IFX_int32_t IFX_TAPI_Dial_SetValidationTime (
                        TAPI_CHANNEL *pChannel,
                        IFX_TAPI_LINE_HOOK_VT_t const *pTime);

extern IFX_void_t  IFX_TAPI_Dial_OffhookTime_Override(
                        TAPI_CHANNEL *pChannel,
                        IFX_uint32_t const nTime);

extern IFX_int32_t IFX_TAPI_Dial_CfgApply (
                        TAPI_CHANNEL *pChannel);
#endif /* TAPI_FEAT_DIAL */

#ifdef TAPI_FEAT_DIALENGINE
extern IFX_void_t  IFX_TAPI_Dial_HookEvent (
                        TAPI_CHANNEL * pTapiCh,
                        IFX_boolean_t bHookState,
                        IFX_uint16_t nTime);

extern IFX_void_t  IFX_TAPI_Dial_HookSet (
                        TAPI_CHANNEL *pChannel,
                        IFX_boolean_t bHookState);
#endif /* TAPI_FEAT_DIALENGINE */

/* ======================================== */
/* Metering services                        */
/* ======================================== */
#ifdef TAPI_FEAT_METERING
extern IFX_int32_t IFX_TAPI_Meter_Initialise_Unprot (
                        TAPI_CHANNEL *pChannel);

extern IFX_void_t  IFX_TAPI_Meter_Cleanup (
                        TAPI_CHANNEL *pChannel);

extern IFX_boolean_t TAPI_Phone_Meter_IsActive (
                        TAPI_CHANNEL *pChannel);

extern IFX_int32_t TAPI_Phone_Meter_Config (
                        TAPI_CHANNEL *pChannel,
                        IFX_TAPI_METER_CFG_t const *pMeterConfig);

extern IFX_int32_t TAPI_Phone_Meter_Start (
                        TAPI_CHANNEL *pChannel,
                        IFX_TAPI_METER_START_t const *pMeterStart);

extern IFX_int32_t TAPI_Phone_Meter_Stop (
                        TAPI_CHANNEL *pChannel);

extern IFX_int32_t TAPI_Phone_Meter_Burst (
                        TAPI_CHANNEL *pChannel,
                        IFX_TAPI_METER_BURST_t const *pMeterBurst);

extern IFX_int32_t TAPI_Phone_Meter_Stat (
                        TAPI_CHANNEL *pChannel,
                        IFX_TAPI_METER_STATISTICS_t *pMeterStat);

extern IFX_void_t IFX_TAPI_Meter_EventServe (
                        TAPI_CHANNEL *pChannel);
#endif /* TAPI_FEAT_METERING */

/* ======================================== */
/* Codec services                           */
/* ======================================== */
#ifdef TAPI_FEAT_VOICE
extern IFX_int32_t TAPI_COD_ENC_CFG_Set (
                        TAPI_CHANNEL *pChannel,
                        IFX_TAPI_ENC_CFG_SET_t const *pEncCfg);

extern IFX_int32_t TAPI_COD_ENC_Hold (
                        TAPI_CHANNEL *pChannel,
                        IFX_int32_t nOnHold);

extern IFX_int32_t TAPI_COD_DEC_HP_Set (
                        TAPI_CHANNEL *pChannel,
                        IFX_int32_t nHp);
#endif /* TAPI_FEAT_VOICE */

#ifdef TAPI_FEAT_BABYPHONE
extern IFX_int32_t TAPI_COD_ENC_Room_Noise_Start (
                        TAPI_CHANNEL *pChannel,
                        IFX_TAPI_ENC_ROOM_NOISE_DETECT_t const *pDet);

extern IFX_int32_t TAPI_COD_ENC_Room_Noise_Stop (
                        TAPI_CHANNEL *pChannel);
#endif /* TAPI_FEAT_BABYPHONE */

#ifdef TAPI_FEAT_AMR
extern IFX_int32_t IFX_TAPI_COD_AMR_Get(
                        TAPI_CHANNEL *pChannel,
                        IFX_TAPI_COD_AMR_t *pTapiAMR);
#endif /* TAPI_FEAT_AMR */

/* ======================================== */
/* DECT services                            */
/* ======================================== */
#ifdef TAPI_FEAT_DECT
extern IFX_int32_t IFX_TAPI_DECT_Cfg_Set (
                        TAPI_CHANNEL *pChannel,
                        IFX_TAPI_DECT_CFG_t const *pDect);

extern IFX_int32_t IFX_TAPI_DECT_Enc_Cfg_Set (
                        TAPI_CHANNEL *pChannel,
                        IFX_TAPI_DECT_ENC_CFG_t const *pDectEnc);

extern IFX_int32_t IFX_TAPI_DECT_Activation_Set (
                        TAPI_CHANNEL *pChannel,
                        IFX_uint32_t nEnable);
#endif /* TAPI_FEAT_DECT */

/* ======================================== */
/* Statistic services                       */
/* ======================================== */
#ifdef TAPI_FEAT_STATISTICS
extern IFX_int32_t IFX_TAPI_Stat_Initialise_Unprot (
                        TAPI_CHANNEL *pChannel);

extern IFX_void_t  IFX_TAPI_Stat_Cleanup (
                        TAPI_CHANNEL *pChannel);

extern IFX_void_t  IFX_TAPI_Stat_Reset (
                        TAPI_CHANNEL *pChannel);

extern IFX_uint32_t IFX_TAPI_Stat_Get (
                        TAPI_CHANNEL *pChannel,
                        IFX_TAPI_STREAM_t stream,
                        TAPI_STAT_COUNTER_t counter);
#endif /* TAPI_FEAT_STATISTICS */

/* ======================================== */
/* RTP event packet generation              */
/* ======================================== */
#ifdef TAPI_FEAT_RTP_OOB
extern IFX_int32_t TAPI_EVENT_PKT_EV_Generate (
                        TAPI_CHANNEL *pChannel,
                        IFX_TAPI_PKT_EV_GENERATE_t const *pPacketEvent);

extern IFX_int32_t TAPI_EVENT_PKT_EV_Oob_Dtmf_Set (
                        TAPI_CHANNEL *pChannel,
                        IFX_uint32_t nOobMode);

extern IFX_int32_t TAPI_EVENT_PKT_EV_Oob_Mftd_Set (
                        TAPI_CHANNEL *pChannel,
                        IFX_TAPI_PKT_EV_OOB_MFTD_t const *pOob);
#endif /* TAPI_FEAT_RTP_OOB */

/* ======================================== */
/* Packet transport services                */
/* ======================================== */
#ifdef TAPI_FEAT_PACKET
extern IFX_int32_t TAPI_PKT_Flush (
                        TAPI_CHANNEL* pChannel);
#endif /* TAPI_FEAT_PACKET */

/* ======================================== */
/* FAX T.38 service                         */
/* ======================================== */
#ifdef TAPI_FEAT_FAX_T38
extern IFX_void_t TAPI_FaxT38_Event_Update (
                        TAPI_CHANNEL *pChannel,
                        IFX_uint8_t status,
                        IFX_uint8_t error);
#endif /* TAPI_FEAT_FAX_T38 */

/* ======================================== */
/* FXS Phone Detection                      */
/* ======================================== */
#ifdef TAPI_FEAT_PHONE_DETECTION
extern IFX_int32_t IFX_TAPI_PPD_Cfg_Get (
                        TAPI_CHANNEL *pChannel,
                        IFX_TAPI_LINE_PHONE_DETECT_CFG_t *pPpdConf);

extern IFX_int32_t IFX_TAPI_PPD_Cfg_Set (
                        TAPI_CHANNEL *pChannel,
                        IFX_TAPI_LINE_PHONE_DETECT_CFG_t const *pPpdConf);
#endif /* TAPI_FEAT_PHONE_DETECTION */

/* ======================================== */
/* SRTP                                     */
/* ======================================== */
#ifdef TAPI_FEAT_SRTP
IFX_int32_t IFX_TAPI_SRTP_Init (TAPI_CHANNEL *pChannel);
IFX_int32_t IFX_TAPI_SRTP_CfgSet (TAPI_CHANNEL *pChannel, IFX_TAPI_PKT_SRTP_CFG_t const *pSrtpCfg);
IFX_int32_t IFX_TAPI_SRTP_MKI_Set (TAPI_CHANNEL *pChannel, IFX_TAPI_PKT_SRTP_MKI_t const *pData);
IFX_int32_t IFX_TAPI_SRTP_MKI_CfgSet (TAPI_CHANNEL *pChannel, IFX_TAPI_PKT_SRTP_MKI_CFG_t const *pData);
IFX_int32_t IFX_TAPI_SRTP_CapGet (TAPI_DEV *pTapiDev, IFX_TAPI_PKT_SRTP_CAP_GET_t *pData);
IFX_int32_t IFX_TAPI_SRTP_StatGet (TAPI_CHANNEL *pChannel, IFX_TAPI_PKT_SRTP_STATISTICS_GET_t *pData);
IFX_int32_t IFX_TAPI_SRTP_StatReset (TAPI_CHANNEL *pChannel, IFX_TAPI_PKT_SRTP_STATISTICS_RESET_t const *pData);
IFX_int32_t IFX_TAPI_SRTP_PckIdxSet (TAPI_CHANNEL *pChannel, IFX_TAPI_PKT_SRTP_PACKET_INDEX_SET_t const *pData);
IFX_int32_t IFX_TAPI_SRTP_PckIdxGet (TAPI_CHANNEL *pChannel, IFX_TAPI_PKT_SRTP_PACKET_INDEX_GET_t *pData);
extern IFX_int32_t IFX_TAPI_SRTP_Initialise_Unprot (TAPI_CHANNEL *pChannel);
extern IFX_void_t  IFX_TAPI_SRTP_Cleanup (TAPI_CHANNEL *pChannel);
#endif /* TAPI_FEAT_SRTP */

/* ======================================== */
/**  Other functions                        */
/* ======================================== */
extern IFX_boolean_t ptr_chk(
                        const IFX_void_t /* const */ *ptr,
                        const IFX_char_t *pPtrName);

extern IFX_int32_t TAPI_DeferWork (
                        IFX_void_t* pFunc,
                        IFX_void_t* pParam);

extern IFX_int32_t TAPI_Test_Hook_Gen (
                        TAPI_CHANNEL *pChannel,
                        IFX_uint32_t nHook);

#endif  /* DRV_TAPI_H */
