/******************************************************************************

                          Copyright (c) 2007-2015
                     Lantiq Beteiligungs-GmbH & Co. KG

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/* ==========================================================================
   Description : AR9 MEI interface access function.
   ========================================================================== */

/* ============================================================================
   Inlcudes
   ========================================================================= */

/* get at first the driver configuration */
#include "drv_mei_cpe_config.h"

#if (MEI_SUPPORT_DEVICE_AR9 == 1)

#include "ifx_types.h"
#include "drv_mei_cpe_os.h"
#include "drv_mei_cpe_dbg.h"

#include "drv_mei_cpe_dbg_driver.h"

#include "drv_mei_cpe_mei_ar9.h"

#include "drv_mei_cpe_mei_interface.h"
#include "drv_mei_cpe_api.h"
#include "drv_mei_cpe_mailbox.h"

#include "cmv_message_format.h"

#include "cmv_message_format_ar9.h"

/* ============================================================================
   Local macro definition
   ========================================================================= */

#if (TRACE_MEI_MEI_REG_ACCESS == 1)

/* MEI-Driver: MEI register trace - create print level variable */
MEI_DRV_PRN_USR_MODULE_CREATE(MEI_REG, MEI_DRV_PRN_LEVEL_OFF);
MEI_DRV_PRN_INT_MODULE_CREATE(MEI_REG, MEI_DRV_PRN_LEVEL_OFF);


#define TRACE_WR_MEI_REG(level, pMeiDrvCntrl, regname, regvalue)\
            PRN_DBG_USR_NL( MEI_REG, level, \
                  ("MEI[%02d]  off[0x%02X] wr: 0x%04X" MEI_DRV_CRLF, \
                  (IFX_uint32_t)(pMeiDrvCntrl->dslLineNum), (IFX_uint32_t)MEI_REG_OFF_##regname, \
                  (IFX_uint32_t)(regvalue) ))

#define TRACE_RD_MEI_REG(level, pMeiDrvCntrl, regname, regvalue)\
            PRN_DBG_USR_NL( MEI_REG, level, \
                  ("MEI[%02d]  off[0x%02X] rd: 0x%04X" MEI_DRV_CRLF, \
                  (IFX_uint32_t)(pMeiDrvCntrl->dslLineNum), (IFX_uint32_t)MEI_REG_OFF_##regname, \
                  (IFX_uint32_t)(regvalue) ))

#define TRACE_WR_MEI_OFF(level, pMeiDrvCntrl, offset, regvalue)\
            PRN_DBG_USR_NL( MEI_REG, level, \
                  ("MEI[%02d]  off[0x%02X]: 0x%04X" MEI_DRV_CRLF, \
                  (IFX_uint32_t)(pMeiDrvCntrl->dslLineNum), (IFX_uint32_t)offset, \
                  (IFX_uint32_t)(regvalue) ))


#define TRACE_RD_MEI_OFF(level, pMeiDrvCntrl, offset, regvalue)\
            PRN_DBG_USR_NL( MEI_REG, level, \
                  ("MEI[%02d]  off[0x%02X]: 0x%04X" MEI_DRV_CRLF, \
                  (IFX_uint32_t)(pMeiDrvCntrl->dslLineNum), (IFX_uint32_t)offset, \
                  (IFX_uint32_t)(regvalue) ))

#else

#define TRACE_WR_MEI_REG(level, pMeiDrvCntrl, regname, regvalue)
#define TRACE_RD_MEI_REG(level, pMeiDrvCntrl, regname, regvalue)
#define TRACE_WR_MEI_OFF(level, pMeiDrvCntrl, offset, regvalue)
#define TRACE_RD_MEI_OFF(level, pMeiDrvCntrl, offset, regvalue)

#endif

/* ============================================================================
   Local Function Declarations
   ========================================================================= */
static IFX_void_t MEI_EnableDbgAccess(
                           MEI_MEI_DRV_CNTRL_T *pMeiDrvCntrl,
                           IFX_uint32_t dbg_dest);
static IFX_void_t MEI_DisableDbgAccess(
                           MEI_MEI_DRV_CNTRL_T *pMeiDrvCntrl);

static IFX_boolean_t MEI_PollForDbgDone(
                           MEI_MEI_DRV_CNTRL_T *pMeiDrvCntrl,
                           IFX_int32_t           pollCount);

#if (MEI_PROTECTED_MEI_DMA_ACCESS == 1)
static IFX_uint32_t MEI_CmpDma(
                           MEI_MEI_DRV_CNTRL_T *pMeiDrvCntrl,
                           IFX_uint32_t         destAddr,
                           IFX_uint16_t         *pData,
                           IFX_uint32_t         dataCount);
#endif

/* ============================================================================
   Local variable definition
   ========================================================================= */

/* MEI-Driver: MEI Access module - create print level variable */
MEI_DRV_PRN_USR_MODULE_CREATE(MEI_MEI_ACCESS, MEI_DRV_PRN_LEVEL_HIGH);
MEI_DRV_PRN_INT_MODULE_CREATE(MEI_MEI_ACCESS, MEI_DRV_PRN_LEVEL_HIGH);


/* ============================================================================
   Macro/functions for Extended MEI Access Sequence
   ========================================================================= */

#if (MEI_EXT_MEI_ACCESS == 1)
   /* set access functions for MEI register access */
#  define MEI_REG_WRAP_GET(pRegAddr)            MEI_RegAccessRd((MEI_MeiReg_t *)&(pRegAddr))
#  define MEI_REG_WRAP_SET(pRegAddr, val)       MEI_RegAccessWr((MEI_MeiReg_t *)&(pRegAddr), val)
#else
   /* set access MACROS for MEI register access */
#  define MEI_REG_WRAP_GET(pRegAddr)            pRegAddr
#  define MEI_REG_WRAP_SET(pRegAddr, val)       pRegAddr = (val & 0xFFFFFFFF)
#endif


#if (MEI_EXT_MEI_ACCESS == 1)
#  if __GNUC__
#  define MEI_INLINE   static inline
#else
#  define MEI_INLINE  static
#endif

#if (MEI_EXT_MEI_ACCESS_ADD_CSE_MIPS == 1)
   /* dummy variable - not cachable memory on the mips controller */
   IFX_vuint32_t *pMEI_DummyAccess = (IFX_vuint32_t*) 0x80000000;
   IFX_uint32_t  MEI_DummyAccess;

#  define MEI_DUMMY_ACCESS_RD   MEI_DummyAccess = *pMEI_DummyAccess
#  define MEI_DUMMY_ACCESS_WR   MEI_DummyAccess = *pMEI_DummyAccess

#elif (MEI_EXT_MEI_ACCESS_ADD_CSE == 1)

   IFX_vuint32_t  MEI_DummyAccess = 0;
#  define MEI_DUMMY_ACCESS_RD   MEI_DummyAccess++
#  define MEI_DUMMY_ACCESS_WR   MEI_DummyAccess++
#else       /* (MEI_EXT_MEI_ACCESS_ADD_CSE == 1) */

#  define MEI_DUMMY_ACCESS_RD
#  define MEI_DUMMY_ACCESS_WR

#endif      /* #if (MEI_EXT_MEI_ACCESS_ADD_CSE_MIPS == 1) */


#if (MEI_EXT_MEI_ACCESS_ADD_CSE_MIPS == 1) || (MEI_EXT_MEI_ACCESS_ADD_CSE == 1)

IFX_int32_t MEI_DummyAccessLoopsRd = 4;
IFX_int32_t MEI_DummyAccessLoopsWr = 3;

