/******************************************************************************

                          Copyright (c) 2007-2015
                     Lantiq Beteiligungs-GmbH & Co. KG

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/* ==========================================================================
   Description : VR9/VR10/AR9 Firmware Download function .
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

#if (MEI_SUPPORT_DSM == 1)
#include "drv_mei_cpe_dsm.h"
#endif /* (MEI_SUPPORT_DSM == 1) */

/* CMV messages */
#include "cmv_message_format.h"

#include "drv_mei_cpe_download.h"
#include "drv_mei_cpe_msg_process.h"

#if (MEI_SUPPORT_DRV_MODEM_TESTS == 1)
#include "drv_mei_cpe_modem_test.h"
#endif

#include "drv_mei_cpe_api_atm_ptm_intern.h"

#if (MEI_EXPORT_INTERNAL_API == 1) && (MEI_DRV_ATM_PTM_INTERFACE_ENABLE == 1)
extern void *g_xdata_addr[];
#endif

static IFX_int32_t MEI_VRX_ImageChunkFree(
                                 MEI_FW_DOWNLOAD_CNTRL_T *pFwDlCtrl,
                                 IFX_uint32_t chunkIdx);

/* ==========================================================================
   Global Variable Definitions
   ========================================================================== */

/* ==========================================================================
   Local variables
   ========================================================================== */
static IFX_int32_t MEI_VRX_PortModeControlStructureCurrentGet(
                                 MEI_DEV_T *pMeiDev,
                                 MEI_FW_PORT_MODE_CONTROL_DMA32_T *pPortModeCtrl);


/* ==========================================================================
   Local function definitions
   ========================================================================== */

static IFX_void_t MEI_VRX_InternalDataDumpShow(MEI_DEV_T *pMeiDev)
{
   IFX_uint32_t i, ret = 0;
   MEI_FW_PORT_MODE_CONTROL_DMA32_T fwPortModeCtrl = {0};
   MEI_MEI_MAILBOX_T  tempMsg;
   CMV_STD_MESSAGE_HEADER_T *header;
   IFX_int32_t count;

   /* Get current FW Port Mode Control Structure*/
   ret = MEI_VRX_PortModeControlStructureCurrentGet(pMeiDev, &fwPortModeCtrl);
   if (ret != IFX_SUCCESS)
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DRV[%02d]: FW Port Mode Control structure get failed!" MEI_DRV_CRLF,
              MEI_DRV_LINENUM_GET(pMeiDev) ));

      return;
   }

   PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL,
         ("============= PORT MODE CONTROL DUMP ============" MEI_DRV_CRLF));

   for(i = 0; i <  sizeof(MEI_FW_PORT_MODE_CONTROL_DMA32_T) / sizeof(IFX_uint32_t); i++)
   {
      PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL,
            ("0x%08X" MEI_DRV_CRLF, *(((IFX_uint32_t *)&fwPortModeCtrl) + i)));
   }

   PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL,
         ("xDSL Mode Lock: 0x%02X, Dual Port Lock: 0x%02X, Signature0: 0x%04X" MEI_DRV_CRLF,
          fwPortModeCtrl.xDslModeLock,
          fwPortModeCtrl.dualPortModeLock,
          fwPortModeCtrl.signature0));

   PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL,
         ("Cur xDSL Mode: 0x%02X, Cur Port Mode: 0x%02X, Pref xDSL Mode: 0x%02X, Pref Port Mode: 0x%02X" MEI_DRV_CRLF,
          fwPortModeCtrl.xDslModeCurrent,
          fwPortModeCtrl.dualPortModeCurrent,
          fwPortModeCtrl.xDslModePreffered,
          fwPortModeCtrl.dualPortModePreffered));

   PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL,
         ("Signature1: 0x%04X, AFE Power Up: 0x%02X, Boot Error: 0x%02X" MEI_DRV_CRLF,
          fwPortModeCtrl.signature1,
          fwPortModeCtrl.afePowerUp,
          fwPortModeCtrl.bootError));

   PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL,
         ("Image Offset SRAM: 0x%08X" MEI_DRV_CRLF,
          fwPortModeCtrl.imageOffsetSRAM));

   PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL,
         ("Bg Port Reg Value: 0x%08X" MEI_DRV_CRLF,
          fwPortModeCtrl.bgPortSelRegValue));

   PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
         ("AFE Init State: 0x%02X, Max Bg Duration: 0x%02X, Bg Duration: 0x%02X, Bg Port: 0x%02X" MEI_DRV_CRLF,
          fwPortModeCtrl.afeInitState,
          fwPortModeCtrl.maxBgDuration,
          fwPortModeCtrl.bgDuration,
          fwPortModeCtrl.bgPort));

   PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL,
         ("==================================================" MEI_DRV_CRLF));

   /* read out the message to temporary buffer */
   count = MEI_GetMailBoxMsg(   &pMeiDev->meiDrvCntrl
                              , pMeiDev->modemData.mBoxDescr.addrArc2Me
                              , &tempMsg
                              , sizeof(MEI_MEI_MAILBOX_T)/sizeof(IFX_uint16_t)
                              , IFX_TRUE);
   if (count > 0)
   {
      PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL,
            ("=============== MAILBOX HEADER DUMP ==============" MEI_DRV_CRLF));

      for(i = 0; i <  sizeof(CMV_STD_MESSAGE_HEADER_T) / sizeof(IFX_uint32_t); i++)
      {
         PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL,
               ("0x%08X" MEI_DRV_CRLF, *(((IFX_uint32_t *)&tempMsg.mbCmv.cmv) + i)));
      }

      header = &tempMsg.mbCmv.cmv.header;
      PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL,
       ("MbxId = 0x%04X, FctCode = 0x%04X" MEI_DRV_CRLF,
         header->mbxCode, header->fctCode));
      PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL,
       ("Cntrl = 0x%04X, MsgId   = 0x%04X" MEI_DRV_CRLF,
         header->paylCntrl, header->MessageID));
      PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL,
       ("Index = 0x%04X, Len     = 0x%04X" MEI_DRV_CRLF,
        header->index, header->length));
      PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL,
            ("==================================================" MEI_DRV_CRLF));
   }
}

static IFX_void_t MEI_VRX_ChunksInfoShow(MEI_DEV_T *pMeiDev)
{
   IFX_uint32_t i;
   MEI_FW_DOWNLOAD_CNTRL_T *pFwDlCtrl = &(pMeiDev->fwDl);
   MEI_FW_IMAGE_CHUNK_CTRL_T *pChunk = pMeiDev->fwDl.imageChunkCtrl;

   PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL,
         (MEI_DRV_CRLF "================= FW IMAGE CHUNKS=================" MEI_DRV_CRLF));

   for (i=0; i<pFwDlCtrl->meiMaxChunkCount; i++)
   {
      if (pChunk[i].eImageChunkType == eMEI_FW_IMAGE_CHUNK_UNDEFINED)
         continue;

      PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL,
            ("chunk[%02d]: virt_addr = 0x%p (0x%p), phys_addr = 0x%p, size = %5d [byte], type = %d" MEI_DRV_CRLF,
            i, pChunk[i].pImageChunk_aligned, pChunk[i].pImageChunk_allocated,
            (void*)pChunk[i].pImageChunk_phy, pChunk[i].imageChunkSize_byte, pChunk[i].eImageChunkType));

   }

   PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL,
         (MEI_DRV_CRLF "==================================================" MEI_DRV_CRLF));
}

static IFX_int32_t MEI_VRX_ArcFromHaltRelease(MEI_DEV_T *pMeiDev)
{
   IFX_uint32_t arc_status = 0;

#if MEI_DBG_CECK_BOOTLOADER_START == 1
   MEI_FW_PORT_MODE_CONTROL_DMA32_T portModeCtrl = {0};

   if (MEI_VRX_PortModeControlStructureCurrentGet(
          pMeiDev, &portModeCtrl) != IFX_SUCCESS)
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
         ("MEI_DRV: Current Port Mode Control Structure get failed!"
         MEI_DRV_CRLF));

      return (-e_MEI_ERR_OP_FAILED);
   }

   PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL,
         (MEI_DRV_CRLF "signature1: 0x%04X"
          MEI_DRV_CRLF, portModeCtrl.signature1));
#endif

   PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_LOW,
      ("MEI_DRV: Starting to release ARC from the HALT state"MEI_DRV_CRLF));

   /* Get ARC_STATUS register*/
   if (MEI_ReadDbg(&(pMeiDev->meiDrvCntrl), MEI_REG_ARC_STATUS,
         ME_DBG_DECODE_DEBUG_DEC_AUX, 1, &arc_status) != 1)
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
         ("MEI_DRV: ARC_STATUS read failed!" MEI_DRV_CRLF));
      return (IFX_int32_t)IFX_ERROR;
   }

   /* Clear 25th (HALT) bit*/
   arc_status &= ~(1 << 25);

   if (MEI_WriteDbg(&(pMeiDev->meiDrvCntrl), MEI_REG_ARC_STATUS,
         ME_DBG_DECODE_DEBUG_DEC_AUX, 1, &arc_status) != 1)
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
         ("MEI_DRV: ARC_STATUS write failed!" MEI_DRV_CRLF));
      return (IFX_int32_t)IFX_ERROR;
   }

   PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_LOW,
      ("MEI_DRV: ARC released from the HALT state"MEI_DRV_CRLF));

   return (IFX_int32_t)IFX_SUCCESS;
}

#if (MEI_SUPPORT_PCI_SLAVE_FW_DOWNLOAD == 1)
MEI_PCI_SLAVE_POOL_T* MEI_VR9_PciSlavePoolCreate(
                                 IFX_uint8_t *pPciStart,
                                 IFX_uint8_t *pDdrStart,
                                 IFX_uint32_t pool_size_byte)
{
   MEI_PCI_SLAVE_POOL_T *pPool;

   if (!pPciStart || !pDdrStart || !pool_size_byte)
   {
      return IFX_NULL;
   }

   if (((IFX_uint32_t)pDdrStart) & (~MEI_FW_IMAGE_CHUNK_ADDR_MASK))
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
         ("MEI_DRV: PCI slave DDR pool address not aligned!" MEI_DRV_CRLF));

      return IFX_NULL;
   }

   /* Allocate pool memory*/
   pPool = (MEI_PCI_SLAVE_POOL_T*)MEI_DRVOS_Malloc(sizeof(MEI_PCI_SLAVE_POOL_T));

   if (!pPool)
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
         ("MEI_DRV: no memory for PCI slave pool size!" MEI_DRV_CRLF));

      return IFX_NULL;
   }

   /* Initialize pool*/
   memset(pPool, 0x0, sizeof(MEI_PCI_SLAVE_POOL_T));

   pPool->pPciStart      = pPciStart;
   pPool->pDdrStart      = pDdrStart;
   pPool->pool_size_byte = pool_size_byte;

   return pPool;
}

IFX_void_t MEI_VR9_PciSlavePoolDelete(
                                 MEI_PCI_SLAVE_POOL_T *pPool)
{
   MEI_PCI_SLAVE_POOL_ELEMENT_T *pElement, *pNextElement = IFX_NULL;

   if (!pPool)
      return;

   pElement = pPool->Head.pNext;

   while(pElement)
   {
      pNextElement = pElement;

      MEI_DRVOS_Free(pElement);

      pElement = pNextElement;
   }

   MEI_DRVOS_Free(pPool);

   return;
}

static IFX_uint8_t* MEI_VR9_PciSlavePoolElementAlloc(
                                 MEI_PCI_SLAVE_POOL_T *pPool,
                                 IFX_uint32_t size)
{
   IFX_uint8_t *pAlloc = IFX_NULL;
   MEI_PCI_SLAVE_POOL_ELEMENT_T *pElement = &(pPool->Head),
                                *pElementPrev = &(pPool->Head);

   if (size > pPool->pool_size_byte)
   {
      return IFX_NULL;
   }

   while(1)
   {
      if (pElement == IFX_NULL)
      { /* Create new pool element*/
         pElement =
            (MEI_PCI_SLAVE_POOL_ELEMENT_T*)MEI_DRVOS_Malloc(sizeof(MEI_PCI_SLAVE_POOL_ELEMENT_T));

         if (!pElement)
         {
            PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
               ("MEI_DRV: no memory for PCI slave pool element!" MEI_DRV_CRLF));
            return IFX_NULL;
         }

         memset(pElement, 0x0, sizeof(MEI_PCI_SLAVE_POOL_ELEMENT_T));
         pElementPrev->pNext = pElement;
      }

      if (!pElement->bUsed)
      { /* Current element is not used*/
         if (pElement->element_size_byte == 0)
         { /* New Element*/
            if (size <= pPool->pool_size_byte - pPool->fill_offset)
            { /* we have some free space, add element to the pool's end*/
                /* Mark new element as Used*/
               pElement->bUsed = IFX_TRUE;
               /* Set allocated pointer*/
               pAlloc = pPool->pDdrStart + pPool->fill_offset;
               /* Update element offset*/
               pElement->element_offset = pPool->fill_offset;
               /* Update pool offset*/
               pPool->fill_offset += size;
               /* Set current element size*/
               pElement->element_size_byte = size;

               return pAlloc;
            }
         }
         else if (size <= pElement->element_size_byte)
         { /* Requested size fits to the available element size*/
            /* Mark existing element as Used*/
            pElement->bUsed = IFX_TRUE;

            pAlloc = pPool->pDdrStart + pElement->element_offset;
            return pAlloc;
         }
      }

      if (size > pPool->pool_size_byte - pElement->element_offset)
      { /* No memory left in the pool, we reached the last pool element*/
         return IFX_NULL;
      }

      /* Switch to the next element*/
      pElementPrev = pElement;
      pElement     = pElement->pNext;
   }
}

static IFX_void_t MEI_VR9_PciSlavePoolElementFree(
                                 MEI_PCI_SLAVE_POOL_T *pPool,
                                 IFX_uint8_t *addr)
{
   MEI_PCI_SLAVE_POOL_ELEMENT_T *pElement = &(pPool->Head);

   while (pElement)
   {
      if (addr == (pPool->pDdrStart + pElement->element_offset))
      {
         if (!pElement->bUsed)
         {
            PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
               ("MEI_DRV: attempt to free not used PCI pool element!" MEI_DRV_CRLF));
         }

         pElement->bUsed = IFX_FALSE;

         break;
      }

      pElement = pElement->pNext;
   }

   return;
}

static IFX_uint8_t* MEI_VR9_PciSlavePciAddrGet(
                                 MEI_PCI_SLAVE_POOL_T *pPool,
                                 IFX_uint8_t *ddr_addr)
{
   IFX_uint8_t *pci_addr;

   pci_addr = pPool->pPciStart + (ddr_addr - pPool->pDdrStart);

   return pci_addr >= pPool->pPciStart ? pci_addr : pPool->pPciStart;
}
#endif /* #if (MEI_SUPPORT_PCI_SLAVE_FW_DOWNLOAD == 1)*/

static IFX_uint8_t* MEI_VRX_TranslateMipsToArc(
                                 IFX_uint8_t *pChunkMips, MEI_DRVOS_DMA_T pChunkMips_phy)
{
   IFX_uint8_t *pChunkArc = IFX_NULL;

   pChunkArc = pChunkMips;

   if (! MEI_DEVICE_CFG_IS_PLATFORM(e_MEI_DEV_PLATFORM_CONFIG_VR9))
   {
      /* Translate chunk addresses located in SDRAM to be accessible by FW in ARC */
      pChunkArc = (IFX_uint8_t *)(long)(pChunkMips_phy + MEI_OUTBOUND_ADDRESS_BASE);
   }

   return pChunkArc;
}

static IFX_int32_t MEI_VRX_ImageChunkAlloc(
                                 MEI_FW_DOWNLOAD_CNTRL_T *pFwDlCtrl,
                                 IFX_uint32_t chunkIdx,
                                 IFX_uint32_t chunkSize_byte)
{
   IFX_int32_t ret = 0;
   IFX_uint8_t *pImageChunk_allocated = NULL;
   MEI_DRVOS_DMA_T pImageChunk_phy;
   MEI_FW_IMAGE_CHUNK_CTRL_T *pImageChunkCtrl = pFwDlCtrl->imageChunkCtrl;

   if ( ((chunkSize_byte > MEI_FW_IMAGE_CHUNK_SIZE_BYTE) &&
         (chunkIdx != MEI_FW_IMAGE_DEBUG_CHUNK_INDEX + pFwDlCtrl->meiSpecialChunkOffset)) ||
         (chunkIdx > pFwDlCtrl->meiMaxChunkCount - 1) )
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
         ("MEI_DRV: chunk[%d] size %d incorrect!" MEI_DRV_CRLF,
         chunkIdx, chunkSize_byte));
      return IFX_ERROR;
   }

   /* Check for the last chunk*/
   if (chunkIdx == pFwDlCtrl->meiMaxChunkCount - 1)
   {
      /* Release last chunk since it's size could vary. Will be allocated
         with a new size below. */
      MEI_VRX_ImageChunkFree(pFwDlCtrl, chunkIdx);
   }

   /* Check if for a released chunk*/
   if (!(pImageChunkCtrl[chunkIdx].pImageChunk_allocated))
   {
#if (MEI_SUPPORT_PCI_SLAVE_FW_DOWNLOAD == 1)
      if (pFwDlCtrl->bPciSlave)
      {
         pImageChunk_allocated = MEI_VR9_PciSlavePoolElementAlloc(
                                    pFwDlCtrl->pPool, chunkSize_byte);
      }
      else
#endif /* #if (MEI_SUPPORT_PCI_SLAVE_FW_DOWNLOAD == 1)*/
      {
         /* Allocate chunk memory with the specified chunk size*/
         pImageChunk_allocated =(IFX_uint8_t *)MEI_DRVOS_DMA_Malloc(
                            MEI_DEVICE_CFG_VALUE_GET(dev),
                            chunkSize_byte, &pImageChunk_phy);
      }

      if (!pImageChunk_allocated)
      {
         PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DRV: no memory for chunk[%d] size %d." MEI_DRV_CRLF,
            chunkIdx, chunkSize_byte));

         ret = -e_MEI_ERR_NO_MEM;
         return ret;
      }

      /* Check if the allocated chunk is not aligned*/
      if ( (((IFX_uint32_t)pImageChunk_allocated) & (~MEI_FW_IMAGE_CHUNK_ADDR_MASK))
#if (MEI_SUPPORT_PCI_SLAVE_FW_DOWNLOAD == 1)
           && !(pFwDlCtrl->bPciSlave)
#endif /* #if (MEI_SUPPORT_PCI_SLAVE_FW_DOWNLOAD == 1)*/
         )
      {
         /* Free current allocated chunk which is not aligned*/
         MEI_DRVOS_DMA_Free(MEI_DEVICE_CFG_VALUE_GET(dev),
                            chunkSize_byte, pImageChunk_allocated, pImageChunk_phy);

         /* Allocate chunk memory for further alignment*/
         pImageChunk_allocated = (IFX_uint8_t *)MEI_DRVOS_DMA_Malloc(
                     MEI_DEVICE_CFG_VALUE_GET(dev),
                     MEI_FW_IMAGE_CHUNK_ALIGNED_SIZE_BYTE(chunkSize_byte),
                                          &pImageChunk_phy);

         if (!pImageChunk_allocated)
         {
            PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
               ("MEI_DRV: no memory for aligned chunk[%d] size %d." MEI_DRV_CRLF,
               chunkIdx, MEI_FW_IMAGE_CHUNK_ALIGNED_SIZE_BYTE(chunkSize_byte)));

            ret = -e_MEI_ERR_NO_MEM;
            return ret;
         }

         /* Assign chunk aligned address*/
         pImageChunkCtrl[chunkIdx].pImageChunk_aligned =
            MEI_FW_IMAGE_CHUNK_ALIGNED_ADDR_GET(pImageChunk_allocated);
      }
      else
      {
         /* Assign chunk allocated address*/
         pImageChunkCtrl[chunkIdx].pImageChunk_aligned = pImageChunk_allocated;
      }

      pImageChunkCtrl[chunkIdx].pImageChunk_phy = pImageChunk_phy;
#if (MEI_SUPPORT_PCI_SLAVE_FW_DOWNLOAD == 1)
      if (pFwDlCtrl->bPciSlave)
      {
         pImageChunkCtrl[chunkIdx].pBARx = MEI_VR9_PciSlavePciAddrGet(
                                             pFwDlCtrl->pPool,
                                             pImageChunkCtrl[chunkIdx].pImageChunk_aligned);
      }
      else
#endif /* #if (MEI_SUPPORT_PCI_SLAVE_FW_DOWNLOAD == 1)*/
      {
         /* Prepare chunk MIPS address for ARC format */
         pImageChunkCtrl[chunkIdx].pBARx =
            MEI_VRX_TranslateMipsToArc(pImageChunkCtrl[chunkIdx].pImageChunk_aligned,
                                       pImageChunkCtrl[chunkIdx].pImageChunk_phy);
      }

      /* Assign chunk allocated address*/
      pImageChunkCtrl[chunkIdx].pImageChunk_allocated = pImageChunk_allocated;
      /* Assign chunk size*/
      pImageChunkCtrl[chunkIdx].imageChunkSize_byte = chunkSize_byte;
   }

#if (MEI_SUPPORT_OPTIMIZED_FW_DL == 1)
   /* Available data is empty */
   pImageChunkCtrl[chunkIdx].imageChunkDataSize_byte = 0;
   /* CRC was not calculated yet */
   pImageChunkCtrl[chunkIdx].imageChunkCRC = 0;
   /* Chunk type is not known yet */
   pImageChunkCtrl[chunkIdx].imageChunkFlavour = eMEI_FW_CHUNK_XDSL;
