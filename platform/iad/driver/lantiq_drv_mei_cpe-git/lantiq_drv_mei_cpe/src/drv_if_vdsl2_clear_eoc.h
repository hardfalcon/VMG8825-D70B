/******************************************************************************

                          Copyright (c) 2007-2015
                     Lantiq Beteiligungs-GmbH & Co. KG

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#ifndef _DRV_IF_VDSL2_CLEAR_EOC_H_
#define _DRV_IF_VDSL2_CLEAR_EOC_H_
/** \file

*/

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

#ifndef MEI_DRV_OS_BYTE_ORDER
#error "missing byte order define, include drv_mei_cpe_os.h before"
#endif

/** @defgroup _DRV_IF_VDSL2_CLEAR_EOC_H
 *  @{
 */

#ifdef __cplusplus
   extern "C" {
#endif

#include "ifx_types.h"


#define  CMD_ClearEOC_TxTrigger_IDLE      0
#define  CMD_ClearEOC_TxTrigger_START     1


#define  ACK_ClearEOCStatusGet_MASK       0x0003
#define  ACK_ClearEOCStatusGet_IDLE       0

#define  ACK_ClearEOCStatusGet_TXPROG     1
#define  ACK_ClearEOCStatusGet_Reserved   2
#define  ACK_ClearEOCStatusGet_TXERR      3

#define  ACK_ClearEOCStatusGet_RXPROG     1
#define  ACK_ClearEOCStatusGet_RXDONE     2
#define  ACK_ClearEOCStatusGet_RXERR      3


#define  EVT_ClearEOCStatusGet_MASK       0x0003
#define  EVT_ClearEOCStatusGet_IDLE       0

#define  EVT_ClearEOCStatusGet_TXPROG     1
#define  EVT_ClearEOCStatusGet_TXDONE     2
#define  EVT_ClearEOCStatusGet_TXERR      3

#define  EVT_ClearEOCStatusGet_RXPROG     1
#define  EVT_ClearEOCStatusGet_RXDONE     2
#define  EVT_ClearEOCStatusGet_RXERR      3



/** Message ID for CMD_ClearEOC_Configure */
#define CMD_CLEAREOC_CONFIGURE 0x0A49

/**
   This message is used to configure the autonomous messaging related to Clear EOC
   transmission.
*/
typedef struct CMD_ClearEOC_Configure CMD_ClearEOC_Configure_t;

/** Message ID for ACK_ClearEOC_Configure */
#define ACK_CLEAREOC_CONFIGURE 0x0A49

/**
   This is the acknowledgement for CMD_ClearEOC_Configure.
*/
typedef struct ACK_ClearEOC_Configure ACK_ClearEOC_Configure_t;

/** Message ID for CMD_ClearEOC_TxTrigger */
#define CMD_CLEAREOC_TXTRIGGER 0x0949

/**
   The message is used to trigger the transmission of  Clear EOC messages that were
   placed into the Clear EOC transmit buffer before with CMD_ClearEOC_Write.
*/
typedef struct CMD_ClearEOC_TxTrigger CMD_ClearEOC_TxTrigger_t;

/** Message ID for ACK_ClearEOC_TxTrigger */
#define ACK_CLEAREOC_TXTRIGGER 0x0949

/**
   This is the acknowledgement for CMD_ClearEOC_TxTrigger.
*/
typedef struct ACK_ClearEOC_TxTrigger ACK_ClearEOC_TxTrigger_t;

/** Message ID for CMD_ClearEOC_Write */
#define CMD_CLEAREOC_WRITE 0x5143

/**
   This message is used to write data to the ClearEOC write buffer of type
   MEI_ClearEOC_t. When the buffer is filled, the transmission is started applying
   CMD_ClearEOC_TxTrigger. If the message to transmit is longer than the mailbox size,
   a sequence of writes to the ClearEOC buffer has to be done before the transmission
   is started with CMD_ClearEOC_TxTrigger. When autonomous Tx status messaging is
   activated via CMD_ClearEOC_Configure, then the finished transmission is indicated by
   EVT_ClearEOCStatusGet.
*/
typedef struct CMD_ClearEOC_Write CMD_ClearEOC_Write_t;

/** Message ID for ACK_ClearEOC_Write */
#define ACK_CLEAREOC_WRITE 0x5143

/**
   This message is the acknowledgement for CMD_ClearEOC_Write.
*/
typedef struct ACK_ClearEOC_Write ACK_ClearEOC_Write_t;

/** Message ID for CMD_ClearEOC_Read */
#define CMD_CLEAREOC_READ 0x5203

/**
   This message is used to read data from the ClearEOC buffer of type MEI_ClearEOC_t.
   The length of the actual Clear EOC message can be found in the buffer. Please refer
   to MEI_ClearEOC_t. The availability of data can either be checked via
   CMD_ClearEOCStatusGet in polling mode or it can be reported by an autonomous
   EVT_ClearEOCStatusGet message when data is received (to be enabled using
   CMD_ClearEOC_Configure).
*/
typedef struct CMD_ClearEOC_Read CMD_ClearEOC_Read_t;

/** Message ID for ACK_ClearEOC_Read */
#define ACK_CLEAREOC_READ 0x5203

/**
   This message is the acknowledgement to CMD_ClearEOC_Read.
*/
typedef struct ACK_ClearEOC_Read ACK_ClearEOC_Read_t;

/** Message ID for EVT_ClearEOC_Read */
#define EVT_CLEAREOC_READ 0x5203

/**
   This message is an autonomous message that is generated when ClearEOC data was
   received and autonomous Clear EOC data messaging has been activated via
   CMD_ClearEOC_Configure. If the ClearEOC data does not fit in one message, then a
   sequence of messages is generated. The ClearEOC buffer is of type MEI_ClearEOC_t.
*/
typedef struct EVT_ClearEOC_Read EVT_ClearEOC_Read_t;

/** Message ID for CMD_ClearEOCStatusGet */
#define CMD_CLEAREOCSTATUSGET 0x0B09

/**
   This message is used to retrieve the status of the clear eoc data transmission.
*/
typedef struct CMD_ClearEOCStatusGet CMD_ClearEOCStatusGet_t;

/** Message ID for ACK_ClearEOCStatusGet */
#define ACK_CLEAREOCSTATUSGET 0x0B09

/**
   This is the acknowledgement for CMD_ClearEOCStatusGet.
*/
typedef struct ACK_ClearEOCStatusGet ACK_ClearEOCStatusGet_t;

/** Message ID for EVT_ClearEOCStatusGet */
#define EVT_CLEAREOCSTATUSGET 0x0B09

/**
   This autonomous message reports the Clear EOC status. It is sent only if the
   "Autonomous Status Message Control" was enabled for Tx and/or Rx direction with
   CMD_ClearEOC_Configure. If Tx direction is enabled, the message is generated when a
   Tx transmission is finished or failed. If Rx direction is enabled, the message is
   generated when the Rx status transitions from "Idle" to "Data Available" for
   retrieval by the host.
*/
typedef struct EVT_ClearEOCStatusGet EVT_ClearEOCStatusGet_t;


#define CMD_CLEAREOC_CONFIGURE_P02_RX_EVT_DATA      0x0001
#define CMD_CLEAREOC_CONFIGURE_P02_RX_EVT_STAT      0x0002
#define CMD_CLEAREOC_CONFIGURE_P02_TX_EVT_STAT      0x0004
#define CMD_CLEAREOC_CONFIGURE_P02_RES00            0xFFF8



/**
   This message is used to configure the autonomous messaging related to Clear EOC
   transmission.
*/
struct CMD_ClearEOC_Configure
{
   /** Index */
   IFX_uint16_t Index;
   /** Length */
   IFX_uint16_t Length;

#if 1
   IFX_uint16_t P02_MsgControl;
#else
/* #if (MEI_DRV_OS_BYTE_ORDER == MEI_DRV_OS_BIG_ENDIAN) */
   /** Reserved */
   IFX_uint16_t Res00 : 13;
   /** Tx Autonomous Status Message Control */
   IFX_uint16_t TxEVTstatus : 1;
   /** Rx Autonomous Status Message Control */
   IFX_uint16_t RxEVTstatus : 1;
   /** Rx Autonomous Clear EOC Data Message Control */
   IFX_uint16_t RxEVTdata : 1;
#endif
} __PACKED__ ;


/**
   This is the acknowledgement for CMD_ClearEOC_Configure.
*/
struct ACK_ClearEOC_Configure
{
   /** Index */
   IFX_uint16_t Index;
   /** Length */
   IFX_uint16_t Length;
} __PACKED__ ;


#define CMD_CLEAREOC_TXTRIGGER_P02_TX_TRIGGER       0x0001
#define CMD_CLEAREOC_TXTRIGGER_P02_RES00            0xFFFE

/**
   The message is used to trigger the transmission of  Clear EOC messages that were
   placed into the Clear EOC transmit buffer before with CMD_ClearEOC_Write.
*/
struct CMD_ClearEOC_TxTrigger
{
   /** Index */
   IFX_uint16_t Index;
   /** Length */
   IFX_uint16_t Length;
#if 1
   IFX_uint16_t P02_TxTrigger;
#else
/* #if (MEI_DRV_OS_BYTE_ORDER == MEI_DRV_OS_BIG_ENDIAN) */
   /** Reserved */
   IFX_uint16_t Res00 : 15;
   /** Transmit Control Trigger */
   IFX_uint16_t txTrigger : 1;
#endif
} __PACKED__ ;


/**
   This is the acknowledgement for CMD_ClearEOC_TxTrigger.
*/
struct ACK_ClearEOC_TxTrigger
{
   /** Index */
   IFX_uint16_t Index;
   /** Length */
   IFX_uint16_t Length;
} __PACKED__ ;


/**
   This message is used to write data to the ClearEOC write buffer of type
   MEI_ClearEOC_t. When the buffer is filled, the transmission is started applying
   CMD_ClearEOC_TxTrigger. If the message to transmit is longer than the mailbox size,
   a sequence of writes to the ClearEOC buffer has to be done before the transmission
   is started with CMD_ClearEOC_TxTrigger. When autonomous Tx status messaging is
   activated via CMD_ClearEOC_Configure, then the finished transmission is indicated by
   EVT_ClearEOCStatusGet.
*/
struct CMD_ClearEOC_Write
{
   /** Index */
   IFX_uint16_t Index;
   /** Length */
   IFX_uint16_t Length;
   /** Data */
   IFX_uint16_t Data[128];
} __PACKED__ ;


/**
   This message is the acknowledgement for CMD_ClearEOC_Write.
*/
struct ACK_ClearEOC_Write
{
   /** Index */
   IFX_uint16_t Index;
   /** Length */
   IFX_uint16_t Length;
} __PACKED__ ;


/**
   This message is used to read data from the ClearEOC buffer of type MEI_ClearEOC_t.
   The length of the actual Clear EOC message can be found in the buffer. Please refer
   to MEI_ClearEOC_t. The availability of data can either be checked via
   CMD_ClearEOCStatusGet in polling mode or it can be reported by an autonomous
   EVT_ClearEOCStatusGet message when data is received (to be enabled using
   CMD_ClearEOC_Configure).
*/
struct CMD_ClearEOC_Read
{
   /** Index */
   IFX_uint16_t Index;
   /** Length */
   IFX_uint16_t Length;
} __PACKED__ ;


/**
   This message is the acknowledgement to CMD_ClearEOC_Read.
*/
struct ACK_ClearEOC_Read
{
   /** Index */
   IFX_uint16_t Index;
   /** Length */
   IFX_uint16_t Length;
   /** Data */
   IFX_uint16_t Data[128];
} __PACKED__ ;


/**
   This message is an autonomous message that is generated when ClearEOC data was
   received and autonomous Clear EOC data messaging has been activated via
   CMD_ClearEOC_Configure. If the ClearEOC data does not fit in one message, then a
   sequence of messages is generated. The ClearEOC buffer is of type MEI_ClearEOC_t.
*/
struct EVT_ClearEOC_Read
{
   /** Index */
   IFX_uint16_t Index;
   /** Length */
   IFX_uint16_t Length;
   /** Data */
   IFX_uint16_t Data[128];
} __PACKED__ ;


/**
   This message is used to retrieve the status of the clear eoc data transmission.
*/
struct CMD_ClearEOCStatusGet
{
   /** Index */
   IFX_uint16_t Index;
   /** Length */
   IFX_uint16_t Length;
} __PACKED__ ;



#define ACK_CLEAREOCSTATUSGET_P02_TX_STAT          0x0003
#define ACK_CLEAREOCSTATUSGET_P02_RES00            0xFFFC
#define ACK_CLEAREOCSTATUSGET_P03_RX_STAT          0x0003
#define ACK_CLEAREOCSTATUSGET_P03_RES01            0xFFFC

/**
   This is the acknowledgement for CMD_ClearEOCStatusGet.
*/
struct ACK_ClearEOCStatusGet
{
   /** Index */
   IFX_uint16_t Index;
   /** Length */
   IFX_uint16_t Length;

#if 1
   IFX_uint16_t P02_TxStat;
   IFX_uint16_t P03_RxStat;
#else
/* #if (MEI_DRV_OS_BYTE_ORDER == MEI_DRV_OS_BIG_ENDIAN) */
   /** Reserved */
   IFX_uint16_t Res00 : 14;
   /** Transmit Status */
   IFX_uint16_t txstat : 2;
   /** Reserved */
   IFX_uint16_t Res01 : 14;
   /** Receive Status */
   IFX_uint16_t rxstat : 2;
#endif
} __PACKED__ ;


/**
   This autonomous message reports the Clear EOC status. It is sent only if the
   "Autonomous Status Message Control" was enabled for Tx and/or Rx direction with
   CMD_ClearEOC_Configure. If Tx direction is enabled, the message is generated when a
   Tx transmission is finished or failed. If Rx direction is enabled, the message is
   generated when the Rx status transitions from "Idle" to "Data Available" for
   retrieval by the host.
*/
struct EVT_ClearEOCStatusGet
{
   /** Index */
   IFX_uint16_t Index;
   /** Length */
   IFX_uint16_t Length;

#if 1
   IFX_uint16_t P02_TxStat;
   IFX_uint16_t P03_RxStat;
#else
/* #if (MEI_DRV_OS_BYTE_ORDER == MEI_DRV_OS_BIG_ENDIAN) */
   /** Reserved */
   IFX_uint16_t Res00 : 14;
   /** Transmit Status */
   IFX_uint16_t txstat : 2;
   /** Reserved */
   IFX_uint16_t Res01 : 14;
   /** Receive Status */
   IFX_uint16_t rxstat : 2;
#endif
} __PACKED__ ;


#ifdef __cplusplus
}
#endif
/** @} */
#endif      /* #ifndef _DRV_IF_VDSL2_CLEAR_EOC_H_ */