/* dummy read loop */
#define MEI_DUMMY_ACCESS_LOOP_RD \
         {\
            IFX_int32_t i;\
            for (i = 0; i < MEI_DummyAccessLoopsRd; i++) { MEI_DUMMY_ACCESS_RD; \
         }

/* dummy write loop */
#define MEI_DUMMY_ACCESS_LOOP_WR \
         {\
            IFX_int32_t i;\
            for (i = 0; i < MEI_DummyAccessLoopsRd; i++) { MEI_DUMMY_ACCESS_RD; \
         }
#else
#define MEI_DUMMY_ACCESS_LOOP_RD
#define MEI_DUMMY_ACCESS_LOOP_WR
#endif

#endif      /* #if (MEI_EXT_MEI_ACCESS == 1) */


#if (MEI_EXT_MEI_ACCESS == 1)
/**
   MEI Register Read Access functions
\param
   pMeiReg  - points to the MEI register

\return
   Register value.

\remark
   For GNU Compile: inline
*/
MEI_INLINE IFX_uint32_t MEI_RegAccessRd(
                           MEI_MeiReg_t *pMeiReg)
{
#if ((MEI_EXT_MEI_ACCESS_ADD_CSE_MIPS == 1) || (MEI_EXT_MEI_ACCESS_ADD_CSE == 1))
   IFX_int32_t i;

   for (i = 0; i < MEI_DummyAccessLoopsRd; i++)
   {
      MEI_DUMMY_ACCESS_RD;
   }
#endif

   return (IFX_uint32_t)(*pMeiReg);
}

/**
   MEI Register Write Access functions

\param
   pMeiReg  - points to the MEI register.

\param
   val      - new register value to set.

\return
   None.

\remark
   For GNU Compile: inline
*/
MEI_INLINE IFX_void_t MEI_RegAccessWr(
                           MEI_MeiReg_t *pMeiReg, IFX_uint32_t val)
{
#if ((MEI_EXT_MEI_ACCESS_ADD_CSE_MIPS == 1) || (MEI_EXT_MEI_ACCESS_ADD_CSE == 1))
   IFX_int32_t i;

   for (i = 0; i < MEI_DummyAccessLoopsWr; i++)
   {
      MEI_DUMMY_ACCESS_WR;
   }
#endif

   *pMeiReg = val;
   return;
}
#endif   /* #if (MEI_EXT_MEI_ACCESS == 1) */



/* ============================================================================
   Function definitions
   ========================================================================= */

/**
   Enable the MEI debug access for read or write transactions.
   - Write a '1' into the HOST_MSTR bit of the ME_DBG_MASTER register
     to grant the ME access to the bus.
   - Write into the ME_DBG_DECODE register to specify whether the access
     will be Core, Auxiliary, or Load/Store space.

\param
   pMeiDrvCntrl:   points to the MEI interface register set
\param
   dbg_dest:      Destination address space within the ARC

\return
   none

\attention:
   Auxiliary and core address space is only accessible when the ARC is halted.
*/
static IFX_void_t MEI_EnableDbgAccess(
                           MEI_MEI_DRV_CNTRL_T *pMeiDrvCntrl,
                           IFX_uint32_t          dbg_dest)
{
   IFX_uint32_t temp;

#if TRACE_MEI_MEI_ACCESS_FCT == 1
   PRN_DBG_USR_NL( MEI_MEI_ACCESS, MEI_DRV_PRN_LEVEL_LOW,
          ("MEI[%02d]: Enable Debug Access" MEI_DRV_CRLF,
          pMeiDrvCntrl->dslLineNum ));
#endif

   TRACE_WR_MEI_REG( MEI_DRV_PRN_LEVEL_LOW, pMeiDrvCntrl, ME_DBG_MASTER,
                     (MEI_REG_ACCESS_ME_DBG_MASTER_GET(pMeiDrvCntrl) | ME_DBG_MASTER_HOST_MSTR));

   /* enable the debug access */
   MEI_ENABLE_DBG_HOST_MASTER_ON(pMeiDrvCntrl);

   temp  = (MEI_REG_ACCESS_ME_DBG_DECODE_GET(pMeiDrvCntrl) & ~(ME_DBG_DECODE_DEBUG_DEC));
   temp |= (dbg_dest & (ME_DBG_DECODE_DEBUG_DEC));

   TRACE_WR_MEI_REG( MEI_DRV_PRN_LEVEL_LOW, pMeiDrvCntrl, ME_DBG_DECODE, temp);

   MEI_REG_ACCESS_ME_DBG_DECODE_SET(pMeiDrvCntrl, temp);

   return ;
}


/**
   Disable the MEI debug access after read or write transactions.
   - Write a '0' into the HOST_MSTR bit of the ME_DBG_MASTER register
     to release the ME access to the bus.

\param
   pMeiDrvCntrl:   points to the MEI interface register set

\return
   none

\attention:
   Auxiliary and core address space is only accessible when the ARC is halted.
*/
static IFX_void_t MEI_DisableDbgAccess(
                           MEI_MEI_DRV_CNTRL_T *pMeiDrvCntrl)
{

#if TRACE_MEI_MEI_ACCESS_FCT == 1
   PRN_DBG_USR_NL( MEI_MEI_ACCESS, MEI_DRV_PRN_LEVEL_LOW,
          ("MEI[%02d]: Disable Debug Access" MEI_DRV_CRLF,
          (IFX_uint32_t)pMeiDrvCntrl->dslLineNum ));
#endif

   TRACE_WR_MEI_REG( MEI_DRV_PRN_LEVEL_LOW, pMeiDrvCntrl, ME_DBG_MASTER,
                     (MEI_REG_ACCESS_ME_DBG_MASTER_GET(pMeiDrvCntrl) & ~(ME_DBG_MASTER_HOST_MSTR)));

   MEI_DISABLE_DBG_HOST_MASTER_OFF(pMeiDrvCntrl);

   return ;
}


/**
   Poll for Debug access Done identification.
   Poll the DBG_DONE bit of the ME_ARC2ME_STAT register to determine that
   the transaction is complete before attempting another debug transaction.

\param
   pMeiDrvCntrl:   points to the MEI interface register set
\param
   pollCount: Number of check attempts.

\return
   IFX_TRUE:  transaction complete
   IFX_FALSE: Transaction timeout

\remarks
   Interrupt for debug access currently not supported.
   Use polling or interrupt to wait for transaction complete signal.
   If waiting for an interrupt, the DBG_DONE bit in the ME_ARC2ME_MASK
   register must be set.
*/
static IFX_boolean_t MEI_PollForDbgDone(
                           MEI_MEI_DRV_CNTRL_T *pMeiDrvCntrl,
                           IFX_int32_t           pollCount)
{
   IFX_uint32_t Arc2MeStatReg = 0;

   while(pollCount > 0)
   {
      Arc2MeStatReg = MEI_REG_ACCESS_ME_ARC2ME_STAT_GET(pMeiDrvCntrl);

      if( (Arc2MeStatReg & (ME_ARC2ME_STAT_DBG_DONE)) ||
          (Arc2MeStatReg & (ME_ARC2ME_STAT_DBG_ERR)) )
         break;

      pollCount--;
   }

   /* clear the debug done flag - independent of the result */
   MEI_DBG_CLEAR_DBG_DONE(pMeiDrvCntrl);

   if ( Arc2MeStatReg & (ME_ARC2ME_STAT_DBG_ERR) )
   {
      PRN_ERR_USR_NL( MEI_MEI_ACCESS, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI[%02d]: ERROR - Debug Access, Dbg ERROR" MEI_DRV_CRLF,
            (IFX_uint32_t)pMeiDrvCntrl->dslLineNum));

      MEI_DBG_CLEAR_DBG_ERR(pMeiDrvCntrl);
      return IFX_FALSE;
   }

   if (pollCount < 0)
   {
      /* TRACE timeout */
      PRN_ERR_USR_NL( MEI_MEI_ACCESS, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI[%02d]: ERROR - Debug Access, Dbg Done timeout" MEI_DRV_CRLF,
            (IFX_uint32_t)pMeiDrvCntrl->dslLineNum));

      return IFX_FALSE;
   }

   TRACE_WR_MEI_REG( MEI_DRV_PRN_LEVEL_LOW, pMeiDrvCntrl, ME_ARC2ME_STAT,
                     (MEI_REG_ACCESS_ME_ARC2ME_STAT_GET(pMeiDrvCntrl) & ~(ME_ARC2ME_STAT_DBG_DONE)));

   return IFX_TRUE;
}     /* IFX_boolean_t MEI_PollForDbgDone(...) */


/**
   Read a target 32 bit data bloack via DMA

\param
   pMeiDrvCntrl:   points to the MEI interface register set
\param
   dmaAddr:        DMA source address where read from.
\param
   pDmaVal:        point to 32bit data block.
\param
   count32Bit:      32 bit data block count.

\return
   Number of read data units (32 bit)

*/
IFX_uint32_t MEI_ReadDma32Bit(
                           MEI_MEI_DRV_CNTRL_T *pMeiDrvCntrl,
                           IFX_uint32_t          destAddr,
                           IFX_uint32_t          *pDmaVal,
                           IFX_uint32_t          count32Bit)
{
   IFX_uint32_t cnt;

   MEI_REG_ACCESS_ME_DX_AD_LONG_SET(pMeiDrvCntrl, destAddr);

   for (cnt = 0; cnt < count32Bit; cnt++)
   {
      /* read 32bit word */
      pDmaVal[cnt] = MEI_REG_ACCESS_ME_DX_DATA_GET(pMeiDrvCntrl);

#if TRACE_MEI_MEI_ACCESS_FCT == 1
      PRN_DBG_INT_NL( MEI_REG, MEI_DRV_PRN_LEVEL_LOW,
            ("MEI[%02d]: ReadDma32Bit - addr = 0x%08X, val = 0x%08X" MEI_DRV_CRLF,
            (IFX_uint32_t)pMeiDrvCntrl->dslLineNum, destAddr, pDmaVal[cnt] ));
#endif
   }

   return cnt;
}


