#ifndef _DRV_MEI_CPE_ATMOAM_H
#define _DRV_MEI_CPE_ATMOAM_H
/******************************************************************************

                          Copyright (c) 2007-2015
                     Lantiq Beteiligungs-GmbH & Co. KG

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/* ==========================================================================
   Description : Internal structures and definitions for ATM OAM.
   ========================================================================== */

/* ==========================================================================
   includes
   ========================================================================== */

/* get at first the driver configuration */
#include "drv_mei_cpe_config.h"

#if (MEI_DRV_ATM_OAM_ENABLE == 1)

#include "ifx_types.h"
#include "drv_mei_cpe_os.h"
#include "drv_mei_cpe_interface.h"

/* ==========================================================================
   Macro defs - VRX ATM OAM control
   ========================================================================== */

/** Max timeout for ATM cell transmit done */
#define MEI_ATMOAM_CELL_TRANS_DONE_TIMEOUT_MS          1000

/** Min poll time for checking ATM cell transmit done */
#define MEI_ATMOAM_MIN_TXBUFFER_POLL_TIME_MS           10

/** Max Poll retry count for checking the tx insert buffer */
#define MEI_ATMOAM_MAX_INSSTATUS_POLL_RETRY            2

/** Max Transmit cell count */
#define MEI_ATMOAM_MAX_TX_CELL_CNT                     2

/** Max Receive cell count */
#define MEI_ATMOAM_MAX_RX_CELL_CNT                     4

/** get the current timeout counter */
#define MEI_ATMOAM_GET_TX_TIMEOUT_CNT(pAtmOamDevCntrl) \
                              (pAtmOamDevCntrl)->timeoutCount

/** decrement the current timeout counter */
#define MEI_ATMOAM_DEC_TX_TIMEOUT_CNT(pAtmOamDevCntrl) \
                              (pAtmOamDevCntrl)->timeoutCount--

/** Set the current timeout counter. */
#define MEI_ATMOAM_SET_TX_TIMEOUT_CNT(pAtmOamDevCntrl, newVal) \
                              (pAtmOamDevCntrl)->timeoutCount = (IFX_uint32_t)(newVal)

/** check transparent mode - alarm extract */
#define MEI_ATMOAM_CHECK_TRANSMODE_ALM_EXTR(pAtmOamDevCntrl) \
            (((pAtmOamDevCntrl)->atmOamTransMode & MEI_ATMOAM_INIT_TRANS_MODE_ALM_EXTR) ? IFX_TRUE : IFX_FALSE)

/** check transparent mode - cell transmit done */
#define MEI_ATMOAM_CHECK_TRANSMODE_EVT_TX(pAtmOamDevCntrl) \
            (((pAtmOamDevCntrl)->atmOamTransMode & MEI_ATMOAM_INIT_TRANS_MODE_EVT_TX) ? IFX_TRUE : IFX_FALSE)

/** check transparent mode - cell extract */
#define MEI_ATMOAM_CHECK_TRANSMODE_CELL_EXTR(pAtmOamDevCntrl) \
            (((pAtmOamDevCntrl)->atmOamTransMode & MEI_ATMOAM_INIT_TRANS_MODE_CELL_EXTR) ? IFX_TRUE : IFX_FALSE)

/**
   Get unique access to the ATM OAM control struct
*/
#define MEI_ATMOAM_GET_UNIQUE_ACCESS(pAtmOamDevCntrl) \
               do { \
                  MEI_DRVOS_SemaphoreLock(&pAtmOamDevCntrl->pDevAtmOamCtrlRWlock); \
               } while(0)

/**
   Release unique access to the ATM OAM control struct
*/
#define MEI_ATMOAM_RELEASE_UNIQUE_ACCESS(pAtmOamDevCntrl) \
               do { \
                  MEI_DRVOS_SemaphoreUnlock(&pAtmOamDevCntrl->pDevAtmOamCtrlRWlock); \
               } while(0)


/** set the VRX drv state - ATM OAM TxBuf */
#define MEI_ATMOAM_CFG_STATE_SET(pAtmOamDevCntrl, newstate) (pAtmOamDevCntrl)->eCfgState = newstate

/** get the VRX drv state - ATM OAM TxBuf */
#define MEI_ATMOAM_CFG_STATE_GET(pAtmOamDevCntrl) (pAtmOamDevCntrl)->eCfgState


/** set the VRX drv state - ATM OAM TxBuf */
#define MEI_ATMOAM_TXBUF_STATE_SET(pAtmOamDevCntrl, newstate) (pAtmOamDevCntrl)->eTxBufState = newstate

