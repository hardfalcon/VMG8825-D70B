/******************************************************************************

                            Copyright (c) 2006-2015
                        Lantiq Beteiligungs-GmbH & Co.KG
                             http://www.lantiq.com

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/**
   \file drv_mps_vmmc_linux.c  Header file of the MPS driver Linux part.
   This file contains the implementation of the linux specific driver functions.
*/

/* ============================= */
/* Includes                      */
/* ============================= */
#include "drv_config.h"

#include "drv_mps_version.h"

#ifdef VMMC_WITH_MPS
   #include "drv_api.h"
#endif

#ifdef LINUX

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/init.h>
#include <linux/poll.h>
#include <linux/vmalloc.h>
#include <linux/ioctl.h>
#include <linux/delay.h>
#include <linux/interrupt.h>

#ifdef VMMC_FEAT_LINUX_PLATFORM_DRIVER
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <linux/clk.h>
#endif /* VMMC_FEAT_LINUX_PLATFORM_DRIVER */

#ifdef CONFIG_PROC_FS
   #include <linux/proc_fs.h>
   #include <linux/seq_file.h>
#endif /* CONFIG_PROC_FS */

/* include for UTS_RELEASE */
#ifndef UTS_RELEASE
   #if   (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
      #include <linux/uts.h>
   #elif (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,33))
      #include <linux/utsrelease.h>
   #else
      #include <generated/utsrelease.h>
   #endif /* LINUX_VERSION_CODE */
#endif /* UTS_RELEASE */

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
#include <linux/cdev.h>
#else
#include <linux/moduleparam.h>
#endif /* LINUX_VERSION_CODE */

#if defined(SYSTEM_FALCON)
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,28))
#include "drv_vmmc_init.h"
#include <lantiq.h>
#include <irq.h>
#ifdef CONFIG_SOC_LANTIQ_FALCON
#include <falcon_irq.h>
#endif
#else
#include <asm/ifx/irq.h>
#include <asm/ifx/ifx_regs.h>
#include <asm/ifx_vpe.h>
#endif
#endif /* SYSTEM_FALCON */

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,5,0))
#define irq_create_mapping(DOMAIN, NUMBER) NUMBER
#endif

/* lib_ifxos headers */
#include "ifx_types.h"
#include "ifxos_lock.h"
#include "ifxos_select.h"
#include "ifxos_copy_user_space.h"
#include "ifxos_event.h"
#include "ifxos_interrupt.h"

#include "drv_mps_vmmc.h"
#include "drv_mps_vmmc_dbg.h"
#include "drv_mps_vmmc_device.h"

/* ============================= */
/* Local Macros & Definitions    */
/* ============================= */
#define IFX_MPS_DEV_NAME       "ltq_mps"

#ifndef CONFIG_MPS_HISTORY_SIZE
#define CONFIG_MPS_HISTORY_SIZE 128
#warning CONFIG_MPS_HISTORY_SIZE should have been set via cofigure - setting to default 128
#endif

/* first minor number */
#define LQ_MPS_FIRST_MINOR 1
/* total file descriptor number */
#define LQ_MPS_TOTAL_FD    16

#ifdef CONFIG_PROC_FS
   #if (LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0))
      #define PDE_DATA(inode) PDE(inode)->data
   #endif
#endif /* CONFIG_PROC_FS */

/* ============================= */
/* Global variable definition    */
/* ============================= */
CREATE_TRACE_GROUP (MPS);
volatile IFX_uint32_t *cpu1_base_addr = IFX_NULL;
#ifdef VMMC_FEAT_LINUX_PLATFORM_DRIVER
struct clk *clk = NULL;
#endif /* VMMC_FEAT_LINUX_PLATFORM_DRIVER */

/* ============================= */
/* Global function declaration   */
/* ============================= */
extern irqreturn_t ifx_mps_ad0_irq (IFX_int32_t irq, IFX_void_t * pDev);
extern IFX_void_t ifx_mps_shutdown (void);
extern IFX_int32_t ifx_mps_event_activation_poll (mps_devices type,
                                                  MbxEventRegs_s * act);
mps_mbx_dev *ifx_mps_get_device (mps_devices type);
extern IFX_int32_t ifx_mps_bufman_get_level (void);

extern IFX_int32_t ifx_mps_hw_enable(void);
extern IFX_void_t ifx_mps_hw_disable(void);

extern int lq_mps_slic_reset_gpio;

/* ============================= */
/* Local function declaration    */
/* ============================= */
#ifndef __KERNEL__
IFX_int32_t ifx_mps_open (struct inode *inode, struct file *file_p);
IFX_int32_t ifx_mps_close (struct inode *inode, struct file *file_p);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36))
IFX_int32_t ifx_mps_ioctl (struct inode *inode, struct file *file_p,
#else
long ifx_mps_ioctl (struct file *file_p,
#endif
                           IFX_uint32_t nCmd, IFX_ulong_t arg);
IFX_int32_t ifx_mps_read_mailbox (mps_devices type, mps_message * rw);
IFX_int32_t ifx_mps_write_mailbox (mps_devices type, mps_message * rw);
IFX_int32_t ifx_mps_register_data_callback (mps_devices type, IFX_uint32_t dir,
                                            IFX_void_t (*callback) (mps_devices
                                                                    type));
IFX_int32_t ifx_mps_unregister_data_callback (mps_devices type,
                                              IFX_uint32_t dir);
IFX_int32_t ifx_mps_register_event_callback (mps_devices type,
                                             MbxEventRegs_s * mask,
                                             IFX_void_t (*callback)
                                             (MbxEventRegs_s * events));
IFX_int32_t ifx_mps_unregister_event_callback (mps_devices type);
#endif  /*__KERNEL__*/
IFX_int32_t ifx_mps_register_event_poll (mps_devices type,
                                         MbxEventRegs_s * mask,
                                         IFX_void_t (*callback) (MbxEventRegs_s
                                                                 * events));
IFX_int32_t ifx_mps_unregister_event_poll (mps_devices type);
static IFX_uint32_t ifx_mps_poll (struct file *file_p, poll_table * wait);

IFX_int32_t ifx_mps_event_mbx_activation_poll (IFX_int32_t value);

/* ============================= */
/* Local variable definition     */
/* ============================= */

/* ============================= */
/* Local function definition     */
/* ============================= */

/* external function declaration */

/* local function declaration */

#if (defined(MODULE) && !defined(VMMC_WITH_MPS))
MODULE_AUTHOR ("Lantiq Deutschland GmbH");
MODULE_DESCRIPTION ("MPS driver for XWAY(TM) XRX100 family, XWAY(TM) XRX200 family, XWAY(TM) XRX300 family");
MODULE_SUPPORTED_DEVICE ("XWAY(TM) XRX100 family, XRX200 family, XRX300 family MIPS34KEc");
MODULE_LICENSE ("Dual BSD/GPL");
#endif /* defined(MODULE) && !defined(VMMC_WITH_MPS) */

static ushort ifx_mps_major_id = 0;
module_param (ifx_mps_major_id, ushort, 0);
MODULE_PARM_DESC (ifx_mps_major_id, "Major ID of device");
IFX_char_t ifx_mps_dev_name[10];
IFX_char_t voice_channel_int_name[NUM_VOICE_CHANNEL][15];

/* the driver callbacks */
static struct file_operations ifx_mps_fops = {
 owner:THIS_MODULE,
 poll:ifx_mps_poll,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36))
 ioctl:ifx_mps_ioctl,
#else
 unlocked_ioctl:ifx_mps_ioctl,
#endif /* LINUX < 2.6.36 */
 open:ifx_mps_open,
 release:ifx_mps_close
};


/* device structure */
extern mps_comm_dev ifx_mps_dev;
#if CONFIG_MPS_HISTORY_SIZE > 0
#define MPS_HISTORY_BUFFER_SIZE (CONFIG_MPS_HISTORY_SIZE)
#endif

#ifdef CONFIG_PROC_FS
static struct proc_dir_entry *ifx_mps_proc_dir;

#if CONFIG_MPS_HISTORY_SIZE > 0
#ifdef DEBUG
extern IFX_int32_t ifx_mps_history_buffer_words_total;
#endif /* DEBUG */
#endif /* CONFIG_MPS_HISTORY_SIZE > 0 */
#endif /* CONFIG_PROC_FS */

extern IFX_int32_t ifx_mps_history_buffer_freeze;
extern IFX_uint32_t ifx_mps_history_buffer[];
extern IFX_int32_t ifx_mps_history_buffer_words;
extern IFX_int32_t ifx_mps_history_buffer_overflowed;

static IFX_char_t ifx_mps_device_version[20];

/* FW ready event */
IFXOS_event_t fw_ready_evt;

/**
   This function registers char device in kernel.
\param   pDev     pointer to mps_comm_dev structure
\return  0        success
\return  -ENOMEM
\return  -EPERM
*/
IFX_int32_t lq_mps_os_register (mps_comm_dev *pDev)
{
   IFX_int32_t ret;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
   IFX_uint8_t i, idx, minor;
   dev_t       dev;

   if (ifx_mps_major_id)
   {
      dev = MKDEV(ifx_mps_major_id, LQ_MPS_FIRST_MINOR);
      ret = register_chrdev_region(dev, LQ_MPS_TOTAL_FD, ifx_mps_dev_name);
   }
   else
   {
      /* dynamic major */
      ret = alloc_chrdev_region(&dev, LQ_MPS_FIRST_MINOR, LQ_MPS_TOTAL_FD, ifx_mps_dev_name);
      ifx_mps_major_id = MAJOR(dev);
   }
#else
   /* older way of char driver registration */
   ret = register_chrdev (ifx_mps_major_id, ifx_mps_dev_name, &ifx_mps_fops);
   if (ret >= 0 && ifx_mps_major_id == 0)
   {
      /* dynamic major */
      ifx_mps_major_id = ret;
   }
#endif
   if (ret < 0)
   {
      TRACE (MPS, DBG_LEVEL_HIGH,
             ("IFX_MPS: can't get major %d\n", ifx_mps_major_id));
      return ret;
   }
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
   for (i=0, idx=0, minor=LQ_MPS_FIRST_MINOR; i<LQ_MPS_TOTAL_FD; i++, minor++)
   {
      struct cdev *p_cdev = cdev_alloc();

      if (NULL == p_cdev)
         return -ENOMEM;

      cdev_init(p_cdev, &ifx_mps_fops);
      p_cdev->owner = THIS_MODULE;

      ret = cdev_add(p_cdev, MKDEV(ifx_mps_major_id, minor), 1);
      if (ret != 0)
      {
         cdev_del (p_cdev);
         return -EPERM;
      }

      if (minor == LQ_MPS_FIRST_MINOR)
      {
         pDev->command_mb.mps_cdev = p_cdev;
      }
      else if (minor == LQ_MPS_TOTAL_FD)
      {
         pDev->event_mbx.mps_cdev = p_cdev;
      }
      else if (minor > LQ_MPS_FIRST_MINOR && minor < LQ_MPS_TOTAL_FD)
      {
         pDev->voice_mb[idx].mps_cdev = p_cdev;
         idx++;
      }
   }
#endif
   return ret;
}

