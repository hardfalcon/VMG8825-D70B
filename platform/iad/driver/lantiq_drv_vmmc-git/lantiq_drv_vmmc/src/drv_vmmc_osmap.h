#ifndef _DRV_VMMC_OSMAP_H
#define _DRV_VMMC_OSMAP_H
/******************************************************************************

                              Copyright (c) 2012
                            Lantiq Deutschland GmbH
                             http://www.lantiq.com

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

*******************************************************************************/

/**
   \file drv_vmmc_osmap.h
   This file contains the includes and the defines specific to the OS.
*/

#if defined(LINUX) || (defined(GENERIC_OS) && defined(GREENHILLS_CHECK))
   #include "drv_vmmc_linux.h"
#else
   #error "VMMC driver - no OS specified"
#endif

#include "ifx_types.h"     /* ifx type definitions */
#include "ifxos_debug.h"   /* debug features */
#include "ifx_fifo.h"      /* fifo (still required for CID RX) */
#include "ifxos_event.h"
#include "ifxos_interrupt.h"
#include "ifxos_mutex.h"
#include "ifxos_lock.h"
#include "ifxos_select.h"
#include "ifxos_memory_alloc.h"
#include "ifxos_copy_user_space.h"
#include "ifxos_thread.h"

#ifdef GREENHILLS_CHECK
   #include "drv_vmmc_ghs.h"
#endif /* GREENHILLS_CHECK */

/* ============================= */
/* Global Defines                */
/* ============================= */
#define DBG_LEVEL_OFF      4
#define DBG_LEVEL_HIGH     3
#define DBG_LEVEL_NORMAL   2
#define DBG_LEVEL_LOW      1

#define CREATE_TRACE_GROUP(name) unsigned int G_nTraceGroup##name = DBG_LEVEL_HIGH
#define DECLARE_TRACE_GROUP(name) extern unsigned int G_nTraceGroup##name
#define PRINTF IFXOS_PRINT_USR_RAW /* defined in lib_ifxos */
#define TRACE(name,level,message) do {if(level >= G_nTraceGroup##name) \
      { PRINTF message ; } } while(0)
#define SetTraceLevel(name, level) G_nTraceGroup##name = (level)


#ifdef DEBUG
/** assert in debug code
\param expr - expression to be evaluated. If expr != TRUE assert is printed
              out with line number */
#define VMMC_ASSERT(expr) BUG_ON(!(expr))
#else /* DEBUG */
/** assert in debug code
\param expr - expression to be evaluated. If expr != TRUE assert is printed
              out with line number */
#define VMMC_ASSERT(expr)
#endif /* DEBUG */

/*
   Mapping table - Dynamic memory handling.
*/
#define VMMC_OS_Malloc                    IFXOS_BlockAlloc
#define VMMC_OS_Free                      IFXOS_BlockFree

/*
   Mapping table - Kernel-space / User-space data exchange.
*/
#define VMMC_OS_CpyKern2Usr               IFXOS_CpyToUser
#define VMMC_OS_CpyUsr2Kern               IFXOS_CpyFromUser

/*
   Mapping table - Mutex handling.
*/
#define VMMC_OS_mutex_t                   IFXOS_mutex_t
#define VMMC_OS_MutexInit                 IFXOS_MutexInit
#define VMMC_OS_MutexDelete               IFXOS_MutexDelete
#define VMMC_OS_MutexGet                  IFXOS_MutexGet
#define VMMC_OS_MutexRelease              IFXOS_MutexRelease
/* not supported in lib_ifxos */
#ifdef LINUX
#define VMMC_OS_MutexLockInterruptible(lockId)   \
         (down_interruptible(&(lockId)->object))
#endif /* LINUX */

/*
   Mapping table - Interrupt handling.
*/
#define VMMC_OS_INTSTAT                   IFXOS_INTSTAT
#define VMMC_OS_IRQ_DISABLE               IFXOS_IRQ_DISABLE
#define VMMC_OS_IRQ_ENABLE                IFXOS_IRQ_ENABLE
#define VMMC_OS_LOCKINT                   IFXOS_LOCKINT
#define VMMC_OS_UNLOCKINT                 IFXOS_UNLOCKINT

/*
   Mapping table - Event signalling.
*/
#define VMMC_OS_event_t                   IFXOS_event_t
#define VMMC_OS_EventInit                 IFXOS_EventInit
#define VMMC_OS_EventWakeUp               IFXOS_EventWakeUp
#define TAPI_OS_event_t                   IFXOS_event_t

/*
   Mapping table - Thread handling.
*/
#define VMMC_OS_THREAD_PRIO_HIGH          IFXOS_THREAD_PRIO_HIGH
#define VMMC_OS_THREAD_PRIO_HIGHEST       IFXOS_THREAD_PRIO_HIGHEST
#define VMMC_OS_ThreadCtrl_t              IFXOS_ThreadCtrl_t
#define VMMC_OS_ThreadParams_t            IFXOS_ThreadParams_t
#define VMMC_OS_ThreadInit                IFXOS_ThreadInit
#define VMMC_OS_THREAD_INIT_VALID         IFXOS_THREAD_INIT_VALID
#define VMMC_OS_ThreadFunction_t          IFXOS_ThreadFunction_t

#ifdef LINUX
   extern IFX_int32_t VMMC_OS_ThreadPriorityModify(IFX_uint32_t newPriority);
   extern IFX_void_t  VMMC_OS_ThreadKill(IFXOS_ThreadCtrl_t *pThrCntrl);
   #define VMMC_OS_THREAD_PRIORITY_MODIFY VMMC_OS_ThreadPriorityModify
   #define VMMC_OS_THREAD_KILL            VMMC_OS_ThreadKill
#endif /* LINUX */
#ifdef VXWORKS
   #define VMMC_OS_THREAD_KILL(pThrCntrl) /* empty */
   #define VMMC_OS_THREAD_PRIORITY_MODIFY(prio) \
      taskPrioritySet(taskIdSelf(), prio)
#endif /* VXWORKS */

#endif /* _DRV_VMMC_OSMAP_H */
