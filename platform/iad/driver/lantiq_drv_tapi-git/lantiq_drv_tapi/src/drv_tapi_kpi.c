/*******************************************************************************

                            Copyright (c) 2007-2015
                        Lantiq Beteiligungs-GmbH & Co.KG
                             http://www.lantiq.com

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

*******************************************************************************/

/**
   \file drv_tapi_kpi.c
   This file contains the implementation of the "Kernel Packet Interface" (KPI).
   The KPI is used to exchange packetised data with other drivers.
*/

/* ========================================================================== */
/*                                 Includes                                   */
/* ========================================================================== */
#include "drv_tapi.h"

#ifdef TAPI_FEAT_KPI

#include "drv_tapi_kpi.h"
#include "lib_bufferpool.h"
#if defined (TAPI_FEAT_SRTP) && defined (TAPI_VERSION3)
#include "drv_tapi_srtp.h"
#include "rtp.h"
#endif /* TAPI_FEAT_SRTP && TAPI_VERSION3*/

/* ========================================================================== */
/*                             Macro definitions                              */
/* ========================================================================== */

/** Get group which is coded in the upper 4 bit of the channel parameter. */
#define KPI_GROUP_GET(channel)    (((channel) >> 12) & 0x000F)
/** Get the channel number without the group number in the upper 4 bits. */
#define KPI_CHANNEL_GET(channel)  ((channel) & 0x0FFF)
/** Definition of maximum KPI group that can be used (allowed range: 1 - 15). */
#define IFX_TAPI_KPI_MAX_GROUP              15

/* ========================================================================== */
/*                             Type definitions                               */
/* ========================================================================== */

/** Struct that holds all data for one KPI group. A group is the interface
    towards one specific driver. */
typedef struct
{
   /** egress fifo */
   FIFO_ID             *pEgressFifo;
   /** egress fifo protection */
   TAPI_OS_mutex_t      semProtectEgressFifo;
   /** control struct for SMP spinlock handing */
   TAPI_OS_spin_lock_s  sl_ProtectEgressFifo;

   /** congestion state of the egress fifo */
   IFX_boolean_t        bEgressFifoCongested;
   /** ingress fifo */
   FIFO_ID             *pIngressFifo;
   /** ingress fifo protection */
   TAPI_OS_mutex_t      semProtectIngressFifo;
   /** control struct for SMP spinlock handing */
   TAPI_OS_spin_lock_s  sl_ProtectIngressFifo;
   /** congestion state of the ingress fifo */
   IFX_boolean_t        bIngressFifoCongested;
   /** Map from KPI channel to the corresponding TAPI channel.
       The KPI channel is the index to this map. */
   TAPI_CHANNEL        *channel_map[IFX_TAPI_KPI_MAX_CHANNEL_PER_GROUP];
   /** Map from KPI channel to the stream that packets belong to. */
   IFX_TAPI_KPI_STREAM_t stream_map[IFX_TAPI_KPI_MAX_CHANNEL_PER_GROUP];
   /** optional KPI egress tasklet for this group */
#ifdef TAPI_FEAT_KPI_TASKLET
   IFX_void_t          *pEgressTasklet;
#endif /* TAPI_FEAT_KPI_TASKLET */
} IFX_TAPI_KPI_GROUP_t;

/** Struct that is put as an element into the fifos and keeps the data
    together with the channel information. The fields for the channels
    are used depending on the direction. In ingress direction only the
    TAPI_CHANNEL is valid and the IFX_TAPI_KPI_CH_t is undefined. In
    egress direction only the IFX_TAPI_KPI_CH_t is valid and the
    TAPI_CHANNEL field is undefined. */
typedef struct
{
   /** KPI channel this buffer is sent on (egress direction) */
   IFX_TAPI_KPI_CH_t    nKpiCh;
   /** TAPI channel this buffer is for (ingress direction) */
   TAPI_CHANNEL        *pTapiCh;
   /** Pointer to a buffer from lib-bufferpool with the payload data */
   IFX_void_t          *pBuf;
   /** Reserved for future use. Pointer to first data in the buffer */
   IFX_void_t          *pData ;
   /** Length of data in the buffer counted in bytes  */
   IFX_uint32_t         nDataLength;
   /** Stream that this packet is for */
   IFX_TAPI_KPI_STREAM_t nStream;
} IFX_TAPI_KPI_FIFO_ELEM_t;


/* ========================================================================== */
/*                             Global variables                               */
/* ========================================================================== */

/** Array with all KPI group specific data */
static IFX_TAPI_KPI_GROUP_t *kpi_group[IFX_TAPI_KPI_MAX_GROUP];
/** Array of semaphores that signals data in the egress fifo of a group */
static TAPI_OS_lock_t        semWaitOnEgressFifo[IFX_TAPI_KPI_MAX_GROUP];
/** One semaphore to signal data in any ingress fifo */
static TAPI_OS_lock_t        semWaitOnIngressFifo;
/** Hold information of the ingress worker thread */
static TAPI_OS_ThreadCtrl_t  ingressThread;
#ifdef TAPI_FEAT_KPI_TASKLET
/** global variable used to configure the ingress packet handling via
    insmod option */
extern IFX_int32_t           block_ingress_tasklet;
#endif /* TAPI_FEAT_KPI_TASKLET */
/** Translate KPI streams into TAPI streams */
const static IFX_TAPI_STREAM_t translateStreamTable[IFX_TAPI_KPI_STREAM_MAX] =
{
   /* IFX_TAPI_KPI_STREAM_COD  -> */ IFX_TAPI_STREAM_COD,
   /* IFX_TAPI_KPI_STREAM_DECT -> */ IFX_TAPI_STREAM_DECT,
   /* IFX_TAPI_KPI_STREAM_HDLC -> */ IFX_TAPI_STREAM_HDLC
};

/* ========================================================================== */
/*                           Function prototypes                              */
/* ========================================================================== */
static IFX_void_t ifx_tapi_KPI_IngressHandler (IFX_ulong_t foo);
#ifdef TAPI_FEAT_KPI_TASKLET
DECLARE_TASKLET(tl_kpi_ingress, ifx_tapi_KPI_IngressHandler, 0L);
#endif /* TAPI_FEAT_KPI_TASKLET */
static IFX_int32_t ifx_tapi_KPI_IngressThread (TAPI_OS_ThreadParams_t *pThread);
static IFX_return_t ifx_tapi_KPI_GroupInit(IFX_uint32_t nKpiGroup);


/* ========================================================================== */
/*                         Function implementation                            */
/* ========================================================================== */
/*lint -esym(529,lock)  lint cannot see that this is used in assembler code */

