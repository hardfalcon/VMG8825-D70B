#ifndef _DRV_TEST_MEI_CPE_LINUX_H
#define _DRV_TEST_MEI_CPE_LINUX_H
/******************************************************************************

                          Copyright (c) 2007-2015
                     Lantiq Beteiligungs-GmbH & Co. KG

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/* ==========================================================================
   Description : This file contains the includes and the defines
                 specific to the linux OS
   Remarks     : Please use the compiler switches here if you have
                 more than one OS.
   ========================================================================== */

#ifdef LINUX

/* ============================================================================
   Global Includes
   ========================================================================= */
#include "drv_mei_cpe_config.h"
#include "ifx_types.h"
#include "drv_mei_cpe_os.h"
#include "drv_mei_cpe_api_intern.h"


/* ============================================================================
   Global Definitons
   ========================================================================= */

#ifndef _MKSTR_1
#define _MKSTR_1(x)    #x
#define _MKSTR(x)      _MKSTR_1(x)
#endif

/** driver version, major number */
#define MEI_TEST_DRV_VER_MAJOR       0
/** driver version, minor number */
#define MEI_TEST_DRV_VER_MINOR         1
/** driver version, build number */
#define MEI_TEST_DRV_VER_STEP            0
/** driver version, package type */
#define MEI_TEST_DRV_VER_TYPE              1
/** driver version as string */
#define MEI_TEST_DRV_VER_STR        _MKSTR(MEI_TEST_DRV_VER_MAJOR) "." \
                                    _MKSTR(MEI_TEST_DRV_VER_MINOR) "." \
                                    _MKSTR(MEI_TEST_DRV_VER_STEP)  "." \
                                    _MKSTR(MEI_TEST_DRV_VER_TYPE)

/** driver version, what string */
#define MEI_TEST_DRV_WHAT_STR \
   "@(#)VRX driver test application, version " \
   MEI_TEST_DRV_VER_STR

/** used driver name for registration */
#define MEI_TEST_DRV_NAME     "mei_test"

/** max msg payload size */
#define MEI_TEST_MAX_MSG_PAYLOAD_SIZE     268

/**
   Vrx Line handles.
*/
typedef struct MEI_TEST_dev_s
{
   MEI_DYN_CNTRL_T    *pVrxTestLine[MEI_MAX_DFE_CHAN_DEVICES];
} MEI_TEST_dev_t;


/* ============================================================================
   global function (LINUX) - declarations
   ========================================================================= */

#endif   /* LINUX */
#endif   /* _DRV_TEST_MEI_CPE_LINUX_H */

