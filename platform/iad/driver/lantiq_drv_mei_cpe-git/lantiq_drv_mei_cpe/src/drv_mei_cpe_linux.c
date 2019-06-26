/******************************************************************************

                          Copyright (c) 2007-2015
                     Lantiq Beteiligungs-GmbH & Co. KG

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/* ==========================================================================
   Description : MEI CPE Driver, Linux part
   ========================================================================== */

#ifdef LINUX

/* ==========================================================================
   includes - LINUX
   ========================================================================== */
/* get at first the driver configuration */
#include "drv_mei_cpe_config.h"

#include "ifx_types.h"
#include "drv_mei_cpe_os.h"

#ifdef __KERNEL__
   #include <linux/kernel.h>
#endif

#include <linux/module.h>
#include <linux/version.h>

#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,17))
   #if (LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0))
      #if (MEI_SUPPORT_DEVICE_VR10_320 == 1)
         #include <generated/utsrelease.h>
         #include <linux/pmu.h>
         #include <linux/clk.h>
      #else
         #include <linux/utsrelease.h>
         #include <asm/ifx/ifx_pmu.h>
      #endif
   #else
      #include <generated/utsrelease.h>
      #include <linux/clk.h>
   #endif
#endif
#include <linux/init.h>

#include <linux/ioport.h>
#include <linux/irq.h>
#include <asm/io.h>

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
   #undef CONFIG_DEVFS_FS
#endif

#ifdef CONFIG_DEVFS_FS
#include <linux/devfs_fs_kernel.h>
#endif /* CONFIG_DEVFS_FS */

#if CONFIG_PROC_FS
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#endif

#if (MEI_SUPPORT_DEBUG_LOGGER == 1)
#include <linux/skbuff.h>
#include <linux/netlink.h>
#include <net/sock.h>
#endif

/* add MEI CPE debug/printout part */
#include "drv_mei_cpe_dbg.h"

#include "drv_mei_cpe_dbg_driver.h"

#include "drv_mei_cpe_linux.h"

/* project specific headers */
#include "drv_mei_cpe_interface.h"
#include "drv_mei_cpe_mei_interface.h"
#include "drv_mei_cpe_api.h"

#include "drv_mei_cpe_msg_process.h"

#if (MEI_SUPPORT_PROCFS_CONFIG == 1)
#include "drv_mei_cpe_linux_proc_config.h"
#endif /* MEI_SUPPORT_PROCFS_CONFIG */

#include "drv_mei_cpe_download.h"

#if (MEI_SUPPORT_DSM == 1)
#include "drv_mei_cpe_dsm.h"
#endif /* (MEI_SUPPORT_DSM == 1) */

#include "drv_mei_cpe_dbg_access.h"

#if (MEI_DRV_ATM_OAM_ENABLE == 1)
#include "drv_mei_cpe_atmoam_common.h"
#endif

#if (MEI_DRV_CLEAR_EOC_ENABLE == 1)
#include "drv_mei_cpe_clear_eoc_common.h"
#endif

#if (MEI_SUPPORT_VDSL2_ADSL_SWAP == 1)
#include "drv_mei_cpe_device_cntrl.h"
#endif

#if (MEI_SUPPORT_DEBUG_STREAMS == 1)
#include "drv_mei_cpe_dbg_streams.h"
#endif

/* ===================================
   extern function declarations
   =================================== */

/* =================================== */
/* Local function declarations (LINUX) */
/* =================================== */

static int MEI_module_init(void);
static void MEI_module_exit(void);

static int MEI_driver_init(int entity);
static void MEI_driver_exit(void);

static IFX_int32_t MEI_CpeDevOpen(
                              IFX_int8_t nLineNum,
                              struct file *filp);
static int MEI_OpenOS(
                     struct inode *inode,
                     struct file *filp);

static int MEI_ReleaseCpeDev(
                     struct inode *inode,
                     struct file *filp);

static ssize_t MEI_Write(
                     struct file *filp,
                     const char *buf,
                     size_t count,
                     loff_t * ppos);

static ssize_t MEI_Read(
                     struct file * filp,
                     char *buf,
                     size_t length,
                     loff_t * ppos);

static long MEI_Ioctl( struct file *filp,
                        unsigned int nCmd, unsigned long nArgument);

static unsigned int MEI_Poll (struct file *filp, poll_table *table);

static IFX_int32_t MEI_DRVOS_RegisterPciDevices(IFX_uint32_t vendor, IFX_uint32_t device, int platform);
static IFX_void_t MEI_DRVOS_RemoveOwnPciPlatformDevices(void);

#if (MEI_SUPPORT_IRQ == 1)

static int MEI_IfxRequestIrq( unsigned int usedIrq,
                                usedIsrHandler_t usedIsrHandler,
                                usedIsrHandler_t usedIsrThreadedHandler,
                                unsigned long usedFlags,
                                const char *pUsedDevName,
                                void *pUsedDevId);

static void MEI_IfxFreeIrq(unsigned int usedIrq, void *pUsedDevId);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
static irqreturn_t MEI_InterruptLinux(int irq, void *dev_id, struct pt_regs *regs);
#else
static irqreturn_t MEI_InterruptLinux(int irq, void *dev_id);
#endif

#endif

#if (MEI_SUPPORT_DFE_DBG_ACCESS == 1)
static int MEI_IoctlMeiDbgAccessWr_Wrap(
                              MEI_DYN_CNTRL_T       *pMeiDynCntrl,
                              IOCTL_MEI_dbgAccess_t *pUserArgument,
                              IOCTL_MEI_dbgAccess_t *pLocalArgument);

static int MEI_IoctlMeiDbgAccessRd_Wrap(
                              MEI_DYN_CNTRL_T       *pMeiDynCntrl,
                              IOCTL_MEI_dbgAccess_t *pUserArgument,
                              IOCTL_MEI_dbgAccess_t *pLocalArgument);
#endif

static int MEI_InitModuleRegCharDev(const char *devName, int entity);
static int MEI_InitModuleBasics(void);

#if (MEI_SUPPORT_DFE_DMA_ACCESS == 1)
static int MEI_IoctlDmaAccessWr_Wrap(
                              MEI_DYN_CNTRL_T        *pMeiDynCntrl,
                              IOCTL_MEI_DMA_access_t *pDmaArgument);

static int MEI_IoctlDmaAccessRd_Wrap(
                              MEI_DYN_CNTRL_T        *pMeiDynCntrl,
                              IOCTL_MEI_DMA_access_t *pDmaArgument);
#endif

/*
   Only for testing
*/
#if (MEI_MISC_TEST == 1)
static void MEI_MemVAllocTest();
#endif

#if (MEI_SUPPORT_DEBUG_LOGGER == 1)
static void MEI_NlSendMsg(IFX_char_t* pMsg);
#endif

/* =================================== */
/* Local variables (LINUX)             */
/* =================================== */
static IFX_uint8_t major_number = 0;
static struct class *mei_class;
#ifdef MODULE
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
MODULE_PARM(major_number, "b");
#else
module_param(major_number, byte, 0);
#endif
MODULE_PARM_DESC(major_number, "to override automatic major number");
#endif /* #ifdef MODULE*/

/** install parameter debug_level: LOW (1), NORMAL (2), HIGH (3), OFF (4) */
#if ((MEI_DEBUG_PRINT == 1) || (MEI_ERROR_PRINT == 1))
static IFX_uint8_t debug_level = MEI_DRV_PRN_LEVEL_OFF;
#else
static IFX_uint8_t debug_level = MEI_DRV_PRN_LEVEL_OFF;
#endif

#ifdef MODULE
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
MODULE_PARM(debug_level, "b");
#else
module_param(debug_level, byte, 0);
#endif
MODULE_PARM_DESC(debug_level, "set to get more (1) or fewer (4) debug outputs");
#endif /* #ifdef MODULE*/

static IFX_uint32_t fsm_set_pre_action = 0;

#ifdef MODULE
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
MODULE_PARM(fsm_set_pre_action, "i");
#else
module_param(fsm_set_pre_action, int, 0);
#endif
MODULE_PARM_DESC(fsm_set_pre_action, "set to activate FSM Set pre-action");
#endif /* #ifdef MODULE*/

/* the driver callbacks */
static struct file_operations MEI_fops =
    {
    owner:
        THIS_MODULE,
    read:
        MEI_Read,
    write:
        MEI_Write,
    poll:
        MEI_Poll,
    unlocked_ioctl:
        MEI_Ioctl,
    open:
        MEI_OpenOS,
    release:
        MEI_ReleaseCpeDev
    };


#ifdef CONFIG_DEVFS_FS
/** handles for Dev FS */
static devfs_handle_t MEI_base_dir_handle;
static devfs_handle_t MEI_cntrl_handle[MEI_MAX_DFEX_ENTITIES];
static devfs_handle_t MEI_dev_handle[MEI_MAX_DFE_CHAN_DEVICES];
#endif


/* temp buffer for handling MEI Debug access */
static IFX_uint32_t MEI_DbgDumpBuffer[MEI_IOCTL_MAX_DBG_COUNT_32BIT];

#if (MEI_SUPPORT_DEBUG_LOGGER == 1)
#define NL_DBG_MSG_GROUP       1

struct sock *nl_debug_sock = IFX_NULL;
EXPORT_SYMBOL(nl_debug_sock);
#endif


/* ============================================================================
   MEI CPE driver interrupt functions (LINUX) - wrapping
   ========================================================================= */

static int MEI_IntSetupLocked = 0;

/* function ptr to the used request_irq function */
static MEI_RequestIrq_WrapLinux_t MEI_RequestIrq_WrapLx = request_threaded_irq;

/* function ptr to the used free_irq function */
static MEI_FreeIrq_WrapLinux_t MEI_FreeIrq_WrapLx = free_irq;


/* function ptr to the used Interrupt Enable Routine */


/* function ptr to the used Interrupt Enable Routine */



/* ============================================================================
   Local function declaration (LINUX)
   ========================================================================= */

/**
   Open a MEI CPE device to control a device channel

\param
   deviceNum   number of the MEI CPE device
\param
   filp        pointer to the file descriptor

*/
static IFX_int32_t MEI_CpeDevOpen( IFX_int8_t nLineNum, struct file *filp)
{
   IFX_int32_t          retVal;
   MEI_DYN_CNTRL_T   *pDynCntrl = NULL;

   if ((retVal = MEI_DevLineAlloc(nLineNum)) != IFX_SUCCESS)
   {
      return retVal;
   }

   if ((retVal = MEI_InstanceLineAlloc(nLineNum, &pDynCntrl)) != IFX_SUCCESS)
   {
      return retVal;
   }

   /* NOTE: open success
      - The device number (channel) was set within the device structure
        for new memory allocations.
      - Now replace the device number within the private data with
        the VRX dynamic control structure to provide open specific data.
   */
   filp->private_data = (void*)pDynCntrl;

   if (! MEI_DEVICE_CFG_IS_PLATFORM(e_MEI_DEV_PLATFORM_CONFIG_VR9))
   {
      /* Init VR10/VR11 device (set mei base addr, pdbram base addr, irq) */
      /* Necessary addresses provided by pcie driver */
      if ((retVal = MEI_VR1x_InternalInitDevice(pDynCntrl)) != IFX_SUCCESS)
      {
         return retVal;
      }
   }

   #if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
   /* increment module use counter */
   MOD_INC_USE_COUNT;
   #endif

   return IFX_SUCCESS;
}

/**
   Open a device from the OS open call.

\param
   inode pointer to the inode
\param
   filp pointer to the file descriptor

\return
   0:    open SUCCESSFULL
   -1:   Error while open device.

*/
static int MEI_OpenOS(struct inode *inode, struct file *filp)
{
   int result, deviceNum;

   /*
      device number = minor number
      The minor number is set via instmod (without devfs)
      or while device registeration
   */

   deviceNum = MINOR(inode->i_rdev);
   filp->private_data = (void*)deviceNum;

   result = MEI_CpeDevOpen(deviceNum, filp);

   return result;
}

/**
   Release the device.

\param
   inode pointer to the inode
\param
   filp pointer to the file descriptor

\return
   0 - on success
*/
static int MEI_ReleaseCpeDev(struct inode *inode, struct file *filp)
{
   MEI_DYN_CNTRL_T   *pDynCntrl = (MEI_DYN_CNTRL_T *)filp->private_data;

   MEI_DevLineClose(pDynCntrl);

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
static ssize_t MEI_Write(struct file *filp, const char *buf,
                                 size_t count, loff_t * ppos)
{
   PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_WRN,
         ("MEI_DRV: write(..., %d, ..) not supported" MEI_DRV_CRLF,
            count));

   return -e_MEI_ERR_INVAL_CMD;
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
static ssize_t MEI_Read(struct file *filp, char *buf, size_t count, loff_t * ppos)
{
   /* While open-time: VRX dynamic struct is stored in filp->private_data */
   MEI_DYN_CNTRL_T    *pDynCntrl = (MEI_DYN_CNTRL_T *)filp->private_data;
   MEI_DEV_T          *pMeiDev;
   MEI_DYN_CMD_DATA_T *pDynCmd;

   /* get the MEI CPE device structure */
   if (pDynCntrl == NULL)
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_WRN,
             ("MEI_DRV: Error read - invalid dyn device struct" MEI_DRV_CRLF));
      return -e_MEI_ERR_INVAL_CMD;
   }

   pMeiDev = pDynCntrl->pMeiDev;
   pDynCmd   = pDynCntrl->pInstDynCmd;

   PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_WRN,
          ("MEI_DRV[0x%02X]: read(.., %d, ..) not supported" MEI_DRV_CRLF,
           MEI_DRV_LINENUM_GET(pMeiDev), count));

   return -e_MEI_ERR_INVAL_CMD;
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
static long MEI_Ioctl( struct file *filp,
                            unsigned int nCmd, unsigned long nArgument)
{
   int ret = 0, retSize = sizeof(IOCTL_MEI_ioctl_t);
   /* While open-time: MEI CPE dynamic struct is stored in filp->private_data */
   MEI_DYN_CNTRL_T *pMeiDynCntrl = (MEI_DYN_CNTRL_T *)filp->private_data;
   MEI_DEV_T    *pMeiDev;
   IOCTL_MEI_arg_t local_args, *pUserArgs;

   /* require an argument (ptr ioctl struct) */
   if (nArgument == 0)
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
             ("MEI_DRV[??] Error ioctl - invalid argument ptr" MEI_DRV_CRLF));
      return -e_MEI_ERR_INVAL_ARG;
   }

   pUserArgs = (IOCTL_MEI_arg_t *)nArgument;

   /* get the MEI CPE device structure */
   if (pMeiDynCntrl == NULL)
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
             ("MEI_DRV[??] Error ioctl - invalid dyn device struct" MEI_DRV_CRLF));

      ret = -e_MEI_ERR_DEV_NOT_EXIST;
      goto MEI_IOCTL_RETURN;
   }

   pMeiDev = pMeiDynCntrl->pMeiDev;

   /*
      Check for valid commands if driver still not init.
   */
   if ( (ret = MEI_CheckIoctlCmdInitState( pMeiDynCntrl, (IFX_uint32_t)nCmd ))
        != IFX_SUCCESS )
   {
      goto MEI_IOCTL_RETURN;
   }


   /* poll all devices in POLLING mode */
   MEI_DevPollAllIrq(e_MEI_DEV_ACCESS_MODE_PASSIV_POLL);

   /*
      check correct driver state for send modem message.
   */
   if ( (ret = MEI_CheckIoctlCmdSendState(pMeiDynCntrl, (IFX_uint32_t)nCmd))
        != IFX_SUCCESS )
   {
      goto MEI_IOCTL_RETURN;
   }

   switch (nCmd)
   {
      case FIO_MEI_DEBUGLEVEL:
         MEI_DRVOS_CpyFromUser(
            &local_args, pUserArgs, sizeof(IOCTL_MEI_dbgLevel_t) );

         ret = MEI_IoctlDebugLevelSet(pMeiDynCntrl, &local_args.dbg_level);
         break;

      case FIO_MEI_VERSION_GET:
         MEI_DRVOS_CpyFromUser(
            &local_args, pUserArgs, sizeof(IOCTL_MEI_drvVersion_t) );

         ret = MEI_IoctlDrvVersionGet(pMeiDynCntrl, &local_args.drv_vers, IFX_FALSE);
         retSize = sizeof(IOCTL_MEI_drvVersion_t);
         #if (MEI_MISC_TEST == 1)
         MEI_MemVAllocTest();
         #endif
         #if (MEI_DBG_DSM_PROFILING == 1)
         MEI_VR9_DSM_DbgTestProfiling(pMeiDev);
         #endif
         break;

      case FIO_MEI_DEV_INIT:
         MEI_DRVOS_CpyFromUser(
            &local_args, pUserArgs, sizeof(IOCTL_MEI_devInit_t) );

         ret = (int)MEI_IoctlInitDevice(pMeiDynCntrl, &local_args.init_dev);

         break;

      case FIO_MEI_DRV_DEVINFO_GET:
         MEI_DRVOS_CpyFromUser(
            &local_args, pUserArgs, sizeof(IOCTL_MEI_devinfo_t) );
         MEI_IoctlDevinfoGet(pMeiDynCntrl, &local_args.devinfo);
         retSize = sizeof(IOCTL_MEI_devinfo_t);
         break;

      case FIO_MEI_DRV_INIT:
         MEI_DRVOS_CpyFromUser(
            &local_args, pUserArgs, sizeof(IOCTL_MEI_drvInit_t) );

         MEI_IoctlDrvInit(pMeiDynCntrl, &local_args.init_drv);
         retSize = sizeof(IOCTL_MEI_drvInit_t);
         break;

      case FIO_MEI_RESET:
         MEI_DRVOS_CpyFromUser(
            &local_args, pUserArgs, sizeof(IOCTL_MEI_reset_t) );

#if (MEI_SUPPORT_DSM == 1)
         /* update fw counters (if fw was download before) */
         if (pMeiDev->meiFwDlCount)
         {
            MEI_VRX_DSM_FwStatsUpdate(pMeiDynCntrl, &pMeiDev->meiDsmStatistic);
         }
#endif /* (MEI_SUPPORT_DSM == 1) */

         ret = MEI_DrvAndDevReset(
               pMeiDev, local_args.rst.rstMode, local_args.rst.rstSelMask, 1);
         break;

      case FIO_MEI_REQ_CONFIG:
         MEI_IoctlRequestConfig(pMeiDynCntrl, &local_args.req_cfg);
         retSize = sizeof(IOCTL_MEI_reqCfg_t);
         break;

#if (MEI_SUPPORT_VDSL2_ADSL_SWAP == 1)
      case FIO_MEI_FW_MODE_SELECT:
         MEI_DRVOS_CpyFromUser(
            &local_args, pUserArgs, sizeof(IOCTL_MEI_fwMode_t) );

         ret = MEI_IoctlDevCfgFwModeSwap(
               pMeiDynCntrl, &local_args.fw_mode);
         break;
#endif      /* #if (MEI_SUPPORT_VDSL2_ADSL_SWAP == 1) */

      case FIO_MEI_FW_MODE_CTRL_SET:
         MEI_DRVOS_CpyFromUser(
            &local_args, pUserArgs, sizeof(IOCTL_MEI_FwModeCtrlSet_t) );

         ret = MEI_IoctlFwModeCtrlSet(
               pMeiDynCntrl, &local_args.fw_mode_ctrl);
         break;

      case FIO_MEI_FW_MODE_STAT_GET:
         ret = MEI_IoctlFwModeStatGet(
               pMeiDynCntrl, &local_args.fw_mode_stat);

         MEI_DRVOS_CpyToUser(
            pUserArgs, &local_args, sizeof(IOCTL_MEI_FwModeStatGet_t) );
         break;

#if (MEI_SUPPORT_STATISTICS == 1)
      case FIO_MEI_REQ_STAT:
         MEI_IoctlRequestStat(pMeiDynCntrl, &local_args.req_stat);
         retSize = sizeof(IOCTL_MEI_statistic_t);
         break;
#endif

#if (MEI_SUPPORT_REGISTER == 1)
      case FIO_MEI_REG_SET:
         MEI_DRVOS_CpyFromUser(
            &local_args.reg_io, &pUserArgs->reg_io, sizeof(IOCTL_MEI_regInOut_t) );

         if( MEI_Set_Register(
               pMeiDev, local_args.reg_io.addr, local_args.reg_io.value) != IFX_SUCCESS )
         {
            ret = -e_MEI_ERR_OP_FAILED;
         }
         break;

      case FIO_MEI_REG_GET:
         MEI_DRVOS_CpyFromUser(
            &local_args.reg_io.addr, &pUserArgs->reg_io.addr, sizeof(local_args.reg_io.addr) );

         if( MEI_Get_Register(
               pMeiDev, local_args.reg_io.addr, (IFX_uint32_t*)&local_args.reg_io.value)
             != IFX_SUCCESS )
         {
            ret = -e_MEI_ERR_OP_FAILED;
         }
         else
         {
            retSize = sizeof(IOCTL_MEI_regInOut_t);
         }
         break;
#endif      /* #if (MEI_SUPPORT_REGISTER == 1) */

#if (MEI_SUPPORT_DRV_LOOPS == 1)
      case FIO_MEI_DRV_LOOP:
         MEI_DRVOS_CpyFromUser(
            &local_args, pUserArgs, sizeof(IOCTL_MEI_drvLoop_t) );

         MEI_MailboxLoop( pMeiDev,
                            (local_args.drv_loop.loopEnDis) ? IFX_TRUE:IFX_FALSE);
         break;
#endif

      case FIO_MEI_FW_DL:
         MEI_DRVOS_CpyFromUser(
            &local_args, pUserArgs, sizeof(IOCTL_MEI_fwDownLoad_t) );

         PRN_DBG_USR( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL, MEI_DRV_DYN_LINENUM_GET(pMeiDynCntrl),
                ("MEI_DRV: ioctl - FIO_MEI_FW_DL: size = %d bytes" MEI_DRV_CRLF,
                 (unsigned int)local_args.fw_dl.size_byte));

         ret = MEI_IoctlFirmwareDownload(pMeiDynCntrl, &local_args.fw_dl, IFX_FALSE);
         break;

      case FIO_MEI_OPT_FW_DL:
         MEI_DRVOS_CpyFromUser(
            &local_args, pUserArgs, sizeof(IOCTL_MEI_fwOptDownLoad_t) );

         PRN_DBG_USR( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL, MEI_DRV_DYN_LINENUM_GET(pMeiDynCntrl),
                ("MEI_DRV: ioctl - FIO_MEI_OPT_FW_DL: size = %d bytes" MEI_DRV_CRLF,
                 (unsigned int)local_args.fw_dl_opt.size_byte));

         ret = MEI_IoctlOptFirmwareDownload(pMeiDynCntrl, &local_args.fw_dl_opt, IFX_FALSE);
         break;

#if (MEI_SUPPORT_DFE_DMA_ACCESS == 1)
      case FIO_MEI_DMA_WRITE:
         MEI_DRVOS_CpyFromUser(
            &local_args.dma_access, &pUserArgs->dma_access, sizeof(IOCTL_MEI_DMA_access_t) );

         ret = MEI_IoctlDmaAccessWr_Wrap( pMeiDynCntrl,
                                            &local_args.dma_access);
         retSize = sizeof(IOCTL_MEI_DMA_access_t);
         break;
      case FIO_MEI_DMA_READ:
         MEI_DRVOS_CpyFromUser(
            &local_args.dma_access, &pUserArgs->dma_access, sizeof(IOCTL_MEI_DMA_access_t) );

         ret = MEI_IoctlDmaAccessRd_Wrap( pMeiDynCntrl,
                                            &local_args.dma_access);
         retSize = sizeof(IOCTL_MEI_DMA_access_t);
         break;
#endif

#if (MEI_SUPPORT_DFE_GPA_ACCESS == 1)
      case FIO_MEI_GPA_WRITE:
         MEI_DRVOS_CpyFromUser(
            &local_args.gpa_access, &pUserArgs->gpa_access, sizeof(IOCTL_MEI_GPA_accessInOut_t) );

         ret = MEI_GpaWrAccess(
                  pMeiDynCntrl, local_args.gpa_access.dest,
                  local_args.gpa_access.addr, local_args.gpa_access.value);
         break;

      case FIO_MEI_GPA_READ:
         MEI_DRVOS_CpyFromUser(
            &local_args.gpa_access, &pUserArgs->gpa_access, sizeof(IOCTL_MEI_GPA_accessInOut_t) );

         ret = MEI_GpaRdAccess(
                  pMeiDynCntrl, local_args.gpa_access.dest,
                  local_args.gpa_access.addr, (IFX_uint32_t *)&local_args.gpa_access.value);
         if (ret != 0)
         {
            PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
                   ("MEI_DRV[%02d]: ERROR ioctl - FIO_MEI_GPA_READ" MEI_DRV_CRLF,
                     MEI_DRV_LINENUM_GET(pMeiDev)));

            ret = -e_MEI_ERR_OP_FAILED;
         }
         else
         {
            retSize = sizeof(IOCTL_MEI_GPA_accessInOut_t);
         }
         break;
