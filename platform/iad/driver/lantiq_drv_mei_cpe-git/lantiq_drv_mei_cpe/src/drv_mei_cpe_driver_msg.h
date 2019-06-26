#ifndef _DRV_MEI_CPE_DRIVER_MSG_H
#define _DRV_MEI_CPE_DRIVER_MSG_H
/******************************************************************************

                          Copyright (c) 2007-2015
                     Lantiq Beteiligungs-GmbH & Co. KG

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/* ==========================================================================
   Description : VRX driver messages
   ========================================================================== */

/* ==========================================================================
   includes
   ========================================================================== */

/* get at first the driver configuration */
#include "drv_mei_cpe_config.h"

#if (MEI_SUPPORT_DRIVER_MSG == 1)

#include "drv_mei_cpe_interface.h"
#include "drv_mei_cpe_api.h"


/* ==========================================================================
   Macro defs - VRX Driver Message ID
   ========================================================================== */

/**
   The current driver interface supports VRX Driver messages

   The driver message ID hase the following structure:

   Bit[ 7.. 0]   message:     Message number
   Bit[15.. 8]   module:      0x00 = COMMON VRX
                              0x01 = COMMON MEIX-Control
                              0x02 = ROM (DOWNLOAD)
                              0x10 = REM ACCESS
   Bit[22..16]   type:        CMD/ACK = 0, EVT = 2, ALARM = 4
   Bit[23]       direction:   IN = 0, OUT = 1
   Bit[24..31]   Return Code: 0x00 = SUCCESS, (!= 0x00) = ERROR

   Currently the following message numbers are supported:
   - 0x01 - ROM Start (Event | Alarm)

   - 0x10 - Remote Firmware download done (Event | Alarm)

*/

/**
   VRX driver event "message number" mask and bit position
*/
#define MEI_DRV_MSG_MESSAGE_NUM_POS        0
#define MEI_DRV_MSG_MESSAGE_NUM_MASK       (0x000000FF << MEI_DRV_MSG_MESSAGE_NUM_POS)

/** VRX Driver Message - ROM Start */
#define MEI_DRV_MSG_MESSAGE_NUM_ROM_START           0x01
/** VRX Driver Message - Remote ATM OAM Extract alarm */
#define MEI_DRV_MSG_MESSAGE_NUM_REM_ATM_OAM_EXTR    0x20

/**
   VRX driver event "module" mask and bit position
*/
#define MEI_DRV_MSG_MODULE_POS             8
#define MEI_DRV_MSG_MODULE_MASK            (0x000000FF << MEI_DRV_MSG_MODULE_POS)

/** VRX Driver Message, MODULE - COMMON */
#define MEI_DRV_MSG_MODULE_COMMON          0x01
/** VRX Driver Message, MODULE - COMMON X-Control */
#define MEI_DRV_MSG_MODULE_COMMON_X        0x02
/** VRX Driver Message, MODULE - ROM (DOWNLOAD) */
#define MEI_DRV_MSG_MODULE_ROM             0x04
/** VRX Driver Message, MODULE - Remote ATM OAM Access */
#define MEI_DRV_MSG_MODULE_REM_ATM_ACCESS  0x20
/** VRX Driver Message, MODULE - Remote Clear EOC Access */
#define MEI_DRV_MSG_MODULE_REM_EOC_ACCESS  0x40

/**
   VRX driver event "type" mask and bit position
*/
#define MEI_DRV_MSG_TYPE_POS               16
#define MEI_DRV_MSG_TYPE_MASK              (0x0000007F << MEI_DRV_MSG_TYPE_POS)

/** VRX Driver Message, TYPE - CMD */
#define MEI_DRV_MSG_TYPE_CMD               0x00
/** VRX Driver Message, TYPE - ACK */
#define MEI_DRV_MSG_TYPE_ACK               0x00
/** VRX Driver Message, TYPE - EVT */
#define MEI_DRV_MSG_TYPE_EVT               0x02
/** VRX Driver Message, TYPE - ALARM */
#define MEI_DRV_MSG_TYPE_ALARM             0x04


