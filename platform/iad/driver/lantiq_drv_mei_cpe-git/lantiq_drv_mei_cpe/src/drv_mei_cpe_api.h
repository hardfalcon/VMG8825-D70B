#ifndef _DRV_MEI_CPE_API_H
#define _DRV_MEI_CPE_API_H
/******************************************************************************

                          Copyright (c) 2007-2015
                     Lantiq Beteiligungs-GmbH & Co. KG

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/* ==========================================================================
   Description : Internal functional API of the driver.
   ========================================================================== */

/* ==========================================================================
   includes
   ========================================================================== */

/* get at first the driver configuration */
#include "drv_mei_cpe_config.h"

#include "ifx_types.h"

/* add MEI DRV OS Layer */
#include "drv_mei_cpe_os.h"
/* add MEI DRV debug/printout part */
#include "drv_mei_cpe_dbg.h"

/* get interface and configuration */
#include "cmv_message_format.h"
#include "drv_mei_cpe_mailbox.h"
#include "drv_mei_cpe_interface.h"
#include "drv_mei_cpe_mei_interface.h"
#include "drv_mei_cpe_api_intern.h"

#include "drv_mei_cpe_download_vrx.h"

#if (MEI_SUPPORT_DSM == 1)
#include "drv_mei_cpe_dsm_common.h"
#endif /* (MEI_SUPPORT_DSM == 1) */

#if (MEI_SUPPORT_ROM_CODE == 1)
/* get ROM handler definitions */
#include "drv_mei_cpe_rom_handler_if.h"
#endif

#if (MEI_DRV_ATM_OAM_ENABLE == 1)
#include "drv_mei_cpe_atmoam.h"
#endif

#if (MEI_DRV_CLEAR_EOC_ENABLE == 1)
#include "drv_mei_cpe_clear_eoc.h"
#endif

#if (MEI_SUPPORT_DEBUG_STREAMS == 1)
#include "drv_mei_cpe_dbg_streams_common.h"
#endif /* (MEI_SUPPORT_DEBUG_STREAMS == 1) */

#include "drv_mei_cpe_api_atm_ptm_intern.h"



/* ==========================================================================
   Global macros - constant definitions
   ========================================================================== */

/** protection flag for the timeout values set from outside */
#define MEI_CFG_DEF_WAIT_PROTECTION_FLAG            0x80000000
/** default startup cfg: max wait time for the MODEM READY online msg */
#if (MEI_EMULATION_CONFIGURATION == 0)
#define MEI_CFG_DEF_WAIT_FOR_MODEM_READY_SEC        10000
#else
/** In case of emulation on the Palladium it might take up to 20 minutes!!! */
#define MEI_CFG_DEF_WAIT_FOR_MODEM_READY_SEC        1200000
#endif
#define MEI_CFG_DEF_WAIT_FOR_PDBRAM_ACCESS_TOTAL    100
#define MEI_CFG_DEF_WAIT_FOR_PDBRAM_ACCESS_ATTEMPT  20

/** default startup cfg: max wait time for the MODEM READY online msg (bootmode 7) */
#define MEI_CFG_DEF_WAIT_FOR_MODEM_READY_BM7_SEC    4000
/** default startup cfg: max wait time for normal msg response */
#define MEI_CFG_DEF_WAIT_FOR_DEVICE_RESPONCE         2000

#define MEI_DBG_FLAGS_ARC_HALT_MASK                 0x00000001
#define MEI_DBG_FLAGS_ARC_HALT_RELEASED             0

/*
   Control definitions for the received msg buffer.
*/

/* marks a buffer for free */
#define MEI_RECV_BUF_CTRL_FREE                0x00000000

/* indicate a modem ACK message */
#define MEI_RECV_BUF_CTRL_MODEM_ACK_MSG       0x00000001

/* indicate a modem Autonomous message */
#define MEI_RECV_BUF_CTRL_MODEM_NFC_MSG       0x00000002

/* indicate a modem Autonomous ATM OAM message */
#define MEI_RECV_BUF_CTRL_MODEM_IP_MSG        0x00000010

/* indicate a modem Autonomous ATM OAM message */
#define MEI_RECV_BUF_CTRL_MODEM_ATMOAM_MSG    0x00000020

/* indicate a modem Autonomous EOC message */
#define MEI_RECV_BUF_CTRL_MODEM_EOC_MSG       0x00000040


/* indicate a modem Autonomous ETH/IP/UDP frames */
#define MEI_RECV_BUF_CTRL_MODEM_ETH_FRAME     0x00001000

/* indicate a modem Autonomous ATM OAM cell block */
#define MEI_RECV_BUF_CTRL_MODEM_ATMOAM_CELL   0x00002000

/* indicate a modem Autonomous EOC frame */
#define MEI_RECV_BUF_CTRL_MODEM_EOC_FRAME     0x00004000

/* indicate a driver autonomous message */
#define MEI_RECV_BUF_CTRL_DRIVER_MSG          0x00010000

/* marks a buffer for processing */
#define MEI_RECV_BUF_CTRL_LOCKED              0x80000000

/* marks a invalid/corrupted modem message */
#define MEI_RECV_BUF_CTRL_MODEM_MSG_ERROR     0x0000FFFF


/* enable FSM State Set pre-action for ethernet OAM */
#define MEI_FSM_STATE_SET_PRE_ACT_ETHOAM_ENABLE  0x00000001
/* enable FSM State Set pre-action for ATM OAM */
#define MEI_FSM_STATE_SET_PRE_ACT_ATMOAM_ENABLE  0x00000002
/* enable FSM State Set pre-action for Clear EOC */
#define MEI_FSM_STATE_SET_PRE_ACT_CEOC_ENABLE    0x00000004

/* enable FSM State Set pre-action for Clear EOC */
#define MEI_FSM_STATE_SET_PRE_ACT_ALL      (  MEI_FSM_STATE_SET_PRE_ACT_ETHOAM_ENABLE \
                                              | MEI_FSM_STATE_SET_PRE_ACT_ATMOAM_ENABLE \
                                              | MEI_FSM_STATE_SET_PRE_ACT_ATMOAM_ENABLE )

/* ==========================================================================
   Global macros - swap big/little endianes
   ========================================================================== */

#if (!defined(MEI_DRV_OS_BYTE_ORDER) || !defined(MEI_DRV_OS_BIG_ENDIAN) || !defined(MEI_DRV_OS_LITTLE_ENDIAN))
#  error "Missing endianess defines"
#endif

#if (MEI_DRV_OS_BYTE_ORDER == MEI_DRV_OS_BIG_ENDIAN)

/** Swap the hi/lo 16 bit word (DMA access width) */
#define SWAP32_DMA_WIDTH_ORDER(x)  ( (((x) & 0xFFFF0000) >> 16)  \
                                   | (((x) & 0x0000FFFF) << 16) )

/** Swap 32 bit */
#define SWAP32_BYTE_ORDER(x)  ( (((x) & 0xFF000000) >> 24) \
                              | (((x) & 0x00FF0000) >> 8)  \
                              | (((x) & 0x0000FF00) << 8)  \
                              | (((x) & 0x000000FF) << 24) )

/** Swap 16 bit */
#define SWAP16_BYTE_ORDER(x)  ( (((x) & 0xFF00) >> 8) \
                              | (((x) & 0x00FF) << 8) )


/* swap a value to little endian */
#define MEI_SWAP_TO_LITTLE_S(xs)  ((((xs) & 0x00FF) << 8) | \
                                   (((xs) & 0xFF00) >> 8) )

#define MEI_SWAP_TO_LITTLE_L(xl)  ((((xl) & 0x000000ff) << 24) | \
                                   (((xl) & 0x0000ff00) <<  8) | \
                                   (((xl) & 0x00ff0000) >>  8) | \
                                   (((xl) & 0xff000000) >> 24) )

/* swap a value to big endian */
#define MEI_SWAP_TO_BIG_S
#define MEI_SWAP_TO_BIG_L

#define MEI_LO_WORD_GET(x) (((x) << 16) & 0xFFFF0000)
#define MEI_HI_WORD_GET(x) ( (x)        & 0x0000FFFF)

#define MEI_MSW_WORD_GET(x) ( (x) & 0x0000FFFF)
#define MEI_LSW_WORD_GET(x) (((x) & 0xFFFF0000) >> 16)

#else

/** Swap the hi/lo 16 bit word (DMA access width) */
#define SWAP32_DMA_WIDTH_ORDER
/** Swap 32 bit */
#define SWAP32_BYTE_ORDER
/** Swap 16 bit */
#define SWAP16_BYTE_ORDER

