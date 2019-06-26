/******************************************************************************

                          Copyright (c) 2007-2015
                     Lantiq Beteiligungs-GmbH & Co. KG

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/* ==========================================================================
   Description : Debug and DMA access to the VRX Device
   ========================================================================== */

/* ============================================================================
   Inlcudes
   ========================================================================= */

/* get at first the driver configuration */
#include "drv_mei_cpe_config.h"

#include "ifx_types.h"
#include "drv_mei_cpe_os.h"
#include "drv_mei_cpe_dbg.h"

#include "drv_mei_cpe_dbg_driver.h"

#include "drv_mei_cpe_interface.h"
#include "drv_mei_cpe_mei_interface.h"
#include "drv_mei_cpe_api.h"

#include "drv_mei_cpe_mei_access.h"
#include "drv_mei_cpe_dbg_access.h"
#include "drv_mei_cpe_device_cntrl.h"
#include "drv_mei_cpe_msg_process.h"


#if (MEI_SUPPORT_DFE_GPA_ACCESS == 1)
#include "cmv_message_format.h"

#if (MEI_SUPPORT_ROM_CODE == 1)
#include "drv_mei_cpe_rom_handler.h"
#endif

#if (MEI_SUPPORT_DRV_MODEM_TESTS == 1)

#endif


#endif      /* #if (MEI_SUPPORT_DFE_GPA_ACCESS == 1) */


/* ============================================================================
   Local function declaration
   ========================================================================= */

#if (MEI_SUPPORT_DFE_DMA_ACCESS == 1)

static IFX_uint32_t MEI_MeiDmaWrite( MEI_DYN_CNTRL_T *pMeiDynCntrl,
                                       IFX_uint32_t      destAddr,
                                       IFX_uint32_t      dataCount_32,
                                       IFX_uint32_t      *pData_32);

static IFX_uint32_t MEI_MeiDmaRead( MEI_DYN_CNTRL_T *pMeiDynCntrl,
                                      IFX_uint32_t      srcAddr,
                                      IFX_uint32_t      dataCount_32,
                                      IFX_uint32_t      *pData_32);

#endif      /* #if (MEI_SUPPORT_DFE_DMA_ACCESS == 1) */


#if (MEI_SUPPORT_DFE_DBG_ACCESS == 1)

static IFX_int32_t MEI_MeiDbgWrite( MEI_DYN_CNTRL_T *pMeiDynCntrl,
                                          IFX_uint32_t  destAddr,
                                          IFX_uint32_t  dbgDest,
                                          IFX_uint32_t  dataCount,
                                          IFX_uint32_t  *pData);

static IFX_int32_t MEI_MeiDbgRead( MEI_DYN_CNTRL_T *pMeiDynCntrl,
                                         IFX_uint32_t  srcAddr,
                                         IFX_uint32_t  dbgSrc,
                                         IFX_uint32_t  dataCount,
                                         IFX_uint32_t  *pData);

#endif      /* #if (MEI_SUPPORT_DFE_DBG_ACCESS == 1) */



/* ============================================================================
   VRX Driver Debug function - Definitions
   ========================================================================= */

#if (MEI_SUPPORT_MEI_DEBUG == 1)
/**
   Display the MEI Register
*/
IFX_void_t MEI_MeiRegsShow(MEI_DEV_T *pMeiDev)
{
   PRN_DBG_USR( MEI_DRV, MEI_DRV_PRN_LEVEL_HIGH, MEI_DRV_LINENUM_GET(pMeiDev),
         ("MEI_DRV[%02d]: show MEI Register Set[0x%08X]:" MEI_DRV_CRLF,
         MEI_DRV_LINENUM_GET(pMeiDev), (IFX_uint32_t)MEI_DRV_MEI_PHY_ADDR_GET(pMeiDev)) );

   MEI_ShowMeiRegs(&pMeiDev->meiDrvCntrl, MEI_DRV_LINENUM_GET(pMeiDev));

   PRN_DBG_USR( MEI_DRV, MEI_DRV_PRN_LEVEL_HIGH, MEI_DRV_LINENUM_GET(pMeiDev),
         ("MEI_DRV[%02d]: ===================================" MEI_DRV_CRLF,
         MEI_DRV_LINENUM_GET(pMeiDev)) );
}


