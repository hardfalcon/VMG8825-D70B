/******************************************************************************

                          Copyright (c) 2007-2015
                     Lantiq Beteiligungs-GmbH & Co. KG

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/


/* ==========================================================================
   Description : ATM OAM Common functions for the VRX Driver
   ========================================================================== */

/* ============================================================================
   Includes
   ========================================================================= */

/* get at first the driver configuration */
#include "drv_mei_cpe_config.h"

#if (MEI_DRV_ATM_OAM_ENABLE == 1)

#include "ifx_types.h"
#include "drv_mei_cpe_os.h"
#include "drv_mei_cpe_dbg.h"
#include "drv_mei_cpe_dbg_driver.h"

#include "drv_mei_cpe_atmoam.h"
#include "drv_mei_cpe_api.h"
#include "drv_mei_cpe_interface.h"

#include "drv_mei_cpe_atmoam_common.h"
#include "drv_mei_cpe_device_cntrl.h"
#include "drv_mei_cpe_msg_process.h"

#if (MEI_SUPPORT_DRIVER_MSG == 1)
#include "drv_mei_cpe_driver_msg.h"
#endif


/* ============================================================================
   Global Macro Definitions
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
   Local Function Declaration
   ========================================================================= */
MEI_STATIC IFX_int32_t MEI_AtmOamCreateDevCntrl(
                              MEI_DYN_CNTRL_T         *pMeiDynCntrl,
                              MEI_ATMOAM_DEV_CNTRL_T **ppAtmOamDevCntrl);

MEI_STATIC IFX_int32_t MEI_AtmOamWaitForTxBuffer(
                              MEI_DYN_CNTRL_T         *pMeiDynCntrl,
                              MEI_ATMOAM_DEV_CNTRL_T  *pAtmOamDevCntrl,
                              IFX_uint32_t              retryCount);

/* ============================================================================
   Global Variable Definition
   ========================================================================= */

/* VRX-Driver: Access module - create print level variable */
MEI_DRV_PRN_USR_MODULE_CREATE(MEI_ATMOAM, MEI_DRV_PRN_LEVEL_LOW);
MEI_DRV_PRN_INT_MODULE_CREATE(MEI_ATMOAM, MEI_DRV_PRN_LEVEL_LOW);

/* ============================================================================
   Local Variable Definition
   ========================================================================= */


/* ============================================================================
   Local Function Definition
   ========================================================================= */

/**
   Allocate the ATM OAM Access Control structure.

\param
   pMeiDynCntrl    private dynamic device data (per open instance) [I]
\param
   ppAtmOamDevCntrl  points to the ATM OAM control struct pointer [IO]
                     In case of success returns pointer to the allocated structure.
\return
   0 (IFX_SUCCESS) if success.
   negative value in case of error.
      -e_MEI_ERR_DEV_INIT: invalid argument
      -e_MEI_ERR_ALREADY_DONE: allocation already done (ptr not null)
      -e_MEI_ERR_NO_MEM: not enough memory
*/
MEI_STATIC IFX_int32_t MEI_AtmOamCreateDevCntrl(
                              MEI_DYN_CNTRL_T         *pMeiDynCntrl,
                              MEI_ATMOAM_DEV_CNTRL_T **ppAtmOamDevCntrl)
{
   MEI_ATMOAM_DEV_CNTRL_T *pAtmOamDevCntrl;
   MEI_DRV_PRN_LOCAL_VAR_CREATE(MEI_DEV_T, *pMeiDev, pMeiDynCntrl->pMeiDev );

   if (ppAtmOamDevCntrl == IFX_NULL)
   {
      PRN_ERR_USR_NL( MEI_ATMOAM, MEI_DRV_PRN_LEVEL_ERR,
             ("MEI_ATM[%02d]: ERROR - invalid arg for create ATM OAM control" MEI_DRV_CRLF
               , MEI_DRV_LINENUM_GET(pMeiDev)) );

      return -e_MEI_ERR_DEV_INIT;
   }

   if (*ppAtmOamDevCntrl != IFX_NULL)
   {
      PRN_ERR_USR_NL( MEI_ATMOAM, MEI_DRV_PRN_LEVEL_ERR,
             ("MEI_ATM[%02d]: ERROR - ATM OAM control already exist" MEI_DRV_CRLF
               , MEI_DRV_LINENUM_GET(pMeiDev)) );

      return -e_MEI_ERR_ALREADY_DONE;
   }

   pAtmOamDevCntrl = (MEI_ATMOAM_DEV_CNTRL_T *)MEI_DRVOS_Malloc(sizeof(MEI_ATMOAM_DEV_CNTRL_T));
   if (!pAtmOamDevCntrl)
   {
      PRN_ERR_USR_NL( MEI_ATMOAM, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_ATM[%02d]: no memory for ATM OAM control structure." MEI_DRV_CRLF
             , MEI_DRV_LINENUM_GET(pMeiDev)));

      return -e_MEI_ERR_NO_MEM;
   }

   memset( (char*)pAtmOamDevCntrl, 0x00, sizeof(MEI_ATMOAM_DEV_CNTRL_T) );

   MEI_DRVOS_SemaphoreInit(&pAtmOamDevCntrl->pDevAtmOamCtrlRWlock);
   MEI_DRVOS_EventInit(&pAtmOamDevCntrl->eventAtmOamInstDone);
   pAtmOamDevCntrl->bAtmOamInstDoneNeedWakeUp = IFX_FALSE;

   MEI_ATMOAM_CFG_STATE_SET(pAtmOamDevCntrl, eMEI_ATMOAM_OP_CFG_INITIAL);
   MEI_ATMOAM_TXBUF_STATE_SET(pAtmOamDevCntrl, eMEI_ATMOAM_OP_TX_BUF_INITIAL);

   *ppAtmOamDevCntrl = pAtmOamDevCntrl;

   return IFX_SUCCESS;
}

