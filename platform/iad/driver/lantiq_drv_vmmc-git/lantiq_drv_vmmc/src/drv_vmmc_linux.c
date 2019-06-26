/******************************************************************************

                            Copyright (c) 2006-2015
                        Lantiq Beteiligungs-GmbH & Co.KG
                             http://www.lantiq.com

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/**
   \file drv_vmmc_linux.c
   This file contains the implementation of the linux specific driver functions.
*/

/* ============================= */
/* Includes                      */
/* ============================= */
#include "drv_api.h"
#include "drv_vmmc_linux.h"

#ifdef LINUX
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/init.h>
#include <linux/wait.h>
#include <linux/vmalloc.h>
#include <linux/sched.h>
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,39))
   #include <linux/smp_lock.h>
#endif
#ifdef VMMC_FEAT_LINUX_PLATFORM_DRIVER
   #include <linux/platform_device.h>
   #include <linux/of.h>
#endif /* VMMC_FEAT_LINUX_PLATFORM_DRIVER */
   #include <asm/mipsregs.h> /* read_c0_count() */
#if (BSP_API_VERSION >= 5)
   #include <linux/clk.h>
#endif /* BSP_API_VERSION */

#ifdef VMMC_USE_PROC
   #include <linux/proc_fs.h>
   /* sequence file is available since Linux 2.6.32 */
   #include <linux/seq_file.h>
#endif /* VMMC_USE_PROC */

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

#include "drv_vmmc_init.h"
#ifdef EVALUATION
   #include "drv_vmmc_eval.h"
#endif /* EVALUATION */
#ifdef VMMC_FEAT_CLOCK_SCALING
   #include "drv_vmmc_pmc.h"
#endif /* VMMC_FEAT_CLOCK_SCALING */
#include "drv_vmmc_alm.h"
#include "drv_vmmc_alm_priv.h"      /* required to access nDcDcType */

/* ============================= */
/* Local Macros & Definitions    */
/* ============================= */

#ifdef VMMC_USE_PROC
   #if (LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0))
      #define PDE_DATA(inode) PDE(inode)->data
   #endif
#endif /* VMMC_USE_PROC */

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))

#ifndef wait_event_interruptible_timeout
#define __wait_event_interruptible_timeout(wq, condition, ret)                 \
do                                                                             \
{                                                                              \
   wait_queue_t __we;                                                          \
   init_waitqueue_entry(&__we, current);                                       \
                                                                               \
   add_wait_queue(&wq, &__we);                                                 \
   for (;;)                                                                    \
   {                                                                           \
      /* go halfway to sleep */                                                \
      set_current_state(TASK_INTERRUPTIBLE);                                   \
      /* check condition again */                                              \
      if (condition)                                                           \
         break;                                                                \
      ret = schedule_timeout(ret);                                             \
      /* timeout? then return */                                               \
      if (!ret)                                                                \
         break;                                                                \
   }                                                                           \
   /* restore the state in case we never fall asleep */                        \
   set_current_state(TASK_RUNNING);                                            \
   remove_wait_queue(&wq, &__we);                                              \
} while (0)

/**
   Linux 2.6 equivalent race condition free implmentation for
   IFXOS_WaitEvent_timeout.
*/
#define wait_event_interruptible_timeout(wq, condition, timeout)               \
({                                                                             \
   IFX_uint32_t __to = timeout;                                                \
   if (!(condition))                                                           \
      __wait_event_interruptible_timeout(wq, condition, __to);                 \
   __to;                                                                       \
})
#endif /* wait_event_interruptible_timeout */

#endif /* LINUX_VERSION_CODE */

/* ============================= */
/* Global variable declaration   */
/* ============================= */
IFX_uint16_t major      = 0; /* default for dynamic allocation */
IFX_uint16_t minorBase  = VMMC_MINOR_BASE;
IFX_char_t*  devName    = "vmmc";

#ifdef ENABLE_TRACE
extern IFX_uint32_t debug_level;
#endif /* ENABLE_TRACE */

#ifdef VMMC_FEAT_LINUX_PLATFORM_DRIVER
struct pinctrl *pinctrl;
struct pinctrl_state *pins_default, *pins_master, *pins_slave;
#endif

/* ============================= */
/* Global function declaration   */
/* ============================= */
#if (BSP_API_VERSION < 5)
   extern unsigned int ifx_get_cpu_hz(void); /* exported by the CGU driver */
#endif /* BSP_API_VERSION */

extern int VMMC_proc_ConGet (
                        struct seq_file *s);

/* ============================= */
/* Local function declaration    */
/* ============================= */
/* Linux operating system proc interface */
#ifdef VMMC_USE_PROC
static IFX_int32_t vmmc_proc_VersionGet (
                        struct seq_file *s);
static IFX_int32_t vmmc_proc_StatusGet  (
                        struct seq_file *s);
#ifdef HAVE_CONFIG_H
static IFX_int32_t vmmc_proc_ConfigureGet (
                        struct seq_file *s);
