#ifndef _MEI_CPE_APPL_OSMAP_H
#define _MEI_CPE_APPL_OSMAP_H
/******************************************************************************

                          Copyright (c) 2007-2015
                     Lantiq Beteiligungs-GmbH & Co. KG

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/* ==========================================================================
   Description : IFX OS mapping fo the used VRX test application OS functions.
   ========================================================================== */
#ifdef __cplusplus
extern "C"
{
#endif


/* ==========================================================================
   includes
   ========================================================================== */
#ifdef VXWORKS
#include <vxworks.h>
#include <iolib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#endif

#ifdef LINUX
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <ctype.h>
#include <sys/ioctl.h>

extern int snprintf (char *__restrict __s, size_t __maxlen,
           __const char *__restrict __format, ...)
     __THROW __attribute__ ((__format__ (__printf__, 3, 4)));
#endif

/* ==========================================================================
   OS Mapping
   ========================================================================== */


#define MEIOS_MemSet                   memset
#define MEIOS_MemCpy                   memcpy
#define MEIOS_StrNCpy                  strncpy
#define MEIOS_StrLen                   strlen
#define MEIOS_StrError                 strerror
#define MEIOS_StrCaseCmp               strncasecmp
#define MEIOS_PError                   perror
#define MEIOS_StrToUl                  strtoul
#define MEIOS_Printf                   printf
#define MEIOS_SPrintf                  sprintf
#define MEIOS_SNPrintf                 snprintf


#include "ifxos_print.h"
#include "ifxos_print_io.h"
#include "ifxos_memory_alloc.h"
#include "ifxos_file_access.h"
#include "ifxos_device_access.h"

#define MEIOS_CRLF                     IFXOS_CRLF

#define MEIOS_FPrintf                  IFXOS_FPrintf
#define MEIOS_GetChar                  IFXOS_GetChar

#define MEIOS_MemAlloc                 IFXOS_MemAlloc
#define MEIOS_MemFree                  IFXOS_MemFree

#define MEIOS_File_t                   IFXOS_File_t
#define MEIOS_FileLoad                 IFXOS_FileLoad
#define MEIOS_FileWrite                IFXOS_FileWrite

#define MEIOS_devFd_set_t              IFXOS_devFd_set_t
#define MEIOS_DevFdSet                 IFXOS_DevFdSet
#define MEIOS_DevFdIsSet               IFXOS_DevFdIsSet

#define MEIOS_DeviceOpen               IFXOS_DeviceOpen
#define MEIOS_DeviceClose              IFXOS_DeviceClose
#define MEIOS_DeviceControl            IFXOS_DeviceControl
#define MEIOS_DeviceSelect             IFXOS_DeviceSelect

#ifdef __cplusplus
/* extern "C" */
}
#endif

#endif      /* #define _MEI_CPE_APPL_OSMAP_H */

