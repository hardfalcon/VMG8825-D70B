/******************************************************************************

                          Copyright (c) 2007-2015
                     Lantiq Beteiligungs-GmbH & Co. KG

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/* ==========================================================================
   Description : Common functions for the MEI CPE Driver
   ========================================================================== */

/* ============================================================================
   Includes
   ========================================================================= */

/* get at first the driver configuration */
#include "drv_mei_cpe_config.h"

#include "ifx_types.h"
#include "drv_mei_cpe_os.h"
#include "drv_mei_cpe_dbg.h"

#include "drv_mei_cpe_dbg_driver.h"

/** get interface and configuration */
#include "drv_mei_cpe_interface.h"
#include "drv_mei_cpe_mei_interface.h"
#include "drv_mei_cpe_api.h"

#include "drv_mei_cpe_download.h"

#if (MEI_SUPPORT_DSM == 1)
#include "drv_mei_cpe_dsm.h"
#endif /* (MEI_SUPPORT_DSM == 1) */

#include "drv_mei_cpe_mailbox.h"
#include "drv_mei_cpe_msg_process.h"
#include "drv_mei_cpe_device_cntrl.h"

#if (MEI_SUPPORT_DRIVER_MSG == 1)
#include "drv_mei_cpe_driver_msg.h"
#endif

#if (MEI_SUPPORT_DFE_GPA_ACCESS == 1)
#include "drv_mei_cpe_dbg_access.h"
#endif

#if (MEI_SUPPORT_ROM_CODE == 1)
#include "drv_mei_cpe_rom_handler_if.h"
#include "drv_mei_cpe_rom_handler.h"
#endif      /* MEI_SUPPORT_ROM_CODE */

#if (MEI_DRV_ATM_OAM_ENABLE == 1)
#include "drv_mei_cpe_atmoam_common.h"
#endif

#if (MEI_DRV_CLEAR_EOC_ENABLE == 1)
#include "drv_mei_cpe_clear_eoc_common.h"
#endif

#if (MEI_SUPPORT_DEBUG_STREAMS == 1)
#include "drv_mei_cpe_dbg_streams.h"
#endif

#include <linux/kthread.h>

/* ============================================================================
   Local Macros & Definitions
   ========================================================================= */

#ifdef MEI_STATIC
#undef MEI_STATIC
#endif

#ifdef MEI_DEBUG
#define MEI_STATIC
#else
#define MEI_STATIC   static
#endif


/* ============================================================================
   Global variable definition
   ========================================================================= */

/** what string support, driver version string */
const char MEI_WHATVERSION[] = DRV_MEI_WHAT_STR;

/** pointer to device structures. */
MEIX_CNTRL_T *MEIX_Cntrl[MEI_MAX_SUPPORTED_DFEX_ENTITIES] = {0};

/** decrement counter to protect "empty" IRQ requests form the OS */
IFX_int32_t MEI_IrqProtectCount = MEI_IRQ_PROTECT_COUNT;

/* each bit marks a pre-action required for the msg FSM State Set (0x0041)
   - Bit0: enable Eth OAM
   - Bit0: enable ATM OAM
   - Bit0: enable Clear Eoc
*/
IFX_uint32_t MEI_FsmStateSetMsgPreAction = 0;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)) || (MEI_SUPPORT_DEVICE_VR10_320 == 1)
/** device tree data */
MEI_DEVCFG_DATA_T MEI_DevCfgData;
#endif

/* ============================================================================
   Proc-FS and debug variable definitions
   ========================================================================= */

/* MEI CPE-Driver: Common debug module - create print level variable */
MEI_DRV_PRN_USR_MODULE_CREATE(MEI_DRV, MEI_DRV_PRN_LEVEL_LOW);
MEI_DRV_PRN_INT_MODULE_CREATE(MEI_DRV, MEI_DRV_PRN_LEVEL_LOW);

/* MEI CPE-Driver: fw message dump debug module - create print level variable */
MEI_DRV_PRN_USR_MODULE_CREATE(MEI_MSG_DUMP_API, MEI_DRV_PRN_LEVEL_LOW);
MEI_DRV_PRN_INT_MODULE_CREATE(MEI_MSG_DUMP_API, MEI_DRV_PRN_LEVEL_LOW);

/* MEI CPE-Driver: notifications debug module - create print level variable */
MEI_DRV_PRN_USR_MODULE_CREATE(MEI_NOTIFICATIONS, MEI_DRV_PRN_LEVEL_LOW);
MEI_DRV_PRN_INT_MODULE_CREATE(MEI_NOTIFICATIONS, MEI_DRV_PRN_LEVEL_LOW);

/* Block timeout (for debugging)
   0: timeout not blocked
   1: timeout blocked
*/
IFX_int32_t MEI_BlockTimeout = 0;

/**
   Max waittime for the AUTONOMOUS_MODEM_READY message.
*/
IFX_uint32_t MEI_MaxWaitForModemReady_ms = MEI_CFG_DEF_WAIT_FOR_MODEM_READY_SEC;

/* Max time Wait: Device Response (ACK, and common handling) */
IFX_int32_t MEI_MaxWaitDfeResponce_ms    = MEI_CFG_DEF_WAIT_FOR_DEVICE_RESPONCE;

/**
   Download via ROM handler will be only possible if the
   CHIP_ID will match.
*/
IFX_uint8_t MEI_DefDownLoadChipId = MEI_DEF_DOWNLOAD_CHIPID;

/* for testing */
IFX_uint32_t MEI_MailboxBase_ME2ARC = MEI_BOOT_MAILBOX_ME2ARC_ADDR;
IFX_uint32_t MEI_MailboxBase_ARC2ME = MEI_BOOT_MAILBOX_ARC2ME_ADDR;

/* Bitmask, currently used bit 0:
      0: ARC is released out of HALT after FW download (DEFAULT!)
      1: ARC is NOT released out of HALT after FW download */
IFX_int32_t MEI_DbgFlags = 0;

#if (MEI_SUPPORT_DEBUG_LOGGER == 1)
/* Logger usage:
      0: Statndart destination is used for debug output (DEFAULT!)
      1: Logger is used for debug output */
IFX_int32_t MEI_DbgLogger = 0;
#endif /* (MEI_SUPPORT_DEBUG_LOGGER == 1) */

#if (MEI_SUPPORT_VDSL2_ADSL_SWAP == 1)
IFX_uint32_t MEI_fwModeSelect  = 0x0;
#endif

/* ============================================================================
   Local function declaration
   ========================================================================= */

MEI_STATIC IFX_int32_t MEI_WaitForFirstResponce(
                              MEI_DEV_T *pMeiDev);

MEI_STATIC IFX_int32_t MEI_MeiDevCntrlAlloc(
                              MEIX_CNTRL_T *pMeiXCntrl,
                              IFX_int32_t    maxPorts);

MEI_STATIC IFX_void_t MEI_MeiDevCntrlFree(
                              MEIX_CNTRL_T *pMeiXCntrl,
                              IFX_int32_t    maxIf,
                              IFX_int32_t    maxPorts);

MEI_STATIC IFX_uint32_t MEI_ProcessIntPerChip(
                              MEIX_CNTRL_T *pMeiXCntrl);

MEI_STATIC IFX_int32_t MEI_DrvAndDevResetAct(
                              MEI_DEV_T *pMeiDev,
                              IFX_uint32_t rstSelect,
                              IFX_int32_t rstSrc );
MEI_STATIC IFX_int32_t MEI_DrvAndDevResetDeAct(
                              MEI_DEV_T *pMeiDev,
                              IFX_uint32_t rstSelect);

MEI_STATIC void MEI_VersionParse(IFX_uint8_t *pVersion, IFX_uint8_t *pMajor,
                                 IFX_uint8_t *pMinor, IFX_uint8_t *pStep);


/* ============================================================================
   Local variable definition
   ========================================================================= */

/* ============================================================================
   Helper's functions
   ========================================================================= */


/* ============================================================================
   Local function definition
   ========================================================================= */

/**
   Wait for the first response from the already started MEI device.
   - MODEM READY:    in case of autonomous boot
   - EVT_INIT_DONE   in case auf ROM handler dwonload.

\param
   *pMeiDev  private device data

\return
   IFX_SUCCESS    Success
   IFX_ERROR      in case of error
*/
MEI_STATIC IFX_int32_t MEI_WaitForFirstResponce(MEI_DEV_T *pMeiDev)
{
   IFX_int32_t ret = IFX_SUCCESS;

   /*
      wait for modem alive
   */
   switch(MEI_DRV_STATE_GET(pMeiDev))
   {
      case e_MEI_DRV_STATE_WAIT_FOR_FIRST_RESP:
      case e_MEI_DRV_STATE_BOOT_WAIT_ROM_ALIVE:
         ret = IFX_SUCCESS;
         MEI_SET_TIMEOUT_CNT( pMeiDev,
               ((MEI_MaxWaitForModemReady_ms & ~MEI_CFG_DEF_WAIT_PROTECTION_FLAG) /
                MEI_MIN_MAILBOX_POLL_TIME_MS));

         while(ret == IFX_SUCCESS)
         {
            /* check if msg already in the mailbox */
            MEI_PollIntPerVrxLine(pMeiDev, e_MEI_DEV_ACCESS_MODE_PASSIV_POLL);

            /* check if modem read / ROM init done received */
            if (    (MEI_DRV_STATE_GET(pMeiDev) == e_MEI_DRV_STATE_DFE_READY)
                 #if (MEI_SUPPORT_ROM_CODE == 1)
                 || (MEI_DRV_STATE_GET(pMeiDev) == e_MEI_DRV_STATE_BOOT_ROM_ALIVE)
                 #endif
               )
               break;
            else
            {
               if ( (MEI_GET_TIMEOUT_CNT(pMeiDev) % 10) == 0)
                  PRN_DBG_USR( MEI_DRV, MEI_DRV_PRN_LEVEL_HIGH, MEI_DRV_LINENUM_GET(pMeiDev),
                        ("*" MEI_DRV_CRLF));
            }

            ret = MEI_WaitForMailbox(pMeiDev);
         }
         break;

      case e_MEI_DRV_STATE_DFE_READY:
      case e_MEI_DRV_STATE_BOOT_ROM_ALIVE:
         PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_WRN,
                ("MEI_DRV[%02d]: Wait for 1. response - already alive (state %d)" MEI_DRV_CRLF,
                  MEI_DRV_LINENUM_GET(pMeiDev), MEI_DRV_STATE_GET(pMeiDev)));
         ret = IFX_SUCCESS;
         break;

      default:
         /* invalid boot state */
         PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
                ("MEI_DRV[%02d]: ERROR Wait for device - invalid drv state %d" MEI_DRV_CRLF,
                  MEI_DRV_LINENUM_GET(pMeiDev), MEI_DRV_STATE_GET(pMeiDev)));

         return -e_MEI_ERR_INVAL_STATE;
   }

   return ret;
}

/**
   Allocate MEI interface device specific control block.

\param
   pMeiXCntrl - points to the Chip device control struct.
\param
   maxPorts     - max number of ports per die

\return
   IFX_SUCCESS - in case of success.
   IFX_ERROR   - in case of error.
*/
MEI_STATIC IFX_int32_t MEI_MeiDevCntrlAlloc(
                              MEIX_CNTRL_T *pMeiXCntrl,
                              IFX_int32_t    maxIf)
{
   IFX_int32_t           ifIdx;
   MEI_MEI_DEV_CNTRL_T *pMeiDevCntrl = IFX_NULL;

   for (ifIdx = 0; ifIdx < maxIf; ifIdx++)
   {
      if ((pMeiDevCntrl = pMeiXCntrl->pMeiDevCntrl[ifIdx]) == IFX_NULL)
      {
         /* allocate and init MEI device control block */
         pMeiDevCntrl = (MEI_MEI_DEV_CNTRL_T*)MEI_DRVOS_Malloc(sizeof(MEI_MEI_DEV_CNTRL_T));
         if (!pMeiDevCntrl)
         {
            PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
                  ("MEI_DRV MEIx[%02d]: ERROR MEI Cntrl Struct Allocate - "
                   "no memory." MEI_DRV_CRLF, pMeiXCntrl->entityNum));

            return IFX_ERROR;
         }

         memset( (char*)pMeiDevCntrl, 0x00, sizeof(MEI_MEI_DEV_CNTRL_T) );
         pMeiDevCntrl->relPort = ifIdx;

         if (MEI_DRVOS_SemaphoreInit(&pMeiDevCntrl->pDevMeiAccessLock) != IFX_SUCCESS)
         {
            PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
                  ("MEI_DRV MEIx[%02d]: ERROR MEI Cntrl Struct Allocate - "
                   "init device lock" MEI_DRV_CRLF, pMeiXCntrl->entityNum));

            MEI_DRVOS_Free(pMeiDevCntrl);
            pMeiDevCntrl = IFX_NULL;
            return IFX_ERROR;
         }

         /* set block into the device control struct */
         pMeiXCntrl->pMeiDevCntrl[ifIdx] = pMeiDevCntrl;
         pMeiDevCntrl = IFX_NULL;
      }
   }

   return IFX_SUCCESS;
}

/**
   Free MEI interface block.

\param
   pMeiXCntrl - points to the Chip device control struct.

\return
   none
*/
MEI_STATIC IFX_void_t MEI_MeiDevCntrlFree(
                              MEIX_CNTRL_T *pMeiXCntrl,
                              IFX_int32_t    maxIf,
                              IFX_int32_t    maxPorts)
{
   MEI_MEI_DEV_CNTRL_T *pMeiDevCntrl = IFX_NULL;
   IFX_int32_t           ifIdx, portIdx;

   for (ifIdx = 0; ifIdx < maxIf; ifIdx++)
   {
      if ((pMeiDevCntrl = pMeiXCntrl->pMeiDevCntrl[ifIdx]) != IFX_NULL)
      {
         MEI_DRVOS_SemaphoreLock(&pMeiDevCntrl->pDevMeiAccessLock);

         pMeiXCntrl->pMeiDevCntrl[ifIdx] = IFX_NULL;

         for (portIdx = 0; portIdx < maxPorts; portIdx++)
         {
            if (pMeiDevCntrl->meiIf[portIdx].phyBaseAddr)
            {
               if (pMeiDevCntrl->meiIf[portIdx].pVirtMeiRegIf)
               {
                  if (MEI_DEVICE_CFG_IS_PLATFORM(e_MEI_DEV_PLATFORM_CONFIG_VR9))
                  {
                     /* do not io unremap for VR10, it is already done in PCIe */
                     MEI_DRVOS_Phy2VirtUnmap(
                        &pMeiDevCntrl->meiIf[portIdx].phyBaseAddr,
                        (IFX_uint32_t)sizeof(MEI_MEI_REG_IF_U),
                        (IFX_uint8_t **)&(pMeiDevCntrl->meiIf[portIdx].pVirtMeiRegIf));
                  }
                  else
                  {
                      /* For all platforms that are using PCIe interface as follows
                         - MEI_DEVICE_CFG_IS_PLATFORM(e_MEI_DEV_PLATFORM_CONFIG_VR10)
                         - MEI_DEVICE_CFG_IS_PLATFORM(e_MEI_DEV_PLATFORM_CONFIG_VR10_320)
                         - MEI_DEVICE_CFG_IS_PLATFORM(e_MEI_DEV_PLATFORM_CONFIG_VR11) */
                      MEI_VR1x_PcieEntityFree(pMeiXCntrl->entityNum);
                  }

                  pMeiDevCntrl->meiIf[portIdx].pVirtMeiRegIf = IFX_NULL;
                  pMeiDevCntrl->meiIf[portIdx].eMeiHwState   = e_MEI_MEI_HW_STATE_UNKNOWN;
               }
               pMeiDevCntrl->meiIf[portIdx].phyBaseAddr = 0;
            }
         }

         MEI_DRVOS_SemaphoreUnlock(&pMeiDevCntrl->pDevMeiAccessLock);
         MEI_DRVOS_SemaphoreDelete(&pMeiDevCntrl->pDevMeiAccessLock);

         MEI_DRVOS_Free(pMeiDevCntrl);
         pMeiDevCntrl = IFX_NULL;
      }
   }

   return;
}