#endif /* HAVE_CONFIG_H */
#ifdef VMMC_FEAT_HDLC
static IFX_int32_t vmmc_proc_HdlcInfoGet (
                        struct seq_file *s);
#endif /* VMMC_FEAT_HDLC */
#ifdef VMMC_FEAT_CLOCK_SCALING
static int vmmc_proc_PmcGet (
                        struct seq_file *s);
#endif /* VMMC_FEAT_CLOCK_SCALING */
#ifdef ENABLE_TRACE
static int vmmc_proc_DebugGet (
                        struct seq_file *s);
#endif /* ENABLE_TRACE */

#ifdef VMMC_FEAT_HDLC
extern IFX_int32_t VMMC_RES_HDLC_InfoGet (
                        struct seq_file *s,
                        VMMC_DEVICE *pDev,
                        IFX_uint16_t nResNr);
#endif /* VMMC_FEAT_HDLC */

static int vmmc_proc_EntriesInstall (void);
static void vmmc_proc_EntriesRemove (void);
#endif /* VMMC_USE_PROC */

/* ============================= */
/* Local variable definition     */
/* ============================= */

/* ============================= */
/* Local function definition     */
/* ============================= */

/* ============================= */
/* Global function definition    */
/* ============================= */

/**
   This function allocates a temporary kernel buffer
      and copies the user buffer contents to it.

   \param  p_buffer     Pointer to the user buffer.
   \param  size         Size of the buffer.

   \return
   Pointer to kernel buffer or NULL.

   \remarks
   Buffer has to be released with \ref OS_UnmapBuffer.
*/
IFX_void_t* VMMC_OS_MapBuffer(IFX_void_t* p_buffer, IFX_uint32_t size)
{
   IFX_void_t* kernel_buffer;

   if (p_buffer == NULL)
   {
      return NULL;
   }
   kernel_buffer = vmalloc (size);
   if (kernel_buffer == NULL)
   {
      return NULL;
   }
   if (copy_from_user(kernel_buffer, p_buffer, size) > 0) /*lint !e506 !e774 */
   {
      vfree (kernel_buffer);
      return NULL;
   }

   return kernel_buffer;
}


/**
   This function releases a temporary kernel buffer.

   \param  p_buffer     Pointer to the kernel buffer.

   \remarks
   Counterpart of \ref OS_MapBuffer.
*/
IFX_void_t VMMC_OS_UnmapBuffer(IFX_void_t* p_buffer)
{
   if (p_buffer != NULL)
   {
      vfree (p_buffer);
      p_buffer = NULL;
   }
}


#ifdef VMMC_USE_PROC
/* ============================= */
/* /proc filesystem support      */
/* ============================= */


/**
 * Process MPS proc file output.
 * This function outputs the history buffer showing the messages
 * sent to command mailbox so far.
 *
 * \param   s        Pointer to seq_file struct.
 * \return  0 on success
 * \ingroup Internal
 */
static IFX_int32_t vmmc_proc_StatusGet (struct seq_file *s)
{
   IFX_uint16_t nDev,
                nCh;
   const char const *DcDcNames[] =
      {
         "DEFAULT (Inverting Buck-Boost Converter)",
         "Inverting Buck-Boost Converter",
         "Combined Inverting Buck-Boost Converter"
      };
   const char *pConverterName;

   /* Loop over all devices */
   for (nDev = 0; nDev < VMMC_MAX_DEVICES; nDev++)
   {
      VMMC_DEVICE *pDev = IFX_NULL;
      VMMC_CHANNEL *pCh;

      if (VMMC_GetDevice (nDev, &pDev) == IFX_SUCCESS)
      {
         /* Headline */
         seq_printf(s, "-- Device: #%d --\n", nDev);

         seq_printf(s, "  SSI status : %s (recoveries: %d)\n",
                       pDev->bSSIcrash ? "CRASHED!" : "ok",
                       pDev->nSlicRecoveryCnt);

         /* Attached DC/DC converter type */
         if (pDev->sdd.nAllowedDcDcType < (sizeof(DcDcNames)/sizeof(char *)))
         {
            pConverterName = DcDcNames[pDev->sdd.nAllowedDcDcType];
            seq_printf(s, "  Connected DC/DC type: %s\n", pConverterName);
         }
         else
         {
            seq_printf(s, "  Connected DC/DC type: %d (unknown)\n",
                          pDev->sdd.nAllowedDcDcType - VMMC_DCDC_TYPE_IBB);
         }

         /* SDD channel status */
         for (nCh = 0; nCh < pDev->caps.nALI; nCh++)
         {
            pCh = &pDev->pChannel[nCh];

            if (pCh->pALM)
            {
               pConverterName = "unknown";
               if (pCh->pALM->nDcDcType <= (sizeof(DcDcNames)/sizeof(char *))-1)
               {
                  pConverterName = DcDcNames[pCh->pALM->nDcDcType];
               }
               if (pCh->pALM->line_type_fxs == IFX_FALSE)
               {
                  pConverterName = "n/a";
               }

               seq_printf(s, "  Channel %d: %s - %s\n",
                             nCh,
                             pCh->pALM->line_type_fxs ? "FXS" : "FXO",
                             pConverterName);
            }
         } /* for all ALI channels */

         seq_printf(s, "  MIPS OL cnt: %d\n", pDev->nMipsOl);

#ifdef VMMC_FEAT_PCM
         if (pDev->nDevState & DS_PCM_EN)
         {
            IFX_uint32_t   i, j, Idx, BitValue;

            /* Print the timeslot allocation bitmap in groups of 8 bits. */

            seq_printf(s, "  PCM TS RX: ");
            for (i = j = 0;  i < pDev->nMaxTimeslot; i++)
            {
               Idx = i >> 5;
               BitValue = 1 << (i & 0x1F);
               seq_putc(s,(pDev->PcmRxTs[0][Idx] & BitValue) ? '1' : '0');
               if ((++j % 8) == 0)
               {
                  seq_putc(s, ' ');
               }
            }
            seq_putc(s, '\n');

            seq_printf(s, "  PCM TS TX: ");
            for (i = j = 0;  i < pDev->nMaxTimeslot; i++)
            {
               Idx = i >> 5;
               BitValue = 1 << (i & 0x1F);
               seq_putc(s,(pDev->PcmTxTs[0][Idx] & BitValue) ? '1' : '0');
               if ((++j % 8) == 0)
               {
                  seq_putc(s, ' ');
               }
            }
            seq_putc(s, '\n');
         }
#endif /* VMMC_FEAT_PCM */
      }
   }

   return 0;
}


