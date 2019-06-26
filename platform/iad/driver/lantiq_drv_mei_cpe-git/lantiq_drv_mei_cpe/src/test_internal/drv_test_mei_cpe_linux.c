/******************************************************************************

                          Copyright (c) 2007-2015
                     Lantiq Beteiligungs-GmbH & Co. KG

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/* ==========================================================================
   Description : VRX Driver Test, Linux part - internal interface.
   ========================================================================== */

#ifdef LINUX

/* ==========================================================================
   includes - LINUX
   ========================================================================== */

/* get at first the VRX driver configuration */
#include "drv_mei_cpe_config.h"

#include "ifx_types.h"
#include "drv_mei_cpe_os.h"

#ifdef __KERNEL__
   #include <linux/kernel.h>
#endif
#ifdef MODULE
   #include <linux/module.h>
#endif

#include <linux/version.h>
#include <linux/init.h>

#include <linux/ioport.h>
#include <linux/irq.h>
#include <asm/io.h>

#ifdef CONFIG_DEVFS_FS
#include <linux/devfs_fs_kernel.h>
#endif /* CONFIG_DEVFS_FS */

#if CONFIG_PROC_FS
#include <linux/proc_fs.h>
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
   #undef CONFIG_DEVFS_FS
#endif

/* add VRX debug/printout part */
#include "drv_mei_cpe_dbg.h"
#include "drv_mei_cpe_interface.h"
#include "drv_mei_cpe_api_intern.h"

#include "drv_test_mei_cpe_linux.h"
#include "drv_test_mei_cpe_interface.h"


/* ==========================================================================
   Local function declarations
   ========================================================================== */

static int __init MEI_TEST_module_init(void);
static void __exit MEI_TEST_module_exit(void);

static int MEI_TEST_RegCharDev(const char *devName);
static int MEI_TEST_Open(struct inode *inode, struct file *filp);
static int MEI_TEST_Release(struct inode *inode, struct file *filp);
static ssize_t MEI_TEST_Write(struct file *filp, const char *buf,
                              size_t count, loff_t * ppos);
static ssize_t MEI_TEST_Read(struct file *filp, char *buf, size_t count, loff_t * ppos);
static int MEI_TEST_Ioctl( struct inode *inode, struct file *filp,
                                   unsigned int nCmd, unsigned long nArgument);
static unsigned int MEI_TEST_Poll (struct file *filp, poll_table *wait);

#if CONFIG_PROC_FS
static int MEI_TEST_InstallProcEntry(unsigned char devNum);
static int MEI_TEST_GetVersionProc(char *buf);
static int MEI_TEST_ReadProc(char *page, char **start, off_t off,
                              int count, int *eof, void *data);
#endif


/* ==========================================================================
   Variable definitions
   ========================================================================== */

/* VRX-Driver: Common debug module - create print level variable */
MEI_DRV_PRN_USR_MODULE_CREATE(MEI_TEST, MEI_DRV_PRN_LEVEL_LOW);
MEI_DRV_PRN_INT_MODULE_CREATE(MEI_TEST, MEI_DRV_PRN_LEVEL_LOW);

static IFX_uint8_t major_number = 0;
MODULE_PARM(major_number, "b");
MODULE_PARM_DESC(major_number, "to override automatic major number");


/** install parameter debug_level: LOW (1), NORMAL (2), HIGH (3), OFF (4) */
static IFX_uint8_t debug_level = MEI_DRV_PRN_LEVEL_HIGH;
MODULE_PARM(debug_level, "b");
MODULE_PARM_DESC(debug_level, "set to get more (1) or fewer (4) debug outputs");


/* the driver callbacks */
static struct file_operations MEI_TEST_fops =
{
   owner:
      THIS_MODULE,
   read:
      MEI_TEST_Read,
   write:
      MEI_TEST_Write,
   poll:
      MEI_TEST_Poll,
   ioctl:
      MEI_TEST_Ioctl,
   open:
      MEI_TEST_Open,
   release:
      MEI_TEST_Release
};


#ifdef CONFIG_DEVFS_FS
/** handles for Dev FS */
static devfs_handle_t MEI_TEST_base_dir_handle;
static devfs_handle_t MEI_TEST_dev_handle[MEI_MAX_DFE_CHAN_DEVICES];
#endif

/**
   device structs for internal handling
*/
MEI_TEST_dev_t MEI_TEST_dev;


/* ==========================================================================
   Local function definitions
   ========================================================================== */


