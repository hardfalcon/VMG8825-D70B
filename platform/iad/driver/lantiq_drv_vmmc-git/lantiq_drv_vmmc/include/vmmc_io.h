#ifndef _VMMC_IO_H
#define _VMMC_IO_H
/****************************************************************************

                              Copyright (c) 2012
                            Lantiq Deutschland GmbH
                             http://www.lantiq.com

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

*******************************************************************************/

/**
   \file vmmc_io.h
   This file contains the types and the defines for the driver user interface.
*/

/** \defgroup VMMC_DRIVER_INTERFACE Driver Interface
    Lists the entire interface to the VMMC Driver */
/*@{*/

/** \defgroup VMMC_DRIVER_INTERFACE_BASIC Basic Interface
    Basic VMMC Access routines as command read and write and initialisation */

/** \defgroup VMMC_KERNEL_INTERFACE Driver Kernel Interface
    Kernel Interface for GPIO/IO pin handling */

/** \defgroup VMMC_DRIVER_INTERFACE_CPE_LT Line Testing Interface CPE
    Interfaces for low level line testing functions for CPE driver. */

/** \defgroup VMMC_DRIVER_INTERFACE_LT Line Testing Interface LineCard
    Interfaces for low level line testing functions for Line Card driver. */

/** \defgroup VMMC_DRIVER_INTERFACE_INIT Driver Initialization Interface */

/*@}*/

/** \defgroup VMMC_DRIVER_INTERFACE_GPIO GPIO Interface
    Control the device and channel specific IO pins */


/** VMMC Chip Revision */
typedef enum
{
   /** Version V1 of VMMC INCAIP2 */
   VMMC_INCAIP2_V1 = 0x01
} VMMC_IO_CHIP_REVISION;

/** VMMC chip types, depending on register REVISION and
    firmware download */
typedef enum
{
   /** chip type INCAIP2 */
   VMMC_TYPE_INCAIP2 = 0x5
} VMMC_IO_CHIP_TYPE;

/** Maximal size of HDLC frame */
#define VMMC_HDLC_MAX_PACKAGE_SIZE 264
/** Minimal size of HDLC frame */
#define VMMC_HDLC_MIN_PACKAGE_SIZE 4


/** \addtogroup VMMC_DRIVER_INTERFACE_BASIC */
/*@{*/

/** Flag for \ref VMMC_IO_INIT to avoid no board configuration.
    Internal use only and will be removed */
#define NO_BCONF            0x00000001
/** Flag for \ref VMMC_IO_INIT to avoid EDSP start.
    Internal use only */
#define NO_EDSP_START       0x00000002
#define VOIP_FILE_MODE      0x00000004
/** Flag for \ref VMMC_IO_INIT to avoid PHI download in case of V1.4.
    Internal use only */
#define NO_PHI_DWLD         0x00000008
/** Flag for \ref VMMC_IO_INIT to avoid CRAM download in case of V1.4.  */
#define NO_CRAM_DWLD        0x00000010
/** Flag for \ref VMMC_IO_INIT to send the auto download command
    in case of V1.4   */
#define FW_AUTODWLD         0x00000020
/** Flag for \ref VMMC_IO_INIT to avoid AC download in case of V1.4.
    No effect with any other chip version */
#define NO_AC_DWLD          0x00000080
/** Flag for \ref VMMC_IO_INIT to avoid firmware download */
#define NO_FW_DWLD          0x00000100
/** Flag for \ref VMMC_IO_INIT to perform a DC control download in case
   of V1.4. Needed for testing  */
#define DC_DWLD             0x00000200

/** reports device reset to TAPI device specific status */
#define TAPI_DEVRESET 0x01
/*@}*/

/* ============================= */
/* VMMC ioctl access          */
/* ============================= */

/* general mode settings (set or get information) */
#define  IOSET    0
#define  IOGET    1
#define  IOMODIFY 2