/**
   Process MPS proc file output.
   This function outputs the history buffer showing the messages
   sent to command mailbox so far.

   \param   s        Pointer to seq_file struct.
   \return  0 on success
   \ingroup Internal
*/
static IFX_int32_t vmmc_proc_VersionGet (struct seq_file *s)
{
   seq_printf (s, "%s\n",
                  &DRV_VMMC_WHATVERSION[4]);
   seq_printf (s, "Compiled on %s, %s for Linux kernel %s\n",
                  __DATE__, __TIME__, UTS_RELEASE);
   return 0;
}


#ifdef HAVE_CONFIG_H
/**
   Read the configure parameters of the driver.

   \param   s        Pointer to seq_file struct.
   \return  0 on success
*/
static IFX_int32_t vmmc_proc_ConfigureGet (struct seq_file *s)
{
   seq_printf (s, "configure %s\n", &DRV_VMMC_WHICHCONFIG[0]);

   return 0;
}
#endif /* HAVE_CONFIG_H */


#ifdef VMMC_FEAT_HDLC
/**
   Read the HDLC information of the driver.

   \param   s        Pointer to seq_file struct.
   \return  0 on success
   \ingroup Internal
*/
static IFX_int32_t vmmc_proc_HdlcInfoGet (struct seq_file *s)
{
   IFX_uint16_t nDev = 0;

   seq_printf (s, "%3s %3s %3s "
                  "%5s "
                  "%6s "
                  "%5s "
                  "%s\n",
                  "Dev", "Ch", "Res",
                  "state",
                  "DD_MBX",
                  "fwbuf",
                  "fifo");

   for (nDev = 0; nDev < VMMC_MAX_DEVICES; nDev++)
   {
      VMMC_DEVICE *pDev = IFX_NULL;

      if (VMMC_GetDevice (nDev, &pDev) == IFX_SUCCESS)
      {
         IFX_uint16_t nResNr = 0;

         for (nResNr = 1; nResNr <= pDev->caps.nHDLC; nResNr++)
         {

            VMMC_RES_HDLC_InfoGet (s, pDev, nResNr);
         }
      }
   }

   return 0;
}
#endif /* VMMC_FEAT_HDLC */


