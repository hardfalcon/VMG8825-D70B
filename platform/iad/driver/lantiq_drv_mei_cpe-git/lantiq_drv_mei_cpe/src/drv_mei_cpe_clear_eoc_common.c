/******************************************************************************

                          Copyright (c) 2007-2015
                     Lantiq Beteiligungs-GmbH & Co. KG

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/


/* ==========================================================================
   Description : Clear EOC Common functions for the VRX Driver
   ========================================================================== */

/* ============================================================================
   Includes
   ========================================================================= */

/* get at first the driver configuration */
#include "drv_mei_cpe_config.h"

#if (MEI_DRV_CLEAR_EOC_ENABLE == 1)

#include "ifx_types.h"
#include "drv_mei_cpe_os.h"
#include "drv_mei_cpe_dbg.h"
#include "drv_mei_cpe_dbg_driver.h"

#include "drv_mei_cpe_clear_eoc.h"
#include "drv_mei_cpe_api.h"
#include "drv_mei_cpe_interface.h"

#include "drv_mei_cpe_clear_eoc_common.h"
#include "drv_mei_cpe_device_cntrl.h"
#include "drv_mei_cpe_msg_process.h"


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
MEI_STATIC IFX_int32_t MEI_CEocCreateDevCntrl(
                              MEI_DYN_CNTRL_T      *pMeiDynCntrl,
                              MEI_CEOC_DEV_CNTRL_T **ppCEocDevCntrl);

MEI_STATIC IFX_boolean_t MEI_CEocDevBufCheck(
                              MEI_CEOC_DEV_CNTRL_T  *pCEocDevCntrl,
                              IFX_boolean_t           bTxState);

MEI_STATIC IFX_int32_t MEI_CEocWaitForDevice(
                              MEI_DYN_CNTRL_T      *pMeiDynCntrl,
                              MEI_CEOC_DEV_CNTRL_T *pCEocDevCntrl,
                              IFX_boolean_t          bTxState);

MEI_STATIC IFX_int32_t MEI_CEocFrameSend(
                              MEI_DYN_CNTRL_T         *pMeiDynCntrl,
                              MEI_CEOC_DEV_CNTRL_T    *pCEocDevCntrl,
                              MEI_CEOC_FRAME_BUFFER_T *pCEocFrameBuf);

MEI_STATIC IFX_int32_t MEI_CEocFrameRead(
                              MEI_DYN_CNTRL_T         *pMeiDynCntrl,
                              MEI_CEOC_DEV_CNTRL_T    *pCEocDevCntrl,
                              MEI_CEOC_FRAME_BUFFER_T *pCEocFrameBuf);
/* ============================================================================
   Global Variable Definition
   ========================================================================= */

/* VRX-Driver: Access module - create print level variable */
MEI_DRV_PRN_USR_MODULE_CREATE(MEI_CEOC, MEI_DRV_PRN_LEVEL_HIGH);
MEI_DRV_PRN_INT_MODULE_CREATE(MEI_CEOC, MEI_DRV_PRN_LEVEL_HIGH);


/* ============================================================================
   Local Variable Definition
   ========================================================================= */


/* ============================================================================
   Local Function Definition
   ========================================================================= */

/**
   Allocate the Cear EOC Access Control structure.

\param
   pMeiDynCntrl    private dynamic device data (per open instance) [I]
\param
   ppCEocDevCntrl    points to the Clear EOC control struct pointer [IO]
                     In case of success returns pointer to the allocated structure.
\return
   0 (IFX_SUCCESS) if success.
   negative value in case of error.
      -e_MEI_ERR_DEV_INIT: invalid argument
      -e_MEI_ERR_ALREADY_DONE: allocation already done (ptr not null)
      -e_MEI_ERR_NO_MEM: not enough memory
*/
MEI_STATIC IFX_int32_t MEI_CEocCreateDevCntrl(
                              MEI_DYN_CNTRL_T      *pMeiDynCntrl,
                              MEI_CEOC_DEV_CNTRL_T **ppCEocDevCntrl)
{
   MEI_CEOC_DEV_CNTRL_T *pCEocDevCntrl;

   if (ppCEocDevCntrl == IFX_NULL)
   {
      PRN_ERR_USR_NL( MEI_CEOC, MEI_DRV_PRN_LEVEL_ERR,
             ("MEI_EOC[%02d]: ERROR - invalid arg for create Clear EOC control" MEI_DRV_CRLF
               , MEI_DRV_DYN_LINENUM_GET(pMeiDynCntrl)));

      return -e_MEI_ERR_DEV_INIT;
   }

   if (*ppCEocDevCntrl != IFX_NULL)
   {
      PRN_ERR_USR_NL( MEI_CEOC, MEI_DRV_PRN_LEVEL_ERR,
             ("MEI_EOC[%02d]: ERROR - Clear EOC control already exist" MEI_DRV_CRLF
               , MEI_DRV_DYN_LINENUM_GET(pMeiDynCntrl)) );

      return -e_MEI_ERR_ALREADY_DONE;
   }

   pCEocDevCntrl = (MEI_CEOC_DEV_CNTRL_T *)MEI_DRVOS_Malloc(sizeof(MEI_CEOC_DEV_CNTRL_T));
   if (!pCEocDevCntrl)
   {
      PRN_ERR_USR_NL( MEI_CEOC, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_EOC[%02d]: no memory for Clear EOC control structure." MEI_DRV_CRLF
             , MEI_DRV_DYN_LINENUM_GET(pMeiDynCntrl)));

      return -e_MEI_ERR_NO_MEM;
   }

   memset( (char*)pCEocDevCntrl, 0x00, sizeof(MEI_CEOC_DEV_CNTRL_T) );

   /*
      init struct
   */
   MEI_DRVOS_SemaphoreInit(&pCEocDevCntrl->pDevCEocCtrlRWlock);

   MEI_DRVOS_EventInit(&pCEocDevCntrl->eventCEocStateChange);
   pCEocDevCntrl->bCEocStateChangeNeedWakeUp = IFX_FALSE;

   MEI_CEOC_CFG_STATE_SET(pCEocDevCntrl, eMEI_CEOC_OP_CFG_INITIAL);

   MEI_CEOC_TX_DEV_BUF_STATE_SET(pCEocDevCntrl, eMEI_CEOC_TX_DEV_BUF_STATE_IDLE);
   MEI_CEOC_RX_DEV_BUF_STATE_SET(pCEocDevCntrl, eMEI_CEOC_RX_DEV_BUF_STATE_IDLE);

   *ppCEocDevCntrl = pCEocDevCntrl;

   return IFX_SUCCESS;
}