#endif      /* #if (MEI_SUPPORT_DFE_GPA_ACCESS == 1) */

#if (MEI_SUPPORT_DFE_DBG_ACCESS == 1)
      case FIO_MEI_DBG_WRITE:
         MEI_DRVOS_CpyFromUser(
            &local_args.dbg_access, &pUserArgs->dbg_access, sizeof(IOCTL_MEI_dbgAccess_t) );

         ret = MEI_IoctlMeiDbgAccessWr_Wrap(
            pMeiDynCntrl, &pUserArgs->dbg_access, &local_args.dbg_access);
         break;

      case FIO_MEI_DBG_READ:
         MEI_DRVOS_CpyFromUser(
            &local_args.dbg_access, &pUserArgs->dbg_access, sizeof(IOCTL_MEI_dbgAccess_t) );

         ret = MEI_IoctlMeiDbgAccessRd_Wrap(
                  pMeiDynCntrl, &pUserArgs->dbg_access, &local_args.dbg_access);
         break;

      case FIO_MEI_DBG_AUX_WRITE:
         MEI_DRVOS_CpyFromUser(
            &local_args.dbg_access, &pUserArgs->dbg_access, sizeof(IOCTL_MEI_dbgAccess_t) );

         local_args.dbg_access.dbgDest = MEI_IOCTL_DEBUG_AUX;
         ret = MEI_IoctlMeiDbgAccessWr_Wrap(
                  pMeiDynCntrl, &pUserArgs->dbg_access, &local_args.dbg_access);
         break;

      case FIO_MEI_DBG_AUX_READ:
         MEI_DRVOS_CpyFromUser(
            &local_args.dbg_access, &pUserArgs->dbg_access, sizeof(IOCTL_MEI_dbgAccess_t) );

         local_args.dbg_access.dbgDest = MEI_IOCTL_DEBUG_AUX;
         ret = MEI_IoctlMeiDbgAccessRd_Wrap(
                  pMeiDynCntrl, &pUserArgs->dbg_access, &local_args.dbg_access);
         break;

      case FIO_MEI_DBG_CORE_WRITE:
         MEI_DRVOS_CpyFromUser(
            &local_args.dbg_access, &pUserArgs->dbg_access, sizeof(IOCTL_MEI_dbgAccess_t) );

         /* set CORE destination and do it */
         local_args.dbg_access.dbgDest = MEI_IOCTL_DEBUG_CORE;
         ret = MEI_IoctlMeiDbgAccessWr_Wrap(
                  pMeiDynCntrl, &pUserArgs->dbg_access, &local_args.dbg_access);
         break;

      case FIO_MEI_DBG_CORE_READ:
         MEI_DRVOS_CpyFromUser(
            &local_args.dbg_access, &pUserArgs->dbg_access, sizeof(IOCTL_MEI_dbgAccess_t) );

         /* set CORE destination and do it */
         local_args.dbg_access.dbgDest = MEI_IOCTL_DEBUG_CORE;
         ret = MEI_IoctlMeiDbgAccessRd_Wrap(
                  pMeiDynCntrl, &pUserArgs->dbg_access, &local_args.dbg_access);
         break;

      case FIO_MEI_DBG_LS_WRITE:
         MEI_DRVOS_CpyFromUser(
            &local_args.dbg_access, &pUserArgs->dbg_access, sizeof(IOCTL_MEI_dbgAccess_t) );

         local_args.dbg_access.dbgDest = MEI_IOCTL_DEBUG_LDST;
         ret = MEI_IoctlMeiDbgAccessWr_Wrap(
                  pMeiDynCntrl, &pUserArgs->dbg_access, &local_args.dbg_access);
         break;
      case FIO_MEI_DBG_LS_READ:
         MEI_DRVOS_CpyFromUser(
            &local_args.dbg_access, &pUserArgs->dbg_access, sizeof(IOCTL_MEI_dbgAccess_t) );

         local_args.dbg_access.dbgDest = MEI_IOCTL_DEBUG_LDST;
         ret = MEI_IoctlMeiDbgAccessRd_Wrap(
                  pMeiDynCntrl, &pUserArgs->dbg_access, &local_args.dbg_access);
         break;
#endif      /* #if (MEI_SUPPORT_DFE_DBG_ACCESS == 1) */

      case FIO_MEI_MBOX_NFC_ENABLE:
         /* Setup the receive NFC feature */
         if (pMeiDynCntrl->pInstDynNfc == IFX_NULL)
         {
            ret = MEI_IoctlNfcEnable(pMeiDynCntrl, 0, 0);
            pMeiDynCntrl->pInstDynNfc->msgProcessCtrl = MEI_MSG_CNTRL_MODEM_MSG_MASK_DEFAULT;
         }
         break;

      case FIO_MEI_MBOX_NFC_DISABLE:
         ret = MEI_IoctlNfcDisable(pMeiDynCntrl);

         break;

      case FIO_MEI_AUTO_MSG_CTRL_SET:
         PRN_DBG_USR( MEI_DRV, MEI_DRV_PRN_LEVEL_LOW, MEI_DRV_LINENUM_GET(pMeiDev),
                ("MEI_DRV[%02d]: ioctl - FIO_MEI_AUTO_MSG_CTRL_SET" MEI_DRV_CRLF,
                  MEI_DRV_LINENUM_GET(pMeiDev)));

         MEI_DRVOS_CpyFromUser(
            &local_args.autoMsgCtrl, &pUserArgs->autoMsgCtrl, sizeof(IOCTL_MEI_autoMsgCtrl_t) );

         ret = MEI_IoctlAutoMsgCtlSet(pMeiDynCntrl, &local_args.autoMsgCtrl);
         retSize = sizeof(IOCTL_MEI_autoMsgCtrl_t);
         break;

      case FIO_MEI_AUTO_MSG_CTRL_GET:
         ret = MEI_IoctlAutoMsgCtlGet(pMeiDynCntrl, &local_args.autoMsgCtrl);
         retSize = sizeof(IOCTL_MEI_autoMsgCtrl_t);
         break;

      case FIO_MEI_MBOX_MSG_SEND:
         PRN_DBG_USR( MEI_DRV, MEI_DRV_PRN_LEVEL_LOW, MEI_DRV_LINENUM_GET(pMeiDev),
                ("MEI_DRV[%02d]: ioctl - FIO_MEI_MBOX_MSG_SEND" MEI_DRV_CRLF,
                  MEI_DRV_LINENUM_GET(pMeiDev)));

         MEI_DRVOS_CpyFromUser(
            &local_args.ifx_msg_send, &pUserArgs->ifx_msg_send, sizeof(IOCTL_MEI_messageSend_t) );

         ret = MEI_IoctlMsgSend(pMeiDynCntrl, &local_args.ifx_msg_send, IFX_FALSE);
         retSize = sizeof(IOCTL_MEI_messageSend_t);
         break;

      case FIO_MEI_MBOX_MSG_WR:
            PRN_DBG_USR( MEI_DRV, MEI_DRV_PRN_LEVEL_LOW, MEI_DRV_LINENUM_GET(pMeiDev),
                   ("MEI_DRV[%02d]: ioctl - FIO_MEI_MBOX_MSG_WR" MEI_DRV_CRLF,
                     MEI_DRV_LINENUM_GET(pMeiDev)));

            MEI_DRVOS_CpyFromUser(
               &local_args.ifx_msg, &pUserArgs->ifx_msg, sizeof(IOCTL_MEI_message_t) );

            MEI_DRVOS_SemaphoreLock(&pMeiDynCntrl->pInstanceRWlock);
            ret = MEI_IoctlCmdMsgWrite(
                     pMeiDynCntrl, &local_args.ifx_msg, IFX_FALSE);
            MEI_DRVOS_SemaphoreUnlock(&pMeiDynCntrl->pInstanceRWlock);
            retSize = sizeof(IOCTL_MEI_message_t);
         break;

      case FIO_MEI_MBOX_ACK_RD:
         PRN_DBG_USR( MEI_DRV, MEI_DRV_PRN_LEVEL_LOW, MEI_DRV_LINENUM_GET(pMeiDev),
                ("MEI_DRV[%02d]: ioctl - FIO_MEI_MBOX_ACK_RD" MEI_DRV_CRLF,
                  MEI_DRV_LINENUM_GET(pMeiDev)));

         MEI_DRVOS_CpyFromUser(
            &local_args.ifx_msg, &pUserArgs->ifx_msg, sizeof(IOCTL_MEI_message_t) );

         MEI_DRVOS_SemaphoreLock(&pMeiDynCntrl->pInstanceRWlock);
         ret = MEI_IoctlAckMsgRead( pMeiDynCntrl,
                                   &local_args.ifx_msg,
                                   IFX_FALSE);

         MEI_DRVOS_SemaphoreUnlock(&pMeiDynCntrl->pInstanceRWlock);
         if (ret < 0)
         {
            local_args.ifx_msg.paylSize_byte = 0;
         }
         retSize = sizeof(IOCTL_MEI_message_t);
         break;

      case FIO_MEI_MBOX_NFC_RD:
         MEI_DRVOS_CpyFromUser(
            &local_args.ifx_msg, &pUserArgs->ifx_msg, sizeof(IOCTL_MEI_message_t) );

         ret = MEI_IoctlNfcMsgRead( pMeiDynCntrl,
                                       &local_args.ifx_msg, IFX_FALSE);
         if (ret < 0)
         {
            local_args.ifx_msg.paylSize_byte = 0;
         }
         retSize = sizeof(IOCTL_MEI_message_t);
         break;

#if (MEI_SUPPORT_RAW_MSG == 1)
      case FIO_MEI_MBOX_MSG_RAW_SEND:
         MEI_DRVOS_CpyFromUser(
            &local_args.mbox_send, &pUserArgs->mbox_send, sizeof(IOCTL_MEI_mboxSend_t) );

         ret = MEI_IoctlRawMsgSend(pMeiDynCntrl, &local_args.mbox_send);
         retSize = sizeof(IOCTL_MEI_mboxSend_t);
         break;

      case FIO_MEI_MBOX_MSG_RAW_WR:
         MEI_DRVOS_CpyFromUser(
            &local_args.mbox_msg, &pUserArgs->mbox_msg, sizeof(IOCTL_MEI_mboxMsg_t) );

         ret = MEI_IoctlRawMsgWrite(
                  pMeiDynCntrl, local_args.mbox_msg.pData_16, local_args.mbox_msg.count_16bit);

         local_args.mbox_msg.count_16bit = (ret < 0) ? 0 : ret >> 1;
         retSize = sizeof(IOCTL_MEI_mboxMsg_t);
         break;

      case FIO_MEI_MBOX_ACK_RAW_RD:
         MEI_DRVOS_CpyFromUser(
            &local_args.mbox_msg, &pUserArgs->mbox_msg, sizeof(IOCTL_MEI_mboxMsg_t) );

         ret = MEI_IoctlRawAckRead(
                  pMeiDynCntrl, local_args.mbox_msg.pData_16, local_args.mbox_msg.count_16bit);

         local_args.mbox_msg.count_16bit = (ret < 0) ? 0 : ret >> 1;
         retSize = sizeof(IOCTL_MEI_mboxMsg_t);
         break;

      case FIO_MEI_MBOX_NFC_RAW_RD:
         MEI_DRVOS_CpyFromUser(
            &local_args.mbox_msg, &pUserArgs->mbox_msg, sizeof(IOCTL_MEI_mboxMsg_t) );

         ret = MEI_IoctlRawNfcRead(
                  pMeiDynCntrl, local_args.mbox_msg.pData_16, local_args.mbox_msg.count_16bit);

         local_args.mbox_msg.count_16bit = (ret < 0) ? 0 : ret >> 1;
         retSize = sizeof(IOCTL_MEI_mboxMsg_t);
         break;
#endif      /* #if (MEI_SUPPORT_RAW_MSG == 1) */

#if (MEI_DRV_ATM_OAM_ENABLE == 1)
      case FIO_MEI_ATMOAM_INIT:
         PRN_DBG_USR( MEI_DRV, MEI_DRV_PRN_LEVEL_LOW, MEI_DRV_LINENUM_GET(pMeiDev),
                ("MEI_DRV[%02d]: ioctl - FIO_MEI_ATMOAM_INIT" MEI_DRV_CRLF,
                  MEI_DRV_LINENUM_GET(pMeiDev)));

         MEI_DRVOS_CpyFromUser(
            &local_args.atmoam_init, &pUserArgs->atmoam_init, sizeof(IOCTL_MEI_ATMOAM_init_t) );

         ret = MEI_ATMOAM_IoctlDrvInit(pMeiDynCntrl, &local_args.atmoam_init);
         retSize = sizeof(IOCTL_MEI_ATMOAM_init_t);
         break;

      case FIO_MEI_ATMOAM_CNTRL:
         PRN_DBG_USR( MEI_DRV, MEI_DRV_PRN_LEVEL_LOW, MEI_DRV_LINENUM_GET(pMeiDev),
                ("MEI_DRV[%02d]: ioctl - FIO_MEI_ATMOAM_CNTRL" MEI_DRV_CRLF,
                  MEI_DRV_LINENUM_GET(pMeiDev)));

         MEI_DRVOS_CpyFromUser(
            &local_args.atmoam_cntrl, &pUserArgs->atmoam_cntrl, sizeof(IOCTL_MEI_ATMOAM_cntrl_t) );

         ret = MEI_ATMOAM_IoctlCntrl(pMeiDynCntrl, &local_args.atmoam_cntrl);
         retSize = sizeof(IOCTL_MEI_ATMOAM_cntrl_t);
         break;

      case FIO_MEI_ATMOAM_REQ_DEV_COUNTER:
         PRN_DBG_USR( MEI_DRV, MEI_DRV_PRN_LEVEL_LOW, MEI_DRV_LINENUM_GET(pMeiDev),
                ("MEI_DRV[%02d]: ioctl - FIO_MEI_ATMOAM_REQ_DEV_COUNTER" MEI_DRV_CRLF,
                  MEI_DRV_LINENUM_GET(pMeiDev)));

         ret = MEI_ATMOAM_IoctlCounterGet(pMeiDynCntrl, &local_args.atmoam_counter);
         retSize = sizeof(IOCTL_MEI_ATMOAM_counter_t);
         break;

      case FIO_MEI_ATMOAM_REQ_DRV_STATUS:
         PRN_DBG_USR( MEI_DRV, MEI_DRV_PRN_LEVEL_LOW, MEI_DRV_LINENUM_GET(pMeiDev),
                ("MEI_DRV[%02d]: ioctl - FIO_MEI_ATMOAM_REQ_DRV_STATUS" MEI_DRV_CRLF,
                  MEI_DRV_LINENUM_GET(pMeiDev)));

         ret = MEI_ATMOAM_IoctlStatusGet(pMeiDynCntrl, &local_args.atmoam_status);
         retSize = sizeof(IOCTL_MEI_ATMOAM_status_t);
         break;

      case FIO_MEI_ATMOAM_CELL_INSERT:
         PRN_DBG_USR( MEI_DRV, MEI_DRV_PRN_LEVEL_LOW, MEI_DRV_LINENUM_GET(pMeiDev),
                ("MEI_DRV[%02d]: ioctl - FIO_MEI_ATMOAM_CELL_INSERT" MEI_DRV_CRLF,
                  MEI_DRV_LINENUM_GET(pMeiDev)));

         MEI_DRVOS_CpyFromUser(
            &local_args.atmoam_cells, &pUserArgs->atmoam_cells, sizeof(IOCTL_MEI_ATMOAM_drvAtmCells_t) );

         ret = MEI_ATMOAM_IoctlCellInsert(pMeiDynCntrl, &local_args.atmoam_cells);
         break;

#endif      /* #if (MEI_DRV_ATM_OAM_ENABLE == 1) */

#if (MEI_DRV_CLEAR_EOC_ENABLE == 1)
      case FIO_MEI_CEOC_INIT:
         PRN_DBG_USR( MEI_DRV, MEI_DRV_PRN_LEVEL_LOW, MEI_DRV_LINENUM_GET(pMeiDev),
                ("MEI_DRV[%02d]: ioctl - FIO_MEI_CEOC_INIT" MEI_DRV_CRLF,
                  MEI_DRV_LINENUM_GET(pMeiDev)));

         MEI_DRVOS_CpyFromUser(
            &local_args.ceoc_init, &pUserArgs->ceoc_init, sizeof(IOCTL_MEI_CEOC_init_t) );

         ret = MEI_CEOC_IoctlDrvInit(pMeiDynCntrl, &local_args.ceoc_init);
         retSize = sizeof(IOCTL_MEI_CEOC_init_t);
         break;

      case FIO_MEI_CEOC_CNTRL:
         PRN_DBG_USR( MEI_DRV, MEI_DRV_PRN_LEVEL_LOW, MEI_DRV_LINENUM_GET(pMeiDev),
                ("MEI_DRV[%02d]: ioctl - FIO_MEI_CEOC_CNTRL" MEI_DRV_CRLF,
                  MEI_DRV_LINENUM_GET(pMeiDev)));

         MEI_DRVOS_CpyFromUser(
            &local_args.ceoc_cntrl, &pUserArgs->ceoc_cntrl, sizeof(IOCTL_MEI_CEOC_cntrl_t) );

         ret = MEI_CEOC_IoctlCntrl(pMeiDynCntrl, &local_args.ceoc_cntrl);
         retSize = sizeof(IOCTL_MEI_CEOC_cntrl_t);
         break;

      case FIO_MEI_CEOC_STATS:
         PRN_DBG_USR( MEI_DRV, MEI_DRV_PRN_LEVEL_LOW, MEI_DRV_LINENUM_GET(pMeiDev),
                ("MEI_DRV[%02d]: ioctl - FIO_MEI_CEOC_STATS" MEI_DRV_CRLF,
                  MEI_DRV_LINENUM_GET(pMeiDev)));

         ret = MEI_CEOC_IoctlStatusGet(pMeiDynCntrl, &local_args.ceoc_statistic);
         retSize = sizeof(IOCTL_MEI_CEOC_statistic_t);
         break;

      case FIO_MEI_CEOC_FRAME_WR:
         PRN_DBG_USR( MEI_DRV, MEI_DRV_PRN_LEVEL_LOW, MEI_DRV_LINENUM_GET(pMeiDev),
                ("MEI_DRV[%02d]: ioctl - FIO_MEI_CEOC_FRAME_WR" MEI_DRV_CRLF,
                  MEI_DRV_LINENUM_GET(pMeiDev)));

         MEI_DRVOS_CpyFromUser(
            &local_args.ceoc_frame, &pUserArgs->ceoc_frame, sizeof(IOCTL_MEI_CEOC_frame_t) );

         ret = MEI_CEOC_IoctlFrameWrite(pMeiDynCntrl, &local_args.ceoc_frame, IFX_FALSE);
         retSize = sizeof(IOCTL_MEI_CEOC_frame_t);
         break;

      case FIO_MEI_CEOC_FRAME_RD:
         PRN_DBG_USR( MEI_DRV, MEI_DRV_PRN_LEVEL_LOW, MEI_DRV_LINENUM_GET(pMeiDev),
                ("MEI_DRV[%02d]: ioctl - FIO_MEI_CEOC_FRAME_RD" MEI_DRV_CRLF,
                  MEI_DRV_LINENUM_GET(pMeiDev)));

         MEI_DRVOS_CpyFromUser(
            &local_args.ceoc_frame, &pUserArgs->ceoc_frame, sizeof(IOCTL_MEI_CEOC_frame_t) );

         ret = MEI_CEOC_IoctlFrameRead(pMeiDynCntrl, &local_args.ceoc_frame, IFX_FALSE);
         retSize = sizeof(IOCTL_MEI_CEOC_frame_t);
         break;