/**
   This function unregisters char device from kernel.
\param   pDev     pointer to mps_comm_dev structure
*/
IFX_void_t lq_mps_os_unregister (mps_comm_dev *pDev)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
   IFX_uint8_t i, idx, minor;

   for (i=0, idx=0, minor=LQ_MPS_FIRST_MINOR; i<LQ_MPS_TOTAL_FD; i++, minor++)
   {
      if (minor == LQ_MPS_FIRST_MINOR)
      {
         cdev_del (pDev->command_mb.mps_cdev);
         pDev->command_mb.mps_cdev = IFX_NULL;
      }
      else if (minor == LQ_MPS_TOTAL_FD)
      {
         cdev_del (pDev->event_mbx.mps_cdev);
         pDev->event_mbx.mps_cdev = IFX_NULL;
      }
      else if (minor > LQ_MPS_FIRST_MINOR && minor < LQ_MPS_TOTAL_FD)
      {
         cdev_del (pDev->voice_mb[idx].mps_cdev);
         pDev->voice_mb[idx].mps_cdev = IFX_NULL;
         idx++;
      }
   }
   unregister_chrdev_region (MKDEV(ifx_mps_major_id, LQ_MPS_FIRST_MINOR), LQ_MPS_TOTAL_FD);
#else
   /* older way of char driver deregistration */
   unregister_chrdev (ifx_mps_major_id, ifx_mps_dev_name);
#endif
}

/**
 * Get mailbox struct by type
 * This function returns the mailbox device structure pointer for the given
 * device.
 *
 * \param   type     DSP device entity
 * \ingroup Internal
 */
mps_mbx_dev *ifx_mps_get_device (mps_devices type)
{
   /* Get corresponding mailbox device structure */
   switch (type)
   {
      case command:
         return (&ifx_mps_dev.command_mb);
      case event_mbx:
         return (&ifx_mps_dev.event_mbx);
      default:
         if (E_MPS_DEVICE_TYPE_IS_VOICE(type))
            {return &ifx_mps_dev.voice_mb[MPS_DEVICE_VOICE_TYPE_2_CHANNEL(type)];}
         else
            {return IFX_NULL;}
   }
}


/**
 * Open MPS device.
 * Open the device from user mode (e.g. application) or kernel mode.
 * For kernel access use NULL for file_p.
 *
 * \param   inode   Pointer to device inode
 * \param   file_p  Pointer to file descriptor
 * \return  0       IFX_SUCCESS, device opened
 * \return  EMFILE  Device already open
 * \return  EINVAL  Invalid minor ID
 * \ingroup API
 */
IFX_int32_t ifx_mps_open (struct inode * inode, struct file * file_p)
{
   mps_comm_dev *pDev = &ifx_mps_dev;
   mps_mbx_dev *pMBDev;
   IFX_int32_t bcommand = 2;
   IFX_int32_t from_kernel = 0;
   mps_devices num;

   /* Determine the channel number. */
   if (file_p == NULL)
   {
      /* If the pointer to the filestruct is NULL this is a call from
         kernel space. In this case the inode has a different meaning. */
      from_kernel = 1;
      num = (IFX_int32_t) inode;
   }
   else
   {
      /* This is an open of the user space filedescriptor. */
      num = (mps_devices) MINOR (inode->i_rdev);        /* the real device */
   }

   /* check the device number */
   switch (num)
   {
      case command:
         pMBDev = &(pDev->command_mb);
         bcommand = 1;
         break;
      case event_mbx:
         pMBDev = &pDev->event_mbx;
         bcommand = 3;
         break;
      default:
         if (E_MPS_DEVICE_TYPE_IS_VOICE(num))
         {
            pMBDev = &(pDev->voice_mb[MPS_DEVICE_VOICE_TYPE_2_CHANNEL(num)]);
         }
         else
         {
            TRACE (MPS, DBG_LEVEL_HIGH,
                   ("IFX_MPS ERROR: max. device number exceed!\n"));
            return -EINVAL;
         }
   }

   if ((IFX_SUCCESS) ==
       ifx_mps_common_open (pDev, pMBDev, bcommand, from_kernel))
   {
      if (!from_kernel)
      {
         /* installation was successfull */
         /* and use file_p->private_data to point to the device data */
         file_p->private_data = pMBDev;
#ifdef MODULE
         /* increment module use counter */
         /* MOD_INC_USE_COUNT; */
#endif /* */
      }
      return 0;
   }
   else
   {
      /* installation failed */
      TRACE (MPS, DBG_LEVEL_HIGH,
             ("IFX_MPS ERROR: Device %d is already open!\n", num));
      return -EMFILE;
   }
}


/**
 * Close MPS device.
 * Close the device from user mode (e.g. application) or kernel mode.
 * For kernel access use NULL for file_p.
 *
 * \param   inode   Pointer to device inode
 * \param   file_p  Pointer to file descriptor for NULL for kernel space open.
 * \return  0       IFX_SUCCESS, device closed
 * \return  ENODEV  Device invalid / Invalid minor ID
 * \ingroup API
 */
IFX_int32_t ifx_mps_close (struct inode * inode, struct file * file_p)
{
   mps_mbx_dev *pMBDev;
   mps_devices num;

   /* Find the mailbox structure and for tracing the channel number. */
   if (file_p == NULL)
   {
      /* If the pointer to the filestruct is NULL this is a call from
         kernel space. In this case the inode has a different meaning. */

      /* Get corresponding mailbox device structure */
      pMBDev = ifx_mps_get_device ((mps_devices)((IFX_int32_t) inode));
      num = (IFX_int32_t) inode;
   }
   else
   {
      /* This is an open of the user space filedescriptor. */
      pMBDev = file_p->private_data;
      num = (mps_devices) MINOR (inode->i_rdev);        /* the real device */
   }

   if (IFX_NULL != pMBDev)
   {
      /* device is still available */
      if (ifx_mps_common_close (pMBDev, /*dummy*/IFX_FALSE) != IFX_SUCCESS)
      {
         TRACE (MPS, DBG_LEVEL_HIGH,
                ("IFX_MPS ERROR: Device %d is not open!\n", num));
         return -ENODEV;
      }

      return 0;
   }
   else
   {
      /* something went totally wrong */
      TRACE (MPS, DBG_LEVEL_HIGH, ("IFX_MPS ERROR: pMBDev pointer is NULL!\n"));
      return -ENODEV;
   }
}


/**
 * Poll handler.
 * The select function of the driver. A user space program may sleep until
 * the driver wakes it up.
 *
 * \param   file_p  File structure of device
 * \param   wait    Internal table of poll wait queues
 * \return  mask    If new data is available the POLLPRI bit is set,
 *                  triggering an exception indication. If the device pointer
 *                  is null POLLERR is set.
 * \ingroup API
 */
static IFX_uint32_t ifx_mps_poll (struct file *file_p, poll_table * wait)
{
   mps_mbx_dev *pMBDev = file_p->private_data;
   IFX_uint32_t mask;

   /* add to poll queue */
   IFXOS_DrvSelectQueueAddTask ((IFXOS_drvSelectOSArg_t *) file_p,
                                &(pMBDev->mps_wakeuplist),
                                (IFXOS_drvSelectTable_t *) wait);

   mask = 0;

   /* upstream queue */
   if (*pMBDev->upstrm_fifo->pwrite_off != *pMBDev->upstrm_fifo->pread_off)
   {
      if (pMBDev->devID == event_mbx)
      {
         mask = POLLPRI;
      }
      else
      {
         mask = POLLIN | POLLRDNORM;
      }
   }
   /* no downstream queue in case of event mailbox */
   if (pMBDev->dwstrm_fifo == IFX_NULL)
      return mask;

   /* downstream queue */
   if (ifx_mps_fifo_mem_available (pMBDev->dwstrm_fifo) != 0)
   {
      /* queue is not full */
      mask |= POLLOUT | POLLWRNORM;
   }

   if (ifx_mps_dev.event.MPS_Ad0Reg.val & pMBDev->event_mask.MPS_Ad0Reg.val)
   {
      mask |= POLLPRI;
   }

   return mask;
}


/**
 * MPS IOCTL handler.
 * An inode value of 0..7 indicates a kernel mode access. In such a case the
 * inode value is used as minor ID.
 * The following IOCTLs are supported for the MPS device.
 * - #FIO_MPS_EVENT_REG
 * - #FIO_MPS_EVENT_UNREG
 * - #FIO_MPS_MB_READ
 * - #FIO_MPS_MB_WRITE
 * - #FIO_MPS_DOWNLOAD
 * - #FIO_MPS_GETVERSION
 * - #FIO_MPS_MB_RST_QUEUE
 * - #FIO_MPS_RESET
 * - #FIO_MPS_RESTART
 * - #FIO_MPS_GET_STATUS
 *
 * If MPS_FIFO_BLOCKING_WRITE is defined the following commands are also
 * available.
 * - #FIO_MPS_TXFIFO_SET
 * - #FIO_MPS_TXFIFO_GET
 *
 * \param   inode        Inode of device
 * \param   file_p       File structure of device
 * \param   nCmd         IOCTL command
 * \param   arg          Argument for some IOCTL commands
 * \return  0            Setting the LED bits was successfull
 * \return  -EINVAL      Invalid minor ID
 * \return  -ENOIOCTLCMD Invalid command
 * \ingroup API
 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36))
IFX_int32_t ifx_mps_ioctl (struct inode * inode, struct file * file_p,
                           IFX_uint32_t nCmd, IFX_ulong_t arg)
#else
long ifx_mps_ioctl (struct file *file_p,
                           IFX_uint32_t nCmd, IFX_ulong_t arg)
#endif /* LINUX < 2.6.36 */
{
   IFX_int32_t retvalue = -EINVAL;
   mps_message rw_struct;
   mps_mbx_dev *pMBDev;
#if CONFIG_MPS_HISTORY_SIZE > 0
   mps_history cmd_history;
#endif /* */
   IFX_int32_t from_kernel = 0;

   /* a trick: VMMC driver passes the first parameter as a value of
      'mps_devices' enum type, which in fact is [0..8]; So, if inode value is
      [0..NUM_VOICE_CHANNEL+1], then we make sure that we are calling from
      kernel space. */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36))
   if (((IFX_int32_t) inode >= 0) &&
       ((IFX_int32_t) inode < NUM_VOICE_CHANNEL + 1))
#else
   if (((IFX_int32_t) file_p >= 0) &&
       ((IFX_int32_t) file_p < NUM_VOICE_CHANNEL + 1))
#endif
   {
      from_kernel = 1;

      /* Get corresponding mailbox device structure */
      if ((pMBDev =
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36))
           ifx_mps_get_device ((mps_devices) ((IFX_int32_t) inode))) == 0)
