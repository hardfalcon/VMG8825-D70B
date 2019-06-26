#ifndef DRV_TAPI_WIN_H
#define DRV_TAPI_WIN_H
/******************************************************************************

                              Copyright (c) 2014
                            Lantiq Deutschland GmbH

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/**
   \file drv_tapi_win.h
   This file contains the declarations of the windows specific driver functions.
*/

/* ============================= */
/* Includes                      */
/* ============================= */

/* ============================= */
/* Macros & Definitions          */
/* ============================= */

#ifndef _IOC_NR
   #define _IOC_NR(cmd) ((cmd) & 0xff)
#endif /* _IOC_NR */

#ifndef _IOC_TYPE
   #define _IOC_TYPE(cmd) (((cmd) >> 8) & 0xFF)
#endif /* _IOC_TYPE */

#ifndef _IOC_SIZE
   #define _IOC_SIZE(cmd) (((cmd) >> 16) & 0x1FF)
#endif /* _IOC_SIZE */

#ifndef _IOC_DIR
   #define _IOC_DIR(cmd)   (((cmd) >> 29) & 0x7)
#endif

#ifndef _IOC_WRITE
#define _IOC_WRITE      IOC_IN
#endif

#ifndef _IOC_READ
   #define _IOC_READ       IOC_OUT
#endif

#ifdef TAPI_HAVE_TIMERS
   #define TIMER_ELEMENT   int

   typedef IFX_void_t (*linux_timer_callback)(IFX_uint32_t);
#endif /* TAPI_HAVE_TIMERS */

/* ============================= */
/* Global Function Declaration   */
/* ============================= */

#endif  /* DRV_TAPI_WIN_H */