/**
   Initialize the module (support devfs - device file system)

   \return
   Error code or 0 on success
   \remark
   Called by the kernel.
*/
static int __init MEI_TEST_module_init (void)
{
   int result;

   printk(KERN_INFO "%s", &MEI_TEST_DRV_WHAT_STR[4]);
   printk(KERN_INFO ", (c) 2013 Lantiq Deutschland GmbH" MEI_DRV_CRLF);

   MEI_DRV_PRN_USR_LEVEL_SET(MEI_TEST, debug_level);
   MEI_DRV_PRN_INT_LEVEL_SET(MEI_TEST, debug_level);


   /* ============================================
      register device
      ============================================ */
   result = MEI_TEST_RegCharDev(MEI_TEST_DRV_NAME);
   if (result != 0)
   {
      MEI_TEST_module_exit();
      return (result);
   }

   /* ============================================
      Do common init_module() stuff
      ============================================ */
   memset(&MEI_TEST_dev, 0x00, sizeof(MEI_TEST_dev_t));

#if CONFIG_PROC_FS
   /* create and install proc file system */
   MEI_TEST_InstallProcEntry(0);
#endif

   return 0;
}

/**
   Clean up the module if unloaded.

   \remark
   Called by the kernel.
*/
static void __exit MEI_TEST_module_exit(void)
{
   int meiLine;
   char path[255];

   for (meiLine = 0; meiLine < MEI_DFE_CHAN_DEVICES; meiLine++)
   {
      if (MEI_TEST_dev.pVrxTestLine[meiLine] != IFX_NULL)
      {
         MEI_InternalDevClose(MEI_TEST_dev.pVrxTestLine[meiLine]);
         MEI_TEST_dev.pVrxTestLine[meiLine] = IFX_NULL;
      }
   }

#ifdef CONFIG_DEVFS_FS

   /* remove VRX channel devices */
   for (meiLine = 0; meiLine < MEI_DFE_CHAN_DEVICES; meiLine++)
   {
      if (MEI_TEST_dev_handle[meiLine])
      {
         PRN_DBG_USR( MEI_TEST, MEI_DRV_PRN_LEVEL_HIGH, meiLine,
               ("MEI_TEST: removing device /%s/%d/" MEI_DRV_CRLF,
                MEI_TEST_DRV_NAME, meiLine));
         devfs_unregister (MEI_TEST_dev_handle[meiLine]);
      }
   }

#else
   unregister_chrdev ( major_number , MEI_TEST_DRV_NAME );
#endif

#if CONFIG_PROC_FS
   PRN_DBG_USR_NL( MEI_TEST, MEI_DRV_PRN_LEVEL_HIGH,
         ("MEI_DRV: cleanup procfs: /%s" MEI_DRV_CRLF, MEI_TEST_DRV_NAME) );

   sprintf(path, "%s%s%s", "driver/", MEI_TEST_DRV_NAME, "/version");
   remove_proc_entry(path ,0);
   sprintf(path, "%s%s", "driver/", MEI_TEST_DRV_NAME);
   remove_proc_entry(path ,0);
#endif
   PRN_DBG_USR_NL( MEI_TEST, MEI_DRV_PRN_LEVEL_NORMAL,
         ("MEI_DRV: cleanup done" MEI_DRV_CRLF));

   /* touch one time this variable to avoid that the linker will remove it */
   debug_level = MEI_DRV_PRN_LEVEL_OFF;
}


