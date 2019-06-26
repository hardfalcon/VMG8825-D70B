/******************************************************************************

                          Copyright (c) 2007-2015
                     Lantiq Beteiligungs-GmbH & Co. KG

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/* ==========================================================================
   Description : VR9/VR10/AR9 device specific functions for the MEI CPE Driver
   ========================================================================== */

/* ============================================================================
   Includes
   ========================================================================= */

/* get at first the driver configuration */
#include "drv_mei_cpe_config.h"

#include "ifx_types.h"
#include "drv_mei_cpe_os.h"
#include "drv_mei_cpe_dbg.h"

#include "drv_mei_cpe_interface.h"
#include "drv_mei_cpe_mei_interface.h"
#include "drv_mei_cpe_api.h"
#include "drv_mei_cpe_mei_vrx.h"

#if defined(LINUX)
#  if (LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0))
#    if (MEI_SUPPORT_DEVICE_VR10_320 == 1)
#       include "../drivers/net/ethernet/lantiq/lantiq_pcie.h"
#    else
#       include "ifx_pcie.h"
#    endif
#  else
#    if (MEI_SUPPORT_DEVICE_VR11 == 1)
#       include "net/dc_ep.h"
#    elif (MEI_SUPPORT_DEVICE_VR10_320 == 1)
#       include "../drivers/net/ethernet/lantiq/lantiq_pcie.h"
#    else
#       include "lantiq_pcie.h"
#    endif
#  endif
#endif /* #if defined(LINUX)*/

IFX_int32_t MEI_GPIntProcess(MEI_MeiRegVal_t processInt, MEI_DEV_T *pMeiDev)
{
   if (processInt & ME_ARC2ME_STAT_ARC_GP_INT0)
   {
      PRN_ERR_INT_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
           ("MEI_DRV[%02d]: - GP INT0 occurred --> FW generated" MEI_DRV_CRLF,
           MEI_DRV_LINENUM_GET(pMeiDev)));
   }

   return IFX_SUCCESS;
}

/**
   Read the Chip Info Register from the VRX devcie.
   - Register offset 0x40 CHIPCFG
   - baseaddress: 0x000E2000 - 0x000E20FF

\param
   pMeiDev  points to the device data

\return
   IFX_SUCCESS - CHIP info available and valid.
   IFX_ERROR   - invalid/not supported bootmode.
*/
IFX_int32_t MEI_GetChipInfo(MEI_DEV_T *pMeiDev)
{
   PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_WRN,
        ("MEI_DRV[%02d]: Boot mode (0x%02X) - autonomous" MEI_DRV_CRLF,
          MEI_DRV_LINENUM_GET(pMeiDev), pMeiDev->modemData.devBootMode));
   MEI_DRV_BOOTMODE_SET(pMeiDev, e_MEI_DRV_BOOT_MODE_AUTO);

   return IFX_SUCCESS;
}

/**
   Check for correct entity value

\param
   nEntityNum:    device number - identify the given device.

\return
   IFX_SUCCESS    Success
   IFX_ERROR      in case of error
*/
IFX_int32_t MEI_VR1x_PcieEntitiesCheck(IFX_uint8_t nEntityNum)
{
   IFX_uint32_t pcie_entitiesNum;

   /* get information from pcie driver */
#if (MEI_SUPPORT_DEVICE_VR11 == 1)
   if (dc_ep_dev_num_get(&pcie_entitiesNum))
#elif (MEI_SUPPORT_DEVICE_VR10_320 == 1)
   if (ltq_pcie_ep_dev_num_get(&pcie_entitiesNum))
#else
   if (ifx_pcie_ep_dev_num_get(&pcie_entitiesNum))
#endif
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
         ("PCIe: failed to get total device number" MEI_DRV_CRLF));

      return IFX_ERROR;
   }
   if (nEntityNum >= pcie_entitiesNum)
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
             ("PCIe: - invalid device num %d, max pcie devices attached %d"
              MEI_DRV_CRLF, nEntityNum, pcie_entitiesNum));

      return IFX_ERROR;
   }

   return IFX_SUCCESS;
}