#else
           ifx_mps_get_device ((mps_devices) ((IFX_int32_t) file_p))) == 0)
#endif
      {
         return (-EINVAL);
      }
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36))
      inode = NULL;
#else
      file_p = NULL;
#endif
   }
   else
   {
      pMBDev = file_p->private_data;
   }

   switch (nCmd)
   {
      case FIO_MPS_EVENT_REG:
         {
            MbxEventRegs_s events;
            if (IFX_NULL ==
                IFXOS_CpyFromUser (&events, (IFX_void_t *) arg,
                                   sizeof (MbxEventRegs_s)))
            {
               TRACE (MPS, DBG_LEVEL_HIGH,
                      (KERN_ERR "[%s %s %d]: copy_from_user error\r\n",
                       __FILE__, __func__, __LINE__));
            }
            retvalue =
               ifx_mps_register_event_poll (pMBDev->devID, &events, IFX_NULL);
            if (retvalue == IFX_SUCCESS)
            {
               retvalue =
                  ifx_mps_event_activation_poll (pMBDev->devID, &events);
            }
            break;
         }
      case FIO_MPS_EVENT_UNREG:
         {
            MbxEventRegs_s events;
            events.MPS_Ad0Reg.val = 0;
            ifx_mps_event_activation_poll (pMBDev->devID, &events);
            retvalue = ifx_mps_unregister_event_poll (pMBDev->devID);
            break;
         }
      case FIO_MPS_MB_READ:
         /* Read the data from mailbox stored in local FIFO */
         if (from_kernel)
         {
            retvalue = ifx_mps_mbx_read (pMBDev, (mps_message *) arg, 0);
         }
         else
         {
            IFX_uint32_t *pUserBuf;

            /* Initialize destination and copy mps_message from usermode */
            memset (&rw_struct, 0, sizeof (mps_message));
            if (IFX_NULL ==
                IFXOS_CpyFromUser (&rw_struct, (IFX_void_t *) arg,
                                   sizeof (mps_message)))
            {
               TRACE (MPS, DBG_LEVEL_HIGH,
                      (KERN_ERR "[%s %s %d]: copy_from_user error\r\n",
                       __FILE__, __func__, __LINE__));
            }
            pUserBuf = (IFX_uint32_t *) rw_struct.pData;        /* Remember
                                                                   usermode
                                                                   buffer */

            /* read data from upstream mailbox FIFO */
            retvalue = ifx_mps_mbx_read (pMBDev, &rw_struct, 0);
            if (retvalue != IFX_SUCCESS)
               return -ENOMSG;

            /* Copy data to usermode buffer... */
            if (IFX_NULL ==
                IFXOS_CpyToUser (pUserBuf, rw_struct.pData,
                                 rw_struct.nDataBytes))
            {
               TRACE (MPS, DBG_LEVEL_HIGH,
                      (KERN_ERR "[%s %s %d]: copy_to_user error\r\n", __FILE__,
                       __func__, __LINE__));
            }
            ifx_mps_bufman_free (rw_struct.pData);

            /* ... and finally restore the buffer pointer and copy mps_message
               back! */
            rw_struct.pData = (IFX_uint8_t *) pUserBuf;
            if (IFX_NULL ==
                IFXOS_CpyToUser ((IFX_void_t *) arg, &rw_struct,
                                 sizeof (mps_message)))
            {
               TRACE (MPS, DBG_LEVEL_HIGH,
                      (KERN_ERR "[%s %s %d]: copy_to_user error\r\n", __FILE__,
                       __func__, __LINE__));
            }
         }
         break;
      case FIO_MPS_MB_WRITE:
         /* Write data to send to the mailbox into the local FIFO */
         if (from_kernel)
         {
            if (pMBDev->devID == command)
            {
               return (ifx_mps_mbx_write_cmd (pMBDev, (mps_message *) arg));
            }
            else
            {
               return (ifx_mps_mbx_write_data (pMBDev, (mps_message *) arg));
            }
         }
         else
         {
            IFX_uint32_t *pUserBuf;
            if (IFX_NULL ==
                IFXOS_CpyFromUser (&rw_struct, (IFX_void_t *) arg,
                                   sizeof (mps_message)))
            {
               TRACE (MPS, DBG_LEVEL_HIGH,
                      (KERN_ERR "[%s %s %d]: copy_from_user error\r\n",
                       __FILE__, __func__, __LINE__));
            }

            /* Remember usermode buffer */
            pUserBuf = (IFX_uint32_t *) rw_struct.pData;

            /* Allocate kernelmode buffer for writing data */
            rw_struct.pData =
               ifx_mps_bufman_malloc (rw_struct.nDataBytes, 0x10);

            /* rw_struct.pData = ifx_mps_bufman_malloc(rw_struct.nDataBytes,
               GFP_KERNEL); */
            if (rw_struct.pData == IFX_NULL)
            {
               return (-ENOMEM);
            }

            /* copy data to kernelmode buffer and write to mailbox FIFO */
            if (IFX_NULL ==
                IFXOS_CpyFromUser (rw_struct.pData, pUserBuf,
                                   rw_struct.nDataBytes))
            {
               TRACE (MPS, DBG_LEVEL_HIGH,
                      (KERN_ERR "[%s %s %d]: copy_from_user error\r\n",
                       __FILE__, __func__, __LINE__));
            }
            if (pMBDev->devID == command)
            {
               retvalue = ifx_mps_mbx_write_cmd (pMBDev, &rw_struct);
               ifx_mps_bufman_free (rw_struct.pData);
            }
            else
            {
               if ((retvalue =
                    ifx_mps_mbx_write_data (pMBDev, &rw_struct)) != IFX_SUCCESS)
                  ifx_mps_bufman_free (rw_struct.pData);
            }

            /* ... and finally restore the buffer pointer and copy mps_message
               back! */
            rw_struct.pData = (IFX_uint8_t *) pUserBuf;
            if (IFX_NULL ==
                IFXOS_CpyToUser ((IFX_void_t *) arg, &rw_struct,
                                 sizeof (mps_message)))
            {
               TRACE (MPS, DBG_LEVEL_HIGH,
                      (KERN_ERR "[%s %s %d]: copy_to_user error\r\n", __FILE__,
                       __func__, __LINE__));
            }
         }
         break;
      case FIO_MPS_DOWNLOAD:
         {
            /* Download firmware file */
            if (pMBDev->devID == command)
            {
               mps_fw dwnld_struct;

               if (from_kernel)
               {
                  dwnld_struct.data = ((mps_fw *) arg)->data;
                  dwnld_struct.length = ((mps_fw *) arg)->length;
               }
               else
               {
                  if (IFX_NULL ==
                      IFXOS_CpyFromUser (&dwnld_struct, (IFX_void_t *) arg,
                                         sizeof (mps_fw)))
                  {
                     TRACE (MPS, DBG_LEVEL_HIGH,
                            (KERN_ERR "[%s %s %d]: copy_from_user error\r\n",
                             __FILE__, __func__, __LINE__));
                  }
               }

               retvalue = ifx_mps_download_firmware (pMBDev, &dwnld_struct);

               if (IFX_SUCCESS != retvalue)
               {
                  TRACE (MPS, DBG_LEVEL_HIGH,
                         (KERN_ERR "IFX_MPS: firmware download error (%i)!\n",
                          retvalue));
               }
               else
                  retvalue = ifx_mps_bufman_init ();
            }
            else
            {
               retvalue = -EINVAL;
            }
            break;
         }                      /* FIO_MPS_DOWNLOAD */
      case FIO_MPS_GETVERSION:
         if (from_kernel)
         {
            memcpy ((IFX_char_t *) arg, (IFX_char_t *) ifx_mps_device_version,
                    strlen (ifx_mps_device_version));
         }
         else
         {
            if (IFX_NULL ==
                IFXOS_CpyToUser ((IFX_void_t *) arg, ifx_mps_device_version,
                                 strlen (ifx_mps_device_version)))
            {
               TRACE (MPS, DBG_LEVEL_HIGH,
                      (KERN_ERR "[%s %s %d]: copy_to_user error\r\n", __FILE__,
                       __func__, __LINE__));
            }
         }
         retvalue = IFX_SUCCESS;
         break;
      case FIO_MPS_RESET:
         /* Reset of the DSP */
         ifx_mps_reset ();
         break;
      case FIO_MPS_RESTART:
         /* Restart of the DSP */
         if (!from_kernel)
         {
            TRACE (MPS, DBG_LEVEL_HIGH, ("IFX_MPS: Restarting firmware..."));
         }
         retvalue = ifx_mps_restart ();
         if (retvalue == IFX_SUCCESS)
         {
            if (!from_kernel)
               ifx_mps_get_fw_version (1);
            retvalue = ifx_mps_bufman_init ();
         }
         break;
#ifdef MPS_FIFO_BLOCKING_WRITE
      case FIO_MPS_TXFIFO_SET:
         /* Set the mailbox TX FIFO blocking mode */
         if (pMBDev->devID == command)
         {
            retvalue = -EINVAL; /* not supported for this command MB */
         }
         else
         {
            if (arg > 0)
            {
               pMBDev->bBlockWriteMB = IFX_TRUE;
            }
            else
            {
               pMBDev->bBlockWriteMB = IFX_FALSE;
               Sem_Unlock (pMBDev->sem_write_fifo);
            }
            retvalue = IFX_SUCCESS;
         }
         break;
      case FIO_MPS_TXFIFO_GET:
         /* Get the mailbox TX FIFO to blocking */
         if (pMBDev->devID == command)
         {
            retvalue = -EINVAL;
         }
         else
         {
            if (!from_kernel)
            {
               if (IFX_NULL ==
                   IFXOS_CpyToUser ((IFX_void_t *) arg, &pMBDev->bBlockWriteMB,
                                    sizeof (bool_t)))
               {
                  TRACE (MPS, DBG_LEVEL_HIGH,
                         (KERN_ERR "[%s %s %d]: copy_to_user error\r\n",
                          __FILE__, __func__, __LINE__));
               }
            }
            retvalue = IFX_SUCCESS;
         }
         break;
#endif /* MPS_FIFO_BLOCKING_WRITE */
      case FIO_MPS_GET_STATUS:
         {
            IFXOS_INTSTAT flags;

            /* get the status of the channel */
            if (!from_kernel)
            {
               if (IFX_NULL ==
                   IFXOS_CpyToUser ((IFX_void_t *) arg, &ifx_mps_dev.event,
                                    sizeof (MbxEventRegs_s)))
               {
                  TRACE (MPS, DBG_LEVEL_HIGH,
                         (KERN_ERR "[%s %s %d]: copy_to_user error\r\n",
                          __FILE__, __func__, __LINE__));
               }
            }
            IFXOS_LOCKINT (flags);
            ifx_mps_dev.event.MPS_Ad0Reg.val &=
               ~pMBDev->event_mask.MPS_Ad0Reg.val;
            IFXOS_UNLOCKINT (flags);
            retvalue = IFX_SUCCESS;
            break;
         }
#if CONFIG_MPS_HISTORY_SIZE > 0
      case FIO_MPS_GET_CMD_HISTORY:
         {
            IFXOS_INTSTAT flags;

            if (from_kernel)
            {

               /* TODO */
            }
            else
            {
               IFX_uint32_t *pUserBuf;
               IFX_uint32_t begin;

               /* Initialize destination and copy mps_message from usermode */
               memset (&cmd_history, 0, sizeof (mps_history));
               if (IFX_NULL ==
                   IFXOS_CpyFromUser (&cmd_history, (IFX_void_t *) arg,
                                      sizeof (mps_history)))
               {
                  TRACE (MPS, DBG_LEVEL_HIGH,
                         (KERN_ERR "[%s %s %d]: copy_from_user error\r\n",
                          __FILE__, __func__, __LINE__));
               }
               if (cmd_history.len < MPS_HISTORY_BUFFER_SIZE)
                  return -ENOBUFS;      /* not enough buffer space */
               pUserBuf = cmd_history.buf;      /* Remember usermode buffer */
               IFXOS_LOCKINT (flags);
               if (ifx_mps_history_buffer_overflowed == 0)
               {
                  cmd_history.len = ifx_mps_history_buffer_words;
                  IFXOS_UNLOCKINT (flags);
                  /* Copy data to usermode buffer... */
                  if (IFX_NULL ==
                      IFXOS_CpyToUser (pUserBuf, ifx_mps_history_buffer,
                                       cmd_history.len * 4))
                  {
                     TRACE (MPS, DBG_LEVEL_HIGH,
                            (KERN_ERR "[%s %s %d]: copy_to_user error\r\n",
                             __FILE__, __func__, __LINE__));
                  }
                  IFXOS_LOCKINT (flags);
               }
               else
               {
                  cmd_history.len = MPS_HISTORY_BUFFER_SIZE;
                  begin =
                     ifx_mps_history_buffer_words % MPS_HISTORY_BUFFER_SIZE;
                  IFXOS_UNLOCKINT (flags);
                  /* Copy data to usermode buffer... */
                  if (IFX_NULL ==
                      IFXOS_CpyToUser (pUserBuf,
                                       (&ifx_mps_history_buffer[begin]),
                                       (MPS_HISTORY_BUFFER_SIZE - begin) * 4))
                  {
                     TRACE (MPS, DBG_LEVEL_HIGH,
                            (KERN_ERR "[%s %s %d]: copy_to_user error\r\n",
                             __FILE__, __func__, __LINE__));
                  }
                  if (IFX_NULL ==
                      IFXOS_CpyToUser ((&pUserBuf
                                        [MPS_HISTORY_BUFFER_SIZE - begin]),
                                       (&ifx_mps_history_buffer[0]), begin * 4))
                  {
                     TRACE (MPS, DBG_LEVEL_HIGH,
                            (KERN_ERR "[%s %s %d]: copy_to_user error\r\n",
                             __FILE__, __func__, __LINE__));
                  }
                  IFXOS_LOCKINT (flags);
               }
               cmd_history.total_words = ifx_mps_history_buffer_words;
               cmd_history.freeze = ifx_mps_history_buffer_freeze;
               if (ifx_mps_history_buffer_freeze)
               {
                  /* restart history logging */
                  ifx_mps_history_buffer_freeze = 0;
                  ifx_mps_history_buffer_words = 0;
                  ifx_mps_history_buffer_overflowed = 0;
               }

               IFXOS_UNLOCKINT (flags);

               /* ... and finally restore the buffer pointer and copy
                  cmd_history back! */
               if (IFX_NULL ==
                   IFXOS_CpyToUser ((IFX_void_t *) arg, &cmd_history,
                                    sizeof (mps_history)))
               {
                  TRACE (MPS, DBG_LEVEL_HIGH,
                         (KERN_ERR "[%s %s %d]: copy_to_user error\r\n",
                          __FILE__, __func__, __LINE__));
               }
            }
            retvalue = IFX_SUCCESS;
            break;
         }
#endif /* CONFIG_MPS_HISTORY_SIZE > 0 */
      case FIO_MPS_EVENT_MBX_REG:
         {
            retvalue = ifx_mps_event_mbx_activation_poll (1);
            break;
         }
      case FIO_MPS_EVENT_MBX_UNREG:
         {
            retvalue = ifx_mps_event_mbx_activation_poll (0);
            break;
         }
      default:
         {
            TRACE (MPS, DBG_LEVEL_HIGH,
                   ("IFX_MPS_Ioctl: Invalid IOCTL handle %d passed.\n", nCmd));
            retvalue = -ENOIOCTLCMD;
            break;
         }
   }
   return retvalue;
}