/**
   Register the given device to the device node.

\param
   devName  device name

\return
   0: success
   <0 in case of errors
*/
static int MEI_TEST_RegCharDev(const char *devName)
{
#if CONFIG_DEVFS_FS
   /* ============================================
      create and setup devfs, register device
      ============================================ */
   int   meiLine;
   char  buf[16];

   memset (&(MEI_TEST_dev_handle), 0x00, sizeof(MEI_TEST_dev_handle));

   if (!(MEI_TEST_base_dir_handle = devfs_mk_dir(NULL, devName, NULL)))
   {
      PRN_DBG_USR_NL( MEI_TEST, MEI_DRV_PRN_LEVEL_HIGH,
            ("MEI_DRV: Unable to create %s base directory in dev_fs" MEI_DRV_CRLF,
              devName));
      return IFX_ERROR;
   }

   /* create VRX channel devices */
   for (meiLine=0; meiLine < MEI_DFE_CHAN_DEVICES; meiLine++)
   {
      /* add VRX channel devices to dev fs */
      sprintf (buf, "%d", meiLine);

      /* private_data contains device number for open function
         instead of minor number (dynamically assigned by dev_fs) */
      if ( ( MEI_TEST_dev_handle[meiLine] =
                  devfs_register(
                        MEI_TEST_base_dir_handle,
                        buf,
                        DEVFS_FL_DEFAULT /* DEVFS_FL_AUTO_DEVNUM */,
                        major_number,
                        meiLine,
                        S_IFCHR | S_IRUGO | S_IWUGO,
                        &MEI_TEST_fops,
                        (void*)( 0 )) ) == NULL)
      {
         PRN_DBG_USR_NL( MEI_TEST, MEI_DRV_PRN_LEVEL_HIGH,
                ("MEI_DRV: unable to add device %s/%s to dev_fs" MEI_DRV_CRLF,
                 devName, buf));
         return IFX_ERROR;
      }
   }

   return 0;

#else       /* CONFIG_DEVFS_FS */
   /* ============================================
      register device
      ============================================ */

   int result;

   result = register_chrdev( major_number , devName, &MEI_TEST_fops);
   if ( result < 0 )
   {
      PRN_DBG_USR_NL( MEI_TEST, MEI_DRV_PRN_LEVEL_HIGH,
            ("MEI_DRV: device %s - can't get major %d" MEI_DRV_CRLF,
              devName, major_number));
      return result;
   }
   /* dynamic major                       */
   if ( major_number == 0 )
   {
      major_number = result;
      PRN_DBG_USR_NL( MEI_TEST, MEI_DRV_PRN_LEVEL_HIGH,
            ("Using major number %d" MEI_DRV_CRLF, major_number));
   }

   return 0;
#endif      /* CONFIG_DEVFS_FS */
}


/**
   Open a VRX test device

\param
   inode pointer to the inode
\param
   filp        pointer to the file descriptor

*/
static int MEI_TEST_Open(struct inode *inode, struct file *filp)
{
   IFX_uint16_t      meiLine  = (IFX_uint16_t)MINOR(inode->i_rdev);
   MEI_DYN_CNTRL_T *pMeiDynCntrl = NULL;

   PRN_DBG_USR_NL( MEI_TEST, MEI_DRV_PRN_LEVEL_NORMAL,
             ("MEI_TEST[%02d]: Open VRX test dev" MEI_DRV_CRLF,
              meiLine));


   if (    ( (meiLine == 0) || (meiLine > 0) )
        && (meiLine <  MEI_DFE_CHAN_DEVICES) )
   {
      if ( MEI_InternalDevOpen(meiLine, &pMeiDynCntrl) != IFX_SUCCESS )
      {
         PRN_ERR_USR_NL( MEI_TEST, MEI_DRV_PRN_LEVEL_NORMAL,
                   ("MEI_TEST[%02d]: Open - internal VRX open failed" MEI_DRV_CRLF,
                    meiLine));
         return IFX_ERROR;
      }

      filp->private_data = (void*)pMeiDynCntrl;
      filp->f_op = &MEI_TEST_fops;

      #if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
      /* increment module use counter */
      MOD_INC_USE_COUNT;
      #endif
   }
   else
   {
      PRN_ERR_USR_NL( MEI_TEST, MEI_DRV_PRN_LEVEL_NORMAL,
                ("MEI_TEST: Open - invalid line num %02d" MEI_DRV_CRLF,
                 meiLine));

      return IFX_ERROR;
   }

   return IFX_SUCCESS;
}

/**
   Release the VRX test dev.

\param
   inode pointer to the inode
\param
   filp pointer to the file descriptor

\return
   0 - on success
*/
static int MEI_TEST_Release(struct inode *inode, struct file *filp)
{
   MEI_DYN_CNTRL_T *pMeiDynCntrl = (MEI_DYN_CNTRL_T *)filp->private_data;
   MEI_DRV_PRN_LOCAL_DBG_VAR_CREATE(IFX_uint16_t, meiLine, (IFX_uint16_t)MINOR(inode->i_rdev));

   if (pMeiDynCntrl)
   {
      PRN_DBG_USR_NL( MEI_TEST, MEI_DRV_PRN_LEVEL_NORMAL,
             ("MEI_DRV[%02d]: Close - internal VRX instance" MEI_DRV_CRLF,
              meiLine));

      MEI_InternalDevClose(pMeiDynCntrl);
   }
   else
   {
      PRN_DBG_USR_NL( MEI_TEST, MEI_DRV_PRN_LEVEL_NORMAL,
             ("MEI_DRV[%02d]: close internal VRX instance failed" MEI_DRV_CRLF,
              meiLine));
   }

   filp->private_data = NULL;

   #if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
   /* decrement use counter */
   MOD_DEC_USE_COUNT;
   #endif

   return 0;
}