/**
   Depending on the configuration POLL or only WAIT for TX Buffer free.

\param
   pMeiDynCntrl    private dynamic device data (per open instance) [I]
\param
   pAtmOamDevCntrl   points to the ATM OAM control struct pointer [IO]
\param
   retryCount        number of attempts to poll the status [I]

\return
   IFX_SUCCESS if success - TX buffer is free.
   IFX_ERROR   TX buffer is not free.
*/
MEI_STATIC IFX_int32_t MEI_AtmOamWaitForTxBuffer(
                              MEI_DYN_CNTRL_T         *pMeiDynCntrl,
                              MEI_ATMOAM_DEV_CNTRL_T  *pAtmOamDevCntrl,
                              IFX_uint32_t              retryCount)
{
   IFX_uint32_t insStatus, count = 0;

   MEI_DEV_T  *pMeiDev = pMeiDynCntrl->pMeiDev;

   if ( (pAtmOamDevCntrl->bEnEvtOnInsert == IFX_FALSE) ||
        (MEI_ATMOAM_CHECK_TRANSMODE_EVT_TX(pAtmOamDevCntrl)) )
   {
      /* events are disabled or in trans mode --> poll status */
      while ( (count < retryCount) &&
              (MEI_ATMOAM_TXBUF_STATE_GET(pAtmOamDevCntrl) == eMEI_ATMOAM_OP_TX_BUF_BUSY) )
      {
         if ( MEI_ATMOAM_CMD_InsStatusGet(
                     pMeiDynCntrl, pAtmOamDevCntrl, &insStatus) != IFX_SUCCESS )
         {
            break;
         }
         MEI_DRVOS_EventWait_timeout(
                     &pAtmOamDevCntrl->eventAtmOamInstDone,
                     MEI_ATMOAM_MIN_TXBUFFER_POLL_TIME_MS);
      }
   }
   else
   {
      /* events are enabled and not in trans mode --> wait for events */
      if (pMeiDev->eModePoll == e_MEI_DEV_ACCESS_MODE_PASSIV_POLL)
      {
         MEI_PollIntPerVrxLine(pMeiDev, e_MEI_DEV_ACCESS_MODE_PASSIV_POLL);

         /* setup timeout counter for TX buffer free */
         MEI_ATMOAM_SET_TX_TIMEOUT_CNT( pAtmOamDevCntrl,
               MEI_ATMOAM_CELL_TRANS_DONE_TIMEOUT_MS / MEI_ATMOAM_MIN_TXBUFFER_POLL_TIME_MS);

         /* wait and then check for new messages */
         while( (MEI_ATMOAM_GET_TX_TIMEOUT_CNT(pAtmOamDevCntrl) > 0) &&
                (MEI_ATMOAM_TXBUF_STATE_GET(pAtmOamDevCntrl) == eMEI_ATMOAM_OP_TX_BUF_BUSY) )
         {
            MEI_DRVOS_EventWait_timeout(
                        &pAtmOamDevCntrl->eventAtmOamInstDone,
                        MEI_ATMOAM_MIN_TXBUFFER_POLL_TIME_MS);
            MEI_PollIntPerVrxLine(pMeiDev, e_MEI_DEV_ACCESS_MODE_PASSIV_POLL);
            MEI_ATMOAM_DEC_TX_TIMEOUT_CNT(pAtmOamDevCntrl);
         }
      }
      else
      {
         MEI_ATMOAM_SET_TX_TIMEOUT_CNT( pAtmOamDevCntrl,
               MEI_ATMOAM_CELL_TRANS_DONE_TIMEOUT_MS / MEI_ATMOAM_MIN_TXBUFFER_POLL_TIME_MS);

         while( (MEI_ATMOAM_GET_TX_TIMEOUT_CNT(pAtmOamDevCntrl) > 0) &&
                (MEI_ATMOAM_TXBUF_STATE_GET(pAtmOamDevCntrl) == eMEI_ATMOAM_OP_TX_BUF_BUSY) )
         {
            pAtmOamDevCntrl->bAtmOamInstDoneNeedWakeUp = IFX_TRUE;
            MEI_DRVOS_EventWait_timeout(
                        &pAtmOamDevCntrl->eventAtmOamInstDone,
                        MEI_ATMOAM_MIN_TXBUFFER_POLL_TIME_MS);
            pAtmOamDevCntrl->bAtmOamInstDoneNeedWakeUp = IFX_FALSE;
            MEI_ATMOAM_DEC_TX_TIMEOUT_CNT(pAtmOamDevCntrl);
         }
      }
   }

   if (MEI_ATMOAM_TXBUF_STATE_GET(pAtmOamDevCntrl) == eMEI_ATMOAM_OP_TX_BUF_FREE)
   {
      return IFX_SUCCESS;
   }
   else
   {
      return IFX_ERROR;
   }
}


/**
   Update the ATM OAM States depending on the driver state and FSM state.

\param
   pMeiDev      Points to the VRX driver device data. [I]

\return
   none.
*/
MEI_STATIC IFX_void_t MEI_ATMOAM_UpdateStates(
                              MEI_DEV_T *pMeiDev)
{
   MEI_ATMOAM_DEV_CNTRL_T   *pAtmOamDevCntrl = pMeiDev->pAtmOamDevCntrl;

   if (!pAtmOamDevCntrl)
      return;

#if 0
   printk(">>> ATMOAM Drv = %d, FSM = %d, CFG = %d, TxBuf = %d\n\r",
           MEI_DRV_STATE_GET(pMeiDev), MEI_DRV_MODEM_STATE_GET(pMeiDev),
           MEI_ATMOAM_CFG_STATE_GET(pAtmOamDevCntrl),
           MEI_ATMOAM_TXBUF_STATE_GET(pAtmOamDevCntrl) );
#endif


   if (MEI_DRV_STATE_GET(pMeiDev) != e_MEI_DRV_STATE_DFE_READY)
   {
      MEI_ATMOAM_ResetControl(pMeiDev);
   }
   else
   {
      if ( MEI_DRV_MODEM_STATE_GET(pMeiDev) == MEI_DRV_FSM_STATE_RESET)
      {
         MEI_ATMOAM_ResetControl(pMeiDev);
      }
   }

#if 0
   printk("<<< ATMOAM Drv = %d, FSM = %d, CFG = %d, TxBuf = %d\n\r",
           MEI_DRV_STATE_GET(pMeiDev), MEI_DRV_MODEM_STATE_GET(pMeiDev),
           MEI_ATMOAM_CFG_STATE_GET(pAtmOamDevCntrl),
           MEI_ATMOAM_TXBUF_STATE_GET(pAtmOamDevCntrl) );
#endif

   return;
}


