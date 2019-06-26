#ifndef _DRV_MEI_CPE_CLEAR_EOC_H
#define _DRV_MEI_CPE_CLEAR_EOC_H
/******************************************************************************

                          Copyright (c) 2007-2015
                     Lantiq Beteiligungs-GmbH & Co. KG

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/* ==========================================================================
   Description : Internal structures and definitions for Clear EOC.
   ========================================================================== */

/* ==========================================================================
   includes
   ========================================================================== */

/* get at first the driver configuration */
#include "drv_mei_cpe_config.h"

#if (MEI_DRV_CLEAR_EOC_ENABLE == 1)

#include "ifx_types.h"
#include "drv_mei_cpe_os.h"
#include "drv_mei_cpe_interface.h"

/* ==========================================================================
   Macro defs - VRX Clear EOC common and constants
   ========================================================================== */

#if defined (__GNUC__) || defined (__GNUG__)
   /* GNU C or C++ compiler */
   #define __PACKED__ __attribute__ ((packed))
#else
   /* Byte alignment adjustment */
   #pragma pack(2)
   #if !defined (_PACKED_)
      #define __PACKED__      /* nothing */
   #endif
#endif

/* ==========================================================================
   Macro defs - VRX Clear EOC constants
   ========================================================================== */

/** Max timeout for Clear EOC frames transmit done */
#define MEI_CEOC_FRAME_TRANS_DONE_TIMEOUT_MS           1000

/** Min poll time for checking Clear EOC state changes */
#define MEI_CEOC_MIN_STATE_CHANGE_POLL_TIME_MS         100

/** Max poll count for request the status */
#define MEI_CEOC_MAX_STATUS_POLL_COUNT \
            (MEI_CEOC_FRAME_TRANS_DONE_TIMEOUT_MS/MEI_CEOC_MIN_STATE_CHANGE_POLL_TIME_MS)


/** max number of modem messages to transfer the frame */
#define MEI_CEOC_MAX_MSG_PER_FRAME                     2

/** Raw EOC data block - max size in byte */
#define MEI_CEOC_RAW_EOC_DATA_SIZE_BYTE                MEI_MAX_CEOC_DATA_SIZE_BYTE
/** Raw EOC data block - max size in 16 bit */
#define MEI_CEOC_RAW_EOC_DATA_SIZE_16BIT               (MEI_CEOC_RAW_EOC_DATA_SIZE_BYTE / 2)



/* ==========================================================================
   Macro defs - VRX Clear EOC control macros
   ========================================================================== */

/** get the current timeout counter */
#define MEI_CEOC_GET_TIMEOUT_CNT(p_ceoc_dev_cntrl) \
                              (p_ceoc_dev_cntrl)->timeoutCount

/** decrement the current timeout counter */
#define MEI_CEOC_DEC_TIMEOUT_CNT(p_ceoc_dev_cntrl) \
                              (p_ceoc_dev_cntrl)->timeoutCount--

/** Set the current timeout counter. */
#define MEI_CEOC_SET_TIMEOUT_CNT(p_ceoc_dev_cntrl, newVal) \
                              (p_ceoc_dev_cntrl)->timeoutCount = (IFX_uint32_t)(newVal)

/**
   Get unique access to the Clear EOC control struct
*/
#define MEI_CEOC_GET_UNIQUE_ACCESS(p_ceoc_dev_cntrl) \
               do { \
                  MEI_DRVOS_SemaphoreLock(&(p_ceoc_dev_cntrl)->pDevCEocCtrlRWlock); \
               } while(0)

/**
   Release unique access to the Clear EOC control struct
*/
#define MEI_CEOC_RELEASE_UNIQUE_ACCESS(p_ceoc_dev_cntrl) \
               do { \
                  MEI_DRVOS_SemaphoreUnlock(&(p_ceoc_dev_cntrl)->pDevCEocCtrlRWlock); \
               } while(0)