/* ============================================================================
   Global function definition
   ========================================================================= */


/**
   Allocate device control structure for a given entity number.
   - Check the entity control struct, and allocate if not exists.
   - Save the struct within the corresponding global struct.

\param
   entityNum:    device number - identify the given device.

\return
   SUCCESS: pointer to the Device control struct.
   ERROR:   IFX_NULL - not enough memory or invalid entity number.
*/
MEIX_CNTRL_T *MEI_DevXCntrlStructAlloc(IFX_uint8_t entityNum)
{
   MEIX_CNTRL_T  *pXCntrl = NULL;

   if (! MEI_DEVICE_CFG_IS_PLATFORM(e_MEI_DEV_PLATFORM_CONFIG_VR9))
   {
      if (MEI_VR1x_PcieEntitiesCheck(entityNum) != IFX_SUCCESS)
      {
         PRN_ERR_USR_NL(MEI_DRV, MEI_DRV_PRN_LEVEL_ERR, ("MEIX_DRV[--]: ERROR "
                   "Cntrl Struct Allocate - invalid device num" MEI_DRV_CRLF));
         return IFX_NULL;
      }
   }

       if (entityNum >= MEI_DFEX_ENTITIES)
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
             ("MEIX_DRV[--]: ERROR Cntrl Struct Allocate - invalid device num %d, max %d (-1)" MEI_DRV_CRLF,
              entityNum, MEI_DFEX_ENTITIES));

      return IFX_NULL;
   }

   /*
      Allocate the global data structs (kept until cleanup)
   */
   if ( (pXCntrl = MEIX_Cntrl[entityNum]) == NULL)
   {
      /* allocate and reset new chip control data block */
      PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_LOW,
            ("MEI_DRV: Cntrl Struct Allocate - Create MEIX[%d], (size = %d byte)" MEI_DRV_CRLF,
              entityNum, sizeof(MEIX_CNTRL_T)));

      pXCntrl = (MEIX_CNTRL_T*)MEI_DRVOS_Malloc(sizeof(MEIX_CNTRL_T));
      if (!pXCntrl)
      {
         PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
               ("MEI_DRV DfeX[%02d]: ERROR Cntrl Struct Allocate - no memory." MEI_DRV_CRLF,
                entityNum));
         return IFX_NULL;
      }

      memset( (char*)pXCntrl, 0x00, sizeof(MEIX_CNTRL_T) );
      pXCntrl->entityNum      = entityNum;
      pXCntrl->IRQ_Protection = MEI_IrqProtectCount;

      /* allocate and setup the MEI interface block */
      if ( MEI_MeiDevCntrlAlloc( pXCntrl,
                                   MEI_MAX_SUPPORTED_MEI_IF_PER_DEVICE) != IFX_SUCCESS )
      {
         MEI_MeiDevCntrlFree( pXCntrl,
                                MEI_MAX_SUPPORTED_MEI_IF_PER_DEVICE, 1);
         MEI_DRVOS_Free(pXCntrl);
         pXCntrl = IFX_NULL;

         return IFX_NULL;
      }

      MEIX_Cntrl[entityNum] = pXCntrl;
   }

   return pXCntrl;
}

/**
   If exists free a device control structure for a given device number.

\param
   entity  chip entity.

\return
   IFX_SUCCESS   control struct has been freed or was already free.
   IFX_ERROR     struct not freed - device lines still in use
*/
IFX_int32_t MEI_DevXCntrlStructFree(IFX_uint8_t entity)
{
   IFX_int8_t i=0;
   MEIX_CNTRL_T *pXCntrl = NULL;

   /* check if chip control block exists */
   if ( (pXCntrl = MEIX_Cntrl[entity]) != NULL)
   {
      for (i=0; i<MEI_DFE_INSTANCE_PER_ENTITY; i++)
      {
         /* check for further devices */
         if ( pXCntrl->MeiDevice[i] != NULL)
            break;
      }

      if ( i >= MEI_DFE_INSTANCE_PER_ENTITY )
      {
         /* no further devices on this chip control */
         MEIX_Cntrl[entity] = NULL;
         MEI_MeiDevCntrlFree(pXCntrl, MEI_DFE_INSTANCE_PER_ENTITY, 1);
         MEI_DRVOS_Free(pXCntrl);
         pXCntrl = IFX_NULL;

         return IFX_SUCCESS;
      }
   }
   else
   {
      return IFX_SUCCESS;
   }

   return IFX_ERROR;
}


/**
   Allocate device control structure for a given device number.
   - Check the device control struct, and allocate if not exists.
   - If not exist allocate and init the device r/w lock (semaphore).

\param
   nLineNum:   device line number - identify the given device line.

\return
   SUCCESS: pointer to the allocated device struct.
   ERROR:   IFX_NULL - not enough memory or invalid dev number.
*/
MEI_DEV_T *MEI_DevLineStructAlloc( IFX_uint8_t nLineNum )
{
   int entity, rel_ch, if_num;
   MEIX_CNTRL_T        *pXCntrl      = NULL;
   MEI_DEV_T           *pMeiDev    = NULL;
   MEI_MEI_DEV_CNTRL_T *pMeiDevCntrl = IFX_NULL;

   /* calculate entity and relative channel number */
   entity = MEI_GET_ENTITY_FROM_DEVNUM(nLineNum);
   rel_ch = MEI_GET_REL_CH_FROM_DEVNUM(nLineNum);
   if_num = MEI_GET_IF_FROM_DEVNUM(nLineNum);

   if ( (pXCntrl = MEIX_Cntrl[entity]) == NULL)
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DRV[%02d]: ERROR Line Struct Allocate - "
             "missing MEIX[%d] entity struct" MEI_DRV_CRLF, nLineNum, entity));

      return IFX_NULL;
   }

   /* check MEI interface */
   if ( (pMeiDevCntrl = pXCntrl->pMeiDevCntrl[if_num]) == NULL)
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DRV[%02d]: ERROR Line Struct Allocate - "
             "missing MEIX[%d] MEI Dev struct" MEI_DRV_CRLF, nLineNum, entity));

      return IFX_NULL;
   }

   /*
      Allocate device structs - keep until cleanup.
   */
   if ( (pMeiDev = pXCntrl->MeiDevice[rel_ch]) == NULL)
   {
      PRN_DBG_USR( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL, nLineNum,
            ("MEI_DRV[%02d]: Line Struct Allocate - create MEIX[%d]/Line[%d] (size %d byte)" MEI_DRV_CRLF,
              nLineNum, entity, rel_ch, sizeof(MEI_DEV_T)));

      pMeiDev = (MEI_DEV_T*)MEI_DRVOS_Malloc(sizeof(MEI_DEV_T));
      if (!pMeiDev)
      {
         PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
               ("MEI_DRV[%02d: ERROR Line Struct Allocate - no memory." MEI_DRV_CRLF,
                 nLineNum));
         return IFX_NULL;
      }
      memset( (char*)pMeiDev, 0x00, sizeof(MEI_DEV_T) );

      /* set the MEI interface */
      pMeiDev->meiDrvCntrl.pMeiDevCntrl = pMeiDevCntrl;
      /* set the port within the selected MEI interface */
      pMeiDev->meiDrvCntrl.pMeiIfCntrl = &pMeiDevCntrl->meiIf[0];
      /* save the device number for new memory blocks */
      MEI_DRV_LINENUM_SET(pMeiDev, nLineNum);
      /* save the device number for downloading firmware*/
      pMeiDev->fwDl.line_num = nLineNum;

      if (! MEI_DEVICE_CFG_IS_PLATFORM(e_MEI_DEV_PLATFORM_CONFIG_VR9))
      {
         if (MEI_VR1x_PcieEntityInit(&pMeiDev->meiDrvCntrl) != IFX_SUCCESS)
         {
             PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
                ("MEI_DRV[%02d]: ERROR Line Struct Allocate - pcie driver failed"
               MEI_DRV_CRLF, nLineNum));

            goto ERR_MEI_DEV_LINE_STRUCT_ALLOC;
         }
      }

#if (MEI_SUPPORT_PCI_SLAVE_FW_DOWNLOAD == 1)
      /* Assume dev#0 to be a PCI master*/
      if (MEI_GET_ENTITY_FROM_DEVNUM(nLineNum))
      {
         pMeiDev->fwDl.bPciSlave = IFX_TRUE;

         /* Create PCI slave pool*/
         pMeiDev->fwDl.pPool = MEI_VR9_PciSlavePoolCreate(
                                 (IFX_uint8_t*)MEI_PCI_SLAVE_PCI_POOL_START_ADDRESS,
                                 (IFX_uint8_t*)MEI_PCI_SLAVE_DDR_POOL_START_ADDRESS,
                                 MEI_PCI_SLAVE_POOL_SIZE_BYTE);

         if (pMeiDev->fwDl.pPool == IFX_NULL)
         {
            PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
                  ("MEI_DRV[%02d]: ERROR Line Struct Allocate - create PCI slave pool!"
                  MEI_DRV_CRLF, nLineNum));

            goto ERR_MEI_DEV_LINE_STRUCT_ALLOC;
         }
      }
#endif /* #if (MEI_SUPPORT_PCI_SLAVE_FW_DOWNLOAD == 1)*/

#if (MEI_SUPPORT_TIME_TRACE == 1)
      pMeiDev->timeStat.waitSendMin_ms = 9999;
      pMeiDev->timeStat.waitAckMin_ms  = 9999;
#endif

      /* init active members for this device */
      if(MEI_DRVOS_SelectQueueInit(&pMeiDev->selNfcWakeupList) != IFX_SUCCESS)
      {
         PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
               ("MEI_DRV[%02d]: ERROR Line Struct Allocate - init driver lock" MEI_DRV_CRLF,
                 nLineNum));

         goto ERR_MEI_DEV_LINE_STRUCT_ALLOC;
      }

      if(MEI_DRVOS_EventInit(&pMeiDev->eventMailboxRecv) != IFX_SUCCESS)
      {
         PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
               ("MEI_DRV[%02d]: ERROR Line Struct Allocate - init driver lock" MEI_DRV_CRLF,
                 nLineNum));

         goto ERR_MEI_DEV_LINE_STRUCT_ALLOC;
      }

      if (MEI_DRVOS_SemaphoreInit(&pMeiDev->pDriverAccessLock) != IFX_SUCCESS)
      {
         PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
               ("MEI_DRV[%02d]: ERROR Line Struct Allocate - init driver lock" MEI_DRV_CRLF,
                 nLineNum));

         goto ERR_MEI_DEV_LINE_STRUCT_ALLOC;
      }

      if (MEI_DRVOS_SemaphoreInit(&pMeiDev->pCallbackLock) != IFX_SUCCESS)
      {
         PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
               ("MEI_DRV[%02d]: ERROR Line Struct Allocate - init driver lock" MEI_DRV_CRLF,
                 nLineNum));

         goto ERR_MEI_DEV_LINE_STRUCT_ALLOC;
      }

      /* Successful: set the new Line struct within the chip control structs */
      pXCntrl->MeiDevice[rel_ch] = pMeiDev;
   }      /* if ( (pMeiDev = pMeiXCntrl->MeiDevice[rel_ch]) == NULL) {...} */

   return pMeiDev;

ERR_MEI_DEV_LINE_STRUCT_ALLOC:

   MEI_DRVOS_EventDelete(&pMeiDev->eventMailboxRecv);
   MEI_DRVOS_Free(pMeiDev);
   pMeiDev = IFX_NULL;

   return IFX_NULL;
}

/**
   If exists free a device structure for a given device number.

\param
   nLineNum   number of the device for free.

\return
   IFX_SUCCESS:  devices struct was not in use and has been freed
   IFX_ERROR:    device struct cannot freed (busy)
*/
IFX_int32_t MEI_DevLineStructFree(IFX_uint8_t nLineNum)
{
   IFX_int8_t entity, relChannel;
   MEIX_CNTRL_T *pXCntrl   = NULL;
   MEI_DEV_T    *pMeiDev = NULL;

   /* calculate entity and entity_ch number */
   entity     = MEI_GET_ENTITY_FROM_DEVNUM(nLineNum);
   relChannel = MEI_GET_REL_CH_FROM_DEVNUM(nLineNum);

   /* check if chip control block exists */
   if ( (pXCntrl = MEIX_Cntrl[entity]) != NULL)
   {
      /* check if device structure exists */
      if ( (pMeiDev = pXCntrl->MeiDevice[relChannel]) != NULL)
      {
         if(pMeiDev->openCount != 0)
         {
            PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
                  ("MEI_DRV[%d]: ERROR free device line - busy (count %d)" MEI_DRV_CRLF,
                    MEI_DRV_LINENUM_GET(pMeiDev), pMeiDev->openCount));
         }

         /* remove form global struct */
         pXCntrl->MeiDevice[relChannel] = NULL;

         /* Free FW download resources*/
         MEI_DEV_FirmwareDownloadResourcesRelease(pMeiDev);

#if (MEI_SUPPORT_DSM == 1)
         /* Free ERB buf */
         MEI_VRX_DSM_ErbFree(pMeiDev);
#endif /* (MEI_SUPPORT_DSM == 1) */


#if (MEI_DRV_ATM_OAM_ENABLE == 1)
         MEI_AtmOamReleaseDevCntrl(pMeiDev);
#endif

#if (MEI_DRV_CLEAR_EOC_ENABLE == 1)
         MEI_CEOC_ReleaseDevCntrl(pMeiDev);
#endif

         /*
            clear active members for this device:
            - clear required: driver lock per line
            - no clear required: pMeiDev->selNfcWakeupList;
            - no clear required: pMeiDev->eventMailboxRecv;
         */
         MEI_DRVOS_SemaphoreDelete(&pMeiDev->pDriverAccessLock);

         MEI_DRVOS_SemaphoreDelete(&pMeiDev->pCallbackLock);

         MEI_DRVOS_Free(pMeiDev);
         pMeiDev = IFX_NULL;
      }
   }      /* if ( (pMeiXCntrl = MEIX_Cntrl[entity]) != NULL) {...} */

   return IFX_SUCCESS;
}


