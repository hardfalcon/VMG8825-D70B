/******************************************************************************

                            Copyright (c) 2006-2016
                        Lantiq Beteiligungs-GmbH & Co.KG

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/** \file drv_tapi_stream.c
    Data stream fifos and buffers for TAPI.
    This module provides management of the fifos and buffers for voice data
    transport in TAPI. */


/* ============================= */
/* Includes                      */
/* ============================= */
#include "drv_tapi.h"

#ifdef TAPI_FEAT_PACKET

#include "drv_tapi_stream.h"
#include "drv_tapi_errno.h"

/* ============================= */
/* Local Macros & Definitions    */
/* ============================= */

/* Parameters for the voice packet bufferpool. */

#ifndef VOICE_PACKET_SIZE
   /** Size of one VOICE PACKET. */
   #define VOICE_PACKET_SIZE 576
#endif /* VOICE_PACKET_SIZE */

#ifndef VOICE_PACKET_COUNT
   /** Initial number of packets in the VOICE bufferpool. */
   #define VOICE_PACKET_COUNT 128
#endif /* VOICE_PACKET_COUNT */

#ifndef VOICE_PACKET_GROW
   /** When bufferpool is full, grow it by this number of packets. */
   #define VOICE_PACKET_GROW 32
#endif /* VOICE_PACKET_GROW */

#ifndef VOICE_PACKET_LIMIT
   /** Limit bufferpool's automatic growth. */
   #define VOICE_PACKET_LIMIT 1024
#endif /* VOICE_PACKET_LIMIT */

/* Parameters for the fifos in each channel to queue voice packets. */

#ifndef VOICE_UP_FIFO_SIZE
   /** Size of FIFO for UpStream packets. */
   #define VOICE_UP_FIFO_SIZE 30
#endif /* VOICE_UP_FIFO_SIZE */

#ifndef VOICE_DOWN_FIFO_SIZE
   /** Size of FIFO for DownStream packets. (Only used for polling.) */
   #define VOICE_DOWN_FIFO_SIZE 16
#endif /* VOICE_DOWN_FIFO_SIZE */

/* ============================= */
/* Local type definitions        */
/* ============================= */

/** Internal struct that is put as an element into the packet fifos and keeps
    the actual packet buffer together with packet related extra information. */
struct IFX_TAPI_PKT_FIFO_ELEM
{
   /** Pointer to a buffer from lib-bufferpool with the payload data. */
   IFX_void_t          *pBuf;
   /** Length of the data in the buffer. */
   IFX_uint32_t         nLength;
   /** Offset from buffer header to the data counted in bytes. */
   IFX_uint32_t         nDataOffset;
   /* add additional attributes here such as length, type, ... */
};

/* ============================= */
/* Global variable definition    */
/* ============================= */

/* ============================= */
/* Local function declaration    */
/* ============================= */
static IFX_int32_t ifx_tapi_ClearFifo__unlocked(
                        FIFO_ID* pFifo);

static IFX_void_t ifx_tapi_UpStreamFifo_Protect(
                        TAPI_CHANNEL* pTapiCh,
                        IFX_TAPI_STREAM_t nStream);

static IFX_void_t ifx_tapi_UpStreamFifo_Unprotect(
                        TAPI_CHANNEL* pTapiCh,
                        IFX_TAPI_STREAM_t nStream);

#ifdef TAPI_FEAT_POLL
static IFX_int32_t ifx_tapi_DownStreamFifo_Reset(TAPI_DEV* pTapiDev);
#endif /* TAPI_FEAT_POLL */

/* ============================= */
/* Local variable definition     */
/* ============================= */
/** Memory of free buffers for voice packets. */
static BUFFERPOOL* pVoicePktBuffer = IFX_NULL;

/* Buffer pool access protection */
#ifdef TAPI_FEAT_LINUX_SMP
static DEFINE_SPINLOCK(lockVoiceBufferPoolAcc);
#else /* TAPI_FEAT_LINUX_SMP */
static TAPI_OS_mutex_t lockVoiceBufferPoolAcc;
#endif /* TAPI_FEAT_LINUX_SMP */


/* ============================= */
/* Function definitions          */
/* ============================= */