#ifdef VMMC_FEAT_CLOCK_SCALING
/**
   Read the Power Management Control status from the driver.

   \param   s        Pointer to seq_file struct.
   \return  0 on success
*/
/*lint -e{715} yes - count and data are unused */
static int vmmc_proc_PmcGet (struct seq_file *s)
{
   IFX_uint16_t       nDev, nCh;
   IFX_int32_t        nClockState, nRequestedClockState;

   VMMC_PMC_ClockStateGet(&nClockState, &nRequestedClockState);

   seq_printf (s, "Clock reported: %d / requested: %d\n\n",
                  nClockState,
                  nRequestedClockState);

   /* Print status from all devices */
   for (nDev=0; nDev < VMMC_MAX_DEVICES; nDev++)
   {
      VMMC_DEVICE *pDev;

      if (VMMC_GetDevice (nDev, &pDev) == IFX_SUCCESS)
      {
         /* Headline */
         seq_printf (s, "-- Device: #%d DSP %s / SSI %s --\n",
                        nDev, pDev->bFwActive ? "busy" : "idle",
                        pDev->bDartActive ? "active" : "sleeps");

         /* Loop over all channels */
         for (nCh=0; nCh < VMMC_MAX_CH_NR; nCh++)
         {
            if ((pDev->pPMC != IFX_NULL) &&
                (pDev->pPMC[nCh].value != 0))
            {
               seq_printf(s, "CH %d: ", nCh);

               seq_printf(s, "%s%s%s%s%s%s%s%s%s%s%s%s",
                  pDev->pPMC[nCh].bits.pcm_ch ? "PCM CH, " : "",
                  pDev->pPMC[nCh].bits.pcm_lec ? "PCM LEC, " : "",
                  pDev->pPMC[nCh].bits.pcm_es ? "PCM ES, " : "",
                  pDev->pPMC[nCh].bits.pcm_lb ? "PCM LB, " : "",
                  pDev->pPMC[nCh].bits.hdlc_ch ? "HDLC CH, " : "",
                  pDev->pPMC[nCh].bits.alm_lec ? "ALM LEC, " : "",
                  pDev->pPMC[nCh].bits.alm_es ? "ALM ES, " : "",
                  pDev->pPMC[nCh].bits.cod_ch ? "COD CH, " : "",
                  pDev->pPMC[nCh].bits.cod_agc ? "COD AGC, " : "",
                  pDev->pPMC[nCh].bits.cod_fdp ? "COD FAX, " : "",
                  pDev->pPMC[nCh].bits.dect_ch ? "DECT CH, " : "",
                  pDev->pPMC[nCh].bits.dect_utg ? "DECT UTG, " : "");

               seq_printf(s, "%s%s%s%s%s%s%s%s%s\n",
                  pDev->pPMC[nCh].bits.sig_ch ? "SIG CH, " : "",
                  pDev->pPMC[nCh].bits.sig_dtmfd ? "SIG DTMFd, " : "",
                  pDev->pPMC[nCh].bits.sig_dtmfg ? "SIG DTMFg, " : "",
                  pDev->pPMC[nCh].bits.sig_fskd ? "SIG FSKd, " : "",
                  pDev->pPMC[nCh].bits.sig_fskg ? "SIG FSKg, " : "",
                  pDev->pPMC[nCh].bits.sig_mftd ? "SIG MFTD, " : "",
                  pDev->pPMC[nCh].bits.sig_utg1 ? "SIG UTG1, " : "",
                  pDev->pPMC[nCh].bits.sig_utg2 ? "SIG UTG2, " : "",
                  pDev->pPMC[nCh].bits.sig_cptd ? "SIG CPTD, " : "");
            }
         }
      }
   }

   return 0;
}
#endif /* VMMC_FEAT_CLOCK_SCALING */


#ifdef ENABLE_TRACE
/**
   Read the debug level setting from the driver.

   \param   s        Pointer to seq_file struct.
   \return  0 on success
*/
/*lint -e{715} yes - count and data are unused */
static int vmmc_proc_DebugGet(struct seq_file *s)
{
   IFX_uint32_t         level = debug_level;
   char                *level_name[] =
                           {
                              "unknown",
                              "VERBOSE",   /* LOW */
                              "NORMAL",    /* NORMAL */
                              "SILENT",    /* HIGH */
                              "NO OUTPUT"  /* OFF */
                           };

   if (level >= (sizeof(level_name) / sizeof(char *)))
   {
      level = 0;
   }

   /* Headline */
   seq_printf (s,
               "VMMC driver global debug level: %d (%s)\n",
               debug_level, level_name[level]);
   return 0;
}


/**
   Set the debug level of the driver.

   This function just looks at the first character of input. If it is within
   the range of 1 -- 4 this is set as the debug level. All other input is
   ignored.

   \param   file    File structure for proc file.
   \param   buffer  Buffer holding the data.
   \param   count   Number of characters in buffer.
   \param   data    Unused.

   \return  count   Number of processed characters.
*/
static int vmmc_proc_DebugSet(struct file *file, const char *buffer,
                              unsigned long count, void *data)
{
   char input_line[1];

   if (count < 1)
   {
      /* error: No input. */
      return -EFAULT;
   }

   if (copy_from_user(input_line, buffer, 1) != 0)
   {
      /* error: Data copy from user space failed. */
      return -EFAULT;
   }

   /* Look only at the first character. */
   if (input_line[0] >= '1' && input_line[0] <= '4')
   {
      debug_level = (input_line[0] - '1') + DBG_LEVEL_LOW;
      SetTraceLevel(VMMC, debug_level);
   }
   else
   {
      /* error: debug level out of range (1 -- 4). */
      return -EFAULT;
   }

   return count;
}
#endif /* ENABLE_TRACE */


typedef void (*vmmc_dump) (struct seq_file *s);

static int vmmc_proc_show ( struct seq_file *s, void *p )
{
   vmmc_dump dump = s->private;

   if (dump != NULL)
      dump(s);

   return 0;
}

static int vmmc_proc_open ( struct inode *inode, struct file *file )
{
   return single_open (file, vmmc_proc_show, PDE_DATA(inode));
}

struct proc_entry
{
   const char *name;
   void *read_function;
   void *write_function;
   struct file_operations fops;
};