/**
   Display the internal driver buffer (DMA / mailbox).

\attention
   Removed in this driver version
*/
IFX_void_t MEI_ShowDrvBuffer(
                              MEI_DYN_CNTRL_T *pMeiDynCntrl,
                              IFX_int8_t bufNum, IFX_int32_t count)
{
   MEI_DRV_PRN_LOCAL_DBG_VAR_CREATE(MEI_DEV_T *, pMeiDev, pMeiDynCntrl->pMeiDev);

#if (MEI_DEBUG_PRINT == 1)
   PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
         ("MEI_DRV[%02d] this feature is not supported anymore in this driver version" MEI_DRV_CRLF,
          MEI_DRV_LINENUM_GET(pMeiDev)) );
#endif /* #if (MEI_DEBUG_PRINT == 1)*/
   return;
}

#endif      /* MEI_SUPPORT_MEI_DEBUG */


/* ============================================================================
   MEI Register function definition
   ========================================================================= */
#if (MEI_SUPPORT_REGISTER == 1)
/**
   Write to hardware register.
   The physical base address has to be set before. The

\param
   pDev    device structure
\param
   offset  register offset (byte)
\param
   val     value to be set

\return
   IFX_SUCCESS    Success
   IFX_ERROR      in case of error
*/
IFX_int32_t MEI_Set_Register(MEI_DEV_T *pMeiDev, IFX_uint32_t offset, IFX_uint32_t val)
{
   PRN_DBG_USR( MEI_DRV, MEI_DRV_PRN_LEVEL_LOW, MEI_DRV_LINENUM_GET(pMeiDev),
       ("MEI_DRV[%02d]: Set register[0x%08x + 0x%0x] = %x." MEI_DRV_CRLF,
         MEI_DRV_LINENUM_GET(pMeiDev), (IFX_uint32_t)MEI_DRV_MEI_PHY_ADDR_GET(pMeiDev),
         (IFX_uint32_t)offset, (IFX_uint32_t)val) );

   if ( MEI_SetMeiReg( &pMeiDev->meiDrvCntrl,
                       (IFX_uint8_t)offset,
                       (MEI_MeiRegVal_t)val )
        == IFX_FALSE )
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
          ("MEI_DRV[%02d]: Error Set register[0x%08x + 0x%0x] = 0x%x." MEI_DRV_CRLF,
            MEI_DRV_LINENUM_GET(pMeiDev),
            (IFX_uint32_t)MEI_DRV_MEI_PHY_ADDR_GET(pMeiDev),
            (IFX_uint32_t)offset,
            (IFX_uint32_t)val) );

      return IFX_ERROR;
   }

   return IFX_SUCCESS;
}


/**
   Read hardware register. The physical base address has to be set before.

\param
   pMeiDev  device structure
\param
   off_byte register offset (byte)
\param
   pVal     points to the return field where the value has to be set

\return
   IFX_SUCCESS Success
   IFX_ERROR   in case of error
*/
IFX_int32_t MEI_Get_Register(MEI_DEV_T *pMeiDev, IFX_uint32_t off_byte, IFX_uint32_t *pVal)
{
   MEI_MeiRegVal_t tempVal;

   if ( MEI_GetMeiReg( &pMeiDev->meiDrvCntrl,
                       (IFX_uint8_t)off_byte,
                       &tempVal )
        == IFX_FALSE )
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
          ("MEI_DRV[%02d]: Error get register[0x%08x + 0x%0x]" MEI_DRV_CRLF,
            MEI_DRV_LINENUM_GET(pMeiDev),
            (IFX_uint32_t)MEI_DRV_MEI_PHY_ADDR_GET(pMeiDev),
            off_byte) );

      return IFX_ERROR;
   }

   *pVal = (IFX_uint32_t)tempVal;

   PRN_DBG_USR( MEI_DRV, MEI_DRV_PRN_LEVEL_LOW, MEI_DRV_LINENUM_GET(pMeiDev),
       ("MEI_DRV[%02d]: get register[0x%08x + 0x%0x] = %x." MEI_DRV_CRLF,
         MEI_DRV_LINENUM_GET(pMeiDev),
         (IFX_uint32_t)MEI_DRV_MEI_PHY_ADDR_GET(pMeiDev), off_byte, *pVal) );

   return IFX_SUCCESS;

}

#endif /* MEI_SUPPORT_REGISTER */


/* ============================================================================
   DMA function definition
   ========================================================================= */
#if (MEI_SUPPORT_DFE_DMA_ACCESS == 1)