/**
 * Register data callback.
 * Allows the upper layer to register a callback function either for
 * downstream (tranmsit mailbox space available) or for upstream (read data
 * available)
 *
 * \param   type     DSP device entity ( 1 - command, 2 - voice0, 3 - voice1,
 *                   4 - voice2, 5 - voice3, 6 - voice4 )
 * \param   dir      Direction (1 - upstream, 2 - downstream)
 * \param   callback Callback function to register
 * \return  0        IFX_SUCCESS, callback registered successfully
 * \return  ENXIO    Wrong DSP device entity (only 1-5 supported)
 * \return  EBUSY    Callback already registered
 * \return  EINVAL   Callback parameter null
 * \ingroup API
 */
IFX_int32_t ifx_mps_register_data_callback (mps_devices type, IFX_uint32_t dir,
                                            IFX_void_t (*callback) (mps_devices
                                                                    type))
{
   mps_mbx_dev *pMBDev;

   if (callback == IFX_NULL)
   {
      return (-EINVAL);
   }

   /* Get corresponding mailbox device structure */
   if ((pMBDev = ifx_mps_get_device (type)) == 0)
      return (-ENXIO);

   /* Enter the desired callback function */
   switch (dir)
   {
      case 1:                  /* register upstream callback function */
         if (pMBDev->up_callback != IFX_NULL)
            {return (-EBUSY);}
         else
            {pMBDev->up_callback = callback;}
         break;
      case 2:                  /* register downstream callback function */
         if (pMBDev->down_callback != IFX_NULL)
            {return (-EBUSY);}
         else
            {pMBDev->down_callback = callback;}
         break;
      default:
         break;
   }

   return (IFX_SUCCESS);
}


/**
 * Unregister data callback.
 * Allows the upper layer to unregister the callback function previously
 * registered.
 *
 * \param   type   DSP device entity ( 1 - command, 2 - voice0, 3 - voice1,
 *                 4 - voice2, 5 - voice3, 6 - voice4)
 * \param   dir    Direction (1 - upstream, 2 - downstream)
 * \return  0      IFX_SUCCESS, callback registered successfully
 * \return  ENXIO  Wrong DSP device entity (only 1-5 supported)
 * \return  EINVAL Nothing to unregister
 * \return  EINVAL Callback value null
 * \ingroup API
 */
IFX_int32_t ifx_mps_unregister_data_callback (mps_devices type,
                                              IFX_uint32_t dir)
{
   mps_mbx_dev *pMBDev;

   /* Get corresponding mailbox device structure */
   if ((pMBDev = ifx_mps_get_device (type)) == 0)
      return (-ENXIO);

   /* Delete the desired callback function */
   switch (dir)
   {
      case 1:
         if (pMBDev->up_callback == IFX_NULL)
            {return (-EINVAL);}
         else
            {pMBDev->up_callback = IFX_NULL;}
         break;
      case 2:
         if (pMBDev->down_callback == IFX_NULL)
            {return (-EINVAL);}
         else
            {pMBDev->down_callback = IFX_NULL;}
         break;
      default:
         {
            TRACE (MPS, DBG_LEVEL_HIGH,
                   ("%s: Invalid Direction %d\n", __FUNCTION__, dir));
            return (-ENXIO);
         }
   }

   return (IFX_SUCCESS);
}


/**
 * Register event callback.
 * Allows the upper layer to register a callback function either for events
 * specified by the mask parameter.
 *
 * \param   type     DSP device entity ( 1 - command, 2 - voice0, 3 - voice1,
 *                   4 - voice2, 5 - voice3, 6 - voice4 )
 * \param   mask     Mask according to MBC_ISR content
 * \param   callback Callback function to register
 * \return  0        IFX_SUCCESS, callback registered successfully
 * \return  ENXIO    Wrong DSP device entity (only 1-5 supported)
 * \return  EBUSY    Callback already registered
 * \ingroup API
 */
IFX_int32_t ifx_mps_register_event_poll (mps_devices type,
                                         MbxEventRegs_s * mask,
                                         IFX_void_t (*callback) (MbxEventRegs_s
                                                                 * events))
{
   mps_mbx_dev *pMBDev;

   callback = callback;

   /* Get corresponding mailbox device structure */
   if ((pMBDev = ifx_mps_get_device (type)) == 0)
      return (-ENXIO);
   memcpy ((IFX_char_t *) & pMBDev->event_mask, (IFX_char_t *) mask,
           sizeof (MbxEventRegs_s));
   return (IFX_SUCCESS);
}


/**
 * Unregister event callback.
 * Allows the upper layer to unregister the callback function previously
 * registered.
 *
 * \param   type   DSP device entity ( 1 - command, 2 - voice0, 3 - voice1,
 *                 4 - voice2, 5 - voice3, 6 - voice4 )
 * \return  0      IFX_SUCCESS, callback registered successfully
 * \return  ENXIO  Wrong DSP device entity (only 1-5 supported)
 * \ingroup API
 */
IFX_int32_t ifx_mps_unregister_event_poll (mps_devices type)
{
   mps_mbx_dev *pMBDev;

   /* Get corresponding mailbox device structure */
   if ((pMBDev = ifx_mps_get_device (type)) == 0)
      return (-ENXIO);

   /* Delete the desired callback function */
   memset ((IFX_char_t *) & pMBDev->event_mask, 0, sizeof (MbxEventRegs_s));
   return (IFX_SUCCESS);
}


/**
 * Register event callback.
 * Allows the upper layer to register a callback function either for events
 * specified by the mask parameter.
 *
 * \param   type     DSP device entity ( 1 - command, 2 - voice0, 3 - voice1,
 *                   4 - voice2, 5 - voice3, 6 - voice4 )
 * \param   mask     Mask according to MBC_ISR content
 * \param   callback Callback function to register
 * \return  0        IFX_SUCCESS, callback registered successfully
 * \return  ENXIO    Wrong DSP device entity (only 1-5 supported)
 * \return  EBUSY    Callback already registered
 * \ingroup API
 */