/* swap a value to little endian */
#define MEI_SWAP_TO_LITTLE_S
#define MEI_SWAP_TO_LITTLE_L

/* swap a value to big endian */
#define MEI_SWAP_TO_BIG_S(xs)  ((((xs) & 0x00FF) << 8) | \
                                (((xs) & 0xFF00) >> 8) )

#define MEI_SWAP_TO_BIG_L(xl)  ((((xl) & 0x000000ff) << 24) | \
                                (((xl) & 0x0000ff00) <<  8) | \
                                (((xl) & 0x00ff0000) >>  8) | \
                                (((xl) & 0xff000000) >> 24) )

#define MEI_LO_WORD_GET(x) ( (x)        & 0x0000FFFF)
#define MEI_HI_WORD_GET(x) (((x) << 16) & 0xFFFF0000)

#define MEI_MSW_WORD_GET(x) (((x) & 0xFFFF0000) >> 16)
#define MEI_LSW_WORD_GET(x) ( (x) & 0x0000FFFF)

#endif      /* #if (MEI_DRV_OS_BYTE_ORDER == MEI_DRV_OS_BIG_ENDIAN) */


/* ==========================================================================
   Global macros - lock handling
   ========================================================================== */

/**
   Get unique access to the driver control struct
*/
#define MEI_DRV_GET_UNIQUE_DRIVER_ACCESS(pMeiDev) \
               do { \
                  MEI_DRVOS_SemaphoreLock(&pMeiDev->pDriverAccessLock); \
               } while(0)


/**
   Release unique access to the driver control struct
*/
#define MEI_DRV_RELEASE_UNIQUE_DRIVER_ACCESS(pMeiDev) \
               do { \
                  MEI_DRVOS_SemaphoreUnlock(&pMeiDev->pDriverAccessLock); \
               } while(0)



/**
   Get unique access to the device
*/
#define MEI_DRV_GET_UNIQUE_DEVICE_ACCESS(pMeiDev) \
               do { \
                  MEI_DRVOS_SemaphoreLock(&pMeiDev->meiDrvCntrl.pMeiDevCntrl->pDevMeiAccessLock); \
               } while(0)


/**
   Release unique access to the device
*/
#define MEI_DRV_RELEASE_UNIQUE_DEVICE_ACCESS(pMeiDev) \
               do { \
                  MEI_DRVOS_SemaphoreUnlock(&pMeiDev->meiDrvCntrl.pMeiDevCntrl->pDevMeiAccessLock); \
               } while(0)


/**
   Get unique access to the device mailbox
*/
#define MEI_DRV_GET_UNIQUE_MAILBOX_ACCESS(pMeiDev) \
               do { \
                  MEI_DisableDeviceInt(pMeiDev); \
                  MEI_DRVOS_SemaphoreLock(&pMeiDev->meiDrvCntrl.pMeiDevCntrl->pDevMeiAccessLock); \
               } while(0)

/**
   Release unique access to the device mailbox
*/
#define MEI_DRV_RELEASE_UNIQUE_MAILBOX_ACCESS(pMeiDev) \
               do { \
                  MEI_DRVOS_SemaphoreUnlock(&pMeiDev->meiDrvCntrl.pMeiDevCntrl->pDevMeiAccessLock); \
                  MEI_EnableDeviceInt(pMeiDev); \
               } while(0)


/**
   Get unique access to the device
*/
#define MEI_DRV_GET_UNIQUE_ACCESS(pMeiDev) \
               do { \
                  MEI_DRVOS_SemaphoreLock(&pMeiDev->pDriverAccessLock); \
                  MEI_DisableDeviceInt(pMeiDev); \
                  MEI_DRVOS_SemaphoreLock(&pMeiDev->meiDrvCntrl.pMeiDevCntrl->pDevMeiAccessLock); \
               } while(0)


/**
   Release unique access to the device
*/
#define MEI_DRV_RELEASE_UNIQUE_ACCESS(pMeiDev) \
               do { \
                  MEI_DRVOS_SemaphoreUnlock(&pMeiDev->meiDrvCntrl.pMeiDevCntrl->pDevMeiAccessLock); \
                  MEI_EnableDeviceInt(pMeiDev); \
                  MEI_DRVOS_SemaphoreUnlock(&pMeiDev->pDriverAccessLock); \
               } while(0)


/**
   Get unique access to the callback control data
*/
#define MEI_DRV_GET_UNIQUE_CALLBACK_ACCESS(pMeiDev) \
               do { \
                  MEI_DRVOS_SemaphoreLock(&pMeiDev->pCallbackLock); \
               } while(0)

/**
   Release unique access to the callback control data
*/
#define MEI_DRV_RELEASE_UNIQUE_CALLBACK_ACCESS(pMeiDev) \
               do { \
                  MEI_DRVOS_SemaphoreUnlock(&pMeiDev->pCallbackLock); \
               } while(0)
/* ==========================================================================
   Global macros - control data access
   ========================================================================== */

/** extract the special extended code form the device number */
#define MEI_NUM_IS_CNTRL_DEV(device_num) (device_num & MEI_ENTITY_CNTRL_DEVICE)

/** get the entity number from the given channel */
#define MEI_GET_ENTITY_FROM_DEVNUM(device_num) (device_num / MEI_DFE_INSTANCE_PER_ENTITY)

/** get the relative channel num on the entity from the given channel */
#define MEI_GET_REL_CH_FROM_DEVNUM(device_num) (device_num % MEI_DFE_INSTANCE_PER_ENTITY)

/** get IF num on the entity from the given channel */
#define MEI_GET_IF_FROM_DEVNUM(device_num) (device_num % MEI_MAX_SUPPORTED_MEI_IF_PER_DEVICE)

/** platform device attr get from device tree */
#define MEI_DEVICE_CFG_VALUE_GET(attr) MEI_DevCfgData.attr

/** platform device attr get from device tree */
#define MEI_DEVICE_CFG_VALUE_SET(attr, value) MEI_DevCfgData.attr = (value);

/** check current platform case */
#define MEI_DEVICE_CFG_IS_PLATFORM(value) \
                (IFX_boolean_t)(MEI_DEVICE_CFG_VALUE_GET(platform) & value)

/** get the entity number from a control device */
#define MEIX_GET_ENTITY_FROM_DEVNNUM(entity_num) (entity_num & MEI_ENTITY_CNTRL_DEV_MASK)

/** set the CPE device number (dsl line num) to the MEI device struct */
#define MEI_DRV_LINENUM_SET(pMeiDev, setDevNum) \
                        (pMeiDev)->meiDrvCntrl.dslLineNum = (IFX_uint8_t)setDevNum

/** get the CPE device number (dsl line num) out form the MEI device struct */
#define MEI_DRV_LINENUM_GET(pMeiDev) \
              (pMeiDev)->meiDrvCntrl.dslLineNum

/** get the MEI device number (dsl line num) out form the MEI device dyn struct */
#define MEI_DRV_DYN_LINENUM_GET(pMeiDynCntrl) \
                        (pMeiDynCntrl)->pMeiDev->meiDrvCntrl.dslLineNum

/** get the MEI device MEI physical address out form the MEI device struct */
#define MEI_DRV_MEI_PHY_ADDR_GET(pMeiDev) \
                        (pMeiDev)->meiDrvCntrl.pMeiIfCntrl->phyBaseAddr

/** set the MEI device MEI physical address to the MEI device struct */
#define MEI_DRV_MEI_PHY_ADDR_SET(pMeiDev, newMeiPhyAddr) \
                        (pMeiDev)->meiDrvCntrl.pMeiIfCntrl->phyBaseAddr = (IFX_ulong_t)newMeiPhyAddr;

/** get the MEI device MEI virtual address out form the MEI device struct */
#define MEI_DRV_MEI_VIRT_ADDR_GET(pMeiDev) \
                        (pMeiDev)->meiDrvCntrl.pMeiIfCntrl->pVirtMeiRegIf

/** set the MEI device MEI virtual address to the MEI device struct */
#define MEI_DRV_MEI_VIRT_ADDR_SET(pMeiDev, pNewMeiVirtAddr) \
                        (pMeiDev)->meiDrvCntrl.pMeiIfCntrl->pVirtMeiRegIf = pNewMeiVirtAddr;

/** get the MEI physical address at PCIe bus */
#define MEI_DRV_PCIE_PHY_MEMBASE_GET(pMeiDrvCntrl) \
                                    (pMeiDrvCntrl)->MEI_pcie_phy_membase

