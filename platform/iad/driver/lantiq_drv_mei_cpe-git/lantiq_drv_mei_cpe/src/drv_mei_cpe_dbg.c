/******************************************************************************

                          Copyright (c) 2007-2015
                     Lantiq Beteiligungs-GmbH & Co. KG

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/* ============================================================================
   Description : VRX Driver, Debug Part
   ========================================================================= */

/* ============================================================================
   Includes
   ========================================================================= */

/* get at first the driver configuration */
#include "drv_mei_cpe_config.h"

#if (MEI_DEBUG_PRINT == 1)

#include "ifx_types.h"
#include "drv_mei_cpe_os.h"
#include "drv_mei_cpe_dbg.h"



/* Global Debug level - control all debug outputs */
MEI_DRV_PRN_USR_MODULE_CREATE(DBG_GLOBAL, MEI_DRV_PRN_LEVEL_LOW);
MEI_DRV_PRN_INT_MODULE_CREATE(DBG_GLOBAL, MEI_DRV_PRN_LEVEL_LOW);

/* VRX-Driver: global debug control variable */
MEI_DRV_PRN_DEBUG_CONTROL_CREATE(0);
/* MEI_DRV_PRN_DEBUG_CONTROL_CREATE((MEI_DBG_LINE_PRINT_INDICATION | 0x1)); */


/* ============================================================================
   Debug helper functions
   ========================================================================= */

/**
   Debug helper function - check if the debug per line is enabled
   - the debug per line must be enabled.
   - the corresponding line must be marked within the line array.
*/
IFX_boolean_t MEI_DbgCheckForLine(
                              IFX_uint8_t  lineNum )
{
   if (MEI_DebugCntrl & MEI_DBG_LINE_PRINT_INDICATION)
   {
      if ( (0x00000001 << (lineNum % 32)) & MEI_DebugCntrlLine[((lineNum / 32) & 0x03)] )
      {
         return IFX_TRUE;
      }
      else
      {
         return IFX_FALSE;
      }
   }

   return IFX_TRUE;
}

#endif   /* #if (MEI_DEBUG_PRINT == 1) */