/**
   Reset FIFO, free elements in in fifo and clear it.

   \param  pFifo        Pointer to FIFO. Must not be NULL.

   \return
   Returns TAPI_statusErr in case of an error, otherwise returns TAPI_statusOk.
*/
static IFX_int32_t ifx_tapi_ClearFifo__unlocked(FIFO_ID* pFifo)
{
   struct IFX_TAPI_PKT_FIFO_ELEM fifo_elem;

   if (pFifo == IFX_NULL)
      return TAPI_statusFIFO;

   memset(&fifo_elem, 0, sizeof(fifo_elem));

   /* Free all voice buffers which are still in the fifo. */
   while (fifoGet(pFifo, &fifo_elem) == IFX_SUCCESS)
   {
      IFX_TAPI_VoiceBufferPut(fifo_elem.pBuf);
   }

   /* Now reset fifo */
   fifoReset(pFifo);

   return TAPI_statusOk;
}


/**
   Initialize UpStream FIFO.

   \param  pChannel     Pointer to TAPI channel. Must not be NULL.

   \return
   Returns TAPI_statusErr in case of an error, otherwise returns TAPI_statusOk.
*/
IFX_int32_t IFX_TAPI_UpStreamFifo_Create (TAPI_CHANNEL* pChannel)
{
   IFX_TAPI_STREAM_t nStream;

   for (nStream = IFX_TAPI_STREAM_COD; nStream < IFX_TAPI_STREAM_MAX; nStream++)
   {

      if (pChannel->nChannel <
          pChannel->pTapiDevice->pDevDrvCtx->readChannels[nStream])
      {
         /* Initialize packet voice fifo. */
         if (IFX_NULL != pChannel->pUpStreamFifo[nStream])
         {
            /* Reset fifo */
            IFX_TAPI_UpStreamFifo_Reset(pChannel, nStream);
         }
         else
         {
            /* Create fifo */
            TAPI_OS_SPIN_LOCK_INIT(&pChannel->sl_up_stream_fifo[nStream]);
            pChannel->pUpStreamFifo[nStream] =
               fifoInit(VOICE_UP_FIFO_SIZE,
                        sizeof(struct IFX_TAPI_PKT_FIFO_ELEM));
            if (IFX_NULL == pChannel->pUpStreamFifo[nStream])
            {
               /* ERROR: Packet FIFO initialization failed, check fifoInit
                description. */
               RETURN_STATUS (TAPI_statusFIFO, 0);

            }
         }
      }
   }

   return TAPI_statusOk;
}


/**
   Protect the UpStreamFifo from concurrent access

   \param  pChannel     Pointer to TAPI channel. Must not be NULL.
   \param  nStream      Selects which FIFO to use from IFX_TAPI_STREAM_t.
*/
static IFX_void_t ifx_tapi_UpStreamFifo_Protect(
                        TAPI_CHANNEL* pTapiCh,
                        IFX_TAPI_STREAM_t nStream)
{
#ifdef TAPI_FEAT_LINUX_SMP
   TAPI_OS_SPIN_LOCK_IRQSAVE(&pTapiCh->sl_up_stream_fifo[nStream]);
#else /* TAPI_FEAT_LINUX_SMP */
   IFX_UNUSED(nStream);
   TAPI_OS_MutexGet(&pTapiCh->semTapiChDataLock);
   if (IFX_TAPI_PtrChk(pTapiCh->pTapiDevice->pDevDrvCtx->IRQ.LockDevice))
   {
      pTapiCh->pTapiDevice->pDevDrvCtx->IRQ.LockDevice(
                                                pTapiCh->pTapiDevice->pLLDev);
   }
#endif /* TAPI_FEAT_LINUX_SMP */
}


/**
   Release protection against concurrent access on the UpStreamFifo

   \param  pChannel     Pointer to TAPI channel. Must not be NULL.
   \param  nStream      Selects which FIFO to use from IFX_TAPI_STREAM_t.
*/
static IFX_void_t ifx_tapi_UpStreamFifo_Unprotect(
                        TAPI_CHANNEL* pTapiCh,
                        IFX_TAPI_STREAM_t nStream)
{
#ifdef TAPI_FEAT_LINUX_SMP
   TAPI_OS_SPIN_UNLOCK_IRQRESTORE(&pTapiCh->sl_up_stream_fifo[nStream]);
#else /* TAPI_FEAT_LINUX_SMP */
   IFX_UNUSED(nStream);
   if (pTapiCh->pTapiDevice->pDevDrvCtx->IRQ.UnlockDevice != IFX_NULL)
   {
      pTapiCh->pTapiDevice->pDevDrvCtx->IRQ.UnlockDevice (
                                                pTapiCh->pTapiDevice->pLLDev);
   }
   TAPI_OS_MutexRelease (&pTapiCh->semTapiChDataLock);
#endif /* TAPI_FEAT_LINUX_SMP */
}


