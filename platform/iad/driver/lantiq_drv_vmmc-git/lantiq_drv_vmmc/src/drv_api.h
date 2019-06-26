#ifndef _DRV_API_H
#define _DRV_API_H
/****************************************************************************
                            Copyright (c) 2006-2015
                        Lantiq Beteiligungs-GmbH & Co.KG
                             http://www.lantiq.com

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

*******************************************************************************/

/**
   \file drv_api.h
      This file contains the defines, the structures declarations,
      the tables declarations and the global functions declarations.
      It is the main configuration file for the project and can be adapted
      to own needs.

   \remarks
      Compiler switch for OS is needed. Use LINUX for linux and VXWORKS for
      VxWorks.
*/

/**
 \mainpage VMMC Driver Description
 \section DRV_INTEGRATION Integrating the Driver
 \subsection COMP_SWITCHES Compiler Switches
 - LINUX        - must be specified for LINUX
 - VXWORKS      - must be specified for VxWorks
 - DEBUG        - enables debug features as asserts for example
 - ENABLE_TRACE - enables print out to terminal
*/

/* ============================= */
/* Global Defines                */
/* ============================= */

/* define prototypes for VxWorks 5 */
#ifndef __PROTOTYPE_5_0
   #define __PROTOTYPE_5_0
#endif

#ifdef HAVE_CONFIG_H
#include "drv_config.h"
/*lint -save -e(19)  we need this ; to stop a runaway arguments from breaking
  out of drv_config.h */
;
/*lint -restore */
#else
#error no drv_config.h
#endif

/** Number of devices that can be handled by this driver. */
#ifndef VMMC_MAX_DEVICES
#define VMMC_MAX_DEVICES               1
#endif /* VMMC_MAX_DEVICES */

/** Number of channels per device. */
#ifndef VMMC_MAX_CH_NR
#define VMMC_MAX_CH_NR                 16
#endif /* VMMC_MAX_CH_NR */

/* ============================= */
/* includes                      */
/* ============================= */

/* this included file is supposed to support following OS:
   - LINUX
   - VXWORKS
*/
#include "drv_vmmc_osmap.h"

/* ============================= */
/* define mapping                */
/* ============================= */

/* define features depending on user controled defines and other defines */

#if   defined(SYSTEM_DANUBE)
   #error Sorry, the Danube platform is no longer supported by this driver.
#elif defined(SYSTEM_FALCON)
   /* nothing */
#else /* default XRX systems */
   #define VMMC_FEAT_VPE1_SW_WD
#endif /* SYSTEM_... */

/* defined unconditionally */
#define VMMC_FEAT_PACKET

#ifdef LINUX_PLATFORM_DRIVER
   #if defined(LINUX)
      #define VMMC_FEAT_LINUX_PLATFORM_DRIVER
   #endif
#endif

#ifdef TAPI_FAX_T38
   #define VMMC_FEAT_FAX_T38
   #define VMMC_FEAT_PACKET
#endif

#ifdef TAPI_FAX_T38_FW
   #define VMMC_FEAT_FAX_T38_FW
   #define VMMC_FEAT_PACKET
#endif

#ifdef HDLC_SUPPORT
   #define VMMC_FEAT_HDLC
   #define VMMC_FEAT_KPI
   #define VMMC_FEAT_PACKET
#endif

#ifdef KPI_SUPPORT
   #define VMMC_FEAT_KPI
   #define VMMC_FEAT_PACKET
#endif

/* RTP Control Protocol Extended Reports (RTCP XR) */
#ifdef TAPI_RTCP_XR
   #define VMMC_FEAT_RTCP_XR
#endif

#ifdef PCM_SUPPORT
   #define VMMC_FEAT_PCM
#endif

#ifdef TAPI_ANNOUNCEMENTS
   #define VMMC_FEAT_ANNOUNCEMENTS
#endif

/* TAPI line testing combines all linetesting features */
#ifdef TAPI_LT
   #define VMMC_FEAT_NLT
   #define VMMC_FEAT_GR909
   #define VMMC_FEAT_CAP_MEASURE
#endif

/* Calibration feature */
#if defined(CALIBRATION_SUPPORT)
   #define VMMC_FEAT_CALIBRATION
#endif

/* Continuous measurement feature */
#if defined(TAPI_CONT_MEASUREMENT)
   #define VMMC_FEAT_CONT_MEASUREMENT
#endif

/* FXS Phone Detection feature */
#if defined(TAPI_PHONE_DETECTION)
   #define VMMC_FEAT_NLT
   #define VMMC_FEAT_CAP_MEASURE
#endif

/* Metering feature */
#if defined(TAPI_METERING)
   #define VMMC_FEAT_METERING
#endif

/* Message waiting lamp feature */
#define VMMC_FEAT_MWL

/* Power Management Control feature */
#ifdef TAPI_PMC
   #define VMMC_FEAT_CLOCK_SCALING
   /* Both interfaces cannot coexist. Abort if both are selected. */
   #if defined(TAPI_PMC_IF_PMCU) && defined(TAPI_PMC_IF_CPUFREQ)
      #error VMMC configure error. More than one interface to the PMCU driver
             cannot be used at the same time. Select only one or none at all.
   #endif
   /* Interface to PMCU driver */
   #ifdef TAPI_PMC_IF_PMCU
      #define VMMC_FEAT_PMCU_IF
   #endif
   #ifdef LINUX
      /* Interface to kernel CPU frequency */
      #ifdef TAPI_PMC_IF_CPUFREQ
         #define VMMC_FEAT_CPUFREQ_IF
      #endif
   #endif
   #if defined(TAPI_PMC_IF_PMCU) || defined(TAPI_PMC_IF_CPUFREQ)
      #define VMMC_FEATURE_PMC_CORE
   #endif
#endif

#ifdef TAPI_NO_SLIC
   #undef VMMC_FEAT_NLT
   #undef VMMC_FEAT_GR909
   #undef VMMC_FEAT_CAP_MEASURE
   #undef VMMC_FEAT_CALIBRATION
   #undef VMMC_FEAT_CONT_MEASUREMENT
   #undef VMMC_FEAT_METERING
   #undef VMMC_FEAT_MWL
#else
   #define VMMC_FEAT_SLIC
#endif

/* Automatically map data channel to data channel feature */
#ifdef TAPI_AUTOMATIC_DATA_MAPPING
  #define VMMC_FEAT_AUTOMATIC_DATA_MAPPING
#endif

#endif /* _DRV_API_H */