/** Clear EOC driver config state - set */
#define MEI_CEOC_CFG_STATE_SET(p_ceoc_dev_cntrl, newstate) \
               (p_ceoc_dev_cntrl)->eCfgState = newstate
/** Clear EOC driver config state - get */
#define MEI_CEOC_CFG_STATE_GET(p_ceoc_dev_cntrl) (p_ceoc_dev_cntrl)->eCfgState

/** Clear EOC VRX TX buffer state state - set */
#define MEI_CEOC_TX_DEV_BUF_STATE_SET(p_ceoc_dev_cntrl, newstate) \
               (p_ceoc_dev_cntrl)->eTxDevBufState = newstate

/** Clear EOC VRX TX buffer state state - get */
#define MEI_CEOC_TX_DEV_BUF_STATE_GET(p_ceoc_dev_cntrl) (p_ceoc_dev_cntrl)->eTxDevBufState

/** Clear EOC VRX RX buffer state state - set */
#define MEI_CEOC_RX_DEV_BUF_STATE_SET(p_ceoc_dev_cntrl, newstate) \
               (p_ceoc_dev_cntrl)->eRxDevBufState = newstate

/** Clear EOC VRX RX buffer state state - get */
#define MEI_CEOC_RX_DEV_BUF_STATE_GET(p_ceoc_dev_cntrl) (p_ceoc_dev_cntrl)->eRxDevBufState

/** check transparent mode - event status */
#define MEI_CEOC_CHECK_TRANSMODE_EVT_STAT(p_ceoc_dev_cntrl) \
            (((p_ceoc_dev_cntrl)->cEocTransMode & \
              (MEI_CEOC_INIT_TRANS_MODE_EVT_RX_STAT | MEI_CEOC_INIT_TRANS_MODE_EVT_TX_STAT)) ? IFX_TRUE : IFX_FALSE)

/** check transparent mode - event read data */
#define MEI_CEOC_CHECK_TRANSMODE_EVT_DATA(p_ceoc_dev_cntrl) \
            (((p_ceoc_dev_cntrl)->cEocTransMode & MEI_CEOC_INIT_TRANS_MODE_EVT_DATA) ? IFX_TRUE : IFX_FALSE)


/* ==========================================================================
   Macro defs - VRX Clear EOC statistics
   ========================================================================== */
#if ( (MEI_SUPPORT_STATISTICS == 1) && (MEI_SUPPORT_CEOC_STATISTICS == 1))

#define MEI_CEOC_STAT_INC_WR_MSG_CNT(p_ceoc_dev_cntrl) \
                              (p_ceoc_dev_cntrl)->statistics.wrMsgCnt++
#define MEI_CEOC_STAT_INC_WR_MSG_ERR_CNT(p_ceoc_dev_cntrl) \
                              (p_ceoc_dev_cntrl)->statistics.wrMsgErrCnt++

#define MEI_CEOC_STAT_INC_RD_MSG_CNT(p_ceoc_dev_cntrl) \
                              (p_ceoc_dev_cntrl)->statistics.rdMsgCnt++
#define MEI_CEOC_STAT_INC_RD_MSG_ERR_CNT(p_ceoc_dev_cntrl) \
                              (p_ceoc_dev_cntrl)->statistics.rdMsgErrCnt++

#define MEI_CEOC_STAT_INC_EVT_RD_MSG_CNT(p_ceoc_dev_cntrl) \
                              (p_ceoc_dev_cntrl)->statistics.evtRdMsgCnt++
#define MEI_CEOC_STAT_INC_EVT_RD_MSG_ERR_CNT(p_ceoc_dev_cntrl) \
                              (p_ceoc_dev_cntrl)->statistics.evtRdMsgErrCnt++

#define MEI_CEOC_STAT_INC_WR_FRAME_CNT(p_ceoc_dev_cntrl) \
                              (p_ceoc_dev_cntrl)->statistics.wrFrameCnt++
