#ifndef _DRV_TAPI_CONFIG_H
#define _DRV_TAPI_CONFIG_H
/****************************************************************************
                              Copyright (c) 2014
                            Lantiq Deutschland GmbH

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

 ****************************************************************************/

/**
   \file drv_tapi_config.h

   This file configures the features depending on user controled defines and
   other defines.

   This is the main configuration file for the project.
*/

/* ========================================================================== */
/*                                 Includes                                   */
/* ========================================================================== */
#ifdef HAVE_CONFIG_H
   #include "drv_tapi_autoconf.h"
   /*lint -save -e(19)  we need this ; to stop runaway arguments from breaking
     out of drv_tapi_autoconf.h */
   ;
   /*lint -restore */
#endif
#ifdef VXWORKS
   #include "prjParams.h"
#endif

#if defined(LINUX) && defined(__KERNEL__)
   #include <linux/kernel.h>
#endif /* defined(LINUX) && defined(__KERNEL__) */

/* ========================================================================== */
/*                    Global Configuration Definitions                        */
/* ========================================================================== */

/* Support for the Linux procfs. */
#ifdef TAPI_USE_PROC
   #if defined(LINUX) && defined(__KERNEL__) && defined(CONFIG_PROC_FS)
      #define TAPI_FEAT_PROCFS
   #else
      #warning Not compiling for Linux kernel-space: procfs support disabled.
   #endif /* defined(LINUX) && defined(__KERNEL__) && defined(CONFIG_PROC_FS) */
#endif /* TAPI_USE_PROC */

#ifdef LINUX_SMP_SUPPORT
   #if defined(LINUX) && defined(__KERNEL__)
      #define TAPI_FEAT_LINUX_SMP
   #else
      #warning Not compiling for Linux kernel-space: SMP support disabled.
   #endif /* defined(LINUX) && defined(__KERNEL__) */
#endif /* LINUX_SMP_SUPPORT */

/* Support for API to execute IOCTLs from Linux kernel-space. */
#ifdef TAPI_KERNEL_API_SUPPORT
   #if defined(LINUX) && defined(__KERNEL__)
      #define TAPI_FEAT_KIOCTL
   #else
      #warning Not compiling for Linux kernel-space: Kernel API disabled.
   #endif /* defined(LINUX) && defined(__KERNEL__) */
#endif /* TAPI_KERNEL_API_SUPPORT */

/* Support for Voice Connection feature. */
#ifdef TAPI_VOICE
   #define TAPI_FEAT_VOICE
   #define TAPI_FEAT_RTP_OOB
   #define TAPI_FEAT_AMR
   #undef TAPI_FEAT_BABYPHONE /* not supported by any current device */
#endif /* TAPI_VOICE */

/* Support for Statistic feature to count packets. */
#ifdef TAPI_STATISTICS
   #define TAPI_FEAT_STATISTICS
#endif /* TAPI_STATISTICS */

/* Support for the MOS-LQE calculation feature. */
#ifdef TAPI_MOS_LQE
   #define TAPI_FEAT_MOS_LQE
   #define TAPI_FEAT_VOICE
#endif /* TAPI_CALIBRATION */

/* Support for FAX feature. There are two variants supported:
   - full T.38 support inside the voice-FW
   - fax data pump support inside the voice-FW (deprecated)
   Only one variant can be used at a time. Full T.38 support takes preceedence
   if both variants are configured. */
/* Keep this block above the translation of the FAX defines below. */
#if defined(TAPI_FAX_T38_FW) && defined(TAPI_FAX_T38)
   #undef TAPI_FAX_T38
   #warning FAX data pump support disabled because full T.38 support is enabled.
#endif

/* Support for T.38 FAX feature using the legacy datapump. */
#ifdef TAPI_FAX_T38
   #define TAPI_FEAT_FAX_T38
#endif /* TAPI_FAX_T38 */

/* Support for T.38 FAX feature using the stack inside the FW. */
#ifdef TAPI_FAX_T38_FW
   #define TAPI_FEAT_FAX_STACK
#endif /* TAPI_FAX_T38_FW */

/* Support for the Announcement Playout feature. */
#ifdef TAPI_ANNOUNCEMENTS
   #define TAPI_FEAT_ANNOUNCEMENTS
