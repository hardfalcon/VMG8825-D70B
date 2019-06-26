#ifndef _DRV_TAPI_OSMAP_H
#define _DRV_TAPI_OSMAP_H
/******************************************************************************

                              Copyright (c) 2015
                        Lantiq Beteiligungs-GmbH & Co.KG
                             http://www.lantiq.com

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

*******************************************************************************/

/**
   \file drv_tapi_osmap.h
   This file contains the includes and the defines specific to the OS.
*/

#include <ifx_types.h>                       /* ifx type definitions */
#include "drv_tapi_config.h"

#if !defined(LINUX) && \
    !defined(VXWORKS) && \
    !defined(WINDOWS) && \
    !defined(WIN32) && \
    !(defined(GENERIC_OS) && defined(GREENHILLS_CHECK))
   #error TAPI driver - no OS specified. Please define your operating system!
#endif /* OS */

#ifdef GREENHILLS_CHECK
   #include "drv_tapi_ghs.h"
#endif /* GREENHILLS_CHECK */

#ifndef TAPI_LIBRARY
   /* Linux kernel space */
   #if defined (LINUX) && defined (__KERNEL__)
      #include <linux/irq.h>
      #include <linux/threads.h>
      #include <linux/interrupt.h>           /* in_interrupt() */
      #include <asm/poll.h>                  /* POLLIN, POLLOUT */
      #ifdef TAPI_FEAT_LINUX_SMP
         #include <linux/spinlock.h>
      #endif /* TAPI_FEAT_LINUX_SMP */
   #endif /* LINUX */

   /* VxWorks */
   #ifdef VXWORKS
      #include <intLib.h>
   #endif /* VXWORKS */

   /* Windows */
   #ifdef WINDOWS
      #include <sys_tickedtimer.h>
   #endif /* WINDOWS */
#else /* TAPI_LIBRARY */
   #include <ifxos_std_defs.h>
#endif /* TAPI_LIBRARY */

#include <ifxos_memory_alloc.h>
#include <ifxos_copy_user_space.h>
#include <ifxos_debug.h> /* debug features */
#include <ifxos_event.h>
#include <ifxos_select.h>
#include <ifxos_mutex.h>
#include <ifxos_lock.h>
#include <ifxos_time.h>
#include <ifx_fifo.h>  /* fifo (used for streaming) */
#include <ifxos_thread.h>

/* ============================= */
/* Global Defines                */
/* ============================= */
#define DBG_LEVEL_OFF      4
#define DBG_LEVEL_HIGH     3
#define DBG_LEVEL_NORMAL   2
#define DBG_LEVEL_LOW      1

/** Define the used CR/LF sequence */
#define TAPI_CRLF      IFXOS_CRLF

/*lint -save -esym(773,SetTraceLevel) -esym(773,CREATE_TRACE_GROUP) */
#define CREATE_TRACE_GROUP(name) \
   unsigned int G_nTraceGroup##name = DBG_LEVEL_HIGH

#define DECLARE_TRACE_GROUP(name) extern unsigned int G_nTraceGroup##name
#define PRINTF IFXOS_PRINT_USR_RAW /* defined in lib_ifxos */
#define TRACE(name,level,message) do {if(level >= G_nTraceGroup##name) \
      { PRINTF message ; } } while(0)
#define SetTraceLevel(name, level) G_nTraceGroup##name = (level)
/*lint -restore */

#ifdef DEBUG
/** assert in debug code
\param expr - expression to be evaluated. If expr != TRUE assert is printed
              out with line number */