#endif      /* #if (MEI_DRV_CLEAR_EOC_ENABLE == 1) */


#if (MEI_SUPPORT_TEST_DEBUG == 1)
      case FIO_MEI_MEI_REGS_SHOW:
         MEI_MeiRegsShow(pMeiDev);
         break;

      case FIO_MEI_DRV_BUF_SHOW:
         /* not further supported with driver version > 0.1.5.x */
         MEI_DRVOS_CpyFromUser(
            &local_args.show_drv_buf, (IOCTL_MEI_drvBufShow_t *)nArgument, sizeof(IOCTL_MEI_drvBufShow_t) );
         MEI_ShowDrvBuffer(
            pMeiDynCntrl, (IFX_int8_t)local_args.show_drv_buf.bufNum, local_args.show_drv_buf.count );
         ret = -e_MEI_ERR_INVAL_CMD;
         retSize = sizeof(IOCTL_MEI_drvBufShow_t);
         break;

      case FIO_MEI_DMA_TEST:
         PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_WRN,
                ("MEI_DRV: ioctl - FIO_MEI_DMA_TEST" MEI_DRV_CRLF));
         /* get arguments use
            dbgAddr: DMA dest address
            count:   DMA range
            dbgDest: test loop count*/
         MEI_DRVOS_CpyFromUser(
            &local_args.dbg_access, (IOCTL_MEI_dbgAccess_t *)nArgument, sizeof(IOCTL_MEI_dbgAccess_t) );

         /* execute access */
         ret = MEI_MeiDmaTest(
                  pMeiDev, local_args.dbg_access.dbgAddr,
                  local_args.dbg_access.count, local_args.dbg_access.dbgDest);
         break;

#endif

#if (MEI_SUPPORT_DSM == 1)
      case FIO_MEI_DSM_STATISTICS_GET:
         MEI_DRVOS_CpyFromUser(
            &local_args, pUserArgs, sizeof(IOCTL_MEI_dsmStatistics_t) );

         ret = (int)MEI_IoctlDsmStatisticGet(pMeiDynCntrl, &local_args.dsm_statistics);
         retSize = sizeof(IOCTL_MEI_dsmStatistics_t);
         #if (MEI_DBG_DSM_PROFILING == 1)
         MEI_VR9_DSM_DbgPrintProfiling(pMeiDev);
         #endif
         break;

      case FIO_MEI_DSM_CONFIG_GET:
         MEI_DRVOS_CpyFromUser(
            &local_args, pUserArgs, sizeof(IOCTL_MEI_dsmConfig_t) );

         ret = (int)MEI_IoctlDsmConfigGet(pMeiDynCntrl, &local_args.dsm_config);
         retSize = sizeof(IOCTL_MEI_dsmConfig_t);
         break;

      case FIO_MEI_DSM_CONFIG_SET:
         MEI_DRVOS_CpyFromUser(
            &local_args, pUserArgs, sizeof(IOCTL_MEI_dsmConfig_t) );

         ret = (int)MEI_IoctlDsmConfigSet(pMeiDynCntrl, &local_args.dsm_config);
         break;

      case FIO_MEI_DSM_STATUS_GET:
         MEI_DRVOS_CpyFromUser(
            &local_args, pUserArgs, sizeof(IOCTL_MEI_dsmStatus_t) );

         ret = (int)MEI_IoctlDsmStatusGet(pMeiDynCntrl, &local_args.dsm_status);
         retSize = sizeof(IOCTL_MEI_dsmStatus_t);
         break;

      case FIO_MEI_MAC_CONFIG_GET:
         MEI_DRVOS_CpyFromUser(
            &local_args, pUserArgs, sizeof(IOCTL_MEI_MacConfig_t) );

         ret = (int)MEI_IoctlMacConfigGet(pMeiDynCntrl, &local_args.mac_config);
         retSize = sizeof(IOCTL_MEI_MacConfig_t);
         #if (MEI_DBG_DSM_PROFILING == 1)
         printk("MEI_DRV[0x%02X]: bErbReset=%d (current)" MEI_DRV_CRLF,
            MEI_DRV_LINENUM_GET(pMeiDev), pMeiDev->bErbReset);
         if (pMeiDev->bErbReset == IFX_TRUE)
         {
            pMeiDev->bErbReset = IFX_FALSE;
         }
         else
         {
            pMeiDev->bErbReset = IFX_TRUE;
         }
         printk("MEI_DRV[0x%02X]: bErbReset=%d (new)" MEI_DRV_CRLF,
            MEI_DRV_LINENUM_GET(pMeiDev), pMeiDev->bErbReset);
         #endif
         break;

      case FIO_MEI_MAC_CONFIG_SET:
         MEI_DRVOS_CpyFromUser(
            &local_args, pUserArgs, sizeof(IOCTL_MEI_MacConfig_t) );

         ret = (int)MEI_IoctlMacConfigSet(pMeiDynCntrl, &local_args.mac_config);
         break;
#endif /* (MEI_SUPPORT_DSM) == 1 */

      case FIO_MEI_PLL_OFFSET_CONFIG_SET:
         MEI_DRVOS_CpyFromUser(
            &local_args, pUserArgs, sizeof(IOCTL_MEI_pllOffsetConfig_t) );

         ret = (int)MEI_IoctlPllOffsetConfigSet(pMeiDynCntrl, &local_args.pll_offset_config);
         break;

      case FIO_MEI_PLL_OFFSET_CONFIG_GET:
         MEI_DRVOS_CpyFromUser(
            &local_args, pUserArgs, sizeof(IOCTL_MEI_pllOffsetConfig_t) );

         ret = (int)MEI_IoctlPllOffsetConfigGet(pMeiDynCntrl, &local_args.pll_offset_config);
         retSize = sizeof(IOCTL_MEI_pllOffsetConfig_t);
         break;

#if (MEI_SUPPORT_DEBUG_STREAMS == 1)
      case FIO_MEI_DEBUG_STREAM_CONFIG_GET:
         MEI_DRVOS_CpyFromUser(
            &local_args, pUserArgs, sizeof(IOCTL_MEI_DEBUG_STREAM_configGet_t) );

         ret = (int)MEI_IoctlDbgStreamConfigGet(pMeiDynCntrl, &local_args.dbg_str_cfg_get);
         retSize = sizeof(IOCTL_MEI_DEBUG_STREAM_configGet_t);
         break;
      case FIO_MEI_DEBUG_STREAM_CONFIG_SET:
         MEI_DRVOS_CpyFromUser(
            &local_args, pUserArgs, sizeof(IOCTL_MEI_DEBUG_STREAM_configGet_t) );

         ret = (int)MEI_IoctlDbgStreamConfigSet(pMeiDynCntrl, &local_args.dbg_str_cfg_set);
         break;
      case FIO_MEI_DEBUG_STREAM_RELEASE:
         MEI_DRVOS_CpyFromUser(
            &local_args, pUserArgs, sizeof(IOCTL_MEI_DEBUG_STREAM_release_t) );

         ret = (int)MEI_IoctlDbgStreamRelease(pMeiDynCntrl, &local_args.dbg_str_release);
         break;
      case FIO_MEI_DEBUG_STREAM_CONTROL:
         MEI_DRVOS_CpyFromUser(
            &local_args, pUserArgs, sizeof(IOCTL_MEI_DEBUG_STREAM_control_t) );

         ret = (int)MEI_IoctlDbgStreamControl(pMeiDynCntrl, &local_args.dbg_str_control);
         break;
      case FIO_MEI_DEBUG_STREAM_STATISTIC_GET:
         MEI_DRVOS_CpyFromUser(
            &local_args, pUserArgs, sizeof(IOCTL_MEI_DEBUG_STREAM_statistic_t) );

         ret = (int)MEI_IoctlDbgStreamStatisticGet(pMeiDynCntrl, &local_args.dbg_str_stat);
         retSize = sizeof(IOCTL_MEI_DEBUG_STREAM_statistic_t);
         break;
      case FIO_MEI_DEBUG_STREAM_DATA_GET:
         MEI_DRVOS_CpyFromUser(
            &local_args, pUserArgs, sizeof(IOCTL_MEI_DEBUG_STREAM_data_t) );

         ret = (int)MEI_IoctlDbgStreamDataGet(pMeiDynCntrl, &local_args.dbg_str_data);
         retSize = sizeof(IOCTL_MEI_DEBUG_STREAM_data_t);
         break;
      case FIO_MEI_DEBUG_STREAM_MASK_SET:
         MEI_DRVOS_CpyFromUser(
            &local_args, pUserArgs, sizeof(IOCTL_MEI_DEBUG_STREAM_mask_set_t) );

         ret = (int)MEI_IoctlDbgStreamMaskSet(pMeiDynCntrl, &local_args.dbg_str_mask_set);
         retSize = sizeof(IOCTL_MEI_DEBUG_STREAM_mask_set_t);
         break;
#endif /* (MEI_SUPPORT_DEBUG_STREAMS == 1) */

      default:
         PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_WRN,
              ("MEI_DRV[%02d-%02d] Unknown IoCtl (0x%08X), arg 0x%08lX." MEI_DRV_CRLF,
               MEI_DRV_LINENUM_GET(pMeiDev), pMeiDynCntrl->openInstance, nCmd, nArgument));
         ret = -e_MEI_ERR_UNKNOWN_CMD;
   }


MEI_IOCTL_RETURN:

   local_args.drv_ioctl.retCode = ret;
   copy_to_user( ((IOCTL_MEI_arg_t *)nArgument), &local_args, retSize);

   return (ret < 0) ? -1 : 0;
}


/**
   LINUX: The select function of the driver.
   A user space program may sleep until the driver wakes it up.

\param
      file_p - pointer to the file descriptor
\param
      wait   -

\return
   \li POLLIN - data available
   \li 0 - no data
   \li POLLERR - device pointer is zero
*/
static unsigned int MEI_Poll (struct file *filp, poll_table *wait)
{
   /* While open-time: MEI CPE dynamic struct is stored in filp->private_data */
   MEI_DYN_CNTRL_T    *pDynCntrl  = (MEI_DYN_CNTRL_T *)filp->private_data;
   MEI_DEV_T          *pMeiDev  = (pDynCntrl) ? pDynCntrl->pMeiDev : NULL;
   MEI_DYN_NFC_DATA_T *pDynNfc    = (pDynCntrl) ? pDynCntrl->pInstDynNfc : NULL;

   /* check device pointer */
   if (pMeiDev == NULL)
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
             ("MEI_DRV: Internal error - NULL device pointer" MEI_DRV_CRLF));
      return POLLERR;
   }

   /* check NFC dynamic control pointer */
   if (pDynNfc == NULL)
   {
      /* receive NFC is not supported */
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
             ("MEI_DRV[%02d-%02d]: NFC not enabled - poll failed" MEI_DRV_CRLF,
             MEI_DRV_LINENUM_GET(pMeiDev), (pDynCntrl) ? pDynCntrl->openInstance : 0xff));
      return POLLERR;
   }

   MEI_DRVOS_SelectQueueAddTask(
               (MEI_DRVOS_select_OSArg_t*) filp,
               &(pMeiDev->selNfcWakeupList),
               (MEI_DRVOS_selectTable_t*)  wait);

   if (pDynNfc->pRecvDataCntrl[pDynNfc->rdIdxRd].bufCtrl != MEI_RECV_BUF_CTRL_FREE)      /* buffer in use */
   {
      /* data available */
      return POLLIN;
   }
   else
   {
      pMeiDev->bNfcNeedWakeUp = IFX_TRUE;
   }

   return 0;
}



#if (MEI_SUPPORT_IRQ == 1)

/**
   Calls the wrapped request_irq() and locks for further updates of
   the interrupt wrapper functions.

\param
   usedIrq        - This is the interrupt number being requested.
\param
   usedIsrHandler - The pointer to the handling function being installed.
\param
   usedIsrThreadedHandler - The pointer to the handling function called in dedicated kernel thread
\param
   usedFlags      - A bit mask of options related to interrupt management.
\param
   pUsedDevName   - The string passed to request_irq is used in /proc/interrupts
\param
   pUsedDevId     - This pointer is used for shared interrupt lines.

\return
   0       in case of success.
   else    if something goes wrong.

\remark
   Per default the LINUX standard request_irq() function is used.
*/
static int MEI_IfxRequestIrq( unsigned int usedIrq,
                                usedIsrHandler_t usedIsrHandler,
                                usedIsrHandler_t usedIsrThreadedHandler,
                                unsigned long usedFlags,
                                const char *pUsedDevName,
                                void *pUsedDevId)
{

   int ret;

   /* here call the wrapped request_irq() function */
   if ( (ret = MEI_RequestIrq_WrapLx( usedIrq,
                                        usedIsrHandler, usedIsrThreadedHandler,
                                        usedFlags, pUsedDevName,  pUsedDevId) ) < 0 )
   {
      /* error while register ISR */
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
             ("MEI_DRV: ERROR - WRAPPER request_irq() = %d" MEI_DRV_CRLF,
               ret));

      return ret;
   }

   /* from now on no further update is possible */
   MEI_IntSetupLocked++;

   PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_HIGH,
          ("MEI_DRV: MEI_IfxRequestIrq(IRQ = %d, .., ), lock = %d" MEI_DRV_CRLF,
          usedIrq, MEI_IntSetupLocked));

   return ret;
}

/**
   Calls the wrapped free_irq function and unlock for further updates of
   the interrupt wrapper functions

\param
   usedIrq        - This is the interrupt number being requested.
\param
   pUsedDevId     - This pointer is used for shared interrupt lines.

\return
   none

\remark
   Per default the LINUX standard free_irq() function is used.
*/
static void MEI_IfxFreeIrq(unsigned int usedIrq, void *pUsedDevId)
{
   MEI_FreeIrq_WrapLx(usedIrq, pUsedDevId);

   if (MEI_IntSetupLocked > 0)
      MEI_IntSetupLocked--;

   PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_WRN,
          ("MEI_DRV: MEI_IfxFreeIrq(IRQ = %d, .., ), lock = %d" MEI_DRV_CRLF,
          usedIrq, MEI_IntSetupLocked));

   return;
}


/**
   The driver interrupt shell.

\param
   irq - irq number
\param
   dev_id - private device data
\param
   regs - not used

\return
   None.

\remark
   None.
*/
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
static irqreturn_t MEI_InterruptLinux(int irq, void *dev_id, struct pt_regs *regs)
#else
static irqreturn_t MEI_InterruptLinux(int irq, void *dev_id)
#endif
{
   IFX_int32_t    meiIntCnt = 0;
   MEIX_CNTRL_T *pMeiXCntrlList = (MEIX_CNTRL_T*)dev_id;
   irqreturn_t ret = IRQ_RETVAL(1);
   IFX_uint8_t entity;
   IFX_uint8_t device;
   MEI_DEV_T *pMeiDev = IFX_NULL;

   pMeiXCntrlList->IRQ_Count++;

   /* PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_LOW,
            ("MEI_DRV: interrupt!" MEI_DRV_CRLF));*/

   meiIntCnt = MEI_ProcessIntPerIrq(pMeiXCntrlList);

   for (entity = 0; entity < MEI_DFEX_ENTITIES && ret != IRQ_WAKE_THREAD; ++entity)
   {
      for (device = 0; device < MEI_MAX_SUPPORTED_DFE_INSTANCE_PER_ENTITY && ret != IRQ_WAKE_THREAD; ++device)
      {
         pMeiDev = pMeiXCntrlList->MeiDevice[device];

         if(pMeiDev != IFX_NULL && pMeiDev->bHandleCallback == IFX_TRUE)
         {
            ret = IRQ_WAKE_THREAD;
         }
      }
   }

   if (meiIntCnt)
      pMeiXCntrlList->IRQ_Protection = MEI_IrqProtectCount;
   else
      pMeiXCntrlList->IRQ_Protection--;

   if (pMeiXCntrlList->IRQ_Protection <= 0)
   {
      /* The OS signals available IRQ's but no interrupt found for processing */
      PRN_ERR_INT_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_HIGH, (MEI_DRV_CRLF));
      PRN_ERR_INT_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_HIGH,
            ("MEI_DRV: IRQ setup - !!! FATAL ERROR !!!" MEI_DRV_CRLF));
      PRN_ERR_INT_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_HIGH, (MEI_DRV_CRLF));
      PRN_ERR_INT_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_HIGH,
            ("MEI_DRV: +++  R I P   R I P   R I P  +++" MEI_DRV_CRLF));
      PRN_ERR_INT_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_HIGH,
            ("MEI_DRV: +++        - IRQ %02d  -      +++" MEI_DRV_CRLF,
             pMeiXCntrlList->IRQ_Num));
      PRN_ERR_INT_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_HIGH,
            ("MEI_DRV: +++ -------- R I P -------- +++" MEI_DRV_CRLF));
      PRN_ERR_INT_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_HIGH, (MEI_DRV_CRLF));

      MEI_IfxFreeIrq(pMeiXCntrlList->IRQ_Num, pMeiXCntrlList);

      /* mark all devices with config error */
      MEI_DisableDevsPerIrq(pMeiXCntrlList);
   }

   return ret;
}
#endif

static irqreturn_t MEI_InterruptThreadLinux(int irq, void *dev_id)
{
   MEIX_CNTRL_T *pMeiXCntrlList = (MEIX_CNTRL_T*)dev_id;
   IFX_uint8_t entity;
   IFX_uint8_t device;
   MEI_DEV_T *pMeiDev = IFX_NULL;
   IFX_boolean_t handled = IFX_FALSE;

   for (entity = 0; entity < MEI_DFEX_ENTITIES && !handled; ++entity)
   {
      for (device = 0; device < MEI_MAX_SUPPORTED_DFE_INSTANCE_PER_ENTITY && !handled; ++device)
      {
         pMeiDev = pMeiXCntrlList->MeiDevice[device];
         
         if(pMeiDev != IFX_NULL && pMeiDev->bHandleCallback == IFX_TRUE)
         {
            MEI_HandleCallback(pMeiDev);
            handled = IFX_TRUE;
         }
      }
   }

   return IRQ_HANDLED;
}

#if CONFIG_PROC_FS
typedef void (*proc_rd_callback_t)(struct seq_file *);
typedef ssize_t (*proc_wr_callback_t)(struct file *file, const char *buf, size_t count, loff_t *ppos);

struct proc_entry {
   char name[32];
   proc_rd_callback_t rd;
   proc_wr_callback_t wr;
   struct file_operations ops;
   int entity;
};

/**
   Read the version information from the driver.

\param
   buf destination buffer.

\return
   length
*/
static void MEI_GetVersionProc(struct seq_file *s)
{
   seq_printf(s, "%s" MEI_DRV_CRLF, &MEI_WHATVERSION[4]);
   seq_printf(s, "Compiled on %s, %s for Linux kernel %s (jiffies: %ld)" MEI_DRV_CRLF,
                                    __DATE__, __TIME__, UTS_RELEASE, jiffies);
}

/**
   Read the version information from the driver.

\param
   buf destination buffer.

\return
   length
*/
static void MEI_GetDevInfoProc(struct seq_file *s)
{
   seq_printf(s, "MaxDeviceNumber=%d\n",MEI_DFEX_ENTITIES);
   seq_printf(s, "LinesPerDevice=%d\n",MEI_DFE_INSTANCE_PER_ENTITY);
   seq_printf(s, "ChannelsPerLine=%d\n",MEI_DEVICE_CFG_VALUE_GET(ChannelsPerLine));
}

