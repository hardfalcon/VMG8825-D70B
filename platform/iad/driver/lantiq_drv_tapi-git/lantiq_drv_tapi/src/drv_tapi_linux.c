/******************************************************************************

                            Copyright (c) 2001-2016
                        Lantiq Beteiligungs-GmbH & Co.KG
                             http://www.lantiq.com

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/**
   \file drv_tapi_linux.c
   This file contains the implementation of High-Level TAPI Driver,
   Linux specific part.

   The implementation includes the following parts:
    -Registration part by which the low-level drivers register themselves.
    -Device node operations (open, close, ioctl, read, write, select)
     are done here.
    -Linux module support.
    -Linux /proc fileystem handlers.
    -Timer abstraction layer.
    -Deferring of a function call for later execution.
    -Export of High-Level TAPI function symbols.
*/

#ifdef LINUX

/* ============================= */
/* Includes                      */
/* ============================= */
#include "drv_tapi_config.h"

#ifdef __KERNEL__
   #include <linux/kernel.h>
   #include <linux/version.h>
#endif /* __KERNEL__ */
#ifdef MODULE
   #include <linux/module.h>
#endif /* MODULE */

#ifdef __KERNEL__
   #ifdef TAPI_FEAT_PROCFS
      #include <linux/proc_fs.h>       /*proc-file system*/
      #include <linux/seq_file.h>
   #endif
   #if (LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0))
      #define PDE_DATA(inode) PDE(inode)->data
   #endif
   #include <linux/timer.h>            /* init_timer() */
   #include <linux/init.h>
   #include <linux/errno.h>
   #ifdef ENABLE_HOTPLUG
      #include <linux/skbuff.h>
      #include <linux/netlink.h>
      #include <net/sock.h>
      #include <linux/kobject.h>
   #endif /* #ifdef ENABLE_HOTPLUG*/
   #include <asm/byteorder.h>
   #include <asm/io.h>
   #if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
      #include <linux/tqueue.h>
      #include <linux/sched.h>
   #else
      #include <linux/device.h>
      #include <linux/sched.h>
   #endif /* LINUX_VERSION_CODE check */
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
#endif /* __KERNEL__ */

#include "drv_tapi.h"
#include "drv_tapi_api.h"
#include "drv_tapi_errno.h"
#include "drv_tapi_stream.h"
#include "drv_tapi_ioctl.h"

#ifndef IFXOS_USE_DEV_IO
   #include "drv_tapi_cid.h"
#endif /* IFXOS_USE_DEV_IO */

#include "drv_tapi_ppd.h"
#include "drv_tapi_qos.h"

/* ============================= */
/* Local Macros & Definitions    */
/* ============================= */

#ifdef ENABLE_HOTPLUG
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,37)
extern struct sock *uevent_sock;
#endif
extern u64 uevent_next_seqnum(void);
static struct class *tapi_class;
#endif /* #ifdef ENABLE_HOTPLUG*/

/* ================================== */
/* channel specific wrapper structure */
/* ================================== */
#ifndef IFXOS_USE_DEV_IO
typedef struct _TAPI_FD_PRIV_DATA TAPI_FD_PRIV_DATA_t;

struct _TAPI_FD_PRIV_DATA
{
   /* ptr to tapi channel or device specific data */
   IFX_void_t                   *pTapiCtx;
   /* channel fifo number                         */
   IFX_TAPI_STREAM_t            fifo_idx;
};

/* ============================= */
/* kernel ioctl api: structure   */
/* to reference a channel        */
/* (see drv_tapi_kio.h)          */
/* ============================= */
typedef struct
{
   /* storage for major and minor numbers */
   struct inode inode;
   /* storage for private date of real open procedure */
   struct file file;
} IFX_TAPI_Ch_KernRef_t ;

#define MAX_EXPECTED_LL_DRV 5

/* ============================= */
/* Local Functions               */
/* ============================= */
#ifdef __KERNEL__
/*lint -save
   -esym(528,TAPI_timer_call_back)
   -esym(528,TAPI_tqueue) */
static IFX_void_t TAPI_timer_call_back (IFX_ulong_t arg);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20))
static IFX_void_t TAPI_tqueue (IFX_void_t *pWork);
#else /* for Kernel newer or equal 2.6.20 */
static IFX_void_t TAPI_tqueue (struct work_struct *pWork);
#endif /* (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)) */
/*lint -restore */
#endif /* __KERNEL__ */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
/* TAPI's timers workqueue */
static struct workqueue_struct *pTAPItimersWq;
/* TAPI's deferred tasks (events) workqueue. Events and timers should use
 * separate workqueues, because events might be expecting triggering of
 * timers, which will not happen if a single queue is used for both. */
static struct workqueue_struct *pTAPIeventsWq;
#  if (LINUX_VERSION_CODE <= KERNEL_VERSION(3,8,0))
/* init struct for the RT workqueue to set the scheduling policy
   via TAPI_DeferWork */
static IFX_TAPI_EXT_EVENT_PARAM_t tapi_wq_setscheduler_param;
#  endif
#endif /* LINUX_VERSION_CODE */

static IFX_int32_t TAPI_SelectCh (TAPI_FD_PRIV_DATA_t *pTapiPriv,
                                  TAPI_OS_drvSelectTable_t *,
                                  TAPI_OS_drvSelectOSArg_t *);

static int ifx_tapi_open (struct inode *inode, struct file *filp);
static int ifx_tapi_release (struct inode *inode, struct file *filp);
static ssize_t ifx_tapi_write(struct file *filp, const char *buf,
                              size_t count, loff_t * ppos);
static ssize_t ifx_tapi_read(struct file * filp, char *buf,
                              size_t length, loff_t * ppos);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36))
static int ifx_tapi_ioctl(struct inode *inode, struct file *filp,
                              unsigned int nCmd, unsigned long nArgument);
#else
static long ifx_tapi_ioctl(struct file *filp,
                              unsigned int nCmd, unsigned long nArgument);
#endif
static IFX_uint32_t ifx_tapi_poll (struct file *filp, poll_table *table);

#ifdef TAPI_FEAT_PROCFS
/* single call read proc entry callback function */
typedef IFX_void_t (*proc_single_callback_t)(struct seq_file *);
/* multiple call read proc entry callback function */
typedef int (*proc_callback_t)(struct seq_file *, int);
/* used to get the number of times multiple call read proc entry callback
 * function should be called to get the full output (typically equals number
 * of devices). The number returned by this function is assigned to nMaxPos
 * field of proc_file_entry structure.  */
typedef int (*proc_init_callback_t)(void);
/* write function */
typedef ssize_t (*proc_write_t)(struct file *file, const char __user *buffer,
   size_t count, loff_t *data);

static void *TAPI_seq_start(struct seq_file *s, loff_t *pos);
static void *TAPI_seq_next(struct seq_file *s, void *v, loff_t *pos);
static void TAPI_seq_stop(struct seq_file *s, void *v);
static int TAPI_seq_show(struct seq_file *s, void *v);

struct proc_file_entry {
   /* function used for data output */
   proc_callback_t callback;
   /* current output position */
   int nPos;
   /* maximum output position */
   int nMaxPos;
};

struct proc_entry {
   char *name;
   proc_single_callback_t single_callback;
   proc_callback_t callback;
   proc_init_callback_t init_callback;
   proc_write_t write_function;
   struct file_operations ops;
};

static struct seq_operations TAPI_seq_ops = {
   .start   = TAPI_seq_start,
   .next = TAPI_seq_next,
   .stop = TAPI_seq_stop,
   .show = TAPI_seq_show
};

static IFX_void_t proc_get_tapi_version(struct seq_file *s);
#ifdef HAVE_CONFIG_H
static IFX_void_t proc_ConfigureGet(struct seq_file *s);
#endif
static IFX_void_t proc_get_tapi_status(struct seq_file *s);
static IFX_void_t proc_get_tapi_registered_drivers(struct seq_file *s);
static IFX_void_t proc_read_bufferpool (struct seq_file *s);
#ifdef TAPI_PACKET_OWNID
static IFX_void_t proc_read_voice_bufferpool (struct seq_file *s);
#endif /* TAPI_PACKET_OWNID */
static int proc_read_fifos (struct seq_file *s, int pos);
static int proc_get_dev_count (void);

#ifdef TAPI_FEAT_PHONE_DETECTION_PROCFS
static IFX_void_t tapi_proc_ppd_read_help(
                        struct seq_file *s);

static int tapi_proc_ppd_read_status(
                        struct seq_file *s, int pos);

static int tapi_proc_ppd_wr_open(
                        struct inode *inode,
                        struct file *file);

static ssize_t tapi_proc_ppd_wr_device(
                        struct file *file,
                        const char __user *buffer,
                        size_t count,
                        loff_t *offset);

static IFX_boolean_t tapi_proc_ppd_wr_devParseCfgLine(
                        IFX_char_t* sCfgLine,
                        IFX_boolean_t* bAllCh,
                        IFX_int32_t* pCh,
                        IFX_boolean_t* bEnable);
#endif /* TAPI_FEAT_PHONE_DETECTION_PROCFS */

extern IFX_int32_t IFX_TAPI_EventWrpBufferPool_ElementCountGet(void);
extern IFX_int32_t IFX_TAPI_EventWrpBufferPool_ElementAvailCountGet(void);

#ifdef ENABLE_HOTPLUG
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,37)
extern struct sock *uevent_sock;
#endif
extern u64 uevent_next_seqnum(void);
#endif /* #ifdef ENABLE_HOTPLUG*/

#ifdef TAPI_FEAT_STATISTICS
static int proc_read_tapi_stats (struct seq_file *s, int pos);
ssize_t proc_write_tapi_stats(struct file *file, const char __user *buffer,
   size_t count, loff_t *data);
#endif /* TAPI_FEAT_STATISTICS */
#endif /* TAPI_FEAT_PROCFS */

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
static IFX_int32_t tapiClassCreate(void);
static IFX_void_t tapiClassRemove(void);
#endif /* >= Linux 2.6.0 */

/*lint -save -e{19, 546, 826} */

/*lint -save -esym(752,TAPI_debug_level) */
/** install parameter TAPI_debug_level: LOW (1), NORMAL (2), HIGH (3), OFF (4) */
#ifdef ENABLE_TRACE
extern IFX_uint32_t TAPI_debug_level;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,17))
MODULE_PARM(TAPI_debug_level, "i");
#else
module_param(TAPI_debug_level, uint, 0);
#endif /* < 2.6.17 */
MODULE_PARM_DESC(TAPI_debug_level, "set to get more (1) or fewer (4) debug outputs");
#endif /* ENABLE_TRACE */
/*lint -restore */

IFX_int32_t block_egress_tasklet = 0;
IFX_int32_t block_ingress_tasklet = 0;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,17))
MODULE_PARM(block_egress_tasklet, "i");
MODULE_PARM(block_ingress_tasklet, "i");
#else
module_param(block_egress_tasklet, int, 0);
module_param(block_ingress_tasklet, int, 0);
#endif /* < 2.6.17 */
MODULE_PARM_DESC(block_egress_tasklet, "block the registration of egress tasklets, i.e. force to use the RT kernel thread");
MODULE_PARM_DESC(block_ingress_tasklet, "block the execution of the ingress tasklet, i.e. force to use the RT kernel thread");

/*lint -restore */

/** The driver callbacks which will be registered with the kernel*/
static struct file_operations tapi_fops;

/* ============================= */
/* Local variables               */
/* ============================= */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
static struct class *pTAPI_Class;
#endif /* >= Linux 2.6.0 */

#ifdef TAPI_FEAT_PROCFS

static struct proc_entry proc_entries[] = {
   {"version",                   proc_get_tapi_version, NULL, NULL, NULL},
#ifdef HAVE_CONFIG_H
   {"configure",                 proc_ConfigureGet, NULL, NULL, NULL},
#endif
   {"status",                    proc_get_tapi_status, NULL, NULL, NULL},
   {"registered_drivers",        proc_get_tapi_registered_drivers, NULL, NULL, NULL},
   {"bufferpool",                proc_read_bufferpool, NULL, NULL, NULL},
#ifdef TAPI_PACKET_OWNID
   {"voice_buffers",             proc_read_voice_bufferpool, NULL, NULL, NULL},
#endif
#ifdef TAPI_FEAT_STATISTICS
   {"statistic",                 NULL, proc_read_tapi_stats, proc_get_dev_count,
      proc_write_tapi_stats},
#endif /* TAPI_FEAT_STATISTICS */
   {"fifo_status",               NULL, proc_read_fifos, proc_get_dev_count,
      NULL}
};

#ifdef TAPI_FEAT_PHONE_DETECTION_PROCFS
static struct proc_dir_entry *ppd_dev_proc_node;

static struct proc_entry proc_entries_ppd[] = {
   {"help",    tapi_proc_ppd_read_help, NULL, NULL, NULL},
   {"status",  NULL, tapi_proc_ppd_read_status, proc_get_dev_count, NULL}
};

static const struct file_operations ppd_device_proc_fops = {
   .open       = tapi_proc_ppd_wr_open,
   .write      = tapi_proc_ppd_wr_device
};
#endif /* TAPI_FEAT_PHONE_DETECTION_PROCFS */

#endif /* TAPI_FEAT_PROCFS */

/* ============================= */
/* Global function definition    */
/* ============================= */
/**
   Register the low level driver with the operating system.

   This function will be called when the low-level driver is added to the
   system. Any OS specific registration or set-up should be done here.

   For Linux kernels with devfs support device nodes will be created here.
   The number of nodes created depend on the "one device node" or "multiple
   device node" configuration.
   For Linux kernels without devfs support the character driver is registered.

   \param  pLLDrvCtx    Pointer to device driver context created by LL-driver.
   \param  pHLDrvCtx    Pointer to high-level driver context.

   \return
   TAPI_statusOk

   \remarks
   For Linux the device nodes need to be registered and then the driver
   itself is registered with the kernel.
*/
IFX_return_t TAPI_OS_RegisterLLDrv (IFX_TAPI_DRV_CTX_t* pLLDrvCtx,
                                    IFX_TAPI_HL_DRV_CTX_t* pHLDrvCtx)
{
   IFX_int32_t ret = 0;
   IFX_uint16_t majorNumber;
   IFX_uint16_t minorNumber;
   IFX_uint16_t maxDevices,
                maxChannels;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
   IFX_uint16_t nDevIdx,
                nChIdx;
   IFX_char_t sDevName[15];
   IFX_uint16_t nTapiFdIdx = 0;
   dev_t       dev;
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)) */
#ifdef ENABLE_HOTPLUG
   int i, j;
#endif /* ENABLE_HOTPLUG */

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36))
   if (tapi_fops.ioctl == IFX_NULL)
#else
   if (tapi_fops.unlocked_ioctl == IFX_NULL)
#endif
   {
#ifdef MODULE
      tapi_fops.owner =    THIS_MODULE;
#endif
      tapi_fops.read =     ifx_tapi_read;
      tapi_fops.write =    ifx_tapi_write;
      tapi_fops.poll =     ifx_tapi_poll;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36))
      tapi_fops.ioctl =    ifx_tapi_ioctl;
#else
      tapi_fops.unlocked_ioctl =    ifx_tapi_ioctl;
#endif
      tapi_fops.open =     ifx_tapi_open;
      tapi_fops.release =  ifx_tapi_release;
   }
   /* copy registration info from Low level driver */
   majorNumber = pLLDrvCtx->majorNumber;

#ifdef TAPI_ONE_DEVNODE
   /* Single device node interface. */
   maxDevices = 1;
   maxChannels = 0;
   /* First minor number. */
   minorNumber = 0;
   /* Number of consecutive minor numbers needed. */
   pHLDrvCtx->nDevNrCount = 1;
#else /* !defined (TAPI_ONE_DEVNODE) */
   /* Multiple device node interface. */
   maxDevices  = pLLDrvCtx->maxDevs;
   maxChannels = pLLDrvCtx->maxChannels;

   /* Allow more than 10 channels only for a single supported device. When
      supporting multiple devices limit the channel number to one digit.
      More than 10 is not possible with our numbering plan in which the decade
      indicates the device number. */
   if ((maxDevices > 1) && (maxChannels > 9))
   {
      TRACE( TAPI_DRV, DBG_LEVEL_HIGH,
         ("TAPI_DRV (major:%d): The channel resources limited up to 9, "
         "due to device entry naming limitation.\n",
         majorNumber));
      maxChannels = 9;
   }

   /* First minor number. */
   minorNumber = pLLDrvCtx->minorBase;

   /* Number of consecutive minor numbers needed. */
   if (maxDevices > 1)
   {
      pHLDrvCtx->nDevNrCount = maxDevices * 10;
   }
   else
   {
      /* Number of consecutive minor numbers needed. */
      pHLDrvCtx->nDevNrCount = 1 /*device*/ + maxChannels;
   }
#endif /* not TAPI_ONE_DEVNODE */

#ifdef ENABLE_HOTPLUG
   tapi_class = class_create(THIS_MODULE, "tapidev");
   if (IS_ERR(tapi_class))
   {
      ret = PTR_ERR(tapi_class);
      return TAPI_statusErr;
   }
   for (j = 0; j < pLLDrvCtx->maxDevs; j++)
      for (i = 0; i <= pLLDrvCtx->maxChannels; i++)
      {
         dev = device_create(tapi_class, NULL, MKDEV(majorNumber, 10 + i),
                      NULL, "%s%d%d", pLLDrvCtx->devNodeName, j+1, i);
         if (IS_ERR((void *)dev))
            return TAPI_statusErr;
      }
#endif /* #ifdef ENABLE_HOTPLUG*/

   /* limit devNodeName to 8 characters */
   sprintf (pHLDrvCtx->registeredDrvName, "ltq_tapi (%.8s)",
            pLLDrvCtx->devNodeName);

#if  (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
   /* Register the character device */
   ret = register_chrdev (majorNumber,
                          pHLDrvCtx->registeredDrvName, &tapi_fops);
   if (ret >= 0 && majorNumber == 0)
   {
      /* dynamic major, older way */
      pLLDrvCtx->majorNumber = majorNumber = ret;
   }

   if (ret < 0)
   {
      TRACE( TAPI_DRV, DBG_LEVEL_HIGH,
           ("TAPI_OS_RegisterLLDrv: unable to register chrdev"
            " using major number %d\n\r", majorNumber));
      return TAPI_statusErr;
   }