/**
   Writes data to the device.

   \param filp pointer to the file descriptor
   \param buf source buffer
   \param count data length
   \param ppos unused

   \return
   length or a negative error code
*/
static ssize_t MEI_TEST_Write(struct file *filp, const char *buf,
                              size_t count, loff_t * ppos)
{
   PRN_ERR_USR_NL( MEI_TEST, MEI_DRV_PRN_LEVEL_WRN,
         ("MEI_TEST: write(...) not supported" MEI_DRV_CRLF));

   return IFX_ERROR;
}

/**
   Reads data from the device.

   \param filp pointer to the file descriptor
   \param buf destination buffer
   \param count max size of data to read
   \param ppos unused

   \return
   len - data length
*/
static ssize_t MEI_TEST_Read(struct file *filp, char *buf, size_t count, loff_t * ppos)
{
   PRN_ERR_USR_NL( MEI_TEST, MEI_DRV_PRN_LEVEL_WRN,
         ("MEI_TEST: read(...) not supported" MEI_DRV_CRLF));

   return IFX_ERROR;
}


/**
   Configuration / Control for the device.

\param
   inode pointer to the inode
\param
   filp pointer to the file descriptor
\param
   nCmd function id's
\param
   nArgument optional argument

\return
   0 and positive values - success,
   negative value - ioctl failed
*/
static int MEI_TEST_Ioctl( struct inode *inode, struct file *filp,
                                   unsigned int nCmd, unsigned long nArgument)
{
   IFX_int32_t                retVal = IFX_SUCCESS;
   IFX_int32_t                retSize = sizeof(IOCTL_MEI_ioctl_t);
   MEI_DYN_CNTRL_T          *pMeiDynCntrl = (MEI_DYN_CNTRL_T *)filp->private_data;
   IOCTL_MEI_TEST_internArg_t *pUserArgs      = IFX_NULL;
   IOCTL_MEI_TEST_internArg_t localArgs;

   unsigned char  *pUsrMsgWrBuf;
   unsigned char  *pUsrMsgAckBuf;
   IFX_uint8_t msgWrBuf[MEI_TEST_MAX_MSG_PAYLOAD_SIZE];
   IFX_uint8_t msgAckBuf[MEI_TEST_MAX_MSG_PAYLOAD_SIZE];

   if (nArgument == 0)
   {
      PRN_ERR_USR_NL( MEI_TEST, MEI_DRV_PRN_LEVEL_ERR,
             ("MEI_TEST[tt] Error ioctl - invalid argument ptr" MEI_DRV_CRLF));
      retVal = IFX_ERROR;
      goto MEI_TEST_IOCTL_RETURN;
   }

   pUserArgs = (IOCTL_MEI_TEST_internArg_t *)nArgument;

   switch(nCmd)
   {
      case FIO_MEI_TEST_DBG_LEVEL:
         PRN_DBG_USR_NL( MEI_TEST, MEI_DRV_PRN_LEVEL_LOW,
             ("MEI_TEST[tt]: ioctl - <FIO_MEI_TEST_DBG_LEVEL>" MEI_DRV_CRLF));

         copy_from_user( &localArgs, pUserArgs, sizeof(IOCTL_MEI_TEST_dbgLevel_t) );

         if ( (localArgs.tst_dbgLevel.valLevel >= MEI_DRV_PRN_LEVEL_LOW) &&
              (localArgs.tst_dbgLevel.valLevel <= MEI_DRV_PRN_LEVEL_OFF) )
         {
            MEI_DRV_PRN_USR_LEVEL_SET(MEI_TEST,localArgs.tst_dbgLevel.valLevel);
            MEI_DRV_PRN_INT_LEVEL_SET(MEI_TEST,localArgs.tst_dbgLevel.valLevel);
         }
         else
         {
            retVal = IFX_ERROR;
         }
         break;
      case FIO_MEI_TEST_VERS_GET:
         {
            IFX_int32_t len = strlen(MEI_TEST_DRV_VER_STR);

            PRN_DBG_USR_NL( MEI_TEST, MEI_DRV_PRN_LEVEL_LOW,
                ("MEI_TEST[tt]: ioctl - <FIO_MEI_TEST_VERS_GET>" MEI_DRV_CRLF));

            copy_from_user( &localArgs, pUserArgs, sizeof(IOCTL_MEI_TEST_drvVers_t) );
            localArgs.tst_version.versionId =   (MEI_TEST_DRV_VER_MAJOR << 24)
                                              | (MEI_TEST_DRV_VER_MINOR << 16)
                                              | (MEI_TEST_DRV_VER_STEP << 8)
                                              | (MEI_TEST_DRV_VER_TYPE) ;

            /* set driver version string */
            if (localArgs.tst_version.pVersionStr && localArgs.tst_version.strSize)
            {
               len = (len > (IFX_int32_t)localArgs.tst_version.strSize) ?
                                 (IFX_int32_t)localArgs.tst_version.strSize : len;

               if ( (MEI_DRVOS_CpyToUser( localArgs.tst_version.pVersionStr, MEI_TEST_DRV_VER_STR, len))
                    == IFX_NULL )
               {
                  PRN_ERR_USR_NL( MEI_TEST, MEI_DRV_PRN_LEVEL_ERR,
                        ("MEI_TEST: DrvVersionGet - CopyToUser(..,..,%d) failed!" MEI_DRV_CRLF,
                         len));
                  localArgs.tst_version.strSize = 0;
                  localArgs.tst_version.ictl.retCode = IFX_ERROR;
                  return IFX_ERROR;
               }
               else
               {
                  localArgs.tst_version.strSize = len;
               }
            }
            retSize = sizeof(IOCTL_MEI_TEST_drvVers_t);
         }
         break;

      case FIO_MEI_INTERN_DBG_LEVEL:
         PRN_DBG_USR_NL( MEI_TEST, MEI_DRV_PRN_LEVEL_LOW,
             ("MEI_TEST[tt]: ioctl - <FIO_MEI_INTERN_DBG_LEVEL>" MEI_DRV_CRLF));

         copy_from_user( &localArgs, pUserArgs, sizeof(IOCTL_MEI_dbgLevel_t) );
         retVal = MEI_InternalDebugLevelSet(pMeiDynCntrl, &localArgs.intern_dbgLevel);
         break;

      case FIO_MEI_INTERN_VERS_GET:
         {
            IFX_char_t  versBuf[127];
            IOCTL_MEI_drvVersion_t tempVers;
            PRN_DBG_USR_NL( MEI_TEST, MEI_DRV_PRN_LEVEL_LOW,
                ("MEI_TEST[tt]: ioctl - <FIO_MEI_INTERN_VERS_GET>" MEI_DRV_CRLF));

            copy_from_user( &tempVers, pUserArgs, sizeof(IOCTL_MEI_drvVersion_t) );

            localArgs.intern_drvVers.strSize = (tempVers.strSize > 127) ? 127 : tempVers.strSize;
            localArgs.intern_drvVers.pVersionStr = versBuf;

            retVal = MEI_InternalDrvVersionGet(pMeiDynCntrl, &localArgs.intern_drvVers);
            if (retVal >= 0)
            {
               copy_to_user( tempVers.pVersionStr, versBuf, localArgs.intern_drvVers.strSize);

            }
            retSize = sizeof(IOCTL_MEI_drvVersion_t);
         }
         break;

      case FIO_MEI_INTERN_INIT:
         copy_from_user( &localArgs, pUserArgs, sizeof(IOCTL_MEI_devInit_t) );
         retVal = MEI_InternalInitDevice(pMeiDynCntrl, &localArgs.intern_devInit);
         retSize = sizeof(IOCTL_MEI_devInit_t);
         break;

      case FIO_MEI_INTERN_RESET:
         copy_from_user( &localArgs, pUserArgs, sizeof(IOCTL_MEI_reset_t) );
         retVal = MEI_InternalDevReset(pMeiDynCntrl, &localArgs.intern_devRst, 1);
         break;

#if (MEI_SUPPORT_DL_DMA_CS == 1)
      case FIO_MEI_INTERN_FW_DL:
         copy_from_user( &localArgs, pUserArgs, sizeof(IOCTL_MEI_fwDownLoad_t) );
         retVal = MEI_InternalFirmwareDownload(pMeiDynCntrl, &localArgs.intern_fwDl);
         break;
#endif

#if (MEI_SUPPORT_VDSL2_ADSL_SWAP == 1)
      case FIO_MEI_INTERN_FW_MODE_SELECT:
         copy_from_user( &localArgs, pUserArgs, sizeof(IOCTL_MEI_fwMode_t) );
         retVal = MEI_InternalDevCfgFwModeSwap(pMeiDynCntrl, &localArgs.intern_fwMode);
         break;
#endif

      case FIO_MEI_INTERN_REQ_CONFIG:
         retVal = MEI_InternalRequestConfig(pMeiDynCntrl, &localArgs.intern_reqCfg);
         retSize = sizeof(IOCTL_MEI_reqCfg_t);
         break;

#if (MEI_SUPPORT_STATISTICS == 1)
      case FIO_MEI_INTERN_REQ_STAT:
         retVal = MEI_InternalRequestStat(pMeiDynCntrl, &localArgs.intern_reqStat);
         retSize = sizeof(IOCTL_MEI_statistic_t);
         break;
#endif

#if (MEI_SUPPORT_REGISTER == 1)
      case FIO_MEI_INTERN_REG_SET:
         copy_from_user( &localArgs, pUserArgs, sizeof(IOCTL_MEI_regInOut_t) );
         retVal = MEI_InternalSetRegister(pMeiDynCntrl, &localArgs.intern_regIo);
         break;

      case FIO_MEI_INTERN_REG_GET:
         copy_from_user( &localArgs, pUserArgs, sizeof(IOCTL_MEI_regInOut_t) );
         retVal = MEI_InternalGetRegister(pMeiDynCntrl, &localArgs.intern_regIo);
         retSize = sizeof(IOCTL_MEI_regInOut_t);
         break;
#endif

#if (MEI_SUPPORT_DFE_GPA_ACCESS == 1)
      case FIO_MEI_INTERN_GPA_WRITE:
         copy_from_user( &localArgs, pUserArgs, sizeof(IOCTL_MEI_GPA_accessInOut_t) );
         retVal = MEI_InternalGpaWrAccess(pMeiDynCntrl, &localArgs.intern_GpaAccess);
         break;

      case FIO_MEI_INTERN_GPA_READ:
         copy_from_user( &localArgs, pUserArgs, sizeof(IOCTL_MEI_GPA_accessInOut_t) );
         retVal = MEI_InternalGpaRdAccess(pMeiDynCntrl, &localArgs.intern_GpaAccess);
         retSize = sizeof(IOCTL_MEI_GPA_accessInOut_t);
         break;
#endif

#if (MEI_SUPPORT_DFE_DBG_ACCESS == 1)
      case FIO_MEI_INTERN_DBG_WRITE:
         copy_from_user( &localArgs, pUserArgs, sizeof(IOCTL_MEI_dbgAccess_t) );
         retVal = MEI_InternalMeiDbgAccessWr(pMeiDynCntrl, &localArgs.intern_DbgAccess);
         break;

      case FIO_MEI_INTERN_DBG_READ:
         copy_from_user( &localArgs, pUserArgs, sizeof(IOCTL_MEI_dbgAccess_t) );
         retVal = MEI_InternalMeiDbgAccessRd(pMeiDynCntrl, &localArgs.intern_DbgAccess);
         retSize = sizeof(IOCTL_MEI_dbgAccess_t);
         break;
#endif

      case FIO_MEI_INTERN_MBOX_NFC_ENABLE:
         retVal = MEI_InternalNfcEnable(pMeiDynCntrl);
         break;

      case FIO_MEI_INTERN_MBOX_NFC_DISABLE:
         retVal = MEI_InternalNfcDisable(pMeiDynCntrl);
         break;

      case FIO_MEI_INTERN_AUTO_MSG_CTRL_SET:
         copy_from_user( &localArgs, pUserArgs, sizeof(IOCTL_MEI_autoMsgCtrl_t) );
         retVal = MEI_InternalAutoMsgCtlSet(pMeiDynCntrl, &localArgs.intern_autoMsgCtrl);
         break;

      case FIO_MEI_INTERN_AUTO_MSG_CTRL_GET:
         retVal = MEI_InternalAutoMsgCtlGet(pMeiDynCntrl, &localArgs.intern_autoMsgCtrl);
         retSize = sizeof(IOCTL_MEI_autoMsgCtrl_t);
         break;

      case FIO_MEI_INTERN_MBOX_MSG_WR:
         {
            copy_from_user( &localArgs, pUserArgs, sizeof(IOCTL_MEI_message_t) );

            if (localArgs.intern_ifxMsg.paylSize_byte > MEI_TEST_MAX_MSG_PAYLOAD_SIZE)
            {
               localArgs.intern_ifxMsg.ictl.retCode = -e_MEI_ERR_INVAL_ARG;
               retVal = IFX_ERROR;
            }
            else
            {
               pUsrMsgWrBuf = localArgs.intern_ifxMsg.pPayload;
               memset(msgWrBuf, 0x00, sizeof(msgWrBuf));
               copy_from_user( msgWrBuf,
                               pUsrMsgWrBuf,
                               localArgs.intern_ifxMsg.paylSize_byte );
               localArgs.intern_ifxMsg.pPayload = msgWrBuf;
               retVal = MEI_InternalCmdMsgWrite(pMeiDynCntrl, &localArgs.intern_ifxMsg);
               localArgs.intern_ifxMsg.pPayload = pUsrMsgWrBuf;
               retSize = sizeof(IOCTL_MEI_message_t);
            }
         }
         break;

      case FIO_MEI_INTERN_MBOX_ACK_RD:
         {
            copy_from_user( &localArgs, pUserArgs, sizeof(IOCTL_MEI_message_t) );

            if (localArgs.intern_ifxMsg.paylSize_byte > MEI_TEST_MAX_MSG_PAYLOAD_SIZE)
            {
               localArgs.intern_ifxMsg.ictl.retCode = -e_MEI_ERR_INVAL_ARG;
               retVal = IFX_ERROR;
            }
            else
            {
               memset(msgAckBuf, 0x00, sizeof(msgAckBuf));
               pUsrMsgAckBuf = localArgs.intern_ifxMsg.pPayload;
               localArgs.intern_ifxMsg.pPayload = msgAckBuf;
               retVal = MEI_InternalAckMsgRead(pMeiDynCntrl, &localArgs.intern_ifxMsg);
               if (retVal > 0)
               {
                  copy_to_user( pUsrMsgAckBuf,
                                localArgs.intern_ifxMsg.pPayload,
                                localArgs.intern_ifxMsg.paylSize_byte );

               }
               localArgs.intern_ifxMsg.pPayload = pUsrMsgAckBuf;
               retSize = sizeof(IOCTL_MEI_message_t);
            }
         }
         break;

      case FIO_MEI_INTERN_MBOX_MSG_SEND:
         copy_from_user( &localArgs, pUserArgs, sizeof(IOCTL_MEI_messageSend_t) );

         PRN_DBG_USR_NL( MEI_TEST, MEI_DRV_PRN_LEVEL_LOW,
             ("MEI_TEST[tt]: ioctl <FIO_MEI_INTERN_MBOX_MSG_SEND> - msgID = 0x%04X" MEI_DRV_CRLF,
               localArgs.intern_ifxMsgSend.write_msg.msgId));

         if (localArgs.intern_ifxMsgSend.write_msg.paylSize_byte > MEI_TEST_MAX_MSG_PAYLOAD_SIZE)
         {
            localArgs.intern_ifxMsgSend.write_msg.ictl.retCode = -e_MEI_ERR_INVAL_ARG;
            retVal = IFX_ERROR;
         }
         else
         {
            memset(msgAckBuf, 0x00, sizeof(msgAckBuf));
            memset(msgWrBuf, 0x00, sizeof(msgWrBuf));

            pUsrMsgAckBuf = localArgs.intern_ifxMsgSend.ack_msg.pPayload;
            pUsrMsgWrBuf = localArgs.intern_ifxMsgSend.write_msg.pPayload;

            /* get msg cmd payload */
            copy_from_user( msgWrBuf,
                            pUsrMsgWrBuf,
                            localArgs.intern_ifxMsgSend.write_msg.paylSize_byte );

            localArgs.intern_ifxMsgSend.write_msg.pPayload = msgWrBuf;
            localArgs.intern_ifxMsgSend.ack_msg.pPayload = msgAckBuf;

            retVal = MEI_InternalMsgSend(pMeiDynCntrl, &localArgs.intern_ifxMsgSend);

            /* return msg ack payload */
            if (retVal == 0)
            {
               if (localArgs.intern_ifxMsgSend.ack_msg.paylSize_byte)
               {
                  copy_to_user(
                        pUsrMsgAckBuf,
                        localArgs.intern_ifxMsgSend.ack_msg.pPayload,
                        localArgs.intern_ifxMsgSend.ack_msg.paylSize_byte );
               }
               else
               {
                  PRN_DBG_USR_NL( MEI_TEST, MEI_DRV_PRN_LEVEL_NORMAL,
                      ("MEI_TEST[tt]: ioctl <FIO_MEI_INTERN_MBOX_MSG_SEND> - msgID = 0x%04X - no data" MEI_DRV_CRLF,
                        localArgs.intern_ifxMsgSend.ack_msg.msgId));
               }
            }
            else
            {
               PRN_ERR_USR_NL( MEI_TEST, MEI_DRV_PRN_LEVEL_ERR,
                   ("MEI_TEST[tt] Error ioctl <FIO_MEI_INTERN_MBOX_MSG_SEND>, retVal = %d" MEI_DRV_CRLF,
                     retVal));
            }
            localArgs.intern_ifxMsgSend.ack_msg.pPayload = pUsrMsgAckBuf;
            retSize = sizeof(IOCTL_MEI_messageSend_t);
         }
         break;

      case FIO_MEI_INTERN_MBOX_NFC_RD:
         retVal = MEI_InternalNfcMsgRead(pMeiDynCntrl, &localArgs.intern_ifxMsg);
         retSize = sizeof(IOCTL_MEI_messageSend_t);

         break;


      default:
         PRN_ERR_USR_NL( MEI_TEST, MEI_DRV_PRN_LEVEL_ERR,
                ("MEI_TEST[tt] Error ioctl - unknown CMD" MEI_DRV_CRLF));
         retVal = IFX_ERROR;
         break;
   }

MEI_TEST_IOCTL_RETURN:

   localArgs.tst_ioctl.retCode = retVal;
   copy_to_user( ((IOCTL_MEI_TEST_internArg_t *)nArgument), &localArgs, retSize);

   if (retVal < 0)
   {
      PRN_DBG_USR_NL( MEI_TEST, MEI_DRV_PRN_LEVEL_HIGH,
          ("MEI_TEST[tt]: ERROR - ioctl, retVal = %d" MEI_DRV_CRLF,
            retVal));
   }

   return (retVal < 0) ? -1 : 0;
}