#endif /* TAPI_ANNOUNCEMENTS */

/* Support for the Peak Detector feature. */
#ifdef TAPI_PEAKD
   #define TAPI_FEAT_PEAKD
#endif /* TAPI_PEAKD */

/* Support for the MF R2 Detector feature. */
#ifdef TAPI_MF_R2
   #define TAPI_FEAT_MF_R2
#endif /* TAPI_MF_R2 */

/* Support for CID (Caller ID) features. */
#ifdef TAPI_CID
   #define TAPI_FEAT_CID
   #define TAPI_FEAT_TONEENGINE
   #define TAPI_FEAT_TONETABLE
   #define TAPI_FEAT_RINGENGINE
#endif /* TAPI_CID */

/* Support for the Analog Line Continuous Measurement feature. */
#ifdef TAPI_CONT_MEASUREMENT
   #define TAPI_FEAT_CONT_MEAS
#endif /* TAPI_CONT_MEASUREMENT */

/* Support for the Analog Line Capacitance Measurement feature. */
#ifdef TAPI_CAPACITANCE_MEASUREMENT_SUPPORT
   #define TAPI_FEAT_CAP_MEAS
#endif /* TAPI_CAPACITANCE_MEASUREMENT_SUPPORT */

/* Support for the DTMF features. */
#ifdef TAPI_DTMF
   #define TAPI_FEAT_DTMF
#endif /* TAPI_DTMF */

/* Support for Pulse dial detection. */
#ifdef TAPI_DIAL
   #define TAPI_FEAT_DIAL
#endif /* TAPI_DIAL */

/* Support for Pulse dial detection with the Hookstate Decoding Statemachine. */
#ifdef TAPI_HOOKSTATE
   #define TAPI_FEAT_DIAL
   #define TAPI_FEAT_DIALENGINE
#endif /* TAPI_HOOKSTATE */

/* Support for the TTX (TeleTax/Metering) feature. */
#ifdef TAPI_METERING
   #define TAPI_FEAT_METERING
#endif /* TAPI_METERING */

/* Support for Voice Packet (AAL/RTP) transport inside TAPI. */
#ifdef TAPI_PACKET
   #define TAPI_FEAT_PACKET
#endif /* TAPI_PACKET */

/* Support for the KPI (Kernel Packet Interface) feature. */
#ifdef KPI_SUPPORT
   #define TAPI_FEAT_KPI
   #define TAPI_FEAT_PACKET
   /* Tasklet is a Linux only feature */
   #ifdef LINUX
      #ifdef KPI_TASKLET
         #define TAPI_FEAT_KPI_TASKLET
      #else
         #warning KPI TASKLET mode disabled!!!
      #endif /* KPI_TASKLET */
   #endif /* LINUX */
#endif /* KPI_SUPPORT */

/* Support for the QOS (packet stream Quality Of Service) feature. */
#ifdef QOS_SUPPORT
   #define TAPI_FEAT_QOS
   #define TAPI_FEAT_KPI
   #define TAPI_FEAT_PACKET
#endif /* QOS_SUPPORT */

/* Support for the PCM interface. */
#ifdef PCM_SUPPORT
   #define TAPI_FEAT_PCM
#endif

/* Support for the HDLC feature. */
#ifdef TAPI_HDLC
   #define TAPI_FEAT_HDLC
   #define TAPI_FEAT_PCM
#endif /* TAPI_HDLC */

/* Support for the polling mode interface. */
#ifdef TAPI_POLL
   #define TAPI_FEAT_POLL
#endif /* QOS_SUPPORT */

/* Support for the DECT feature. */
#ifdef DECT_SUPPORT
   #define TAPI_FEAT_DECT
   #define TAPI_FEAT_PACKET
   #define TAPI_FEAT_TONEENGINE
   #define TAPI_FEAT_TONETABLE
#endif /* DECT_SUPPORT */

/* Support for the FXO feature. */
#ifdef TAPI_FXO_SUPPORT
   #define TAPI_FEAT_FXO
#endif /* TAPI_FXO */

/* Support for the Phone Detection feature. */
#ifdef TAPI_PHONE_DETECTION
   #define TAPI_FEAT_PHONE_DETECTION
   #ifdef TAPI_FEAT_PROCFS
      #define TAPI_FEAT_PHONE_DETECTION_PROCFS
   #endif /* TAPI_FEAT_PROCFS */
