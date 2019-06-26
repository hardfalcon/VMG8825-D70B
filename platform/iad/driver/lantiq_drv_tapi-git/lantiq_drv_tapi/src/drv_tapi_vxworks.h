#ifndef _DRV_TAPI_VXWORKS_H
#define _DRV_TAPI_VXWORKS_H
/******************************************************************************

                              Copyright (c) 2014
                            Lantiq Deutschland GmbH

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/**
   This file contains the declarations of the vxworks specific driver functions.
*/

/* ============================= */
/* Global Includes               */
/* ============================= */

/* ============================= */
/* Macros & Definitions          */
/* ============================= */

#ifndef _IOC_NR
   #define _IOC_NR(cmd) (IOCBASECMD(cmd) & 0xff)
#endif /* _IOC_NR */

#ifndef _IOC_SIZE
   #define _IOC_SIZE(cmd) IOCPARM_LEN(cmd)
#endif /* _IOC_SIZE */

#ifndef _IOC_TYPE
   #define _IOC_TYPE(cmd) IOCGROUP(cmd)
#endif /* _IOC_TYPE */

#ifndef _IOC_DIR
   #define _IOC_DIR(cmd)   (cmd)
   #define _IOC_WRITE      IOC_IN
   #define _IOC_READ       IOC_OUT
#endif /* _IOC_TYPE */

/* ============================= */
/* Global Structures             */
/* ============================= */

/* ============================= */
/* Global function declaration   */
/* ============================= */

#endif /* _DRV_TAPI_VXWORKS_H */