/**
   Check and interprete the current TX / RX device buffer state

\param
   pCEocDevCntrl  points to the Clear EOC control struct pointer [I]
\param
   bTxState       if set check the TX state, else check the RX state [I]

\return
   IFX_TRUE  - state change occurred or ERROR
   IFX_FALSE - no state change occured.
*/
MEI_STATIC IFX_boolean_t MEI_CEocDevBufCheck(
                              MEI_CEOC_DEV_CNTRL_T  *pCEocDevCntrl,
                              IFX_boolean_t           bTxState)
{
   if (bTxState)
   {
      if (   (MEI_CEOC_TX_DEV_BUF_STATE_GET(pCEocDevCntrl) == eMEI_CEOC_TX_DEV_BUF_STATE_IDLE)
          || (MEI_CEOC_TX_DEV_BUF_STATE_GET(pCEocDevCntrl) == eMEI_CEOC_TX_DEV_BUF_STATE_ERROR) )
      {
         return IFX_TRUE;
      }
   }
   else
   {
      if (   (MEI_CEOC_RX_DEV_BUF_STATE_GET(pCEocDevCntrl) == eMEI_CEOC_RX_DEV_BUF_STATE_DONE)
          || (MEI_CEOC_RX_DEV_BUF_STATE_GET(pCEocDevCntrl) == eMEI_CEOC_RX_DEV_BUF_STATE_ERROR) )
      {
         return IFX_TRUE;
      }
   }

   return IFX_FALSE;
}

/**
   Wait /check TX/RX Buffer states for a EOC operation.

\param
   pMeiDynCntrl private dynamic device data (per open instance) [I]
\param
   pCEocDevCntrl  points to the Clear EOC control struct pointer [I]
\param
   bTxState       if set check the TX buffer state, else check the RX buffer state [I]

\return
   IFX_SUCCESS - TX Buffer is free for TX send opertaion
                 RX Buffer has data for RX read operation.
   IFX_ERROR   - Error occurred, requested op state not reached.
*/
MEI_STATIC IFX_int32_t MEI_CEocWaitForDevice(
                              MEI_DYN_CNTRL_T       *pMeiDynCntrl,
                              MEI_CEOC_DEV_CNTRL_T  *pCEocDevCntrl,
                              IFX_boolean_t           bTxState)
{
   MEI_DEV_T   *pMeiDev = pMeiDynCntrl->pMeiDev;

   if (    ( bTxState && (!pCEocDevCntrl->bEnEvtTxStatus))
        || (!bTxState && (!pCEocDevCntrl->bEnEvtRxStatus)) )
   {
      /* poll status */
      MEI_CEOC_SET_TIMEOUT_CNT( pCEocDevCntrl, MEI_CEOC_MAX_STATUS_POLL_COUNT);

      while(MEI_CEOC_GET_TIMEOUT_CNT(pCEocDevCntrl) > 0)
      {
         if ( MEI_CEOC_CMD_ClearEOCStatusGet(
                  pMeiDynCntrl,pCEocDevCntrl) != IFX_SUCCESS)
         {
            return IFX_ERROR;
         }

         if (MEI_CEocDevBufCheck(pCEocDevCntrl, bTxState) == IFX_TRUE)
            break;

         MEI_DRVOS_EventWait_timeout(
                     &pCEocDevCntrl->eventCEocStateChange,
                     MEI_CEOC_MIN_STATE_CHANGE_POLL_TIME_MS);
         MEI_CEOC_DEC_TIMEOUT_CNT(pCEocDevCntrl);
      }
   }
   else
   {
      if (pMeiDev->eModePoll == e_MEI_DEV_ACCESS_MODE_PASSIV_POLL)
      {
         MEI_PollIntPerVrxLine(pMeiDev, e_MEI_DEV_ACCESS_MODE_PASSIV_POLL);

         MEI_CEOC_SET_TIMEOUT_CNT( pCEocDevCntrl,
               MEI_CEOC_FRAME_TRANS_DONE_TIMEOUT_MS / MEI_CEOC_MIN_STATE_CHANGE_POLL_TIME_MS);

         while(MEI_CEOC_GET_TIMEOUT_CNT(pCEocDevCntrl) > 0)
         {
            if (MEI_CEocDevBufCheck(pCEocDevCntrl, bTxState) == IFX_TRUE)
               break;

            MEI_DRVOS_EventWait_timeout(
                        &pCEocDevCntrl->eventCEocStateChange,
                        MEI_CEOC_MIN_STATE_CHANGE_POLL_TIME_MS);
            MEI_PollIntPerVrxLine(pMeiDev, e_MEI_DEV_ACCESS_MODE_PASSIV_POLL);
            MEI_CEOC_DEC_TIMEOUT_CNT(pCEocDevCntrl);
         }
      }
      else
      {
         MEI_CEOC_SET_TIMEOUT_CNT( pCEocDevCntrl,
               MEI_CEOC_FRAME_TRANS_DONE_TIMEOUT_MS / MEI_CEOC_MIN_STATE_CHANGE_POLL_TIME_MS);

         while(MEI_CEOC_GET_TIMEOUT_CNT(pCEocDevCntrl) > 0)
         {
            if (MEI_CEocDevBufCheck(pCEocDevCntrl, bTxState) == IFX_TRUE)
               break;

            pCEocDevCntrl->bCEocStateChangeNeedWakeUp = IFX_TRUE;
            MEI_DRVOS_EventWait_timeout(
                        &pCEocDevCntrl->eventCEocStateChange,
                        MEI_CEOC_MIN_STATE_CHANGE_POLL_TIME_MS);
            pCEocDevCntrl->bCEocStateChangeNeedWakeUp = IFX_FALSE;
            MEI_CEOC_DEC_TIMEOUT_CNT(pCEocDevCntrl);
         }
      }
   }

   if (bTxState)
      return (MEI_CEOC_TX_DEV_BUF_STATE_GET(pCEocDevCntrl) == eMEI_CEOC_TX_DEV_BUF_STATE_IDLE) ?
                  IFX_SUCCESS : IFX_ERROR;
   else
      return (MEI_CEOC_RX_DEV_BUF_STATE_GET(pCEocDevCntrl) == eMEI_CEOC_RX_DEV_BUF_STATE_DONE) ?
                  IFX_SUCCESS : IFX_ERROR;
}