#endif /* (MEI_SUPPORT_OPTIMIZED_FW_DL == 1) */

   /* Assign chunk type*/
   if (chunkIdx == MEI_FW_IMAGE_DATA_CHUNK_INDEX+pFwDlCtrl->meiSpecialChunkOffset)
   {
#if (MEI_EXPORT_INTERNAL_API == 1) && (MEI_DRV_ATM_PTM_INTERFACE_ENABLE == 1)
      /* Place this assignment here only as a workaround to provide
         DATA address to the ATM/PTM drivers*/
      g_xdata_addr[pFwDlCtrl->line_num] = (void*)pImageChunkCtrl[chunkIdx].pImageChunk_allocated;
#endif
      pImageChunkCtrl[chunkIdx].eImageChunkType = eMEI_FW_IMAGE_CHUNK_DATA;
   }
   else
   {
      pImageChunkCtrl[chunkIdx].eImageChunkType =
         (chunkIdx*chunkSize_byte < pFwDlCtrl->cachedRegionSize_byte) ?
            eMEI_FW_IMAGE_CHUNK_CACHED : eMEI_FW_IMAGE_CHUNK_REALLOC;
   }

   return ret;
}

static IFX_int32_t MEI_VRX_ImageChunkFree(
                                 MEI_FW_DOWNLOAD_CNTRL_T *pFwDlCtrl,
                                 IFX_uint32_t chunkIdx)
{
   MEI_FW_IMAGE_CHUNK_CTRL_T *pImageChunkCtrl = pFwDlCtrl->imageChunkCtrl;

   if (chunkIdx > pFwDlCtrl->meiMaxChunkCount - 1)
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
         ("MEI_DRV: chunk index %d incorrect!" MEI_DRV_CRLF, chunkIdx));
      return IFX_ERROR;
   }

   if (pImageChunkCtrl[chunkIdx].pImageChunk_allocated)
   {
#if (MEI_SUPPORT_PCI_SLAVE_FW_DOWNLOAD == 1)
      if (pFwDlCtrl->bPciSlave)
      {
         MEI_VR9_PciSlavePoolElementFree(
            pFwDlCtrl->pPool,
            pImageChunkCtrl[chunkIdx].pImageChunk_allocated);
      }
      else
#endif /* #if (MEI_SUPPORT_PCI_SLAVE_FW_DOWNLOAD == 1)*/
      {
         MEI_DRVOS_DMA_Free(MEI_DEVICE_CFG_VALUE_GET(dev),
                            pImageChunkCtrl[chunkIdx].imageChunkSize_byte,
                            pImageChunkCtrl[chunkIdx].pImageChunk_aligned,
                            pImageChunkCtrl[chunkIdx].pImageChunk_phy);
         pImageChunkCtrl[chunkIdx].pImageChunk_phy = 0;
      }
      pImageChunkCtrl[chunkIdx].pImageChunk_allocated = NULL;
      pImageChunkCtrl[chunkIdx].pImageChunk_aligned   = NULL;
      pImageChunkCtrl[chunkIdx].imageChunkSize_byte   = 0;
      pImageChunkCtrl[chunkIdx].eImageChunkType =
         eMEI_FW_IMAGE_CHUNK_UNDEFINED;

#if (MEI_SUPPORT_OPTIMIZED_FW_DL == 1)
      pImageChunkCtrl[chunkIdx].imageChunkDataSize_byte = 0;
      pImageChunkCtrl[chunkIdx].imageChunkCRC         = 0;
      pImageChunkCtrl[chunkIdx].imageChunkFlavour =
         eMEI_FW_CHUNK_XDSL;
#endif /* (MEI_SUPPORT_OPTIMIZED_FW_DL == 1) */
   }

#if (MEI_EXPORT_INTERNAL_API == 1) && (MEI_DRV_ATM_PTM_INTERFACE_ENABLE == 1)
   /* Place this assignment here only as a workaround to profide
      DATA address to the ATM/PTM drivers*/
   if (chunkIdx == MEI_FW_IMAGE_DATA_CHUNK_INDEX + pFwDlCtrl->meiSpecialChunkOffset)
   {
      g_xdata_addr[pFwDlCtrl->line_num] = IFX_NULL;
   }
#endif

   return IFX_SUCCESS;
}

/**
   Fill temporary Xpage with the code/data words.

\param
   pMeiDrvCntrl:   points to the MEI interface register set
\param
   pChunkCtrl  points to chunk control structure.
\param
   pXpageInfo  points to current X page info header.

\return
   IFX_SUCCESS
   IFX_ERROR
*/
static IFX_int32_t MEI_VRX_XpageWrite(
                                 MEI_FW_DOWNLOAD_CNTRL_T *pFwDlCtrl,
                                 MEI_MEI_DRV_CNTRL_T *pMeiDrvCntrl,
                                 MEI_FW_IMAGE_CHUNK_CTRL_T *pChunkCtrl,
                                 MEI_FW_IMAGE_PAGE_T *pXpageInfo,
                                 IFX_boolean_t bData)
{
   IFX_int32_t ret = 0;
   IFX_uint32_t chunkIdx, page_offset_bytes, wordCount, *pChunkPageData = NULL;
   IFX_uint32_t destAddr, pageSize_32Bit, cnt, pageWord = 0;

   /* Get code/data Page destination address*/
   destAddr = bData ? pXpageInfo->dataDestAddr :
                              pXpageInfo->codeDestAddr;

   /* Get code/data Page size [32bit]*/
   pageSize_32Bit = (bData ? pXpageInfo->dataPageSize_32Bit :
                                    pXpageInfo->codePageSize_32Bit) & (~MEI_BOOT_FLAG);

   /* Get code/data Page offset within image [bytes]*/
   page_offset_bytes = bData ? pXpageInfo->dataOffset_Byte :
                               pXpageInfo->codeOffset_Byte;

   /* Check if page offset is within the last large chunk*/
   if (page_offset_bytes >= (pFwDlCtrl->meiMaxChunkCount-2)*MEI_FW_IMAGE_CHUNK_SIZE_BYTE)
   {
      chunkIdx = pFwDlCtrl->meiMaxChunkCount - 1;
   }
   else
   {
      /* Get chunk index*/
      chunkIdx = page_offset_bytes / MEI_FW_IMAGE_CHUNK_SIZE_BYTE;
   }

   if (chunkIdx > (pFwDlCtrl->meiMaxChunkCount - 1))
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
         ("MEI_DRV: Out of boundaries chunk[%d] detected while preparing Xpage"
         " with offset %d bytes!"
         MEI_DRV_CRLF, chunkIdx, page_offset_bytes));

      return IFX_ERROR;
   }

   /* Skip DATA chunk*/
   chunkIdx = chunkIdx == (MEI_FW_IMAGE_DATA_CHUNK_INDEX + pFwDlCtrl->meiSpecialChunkOffset)?
                                                                  chunkIdx + 1 : chunkIdx;

   /* Check for a valid chunk*/
   if (pChunkCtrl[chunkIdx].eImageChunkType == eMEI_FW_IMAGE_CHUNK_UNDEFINED)
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
         ("MEI_DRV: Undefined chunk[%d] detected while trying to fill Xpage!"
         MEI_DRV_CRLF, chunkIdx));

      return IFX_ERROR;
   }

   /* Get relative page offset*/
   page_offset_bytes = page_offset_bytes % pChunkCtrl[chunkIdx].imageChunkSize_byte;

   pChunkPageData = (IFX_uint32_t*)(pChunkCtrl[chunkIdx].pImageChunk_aligned + page_offset_bytes);

   /* Fill Xpage*/
   for (wordCount=0; wordCount<pageSize_32Bit; wordCount++)
   {
      if (wordCount*4 + page_offset_bytes >= pChunkCtrl[chunkIdx].imageChunkSize_byte)
      {
         /* Move to the next chunk, skip DATA chunk if necessary*/
         chunkIdx += (chunkIdx ==
                     (MEI_FW_IMAGE_DATA_CHUNK_INDEX + pFwDlCtrl->meiSpecialChunkOffset)? 2 : 1);

         if (chunkIdx > (pFwDlCtrl->meiMaxChunkCount - 1))
         {
            PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
               ("MEI_DRV: Undefined chunk[%d] detected while trying to fill Xpage!"
               MEI_DRV_CRLF, chunkIdx));

            ret = IFX_ERROR;
            break;
         }

         /* Check for a valid chunk*/
         if (pChunkCtrl[chunkIdx].eImageChunkType == eMEI_FW_IMAGE_CHUNK_UNDEFINED)
         {
            PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
               ("MEI_DRV: Undefined chunk[%d] detected while trying to fill Xpage!"
               MEI_DRV_CRLF, chunkIdx));

            ret = IFX_ERROR;
            break;
         }

         /* Reset relative offset*/
         page_offset_bytes = 0;
         /* ...and now data starts directly from the 1st chunk address*/
         pChunkPageData = (IFX_uint32_t*)(pChunkCtrl[chunkIdx].pImageChunk_aligned);
      }

      /* Copy Page data from chunks*/
      pageWord = *pChunkPageData++;

      /* Write one 32-bit word*/
      cnt = MEI_WriteDma32Bit(
               pMeiDrvCntrl, destAddr, &pageWord, 1, 0);

      if (cnt != 1)
      {
         ret = IFX_ERROR;
         break;
      }

      destAddr += 4;
   }

   return ret;
}

/**
   Get firmware Port Mode Control Structure.

\param
   pMeiDrvCntrl:   points to the MEI interface register set
\param
   pXpage:         points to the Port Mode Control Structure.

\return
   IFX_SUCCESS
   IFX_ERROR
*/
static IFX_int32_t MEI_VRX_PortModeControlStructureCurrentGet(
                                 MEI_DEV_T *pMeiDev,
                                 MEI_FW_PORT_MODE_CONTROL_DMA32_T *pPortModeCtrl)
{
   IFX_int32_t ret = 0;
   IFX_uint32_t cnt = 0;

   /* Protect device DMA access */
   MEI_DRV_GET_UNIQUE_ACCESS(pMeiDev);

   cnt = MEI_ReadDma32Bit(
            &(pMeiDev->meiDrvCntrl),
            MEI_FW_PORT_MODE_CONTROL_STRUCTURE_ADDR,
            (IFX_uint32_t*)pPortModeCtrl,
            sizeof(MEI_FW_PORT_MODE_CONTROL_DMA32_T)/sizeof(IFX_uint32_t));

   MEI_DRV_RELEASE_UNIQUE_ACCESS(pMeiDev);

   if (cnt != sizeof(MEI_FW_PORT_MODE_CONTROL_DMA32_T)/sizeof(IFX_uint32_t))
      ret = IFX_ERROR;

   return ret;
}

/**
   Set firmware Port Mode Control Structure.

\param
   pMeiDrvCntrl:   points to the MEI interface register set
\param
   pPortModeCtrl:         points to the Port Mode Control Structure.

\return
   IFX_SUCCESS
   IFX_ERROR
*/
static IFX_int32_t MEI_VRX_PortModeControlStructureCurrentSet(
                                 MEI_DEV_T *pMeiDev,
                                 MEI_FW_PORT_MODE_CONTROL_DMA32_T *pPortModeCtrl)
{
   IFX_int32_t ret = 0;
   IFX_uint32_t cnt = 0;

   /* Protect device DMA access */
   MEI_DRV_GET_UNIQUE_ACCESS(pMeiDev);

   cnt = MEI_WriteDma32Bit(
            &(pMeiDev->meiDrvCntrl),
            MEI_FW_PORT_MODE_CONTROL_STRUCTURE_ADDR,
            (IFX_uint32_t*)pPortModeCtrl,
            sizeof(MEI_FW_PORT_MODE_CONTROL_DMA32_T)/sizeof(IFX_uint32_t),
            0);

   MEI_DRV_RELEASE_UNIQUE_ACCESS(pMeiDev);

   if (cnt != sizeof(MEI_FW_PORT_MODE_CONTROL_DMA32_T)/sizeof(IFX_uint32_t))
      ret = IFX_ERROR;

   return ret;
}

static IFX_int32_t MEI_VRX_PortModeControlStructureDefaultSet(
                                 MEI_DEV_T *pMeiDev)
{
   IFX_int32_t ret = 0;
   MEI_FW_PORT_MODE_CONTROL_DMA32_T fwPortModeCtrl = {0};
   IFX_uint32_t P0_IN;
   IFX_uint8_t hybrid_type;

   /* Get current FW Port Mode Control Structure*/
   ret = MEI_VRX_PortModeControlStructureCurrentGet(pMeiDev, &fwPortModeCtrl);
   if (ret != IFX_SUCCESS)
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DRV[%02d]: FW Port Mode Control structure get failed!" MEI_DRV_CRLF,
              MEI_DRV_LINENUM_GET(pMeiDev) ));

      return (-e_MEI_ERR_OP_FAILED);
   }

   /* Overwrite values with defauls from the FW image header. These values are not set
      by the upper layer SW yet*/
   fwPortModeCtrl.afeInitState      = pMeiDev->fwDl.defaultPortModeCtrl.afeInitState;
   fwPortModeCtrl.afePowerUp        = pMeiDev->fwDl.defaultPortModeCtrl.afePowerUp;
   fwPortModeCtrl.bgDuration        = pMeiDev->fwDl.defaultPortModeCtrl.bgDuration;
   fwPortModeCtrl.bgPort            = pMeiDev->fwDl.defaultPortModeCtrl.bgPort;
   fwPortModeCtrl.bgPortSelRegValue = pMeiDev->fwDl.defaultPortModeCtrl.bgPortSelRegValue;
   fwPortModeCtrl.imageOffsetSRAM   = pMeiDev->fwDl.defaultPortModeCtrl.imageOffsetSRAM;
   fwPortModeCtrl.maxBgDuration     = pMeiDev->fwDl.defaultPortModeCtrl.maxBgDuration;

   /** \todo [VRX518] Check/rework GPIO handling for LIF detection on VRX500
       platform. By now just set it to AnnexA. */
   if (MEI_DEVICE_CFG_IS_PLATFORM(e_MEI_DEV_PLATFORM_CONFIG_VR11))
   {
      /* bit 2-4: Hybrid Type (LIF module ID) */
      fwPortModeCtrl.afePowerUp |= MEI_HYBRID_TYPE_A << 2;
   }
   else if (! MEI_DEVICE_CFG_IS_PLATFORM(e_MEI_DEV_PLATFORM_CONFIG_VR9))
   {
      P0_IN = *MEI_GPIO_U32REG(GPIO_P0_IN);

      PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_LOW,
         ("MEI_DRV: *MEI_GPIO_U32REG(%p)=0x%08X" MEI_DRV_CRLF,
          MEI_GPIO_U32REG(GPIO_P0_IN), P0_IN));

      /* LIF Det 0 - bit 0 of P0_IN
         LIF Det 1 - bit 3 of P0_IN
         LIF Det 2 - bit 8 of P0_IN */
      hybrid_type = (P0_IN & 0x1) | ((P0_IN >> 2) & 0x2) | ((P0_IN >> 6) & 0x4);

      if ((hybrid_type != MEI_HYBRID_TYPE_A) &&
          (hybrid_type != MEI_HYBRID_TYPE_BJ))
      {
         PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DRV[%02d]: Ignore unknown GPIO hybrid type 0x%x"MEI_DRV_CRLF,
             MEI_DRV_LINENUM_GET(pMeiDev), hybrid_type));

         PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_WRN,
            ("MEI_DRV[%02d]: Set hybrid type in sync with firmware"
             " xDSL Mode 0x%04X" MEI_DRV_CRLF, MEI_DRV_LINENUM_GET(pMeiDev),
             pMeiDev->firmwareFeatures.eFirmwareXdslModes));

         if (pMeiDev->firmwareFeatures.eFirmwareXdslModes &
                                                      e_MEI_FW_XDSLMODE_ADSL_B)
         {
            hybrid_type = MEI_HYBRID_TYPE_BJ;
         }
         else
         {
            hybrid_type = MEI_HYBRID_TYPE_A;
         }
      }

      PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_WRN,
         ("MEI_DRV[%02d]: Hybrid Type (LIF module ID) 0x%x" MEI_DRV_CRLF,
         MEI_DRV_LINENUM_GET(pMeiDev), hybrid_type));

      /* clean LIF Det bits (afe bits 2,3,4)*/
      fwPortModeCtrl.afePowerUp &= ~((1<<2) | (1<<3) | (1<<4));
      /* bit 2-4: Hybrid Type (LIF module ID) */
      fwPortModeCtrl.afePowerUp |= hybrid_type << 2;
   }

#if MEI_DBG_CECK_BOOTLOADER_START == 1
   /* Included for debug purpose (bringup) to set dummy value for signature 1 */
   fwPortModeCtrl.signature1 = (IFX_uint16_t)0xCAFE;
   PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL,
         (MEI_DRV_CRLF "======= Port Mode Control (debug modify) =========" MEI_DRV_CRLF));
   PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL,
         (MEI_DRV_CRLF "Set signature1 to dummy value (0xCAFE)" MEI_DRV_CRLF));
   PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL,
         (MEI_DRV_CRLF "==================================================" MEI_DRV_CRLF));
#endif

   /* Update current FW Port Mode Control Structure*/
   ret = MEI_VRX_PortModeControlStructureCurrentSet(pMeiDev, &fwPortModeCtrl);
   if (ret != IFX_SUCCESS)
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DRV[%02d]: FW Port Mode Control structure set failed!" MEI_DRV_CRLF,
              MEI_DRV_LINENUM_GET(pMeiDev) ));

      return (-e_MEI_ERR_OP_FAILED);
   }

   return ret;
}

#if (MEI_SUPPORT_OPTIMIZED_FW_DL == 1)
static IFX_int32_t MEI_VRX_ChunksCRC_Check(
                                 MEI_DEV_T *pMeiDev)
{
   IFX_int32_t ret = 0;
   IFX_uint32_t crc, chunkIdx;
   MEI_FW_DOWNLOAD_CNTRL_T *pFwDlCtrl = &(pMeiDev->fwDl);
   MEI_FW_PORT_MODE_CONTROL_DMA32_T fwPortModeCtrl = {0};

   /* Get current FW Port Mode Control Structure*/
   ret = MEI_VRX_PortModeControlStructureCurrentGet(pMeiDev, &fwPortModeCtrl);
   if (ret != IFX_SUCCESS)
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DRV[%02d]: FW Port Mode Control structure get failed!" MEI_DRV_CRLF,
              MEI_DRV_LINENUM_GET(pMeiDev) ));

      return (-e_MEI_ERR_OP_FAILED);
   }

   for (chunkIdx = 0; chunkIdx < pFwDlCtrl->meiUsedChunkCount; chunkIdx++)
   {
      /* for VDSL ignore ADSL chunks and vice verse */
      switch (fwPortModeCtrl.xDslModeCurrent)
      {
         case MEI_FW_XDSL_MODE_VDSL:
            if (pFwDlCtrl->imageChunkCtrl[chunkIdx].imageChunkFlavour ==
                                                            eMEI_FW_CHUNK_ADSL)
            {
               continue;
            }
            break;

         case MEI_FW_XDSL_MODE_ADSL:
            if (pFwDlCtrl->imageChunkCtrl[chunkIdx].imageChunkFlavour ==
                                                            eMEI_FW_CHUNK_VDSL)
            {
               continue;
            }
            break;
      }

      crc = crc32(~(uint32_t)0, pFwDlCtrl->imageChunkCtrl[chunkIdx].pImageChunk_aligned,
             pFwDlCtrl->imageChunkCtrl[chunkIdx].imageChunkDataSize_byte);

      if (crc != pFwDlCtrl->imageChunkCtrl[chunkIdx].imageChunkCRC)
      {
         PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DRV: allocated chunks reuse failed! (chunk[%d] %d bytes, CRC 0x%08X)"
             MEI_DRV_CRLF, chunkIdx,
             pFwDlCtrl->imageChunkCtrl[chunkIdx].imageChunkDataSize_byte, crc));

         return (-e_MEI_ERR_OPTIMIZED_FW_DL_FAILED);
      }
   }

   PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL,
            ("MEI_DRV: reused allocated chunks" MEI_DRV_CRLF));

   return ret;
}

static IFX_int32_t MEI_VRX_ChunksCRC_Save(
                                 MEI_DEV_T *pMeiDev)
{
   IFX_int32_t ret = 0;
   IFX_uint32_t chunkIdx;
   MEI_FW_DOWNLOAD_CNTRL_T *pFwDlCtrl = &(pMeiDev->fwDl);

   if (pFwDlCtrl->bChunksFilled)
   {
      /* Calc/save chunks CRC only first time fw download (after chunks filled) */
      /* Next fw download will skip CRC calc - will use saved chunks CRC */
      pFwDlCtrl->bChunksFilled = IFX_FALSE;

      for (chunkIdx = 0; chunkIdx < pFwDlCtrl->meiUsedChunkCount; chunkIdx++)
      {
         pFwDlCtrl->imageChunkCtrl[chunkIdx].imageChunkCRC =
            crc32(~(uint32_t)0,
            pFwDlCtrl->imageChunkCtrl[chunkIdx].pImageChunk_aligned,
            pFwDlCtrl->imageChunkCtrl[chunkIdx].imageChunkDataSize_byte);
      }
   }

   return ret;
}
#endif /* (MEI_SUPPORT_OPTIMIZED_FW_DL == 1) */