/**
   Initialise the Kernel Packet Interface (KPI)

   \return Return values are defined within the \ref IFX_return_t definition
   - IFX_SUCCESS  in case of success
   - IFX_ERROR if operation failed
*/
IFX_return_t IFX_TAPI_KPI_Init (void)
{
   IFX_uint8_t  i;

   /* set array of groups to NULL */
   memset(kpi_group, 0x00, sizeof(kpi_group));
   /* the groups are allocated later on configuration */

   /* create semaphore to signal data is in the ingress fifos */
   TAPI_OS_LockInit (&semWaitOnIngressFifo);
   /* inital state of the semaphore should be locked so take it */
   TAPI_OS_LockGet (&semWaitOnIngressFifo);

   /* Loop over all groups in the KPI */
   for (i = 0; i < IFX_TAPI_KPI_MAX_GROUP; i++)
   {
      /* create semaphore to signal data in the egress fifo */
      TAPI_OS_LockInit (&semWaitOnEgressFifo[i]);
      /* inital state of the semaphore should be locked so take it */
      TAPI_OS_LockGet (&semWaitOnEgressFifo[i]);
   }

   /* start a thread working on the ingress queues */
   return (0 == TAPI_OS_ThreadInit (&ingressThread, "TAPIkpi_in",
                              ifx_tapi_KPI_IngressThread,
                              0, TAPI_OS_THREAD_PRIO_HIGHEST, 0, 0)) ?
                              IFX_SUCCESS : IFX_ERROR;
}


/**
   Clean-up the Kernel Packet Interface (KPI)

   \return none
   \remarks
   There is currently no protection here during the cleanup phase. So the read
   and write functions may crash when done while the cleanup is called.
   So first shut down all clients using the KPI before calling the cleanup.
   If driver crashes on unload protection could be added later.
*/
IFX_void_t IFX_TAPI_KPI_Cleanup (void)
{
   IFX_TAPI_KPI_FIFO_ELEM_t  sElem;
   IFX_uint8_t               i, j;
   IFX_TAPI_KPI_STREAM_t     nStream;
   TAPI_OS_INTSTAT           lock;
   TAPI_CHANNEL             *pTapiCh;

   /* stop the task working on the ingress queues */
   TAPI_OS_THREAD_KILL (&ingressThread, &semWaitOnIngressFifo);

   /* Loop over all groups in the KPI */
   for (i = 0; i < IFX_TAPI_KPI_MAX_GROUP; i++)
   {
      if (kpi_group[i] != IFX_NULL)
      {
         /* Reset all stream switch structs to stop traffic into fifos of this
            KPI group. Loop over all KPI channels in this group. */
         for (j=0; j < IFX_TAPI_KPI_MAX_CHANNEL_PER_GROUP; j++)
         {
            /* lookup the tapi channel associated with a KPI channel */
            pTapiCh = kpi_group[i]->channel_map[j];
            if (pTapiCh != IFX_NULL)
            {
               /* take protection semaphore */
               TAPI_OS_MutexGet (&pTapiCh->semTapiChDataLock);
               /* global irq lock */
               TAPI_OS_LOCKINT(lock);

               /* send all streams to the application */
               for (nStream = IFX_TAPI_KPI_STREAM_COD;
                    nStream < IFX_TAPI_KPI_STREAM_MAX; nStream++)
               {
                  pTapiCh->pKpiStream[nStream].nKpiCh = 0;
                  pTapiCh->pKpiStream[nStream].pEgressFifo = IFX_NULL;
               }
               /* global irq unlock */
               TAPI_OS_UNLOCKINT(lock);
               /* release protection semaphore */
               TAPI_OS_MutexRelease (&pTapiCh->semTapiChDataLock);
            }
         }

         /* delete semaphores for protecting the fifos */
         TAPI_OS_MutexDelete (&kpi_group[i]->semProtectEgressFifo);
         TAPI_OS_MutexDelete (&kpi_group[i]->semProtectIngressFifo);

         memset(&sElem, 0, sizeof(sElem));

         /* flush the data fifos for egress and ingress direction */
         while (fifoGet(kpi_group[i]->pEgressFifo, &sElem) == IFX_SUCCESS)
         {
            if (IFX_TAPI_VoiceBufferPut(sElem.pBuf) != IFX_SUCCESS)
            {
               /* This should never happen! Warn but do not stop here. */
               TRACE (TAPI_DRV, DBG_LEVEL_HIGH,
                      ("\nBuffer put-back error(1)\n"));
            }
         }
         while (fifoGet(kpi_group[i]->pIngressFifo, &sElem) == IFX_SUCCESS)
         {
            if (IFX_TAPI_VoiceBufferPut(sElem.pBuf) != IFX_SUCCESS)
            {
               /* This should never happen! Warn but do not stop here. */
               TRACE (TAPI_DRV, DBG_LEVEL_HIGH,
                      ("\nBuffer put-back error(2)\n"));
            }
         }

         /* delete data fifos for ingress and egress direction */
         fifoFree (kpi_group[i]->pEgressFifo);
         fifoFree (kpi_group[i]->pIngressFifo);

         /* free the allocated group structures */
         TAPI_OS_Free (kpi_group[i]);
         kpi_group[i] = IFX_NULL;
      }
      /* delete semaphore to signal data in the egress fifo */
      TAPI_OS_LockDelete (&semWaitOnEgressFifo[i]);
   }

   TAPI_OS_LockDelete (&semWaitOnIngressFifo);
}


/**
   Sleep until data is available for reading with \ref IFX_TAPI_KPI_ReadData.

   \param  nKpiGroup    KPI group to wait on for new data.

   \return Returns value as follows:
   - IFX_SUCCESS:  Data is now available for reading in the specified
                        KPI group.
   - IFX_ERROR:    If invalid parameters were given or interrupted by
                        signal.
*/
IFX_return_t IFX_TAPI_KPI_WaitForData( IFX_TAPI_KPI_CH_t nKpiGroup )
{
   /* Get the KPI-group number */
   nKpiGroup = KPI_GROUP_GET(nKpiGroup);
   /* Reject group values which are out of the configured range. */
   if ((nKpiGroup == 0) || (nKpiGroup > IFX_TAPI_KPI_MAX_GROUP))
      return IFX_ERROR;
   /* Take the signalling semaphore - this is blocking until data is available
      or a signal is sent to the process. */
   if (TAPI_OS_LOCK_GET_INTERRUPTIBLE (&semWaitOnEgressFifo[nKpiGroup-1]) != 0)
   {
      /* interrupted by signal */
      return IFX_ERROR;
   }

   return IFX_SUCCESS;
}


