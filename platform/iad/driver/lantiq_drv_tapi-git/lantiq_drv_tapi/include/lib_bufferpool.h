#ifndef LIB_BUFFERPOOL_H
#define LIB_BUFFERPOOL_H
/******************************************************************************

                              Copyright (c) 2014
                            Lantiq Deutschland GmbH

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

*******************************************************************************/

/**
   \file lib_bufferpool.h
   The buffer pool provides buffers with pre-allocated memory.
*/

/* ========================================================================== */
/* Includes                                                                   */
/* ========================================================================== */
#include <ifx_types.h>


/* ========================================================================== */
/* Global Defines                                                             */
/* ========================================================================== */
#define BUFFERPOOL_ERROR         (-1)
#define BUFFERPOOL_SUCCESS       0


/* ========================================================================== */
/* Configuration                                                              */
/* ========================================================================== */
#define  GET_CLEAN_BUFFERS       0
#define  DO_CHECKS_ON_PUT        1
#define  SHOW_MAGIC_ERROR        1
#define  SHOW_INUSE_ERROR        1
#define  SHOW_NOFREEBUF_ERROR    1
#define  SHOW_ERRORS             1
#define  SHOW_PATTERN_WARNING    1
#define  SHOW_WARNINGS           1


/* ========================================================================== */
/* Structure definition.                                                      */
/* ========================================================================== */
struct _BUFFERPOOL;
typedef struct _BUFFERPOOL BUFFERPOOL;


/* ========================================================================== */
/* Exported Functions                                                         */
/* ========================================================================== */
BUFFERPOOL*   bufferPoolInit (const IFX_uint32_t bufferSize,
                              const IFX_uint32_t initialElements,
                              const IFX_uint32_t extensionStep,
                              const IFX_uint32_t growthLimit);
IFX_int32_t   bufferPoolFree (BUFFERPOOL *pbp);

IFX_void_t*   bufferPoolGet  (IFX_void_t *pbp);
IFX_int32_t   bufferPoolPut        (IFX_void_t *pb);

IFX_void_t*   bufferPoolGetWithOwnerId(BUFFERPOOL *pbp, IFX_uint32_t ownerId);
IFX_int32_t   bufferPoolFreeAllOwnerId(BUFFERPOOL *pbp, IFX_uint32_t ownerId);
IFX_int32_t   bufferPoolEnumerate(BUFFERPOOL *pbp,
                                  IFX_uint32_t (*pCbHandler) (
                                      IFX_void_t *pArgs,
                                      IFX_void_t *pHandle,
                                      IFX_uint32_t ownerID,
                                      IFX_uint32_t state),
                                  IFX_void_t *pArgs);
IFX_int32_t   bufferPoolChOwn(IFX_void_t *pb, IFX_uint32_t ownerID);

IFX_int32_t   bufferPoolAvail (const BUFFERPOOL *pbp);
IFX_int32_t   bufferPoolElementSize (BUFFERPOOL *pbp);
IFX_int32_t   bufferPoolSize (const BUFFERPOOL *pbp);
IFX_void_t    bufferPoolIDSet (const BUFFERPOOL *pbp, const IFX_uint32_t id);

/* -- UNDER CONSTRUCTION / DEBUG functions -- */
IFX_int32_t   bufferPoolDump (const IFX_char_t *whatHappened,
                              const IFX_void_t *pb);
IFX_void_t    printBufferPoolErrors(void);
IFX_int32_t   bufferPoolDumpRTP (const char *whatHappened,
                                 const IFX_void_t *pb);

#endif /* LIB_BUFFERPOOL_H */
