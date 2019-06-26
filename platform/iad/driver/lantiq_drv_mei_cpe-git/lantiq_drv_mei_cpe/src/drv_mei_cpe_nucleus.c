/******************************************************************************

                          Copyright (c) 2007-2015
                     Lantiq Beteiligungs-GmbH & Co. KG

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/* ============================================================================
   Description : VRX Driver, Nucleus part
   ========================================================================= */

#ifdef NUCLEUS_PLUS

/* ============================================================================
   Inlcudes
   ========================================================================= */

/* get at first the driver configuration */
#include "drv_mei_cpe_config.h"

#include "ifx_types.h"
#include "drv_mei_cpe_os.h"
#include "drv_mei_cpe_dbg.h"

#include "drv_mei_cpe_dbg_driver.h"

#include "drv_mei_cpe_nucleus.h"

#include "drv_mei_cpe_interface.h"
#include "drv_mei_cpe_mei_interface.h"
#include "drv_mei_cpe_api.h"


#include "drv_mei_cpe_msg_process.h"

#if (MEI_SUPPORT_DL_DMA_CS == 1)
#include "drv_mei_cpe_download.h"
#endif

#include "drv_mei_cpe_dbg_access.h"

#if (MEI_SUPPORT_VDSL2_ADSL_SWAP == 1)
#include "drv_mei_cpe_device_cntrl.h"
#endif

#ifdef MEI_GENERIC_PROC_FS
#include "drv_mei_cpe_generic_proc.h"
#endif

#include "ifxos_device_io.h" /* DEVIO_driver_install ... */


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
   Local variables (Nucleus)
   ========================================================================= */

/* VRX driver number */
MEI_STATIC IFX_int32_t VRXDrvNum = 0;
/* DFEX driver number */
MEI_STATIC IFX_int32_t MEIXDrvNum = 0;


/* ============================================================================
   I/O VRX driver functions (Nucleus) - declaration
   ========================================================================= */

MEI_STATIC IFX_int_t MEI_CpeDevOpen(
                              void              *priv,
                              const IFX_char_t  *pAnnex);

MEI_STATIC IFX_int_t MEI_Close(
                              void              *priv);

MEI_STATIC IFX_int_t MEI_Write(
                              void              *priv,
                              const IFX_char_t  *pSrc,
                              const IFX_int_t   nLength);

MEI_STATIC IFX_int_t MEI_Read(
                              void              *priv,
                              IFX_char_t        *pDst,
                              const IFX_int_t   nLength);

MEI_STATIC IFX_int_t MEI_IoCtl(
                              void              *priv,
                              IFX_uint32_t      nCmd,
                              IFX_uint32_t      nArgument);

MEI_STATIC int MEI_IfxIntConnect(
                              void    *pIntVector,
                              void    *pISRRoutine,
                              int     ISRParams);

MEI_STATIC void MEI_InterruptNucleus(
                              int ISRParams);


/* ============================================================================
   I/O MEI X (Control) driver functions (Nucleus) - declaration
   ========================================================================= */

MEI_STATIC IFX_int32_t MEI_DfeXDevOpen(
                                 void              *priv,
                                 const IFX_char_t  *pAnnex);

MEI_STATIC IFX_int32_t MEI_CloseDfeX(void *priv);

/* ============================================================================
   Local function (Nucleus) - declarations
   ========================================================================= */

MEI_STATIC IFX_int32_t MEI_DevAllocAll(void);

MEI_STATIC IFX_int32_t MEI_DevFreeAll(void);

MEI_STATIC IFX_int32_t MEI_DevAddAll(
                                 IFX_char_t *pDrvBaseName,
                                 IFX_int32_t drvNum,
                                 IFX_uint8_t *pDevNumLast);

MEI_STATIC IFX_void_t MEI_DevDeleteAll(IFX_uint8_t devNumLast);


/* ============================================================================
   ToDo: functions for the common part - declaration
   ========================================================================= */

IFX_int32_t MEI_DfeDevClose(
                     MEI_DYN_CNTRL_T *pMeiDynCntrl);



/* ============================================================================
   VRX driver interrupt functions (Nucleus) - wrapping
   ========================================================================= */
#if (MEI_SUPPORT_IRQ == 1)
MEI_STATIC int MEI_IntSetupLocked = 0;

/* function ptr to the used intConnect function */
MEI_STATIC MEI_IntConnect_WrapNUCLEUS_t MEI_IntConnect_WrapNUCLEUS = IFX_NULL;

/* function ptr to the used Interrupt Enable Routine */
MEI_STATIC MEI_IntEnable_WrapNUCLEUS_t MEI_IntEnable_WrapNUCLEUS = IFX_NULL;

/* function ptr to the used Interrupt Enable Routine */
MEI_STATIC MEI_IntDisable_WrapNUCLEUS_t MEI_IntDisable_WrapNUCLEUS = IFX_NULL;
#endif

/* ============================================================================
   I/O driver functions (Nucleus) - definition
   ========================================================================= */

/**
   Open the device.

\param
   pMeiDev private device data
\param
   pAnnex remaing part of string used during open()
\param
   flags additional flags

\return
   -1 - on failure,
   non zero value on success
*/
MEI_STATIC IFX_int_t MEI_CpeDevOpen(
                                 void *priv,
                                 const IFX_char_t *pAnnex)
{
   MEI_DEV_T          *pMeiDev = (MEI_DEV_T *)priv;
   MEI_DYN_CNTRL_T    *pMeiDynCntrl = NULL;
   IFX_int8_t           nLineNum;

   /* get device number from VRX struct */
   nLineNum = MEI_DRV_LINENUM_GET(pMeiDev);

   if (MEI_InstanceLineAlloc(nLineNum, &pMeiDynCntrl) != IFX_SUCCESS)
   {
      errno = e_MEI_ERR_DEV_NO_RESOURCE;
      return 0;
   }

   /*
      Return the dynamic control data block - assign to the new fd.
   */
   return (int)pMeiDynCntrl;
}


/**
   Close the device.

\param
   pMeiDev private device data

\return
   IFX_SUCCESS - on success.
   IFX_ERROR   - if privat dynamic data has be lost.
*/
MEI_STATIC IFX_int_t MEI_Close(void *priv)
{
   MEI_DYN_CNTRL_T *pMeiDynCntrl = (MEI_DYN_CNTRL_T *)priv;

   /* get the VRX device structure */
   if (pMeiDynCntrl == NULL)
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
             ("MEI_DRV: FATAL ERROR - private dynamic data lost for close" MEI_DRV_CRLF));

      return IFX_ERROR;
   }

   MEI_DfeDevClose(pMeiDynCntrl);


   return IFX_SUCCESS;
}


/**
   Read from device

\param
   pMeiDynCntrl - private dynamic control data (per open instance)
\param
   pDst destination buffer
\param
   nLength max. length to read

\return
   number of bytes returned in destination buffer or
   -1 - on failure
*/
MEI_STATIC IFX_int_t MEI_Read(
                              void              *priv,
                              IFX_char_t        *pDst,
                              const IFX_int_t   nLength)
{
   MEI_DYN_CNTRL_T *pMeiDynCntrl = (MEI_DYN_CNTRL_T *)priv;
   MEI_DEV_T          *pMeiDev;
   MEI_DYN_CMD_DATA_T *pDynCmd;

   /* get the VRX device structure */
   if (pMeiDynCntrl == NULL)
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_WRN,
             ("MEI_DRV: Error read - invalid dyn device struct" MEI_DRV_CRLF));

      errno = -e_MEI_ERR_INVAL_CMD;
      return IFX_ERROR;
   }

   pMeiDev = pMeiDynCntrl->pMeiDev;
   pDynCmd   = pMeiDynCntrl->pInstDynCmd;

   PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_WRN,
          ("MEI_DRV[0x%02X]: read(.., %d, ..) not supported" MEI_DRV_CRLF,
           MEI_DRV_LINENUM_GET(pMeiDev), nLength));

   errno = -e_MEI_ERR_INVAL_CMD;
   return IFX_ERROR;
}