#else /* >= Linux 2.6.0 */
   /* Reserve the device number range. */
   if (majorNumber)
   {
      /* Register a major defined at module load time with a range of minors. */
      dev = MKDEV(majorNumber, 0);
      ret = register_chrdev_region(dev,
                        pHLDrvCtx->nDevNrCount,
                        pHLDrvCtx->registeredDrvName);
   }
   else
   {
      /* Request a dynamic major with a range of minors. */
      ret = alloc_chrdev_region(&dev,
                        pLLDrvCtx->minorBase,
                        pHLDrvCtx->nDevNrCount,
                        pHLDrvCtx->registeredDrvName);
      if (0 == ret)
      {
         pLLDrvCtx->majorNumber = majorNumber = MAJOR(dev);
      }
   }

   if (ret < 0)
   {
      TRACE( TAPI_DRV, DBG_LEVEL_HIGH,
           ("TAPI_OS_RegisterLLDrv: unable to register chrdev number range"
            " using major number %d with %d minors\n\r",
            majorNumber, pHLDrvCtx->nDevNrCount));
      return TAPI_statusErr;
   }

   for (nDevIdx = 0; nDevIdx < maxDevices; nDevIdx++)
   {
#if defined (TAPI_ONE_DEVNODE)
      minorNumber = 0;
#else /* TAPI_ONE_DEVNODE */
      minorNumber = pLLDrvCtx->minorBase * (nDevIdx + 1);
#endif /* TAPI_ONE_DEVNODE */

      /* add character device to Linux */
      cdev_init(&(pLLDrvCtx->pTapiDev[nDevIdx].cdev), &tapi_fops);
      pLLDrvCtx->pTapiDev[nDevIdx].cdev.owner = THIS_MODULE;

      ret = cdev_add(&(pLLDrvCtx->pTapiDev[nDevIdx].cdev),
                     MKDEV(majorNumber, minorNumber),
                     1 /*device */ + maxChannels);
      if (ret != 0)
      {
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
               ("TAPI: unable to add chrdev %s\n\r", pLLDrvCtx->devNodeName));

         cdev_del (&(pLLDrvCtx->pTapiDev[nDevIdx].cdev));
         return TAPI_statusErr;
      }

      for (nChIdx = 0; nChIdx <= maxChannels; nChIdx++)
      {
#if defined (TAPI_ONE_DEVNODE)
         minorNumber = 0;
         /* limit devNodeName to 8 characters */
         snprintf (sDevName, sizeof(sDevName), "%.8s",
            pLLDrvCtx->devNodeName);
#else /* TAPI_ONE_DEVNODE */
         /* First device have index 1 */
         minorNumber = pLLDrvCtx->minorBase * (nDevIdx + 1);
         minorNumber += nChIdx;
         /* limit devNodeName to 8 characters */
         snprintf (sDevName, sizeof(sDevName), "%.8s%d",
            pLLDrvCtx->devNodeName, minorNumber);
#endif /* TAPI_ONE_DEVNODE */

         if (nTapiFdIdx >= TAPI_MAX_DEVFS_HANDLES)
         {
            TRACE( TAPI_DRV, DBG_LEVEL_HIGH,
                 ("TAPI_DRV: Stopped character device creation on '%s'. "
                  "Reached maximal number(nTapiFdIdx=%d) of file "
                  "descriptors supported by TAPI\n",
                  sDevName, nTapiFdIdx));
            /* terminate node creation */
            nDevIdx = maxDevices;
            break;
         }

         pHLDrvCtx->pLL_Device[nTapiFdIdx] =
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,18))
            class_device_create(pTAPI_Class, MKDEV(majorNumber, minorNumber),
                                IFX_NULL, "%s", sDevName);
#elif (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,26))
            device_create(pTAPI_Class, IFX_NULL,
                          MKDEV(majorNumber, minorNumber),
                          "%s", (IFX_char_t*)sDevName);
#else
            device_create(pTAPI_Class, IFX_NULL,
                          MKDEV(majorNumber, minorNumber), IFX_NULL,
                          "%s", (IFX_char_t*)sDevName);
#endif /* < Linux 2.6.18 */
         if (IS_ERR(pHLDrvCtx->pLL_Device[nTapiFdIdx]))
         {
            TRACE( TAPI_DRV, DBG_LEVEL_HIGH,
                 ("IFX_TAPI_Register_LL_Drv: unable to create class device "
                  "'%s'\n\r", sDevName));
            return TAPI_statusErr;
         }

         nTapiFdIdx++;
      }
   }
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)) */

   return TAPI_statusOk;
}

/**
   UnRegister the low level driver from the system.

   This function will be called when the low-level driver is removed from the
   system. Any OS specific deregistration should be done here.

   For Linux kernels with devfs support device nodes will be unregistered here.
   The number of nodes unregistered depend on the "one device node" or "multiple
   device node" configuration.
   For Linux kernels without devfs support the character driver is unregistered.

   \param  pLLDrvCtx    Pointer to device driver context created by LL-driver.
   \param  pHLDrvCtx    Pointer to high-level driver context.

   \return
   TAPI_statusErr on error, otherwise TAPI_statusOk
*/
IFX_return_t TAPI_OS_UnregisterLLDrv (IFX_TAPI_DRV_CTX_t* pLLDrvCtx,
                                      IFX_TAPI_HL_DRV_CTX_t* pHLDrvCtx)
{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
   unregister_chrdev (pLLDrvCtx->majorNumber, pHLDrvCtx->registeredDrvName);
#else /* >= Linux 2.6.0 */
   int i;
   IFX_uint16_t maxDevices, nDevIdx;

   for (i=0; i < TAPI_MAX_DEVFS_HANDLES; ++i)
   {
      if (pHLDrvCtx->pLL_Device[i])
      {
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,18))
         class_device_destroy(pTAPI_Class, pHLDrvCtx->pLL_Device[i]->devt);
#else
         device_destroy(pTAPI_Class, pHLDrvCtx->pLL_Device[i]->devt);
#endif /* < Linux 2.6.18 */
         pHLDrvCtx->pLL_Device[i] = IFX_NULL;
      }
   }

#ifdef TAPI_ONE_DEVNODE
   /* Single device node interface. */
   maxDevices = 1;
#else /* !defined (TAPI_ONE_DEVNODE) */
   /* Multiple device node interface. */
   maxDevices  = pLLDrvCtx->maxDevs;
#endif /* not TAPI_ONE_DEVNODE */

   for (nDevIdx = 0; nDevIdx < maxDevices; nDevIdx++)
   {
      /* remove character device */
      cdev_del (&(pLLDrvCtx->pTapiDev[nDevIdx].cdev));
   } /* for all devices */

   unregister_chrdev_region (MKDEV(pLLDrvCtx->majorNumber, 0),
                             pHLDrvCtx->nDevNrCount);
#endif

   return TAPI_statusOk;
}

/**
   Open the device.

   At the first time:
   - Initialize the high-level TAPI device structure
   - Call the low-level function to initialise the low-level device structure
   - Initialize the high-level TAPI channel structure
   - Call the low-level function to initialise the low-level channel structure

   \param  inode        Pointer to the inode.
   \param  filp         Pointer to the file descriptor.

   \return
   0 - if no error,
   otherwise error code
*/
static int ifx_tapi_open (struct inode *inode, struct file *filp)
{
   TAPI_FD_PRIV_DATA_t     *pTapiPriv  = IFX_NULL;
   IFX_TAPI_DRV_CTX_t      *pDrvCtx    = IFX_NULL;
   TAPI_DEV                *pTapiDev   = IFX_NULL;
#if !defined(TAPI_ONE_DEVNODE)
   TAPI_CHANNEL            *pTapiCh    = IFX_NULL;
#endif /* not TAPI_ONE_DEVNODE */
   IFX_uint32_t            nDev = 0,
                           nCh = 0;
   IFX_uint32_t            majorNum = 0;
   IFX_uint32_t            minorNum = 0;

   majorNum = MAJOR(inode->i_rdev);
   minorNum = MINOR(inode->i_rdev);
   TRACE (TAPI_DRV, DBG_LEVEL_LOW,
         ("ifxTAPI open %d/%d\n", majorNum, minorNum));

   /* Get the pointer to the device driver context based on the major number */
   pDrvCtx = IFX_TAPI_DeviceDriverContextGet (majorNum);

   if (pDrvCtx == IFX_NULL)
   {
      /* This should actually never happen because the file descriptors are
         registered in TAPI_OS_RegisterLLDrv after a driver context is known. */
      printk (KERN_INFO "tapi_open: error finding DrvCtx\n");
      return -ENODEV;
   }

#if !defined(TAPI_ONE_DEVNODE)
   if (pDrvCtx->maxDevs == 1)
   {
      /* Extended fd numbering scheme for single device only. */
      /* If only one device is supported allow more than 9 channel fd. */
      nDev = 0;
      nCh = (IFX_uint32_t)minorNum - pDrvCtx->minorBase;
   }
   else
   {
      /* Regular fd numbering scheme for multiple devices. */
      /* calculate the device number and channel number */
      nDev = ((IFX_uint32_t)minorNum / pDrvCtx->minorBase) - 1;
      nCh   = ((IFX_uint32_t)minorNum % pDrvCtx->minorBase);

      /* check the device number */
      if (nDev >= pDrvCtx->maxDevs)
      {
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
              ("TAPI_DRV: max. device number exceed\n"));
         return -ENODEV;
      }
   }

   /* check the channel number */
   if (nCh > pDrvCtx->maxChannels)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
          ("TAPI_DRV: max. channel number exceed\n"));
      return -ENODEV;
   }
#endif /* not TAPI_ONE_DEVNODE */

   pTapiDev = pDrvCtx->pTapiDev + nDev;

   /* Allocate memory for a new TAPI context wrapper structure */
   pTapiPriv = (TAPI_FD_PRIV_DATA_t* )
               TAPI_OS_Malloc (sizeof (TAPI_FD_PRIV_DATA_t));

   if (pTapiPriv == IFX_NULL)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
            ("Unable to allocate memory for TAPI private data. "
             "[dev:%d ch:%d]\n", nDev, nCh));
      return -ENOMEM;
   }

#if !defined(TAPI_ONE_DEVNODE)
   if (nCh == 0)
#endif /* not TAPI_ONE_DEVNODE */
   {
      pTapiPriv->pTapiCtx = pTapiDev;
   }
#if !defined(TAPI_ONE_DEVNODE)
   else
   {
      pTapiCh = pTapiDev->pChannel + nCh - 1;

      pTapiPriv->pTapiCtx = pTapiCh;
   }
#endif /* not TAPI_ONE_DEVNODE */

   pTapiPriv->fifo_idx = IFX_TAPI_STREAM_COD;
   filp->private_data = pTapiPriv;

   /* Call the Low level Device specific open routine */
   if (IFX_TAPI_PtrChk (pDrvCtx->Open))
   {
      IFX_int32_t retLL;

      retLL = pDrvCtx->Open (pTapiPriv->pTapiCtx);

      if (!TAPI_SUCCESS(retLL))
      {
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
               ("Open LL channel failed. [dev:%d ch:%d]\n", nDev, nCh));

         TAPI_OS_Free (pTapiPriv);

         filp->private_data = IFX_NULL;
         return -ENODEV;
      }
   }

   /* increment the use counter */
   pTapiDev->nInUse++;

#if !defined(TAPI_ONE_DEVNODE)
   /* increment the use counters */
   if (IFX_NULL != pTapiCh)
   {
      pTapiCh->nInUse++;
   }
#endif /* not TAPI_ONE_DEVNODE */

   /* increment module use counter */
#ifdef MODULE
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
   MOD_INC_USE_COUNT;
#endif /* (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)) */
#endif /* MODULE */

   return 0;
}


/**
   Close a file.

   This function gets called when a close is called on the file descriptor.
   Both types device and channel file descriptors are handled here. The
   function decrements the usage count, and calls the LL release routine.

   \param  inode        Pointer to the inode.
   \param  filp         Pointer to the file descriptor.

   \return
   0 - if no error,
   otherwise error code
*/
static int ifx_tapi_release (struct inode *inode, struct file *filp)
{
   /* nCh is the first field in both TAPI_DEV and TAPI_CHANNEL */
   TAPI_FD_PRIV_DATA_t *pTapiPriv = IFX_NULL;
   IFX_uint8_t          nCh = 0;
   IFX_TAPI_DRV_CTX_t  *pDrvCtx = IFX_NULL;
   TAPI_DEV            *pTapiDev = IFX_NULL;
   TAPI_CHANNEL        *pTapiCh = IFX_NULL;
   IFX_TAPI_LL_CH_t    *pLLChDev = IFX_NULL;

   if (IFX_NULL == filp->private_data)
   {
      /* device was already released */
      return -ENODEV;
   }

   pTapiPriv = (TAPI_FD_PRIV_DATA_t* )filp->private_data;
   nCh = ((TAPI_DEV *) pTapiPriv->pTapiCtx)->nChannel;

   TRACE(TAPI_DRV, DBG_LEVEL_LOW, ("ifxTAPI close %d/%d tapi-ch %d\n\r",
         MAJOR(inode->i_rdev), MINOR(inode->i_rdev), nCh));

   if (nCh != IFX_TAPI_DEVICE_CH_NUMBER)
   {
      /* closing channel file descriptor */
      pTapiCh = (TAPI_CHANNEL *) pTapiPriv->pTapiCtx;

      pTapiDev = pTapiCh->pTapiDevice;
      pLLChDev = pTapiCh->pLLChannel;

   }
   else
   {
      /* closing device file descriptor */
      pTapiDev = (TAPI_DEV *) pTapiPriv->pTapiCtx;
      pLLChDev = pTapiDev->pLLDev;
   }

   if ((IFX_NULL == pTapiDev) || (IFX_NULL == pTapiDev->pDevDrvCtx))
   {
      /* resource already removed, nothing to do for release */
      /* FIXME: the driver should not be unloaded while file descriptor is opened */
      return 0;
   }

   pDrvCtx = pTapiDev->pDevDrvCtx;

   /* Call the Low-level Device specific release routine. */
   /* Not having such a function is not an error. */
   if (IFX_TAPI_PtrChk (pDrvCtx->Release))
   {
      IFX_int32_t retLL;

      retLL = pDrvCtx->Release (pLLChDev);

      if (!TAPI_SUCCESS (retLL))
      {
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
               ("Release LL channel failed for ch: %d\n", nCh));
         return -EBUSY;
      }
   }

   /* decrement the use counters */
   if ((IFX_NULL != pTapiCh) && (pTapiCh->nInUse > 0))
   {
      pTapiCh->nInUse--;
   }

   /* decrement the use counter */
   if (pTapiDev->nInUse > 0)
   {
      pTapiDev->nInUse--;
   }

   /* decrement use counter */
#ifdef MODULE
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
   MOD_DEC_USE_COUNT;
#endif /* (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)) */
#endif /* MODULE */

   /* free private data */
   TAPI_OS_Free (pTapiPriv);

   filp->private_data = IFX_NULL;

   return 0;
}


/**
   Writes data to the device.

   \param  filp         Pointer to the file descriptor.
   \param  buf          Pointer to data to be written.
   \param  count        Data length.
   \param  ppos         unused.

   \return
   length or a negative error code
   \remarks
   Currently the low-level write function is called transparently
*/
static ssize_t ifx_tapi_write (struct file *filp, const char *buf,
                               size_t count, loff_t * ppos)
{
#ifdef TAPI_FEAT_PACKET
   TAPI_FD_PRIV_DATA_t *pTapiPriv = (TAPI_FD_PRIV_DATA_t* )filp->private_data;
   TAPI_CHANNEL        *pTapiCh = IFX_NULL;
   TAPI_DEV            *pTapiDev = IFX_NULL;
   IFX_TAPI_DRV_CTX_t  *pDrvCtx = IFX_NULL;
   IFX_TAPI_STREAM_t    fifo_idx;
   IFX_uint8_t         *pData = IFX_NULL;
   IFX_uint32_t         buf_size;
   IFX_ssize_t          size = 0;

   if ((IFX_NULL == pTapiPriv) || (IFX_NULL == pTapiPriv->pTapiCtx))
   {
      /* resource removed */
      /* FIXME: the driver should not be unloaded while file descriptor is opened */
      return (ssize_t)-ENODEV;
   }

   fifo_idx = pTapiPriv->fifo_idx;
   pTapiCh = (TAPI_CHANNEL* )pTapiPriv->pTapiCtx;

   /* Write to device file descriptor is not allowed. */
   if (((TAPI_DEV*) pTapiPriv->pTapiCtx)->nChannel == IFX_TAPI_DEVICE_CH_NUMBER)
   {
      return -EFAULT;
   }

   if ((IFX_NULL == pTapiCh->pTapiDevice) || (IFX_NULL == pTapiCh->pTapiDevice->pDevDrvCtx))
   {
      /* resource removed */
      /* FIXME: the driver should not be unloaded while file descriptor is opened */
      return (ssize_t)-ENODEV;
   }

   pTapiDev = pTapiCh->pTapiDevice;
   pDrvCtx = pTapiDev->pDevDrvCtx;

   if (pTapiDev->bInitialized == IFX_FALSE)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_LOW,
            ("TAPI_DRV: write failed because device is not initialised\n"));
      return -EFAULT;
   }


   if (!IFX_TAPI_PtrChk (pDrvCtx->Write))
   {
      IFX_TAPI_Stat_Add(pTapiCh, fifo_idx,
                        TAPI_STAT_COUNTER_INGRESS_DISCARDED, 1);
      TRACE(TAPI_DRV, DBG_LEVEL_LOW,
            ("TAPI_DRV: LL-driver does not provide packet write\n"));
      return -EFAULT;
   }

   /* Truncate data to the size of our voice buffer. */
   buf_size = IFX_TAPI_VoiceBufferPool_ElementSizeGet();
   buf_size -= pDrvCtx->pktBufPrependSpace;
   if ((IFX_uint32_t)count < buf_size)
   {
      buf_size = (IFX_uint32_t)count;
   }

   /* Get a buffer from the voice buffer pool. */
   if ((pData = IFX_TAPI_VoiceBufferGet()) == IFX_NULL)
   {
      /* voice buffer pool is depleted */
      IFX_TAPI_Stat_Add(pTapiCh, fifo_idx,
                        TAPI_STAT_COUNTER_INGRESS_DISCARDED, 1);
      return -EFAULT;
   }

   /* if you have time and want clean buffers you might like this */
   /* memset(pData, 0, buf_size); */

   /* Copy data from userspace into kernelspace buffer. */
   if (!TAPI_OS_CpyUsr2Kern (
         (void *)(pData + pDrvCtx->pktBufPrependSpace),
         (void *)buf, buf_size))
   {
      /* copy into kernel space failed */
      IFX_TAPI_Stat_Add(pTapiCh, fifo_idx,
                        TAPI_STAT_COUNTER_INGRESS_DISCARDED, 1);
      IFX_TAPI_VoiceBufferPut(pData);
      return -EFAULT;
   }

   /* Call the Low level driver's write function. */
   size = pDrvCtx->Write (pTapiCh->pLLChannel, pData, (IFX_int32_t)buf_size,
                          (IFX_int32_t*)(IFX_void_t*)ppos, fifo_idx);

   if (size == IFX_ERROR)
   {
      /* Low level driver's write failed */
      IFX_TAPI_Stat_Add(pTapiCh, fifo_idx,
                        TAPI_STAT_COUNTER_INGRESS_DISCARDED, 1);
      IFX_TAPI_VoiceBufferPut(pData);
      return -EFAULT;
   }

   return (ssize_t) size;
