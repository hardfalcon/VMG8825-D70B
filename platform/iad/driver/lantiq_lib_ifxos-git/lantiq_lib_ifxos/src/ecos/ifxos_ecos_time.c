/******************************************************************************

                              Copyright (c) 2009
                            Lantiq Deutschland GmbH
                     Am Campeon 3; 85579 Neubiberg, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#ifdef ECOS

/** \file
   This file contains the IFXOS Layer implementation for eCos 
   Time and Wait.
*/

/* ============================================================================
   IFX eCos adaptation - Global Includes
   ========================================================================= */
#include <pkgconf/system.h>
#include <cyg/kernel/kapi.h>  /* All the kernel specific stuff like cyg_flag_t, ... */

#include "ifx_types.h"
#include "ifxos_common.h"     /* tick */
#include "ifxos_time.h"

#define DELAYLIB_DEBUG 0

#if (DELAYLIB_DEBUG == 1)
unsigned int delayLoop = 0;
#endif /* DELAYLIB_DEBUG */

/* ============================================================================
   IFX eCos adaptation - time handling
   ========================================================================= */

/** \addtogroup IFXOS_TIME_ECOS
@{ */

#if (defined(IFXOS_HAVE_TIME_ELAPSED_TIME_GET_MS) && (IFXOS_HAVE_TIME_ELAPSED_TIME_GET_MS == 1))
#  define IFXOS_TIME_MAX      (~((IFX_time_t)0))
#  define IFXOS_RESOLUTION_MS (1000UL / IFXOS_TICKS_PER_SECOND)
#  define IFXOS_TICKS_SCALE   (IFXOS_TIME_MAX / IFXOS_RESOLUTION_MS)
#  define IFXOS_MSEC_SCALE    (IFXOS_TICKS_SCALE * IFXOS_RESOLUTION_MS) 
#endif

#if (defined(IFXOS_HAVE_TIME_SLEEP_US) && (IFXOS_HAVE_TIME_SLEEP_US == 1))

/**
   eCos - Sleep a given time in [us].

\par Implementation
   In case that the system is not calibrated, a calibration will be done.
   The "sleeping" will be performed by looping for a certain amount of loops. Therefore
   no scheduler is involved.

\attention
   The implementation is designed as "busy wait", the scheduler will not be called.

\param
   sleepTime_us   Time to sleep [us]

\return
   None.

\remarks
   Available in Driver and Application Space
*/
IFX_void_t IFXOS_USecSleep(
               IFX_time_t sleepTime_us)
{
    #if (DELAYLIB_DEBUG == 0)
    static unsigned int delayLoop = 0;
    #endif /* (DELAYLIB_DEBUG == 0) */
    unsigned int ix;
    unsigned int iy;

    if (delayLoop == 0 || sleepTime_us == (unsigned int)0xffffffff)      /* need calibration?          */
    {
        unsigned int maxLoop;
        unsigned int start = 0;
        unsigned int stop = 0;
        unsigned int mpt = (1000 * 1000) / IFXOS_TICKS_PER_SECOND; /* microsecs per tick     */

        for (delayLoop = 1; delayLoop < 0x1000 && stop == start; delayLoop<<=1)
        {
            /* wait for clock turn over */
            for (stop = start = cyg_current_time (); start == stop; start = cyg_current_time ());

            IFXOS_USecSleep (mpt);            /* single recursion                   */
            stop = cyg_current_time ();
        }

        maxLoop = delayLoop / 2;        /* loop above overshoots              */
        #if (DELAYLIB_DEBUG == 1)
        printf ("maxLoop = %d\n", maxLoop);
        #endif /* DELAYLIB_DEBUG */
        start = 0;
        stop = 0;
        if (delayLoop < 4)
            delayLoop = 4;
        for (delayLoop /= 4; delayLoop<maxLoop && stop == start; delayLoop++)
        {
            /* wait for clock turn over */
            for (stop = start = cyg_current_time (); start == stop; start = cyg_current_time ());

            IFXOS_USecSleep (mpt);            /* single recursion                   */
            stop = cyg_current_time ();
        }
        #if (DELAYLIB_DEBUG == 1)
        printf ("delayLoop = %d\n", delayLoop);
        #endif /* DELAYLIB_DEBUG */
    }

    for (iy = 0; iy < sleepTime_us; iy++)
    {
        for (ix = 0; ix < delayLoop; ix++);
    }
 }
#endif

#if (defined(IFXOS_HAVE_TIME_SLEEP_MS) && (IFXOS_HAVE_TIME_SLEEP_MS == 1))

