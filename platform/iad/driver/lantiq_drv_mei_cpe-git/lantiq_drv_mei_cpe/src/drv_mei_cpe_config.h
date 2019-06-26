#ifndef _DRV_MEI_CPE_CONFIG_H
#define _DRV_MEI_CPE_CONFIG_H
/******************************************************************************

                          Copyright (c) 2007-2015
                     Lantiq Beteiligungs-GmbH & Co. KG

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#if defined(MEI_INTERN)
#  if defined(LINUX)
#     include "drv_mei_cpe_config_auto.h"
#else
#     error "Please specify configuration header for non Linux build!"
#  endif
#endif

/* ==========================================================================
   Description : Internal functional API of the driver.
   ========================================================================== */


/* ==========================================================================
   MISC
   ========================================================================== */
/* AR9 Device is not fully supported by the MEI CPE driver. At least FW download
   and READY message handling works fine */

#ifndef MEI_SUPPORT_DEVICE_VR9
#  define MEI_SUPPORT_DEVICE_VR9       0
#else
#  if (MEI_SUPPORT_DEVICE_VR9 != 1)
#     define MEI_SUPPORT_DEVICE_VR9    0
#  endif
#endif

#ifndef MEI_SUPPORT_DEVICE_VR10
#  define MEI_SUPPORT_DEVICE_VR10       0
#else
#  if (MEI_SUPPORT_DEVICE_VR10 != 1)
#     define MEI_SUPPORT_DEVICE_VR10    0
#  endif
#endif

#ifndef MEI_SUPPORT_DEVICE_VR10_320
#  define MEI_SUPPORT_DEVICE_VR10_320      0
#else
#  if (MEI_SUPPORT_DEVICE_VR10_320 != 1)
#     define MEI_SUPPORT_DEVICE_VR10_320   0
#  endif
#endif

#ifndef MEI_TARGET_x86
#  define MEI_TARGET_x86      0
#else
#  if (MEI_TARGET_x86 != 1)
#     define MEI_TARGET_x86   0
#  endif
#endif

#ifndef MEI_SUPPORT_DEVICE_VR11
#  define MEI_SUPPORT_DEVICE_VR11       0
#else
#  if (MEI_SUPPORT_DEVICE_VR11 != 1)
#     define MEI_SUPPORT_DEVICE_VR11    0
#  endif
#endif

#ifndef MEI_SUPPORT_DEVICE_AR9
#  define MEI_SUPPORT_DEVICE_AR9       0
#else
#  if (MEI_SUPPORT_DEVICE_AR9 != 1)
#     define MEI_SUPPORT_DEVICE_AR9    0
#  endif
#endif

#if (MEI_SUPPORT_DEVICE_VR9 == 1)
#  if (MEI_SUPPORT_DEVICE_VR10 == 1) || (MEI_SUPPORT_DEVICE_VR10_320 == 1) || (MEI_SUPPORT_DEVICE_VR11) || (MEI_SUPPORT_DEVICE_AR9 == 1)
     #error "Only one device can be selected!"
#  endif
#elif (MEI_SUPPORT_DEVICE_VR10 == 1)
#  if (MEI_SUPPORT_DEVICE_VR9 == 1) || (MEI_SUPPORT_DEVICE_VR10_320 == 1) || (MEI_SUPPORT_DEVICE_VR11) || (MEI_SUPPORT_DEVICE_AR9 == 1)
     #error "Only one device can be selected!"
#  endif
#elif (MEI_SUPPORT_DEVICE_VR10_320 == 1)
#  if (MEI_SUPPORT_DEVICE_VR9 == 1) || (MEI_SUPPORT_DEVICE_VR10 == 1) || (MEI_SUPPORT_DEVICE_VR11) || (MEI_SUPPORT_DEVICE_AR9 == 1)
     #error "Only one device can be selected!"