/**
   Read function for KPI clients to read a packet from TAPI KPI.

   \param  nKpiGroup    KPI group where to read data from.
   \param  *nKpiChannel Returns the KPI channel number within the given
                        group where the packet was received.
   \param  **pPacket    Returns a pointer to a bufferpool element with the
                        received data. The ownership of the returned bufferpool
                        element is passed to the client calling this interface.
                        It is responsibility of the client to free this element
                        by calling \ref IFX_TAPI_VoiceBufferPut after having
                        processed the data.
   \param  *nPacketLength  Returns the length of the received data. If the
                        returned length is 0, it means that no packets were
                        available for reading.
   \param  *nMore       Returns whether more packets are ready to be read
                        within the same KPI group. 0 means no more packets
                        ready, 1 means more packets available for reading.

   \return
   Returns the number of data bytes successfully read or IFX_ERROR otherwise.
*/
IFX_int32_t IFX_TAPI_KPI_ReadData( IFX_TAPI_KPI_CH_t nKpiGroup,
                                   IFX_TAPI_KPI_CH_t *nKpiChannel,
                                   IFX_void_t **pPacket,
                                   IFX_uint32_t *nPacketLength,
                                   IFX_uint8_t *nMore)
{
   TAPI_OS_INTSTAT           lock;
   IFX_TAPI_KPI_FIFO_ELEM_t  sElem;
   IFX_TAPI_KPI_CH_t         nKpiChannelOnly;
   IFX_int32_t               ret;
#if defined (TAPI_FEAT_SRTP) && defined (TAPI_VERSION3)
   TAPI_CHANNEL             *pTapiCh;
   IFX_TAPI_EVENT_t          tapiEvent;
#endif

   /* clean return values */
   *pPacket = IFX_NULL;
   *nPacketLength = 0;
   *nMore = 0;

   /* Get the KPI-group number */
   nKpiGroup = KPI_GROUP_GET(nKpiGroup);
   /* Reject group values which are out of configured range or for groups
      which have not been configured yet. */
   if ((nKpiGroup == 0) || (nKpiGroup > IFX_TAPI_KPI_MAX_GROUP) ||
       (kpi_group[nKpiGroup-1] == IFX_NULL))
      return IFX_ERROR;
   /* Adjust group values from channel notation to internal representation */
   nKpiGroup--;

   memset(&sElem, 0, sizeof(sElem));

   /* The read access to the fifo is protected in two ways:
       First it is protected from concurrent reads with this function
       second it is protected from writing of new data in irq context. */

   /* take protection semaphore */
   if (!TAPI_OS_IN_INTERRUPT())
      TAPI_OS_MutexGet (&kpi_group[nKpiGroup]->semProtectEgressFifo);
   /* global irq lock */
   TAPI_OS_LOCKINT(lock);

   TAPI_OS_SPIN_LOCK_IRQSAVE(&kpi_group[nKpiGroup]->sl_ProtectEgressFifo);
   /* read element from fifo */
   ret = fifoGet(kpi_group[nKpiGroup]->pEgressFifo, &sElem);
   /* set the more flag */
   *nMore = fifoEmpty(kpi_group[nKpiGroup]->pEgressFifo) ? 0 : 1;
   TAPI_OS_SPIN_UNLOCK_IRQRESTORE(&kpi_group[nKpiGroup]->sl_ProtectEgressFifo);
   /* global irq unlock */
   TAPI_OS_UNLOCKINT(lock);
   /* release protection semaphore */
   if (!TAPI_OS_IN_INTERRUPT())
      TAPI_OS_MutexRelease (&kpi_group[nKpiGroup]->semProtectEgressFifo);

   /* when there was data in the fifo return values */
   if (ret == IFX_SUCCESS)
   {
      /* store return values in the parameters */
      *nKpiChannel = sElem.nKpiCh;
      *pPacket = sElem.pBuf;
      *nPacketLength = sElem.nDataLength;
      /* strip the group from the channel parameter */
      nKpiChannelOnly = KPI_CHANNEL_GET (sElem.nKpiCh);
      /* check range and update the statistic */
      if (nKpiChannelOnly < IFX_TAPI_KPI_MAX_CHANNEL_PER_GROUP)
      {
         IFX_TAPI_Stat_Add(kpi_group[nKpiGroup]->channel_map[nKpiChannelOnly],
                           translateStreamTable[
                           kpi_group[nKpiGroup]->stream_map[nKpiChannelOnly]],
                           TAPI_STAT_COUNTER_EGRESS_DELIVERED, 1);
      }
#if defined (TAPI_FEAT_SRTP) && defined (TAPI_VERSION3)
      pTapiCh = kpi_group[nKpiGroup]->channel_map[nKpiChannelOnly];
      /* Encrypt data KPI->Ethernet, encryption will be done in driver context and can be delayed,
       * data was passed here from IRQ context, but now processing can be delayed */
      if (pTapiCh->pSrtp->bSrtpEnabled == IFX_TRUE)
      {
         /*TAPI_OS_MutexGet (&semMutex);*/
         ret = (IFX_int32_t)srtp_protect(pTapiCh->pSrtp->tx_session, (char*)sElem.pBuf, &sElem.nDataLength);
         if (ret != err_status_ok)
         {
            memset(&tapiEvent, 0, sizeof(IFX_TAPI_EVENT_t));
            tapiEvent.id = IFX_TAPI_EVENT_SRTP_DISCARD;
            tapiEvent.module = IFX_TAPI_MODULE_TYPE_NONE;
            tapiEvent.data.srtp_discard.bSRTP_Lib_Tx_err = IFX_TRUE;
            tapiEvent.data.srtp_discard.SRTP_MKI = (IFX_uint32_t)ret;
            switch (ret)
            {
               case err_status_auth_fail:
                  pTapiCh->pSrtp->TapiSrtpData.nSRTP_DiscardAuth++;
                  tapiEvent.data.srtp_discard.bSRTCP_Auth = IFX_TRUE;
                  break;
               case err_status_key_expired:
                  pTapiCh->pSrtp->TapiSrtpData.nSRTP_DiscardLife++;
               default:
                  break;
            }

            /* Dispatch event in case if it was enabled and acknowledged */
            if (pTapiCh->pSrtp->bEvDiscardEn == IFX_TRUE)
            {
               IFX_TAPI_Event_Dispatch(pTapiCh, &tapiEvent);
               /* Disable event reporting to protect from events spam */
               pTapiCh->pSrtp->bEvDiscardEn = IFX_FALSE;
            }
            ret = IFX_ERROR;
         }
         else
            pTapiCh->pSrtp->TapiSrtpData.nSRTP_Tx++;
         /*TAPI_OS_MutexRelease (&semMutex);*/
         *nPacketLength = sElem.nDataLength;
         *pPacket = sElem.pBuf;
      }
#endif /* TAPI_FEAT_SRTP */
   }

   /* taking in consideration size of IFX_int32_t casting below should be safe */
   return (ret == IFX_SUCCESS) ? (IFX_int32_t) *nPacketLength : IFX_ERROR;
}

/**
   Schedule handler for KPI ingress direction

   \return
      None
*/
IFX_void_t IFX_TAPI_KPI_ScheduleIngressHandling (void)
{
   /* signal the event that there is data in one of the ingress fifos */
   TAPI_OS_LockRelease (&semWaitOnIngressFifo);
}

