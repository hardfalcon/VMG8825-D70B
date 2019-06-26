#ifndef _DRV_MPS_VMMC_CRC32_H
#define _DRV_MPS_VMMC_CRC32_H
/******************************************************************************

                              Copyright (c) 2012
                            Lantiq Deutschland GmbH
                             http://www.lantiq.com

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

****************************************************************************
   Module      : drv_mps_vmmc_crc32.h
   Description : This file contains the macro definitions and function
                 prototypes for CRC-32 checksum calculation.
*******************************************************************************/
/* ============================= */
/* Includes                      */
/* ============================= */
#include "ifx_types.h"

/* ============================= */
/* Local Macros & Definitions    */
/* ============================= */
/* crc32 calculation method via lookup table is used per default for fast code;
   however, if a compact but slow code is required - comment out the below
   definition of CRC32_USE_LOOKUP_TBL */
#define CRC32_USE_LOOKUP_TBL

#define POLY 0x04c11db7L   /* standard CRC-32 polynomial */

/* ============================= */
/* Function prototypes           */
/* ============================= */
IFX_void_t   ifx_mps_gen_crc32_tbl(void);
IFX_uint32_t ifx_mps_calc_crc32(IFX_uint32_t crc, IFX_uint8_t *pData,
                                IFX_uint32_t size, IFX_boolean_t bEnd);

#endif  /* _DRV_MPS_VMMC_CRC32_H */