/** get the MEI virtual address at PCIe bus */
#define MEI_DRV_PCIE_VIRT_MEMBASE_GET(pMeiDrvCntrl) \
                                    (pMeiDrvCntrl)->MEI_pcie_virt_membase

/** get the MEI irq at PCIe bus */
#define MEI_DRV_PCIE_IRQ_GET(pMeiDrvCntrl) \
                                    (pMeiDrvCntrl)->MEI_pcie_irq

/** get the PDBRAM physical address out form the MEI device struct */
#define MEI_DRV_PDBRAM_PHY_ADDR_GET(pMeiDev) \
                        (pMeiDev)->meiDrvCntrl.pMeiIfCntrl->phyPDBRAMaddr

/** set the PDBRAM physical address to the MEI device struct */
#define MEI_DRV_PDBRAM_PHY_ADDR_SET(pMeiDev, newPDBRAM_PhyAddr) \
                        (pMeiDev)->meiDrvCntrl.pMeiIfCntrl->phyPDBRAMaddr = (IFX_ulong_t)newPDBRAM_PhyAddr;

/** get the PDBRAM virtual address out form the MEI device struct */
#define MEI_DRV_PDBRAM_VIRT_ADDR_GET(pMeiDev) \
                        (pMeiDev)->meiDrvCntrl.pMeiIfCntrl->virtPDBRAMaddr

/** set the PDBRAM virtual address to the MEI device struct */
#define MEI_DRV_PDBRAM_VIRT_ADDR_SET(pMeiDev, newPDBRAM_VirtAddr) \
                        (pMeiDev)->meiDrvCntrl.pMeiIfCntrl->virtPDBRAMaddr = (IFX_ulong_t)newPDBRAM_VirtAddr;

/** get the MEI MEI interface state this interface */
#define MEI_DRV_MEI_IF_STATE_GET(pMeiDev) \
                        (pMeiDev)->meiDrvCntrl.pMeiIfCntrl->eMeiHwState

/** set the MEI MEI interface state this interface */
#define MEI_DRV_MEI_IF_STATE_SET(pMeiDev, new_meiif_state) \
                        (pMeiDev)->meiDrvCntrl.pMeiIfCntrl->eMeiHwState = new_meiif_state

/** set BAR register type */
#define MEI_BAR_TYPE_SET(pMeiDevm, barIdx, val) \
                        (pMeiDev)->fwDl.eBarType[barIdx] = val;

/** get BAR register type */
#define MEI_BAR_TYPE_GET(pMeiDevm, barIdx) \
                        (pMeiDev)->fwDl.eBarType[barIdx]

#if (MEI_PREDEF_DBG_BAR == 1)
/** set BAR register debug addr */
#define MEI_BAR_DBG_ADDR_SET(pMeiDevm, barIdx, val) \
                        (pMeiDev)->fwDl.dbgBarAddr[barIdx] = val;

/** get BAR register debug addr */
#define MEI_BAR_DBG_ADDR_GET(pMeiDevm, barIdx) \
                        (pMeiDev)->fwDl.dbgBarAddr[barIdx]
#endif /* (MEI_PREDEF_DBG_BAR == 1) */

/** set the MEI driver state for this channel */
#define MEI_TRACE_DRV_STATE_CHANGES   0

#if MEI_TRACE_DRV_STATE_CHANGES == 1
#define MEI_DRV_STATE_SET(pMeiDev, newstate) \
                        MEI_DrvStateSet_Trace( pMeiDev, newstate)
#else
#define MEI_DRV_STATE_SET(pMeiDev, newstate) (pMeiDev)->eDevDrvState = newstate
#endif

/** get the MEI driver state for this channel */
#define MEI_DRV_STATE_GET(pMeiDev) (pMeiDev)->eDevDrvState

/** set the MEI driver bootmode for this channel */
#define MEI_DRV_BOOTMODE_SET(pMeiDev, newBootMode) (pMeiDev)->eDrvBootMode = newBootMode

/** get the MEI driver bootmode for this channel */
#define MEI_DRV_BOOTMODE_GET(pMeiDev) (pMeiDev)->eDrvBootMode



#if MEI_TRACE_DRV_STATE_CHANGES == 1
#define MEI_DRV_MODEM_STATE_SET(pMeiDev, newstate) \
                        MEI_ModemStateSet_Trace( pMeiDev, newstate)
#else
#define MEI_DRV_MODEM_STATE_SET(pMeiDev, newstate) (pMeiDev)->modemData.modemState = newstate
#endif

/** get the modem FSM state for this channel */
#define MEI_DRV_MODEM_STATE_GET(pMeiDev) (pMeiDev)->modemData.modemState


/** set the MEI driver mailbox state for this channel */
#define MEI_TRACE_MB_STATE_CHANGES   0

/** Special test handling that tests whether the bootloader is corrctly
    startred.
    \note This handling is only required in case there is no MODEM_READY
          message received.The following steps are done to check if the
          bootloader has been started correctly
          - Modify the port mode control structure header by overwriting the
            signture1 with a dummy value (0xCAFE) before releasing the ARC from
            halt
          - Release the ARC from halt
          - If the bootloader is started it will overwrite the signature1
            with the default value of MEI_FW_IMAGE_SIGNATURE1 (0xB11D) again
          - Value of signature1 is checked after timeout waiting for MODEM_READY
          - In case of signature1 is still the dummy value bootloader has not
            been started otherwise it should be signature1 default value.
*/
#define MEI_DBG_CECK_BOOTLOADER_START   0

/** Special test and debug functionality that provides possibility to measure
    timings for code part executions and generate a statistic array out of
    it. ONLY supported for Linux! */
#if defined(LINUX) && (MEI_SUPPORT_DSM == 1)
#define MEI_DBG_DSM_PROFILING 0
#else
#undef MEI_DBG_DSM_PROFILING
#define MEI_DBG_DSM_PROFILING 0
#endif

#if MEI_TRACE_MB_STATE_CHANGES == 1
#define MEI_DRV_MAILBOX_STATE_SET(pMeiDev, new_mb_state) \
                        MEI_DrvMailBoxStateSet_Trace( pMeiDev, new_mb_state)
#else
#define MEI_DRV_MAILBOX_STATE_SET(pMeiDev, new_mb_state) (pMeiDev)->modemData.mBoxState = new_mb_state
#endif

/** get the MEI driver mailbox state for this channel */
#define MEI_DRV_MAILBOX_STATE_GET(pMeiDev) (pMeiDev)->modemData.mBoxState


/** set the MEI driver mailbox buffer state for this open instance */
#define MEI_DRV_DYN_MBBUF_STATE_SET(pMeiDynCmdData, new_mbbuf_state) (pMeiDynCmdData)->wrState = new_mbbuf_state

/** get the MEI driver mailbox buffer state for this open instance */
#define MEI_DRV_DYN_MBBUF_STATE_GET(pMeiDynCmdData) (pMeiDynCmdData)->wrState


/** get the current message index (per MEI) */
#define MEI_GET_MSG_INDEX(pMeiDev) (pMeiDev)->modemData.msgIndex

/** Increment the current message index (per MEI) */
#define MEI_INC_MSG_INDEX(pMeiDev) (pMeiDev)->modemData.msgIndex++


/** get the current timeout counter */
#define MEI_GET_TIMEOUT_CNT(pMeiDev) (pMeiDev)->timeoutCount

/** Set the current timeout counter. */
#define MEI_SET_TIMEOUT_CNT(pMeiDev, newVal) (pMeiDev)->timeoutCount = (IFX_uint32_t)(newVal)

/** Decrement the current timeout counter. */
#define MEI_DEC_TIMEOUT_CNT(pMeiDev) \
            (!MEI_BlockTimeout) ? (pMeiDev)->timeoutCount-- : (pMeiDev)->timeoutCount

/** get the current timeout counter per instance. */
#define MEI_GET_DYN_TIMEOUT_CNT(pDynWrData) (pDynWrData)->timeoutCount

/** Set the current timeout counter per instance. */
#define MEI_SET_DYN_TIMEOUT_CNT(pDynWrData, newVal) (pDynWrData)->timeoutCount = (IFX_uint32_t)(newVal)

/** Decrement the current timeout counter per instance. */
#define MEI_DEC_DYN_TIMEOUT_CNT(pDynWrData) \
            (!MEI_BlockTimeout) ? (pDynWrData)->timeoutCount-- : (pDynWrData)->timeoutCount


/* ==========================================================================
   Global macros - statistic data access
   ========================================================================== */
#if (MEI_SUPPORT_STATISTICS == 1)

