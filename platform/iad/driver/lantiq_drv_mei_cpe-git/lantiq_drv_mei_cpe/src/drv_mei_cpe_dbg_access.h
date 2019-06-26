#ifndef _DRV_MEI_CPE_DBG_ACCESS_H
#define _DRV_MEI_CPE_DBG_ACCESS_H
/******************************************************************************

                          Copyright (c) 2007-2015
                     Lantiq Beteiligungs-GmbH & Co. KG

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/* ==========================================================================
   Description : Debug and DMA access to the VRX Device
   ========================================================================== */


#ifdef __cplusplus
extern "C"
{
#endif

/* ============================================================================
   Inlcudes
   ========================================================================= */

/* get at first the driver configuration */
#include "drv_mei_cpe_config.h"

#include "ifx_types.h"

#include "cmv_message_format.h"
#include "drv_mei_cpe_interface.h"
#include "drv_mei_cpe_api.h"


/* ============================================================================
   Macro Definitions
   ========================================================================= */
#define MEI_MAX_DMA_COUNT_32BIT     0x0FF

/*
   Headline - NFC debug output:

   <MEI_DRV[00]: NFC List[00] - rdIdx =  0, wrIdx =  0 t = 1431635>  --> len = 72
   <        NFC Buffer Length: - max NFC buffers: 5>                   --> len = 48
*/
#define MEI_NFC_DBG_OUT_HEADLINE           (72 + 48 + 32)


/*
   Buffer Content - NFC debug output:

   <        NfcBuf[00] cntrl = 0x00000000 len =   0 t =        0 -  MSG: 0x0000 0x0000 0x0000 0x0000 0x0000 0x0000>

   len = 111 * (NFC buffer per instance)
*/
#define MEI_NFC_DBG_OUT_BUFLINE            128


/* size of the display buffer for NFC (max 4 open instances) */
#define MEI_NFC_DISPLAY_BUFFER_SIZE  (4 * (MEI_NFC_DBG_OUT_HEADLINE + \
                                             (MEI_NFC_DBG_OUT_BUFLINE*MEI_MAX_RD_DEV_BUF_PER_DEV)) )

/* ============================================================================
   Global Firmware Debug and DMA functions
   ========================================================================= */

#if (MEI_SUPPORT_MEI_DEBUG == 1)
extern IFX_void_t MEI_MeiRegsShow(
                              MEI_DEV_T *pMeiDev);

extern IFX_void_t MEI_ShowDrvBuffer(
                              MEI_DYN_CNTRL_T *pMeiDynCntrl,
                              IFX_int8_t        bufNum,
                              IFX_int32_t       count);
#endif

#if (MEI_SUPPORT_REGISTER == 1)
extern IFX_int32_t MEI_Set_Register(
                              MEI_DEV_T *pMeiDev,
                              IFX_uint32_t offset,
                              IFX_uint32_t val);

extern IFX_int32_t MEI_Get_Register(
                              MEI_DEV_T *pMeiDev,
                              IFX_uint32_t regadr,
                              IFX_uint32_t *val);
#endif

#if (MEI_SUPPORT_DFE_DBG_ACCESS == 1)
extern IFX_int32_t MEI_IoctlMeiDbgAccessWr(
                              MEI_DYN_CNTRL_T       *pMeiDynCntrl,
                              IOCTL_MEI_dbgAccess_t *pArgDbgAccessInOut);

extern IFX_int32_t MEI_IoctlMeiDbgAccessRd(
                              MEI_DYN_CNTRL_T       *pMeiDynCntrl,
                              IOCTL_MEI_dbgAccess_t *pArgDbgAccessInOut);
#endif


#if (MEI_SUPPORT_DFE_DMA_ACCESS == 1)
extern IFX_int32_t MEI_MeiDmaTest(
                              MEI_DEV_T    *pMeiDev,
                              IFX_uint32_t   destAddr,
                              IFX_uint32_t   dma_count,
                              IFX_uint32_t   test_count);

extern IFX_int32_t MEI_IoctlDmaAccessWr(
                              MEI_DYN_CNTRL_T        *pMeiDynCntrl,
                              IOCTL_MEI_DMA_access_t *pDmaArgument);

extern IFX_int32_t MEI_IoctlDmaAccessRd(
                              MEI_DYN_CNTRL_T        *pMeiDynCntrl,
                              IOCTL_MEI_DMA_access_t *pDmaArgument);
#endif

#if (MEI_SUPPORT_DFE_GPA_ACCESS == 1)
extern IFX_int32_t MEI_GpaWrAccess(
                              MEI_DYN_CNTRL_T *pMeiDynCntrl,
                              IFX_uint32_t      dest,
                              IFX_uint32_t      addr, IFX_uint32_t val);

extern IFX_int32_t MEI_GpaRdAccess(
                              MEI_DYN_CNTRL_T *pMeiDynCntrl,
                              IFX_uint32_t      dest,
                              IFX_uint32_t      addr, IFX_uint32_t *val);

#endif


#ifdef __cplusplus
/* extern "C" */
}
#endif

#endif      /* _DRV_MEI_CPE_DBG_ACCESS_H */