/* ============================= */
/* VMMC ioctl Defines         */
/* ============================= */

/* magic number */
#define IFX_DEV_IOC_MAGIC 'V'

/* IOCMD global defines */

/** \addtogroup VMMC_DRIVER_INTERFACE_BASIC */
/*@{*/

/** Read relevant version information.
   The parameter points to a \ref VMMC_IO_VERSION structure.

   A decoding example:
   \code

   char chipType[][9] = {"INCA-IP2", "Danube", "Twinpass", "SVIP", "AR9"};
   char chipvers[][6] = {"V1.1", "V1.2", "V1.3", "V1.4", "V2.1", "V2.2"};
   char *sChipVers;

      printf("%s voice FW %d.%d.%d\n\r",
            chipType[ioCmd.nType],
            ioCmd.nEdspVers,
            ioCmd.nEdspIntern,
            ioCmd.nEDSPHotFix);

      if (ioCmd.nType == 4)
      {
         printf("feat. VMMC DCCtrl %d.%d, ASDSP %d.%d, "
                "SmartSLIC HW rev %d, id 0x%04x\n\r",
                  ioCmd.nDCCtrlVers, ioCmd.nDCCtrlStep,
                  ioCmd.nASDSPVers, ioCmd.nASDSPStep,
                  ioCmd.nSmartSLIC_HW_rev, ioCmd.nSmartSLIC_DevId
               );
      }
   \endcode
  */
#define FIO_GET_VERS              _IO(IFX_DEV_IOC_MAGIC,  7)

/** Set the driver report levels if the driver is compiled with
    ENABLE_TRACE
    \remarks
    valid arguments are
    \arg 0: off
    \arg 1: low, high output
    \arg 2: normal, general information and warnings
    \arg 3: high, only errors are reported      */
#define FIO_REPORT_SET            _IO(IFX_DEV_IOC_MAGIC, 10)

/** Write a VMMC command.
   The parameter points to a \ref VMMC_IO_MB_CMD structure.
   \note This interface is only for debugging and testing purposes.
   The use of this interface may disturb the driver operation */
#define FIO_WRITE_CMD                  _IO(IFX_DEV_IOC_MAGIC, 11)

/** Read command.
   The parameter points to a \ref VMMC_IO_MB_CMD structure
   \note This interface is only for debugging and testing purposes.
   The use of this interface may disturb the driver operation

   \code
   VMMC_IO_MB_CMD ioCmd;
   int err;

   ioCmd.cmd1 = 0x8300 | ch;
   //read OPMODE_CUR
   ioCmd.cmd2 = 0x2101;

   err = ioctl (fd, FIO_RCMD, (INT) &ioCmd);
   \endcode
    */
#define FIO_READ_CMD                  _IO(IFX_DEV_IOC_MAGIC, 12)

/** Download Firmware to the VMMC device.
   The parameter is a pointer to a \ref VMMC_IO_INIT structure
   \code
   VMMC_IO_INIT IoInit;

   memset (&IoInit, 0, sizeof (IoInit));
   IoInit.pPRAMfw = ...
   IoInit.pram_size = ...
   ....
   ret = ioctl(cmd_fd, FIO_FW_DOWNLOAD, (int)&IoInit);
   \endcode
   */
#define FIO_FW_DOWNLOAD         _IO(IFX_DEV_IOC_MAGIC, 15)

/** Get the last error that occured. The error codes are listed in
   drv_vmmc_errno.h and are enumerated in \ref VMMC_DEV_ERR.
   \code
   ioctl (fd, FIO_LASTERR, (INT) &err);
   \endcode
    */
#define FIO_LASTERR               _IO(IFX_DEV_IOC_MAGIC, 16)
/*@}*/


/** \addtogroup VMMC_DRIVER_INTERFACE_LT */
/** @{ */