#endif /* TAPI_PHONE_DETECTION */

/* Support for the Ring engine feature. */
#ifdef TAPI_RING_ENGINE
   #define TAPI_FEAT_RINGENGINE
#endif /* TAPI_RING_ENGINE */

/* Feature: Power Management Control */
#if defined(TAPI_PMC) && defined(TAPI_PHONE_DETECTION)
   /* Both interfaces cannot coexist. Abort if both are selected. */
   #if defined(TAPI_PMC_IF_PMCU) && defined(TAPI_PMC_IF_CPUFREQ)
      #error TAPI configure error. More than one interface to the PMCU driver
             cannot be used at the same time. Select only one or none at all.
   #endif
   /* Interface to PMCU driver */
   #ifdef TAPI_PMC_IF_PMCU
      #define TAPI_FEAT_PMCU_IF
   #endif
   /* Interface to kernel CPU frequency */
   #ifdef TAPI_PMC_IF_CPUFREQ
      #define TAPI_FEAT_CPUFREQ_IF
   #endif
   #if defined(TAPI_PMC_IF_PMCU) || defined(TAPI_PMC_IF_CPUFREQ)
      #define TAPI_FEAT_POWER
   #endif
#endif /* defined(TAPI_PMC) && defined(TAPI_PHONE_DETECTION) */

/* Support for SRTP/SRTCP features. */
#ifdef TAPI_SRTP
   #define TAPI_FEAT_SRTP
#endif /* TAPI_SRTP */

/* Support for tone generator features. */
#ifdef TAPI_TONE_GENERATOR_SUPPORT
   #define TAPI_FEAT_TONEGEN
   #define TAPI_FEAT_TONEENGINE
   #define TAPI_FEAT_TONETABLE
#endif /* TAPI_TONEGEN */

/* Support for the MFTD (Modem Fax Tone Detector) feature. */
#ifdef TAPI_MFTD_SUPPORT
   #define TAPI_FEAT_MFTD
#endif /* TAPI_MFTD_SUPPORT */

/* Support for the CPTD (Call Progress Tone Detector) feature. */
#ifdef TAPI_CPTD
   #define TAPI_FEAT_CPTD
   #define TAPI_FEAT_TONETABLE
#endif /* TAPI_CPTD */

/* Support for the MWL (Message Waiting Lamp) feature. */
#ifdef TAPI_MWL
   #define TAPI_FEAT_MWL
#endif /* TAPI_MWL */

/* Support for NLT (Network Line Testing) features. */
#ifdef TAPI_NLT
   #define TAPI_FEAT_NLT
#endif /* TAPI_NLT */

/* Support for the GR-909 test feature. */
#ifdef TAPI_GR909
   #define TAPI_FEAT_GR909
#endif /* TAPI_GR909 */

/* Support for the Analog Line Calibration feature. */
#ifdef TAPI_CALIBRATION
   #define TAPI_FEAT_CALIBRATION
#endif /* TAPI_CALIBRATION */

/* Support for the Line Echo Cancellation (LEC) features. */
#ifdef TAPI_LEC_SUPPORT
   #define TAPI_FEAT_ALM_LEC
   #define TAPI_FEAT_PCM_LEC
#endif /* TAPI_LEC_SUPPORT */

/* Support for SmartSLIC specific features. */
#ifdef TAPI_SSLIC_SUPPORT
   #define TAPI_FEAT_SSLIC_RECOVERY
#endif /* TAPI_SSLIC_SUPPORT */

#ifndef TAPI_HAVE_TIMERS
   /* All these features make no sense without timers.
      Deactivate them completely when no timers are available. */
   #undef TAPI_FEAT_CID
   #undef TAPI_FEAT_TONEGEN
   #undef TAPI_FEAT_TONEENGINE
   #undef TAPI_FEAT_DIALENGINE
   #undef TAPI_FEAT_RINGENGINE
   #undef TAPI_FEAT_METERING
   #undef TAPI_FEAT_SSLIC_RECOVERY
   #warning TAPI without timers: deactivated TAPI features depending on timers.
#endif /* TAPI_HAVE_TIMERS */

#endif /* _DRV_TAPI_CONFIG_H */