/**
   Write to device

\param
   pMeiDynCntrl    private dynamic control data (per open instance)
\param
   pSrc source buffer
\param
   nLength length to write

\return
   number of bytes to write or
   -1 - on failure
*/
MEI_STATIC IFX_int_t MEI_Write(
                              void              *priv,
                              const IFX_char_t  *pSrc,
                              const IFX_int_t   nLength)
{
   /* MEI_DYN_CNTRL_T *pMeiDynCntrl = (MEI_DYN_CNTRL_T *)priv; */
   PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
          ("MEI_DRV: write(..., %d, ..) not supported" MEI_DRV_CRLF, nLength));

   errno = -e_MEI_ERR_INVAL_CMD;
   return IFX_ERROR;
}

/**
   Configuration / Control for the device.

\param
   pMeiDynCntrl - private dynamic control data (per open instance)
\param
   nCmd function id's
\param
   nArgument optional argument

\return
   0 and positive values - success,
   negative value - ioctl failed
*/
MEI_STATIC IFX_int32_t MEI_IoCtl(
                              void              *priv,
                              IFX_uint32_t       nCmd,
                              IFX_uint32_t       nArgument)
{
   MEI_DYN_CNTRL_T *pMeiDynCntrl = (MEI_DYN_CNTRL_T *)priv;
   int ret = 0;
   MEI_DEV_T       *pMeiDev;
   IOCTL_MEI_arg_t *pUserArgs;


   /* require an argument (ptr ioctl struct) */
   if (nArgument == 0)
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
             ("MEI_DRV[??] Error ioctl - invalid argument ptr" MEI_DRV_CRLF));
      errno = e_MEI_ERR_INVAL_ARG;
      return IFX_ERROR;
   }

   pUserArgs = (IOCTL_MEI_arg_t *)nArgument;

   /* get the VRX device structure */
   if (pMeiDynCntrl == NULL)
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
             ("MEI_DRV[??] Error ioctl - invalid dyn device struct" MEI_DRV_CRLF));

      switch (nCmd)
      {
         default:
            pUserArgs->drv_ioctl.retCode = -e_MEI_ERR_DEV_NOT_EXIST;
            break;
      }

      errno = e_MEI_ERR_DEV_NOT_EXIST;
      return IFX_ERROR;
   }

   pMeiDev = pMeiDynCntrl->pMeiDev;

   /*
      Check for valid commands if driver still not init.
   */
   if ( (ret = MEI_CheckIoctlCmdInitState(pMeiDynCntrl, (IFX_uint32_t)nCmd))
        != IFX_SUCCESS )
   {
      goto MEI_IOCTL_RETURN;
   }

   /* poll all devices in POLLING mode */
   MEI_DevPollAllIrq(e_MEI_DEV_ACCESS_MODE_PASSIV_POLL);

   /*
      check correct driver state for send modem message.
   */
   if ( (ret = MEI_CheckIoctlCmdSendState(pMeiDynCntrl, (IFX_uint32_t)nCmd))
        != IFX_SUCCESS )
   {
      goto MEI_IOCTL_RETURN;
   }

   switch (nCmd)
   {
      case FIO_MEI_DEBUGLEVEL:
         ret = MEI_IoctlDebugLevelSet(pMeiDynCntrl, &pUserArgs->dbg_level);
         break;

      case FIO_MEI_VERSION_GET:
         {
            ret = MEI_IoctlDrvVersionGet(pMeiDynCntrl, &pUserArgs->drv_vers, IFX_FALSE);

            #if (MEI_MISC_TEST == 1)
            MEI_MemVAllocTest();
            #endif
         }
         break;

      case FIO_MEI_DEV_INIT:
         ret = MEI_IoctlInitDevice(pMeiDynCntrl, &pUserArgs->init_dev);
         break;

      case FIO_MEI_DRV_INIT:
         MEI_IoctlDrvInit(pMeiDynCntrl, &pUserArgs->init_drv);
         break;

      case FIO_MEI_RESET:
         {
            ret = MEI_DrvAndDevReset(
                                   pMeiDev
                                 , pUserArgs->rst.rstMode
                                 , pUserArgs->rst.rstSelMask
                                 , 1);
         }
         break;

      case FIO_MEI_REQ_CONFIG:
         MEI_IoctlRequestConfig(pMeiDynCntrl, &pUserArgs->req_cfg);
         break;

#if (MEI_SUPPORT_VDSL2_ADSL_SWAP == 1)
      case FIO_MEI_FW_MODE_SELECT:
         ret = MEI_IoctlDevCfgFwModeSwap(
                                 pMeiDynCntrl, &pUserArgs->fw_mode);
         break;
#endif      /* #if (MEI_SUPPORT_VDSL2_ADSL_SWAP == 1) */

      case FIO_MEI_FW_MODE_CTRL_SET:
         ret = MEI_IoctlFwModeCtrlSet(
               pMeiDynCntrl, &pUserArgs->fw_mode_ctrl);
         break;

      case FIO_MEI_FW_MODE_STAT_GET:
         ret = MEI_IoctlFwModeStatGet(
               pMeiDynCntrl, &pUserArgs->fw_mode_stat);
         break;

#if (MEI_SUPPORT_STATISTICS == 1)
      case FIO_MEI_REQ_STAT:
         MEI_IoctlRequestStat(pMeiDynCntrl, &pUserArgs->req_stat);
         break;
#endif

#if (MEI_SUPPORT_REGISTER == 1)
      case FIO_MEI_REG_SET:
         if( MEI_Set_Register( pMeiDev,
                                 pUserArgs->reg_io.addr,
                                 pUserArgs->reg_io.value)
             != IFX_SUCCESS )
         {
            ret = -e_MEI_ERR_OP_FAILED;
         }
         break;

      case FIO_MEI_REG_GET:
         if( MEI_Get_Register( pMeiDev,
                                 pUserArgs->reg_io.addr,
                                 (IFX_uint32_t*)&pUserArgs->reg_io.value)
             != IFX_SUCCESS )
         {
            ret = -e_MEI_ERR_OP_FAILED;
         }
         break;
#endif      /* #if (MEI_SUPPORT_REGISTER == 1) */

#if (MEI_SUPPORT_DRV_LOOPS == 1)
      case FIO_MEI_DRV_LOOP:
         {
            MEI_MailboxLoop( pMeiDev,
                               (pUserArgs->drv_loop.loopEnDis) ? IFX_TRUE:IFX_FALSE);
         }
         break;
#endif

      case FIO_MEI_FW_DL:
         {
            PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
                   ("MEI_DRV: ioctl - FIO_MEI_FW_DL: size = %d bytes" MEI_DRV_CRLF,
                    (unsigned int)pUserArgs->fw_dl.size_byte));

            ret = MEI_IoctlFirmwareDownload(pMeiDynCntrl, &pUserArgs->fw_dl, IFX_FALSE);
         }
         break;

      case FIO_MEI_OPT_FW_DL:
         {
            PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
                   ("MEI_DRV: ioctl - FIO_MEI_OPT_FW_DL: size = %d bytes" MEI_DRV_CRLF,
                    (unsigned int)pUserArgs->fw_dl_opt.size_byte));

            ret = MEI_IoctlOptFirmwareDownload(pMeiDynCntrl, &pUserArgs->fw_dl_opt, IFX_FALSE);
         }
         break;