/**
   Fragment and write a given Clear EOC frame to the VRX device and
   trigger the VRX to transmit.

\param
   pMeiDynCntrl private dynamic device data (per open instance) [I]
\param
   pCEocDevCntrl  points to the Clear EOC control struct pointer [IO]
\param
   pCEocFrameBuf points to the frame struct to be send.

\return
   IFX_SUCCES in case of successful send frame else
   IFX_ERROR and TX operation state is set.
*/
MEI_STATIC IFX_int32_t MEI_CEocFrameSend(
                              MEI_DYN_CNTRL_T         *pMeiDynCntrl,
                              MEI_CEOC_DEV_CNTRL_T    *pCEocDevCntrl,
                              MEI_CEOC_FRAME_BUFFER_T *pCEocFrameBuf)
{
   IFX_int32_t msgCnt = MEI_CEOC_MAX_MSG_PER_FRAME;

   if (MEI_CEocWaitForDevice(
         pMeiDynCntrl, pCEocDevCntrl, IFX_TRUE) != IFX_SUCCESS)
   {
      PRN_ERR_USR_NL( MEI_CEOC, MEI_DRV_PRN_LEVEL_ERR,
             ("MEI_EOC[%02d - %02d]: WRN - frame send, reset TX buffer state " MEI_DRV_CRLF
               , MEI_DRV_DYN_LINENUM_GET(pMeiDynCntrl), pMeiDynCntrl->openInstance) );
   }

   MEI_CEOC_TX_DEV_BUF_STATE_SET(pCEocDevCntrl, eMEI_CEOC_TX_DEV_BUF_STATE_IDLE);

   /*
      Transfer the frame to the VRX TX EOC buffer
   */
   while( msgCnt &&
          (pCEocFrameBuf->transferedSize_byte < pCEocFrameBuf->frameSize_byte) )
   {
      if( MEI_CEOC_CMD_ClearEOC_Write( pMeiDynCntrl,
                                         pCEocDevCntrl,
                                         pCEocFrameBuf) != IFX_SUCCESS)
      {
         break;
      }
   }

   if (pCEocFrameBuf->transferedSize_byte != pCEocFrameBuf->frameSize_byte)
   {
      /* error send frame */
      PRN_ERR_USR_NL( MEI_CEOC, MEI_DRV_PRN_LEVEL_ERR,
             ("MEI_EOC[%02d - %02d]: ERROR - frame send, size missmatch (Size: %d != tx: %d)" MEI_DRV_CRLF
               , MEI_DRV_DYN_LINENUM_GET(pMeiDynCntrl), pMeiDynCntrl->openInstance
               , pCEocFrameBuf->frameSize_byte, pCEocFrameBuf->transferedSize_byte) );

      return IFX_ERROR;
   }

   /*
      Trigger the VRX to transmit the TX EOC buffer.
   */
   MEI_CEOC_TX_DEV_BUF_STATE_SET(pCEocDevCntrl, eMEI_CEOC_TX_DEV_BUF_STATE_IN_PROGRESS);
   if( MEI_CEOC_CMD_ClearEOC_TxTrigger( pMeiDynCntrl,
                                          pCEocDevCntrl) != IFX_SUCCESS )
   {
      return IFX_ERROR;
   }

   /*
      Wait for the notification from the VRX - transmit done
   */
   if (MEI_CEocWaitForDevice(
         pMeiDynCntrl, pCEocDevCntrl, IFX_TRUE) != IFX_SUCCESS)
   {
      PRN_ERR_USR_NL( MEI_CEOC, MEI_DRV_PRN_LEVEL_ERR,
             ("MEI_EOC[%02d - %02d]: ERROR - frame send, timeout - <trans done>" MEI_DRV_CRLF
               , MEI_DRV_DYN_LINENUM_GET(pMeiDynCntrl), pMeiDynCntrl->openInstance) );

      MEI_CEOC_STAT_INC_INST_TIMEOUT(pCEocDevCntrl);

      return IFX_ERROR;
   }

   MEI_CEOC_TX_DEV_BUF_STATE_SET(pCEocDevCntrl, eMEI_CEOC_TX_DEV_BUF_STATE_IDLE);
   return IFX_SUCCESS;
}


/**
   Read and defragment a received Clear EOC frame from the VRX device.

\param
   pMeiDynCntrl private dynamic device data (per open instance) [I]
\param
   pCEocDevCntrl  points to the Clear EOC control struct pointer [IO]
\param
   pCEocFrameBuf points to the frame struct to put the frame.

\return
   IFX_SUCCES in case of successful send frame else
   IFX_ERROR and TX operation state is set.
*/
MEI_STATIC IFX_int32_t MEI_CEocFrameRead(
                              MEI_DYN_CNTRL_T         *pMeiDynCntrl,
                              MEI_CEOC_DEV_CNTRL_T    *pCEocDevCntrl,
                              MEI_CEOC_FRAME_BUFFER_T *pCEocFrameBuf)
{
   IFX_int32_t msgCnt = MEI_CEOC_MAX_MSG_PER_FRAME;

   /* check for data */
   if (MEI_CEocWaitForDevice(
         pMeiDynCntrl, pCEocDevCntrl, IFX_FALSE) != IFX_SUCCESS)
   {
      PRN_DBG_USR_NL( MEI_CEOC, MEI_DRV_PRN_LEVEL_LOW,
             ("MEI_EOC[%02d - %02d]: frame recv - no data avail" MEI_DRV_CRLF
               , MEI_DRV_DYN_LINENUM_GET(pMeiDynCntrl), pMeiDynCntrl->openInstance) );

      MEI_CEOC_RX_DEV_BUF_STATE_SET(pCEocDevCntrl, eMEI_CEOC_RX_DEV_BUF_STATE_IDLE);
      return IFX_SUCCESS;
   }

   /* data available - read the data */
   while( msgCnt &&
          (pCEocFrameBuf->transferedSize_byte < pCEocFrameBuf->frameSize_byte) )
   {
      if( MEI_CEOC_CMD_ClearEOC_Read( pMeiDynCntrl,
                                        pCEocDevCntrl,
                                        pCEocFrameBuf) != IFX_SUCCESS)
      {
         break;
      }

      msgCnt--;
   }

   if (pCEocFrameBuf->transferedSize_byte == pCEocFrameBuf->frameSize_byte)
   {
      MEI_CEOC_STAT_INC_RD_FRAME_CNT(pCEocDevCntrl);
      return IFX_SUCCESS;
   }
   else
   {
      MEI_CEOC_STAT_INC_RECV_FRAME_ERR_CNT(pCEocDevCntrl);
      return IFX_ERROR;
   }
}


/**
   Read and defragment a received Clear EOC frame from the VRX device.

\param
   pMeiDev      points to the VRX driver private device data [I]
\param
   pCEocDevCntrl  points to the Clear EOC control struct pointer [IO]
\param
   pCEocFrameBuf  points to the frame struct to put the frame.

\return
   IFX_SUCCES in case of successful send frame else
   IFX_ERROR and TX operation state is set.
*/
IFX_int32_t MEI_CEocFrameEvt(
                              MEI_DEV_T             *pMeiDev,
                              MEI_CEOC_DEV_CNTRL_T  *pCEocDevCntrl,
                              CMV_STD_MESSAGE_T       *pModemMsg)
{
   IFX_int32_t                retVal = IFX_SUCCESS, distCount = 0;
   MEI_CEOC_FRAME_BUFFER_T  *pCEocFrameBuf = &pCEocDevCntrl->rxEocFrame;

   if (pCEocFrameBuf->transferedSize_byte == 0)
   {
      /* first msg for this frame - setup frame buffer */
      pCEocFrameBuf->vrxEocFrame.cEocId      = MEI_DRV_MSG_CTRL_IF_MODEM_EOC_FRAME_ON;
      pCEocFrameBuf->vrxEocFrame.length_byte = 0;
      pCEocFrameBuf->frameSize_byte = MEI_CEOC_RAW_EOC_DATA_SIZE_BYTE +
                                      sizeof(pCEocFrameBuf->vrxEocFrame.protIdent) +
                                      sizeof(pCEocFrameBuf->vrxEocFrame.length_byte);
   }


   if ( (retVal = MEI_CEOC_doEVT_ClearEOC_Read(
                     pMeiDev, pCEocDevCntrl, pModemMsg, pCEocFrameBuf)) != IFX_SUCCESS)
   {
      /* error get data */
      pCEocFrameBuf->frameSize_byte          = 0;
      pCEocFrameBuf->vrxEocFrame.cEocId      = 0;
      pCEocFrameBuf->vrxEocFrame.length_byte = 0;
      pCEocFrameBuf->transferedSize_byte     = 0;

      return IFX_ERROR;
   }

   /* check for distribution */
   if (   (pCEocFrameBuf->transferedSize_byte > 0)
       && (pCEocFrameBuf->transferedSize_byte == pCEocFrameBuf->frameSize_byte) )
   {
      /* done ready for distribution */
      distCount = MEI_DistributeAutoMsg(
                     pMeiDev, pMeiDev->pRootNfcRecvFirst,
                     (IFX_uint8_t *)&pCEocFrameBuf->vrxEocFrame,
                     (pCEocFrameBuf->vrxEocFrame.length_byte +
                           sizeof(pCEocFrameBuf->vrxEocFrame.length_byte) +
                           sizeof(pCEocFrameBuf->vrxEocFrame.cEocId)),
                     MEI_RECV_BUF_CTRL_MODEM_EOC_FRAME);
      if (distCount <= 0)
      {
         PRN_DBG_USR( MEI_CEOC, MEI_DRV_PRN_LEVEL_NORMAL, MEI_DRV_LINENUM_GET(pMeiDev),
                ("MEI_EOC[%02d]: WARNING - "
                 "no waiting user, discard Clear EOC Frame auto. msg!" MEI_DRV_CRLF,
                  MEI_DRV_LINENUM_GET(pMeiDev)));
      }

      /* reset buffer */
      pCEocFrameBuf->frameSize_byte          = 0;
      pCEocFrameBuf->vrxEocFrame.cEocId      = 0;
      pCEocFrameBuf->vrxEocFrame.length_byte = 0;
      pCEocFrameBuf->transferedSize_byte     = 0;
   }

   return IFX_SUCCESS;
}