static struct proc_entry proc_entries[] =
{
   { "version", vmmc_proc_VersionGet },
   { "status", vmmc_proc_StatusGet },
   { "mapping", VMMC_proc_ConGet },
#ifdef HAVE_CONFIG_H
   { "configure", vmmc_proc_ConfigureGet },
#endif /* HAVE_CONFIG_H */
#ifdef VMMC_FEAT_CLOCK_SCALING
   { "power", vmmc_proc_PmcGet },
#endif /* VMMC_FEAT_CLOCK_SCALING */
#ifdef VMMC_FEAT_HDLC
   { "hdlc", vmmc_proc_HdlcInfoGet},
#endif /* VMMC_FEAT_HDLC */
#ifdef ENABLE_TRACE
   { "debug_level", vmmc_proc_DebugGet, vmmc_proc_DebugSet},
#endif /* ENABLE_TRACE */
#ifdef EVALUATION
   { "interrupt", VMMC_EvalProcInterruptGet, VMMC_EvalProcInterruptPut},
#endif /* EVALUATION */
};

static void vmmc_proc_EntryCreate ( struct proc_dir_entry *parent_node,
                 struct proc_entry *proc_entry)
{
   memset(&proc_entry->fops, 0, sizeof(struct file_operations));
   proc_entry->fops.owner   = THIS_MODULE;
   proc_entry->fops.open    = vmmc_proc_open;
   proc_entry->fops.read    = seq_read;
   proc_entry->fops.write   = proc_entry->write_function;
   proc_entry->fops.llseek  = seq_lseek;
   proc_entry->fops.release = single_release;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,30))
   {
      struct proc_dir_entry *entry;

      entry = create_proc_entry (proc_entry->name, 0, parent_node);
      if (entry)
      {
         entry->proc_fops = &proc_entry->fops;
         entry->data = proc_entry;
         entry->owner = THIS_MODULE;
      }
   }
#else
   proc_create_data ( proc_entry->name, 0, parent_node,
            &proc_entry->fops, proc_entry->read_function);
#endif
}


/**
   Initialize and install the /proc filesystem entries.

   \return
   IFX_SUCCESS or IFX_ERROR
*/
static int vmmc_proc_EntriesInstall (void)
{
   struct proc_dir_entry *driver_proc_node;

   /* install the proc entry */
   TRACE(VMMC, DBG_LEVEL_NORMAL, ("vmmc: using proc fs\n"));

   driver_proc_node = proc_mkdir( "driver/" DEV_NAME, NULL);
   if (driver_proc_node != NULL)
   {
      IFX_uint32_t i;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,30))
      driver_proc_node->owner = THIS_MODULE;
#endif /* < Linux 2.6.30 */

      for(i=0; i<sizeof(proc_entries)/sizeof(proc_entries[0]); i++) {
         vmmc_proc_EntryCreate (driver_proc_node, &proc_entries[i]);
      }
   }
   else
   {
      TRACE(VMMC, DBG_LEVEL_HIGH,("vmmc: cannot create proc entry\n"));
      return IFX_ERROR;
   }

   return IFX_SUCCESS;
}


/**
   Remove proc filesystem entries.
*/
static void vmmc_proc_EntriesRemove(void)
{
#ifdef ENABLE_TRACE
   remove_proc_entry("driver/" DEV_NAME "/debug_level", 0);
#endif /* ENABLE_TRACE */
#ifdef VMMC_FEAT_CLOCK_SCALING
   remove_proc_entry("driver/" DEV_NAME "/power", 0);
#endif /* VMMC_FEAT_CLOCK_SCALING */
#ifdef EVALUATION
   remove_proc_entry("driver/" DEV_NAME "/interrupt", 0);
#endif /* #ifdef EVALUATION */
   remove_proc_entry("driver/" DEV_NAME "/version", 0);
   remove_proc_entry("driver/" DEV_NAME "/status", 0);
   remove_proc_entry("driver/" DEV_NAME "/mapping", 0);
#ifdef VMMC_FEAT_HDLC
   remove_proc_entry("driver/" DEV_NAME "/hdlc", 0);
#endif /* VMMC_FEAT_HDLC */
#ifdef HAVE_CONFIG_H
   remove_proc_entry("driver/" DEV_NAME "/configure", 0);
#endif /* HAVE_CONFIG_H */
   remove_proc_entry("driver/" DEV_NAME , 0);

   return;
}

#endif /* VMMC_USE_PROC */


/* ============================= */
/* interrupt locking/unlocking   */
/* ============================= */