#if (MEI_SUPPORT_DFE_DMA_ACCESS == 1)
      case FIO_MEI_DMA_WRITE:
         {
            /* execute access */
            ret = MEI_IoctlDmaAccessWr( pMeiDynCntrl,
                                          &pUserArgs->dma_access);
         }
         break;

      case FIO_MEI_DMA_READ:
         {
            /* execute access */
            ret = MEI_IoctlDmaAccessRd( pMeiDynCntrl,
                                          &pUserArgs->dma_access);
         }
         break;
#endif

#if (MEI_SUPPORT_DFE_GPA_ACCESS == 1)
      case FIO_MEI_GPA_WRITE:
         {
            PRN_DBG_USR( MEI_DRV, MEI_DRV_PRN_LEVEL_LOW, MEI_DRV_LINENUM_GET(pMeiDev),
                   ("MEI_DRV[???]: ioctl - FIO_MEI_GPA_WRITE" MEI_DRV_CRLF));

            /* execute access */
            ret = MEI_GpaWrAccess( pMeiDynCntrl,
                                     pUserArgs->gpa_access.dest,
                                     pUserArgs->gpa_access.addr,
                                     pUserArgs->gpa_access.value);
         }
         break;

      case FIO_MEI_GPA_READ:
         {
            PRN_DBG_USR( MEI_DRV, MEI_DRV_PRN_LEVEL_LOW, MEI_DRV_LINENUM_GET(pMeiDev),
                   ("MEI_DRV[???]: ioctl - FIO_MEI_GPA_READ" MEI_DRV_CRLF));

            /* execute access */
            ret = MEI_GpaRdAccess( pMeiDynCntrl,
                                     pUserArgs->gpa_access.dest,
                                     pUserArgs->gpa_access.addr,
                                     (IFX_uint32_t *)&pUserArgs->gpa_access.value);
            if (ret != IFX_SUCCESS)
            {
               /* ERROR */
               PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
                      ("MEI_DRV[???]: ERROR ioctl - FIO_MEI_GPA_READ" MEI_DRV_CRLF));

               ret = -e_MEI_ERR_OP_FAILED;
            }
         }
         break;
#endif      /* #if (MEI_SUPPORT_DFE_GPA_ACCESS == 1) */

#if (MEI_SUPPORT_DFE_DBG_ACCESS == 1)
      case FIO_MEI_DBG_WRITE:
         {
            /* execute access */
            ret = MEI_IoctlMeiDbgAccessWr(pMeiDynCntrl, &pUserArgs->dbg_access);
         }
         break;

      case FIO_MEI_DBG_READ:
         {
            /* execute access */
            ret = MEI_IoctlMeiDbgAccessRd(pMeiDynCntrl, &pUserArgs->dbg_access);
         }
         break;

      case FIO_MEI_DBG_AUX_WRITE:
         {
            /* set AUX destination and do it */
            pUserArgs->dbg_access.dbgDest = MEI_IOCTL_DEBUG_AUX;
            ret = MEI_IoctlMeiDbgAccessWr(pMeiDynCntrl, &pUserArgs->dbg_access);
         }
         break;

      case FIO_MEI_DBG_AUX_READ:
         {
            /* set AUX destination and do it */
            pUserArgs->dbg_access.dbgDest = MEI_IOCTL_DEBUG_AUX;
            ret = MEI_IoctlMeiDbgAccessRd(pMeiDynCntrl, &pUserArgs->dbg_access);
         }
         break;

      case FIO_MEI_DBG_CORE_WRITE:
         {
            /* set CORE destination and do it */
            pUserArgs->dbg_access.dbgDest = MEI_IOCTL_DEBUG_CORE;
            ret = MEI_IoctlMeiDbgAccessWr( pMeiDynCntrl, &pUserArgs->dbg_access);
         }
         break;

      case FIO_MEI_DBG_CORE_READ:
         {
            /* set CORE destination and do it */
            pUserArgs->dbg_access.dbgDest = MEI_IOCTL_DEBUG_CORE;
            ret = MEI_IoctlMeiDbgAccessRd( pMeiDynCntrl, &pUserArgs->dbg_access);
         }
         break;

      case FIO_MEI_DBG_LS_WRITE:
         {
            /* set CORE destination and do it */
            pUserArgs->dbg_access.dbgDest = MEI_IOCTL_DEBUG_LDST;
            ret = MEI_IoctlMeiDbgAccessWr( pMeiDynCntrl, &pUserArgs->dbg_access);
         }
         break;
      case FIO_MEI_DBG_LS_READ:
         {
            /* set Load/Store destination and do it */
            pUserArgs->dbg_access.dbgDest = MEI_IOCTL_DEBUG_LDST;
            ret = MEI_IoctlMeiDbgAccessRd( pMeiDynCntrl, &pUserArgs->dbg_access);
         }
         break;