/**
   Read the status information from the driver.

\param
   buf   destination buffer

\param
   entity MEIx device

\return
  length
*/
static void MEI_GetStatusProcPerDev(struct seq_file *s)
{
   int devNum;
   MEI_DEV_T *pMeiDev;
   struct proc_entry *p = s->private;
   int entity = p->entity;

   if (entity < MEI_DFEX_ENTITIES)
   {
      seq_printf(s, "********************************" MEI_DRV_CRLF);
      seq_printf(s, "pMEIX(%d) = 0x%08X" MEI_DRV_CRLF, entity, (int)MEIX_Cntrl[entity]);
      seq_printf(s, "++++++++++++++++++++++++++++++++" MEI_DRV_CRLF);
      if (MEIX_Cntrl[entity] != NULL)
      {
#if (MEI_SUPPORT_IRQ == 1)
         seq_printf(s, "IRQ Count = %d" MEI_DRV_CRLF MEI_DRV_CRLF,
                        MEIX_Cntrl[entity]->IRQ_Count);
#endif
         for (devNum=0; devNum < MEI_DFE_INSTANCE_PER_ENTITY; devNum++)
         {
            seq_printf(s, MEI_DRV_CRLF "pMeiCh[%d] = 0x%08X" MEI_DRV_CRLF,
                                     (entity*MEI_DFE_INSTANCE_PER_ENTITY) + devNum,
                                     (int)MEIX_Cntrl[entity]->MeiDevice[devNum]);
            seq_printf(s, "--------------------------------" MEI_DRV_CRLF);

            if (MEIX_Cntrl[entity]->MeiDevice[devNum] != NULL)
            {
               pMeiDev = MEIX_Cntrl[entity]->MeiDevice[devNum];

               seq_printf(s, "HW Vers    = %5d\tMEI State   = %s" MEI_DRV_CRLF,
                              pMeiDev->modemData.hwVersion,
                              (MEI_DRV_MEI_IF_STATE_GET(pMeiDev))?
                                 ((MEI_DRV_MEI_IF_STATE_GET(pMeiDev) == e_MEI_MEI_HW_STATE_UP)? "UP":"DOWN") : "unknown");
               seq_printf(s, "bOpen      = %5d\tDrv State   = %5d\tModemFSM = %d" MEI_DRV_CRLF,
                              pMeiDev->openCount,
                              MEI_DRV_STATE_GET(pMeiDev),
                              MEI_DRV_MODEM_STATE_GET(pMeiDev));
#if (MEI_SUPPORT_STATISTICS == 1)
               seq_printf(s, "DrvSwRst   = %5d\tMeiHwRst    = %5d" MEI_DRV_CRLF,
                              pMeiDev->statistics.drvSwRstCount,
                              pMeiDev->statistics.meiHwRstCount);
               seq_printf(s, "GP1 Int    = %5d\tMsgAv Int   = %5d" MEI_DRV_CRLF,
                              pMeiDev->statistics.dfeGp1IntCount,
                              pMeiDev->statistics.dfeMsgAvIntCount);
               seq_printf(s, "FwDownl    = %5d\tCodeSwap(%s) = %5d" MEI_DRV_CRLF,
                              pMeiDev->statistics.fwDlCount,
                              (MEI_BM7_CODESWAP_MEIDBG == 1) ? "M" : "D",
                              pMeiDev->statistics.dfeCodeSwapCount);
#if ((MEI_SUPPORT_TIME_TRACE == 1) && (MEI_SUPPORT_ROM_CODE == 1) && (MEI_SUPPORT_DL_DMA_CS == 1))
               seq_printf(s, "FwDownlErr = %5d\tCS max Time = %5d [ms]" MEI_DRV_CRLF,
                              pMeiDev->statistics.fwDlErrCount,
                              pMeiDev->timeStat.processCsMax_ms);
#else
               seq_printf(s, "FwDownlErr = %5d" MEI_DRV_CRLF,
                              pMeiDev->statistics.fwDlErrCount);
#endif

#if (MEI_SUPPORT_OPTIMIZED_FW_DL == 1)
               seq_printf(s, "FwDownlOpt = %5d" MEI_DRV_CRLF,
                              pMeiDev->statistics.fwDlOptSuccessCount);
               seq_printf(s, "FwDOptFail = %5d" MEI_DRV_CRLF,
                              pMeiDev->statistics.fwDlOptFailedCount);
#endif

               seq_printf(s, "TxMsg      = %5d\tRxAck       = %5d" MEI_DRV_CRLF,
                              pMeiDev->statistics.sendMsgCount,
                              pMeiDev->statistics.recvAckCount);
               seq_printf(s, "RxMsg      = %5d\tRxMsgDisc   = %5d" MEI_DRV_CRLF,
                              pMeiDev->statistics.recvMsgCount,
                              pMeiDev->statistics.recvMsgDiscardCount);
               seq_printf(s, "RxMsgErr   = %5d\tTxMsgErr    = %5d" MEI_DRV_CRLF,
                              pMeiDev->statistics.recvMsgErrCount,
                              pMeiDev->statistics.errorCount);
               seq_printf(s, "Nfc        = %5d\tNfcDisc     = %5d" MEI_DRV_CRLF,
                              pMeiDev->statistics.recvNfcCount,
                              pMeiDev->statistics.recvNfcDiscardCount);
               seq_printf(s, "NfcDist    = %5d\tNfcDistDisc = %5d" MEI_DRV_CRLF MEI_DRV_CRLF,
                              pMeiDev->statistics.recvNfcDistCount,
                              pMeiDev->statistics.recvNfcDistDiscardCount);
#endif   /* #if (MEI_SUPPORT_STATISTICS == 1) */

#if (MEI_SUPPORT_TIME_TRACE == 1)
               seq_printf(s, "WSendMin   = %5d\tWSendMax    = %5d [ms]" MEI_DRV_CRLF,
                              pMeiDev->timeStat.waitSendMin_ms,
                              pMeiDev->timeStat.waitSendMax_ms);
               seq_printf(s, "WAckMin    = %5d\tWAckMax     = %5d [ms]" MEI_DRV_CRLF MEI_DRV_CRLF,
                              pMeiDev->timeStat.waitAckMin_ms,
                              pMeiDev->timeStat.waitAckMax_ms);
#endif   /* #ifdef MEI_SUPPORT_TIME_TRACE */

            }
         }        /* for ( ; devNum<MEI_DFE_INSTANCE_PER_ENTITY; ) {...} */
      }        /* if (MEIX_Cntrl[entity] != NULL) {...} */
   }        /* if (entity < MEI_MAX_DFEX_ENTITIES) {...} */
}



#if (MEI_SUPPORT_DRV_NFC_DEBUG == 1)
/**
   Read the status information from the driver.

\param
   buf   destination buffer

\param
   entity MEIx device

\return
  length
*/
static void MEI_GetNfcProcPerDev(struct seq_file *s)
{
   int devNum;
   MEI_DEV_T *pMeiDev;
   struct proc_entry *p = s->private;
   int entity = p->entity;
   char* nfcDisplay;

   nfcDisplay = MEI_DRVOS_Malloc(MEI_NFC_DISPLAY_BUFFER_SIZE);
   if (!nfcDisplay)
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DRV: no memory for NFC data" MEI_DRV_CRLF));
      return;
   }

   if (entity < MEI_DFEX_ENTITIES)
   {
      seq_printf(s, "********************************" MEI_DRV_CRLF);
      seq_printf(s, "pMEIX(%d) = 0x%08X" MEI_DRV_CRLF, entity, (int)MEIX_Cntrl[entity]);
      seq_printf(s, "++++++++++++++++++++++++++++++++" MEI_DRV_CRLF);
      if (MEIX_Cntrl[entity] != NULL)
      {
         for (devNum=0; devNum<MEI_DFE_INSTANCE_PER_ENTITY; devNum++)
         {
            if (MEIX_Cntrl[entity]->MeiDevice[devNum] != NULL)
            {
               pMeiDev = MEIX_Cntrl[entity]->MeiDevice[devNum];
               MEI_ShowNfcData(pMeiDev, nfcDisplay, MEI_NFC_DISPLAY_BUFFER_SIZE);
               seq_printf(s, "%s", nfcDisplay);
            }
         }        /* for ( ; devNum<MEI_DFE_INSTANCE_PER_ENTITY; ) {...} */
      }        /* if (MEIX_Cntrl[entity] != NULL) {...} */
   }        /* if (entity < MEI_MAX_DFEX_ENTITIES) {...} */

   MEI_DRVOS_Free(nfcDisplay);
}
#endif

/**
   Read the memory status information from the driver.

\param
   buf   destination buffer

\param
   entity MEIx device

\return
  length
*/
static void MEI_MeminfoProcPerDevGet(struct seq_file *s)
{
   int devNum;
   MEI_DEV_T *pMeiDev;
   struct proc_entry *p = s->private;
   int entity = p->entity;

   MEI_FW_DOWNLOAD_CNTRL_T *pFwDlCtrl;
   MEI_FW_IMAGE_CHUNK_CTRL_T *pChunk;
   MEI_MEI_DRV_CNTRL_T *pMeiDrvCntrl;
   IFX_uint32_t chunkIdx, barIdx, currBAR;
#if (MEI_SUPPORT_DEVICE_VR11 == 1)
   IFX_uint8_t *ppPDBRAM = 0;
#else
   IFX_uint8_t *ppPDBRAM =
      (IFX_uint8_t *)(MEI_INTERNAL_ADDRESS_BASE + MEI_PDBRAM_OFFSET);
#endif

   printk ("\n++++++++++++++++++ MEI_MeminfoProcPerDevGet ++++++++++++++++++\n\n");
   if ((entity < MEI_DFEX_ENTITIES) && (MEIX_Cntrl[entity] != NULL))
   {
      for (devNum=0; devNum<MEI_DFE_INSTANCE_PER_ENTITY; devNum++)
      {
         if (MEIX_Cntrl[entity]->MeiDevice[devNum] != NULL)
         {
            pMeiDev = MEIX_Cntrl[entity]->MeiDevice[devNum];
            pMeiDrvCntrl = &(pMeiDev->meiDrvCntrl);
#if (MEI_SUPPORT_DEVICE_VR11 == 1)
            ppPDBRAM = (IFX_uint8_t *)(MEI_DRV_PCIE_PHY_MEMBASE_GET(&pMeiDev->meiDrvCntrl)
                                                                + MEI_PDBRAM_OFFSET);
#endif /* (MEI_SUPPORT_DEVICE_VR11 == 1) */
            pFwDlCtrl = &(pMeiDev->fwDl);
            pChunk = pMeiDev->fwDl.imageChunkCtrl;

            for (chunkIdx = 0; chunkIdx < pFwDlCtrl->meiMaxChunkCount; chunkIdx++)
            {
               if (pChunk[chunkIdx].eImageChunkType == eMEI_FW_IMAGE_CHUNK_UNDEFINED)
                  continue;

               seq_printf(s, "chunk[%02d]: addr = %p (%p), "
                     "size = %5d [byte], type = %d"
#if (MEI_SUPPORT_OPTIMIZED_FW_DL == 1)
                     ", CRC = 0x%08X"
#endif
                     MEI_DRV_CRLF,
                     chunkIdx, pChunk[chunkIdx].pImageChunk_aligned,
                     pChunk[chunkIdx].pImageChunk_allocated,
                     pChunk[chunkIdx].imageChunkSize_byte,
                     pChunk[chunkIdx].eImageChunkType
#if (MEI_SUPPORT_OPTIMIZED_FW_DL == 1)
                     , pChunk[chunkIdx].imageChunkCRC
#endif
                     );
            }

            seq_printf(s, MEI_DRV_CRLF);

            for (barIdx = 0; barIdx < MEI_TOTAL_BAR_REGISTER_COUNT + MEI_TOTAL_BAR_REGISTER_COUNT2; barIdx++)
            {
               currBAR = MEI_REG_ACCESS_ME_XMEM_BAR_GET(pMeiDrvCntrl, barIdx);

               seq_printf(s, "BAR[%02d] = 0x%08X", barIdx, currBAR);

               switch(MEI_BAR_TYPE_GET(pMeiDev, barIdx))
               {
                  case eMEI_BAR_TYPE_UNUSED:
                     seq_printf(s, " (unused)");
                     break;

                  case eMEI_BAR_TYPE_USER:
                     seq_printf(s, " (user debug)");
                     break;

                  case eMEI_BAR_TYPE_PDBRAM:
                     if ((! MEI_DEVICE_CFG_IS_PLATFORM(e_MEI_DEV_PLATFORM_CONFIG_VR9))
                          && (barIdx < MEI_MAX_CACHE_CHUNK_COUNT))
                     {
                        if (currBAR == (IFX_int32_t)ppPDBRAM + barIdx*MEI_FW_IMAGE_CHUNK_SIZE_BYTE)
                        {
                           /* BAR0, BAR1 (, BAR2) are pointing to the PDBRAM */
                           seq_printf(s, " (-> PDBRAM+0x%05X)", barIdx*MEI_FW_IMAGE_CHUNK_SIZE_BYTE);
                        }
                     }
                     break;

#if (MEI_SUPPORT_DSM == 1)
                  case eMEI_BAR_TYPE_ERB:
                     seq_printf(s, " (-> ERB block)");
                     break;
#endif /* (MEI_SUPPORT_DSM == 1) */

                  case eMEI_BAR_TYPE_CHUNK:
                  case eMEI_BAR_TYPE_SPECIAL:
                        for(chunkIdx = 0; chunkIdx < pFwDlCtrl->meiMaxChunkCount; chunkIdx++)
                        {
                           if (currBAR == (IFX_uint32_t)pChunk[chunkIdx].pBARx)
                           {
                              seq_printf(s, " (-> chunk[%02d])", chunkIdx);
                              break;
                           }
                        }
                        break;

                  default:
                     seq_printf(s, " (-> unknown BAR type)");
               }
               seq_printf(s, MEI_DRV_CRLF);
            }
         }
      }
   }
}

#if (MEI_PREDEF_DBG_BAR == 1)
static int MEI_BarUsrDbgProcPerDevSet(struct file *file,
                  const char *buf, size_t count, loff_t *ppos)
{
   int devNum;
   MEI_DEV_T *pMeiDev;
   struct seq_file *s = file->private_data;
   struct proc_entry *p = s->private;
   int entity = p->entity;
   char proc_str[16] = { '\0' };
   int barIdx = -1;
   uint32_t bar_addr = 0;

   if ((entity < MEI_DFEX_ENTITIES) && (MEIX_Cntrl[entity] != NULL))
   {
      for (devNum=0; devNum<MEI_DFE_INSTANCE_PER_ENTITY; devNum++)
      {
         if (MEIX_Cntrl[entity]->MeiDevice[devNum] != NULL)
         {
            pMeiDev = MEIX_Cntrl[entity]->MeiDevice[devNum];

            if (count > sizeof(proc_str) - 1)
            {
               return -EINVAL;
            }

            if (copy_from_user(proc_str, buf, count))
            {
               return -EFAULT;
            }

            proc_str[count] = 0;

            sscanf(proc_str, "%d %x", &barIdx, &bar_addr);

            if ((barIdx >= MEI_TOTAL_BAR_REGISTER_COUNT + MEI_TOTAL_BAR_REGISTER_COUNT2) || (barIdx < 0))
            {
               PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_OFF,
                     ("BAR register set: BAR[%02d] invalid index " MEI_DRV_CRLF, barIdx));
               break;
            }

            if ((MEI_BAR_TYPE_GET(pMeiDev, barIdx) == eMEI_BAR_TYPE_USER) ||
                (MEI_BAR_TYPE_GET(pMeiDev, barIdx) == eMEI_BAR_TYPE_UNUSED))
            {
               PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_OFF,
                     ("BAR register set: BAR[%02d]=0x%08X" MEI_DRV_CRLF, barIdx, bar_addr));
               PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_OFF,
                     ("Please note that changes will be valid after link restart only!"
                      MEI_DRV_CRLF));

               MEI_REG_ACCESS_ME_XMEM_BAR_SET(&(pMeiDev->meiDrvCntrl), barIdx, bar_addr);
               MEI_BAR_DBG_ADDR_SET(pMeiDev, barIdx, bar_addr);

               if (bar_addr == 0)
               {
                  MEI_BAR_TYPE_SET(pMeiDev, barIdx, eMEI_BAR_TYPE_UNUSED);
               }
               else
               {
                  MEI_BAR_TYPE_SET(pMeiDev, barIdx, eMEI_BAR_TYPE_USER);
               }
            }
            else
            {
               PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_OFF,
                     ("BAR register set: BAR[%02d] already in use " MEI_DRV_CRLF, barIdx));
            }
         }
      }
   }

   return count;
}
#endif /* (MEI_PREDEF_DBG_BAR == 1) */

static int mei_seq_single_show(struct seq_file *s, void *v)
{
   struct proc_entry *p = s->private;
   if (p->rd)
      p->rd(s);
   return 0;
}

static int mei_proc_single_open(struct inode *inode, struct file *file)
{
   return single_open(file, mei_seq_single_show, PDE_DATA(inode));
}

static void mei_proc_entry_create(struct proc_dir_entry *parent_node,
                                  struct proc_entry *proc_entry)
{
   memset(&proc_entry->ops, 0, sizeof(struct file_operations));
   proc_entry->ops.owner = THIS_MODULE;

   proc_entry->ops.open = mei_proc_single_open;
   proc_entry->ops.release = single_release;

   proc_entry->ops.read = seq_read;
   proc_entry->ops.llseek = seq_lseek;
   if (proc_entry->wr)
      proc_entry->ops.write = proc_entry->wr;

   proc_create_data(proc_entry->name,
                     (S_IFREG | S_IRUGO),
                     parent_node, &proc_entry->ops, proc_entry);
}

static struct proc_entry proc_entry_version = {"version", MEI_GetVersionProc, NULL};
static struct proc_entry proc_entry_devinfo = {"devinfo", MEI_GetDevInfoProc, NULL};
static struct proc_entry proc_entry_status[MEI_MAX_SUPPORTED_DFEX_ENTITIES];
#if (MEI_SUPPORT_DRV_NFC_DEBUG == 1)
static struct proc_entry proc_entry_nfc[MEI_MAX_SUPPORTED_DFEX_ENTITIES];
#endif

static struct proc_entry proc_entry_meminfo[MEI_MAX_SUPPORTED_DFEX_ENTITIES];
#if (MEI_PREDEF_DBG_BAR == 1)
static struct proc_entry proc_entry_bar_usr_dbg[MEI_MAX_SUPPORTED_DFEX_ENTITIES];
#endif

/**
   Initialize and install the proc entry

\param
   devName

\return
   -1 or 0 on success
\remark
   Called by the kernel.
*/
static int MEI_InstallProcEntry(unsigned char entity)
{
   static struct proc_dir_entry *driver_proc_node = NULL;
   static struct proc_dir_entry *driver_status_proc_node = NULL;
   static struct proc_dir_entry *driver_nfc_proc_node = NULL;
   static struct proc_dir_entry *driver_meminfo_proc_node = NULL;
   static struct proc_dir_entry *driver_bar_usr_dbg_proc_node = NULL;


   /* install the proc entry */
   PRN_DBG_USR_NL( MEI_DRV,MEI_DRV_PRN_LEVEL_LOW,
         ("MEI_DRV: using proc fs" MEI_DRV_CRLF));
   if (!driver_proc_node)
   {
      driver_proc_node = proc_mkdir( "driver/" DRV_MEI_NAME, NULL);
      if (driver_proc_node != NULL)
      {
         mei_proc_entry_create(driver_proc_node, &proc_entry_version);
         mei_proc_entry_create(driver_proc_node, &proc_entry_devinfo);
#if (MEI_SUPPORT_PROCFS_CONFIG == 1)
         MEI_InstallProcEntryConfig(driver_proc_node);
#endif
      }
      else
      {
         PRN_DBG_USR_NL( MEI_DRV,MEI_DRV_PRN_LEVEL_HIGH,
                         ("MEI_DRV: cannot create proc entry version" MEI_DRV_CRLF));
         return -e_MEI_ERR_DEV_INIT;
      }
   }

   if (driver_status_proc_node == NULL)
   {
      driver_status_proc_node = proc_mkdir( "driver/" DRV_MEI_NAME "/status", NULL);
   }
   if (driver_status_proc_node != NULL)
   {
      char buf[8];
      sprintf(buf,"%02d",entity);
      strcpy(proc_entry_status[entity].name, buf);
      proc_entry_status[entity].entity = entity;
      proc_entry_status[entity].rd = MEI_GetStatusProcPerDev;
      mei_proc_entry_create(driver_status_proc_node, &proc_entry_status[entity]);
   }
   else
   {
      PRN_DBG_USR_NL( MEI_DRV,MEI_DRV_PRN_LEVEL_HIGH,
            ("MEI_DRV: cannot create proc entry status" MEI_DRV_CRLF));
      return -e_MEI_ERR_DEV_INIT;
   }


#if (MEI_SUPPORT_DRV_NFC_DEBUG == 1)
   if (driver_nfc_proc_node == NULL)
   {
      driver_nfc_proc_node = proc_mkdir( "driver/" DRV_MEI_NAME "/nfc", NULL);
   }
   if (driver_nfc_proc_node != NULL)
   {
      char buf[8];
      sprintf(buf,"%02d",entity);
      strcpy(proc_entry_nfc[entity].name, buf);
      proc_entry_nfc[entity].entity = entity;
      proc_entry_nfc[entity].rd = MEI_GetNfcProcPerDev;
      mei_proc_entry_create(driver_nfc_proc_node, &proc_entry_nfc[entity]);
   }
   else
   {
      PRN_DBG_USR_NL( MEI_DRV,MEI_DRV_PRN_LEVEL_HIGH,
            ("MEI_DRV: cannot create proc entry nfc" MEI_DRV_CRLF));
      return -e_MEI_ERR_DEV_INIT;
   }
#endif

   if (driver_meminfo_proc_node == NULL)
   {
      driver_meminfo_proc_node = proc_mkdir( "driver/" DRV_MEI_NAME "/meminfo", NULL);
   }
   if (driver_meminfo_proc_node != NULL)
   {
      char buf[8];
      sprintf(buf,"%02d",entity);
      strcpy(proc_entry_meminfo[entity].name, buf);
      proc_entry_meminfo[entity].entity = entity;
      proc_entry_meminfo[entity].rd = MEI_MeminfoProcPerDevGet;
      mei_proc_entry_create(driver_meminfo_proc_node, &proc_entry_meminfo[entity]);
   }
   else
   {
      PRN_DBG_USR_NL( MEI_DRV,MEI_DRV_PRN_LEVEL_HIGH,
            ("MEI_DRV: cannot create proc entry meminfo" MEI_DRV_CRLF));
      return -e_MEI_ERR_DEV_INIT;
   }

#if (MEI_PREDEF_DBG_BAR == 1)
   if (driver_bar_usr_dbg_proc_node == NULL)
   {
      driver_bar_usr_dbg_proc_node = proc_mkdir( "driver/" DRV_MEI_NAME "/bar_usr_dbg", NULL);
   }
   if (driver_bar_usr_dbg_proc_node != NULL)
   {
      char buf[8];
      sprintf(buf,"%02d",entity);
      strcpy(proc_entry_bar_usr_dbg[entity].name, buf);
      proc_entry_bar_usr_dbg[entity].entity = entity;
      proc_entry_bar_usr_dbg[entity].wr = MEI_BarUsrDbgProcPerDevSet;
      mei_proc_entry_create(driver_bar_usr_dbg_proc_node, &proc_entry_bar_usr_dbg[entity]);
   }
   else
   {
      PRN_DBG_USR_NL( MEI_DRV,MEI_DRV_PRN_LEVEL_HIGH,
            ("MEI_DRV: cannot create proc entry meminfo" MEI_DRV_CRLF));
      return -e_MEI_ERR_DEV_INIT;
   }
#endif /* (MEI_PREDEF_DBG_BAR == 1) */

   return 0;
}
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0))
/* contains the number of devices found. */
static int dev_found = 0;
/* containt the number of devices registered in module_init */
static int dev_registered = 0;