/**
   LINUX: The select function of the driver.
   A user space program may sleep until the driver wakes it up.
*/
static unsigned int MEI_TEST_Poll (struct file *filp, poll_table *wait)
{

   PRN_ERR_USR_NL( MEI_TEST, MEI_DRV_PRN_LEVEL_WRN,
         ("MEI_TEST: poll(...) not supported" MEI_DRV_CRLF));

   return IFX_ERROR;
}


#if CONFIG_PROC_FS

/**
   Initialize and install the proc entry

\param
   devName

\return
   -1 or 0 on success
\remark
   Called by the kernel.
*/
static int MEI_TEST_InstallProcEntry(unsigned char devNum)
{
   struct proc_dir_entry *driver_proc_node;

   /* install the proc entry */
   PRN_DBG_USR_NL( MEI_TEST, MEI_DRV_PRN_LEVEL_LOW,
         ("MEI_DRV: using proc fs" MEI_DRV_CRLF));

   driver_proc_node = proc_mkdir( "driver/" MEI_TEST_DRV_NAME, NULL);

   if (driver_proc_node != NULL)
   {
      create_proc_read_entry( "version" , S_IFREG|S_IRUGO,
                             driver_proc_node, MEI_TEST_ReadProc, (void *)MEI_TEST_GetVersionProc );
   }
   else
   {
      PRN_DBG_USR_NL( MEI_TEST, MEI_DRV_PRN_LEVEL_HIGH,
            ("MEI_DRV: cannot create proc entry version" MEI_DRV_CRLF));
      return IFX_ERROR;
   }

   return 0;
}

