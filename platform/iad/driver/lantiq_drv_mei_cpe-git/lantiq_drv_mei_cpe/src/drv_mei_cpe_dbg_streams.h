/****************************************************************************

                          Copyright (c) 2007-2015
                     Lantiq Beteiligungs-GmbH & Co. KG

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

*****************************************************************************/
#ifndef _DRV_MEI_CPE_DBG_STREAMS_H
#define _DRV_MEI_CPE_DBG_STREAMS_H



/** \file
   Debug Streams: common defines for the MEI Debug stream handling.
*/

/* ==========================================================================
   includes
   ========================================================================== */
#ifdef __cplusplus
   extern "C" {
#endif

#include "drv_mei_cpe_config.h"

#if (MEI_SUPPORT_DEBUG_STREAMS == 1)

#include "ifx_types.h"
#include "drv_mei_cpe_os.h"
#include "drv_mei_cpe_api.h"

extern IFX_void_t MEI_DBG_STREAM_EventRecv(
                              MEI_DEV_T               *pMeiDev,
                              MEI_DYN_DBG_STRM_DATA_T *pDbgStrmRootInstance,
                              CMV_STD_MESSAGE_T       *pMsg);
                              
/* ==========================================================================
   Debug Streams - Global Variables.
   ========================================================================== */

/* MEI Driver: Debug Stream debug module - declare print level variable */
MEI_DRV_PRN_USR_MODULE_DECL(MEI_DBG_STREAM);
MEI_DRV_PRN_INT_MODULE_DECL(MEI_DBG_STREAM);
MEI_DRV_PRN_INT_MODULE_DECL(MEI_DBG_STREAM_DUMP);
extern IFX_uint8_t MEI_DRV_dbgStreamsOn[]; 

#endif      /* #if (MEI_SUPPORT_DEBUG_STREAMS == 1) */

#ifdef __cplusplus
}
#endif

#endif      /* #ifndef _DRV_MEI_CPE_DBG_STREAMS_H */