#define MEI_CEOC_STAT_INC_RD_FRAME_CNT(p_ceoc_dev_cntrl) \
                              (p_ceoc_dev_cntrl)->statistics.rdFrameCnt++
#define MEI_CEOC_STAT_INC_EVT_RD_FRAME_CNT(p_ceoc_dev_cntrl) \
                              (p_ceoc_dev_cntrl)->statistics.evtRdFrameCnt++
#define MEI_CEOC_STAT_INC_RECV_FRAME_ERR_CNT(p_ceoc_dev_cntrl) \
                              (p_ceoc_dev_cntrl)->statistics.recvFrameErrCnt++

#define MEI_CEOC_STAT_INC_SEND_TX_TRIGGER(p_ceoc_dev_cntrl) \
                              (p_ceoc_dev_cntrl)->statistics.sendTxTriggerCnt++

#define MEI_CEOC_STAT_INC_INST_TIMEOUT(p_ceoc_dev_cntrl) \
                              (p_ceoc_dev_cntrl)->statistics.insertTimeoutCnt++

#define MEI_CEOC_STAT_INC_RX_STAT_ERR(p_ceoc_dev_cntrl) \
                              (p_ceoc_dev_cntrl)->statistics.rxStatErrCnt++
#define MEI_CEOC_STAT_INC_TX_STAT_ERR(p_ceoc_dev_cntrl) \
                              (p_ceoc_dev_cntrl)->statistics.txStatErrCnt++

#else

#define MEI_CEOC_STAT_INC_WR_MSG_CNT(p_ceoc_dev_cntrl)
#define MEI_CEOC_STAT_INC_WR_MSG_ERR_CNT(p_ceoc_dev_cntrl)
#define MEI_CEOC_STAT_INC_RD_MSG_CNT(p_ceoc_dev_cntrl)
#define MEI_CEOC_STAT_INC_RD_MSG_ERR_CNT(p_ceoc_dev_cntrl)
#define MEI_CEOC_STAT_INC_EVT_RD_MSG_CNT(p_ceoc_dev_cntrl)
#define MEI_CEOC_STAT_INC_EVT_RD_MSG_ERR_CNT(p_ceoc_dev_cntrl)
#define MEI_CEOC_STAT_INC_WR_FRAME_CNT(p_ceoc_dev_cntrl)
#define MEI_CEOC_STAT_INC_RD_FRAME_CNT(p_ceoc_dev_cntrl)
#define MEI_CEOC_STAT_INC_EVT_RD_FRAME_CNT(p_ceoc_dev_cntrl)
#define MEI_CEOC_STAT_INC_RECV_FRAME_ERR_CNT(p_ceoc_dev_cntrl)
#define MEI_CEOC_STAT_INC_SEND_TX_TRIGGER(p_ceoc_dev_cntrl)
#define MEI_CEOC_STAT_INC_INST_TIMEOUT(p_ceoc_dev_cntrl)
#define MEI_CEOC_STAT_INC_RX_STAT_ERR(p_ceoc_dev_cntrl)
#define MEI_CEOC_STAT_INC_TX_STAT_ERR(p_ceoc_dev_cntrl)

#endif      /* #if ( (MEI_SUPPORT_STATISTICS == 1) && (MEI_SUPPORT_CEOC_STATISTICS == 1)) */


/* ==========================================================================
   typedefs - VRX Clear EOC
   ========================================================================== */

/**
   State for the VRX Clear EOC configuration.
*/
typedef enum
{
   /** initial state after startup, first enable */
   eMEI_CEOC_OP_CFG_INITIAL,
   /** the VRX Clear EOC configuration is valid */
   eMEI_CEOC_OP_CFG_VALID,
   /** the VRX Clear EOC configuration is valid and written to the device */
   eMEI_CEOC_OP_CFG_WRITTEN
} MEI_CEOC_OP_CFG_E;


