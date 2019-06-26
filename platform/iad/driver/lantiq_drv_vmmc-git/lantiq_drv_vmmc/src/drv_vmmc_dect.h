#ifndef _DRV_VMMC_DECT_H
#define _DRV_VMMC_DECT_H
/******************************************************************************

                              Copyright (c) 2012
                            Lantiq Deutschland GmbH
                             http://www.lantiq.com

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

*******************************************************************************
   Module      : drv_vmmc_dect.h
   Description : This file contains the defines
                 and the global functions declarations of DECT module
******************************************************************************/

/*
 * Function prototypes
 */

extern IFX_return_t VMMC_DECT_InitCh (VMMC_CHANNEL *pCh);
extern IFX_return_t VMMC_DECT_Set_Inputs (VMMC_CHANNEL *pCh);
extern IFX_int32_t  VMMC_DECT_ChStop (VMMC_CHANNEL *pCh);

extern IFX_void_t   VMMC_DECT_Func_Register (IFX_TAPI_DRV_CTX_DECT_t *pDECT);
extern IFX_int32_t  VMMC_DECT_Allocate_Ch_Structures (VMMC_CHANNEL *pCh);
extern IFX_void_t   VMMC_DECT_Free_Ch_Structures (VMMC_CHANNEL *pCh);
extern IFX_int32_t  VMMC_DECT_SamplingMode (VMMC_CHANNEL *pCh,
                                            SM_ACTION action,
                                            OPMODE_SMPL mode);

#endif /* _DRV_VMMC_DECT_H */
