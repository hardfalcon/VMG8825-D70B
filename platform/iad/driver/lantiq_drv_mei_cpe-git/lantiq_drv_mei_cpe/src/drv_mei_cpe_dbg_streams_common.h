/****************************************************************************

                          Copyright (c) 2007-2015
                     Lantiq Beteiligungs-GmbH & Co. KG

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

*****************************************************************************/
#ifndef _DRV_MEI_CPE_DBG_STREAMS_COMMON_H
#define _DRV_MEI_CPE_DBG_STREAMS_COMMON_H

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

/** \file
   Debug Streams: common defines for the MEI Debug stream handling.
*/

/* ==========================================================================
   includes
   ========================================================================== */
#ifdef __cplusplus
   extern "C" {
#endif

#include "drv_mei_cpe_config.h"

#if (MEI_SUPPORT_DEBUG_STREAMS == 1)

#include "ifx_types.h"

                             
/* ==========================================================================
   Debug Streams - Macro defs, Common 
   ========================================================================== */

/** Debug Streams - max buffer size. */
#define MEI_DBG_STREAMS_MAX_BUFFER               0x80000

/** Debug Streams - wait poll time */
#define MEI_DBG_STREAMS_WAIT_POLL_TIME_MS        10


/** Message ID for CMD_DBG_DebugStreamControl */
#define CMD_DBG_DEBUG_STREAM_CONTROL    0x7343

/**
   This message is used to enable/disable  
   the  output  of  so-called  "debug  streams"  with  additional  debug  information 
*/
typedef struct CMD_DBG_DebugStreamControl CMD_DBG_DebugStreamControl_t;

/** Message ID for ACK_DBG_DebugStreamControl */
#define ACK_DBG_DEBUG_STREAM_CONTROL    0x7343

/** Acknowledgement to CMD_DBG_DebugStreamControl */
typedef struct ACK_DBG_DebugStreamControl ACK_DBG_DebugStreamControl_t;

/**
   Structures to provide CMV control and filter of Debug Streams
   Enables/Disables the output of so-called "debug streams" with additional
   debug information (see also EVT_DBG_DebugStream). 
*/
struct CMD_DBG_DebugStreamControl
{
#if MEI_DRV_OS_BYTE_ORDER == MEI_DRV_OS_BIG_ENDIAN
   /** Index */
   IFX_uint16_t Index;
   /** Length */
   IFX_uint16_t Length;
   /** Parameter 0  */
   IFX_uint16_t Parameter0;
#else
   /** Index */
   IFX_uint16_t Index;
   /** Length */
   IFX_uint16_t Length;
   /** Parameter 0  */
   IFX_uint16_t Parameter0;
#endif
} __PACKED__ ;

/**
   Acknowledgement to CMD_DBG_DebugStreamControl.
*/
struct ACK_DBG_DebugStreamControl
{
#if MEI_DRV_OS_BYTE_ORDER == MEI_DRV_OS_BIG_ENDIAN
   /** Index */
   IFX_uint16_t Index;
   /** Length */
   IFX_uint16_t Length;
#else
   /** Index */
   IFX_uint16_t Index;
   /** Length */
   IFX_uint16_t Length;
#endif
} __PACKED__ ;


/** Message ID for CMD_DBG_DebugStreamConfigure */
#define CMD_DBG_DEBUG_STREAM_CONFIGURE  0x7443

/**
   This message is used to write a bit mask to configure which messages 
   are output in the debug stream.
*/   
typedef struct CMD_DBG_DebugStreamConfigure CMD_DBG_DebugStreamConfigure_t;

/** Message ID for ACK_DBG_DebugStreamControl */
#define ACK_DBG_DEBUG_STREAM_CONFIGURE  0x7443

/** Acknowledgement to CMD_DBG_DebugStreamConfigure */
typedef struct ACK_DBG_DebugStreamConfigure ACK_DBG_DebugStreamConfigure_t;

/**
   Writes a bit mask to configure which messages are output in the debug stream.
   The mask parameters constitute a 160-bit mask for enabling different sub-streams 
   that will contribute to the over-all debug-stream. All masks have to be sent at once.
   Remarks: A pre-defined set of masks shall be used only. The bit position in the bitmask 
   corresponds to the subtype number of the debug-(sub)stream as listed in Definitions 
   of Debug Stream Subtypes.
*/
struct CMD_DBG_DebugStreamConfigure
{
#if MEI_DRV_OS_BYTE_ORDER == MEI_DRV_OS_BIG_ENDIAN
   /** Index */
   IFX_uint16_t Index;
   /** Length */
   IFX_uint16_t Length;
   /** Mask_1. Default: 0x00000000 */
   IFX_uint32_t Mask1;
   /** Mask_2. Default: 0x00000192 */
   IFX_uint32_t Mask2;
   /** Mask_3. Default: 0x0000007F */
   IFX_uint32_t Mask3;
   /** Mask_4. Default: 0x0000007F */
   IFX_uint32_t Mask4;
   /** Mask_5. Useful in VDSL only, not evaluated in ADSL modes ("Don't care")
       Default: 0x00000000 */
   IFX_uint32_t Mask5;            
#else
   /** Index */
   IFX_uint16_t Index;
   /** Length */
   IFX_uint16_t Length;
   /** Mask_1. Default: 0x00000000 */
   IFX_uint32_t Mask1;
   /** Mask_2. Default: 0x00000192 */
   IFX_uint32_t Mask2;
   /** Mask_3. Default: 0x0000007F */
   IFX_uint32_t Mask3;
   /** Mask_4. Default: 0x0000007F */
   IFX_uint32_t Mask4;
   /** Mask_5. Useful in VDSL only, not evaluated in ADSL modes ("Don't care")
       Default: 0x00000000 */
   IFX_uint32_t Mask5;
#endif
} __PACKED__ ;

/**
   Acknowledgement to CMD_DBG_DebugStreamControl.
*/
struct ACK_DBG_DebugStreamConfigure
{
#if MEI_DRV_OS_BYTE_ORDER == MEI_DRV_OS_BIG_ENDIAN
   /** Index */
   IFX_uint16_t Index;
   /** Length */
   IFX_uint16_t Length;
#else
   /** Index */
   IFX_uint16_t Index;
   /** Length */
   IFX_uint16_t Length;
#endif
} __PACKED__ ;


/** Message ID for EVT_DBG_BufferOverflow */
#define EVT_DBG_BUFFER_OVERFLOW 0x7503

/**
   This autononmous message is used to indicate a stream buffer overflow on the host controller.
*/   
typedef struct EVT_DBG_BufferOverflow EVT_DBG_BufferOverflow_t;

/**
   Autonomous message indicating a FIFO overflow in the mailbox interface.
   The message is placed into the mailbox  b e f o r e  the message which
   would cause an overflow.
*/
struct EVT_DBG_BufferOverflow
{
#if MEI_DRV_OS_BYTE_ORDER == MEI_DRV_OS_BIG_ENDIAN
   /** Index */
   IFX_uint16_t Index;
   /** Length */
   IFX_uint16_t Length;
#else
   /** Index */
   IFX_uint16_t Index;
   /** Length */
   IFX_uint16_t Length;
#endif
} __PACKED__ ;


/** Message ID for EVT_DBG_DebugStream */
#define EVT_DBG_DEBUG_STREAM    0x7603

/**
   This autononmous message is is an auto message containing debug stream content.
*/   
typedef struct EVT_DBG_DebugStream EVT_DBG_DebugStream_t;

/*
   Provides debug information as configured with @ref _MSG_CMD_DBG_DEBUG_STREAM_CONFIGURE_.
   The EVTs are generated only if streaming was enabled by @ref _MSG_CMD_DBG_DEBUG_STREAM_CONTROL_.
   One EVT can contain a whole debug substream or only a fragment (see parameter StreamStatus).
   For overflow situations an "Overflow Pre-Indication" flag and a stream counter are contained.
   @note
   The debug streams are queued in a debug FIFO and then moved to the host-IF
   mailbox (if bigger than the mailbox size, then in fragments). If a debug
   stream does not fit into the FIFO because it is too full, it is discarded
   completely and the overflow flag is set in the last fragment of the
   preceeding message. A stream is either delivered to the host completely or
   discarded completely, not just single fragments of it.
   Protocol version >= 3.
**/
struct EVT_DBG_DebugStream
{
#if MEI_DRV_OS_BYTE_ORDER == MEI_DRV_OS_BIG_ENDIAN
   /** Index */
   IFX_uint16_t Index;
   /** Length */
   IFX_uint16_t Length;
   /** Stream Size.
       The number of bytes following in this single EVT, excluding this field. \n
       It is not the overall stream size in case of fragmented streams which are
       reported with more than one EVT.
   */
   IFX_uint16_t StreamSize;
   /** Stream ID & Protocol Version.
       Bitmap, description starting at @ref HMH_33BC_STREAM_ID_MASK.
   */
   IFX_uint16_t StreamId;
   /** Stream Status.
       Bitmap, description starting at @ref HMH_33BC_SOF_EOF_MASK.
   */
   IFX_uint16_t StreamStatus;
   /** Tx Symbol Count.
       Transmit symbol counter.
   */
   IFX_uint32_t TxSymCount;
   /** Stream CRC.
       CRC16 over the stream's data. Only available for protocol version (see StreamId) >=3.
   */
   IFX_uint16_t StreamCrc;
   /** Debug Stream Data.
       Data array of variable length: (StreamSize - 8) bytes. The output
       is as bytes in 16-bit words. \n The sequence of data is as follows: The first byte
       of the debug data (GHS/SOC message) is the LS Byte of the first 16-bit word.
       - [7:0] - Octet 1 of debug stream data
       - [15:8] - Octet 2 of debug stream data
   */
   IFX_uint16_t Data[122];
#else
   /** Index */
   IFX_uint16_t Index;
   /** Length */
   IFX_uint16_t Length;
   /** Stream Size.
       The number of bytes following in this single EVT, excluding this field. \n
       It is not the overall stream size in case of fragmented streams which are
       reported with more than one EVT.
   */
   IFX_uint16_t StreamSize;
   /** Stream ID & Protocol Version.
       Bitmap, description starting at @ref HMH_33BC_STREAM_ID_MASK.
   */
   IFX_uint16_t StreamId;
   /** Stream Status.
       Bitmap, description starting at @ref HMH_33BC_SOF_EOF_MASK.
   */
   IFX_uint16_t StreamStatus;
   /** Tx Symbol Count.
       Transmit symbol counter.
   */
   IFX_uint32_t TxSymCount;
   /** Stream CRC.
       CRC16 over the stream's data. Only available for protocol version (see StreamId) >=3.
   */
   IFX_uint16_t StreamCrc;
   /** Debug Stream Data.
       Data array of variable length: (StreamSize - 8) bytes. The output
       is as bytes in 16-bit words. \n The sequence of data is as follows: The first byte
       of the debug data (GHS/SOC message) is the LS Byte of the first 16-bit word.
       - [7:0] - Octet 1 of debug stream data
       - [15:8] - Octet 2 of debug stream data
   */
   IFX_uint16_t Data[122];
#endif
} __PACKED__ ;

/* ==========================================================================
   Debug Streams - Macro defs
   ========================================================================== */

/**
   Get unique access to the debug streams context struct
*/
#define MEI_DBG_STREAMS_CTX_ACCESS_GET(pMeiDynCntrl) \
               /*lint -e{717} */ \
               do { \
                  (void)MEI_DRVOS_SemaphoreLock(&pMeiDynCntrl->dbgStreamLock); \
               } while(0)

/**
   Release unique access to the debug streams context struct
*/
#define MEI_DBG_STREAMS_CTX_ACCESS_RELEASE(pMeiDynCntrl) \
               /*lint -e{717} */ \
               do { \
                  (void)MEI_DRVOS_SemaphoreUnlock(&pMeiDynCntrl->dbgStreamLock); \
               } while(0)


/**
   Get unique access to the debug streams handling
*/
#define MEI_DBG_STREAMS_UNIQUE_ACCESS_GET(pMeiDynCntrl) \
               /*lint -e{717} */ \
               do { \
                  (void)MEI_DRVOS_SemaphoreLock(&pMeiDynCntrl->dbgStreamLock); \
                  (void)MEI_DRVOS_SemaphoreLock(&pMeiDynCntrl->pMeiDev->meiDrvCntrl.pMeiDevCntrl->pDevMeiAccessLock); \
                  MEI_MeiIfPortIntDisable(pMeiDynCntrl); \
               } while(0)

/**
   Release unique access to the debug streams handling
*/
#define MEI_DBG_STREAMS_UNIQUE_ACCESS_RELEASE(pMeiDynCntrl) \
               /*lint -e{717} */ \
               do { \
                  MEI_MeiIfPortIntEnable(pMeiDynCntrl); \
                  (void)MEI_DRVOS_SemaphoreUnlock(&pMeiDynCntrl->pMeiDev->meiDrvCntrl.pMeiDevCntrl->pDevMeiAccessLock); \
                  (void)MEI_DRVOS_SemaphoreUnlock(&pMeiDynCntrl->dbgStreamLock); \
               } while(0)


/**
   Get unique access to the debug streams global config
*/
#define MEI_DBG_STREAMS_GLOBAL_CFG_ACCESS_GET \
               /*lint -e{717} */ \
               do { \
                  (void)MEI_DRVOS_SemaphoreLock(&MEI_dbgStreamCntrlLock); \
               } while(0)

/**
   Release unique access to the debug streams global config
*/
#define MEI_DBG_STREAMS_GLOBAL_CFG_ACCESS_RELEASE \
               /*lint -e{717} */ \
               do { \
                  (void)MEI_DRVOS_SemaphoreUnlock(&MEI_dbgStreamCntrlLock); \
               } while(0)


/**
   Increment the per port counter for the given device.
*/
#define MEI_DBG_STREAMS_GLOBAL_CFG_ADD_DEV(MEI_DEVNUM) \
               MEI_DRV_dbgStreamsOn[(MEI_DEVNUM)]++

/**
   Decrement the per port counter for the given device.
*/
#define MEI_DBG_STREAMS_GLOBAL_CFG_REMOVE_DEV(MEI_DEVNUM) \
               /*lint -e{717} */ \
               do { \
                  if (MEI_DRV_dbgStreamsOn[(MEI_DEVNUM)]) \
                  { MEI_DRV_dbgStreamsOn[(MEI_DEVNUM)]--; } \
               } while(0)

/**
   Return the per port counter for the given device.
*/
#define MEI_DBG_STREAMS_GLOBAL_CFG_DEV_STATE_GET(MEI_DEVNUM) \
               MEI_DRV_dbgStreamsOn[(MEI_DEVNUM)]


#if (MEI_DBG_STREAMS_SUPPORT_STATISTICS == 1)

#  define MEI_DBG_STREAMS_STAT_MSG_D2H_CNT_INC(pInstDynDebugStream) \
               (pInstDynDebugStream)->dbgStreamStatistics.msgD2HCnt++
#  define MEI_DBG_STREAMS_STAT_DATA_D2H_SIZE_ADD(pInstDynDebugStream, dataSize) \
               /*lint -e{717} */ \
               do { \
                  (pInstDynDebugStream)->dbgStreamStatistics.dataD2HSize_byte += (dataSize); \
               } while(0)

#  define MEI_DBG_STREAMS_STAT_MSG_H2BUF_CNT_INC(pInstDynDebugStream) \
               (pInstDynDebugStream)->dbgStreamStatistics.msgH2BufCnt++
#  define MEI_DBG_STREAMS_STAT_DATA_H2BUF_SIZE_ADD(pInstDynDebugStream, dataSize) \
               /*lint -e{717} */ \
               do { \
                  (pInstDynDebugStream)->dbgStreamStatistics.dataH2BufSize_byte += (dataSize); \
               } while(0)

#  define MEI_DBG_STREAMS_STAT_MSG_BUF2USR_CNT_INC(pInstDynDebugStream) \
               (pInstDynDebugStream)->dbgStreamStatistics.msgBuf2UsrCnt++
#  define MEI_DBG_STREAMS_STAT_DATA_BUF2USR_SIZE_ADD(pInstDynDebugStream, dataSize) \
               /*lint -e{717} */ \
               do { \
                  (pInstDynDebugStream)->dbgStreamStatistics.dataBuf2UsrSize_byte += (dataSize); \
               } while(0)

#  define MEI_DBG_STREAMS_STAT_MSG_RBUFFER_OVERWR_CNT_INC(pInstDynDebugStream) \
               (pInstDynDebugStream)->dbgStreamStatistics.msgRingBufferOverwriteCnt++
#  define MEI_DBG_STREAMS_STAT_DATA_RBUFFER_OVERWR_SIZE_ADD(pInstDynDebugStream, dataSize) \
               /*lint -e{717} */ \
               do { \
                  (pInstDynDebugStream)->dbgStreamStatistics.dataRingBufferOverwriteSize_byte += (dataSize); \
               } while(0)

#  define MEI_DBG_STREAMS_STAT_MSG_BFULL_DISCARD_CNT_INC(pInstDynDebugStream) \
               (pInstDynDebugStream)->dbgStreamStatistics.msgBufferFullDiscardCnt++
#  define MEI_DBG_STREAMS_STAT_DATA_BFULL_DISCARD_SIZE_ADD(pInstDynDebugStream, dataSize) \
               /*lint -e{717} */ \
               do { \
                  (pInstDynDebugStream)->dbgStreamStatistics.dataBufferFullDiscardSize_byte += (dataSize); \
               } while(0)

#  define MEI_DBG_STREAMS_STAT_MSG_ERR_DISCARD_CNT_INC(pInstDynDebugStream) \
               (pInstDynDebugStream)->dbgStreamStatistics.msgProcessErrDiscardCnt++
#  define MEI_DBG_STREAMS_STAT_DATA_ERR_DISCARD_SIZE_ADD(pInstDynDebugStream, dataSize) \
               /*lint -e{717} */ \
               do { \
                  (pInstDynDebugStream)->dbgStreamStatistics.dataProcessErrDiscardSize_byte += (dataSize); \
               } while(0)

#  define MEI_DBG_STREAMS_STAT_MSG_CFG_DISCARD_CNT_INC(pInstDynDebugStream) \
               (pInstDynDebugStream)->dbgStreamStatistics.msgCfgDiscardCnt++;
#  define MEI_DBG_STREAMS_STAT_DATA_CFG_DISCARD_SIZE_ADD(pInstDynDebugStream, dataSize) \
               /*lint -e{717} */ \
               do { \
                  (pInstDynDebugStream)->dbgStreamStatistics.dataCfgDiscardSize_byte += (dataSize); \
               } while(0)


#  define MEI_DBG_STREAMS_STAT_MSG_CORRUPTED_CNT_INC(pInstDynDebugStream) \
               (pInstDynDebugStream)->dbgStreamStatistics.msgCorruptedCnt++

#  define MEI_DBG_STREAMS_STAT_DEV_OVERFLOW_CNT_INC(pInstDynDebugStream) \
               (pInstDynDebugStream)->dbgStreamStatistics.deviceOverflowCnt++

#  define MEI_DBG_STREAMS_STAT_BUF_OVERFLOW_CNT_INC(pInstDynDebugStream) \
               (pInstDynDebugStream)->dbgStreamStatistics.bufferOverflowCnt++

#  define MEI_DBG_STREAMS_STAT_D2H_STREAM_CNT_INC(pInstDynDebugStream) \
               (pInstDynDebugStream)->dbgStreamStatistics.streamD2HCnt++

#else

#  define MEI_DBG_STREAMS_STAT_MSG_D2H_CNT_INC(pInstDynDebugStream)
#  define MEI_DBG_STREAMS_STAT_DATA_D2H_SIZE_ADD(pInstDynDebugStream, dataSize)

#  define MEI_DBG_STREAMS_STAT_MSG_H2BUF_CNT_INC(pInstDynDebugStream)
#  define MEI_DBG_STREAMS_STAT_DATA_H2BUF_SIZE_ADD(pInstDynDebugStream, dataSize)

#  define MEI_DBG_STREAMS_STAT_MSG_BUF2USR_CNT_INC(pInstDynDebugStream)
#  define MEI_DBG_STREAMS_STAT_DATA_BUF2USR_SIZE_ADD(pInstDynDebugStream, dataSize)

#  define MEI_DBG_STREAMS_STAT_MSG_RBUFFER_OVERWR_CNT_INC(pInstDynDebugStream)
#  define MEI_DBG_STREAMS_STAT_DATA_RBUFFER_OVERWR_SIZE_ADD(pInstDynDebugStream, dataSize)

#  define MEI_DBG_STREAMS_STAT_MSG_BFULL_DISCARD_CNT_INC(pInstDynDebugStream)
#  define MEI_DBG_STREAMS_STAT_DATA_BFULL_DISCARD_SIZE_ADD(pInstDynDebugStream, dataSize)

#  define MEI_DBG_STREAMS_STAT_MSG_ERR_DISCARD_CNT_INC(pInstDynDebugStream)
#  define MEI_DBG_STREAMS_STAT_DATA_ERR_DISCARD_SIZE_ADD(pInstDynDebugStream, dataSize)

#  define MEI_DBG_STREAMS_STAT_MSG_CFG_DISCARD_CNT_INC(pInstDynDebugStream)
#  define MEI_DBG_STREAMS_STAT_DATA_CFG_DISCARD_SIZE_ADD(pInstDynDebugStream, dataSize)

#  define MEI_DBG_STREAMS_STAT_MSG_CORRUPTED_CNT_INC(pInstDynDebugStream)

#  define MEI_DBG_STREAMS_STAT_DEV_OVERFLOW_CNT_INC(pInstDynDebugStream)

#  define MEI_DBG_STREAMS_STAT_BUF_OVERFLOW_CNT_INC(pInstDynDebugStream)

#  define MEI_DBG_STREAMS_STAT_D2H_STREAM_CNT_INC(pInstDynDebugStream)
#endif

#ifndef MEI_DBG_STREAMS_CAST
#  define MEI_DBG_STREAMS_CAST(x, value)   (x)(value)
#endif

#ifndef MEI_DBG_STREAMS_PTR_CAST_GET_ULONG
#  define MEI_DBG_STREAMS_PTR_CAST_GET_ULONG(buffer_ptr) \
                  /*lint -e{506} -e{826} */ \
                  ( (sizeof(IFX_addr_t) == 4) ? \
                     ((((IFX_addr_t)(buffer_ptr).pUInt8 & 0x03) != 0) ? IFX_NULL : (buffer_ptr).pULong) : \
                     ((((IFX_addr_t)(buffer_ptr).pUInt8 & 0x07) != 0) ? IFX_NULL : (buffer_ptr).pULong) )
#endif 

#ifndef MEI_DBG_STREAMS_PTR_CAST_GET_UINT32
#  define MEI_DBG_STREAMS_PTR_CAST_GET_UINT32(buffer_ptr) \
                  ((((IFX_addr_t)(buffer_ptr).pUInt8 & 0x03) != 0) ? IFX_NULL : (buffer_ptr).pUInt32)
#endif

/* ==========================================================================
   Debug Streams - type definitions
   ========================================================================== */

/**
   Config Status of the device related data.
*/
typedef enum
{
   /** current stettings still not written */
   eMEI_DbgStreamCfgStatusNotWritten = 0,
   /** current stettings written */
   eMEI_DbgStreamCfgStatusWritten = 1

} MEI_DbgStreamCfgStatus_e;


/**
   Config Status of the device related data.
*/
typedef enum
{
   /** no overflow processing necessary */
   eMEI_DbgStreamOverflowState_Clear = 0,
   /** overflow occurred - still not processed */
   eMEI_DbgStreamOverflowState_Pending = 1,
   /** overflow occurred - processed */
   eMEI_DbgStreamOverflowState_Processed = 2
} MEI_DbgStreamOverflowState_e;

/**
   Defines the debug stream messages filter state.
*/
typedef enum
{
   /** a current message will be passed by the filter */
   eMEI_DbgStreamFilterState_Store = 0,
   /** a current message will be skipped by the filter */
   eMEI_DbgStreamFilterState_SkipThis = 1,
   /** a next message will be skipped by the filter */
   eMEI_DbgStreamFilterState_SkipNext = 2
} MEI_DbgStreamFilterState_e; 

/**
   This is the addr pointer datatype (On 32bit systems it is 4 byte wide).
*/
typedef IFX_ulong_t IFX_addr_t;

/**
   Pointer Struct for alligned buffer allocation and access.
*/
typedef union
{
   IFX_uint8_t    *pUInt8;
   IFX_uint16_t   *pUInt16;
   IFX_uint32_t   *pUInt32;
   IFX_ulong_t    *pULong;
   IFX_void_t     *pVoid;
   IFX_addr_t     *pAddr;
} MEI_OS_BUFFER_PTR_T;

#if (MEI_DBG_STREAMS_SUPPORT_STATISTICS == 1)
/*
   - statistics
   - context
*/
typedef struct MEI_DbgStreamStatistic_s
{
   /** number of received debug stream event msg's (device 2 host) */
   IFX_uint_t  msgD2HCnt;
   /** size of received debug stream event msg's (device 2 host) */
   IFX_uint_t  dataD2HSize_byte;

   /* number of written debug stream event msg's (host 2 buffer) */
   IFX_uint_t  msgH2BufCnt;
   /** size of written debug stream event msg's (host 2 buffer) */
   IFX_uint_t  dataH2BufSize_byte;

   /* number of read debug stream event msg's */
   IFX_uint_t  msgBuf2UsrCnt;
   /** size of read debug stream event msg's */
   IFX_uint_t  dataBuf2UsrSize_byte;

   /* number of overwritten stream event msg's (ring buffer overwrite) */
   IFX_uint_t  msgRingBufferOverwriteCnt;
   /* size of overwritten stream event msg's (ring buffer overwrite) */
   IFX_uint_t  dataRingBufferOverwriteSize_byte;

   /* number of discarded stream event msg's (no space in receive buffer) */
   IFX_uint_t  msgBufferFullDiscardCnt;
   /** size of discarded stream event msg's (no space in receive buffer) */
   IFX_uint_t  dataBufferFullDiscardSize_byte;

   /* number of discarded stream event msg's because of a processing error */
   IFX_uint_t  msgProcessErrDiscardCnt;
   /** size of discarded stream event msg's because of a processing error */
   IFX_uint_t  dataProcessErrDiscardSize_byte;

   /* number of discarded stream event msg's because of miss / not configured */
   IFX_uint_t  msgCfgDiscardCnt;
   /** size of discarded stream event msg's because of miss / not configured */
   IFX_uint_t  dataCfgDiscardSize_byte;

   /* number of corrupted stream event msg's */
   IFX_uint_t  msgCorruptedCnt;

   /* number of overflows on the device */
   IFX_uint_t  deviceOverflowCnt;

   /* number of overflows in FIFO mode */
   IFX_uint_t  bufferOverflowCnt;

   /* number of received debug streams, fragment start (device 2 host) */
   IFX_uint_t  streamD2HCnt;

   /** start time of the debug stream handling */
   IFX_time_t  startTime;
   /** current time */
   IFX_time_t  currTime;

} MEI_DbgStreamStatistic_t;
#endif /* #if (MEI_DBG_STREAMS_SUPPORT_STATISTICS == 1) */

/**
   MEI device Dynamic data struct to handle debug streams.
*/
typedef struct MEI_dyn_dbg_strm_data_s   MEI_DYN_DBG_STRM_DATA_T;

/** MEI device Dynamic debug streams receive Data Block */
struct MEI_dyn_dbg_strm_data_s
{
  /** system start time when the init has been done */
   IFX_time_t                 startTime_ms;
   /** operation mode - fill, ring */
   MEI_DBG_STREAM_BUF_OPMODE_E     eOpMode;
   /** filter mode - start/stop, snapshot */
   MEI_DBG_STREAM_FILTER_MODE_E    eFilterMode;

   /** control msg - enable / disable */
   IFX_uint32_t               control;
   /** overflow occurred - elapsed time since start */
   IFX_time_t                 overflowTime_ms;
   /** overflow occurred */
   MEI_DbgStreamOverflowState_e bOverflowOccurred;
   /** overwrite protection for read operations */
   IFX_boolean_t              bReadPending;
   /** Notify data availability */
   IFX_boolean_t              bNotify;
   /** pointer to NFC dynamic control structure that is used for
       data availability notification */
   IFX_void_t                 *pDynNfcDbgEvtRecieved;
   /** current FIFO buffer size */
   IFX_uint_t                 dbgStreamFifoBufferSize;
   /** FIFO buffer */
   MEI_DRVOS_VFIFO            dbgStreamFifo;
   /** stream ID to find the first debug stream message to be stored */
   IFX_uint16_t startStreamId;
   /** event ID to find the first debug stream message to be stored */
   IFX_uint16_t startEventId;
   /** stream ID to find the last debug stream message to be stored */
   IFX_uint16_t stopStreamId;
   /** event ID to find the last debug stream message to be stored */
   IFX_uint16_t stopEventId;
   /** mask to find the first debug stream message to be stored */
   IFX_uint16_t startMask[MEI_DBG_STREAM_MAX_COMPARE_SIZE_IN_WORDS];
   /** pattern to find the first debug stream message to be stored */
   IFX_uint16_t startPattern[MEI_DBG_STREAM_MAX_COMPARE_SIZE_IN_WORDS];
   /** mask to find the last debug stream message to be stored */
   IFX_uint16_t stopMask[MEI_DBG_STREAM_MAX_COMPARE_SIZE_IN_WORDS];
   /** pattern to find the last debug stream message to be stored */
   IFX_uint16_t stopPattern[MEI_DBG_STREAM_MAX_COMPARE_SIZE_IN_WORDS]; 
   /** the current debug stream message filter state */
   MEI_DbgStreamFilterState_e eFilterState;

#  if (MEI_DBG_STREAMS_SUPPORT_STATISTICS == 1)
   /** debug stream statistic data */
   MEI_DbgStreamStatistic_t   dbgStreamStatistics;
#  endif
  
   /* List of dynamic debug stream receive data blocks (per open, per MEI device) */
   MEI_DYN_DBG_STRM_DATA_T *pNext;
   MEI_DYN_DBG_STRM_DATA_T *pPrev;
};

#endif      /* #if (MEI_SUPPORT_DEBUG_STREAMS == 1) */

#ifdef __cplusplus
}
#endif

#endif      /* #ifndef _DRV_MEI_CPE_DBG_STREAMS_COMMON_H */