/* ============================================================================
   Global Function Definition
   ========================================================================= */

/**
   Release the ATM OAM Access Control structure.

\param
   pMeiDev      Points to the VRX driver device data. [I]

\return
   0 (IFX_SUCCESS) if success.
   negative value in case of error.
*/
IFX_int32_t MEI_AtmOamReleaseDevCntrl(
                              MEI_DEV_T       *pMeiDev)
{
   MEI_ATMOAM_DEV_CNTRL_T *pAtmOamDevCntrl;

   if (pMeiDev->pAtmOamDevCntrl == IFX_NULL)
   {
      PRN_ERR_USR_NL( MEI_ATMOAM, MEI_DRV_PRN_LEVEL_ERR,
             ("MEI_ATM[%02d]: ERROR - invalid arg for release ATM OAM control" MEI_DRV_CRLF
               , MEI_DRV_LINENUM_GET(pMeiDev)) );

      return -e_MEI_ERR_DEV_INIT;
   }

   pAtmOamDevCntrl = pMeiDev->pAtmOamDevCntrl;

   /* get unique access to the module, dechain the ATM OAM block */
   MEI_ATMOAM_GET_UNIQUE_ACCESS(pAtmOamDevCntrl);
   MEI_DRV_GET_UNIQUE_ACCESS(pMeiDev);

   pMeiDev->pAtmOamDevCntrl = IFX_NULL;

   MEI_DRV_RELEASE_UNIQUE_ACCESS(pMeiDev);
   MEI_ATMOAM_RELEASE_UNIQUE_ACCESS(pAtmOamDevCntrl);

   /* release resourcen */
   MEI_DRV_GET_UNIQUE_ACCESS(pMeiDev);

   MEI_DRVOS_EventDelete(&pAtmOamDevCntrl->eventAtmOamInstDone);
   MEI_DRVOS_SemaphoreDelete(&pAtmOamDevCntrl->pDevAtmOamCtrlRWlock);
   MEI_DRVOS_Free(pAtmOamDevCntrl);
   pAtmOamDevCntrl = IFX_NULL;

   MEI_DRV_RELEASE_UNIQUE_ACCESS(pMeiDev);

   return IFX_SUCCESS;
}


/**
   Enable the ATM OAM Access Control on the modem.

\param
   pMeiDev      Points to the VRX driver device data. [I]

\return
   0 (IFX_SUCCESS) if success.
   negative value in case of error.
*/
IFX_int32_t MEI_AtmOamControlEnable(
                              MEI_DYN_CNTRL_T *pMeiDynCntrl,
                              MEI_DEV_T       *pMeiDev)
{
   IFX_int32_t              ret = IFX_ERROR;
   MEI_ATMOAM_DEV_CNTRL_T *pAtmOamDevCntrl;

   if (pMeiDev->pAtmOamDevCntrl == IFX_NULL)
   {
      PRN_ERR_USR_NL( MEI_ATMOAM, MEI_DRV_PRN_LEVEL_ERR,
             ("MEI_ATM[%02d]: ERROR - invalid arg for ATM OAM control enable " MEI_DRV_CRLF
               , MEI_DRV_LINENUM_GET(pMeiDev)) );

      return IFX_ERROR;
   }

   pAtmOamDevCntrl = pMeiDev->pAtmOamDevCntrl;

   MEI_ATMOAM_GET_UNIQUE_ACCESS(pAtmOamDevCntrl);

   if (MEI_ATMOAM_CFG_STATE_GET(pAtmOamDevCntrl) == eMEI_ATMOAM_OP_CFG_INITIAL)
   {
      PRN_ERR_USR_NL( MEI_ATMOAM, MEI_DRV_PRN_LEVEL_ERR,
             ("MEI_ATM[%02d]: ERROR - invalid ATM OAM Config for control enable " MEI_DRV_CRLF
               , MEI_DRV_LINENUM_GET(pMeiDev)) );

      MEI_ATMOAM_RELEASE_UNIQUE_ACCESS(pAtmOamDevCntrl);
      return IFX_ERROR;
   }

   MEI_ATMOAM_CFG_STATE_SET(pAtmOamDevCntrl, eMEI_ATMOAM_OP_CFG_VALID);
   MEI_ATMOAM_TXBUF_STATE_SET(pAtmOamDevCntrl, eMEI_ATMOAM_OP_TX_BUF_FREE);

   /* send the new configuration to the device */
   if ( (ret = MEI_ATMOAM_CMD_InsExtControl(pMeiDynCntrl, pAtmOamDevCntrl)) != IFX_SUCCESS)
   {
      MEI_ATMOAM_RELEASE_UNIQUE_ACCESS(pAtmOamDevCntrl);
      return ret;
   }

   MEI_ATMOAM_CFG_STATE_SET(pAtmOamDevCntrl, eMEI_ATMOAM_OP_CFG_WRITTEN);
   MEI_ATMOAM_TXBUF_STATE_SET(pAtmOamDevCntrl, eMEI_ATMOAM_OP_TX_BUF_FREE);

   MEI_ATMOAM_RELEASE_UNIQUE_ACCESS(pAtmOamDevCntrl);

   return IFX_SUCCESS;
}