/**
   Disables the irq line if the driver is in interrupt mode and irq line is
   actually enabled according to device flag bIntEnabled.
   Disable the global interrupt if the device is not in polling mode and no
   interrupt line is connected.

   \param  pLLDev       Vmmc device handle.

   \remarks
   If the driver works in Polling mode, nothing is done.
   If the driver works in interrupt mode and the irq was already disabled
   (flag bIntEnabled is IFX_FALSE ), the OS disable function will
   not be called.

   Very important: It is assumed that disable and enable irq are done
   subsequently and that no routine calling disable/enable is executed
   inbetween as stated in following code example:

   \verbatim
   Allowed :

      Vmmc_IrqLockDevice;
      .. some instructions
      Vmmc_IrqUnlockDevice

   Not allowed:

      routineX (IFX_int32_t x)
      {
         Vmmc_IrqLockDevice;
         .. some instructions;
         Vmmc_IrqUnlockDevice
      }

      routineY (IFX_int32_t y)
      {
         Vmmc_IrqLockDevice;
         routine (x);    <---------------- routineX unlocks interrupts..
         be carefull!
         ... some more instructions;
         Vmmc_IrqUnlockDevice;
      }
   \endverbatim
*/
IFX_void_t Vmmc_IrqLockDevice (IFX_TAPI_LL_DEV_t *pLLDev)
{
   VMMC_DEVICE *pDev = (VMMC_DEVICE *) pLLDev;

   if (pDev->nIrqMask == 0)
   {
      VMMC_DISABLE_IRQGLOBAL(pDev->nIrqMask);
   }
}


/**
   Enables the irq line if the driver is in interrupt mode and irq line is
   actually disabled according to device flag bIntEnabled.
   Enable the global interrupt, if this was disabled by 'Vmmc_IrqDisable'

   \param  pLLDev       Vmmc device handle.
*/
IFX_void_t Vmmc_IrqEnable (IFX_TAPI_LL_DEV_t *pLLDev)
{
   VMMC_DEVICE *pDev = (VMMC_DEVICE *) pLLDev;
   vmmc_interrupt_t* pIrq = pDev->pIrq;

   /* invoke board or os routine to enable irq */
   VMMC_ENABLE_IRQLINE(pIrq->nIrq);
}


/**
   Enables the irq line if the driver is in interrupt mode and irq line is
   actually disabled according to device flag bIntEnabled.
   Enable the global interrupt, if this was disabled by 'Vmmc_IrqEnable'

   \param  pLLDev       Vmmc device handle.
*/
IFX_void_t Vmmc_IrqDisable (IFX_TAPI_LL_DEV_t *pLLDev)
{
   VMMC_DEVICE *pDev = (VMMC_DEVICE *) pLLDev;
   vmmc_interrupt_t* pIrq = pDev->pIrq;

   /* invoke board or os routine to disable irq */
   VMMC_DISABLE_IRQLINE(pIrq->nIrq);
}


/**
   Enables the irq line if the driver is in interrupt mode and irq line is
   actually disabled according to device flag bIntEnabled.
   Enable the global interrupt, if this was disabled by 'Vmmc_IrqLockDevice'

   \param  pLLDev       Vmmc device handle.

   Remarks
   cf Remarks of Vmmc_IrqLockDevice () in this file.
*/
IFX_void_t Vmmc_IrqUnlockDevice (IFX_TAPI_LL_DEV_t *pLLDev)
{
   VMMC_DEVICE * pDev = (VMMC_DEVICE *) pLLDev;

   if (pDev->nIrqMask != 0)
   {
      VMMC_OS_INTSTAT nMask = pDev->nIrqMask; /*lint --e{529} */

      pDev->nIrqMask = 0;
      VMMC_ENABLE_IRQGLOBAL(nMask);
   }
}


/**
   Enable the global interrupt if it was disabled by 'Vmmc_IrqLockDevice'.
   Do not enable the interrupt line if it is configured on the device.
   This function has to be called in error case inside the driver to unblock
   global interrupts but do not device interrupt. The interrupt line for that
   specific device is not enable in case of an error.
   \param  pDev         Vmmc device handle.

   Remarks
   cf Remarks of Vmmc_IrqLockDevice () in this file.
*/
IFX_void_t Vmmc_IrqUnlockGlobal (VMMC_DEVICE *pDev)
{
   if (pDev->nIrqMask != 0)
   {
      VMMC_OS_INTSTAT nMask = pDev->nIrqMask; /*lint --e{529} */
      pDev->nIrqMask = 0;
      VMMC_ENABLE_IRQGLOBAL(nMask);
   }

#ifdef DEBUG_INT
   pIrq->nCount--;
#endif /* DEBUG_INT */
}


/**
   Race condition free implmentation for IFXOS_WaitEvent_timeout
   \param  pDev         Reference to device context.

   \return  >0      wakeup occured, remaining timeout
            0       timeout

   \remarks
   Porting instructions

   Different OS implement the wait queue mechanism in different ways. The
   major difference is the way they handle a wakeup which occurs before
   the thread is completely sleeping. Linux is known not to handle this
   "early" wakeup properly, i.e. we'll sleep forever - or when we are lucky
   the same event occurs again. To prevent this case, we need to check a
   condition "half the way to sleep" - where we are still able to rollback
   our preparations to sleep.
   If your OS doesn't require this special handling, you can continue using
   IFXOS_WaitEvent_timeout - or your specific port of this macro - (and
   ignore the additional condition in pDev->bCmdOutBoxData).
*/
IFX_int32_t VMMC_WaitForCmdMbxData(VMMC_DEVICE *pDev)
{
   /*lint -esym(715, pDev) */
   return wait_event_interruptible_timeout(pDev->mpsCmdWakeUp.object,
                                           pDev->bCmdOutBoxData,
                                           (CMD_MBX_RD_TIMEOUT_MS * HZ / 1000));
}