/**
   Setup and fill VRX firmware chunks for one partition type.
   Used for for revision 1 (new) for layout type 2.

\param
   pFwDlCtrl      points to the FW dowmload control structure.
\param
   pFwPartition   points to the FW binary partition
\param
   partitionSize  Fw binary partition size
\param
   partitionType  Fw binary partition type
\param
   *chunkIdx      chunk index (common index sequence for all partitions)
\param
   bInternCall    - indicates if the call is form the internal interface
                    (image and data already in kernel space)

\return
   IFX_SUCCESS
   IFX_ERROR
*/
static IFX_int32_t MEI_VRX_PartitionChunksFill(
                                 MEI_FW_DOWNLOAD_CNTRL_T *pFwDlCtrl,
                                 unsigned char *pFwPartition,
                                 IFX_int32_t partitionSize,
                                 MEI_FW_PARTITION_TYPE partitionType,
#if (MEI_SUPPORT_OPTIMIZED_FW_DL == 1)
                                 MEI_FW_CHUNK_FLAVOUR chunkFlavour,
#endif
                                 IFX_uint32_t *chunkIdx,
                                 IFX_boolean_t bInternCall)
{
   IFX_int32_t ret = IFX_SUCCESS;
   IFX_uint32_t chunkSize_byte, copySize_byte, idx_32bit;
   IFX_uint32_t *pNonCachedChunk;
   IFX_boolean_t continue_chunk = IFX_FALSE;
   IFX_uint32_t prevChunkIdx = -1;

   /* Cache started at bootloader chunk, does not need to allocate it*/
   if (partitionType == eMEI_FW_PARTITION_XDSL_CACHE)
   {
      continue_chunk = IFX_TRUE;
      /* roll back and continue previous chunk */
      (*chunkIdx)--;
   }

   for (; *chunkIdx<pFwDlCtrl->meiMaxChunkCount && partitionSize > 0;(*chunkIdx)++)
   {
      /* Alloc new or use current chunk */
      if (!continue_chunk)
      {
         if (*chunkIdx == MEI_FW_IMAGE_DATA_CHUNK_INDEX+pFwDlCtrl->meiSpecialChunkOffset)
            continue;

         /* Set chunk size [bytes]*/
         chunkSize_byte = (*chunkIdx == pFwDlCtrl->meiMaxChunkCount - 1) ?
                             partitionSize : MEI_FW_IMAGE_CHUNK_SIZE_BYTE;

         /* Check for the maximum allowed chunk size*/
         if (chunkSize_byte > MEI_BAR16_SIZE_BYTE)
         {
            PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
               ("MEI_DRV: chunk[%d] invalid size %d!"MEI_DRV_CRLF,
               *chunkIdx, chunkSize_byte));

            ret = IFX_ERROR;
            break;
         }

         /* Allocate chunk*/
         ret = MEI_VRX_ImageChunkAlloc(pFwDlCtrl, *chunkIdx, chunkSize_byte);
         if (ret != 0)
         {
            PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
               ("MEI_DRV: chunk[%d] size %d allocation failed!"
               MEI_DRV_CRLF, *chunkIdx, chunkSize_byte));

            break;
         }

         if (partitionType == eMEI_FW_PARTITION_BOOTLOADER)
         {
            /* Bootloader must be located at one chunk */
            if (pFwDlCtrl->meiPartitions.bootloader_size > MEI_FW_IMAGE_CHUNK_SIZE_BYTE)
            {
               PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
                  ("MEI_DRV: bootloader size %d is too large!"
                  MEI_DRV_CRLF, pFwDlCtrl->meiPartitions.bootloader_size));
            }

            copySize_byte = partitionSize;
         }
         else
         {
            /* Get number of bytes to fill current chunk*/
            copySize_byte = partitionSize > MEI_FW_IMAGE_CHUNK_SIZE_BYTE ?
                            MEI_FW_IMAGE_CHUNK_SIZE_BYTE : partitionSize;
         }

#if (MEI_SUPPORT_OPTIMIZED_FW_DL == 1)
         pFwDlCtrl->imageChunkCtrl[*chunkIdx].imageChunkFlavour = chunkFlavour;
#endif /* (MEI_SUPPORT_OPTIMIZED_FW_DL == 1)*/
      }
      else
      {
         chunkSize_byte = MEI_FW_IMAGE_CHUNK_SIZE_BYTE -
                                        pFwDlCtrl->meiPartitions.bootloader_size;

         copySize_byte = (IFX_uint32_t)partitionSize > chunkSize_byte ?
                           chunkSize_byte : (IFX_uint32_t)partitionSize;

#if (MEI_SUPPORT_OPTIMIZED_FW_DL == 1)
         if (pFwDlCtrl->imageChunkCtrl[*chunkIdx].imageChunkFlavour != chunkFlavour)
         {
            pFwDlCtrl->imageChunkCtrl[*chunkIdx].imageChunkFlavour = eMEI_FW_CHUNK_XDSL;
         }
#endif /* (MEI_SUPPORT_OPTIMIZED_FW_DL == 1)*/
      }

      pNonCachedChunk =
         (IFX_uint32_t*)(pFwDlCtrl->imageChunkCtrl[*chunkIdx].pImageChunk_aligned);

      if (continue_chunk)
      {
         /* First part of the chunk was filled by bootloader */
         pNonCachedChunk += pFwDlCtrl->meiPartitions.bootloader_size/sizeof(IFX_uint32_t);
         continue_chunk = IFX_FALSE;
      }

      /* Fill chunk */
      if (bInternCall)
      {
         memcpy(pNonCachedChunk, pFwPartition, copySize_byte);
      }
      else
      {
         /* copy data to kernel space */
         if ( MEI_DRVOS_CpyFromUser(pNonCachedChunk, pFwPartition, copySize_byte) == IFX_NULL)
         {
            PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
                  ("MEI_DRV: copy_from_user(chunk[%d], size %d) failed"
                  MEI_DRV_CRLF, *chunkIdx, copySize_byte));

            ret = -e_MEI_ERR_GET_ARG;
            break;
         }
      }

      /* Swap chunk to target order*/
      for (idx_32bit=0; idx_32bit<copySize_byte/sizeof(IFX_uint32_t); idx_32bit++)
      {
         /* Do not swap in case of VR11/VRX518 and xDSL image partition!
            As these memory will be accessed by the DSL Firmware via XDMA
            path and there is no HW support for swapping endianes from host
            (MIPS) order to Firmware binary order (LE) the original (not swapped
            firmware in LE format) shall be stored in SDRAM block.
            ATTENTION:
            Reading these values on a BE system like GRX from host side it will
            return swapped information. However, as this data is transparent to
            the MEI Driver this might only play a role in case of debugging. */
         if (!((MEI_DEVICE_CFG_IS_PLATFORM(e_MEI_DEV_PLATFORM_CONFIG_VR11)) &&
               (partitionType == eMEI_FW_PARTITION_XDSL_IMAGE)))
         {
            pNonCachedChunk[idx_32bit] = SWAP32_BYTE_ORDER(pNonCachedChunk[idx_32bit]);
         }
         else
         {
            if (prevChunkIdx != *chunkIdx)
            {
               prevChunkIdx = *chunkIdx;
               PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL,
                     ("MEI_DRV: Skipping swap to host order for chunk[%d]"
                     MEI_DRV_CRLF, *chunkIdx));
            }
         }
      }

#if (MEI_SUPPORT_OPTIMIZED_FW_DL == 1)
      pFwDlCtrl->imageChunkCtrl[*chunkIdx].imageChunkDataSize_byte += copySize_byte;
#endif /* (MEI_SUPPORT_OPTIMIZED_FW_DL == 1) */

      pFwPartition    += copySize_byte;
      partitionSize   -= chunkSize_byte;
   }

   return ret;
}
/**
   Setup and fill VRX firmware chunks for revision 1 (new) for layout type 2.


\param
   pFwDlCtrl   points to the FW dowmload control structure.
\param
   pFwImage    points to the FW binary image
\param
   bInternCall    - indicates if the call is form the internal interface
                    (image and data already in kernel space)

\return
   IFX_SUCCESS
   IFX_ERROR
*/
static IFX_int32_t MEI_VRX_FMLT2_ChunksFill(
                                 MEI_FW_DOWNLOAD_CNTRL_T *pFwDlCtrl,
                                 unsigned char *pFwImage,
                                 IFX_boolean_t bInternCall)
{
   IFX_int32_t ret = IFX_SUCCESS;
   IFX_uint32_t chunkIdx = 0;

   if (ret == IFX_SUCCESS)
   {
      /* Bootloader */
      pFwDlCtrl->meiPartitions.vDSL_cache_chunk_idx = chunkIdx;
      ret = MEI_VRX_PartitionChunksFill(
               pFwDlCtrl, pFwImage,
               pFwDlCtrl->meiPartitions.bootloader_size,
               eMEI_FW_PARTITION_BOOTLOADER,
#if (MEI_SUPPORT_OPTIMIZED_FW_DL == 1)
               eMEI_FW_CHUNK_XDSL,
#endif
               &chunkIdx, bInternCall);
   }

   if (ret == IFX_SUCCESS)
   {
      /* VDSL cache */
      ret = MEI_VRX_PartitionChunksFill(
               pFwDlCtrl, pFwImage + pFwDlCtrl->meiPartitions.vDSL_cache_offset,
               pFwDlCtrl->meiPartitions.vDSL_cache_size,
               eMEI_FW_PARTITION_XDSL_CACHE,
#if (MEI_SUPPORT_OPTIMIZED_FW_DL == 1)
               eMEI_FW_CHUNK_VDSL,
#endif
               &chunkIdx, bInternCall);
   }

   if (ret == IFX_SUCCESS)
   {
      /* Bootloader */
      pFwDlCtrl->meiPartitions.aDSL_cache_chunk_idx = chunkIdx;
      ret = MEI_VRX_PartitionChunksFill(
               pFwDlCtrl, pFwImage,
               pFwDlCtrl->meiPartitions.bootloader_size,
               eMEI_FW_PARTITION_BOOTLOADER,
#if (MEI_SUPPORT_OPTIMIZED_FW_DL == 1)
               eMEI_FW_CHUNK_XDSL,
#endif
               &chunkIdx, bInternCall);
   }

   if (ret == IFX_SUCCESS)
   {
      /* ADSL cache */
      ret = MEI_VRX_PartitionChunksFill(
               pFwDlCtrl, pFwImage + pFwDlCtrl->meiPartitions.aDSL_cache_offset,
               pFwDlCtrl->meiPartitions.aDSL_cache_size,
               eMEI_FW_PARTITION_XDSL_CACHE,
#if (MEI_SUPPORT_OPTIMIZED_FW_DL == 1)
               eMEI_FW_CHUNK_ADSL,
#endif
               &chunkIdx, bInternCall);
   }

   if (ret == IFX_SUCCESS)
   {
      /* VDSL image */
      pFwDlCtrl->meiPartitions.vDSL_image_chunk_idx = chunkIdx;
      ret = MEI_VRX_PartitionChunksFill(
               pFwDlCtrl, pFwImage + pFwDlCtrl->meiPartitions.vDSL_image_offset,
               pFwDlCtrl->meiPartitions.vDSL_image_size,
               eMEI_FW_PARTITION_XDSL_IMAGE,
#if (MEI_SUPPORT_OPTIMIZED_FW_DL == 1)
               eMEI_FW_CHUNK_VDSL,
#endif
               &chunkIdx, bInternCall);
   }

   if (ret == IFX_SUCCESS)
   {
      /* ADSL image */
      pFwDlCtrl->meiPartitions.aDSL_image_chunk_idx = chunkIdx;
      ret = MEI_VRX_PartitionChunksFill(
               pFwDlCtrl, pFwImage + pFwDlCtrl->meiPartitions.aDSL_image_offset,
               pFwDlCtrl->meiPartitions.aDSL_image_size,
               eMEI_FW_PARTITION_XDSL_IMAGE,
#if (MEI_SUPPORT_OPTIMIZED_FW_DL == 1)
               eMEI_FW_CHUNK_ADSL,
#endif
               &chunkIdx, bInternCall);
   }

#if (MEI_SUPPORT_OPTIMIZED_FW_DL == 1)
   /* set amount of really used/filled chunks */
   if (ret == IFX_SUCCESS)
   {
      pFwDlCtrl->meiUsedChunkCount = chunkIdx;
   }
#endif /* (MEI_SUPPORT_OPTIMIZED_FW_DL == 1) */

   if (ret == IFX_SUCCESS)
   {
      /* Release all unused chunks*/
      for (; chunkIdx<pFwDlCtrl->meiMaxChunkCount; chunkIdx++)
      {
         if (chunkIdx == MEI_FW_IMAGE_DATA_CHUNK_INDEX + pFwDlCtrl->meiSpecialChunkOffset)
            continue;

         MEI_VRX_ImageChunkFree(pFwDlCtrl, chunkIdx);
      }

      if (pFwDlCtrl->dataRegionSize_Byte)
      {
         chunkIdx = MEI_FW_IMAGE_DATA_CHUNK_INDEX + pFwDlCtrl->meiSpecialChunkOffset;
         /* Allocate chunk for external writable DATA region */
         ret = MEI_VRX_ImageChunkAlloc(pFwDlCtrl, chunkIdx,
                                               pFwDlCtrl->dataRegionSize_Byte);

         if (ret != 0)
         {
            PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
               ("MEI_DRV: external DATA region chunk[%02d] size %d allocation failed!"
               MEI_DRV_CRLF, chunkIdx, pFwDlCtrl->dataRegionSize_Byte));
         }
      }
   }

   if (ret == IFX_SUCCESS)
   {
      if (pFwDlCtrl->meiPartitions.debug_data_size)
      {
         chunkIdx = MEI_FW_IMAGE_DEBUG_CHUNK_INDEX + pFwDlCtrl->meiSpecialChunkOffset;
         /* Allocate chunk for external writable DATA region */
         ret = MEI_VRX_ImageChunkAlloc(pFwDlCtrl, chunkIdx,
                                       pFwDlCtrl->meiPartitions.debug_data_size);

         if (ret != 0)
         {
            PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
               ("MEI_DRV: debug DATA region chunk[%02d] size %d allocation failed!"
               MEI_DRV_CRLF, chunkIdx, pFwDlCtrl->meiPartitions.debug_data_size));
         }
      }
   }

   if (ret != IFX_SUCCESS)
   {
      /* Release all chunks*/
      for (chunkIdx=0; chunkIdx<pFwDlCtrl->meiMaxChunkCount; chunkIdx++)
         MEI_VRX_ImageChunkFree(pFwDlCtrl, chunkIdx);
   }

   return ret;
}

/**
   Setup and fill VRX firmware chunks.


\param
   pFwDlCtrl   points to the FW dowmload control structure.
\param
   pFwImage    points to the FW binary image
\param
   bInternCall    - indicates if the call is form the internal interface
                    (image and data already in kernel space)

\return
   IFX_SUCCESS
   IFX_ERROR
*/
static IFX_int32_t MEI_VRX_FMLT0_ChunksFill(
                                 MEI_FW_DOWNLOAD_CNTRL_T *pFwDlCtrl,
                                 unsigned char *pFwImage,
                                 IFX_boolean_t bInternCall)
{
   IFX_int32_t ret = 0, chunkSize_byte;
   IFX_uint32_t chunkIdx, idx_32bit, copySize_byte;
   IFX_int32_t imageSize = (IFX_int32_t)pFwDlCtrl->size_byte;
   unsigned char *pImage = pFwImage;
   IFX_uint32_t *pNonCachedChunk;

   for (chunkIdx=0; chunkIdx<MEI_FW_IMAGE_MAX_CHUNK_COUNT && imageSize > 0;
        chunkIdx++)
   {
      if (chunkIdx == MEI_FW_IMAGE_DATA_CHUNK_INDEX)
         continue;

      /* Set chunk size [bytes]*/
      chunkSize_byte = (chunkIdx == MEI_FW_IMAGE_MAX_CHUNK_COUNT - 1) ?
                          imageSize : MEI_FW_IMAGE_CHUNK_SIZE_BYTE;

      /* Check for the maximum allowed chunk size*/
      if (chunkSize_byte > MEI_BAR16_SIZE_BYTE)
      {
         PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DRV: chunk[%d] invalid size %d!"MEI_DRV_CRLF,
            chunkIdx, chunkSize_byte));

         ret = IFX_ERROR;
         break;
      }

      /* Allocate chunk*/
      ret = MEI_VRX_ImageChunkAlloc(pFwDlCtrl, chunkIdx, chunkSize_byte);
      if (ret != 0)
      {
         PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DRV: chunk[%d] size %d allocation failed!"
            MEI_DRV_CRLF, chunkIdx, chunkSize_byte));

         break;
      }

#if (MEI_SUPPORT_OPTIMIZED_FW_DL == 1)
      /* Set chunk used size (to be compatible with FMLT2 to calc CRC) */
      pFwDlCtrl->imageChunkCtrl[chunkIdx].imageChunkDataSize_byte = chunkSize_byte;
#endif /* (MEI_SUPPORT_OPTIMIZED_FW_DL == 1) */

      /* Get number of bytes to fill current chunk*/
      copySize_byte = imageSize > MEI_FW_IMAGE_CHUNK_SIZE_BYTE ?
                         MEI_FW_IMAGE_CHUNK_SIZE_BYTE : imageSize;

      pNonCachedChunk =
         (IFX_uint32_t*)(pFwDlCtrl->imageChunkCtrl[chunkIdx].pImageChunk_aligned);

      /* Fill chunk */
      if (bInternCall)
      {
         memcpy(pNonCachedChunk, pImage, copySize_byte);
      }
      else
      {
         /* copy data to kernel space */
         if ( MEI_DRVOS_CpyFromUser(pNonCachedChunk, pImage, copySize_byte) == IFX_NULL)
         {
            PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
                  ("MEI_DRV: copy_from_user(chunk[%d], size %d) failed"
                  MEI_DRV_CRLF, chunkIdx, copySize_byte));

            ret = -e_MEI_ERR_GET_ARG;
            break;
         }
      }

      /* Swap chunk to target order*/
      for (idx_32bit=0; idx_32bit<copySize_byte/sizeof(IFX_uint32_t); idx_32bit++)
      {
         pNonCachedChunk[idx_32bit] = SWAP32_BYTE_ORDER(pNonCachedChunk[idx_32bit]);
      }

      pImage    += copySize_byte;
      imageSize -= chunkSize_byte;
   }

#if (MEI_SUPPORT_OPTIMIZED_FW_DL == 1)
   /* Set amount of used chunks => use for CRC calc */
   pFwDlCtrl->meiUsedChunkCount = chunkIdx;
#endif /* (MEI_SUPPORT_OPTIMIZED_FW_DL == 1)*/

   if (ret == 0)
   {
      /* Release all unused chunks*/
      for (; chunkIdx<MEI_FW_IMAGE_MAX_CHUNK_COUNT; chunkIdx++)
      {
         if (chunkIdx == MEI_FW_IMAGE_DATA_CHUNK_INDEX)
            continue;

         MEI_VRX_ImageChunkFree(pFwDlCtrl, chunkIdx);
      }

      if (pFwDlCtrl->dataRegionSize_Byte)
      {
         /* Allocate chunk for external writable DATA region */
         ret = MEI_VRX_ImageChunkAlloc(
                  pFwDlCtrl, MEI_FW_IMAGE_DATA_CHUNK_INDEX,
                  pFwDlCtrl->dataRegionSize_Byte);

         if (ret != 0)
         {
            PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
               ("MEI_DRV: external DATA region chunk[%d] size %d allocation failed!"
               MEI_DRV_CRLF, chunkIdx, pFwDlCtrl->dataRegionSize_Byte));
         }
      }
   }

   if (ret != 0)
   {
      /* Release all chunks*/
      for (chunkIdx=0; chunkIdx<MEI_FW_IMAGE_MAX_CHUNK_COUNT; chunkIdx++)
         MEI_VRX_ImageChunkFree(pFwDlCtrl, chunkIdx);
   }

   return ret;
}

/**
   Check VR10 PDBRAM share access with PPE driver, manage
   to get access in case of PPE busy

\param
   pMeiDev     points to the current VR10

\return
   IFX_SUCCESS
   IFX_ERROR
*/
static IFX_int32_t MEI_VR10_PDBRAM_AccessGet(
                                 MEI_DEV_T *pMeiDev)
{
   IFX_int32_t ret = 0;
   IFX_uint32_t ppe_timeout = 0;

   /* Check PDBRAM ownership */
   if (*MEI_PPE_U32REG(PPE_S_44K_OWN) & 0x1)
   {
      /* PDBRAM is busy by PPE */
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_LOW,
         ("MEI_DRV: waiting PDBRAM access"MEI_DRV_CRLF));

      /* Set FORCE_LINK_DOWN flag for PPE */
      *MEI_PPE_U32REG(PPE_FORCE_LINK_DOWN) |= 0x1;
      /* Try several attemps to access PDBRAM */
      do
      {
         /* check for PDBRAM access timeout */
         if (ppe_timeout >= MEI_CFG_DEF_WAIT_FOR_PDBRAM_ACCESS_TOTAL)
         {
            /* Clear FORCE_LINK_DOWN flag for PPE */
            *MEI_PPE_U32REG(PPE_FORCE_LINK_DOWN) &= ~0x1;

            PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
               ("MEI_DRV: PDBRAM access timeout"MEI_DRV_CRLF));

            return (-e_MEI_ERR_PDBRAM_LOCKED);
         }
         /* Sleep some time... */
         MEI_DRVOS_Wait_ms(MEI_CFG_DEF_WAIT_FOR_PDBRAM_ACCESS_ATTEMPT);
         ppe_timeout += MEI_CFG_DEF_WAIT_FOR_PDBRAM_ACCESS_ATTEMPT;
      } while (*MEI_PPE_U32REG(PPE_S_44K_OWN) & 0x1);
   }

   /* PDBRAM is free, we could use it */
   PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_LOW,
      ("MEI_DRV: PDBRAM access granted"MEI_DRV_CRLF));

   return ret;
}

