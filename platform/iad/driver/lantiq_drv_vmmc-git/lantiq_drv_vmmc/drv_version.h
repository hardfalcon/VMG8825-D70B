#ifndef _DRV_VERSION_H
#define _DRV_VERSION_H

/******************************************************************************

                            Copyright (c) 2006-2016
                        Lantiq Beteiligungs-GmbH & Co.KG
                             http://www.lantiq.com

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#include "drv_vmmc_ll_if_version.h"

#define MAJORSTEP    1
#define MINORSTEP    21
#define VERSIONSTEP  2
#define VERS_TYPE    0

/* Minimum version of firmware required */
/* Platform XRX100 */
#define XRX100_MIN_FW_MAJORSTEP   5
#define XRX100_MIN_FW_MINORSTEP   0
#define XRX100_MIN_FW_HOTFIXSTEP  0
/* Platform XRX200 */
#define XRX200_MIN_FW_MAJORSTEP   3
#define XRX200_MIN_FW_MINORSTEP   5
#define XRX200_MIN_FW_HOTFIXSTEP  0
/* Platform XRX300 */
#define XRX300_MIN_FW_MAJORSTEP   2
#define XRX300_MIN_FW_MINORSTEP   1
#define XRX300_MIN_FW_HOTFIXSTEP  0
/* Platform FALCON */
#define FALCON_MIN_FW_MAJORSTEP   2
#define FALCON_MIN_FW_MINORSTEP   0
#define FALCON_MIN_FW_HOTFIXSTEP  0

#endif /* _DRV_VERSION_H */
