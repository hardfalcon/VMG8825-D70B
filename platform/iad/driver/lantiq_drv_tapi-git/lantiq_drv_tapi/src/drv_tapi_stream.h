#ifndef DRV_TAPI_STREAM_H
#define DRV_TAPI_STREAM_H
/******************************************************************************

                            Copyright (c) 2014,2016
                        Lantiq Beteiligungs-GmbH & Co.KG
                             http://www.lantiq.com

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/**
   \file drv_tapi_stream.h
   Contains TAPI functions declaration for fifo, bufferpool.
*/

/* ============================= */
/* Includes                      */
/* ============================= */

/* ============================= */
/* Global type declarations      */
/* ============================= */
/* struct that reports counters for the selected upstream fifo */
typedef struct
{
   IFX_uint32_t      nWaiting;
   /* Possible extensions are counters on the size, the full state or
      the number of transactions. */
} IFX_TAPI_UPSTREAM_FIFO_COUNTERS_t;

/* ============================= */
/* Global variable definition    */
/* ============================= */

/* ============================= */
/* Global function declaration   */
/* ============================= */


/* Used only on TAPI side */
extern IFX_int32_t IFX_TAPI_UpStreamFifo_Create(TAPI_CHANNEL* pTapiCh);
extern IFX_int32_t IFX_TAPI_UpStreamFifo_Delete(TAPI_CHANNEL* pTapiCh);

extern IFX_void_t * IFX_TAPI_UpStreamFifo_Get(
                        TAPI_CHANNEL* pTapiCh,
                        IFX_TAPI_STREAM_t nStream,
                        IFX_uint32_t *pLength,
                        IFX_uint32_t *pOffset);

extern IFX_boolean_t IFX_TAPI_UpStreamFifo_IsEmpty(
                        TAPI_CHANNEL* pChannel,
                        IFX_TAPI_STREAM_t nStream);

extern IFX_int32_t IFX_TAPI_UpStreamFifo_Counters(
                        TAPI_CHANNEL* pChannel,
                        IFX_TAPI_STREAM_t nStream,
                        IFX_TAPI_UPSTREAM_FIFO_COUNTERS_t *pCounters);

#ifdef TAPI_FEAT_POLL
extern IFX_int32_t  IFX_TAPI_DownStreamFifo_Create(TAPI_DEV* pTapiDev);
extern IFX_int32_t  IFX_TAPI_DownStreamFifo_Delete(TAPI_DEV* pTapiDev);
#endif /* TAPI_FEAT_POLL */

/* global buffer pool for voice packets of all devices of all drivers */
extern IFX_int32_t  IFX_TAPI_VoiceBufferPool_Create(void);
extern IFX_int32_t  IFX_TAPI_VoiceBufferPool_Delete(void);
#ifdef TAPI_PACKET_OWNID
extern IFX_void_t   IFX_TAPI_VoiceBufferPoolStatusShow(void);
#endif /* TAPI_PACKET_OWNID */
extern IFX_uint32_t IFX_TAPI_VoiceBufferPool_ElementSizeGet(void);
extern IFX_int32_t  IFX_TAPI_VoiceBufferPool_ElementCountGet(void);
extern IFX_int32_t  IFX_TAPI_VoiceBufferPool_ElementAvailCountGet(void);

#endif /* DRV_TAPI_STREAM_H */