/**
   Allocate the dynamic control structs for the open instance.

\remark
   Therefore allocate all required data blocks at once at the beginning
   because under LINUX the alloc-function allocates at least a minimum of
   a memory page.

\param
   deviceNum:     device number - identify the given device.
\param
   ppDfeDynCntrl: [OUT] points to a (MEI_DYN_CNTRL_T *)pointer
                  Returns device dynamic structure (per open).

\return
   IFX_SUCCESS: allocation successful.
   IFX_ERROR:   not enough memory or invalid dev number.
*/
IFX_int32_t MEI_DynCntrlStructAlloc(
                                 IFX_uint8_t       nLineNum,
                                 MEI_DYN_CNTRL_T **ppMeiDynCntrl )
{
   IFX_uint32_t         entity, rel_ch, structSize = 0;
   IFX_uint8_t          *pAll       = IFX_NULL;
   MEIX_CNTRL_T       *pXCntrl    = IFX_NULL;
   MEI_DEV_T          *pMeiDev  = IFX_NULL;
   MEI_DYN_CMD_DATA_T *pDynCmd    = IFX_NULL;
   MEI_DYN_CNTRL_T    *pDynCntrl  = IFX_NULL;

   /* calculate entity and relative channel number */
   entity = MEI_GET_ENTITY_FROM_DEVNUM(nLineNum);
   rel_ch = MEI_GET_REL_CH_FROM_DEVNUM(nLineNum);

   if ( (pXCntrl = MEIX_Cntrl[entity]) == NULL)
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DRV[%02d]: ERROR Dyn Struct Allocate - missing MEIX[%d] entity struct" MEI_DRV_CRLF,
              nLineNum, entity));

      return IFX_ERROR;
   }

   if ( (pMeiDev = pXCntrl->MeiDevice[rel_ch]) == NULL)
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DRV[%02d]: ERROR Dyn Struct Allocate - missing device struct" MEI_DRV_CRLF,
              nLineNum));

      return IFX_ERROR;
   }

   structSize =
        /* MEI_DYN_CNTRL_T struct + alignement */
        ( (sizeof(MEI_DYN_CNTRL_T) + (sizeof(MEI_DYN_CNTRL_T) & 0x3)) )
        /* MEI_DYN_CMD_DATA_T struct + alignement */
      + ( (sizeof(MEI_DYN_CMD_DATA_T) + (sizeof(MEI_DYN_CMD_DATA_T) & 0x3)) )
        /* cmd message buffer + alignement */
      + (  sizeof(MEI_MEI_MAILBOX_T) + (sizeof(MEI_MEI_MAILBOX_T) & 0x3) )
        /* ack message buffer + alignement */
      + (  sizeof(MEI_MEI_MAILBOX_T) + (sizeof(MEI_MEI_MAILBOX_T) & 0x3) );

   /* allocate required block for standard handling of this instance */
   pAll = (IFX_uint8_t *)MEI_DRVOS_Malloc(structSize);
   if (!pAll)
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DRV[%02d]: ERROR Dyn Struct Allocate - no memory, size %d (curr user: %d)" MEI_DRV_CRLF,
              nLineNum, structSize, pMeiDev->openCount));

      return IFX_ERROR;
   }
   memset(pAll, 0x00, structSize);

   /* setup internal per instance pointer */
   pDynCntrl = (MEI_DYN_CNTRL_T*)pAll;

   pDynCmd   = (MEI_DYN_CMD_DATA_T *)(pAll + (sizeof(MEI_DYN_CNTRL_T) + (sizeof(MEI_DYN_CNTRL_T) & 0x3))) ;

   pDynCmd->cmdWrBuf.bufSize_byte = sizeof(MEI_MEI_MAILBOX_T);
   pDynCmd->cmdWrBuf.pBuffer =
      pAll + (   (sizeof(MEI_DYN_CNTRL_T)    + (sizeof(MEI_DYN_CNTRL_T) & 0x3))
               + (sizeof(MEI_DYN_CMD_DATA_T) + (sizeof(MEI_DYN_CMD_DATA_T) & 0x3)) );

   pDynCmd->cmdAckCntrl.recvDataBuf_s.bufSize_byte = sizeof(MEI_MEI_MAILBOX_T);
   pDynCmd->cmdAckCntrl.recvDataBuf_s.pBuffer =
      pAll + (   ( sizeof(MEI_DYN_CNTRL_T)    + (sizeof(MEI_DYN_CNTRL_T)    & 0x3) )
               + ( sizeof(MEI_DYN_CMD_DATA_T) + (sizeof(MEI_DYN_CMD_DATA_T) & 0x3) )
               + ( sizeof(MEI_MEI_MAILBOX_T)  + (sizeof(MEI_MEI_MAILBOX_T)  & 0x3) ) );

   /* Allocation and setup successful - set return data */
   pDynCntrl->pDfeX       = pXCntrl;
   pDynCntrl->pMeiDev   = pMeiDev;
   pDynCntrl->pInstDynCmd = pDynCmd;
   /* enabled receive NFC via ioctl(...; ENABLE_NFC, ...) */
   pDynCntrl->pInstDynNfc = NULL;

   /* init the open instance mutex (semaphore) */
   MEI_DRVOS_SemaphoreInit(&pDynCntrl->pInstanceRWlock);

   *ppMeiDynCntrl = pDynCntrl;

#if (MEI_SUPPORT_DEBUG_STREAMS == 1)
   MEI_DBG_STREAM_ControlReset(pDynCntrl);
#endif

   return IFX_SUCCESS;
}


/**
   Free dynamic allocated data structs of an open instance

\param
   ppDfeDynCntrl: [OUT] points to a (MEI_DYN_CNTRL_T *)pointer
                  Returns the device dynamic structure (per open).

\return
   IFX_SUCCESS: free successful.

*/
IFX_int32_t MEI_DynCntrlStructFree( MEI_DYN_CNTRL_T **ppMeiDynCntrl )
{
   MEI_DYN_CNTRL_T *pDynCntrl = IFX_NULL;

   if (*ppMeiDynCntrl)
   {
      pDynCntrl = *ppMeiDynCntrl;

#if (MEI_SUPPORT_DEBUG_STREAMS == 1)
         if (pDynCntrl->pInstDynDebugStream)
         {
            (void)MEI_DBG_STREAM_ControlRelease(pDynCntrl);
         }
         MEI_DRVOS_SemaphoreDelete(&pDynCntrl->dbgStreamLock);
#endif

      *ppMeiDynCntrl = NULL;

      pDynCntrl->pInstDynCmd = IFX_NULL;
      if (pDynCntrl->pInstDynNfc)
      {
         MEI_DRVOS_Free(pDynCntrl->pInstDynNfc);
         pDynCntrl->pInstDynNfc = IFX_NULL;
      }
      MEI_DRVOS_SemaphoreDelete(&pDynCntrl->pInstanceRWlock);

      MEI_DRVOS_Free(pDynCntrl);
      pDynCntrl = IFX_NULL;
   }

   return IFX_SUCCESS;
}


/**
   Allocate required structures for a line.

\param
   nLineNum    line number of the device

\return
   IFX_SUCCESS in case of success else
   negative return code.

*/
IFX_int32_t MEI_DevLineAlloc(
                                 IFX_int8_t        nLineNum)
{
   MEI_DEV_T         *pMeiDev = NULL;

   if (nLineNum > (MEI_DFE_CHAN_DEVICES -1))
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
             ("MEI_DRV[--]: ERROR device setup - invalid line num %d max %d (-1)" MEI_DRV_CRLF,
              nLineNum, MEI_DFE_CHAN_DEVICES));

      return -e_MEI_ERR_DEV_NOT_EXIST;
   }

   /* check/allocate device entity control block */
   if ( MEI_DevXCntrlStructAlloc( MEI_GET_ENTITY_FROM_DEVNUM(nLineNum) ) == IFX_NULL )
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
             ("MEI_DRV[%02d]: ERROR device line alloc - MEIX entity data" MEI_DRV_CRLF,
              nLineNum));

      return -e_MEI_ERR_NO_MEM;
   }

   /* check/allocate device control block */
   if ( (pMeiDev = MEI_DevLineStructAlloc(nLineNum)) == IFX_NULL )
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
             ("MEI_DRV[%02d]: ERROR device line alloc - device line struct" MEI_DRV_CRLF,
              nLineNum));

      return -e_MEI_ERR_NO_MEM;
   }

   return IFX_SUCCESS;
}


/**
   Allocate and init required structures for a open instance of a line.

\param
   nLineNum    line number of the device
\param
   ppMeiDynCntrl return pointer for the open instance struct.

\return
   IFX_SUCCESS in case of success else
   negative return code.

*/
IFX_int32_t MEI_InstanceLineAlloc(
                                 IFX_int8_t        nLineNum,
                                 MEI_DYN_CNTRL_T **ppMeiDynCntrl)
{
   MEI_DEV_T         *pMeiDev = NULL;
   MEI_DYN_CNTRL_T   *pMeiDynCntrl = NULL;

   /* allocate dynamic data */
   if( MEI_DynCntrlStructAlloc(nLineNum, &pMeiDynCntrl ) != IFX_SUCCESS )
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
             ("MEI_DRV[%02d]: ERROR instance line alloc - user dynamic data" MEI_DRV_CRLF,
              nLineNum));

      return -e_MEI_ERR_NO_MEM;
   }

   if ((pMeiDynCntrl == IFX_NULL) || (pMeiDynCntrl->pMeiDev == IFX_NULL))
   {
      return -e_MEI_ERR_NO_MEM;
   }

   pMeiDev = pMeiDynCntrl->pMeiDev;

   PRN_DBG_USR( MEI_DRV, MEI_DRV_PRN_LEVEL_LOW, MEI_DRV_LINENUM_GET(pMeiDev),
          ("MEI_DRV[%02d]: instance line alloc - (current user %d)" MEI_DRV_CRLF,
          MEI_DRV_LINENUM_GET(pMeiDev), pMeiDev->openCount));


   /* device allocated - do exclusive init */
   MEI_DRV_GET_UNIQUE_DRIVER_ACCESS(pMeiDev);

   if (pMeiDev->openCount < MEI_MAX_OPEN)
   {
      pMeiDev->openCount++;
      pMeiDynCntrl->openInstance = pMeiDev->openCount;
      MEI_DRV_RELEASE_UNIQUE_DRIVER_ACCESS(pMeiDev);

      PRN_DBG_USR( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL, MEI_DRV_LINENUM_GET(pMeiDev),
            ("MEI_DRV[%d]: device line alloc (instance %d)" MEI_DRV_CRLF,
             nLineNum, pMeiDev->openCount));
   }
   else
   {
      MEI_DRV_RELEASE_UNIQUE_DRIVER_ACCESS(pMeiDev);

      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DRV[%d]:  ERROR device line alloc - limit (%d) exeeded" MEI_DRV_CRLF,
              nLineNum, MEI_MAX_OPEN));

      /*
         Limit reached - free only the dynamic per open data blocks.
      */
      if (pMeiDynCntrl)
      {
         MEI_DynCntrlStructFree(&pMeiDynCntrl);
      }

      return e_MEI_ERR_DEV_NO_RESOURCE;
   }

   *ppMeiDynCntrl = pMeiDynCntrl;

   return IFX_SUCCESS;
}


/**
   Check the current driver state against the given ioctl command.
   - Info calls are also allowed when the driver is still not init.
   - the basic init call is only allowed if still not done.
   - all further calls requires a previous init.

\param
   pMeiDynCntrl - points to the dynamic control struct.
\param
   pMeiDev      - points to the driver device struct.
\param
   ioctlCmd       - ioctl command which has to be performed.

\return
   0 (IFX_SUCCESS) if success
   negative value in case of error
*/
IFX_int32_t MEI_CheckIoctlCmdInitState(
                                 MEI_DYN_CNTRL_T *pMeiDynCntrl,
                                 IFX_uint32_t      ioctlCmd )
{
   IFX_int32_t    retVal = IFX_SUCCESS;
   MEI_DEV_T    *pMeiDev;

   if (pMeiDynCntrl && pMeiDynCntrl->pMeiDev)
   {
      pMeiDev = pMeiDynCntrl->pMeiDev;
   }
   else
   {
      return -e_MEI_ERR_INVAL_ARG;
   }

   switch (ioctlCmd)
   {
      case FIO_MEI_DEBUGLEVEL:
      case FIO_MEI_VERSION_GET:
      case FIO_MEI_REQ_CONFIG:
      case FIO_MEI_DRV_INIT:
      case FIO_MEI_MBOX_NFC_ENABLE:
      case FIO_MEI_AUTO_MSG_CTRL_SET:
      case FIO_MEI_DRV_DEVINFO_GET:
         /* ioctl cmd is also valid within the not init state */
         break;

#if (MEI_SUPPORT_MEI_DEBUG == 1)
      case FIO_MEI_DRV_BUF_SHOW:
         break;
#endif

      case FIO_MEI_DEV_INIT:
         if (MEI_DRV_STATE_GET(pMeiDev) != e_MEI_DRV_STATE_NOT_INIT)
         {
            /* init already done --> block command */
            PRN_DBG_USR( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL, MEI_DRV_LINENUM_GET(pMeiDev),
                   ("MEI_DRV[%02d-%02d] WARNING ioctl - init already done (%d), invalid cmd !!!" MEI_DRV_CRLF,
                    MEI_DRV_LINENUM_GET(pMeiDev), pMeiDynCntrl->openInstance, e_MEI_ERR_ALREADY_DONE));

            retVal = -e_MEI_ERR_ALREADY_DONE;
         }
         break;
      default:
         if (MEI_DRV_STATE_GET(pMeiDev) == e_MEI_DRV_STATE_NOT_INIT)
         {
            /* device is still not initialized --> block commands */
            PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
                   ("MEI_DRV[%02d-%02d] Error ioctl - missing init, invalid cmd !!!" MEI_DRV_CRLF,
                    MEI_DRV_LINENUM_GET(pMeiDev), pMeiDynCntrl->openInstance));

            retVal = -e_MEI_ERR_INVAL_STATE;
         }
   }

   return retVal;
}