/** get the VRX drv state - ATM OAM TxBuf */
#define MEI_ATMOAM_TXBUF_STATE_GET(pAtmOamDevCntrl) (pAtmOamDevCntrl)->eTxBufState


/* ==========================================================================
   Macro defs - VRX ATM OAM statistics
   ========================================================================== */
#if ( (MEI_SUPPORT_STATISTICS == 1) && (MEI_SUPPORT_ATM_OAM_STATISTICS == 1))

#define MEI_ATMOAM_STAT_INC_INST_MSG_CNT(p_atmoam_struct) \
                              (p_atmoam_struct)->statistics.instMsgCnt++
#define MEI_ATMOAM_STAT_INC_INST_MSG_ERR_CNT(p_atmoam_struct) \
                              (p_atmoam_struct)->statistics.instMsgErrCnt++
#define MEI_ATMOAM_STAT_INC_INST_MSG_NFC_CNT(p_atmoam_struct) \
                              (p_atmoam_struct)->statistics.instMsgNfcCnt++
#define MEI_ATMOAM_STAT_ADD_INST_CELL_CNT(p_atmoam_struct, value) \
                              (p_atmoam_struct)->statistics.instCellCnt += (value)

#define MEI_ATMOAM_STAT_INC_EXTR_MSG_CNT(p_atmoam_struct) \
                              (p_atmoam_struct)->statistics.extrMsgCnt++
#define MEI_ATMOAM_STAT_ADD_EXTR_CELL_CNT(p_atmoam_struct, value) \
                              (p_atmoam_struct)->statistics.extrCellCnt += (value)
#define MEI_ATMOAM_STAT_INC_EXTR_MSG_ERR_CNT(p_atmoam_struct) \
                              (p_atmoam_struct)->statistics.extrMsgErrCnt++
#define MEI_ATMOAM_STAT_ADD_EXTR_CELL_ERR_CNT(p_atmoam_struct, value) \
                              (p_atmoam_struct)->statistics.extrCellErrCnt += (value)

#define MEI_ATMOAM_STAT_INC_ALM_MSG_CNT(p_atmoam_struct) \
                              (p_atmoam_struct)->statistics.almMsgCnt++
#define MEI_ATMOAM_STAT_SET_EXTR_FAILED_CELL_CNT(p_atmoam_struct, value) \
                              (p_atmoam_struct)->statistics.almMsgExtrFailCellCnt = (value)

#else

#define MEI_ATMOAM_STAT_INC_INST_MSG_CNT(p_atmoam_struct)
#define MEI_ATMOAM_STAT_INC_INST_MSG_ERR_CNT(p_atmoam_struct)
#define MEI_ATMOAM_STAT_INC_INST_MSG_NFC_CNT(p_atmoam_struct)
#define MEI_ATMOAM_STAT_ADD_INST_CELL_CNT(p_atmoam_struct, value)

#define MEI_ATMOAM_STAT_INC_EXTR_MSG_CNT(p_atmoam_struct)
#define MEI_ATMOAM_STAT_ADD_EXTR_CELL_CNT(p_atmoam_struct, value)
#define MEI_ATMOAM_STAT_INC_EXTR_MSG_ERR_CNT(p_atmoam_struct)
#define MEI_ATMOAM_STAT_ADD_EXTR_CELL_ERR_CNT(p_atmoam_struct, value)

#define MEI_ATMOAM_STAT_INC_ALM_MSG_CNT(p_atmoam_struct)
#define MEI_ATMOAM_STAT_SET_EXTR_FAILED_CELL_CNT(p_atmoam_struct, value)

#endif      /* #if ( (MEI_SUPPORT_STATISTICS == 1) && (MEI_SUPPORT_ATM_OAM_STATISTICS == 1)) */

/* ==========================================================================
   typedefs - VRX ATM OAM
   ========================================================================== */

/**
   State for the VRX ATM OAM configurtaionr.
*/
typedef enum
{
   /** initial state after startup, first enable */
   eMEI_ATMOAM_OP_CFG_INITIAL,
   /** the VRX ATM OAM configuration is valid */
   eMEI_ATMOAM_OP_CFG_VALID,
   /** the VRX ATM OAM configuration is valid and written to the device */
   eMEI_ATMOAM_OP_CFG_WRITTEN
} MEI_ATMOAM_OP_CFG_E;


/**
   State for the VRX ATM OAM internal TX buffer.
*/
typedef enum
{
   /** initial state after startup, first enable */
   eMEI_ATMOAM_OP_TX_BUF_INITIAL,
   /** VRX internal ATM OAM TX buffer - status free */
   eMEI_ATMOAM_OP_TX_BUF_FREE,
   /** VRX internal ATM OAM TX buffer - status busy */
   eMEI_ATMOAM_OP_TX_BUF_BUSY,
   /** VRX internal ATM OAM TX buffer - status error */
   eMEI_ATMOAM_OP_TX_BUF_ERROR
} MEI_ATMOAM_OP_TX_BUF_E;