#define TAPI_ASSERT(expr) \
   do { \
      /*lint -save -e{506, 774} */ \
      if(!(expr)) { \
         (void) IFXOS_DBG_PRINT_USR ("\n\r" __FILE__ ":%d: Assertion %s " \
                                     "failed!\n\r",__LINE__, #expr); \
      } \
      /*lint -restore */ \
   } while (0)
#else /* DEBUG */
/** assert in debug code
\param expr - expression to be evaluated. If expr != TRUE assert is printed
              out with line number */
#define TAPI_ASSERT(expr)
#endif /* DEBUG */

#ifdef VXWORKS
   #ifndef GetImmr
      #define GetImmr()                   vxImmrGet()
   #endif
   #define IMAP_ADDR                      vxImmrGet()
#endif /* VXWORKS */


/* ============================= */
/* Name mapping tables           */
/* ============================= */

/*
   Mapping table - Dynamic memory handling.
*/
#if defined(LINUX) && defined(__KERNEL__)
#define TAPI_OS_Malloc                    IFXOS_BlockAlloc
#define TAPI_OS_Free                      IFXOS_BlockFree
#else
#define TAPI_OS_Malloc                    IFXOS_MemAlloc
#define TAPI_OS_Free                      IFXOS_MemFree
#endif

/*
   Mapping table - Kernel-space / User-space data exchange.
*/
#if defined(LINUX) && !defined(__KERNEL__)
#define TAPI_OS_CpyKern2Usr(pTo,pFrom,size) \
      (memcpy(pTo,pFrom,size)!=IFX_NULL)
#define TAPI_OS_CpyUsr2Kern(pTo,pFrom,size) \
      (memcpy(pTo,pFrom,size)!=IFX_NULL)
#else
#define TAPI_OS_CpyKern2Usr               IFXOS_CpyToUser
#define TAPI_OS_CpyUsr2Kern               IFXOS_CpyFromUser
#endif /* defined(LINUX) && !defined(__KERNEL__) */

/*
   Mapping table - Mutex handling.
*/
#define TAPI_OS_mutex_t                   IFXOS_mutex_t
#define TAPI_OS_MutexInit                 IFXOS_MutexInit
#define TAPI_OS_MutexDelete               IFXOS_MutexDelete
#define TAPI_OS_MutexGet                  IFXOS_MutexGet
#define TAPI_OS_MutexRelease              IFXOS_MutexRelease

/*
   Mapping table - Lock handling.
*/
#define TAPI_OS_lock_t                    IFXOS_lock_t
#define TAPI_OS_LockInit                  IFXOS_LockInit
#define TAPI_OS_LockGet                   IFXOS_LockGet
#define TAPI_OS_LockRelease               IFXOS_LockRelease
#define TAPI_OS_LockDelete                IFXOS_LockDelete
/* Lock (take the mutex) but be interruptable by signals.
   Difference to TAPI_OS_LockGet is that the sleep can be interrupted
   when a signal is received by the sleeping thread. */
#if defined (LINUX) && defined (__KERNEL__)
   #define TAPI_OS_LOCK_GET_INTERRUPTIBLE(lockId) \
      /*lint -save -e{155, 506, 774} */ \
      (down_interruptible(&(lockId)->object)) \
      /*lint -restore */
#endif /* LINUX */
#ifdef VXWORKS
   #define TAPI_OS_LOCK_GET_INTERRUPTIBLE TAPI_OS_LockGet
#endif /* VXWORKS */
#ifdef WINDOWS
   #define TAPI_OS_LOCK_GET_INTERRUPTIBLE TAPI_OS_LockGet
#endif /* WINDOWS */

#ifndef TAPI_LIBRARY
   #ifdef TAPI_FEAT_LINUX_SMP
      #define TAPI_OS_PROTECT_IRQLOCK(spinlock, lockId) \
         spin_lock_irqsave((spinlock), (lockId))
      #define TAPI_OS_UNPROTECT_IRQLOCK(spinlock, lockId) \
         spin_unlock_irqrestore((spinlock), (lockId))
   #else /* TAPI_FEAT_LINUX_SMP */
      #define TAPI_OS_PROTECT_IRQLOCK(semaphore, lockId) \
         if (!TAPI_OS_IN_INTERRUPT()) \
         { \
            TAPI_OS_MutexGet (semaphore); \
         } \
         TAPI_OS_LOCKINT (lockId)
      #define TAPI_OS_UNPROTECT_IRQLOCK(semaphore, lockId) \
         TAPI_OS_UNLOCKINT (lockId); \
         if (!TAPI_OS_IN_INTERRUPT()) \
         { \
            TAPI_OS_MutexRelease (semaphore); \
         }
   #endif /* TAPI_FEAT_LINUX_SMP */
#else /* TAPI_LIBRARY */
   #define TAPI_OS_PROTECT_IRQLOCK(semaphore, lock) \
         ((IFX_void_t)(lock)); /* variable unused */ \
         TAPI_OS_MutexGet (semaphore)
   #define TAPI_OS_UNPROTECT_IRQLOCK(semaphore, lock) \
         ((IFX_void_t)(lock)); /* variable unused */ \
         TAPI_OS_MutexRelease (semaphore)
#endif /* TAPI_LIBRARY */

/*
   Mapping table - Interrupt handling.
*/
#ifndef TAPI_LIBRARY
   #if defined (LINUX) && defined (__KERNEL__)
      /** Determine if the current state is in interrupt or task context. */
      #define TAPI_OS_IN_INTERRUPT() \
         (/*lint -save -e{737} */in_interrupt()/*lint -restore*/ ? \
          IFX_TRUE : IFX_FALSE)
      #define TAPI_OS_INTSTAT \
         unsigned long
      #define TAPI_OS_LOCKINT(var) \
         local_irq_save(var)
      #define TAPI_OS_UNLOCKINT(var) \
         local_irq_restore(var)
   #endif /* LINUX */
   #ifdef VXWORKS
      /** Determine if the current state is in interrupt or task context. */
      #define TAPI_OS_IN_INTERRUPT() \
         ((intContext() == TRUE) ? IFX_TRUE : IFX_FALSE)
      #define TAPI_OS_INTSTAT \
         int
      #define TAPI_OS_LOCKINT(var) \
         var = intLock()
      #define TAPI_OS_UNLOCKINT(var) \
         intUnlock(var)
   #endif
   #ifdef WINDOWS
      /** Determine if the current state is in interrupt or task context. */
      #define TAPI_OS_IN_INTERRUPT() IFX_FALSE
      /*lint -restore */
   #endif
#else /* TAPI_LIBRARY */
   /** Determine if the current state is in interrupt or task context. */
   #define TAPI_OS_IN_INTERRUPT() IFX_FALSE
   #define TAPI_OS_INTSTAT \
      unsigned
   #define TAPI_OS_LOCKINT(var) \
      ((IFX_void_t)(var)) /* variable unused */
   #define TAPI_OS_UNLOCKINT(var) \
      ((IFX_void_t)(var)) /* variable unused */
#endif /* TAPI_LIBRARY */

/*
   Mapping table - Event signalling.
*/
#define TAPI_OS_event_t                   IFXOS_event_t
#define TAPI_OS_EventInit                 IFXOS_EventInit
#define TAPI_OS_EventWakeUp               IFXOS_EventWakeUp
#define TAPI_OS_EventWait                 IFXOS_EventWait
#define TAPI_OS_EventDelete               IFXOS_EventDelete
#define TAPI_OS_WAIT_FOREVER              IFXOS_WAIT_FOREVER

/*
   Mapping table - Select handling.
*/
#define TAPI_OS_drvSelectQueue_t          IFXOS_drvSelectQueue_t
#define TAPI_OS_drvSelectTable_t          IFXOS_drvSelectTable_t
#define TAPI_OS_drvSelectOSArg_t          IFXOS_drvSelectOSArg_t
#ifdef TAPI_LIBRARY
#define TAPI_OS_DrvSelectQueueInit(pEventId) while(0)
#define TAPI_OS_DrvSelectQueueWakeUp(pDrvSelectQueue,drvSelType) while(0)
#else
#define TAPI_OS_DrvSelectQueueInit        IFXOS_DrvSelectQueueInit
#define TAPI_OS_DrvSelectQueueWakeUp      IFXOS_DrvSelectQueueWakeUp
#endif /* TAPI_LIBRARY */
#define TAPI_OS_DRV_SEL_WAKEUP_TYPE_RD    IFXOS_DRV_SEL_WAKEUP_TYPE_RD
#define TAPI_OS_DRV_SEL_WAKEUP_TYPE_WR    IFXOS_DRV_SEL_WAKEUP_TYPE_WR
#define TAPI_OS_DrvSelectQueueAddTask     IFXOS_DrvSelectQueueAddTask
#define TAPI_OS_DrvSelectQueueDelete(param) /* does not exist */

/** Definitions for select queues. */
/* TAPI_OS_SYSWRITE is returned by select() to indicate that the fd is ready
   for writing.
   TAPI_OS_SYSREAD is returned by select() to indicate that the fd is ready
   for reading. */
#if defined (LINUX) && defined (__KERNEL__)
   #define  TAPI_OS_SYSWRITE  POLLOUT
   #define  TAPI_OS_SYSREAD   POLLIN
#endif /* LINUX */
#ifdef VXWORKS
   #define TAPI_OS_SYSWRITE   0x00000002
   #define TAPI_OS_SYSREAD    0x00000001
#endif /* VXWORKS */
#ifdef WINDOWS
   #define  TAPI_OS_SYSWRITE  0x00000002
   #define  TAPI_OS_SYSREAD   0x00000001
#endif /* WINDOWS */

/*
   Mapping table - Thread handling.
*/
#define TAPI_OS_THREAD_PRIO_HIGH          IFXOS_THREAD_PRIO_HIGH
#define TAPI_OS_THREAD_PRIO_HIGHEST       IFXOS_THREAD_PRIO_HIGHEST
#define TAPI_OS_ThreadCtrl_t              IFXOS_ThreadCtrl_t
#define TAPI_OS_ThreadParams_t            IFXOS_ThreadParams_t
#define TAPI_OS_ThreadInit                IFXOS_ThreadInit
#if defined (LINUX)
   #ifndef IFXOS_THREAD_PRIO_HIGH
   #define IFXOS_THREAD_PRIO_HIGH         60
   #endif
   #ifndef IFXOS_THREAD_PRIO_HIGHEST
   #define IFXOS_THREAD_PRIO_HIGHEST      80
   #endif
#endif /* LINUX */
#if defined (LINUX) && defined (__KERNEL__)
   #define TAPI_OS_THREAD_KILL            TAPI_OS_ThreadKill
   #define TAPI_OS_THREAD_PRIORITY_MODIFY TAPI_OS_ThreadPriorityModify
#endif /* LINUX && __KERNEL */
#ifdef VXWORKS
   #define TAPI_OS_THREAD_KILL(pThrCntrl, pLock) /* empty */
   #define TAPI_OS_THREAD_PRIORITY_MODIFY(prio) \
      taskPrioritySet(taskIdSelf(), prio)
#endif /* VXWORKS */
#ifdef WINDOWS
   #define TAPI_OS_THREAD_KILL(pThrCntrl, pLock) /* empty */
   #define TAPI_OS_THREAD_PRIORITY_MODIFY IFXOS_ThreadPriorityModify
#endif /* WINDOWS */

/*
   Mapping table - Time and Waiting.
*/
#define TAPI_OS_MSecSleep                 IFXOS_MSecSleep

/*
   Function map - IOCTL Handling
*/
/** Returns the magic number of IOCTL command
\param iocmd - ioctl command of which magic number is decoded */
#define TAPI_IOC_MAGIC(cmd)   (_IOC_TYPE(cmd))

/** Returns the write request of IOCTL command
\param iocmd - ioctl command */
#define TAPI_IOC_WRITE(cmd)   ((_IOC_DIR(cmd) & _IOC_WRITE) ? IFX_TRUE : IFX_FALSE)

/** Returns the read request of IOCTL command
\param iocmd - ioctl command */
#define TAPI_IOC_READ(cmd)    ((_IOC_DIR(cmd) & _IOC_READ) ? IFX_TRUE : IFX_FALSE)

/** Returns the IOCTL command number
\param iocmd - ioctl command */
#define TAPI_IOC_IDX(cmd)     (_IOC_NR(cmd))

/** Returns the argument size of IOCTL command
\param iocmd - ioctl command */
#define TAPI_IOC_SIZE(cmd)    (_IOC_SIZE(cmd))

#ifndef WIN32
/* To make EventLogger happy */
#define IFXOS_GET_TICK()                  IFXOS_ElapsedTimeMSecGet(0)
#else
/* IFXOS_ElapsedTimeMSecGet can't be used, because with correct time it renders
 * overflow and returns a negative value. It calls time() and then multiplies it
 * by 1000.
 * GetCurrentTime returns uptime in ms.
 */
#define IFXOS_GET_TICK()                  GetCurrentTime()
#endif

/*
   Function map - Time and Wait Functions and Defines.
*/
#define TAPI_OS_USecSleep                 IFXOS_USecSleep


#if defined(TAPI_FEAT_LINUX_SMP)
#  include <linux/spinlock.h>
#  include <linux/spinlock_types.h>

#  define IFXOS_spinlock_t spinlock_t
#  define TAPI_OS_SPIN_LOCK_INIT(P_TAPI_SPIN_LOCK)\
      do {spin_lock_init(&(P_TAPI_SPIN_LOCK)->sl_handle);} while(0)

#  define TAPI_OS_SPIN_LOCK(P_TAPI_SPIN_LOCK) \
      do {spin_lock(&(P_TAPI_SPIN_LOCK)->sl_handle);} while(0)
#  define TAPI_OS_SPIN_UNLOCK(P_TAPI_SPIN_LOCK) \
      do {spin_unlock(&(P_TAPI_SPIN_LOCK)->sl_handle);} while(0)

#  define TAPI_OS_SPIN_LOCK_IRQ(P_TAPI_SPIN_LOCK) \
      do {spin_lock_irq(&(P_TAPI_SPIN_LOCK)->sl_handle);} while(0)
#  define TAPI_OS_SPIN_UNLOCK_IRQ(P_TAPI_SPIN_LOCK) \
      do {spin_unlock_irq(&(P_TAPI_SPIN_LOCK)->sl_handle);} while(0)

#  define TAPI_OS_SPIN_LOCK_IRQSAVE(P_TAPI_SPIN_LOCK) \
      do {spin_lock_irqsave(&(P_TAPI_SPIN_LOCK)->sl_handle, (P_TAPI_SPIN_LOCK)->irq_flags);} while(0)
#  define TAPI_OS_SPIN_UNLOCK_IRQRESTORE(P_TAPI_SPIN_LOCK) \
      do {spin_unlock_irqrestore(&(P_TAPI_SPIN_LOCK)->sl_handle, (P_TAPI_SPIN_LOCK)->irq_flags);} while(0)
#else
#  define IFXOS_spinlock_t unsigned int
#  define TAPI_OS_SPIN_LOCK_INIT(P_TAPI_SPIN_LOCK)
#  define TAPI_OS_SPIN_LOCK(P_TAPI_SPIN_LOCK)
#  define TAPI_OS_SPIN_UNLOCK(P_TAPI_SPIN_LOCK)
#  define TAPI_OS_SPIN_LOCK_IRQSAVE(P_TAPI_SPIN_LOCK)
#  define TAPI_OS_SPIN_UNLOCK_IRQRESTORE(P_TAPI_SPIN_LOCK)
#  define TAPI_OS_SPIN_LOCK_IRQ(P_TAPI_SPIN_LOCK)
#  define TAPI_OS_SPIN_UNLOCK_IRQ(P_TAPI_SPIN_LOCK)
#endif   /* #if defined(TAPI_FEAT_LINUX_SMP) */


typedef struct
{
   IFXOS_spinlock_t sl_handle;
   TAPI_OS_INTSTAT  irq_flags;
}  TAPI_OS_spin_lock_s;



#ifdef LINUX
#include "drv_tapi_linux.h"
#endif
#ifdef VXWORKS
#include "drv_tapi_vxworks.h"
#endif
#ifdef WINDOWS
#include "drv_tapi_win.h"
#endif
#endif /* _DRV_TAPI_OSMAP_H */