/**
   Fill VR10 PDBRAM

\param
   pMeiDev          points to the current VR10
   bootloader_size  size in bytes need to copy into ARC

\return
   IFX_SUCCESS
   IFX_ERROR
*/
static IFX_int32_t MEI_VR1x_PDBRAM_Fill(
                                 MEI_DEV_T *pMeiDev, IFX_uint32_t bootloader_size)
{
   IFX_int32_t ret = 0;
   MEI_FW_IMAGE_CHUNK_CTRL_T *pChunks = pMeiDev->fwDl.imageChunkCtrl;
   MEI_FW_PORT_MODE_CONTROL_DMA32_T fwPortModeCtrl = {0};
   IFX_uint8_t *pvPDBRAM = (IFX_uint8_t *)MEI_DRV_PDBRAM_VIRT_ADDR_GET(pMeiDev);
   IFX_uint8_t *ppPDBRAM = (IFX_uint8_t *)MEI_DRV_PDBRAM_PHY_ADDR_GET(pMeiDev);
   IFX_uint8_t *pChunk;
   IFX_uint32_t cacheSize_byte = MEI_PDBRAM_CACHE_SIZE_BYTE;
   IFX_uint32_t copySize_byte, chunkIdx = 0;
   IFX_int32_t bootloader_chunk = IFX_TRUE;
#if (MEI_SUPPORT_DEVICE_VR11 == 1)
   /* ADSL cache offset: BOOTLOADER_SIZE + 128k (chunk 2) */
   IFX_uint32_t adslChunkOffset = 2;
#else
   /* ADSL cache offset: BOOTLOADER_SIZE + 192k (chunk 3) */
   IFX_uint32_t adslChunkOffset = 3;
#endif

   if (!bootloader_size)
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DRV: Bootloader size is zero!"
            MEI_DRV_CRLF));
      return IFX_ERROR;
   }

   if (bootloader_size > MEI_FW_MAX_BOOTLOADER_SIZE_BYTE)
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DRV: Bootloader size %d bytes is too large (max size %d)!"
            MEI_DRV_CRLF, bootloader_size, MEI_FW_MAX_BOOTLOADER_SIZE_BYTE));
      return IFX_ERROR;
   }

   /* Get current FW Port Mode Control Structure*/
   ret = MEI_VRX_PortModeControlStructureCurrentGet(pMeiDev, &fwPortModeCtrl);
   if (ret != IFX_SUCCESS)
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DRV[%02d]: FW Port Mode Control structure get failed!" MEI_DRV_CRLF,
              MEI_DRV_LINENUM_GET(pMeiDev) ));

      return (-e_MEI_ERR_OP_FAILED);
   }

   if ((fwPortModeCtrl.xDslModeCurrent != MEI_FW_XDSL_MODE_VDSL) &&
      (fwPortModeCtrl.xDslModeCurrent != MEI_FW_XDSL_MODE_ADSL))
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DRV[%02d]: FW Port Mode Control structure: no current XDSL mode!"
              MEI_DRV_CRLF, MEI_DRV_LINENUM_GET(pMeiDev) ));

      return (-e_MEI_ERR_INVAL_CONFIG);
   }

   PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL,
         (MEI_DRV_CRLF "================== FILL PDBRAM ===================" MEI_DRV_CRLF));


   /* Copy bootloader from the first part of chunk 0, offset 0 */
   pChunk = (IFX_uint8_t*)(pChunks[chunkIdx].pImageChunk_aligned);
   copySize_byte = bootloader_size;

   PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL,
         ("================== BOOTLOADER ====================" MEI_DRV_CRLF));

   do {
      memcpy(pvPDBRAM, pChunk, copySize_byte);

      PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL,
         ("Dest (PDBRAM, addr %p), Src (chunk[%02d], addr %p), copy %i (0x%x) bytes" MEI_DRV_CRLF,
         ppPDBRAM, chunkIdx, pChunk, copySize_byte, copySize_byte));

      pvPDBRAM += copySize_byte;
      ppPDBRAM += copySize_byte;
      cacheSize_byte -= copySize_byte;

      if (bootloader_chunk)
      {
         /* VDSL cache offset: BOOTLOADER_SIZE (chunk 0) */
         chunkIdx = fwPortModeCtrl.xDslModeCurrent == MEI_FW_XDSL_MODE_VDSL ?
                        0 : adslChunkOffset;

         /* Copy XDSL cache from the second part of the chunk chunkIdx
                                       (VDSL: chunkIdx=0; ADSL: chunkIdx=3) */
         pChunk = (IFX_uint8_t*)(pChunks[chunkIdx].pImageChunk_aligned);
         /* Cache starts in non-zero offset of chunk */
         pChunk += bootloader_size;
         copySize_byte = MEI_FW_IMAGE_CHUNK_SIZE_BYTE - bootloader_size;

         PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL,
            ("===================== CACHE ======================" MEI_DRV_CRLF));

         bootloader_chunk = IFX_FALSE;
      }
      else
      {
         /* get next chunk */
         pChunk = (IFX_uint8_t*)(pChunks[++chunkIdx].pImageChunk_aligned);
         copySize_byte = cacheSize_byte > MEI_FW_IMAGE_CHUNK_SIZE_BYTE ?
                        MEI_FW_IMAGE_CHUNK_SIZE_BYTE : cacheSize_byte;
      }
   } while (cacheSize_byte);

   PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL,
         ("==================================================" MEI_DRV_CRLF MEI_DRV_CRLF));

   return ret;
}

/**
   Update VR9/VR10/VR11 BAR registers for new fw revision layout type 2.

\param
   pMeiDev     points to the current VR9/VR10/VR11/AR9 channel device.

\return
   IFX_SUCCESS
   IFX_ERROR
*/
static IFX_int32_t MEI_VRX_FMLT2_BarRegistersUpdate(
                                 MEI_DEV_T *pMeiDev)
{
   IFX_int32_t ret = 0;
   IFX_uint32_t chunkIdx, chunks, barIdx, last_used_chunkIdx;
   MEI_MEI_DRV_CNTRL_T *pMeiDrvCntrl = &(pMeiDev->meiDrvCntrl);
   MEI_FW_DOWNLOAD_CNTRL_T *pFwDlCtrl = &(pMeiDev->fwDl);
   MEI_FW_IMAGE_CHUNK_CTRL_T *pChunk = pFwDlCtrl->imageChunkCtrl;
   MEI_FW_PORT_MODE_CONTROL_DMA32_T fwPortModeCtrl = {0};
   IFX_uint32_t xDSL_image_chunk_idx, xDSL_image_chunks;
   IFX_uint32_t xDSL_cache_chunk_idx, xDSL_cache_chunks;
   IFX_uint32_t part_size;
#if (MEI_SUPPORT_DEVICE_VR11 == 1)
   IFX_uint8_t *ppPDBRAM =
      (IFX_uint8_t *)(MEI_DRV_PCIE_PHY_MEMBASE_GET(&pMeiDev->meiDrvCntrl)
                         + MEI_PDBRAM_OFFSET);
#else
   IFX_uint8_t *ppPDBRAM =
      (IFX_uint8_t *)(MEI_INTERNAL_ADDRESS_BASE + MEI_PDBRAM_OFFSET);
#endif

   if (pFwDlCtrl->eFwMemLayoutType != eMEI_FW_MEM_LAYOUT_TYPE_2)
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DRV[%02d]: memory layout type %d is not supported!" MEI_DRV_CRLF,
              MEI_DRV_LINENUM_GET(pMeiDev), pFwDlCtrl->eFwMemLayoutType));

      return (-e_MEI_ERR_INVAL_CONFIG);
   }

   /* Get current FW Port Mode Control Structure*/
   ret = MEI_VRX_PortModeControlStructureCurrentGet(pMeiDev, &fwPortModeCtrl);
   if (ret != IFX_SUCCESS)
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DRV[%02d]: FW Port Mode Control structure get failed!" MEI_DRV_CRLF,
              MEI_DRV_LINENUM_GET(pMeiDev) ));

      return (-e_MEI_ERR_OP_FAILED);
   }

   if ((fwPortModeCtrl.xDslModeCurrent != MEI_FW_XDSL_MODE_VDSL) &&
      (fwPortModeCtrl.xDslModeCurrent != MEI_FW_XDSL_MODE_ADSL))
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DRV[%02d]: FW Port Mode Control structure: no current XDSL mode!"
              MEI_DRV_CRLF, MEI_DRV_LINENUM_GET(pMeiDev) ));

      return (-e_MEI_ERR_INVAL_CONFIG);
   }

   /* Get xDSL image size and started chunk index */
   if (fwPortModeCtrl.xDslModeCurrent == MEI_FW_XDSL_MODE_VDSL)
   {
      /* Started chunk for VDSL full image */
      xDSL_image_chunk_idx  = pFwDlCtrl->meiPartitions.vDSL_image_chunk_idx;

      part_size = pFwDlCtrl->meiPartitions.vDSL_image_size;
      xDSL_image_chunks = part_size / MEI_FW_IMAGE_CHUNK_SIZE_BYTE
                                + !!(part_size % MEI_FW_IMAGE_CHUNK_SIZE_BYTE);

      /* Started chunk for VDSL cache */
      xDSL_cache_chunk_idx  = pFwDlCtrl->meiPartitions.vDSL_cache_chunk_idx;

      part_size = pFwDlCtrl->meiPartitions.bootloader_size +
                                        pFwDlCtrl->meiPartitions.vDSL_cache_size;
      xDSL_cache_chunks = part_size / MEI_FW_IMAGE_CHUNK_SIZE_BYTE
                                + !!(part_size % MEI_FW_IMAGE_CHUNK_SIZE_BYTE);
   }
   else
   {
      /* Started chunk for ADSL full image */
      xDSL_image_chunk_idx  = pFwDlCtrl->meiPartitions.aDSL_image_chunk_idx;

      part_size = pFwDlCtrl->meiPartitions.aDSL_image_size;
      xDSL_image_chunks = part_size / MEI_FW_IMAGE_CHUNK_SIZE_BYTE
                                + !!(part_size % MEI_FW_IMAGE_CHUNK_SIZE_BYTE);

      /* Started chunk for ADSL cache */
      xDSL_cache_chunk_idx  = pFwDlCtrl->meiPartitions.aDSL_cache_chunk_idx;

      part_size = pFwDlCtrl->meiPartitions.bootloader_size +
                                        pFwDlCtrl->meiPartitions.aDSL_cache_size;
      xDSL_cache_chunks = part_size / MEI_FW_IMAGE_CHUNK_SIZE_BYTE
                                + !!(part_size % MEI_FW_IMAGE_CHUNK_SIZE_BYTE);
   }

   if (! MEI_DEVICE_CFG_IS_PLATFORM(e_MEI_DEV_PLATFORM_CONFIG_VR9))
   {
      if (xDSL_cache_chunks > MEI_MAX_CACHE_CHUNK_COUNT)
      {
         PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DRV: bootloader+cache size 0x%X are too big for PDBRAM!"
            MEI_DRV_CRLF, part_size));
      }
   }


   PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL,
         (MEI_DRV_CRLF "================== BAR REGS INIT =================" MEI_DRV_CRLF));

   /* Clear all BAR regs (except debug)*/
   for (barIdx=0; barIdx<MEI_TOTAL_BAR_REGISTER_COUNT + MEI_TOTAL_BAR_REGISTER_COUNT2; barIdx++)
   {
#if (MEI_PREDEF_DBG_BAR == 1)
      if (MEI_BAR_TYPE_GET(pMeiDev, barIdx) != eMEI_BAR_TYPE_USER)
      {
#endif /* (MEI_PREDEF_DBG_BAR == 1) */
         /* Write unused BARx register with the BAR0 content*/
         MEI_REG_ACCESS_ME_XMEM_BAR_SET(pMeiDrvCntrl, barIdx, pChunk[0].pBARx);
         MEI_BAR_TYPE_SET(pMeiDev, barIdx, eMEI_BAR_TYPE_UNUSED);
#if (MEI_PREDEF_DBG_BAR == 1)
      }
      else
      {
         /* reload dbg addr after reset */
         MEI_REG_ACCESS_ME_XMEM_BAR_SET(pMeiDrvCntrl, barIdx,
                        MEI_BAR_DBG_ADDR_GET(pMeiDev, barIdx));
      }
#endif /* (MEI_PREDEF_DBG_BAR == 1) */
   }

   /* init BAR->cache */
   for (chunkIdx=xDSL_cache_chunk_idx, barIdx=0, chunks=0;
                            chunks < xDSL_cache_chunks; chunkIdx++, barIdx++, chunks++)
   {
      if (! MEI_DEVICE_CFG_IS_PLATFORM(e_MEI_DEV_PLATFORM_CONFIG_VR9))
      {
         /* BAR0, BAR1 (, BAR2) are pointing to the PDBRAM */
         MEI_REG_ACCESS_ME_XMEM_BAR_SET(pMeiDrvCntrl, barIdx, ppPDBRAM);
         MEI_BAR_TYPE_SET(pMeiDev, barIdx, eMEI_BAR_TYPE_PDBRAM);
         PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL,
            ("BAR[%02d] = 0x%p (-> PDBRAM+0x%05X)" MEI_DRV_CRLF, barIdx,
            ppPDBRAM, barIdx*MEI_FW_IMAGE_CHUNK_SIZE_BYTE));
         ppPDBRAM += MEI_FW_IMAGE_CHUNK_SIZE_BYTE;
         continue;
      }

      /* Skip unused chunks*/
      if (pChunk[chunkIdx].eImageChunkType == eMEI_FW_IMAGE_CHUNK_UNDEFINED)
         continue;

      /* Write BARx register with the chunk address*/
      MEI_REG_ACCESS_ME_XMEM_BAR_SET(pMeiDrvCntrl, barIdx, pChunk[chunkIdx].pBARx);
      MEI_BAR_TYPE_SET(pMeiDev, barIdx, eMEI_BAR_TYPE_CHUNK);
      PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL,
            ("BAR[%02d] = 0x%08X (-> chunk[%02d])" MEI_DRV_CRLF, barIdx,
            MEI_REG_ACCESS_ME_XMEM_BAR_GET(pMeiDrvCntrl, barIdx), chunkIdx));
   }

   /* init BAR->image */
   for (chunkIdx=xDSL_image_chunk_idx, chunks=0;
                            chunks < xDSL_image_chunks; chunkIdx++, barIdx++, chunks++)
   {
      /* Skip unused chunks*/
      if (pChunk[chunkIdx].eImageChunkType == eMEI_FW_IMAGE_CHUNK_UNDEFINED)
         continue;

      /* Write BARx register with the chunk address*/
      MEI_REG_ACCESS_ME_XMEM_BAR_SET(pMeiDrvCntrl, barIdx, pChunk[chunkIdx].pBARx);
      MEI_BAR_TYPE_SET(pMeiDev, barIdx, eMEI_BAR_TYPE_CHUNK);
      PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL,
            ("BAR[%02d] = 0x%08X (-> chunk[%02d])" MEI_DRV_CRLF, barIdx,
            MEI_REG_ACCESS_ME_XMEM_BAR_GET(pMeiDrvCntrl, barIdx), chunkIdx));
   }

   /* unused BAR registers shall be set to the last used firmware A/VDSL fullimage chunk */
   last_used_chunkIdx = chunkIdx - 1;
   for (barIdx=0; barIdx<MEI_TOTAL_BAR_REGISTER_COUNT + MEI_TOTAL_BAR_REGISTER_COUNT2; barIdx++)
   {
      if (MEI_BAR_TYPE_GET(pMeiDev, barIdx) == eMEI_BAR_TYPE_UNUSED)
      {
         MEI_REG_ACCESS_ME_XMEM_BAR_SET(pMeiDrvCntrl, barIdx, pChunk[last_used_chunkIdx].pBARx);
      }
   }

#if (MEI_SUPPORT_DSM == 1)
#if (MEI_SUPPORT_DEVICE_VR11 != 1)
   /* chunkIdx points to ERB block */
   barIdx = MEI_FW_IMAGE_ERB_CHUNK_INDEX;
   chunkIdx = barIdx + pFwDlCtrl->meiSpecialChunkOffset;

   if ((pChunk[chunkIdx].eImageChunkType != eMEI_FW_IMAGE_CHUNK_UNDEFINED) &&
      (pChunk[chunkIdx].eImageChunkType != eMEI_FW_IMAGE_CHUNK_ERB))
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_WRN,
         ("MEI_DRV: could not init ERB BAR[%02d], busy by firmware!"
         MEI_DRV_CRLF, chunkIdx));
   }

   /* Update BAR register pointed to ERB block */
   MEI_REG_ACCESS_ME_XMEM_BAR_SET(pMeiDrvCntrl, barIdx,
                          MEI_VRX_TranslateMipsToArc(pMeiDev->meiERBbuf.pERB_virt,
                                                     (MEI_DRVOS_DMA_T)pMeiDev->meiERBbuf.pERB_phy));
   MEI_BAR_TYPE_SET(pMeiDev, barIdx, eMEI_BAR_TYPE_ERB);
   PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL,
      ("BAR[%02d] = 0x%08X (-> ERB block)"MEI_DRV_CRLF, barIdx,
      MEI_REG_ACCESS_ME_XMEM_BAR_GET(pMeiDrvCntrl, barIdx)));

   pChunk[chunkIdx].eImageChunkType = eMEI_FW_IMAGE_CHUNK_ERB;
#else /* (MEI_SUPPORT_DEVICE_VR11 != 1) */
   if (!pMeiDev->bIsSetErb)
   {
      MEI_VR11_ErbBarSet(pMeiDev,
                         (unsigned int)(pChunk[last_used_chunkIdx].pBARx),
                         (unsigned int)(pChunk[last_used_chunkIdx].pBARx));
   }
   else
   {
      MEI_VR11_ErbBarSet(pMeiDev, pMeiDev->bar14, pMeiDev->bar17);
   }

#endif
#endif /* (MEI_SUPPORT_DSM == 1) */

#if (MEI_SUPPORT_DEVICE_VR11 == 1)
   barIdx = MEI_FW_IMAGE_CHIPID_EFUSE_CHUNK_INDEX;

   MEI_REG_ACCESS_ME_XMEM_BAR_SET(pMeiDrvCntrl, barIdx,
      (MEI_DRV_PCIE_PHY_MEMBASE_GET(&pMeiDev->meiDrvCntrl) +
      MEI_CHIPID_EFUSE_OFFSET));
   MEI_BAR_TYPE_SET(pMeiDev, barIdx, eMEI_BAR_TYPE_SPECIAL);
   PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL,
      ("BAR[%02d] = 0x%08x (-> CHIPID_EFUSE + GPIO_FUNC)" MEI_DRV_CRLF, barIdx,
      (MEI_DRV_PCIE_PHY_MEMBASE_GET(&pMeiDev->meiDrvCntrl) +
      MEI_CHIPID_EFUSE_OFFSET)));

   barIdx = MEI_FW_IMAGE_FREQ_SCAL_PPE_CHUNK_INDEX;

   MEI_REG_ACCESS_ME_XMEM_BAR_SET(pMeiDrvCntrl, barIdx,
      (MEI_DRV_PCIE_PHY_MEMBASE_GET(&pMeiDev->meiDrvCntrl) +
      MEI_CGU_OFFSET));
   MEI_BAR_TYPE_SET(pMeiDev, barIdx, eMEI_BAR_TYPE_SPECIAL);
   PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL,
      ("BAR[%02d] = 0x%08x (-> CGU / PLL_OMCFG)" MEI_DRV_CRLF, barIdx,
      (MEI_DRV_PCIE_PHY_MEMBASE_GET(&pMeiDev->meiDrvCntrl) +
      MEI_CGU_OFFSET)));

   pMeiDev->barSafeAddr = (IFX_uint32_t)(pChunk[last_used_chunkIdx].pBARx);

#endif /* (MEI_SUPPORT_DEVICE_VR11 == 1) */

   /* Check for the valid DATA chunk*/
   barIdx = MEI_FW_IMAGE_DATA_CHUNK_INDEX;
   chunkIdx = barIdx + pFwDlCtrl->meiSpecialChunkOffset;
   if (pChunk[chunkIdx].eImageChunkType != eMEI_FW_IMAGE_CHUNK_DATA)
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_WRN,
         ("MEI_DRV: external DATA region chunk[%02d] not specified!"
         MEI_DRV_CRLF, chunkIdx));
   }
   else
   {
      MEI_REG_ACCESS_ME_XMEM_BAR_SET(pMeiDrvCntrl, barIdx, pChunk[chunkIdx].pBARx);
      MEI_BAR_TYPE_SET(pMeiDev, barIdx, eMEI_BAR_TYPE_SPECIAL);
      PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL,
         ("BAR[%02d] = 0x%08X (-> chunk[%02d])" MEI_DRV_CRLF, barIdx,
         MEI_REG_ACCESS_ME_XMEM_BAR_GET(pMeiDrvCntrl, barIdx), chunkIdx));

      /* Write Shadow register with the DATA chunk address*/
      MEI_REG_ACCESS_ME_XDATA_BASE_SH_SET(pMeiDrvCntrl,
         pChunk[chunkIdx].pBARx);
   }

   /* Check for the valid DEBUG chunk*/
   barIdx = MEI_FW_IMAGE_DEBUG_CHUNK_INDEX;
   chunkIdx = barIdx + pFwDlCtrl->meiSpecialChunkOffset;
   if (pChunk[chunkIdx].eImageChunkType != eMEI_FW_IMAGE_CHUNK_UNDEFINED)
   {
      MEI_REG_ACCESS_ME_XMEM_BAR_SET(pMeiDrvCntrl, barIdx, pChunk[chunkIdx].pBARx);
      MEI_BAR_TYPE_SET(pMeiDev, barIdx, eMEI_BAR_TYPE_SPECIAL);
      PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL,
         ("BAR[%02d] = 0x%08X (-> chunk[%02d])" MEI_DRV_CRLF, barIdx,
         MEI_REG_ACCESS_ME_XMEM_BAR_GET(pMeiDrvCntrl, barIdx), chunkIdx));
   }

   PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL,
         ("==================================================" MEI_DRV_CRLF MEI_DRV_CRLF));

   return ret;
}

