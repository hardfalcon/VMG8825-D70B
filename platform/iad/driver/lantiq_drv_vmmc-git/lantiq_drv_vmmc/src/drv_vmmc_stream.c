/******************************************************************************

                              Copyright (c) 2012
                            Lantiq Deutschland GmbH
                             http://www.lantiq.com

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

*******************************************************************************/

/**
   \file drv_vmmc_stream.c
   This file contains the implementation of the packet streaming.

   \remarks
   Only downstream packet handling is done here. The upstream packet handling
   is done directly in the interrupt handler in drv_vmmc_int.c.
*/

/* ============================= */
/* Includes                      */
/* ============================= */
#include "drv_vmmc_api.h"
#include "drv_vmmc_access.h"
#include "drv_mps_vmmc.h"
#include "drv_vmmc_stream.h"
#include "drv_vmmc_pcm.h"

/* ============================= */
/* Local Macros & Definitions    */
/* ============================= */

/** Convert channel number to MPS device number for mps first channel
    has number 2 and for vmmc first channel has number 1. */
#define VMMC_CH_TO_MPS_DEV(m_nCh) ((mps_devices)((m_nCh) + 1))

/* ============================= */
/* Global variable definition    */
/* ============================= */

/* ============================= */
/* Global function declaration   */
/* ============================= */

/* ============================= */
/* Local function declaration    */
/* ============================= */

/* ============================= */
/* Local variable definition     */
/* ============================= */

/* ============================= */
/* Local function definition     */
/* ============================= */

/* ============================= */
/* Global function definition    */
/* ============================= */

/**
   Open a VMMC channel

   Just increment the linux module use count.

   \param  pLLChDev     Pointer to device struct OR channel struct.

   \return \ref IFX_SUCCESS

   \remarks
   - Called by the High Level TAPI module (ifx_tapi_open).
   - Use nChannel to distinguish between channel and device struct.
*/
/*lint -esym(715, pLLChDev) */
IFX_int32_t VMMC_LL_Open(IFX_TAPI_LL_CH_t *pLLChDev)
{
   VMMC_UNUSED(pLLChDev);

#ifdef MODULE
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
   /* This driver does not own device nodes so the Linux kernel does not change
      the use count of this module. All device nodes are owned by the TAPI-HL
      driver. So we modify the use count explicitly. */
   try_module_get(THIS_MODULE);
#else
   MOD_INC_USE_COUNT;
#endif
#endif

   return IFX_SUCCESS;
}
/*lint +esym(715, pLLChDev) */


/**
   Close a VMMC channel

   \param  pLLChDev     Pointer to device struct OR channel struct.

   \return \ref IFX_SUCCESS

   \remarks
   - Called by the High Level TAPI module (ifx_tapi_release).
   - Use nChannel to distinguish between channel and device struct.
*/
/*lint -esym(715, pLLChDev) */
IFX_int32_t VMMC_LL_Close(IFX_TAPI_LL_CH_t *pLLChDev)
{
   VMMC_UNUSED(pLLChDev);

   /* we don't close anything here as we want to allow the case that
      the application closes all fds while voice streaming via KPI
      is ongoing...
   */
#ifdef MODULE
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
   /* This driver does not own device nodes so the Linux kernel does not change
      the use count of this module. All device nodes are owned by the TAPI-HL
      driver. So we modify the use count explicitly. */
   module_put(THIS_MODULE);
#else
   MOD_DEC_USE_COUNT;
#endif
#endif

   return IFX_SUCCESS;
}
/*lint +esym(715, pLLChDev) */


