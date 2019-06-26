#ifndef _DRV_TAPI_LINUX_H
#define _DRV_TAPI_LINUX_H
/******************************************************************************

                              Copyright (c) 2014
                            Lantiq Deutschland GmbH

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/**
   \file drv_tapi_linux.h
   This file contains the declarations of the linux specific driver functions.
*/

/* ============================= */
/* Global Includes               */
/* ============================= */

#ifdef __KERNEL__
   #if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
      #include <linux/tqueue.h>
   #else
      #include <linux/workqueue.h>           /* work_struct */
   #endif /* LINUX_VERSION_CODE */
#endif /* __KERNEL__ */


/* ============================= */
/* Global variables              */
/* ============================= */

/* ============================= */
/* Macros & Definitions          */
/* ============================= */

/* ============================= */
/* Global Structures             */
/* ============================= */

/* ============================= */
/* Global function declaration   */
/* ============================= */
#ifdef __KERNEL__
   extern IFX_int32_t TAPI_OS_ThreadPriorityModify(IFX_uint32_t newPriority);
   extern IFX_void_t  TAPI_OS_ThreadKill(IFXOS_ThreadCtrl_t *pThrCntrl,
                                         TAPI_OS_lock_t *pLock);
#endif /* __KERNEL__ */
#ifndef TAPI_LIBRARY
#ifdef ENABLE_HOTPLUG
   extern IFX_void_t TAPI_OS_EventReport(IFX_void_t *pEvent);
#endif /* #ifdef ENABLE_HOTPLUG*/
#endif /* !TAPI_LIBRARY */

#endif /*_DRV_TAPI_LINUX_H */