/**
   Waiting for completion of SDD Operating Mode change or timeout.

   \param  pCh          Pointer to VMMC channel context .

   \return  >0      wakeup occured, remaining jiffies
            0       timeout
            <0      interrupted by a signal
*/
IFX_int32_t VMMC_WaitForSddOpmodeChEvt(VMMC_CHANNEL *pCh)
{
   return wait_event_interruptible_timeout(pCh->pALM->sdd_event.object,
                                           !pCh->pALM->bOpmodeChangePending,
                                           (SDD_EVT_TIMEOUT_MS * HZ / 1000));
}


/* ============================= */
/* Linux kernel thread support   */
/* ============================= */

/**
   Modify own thread priority.

   \param  newPriority  New thread priority.

   \return
   - IFX_SUCCESS priority changed.
   - IFX_ERROR priority not changed.
*/
IFX_int32_t VMMC_OS_ThreadPriorityModify(IFX_uint32_t newPriority)
{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
   current->policy = SCHED_FIFO;
   current->rt_priority = newPriority;
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
   Stop a kernel thread. Called by the removing instance.

   \param  pThrCntrl    Pointer to a thread control struct.
*/
IFX_void_t VMMC_OS_ThreadKill(VMMC_OS_ThreadCtrl_t *pThrCntrl)
{
   if ((pThrCntrl) &&
       (VMMC_OS_THREAD_INIT_VALID(pThrCntrl) == IFX_TRUE) &&
       (pThrCntrl->thrParams.bRunning == 1))
   {
      /* signal the thread routine to shutdown */
      pThrCntrl->thrParams.bShutDown = IFX_TRUE;
      mb();
      /* Send a signal to wake the process when it is blocked but
         interruptible. */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,28)
      kill_proc(pThrCntrl->tid, SIGKILL, 1);
#else
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,8,0)
      kill_pid(find_vpid(pThrCntrl->tid), SIGKILL, 1);
#else
      kill_pid(get_task_pid(pThrCntrl->tid, PIDTYPE_PID), SIGKILL, 1);
#endif /* IFXOS_VERSION_CHECK_L_THAN(1, 5, 19) */
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(2,6,28) */
      wait_for_completion (&pThrCntrl->thrCompletion);

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,23)
      /* Now that we are sure the thread is in zombie state.
         We notify keventd to clean the process up. */
      kill_proc(2, SIGCHLD, 1);
#endif /* kernel 2.6.23 */
      pThrCntrl->bValid = IFX_FALSE;
   }
}


/**
   Calculate the time difference to the given timestamp.

   \param  last_timestamp     Value of timestamp from the last call.
   \param  current_timestamp  Current timestamp from this call.

   \return
   Difference to the given timestamp in units of 1/8 ms.
*/
IFX_uint32_t VMMC_OS_GetTimeDelta(
                        IFX_uint32_t last_timestamp,
                        IFX_uint32_t *current_timestamp)
{
   IFX_uint32_t   now, time_difference;
   IFX_uint32_t   rate, divider;

   /* Use the MIPS core count register to get a timestamp. */

   /* Read the MIPS core count register. */
   now = read_c0_count();

   /* This is an 32 bit unsigned int subtraction. */
   time_difference = now - last_timestamp;

   /** Returns the MIPS core frequency in Hz. */
#if !defined(SYSTEM_FALCON)
   #if (BSP_API_VERSION < 5)
      rate = ifx_get_cpu_hz();
   #else /* BSP_API_VERSION */
      {
         struct clk *clk = clk_get_sys("cpu", "cpu");
         rate = clk_get_rate(clk);
      }
   #endif /* BSP_API_VERSION */
#else /* SYSTEM_FALCON */
   #define CLOCK_400M   400000000
   rate = CLOCK_400M;
#endif /* SYSTEM_FALCON */

   /* The timestamp counter runs with half the CPU frequency.
      Addtionally the return value is defined as 1/8ms steps. */
   divider = (rate / 2) / 8000;

   /* convert time difference to units of 1/8 ms */
   time_difference /= divider;

   *current_timestamp = now;
   return time_difference;
}

#ifdef VMMC_FEAT_LINUX_PLATFORM_DRIVER
int vmmc_pcm_pin_config(unsigned int mode)
{
   /* do nothing if a platform doesn't have registered TDM pinmux */
   if (!pins_master || !pins_slave)
      return;

   if (mode == 2)
      return pinctrl_select_state(pinctrl, pins_master);
   else
      return pinctrl_select_state(pinctrl, pins_slave);
}
#endif

/* ============================= */
/* Linux module support          */
/* ============================= */

#ifdef VMMC_FEAT_LINUX_PLATFORM_DRIVER
static int vmmc_probe(struct platform_device *pdev);
static int vmmc_remove(struct platform_device *pdev);
extern int ifx_mps_probe(struct platform_device *pdev);