/**
   Reset UpStream FIFO.

   \param  pTapiCh      Pointer to TAPI channel. Must not be NULL.
   \param  nStream      Selects which FIFO to use from IFX_TAPI_STREAM_t.

   \return
   Returns TAPI_statusErr in case of an error, otherwise returns TAPI_statusOk.
*/
IFX_int32_t IFX_TAPI_UpStreamFifo_Reset(
                        TAPI_CHANNEL* pChannel,
                        IFX_TAPI_STREAM_t nStream)
{
   IFX_int32_t ret;

   /* sanity check if this channel has a fifo */
   if ((nStream < IFX_TAPI_STREAM_MAX) &&
       (pChannel->pUpStreamFifo[nStream] != IFX_NULL))
   {
      ifx_tapi_UpStreamFifo_Protect(pChannel, nStream);
      ret = ifx_tapi_ClearFifo__unlocked(pChannel->pUpStreamFifo[nStream]);
      ifx_tapi_UpStreamFifo_Unprotect(pChannel, nStream);
   }
   else
   {
      ret = TAPI_statusFIFO;
   }

   RETURN_STATUS (ret, 0);
}


/**
   Reset UpStream FIFO.

   \param  pChannel  Handle to the TAPI channel
   \param  nStream   FIFO selector \ref IFX_TAPI_STREAM_t.

   \return TAPI_statusOk on success
*/
IFX_int32_t TAPI_PKT_Flush (TAPI_CHANNEL* pChannel)
{
   return IFX_TAPI_UpStreamFifo_Reset (pChannel, IFX_TAPI_STREAM_COD);
}

static IFX_int32_t IFX_TAPI_UpStreamFifo_Counters__unlocked(
                        TAPI_CHANNEL* pChannel,
                        IFX_TAPI_STREAM_t nStream,
                        IFX_TAPI_UPSTREAM_FIFO_COUNTERS_t *pCounters)
{

   pCounters->nWaiting = fifoElements(pChannel->pUpStreamFifo[nStream]);

   return TAPI_statusOk;
}

/**
   Return counters from the selected upstream fifo of the given channel.

   \param  pChannel     Pointer to TAPI channel. Must not be NULL.
   \param  nStream      Selects which FIFO to use from IFX_TAPI_STREAM_t.
   \param  pCounters    Pointer to struct in which the counters are returned.
                        Content is valid only if return is TAPI_statusOk.

   \return
   Returns TAPI_statusErr in case of an error, otherwise returns TAPI_statusOk.
*/
IFX_int32_t IFX_TAPI_UpStreamFifo_Counters(
                        TAPI_CHANNEL* pChannel,
                        IFX_TAPI_STREAM_t nStream,
                        IFX_TAPI_UPSTREAM_FIFO_COUNTERS_t *pCounters)
{
   IFX_int32_t retval;

   if ((pChannel == IFX_NULL) || (nStream >= IFX_TAPI_STREAM_MAX) ||
       (pChannel->pUpStreamFifo[nStream] == IFX_NULL))
      return TAPI_statusErr;

   TAPI_OS_SPIN_LOCK_IRQSAVE(&pChannel->sl_up_stream_fifo[nStream]);
   retval = IFX_TAPI_UpStreamFifo_Counters__unlocked(pChannel, nStream, pCounters);
   TAPI_OS_SPIN_UNLOCK_IRQRESTORE(&pChannel->sl_up_stream_fifo[nStream]);

   return retval;
}

static IFX_int32_t IFX_TAPI_UpStreamFifo_Put__unlocked(
                        TAPI_CHANNEL* pChannel,
                        IFX_TAPI_STREAM_t nStream,
                        const IFX_void_t * const pData,
                        const IFX_uint32_t nLength,
                        const IFX_uint32_t nOffset)
{
   IFX_int32_t ret = IFX_ERROR;

   struct IFX_TAPI_PKT_FIFO_ELEM fifo_elem;

   /* fill element to be put onto the fifo */
   fifo_elem.pBuf = (IFX_void_t *)pData;
   fifo_elem.nLength = nLength;
   fifo_elem.nDataOffset = nOffset;

   /* put element into the upstream fifo */
   ret = fifoPut(pChannel->pUpStreamFifo[nStream], &fifo_elem);

   return ret;
}