/**
   Write a 32 bit data block value via DMA to the target.

\param
   pMeiDrvCntrl:   points to the MEI interface register set
\param
   dmaAddr:        DMA source address where read from.
\param
   pData:          point to 32bit data block.
\param
   dataCount:      32 bit data block count.

\return
   Number of written data units (32 bit)

*/
IFX_uint32_t MEI_WriteDma32Bit(
                           MEI_MEI_DRV_CNTRL_T *pMeiDrvCntrl,
                           IFX_uint32_t          destAddr,
                           IFX_uint32_t          *pData,
                           IFX_uint32_t          count32Bit,
                           IFX_int32_t           retryCnt)
{
   IFX_uint32_t cnt;
   IFX_uint32_t tmpRegValue;

   MEI_REG_ACCESS_ME_DX_AD_LONG_SET(pMeiDrvCntrl, destAddr);

   for (cnt = 0; cnt < count32Bit; cnt++)
   {
#if (TRACE_MEI_MEI_ACCESS_FCT == 1)
      PRN_DBG_INT_NL( MEI_REG, MEI_DRV_PRN_LEVEL_LOW,
            ("MEI[%02d]: WriteDma32Bit - addr = 0x%08X, val = 0x%08X" MEI_DRV_CRLF,
            (IFX_uint32_t)pMeiDrvCntrl->dslLineNum, destAddr, pData[cnt] ));
#endif
      /* write 32bit word */
      MEI_REG_ACCESS_ME_DX_DATA_SET( pMeiDrvCntrl, pData[cnt]);
   }

   /* check for DMA error */
   tmpRegValue = MEI_REG_ACCESS_ME_DX_STAT_GET(pMeiDrvCntrl);
   if (tmpRegValue & ME_DX_STAT_DX_ERR)
   {
      /* clear error flag */
      MEI_REG_ACCESS_ME_DX_STAT_SET(pMeiDrvCntrl, ME_DX_STAT_DX_ERR);
      PRN_ERR_INT_NL( MEI_MEI_ACCESS, MEI_DRV_PRN_LEVEL_WRN,
            ("MEI[%02d]: Write(%d) DMA ERROR - dest addr = 0x%08X, ME_DX_STAT = 0x%04X\n\r",
            (IFX_uint32_t)pMeiDrvCntrl->dslLineNum, retryCnt, destAddr, tmpRegValue) );

      cnt = (IFX_uint32_t)IFX_ERROR;
   }

   return cnt;
}


/**
   Reset the indicated MEI device subsystems.

\param
   pMeiDrvCntrl:   points to the MEI interface register set.
\param
   resetMode:  Activate (1) / Deactivate (0) the reset
\param
   selMask:    Indicates the subsystems for reset.
               bit 0: XMEM_RST - external mem controler.
               bit 1: DSP_RST  - reset all of ALCMENE
                                 (except exernal mem controller and MEI regs).
               bit 2: XDSL_RST - XDSL accelarator SW controlled reset.
               bit 3: SPI_RST  - Serial peripheral interface reset.
               bit 4: PER_RST  - Peripheral reset

\return
   IFX_SUCCESS
*/
IFX_int32_t MEI_ResetDfeBlocks(
                           MEI_MEI_DRV_CNTRL_T *pMeiDrvCntrl,
                           IFX_boolean_t         resetMode,
                           IFX_int32_t           selMask)
{
   IFX_uint32_t tmpReg = 0;

   tmpReg = MEI_REG_ACCESS_ME_RST_CTRL_GET(pMeiDrvCntrl);

   if (resetMode)
   {
      /* activate - set reset flag */
      tmpReg |= ((IFX_uint32_t)(selMask & (ME_RST_CTRL_ALL)));
   }
   else
   {
      /* deactivate - release reset flag */
      tmpReg &= ((IFX_uint32_t)(~selMask & (ME_RST_CTRL_ALL)));
   }

#if TRACE_MEI_MEI_ACCESS_FCT == 1
   PRN_DBG_USR_NL( MEI_MEI_ACCESS, MEI_DRV_PRN_LEVEL_NORMAL,
              ("MEI[%02d]: MEI_ResetDfeBlocks - %s reset AR9: 0x%04X" MEI_DRV_CRLF,
              (IFX_uint32_t)pMeiDrvCntrl->dslLineNum, (resetMode)?"act":"deact", tmpReg));
#endif

   MEI_REG_ACCESS_ME_RST_CTRL_SET( pMeiDrvCntrl, tmpReg );

   return IFX_SUCCESS;
}

IFX_int32_t MEI_InterfaceRecover(
                           MEI_MEI_DRV_CNTRL_T *pMeiDrvCntrl)
{
   return IFX_SUCCESS;
}

/**
   Mask the indicated interrupts.

\param
   pMeiDrvCntrl:   points to the MEI interface register set.
\param
   intMask:    Indicates the interrupts for clear.
               bit 0: ARC_MSGAV   - ARC2ME message available.
               bit 1: ARC_GP_INT0 - Genaral Purpose Interrupt 0.
               bit 2: ARC_GP_INT1 - Genaral Purpose Interrupt 1.
               bit 3: DBG_DONE    - Debug Access Complete.
               bit 4: DBG_ERR     - Debug Access Error.

\return
   IFX_SUCCESS

*/
IFX_int32_t MEI_MaskInterrupts(
                           MEI_MEI_DRV_CNTRL_T *pMeiDrvCntrl,
                           IFX_int32_t           intMask)
{

   MEI_REG_ACCESS_ME_ARC2ME_MASK_SET(pMeiDrvCntrl, (~intMask & (ME_ARC2ME_INTERRUPT_MASK_ALL)) );

   return IFX_SUCCESS;
}

/**
   Unmask the indicated interrupts.

\param
   pMeiDrvCntrl:   points to the MEI interface register set.
\param
   intMask:    Indicates the interrupts for clear.
               bit 0: ARC_MSGAV   - ARC2ME message available.
               bit 1: ARC_GP_INT0 - Genaral Purpose Interrupt 0.
               bit 2: ARC_GP_INT1 - Genaral Purpose Interrupt 1.
               bit 3: DBG_DONE    - Debug Access Complete.
               bit 4: DBG_ERR     - Debug Access Error.

\return
   IFX_SUCCESS

*/
IFX_int32_t MEI_UnMaskInterrupts(
                           MEI_MEI_DRV_CNTRL_T *pMeiDrvCntrl,
                           IFX_int32_t           intMask )
{

   MEI_REG_ACCESS_ME_ARC2ME_MASK_SET(pMeiDrvCntrl, (intMask & (ME_ARC2ME_INTERRUPT_UNMASK_ALL)) );

#if TRACE_MEI_MEI_ACCESS_FCT == 1
   PRN_DBG_USR_NL( MEI_MEI_ACCESS, MEI_DRV_PRN_LEVEL_NORMAL,
         ("MEI[%02d]: MEI_UnMaskInterrupts - unmask interrupts: 0x%04X (STAT 0x%04X)" MEI_DRV_CRLF,
         (IFX_uint32_t)pMeiDrvCntrl->dslLineNum,
         /* (intMask & (ME_ARC2ME_INTERRUPT_MASK_ALL)), */
         MEI_REG_ACCESS_ME_ARC2ME_MASK_GET(pMeiDrvCntrl),
         MEI_REG_ACCESS_ME_ARC2ME_STAT_GET(pMeiDrvCntrl) ));
#endif


   return IFX_SUCCESS;
}