/**
   State for the Clear EOC VRX TX buffer.
*/
typedef enum
{
   /** Clear EOC device TX buffer is IDLE       - free for next message */
   eMEI_CEOC_TX_DEV_BUF_STATE_IDLE = 0,
   /** Clear EOC device TX buffer is PROGRESS   - transmission in progress */
   eMEI_CEOC_TX_DEV_BUF_STATE_IN_PROGRESS = 1,
   /** Clear EOC device TX buffer is DONE       - transmissinion done (Evt only) */
   eMEI_CEOC_TX_DEV_BUF_STATE_DONE = 2,
   /** Clear EOC device TX buffer is DONE       - not used for CMD status */
   eMEI_CEOC_TX_DEV_BUF_STATE_RESV = 2,
   /** Clear EOC device TX buffer is ERROR      - transmission failed */
   eMEI_CEOC_TX_DEV_BUF_STATE_ERROR = 3
} MEI_CEOC_TX_BUF_STATE_E;

/**
   State for the Clear EOC VRX RX buffer.
*/
typedef enum
{
   /** Clear EOC device RX buffer is IDLE       - no data available */
   eMEI_CEOC_RX_DEV_BUF_STATE_IDLE = 0,
   /** Clear EOC device RX buffer is PROGRESS   - receive in progress */
   eMEI_CEOC_RX_DEV_BUF_STATE_IN_PROGRESS = 1,
   /** Clear EOC device RX buffer is DONE       - data available */
   eMEI_CEOC_RX_DEV_BUF_STATE_DONE = 2,
   /** Clear EOC device RX buffer is ERROR      - receive failed */
   eMEI_CEOC_RX_DEV_BUF_STATE_ERROR = 3
} MEI_CEOC_RX_BUF_STATE_E;


#if ((MEI_SUPPORT_STATISTICS == 1) && (MEI_SUPPORT_CEOC_STATISTICS == 1))
/**
   VRX ATM OAM statistic informations
*/
typedef struct vrx_ceoc_stats_s
{
   /** number of written CEOC modem "write" messages */
   IFX_uint32_t wrMsgCnt;
   /** number of error written CEOC modem "write" messages */
   IFX_uint32_t wrMsgErrCnt;

   /** number of read CEOC modem "read" messages */
   IFX_uint32_t rdMsgCnt;
   /** number of error read CEOC modem "read" messages */
   IFX_uint32_t rdMsgErrCnt;

   /** number of read CEOC modem "event read" messages */
   IFX_uint32_t evtRdMsgCnt;
   /** number of error read CEOC modem "event read" messages */
   IFX_uint32_t evtRdMsgErrCnt;

   /** number of written CEOC frames */
   IFX_uint32_t wrFrameCnt;
   /** number of read CEOC frames */
   IFX_uint32_t rdFrameCnt;
   /** number of event read CEOC frames */
   IFX_uint32_t evtRdFrameCnt;
   /** number of received corrupted CEOC frames */
   IFX_uint32_t recvFrameErrCnt;

   /** number of TX trigger send */
   IFX_uint32_t sendTxTriggerCnt;

   /** number of timeout after TX trigger send (timeout insert frame) */
   IFX_uint32_t insertTimeoutCnt;

   /** number of RX status error */
   IFX_uint32_t rxStatErrCnt;
   /** number of TX status error */
   IFX_uint32_t txStatErrCnt;

} MEI_CEOC_DRV_STATS_T;
#endif      /* #if ( (MEI_SUPPORT_STATISTICS == 1) && (MEI_SUPPORT_CEOC_STATISTICS == 1)) */


/**
   Union for the Clear EOC raw data block.
*/
union vrx_ceoc_raw_eoc_data_u
{
   /** Clear EOC data block [byte] */
   IFX_uint8_t    d_8[MEI_CEOC_RAW_EOC_DATA_SIZE_BYTE];
   /** Clear EOC data block [16 bit]*/
   IFX_uint16_t   d_16[MEI_CEOC_RAW_EOC_DATA_SIZE_16BIT];
} __PACKED__ ;

