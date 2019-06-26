#ifndef _IFXOS_XAPI_TIME_H
#define _IFXOS_XAPI_TIME_H
/******************************************************************************

                              Copyright (c) 2014
                            Lantiq Deutschland GmbH

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#ifdef XAPI
/** \file
   This file contains XAPI definitions for timer and wait handling.
*/

/** \defgroup IFXOS_TIME_XAPI Time and Wait (XAPI).

   This Group contains the XAPI time and wait definitions.

\ingroup IFXOS_LAYER_XAPI
*/

#ifdef __cplusplus
   extern "C" {
#endif

/* ============================================================================
   IFX XAPI adaptation - Includes
   ========================================================================= */
#include "ifx_types.h"

/* ============================================================================
   IFX XAPI adaptation - supported features
   ========================================================================= */
/** IFX XAPI adaptation - support "sleep in us" */

#ifndef IFXOS_HAVE_TIME_SLEEP_US
#  define IFXOS_HAVE_TIME_SLEEP_US                1
#endif

/** IFX XAPI adaptation - support "sleep in ms" */
#ifndef IFXOS_HAVE_TIME_SLEEP_MS
#  define IFXOS_HAVE_TIME_SLEEP_MS                1
#endif

/** IFX XAPI adaptation - support "sleep in sec" */
#ifndef IFXOS_HAVE_TIME_SLEEP_SEC
#  define IFXOS_HAVE_TIME_SLEEP_SEC               1
#endif

/** IFX XAPI adaptation - support "get elapesed time in ms" */
#ifndef IFXOS_HAVE_TIME_ELAPSED_TIME_GET_MS
#  define IFXOS_HAVE_TIME_ELAPSED_TIME_GET_MS     1
#endif

/** IFX XAPI adaptation - support "get elapesed time in sec" */
#ifndef IFXOS_HAVE_TIME_ELAPSED_TIME_GET_SEC
#  define IFXOS_HAVE_TIME_ELAPSED_TIME_GET_SEC    0
#endif

/** IFX XAPI adaptation - support "get system time in sec" */
#ifndef IFXOS_HAVE_TIME_SYS_TIME_GET
#  define IFXOS_HAVE_TIME_SYS_TIME_GET            1
#endif

#ifdef __cplusplus
}
#endif
#endif      /* #ifdef XAPI */
#endif      /* #ifndef _IFXOS_XAPI_TIME_H */