/**
   Read and return an MEI register.

\param
   pMeiDrvCntrl:   points to the MEI interface register set.
\param
   offset:     Relative byte offset within the MEI register set.
\param
   pRegVal:    Points to the return field (32 bit) where to write the value.

\return
   TRUE if success.
   FALSE in case of error (invalid offset)
*/
IFX_boolean_t MEI_GetMeiReg(
                           MEI_MEI_DRV_CNTRL_T *pMeiDrvCntrl,
                           IFX_uint8_t         off_byte,
                           MEI_MeiRegVal_t     *pRegVal)
{
   IFX_uint8_t off_32bit = off_byte / MEI_MEI_DMA_DATA_WIDTH;

   if (
       /* (off_32bit < MEI_REG_OFFSET_FIRST) ||*/  /* currently 0 */
       (off_32bit > MEI_REG_OFFSET_LAST) ||        /* max offset */
       (off_byte % MEI_MEI_DMA_DATA_WIDTH)             /* alignment */
      )
   {
#if TRACE_MEI_MEI_ACCESS_FCT == 1
      PRN_ERR_USR_NL( MEI_MEI_ACCESS, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI[%02d]: ERROR GetMeiReg - invalid offset 0x%02X" MEI_DRV_CRLF,
            (IFX_uint32_t)pMeiDrvCntrl->dslLineNum, (IFX_uint32_t)off_byte));
#endif
      return IFX_FALSE;
   }

   *pRegVal = MEI_REG_ACCESS_OFFSET_GET(pMeiDrvCntrl, off_32bit);

   TRACE_RD_MEI_OFF( MEI_DRV_PRN_LEVEL_LOW, pMeiDrvCntrl, off_32bit, *pRegVal);

   return IFX_TRUE;
}


/**
   Write a value to the MEI register.

\param
   pMeiDrvCntrl:   points to the MEI interface register set.
\param
   offset:     Relative byte offset within the MEI register set.
\param
   regVal:    Value to write.

\return
   TRUE if success.
   FALSE in case of error (invalid offset)
*/
IFX_boolean_t MEI_SetMeiReg(
                           MEI_MEI_DRV_CNTRL_T *pMeiDrvCntrl,
                           IFX_uint8_t          off_byte,
                           MEI_MeiRegVal_t      regVal)
{
   IFX_uint8_t off_32bit = off_byte / MEI_MEI_DMA_DATA_WIDTH;

   if (
       /* (off_32bit < MEI_REG_OFFSET_FIRST) ||*/  /* currently 0 */
       (off_32bit > MEI_REG_OFFSET_LAST) ||        /* max offset */
       (off_byte % MEI_MEI_DMA_DATA_WIDTH)             /* alignment */
      )
   {
#if TRACE_MEI_MEI_ACCESS_FCT == 1
      PRN_ERR_USR_NL( MEI_MEI_ACCESS, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI[%02d]: ERROR SetMeiReg - invalid offset 0x%02X" MEI_DRV_CRLF,
            (IFX_uint32_t)pMeiDrvCntrl->dslLineNum, (IFX_uint32_t)off_byte));
#endif
      return IFX_FALSE;
   }

   TRACE_WR_MEI_OFF( MEI_DRV_PRN_LEVEL_LOW, pMeiDrvCntrl, off_32bit, regVal);

   MEI_REG_ACCESS_OFFSET_SET(pMeiDrvCntrl, off_32bit, regVal);

   return IFX_TRUE;
}


/**
   Write a data block to the MEI debug interface.
   - Enable debug access
   - Do debug write transaction.
   - Disable debug access.

\param
   pMeiDrvCntrl:   points to the MEI interface register set.
\param
   destAddr:   Destination debug address where to write to.
\param
   dbgDest:    Destination address space within the ARC.
\param
   pData:      Points to the data block to write.
\param
   dataCount:  Number of data units (32 bit) to write.

\return
   Number of written data units (32 bit).

\attention:
   Auxiliary and core address space is only accessible when the ARC is halted.
*/
IFX_int32_t MEI_WriteDbg(
                           MEI_MEI_DRV_CNTRL_T *pMeiDrvCntrl,
                           IFX_uint32_t          destAddr,
                           IFX_uint32_t          dbgDest,
                           IFX_uint32_t          dataCount,
                           IFX_uint32_t          *pData)
{
   IFX_uint32_t count;

#if TRACE_MEI_MEI_ACCESS_FCT == 1
      PRN_DBG_USR_NL( MEI_REG, MEI_DRV_PRN_LEVEL_LOW,
            ("MEI[%02d]: WriteDbg - addr = 0x%08X, dest = %d, count = %d" MEI_DRV_CRLF,
            (IFX_uint32_t)pMeiDrvCntrl->dslLineNum, destAddr, dbgDest, dataCount) );
#endif

   MEI_EnableDbgAccess(pMeiDrvCntrl, dbgDest);

   for (count=0; count<dataCount; count++)
   {
#if TRACE_MEI_MEI_REG_ACCESS == 1
      PRN_DBG_USR_NL( MEI_REG, MEI_DRV_PRN_LEVEL_LOW,
            ("MEI[%02d] wr: addr [0x%02X] = 0x%08X, data [0x%02X]: 0x%08X" MEI_DRV_CRLF,
            (IFX_uint32_t)pMeiDrvCntrl->dslLineNum,
            MEI_REG_OFF_ME_DBG_WR_AD, destAddr,
            MEI_REG_OFF_ME_DBG_DATA, (IFX_uint32_t)*pData ));
#endif

      /* set address */
      MEI_REG_ACCESS_ME_DBG_WR_AD_LONG_SET(pMeiDrvCntrl, destAddr);

      MEI_REG_ACCESS_ME_DBG_DATA_LONG_SET(pMeiDrvCntrl, *pData);

      if ( MEI_PollForDbgDone(pMeiDrvCntrl, MEI_MEI_DBG_POLL_WAIT_COUNT) == IFX_FALSE )
         break;

      pData++;
      destAddr += 4;
   }

   MEI_DisableDbgAccess(pMeiDrvCntrl);

   return (IFX_int32_t)count;
}     /* IFX_int32_t MEI_WriteDbg(...) */


/**
   Read a data block from the MEI debug interface.
   - Enable debug access
   - Do debug read transaction.
   - Disable debug access.

\param
   pMeiDrvCntrl:   points to the MEI interface register set
\param
   srcAddr:    Debug source address where to read from.
\param
   dbgSrc:     Debug address space within the ARC
\param
   pData:      Points to the data block where to put the data.
\param
   dataCount:  Number of data units (32 bit) to read

\return
   Number of read data units (32 bit)

\attention:
   Auxiliary and core address space is only accessible when the ARC is halted.
*/
IFX_int32_t MEI_ReadDbg(
                           MEI_MEI_DRV_CNTRL_T *pMeiDrvCntrl,
                           IFX_uint32_t          srcAddr,
                           IFX_uint32_t          dbgSrc,
                           IFX_uint32_t          dataCount,
                           IFX_uint32_t          *pData)
{
   IFX_uint32_t count;
   IFX_uint32_t *pTempData = pData;

#if TRACE_MEI_MEI_ACCESS_FCT == 1
      PRN_DBG_USR_NL( MEI_REG, MEI_DRV_PRN_LEVEL_LOW,
            ("MEI[%02d]: ReadDbg - addr = 0x%08X, dest = %d, count = %d" MEI_DRV_CRLF,
            (IFX_uint32_t)pMeiDrvCntrl->dslLineNum, srcAddr, dbgSrc, dataCount) );
#endif

   MEI_EnableDbgAccess(pMeiDrvCntrl, dbgSrc);

   for (count=0; count<dataCount; count++)
   {
      MEI_REG_ACCESS_ME_DBG_RD_AD_LONG_SET(pMeiDrvCntrl, srcAddr);

      if ( MEI_PollForDbgDone(pMeiDrvCntrl, MEI_MEI_DBG_POLL_WAIT_COUNT) == IFX_FALSE )
         break;

      /* now Read data */
      *pTempData = MEI_REG_ACCESS_ME_DBG_DATA_LONG_GET(pMeiDrvCntrl);

#if TRACE_MEI_MEI_REG_ACCESS == 1
      PRN_DBG_USR_NL( MEI_REG, MEI_DRV_PRN_LEVEL_LOW,
            ("MEI[%02d] rd: addr [0x%02X] = 0x%08X, data [0x%02X]: 0x%08X" MEI_DRV_CRLF,
            (IFX_uint32_t)pMeiDrvCntrl->dslLineNum,
            MEI_REG_OFF_ME_DBG_RD_AD, srcAddr,
            MEI_REG_OFF_ME_DBG_DATA, *pTempData ));
#endif

      pTempData++;
      srcAddr += 4;
   }

   MEI_DisableDbgAccess(pMeiDrvCntrl);

   return (IFX_int32_t)count;
}


