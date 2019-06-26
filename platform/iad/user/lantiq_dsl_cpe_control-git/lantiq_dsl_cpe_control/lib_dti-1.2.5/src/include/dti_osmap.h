#ifndef _DTI_OSMAP_H
#define _DTI_OSMAP_H
/****************************************************************************

                              Copyright (c) 2009
                            Lantiq Deutschland GmbH
                     Am Campeon 3; 85579 Neubiberg, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

*****************************************************************************/

/* ==========================================================================
   Description : IFX OS mapping of the used DTI OS functions.
   ========================================================================== */

/** \file
   Mapping of the OS functions used by the DTI.
*/

#ifdef __cplusplus
extern "C"
{
#endif

#include "ifxos_common.h"

#if defined(IFXOS_HAVE_VERSION_EG_THAN_1_1_2)
#  undef IFXOS_HAVE_VERSION_EG_THAN_1_1_2
#endif
#define IFXOS_HAVE_VERSION_EG_THAN_1_1_2        0

#if defined(IFXOS_HAVE_VERSION_CHECK)
#  if (IFXOS_VERSION_CHECK_EG_THAN(1,1,2))
#     undef  IFXOS_HAVE_VERSION_EG_THAN_1_1_2
#     define IFXOS_HAVE_VERSION_EG_THAN_1_1_2   1
#  endif
#endif

#if (IFXOS_HAVE_VERSION_EG_THAN_1_1_2 == 1)
#   include "ifxos_std_defs.h"
#else
#   if defined(LINUX)
#     include <stdio.h>
#     include <stdlib.h>
#     include <string.h>
#     include <ctype.h>
#     include <errno.h>
#   elif defined(VXWORKS)
#     include <vxworks.h>
#     include <stdio.h>
#     include <stdlib.h>
#     include <ctype.h>
#   elif defined(ECOS)
#     include <stdio.h>
#     include <stdlib.h>
#     include <ctype.h>
#   elif defined(WIN32)
#     include <stdio.h>
#     include <stdlib.h>
#     include <ctype.h>
#   elif defined(GENERIC_OS)
#      ifdef _lint
#         error "DTI - for LINT compile you require IFXOS Version > 1.1.2"
#      else
#         error "DTI - define the CUSTOMER OS includes"
#      endif
#   else
#      error "DTI - no OS specified"
#   endif
#endif      /* IFXOS_HAVE_VERSION_EG_THAN_1_1_2 */


/* ==========================================================================
   IFXOS Includes
   ========================================================================== */

#include "ifx_types.h"
#include "ifxos_common.h"
#include "ifxos_debug.h"

#include "ifxos_print.h"
#include "ifxos_print_io.h"
#include "ifxos_memory_alloc.h"
#include "ifxos_device_access.h"
#include "ifxos_file_access.h"

#include "ifxos_time.h"
#include "ifxos_lock.h"
#include "ifxos_thread.h"


#include "ifx_fifo.h"

/* ==========================================================================
   Defines
   ========================================================================== */

/**
   Pointer Struct for alligned buffer allocation and access.
*/
typedef union
{
   IFX_uint8_t    *pUInt8;
   IFX_uint16_t   *pUInt16;
   IFX_uint32_t   *pUInt32;
   IFX_ulong_t    *pULong;
} DTI_PTR_U;


#define DTI_PTR_CAST_GET_UINT8(buffer_ptr)  (buffer_ptr).pUInt8

#define DTI_PTR_CAST_GET_UINT16(buffer_ptr) \
                  ((((IFX_ulong_t)(buffer_ptr).pUInt8 & 0x01) != 0) ? IFX_NULL : (buffer_ptr).pUInt16)

#define DTI_PTR_CAST_GET_UINT32(buffer_ptr) \
                  ((((IFX_ulong_t)(buffer_ptr).pUInt8 & 0x03) != 0) ? IFX_NULL : (buffer_ptr).pUInt32)

#define DTI_PTR_CAST_GET_ULONG(buffer_ptr) \
                  /*lint -e{506} -e{826} */ \
                  ( (sizeof(IFX_ulong_t) == 4) ? \
                     ((((IFX_ulong_t)(buffer_ptr).pUInt8 & 0x03) != 0) ? IFX_NULL : (buffer_ptr).pULong) : \
                     ((((IFX_ulong_t)(buffer_ptr).pUInt8 & 0x07) != 0) ? IFX_NULL : (buffer_ptr).pULong) )


/* ==========================================================================
   Map the standard types and functions to the project naming conventions
   ========================================================================== */
#ifndef DTI_DEBUG_PRINT
#	define DTI_DEBUG_PRINT	1
#endif

#ifndef DTI_ERROR_PRINT
#	define DTI_ERROR_PRINT	1
#endif
#if (   !defined(IFXOS_HAVE_PRINT) \
	|| (defined(IFXOS_HAVE_PRINT) && (IFXOS_HAVE_PRINT == 0)))
#	ifdef DTI_DEBUG_PRINT
#		undef DTI_DEBUG_PRINT
#	endif
#	ifdef DTI_ERROR_PRINT
#		undef DTI_ERROR_PRINT
#	endif
#	define DTI_DEBUG_PRINT  0
#	define DTI_ERROR_PRINT  0
#endif      /* #if ( !defined(IFXOS_HAVE_PRINT) || ... ) */

