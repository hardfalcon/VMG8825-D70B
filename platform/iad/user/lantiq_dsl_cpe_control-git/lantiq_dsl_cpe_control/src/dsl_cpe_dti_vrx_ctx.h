#ifndef _DTI_VRX_CTX_H
#define _DTI_VRX_CTX_H
/******************************************************************************

                          Copyright (c) 2007-2015
                     Lantiq Beteiligungs-GmbH & Co. KG

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/
#ifdef __cplusplus
   extern "C" {
#endif

/** \file
   VRX device context, control struct for xDSL Rev3 access and handling.
*/

/* ==========================================================================
   includes
   ========================================================================== */
#include "ifx_types.h"
#include "ifxos_debug.h"
#include "ifxos_device_access.h"

/* ==========================================================================
   defines
   ========================================================================== */
#ifndef DTI_DEV_MAX_LINE_NUMBER
#  define DTI_DEV_MAX_LINE_NUMBER                           1
#endif

/* ==========================================================================
   macro
   ========================================================================== */

/* ==========================================================================
   types
   ========================================================================== */

/**
   VRX Driver Control Struct
*/
typedef struct DTI_DEV_VrxDriverCtx_s
{
   /** number of devices */
   IFX_int_t   numOfDevs;
   /** ports per device */
   IFX_int_t   portsPerDev;
   /** number of ports */
   IFX_int_t   numOfPorts;
   /** points to the array of devices fds */
   IFX_int_t   *pDevFds;
   /*
      DTI Config
   */
   /** max number of register requests / sets */
   IFX_int_t   numOfRegAccess;
   /** max number of debug requests / sets */
   IFX_int_t   numOfDebugAccess;    
   
   /** auto msg handling - supported */
   IFX_boolean_t     bAutoDevMsgSupport;
   /** auto msg handling - max device fd for select */
   IFX_int_t         nfcMaxDevFd;
   /** auto msg handling - device FDSET */
   IFXOS_devFd_set_t nfcDevFds;
   /** auto msg handling - for temporary use, device FDSET */
   IFXOS_devFd_set_t tmpDevFds;
   /** auto msg handling - recv buffer for auto msg's */
   IFX_ulong_t       *pAutoMsgBuf;
   /** auto msg handling - size of the modem message buffer for NFC handling */
   IFX_int_t         autoMsgBuf_byte;
   /** debug stream local user buffer */
   IFX_uint8_t       *pDbgStreamUserBuf;
   /** size of the local user buffer for debug stream handling */
   IFX_int_t         dbgStreamUserBuf_byte;
} DTI_DEV_VrxDriverCtx_t;


/* ==========================================================================
   exports
   ========================================================================== */



#ifdef __cplusplus
}
#endif

#endif /* _DTI_VRX_CTX_H */