#  endif
#elif (MEI_SUPPORT_DEVICE_VR11 == 1)
#  if (MEI_SUPPORT_DEVICE_VR9 == 1) || (MEI_SUPPORT_DEVICE_VR10 == 1) || (MEI_SUPPORT_DEVICE_VR10_320) || (MEI_SUPPORT_DEVICE_AR9 == 1)
     #error "Only one device can be selected!"
#  endif
#elif (MEI_SUPPORT_DEVICE_AR9 == 1)
#  if (MEI_SUPPORT_DEVICE_VR9 == 1) || (MEI_SUPPORT_DEVICE_VR10 == 1) || (MEI_SUPPORT_DEVICE_VR10_320 == 1) || (MEI_SUPPORT_DEVICE_VR11)
     #error "Only one device can be selected!"
#  endif
#else
     #error "At least one device must be selected!"
#endif

#ifdef DEBUG_PRINT
#  define MEI_DEBUG_PRINT               1
#  define MEI_ERROR_PRINT               1
#else
#  define MEI_DEBUG_PRINT               0
#  define MEI_ERROR_PRINT               0
#endif

#ifdef ERROR_PRINT
#  ifdef MEI_ERROR_PRINT
#     undef MEI_ERROR_PRINT
#  endif
#  define MEI_ERROR_PRINT               1
#endif


/** trace / log file names and line number */
#if 0
#define FILE_AND_LINE_PARAMS  ,int currLine, char *pCurrFile
#define FILE_AND_LINE_ARGS    ,__LINE__ , __FILE__
#else
#define FILE_AND_LINE_PARAMS
#define FILE_AND_LINE_ARGS
#endif

/* ==========================================================================
   Feature Configuration
   ========================================================================== */

/** export the ioctl internal interface to the kernel namespace */
#ifndef MEI_EXPORT_INTERNAL_API
#  define MEI_EXPORT_INTERNAL_API             1
#endif

/** support internal preiodic task handling */
#ifndef MEI_SUPPORT_PERIODIC_TASK
#  define MEI_SUPPORT_PERIODIC_TASK           1
#endif

/** support MEI register access */
#ifndef MEI_SUPPORT_REGISTER
#  define MEI_SUPPORT_REGISTER                1
#else
#  if (MEI_SUPPORT_REGISTER != 1)
#     define MEI_SUPPORT_REGISTER             0
#  endif
#endif

/** support MEI CPE driver loop for mailbox access */
#ifndef MEI_SUPPORT_DRV_LOOPS
#  define MEI_SUPPORT_DRV_LOOPS               1
#else
#  if (MEI_SUPPORT_DRV_LOOPS != 1)
#     define MEI_SUPPORT_DRV_LOOPS            0
#  endif
#endif

#undef MEI_SUPPORT_ROM_CODE
#define MEI_SUPPORT_ROM_CODE                  0

#undef MEI_SUPPORT_DL_DMA_CS
#define MEI_SUPPORT_DL_DMA_CS                 0

#undef MEI_BM7_CODESWAP_MEIDBG
#define MEI_BM7_CODESWAP_MEIDBG               0

#undef MEI_MAX_FW_IMAGES
#define MEI_MAX_FW_IMAGES                     1

#undef MEI_DRV_POWERSAVING_ENABLED
#define MEI_DRV_POWERSAVING_ENABLED           0

#undef MEI_DRV_ATM_OAM_ENABLE
#define MEI_DRV_ATM_OAM_ENABLE                0

#undef MEI_DRV_CLEAR_EOC_ENABLE
#define MEI_DRV_CLEAR_EOC_ENABLE              0

#undef MEI_SUPPORT_VDSL2_ADSL_SWAP
#define MEI_SUPPORT_VDSL2_ADSL_SWAP           0