static int ltq_dsl_cpe_mei_remove(struct platform_device *pdev)
{
   /* clear platform */
   MEI_DEVICE_CFG_VALUE_SET(platform, e_MEI_DEV_PLATFORM_CONFIG_UNKNOWN);
#ifdef CONFIG_OF
    if (dev_registered > 0)
    {
        dev_registered--;
    }
#endif
   return 0;
}

static const struct of_device_id ltq_dsl_cpe_mei_match[] = {
   { .compatible = "lantiq,mei-xrx200", .data = (void *)e_MEI_DEV_PLATFORM_CONFIG_VR9},
   { .compatible = "lantiq,mei-xrx300", .data = (void *)e_MEI_DEV_PLATFORM_CONFIG_VR10},
   /** \todo [VRX518] Check about removing device tree handling for
       VRX300/500 based platforms as the information is anyhow provided by
       the PCIe EP Driver. */
   { .compatible = "lantiq,mei-xrx500", .data = (void *)e_MEI_DEV_PLATFORM_CONFIG_VR11},
   {},
};
MODULE_DEVICE_TABLE(of, ltq_dsl_cpe_mei_match);

static int ltq_dsl_cpe_mei_probe(struct platform_device *pdev);

static struct platform_driver ltq_dsl_cpe_mei_driver = {
   .probe = ltq_dsl_cpe_mei_probe,
   .driver = {
      .name           = DRV_MEI_NAME,
      .owner          = THIS_MODULE,
      .of_match_table = ltq_dsl_cpe_mei_match,
   },
   .remove = ltq_dsl_cpe_mei_remove,
};

static int ltq_dsl_cpe_mei_probe(struct platform_device *pdev)
{
   int ret;
   struct resource baseres, irqres;
#if (MEI_SUPPORT_DEVICE_VR10_320 != 1) && (MEI_SUPPORT_DEVICE_VR11 != 1)
   const struct of_device_id *match;
#endif
   struct device *dev = &pdev->dev;

   dev_found++;
   if (dev_registered == 0)
   {
       MEI_DEVICE_CFG_VALUE_SET(MaxDeviceNumber, dev_found);
       MEI_DEVICE_CFG_VALUE_SET(DfeChanDevices, dev_found);
   }
   else
   {
       if (dev_found > dev_registered) {

           dev_info(dev, "too many devices found\n");
           dev_found--;
           return -ENODEV;
       }
   }

   memset(&baseres, 0x00, sizeof(struct resource));
   memset(&irqres, 0x00, sizeof(struct resource));

   /* clear platform */
   MEI_DEVICE_CFG_VALUE_SET(platform, e_MEI_DEV_PLATFORM_CONFIG_UNKNOWN);

#if (MEI_SUPPORT_DEVICE_VR11 == 1)
   /** \todo Rework the handling that identifies the used platform,finally
       device tree should not be required here anymore */
   MEI_DEVICE_CFG_VALUE_SET(platform,  e_MEI_DEV_PLATFORM_CONFIG_VR11);
#elif (MEI_SUPPORT_DEVICE_VR10_320 != 1)
   match = of_match_node(ltq_dsl_cpe_mei_match, pdev->dev.of_node);
   if (match) {
       MEI_DEVICE_CFG_VALUE_SET(platform,  (MEI_DEV_PLATFORM_CONFIG_E)match->data);
   }
   else if (pdev->dev.platform_data)
   {
       MEI_DEVICE_CFG_VALUE_SET(platform, *(MEI_DEV_PLATFORM_CONFIG_E *)pdev->dev.platform_data);
   }
   else
   {
      dev_err(&pdev->dev, "failed to identify platform relase\n");
      return -ENODEV;
   }

#endif

   if (MEI_DEVICE_CFG_IS_PLATFORM(e_MEI_DEV_PLATFORM_CONFIG_VR9))
   {
      /* get base address & irq numbers */
      ret = of_address_to_resource(pdev->dev.of_node, 0, &baseres);
      if (ret) {
         dev_err(&pdev->dev, "could not determine device base address\n");

         /* clear platform */
         MEI_DEVICE_CFG_VALUE_SET(platform, e_MEI_DEV_PLATFORM_CONFIG_UNKNOWN);

         return ret;
      }

      if (!of_irq_to_resource(pdev->dev.of_node, 0, &irqres))
      {
         dev_err(&pdev->dev, "no IRQ found\n");

         /* clear platform */
         MEI_DEVICE_CFG_VALUE_SET(platform, e_MEI_DEV_PLATFORM_CONFIG_UNKNOWN);

         return -ENODEV;
      }
   }

   MEI_DEVICE_CFG_VALUE_SET(nIrq,      irqres.start);
   MEI_DEVICE_CFG_VALUE_SET(nBaseAddr, baseres.start);
   MEI_DEVICE_CFG_VALUE_SET(clk,       clk_get(&pdev->dev, "dfe"));
#if (MEI_SUPPORT_DEVICE_VR10_320 == 1) || (MEI_TARGET_x86 == 1)
   MEI_DEVICE_CFG_VALUE_SET(dev,       pdev->dev.parent);
#else
   MEI_DEVICE_CFG_VALUE_SET(dev,       &pdev->dev);
#endif

   ret = MEI_driver_init(dev_found -1);

   return ret;
}
#endif

/**
   Initialize the module (support devfs - device file system)

   \return
   Error code or 0 on success
   \remark
   Called by the kernel.
*/
static int MEI_driver_init (int entity)
{

   int result;
#if (MEI_SUPPORT_DEBUG_LOGGER == 1)
   struct netlink_kernel_cfg cfg = {
       .groups = NL_DBG_MSG_GROUP,
   };

   if (!entity)
   {
       nl_debug_sock = netlink_kernel_create(&init_net,
                                         DSL_DBG_MSG_NETLINK_ID,
                                         &cfg);
       if (!nl_debug_sock)
       {
           printk("Could not create netlink socket.\n");
       }
   }
#endif

   if (!entity)
   {
       MEI_DRV_PRN_USR_LEVEL_SET(MEI_DRV, debug_level);
       MEI_DRV_PRN_INT_LEVEL_SET(MEI_DRV, debug_level);

       MEI_DRV_PRN_USR_LEVEL_SET(MEI_MSG_DUMP_API, debug_level);
       MEI_DRV_PRN_INT_LEVEL_SET(MEI_MSG_DUMP_API, debug_level);

       MEI_DRV_PRN_USR_LEVEL_SET(MEI_NOTIFICATIONS, debug_level);
       MEI_DRV_PRN_INT_LEVEL_SET(MEI_NOTIFICATIONS, debug_level);

#if (MEI_SUPPORT_ROM_CODE == 1)
       MEI_DRV_PRN_USR_LEVEL_SET(MEI_ROM, debug_level);
       MEI_DRV_PRN_INT_LEVEL_SET(MEI_ROM, debug_level);
#endif

       MEI_DRV_PRN_USR_LEVEL_SET(MEI_MEI_ACCESS, debug_level);
       MEI_DRV_PRN_INT_LEVEL_SET(MEI_MEI_ACCESS, debug_level);

       MEI_FsmStateSetMsgPreAction = (IFX_uint32_t)(fsm_set_pre_action & MEI_FSM_STATE_SET_PRE_ACT_ALL);

   }
#if (MEI_SUPPORT_DEVICE_VR10_320 == 1)
       MEI_DEVICE_CFG_VALUE_SET(platform, e_MEI_DEV_PLATFORM_CONFIG_VR10_320);
#endif

   /* ============================================
      register device
      ============================================ */
   result = MEI_InitModuleRegCharDev(DRV_MEI_NAME, entity);
   if (result != 0)
   {
      MEI_driver_exit();
      return (result);
   }

   /* ============================================
      Do common init_module() stuff
      ============================================ */
   if (!entity)
   {
       result = MEI_InitModuleBasics();
       if (result != 0)
       {
           MEI_driver_exit();
           return (result);
       }
   }

#ifdef PPA_SUPPORTS_CALLBACKS
#if (MEI_DRV_ATM_PTM_INTERFACE_ENABLE == 1)
   ppa_callback_set(LTQ_MEI_SHOWTIME_CHECK, (void *)ltq_mei_showtime_check);
#endif /* #if (MEI_DRV_ATM_PTM_INTERFACE_ENABLE == 1) */
#endif /* PPA_SUPPORTS_CALLBACKS */

   return 0;
}

/**
   Clean up the module if unloaded.

   \remark
   Called by the kernel.
*/
static void MEI_driver_exit (void)
{
   int entity;
   int channel_num;
   char path[255];
#ifndef CONFIG_DEVFS_FS
   static dev_t mei_devt;
#endif

   printk("MEI_DRV: Module will be unloaded" MEI_DRV_CRLF);

#if (MEI_SUPPORT_DEBUG_STREAMS == 1)
   (void)MEI_DBG_STREAM_ModuleDelete();
#endif

#if (MEI_SUPPORT_PERIODIC_TASK == 1)
   if ( MEI_DrvCntrlThreadParams.bValid == IFX_TRUE)
   {
      PRN_DBG_USR_NL( MEI_DRV,MEI_DRV_PRN_LEVEL_HIGH,
            ("MEI_DRV: stop Periodic Task" MEI_DRV_CRLF));

      MEI_DRVOS_ThreadDelete(&MEI_DrvCntrlThreadParams);
   }
#endif   /* #if (MEI_SUPPORT_PERIODIC_TASK == 1) */

   PRN_DBG_USR_NL( MEI_DRV,MEI_DRV_PRN_LEVEL_HIGH,
         ("MEI_DRV: MEI IRQ list" MEI_DRV_CRLF));

   /* release the MEIx IRQ list */
   for (entity = 0; entity < MEI_DFEX_ENTITIES; entity++)
   {
      if (!MEIX_Cntrl[entity])
         continue;

      if ( (MEIX_Cntrl[entity]->IRQ_Base != 0) &&
           (MEIX_Cntrl[entity]->IRQ_Base != 99) )
      {
         PRN_DBG_USR_NL( MEI_DRV,MEI_DRV_PRN_LEVEL_HIGH,
               ("MEI_DRV: MEIx[%02d] free IRQ %d (%s)" MEI_DRV_CRLF,
                entity, MEIX_Cntrl[entity]->IRQ_Base,
                DRV_MEI_NAME));

         /* free interrupt and clear list */
         MEI_IfxFreeIrq( (int)MEIX_Cntrl[entity]->IRQ_Base,
                         (void *)MEIX_Cntrl[entity] );
         MEIX_Cntrl[entity]->IRQ_Base = 0;

         MEI_VrxXDevFromIrqListClear(MEIX_Cntrl[entity]);
      }
   }

   PRN_DBG_USR_NL( MEI_DRV,MEI_DRV_PRN_LEVEL_HIGH,
         ("MEI_DRV: free dynamic data" MEI_DRV_CRLF));

   /* Free the dynamic allocated memory and memory mapping */
   for (channel_num = 0; channel_num < MEI_DFE_CHAN_DEVICES; channel_num++)
   {
      MEI_DevLineStructFree(channel_num);
   }

   PRN_DBG_USR_NL( MEI_DRV,MEI_DRV_PRN_LEVEL_HIGH,
         ("MEI_DRV: free static data" MEI_DRV_CRLF));

   for (entity = 0; entity < MEI_DFEX_ENTITIES; entity++)
   {
      MEI_DevXCntrlStructFree(entity);
   }

#ifdef CONFIG_DEVFS_FS

   PRN_DBG_USR_NL( MEI_DRV,MEI_DRV_PRN_LEVEL_HIGH,
         ("MEI_DRV: remove MEI devs" MEI_DRV_CRLF));

   /* remove MEI CPE channel devices */
   for (channel_num=0; channel_num < MEI_DFE_CHAN_DEVICES; channel_num++)
   {
      if (MEI_dev_handle[channel_num])
      {
         PRN_DBG_USR( MEI_DRV,MEI_DRV_PRN_LEVEL_HIGH, channel_num,
               ("MEI_DRV: removing device /%s/%d/" MEI_DRV_CRLF,
                DRV_MEI_NAME,
                channel_num));
         devfs_unregister (MEI_dev_handle[channel_num]);
      }
   }

   PRN_DBG_USR_NL( MEI_DRV,MEI_DRV_PRN_LEVEL_HIGH,
         ("MEI_DRV: remove control devs" MEI_DRV_CRLF));

   /* remove MEI CPE chip control devices */
   for (entity=0; entity < MEI_DFEX_ENTITIES; entity++)
   {
      if (MEI_cntrl_handle[entity])
      {
         PRN_DBG_USR_NL( MEI_DRV,MEI_DRV_PRN_LEVEL_HIGH,
               ("MEI_DRV: removing device /%s/%s%d/" MEI_DRV_CRLF,
                DRV_MEI_NAME,
                DRV_MEI_CNTRL_PREFIX,
                entity));
         devfs_unregister (MEI_cntrl_handle[entity]);
      }
   }

   PRN_DBG_USR_NL( MEI_DRV,MEI_DRV_PRN_LEVEL_HIGH,
         ("MEI_DRV: remove devfs handles" MEI_DRV_CRLF));

   /* remove chip base directory /dev/MEIX/<chip_no> from dev fs*/
   if (MEI_base_dir_handle)
   {
      PRN_DBG_USR_NL( MEI_DRV,MEI_DRV_PRN_LEVEL_HIGH,
            ("MEI_DRV: removing device dir /%s" MEI_DRV_CRLF, DRV_MEI_NAME));
      devfs_unregister (MEI_base_dir_handle);
   }

#else
   for (entity=0; entity < MEI_DFEX_ENTITIES; entity++)
   {
      mei_devt = MKDEV(major_number, entity);
      device_destroy(mei_class, mei_devt);
   }
   class_destroy(mei_class);
   mei_class = NULL;

   unregister_chrdev ( major_number , DRV_MEI_NAME );
#endif

#if CONFIG_PROC_FS

   PRN_DBG_USR_NL( MEI_DRV,MEI_DRV_PRN_LEVEL_HIGH,
         ("MEI_DRV: cleanup procfs: /%s" MEI_DRV_CRLF, DRV_MEI_NAME) );
   sprintf(path, "%s%s", "driver/", DRV_MEI_NAME);
   remove_proc_subtree(path, 0);

#endif

   PRN_DBG_USR_NL( MEI_DRV,MEI_DRV_PRN_LEVEL_NORMAL,
         ("MEI_DRV: cleanup successful" MEI_DRV_CRLF));

   if (MEI_BasicChipExit() != IFX_SUCCESS)
   {
      PRN_DBG_USR_NL( MEI_DRV,MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DRV: Chipset Basic Exit failed" MEI_DRV_CRLF));
   }

#ifdef PPA_SUPPORTS_CALLBACKS
#if (MEI_DRV_ATM_PTM_INTERFACE_ENABLE == 1)
   ppa_callback_set(LTQ_MEI_SHOWTIME_CHECK, (void *)NULL);
#endif /* #if (MEI_DRV_ATM_PTM_INTERFACE_ENABLE == 1) */
#endif /* PPA_SUPPORTS_CALLBACKS */

#if (MEI_SUPPORT_DEBUG_LOGGER == 1)
   if (nl_debug_sock)
   {
      netlink_kernel_release(nl_debug_sock);
   }
#endif

   /* touch one time this variable to avoid that the linker will remove it */
   debug_level = MEI_DRV_PRN_LEVEL_OFF;
   return;
}


/**
   Register the given device to the device node.

\param
   devName  device name

\return
   0: success
   <0 in case of errors
*/
static int MEI_InitModuleRegCharDev(const char *devName, int entity)
{
#ifdef CONFIG_DEVFS_FS
   /* ============================================
      create and setup devfs, register device
      ============================================ */

   int entity, dfe_ch;
   char buf[10];


   if (!(MEI_base_dir_handle = devfs_mk_dir(NULL, devName, NULL)))
   {
      PRN_DBG_USR_NL( MEI_DRV,MEI_DRV_PRN_LEVEL_HIGH,
            ("MEI_DRV: Unable to create %s base directory in dev_fs" MEI_DRV_CRLF,
              DRV_MEI_NAME));
      return -e_MEI_ERR_DEV_INIT;
   }

   /* add MEI CPE chip control devices to dev fs */
   sprintf (buf, "%s%d", DRV_MEI_CNTRL_PREFIX, entity);

   /* private_data contains device number for open function
      instead of minor number (dynamically assigned by dev_fs) */
   if ( ( MEI_cntrl_handle[entity] = devfs_register(
               MEI_base_dir_handle,
               buf,
               DEVFS_FL_DEFAULT /* DEVFS_FL_AUTO_DEVNUM */,
               major_number,
               entity | MEI_ENTITY_CNTRL_DEVICE,
               S_IFCHR | S_IRUGO | S_IWUGO,
               &MEI_fops,
               (void*)( 0 ) )
        ) == NULL)
   {
      PRN_DBG_USR_NL( MEI_DRV,MEI_DRV_PRN_LEVEL_HIGH,
             ("MEI_DRV: unable to add device %s/%s to dev_fs" MEI_DRV_CRLF,
              devName, buf));
      return -e_MEI_ERR_DEV_INIT;
   }

   /* create MEI CPE channel devices */
   for (dfe_ch=entity * MEI_DFE_INSTANCE_PER_ENTITY;
         dfe_ch < MEI_DFE_INSTANCE_PER_ENTITY * (entity + 1); dfe_ch++)
   {
      /* add MEI CPE channel devices to dev fs */
      sprintf (buf, "%d", dfe_ch);

      /* private_data contains device number for open function
         instead of minor number (dynamically assigned by dev_fs) */
      if ( ( MEI_dev_handle[dfe_ch] = devfs_register(
                  MEI_base_dir_handle,
                  buf,
                  DEVFS_FL_DEFAULT /* DEVFS_FL_AUTO_DEVNUM */,
                  major_number,
                  dfe_ch & MEI_CHANNEL_DEVICE_MASK,
                  S_IFCHR | S_IRUGO | S_IWUGO,
                  &MEI_fops,
                  (void*)( 0 ) )
           ) == NULL)
      {
         PRN_DBG_USR_NL( MEI_DRV,MEI_DRV_PRN_LEVEL_HIGH,
                ("MEI_DRV: unable to add device %s/%s to dev_fs" MEI_DRV_CRLF,
                 devName, buf));
         return -e_MEI_ERR_DEV_INIT;
      }
   }


#else       /* CONFIG_DEVFS_FS */
   /* ============================================
      register device
      ============================================ */

   int result;
   static dev_t mei_devt;
   if (major_number == 0)
   {
      result = register_chrdev( major_number , DRV_MEI_NAME, &MEI_fops);
      if ( result < 0 )
      {
         PRN_DBG_USR_NL( MEI_DRV,MEI_DRV_PRN_LEVEL_HIGH,
                         ("MEI_DRV: device %s - can't get major %d" MEI_DRV_CRLF,
                          devName, major_number));
         return result;
      }
      /* dynamic major                       */
      if ( major_number == 0 )
      {
         major_number = result;
         PRN_DBG_USR_NL( MEI_DRV,MEI_DRV_PRN_LEVEL_HIGH,
                         ("Using major number %d" MEI_DRV_CRLF, major_number));
      }

      /* create a device class used for createing /dev/ entries */
      mei_class = class_create(THIS_MODULE, devName);
      if (IS_ERR(mei_class)) {
         PRN_DBG_USR_NL( MEI_DRV,MEI_DRV_PRN_LEVEL_HIGH,
                         ("MEI_DRV: can not create class for %s" MEI_DRV_CRLF, devName));
         return PTR_ERR(mei_class);
      }
   }

   /* create /dev/ entry for each device */
   mei_devt = MKDEV(major_number, entity);
   device_create(mei_class, NULL, mei_devt, NULL, "%s/%i", devName, entity);

#endif      /* CONFIG_DEVFS_FS */

#if CONFIG_PROC_FS
       /* create and install proc file system */
       MEI_InstallProcEntry(entity);
#endif
    return 0;
}

/**
   Do the common init module stuff (with or without devfs).
*/
static int MEI_InitModuleBasics(void)
{
   /* reset the chip control structure */
   memset(MEIX_Cntrl, 0x00, sizeof(MEIX_Cntrl));

#if (MEI_SUPPORT_DL_DMA_CS == 1)
   /* Reset firmware image cntrl block */
   memset(&ChipFwImage, 0x00, sizeof(ChipFwImage[MEI_MAX_FW_IMAGES]));
#endif /* #if (MEI_SUPPORT_DL_DMA_CS == 1)*/

   if (MEI_DRVOS_SemaphoreInit(&pFwDlCntrlLock) != IFX_SUCCESS)
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DRV: ERROR - Init FW DL control lock" MEI_DRV_CRLF));
      return -1;
   }

   if (MEI_BasicChipInit() != IFX_SUCCESS)
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DRV: ERROR - Chipset Basic Init failed!" MEI_DRV_CRLF));
      return -1;
   }