#define DTI_Printf                  (void)printf
#define DTI_GetChar                 IFXOS_GetChar

#define DTI_StrLen                  strlen
#define DTI_StrCpy                  (void)strcpy
#define DTI_StrNCpy                 strncpy
#define DTI_StrNCmp                 strncmp

#define DTI_MemSet                  (void)memset
#define DTI_MemCpy                  (void)memcpy
#define DTI_StrToUl                 strtoul

#define DTI_htons                   htons
#define DTI_htonl                   htonl
#define DTI_ntohs                   ntohs
#define DTI_ntohl                   ntohl

#define DTI_IsSpace                 isspace
#define DTI_IsDigit                 isdigit

/* disable lint checks until those function prototypes are available */
#define DTI_IsAlpha                 /*lint -e{718} -e{746} */ isalpha
#define DTI_IsAlNum                 /*lint -e{718} -e{746} */ isalnum

/* ==========================================================================
   Map the IFXOS types and functions to the project naming conventions
   ========================================================================== */

/* elapsed time [ms] */
#define DTI_ElapsedTimeMSecGet      IFXOS_ElapsedTimeMSecGet
#define DTI_SecSleep                IFXOS_SecSleep
#define DTI_MSecSleep               IFXOS_MSecSleep


/* syncronization */
#define DTI_LockInit                (void)IFXOS_LockInit
#define DTI_LockGet                 IFXOS_LockGet
#define DTI_LockRelease             (void)IFXOS_LockRelease
#define DTI_LockDelete              (void)IFXOS_LockDelete

/* threads */
#define DTI_ThreadInit              IFXOS_ThreadInit
#define DTI_ThreadDelete            (void)IFXOS_ThreadDelete
#define DTI_ThreadShutdown          IFXOS_ThreadShutdown

/* string functions */
#define DTI_snprintf                IFXOS_SNPrintf
#define DTI_CRLF                    IFXOS_CRLF

/* memory functions */
#define DTI_Malloc                  IFXOS_MemAlloc
#define DTI_Free                    IFXOS_MemFree

/* debug stuff */
#if (DTI_DEBUG_PRINT == 1)
#define DTI_PRN_USR_DBG_NL          IFXOS_PRN_USR_DBG_NL
#else
#define DTI_PRN_USR_DBG_NL(module_name, dbg_level, print_message)  /*lint -e{19} */
#endif  /* #if (DLI_DEBUG_PRINT == 1) */
#if (DTI_ERROR_PRINT == 1)
#define DTI_PRN_USR_ERR_NL          IFXOS_PRN_USR_ERR_NL
#else
#define DTI_PRN_USR_ERR_NL(module_name, dbg_level, print_message)  /*lint -e{19} */
#endif  /* #if (DLI_DEBUG_PRINT == 1) */

#define DTI_PRN_LEVEL_HIGH          IFXOS_PRN_LEVEL_HIGH
#define DTI_PRN_LEVEL_NORMAL        IFXOS_PRN_LEVEL_NORMAL
#define DTI_PRN_LEVEL_LOW           IFXOS_PRN_LEVEL_LOW
#define DTI_PRN_LEVEL_WRN           IFXOS_PRN_LEVEL_WRN
#define DTI_PRN_LEVEL_ERR           IFXOS_PRN_LEVEL_ERR


/* device stuff */
#define DTI_DeviceOpen              IFXOS_DeviceOpen
#define DTI_DeviceClose             IFXOS_DeviceClose
#define DTI_DeviceControl           IFXOS_DeviceControl
#define DTI_DeviceSelect            IFXOS_DeviceSelect
#define DTI_DevFdSet                IFXOS_DevFdSet
#define DTI_DevFdIsSet              IFXOS_DevFdIsSet
#define DTI_DevFdZero               IFXOS_DevFdZero

#if defined(IFXOS_HAVE_VERSION_CHECK)
#  if (IFXOS_VERSION_CHECK_EG_THAN(1,1,0))
#     define IFXOS_SUPPORTS_FIFO_PEEK                 1
#  else
#     define IFXOS_SUPPORTS_FIFO_PEEK                 0
#  endif
#endif

#define DTI_Var_Fifo_Init           IFX_Var_Fifo_Init
#define DTI_Var_Fifo_Clear          IFX_Var_Fifo_Clear
#define DTI_Var_Fifo_readElement    IFX_Var_Fifo_readElement
#define DTI_Var_Fifo_peekElement    IFX_Var_Fifo_peekElement
#define DTI_Var_Fifo_writeElement   IFX_Var_Fifo_writeElement
#define DTI_Var_Fifo_isEmpty        IFX_Var_Fifo_isEmpty
#define DTI_Var_Fifo_isFull         IFX_Var_Fifo_isFull
#define DTI_Var_Fifo_getCount       IFX_Var_Fifo_getCount


#if (defined(IFXOS_HAVE_FILE_ACCESS) && (IFXOS_HAVE_FILE_ACCESS == 1))
#  define DTI_File_t       IFXOS_File_t
#else
#  define DTI_File_t       IFX_void_t
#endif

#define DTI_FOpen                   IFXOS_FOpen
#define DTI_FClose                  (void)IFXOS_FClose
#define DTI_FWrite                  IFXOS_FWrite

#ifdef __cplusplus
/* extern "C" */
}
#endif

#endif      /* #define _DTI_OSMAP_H */

