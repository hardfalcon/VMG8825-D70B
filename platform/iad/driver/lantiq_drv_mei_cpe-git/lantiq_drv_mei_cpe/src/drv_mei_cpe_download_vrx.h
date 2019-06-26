/******************************************************************************

                          Copyright (c) 2007-2015
                     Lantiq Beteiligungs-GmbH & Co. KG

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/
#ifndef _DRV_MEI_CPE_DOWNLOAD_VRX_H
#define _DRV_MEI_CPE_DOWNLOAD_VRX_H

/* ==========================================================================
   Description : VRX Firmware Download definitions (ROM START).
   ========================================================================== */

#ifdef __cplusplus
extern "C"
{
#endif

/* get config */
#include "drv_mei_cpe_config.h"

/* ============================================================================
   Inlcudes
   ========================================================================= */

#include "ifx_types.h"
#include "drv_mei_cpe_mei_vrx.h"

/* ==========================================================================
   Macro defs
   ========================================================================== */

#if (MEI_SUPPORT_PCI_SLAVE_FW_DOWNLOAD == 1)

# if (MEI_SUPPORT_PCI_SLAVE_ADDR_RANGE_BONDING_EXTERNAL == 1)
#  define MEI_PCI_SLAVE_DDR_POOL_START_ADDRESS   (0x81E00000)
#  define MEI_PCI_SLAVE_PCI_POOL_START_ADDRESS   (0x18000000)
# elif (MEI_SUPPORT_PCI_SLAVE_ADDR_RANGE_BONDING == 1)
#  define MEI_PCI_SLAVE_DDR_POOL_START_ADDRESS   (0x81C00000)
#  define MEI_PCI_SLAVE_PCI_POOL_START_ADDRESS   (0x18C00000)
# else
#  error "PCI Slave FW download address range type is not defined!"
# endif

# define MEI_PCI_SLAVE_POOL_SIZE_BYTE           (0x200000)
#endif /* #if (MEI_SUPPORT_PCI_SLAVE_FW_DOWNLOAD == 1)*/

/* VR9 FW image chunk size [bytes] */
#define MEI_FW_IMAGE_CHUNK_SIZE_BYTE            (64*1024)

/* Fw bootloader default size */
#ifndef MEI_FW_DEFAULT_BOOTLOADER_SIZE_BYTE
#define MEI_FW_DEFAULT_BOOTLOADER_SIZE_BYTE     (3332)
#endif

/* Fw bootloader max size */
#ifndef MEI_FW_MAX_BOOTLOADER_SIZE_BYTE
#define MEI_FW_MAX_BOOTLOADER_SIZE_BYTE         (4096)
#endif

#if (MEI_FW_MAX_BOOTLOADER_SIZE_BYTE > MEI_FW_IMAGE_CHUNK_SIZE_BYTE)
#error Max bootloader size are more then chunk size!
#endif

#define MEI_FW_IMAGE_CHUNK_ZERO_BITS_COUNT      (5)
#define MEI_FW_IMAGE_CHUNK_OVERHEAD_SIZE_BYTE \
           (1 << MEI_FW_IMAGE_CHUNK_ZERO_BITS_COUNT)

#define MEI_FW_IMAGE_CHUNK_ADDR_MASK \
           (~(MEI_FW_IMAGE_CHUNK_OVERHEAD_SIZE_BYTE - 1))

/** \todo [VRX518] Number of BAR registers has been increased from 17 to 25.
    However initially there are only 17 used also for VRX518. Rework handling
    as/if required later on. */
#define MEI_FW_IMAGE_MAX_CHUNK_COUNT            (MEI_TOTAL_BAR_REGISTER_COUNT)

/* Image: VDSL cache (3) + ADSL cache (3) + ADSL & VDSL image (8) */
/* BAR: VDSL cache (3) + ADSL cache (3) + ADSL & VDSL image (8) + */
/* ERB chunk + Data chunk + Debug data chunk                      */
#define MEI_FW_IMAGE_MAX_CHUNK_COUNT_TYPE_0     (MEI_TOTAL_BAR_REGISTER_COUNT)

/* VR10 only */
/* Image: VDSL cache (3) + ADSL cache (3) + ADSL & VDSL image (11) */
/* BAR: ADSL | VDSL cache (3) + ADSL & VDSL image (11) +           */
/* ERB chunk + Data chunk + Debug data chunk                       */
#define MEI_FW_IMAGE_MAX_CHUNK_COUNT_TYPE_1     (MEI_TOTAL_BAR_REGISTER_COUNT + 3)

/* Image: VDSL cache (3) + ADSL cache (3) + ADSL & VDSL image (11+11)             */
/* BAR: [ADSL cache (3) + ADSL image (11)] | [VDSL cache (3) + VDSL image (11)] + */
/* ERB chunk + Data chunk + Debug data chunk                                      */
#define MEI_FW_IMAGE_MAX_CHUNK_COUNT_TYPE_2     (MEI_TOTAL_BAR_REGISTER_COUNT + 14)

#define MEI_FW_IMAGE_ERB_CHUNK_INDEX            (14)
#define MEI_FW_IMAGE_DATA_CHUNK_INDEX           (15)
#define MEI_FW_IMAGE_DEBUG_CHUNK_INDEX          (16)
#if (MEI_SUPPORT_DEVICE_VR11 == 1)
#define MEI_FW_IMAGE_ERB_PPE_CHUNK_INDEX        (17)
#define MEI_FW_IMAGE_CHIPID_EFUSE_CHUNK_INDEX   (18)
#define MEI_FW_IMAGE_FREQ_SCAL_PPE_CHUNK_INDEX  (19)
#endif /* (MEI_SUPPORT_DEVICE_VR11 == 1) */

/* VR9 FW image header size [bytes] (including Page#0)*/
#define MEI_FW_IMAGE_HEADER_SIZE_BYTE  sizeof(MEI_FW_IMAGE_CNTRL_T)

/* VR9 FW Image maximum allowable size [bytes] */
#define MEI_FW_IMAGE_MAX_SIZE_BYTE \
   (MEI_FW_IMAGE_CHUNK_SIZE_BYTE * (MEI_FW_IMAGE_MAX_CHUNK_COUNT - 1) + \
    MEI_BAR16_SIZE_BYTE)

/* VRX FW Image maximum supported cache size for one flavour (ADSL or VDSL) */
#define MEI_FW_IMAGE_MAX_CACHE_SIZE_BYTE \
   (MEI_FW_IMAGE_CHUNK_SIZE_BYTE * MEI_MAX_CACHE_CHUNK_COUNT)

/* VRX FW Image maximum supported image size for one flavour (ADSL or VDSL) */
#define MEI_FW_IMAGE_MAX_XDSL_SIZE_BYTE \
   (MEI_FW_IMAGE_CHUNK_SIZE_BYTE * 11)

/* VRX FW Image maximum size supported for different mem layout types */
/* Type 0 only for old revision, could erase ERB block by ADSL (one more chunk)*/
#define MEI_FW_IMAGE_MAX_SIZE_TYPE_0_BYTE \
   (MEI_FW_IMAGE_MAX_CACHE_SIZE_BYTE + MEI_FW_IMAGE_MAX_XDSL_SIZE_BYTE + \
    MEI_FW_IMAGE_CHUNK_SIZE_BYTE)

#define MEI_FW_IMAGE_MAX_SIZE_TYPE_1_BYTE \
   (2*MEI_FW_IMAGE_MAX_CACHE_SIZE_BYTE + MEI_FW_IMAGE_MAX_XDSL_SIZE_BYTE)

#define MEI_FW_IMAGE_MAX_SIZE_TYPE_2_BYTE \
   (2*MEI_FW_IMAGE_MAX_CACHE_SIZE_BYTE + 2*MEI_FW_IMAGE_MAX_XDSL_SIZE_BYTE)
#if (MEI_SUPPORT_DEVICE_VR11 != 1)

#define MEI_FW_IMAGE_MAX_DEBUG_DATA (1024*1024)

#else
/* Max debug data Debug Data (e.g. RTT) */
#define MEI_FW_IMAGE_MAX_DEBUG_DATA (512*1024)
#endif
/* VR9 FW combined image unique Signature#0*/
#define MEI_FW_IMAGE_SIGNATURE0   (0x2468)

/* VR9 FW combined image unique Signature#1*/
#define MEI_FW_IMAGE_SIGNATURE1   (0xB11D)

/* VR9 FW Port Mode Control Structure address within ARC internal memory*/
#define MEI_FW_PORT_MODE_CONTROL_STRUCTURE_ADDR   (0x0080)

/* VRX FW Page#2 offset in 32bit words*/
#define MEI_FW_IMAGE_PAGE2_OFFSET_32BIT   (18)

/* VR9 FW Page#3 offset in 32bit words*/
#define MEI_FW_IMAGE_PAGE3_OFFSET_32BIT   (21)

/* VRX FW Page#10 offset in 32bit words*/
#define MEI_FW_IMAGE_PAGE10_OFFSET_32BIT  (63)

/* VRX FW PartitionsPage offset in 32bit words*/
#define MEI_FW_IMAGE_PARTITIONS_PAGE_OFFSET_32BIT   (27)

#define MEI_FW_XDSL_MODE_VDSL   (0x0)
#define MEI_FW_XDSL_MODE_ADSL   (0x1)

#define MEI_FW_PORT_MODE_SINGLE (0x0)
#define MEI_FW_PORT_MODE_DUAL   (0x1)

#define MEI_FW_XDSL_MODE_LOCK   (0x1)
#define MEI_FW_XDSL_MODE_UNLOCK (0x0)

#define MEI_FW_PORT_MODE_LOCK   (0x1)
#define MEI_FW_PORT_MODE_UNLOCK (0x0)

#define MEI_HYBRID_TYPE_A       (0x6)
#define MEI_HYBRID_TYPE_BJ      (0x3)

#define MEI_FW_BOOTLOADER_ERR_INVAL_IMAGE        (0xE1)
#define MEI_FW_BOOTLOADER_ERR_INVAL_MEMEXT_SEL   (0xE2)
#define MEI_FW_BOOTLOADER_ERR_XDMA_FAILURE       (0xE3)

/* ============================================================================
   Macros
   ========================================================================= */
#define MEI_FW_IMAGE_CHUNK_ALIGNED_SIZE_BYTE(size) \
           (size + MEI_FW_IMAGE_CHUNK_OVERHEAD_SIZE_BYTE)

#define MEI_FW_IMAGE_CHUNK_ALIGNED_ADDR_GET(addr) \
           (IFX_uint8_t*)(((((IFX_uint32_t)addr) + (MEI_FW_IMAGE_CHUNK_OVERHEAD_SIZE_BYTE-1)) & \
            MEI_FW_IMAGE_CHUNK_ADDR_MASK))

/* ============================================================================
   Type definitions
   ========================================================================= */
/**
   Describes a single page within the firmware image.

\attention
   Bit 31 = 1 of the page size field (program and data)
   indicates that the page has to be loaded at boot time.
*/
typedef struct MEI_fw_image_page_s
{
   /** page[X] program memory: offset [bytes] within the image */
   IFX_uint32_t   codeOffset_Byte;
   /** page[X] program memory: destination addr. within the target */
   IFX_uint32_t   codeDestAddr;
   /** page[X] program memory: page size [32 bit words] */
   IFX_uint32_t   codePageSize_32Bit;

   /** page[X] data memory: offset [bytes] within the image */
   IFX_uint32_t   dataOffset_Byte;
   /** page[X] data memory: destination addr. within the target */
   IFX_uint32_t   dataDestAddr;
   /** page[X] data memory: page size [32 bit words] */
   IFX_uint32_t   dataPageSize_32Bit;
}  MEI_FW_IMAGE_PAGE_T;

/**
   Describes a single X (data or code) page within the firmware image.

\attention
   Bit 31 = 1 of the page size field (program and data)
   indicates that the page has to be loaded at boot time.
*/
typedef struct MEI_fw_image_pageX_s
{
   /** page[X] destination addr. within the target */
   IFX_uint32_t   destAddr;
   /** page[X] page size [32 bit words] */
   IFX_uint32_t   pageSize_32Bit;
   /** X page buffer*/
   IFX_uint32_t   *pXpage;
}  MEI_FW_IMAGE_PAGEX_T;

/**
   Describes Port Mode Control Structure.
*/
typedef struct MEI_fw_image_port_mode_control_s
{
   /**
      Offset 0, a hardcoded value of 0x2468*/
   IFX_uint16_t signature0;
   /**
      Offset 2, true/false that mode is locked by a port at the end of GHS*/
   IFX_uint8_t  dualPortModeLock;
   /**
      Offset 3, TBD */
   IFX_uint8_t  xDslModeLock;
   /**
      Offset 4, power-up default*/
   IFX_uint8_t  dualPortModePreffered;
   /**
      Offset 5, power-up default*/
   IFX_uint8_t  xDslModePreffered;
   /**
      Offset 6, being loaded, default - 0xFF*/
   IFX_uint8_t  dualPortModeCurrent;
   /**
      Offset 7, being loaded, default - 0xFF*/
   IFX_uint8_t  xDslModeCurrent;
   /**
      Offset 8, this will be used to communicate with driver if errors
      detected early in the boot process*/
   IFX_uint8_t  bootError;
   /**
      Offset 9, AFE_COLD_START = 0; AFE_WARM_START = 1*/
   IFX_uint8_t  afePowerUp;
   /**
      Offset 10, a hardcode value of 0xB11D*/
   IFX_uint16_t signature1;
   /**
      offset 12, set by the BootLoader based on info in .bin file */
   IFX_uint32_t imageOffsetSRAM;
   /**
      offset 16, TBD*/
   IFX_uint32_t bgPortSelRegValue;
   /**
      offset 20, TBD */
   IFX_uint8_t bgPort;
   /**
      offset 21, TBD*/
   IFX_uint8_t bgDuration;
   /**
      offset 22, TBD*/
   IFX_uint8_t maxBgDuration;
   /**
      offset 23; TBD*/
   IFX_uint8_t afeInitState;
}  MEI_FW_PORT_MODE_CONTROL_T;

/**
   Describes Port Mode Control Structure for 32-bit DMA transfers.
*/
typedef struct MEI_fw_image_port_mode_control_dma32_s
{
#if (MEI_DRV_OS_BYTE_ORDER == MEI_DRV_OS_BIG_ENDIAN)
   /**
      Offset 3, TBD */
   IFX_uint8_t  xDslModeLock;
   /**
      Offset 2, true/false that mode is locked by a port at the end of GHS*/
   IFX_uint8_t  dualPortModeLock;
   /**
      Offset 0, a hardcoded value of 0x2468*/
   IFX_uint16_t signature0;
   /**
      Offset 7, being loaded, default - 0xFF*/
   IFX_uint8_t  xDslModeCurrent;
   /**
      Offset 6, being loaded, default - 0xFF*/
   IFX_uint8_t  dualPortModeCurrent;
   /**
      Offset 5, power-up default*/
   IFX_uint8_t  xDslModePreffered;
   /**
      Offset 4, power-up default*/
   IFX_uint8_t  dualPortModePreffered;
   /**
      Offset 10, a hardcode value of 0xB11D*/
   IFX_uint16_t signature1;
   /**
      Offset 9, AFE_COLD_START = 0; AFE_WARM_START = 1*/
   IFX_uint8_t  afePowerUp;
   /**
      Offset 8, this will be used to communicate with driver if errors
      detected early in the boot process*/
   IFX_uint8_t  bootError;
   /**
      offset 12, set by the BootLoader based on info in .bin file */
   IFX_uint32_t imageOffsetSRAM;
   /**
      offset 16, TBD*/
   IFX_uint32_t bgPortSelRegValue;
   /**
      offset 23; TBD*/
   IFX_uint8_t afeInitState;
   /**
      offset 22, TBD*/
   IFX_uint8_t maxBgDuration;
   /**
      offset 21, TBD*/
   IFX_uint8_t bgDuration;
   /**
      offset 20, TBD */
   IFX_uint8_t bgPort;
#else
   /**
      Offset 0, a hardcoded value of 0x2468*/
   IFX_uint16_t signature0;
   /**
      Offset 2, true/false that mode is locked by a port at the end of GHS*/
   IFX_uint8_t  dualPortModeLock;
   /**
      Offset 3, TBD */
   IFX_uint8_t  xDslModeLock;
   /**
      Offset 4, power-up default*/
   IFX_uint8_t  dualPortModePreffered;
   /**
      Offset 5, power-up default*/
   IFX_uint8_t  xDslModePreffered;
   /**
      Offset 6, being loaded, default - 0xFF*/
   IFX_uint8_t  dualPortModeCurrent;
   /**
      Offset 7, being loaded, default - 0xFF*/
   IFX_uint8_t  xDslModeCurrent;
   /**
      Offset 8, this will be used to communicate with driver if errors
      detected early in the boot process*/
   IFX_uint8_t  bootError;
   /**
      Offset 9, AFE_COLD_START = 0; AFE_WARM_START = 1*/
   IFX_uint8_t  afePowerUp;
   /**
      Offset 10, a hardcode value of 0xB11D*/
   IFX_uint16_t signature1;
   /**
      offset 12, set by the BootLoader based on info in .bin file */
   IFX_uint32_t imageOffsetSRAM;
   /**
      offset 16, TBD*/
   IFX_uint32_t bgPortSelRegValue;
   /**
      offset 20, TBD */
   IFX_uint8_t bgPort;
   /**
      offset 21, TBD*/
   IFX_uint8_t bgDuration;
   /**
      offset 22, TBD*/
   IFX_uint8_t maxBgDuration;
   /**
      offset 23; TBD*/
   IFX_uint8_t afeInitState;
#endif
}  MEI_FW_PORT_MODE_CONTROL_DMA32_T;

typedef MEI_FW_PORT_MODE_CONTROL_T MEI_FW_IMAGE_PAGE0_T;

/**
   Describes xDSL FW image Page#2 (for revision 1).
*/
typedef struct MEI_fw_image_combined_page2_s
{
   IFX_uint32_t fwRevision;
   IFX_uint32_t fwMemLayout_Type;
   IFX_uint32_t debug_data_size;
   IFX_uint32_t bootloader_size;
} MEI_FW_IMAGE_PAGE2_T;

/**
   Describes xDSL FW image Page#3 (CACHE info).
*/
typedef struct MEI_fw_image_combined_page3_s
{
   IFX_uint32_t cache_offset_Byte;
   IFX_uint32_t reserved;
   IFX_uint32_t cachedRegionSize_Byte;
} MEI_FW_IMAGE_PAGE3_T;

typedef struct MEI_fw_image_partitions_page_s
{
   IFX_uint32_t vDSL_image_offset_ARC;
   IFX_uint32_t vDSL_image_offset;
   IFX_uint32_t vDSL_image_size;
   IFX_uint32_t aDSL_image_offset_ARC;
   IFX_uint32_t aDSL_image_offset;
   IFX_uint32_t aDSL_image_size;
   IFX_uint32_t vDSL_cache_offset_ARC;
   IFX_uint32_t vDSL_cache_offset;
   IFX_uint32_t vDSL_cache_size;
   IFX_uint32_t aDSL_cache_offset_ARC;
   IFX_uint32_t aDSL_cache_offset;
   IFX_uint32_t aDSL_cache_size;
} MEI_FW_IMAGE_PARTITIONS_PAGE_T;

/**
   MEI Firmware image control data.
*/
typedef struct MEI_fw_image_cntrl_s
{
   /** fw image size in bytes */
   IFX_uint32_t         imageSize_Bytes;
   /** fw image checksum */
   IFX_uint32_t         imageCheckSum;
   /** fw image number of pages */
   IFX_uint32_t         imageNumOfPages;
   /** fw image pages info */
   union
   {
      /** fw image Page#0. */
      MEI_FW_IMAGE_PAGE0_T          imagePage0;
      /** fw image Page#X*/
      MEI_FW_IMAGE_PAGE_T           imagePageX[1];
   }imagePage;
} MEI_FW_IMAGE_CNTRL_T;

typedef enum
{
   /** undefined image chunk*/
   eMEI_FW_IMAGE_CHUNK_UNDEFINED,
   /** cached image chunk*/
   eMEI_FW_IMAGE_CHUNK_CACHED,
   /** permanent data image chunk*/
   eMEI_FW_IMAGE_CHUNK_DATA,
   /** reallocatable image chunk*/
   eMEI_FW_IMAGE_CHUNK_REALLOC,
#if (MEI_SUPPORT_DSM == 1)
   /** DSM Vectoring chunk for ERB buffer */
   eMEI_FW_IMAGE_CHUNK_ERB,
#endif /* (MEI_SUPPORT_DSM == 1) */
} MEI_FW_IMAGE_CHUNK_TYPE_E;

/**
   MEI Firmware partitions information for fw layout type 2
*/
typedef struct MEI_fw_partitions_s
{
   /** bootloader size [bytes] */
   /** bootloader offset is 0 */
   IFX_uint32_t bootloader_size;

   /** VDSL cache offset [bytes] */
   IFX_uint32_t vDSL_cache_offset;
   /** VDSL cache size [bytes] */
   IFX_uint32_t vDSL_cache_size;
   /** bootloader + VDSL cache first chunk */
   IFX_uint32_t vDSL_cache_chunk_idx;

   /** ADSL cache offset [bytes] */
   IFX_uint32_t aDSL_cache_offset;
   /** ADSL cache size [bytes] */
   IFX_uint32_t aDSL_cache_size;
   /** bootloader + ADSL cache first chunk */
   IFX_uint32_t aDSL_cache_chunk_idx;

   /** VDSL full image offset [bytes] */
   IFX_uint32_t vDSL_image_offset;
   /** VDSL full image size [bytes] */
   IFX_uint32_t vDSL_image_size;
   /** VDSL fullimage first chunk */
   IFX_uint32_t vDSL_image_chunk_idx;

   /** ADSL full image offset [bytes] */
   IFX_uint32_t aDSL_image_offset;
   /** ADSL full image size [bytes] */
   IFX_uint32_t aDSL_image_size;
   /** ADSL fullimage first chunk */
   IFX_uint32_t aDSL_image_chunk_idx;

   /** Debug Data (e.g. RTT)  */
   IFX_uint32_t debug_data_size;
} MEI_FW_PARTITIONS;

/**
   MEI Firmware binary partitions available types
*/
typedef enum
{
   /** bootloader  */
   eMEI_FW_PARTITION_BOOTLOADER,
   /** xDSL cache */
   eMEI_FW_PARTITION_XDSL_CACHE,
   /** xDSL full image */
   eMEI_FW_PARTITION_XDSL_IMAGE
} MEI_FW_PARTITION_TYPE;

#if (MEI_SUPPORT_OPTIMIZED_FW_DL == 1)
/**
   MEI Firmware binary partitions suitable flavours
*/
typedef enum
{
   /** for ADSL and VDSL */
   eMEI_FW_CHUNK_XDSL,
   /** only for ADSL  */
   eMEI_FW_CHUNK_ADSL,
   /** only for VDSL */
   eMEI_FW_CHUNK_VDSL
} MEI_FW_CHUNK_FLAVOUR;;
#endif /* (MEI_SUPPORT_OPTIMIZED_FW_DL == 1)*/

typedef enum
{
   /** original revision 0 */
   eMEI_FW_INTERFACE_REV_0,
   /** new revision 1 (for layout type 2) */
   eMEI_FW_INTERFACE_REV_1
} MEI_FW_REVISION;

typedef enum
{
   /** original mem layout type 0 */
   eMEI_FW_MEM_LAYOUT_TYPE_0,
   /** mem layout type 1 (ADSL cache excluded) */
   eMEI_FW_MEM_LAYOUT_TYPE_1,
   /** mem layout type 2 (cache and image according to the current xDSL mode) */
   eMEI_FW_MEM_LAYOUT_TYPE_2
} MEI_FW_MEM_LAYOUT_TYPE;

typedef struct
{
   /* FW image chunk size [byte]*/
   IFX_uint32_t              imageChunkSize_byte;
   /* FW image chunk type*/
   MEI_FW_IMAGE_CHUNK_TYPE_E eImageChunkType;
   /* Pointer to the FW image allocated chunk*/
   IFX_uint8_t               *pImageChunk_allocated;
   /* Pointer to the FW image virtual address aligned chunk*/
   IFX_uint8_t               *pImageChunk_aligned;
   /* Pointer to the FW image physical address aligned chunk*/
   MEI_DRVOS_DMA_T           pImageChunk_phy;
   /* BARx content*/
   IFX_uint8_t               *pBARx;
#if (MEI_SUPPORT_OPTIMIZED_FW_DL == 1)
   /* FW image chunk available data size [byte] [0,ChunkSize]*/
   IFX_uint32_t              imageChunkDataSize_byte;
   /* FW image chunk CRC */
   IFX_uint32_t              imageChunkCRC;
   /* FW image chunk flavour */
   MEI_FW_CHUNK_FLAVOUR      imageChunkFlavour;
#endif /* (MEI_SUPPORT_OPTIMIZED_FW_DL == 1)*/
} MEI_FW_IMAGE_CHUNK_CTRL_T;

typedef enum
{
   /** could not be used for allocated chunk, PDBRAM or debug mode */
   eMEI_BAR_TYPE_UNUSED,
   /** used for allocated chunk */
   eMEI_BAR_TYPE_CHUNK,
   /** used for PDBRAM */
   eMEI_BAR_TYPE_PDBRAM,
#if (MEI_SUPPORT_DSM == 1)
   /** used for ERB block, could not be used for debug mode */
   eMEI_BAR_TYPE_ERB,
#endif /* (MEI_SUPPORT_DSM == 1)*/
   /** only for special cases (RTT, etc) could not be used for debug mode */
   eMEI_BAR_TYPE_SPECIAL,
   /** used for debug mode via proc */
   eMEI_BAR_TYPE_USER,
} MEI_BAR_TYPE;

#if (MEI_SUPPORT_PCI_SLAVE_FW_DOWNLOAD == 1)
typedef struct MEI_pci_slave_pool_element_s
{
   IFX_uint32_t  element_offset;
   IFX_uint32_t  element_size_byte;
   IFX_boolean_t bUsed;
   struct MEI_pci_slave_pool_element_s *pNext;
} MEI_PCI_SLAVE_POOL_ELEMENT_T;


typedef struct MEI_pci_slave_pool_s
{
   IFX_uint8_t *pPciStart;
   IFX_uint8_t *pDdrStart;
   IFX_uint32_t pool_size_byte;
   IFX_uint32_t fill_offset;
   MEI_PCI_SLAVE_POOL_ELEMENT_T Head;
} MEI_PCI_SLAVE_POOL_T;
#endif /* #if (MEI_SUPPORT_PCI_SLAVE_FW_DOWNLOAD == 1)*/

/**
   Firmware download control struct
*/
typedef struct MEI_fw_download_cntrl_s
{
   /** line number */
   IFX_uint8_t                line_num;
#if (MEI_SUPPORT_PCI_SLAVE_FW_DOWNLOAD == 1)
   /** PCI slave/master download*/
   IFX_boolean_t              bPciSlave;
   MEI_PCI_SLAVE_POOL_T       *pPool;
#endif /* #if (MEI_SUPPORT_PCI_SLAVE_FW_DOWNLOAD == 1)*/
   /** size of the firmware image [byte] */
   IFX_uint32_t               size_byte;
   /** size of the permanent CACHED region [byte] */
   IFX_uint32_t               cachedRegionSize_byte;
   /** size of external  writeable DATA region [bytes]  */
   IFX_uint32_t               dataRegionSize_Byte;
   /** Total number of swap pages*/
   IFX_uint32_t               imageNumOfPages;
   /** Default Port Mode Control Structure (extracted from the FW header)*/
   MEI_FW_PORT_MODE_CONTROL_T defaultPortModeCtrl;
   /** FW image chunk control data*/
   MEI_FW_IMAGE_CHUNK_CTRL_T  imageChunkCtrl[MEI_FW_IMAGE_MAX_CHUNK_COUNT_TYPE_2];

   /** Current firmware revision */
   MEI_FW_REVISION            eFwRevision;

   /** Current firmware memory layout type */
   MEI_FW_MEM_LAYOUT_TYPE     eFwMemLayoutType;

   /** Partitions information (available for fw layout type 2) */
   MEI_FW_PARTITIONS          meiPartitions;

   /** BAR registers type table */
   MEI_BAR_TYPE               eBarType[MEI_TOTAL_BAR_REGISTER_COUNT + MEI_TOTAL_BAR_REGISTER_COUNT2];
#if (MEI_PREDEF_DBG_BAR == 1)
   IFX_uint32_t               dbgBarAddr[MEI_TOTAL_BAR_REGISTER_COUNT + MEI_TOTAL_BAR_REGISTER_COUNT2];
#endif /* (MEI_PREDEF_DBG_BAR == 1) */

   /** Max amount of chunks (depends of fw layout type) */
   IFX_uint32_t               meiMaxChunkCount;
   /** Used (allocated) amount of chunks (depends of fw layout type) */
#if (MEI_SUPPORT_OPTIMIZED_FW_DL == 1)
   IFX_uint32_t               meiUsedChunkCount;
   /** If chunks already filled they could be reused next fw download */
   IFX_boolean_t              bChunksFilled;
#endif /* (MEI_SUPPORT_OPTIMIZED_FW_DL == 1)*/
   /** Offset of special chunks (linked to the BAR14, BAR15, BAR16) */
   /** BAR[14] -> chunk[14+meiSpecialChunkOffset] */
   IFX_uint32_t               meiSpecialChunkOffset;
} MEI_FW_DOWNLOAD_CNTRL_T;

/**
   Firmware binary partitions info
*/
typedef struct MEI_fw_parts_cntrl_s
{
   unsigned char *pImage;
   IFX_boolean_t bHandleNeed;
}  MEI_FW_PARTS_CNTRL_T;

/* ==========================================================================
   Global Variable Definitions
   ========================================================================== */


/* ============================================================================
   Exports
   ========================================================================= */

extern IFX_int32_t MEI_DEV_IoctlFirmwareDownload(
                              MEI_DYN_CNTRL_T        *pMeiDynCntrl,
                              IOCTL_MEI_fwDownLoad_t *pArgFwDl,
                              IFX_boolean_t            bInternCall);

#if (MEI_SUPPORT_PCI_SLAVE_FW_DOWNLOAD == 1)
MEI_PCI_SLAVE_POOL_T* MEI_VR9_PciSlavePoolCreate(
                                 IFX_uint8_t *pPciStart,
                                 IFX_uint8_t *pDdrStart,
                                 IFX_uint32_t pool_size_byte);

IFX_void_t MEI_VR9_PciSlavePoolDelete(
                                 MEI_PCI_SLAVE_POOL_T *pPool);
#endif /* #if (MEI_SUPPORT_PCI_SLAVE_FW_DOWNLOAD == 1)*/
#if (MEI_SUPPORT_DEVICE_VR11 == 1)
struct MEI_dev_s;
typedef struct MEI_dev_s MEI_DEV_T;
IFX_int32_t MEI_VR11_ErbBarSet(
                                 MEI_DEV_T *pMeiDev,
                                 unsigned int data,
                                 unsigned int descriptor);

IFX_int32_t MEI_VR11_BarSafeLocationAddressUpdate(
                                 MEI_DEV_T *pMeiDev);
#endif /* (MEI_SUPPORT_DEVICE_VR11 == 1) */


#ifdef __cplusplus
/* extern "C" */
}
#endif

#endif   /* #ifndef _DRV_MEI_CPE_DOWNLOAD_VRX_H */