/**
   Write function for KPI clients to write a packet to TAPI KPI.

   \param  nKpiChannel  KPI channel the data is written to.
   \param  pPacket      Pointer to a bufferpool element with the data to be
                        written. Bufferpool element must be from the TAPI
                        voice buffer pool. Use \ref IFX_TAPI_VoiceBufferGet
                        to get a bufferpool element.
   \param  nPacketLength  Length of the data to be written in bytes.

   \return
   On success, the number of bytes written is returned (zero indicates
   nothing was written). On error, IFX_ERROR is returned.

   \remarks
   The ownership of the bufferpool element is only passed to the KPI if this
   call successfuly wrote all data. This is only the case if this function
   returns the same value as given in the parameter nPacketLength. When this
   write fails or fewer data than given was written the buffer still
   belongs to the caller and this has to discard it or write it again.
*/
IFX_int32_t IFX_TAPI_KPI_WriteData( IFX_TAPI_KPI_CH_t nKpiChannel,
                                    IFX_void_t *pPacket,
                                    IFX_uint32_t nPacketLength)
{
   IFX_TAPI_KPI_FIFO_ELEM_t sElem;
   TAPI_CHANNEL             *pTapiCh;
   TAPI_OS_INTSTAT           lock;
   IFX_uint32_t              space;
   IFX_int32_t               ret;
#if defined (TAPI_FEAT_SRTP) && defined (TAPI_VERSION3)
   IFX_TAPI_EVENT_t          tapiEvent;
#endif

   /* Get the KPI-group number */
   IFX_TAPI_KPI_CH_t nKpiGroup = KPI_GROUP_GET(nKpiChannel);
   /* Reject group values which are out of configured range or for groups
      which have not been configured yet. */
   if ((nKpiGroup == 0) || (nKpiGroup > IFX_TAPI_KPI_MAX_GROUP) ||
       (kpi_group[nKpiGroup-1] == IFX_NULL))
      return IFX_ERROR;
   /* strip the group from the channel parameter */
   nKpiChannel = KPI_CHANNEL_GET(nKpiChannel);
   /* reject channel values which are out of the configured range */
   if (nKpiChannel >= IFX_TAPI_KPI_MAX_CHANNEL_PER_GROUP)
      return IFX_ERROR;
   /* Adjust group values from channel notation to internal representation */
   nKpiGroup--;

   /* lookup the tapi channel associated with the kpi channel */
   pTapiCh = kpi_group[nKpiGroup]->channel_map[nKpiChannel];
   /* abort if no channel association exists */
   if (pTapiCh == IFX_NULL)
      return IFX_ERROR;

   /* store the tapi channel and pointer to the data buffer */
   sElem.pTapiCh = pTapiCh;
   sElem.nKpiCh = nKpiChannel;
   sElem.pBuf = sElem.pData = pPacket;
   sElem.nDataLength = nPacketLength;

   /* The write access to the fifo is protected in two ways:
       First it is protected from concurrent reads by the ingress thread
       second it is protected from writing of new data in irq context. */
#if defined (TAPI_FEAT_SRTP) && defined (TAPI_VERSION3)
   /* Decrypt SRTP message */
   if (pTapiCh->pSrtp->bSrtpEnabled == IFX_TRUE)
   {
      ret = (IFX_int32_t)srtp_unprotect(pTapiCh->pSrtp->rx_session,
            sElem.pBuf, &sElem.nDataLength);
      if (ret != err_status_ok)
      {
         memset(&tapiEvent, 0, sizeof(IFX_TAPI_EVENT_t));
         tapiEvent.id = IFX_TAPI_EVENT_SRTP_DISCARD;
         tapiEvent.module = IFX_TAPI_MODULE_TYPE_NONE;
         tapiEvent.data.srtp_discard.bSRTP_Lib_Rx_err = IFX_TRUE;
         tapiEvent.data.srtp_discard.SRTP_MKI = (IFX_uint32_t)ret;
         switch (ret)
         {
            case err_status_auth_fail:
               pTapiCh->pSrtp->TapiSrtpData.nSRTP_DiscardAuth++;
               tapiEvent.data.srtp_discard.bSRTCP_Auth = IFX_TRUE;
               break;
            case err_status_replay_fail:
               pTapiCh->pSrtp->TapiSrtpData.nSRTP_DiscardReplay++;
               tapiEvent.data.srtp_discard.bSRTP_Replay = IFX_TRUE;
               break;
            case err_status_replay_old:
               pTapiCh->pSrtp->TapiSrtpData.nSRTP_DiscardOld++;
               tapiEvent.data.srtp_discard.bSRTP_Replay = IFX_TRUE;
               break;
            case err_status_key_expired:
               pTapiCh->pSrtp->TapiSrtpData.nSRTP_DiscardLife++;
            default:
               break;
         }
         /* Dispatch event in case if it was enabled and acknowledged */
         if (pTapiCh->pSrtp->bEvDiscardEn == IFX_TRUE)
         {
            IFX_TAPI_Event_Dispatch(pTapiCh, &tapiEvent);
            /* Disable event reporting to protect from events spam,
             * event should be acknowledged */
            pTapiCh->pSrtp->bEvDiscardEn = IFX_FALSE;
         }
         ret = IFX_ERROR;
      }
      else
      {
         pTapiCh->pSrtp->TapiSrtpData.nSRTP_Rx++;
      }
   }
#endif /* TAPI_FEAT_SRTP && TAPI_VERSION3 */
   /* if ! in irq take protection semaphore */
   if (!TAPI_OS_IN_INTERRUPT())
      TAPI_OS_MutexGet (&kpi_group[nKpiGroup]->semProtectIngressFifo);

   /* here we are protected from changes from the ioctl so copy this data */
   sElem.nStream = kpi_group[nKpiGroup]->stream_map[nKpiChannel];

   /* global irq lock */
   TAPI_OS_LOCKINT(lock);
   TAPI_OS_SPIN_LOCK_IRQSAVE(&kpi_group[nKpiGroup]->sl_ProtectIngressFifo);

   /* store data to fifo */
   ret = fifoPut(kpi_group[nKpiGroup]->pIngressFifo, &sElem);
   /* free slots in the fifo for checking versus the threshold below */
   space = fifoSize(kpi_group[nKpiGroup]->pIngressFifo) -
           fifoElements(kpi_group[nKpiGroup]->pIngressFifo);

   TAPI_OS_SPIN_UNLOCK_IRQRESTORE(&kpi_group[nKpiGroup]->sl_ProtectIngressFifo);
   /* global irq unlock */
   TAPI_OS_UNLOCKINT(lock);
   /* if ! in irq release protection semaphore */
   if (!TAPI_OS_IN_INTERRUPT())
      TAPI_OS_MutexRelease (&kpi_group[nKpiGroup]->semProtectIngressFifo);

   /* if putting to fifo succeeded set flag that there is data in one of
      the ingress fifos otherwise cleanup */
   if (ret == IFX_SUCCESS)
   {
#ifdef TAPI_PACKET_OWNID
      if (sElem.nStream == IFX_TAPI_KPI_STREAM_HDLC)
         IFX_TAPI_VoiceBufferChOwn (pPacket, IFX_TAPI_BUFFER_OWNER_HDLC_KPI);
      else if (sElem.nStream == IFX_TAPI_KPI_STREAM_COD)
         IFX_TAPI_VoiceBufferChOwn (pPacket, IFX_TAPI_BUFFER_OWNER_COD_KPI);
#endif /* TAPI_PACKET_OWNID */

#ifdef TAPI_FEAT_KPI_TASKLET
      if (TAPI_OS_IN_INTERRUPT() && !block_ingress_tasklet)
      {
         tasklet_hi_schedule(&tl_kpi_ingress);
      }
      else
#endif /* TAPI_FEAT_KPI_TASKLET */
      {
         /* signal the event that there is data in one of the ingress fifos */
         IFX_TAPI_KPI_ScheduleIngressHandling();
      }
      /* Wait for some free slots in the fifo before resetting the congestion
         indication. This stops a too fast oscillation which occurs when it is
         reset upon the first free slot. */
      if (space > IFX_TAPI_KPI_INGRESS_FIFO_SIZE / 2)
      {
         /* fifo state is: not congested */
         kpi_group[nKpiGroup]->bIngressFifoCongested = IFX_FALSE;
      }
   }
   else
   {
      /* zero bytes handled */
      nPacketLength = 0;

      /* This case happens if the ingress thread is not fast enough to read
         the data from the fifo and put it into the firmware data mailbox.
         Send an event to the application to notify that we were too slow. */
      /* Report congestion of the fifo to the application - but only once */
      if (kpi_group[nKpiGroup]->bIngressFifoCongested == IFX_FALSE) {
         IFX_TAPI_EVENT_t  tapiEvent;
         memset(&tapiEvent, 0, sizeof(IFX_TAPI_EVENT_t));
         tapiEvent.id = IFX_TAPI_EVENT_KPI_INGRESS_FIFO_FULL;
         tapiEvent.module = IFX_TAPI_MODULE_TYPE_NONE;
         IFX_TAPI_Event_Dispatch(pTapiCh, &tapiEvent);
         /* fifo state is: congested */
         kpi_group[nKpiGroup]->bIngressFifoCongested = IFX_TRUE;
      }
      IFX_TAPI_Stat_Add(pTapiCh, translateStreamTable[sElem.nStream],
                        TAPI_STAT_COUNTER_INGRESS_CONGESTED, 1);
      /* TRACE (TAPI_DRV, DBG_LEVEL_LOW,
                ("INFO: KPI-group 0x%X ingress fifo full\n", nKpiGroup+1)); */
   }

   /* taking in consideration size of IFX_int32_t casting below should be safe */
   return (ret == IFX_SUCCESS) ? (IFX_int32_t) nPacketLength : IFX_ERROR;
}