/**
   eCos - Sleep a given time in [ms].

\par Implementation
   Use the eCos scheduler to set the caller task into "sleep".

\attention
   The sleep requires a "sleep wait". "busy wait" implementation will not work.

\param
   sleepTime_ms   Time to sleep [ms]

\return
   None.

\remarks
   sleepTime_ms = 0 force a rescheduling.

\remarks
   Available in Driver and Application Space
*/
IFX_void_t IFXOS_MSecSleep(
               IFX_time_t sleepTime_ms)
{
   cyg_thread_delay( (int)((sleepTime_ms==0) ? 

                        0 : ( ((IFXOS_MSEC_TO_TICK(sleepTime_ms)) <= 1) ? 

                                    1 : (IFXOS_MSEC_TO_TICK(sleepTime_ms)) ) ) );
}
#endif


#if (defined(IFXOS_HAVE_TIME_SLEEP_SEC) && (IFXOS_HAVE_TIME_SLEEP_SEC == 1))
/**
   eCos - Sleep a given time in [seconds].

\par Implementation
   Use the eCos scheduler to set the caller task into "sleep".

\param
   sleepTime_sec  Time to sleep [sec]

\return
   None.

\remarks
   Available in Application Space
*/
IFX_void_t IFXOS_SecSleep(
               IFX_time_t sleepTime_sec)
{
   cyg_thread_delay (IFXOS_MSEC_TO_TICK(sleepTime_sec*1000));
}
#endif



#if (defined(IFXOS_HAVE_TIME_ELAPSED_TIME_GET_MS) && (IFXOS_HAVE_TIME_ELAPSED_TIME_GET_MS == 1))
/**
   eCos - Get the elapsed time in [ms].

\par Implementation
   Based on the "cyg_current_time" function and  "IFXOS_TICK_TO_MSEC" macro we calculate the 
   elapsed time since startup or based on the given ref-time.

\param
   refTime_ms  Reference time to calculate the elapsed time in [ms].

\return 
   Elapsed time in [ms] based on the given reference time

\remark
   Provide refTime_ms = 0 to get the current elapsed time. For messurement provide
   the current time as reference.
*/
IFX_time_t IFXOS_ElapsedTimeMSecGet(
               IFX_time_t refTime_ms)
{
   IFX_time_t currTime_ms, ticks_mod;

   if (refTime_ms > IFXOS_MSEC_SCALE)
      return 0;

   ticks_mod = (IFXOS_TICKS_SCALE + 1) ?
                        (IFX_time_t)(cyg_current_time() % (IFXOS_TICKS_SCALE + 1)) :
                        (IFX_time_t)(cyg_current_time());
   currTime_ms = ticks_mod * IFXOS_RESOLUTION_MS;

   return (currTime_ms >= refTime_ms) ? (currTime_ms - refTime_ms) :
	    (IFXOS_MSEC_SCALE - refTime_ms + currTime_ms + IFXOS_RESOLUTION_MS);
}
#endif

#if (defined(IFXOS_HAVE_TIME_ELAPSED_TIME_GET_SEC) && (IFXOS_HAVE_TIME_ELAPSED_TIME_GET_SEC == 1))
/**
   eCos - Get the elapsed time since startup in [seconds]

\par Implementation
   Based on the "cyg_current_time" function and  "IFXOS_TICK_TO_MSEC" macro we calculate the 
   elapsed time since startup or based on the given ref-time.

\param
   refTime_sec Reference time to calculate the elapsed time in [sec].

\return 
   Elapsed time in [sec] based on the given reference time

\remark
   Provide refTime_sec = 0 to get the elapsed time since startup.

\remarks
   Available in Application Space.
*/
IFX_time_t IFXOS_ElapsedTimeSecGet(
               IFX_time_t refTime_sec)
{
   IFX_time_t currTime_sec = 0;

   currTime_sec = (IFX_time_t)IFXOS_TICK_TO_MSEC(cyg_current_time())/1000;

   if ( (refTime_sec == 0) || (refTime_sec > currTime_sec) )
   {
      return currTime_sec;
   }

   return (currTime_sec - refTime_sec);
}
#endif


#if (defined(IFXOS_HAVE_TIME_SYS_TIME_GET) && (IFXOS_HAVE_TIME_SYS_TIME_GET == 1))
/**
   eCos - Reads the actual system time.

\return
   Returns the actual system time in seconds.
*/
IFX_time_t IFXOS_SysTimeGet(void)
{
   return (IFX_uint32_t)IFXOS_TICK_TO_MSEC(cyg_current_time())/1000;
}
#endif


/** @} */

#endif      /* #ifdef ECOS */