/**
   ioctl-function for initialization of the ATM OAM access.
   Enable and allocate the resoucres for the ATM OAM feature within the driver.

\param
   pMeiDynCntrl Private dynamic device data (per open instance). [I]
\param
   pAtmOamInit    Points to the user data (user space). [I]

\return
   0 (IFX_SUCCESS) if success
   negative value in case of error
*/
IFX_int32_t MEI_ATMOAM_IoctlDrvInit(
                              MEI_DYN_CNTRL_T         *pMeiDynCntrl,
                              IOCTL_MEI_ATMOAM_init_t *pAtmOamInit )
{
   IFX_int32_t                ret = IFX_SUCCESS;
   MEI_DEV_T                *pMeiDev = pMeiDynCntrl->pMeiDev;
   MEI_ATMOAM_DEV_CNTRL_T   *pAtmOamDevCntrl = IFX_NULL;

   MEI_DRV_GET_UNIQUE_DRIVER_ACCESS(pMeiDev);
   if (pMeiDev->pAtmOamDevCntrl)
   {
      pAtmOamDevCntrl = pMeiDev->pAtmOamDevCntrl;
   }
   else
   {
      if ( (ret = MEI_AtmOamCreateDevCntrl(pMeiDynCntrl, &pAtmOamDevCntrl)) != IFX_SUCCESS)
      {
         goto MEI_ATMOAM_IOCTL_DRV_INIT_EXIT_DRV;
      }
      pMeiDev->pAtmOamDevCntrl = pAtmOamDevCntrl;
   }
   MEI_DRV_RELEASE_UNIQUE_DRIVER_ACCESS(pMeiDev);

   return ret;

MEI_ATMOAM_IOCTL_DRV_INIT_EXIT_DRV:
   MEI_DRV_RELEASE_UNIQUE_DRIVER_ACCESS(pMeiDev);

   PRN_ERR_USR_NL( MEI_ATMOAM, MEI_DRV_PRN_LEVEL_ERR,
          ("MEI_ATM[%02d - %02d]: ERROR ioctl(FIO_MEI_ATMOAM_INIT) "
           "-  allocate control" MEI_DRV_CRLF
            , MEI_DRV_LINENUM_GET(pMeiDev), pMeiDynCntrl->openInstance));

   return ret;
}

/**
   ioctl-function for enable the ATM OAM access.
   Setup the ATM OAM access feature within the VRX device.

\param
   pMeiDynCntrl    Private dynamic device data (per open instance). [I]
\param
   pUserRemDevInit   Points to the enable struct. [I]

\return
   0 (IFX_SUCCESS) if success
   negative value in case of error
*/
IFX_int32_t MEI_ATMOAM_IoctlCntrl(
                              MEI_DYN_CNTRL_T           *pMeiDynCntrl,
                              IOCTL_MEI_ATMOAM_cntrl_t  *pAtmOamCntrl )
{
   IFX_int32_t                ret = IFX_ERROR;
   MEI_DEV_T                *pMeiDev = pMeiDynCntrl->pMeiDev;
   MEI_ATMOAM_DEV_CNTRL_T   *pAtmOamDevCntrl = IFX_NULL;

   if (pMeiDev->pAtmOamDevCntrl)
   {
      pAtmOamDevCntrl = pMeiDev->pAtmOamDevCntrl;
   }
   else
   {
      PRN_ERR_USR_NL( MEI_ATMOAM, MEI_DRV_PRN_LEVEL_ERR,
             ("MEI_ATM[%02d - %02d]: ERROR ioctl(FIO_MEI_ATMOAM_CNTRL) "
              "- missing init" MEI_DRV_CRLF,
               MEI_DRV_LINENUM_GET(pMeiDev), pMeiDynCntrl->openInstance));

      return -e_MEI_ERR_REM_SETUP;
   }

   if ( (MEI_DRV_STATE_GET(pMeiDev) != e_MEI_DRV_STATE_DFE_READY) ||
        (MEI_DevCntlMsgAvailable(
            pMeiDynCntrl, (IFX_uint32_t)MEI_DRV_CMD_ATMINSERTEXTRACT_CONTROL) != IFX_SUCCESS) )
   {
      return -e_MEI_ERR_REM_INVAL_LINE_STATE;
   }

   switch(pAtmOamCntrl->aoOpMode)
   {
      case MEI_ATMOAM_OPERATION_MODE_DIRECT:
         pAtmOamCntrl->aoTransMode  &= MEI_ATMOAM_INIT_TRANS_MODE_ALL;
         pAtmOamCntrl->aoCntrl      &= MEI_ATMOAM_ENABLE_CNTRL_EVT_ALL;
         break;

      case MEI_ATMOAM_OPERATION_MODE_AUTO:
         pAtmOamCntrl->aoTransMode  = MEI_ATMOAM_INIT_TRANS_MODE_NONE;
         pAtmOamCntrl->aoCntrl      = MEI_ATMOAM_ENABLE_CNTRL_EVT_ALL;
         break;

      default:
         PRN_ERR_USR_NL( MEI_ATMOAM, MEI_DRV_PRN_LEVEL_ERR,
                ("MEI_ATM[%02d - %02d]: ERROR ioctl(FIO_MEI_ATMOAM_CNTRL) "
                 "- invalid operation mode" MEI_DRV_CRLF,
                 MEI_DRV_LINENUM_GET(pMeiDev), pMeiDynCntrl->openInstance));
         return -e_MEI_ERR_REM_SETUP;
   }

   MEI_ATMOAM_GET_UNIQUE_ACCESS(pAtmOamDevCntrl);

   pAtmOamDevCntrl->atmOamOpMode    = (IFX_uint32_t)pAtmOamCntrl->aoOpMode;
   pAtmOamDevCntrl->atmOamTransMode = (IFX_uint32_t)pAtmOamCntrl->aoTransMode;
   pAtmOamDevCntrl->bEnInsExt       =
      (pAtmOamCntrl->aoCntrl & MEI_ATMOAM_ENABLE_CNTRL_GL_EN_FLAG)  ? IFX_TRUE : IFX_FALSE;
   pAtmOamDevCntrl->bEnAlmOnExtract =
      (pAtmOamCntrl->aoCntrl & MEI_ATMOAM_ENABLE_CNTRL_ALM_EN_FLAG) ? IFX_TRUE : IFX_FALSE;
   pAtmOamDevCntrl->bEnEvtOnInsert  =
      (pAtmOamCntrl->aoCntrl & MEI_ATMOAM_ENABLE_CNTRL_EVT_EN_FLAG) ? IFX_TRUE : IFX_FALSE;

   MEI_ATMOAM_CFG_STATE_SET(pAtmOamDevCntrl, eMEI_ATMOAM_OP_CFG_VALID);
   MEI_ATMOAM_TXBUF_STATE_SET(pAtmOamDevCntrl, eMEI_ATMOAM_OP_TX_BUF_FREE);

   MEI_ATMOAM_RELEASE_UNIQUE_ACCESS(pAtmOamDevCntrl);

   if ( (ret = MEI_AtmOamControlEnable(pMeiDynCntrl, pMeiDev)) != IFX_SUCCESS )
   {
      return ret;
   }

   PRN_DBG_USR( MEI_ATMOAM, MEI_DRV_PRN_LEVEL_LOW, MEI_DRV_LINENUM_GET(pMeiDev),
          ("MEI_ATM[%02d - %02d]: ioctl(FIO_MEI_ATMOAM_CNTRL) "
           "- done, cfgSt = %d, txSt = %d " MEI_DRV_CRLF,
            MEI_DRV_LINENUM_GET(pMeiDev), pMeiDynCntrl->openInstance,
            MEI_ATMOAM_CFG_STATE_GET(pAtmOamDevCntrl), MEI_ATMOAM_TXBUF_STATE_GET(pAtmOamDevCntrl)));

   return IFX_SUCCESS;
}