/**
   Add a new element to the UpStream FIFO.

   Adds a new element to the upstream fifo and if successful wakes up
   the event indicating that the fifo contains data.
   The function also updates the statistic counter for the error cases that
   the fifo is full or no fifo could be found. For the success case no
   statistic counter is updated - the packet then belongs to the fifo.

   \param  pChannel     Pointer to TAPI channel. Must not be NULL.
   \param  nStream      Selects which FIFO to use from IFX_TAPI_STREAM_t.
   \param  pData        Pointer that is stored in the fifo.
   \param  nLength      length information to be stored.
   \param  nOffset      offset information to be stored.

   \return
   SUCCESS or ERROR if no more element can be added to the FIFO
*/
IFX_int32_t IFX_TAPI_UpStreamFifo_Put(
                        TAPI_CHANNEL* pChannel,
                        IFX_TAPI_STREAM_t nStream,
                        const IFX_void_t * const pData,
                        const IFX_uint32_t nLength,
                        const IFX_uint32_t nOffset)
{
   IFX_int32_t ret = IFX_ERROR;

   /* sanity check if this channel has a fifo */
   if ((nStream < IFX_TAPI_STREAM_MAX) &&
       (pChannel->pUpStreamFifo[nStream] != IFX_NULL))
   {
      TAPI_OS_SPIN_LOCK_IRQSAVE(&pChannel->sl_up_stream_fifo[nStream]);
      ret = IFX_TAPI_UpStreamFifo_Put__unlocked(pChannel, nStream,
                                                pData, nLength, nOffset);
      TAPI_OS_SPIN_UNLOCK_IRQRESTORE(&pChannel->sl_up_stream_fifo[nStream]);

      if (ret == IFX_SUCCESS)
      {
         /* Wake up an already sleeping blocking read. */
         if (!(pChannel->nFlags & CF_NONBLOCK))
         {
            /* data available, wake up waiting upstream function */
            TAPI_OS_EventWakeUp (&pChannel->semReadBlock);
         }
         /* Wake up a sleeping select call.  */
         if (pChannel->nFlags & CF_NEED_WAKEUP)
         {
            /* don't wake up any more */
            pChannel->nFlags &= ~CF_NEED_WAKEUP;
            TAPI_OS_DrvSelectQueueWakeUp (&pChannel->wqRead,
                                          TAPI_OS_DRV_SEL_WAKEUP_TYPE_RD);
         }
      }
      else
      {
         /* Packet was not stored in the fifo. Fifo is congested. */
         IFX_TAPI_Stat_Add(pChannel, nStream,
                           TAPI_STAT_COUNTER_EGRESS_CONGESTED, 1);
      }
   }
   else
   {
      /* No fifo to store a packet found. Packet will be dropped. */
      IFX_TAPI_Stat_Add(pChannel, nStream,
                        TAPI_STAT_COUNTER_EGRESS_DISCARDED, 1);
   }

   return ret;
}


static IFX_void_t *IFX_TAPI_UpStreamFifo_Get__unlocked(
         TAPI_CHANNEL* pChannel,
         IFX_TAPI_STREAM_t nStream,
         IFX_uint32_t *pLength,
         IFX_uint32_t *pOffset)
{
   IFX_void_t *ret = IFX_NULL;
   struct IFX_TAPI_PKT_FIFO_ELEM fifo_elem;

   memset(&fifo_elem, 0, sizeof(fifo_elem));

   if (fifoGet(pChannel->pUpStreamFifo[nStream], &fifo_elem) == IFX_SUCCESS)
   {
      /* copy data from the fifo element */
      ret = fifo_elem.pBuf;
      if (pLength != IFX_NULL)
      {
         *pLength = fifo_elem.nLength;
      }
      if (pOffset != IFX_NULL)
      {
         *pOffset = fifo_elem.nDataOffset;
      }
   }

   return ret;
}


/**
   Get an element from the UpStream FIFO.

   \param  pChannel     Pointer to TAPI channel. Must not be NULL.
   \param  nStream      Selects which FIFO to use from IFX_TAPI_STREAM_t.
   \param  pLength      Pointer where length information is to be stored.
   \param  pOffset      Pointer where offset information is to be stored.

   \return
   pointer that was stored in the fifo -- IFX_NULL if empty or on error.
*/
IFX_void_t *IFX_TAPI_UpStreamFifo_Get(
                        TAPI_CHANNEL* pTapiCh,
                        IFX_TAPI_STREAM_t nStream,
                        IFX_uint32_t *pLength,
                        IFX_uint32_t *pOffset)
{
   IFX_void_t *ret = IFX_NULL;

   /* sanity check if this channel has a fifo */
   if ((nStream < IFX_TAPI_STREAM_MAX) &&
       (pTapiCh->pUpStreamFifo[nStream] != IFX_NULL))
   {
      ifx_tapi_UpStreamFifo_Protect(pTapiCh, nStream);
      ret = IFX_TAPI_UpStreamFifo_Get__unlocked(
               pTapiCh, nStream, pLength, pOffset);
      ifx_tapi_UpStreamFifo_Unprotect(pTapiCh, nStream);
   }

   return ret;
}


