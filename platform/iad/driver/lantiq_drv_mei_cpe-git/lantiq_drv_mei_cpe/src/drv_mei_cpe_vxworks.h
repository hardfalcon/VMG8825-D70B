#ifndef _DRV_MEI_CPE_VXWORKS_H
#define _DRV_MEI_CPE_VXWORKS_H
/******************************************************************************

                          Copyright (c) 2007-2015
                     Lantiq Beteiligungs-GmbH & Co. KG

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/* ============================================================================
   Description : This file contains the includes and the defines
                 specific to the vxWorks OS
   Remarks     : Please use the compiler switches here if you have
                 more than one OS.
   ========================================================================= */

/* ============================================================================
   Global Includes
   ========================================================================= */

/* VxWorks Includes*/

#include <vxworks.h>
#include "intLib.h"

/* ============================================================================
   typedefs interrupt wrapping (vxWorks)
   ========================================================================= */

#define MEI_DRVOS_SIGNAL_PENDING             0

/**
   Function typedef for the VxWorks intConnect()
*/
typedef STATUS (*MEI_IntConnect_WrapVxW_t)(VOIDFUNCPTR *pIntVector, VOIDFUNCPTR pISRRoutine, int ISRParams);

/**
   Function typedef for the VxWorks Interrupt enable intEnable()
*/
typedef int (*MEI_IntEnable_WrapVxW_t)(int IRQNum);

/**
   Function typedef for the VxWorks Interrupt disable intDisable()
*/
typedef int (*MEI_IntDisable_WrapVxW_t)(int IRQNum);

/**
   Function typedef for the VxWorks Interrupt Service Routine
*/
typedef void (*MEI_IntServRoutine_WrapVxW_t)(int ISRParams);


/* ============================================================================
   global function (vxWorks) - declarations
   ========================================================================= */

extern int MEI_DevCreate(void);
extern int MEI_DevDelete(void);

/* set wrapper functions for the interrupt handling */
extern int MEI_FctIntConnectSet(MEI_IntConnect_WrapVxW_t pIntConnectFct);

extern int MEI_FctIntEnableSet(MEI_IntEnable_WrapVxW_t  pIntEnableFct);
extern int MEI_FctIntDisableSet(MEI_IntDisable_WrapVxW_t pIntDisableFct);

#endif /* _DRV_MEI_CPE_VXWORKS_H */