/**
   ioctl-function for request the ATM OAM counter from the VRX.

\param
   pMeiDynCntrl    Private dynamic device data (per open instance). [I]
\param
   pAtmOamStats      Points to the ATM OAM counter struct. [O]

\return
   0 (IFX_SUCCESS) if success
   negative value in case of error
*/
IFX_int32_t MEI_ATMOAM_IoctlCounterGet(
                              MEI_DYN_CNTRL_T            *pMeiDynCntrl,
                              IOCTL_MEI_ATMOAM_counter_t *pAtmOamStats )
{
   IFX_int32_t                ret = IFX_ERROR;
   MEI_DEV_T                *pMeiDev = pMeiDynCntrl->pMeiDev;
   MEI_ATMOAM_DEV_CNTRL_T   *pAtmOamDevCntrl = IFX_NULL;

   if (pMeiDev->pAtmOamDevCntrl)
   {
      pAtmOamDevCntrl = pMeiDev->pAtmOamDevCntrl;
   }
   else
   {
      PRN_ERR_USR_NL( MEI_ATMOAM, MEI_DRV_PRN_LEVEL_ERR,
             ("MEI_ATM[%02d - %02d]: ERROR ioctl(FIO_MEI_ATMOAM_REQ_DEV_COUNTER) "
              "- missing init" MEI_DRV_CRLF
               , MEI_DRV_LINENUM_GET(pMeiDev), pMeiDynCntrl->openInstance));

      return -e_MEI_ERR_REM_SETUP;
   }

   MEI_ATMOAM_GET_UNIQUE_ACCESS(pAtmOamDevCntrl);

   MEI_ATMOAM_UpdateStates(pMeiDev);

   if (MEI_ATMOAM_CFG_STATE_GET(pAtmOamDevCntrl) == eMEI_ATMOAM_OP_CFG_WRITTEN)
   {
      /* send the request to the device */
      ret = MEI_ATMOAM_CMD_InsExtStatsGet( pMeiDynCntrl,
                                             pAtmOamDevCntrl,
                                             pAtmOamStats);
   }
   else
   {
      PRN_ERR_USR_NL( MEI_ATMOAM, MEI_DRV_PRN_LEVEL_ERR,
             ("MEI_ATM[%02d - %02d]: ERROR ioctl(FIO_MEI_ATMOAM_REQ_DEV_COUNTER) "
              "- not configured" MEI_DRV_CRLF
               , MEI_DRV_LINENUM_GET(pMeiDev), pMeiDynCntrl->openInstance));

      ret = -e_MEI_ERR_INVAL_CONFIG;
   }

   MEI_ATMOAM_RELEASE_UNIQUE_ACCESS(pAtmOamDevCntrl);

   return ret;
}