#ifdef VMMC_FEAT_PACKET
/**
   Unprotected data writing to the VMMC.

   \param  pLLCh    Pointer to the VMMC channel.
   \param  buf      Pointer to a buffer with the data to be sent.
   \param  count    Data length in bytes.
   \param  stream   Tag that identifies the packet contents.

   \return
      - IFX_ERROR    on failure
      - nLength      length of handled data
*/
static IFX_int32_t VMMC_LL_Write_Unprot (IFX_TAPI_LL_CH_t *pLLCh,
                                         const char *buf,
                                         IFX_int32_t count,
                                         IFX_TAPI_STREAM_t stream)
{
   VMMC_CHANNEL      *pCh  = (VMMC_CHANNEL*)pLLCh;
   mps_message       msg;
   IFX_int32_t       err   = VMMC_statusOk;

   /* truncate the length to the size of a packet we can handle */
   if (count > (MAX_PACKET_WORD << 1))
      count = (MAX_PACKET_WORD << 1);

   msg.pData          = (IFX_uint8_t *)buf;
   msg.nDataBytes     = (IFX_uint32_t)count;
   msg.RTP_PaylOffset = 0;

   switch (stream)
   {
   case IFX_TAPI_STREAM_COD:
      /* Stream towards the COD module. */
#ifdef VMMC_FEAT_FAX_T38
      /* if datapump is running declare packet to be a fax data packet */
      if (pCh->TapiFaxStatus.nStatus & IFX_TAPI_FAX_T38_DP_ON)
      {
         msg.cmd_type = DAT_PAYL_PTR_MSG_FAX_DATA_PACKET;
         /* clear the request no matter if the write below succeeds */
         IFX_TAPI_DownStream_RequestData(pCh->pTapiCh, IFX_FALSE);
         pCh->nFdpReqServiced++;
      }
      else
#endif /* VMMC_FEAT_FAX_T38 */
      {
         /* Check the packet payload for the actual packet type.
            pData is in network byte order but an array of bytes. */
         if ((msg.pData[1] & RTP_PT) != pCh->nEvtPT)
            msg.cmd_type = DAT_PAYL_PTR_MSG_VOICE_PACKET;
         else
            msg.cmd_type = DAT_PAYL_PTR_MSG_EVENT_PACKET;
      }
#ifdef VMMC_FEAT_FAX_T38_FW
      if (IFX_TRUE == pCh->bTapiT38Status)
      {
         msg.cmd_type = DAT_PAYL_PTR_MSG_FAX_T38_PACKET;
      }
#endif /* VMMC_FEAT_FAX_T38_FW */
#ifdef TAPI_PACKET_OWNID
      IFX_TAPI_VoiceBufferChOwn (msg.pData, IFX_TAPI_BUFFER_OWNER_COD_MPS);
#endif /* TAPI_PACKET_OWNID */
      break;
   case IFX_TAPI_STREAM_DECT:
      /* Stream towards the DECT module. */
      msg.cmd_type = DAT_PAYL_PTR_MSG_DECT_PACKET;
      break;
#ifdef VMMC_FEAT_HDLC
   case IFX_TAPI_STREAM_HDLC:
   {

      /* static HDLC frame size checks for length */
      if (msg.nDataBytes > VMMC_HDLC_MAX_PACKAGE_SIZE)
      {
         IFX_TAPI_EVENT_t tapiEvent;

         memset(&tapiEvent, 0, sizeof(IFX_TAPI_EVENT_t));
         tapiEvent.ch = pCh->nChannel - 1;

         tapiEvent.id = IFX_TAPI_EVENT_FAULT_HDLC_FRAME_LENGTH;
         IFX_TAPI_Event_Dispatch(pCh->pTapiCh, &tapiEvent);
         return 0;
      }

      err = VMMC_PCM_HDLC_Write (pCh, msg.pData, msg.nDataBytes);
      if (!VMMC_SUCCESS (err))
      {
         /* keep the error code, but return as a negative number */
         return (-err);
      }

      return msg.nDataBytes;
   }
#endif /* VMMC_FEAT_HDLC */
   default:
      /* not supported - drop packet */
      return 0;
   }

   err = VMMC_MPS_Write (pCh, stream, &msg);
   if (!VMMC_SUCCESS (err))
      return IFX_ERROR;

   return msg.nDataBytes;
}
/*lint +esym(715, ppos) */