/**
   Check the current driver state against the given ioctl send command.
   - Depending on the state only specific message send calls are allowded.

\param
   pMeiDynCntrl - points to the dynamic control struct.
\param
   pMeiDev      - points to the driver device struct.
\param
   ioctlCmd       - ioctl command which has to be performed.

\return
   0 (IFX_SUCCESS) if success
   negative value in case of error
*/
IFX_int32_t MEI_CheckIoctlCmdSendState(
                                 MEI_DYN_CNTRL_T *pMeiDynCntrl,
                                 IFX_uint32_t      ioctlCmd )
{
   IFX_int32_t    retVal = IFX_SUCCESS;
   MEI_DEV_T    *pMeiDev;

   if (pMeiDynCntrl && pMeiDynCntrl->pMeiDev)
   {
      pMeiDev = pMeiDynCntrl->pMeiDev;
   }
   else
   {
      return -e_MEI_ERR_INVAL_ARG;
   }

   switch (ioctlCmd)
   {
      case FIO_MEI_MBOX_MSG_WR:
      case FIO_MEI_MBOX_ACK_RD:
      case FIO_MEI_MBOX_MSG_SEND:
      case FIO_MEI_MBOX_NFC_RD:
      case FIO_MEI_MBOX_MSG_RAW_WR:
      case FIO_MEI_MBOX_ACK_RAW_RD:
      case FIO_MEI_MBOX_MSG_RAW_SEND:
      case FIO_MEI_MBOX_NFC_RAW_RD:
#if (MEI_SUPPORT_VDSL2_ADSL_SWAP == 1)
      case FIO_MEI_FW_MODE_SELECT:
#endif
         if (MEI_DRV_STATE_GET(pMeiDev) == e_MEI_DRV_STATE_SW_INIT_DONE)
         {
            /* Start device with the corresponding boot mode */
            if ( (retVal = MEI_StartupDevice(pMeiDev)) != IFX_SUCCESS)
            {
               PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
                      ("MEI_DRV[%02d-%02d]: ERROR - ioctl - Start device, no response" MEI_DRV_CRLF,
                      MEI_DRV_LINENUM_GET(pMeiDev), pMeiDynCntrl->openInstance));

               retVal = -e_MEI_ERR_DEV_NO_RESP;
            }
         }

         if ( (MEI_DRV_STATE_GET(pMeiDev) != e_MEI_DRV_STATE_WAIT_FOR_FIRST_RESP) &&
              (MEI_DRV_STATE_GET(pMeiDev) != e_MEI_DRV_STATE_DFE_READY) )
         {
            /* device is still not ready --> block commands */
            PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
                   ("MEI_DRV[%02d-%02d]: ERROR ioctl - inval state %d, not ready for send !!!" MEI_DRV_CRLF,
                    MEI_DRV_LINENUM_GET(pMeiDev), pMeiDynCntrl->openInstance, MEI_DRV_STATE_GET(pMeiDev)));

            retVal = -e_MEI_ERR_INVAL_STATE;
         }
         break;

      default:
         break;
   }

   return retVal;
}


/**
   Close MEI device - free all dynamic allocated memory blocks.

\param
   pMeiDynCntrl   private dynamic device data

\return
   IFX_SUCCESS - successful close MEI device
   IFX_ERROR   - error on   close MEI device

*/
IFX_int32_t MEI_DevLineClose(
                           MEI_DYN_CNTRL_T *pMeiDynCntrl)
{
   MEI_DEV_T          *pMeiDev = NULL;
   MEI_DYN_NFC_DATA_T *pDynNfc = NULL;
   IFX_uint8_t          nLineNum = 0xFF;


   if (pMeiDynCntrl == IFX_NULL)
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
             ("MEI_DRV: Fatal ERROR - private data lost for close" MEI_DRV_CRLF));
      return IFX_ERROR;
   }

   pMeiDev = pMeiDynCntrl->pMeiDev;

   /* =======================================
      admin: cleanup temp dependencies
      ======================================= */
   if (pMeiDev != NULL)
   {
      nLineNum = MEI_DRV_LINENUM_GET(pMeiDev);

      PRN_DBG_USR( MEI_DRV, MEI_DRV_PRN_LEVEL_LOW, nLineNum,
            ("MEI_DRV[%02d-%02d]: close" MEI_DRV_CRLF,
            nLineNum, pMeiDynCntrl->openInstance));

      /*
         - dec open count
         - cleanup the dynamic data structures
      */
      pMeiDev->openCount-- ;
      if (MEI_DRV_STATE_GET(pMeiDev) != e_MEI_DRV_STATE_NOT_INIT)
      {
         if ( (MEI_DRV_DYN_MBBUF_STATE_GET(pMeiDynCntrl->pInstDynCmd) == e_MEI_MB_BUF_ACK_PENDING)||
              (pMeiDynCntrl->pInstDynNfc != NULL) )
         {
            /*
               protect configuration change against ISR action:
               - reception of an outstanding ACK msg
               - reception of a NFC msg
            */
            MEI_DRV_GET_UNIQUE_MAILBOX_ACCESS(pMeiDev);

            /* check for pending ACK */
            if ( (MEI_DRV_DYN_MBBUF_STATE_GET(pMeiDynCntrl->pInstDynCmd) == e_MEI_MB_BUF_ACK_PENDING) ||
                 (pMeiDev->pCurrDynCmd == pMeiDynCntrl->pInstDynCmd) )
            {
               /* clear the pending ACK */
               if (pMeiDev->pCurrDynCmd == pMeiDynCntrl->pInstDynCmd)
               {
                  pMeiDev->pCurrDynCmd = NULL;
               }
               else
               {
                  /* Error: inconsistent configuration */
                  PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_WRN,
                        ("MEI_DRV[%02d - %02d]: WARNING close - no valid wr state" MEI_DRV_CRLF,
                          nLineNum, pMeiDynCntrl->openInstance));
               }

               MEI_DRV_DYN_MBBUF_STATE_SET(pMeiDynCntrl->pInstDynCmd, e_MEI_MB_BUF_FREE);

               PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
                     ("MEI_DRV[%02d - %02d]: WARNING close - pending ACK" MEI_DRV_CRLF,
                       nLineNum, pMeiDynCntrl->openInstance));
            }

            /* remove the NFC receive block from the device list */
            if (pMeiDynCntrl->pInstDynNfc != NULL)
            {
               MEI_RemoveNfcFromDevList(pMeiDynCntrl, &pDynNfc);
            }

            /* enable int again */
            MEI_DRV_RELEASE_UNIQUE_MAILBOX_ACCESS(pMeiDev);
         }        /* if ( .. || .. || ..) { interrupt disable } */

      }        /* if (MEI_DRV_STATE_GET(pMeiDev) != e_MEI_DRV_STATE_NOT_INIT) {... } */
   }
   else     /* if (pMeiDev != NULL) {...} else ... */
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DRV: FATAL ERROR - device number lost for close" MEI_DRV_CRLF));
   }        /* if (pMeiDev != NULL) {...} else {...} */


   /* =======================================
      admin done - now free dynamic resources
      ======================================= */
   MEI_DynCntrlStructFree( &pMeiDynCntrl );

   return IFX_SUCCESS;
}


/**
   Set the MEI CPE Driver debug level.
\param
   pMeiDynCntrl    points to the dynamic control struct.
\param
   pArgDbgLevel      points to the debug level struct.

\return
   0 (IFX_SUCCESS) if success
   negative value in case of error
*/
IFX_int32_t MEI_IoctlDebugLevelSet(
                              MEI_DYN_CNTRL_T       *pMeiDynCntrl,
                              IOCTL_MEI_dbgLevel_t  *pArgDbgLevel)
{
   if ( (pArgDbgLevel->valLevel   >= MEI_DRV_PRN_LEVEL_LOW) &&
        (pArgDbgLevel->valLevel   <= MEI_DRV_PRN_LEVEL_OFF) &&
        (pArgDbgLevel->eDbgModule >= e_MEI_DBGMOD_MEI_DRV)  &&
        (pArgDbgLevel->eDbgModule <  e_MEI_DBGMOD_LAST) )
   {
      switch(pArgDbgLevel->eDbgModule)
      {
         case e_MEI_DBGMOD_MEI_DRV:
            MEI_DRV_PRN_USR_LEVEL_SET(MEI_DRV, pArgDbgLevel->valLevel);
            MEI_DRV_PRN_INT_LEVEL_SET(MEI_DRV, pArgDbgLevel->valLevel);
            break;

         case e_MEI_DBGMOD_MEI_MSG_DUMP_API:
            MEI_DRV_PRN_USR_LEVEL_SET(MEI_MSG_DUMP_API, pArgDbgLevel->valLevel);
            MEI_DRV_PRN_INT_LEVEL_SET(MEI_MSG_DUMP_API, pArgDbgLevel->valLevel);
            break;

         case e_MEI_DBGMOD_MEI_NOTIFICATIONS:
            MEI_DRV_PRN_USR_LEVEL_SET(MEI_NOTIFICATIONS, pArgDbgLevel->valLevel);
            MEI_DRV_PRN_INT_LEVEL_SET(MEI_NOTIFICATIONS, pArgDbgLevel->valLevel);
            break;

         default:
            return -e_MEI_ERR_INVAL_ARG;
      }
   }
   else
   {
      return -e_MEI_ERR_INVAL_ARG;
   }

   return IFX_SUCCESS;
}


/**
   Return the MEI CPE driver configuration settings.

\param
   pMeiDynCntrl    points to the dynamic control struct.
\param
   pArgDevCfg_out    points to the return struct.

\return
   NONE.

\remarks
   The online chip parameters (chip id, bootmode, online mailbox params)
   are only returned if the modem is in online mode (modem ready).

*/
IFX_void_t MEI_IoctlRequestConfig(
                              MEI_DYN_CNTRL_T    *pMeiDynCntrl,
                              IOCTL_MEI_reqCfg_t *pArgDevCfg_out)
{
   MEI_DEV_T *pMeiDev = pMeiDynCntrl->pMeiDev;

   /* return driver settings */
   pArgDevCfg_out->devNum       = (unsigned int)MEI_DRV_LINENUM_GET(pMeiDev);
   pArgDevCfg_out->currOpenInst = (unsigned int)pMeiDynCntrl->openInstance;
   pArgDevCfg_out->phyBaseAddr  = (unsigned int)MEI_DRV_MEI_PHY_ADDR_GET(pMeiDev);
   pArgDevCfg_out->virtBaseAddr = (unsigned int)MEI_DRV_MEI_VIRT_ADDR_GET(pMeiDev);
   if (! MEI_DEVICE_CFG_IS_PLATFORM(e_MEI_DEV_PLATFORM_CONFIG_VR9))
   {
      pArgDevCfg_out->phyPDBRAMaddr  = (unsigned int)MEI_DRV_PDBRAM_PHY_ADDR_GET(pMeiDev);
      pArgDevCfg_out->virtPDBRAMaddr = (unsigned int)MEI_DRV_PDBRAM_VIRT_ADDR_GET(pMeiDev);
   }
   pArgDevCfg_out->usedIRQ      = (unsigned int)pMeiDynCntrl->pDfeX->IRQ_Num;
   pArgDevCfg_out->drvArc2MeMbSize = (unsigned int)sizeof(MEI_MEI_MAILBOX_T);
   pArgDevCfg_out->drvMe2ArcMbSize = (unsigned int)sizeof(MEI_MEI_MAILBOX_T);

   pArgDevCfg_out->Arc2MeBootMbAddr = (unsigned int)MEI_MAILBOX_BASE_ARC2ME;
   pArgDevCfg_out->Arc2MeBootMbSize = (unsigned int)MEI_BOOT_MAILBOX_ARC2ME_LEN;
   pArgDevCfg_out->Me2ArcBootMbAddr = (unsigned int)MEI_MAILBOX_BASE_ME2ARC;
   pArgDevCfg_out->Me2ArcBootMbSize = (unsigned int)MEI_BOOT_MAILBOX_ME2ARC_LEN;

   pArgDevCfg_out->currDrvState = MEI_DRV_STATE_GET(pMeiDev);

   if ( (pArgDevCfg_out->currDrvState != e_MEI_DRV_STATE_NOT_INIT) )
   {
      pArgDevCfg_out->chipId    = (unsigned int)pMeiDev->modemData.chipId;
      pArgDevCfg_out->bootMode  = (unsigned int)pMeiDev->modemData.devBootMode;
   }
   else
   {
      pArgDevCfg_out->chipId    = 0xFFFFFFFF;
      pArgDevCfg_out->bootMode  = 0xFFFFFFFF;
   }

   if ( (pArgDevCfg_out->currDrvState == e_MEI_DRV_STATE_DFE_READY) )
   {
      pArgDevCfg_out->Arc2MeOnlineMbAddr = (unsigned int)pMeiDev->modemData.mBoxDescr.addrArc2Me;
      pArgDevCfg_out->Arc2MeOnlineMbSize = (unsigned int)pMeiDev->modemData.mBoxDescr.lenArc2Me;
      pArgDevCfg_out->Me2ArcOnlineMbAddr = (unsigned int)pMeiDev->modemData.mBoxDescr.addrMe2Arc;
      pArgDevCfg_out->Me2ArcOnlineMbSize = (unsigned int)pMeiDev->modemData.mBoxDescr.lenMe2Arc;

      pArgDevCfg_out->currModemFsmState = MEI_DRV_MODEM_STATE_GET(pMeiDev);
   }
   else
   {
      pArgDevCfg_out->Arc2MeOnlineMbAddr = 0xFFFFFFFF;
      pArgDevCfg_out->Arc2MeOnlineMbSize = 0xFFFFFFFF;
      pArgDevCfg_out->Me2ArcOnlineMbAddr = 0xFFFFFFFF;
      pArgDevCfg_out->Me2ArcOnlineMbSize = 0xFFFFFFFF;

      pArgDevCfg_out->currModemFsmState  = 0xFFFFFFFF;
   }

   pArgDevCfg_out->ictl.retCode = IFX_SUCCESS;

   return;
}

#if (MEI_SUPPORT_STATISTICS == 1)
/**
   Return the MEI CPE statistic settings.

\param
   pMeiDynCntrl points to the dynamic control struct.
\param
   outDevStat     points to the return struct.

\return
   NONE.
*/
IFX_void_t MEI_IoctlRequestStat(
                              MEI_DYN_CNTRL_T       *pMeiDynCntrl,
                              IOCTL_MEI_statistic_t *pArgDevStat_out )
{
   /* ToDo if required: block interrupt while get statistics ? */
   memcpy( pArgDevStat_out,
           &pMeiDynCntrl->pMeiDev->statistics,
           sizeof(IOCTL_MEI_statistic_t) );

   pArgDevStat_out->ictl.retCode = IFX_SUCCESS;
   return;
}
#endif