/**
   ioctl-function for request the ATM OAM counter from the VRX.

\param
   pMeiDynCntrl    Private dynamic device data (per open instance). [I]
\param
   pAtmOamStats      Points to the ATM OAM counter struct. [O]

\return
   0 (IFX_SUCCESS) if success
   negative value in case of error
*/
IFX_int32_t MEI_ATMOAM_IoctlStatusGet(
                              MEI_DYN_CNTRL_T           *pMeiDynCntrl,
                              IOCTL_MEI_ATMOAM_status_t *pAtmOamStatus )
{
   IFX_int32_t                ret = IFX_SUCCESS;
   MEI_DEV_T                *pMeiDev = pMeiDynCntrl->pMeiDev;
   MEI_ATMOAM_DEV_CNTRL_T   *pAtmOamDevCntrl = IFX_NULL;

#if (MEI_SUPPORT_ATM_OAM_STATISTICS == 1)
   MEI_ATMOAM_DRV_STATS_T        *pAtmOamStats;
   IOCTL_MEI_ATMOAM_statistics_t *pIoctlStats;
#endif

   if (pMeiDev->pAtmOamDevCntrl)
   {
      pAtmOamDevCntrl = pMeiDev->pAtmOamDevCntrl;
   }
   else
   {
      PRN_ERR_USR_NL( MEI_ATMOAM, MEI_DRV_PRN_LEVEL_ERR,
             ("MEI_ATM[%02d - %02d]: ERROR ioctl(FIO_MEI_ATMOAM_REQ_DRV_STATUS) "
              "- missing init" MEI_DRV_CRLF
               , MEI_DRV_LINENUM_GET(pMeiDev), pMeiDynCntrl->openInstance));

      return -e_MEI_ERR_REM_SETUP;
   }

   MEI_ATMOAM_GET_UNIQUE_ACCESS(pAtmOamDevCntrl);

   pAtmOamStatus->opMode = pAtmOamDevCntrl->atmOamOpMode;
   pAtmOamStatus->transparentMode = pAtmOamDevCntrl->atmOamTransMode;

   pAtmOamStatus->deviceCntrl = 0;
   if (pAtmOamDevCntrl->bEnInsExt)
      pAtmOamStatus->deviceCntrl |= MEI_ATMOAM_ENABLE_CNTRL_GL_EN_FLAG;

   if (pAtmOamDevCntrl->bEnAlmOnExtract)
      pAtmOamStatus->deviceCntrl |= MEI_ATMOAM_ENABLE_CNTRL_ALM_EN_FLAG;

   if (pAtmOamDevCntrl->bEnEvtOnInsert)
      pAtmOamStatus->deviceCntrl |= MEI_ATMOAM_ENABLE_CNTRL_EVT_EN_FLAG;

   pAtmOamStatus->drvCfgState = MEI_ATMOAM_CFG_STATE_GET(pAtmOamDevCntrl);
   pAtmOamStatus->devTxBufState = MEI_ATMOAM_TXBUF_STATE_GET(pAtmOamDevCntrl);

   memset(&pAtmOamStatus->statistics, 0x00, sizeof(IOCTL_MEI_ATMOAM_statistics_t));

#if (MEI_SUPPORT_ATM_OAM_STATISTICS == 1)
   pAtmOamStats = &pAtmOamDevCntrl->statistics;
   pIoctlStats = &pAtmOamStatus->statistics;

   pIoctlStats->instMsgCnt       = (unsigned int)pAtmOamStats->instMsgCnt;
   pIoctlStats->instMsgErrCnt    = (unsigned int)pAtmOamStats->instMsgErrCnt;
   pIoctlStats->instMsgNfcCnt    = (unsigned int)pAtmOamStats->instMsgNfcCnt;
   pIoctlStats->instCellCnt      = (unsigned int)pAtmOamStats->instCellCnt;
   pIoctlStats->extrMsgCnt       = (unsigned int)pAtmOamStats->extrMsgCnt;
   pIoctlStats->extrCellCnt      = (unsigned int)pAtmOamStats->extrCellCnt;
   pIoctlStats->extrMsgErrCnt    = (unsigned int)pAtmOamStats->extrMsgErrCnt;
   pIoctlStats->extrCellErrCnt   = (unsigned int)pAtmOamStats->extrCellErrCnt;
   pIoctlStats->almMsgCnt        = (unsigned int)pAtmOamStats->almMsgCnt;
   pIoctlStats->almMsgExtrFailCellCnt = (unsigned int)pAtmOamStats->almMsgExtrFailCellCnt;
#endif

   MEI_ATMOAM_RELEASE_UNIQUE_ACCESS(pAtmOamDevCntrl);

   return ret;
}