#endif      /* #if (MEI_SUPPORT_DFE_DBG_ACCESS == 1) */

      case FIO_MEI_MBOX_NFC_ENABLE:

         PRN_DBG_USR( MEI_DRV, MEI_DRV_PRN_LEVEL_LOW, MEI_DRV_LINENUM_GET(pMeiDev),
                ("MEI_DRV[%02d-%02d] ioctl - FIO_MEI_MBOX_NFC_ENABLE" MEI_DRV_CRLF,
                  MEI_DRV_LINENUM_GET(pMeiDev), pMeiDynCntrl->openInstance));

         /* Setup the receive NFC feature */
         if (pMeiDynCntrl->pInstDynNfc == IFX_NULL)
         {
            ret = MEI_IoctlNfcEnable(pMeiDynCntrl, 0, 0);
            pMeiDynCntrl->pInstDynNfc->msgProcessCtrl = MEI_MSG_CNTRL_MODEM_MSG_MASK_DEFAULT;
         }
         break;

      case FIO_MEI_MBOX_NFC_DISABLE:

         PRN_DBG_USR( MEI_DRV, MEI_DRV_PRN_LEVEL_LOW, MEI_DRV_LINENUM_GET(pMeiDev),
                ("MEI_DRV[%02d-%02d] ioctl - FIO_MEI_MBOX_NFC_DISABLE" MEI_DRV_CRLF,
                  MEI_DRV_LINENUM_GET(pMeiDev), pMeiDynCntrl->openInstance));

         /* Setup the receive NFC feature */
         ret = MEI_IoctlNfcDisable(pMeiDynCntrl);
         break;



      case FIO_MEI_AUTO_MSG_CTRL_SET:
         {
            PRN_DBG_USR( MEI_DRV, MEI_DRV_PRN_LEVEL_LOW, MEI_DRV_LINENUM_GET(pMeiDev),
                   ("MEI_DRV[???]: ioctl - FIO_MEI_AUTO_MSG_CTRL_SET" MEI_DRV_CRLF));

            /* Set auto message control */
            ret = MEI_IoctlAutoMsgCtlSet(pMeiDynCntrl, &pUserArgs->autoMsgCtrl);
         }
         break;

      case FIO_MEI_AUTO_MSG_CTRL_GET:
         {
            PRN_DBG_USR( MEI_DRV, MEI_DRV_PRN_LEVEL_LOW, MEI_DRV_LINENUM_GET(pMeiDev),
                   ("MEI_DRV[???]: ioctl - FIO_MEI_AUTO_MSG_CTRL_GET" MEI_DRV_CRLF));

            /* Get auto message control */
            ret = MEI_IoctlAutoMsgCtlGet(pMeiDynCntrl, &pUserArgs->autoMsgCtrl);
         }
         break;



      case FIO_MEI_MBOX_MSG_SEND:
         {
            PRN_DBG_USR( MEI_DRV, MEI_DRV_PRN_LEVEL_LOW, MEI_DRV_LINENUM_GET(pMeiDev),
                   ("MEI_DRV[???]: ioctl - FIO_MEI_MBOX_MSG_SEND" MEI_DRV_CRLF));

            /* send msg */
            ret = MEI_IoctlMsgSend(pMeiDynCntrl, &pUserArgs->ifx_msg_send, IFX_FALSE);
         }
         break;

      case FIO_MEI_MBOX_MSG_WR:
         {
            PRN_DBG_USR( MEI_DRV, MEI_DRV_PRN_LEVEL_LOW, MEI_DRV_LINENUM_GET(pMeiDev),
                   ("MEI_DRV[???]: ioctl - FIO_MEI_MBOX_MSG_WR" MEI_DRV_CRLF));

            MEI_DRVOS_SemaphoreLock(&pMeiDynCntrl->pInstanceRWlock);
            ret = MEI_IoctlCmdMsgWrite( pMeiDynCntrl,
                                          &pUserArgs->ifx_msg,
                                          IFX_FALSE);
            MEI_DRVOS_SemaphoreUnlock(&pMeiDynCntrl->pInstanceRWlock);
         }
         break;

      case FIO_MEI_MBOX_ACK_RD:
         {
            PRN_DBG_USR( MEI_DRV, MEI_DRV_PRN_LEVEL_LOW, MEI_DRV_LINENUM_GET(pMeiDev),
                   ("MEI_DRV[???]: ioctl - FIO_MEI_MBOX_ACK_RD" MEI_DRV_CRLF));

            MEI_DRVOS_SemaphoreLock(&pMeiDynCntrl->pInstanceRWlock);
            ret = MEI_IoctlAckMsgRead( pMeiDynCntrl,
                                         &pUserArgs->ifx_msg,
                                         IFX_FALSE);
            MEI_DRVOS_SemaphoreUnlock(&pMeiDynCntrl->pInstanceRWlock);
            if (ret < 0)
               pUserArgs->ifx_msg.paylSize_byte = 0;
         }
         break;

      case FIO_MEI_MBOX_NFC_RD:
         {
            /*
            PRN_DBG_USR( MEI_DRV, MEI_DRV_PRN_LEVEL_LOW, MEI_DRV_LINENUM_GET(pMeiDev),
                   ("MEI_DRV: ioctl - FIO_MEI_MBOX_NFC_RD" MEI_DRV_CRLF));
            */

            /* send msg */
            ret = MEI_IoctlNfcMsgRead( pMeiDynCntrl,
                                      &pUserArgs->ifx_msg, IFX_FALSE);

            /* set return value */
            if (ret < 0)
               pUserArgs->ifx_msg.paylSize_byte = 0;
         }
         break;

#if (MEI_SUPPORT_RAW_MSG == 1)
      case FIO_MEI_MBOX_MSG_RAW_SEND:
         {
            PRN_DBG_USR( MEI_DRV, MEI_DRV_PRN_LEVEL_LOW, MEI_DRV_LINENUM_GET(pMeiDev),
                   ("MEI_DRV[???]: ioctl - FIO_MEI_MBOX_MSG_RAW_SEND" MEI_DRV_CRLF));

            /* send msg */
            ret = MEI_IoctlRawMsgSend(pMeiDynCntrl, &pUserArgs->mbox_send);
         }
         break;

      case FIO_MEI_MBOX_MSG_RAW_WR:
         {
            PRN_DBG_USR( MEI_DRV, MEI_DRV_PRN_LEVEL_LOW, MEI_DRV_LINENUM_GET(pMeiDev),
                   ("MEI_DRV[???]: ioctl - FIO_MEI_MBOX_MSG_RAW_WR" MEI_DRV_CRLF));

            /* send msg - return number of written bytes */
            ret = MEI_IoctlRawMsgWrite( pMeiDynCntrl,
                                          pUserArgs->mbox_msg.pData_16,
                                          pUserArgs->mbox_msg.count_16bit);

            /* set return value */
            pUserArgs->mbox_msg.count_16bit = (ret < 0) ? 0 : ret >> 1;
         }
         break;

      case FIO_MEI_MBOX_ACK_RAW_RD:
         {
            PRN_DBG_USR( MEI_DRV, MEI_DRV_PRN_LEVEL_LOW, MEI_DRV_LINENUM_GET(pMeiDev),
                   ("MEI_DRV[???]: ioctl - FIO_MEI_MBOX_ACK_RAW_RD" MEI_DRV_CRLF));

            /* send msg */
            ret = MEI_IoctlRawAckRead( pMeiDynCntrl,
                                         pUserArgs->mbox_msg.pData_16,
                                         pUserArgs->mbox_msg.count_16bit);

            /* set return value */
            pUserArgs->mbox_msg.count_16bit = (ret < 0) ? 0 : ret >> 1;
         }
         break;

      case FIO_MEI_MBOX_NFC_RAW_RD:
         {
            /*
            PRN_DBG_USR( MEI_DRV, MEI_DRV_PRN_LEVEL_LOW, MEI_DRV_LINENUM_GET(pMeiDev),
                   ("MEI_DRV: ioctl - FIO_MEI_MBOX_NFC_RAW_RD" MEI_DRV_CRLF));
            */
            /* send msg */
            ret = MEI_IoctlRawNfcRead( pMeiDynCntrl,
                                         pUserArgs->mbox_msg.pData_16,
                                         pUserArgs->mbox_msg.count_16bit);

            /* set return value */
            pUserArgs->mbox_msg.count_16bit = (ret < 0) ? 0 : ret >> 1;
         }
         break;
#endif      /* #if (MEI_SUPPORT_RAW_MSG == 1) */

#if (MEI_SUPPORT_TEST_DEBUG == 1)
      case FIO_MEI_MEI_REGS_SHOW:
         MEI_MeiRegsShow(pMeiDev);
         break;

      case FIO_MEI_DRV_BUF_SHOW:
         {
            /* not further supported with driver version > 0.1.5.x */
            MEI_ShowDrvBuffer( pMeiDynCntrl,
                                 (IFX_int8_t)pUserArgs->show_drv_buf.bufNum,
                                 pUserArgs->show_drv_buf.count );
            ret = -e_MEI_ERR_INVAL_CMD;
         }
         break;

      case FIO_MEI_DMA_TEST:
         {
            PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL,
                   ("MEI_DRV: ioctl - FIO_MEI_DMA_TEST" MEI_DRV_CRLF));

            /* execute access */
            ret = MEI_MeiDmaTest( pMeiDev,
                                    pUserArgs->dbg_access.dbgAddr,
                                    pUserArgs->dbg_access.count,
                                    pUserArgs->dbg_access.dbgDest);
         }
         break;

#endif

      default:
         PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_WRN,
              ("MEI_DRV[%02d-%02d] Unknown IoCtl (0x%08X, magic 0x%08X), arg 0x%08X." MEI_DRV_CRLF,
               MEI_DRV_LINENUM_GET(pMeiDev), pMeiDynCntrl->openInstance, nCmd, MEIX_IOC_MAGIC, nArgument));
         ret = -e_MEI_ERR_UNKNOWN_CMD;
   }