/** Report an event from KPI client to TAPI.

   \param  nKpiChannel  KPI channel the data is written to.
   \param  tapiEvent    Pointer to TAPI event to report.

   \remarks
   The function will overwrite the TAPI channel in the given event with the
   TAPI channel that is associated with the given KPI channel.
*/
extern IFX_void_t   IFX_TAPI_KPI_ReportEvent (IFX_TAPI_KPI_CH_t nKpiChannel,
                                              IFX_TAPI_EVENT_t *pTapiEvent)
{
   TAPI_CHANNEL             *pTapiCh;

   /* Get the KPI-group number */
   IFX_TAPI_KPI_CH_t nKpiGroup = KPI_GROUP_GET(nKpiChannel);
   /* Reject group values which are out of configured range or for groups
      which have not been configured yet. */
   if ((nKpiGroup == 0) || (nKpiGroup > IFX_TAPI_KPI_MAX_GROUP) ||
       (kpi_group[nKpiGroup-1] == IFX_NULL))
      return;
   /* strip the group from the channel parameter */
   nKpiChannel = KPI_CHANNEL_GET(nKpiChannel);
   /* reject channel values which are out of the configured range */
   if (nKpiChannel >= IFX_TAPI_KPI_MAX_CHANNEL_PER_GROUP)
      return;
   /* Adjust group values from channel notation to internal representation */
   nKpiGroup--;

   /* lookup the tapi channel associated with the kpi channel */
   pTapiCh = kpi_group[nKpiGroup]->channel_map[nKpiChannel];
   /* abort if no channel association exists */
   if (pTapiCh == IFX_NULL)
      return;

   IFX_TAPI_Event_Dispatch(pTapiCh, pTapiEvent);
}


/**
   Function to put a packet from irq context into an KPI egress fifo.

   \param  pChannel     Handle to TAPI_CHANNEL structure.
   \param  stream       Stream Type id.
   \param  *pPacket     Pointer to a bufferpool element with the data to be
                        written.
   \param  nPacketLength  Length of the data to be written.

   \return
   Return values are defined within the \ref IFX_return_t definition:
   - IFX_SUCCESS  if buffer was successfully sent
   - IFX_ERROR    if buffer was not sent.

   \remarks
   In case of error the caller still owns the buffer and has to take care to it.
*/
IFX_int32_t irq_IFX_TAPI_KPI_PutToEgress(TAPI_CHANNEL *pChannel,
                                          IFX_TAPI_KPI_STREAM_t stream,
                                          IFX_void_t *pPacket,
                                          IFX_uint32_t nPacketLength)
{
   IFX_TAPI_KPI_STREAM_SWITCH *pStreamSwitch = &pChannel->pKpiStream[stream];
   IFX_TAPI_KPI_FIFO_ELEM_t    sElem;
   TAPI_OS_INTSTAT             lock;
   IFX_int32_t                 ret = IFX_ERROR;
   IFX_TAPI_KPI_CH_t           k_grp;

   /* Protect fifo access from concurrent writes by other drivers in
      irq context by locking the irq globally. */
   TAPI_OS_LOCKINT(lock);

   k_grp = KPI_GROUP_GET(pStreamSwitch->nKpiCh)-1;

   /* fill the element struct with data */
   sElem.nKpiCh = pStreamSwitch->nKpiCh;
   sElem.pBuf = sElem.pData = pPacket;
   sElem.nDataLength = nPacketLength;


   /* store data to fifo */
   ret = fifoPut(pStreamSwitch->pEgressFifo, (IFX_void_t *)&sElem);

   /* signal the event that now there is data in the fifo
      Even if the put failed because the fifo is full trigger the
      processing to avoid a possible deadlock caused by the client. */
#ifdef TAPI_FEAT_KPI_TASKLET
   if ((kpi_group[k_grp]->pEgressTasklet) && (TAPI_OS_IN_INTERRUPT()))
   {
      tasklet_hi_schedule((struct tasklet_struct*) kpi_group[k_grp]->pEgressTasklet);
   }
   else
#endif /* TAPI_FEAT_KPI_TASKLET */
   {
      TAPI_OS_LockRelease (&semWaitOnEgressFifo[k_grp]);
   }

   if (ret == IFX_SUCCESS)
   {
      if (kpi_group[k_grp]->bEgressFifoCongested != IFX_FALSE)
      {
         IFX_uint32_t space;
         /* free slots in the egress fifo */
         space = fifoSize(pStreamSwitch->pEgressFifo) -
                 fifoElements(pStreamSwitch->pEgressFifo);
         if (space > IFX_TAPI_KPI_EGRESS_FIFO_SIZE / 2)
         {
            /* fifo state is: not congested */
            kpi_group[k_grp]->bEgressFifoCongested = IFX_FALSE;
         }
      }
   }
   else
   {
      /* This happens when the client for this KPI group fails to get
         the data from the fifo. When the caller discards the packet
         voice data is lost in this case. */

      if (kpi_group[k_grp]->bEgressFifoCongested == IFX_FALSE) {
         IFX_TAPI_EVENT_t  tapiEvent;
         memset(&tapiEvent, 0, sizeof(IFX_TAPI_EVENT_t));
         tapiEvent.id = IFX_TAPI_EVENT_KPI_EGRESS_FIFO_FULL;
         tapiEvent.module = IFX_TAPI_MODULE_TYPE_NONE;
         IFX_TAPI_Event_Dispatch(pChannel, &tapiEvent);
         /* fifo state is: congested */
         kpi_group[k_grp]->bEgressFifoCongested = IFX_TRUE;
         /* trace for debugging */
         TRACE (TAPI_DRV, DBG_LEVEL_HIGH,
                ("WARN: KPI-grp 0x%X egress fifo full\n", k_grp));
      }
   }

   /* global irq unlock */
   TAPI_OS_UNLOCKINT(lock);

   /* If putting to fifo failed IFX_ERROR is returned to the caller.
      The data buffer then still belongs to the caller who has to free it. */
   return (ret == IFX_SUCCESS) ? IFX_SUCCESS : IFX_ERROR;
}