/** Read host register (INCAIP2).
   The parameter points to a \ref VMMC_IO_REG_ACCESS structure
   \note This interface is only for debugging and testing purposes.
   Using of this interface may disturb the driver operation

   \code
   VMMC_IO_REG_ACCESS ioCmd;
   int err;

   ioCmd.offset = 0x0C; // register STAT_INT

   err = ioctl (fd, FIO_RDREG, (INT) &ioCmd);
   \endcode
*/
#define FIO_RDREG                 _IO(IFX_DEV_IOC_MAGIC, 36)

/** Write ro host register (INCAIP2, only)
   The parameter points to a \ref VMMC_IO_REG_ACCESS structure
   \note This interface is provided for debugging and testing purposes.
   Using of this interface may disturb the driver operation */
#define FIO_WRREG                 _IO(IFX_DEV_IOC_MAGIC, 37)

/*@}*/

/** \addtogroup VMMC_DRIVER_INTERFACE_INIT */
/*@{*/

/** Reset Voice FW CPU/VPE. */
#define FIO_DEV_RESET               _IO(IFX_DEV_IOC_MAGIC, 39)

/** Restart Voice FW CPU/VPE. */
#define FIO_DEV_RESTART             _IO(IFX_DEV_IOC_MAGIC, 40)

/** Does a download according to bbd format.
    Parameter is a pointer to a VMMC_DWLD_t buffer */
#define FIO_BBD_DOWNLOAD             _IO(IFX_DEV_IOC_MAGIC, 41)
/*@}*/

/* ============================= */
/* RTP packet defines            */
/* ============================= */

#define RTP_PT 0x007F

#define VMMC_IO_LINEMODE_MASK 0x0F00
#define VMMC_IO_LINESUB_MASK  0x00F0

typedef enum
{
   VMMC_IO_LM_PDNH         = 0x0000,
   VMMC_IO_LM_RING_PAUSE   = 0x0100,
   VMMC_IO_LM_ACTIVE       = 0x0200,
   VMMC_IO_LM_SLEEP_PDNR   = 0x0300,
   VMMC_IO_LM_RING_BURST   = 0x0400,
   VMMC_IO_LM_ACT_METERING = 0x0500,
   VMMC_IO_LM_PDNR         = 0x0700
}  VMMC_IO_LINEMODES;

/* ============================= */
/* Global Structures             */
/* ============================= */

/** \addtogroup VMMC_DRIVER_INTERFACE_BASIC */
/*@{*/

/** VMMC User Interface Structure.
   Used for all firmware messages and if messages
   for input and output are very different, where a
   simple structure will not be optimal  */
typedef struct
{
   /* Byte Data Buffer from Task */
   unsigned char *pWrBuf;
   /* Size of Byte Data Buffer  */
   unsigned long nWrCnt;
   /* Byte Data Buffer to Task  */
   unsigned char *pRdBuf;
   /* Number of Data to Task */
   unsigned long nRdCnt;
} VMMC_IO_USR;

/* ============================= */
/* Version Request               */
/* ============================= */

/** Version Io structure */
typedef struct
{
   /** chip type (0:INCA2, 1:Danube, 4:AR9) */
   unsigned char nType;
   /** number of supported analog channels */
   unsigned char nChannel;
   /** chip revision */
   unsigned short nChip;
   /** TAPI version */
   unsigned long nTapiVers;
   /** driver version */
   unsigned long nDrvVers;
   /* The full EDSP firmware version number is:
      nEdspVers.nEdspIntern.nEDSPHotFix.nType.nEDSPVariant */
   /** EDSP major version */
   unsigned short nEdspVers;
   /** EDSP version step */
   unsigned short nEdspIntern;
   /** EDSP hotfix version (counted up for bugfixes) */
   unsigned short nEDSPHotFix;
   /** EDSP version variant */
   unsigned short nEDSPVariant;
   /** EDSP build timestamp (0 if not set) */
   unsigned int   nEDSPTimestamp;

   /** DCCTRL version/version step (if applicable, SmartSLIC only) */
   unsigned short nDCCtrlVers;
   unsigned short nDCCtrlStep;
   unsigned short nDCCtrlHotFix;

   /** ASDSP version/version step (if applicable, SmartSLIC only) */
   unsigned short nASDSPVers;
   unsigned short nASDSPStep;

   /** SmartSLIC HW revision and device ID (if applicable, SmartSLIC only) */
   unsigned short nSmartSLIC_HW_rev;
   unsigned short nSmartSLIC_DevId;
} VMMC_IO_VERSION;