/**
   MEI DMA test.
   - Write to the given destination number of units.
*/
IFX_int32_t MEI_MeiDmaTest( MEI_DEV_T *pMeiDev,
                                  IFX_uint32_t destAddr,
                                  IFX_uint32_t dma_count,
                                  IFX_uint32_t test_count)
{
   IFX_boolean_t ret;

   PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_HIGH,
          ( "MEI_DRV[%02d]: === Start DMA test ==========================" MEI_DRV_CRLF
            "DMA addr: 0x%08X, DMA Range: 0x%X, Loops: %d" MEI_DRV_CRLF,
            MEI_DRV_LINENUM_GET(pMeiDev), destAddr, dma_count, test_count));

   ret = MEI_DmaTest(&pMeiDev->meiDrvCntrl, destAddr, dma_count, test_count);

   PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_HIGH,
          ( "MEI_DRV[%02d]: === End DMA test ============================" MEI_DRV_CRLF,
            MEI_DRV_LINENUM_GET(pMeiDev)) );
   return (ret == IFX_TRUE) ? IFX_SUCCESS : IFX_ERROR;
}


/**
   Write to hardware via MEI DMA access.

\param
   pMeiDynCntrl dynamic control structure
\param
   destAddr       DMA destination address within the target system
\param
   dataCount_32   Number of units to write (32 bit).
\param
   pData_32       points to the data to write.

\return
   Success:    number of written units.
*/
static IFX_uint32_t MEI_MeiDmaWrite(
                              MEI_DYN_CNTRL_T *pMeiDynCntrl,
                              IFX_uint32_t      destAddr,
                              IFX_uint32_t      dataCount_32,
                              IFX_uint32_t      *pData_32)
{
   IFX_uint32_t ret, i;
   MEI_DEV_T *pMeiDev = pMeiDynCntrl->pMeiDev;

   /* swap data */
   for (i = 0; i < dataCount_32; i++)
   {
      pData_32[i] = SWAP32_DMA_WIDTH_ORDER(pData_32[i]);
   }

   /* protect against interrupt driven MailBox read */
   MEI_DRV_GET_UNIQUE_MAILBOX_ACCESS(pMeiDev);

   ret = MEI_WriteDma16Bit( &pMeiDev->meiDrvCntrl, destAddr,
                       (IFX_uint16_t *)pData_32, dataCount_32 * 2,
                       MEI_PROTECTED_MEI_DMA_RETRY_CNT_MISC /* disable check no retry */ );


   MEI_DRV_RELEASE_UNIQUE_MAILBOX_ACCESS(pMeiDev);

   if (ret != dataCount_32 * 2)
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
             ("MEI_DRV[%02d - %02d] MEI DMA Write Error" MEI_DRV_CRLF,
             MEI_DRV_LINENUM_GET(pMeiDev), pMeiDynCntrl->openInstance));
   }
   else
   {
      ret /= 2;
      PRN_DBG_USR( MEI_DRV, MEI_DRV_PRN_LEVEL_LOW, MEI_DRV_LINENUM_GET(pMeiDev),
          ("MEI_DRV[%02d - %02d] MEI DMA Write - %d unit[32] written" MEI_DRV_CRLF,
           MEI_DRV_LINENUM_GET(pMeiDev), pMeiDynCntrl->openInstance, ret));
   }

   return ret;
}