/**
   Function to handle the KPI ingress direction

   Depending on the context it might be executed in a kernel thread or
   with Linux in a tasklet context

   \param foo        unused, but required for tasklets
   \return           void
*/
static IFX_void_t ifx_tapi_KPI_IngressHandler (IFX_ulong_t foo)
{
   IFX_TAPI_KPI_FIFO_ELEM_t  sElem;
   IFX_uint8_t               nThisGroup;
   TAPI_OS_INTSTAT           lock;
   IFX_int32_t               ret;
   /* Some KPI groups can not lost packages from FIFO while error
      occurred on LL side. Therefore that group has to be skipped
      in order not to block other groups handling. */
   IFX_boolean_t             bSkipGroup;

   IFX_UNUSED (foo);

   for (nThisGroup = 0; nThisGroup < IFX_TAPI_KPI_MAX_GROUP; nThisGroup++)
   {
#ifdef TAPI_FEAT_KPI_TASKLET
      /* don't handle HDLC group in tasklet mode */
      if (TAPI_OS_IN_INTERRUPT() && (nThisGroup == ((IFX_TAPI_KPI_HDLC>>12)-1)))
         continue;
#endif /* TAPI_FEAT_KPI_TASKLET */
      bSkipGroup = IFX_FALSE;

      /* if the group do not needed to skip and configured,
         check if there is data in the fifo of this group */
      while (bSkipGroup == IFX_FALSE &&
             kpi_group[nThisGroup] &&
             !fifoEmpty(kpi_group[nThisGroup]->pIngressFifo))
      {
         memset(&sElem, 0, sizeof(sElem));

         /* protect fifo get access from other tasks and any irq's
            Locking individual irq's is too costly so we lock globally */
         if (!TAPI_OS_IN_INTERRUPT())
            TAPI_OS_MutexGet (&kpi_group[nThisGroup]->semProtectIngressFifo);
         TAPI_OS_LOCKINT(lock);
         TAPI_OS_SPIN_LOCK_IRQSAVE(&kpi_group[nThisGroup]->sl_ProtectIngressFifo);
         /* read element from fifo */
         ret = fifoPeek(kpi_group[nThisGroup]->pIngressFifo, &sElem);
         TAPI_OS_SPIN_UNLOCK_IRQRESTORE(&kpi_group[nThisGroup]->sl_ProtectIngressFifo);
         TAPI_OS_UNLOCKINT(lock);
         if (!TAPI_OS_IN_INTERRUPT())
            TAPI_OS_MutexRelease (&kpi_group[nThisGroup]->semProtectIngressFifo);

         if (ret == IFX_SUCCESS)
         {
            /* we got data from the fifo */
            TAPI_CHANNEL *pTapiCh = sElem.pTapiCh;
            IFX_TAPI_DRV_CTX_t *pDrvCtx = pTapiCh->pTapiDevice->pDevDrvCtx;
            IFX_TAPI_LL_CH_t* pCh = pTapiCh->pLLChannel;
            IFX_int32_t size = 0;

            /* write data to the mailbox using LL function */
            if (pDrvCtx->Write)
            {
               size = pDrvCtx->Write(pCh, sElem.pBuf, sElem.nDataLength,
                                     (IFX_int32_t*)IFX_NULL,
                                     translateStreamTable[sElem.nStream]);

               /* if forwarding to low level write routine failed... */
               if (size <= 0)
               {
                  if (sElem.nStream == IFX_TAPI_KPI_STREAM_HDLC)
                  {
                     /* leave element in FIFO and skip that group */
                     bSkipGroup = IFX_TRUE;
                     continue; /* while (!fifoEmpty) */
                  }

                  /* The mailbox congestion event was already sent by the
                     lower layer so no need to do it here again. */
                  /* writing to mailbox failed - discard data */
                  if (IFX_TAPI_VoiceBufferPut(sElem.pBuf) != IFX_SUCCESS)
                     TRACE (TAPI_DRV, DBG_LEVEL_HIGH,
                           ("\nBuffer put-back error(6)\n"));
               }
            }
            else
            {
               TRACE(TAPI_DRV, DBG_LEVEL_LOW,
                     ("TAPI_DRV: LL-driver does not provide packet write\n"));
            }

            if (!TAPI_OS_IN_INTERRUPT())
               TAPI_OS_MutexGet (&kpi_group[nThisGroup]->semProtectIngressFifo);
            TAPI_OS_LOCKINT(lock);
            TAPI_OS_SPIN_LOCK_IRQSAVE(&kpi_group[nThisGroup]->sl_ProtectIngressFifo);
            /* remove element from FIFO */
            (void)fifoGet(kpi_group[nThisGroup]->pIngressFifo, &sElem);
            TAPI_OS_SPIN_UNLOCK_IRQRESTORE(&kpi_group[nThisGroup]->sl_ProtectIngressFifo);
            TAPI_OS_UNLOCKINT(lock);
            if (!TAPI_OS_IN_INTERRUPT())
               TAPI_OS_MutexRelease (&kpi_group[nThisGroup]->semProtectIngressFifo);
         }
      }
   }
}


