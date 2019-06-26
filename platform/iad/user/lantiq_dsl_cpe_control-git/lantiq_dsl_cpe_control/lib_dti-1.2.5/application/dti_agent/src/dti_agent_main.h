#ifndef _DTI_CLI_H
#define _DTI_CLI_H
/******************************************************************************

                              Copyright (c) 2009
                            Lantiq Deutschland GmbH
                     Am Campeon 3; 85579 Neubiberg, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/** \file
   Debug and Trace Interface - Command Line Interface (CLI) definitions and declarations.
*/

#ifdef __cplusplus
   extern "C" {
#endif


/* ============================================================================
   Includes
   ========================================================================= */
#include "ifx_types.h"


/* ============================================================================
   Defines
   ========================================================================= */

/* ============================================================================
   Typedefs
   ========================================================================= */

/* ============================================================================
   Exports
   ========================================================================= */

/**
   DTI Agent - Standalone
*/
extern IFX_int_t DTI_agent_libmain(
                     const IFX_char_t  *pArgString);



#ifdef __cplusplus
}
#endif

#endif   /* #ifndef _DTI_CLI_H */


