#ifndef _DRV_TEST_MEI_CPE_INTERFACE_H
#define _DRV_TEST_MEI_CPE_INTERFACE_H
/******************************************************************************

                          Copyright (c) 2007-2015
                     Lantiq Beteiligungs-GmbH & Co. KG

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/* ==========================================================================
   Description : Test Interface to user application of the VRX Driver
   ========================================================================== */

/* get the data definitions from the VRX driver */
#include "drv_vrx_interface.h"


/* ==========================================================================
   ioctl commands - access VRX Driver Test module
   ========================================================================== */

/** magic number */
#define MEI_TEST_IOC_MAGIC 'R'

/**
   This changes the level of debug outputs (This service is non-blocking).
*/
#define FIO_MEI_TEST_DBG_LEVEL                  _IO(MEI_TEST_IOC_MAGIC, 1)

/**
   This returns the version information.
*/
#define FIO_MEI_TEST_VERS_GET                   _IO(MEI_TEST_IOC_MAGIC, 2)


/* ==========================================================================
   ioctl commands - access VRX Driver module via test module
   ========================================================================== */

/** magic number */
#define MEI_TEST_IOC_MAGIC_INTERN 'S'

/**
   This changes the level of debug outputs (This service is non-blocking).
*/
#define FIO_MEI_INTERN_DBG_LEVEL                _IO(MEI_TEST_IOC_MAGIC_INTERN, 1)

/**
   This returns the version information.
*/
#define FIO_MEI_INTERN_VERS_GET                 _IO(MEI_TEST_IOC_MAGIC_INTERN, 2)

/**
   Internal Interface - Init a VRX device line.
*/
#define FIO_MEI_INTERN_INIT                     _IO(MEI_TEST_IOC_MAGIC_INTERN, 3)

/**
   Internal Interface - Close a VRX device line.
*/
#define FIO_MEI_INTERN_CLOSE                    _IO(MEI_TEST_IOC_MAGIC_INTERN, 4)

/**
   Internal Interface - This service resets the VRX Device Driver and optional
   the VRX blocks via MEI register.
*/
#define FIO_MEI_INTERN_RESET                    _IO(MEI_TEST_IOC_MAGIC_INTERN, 5)

/**
   Internal Interface - Firmware download to the VRX device.
*/
#define FIO_MEI_INTERN_FW_DL                    _IO(MEI_TEST_IOC_MAGIC_INTERN, 6)

/**
   Internal Interface - Select the firmware mode (VDSL2 / ADSL) and start.
*/
#define FIO_MEI_INTERN_FW_MODE_SELECT           _IO(MEI_TEST_IOC_MAGIC_INTERN, 7)

/**
   Internal Interface - Requests the VRX Driver config.
*/
#define FIO_MEI_INTERN_REQ_CONFIG               _IO(MEI_TEST_IOC_MAGIC_INTERN, 11)

/**
   Internal Interface - Requests the VRX Driver statistic.
*/
#define FIO_MEI_INTERN_REQ_STAT                 _IO(MEI_TEST_IOC_MAGIC_INTERN, 12)

/**
   Internal Interface - Set an MEI register.
*/
#define FIO_MEI_INTERN_REG_SET                  _IO(MEI_TEST_IOC_MAGIC_INTERN, 13)

/**
   Internal Interface - Get an MEI register.
*/
#define FIO_MEI_INTERN_REG_GET                  _IO(MEI_TEST_IOC_MAGIC_INTERN, 14)

/**
   Internal Interface - General Purpose Access Write to the target.
   Internal memory or AUX register contents can be written.
*/
#define FIO_MEI_INTERN_GPA_WRITE                _IO(MEI_TEST_IOC_MAGIC_INTERN, 15)

/**
   Internal Interface - General Purpose Access Read from the target.
   Internal memory or AUX register contents can be read.
*/
#define FIO_MEI_INTERN_GPA_READ                 _IO(MEI_TEST_IOC_MAGIC_INTERN, 16)


/**
   Internal Interface - write to the VRX via MEI (debug write)
*/
#define FIO_MEI_INTERN_DBG_WRITE                _IO(MEI_TEST_IOC_MAGIC_INTERN, 17)

