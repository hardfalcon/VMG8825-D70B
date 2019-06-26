#ifndef _drv_MEI_CPE_rom_handler_h
#define _drv_MEI_CPE_rom_handler_h
/******************************************************************************

                          Copyright (c) 2007-2015
                     Lantiq Beteiligungs-GmbH & Co. KG

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/* ==========================================================================
   Description : VRX Firmware Boot ROM handler
   ========================================================================== */

#ifdef __cplusplus
extern "C"
{
#endif

/* ==========================================================================
   includes
   ========================================================================== */

/* get at first the driver configuration */
#include "drv_mei_cpe_config.h"

#include "ifx_types.h"
#include "drv_mei_cpe_api.h"


#if (MEI_SUPPORT_ROM_CODE == 1)
/* device boot code messages */
#include "user_if_vdsl2_boot_messages.h"
#endif

/* ============================================================================
   Macros for boot messages - default configuration
   ========================================================================= */


/* ==========================================================================
   BOOT modes
*/

/**
   Defintions of the boot mode.

   Boot Mode:  0x00 --> 0000 0000 b
   ==========              | ----
                           |   |
                           |   +----> HW BTCFG[3:0]
                           +--------> 0: "AUTO"
                                      1: "Firmware Download"
*/
#define MEI_DEV_BOOT_MODE_START_AUTO             0x10

/** Boot by external Host with FW download by ARC */
#define MEI_DEV_BOOT_MODE_FWDL_HOST              0x00

/** Boot by external Host with FW download by ARC */
#define MEI_DEV_BOOT_MODE_FWDL_HOST_WITH_TARGET  0x0A
/** Boot by external host without FW downlaod (via CodeSwap) */
#define MEI_DEV_BOOT_MODE_FWDL_HOST_VIA_CS       0x0B
/** Boot by external host with FW download by Host (via DMA) */
#define MEI_DEV_BOOT_MODE_FWDL_HOST_VIA_DMA      0x0B

/** Boot from ROM and download from Host via DMA and CodeSwap */
#define MEI_DEV_BOOT_MODE_FWDL_ROM_DMA_CS        0x07

/** Boot from ROM and download from Host's FLASH into on-chip SRAM */
#define MEI_DEV_BOOT_MODE_FWDL_ROM_START_SRAM    0x08
/** Boot from ROM and download from Host's FLASH into external SRAM */
#define MEI_DEV_BOOT_MODE_FWDL_ROM_START_EXT_RAM 0x09


/* ============================================================================
   Exports
   ========================================================================= */

extern IFX_int32_t MEI_WriteRomBootMsg( MEI_DEV_T *pMeiDev,
                                              MEI_Mailbox_t *pBootMsg,
                                              IFX_int32_t  msgSize );

extern IFX_void_t MEI_RecvRomBootMsg(MEI_DEV_T *pMeiDev);


#if (MEI_SUPPORT_DFE_GPA_ACCESS == 1)
extern IFX_int32_t MEI_RomHandlerWrGpa( MEI_DEV_T *pMeiDev,
                                              IFX_boolean_t aux,
                                              IFX_uint32_t addr, IFX_uint32_t val);

extern IFX_int32_t MEI_RomHandlerRdGpa( MEI_DEV_T *pMeiDev,
                                              IFX_boolean_t aux,
                                              IFX_uint32_t addr, IFX_uint32_t *val);
#endif

#ifdef __cplusplus
/* extern "C" */
}
#endif

#endif      /* #ifndef _drv_MEI_CPE_rom_handler_h */