/**
   Write data to the VMMC.

   \param  pLLCh    Pointer to the VMMC channel.
   \param  buf      Pointer to a buffer with the data to be sent.
   \param  count    Data length in bytes.
   \param  ppos     unused
   \param  stream   Tag that identifies the packet contents.

   \return
      - IFX_ERROR    on failure
      - nLength      length of handled data
*/
IFX_int32_t VMMC_LL_Write (IFX_TAPI_LL_CH_t *pLLCh, const char *buf,
                           IFX_int32_t count, IFX_int32_t* ppos,
                           IFX_TAPI_STREAM_t stream)
{
   IFX_int32_t    nBytesSent = 0;

   VMMC_UNUSED(ppos);

   nBytesSent = VMMC_LL_Write_Unprot (pLLCh, buf, count, stream);

   return nBytesSent;
}

/**
   Write data to the MPS.

   \param  pCh       Pointer to the VMMC channel structure.
   \param  stream    Tag that identifies the packet contents.
   \param  pMsg      Pointer to a MPS message.

   \return
      - VMMC_statusMpsWriteFail Failed to write data in to the MPS
      - VMMC_statusOk if successful
*/
IFX_int32_t VMMC_MPS_Write (VMMC_CHANNEL *pCh, IFX_TAPI_STREAM_t stream,
                            mps_message *pMsg)
{
   VMMC_DEVICE       *pDev = pCh->pParent;
   IFX_boolean_t     irq_context = VMMC_OS_IN_INTERRUPT();
   IFX_int32_t       ret   = VMMC_statusOk;
   IFX_TAPI_EVENT_t  tapiEvent;

   /* protect the data mailbox write access */
   if (!irq_context)
      VMMC_OS_MutexGet (&(pDev)->mtxDataMbxAcc);
   Vmmc_IrqLockDevice((pDev));
   if (IFX_SUCCESS == ifx_mps_write_mailbox(VMMC_CH_TO_MPS_DEV(pCh->nChannel),
                                            pMsg))
   {
      /* write was successful */
      pDev->bPktMbxCongested = IFX_FALSE;
      IFX_TAPI_Stat_Add(pCh->pTapiCh, stream,
                        TAPI_STAT_COUNTER_INGRESS_DELIVERED, 1);
   }
   else
   {
      /* write failed -> send event if this is the first time */
      if (! pDev->bPktMbxCongested)
      {
         memset(&tapiEvent, 0, sizeof(IFX_TAPI_EVENT_t));
         if (pMsg->cmd_type != DAT_PAYL_PTR_MSG_FAX_DATA_PACKET)
         {
            tapiEvent.id = IFX_TAPI_EVENT_INFO_MBX_CONGESTION;
         }
         else
         {
            tapiEvent.id = IFX_TAPI_EVENT_T38_ERROR_WRITE;
         }
         IFX_TAPI_Event_Dispatch(pCh->pTapiCh, &tapiEvent);
         pDev->bPktMbxCongested = IFX_TRUE;
      }
      IFX_TAPI_Stat_Add(pCh->pTapiCh, stream,
                        TAPI_STAT_COUNTER_INGRESS_CONGESTED, 1);

      /* errmsg: Failed to write data in to the MPS */
      ret = VMMC_statusMpsWriteFail;
   }
   /*if (pMsg->cmd_type == DAT_PAYL_PTR_MSG_HDLC_PACKET)*/
   LOG_WR_PKT((pDev->nDevNr), (pCh->nChannel-1),
               pMsg->pData, pMsg->nDataBytes, !ret ? ret : pDev->err);

   /* release the data mailbox write access */
   Vmmc_IrqUnlockDevice((pDev));
   if (!irq_context)
       VMMC_OS_MutexRelease (&(pDev)->mtxDataMbxAcc);

   return ret;
}

#endif /* VMMC_FEAT_PACKET */
