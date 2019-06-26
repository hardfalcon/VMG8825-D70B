#ifndef _DRV_MEI_CPE_MEI_INTERFACE_H
#define _DRV_MEI_CPE_MEI_INTERFACE_H
/******************************************************************************

                          Copyright (c) 2007-2015
                     Lantiq Beteiligungs-GmbH & Co. KG

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/* ==========================================================================
   Description : VRX driver - MEI interface definitions
   Remarks:
   ========================================================================== */

#ifdef __cplusplus
extern "C"
{
#endif

/* ============================================================================
   Includes
   ========================================================================= */

/* include the IFX type definitions */
#include "ifx_types.h"
#include "drv_mei_cpe_config.h"
#include "drv_mei_cpe_os.h"
#include "drv_mei_cpe_dbg.h"

/* get the mailbox definitions */
#include "drv_mei_cpe_mailbox.h"


/* ============================================================================
   Local macro for packed structure definition.
   ========================================================================= */

#include "drv_mei_cpe_mei_access.h"

/* ============================================================================
   Configure Extended MEI Access Sequence
   ========================================================================= */

#ifndef MEI_EXT_MEI_ACCESS
/** support extended MEI access - via (inline) access function */
#  define MEI_EXT_MEI_ACCESS                  0
#endif


#if (MEI_EXT_MEI_ACCESS == 1)
#  ifndef MEI_EXT_MEI_ACCESS_ADD_CSE
      /* support extended MEI access and additional chip select extention */
#     define MEI_EXT_MEI_ACCESS_ADD_CSE       0
#  endif      /* #ifndef MEI_EXT_MEI_ACCESS_ADD_CSE */
#  ifndef MEI_EXT_MEI_ACCESS_ADD_CSE_MIPS
      /* support extended MEI access and additional chip select extention (MIPS) */
#     define MEI_EXT_MEI_ACCESS_ADD_CSE_MIPS  0
#  endif
#  if (MEI_EXT_MEI_ACCESS_ADD_CSE == 1) && (MEI_EXT_MEI_ACCESS_ADD_CSE_MIPS)
#     error "Select only ONE chip select extention"
#  endif
#else      /* #if (MEI_EXT_MEI_ACCESS == 1) */
#  ifdef MEI_EXT_MEI_ACCESS_ADD_CSE
#     undef MEI_EXT_MEI_ACCESS_ADD_CSE
#  endif
#  ifdef MEI_EXT_MEI_ACCESS_ADD_CSE_MIPS
#     undef MEI_EXT_MEI_ACCESS_ADD_CSE_MIPS
#  endif
#  define MEI_EXT_MEI_ACCESS_ADD_CSE         0
#  define MEI_EXT_MEI_ACCESS_ADD_CSE_MIPS    0
#endif      /* #if (MEI_EXT_MEI_ACCESS == 1) */



/* ============================================================================
   Local MACRO definitions
   ========================================================================= */

/*
   Setup the channel specific trace
*/

#ifdef LINUX
/** MEI Debug: Trace MEI Register access */
#define TRACE_MEI_MEI_REG_ACCESS     1
/** MEI Debug: Trace MEI Access Functions */
#define TRACE_MEI_MEI_ACCESS_FCT     1
#else

/* not supported for VxWorks - no printf on int-level */
#define TRACE_MEI_MEI_REG_ACCESS     0
#define TRACE_MEI_MEI_ACCESS_FCT     0
#endif


/** DMA message write: retry count in case of error */
#define MEI_PROTECTED_MEI_DMA_RETRY_CNT_MSG   1
/** DMA codeswap process: do no retry here (timing) */
#define MEI_PROTECTED_MEI_DMA_RETRY_CNT_CS    0
/** DMA misc write: retry count in case of error */
#define MEI_PROTECTED_MEI_DMA_RETRY_CNT_MISC  0


/* ============================================================================
   Target Control Macros
   ========================================================================= */

/** base address of the VRX peripheral block (PLL chipcfg, etc) */
#define MEI_PERIPHERAL_CONFIG_OFF   0x000E2000
#define MEI_PERIPHERAL_CONFIG_SIZE  0x000000FF


/** register offset "CHIPCFG" */
#define MEI_CHIPCFG_REG_OFF         0x40

/** register "CHIPCFG:MEMODE" */
#define MEI_CHIPCFG_MEMODE_POS      20
#define MEI_CHIPCFG_MEMODE_MASK     0x03
#define MEI_CHIPCFG_MEMODE_GET(chip_cfg) \
            ( (chip_cfg >> MEI_CHIPCFG_MEMODE_POS) & MEI_CHIPCFG_MEMODE_MASK)

/** register "CHIPCFG:HW_VER" */
#define MEI_CHIPCFG_HW_VER_POS      16
#define MEI_CHIPCFG_HW_VER_MASK     0x07
#define MEI_CHIPCFG_HW_VER_GET(chip_cfg) \
            ( (chip_cfg >> MEI_CHIPCFG_HW_VER_POS) & MEI_CHIPCFG_HW_VER_MASK)

/** register "CHIPCFG:CHIP_ID" */
#define MEI_CHIPCFG_CHIP_ID_POS     11
#define MEI_CHIPCFG_CHIP_ID_MASK    0x1F
#define MEI_CHIPCFG_CHIP_ID_GET(chip_cfg) \
            ( (chip_cfg >> MEI_CHIPCFG_CHIP_ID_POS) & MEI_CHIPCFG_CHIP_ID_MASK)

/** register "CHIPCFG:BTCFG" */
#define MEI_CHIPCFG_BTCFG_POS       5
#define MEI_CHIPCFG_BTCFG_MASK      0x0F
#define MEI_CHIPCFG_BTCFG_GET(chip_cfg) \
            ( (chip_cfg >> MEI_CHIPCFG_BTCFG_POS) & MEI_CHIPCFG_BTCFG_MASK)


/* ============================================================================
   VRX MEI interface - enumerations
   ========================================================================= */

/**
   Defines the HW status of the MEI resgister set.
*/
typedef enum
{
   /** no MEI access possible, IF is down */
   e_MEI_MEI_HW_STATE_DOWN     = -1,
   /** still not checked */
   e_MEI_MEI_HW_STATE_UNKNOWN  =  0,
   /** MEI access possible, IF is up */
   e_MEI_MEI_HW_STATE_UP       =  1
} MEI_MEI_HW_STATE_E;


/* ============================================================================
   VRX Rev 1 MEI interface - data struct typedefs
   ========================================================================= */

/**
   Control struct for handling the MEI interface data.

\remark
   Depending on the configuration one die can provide 2 or 4 DSL ports.
*/
typedef struct mei_if_cntrl_s
{
   /** state of the MEI HW */
   MEI_MEI_HW_STATE_E    eMeiHwState;
   /** physical address of the MEI register interface */
   IFX_ulong_t             phyBaseAddr;
   /** mapped virtual address of the physical MEI register interface */
   MEI_MEI_REG_IF_U      *pVirtMeiRegIf;
   /** physical address of the PDBRAM */
   IFX_ulong_t             phyPDBRAMaddr;
   /** mapped virtual address of the physical MEI register interface */
   IFX_ulong_t             virtPDBRAMaddr;
} MEI_MEI_IF_CNTRL_T;


/**
   VRX Rev 2 - device specific MEI Control struct (per device and per MEI
   Interface).

   This struct contains the data for handling the MEI Interface per devcie
   - physical Address
   - mapped virtual address (to a MEI register struct)
   - die-number
   - protection for unique access
*/
typedef struct mei_dev_cntrl_s
{
   /** port number (channel) within this device */
   IFX_uint8_t          relPort;

   /** protection - lock to garantee unique access */
   MEI_DRVOS_sema_t     pDevMeiAccessLock;

   /** MEI interface control data for driver handling */
   MEI_MEI_IF_CNTRL_T meiIf[1];

} MEI_MEI_DEV_CNTRL_T;


/**
   Driver specific MEI control struct.
*/
typedef struct mei_drv_cntrl_s
{
   /** points to the required driver data
       - line number
       - lock to protect the register access
   */

   /** line number (channel) of this port */
   IFX_uint8_t             dslLineNum;

   /** used MSG interrupt mask  */
   IFX_uint32_t            intMsgMask;

   /** Memory base addresses provided by PCIe driver */
   IFX_uint32_t MEI_pcie_phy_membase;
   IFX_uint32_t MEI_pcie_virt_membase;
   IFX_uint32_t MEI_pcie_irq;

   /** MEI control per device */
   MEI_MEI_DEV_CNTRL_T   *pMeiDevCntrl;

   /** pointer to the virtual address of the MEI register interface */
   MEI_MEI_IF_CNTRL_T    *pMeiIfCntrl;

} MEI_MEI_DRV_CNTRL_T;

/* ============================================================================
   Global MEI access variables
   ========================================================================= */

#if (MEI_EXT_MEI_ACCESS_ADD_CSE_MIPS == 1) || (MEI_EXT_MEI_ACCESS_ADD_CSE == 1)
extern IFX_int32_t MEI_DummyAccessLoopsRd;
extern IFX_int32_t MEI_DummyAccessLoopsWr;
#endif

/* VRX-Driver: MEI Access debug module - declare print level variable */
MEI_DRV_PRN_USR_MODULE_DECL(MEI_MEI_ACCESS);
MEI_DRV_PRN_INT_MODULE_DECL(MEI_MEI_ACCESS);


/* ============================================================================
   Global MEI access functions.
   ========================================================================= */

extern MEI_MeiRegVal_t MEI_RegAccOffGet(
                           MEI_MEI_DRV_CNTRL_T *pMeiDrvCntrl,
                           IFX_uint8_t          offset );

extern IFX_void_t MEI_RegAccOffSet(
                           MEI_MEI_DRV_CNTRL_T *pMeiDrvCntrl,
                           IFX_uint8_t          offset,
                           MEI_MeiRegVal_t      regValue);

extern IFX_int32_t MEI_InterfaceDetect(
                           MEI_MEI_DRV_CNTRL_T *pMeiDrvCntrl);

extern IFX_int32_t MEI_BasicChipInit(
                           IFX_void_t);

extern IFX_int32_t MEI_BasicChipExit(
                           IFX_void_t);

#if (MEI_SUPPORT_ROM_CODE == 1)
extern IFX_void_t MEI_RegNotifyCodeSwapDone(
                           MEI_MEI_DRV_CNTRL_T *pMeiDrvCntrl);
#endif

extern IFX_int32_t MEI_ResetDfeBlocks(
                           MEI_MEI_DRV_CNTRL_T *pMeiDrvCntrl,
                           IFX_boolean_t        resetMode,
                           IFX_int32_t          selMask);

extern IFX_int32_t MEI_InterfaceRecover(
                           MEI_MEI_DRV_CNTRL_T *pMeiDrvCntrl);

extern IFX_int32_t MEI_MaskInterrupts(
                           MEI_MEI_DRV_CNTRL_T *pMeiDrvCntrl,
                           IFX_int32_t           intMask);
extern IFX_int32_t MEI_UnMaskInterrupts(
                           MEI_MEI_DRV_CNTRL_T *pMeiDrvCntrl,
                           IFX_int32_t           intMask);

extern IFX_int32_t MEI_LowLevelInit(
                           MEI_MEI_DRV_CNTRL_T *pMeiDrvCntrl);

extern IFX_boolean_t MEI_GetMeiReg(
                           MEI_MEI_DRV_CNTRL_T *pMeiDrvCntrl,
                           IFX_uint8_t         off_byte,
                           MEI_MeiRegVal_t     *pRegVal);

extern IFX_boolean_t MEI_SetMeiReg(
                           MEI_MEI_DRV_CNTRL_T *pMeiDrvCntrl,
                           IFX_uint8_t          off_byte,
                           MEI_MeiRegVal_t      regVal);

extern IFX_int32_t MEI_WriteDbg(
                           MEI_MEI_DRV_CNTRL_T *pMeiDrvCntrl,
                           IFX_uint32_t          destAddr,
                           IFX_uint32_t          dbgDest,
                           IFX_uint32_t          dataCount,
                           IFX_uint32_t          *pData);

extern IFX_int32_t MEI_ReadDbg(
                           MEI_MEI_DRV_CNTRL_T *pMeiDrvCntrl,
                           IFX_uint32_t          srcAddr,
                           IFX_uint32_t          dbgSrc,
                           IFX_uint32_t          dataCount,
                           IFX_uint32_t          *pData);

extern IFX_uint32_t MEI_WriteDma16Bit(
                           MEI_MEI_DRV_CNTRL_T *pMeiDrvCntrl,
                           IFX_uint32_t          destAddr,
                           IFX_uint16_t          *pData,
                           IFX_uint32_t          dataCount,
                           IFX_int32_t           retryCnt);

extern IFX_uint32_t MEI_ReadDma16Bit(
                           MEI_MEI_DRV_CNTRL_T *pMeiDrvCntrl,
                           IFX_uint32_t          srcAddr,
                           IFX_uint16_t          *pData,
                           IFX_uint32_t          dataCount);

extern IFX_uint32_t MEI_ReadDma32Bit(
                           MEI_MEI_DRV_CNTRL_T *pMeiDrvCntrl,
                           IFX_uint32_t          destAddr,
                           IFX_uint32_t          *pDmaVal,
                           IFX_uint32_t          count32Bit);

extern IFX_uint32_t MEI_WriteDma32Bit(
                           MEI_MEI_DRV_CNTRL_T *pMeiDrvCntrl,
                           IFX_uint32_t          destAddr,
                           IFX_uint32_t          *pData,
                           IFX_uint32_t          count32Bit,
                           IFX_int32_t           retryCnt);

extern IFX_uint32_t MEI_GetDma(
                           MEI_MEI_DRV_CNTRL_T *pMeiDrvCntrl,
                           IFX_uint16_t          *pData,
                           IFX_uint32_t          dataCount);

extern IFX_int32_t  MEI_WriteMailBox(
                           MEI_MEI_DRV_CNTRL_T *pMeiDrvCntrl,
                           IFX_uint32_t          mbAddr,
                           MEI_MEI_MAILBOX_T   *pMeiMbBlk,
                           IFX_uint32_t          mbCount);

extern IFX_uint16_t MEI_GetMailBoxCode(
                           MEI_MEI_DRV_CNTRL_T *pMeiDrvCntrl,
                           IFX_uint32_t          mbAddr);

extern IFX_int32_t  MEI_GetMailBoxMsg(
                           MEI_MEI_DRV_CNTRL_T *pMeiDrvCntrl,
                           IFX_uint32_t          mbAddr,
                           MEI_MEI_MAILBOX_T   *pMeiMbBlk,
                           IFX_uint32_t          mbCount,
                           IFX_boolean_t         releaseMb);

extern IFX_boolean_t MEI_ReleaseMailboxMsg(
                           MEI_MEI_DRV_CNTRL_T *pMeiDrvCntrl);

#if (MEI_SUPPORT_TEST_DEBUG == 1)
extern IFX_void_t MEI_ShowMeiRegs(
                           MEI_MEI_DRV_CNTRL_T *pMeiDrvCntrl,
                           IFX_uint8_t           devNum);

extern IFX_boolean_t MEI_DmaTest(
                           MEI_MEI_DRV_CNTRL_T *pMeiDrvCntrl,
                           IFX_uint32_t          destAddr,
                           IFX_uint32_t          dma_count,
                           IFX_uint32_t          test_count);
#endif

#if (MEI_SUPPORT_ROM_CODE == 1)
/* ============================================================================
   Export MEI Register Access Functions (Firmware Download ROM start)
   ========================================================================= */
extern IFX_boolean_t MEI_ReleaseRomInt(
                           MEI_MEI_DRV_CNTRL_T *pMeiDrvCntrl);

extern IFX_int32_t MEI_GetMailBoxBootMsg(
                           MEI_MEI_DRV_CNTRL_T *pMeiDrvCntrl,
                           MEI_Mailbox_t       *pMeiMbBlk,
                           IFX_uint32_t          mbCount,
                           IFX_boolean_t         releaseMsg);
#endif


#ifdef __cplusplus
/* extern "C" */
}
#endif

#endif      /* #define _DRV_MEI_CPE_MEI_INTERFACE_H */