/** support interface to ATM/PTM drivers */
#ifndef MEI_DRV_ATM_PTM_INTERFACE_ENABLE
#  define MEI_DRV_ATM_PTM_INTERFACE_ENABLE    1
#else
#  if ( (MEI_DRV_ATM_PTM_INTERFACE_ENABLE != 1) && (MEI_DRV_ATM_PTM_INTERFACE_ENABLE != 0) )
#     undef  MEI_DRV_ATM_PTM_INTERFACE_ENABLE
#     define MEI_DRV_ATM_PTM_INTERFACE_ENABLE 0
#     warning "switch off internal ATM/PTM interface - invalid config"
#  endif
#endif

#ifndef MEI_SUPPORT_PCI_SLAVE_FW_DOWNLOAD
#  define MEI_SUPPORT_PCI_SLAVE_FW_DOWNLOAD    0
#else
#  if ( (MEI_SUPPORT_PCI_SLAVE_FW_DOWNLOAD != 1) && (MEI_SUPPORT_PCI_SLAVE_FW_DOWNLOAD != 0) )
#     undef  MEI_SUPPORT_PCI_SLAVE_FW_DOWNLOAD
#     define MEI_SUPPORT_PCI_SLAVE_FW_DOWNLOAD 0
#     warning "switch off PCI slave FW download - invalid config"
#  endif
#endif

#ifndef MEI_SUPPORT_PCI_SLAVE_ADDR_RANGE_BONDING_EXTERNAL
#  define MEI_SUPPORT_PCI_SLAVE_ADDR_RANGE_BONDING_EXTERNAL   0
#else
#  if ( (MEI_SUPPORT_PCI_SLAVE_ADDR_RANGE_BONDING_EXTERNAL != 1) && (MEI_SUPPORT_PCI_SLAVE_ADDR_RANGE_BONDING_EXTERNAL != 0) )
#     undef  MEI_SUPPORT_PCI_SLAVE_ADDR_RANGE_BONDING_EXTERNAL
#     define MEI_SUPPORT_PCI_SLAVE_ADDR_RANGE_BONDING_EXTERNAL 0
#     warning "switch off PCI Slave BONDING_EXTERNAL address range - invalid config"
#  endif
#endif

#ifndef MEI_SUPPORT_PCI_SLAVE_ADDR_RANGE_BONDING
#  define MEI_SUPPORT_PCI_SLAVE_ADDR_RANGE_BONDING   0
#else
#  if ( (MEI_SUPPORT_PCI_SLAVE_ADDR_RANGE_BONDING != 1) && (MEI_SUPPORT_PCI_SLAVE_ADDR_RANGE_BONDING != 0) )
#     undef  MEI_SUPPORT_PCI_SLAVE_ADDR_RANGE_BONDING
#     define MEI_SUPPORT_PCI_SLAVE_ADDR_RANGE_BONDING_EXTERNAL 0
#     warning "switch off PCI Slave offchip bonding address range - invalid config"
#  endif
#endif

#if (MEI_SUPPORT_PCI_SLAVE_ADDR_RANGE_BONDING_EXTERNAL != 0) && (MEI_SUPPORT_PCI_SLAVE_ADDR_RANGE_BONDING != 0)
#  error "Please select only one possible PCI Slave address range!"
#endif

#if (MEI_SUPPORT_PCI_SLAVE_FW_DOWNLOAD != 0)
# if (MEI_SUPPORT_PCI_SLAVE_ADDR_RANGE_BONDING_EXTERNAL != 1) && (MEI_SUPPORT_PCI_SLAVE_ADDR_RANGE_BONDING != 1)
#  error "Please select at least one possible PCI Slave address range!"
# endif
#endif

#if (MEI_SUPPORT_DEVICE_VR9 == 1) || (MEI_SUPPORT_DEVICE_VR10 == 1) || (MEI_SUPPORT_DEVICE_VR10_320 == 1) || (MEI_SUPPORT_DEVICE_VR11 == 1)
   /** support Debug access (via General Purpose Access - msg's) */
   #ifndef MEI_SUPPORT_DFE_GPA_ACCESS
   #  define MEI_SUPPORT_DFE_GPA_ACCESS          1
   #else
   #  if (MEI_SUPPORT_DFE_GPA_ACCESS != 1)
   #     define MEI_SUPPORT_DFE_GPA_ACCESS       0
   #  endif
   #endif