/**
   Write to hardware via MEI DMA access.

\param
   pMeiDynCntrl dynamic control structure
\param
   srcAddr        DMA source address within the target system
\param
   dataCount_32   Number of units to write (32 bit).
\param
   pData_32       points to the buffer where the data has to read.

\return
   Success:    number of read units.
*/
static IFX_uint32_t MEI_MeiDmaRead(
                              MEI_DYN_CNTRL_T *pMeiDynCntrl,
                              IFX_uint32_t      srcAddr,
                              IFX_uint32_t      dataCount_32,
                              IFX_uint32_t      *pData_32)
{
   IFX_uint32_t ret, i;
   MEI_DEV_T *pMeiDev = pMeiDynCntrl->pMeiDev;

   /* protect against interrupt driven MailBox read */
   MEI_DRV_GET_UNIQUE_MAILBOX_ACCESS(pMeiDev);

   ret = MEI_ReadDma16Bit( &pMeiDev->meiDrvCntrl, srcAddr,
                      (IFX_uint16_t *)pData_32, dataCount_32 * 2 );

   MEI_DRV_RELEASE_UNIQUE_MAILBOX_ACCESS(pMeiDev);

   if (ret != dataCount_32 * 2)
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
          ("MEI_DRV[%02d - %02d] MEI DMA Read Error" MEI_DRV_CRLF,
           MEI_DRV_LINENUM_GET(pMeiDev), pMeiDynCntrl->openInstance));
   }
   else
   {
      ret /= 2;
      PRN_DBG_USR( MEI_DRV, MEI_DRV_PRN_LEVEL_LOW, MEI_DRV_LINENUM_GET(pMeiDev),
             ("MEI_DRV[%02d - %02d] MEI DMA Read - %d unit read" MEI_DRV_CRLF,
             MEI_DRV_LINENUM_GET(pMeiDev), pMeiDynCntrl->openInstance, ret));

      /* swap data */
      for (i = 0; i < dataCount_32; i++)
      {
         pData_32[i] = SWAP32_DMA_WIDTH_ORDER(pData_32[i]);
      }
   }

   return ret;
}


/**
   DMA write to the target via the MEI interface.

\param
   pMeiDynCntrl - private dynamic control data (per open instance)
\param
   pDmaArgument   - points to the DMA user information data

\return
   0: in case of success number of written 32 bit units are returned via arg.
   else negative value
   - e_MEI_ERR_INVAL_ARG: max size exceeded
   - e_MEI_ERR_NOT_COMPLETE: not all units have been written.

*/
IFX_int32_t MEI_IoctlDmaAccessWr(
                              MEI_DYN_CNTRL_T        *pMeiDynCntrl,
                              IOCTL_MEI_DMA_access_t *pDmaArgument)
{
   IFX_uint32_t ret;
   MEI_DRV_PRN_LOCAL_VAR_CREATE(MEI_DEV_T, *pMeiDev, pMeiDynCntrl->pMeiDev);

   /*
      - (allocate memory and) get the test pattern (max size 0x3FE (1024-2) byte)
   */

   if (pDmaArgument->count_32bit > MEI_MAX_DMA_COUNT_32BIT)
   {
      /* ERROR: invalid buffer size - max 0x3FF */
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DRV[0x%02X]: DMA write - invalid buffer size (max 0x%X 16bit)!" MEI_DRV_CRLF,
             MEI_DRV_LINENUM_GET(pMeiDev), MEI_MAX_DMA_COUNT_32BIT));

      return -e_MEI_ERR_INVAL_ARG;
   }

   PRN_DBG_USR( MEI_DRV, MEI_DRV_PRN_LEVEL_HIGH, MEI_DRV_LINENUM_GET(pMeiDev),
         ("MEI_DRV[0x%02X]: DMA write - addr = 0x%08X, count = 0x%X (buf 0x%08X)" MEI_DRV_CRLF,
          MEI_DRV_LINENUM_GET(pMeiDev), (IFX_uint32_t)pDmaArgument->dmaAddr, (IFX_uint32_t)pDmaArgument->count_32bit,
          (IFX_uint32_t)pDmaArgument->pData_32));


   /* write DMA */
   ret = MEI_MeiDmaWrite( pMeiDynCntrl, pDmaArgument->dmaAddr,
                            pDmaArgument->count_32bit,
                            (IFX_uint32_t *)pDmaArgument->pData_32);

   /* set written units */
   pDmaArgument->count_32bit = ret;

   if (ret != pDmaArgument->count_32bit)
      return -e_MEI_ERR_NOT_COMPLETE;

   return IFX_SUCCESS;
}