#define MEI_IF_STAT_INC_GP1_INT_COUNT(p_dev_struct) \
                              (p_dev_struct)->statistics.dfeGp1IntCount++
#define MEI_IF_STAT_INC_MSGAV_INT_COUNT(p_dev_struct) \
                              (p_dev_struct)->statistics.dfeMsgAvIntCount++
#define MEI_IF_STAT_INC_CODESWAP_COUNT(p_dev_struct) \
                              (p_dev_struct)->statistics.dfeCodeSwapCount++

#define MEI_IF_STAT_INC_RECV_MSG_COUNT(p_dev_struct) \
                              (p_dev_struct)->statistics.recvMsgCount++
#define MEI_IF_STAT_INC_RECV_MSG_ERR_COUNT(p_dev_struct) \
                              (p_dev_struct)->statistics.recvMsgErrCount++
#define MEI_IF_STAT_INC_RECV_MSG_DISCARD_COUNT(p_dev_struct) \
                              (p_dev_struct)->statistics.recvMsgDiscardCount++

#define MEI_IF_STAT_INC_RECV_ACK_COUNT(p_dev_struct) \
                              (p_dev_struct)->statistics.recvAckCount++

#define MEI_IF_STAT_INC_RECV_NFC_COUNT(p_dev_struct) \
                              (p_dev_struct)->statistics.recvNfcCount++
#define MEI_IF_STAT_INC_RECV_NFC_DISCARD_COUNT(p_dev_struct) \
                              (p_dev_struct)->statistics.recvNfcDiscardCount++
#define MEI_IF_STAT_ADD_RECV_NFC_DISCARD_COUNT(p_dev_struct, addCnt) \
                              (p_dev_struct)->statistics.recvNfcDiscardCount += addCnt
#define MEI_IF_STAT_INC_RECV_NFC_DIST_COUNT(p_dev_struct) \
                              (p_dev_struct)->statistics.recvNfcDistCount++
#define MEI_IF_STAT_ADD_RECV_NFC_DIST_COUNT(p_dev_struct, addCnt) \
                              (p_dev_struct)->statistics.recvNfcDistCount += addCnt
#define MEI_IF_STAT_INC_RECV_NFC_DIST_DISCARD_COUNT(p_dev_struct) \
                              (p_dev_struct)->statistics.recvNfcDistDiscardCount++
#define MEI_IF_STAT_ADD_RECV_NFC_DIST_DISCARD_COUNT(p_dev_struct, addCnt) \
                              (p_dev_struct)->statistics.recvNfcDistDiscardCount += addCnt

#define MEI_IF_STAT_INC_RECV_NFC_UNKNOWN_DISCARD_COUNT(p_dev_struct) \
                              (p_dev_struct)->statistics.recvNfcUnknownDiscardCount++


#define MEI_IF_STAT_INC_SEND_MSG_COUNT(p_dev_struct) \
                              (p_dev_struct)->statistics.sendMsgCount++
#define MEI_IF_STAT_INC_ERROR_COUNT(p_dev_struct) \
                              (p_dev_struct)->statistics.errorCount++

#define MEI_IF_STAT_INC_FWDL_COUNT_INC(p_dev_struct) \
                              (p_dev_struct)->statistics.fwDlCount++

#define MEI_IF_STAT_INC_FWDL_COUNT_GET(p_dev_struct) \
                              (p_dev_struct)->statistics.fwDlCount

#define MEI_IF_STAT_INC_FWDL_ERR_COUNT(p_dev_struct) \
                              (p_dev_struct)->statistics.fwDlErrCount++

#define MEI_IF_STAT_INC_FWDL_OPT_COUNT_INC(p_dev_struct) \
                              (p_dev_struct)->statistics.fwDlOptSuccessCount++

#define MEI_IF_STAT_INC_FWDL_OPT_ERR_COUNT(p_dev_struct) \
                              (p_dev_struct)->statistics.fwDlOptFailedCount++

#define MEI_IF_STAT_INC_SWRST_COUNT(p_dev_struct) \
                              (p_dev_struct)->statistics.drvSwRstCount++

#define MEI_IF_STAT_INC_HWRST_COUNT(p_dev_struct) \
                              (p_dev_struct)->statistics.meiHwRstCount++

#else    /* #if (MEI_SUPPORT_STATISTICS == 1) */

#define MEI_IF_STAT_INC_GP1_INT_COUNT(p_dev_struct)
#define MEI_IF_STAT_INC_MSGAV_INT_COUNT(p_dev_struct)
#define MEI_IF_STAT_INC_CODESWAP_COUNT(p_dev_struct)

#define MEI_IF_STAT_INC_RECV_MSG_COUNT(p_dev_struct)
#define MEI_IF_STAT_INC_RECV_MSG_DISCARD_COUNT(p_dev_struct)
#define MEI_IF_STAT_INC_RECV_MSG_ERR_COUNT(p_dev_struct)

#define MEI_IF_STAT_INC_RECV_ACK_COUNT(p_dev_struct)
#define MEI_IF_STAT_INC_RECV_NFC_COUNT(p_dev_struct)
#define MEI_IF_STAT_INC_RECV_NFC_DISCARD_COUNT(p_dev_struct)
#define MEI_IF_STAT_ADD_RECV_NFC_DISCARD_COUNT(p_dev_struct, addCnt)
#define MEI_IF_STAT_INC_RECV_NFC_DIST_COUNT(p_dev_struct)
#define MEI_IF_STAT_ADD_RECV_NFC_DIST_COUNT(p_dev_struct, addCnt)
#define MEI_IF_STAT_INC_RECV_NFC_DIST_DISCARD_COUNT(p_dev_struct)
#define MEI_IF_STAT_ADD_RECV_NFC_DIST_DISCARD_COUNT(p_dev_struct, addCnt)

#define MEI_IF_STAT_INC_RECV_NFC_UNKNOWN_DISCARD_COUNT(p_dev_struct)

#define MEI_IF_STAT_INC_SEND_MSG_COUNT(p_dev_struct)
#define MEI_IF_STAT_INC_ERROR_COUNT(p_dev_struct)

#define MEI_IF_STAT_INC_FWDL_COUNT(p_dev_struct)
#define MEI_IF_STAT_INC_FWDL_ERR_COUNT(p_dev_struct)

#define MEI_IF_STAT_INC_SWRST_COUNT(p_dev_struct)
#define MEI_IF_STAT_INC_HWRST_COUNT(p_dev_struct)

#endif   /* #if (MEI_SUPPORT_STATISTICS == 1) */


/* ==========================================================================
   Global macros - handling timer tracer
   ========================================================================== */
#if (MEI_SUPPORT_TIME_TRACE == 1)

#define MEI_GET_TICK_MS_TIME_TRACE(tick_ms)  tick_ms = MEI_DRVOS_GetElapsedTime_ms(0)

#define MEI_TIME_TRACE_GET_TICK_MS(tick_start, tick_end) \
         ( ((tick_end) > (tick_start)) ? \
            (IFX_uint32_t)((tick_end  ) - (tick_start)) : \
            (IFX_uint32_t)((tick_start) - (tick_end  )) )

#define MEI_TIME_TRACE_CHECK_WAIT_SEND_MIN_MAX(timeStatistic, tick_ms, timeout_ms) \
         if ( (tick_ms) < timeStatistic.waitSendMin_ms) \
            timeStatistic.waitSendMin_ms = (tick_ms); \
         if (((tick_ms) > timeStatistic.waitSendMax_ms) && ((tick_ms) < (IFX_uint32_t)(timeout_ms)) )  \
            timeStatistic.waitSendMax_ms = (tick_ms)

#define MEI_TIME_TRACE_CHECK_WAIT_ACK_MIN_MAX(timeStatistic, tick_ms, timeout_ms) \
         if ( (tick_ms) < timeStatistic.waitAckMin_ms) \
            timeStatistic.waitAckMin_ms = (tick_ms); \
         if (((tick_ms) > timeStatistic.waitAckMax_ms) && ((tick_ms) < (IFX_uint32_t)(timeout_ms)) )  \
            timeStatistic.waitAckMax_ms = (tick_ms)

#if ((MEI_SUPPORT_ROM_CODE == 1) && (MEI_SUPPORT_DL_DMA_CS == 1))
#define MEI_TIME_TRACE_CHECK_PROCESS_CS_MAX(timeStatistic, tick_ms) \
         if ((tick_ms) > timeStatistic.processCsMax_ms) \
            timeStatistic.processCsMax_ms = (tick_ms)
#endif

#else