typedef union vrx_ceoc_raw_eoc_data_u      MEI_CEOC_RAW_EOC_DATA_T;

/**
   Basic Clear EOC frame used by the VRX firmware.
*/
typedef struct vrx_ceoc_vrx_eoc_frame_s
{
   /** internal - VRX driver message id */
   IFX_uint32_t   cEocId;
   /** lenght of the protocol identifier + data block [byte] */
   IFX_uint16_t   length_byte;
   /** Protocol identifier (default value: 814CH, SNMP) */
   IFX_uint16_t   protIdent;
   /** Clear EOC data block */
   MEI_CEOC_RAW_EOC_DATA_T cEocRawData;
} MEI_CEOC_MEI_EOC_FRAME_T;

/**
   VRX CEOC frame buffer
*/
typedef struct vrx_ceoc_frame_buffer_s
{
   /** frame size: raw frame size + sizeof(protIdent) + sizeof(length_byte) + padding*/
   IFX_uint32_t                 frameSize_byte;
   /** already transfered size [bytes] */
   IFX_uint32_t                 transferedSize_byte;
   /** EOC frame struct */
   MEI_CEOC_MEI_EOC_FRAME_T   vrxEocFrame;
} MEI_CEOC_FRAME_BUFFER_T;


/**
   VRX Clear EOC - data for Clear EOC insert extract.
*/
typedef struct vrx_ceoc_dev_cntrl_s
{
   /** confifure the operation mode for the Clear EOC part */
   IFX_uint32_t               opMode;
   /** config device - enable Event RX: autonomous msg data */
   IFX_boolean_t              bEnEvtRxData;
   /** config device - enable Event RX: idle to message available */
   IFX_boolean_t              bEnEvtRxStatus;
   /** config device - enable Event TX: transmission in progress to idle */
   IFX_boolean_t              bEnEvtTxStatus;


   /** current VRX TX device buffer state */
   MEI_CEOC_TX_BUF_STATE_E  eTxDevBufState;
   /** current VRX RX device buffer state */
   MEI_CEOC_RX_BUF_STATE_E  eRxDevBufState;

   /** config driver - transparent mode for incoming event messages */
   IFX_uint32_t               cEocTransMode;

   /** current configuration has been witten to the device */
   MEI_CEOC_OP_CFG_E        eCfgState;

   /** Clear EOC cntrl access semaphore */
   MEI_DRVOS_sema_t           pDevCEocCtrlRWlock;
   /** wakeup flag for Clear EOC state change */
   IFX_boolean_t              bCEocStateChangeNeedWakeUp;
   /** signal EOC state change */
   MEI_DRVOS_event_t          eventCEocStateChange;
   /** timeout count for TX done event */
   IFX_uint32_t               timeoutCount;

   /** transmit buffer for the EOC frame */
   MEI_CEOC_FRAME_BUFFER_T  txEocFrame;
   /** receive buffer for the EOC frame */
   MEI_CEOC_FRAME_BUFFER_T  rxEocFrame;

#if ( (MEI_SUPPORT_STATISTICS == 1) && (MEI_SUPPORT_CEOC_STATISTICS == 1))
   /** statistics for Clear EOC insert extract */
   MEI_CEOC_DRV_STATS_T     statistics;
#endif

} MEI_CEOC_DEV_CNTRL_T;


/* ==========================================================================
   ATM OAM Global Variables.
   ========================================================================== */

MEI_DRV_PRN_USR_MODULE_DECL(MEI_CEOC);
MEI_DRV_PRN_INT_MODULE_DECL(MEI_CEOC);

#endif      /* #if (MEI_DRV_CLEAR_EOC_ENABLE == 1) */

#endif      /* #ifndef _DRV_MEI_CPE_CLEAR_EOC_H */