/**
   Check if the UpStream FIFO is empty.

   \param  pChannel     Pointer to TAPI channel. Must not be NULL.
   \param  nStream      Selects which FIFO to use from IFX_TAPI_STREAM_t.

   \return
   IFX_TRUE if the fifo is empty or does not exist; IFX_FALSE otherwise.
*/
IFX_boolean_t IFX_TAPI_UpStreamFifo_IsEmpty(
                        TAPI_CHANNEL* pTapiCh,
                        IFX_TAPI_STREAM_t nStream)
{
   IFX_boolean_t ret = IFX_TRUE;

   /* sanity check if this channel has a fifo */
   if ((nStream < IFX_TAPI_STREAM_MAX) &&
       (pTapiCh->pUpStreamFifo[nStream] != IFX_NULL))
   {
      ifx_tapi_UpStreamFifo_Protect(pTapiCh, nStream);

      ret = (fifoEmpty(pTapiCh->pUpStreamFifo[nStream]) != 0) ?
            IFX_TRUE : IFX_FALSE;

      ifx_tapi_UpStreamFifo_Unprotect(pTapiCh, nStream);
   }

   return ret;
}


/**
   Delete the UpStream FIFOs.

   \param  pChannel      Pointer to TAPI channel. Must not be NULL.

   \return
   Returns IFX_ERROR in case of an error, otherwise returns IFX_SUCCESS.
*/
IFX_int32_t IFX_TAPI_UpStreamFifo_Delete(TAPI_CHANNEL* pChannel)
{
   IFX_TAPI_STREAM_t nStream;
   IFX_int32_t ret = IFX_SUCCESS;

   TAPI_ASSERT(pChannel != IFX_NULL);

   for (nStream = IFX_TAPI_STREAM_COD; nStream < IFX_TAPI_STREAM_MAX; nStream++)
   {
      if (pChannel->pUpStreamFifo[nStream] != IFX_NULL)
      {
         TAPI_OS_SPIN_LOCK_IRQSAVE(&pChannel->sl_up_stream_fifo[nStream]);

         ret = ifx_tapi_ClearFifo__unlocked(pChannel->pUpStreamFifo[nStream]);
         if (ret == IFX_SUCCESS)
         {
            ret = fifoFree (pChannel->pUpStreamFifo[nStream]);
            pChannel->pUpStreamFifo[nStream] = IFX_NULL;
         }

         TAPI_OS_SPIN_UNLOCK_IRQRESTORE(&pChannel->sl_up_stream_fifo[nStream]);
      }
   }

   return ret;
}


#ifdef TAPI_FEAT_POLL
/* The downstream FIFO is only needed for the polling implementation */

/**
   Initialize DownStream FIFO.

   \param  pTapiDev     Pointer to TAPI device. Must not be NULL.

   \return
   Returns TAPI_statusErr in case of an error, otherwise returns TAPI_statusOk.
*/
IFX_int32_t IFX_TAPI_DownStreamFifo_Create(TAPI_DEV* pTapiDev)
{
   /* Initialize packet voice fifo. */
   if (IFX_NULL != pTapiDev->pDownStreamFifo)
   {
      /* Reset fifo and return */
      return ifx_tapi_DownStreamFifo_Reset(pTapiDev);
   }

   TAPI_OS_SPIN_LOCK_INIT(&pTapiDev->sl_down_stream_fifo);
   pTapiDev->pDownStreamFifo =
      fifoInit(VOICE_DOWN_FIFO_SIZE /* elem count */,
               sizeof(struct IFX_TAPI_PKT_FIFO_ELEM));
   if (IFX_NULL == pTapiDev->pDownStreamFifo)
   {
      /* ERROR: Packet FIFO initialization failed, check fifoInit
                description. */
      RETURN_DEVSTATUS (TAPI_statusFIFO, 0);
   }

   return TAPI_statusOk;
}
#endif /* TAPI_FEAT_POLL */