#define MEI_GET_TICK_MS_TIME_TRACE(tick_ms)
#define MEI_TIME_TRACE_GET_TICK_MS(tick_start, tick_end)
#define MEI_TIME_TRACE_CHECK_WAIT_SEND_MIN_MAX(timeStatistic, tick_ms, timeout_ms)
#define MEI_TIME_TRACE_CHECK_WAIT_ACK_MIN_MAX(timeStatistic, tick_ms, timeout_ms)

#if ((MEI_SUPPORT_ROM_CODE == 1) && (MEI_SUPPORT_DL_DMA_CS == 1))
#define MEI_TIME_TRACE_CHECK_PROCESS_CS_MAX(timeStatistic, tick_ms)
#endif

#endif      /* MEI_SUPPORT_TIME_TRACE */


/* ==========================================================================
   Global defs - enumeration types
   ========================================================================== */

/**
   Defines Access modes - IRQ, passive poll, active poll.
*/
typedef enum
{
   /** use IRQ for this line */
   e_MEI_DEV_ACCESS_MODE_IRQ   = 0,
   /** use passive polling for this line */
   e_MEI_DEV_ACCESS_MODE_PASSIV_POLL,
   /** use active polling for this line */
   e_MEI_DEV_ACCESS_MODE_ACTIV_POLL

} MEI_DEV_ACCESS_MODE_E;


/**
   Defines the boot modes.
*/
typedef enum
{
   /** boot mode: automatical by modem */
   e_MEI_DRV_BOOT_MODE_AUTO,
   /** boot mode: download ROM handler - data transfer via DMA and CodeSwap */
   e_MEI_DRV_BOOT_MODE_7,
   /** boot mode: invalid */
   e_MEI_DRV_BOOT_MODE_INVALID
} MEI_DRV_BOOT_MODES_E;


/**
   Defines the mailbox status.
*/
typedef enum
{
   /** no ACK pending */
   e_MEI_MB_FREE,
   /** wait for outstanding ACK */
   e_MEI_MB_PENDING_ACK_1
}  MEI_MB_STATE_E;


/**
   Defines the dynamic user buffer status.
*/
typedef enum
{
   /** no ACK pending */
   e_MEI_MB_BUF_FREE,
   /** wait for outstanding ACK */
   e_MEI_MB_BUF_ACK_PENDING,
   /** ACK available */
   e_MEI_MB_BUF_ACK_AVAIL,
   /** timeout occurred */
   e_MEI_MB_BUF_TIMEOUT,
   /** reset of the MEI driver structure */
   e_MEI_MB_BUF_RESET_DFE

}  MEI_MB_BUF_STATE_E;

/**
   Defines possible devices
   \note This value is used as a bit field!
*/
typedef enum
{
   /** platform unknown or undefined */
   e_MEI_DEV_PLATFORM_CONFIG_UNKNOWN   = 0x0,
   /** VR9 (xrx200) */
   e_MEI_DEV_PLATFORM_CONFIG_VR9       = 0x1,
   /** VR10 (xrx300, smartphy) */
   e_MEI_DEV_PLATFORM_CONFIG_VR10      = 0x2,
   /** VR10_320 (xrx320) */
   e_MEI_DEV_PLATFORM_CONFIG_VR10_320  = 0x4,
   /** VR11 (xrx500 family) */
   e_MEI_DEV_PLATFORM_CONFIG_VR11      = 0x8
} MEI_DEV_PLATFORM_CONFIG_E;

/* ==========================================================================
   Global defs - data and struct types
   ========================================================================== */

/**
   MEI dynamic buffer - user for internal buffer handling.
*/
typedef struct MEI_dyn_buffer_s
{
   IFX_uint8_t    *pBuffer;
   IFX_uint32_t   bufSize_byte;
} MEI_DYN_BUFFER_T;


#if (MEI_SUPPORT_TIME_TRACE == 1)
/**
   MEI data struct to handle timing statistics.
*/
typedef struct MEI_time_stat_s
{
   /** min wait time: Wait for Send msg */
   IFX_uint32_t   waitSendMin_ms;
   /** max wait time: Wait for Send msg */
   IFX_uint32_t   waitSendMax_ms;
   /** min wait time: Wait for ACK */
   IFX_uint32_t   waitAckMin_ms;
   /** max wait time: Wait for ACK */
   IFX_uint32_t   waitAckMax_ms;

#if ((MEI_SUPPORT_ROM_CODE == 1) && (MEI_SUPPORT_DL_DMA_CS == 1))
   /** max time: BM7 CodeSwap processing time */
   IFX_uint32_t   processCsMax_ms;
#endif

} MEI_TIME_STAT_T;
#endif


/**
   Defines a message buffer struct for receive messages.
*/
typedef struct MEI_recv_buff_s
{
   /** msg control */
   IFX_uint32_t         bufCtrl;
   /** length of recvied msg */
   IFX_uint32_t         msgLen;
   /** time tick when the message has been received */
   IFX_uint32_t         recvTime;

   /** MEI msg receive buffer */
   MEI_DYN_BUFFER_T   recvDataBuf_s;
} MEI_RECV_BUF_T;

/**
   MEI device Dynamic data struct to handle write operation.
   - send a messages and receive the correspinding ACK
*/
typedef struct MEI_dyn_wr_data_s
{
   MEI_MB_BUF_STATE_E wrState;
   IFX_uint32_t         timeoutCount;

   /** send buffer (send) */
   MEI_DYN_BUFFER_T   cmdWrBuf;

   /** receive ACK messages */
   MEI_RECV_BUF_T     cmdAckCntrl;

#if (MEI_SUPPORT_TIME_TRACE == 1)
   IFX_uint32_t         ackWaitStart_ms;
   IFX_uint32_t         ackWaitEnd_ms;
#endif
} MEI_DYN_CMD_DATA_T;


/**
   MEI device Dynamic data struct to handle receive operation.
   - receive a notification messages from the device.
*/
typedef struct MEI_dyn_nfc_data_s   MEI_DYN_NFC_DATA_T;

/** MEI device Dynamic NFC receive Data Block */
struct MEI_dyn_nfc_data_s
{
   /** receive NFC messages */
   MEI_RECV_BUF_T *pRecvDataCntrl;
   IFX_uint8_t       numOfBuf;
   IFX_uint8_t       rdIdxRd;
   IFX_uint8_t       rdIdxWr;


   /* control the supported events
      - modem  autonomous messages
        --> NFC, EVT, ALM, DBG
      - driver autonomous messages
        --> EVT, ALARM, DBG
   */
   IFX_uint32_t      msgProcessCtrl;

   /* List of dynamic NFC receive data blocks (per open, per MEI device) */
   MEI_DYN_NFC_DATA_T *pNext;
   MEI_DYN_NFC_DATA_T *pPrev;

#if (MEI_EXPORT_INTERNAL_API == 1)
   MEI_InternalMsgRecvCallBack pCallBackFunc;
   IFX_void_t                    *pCallBackData;
#endif

};

typedef struct MEI_TcLayerRequestCallback_s
{
   IFX_int32_t (*func)(void*);
   MEI_DEV_T *pMeiDev;
   IFX_int8_t line;
   IFX_int32_t nTcLayer;
} MEI_TcLayerRequestCallback_t;

/** General callback structure */
typedef union MEI_Callback_u {
    MEI_TcLayerRequestCallback_t tcLayerRequest;
} MEI_CALLBACK_T;

enum MEI_CallbackType
{
    MEI_CALLBACK_TYPE_NONE = 0,
    MEI_CALLBACK_TYPE_TC_LAYER_REQUEST = 1,
};

#if (MEI_SUPPORT_DFE_GPA_ACCESS == 1)
/* For General Purpose Access - message buffer */
typedef struct MEI_gpa_buf_s
{
   IFX_uint32_t   MessageID;
   IFX_uint32_t   GpaBuffer[3];

   IFX_uint32_t   ackWaitStart_ms;
   IFX_uint32_t   ackWaitEnd_ms;
} MEI_GPA_BUF_T;
#endif

/**
   Modem specific data.
*/
typedef struct MEI_modem_data_s
{
   /** chip ID of the current MEI device */
   IFX_uint8_t             chipId;
   /** bootmode of the current MEI device */
   IFX_uint8_t             devBootMode;
   /** detected HW version of the current MEI device */
   IFX_uint8_t             hwVersion;

   /** PLL offset config */
   IFX_int32_t             nPllOffset;

   /** mailbox description of the corresponding MEI devcie */
   MEI_MAILBOX_DESCR_T   mBoxDescr;
   /** next used message index */
   IFX_uint32_t            msgIndex;
   /** status of the current MEI mailbox */
   MEI_MB_STATE_E        mBoxState;
   /** current modem FSM state */
   IFX_uint32_t            modemState;
} MEI_MODEM_DATA_T;