#if 1
/**
   Check the MEI register for availability (Power Up/Down).
   - in case of Power Down all lines go to high.

\param
   pMeiDev  points to the device data

\return

\remarks
   Try to read the HW version register to check for power up/down.

\attention
   - also called on int-level
*/
IFX_int32_t MEI_MeiRegisterDetect(MEI_DEV_T *pMeiDev)
{
   if ( pMeiDev && MEI_DRV_MEI_VIRT_ADDR_GET(pMeiDev) )
   {
      if (MEI_InterfaceDetect(&pMeiDev->meiDrvCntrl) == IFX_SUCCESS)
      {
         MEI_DRV_MEI_IF_STATE_SET(pMeiDev, e_MEI_MEI_HW_STATE_UP);
         return IFX_SUCCESS;
      }
   }

   if (pMeiDev)
   {
      MEI_DRV_MEI_IF_STATE_SET(pMeiDev, e_MEI_MEI_HW_STATE_DOWN);
   }

   return -e_MEI_ERR_DEV_DOWN;
}


/**
   Reset the MEI CPE device driver structure
   - Clear all pending messages (ACK)
   - (Wakeup waiting users)
   - Release FW / Download / CodeSwap
   - Reset Mailbox descritor

\param
   pMeiDev  points to the device data
\param
   rstSrc   Reset reason
            0 Software / user triggered.
            1 GP1 FW failure, HW Failure.

\remarks
   - Interrupt must be already blocked (masked).
   - Driver state must be in reset state.

\attention
   - also called on int-level
*/
IFX_int32_t MEI_ResetDrvStruct(
                              MEI_DEV_T *pMeiDev,
                              IFX_int32_t rstSrc)
{
   MEI_DYN_NFC_DATA_T *pDynNfc = NULL;

   /*
      Clear a pending ACK message
   */
   if (MEI_DRV_MAILBOX_STATE_GET(pMeiDev) != e_MEI_MB_FREE)
   {
      if ( pMeiDev->pCurrDynCmd != NULL)
      {
         MEI_DRV_DYN_MBBUF_STATE_SET(pMeiDev->pCurrDynCmd, e_MEI_MB_BUF_RESET_DFE);
      }
   }

   MEI_DRV_MAILBOX_STATE_SET(pMeiDev, e_MEI_MB_FREE);
   pMeiDev->pCurrDynCmd = NULL;

   if (pMeiDev->bAckNeedWakeUp)
   {
      PRN_DBG_INT( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL, MEI_DRV_LINENUM_GET(pMeiDev),
           ("MEI_DRV[%02d]: ResetDfeDevStruct - wakeup" MEI_DRV_CRLF,
           MEI_DRV_LINENUM_GET(pMeiDev)));

      pMeiDev->bAckNeedWakeUp = IFX_FALSE;
      MEI_DRVOS_EventWakeUp(&pMeiDev->eventMailboxRecv);
   }

   /*
      Reset NFC user struct
      - Reset pending NFC events
      - Wakeup or let them wait for timeout ?
   */
   pDynNfc = pMeiDev->pRootNfcRecvFirst;
   while(pDynNfc)
   {
      IFX_uint8_t idx;

      for (idx = 0; idx < pDynNfc->numOfBuf; idx++)
      {
         pDynNfc->pRecvDataCntrl[idx].msgLen = 0;
         pDynNfc->pRecvDataCntrl[idx].bufCtrl = MEI_RECV_BUF_CTRL_FREE;
      }

      pDynNfc->rdIdxRd = pDynNfc->rdIdxWr;
      pDynNfc          = pDynNfc->pNext;
   }

   /*
      Clear FW download data
   */
#if (MEI_SUPPORT_DL_DMA_CS == 1)
   if (rstSrc /* ==  HW fault or User forced */)
   {
      MEI_DlDataClear(pMeiDev);
   }
#endif

   /*
      Reset the Mailbox description infos
   */
   pMeiDev->modemData.mBoxDescr.addrArc2Me = (IFX_uint32_t)NULL;
   pMeiDev->modemData.mBoxDescr.lenArc2Me  = 0;
   pMeiDev->modemData.mBoxDescr.addrMe2Arc = (IFX_uint32_t)NULL;
   pMeiDev->modemData.mBoxDescr.lenMe2Arc  = 0;

#if (MEI_DRV_ATM_OAM_ENABLE == 1)
   MEI_ATMOAM_ResetControl(pMeiDev);
#endif

#if (MEI_DRV_CLEAR_EOC_ENABLE == 1)
   MEI_CEOC_ResetControl(pMeiDev);
#endif

   return IFX_SUCCESS;
}

#endif

/**
   Init the corresponding device
   - reset the device, clear and mask interrupts interrupts.
   - get the device infos via the MEI interface

\param
   *pMeiDev  private device data
\param
   resetMode   identify if the device reset mode.

\return
   IFX_TRUE     Success
   IFX_FALSE    in case of error
*/
IFX_uint32_t MEI_DefaultInitDevice(
                           MEI_DEV_T *pMeiDev,
                           IFX_uint32_t resetMode)
{

   MEI_DisableDeviceInt(pMeiDev);

   if (MEI_GetChipInfo(pMeiDev) != IFX_SUCCESS)
   {
      return -e_MEI_ERR_DEV_DOWN;
   }

   if (MEI_LowLevelInit(&(pMeiDev->meiDrvCntrl)) != IFX_SUCCESS)
   {
      return -e_MEI_ERR_DEV_DOWN;
   }

#if (MEI_SUPPORT_VDSL2_ADSL_SWAP == 1)
   if (MEI_fwModeSelect & 0x80000000)
   {

      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_WRN,
             ("MEI_DRV[%02d]: Set FW Mode - %s" MEI_DRV_CRLF,
              MEI_DRV_LINENUM_GET(pMeiDev), (MEI_fwModeSelect & 0x1) ? "ADSL" : "VDSL2" ));

      /* set the default FW mode (VDSL2 / ADLS) */
      if ( MEI_DevCfgFwModeSelect( pMeiDev,
                                     (MEI_DEV_CFG_FW_MODE_E)(MEI_fwModeSelect & 0x1))
            != IFX_SUCCESS )
      {
         return -e_MEI_ERR_OP_FAILED;
      }
   }
#endif

   return IFX_SUCCESS;
}


/**
   Start the corresponding device
   - release reset, unmask interrupts and wait for the first response.

\param
   *pMeiDev  private device data

\return
   IFX_SUCCESS Success
   IFX_ERROR   In case of error
               - wrong / not supported bootmode
               - no response from the device
\remarks
   The driver must be in e_MEI_DRV_STATE_SW_INIT_DONE state

*/
IFX_int32_t MEI_StartupDevice(
                                 MEI_DEV_T *pMeiDev)
{
   IFX_int32_t ret = IFX_ERROR;

   /*
      wait for modem alive
   */
   if ( (ret = MEI_WaitForFirstResponce(pMeiDev)) != IFX_SUCCESS)
   {
      /* ERROR start the device --> bring it back to reset mode */
      MEI_DisableDeviceInt(pMeiDev);

      /* restore the init driver state */
      MEI_DRV_STATE_SET(pMeiDev, e_MEI_DRV_STATE_SW_INIT_DONE);
   }

   return ret;
}

/**
   Helper function - counts/checks/update the marked devices of the given mask.

\param
   pVrxDevMask      - device mask - all devices for update are set to 1
\prame
   fctCheckDevState - callback function used for check an dedicated state of the
                      device.
                      If this parameter is NULL no check will be done and the
                      function will only count all marked devices.

\return
   Num of marked devices after the check and update.

*/
IFX_int32_t MEIX_DevMaskCheck(
                              IFX_uint32_t               *pVrxDevMask,
                              MEI_FCT_CHECK_DEV_STATE  fctCheckDevState)
{
   IFX_uint8_t devCount=0, iMask, devFlag;
   IFX_uint8_t deviceNum, entityNum, relChannel;
   MEI_DEV_T *pMeiDev;

   for (iMask=0; iMask < 4; iMask++)
   {
       /** TODO: Check line below (will be true for imask 1..3 **/
      if ( (iMask*32) > MEI_DFE_CHAN_DEVICES)
         break;

      /* parse the given mask */
      if (pVrxDevMask[iMask] != 0x0)
      {
         for (devFlag=0; devFlag<32; devFlag++)
         {
            if ( ! (pVrxDevMask[iMask] & (0x00000001 << devFlag)) )
               continue;

            /* marked device found - check state and update mask */
            if (fctCheckDevState)
            {
               deviceNum = iMask*32 +devFlag;
               entityNum  = MEI_GET_ENTITY_FROM_DEVNUM(deviceNum);
               relChannel = MEI_GET_REL_CH_FROM_DEVNUM(deviceNum);

               if ( (entityNum < MEI_DFEX_ENTITIES) && (MEIX_Cntrl[entityNum] != NULL) &&
                    ((pMeiDev = MEIX_Cntrl[entityNum]->MeiDevice[relChannel]) != NULL) )
               {
                  /* device already established - check if state is valid */
                  if (fctCheckDevState(pMeiDev) == IFX_SUCCESS)
                  {
                     devCount++;
                  }
                  else
                  {
                     /* invalid state - clear flag */
                     pVrxDevMask[iMask] &= (~(0x00000001 << devFlag));
                  }
               }
               else
               {
                  /* device not exists - clear flag */
                  pVrxDevMask[iMask] &= (~(0x00000001 << devFlag));
               }
            }
            else
            {
               devCount++;
            }

            if (iMask*32 +devFlag >= MEI_DFE_CHAN_DEVICES)
               break;
         }
      }
   }        /* for (iMask=0; iMask < 4; iMask++) { step through the mask fields}  */

   return devCount;
}

/**
   Add a new MEI device to the current list of this IRQ.
   - Check if the current IRQ already in use --> add to the list.
   - If first use return the list root for IRQ request.

\param
   irqNum         IRQ number which will be used for this device.
\param
   pMeiXCntrl   points to the current device struct.

\return
   NULL     IRQ already in use.
   else     root of the list which use these IRQ.

*/
MEIX_CNTRL_T *MEI_VrxXDevToIrqListAdd(
                                 IFX_uint8_t    devNum,
                                 IFX_uint32_t   irqNum,
                                 MEIX_CNTRL_T *pMeiXCntrl)
{
   /* 0: new IRQ, 1: new device, 2: already chained */
   int found = 0;
   int entity, rel_VrxNum;
   MEIX_CNTRL_T *pTmpXCntrl, *plastDfeXCntrl = NULL;

   /* loop through all Entities to check if IRQ is already used */
   for (entity=0; entity<MEI_DFEX_ENTITIES; entity++)
   {
      if ( (pTmpXCntrl = MEIX_Cntrl[entity]) != NULL )
      {
         /* exist: check used IRQ num */
         if (pTmpXCntrl->IRQ_Num == irqNum)
         {
            if (pTmpXCntrl == pMeiXCntrl)
            {
               /* the IRQ is already in use for this Entity */
               found = 2;
               break;
            }
            else
            {
               /* the IRQ is already in use
                  but for a different Entity */
               found = 1;
               /* remember this entity and check the next */
               plastDfeXCntrl = pTmpXCntrl;
            }

            /* check the IRQ protection */
            if (pTmpXCntrl->IRQ_Protection <= 0)
            {
               /* error - this IRQ is already disabled */
               found = 3;
               break;
            }
         }
      }
   }        /* for (entity=0; entity<MEI_MAX_DFEX_ENTITIES; entity++) {...} */

   /* check if existing VRX found */
   switch(found)
   {
      case 0:
         /* new IRQ */
         pMeiXCntrl->IRQ_Num = irqNum;
         /* mark the first entry of the list */
         pMeiXCntrl->IRQ_Base = irqNum;
         pTmpXCntrl = pMeiXCntrl;
         PRN_DBG_USR( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL, devNum,
                ("MEI_DRV[%02d]: First assign MEIx dev to IRQ %d" MEI_DRV_CRLF,
                 devNum, irqNum));
         break;
      case 1:
         /* new VRX device to register for this IRQ
            --> add to end of the list */
         pMeiXCntrl->IRQ_Num = irqNum;
         pTmpXCntrl = plastDfeXCntrl;
         while(pTmpXCntrl)
         {
            plastDfeXCntrl = pTmpXCntrl;
            pTmpXCntrl = plastDfeXCntrl->pNextMeiXCntrl;

         }
         plastDfeXCntrl->pNextMeiXCntrl = pMeiXCntrl;
         /* return NULL to indicate IRQ was already in use */
         pTmpXCntrl = NULL;

         PRN_DBG_USR( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL, devNum,
                ("MEI_DRV[%02d]: Add MEIx dev to IRQ %d" MEI_DRV_CRLF,
                 devNum, irqNum));
         break;
      case 2:
         /* already in use */
         pTmpXCntrl = NULL;
         PRN_DBG_USR( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL, devNum,
                ("MEI_DRV[%02d]: MEIx dev to IRQ %d already assigned" MEI_DRV_CRLF,
                 devNum, irqNum));
         break;
      case 3:
         PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
                ("MEI_DRV[%02d]: ERROR while assign MEIx dev to IRQ - IRQ already blocked%d" MEI_DRV_CRLF,
                 devNum, irqNum));

         for (rel_VrxNum=0; rel_VrxNum < MEI_DFE_INSTANCE_PER_ENTITY ; rel_VrxNum++)
         {
            if ( pMeiXCntrl->MeiDevice[rel_VrxNum] )
            {
               MEI_DRV_STATE_SET(pMeiXCntrl->MeiDevice[rel_VrxNum], e_MEI_DRV_STATE_CFG_ERROR);
            }
         }
         pTmpXCntrl = NULL;
         break;
      default:
         pTmpXCntrl = NULL;
         PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
                ("MEI_DRV[%02d]: ERROR while assign MEIx dev to IRQ %d" MEI_DRV_CRLF,
                 devNum, irqNum));
         break;
   }        /* switch(found} {...} */

   return pTmpXCntrl;
}


/**
   Clear the given VRX device list.

\param
   pMeiXCntrl  points to the current VRX device struct.

\return
   NONE
*/
IFX_void_t MEI_VrxXDevFromIrqListClear(
                                 MEIX_CNTRL_T *pMeiXCntrl)
{
   MEIX_CNTRL_T *pTmpCurr, *pTmpNext;

   pTmpCurr = pMeiXCntrl;

   while(pTmpCurr)
   {
      if ( (pTmpCurr->IRQ_Num != 0) &&
           (pTmpCurr->IRQ_Num != 99) )
      {
         /* disable int for remove from list */
         pTmpCurr->IRQ_Num = 0;
      }

      pTmpNext = pTmpCurr->pNextMeiXCntrl;
      pTmpCurr->pNextMeiXCntrl = NULL;
      pTmpCurr = pTmpNext;
   }

   return;
}

