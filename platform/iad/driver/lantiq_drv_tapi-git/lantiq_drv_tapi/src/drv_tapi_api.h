#ifndef _DRV_TAPI_API_H
#define _DRV_TAPI_API_H
/******************************************************************************

                              Copyright (c) 2014
                            Lantiq Deutschland GmbH

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/**
   \file drv_tapi_api.h
   Internal functional API of the driver.
*/

/* ============================= */
/* includes                      */
/* ============================= */

/* ============================= */
/* Global defs                   */
/* ============================= */
#ifndef DRV_TAPI_NAME
   #ifdef LINUX
      /** device name */
      #define DRV_TAPI_NAME          "tapi"
   #else
      /** device name */
      #define DRV_TAPI_NAME          "/dev/tapi"
   #endif
#else
   #error module name already specified
#endif

#endif