/**
   Global MEI interface related data.
*/
struct MEI_dev_s
{
#ifdef VXWORKS
   /** driver specific parameter */
   DEV_HDR                 DevHdr;

   /** to see first open */
   IFX_boolean_t           bNotFirst;
#endif

   /* =======================================================================
      Configuration and device setup data
   */

   /** interrupt, passive polling, active polling */
   MEI_DEV_ACCESS_MODE_E    eModePoll;

#if (MEI_SUPPORT_DRV_LOOPS == 1)
   /** loop within the MEI driver (TRUE: loop on, FALSE: loop off) */
   IFX_boolean_t              bDrvLoop;
#endif

   /** control struct for the mei access */
   MEI_MEI_DRV_CNTRL_T      meiDrvCntrl;

   /* =======================================================================
      Dynamic driver data
   */
   /** current driver status */
   MEI_DRV_STATE_E          eDevDrvState;

   /** current used driver bootmode */
   MEI_DRV_BOOT_MODES_E     eDrvBootMode;

   /** open count open */
   IFX_uint8_t                openCount;

   /** used interrupt mask  */
   IFX_uint32_t               intMask;

   /** signals a previous User Reset action */
   IFX_boolean_t              bUsrRst;

#if (MEI_SUPPORT_STATISTICS == 1)
   /* statistic data */
   IOCTL_MEI_statistic_t    statistics;
#endif

#if (MEI_SUPPORT_TIME_TRACE == 1)
   MEI_TIME_STAT_T          timeStat;
#endif

#if (MEI_SUPPORT_DSM == 1)
#if MEI_SUPPORT_DEVICE_VR11 != 1
   /** dsm vectoring data */
   MEI_DSM_VECTOR_DYN_ERB   meiERBbuf;
#endif
   #if (MEI_DBG_DSM_PROFILING == 1)
#error MEI_DBG_DSM_PROFILING included
   /** Array to store profiling timing data for event handling. */
   IFX_uint32_t meiDbgProfilingData[32];
   /** Bool variable to switch on/off the ERB data reset handling (for FW
       debug purpose) */
   IFX_boolean_t bErbReset;
   #endif

   /** Successful firmware downloads including full G.Vector (Annex N)
       support */
   IFX_uint32_t meiFwDlCount;

   /** dsm vectoring statistic */
   IOCTL_MEI_dsmStatistics_t meiDsmStatistic;

   /** dsm vectoring config */
   IOCTL_MEI_dsmConfig_t meiDsmConfig;

   /** Initial initialization flag. To set initial default confifuration for
       meiDsmConfig.eVectorControl according to first FW that will be
       downloaded. */
   IFX_boolean_t         bDsmConfigInit;

   /** MAC address config */
   IOCTL_MEI_MacConfig_t meiMacConfig;

   /** dsm vectoring support within firmware */
   MEI_DSM_VECTOR_FW_SUPPORT_MODE_E nFwVectorSupport;

#if (MEI_SUPPORT_DEVICE_VR11 == 1)
   IFX_boolean_t bIsSetErb;
   IFX_uint32_t bar14;
   IFX_uint32_t bar17;
   IFX_uint32_t barSafeAddr;
#endif
#endif /* (MEI_SUPPORT_DSM == 1) */
   IOCTL_MEI_firmwareFeatures_t firmwareFeatures;

   /* =======================================================================
      Modem data
   */

   /** contains modem status and dynamic informations */
   MEI_MODEM_DATA_T      modemData;

   /* =======================================================================
      Message handling
   */
   /** handling message protocol: ACK received */
   IFX_boolean_t         bAckNeedWakeUp;
   MEI_DRVOS_event_t     eventMailboxRecv;
   IFX_uint32_t          timeoutCount;

   /** handling message protocol: points to the open instance waiting for an ACK */
   MEI_DYN_CMD_DATA_T    *pCurrDynCmd;

#if ( defined(MEI_DRVOS_HAVE_DRV_SELECT) && (MEI_DRVOS_HAVE_DRV_SELECT == 1) )
   /** support for select() */
   IFX_boolean_t         bNfcNeedWakeUp;
   MEI_DRVOS_selectQueue_t     selNfcWakeupList;
#endif

   /** list of all open instances which can receive NFC's, EVT's ALM's */
   MEI_DYN_NFC_DATA_T    *pRootNfcRecvFirst;
   MEI_DYN_NFC_DATA_T    *pRootNfcRecvLast;

#if (MEI_SUPPORT_DEBUG_STREAMS == 1)
   /** list of all open instances which can receive debug streams */
   MEI_DYN_DBG_STRM_DATA_T    *pRootDbgStrmRecvFirst;
   MEI_DYN_DBG_STRM_DATA_T    *pRootDbgStrmRecvLast;
#endif

   /* =======================================================================
      Protection
   */
   /** device read/write lock (semaphore) */
   MEI_DRVOS_sema_t        pDriverAccessLock;

   /* =======================================================================
      Addition addons and interfaces
   */

#if (MEI_SUPPORT_DFE_GPA_ACCESS == 1)
   /* General Access Buffer */
   MEI_GPA_BUF_T  gpaBuf;
#endif
   /** control data for the download */
   MEI_FW_DOWNLOAD_CNTRL_T fwDl;
#if (MEI_DRV_ATM_OAM_ENABLE == 1)
   MEI_ATMOAM_DEV_CNTRL_T   *pAtmOamDevCntrl;
#endif

#if (MEI_DRV_CLEAR_EOC_ENABLE == 1)
   MEI_CEOC_DEV_CNTRL_T     *pCEocDevCntrl;
#endif

   MEI_DRVOS_sema_t        pCallbackLock;
   IFX_boolean_t           bHandleCallback;
   enum MEI_CallbackType   callbackType;
   MEI_CALLBACK_T          callbackData;
};

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)) || (MEI_SUPPORT_DEVICE_VR10_320 == 1)
typedef struct MEI_devcfg_data_s
{
   IFX_uint32_t               nIrq;
   IFX_uint32_t               nBaseAddr;
   struct clk                 *clk;
   MEI_DEV_PLATFORM_CONFIG_E  platform;
   struct device              *dev;
   IFX_uint32_t              MaxDeviceNumber;
   IFX_uint32_t              LinesPerDevice;
   IFX_uint32_t              ChannelsPerLine;
   IFX_uint32_t              DfeChanDevices;

} MEI_DEVCFG_DATA_T;
#endif


/**
   Global xDSL chip related data
*/
typedef struct MEIx_cntrl_s MEIX_CNTRL_T;

struct MEIx_cntrl_s
{
#ifdef VXWORKS
   /** driver specific parameter */
   DEV_HDR           DevHdr;

   /** to see first open */
   IFX_boolean_t     bNotFirst;
#endif

   IFX_uint8_t       entityNum;

   /** number of interrupts per MEI device */
   IFX_uint32_t      meiIntCnt;
   /** IRQ num */
   IFX_uint32_t      IRQ_Num;
   /** number of interrupts per IRQ's */
   IFX_uint32_t      IRQ_Count;
   /** marks the base of the IRQ device list */
   IFX_uint32_t      IRQ_Base;
   /** Protection Counter
       If this counter becomes 0 there must be something wrong
       - decremented if an IRQ occurrs but no interrupt has been processed.
       - reset to the default value if an IRQ and a interrupt has been processed.
   */
   IFX_int32_t      IRQ_Protection;

   /** available MEI interfaces */
   MEI_MEI_DEV_CNTRL_T *pMeiDevCntrl[MEI_MAX_SUPPORTED_MEI_IF_PER_DEVICE];

   /** xDSL chip interface's control structure */
   MEI_DEV_T           *MeiDevice[MEI_MAX_SUPPORTED_DFE_INSTANCE_PER_ENTITY];

   /** List of xDSL chips - for shared interrupts */
   MEIX_CNTRL_T        *pNextMeiXCntrl;
};