static const struct of_device_id vmmc_match_table[] = {
        { .compatible = "lantiq,mps-xrx100" },
        {},
};
MODULE_DEVICE_TABLE(of, vmmc_match_table);

static struct platform_driver vmmc_driver =
{
   .probe = vmmc_probe,
   .remove = vmmc_remove,
   .driver =
   {
      .name = "lantiq-vmmc",
      .owner = THIS_MODULE,
      .of_match_table = vmmc_match_table
   }
};



/**
   Linux platform driver probe function.

   \param  pdev         Pointer to struct platform_device.

   \return
   0 Successful
   !0 Failed to find the config or the device.
*/
static int vmmc_probe(struct platform_device *pdev)
{
   int   ret;

#ifdef VMMC_WITH_MPS
   ret = ifx_mps_probe(pdev);
   if (ret)
       return ret;
#endif

   pinctrl = devm_pinctrl_get(&pdev->dev);
   if (IS_ERR(pinctrl)) {
      dev_err(&pdev->dev, "failed to get pinctrl handle\n");
      return IFX_ERROR;
   }

   pins_default = pinctrl_lookup_state(pinctrl, "default");
   if (IS_ERR(pins_default)) {
      dev_err(&pdev->dev, "failed to get pinctrl_state 'default'\n");
      return IFX_ERROR;
   }

   pins_master = pinctrl_lookup_state(pinctrl, "master");
   if (IS_ERR(pins_master)) {
      dev_err(&pdev->dev, "failed to get pinctrl_state 'master'\n");
      pins_master = NULL;
   }

   pins_slave = pinctrl_lookup_state(pinctrl, "slave");
   if (IS_ERR(pins_slave)) {
      dev_err(&pdev->dev, "failed to get pinctrl_state 'slave'\n");
      pins_slave = NULL;
   }

   ret = VMMC_DeviceDriverStart();

   return ret;
}


/**
   Linux platform driver remove function.

   \return Always zero to indicate success.
*/
static int vmmc_remove(struct platform_device *pdev)
{
   VMMC_DeviceDriverStop();

   pinctrl_select_state(pinctrl, pins_default);

   return 0;
}
#endif /* VMMC_FEAT_LINUX_PLATFORM_DRIVER */


/**
   Called by the kernel to initialise the module.

   \return Always zero to indicate success.
*/
static int __init vmmc_module_init(void)
{
#ifdef VMMC_USE_PROC
   vmmc_proc_EntriesInstall();
#endif /* VMMC_USE_PROC */

#ifdef VMMC_FEAT_LINUX_PLATFORM_DRIVER
   return platform_driver_register(&vmmc_driver);
#else
   return VMMC_DeviceDriverStart();
#endif /* VMMC_FEAT_LINUX_PLATFORM_DRIVER */
}


/**
   Called by the kernel to clean up the module when unloaded.
*/
static void __exit vmmc_module_exit(void)
{
#ifdef VMMC_USE_PROC
   vmmc_proc_EntriesRemove();
#endif /* VMMC_USE_PROC */

#ifdef VMMC_FEAT_LINUX_PLATFORM_DRIVER
   return platform_driver_unregister(&vmmc_driver);
#else
   VMMC_DeviceDriverStop();
#endif /* VMMC_FEAT_LINUX_PLATFORM_DRIVER */

   TRACE (VMMC, DBG_LEVEL_LOW, ("cleaned up %s module.\n", DEV_NAME));
}


#ifdef MODULE
MODULE_DESCRIPTION("VMMC (VoiceMacroMipsCore) device driver - www.lantiq.com");
MODULE_AUTHOR("Lantiq Beteiligungs-GmbH & Co.KG");
MODULE_SUPPORTED_DEVICE("XRX100, XRX200, XRX300, FALCON");
MODULE_LICENSE("Dual BSD/GPL");

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,17))
MODULE_PARM(major, "h" );
MODULE_PARM(minorBase, "h" );
MODULE_PARM(devName, "s" );
#ifdef ENABLE_TRACE
MODULE_PARM(debug_level, "i");
#endif /* ENABLE_TRACE */
#else
module_param(major, ushort, 0);
module_param(minorBase, ushort, 0);
module_param(devName, charp, 0);
#ifdef ENABLE_TRACE
module_param(debug_level, uint, 0);
#endif /* ENABLE_TRACE */
#endif /* < 2.6.17 */

MODULE_PARM_DESC(major ,"Device Major number");
MODULE_PARM_DESC(minorBase ,"Number of devices to be created");
MODULE_PARM_DESC(devName ,"Name of the device");
#ifdef ENABLE_TRACE
MODULE_PARM_DESC(debug_level, "Debug level: 1 (maxium) - 4 (no) trace output");
#endif /* ENABLE_TRACE */

/*lint -save -e19  this is not as useless as lint thinks */
module_init(vmmc_module_init);
module_exit(vmmc_module_exit);
/*lint -restore */
#endif

#endif /* LINUX */