/* ============================================================================
   Global Function Definition
   ========================================================================= */

/**
   Release the Clear EOC Access Control structure.

\param
   pMeiDev      points to the VRX driver private device data [I]

\return
   0 (IFX_SUCCESS) if success.
   negative value in case of error.
*/
IFX_int32_t MEI_CEOC_ReleaseDevCntrl(
                              MEI_DEV_T       *pMeiDev)
{
   MEI_CEOC_DEV_CNTRL_T *pCEocDevCntrl;

   if (pMeiDev->pCEocDevCntrl == IFX_NULL)
   {
      PRN_ERR_USR_NL( MEI_CEOC, MEI_DRV_PRN_LEVEL_ERR,
             ("MEI_EOC[%02d]: ERROR - invalid arg for release Clear EOC control" MEI_DRV_CRLF,
              MEI_DRV_LINENUM_GET(pMeiDev)) );

      return -e_MEI_ERR_DEV_INIT;
   }

   pCEocDevCntrl = pMeiDev->pCEocDevCntrl;

   /* get unique access to the module, dechain the Clear EOC block */
   MEI_CEOC_GET_UNIQUE_ACCESS(pCEocDevCntrl);
   MEI_DRV_GET_UNIQUE_ACCESS(pMeiDev);

   pMeiDev->pCEocDevCntrl = IFX_NULL;

   MEI_DRV_RELEASE_UNIQUE_ACCESS(pMeiDev);
   MEI_CEOC_RELEASE_UNIQUE_ACCESS(pCEocDevCntrl);

   /* release resourcen */
   MEI_DRV_GET_UNIQUE_ACCESS(pMeiDev);

   MEI_DRVOS_EventDelete(&pCEocDevCntrl->eventCEocStateChange);
   MEI_DRVOS_SemaphoreDelete(&pCEocDevCntrl->pDevCEocCtrlRWlock);
   MEI_DRVOS_Free(pCEocDevCntrl);
   pCEocDevCntrl = IFX_NULL;

   MEI_DRV_RELEASE_UNIQUE_ACCESS(pMeiDev);

   return IFX_SUCCESS;
}

/**
   Reset the control infos within the Clear EOC struct.

\param
   pMeiDev      points to the VRX driver private device data [I]


*/
IFX_int32_t MEI_CEOC_ResetControl(
                              MEI_DEV_T       *pMeiDev)
{
   MEI_CEOC_DEV_CNTRL_T *pCEocDevCntrl = pMeiDev->pCEocDevCntrl;

   if (!pCEocDevCntrl)
      return IFX_SUCCESS;

   if (MEI_CEOC_CFG_STATE_GET(pCEocDevCntrl) != eMEI_CEOC_OP_CFG_INITIAL)
   {
      MEI_CEOC_CFG_STATE_SET(pCEocDevCntrl, eMEI_CEOC_OP_CFG_VALID);
   }

   MEI_CEOC_TX_DEV_BUF_STATE_SET(pCEocDevCntrl, eMEI_CEOC_TX_DEV_BUF_STATE_IDLE);
   MEI_CEOC_RX_DEV_BUF_STATE_SET(pCEocDevCntrl, eMEI_CEOC_RX_DEV_BUF_STATE_IDLE);

   return IFX_SUCCESS;
}


/**
   Check the CEOC config if processing is neccessary

\param
   pMeiDev      Points to the VRX driver device data. [I]
\param
   pCEocDevCntrl  Points to the EOC control struct. [I]
\param
   msgId          Message ID of the reveived modem message. [I]

\return
   IFX_TRUE  - work neccessary (no transparent mode).
   IFX_FALSE - no work neccessary, forward to upper layer (transparent mode).
*/
IFX_boolean_t MEI_CEOC_CheckForWork(
                              MEI_DEV_T            *pMeiDev,
                              MEI_CEOC_DEV_CNTRL_T *pCEocDevCntrl,
                              IFX_uint16_t           msgId)
{
   IFX_boolean_t bDoProcessing = IFX_FALSE;

   /* check - Clear EOC block available */
   if (!pCEocDevCntrl)
   {
      return IFX_FALSE;
   }

   /* check - if transparent mode --> no driver CEOC processing */
   switch (msgId)
   {
      case MEI_DRV_EVT_CLEAREOCSTATUSGET:
         bDoProcessing = MEI_CEOC_CHECK_TRANSMODE_EVT_STAT(pCEocDevCntrl) ? IFX_FALSE : IFX_TRUE;
         break;
      case MEI_MSG_EVT_CLEAREOC_READ:
         bDoProcessing = MEI_CEOC_CHECK_TRANSMODE_EVT_DATA(pCEocDevCntrl) ? IFX_FALSE : IFX_TRUE;
         break;
      default:
         break;
   }

   return bDoProcessing;
}