/**
   MEI DMA Write - write a 16bit data block to the MEI DMA interface.

\param
   pMeiDrvCntrl:   points to the MEI interface register set
\param
   destAddr:   DMA destination address where write to.
\param
   pData:      Points to the data block where to get the data.
\param
   dataCount:  Number of data units (16 bit) to write

\return
   Number of written data units (16 bit)

*/
IFX_uint32_t MEI_WriteDma16Bit( MEI_MEI_DRV_CNTRL_T *pMeiDrvCntrl,
                                IFX_uint32_t          destAddr,
                                IFX_uint16_t          *pData,
                                IFX_uint32_t          dataCount,
                                IFX_int32_t           retryCnt)
{
   IFX_uint32_t count = 0;
   IFX_uint16_t *pTmpData;
   IFX_uint32_t tmpRegValue;

#if TRACE_MEI_MEI_ACCESS_FCT == 1
   PRN_DBG_USR_NL( MEI_REG, MEI_DRV_PRN_LEVEL_NORMAL,
      ("MEI[%02d]: Write(%d) DMA - addr = 0x%08X, count = %d\n\r",
      (IFX_uint32_t)pMeiDrvCntrl->dslLineNum, retryCnt, destAddr, dataCount) );
#endif

   pTmpData = pData;

   do {

      /* set DMA address */
      MEI_REG_ACCESS_ME_DX_AD_LONG_SET(pMeiDrvCntrl, destAddr);

      while (count < dataCount - (dataCount % 2))
      {
         tmpRegValue = (((pTmpData[count] << 16) & 0xFFFF0000) | (pTmpData[count+1] & 0x0000FFFF));

         tmpRegValue = SWAP32_DMA_WIDTH_ORDER(tmpRegValue);

         MEI_REG_ACCESS_ME_DX_DATA_SET(pMeiDrvCntrl, tmpRegValue);

         count += 2;
      }

      /* write unaligned block - last */
      if (count < dataCount)
      {
         tmpRegValue = ((pTmpData[count++] << 16) & 0xFFFF0000);

         tmpRegValue = SWAP32_DMA_WIDTH_ORDER(tmpRegValue);

         MEI_REG_ACCESS_ME_DX_DATA_SET(pMeiDrvCntrl, tmpRegValue);
      }

      /* check for DMA error */
      tmpRegValue = MEI_REG_ACCESS_ME_DX_STAT_GET(pMeiDrvCntrl);
      if (tmpRegValue & ME_DX_STAT_DX_ERR)
      {
         /* clear error flag */
         MEI_REG_ACCESS_ME_DX_STAT_SET(pMeiDrvCntrl, ME_DX_STAT_DX_ERR);
         PRN_ERR_INT_NL( MEI_MEI_ACCESS, MEI_DRV_PRN_LEVEL_WRN,
               ("MEI[%02d]: Write(%d) DMA ERROR - dest addr = 0x%08X, ME_DX_STAT = 0x%04X\n\r",
               (IFX_uint32_t)pMeiDrvCntrl->dslLineNum, retryCnt, destAddr, tmpRegValue) );

         count = (IFX_uint32_t)IFX_ERROR;
      }

#if (MEI_PROTECTED_MEI_DMA_ACCESS == 1)
      if (retryCnt > 0)
      {
         if (ret != (IFX_uint32_t)IFX_ERROR)
         {
            /* no previous DX error - check data */
            if ( (MEI_CmpDma(pMeiDrvCntrl, destAddr, pData, dataCount)) == IFX_SUCCESS)
            {
               retryCnt = 0;
            }
         }
      }
      retryCnt--;
#else
      retryCnt = -1;
#endif
   } while (retryCnt >= 0);

   if (count == (IFX_uint32_t)IFX_ERROR)
   {
      PRN_ERR_INT_NL( MEI_MEI_ACCESS, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI[%02d]: DMA ERROR - Write dest addr = 0x%08X, ME_DX_STAT = 0x%04X\n\r",
            (IFX_uint32_t)pMeiDrvCntrl->dslLineNum, destAddr, tmpRegValue) );
   }

   return count;
}


/**
   MEI DMA Read - read a data block from the MEI DMA interface.

\param
   pMeiDrvCntrl:   points to the MEI interface register set
\param
   srcAddr:    DMA read source address where read from.
\param
   pData:      Points to the data block where to put the data.
\param
   dataCount:  Number of data units (16 bit) to read.

\return
   Number of read data units (16 bit)
*/
IFX_uint32_t MEI_ReadDma16Bit(
                           MEI_MEI_DRV_CNTRL_T *pMeiDrvCntrl,
                           IFX_uint32_t          srcAddr,
                           IFX_uint16_t          *pData,
                           IFX_uint32_t          dataCount)
{
   IFX_uint32_t count = 0;
   IFX_uint32_t tmpRegValue;

#if TRACE_MEI_MEI_ACCESS_FCT == 1
      PRN_DBG_USR_NL( MEI_MEI_ACCESS, MEI_DRV_PRN_LEVEL_LOW,
            ("MEI[%02d]: Read DMA - addr = 0x%08X, count = %d" MEI_DRV_CRLF,
            (IFX_uint32_t)pMeiDrvCntrl->dslLineNum, srcAddr,  dataCount) );
#endif

   /* set DMA address */
   MEI_REG_ACCESS_ME_DX_AD_LONG_SET(pMeiDrvCntrl, srcAddr);


   while (count < dataCount)
   {
      tmpRegValue = MEI_REG_ACCESS_ME_DX_DATA_GET(pMeiDrvCntrl);

      tmpRegValue = SWAP32_DMA_WIDTH_ORDER(tmpRegValue);

      pData[count] = (tmpRegValue & 0xFFFF0000) >> 16;

      count++;

      /* high word */
      if (count < dataCount)
      {
         pData[count] = (tmpRegValue & 0x00000FFFF);
         count++;
      }
   }

   return count;
}


/**
   MEI DMA Read - read a data block from the MEI DMA interface.

   1. Read desired number of 16-bits words from the ME_DX_DATA register.
      - Keep the current ME_DX_ADDR_xx
      - ME_DX_ADDR_LO and ME_DX_ADDR_HI will be incremented to point to the
        next word address automatically after each read.

\param
   pMeiDrvCntrl:   points to the MEI interface register set
\param
   pData:      Points to the data block where to put the data.
\param
   dataCount:  Number of data units (16 bit) to read.

\return
   Number of read data units (16 bit)
*/
IFX_uint32_t MEI_GetDma(
                           MEI_MEI_DRV_CNTRL_T *pMeiDrvCntrl,
                           IFX_uint16_t        *pData,
                           IFX_uint32_t        dataCount)
{
   IFX_uint32_t count = 0, tmpRegValue;

   while (count < dataCount)
   {
      tmpRegValue = MEI_REG_ACCESS_ME_DX_DATA_GET(pMeiDrvCntrl);

      tmpRegValue = SWAP32_DMA_WIDTH_ORDER(tmpRegValue);

      pData[count] = (tmpRegValue & 0xFFFF0000) >> 16;

#if (TRACE_MEI_MEI_REG_ACCESS == 1)
      PRN_DBG_USR_NL( MEI_REG, MEI_DRV_PRN_LEVEL_NORMAL,
         ("MEI[%02d]: DMA Get[0x%X]: 0x%04X" MEI_DRV_CRLF,
         (IFX_uint32_t)pMeiDrvCntrl->dslLineNum, count<<1, (IFX_uint32_t)(*pData)));
#endif
      count++;

      /* high word */
      if (count < dataCount)
      {
         pData[count] = (tmpRegValue & 0x00000FFFF);

#if (TRACE_MEI_MEI_REG_ACCESS == 1)
         PRN_DBG_USR_NL( MEI_REG, MEI_DRV_PRN_LEVEL_NORMAL,
            ("MEI[%02d]: DMA Get[0x%X]: 0x%04X" MEI_DRV_CRLF,
            (IFX_uint32_t)pMeiDrvCntrl->dslLineNum, count<<1, (IFX_uint32_t)(*pData)));
#endif
         count++;
      }
   }

#if (TRACE_MEI_MEI_ACCESS_FCT == 1)
   PRN_DBG_USR_NL( MEI_REG, MEI_DRV_PRN_LEVEL_LOW,
          ("MEI[%02d]: Get DMA done - count = %d" MEI_DRV_CRLF,
          (IFX_uint32_t)pMeiDrvCntrl->dslLineNum, count));
#endif

   return count;
}