#ifdef TAPI_FEAT_POLL
/**
   Reset DownStream FIFO.

   \param  pTapiDev     Pointer to TAPI device. Must not be NULL.

   \return
   Returns TAPI_statusErr in case of an error, otherwise returns TAPI_statusOk.
*/
IFX_int32_t ifx_tapi_DownStreamFifo_Reset(TAPI_DEV* pTapiDev)
{
   IFX_int32_t retval = TAPI_statusFIFO;

   if (IFX_NULL != pTapiDev->pDownStreamFifo)
   {
      retval = ifx_tapi_ClearFifo__unlocked(pTapiDev->pDownStreamFifo,
                                            &pTapiDev->sl_down_stream_fifo);
   }

   RETURN_DEVSTATUS (retval, 0);
}
#endif /* TAPI_FEAT_POLL */


#ifdef TAPI_FEAT_POLL
/**
   Retrieve handle to DownStream FIFO.

   \param  pTapiDev     Pointer to TAPI device. Must not be NULL.

   \return
   Returns IFX_NULL in case of an error, otherwise returns fifo handle.
*/
IFX_void_t* IFX_TAPI_DownStreamFifo_Handle_Get(TAPI_DEV* pTapiDev)
{
   if (IFX_NULL == pTapiDev)
   {
      /* Wrong input arguments */
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
           ("Invalid input argument(s). (File: %s, line: %d)\n",
            __FILE__, __LINE__));
      return IFX_NULL;
   }

   return pTapiDev->pDownStreamFifo;
}
#endif /* TAPI_FEAT_POLL */


#ifdef TAPI_FEAT_POLL
/**
   Delete DownStream FIFO.

   \param  pTapiDev     Pointer to TAPI device. Must not be NULL.

   \return
   Returns IFX_ERROR in case of an error, otherwise returns IFX_SUCCESS.
*/
IFX_int32_t IFX_TAPI_DownStreamFifo_Delete(TAPI_DEV* pTapiDev)
{
   IFX_int32_t ret = IFX_SUCCESS;

   if (pTapiDev->pDownStreamFifo != IFX_NULL)
   {
      TAPI_OS_SPIN_LOCK_IRQSAVE(&pTapiDev->sl_down_stream_fifo);
      ret = fifoReset(pTapiDev->pDownStreamFifo);
      if (ret == IFX_SUCCESS)
      {
         ret = fifoFree(pTapiDev->pDownStreamFifo);
      }
      pTapiDev->pDownStreamFifo = IFX_NULL;
      TAPI_OS_SPIN_UNLOCK_IRQRESTORE(&pTapiDev->sl_down_stream_fifo);
   }

   return ret;
}
#endif /* TAPI_FEAT_POLL */


/**
   Set or clear downstream data request.

   Wake up the application to request downstream data.

   \param  pChannel     Pointer to TAPI channel. Must not be NULL.
   \param  bRequest     Set or clear the request.
                        - IFX_TRUE: Request data.
                        - IFX_FALSE: Clear data request.
*/
void IFX_TAPI_DownStream_RequestData(
                        TAPI_CHANNEL* pChannel,
                        IFX_boolean_t bRequest)
{
   if (bRequest == IFX_TRUE)
   {
      pChannel->bFaxDataRequest = IFX_TRUE;
      TAPI_OS_DrvSelectQueueWakeUp (&pChannel->wqWrite,
                                    TAPI_OS_DRV_SEL_WAKEUP_TYPE_WR);
   }
   else
   {
      pChannel->bFaxDataRequest = IFX_FALSE;
   }
}


/**
   Prepare bufferpool for voice packets.

   There is just one global pool for the entire tapi. It will be initialised
   with the first call and subsequent calls just return and create nothing new.

   \return
   Returns TAPI_statusErr in case of an error, otherwise returns TAPI_statusOk.
*/
IFX_int32_t IFX_TAPI_VoiceBufferPool_Create(void)
{
   if (IFX_NULL != pVoicePktBuffer)
   {
      return TAPI_statusOk;
   }

   pVoicePktBuffer = bufferPoolInit(VOICE_PACKET_SIZE, VOICE_PACKET_COUNT,
                                    VOICE_PACKET_GROW, VOICE_PACKET_LIMIT);
   if (IFX_NULL == pVoicePktBuffer)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
            ("Failed to init bufferpool handle pVoicePktBuffer.\n"));
      return IFX_ERROR;
   }
   bufferPoolIDSet (pVoicePktBuffer, 20);

   TRACE(TAPI_DRV, DBG_LEVEL_LOW, ("pVoicePktBuffer initialized "
         "%d buffers free.\n", bufferPoolAvail(pVoicePktBuffer)));