IFX_int32_t ifx_mps_register_event_callback (mps_devices type,
                                             MbxEventRegs_s * mask,
                                             IFX_void_t (*callback)
                                             (MbxEventRegs_s * events))
{
   mps_mbx_dev *pMBDev;

   /* Get corresponding mailbox device structure */
   if ((pMBDev = ifx_mps_get_device (type)) == 0)
      return (-ENXIO);

   /* Enter the desired callback function */
   if (pMBDev->event_callback != IFX_NULL)
   {
      return (-EBUSY);
   }
   else
   {
      memcpy ((IFX_char_t *) & pMBDev->callback_event_mask, (IFX_char_t *) mask,
              sizeof (MbxEventRegs_s));
      pMBDev->event_callback = callback;
   }
   return (IFX_SUCCESS);
}


/**
 * Unregister event callback.
 * Allows the upper layer to unregister the callback function previously
 * registered.
 *
 * \param   type   DSP device entity ( 1 - command, 2 - voice0, 3 - voice1,
 *                 4 - voice2, 5 - voice3, 6 - voice4 )
 * \return  0      IFX_SUCCESS, callback registered successfully
 * \return  ENXIO  Wrong DSP device entity (only 1-5 supported)
 * \ingroup API
 */
IFX_int32_t ifx_mps_unregister_event_callback (mps_devices type)
{
   mps_mbx_dev *pMBDev;

   /* Get corresponding mailbox device structure */
   if ((pMBDev = ifx_mps_get_device (type)) == 0)
      return (-ENXIO);

   /* Delete the desired callback function */
   memset ((IFX_char_t *) & pMBDev->callback_event_mask, 0,
           sizeof (MbxEventRegs_s));
   pMBDev->event_callback = IFX_NULL;
   return (IFX_SUCCESS);
}


/**
 * Register event mailbox callback.
 * Allows the upper layer to register a callback function for events in the
 * specified by the mask parameter.
 *
 * \param   callback Callback function to register
 * \return  0        IFX_SUCCESS, callback registered successfully
 * \return  EBUSY    Callback already registered
 * \ingroup API
 */
IFX_int32_t ifx_mps_register_event_mbx_callback (IFX_uint32_t pDev,
                                                 IFX_void_t (*callback)
                                                 (IFX_uint32_t pDev,
                                                  mps_event_msg * msg))
{
   mps_mbx_dev *pMBDev;

   /* Get corresponding mailbox device structure */
   pMBDev = ifx_mps_get_device (event_mbx);

   /* Enter the desired callback function */
   if (pMBDev->event_mbx_callback != IFX_NULL)
   {
      return (-EBUSY);
   }
   else
   {
      pMBDev->event_callback_handle = pDev;
      pMBDev->event_mbx_callback = callback;
   }

   return (ifx_mps_event_mbx_activation_poll (1));
}


/**
 * Unregister event mailbox callback.
 * Allows the upper layer to unregister the callback function previously
 * registered.
 *
 * \return  0      IFX_SUCCESS, callback registered successfully
 * \ingroup API
 */
IFX_int32_t ifx_mps_unregister_event_mbx_callback (void)
{
   mps_mbx_dev *pMBDev;

   ifx_mps_event_mbx_activation_poll (0);
   /* Get corresponding mailbox device structure */
   pMBDev = ifx_mps_get_device (event_mbx);
   pMBDev->event_callback_handle = 0;
   pMBDev->event_mbx_callback = IFX_NULL;
   return (IFX_SUCCESS);
}

/**
 * Read from mailbox upstream FIFO.
 * This function reads from the mailbox upstream FIFO selected by type.
 *
 * \param   type  DSP device entity ( 1 - command, 2 - voice0, 3 - voice1,
 *                4 - voice2, 5 - voice3 )
 * \param   rw    Pointer to message structure for received data
 * \return  0     IFX_SUCCESS, successful read operation
 * \return  ENXIO Wrong DSP device entity (only 1-5 supported)
 * \return  -1    ERROR, in case of read error.
 * \ingroup API
 */
IFX_int32_t ifx_mps_read_mailbox (mps_devices type, mps_message * rw)
{
   IFX_int32_t ret;

   switch (type)
   {
      case command:
         ret = ifx_mps_mbx_read (&ifx_mps_dev.command_mb, rw, 0);
         break;
      default:
         if (E_MPS_DEVICE_TYPE_IS_VOICE(type))
         {
            ret = ifx_mps_mbx_read (&ifx_mps_dev.voice_mb[MPS_DEVICE_VOICE_TYPE_2_CHANNEL(type)], rw, 0);
         }
         else
         {
            ret = -ENXIO;
         }
   }
   return (ret);
}


/**
 * Write to downstream mailbox buffer.
 * This function writes data to either the command or to the voice FIFO
 *
 * \param   type  DSP device entity ( 1 - command, 2 - voice0, 3 - voice1,
 *                4 - voice2, 5 - voice3 )
 * \param   rw    Pointer to message structure
 * \return  0       IFX_SUCCESS, successful write operation
 * \return  -ENXIO  Wrong DSP device entity (only 1-5 supported)
 * \return  -EAGAIN ERROR, in case of FIFO overflow.
 * \ingroup API
 */
IFX_int32_t ifx_mps_write_mailbox (mps_devices type, mps_message * rw)
{
   IFX_int32_t ret;

   switch (type)
   {
      case command:
         ret = ifx_mps_mbx_write_cmd (&ifx_mps_dev.command_mb, rw);
         break;
      default:
         if (E_MPS_DEVICE_TYPE_IS_VOICE(type))
         {
            ret = ifx_mps_mbx_write_data (&ifx_mps_dev.voice_mb[MPS_DEVICE_VOICE_TYPE_2_CHANNEL(type)], rw);
         }
         else
         {
            ret = -ENXIO;
         }
   }

   return (ret);
}


#ifdef CONFIG_PROC_FS
/**
 * Create MPS version proc file output.
 * This function creates the output for the MPS version proc file
 *
 * \param   s        Pointer to seq_file struct.
 * \return  0 on success
 * \ingroup Internal
 */
static int ifx_mps_get_version_proc (struct seq_file *s)
{
   seq_printf(s, "%s%s\n", IFX_MPS_INFO_STR, ifx_mps_device_version);
   seq_printf(s, "Compiled on %s, %s for Linux kernel %s\n",
               __DATE__, __TIME__, UTS_RELEASE);

   return 0;
}


/**
 * Create MPS status proc file output.
 * This function creates the output for the MPS status proc file
 *
 * \param   s        Pointer to seq_file struct.
 * \return  0 on success
 * \ingroup Internal
 */
IFX_int32_t ifx_mps_get_status_proc (struct seq_file *s)
{
   IFX_int32_t i;

   seq_printf (s, "Status registers:\n");
   seq_printf (s, "   AD0ENR = 0x%08x\n", *IFX_MPS_AD0ENR);
   seq_printf (s, "   RAD0SR = 0x%08x\n", *IFX_MPS_RAD0SR);

   seq_printf (s, "\n   Buffers held by FW: %u\n",
               ifx_mps_bufman_get_level());

   /* Print internals of the command mailbox fifo */
   seq_printf (s, "\n * CMD *\t\tUP\t\tDO\t(%s, %s)\n",
               MPS_MBX_DEV_INSTALL_INST_ISSET(&ifx_mps_dev.command_mb) ? "active" : "idle",
               MPS_MBX_DEV_INSTALL_INST_ISSET(&ifx_mps_dev.command_mb) ?
                  (MPS_MBX_DEV_INSTALL_RECONFIG_ISSET(&ifx_mps_dev.command_mb) ? "cfg-pending" : "cfg-valid") : "-");
   seq_printf (s, "   Size: \t  %8d\t  %8d\n",
               ifx_mps_dev.cmd_upstrm_fifo.size,
               ifx_mps_dev.cmd_dwstrm_fifo.size);
   seq_printf (s, "   Fill: \t  %8d\t  %8d\n",
               ifx_mps_dev.cmd_upstrm_fifo.size - 1 -
               ifx_mps_fifo_mem_available (&ifx_mps_dev.cmd_upstrm_fifo),
               ifx_mps_dev.cmd_dwstrm_fifo.size - 1 -
               ifx_mps_fifo_mem_available (&ifx_mps_dev.cmd_dwstrm_fifo));
   seq_printf (s, "   Free: \t  %8d\t  %8d\n",
               ifx_mps_fifo_mem_available (&ifx_mps_dev.cmd_upstrm_fifo),
               ifx_mps_fifo_mem_available (&ifx_mps_dev.cmd_dwstrm_fifo));
   seq_printf (s, "   Pkts: \t  %8d\t  %8d\n",
               ifx_mps_dev.cmd_upstrm_fifo.pkts,
               ifx_mps_dev.cmd_dwstrm_fifo.pkts);
   seq_printf (s, "   Bytes:\t  %8d\t  %8d\n",
               ifx_mps_dev.cmd_upstrm_fifo.bytes,
               ifx_mps_dev.cmd_dwstrm_fifo.bytes);

   seq_printf (s, "\n * VOICE *\t\tUP\t\tDO\n");
   seq_printf (s, "   Size: \t  %8d\t  %8d\n",
               ifx_mps_dev.voice_upstrm_fifo.size,
               ifx_mps_dev.voice_dwstrm_fifo.size);
   seq_printf (s, "   Fill: \t  %8d\t  %8d\n",
               ifx_mps_dev.voice_upstrm_fifo.size - 1 -
               ifx_mps_fifo_mem_available (&ifx_mps_dev.voice_upstrm_fifo),
               ifx_mps_dev.voice_dwstrm_fifo.size - 1 -
               ifx_mps_fifo_mem_available (&ifx_mps_dev.voice_dwstrm_fifo));
   seq_printf (s, "   Free: \t  %8d\t  %8d\n",
               ifx_mps_fifo_mem_available (&ifx_mps_dev.voice_upstrm_fifo),
               ifx_mps_fifo_mem_available (&ifx_mps_dev.voice_dwstrm_fifo));
   seq_printf (s, "   Pkts: \t  %8d\t  %8d\n",
               ifx_mps_dev.voice_upstrm_fifo.pkts,
               ifx_mps_dev.voice_dwstrm_fifo.pkts);
   seq_printf (s, "   Bytes: \t  %8d\t  %8d\n",
               ifx_mps_dev.voice_upstrm_fifo.bytes,
               ifx_mps_dev.voice_dwstrm_fifo.bytes);
   seq_printf (s, "   Discd: \t  %8d\n",
               ifx_mps_dev.voice_upstrm_fifo.discards);
   seq_printf (s, "   minLv: \t  %8d\t  %8d\n",
               ifx_mps_dev.voice_upstrm_fifo.min_space,
               ifx_mps_dev.voice_dwstrm_fifo.min_space);

   for (i = 0; i < NUM_VOICE_CHANNEL; i++)
   {
      seq_printf (s, "\n * CH%i *\t\tUP\t\tDO\t%s\n", i,
                  MPS_MBX_DEV_INSTALL_INST_ISSET(&ifx_mps_dev.voice_mb[i]) ? "(idle)" : "(active)");
      seq_printf (s, "   Size: \t  %8d\n",
                  ifx_mps_dev.voice_mb[i].upstrm_fifo->size);
      seq_printf (s, "   Fill: \t  %8d\n",
                  ifx_mps_dev.voice_mb[i].upstrm_fifo->size - 1 -
                  ifx_mps_fifo_mem_available (ifx_mps_dev.voice_mb[i].
                                              upstrm_fifo));
      seq_printf (s, "   Free: \t  %8d\n",
                  ifx_mps_fifo_mem_available (ifx_mps_dev.voice_mb[i].
                                              upstrm_fifo));
      seq_printf (s, "   Pkts: \t  %8d\n",
                  ifx_mps_dev.voice_mb[i].upstrm_fifo->pkts);
      seq_printf (s, "   Bytes: \t  %8d\n",
                  ifx_mps_dev.voice_mb[i].upstrm_fifo->bytes);
      seq_printf (s, "   Discd: \t  %8d\n",
                  ifx_mps_dev.voice_mb[i].upstrm_fifo->discards);
      seq_printf (s, "   minLv: \t  %8d\n",
                  ifx_mps_dev.voice_mb[i].upstrm_fifo->min_space);
   }

   return 0;
}