#if (MEI_PROTECTED_MEI_DMA_ACCESS == 1)
/**
   Compare mem blocks.
*/
static IFX_uint32_t MEI_CmpDma(
                         MEI_MEI_DRV_CNTRL_T *pMeiDrvCntrl,
                         IFX_uint32_t         destAddr,
                         IFX_uint16_t         *pData,
                         IFX_uint32_t         dataCount)
{
   IFX_uint32_t nRet = IFX_SUCCESS;
   IFX_uint32_t count;
   IFX_uint16_t tmpValue[2];
   IFX_uint32_t *pTmpValue, dxStatReg;

   pTmpValue = (IFX_uint32_t*)tmpValue;

   /* set DMA address */
   MEI_REG_ACCESS_ME_DX_AD_LONG_SET(pMeiDrvCntrl, destAddr);

   while (count < dataCount)
   {
      *pTmpValue = MEI_REG_ACCESS_ME_DX_DATA_GET(pMeiDrvCntrl);

      *pTmpValue = SWAP32_DMA_WIDTH_ORDER(*pTmpValue);

      if (tmpValue[0] != pData[count] )
      {
         dxStatReg = MEI_REG_ACCESS_ME_DX_STAT_GET(pMeiDrvCntrl);

         nRet = IFX_ERROR;
         break;
      }
      count++;

      if (count < dataCount)
      {
         if (tmpValue[1] != pData[count] )
         {
            dxStatReg = MEI_REG_ACCESS_ME_DX_STAT_GET(pMeiDrvCntrl);

            nRet = IFX_ERROR;
            break;
         }
         count++;
      }
   }

   if (nRet != IFX_SUCCESS)
   {
      PRN_ERR_INT_NL( MEI_MEI_ACCESS, MEI_DRV_PRN_LEVEL_ERR,
             ("MEI[%02d]: Cmp DMA ERROR - "
              "dAddr = 0x%08X, (hData[%d]= 0x%04X) != (mData= 0x%04X), DXStat = 0x%04X" MEI_DRV_CRLF,
              (IFX_uint32_t)pMeiDrvCntrl->dslLineNum, destAddr, count,
              pData[count], tmpValue, dxStatReg) );

      if (dxStatReg & ME_DX_STAT_DX_ERR)
      {
         MEI_REG_ACCESS_ME_DX_STAT_SET(pMeiDrvCntrl, ME_DX_STAT_DX_ERR);
      }
      return nRet;
   }

   dxStatReg = MEI_REG_ACCESS_ME_DX_STAT_GET(pMeiDrvCntrl);
   if (dxStatReg & ME_DX_STAT_DX_ERR)
   {
      PRN_ERR_INT_NL( MEI_MEI_ACCESS, MEI_DRV_PRN_LEVEL_ERR,
             ("MEI[%02d]: Cmp DMA Error - "
              "dAddr = 0x%08X, count = %d, DXStat = 0x%04X" MEI_DRV_CRLF,
              (IFX_uint32_t)pMeiDrvCntrl->dslLineNum, destAddr, count, dxStatReg) );

      MEI_REG_ACCESS_ME_DX_STAT_SET(pMeiDrvCntrl, ME_DX_STAT_DX_ERR);
      return IFX_ERROR;
   }

   PRN_DBG_INT_NL( MEI_REG, MEI_DRV_PRN_LEVEL_LOW,
          ("MEI[%02d]: Cmp DMA success - dAddr = 0x%08X, count = %d, DXStat = 0x%04X" MEI_DRV_CRLF,
          (IFX_uint32_t)pMeiDrvCntrl->dslLineNum, destAddr, count, dxStatReg) );

   return nRet;
}
#endif /* #if (MEI_PROTECTED_MEI_DMA_ACCESS == 1) */


/**
   MEI MailBox - write a message to the MEI MailBox interface.
   When the ME has a CMV message to send to the ARC, the ME will perform
   the following steps:
   1. The ME waits for the processing of any previous Mailbox Messaging to be
      complete by insuring that
      (i) a response was received (or time-out occurred) following
          the previous query, and
      (ii) the ME_MSGAV  bit in ME_ME2ARC_STAT register is 0.
   2. The ME transfers the message to the MEI-to-ARC Mailbox buffer via DMA
   3. The ME sets the ME_MSGAV bit in the ME_ME2ARC_INT register to notify the
      ARC that a message is available.
   4. Check ME_DX_STAT register to see that no Data Transfer errors have occurred.

   NOTE:
   The ARC4 will either poll its interrupt registers or take the corresponding
   interrupt. Upon receipt of the message, the ARC4 will clear its bit in
   its local interrupt register and thereby clear the corresponding bit
   in the ME_ME2ARC_STAT register.

\param
   pMeiDrvCntrl:   points to the MEI interface register set
\param
   pMeiMbBlk:  Points to the Mailbox block (message + mailboc code).
\param
   mbCount:    MEI Mailbox unit count (16 bit)

\return
   Number of 16bit units bytes.
*/
IFX_int32_t MEI_WriteMailBox(
                           MEI_MEI_DRV_CNTRL_T *pMeiDrvCntrl,
                           IFX_uint32_t          mbAddr,
                           MEI_MEI_MAILBOX_T   *pMeiMbBlk,
                           IFX_uint32_t          mbCount)
{
#if TRACE_MEI_MEI_ACCESS_FCT == 1
      PRN_DBG_USR_NL( MEI_REG, MEI_DRV_PRN_LEVEL_LOW,
            ("MEI[%02d]: Write Mailbox - count = %d, CmvOpCode = 0x%04X" MEI_DRV_CRLF,
            (IFX_uint32_t)pMeiDrvCntrl->dslLineNum, mbCount, pMeiMbBlk->mbRaw.rawMsg[0]) );
#endif

   if ( MEI_MAILBOX_BUSY_FOR_WR(pMeiDrvCntrl) )
   {
      PRN_ERR_USR_NL( MEI_MEI_ACCESS, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI[0x%08X] ERROR: WR Mbox - target BUSY" MEI_DRV_CRLF,
            (IFX_uint32_t)pMeiDrvCntrl->dslLineNum) );
      return IFX_ERROR;
   }

   /* write mailbox message via DMA */
   if (mbCount != ( MEI_WriteDma16Bit(
                     pMeiDrvCntrl, mbAddr,
                     (IFX_uint16_t *)pMeiMbBlk, mbCount,
                     MEI_PROTECTED_MEI_DMA_RETRY_CNT_MSG  /* enable check and retry count */ ) ) )
   {
      PRN_ERR_USR_NL( MEI_MEI_ACCESS, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI[0x%08X] ERROR: WR Mbox - DMA transfer" MEI_DRV_CRLF,
            (IFX_uint32_t)pMeiDrvCntrl->dslLineNum) );

      return IFX_ERROR;
   }

   /* inform the target system */
   MEI_MAILBOX_NFC_NEW_MSG(pMeiDrvCntrl);

   return (mbCount);
}