MEI_IOCTL_RETURN:

   switch (nCmd)
   {
      default:
         pUserArgs->drv_ioctl.retCode = ret;
         break;
   }

   if (ret < 0)
   {
      errno = -ret;
      return IFX_ERROR;
   }
   else
   {
      errno = IFX_SUCCESS;
      return IFX_SUCCESS;
   }
}

#if (MEI_SUPPORT_IRQ == 1)
/**
   The interrupt connect wrapper function.
   - do the standard Nucleus intConnect
   - lock further updates of the interrupt wrapper functions

\param
   pIntVector  - points to the interrupt vector (IRQ)
\param
   pISRRoutine - points to the interrupt service routine
\param
   ISRParams   - the ISR parameter (will be casted to the DFEx devcie struct).

\return
   0       in case of success.
   -1    if something goes wrong.

\remark
   The function wrapps the normal Nucleus intConnect function and additional
   locks the actually set interrupt functions against further updates.
   So no further update of the following interrupt functions is allowded:
   - Interrrupt Service Routine
   - intEnable, intDisable
   - This function.
*/
MEI_STATIC IFX_int_t MEI_IfxIntConnect(
                                 void *pIntVector,
                                 void *pISRRoutine,
                                 int ISRParams)
{
   IFX_int_t ret;

   /*
      Connect the interrupt service routine
   */
   if ( (ret = MEI_IntConnect_WrapNUCLEUS( pIntVector, pISRRoutine, ISRParams)) == -1 )
   {
      /* error while register ISR */
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
             ("MEI_DRV: ERROR - WRAPPER intConnect() = %d" MEI_DRV_CRLF, ret));

      return ret;
   }

   /* LOCK: from now on no further update is possible */
   MEI_IntSetupLocked++;

   PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
          ("MEI_DRV:  MEI_IfxIntConnect(%d, .., ..), lock = %d" MEI_DRV_CRLF,
          (int)pIntVector, MEI_IntSetupLocked));


   return ret;
}


/**
   The driver interrupt shell.

\param
   IsrParam  - points to the DFEx control struct (always the fist in the list)

\return
   None.

\remark
   Per IRQ only the count of the first DFEx will be incremented.
*/
MEI_STATIC void MEI_InterruptNucleus(int ISRParams)
{
   IFX_int32_t    meiIntCnt = 0;
   MEIX_CNTRL_T *pMeiXCntrlList = (MEIX_CNTRL_T *)ISRParams;

   pMeiXCntrlList->IRQ_Count++;

   meiIntCnt = MEI_ProcessIntPerIrq(pMeiXCntrlList);

   if (meiIntCnt)
      pMeiXCntrlList->IRQ_Protection = MEI_IrqProtectCount;
   else
      pMeiXCntrlList->IRQ_Protection--;

   if (pMeiXCntrlList->IRQ_Protection <= 0)
   {
      /* The OS signals available IRQ's but no interrupt found for processing */
      PRN_ERR_INT_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR, (MEI_DRV_CRLF));
      PRN_ERR_INT_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DRV: IRQ setup - !!! FATAL ERROR !!!" MEI_DRV_CRLF));
      PRN_ERR_INT_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR, (MEI_DRV_CRLF));
      PRN_ERR_INT_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DRV: +++  R I P   R I P   R I P  +++" MEI_DRV_CRLF));
      PRN_ERR_INT_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DRV: +++        - IRQ %02d  -      +++" MEI_DRV_CRLF,
             pMeiXCntrlList->IRQ_Num));
      PRN_ERR_INT_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DRV: +++ -------- R I P -------- +++" MEI_DRV_CRLF));
      PRN_ERR_INT_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR, (MEI_DRV_CRLF));
      MEI_IntDisable_WrapNUCLEUS(pMeiXCntrlList->IRQ_Num);

      /* mark all devices with config error */
      MEI_DisableDevsPerIrq(pMeiXCntrlList);
   }

}

#endif


/* ============================================================================
   I/O VRX X (Control) driver functions (Nucleus) - definition
   ========================================================================= */



/**
   Open a DFEx control device to control a DFEx chip device

\param
   pDfeXCntrl  private control device (chip) data
\param
   pAnnex      remaing part of string used during open()
\param
   flags       additional flags

\return
   -1 - on failure,
   non zero value on success

*/
MEI_STATIC IFX_int_t MEI_DfeXDevOpen(
                                 void *priv,
                                 const IFX_char_t *pAnnex)
{
   MEIX_CNTRL_T *pMeiXCntrl = (MEIX_CNTRL_T *)priv;
   /*
      now special action required
   */

   PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL,
          ("MEI_DRV entity[%02d]: Open DFEx control" MEI_DRV_CRLF, pMeiXCntrl->entityNum));

   /*
      Return the DFEx control data block - assign to the new fd.
   */
   return (IFX_int32_t)pMeiXCntrl;
}


/**
   Release the device.

\param
   pDfeXCntrl  private control device (chip) data

\return
   IFX_SUCCESS - on success.
   IFX_ERROR   - if privat dynamic data has be lost.
*/
MEI_STATIC IFX_int32_t MEI_CloseDfeX(void *priv)
{
   MEIX_CNTRL_T *pMeiXCntrl = (MEIX_CNTRL_T *)priv;
   /*
      now special action required
   */

   PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_LOW,
          ("MEI_DRV entity[%02d]: Close DFEx control" MEI_DRV_CRLF, pMeiXCntrl->entityNum));

   return IFX_SUCCESS;
}

/* ============================================================================
   Local function definitions (Nucleus)
   ========================================================================= */

/**
   Allocate DFEx/VRX structure blocks for all devices and channels

\return
   IFX_SUCCESS in case of success
   e_MEI_ERR_NO_MEM in case of error (no mem)

*/
MEI_STATIC IFX_int32_t MEI_DevAllocAll(void)
{
   IFX_uint8_t deviceNum;
   IFX_int32_t retVal = IFX_SUCCESS;

   for (deviceNum=0; deviceNum < MEI_DFE_CHAN_DEVICES; deviceNum++)
   {
      /* check/allocate VRX device control block */
      if ( (retVal = MEI_DevLineAlloc(deviceNum)) != IFX_SUCCESS )
      {
         return retVal;
      }
   }

   return IFX_SUCCESS;
}


/**
   Free DFEx/VRX structure blocks for all devices and channels

\return
   none

*/
MEI_STATIC IFX_int32_t MEI_DevFreeAll(void)
{
   IFX_uint8_t entityNum, deviceNum, allFree = 0;


   for (deviceNum=0; deviceNum < MEI_DFE_CHAN_DEVICES; deviceNum++)
   {
      /* check/allocate VRX device control block */
      if ( MEI_DevLineStructFree(deviceNum) != IFX_SUCCESS )
         allFree++;
   }

   if ( allFree )
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
             ("MEI_DRV: WARNING FreeAll - %d VRX device(s) busy" MEI_DRV_CRLF,
              allFree));
   }


   allFree = 0;
   for (entityNum=0; entityNum < MEI_DFEX_ENTITIES; entityNum++)
   {
      /* check/allocate DFEx entity control block */
      if ( MEI_DevXCntrlStructFree(entityNum) != IFX_SUCCESS )
         allFree++;
   }

   if ( allFree )
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
             ("MEI_DRV: WARNING FreeAll - %d DFEx device(s) not free" MEI_DRV_CRLF,
              allFree));
   }

   return (allFree) ? IFX_ERROR : IFX_SUCCESS;
}