/**
   ???

\param
   pMeiDynCntrl    Private dynamic device data (per open instance). [I]
\param
   pAtmOamCells      Points to the ATM OAM cells to send. [I]

\return
   0 (IFX_SUCCESS) if success
   negative value in case of error
*/
IFX_int32_t MEI_ATMOAM_IoctlCellInsert(
                              MEI_DYN_CNTRL_T                *pMeiDynCntrl,
                              IOCTL_MEI_ATMOAM_drvAtmCells_t *pAtmOamCells )
{
   IFX_int32_t                ret = IFX_ERROR;
   MEI_DEV_T                *pMeiDev = pMeiDynCntrl->pMeiDev;
   MEI_ATMOAM_DEV_CNTRL_T   *pAtmOamDevCntrl = IFX_NULL;

   if (pMeiDev->pAtmOamDevCntrl)
   {
      pAtmOamDevCntrl = pMeiDev->pAtmOamDevCntrl;
   }
   else
   {
      PRN_ERR_USR_NL( MEI_ATMOAM, MEI_DRV_PRN_LEVEL_ERR,
             ("MEI_ATM[%02d - %02d]: ERROR ioctl(FIO_MEI_ATMOAM_CELL_INSERT) "
              "- missing init" MEI_DRV_CRLF
               , MEI_DRV_LINENUM_GET(pMeiDev), pMeiDynCntrl->openInstance));

      return -e_MEI_ERR_REM_SETUP;
   }


   MEI_ATMOAM_GET_UNIQUE_ACCESS(pAtmOamDevCntrl);

   MEI_ATMOAM_UpdateStates(pMeiDev);

   if (MEI_ATMOAM_CFG_STATE_GET(pAtmOamDevCntrl) != eMEI_ATMOAM_OP_CFG_WRITTEN)
   {
      PRN_ERR_USR_NL( MEI_ATMOAM, MEI_DRV_PRN_LEVEL_ERR,
             ("MEI_ATM[%02d - %02d]: ERROR ioctl(FIO_MEI_ATMOAM_CELL_INSERT) "
              "- not configured" MEI_DRV_CRLF
               , MEI_DRV_LINENUM_GET(pMeiDev), pMeiDynCntrl->openInstance));

      ret = -e_MEI_ERR_INVAL_CONFIG;
      goto MEI_ATMOAM_IOCTLCELLINSERT_ERR;
   }

   if ( MEI_AtmOamWaitForTxBuffer(
         pMeiDynCntrl, pAtmOamDevCntrl, MEI_ATMOAM_MAX_INSSTATUS_POLL_RETRY) != IFX_SUCCESS)
   {
      PRN_ERR_USR_NL( MEI_ATMOAM, MEI_DRV_PRN_LEVEL_ERR,
             ("MEI_ATM[%02d]: ERROR - timeout wait for TX buf (state = %d) before" MEI_DRV_CRLF,
              MEI_DRV_LINENUM_GET(pMeiDev), MEI_ATMOAM_TXBUF_STATE_GET(pAtmOamDevCntrl)));

      /* reset the TX buffer for next trail */
      MEI_ATMOAM_TXBUF_STATE_SET(pAtmOamDevCntrl, eMEI_ATMOAM_OP_TX_BUF_FREE);

      ret = IFX_ERROR;
      goto MEI_ATMOAM_IOCTLCELLINSERT_ERR;
   }

   PRN_DBG_USR( MEI_ATMOAM, MEI_DRV_PRN_LEVEL_LOW, MEI_DRV_LINENUM_GET(pMeiDev),
          ("MEI_ATM[%02d]: Insert ATM[%d] - [0]: 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X" MEI_DRV_CRLF,
           MEI_DRV_LINENUM_GET(pMeiDev), pAtmOamCells->cellCount,
           pAtmOamCells->atmCells[0].ATMcell[0], pAtmOamCells->atmCells[0].ATMcell[1],
           pAtmOamCells->atmCells[0].ATMcell[2], pAtmOamCells->atmCells[0].ATMcell[3],
           pAtmOamCells->atmCells[0].ATMcell[4], pAtmOamCells->atmCells[0].ATMcell[4] ));

   /* send the request to the device */
   if ( (ret =MEI_ATMOAM_CMD_AtmCellLineInsert(
                                       pMeiDynCntrl,
                                       pAtmOamDevCntrl,
                                       pAtmOamCells->atmCells,
                                       (IFX_uint32_t)pAtmOamCells->cellCount))
        != IFX_SUCCESS)
   {
      goto MEI_ATMOAM_IOCTLCELLINSERT_ERR;
   }

   /* wait for notification for transmit done */
   if ( MEI_AtmOamWaitForTxBuffer(
         pMeiDynCntrl, pAtmOamDevCntrl, MEI_ATMOAM_MAX_INSSTATUS_POLL_RETRY) != IFX_SUCCESS)
   {
      PRN_ERR_USR_NL( MEI_ATMOAM, MEI_DRV_PRN_LEVEL_ERR,
             ("MEI_ATM[%02d]: ERROR - timeout wait for TX buf (state = %d) <after>" MEI_DRV_CRLF,
              MEI_DRV_LINENUM_GET(pMeiDev), MEI_ATMOAM_TXBUF_STATE_GET(pAtmOamDevCntrl)));

      /* reset the TX buffer for next trail */
      MEI_ATMOAM_TXBUF_STATE_SET(pAtmOamDevCntrl, eMEI_ATMOAM_OP_TX_BUF_FREE);

      ret = IFX_ERROR;
      goto MEI_ATMOAM_IOCTLCELLINSERT_ERR;
   }

   ret = IFX_SUCCESS;

MEI_ATMOAM_IOCTLCELLINSERT_ERR:

   MEI_ATMOAM_RELEASE_UNIQUE_ACCESS(pAtmOamDevCntrl);
   return ret;
}

/**
   Check the ATM OAM config if processing is neccessary

\param
   pMeiDev         Points to the VRX driver device data. [I]
\param
   pAtmOamDevCntrl   Points to the ATM OAM control struct. [I]
\param
   msgId             Message ID of the reveived modem message. [I]

\return
   IFX_TRUE  - work neccessary (no transparent mode).
   IFX_FALSE - no work neccessary, forward to upper layer (transparent mode).
*/
IFX_boolean_t MEI_ATMOAM_CheckForWork(
                              MEI_DEV_T              *pMeiDev,
                              MEI_ATMOAM_DEV_CNTRL_T *pAtmOamDevCntrl,
                              IFX_uint16_t             msgId)
{
   IFX_boolean_t              bDoProcessing = IFX_FALSE;

   /* check - ATM OAM block available */
   if (!pAtmOamDevCntrl)
   {
      return IFX_FALSE;
   }

   /* check - if transparent mode --> no ATM OAM processing */
   switch (msgId)
   {
      case MEI_DRV_EVT_ATMCELLLINEINSERTSTATUSGET:
         bDoProcessing = MEI_ATMOAM_CHECK_TRANSMODE_EVT_TX(pAtmOamDevCntrl) ? IFX_FALSE : IFX_TRUE;
         break;
      case MEI_DRV_EVT_ATMCELLLINEEXTRACT:
         bDoProcessing = MEI_ATMOAM_CHECK_TRANSMODE_CELL_EXTR(pAtmOamDevCntrl) ? IFX_FALSE : IFX_TRUE;
         break;
      case MEI_DRV_ALM_ATMCELLEXTRACTFAILED:
         bDoProcessing = MEI_ATMOAM_CHECK_TRANSMODE_ALM_EXTR(pAtmOamDevCntrl) ? IFX_FALSE : IFX_TRUE;
         break;
      default:
         break;
   }

#if 0
   PRN_DBG_INT( MEI_ATMOAM, MEI_DRV_PRN_LEVEL_LOW, MEI_DRV_LINENUM_GET(pMeiDev),
          ("MEI_ATM[%02d]: AutoMsg - msgId = 0x%04X work = %d" MEI_DRV_CRLF,
           MEI_DRV_LINENUM_GET(pMeiDev), msgId, (bDoProcessing) ? 1 : 0));
#endif

   return bDoProcessing;
}