/**
   Do polling for interrupts on all devices.

\return
   IFX_SUCCESS if all devices polled successful
*/
IFX_int32_t MEI_DevPollAllIrq (MEI_DEV_ACCESS_MODE_E eAccessMode)
{
   MEI_DEV_T *pMeiDev = NULL;
   IFX_uint8_t deviceNum, entity, rel_ch;

   for (deviceNum = 0; deviceNum < MEI_DFE_CHAN_DEVICES; deviceNum++)
   {
      /* calculate entity and relative channel number */
      entity = MEI_GET_ENTITY_FROM_DEVNUM (deviceNum);
      rel_ch = MEI_GET_REL_CH_FROM_DEVNUM (deviceNum);

      if ((MEIX_Cntrl[entity] != NULL) &&
          ((pMeiDev = (MEIX_Cntrl[entity])->MeiDevice[rel_ch]) != NULL))
      {
         if ( (MEI_DRV_STATE_GET(pMeiDev) != e_MEI_DRV_STATE_NOT_INIT) &&
              (MEI_DRV_STATE_GET(pMeiDev) != e_MEI_DRV_STATE_DFE_RESET) )
         {
            /* in polling mode: protect the read access */
            MEI_PollIntPerVrxLine(pMeiDev, eAccessMode);
         }
      }
   }

   return IFX_SUCCESS;
}

IFX_void_t MEI_HandleCallback(MEI_DEV_T *pMeiDev)
{
   if(pMeiDev != NULL && pMeiDev->bHandleCallback == IFX_TRUE)
   {
      pMeiDev->bHandleCallback = IFX_FALSE;

      switch(pMeiDev->callbackType)
      {
      case MEI_CALLBACK_TYPE_TC_LAYER_REQUEST:
         {
            if(pMeiDev->callbackData.tcLayerRequest.func != NULL)
            {
#ifdef IRQ_POLLING_FORCE
               PRN_DBG_USR_NL( MEI_NOTIFICATIONS, MEI_DRV_PRN_LEVEL_MSG,
                  ("MEI_DRV[%02d]: Handling TcRequest for IRQ[%d]"
                  MEI_DRV_CRLF, MEI_DRV_LINENUM_GET(pMeiDev), IRQ_POLLING_FORCE));

               kthread_run(
                  pMeiDev->callbackData.tcLayerRequest.func,
                  (void *)(&pMeiDev->callbackData.tcLayerRequest),
                  "tcReqCall");
#else

               PRN_DBG_USR_NL( MEI_NOTIFICATIONS, MEI_DRV_PRN_LEVEL_MSG,
                  ("MEI_DRV[%02d]: Handling TcRequest for normal IRQ"
                  MEI_DRV_CRLF, MEI_DRV_LINENUM_GET(pMeiDev)));
               pMeiDev->callbackData.tcLayerRequest.func(
                  (void *)&pMeiDev->callbackData.tcLayerRequest);
#endif /* IRQ_POLLING_FORCE */
            }
            else
            {
               PRN_DBG_USR_NL( MEI_NOTIFICATIONS, MEI_DRV_PRN_LEVEL_ERR,
                  ("MEI_DRV[%02d]: No callback function for TcRequest!"
                  MEI_DRV_CRLF, MEI_DRV_LINENUM_GET(pMeiDev)));
            }
            break;
         }
      case MEI_CALLBACK_TYPE_NONE:
      default:
         {
            PRN_DBG_USR_NL( MEI_NOTIFICATIONS, MEI_DRV_PRN_LEVEL_ERR,
               ("MEI_DRV[%02d]: None callback type selected!"
               MEI_DRV_CRLF, MEI_DRV_LINENUM_GET(pMeiDev)));
            break;
         }
      }
   }
}

/**
   Handles interrupts on VRX device level in polling mode.
   Therefore this function lock the driver struct before the interrupt
   routine is called.

\param
   pMeiDev     private VRX device data.

\return
   Number of processed interrupts.
*/
IFX_int32_t MEI_PollIntPerVrxLine(
                           MEI_DEV_T             *pMeiDev,
                           MEI_DEV_ACCESS_MODE_E eAccessMode)
{
   IFX_int32_t meiIntCnt = 0;

   if (pMeiDev->eModePoll == eAccessMode)
   {
      if (pMeiDev->eModePoll == e_MEI_DEV_ACCESS_MODE_ACTIV_POLL)
      {
         /* Exclude SMP platforms race contiditions caused driver access lock */
         /* \TODO: investigate handling */
         /* MEI_DRV_GET_UNIQUE_DRIVER_ACCESS(pMeiDev); */
         MEI_DRV_GET_UNIQUE_DEVICE_ACCESS(pMeiDev);

         meiIntCnt = MEI_ProcessIntPerVrxLine(pMeiDev);

         MEI_HandleCallback(pMeiDev);

         /* Exclude SMP platforms race contiditions caused driver access lock */
         /* \TODO: investigate handling */
         /* MEI_DRV_RELEASE_UNIQUE_DRIVER_ACCESS(pMeiDev); */
         MEI_DRV_RELEASE_UNIQUE_DEVICE_ACCESS(pMeiDev);
      }
      else
      {
         MEI_DRV_GET_UNIQUE_DEVICE_ACCESS(pMeiDev);

         meiIntCnt = MEI_ProcessIntPerVrxLine(pMeiDev);

         MEI_HandleCallback(pMeiDev);

         MEI_DRV_RELEASE_UNIQUE_DEVICE_ACCESS(pMeiDev);
      }
   }

   return meiIntCnt;
}

/**
   Handles interrupts on VRX device level.

\param
   pMeiDev     private VRX device data.

\return
   Number of processed interrupts
*/
IFX_int32_t MEI_ProcessIntPerVrxLine(
                                 MEI_DEV_T *pMeiDev)
{
   IFX_int32_t  processCnt = 0, gpIntCnt = 0;
   MEI_MeiRegVal_t processInt, intMask, tmpInt;
#if (MEI_DBG_DSM_PROFILING == 1 && MEI_SUPPORT_DEVICE_VR11 != 1)
   IFX_uint32_t *pMeiErrVecSize = (IFX_uint32_t *)pMeiDev->meiERBbuf.pERB_virt;
#endif

   IFX_uint8_t intPollLoop = 5;
   do
   {
      if (MEI_DRV_MEI_IF_STATE_GET(pMeiDev) == e_MEI_MEI_HW_STATE_UP)
      {
         /* get current interrupt status register */
         processInt = ( (MEI_RegAccOffGet(&pMeiDev->meiDrvCntrl, MEI_REG_OFF_ME_ARC2ME_STAT))
                        & ME_ARC2ME_INTERRUPT_MASK_ALL);
         intMask    = ( (MEI_RegAccOffGet(&pMeiDev->meiDrvCntrl, MEI_REG_OFF_ME_ARC2ME_MASK))
                        & ME_ARC2ME_INTERRUPT_MASK_ALL);
         tmpInt     = processInt;
      }
      else
      {
         /* MEI Failure - no valid settings, not interrupts processed */
         return 0;
      }

      if (pMeiDev->eModePoll == e_MEI_DEV_ACCESS_MODE_IRQ)
      {
         /* device not in poll mode --> handle only not masked interrupts */
         processInt &= intMask;
      }

      /* Process General Purpose interrupts*/
      if ((gpIntCnt = MEI_GPIntProcess(processInt, pMeiDev)) < 0)
      {
            /* HW failure
               --> cancel the other interrupts
               --> interrupts are blocked
               --> driver is in reset state
            */
            processInt = 0;
      }
      else
      {
         processCnt += gpIntCnt;
      }

      /* check for message available */
      if ( processInt & ME_ARC2ME_STAT_ARC_MSGAV_GET(pMeiDev))
      {
         /*
            read message header from mail box
            - distribute
            - signal mailbox free
         */
         processCnt++;
         MEI_IF_STAT_INC_MSGAV_INT_COUNT(pMeiDev);

#if (MEI_DBG_DSM_PROFILING == 1)
         /* [TD, 2012-11-19] Reset ERB data word (32 bit) - index: 8, offset: 0x20
            For FW debugging: Directly after an IRQ has been detected. */
         if ((pMeiErrVecSize != NULL) && (pMeiDev->meiERBbuf.nERBsize_byte > 0) &&
             (pMeiDev->bErbReset == IFX_TRUE))
         {
            *(pMeiErrVecSize + 8) = 0x0;
         }
#endif
         PRN_DBG_INT( MEI_DRV, MEI_DRV_PRN_LEVEL_LOW, MEI_DRV_LINENUM_GET(pMeiDev),
              ("MEI_DRV[%02d]: MEI_ProcessIntPerVrxLine - MSGAV (0x%08X)" MEI_DRV_CRLF,
              MEI_DRV_LINENUM_GET(pMeiDev), processInt));
         MEI_ReadMailbox(pMeiDev);
      }


      intPollLoop --;
   } while(intPollLoop);


#if 0
   /* check for Debug access done (currently only via polling) */
   if (processInt & ME_ARC2ME_STAT_DBG_DONE)
   {
      /*
         - signal/wakeup event for DBG done
      */
      PRN_DBG_INT( MEI_DRV, MEI_DRV_PRN_LEVEL_LOW, MEI_DRV_LINENUM_GET(pMeiDev),
           ("MEI_DRV[%02d]: MEI_ProcessIntPerVrxLine - DBG_DONE" MEI_DRV_CRLF,
           MEI_DRV_LINENUM_GET(pMeiDev)));

   }
#endif
   return processCnt;
}


/**
   Handles interrupts on VRX chip level.

\param
   pMeiXCntrl  private VRX chip data.

\return
   IFX_TRUE if data for upper layer is available
*/
MEI_STATIC IFX_uint32_t MEI_ProcessIntPerChip(MEIX_CNTRL_T *pMeiXCntrl)
{
   int rel_VrxNum;
   IFX_int32_t meiIntCnt = 0;

   for (rel_VrxNum=0; rel_VrxNum < MEI_DFE_INSTANCE_PER_ENTITY ; rel_VrxNum++)
   {
      if ( pMeiXCntrl->MeiDevice[rel_VrxNum] &&
           MEI_DRV_STATE_GET(pMeiXCntrl->MeiDevice[rel_VrxNum]) != e_MEI_DRV_STATE_NOT_INIT)
      {
         if (((MEI_DEV_T *)(pMeiXCntrl->MeiDevice[rel_VrxNum]))->eModePoll !=
                                                     e_MEI_DEV_ACCESS_MODE_IRQ)
         {
            PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
                ("MEI_DRV[%02d]: Unexpected interrupt within polling mode!"
                 MEI_DRV_CRLF, MEI_DRV_LINENUM_GET(pMeiXCntrl->MeiDevice[rel_VrxNum])));
         }

         meiIntCnt += MEI_ProcessIntPerVrxLine(pMeiXCntrl->MeiDevice[rel_VrxNum]);
      }
   }

   if (meiIntCnt)
   {
      /* at least one interrupt has been processed for this VRX device */
      pMeiXCntrl->meiIntCnt++;
   }

   return meiIntCnt;
}


/**
   Handles interrupts on IRQ level (more MEIx chips can be assigned).

\param
   T_MEIX_DEV *pMeiXCntrlList  list of private MEIx device data

\return
   Number of processed interrupts for this device
*/
IFX_int32_t MEI_ProcessIntPerIrq(MEIX_CNTRL_T *pMeiXCntrlList)
{
   IFX_int32_t meiIntCnt = 0;
   MEIX_CNTRL_T *pNextXCntrl = pMeiXCntrlList;
#if (MEI_SUPPORT_DEBUG_STREAMS == 1)
   IFX_int_t   extraDbgStreamLoop = 0;
#endif

   /* get the actual chip device from the list and step through the VRX devices */
   while(pNextXCntrl)
   {
#if (MEI_SUPPORT_DEBUG_STREAMS == 1)
      if (MEI_DBG_STREAMS_GLOBAL_CFG_DEV_STATE_GET(pNextXCntrl->entityNum) != 0)
      {
         extraDbgStreamLoop = 1;
      }
#endif
      meiIntCnt += MEI_ProcessIntPerChip(pNextXCntrl);

      pNextXCntrl = pNextXCntrl->pNextMeiXCntrl;
   }

#if (MEI_SUPPORT_DEBUG_STREAMS == 1)
   if (extraDbgStreamLoop == 1)
   {
      pNextXCntrl = pMeiXCntrlList;

      while(pNextXCntrl)
      {
         if (MEI_DBG_STREAMS_GLOBAL_CFG_DEV_STATE_GET(pNextXCntrl->entityNum) != 0)
         {
            meiIntCnt += MEI_DBG_STREAMS_CAST(IFX_int32_t, MEI_ProcessIntPerChip(pNextXCntrl));
         }

         pNextXCntrl = pNextXCntrl->pNextMeiXCntrl;
      }

   }
#endif

   return meiIntCnt;
}



/**
   This function disable all devices for this IRQ.
\remarks
   In case of misconfiguration interrupts can flood the system. The interrupt
   protection will disable the corresponding IRQ and all asigned devices will
   be blocked.

\param
   pMeiXCntrlList  list of devices for this IRQ.

\return
   Number of VRX VRX'S assigned to this IRQ.
*/
IFX_int32_t MEI_DisableDevsPerIrq(MEIX_CNTRL_T *pMeiXCntrlList)
{
   IFX_int32_t rel_VrxNum, meiDfeCnt = 0;
   MEIX_CNTRL_T *pNextXCntrl = pMeiXCntrlList;

   /* get the actual chip device from the list and step through the VRX devices */
   while(pNextXCntrl)
   {
      pNextXCntrl->IRQ_Protection = -1;

      for (rel_VrxNum=0; rel_VrxNum < MEI_DFE_INSTANCE_PER_ENTITY ; rel_VrxNum++)
      {
         if ( pNextXCntrl->MeiDevice[rel_VrxNum] )
         {
            MEI_DRV_STATE_SET(pNextXCntrl->MeiDevice[rel_VrxNum], e_MEI_DRV_STATE_CFG_ERROR);
            meiDfeCnt ++;
         }
      }
      pNextXCntrl = pNextXCntrl->pNextMeiXCntrl;
   }

   return meiDfeCnt;
}


/**
   Activate/Deactivate VRX Reset

\param
   pDev  private device data
\param
   rstMode  reset action
            - e_MEI_RESET
            - e_MEI_RESET_ACTIVATE
            - e_MEI_RESET_DEACTIVATE
\param
   rstSelMask  VRX device blocks to reset
\param
   rstSrc   Reset reason
            0 Software / user triggered.
            1 GP1 FW failure, HW Failure.


\return
   IFX_SUCCESS    now the driver is in reset state
   IFX_ERROR      HW Failure has been detected, driver is in reset state.
*/
IFX_int32_t MEI_DrvAndDevReset(
                             MEI_DEV_T              *pMeiDev
                           , IOCTL_MEI_resetMode_e  rstMode
                           , IFX_uint32_t           rstSelMask
                           , IFX_int32_t            rstSrc)
{
   IFX_int32_t ret = IFX_SUCCESS;

   /* Check for the device supported reset mode*/
   if (!MEI_IS_RESET_MODE_SUPPORTED(rstMode))
   {
      return -e_MEI_ERR_OP_FAILED;
   }

   MEI_DRV_GET_UNIQUE_ACCESS(pMeiDev);

   if ( (rstMode == e_MEI_RESET) || (rstMode == e_MEI_RESET_ACTIVATE) )
   {
      ret = MEI_DrvAndDevResetAct(pMeiDev, rstSelMask, rstSrc);
   }

   if ( (ret == IFX_SUCCESS) &&
        ((rstMode == e_MEI_RESET) || (rstMode == e_MEI_RESET_DEACTIVATE)) )
   {
      ret = MEI_DrvAndDevResetDeAct(pMeiDev, rstSelMask);
   }

   if (MEI_DRV_MEI_IF_STATE_GET(pMeiDev) == e_MEI_MEI_HW_STATE_UP)
   {
      MEI_DRV_RELEASE_UNIQUE_ACCESS(pMeiDev);
   }
   else
   {
      MEI_DRV_RELEASE_UNIQUE_DEVICE_ACCESS(pMeiDev);
      /* keep the interrupt disabled */
      MEI_DRV_RELEASE_UNIQUE_DRIVER_ACCESS(pMeiDev);
   }

   return (ret == IFX_SUCCESS) ? IFX_SUCCESS : -e_MEI_ERR_OP_FAILED;
}