/**
   Add all devices to the IO system.

\param
   pDrvBaseName   - basename of the corresponding driver.
\param
   drvNum         - number of the corresponding driver.
\param
   pDevNumLast    - number of last processed device

\return
   IFX_SUCCESS if all devices added successful

*/
MEI_STATIC IFX_int32_t MEI_DevAddAll(
                                 IFX_char_t  *pDrvBaseName,
                                 IFX_int32_t drvNum,
                                 IFX_uint8_t *pDevNumLast)
{
   MEI_DEV_T   *pMeiDev = NULL;
   IFX_uint8_t      deviceNum, entity, rel_ch;
   IFX_char_t        buf[64];

   for (deviceNum=0; deviceNum < MEI_DFE_CHAN_DEVICES; deviceNum++)
   {
      /* calculate entity and relative channel number */
      entity = MEI_GET_ENTITY_FROM_DEVNUM(deviceNum);
      rel_ch = MEI_GET_REL_CH_FROM_DEVNUM(deviceNum);

      if ( (MEIX_Cntrl[entity] != NULL) &&
           ((pMeiDev = (MEIX_Cntrl[entity])->MeiDevice[rel_ch]) != NULL) )
      {
         /* Create device name: /dev/<vrx drv name>/<device number> */
         sprintf(buf, "%s/%d", pDrvBaseName, deviceNum);

         PRN_DBG_USR( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL, deviceNum,
                ("MEI_DRV: DEVIO_device_add - device <%s>" MEI_DRV_CRLF, buf));

         /* Add device to the device list */
         if (DEVIO_device_add(pMeiDev, buf, drvNum) == -1)
         {
            PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
                   ("MEI_DRV[???]: unable to create device." MEI_DRV_CRLF));

            *pDevNumLast = deviceNum;
            return -e_MEI_ERR_DEV_INIT;
         }
      }
      else
      {
         *pDevNumLast = deviceNum;
         return -e_MEI_ERR_DEV_NOT_EXIST;
      }
   }

   *pDevNumLast = MEI_DFE_CHAN_DEVICES;

   return IFX_SUCCESS;
}



/**
   Delete all devices from the IO system.

\param
   pDevNumLast    - number of last processed device

\return
   none

*/
MEI_STATIC IFX_void_t MEI_DevDeleteAll(IFX_uint8_t devNumLast)
{
   MEI_DEV_T   *pMeiDev = NULL;
   IFX_uint8_t      deviceNum, entity, rel_ch;

   for (deviceNum=0; deviceNum < devNumLast; deviceNum++)
   {
      /* calculate entity and relative channel number */
      entity = MEI_GET_ENTITY_FROM_DEVNUM(deviceNum);
      rel_ch = MEI_GET_REL_CH_FROM_DEVNUM(deviceNum);

      if ( (MEIX_Cntrl[entity] != NULL) &&
           ((pMeiDev = (MEIX_Cntrl[entity])->MeiDevice[rel_ch]) != NULL) )
      {
         /* remove device */
         PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_WRN,
                ("MEI_DRV: DEVIO_device_delete - device %s/%d" MEI_DRV_CRLF,
                  DRV_MEI_NAME, deviceNum));
         DEVIO_device_delete(pMeiDev);
      }
   }

   return;
}

/**
   Init the corresponding device.

\param
   pMeiDynCntrl - private dynamic control data (per open instance)
\param
   pInitDev       - init information data

\return
   0: success
   else < 0 in case of error

*/
IFX_int32_t MEI_IoctlInitDevice(
                                 MEI_DYN_CNTRL_T     *pMeiDynCntrl,
                                 IOCTL_MEI_devInit_t *pInitDev)
{
   int ret = 0;
   MEI_DEV_T    *pMeiDev = pMeiDynCntrl->pMeiDev;

   PRN_DBG_USR( MEI_DRV, MEI_DRV_PRN_LEVEL_LOW, MEI_DRV_LINENUM_GET(pMeiDev),
          ("MEI_DRV[%02d]: ioctl - FIO_MEI_DEV_INIT "
           "base addr = 0x%08X, IRQ = %d" MEI_DRV_CRLF,
            MEI_DRV_LINENUM_GET(pMeiDev),
            (IFX_uint32_t)pInitDev->meiBaseAddr, (IFX_uint32_t)pInitDev->usedIRQ));

   /*
      Do IO remap
   */
   if ( (ret = MEI_DRVOS_Phy2VirtMap( pInitDev->meiBaseAddr,
                                      sizeof(MEI_MEI_REG_IF_U),
                                      (IFX_char_t*)DRV_MEI_NAME,
                                      (IFX_uint8_t **)(&pMeiDev->meiDrvCntrl.pMeiIfCntrl->pVirtMeiRegIf)) )
        != IFX_SUCCESS )
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
             ("MEI_DRV[%02d]: ERROR - INIT DEVICE, IO remap" MEI_DRV_CRLF,
             MEI_DRV_LINENUM_GET(pMeiDev)));

      return -e_MEI_ERR_DEV_IO_MAP;
   }

   MEI_DRV_MEI_PHY_ADDR_SET(pMeiDev, pInitDev->meiBaseAddr);

   /* check HW access */
   if ( (ret = MEI_MeiRegisterDetect(pMeiDev)) != IFX_SUCCESS)
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
             ("MEI_DRV[%02d]: ERROR - INIT DEVICE, HW failure" MEI_DRV_CRLF,
             MEI_DRV_LINENUM_GET(pMeiDev)));

      goto MEI_IOCTL_INITDEV_BASIC_ERROR;
   }

   /*
      Init the VRX
      - MEI reset
      - mask interrupts, clear interrupts
      - get VRX info
      NOTE:
         MEI Block reset and clear interrupts depends on the
         resetMode param.
   */
   if ( (ret = MEI_DefaultInitDevice(pMeiDev, 0)) < 0 )
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
             ("MEI_DRV[%02d]: ERROR - INIT DEVICE, Reset VRX device, get VRX info" MEI_DRV_CRLF,
             MEI_DRV_LINENUM_GET(pMeiDev)));

      goto MEI_IOCTL_INITDEV_BASIC_ERROR;
   }

   if ( (pInitDev->usedIRQ == 0) || (pInitDev->usedIRQ == 99) )
   {
      PRN_DBG_USR( MEI_DRV, MEI_DRV_PRN_LEVEL_HIGH, MEI_DRV_LINENUM_GET(pMeiDev),
             ("MEI_DRV[%02d]: INIT DEVICE - NO INTERUPTS --> %s POLL MODE" MEI_DRV_CRLF,
             MEI_DRV_LINENUM_GET(pMeiDev), (pInitDev->usedIRQ == 0) ? "PASSIVE" : "ACTIVE" ));
   }

   switch (pInitDev->usedIRQ)
   {
      case 0:
         /* IRQ = 0: passive poll mode */
         pMeiDynCntrl->pDfeX->IRQ_Num = 0;
         pMeiDev->eModePoll           = e_MEI_DEV_ACCESS_MODE_PASSIV_POLL;
         pMeiDev->intMask             = 0;
         break;

      case 99:
#if (MEI_SUPPORT_PERIODIC_TASK == 1)
         /* IRQ = 99: active poll mode */
         pMeiDynCntrl->pDfeX->IRQ_Num = 99;
         pMeiDev->eModePoll           = e_MEI_DEV_ACCESS_MODE_ACTIV_POLL;
         pMeiDev->intMask             = 0;

         /* check if the periodic task is already running */
         if ( MEI_DrvCntrlThreadParams.bValid == IFX_FALSE)
         {
            PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_HIGH,
                  ("MEI_DRV[%02d]: Start Driver Control Thread" MEI_DRV_CRLF,
                   MEI_DRV_LINENUM_GET(pMeiDev)));

            if (MEI_DRVOS_ThreadInit(&MEI_DrvCntrlThreadParams,
                                    "VrxCtrl",
                                    MEI_DrvCntrlThr,
                                    0, 0) != IFX_SUCCESS)
            {
               PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
                     ("MEI_DRV[%02d]: ERROR - start Driver Control Thread" MEI_DRV_CRLF,
                       MEI_DRV_LINENUM_GET(pMeiDev)));
            }
         }
         else
         {
            PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_HIGH,
                  ("MEI_DRV[%02d]: Driver Control Thread - already running" MEI_DRV_CRLF,
                    MEI_DRV_LINENUM_GET(pMeiDev)));

         }
