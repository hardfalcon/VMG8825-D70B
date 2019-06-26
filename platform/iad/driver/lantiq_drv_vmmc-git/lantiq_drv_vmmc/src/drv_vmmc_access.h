#ifndef _DRV_VINETIC_ACCESS_H
#define _DRV_VINETIC_ACCESS_H
/******************************************************************************

                              Copyright (c) 2013
                            Lantiq Deutschland GmbH
                             http://www.lantiq.com

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/**
   \file drv_vmmc_access.h
   Low level access macros and functions declarations.
*/

/* ============================= */
/* Includes                      */
/* ============================= */
#include "drv_vmmc_osmap.h"
#ifdef LINUX
#include "drv_mps_vmmc.h"
#endif

/* ============================= */
/* Global Defines                */
/* ============================= */

/** \defgroup HOST_MBX_ACCESS_PROTECTION Mailbox access protection
*/
/*@{*/

/** Protects command mailbox access.
\param  pDev - handle to device
\remark Protection is done against concurrent tasks and interrupts
*/
#define VMMC_HOST_PROTECT(pDev) \
   do{\
         if (!VMMC_OS_IN_INTERRUPT())\
            VMMC_OS_MutexGet (&(pDev)->mtxCmdMbxAcc);\
         Vmmc_IrqLockDevice((pDev));\
   } while(0)

/** Releases host mailbox access protection
\param  pDev - handle to device
*/
#define VMMC_HOST_RELEASE(pDev) \
   do{\
         Vmmc_IrqUnlockDevice((pDev));\
         if (!VMMC_OS_IN_INTERRUPT())\
            VMMC_OS_MutexRelease (&(pDev)->mtxCmdMbxAcc);\
   } while(0)
/*@}*/


/** \defgroup 32BIT_MUX_ACCESS_MACROS 32 Bit MUX access macros
*/
/*@{*/

/** Read a register */
#define VMMC_READ_REG(offset)\
   (*((volatile u32*)(IFX_MPS_BASE_ADDR + (offset))))

/** Write a register */
#define VMMC_WRITE_REG(offset, value)\
   (*((volatile u32*)(IFX_MPS_BASE_ADDR + (offset))) = (value))

/*@}*/

#endif /* _DRV_VINETIC_ACCESS_H */