/**
   Internal Interface - read from the VRX via MEI (debug read)
*/
#define FIO_MEI_INTERN_DBG_READ                 _IO(MEI_TEST_IOC_MAGIC_INTERN, 18)


/**
   Internal Interface - NFC's enable
*/
#define FIO_MEI_INTERN_MBOX_NFC_ENABLE          _IO(MEI_TEST_IOC_MAGIC_INTERN, 21)

/**
   Internal Interface - NFC's disable
*/
#define FIO_MEI_INTERN_MBOX_NFC_DISABLE         _IO(MEI_TEST_IOC_MAGIC_INTERN, 22)


/**
   Internal Interface - set VRX driver message control for autonomous messages.
*/
#define FIO_MEI_INTERN_AUTO_MSG_CTRL_SET        _IO(MEI_TEST_IOC_MAGIC_INTERN, 23)


/**
   Internal Interface - get VRX driver message control for autonomous messages.
*/
#define FIO_MEI_INTERN_AUTO_MSG_CTRL_GET        _IO(MEI_TEST_IOC_MAGIC_INTERN, 24)

/**
   Internal Interface - write message (see message catalog)
*/
#define FIO_MEI_INTERN_MBOX_MSG_WR              _IO(MEI_TEST_IOC_MAGIC_INTERN, 31)

/**
   Internal Interface - check for and read ACK message
*/
#define FIO_MEI_INTERN_MBOX_ACK_RD              _IO(MEI_TEST_IOC_MAGIC_INTERN, 32)

/**
   Internal Interface - send message and wait for ACK
*/
#define FIO_MEI_INTERN_MBOX_MSG_SEND            _IO(MEI_TEST_IOC_MAGIC_INTERN, 33)

/**
   Internal Interface - check for and read NFC message
*/
#define FIO_MEI_INTERN_MBOX_NFC_RD              _IO(MEI_TEST_IOC_MAGIC_INTERN, 34)


/* ==========================================================================
   ioctl commands - map VRX Driver data structures
   ========================================================================== */

/**
   ioctl structure for set the test module debug level.
*/
typedef struct
{
   /** control data of the ioctl */
   IOCTL_MEI_ioctl_t ictl;

   /** new debug level to set */
   unsigned int   valLevel;
} IOCTL_MEI_TEST_dbgLevel_t;

/**
   ioctl structure for get the test module version string.
*/
typedef struct
{
   /** control data of the ioctl */
   IOCTL_MEI_ioctl_t ictl;

   /** points to the user version string */
   char           *pVersionStr;
   /** size of the user version string */
   unsigned int   strSize;

   /** driver version id (hex) */
   unsigned int   versionId;
} IOCTL_MEI_TEST_drvVers_t;


/** encapsulate all ioctl command arguments (VRX devices) */
typedef union
{
   IOCTL_MEI_ioctl_t                 tst_ioctl;
   IOCTL_MEI_TEST_dbgLevel_t           tst_dbgLevel;
   IOCTL_MEI_TEST_drvVers_t            tst_version;
   IOCTL_MEI_dbgLevel_t              intern_dbgLevel;
   IOCTL_MEI_drvVersion_t            intern_drvVers;
   IOCTL_MEI_devInit_t               intern_devInit;
   IOCTL_MEI_reset_t                 intern_devRst;
   IOCTL_MEI_reqCfg_t                intern_reqCfg;
   IOCTL_MEI_statistic_t             intern_reqStat;
   IOCTL_MEI_fwDownLoad_t            intern_fwDl;
   IOCTL_MEI_fwMode_t                intern_fwMode;
   IOCTL_MEI_regInOut_t              intern_regIo;
   IOCTL_MEI_GPA_accessInOut_t       intern_GpaAccess;
   IOCTL_MEI_dbgAccess_t             intern_DbgAccess;
   IOCTL_MEI_autoMsgCtrl_t           intern_autoMsgCtrl;
   IOCTL_MEI_message_t               intern_ifxMsg;
   IOCTL_MEI_messageSend_t           intern_ifxMsgSend;
} IOCTL_MEI_TEST_internArg_t;



#endif      /* #ifndef _DRV_TEST_MEI_CPE_INTERFACE_H */