/**
   Update VR9/VR10/AR9 BAR registers.

\param
   pMeiDev     points to the current VR9/VR10/AR9 channel device.

\return
   IFX_SUCCESS
   IFX_ERROR
*/
static IFX_int32_t MEI_VRX_FMLT0_BarRegistersUpdate(
                                 MEI_DEV_T *pMeiDev)
{
   IFX_int32_t ret = 0;
   IFX_uint32_t chunkIdx;
   MEI_MEI_DRV_CNTRL_T *pMeiDrvCntrl = &(pMeiDev->meiDrvCntrl);
   MEI_FW_DOWNLOAD_CNTRL_T *pFwDlCtrl = &(pMeiDev->fwDl);
   MEI_FW_IMAGE_CHUNK_CTRL_T *pChunk = pFwDlCtrl->imageChunkCtrl;
#if (MEI_SUPPORT_DEVICE_VR11 == 1)
   IFX_uint8_t *ppPDBRAM =
      (IFX_uint8_t *)(MEI_DRV_PCIE_PHY_MEMBASE_GET(&pMeiDev->meiDrvCntrl)
                         + MEI_PDBRAM_OFFSET);
#else
   IFX_uint8_t *ppPDBRAM =
      (IFX_uint8_t *)(MEI_INTERNAL_ADDRESS_BASE + MEI_PDBRAM_OFFSET);
#endif


   PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL,
         (MEI_DRV_CRLF "================== BAR REGS INIT =================" MEI_DRV_CRLF));

   for (chunkIdx=0; chunkIdx<MEI_FW_IMAGE_MAX_CHUNK_COUNT; chunkIdx++)
   {
      if (! MEI_DEVICE_CFG_IS_PLATFORM(e_MEI_DEV_PLATFORM_CONFIG_VR9))
      {
         /* BAR0, BAR1 (, BAR2) are pointing to the PDBRAM */
         if (chunkIdx < MEI_MAX_CACHE_CHUNK_COUNT)
         {
            MEI_REG_ACCESS_ME_XMEM_BAR_SET(pMeiDrvCntrl, chunkIdx, ppPDBRAM);
            MEI_BAR_TYPE_SET(pMeiDev, chunkIdx, eMEI_BAR_TYPE_PDBRAM);
            PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL,
               ("BAR[%02d] = %p" MEI_DRV_CRLF, chunkIdx, ppPDBRAM));
            ppPDBRAM += MEI_FW_IMAGE_CHUNK_SIZE_BYTE;
            continue;
         }
      }

      /* Skip unused chunks*/
      if (pChunk[chunkIdx].eImageChunkType == eMEI_FW_IMAGE_CHUNK_UNDEFINED)
         continue;

      /* Write BARx register with the chunk address*/
      MEI_REG_ACCESS_ME_XMEM_BAR_SET(pMeiDrvCntrl, chunkIdx, pChunk[chunkIdx].pBARx);
      MEI_BAR_TYPE_SET(pMeiDev, chunkIdx, eMEI_BAR_TYPE_CHUNK);

      PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL,
            ("BAR[%02d] = 0x%08X" MEI_DRV_CRLF, chunkIdx,
            MEI_REG_ACCESS_ME_XMEM_BAR_GET(pMeiDrvCntrl, chunkIdx)));
   }

#if (MEI_SUPPORT_DSM == 1)
#if (MEI_SUPPORT_DEVICE_VR11 != 1)
   /* chunkIdx points to ERB block */
   chunkIdx = MEI_FW_IMAGE_ERB_CHUNK_INDEX;
   if ((pChunk[chunkIdx].eImageChunkType != eMEI_FW_IMAGE_CHUNK_UNDEFINED) &&
      (pChunk[chunkIdx].eImageChunkType != eMEI_FW_IMAGE_CHUNK_ERB))
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_WRN,
         ("MEI_DRV: could not init ERB BAR[%02d], busy by firmware!"
         MEI_DRV_CRLF, chunkIdx));
   }

   /* Update BAR register pointed to ERB block */
   MEI_REG_ACCESS_ME_XMEM_BAR_SET(pMeiDrvCntrl, chunkIdx,
                          MEI_VRX_TranslateMipsToArc(pMeiDev->meiERBbuf.pERB_virt,
                                                     (MEI_DRVOS_DMA_T)pMeiDev->meiERBbuf.pERB_phy));
   MEI_BAR_TYPE_SET(pMeiDev, chunkIdx, eMEI_BAR_TYPE_ERB);
   PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL, ("BAR[%02d] = 0x%08X" MEI_DRV_CRLF,
            chunkIdx, MEI_REG_ACCESS_ME_XMEM_BAR_GET(pMeiDrvCntrl, chunkIdx)));

   pChunk[chunkIdx].eImageChunkType = eMEI_FW_IMAGE_CHUNK_ERB;
#endif /* (MEI_SUPPORT_DSM == 1) */
#endif /* (MEI_SUPPORT_DEVICE_VR11 != 1) */

   for (chunkIdx=0; chunkIdx<MEI_TOTAL_BAR_REGISTER_COUNT + MEI_TOTAL_BAR_REGISTER_COUNT2; chunkIdx++)
   {
      /* Fill unused chunks*/
      if (pChunk[chunkIdx].eImageChunkType == eMEI_FW_IMAGE_CHUNK_UNDEFINED)
      {
#if (MEI_PREDEF_DBG_BAR == 1)
         if (MEI_BAR_TYPE_GET(pMeiDev, chunkIdx) != eMEI_BAR_TYPE_USER)
         {
#endif /* (MEI_PREDEF_DBG_BAR == 1) */
            /* Write unused BARx register with the BAR0 content*/
            MEI_REG_ACCESS_ME_XMEM_BAR_SET(pMeiDrvCntrl, chunkIdx, pChunk[0].pBARx);
            MEI_BAR_TYPE_SET(pMeiDev, chunkIdx, eMEI_BAR_TYPE_UNUSED);
#if (MEI_PREDEF_DBG_BAR == 1)
         }
         else
         {
            /* reload dbg addr after reset */
            MEI_REG_ACCESS_ME_XMEM_BAR_SET(pMeiDrvCntrl, chunkIdx,
                           MEI_BAR_DBG_ADDR_GET(pMeiDev, chunkIdx));
         }
#endif /* (MEI_PREDEF_DBG_BAR == 1) */
      }
   }


   /* Check for the valid DATA chunk*/
   if (pChunk[MEI_FW_IMAGE_DATA_CHUNK_INDEX].eImageChunkType != eMEI_FW_IMAGE_CHUNK_DATA)
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_WRN,
         ("MEI_DRV: external DATA region chunk[%02d] not specified!"
         MEI_DRV_CRLF, MEI_FW_IMAGE_DATA_CHUNK_INDEX));
   }
   else
   {
      /* Write Shadow register with the DATA chunk address*/
      MEI_REG_ACCESS_ME_XDATA_BASE_SH_SET(pMeiDrvCntrl,
         pChunk[MEI_FW_IMAGE_DATA_CHUNK_INDEX].pBARx);
      MEI_BAR_TYPE_SET(pMeiDev, MEI_FW_IMAGE_DATA_CHUNK_INDEX, eMEI_BAR_TYPE_SPECIAL);
   }

   PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL,
         ("==================================================" MEI_DRV_CRLF MEI_DRV_CRLF));

   return ret;
}

/**
   Read bootloader size from image header.

\param
   pMeiDev     points to the current VR9/VR10/AR9 channel device.

\param
   pVal        bootloader size.

\return
   IFX_SUCCESS
   IFX_ERROR
*/
static IFX_int32_t MEI_VRX_BootLoaderSizeGet(
                                 MEI_DEV_T *pMeiDev, IFX_uint32_t *pVal)
{
   IFX_int32_t ret = 0;
   MEI_FW_DOWNLOAD_CNTRL_T *pFwDlCtrl = &(pMeiDev->fwDl);
   MEI_FW_IMAGE_CHUNK_CTRL_T *pChunk = pFwDlCtrl->imageChunkCtrl;
   MEI_FW_IMAGE_CNTRL_T *pFwImageHeader = NULL;

   if (pFwDlCtrl->eFwRevision == eMEI_FW_INTERFACE_REV_0)
   {
      /* Set Image Header pointer. By default Image Header is located in the
         1st chunk*/
      pFwImageHeader = (MEI_FW_IMAGE_CNTRL_T*)(pChunk->pImageChunk_aligned);

      if (!pFwImageHeader)
      {
         PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DRV: No chunk specified to extract FW image header!"
            MEI_DRV_CRLF));
         return IFX_ERROR;
      }

      /*
         All further handlings are performed on the swapped image
      */
      if (MEI_BOOTLOADER_SIZE_PAGE >= pFwImageHeader->imageNumOfPages)
      {
         PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_WRN,
            ("MEI_DRV: Could not read image header bootloader page %d, total pages %d"
            MEI_DRV_CRLF, MEI_BOOTLOADER_SIZE_PAGE, pFwImageHeader->imageNumOfPages));
         return IFX_ERROR;
      }

      *pVal = pFwImageHeader->imagePage.imagePageX[MEI_BOOTLOADER_SIZE_PAGE].codePageSize_32Bit;
   }
   else
   {
      *pVal = pFwDlCtrl->meiPartitions.bootloader_size;
   }

   PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL,
      ("MEI_DRV: read bootloader size %d bytes"
      MEI_DRV_CRLF, *pVal));

   if (! MEI_DEVICE_CFG_IS_PLATFORM(e_MEI_DEV_PLATFORM_CONFIG_VR9))
   {
      if (*pVal == 0)
      {
         /* set default value */
         *pVal = MEI_FW_DEFAULT_BOOTLOADER_SIZE_BYTE;

         PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_WRN,
            ("MEI_DRV: bootloader size is zero, use default size %d bytes"
            MEI_DRV_CRLF, *pVal));
      }
   }

   return ret;
}

/**
   Download VR9/VR10/AR9 initial boot pages.

\param
   pMeiDev     points to the current VR9/VR10/AR9 channel device.

\return
   IFX_SUCCESS
   IFX_ERROR
*/
static IFX_int32_t MEI_VRX_BootPagesDownload(
                                 MEI_DEV_T *pMeiDev)
{
   IFX_int32_t ret = 0;
   IFX_uint32_t pageIdx;
   MEI_MEI_DRV_CNTRL_T *pMeiDrvCntrl = &(pMeiDev->meiDrvCntrl);
   MEI_FW_DOWNLOAD_CNTRL_T *pFwDlCtrl = &(pMeiDev->fwDl);
   MEI_FW_IMAGE_CHUNK_CTRL_T *pChunk = pFwDlCtrl->imageChunkCtrl;
   MEI_FW_IMAGE_CNTRL_T *pFwImageHeader = NULL;

   /* Set Image Header pointer. By default Image Header is located in the
      1st chunk*/
   pFwImageHeader = (MEI_FW_IMAGE_CNTRL_T*)(pChunk->pImageChunk_aligned);

   if (!pFwImageHeader)
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
         ("MEI_DRV: No chunk specified to extract FW image header!"
         MEI_DRV_CRLF));
      return IFX_ERROR;
   }

   /*
      All further handlings are performed on the swapped image
   */

   /* Check for the valid chunk#0 size*/
   if (pFwImageHeader->imageNumOfPages * sizeof(MEI_FW_IMAGE_PAGE_T) > pChunk->imageChunkSize_byte)
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
         ("MEI_DRV: Chunk#0 size is not enough to extract FW image header info!"
         MEI_DRV_CRLF));
      return IFX_ERROR;
   }

   /*
      Write Code/Data pages via DMA
      - Protect device DMA access
   */
   MEI_DRV_GET_UNIQUE_ACCESS(pMeiDev);

   /* Skip dummy Page#0*/
   for (pageIdx=1; pageIdx<pFwImageHeader->imageNumOfPages; pageIdx++)
   {
      if (pFwImageHeader->imagePage.imagePageX[pageIdx].codePageSize_32Bit & MEI_BOOT_FLAG)
      {
         PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_LOW,
            ("MEI_DRV: BOOT Code Page[%d] download, size = %5d [32-bit]"MEI_DRV_CRLF, pageIdx,
            (pFwImageHeader->imagePage.imagePageX[pageIdx].codePageSize_32Bit) & (~MEI_BOOT_FLAG)));

         /* Fill Xpage with the PROGRAM Memory Data*/
         ret = MEI_VRX_XpageWrite(
                  pFwDlCtrl, pMeiDrvCntrl, pChunk,
                  &(pFwImageHeader->imagePage.imagePageX[pageIdx]), IFX_FALSE);

         if (ret != 0)
            break;
      }

      if (pFwImageHeader->imagePage.imagePageX[pageIdx].dataPageSize_32Bit & MEI_BOOT_FLAG)
      {
         PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_LOW,
            ("MEI_DRV: BOOT Data Page[%d] download, size = %5d [32-bit]"MEI_DRV_CRLF, pageIdx,
            (pFwImageHeader->imagePage.imagePageX[pageIdx].dataPageSize_32Bit) & (~MEI_BOOT_FLAG)));

         /* Fill Xpage with the DATA memory Data*/
         ret = MEI_VRX_XpageWrite(
                  pFwDlCtrl, pMeiDrvCntrl, pChunk,
                  &(pFwImageHeader->imagePage.imagePageX[pageIdx]), IFX_TRUE);

         if (ret != 0)
            break;
      }
   }

   PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_LOW,
      ("MEI_DRV: BOOT pages loading finished, ret=%d"MEI_DRV_CRLF, ret));

   MEI_DRV_RELEASE_UNIQUE_ACCESS(pMeiDev);

   return ret;
}

static IFX_int32_t MEI_VRX_FinishFwDownload(
                                 MEI_DEV_T *pMeiDev)
{
   IFX_int32_t ret = 0;
   MEI_FW_PORT_MODE_CONTROL_DMA32_T portModeCtrl = {0};

#if (MEI_EMULATION_CONFIGURATION == 1)
   /* Delay ARC releasing from the HALT state. This is true at least for the
      VR9 emulation platform. This workaround has been figured out during
      debug sessions on the VR9 emulation platform.*/
   MEI_DRVOS_Wait_ms(1000);
#endif /* (MEI_EMULATION_CONFIGURATION == 1) */

   /* Change driver state*/
   MEI_DRV_STATE_SET(pMeiDev, e_MEI_DRV_STATE_WAIT_FOR_FIRST_RESP);

   PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_LOW,
      ("MEI_DRV: Driver state changed to e_MEI_DRV_STATE_WAIT_FOR_FIRST_RESP"MEI_DRV_CRLF));

   if ((MEI_DbgFlags & MEI_DBG_FLAGS_ARC_HALT_MASK) ==
                                               MEI_DBG_FLAGS_ARC_HALT_RELEASED)
   {
      /* Release ARC from the HALT state*/
      if ((ret = MEI_VRX_ArcFromHaltRelease(pMeiDev)) != IFX_SUCCESS)
      {
         PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DRV: ARC release from HALT state failed!" MEI_DRV_CRLF));

         return ret;
      }
   }

   PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_LOW,
      ("MEI_DRV: enter MODEM_READY wait state"MEI_DRV_CRLF));

   if (MEI_DRV_STATE_GET(pMeiDev) == e_MEI_DRV_STATE_WAIT_FOR_FIRST_RESP)
   {
      MEI_SET_TIMEOUT_CNT( pMeiDev,
               ((MEI_MaxWaitForModemReady_ms & ~MEI_CFG_DEF_WAIT_PROTECTION_FLAG) /
                 MEI_MIN_MAILBOX_POLL_TIME_MS));

      while(MEI_DRV_STATE_GET(pMeiDev) == e_MEI_DRV_STATE_WAIT_FOR_FIRST_RESP)
      {
         MEI_PollIntPerVrxLine(pMeiDev, e_MEI_DEV_ACCESS_MODE_PASSIV_POLL);

         /* check if modem read  */
         if ( MEI_DRV_STATE_GET(pMeiDev) == e_MEI_DRV_STATE_DFE_READY )
         {
            break;
         }
         else
         {
            if ( MEI_WaitForMailbox(pMeiDev) != IFX_SUCCESS )
            {
               PRN_ERR_INT_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
                    ("MEI_DRV[%02d]: MODEM_READY wait timeout %d ms" MEI_DRV_CRLF,
                      MEI_DRV_LINENUM_GET(pMeiDev), MEI_MaxWaitForModemReady_ms));

               /* Provide fail details*/
               MEI_VRX_InternalDataDumpShow(pMeiDev);

               break;
            }
         #if (MEI_EMULATION_CONFIGURATION == 1)
            if ( (MEI_GET_TIMEOUT_CNT(pMeiDev) %
                 (MEI_MODEM_READY_STATUS_PRINT_MS / MEI_MIN_MAILBOX_POLL_TIME_MS)) == 0)
            {
               printk(KERN_INFO
                  "MEI_DRV[%02d]: Still waiting for MODEM_READY (%d/%d sec)"
                  MEI_DRV_CRLF,
                  MEI_DRV_LINENUM_GET(pMeiDev),
                  ((((MEI_MaxWaitForModemReady_ms & ~MEI_CFG_DEF_WAIT_PROTECTION_FLAG) /
                   MEI_MIN_MAILBOX_POLL_TIME_MS) - MEI_GET_TIMEOUT_CNT(pMeiDev)) *
                   MEI_MIN_MAILBOX_POLL_TIME_MS / 1000),
                  (MEI_MaxWaitForModemReady_ms & ~MEI_CFG_DEF_WAIT_PROTECTION_FLAG) / 1000
                  );
            }
         #endif /* (MEI_EMULATION_CONFIGURATION == 1) */
         }
      }
   }

#if (MEI_SUPPORT_OPTIMIZED_FW_DL == 1)
   /* Calculate CRC after MODEM_READY get */
   MEI_VRX_ChunksCRC_Save(pMeiDev);
#endif /* (MEI_SUPPORT_OPTIMIZED_FW_DL == 1) */

   if (MEI_DEVICE_CFG_IS_PLATFORM(e_MEI_DEV_PLATFORM_CONFIG_VR10) ||
       MEI_DEVICE_CFG_IS_PLATFORM(e_MEI_DEV_PLATFORM_CONFIG_VR10_320))
   {
      /* Clear FORCE_LINK_DOWN flag for PPE */
      *MEI_PPE_U32REG(PPE_FORCE_LINK_DOWN) &= ~0x1;
   }

#if MEI_DBG_CECK_BOOTLOADER_START == 1
   if (MEI_VRX_PortModeControlStructureCurrentGet(
          pMeiDev, &portModeCtrl) != IFX_SUCCESS)
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
         ("MEI_DRV: Current Port Mode Control Structure get failed!"
         MEI_DRV_CRLF));

      return (-e_MEI_ERR_OP_FAILED);
   }

   PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL,
         (MEI_DRV_CRLF "signature1: 0x%04X"
          MEI_DRV_CRLF, portModeCtrl.signature1));
#endif

   /* check the current state */
   if ( MEI_DRV_STATE_GET(pMeiDev) != e_MEI_DRV_STATE_DFE_READY )
   {
      /* reset the driver state to "Init Done" */
      MEI_DRV_GET_UNIQUE_MAILBOX_ACCESS(pMeiDev);

      if ( MEI_DRV_STATE_GET(pMeiDev) != e_MEI_DRV_STATE_DFE_READY )
      {
         MEI_DRV_STATE_SET(pMeiDev, e_MEI_DRV_STATE_SW_INIT_DONE);
      }

      MEI_DRV_RELEASE_UNIQUE_MAILBOX_ACCESS(pMeiDev);


      /* Get Current Port Mode Control structure to check error codes*/
      if (MEI_VRX_PortModeControlStructureCurrentGet(
             pMeiDev, &portModeCtrl) != IFX_SUCCESS)
      {
         PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DRV: Current Port Mode Control Structure get failed!"
            MEI_DRV_CRLF));

         return (-e_MEI_ERR_OP_FAILED);
      }

      if (portModeCtrl.bootError)
      {
         PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DRV: BootLoader62 finished with error code 0x%02X"
            MEI_DRV_CRLF, portModeCtrl.bootError));

         return (portModeCtrl.bootError == MEI_FW_BOOTLOADER_ERR_INVAL_IMAGE) ?
            (-e_MEI_ERR_INVAL_FW_IMAGE) : (-e_MEI_ERR_OP_FAILED);
      }
      return (-e_MEI_ERR_OP_FAILED);
   }
   if (MEI_DFE_INSTANCE_PER_ENTITY != MEI_MAX_SUPPORTED_MEI_IF_PER_DEVICE) 
   /* Populate data to the SLAVE line*/
   {
      MEIX_CNTRL_T *pXCntrl = NULL;
      MEI_DEV_T    *pMeiDevSlave = NULL;
      int entity;

      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_LOW,
         ("MEI_DRV: populate FW download data to the slave line"MEI_DRV_CRLF));

      entity = MEI_GET_ENTITY_FROM_DEVNUM(MEI_DRV_LINENUM_GET(pMeiDev));

      if ( (pXCntrl = MEIX_Cntrl[entity]) == NULL)
      {
         PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
               ("MEI_DRV[%02d]: ERROR Line Struct Allocate - "
                "missing MEIX[%d] entity struct" MEI_DRV_CRLF,
                MEI_DRV_LINENUM_GET(pMeiDev), entity));

         return -e_MEI_ERR_OP_FAILED;
      }

      if ( (pMeiDevSlave = pXCntrl->MeiDevice[1]) == NULL)
      {
         PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
               ("MEI_DRV[%02d]: SLAVE line not exists yet" MEI_DRV_CRLF,
               MEI_DRV_LINENUM_GET(pMeiDev)));

         return -e_MEI_ERR_OP_FAILED;
      }

      /* Copy Mbox descriptor from the MASTER line*/
      memcpy(&(pMeiDevSlave->modemData.mBoxDescr),
             &(pMeiDev->modemData.mBoxDescr),
             sizeof((pMeiDev->modemData.mBoxDescr)));

      /* Release SLAVE line MBox*/
      MEI_ReleaseMailboxMsg(&(pMeiDevSlave->meiDrvCntrl));

      /* Set SLAVE line state*/
      MEI_DRV_STATE_SET(pMeiDevSlave, MEI_DRV_STATE_GET(pMeiDev));
   }

   return ret;
}