#else
   #undef MEI_SUPPORT_DFE_GPA_ACCESS
   #define MEI_SUPPORT_DFE_GPA_ACCESS            0
#endif /* #if (MEI_SUPPORT_DEVICE_VR9 == 1) || (MEI_SUPPORT_DEVICE_VR10 == 1) || (MEI_SUPPORT_DEVICE_VR11 == 1) */

/* Protected DMA access - write, read back and compare */
#ifndef MEI_PROTECTED_MEI_DMA_ACCESS
#  define MEI_PROTECTED_MEI_DMA_ACCESS        0
#endif

/** support DMA access (via MEI) */
#ifndef MEI_SUPPORT_DFE_DMA_ACCESS
#  define MEI_SUPPORT_DFE_DMA_ACCESS          1
#else
#  if (MEI_SUPPORT_DFE_DMA_ACCESS != 1)
#     define MEI_SUPPORT_DFE_DMA_ACCESS       0
#  endif
#endif

/** support Debug access (via MEI) */
#ifndef MEI_SUPPORT_DFE_DBG_ACCESS
#  define MEI_SUPPORT_DFE_DBG_ACCESS          1
#else
#  if (MEI_SUPPORT_DFE_DBG_ACCESS != 1)
#     define MEI_SUPPORT_DFE_DBG_ACCESS       0
#  endif
#endif

/** support RAW CMV messages for advanced Debug */
#ifndef MEI_SUPPORT_RAW_MSG
#  define MEI_SUPPORT_RAW_MSG                 1
#else
#  if (MEI_SUPPORT_RAW_MSG != 1)
#     define MEI_SUPPORT_RAW_MSG              0
#  endif
#endif

/** support MEI CPE driver statistics */
#ifndef MEI_SUPPORT_STATISTICS
#  define MEI_SUPPORT_STATISTICS              1
#else
#  if (MEI_SUPPORT_STATISTICS != 1)
#     define MEI_SUPPORT_STATISTICS           0
#  endif
#endif

/** support ATM OAM Statistics */
#if ( (MEI_DRV_ATM_OAM_ENABLE == 1) && (MEI_SUPPORT_STATISTICS == 1) )
#  ifndef MEI_SUPPORT_ATM_OAM_STATISTICS
#     define MEI_SUPPORT_ATM_OAM_STATISTICS   1
#  endif
#else
#  ifdef MEI_SUPPORT_ATM_OAM_STATISTICS
#     undef MEI_SUPPORT_ATM_OAM_STATISTICS
#  endif
#  define MEI_SUPPORT_ATM_OAM_STATISTICS      0
#endif

/** support Clear EOC Statisitics */
#if ( (MEI_DRV_CLEAR_EOC_ENABLE == 1) && (MEI_SUPPORT_STATISTICS == 1) )
#  ifndef MEI_SUPPORT_CEOC_STATISTICS
#  define MEI_SUPPORT_CEOC_STATISTICS         1
#  endif
#else
#  ifdef MEI_SUPPORT_CEOC_STATISTICS
#  undef MEI_SUPPORT_CEOC_STATISTICS
#  endif
#  define MEI_SUPPORT_CEOC_STATISTICS         0
#endif

/** support MEI CPE driver timing statistics */
#ifndef MEI_SUPPORT_TIME_TRACE
#  define MEI_SUPPORT_TIME_TRACE              1
#else
#  if (MEI_SUPPORT_TIME_TRACE != 1)
#     define MEI_SUPPORT_TIME_TRACE           0
#  endif
#endif

/** support MEI CPE IRQ handling */
#ifndef MEI_SUPPORT_IRQ
#  define MEI_SUPPORT_IRQ                     1
#else
#  if (MEI_SUPPORT_IRQ != 1)
#     define MEI_SUPPORT_IRQ                  0
#  endif
#endif