#if ((MEI_SUPPORT_STATISTICS == 1) && (MEI_SUPPORT_ATM_OAM_STATISTICS == 1))
/**
   VRX ATM OAM statistic informations
*/
typedef struct vrx_atmoam_stats_s
{
   /** number of ATM OAM modem insert messages */
   IFX_uint32_t instMsgCnt;
   /** number of failed ATM OAM modem insert messages */
   IFX_uint32_t instMsgErrCnt;
   /** number of modem indication after a ATM OAM modem insert op */
   IFX_uint32_t instMsgNfcCnt;
   /** number of ATM OAM inserted cells */
   IFX_uint32_t instCellCnt;

   /** number of ATM OAM modem extract messages */
   IFX_uint32_t extrMsgCnt;
   /** number of ATM OAM extracted cells */
   IFX_uint32_t extrCellCnt;
   /** number of ATM OAM modem extract messages errors */
   IFX_uint32_t extrMsgErrCnt;
   /** number of ATM OAM modem extract cell errors */
   IFX_uint32_t extrCellErrCnt;

   /** number of ATM OAM alarm messages */
   IFX_uint32_t almMsgCnt;
   /** number of ATM OAM extract failed cells */
   IFX_uint32_t almMsgExtrFailCellCnt;

} MEI_ATMOAM_DRV_STATS_T;
#endif      /* #if ( (MEI_SUPPORT_STATISTICS == 1) && (MEI_SUPPORT_ATM_OAM_STATISTICS == 1)) */

/**
   VRX ATM OAM cell buffer
*/
typedef struct vrx_atmoam_cell_buffer_s
{
   IFX_uint32_t                 atmOamId;
   IFX_uint32_t                 cellCnt;
   IOCTL_MEI_ATMOAM_rawCell_t atmCells[MEI_ATMOAM_MAX_RX_CELL_CNT];
} MEI_ATMOAM_CELL_BUFFER_T;

/**
   VRX ATM OAM - data for ATM OAM insert extract.

   - TX buffer state
   - LOCK
   - Statistic data
*/
typedef struct vrx_atmoam_dev_cntrl_s
{
   /** confifure the operation mode for the ATM OAM part */
   IFX_uint32_t               atmOamOpMode;

   /** config device - enable insert extract */
   IFX_boolean_t              bEnInsExt;
   /** config device - enable alarms for extraction failure */
   IFX_boolean_t              bEnAlmOnExtract;
   /** config device - enable events for TX insert done */
   IFX_boolean_t              bEnEvtOnInsert;
   /** config device - currenly not use (line only = 0) */
   IFX_uint32_t               dir;
   /** config device - currently not used bearer channel 0 */
   IFX_uint32_t               linkNo;

   /** config driver - transparent mode for incoming event messages */
   IFX_uint32_t               atmOamTransMode;

   /** current configuration has been witten to the device */
   MEI_ATMOAM_OP_CFG_E      eCfgState;
   /** status - ATM OAM current TX buffer state */
   MEI_ATMOAM_OP_TX_BUF_E   eTxBufState;

   /** ATM OAM cntrl access semaphore */
   MEI_DRVOS_sema_t           pDevAtmOamCtrlRWlock;
   /** trigger for ATM OAM insert done */
   IFX_boolean_t              bAtmOamInstDoneNeedWakeUp;
   /** signal cell transmit done */
   MEI_DRVOS_event_t          eventAtmOamInstDone;

   /** timeout count for TX done event */
   IFX_uint32_t               timeoutCount;

   /** receive buffer ATM OAM cells */
   MEI_ATMOAM_CELL_BUFFER_T rxAtmCells;

#if ( (MEI_SUPPORT_STATISTICS == 1) && (MEI_SUPPORT_ATM_OAM_STATISTICS == 1))
   /** statistics for ATM OAM insert extract */
   MEI_ATMOAM_DRV_STATS_T   statistics;
#endif

} MEI_ATMOAM_DEV_CNTRL_T;


/* ==========================================================================
   ATM OAM Global Variables.
   ========================================================================== */

MEI_DRV_PRN_USR_MODULE_DECL(MEI_ATMOAM);
MEI_DRV_PRN_INT_MODULE_DECL(MEI_ATMOAM);

#endif      /* #if (MEI_DRV_ATM_OAM_ENABLE == 1) */

#endif      /* #ifndef _DRV_MEI_CPE_ATMOAM_H */

