#ifndef _DRV_TAPI_VERSION_H
#define _DRV_TAPI_VERSION_H
/******************************************************************************

                            Copyright (c) 2001-2016
                        Lantiq Beteiligungs-GmbH & Co.KG
                             http://www.lantiq.com

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#include "drv_tapi_ll_if_version.h"

/** driver version, major number */
#define DRV_TAPI_VER_MAJOR 4
/** driver version, minor number */
#define DRV_TAPI_VER_MINOR 16
/** driver version, build number */
#define DRV_TAPI_VER_STEP 3
/** driver version, package type */
#define DRV_TAPI_VER_TYPE 0

#define IFX_TAPI_VERSION(a,b,c) (((a) << 16) + ((b) << 8) + (c))

#endif /* _DRV_TAPI_VERSION_H */