/**
   Start the FW download

\param
   pMeiDynCntrl: Points to the dynamic control struct.
\param
   pArgFwDl:     Points to the FW downlosd information data
\param
   bInternCall:  Indicates if the call is form the internal interface
                (image and data already in kernel space)

\return
   IFX_SUCCESS: if the FW was successful.
   negative value if something went wrong.

*/
static IFX_int32_t MEI_VRX_StartFwDownload(
                                 MEI_DYN_CNTRL_T        *pMeiDynCntrl,
                                 IOCTL_MEI_fwDownLoad_t *pArgFwDl,
                                 IFX_boolean_t            bInternCall)
{
   IFX_int32_t ret = 0;
   MEI_FW_DOWNLOAD_CNTRL_T *pFwDlCtrl = &(pMeiDynCntrl->pMeiDev->fwDl);
   MEI_DEV_T *pMeiDev = pMeiDynCntrl->pMeiDev;
   IFX_uint32_t bootloader_size = 0;

   /* Low Level MEI init*/
   if ((ret = MEI_LowLevelInit(&(pMeiDev->meiDrvCntrl))) != IFX_SUCCESS)
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
         ("MEI_DRV: Access port select failed!" MEI_DRV_CRLF));

      return ret;
   }

#if (MEI_SUPPORT_OPTIMIZED_FW_DL == 1)
   if (pArgFwDl->pFwImage == IFX_NULL)
   {
      /* Try to reuse chunks */
      ret = MEI_VRX_ChunksCRC_Check(pMeiDev);
   }
   else
#endif /* (MEI_SUPPORT_OPTIMIZED_FW_DL == 1)*/
   {
      if (pFwDlCtrl->eFwRevision == eMEI_FW_INTERFACE_REV_0)
      {
         /* Setup and Fill Fw download chunks for old revision */
         ret = MEI_VRX_FMLT0_ChunksFill(pFwDlCtrl, pArgFwDl->pFwImage, bInternCall);
      }
      else
      {
         /* Setup and Fill Fw download chunks for new revision (extended) */
         ret = MEI_VRX_FMLT2_ChunksFill(pFwDlCtrl, pArgFwDl->pFwImage, bInternCall);
      }

#if (MEI_SUPPORT_OPTIMIZED_FW_DL == 1)
      if (ret == IFX_SUCCESS)
      {
         /* Chunks successfully filled and could be reused*/
         pFwDlCtrl->bChunksFilled = IFX_TRUE;
      }
#endif /* (MEI_SUPPORT_OPTIMIZED_FW_DL == 1)*/
   }

   if (ret != IFX_SUCCESS)
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
         ("MEI_DRV: FW chunks setup/fill failed!" MEI_DRV_CRLF));

      return ret;
   }

   /* Display Chunks info*/
   MEI_VRX_ChunksInfoShow(pMeiDev);

   /* Read bootloader size, but use only for VR10/VR11 */
   ret = MEI_VRX_BootLoaderSizeGet(pMeiDev, &bootloader_size);

   if (! MEI_DEVICE_CFG_IS_PLATFORM(e_MEI_DEV_PLATFORM_CONFIG_VR9))
   {
      if (ret != IFX_SUCCESS )
      {
         /* set default size */
         bootloader_size = MEI_FW_DEFAULT_BOOTLOADER_SIZE_BYTE;

         PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_WRN,
            ("MEI_DRV: read bootloader size fails, use default size %d bytes"
            MEI_DRV_CRLF, bootloader_size));
      }
   }

   /* Shared memory section in PDBRAM is only used for VR10/VRX300 based
      platforms, but PDBRAM itself is used for all platform except VR9/VRX200 */

   if (MEI_DEVICE_CFG_IS_PLATFORM(e_MEI_DEV_PLATFORM_CONFIG_VR10) ||
       MEI_DEVICE_CFG_IS_PLATFORM(e_MEI_DEV_PLATFORM_CONFIG_VR10_320))
   {
      /* Get access to PDBRAM shared with PPE driver */
      if ((ret = MEI_VR10_PDBRAM_AccessGet(pMeiDev)) != IFX_SUCCESS)
      {
         PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DRV: PDBRAM access failed!" MEI_DRV_CRLF));

         return ret;
      }
   }

   if (! MEI_DEVICE_CFG_IS_PLATFORM(e_MEI_DEV_PLATFORM_CONFIG_VR9))
   {
      /* Fill PDBRAM */
      if ((ret = MEI_VR1x_PDBRAM_Fill(pMeiDev, bootloader_size)) != IFX_SUCCESS)
      {
         PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DRV: PDBRAM fill failed!" MEI_DRV_CRLF));

         return ret;
      }
   }

   /* Update BAR registers*/
   if (pFwDlCtrl->eFwRevision == eMEI_FW_INTERFACE_REV_0)
   {
      ret = MEI_VRX_FMLT0_BarRegistersUpdate(pMeiDev);
   }
   else
   {
      ret = MEI_VRX_FMLT2_BarRegistersUpdate(pMeiDev);
   }

   if (ret != IFX_SUCCESS)
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
         ("MEI_DRV: BAR registers update failed!" MEI_DRV_CRLF));

      return ret;
   }

   /* Download Boot Pages*/
   if ((ret = MEI_VRX_BootPagesDownload(pMeiDev)) != IFX_SUCCESS)
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
         ("MEI_DRV: FW Boot Pages download failed!" MEI_DRV_CRLF));

      return ret;
   }

   return ret;
}

/**
   Start Optimized FW download

\param
   pMeiDynCntrl: Points to the dynamic control struct.
\param
   pArgFwDl:     Points to the FW download information data
\param
   bInternCall:  Indicates if the call is form the internal interface
                (image and data already in kernel space)

\return
   IFX_SUCCESS: if the FW was successful.
   negative value if something went wrong.

*/
static IFX_int32_t MEI_VRX_StartOptFwDownload(
                                 MEI_DYN_CNTRL_T           *pMeiDynCntrl,
                                 IOCTL_MEI_fwOptDownLoad_t *pArgFwDl,
                                 IFX_boolean_t             bInternCall)
{
   IFX_int32_t ret = 0;
   IFX_uint32_t chunkIdx =
      pArgFwDl->chunk_num == MEI_FW_IMAGE_DATA_CHUNK_INDEX ?
      pArgFwDl->chunk_num + 1 : pArgFwDl->chunk_num;
   IFX_uint32_t idx_32bit;
   MEI_FW_DOWNLOAD_CNTRL_T *pFwDlCtrl = &(pMeiDynCntrl->pMeiDev->fwDl);
   MEI_DEV_T *pMeiDev = pMeiDynCntrl->pMeiDev;

   if (chunkIdx == 0)
   {
      /* Low Level MEI init*/
      if ((ret = MEI_LowLevelInit(&(pMeiDev->meiDrvCntrl))) != IFX_SUCCESS)
      {
         PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DRV: Access port select failed!" MEI_DRV_CRLF));

         return ret;
      }
   }

   /* Allocate chunk*/
   ret = MEI_VRX_ImageChunkAlloc(pFwDlCtrl, chunkIdx, pArgFwDl->size_byte);
   if (ret != 0)
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
         ("MEI_DRV: chunk[%d] size %d allocation failed!"
         MEI_DRV_CRLF, chunkIdx, pArgFwDl->size_byte));

      return ret;
   }

   /* Fill chunk */
   if (bInternCall)
   {
      memcpy(
         pFwDlCtrl->imageChunkCtrl[chunkIdx].pImageChunk_aligned,
         pArgFwDl->pFwImageChunk, pArgFwDl->size_byte);
   }
   else
   {
      /* copy data to kernel space */
      if ( MEI_DRVOS_CpyFromUser(
               pFwDlCtrl->imageChunkCtrl[chunkIdx].pImageChunk_aligned,
               pArgFwDl->pFwImageChunk, pArgFwDl->size_byte) == IFX_NULL)
      {
         PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
               ("MEI_DRV: copy_from_user(chunk[%d], size %d) failed"
               MEI_DRV_CRLF, chunkIdx, pArgFwDl->size_byte));

         ret = -e_MEI_ERR_GET_ARG;
         return ret;
      }
   }

   /* Swap chunk to target order*/
   for (idx_32bit=0; idx_32bit<pArgFwDl->size_byte/sizeof(IFX_uint32_t); idx_32bit++)
   {
      ((IFX_uint32_t*)(pFwDlCtrl->imageChunkCtrl[chunkIdx].pImageChunk_aligned))[idx_32bit] =
         SWAP32_BYTE_ORDER(((IFX_uint32_t*)(pFwDlCtrl->imageChunkCtrl[chunkIdx].pImageChunk_aligned))[idx_32bit]);
   }

   if (pArgFwDl->bLastChunk)
   {
      if (pFwDlCtrl->dataRegionSize_Byte)
      {
         /* Allocate chunk for external writable DATA region */
         ret = MEI_VRX_ImageChunkAlloc(
                  pFwDlCtrl, MEI_FW_IMAGE_DATA_CHUNK_INDEX,
                  pFwDlCtrl->dataRegionSize_Byte);

         if (ret != 0)
         {
            PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
               ("MEI_DRV: external DATA region chunk[%d] size %d allocation failed!"
               MEI_DRV_CRLF, chunkIdx, pFwDlCtrl->dataRegionSize_Byte));
            return ret;
         }
      }

      /* Display Chunks info*/
      MEI_VRX_ChunksInfoShow(pMeiDev);

      /* Update BAR registers*/
      if ((ret = MEI_VRX_FMLT0_BarRegistersUpdate(pMeiDev)) != IFX_SUCCESS)
      {
         PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DRV: BAR registers update failed!" MEI_DRV_CRLF));

         return ret;
      }

      /* Download Boot Pages*/
      if ((ret = MEI_VRX_BootPagesDownload(pMeiDev)) != IFX_SUCCESS)
      {
         PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DRV: FW Boot Pages download failed!" MEI_DRV_CRLF));

         return ret;
      }
   }

   return ret;
}

IFX_void_t MEI_DEV_FirmwareDownloadResourcesRelease(
                                 MEI_DEV_T       *pMeiDev)
{
   IFX_uint32_t chunkIdx;

   MEI_DRVOS_SemaphoreLock(&pFwDlCntrlLock);

   /* Free FW chunks*/
   for (chunkIdx=0; chunkIdx < MEI_FW_IMAGE_MAX_CHUNK_COUNT - 1; chunkIdx++)
   {
      MEI_VRX_ImageChunkFree(&(pMeiDev->fwDl), chunkIdx);
   }

   MEI_DRVOS_SemaphoreUnlock(&pFwDlCntrlLock);

   /* mutex exist */
   MEI_DRVOS_SemaphoreDelete(&pFwDlCntrlLock);

#if (MEI_SUPPORT_PCI_SLAVE_FW_DOWNLOAD == 1)
   /* Delete PCI slave pool*/
   if (pMeiDev->fwDl.pPool)
   {
      MEI_VR9_PciSlavePoolDelete(pMeiDev->fwDl.pPool);
   }
#endif /* #if (MEI_SUPPORT_PCI_SLAVE_FW_DOWNLOAD == 1)*/
}

static IFX_int32_t MEI_DEV_FirmwareImageHeaderGet(
                                 MEI_DEV_T     *pMeiDev,
                                 IFX_uint8_t   *pFwImage,
                                 IFX_boolean_t bInternCall)
{
   IFX_int32_t ret = 0;
   MEI_FW_DOWNLOAD_CNTRL_T *pFwDlCtrl = &(pMeiDev->fwDl);
   MEI_FW_IMAGE_CNTRL_T fwImageCtrl;
   MEI_FW_IMAGE_PAGE2_T fwImagePage2;
   MEI_FW_IMAGE_PAGE3_T fwImagePage3;
   MEI_FW_IMAGE_PARTITIONS_PAGE_T fwImagePartitionsPage;
   MEI_FW_PORT_MODE_CONTROL_DMA32_T fwPortModeCtrlCurrent = {0};
   IFX_uint32_t fwMaxImageSizeOfType[3] = {MEI_FW_IMAGE_MAX_SIZE_TYPE_0_BYTE,
                                           MEI_FW_IMAGE_MAX_SIZE_TYPE_1_BYTE,
                                           MEI_FW_IMAGE_MAX_SIZE_TYPE_2_BYTE};

   IFX_uint32_t fwMaxImageChunkOfType[3] = {MEI_FW_IMAGE_MAX_CHUNK_COUNT_TYPE_0,
                                            MEI_FW_IMAGE_MAX_CHUNK_COUNT_TYPE_1,
                                            MEI_FW_IMAGE_MAX_CHUNK_COUNT_TYPE_2};

   memset(&fwImageCtrl, 0x00, sizeof(MEI_FW_IMAGE_CNTRL_T));
   memset(&fwImagePage2, 0x00, sizeof(MEI_FW_IMAGE_PAGE2_T));
   memset(&fwImagePage3, 0x00, sizeof(MEI_FW_IMAGE_PAGE3_T));
   memset(&fwImagePartitionsPage, 0x00, sizeof(MEI_FW_IMAGE_PARTITIONS_PAGE_T));

   /* Get Firmware Image header (including Page#0 info)*/
   if (bInternCall)
   {
      memcpy(&fwImageCtrl, pFwImage, sizeof(fwImageCtrl));
   }
   else
   {
      /* copy data to kernel space */
      if ( MEI_DRVOS_CpyFromUser( &fwImageCtrl, pFwImage,
                                   sizeof(fwImageCtrl)) == IFX_NULL)
      {
         PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
               ("MEI_DRV[0x%02X]: download - copy_from_user(image ctrl data) failed!" MEI_DRV_CRLF,
                MEI_DRV_LINENUM_GET(pMeiDev)));

         ret = -e_MEI_ERR_GET_ARG;
         return ret;
      }
   }

   /* Get total number of swap pages */
   pMeiDev->fwDl.imageNumOfPages =
      SWAP32_BYTE_ORDER(fwImageCtrl.imageNumOfPages);

   if (pMeiDev->fwDl.imageNumOfPages == 0)
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DRV[0x%02X]: download - no FW swap pages detected!" MEI_DRV_CRLF,
             MEI_DRV_LINENUM_GET(pMeiDev)));

      ret = -e_MEI_ERR_GET_ARG;
      return ret;
   }

   /* Get FW size */
   pMeiDev->fwDl.size_byte =
      SWAP32_BYTE_ORDER(fwImageCtrl.imageSize_Bytes);

   if (pMeiDev->fwDl.size_byte == 0)
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DRV[0x%02X]: download - FW header binary size is zero!"
            MEI_DRV_CRLF, MEI_DRV_LINENUM_GET(pMeiDev)));

      ret = -e_MEI_ERR_GET_ARG;
      return ret;
   }

   pMeiDev->fwDl.size_byte =
      SWAP32_BYTE_ORDER(fwImageCtrl.imageSize_Bytes) +
      sizeof(fwImageCtrl.imageSize_Bytes) + sizeof(fwImageCtrl.imageCheckSum);

   /* Get revision number */
   /* Get Firmware Image Page#2 info*/
   if (bInternCall)
   {
      memcpy(&fwImagePage2,
         pFwImage + MEI_FW_IMAGE_PAGE2_OFFSET_32BIT * sizeof(IFX_uint32_t),
         sizeof(fwImagePage2));
   }
   else
   {
      /* copy data to kernel space */
      if ( MEI_DRVOS_CpyFromUser(
              &fwImagePage2,
              pFwImage + MEI_FW_IMAGE_PAGE2_OFFSET_32BIT * sizeof(IFX_uint32_t),
              sizeof(fwImagePage2)) == IFX_NULL)
      {
         PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
               ("MEI_DRV[0x%02X]: download - copy_from_user(page#2 ctrl data) failed!" MEI_DRV_CRLF,
                MEI_DRV_LINENUM_GET(pMeiDev)));

         ret = -e_MEI_ERR_GET_ARG;
         return ret;
      }
   }

   pFwDlCtrl->eFwRevision = SWAP32_BYTE_ORDER(fwImagePage2.fwRevision);
   if ((pFwDlCtrl->eFwRevision != eMEI_FW_INTERFACE_REV_0) &&
       (pFwDlCtrl->eFwRevision != eMEI_FW_INTERFACE_REV_1))
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
         ("MEI_DRV[0x%02X]: unsupported firmware revision %d!"
         MEI_DRV_CRLF, MEI_DRV_LINENUM_GET(pMeiDev), pFwDlCtrl->eFwRevision));

      ret = -e_MEI_ERR_INVAL_CONFIG;
      return ret;
   }

   pFwDlCtrl->eFwMemLayoutType = SWAP32_BYTE_ORDER(fwImagePage2.fwMemLayout_Type);

   PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL,
         (MEI_DRV_CRLF"Fw header info: fw revision %d, layout type %d" MEI_DRV_CRLF,
          pFwDlCtrl->eFwRevision, pFwDlCtrl->eFwMemLayoutType));

   /* For old revision (0) layout type at page 2 is not used (zero by default) */
   if ((pFwDlCtrl->eFwRevision == eMEI_FW_INTERFACE_REV_0) &&
                    (pFwDlCtrl->eFwMemLayoutType != eMEI_FW_MEM_LAYOUT_TYPE_0))
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR, ("MEI_DRV[0x%02X]: "
         "image header revision %d is not compatible with layout type %d!"
         MEI_DRV_CRLF, MEI_DRV_LINENUM_GET(pMeiDev), pFwDlCtrl->eFwRevision,
         pFwDlCtrl->eFwMemLayoutType));

      ret = -e_MEI_ERR_INVAL_CONFIG;
      return ret;
   }

   if (pFwDlCtrl->eFwRevision == eMEI_FW_INTERFACE_REV_1)
   {
      /* Get Firmware Image PartitionsPage info*/
      if (bInternCall)
      {
         memcpy(&fwImagePartitionsPage,
            pFwImage + MEI_FW_IMAGE_PARTITIONS_PAGE_OFFSET_32BIT * sizeof(IFX_uint32_t),
            sizeof(fwImagePartitionsPage));
      }
      else
      {
         /* copy data to kernel space */
         if ( MEI_DRVOS_CpyFromUser(
                 &fwImagePartitionsPage,
                 pFwImage + MEI_FW_IMAGE_PARTITIONS_PAGE_OFFSET_32BIT * sizeof(IFX_uint32_t),
               sizeof(fwImagePartitionsPage)) == IFX_NULL)
         {
            PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
                  ("MEI_DRV[0x%02X]: download - copy_from_user(partitions pages ctrl data) failed!" MEI_DRV_CRLF,
                   MEI_DRV_LINENUM_GET(pMeiDev)));

            ret = -e_MEI_ERR_GET_ARG;
            return ret;
         }
      }
      pFwDlCtrl->meiPartitions.bootloader_size   = SWAP32_BYTE_ORDER(fwImagePage2.bootloader_size);
      pFwDlCtrl->meiPartitions.debug_data_size   = SWAP32_BYTE_ORDER(fwImagePage2.debug_data_size);
      pFwDlCtrl->meiPartitions.vDSL_image_offset = SWAP32_BYTE_ORDER(fwImagePartitionsPage.vDSL_image_offset);
      pFwDlCtrl->meiPartitions.vDSL_image_size   = SWAP32_BYTE_ORDER(fwImagePartitionsPage.vDSL_image_size);
      pFwDlCtrl->meiPartitions.aDSL_image_offset = SWAP32_BYTE_ORDER(fwImagePartitionsPage.aDSL_image_offset);
      pFwDlCtrl->meiPartitions.aDSL_image_size   = SWAP32_BYTE_ORDER(fwImagePartitionsPage.aDSL_image_size);
      pFwDlCtrl->meiPartitions.vDSL_cache_offset = SWAP32_BYTE_ORDER(fwImagePartitionsPage.vDSL_cache_offset);
      pFwDlCtrl->meiPartitions.vDSL_cache_size   = SWAP32_BYTE_ORDER(fwImagePartitionsPage.vDSL_cache_size);
      pFwDlCtrl->meiPartitions.aDSL_cache_offset = SWAP32_BYTE_ORDER(fwImagePartitionsPage.aDSL_cache_offset);
      pFwDlCtrl->meiPartitions.aDSL_cache_size   = SWAP32_BYTE_ORDER(fwImagePartitionsPage.aDSL_cache_size);

   PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL,
         (MEI_DRV_CRLF "================= FW PARTITIONS =================" MEI_DRV_CRLF));

   PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL,
         ("bootloader size   [byte] %d(0x%X)" MEI_DRV_CRLF, pFwDlCtrl->meiPartitions.bootloader_size,
                                                            pFwDlCtrl->meiPartitions.bootloader_size));
   PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL,
         ("debug data size   [byte] %d(0x%X)" MEI_DRV_CRLF, pFwDlCtrl->meiPartitions.debug_data_size,
                                                            pFwDlCtrl->meiPartitions.debug_data_size));
   PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL,
         ("vdsl image offset [byte] %d(0x%X)" MEI_DRV_CRLF, pFwDlCtrl->meiPartitions.vDSL_image_offset,
                                                            pFwDlCtrl->meiPartitions.vDSL_image_offset));
   PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL,
         ("vdsl image size   [byte] %d(0x%X)" MEI_DRV_CRLF, pFwDlCtrl->meiPartitions.vDSL_image_size,
                                                            pFwDlCtrl->meiPartitions.vDSL_image_size));
   PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL,
         ("adsl image offset [byte] %d(0x%X)" MEI_DRV_CRLF, pFwDlCtrl->meiPartitions.aDSL_image_offset,
                                                            pFwDlCtrl->meiPartitions.aDSL_image_offset));
   PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL,
         ("adsl image size   [byte] %d(0x%X)" MEI_DRV_CRLF, pFwDlCtrl->meiPartitions.aDSL_image_size,
                                                            pFwDlCtrl->meiPartitions.aDSL_image_size));
   PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL,
         ("vdsl cache offset [byte] %d(0x%X)" MEI_DRV_CRLF, pFwDlCtrl->meiPartitions.vDSL_cache_offset,
                                                            pFwDlCtrl->meiPartitions.vDSL_cache_offset));
   PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL,
         ("vdsl cache size   [byte] %d(0x%X)" MEI_DRV_CRLF, pFwDlCtrl->meiPartitions.vDSL_cache_size,
                                                            pFwDlCtrl->meiPartitions.vDSL_cache_size));
   PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL,
         ("adsl cache offset [byte] %d(0x%X)" MEI_DRV_CRLF, pFwDlCtrl->meiPartitions.aDSL_cache_offset,
                                                            pFwDlCtrl->meiPartitions.aDSL_cache_offset));
   PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL,
         ("adsl cache size   [byte] %d(0x%X)" MEI_DRV_CRLF, pFwDlCtrl->meiPartitions.aDSL_cache_size,
                                                            pFwDlCtrl->meiPartitions.aDSL_cache_size));

   PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL,
         ("=================================================" MEI_DRV_CRLF));

      if (! MEI_DEVICE_CFG_IS_PLATFORM(e_MEI_DEV_PLATFORM_CONFIG_VR9))
      {
         if ((pFwDlCtrl->meiPartitions.bootloader_size + pFwDlCtrl->meiPartitions.vDSL_cache_size
                                 > MEI_PDBRAM_CACHE_SIZE_BYTE) ||
             (pFwDlCtrl->meiPartitions.bootloader_size + pFwDlCtrl->meiPartitions.aDSL_cache_size
                                 > MEI_PDBRAM_CACHE_SIZE_BYTE))
         {
            PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
               ("MEI_DRV[0x%02X]: bootloader and cache are too big for PDBRAM!"
               MEI_DRV_CRLF, MEI_DRV_LINENUM_GET(pMeiDev)));
#if (MEI_SUPPORT_DEVICE_VR11 == 1)
            pFwDlCtrl->meiPartitions.vDSL_cache_size =
               MEI_PDBRAM_CACHE_SIZE_BYTE - pFwDlCtrl->meiPartitions.bootloader_size;
            PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
               ("MEI_DRV[0x%02X]: Corrected vdsl cache size - set it to max. possible value:"
               MEI_DRV_CRLF, MEI_DRV_LINENUM_GET(pMeiDev)));
            PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL,
                  ("vdsl cache size   [byte] %d(0x%X)" MEI_DRV_CRLF, pFwDlCtrl->meiPartitions.vDSL_cache_size,
                                                                     pFwDlCtrl->meiPartitions.vDSL_cache_size));