/**
   DMA read from the target via the MEI interface.

\param
   pMeiDynCntrl - private dynamic control struct (per open instance)
\param
   pDmaArgument   - points to the DMA user information data

\return
   Number of read 32 bit units.
   0: in case of not complete read, the number of read 32 bit units are returned
      via the argument structure.
   else negative value
   - e_MEI_ERR_INVAL_ARG: max size exceeded
   - e_MEI_ERR_OP_FAILED: read operation failed

*/
IFX_int32_t MEI_IoctlDmaAccessRd(
                              MEI_DYN_CNTRL_T        *pMeiDynCntrl,
                              IOCTL_MEI_DMA_access_t *pDmaArgument)
{
   IFX_uint32_t   ret;
   MEI_DRV_PRN_LOCAL_VAR_CREATE(MEI_DEV_T, *pMeiDev, pMeiDynCntrl->pMeiDev);

   /*
      - (allocate memory and) get the test pattern (max size 0x3FE (1024-2) byte)
   */

   if (pDmaArgument->count_32bit > MEI_MAX_DMA_COUNT_32BIT)
   {
      /* ERROR: invalid buffer size - max 0x3FF */
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DRV[0x%02X]: DMA read - invalid buffer size (max 0x%X 32bit)!" MEI_DRV_CRLF,
             MEI_DRV_LINENUM_GET(pMeiDev), MEI_MAX_DMA_COUNT_32BIT));

      return -e_MEI_ERR_INVAL_ARG;
   }

   /* read DMA */
   ret = MEI_MeiDmaRead( pMeiDynCntrl, pDmaArgument->dmaAddr,
                           pDmaArgument->count_32bit,
                           (IFX_uint32_t *)pDmaArgument->pData_32);

   if (ret != pDmaArgument->count_32bit)
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_WRN,
            ("MEI_DRV[0x%02X]: DMA read - WARNING read %d  != expect %d!" MEI_DRV_CRLF,
             MEI_DRV_LINENUM_GET(pMeiDev), ret, (IFX_int32_t)pDmaArgument->count_32bit));
   }

   if ( (ret > 0) && (ret <= pDmaArgument->count_32bit) )
   {
      /* some data has been read */
      pDmaArgument->count_32bit = ret;
      ret = IFX_SUCCESS;
   }
   else
   {
      /* read failed */
      pDmaArgument->count_32bit = 0;
      ret = -e_MEI_ERR_OP_FAILED;
   }

   return ret;
}

#endif      /* #if (MEI_SUPPORT_DFE_DMA_ACCESS == 1) */


/* ============================================================================
   MEI DBG Access function definition
   ========================================================================= */
#if (MEI_SUPPORT_DFE_DBG_ACCESS == 1)

/**
   Write to hardware via MEI debug access.

\param
   pMeiDev     device structure
\param
   destAddr    destination address within the target system
\param
   dbgDest     destination spcae on the target system
               00 = Auxiliary address01,
               10 = LD/ST address
               11 = Core address
\param
   dataCount   Number of units to write (32 bit).
\param
   pData       points to the data to write.

\return
   Success:    number of written units.
   Error:      invalid destination.
*/
static IFX_int32_t MEI_MeiDbgWrite(
                              MEI_DYN_CNTRL_T *pMeiDynCntrl,
                              IFX_uint32_t      destAddr,
                              IFX_uint32_t      dbgDest,
                              IFX_uint32_t      dataCount,
                              IFX_uint32_t      *pData)
{
   IFX_int32_t ret ;
   MEI_DEV_T *pMeiDev = pMeiDynCntrl->pMeiDev;

   /* check parameters */
   switch(dbgDest)
   {
      case ME_DBG_DECODE_DEBUG_DEC_AUX:
      case ME_DBG_DECODE_DEBUG_DEC_LDST:
      case ME_DBG_DECODE_DEBUG_DEC_CORE:
         break;
      default:
         /* Error */
         PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_WRN,
             ("MEI_DRV[%02d - %02d] MEI debug Write - invalid dest[%d]" MEI_DRV_CRLF,
              MEI_DRV_LINENUM_GET(pMeiDev), pMeiDynCntrl->openInstance, dbgDest));
         return IFX_ERROR;
   }

#if (MEI_BM7_CODESWAP_MEIDBG == 1)
   /*
      !!! Disable mailbox to ensure that no CS request can occure
   */
   if ( MEI_DRV_BOOTMODE_GET(pMeiDev) == e_MEI_DRV_BOOT_MODE_7)
   {
      MEI_DRV_GET_UNIQUE_MAILBOX_ACCESS(pMeiDev);
   }
#endif

   ret = MEI_WriteDbg( &pMeiDev->meiDrvCntrl,
                       destAddr, dbgDest, dataCount, pData );

#if (MEI_BM7_CODESWAP_MEIDBG == 1)
   if ( MEI_DRV_BOOTMODE_GET(pMeiDev) == e_MEI_DRV_BOOT_MODE_7)
   {
      MEI_DRV_RELEASE_UNIQUE_MAILBOX_ACCESS(pMeiDev);
   }
