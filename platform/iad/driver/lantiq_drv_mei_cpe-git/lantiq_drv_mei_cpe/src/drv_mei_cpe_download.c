/******************************************************************************

                          Copyright (c) 2007-2015
                     Lantiq Beteiligungs-GmbH & Co. KG

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/* ==========================================================================
   Description : Common functions used for FW download via the VRX Driver
   ========================================================================== */


/* ============================================================================
   Inlcudes
   ========================================================================= */

/* get at first the driver configuration */
#include "drv_mei_cpe_config.h"

#include "ifx_types.h"
#include "drv_mei_cpe_os.h"
#include "drv_mei_cpe_dbg.h"

#include "drv_mei_cpe_dbg_driver.h"

#include "drv_mei_cpe_interface.h"
#include "drv_mei_cpe_mei_interface.h"
#include "drv_mei_cpe_api.h"

#include "drv_mei_cpe_msg_process.h"
#include "drv_mei_cpe_download.h"

#if (MEI_SUPPORT_DRV_MODEM_TESTS == 1)
#include "drv_mei_cpe_modem_test.h"
#endif /* (MEI_SUPPORT_DRV_MODEM_TESTS == 1)*/


/**
   Semaphore to protect:
   - config changes (FW download, image load)
*/
MEI_DRVOS_sema_t  pFwDlCntrlLock;


/* ============================================================================
   Local function declaration
   ========================================================================= */


/* ============================================================================
   Local variable definitions
   ========================================================================= */


/* ============================================================================
   Firmware Download function definition
   ========================================================================= */

/* ============================================================================
   Function definition (FW Download ioctl)
   ========================================================================= */

/**
   Do the firmware download for the current VRX.

\param
   pMeiDynCntrl - private dynamic comtrol data (per open instance)
\param
   pFwDl          - points to the FW downlosd information data
\param
   bInternCall    - indicates if the call is form the internal interface
                    (image and data already in kernel space)
\return
   IFX_SUCCESS: if the FW was successful.
   negative value if something went wrong.

*/
IFX_int32_t MEI_IoctlFirmwareDownload(
                                 MEI_DYN_CNTRL_T        *pMeiDynCntrl,
                                 IOCTL_MEI_fwDownLoad_t *pArgFwDl,
                                 IFX_boolean_t            bInternCall)
{
   return MEI_DEV_IoctlFirmwareDownload(
             pMeiDynCntrl, pArgFwDl, bInternCall);
}