/** support driver autonomous messages */
#ifndef MEI_SUPPORT_DRIVER_MSG
#  define MEI_SUPPORT_DRIVER_MSG              1
#endif

/** support driver config via procfs */
#ifndef MEI_SUPPORT_PROCFS_CONFIG
#  define MEI_SUPPORT_PROCFS_CONFIG           1
#else
#  if (MEI_SUPPORT_PROCFS_CONFIG != 1)
#     define MEI_SUPPORT_PROCFS_CONFIG        0
#  endif
#endif

/** only for testing */
#ifndef MEI_SUPPORT_TEST_DEBUG
#  define MEI_SUPPORT_TEST_DEBUG              1
#endif

#if (MEI_SUPPORT_TEST_DEBUG == 1)
#  define MEI_MISC_TEST                       0
#  define MEI_SUPPORT_MEI_DEBUG               1
#  define MEI_SUPPORT_DRV_NFC_DEBUG           1
#endif

/** Support/inlclude special runtime tests */
#ifndef MEI_SUPPORT_DRV_MODEM_TESTS
#  define MEI_SUPPORT_DRV_MODEM_TESTS         0
#endif

/** support the driver message dump */
#ifndef MEI_MSG_DUMP_ENABLE
#  define MEI_MSG_DUMP_ENABLE                 1
#else
#  if (MEI_MSG_DUMP_ENABLE != 1)
#     define MEI_MSG_DUMP_ENABLE              0
#  endif
#endif

/** enable the trigger functions for debug */
#ifndef MEI_EB_TRIGGER_FCT
#  define MEI_EB_TRIGGER_FCT                  0
#else
#  if (MEI_EB_TRIGGER_FCT != 1)
#     define MEI_EB_TRIGGER_FCT               0
#  endif
#endif

/* exclude emulator option */
#ifdef INCLUDE_VR9_EMULATION_WORKAROUNDS
#undef INCLUDE_VR9_EMULATION_WORKAROUNDS
#endif

/* ==========================================================================
   Driver version
   ========================================================================== */
/** driver version as string from the auto config header*/
#define DRV_MEI_VER_STR        PACKAGE_VERSION

/** driver version, what string */
#define DRV_MEI_WHAT_STR \
   "@(#)Lantiq (VRX) DSL CPE MEI driver, version " \
   DRV_MEI_VER_STR


/* ==========================================================================
   Driver Configuration
   ========================================================================== */

#ifndef MEI_DRV_IFXOS_ENABLE
#  define MEI_DRV_IFXOS_ENABLE      1
#else
#  if (MEI_DRV_IFXOS_ENABLE != 1)
#     define MEI_DRV_IFXOS_ENABLE   0
#  endif
#endif

#ifndef MEI_DRV_LKM_ENABLE
#  define MEI_DRV_LKM_ENABLE      1
#else
#  if (MEI_DRV_LKM_ENABLE != 1)
#     define MEI_DRV_LKM_ENABLE   0
#  endif
#endif

#ifdef MEI_DRV_LKM_ENABLE
#  if (MEI_DRV_LKM_ENABLE == 0)
   #ifdef MODULE
      #undef MODULE
   #endif
#  endif
#endif

/** device name */
#ifndef DRV_MEI_NAME
#  ifdef LINUX
#     define DRV_MEI_NAME                     "mei_cpe"
      /** proc fs register name */
#     define DRV_MEI_PROC_FS_NAME             "MEI"
#     define DRV_MEIX_PROC_FS_NAME            "MEIx"
#  else
      #define DRV_MEI_NAME                    "/dev/mei_cpe"
#  endif
#else
#  error module name already specified
#endif

#ifndef PDBRAM_NAME
#define PDBRAM_NAME                          "PDBRAM"
#endif