/**
   Read the version information from the driver.

\param
   buf destination buffer.

\return
   length
*/
static int MEI_TEST_GetVersionProc(char *buf)
{
    int len;

    len = sprintf(buf, "%s" MEI_DRV_CRLF, &MEI_TEST_DRV_WHAT_STR[4]);

    len += sprintf(buf + len, "Compiled on %s, %s for Linux kernel %s (jiffies: %ld)" MEI_DRV_CRLF,
                   __DATE__, __TIME__, UTS_RELEASE, jiffies);

    return len;
}



/**
   The proc filesystem: function to read and entry.
*/
static int MEI_TEST_ReadProc(char *page, char **start, off_t off,
                              int count, int *eof, void *data)
{
   int len;

   int (*fn)(char *buf);

   if (data != NULL)
   {
       fn = (int (*)(char*))data;
       len = fn(page);
   }
   else
       return 0;

   if (len <= off+count)
       *eof = 1;
   *start = page + off;
   len -= off;
   if (len>count)
       len = count;
   if (len<0)
       len = 0;

   return len;
}

#endif   /* #if CONFIG_PROC_FS */



/* ==========================================================================
   Global function definitions
   ========================================================================== */

/*
   register module init and exit
*/
module_init (MEI_TEST_module_init);
module_exit (MEI_TEST_module_exit);

/****************************************************************************/

MODULE_AUTHOR("name <surname.lastname@lantiq.com>");
MODULE_DESCRIPTION("VRX Test Interface, Internal - www.lantiq.com");
MODULE_SUPPORTED_DEVICE("VRX Test Interface Internal");
MODULE_LICENSE("proprietary");


#endif /* LINUX */

