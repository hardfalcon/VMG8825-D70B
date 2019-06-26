#ifndef _drv_MEI_CPE_rom_handler_if_h
#define _drv_MEI_CPE_rom_handler_if_h
/******************************************************************************

                          Copyright (c) 2007-2015
                     Lantiq Beteiligungs-GmbH & Co. KG

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/* ==========================================================================
   Description : VRX Firmware Download interface function (ROM START)
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
/* get at first the driver configuration */
#include "ifx_types.h"

/* add VRX OS Layer */
#include "drv_mei_cpe_os.h"
/* add VRX debug/printout part */
#include "drv_mei_cpe_dbg.h"

/*
   support ROM Code
*/
#if (MEI_SUPPORT_ROM_CODE == 1)

/* ============================================================================
   defs ROM Handler interface
   ========================================================================= */

/**
   Header size [16 bit] of the boot message.
   - Will be the same like for the CMV messages
*/
#define MEI_BOOT_HEADER_16BIT_SIZE      6

#if (MEI_SUPPORT_DL_DMA_CS == 1)
/* ============================================================================
   Defs ROM DMA CodeSwap firmware download
   ========================================================================= */

/**
   State Machine - ROM DMA CodeSwap firmware download.
*/
typedef enum
{
   /** download still not started */
   e_MEI_FWDL_DMA_INIT,
   /** download boot code */
   e_MEI_FWDL_DMA_DO_BOOT_DL,
   /** the boot code download done */
   e_MEI_FWDL_DMA_BOOT_DL_DONE,
   /** now wait for modem */
   e_MEI_FWDL_DMA_WAIT_ACK_GOONLINE,
   /** download successful finished */
   e_MEI_FWDL_DMA_FINISHED,
   /** download has been aborted */
   e_MEI_FWDL_DMA_ABORT
} MEI_FWDL_DMACS_STATE_E;

/**
   Contains firmware download control data (via DMA CodeSwap).
*/
typedef struct MEI_fwdl_dmacs_control_s
{
   /** firmware download state */
   MEI_FWDL_DMACS_STATE_E fwDlState;

} MEI_FWDL_DMACS_CONTROL_T;


#endif      /* MEI_SUPPORT_DL_DMA_CS */

/* ============================================================================
   Exports
   ========================================================================= */

/* VRX-Driver: ROM Code debug module - declare print level variable */
MEI_DRV_PRN_USR_MODULE_DECL(MEI_ROM);
MEI_DRV_PRN_INT_MODULE_DECL(MEI_ROM);

extern IFX_int32_t MEI_MaxWaitInitDone_ms;

#endif      /* MEI_SUPPORT_ROM_CODE */

#ifdef __cplusplus
/* extern "C" */
}
#endif

#endif      /* #ifndef _drv_MEI_CPE_rom_handler_if_h */