/**
 * Create MPS mailbox proc file output.
 * This function creates the output for the MPS mailbox proc file
 *
 * \param   s        Pointer to seq_file struct.
 * \return  0 on success
 * \ingroup Internal
 */
static IFX_int32_t ifx_mps_get_mailbox_proc (struct seq_file *s)
{
   IFX_uint32_t i;

   seq_printf (s, " * CMD * UP");
   seq_printf (s, " (wr: 0x%08x, rd: 0x%08x)\n",
               (IFX_uint32_t) ifx_mps_dev.cmd_upstrm_fifo.pend +
               (IFX_uint32_t) * ifx_mps_dev.cmd_upstrm_fifo.pwrite_off,
               (IFX_uint32_t) ifx_mps_dev.cmd_upstrm_fifo.pend +
               (IFX_uint32_t) * ifx_mps_dev.cmd_upstrm_fifo.pread_off);
   for (i = 0; i < ifx_mps_dev.cmd_upstrm_fifo.size; i += 16)
   {
      seq_printf (s, "   0x%08x: %08x %08x %08x %08x\n",
                  (IFX_uint32_t) (ifx_mps_dev.cmd_upstrm_fifo.pend + (i / 4)),
                  *(ifx_mps_dev.cmd_upstrm_fifo.pend + (i / 4)),
                  *(ifx_mps_dev.cmd_upstrm_fifo.pend + 1 + (i / 4)),
                  *(ifx_mps_dev.cmd_upstrm_fifo.pend + 2 + (i / 4)),
                  *(ifx_mps_dev.cmd_upstrm_fifo.pend + 3 + (i / 4)));
   }

   seq_printf (s, "\n * CMD * DO");
   seq_printf (s, " (wr: 0x%08x, rd: 0x%08x)\n",
               (IFX_uint32_t) ifx_mps_dev.cmd_dwstrm_fifo.pend +
               (IFX_uint32_t) * ifx_mps_dev.cmd_dwstrm_fifo.pwrite_off,
               (IFX_uint32_t) ifx_mps_dev.cmd_dwstrm_fifo.pend +
               (IFX_uint32_t) * ifx_mps_dev.cmd_dwstrm_fifo.pread_off);
   for (i = 0; i < ifx_mps_dev.cmd_dwstrm_fifo.size; i += 16)
   {
      seq_printf (s, "   0x%08x: %08x %08x %08x %08x\n",
                  (IFX_uint32_t) (ifx_mps_dev.cmd_dwstrm_fifo.pend + (i / 4)),
                  *(ifx_mps_dev.cmd_dwstrm_fifo.pend + (i / 4)),
                  *(ifx_mps_dev.cmd_dwstrm_fifo.pend + 1 + (i / 4)),
                  *(ifx_mps_dev.cmd_dwstrm_fifo.pend + 2 + (i / 4)),
                  *(ifx_mps_dev.cmd_dwstrm_fifo.pend + 3 + (i / 4)));
   }

   seq_printf (s, "\n * VOICE * UP");
   seq_printf (s, " (wr:0x%08x, rd: 0x%08x)\n",
               (IFX_uint32_t) ifx_mps_dev.voice_upstrm_fifo.pend +
               (IFX_uint32_t) * ifx_mps_dev.voice_upstrm_fifo.pwrite_off,
               (IFX_uint32_t) ifx_mps_dev.voice_upstrm_fifo.pend +
               (IFX_uint32_t) * ifx_mps_dev.voice_upstrm_fifo.pread_off);
   for (i = 0; i < ifx_mps_dev.voice_upstrm_fifo.size; i += 16)
   {
      seq_printf (s, "   0x%08x: %08x %08x %08x %08x\n",
                  (IFX_uint32_t) (ifx_mps_dev.voice_upstrm_fifo.pend + (i / 4)),
                  *(ifx_mps_dev.voice_upstrm_fifo.pend + (i / 4)),
                  *(ifx_mps_dev.voice_upstrm_fifo.pend + 1 + (i / 4)),
                  *(ifx_mps_dev.voice_upstrm_fifo.pend + 2 + (i / 4)),
                  *(ifx_mps_dev.voice_upstrm_fifo.pend + 3 + (i / 4)));
   }

   seq_printf (s, "\n * VOICE * DO");
   seq_printf (s, " (wr: 0x%08x, rd: 0x%08x)\n",
               (IFX_uint32_t) ifx_mps_dev.voice_dwstrm_fifo.pend +
               (IFX_uint32_t) * ifx_mps_dev.voice_dwstrm_fifo.pwrite_off,
               (IFX_uint32_t) ifx_mps_dev.voice_dwstrm_fifo.pend +
               (IFX_uint32_t) * ifx_mps_dev.voice_dwstrm_fifo.pread_off);
   for (i = 0; i < ifx_mps_dev.voice_dwstrm_fifo.size; i += 16)
   {
      seq_printf (s, "   0x%08x: %08x %08x %08x %08x\n",
                  (IFX_uint32_t) (ifx_mps_dev.voice_dwstrm_fifo.pend + (i / 4)),
                  *(ifx_mps_dev.voice_dwstrm_fifo.pend + (i / 4)),
                  *(ifx_mps_dev.voice_dwstrm_fifo.pend + 1 + (i / 4)),
                  *(ifx_mps_dev.voice_dwstrm_fifo.pend + 2 + (i / 4)),
                  *(ifx_mps_dev.voice_dwstrm_fifo.pend + 3 + (i / 4)));
   }

   seq_printf (s, "\n * EVENT * UP");
   seq_printf (s, " (wr: 0x%08x, rd: 0x%08x)\n",
               (IFX_uint32_t) ifx_mps_dev.event_upstrm_fifo.pend +
               (IFX_uint32_t) * ifx_mps_dev.event_upstrm_fifo.pwrite_off,
               (IFX_uint32_t) ifx_mps_dev.event_upstrm_fifo.pend +
               (IFX_uint32_t) * ifx_mps_dev.event_upstrm_fifo.pread_off);
   for (i = 0; i < ifx_mps_dev.event_upstrm_fifo.size; i += 16)
   {
      seq_printf (s, "   0x%08x: %08x %08x %08x %08x\n",
                  (IFX_uint32_t) (ifx_mps_dev.event_upstrm_fifo.pend + (i / 4)),
                  *(ifx_mps_dev.event_upstrm_fifo.pend + (i / 4)),
                  *(ifx_mps_dev.event_upstrm_fifo.pend + 1 + (i / 4)),
                  *(ifx_mps_dev.event_upstrm_fifo.pend + 2 + (i / 4)),
                  *(ifx_mps_dev.event_upstrm_fifo.pend + 3 + (i / 4)));
   }

   return 0;
}


/**
 * Create MPS sw fifo proc file output.
 * This function creates the output for the sw fifo proc file
 *
 * \param   s        Pointer to seq_file struct.
 * \return  0 on success
 * \ingroup Internal
 */
static IFX_int32_t ifx_mps_get_swfifo_proc (struct seq_file *s)
{
   IFX_int32_t i, chan;

   for (chan = 0; chan < (NUM_VOICE_CHANNEL - 1); chan++)
   {
      seq_printf (s, "\n"
                     " * CH%i * UP", chan);
      seq_printf (s, " (wr:0x%08x, rd: 0x%08x)\n",
                  (IFX_uint32_t) ifx_mps_dev.sw_upstrm_fifo[chan].pend +
                  (IFX_uint32_t) * ifx_mps_dev.sw_upstrm_fifo[chan].pwrite_off,
                  (IFX_uint32_t) ifx_mps_dev.sw_upstrm_fifo[chan].pend +
                  (IFX_uint32_t) * ifx_mps_dev.sw_upstrm_fifo[chan].pread_off);

      for (i = 0; i < ifx_mps_dev.sw_upstrm_fifo[chan].size; i += 16)
      {
         seq_printf (s, "   0x%08x: %08x %08x %08x %08x\n",
                     (IFX_uint32_t) (ifx_mps_dev.sw_upstrm_fifo[chan].pend +
                                     (i / 4)),
                     *(ifx_mps_dev.sw_upstrm_fifo[chan].pend + (i / 4)),
                     *(ifx_mps_dev.sw_upstrm_fifo[chan].pend + 1 + (i / 4)),
                     *(ifx_mps_dev.sw_upstrm_fifo[chan].pend + 2 + (i / 4)),
                     *(ifx_mps_dev.sw_upstrm_fifo[chan].pend + 3 + (i / 4)));
      }
   }

   return 0;
}


#if CONFIG_MPS_HISTORY_SIZE > 0
/**
 * Process MPS proc file output.
 * This function outputs the history buffer showing the messages
 * sent to command mailbox so far.
 *
 * \param   s        Pointer to seq_file struct.
 * \return  0 on success
 * \ingroup Internal
 */