/**
   ATM OAM Autonomous Message Handler.

\param
   pMeiDev      Points to the VRX driver device data. [I]
\param
   msgId          Message ID of the reveived modem message. [I]
\param
   pModemMsg      Points to the reveived modem message. [I]

\return
   0 (IFX_SUCCESS) if success
   negative value in case of error

*/
IFX_int32_t MEI_ATMOAM_AutoMsgHandler(
                              MEI_DEV_T       *pMeiDev,
                              IFX_uint16_t      msgId,
                              CMV_STD_MESSAGE_T *pModemMsg)
{
   IFX_int32_t    atmOamMsgSize, retVal = IFX_ERROR;
   IFX_uint32_t   distCount = 0;
   MEI_ATMOAM_DEV_CNTRL_T   *pAtmOamDevCntrl = pMeiDev->pAtmOamDevCntrl;

#if 0
   PRN_DBG_INT( MEI_ATMOAM, MEI_DRV_PRN_LEVEL_LOW, MEI_DRV_LINENUM_GET(pMeiDev),
          ("MEI_ATM[%02d]: AutoMsg - msgId = 0x%04X" MEI_DRV_CRLF,
           MEI_DRV_LINENUM_GET(pMeiDev), P_CMV_MSGHDR_MSGID_GET(pModemMsg)));
#endif

   switch (P_CMV_MSGHDR_MSGID_GET(pModemMsg))
   {
      case MEI_DRV_EVT_ATMCELLLINEINSERTSTATUSGET:
         {
            retVal = MEI_ATMOAM_doEVT_AtmCellLineInsertStatus(
                                                   pMeiDev,
                                                   pAtmOamDevCntrl,
                                                   pModemMsg);
         }
         break;

      case MEI_DRV_EVT_ATMCELLLINEEXTRACT:
         {
            retVal = MEI_ATMOAM_doEVT_AtmCellExtract(
                                                   pMeiDev,
                                                   pAtmOamDevCntrl,
                                                   pModemMsg,
                                                   &pAtmOamDevCntrl->rxAtmCells,
                                                   MEI_ATMOAM_MAX_RX_CELL_CNT);
            if ( (retVal == IFX_SUCCESS) &&
                 (pAtmOamDevCntrl->rxAtmCells.cellCnt) )
            {
               atmOamMsgSize = ( (pAtmOamDevCntrl->rxAtmCells.cellCnt * sizeof(IOCTL_MEI_ATMOAM_rawCell_t))
                                 + sizeof(pAtmOamDevCntrl->rxAtmCells.cellCnt)
                                 + sizeof(pAtmOamDevCntrl->rxAtmCells.atmOamId) );

               distCount = MEI_DistributeAutoMsg(
                              pMeiDev, pMeiDev->pRootNfcRecvFirst,
                              (IFX_uint8_t *)&pAtmOamDevCntrl->rxAtmCells,
                              atmOamMsgSize, MEI_RECV_BUF_CTRL_MODEM_ATMOAM_CELL);
               if (distCount <= 0)
               {
                  PRN_DBG_USR( MEI_ATMOAM, MEI_DRV_PRN_LEVEL_NORMAL, MEI_DRV_LINENUM_GET(pMeiDev),
                         ("MEI_ATM[%02d]: WARNING - "
                          "no waiting user, discard driver ATM OAM auto. msg (cell count %d)!" MEI_DRV_CRLF,
                           MEI_DRV_LINENUM_GET(pMeiDev),
                           pAtmOamDevCntrl->rxAtmCells.cellCnt));
               }
            }
         }
         break;

      case MEI_DRV_ALM_ATMCELLEXTRACTFAILED:
         {
            IFX_uint32_t linkNo, failCount;

            retVal = MEI_ATMOAM_doALM_AtmCellExtractFailed(
                        pMeiDev, pAtmOamDevCntrl, pModemMsg,
                        &linkNo, &failCount);

            #if (MEI_SUPPORT_DRIVER_MSG == 1)
            distCount = MEI_DrvMsg_RemAtmOamDistribute(
                           pMeiDev, linkNo, failCount);
            if (distCount <= 0)
            {
               PRN_DBG_USR( MEI_ATMOAM, MEI_DRV_PRN_LEVEL_NORMAL, MEI_DRV_LINENUM_GET(pMeiDev),
                      ("MEI_ATM[%02d]: WARNING - "
                       "no waiting user, discard driver ATM OAM auto. Alarm msg (failCnt %d)!" MEI_DRV_CRLF,
                        MEI_DRV_LINENUM_GET(pMeiDev), failCount));
            }
            #else
            distCount = 0;
            #endif
         }
         break;

      default:
         break;
   }

   return retVal;
}


/**
   ATM OAM Reset the current states.

\param
   pMeiDev      Points to the VRX driver device data. [I]

\return
   0 (IFX_SUCCESS) if success
   negative value in case of error

*/
IFX_int32_t MEI_ATMOAM_ResetControl(
                              MEI_DEV_T       *pMeiDev)
{
   MEI_ATMOAM_DEV_CNTRL_T   *pAtmOamDevCntrl = pMeiDev->pAtmOamDevCntrl;

   if (!pAtmOamDevCntrl)
      return IFX_SUCCESS;

   if (MEI_ATMOAM_CFG_STATE_GET(pAtmOamDevCntrl) != eMEI_ATMOAM_OP_CFG_INITIAL)
   {
      MEI_ATMOAM_CFG_STATE_SET(pAtmOamDevCntrl, eMEI_ATMOAM_OP_CFG_VALID);
   }

   if (MEI_ATMOAM_TXBUF_STATE_GET(pAtmOamDevCntrl) != eMEI_ATMOAM_OP_TX_BUF_INITIAL)
   {
      MEI_ATMOAM_TXBUF_STATE_SET(pAtmOamDevCntrl, eMEI_ATMOAM_OP_TX_BUF_FREE);
   }

   return IFX_SUCCESS;
}


#endif      /* #if (MEI_DRV_ATM_OAM_ENABLE == 1) */