/**
   Release the MEI driver from reset state.

\param
   pDev        private device data
\param
   rstSelect   select also the VRX device internal blocks.

\return
   IFX_SUCCESS    now the driver is in init done state.
   IFX_ERROR      HW Failure has been detected, driver is in reset state.
*/
MEI_STATIC IFX_int32_t MEI_DrvAndDevResetDeAct(
                                       MEI_DEV_T *pMeiDev,
                                       IFX_uint32_t rstSelect)
{
   if( MEI_DRV_STATE_GET(pMeiDev) != e_MEI_DRV_STATE_DFE_RESET )
   {
      /* driver not in reset */
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
             ("MEI_DRV[%02d]: WARNING cannot release reset (curr state %d)" MEI_DRV_CRLF,
             MEI_DRV_LINENUM_GET(pMeiDev), MEI_DRV_STATE_GET(pMeiDev) ));

      return IFX_SUCCESS;
   }

   /* Check HW access */
   if (MEI_MeiRegisterDetect(pMeiDev) == IFX_SUCCESS)
   {
      PRN_DBG_USR( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL, MEI_DRV_LINENUM_GET(pMeiDev),
         ("MEI_DRV[%02d]: Release Reset" MEI_DRV_CRLF, MEI_DRV_LINENUM_GET(pMeiDev) ));

      MEI_ResetDfeBlocks(&pMeiDev->meiDrvCntrl, IFX_FALSE, rstSelect );

      MEI_DRV_STATE_SET(pMeiDev, e_MEI_DRV_STATE_SW_INIT_DONE);

      return IFX_SUCCESS;
   }

   PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
          ("MEI_DRV[%02d]: Release RESET - MEI IF failure !!!" MEI_DRV_CRLF,
          MEI_DRV_LINENUM_GET(pMeiDev)));


   return IFX_ERROR;
}

/**
   Reset MEI Device

\param
   pDev        private device data
\param
   rstSelect   select also the VRX device internal blocks.
\param
   rstSrc   Reset reason
            0 Software / user triggered.
            1 GP1 FW failure, HW Failure.

\return
   IFX_SUCCESS    now the driver is in reset state
   IFX_ERROR      HW Failure has been detected, driver is in reset state.
*/
MEI_STATIC IFX_int32_t MEI_DrvAndDevResetAct(
                                 MEI_DEV_T *pMeiDev,
                                 IFX_uint32_t rstSelect,
                                 IFX_int32_t  rstSrc)
{
   IFX_int32_t nRet = IFX_ERROR;

   PRN_DBG_USR( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL, MEI_DRV_LINENUM_GET(pMeiDev),
      ("MEI_DRV[%02d]: Reset MEI CPE Drv" MEI_DRV_CRLF, MEI_DRV_LINENUM_GET(pMeiDev) ));

   MEI_DRV_STATE_SET(pMeiDev, e_MEI_DRV_STATE_DFE_RESET);
   /* user reset done */
   pMeiDev->bUsrRst = IFX_TRUE;

   MEI_IF_STAT_INC_SWRST_COUNT(pMeiDev);

   /* Check HW access */
   nRet = MEI_MeiRegisterDetect(pMeiDev);
   if (nRet == IFX_SUCCESS)
   {
      PRN_DBG_USR( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL, MEI_DRV_LINENUM_GET(pMeiDev),
             ("MEI_DRV[%02d]: RESET - MEI IF up" MEI_DRV_CRLF,
             MEI_DRV_LINENUM_GET(pMeiDev)));

      MEI_ResetDfeBlocks(&pMeiDev->meiDrvCntrl, IFX_TRUE, rstSelect );

      if (rstSelect)
         MEI_IF_STAT_INC_HWRST_COUNT(pMeiDev);

   }
   else
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
             ("MEI_DRV[%02d]: RESET - MEI IF failure (%d) - trying to recover!!!"
             MEI_DRV_CRLF, MEI_DRV_LINENUM_GET(pMeiDev), nRet));

      /* Try to recover MEI interface*/
      MEI_InterfaceRecover(&(pMeiDev->meiDrvCntrl));
   }

   /* reset the current driver struct */
   MEI_ResetDrvStruct( pMeiDev, rstSrc);

   /* Stay in reset mode */
   return (MEI_DRV_MEI_IF_STATE_GET(pMeiDev) == e_MEI_MEI_HW_STATE_UP) ? IFX_SUCCESS : IFX_ERROR;
}



/**
   Setup of the VRX driver settings and return the current settings.

\param
   pMeiDynCntrl points to the dynamic control struct.
\param
   pDrvCfg        points to the ioctl driver init struct.

\return
   NONE.


\remarks
   The blockTimeout value is always set,
   for the rest "0" value means keep current value.
*/
IFX_void_t MEI_IoctlDrvInit(
                        MEI_DYN_CNTRL_T     *pMeiDynCntrl,
                        IOCTL_MEI_drvInit_t *pDrvCfg)
{
   /* set always the given value */
   MEI_BlockTimeout = (pDrvCfg->blockTimeout) ? 1 : 0;

   if (pDrvCfg->waitModemMsg_ms)
      MEI_MaxWaitDfeResponce_ms = (IFX_int32_t)pDrvCfg->waitModemMsg_ms;
   else
      pDrvCfg->waitModemMsg_ms = (unsigned int)MEI_MaxWaitDfeResponce_ms;

   if (pDrvCfg->waitFirstResp_ms)
      MEI_MaxWaitForModemReady_ms =
            (IFX_int32_t)(pDrvCfg->waitFirstResp_ms | MEI_CFG_DEF_WAIT_PROTECTION_FLAG);
   else
      pDrvCfg->waitFirstResp_ms =
            (unsigned int)(MEI_MaxWaitForModemReady_ms & ~MEI_CFG_DEF_WAIT_PROTECTION_FLAG);

   pDrvCfg->bmWaitForDl_ms    = (unsigned int)-1;
   pDrvCfg->bmWaitDlInit_ms   = (unsigned int)-1;
   pDrvCfg->bmWaitNextBlk_ms  = (unsigned int)-1;

   pDrvCfg->bmDatawidth       = (unsigned int)-1;
   pDrvCfg->bmWaitStates      = (unsigned int)-1;

   return;
}

/**
   Return the VRX driver version.

\param
   pMeiDynCntrl - points to the dynamic control struct.
\param
   pDrvVersion    - points to the ioctl driver version struct.
\param
   bInternCall    - indicates if the call is form the internal interface
                    (image and data already in kernel space)

\return
   0 (IFX_SUCCESS) if success
   negative value in case of error

*/
IFX_int32_t MEI_IoctlDrvVersionGet(
                              MEI_DYN_CNTRL_T        *pMeiDynCntrl,
                              IOCTL_MEI_drvVersion_t *pArgDrvVersion_out,
                              IFX_boolean_t            bInternCall)
{
   IFX_int32_t len = strlen(DRV_MEI_VER_STR);
   IFX_int8_t major, minor, step;

   MEI_VersionParse(DRV_MEI_VER_STR, &major, &minor, &step);

   /* set driver version (digit) */
   pArgDrvVersion_out->versionId =    (major << 16)
                                    | (minor << 8)
                                    | (step);

   /* set driver version string */
   if (pArgDrvVersion_out->pVersionStr && pArgDrvVersion_out->strSize)
   {
      len = (len > (IFX_int32_t)pArgDrvVersion_out->strSize) ?
                                 (IFX_int32_t)pArgDrvVersion_out->strSize : len;

      if (bInternCall)
      {
         strcpy(pArgDrvVersion_out->pVersionStr, DRV_MEI_VER_STR);
         pArgDrvVersion_out->strSize = len;
      }
      else
      {
         if ( (MEI_DRVOS_CpyToUser( pArgDrvVersion_out->pVersionStr, DRV_MEI_VER_STR, len))
              == IFX_NULL )
         {
            PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
                  ("MEI_DRV[%02d - %02d]: DrvVersionGet - CopyToUser(..,..,%d) failed!" MEI_DRV_CRLF,
                   MEI_DRV_DYN_LINENUM_GET(pMeiDynCntrl), pMeiDynCntrl->openInstance, len));

            pArgDrvVersion_out->strSize = 0;
            pArgDrvVersion_out->ictl.retCode = -e_MEI_ERR_RETURN_ARG;
            return -e_MEI_ERR_RETURN_ARG;
         }
         else
         {
            pArgDrvVersion_out->strSize = len;
         }
      }
   }

   return IFX_SUCCESS;
}


/**
   Return the VRX device information.

\param
   pMeiDynCntrl - points to the dynamic control struct.
\param
   pDrvDevinfo    - points to the ioctl device info struct.
\param
   bInternCall    - indicates if the call is form the internal interface
                    (image and data already in kernel space)

\return
   0 (IFX_SUCCESS) if success
   negative value in case of error

*/
IFX_int32_t MEI_IoctlDevinfoGet(
                              MEI_DYN_CNTRL_T        *pMeiDynCntrl,
                              IOCTL_MEI_devinfo_t  *pArgDevinfo_out)
{
   pArgDevinfo_out->maxDeviceNumber = MEI_DFEX_ENTITIES;
   pArgDevinfo_out->linesPerDevice = MEI_DFE_INSTANCE_PER_ENTITY;
   pArgDevinfo_out->channelsPerLine = MEI_DEVICE_CFG_VALUE_GET(ChannelsPerLine);
   return IFX_SUCCESS;
}

#if (MEI_SUPPORT_VDSL2_ADSL_SWAP == 1)
/**
   Swap the FW mode and restart the firmware (swap).

\param
   pMeiDynCntrl points to the dynamic control struct.
\param
   fwMode         selects the mode (VDSL2 or ADSL)

\return
   0 (IFX_SUCCESS) if success
   negative value in case of error

*/
IFX_int32_t MEI_IoctlDevCfgFwModeSwap(
                              MEI_DYN_CNTRL_T    *pMeiDynCntrl,
                              IOCTL_MEI_fwMode_t *pArgFwMode)
{
   MEI_DEV_T *pMeiDev = pMeiDynCntrl->pMeiDev;

#if 1
   MEI_DRV_GET_UNIQUE_DRIVER_ACCESS(pMeiDev);

   /* set driver state "wait for first response" */
   MEI_DRV_STATE_SET(pMeiDev, e_MEI_DRV_STATE_WAIT_FOR_FIRST_RESP);

   MEI_DRV_RELEASE_UNIQUE_DRIVER_ACCESS(pMeiDev);
#endif

   /* swap image */
   if (MEI_DevCfgFwModeSwap( pMeiDynCntrl,
                               (MEI_DEV_CFG_FW_MODE_E)pArgFwMode->fwMode) != IFX_SUCCESS)
   {
      return -e_MEI_ERR_OP_FAILED;
   }

   /* wait for first response */
   if ( MEI_WaitForFirstResponce(pMeiDev) != IFX_SUCCESS)
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
             ("MEI_DRV[%02d]: SWAP FW failure - no response!!!" MEI_DRV_CRLF,
             MEI_DRV_LINENUM_GET(pMeiDev)));
   }

   return IFX_SUCCESS;
}
#endif


/**
   This function checks for configured action required for this message.

\param
   pMeiDynCntrl - private dynamic device data (per open instance)
\param
   pUserMsgs      - points to the user message information data
\param
   bInternCall    - indicates if the call is form the internal interface
                    (image and data already in kernel space)

\return
   0 if success
   negative value in case of error.

*/
IFX_int32_t MEI_MsgSendPreAction(
                                 MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                                 IOCTL_MEI_messageSend_t  *pUserMsgs,
                                 IFX_boolean_t              bInternCall)
{
   IFX_uint16_t param_16bit = 0;
   IFX_uint8_t *pPaylSrc   = IFX_NULL;

   pPaylSrc = &pUserMsgs->write_msg.pPayload[4];
   switch (pUserMsgs->write_msg.msgId)
   {
      case 0x0041:
         if (MEI_FsmStateSetMsgPreAction == 0)
         {
            /* No per-action set - nothing to do */
            return IFX_SUCCESS;
         }

         /* FSm State Set - check new state to set */
         if (!bInternCall)
         {
            memcpy(&param_16bit, pPaylSrc, sizeof(IFX_uint16_t));
         }
         else
         {
            MEI_DRVOS_CpyFromUser(&param_16bit, pPaylSrc, sizeof(IFX_uint16_t));
         }

         switch (param_16bit)
         {
            case 2:
#if (MEI_DRV_ATM_OAM_ENABLE == 1)
               if (MEI_FsmStateSetMsgPreAction & MEI_FSM_STATE_SET_PRE_ACT_ATMOAM_ENABLE)
               {
                  if (MEI_AtmOamControlEnable(
                              pMeiDynCntrl, pMeiDynCntrl->pMeiDev) != IFX_SUCCESS)
                  {
                     PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_WRN,
                            ("MEI_DRV[%02d]: ERROR - FSM Set 2, ATM OAM pre-action failed!!!" MEI_DRV_CRLF,
                            MEI_DRV_LINENUM_GET(pMeiDynCntrl->pMeiDev)));
                  }
               }
#endif /* #if (MEI_DRV_ATM_OAM_ENABLE == 1)*/

#if (MEI_DRV_CLEAR_EOC_ENABLE == 1)
               if (MEI_FsmStateSetMsgPreAction & MEI_FSM_STATE_SET_PRE_ACT_CEOC_ENABLE)
               {
                  /* currently not implemented */
               }
#endif /* (MEI_DRV_CLEAR_EOC_ENABLE == 1)*/
               break;

            default:
               /* nothing to do for this state */
               break;
         }

         break;
      default:
         /* nothing to do for this msg */
         break;
   }

   return IFX_ERROR;
}

/**
   Clear (disable) the interrupt mask for the the given device.

\param
   pMeiDev: Points to the VRX device control struct.

\return
   none
*/
IFX_void_t MEI_DisableDeviceInt(MEI_DEV_T *pMeiDev)
{
   MEI_MaskInterrupts( &pMeiDev->meiDrvCntrl,
                       ME_ARC2ME_INTERRUPT_MASK_ALL);

   return;
}