/* VRX FW Image maximum supported cache chunks for one flavour (ADSL or VDSL) */
#if (MEI_SUPPORT_DEVICE_VR10 == 1) || (MEI_SUPPORT_DEVICE_VR10_320 == 1)
   /* VRX FW Image maximum supported cache size for PDBRAM */
   #define MEI_PDBRAM_CACHE_SIZE_BYTE           (140*1024)
   #define MEI_MAX_CACHE_CHUNK_COUNT            (3)
#else
   /** \todo [VRX518] Check consistency */
   /* VRX FW Image maximum supported cache size for PDBRAM */
   #define MEI_PDBRAM_CACHE_SIZE_BYTE           (96*1024)
   #define MEI_MAX_CACHE_CHUNK_COUNT            (2)
#endif

/** device control prefix */
#define DRV_MEI_CNTRL_PREFIX                  "cntrl"

/** Limit - max open of the same device */
#ifndef MEI_MAX_OPEN
#  define MEI_MAX_OPEN                        10
#endif


/* device is a channel device - for channel specific operation */
#define MEI_CHANNEL_DEVICE_MASK               0x7F

/** device is a control device - per MEIx chip */
#define MEI_ENTITY_CNTRL_DEVICE               0x80

/** control device mask */
#define MEI_ENTITY_CNTRL_DEV_MASK             0x3F



/** maximum instances of MEIx chip devices handled by this driver */
#ifndef MEI_MAX_DFEX_ENTITIES
#   define MEI_MAX_SUPPORTED_DFEX_ENTITIES              2
#else
#  if (MEI_MAX_DFEX_ENTITIES == 0)
#     error MEI_MAX_DFEX_ENTITIES not defined
#  endif
#endif

/** maximum devices per MEIx chip instance */
#ifndef MEI_MAX_DFE_INSTANCE_PER_ENTITY
#   if (MEI_SUPPORT_DEVICE_VR9 == 1) || (MEI_SUPPORT_DEVICE_VR10 == 1) || (MEI_SUPPORT_DEVICE_VR10_320 == 1) || (MEI_SUPPORT_DEVICE_VR11 == 1)
#       define MEI_MAX_SUPPORTED_DFE_INSTANCE_PER_ENTITY     2
#   else
#       define MEI_MAX_SUPPORTED_DFE_INSTANCE_PER_ENTITY     1
#   endif
#else
#  if (MEI_MAX_DFE_INSTANCE_PER_ENTITY == 0)
#     error MEI_MAX_DFE_INSTANCE_PER_ENTITY not defined
#  endif
#endif

#define MEI_DFE_INSTANCE_PER_ENTITY MEI_DEVICE_CFG_VALUE_GET(LinesPerDevice)
#define MEI_DFEX_ENTITIES           MEI_DEVICE_CFG_VALUE_GET(MaxDeviceNumber)
#define MEI_DFE_CHAN_DEVICES        MEI_DEVICE_CFG_VALUE_GET(DfeChanDevices)
/** maximum MEI IF per device instance */
#define MEI_MAX_SUPPORTED_MEI_IF_PER_DEVICE             1


/** maximum devices handled by this driver */
#define MEI_MAX_SUPPORTED_DFE_CHAN_DEVICES     2

/** Decrement counter value to protect "empty" IRQ requests form the OS */
#define MEI_IRQ_PROTECT_COUNT                 1000

/* ==========================================================================
   Device Buffer setup
*/

/**
   Maximum number of receive buffer.
   Currently we expect
   - NFC indication form the device.
   - [Trace Messages]
*/
#ifndef MEI_DRV_NFC_BUFFER
#  define MEI_MAX_RD_DEV_BUF_PER_DEV          2
#else
#  if MEI_DRV_NFC_BUFFER < 2
#     define MEI_MAX_RD_DEV_BUF_PER_DEV       2
#  else
#     define MEI_MAX_RD_DEV_BUF_PER_DEV       MEI_DRV_NFC_BUFFER
#  endif
#endif

