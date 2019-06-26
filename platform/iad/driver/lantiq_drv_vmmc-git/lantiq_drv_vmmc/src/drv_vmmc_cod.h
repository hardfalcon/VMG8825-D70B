#ifndef _DRV_VMMC_COD_H
#define _DRV_VMMC_COD_H
/******************************************************************************

                              Copyright (c) 2012
                            Lantiq Deutschland GmbH
                             http://www.lantiq.com

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/**
   \file drv_vmmc_cod.h  Header file of the CODer module.
   This file contains the defines and the global functions declarations
   of the coder module.
*/

/* ============================= */
/* Includes                      */
/* ============================= */
#include "drv_tapi_ll_interface.h"

/* ============================= */
/* Global Defines                */
/* ============================= */

/* ============================= */
/* Global Macros                 */
/* ============================= */
/** True if AMR codec. otherwise not true. */
#define AMR_CODEC(m_nCod, m_bAMRE) \
   ( ( !m_bAMRE && \
       ( m_nCod >= COD_CHAN_SPEECH_ENC_AMR_4_75 && \
         m_nCod <= COD_CHAN_SPEECH_ENC_AMR_12_2 ) ) || \
     ( m_bAMRE && \
       ( m_nCod == COD_CHAN_SPEECH_ENC_AMR_NB || \
         m_nCod == COD_CHAN_SPEECH_ENC_AMR_WB) ) )

/* ============================= */
/* Global Types                  */
/* ============================= */

/* ============================= */
/* Global function declaration   */
/* ============================= */
extern IFX_int32_t VMMC_COD_InitCh (VMMC_CHANNEL *pCh);
extern IFX_int32_t VMMC_COD_Set_Inputs (VMMC_CHANNEL *pCh);
extern IFX_int32_t VMMC_COD_baseConf (VMMC_CHANNEL *pCh);
extern IFX_int32_t VMMC_COD_ChStop (VMMC_CHANNEL *pCh);

extern IFX_void_t  VMMC_COD_Func_Register (IFX_TAPI_DRV_CTX_COD_t *pCOD);
extern IFX_int32_t VMMC_COD_Allocate_Ch_Structures (VMMC_CHANNEL *pCh);
extern IFX_void_t  VMMC_COD_Free_Ch_Structures (VMMC_CHANNEL *pCh);

extern IFX_int32_t VMMC_COD_SamplingMode (VMMC_CHANNEL *pCh,
                                          SM_ACTION action,
                                          OPMODE_SMPL mode);

extern IFX_TAPI_COD_TYPE_t VMMC_COD_trans_cod_fw2tapi (IFX_int8_t nCoder,
                                                       IFX_uint8_t bAMRE);
extern IFX_uint8_t VMMC_COD_trans_cod_tapi2fw (IFX_TAPI_COD_TYPE_t nCoder,
                                               IFX_TAPI_COD_BITRATE_t nBitRate,
                                               IFX_uint8_t bAMRE);

extern IFX_enDis_t  VMMC_COD_ChStatusGet (VMMC_CHANNEL *pCh);

extern IFX_int32_t VMMC_COD_ENC_Hold(VMMC_CHANNEL     *pCh,
                                     IFX_operation_t  nOnHold);
extern IFX_void_t VMMC_COD_CmrDec_Update (VMMC_CHANNEL     *pCh,
                                          IFX_uint8_t CmrDec);

#endif /* _DRV_VMMC_COD_H */