static IFX_int32_t ifx_mps_read_history_proc (struct seq_file *s)
{
   IFX_int32_t begin, len, i, index = 0;

   /* ifx_mps_history_buffer_overflowed is set to 1 if the number of written
      command words is >= MPS_HISTORY_BUFFER_SIZE; See drv_mps_vmmc_common.c:
      ifx_mps_mbx_write_cmd() ifx_mps_history_buffer_words is in the range
      [0 ... (MPS_HISTORY_BUFFER_SIZE-1)] */
   if (ifx_mps_history_buffer_overflowed == 0)
   {
      /* print ifx_mps_history_buffer_words words from 0 index to
         ifx_mps_history_buffer_words */
      len = ifx_mps_history_buffer_words;
      begin = 0;
   }
   else
   {
      /* print MPS_HISTORY_BUFFER_SIZE words from ifx_mps_history_buffer_words
         index to the end of buffer, then from 0 to the remaining index (ring
         operation) */
      len = MPS_HISTORY_BUFFER_SIZE;
      begin = ifx_mps_history_buffer_words;
   }

   seq_printf (s, "Printing last %d words...\n", len);

   for (i = begin, index = 0; len > 0; len--, i++, index++ )
   {
      if (i >= MPS_HISTORY_BUFFER_SIZE)
         i = 0;

      seq_printf (s, "%5d: %08x\n", index,
                  ifx_mps_history_buffer[i]);
   }

   if (ifx_mps_history_buffer_freeze)
      seq_printf (s,
                  "---- FREEZE ----\n"
                  "To restart logging write '1' to this proc file.\n");

   return 0;
}


/**
 * Process MPS proc file input.
 * This function unlocks the command history logging.
 *
 * \param   file    file structure for proc file
 * \param   buffer  buffer holding the data
 * \param   count   number of characters in buffer
 * \param   data    unused
 * \return  count   Number of processed characters
 * \ingroup Internal
 */
static IFX_int32_t ifx_mps_write_history_proc (struct file *pfile,
                                               const IFX_char_t * buffer,
                                               IFX_ulong_t count,
                                               IFX_void_t * data)
{
   pfile = pfile;
   buffer = buffer;
   data = data;

   ifx_mps_history_buffer_freeze = 0;
   ifx_mps_history_buffer_words = 0;
   ifx_mps_history_buffer_overflowed = 0;
   TRACE (MPS, DBG_LEVEL_HIGH, ("MPS command history logging restarted!\n"));
   return count;
}
#endif /* CONFIG_MPS_HISTORY_SIZE > 0 */


/**
 * Create MPS config proc file output.
 * This function creates the output for MPS runtime dynamic configuration.
 *
 * \param   s        Pointer to seq_file struct.
 * \return  0 on success
 * \ingroup Internal
 */
static IFX_int32_t ifx_mps_config_get_proc(struct seq_file *s)
{
   mps_comm_dev *pDev = &ifx_mps_dev;

   seq_printf (s, "CPU1 LOAD ADDRESS = 0x%08X\n", (IFX_uint32_t)cpu1_base_addr);
   seq_printf (s, "MPS REG BASE      = 0x%08X\n", ifx_mps_reg_base);
   seq_printf (s, "MPS RAM BASE      = 0x%08X\n", ifx_mps_ram_base);
   seq_printf (s, "MPS CHIPID        = 0x%08X\n", *IFX_MPS_CHIPID);
   seq_printf (s, "MPS INTERRUPT     = %d\n", ifx_mps_ir4);

   seq_printf (s, "MBox Layout       = 0x%X\n", ifx_mps_dev.mbx_descr);

   switch (ifx_mps_dev.mbx_descr & LTQ_VOICE_MPS_MBOX_TYPE_ALL)
   {
   case LTQ_VOICE_MPS_MBOX_TYPE_L1:
      {
         if (ifx_mps_dev.base_global == IFX_NULL)
         {
            seq_printf (s, "MBox Layout 1 - not initialized\n");
         }
         else
         {
            mps_mbx_reg *p_mb = ifx_mps_dev.base_global;

            seq_printf (s, "MBox Layout 1 @ 0x%08X          size: 0x%04X (%u)\n",
               (IFX_uint32_t)p_mb, sizeof(*p_mb), sizeof(*p_mb));
            seq_printf (s, "  UPSTR_CMD   : 0x%08X / 0x%08X / 0x%04X (%u)\n",
               (IFX_uint32_t)pDev->cmd_upstrm_fifo.pend,
               (IFX_uint32_t)p_mb->MBX_UPSTR_CMD_BASE,
               p_mb->MBX_UPSTR_CMD_SIZE, p_mb->MBX_UPSTR_CMD_SIZE);
            seq_printf (s, "  DNSTR_CMD   : 0x%08X / 0x%08X / 0x%04X (%u)\n",
               (IFX_uint32_t)pDev->cmd_dwstrm_fifo.pend,
               (IFX_uint32_t)p_mb->MBX_DNSTR_CMD_BASE,
               p_mb->MBX_DNSTR_CMD_SIZE, p_mb->MBX_DNSTR_CMD_SIZE);
            seq_printf (s, "  UPSTR_DATA  : 0x%08X / 0x%08X / 0x%04X (%u)\n",
               (IFX_uint32_t)pDev->voice_upstrm_fifo.pend,
               (IFX_uint32_t)p_mb->MBX_UPSTR_DATA_BASE,
               p_mb->MBX_UPSTR_DATA_SIZE, p_mb->MBX_UPSTR_DATA_SIZE);
            seq_printf (s, "  DNSTR_DATA  : 0x%08X / 0x%08X / 0x%04X (%u)\n",
               (IFX_uint32_t)pDev->voice_dwstrm_fifo.pend,
               (IFX_uint32_t)p_mb->MBX_DNSTR_DATA_BASE,
               p_mb->MBX_DNSTR_DATA_SIZE, p_mb->MBX_DNSTR_DATA_SIZE);
            seq_printf (s, "  UPSTR_EVENT : 0x%08X / 0x%08X / 0x%04X (%u)\n",
               (IFX_uint32_t)pDev->event_upstrm_fifo.pend,
               (IFX_uint32_t)p_mb->MBX_UPSTR_EVENT_BASE,
               p_mb->MBX_UPSTR_EVENT_SIZE, p_mb->MBX_UPSTR_EVENT_SIZE);
         }
      }
      break;

   case LTQ_VOICE_MPS_MBOX_TYPE_L2:
      {
         if (ifx_mps_dev.base_global_l2 == IFX_NULL)
         {
            seq_printf (s, "MBox Layout 2 - not initialized\n");
         }
         else
         {
            mps_mbx_reg_defs_l2 *p_mb = &ifx_mps_dev.base_global_l2->mbx_reg_defs_l2;

            seq_printf (s, "MBox Layout 2 @ 0x%08X          size: 0x%04X (%u)\n",
               (IFX_uint32_t)p_mb, sizeof(*p_mb), sizeof(*p_mb));
            seq_printf (s, "  UPSTR_CMD   : 0x%08X / 0x%08X / 0x%04X (%u)\n",
               (IFX_uint32_t)pDev->cmd_upstrm_fifo.pend,
               (IFX_uint32_t)p_mb->MBX_UPSTR_CMD_BASE,
               VOICE_MBX_US_CMD_SIZE_GET(p_mb), VOICE_MBX_US_CMD_SIZE_GET(p_mb));
            seq_printf (s, "  DNSTR_CMD   : 0x%08X / 0x%08X / 0x%04X (%u)\n",
               (IFX_uint32_t)pDev->cmd_dwstrm_fifo.pend,
               (IFX_uint32_t)p_mb->MBX_DNSTR_CMD_BASE,
               p_mb->MBX_DNSTR_CMD_SIZE, p_mb->MBX_DNSTR_CMD_SIZE);
            seq_printf (s, "  UPSTR_DATA  : 0x%08X / 0x%08X / 0x%04X (%u)\n",
               (IFX_uint32_t)pDev->voice_upstrm_fifo.pend,
               (IFX_uint32_t)p_mb->MBX_UPSTR_DATA_BASE,
               p_mb->MBX_UPSTR_DATA_SIZE, p_mb->MBX_UPSTR_DATA_SIZE);
            seq_printf (s, "  DNSTR_DATA  : 0x%08X / 0x%08X / 0x%04X (%u)\n",
               (IFX_uint32_t)pDev->voice_dwstrm_fifo.pend,
               (IFX_uint32_t)p_mb->MBX_DNSTR_DATA_BASE,
               p_mb->MBX_DNSTR_DATA_SIZE, p_mb->MBX_DNSTR_DATA_SIZE);
            seq_printf (s, "  UPSTR_EVENT : 0x%08X / 0x%08X / 0x%04X (%u)\n",
               (IFX_uint32_t)pDev->event_upstrm_fifo.pend,
               (IFX_uint32_t)p_mb->MBX_UPSTR_EVENT_BASE,
               p_mb->MBX_UPSTR_EVENT_SIZE, p_mb->MBX_UPSTR_EVENT_SIZE);
            seq_printf (s, "  MPS configuration options: 0x%04X\n",
               VOICE_MBX_MPS_OPT_GET(p_mb));
         }
      }
      break;

   default:
      seq_printf (s, "MBox Layout unknown!\n");
      break;
   }

   return 0;
}


typedef void (*mps_dump) (struct seq_file *s);

static int mps_proc_show ( struct seq_file *s, void *p )
{
   mps_dump dump = s->private;

   if (dump != NULL)
      dump(s);

   return 0;
}


static int mps_proc_open ( struct inode *inode, struct file *file )
{
   return single_open (file, mps_proc_show, PDE_DATA(inode));
}


struct proc_entry
{
   const char *name;
   void *read_function;
   void *write_function;
   struct file_operations ops;
};


static struct proc_entry proc_entries[] =
{
   { "version", ifx_mps_get_version_proc},
   { "status", ifx_mps_get_status_proc},
   { "mailbox", ifx_mps_get_mailbox_proc},
   { "swfifo", ifx_mps_get_swfifo_proc},
#if CONFIG_MPS_HISTORY_SIZE > 0
   { "history", ifx_mps_read_history_proc, ifx_mps_write_history_proc},
#endif /* CONFIG_MPS_HISTORY_SIZE > 0 */
   { "config", ifx_mps_config_get_proc},
};


static void mps_proc_entrycreate ( struct proc_dir_entry *parent_node,
                 struct proc_entry *proc_entry)
{
   memset(&proc_entry->ops, 0, sizeof(struct file_operations));
   proc_entry->ops.owner   = THIS_MODULE;
   proc_entry->ops.open    = mps_proc_open;
   proc_entry->ops.read    = seq_read;
   proc_entry->ops.write   = proc_entry->write_function;
   proc_entry->ops.llseek  = seq_lseek;
   proc_entry->ops.release = single_release;

   proc_create_data ( proc_entry->name, 0, parent_node,
            &proc_entry->ops, proc_entry->read_function);
}
#endif /* CONFIG_PROC_FS */