/**
   Function to be executed from a thread to serve all ingress fifos of all
   KPI groups.

   This function is started as a thread and runs in an endless loop.
   It sleeps until data in an ingress fifo is signalled with an event.
   Upon wakeup the ingress fifos are searched in a round-robin manner
   for data and the first data found is written to the FW downstream mailbox.
   This function returns only when the thread is explicitly terminated
   on cleanup.

   \param  *pThread     Pointer to thread parameters
   \return  IFX_SUCCESS
*/
static IFX_int32_t ifx_tapi_KPI_IngressThread (TAPI_OS_ThreadParams_t *pThread)
{
   /* acquire realtime priority. */
   TAPI_OS_THREAD_PRIORITY_MODIFY (TAPI_OS_THREAD_PRIO_HIGHEST);

   /* main loop is waiting for the event that data is available */
   while ((pThread->bShutDown == IFX_FALSE) &&
          (TAPI_OS_LOCK_GET_INTERRUPTIBLE (&semWaitOnIngressFifo) ==
                                                                 IFX_SUCCESS) &&
          (pThread->bShutDown == IFX_FALSE))
   {
      ifx_tapi_KPI_IngressHandler(0L);
   }

   return IFX_SUCCESS;
}


/**

*/
static IFX_return_t ifx_tapi_KPI_GroupInit(IFX_uint32_t nKpiGroup)
{
   /* if group has not yet been initialised create it now */
   if ((nKpiGroup > 0) && (kpi_group[nKpiGroup-1] == IFX_NULL))
   {
      kpi_group[nKpiGroup-1] = (IFX_TAPI_KPI_GROUP_t *)
                               TAPI_OS_Malloc (sizeof(IFX_TAPI_KPI_GROUP_t));
      if (kpi_group[nKpiGroup-1] != IFX_NULL)
      {
         /* set structure defined to zero */
         memset (&(*kpi_group[nKpiGroup-1]), 0x00,
                 sizeof(IFX_TAPI_KPI_GROUP_t));
         /* create semaphores for protecting the fifos */
         TAPI_OS_MutexInit (&kpi_group[nKpiGroup-1]->semProtectEgressFifo);
         TAPI_OS_MutexInit (&kpi_group[nKpiGroup-1]->semProtectIngressFifo);
         TAPI_OS_SPIN_LOCK_INIT(&kpi_group[nKpiGroup-1]->sl_ProtectEgressFifo);
         TAPI_OS_SPIN_LOCK_INIT(&kpi_group[nKpiGroup-1]->sl_ProtectIngressFifo);
         /* create data fifos for ingress and egress direction */
         kpi_group[nKpiGroup-1]->pEgressFifo =
            fifoInit (IFX_TAPI_KPI_EGRESS_FIFO_SIZE,
                      sizeof(IFX_TAPI_KPI_FIFO_ELEM_t));
         kpi_group[nKpiGroup-1]->pIngressFifo =
            fifoInit (IFX_TAPI_KPI_INGRESS_FIFO_SIZE,
                      sizeof(IFX_TAPI_KPI_FIFO_ELEM_t));
      }
      else
      {
         /* error: no memory for group struct */
         return IFX_ERROR;
      }
   }
   return IFX_SUCCESS;
}


/**
   Handler for the ioctl IFX_TAPI_KPI_CH_CFG_SET.

   This function sets the internal data structures to associate a
   TAPI packet stream with a KPI channel.
   If the group has never been used before the group structure is created
   and the group specific resources are allocated.

   \param  pChannel     Handle to TAPI_CHANNEL structure.
   \param  *pCfg        Pointer to \ref IFX_TAPI_KPI_CH_CFG_t containing the
                        configuration.

   \return
   Return values are defined within the \ref IFX_return_t definition
   - IFX_SUCCESS  if configuration was successfully set
   - IFX_ERROR    on invalid values in the configuration

   \remarks
   For testing a loopback thread is started on group 1 as soon as it is
   configured for the first time.
*/
IFX_int32_t IFX_TAPI_KPI_ChCfgSet (TAPI_CHANNEL *pChannel,
                                   IFX_TAPI_KPI_CH_CFG_t const *pCfg)
{
   IFX_TAPI_KPI_CH_t         nKpiGroup,
                             nKpiChannel;
   TAPI_OS_INTSTAT           lock;

   /* Get the KPI-group number */
   nKpiGroup = KPI_GROUP_GET(pCfg->nKpiCh);
   /* reject group values which are out of the configured range */
   if (nKpiGroup > IFX_TAPI_KPI_MAX_GROUP)
      return IFX_ERROR;
   /* strip the group from the channel parameter */
   nKpiChannel = KPI_CHANNEL_GET(pCfg->nKpiCh);
   /* reject channel values which are out of the configured range */
   if (nKpiChannel >= IFX_TAPI_KPI_MAX_CHANNEL_PER_GROUP)
      return IFX_ERROR;
   /* reject source stream identifiers which are out of range */
   if (pCfg->nStream >= IFX_TAPI_KPI_STREAM_MAX)
      return IFX_ERROR;

   /* make sure the group is initialized already */
   if (ifx_tapi_KPI_GroupInit (nKpiGroup) != IFX_SUCCESS)
   {
      return IFX_ERROR;
   }

   /* Currently no flushing of fifos is done. To do this we had to lock global
      interrupts during this time. This is not desirable because it will take
      quite some time to release all the buffers possibly in the fifo.
      But because the data needed to deliver the packet is stored in the
      fifo when the packet is received we still get the packets delivered
      to the correct destination. */

   /* update the stream switch struct to indicate the new target */
   /* take protection semaphore */
   TAPI_OS_MutexGet (&pChannel->semTapiChDataLock);
   /* global irq lock */
   TAPI_OS_LOCKINT(lock);
   /* Set the stream switch struct to point to the new KPI channel.
      Group 0 is reserved for sending streams to the application. In this
      group force all channels to 0 to ease checks when using. */
   pChannel->pKpiStream[pCfg->nStream].nKpiCh =
      (nKpiGroup > 0) ? pCfg->nKpiCh : 0;
   pChannel->pKpiStream[pCfg->nStream].pEgressFifo =
      (nKpiGroup > 0) ? kpi_group[nKpiGroup-1]->pEgressFifo : IFX_NULL;
   /* global irq unlock */
   TAPI_OS_UNLOCKINT(lock);
   /* release protection semaphore */
   TAPI_OS_MutexRelease (&pChannel->semTapiChDataLock);

   if (nKpiGroup > 0)
   {
      /* set tapi channel reference in kpi channel reference array */
      /* take protection semaphore */
      TAPI_OS_MutexGet (&kpi_group[nKpiGroup-1]->semProtectIngressFifo);
      /* global irq lock */
      TAPI_OS_LOCKINT(lock);
      TAPI_OS_SPIN_LOCK_IRQSAVE(&kpi_group[nKpiGroup-1]->sl_ProtectIngressFifo);
      kpi_group[nKpiGroup-1]->channel_map[nKpiChannel] = pChannel;
      kpi_group[nKpiGroup-1]->stream_map[nKpiChannel] = pCfg->nStream;
      TAPI_OS_SPIN_UNLOCK_IRQRESTORE(&kpi_group[nKpiGroup-1]->sl_ProtectIngressFifo);
      /* global irq unlock */
      TAPI_OS_UNLOCKINT(lock);
      /* release protection semaphore */
      TAPI_OS_MutexRelease (&kpi_group[nKpiGroup-1]->semProtectIngressFifo);
   }

   return IFX_SUCCESS;
}


