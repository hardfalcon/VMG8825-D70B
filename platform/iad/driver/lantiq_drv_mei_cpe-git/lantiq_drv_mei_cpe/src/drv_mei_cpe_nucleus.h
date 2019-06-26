#ifndef _DRV_MEI_CPE_NUCLEUS_H
#define _DRV_MEI_CPE_NUCLEUS_H
/******************************************************************************

                          Copyright (c) 2007-2015
                     Lantiq Beteiligungs-GmbH & Co. KG

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/* ============================================================================
   Description : This file contains the includes and the defines
                 specific to the Nucleus OS
   Remarks     : Please use the compiler switches here if you have
                 more than one OS.
   ========================================================================= */

/* ============================================================================
   Global Includes
   ========================================================================= */

/* NUCLEUS Includes*/

/* ============================================================================
   typedefs interrupt wrapping (Nucleus)
   ========================================================================= */

#define MEI_DRVOS_SIGNAL_PENDING             0

/**
   Function typedef for the NUCLEUS intConnect()
*/
typedef int (*MEI_IntConnect_WrapNUCLEUS_t)(void *pIntVector,  void* pISRRoutine, int ISRParams );

/**
   Function typedef for the NUCLEUS Interrupt enable intEnable()
*/
typedef int (*MEI_IntEnable_WrapNUCLEUS_t)(int IRQNum);

/**
   Function typedef for the NUCLEUS Interrupt disable intDisable()
*/
typedef int (*MEI_IntDisable_WrapNUCLEUS_t)(int IRQNum);

/**
   Function typedef for the NUCLEUS Interrupt Service Routine
*/
typedef void (*MEI_IntServRoutine_WrapNUCLEUS_t)(int ISRParams);


/* ============================================================================
   global function (Nucleus) - declarations
   ========================================================================= */

extern int MEI_DevCreate(void);
extern int MEI_DevDelete(void);

/* set wrapper functions for the interrupt handling */
extern int MEI_FctIntConnectSet(MEI_IntConnect_WrapNUCLEUS_t pIntConnectFct);

extern int MEI_FctIntEnableSet(MEI_IntEnable_WrapNUCLEUS_t  pIntEnableFct);
extern int MEI_FctIntDisableSet(MEI_IntDisable_WrapNUCLEUS_t pIntDisableFct);

#endif /* _DRV_MEI_CPE_NUCLEUS_H */