#ifndef TAPI_FEAT_LINUX_SMP
   /* initialize buffer pool access protection semaphore */
   TAPI_OS_MutexInit (&lockVoiceBufferPoolAcc);
#endif /* TAPI_FEAT_LINUX_SMP */

   return TAPI_statusOk;
}


/**
   Retrieve a buffer for a voice packet.

   \return
   Returns IFX_NULL in case of an error, otherwise returns packet buffer.

   \remarks
   This function uses a global irq lock - multiple drivers may be loaded and
   all may try to retrieve buffers from this shared pool in irq context. So we
   lock the interrupt during access to the shared buffer pool.
*/
IFX_void_t* IFX_TAPI_VoiceBufferGet(void)
{
   return IFX_TAPI_VoiceBufferGetWithOwnerId (IFX_TAPI_BUFFER_OWNER_UNKNOWN);
}


/**
   Retrieve a buffer for a voice packet and set the ID of the caller.

   \param  ownerId      ID value provided by the caller.

   \return
   Returns IFX_NULL in case of an error, otherwise returns packet buffer.

   \remarks
   This function uses a global irq lock - multiple drivers may be loaded and
   all may try to retrieve buffers from this shared pool in irq context. So we
   lock the interrupt during access to the shared buffer pool.
*/
IFX_void_t* IFX_TAPI_VoiceBufferGetWithOwnerId (IFX_uint32_t ownerId)
{
   IFX_void_t* buf = IFX_NULL;
   TAPI_OS_INTSTAT lock;

   TAPI_OS_PROTECT_IRQLOCK(&lockVoiceBufferPoolAcc, lock);
   buf = (IFX_void_t *) bufferPoolGetWithOwnerId(pVoicePktBuffer, ownerId);
   TAPI_OS_UNPROTECT_IRQLOCK(&lockVoiceBufferPoolAcc, lock);

   return buf;
}


#ifdef TAPI_PACKET_OWNID
/**
   Update owner ID for voice packet buffer.

   \param  pData     Pointer to a buffer that was retrieved through a call of
                     \ref IFX_TAPI_VoiceBufferGet().
   \param  ownerId   ID value provided by the caller.

   \return IFX_SUCCESS or IFX_ERROR
*/
IFX_int32_t  IFX_TAPI_VoiceBufferChOwn(IFX_void_t *pb, IFX_uint32_t ownerID)
{
   IFX_int32_t ret;

   ret = bufferPoolChOwn(pb, ownerID);

   return ret;
}
#endif /* TAPI_PACKET_OWNID */


/**
   Discard a voice packet buffer.

   \param  pData  Pointer to a buffer that was retrieved through a call of
                  \ref IFX_TAPI_VoiceBufferGet().

   \return IFX_SUCCESS or IFX_ERROR

   \remarks
   This function uses a global irq lock - multiple drivers may be loaded and
   all may try to retrieve buffers from this shared pool in irq context. So we
   lock the interrupt during access to the shared buffer pool.
*/
IFX_int32_t IFX_TAPI_VoiceBufferPut(IFX_void_t *pData)
{
   TAPI_OS_INTSTAT lock;
   IFX_int32_t  ret;

   TAPI_OS_PROTECT_IRQLOCK(&lockVoiceBufferPoolAcc, lock);
   /* bufferPool has it's own return values, but they match IFX_SUCCESS,
       _ERROR */
   ret = bufferPoolPut((void *)pData);
   TAPI_OS_UNPROTECT_IRQLOCK(&lockVoiceBufferPoolAcc, lock);

   return ret;
}


/**
   Frees all buffers in the pool that are allocated and carry the given ID.

   \param  ownerId      ID value provided by the caller.

   \return IFX_SUCCESS or IFX_ERROR

   \remarks
   This function uses a global irq lock - multiple drivers may be loaded and
   all may try to retrieve buffers from this shared pool in irq context. So we
   lock the interrupt during access to the shared buffer pool.
*/
IFX_int32_t   IFX_TAPI_VoiceBufferFreeAllOwnerId (IFX_uint32_t ownerId)
{
   TAPI_OS_INTSTAT lock;
   IFX_int32_t  ret;

   TAPI_OS_PROTECT_IRQLOCK(&lockVoiceBufferPoolAcc, lock);
   ret = bufferPoolFreeAllOwnerId(pVoicePktBuffer, ownerId);
   TAPI_OS_UNPROTECT_IRQLOCK(&lockVoiceBufferPoolAcc, lock);

   return ret;
}


