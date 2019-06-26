#ifndef _DRV_MEI_CPE_MEI_ACCESS_VRX_H
#define _DRV_MEI_CPE_MEI_ACCESS_VRX_H
/******************************************************************************

                          Copyright (c) 2007-2015
                     Lantiq Beteiligungs-GmbH & Co. KG

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/* ==========================================================================
   Description : MEI driver - VRX MEI Register access macros
   Remarks:
   ========================================================================== */

#ifdef __cplusplus
extern "C"
{
#endif

/* ============================================================================
   Includes
   ========================================================================= */

/* get at first the driver configuration */
#include "drv_mei_cpe_config.h"

/* MEI register definitions */
#include "drv_mei_cpe_mei_vrx.h"

/* ============================================================================
   MEI Register Access Macros (Common)
   ========================================================================= */

/* forward declaration of MEI_MEI_DRV_CNTRL_T */
struct mei_drv_cntrl_s;
typedef struct mei_drv_cntrl_s MEI_MEI_DRV_CNTRL_T;

void MEI_CGU_PLLOMCFG_CLK5_Set(MEI_MEI_DRV_CNTRL_T *pMeiDrvCntrl, IOCTL_MEI_xdslMode_t xDslModeCurrent);

#if MEI_SUPPORT_DEVICE_VR11 == 1
void MEI_CGU_PPLOMCFG_print(MEI_MEI_DRV_CNTRL_T *pMeiDrvCntrl);
#endif /* (MEI_SUPPORT_DEVICE_VR11 == 1) */

#define MEI_PARAM_COUNT_32_TO_16(paramCount32bit)   ((paramCount32bit)*sizeof(IFX_uint16_t))
#define MEI_PARAM_COUNT_16_TO_8(paramCount16bit)    ((paramCount16bit)*sizeof(IFX_uint16_t))

/** MEI Register Access */
#define MEI_REG_ACCESS_OFFSET_SET(pMeiDrvCntrl, offset, val) \
            MEI_REG_WRAP_SET((pMeiDrvCntrl)->pMeiIfCntrl->pVirtMeiRegIf->regRaw[offset], (val))
#define MEI_REG_ACCESS_OFFSET_GET(pMeiDrvCntrl, offset) \
            MEI_REG_WRAP_GET((pMeiDrvCntrl)->pMeiIfCntrl->pVirtMeiRegIf->regRaw[offset])

/** Chip Version Number Register */
#define MEI_REG_ACCESS_ME_VERSION_SET(pMeiDrvCntrl, val) \
            MEI_REG_WRAP_SET((pMeiDrvCntrl)->pMeiIfCntrl->pVirtMeiRegIf->regStruct.ME_VERSION, (val))
#define MEI_REG_ACCESS_ME_VERSION_GET(pMeiDrvCntrl) \
            MEI_REG_WRAP_GET((pMeiDrvCntrl)->pMeiIfCntrl->pVirtMeiRegIf->regStruct.ME_VERSION)

/** ARC4 to ME Interrupt Status Register */
#define MEI_REG_ACCESS_ME_ARC2ME_STAT_SET(pMeiDrvCntrl, val) \
            MEI_REG_WRAP_SET((pMeiDrvCntrl)->pMeiIfCntrl->pVirtMeiRegIf->regStruct.ME_ARC2ME_STAT, (val))
#define MEI_REG_ACCESS_ME_ARC2ME_STAT_GET(pMeiDrvCntrl) \
            MEI_REG_WRAP_GET((pMeiDrvCntrl)->pMeiIfCntrl->pVirtMeiRegIf->regStruct.ME_ARC2ME_STAT)

/** ARC4 to ME Interrupt Mask Register */
#define MEI_REG_ACCESS_ME_ARC2ME_MASK_SET(pMeiDrvCntrl, val) \
            MEI_REG_WRAP_SET((pMeiDrvCntrl)->pMeiIfCntrl->pVirtMeiRegIf->regStruct.ME_ARC2ME_MASK, (val))
#define MEI_REG_ACCESS_ME_ARC2ME_MASK_GET(pMeiDrvCntrl) \
            MEI_REG_WRAP_GET((pMeiDrvCntrl)->pMeiIfCntrl->pVirtMeiRegIf->regStruct.ME_ARC2ME_MASK)

/** Reset Control Register */
#define MEI_REG_ACCESS_ME_RST_CTRL_SET(pMeiDrvCntrl, val) \
            MEI_REG_WRAP_SET((pMeiDrvCntrl)->pMeiIfCntrl->pVirtMeiRegIf->regStruct.ME_RST_CTRL, (val))
#define MEI_REG_ACCESS_ME_RST_CTRL_GET(pMeiDrvCntrl) \
            MEI_REG_WRAP_GET((pMeiDrvCntrl)->pMeiIfCntrl->pVirtMeiRegIf->regStruct.ME_RST_CTRL)

/** Configuration Register */
#define MEI_REG_ACCESS_ME_CONFIG_SET(pMeiDrvCntrl, val) \
            MEI_REG_WRAP_SET((pMeiDrvCntrl)->pMeiIfCntrl->pVirtMeiRegIf->regStruct.ME_CONFIG, (val))
#define MEI_REG_ACCESS_ME_CONFIG_GET(pMeiDrvCntrl) \
            MEI_REG_WRAP_GET((pMeiDrvCntrl)->pMeiIfCntrl->pVirtMeiRegIf->regStruct.ME_CONFIG)

/** ME to ARC4 Interrupt Register */
#define MEI_REG_ACCESS_ME_ME2ARC_INT_SET(pMeiDrvCntrl, val) \
            MEI_REG_WRAP_SET((pMeiDrvCntrl)->pMeiIfCntrl->pVirtMeiRegIf->regStruct.ME_ME2ARC_INT, (val))
#define MEI_REG_ACCESS_ME_ME2ARC_INT_GET(pMeiDrvCntrl) \
            MEI_REG_WRAP_GET((pMeiDrvCntrl)->pMeiIfCntrl->pVirtMeiRegIf->regStruct.ME_ME2ARC_INT)

/** Debug Decode Register */
#define MEI_REG_ACCESS_ME_DBG_DECODE_SET(pMeiDrvCntrl, val) \
            MEI_REG_WRAP_SET((pMeiDrvCntrl)->pMeiIfCntrl->pVirtMeiRegIf->regStruct.ME_DBG_DECODE, (val))
#define MEI_REG_ACCESS_ME_DBG_DECODE_GET(pMeiDrvCntrl) \
            MEI_REG_WRAP_GET((pMeiDrvCntrl)->pMeiIfCntrl->pVirtMeiRegIf->regStruct.ME_DBG_DECODE)

/** Debug Master Register */
#define MEI_REG_ACCESS_ME_DBG_MASTER_SET(pMeiDrvCntrl, val) \
            MEI_REG_WRAP_SET((pMeiDrvCntrl)->pMeiIfCntrl->pVirtMeiRegIf->regStruct.ME_DBG_MASTER, (val))
#define MEI_REG_ACCESS_ME_DBG_MASTER_GET(pMeiDrvCntrl) \
            MEI_REG_WRAP_GET((pMeiDrvCntrl)->pMeiIfCntrl->pVirtMeiRegIf->regStruct.ME_DBG_MASTER)

/** Clock Control Register */
#define MEI_REG_ACCESS_ME_CLK_CTRL_SET(pMeiDrvCntrl, val) \
            MEI_REG_WRAP_SET((pMeiDrvCntrl)->pMeiIfCntrl->pVirtMeiRegIf->regStruct.ME_CLK_CTRL, (val))
#define MEI_REG_ACCESS_ME_CLK_CTRL_GET(pMeiDrvCntrl) \
            MEI_REG_WRAP_GET((pMeiDrvCntrl)->pMeiIfCntrl->pVirtMeiRegIf->regStruct.ME_CLK_CTRL)

/** Data Transfer Data Register */
#define MEI_REG_ACCESS_ME_DX_DATA_SET(pMeiDrvCntrl, val) \
            MEI_REG_WRAP_SET((pMeiDrvCntrl)->pMeiIfCntrl->pVirtMeiRegIf->regStruct.ME_DX_DATA, (val))
#define MEI_REG_ACCESS_ME_DX_DATA_GET(pMeiDrvCntrl) \
            MEI_REG_WRAP_GET((pMeiDrvCntrl)->pMeiIfCntrl->pVirtMeiRegIf->regStruct.ME_DX_DATA)

/** Data Transfer Status Register */
#define MEI_REG_ACCESS_ME_DX_STAT_SET(pMeiDrvCntrl, val) \
            MEI_REG_WRAP_SET((pMeiDrvCntrl)->pMeiIfCntrl->pVirtMeiRegIf->regStruct.ME_DX_STAT, (val))
#define MEI_REG_ACCESS_ME_DX_STAT_GET(pMeiDrvCntrl) \
            MEI_REG_WRAP_GET((pMeiDrvCntrl)->pMeiIfCntrl->pVirtMeiRegIf->regStruct.ME_DX_STAT)

/** General Purpose Register from ARC */
#define MEI_REG_ACCESS_ME_ARC_GP_STAT_SET(pMeiDrvCntrl) \
            MEI_REG_WRAP_SET((pMeiDrvCntrl)->pMeiIfCntrl->pVirtMeiRegIf->regStruct.ME_ARC_GP_STAT, (val))
#define MEI_REG_ACCESS_ME_ARC_GP_STAT_GET(pMeiDrvCntrl, val) \
            MEI_REG_WRAP_GET((pMeiDrvCntrl)->pMeiIfCntrl->pVirtMeiRegIf->regStruct.ME_ARC_GP_STAT)

/* ============================================================================
   MEI Register Access Macros (xDSL chipset specific)
   ========================================================================= */
/** Debug port select Register */
#define MEI_REG_ACCESS_ME_DBG_PORT_SEL_SET(pMeiDrvCntrl, val) \
            MEI_REG_WRAP_SET((pMeiDrvCntrl)->pMeiIfCntrl->pVirtMeiRegIf->regStruct.ME_DBG_PORT_SEL, (val))
#define MEI_REG_ACCESS_ME_DBG_PORT_SEL_GET(pMeiDrvCntrl) \
            MEI_REG_WRAP_GET((pMeiDrvCntrl)->pMeiIfCntrl->pVirtMeiRegIf->regStruct.ME_DBG_PORT_SEL)

/** DMA port select Register */
#define MEI_REG_ACCESS_ME_DX_PORT_SEL_SET(pMeiDrvCntrl, val) \
            MEI_REG_WRAP_SET((pMeiDrvCntrl)->pMeiIfCntrl->pVirtMeiRegIf->regStruct.ME_DX_PORT_SEL, (val))
#define MEI_REG_ACCESS_ME_DX_PORT_SEL_GET(pMeiDrvCntrl) \
            MEI_REG_WRAP_GET((pMeiDrvCntrl)->pMeiIfCntrl->pVirtMeiRegIf->regStruct.ME_DX_PORT_SEL)

/** ME to ARC4 Interrupt Status Register */
#define MEI_REG_ACCESS_ME_ME2ARC_STAT_SET(pMeiDrvCntrl, val) \
            MEI_REG_WRAP_SET((pMeiDrvCntrl)->pMeiIfCntrl->pVirtMeiRegIf->regStruct.ME_ME2ARC_STAT, (val))
#define MEI_REG_ACCESS_ME_ME2ARC_STAT_GET(pMeiDrvCntrl) \
            MEI_REG_WRAP_GET((pMeiDrvCntrl)->pMeiIfCntrl->pVirtMeiRegIf->regStruct.ME_ME2ARC_STAT)

/** Debug Write Address Register LO/High */
#define MEI_REG_ACCESS_ME_DBG_WR_AD_LONG_SET(pMeiDrvCntrl, val) \
            MEI_REG_WRAP_SET((pMeiDrvCntrl)->pMeiIfCntrl->pVirtMeiRegIf->regStruct.ME_DBG_WR_AD, (val))

#define MEI_REG_ACCESS_ME_DBG_WR_AD_LONG_GET(pMeiDrvCntrl) \
            MEI_REG_WRAP_GET((pMeiDrvCntrl)->pMeiIfCntrl->pVirtMeiRegIf->regStruct.ME_DBG_WR_AD)

/** Debug Read Address Register LO/High */
#define MEI_REG_ACCESS_ME_DBG_RD_AD_LONG_SET(pMeiDrvCntrl, val) \
            MEI_REG_WRAP_SET((pMeiDrvCntrl)->pMeiIfCntrl->pVirtMeiRegIf->regStruct.ME_DBG_RD_AD, (val))

#define MEI_REG_ACCESS_ME_DBG_RD_AD_LONG_GET(pMeiDrvCntrl) \
            MEI_REG_WRAP_GET((pMeiDrvCntrl)->pMeiIfCntrl->pVirtMeiRegIf->regStruct.ME_DBG_RD_AD)

/** Debug Data Register LO/High */
#define MEI_REG_ACCESS_ME_DBG_DATA_LONG_SET(pMeiDrvCntrl, val) \
            MEI_REG_WRAP_SET((pMeiDrvCntrl)->pMeiIfCntrl->pVirtMeiRegIf->regStruct.ME_DBG_DATA, (val))

#define MEI_REG_ACCESS_ME_DBG_DATA_LONG_GET(pMeiDrvCntrl) \
            MEI_REG_WRAP_GET((pMeiDrvCntrl)->pMeiIfCntrl->pVirtMeiRegIf->regStruct.ME_DBG_DATA)

/** Data Transfer Address Register LO/High */
#define MEI_REG_ACCESS_ME_DX_AD_LONG_SET(pMeiDrvCntrl, val) \
            MEI_REG_WRAP_SET((pMeiDrvCntrl)->pMeiIfCntrl->pVirtMeiRegIf->regStruct.ME_DX_AD, (val))
#define MEI_REG_ACCESS_ME_DX_AD_LONG_GET(pMeiDrvCntrl) \
            MEI_REG_WRAP_GET((pMeiDrvCntrl)->pMeiIfCntrl->pVirtMeiRegIf->regStruct.ME_DX_AD)

#if (MEI_TOTAL_BAR_REGISTER_COUNT2 == 0)
/** BARx Register*/
#define MEI_REG_ACCESS_ME_XMEM_BAR_SET(pMeiDrvCntrl, barIdx,val) \
            (pMeiDrvCntrl)->pMeiIfCntrl->pVirtMeiRegIf->regStruct.ME_XMEM_BARx[ \
            barIdx % (MEI_TOTAL_BAR_REGISTER_COUNT)] = (IFX_vuint32_t)(val)

#define MEI_REG_ACCESS_ME_XMEM_BAR_GET(pMeiDrvCntrl, barIdx) \
            (pMeiDrvCntrl)->pMeiIfCntrl->pVirtMeiRegIf->regStruct.ME_XMEM_BARx[ \
            barIdx % (MEI_TOTAL_BAR_REGISTER_COUNT)]

#else
/** BARx && BARy registers*/
#define MEI_REG_ACCESS_ME_XMEM_BAR_SET(pMeiDrvCntrl, barIdx,val) \
            do {\
            if (barIdx % (MEI_TOTAL_BAR_REGISTER_COUNT + MEI_TOTAL_BAR_REGISTER_COUNT2) < MEI_TOTAL_BAR_REGISTER_COUNT)\
               { \
                  (pMeiDrvCntrl)->pMeiIfCntrl->pVirtMeiRegIf->regStruct.ME_XMEM_BARx[ \
                     barIdx % (MEI_TOTAL_BAR_REGISTER_COUNT + MEI_TOTAL_BAR_REGISTER_COUNT2)] = (IFX_vuint32_t)(val);\
               }\
            else\
               {\
                  (pMeiDrvCntrl)->pMeiIfCntrl->pVirtMeiRegIf->regStruct.ME_XMEM_BARy[ \
                     barIdx % (MEI_TOTAL_BAR_REGISTER_COUNT + MEI_TOTAL_BAR_REGISTER_COUNT2) - MEI_TOTAL_BAR_REGISTER_COUNT] = (IFX_vuint32_t)(val);\
               }\
            }while (0)
#define MEI_REG_ACCESS_ME_XMEM_BAR_GET(pMeiDrvCntrl, barIdx) \
            barIdx % (MEI_TOTAL_BAR_REGISTER_COUNT + MEI_TOTAL_BAR_REGISTER_COUNT2) < MEI_TOTAL_BAR_REGISTER_COUNT ?\
            (pMeiDrvCntrl)->pMeiIfCntrl->pVirtMeiRegIf->regStruct.ME_XMEM_BARx[ \
            barIdx % (MEI_TOTAL_BAR_REGISTER_COUNT + MEI_TOTAL_BAR_REGISTER_COUNT2)] :\
            (pMeiDrvCntrl)->pMeiIfCntrl->pVirtMeiRegIf->regStruct.ME_XMEM_BARy[ \
                        barIdx % (MEI_TOTAL_BAR_REGISTER_COUNT + MEI_TOTAL_BAR_REGISTER_COUNT2) - MEI_TOTAL_BAR_REGISTER_COUNT]
#endif

/** Shadow Register for XDATA base address*/
#define MEI_REG_ACCESS_ME_XDATA_BASE_SH_SET(pMeiDrvCntrl, val) \
            (pMeiDrvCntrl)->pMeiIfCntrl->pVirtMeiRegIf->regStruct.ME_XDATA_BASE_SH = (IFX_vuint32_t)(val)

#define MEI_REG_ACCESS_ME_XDATA_BASE_SH_GET(pMeiDrvCntrl) \
            (pMeiDrvCntrl)->pMeiIfCntrl->pVirtMeiRegIf->regStruct.ME_XDATA_BASE_SH

/* ============================================================================
   Target Control Macros (common)
  ========================================================================= */

/*
   Clear DBG Done interrupt flag
      NOTE: write '1' to clear the corresponding flag
*/
#define MEI_DBG_CLEAR_DBG_DONE(pMeiDrvCntrl) \
            MEI_REG_ACCESS_ME_ARC2ME_STAT_SET(pMeiDrvCntrl, (ME_ARC2ME_STAT_DBG_DONE) )
/*
   Clear DBG ERR interrupt flag
      NOTE: write '1' to clear the corresponding flag
*/
#define MEI_DBG_CLEAR_DBG_ERR(pMeiDrvCntrl) \
            MEI_REG_ACCESS_ME_ARC2ME_STAT_SET(pMeiDrvCntrl, (ME_ARC2ME_STAT_DBG_ERR) )
/*
   Enable debug access: host master
*/
#define MEI_ENABLE_DBG_HOST_MASTER_ON(pMeiDrvCntrl) \
            MEI_REG_ACCESS_ME_DBG_MASTER_SET(pMeiDrvCntrl, (MEI_REG_ACCESS_ME_DBG_MASTER_GET(pMeiDrvCntrl) | ME_DBG_MASTER_HOST_MSTR) )
/*
   Disable debug access: host master
*/
#define MEI_DISABLE_DBG_HOST_MASTER_OFF(pMeiDrvCntrl) \
            MEI_REG_ACCESS_ME_DBG_MASTER_SET(pMeiDrvCntrl, (MEI_REG_ACCESS_ME_DBG_MASTER_GET(pMeiDrvCntrl) & ~(ME_DBG_MASTER_HOST_MSTR)) )
/*
   Check if the Mailbox write action was successful.
*/
#define MEI_MAILBOX_WRITE_ERROR(pMeiDrvCntrl) \
            (MEI_REG_ACCESS_ME_DX_STAT_GET(pMeiDrvCntrl) & ME_DX_STAT_DX_ERR)

/* ============================================================================
   Target Control Macros (xDSL chipset specific)
  ========================================================================= */

#define ME_ARC2ME_STAT_ARC_MSGAV_GET(pMeiDev) \
            (pMeiDev->meiDrvCntrl.intMsgMask)
/*
   Notify the ARC about mailbox message read done.
      NOTE: write '1' to clear the corresponding flag
*/
#define MEI_MAILBOX_MSG_READ_DONE(pMeiDrvCntrl) \
            MEI_REG_ACCESS_ME_ARC2ME_STAT_SET(pMeiDrvCntrl, (pMeiDrvCntrl->intMsgMask) )
/*
   Release the ROM code interrupt.
      NOTE: write '1' to clear the corresponding flag
*/
#define MEI_RELEASE_ROM_INT(pMeiDrvCntrl) \
            MEI_REG_ACCESS_ME_ARC2ME_STAT_SET(pMeiDrvCntrl, (ME_ARC2ME_STAT_ARC_GP_INT0) )
/*
   Notify the ARC for a new Mailbox message.
*/
#define MEI_MAILBOX_NFC_NEW_MSG(pMeiDrvCntrl) \
            MEI_REG_ACCESS_ME_ME2ARC_INT_SET(pMeiDrvCntrl, (pMeiDrvCntrl->intMsgMask) )
/*
   Check the Mailbox ready for next msg.
*/
#define MEI_MAILBOX_BUSY_FOR_WR(pMeiDrvCntrl) \
            (MEI_REG_ACCESS_ME_ME2ARC_STAT_GET(pMeiDrvCntrl) & (pMeiDrvCntrl->intMsgMask))

#define MEI_IS_RESET_MODE_SUPPORTED(x) ((x) == e_MEI_RESET ? IFX_TRUE : IFX_FALSE)

#ifdef __cplusplus
/* extern "C" */
}
#endif

#endif   /* #ifndef _DRV_MEI_CPE_MEI_ACCESS_VRX_H */