#else
         PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
               ("MEI_DRV[%02d]: ERROR - IRQ 99 not supported without periodical task" MEI_DRV_CRLF,
                 MEI_DRV_LINENUM_GET(pMeiDev)));

         ret = -e_MEI_ERR_INVAL_BASE_CFG;
         goto MEI_IOCTL_INITDEV_BASIC_ERROR;
#endif   /* #if (MEI_SUPPORT_PERIODIC_TASK == 1) */
         break;

      default:
#if (MEI_SUPPORT_IRQ == 1)
         {
            /*
               Check if the given IRQ already in use
               - add to existing list (return NULL already registered)
               - create a new list and return the list root
            */
            MEIX_CNTRL_T *pTmpXCntrl;

            pMeiDev->eModePoll = e_MEI_DEV_ACCESS_MODE_IRQ;
            pMeiDev->intMask   = ME_ARC2ME_INTERRUPT_UNMASK_ALL;

            pTmpXCntrl = MEI_VrxXDevToIrqListAdd(
                                          MEI_DRV_LINENUM_GET(pMeiDev),
                                          (IFX_uint32_t)pInitDev->usedIRQ,
                                          pMeiDynCntrl->pDfeX);

            if (pTmpXCntrl)
            {
               /*
                  Connect the interrupt service routine via the wrapper function.
                  Note:
                     The IFX default intConnect function allows this only once a time.
               */
               if ( (ret = MEI_IfxIntConnect( ((void *) (pTmpXCntrl->IRQ_Num)),
                                                MEI_InterruptNucleus,
                                                (int)pTmpXCntrl)) == IFX_ERROR )
               {
                  /* error while register ISR */
                  PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
                        ("MEI_DRV[%02d]: ERROR - INIT DEVICE, intConnect() = %d" MEI_DRV_CRLF,
                        MEI_DRV_LINENUM_GET(pMeiDev), ret));

                  ret = -e_MEI_ERR_INVAL_BASE_CFG;
                  goto MEI_IOCTL_INITDEV_BASIC_ERROR;
               }

               MEI_IntEnable_WrapNUCLEUS(pTmpXCntrl->IRQ_Num);
            }
         }
#else
         PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
               ("MEI_DRV[%02d]: ERROR - VRX Driver IRQ Support not enabled" MEI_DRV_CRLF,
                 MEI_DRV_LINENUM_GET(pMeiDev)));

         MEI_DRV_STATE_SET(pMeiDev, e_MEI_DRV_STATE_CFG_ERROR);
#endif
         break;
   }

   if (MEI_DRV_STATE_GET(pMeiDev) == e_MEI_DRV_STATE_CFG_ERROR)
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
      ("MEI_DRV[%02d]: ERROR - INIT DEVICE, IRQ = %d already blocked !!!" MEI_DRV_CRLF,
       MEI_DRV_LINENUM_GET(pMeiDev), pInitDev->usedIRQ));

      ret = -e_MEI_ERR_INVAL_BASE_CFG;
   }
   else
   {
      PRN_DBG_USR( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL, MEI_DRV_LINENUM_GET(pMeiDev),
             ("MEI_DRV[%02d]: INIT VRX, phy addr = 0x%08X, virt addr = 0x%08X IRQ = %d" MEI_DRV_CRLF,
               MEI_DRV_LINENUM_GET(pMeiDev), MEI_DRV_MEI_PHY_ADDR_GET(pMeiDev),
               (IFX_uint32_t)(MEI_DRV_MEI_VIRT_ADDR_GET(pMeiDev)),
               (IFX_uint32_t)pInitDev->usedIRQ));

      /*
         next state
      */
      MEI_DRV_STATE_SET(pMeiDev, e_MEI_DRV_STATE_SW_INIT_DONE);
      MEI_EnableDeviceInt(pMeiDev);

      if (pMeiDev->eModePoll == e_MEI_DEV_ACCESS_MODE_PASSIV_POLL)
      {
         MEI_PollIntPerVrxLine(pMeiDev, e_MEI_DEV_ACCESS_MODE_PASSIV_POLL);
      }
   }

   return ret;

MEI_IOCTL_INITDEV_BASIC_ERROR:

   /* unmap memory and release memory region */
   MEI_DRVOS_Phy2VirtUnmap(
                  &(MEI_DRV_MEI_PHY_ADDR_GET(pMeiDev)),
                  sizeof(MEI_MEI_REG_IF_U),
                  (IFX_uint8_t **)(&pMeiDev->meiDrvCntrl.pMeiIfCntrl->pVirtMeiRegIf));

   /* reset init done */
   MEI_DRV_STATE_SET(pMeiDev, e_MEI_DRV_STATE_NOT_INIT);

   return ret;
}


/* ============================================================================
   Global I/O VRX driver functions (Nucleus) - definition
   ========================================================================= */