/**
   Retrieve the size of one element in the voice-packet bufferpool.

   \return Size of one element in bytes.
*/
IFX_uint32_t IFX_TAPI_VoiceBufferPool_ElementSizeGet(void)
{
   return VOICE_PACKET_SIZE;
}


/**
   Retrieve the overall number of elements of the voice-packet bufferpool.

   \return the overall number of elements
*/
IFX_int32_t IFX_TAPI_VoiceBufferPool_ElementCountGet(void)
{
   TAPI_OS_INTSTAT lock;
   IFX_int32_t  elements = 0;

   if (!pVoicePktBuffer)
      return 0;

   TAPI_OS_PROTECT_IRQLOCK(&lockVoiceBufferPoolAcc, lock);
   elements = bufferPoolSize( pVoicePktBuffer );
   TAPI_OS_UNPROTECT_IRQLOCK(&lockVoiceBufferPoolAcc, lock);

   return elements;
}


/**
   Retrieve the available (free) number of elements of the
   voice-packet bufferpool.

   \return the number of available elements
*/
IFX_int32_t IFX_TAPI_VoiceBufferPool_ElementAvailCountGet(void)
{
   TAPI_OS_INTSTAT lock;
   IFX_int32_t  elements = 0;

   if (!pVoicePktBuffer)
      return 0;

   TAPI_OS_PROTECT_IRQLOCK(&lockVoiceBufferPoolAcc, lock);
   elements = bufferPoolAvail( pVoicePktBuffer );
   TAPI_OS_UNPROTECT_IRQLOCK(&lockVoiceBufferPoolAcc, lock);

   return elements;
}


#ifdef TAPI_PACKET_OWNID
/**
   Print buffer information.

   \return 0 - continue enumeration
*/
static IFX_uint32_t ifx_tapi_VoiceBufferStatusShow (IFX_void_t *pArgs,
   IFX_void_t *pHandle,  IFX_uint32_t nOwnerID, IFX_uint32_t nState)
{
   TRACE (TAPI_DRV, DBG_LEVEL_HIGH, ("%p 0x%-8x %-5u\n", pHandle, nOwnerID, nState));

   return 0;
}


/**
   Print out voice buffer information
*/
IFX_void_t IFX_TAPI_VoiceBufferPoolStatusShow (void)
{
   TAPI_OS_INTSTAT lock;

   if (IFX_NULL == pVoicePktBuffer)
   {
      TRACE (TAPI_DRV, DBG_LEVEL_HIGH, (KERN_CRIT "Voice bufferpool has not been initialized yet.\n"));
      return;
   }

   TRACE (TAPI_DRV, DBG_LEVEL_HIGH, ("%-8s %-10s %-5s\n", "pointer", "owner_id", "state"));
   TAPI_OS_PROTECT_IRQLOCK(&lockVoiceBufferPoolAcc, lock);
   bufferPoolEnumerate (pVoicePktBuffer, ifx_tapi_VoiceBufferStatusShow, NULL);
   TAPI_OS_UNPROTECT_IRQLOCK(&lockVoiceBufferPoolAcc, lock);
}
#endif /* TAPI_PACKET_OWNID */


/**
   Destruct bufferpool for voice packets.

   \return
   Returns IFX_ERROR in case of an error, otherwise returns IFX_SUCCESS.
*/
IFX_int32_t IFX_TAPI_VoiceBufferPool_Delete(void)
{
   TAPI_OS_INTSTAT lock;
   IFX_int32_t ret = IFX_SUCCESS;

   TAPI_OS_PROTECT_IRQLOCK(&lockVoiceBufferPoolAcc, lock);
   if (pVoicePktBuffer != IFX_NULL)
   {
      if (bufferPoolFree(pVoicePktBuffer) != BUFFERPOOL_SUCCESS)
      {
         /* record the error but go with the other bufferpool anyhow */
         ret = IFX_ERROR;
      }
      pVoicePktBuffer = IFX_NULL;
   }
   TAPI_OS_UNPROTECT_IRQLOCK(&lockVoiceBufferPoolAcc, lock);

#ifndef TAPI_FEAT_LINUX_SMP
   /* delete the buffer pool access protection semaphore */
   TAPI_OS_MutexDelete (&lockVoiceBufferPoolAcc);
#endif /* TAPI_FEAT_LINUX_SMP */

   return ret;
}
#endif /* TAPI_FEAT_PACKET */
