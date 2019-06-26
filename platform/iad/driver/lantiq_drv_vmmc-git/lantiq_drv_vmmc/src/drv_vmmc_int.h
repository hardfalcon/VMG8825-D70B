#ifndef _drv_vmmc_int_H
#define _drv_vmmc_int_H
/****************************************************************************

                              Copyright (c) 2012
                            Lantiq Deutschland GmbH
                             http://www.lantiq.com

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

 ****************************************************************************
   Module      : drv_vmmc_int.h
   Description : This file contains the declaration of all interrup specific
                 messages.
*******************************************************************************/

/* ============================= */
/* Global Defines                */
/* ============================= */

/* ============================= */
/* Global Structures             */
/* ============================= */

/* ============================= */
/* Global Variables              */
/* ============================= */

/* ============================= */
/* Global function declaration   */
/* ============================= */
IFX_return_t VMMC_Register_Callback(VMMC_DEVICE *pDev);
IFX_return_t VMMC_UnRegister_Callback(void);

#endif /* _drv_vmmc_int_H */