/*
   Set (enable) the interrupt mask for the the given device.
   The used mask depends on:
   - Poll Mode (all interrupts disabled)
   - Driver Loop (message available interrupt disabled)

\param
   pMeiDev: Points to the VRX device control struct.

\return
   none
*/
IFX_void_t MEI_EnableDeviceInt(MEI_DEV_T *pMeiDev)
{
   PRN_DBG_USR( MEI_DRV, MEI_DRV_PRN_LEVEL_LOW, MEI_DRV_LINENUM_GET(pMeiDev),
         ("MEI_DRV[%02d]:interrupt enable (0x%04X)\n\r",
           MEI_DRV_LINENUM_GET(pMeiDev), pMeiDev->intMask) );

   MEI_UnMaskInterrupts( &pMeiDev->meiDrvCntrl,
                         pMeiDev->intMask);

   return;
}

IFX_void_t MEI_MsgIntSet(MEI_DEV_T *pMeiDev)
{

   pMeiDev->meiDrvCntrl.intMsgMask = MEI_GET_REL_CH_FROM_DEVNUM(pMeiDev->meiDrvCntrl.dslLineNum) == 0 ?
      ME_ARC2ME_MASK_ARC_MSGAV0_ENA :
      ME_ARC2ME_MASK_ARC_MSGAV1_ENA;

   return;
}

MEI_STATIC void MEI_VersionParse(IFX_uint8_t *pVersion, IFX_uint8_t *pMajor,
                                 IFX_uint8_t *pMinor, IFX_uint8_t *pStep)
{
   IFX_uint32_t major, minor, step;

   if (pMajor == IFX_NULL || pMinor == IFX_NULL || pStep == IFX_NULL)
   {
      return;
   }

   sscanf(pVersion, "%u.%u.%u", &major, &minor, &step);
   *pMajor = major;
   *pMinor = minor;
   *pStep = step;
}

#if (MEI_SUPPORT_PERIODIC_TASK == 1)

/* Periodic tast parameter */
MEI_DRVOS_ThreadCtrl_t MEI_DrvCntrlThreadParams;

/**
   Vrx driver internal control task for periodic handling.
*/
IFX_int32_t MEI_DrvCntrlThr(
                        MEI_DRVOS_ThreadParams_t *pCntrlThrParams)
{
   MEI_DRVOS_event_t eventCntrlThr;
   IFX_uint32_t count = 0;

   /* do basic setup */
   PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_HIGH,
         ("MEI_DRV: Cntrl Thr Enter - %s" MEI_DRV_CRLF,
          pCntrlThrParams->pName) );

   if(pCntrlThrParams->nArg1 != 0)
   {
      MEI_DRVOS_ThreadPriorityModify(pCntrlThrParams->nArg1);
   }

   MEI_DRVOS_EventInit(&eventCntrlThr);

   /* start endless loop */
   while(!pCntrlThrParams->bShutDown)
   {
      count++;

      /* wait first */
      MEI_DRVOS_EventWait_timeout(
                  &eventCntrlThr, 10 /* ms */);

      MEI_DevPollAllIrq(e_MEI_DEV_ACCESS_MODE_ACTIV_POLL);

      if (MEI_DRVOS_SIGNAL_PENDING)
      {
         break;
      }

#if 0
      if ((count % 100) == 0)
      {
         printk(".");
      }
#endif
   }

   MEI_DRVOS_EventDelete(&eventCntrlThr);

   PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_HIGH,
         ("MEI_DRV: Cntrl Thr Leave - %s" MEI_DRV_CRLF,
          pCntrlThrParams->pName) );

   return IFX_SUCCESS;
}


#endif   /* #if (MEI_SUPPORT_PERIODIC_TASK == 1) */

#if (MEI_SUPPORT_DSM == 1)
IFX_int32_t MEI_IoctlDsmStatisticGet(MEI_DYN_CNTRL_T *pMeiDynCntrl,
                             IOCTL_MEI_dsmStatistics_t *pDsmStatistic)
{
   MEI_DEV_T *pMeiDev = pMeiDynCntrl->pMeiDev;

   MEI_VRX_DSM_FwStatsUpdate(pMeiDynCntrl, &pMeiDev->meiDsmStatistic);

   pDsmStatistic->n_fw_dropped_size = pMeiDev->meiDsmStatistic.n_fw_dropped_size;

   if(MEI_DEVICE_CFG_IS_PLATFORM(e_MEI_DEV_PLATFORM_CONFIG_VR11))
   {
      pDsmStatistic->n_processed             = pMeiDev->meiDsmStatistic.n_processed;
      pDsmStatistic->n_mei_dropped_size      = 0;
      pDsmStatistic->n_mei_dropped_no_pp_cb  = 0;
      pDsmStatistic->n_pp_dropped            = 0;
      pDsmStatistic->n_fw_total              = pMeiDev->meiDsmStatistic.n_fw_total;
   }
   else
   {
      pDsmStatistic->n_processed             = pMeiDev->meiDsmStatistic.n_processed;
      pDsmStatistic->n_mei_dropped_size      = pMeiDev->meiDsmStatistic.n_mei_dropped_size;
      pDsmStatistic->n_mei_dropped_no_pp_cb  = pMeiDev->meiDsmStatistic.n_mei_dropped_no_pp_cb;
      pDsmStatistic->n_pp_dropped            = pMeiDev->meiDsmStatistic.n_pp_dropped;
      pDsmStatistic->n_fw_total              = 0;
   }

   return IFX_SUCCESS;
}

IFX_int32_t MEI_IoctlDsmConfigGet(MEI_DYN_CNTRL_T *pMeiDynCntrl,
                                 IOCTL_MEI_dsmConfig_t *pDsmConfig)
{
   MEI_DEV_T *pMeiDev = pMeiDynCntrl->pMeiDev;

   pDsmConfig->eVectorControl = pMeiDev->meiDsmConfig.eVectorControl;

   return IFX_SUCCESS;
}

IFX_int32_t MEI_IoctlDsmConfigSet(MEI_DYN_CNTRL_T *pMeiDynCntrl,
                            IOCTL_MEI_dsmConfig_t *pDsmConfig)
{
   IFX_int32_t ret = 0;
   MEI_DEV_T *pMeiDev = pMeiDynCntrl->pMeiDev;
   IOCTL_MEI_firmwareFeatures_t *pFwFeatures = &(pMeiDev->firmwareFeatures);

   /* Only proceed if a firmware was download before (otherwise following
      feature checks will not return meaningful values) */
   if (MEI_IF_STAT_INC_FWDL_COUNT_GET(pMeiDev) == 0)
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
         ("MEI_DRV: Please download a firmware first!"MEI_DRV_CRLF));
      return (-e_MEI_ERR_INVAL_STATE);
   }

   /* Check Fw application type */
   if ( !((pFwFeatures->eFirmwareXdslModes) &
          (e_MEI_FW_XDSLMODE_VDSL2 | e_MEI_FW_XDSLMODE_VDSL2_VECTOR)) )
   {
      /* Firmware does not include any VDSL functionality (ADSL only).
         Not intended usage case!
         Do not allow any vectoring enable configuration in this case. */
      if (pDsmConfig->eVectorControl > e_MEI_VECTOR_CTRL_OFF)
      {
         PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DRV: Firmware does not support G.Vector"MEI_DRV_CRLF));
         return (-e_MEI_ERR_NOT_SUPPORTED_BY_FIRMWARE);
      }
   }
   else if ( !((pFwFeatures->eFirmwareXdslModes) &
               (e_MEI_FW_XDSLMODE_VDSL2_VECTOR)) )
   {
      /* Firmware only supports G.Vector friendly (Annex O) operation.
         Do not allow to switch on full G.Vector (Annex N). */
      if (pDsmConfig->eVectorControl == e_MEI_VECTOR_CTRL_ON)
      {
         PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DRV: Firmware does not support full G.Vector (Annex N)"
             MEI_DRV_CRLF));
         return (-e_MEI_ERR_NOT_SUPPORTED_BY_FIRMWARE);
      }
   }

   /* save value at the ctx */
   pMeiDev->meiDsmConfig.eVectorControl = pDsmConfig->eVectorControl;

   if (pMeiDev->bDsmConfigInit == IFX_FALSE)
   {
      pMeiDev->bDsmConfigInit = IFX_TRUE;
   }

   /* Update Modem State */
   MEI_VRX_ModemStateUpdate(pMeiDynCntrl);

   /* pass fw settings at RESET state */
   if (MEI_DRV_MODEM_STATE_GET(pMeiDev) == 0)
   {
      /* Only for fw support vectoring */
      if (pMeiDev->nFwVectorSupport)
      {
         if ((ret = MEI_VRX_DSM_ControlSet(pMeiDynCntrl, &pMeiDev->meiDsmConfig)) < 0)
         {
            PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
               ("MEI_DRV[0x%02X]: fail to set DSM config!" MEI_DRV_CRLF,
               MEI_DRV_LINENUM_GET(pMeiDev)));
         }
      }
   }

   return ret;
}

IFX_int32_t MEI_IoctlDsmStatusGet(MEI_DYN_CNTRL_T *pMeiDynCntrl,
                            IOCTL_MEI_dsmStatus_t *pDsmStatus)
{
   IFX_int32_t ret = 0;
   MEI_DEV_T *pMeiDev = pMeiDynCntrl->pMeiDev;

   /* Update Modem State */
   MEI_VRX_ModemStateUpdate(pMeiDynCntrl);

   if ((MEI_DRV_MODEM_STATE_GET(pMeiDev) == 7) || (MEI_DRV_MODEM_STATE_GET(pMeiDev) == 8))
   {
      ret = MEI_VRX_DSM_StatusGet(pMeiDynCntrl, pDsmStatus);
   }
   else
   {
      PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_WRN,
         ("MEI_DRV: WARNING - Function is only available in SHOWTIME!" MEI_DRV_CRLF));

      pDsmStatus->eVectorStatus = e_MEI_VECTOR_STAT_OFF;
      pDsmStatus->eVectorFriendlyStatus = e_MEI_VECTOR_FRIENDLY_STAT_OFF;

      ret = -e_MEI_ERR_INCOMPLETE_RETURN_VALUES;
   }

   return ret;
}

IFX_int32_t MEI_IoctlMacConfigGet(MEI_DYN_CNTRL_T *pMeiDynCntrl,
                            IOCTL_MEI_MacConfig_t *pMacConfig)
{
   MEI_DEV_T *pMeiDev = pMeiDynCntrl->pMeiDev;

   memcpy(pMacConfig->nMacAddress, &pMeiDev->meiMacConfig.nMacAddress,
                                                       MEI_MAC_ADDRESS_OCTETS);
   return IFX_SUCCESS;
}

IFX_int32_t MEI_IoctlMacConfigSet(MEI_DYN_CNTRL_T *pMeiDynCntrl,
                            IOCTL_MEI_MacConfig_t *pMacConfig)
{
   IFX_int32_t ret = 0;
   MEI_DEV_T *pMeiDev = pMeiDynCntrl->pMeiDev;

   /* Only proceed if a firmware was download before (otherwise following
      feature checks will not return meaningful values) */
   if (MEI_IF_STAT_INC_FWDL_COUNT_GET(pMeiDev) == 0)
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
         ("MEI_DRV: Please download a firmware first!"MEI_DRV_CRLF));
      return (-e_MEI_ERR_INVAL_STATE);
   }

   /* save value at the ctx */
   memcpy(&pMeiDev->meiMacConfig.nMacAddress, pMacConfig->nMacAddress,
                                                       MEI_MAC_ADDRESS_OCTETS);
   /* Update Modem State */
   MEI_VRX_ModemStateUpdate(pMeiDynCntrl);

   /* pass fw settings at RESET state */
   if (MEI_DRV_MODEM_STATE_GET(pMeiDev) == 0)
   {
      /* Only for fw support vectoring */
      if (pMeiDev->nFwVectorSupport)
      {
         if ((ret = MEI_VRX_DSM_MacConfigSet(pMeiDynCntrl, &pMeiDev->meiMacConfig)) < 0)
         {
            PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
               ("MEI_DRV[0x%02X]: fail to set MAC config!" MEI_DRV_CRLF,
               MEI_DRV_LINENUM_GET(pMeiDev)));
         }
      }
   }

   return ret;
}
#endif /* (MEI_SUPPORT_DSM == 1) */

IFX_int32_t MEI_IoctlPllOffsetConfigSet(
                                 MEI_DYN_CNTRL_T             *pMeiDynCntrl,
                                 IOCTL_MEI_pllOffsetConfig_t *pPllOffsetConfig)
{
   IFX_int32_t ret = 0;
   MEI_DEV_T *pMeiDev = pMeiDynCntrl->pMeiDev;

   /* Only proceed if a firmware was download before (otherwise following
      feature checks will not return meaningful values) */
   if (MEI_IF_STAT_INC_FWDL_COUNT_GET(pMeiDev) == 0)
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
         ("MEI_DRV: Please download a firmware first!"MEI_DRV_CRLF));
      return (-e_MEI_ERR_INVAL_STATE);
   }

   if (pPllOffsetConfig->nPllOffset == MEI_PLL_DISABLED)
   {
      PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_WRN,
         ("MEI_DRV: PLL offset disabled" MEI_DRV_CRLF));
   }

   else if ((pPllOffsetConfig->nPllOffset < MEI_PLL_CLOCKSET_MIN) ||
       (pPllOffsetConfig->nPllOffset > MEI_PLL_CLOCKSET_MAX))
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
         ("MEI_DRV: PLL offset value %d out of range [%d..%d]"
          MEI_DRV_CRLF, pPllOffsetConfig->nPllOffset,
          MEI_PLL_CLOCKSET_MIN, MEI_PLL_CLOCKSET_MAX));

      return (-e_MEI_ERR_INVAL_CONFIG);
   }

   /* save value at the ctx */
   pMeiDev->modemData.nPllOffset = pPllOffsetConfig->nPllOffset;

   /* Update Modem State */
   MEI_VRX_ModemStateUpdate(pMeiDynCntrl);

   /* pass fw settings at RESET state */
   if (MEI_DRV_MODEM_STATE_GET(pMeiDev) == 0)
   {
      ret = MEI_PLL_ConfigSet(pMeiDynCntrl);
   }

   return ret;
}

IFX_int32_t MEI_IoctlPllOffsetConfigGet(
                                 MEI_DYN_CNTRL_T             *pMeiDynCntrl,
                                 IOCTL_MEI_pllOffsetConfig_t *pPllOffsetConfig)
{
   MEI_DEV_T *pMeiDev = pMeiDynCntrl->pMeiDev;

   pPllOffsetConfig->nPllOffset = pMeiDev->modemData.nPllOffset;
   return IFX_SUCCESS;
}