/**
   Dynamic MEI related data to handle multiple open().
   These structure contains all necessary data to handle the driver
   access per open.
   - Global xDSL Chip struct
   - Global MEI device struct
   - Dynamic buffer per open()
*/
struct MEI_dyn_cntrl_s
{
   /** current open instance of this device */
   IFX_uint8_t         openInstance;
   /** points to the MEIx data struct */
   MEIX_CNTRL_T       *pDfeX;
   /** points to the MEI device data struct */
   MEI_DEV_T          *pMeiDev;
   /** points to the dynamic block for write msg (open specific) */
   MEI_DYN_CMD_DATA_T *pInstDynCmd;
   /** points to the dynamic block for receive msg (open specific) */
   MEI_DYN_NFC_DATA_T *pInstDynNfc;
   /** device send lock (semaphore)
       - protection against multiple use of the same file descriptor */
   MEI_DRVOS_sema_t    pInstanceRWlock;

#if (MEI_SUPPORT_DEBUG_STREAMS == 1)
   /** points to the dynamic block for receive msg (open specific) */
   MEI_DYN_DBG_STRM_DATA_T    *pInstDynDebugStream;
   /** protection of the control struct and FIFO */
   MEI_DRVOS_sema_t            dbgStreamLock;
#endif
};

/**
   Callback function for check the valid state of a given device
*/
typedef IFX_int32_t(*MEI_FCT_CHECK_DEV_STATE)(MEI_DEV_T *);


/* ==========================================================================
   Global Variable Definitions
   ========================================================================== */

/** devices */
extern MEIX_CNTRL_T* MEIX_Cntrl[MEI_MAX_SUPPORTED_DFEX_ENTITIES];

/** decrement counter to protect "empty" IRQ requests form the OS */
extern IFX_int32_t MEI_IrqProtectCount;

/** what string */
extern const char MEI_WHATVERSION[] ;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)) || (MEI_SUPPORT_DEVICE_VR10_320 == 1)
/** device tree data */
extern MEI_DEVCFG_DATA_T MEI_DevCfgData;
#endif

/* VRX-Driver: Common debug module - declare print level variable */
MEI_DRV_PRN_USR_MODULE_DECL(MEI_DRV);
MEI_DRV_PRN_INT_MODULE_DECL(MEI_DRV);

/* MEI CPE-Driver: fw message dump debug module - declare print level variable */
MEI_DRV_PRN_USR_MODULE_DECL(MEI_MSG_DUMP_API);
MEI_DRV_PRN_INT_MODULE_DECL(MEI_MSG_DUMP_API);

/* MEI Driver: notifications debug module - declare print level variable */
MEI_DRV_PRN_USR_MODULE_DECL(MEI_NOTIFICATIONS);
MEI_DRV_PRN_INT_MODULE_DECL(MEI_NOTIFICATIONS);

/** define the boot mode */
extern IFX_int32_t  MEI_BlockTimeout;
extern IFX_uint8_t  MEI_DefDownLoadChipId;
extern IFX_uint32_t MEI_MaxWaitForModemReady_ms;
extern IFX_int32_t  MEI_MaxWaitDfeResponce_ms;
extern IFX_uint32_t MEI_FsmStateSetMsgPreAction;
extern IFX_int32_t  MEI_DbgFlags;
#if (MEI_SUPPORT_DEBUG_LOGGER == 1)
extern IFX_int32_t  MEI_DbgLogger;
#endif /* (MEI_SUPPORT_DEBUG_LOGGER == 1) */

#if (MEI_SUPPORT_VDSL2_ADSL_SWAP == 1)
extern IFX_uint32_t MEI_fwModeSelect;
#endif

/* ==========================================================================
   Global Functions Definitions
   ========================================================================== */

extern IFX_int32_t MEI_VR1x_PcieEntitiesCheck(
                           IFX_uint8_t nEntityNum);

extern IFX_int32_t MEI_VR1x_PcieEntityInit(
                           MEI_MEI_DRV_CNTRL_T *pMeiDrvCntrl);

extern IFX_int32_t MEI_VR1x_PcieEntityFree(
                           IFX_uint8_t entityNum);

extern IFX_int32_t MEI_VR1x_InternalInitDevice(
                           MEI_DYN_CNTRL_T *pMeiDynCntrl);

extern IFX_int32_t MEI_CheckIoctlCmdInitState(
                                 MEI_DYN_CNTRL_T *pMeiDynCntrl,
                                 IFX_uint32_t      ioctlCmd );

extern IFX_int32_t MEI_CheckIoctlCmdSendState(
                                 MEI_DYN_CNTRL_T *pMeiDynCntrl,
                                 IFX_uint32_t      ioctlCmd );

extern IFX_uint32_t MEI_DefaultInitDevice(
                                 MEI_DEV_T *pMeiDev,
                                 IFX_uint32_t resetMode);

extern MEIX_CNTRL_T *MEI_DevXCntrlStructAlloc(
                                 IFX_uint8_t entityNum);

extern MEI_DEV_T *MEI_DevLineStructAlloc(
                                 IFX_uint8_t nLineNum);

extern IFX_int32_t MEI_DynCntrlStructAlloc(
                                 IFX_uint8_t nLineNum,
                                 MEI_DYN_CNTRL_T **ppMeiDynCntrl );

extern IFX_int32_t MEI_DevXCntrlStructFree(
                                 IFX_uint8_t entity);

extern IFX_int32_t MEI_DevLineStructFree(
                                 IFX_uint8_t nLineNum);

extern IFX_int32_t MEI_DynCntrlStructFree(
                                 MEI_DYN_CNTRL_T **ppMeiDynCntrl);

extern IFX_int32_t MEI_DevLineAlloc(
                                 IFX_int8_t nLineNum);

extern IFX_int32_t MEI_InstanceLineAlloc(
                                 IFX_int8_t        nLineNum,
                                 MEI_DYN_CNTRL_T **ppMeiDynCntrl);

extern IFX_int32_t MEI_DevLineClose(
                                 MEI_DYN_CNTRL_T *pMeiDynCntrl);

extern IFX_int32_t MEI_PowerUp(void);

extern IFX_int32_t MEI_PowerDown(void);

extern IFX_int32_t MEI_IoctlInitDevice(
                                 MEI_DYN_CNTRL_T     *pMeiDynCntrl,
                                 IOCTL_MEI_devInit_t *pArgInitDev);

extern IFX_int32_t MEI_IoctlDrvVersionGet(
                                 MEI_DYN_CNTRL_T        *pMeiDynCntrl,
                                 IOCTL_MEI_drvVersion_t *pArgDrvVersion_out,
                                 IFX_boolean_t            bInternCall);

extern IFX_int32_t MEI_IoctlDebugLevelSet(
                                 MEI_DYN_CNTRL_T       *pMeiDynCntrl,
                                 IOCTL_MEI_dbgLevel_t  *pArgDbgLevel);

extern IFX_void_t  MEI_IoctlRequestConfig(
                                 MEI_DYN_CNTRL_T    *pMeiDynCntrl,
                                 IOCTL_MEI_reqCfg_t *pArgDevCfg_out);

extern IFX_void_t  MEI_IoctlDrvInit(
                                 MEI_DYN_CNTRL_T     *pMeiDynCntrl,
                                 IOCTL_MEI_drvInit_t *pDrvCfg);

extern IFX_int32_t MEI_IoctlDevinfoGet(
                              MEI_DYN_CNTRL_T        *pMeiDynCntrl,
                              IOCTL_MEI_devinfo_t  *pArgDevinfo_out);

extern IFX_int32_t MEI_IoctlDevCfgFwModeSwap(
                                 MEI_DYN_CNTRL_T    *pMeiDynCntrl,
                                 IOCTL_MEI_fwMode_t *pArgFwMode);

#if (MEI_SUPPORT_STATISTICS == 1)
extern IFX_void_t MEI_IoctlRequestStat(
                                 MEI_DYN_CNTRL_T       *pMeiDynCntrl,
                                 IOCTL_MEI_statistic_t *pArgDevStat_out );
#endif

extern IFX_int32_t MEI_StartupDevice(
                                 MEI_DEV_T *pMeiDev);

extern IFX_int32_t MEI_GetChipInfo(
                                 MEI_DEV_T *pMeiDev);

extern IFX_int32_t MEI_MeiRegisterDetect(
                                 MEI_DEV_T *pMeiDev);

extern IFX_int32_t MEI_ResetDrvStruct(
                              MEI_DEV_T *pMeiDev,
                              IFX_int32_t rstSrc);

extern IFX_int32_t MEI_GPIntProcess(MEI_MeiRegVal_t processInt, MEI_DEV_T *pMeiDev);

extern IFX_int32_t MEIX_DevMaskCheck(
                                 IFX_uint32_t               *pVrxDevMask,
                                 MEI_FCT_CHECK_DEV_STATE  fctCheckDevState);


#if (MEI_SUPPORT_IRQ == 1)
extern MEIX_CNTRL_T * MEI_VrxXDevToIrqListAdd(
                                 IFX_uint8_t    devNum,
                                 IFX_uint32_t   irqNum,
                                 MEIX_CNTRL_T *pMeiXCntrl);

