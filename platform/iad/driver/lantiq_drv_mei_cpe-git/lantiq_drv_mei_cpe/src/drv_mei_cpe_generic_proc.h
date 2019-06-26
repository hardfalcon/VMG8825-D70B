#ifndef _DRV_MEI_CPE_VXWORKS_PROC_H
#define _DRV_MEI_CPE_VXWORKS_PROC_H
/******************************************************************************

                          Copyright (c) 2007-2015
                     Lantiq Beteiligungs-GmbH & Co. KG

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/* ============================================================================
   Description : VRX Driver, proc file system replacement, generic part
   ========================================================================= */

#if !defined(LINUX) && defined(MEI_GENERIC_PROC_FS)

#ifdef __cplusplus
extern "C"
{
#endif

/* ============================================================================
   Inlcudes
   ========================================================================= */
#include "ifx_types.h"
#include <stdio.h>

/* ==========================================================================
   Global proc config functions
   ========================================================================== */

extern IFX_void_t MEI_ShowConfigProc(FILE *streamOut);
extern IFX_void_t MEI_SetConfigProc(FILE *streamOut, IFX_char_t shortCut, IFX_int32_t setVal);
extern IFX_void_t MEI_ShowVersionProc(FILE *streamOut);
extern IFX_void_t MEI_ShowStatusProc(FILE *streamOut);
extern IFX_void_t MEI_ShowNfcProc(FILE *streamOut);


extern int doVrxProcFs_fd(
                     FILE *streamOut,
                     int cmd, int procEntry, int param0, int param1, int param2);

extern int doVrxProcFs(
                     int cmd, int procEntry, int param0, int param1, int param2);


#ifdef __cplusplus
/* extern "C" */
}
#endif

#endif      /* #if !defined(LINUX) && defined(MEI_GENERIC_PROC_FS) */

#endif      /* #ifndef _DRV_MEI_CPE_GENERIC_PROC_H */