/**
   Init pcie related info (membase, irq number)

\param
   pMeiDrvCntrl:   points to the MEI interface register set.

\return
   IFX_SUCCESS    Success
   IFX_ERROR      in case of error
*/
IFX_int32_t MEI_VR1x_PcieEntityInit(MEI_MEI_DRV_CNTRL_T *pMeiDrvCntrl)
{
   IFX_uint8_t entityNum;
#if (MEI_SUPPORT_DEVICE_VR11 == 1)
   struct dc_ep_dev MEI_pcie_ep_dev;
#elif (MEI_SUPPORT_DEVICE_VR10_320 == 1)
   ltq_pcie_ep_dev_t MEI_pcie_ep_dev;
#else
   ifx_pcie_ep_dev_t MEI_pcie_ep_dev;
#endif

   entityNum = MEI_GET_ENTITY_FROM_DEVNUM(pMeiDrvCntrl->dslLineNum);

#if (MEI_SUPPORT_DEVICE_VR11 == 1)
   if (dc_ep_dev_info_req(entityNum, DC_EP_INT_MEI, &MEI_pcie_ep_dev))
#elif (MEI_SUPPORT_DEVICE_VR10_320 == 1)
   if (ltq_pcie_ep_dev_info_req(entityNum, IFX_PCIE_EP_INT_MEI, &MEI_pcie_ep_dev))
#else
   if (ifx_pcie_ep_dev_info_req(entityNum, IFX_PCIE_EP_INT_MEI, &MEI_pcie_ep_dev))
#endif
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
         ("PCIe: failed to get EP device %i information" MEI_DRV_CRLF, entityNum));

      return IFX_ERROR;
   }

   PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_LOW,
      ("PCIe: EP device %i, irq %i, physical membase 0x%08x, virtual membase %p" MEI_DRV_CRLF,
      entityNum, MEI_pcie_ep_dev.irq, MEI_pcie_ep_dev.phy_membase, MEI_pcie_ep_dev.membase));

   pMeiDrvCntrl->MEI_pcie_phy_membase = (IFX_uint32_t)MEI_pcie_ep_dev.phy_membase;
   pMeiDrvCntrl->MEI_pcie_virt_membase = (IFX_uint32_t)MEI_pcie_ep_dev.membase;
#ifdef IRQ_POLLING_FORCE
   PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL,
      ("Force IRQ polling mode to: \"%i\", normal IRQ ignored" MEI_DRV_CRLF,
      IRQ_POLLING_FORCE));
   pMeiDrvCntrl->MEI_pcie_irq = IRQ_POLLING_FORCE; /* polling mode */
#else
   pMeiDrvCntrl->MEI_pcie_irq = MEI_pcie_ep_dev.irq;
#endif

   return IFX_SUCCESS;
}

/**
   Release usage PCIe module by MEI driver

\param
   entityNum:    device number - identify the given device.

\return
   IFX_SUCCESS    Success
   IFX_ERROR      in case of error
*/
IFX_int32_t MEI_VR1x_PcieEntityFree(IFX_uint8_t entityNum)
{
#if (MEI_SUPPORT_DEVICE_VR11 == 1)
   if (dc_ep_dev_info_release(entityNum))
#elif (MEI_SUPPORT_DEVICE_VR10_320 == 1)
   if (ltq_pcie_ep_dev_info_release(entityNum))
#else
   if (ifx_pcie_ep_dev_info_release(entityNum))
#endif
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
         ("PCIe: failed to release EP device %i" MEI_DRV_CRLF, entityNum));

      return IFX_ERROR;
   }

   return IFX_SUCCESS;
}

/**
   Internal init device with info provided by pcie driver

\param
   pMeiDynCntrl   - private dynamic device data (per open instance)

\return
   IFX_SUCCESS    Success
   IFX_ERROR      in case of error
*/
IFX_int32_t MEI_VR1x_InternalInitDevice(MEI_DYN_CNTRL_T *pMeiDynCntrl)
{
   IFX_int32_t         retVal;
   IOCTL_MEI_devInit_t InitDev;
   MEI_DEV_T           *pMeiDev = pMeiDynCntrl->pMeiDev;

   /* Init membase addresses and driver for the first usage */
   if (MEI_DRV_STATE_GET(pMeiDev) == e_MEI_DRV_STATE_NOT_INIT)
   {
      InitDev.usedIRQ = MEI_DRV_PCIE_IRQ_GET(&pMeiDev->meiDrvCntrl);
      InitDev.meiBaseAddr = MEI_DRV_PCIE_PHY_MEMBASE_GET(&pMeiDev->meiDrvCntrl)
                              + MEI_DSL_MEI_OFFSET;
      InitDev.PDBRAMaddr = MEI_DRV_PCIE_PHY_MEMBASE_GET(&pMeiDev->meiDrvCntrl)
                              + MEI_PDBRAM_OFFSET;

      if ((retVal = MEI_InternalInitDevice(pMeiDynCntrl, &InitDev)) != IFX_SUCCESS)
      {
         return retVal;
      }
   }

   /** \todo [VRX518] Confirm that it should be excluded */
#if (MEI_SUPPORT_DEVICE_VR10 == 1) || (MEI_SUPPORT_DEVICE_VR10_320 == 1)
   /* Clear LIF bits (0, 3, 8) of P0_ALSEL0 and P0_ALSEL1 */
   *MEI_GPIO_U32REG(GPIO_P0_ALSEL0) &= ~((1 << 0) | (1 << 3) | (1 << 8));
   *MEI_GPIO_U32REG(GPIO_P0_ALSEL1) &= ~((1 << 0) | (1 << 3) | (1 << 8));
#endif

   return IFX_SUCCESS;
}