/**
   VRX Driver initialization.

\return
   IFX_SUCCESS or IFX_ERROR

*/
int MEI_DevCreate(void)
{
   IFX_uint8_t       deviceNumLast;
   IFX_int32_t       ret = IFX_SUCCESS;

   printf("%s", &MEI_WHATVERSION[4]);
   printf(", (c) 2013 Lantiq Deutschland GmbH" MEI_DRV_CRLF MEI_DRV_CRLF);

   MEI_DRV_PRN_USR_LEVEL_SET(MEI_DRV, MEI_DRV_PRN_LEVEL_HIGH);
   MEI_DRV_PRN_INT_LEVEL_SET(MEI_DRV, MEI_DRV_PRN_LEVEL_HIGH);

#if (MEI_SUPPORT_ROM_CODE == 1)
   MEI_DRV_PRN_USR_LEVEL_SET(MEI_ROM, MEI_DRV_PRN_LEVEL_HIGH);
   MEI_DRV_PRN_INT_LEVEL_SET(MEI_ROM, MEI_DRV_PRN_LEVEL_HIGH);
#endif

   MEI_DRV_PRN_USR_LEVEL_SET(MEI_MEI_ACCESS, MEI_DRV_PRN_LEVEL_HIGH);
   MEI_DRV_PRN_INT_LEVEL_SET(MEI_MEI_ACCESS, MEI_DRV_PRN_LEVEL_HIGH);

#ifdef MEI_GENERIC_PROC_FS
   /* dummy call - prevent linker from remove this object */
   if (0)
      doVrxProcFs(0, 0, 0, 0, 0);

#endif

   /* ===========================================================
      Reset the global VRX devices data blocks
      =========================================================== */
   if (VRXDrvNum <= 0)
   {
      /* first create of VRX / DFEx devices */
      if (MEIXDrvNum <= 0)
      {
         memset(MEIX_Cntrl, 0x00, sizeof(MEIX_Cntrl));
      }

#if (MEI_SUPPORT_DL_DMA_CS == 1)
      /* Reset firmware image cntrl block */
      memset(&ChipFwImage, 0x00, sizeof(ChipFwImage[MEI_MAX_FW_IMAGES]));

      if (MEI_DRVOS_SemaphoreInit(&pFwDlCntrlLock) != IFX_SUCCESS)
      {
         PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
               ("MEI_DRV: ERROR - Init FW DL control lock" MEI_DRV_CRLF));
         return IFX_ERROR;
      }
#endif
   }


   /* ===========================================================
      Install the VRX driver.
      =========================================================== */
   if (VRXDrvNum <= 0)
   {
      /* add driver to driver table */
      VRXDrvNum = DEVIO_driver_install(
                                 MEI_CpeDevOpen,
                                 MEI_Close,
                                 MEI_Read,
                                 MEI_Write,
                                 MEI_IoCtl, NULL);

      if (VRXDrvNum < 0)
      {
         PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
                ("MEI_DRV: unable to install the driver." MEI_DRV_CRLF));
         return (IFX_ERROR);
      }
   }
   else
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_WRN,
             ("MEI_DRV: driver already installed." MEI_DRV_CRLF));
      return (IFX_ERROR);
   }


   /* ===========================================================
      Allocate the VRX devices data blocks
      =========================================================== */
   if ( (ret = MEI_DevAllocAll()) != IFX_SUCCESS)
   {
      goto MEI_DEV_CREATE_ALLOC_ERROR;
   }


   /* ===========================================================
      Add the devices to the device list
      =========================================================== */
   if ( (ret = MEI_DevAddAll( DRV_MEI_NAME,
                                    VRXDrvNum,
                                    &deviceNumLast)
        ) != IFX_SUCCESS )
   {
      goto MEI_DEV_CREATE_ADD_DEV_ERROR;
   }



   return ret;

MEI_DEV_CREATE_ADD_DEV_ERROR:

   MEI_DevDeleteAll(deviceNumLast);

MEI_DEV_CREATE_ALLOC_ERROR:

   /* free allocated memory, check for the DFEx Control driver part */
   if (MEIXDrvNum <= 0)
   {
      /* the DFEx Control part is currently not installed */
      MEI_DevFreeAll();
   }

   /* remove the driver also and allow new DevCreate */
   DEVIO_driver_remove(VRXDrvNum, IFX_TRUE);
   VRXDrvNum = 0;

   errno = ret;
   return ret;
}


/**
   Remove the VRX Driver .

\return
   IFX_SUCCESS successful, all freed and driver removed
   IFX_ERROR   not successful, devices still busy
*/
int MEI_DevDelete(void)
{
   /*
      Delete all devices from the IO system
   */
   MEI_DevDeleteAll(MEI_DFE_CHAN_DEVICES);

   /*
      only if Control driver is already removed
   */
   if (MEIXDrvNum <= 0)
   {
      /*
         free allocated memory device structure
      */
      MEI_DevFreeAll();
   }

   /* remove the driver also and allow new DevCreate */
   DEVIO_driver_remove(VRXDrvNum, IFX_TRUE);
   VRXDrvNum = 0;

#if (MEI_SUPPORT_DL_DMA_CS == 1)
   /* mutex exist */
   MEI_DRVOS_SemaphoreDelete(&pFwDlCntrlLock);
#endif

   return IFX_SUCCESS;
}

/* ============================================================================
   Shift to the common part
   ========================================================================= */


/**
   Close a VRX device - free all dynamic allocated memory blocks.

\param
   pMeiDynCntrl  private dynamic control data

\return
   IFX_SUCCESS - successful close VRX device
   IFX_ERROR   - error on close VRX device

*/
IFX_int32_t MEI_DfeDevClose(MEI_DYN_CNTRL_T *pMeiDynCntrl)
{
   MEI_DevLineClose(pMeiDynCntrl);

   return IFX_SUCCESS;
}


#if (MEI_SUPPORT_IRQ == 1)
/* ============================================================================
   Setup VRX driver interrupt functions (Nucleus) - wrapping
   ========================================================================= */

/**
   Set the wrapper function for the Nucleus intConnect function.

\param
   pIntConnectFct  - points to the custumer intConnect function

\return
   0     in case of success.
   -1    if the function pointer is not valid, or if the interrupt routines
         already in use.

\remark
   The MEI_IfxIntConnect() function locks also further updates of the
   interrupt wrapper functions.

\remark
   After the first intConnect call further updates should not be possible anymore.

*/
int MEI_FctIntConnectSet(MEI_IntConnect_WrapNUCLEUS_t pIntConnectFct)
{

   if ( (MEI_IntSetupLocked != 0) || (pIntConnectFct == NULL) )
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DRV: ERROR - setup intConnect wrap, locked = %d" MEI_DRV_CRLF,
              MEI_IntSetupLocked));

      return -1;
   }

   PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_HIGH,
         ("MEI_DRV: setup intConnect wrapper" MEI_DRV_CRLF));

   MEI_IntConnect_WrapNUCLEUS = pIntConnectFct;

   return 0;
}
#endif /* (MEI_SUPPORT_IRQ == 1) */

#if (MEI_SUPPORT_IRQ == 1)
/**
   Set the wrapper function for the Nucleus intEnable function.

\param
   pIntEnableFct  - points to the custumer intEnable function

\return
   0     in case of success.
   -1    if the function pointer is not valid, or if the interrupt routines
         already in use.

\remark
   After the first intConnect call further updates should not be possible anymore.

*/
int MEI_FctIntEnableSet(MEI_IntEnable_WrapNUCLEUS_t  pIntEnableFct)
{

   if ( (MEI_IntSetupLocked != 0) || (pIntEnableFct == NULL) )
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DRV: ERROR - setup INT Enable wrap, locked = %d" MEI_DRV_CRLF,
              MEI_IntSetupLocked));

      return -1;
   }

   PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_HIGH,
         ("MEI_DRV: setup INT Enable wrapper" MEI_DRV_CRLF));

   MEI_IntEnable_WrapNUCLEUS  = pIntEnableFct;

   return 0;
}
#endif /* (MEI_SUPPORT_IRQ == 1) */

#if (MEI_SUPPORT_IRQ == 1)
/**
   Set the wrapper function for the Nucleus intDisable function.

\param
   pIntDisableFct - points to the custumer intDisable function

\return
   0     in case of success.
   -1    if the function pointer is not valid, or if the interrupt routines
         already in use.

\remark
   After the first intConnect call further updates should not be possible anymore.

*/
int MEI_FctIntDisableSet(MEI_IntDisable_WrapNUCLEUS_t pIntDisableFct)
{

   if ( (MEI_IntSetupLocked != 0) || (pIntDisableFct == NULL) )
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DRV: ERROR - setup INT Disable wrap, locked = %d" MEI_DRV_CRLF,
              MEI_IntSetupLocked));

      return -1;
   }

   PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_HIGH,
         ("MEI_DRV: setup INT En/Disable wrapper" MEI_DRV_CRLF));

   MEI_IntDisable_WrapNUCLEUS = pIntDisableFct;

   return 0;
}
#endif /* (MEI_SUPPORT_IRQ == 1) */

#endif /* NUCLEUS_PLUS */