/**
   VRX driver event "direction" mask and bit position
*/
#define MEI_DRV_MSG_DIRECTION_POS          23
#define MEI_DRV_MSG_DIRECTION_MASK         (0x00000001 << MEI_DRV_MSG_DIRECTION_POS)

/** VRX Driver Message, DIRECTION - IN */
#define MEI_DRV_MSG_DIRECTION_IN           0x0
/** VRX Driver Message, DIRECTION - OUT */
#define MEI_DRV_MSG_DIRECTION_OUT          0x1


/**
   VRX driver event "return code" mask and bit position
*/
#define MEI_DRV_MSG_RET_VAL_POS            24
#define MEI_DRV_MSG_RET_VAL_MASK           (0x000000FF << MEI_DRV_MSG_RET_VAL_POS)


/** Get the Driver Message MESSAGE_NUM information from a drv MsgID */
#define MEI_DRV_MSG_MESSAGE_NUM_GET(drvMsgId)\
            ((drvMsgId & MEI_DRV_MSG_MESSAGE_NUM_MASK) >> MEI_DRV_MSG_MESSAGE_NUM_POS)

/** Set the Driver Message MESSAGE_NUM information within a drv MsgID */
#define MEI_DRV_MSG_MESSAGE_NUM_SET(drvMsgId, newDrvMsgNum) drvMsgId = \
            ( (drvMsgId & ~(MEI_DRV_MSG_MESSAGE_NUM_MASK)) | \
              ((newDrvMsgNum << MEI_DRV_MSG_MESSAGE_NUM_POS) & (MEI_DRV_MSG_MESSAGE_NUM_MASK)) )

/** Get the Driver Message MODULE information from a drv MsgID */
#define MEI_DRV_MSG_MODULE_GET(drvMsgId)\
            ((drvMsgId & MEI_DRV_MSG_MODULE_MASK) >> MEI_DRV_MSG_MODULE_POS)

/** Set the Driver Message MODULE information within a drv MsgID */
#define MEI_DRV_MSG_MODULE_SET(drvMsgId, newModule) drvMsgId = \
            ( (drvMsgId & ~(MEI_DRV_MSG_MODULE_MASK)) | \
              ((newModule << MEI_DRV_MSG_MODULE_POS) & (MEI_DRV_MSG_MODULE_MASK)) )

/** Get the Driver Message TYPE information from a drv MsgID */
#define MEI_DRV_MSG_TYPE_GET(drvMsgId)\
            ((drvMsgId & MEI_DRV_MSG_TYPE_MASK) >> MEI_DRV_MSG_TYPE_POS)

/** Set the Driver Message TYPE information within a drv MsgID */
#define MEI_DRV_MSG_TYPE_SET(drvMsgId, newType) drvMsgId = \
            ( (drvMsgId & ~(MEI_DRV_MSG_TYPE_MASK)) | \
              ((newType << MEI_DRV_MSG_TYPE_POS) & (MEI_DRV_MSG_TYPE_MASK)) )

/** Get the Driver Message DIRECTION information from a drv MsgID */
#define MEI_DRV_MSG_DIRECTION_GET(drvMsgId)\
            ((drvMsgId & MEI_DRV_MSG_DIRECTION_MASK) >> MEI_DRV_MSG_DIRECTION_POS)

/** Set the Driver Message DIRECTION information within a drv MsgID */
#define MEI_DRV_MSG_DIRECTION_SET(drvMsgId, newDirection) drvMsgId = \
            ( (drvMsgId & ~(MEI_DRV_MSG_DIRECTION_MASK)) | \
              ((newDirection << MEI_DRV_MSG_DIRECTION_POS) & (MEI_DRV_MSG_DIRECTION_MASK)) )

/** Get the Driver Message RETVAL information from a drv MsgID */
#define MEI_DRV_MSG_RETVAL_GET(drvMsgId)\
            ((drvMsgId & MEI_DRV_MSG_DIRECTION_MASK) >> MEI_DRV_MSG_RET_VAL_POS)