/** Digital Spectrum Management (vectoring) support */
#ifndef MEI_SUPPORT_DSM
#  define MEI_SUPPORT_DSM    1
#else
#  if ( (MEI_SUPPORT_DSM != 1) && (MEI_SUPPORT_DSM != 0) )
#     undef  MEI_SUPPORT_DSM
#     define MEI_SUPPORT_DSM 1
#     warning "switch on DSM vectoring support - invalid config"
#  endif
#endif

#ifndef MEI_PREDEF_DBG_BAR
#  define MEI_PREDEF_DBG_BAR    1
#else
#  if ( (MEI_PREDEF_DBG_BAR != 1) && (MEI_PREDEF_DBG_BAR != 0) )
#     undef  MEI_PREDEF_DBG_BAR
#     define MEI_PREDEF_DBG_BAR 1
#     warning "switch on BAR debug user support - invalid config"
#  endif
#endif

#ifndef MEI_EMULATION_CONFIGURATION
#  define MEI_EMULATION_CONFIGURATION    0
#else
#  if ( (MEI_EMULATION_CONFIGURATION != 1) && (MEI_EMULATION_CONFIGURATION != 0) )
#     undef  MEI_EMULATION_CONFIGURATION
#     define MEI_EMULATION_CONFIGURATION 0
#     warning "disable emulation configuration - invalid config"
#  endif
#endif

#ifndef MEI_SUPPORT_OPTIMIZED_FW_DL
#  define MEI_SUPPORT_OPTIMIZED_FW_DL    1
#else
#  if ( (MEI_SUPPORT_OPTIMIZED_FW_DL != 1) && (MEI_SUPPORT_OPTIMIZED_FW_DL != 0) )
#     undef  MEI_SUPPORT_OPTIMIZED_FW_DL
#     define MEI_SUPPORT_OPTIMIZED_FW_DL 1
#     warning "switch on optimised fw download - invalid config"
#  endif
#endif

/** Debug Streams support */
#ifndef MEI_SUPPORT_DEBUG_STREAMS
#  define MEI_SUPPORT_DEBUG_STREAMS    1
#else
#  if ( (MEI_SUPPORT_DEBUG_STREAMS != 1) && (MEI_SUPPORT_DEBUG_STREAMS != 0) )
#     undef  MEI_SUPPORT_DEBUG_STREAMS
#     define MEI_SUPPORT_DEBUG_STREAMS 1
#     warning "switch on debug stremas support - invalid config"
#  endif
#endif

/** Debug Streams - support statistics. */
#if (MEI_SUPPORT_DEBUG_STREAMS == 1)
#  ifndef MEI_DBG_STREAMS_SUPPORT_STATISTICS
#     define MEI_DBG_STREAMS_SUPPORT_STATISTICS              1
#  endif
#else
#  ifdef MEI_DBG_STREAMS_SUPPORT_STATISTICS
#     undef MEI_DBG_STREAMS_SUPPORT_STATISTICS
#  endif
#  define MEI_DBG_STREAMS_SUPPORT_STATISTICS                 0
#endif

/** Debug Logger support */
#ifndef MEI_SUPPORT_DEBUG_LOGGER
#  define MEI_SUPPORT_DEBUG_LOGGER    1
#else
#  if ( (MEI_SUPPORT_DEBUG_LOGGER != 1) && (MEI_SUPPORT_DEBUG_LOGGER != 0) )
#     undef  MEI_SUPPORT_DEBUG_LOGGER
#     define MEI_SUPPORT_DEBUG_LOGGER 1
#     warning "switch on debug logger support - invalid config"
#  endif
#endif

/** get the ID of netlink debug messages channel from extern */
#ifndef DSL_DBG_MSG_NETLINK_ID
#  define DSL_DBG_MSG_NETLINK_ID     28
#endif


#endif      /* #ifndef _DRV_MEI_CPE_CONFIG_H */