/* ============================= */
/* Initialization                */
/* ============================= */

/** structure used for device initialization
 */
typedef struct
{
   /** Firmware PRAM pointer or NULL if not needed */
   /* PRAM means Program RAM of firmware in VMMC (INCAIP2). winder*/
   unsigned char *pPRAMfw ;
   /** size of PRAM firmware in bytes */
   unsigned long pram_size;
   /** pointer to block based download format data */
   unsigned char *pBBDbuf ;
   /** size of block based download buffer */
   unsigned long bbd_size;
   /** Flags for initialization. Most of the flags are only used from
       experts to modify the default initialization.
   \arg NO_BCONF       : no board configuration will be done. VMMC
                         must be properly got out of reset
   \arg NO_PHI_DWLD    : no PHI download will be done, a properly PHI
                         download before is assumed
   \arg NO_EDSP_START  : no EDPS start is done. The VMMC will not work
                         until that command is given
   \arg NO_CRAM_DWLD : no CRAM coefficients are downloaded. The default
                         ROM coefficients are used
   \arg FW_AUTODWLD    : firmware auto download
   \arg NO_AC_DWLD     : avoid AC download in case of V1.4 or INCAIP2, no effect
                         with any other chip version
   \arg DC_DWLD        : do a DC download in case of V1.4
   \arg NO_FW_DWLD     : avoid firmware download in case of V1.5 for example */
   unsigned long nFlags;
   /** return values of PHI after PHI program download. 0 if not done */
   unsigned short nPhiCrc;
   /** return values of DC control after DC control download. 0 if not done */
   unsigned short nDcCrc;
   /** return values of AC control after AC control download. 0 if not done */
   unsigned short nAcCrc;
} VMMC_IO_INIT;


/* ============================= */
/* Basic Access                  */
/* ============================= */

/** IO structure for read short commands
  */
typedef struct
{
   /** if set to 1 a broadcast on all channel will be done.
      Not available for every command */
   unsigned char nBc;
   /** write command */
   unsigned short nCmd;
   /** channel id */
   unsigned char nCh;
   /** read words count */
   unsigned char count;
   /** read data */
   unsigned short pData [256];
} VMMC_IO_READ_SC;

/** IO structure for writing short commands */
typedef struct
{
   /** if set to 1 a broadcast on all channel will be done.
      Not available for every command */
   unsigned char nBc;
   /** write command */
   unsigned short nCmd;
   /** channel id */
   unsigned char nCh;
} VMMC_IO_WRITE_SC;

/** IO structure for write and read chip commands for debugging
    purposes only */
typedef struct
{
   /** command according users manual */
   IFX_uint32_t cmd;
   /** read or write data */
   IFX_uint32_t pData [128];
} VMMC_IO_MB_CMD;

/** IO structure used for direct register access. Applicable to INCAIP2, only */
typedef struct
{
   /** offset to host register */
   unsigned short offset;
   /** number of host registers to read/write */
   unsigned short count;
   /** contains written/read data */
   unsigned long pData[100];
} VMMC_IO_REG_ACCESS;


/* ============================= */
/* Download FPI                  */
/* ============================= */

typedef struct
{
   /* FPI Start Address */
   unsigned long nStartAddr;
   /* FPI Stop Address */
   unsigned long nStopAddr;
   /* Data Size*/
   int nSize;
   /* Data Buffer of 16 kB */
   unsigned short  *pData;
   /*return value: CRC*/
   unsigned short  nCrc;
} VMMC_IO_FPI_DOWNLOAD;


