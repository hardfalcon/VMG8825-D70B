/******************************************************************************

                              Copyright (c) 2014
                            Lantiq Deutschland GmbH

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#ifdef XAPI

/** \file
   This file contains the IFXOS Layer implementation for XAPI
   Event Handling.
*/

/* ============================================================================
   IFX XAPI adaptation - Global Includes
   ========================================================================= */

#include <xapi/xapi.h>

#include "ifx_types.h"
#include "ifxos_time.h"
#include "ifxos_event.h"

/* ============================================================================
   IFX XAPI adaptation - EVENT handling
   ========================================================================= */
/** \addtogroup IFXOS_EVENT_XAPI
@{ */


#if ( defined(IFXOS_HAVE_EVENT) && (IFXOS_HAVE_EVENT == 1) )

/**
   XAPI - Initialize a Event Object for synchronisation.

\par Implementation
   - a semaphore is created
   - init state is "EMPTY" - locked.
   - the queuing is FIFO based

\param
   pEventId    Prointer to the Event Object.

\return
   IFX_SUCCESS if the creation was successful, else
   IFX_ERROR in case of error.
*/
IFX_int_t IFXOS_EventInit(
               IFXOS_event_t  *pEventId)
{
   if(pEventId)
   {
      if (IFXOS_EVENT_INIT_VALID(pEventId) == IFX_FALSE)
      {
         xsm_create ("IFE", 0, 0, &(pEventId->object));
         pEventId->bValid = IFX_TRUE;

         return IFX_SUCCESS;
      }
   }

   return IFX_ERROR;
}

/**
   XAPI - Delete the given Event Object.

\par Implementation
   - delete the semaphore object (see "semDelete").

\param
   pEventId    Prointer to the Event Object.

\return
   IFX_SUCCESS if delete was successful, else
   IFX_ERROR if something was wrong
*/
IFX_int_t IFXOS_EventDelete(
               IFXOS_event_t  *pEventId)
{
   if(pEventId)
   {
      if (IFXOS_EVENT_INIT_VALID(pEventId) == IFX_TRUE)
      {
         xsm_delete(pEventId->object);
         pEventId->bValid = IFX_FALSE;

         return IFX_SUCCESS;
      }
   }

   return IFX_ERROR;
}

/**
   XAPI - Wakeup a Event Object to signal the occurance of the "event" to
   the waiting processes.

\par Implementation
   - Release the semaphore to signal the event.

\param
   pEventId    Prointer to the Event Object.

\return
   IFX_SUCCESS on success.
   IFX_ERROR   on error
*/
IFX_int_t IFXOS_EventWakeUp(
               IFXOS_event_t  *pEventId)
{
   if (pEventId)
   {
      if (IFXOS_EVENT_INIT_VALID(pEventId) == IFX_TRUE)
      {
         if (!xsm_v(pEventId->object))
         {
            return IFX_SUCCESS;
         }
      }
   }

   return IFX_ERROR;
}

/**
   XAPI - Wait for the occurance of an "event" with timeout.

\par Implementation
   - Take the semaphore with timeout [ms] for wait for the event.
   - timeout signaling currently not supported.

\param
   pEventId       Prointer to the Event Object.
\param
   waitTime_ms    Max time to wait [ms].
\param
   pRetCode    Points to the return code variable. [O]
               - If the pointer is NULL the return code will be ignored, else
                 the corresponding return code will be set
               - For timeout the return code is set to 1.

\return
   IFX_SUCCESS on success.
   IFX_ERROR   on error or timeout.
*/
IFX_int_t IFXOS_EventWait(IFXOS_event_t *pEventId,
               IFX_uint32_t waitTime_ms,
               IFX_int32_t *pRetCode)
{
   if (pEventId)
   {
      if (IFXOS_EVENT_INIT_VALID(pEventId) == IFX_TRUE)
      {
         if (xsm_p(pEventId->object, 0, waitTime_ms) != 0)
         {
            if (pRetCode)
               *pRetCode = 1;
         }
         else
         {
            if (pRetCode)
               *pRetCode = 0;

            return IFX_SUCCESS;
         }
      }
   }
   return IFX_ERROR;
}

#endif      /* #if ( defined(IFXOS_HAVE_EVENT) && (IFXOS_HAVE_EVENT == 1) ) */

/** @} */

#endif      /* #ifdef XAPI */