#if (MEI_SUPPORT_DEBUG_STREAMS == 1)
   if (MEI_DBG_STREAM_ModuleInit() != IFX_SUCCESS)
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DEV_DRV[??]: ERROR - Drv Module ComInit - Init Dbg Streams" MEI_DRV_CRLF));
      return IFX_ERROR;
   }
#endif

   return 0;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0))
static int MEI_SysClkEnable(struct clk *clk)
{
   if (IS_ERR(clk))
      return -1;
   clk_enable(clk);

   return 0;
}

static int MEI_SysClkDisable(struct clk *clk)
{
   if (IS_ERR(clk))
      return -1;
   clk_disable(clk);
   clk_put(clk);

   return 0;
}
#endif

IFX_int32_t MEI_PowerUp(void)
{
#if (MEI_SUPPORT_DEVICE_VR10_320 != 1)
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0))
   /* Power up MEI */
   DSL_DFE_PMU_SETUP(IFX_PMU_ENABLE);

   if (ifx_pmu_pg_dsl_dfe_enable() != 0)
   {
      PRN_ERR_USR_NL( MEI_MEI_ACCESS, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI: ERROR - DSL DFE PG enable failed!" MEI_DRV_CRLF));

      /* ignore for VR10/VR11 (for emulator) */
      /** \todo [VRX518] Check if this check can be bind to emulation
          configure option. */
      if (!MEI_DEVICE_CFG_IS_PLATFORM(e_MEI_DEV_PLATFORM_CONFIG_VR10) &&
          !MEI_DEVICE_CFG_IS_PLATFORM(e_MEI_DEV_PLATFORM_CONFIG_VR10_320) &&
          !MEI_DEVICE_CFG_IS_PLATFORM(e_MEI_DEV_PLATFORM_CONFIG_VR11))
      {
         return IFX_ERROR;
      }
   }
#else
   if (MEI_SysClkEnable(MEI_DEVICE_CFG_VALUE_GET(clk)) != 0)
   {
      PRN_ERR_USR_NL( MEI_MEI_ACCESS, MEI_DRV_PRN_LEVEL_WRN,
            ("MEI: WARNING - dfe sys clk enable failed!" MEI_DRV_CRLF));
   }
#endif
#endif

   return IFX_SUCCESS;
}

IFX_int32_t MEI_PowerDown(void)
{
#if (MEI_SUPPORT_DEVICE_VR10_320 != 1)
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0))
   if (ifx_pmu_pg_dsl_dfe_disable() != 0)
   {
      PRN_ERR_USR_NL( MEI_MEI_ACCESS, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI: ERROR - DSL DFE PG disable failed!" MEI_DRV_CRLF));
      return IFX_ERROR;
   }

   /* Power down MEI */
   DSL_DFE_PMU_SETUP(IFX_PMU_DISABLE);
#else
   if (MEI_SysClkDisable(MEI_DEVICE_CFG_VALUE_GET(clk)) != 0)
   {
      PRN_ERR_USR_NL( MEI_MEI_ACCESS, MEI_DRV_PRN_LEVEL_WRN,
            ("MEI: WARNING - dfe sys clk disable failed!" MEI_DRV_CRLF));
   }
#endif
#endif
   return IFX_SUCCESS;
}

/**
   Init the corresponding device.

\param
   pMeiDynCntrl   - private dynamic device data (per open instance)
\param
   pInitDev       - init information data

\return
   0: success
   else < 0 in case of error

*/
IFX_int32_t MEI_IoctlInitDevice(
                              MEI_DYN_CNTRL_T     *pMeiDynCntrl,
                              IOCTL_MEI_devInit_t *pInitDev)
{
   int ret = 0, reinit = 0;
   MEI_DEV_T    *pMeiDev = pMeiDynCntrl->pMeiDev;
   unsigned long usedFlags = 0;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0))
   /* ignore DT data for VR10 (get it from pcie driver) */
   /* for VR9 irqnum & baseaddr read within dt file at probe function */
   if (MEI_DEVICE_CFG_IS_PLATFORM(e_MEI_DEV_PLATFORM_CONFIG_VR9))
   {
      pInitDev->meiBaseAddr = MEI_DEVICE_CFG_VALUE_GET(nBaseAddr);
#ifdef IRQ_POLLING_FORCE
   PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL,
      ("Force IRQ polling mode to: \"%i\", normal IRQ ignored" MEI_DRV_CRLF,
      IRQ_POLLING_FORCE));
      pInitDev->usedIRQ     = IRQ_POLLING_FORCE; /* polling mode */
#else
      pInitDev->usedIRQ     = MEI_DEVICE_CFG_VALUE_GET(nIrq);
#endif

      reinit = e_MEI_ERR_ALREADY_DONE;
   }
#endif

   PRN_DBG_USR( MEI_DRV, MEI_DRV_PRN_LEVEL_LOW, MEI_DRV_LINENUM_GET(pMeiDev),
      ("MEI_DRV[%02d]: ioctl - FIO_MEI_DEV_INIT base addr = 0x%08X, IRQ = %d"
      MEI_DRV_CRLF, MEI_DRV_LINENUM_GET(pMeiDev),
      (IFX_uint32_t)pInitDev->meiBaseAddr, (IFX_uint32_t)pInitDev->usedIRQ));

   if (! MEI_DEVICE_CFG_IS_PLATFORM(e_MEI_DEV_PLATFORM_CONFIG_VR9))
   {
      PRN_DBG_USR( MEI_DRV, MEI_DRV_PRN_LEVEL_LOW, MEI_DRV_LINENUM_GET(pMeiDev),
         ("MEI_DRV[%02d]: ioctl - FIO_MEI_DEV_INIT PDBRAM addr = 0x%08X"
         MEI_DRV_CRLF, MEI_DRV_LINENUM_GET(pMeiDev),
         (IFX_uint32_t)pInitDev->PDBRAMaddr));
   }

   MEI_MsgIntSet(pMeiDev);

   if (MEI_DFE_INSTANCE_PER_ENTITY != MEI_MAX_SUPPORTED_MEI_IF_PER_DEVICE)
   {
       /* Check for SLAVE lines (VRx related only)*/
       if (MEI_DRV_LINENUM_GET(pMeiDev) % MEI_DFE_INSTANCE_PER_ENTITY)
       {
           MEIX_CNTRL_T *pXCntrl = NULL;
           MEI_DEV_T    *pMeiDevMaster = NULL;
           int entity;

           entity = MEI_GET_ENTITY_FROM_DEVNUM(MEI_DRV_LINENUM_GET(pMeiDev));

           if ( (pXCntrl = MEIX_Cntrl[entity]) == NULL)
           {
               PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
                               ("MEI_DRV[%02d]: ERROR Line Struct Allocate - "
                                "missing MEIX[%d] entity struct" MEI_DRV_CRLF,
                                MEI_DRV_LINENUM_GET(pMeiDev), entity));

               return -e_MEI_ERR_DEV_INIT;
           }

           if ( pXCntrl->MeiDevice[0] == NULL)
           {
               PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
                               ("MEI_DRV[%02d]: MASTER line not exists yet" MEI_DRV_CRLF,
                                MEI_DRV_LINENUM_GET(pMeiDev)));

               return -e_MEI_ERR_DEV_INIT;
           }

           pMeiDevMaster = pXCntrl->MeiDevice[0];

           /* Check for the MASTER line INIT_DONE and copy config to the SLAVE line*/
           if (MEI_DRV_STATE_GET(pMeiDevMaster) == e_MEI_DRV_STATE_NOT_INIT)
           {
               PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
                               ("MEI_DRV[%02d]: MASTER line not initialized yet, please init it first"
                                MEI_DRV_CRLF, MEI_DRV_LINENUM_GET(pMeiDev)));

               return -e_MEI_ERR_DEV_INIT;
           }

           MEI_DRV_MEI_VIRT_ADDR_SET(pMeiDev, MEI_DRV_MEI_VIRT_ADDR_GET(pMeiDevMaster));

           MEI_DRV_MEI_PHY_ADDR_SET(pMeiDev, MEI_DRV_MEI_PHY_ADDR_GET(pMeiDevMaster));

           MEI_DRV_MEI_IF_STATE_SET(pMeiDev, MEI_DRV_MEI_IF_STATE_GET(pMeiDevMaster));

           if (! MEI_DEVICE_CFG_IS_PLATFORM(e_MEI_DEV_PLATFORM_CONFIG_VR9))
           {
               MEI_DRV_PDBRAM_VIRT_ADDR_SET(pMeiDev, MEI_DRV_PDBRAM_VIRT_ADDR_GET(pMeiDevMaster));

               MEI_DRV_PDBRAM_PHY_ADDR_SET(pMeiDev, MEI_DRV_PDBRAM_PHY_ADDR_GET(pMeiDevMaster));
           }

           pMeiDev->eModePoll = pMeiDevMaster->eModePoll;
           pMeiDev->intMask   = pMeiDevMaster->intMask;

           MEI_DRV_STATE_SET(pMeiDev,  MEI_DRV_STATE_GET(pMeiDevMaster));

           return ret;
       }
   }

   /*
      Do IO remap
   */
   if (MEI_DEVICE_CFG_IS_PLATFORM(e_MEI_DEV_PLATFORM_CONFIG_VR9))
   {
      if ( (ret = MEI_DRVOS_Phy2VirtMap( pInitDev->meiBaseAddr,
                                      sizeof(MEI_MEI_REG_IF_U),
                                      (IFX_char_t*)DRV_MEI_NAME,
                                      (IFX_uint8_t **)(&pMeiDev->meiDrvCntrl.pMeiIfCntrl->pVirtMeiRegIf) )
           ) != IFX_SUCCESS)
      {
         PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
             ("MEI_DRV[%02d]: ERROR - INIT DEVICE, IO remap" MEI_DRV_CRLF,
             MEI_DRV_LINENUM_GET(pMeiDev)));

         return -e_MEI_ERR_DEV_IO_MAP;
      }

      MEI_DRV_MEI_PHY_ADDR_SET(pMeiDev, pInitDev->meiBaseAddr);
   }
   else if (! MEI_DEVICE_CFG_IS_PLATFORM(e_MEI_DEV_PLATFORM_CONFIG_VR9))
   {
      MEI_DRV_MEI_PHY_ADDR_SET(pMeiDev, pInitDev->meiBaseAddr);
      MEI_DRV_PDBRAM_PHY_ADDR_SET(pMeiDev, pInitDev->PDBRAMaddr);

      /* no io-remap for VR10 and VR11 as it is already done in PCIe */
      MEI_DRV_MEI_VIRT_ADDR_SET(pMeiDev,
         (IFX_void_t *)(MEI_DRV_PCIE_VIRT_MEMBASE_GET(&pMeiDev->meiDrvCntrl) + MEI_DSL_MEI_OFFSET));
      MEI_DRV_PDBRAM_VIRT_ADDR_SET(pMeiDev,
         (IFX_void_t *)(MEI_DRV_PCIE_VIRT_MEMBASE_GET(&pMeiDev->meiDrvCntrl) + MEI_PDBRAM_OFFSET));
   }

   /* check HW access */
   if ( (ret = MEI_MeiRegisterDetect(pMeiDev)) != IFX_SUCCESS)
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
             ("MEI_DRV[%02d]: ERROR - INIT DEVICE, HW failure" MEI_DRV_CRLF,
             MEI_DRV_LINENUM_GET(pMeiDev)));

      goto MEI_IOCTL_INITDEV_BASIC_ERROR;
   }

   PRN_DBG_USR( MEI_DRV, MEI_DRV_PRN_LEVEL_LOW, MEI_DRV_LINENUM_GET(pMeiDev),
          ("MEI_DRV[%02d]: PHY2VIRT MAP, phy addr = 0x%08lX, virt addr = 0x%08X" MEI_DRV_CRLF,
            MEI_DRV_LINENUM_GET(pMeiDev), MEI_DRV_MEI_PHY_ADDR_GET(pMeiDev),
            (IFX_uint32_t)(MEI_DRV_MEI_VIRT_ADDR_GET(pMeiDev))));

   /* Shared memory section in PDBRAM is only used for VR10/VRX300 based
      platforms, but PDBRAM itself is used for all platform except VR9/VRX200 */

   if (! MEI_DEVICE_CFG_IS_PLATFORM(e_MEI_DEV_PLATFORM_CONFIG_VR9))
   {
      PRN_DBG_USR( MEI_DRV, MEI_DRV_PRN_LEVEL_LOW, MEI_DRV_LINENUM_GET(pMeiDev),
          ("MEI_DRV[%02d]: PHY2VIRT MAP, PDBRAM phy addr = 0x%08lX, PDBRAM virt addr = 0x%08lX"
           MEI_DRV_CRLF, MEI_DRV_LINENUM_GET(pMeiDev), MEI_DRV_PDBRAM_PHY_ADDR_GET(pMeiDev),
            MEI_DRV_PDBRAM_VIRT_ADDR_GET(pMeiDev)));
   }

   if (MEI_DEVICE_CFG_IS_PLATFORM(e_MEI_DEV_PLATFORM_CONFIG_VR10) ||
       MEI_DEVICE_CFG_IS_PLATFORM(e_MEI_DEV_PLATFORM_CONFIG_VR10_320))
   {
      /* Clear FORCE_LINK_DOWN flag for PPE */
      *MEI_PPE_U32REG(PPE_FORCE_LINK_DOWN) &= ~0x1;
   }

   /*
      Init device
      - MEI reset
      - mask interrupts, clear interrupts
      - get Device info
      NOTE:
         MEI Block reset and clear interrupts depends on the
         resetMode param.
   */
   if ( (ret = MEI_DefaultInitDevice(pMeiDev, 0)) < 0 )
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
             ("MEI_DRV[%02d]: ERROR - INIT DEVICE, Reset device, get device info" MEI_DRV_CRLF,
             MEI_DRV_LINENUM_GET(pMeiDev)));

      goto MEI_IOCTL_INITDEV_BASIC_ERROR;
   }

   if ( (pInitDev->usedIRQ == 0) || (pInitDev->usedIRQ == 99) )
   {
      PRN_DBG_USR( MEI_DRV, MEI_DRV_PRN_LEVEL_HIGH, MEI_DRV_LINENUM_GET(pMeiDev),
             ("MEI_DRV[%02d]: INIT DEVICE - NO INTERUPTS --> %s POLL MODE" MEI_DRV_CRLF,
             MEI_DRV_LINENUM_GET(pMeiDev), (pInitDev->usedIRQ == 0) ? "PASSIVE" : "ACTIVE" ));
   }


   switch (pInitDev->usedIRQ)
   {
      case 0:
         /* IRQ = 0: passive poll mode */
         pMeiDynCntrl->pDfeX->IRQ_Num = 0;
         pMeiDev->eModePoll = e_MEI_DEV_ACCESS_MODE_PASSIV_POLL;
         pMeiDev->intMask = 0;
         break;

      case 99:
#if (MEI_SUPPORT_PERIODIC_TASK == 1)
         /* IRQ = 99: active poll mode */
         pMeiDynCntrl->pDfeX->IRQ_Num = 99;
         pMeiDev->eModePoll = e_MEI_DEV_ACCESS_MODE_ACTIV_POLL;
         pMeiDev->intMask   = 0;

         /* check if the periodic task is already running */
         if ( MEI_DrvCntrlThreadParams.bValid == IFX_FALSE)
         {
            PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_HIGH,
                  ("MEI_DRV[%02d]: Start Driver Control Thread" MEI_DRV_CRLF,
                   MEI_DRV_LINENUM_GET(pMeiDev)));

            if (MEI_DRVOS_ThreadInit(&MEI_DrvCntrlThreadParams,
                                    "VrxCtrl",
                                    MEI_DrvCntrlThr,
                                    0, 0) != IFX_SUCCESS)
            {
               PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
                     ("MEI_DRV[%02d]: ERROR - start Driver Control Thread" MEI_DRV_CRLF,
                       MEI_DRV_LINENUM_GET(pMeiDev)));
            }
         }
         else
         {
            PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_HIGH,
                  ("MEI_DRV[%02d]: Driver Control Thread - already running" MEI_DRV_CRLF,
                    MEI_DRV_LINENUM_GET(pMeiDev)));

         }
#else
         PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
               ("MEI_DRV[%02d]: ERROR - IRQ 99 not supported without periodical task" MEI_DRV_CRLF,
                 MEI_DRV_LINENUM_GET(pMeiDev)));

         ret = -e_MEI_ERR_INVAL_BASE_CFG;
         goto MEI_IOCTL_INITDEV_BASIC_ERROR;
#endif   /* #if (MEI_SUPPORT_PERIODIC_TASK == 1) */
         break;

      default:
#if (MEI_SUPPORT_IRQ == 1)
         {
            /*
               Check if the given IRQ already in use
               - add to existing list (return NULL already registered)
               - create a new list and return the list root
            */
            MEIX_CNTRL_T *pTmpXCntrl;
            IFX_uint32_t virq;

            pMeiDev->eModePoll = e_MEI_DEV_ACCESS_MODE_IRQ;
            pMeiDev->intMask   = ME_ARC2ME_INTERRUPT_UNMASK_ALL;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,8,0)) \
      || (MEI_SUPPORT_DEVICE_VR10_320 == 1) \
      || (MEI_TARGET_x86 == 1)
            virq = (IFX_uint32_t)pInitDev->usedIRQ;
#else
            virq = irq_create_mapping(NULL, (IFX_uint32_t)pInitDev->usedIRQ);
#endif

            pTmpXCntrl = MEI_VrxXDevToIrqListAdd(
                                          MEI_DRV_LINENUM_GET(pMeiDev),
                                          virq,
                                          pMeiDynCntrl->pDfeX);
            if (pTmpXCntrl)
            {
               /* non default settings only for VR9 platform */
               if (MEI_DEVICE_CFG_IS_PLATFORM(e_MEI_DEV_PLATFORM_CONFIG_VR9))
               {
                  usedFlags = IRQ_LEVEL;
               }

               if ( (ret = MEI_IfxRequestIrq( pTmpXCntrl->IRQ_Num,
                                              MEI_InterruptLinux,
                                              MEI_InterruptThreadLinux,
                                              usedFlags,
                                              DRV_MEI_NAME,
                                              (void *)pTmpXCntrl)
                    ) < 0 )
               {
                  /* error while register ISR */
                  PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
                        ("MEI_DRV[%02d]: ERROR - INIT DEVICE, ifx_request_irq() = %d" MEI_DRV_CRLF,
                          MEI_DRV_LINENUM_GET(pMeiDev), ret));

                  ret = -e_MEI_ERR_INVAL_BASE_CFG;
                  goto MEI_IOCTL_INITDEV_BASIC_ERROR;
               }
            }
         }
#else
         PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
               ("MEI_DRV[%02d]: ERROR - MEI CPE Driver IRQ Support not enabled" MEI_DRV_CRLF,
                 MEI_DRV_LINENUM_GET(pMeiDev)));

         MEI_DRV_STATE_SET(pMeiDev, e_MEI_DRV_STATE_CFG_ERROR);
#endif
         break;
   }

   if (MEI_DRV_STATE_GET(pMeiDev) == e_MEI_DRV_STATE_CFG_ERROR)
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
      ("MEI_DRV[%02d]: ERROR - INIT DEVICE, IRQ = %d already blocked !!!" MEI_DRV_CRLF,
       MEI_DRV_LINENUM_GET(pMeiDev), pInitDev->usedIRQ));

      ret = -e_MEI_ERR_INVAL_BASE_CFG;
   }
   else
   {
      PRN_DBG_USR( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL, MEI_DRV_LINENUM_GET(pMeiDev),
             ("MEI_DRV[%02d]: INIT DEVICE, phy addr = 0x%08lX, virt addr = 0x%08X IRQ = %d" MEI_DRV_CRLF,
               MEI_DRV_LINENUM_GET(pMeiDev), MEI_DRV_MEI_PHY_ADDR_GET(pMeiDev),
               (IFX_uint32_t)(MEI_DRV_MEI_VIRT_ADDR_GET(pMeiDev)),
               (IFX_uint32_t)pInitDev->usedIRQ));

      if (! MEI_DEVICE_CFG_IS_PLATFORM(e_MEI_DEV_PLATFORM_CONFIG_VR9))
      {
         PRN_DBG_USR( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL, MEI_DRV_LINENUM_GET(pMeiDev),
             ("MEI_DRV[%02d]: INIT DEVICE, PDBRAM phy addr = 0x%08X, PDBRAM virt addr = 0x%08X"
              MEI_DRV_CRLF, MEI_DRV_LINENUM_GET(pMeiDev),
              (IFX_uint32_t)(MEI_DRV_PDBRAM_PHY_ADDR_GET(pMeiDev)),
              (IFX_uint32_t)(MEI_DRV_PDBRAM_VIRT_ADDR_GET(pMeiDev))));
      }

#if (MEI_SUPPORT_DSM == 1)
      /* Init necessary DSM data: ERB buf */
      if ((ret = MEI_VRX_DSM_ErbAlloc(pMeiDev, MEI_DSM_VECTOR_ERB_SIZE_BYTE)) < 0)
      {
         PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DRV[0x%02X]: fail to init ERB block!" MEI_DRV_CRLF,
              MEI_DRV_LINENUM_GET(pMeiDev)));

         goto MEI_IOCTL_INITDEV_BASIC_ERROR;
      }