#else
   IFX_UNUSED (filp);
   IFX_UNUSED (buf);
   IFX_UNUSED (count);
   IFX_UNUSED (ppos);

   return (ssize_t) 0;
#endif /* TAPI_FEAT_PACKET */
}


/**
   Reads data from the device.

   \param  filp         Pointer to the file descriptor.
   \param  buf          Pointer to buffer that receives the data.
   \param  count        Max size of data to read in bytes.
   \param  ppos         unused.

   \return
   Number of bytes read successfully into the buffer.
*/
static ssize_t ifx_tapi_read(struct file *filp,
                             char *buf, size_t count, loff_t * ppos)
{
#ifdef TAPI_FEAT_PACKET
   TAPI_FD_PRIV_DATA_t *pTapiPriv = (TAPI_FD_PRIV_DATA_t* )filp->private_data;
   TAPI_CHANNEL        *pTapiCh = IFX_NULL;
   TAPI_DEV            *pTapiDev = IFX_NULL;
   IFX_TAPI_DRV_CTX_t  *pDrvCtx = IFX_NULL;
   IFX_TAPI_STREAM_t    fifo_idx;
   IFX_void_t const    *pPacket   = NULL;
   IFX_uint32_t         nOffset;
   IFX_uint32_t         size = 0;

   IFX_UNUSED (ppos);

   if ((IFX_NULL == pTapiPriv) || (IFX_NULL == pTapiPriv->pTapiCtx))
   {
      /* resource removed */
      /* FIXME: the driver should not be unloaded while file descriptor is opened */
      return (ssize_t)-ENODEV;
   }

   fifo_idx = pTapiPriv->fifo_idx;
   pTapiCh = (TAPI_CHANNEL* )pTapiPriv->pTapiCtx;

   if ((IFX_NULL == pTapiCh->pTapiDevice) || (IFX_NULL == pTapiCh->pTapiDevice->pDevDrvCtx))
   {
      /* resource removed */
      /* FIXME: the driver should not be unloaded while file descriptor is opened */
      return (ssize_t)-ENODEV;
   }

   pTapiDev = pTapiCh->pTapiDevice;
   pDrvCtx = pTapiDev->pDevDrvCtx;

   if (pTapiDev->bInitialized == IFX_FALSE)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_LOW,
            ("TAPI_DRV: read failed because device is not initialised\n"));
      return -EFAULT;
   }

   if (fifo_idx >= IFX_TAPI_STREAM_MAX)
   {
      /* index out of range - internal error */
      TRACE (TAPI_DRV, DBG_LEVEL_LOW, ("INFO: FIFO index out of range\n"));
      /* return linux error code to the application */
      return -EINVAL;
   }

   /* All packets to be read are already in the channel specific upstream
      fifo of the driver. They are put there by whoever receives packets.
      So all that is done here operates on this fifo and does not need to call
      any ll-driver function any more. */

   /* check for non blocking flag */
   if (filp->f_flags & O_NONBLOCK)
      pTapiCh->nFlags |= CF_NONBLOCK;

   /* If FIFO is empty or does not exist we cannot read anything. In this
      case we either wait until data arrives or return if non-blocking.
      If the FIFO does not exist no data will ever arrive and the possible
      wait will last forever. */
   if (IFX_TAPI_UpStreamFifo_IsEmpty(pTapiCh, fifo_idx) == IFX_TRUE)
   {
      /* check the non-blocking flag */
      if (pTapiCh->nFlags & CF_NONBLOCK)
      {
         /* this is a non blocking read call - return immediately */
         return 0;
      }

      /* this is a blocking read call so go to sleep now */
      if (TAPI_OS_EventWait (&pTapiCh->semReadBlock,
                             TAPI_OS_WAIT_FOREVER, IFX_NULL) == IFX_ERROR)
      {
         /* timeout has expired without arrival of data */
         return 0;
      }

      /* we have been woken because data has arrived */
   }

   /* read data from the fifo */
   pPacket = IFX_TAPI_UpStreamFifo_Get(pTapiCh, fifo_idx, &size, &nOffset);

   /* sanity check on the received packet */
   if (pPacket == NULL)
   {
      /* packet may not be NULL - internal error */
      TRACE (TAPI_DRV, DBG_LEVEL_LOW, ("INFO: pPacket Null\n"));
      return 0;
   }
   if (size > (IFX_uint32_t)count)
   {
      /* output buffer not large enough for data */
      /* drop the packet */
      IFX_TAPI_VoiceBufferPut((IFX_void_t *)pPacket);
      /* update statistic */
      IFX_TAPI_Stat_Add(pTapiCh, fifo_idx,
                        TAPI_STAT_COUNTER_EGRESS_DISCARDED, 1);
      /* return linux error code to the application */
      return -EINVAL;
   }

   /* unmap data */
   TAPI_OS_CpyKern2Usr (buf, ((IFX_char_t *)pPacket + nOffset),
                        (IFX_uint32_t)size);

   /* return buffer back to the pool now that is has been copied */
   IFX_TAPI_VoiceBufferPut((IFX_void_t *)pPacket);
   /* update statistic */
   IFX_TAPI_Stat_Add(pTapiCh, fifo_idx,
                     TAPI_STAT_COUNTER_EGRESS_DELIVERED, 1);

   return (ssize_t) size;
#else
   IFX_UNUSED (filp);
   IFX_UNUSED (buf);
   IFX_UNUSED (count);
   IFX_UNUSED (ppos);

   return (ssize_t) 0;
#endif /* TAPI_FEAT_PACKET */
}

/**
   Poll implementation for the device.

   \param file pointer to the file descriptor
   \param wait pointer to the poll table

   \return
    status of the events occured on the device which are
    0, TAPI_OS_SYSEXCEPT, TAPI_OS_SYSREAD, TAPI_OS_SYSWRITE
   \remarks
   This function does the following functions:
      - Put the event queue in the wait table and sleep if select
        is called on the device itself.
      - Put the read/write queue in the wait table and sleep if select
        is called on the channels
*/
static IFX_uint32_t ifx_tapi_poll (struct file *filp, poll_table *wait)
{
   TAPI_FD_PRIV_DATA_t *pTapiPriv = (TAPI_FD_PRIV_DATA_t* )filp->private_data;
   TAPI_DEV            *pTapiDev = IFX_NULL;
   IFX_uint16_t         i;
#ifdef TAPI_ONE_DEVNODE
   IFX_uint16_t         j;
   IFX_TAPI_DRV_CTX_t*  pDrvCtx = IFX_NULL;
#endif /* TAPI_ONE_DEVNODE */
   IFX_int32_t          ret = 0;

   /* check device ptr */
   if ((IFX_NULL == pTapiPriv) || (IFX_NULL == pTapiPriv->pTapiCtx))
   {
      /* resource removed */
      /* FIXME: the driver should not be unloaded while file descriptor is opened */
      return (IFX_uint32_t)-ENODEV;
   }

   pTapiDev  = (TAPI_DEV* )pTapiPriv->pTapiCtx;

   if (IFX_NULL == pTapiDev->pDevDrvCtx)
   {
      /* resource removed */
      /* FIXME: the driver should not be unloaded while file descriptor is opened */
      return (IFX_uint32_t)-ENODEV;
   }

#ifdef TAPI_ONE_DEVNODE
   /* get driver context in case select called on device file descriptor */
   if (pTapiDev->nChannel == IFX_TAPI_DEVICE_CH_NUMBER)
   {
      pDrvCtx = IFX_TAPI_DeviceDriverContextGet (
         pTapiDev->pDevDrvCtx->majorNumber);
   }

   if (pTapiDev->nChannel == IFX_TAPI_DEVICE_CH_NUMBER &&
       pDrvCtx != IFX_NULL)
   {
      for (j= 0;j < pDrvCtx->maxDevs; ++j)
      {
         pTapiDev = &pDrvCtx->pTapiDev[j];

         TAPI_OS_DrvSelectQueueAddTask (filp, &pTapiDev->wqEvent, wait);

         if (pTapiDev->pChannel == IFX_NULL)
            continue;

         for (i = 0; i < pTapiDev->nMaxChannel; i++)
         {
            TAPI_CHANNEL *pTapiCh = &(pTapiDev->pChannel[i]);

            TAPI_OS_DrvSelectQueueAddTask (filp, &pTapiCh->semReadBlock.object,
                                           wait);
            TAPI_OS_DrvSelectQueueAddTask (filp, &pTapiCh->wqRead, wait);

            if ((pTapiCh->bInitialized == IFX_TRUE) &&
                (IFX_TAPI_EventFifoEmpty(pTapiCh) == IFX_FALSE))
            {
               /* exception available so return action */
               ret |= TAPI_OS_SYSREAD;
               break;
            }
         }
      }
   }
   else
#endif /* TAPI_ONE_DEVNODE */
   {
      if (pTapiDev->nChannel == IFX_TAPI_DEVICE_CH_NUMBER)
      {
         /* Register the TAPI-event waitqueue as wakeup source. */
         TAPI_OS_DrvSelectQueueAddTask (filp, &pTapiDev->wqEvent, wait);

         /* Check if there is any TAPI-event on any of the TAPI channels and
            return if file operations are possible without blocking. */

         TAPI_ASSERT (pTapiDev->pChannel != IFX_NULL);

         for (i = 0; i < pTapiDev->nMaxChannel; i++)
         {
            TAPI_CHANNEL *pTapiCh = &(pTapiDev->pChannel[i]);
            if ((pTapiCh->bInitialized == IFX_TRUE) &&
                (IFX_TAPI_EventFifoEmpty(pTapiCh) == IFX_FALSE))
            {
               /* TAPI-event available so return action */
               ret |= TAPI_OS_SYSREAD;
               break;
            }
         }
      }
      else
      {
         TAPI_CHANNEL *pTapiCh = (TAPI_CHANNEL *)pTapiDev;
         pTapiDev  = pTapiCh->pTapiDevice;

         if (pTapiCh->nChannel < pTapiDev->nMaxChannel)
         {
            /* Check if any packet is available in any of the upstream fifos
               and report if file operations are possible without blocking. */
            ret |= TAPI_SelectCh (pTapiPriv,
                                  (TAPI_OS_drvSelectTable_t *)wait,
                                  (TAPI_OS_drvSelectOSArg_t *)filp);
         }
      }
   }

   return (IFX_uint32_t)ret;
}

/**
   Configuration / Control for the device.

   \param  inode        Pointer to the inode.
   \param  filp         Pointer to the file descriptor.
   \param  nCmd         IOCTL identifier.
   \param  nArg         Optional argument.

   \return
   0 and positive values - success,
   negative value - ioctl failed
   \remarks
   This function does the following functions:
      - If the ioctl command is device specific, low-level driver's ioctl function
      - If the ioctl command is TAPI specific, it is handled at this level
*/
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36))
static int ifx_tapi_ioctl(struct inode *inode, struct file *filp,
                          unsigned int nCmd, unsigned long nArg)
#else
static long ifx_tapi_ioctl(struct file *filp,
                          unsigned int nCmd, unsigned long nArg)
#endif
{
   IFX_TAPI_DRV_CTX_t *pDrvCtx = IFX_NULL;
   IFX_TAPI_ioctlCtx_t ctx;
   IFX_int32_t ret;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,36))
   struct inode *inode = filp->f_dentry->d_inode;
#endif

   IFX_UNUSED (filp);

   /* get the device driver context */
   pDrvCtx = IFX_TAPI_DeviceDriverContextGet (MAJOR(inode->i_rdev));
   if (pDrvCtx == IFX_NULL)
   {
      return IFX_ERROR;
   }

   /* get the ioctl context: channel, device etc. */
   ret = TAPI_ioctlContextGet (pDrvCtx, MINOR(inode->i_rdev),
      nCmd, nArg, IFX_TRUE, &ctx);
   if (IFX_SUCCESS == ret)
   {
      ret = TAPI_Ioctl (&ctx);
   }

   TAPI_ioctlContextPut (nArg, &ctx);

   return ret;
}


#ifdef TAPI_FEAT_PROCFS
/**
   Read the version information from the driver.

   \param  s

   \return
   none
*/
static IFX_void_t proc_get_tapi_version(struct seq_file *s)
{
   seq_printf(s, "%s\n", &TAPI_WHATVERSION[4]);
   seq_printf(s, "Compiled on %s, %s for Linux kernel %s\n",
      __DATE__, __TIME__, UTS_RELEASE);
}
#endif /* TAPI_FEAT_PROCFS */


#ifdef TAPI_FEAT_PROCFS
#ifdef HAVE_CONFIG_H
/**
   Read the configure parameters of the driver.

   \param  s

   \return
   none
*/
static IFX_void_t proc_ConfigureGet(struct seq_file *s)
{
   seq_printf(s, "configure %s\n", &DRV_TAPI_WHICHCONFIG[0]);
}
#endif /* HAVE_CONFIG_H */
#endif /* TAPI_FEAT_PROCFS */


#ifdef TAPI_FEAT_PROCFS
/**
   Read the status information from the driver.

   \param  s

   \return
   none
*/
static IFX_void_t proc_get_tapi_status(struct seq_file *s)
{
   IFX_TAPI_DRV_CTX_t *pDrvCtx  = IFX_NULL;
   TAPI_DEV           *pTapiDev = IFX_NULL;
   IFX_int32_t i, j;

   for (i = 0; i < TAPI_MAX_LL_DRIVERS; i++)
   {
      if (gHLDrvCtx [i].pDrvCtx == IFX_NULL)
      {
         continue;
      }

      pDrvCtx = gHLDrvCtx [i].pDrvCtx;
      seq_printf(s, "-- %s --\n", pDrvCtx->drvName);

      if (pDrvCtx->pTapiDev == IFX_NULL)
      {
         /* driver not initialised yet -> skip it */
         break;
      }

      for (j = 0; j < pDrvCtx->maxDevs; j++)
      {
         pTapiDev = &(pDrvCtx->pTapiDev[j]);
         seq_printf(s, "  Device #%d: Initialized = %d\n",
            j, pTapiDev->bInitialized);
      }
   }
}
#endif /* TAPI_FEAT_PROCFS */


#ifdef TAPI_FEAT_PROCFS
/**
   Read the registered low level driver information

   \param  s

   \return
   none
*/
static IFX_void_t proc_get_tapi_registered_drivers(struct seq_file *s)
{
   IFX_TAPI_DRV_CTX_t *pDrvCtx = IFX_NULL;
   IFX_int32_t i;

   seq_printf(s, "%-10s %-10s %-10s %7s %10s\n",
                  "LL-Driver", "devname", "version", "major", "devices");
   seq_printf(s, "===================================================\n");

   for (i = 0; i < TAPI_MAX_LL_DRIVERS; i++)
   {
      if (gHLDrvCtx [i].pDrvCtx != IFX_NULL)
      {
         pDrvCtx = gHLDrvCtx [i].pDrvCtx;
         seq_printf(s, "%-10s %-10s %-10s %7d %10d\n",
            pDrvCtx->drvName, pDrvCtx->devNodeName, pDrvCtx->drvVersion,
            pDrvCtx->majorNumber,pDrvCtx->maxDevs);
      }
   }

#ifdef TAPI_FEAT_QOS
   /* Print details about the QOS driver that is registered with TAPI. */
   seq_printf(s, "\nQOS driver:\n");
   IFX_TAPI_QOS_proc_read_registration(s);
#endif /* TAPI_FEAT_QOS */
}
#endif /* TAPI_FEAT_PROCFS */


#ifdef TAPI_FEAT_PROCFS
/**
   Read the current bufferpool status

   \param  s

   \return
   none
*/
static IFX_void_t proc_read_bufferpool (struct seq_file *s)
{
   seq_printf(s, "TAPIpool (free/total): "
#ifdef TAPI_FEAT_PACKET
                  "voice (%4d/%4d), "
#endif /* TAPI_FEAT_PACKET */
                  "event (%4d/%4d)\n",
#ifdef TAPI_FEAT_PACKET
                  IFX_TAPI_VoiceBufferPool_ElementAvailCountGet(),
                  IFX_TAPI_VoiceBufferPool_ElementCountGet(),
#endif /* TAPI_FEAT_PACKET */
                  IFX_TAPI_EventWrpBufferPool_ElementAvailCountGet(),
                  IFX_TAPI_EventWrpBufferPool_ElementCountGet());
}
#endif /* TAPI_FEAT_PROCFS */


#ifdef TAPI_FEAT_PROCFS
#ifdef TAPI_PACKET_OWNID
/**
   Print out to the console voice buffer information

   \param  s

   \return
      none.

   \remarks
      That function does not use s because output too large for
      **non-circled** buffer with **unknown** size.
*/
static IFX_void_t proc_read_voice_bufferpool (struct seq_file *s)
{
   IFX_TAPI_VoiceBufferPoolStatusShow();
}
#endif /* TAPI_PACKET_OWNID */
#endif /* TAPI_FEAT_PROCFS */