/** Set the Driver Message RETVAL information within a drv MsgID */
#define MEI_DRV_MSG_RETVAL_SET(drvMsgId, newRetVal) drvMsgId = \
            ( (drvMsgId & ~(MEI_DRV_MSG_RET_VAL_MASK)) | \
              ((newRetVal << MEI_DRV_MSG_RET_VAL_POS) & (MEI_DRV_MSG_RET_VAL_MASK)) )




/* ==========================================================================
   VRX Driver Message - check aginst interface defines
   ========================================================================== */

#define MEI_DRV_MSG_EVT_ROM_START \
          (  (MEI_DRV_MSG_MESSAGE_NUM_ROM_START << MEI_DRV_MSG_MESSAGE_NUM_POS) \
           | (MEI_DRV_MSG_MODULE_COMMON << MEI_DRV_MSG_MODULE_POS) \
           | (MEI_DRV_MSG_TYPE_EVT << MEI_DRV_MSG_TYPE_POS) \
           | (MEI_DRV_MSG_DIRECTION_OUT << MEI_DRV_MSG_DIRECTION_POS) \
           | (0 << MEI_DRV_MSG_RET_VAL_POS) )

#if (MEI_DRV_MSG_EVT_ROM_START != MEI_DRV_MSG_IF_ROM_START_EVT)
#error "VRX Interface: missmatch for EVT_ROM_START"
#endif

#define MEI_DRV_MSG_ALM_ROM_START\
          (  (MEI_DRV_MSG_MESSAGE_NUM_ROM_START << MEI_DRV_MSG_MESSAGE_NUM_POS) \
           | (MEI_DRV_MSG_MODULE_COMMON << MEI_DRV_MSG_MODULE_POS) \
           | (MEI_DRV_MSG_TYPE_ALARM << MEI_DRV_MSG_TYPE_POS) \
           | (MEI_DRV_MSG_DIRECTION_OUT << MEI_DRV_MSG_DIRECTION_POS) \
           | (0 << MEI_DRV_MSG_RET_VAL_POS) )

#if (MEI_DRV_MSG_ALM_ROM_START != MEI_DRV_MSG_IF_ROM_START_ALM)
#error "VRX Interface: missmatch for ALM_ROM_START"
#endif

#define MEI_DRV_MSG_ALM_REM_ATM_OAM_EXTR \
          (  (MEI_DRV_MSG_MESSAGE_NUM_REM_ATM_OAM_EXTR << MEI_DRV_MSG_MESSAGE_NUM_POS) \
           | (MEI_DRV_MSG_MODULE_REM_ATM_ACCESS << MEI_DRV_MSG_MODULE_POS) \
           | (MEI_DRV_MSG_TYPE_ALARM << MEI_DRV_MSG_TYPE_POS) \
           | (MEI_DRV_MSG_DIRECTION_OUT << MEI_DRV_MSG_DIRECTION_POS) \
           | (0 << MEI_DRV_MSG_RET_VAL_POS) )

#if (MEI_DRV_MSG_ALM_REM_ATM_OAM_EXTR != MEI_DRV_MSG_IF_REM_ATM_OAM_EXTR_ALM)
#error "VRX Interface: missmatch for ALM_REM_ATM_OAM_EXTR"
#endif

extern IFX_int32_t MEI_DrvMsg_RomStartDistribute(
                              MEI_DEV_T              *pMeiDev,
                              MEI_DRV_STATE_E        prevDrvState);

#if (MEI_DRV_ATM_OAM_ENABLE == 1)
extern IFX_int32_t MEI_DrvMsg_RemAtmOamDistribute(
                              MEI_DEV_T    *pMeiDev,
                              IFX_uint32_t   linkNo,
                              IFX_uint32_t   extrFailedCnt );
#endif

#endif   /* #if (MEI_SUPPORT_DRIVER_MSG == 1) */
#endif   /* #ifndef _DRV_MEI_CPE_DRIVER_MSG_H */

