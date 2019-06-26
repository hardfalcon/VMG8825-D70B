#ifndef _drv_vmmc_con_H
#define _drv_vmmc_con_H
/******************************************************************************

                    Copyright (c) 2006-2009, 2011, 2013-2015
                        Lantiq Beteiligungs-GmbH & Co.KG
                             http://www.lantiq.com

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/**
   \file drv_vmmc_con.h
   This file contains the declaration of the connection module.
*/

/* ============================= */
/* Includes                      */
/* ============================= */
#include "drv_vmmc_api.h"
#include "drv_tapi_ll_interface.h"

/* ============================= */
/* Global Defines                */
/* ============================= */

/* ============================= */
/* Global Types                  */
/* ============================= */

/** defines which output of the signaling module is to be used
    ALM and PCM are normally attached on local side and COD on remote side */
typedef enum
{
   REMOTE_SIG_OUT = 0,
   LOCAL_SIG_OUT = 1
} SIG_OUTPUT_SIDE;

/** which signaling input is used for local (with auto suppression) and remote
    (with event playout) connections. This is fixed by firmware */
enum
{
   REMOTE_SIG_IN = 1,
   LOCAL_SIG_IN = 0
};

/** Module types of the firmware */
typedef enum
{
   VMMCDSP_MT_ALM,
   VMMCDSP_MT_SIG,
   VMMCDSP_MT_COD,
   VMMCDSP_MT_PCM,
   VMMCDSP_MT_DECT,
   VMMCDSP_MT_LIN
} VMMCDSP_MT;

/** Defines module sampling operation mode */
typedef enum
{
   /** Module sampling is disabled */
   VMMC_CON_SMPL_OFF,
   /** Module can only operate in narrowband mode */
   VMMC_CON_SMPL_NB,
   /** Module can only operate in wideband mode */
   VMMC_CON_SMPL_WB,
   /** Module can operate in both - NB and WB - mode. Actual mode is
       determined by the operation mode of conference this module belongs to. */
   VMMC_CON_SMPL_AUTO
} VMMC_CON_SAMPLING;

/* ============================= */
/* Global Variables              */
/* ============================= */

/* ============================= */
/* Global function declaration   */
/* ============================= */

extern IFX_int32_t VMMC_CON_Allocate_Ch_Structures (VMMC_CHANNEL *pCh);
extern IFX_void_t VMMC_CON_Free_Ch_Structures (VMMC_CHANNEL *pCh);

extern IFX_void_t VMMC_CON_Init_AlmCh (VMMC_CHANNEL *pCh);
#ifdef VMMC_FEAT_PCM
extern IFX_void_t VMMC_CON_Init_PcmCh (VMMC_CHANNEL *pCh, IFX_uint8_t pcmCh);
#endif /* VMMC_FEAT_PCM */
extern IFX_void_t VMMC_CON_Init_CodCh (VMMC_CHANNEL *pCh);
extern IFX_void_t VMMC_CON_Init_SigCh (VMMC_CHANNEL *pCh);
#ifdef DECT_SUPPORT
extern IFX_void_t VMMC_CON_Init_DectCh (VMMC_CHANNEL *pCh);
#endif /* DECT_SUPPORT */
extern IFX_void_t VMMC_CON_Init_LinCh (VMMC_CHANNEL *pCh);


extern IFX_uint8_t VMMC_CON_Get_ALM_SignalInput (
                        VMMC_CHANNEL *pCh,
                        IFX_uint8_t sig_index);

#ifdef VMMC_FEAT_PCM
extern IFX_uint8_t VMMC_CON_Get_PCM_SignalInput (
                        VMMC_CHANNEL *pCh,
                        IFX_uint8_t sig_index);
#endif /* VMMC_FEAT_PCM */

extern IFX_uint8_t VMMC_CON_Get_COD_SignalInput (
                        VMMC_CHANNEL *pCh,
                        IFX_uint8_t sig_index);

extern IFX_uint8_t VMMC_CON_Get_SIG_SignalInput (
                        VMMC_CHANNEL *pCh,
                        IFX_uint8_t sig_index);

#ifdef DECT_SUPPORT
extern IFX_uint8_t VMMC_CON_Get_DECT_SignalInput (
                        VMMC_CHANNEL *pCh,
                        IFX_uint8_t sig_index);
#endif /* DECT_SUPPORT */

extern IFX_void_t  VMMC_CON_Func_Register (
                        IFX_TAPI_DRV_CTX_CON_t *pCON);

extern IFX_int32_t VMMC_CON_ConnectPrepare (
                        VMMC_CHANNEL *pSrcCh,
                        VMMCDSP_MT src,
                        VMMC_CHANNEL *pDstCh,
                        VMMCDSP_MT dst,
                        SIG_OUTPUT_SIDE nSide);

extern IFX_int32_t VMMC_CON_ConnectConfigure (
                        VMMC_DEVICE *pDev);

extern IFX_int32_t VMMC_CON_MatchConfSmplRate (
                        VMMC_CHANNEL *pCh,
                        VMMCDSP_MT start_module);

extern IFX_void_t  VMMC_CON_ModuleSamplingModeSet (
                        VMMC_CHANNEL *pCh,
                        VMMCDSP_MT src,
                        VMMC_CON_SAMPLING sampling_mode);

extern VMMC_CHANNEL *VMMC_CON_SingleDataChannelCodFind (
                        VMMC_CHANNEL *pCh,
                        VMMCDSP_MT mod);

#endif /* _drv_vmmc_con_H */