#endif
         }
      }

      if (pFwDlCtrl->meiPartitions.debug_data_size > MEI_FW_IMAGE_MAX_DEBUG_DATA)
      {
         PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_WRN,
            ("MEI_DRV[0x%02X]: reduce debug data size %u to max size %u"
              MEI_DRV_CRLF, MEI_DRV_LINENUM_GET(pMeiDev),
              pFwDlCtrl->meiPartitions.debug_data_size,
              MEI_FW_IMAGE_MAX_DEBUG_DATA));
      }

   }

   switch (pFwDlCtrl->eFwMemLayoutType)
   {
      case eMEI_FW_MEM_LAYOUT_TYPE_0:

      case eMEI_FW_MEM_LAYOUT_TYPE_1:

      case eMEI_FW_MEM_LAYOUT_TYPE_2:

         if (pMeiDev->fwDl.size_byte > fwMaxImageSizeOfType[pFwDlCtrl->eFwMemLayoutType])
         {
            PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
               ("MEI_DRV: ERROR - Firmware Image size %d is too large!" MEI_DRV_CRLF,
               pMeiDev->fwDl.size_byte));

            PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
               ("MEI_DRV: ERROR - Max supported Firmware Image size for mem layout type %d is %d!" MEI_DRV_CRLF,
               pFwDlCtrl->eFwMemLayoutType, fwMaxImageSizeOfType[pFwDlCtrl->eFwMemLayoutType]));

            return IFX_ERROR;
         }

         /* set max amount of chunks to contain image */
         pFwDlCtrl->meiMaxChunkCount = fwMaxImageChunkOfType[pFwDlCtrl->eFwMemLayoutType];

         break;

      default:
         PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DRV[0x%02X]: download - FW memory layout type %d is not supported"
            MEI_DRV_CRLF, MEI_DRV_LINENUM_GET(pMeiDev), pFwDlCtrl->eFwMemLayoutType));

         ret = -e_MEI_ERR_INVAL_ARG;
         return ret;
   }

   switch(pFwDlCtrl->eFwMemLayoutType)
   {
      case eMEI_FW_MEM_LAYOUT_TYPE_0:
      default:
         pFwDlCtrl->meiSpecialChunkOffset =
            MEI_FW_IMAGE_MAX_CHUNK_COUNT_TYPE_0 - MEI_TOTAL_BAR_REGISTER_COUNT;
         break;

      case eMEI_FW_MEM_LAYOUT_TYPE_1:
         pFwDlCtrl->meiSpecialChunkOffset =
            MEI_FW_IMAGE_MAX_CHUNK_COUNT_TYPE_1 - MEI_TOTAL_BAR_REGISTER_COUNT;
         break;

      case eMEI_FW_MEM_LAYOUT_TYPE_2:
         pFwDlCtrl->meiSpecialChunkOffset =
            MEI_FW_IMAGE_MAX_CHUNK_COUNT_TYPE_2 - MEI_TOTAL_BAR_REGISTER_COUNT;
         break;
   }

   /*
      Get default Port Mode Control structure from the FW header Page#0
   */
   memcpy(&(pMeiDev->fwDl.defaultPortModeCtrl), &(fwImageCtrl.imagePage.imagePage0),
      sizeof(pMeiDev->fwDl.defaultPortModeCtrl));


   pMeiDev->fwDl.defaultPortModeCtrl.bgPortSelRegValue =
      SWAP32_BYTE_ORDER(pMeiDev->fwDl.defaultPortModeCtrl.bgPortSelRegValue);
   pMeiDev->fwDl.defaultPortModeCtrl.imageOffsetSRAM =
      SWAP32_BYTE_ORDER(pMeiDev->fwDl.defaultPortModeCtrl.imageOffsetSRAM);

   pMeiDev->fwDl.defaultPortModeCtrl.signature0 =
      SWAP16_BYTE_ORDER(pMeiDev->fwDl.defaultPortModeCtrl.signature0);

   /* Check Signature#0*/
   if (pMeiDev->fwDl.defaultPortModeCtrl.signature0 != MEI_FW_IMAGE_SIGNATURE0)
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DRV[0x%02X]: download - wrong Signature0 = 0x%04X detected "
            "within the firmware binary!"
            MEI_DRV_CRLF, MEI_DRV_LINENUM_GET(pMeiDev),
            pMeiDev->fwDl.defaultPortModeCtrl.signature0));

      ret = -e_MEI_ERR_GET_ARG;
      return ret;
   }

   /* Get Firmware Image Page#3 info*/
   if (bInternCall)
   {
      memcpy(&fwImagePage3,
         pFwImage + MEI_FW_IMAGE_PAGE3_OFFSET_32BIT * sizeof(IFX_uint32_t),
         sizeof(fwImagePage3));
   }
   else
   {
      /* copy data to kernel space */
      if ( MEI_DRVOS_CpyFromUser(
              &fwImagePage3,
              pFwImage + MEI_FW_IMAGE_PAGE3_OFFSET_32BIT * sizeof(IFX_uint32_t),
              sizeof(fwImagePage3)) == IFX_NULL)
      {
         PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
               ("MEI_DRV[0x%02X]: download - copy_from_user(page#3 ctrl data) failed!" MEI_DRV_CRLF,
                MEI_DRV_LINENUM_GET(pMeiDev)));

         ret = -e_MEI_ERR_GET_ARG;
         return ret;
      }
   }

   /* Get size of the permanent CACHED region [bytes] */
   pMeiDev->fwDl.cachedRegionSize_byte =
      SWAP32_BYTE_ORDER(fwImagePage3.cachedRegionSize_Byte);

   /* Not defined yet in the VR9 MEI specification,
      set to the maximum size*/
   pMeiDev->fwDl.dataRegionSize_Byte = MEI_FW_IMAGE_CHUNK_SIZE_BYTE;

   PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL,
         (MEI_DRV_CRLF "================= FW IMAGE INFO ==================" MEI_DRV_CRLF));
   PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL,
         ("Size = %d [byte], CheckSum = 0x%08X, NumOfPages = %d" MEI_DRV_CRLF,
          pMeiDev->fwDl.size_byte, SWAP32_BYTE_ORDER(fwImageCtrl.imageCheckSum),
          pMeiDev->fwDl.imageNumOfPages));
   PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL,
         ("CACHED region size = %d [byte], DATA region size = %d [byte]" MEI_DRV_CRLF,
          pMeiDev->fwDl.cachedRegionSize_byte, pMeiDev->fwDl.dataRegionSize_Byte));
   PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL,
         (MEI_DRV_CRLF "========== Port Mode Control (default) ===========" MEI_DRV_CRLF));
   PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL,
         ("Dual Port Lock      : %s" MEI_DRV_CRLF,
          pMeiDev->fwDl.defaultPortModeCtrl.dualPortModeLock == 1 ? "LOCKED" : "UNLOCKED"));
   PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL,
         ("xDSL Mode Lock      : %s" MEI_DRV_CRLF,
          pMeiDev->fwDl.defaultPortModeCtrl.xDslModeLock == 1 ? "LOCKED" : "UNLOCKED"));
   PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL,
         ("Preferred Port Mode : %s" MEI_DRV_CRLF,
          pMeiDev->fwDl.defaultPortModeCtrl.dualPortModePreffered == 1 ? "DUAL" : "SINGLE"));
   PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL,
         ("Preferred xDSL Mode : %s" MEI_DRV_CRLF,
          pMeiDev->fwDl.defaultPortModeCtrl.xDslModePreffered == 1 ? "ADSL" : "VDSL"));
   PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL,
         ("Current Port Mode   : %s" MEI_DRV_CRLF,
          pMeiDev->fwDl.defaultPortModeCtrl.dualPortModeCurrent == 0 ? "SINGLE" :
          pMeiDev->fwDl.defaultPortModeCtrl.dualPortModeCurrent == 1 ? "DUAL" : "NONE"));
   PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL,
         ("Current xDSL Mode   : %s" MEI_DRV_CRLF,
          pMeiDev->fwDl.defaultPortModeCtrl.xDslModeCurrent == 0 ? "VDSL" :
          pMeiDev->fwDl.defaultPortModeCtrl.xDslModeCurrent == 1 ? "ADSL" : "NONE"));
   PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL,
         (MEI_DRV_CRLF "==================================================" MEI_DRV_CRLF));

   /* Get current Port Mode Control Structure*/
   if ( MEI_VRX_PortModeControlStructureCurrentGet(
           pMeiDev, &fwPortModeCtrlCurrent) != IFX_SUCCESS)
   {
      PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_WRN,
            (MEI_DRV_CRLF "Current Port Mode Control Structure get failed!"
            MEI_DRV_CRLF));
      return ret;
   }
   else
   {
      if ( (fwPortModeCtrlCurrent.signature0 != MEI_FW_IMAGE_SIGNATURE0) ||
           (fwPortModeCtrlCurrent.signature1 != MEI_FW_IMAGE_SIGNATURE1) )
      {
         PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_WRN,
               (MEI_DRV_CRLF "Current Port Mode Control Structure not "
               "initialized yet, bootloader will take the default structure"
               MEI_DRV_CRLF));
         return ret;
      }
   }

   PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL,
         (MEI_DRV_CRLF "========== Port Mode Control (current) ===========" MEI_DRV_CRLF));
   PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL,
         ("Dual Port Lock      : %s" MEI_DRV_CRLF,
          fwPortModeCtrlCurrent.dualPortModeLock == 1 ? "LOCKED" : "UNLOCKED"));
   PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL,
         ("xDSL Mode Lock      : %s" MEI_DRV_CRLF,
          fwPortModeCtrlCurrent.xDslModeLock == 1 ? "LOCKED" : "UNLOCKED"));
   PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL,
         ("Preferred Port Mode : %s" MEI_DRV_CRLF,
          fwPortModeCtrlCurrent.dualPortModePreffered == 1 ? "DUAL" : "SINGLE"));
   PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL,
         ("Preferred xDSL Mode : %s" MEI_DRV_CRLF,
          fwPortModeCtrlCurrent.xDslModePreffered == 1 ? "ADSL" : "VDSL"));
   PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL,
         ("Current Port Mode   : %s" MEI_DRV_CRLF,
          fwPortModeCtrlCurrent.dualPortModeCurrent == 0 ? "SINGLE" :
          fwPortModeCtrlCurrent.dualPortModeCurrent == 1 ? "DUAL" : "NONE"));
   PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL,
         ("Current xDSL Mode   : %s" MEI_DRV_CRLF,
          fwPortModeCtrlCurrent.xDslModeCurrent == 0 ? "VDSL" :
          fwPortModeCtrlCurrent.xDslModeCurrent == 1 ? "ADSL" : "NONE"));
   PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL,
         (MEI_DRV_CRLF "==================================================" MEI_DRV_CRLF));

   return ret;
}


/* ============================================================================
   Function definition (FW Download ioctl)
   ========================================================================= */

/**
   Do the firmware download for the current VR9/VR10 device.

\param
   pMeiDynCntrl - private dynamic comtrol data (per open instance)
\param
   pFwDl          - points to the FW download information data
\param
   bInternCall    - indicates if the call is form the internal interface
                    (image and data already in kernel space)
\return
   IFX_SUCCESS: if the FW was successful.
   negative value if something went wrong.

*/
IFX_int32_t MEI_DEV_IoctlFirmwareDownload(
                                 MEI_DYN_CNTRL_T        *pMeiDynCntrl,
                                 IOCTL_MEI_fwDownLoad_t *pArgFwDl,
                                 IFX_boolean_t            bInternCall)
{
   IFX_int32_t ret = 0;
   MEI_DEV_T *pMeiDev = pMeiDynCntrl->pMeiDev;
#if (MEI_EXPORT_INTERNAL_API == 1) && (MEI_DRV_ATM_PTM_INTERFACE_ENABLE == 1)
   MEI_TC_Reset_t tc_reset = {0};
#endif
#if (MEI_SUPPORT_OPTIMIZED_FW_DL == 1)
   IFX_boolean_t bChunksReuse = IFX_FALSE;
#endif

   PRN_DBG_USR( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL, MEI_DRV_LINENUM_GET(pMeiDev),
         ("MEI_DRV[%02d]: start FW download -  entity %d" MEI_DRV_CRLF,
           MEI_DRV_LINENUM_GET(pMeiDev), MEI_GET_ENTITY_FROM_DEVNUM(MEI_DRV_LINENUM_GET(pMeiDev))));
   if (MEI_DFE_INSTANCE_PER_ENTITY != MEI_MAX_SUPPORTED_MEI_IF_PER_DEVICE)
   {
       if (MEI_DRV_LINENUM_GET(pMeiDev) % MEI_DFE_INSTANCE_PER_ENTITY)
       {
           if (MEI_DRV_STATE_GET(pMeiDev) != e_MEI_DRV_STATE_DFE_READY)
           {
               PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
                               ("MEI_DRV[%02d]: SLAVE line is not ready, download MASTER line FW first!"
                                MEI_DRV_CRLF, MEI_DRV_LINENUM_GET(pMeiDev) ));
               return (-e_MEI_ERR_OP_FAILED);
           }
           else
           {
               return 0;
           }
       }
   }

#if (MEI_SUPPORT_OPTIMIZED_FW_DL == 1)
   /* Chunks reuse mode */
   if ((pArgFwDl->pFwImage == IFX_NULL) && (pArgFwDl->size_byte == 0))
   {
      bChunksReuse = IFX_TRUE;
   }
#endif /* (MEI_SUPPORT_OPTIMIZED_FW_DL == 1)*/

   if ((!pArgFwDl->pFwImage)
#if (MEI_SUPPORT_OPTIMIZED_FW_DL == 1)
        && (!bChunksReuse)
#endif
       )
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DRV[%02d]: missing FW image for download." MEI_DRV_CRLF,
              MEI_DRV_LINENUM_GET(pMeiDev) ));

      return (-e_MEI_ERR_INVAL_ARG);
   }

   /* Check for the minimum Image Size. At least one page should be available
      (excluding Page#0 which is a dummy page)*/
   if ((pArgFwDl->size_byte <= MEI_FW_IMAGE_HEADER_SIZE_BYTE)
#if (MEI_SUPPORT_OPTIMIZED_FW_DL == 1)
        && (!bChunksReuse)
#endif
       )
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
         ("MEI_DRV: ERROR - Firmware Image size %d is too small!" MEI_DRV_CRLF,
         pArgFwDl->size_byte));

      return IFX_ERROR;
   }

   /* Check for the maximum Firmware Image size */
   if (pArgFwDl->size_byte > MEI_FW_IMAGE_MAX_SIZE_BYTE)
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
         ("MEI_DRV: ERROR - Firmware Image size %d is too large!" MEI_DRV_CRLF,
         pArgFwDl->size_byte));

      return IFX_ERROR;
   }

   /* Check current driver state. */
   if (MEI_DRV_STATE_GET(pMeiDev) != e_MEI_DRV_STATE_SW_INIT_DONE)
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DRV[%02d]: invalid state (%d) for load FW image." MEI_DRV_CRLF,
              MEI_DRV_LINENUM_GET(pMeiDev), MEI_DRV_STATE_GET(pMeiDev) ));

      return -e_MEI_ERR_INVAL_STATE;
   }

   MEI_DRVOS_SemaphoreLock(&pFwDlCntrlLock);

#if (MEI_SUPPORT_OPTIMIZED_FW_DL == 1)
   if (!bChunksReuse)
#endif
   {
      /* Get FW Image Header info*/
      ret = MEI_DEV_FirmwareImageHeaderGet(pMeiDev, pArgFwDl->pFwImage, bInternCall);

      if (ret != 0)
      {
         PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
               ("MEI_DRV[0x%02X]: FW Image Header Info get failed!" MEI_DRV_CRLF,
                MEI_DRV_LINENUM_GET(pMeiDev)));

         goto ERROR_MEI_IOCTL_FWDL_LOAD_IMAGE;
      }

      /* Check Image size [bytes]*/
      if (pArgFwDl->size_byte != pMeiDev->fwDl.size_byte)
      {
         PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
               ("MEI_DRV[0x%02X]: FW Image size mismatch %u/%u!" MEI_DRV_CRLF,
                MEI_DRV_LINENUM_GET(pMeiDev), pArgFwDl->size_byte, pMeiDev->fwDl.size_byte));

         goto ERROR_MEI_IOCTL_FWDL_LOAD_IMAGE;
      }

      /* Merge Default Port Mode Control Structure*/
      ret = MEI_VRX_PortModeControlStructureDefaultSet(pMeiDev);

      if (ret != 0)
      {
         PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
               ("MEI_DRV[0x%02X]: Port Mode Control Structure Default set failed!" MEI_DRV_CRLF,
                MEI_DRV_LINENUM_GET(pMeiDev)));

         goto ERROR_MEI_IOCTL_FWDL_LOAD_IMAGE;
      }
   }

   /* Start FW download*/
   ret = MEI_VRX_StartFwDownload(pMeiDynCntrl, pArgFwDl, bInternCall);

   PRN_DBG_USR( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL, MEI_DRV_LINENUM_GET(pMeiDev),
         ("MEI_DRV[%02d]: FW download step 1 ready" MEI_DRV_CRLF,
           MEI_DRV_LINENUM_GET(pMeiDev)));

   /* Finish FW download*/
   if (ret == 0)
   {
      ret = MEI_VRX_FinishFwDownload(pMeiDev);
   }

#if (MEI_EXPORT_INTERNAL_API == 1) && (MEI_DRV_ATM_PTM_INTERFACE_ENABLE == 1)
   /* reset TC layer */
   if (ret == 0)
   {
      if (MEI_InternalTcReset(pMeiDynCntrl, &tc_reset) != 0)
      {
            PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_WRN,
                  ("MEI_DRV[%02d]: Could not perform reset of TC-Layer!"
                   MEI_DRV_CRLF, MEI_DRV_LINENUM_GET(pMeiDev)));
      }
   }
#endif

   /* Configure downloaded firmware for DSM */
#if (MEI_SUPPORT_DSM == 1)
   if (ret == 0)
   {
      /* Only for fw support vectoring */
      if (pMeiDev->nFwVectorSupport)
      {
         /* check fw statistics values for correct values */
         MEI_VRX_DSM_FwStatsCheck(pMeiDynCntrl);

         /* pass DSM fw settings*/
         ret = MEI_VRX_DSM_FwConfigSet(pMeiDynCntrl);
      }
   }
#endif /* (MEI_SUPPORT_DSM == 1) */

#if (MEI_SUPPORT_DEBUG_STREAMS == 1)
   if (ret == 0)
   {
      ret = MEI_DBG_STREAM_FwConfigSet(pMeiDynCntrl);
   }
#endif /* (MEI_SUPPORT_DEBUG_STREAMS == 1) */

   if (ret == 0)
   {
      /* pass PLL offset fw settings*/
      ret = MEI_PLL_ConfigSet(pMeiDynCntrl);
   }

   MEI_DRVOS_SemaphoreUnlock(&pFwDlCntrlLock);

   if (ret != 0)
   {
      MEI_IF_STAT_INC_FWDL_ERR_COUNT(pMeiDev);
#if (MEI_SUPPORT_OPTIMIZED_FW_DL == 1)
      if (bChunksReuse)
      {
         MEI_IF_STAT_INC_FWDL_OPT_ERR_COUNT(pMeiDev);
      }
#endif
   }
   else
   {
      /* Download succeeded incl. MODEM_READY, increase FW download counter */
      MEI_IF_STAT_INC_FWDL_COUNT_INC(pMeiDev);
#if (MEI_SUPPORT_OPTIMIZED_FW_DL == 1)
      if (bChunksReuse)
      {
         MEI_IF_STAT_INC_FWDL_OPT_COUNT_INC(pMeiDev);
      }
#endif
   }

   return ret;