#if (MEI_SUPPORT_DEVICE_VR11 == 1)
      MEI_VR11_ErbBarSet(pMeiDev, 0, 0);
#endif
      /* Init necessary DSM data */
      MEI_VRX_DSM_DataInit(pMeiDev);
#endif /* (MEI_SUPPORT_DSM == 1) */

      /* Init PLL offset config data */
      MEI_PLL_ConfigInit(pMeiDev);

      /*
         next state
      */
      MEI_DRV_STATE_SET(pMeiDev, e_MEI_DRV_STATE_SW_INIT_DONE);

      MEI_EnableDeviceInt(pMeiDev);

      if (pMeiDev->eModePoll == e_MEI_DEV_ACCESS_MODE_PASSIV_POLL)
      {
         MEI_PollIntPerVrxLine(pMeiDev, e_MEI_DEV_ACCESS_MODE_PASSIV_POLL);
      }
   }

   if (reinit)
   {
      ret = reinit;
   }

   return ret;

MEI_IOCTL_INITDEV_BASIC_ERROR:

   /* Do not need to unpam memory for VR10 */
   if (MEI_DEVICE_CFG_IS_PLATFORM(e_MEI_DEV_PLATFORM_CONFIG_VR9))
   {
      /* unmap memory and release memory region */
      MEI_DRVOS_Phy2VirtUnmap(
         &(MEI_DRV_MEI_PHY_ADDR_GET(pMeiDev)), sizeof(MEI_MEI_REG_IF_U),
         (IFX_uint8_t **)(&pMeiDev->meiDrvCntrl.pMeiIfCntrl->pVirtMeiRegIf));
   }

   /* reset init done */
   MEI_DRV_STATE_SET(pMeiDev, e_MEI_DRV_STATE_NOT_INIT);

   pInitDev->ictl.retCode = ret;
   return ret;
}



#if (MEI_SUPPORT_DFE_DMA_ACCESS == 1)

/**
   DMA write to the target via the MEI interface.

\param
   pMeiDynCntrl - private dynamic device data (per open instance)
\param
   pDmaArgument   - points to the DMA user information data

\return
   In case of success number of written 32 bit units
   else negative value
   - e_MEI_ERR_INVAL_ARG: max size exceeded
   - e_MEI_ERR_NOT_COMPLETE: not all units have been written.

\remark
   Under Linux a temporary buffer will be allocated for transfer data between
   user and kernel space.

*/
static int MEI_IoctlDmaAccessWr_Wrap(
                              MEI_DYN_CNTRL_T        *pMeiDynCntrl,
                              IOCTL_MEI_DMA_access_t *pDmaArgument)
{
   IFX_uint32_t *pBuf, *pUserData = (IFX_uint32_t *)pDmaArgument->pData_32;
   IFX_uint32_t ret;
   MEI_DRV_PRN_LOCAL_VAR_CREATE(MEI_DEV_T, *pMeiDev, pMeiDynCntrl->pMeiDev);

   /*
      - allocate memory and get the test pattern (max size 0x3FE (1024-2) byte)
   */

   if (pDmaArgument->count_32bit > MEI_MAX_DMA_COUNT_32BIT)
   {
      /* ERROR: invalid buffer size - max 0x3FF */
      PRN_DBG_USR( MEI_DRV, MEI_DRV_PRN_LEVEL_HIGH, MEI_DRV_LINENUM_GET(pMeiDev),
            ("MEI_DRV[0x%02X]: DMA write - invalid buffer size (max 0x%X 16bit)!" MEI_DRV_CRLF,
             MEI_DRV_LINENUM_GET(pMeiDev), MEI_MAX_DMA_COUNT_32BIT));

      return -e_MEI_ERR_INVAL_ARG;
   }

   PRN_DBG_USR( MEI_DRV, MEI_DRV_PRN_LEVEL_LOW, MEI_DRV_LINENUM_GET(pMeiDev),
         ("MEI_DRV[0x%02X]: DMA write - addr = 0x%08X, count = 0x%X (buf 0x%08X)" MEI_DRV_CRLF,
          MEI_DRV_LINENUM_GET(pMeiDev), (IFX_uint32_t)pDmaArgument->dmaAddr, (IFX_uint32_t)pDmaArgument->count_32bit,
          (IFX_uint32_t)pDmaArgument->pData_32));

   /* allocate memory */
   ret = (MEI_PARAM_COUNT_32_TO_16(pDmaArgument->count_32bit) * 2) + 3;
   pBuf = MEI_DRVOS_Malloc( (MEI_PARAM_COUNT_32_TO_16(pDmaArgument->count_32bit) * 2) + 3);
   if (!pBuf)
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DRV[0x%02X]: no memory for load DMA write data." MEI_DRV_CRLF,
              MEI_DRV_LINENUM_GET(pMeiDev)));
      return -e_MEI_ERR_NO_MEM;
   }

   /* align memory block to 32 bit, get user data */
   pDmaArgument->pData_32 = (unsigned int *)
      ( (((IFX_uint32_t)pBuf & 0x00000003)) ?
         ((IFX_uint32_t*)((IFX_uint32_t)pBuf & 0x00000003) + 1) : pBuf );

   /* copy data to kernel space */
   if ( MEI_DRVOS_CpyFromUser( pDmaArgument->pData_32, pUserData,
                        (MEI_PARAM_COUNT_32_TO_16(pDmaArgument->count_32bit) * 2) ) == IFX_NULL)
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DRV[0x%02X]: DMA write - copy_from_user(DMA data) failed!" MEI_DRV_CRLF,
             MEI_DRV_LINENUM_GET(pMeiDev)));

      /* free the mem again */
      MEI_DRVOS_Free(pBuf);
      pBuf = IFX_NULL;

      return -e_MEI_ERR_GET_ARG;
   }

   ret = MEI_IoctlDmaAccessWr( pMeiDynCntrl, pDmaArgument);

   /* free the mem, set return values */
   MEI_DRVOS_Free(pBuf);
   pBuf = IFX_NULL;

   /* Restore User Pointer*/
   pDmaArgument->pData_32 = (unsigned int*)pUserData;

   return ret;
}


/**
   DMA read from the target via the MEI interface.

\param
   pMeiDynCntrl - private dynamic device data (per open instance)
\param
   pDmaArgument   - points to the DMA user information data

\return
   Number of read 32 bit units.
   0: in case of not complete read, the number of read 32 bit units are returned
      via the argument structure.
   else negative value
   - e_MEI_ERR_INVAL_ARG: max size exceeded
   - e_MEI_ERR_OP_FAILED: read operation failed

\remark
   Under Linux a temporary buffer will be allocated for transfer data between
   user and kernel space.

*/
static int MEI_IoctlDmaAccessRd_Wrap(
                              MEI_DYN_CNTRL_T        *pMeiDynCntrl,
                              IOCTL_MEI_DMA_access_t *pDmaArgument)
{

   IFX_uint32_t   *pBuf, *pUserData = (IFX_uint32_t *)pDmaArgument->pData_32;
   IFX_uint32_t   ret;
   MEI_DRV_PRN_LOCAL_VAR_CREATE(MEI_DEV_T, *pMeiDev, pMeiDynCntrl->pMeiDev);

   /*
      - allocate memory and get the test pattern (max size 0x3FE (1024-2) byte)
   */

   if (pDmaArgument->count_32bit > MEI_MAX_DMA_COUNT_32BIT)
   {
      /* ERROR: invalid buffer size - max 0x3FF */
      PRN_DBG_USR( MEI_DRV, MEI_DRV_PRN_LEVEL_HIGH, MEI_DRV_LINENUM_GET(pMeiDev),
            ("MEI_DRV[0x%02X]: DMA read - invalid buffer size (max 0x%X 32bit)!" MEI_DRV_CRLF,
             MEI_DRV_LINENUM_GET(pMeiDev), MEI_MAX_DMA_COUNT_32BIT));

      return -e_MEI_ERR_INVAL_ARG;
   }

   /* allocate memory */
   pBuf = MEI_DRVOS_Malloc( (MEI_PARAM_COUNT_32_TO_16(pDmaArgument->count_32bit) * 2) + 3);
   if (!pBuf)
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DRV[0x%02X]: no memory for load DMA read data." MEI_DRV_CRLF,
              MEI_DRV_LINENUM_GET(pMeiDev)));
      return -e_MEI_ERR_NO_MEM;
   }

   /* align memory block to 32 bit, get user data */
   pDmaArgument->pData_32 = (unsigned int *)
      ( (((IFX_uint32_t)pBuf & 0x00000003)) ?
         ((IFX_uint32_t*)((IFX_uint32_t)pBuf & 0x00000003) + 1) : pBuf );

   /* read DMA */
   ret = MEI_IoctlDmaAccessRd(pMeiDynCntrl, pDmaArgument);

   if (ret == IFX_SUCCESS)
   {
      /* some data has been read */
      if ( copy_to_user( pUserData,
                         pDmaArgument->pData_32,
                         (MEI_PARAM_COUNT_32_TO_16(pDmaArgument->count_32bit) * 2)) )
      {
         PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
               ("MEI_DRV[%02d]: Read DMA - copy_to_user() failed!" MEI_DRV_CRLF,
                MEI_DRV_LINENUM_GET(pMeiDev)));
         /* error during copy */
         pDmaArgument->count_32bit = 0;
         ret = -e_MEI_ERR_RETURN_ARG;
      }
   }

   /* free the mem */
   MEI_DRVOS_Free(pBuf);
   pBuf = IFX_NULL;

   /* Restore User Pointer*/
   pDmaArgument->pData_32 = (unsigned int *)pUserData;

   return ret;
}

#endif      /* #if (MEI_SUPPORT_DFE_DMA_ACCESS == 1) */


#if (MEI_SUPPORT_DFE_DBG_ACCESS == 1)
/**
   Do an MEI debug write access.
*/
static int MEI_IoctlMeiDbgAccessWr_Wrap(
                              MEI_DYN_CNTRL_T       *pMeiDynCntrl,
                              IOCTL_MEI_dbgAccess_t *pUserArgument,
                              IOCTL_MEI_dbgAccess_t *pLocalArgument)
{
   IFX_uint32_t *pUserBuf = (IFX_uint32_t *)pLocalArgument->pData_32;
   int ret;

   /* Attention: dump buffer is not protected (this is only for debug) */
   pLocalArgument->pData_32 = (unsigned int *)MEI_DbgDumpBuffer;
   pLocalArgument->count = (pLocalArgument->count > MEI_IOCTL_MAX_DBG_COUNT_32BIT) ?
                            MEI_IOCTL_MAX_DBG_COUNT_32BIT : pLocalArgument->count;

   /* get the buffer */
   MEI_DRVOS_CpyFromUser( (void *)pLocalArgument->pData_32,
                   (void *)pUserBuf,
                   pLocalArgument->count * sizeof(IFX_uint32_t) ) ;

   ret =  MEI_IoctlMeiDbgAccessWr( pMeiDynCntrl, pLocalArgument);

   /* return arguments - count */
   copy_to_user( (void *)&pUserArgument->count,
                 (void *)&pLocalArgument->count,
                 sizeof(pUserArgument->count) ) ;

   return ret;
}

/**
   Do an MEI debug read access.
*/
static int MEI_IoctlMeiDbgAccessRd_Wrap(
                              MEI_DYN_CNTRL_T       *pMeiDynCntrl,
                              IOCTL_MEI_dbgAccess_t *pUserArgument,
                              IOCTL_MEI_dbgAccess_t *pLocalArgument)
{
   IFX_uint32_t *pUserBuf = (IFX_uint32_t *)pLocalArgument->pData_32;
   int ret;

   /* Attention: dump buffer is not protected (this is only for debug) */
   pLocalArgument->pData_32 = (unsigned int *)MEI_DbgDumpBuffer;
   pLocalArgument->count = (pLocalArgument->count > MEI_IOCTL_MAX_DBG_COUNT_32BIT) ?
                            MEI_IOCTL_MAX_DBG_COUNT_32BIT : pLocalArgument->count;

   ret = MEI_IoctlMeiDbgAccessRd( pMeiDynCntrl, pLocalArgument);

   /* return data of something in the buffer */
   if ( pLocalArgument->count )
   {
      /* return the buffer */
      copy_to_user( pUserBuf,
                    pLocalArgument->pData_32,
                    pLocalArgument->count * sizeof(IFX_uint32_t) ) ;

   }

   /* return count argument */
   copy_to_user( (void *)&pUserArgument->count,
                 (void *)&pLocalArgument->count,
                 sizeof(pUserArgument->count) ) ;

   return ret;
}

#endif      /* #if (MEI_SUPPORT_DFE_DBG_ACCESS == 1) */



#if (MEI_MISC_TEST == 1)

/* allocated bock size: 0x90000 --> ~ 590 000 byte */
#define MEI_MAX_VMALLOC  0x90000

/**
   Only for testing: allocate and free large memory ranges.
*/
void MEI_MemVAllocTest()
{
   char *pTemp1 = NULL, *pTemp2 = NULL;

   pTemp1 = vmalloc(MEI_MAX_VMALLOC);
   if (!pTemp1)
   {
      printk( "MEI_DRV: vmalloc test: alloc mem 1. block[0x%X] (%d) - FAILED !!!" MEI_DRV_CRLF,
              (unsigned int)MEI_MAX_VMALLOC, MEI_MAX_VMALLOC);
      return;
   }
   else
   {
      printk( "MEI_DRV: vmalloc test: alloc mem 1. block[0x%X] (%d) - SUCCESS" MEI_DRV_CRLF,
              (unsigned int)MEI_MAX_VMALLOC, MEI_MAX_VMALLOC);
   }

   pTemp2 = vmalloc(MEI_MAX_VMALLOC);
   if (!pTemp2)
   {
      printk( "MEI_DRV: vmalloc test: alloc mem 2. block[0x%X] (%d) - FAILED !!!" MEI_DRV_CRLF,
              (unsigned int)MEI_MAX_VMALLOC, MEI_MAX_VMALLOC);
      vfree(pTemp1);
      return;
   }
   else
   {
      printk( "MEI_DRV: vmalloc test: alloc mem 2. block[0x%X] (%d) - SUCCESS" MEI_DRV_CRLF,
              (unsigned int)MEI_MAX_VMALLOC, MEI_MAX_VMALLOC);
   }

   vfree(pTemp1);
   vfree(pTemp2);

   return;
}
#endif



/* ============================================================================
   Setup MEI CPE driver interrupt functions (LINUX) - wrapping
   ========================================================================= */

/**
   Set the wrapper function for the LINUX request_irq() function.

\param
   pRequestIrqFct  - points to the custumer intConnect function

\return
   0     in case of success.
   -1    if the function pointer is not valid, or if the interrupt routines
         already in use.

\remark
   The MEI_IfxRequestIrq() function locks also further updates of the
   interrupt wrapper functions.

\remark
   After the first request_irq call further updates should not be possible anymore.

*/
int MEI_FctRequestIrqSet(MEI_RequestIrq_WrapLinux_t pRequestIrqFct)
{

   if ( (MEI_IntSetupLocked != 0) || (pRequestIrqFct == NULL) )
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DRV: ERROR - setup request_irq wrap, locked = %d" MEI_DRV_CRLF,
              MEI_IntSetupLocked));

      return -1;
   }

   PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL,
         ("MEI_DRV: setup request_irq wrapper" MEI_DRV_CRLF));

   MEI_RequestIrq_WrapLx = pRequestIrqFct;

   return 0;
}


/**
   Set the wrapper function for the LINUX free_irq() function.

\param
   pRequestIrqFct  - points to the custumer intConnect function

\return
   0     in case of success.
   -1    if the function pointer is not valid, or if the interrupt routines
         already in use.

\remark
   The MEI_IfxFreeIrq() function unlocks also further updates of the
   interrupt wrapper functions.

\remark
   After the last free_irq call further updates should be possible again.

*/
int MEI_FctFreeIrqSet(MEI_FreeIrq_WrapLinux_t pFreeIrqFct)
{

   if ( (MEI_IntSetupLocked != 0) || (pFreeIrqFct == NULL) )
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DRV: ERROR - setup free_irq wrap, locked = %d" MEI_DRV_CRLF,
              MEI_IntSetupLocked));

      return -1;
   }

   PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL,
         ("MEI_DRV: setup free_irq wrapper" MEI_DRV_CRLF));

   MEI_FreeIrq_WrapLx = pFreeIrqFct;

   return 0;
}


#if (MEI_EXPORT_INTERNAL_API == 1)
IFX_int32_t MEI_InternalDevOpen(
                              IFX_uint16_t      nLine,
                              MEI_DYN_CNTRL_T **ppMeiDynCntrl)
{
   IFX_int32_t          retVal;
   MEI_DYN_CNTRL_T   *pMeiDynCntrl = NULL;

   printk ("\n++++++++++++++++++ MEI_InternalDevOpen ++++++++++++++++++\n\n");
   if ((retVal = MEI_DevLineAlloc((IFX_uint8_t)nLine)) != IFX_SUCCESS)
   {
      return retVal;
   }

   if ((retVal = MEI_InstanceLineAlloc(
                                 (IFX_uint8_t)nLine,
                                 &pMeiDynCntrl)) != IFX_SUCCESS)
   {
      return retVal;
   }

   /* return the allocated struct */
   *ppMeiDynCntrl = pMeiDynCntrl;

   /* Init VR10/VR11 device (set mei base addr, pdbram base addr, irq) */
   /* Necessary addresses provided by pcie driver */
   if (! MEI_DEVICE_CFG_IS_PLATFORM(e_MEI_DEV_PLATFORM_CONFIG_VR9))
   {
      if ((retVal = MEI_VR1x_InternalInitDevice(pMeiDynCntrl)) != IFX_SUCCESS)
      {
         return retVal;
      }
   }

   #if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
   /* increment module use counter */
   MOD_INC_USE_COUNT;
   #endif

   return IFX_SUCCESS;
}

IFX_int32_t MEI_InternalDevClose(
                              MEI_DYN_CNTRL_T *pMeiDynCntrl)
{
   MEI_DevLineClose(pMeiDynCntrl);

   #if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
   /* decrement use counter */
   MOD_DEC_USE_COUNT;
   #endif

   return IFX_SUCCESS;
}

#endif /* #if (MEI_EXPORT_INTERNAL_API == 1)*/

#if (MEI_SUPPORT_DEBUG_LOGGER == 1)
static void MEI_NlSendMsg(IFX_char_t* pMsg)
{
   struct nlmsghdr *pNlMsgHdr;
   struct sk_buff *pSkbOut;
   IFX_int_t nMsgSize = 0;

   if(pMsg == IFX_NULL)
   {
      printk("Debug string is empty!\n");
      return;
   }

   nMsgSize = strlen(pMsg);
   pSkbOut = nlmsg_new(nMsgSize, GFP_KERNEL);

   if (!pSkbOut)
   {
      printk("Failed to allocate new skb\n");
      return;
   }
   pNlMsgHdr = nlmsg_put(pSkbOut, 0, 0, NLMSG_DONE, nMsgSize, 0);
   NETLINK_CB(pSkbOut).dst_group = NL_DBG_MSG_GROUP;
   strncpy(nlmsg_data(pNlMsgHdr), pMsg, nMsgSize);

   if (!nl_debug_sock)
   {
      kfree_skb(pSkbOut);
      printk("Failed to free skb\n");
      return;
   }

   nlmsg_multicast(nl_debug_sock, pSkbOut, 0, NL_DBG_MSG_GROUP, GFP_KERNEL);
}

int MEI_debugLogSend(IFX_char_t *fmt, ...)
{
   va_list ap;   /* points to each unnamed arg in turn */
   IFX_int_t nRet = 0;
   #define MEI_DBG_MAX_DEBUG_PRINT_CHAR 256
   IFX_char_t debugString[MEI_DBG_MAX_DEBUG_PRINT_CHAR + 1] = {0};

   /* add debug string */
   va_start(ap, fmt);   /* set ap pointer to 1st unnamed arg */
   nRet = vsnprintf(&debugString[0], MEI_DBG_MAX_DEBUG_PRINT_CHAR, fmt, ap);
   va_end(ap);

   if (MEI_DbgLogger == 1)
   {
      /* send the formed string to the logger */
      MEI_NlSendMsg(debugString);
   }
   else
   {
      printk("%s", debugString);
   }

   return nRet;
}
#endif

#if (MEI_DRV_IFXOS_ENABLE == 0)

/**
   LINUX Kernel - Copy a block form driver space (kernel) TO USER space (application).

\par Implementation
   Copy data from kernel to user space by use of the kernel function "copy_to_user"

\param
   pTo         Points to the source (in driver space)
\param
   pFrom       Points to the destination (in user space)
\param
   size_byte   Block size to copy [byte]

\return
   IFX_NULL if an error occured, else pTo
*/
IFX_void_t *MEI_DRVOS_CpyToUser(
               IFX_void_t *pTo,
               const IFX_void_t *pFrom,
               IFX_uint32_t size_byte)
{
   IFX_uint32_t remainBytes = 0;

   if ((pTo == IFX_NULL) || (pFrom == IFX_NULL))
      return IFX_NULL;

   if (!size_byte)
      return IFX_NULL;

   remainBytes = (IFX_uint32_t)copy_to_user( (void *)pTo,
                                           (const void *)pFrom,
                                           (unsigned long)size_byte);

   return (remainBytes) ? IFX_NULL : pTo;
}