/**
   Clear EOC Autonomous Message Handler.

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
IFX_int32_t MEI_CEOC_AutoMsgHandler(
                              MEI_DEV_T       *pMeiDev,
                              CMV_STD_MESSAGE_T *pModemMsg)
{
   IFX_int32_t retVal = IFX_ERROR;
   MEI_CEOC_DEV_CNTRL_T *pCEocDevCntrl = pMeiDev->pCEocDevCntrl;

   switch (P_CMV_MSGHDR_MSGID_GET(pModemMsg))
   {
      case MEI_DRV_EVT_CLEAREOCSTATUSGET:
         {
            retVal = MEI_CEOC_doEVT_ClearEOCStatusGet(
                              pMeiDev, pCEocDevCntrl, pModemMsg);
         }
         break;

      case MEI_MSG_EVT_CLEAREOC_READ:
         {
            retVal = MEI_CEocFrameEvt(pMeiDev, pCEocDevCntrl, pModemMsg);
         }
         break;

      default:
         break;
   }

   return retVal;
}


/**
   ioctl-function for initialization of the Clear EOC access.
   Setup the Clear EOC access feature within the driver.

\param
   pMeiDynCntrl    Private dynamic device data (per open instance). [I]
\param
   pIoctlCEocInit    Points to the user ioctl data. [I]

\return
   0 (IFX_SUCCESS) if success
   negative value in case of error
*/
IFX_int32_t MEI_CEOC_IoctlDrvInit(
                              MEI_DYN_CNTRL_T       *pMeiDynCntrl,
                              IOCTL_MEI_CEOC_init_t *pIoctlCEocInit )
{
   IFX_int32_t             ret = IFX_ERROR;
   MEI_DEV_T             *pMeiDev = pMeiDynCntrl->pMeiDev;
   MEI_CEOC_DEV_CNTRL_T  *pCEocDevCntrl = IFX_NULL;

   if (pMeiDev->pCEocDevCntrl)
   {
      pCEocDevCntrl = pMeiDev->pCEocDevCntrl;
   }
   else
   {
      if ( (ret = MEI_CEocCreateDevCntrl(pMeiDynCntrl, &pCEocDevCntrl)) != IFX_SUCCESS)
      {
         PRN_ERR_USR_NL( MEI_CEOC, MEI_DRV_PRN_LEVEL_ERR,
                ("MEI_EOC[%02d - %02d]: ERROR ioctl(FIO_MEI_CEOC_INIT) "
                 "-  allocate control" MEI_DRV_CRLF
                  , MEI_DRV_LINENUM_GET(pMeiDev), pMeiDynCntrl->openInstance));

         return ret;
      }
      pMeiDev->pCEocDevCntrl = pCEocDevCntrl;
   }

   return IFX_SUCCESS;
}


/**
   ioctl-function for configure the Clear EOC access.
   Setup the Clear EOC access feature within the VRX device.

\param
   pMeiDynCntrl    Private dynamic device data (per open instance). [I]
\param
   pIoctlCEocCntrl   Points to the user ioctl data. [I]

\return
   0 (IFX_SUCCESS) if success
   negative value in case of error
*/
IFX_int32_t MEI_CEOC_IoctlCntrl(
                              MEI_DYN_CNTRL_T          *pMeiDynCntrl,
                              IOCTL_MEI_CEOC_cntrl_t   *pIoctlCEocCntrl )
{
   IFX_int32_t             ret = IFX_ERROR;
   IFX_uint32_t            devCntrlMode = 0, opMode = 0, transmode = 0;
   MEI_DEV_T             *pMeiDev = pMeiDynCntrl->pMeiDev;
   MEI_CEOC_DEV_CNTRL_T  *pCEocDevCntrl = IFX_NULL;

   if (pMeiDev->pCEocDevCntrl)
   {
      pCEocDevCntrl = pMeiDev->pCEocDevCntrl;
   }
   else
   {
      if ( (ret = MEI_CEocCreateDevCntrl(pMeiDynCntrl, &pCEocDevCntrl)) != IFX_SUCCESS)
      {
         PRN_ERR_USR_NL( MEI_CEOC, MEI_DRV_PRN_LEVEL_ERR,
                ("MEI_EOC[%02d - %02d]: ERROR ioctl(FIO_MEI_CEOC_CNTRL) "
                 "-  allocate control" MEI_DRV_CRLF
                  , MEI_DRV_LINENUM_GET(pMeiDev), pMeiDynCntrl->openInstance));

         return ret;
      }
      pMeiDev->pCEocDevCntrl = pCEocDevCntrl;
   }

   if ( MEI_DRV_MSG_AVAILABLE( pMeiDynCntrl->pMeiDev,
                                 MEI_DRV_CMD_CLEAREOC_CONFIGURE) != IFX_TRUE )
   {
      PRN_ERR_USR_NL( MEI_CEOC, MEI_DRV_PRN_LEVEL_ERR,
             ("MEI_EOC[%02d - %02d]: ERROR ioctl(FIO_MEI_CEOC_CNTRL) "
              "- invalid state, cannot setup config" MEI_DRV_CRLF
               , MEI_DRV_LINENUM_GET(pMeiDev), pMeiDynCntrl->openInstance));

      return -e_MEI_ERR_REM_SETUP;
   }

   switch(pIoctlCEocCntrl->opMode)
   {
      case MEI_CEOC_OPERATION_MODE_DIRECT:
         opMode       = MEI_CEOC_OPERATION_MODE_DIRECT;
         transmode    = (pIoctlCEocCntrl->transMode & MEI_CEOC_INIT_TRANS_MODE_ALL);
         devCntrlMode = (pIoctlCEocCntrl->cntrl & MEI_CEOC_CNTRL_FLAG_EVT_ALL);
         break;
      case MEI_CEOC_OPERATION_MODE_AUTO:
         opMode       = MEI_CEOC_OPERATION_MODE_AUTO;
         transmode    = 0; /* no transparent forwarding */
         devCntrlMode = (  MEI_CEOC_CNTRL_FLAG_EVT_DATA_READ
                         | MEI_CEOC_CNTRL_FLAG_EVT_TX_STAT);
         break;
      case MEI_CEOC_OPERATION_MODE_AUTO_READ:
         opMode       = MEI_CEOC_OPERATION_MODE_AUTO_READ;
         transmode    = MEI_CEOC_INIT_TRANS_MODE_EVT_RX_STAT;
         devCntrlMode = (  MEI_CEOC_CNTRL_FLAG_EVT_TX_STAT
                         | MEI_CEOC_CNTRL_FLAG_EVT_RX_STAT);
         break;
      case MEI_CEOC_OPERATION_MODE_POLL_READ:
         opMode       = MEI_CEOC_OPERATION_MODE_POLL_READ;
         transmode    = 0;
         devCntrlMode = (MEI_CEOC_CNTRL_FLAG_EVT_TX_STAT);
         break;
      default:
         PRN_ERR_USR_NL( MEI_CEOC, MEI_DRV_PRN_LEVEL_ERR,
                ("MEI_EOC[%02d - %02d]: ERROR ioctl(FIO_MEI_CEOC_CNTRL) "
                 "- invalid operation mode" MEI_DRV_CRLF
                  , MEI_DRV_LINENUM_GET(pMeiDev), pMeiDynCntrl->openInstance));
         return -e_MEI_ERR_REM_SETUP;
   }

   MEI_CEOC_GET_UNIQUE_ACCESS(pCEocDevCntrl);

   pCEocDevCntrl->opMode        = opMode;
   pCEocDevCntrl->cEocTransMode = transmode;

   pCEocDevCntrl->bEnEvtRxData =
         (devCntrlMode & MEI_CEOC_CNTRL_FLAG_EVT_DATA_READ) ? IFX_TRUE : IFX_FALSE;
   pCEocDevCntrl->bEnEvtRxStatus =
         (devCntrlMode & MEI_CEOC_CNTRL_FLAG_EVT_RX_STAT) ? IFX_TRUE : IFX_FALSE;
   pCEocDevCntrl->bEnEvtTxStatus =
         (devCntrlMode & MEI_CEOC_CNTRL_FLAG_EVT_TX_STAT) ? IFX_TRUE : IFX_FALSE;

   MEI_CEOC_CFG_STATE_SET(pCEocDevCntrl, eMEI_CEOC_OP_CFG_VALID);

   /* send the new configuration to the device */
   if ( (ret = MEI_CEOC_CMD_ClearEOC_Configure(pMeiDynCntrl, pCEocDevCntrl)) != IFX_SUCCESS)
   {
      MEI_CEOC_RELEASE_UNIQUE_ACCESS(pCEocDevCntrl);
      return ret;
   }

   MEI_CEOC_CFG_STATE_SET(pCEocDevCntrl, eMEI_CEOC_OP_CFG_WRITTEN);
   MEI_CEOC_TX_DEV_BUF_STATE_SET(pCEocDevCntrl, eMEI_CEOC_TX_DEV_BUF_STATE_IDLE);
   MEI_CEOC_RX_DEV_BUF_STATE_SET(pCEocDevCntrl, eMEI_CEOC_RX_DEV_BUF_STATE_IDLE);

   MEI_CEOC_RELEASE_UNIQUE_ACCESS(pCEocDevCntrl);

   return IFX_SUCCESS;
}


