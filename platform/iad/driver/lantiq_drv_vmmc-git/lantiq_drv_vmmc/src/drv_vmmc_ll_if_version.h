#ifndef _DRV_TAPI_LL_INTERFACE_VERSION_H
#define _DRV_TAPI_LL_INTERFACE_VERSION_H
/******************************************************************************

                            Copyright (c) 2006-2015
                        Lantiq Beteiligungs-GmbH & Co.KG
                             http://www.lantiq.com

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/** LL Interface version, major number */
#define DRV_TAPI_LL_IF_VER_MAJOR      1
/** LL Interface version, minor number */
#define DRV_TAPI_LL_IF_VER_MINOR      4
/** LL Interface version, build number */
#define DRV_TAPI_LL_IF_VER_STEP       10
/** LL Interface version, package type */
#define DRV_TAPI_LL_IF_VER_TYPE       1

/* some ll device drivers use the short define, so we define some alias */
#define LL_IF_MAJORSTEP             DRV_TAPI_LL_IF_VER_MAJOR
#define LL_IF_MINORSTEP             DRV_TAPI_LL_IF_VER_MINOR
#define LL_IF_VERSIONSTEP           DRV_TAPI_LL_IF_VER_STEP
#define LL_IF_VERS_TYPE             DRV_TAPI_LL_IF_VER_TYPE

#endif /* _DRV_TAPI_LL_INTERFACE_VERSION_H */