/**
   LINUX Kernel - Copy a block FORM USER space (application) to driver space (kernel).

\par Implementation
   Copy data from user to kernel space by use of the kernel function "copy_from_user"

\param
   pTo         Points to the source (in user space).
\param
   pFrom       Points to the destination (in driver space).
\param
   size_byte   Block size to copy [byte].

\return
   IFX_NULL if an error occured, else pTo
*/
IFX_void_t *MEI_DRVOS_CpyFromUser(
               IFX_void_t        *pTo,
               const IFX_void_t  *pFrom,
               IFX_uint32_t      size_byte)
{
   IFX_uint32_t remainBytes = 0;

   if ((pTo == IFX_NULL) || (pFrom == IFX_NULL))
      return IFX_NULL;

   if (!size_byte)
      return IFX_NULL;

   remainBytes = (IFX_uint32_t)copy_from_user( (void *)pTo,
                                                (const void *)pFrom,
                                                (unsigned long)size_byte);

   return (remainBytes) ? IFX_NULL : pTo;
}

/**
   LINUX Kernel - Map the physical address to a virtual memory space.
   For virtual memory management this is required.

\par Implementation
   - check if the given physical memory region is free (see "check_mem_region")
   - reserve the given physical memory region (see "request_mem_region")
   - map the given physical memory region - no cache (see "ioremap_nocache")

\attention
   This sequence will reserve the requested memory region, so no following user
   can remap the same area after this.
\attention
   Other users (driver) which have map the area before (without reservation)
   will still have access to the area.

\param
   physicalAddr         The physical address for mapping [I]
\param
   addrRangeSize_byte   Range of the address space to map [I]
\param
   pName                The name of the address space, for administration [I]
\param
   ppVirtAddr           Returns the pointer to the virtual mapped address [O]

\return
   IFX_SUCCESS if the mapping was successful and the ppVirtAddr is set, else
   IFX_ERROR   if something was wrong.

*/
IFX_int32_t MEI_DRVOS_Phy2VirtMap(
               IFX_ulong_t    physicalAddr,
               IFX_ulong_t    addrRangeSize_byte,
               IFX_char_t     *pName,
               IFX_uint8_t    **ppVirtAddr)
{
   IFX_uint8_t *pVirtAddr = IFX_NULL;

   if (ppVirtAddr == IFX_NULL)
      return IFX_ERROR;

   if (*ppVirtAddr != IFX_NULL)
      return IFX_ERROR;

   if (!addrRangeSize_byte)
      return IFX_ERROR;

   if ( check_mem_region(physicalAddr, addrRangeSize_byte) )
   {
      return IFX_ERROR;
   }

   /* can't fail */
   request_mem_region(physicalAddr, addrRangeSize_byte, pName);

   /* remap memory (not cache able): physical --> virtual */
   pVirtAddr = (IFX_uint8_t *)ioremap_nocache( physicalAddr,
                                               addrRangeSize_byte );
   if (pVirtAddr == IFX_NULL)
   {
      release_mem_region(physicalAddr, addrRangeSize_byte);
      return IFX_ERROR;
   }

   *ppVirtAddr = pVirtAddr;

   return IFX_SUCCESS;
}

/**
   LINUX Kernel - Release the virtual memory range of a mapped physical address.
   For virtual memory management this is required.

\par Implementation
   - unmap the given physical memory region (see "iounmap")
   - release the given physical memory region (see "release_mem_region")

\param
   pPhysicalAddr        Points to the physical address for release mapping [IO]
                        (Cleared if success)
\param
   addrRangeSize_byte   Range of the address space to map [I]
\param
   ppVirtAddr           Provides the pointer to the virtual mapped address [IO]
                        (Cleared if success)

\return
   IFX_SUCCESS if the release was successful. The physicalAddr and the ppVirtAddr
               pointer is cleared, else
   IFX_ERROR   if something was wrong.
*/
IFX_int32_t MEI_DRVOS_Phy2VirtUnmap(
               IFX_ulong_t    *pPhysicalAddr,
               IFX_ulong_t    addrRangeSize_byte,
               IFX_uint8_t    **ppVirtAddr)
{
   /* unmap the virtual address */
   if ( (ppVirtAddr != IFX_NULL) && (*ppVirtAddr != IFX_NULL) )
   {
      iounmap((void *)(*ppVirtAddr));
      *ppVirtAddr = IFX_NULL;
   }

   /* release the memory region */
   if ( (pPhysicalAddr != IFX_NULL)  && (*pPhysicalAddr != 0) )
   {
      release_mem_region( (unsigned long)(*pPhysicalAddr), addrRangeSize_byte );
      *pPhysicalAddr = 0;
   }

   return IFX_SUCCESS;
}

/**
   LINUX Kernel - Thread stub function. The stub function will be called
   before calling the user defined thread routine. This gives
   us the possibility to add checks etc.

\par Implementation
   Before the stub function enters the user thread routin the following setup will
   be done:
   - make the kernel thread to a daemon
   - asign the parent to the init process (avoid termination if the parent thread dies).
   - setup thread name, and signal handling (if required).
   After this the user thread routine will be entered.

\param
   pThrCntrl   Thread information data

\return
   - IFX_SUCCESS on success
   - IFX_ERROR on error
*/
static IFX_int32_t MEI_DRVOS_KernelThreadStartup(
                              MEI_DRVOS_ThreadCtrl_t *pThrCntrl)
{
   IFX_int32_t retVal          = IFX_ERROR;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
   struct task_struct *kthread = current;
#endif

   if(!pThrCntrl)
   {
      return retVal;
   }

   /* terminate the name if necessary */
   pThrCntrl->thrParams.pName[16 -1] = 0;

   /* do LINUX specific setup */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
   daemonize();
   reparent_to_init();

   /* lock the kernel. A new kernel thread starts without
      the big kernel lock, regardless of the lock state
      of the creator (the lock level is *not* inheritated)
   */
   lock_kernel();

   /* set signal mask to what we want to respond */
   siginitsetinv(&current->blocked, sigmask(SIGKILL)|sigmask(SIGINT)|sigmask(SIGTERM));

   /* set name of this process */
   strcpy(kthread->comm, pThrCntrl->thrParams.pName);

   /* let others run */
   unlock_kernel();
#else
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0))
   daemonize(pThrCntrl->thrParams.pName);
#endif

   /* Enable signals in Kernel >= 2.6 */
   allow_signal(SIGKILL);
   allow_signal(SIGINT);
   allow_signal(SIGTERM);
#endif


   pThrCntrl->thrParams.bRunning = IFX_TRUE;
   retVal = pThrCntrl->pThrFct(&pThrCntrl->thrParams);
   pThrCntrl->thrParams.bRunning = IFX_FALSE;

   complete_and_exit(&pThrCntrl->thrCompletion, (long)retVal);

   return retVal;
}

/**
   LINUX Kernel - Creates a new thread / task.

\par Implementation
   - Allocate and setup the internal thread control structure.
   - setup the LINUX specific thread parameter (see "init_completion").
   - start the LINUX Kernel thread with the internal stub function (see "kernel_thread")

\param
   pThrCntrl         Pointer to thread control structure. This structure has to
                     be allocated outside and will be initialized.
\param
   pName             specifies the 8-char thread / task name.
\param
   pThreadFunction   specifies the user entry function of the thread / task.
\param
   nArg1             first argument passed to thread / task entry function.
\param
   nArg2             second argument passed to thread / task entry function.

\return
   - IFX_SUCCESS thread was successful started.
   - IFX_ERROR thread was not started
*/
IFX_int32_t MEI_DRVOS_ThreadInit(
               MEI_DRVOS_ThreadCtrl_t *pThrCntrl,
               IFX_char_t *pName,
               MEI_DRVOS_ThreadFunction_t pThreadFunction,
               IFX_ulong_t nArg1,
               IFX_ulong_t nArg2)
{
   if (pThreadFunction == IFX_NULL)
      return IFX_ERROR;

   if (pName == IFX_NULL)
      return IFX_ERROR;


   if(pThrCntrl)
   {
      if (MEI_DRVOS_THREAD_INIT_VALID(pThrCntrl) == IFX_FALSE)
      {
         /* set thread function arguments */
         strcpy(pThrCntrl->thrParams.pName, pName);
         pThrCntrl->thrParams.nArg1 = nArg1;
         pThrCntrl->thrParams.nArg2 = nArg2;

         /* set thread control settings */
         pThrCntrl->pThrFct = pThreadFunction;
         init_completion(&pThrCntrl->thrCompletion);

         /* start kernel thread via the wrapper function */
         pThrCntrl->tid = kernel_thread(
                        (MEI_DRVOS_KERNEL_THREAD_StartRoutine)MEI_DRVOS_KernelThreadStartup,
                        (void *)pThrCntrl,
                        MEI_DRVOS_DRV_THREAD_OPTIONS);

         pThrCntrl->bValid = IFX_TRUE;

         return IFX_SUCCESS;
      }
      else
      {
         MEI_DRVOS_PRN_USR_ERR_NL( MEI_DRVOS, MEI_DRVOS_PRN_LEVEL_ERR,
            ("MEI_DRVOS ERROR - Kernel ThreadInit, object already valid" MEI_DRVOS_CRLF));
      }
   }
   else
   {
      MEI_DRVOS_PRN_USR_ERR_NL( MEI_DRVOS, MEI_DRVOS_PRN_LEVEL_ERR,
         ("MEI_DRVOS ERROR - Kernel ThreadInit, missing object" MEI_DRVOS_CRLF));
   }

   return IFX_ERROR;
}

/**
   LINUX Kernel - Shutdown and terminate a given thread.
   Therefore the thread delete functions triggers the user thread function
   to shutdown. In case of not response (timeout) the thread will be canceled.

\par Implementation
   - force a shutdown via the shutdown flag and wait.
   - wait for completion (see "wait_for_completion").
   - free previous allocated internal data.

\param
   pThrCntrl   Thread control struct.

\return
   - IFX_SUCCESS thread was successful deleted - thread control struct is freed.
   - IFX_ERROR thread was not deleted
*/
IFX_int32_t MEI_DRVOS_ThreadDelete(
               MEI_DRVOS_ThreadCtrl_t *pThrCntrl)
{
   IFX_uint32_t   waitCnt = 1;
   IFX_uint32_t   waitTime_ms = 0;

   if(pThrCntrl)
   {
      if (MEI_DRVOS_THREAD_INIT_VALID(pThrCntrl) == IFX_TRUE)
      {
         if (pThrCntrl->thrParams.bRunning == 1)
         {
            /* trigger user thread routine to shutdown */
            pThrCntrl->thrParams.bShutDown = IFX_TRUE;

            if (waitTime_ms != MEI_DRVOS_THREAD_DELETE_WAIT_FOREVER)
            {
               waitCnt = waitTime_ms / MEI_DRVOS_THREAD_DOWN_WAIT_POLL_MS;
            }

            while (waitCnt && (pThrCntrl->thrParams.bRunning == 1) )
            {
               MEI_DRVOS_MSecSleep(MEI_DRVOS_THREAD_DOWN_WAIT_POLL_MS);

               if (waitTime_ms != MEI_DRVOS_THREAD_DELETE_WAIT_FOREVER)
                  waitCnt--;
            }

            /* wait for thread end */
            wait_for_completion (&pThrCntrl->thrCompletion);
         }
         else
         {
            MEI_DRVOS_PRN_USR_ERR_NL( MEI_DRVOS, MEI_DRVOS_PRN_LEVEL_ERR,
               ("MEI_DRVOS WRN - Kernel Thread Delete <%s> - not running" MEI_DRVOS_CRLF,
                 pThrCntrl->thrParams.pName));
         }

         pThrCntrl->bValid = IFX_FALSE;

         if (pThrCntrl->thrParams.bRunning != 0)
         {
            MEI_DRVOS_PRN_USR_ERR_NL( MEI_DRVOS, MEI_DRVOS_PRN_LEVEL_ERR,
               ("ERROR - Kernel Thread Delete <%s> not stopped" MEI_DRVOS_CRLF,
                 pThrCntrl->thrParams.pName));

            return IFX_ERROR;
         }

         return IFX_SUCCESS;
      }
      else
      {
         MEI_DRVOS_PRN_USR_ERR_NL( MEI_DRVOS, MEI_DRVOS_PRN_LEVEL_ERR,
            ("IFXOS ERROR - Kernel ThreadDelete, invalid object" MEI_DRVOS_CRLF));
      }
   }
   else
   {
      MEI_DRVOS_PRN_USR_ERR_NL( MEI_DRVOS, MEI_DRVOS_PRN_LEVEL_ERR,
         ("MEI_DRVOS ERROR - Kernel ThreadDelete, missing object" MEI_DRVOS_CRLF));
   }

   return IFX_ERROR;
}

/**
   LINUX Kernel - Sleep a given time in [ms].

\attention
   The sleep requires a "sleep wait". "busy wait" implementation will not work.

\par Implementation
   Within the Kernel we use the LINUX scheduler to set the thread into "sleep".

\param
   sleepTime_ms   Time to sleep [ms]

\return
   None.

\remark
   Available in
   - Driver space
*/
IFX_void_t MEI_DRVOS_MSecSleep(
               IFX_time_t sleepTime_ms)
{
   /* current->state = TASK_INTERRUPTIBLE; */
   set_current_state(TASK_INTERRUPTIBLE);
   schedule_timeout(HZ * (sleepTime_ms) / 1000);

   return;
}

/**
   LINUX Kernel - Get the elapsed time in [ms].

\par Implementation
   Based on the HZ and jiffies defines we calculate the elapsed time since
   startup or based on the given ref-time.

\param
   refTime_ms  Reference time to calculate the elapsed time in [ms].

\return
   Elapsed time in [ms] based on the given reference time

\remark
   Provide refTime_ms = 0 to get the current elapsed time. For messurement provide
   the current time as reference.
*/
IFX_time_t MEI_DRVOS_ElapsedTimeMSecGet(
               IFX_time_t refTime_ms)
{
   IFX_time_t currTime_ms = 0;

   currTime_ms = (IFX_time_t)(jiffies * 1000 / HZ);

   return (currTime_ms > refTime_ms) ? (currTime_ms - refTime_ms)
                                     : (refTime_ms - currTime_ms);
}

#endif /* #if (MEI_DRV_IFXOS_ENABLE == 0)*/

struct platform_device* own_devices[2] = {NULL};

static IFX_int32_t MEI_DRVOS_RegisterPciDevices(IFX_uint32_t vendor, IFX_uint32_t device, int platform)
{

    IFX_int32_t count = 0;
    struct pci_dev *vrx_dev = NULL;
    vrx_dev = pci_get_device(vendor, device, vrx_dev);
    while (vrx_dev != NULL && count < 2)
    {
        count++;
        own_devices[count-1] = platform_device_register_data(&vrx_dev->dev, "mei_cpe", PLATFORM_DEVID_AUTO, &platform, sizeof(platform));
        vrx_dev = pci_get_device(vendor, device, vrx_dev);

    }
    return count;
}

static IFX_void_t MEI_DRVOS_RemoveOwnPciPlatformDevices(void)
{
   IFX_int32_t i = 0;

   for (i = 0; i < 2 ; i++)
   {
      if (own_devices[i] != NULL)
      {
         platform_device_del(own_devices[i]);
      }
   }
}


static int MEI_module_init (void)
{
   printk(KERN_INFO "%s, (c) 2007-2016 Lantiq Deutschland GmbH\n", &MEI_WHATVERSION[4]);

#ifdef CONFIG_DEVFS_FS
   /* clear devfs structures */
   memset (&(MEI_cntrl_handle), 0x00, sizeof (MEI_cntrl_handle));
   memset (&(MEI_dev_handle), 0x00, sizeof(MEI_dev_handle));
#endif

   /** VRX518 **/
#ifdef CONFIG_OF
   dev_registered = MEI_DRVOS_RegisterPciDevices(0x8086, 0x09a9, e_MEI_DEV_PLATFORM_CONFIG_VR11);
   if (dev_registered == 0)
   {
       /** VRX318 **/ /**TODO: review this mapping **/
       dev_registered = MEI_DRVOS_RegisterPciDevices(0x1bef, 0x100, e_MEI_DEV_PLATFORM_CONFIG_VR10_320);
   }

   if (dev_registered == 0)
   {
       /** VRX318 **/
       dev_registered = MEI_DRVOS_RegisterPciDevices(0x1bef, 0x020, e_MEI_DEV_PLATFORM_CONFIG_VR10);
   }
#endif
   if (dev_registered == 0)
   {
       /** Device tree based probe - number of device according to DT **/
       printk(KERN_INFO "PCI VRX device not found, relying on probe\n");
   }
   else
   {
       printk(KERN_INFO "Found %d PCI VRX devices, \n", dev_registered);
       /** disable DT based probe **/
       ltq_dsl_cpe_mei_driver.driver.of_match_table = NULL;
   }
   MEI_DEVICE_CFG_VALUE_SET(MaxDeviceNumber, dev_registered);
   MEI_DEVICE_CFG_VALUE_SET(LinesPerDevice, 1);
   MEI_DEVICE_CFG_VALUE_SET(ChannelsPerLine, 1);
   MEI_DEVICE_CFG_VALUE_SET(DfeChanDevices, dev_registered);

   return platform_driver_register(&ltq_dsl_cpe_mei_driver);
}


static void MEI_module_exit(void)
{
    MEI_driver_exit();
    platform_driver_unregister(&ltq_dsl_cpe_mei_driver);
    MEI_DRVOS_RemoveOwnPciPlatformDevices();
}


#if (MEI_EXPORT_INTERNAL_API == 1)
/**
   export the MEI CPE ioctl internal interface to the kernel namespace
*/
EXPORT_SYMBOL(MEI_InternalDevOpen);
EXPORT_SYMBOL(MEI_InternalDevClose);
EXPORT_SYMBOL(MEI_InternalDrvVersionGet);
EXPORT_SYMBOL(MEI_InternalDebugLevelSet);
EXPORT_SYMBOL(MEI_InternalInitDevice);
EXPORT_SYMBOL(MEI_InternalDevReset);
EXPORT_SYMBOL(MEI_InternalRequestConfig);

#if (MEI_SUPPORT_VDSL2_ADSL_SWAP == 1)
EXPORT_SYMBOL(MEI_InternalDevCfgFwModeSwap);
#endif

#if (MEI_SUPPORT_STATISTICS == 1)
EXPORT_SYMBOL(MEI_InternalRequestStat);
#endif

#if (MEI_SUPPORT_REGISTER == 1)
EXPORT_SYMBOL(MEI_InternalSetRegister);
EXPORT_SYMBOL(MEI_InternalGetRegister);
#endif

EXPORT_SYMBOL(MEI_InternalFirmwareDownload);
EXPORT_SYMBOL(MEI_InternalOptFirmwareDownload);
EXPORT_SYMBOL(MEI_InternalFwModeCtrlSet);
EXPORT_SYMBOL(MEI_InternalFwModeStatGet);

#if (MEI_SUPPORT_DFE_GPA_ACCESS == 1)
EXPORT_SYMBOL(MEI_InternalGpaWrAccess);
EXPORT_SYMBOL(MEI_InternalGpaRdAccess);
#endif

#if (MEI_SUPPORT_DFE_DBG_ACCESS == 1)
EXPORT_SYMBOL(MEI_InternalMeiDbgAccessWr);
EXPORT_SYMBOL(MEI_InternalMeiDbgAccessRd);
#endif

EXPORT_SYMBOL(MEI_InternalNfcCallBackDataSet);
EXPORT_SYMBOL(MEI_InternalNfcEnable);
EXPORT_SYMBOL(MEI_InternalNfcDisable);
EXPORT_SYMBOL(MEI_InternalAutoMsgCtlSet);
EXPORT_SYMBOL(MEI_InternalAutoMsgCtlGet);

EXPORT_SYMBOL(MEI_InternalCmdMsgWrite);
EXPORT_SYMBOL(MEI_InternalAckMsgRead);
EXPORT_SYMBOL(MEI_InternalMsgSend);
EXPORT_SYMBOL(MEI_InternalNfcMsgRead);

#if (MEI_DRV_ATM_OAM_ENABLE == 1)
EXPORT_SYMBOL(MEI_InternalAtmOamInit);
EXPORT_SYMBOL(MEI_InternalAtmOamCntrl);
EXPORT_SYMBOL(MEI_InternalAtmOamCounterGet);
EXPORT_SYMBOL(MEI_InternalAtmOamStatusGet);
EXPORT_SYMBOL(MEI_InternalAtmOamCellInsert);
#endif

#if (MEI_DRV_CLEAR_EOC_ENABLE == 1)
EXPORT_SYMBOL(MEI_InternalCEocInit);
EXPORT_SYMBOL(MEI_InternalCEocCntrl);
EXPORT_SYMBOL(MEI_InternalCEocStats);
EXPORT_SYMBOL(MEI_InternalCEocFrameWr);
EXPORT_SYMBOL(MEI_InternalCEocFrameRd);
#endif

#endif      /* #if (MEI_EXPORT_INTERNAL_API == 1) */

/*
   Export the functions to the system
*/
EXPORT_SYMBOL(MEI_FctRequestIrqSet);
EXPORT_SYMBOL(MEI_FctFreeIrqSet);

/*
   register module init and exit
*/
module_init (MEI_module_init);
module_exit (MEI_module_exit);

/****************************************************************************/
#ifdef MODULE
MODULE_AUTHOR("www.lantiq.com");
MODULE_DESCRIPTION("MEI CPE Driver - www.lantiq.com");
MODULE_SUPPORTED_DEVICE("MEI CPE Interface");
MODULE_LICENSE ("GPL");
#endif /* #ifdef MODULE*/

#endif /* LINUX */