#ifdef VMMC_FEAT_LINUX_PLATFORM_DRIVER
/**
   Linux platform driver probe function.

   Read parameters from device tree and configure driver.

   Example entry of needed device tree nodes.
   \verbatim
   mps@107000 {
      compatible = "lantiq,mps-xrx100";
      reg = <0x107000 0x400>;
      interrupt-parent = <&icu0>;
      interrupts = <154 155>;
      lantiq,mbx = <&mpsmbx>;
   };

   mpsmbx: mpsmbx@20000 {
      reg = <0x200000 0x200>;
   };
   \endverbatim

   \param  pdev         Pointer to struct platform_device.

   \return
   0 Successful
   !0 Failed to find the config or the device.
*/
int ifx_mps_probe(struct platform_device *pdev)
{
   struct device_node   *pdn;
   struct resource      *res = NULL;
   IFX_int32_t          ret;

   /* Clear parameters to detect if all can be set below. */
   ifx_mps_ir4 = 0;
   ifx_mps_reg_base = ifx_mps_ram_base = 0;

   /* Interrupt number connected to the AD0 register. */
   res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
   if (res)
   {
      ifx_mps_ir4 = res->start;
   }

   /* Baseaddress of the MPS register block. */
   res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
   if (res)
   {
      static void __iomem *mps_reg_base;

      mps_reg_base = devm_ioremap_resource(&pdev->dev, res);
      if (IS_ERR(mps_reg_base))
         return PTR_ERR(mps_reg_base);

      ifx_mps_reg_base = (IFX_uint32_t)mps_reg_base;
   }

   ret = of_get_named_gpio(pdev->dev.of_node, "lantiq,slic-reset", 0);
   if (ret < 0) {
        dev_err(&pdev->dev, "failed to get slic-reset GPIO from DT\n");
        return ret;
   }
   lq_mps_slic_reset_gpio = ret;

   /* Baseaddress of the MPS SRAM region. This is used for mailboxes. */
   /* Dereference the phandle pointing to the node with the actual address. */
   pdn = of_parse_phandle(pdev->dev.of_node, "lantiq,mbx", 0);
   if (pdn)
   {
      struct resource mbx_res;

      if (of_address_to_resource(pdn, 0, &mbx_res) == 0)
      {
         static void __iomem *mps_ram_base;

         mps_ram_base = devm_ioremap_resource(&pdev->dev, &mbx_res);
         if (IS_ERR(mps_ram_base))
         {
            of_node_put(pdn);
            return PTR_ERR(mps_ram_base);
         }

         ifx_mps_ram_base = (IFX_uint32_t)mps_ram_base;
      }
      of_node_put(pdn);
   }

   /* Verify that all parameters were set. */
   if (!ifx_mps_ir4 || !ifx_mps_reg_base || !ifx_mps_ram_base)
   {
      if (!ifx_mps_ir4)
      {
         printk(KERN_ERR "Failed to find MPS AD0 irq in device tree.\n");
      }
      if (!ifx_mps_reg_base)
      {
         printk(KERN_ERR "Failed to find MPS reg base in device tree.\n");
      }
      if (!ifx_mps_ram_base)
      {
         printk(KERN_ERR "Failed to find MPS mbx base in device tree.\n");
      }

      return -ENXIO;
   }

   /* enable the voice clocks */
   if (clk == NULL)
   {
      clk = clk_get_sys("voice", "voice");
      if (IS_ERR(clk)) {
         dev_warn(&pdev->dev, "failed to get clock\n");
      }
      else
      {
         clk_enable(clk);
      }
   }

   ret = ifx_mps_hw_enable();
   if ((ret != 0) && !IS_ERR(clk))
   {
      clk_put(clk);
      clk = NULL;
   }

   return ret;
}
#endif /* VMMC_FEAT_LINUX_PLATFORM_DRIVER */

/**
   This function initializes the module.
\param
   None.
\return  IFX_SUCCESS, module initialized
\return  EPERM    Reset of CPU1 failed
\return  ENOMEM   No memory left for structures
*/
#ifdef VMMC_WITH_MPS
IFX_int32_t
#else
static int __init
#endif
ifx_mps_init_module (void)
{
   IFX_int32_t result;
   unsigned int virt;

   sprintf (ifx_mps_device_version, "%d.%d.%d.%d", MAJORSTEP, MINORSTEP,
            VERSIONSTEP, VERS_TYPE);

   TRACE (MPS, DBG_LEVEL_HIGH,
          ("%s%s, (c) 2006-2015 Lantiq Deutschland GmbH\n", IFX_MPS_INFO_STR,
           ifx_mps_device_version));

   sprintf (ifx_mps_dev_name, IFX_MPS_DEV_NAME);

   /* setup cache operations */
#if defined(CONFIG_MIPS) && !defined(CONFIG_MIPS_UNCACHED)
   /* on AR9/VR9 cache is configured by BSP;
      here we check whether the D-cache is shared or partitioned;
      1) in case of shared D-cache all cache operations are omitted;
      2) in case of partitioned D-cache the cache operations are performed,
      the same way as on Danube */
   if(read_c0_mvpcontrol() & MVPCONTROL_CPA_BIT)
   {
      if (read_vpe_c0_vpeopt() & VPEOPT_DWX_MASK &&
          (read_vpe_c0_vpeopt() & VPEOPT_DWX_MASK) != VPEOPT_DWX_MASK)
      {
         bDoCacheOps = IFX_TRUE;
      }
   }
#endif /*defined(CONFIG_MIPS) && !defined(CONFIG_MIPS_UNCACHED)*/

#ifndef VMMC_FEAT_LINUX_PLATFORM_DRIVER
   lq_mps_slic_reset_gpio = 31;

   /* Enable the HW. This means power-domains, clock-domains, ... */
   result = ifx_mps_hw_enable();
   if (result)
      return result;
#endif /* !VMMC_FEAT_LINUX_PLATFORM_DRIVER */

   /* init the device driver structure */
   if (0 != ifx_mps_init_structures (&ifx_mps_dev))
      return -ENOMEM;

   /* register char module in kernel */
   result = lq_mps_os_register (&ifx_mps_dev);
   if (result)
      return result;

   /* reset the device before initializing the device driver */
   ifx_mps_reset ();

   /* Set handler for interrupt generated by voice-FW AD0 status register. */
   virt = irq_create_mapping(NULL, ifx_mps_ir4);
   if (!virt)
      return -1;
   result = request_irq (virt,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
                         ifx_mps_ad0_irq, IRQF_DISABLED
#else /* */
                         (irqreturn_t (*)(int, IFX_void_t *, struct pt_regs *))
                         ifx_mps_ad0_irq, SA_INTERRUPT
#endif /* */
                         , "mps_mbx ad0", &ifx_mps_dev);
   if (result)
      return result;

#if !defined(CONFIG_LANTIQ)
   /* Legacy support only. To be removed without notice. */
   /* Enable the AD0 interrupt at ICU0. */
   ICU0_IM4_IRQ_ENABLE(IMx_IR18);
#endif

   /* enable mailbox interrupts */
   ifx_mps_enable_mailbox_int ();
   /* init FW ready event */
   IFXOS_EventInit (&fw_ready_evt);

#ifdef CONFIG_PROC_FS
   /* install the proc entry */
#ifdef DEBUG
   TRACE (MPS, DBG_LEVEL_HIGH, (KERN_INFO "IFX_MPS: using proc fs\n"));
#endif /* */
   ifx_mps_proc_dir = proc_mkdir ("driver/" IFX_MPS_DEV_NAME, IFX_NULL);
   if (ifx_mps_proc_dir != IFX_NULL)
   {
      IFX_uint32_t i;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,30))
      ifx_mps_proc_dir->owner = THIS_MODULE;
#endif /* < Linux 2.6.30 */

      for(i=0; i<sizeof(proc_entries)/sizeof(proc_entries[0]);i++) {
         mps_proc_entrycreate (ifx_mps_proc_dir, &proc_entries[i]);
      }
   }
   else
   {
      TRACE (MPS, DBG_LEVEL_HIGH, ("IFX_MPS: cannot create proc entry\n"));
   }
#endif /* */

   return IFX_SUCCESS;
}


/**
   This function cleans up the module.
\param
   None.
\return
   None.
*/
#ifdef VMMC_WITH_MPS
IFX_void_t
#else
static void __exit
#endif
ifx_mps_cleanup_module (void)
{
   /* disable mailbox interrupts */
   ifx_mps_disable_mailbox_int ();

#if !defined(CONFIG_LANTIQ)
   /* Legacy support only. To be removed without notice. */
   /* Disable DFE/AFE 0 interrupt. */
   ICU0_IM4_IRQ_DISABLE(IMx_IR18);
#endif

   /* disable all MPS interrupts */
   ifx_mps_disable_all_int ();
   ifx_mps_shutdown ();

   /* unregister char module from kernel */
   lq_mps_os_unregister (&ifx_mps_dev);

   /* release the memory usage of the device driver structure */
   ifx_mps_release_structures (&ifx_mps_dev);

   /* release all interrupts at the system */
   free_irq (ifx_mps_ir4, &ifx_mps_dev);

#ifdef CONFIG_PROC_FS
#if CONFIG_MPS_HISTORY_SIZE > 0
   remove_proc_entry ("history", ifx_mps_proc_dir);
#endif /* CONFIG_MPS_HISTORY_SIZE > 0 */
   remove_proc_entry ("mailbox", ifx_mps_proc_dir);
   remove_proc_entry ("swfifo", ifx_mps_proc_dir);
   remove_proc_entry ("version", ifx_mps_proc_dir);
   remove_proc_entry ("status", ifx_mps_proc_dir);
   remove_proc_entry ("config", ifx_mps_proc_dir);
   remove_proc_entry ("driver/" IFX_MPS_DEV_NAME, IFX_NULL);
#endif /* CONFIG_PROC_FS */

   /* Disable the HW. This means power-domains, clock-domains, ... */
   ifx_mps_hw_disable();

#ifdef VMMC_FEAT_LINUX_PLATFORM_DRIVER
   if (!IS_ERR(clk))
   {
      clk_put(clk);
      clk = NULL;
   }
#endif /* VMMC_FEAT_LINUX_PLATFORM_DRIVER */

   TRACE (MPS, DBG_LEVEL_HIGH, (KERN_INFO "Lantiq MPS driver: cleanup done\n"));
}

#ifndef VMMC_WITH_MPS
module_init (ifx_mps_init_module);
module_exit (ifx_mps_cleanup_module);

#ifndef DEBUG
EXPORT_SYMBOL (ifx_mps_write_mailbox);
EXPORT_SYMBOL (ifx_mps_register_data_callback);
EXPORT_SYMBOL (ifx_mps_unregister_data_callback);
EXPORT_SYMBOL (ifx_mps_register_event_callback);
EXPORT_SYMBOL (ifx_mps_unregister_event_callback);
EXPORT_SYMBOL (ifx_mps_read_mailbox);

EXPORT_SYMBOL (ifx_mps_dd_mbx_int_enable);
EXPORT_SYMBOL (ifx_mps_dd_mbx_int_disable);

EXPORT_SYMBOL (ifx_mps_register_event_mbx_callback);
EXPORT_SYMBOL (ifx_mps_unregister_event_mbx_callback);

EXPORT_SYMBOL (ifx_mps_ioctl);
EXPORT_SYMBOL (ifx_mps_open);
EXPORT_SYMBOL (ifx_mps_close);

#endif /* DEBUG */
#endif /* !VMMC_WITH_MPS */
#endif /* LINUX */