extern IFX_void_t MEI_VrxXDevFromIrqListClear(
                                 MEIX_CNTRL_T *pMeiXCntrl);


extern IFX_int32_t MEI_ProcessIntPerIrq(
                                 MEIX_CNTRL_T *pMeiXCntrlList);
#endif

/* called in poll mode for manually/periodically interrupt check */
extern IFX_int32_t MEI_DevPollAllIrq(
                                 MEI_DEV_ACCESS_MODE_E eAccessMode);

extern IFX_int32_t MEI_PollIntPerVrxLine(
                                 MEI_DEV_T             *pMeiDev,
                                 MEI_DEV_ACCESS_MODE_E eAccessMode);

extern IFX_void_t MEI_HandleCallback(
                                 MEI_DEV_T *pMeiDev);

extern IFX_int32_t MEI_TcRequest(
                                 void *data);

extern IFX_int32_t MEI_ProcessIntPerVrxLine(
                                 MEI_DEV_T *pMeiDev);

extern IFX_int32_t MEI_DisableDevsPerIrq(
                                 MEIX_CNTRL_T *pMeiXCntrlList);

extern IFX_int32_t MEI_DrvAndDevReset(
                                 MEI_DEV_T              *pMeiDev,
                                 IOCTL_MEI_resetMode_e  rstMode,
                                 IFX_uint32_t             rstSelMask,
                                 IFX_int32_t              rstSrc);

extern IFX_int32_t MEI_MsgSendPreAction(
                                 MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                                 IOCTL_MEI_messageSend_t  *pUserMsgs,
                                 IFX_boolean_t              bInternCall);

extern IFX_void_t MEI_DisableDeviceInt(
                                 MEI_DEV_T *pMeiDev);

extern IFX_void_t MEI_EnableDeviceInt(
                                 MEI_DEV_T *pMeiDev);

extern IFX_void_t MEI_MsgIntSet(MEI_DEV_T *pMeiDev);

#if (MEI_SUPPORT_PERIODIC_TASK == 1)

extern MEI_DRVOS_ThreadCtrl_t MEI_DrvCntrlThreadParams;

extern IFX_int32_t MEI_DrvCntrlThr(
                                 MEI_DRVOS_ThreadParams_t *pCntrlThrParams);

#endif   /* #if (MEI_SUPPORT_PERIODIC_TASK == 1) */

#if (MEI_SUPPORT_DSM == 1)
extern IFX_int32_t MEI_IoctlDsmStatisticGet(
                                 MEI_DYN_CNTRL_T           *pMeiDynCntrl,
                                 IOCTL_MEI_dsmStatistics_t *pDsmStatistic);

extern IFX_int32_t MEI_IoctlDsmConfigGet(
                                 MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                                 IOCTL_MEI_dsmConfig_t    *pDsmConfig);

extern IFX_int32_t MEI_IoctlDsmConfigSet(
                                 MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                                 IOCTL_MEI_dsmConfig_t    *pDsmConfig);

extern IFX_int32_t MEI_IoctlDsmStatusGet(
                                 MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                                 IOCTL_MEI_dsmStatus_t    *pDsmStatus);

extern IFX_int32_t MEI_IoctlMacConfigGet(
                                 MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                                 IOCTL_MEI_MacConfig_t    *pMacConfig);

extern IFX_int32_t MEI_IoctlMacConfigSet(
                                 MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                                 IOCTL_MEI_MacConfig_t    *pMacConfig);
#endif /* (MEI_SUPPORT_DSM == 1) */

extern IFX_int32_t MEI_IoctlPllOffsetConfigSet(
                                 MEI_DYN_CNTRL_T             *pMeiDynCntrl,
                                 IOCTL_MEI_pllOffsetConfig_t *pPllOffsetConfig);

extern IFX_int32_t MEI_IoctlPllOffsetConfigGet(
                                 MEI_DYN_CNTRL_T             *pMeiDynCntrl,
                                 IOCTL_MEI_pllOffsetConfig_t *pPllOffsetConfig);

extern IFX_int32_t MEI_PLL_ConfigInit(MEI_DEV_T *pMeiDev);
extern IFX_int32_t MEI_PLL_ConfigSet(MEI_DYN_CNTRL_T *pMeiDynCntrl);
extern IFX_void_t MEI_VRX_ModemStateUpdate(MEI_DYN_CNTRL_T *pMeiDynCntrl);
extern IFX_boolean_t MEI_PAF_EnableGet(MEI_DYN_CNTRL_T *pMeiDynCntrl);

#if defined(MEI_SUPPORT_DEBUG_STREAMS) && (MEI_SUPPORT_DEBUG_STREAMS == 1)
extern IFX_int_t MEI_DBG_STREAM_ModuleInit(void);

extern IFX_int_t MEI_DBG_STREAM_ModuleDelete(void);

extern IFX_int_t MEI_DBG_STREAM_ControlRelease(
                              MEI_DYN_CNTRL_T  *pMeiDynCntrl);

extern IFX_int_t MEI_DBG_STREAM_ControlReset(
                              MEI_DYN_CNTRL_T  *pMeiDynCntrl);

extern IFX_int_t MEI_DBG_STREAM_FwConfigSet(
                              MEI_DYN_CNTRL_T  *pMeiDynCntrl);

extern IFX_int_t MEI_IoctlDbgStreamDataGet(
                              MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                              IOCTL_MEI_DEBUG_STREAM_data_t  *pDbgStreamRead);

extern IFX_int_t MEI_IoctlDbgStreamMaskSet(
                              MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                              IOCTL_MEI_DEBUG_STREAM_mask_set_t  *pDbgStreamMask);

extern IFX_int_t MEI_IoctlDbgStreamConfigGet(
                              MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                              IOCTL_MEI_DEBUG_STREAM_configGet_t   *param);

extern IFX_int_t MEI_IoctlDbgStreamConfigSet(
                              MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                              const IOCTL_MEI_DEBUG_STREAM_configSet_t   *param);

extern IFX_int_t MEI_IoctlDbgStreamRelease(
                              MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                              const IOCTL_MEI_DEBUG_STREAM_release_t  *param);

extern IFX_int_t MEI_IoctlDbgStreamControl(
                              MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                              const IOCTL_MEI_DEBUG_STREAM_control_t  *pDbgStreamCntrl);

extern IFX_int_t MEI_IoctlDbgStreamStatisticGet(
                              MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                              IOCTL_MEI_DEBUG_STREAM_statistic_t   *pDbgStreamStatistics);

#endif /* (MEI_SUPPORT_DEBUG_STREAMS == 1) */
/* ==========================================================================
   Extern function definitions from the board driver for debug (trigger)
   ========================================================================== */

#if (MEI_EB_TRIGGER_FCT == 1)

/* inlcude the trigger functions from the VRX Eval Board driver */

extern IFX_void_t MEI_Eb_DfeTriggerWrite( IFX_uint8_t  DeviceNum,
                                            IFX_uint32_t regOffset,
                                            IFX_uint32_t regValue);

extern IFX_void_t MEI_Eb_DfeTriggerRead( IFX_uint8_t  DeviceNum,
                                           IFX_uint32_t regOffset);

/*  Access Macros */
#define MEI_EB_DFE_TRIGGER_ADDR_OFF     0x00100000
#define MEI_EB_DFE_TRIGGER_VALUE_WR     0x0000DEAD

#define MEI_EB_TRIGGER_WRITE       MEI_Eb_DfeTriggerWrite(0, MEI_EB_DFE_TRIGGER_ADDR_OFF, MEI_EB_DFE_TRIGGER_VALUE_WR)
#define MEI_EB_TRIGGER_READ        MEI_Eb_DfeTriggerRead(0, MEI_EB_DFE_TRIGGER_ADDR_OFF)

#else       /* #if (MEI_EB_TRIGGER_FCT == 1) */

#define MEI_EB_TRIGGER_WRITE
#define MEI_EB_TRIGGER_READ
#endif      /* #if (MEI_EB_TRIGGER_FCT == 1) */

#if (MEI_SUPPORT_DEBUG_LOGGER == 1)
extern int MEI_debugLogSend(IFX_char_t *fmt, ...)
#if __GNUC__ >= 3
__attribute__ ((__format__ (__printf__, 1, 2)))
#endif
;
#endif /* (MEI_SUPPORT_DEBUG_LOGGER == 1) */

#endif      /* #ifndef _DRV_MEI_CPE_API_H */

