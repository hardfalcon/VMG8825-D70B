/******************************************************************************

                              Copyright (c) 2009
                            Lantiq Deutschland GmbH
                     Am Campeon 3; 85579 Neubiberg, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/
#ifndef _DTI_DEV_GENERIC_CTX_H
#define _DTI_DEV_GENERIC_CTX_H

#ifdef __cplusplus
   extern "C" {
#endif

/** \file
   Generic device context, control struct for Generic device access and handling.
*/

/* ==========================================================================
   includes
   ========================================================================== */
#include "ifx_types.h"
#include "ifxos_debug.h"

/* ==========================================================================
   defines
   ========================================================================== */

/* ==========================================================================
   macro
   ========================================================================== */

/* ==========================================================================
   types
   ========================================================================== */

/**
   Generic Driver Control Struct
*/
typedef struct DTI_DEV_GenericDriverCtx_s
{
   /** number of devices */
   IFX_int_t   numOfDevs;
   /** ports per device */
   IFX_int_t   portsPerDev;
   /** number of ports */
   IFX_int_t   numOfPorts;

} DTI_DEV_GenericDriverCtx_t;


/* ==========================================================================
   exports
   ========================================================================== */

IFXOS_PRN_USR_MODULE_DECL(DTI_GEN);


#ifdef __cplusplus
}
#endif

#endif /* _DTI_DEV_GENERIC_CTX_H */