/**
   Handler for the ioctl IFX_TAPI_KPI_GRP_CFG_SET.

   This function configures the given KPI group. With this the size of the
   FIFOs can be set to individual values.
   If the group has never been used before the group structure is created
   and the group specific resources are allocated.

   NOTE: This function locks the interrupts for quite a long time and so may
   have a significant effect on system performance. It is recommended to use
   this function only then data transfer on the KPI group has stopped. For
   example during startup when nothing else is done in the sytem.

   \param  *pCfg        Pointer to \ref IFX_TAPI_KPI_GRP_CFG_t containing the
                        configuration.

   \return
   Return values are defined within the \ref IFX_return_t definition
   - IFX_SUCCESS  if configuration was successfully set
   - IFX_ERROR    on invalid values in the configuration
*/
IFX_int32_t IFX_TAPI_KPI_GrpCfgSet (IFX_TAPI_KPI_GRP_CFG_t const *pCfg)
{
   TAPI_OS_INTSTAT           lock;
   IFX_TAPI_KPI_FIFO_ELEM_t  sElem;
   IFX_TAPI_KPI_CH_t         nKpiGroup;
   IFX_uint8_t               egressSize,
                             ingressSize;

   /* Get the KPI-group number */
   nKpiGroup = KPI_GROUP_GET(pCfg->nKpiGroup);
   /* reject group values which are out of the configured range */
   if ((nKpiGroup == 0) || (nKpiGroup > IFX_TAPI_KPI_MAX_GROUP))
   {
      return IFX_ERROR;
   }

   /* when a size value of 0 is given the defaults will be used */
   egressSize  = (pCfg->nEgressFifoSize == 0) ?
                 IFX_TAPI_KPI_EGRESS_FIFO_SIZE : pCfg->nEgressFifoSize;
   ingressSize = (pCfg->nIngressFifoSize == 0) ?
                 IFX_TAPI_KPI_INGRESS_FIFO_SIZE : pCfg->nIngressFifoSize;

   /* make sure the group is already initialized */
   if (ifx_tapi_KPI_GroupInit (nKpiGroup) != IFX_SUCCESS)
   {
      return IFX_ERROR;
   }

   if (!TAPI_OS_IN_INTERRUPT())
   {
      TAPI_OS_MutexGet (&kpi_group[nKpiGroup-1]->semProtectIngressFifo);
      TAPI_OS_MutexGet (&kpi_group[nKpiGroup-1]->semProtectEgressFifo);
   }
   /* global irq lock */
   TAPI_OS_LOCKINT(lock);
   TAPI_OS_SPIN_LOCK_IRQSAVE(&kpi_group[nKpiGroup-1]->sl_ProtectIngressFifo);
   TAPI_OS_SPIN_LOCK_IRQSAVE(&kpi_group[nKpiGroup-1]->sl_ProtectEgressFifo);

   memset(&sElem, 0, sizeof(sElem));

   /* flush the data fifos for egress and ingress direction */
   while (fifoGet(kpi_group[nKpiGroup-1]->pEgressFifo, &sElem) == IFX_SUCCESS)
   {
      if (IFX_TAPI_VoiceBufferPut(sElem.pBuf) != IFX_SUCCESS)
      {
         /* This should never happen! Warn but do not stop here. */
         TRACE (TAPI_DRV, DBG_LEVEL_HIGH, ("\nBuffer put-back error(7)\n"));
      }
   }
   while (fifoGet(kpi_group[nKpiGroup-1]->pIngressFifo, &sElem) == IFX_SUCCESS)
   {
      if (IFX_TAPI_VoiceBufferPut(sElem.pBuf) != IFX_SUCCESS)
      {
         /* This should never happen! Warn but do not stop here. */
         TRACE (TAPI_DRV, DBG_LEVEL_HIGH, ("\nBuffer put-back error(8)\n"));
      }
   }

   /* delete data fifos for ingress and egress direction */
   fifoFree (kpi_group[nKpiGroup-1]->pEgressFifo);
   fifoFree (kpi_group[nKpiGroup-1]->pIngressFifo);

   /* recreate the fifos with the new size */
   kpi_group[nKpiGroup-1]->pEgressFifo =
      fifoInit(egressSize, sizeof(IFX_TAPI_KPI_FIFO_ELEM_t));
   kpi_group[nKpiGroup-1]->pIngressFifo =
      fifoInit(ingressSize, sizeof(IFX_TAPI_KPI_FIFO_ELEM_t));

   TAPI_OS_SPIN_UNLOCK_IRQRESTORE(&kpi_group[nKpiGroup-1]->sl_ProtectEgressFifo);
   TAPI_OS_SPIN_UNLOCK_IRQRESTORE(&kpi_group[nKpiGroup-1]->sl_ProtectIngressFifo);

   /* global irq unlock */
   TAPI_OS_UNLOCKINT(lock);

   if (!TAPI_OS_IN_INTERRUPT())
   {
      TAPI_OS_MutexRelease (&kpi_group[nKpiGroup-1]->semProtectEgressFifo);
      TAPI_OS_MutexRelease (&kpi_group[nKpiGroup-1]->semProtectIngressFifo);
   }

   return IFX_SUCCESS;
}


/**
   Retrieve the KPI Channel number of a given stream on a given TAPI Channel
   \param  pChannel     Handle to TAPI_CHANNEL structure.
   \param  stream       Stream Type id.

   \return KPI Channel number
*/
IFX_TAPI_KPI_CH_t IFX_TAPI_KPI_ChGet(TAPI_CHANNEL *pChannel,
                                     IFX_TAPI_KPI_STREAM_t stream)
{
   return pChannel->pKpiStream[stream].nKpiCh;
}


/** optionally: the KPI client might register a pointer to
                an egress tasklet (Linux) structure to its group
   \param nKpiGroup        KPI Group to register the tasklet to
   \param pEgressTasklet   void pointer to a (Linux) tasklet_struct

   \return void
*/
IFX_return_t IFX_TAPI_KPI_EgressTaskletRegister (IFX_TAPI_KPI_CH_t nKpiGroup,
                                                 IFX_void_t *pEgressTasklet )
{
#ifdef TAPI_FEAT_KPI_TASKLET
   nKpiGroup = KPI_GROUP_GET(nKpiGroup);

   if ((nKpiGroup == 0) || (nKpiGroup > IFX_TAPI_KPI_MAX_GROUP))
   {
      return IFX_ERROR;
   }

   /* make sure the group is already initialized */
   if (ifx_tapi_KPI_GroupInit (nKpiGroup) != IFX_SUCCESS)
   {
      return IFX_ERROR;
   }

   kpi_group[nKpiGroup-1]->pEgressTasklet = pEgressTasklet;
#else /* TAPI_FEAT_KPI_TASKLET */
   IFX_UNUSED(nKpiGroup);
   IFX_UNUSED(pEgressTasklet);
#endif /* TAPI_FEAT_KPI_TASKLET */
   return IFX_SUCCESS;
}
#endif /* TAPI_FEAT_KPI */