IFX_int32_t MEI_PLL_ConfigInit(MEI_DEV_T *pMeiDev)
{
   pMeiDev->modemData.nPllOffset = MEI_PLL_DISABLED;

   return IFX_SUCCESS;
}

IFX_int32_t MEI_PLL_ConfigSet(MEI_DYN_CNTRL_T *pMeiDynCntrl)
{
   MEI_DEV_T *pMeiDev = pMeiDynCntrl->pMeiDev;
   IFX_int32_t ret = 0;
   CMD_PLL_ClockSet_t sCmd;
   ACK_PLL_ClockSet_t sAck;

   if (pMeiDev->modemData.nPllOffset != MEI_PLL_DISABLED)
   {
      memset(&sCmd, 0x00, sizeof(sCmd));
      memset(&sAck, 0x00, sizeof(sAck));
      sCmd.Length = 1;

      sCmd.pllClockOffset = (IFX_uint16_t)(pMeiDev->modemData.nPllOffset);

      ret = MEI_InternalSendMessage(pMeiDynCntrl, CMD_PLL_CLOCKSET,
                                 sizeof(sCmd), (unsigned char *)&sCmd,
                                 sizeof(sAck), (unsigned char *)&sAck);
   }

   return ret;
}

/**
   Get modem state via firmware message
   Use MEI_DRV_MODEM_STATE_GET macro to get received state from context

\param
   pMeiDynCntrl   - private dynamic device data (per open instance)

\return
   NONE
*/
IFX_void_t MEI_VRX_ModemStateUpdate(MEI_DYN_CNTRL_T *pMeiDynCntrl)
{
   IFX_int32_t ret = 0;
   MEI_DEV_T *pMeiDev = pMeiDynCntrl->pMeiDev;
   CMD_ModemFSM_StateGet_t sCmd;
   ACK_ModemFSM_StateGet_t sAck;

   memset(&sCmd, 0x00, sizeof(sCmd));
   memset(&sAck, 0x00, sizeof(sAck));
   sCmd.Length = 1;

   ret = MEI_InternalSendMessage(pMeiDynCntrl, CMD_MODEMFSM_STATEGET,
                                 sizeof(sCmd), (unsigned char *)&sCmd,
                                 sizeof(sAck), (unsigned char *)&sAck);
   if (ret < 0)
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
         ("MEI_DRV[0x%02X]: fail to get modem state!" MEI_DRV_CRLF,
         MEI_DRV_LINENUM_GET(pMeiDev)));
   }
}

IFX_boolean_t MEI_PAF_EnableGet(MEI_DYN_CNTRL_T *pMeiDynCntrl)
{
   MEI_DEV_T *pMeiDev = pMeiDynCntrl->pMeiDev;
   IFX_boolean_t ret = IFX_FALSE;
   CMD_PAF_HS_StatusGet_t sCmd;
   ACK_PAF_HS_StatusGet_t sAck;

   memset(&sCmd, 0x00, sizeof(sCmd));
   memset(&sAck, 0x00, sizeof(sAck));
   sCmd.Length = 6;

   ret = MEI_InternalSendMessage(pMeiDynCntrl, CMD_PAF_HS_STATUSGET,
                                 sizeof(sCmd), (unsigned char *)&sCmd,
                                 sizeof(sAck), (unsigned char *)&sAck);

   if (ret < 0)
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
         ("MEI_DRV[0x%02X]: DEBUG: fail to get bonding status!" MEI_DRV_CRLF,
         MEI_DRV_LINENUM_GET(pMeiDev)));
   }

   return (IFX_boolean_t) sAck.PAF_Enable;
}