#endif

   PRN_DBG_USR( MEI_DRV, MEI_DRV_PRN_LEVEL_LOW, MEI_DRV_LINENUM_GET(pMeiDev),
       ("MEI_DRV[%02d - %02d] MEI DbgWr[0x%08X - %s] - 0x%08X ... %d 32bit (%d)" MEI_DRV_CRLF,
        MEI_DRV_LINENUM_GET(pMeiDev), pMeiDynCntrl->openInstance,
        destAddr, ( (dbgDest == MEI_IOCTL_DEBUG_AUX) ? "AUX " :
                    (dbgDest == MEI_IOCTL_DEBUG_LDST) ? "LS  " : "CORE"),
        *pData, dataCount, ret));

   return (ret == (IFX_int32_t)dataCount) ? ret : IFX_ERROR;
}


/**
   Read from the target system via MEI debug access.

\param
   pMeiDynCntrl  dyn. control structure per open instance
\param
   srcAddr        source address within the target system
\param
   dbgSrc         debug source space on the target system
                     00 = Auxiliary address01,
                     10 = LD/ST address
                     11 = Core address
\param
   dataCount      Number of units to read (32 bit).
\param
   pData          points where the read data will be stored.

\return
   Success:    number of read units.
   Error:      invalid destination.
*/
static IFX_int32_t MEI_MeiDbgRead(
                              MEI_DYN_CNTRL_T *pMeiDynCntrl,
                              IFX_uint32_t      srcAddr,
                              IFX_uint32_t      dbgSrc,
                              IFX_uint32_t      dataCount,
                              IFX_uint32_t      *pData)
{
   IFX_int32_t ret ;
   MEI_DEV_T *pMeiDev = pMeiDynCntrl->pMeiDev;

   /* check parameters */
   switch(dbgSrc)
   {
      case ME_DBG_DECODE_DEBUG_DEC_AUX:
      case ME_DBG_DECODE_DEBUG_DEC_LDST:
      case ME_DBG_DECODE_DEBUG_DEC_CORE:
         break;
      default:
         /* Error */
         PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_WRN,
             ("MEI_DRV[%02d - %02d] MEI debug Read - invalid dest[%d]" MEI_DRV_CRLF,
              MEI_DRV_LINENUM_GET(pMeiDev), pMeiDynCntrl->openInstance, dbgSrc));
         return IFX_ERROR;
   }

   ret = MEI_ReadDbg( &pMeiDev->meiDrvCntrl,
                      srcAddr, dbgSrc, dataCount, pData);

   PRN_DBG_USR( MEI_DRV, MEI_DRV_PRN_LEVEL_LOW, MEI_DRV_LINENUM_GET(pMeiDev),
       ("MEI_DRV[%02d - %02d] MEI DbgRd[0x%08X - %s] - 0x%08X ... %d 32bit (%d)" MEI_DRV_CRLF,
        MEI_DRV_LINENUM_GET(pMeiDev), pMeiDynCntrl->openInstance,
        srcAddr, ( (dbgSrc == MEI_IOCTL_DEBUG_AUX) ? "AUX " :
                   (dbgSrc == MEI_IOCTL_DEBUG_LDST) ? "LS  " : "CORE"),
        *pData, dataCount, ret));

   return (ret == (IFX_int32_t)dataCount) ? ret : IFX_ERROR;
}

#endif      /* #if (MEI_SUPPORT_DFE_DBG_ACCESS == 1) */

#if (MEI_SUPPORT_DFE_DBG_ACCESS == 1)
/**
   Do an MEI debug write access.

\param
   pMeiDynCntrl - private dynamic control data (per open instance)
\param
   pDbgAccess     - Dbg Access write data.

\return
   IFX_SUCCESS in case of operation success
   IFX_ERROR   if written units != given units

*/
IFX_int32_t MEI_IoctlMeiDbgAccessWr(
                              MEI_DYN_CNTRL_T       *pMeiDynCntrl,
                              IOCTL_MEI_dbgAccess_t *pDbgAccess)
{
   int ret;

   /* write data */
   ret = MEI_MeiDbgWrite( pMeiDynCntrl,
                            pDbgAccess->dbgAddr, pDbgAccess->dbgDest,
                            pDbgAccess->count,   (IFX_uint32_t *)pDbgAccess->pData_32 );

   /* access done */
   if (ret != IFX_ERROR)
   {
      if (ret != (int)pDbgAccess->count)
      {
         /* some units written */
         pDbgAccess->count = (unsigned int)ret;
         ret = -e_MEI_ERR_NOT_COMPLETE;
      }
      else
         /* all units written */
         ret = IFX_SUCCESS;
   }
   else
   {
      pDbgAccess->count = 0;
      ret = -e_MEI_ERR_OP_FAILED;
   }

   pDbgAccess->ictl.retCode = ret;
   return ret;
}