/**
   ioctl-function for request the Clear EOC Status and Statistics.

\param
   pMeiDynCntrl    Private dynamic device data (per open instance). [I]
\param
   pIoctlCEocStats   Points to the user ioctl data. [O]

\return
   0 (IFX_SUCCESS) if success
   negative value in case of error
*/
IFX_int32_t MEI_CEOC_IoctlStatusGet(
                              MEI_DYN_CNTRL_T            *pMeiDynCntrl,
                              IOCTL_MEI_CEOC_statistic_t *pIoctlCEocStats)
{
#if ((MEI_SUPPORT_STATISTICS == 1) && (MEI_SUPPORT_CEOC_STATISTICS == 1))
   MEI_CEOC_DRV_STATS_T  *pCEocStats = IFX_NULL;

   if (!pMeiDynCntrl->pMeiDev->pCEocDevCntrl)
   {
      PRN_ERR_USR_NL( MEI_CEOC, MEI_DRV_PRN_LEVEL_ERR,
             ("MEI_EOC[%02d - %02d]: ERROR ioctl(FIO_MEI_CEOC_STATS) "
              "- missing init" MEI_DRV_CRLF
               , MEI_DRV_DYN_LINENUM_GET(pMeiDynCntrl), pMeiDynCntrl->openInstance));

      return -e_MEI_ERR_REM_SETUP;
   }

   pCEocStats = &pMeiDynCntrl->pMeiDev->pCEocDevCntrl->statistics;

   MEI_CEOC_GET_UNIQUE_ACCESS(pMeiDynCntrl->pMeiDev->pCEocDevCntrl);

   /** number of written CEOC modem "write" messages */
   pIoctlCEocStats->wrMsgCnt        = pCEocStats->wrMsgCnt;
   /** number of error written CEOC modem "write" messages */
   pIoctlCEocStats->wrMsgErrCnt     = pCEocStats->wrMsgErrCnt;

   /** number of read CEOC modem "read" messages */
   pIoctlCEocStats->rdMsgCnt        = pCEocStats->rdMsgCnt;
   /** number of error read CEOC modem "read" messages */
   pIoctlCEocStats->rdMsgErrCnt     = pCEocStats->rdMsgErrCnt;

   /** number of read CEOC modem "event read" messages */
   pIoctlCEocStats->evtRdMsgCnt     = pCEocStats->evtRdMsgCnt;
   /** number of error read CEOC modem "event read" messages */
   pIoctlCEocStats->evtRdMsgErrCnt  = pCEocStats->evtRdMsgErrCnt;

   /** number of written CEOC frames */
   pIoctlCEocStats->wrFrameCnt      = pCEocStats->wrFrameCnt;
   /** number of read CEOC frames */
   pIoctlCEocStats->rdFrameCnt      = pCEocStats->rdFrameCnt;
   /** number of event read CEOC frames */
   pIoctlCEocStats->evtRdFrameCnt   = pCEocStats->evtRdFrameCnt;
   /** number of received corrupted CEOC frames */
   pIoctlCEocStats->recvFrameErrCnt = pCEocStats->recvFrameErrCnt;

   /** number of TX trigger send */
   pIoctlCEocStats->sendTxTriggerCnt = pCEocStats->sendTxTriggerCnt;

   /** number of timeout after TX trigger send (timeout insert frame) */
   pIoctlCEocStats->insertTimeoutCnt = pCEocStats->insertTimeoutCnt;

   /** number of RX status error */
   pIoctlCEocStats->rxStatErrCnt    = pCEocStats->rxStatErrCnt;
   /** number of TX status error */
   pIoctlCEocStats->txStatErrCnt    = pCEocStats->txStatErrCnt;

   MEI_CEOC_RELEASE_UNIQUE_ACCESS(pMeiDynCntrl->pMeiDev->pCEocDevCntrl);
#endif

   return IFX_SUCCESS;
}