/* ============================= */
/* Download PHI                  */
/* ============================= */
typedef struct
{
   /* Data Buffer for PHI */
   unsigned short *pData;
   /* Data Size */
   unsigned long nSize;
   /* returned PHI CRC read from VMMC */
   unsigned short nCrc;
} VMMC_IO_PHI;

/* ============================= */
/* Download CRAM                 */
/* ============================= */
/** maximum size */
#define VMMC_IO_CRAM_DATA_SIZE     250    /* Words */
/** CRAM file format version */
typedef enum
{
   /** CRAM download format for V1.6 and older */
   VMMC_IO_CRAM_FORMAT_1,
   /** CRAM download format 2.1 for VMMC V2.x */
   VMMC_IO_CRAM_FORMAT_2_1,
   /** CRAM download format 2.2 for VMMC V2.x */
   VMMC_IO_CRAM_FORMAT_2_2
} VMMC_IO_CRAM_FORMAT;

/** CRAM download related setting of register values (used for activation) */
typedef struct
{
   /** contains the values of the bits that should be changed (see nMask) */
   unsigned short             nData;
   /** defines which bits should be modified to the corresponding
       value in nData by setting them to 1 */
   unsigned short             nMask;
} VMMC_IO_CRAM_REG_CFG;

/** Structure for CRAM download with the V1.4 and the V2.x format.
   This structure is used for the ioctl FIO_VMMC_DOWNLOAD_CRAM. */
typedef struct
{
   /** set to 1 if the CRAM coefficients should be downloaded
       to all channels, if set to 0, the CRAM download is only done on
       one channel (according to the filedescriptor) */
   unsigned short             bBroadCast;
   /** defines the CRAM download format version */
   VMMC_IO_CRAM_FORMAT        nFormat;
   /** start offset of the CRAM coefficients in this download */
   unsigned char              nStartAddr;
   /** number of coefficients to be downloaded in (16bit) Words */
   unsigned long              nLength;
   /** coefficients array to be downloaded */
   unsigned short             aData [VMMC_IO_CRAM_DATA_SIZE];
   /** CRAM coefficient specific settings for bcr1 register */
   VMMC_IO_CRAM_REG_CFG       bcr1;
   /** CRAM coefficient specific settings for bcr2 register */
   VMMC_IO_CRAM_REG_CFG       bcr2;
   /** CRAM coefficient specific settings for tstr2 register */
   VMMC_IO_CRAM_REG_CFG       tstr2;
   /** CRAM CRC checksum */
   unsigned short             nCRC;
} VMMC_IO_CRAM_t;

/** VMMC download format */
typedef struct
{
   /** pointer to download buffer */
   IFX_uint8_t *buf;
   /** size of buffer in bytes */
   IFX_uint32_t size;
} VMMC_DWLD_t;

/** Embedded Controller Download structure */
typedef struct
{
   /** Data buffer */
   unsigned long *pData;
   /** Amount of Data Size in bytes */
   unsigned long nSize;
} VMMC_IO_EMBDCTRL;

/** @} */


/* =============================== */
/*    GPIO  Interface              */
/* =============================== */

/** \addtogroup VMMC_DRIVER_INTERFACE_GPIO */
/*@{*/

/** GPIO Control Structure */
typedef struct
{
   /** GPIO handle. The GPIO handle is returned with the successful reservation
       of GPIO pins. The handle has to be passed to all other GPIO commands. */
   int ioHandle;
   /** GPIO status, refer to the GPIO command for a description of this parameter */
   unsigned short nGpio;
   /** GPIO mask, refer to the GPIO command for a description of this parameter  */
   unsigned short nMask;
} VMMC_IO_GPIO_CONTROL;

/*@}*/

#endif /* _VMMC_IO_H */
