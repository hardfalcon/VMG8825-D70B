/******************************************************************************

                          Copyright (c) 2007-2015
                     Lantiq Beteiligungs-GmbH & Co. KG

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/* ============================================================================
   Description : VRX Driver, VxWorks BSP part
   ========================================================================= */

/* ============================= */
/* Group=Main                    */
/* ============================= */


#if defined(VXWORKS) && defined(INCLUDE_VRX)
/*
  sorry for this style, but this is a easy way to include the driver code
  into the board support package of VxWorks
*/

/* get at first the driver configuration */
#include "drv_mei_cpe_config.h"

#include "ifx_types.h"
/* add VRX OS Layer */
#include "drv_mei_cpe_os.h"
/* add VRX debug/printout part */
#include "drv_mei_cpe_dbg.h"

#include "drv_mei_cpe_api.h"
#include "drv_mei_cpe_vxworks.c"
#include "drv_mei_cpe_common.c"

#endif

