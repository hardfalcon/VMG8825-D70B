/******************************************************************************

                              Copyright (c) 2013
                            Lantiq Deutschland GmbH
                             http://www.lantiq.com

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/**
   \file drv_vmmc_linux.h
   This file contains the declarations of the linux specific driver functions.
*/

/* ============================= */
/* Includes                      */
/* ============================= */
#ifdef LINUX
#include <linux/kernel.h>
#ifdef MODULE
   #include <linux/module.h>
#endif
#include <linux/version.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
   #include <linux/hardirq.h>
#else
   #include <asm/hardirq.h>
#endif /* Ver >= 2.6 */
#include <asm/byteorder.h>
#include <linux/interrupt.h>
#endif /* LINUX */

#include "ifx_types.h"     /* ifx type definitions */

/* ============================= */
/* Macros & Definitions          */
/* ============================= */
#define VMMC_OS_IN_INTERRUPT() \
   /*lint -save -e 155 -e 506 -e 774 */    \
   (in_interrupt() ? IFX_TRUE : IFX_FALSE) \
   /*lint -restore */

/** Returns system tick in milliseconds
Maybe used to measure roughly times for testing
\return system tick in milliseconds  */
#define IFXOS_GET_TICK()      \
   (IFX_ulong_t)(jiffies * 1000 / HZ)

/* Since the BSP API does not have a version that can be checked this driver
   defines an version number for itself. The version is derived indirectly
   by looking at the kernel version. Although the kernel version has nothing
   directly to do with the BSP API major changes of the board drivers were
   only done with kernel updates. */
#ifndef BSP_API_VERSION
   #if   (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
      #define BSP_API_VERSION 1
   #elif (LINUX_VERSION_CODE < KERNEL_VERSION(3,0,0))
      #define BSP_API_VERSION 2
   #elif (LINUX_VERSION_CODE < KERNEL_VERSION(3,8,0))
      #define BSP_API_VERSION 3
   #elif (LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0))
      #define BSP_API_VERSION 4
   #else
      #define BSP_API_VERSION 5
   #endif /* LINUX_VERSION_CODE */
#endif /* BSP_API_VERSION */

/* ============================= */
/* Global variable declaration   */
/* ============================= */

/* ============================= */
/* Global function declaration   */
/* ============================= */
IFX_void_t *VMMC_OS_MapBuffer          (IFX_void_t *p_buffer,
                                        IFX_uint32_t size);
IFX_void_t  VMMC_OS_UnmapBuffer        (IFX_void_t *p_buffer);