#ifdef TAPI_FEAT_PROCFS
#ifdef TAPI_FEAT_STATISTICS
/**
   Read the current fifo status

   \param   s
   \param   pos     Device number starting from 1

   \return  zero or an error code
*/
static int proc_read_tapi_stats (struct seq_file *s, int pos)
{
   IFX_TAPI_DRV_CTX_t *pDrvCtx  = IFX_NULL;
   TAPI_DEV           *pTapiDev = IFX_NULL;
   TAPI_CHANNEL       *pChannel = IFX_NULL;
   IFX_int32_t        i, j, k, nDeviceCounter = 0;
   IFX_boolean_t      bDeviceFound = IFX_FALSE;

   /* pos = 0 means print header, but this procfs entry has no header */
   if (pos <= 0)
      return 0;

   /* Check all slots for registered drivers */
   for (i = 0; i < TAPI_MAX_LL_DRIVERS; i++)
   {
      if (bDeviceFound)
         break;

      pDrvCtx = gHLDrvCtx[i].pDrvCtx;

      if (pDrvCtx == IFX_NULL)
      {
         continue;
      }

      if (pDrvCtx->pTapiDev == IFX_NULL)
      {
         /* driver not initialised yet -> skip it */
         break;
      }

      /* Loop over all devices of the registered driver */
      for (j = 0; j < pDrvCtx->maxDevs; j++)
      {
         pTapiDev = &(pDrvCtx->pTapiDev[j]);

         if (++nDeviceCounter < pos)
            continue;
         bDeviceFound = IFX_TRUE;

         if (!pTapiDev->bInitialized)
         {
            continue;
         }

         /* Headline */
         seq_printf(s, "-- %s #%d --\n", pDrvCtx->drvName, j);
         seq_printf(s, "CH STRM "
            "TX:     ok  discarded  congested "
            "RX:     ok  discarded  congested\n");

         /* Loop over all TAPI channels of this device */
         for (k=0; k < pDrvCtx->maxChannels; k++)
         {
            pChannel = &pTapiDev->pChannel[k];

            if (k < pChannel->pTapiDevice->nResource.CodCount)
            {
               seq_printf(s, "%2d COD  "
                  "%10u %10u %10u "
                  "%10u %10u %10u\n", k,
                  IFX_TAPI_Stat_Get(pChannel, IFX_TAPI_STREAM_COD,
                                    TAPI_STAT_COUNTER_EGRESS_DELIVERED),
                  IFX_TAPI_Stat_Get(pChannel, IFX_TAPI_STREAM_COD,
                                    TAPI_STAT_COUNTER_EGRESS_DISCARDED),
                  IFX_TAPI_Stat_Get(pChannel, IFX_TAPI_STREAM_COD,
                                    TAPI_STAT_COUNTER_EGRESS_CONGESTED),
                  IFX_TAPI_Stat_Get(pChannel, IFX_TAPI_STREAM_COD,
                                    TAPI_STAT_COUNTER_INGRESS_DELIVERED),
                  IFX_TAPI_Stat_Get(pChannel, IFX_TAPI_STREAM_COD,
                                    TAPI_STAT_COUNTER_INGRESS_DISCARDED),
                  IFX_TAPI_Stat_Get(pChannel, IFX_TAPI_STREAM_COD,
                                    TAPI_STAT_COUNTER_INGRESS_CONGESTED));
            }

            if (k < pChannel->pTapiDevice->nResource.DectCount)
            {
               seq_printf(s, "%2d DECT "
                  "%10u %10u %10u "
                  "%10u %10u %10u\n", k,
                  IFX_TAPI_Stat_Get(pChannel, IFX_TAPI_STREAM_DECT,
                                    TAPI_STAT_COUNTER_EGRESS_DELIVERED),
                  IFX_TAPI_Stat_Get(pChannel, IFX_TAPI_STREAM_DECT,
                                    TAPI_STAT_COUNTER_EGRESS_DISCARDED),
                  IFX_TAPI_Stat_Get(pChannel, IFX_TAPI_STREAM_DECT,
                                    TAPI_STAT_COUNTER_EGRESS_CONGESTED),
                  IFX_TAPI_Stat_Get(pChannel, IFX_TAPI_STREAM_DECT,
                                    TAPI_STAT_COUNTER_INGRESS_DELIVERED),
                  IFX_TAPI_Stat_Get(pChannel, IFX_TAPI_STREAM_DECT,
                                    TAPI_STAT_COUNTER_INGRESS_DISCARDED),
                  IFX_TAPI_Stat_Get(pChannel, IFX_TAPI_STREAM_DECT,
                                    TAPI_STAT_COUNTER_INGRESS_CONGESTED));
            }

#ifdef TAPI_FEAT_HDLC
            if (k < pChannel->pTapiDevice->nResource.HdlcCount)
            {
               seq_printf(s, "%2d HDLC "
                  "%10u %10u %10u "
                  "%10u %10u %10u\n", k,
                  IFX_TAPI_Stat_Get(pChannel, IFX_TAPI_STREAM_HDLC,
                                    TAPI_STAT_COUNTER_EGRESS_DELIVERED),
                  IFX_TAPI_Stat_Get(pChannel, IFX_TAPI_STREAM_HDLC,
                                    TAPI_STAT_COUNTER_EGRESS_DISCARDED),
                  IFX_TAPI_Stat_Get(pChannel, IFX_TAPI_STREAM_HDLC,
                                    TAPI_STAT_COUNTER_EGRESS_CONGESTED),
                  IFX_TAPI_Stat_Get(pChannel, IFX_TAPI_STREAM_HDLC,
                                    TAPI_STAT_COUNTER_INGRESS_DELIVERED),
                  IFX_TAPI_Stat_Get(pChannel, IFX_TAPI_STREAM_HDLC,
                                    TAPI_STAT_COUNTER_INGRESS_DISCARDED),
                  IFX_TAPI_Stat_Get(pChannel, IFX_TAPI_STREAM_HDLC,
                                    TAPI_STAT_COUNTER_INGRESS_CONGESTED));
            }
#endif /* TAPI_FEAT_HDLC */
         }
         break;
      }
   }

   return 0;
}
#endif /* TAPI_FEAT_STATISTICS */
#endif /* TAPI_FEAT_PROCFS */


#ifdef TAPI_FEAT_PROCFS
#ifdef TAPI_FEAT_STATISTICS
/**
   Reset all the packet path statistic information of the driver.

   \param   file    file structure for proc file
   \param   buffer  buffer holding the data
   \param   count   number of characters in buffer
   \param   data    unused
   \return  count   Number of processed characters
*/
ssize_t proc_write_tapi_stats(struct file *file, const char __user *buffer,
   size_t count, loff_t *data)
{
   IFX_TAPI_DRV_CTX_t *pDrvCtx  = IFX_NULL;
   TAPI_DEV           *pTapiDev = IFX_NULL;
   TAPI_CHANNEL       *pChannel = IFX_NULL;
   IFX_uint16_t       i, j, k;

   IFX_UNUSED(file);
   IFX_UNUSED(buffer);
   IFX_UNUSED(data);

   for (i = 0; i < TAPI_MAX_LL_DRIVERS; i++)
   {
      pDrvCtx = gHLDrvCtx[i].pDrvCtx;

      if (pDrvCtx == IFX_NULL)
      {
         continue;
      }

      /* Loop over all devices of the registered driver */
      for (j = 0; j < pDrvCtx->maxDevs; j++)
      {
         if (pDrvCtx->pTapiDev == IFX_NULL)
         {
            /* driver not initialised yet -> skip it */
            break;
         }

         pTapiDev = &(pDrvCtx->pTapiDev[j]);

         if (!pTapiDev->bInitialized)
         {
            continue;
         }

         /* Loop over all TAPI channels of this device */
         for (k=0; k < pDrvCtx->maxChannels; k++)
         {
            pChannel = &pTapiDev->pChannel[k];
            IFX_TAPI_Stat_Reset(pChannel);
         }
      }
   }

   return count;
}
#endif /* TAPI_FEAT_STATISTICS */
#endif /* TAPI_FEAT_PROCFS */


#ifdef TAPI_FEAT_PROCFS
static void *TAPI_seq_start(struct seq_file *s, loff_t *pos)
{
   struct proc_file_entry *p = s->private;

   if (*pos > p->nMaxPos || *pos < 0)
      return NULL;

   /* set current position */
   p->nPos = *pos;

   return *pos ? p : SEQ_START_TOKEN;
}
#endif /* TAPI_FEAT_PROCFS */


#ifdef TAPI_FEAT_PROCFS
static void *TAPI_seq_next(struct seq_file *s, void *v, loff_t *pos)
{
   struct proc_file_entry *p = s->private;

   (*pos)++;
   if (*pos > p->nMaxPos || *pos < 0)
      return NULL;

   /* update current position */
   p->nPos = *pos;

   return p;
}
#endif /* TAPI_FEAT_PROCFS */


#ifdef TAPI_FEAT_PROCFS
static void TAPI_seq_stop(struct seq_file *s, void *v)
{
}
#endif /* TAPI_FEAT_PROCFS */


#ifdef TAPI_FEAT_PROCFS
static int TAPI_seq_show(struct seq_file *s, void *v)
{
   struct proc_file_entry *p = s->private;


   if (v != SEQ_START_TOKEN)
      return p->callback(s, p->nPos);

   /* print header */
   return p->callback(s, 0);
}
#endif /* TAPI_FEAT_PROCFS */


#ifdef TAPI_FEAT_PROCFS
static int TAPI_proc_open(struct inode *inode, struct file *file)
{
   struct seq_file *s;
   struct proc_file_entry *p;
   struct proc_entry *entry;
   int ret;

   ret = seq_open(file, &TAPI_seq_ops);
   if (ret)
      return ret;

   s = file->private_data;
   p = kmalloc(sizeof(*p), GFP_KERNEL);

   if (!p)
   {
      (void)seq_release(inode, file);
      return -ENOMEM;
   }

   entry = PDE_DATA(inode);

   p->callback = entry->callback;
   if (entry->init_callback)
      p->nMaxPos = entry->init_callback();
   else
      p->nMaxPos = 1;

   s->private = p;

   return 0;
}
#endif /* TAPI_FEAT_PROCFS */


#ifdef TAPI_FEAT_PROCFS
static int TAPI_proc_release(struct inode *inode, struct file *file)
{
   struct seq_file *s;

   s = file->private_data;
   kfree(s->private);

   return seq_release(inode, file);
}
#endif /* TAPI_FEAT_PROCFS */


#ifdef TAPI_FEAT_PROCFS
static int TAPI_seq_single_show(struct seq_file *s, void *v)
{
   struct proc_entry *p = s->private;

   p->single_callback(s);
   return 0;
}
#endif /* TAPI_FEAT_PROCFS */


#ifdef TAPI_FEAT_PROCFS
static int TAPI_proc_single_open(struct inode *inode, struct file *file)
{
   return single_open(file, TAPI_seq_single_show, PDE_DATA(inode));
}
#endif /* TAPI_FEAT_PROCFS */


#ifdef TAPI_FEAT_PROCFS
static void TAPI_proc_entry_create(struct proc_dir_entry *parent_node,
              struct proc_entry *proc_entry)
{
   mode_t mode = S_IFREG | S_IRUGO;

   memset(&proc_entry->ops, 0, sizeof(struct file_operations));
   proc_entry->ops.owner = THIS_MODULE;

   if (proc_entry->single_callback)
   {
      proc_entry->ops.open = TAPI_proc_single_open;
      proc_entry->ops.release = single_release;
   } else
   {
      proc_entry->ops.open = TAPI_proc_open;
      proc_entry->ops.release = TAPI_proc_release;
   }

   proc_entry->ops.read = seq_read;
   proc_entry->ops.write = proc_entry->write_function;
   if (proc_entry->write_function != NULL)
      mode |= S_IWUGO;
   proc_entry->ops.llseek = seq_lseek;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,30))
   {
      struct proc_dir_entry *entry;

      entry = create_proc_entry (proc_entry->name, mode, parent_node);
      if (entry)
      {
         entry->proc_fops = &proc_entry->ops;
         entry->data = proc_entry;
         entry->owner = THIS_MODULE;
      }
   }
#else
   proc_create_data(proc_entry->name, mode, parent_node, &proc_entry->ops,
      proc_entry);
#endif
}
#endif /* TAPI_FEAT_PROCFS */


#ifdef TAPI_FEAT_PHONE_DETECTION_PROCFS
/**
   Displays help information about 'phone_detection' entry.

   \param  s            Pointer to sequence file struct.
*/
static IFX_void_t tapi_proc_ppd_read_help(
                        struct seq_file *s)
{
   seq_printf(s,
      "Reading the 'status' node shows the current status of Phone Detection on all devices.\n");
   seq_printf(s,
      "Writing to 'devices/<dev_name>' node allows to enable/disable the Phone Detection.\n");
   seq_printf(s,
      "feature on selected channel(s) of '<dev_name>' device.\n\n");

   seq_printf(s,
      "Valid configuration line can be:\n\n");
   seq_printf(s,
      "{-}{+}<channel>\n\n");
   seq_printf(s,
      "<channel> - channel number\n");
   seq_printf(s,
      "{-} - disable Phone Detection on this channel. If nothing follows this symbol,\n"
      "      it applies to all channels. \n");
   seq_printf(s,
      "{+} - enable Phone Detection on this channel. This is the default action\n"
      "      if none of the signs precedes a channel number. If nothing follows\n"
      "      this symbol, it applies to all channels.\n\n");
   seq_printf(s,
      "Examples of valid cfg lines:\n");
   seq_printf(s,
      "\"+\"  - enable Phone Detection on all channels\n");
   seq_printf(s,
      "\"-\"  - disable Phone Detection on all channels\n");
   seq_printf(s,
      "\"-1\" - disable Phone Detection on channel no 1\n");
   seq_printf(s,
      "\"2\"  - enable Phone Detection on channel no 2\n");
   seq_printf(s,
      "\"+2\" - enable Phone Detection on channel no 2\n\n");

   seq_printf(s,
      "Example of usage:\n");
   seq_printf(s,
      "echo \"-1\" > /proc/driver/tapi/phone_detection/devices/vmmc0\n");
}
#endif /* TAPI_FEAT_PHONE_DETECTION_PROCFS */


#ifdef TAPI_FEAT_PHONE_DETECTION_PROCFS
/**
   Show the status of the phone detection on all devices

   \param  s            Pointer to sequence file struct.
   \param  pos          Device number starting from 1.

   \return 0 in all cases.
*/
static int tapi_proc_ppd_read_status (
                        struct seq_file *s,
                        int pos)
{
   IFX_TAPI_DRV_CTX_t *pDrvCtx = IFX_NULL;
   TAPI_DEV           *pTapiDev = IFX_NULL;
   IFX_int32_t        i, j, k, nDeviceCounter = 0;
   IFX_boolean_t      bDeviceFound = IFX_FALSE;

   if (pos < 0)
      return 0;

   if (pos == 0)
   {
      seq_printf(s,
         "===========================================================================================================\n");
      seq_printf(s, "%10s|%3s|%6s|%4s|%5s|%18s|%5s|%21s|%6s|%6s|%6s|%6s|\n",
          "",   "",       "",   "", "phone",      "",   "act.",         "last",   "cap.",      "",       "",       "");
      seq_printf(s, "%10s|%3s|%6s|%4s|%5s|%18s|%5s|%21s|%6s|%6s|%6s|%6s|\n",
         "dev", "ch", "status", "sm",  "det.", "state",  "timer", "capacity[nF]", "thresh", "T1[s]", "T2[ms]", "T3[ms]");
      seq_printf(s,
         "===========================================================================================================\n");
      return 0;
   }

   for (i = 0; i < TAPI_MAX_LL_DRIVERS; i++)
   {
      if (bDeviceFound)
         break;
      pDrvCtx = gHLDrvCtx[i].pDrvCtx;

      if (pDrvCtx == IFX_NULL)
      {
         continue;
      }

      if (pDrvCtx->pTapiDev == IFX_NULL)
      {
         /* driver not initialised yet -> skip it */
         break;
      }

      /* Loop over all devices of the registered driver */
      for (j = 0; j < pDrvCtx->maxDevs; j++)
      {
         pTapiDev = &(pDrvCtx->pTapiDev[j]);

         if (++nDeviceCounter < pos)
            continue;
         bDeviceFound = IFX_TRUE;
         if (!pTapiDev->bInitialized)
         {
            continue;
         }
         /* channel status */
         /* for the moment limit printout to two channels until the real
            number of channels is known internally */
         for (k = 0; (k < pTapiDev->nMaxChannel) && (k < 2); k++)
         {
            TAPI_CHANNEL *pChannel = pTapiDev->pChannel + k;
            TAPI_PPD_DATA_t    *pTapiPpdData = pChannel->pTapiPpdData;

            if (pTapiPpdData == IFX_NULL)
            {
               continue;
            }
            IFX_TAPI_PPD_proc_read(s, pChannel, j);
         }
      }
      break;
   }

   return 0;
}
#endif /* TAPI_FEAT_PHONE_DETECTION_PROCFS */


#ifdef TAPI_FEAT_PHONE_DETECTION_PROCFS
/**
   Open proc node for write

   \param  inode        Pointer to inode struct.
   \param  file         File structure for proc file.

   \return 0 in all cases.
*/
static int tapi_proc_ppd_wr_open(
                        struct inode *inode,
                        struct file *file)
{
   /* Copy the pointer to the tapi device struct from the inode to the file
      private data. */
   file->private_data = PDE_DATA(inode);

   return 0;
}
#endif /* TAPI_FEAT_PHONE_DETECTION_PROCFS */


#ifdef TAPI_FEAT_PHONE_DETECTION_PROCFS
/**
    Enable/Disable Phone detection functionality.

   \param  file         File structure for proc file.
   \param  buffer       Buffer holding the data.
   \param  count        Number of characters in buffer.
   \param  offset       unused.

   \return count        Number of processed characters
*/
static ssize_t tapi_proc_ppd_wr_device(
                        struct file *file,
                        const char __user *buffer,
                        size_t count,
                        loff_t *offset)
{
   enum { MAX_IN_BUF_SIZE = 100 };
   TAPI_DEV           *pTapiDev = (TAPI_DEV *)file->private_data;
   IFX_char_t *temp;
   IFX_int32_t len = -1;
   IFX_int32_t i;
   IFX_boolean_t bAllCh = IFX_FALSE;
   IFX_int32_t nCh = 0;
   IFX_boolean_t bEnable = IFX_FALSE;

   IFX_UNUSED (offset);

   if (!pTapiDev || !pTapiDev->bInitialized)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
            ("DRV_ERROR: Device not initalised yet.\n"));
      return -EFAULT;
   }

   temp = (IFX_char_t*)TAPI_OS_Malloc (MAX_IN_BUF_SIZE);
   if (temp == IFX_NULL)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
            ("DRV_ERROR: Unable to allocate memory for TAPI private data.\n"));
      return -ENOMEM;
   }

   len = count;
   if (count > MAX_IN_BUF_SIZE)
   {
      len = MAX_IN_BUF_SIZE;
   }

   if (copy_from_user(temp, buffer, (unsigned int) len) != 0)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
            ("DRV_ERROR: Data copy from user space failed.\n"));
      TAPI_OS_Free(temp);
      return -EFAULT;
   }

   /* Because we get string terminated with linefeed (0x0A)
       we terminate it with null terminator. */
   temp[len - 1] = '\0';

   if (tapi_proc_ppd_wr_devParseCfgLine(temp, &bAllCh, &nCh, &bEnable) !=
       IFX_TRUE)
   {
      TAPI_OS_Free(temp);
      return -EFAULT;
   }

   /* Enable/Disable Phone Detection functionality on channel(s) */
   for (i = 0; i < pTapiDev->nMaxChannel; i++)
   {
      TAPI_CHANNEL *pChannel = pTapiDev->pChannel + i;
      TAPI_PPD_DATA_t    *pTapiPpdData = pChannel->pTapiPpdData;

      if (pTapiPpdData == IFX_NULL)
      {
         continue;
      }
      if ((bAllCh == IFX_TRUE) || (nCh == pChannel->nChannel))
      {
         if (bEnable == IFX_TRUE)
            IFX_TAPI_PPD_EnPhoneDetOnCh(pChannel);
         else
            IFX_TAPI_PPD_DisPhoneDetOnCh(pChannel);
      }
   }

   TAPI_OS_Free(temp);
   return len;
}
#endif /* TAPI_FEAT_PHONE_DETECTION_PROCFS */


