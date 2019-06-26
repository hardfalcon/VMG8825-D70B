/******************************************************************************

                              Copyright (c) 2014
                            Lantiq Deutschland GmbH
                             http://www.lantiq.com

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

*******************************************************************************/

/**
   \file drv_mps_vmmc_bsp.h
   This file contains the adaption to interfaces of other drivers from the
   board support package (BSP).
*/

/* ============================= */
/* Includes                      */
/* ============================= */
#include <linux/version.h>

/* ============================= */
/*  Macros & Definitions         */
/* ============================= */

/* Since the BSP API does not have a version that can be checked this driver
   defines an version number for itself. The version is derived indirectly
   by looking at the kernel version. Although the kernel version has nothing
   directly to do with the BSP API major changes of the board drivers were
   only done with kernel updates. */
#ifndef BSP_API_VERSION
   #if   (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
      #define BSP_API_VERSION 1        /* LXDB */
   #elif (LINUX_VERSION_CODE < KERNEL_VERSION(3,0,0))
      #define BSP_API_VERSION 2        /* UGW xxx - UGW 5.4; Linux 2.6.32,
                                                             Linux 2.6.42 */
   #elif (LINUX_VERSION_CODE < KERNEL_VERSION(3,8,0))
      #define BSP_API_VERSION 3        /* development version; Linux 3.5 */
   #elif (LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0))
      #define BSP_API_VERSION 4        /* development version; Linux 3.8 */
   #else
      #define BSP_API_VERSION 5        /* UGW 6.1; Linux 3.10.12 and later */
   #endif /* LINUX_VERSION_CODE */
#endif /* BSP_API_VERSION */

/* Workaround for a bug in some BSP files. There a check for AUTOCONF_INCLUDED
   is done which is missing. */
#if ((BSP_API_VERSION == 3) || (BSP_API_VERSION == 4))
   #include <generated/autoconf.h>
   #define AUTOCONF_INCLUDED
#endif /* LINUX_VERSION_CODE */