/**
   ioctl-function for write a Clear EOC frame.
   Segement the frame and insert the messages to the VRX device.

\param
   pMeiDynCntrl    Private dynamic device data (per open instance). [I]
\param
   pIoctlCEocFrame   Points to the EOC frame to send. [I]

\return
   0 (IFX_SUCCESS) if success
   negative value in case of error
*/
IFX_int32_t MEI_CEOC_IoctlFrameWrite(
                              MEI_DYN_CNTRL_T        *pMeiDynCntrl,
                              IOCTL_MEI_CEOC_frame_t *pIoctlCEocFrame,
                              IFX_boolean_t            bInternCall)
{
   IFX_int32_t                retVal = IFX_SUCCESS;
   MEI_CEOC_DEV_CNTRL_T     *pCEocDevCntrl;
   MEI_CEOC_FRAME_BUFFER_T  *pCEocFrameBuf;
   MEI_CEOC_MEI_EOC_FRAME_T *pVrxEocFrame;

   if (!pMeiDynCntrl->pMeiDev->pCEocDevCntrl)
   {
      PRN_ERR_USR_NL( MEI_CEOC, MEI_DRV_PRN_LEVEL_ERR,
             ("MEI_EOC[%02d - %02d]: ERROR ioctl(FIO_MEI_CEOC_FRAME_WR) "
              "- missing init" MEI_DRV_CRLF,
              MEI_DRV_DYN_LINENUM_GET(pMeiDynCntrl), pMeiDynCntrl->openInstance));

      return -e_MEI_ERR_REM_SETUP;
   }

   if ( (pIoctlCEocFrame->dataSize_byte > MEI_CEOC_RAW_EOC_DATA_SIZE_BYTE) ||
        (pIoctlCEocFrame->pEocData == IFX_NULL) )
   {
      PRN_ERR_USR_NL( MEI_CEOC, MEI_DRV_PRN_LEVEL_ERR,
             ("MEI_EOC[%02d - %02d]: ERROR ioctl(FIO_MEI_CEOC_FRAME_WR) "
              "- invalid args size = %d (max = %d)" MEI_DRV_CRLF,
               MEI_DRV_DYN_LINENUM_GET(pMeiDynCntrl), pMeiDynCntrl->openInstance,
               pIoctlCEocFrame->dataSize_byte, MEI_CEOC_RAW_EOC_DATA_SIZE_BYTE));
      return -e_MEI_ERR_INVAL_ARG;
   }

   pCEocDevCntrl = pMeiDynCntrl->pMeiDev->pCEocDevCntrl;
   pCEocFrameBuf = &pCEocDevCntrl->txEocFrame;
   pVrxEocFrame  = &pCEocFrameBuf->vrxEocFrame;

   MEI_CEOC_GET_UNIQUE_ACCESS(pCEocDevCntrl);

   if (MEI_CEOC_CFG_STATE_GET(pCEocDevCntrl) != eMEI_CEOC_OP_CFG_WRITTEN)
   {
      PRN_ERR_USR_NL( MEI_CEOC, MEI_DRV_PRN_LEVEL_ERR,
             ("MEI_EOC[%02d - %02d]: ERROR ioctl(FIO_MEI_CEOC_FRAME_WR) "
              "- missing device setup" MEI_DRV_CRLF,
              MEI_DRV_DYN_LINENUM_GET(pMeiDynCntrl), pMeiDynCntrl->openInstance));

      retVal = -e_MEI_ERR_REM_SETUP;
      goto MEI_CEOC_IOCTL_FRAME_WRITE_ERR;
   }

   if (pCEocFrameBuf->frameSize_byte != 0)
   {
      PRN_ERR_USR_NL( MEI_CEOC, MEI_DRV_PRN_LEVEL_WRN,
             ("MEI_EOC[%02d - %02d]: WRN ioctl(FIO_MEI_CEOC_FRAME_WR) "
              "- overwrite local buffer" MEI_DRV_CRLF,
              MEI_DRV_DYN_LINENUM_GET(pMeiDynCntrl), pMeiDynCntrl->openInstance));
   }

   /* the VRX EOC Frame struct expects here: sizeof(data) + sizeof(prot-ID) */
   pVrxEocFrame->length_byte = (IFX_uint16_t)(pIoctlCEocFrame->dataSize_byte +
                                              sizeof(pVrxEocFrame->protIdent));
   pVrxEocFrame->protIdent   = (IFX_uint16_t)(pIoctlCEocFrame->protIdent & 0x0000FFFF);
   if (bInternCall)
   {
      memcpy( pVrxEocFrame->cEocRawData.d_8,
              pIoctlCEocFrame->pEocData,
              pIoctlCEocFrame->dataSize_byte);
   }
   else
   {
      if ( (MEI_DRVOS_CpyFromUser( pVrxEocFrame->cEocRawData.d_8,
                                   pIoctlCEocFrame->pEocData,
                                   pIoctlCEocFrame->dataSize_byte)) == IFX_NULL )
      {
         PRN_ERR_USR_NL( MEI_CEOC, MEI_DRV_PRN_LEVEL_ERR,
               ("MEI_EOC[%02d - %02d]: ERROR ioctl(FIO_MEI_CEOC_FRAME_WR) - "
                "copy_from_user() failed!" MEI_DRV_CRLF,
                MEI_DRV_DYN_LINENUM_GET(pMeiDynCntrl), pMeiDynCntrl->openInstance));

         retVal = -e_MEI_ERR_GET_ARG;
         goto MEI_CEOC_IOCTL_FRAME_WRITE_ERR;
      }
   }

   /* setup frame buffer + padding */
   pCEocFrameBuf->transferedSize_byte = 0;
   pCEocFrameBuf->frameSize_byte      = pVrxEocFrame->length_byte +
                                        sizeof(pVrxEocFrame->protIdent) +
                                        sizeof(pVrxEocFrame->length_byte);

   if (pIoctlCEocFrame->dataSize_byte & 0x1)
   {
      pVrxEocFrame->cEocRawData.d_8[pIoctlCEocFrame->dataSize_byte] = 0;
      pCEocFrameBuf->frameSize_byte++;
   }


   PRN_DBG_USR( MEI_CEOC, MEI_DRV_PRN_LEVEL_LOW, MEI_DRV_DYN_LINENUM_GET(pMeiDynCntrl),
         ("MEI_EOC[%02d - %02d]: Write VrxFrame "
          "len = %3d - Prot 0x%04X | 0x%02X 0x%02X ... !" MEI_DRV_CRLF,
          MEI_DRV_DYN_LINENUM_GET(pMeiDynCntrl), pMeiDynCntrl->openInstance,
          pVrxEocFrame->length_byte, pVrxEocFrame->protIdent,
          pVrxEocFrame->cEocRawData.d_8[0], pVrxEocFrame->cEocRawData.d_8[1]));

   if ( (retVal = MEI_CEocFrameSend(
                     pMeiDynCntrl, pCEocDevCntrl, pCEocFrameBuf)) != IFX_SUCCESS)
   {
      PRN_ERR_USR_NL( MEI_CEOC, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_EOC[%02d - %02d]: ERROR ioctl(FIO_MEI_CEOC_FRAME_WR) - "
             "send frame failed" MEI_DRV_CRLF,
             MEI_DRV_DYN_LINENUM_GET(pMeiDynCntrl), pMeiDynCntrl->openInstance ));

      goto MEI_CEOC_IOCTL_FRAME_WRITE_ERR;
   }

   MEI_CEOC_STAT_INC_WR_FRAME_CNT(pCEocDevCntrl);
   MEI_CEOC_RELEASE_UNIQUE_ACCESS(pCEocDevCntrl);

   return IFX_SUCCESS;

MEI_CEOC_IOCTL_FRAME_WRITE_ERR:
   MEI_CEOC_RELEASE_UNIQUE_ACCESS(pCEocDevCntrl);
   return retVal;
}