#ifdef TAPI_FEAT_PHONE_DETECTION_PROCFS
/**
   Parsing input string to extract commands for enabling/disabling Phone
   Detection functionality.

   \param  sCfgLine     Buffer with configuration line.
   \param  bAllCh       Will return IFX_TRUE if action should be performed
                        on all channels, otherwise IFX_FALSE.
   \param  pCh          Returns the channel on which Phone Detection should
                        be changed.
   \param  bEnable      Returns if enable (IFX_TRUE) or disable it (IFX_FALSE)
                        is selected.

   \return IFX_TRUE if configuration line was parsed or IFX_FALSE if not.

   \remarks Valid configuration line can be:

           "{-}{+}<channel>"

   <channel> - channel number

   {-} - disable Phone Detection on this channel. If nothing follows this
         symbol, it applies to all channels.
   {+} - enable Phone Detection on this channel. This is the default action
         if none of the signs precede a channel number. If nothing follows this
         symbol, it applies to all channels.

   Examples of valid cfg lines:
   ----------------------------
   "+" - enable Phone Detection on all channels
   "-" - disable Phone Detection on all channels
   "-1" - disable Phone Detection on channel no 1
   "2" - enable Phone Detection on channel no 2
   "+2" - enable Phone Detection on channel no 2
*/
static IFX_boolean_t tapi_proc_ppd_wr_devParseCfgLine(
                        IFX_char_t* sCfgLine,
                        IFX_boolean_t* bAllCh,
                        IFX_int32_t* pCh,
                        IFX_boolean_t* bEnable)
{
   /** Null terminiating character for strings */
   const IFX_char_t STRING_END = '\0';
   /** Sign for disabling channel(s) */
   const IFX_char_t DISABLE_CHAN = '-';
   /** Sign for enabling channel */
   const IFX_char_t ENABLE_CHAN = '+';

   IFX_char_t* leftover_tmp = IFX_NULL;
   IFX_char_t* start_tmp = IFX_NULL;
   IFX_int32_t ch = 0;

   TAPI_ASSERT (sCfgLine);
   TAPI_ASSERT (bAllCh);
   TAPI_ASSERT (pCh);

   *bEnable = IFX_TRUE;
   *bAllCh = IFX_FALSE;

   if (strlen(sCfgLine) == 1)
   {
      if (*sCfgLine == ENABLE_CHAN)
      {
         /* Enable it on all channels */
         *bEnable = IFX_TRUE;
         *bAllCh = IFX_TRUE;
         return IFX_TRUE;
      }
      else if (*sCfgLine == DISABLE_CHAN)
      {
         /* Disable it on all channels */
         *bEnable = IFX_FALSE;
         *bAllCh = IFX_TRUE;
          return IFX_TRUE;
      }
   }

   if (*sCfgLine == DISABLE_CHAN)
   {
      /* Will disable channel */
      *bEnable = IFX_FALSE;
      start_tmp = sCfgLine + 1;
   }
   else if (*sCfgLine == ENABLE_CHAN)
   {
      /* Will enable channel */
      *bEnable = IFX_TRUE;
      start_tmp = sCfgLine + 1;
   }
   else
   {
      start_tmp = sCfgLine;
   }

   /* ---------------     extract channel     ------------------- */

   ch = simple_strtol(start_tmp, &leftover_tmp, 10);

   if (*leftover_tmp != STRING_END)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
              ("ERROR, Invalid sign '%c'. (File: %s, line: %d)\n",
               *leftover_tmp, __FILE__, __LINE__));
      return IFX_FALSE;
   }

   if (abs(ch) > 128)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
              ("ERROR, Wrong channel number %d. (File: %s, line: %d)\n",
               ch, __FILE__, __LINE__));
      return IFX_FALSE;
   }
   *pCh = ch;

   return IFX_TRUE;
}
#endif /* TAPI_FEAT_PHONE_DETECTION_PROCFS */


#ifdef TAPI_FEAT_PROCFS
/**
   Get device count

   \return  number of registered devices
*/
static int proc_get_dev_count (void)
{
   IFX_TAPI_DRV_CTX_t *pDrvCtx  = IFX_NULL;
   IFX_int32_t        i, nDeviceCounter = 0;

   /* for all LL-drivers */
   for (i = 0; i < TAPI_MAX_LL_DRIVERS; i++)
   {
      if (gHLDrvCtx [i].pDrvCtx == IFX_NULL)
      {
         continue;
      }

      pDrvCtx = gHLDrvCtx [i].pDrvCtx;

      if (pDrvCtx->pTapiDev == IFX_NULL)
      {
         /* driver not initialised yet -> skip it */
         break;
      }

      nDeviceCounter += pDrvCtx->maxDevs;
   } /* for all LL-drivers */
   return nDeviceCounter;
}
#endif /* TAPI_FEAT_PROCFS */


#ifdef TAPI_FEAT_PROCFS
/**
   Read the current fifo status

   \param   s
   \param   pos     Device number starting from 1

   \return  zero or an error code
*/
static int proc_read_fifos (struct seq_file *s, int pos)
{
   IFX_TAPI_DRV_CTX_t *pDrvCtx  = IFX_NULL;
   TAPI_DEV           *pTapiDev = IFX_NULL;
   TAPI_CHANNEL       *pChannel = IFX_NULL;
   IFX_int32_t        i, j, k, nDeviceCounter = 0;
   IFX_boolean_t      bDeviceFound = IFX_FALSE;
#if defined(TAPI_FEAT_KPI) || defined (TAPI_FEAT_PACKET)
   IFX_TAPI_STREAM_t  nStream;
   IFX_uint32_t       nMaxStream;
   static char        *pStreamName[] = { "COD", "DECT", "HDLC"};
#endif /* defined(TAPI_FEAT_KPI) || defined (TAPI_FEAT_PACKET) */
   IFX_TAPI_EVENT_FIFO_COUNTERS_t evtCounters;
#ifdef TAPI_FEAT_PACKET
   IFX_TAPI_UPSTREAM_FIFO_COUNTERS_t fdCounters;
#endif /* TAPI_FEAT_PACKET */

#if defined(TAPI_FEAT_KPI) || defined (TAPI_FEAT_PACKET)
   /* maximum stream we are going to print */
   nMaxStream = sizeof(pStreamName)/sizeof(*pStreamName);
   if (IFX_TAPI_STREAM_MAX < nMaxStream)
   {
      nMaxStream = IFX_TAPI_STREAM_MAX;
   }
#endif /* defined(TAPI_FEAT_KPI) || defined (TAPI_FEAT_PACKET) */

   /* pos = 0 means print header, but this procfs entry has no header */
   if (pos <= 0)
      return 0;

   /* for all LL-drivers */
   for (i = 0; i < TAPI_MAX_LL_DRIVERS; i++)
   {
      if (bDeviceFound)
         break;
      if (gHLDrvCtx [i].pDrvCtx == IFX_NULL)
      {
         continue;
      }

      pDrvCtx = gHLDrvCtx [i].pDrvCtx;

      if (pDrvCtx->pTapiDev == IFX_NULL)
      {
         /* driver not initialised yet -> skip it */
         break;
      }

      /* for all devices */
      for (j = 0; j < pDrvCtx->maxDevs; j++)
      {
         pTapiDev = &(pDrvCtx->pTapiDev[j]);

         if (++nDeviceCounter < pos)
            continue;
         bDeviceFound = IFX_TRUE;
         /* Headline (once per device) */
         seq_printf(s, "-- %s --\n"
                       "DEV%2d: evtH evtL ", pDrvCtx->drvName, j);

         if (pDrvCtx->bProvidePktRead)
         {
#ifdef TAPI_FEAT_PACKET
            seq_printf(s, " fd:");
            for (nStream = 0; nStream < nMaxStream; nStream++)
            {
               if (pDrvCtx->readChannels[nStream] > 0)
               {
                  seq_printf(s, "%4s ", pStreamName[nStream]);
               }
            }
#endif /* TAPI_FEAT_PACKET */
#ifdef TAPI_FEAT_KPI
            seq_printf(s, " KPI-CH:");
            for (nStream = 0; nStream < nMaxStream; nStream++)
            {
               seq_printf(s, "%4s ", pStreamName[nStream]);
            }
#endif /* TAPI_FEAT_KPI */
         }

         /* Data line (one per channel) */

         /* Loop over all TAPI channels of this device */
         for (k=0; k < pDrvCtx->maxChannels; k++)
         {
            pChannel = &pTapiDev->pChannel[k];

            seq_printf(s, "\n CH%2d:", k);

            /* Event fifos */
            if (TAPI_statusOk ==
                IFX_TAPI_EventFifoCounters (pChannel, &evtCounters))
            {
               seq_printf(s, " %4d %4d ",
                              evtCounters.nHighWaiting,
                              evtCounters.nLowWaiting);
            }
            else
            {
               seq_printf(s, " FAIL FAIL ");
            }

            if (pDrvCtx->bProvidePktRead)
            {
            /* Fifos on file descriptors */
#ifdef TAPI_FEAT_PACKET
               seq_printf(s, "    ");
               for (nStream = 0; nStream < nMaxStream; nStream++)
               {
                  if (pDrvCtx->readChannels[nStream] == 0)
                     continue;
                  if (TAPI_statusOk ==
                      IFX_TAPI_UpStreamFifo_Counters(
                                 pChannel, nStream, &fdCounters))
                  {
                     seq_printf(s, "%4d ", fdCounters.nWaiting);
                  }
                  else
                  {
                     seq_printf(s, "FAIL ");
                  }
               }
#endif /* TAPI_FEAT_PACKET */

            /* KPI Channels */
#ifdef TAPI_FEAT_KPI
               seq_printf(s, "        ");
               /* protect channel-data from mutual access */
               TAPI_OS_MutexGet (&pChannel->semTapiChDataLock);
               for (nStream = 0; nStream < nMaxStream; nStream++)
               {
                  seq_printf(s, "%04X ", IFX_TAPI_KPI_ChGet(pChannel, nStream));
               }
               /* release channel-data protection */
               TAPI_OS_MutexRelease (&pChannel->semTapiChDataLock);
#endif /* TAPI_FEAT_KPI */
            } /* if bProvidePktRead */
         } /* for all channels */
         seq_printf(s, "\n");
         break;
      } /* for all devices */
   } /* for all LL-drivers */
   return 0;
}
#endif /* TAPI_FEAT_PROCFS */


#ifdef TAPI_FEAT_PHONE_DETECTION_PROCFS
/**
   Install proc entry for device which supports phone detection.

   \param  pTapiDev     Pointer to TAPI_DEV structure.

   \return
     - TAPI_statusOk: if successful
     - error code: in case of an error
*/
IFX_int32_t TAPI_ProcPpdDeviceEntryInstall(
                        TAPI_DEV *pTapiDev)
{
   enum { ENTRY_NAME_BUF_SIZE = 20 };
   IFX_char_t EntryName[ENTRY_NAME_BUF_SIZE];
   struct proc_dir_entry *proc_stat_node;

   /* Check if the procfs entry was created. */
   if(pTapiDev->nInitStatusFlags & TAPI_INITSTATUS_PPD_PROCFS_CREATED)
   {
      return TAPI_statusOk;
   }

   /* Build directory name */
   memset (EntryName, 0, ENTRY_NAME_BUF_SIZE);
   snprintf(EntryName, sizeof(EntryName), "%.8s%.1d",
           pTapiDev->pDevDrvCtx->drvName, pTapiDev->nDev);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,30))
   proc_stat_node =
      create_proc_entry (EntryName, S_IFREG | S_IWUGO, ppd_dev_proc_node);
   if (proc_stat_node)
   {
      proc_stat_node->proc_fops = /* typecast to remove the const qualifier */
         (struct file_operations *)&ppd_device_proc_fops;
      proc_stat_node->data = pTapiDev;
      proc_stat_node->owner = THIS_MODULE;
   }
#else
   proc_stat_node = proc_create_data(EntryName, S_IFREG | S_IWUGO,
      ppd_dev_proc_node, &ppd_device_proc_fops, pTapiDev);
#endif
   if (!proc_stat_node)
   {
      /* errmsg: Device not added to proc fs */
      return TAPI_statusDeviceNotAddedToProcFs;
   }

   /* Remember for the remove function that the procfs entry was created. */
   pTapiDev->nInitStatusFlags |= TAPI_INITSTATUS_PPD_PROCFS_CREATED;

   return TAPI_statusOk;
}
#endif /* TAPI_FEAT_PHONE_DETECTION_PROCFS */


#ifdef TAPI_FEAT_PHONE_DETECTION_PROCFS
/**
   Remove proc entry for device which supports phone detection.

   \param  pTapiDev     Pointer to TAPI_DEV structure.
*/
IFX_void_t TAPI_ProcPpdDeviceEntryRemove(
                        TAPI_DEV *pTapiDev)
{
   IFX_uint32_t   len;

   enum { ENTRY_NAME_BUF_SIZE = 80 };
   IFX_char_t EntryName[ENTRY_NAME_BUF_SIZE];

   /* Only do the remove when the create was successful.
      Otherwise the procfs API will throw a warning. */
   if ((pTapiDev->nInitStatusFlags & TAPI_INITSTATUS_PPD_PROCFS_CREATED) == 0)
   {
      return;
   }
   pTapiDev->nInitStatusFlags &= ~TAPI_INITSTATUS_PPD_PROCFS_CREATED;

   memset (EntryName, 0, ENTRY_NAME_BUF_SIZE);
   len  = snprintf(EntryName, sizeof(EntryName),
                  "driver/" DRV_TAPI_NAME "/phone_detection/devices/%.8s%.1d",
                  pTapiDev->pDevDrvCtx->drvName, pTapiDev->nDev);

   remove_proc_entry(EntryName ,0);
}
#endif /* TAPI_FEAT_PHONE_DETECTION_PROCFS */


#ifdef TAPI_FEAT_PROCFS
/**
   Initialize and install the proc entry

   \return
   -1 or 0 on success
*/
/*lint -save -e611
\todo: FIXME
   (ANSI X3.159-1989)
   It is invalid to convert a function pointer to an object pointer
   or a pointer to void, or vice-versa.
*/
static IFX_int32_t proc_EntriesInstall(void)
{
   struct proc_dir_entry *driver_proc_node;
   int i;

   /* install the proc entry */
   TRACE(TAPI_DRV, DBG_LEVEL_LOW, ("TAPI_DRV: using proc fs\n"));
   driver_proc_node = proc_mkdir( "driver/" DRV_TAPI_NAME, IFX_NULL);
   if (driver_proc_node != IFX_NULL)
   {
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,30))
      driver_proc_node->owner = THIS_MODULE;
#endif /* < Linux 2.6.30 */

      for (i = 0; i < ARRAY_SIZE(proc_entries); i++)
         TAPI_proc_entry_create(driver_proc_node, &proc_entries[i]);

#ifdef TAPI_FEAT_PHONE_DETECTION_PROCFS
      {
         struct proc_dir_entry *ppd_proc_node;
         IFX_int32_t i;

         ppd_proc_node = proc_mkdir( "driver/" DRV_TAPI_NAME "/phone_detection",
                                     IFX_NULL);
         if (ppd_proc_node != IFX_NULL)
         {
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,30))
            driver_proc_node->owner = THIS_MODULE;
#endif /* < Linux 2.6.30 */
            for (i = 0; i < ARRAY_SIZE(proc_entries_ppd); i++)
               TAPI_proc_entry_create(ppd_proc_node, &proc_entries_ppd[i]);

            ppd_dev_proc_node = proc_mkdir( "driver/" DRV_TAPI_NAME "/phone_detection/devices",
               IFX_NULL);
            if (ppd_dev_proc_node == IFX_NULL)
            {
               TRACE(TAPI_DRV,DBG_LEVEL_HIGH,
                    ("TAPI_DRV: cannot create proc entry for phone "
                     "detection devices.\n"));
               return -1;
            }
         }
         else
         {
            TRACE(TAPI_DRV,DBG_LEVEL_HIGH,
                  ("TAPI_DRV: cannot create phone detection proc entry\n"));
            return -1;
         }
      }
#endif /* TAPI_FEAT_PHONE_DETECTION_PROCFS */
   }
   else
   {
      TRACE(TAPI_DRV,DBG_LEVEL_HIGH,("TAPI_DRV: cannot create proc entry\n"));
      return -1;
   }

   return 0;
}
/*lint -restore */
#endif /* TAPI_FEAT_PROCFS */


#ifdef TAPI_FEAT_PROCFS
/**
   Remove proc filesystem entries.

   \return
   None.

   \remarks
   Called by the kernel.
*/
static IFX_void_t proc_EntriesRemove(void)
{
   IFX_char_t buf[64];
   IFX_int32_t i;
#ifdef TAPI_FEAT_PHONE_DETECTION_PROCFS
   remove_proc_entry("driver/" DRV_TAPI_NAME "/phone_detection/devices" ,0);
   for (i = 0; i < ARRAY_SIZE(proc_entries_ppd); i++)
   {
      sprintf(buf, "driver/" DRV_TAPI_NAME "/phone_detection/%s",
         proc_entries_ppd[i].name);
      remove_proc_entry(buf, 0);
   }
   remove_proc_entry("driver/" DRV_TAPI_NAME "/phone_detection" ,0);
#endif /* TAPI_FEAT_PHONE_DETECTION_PROCFS */
   for (i = 0; i < ARRAY_SIZE(proc_entries); i++)
   {
      sprintf(buf, "driver/" DRV_TAPI_NAME "/%s", proc_entries[i].name);
      remove_proc_entry(buf, 0);
   }
   remove_proc_entry("driver/" DRV_TAPI_NAME ,0);

   return;
}
#endif /* TAPI_FEAT_PROCFS */


#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)) && \
    (LINUX_VERSION_CODE <= KERNEL_VERSION(3,8,0))
/**
   Set the scheduling policy for the TAPIevent workqueue to RT scheduling

   \param  foo          unused.
*/
static IFX_void_t tapi_wq_setscheduler (IFX_int32_t foo)
{
   struct sched_param sched_params;

   IFX_UNUSED(foo);

   sched_params.sched_priority = TAPI_OS_THREAD_PRIO_HIGH;
   sched_setscheduler(current, SCHED_FIFO, &sched_params);
}
#endif /* LINUX_VERSION_CODE */


#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
static IFX_int32_t tapiClassCreate(void)
{
   pTAPI_Class = class_create(THIS_MODULE, "tapi");
   if (IS_ERR(pTAPI_Class))
   {
      TRACE( TAPI_DRV, DBG_LEVEL_HIGH,
         ("tapiClassCreate: unable to create tapi class\n"));
      return TAPI_statusErr;
   }
   return 0;
}