/**
   MEI MailBox - set the Mailbox address and read the Mailbox Code.
\param
   pMeiDrvCntrl:   points to the MEI interface register set

\return
   mailbox code

\attention
   - called on int-level
*/
IFX_uint16_t MEI_GetMailBoxCode(
                           MEI_MEI_DRV_CNTRL_T *pMeiDrvCntrl,
                           IFX_uint32_t          mbAddr)
{
   IFX_uint32_t tmpRegVal = 0;
   IFX_uint16_t mbCode;

#if (TRACE_MEI_MEI_ACCESS_FCT == 1)
      PRN_DBG_USR_NL( MEI_REG, MEI_DRV_PRN_LEVEL_LOW,
            ("MEI[%02d]: MEI_GetMailBoxCode - ARC2ME addr = 0x%08X" MEI_DRV_CRLF,
            (IFX_uint32_t)pMeiDrvCntrl->dslLineNum, mbAddr) );
#endif

   /* set DMA address */
   MEI_REG_ACCESS_ME_DX_AD_LONG_SET(pMeiDrvCntrl, (mbAddr + 0x2C));

   /* Get Message Type and Control Information*/
   tmpRegVal = MEI_REG_ACCESS_ME_DX_DATA_GET(pMeiDrvCntrl);

   if(tmpRegVal & 0x80000000)
   {
      PRN_ERR_INT_NL( MEI_MEI_ACCESS, MEI_DRV_PRN_LEVEL_LOW,
          ("MEI[0x%02X]: Code Swap Request received (UNHANDLED)" MEI_DRV_CRLF,
          (IFX_uint32_t)pMeiDrvCntrl->dslLineNum) );
      mbCode = 0xFFFF;
   }
   else if (tmpRegVal & 0x00020000)
   {
      PRN_ERR_INT_NL( MEI_MEI_ACCESS, MEI_DRV_PRN_LEVEL_LOW,
          ("MEI[0x%02X]: Clear EOC Request received (UNHANDLED)" MEI_DRV_CRLF,
          (IFX_uint32_t)pMeiDrvCntrl->dslLineNum) );
      mbCode = 0xFFFF;
   }
   else if (tmpRegVal & 0x00040000)
   {
      PRN_ERR_INT_NL( MEI_MEI_ACCESS, MEI_DRV_PRN_LEVEL_LOW,
          ("MEI[0x%02X]: Reboot Request received (UNHANDLED)" MEI_DRV_CRLF,
          (IFX_uint32_t)pMeiDrvCntrl->dslLineNum) );
      mbCode = 0xFFFF;
   }
   else
   {
      PRN_ERR_INT_NL( MEI_MEI_ACCESS, MEI_DRV_PRN_LEVEL_LOW,
          ("MEI[0x%02X]: CMV Mailbox Message received" MEI_DRV_CRLF,
          (IFX_uint32_t)pMeiDrvCntrl->dslLineNum) );

      /* set DMA address */
      MEI_REG_ACCESS_ME_DX_AD_LONG_SET(pMeiDrvCntrl, mbAddr);

      tmpRegVal = MEI_REG_ACCESS_ME_DX_DATA_GET(pMeiDrvCntrl);
      tmpRegVal = SWAP32_DMA_WIDTH_ORDER(tmpRegVal);

      mbCode    = (tmpRegVal & 0xFFFF0000) >> 16;

      if (mbCode == 0x0F11)
      {
         /* READY message. Treat it as EVT */
         mbCode = MEI_MBOX_CODE_EVT_REQ;
      }
      else
      {
         mbCode = MEI_MBOX_CODE_MSG_ACK;
      }
   }

   /* return the mailbox code */
   return mbCode;
}

/**
   MEI MailBox - read a message from the MEI MailBox interface.

\param
   pMeiDrvCntrl:   points to the MEI interface register set
\param
   pMeiMbBlk:  Points to the Mailbox block.
\param
   mbCount:    MEI Mailbox unit count (16 bit)

\return
   msg size (16 bit) if success.
   IFX_ERROR in case of error.

\remark
   The function returns the 16bit units of the complete message
      + header ( inlucdes the previous read mailbox code) [16 bit units]
      + payload [16 bit units]

\attention
   - called on int-level
*/
IFX_int32_t MEI_GetMailBoxMsg(
                           MEI_MEI_DRV_CNTRL_T *pMeiDrvCntrl,
                           IFX_uint32_t          mbAddr,
                           MEI_MEI_MAILBOX_T   *pMeiMbBlk,
                           IFX_uint32_t          mbCount,
                           IFX_boolean_t         releaseMb)
{
   AR9_CMV_STD_MESSAGE_T *pTmpRead = (AR9_CMV_STD_MESSAGE_T *) (&pMeiMbBlk->mbCmv.cmv);
   IFX_uint32_t payloadSize_16bit;
   IFX_uint16_t tmpRegValue = 0;

   /* Read Header - check for enough buffer */
   if ( mbCount < AR9_CMV_HEADER_16BIT_SIZE)
   {
      PRN_ERR_INT_NL( MEI_MEI_ACCESS, MEI_DRV_PRN_LEVEL_ERR,
          ("MEI[0x%08X] ERROR: MEI_GetMailBoxMsg - buffer[%d] to small for header" MEI_DRV_CRLF,
            (IFX_uint32_t)pMeiDrvCntrl->dslLineNum, mbCount) );

      goto MEI_GET_MAILBOX_MSG_ERROR;
   }

   /* set DMA address */
   MEI_REG_ACCESS_ME_DX_AD_LONG_SET(pMeiDrvCntrl, mbAddr);

   /* read msg header */
   MEI_GetDma( pMeiDrvCntrl, &pMeiMbBlk->mbRaw.rawMsg[0], AR9_CMV_HEADER_16BIT_SIZE );

   /* size field contains number of 16 bit payload elements of the message */
   payloadSize_16bit = P_AR9_CMV_MSGHDR_PAYLOAD_SIZE_GET(pTmpRead);

   /* check the payload len */
   if ( mbCount < ((AR9_CMV_HEADER_16BIT_SIZE) + payloadSize_16bit) )
   {
      /* invalid msg size */
      PRN_ERR_INT_NL( MEI_MEI_ACCESS, MEI_DRV_PRN_LEVEL_ERR,
           ("MEI[0x%02X] ERROR: MEI_GetMailBoxMsg - buffer[%d] to small for msg[%d]" MEI_DRV_CRLF,
            (IFX_uint32_t)pMeiDrvCntrl->dslLineNum, mbCount,
            (AR9_CMV_HEADER_16BIT_SIZE + payloadSize_16bit) ));

      goto MEI_GET_MAILBOX_MSG_ERROR;
   }

   /* Get Payload*/
   MEI_GetDma( pMeiDrvCntrl,
               pTmpRead->payload.params_16Bit, payloadSize_16bit);

   /* check DMA read action */
   tmpRegValue = MEI_REG_ACCESS_ME_DX_STAT_GET(pMeiDrvCntrl);
   if (tmpRegValue & ME_DX_STAT_DX_ERR)
   {
      /* clear error flag */
      MEI_REG_ACCESS_ME_DX_STAT_SET(pMeiDrvCntrl, ME_DX_STAT_DX_ERR);
      goto MEI_GET_MAILBOX_MSG_ERROR;
   }

   /* Release mailbox - msg read done */
   if (releaseMb == IFX_TRUE)
   {
      MEI_MAILBOX_MSG_READ_DONE(pMeiDrvCntrl);
   }

#if TRACE_MEI_MEI_ACCESS_FCT == 1
   PRN_DBG_USR_NL( MEI_REG, MEI_DRV_PRN_LEVEL_NORMAL,
          ("MEI[%02d]: MEI_GetMailBoxMsg done - count %d [16 bit]" MEI_DRV_CRLF,
           (IFX_uint32_t)pMeiDrvCntrl->dslLineNum,
           (AR9_CMV_HEADER_16BIT_SIZE + payloadSize_16bit) ));
#endif

   /* header + payload */
   return (AR9_CMV_HEADER_16BIT_SIZE + payloadSize_16bit);

MEI_GET_MAILBOX_MSG_ERROR:

   /* Release mailbox - msg read done */
   if (releaseMb == IFX_TRUE)
   {
      MEI_MAILBOX_MSG_READ_DONE(pMeiDrvCntrl);
   }

   PRN_ERR_INT_NL( MEI_MEI_ACCESS, MEI_DRV_PRN_LEVEL_ERR,
        ("MEI[%02d]: MEI_GetMailBoxMsg ERROR done - ARC2ME_STAT = 0x%04X, ME_DX_STAT = 0x%04X" MEI_DRV_CRLF,
          (IFX_uint32_t)pMeiDrvCntrl->dslLineNum,
          MEI_REG_ACCESS_ME_ARC2ME_STAT_GET(pMeiDrvCntrl), tmpRegValue ));

   return IFX_ERROR;
}

/**
   Release / free the MEI mailbox .

\param
   pMeiDrvCntrl:   points to the MEI interface register set.

\return
   - TRUE: int was enabled

\attention
   - Called on int-level.

*/
IFX_boolean_t MEI_ReleaseMailboxMsg(
                           MEI_MEI_DRV_CNTRL_T *pMeiDrvCntrl)
{

   /* Release mailbox - msg read done */
   MEI_MAILBOX_MSG_READ_DONE(pMeiDrvCntrl);

#if (TRACE_MEI_MEI_ACCESS_FCT == 1)
   PRN_DBG_USR_NL( MEI_REG, MEI_DRV_PRN_LEVEL_LOW,
      ("MEI[%02d]: MEI_ReleaseMailboxMsg done - ARC2ME_STAT = 0x%04X" MEI_DRV_CRLF,
      (IFX_uint32_t)pMeiDrvCntrl->dslLineNum,
      MEI_REG_ACCESS_ME_ARC2ME_STAT_GET(pMeiDrvCntrl) ));
#endif

   return IFX_TRUE;
}