/**
   Do an MEI debug read access.

\param
   pMeiDynCntrl - private dynamic control data (per open instance)
\param
   pDbgAccess     - Dbg Access read data with user buffer.

\return
   IFX_SUCCESS in case of operation success
   IFX_ERROR   if read units != given units

*/
IFX_int32_t MEI_IoctlMeiDbgAccessRd(
                              MEI_DYN_CNTRL_T       *pMeiDynCntrl,
                              IOCTL_MEI_dbgAccess_t *pDbgAccess)
{
   int ret;

   /* read data */
   ret = MEI_MeiDbgRead( pMeiDynCntrl,
                           pDbgAccess->dbgAddr, pDbgAccess->dbgDest,
                           pDbgAccess->count,   (IFX_uint32_t *)pDbgAccess->pData_32);

   /* access done */
   if (ret != IFX_ERROR)
   {
      if (ret != (int)pDbgAccess->count)
      {
         pDbgAccess->count = (unsigned int)ret;
         ret = -e_MEI_ERR_NOT_COMPLETE;
      }
      else
         ret = IFX_SUCCESS;

   }
   else
   {
      pDbgAccess->count = 0;
      ret = -e_MEI_ERR_OP_FAILED;
   }

   pDbgAccess->ictl.retCode = ret;
   return ret;
}

#endif      /* #if (MEI_SUPPORT_DFE_DBG_ACCESS == 1) */


#if (MEI_SUPPORT_DFE_GPA_ACCESS == 1)
/**
   General Purpose Access - WRITE.
   - via these function access to the devcie will be possible
     in a general way

\param
   pMeiDynCntrl - points to the dynamic control data struct.
\param
   dest           - target destination where to write to (MEM 0, AUX 1).
\param
   addr           - destination address
\param
   val            - 32 bit value to write

\return
   IFX_ERROR   in case of error
   IFX_SUCCESS in case of success
*/
IFX_int32_t MEI_GpaWrAccess(
                              MEI_DYN_CNTRL_T *pMeiDynCntrl,
                              IFX_uint32_t dest,
                              IFX_uint32_t addr, IFX_uint32_t val)
{
   IFX_int32_t ret;
   MEI_DEV_T *pMeiDev = pMeiDynCntrl->pMeiDev;

   /* check destination */
   switch(dest)
   {
      case MEI_IOCTL_GPA_DEST_MEM:
      case MEI_IOCTL_GPA_DEST_AUX:
         break;
      default:
         PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_WRN,
                ("MEI_DRV [%02d-%02d]: GPA write - invalid dest = %d" MEI_DRV_CRLF,
                 MEI_DRV_LINENUM_GET(pMeiDev), pMeiDynCntrl->openInstance, dest ));
         return -e_MEI_ERR_INVAL_ARG;
   }

   if (MEI_DRV_STATE_GET(pMeiDev) == e_MEI_DRV_STATE_SW_INIT_DONE)
   {
      /* Start device with the corresponding boot mode */
      if ( (ret = MEI_StartupDevice(pMeiDev)) != 0)
      {
         PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
                ("MEI_DRV[%02d]: ERROR - GPA Write - Start device" MEI_DRV_CRLF,
                MEI_DRV_LINENUM_GET(pMeiDev)));

         return -e_MEI_ERR_DEV_NO_RESOURCE;
      }
   }

   switch (MEI_DRV_STATE_GET(pMeiDev))
   {
#if (MEI_SUPPORT_ROM_CODE == 1)
      case e_MEI_DRV_STATE_BOOT_ROM_ALIVE:
         /* access via boot ROM code
            - write data --> wait for response */
         ret = MEI_RomHandlerWrGpa(pMeiDev, dest, addr, val);

         break;
#endif
      case e_MEI_DRV_STATE_DFE_READY:
         /* access via Online code
            - write data --> wait for response */
         ret = MEI_OnlineGpaWr(pMeiDynCntrl, dest, addr, val);

         break;
      default:
         PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_WRN,
                ("MEI_DRV [%02d-%02d]: GPA write - not available in curr state %d" MEI_DRV_CRLF,
                 MEI_DRV_LINENUM_GET(pMeiDev), pMeiDynCntrl->openInstance,
                 MEI_DRV_STATE_GET(pMeiDev) ));

         return -e_MEI_ERR_INVAL_STATE;
   }

   return ret;
}