static IFX_void_t tapiClassRemove(void)
{
   class_destroy(pTAPI_Class);
}
#endif /* >= Linux 2.6.0 */

/**
   Initialize the module.

   \return
   Error code or 0 on success

   \remarks
   Called by the kernel.
*/
static int __init ifx_tapi_module_init(void)
{
   /* copyright trace shall not be prefixed with KERN_INFO */
   printk("%s, (c) 2001-2016 Lantiq Beteiligungs-GmbH & Co.KG\n", &TAPI_WHATVERSION[4]);

   if (IFX_TAPI_Driver_Start() != IFX_SUCCESS)
   {
      printk (KERN_ERR "Driver start failed\n");
      return -1;
   }

   memset (&tapi_fops, 0, sizeof (tapi_fops));

#ifdef TAPI_FEAT_PROCFS
   proc_EntriesInstall();
#endif /* TAPI_FEAT_PROCFS */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
   if (tapiClassCreate() != IFX_SUCCESS)
      return -1;
#endif /* >= Linux 2.6.0 */


#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
#  if (LINUX_VERSION_CODE > KERNEL_VERSION(3,8,0))
   /*
     For "Concurrency Managed Workqueue" use related flags to increase the
     priority.

     WQ_MEM_RECLAIM

      All wq which might be used in the memory reclaim paths _MUST_
      have this flag set.  The wq is guaranteed to have at least one
      execution context regardless of memory pressure.

     WQ_HIGHPRI

      Work items of a highpri wq are queued to the highpri
      thread-pool of the target gcwq.  Highpri thread-pools are
      served by worker threads with elevated nice level.

      Note that normal and highpri thread-pools don't interact with
      each other.  Each maintain its separate pool of workers and
      implements concurrency management among its workers.
   */
   pTAPItimersWq = alloc_workqueue("TAPItimers", WQ_MEM_RECLAIM | WQ_HIGHPRI, 0);
   pTAPIeventsWq = alloc_workqueue("TAPIevents",
      WQ_MEM_RECLAIM | WQ_HIGHPRI, 0);
#  else
   pTAPItimersWq = create_workqueue("TAPItimers");
   pTAPIeventsWq = create_workqueue("TAPIevents");
   TAPI_DeferWork((IFX_void_t *) tapi_wq_setscheduler,
                  (IFX_void_t *)&tapi_wq_setscheduler_param);
#  endif
#endif /* LINUX_VERSION_CODE */

   return 0;
}

/**
   Clean up the module.

   \remarks
   Called by the kernel.
*/
static void __exit ifx_tapi_module_exit(void)
{
   int i;

   printk (KERN_INFO "Removing Highlevel TAPI module\n");

   /* Actually all LL drivers should have unregistered here. Being careful
      we force unregister of any drivers which may still be registered. */
   for (i = 0; i < TAPI_MAX_LL_DRIVERS; i++)
   {
      if (gHLDrvCtx [i].pDrvCtx != IFX_NULL)
      {
         IFX_TAPI_Unregister_LL_Drv (gHLDrvCtx [i].pDrvCtx->majorNumber);
      }
   }

#ifdef TAPI_FEAT_PROCFS
   proc_EntriesRemove();
#endif /* TAPI_FEAT_PROCFS */

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
   tapiClassRemove();
#endif /* >= Linux 2.6.0 */

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
   /* as we are using work queues to schedule events from the interrupt
      context to the process context, we use work queues in case of
      Linux 2.6. they must be flushed on driver unload... */
   flush_workqueue(pTAPItimersWq);
   destroy_workqueue(pTAPItimersWq);
   flush_workqueue(pTAPIeventsWq);
   destroy_workqueue(pTAPIeventsWq);
#endif /* LINUX_VERSION_CODE */

   IFX_TAPI_Driver_Stop();

   TRACE(TAPI_DRV, DBG_LEVEL_NORMAL, ("TAPI_DRV: cleanup successful\n"));
}
#endif /* IFXOS_USE_DEV_IO */


/* ============================= */
/* Timer abstraction             */
/* ============================= */
#ifdef TAPI_HAVE_TIMERS

#ifdef __KERNEL__
   /* Functionpointer to the callbackfunction */
   typedef IFX_void_t (*linux_timer_callback)(IFX_ulong_t);

   /* Timer ID */
   struct Timer_ID_s
   {
   #if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
      /* !!! important, work struct must be the first element, because we
             need to cast it later on to Timer_ID */
      struct tq_struct timerTask;
   #else
      struct work_struct timerTask;
   #endif /* LINUX_VERSION_CODE */
      struct timer_list Timer_List;
      IFX_boolean_t bPeriodical;
      IFX_uint32_t Periodical_Time;
      TIMER_ENTRY pTimerEntry;
      IFX_ulong_t nArgument;
      IFX_boolean_t bStopped;
   };
#else /* ! __KERNEL__ */
/** List entry */
struct list_entry {
   /** Previous list entry */
   struct list_entry *next;
   /** Next list entry */
   struct list_entry *prev;
};

/** List structure */
struct list {
   /** Used list entries, this first element is empty
       and can be used only to pint to other elements */
   struct list_entry first_element;
   /** Size of list entry data */
   size_t payload_size;
   /** List lock */
   IFXOS_lock_t lock;
};

/** timeout descriptor structure */
struct timeout {
   /** set IFX_TRUE if handler should be called periodically,
       otherwise set IFX_FALSE */
   IFX_boolean_t bPeriodical;
   /** timeout argument */
   unsigned long arg1;
   /** timeout handler */
   TIMER_ENTRY handler;
};

/** timeout list entry */
struct timeout_list_entry {
   /** Time when the timeout becomes active (in milliseconds) */
   time_t timeout_time;
   /** Time to wait from setting the timeout, used for periodic events */
   time_t time_in;
   /** timeout descriptor */
   struct timeout timeout;
};

/** timeout control structure */
struct TAPI_TM_Context {
   /* IFX_TRUE if timers were initialized */
   IFX_boolean_t bTimersInialized;
   /** timeout list */
   struct list timeout_list;
   /** Timeout thread control structure */
   IFXOS_ThreadCtrl_t timeout_thread_ctrl;
};

/** default timeout thread poll time (in milliseconds),
    this is the polling time used to check if new entiries/events were added */
#ifndef TIMEOUT_THREAD_POLL_TIME
   #define TIMEOUT_THREAD_POLL_TIME   50
#endif

/** get pointer to payload of given entry */
#define list_entry_data(ENTRY) ((void *)((char *)ENTRY + sizeof(struct list_entry)))

/** check if list is empty */
#define is_list_empty(LIST) \
   (((LIST)->first_element.next == &(LIST)->first_element) ? IFX_TRUE : IFX_FALSE)
#define foreach_list_entry_safe_ll(PLIST, ENTRY, NEXT_ENTRY) \
   for ((ENTRY) = (PLIST)->next, (NEXT_ENTRY) = (ENTRY)->next; \
        (ENTRY)->next != (PLIST)->next; \
        (ENTRY) = (NEXT_ENTRY), (NEXT_ENTRY) = (ENTRY)->next)
#define foreach_list_entry(PLIST, ENTRY) \
   for ((ENTRY) = (PLIST)->first_element.next; \
        (ENTRY)->next != (PLIST)->first_element.next; \
        (ENTRY) = (ENTRY)->next)

static IFX_return_t TAPI_TM_list_init(struct list *list, size_t payload_size);
static void TAPI_TM_list_entry_free(struct list *list,
             struct list_entry *entry);
static IFX_return_t TAPI_TM_lockless_event_remove(struct TAPI_TM_Context *context,
                    struct list_entry *entry_to_remove);
static IFX_return_t TAPI_TM_timeout_event_remove(struct TAPI_TM_Context *context,
                 struct list_entry *timer_entry);
static IFX_return_t TAPI_TM_timeout_event_stop(struct TAPI_TM_Context *context,
                 struct list_entry *timer_entry);
static struct list_entry *TAPI_TM_next_active_event_get(struct TAPI_TM_Context *context,
                    struct timeout *timeout,
                    time_t * time_to_next_entry);
static void TAPI_TM_list_entry_remove(struct list *list,
             struct list_entry *entry);
static IFX_return_t TAPI_TM_lockless_event_stop(struct TAPI_TM_Context *context,
                    struct list_entry *entry_to_stop);
static IFX_int32_t TAPI_TM_timeout_thread_main(struct IFXOS_ThreadParams_s *thr_params);
static void TAPI_TM_list_delete(struct list *list);
static IFX_return_t TAPI_TM_timeout_init(struct TAPI_TM_Context *context);
static struct list_entry *TAPI_TM_event_entry_create(struct TAPI_TM_Context *context,
                   const struct timeout *timeout);
static struct list_entry *TAPI_TM_timeout_event_create(struct TAPI_TM_Context *context,
              TIMER_ENTRY handler,
              unsigned long arg1);
static IFX_return_t TAPI_TM_timeout_event_start(struct TAPI_TM_Context *context,
              struct list_entry *timer_entry,
              time_t timeout_time, IFX_boolean_t bPeriodical);
static IFX_return_t TAPI_TM_event_entry_start(struct TAPI_TM_Context *context,
                   struct list_entry *timer_entry,
                   time_t timeout_time,
                   IFX_boolean_t bPeriodical);
static IFX_return_t TAPI_TM_lockless_event_entry_start(struct TAPI_TM_Context *context,
                   struct list_entry *timer_entry,
                   time_t timeout_time);
static void TAPI_TM_list_entry_add_before(struct list *list,
            struct list_entry *entry,
            struct list_entry *new_entry);
static void TAPI_TM_list_entry_add_tail(struct list *list,
                   struct list_entry *new_entry);
static void TAPI_TM_list_entry_add_after(struct list *list,
           struct list_entry *entry,
           struct list_entry *new_entry);

/** \todo deleting/clean up of timers is missing for user space, it should
    be called upon closing the tapi thread or even maybe for dev stop */
/** control structure for timers in user's space */
struct TAPI_TM_Context G_timers /* = {.bTimersInialized = IFX_FALSE} */;

/**
   Create new list_entry element and assign a callback function to it.

   \param  context      pointer to tapi timer context structure
   \param  handler      pointer to callback function
   \param  arg1         private data for the callback, can be pointer or int

   \return
     - pointer to newly allocated list_entry
     - IFX_NULL in case of error, return value of TAPI_TM_event_entry_create()
*/
static struct list_entry *TAPI_TM_timeout_event_create(struct TAPI_TM_Context *context,
              TIMER_ENTRY handler,
              unsigned long arg1)
{
   struct timeout timeout;

   timeout.handler = handler;
   timeout.arg1 = arg1;
   timeout.bPeriodical = IFX_FALSE;

   return TAPI_TM_event_entry_create(context, &timeout);
}

/**
   Create new list_entry element, allocate memory and copy timeout data.

   \param  context      pointer to tapi timer context structure
   \param  timeout      pointer to timeout stucture

   \return
     - pointer to newly allocated list_entry
     - IFX_NULL in case of error
*/
static struct list_entry *TAPI_TM_event_entry_create(struct TAPI_TM_Context *context,
                   const struct timeout *timeout)
{
   struct list_entry *new_entry;
   struct timeout_list_entry *new_timeout_entry;

   TAPI_OS_LockGet(&context->timeout_list.lock);
   /* allocate memory for list entry with payload */
   new_entry = TAPI_OS_Malloc(sizeof(struct list_entry) + context->timeout_list.payload_size);
   if (!new_entry) {
      TAPI_OS_LockRelease(&context->timeout_list.lock);
      return IFX_NULL;
   }

   new_timeout_entry = list_entry_data(new_entry);

   memcpy(&new_timeout_entry->timeout, timeout,
          sizeof(struct timeout));

   TAPI_OS_LockRelease(&context->timeout_list.lock);

   return new_entry;
}

/**
   Initialize TAPI_TM_Context structure and its list and start timeout control
   thread.

   \param  context      pointer to tapi timer context structure

   \return
     - IFX_SUCCESS or IFX_ERROR in case of errors
*/
static IFX_return_t TAPI_TM_timeout_init(struct TAPI_TM_Context *context)
{
   IFX_return_t error;

   error = TAPI_TM_list_init(&context->timeout_list, sizeof(struct timeout_list_entry));

   if (error)
   {
      TRACE( TAPI_DRV, DBG_LEVEL_HIGH,
           ("TAPI_DRV: TAPI_TM_list_init() failed for timer Initialization\n"));
      return error;
   }

   error = (IFX_SUCCESS == TAPI_OS_ThreadInit(&context->timeout_thread_ctrl,
                 "tapitm",
                 TAPI_TM_timeout_thread_main,
                 IFXOS_DEFAULT_STACK_SIZE,
                 TAPI_OS_THREAD_PRIO_HIGHEST,
                 (unsigned long)context,
                 0)) ? IFX_SUCCESS : IFX_ERROR;

   if (error)
   {
      TRACE( TAPI_DRV, DBG_LEVEL_HIGH,
           ("TAPI_DRV: TAPI_OS_ThreadInit() failed for timer Initialization\n"));
      TAPI_TM_list_delete(&context->timeout_list);
      return error;
   }
   return IFX_SUCCESS;
}

/**
   Release memory and other resources used.

   \param  list         pointer to list

*/
static void TAPI_TM_list_delete(struct list *list)
{
   struct list_entry *entry, *tmp_entry;

   foreach_list_entry_safe_ll(&list->first_element, entry, tmp_entry)
   {
      TAPI_OS_Free(entry);
   }
   /* if list is empty fileds next and prev should point first_element */
   list->first_element.next = &list->first_element;
   list->first_element.prev = &list->first_element;
   (void)TAPI_OS_LockDelete(&list->lock);
}

/** timeout events handling thread

   \param[in] thr_params Thread arguments

   \return 0
*/
static IFX_int32_t TAPI_TM_timeout_thread_main(struct IFXOS_ThreadParams_s *thr_params)
{
   struct TAPI_TM_Context *context = (struct TAPI_TM_Context *)thr_params->nArg1;
   struct timeout timer;
   struct list_entry *timer_to_execute_entry;
   time_t wait_time;

   memset (&timer, 0, sizeof (timer));
   /* while thread is running */
   while (thr_params->bRunning == IFX_TRUE &&
          thr_params->bShutDown == IFX_FALSE)
   {
      /* wait for message in FIFO */
      while (1)
      {
         wait_time = 0;
         TAPI_OS_LockGet(&context->timeout_list.lock);
         timer_to_execute_entry = TAPI_TM_next_active_event_get(context, &timer,
                                     &wait_time);
         TAPI_OS_LockRelease(&context->timeout_list.lock);
         if (timer_to_execute_entry == IFX_NULL
             && thr_params->bShutDown == IFX_FALSE
             && thr_params->bRunning == IFX_TRUE)
         {
            /* if time to the next timeout is longer than the poll time,
               then wait only TIMEOUT_THREAD_POLL_TIME ms */
            if ((wait_time == 0) || (wait_time > TIMEOUT_THREAD_POLL_TIME))
            {
               wait_time = TIMEOUT_THREAD_POLL_TIME;
            }
            /** \todo replace sleep with select, for this some sync mechanism
                with add/start entry is needed, for example select could wait
                for a pipe fd being readable, each function for add/start entry
                would write to this pipe, this could replace the polling,
                if above is implemented then wait_time could be set to exact
                time to next timeout or wait forever untill and event/entry
                is added */
            TAPI_OS_MSecSleep((IFX_time_t) wait_time);
         }
         else
         {
            break;
         }
      }

      /* check if we are shutting down */
      if (thr_params->bShutDown == IFX_TRUE
          || thr_params->bRunning == IFX_FALSE)
      {
         break;
      }

      TAPI_OS_LockGet(&context->timeout_list.lock);
      {
         struct timeout_list_entry *periodical_timeout_entry;
         time_t time_in;

         periodical_timeout_entry = list_entry_data(timer_to_execute_entry);
         (void)TAPI_TM_lockless_event_stop(context, timer_to_execute_entry);
         if (periodical_timeout_entry && timer.bPeriodical)
         {
            /* for periodical events get time to next timeout and add new timeout */
            time_in = periodical_timeout_entry->time_in;
            (void) TAPI_TM_lockless_event_entry_start(context, timer_to_execute_entry,
                                                      time_in);
         }
      }
      TAPI_OS_LockRelease(&context->timeout_list.lock);
      if (timer.handler != NULL)
      {
         (void) timer.handler((Timer_ID) timer_to_execute_entry, (IFX_ulong_t) timer.arg1);
      }
      else
      {
         TRACE( TAPI_DRV, DBG_LEVEL_HIGH,
               ("TAPI_TM_timeout_thread_main - ERROR: found event without timer handler"));
      }
   }
   return 0;
}

/**
   Lockless version of TAPI_TM_timeout_event_remove.

   \param  context         pointer to tapi timer context structure
   \param  entry_to_stop   pointer to tapi entry structure

   \return
     - IFX_SUCCESS
*/
static IFX_return_t TAPI_TM_lockless_event_stop(struct TAPI_TM_Context *context,
                    struct list_entry *entry_to_stop)
{
   struct list_entry *current_entry;

   if (is_list_empty(&context->timeout_list))
   {
      return IFX_SUCCESS;
   }
   foreach_list_entry(&context->timeout_list, current_entry)
   {
      if (current_entry == entry_to_stop)
      {
         TAPI_TM_list_entry_remove(&context->timeout_list, current_entry);
         return IFX_SUCCESS;
      }
   }
   return IFX_SUCCESS;
}

/**
   Remove entry from the list.

   \param  list      pointer to list structure
   \param  entry     pointer to entry structure that will be removed
*/
static void TAPI_TM_list_entry_remove(struct list *list,
             struct list_entry *entry)
{
   entry->next->prev = entry->prev;
   entry->prev->next = entry->next;
   entry->prev = IFX_NULL;
   entry->next = IFX_NULL;
   if (list->first_element.next == entry)
   {
      list->first_element.next = &list->first_element;
   }
}

/** Get next timeouted event

   \param[in]  context              timer context pointer
   \param[out] timeout              Returns timeout descriptor
   \param[out] time_to_next_entry   time till the next timeout expires,
                                    set only if no entry was found

   \return first entry on the list for which timeout expired or IFX_NULL
*/
static struct list_entry *TAPI_TM_next_active_event_get(struct TAPI_TM_Context *context,
                    struct timeout *timeout,
                    time_t * time_to_next_entry)
{
   struct timeout_list_entry *first_entry;
   time_t currentTime;

   if (is_list_empty(&context->timeout_list)) {
      return IFX_NULL;
   }

   /* get the first entry from the list,
      should be the one for which timeout expires first */
   first_entry = list_entry_data(context->timeout_list.first_element.next);
   if (!first_entry)
   {
      return IFX_NULL;
   }
   currentTime = (time_t)IFXOS_ElapsedTimeMSecGet(0);
   if (first_entry->timeout_time <= currentTime)
   {
      /* timeout occured for this entry */
      *timeout = first_entry->timeout;
      return context->timeout_list.first_element.next;
   }
   else
   {
      /* get time to next timeout */
      *time_to_next_entry = first_entry->timeout_time - currentTime;
   }
   return IFX_NULL;
}