/**
   Return a MEI Register via the 16Bit offset.
   - set when the ROM code is entered.

\param
   pMeiDrvCntrl:   points to the MEI interface register set
\param
   offset      :  offset (32 Bit) within the MEI register interface.

\return
   register value.
*/
MEI_MeiRegVal_t MEI_RegAccOffGet(
                           MEI_MEI_DRV_CNTRL_T *pMeiDrvCntrl,
                           IFX_uint8_t         offset )
{
   return (MEI_MeiRegVal_t)(MEI_REG_ACCESS_OFFSET_GET(pMeiDrvCntrl, offset));
}


/**
   Return a MEI Register via the 16Bit offset.
   - set when the ROM code is entered.

\param
   pMeiDrvCntrl:   points to the MEI interface register set
\param
   offset      :  offset (32 Bit) within the MEI register interface.
\param
   regValue:      new register value to set.
\return
   none.
*/
IFX_void_t MEI_RegAccOffSet(
                           MEI_MEI_DRV_CNTRL_T *pMeiDrvCntrl,
                           IFX_uint8_t          offset,
                           MEI_MeiRegVal_t      regValue)
{

   MEI_REG_ACCESS_OFFSET_SET(pMeiDrvCntrl, offset, regValue);

   return;
}

/**
   Detect the MEI Register interface block.

\param
   pMeiDrvCntrl:   points to the MEI interface register set.

\return
   IFX_SUCCESS in case of success (interface up) else
   IFX_ERROR   in case of en error (Interface DOWN).
*/
IFX_int32_t MEI_InterfaceDetect(
                           MEI_MEI_DRV_CNTRL_T *pMeiDrvCntrl)
{
   IFX_uint32_t hwVers = 0xFFFFFFFF;

   if (pMeiDrvCntrl->pMeiIfCntrl->pVirtMeiRegIf)
   {
      MEI_REG_ACCESS_ME_VERSION_SET(pMeiDrvCntrl, 0x00000000);
      hwVers = MEI_REG_ACCESS_ME_VERSION_GET(pMeiDrvCntrl);

      if (hwVers == 0x00000003)
      {
         return IFX_SUCCESS;
      }
   }

   return IFX_ERROR;
}

#if (MEI_SUPPORT_MEI_DEBUG == 1)

/**
   Display the MEI Register (for test purposals).

\param
   pMeiDrvCntrl:   points to the MEI interface register set

\return
   NONE
*/
IFX_void_t MEI_ShowMeiRegs(
                           MEI_MEI_DRV_CNTRL_T *pMeiDrvCntrl,
                           IFX_uint8_t          devNum)
{
   int meiIdx = 0;
   int maxMeiIdx = (sizeof(MEI_MEI_REG_IF_U)/MEI_MEI_REGISTER_WIDTH);

   while (meiIdx < maxMeiIdx)
   {
      MEI_PRINT_USR("MEI[%02d] 0x%02X: 0x%08X  0x%08X" MEI_DRV_CRLF,
         devNum, meiIdx*MEI_MEI_REGISTER_WIDTH,
         ( ( meiIdx    < maxMeiIdx) ? MEI_REG_ACCESS_OFFSET_GET(pMeiDrvCntrl,  meiIdx   ) : 0xA5A5A5A5 ),
         ( ((meiIdx+1) < maxMeiIdx) ? MEI_REG_ACCESS_OFFSET_GET(pMeiDrvCntrl, (meiIdx+1)) : 0x5A5A5A5A ) );
      meiIdx +=2;
   }
}


/**
   Stress test to the MEI register.
   - write 0xA5A5 pattern
   - write 0x5A5A pattern
   - read back and check
*/
IFX_boolean_t MEI_DmaTest(
                           MEI_MEI_DRV_CNTRL_T *pMeiDrvCntrl,
                           IFX_uint32_t         destAddr,
                           IFX_uint32_t         dma_count,
                           IFX_uint32_t         test_count)
{
   IFX_uint32_t i, addr, count = test_count;
   IFX_uint32_t result = 0;

   while (count > 0)
   {
      /* set DMA address */
      addr = destAddr;

      for (i=0; i < dma_count; i++)
      {
         /* write pattern and check */
         MEI_REG_ACCESS_ME_DX_AD_LONG_SET(pMeiDrvCntrl, addr);
         MEI_REG_ACCESS_ME_DX_DATA_SET(pMeiDrvCntrl, 0xA5A5A5A5);

         MEI_REG_ACCESS_ME_DX_AD_LONG_SET(pMeiDrvCntrl, addr);
         MEI_REG_ACCESS_ME_DX_DATA_SET(pMeiDrvCntrl, 0x5A5A5A5A);

         MEI_REG_ACCESS_ME_DX_AD_LONG_SET(pMeiDrvCntrl, addr);
         if ( (result = MEI_REG_ACCESS_ME_DX_DATA_GET(pMeiDrvCntrl)) != 0x5A5A5A5A )
         {
            /* error */
            count = 0;

            /* check for DMA error */
            if ( MEI_MAILBOX_WRITE_ERROR(pMeiDrvCntrl) )
            {
               PRN_DBG_USR_NL( MEI_MEI_ACCESS, MEI_DRV_PRN_LEVEL_HIGH,
                      ( "MEI[%02d]: !!! internal DMX ERROR !!!!" MEI_DRV_CRLF
                        "TestLoop: %d, DMA addr: 0x%08X, readback: 0x%04X, expect: 0x5A5A" MEI_DRV_CRLF,
                        (IFX_uint32_t)pMeiDrvCntrl->dslLineNum, count,
                        addr, result));
            }
            else
            {
               PRN_DBG_USR_NL( MEI_MEI_ACCESS, MEI_DRV_PRN_LEVEL_HIGH,
                      ( "MEI[%02d]: !!! ERROR DMA test !!!!" MEI_DRV_CRLF
                        "TestLoop: %d, DMA addr: 0x%08X, readback: 0x%04X, expect: 0x5A5A" MEI_DRV_CRLF,
                        (IFX_uint32_t)pMeiDrvCntrl->dslLineNum, count,
                        addr, result));
            }
            return IFX_FALSE;
         }
         addr += 4;
      }

      /* check for DMA error */
      if ( MEI_MAILBOX_WRITE_ERROR(pMeiDrvCntrl) )
      {
         PRN_ERR_USR_NL( MEI_MEI_ACCESS, MEI_DRV_PRN_LEVEL_ERR,
                ( "MEI[%02d]: !!! ERROR AR9 DMA transfer - TestLoop: %d!!!!" MEI_DRV_CRLF,
                  (IFX_uint32_t)pMeiDrvCntrl->dslLineNum, count));

         return IFX_FALSE;
      }

      PRN_DBG_USR_NL( MEI_MEI_ACCESS, MEI_DRV_PRN_LEVEL_LOW,
             ( "MEI[%02d]: DMA test LOOP %d" MEI_DRV_CRLF,
               (IFX_uint32_t)pMeiDrvCntrl->dslLineNum, count));

      count-- ;
   }

   PRN_ERR_USR_NL( MEI_MEI_ACCESS, MEI_DRV_PRN_LEVEL_ERR,
          ( "MEI[%02d]: DMA test done addr 0x%08X, range 0x%08X, loops %d" MEI_DRV_CRLF,
            (IFX_uint32_t)pMeiDrvCntrl->dslLineNum, destAddr, dma_count, test_count));

   return IFX_TRUE;
}

#endif      /* #ifdef MEI_SUPPORT_MEI_DEBUG */

IFX_int32_t MEI_LowLevelInit(MEI_MEI_DRV_CNTRL_T *pMeiDrvCntrl)
{

   IFX_uint32_t arc_cri_ccr0 = 0;

   /* Get CRI_CCR0*/
   if (MEI_ReadDbg(pMeiDrvCntrl, 0x31F00, 0x1, 1, &arc_cri_ccr0) != 1)
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
         ("MEI_DRV: ARC_STATUS32 read failed!" MEI_DRV_CRLF));
      return (IFX_int32_t)IFX_ERROR;
   }

   /* enable ac_clk signal*/
   arc_cri_ccr0 |= (1 << 4);

   /* Set CRI_CCR0*/
   if (MEI_WriteDbg(pMeiDrvCntrl, 0x31F00, 0x1, 1, &arc_cri_ccr0) != 1)
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
         ("MEI_DRV: CRI_CCR0 write failed!" MEI_DRV_CRLF));
      return (IFX_int32_t)IFX_ERROR;
   }

   return IFX_SUCCESS;
}

#endif      /* #if (MEI_SUPPORT_DEVICE_AR9 == 1)*/