ERROR_MEI_IOCTL_FWDL_LOAD_IMAGE:

   MEI_DRVOS_SemaphoreUnlock(&pFwDlCntrlLock);

   return ret;
}

/**
   Do the optimized firmware download (via chunks) for the current VR9/VR10
   device.

\param
   pMeiDynCntrl - private dynamic comtrol data (per open instance)
\param
   pFwDl          - points to the FW download information data
\param
   bInternCall    - indicates if the call is form the internal interface
                    (image and data already in kernel space)
\return
   IFX_SUCCESS: if the FW was successful.
   negative value if something went wrong.

*/
IFX_int32_t MEI_IoctlOptFirmwareDownload(
                                 MEI_DYN_CNTRL_T           *pMeiDynCntrl,
                                 IOCTL_MEI_fwOptDownLoad_t *pArgFwDl,
                                 IFX_boolean_t             bInternCall)
{
   IFX_int32_t ret = 0;
   MEI_DEV_T *pMeiDev = pMeiDynCntrl->pMeiDev;

   PRN_DBG_USR( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL, MEI_DRV_LINENUM_GET(pMeiDev),
         ("MEI_DRV[%02d]: Optimized FW download -  entity %d, chunk %d" MEI_DRV_CRLF,
           MEI_DRV_LINENUM_GET(pMeiDev), MEI_GET_ENTITY_FROM_DEVNUM(MEI_DRV_LINENUM_GET(pMeiDev)),
           pArgFwDl->chunk_num));

   if (!pArgFwDl->pFwImageChunk)
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DRV[%02d]: missing FW image chunk for download." MEI_DRV_CRLF,
              MEI_DRV_LINENUM_GET(pMeiDev) ));

      return (-e_MEI_ERR_INVAL_ARG);
   }

   /* Check for the minimum Image Size. At least one page should be available
      (excluding Page#0 which is a dummy page)*/
   if ((pArgFwDl->size_byte <= MEI_FW_IMAGE_HEADER_SIZE_BYTE) &&
       (pArgFwDl->chunk_num == 0))
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
         ("MEI_DRV: ERROR - Firmware Image chunk size %d is too small!" MEI_DRV_CRLF,
         pArgFwDl->size_byte));

      return IFX_ERROR;
   }

   /* Check for the correct chunk number*/
   if (pArgFwDl->chunk_num > MEI_FW_IMAGE_MAX_CHUNK_COUNT - 1)
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
         ("MEI_DRV: ERROR - Firmware Image chunk number %d exceeds maximum allowed value %d!"
         MEI_DRV_CRLF, pArgFwDl->chunk_num, (MEI_FW_IMAGE_MAX_CHUNK_COUNT - 1)));

      return IFX_ERROR;
   }

   /* Check for the maximum chunk sizes*/
   if (pArgFwDl->chunk_num == (MEI_FW_IMAGE_MAX_CHUNK_COUNT - 1))
   {
      /* Check for the maximum Firmware Image chunk #16 size */
      if (pArgFwDl->size_byte > MEI_BAR16_SIZE_BYTE)
      {
         PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DRV: ERROR - Firmware Image chunk #16 size %d is too large!" MEI_DRV_CRLF,
            pArgFwDl->size_byte));

         return IFX_ERROR;
      }
   }
   else
   {
      /* Check for the maximum Firmware Image chunk size */
      if (pArgFwDl->size_byte > MEI_FW_IMAGE_CHUNK_SIZE_BYTE)
      {
         PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DRV: ERROR - Firmware Image chunk size %d is too large!" MEI_DRV_CRLF,
            pArgFwDl->size_byte));

         return IFX_ERROR;
      }
   }

   /* Check current driver state. */
   if (MEI_DRV_STATE_GET(pMeiDev) != e_MEI_DRV_STATE_SW_INIT_DONE)
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DRV[%02d]: invalid state (%d) for load FW image." MEI_DRV_CRLF,
              MEI_DRV_LINENUM_GET(pMeiDev), MEI_DRV_STATE_GET(pMeiDev) ));

      return -e_MEI_ERR_INVAL_STATE;
   }

   MEI_DRVOS_SemaphoreLock(&pFwDlCntrlLock);

   /* Get Image Info only from the 1st chunk*/
   if (pArgFwDl->chunk_num == 0)
   {
      /* Get FW Image Header info*/
      ret = MEI_DEV_FirmwareImageHeaderGet(
               pMeiDev, pArgFwDl->pFwImageChunk, bInternCall);

      if (ret != 0)
      {
         PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
               ("MEI_DRV[0x%02X]: FW Image Header Info get failed!" MEI_DRV_CRLF,
                MEI_DRV_LINENUM_GET(pMeiDev)));

         goto ERROR_MEI_IOCTL_OPT_FWDL_LOAD_IMAGE;
      }
   }

   /* Start FW download*/
   ret = MEI_VRX_StartOptFwDownload(pMeiDynCntrl, pArgFwDl, bInternCall);

   /* Finish FW download*/
   if ((ret == 0) && pArgFwDl->bLastChunk)
   {
      ret = MEI_VRX_FinishFwDownload(pMeiDev);
   }

   MEI_DRVOS_SemaphoreUnlock(&pFwDlCntrlLock);

   return ret;

ERROR_MEI_IOCTL_OPT_FWDL_LOAD_IMAGE:

   MEI_DRVOS_SemaphoreUnlock(&pFwDlCntrlLock);

   return ret;
}

/**
   Set FW xDSL/DualPort mode control.

\param
   pMeiDynCntrl - private dynamic comtrol data (per open instance)
\param
   pArgFwModeCtrl - points to the xDSL/DualPort mode control data

\return
   IFX_SUCCESS: if the FW Mode Control setting was successful.
   negative value if something went wrong.
*/
IFX_int32_t MEI_IoctlFwModeCtrlSet(
                                 MEI_DYN_CNTRL_T           *pMeiDynCntrl,
                                 IOCTL_MEI_FwModeCtrlSet_t *pArgFwModeCtrl)
{
   IFX_int32_t ret = 0;
   MEI_DEV_T *pMeiDev = pMeiDynCntrl->pMeiDev;
   MEI_FW_PORT_MODE_CONTROL_DMA32_T fwPortModeCtrl = {0};
#if (MEI_SUPPORT_DSM == 1)
   IOCTL_MEI_firmwareFeatures_t *pFwFeatures = &(pMeiDev->firmwareFeatures);
   IOCTL_MEI_VectorControl_t eFwVectorCfgCurrent = pMeiDev->meiDsmConfig.eVectorControl;
#endif /* (MEI_SUPPORT_DSM == 1) */

   /* Check current driver state. */
   if (MEI_DRV_STATE_GET(pMeiDev) != e_MEI_DRV_STATE_SW_INIT_DONE)
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DRV[%02d]: invalid state (%d) for setting FW mode!" MEI_DRV_CRLF,
              MEI_DRV_LINENUM_GET(pMeiDev), MEI_DRV_STATE_GET(pMeiDev) ));

      return -e_MEI_ERR_INVAL_STATE;
   }

   /* Get current FW Port Mode Control Structure*/
   ret = MEI_VRX_PortModeControlStructureCurrentGet(pMeiDev, &fwPortModeCtrl);
   if (ret != IFX_SUCCESS)
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DRV[%02d]: FW Port Mode Control structure get failed!" MEI_DRV_CRLF,
              MEI_DRV_LINENUM_GET(pMeiDev) ));

      return (-e_MEI_ERR_OP_FAILED);
   }

   /* Overwrite signatures*/
   fwPortModeCtrl.signature0 = MEI_FW_IMAGE_SIGNATURE0;
   fwPortModeCtrl.signature1 = MEI_FW_IMAGE_SIGNATURE1;

   /* Set Locks*/
   fwPortModeCtrl.dualPortModeLock = pArgFwModeCtrl->bMultiLineModeLock ?
      MEI_FW_PORT_MODE_LOCK : MEI_FW_PORT_MODE_UNLOCK;
   fwPortModeCtrl.xDslModeLock     = pArgFwModeCtrl->bXdslModeLock ?
      MEI_FW_XDSL_MODE_LOCK : MEI_FW_XDSL_MODE_UNLOCK;

   /* Set Line mode (Single/Dual)*/
   switch(pArgFwModeCtrl->eMultiLineModeCurrent)
   {
   case e_MEI_MULTI_LINEMODE_NA:
      /* nothing to change*/
      break;
   case e_MEI_MULTI_LINEMODE_SINGLE:
      fwPortModeCtrl.dualPortModeCurrent = fwPortModeCtrl.dualPortModePreffered =
         MEI_FW_PORT_MODE_SINGLE;
      break;
   case e_MEI_MULTI_LINEMODE_DUAL:
      fwPortModeCtrl.dualPortModeCurrent = fwPortModeCtrl.dualPortModePreffered =
         MEI_FW_PORT_MODE_DUAL;
      break;
   default:
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DRV[%02d]: Invalid eMultiLineModePreferred=%d specified!" MEI_DRV_CRLF,
              MEI_DRV_LINENUM_GET(pMeiDev), pArgFwModeCtrl->eMultiLineModePreferred));

      return (-e_MEI_ERR_INVAL_ARG);
   }

   /* Set xDSL Mode (VDSL/ADSL)*/
   switch(pArgFwModeCtrl->eXdslModeCurrent)
   {
   case e_MEI_XDSLMODE_NA:
      /* nothing to change*/
      break;
   case e_MEI_XDSLMODE_VDSL:
      fwPortModeCtrl.xDslModeCurrent = fwPortModeCtrl.xDslModePreffered =
         MEI_FW_XDSL_MODE_VDSL;
      break;
   case e_MEI_XDSLMODE_ADSL:
      fwPortModeCtrl.xDslModeCurrent = fwPortModeCtrl.xDslModePreffered =
         MEI_FW_XDSL_MODE_ADSL;
      break;
   default:
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DRV[%02d]: Invalid eXdslModePreferred=%d specified!" MEI_DRV_CRLF,
              MEI_DRV_LINENUM_GET(pMeiDev), pArgFwModeCtrl->eXdslModePreferred));

      return (-e_MEI_ERR_INVAL_ARG);
   }
   
#if (MEI_SUPPORT_DEVICE_VR11 == 1)
   /* Configure the PLL_OMCFG register */
   MEI_CGU_PLLOMCFG_CLK5_Set(&(pMeiDynCntrl->pMeiDev->meiDrvCntrl), fwPortModeCtrl.xDslModeCurrent);
#endif

   /* Set current FW Port Mode Control Structure*/
   ret = MEI_VRX_PortModeControlStructureCurrentSet(pMeiDev, &fwPortModeCtrl);
   if (ret != IFX_SUCCESS)
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DRV[%02d]: FW Port Mode Control structure set failed!" MEI_DRV_CRLF,
              MEI_DRV_LINENUM_GET(pMeiDev) ));

      return (-e_MEI_ERR_OP_FAILED);
   }

#if (MEI_SUPPORT_DSM == 1)
   /* Set default configuration *only* if not already done */
   if (pMeiDev->bDsmConfigInit == IFX_FALSE || pMeiDev->meiDsmConfig.eVectorControl == e_MEI_VECTOR_CTRL_AUTO)
   {
      pMeiDev->bDsmConfigInit = IFX_TRUE;

      if ( (pArgFwModeCtrl->firmwareFeatures.eFirmwareXdslModes) &
           (e_MEI_FW_XDSLMODE_VDSL2_VECTOR) )
      {
         pMeiDev->meiDsmConfig.eVectorControl = e_MEI_VECTOR_CTRL_ON;
      }
      else if( (pArgFwModeCtrl->firmwareFeatures.eFirmwareXdslModes) &
           (e_MEI_FW_XDSLMODE_VDSL2) )
      {
         pMeiDev->meiDsmConfig.eVectorControl = e_MEI_VECTOR_CTRL_OFF;
      }
      else
      {
         pMeiDev->meiDsmConfig.eVectorControl = e_MEI_VECTOR_CTRL_OFF;
      }
   }

   /* Set vectoring support level */
   if ( (pArgFwModeCtrl->firmwareFeatures.eFirmwareXdslModes) &
        (e_MEI_FW_XDSLMODE_VDSL2_VECTOR | e_MEI_FW_XDSLMODE_VDSL2) )
   {
      if (pArgFwModeCtrl->eXdslModeCurrent == e_MEI_XDSLMODE_VDSL)
      {
         pMeiDev->nFwVectorSupport = e_MEI_DSM_VECTOR_FW_SUPPORT_MODE_FULL;
      }
      else
      {
         pMeiDev->nFwVectorSupport = e_MEI_DSM_VECTOR_FW_SUPPORT_MODE_REDUCE;
      }
   }
   else
   {
      pMeiDev->nFwVectorSupport = e_MEI_DSM_VECTOR_FW_SUPPORT_MODE_NONE;
   }

   /* Current FW supports full vectoring, new FW does not support full vectoring */
   if ( (pFwFeatures->eFirmwareXdslModes &
         e_MEI_FW_XDSLMODE_VDSL2_VECTOR) &&
        (pArgFwModeCtrl->firmwareFeatures.eFirmwareXdslModes &
         e_MEI_FW_XDSLMODE_VDSL2) )
   {
      /* If full vectoring was enabled before, set vectoring friendly. */
      if (eFwVectorCfgCurrent == e_MEI_VECTOR_CTRL_ON)
      {
         pMeiDev->meiDsmConfig.eVectorControl = e_MEI_VECTOR_CTRL_FRIENDLY_ON;
      }
   }

   /* Current FW does not support full vectoring, new FW supports full vectoring */
   if ( (pFwFeatures->eFirmwareXdslModes &
         e_MEI_FW_XDSLMODE_VDSL2) &&
        (pArgFwModeCtrl->firmwareFeatures.eFirmwareXdslModes &
         e_MEI_FW_XDSLMODE_VDSL2_VECTOR) )
   {
      /* If vectoring friendly was enabled before, switch to full vectoring
         configuration. */
      if (eFwVectorCfgCurrent == e_MEI_VECTOR_CTRL_FRIENDLY_ON)
      {
         pMeiDev->meiDsmConfig.eVectorControl = e_MEI_VECTOR_CTRL_ON;
      }
   }

#endif /* (MEI_SUPPORT_DSM == 1) */

   /* Update firmware features within MEI driver context with values of new FW. */
   memcpy(&(pMeiDev->firmwareFeatures),
          &(pArgFwModeCtrl->firmwareFeatures),
          sizeof(IOCTL_MEI_firmwareFeatures_t));

   return ret;
}

/**
   Get FW xDSL/DualPort mode control status.

\param
   pMeiDynCntrl - private dynamic comtrol data (per open instance)
\param
   pArgFwModeStat - points to the xDSL/DualPort mode control data

\return
   IFX_SUCCESS: if the FW Mode Control status get was successful.
   negative value if something went wrong.
*/
IFX_int32_t MEI_IoctlFwModeStatGet(
                                 MEI_DYN_CNTRL_T           *pMeiDynCntrl,
                                 IOCTL_MEI_FwModeStatGet_t *pArgFwModeStat)
{
   IFX_int32_t ret = 0;
   MEI_DEV_T *pMeiDev = pMeiDynCntrl->pMeiDev;
   MEI_FW_PORT_MODE_CONTROL_DMA32_T fwPortModeCtrl = {0};

   /* Get current FW Port Mode Control Structure*/
   ret = MEI_VRX_PortModeControlStructureCurrentGet(pMeiDev, &fwPortModeCtrl);
   if (ret != IFX_SUCCESS)
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DRV[%02d]: FW Port Mode Control structure get failed!" MEI_DRV_CRLF,
              MEI_DRV_LINENUM_GET(pMeiDev) ));

      return (-e_MEI_ERR_OP_FAILED);
   }

   /* Check for a valid signatures*/
   if ( (fwPortModeCtrl.signature0 != MEI_FW_IMAGE_SIGNATURE0) ||
        (fwPortModeCtrl.signature1 != MEI_FW_IMAGE_SIGNATURE1))
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_WRN,
            ("MEI_DRV[%02d]: No valid FW Port Mode Control structure detected!" MEI_DRV_CRLF,
              MEI_DRV_LINENUM_GET(pMeiDev) ));

      pArgFwModeStat->eXdslModeCurrent = e_MEI_XDSLMODE_NA;
      pArgFwModeStat->eMultiLineModeCurrent = e_MEI_MULTI_LINEMODE_NA;
   }
   else
   {
      /* Set Current xDSL mode*/
      pArgFwModeStat->eXdslModeCurrent =
         fwPortModeCtrl.xDslModeCurrent == MEI_FW_XDSL_MODE_VDSL ? e_MEI_XDSLMODE_VDSL :
         fwPortModeCtrl.xDslModeCurrent == MEI_FW_XDSL_MODE_ADSL ? e_MEI_XDSLMODE_ADSL :
         e_MEI_XDSLMODE_NA;

      /* Set current Multi Port mode*/
      pArgFwModeStat->eMultiLineModeCurrent =
         fwPortModeCtrl.dualPortModeCurrent == MEI_FW_PORT_MODE_SINGLE ? e_MEI_MULTI_LINEMODE_SINGLE :
         fwPortModeCtrl.dualPortModeCurrent == MEI_FW_PORT_MODE_DUAL ? e_MEI_MULTI_LINEMODE_DUAL :
         e_MEI_MULTI_LINEMODE_NA;
   }

   return ret;
}

#if (MEI_SUPPORT_DSM == 1)
#if(MEI_SUPPORT_DEVICE_VR11 == 1)
/**
  Initalize BAR registers for ERB on VR11 platform

\param
   pMeiDev     points to the current VR11 channel device.

\return
   IFX_SUCCESS
   IFX_ERROR
*/
IFX_int32_t MEI_VR11_ErbBarSet(
                                 MEI_DEV_T *pMeiDev,
                                 unsigned int data,
                                 unsigned int descriptor)
{
   IFX_int32_t ret = 0;
   IFX_uint32_t barIdx;
   MEI_MEI_DRV_CNTRL_T *pMeiDrvCntrl = &(pMeiDev->meiDrvCntrl);
   /* chunkIdx points to ERB block */
   barIdx = MEI_FW_IMAGE_ERB_CHUNK_INDEX;

   /* Update BAR register pointed to ERB block */
   MEI_REG_ACCESS_ME_XMEM_BAR_SET(pMeiDrvCntrl, barIdx, data);
   MEI_BAR_TYPE_SET(pMeiDev, barIdx, eMEI_BAR_TYPE_ERB);
   PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL,
      ("BAR[%02d] = 0x%08X (-> ERB block)"MEI_DRV_CRLF, barIdx,
      MEI_REG_ACCESS_ME_XMEM_BAR_GET(pMeiDrvCntrl, barIdx)));

   barIdx = MEI_FW_IMAGE_ERB_PPE_CHUNK_INDEX;
   MEI_REG_ACCESS_ME_XMEM_BAR_SET(pMeiDrvCntrl, barIdx, descriptor);
   MEI_BAR_TYPE_SET(pMeiDev, barIdx, eMEI_BAR_TYPE_ERB);
   PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL,
      ("BAR[%02d] = 0x%08X (-> ERB block)"MEI_DRV_CRLF, barIdx,
      MEI_REG_ACCESS_ME_XMEM_BAR_GET(pMeiDrvCntrl, barIdx)));

   return ret;
}

#endif /* (MEI_SUPPORT_DEVICE_VR11 == 1) */
#endif /* (MEI_SUPPORT_DSM == 1) */

#if(MEI_SUPPORT_DEVICE_VR11 == 1)
IFX_int32_t MEI_VR11_BarSafeLocationAddressUpdate(
                                 MEI_DEV_T *pMeiDev)
{
   IFX_int32_t ret = 0;
   IFX_uint32_t barIdx;
   MEI_MEI_DRV_CNTRL_T *pMeiDrvCntrl;

   pMeiDrvCntrl = &(pMeiDev->meiDrvCntrl);

   /* BAR18 */
   barIdx = MEI_FW_IMAGE_CHIPID_EFUSE_CHUNK_INDEX;

   MEI_REG_ACCESS_ME_XMEM_BAR_SET(pMeiDrvCntrl, barIdx, (unsigned int)pMeiDev->barSafeAddr);
   MEI_BAR_TYPE_SET(pMeiDev, barIdx, eMEI_BAR_TYPE_SPECIAL);
   PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL,
      ("BAR[%02d] = 0x%08X (-> CHIPID_EFUSE + GPIO_FUNC)"MEI_DRV_CRLF, barIdx,
      MEI_REG_ACCESS_ME_XMEM_BAR_GET(pMeiDrvCntrl, barIdx)));

   /* BAR19 */
   barIdx = MEI_FW_IMAGE_FREQ_SCAL_PPE_CHUNK_INDEX;

   MEI_REG_ACCESS_ME_XMEM_BAR_SET(pMeiDrvCntrl, barIdx,  (unsigned int)pMeiDev->barSafeAddr);
   MEI_BAR_TYPE_SET(pMeiDev, barIdx, eMEI_BAR_TYPE_SPECIAL);
   PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL,
      ("BAR[%02d] = 0x%08X (-> CGU / PLL_OMCFG)"MEI_DRV_CRLF, barIdx,
      MEI_REG_ACCESS_ME_XMEM_BAR_GET(pMeiDrvCntrl, barIdx)));

   return ret;
}
#endif /* (MEI_SUPPORT_DEVICE_VR11 == 1) */