/** Stop handling of given entry

   \param[in]  context        timer context pointer
   \param[in]  timer_entry    timer entry to stop

   \return always IFX_SUCCESS as TAPI_TM_lockless_event_stop does
*/
static IFX_return_t TAPI_TM_timeout_event_stop(struct TAPI_TM_Context *context,
                 struct list_entry *timer_entry)
{
   IFX_return_t error;
   struct timeout_list_entry *timeout_entry_to_start;

   TAPI_OS_LockGet(&context->timeout_list.lock);
   timeout_entry_to_start = list_entry_data(timer_entry);
   timeout_entry_to_start->timeout.bPeriodical = IFX_FALSE;
   error = TAPI_TM_lockless_event_stop(context, timer_entry);
   TAPI_OS_LockRelease(&context->timeout_list.lock);

   return error;
}

/** Remove given entry from the list

   \param[in]  context        timer context pointer
   \param[in]  timer_entry    timer entry to remove

   \return always IFX_SUCCESS as TAPI_TM_lockless_event_remove does
*/
static IFX_return_t TAPI_TM_timeout_event_remove(struct TAPI_TM_Context *context,
                 struct list_entry *timer_entry)
{
   IFX_return_t error;

   TAPI_OS_LockGet(&context->timeout_list.lock);
   error = TAPI_TM_lockless_event_remove(context, timer_entry);
   TAPI_OS_LockRelease(&context->timeout_list.lock);

   return error;
}

/** Lockless version of TAPI_TM_timeout_event_remove

   \param[in]  context           timer context pointer
   \param[in]  entry_to_remove   entry to remove

   \return always IFX_SUCCESS
 */
static IFX_return_t TAPI_TM_lockless_event_remove(struct TAPI_TM_Context *context,
                    struct list_entry *entry_to_remove)
{
   struct list_entry *current_entry;

   if (is_list_empty(&context->timeout_list))
   {
      return IFX_SUCCESS;
   }
   foreach_list_entry(&context->timeout_list, current_entry)
   {
      if (current_entry == entry_to_remove)
      {
         TAPI_TM_list_entry_free(&context->timeout_list, current_entry);
         return IFX_SUCCESS;
      }
   }
   return IFX_SUCCESS;
}

/** Release memory allocated for given entry

   \param[in]  list     pointeer to the list - unused
   \param[in]  entry    entry to remove and free memory
 */
static void TAPI_TM_list_entry_free(struct list *list,
             struct list_entry *entry)
{
   IFX_UNUSED(list);

   if ((IFX_NULL != entry->next) && (IFX_NULL != entry->prev))
   {
      entry->next->prev = entry->prev;
      entry->prev->next = entry->next;
   }
   TAPI_OS_Free(entry);
}

/** Release memory allocated for given entry

   \param[in]  list           pointeer to the list
   \param[in]  payload_size   size of memory in bytes needed to hold the entry data

   \return value TAPI_OS_LockInit() call
 */
static IFX_return_t TAPI_TM_list_init(struct list *list, size_t payload_size)
{
   /* for first element next and prev point to first element if list is empty */
   list->first_element.next = &list->first_element;
   list->first_element.prev = &list->first_element;
   list->payload_size = payload_size;
   /* get the lock */
   return TAPI_OS_LockInit(&list->lock);
}

/** Start handling of given timeout entry

   \param[in]  context        timer context pointer
   \param[in]  timer_entry    timer entry to start
   \param[in]  timeout_time   timoeout time for new timer entry
   \param[in]  bPeriodical    if IFX_TRUE, then timeout for event will be
                              checked periodicaly to execute the handler

   \return always IFX_SUCCESS as TAPI_TM_event_entry_start does
*/
static IFX_return_t TAPI_TM_timeout_event_start(struct TAPI_TM_Context *context,
              struct list_entry *timer_entry,
              time_t timeout_time, IFX_boolean_t bPeriodical)
{
   /** \todo add a check if timers are initialized */
   return TAPI_TM_event_entry_start(context, timer_entry, timeout_time, bPeriodical);
}

/** Start handling of given timeout entry

   \param[in]  context        timer context pointer
   \param[in]  timer_entry    timer entry to start
   \param[in]  timeout_time   timoeout time for new timer entry
   \param[in]  bPeriodical    if IFX_TRUE, then timeout for event will be
                              checked periodicaly to execute the handler

   \return always IFX_SUCCESS
*/
static IFX_return_t TAPI_TM_event_entry_start(struct TAPI_TM_Context *context,
                   struct list_entry *timer_entry,
                   time_t timeout_time,
                   IFX_boolean_t bPeriodical)
{
   struct timeout_list_entry *timeout_entry_to_start;

   TAPI_OS_LockGet(&context->timeout_list.lock);
   timeout_entry_to_start = list_entry_data(timer_entry);
   timeout_entry_to_start->timeout.bPeriodical = bPeriodical;
   TAPI_TM_lockless_event_entry_start(context, timer_entry, timeout_time);
   TAPI_OS_LockRelease(&context->timeout_list.lock);

   return IFX_SUCCESS;
}

/** Add timeout event

   \param[in]  context        timer context pointer
   \param[in]  timer_entry    timer entry to start
   \param[in]  timeout_time   timoeout time for new timer entry (in ms)

   \return always IFX_SUCCESS
*/
static IFX_return_t TAPI_TM_lockless_event_entry_start(struct TAPI_TM_Context *context,
                   struct list_entry *timer_entry,
                   time_t timeout_time)
{
   struct list_entry *current_entry;
   struct timeout_list_entry *timeout_entry, *timeout_entry_to_start;
   IFX_boolean_t added = IFX_FALSE;

   timeout_entry_to_start = list_entry_data(timer_entry);
   /* get timout time to compare with current time read
      with IFXOS_ElapsedTimeMSecGet(0) */
   timeout_entry_to_start->timeout_time = IFXOS_ElapsedTimeMSecGet(0) + timeout_time;
   timeout_entry_to_start->time_in = timeout_time;
   foreach_list_entry(&context->timeout_list, current_entry)
   {
      timeout_entry = list_entry_data(current_entry);
      if (timeout_entry->timeout_time >
          timeout_entry_to_start->timeout_time)
      {
         TAPI_TM_list_entry_add_before(&context->timeout_list, current_entry,
                     timer_entry);
         added = IFX_TRUE;
         break;
      }
   }
   if (!added)
   {
      TAPI_TM_list_entry_add_tail(&context->timeout_list, timer_entry);
   }
   return IFX_SUCCESS;
}

/** Add add new entry before entry that is already on the list

   \param[in]  list           the list - unused
   \param[in]  entry          entry from the list
   \param[in]  new_entry      entry to add
*/
static void TAPI_TM_list_entry_add_before(struct list *list,
            struct list_entry *entry,
            struct list_entry *new_entry)
{
   IFX_UNUSED(list);

   entry->prev->next = new_entry;
   new_entry->prev = entry->prev;
   new_entry->next = entry;
   entry->prev = new_entry;
}

/** Add add new entry at the end of the list

   \param[in]  list           the list
   \param[in]  entry          entry to add
*/
static void TAPI_TM_list_entry_add_tail(struct list *list,
                   struct list_entry *new_entry)
{
   TAPI_TM_list_entry_add_after(list, list->first_element.prev, new_entry);
}

/** Add add new entry after entry that is already on the list

   \param[in]  list           the list - unused
   \param[in]  entry          entry from the list
   \param[in]  new_entry      entry to add
*/
static void TAPI_TM_list_entry_add_after(struct list *list,
           struct list_entry *entry,
           struct list_entry *new_entry)
{
   IFX_UNUSED(list);

   entry->next->prev = new_entry;
   new_entry->next = entry->next;
   entry->next = new_entry;
   new_entry->prev = entry;
}
#endif /* __KERNEL__ */

/**
   Create a timer.

   \param  pTimerEntry  Function pointer to the call back function.
   \param  nArgument    Pointer to TAPI channel structure.

   \return
   Timer_ID  Pointer to internal timer structure.

   \remarks
   Initialize a task queue which will be scheduled once a timer interrupt occurs
   to execute the appropriate operation in a process context, process in which
   semaphores ... are allowed.
   Please notice that this task has to run under the keventd process, in which
   it can be executed thousands of times within a single timer tick.
*/
Timer_ID TAPI_Create_Timer(TIMER_ENTRY pTimerEntry, IFX_ulong_t nArgument)
{
#ifdef __KERNEL__
   struct Timer_ID_s *pTimerData;

   /* allocate memory for the timer data structure */
   pTimerData = kmalloc(sizeof (*pTimerData), GFP_KERNEL);
   if (pTimerData == IFX_NULL)
      return IFX_NULL;

   /* set function to be called after timer expires */
   pTimerData->pTimerEntry = pTimerEntry;
   pTimerData->nArgument = nArgument;
   pTimerData->bStopped = IFX_FALSE;

   init_timer(&(pTimerData->Timer_List));

   /* set timer call back function */
   pTimerData->Timer_List.function = TAPI_timer_call_back;
   pTimerData->Timer_List.data = (IFX_ulong_t) pTimerData;

   /* Initialize Timer Task */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
   /* initialize tq_struct member of Timer_ID structure */
   memset(&(pTimerData->timerTask), 0, sizeof (struct tq_struct));
   pTimerData->timerTask.routine = TAPI_tqueue;
   pTimerData->timerTask.data = (IFX_void_t *) pTimerData;
#elif (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20))
   INIT_WORK(&(pTimerData->timerTask), TAPI_tqueue, (IFX_void_t *) pTimerData);
#else
   INIT_WORK(&(pTimerData->timerTask), TAPI_tqueue);
#endif /* LINUX_VERSION_CODE */

   return (Timer_ID)pTimerData;
#else /* ! __KERNEL__ */
   if (IFX_TRUE != G_timers.bTimersInialized)
   {
      /* timeout thread needs to be started only once */
      G_timers.bTimersInialized = IFX_TRUE;
      TAPI_TM_timeout_init(&G_timers);
   }
   return (Timer_ID) TAPI_TM_timeout_event_create(&G_timers, pTimerEntry, nArgument);
#endif /* __KERNEL__ */
}

/**
   Sets a timer to the specified time and starts it. It can be choose if the
   timer starts periodically.

   \param  Timer_ID     Pointer to internal timer structure.
   \param  nTime        Time in ms.
   \param  bPeriodically Starts the timer periodically or not.
   \param  bRestart     Restart the timer or normal start.

   \return
   Returns an error code: IFX_TRUE / IFX_FALSE
*/
IFX_boolean_t TAPI_SetTime_Timer(Timer_ID Timer,
                                 IFX_uint32_t nTime,
                                 IFX_boolean_t bPeriodically,
                                 IFX_boolean_t bRestart)
{
#ifdef __KERNEL__
   struct Timer_ID_s *pTimerData = (struct Timer_ID_s *)Timer;

   if (pTimerData == IFX_NULL)
      return IFX_FALSE;

   pTimerData->Periodical_Time = HZ*nTime/1000;
   pTimerData->bPeriodical = bPeriodically;
   pTimerData->bStopped = IFX_FALSE;

   if (timer_pending(&(pTimerData->Timer_List)))
   {
      if (IFX_FALSE == bRestart)
         return IFX_TRUE;

      if (mod_timer(&(pTimerData->Timer_List),
                    jiffies + pTimerData->Periodical_Time))
         return IFX_TRUE;
   }

   pTimerData->Timer_List.expires = jiffies + pTimerData->Periodical_Time;
   add_timer(&(pTimerData->Timer_List));
#else /* !__KERNEL__ */
   /** \todo add a check if timers are initialized */
   if (bRestart == IFX_TRUE)
   {
      (void) TAPI_TM_timeout_event_stop(&G_timers, (struct list_entry *) Timer);
   }

   if (TAPI_TM_timeout_event_start(&G_timers,
              (struct list_entry *) Timer,
              (time_t) nTime,
              bPeriodically) != 0)
   {
      TRACE( TAPI_DRV, DBG_LEVEL_HIGH, ("TAPI_SetTime_Timer: failed to start timer\n"));
      return IFX_FALSE;
   }
#endif /* __KERNEL__ */

   return IFX_TRUE;
}

/**
   Stop a timer.

   \param  Timer_ID     Pointer to internal timer structure.

   \return
   Returns an error code: IFX_TRUE / IFX_FALSE
*/
IFX_boolean_t TAPI_Stop_Timer(Timer_ID Timer)
{
#ifdef __KERNEL__
   struct Timer_ID_s *pTimerData = (struct Timer_ID_s *)Timer;

   if (pTimerData != IFX_NULL)
   {
      /* stop timer */
      pTimerData->bStopped = IFX_TRUE;
      /* prevent restart of driver */
      pTimerData->bPeriodical = IFX_FALSE;

      if (timer_pending(&(pTimerData->Timer_List)))
      {
         /* remove driver from list */
         del_timer_sync(&(pTimerData->Timer_List));
      }
   }
#else /* ! __KERNEL__ */
   if(Timer)
      (void) TAPI_TM_timeout_event_stop(&G_timers, (struct list_entry *) Timer);
#endif /* __KERNEL__ */

   return IFX_TRUE;
}

/**
   Delete a timer.

   \param  Timer_ID     Pointer to internal timer structure.

   \return
   Returns an error code: IFX_TRUE / IFX_FALSE
*/
IFX_boolean_t TAPI_Delete_Timer(Timer_ID Timer)
{
   if (Timer == IFX_NULL)
      return IFX_FALSE;

   TAPI_Stop_Timer(Timer);

   /* free memory */
#ifdef __KERNEL__
   kfree(Timer);
#else
   (void) TAPI_TM_timeout_event_remove(&G_timers, (struct list_entry *)Timer);
#endif /* __KERNEL__ */

   return IFX_TRUE;
}
#endif /* TAPI_HAVE_TIMERS */

#ifdef __KERNEL__
/**
   Helper function to get a periodical timer.

   \param  pWork        Pointer to corresponding timer ID.

   \remarks
   This function will be executed in  the process context, so to avoid
   scheduling in Interrupt Mode while working with semaphores etc...
   The task is always running under the keventd process and is also running
   very quickly. Even on a very heavily loaded system, the latency in the
   scheduler queue is quite small
*/
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20))
static IFX_void_t TAPI_tqueue (IFX_void_t *pWork)
#else /* for Kernel newer or equal 2.6.20 */
static IFX_void_t TAPI_tqueue (struct work_struct *pWork)
#endif /* (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)) */
{
   struct Timer_ID_s *pTimerData = (struct Timer_ID_s *)pWork;

   if (pTimerData->bStopped)
      return;

   /* Call TAPI Timer function */
   pTimerData->pTimerEntry(pTimerData, pTimerData->nArgument);
   if (pTimerData->bPeriodical)
   {
      if (timer_pending(&(pTimerData->Timer_List)))
      {
         /* update existed timer */
         if (mod_timer(&(pTimerData->Timer_List),
                       jiffies + pTimerData->Periodical_Time))
            return;
      }

      /* create new timer */
      pTimerData->Timer_List.expires = jiffies + pTimerData->Periodical_Time;
      add_timer(&(pTimerData->Timer_List));
   }
}

#ifdef __KERNEL__
/**
   Helper function to get a periodical timer.

   \param  arg          Pointer to corresponding timer ID.
*/
static IFX_void_t TAPI_timer_call_back (IFX_ulong_t arg)
{
   struct Timer_ID_s *pTimerData = (struct Timer_ID_s *) arg;
   /* do the operation in process context,
      not in interrupt context */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
   schedule_task (&(pTimerData->timerTask));
#else
   queue_work (pTAPItimersWq, &(pTimerData->timerTask));
#endif /* LINUX_VERSION_CODE */
}
#endif /* __KERNEL__ */


/* ============================= */
/* Defer work to process context */
/* ============================= */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20))
/* High Level Event Dispatcher function. */
static IFX_void_t Deferred_Worker (IFX_void_t *pWork)
{
   IFX_TAPI_EXT_EVENT_PARAM_t *pEvParam = (IFX_TAPI_EXT_EVENT_PARAM_t *) pWork;
   pEvParam->pFunc(pEvParam);
}
#else /* for Kernel newer or equal 2.6.20 */
static IFX_void_t Deferred_Worker (struct work_struct *pWork)
{
   IFX_TAPI_EXT_EVENT_PARAM_t *pEvParam = (IFX_TAPI_EXT_EVENT_PARAM_t *) pWork;
   pEvParam->pFunc(pEvParam);
}
#endif

/**
   Defer work to process context

   \param  pFunc        Pointer to function to be called.
   \param  pParam       Parameter passed to the function.
                        Attention: this must be a valid structure of
                        type IFX_TAPI_EXT_EVENT_PARAM_t as we need to
                        do some casting for Linux.

   \return TAPI_statusOk or TAPI_statusErr in case of an error.
*/
IFX_int32_t TAPI_DeferWork (IFX_void_t *pFunc, IFX_void_t *pParam)
{
   IFX_int32_t ret = TAPI_statusOk;
   IFX_TAPI_EXT_EVENT_PARAM_t *pEvParam = (IFX_TAPI_EXT_EVENT_PARAM_t *) pParam;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
   struct tq_struct           *pTapiTq;

   pTapiTq           = (struct tq_struct *)&pEvParam->tapiTq;
   INIT_TQUEUE(pTapiTq, Deferred_Worker, (IFX_void_t *)pEvParam);
   pEvParam->pFunc   = (IFX_void_t *)pFunc;
   if (schedule_task (pTapiTq) == 0)
   {
      ret = TAPI_statusWorkFail;
   }
#else
   struct work_struct         *pTapiWs;

   pTapiWs = (struct work_struct *) &pEvParam->tapiWs;
   pEvParam->pFunc   = (IFX_void_t *)pFunc;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20))
   INIT_WORK(pTapiWs, Deferred_Worker, (IFX_void_t *) pEvParam);
#else
   INIT_WORK(pTapiWs, Deferred_Worker);
#endif

   if (queue_work (pTAPIeventsWq, pTapiWs) == 0)
   {
      ret = TAPI_statusWorkFail;
   }
#endif /* LINUX_VERSION_CODE */
   return ret;
}

