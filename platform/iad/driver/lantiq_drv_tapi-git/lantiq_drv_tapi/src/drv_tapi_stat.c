/******************************************************************************

                              Copyright (c) 2014
                            Lantiq Deutschland GmbH

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

*******************************************************************************/

/**
   \file drv_tapi_stat.c
   Implements statistics for the packet path.
*/

/* ============================= */
/* Includes                      */
/* ============================= */
#include "drv_tapi.h"

#ifdef TAPI_FEAT_STATISTICS

/* ============================= */
/* Local macros and definitions  */
/* ============================= */


/* ============================= */
/* Type declarations             */
/* ============================= */

/** Statistic Data for a bidirectional stream of packets. */
struct TAPI_STAT_DATA
{
   /** counters for every stream  */
   IFX_uint32_t  counter[IFX_TAPI_STREAM_MAX][TAPI_STAT_COUNTER_MAX];
};


/* ============================= */
/* Local function declarations   */
/* ============================= */

/* ============================= */
/* Local function definitions    */
/* ============================= */

/* ============================= */
/* Global function definitions   */
/* ============================= */

/**
   Initialise the statistics on the given channel.

   Initialise the data structures and resources needed for packet statistics.

   \param   pChannel    Pointer to TAPI_CHANNEL structure.

   \return
     - IFX_SUCCESS: if successful
     - IFX_ERROR: in case of an error

   \remarks This function is not protected. The global protection is done
   in the calling function with TAPI_OS_MutexGet (&pChannel->semTapiChDataLock)
*/
IFX_int32_t IFX_TAPI_Stat_Initialise_Unprot (TAPI_CHANNEL *pChannel)
{
   TAPI_STAT_DATA_t  *pTapiStatData  = pChannel->pTapiStatData;

   /* allocate data storage for the statistics on the channel
      if not already existing */
   if (pTapiStatData == IFX_NULL)
   {
      /* allocate data storage */
      if ((pTapiStatData = TAPI_OS_Malloc (sizeof(*pTapiStatData))) == IFX_NULL)
      {
         return IFX_ERROR;
      }
      /* Store pointer to data in the channel or we lose it on exit. */
      pChannel->pTapiStatData = pTapiStatData;
      memset (pTapiStatData, 0x00, sizeof(*pTapiStatData));
   }
   /* from here on pTapiStatData and pChannel->pTapiDialData are valid */

   return IFX_SUCCESS;
}


/**
   Cleanup the statistics on the given channel.

   Free the data structures and resources needed for packet statistics.

   \param   pChannel    Pointer to TAPI_CHANNEL structure.
*/
IFX_void_t IFX_TAPI_Stat_Cleanup(TAPI_CHANNEL *pChannel)
{
   TAPI_STAT_DATA_t  *pTapiStatData  = pChannel->pTapiStatData;

   TAPI_OS_MutexGet (&pChannel->semTapiChDataLock);

   if (pTapiStatData != IFX_NULL)
   {
      /* free the data storage on the channel */
      TAPI_OS_Free (pTapiStatData);
      pChannel->pTapiStatData = IFX_NULL;
   }

   TAPI_OS_MutexRelease (&pChannel->semTapiChDataLock);
}


/**
   Reset all statistic counters of the specified channel to zero.

   Resets the statistic counters of all streams inside a channel.

   \param  pChannel     Pointer to TAPI_CHANNEL structure.
*/
IFX_void_t IFX_TAPI_Stat_Reset(TAPI_CHANNEL *pChannel)
{
   TAPI_STAT_DATA_t  *pTapiStatData  = pChannel->pTapiStatData;

   TAPI_OS_MutexGet (&pChannel->semTapiChDataLock);

   if (pTapiStatData != IFX_NULL)
   {
      memset (pTapiStatData, 0x00, sizeof(*pTapiStatData));
   }

   TAPI_OS_MutexRelease (&pChannel->semTapiChDataLock);
}


/**
   Get a specified statistic counter value.

   \param  pChannel     Pointer to TAPI_CHANNEL structure.
   \param  stream       Selects the stream from \ref IFX_TAPI_STREAM_t.
   \param  counter      Selects the counter from \ref TAPI_STAT_COUNTER_t.

   \return
   Value of selected statistic counter or 0 if not initialised.
*/
IFX_uint32_t IFX_TAPI_Stat_Get(TAPI_CHANNEL *pChannel,
                             IFX_TAPI_STREAM_t stream,
                             TAPI_STAT_COUNTER_t counter)
{
   TAPI_STAT_DATA_t  *pTapiStatData  = pChannel->pTapiStatData;

   if (pTapiStatData != IFX_NULL)
   {
      return pTapiStatData->counter[stream][counter];
   }

   return 0L;
}

#endif /* TAPI_FEAT_STATISTICS */


/**
   Add a value to a specific statistic counter.

   Statistic counters are associated to streams inside a channel. With this
   for example DECT and COD can be distinguished on the same channel.
   The given value will be added to the selected counter on the given channel
   and stream.

   \param  pChannel     Pointer to TAPI_CHANNEL structure.
   \param  stream       Selects the stream from \ref IFX_TAPI_STREAM_t.
   \param  counter      Selects the counter from \ref TAPI_STAT_COUNTER_t.
   \param  value        Value to be added to the selected counter.
*/
IFX_void_t IFX_TAPI_Stat_Add(TAPI_CHANNEL *pChannel,
                             IFX_TAPI_STREAM_t stream,
                             TAPI_STAT_COUNTER_t counter,
                             IFX_uint32_t value)
{
#ifdef TAPI_FEAT_STATISTICS
   TAPI_STAT_DATA_t  *pTapiStatData  = pChannel->pTapiStatData;

   if (pTapiStatData != IFX_NULL)
   {
      pTapiStatData->counter[stream][counter] += value;

      switch (counter)
      {
      case TAPI_STAT_COUNTER_EGRESS_CONGESTED:
         pTapiStatData->counter[stream][TAPI_STAT_COUNTER_EGRESS_DISCARDED] += value;
         break;
      case TAPI_STAT_COUNTER_INGRESS_CONGESTED:
         pTapiStatData->counter[stream][TAPI_STAT_COUNTER_INGRESS_DISCARDED] += value;
         break;
      default:
         /* do nothing */
         break;
      }
   }
#else
   IFX_UNUSED (pChannel);
   IFX_UNUSED (stream);
   IFX_UNUSED (counter);
   IFX_UNUSED (value);
#endif /* TAPI_FEAT_STATISTICS */
}
