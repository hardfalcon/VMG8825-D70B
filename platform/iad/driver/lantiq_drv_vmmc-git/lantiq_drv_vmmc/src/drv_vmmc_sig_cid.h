#ifndef _DRV_VMMC_CID_H
#define _DRV_VMMC_CID_H
/******************************************************************************

                              Copyright (c) 2012
                            Lantiq Deutschland GmbH
                             http://www.lantiq.com

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

*******************************************************************************/

/*
   \file drv_vmmc_cid.h
   This file contains the declaration of the functions for CID operations
*/

/* ============================= */
/* Check if feature is enabled   */
/* ============================= */
#ifdef HAVE_CONFIG_H
#include <drv_config.h>
#endif

#ifdef TAPI_CID

/* ============================= */
/* Global Defines                */
/* ============================= */

/** CID transistions */
enum VMMC_CID_ACTION
{
   VMMC_CID_ACTION_STOP     = 0,
   VMMC_CID_ACTION_START    = 1,
   VMMC_CID_ACTION_REQ_DATA = 2
};

/* ============================= */
/* Global Structures             */
/* ============================= */

/* ============================= */
/* Global function declaration   */
/* ============================= */

extern IFX_int32_t  VMMC_CidFskMachine  (VMMC_CHANNEL *pCh,
                                         enum VMMC_CID_ACTION nAction);

extern IFX_void_t   VMMC_CidDtmfMachine (VMMC_CHANNEL *pCh);

extern IFX_return_t irq_VMMC_SIG_CID_RX_Data_Collect (TAPI_CHANNEL *pChannel,
                                                      IFX_uint16_t *pPacket,
                                                      IFX_uint32_t nLength);


/* Prototypes for functions which are exported via the function
   VINETIC_SIG_Func_Register(). The prototype for the function pointers are
   defined in drv_tapi_ll_interface.h and must be identical to the prototypes
   below. */

extern IFX_int32_t VMMC_TAPI_LL_SIG_CID_TX_Start (
                        IFX_TAPI_LL_CH_t *pLLChannel,
                        IFX_TAPI_CID_TX_t const *pCidData);

extern IFX_int32_t VMMC_TAPI_LL_SIG_CID_TX_Stop (
                        IFX_TAPI_LL_CH_t *pLLChannel);


extern IFX_int32_t VMMC_TAPI_LL_SIG_CID_RX_Start (
                        IFX_TAPI_LL_CH_t *pLLChannel,
                        IFX_TAPI_CID_RX_t const *pCidData);

extern IFX_int32_t VMMC_TAPI_LL_SIG_CID_RX_Stop (
                        IFX_TAPI_LL_CH_t *pLLChannel,
                        IFX_TAPI_CID_RX_CFG_t const *pCid);

#endif /* TAPI_CID */
#endif /* _DRV_VMMC_CID_H */