/**
   Executes the select for the channel fd

   \param  pTapiPriv    Pointer fd private data carrying the context.
   \param  pNode        node list.
   \param  pOpt         Optional argument, which contains needed information for
                        TAPI_OS_DrvSelectQueueAddTask.

   \return
   System event qualifier. Either 0 or TAPI_OS_SYSREAD

   \remarks
   This function needs operating system services, that are hidden by
   IFXOS macros.
*/
static IFX_int32_t TAPI_SelectCh (TAPI_FD_PRIV_DATA_t *pTapiPriv,
                                  TAPI_OS_drvSelectTable_t *pNode,
                                  TAPI_OS_drvSelectOSArg_t *pOpt)
{
   TAPI_CHANNEL *pTapiCh = (TAPI_CHANNEL* )pTapiPriv->pTapiCtx;
   IFX_int32_t   ret = 0;

   /* Register the voice channel waitqueues as wakeup source. */
   TAPI_OS_DrvSelectQueueAddTask (pOpt, &pTapiCh->semReadBlock.object, pNode);
   TAPI_OS_DrvSelectQueueAddTask (pOpt, &pTapiCh->wqRead, pNode);

   /* In Linux the wakeup is always to be used. */
   pTapiCh->nFlags |= CF_NEED_WAKEUP;

#ifdef TAPI_FEAT_PACKET
   /* If the channel has data set return value to allow immediate reading. */
   if (IFX_TAPI_UpStreamFifo_IsEmpty(pTapiCh, pTapiPriv->fifo_idx) != IFX_TRUE)
   {
      ret |= TAPI_OS_SYSREAD;
   }
#endif /* TAPI_FEAT_PACKET */

#ifdef TAPI_FEAT_FAX_T38
   {
      IFX_TAPI_DRV_CTX_t *pDrvCtx = (IFX_TAPI_DRV_CTX_t*)
                                    pTapiCh->pTapiDevice->pDevDrvCtx;
      IFX_TAPI_T38_STATUS_t TapiFaxStatus;

      /* Register the write waitqueue as wakeup source. */
      TAPI_OS_DrvSelectQueueAddTask (pOpt, &pTapiCh->wqWrite, pNode);

      memset (&TapiFaxStatus, 0, sizeof (IFX_TAPI_T38_STATUS_t));

      /* Get the Status from the low level driver */
      if (IFX_TAPI_PtrChk (pDrvCtx->COD.T38_Status_Get))
         (IFX_void_t)pDrvCtx->COD.T38_Status_Get(
                                       pTapiCh->pLLChannel, &TapiFaxStatus);

      if ((TapiFaxStatus.nStatus & IFX_TAPI_FAX_T38_TX_ON) &&
          (pTapiCh->bFaxDataRequest == IFX_TRUE))
      {
         /* task should write a new packet now */
         ret |= TAPI_OS_SYSWRITE;
      }
   }
#endif /* TAPI_FEAT_FAX_T38 */

   return ret;
}


/**
   Modify own thread priority.

   \param  newPriority  New thread priority.

   \return
   - IFX_SUCCESS priority changed.
   - IFX_ERROR priority not changed.
*/
IFX_int32_t TAPI_OS_ThreadPriorityModify(IFX_uint32_t newPriority)
{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
#if 0
   ret = setscheduler(current->pid, SCHED_FIFO, &sched_params);
#else /* 0 */
   current->policy = SCHED_FIFO;
   current->rt_priority = newPriority;
#endif /* 0 */
#else
   struct sched_param sched_params;
   IFX_int32_t ret = 0;

   sched_params.sched_priority = newPriority;

   ret = sched_setscheduler(current, SCHED_FIFO, &sched_params);

   if (ret < 0)
   {
      printk(KERN_ERR "Failed to set the thread priority to %d"
             " ret=%d\n", newPriority, ret);

      return IFX_ERROR;
   }
#endif

   return IFX_SUCCESS;
}


/**
   stop a kernel thread. Called by the removing instance

   \param  pThrCntrl    Pointer to a thread control struct.
   \param  pMutex       Pointer to mutex to unblock the thread to be killed.
*/
IFX_void_t TAPI_OS_ThreadKill(IFXOS_ThreadCtrl_t *pThrCntrl,
                              TAPI_OS_lock_t *pLock)
{
   if ((pThrCntrl) &&
       (IFXOS_THREAD_INIT_VALID(pThrCntrl) == IFX_TRUE) &&
       (pThrCntrl->thrParams.bRunning == 1))
   {
      /* signal the thread routine to shutdown */
      pThrCntrl->thrParams.bShutDown = IFX_TRUE;
      mb();

      /* Wake the process so that is able to see the shutdown flag and
         terminate itself. */
      if (pLock != IFX_NULL)
         TAPI_OS_LockRelease(pLock);

      wait_for_completion (&pThrCntrl->thrCompletion);

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,23)
      /* Now that we are sure the thread is in zombie state.
         We notify keventd to clean the process up. */
      kill_proc(2, SIGCHLD, 1);
#endif /* kernel 2.6.23 */
      pThrCntrl->bValid = IFX_FALSE;
   }
}

#ifdef ENABLE_HOTPLUG
static void TAPI_OS_MessageAdd(struct sk_buff *skb, char *msg)
{
   char *scratch;
   scratch = skb_put(skb, strlen(msg) + 1);
   sprintf(scratch, msg);
}

static void TAPI_OS_EventHookStatusReport(const IFX_uint16_t nChNum,
                                          const IFX_boolean_t bOn)
{
   struct sk_buff *skb;
   char buf[128];
   u64 seq;
   size_t len;
   char *scratch;

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,37)
   if (!uevent_sock) {
      printk(KERN_ERR
             "Unable to create netlink socket.\n");
      return;
   }
#endif
   len = strlen("0") + 2;
   skb = alloc_skb(len + 2048, GFP_KERNEL);
   if (!skb) {
      return;
   }

   scratch = skb_put(skb, len);

   sprintf(scratch, "%u@", (IFX_uint32_t)nChNum);
   TAPI_OS_MessageAdd(skb, "HOME=/");
   TAPI_OS_MessageAdd(skb, "PATH=/sbin:/bin:/usr/sbin:/usr/bin");
   TAPI_OS_MessageAdd(skb, "SUBSYSTEM=tapi");

   snprintf(buf, 128, "TAPI_CH_NO=%u", (IFX_uint32_t)nChNum);
   TAPI_OS_MessageAdd(skb, buf);

   snprintf(buf, 128, "TAPI_HOOK_STATE=%d", bOn ? 1 : 0);
   TAPI_OS_MessageAdd(skb, buf);

   seq = uevent_next_seqnum();
   snprintf(buf, 128, "SEQNUM=%llu", (unsigned long long)seq);
   TAPI_OS_MessageAdd(skb, buf);

   NETLINK_CB(skb).dst_group = 1;
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,37)
   netlink_broadcast(uevent_sock, skb, 0, 1, GFP_KERNEL);
#else
   broadcast_uevent(skb, 0, 1, GFP_KERNEL);
#endif
}

IFX_void_t TAPI_OS_EventReport(IFX_void_t *pEvent)
{
   IFX_TAPI_EVENT_t *pOsEvent = (IFX_TAPI_EVENT_t*)pEvent;

   switch (pOsEvent->id)
   {
   case IFX_TAPI_EVENT_FXS_ONHOOK:
      TAPI_OS_EventHookStatusReport(pOsEvent->ch, IFX_TRUE);
      break;
   case IFX_TAPI_EVENT_FXS_OFFHOOK:
      TAPI_OS_EventHookStatusReport(pOsEvent->ch, IFX_FALSE);
      break;
   default:
      /* ignore*/
      break;
   }
}
#endif /* #ifdef ENABLE_HOTPLUG*/

#ifdef TAPI_FEAT_KIOCTL
/**
   Open API to be called from kernel space.

   \param  name      Pointer to a dev node name (character string)

   \return
      IFX_TAPI_KIO_HANDLE  handler
*/
IFX_TAPI_KIO_HANDLE ifx_tapi_kopen(char *name)
{
   IFX_TAPI_Ch_KernRef_t *pRef;
   IFX_char_t *p, *p1, dev_name[8];
   IFX_int_t i;
   IFX_uint8_t ucMajor = 0;
   IFX_uint8_t ucMinor = 0;
   IFX_char_t *expected_dev_names[MAX_EXPECTED_LL_DRV] =
                                 {"vmmc", "vin", "dxt", "svip", "vxt"};

   /* sanity check */
   for (i=0; i<MAX_EXPECTED_LL_DRV; i++)
   {
      if ((p = strstr (name, expected_dev_names[i])) != IFX_NULL)
         break;
   }

   if (i == MAX_EXPECTED_LL_DRV)
   {
      /* device not found among expected names */
      return IFX_TAPI_KIO_FD_INVALID;
   }

   if (!strncmp (p, "dxt", 3))
   {
      /* patch for DuSLIC-xT dev name */
      strcpy (dev_name, "duslic");
   }
   else
   {
      strcpy (dev_name, expected_dev_names[i]);
   }

   /* extract minor number from name */
   p += strlen(expected_dev_names[i]);
   if (*p)
   {
      ucMinor = simple_strtol(p, &p1, 10);
   }

   /* find a proper major number */
   for (i = 0; i < TAPI_MAX_LL_DRIVERS; i++)
   {
      if (gHLDrvCtx[i].pDrvCtx != IFX_NULL &&
          IFX_NULL != strstr ((gHLDrvCtx[i].pDrvCtx)->drvName, dev_name))
      {
         ucMajor = (gHLDrvCtx[i].pDrvCtx)->majorNumber;
         break;
      }
   }

   if (i == TAPI_MAX_LL_DRIVERS && !ucMajor)
   {
      /* major number not found */
      return IFX_TAPI_KIO_FD_INVALID;
   }

   pRef = TAPI_OS_Malloc (sizeof(*pRef));
   if (IFX_NULL == pRef)
   {
      /* malloc error */
      return IFX_TAPI_KIO_FD_INVALID;
   }

   memset (pRef, 0, sizeof(*pRef));

   pRef->inode.i_rdev = MKDEV (ucMajor, ucMinor);

   if (0 != ifx_tapi_open (&pRef->inode, &pRef->file))
   {
      TAPI_OS_Free (pRef);
      return IFX_TAPI_KIO_FD_INVALID;
   }

   return (IFX_TAPI_KIO_HANDLE)pRef;
}
#endif /* TAPI_FEAT_KIOCTL */

#ifdef TAPI_FEAT_KIOCTL
/**
   Close API to be called from kernel space.

   \param  handle      IFX_TAPI_KIO_HANDLE type

   \return
   0 - if no error,
   otherwise error code
*/
int ifx_tapi_kclose(IFX_TAPI_KIO_HANDLE handle)
{
   int ret = 0;
   IFX_TAPI_Ch_KernRef_t *pRef = (IFX_TAPI_Ch_KernRef_t *)handle;

   if (pRef == IFX_NULL)
   {
      return -ENODEV;
   }

   ret = ifx_tapi_release (&pRef->inode, &pRef->file);
   if (0 != ret)
      return ret;

   TAPI_OS_Free (pRef);

   return 0;
}
#endif /* TAPI_FEAT_KIOCTL */

#ifdef TAPI_FEAT_KIOCTL
/**
   Ioctl API to be called from kernel space.

   \param  handle      IFX_TAPI_KIO_HANDLE type
   \param  command     Ioctl command
   \param  arg         Ioctl argument

   \return
      IFX_SUCCESS or IFX_ERROR
*/
int ifx_tapi_kioctl(IFX_TAPI_KIO_HANDLE handle, unsigned int command, void *arg)
{
   IFX_TAPI_DRV_CTX_t *pDrvCtx = IFX_NULL;
   IFX_TAPI_ioctlCtx_t ctx;
   IFX_int32_t ret;
   struct inode *inode = &((IFX_TAPI_Ch_KernRef_t *)handle)->inode;

   /*lint -save -e{30, 506} */
   switch (command) {
      case IFX_TAPI_CID_TX_SEQ_START:
         /*lint -fallthrough*/
      case IFX_TAPI_CID_TX_INFO_START:
         /*lint -fallthrough*/
      case IFX_TAPI_CID_CFG_SET:
         /*lint -fallthrough*/
      case IFX_TAPI_EVENT_MULTI_ENABLE:
         /*lint -fallthrough*/
      case IFX_TAPI_EVENT_MULTI_DISABLE:
         /*lint -fallthrough*/
      case IFX_TAPI_CAP_LIST:
         /* that services require nested copy from/to user-space,
            which are not supported for KIO interface */
         return IFX_ERROR;
      default:
         break;
   }
   /*lint -restore */

   /* get the device driver context */
   pDrvCtx = IFX_TAPI_DeviceDriverContextGet (MAJOR(inode->i_rdev));
   if (pDrvCtx == IFX_NULL)
   {
      return IFX_ERROR;
   }

   /* get the ioctl context: channel, device etc. */
   ret = TAPI_ioctlContextGet (pDrvCtx, MINOR(inode->i_rdev),
      command, (IFX_ulong_t)arg, IFX_FALSE, &ctx);
   if (IFX_SUCCESS == ret)
   {
      ret = TAPI_Ioctl (&ctx);
   }

   TAPI_ioctlContextPut ((IFX_ulong_t)arg, &ctx);

   return ret;
}
#endif /* TAPI_FEAT_KIOCTL */

/*lint -save -e{528, 546} */
module_init (ifx_tapi_module_init);
module_exit (ifx_tapi_module_exit);

/****************************************************************************/

MODULE_AUTHOR           ("Lantiq Beteiligungs-GmbH & Co.KG");
MODULE_DESCRIPTION      ("TAPI Driver - www.lantiq.com");
MODULE_SUPPORTED_DEVICE ("TAPI DEVICE");
MODULE_LICENSE          ("Dual BSD/GPL");

EXPORT_SYMBOL (IFX_TAPI_Register_LL_Drv);
EXPORT_SYMBOL (IFX_TAPI_Unregister_LL_Drv);
EXPORT_SYMBOL (IFX_TAPI_DeviceReset);
EXPORT_SYMBOL (IFX_TAPI_ReportResources);

EXPORT_SYMBOL (IFX_TAPI_Event_DispatchExt);
EXPORT_SYMBOL (IFX_TAPI_Event_Dispatch);
/*
available as macros
EXPORT_SYMBOL (IFX_TAPI_Event_DeferredDispatch);
EXPORT_SYMBOL (IFX_TAPI_Event_ImmediateDispatch);
*/

#ifdef TAPI_FEAT_CID
EXPORT_SYMBOL (TAPI_Phone_GetCidRxBuf);
EXPORT_SYMBOL (TAPI_Cid_Abort);
EXPORT_SYMBOL (TAPI_Cid_IsActive);
#endif /* TAPI_FEAT_CID */

#ifdef TAPI_HAVE_TIMERS
EXPORT_SYMBOL (TAPI_Create_Timer);
EXPORT_SYMBOL (TAPI_SetTime_Timer);
EXPORT_SYMBOL (TAPI_Stop_Timer);
EXPORT_SYMBOL (TAPI_Delete_Timer);
#endif /* TAPI_HAVE_TIMERS */

#ifdef TAPI_VERSION3
EXPORT_SYMBOL (TAPI_Tone_Set_Source);
#endif /* TAPI_VERSION3 */
EXPORT_SYMBOL (TAPI_ToneState);

EXPORT_SYMBOL(fifoInit);
EXPORT_SYMBOL(fifoReset);
EXPORT_SYMBOL(fifoPut);
EXPORT_SYMBOL(fifoGet);
EXPORT_SYMBOL(fifoPeek);
EXPORT_SYMBOL(fifoEmpty);
EXPORT_SYMBOL(fifoFree);
EXPORT_SYMBOL(fifoElements);
EXPORT_SYMBOL(fifoSize);

#ifdef TAPI_FEAT_KPI
EXPORT_SYMBOL(IFX_TAPI_KPI_WaitForData);
EXPORT_SYMBOL(IFX_TAPI_KPI_ReadData);
EXPORT_SYMBOL(IFX_TAPI_KPI_WriteData);
EXPORT_SYMBOL(IFX_TAPI_KPI_ReportEvent);
EXPORT_SYMBOL(IFX_TAPI_KPI_ChGet);
EXPORT_SYMBOL(IFX_TAPI_KPI_EgressTaskletRegister);
EXPORT_SYMBOL(irq_IFX_TAPI_KPI_PutToEgress);
EXPORT_SYMBOL(IFX_TAPI_KPI_ScheduleIngressHandling);
#endif /* TAPI_FEAT_KPI */

#ifdef TAPI_FEAT_PACKET
EXPORT_SYMBOL(IFX_TAPI_VoiceBufferGet);
EXPORT_SYMBOL(IFX_TAPI_VoiceBufferPut);
EXPORT_SYMBOL(IFX_TAPI_UpStreamFifo_Put);
EXPORT_SYMBOL(IFX_TAPI_UpStreamFifo_Reset);
EXPORT_SYMBOL(IFX_TAPI_VoiceBufferGetWithOwnerId);
EXPORT_SYMBOL(IFX_TAPI_VoiceBufferFreeAllOwnerId);
#ifdef TAPI_PACKET_OWNID
EXPORT_SYMBOL(IFX_TAPI_VoiceBufferChOwn);
#endif /* TAPI_PACKET_OWNID */
#ifdef TAPI_FEAT_POLL
EXPORT_SYMBOL(IFX_TAPI_DownStreamFifo_Handle_Get);
#endif /* TAPI_FEAT_POLL */
EXPORT_SYMBOL(IFX_TAPI_DownStream_RequestData);
EXPORT_SYMBOL(bufferPoolInit);
EXPORT_SYMBOL(bufferPoolGet);
EXPORT_SYMBOL(bufferPoolPut);
#endif /* TAPI_FEAT_PACKET */

/* packet statistic related exports */
EXPORT_SYMBOL(IFX_TAPI_Stat_Add);

/* FXO related exports */
#ifdef TAPI_FEAT_FXO
EXPORT_SYMBOL(IFX_TAPI_Register_DAA_Drv);
EXPORT_SYMBOL(IFX_TAPI_FXO_Event_Dispatch);
#endif /* TAPI_FEAT_FXO */
EXPORT_SYMBOL(IFX_TAPI_Update_SlicFxo);

#ifdef TAPI_FEAT_QOS
EXPORT_SYMBOL(IFX_TAPI_QOS_DrvRegister);
#endif /* TAPI_FEAT_QOS */

#ifdef TAPI_FEAT_KIOCTL
EXPORT_SYMBOL (ifx_tapi_kopen);
EXPORT_SYMBOL (ifx_tapi_kclose);
EXPORT_SYMBOL (ifx_tapi_kioctl);
#endif /* TAPI_FEAT_KIOCTL */
#endif /* __KERNEL__ */

#endif /* LINUX */

/*lint -restore*/