/**
   ioctl-function for read a Clear EOC frame.
   Receive the segment messages from the VRX device and reassamble the frame.

\param
   pMeiDynCntrl    Private dynamic device data (per open instance). [I]
\param
   pIoctlCEocFrame   Points to the EOC frame to send. [I]

\return
   0 (IFX_SUCCESS) if success
   negative value in case of error
*/
IFX_int32_t MEI_CEOC_IoctlFrameRead(
                              MEI_DYN_CNTRL_T        *pMeiDynCntrl,
                              IOCTL_MEI_CEOC_frame_t *pIoctlCEocFrame,
                              IFX_boolean_t            bInternCall)
{
   IFX_int32_t                retVal = IFX_SUCCESS;
   MEI_CEOC_DEV_CNTRL_T     *pCEocDevCntrl;
   MEI_CEOC_FRAME_BUFFER_T  *pCEocFrameBuf;

   if (!pMeiDynCntrl->pMeiDev->pCEocDevCntrl)
   {
      PRN_ERR_USR_NL( MEI_CEOC, MEI_DRV_PRN_LEVEL_ERR,
             ("MEI_EOC[%02d - %02d]: ERROR ioctl(FIO_MEI_CEOC_FRAME_RD) "
              "- missing init" MEI_DRV_CRLF,
              MEI_DRV_DYN_LINENUM_GET(pMeiDynCntrl), pMeiDynCntrl->openInstance));

      return -e_MEI_ERR_REM_SETUP;
   }

   if ( (pIoctlCEocFrame->dataSize_byte < MEI_CEOC_RAW_EOC_DATA_SIZE_BYTE) ||
        (pIoctlCEocFrame->pEocData == IFX_NULL) )
   {
      PRN_ERR_USR_NL( MEI_CEOC, MEI_DRV_PRN_LEVEL_ERR,
             ("MEI_EOC[%02d - %02d]: ERROR ioctl(FIO_MEI_CEOC_FRAME_RD) "
              "- invalid args size = %d (max = %d)" MEI_DRV_CRLF,
               MEI_DRV_DYN_LINENUM_GET(pMeiDynCntrl), pMeiDynCntrl->openInstance,
               pIoctlCEocFrame->dataSize_byte, MEI_CEOC_RAW_EOC_DATA_SIZE_BYTE));
      return -e_MEI_ERR_INVAL_ARG;
   }

   pCEocDevCntrl = pMeiDynCntrl->pMeiDev->pCEocDevCntrl;
   pCEocFrameBuf = &pCEocDevCntrl->rxEocFrame;

   MEI_CEOC_GET_UNIQUE_ACCESS(pCEocDevCntrl);

   if (MEI_CEOC_CFG_STATE_GET(pCEocDevCntrl) != eMEI_CEOC_OP_CFG_WRITTEN)
   {
      PRN_ERR_USR_NL( MEI_CEOC, MEI_DRV_PRN_LEVEL_ERR,
             ("MEI_EOC[%02d - %02d]: ERROR ioctl(FIO_MEI_CEOC_FRAME_RD) "
              "- missing device setup" MEI_DRV_CRLF,
              MEI_DRV_DYN_LINENUM_GET(pMeiDynCntrl), pMeiDynCntrl->openInstance));

      retVal = -e_MEI_ERR_REM_SETUP;
      goto MEI_CEOC_IOCTL_FRAME_READ_ERR;
   }
   else
   {
      if (pCEocDevCntrl->bEnEvtRxData)
      {
         PRN_ERR_USR_NL( MEI_CEOC, MEI_DRV_PRN_LEVEL_ERR,
                ("MEI_EOC[%02d - %02d]: ERROR ioctl(FIO_MEI_CEOC_FRAME_RD) "
                 "- canot read, recv DATA via EVT" MEI_DRV_CRLF,
                 MEI_DRV_DYN_LINENUM_GET(pMeiDynCntrl), pMeiDynCntrl->openInstance));

         retVal = -e_MEI_ERR_REM_SETUP;
         goto MEI_CEOC_IOCTL_FRAME_READ_ERR;
      }
   }

   /* setup receive message buffer */
   pCEocFrameBuf->vrxEocFrame.cEocId      = MEI_DRV_MSG_CTRL_IF_MODEM_EOC_FRAME_ON;
   pCEocFrameBuf->vrxEocFrame.length_byte = 0;
   pCEocFrameBuf->transferedSize_byte     = 0;
   pCEocFrameBuf->frameSize_byte = MEI_CEOC_RAW_EOC_DATA_SIZE_BYTE +
                                   sizeof(pCEocFrameBuf->vrxEocFrame.protIdent) +
                                   sizeof(pCEocFrameBuf->vrxEocFrame.length_byte);

   if ( (retVal = MEI_CEocFrameRead(
                     pMeiDynCntrl, pCEocDevCntrl, pCEocFrameBuf)) != IFX_SUCCESS)
   {
      PRN_ERR_USR_NL( MEI_CEOC, MEI_DRV_PRN_LEVEL_ERR,
             ("MEI_EOC[%02d - %02d]: ERROR ioctl(FIO_MEI_CEOC_FRAME_RD) "
              "- read data from device" MEI_DRV_CRLF,
              MEI_DRV_DYN_LINENUM_GET(pMeiDynCntrl), pMeiDynCntrl->openInstance));

      goto MEI_CEOC_IOCTL_FRAME_READ_ERR;
   }

   /* check result */
   if (pCEocFrameBuf->transferedSize_byte > 0)
   {
      if (pCEocFrameBuf->transferedSize_byte == pCEocFrameBuf->frameSize_byte)
      {
         pIoctlCEocFrame->protIdent     = (unsigned int)pCEocFrameBuf->vrxEocFrame.protIdent;
         pIoctlCEocFrame->dataSize_byte = (unsigned int)(pCEocFrameBuf->vrxEocFrame.length_byte -
                                                         sizeof(pCEocFrameBuf->vrxEocFrame.protIdent));
         if (bInternCall)
         {
            memcpy( pIoctlCEocFrame->pEocData,
                    pCEocFrameBuf->vrxEocFrame.cEocRawData.d_8,
                    pIoctlCEocFrame->dataSize_byte);
         }
         else
         {
            if ( (MEI_DRVOS_CpyToUser( pIoctlCEocFrame->pEocData,
                                       pCEocFrameBuf->vrxEocFrame.cEocRawData.d_8,
                                       pIoctlCEocFrame->dataSize_byte)) == IFX_NULL )
            {
               PRN_ERR_USR_NL( MEI_CEOC, MEI_DRV_PRN_LEVEL_ERR,
                     ("MEI_EOC[%02d - %02d]: ERROR ioctl(FIO_MEI_CEOC_FRAME_RD) - "
                      "copy_to_user() failed!" MEI_DRV_CRLF,
                      MEI_DRV_DYN_LINENUM_GET(pMeiDynCntrl), pMeiDynCntrl->openInstance));

               retVal = -e_MEI_ERR_GET_ARG;
               goto MEI_CEOC_IOCTL_FRAME_READ_ERR;
            }
         }
      }
      else
      {
         /* missmatch */
         pIoctlCEocFrame->protIdent     = 0;
         pIoctlCEocFrame->dataSize_byte = 0;

         PRN_ERR_USR_NL( MEI_CEOC, MEI_DRV_PRN_LEVEL_ERR,
               ("MEI_EOC[%02d - %02d]: ERROR ioctl(FIO_MEI_CEOC_FRAME_RD) - "
                "not complete (frame size = %d, recv = %d)!" MEI_DRV_CRLF,
                MEI_DRV_DYN_LINENUM_GET(pMeiDynCntrl), pMeiDynCntrl->openInstance,
                pCEocFrameBuf->transferedSize_byte, pCEocFrameBuf->frameSize_byte));

         retVal = -e_MEI_ERR_NOT_COMPLETE;
         goto MEI_CEOC_IOCTL_FRAME_READ_ERR;
      }
   }
   else
   {
      pIoctlCEocFrame->protIdent     = 0;
      pIoctlCEocFrame->dataSize_byte = 0;
   }

   pCEocFrameBuf->vrxEocFrame.length_byte = 0;
   pCEocFrameBuf->transferedSize_byte     = 0;

   MEI_CEOC_RELEASE_UNIQUE_ACCESS(pCEocDevCntrl);
   return IFX_SUCCESS;

MEI_CEOC_IOCTL_FRAME_READ_ERR:
   pCEocFrameBuf->vrxEocFrame.cEocId      = 0;
   pCEocFrameBuf->vrxEocFrame.length_byte = 0;
   pCEocFrameBuf->transferedSize_byte     = 0;

   MEI_CEOC_RELEASE_UNIQUE_ACCESS(pCEocDevCntrl);
   return retVal;
}

#endif      /* #if (MEI_DRV_CLEAR_EOC_ENABLE == 1) */