/**
   General Purpose Access - READ.
   - via these function access to the VRX devcie will be possible
     in a general way

\param
   pMeiDynCntrl - points to the dynamic control struct.
\param
   dest           - target destination where to read from (MEM 0, AUX 1).
\param
   addr           - destination address where to read from.
\param
   val            - points to the 32 bit return value.

\return
   IFX_ERROR   in case of error
   IFX_SUCCESS in case of success

*/
IFX_int32_t MEI_GpaRdAccess(
                              MEI_DYN_CNTRL_T *pMeiDynCntrl,
                              IFX_uint32_t dest,
                              IFX_uint32_t addr, IFX_uint32_t *val)
{
   IFX_int32_t ret;
   MEI_DEV_T *pMeiDev = pMeiDynCntrl->pMeiDev;

   /* check destination */
   switch(dest)
   {
      case MEI_IOCTL_GPA_DEST_MEM:
      case MEI_IOCTL_GPA_DEST_AUX:
         break;
      default:
         PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_WRN,
                ("MEI_DRV [%02d-%02d]: GPA read - invalid dest = %d" MEI_DRV_CRLF,
                 MEI_DRV_LINENUM_GET(pMeiDev), pMeiDynCntrl->openInstance, dest ));
         return IFX_ERROR;
   }

   if (MEI_DRV_STATE_GET(pMeiDev) == e_MEI_DRV_STATE_SW_INIT_DONE)
   {
      /* Start device with the corresponding boot mode */
      if ( (ret = MEI_StartupDevice(pMeiDev)) != 0)
      {
         PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
                ("MEI_DRV[%02d]: ERROR - GPA Read - Start device" MEI_DRV_CRLF,
                MEI_DRV_LINENUM_GET(pMeiDev)));

         return IFX_ERROR;
      }
      else
      {
         PRN_DBG_USR( MEI_DRV, MEI_DRV_PRN_LEVEL_LOW, MEI_DRV_LINENUM_GET(pMeiDev),
                ("MEI_DRV[%02d]: GPA Read - Start device - successful" MEI_DRV_CRLF,
                MEI_DRV_LINENUM_GET(pMeiDev)));
      }

   }

   switch (MEI_DRV_STATE_GET(pMeiDev))
   {
#if (MEI_SUPPORT_ROM_CODE == 1)
      case e_MEI_DRV_STATE_BOOT_ROM_ALIVE:
         /* access via boot ROM code
            - read data --> wait for response */
         ret = MEI_RomHandlerRdGpa(pMeiDev, dest, addr, val);
         break;
#endif
      case e_MEI_DRV_STATE_DFE_READY:
         /* access via Online code
            - read data --> wait for response */
         PRN_DBG_USR( MEI_DRV, MEI_DRV_PRN_LEVEL_LOW, MEI_DRV_LINENUM_GET(pMeiDev),
                ("MEI_DRV[%02d]: GPA Rd Acc Onl - dest: %d, addr: 0x%08X, val = 0x%08X" MEI_DRV_CRLF,
                 MEI_DRV_LINENUM_GET(pMeiDev), dest, addr, *val));
         ret = MEI_OnlineGpaRd(pMeiDynCntrl, dest, addr, val);

         break;

      default:
         PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
                ("MEI_DRV [%02d-%02d]: GPA read - not available in state %d" MEI_DRV_CRLF,
                 MEI_DRV_LINENUM_GET(pMeiDev), pMeiDynCntrl->openInstance, MEI_DRV_STATE_GET(pMeiDev) ));

         return IFX_ERROR;
   }

   PRN_DBG_USR( MEI_DRV, MEI_DRV_PRN_LEVEL_LOW, MEI_DRV_LINENUM_GET(pMeiDev),
          ("MEI_DRV[%02d]: GPA Rd Acc - dest: %d, addr: 0x%08X, val = 0x%08X" MEI_DRV_CRLF,
          MEI_DRV_LINENUM_GET(pMeiDev), dest, addr, *val));

   return ret;
}

#endif /* #if (MEI_SUPPORT_DFE_GPA_ACCESS == 1) */




