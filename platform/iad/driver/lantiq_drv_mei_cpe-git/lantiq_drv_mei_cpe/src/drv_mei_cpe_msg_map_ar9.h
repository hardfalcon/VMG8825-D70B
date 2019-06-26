/******************************************************************************

                          Copyright (c) 2007-2015
                     Lantiq Beteiligungs-GmbH & Co. KG

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/
#ifndef _DRV_MEI_CPE_MSG_MAP_AR9_H
#define _DRV_MEI_CPE_MSG_MAP_AR9_H

/* ==========================================================================
   Description : Message Map Handling between the driver and the AR9 device.
   ========================================================================== */

#ifdef __cplusplus
extern "C"
{
#endif

/* ============================================================================
   Inlcudes
   ========================================================================= */

/* get at first the driver configuration */
#include "drv_mei_cpe_config.h"

#include "ifx_types.h"
#include "drv_mei_cpe_os.h"
#include "drv_mei_cpe_api.h"
#include "drv_mei_cpe_mailbox.h"

/* ============================================================================
   Defines
   ========================================================================= */

#define CMV_GROUP_CNTL   0x01
#define CMV_GROUP_STAT   0x02
#define CMV_GROUP_INFO   0x03
#define CMV_GROUP_TEST   0x04
#define CMV_GROUP_OPTN   0x05
#define CMV_GROUP_RATE   0x06
#define CMV_GROUP_PLAM   0x07
#define CMV_GROUP_CNFG   0x08
#define CMV_GROUP_NA     0xFF

#define CMV_ME_HDLC_IDLE              0x0
#define CMV_ME_HDLC_MSG_QUEUED        0x2
#define CMV_ME_HDLC_MSG_SENT          0x3
#define CMV_ME_HDLC_RESP_RCVD         0x4
#define CMV_ME_HDLC_RESP_TIMEOUT      0x5
#define CMV_ME_HDLC_MSG_NOT_SUPPORTED 0x7
#define CMV_ME_HDLC_UNRESOLVED        0x1
#define CMV_ME_HDLC_RESOLVED          0x2

#define CMV_STAT_MODEM_STATUS_RESET       0x0
#define CMV_STAT_MODEM_STATUS_READY       0x1
#define CMV_STAT_MODEM_STATUS_FAIL        0x2
#define CMV_STAT_MODEM_STATUS_IDLE        0x3
#define CMV_STAT_MODEM_STATUS_GHS         0x5
#define CMV_STAT_MODEM_STATUS_FULL_INIT   0x6
#define CMV_STAT_MODEM_STATUS_SHOWTIME    0x7
#define CMV_STAT_MODEM_STATUS_RETRAIN     0x8
#define CMV_STAT_MODEM_STATUS_DIAG        0x9
#define CMV_STAT_MODEM_STATUS_SHORT_INIT  0xA
#define CMV_STAT_MODEM_STATUS_T1_413      0xB

#define MEI_MAX_CMV_MSG_LENGTH            16

/* ============================================================================
   Types
   ========================================================================= */

typedef struct
{
   /* Index field */
   IFX_uint16_t nIndex;
   /* Length field */
   IFX_uint16_t nLength;
} MEI_UserMsgHeader_t;


typedef IFX_int32_t (*MEI_AR9_MsgHandlerFunc_t)(
                              MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                              IOCTL_MEI_messageSend_t  *pUserMsgs,
                              IFX_boolean_t             bInternCall);


typedef struct
{
   /* Hercules MCAT Message ID*/
   IFX_uint16_t msgId;
   /* Hercules MCAT Message Handler*/
   MEI_AR9_MsgHandlerFunc_t pFunc;
} MEI_AR9_CmdEntry_t;

/* ============================================================================
   Macros
   ========================================================================= */

#define MEI_AR9_CMD_ADD(msgId, pHandler) {msgId, pHandler}

#define MEI_AR9_BIT_GET(val, bit_num)    ((val >> bit_num) & 0x1)

/* ============================================================================
   Map Message functions
   ========================================================================= */

IFX_int32_t MEI_AR9_MapMsgSend(
                              MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                              IOCTL_MEI_messageSend_t  *pUserMsgs,
                              IFX_boolean_t              bInternCall);


#ifdef __cplusplus
/* extern "C" */
}
#endif

#endif      /* _DRV_MEI_CPE_MSG_MAP_AR9_H */

