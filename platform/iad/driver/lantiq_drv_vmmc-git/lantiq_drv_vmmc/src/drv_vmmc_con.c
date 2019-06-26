/******************************************************************************

                    Copyright (c) 2006-2009, 2011, 2013-2016
                        Lantiq Beteiligungs-GmbH & Co.KG
                             http://www.lantiq.com

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/**
   \file drv_vmmc_con.c VMMC DSP module interconnection managment module.
   This module provides functions to connect different DSP modules. It is
   used for all kinds of connections including multi party conferences.
*/

/* ============================= */
/* Includes                      */
/* ============================= */
#include "drv_api.h"
#include "drv_vmmc_con_priv.h"
#include "drv_vmmc_api.h"
#include "drv_vmmc_con.h"
#include "drv_vmmc_alm.h"
#include "drv_vmmc_sig.h"
#include "drv_vmmc_cod.h"
#ifdef VMMC_FEAT_PCM
#include "drv_vmmc_pcm.h"
#endif /* VMMC_FEAT_PCM */
#ifdef DECT_SUPPORT
#include "drv_vmmc_dect.h"
#endif /* DECT_SUPPORT */
#ifdef VMMC_USE_PROC
#ifdef LINUX
   /* sequence file is available since Linux 2.6.32 */
   #include <linux/seq_file.h>
#endif /* LINUX */
#include "drv_vmmc_init.h"
#endif /* VMMC_USE_PROC */

/* ============================= */
/* Local Macros & Definitions    */
/* ============================= */

#ifdef DEBUG
static const IFX_char_t *signalName[64] = {
   "Null",    "COD6",    "COD7",    "COD8",
   "ALM0",    "ALM1",    "ALM2",    "COD9",
   "PCM0",    "PCM1",    "PCM2",    "PCM3",
   "PCM4",    "PCM5",    "PCM6",    "PCM7",
   "PCM8",    "PCM9",    "PCM10",   "PCM11",
   "PCM12",   "PCM13",   "PCM14",   "PCM15",
   "COD0",    "COD1",    "COD2",    "COD3",
   "COD4",    "COD5",    "SIG4-A",  "SIG4-B",
   "SIG0-A",  "SIG0-B",  "SIG1-A",  "SIG1-B",
   "SIG2-A",  "SIG2-B",  "SIG3-A",  "SIG3-B",
   "",        "",        "SIG5-A",  "SIG5-B",
   "SIG6-A",  "SIG6-B",  "SIG7-A",  "SIG7-B",
   "COD10",   "COD11",
   "DECT0/COD12",
   "DECT1/COD13",
   "DECT2/SIG12-A",
   "DECT3/SIG12-B",
   "DECT4/SIG13-A",
   "DECT5/SIG13-B",
   "SIG8-A",  "SIG8-B",  "SIG9-A",   "SIG9B",
   "SIG10-A", "SIG10-B", "SIG11-A",  "SIG11-B"
};
#endif /* DEBUG */

typedef enum
{
   VMMC_CON_ACTION_CREATE = 0,
   VMMC_CON_ACTION_REMOVE = 1
} VMMC_CON_ACTION_t;


/* ============================= */
/* Local function declaration    */
/* ============================= */

static
IFX_int32_t VMMC_TAPI_LL_Data_Channel_Add (
                        IFX_TAPI_LL_CH_t *pLLChannel,
                        IFX_TAPI_MAP_DATA_t const *pMap);

static
IFX_int32_t VMMC_TAPI_LL_Data_Channel_Remove (
                        IFX_TAPI_LL_CH_t *pLLChannel,
                        IFX_TAPI_MAP_DATA_t const *pMap);

static
IFX_int32_t vmmc_con_DataChannelOrderInputs(
                        struct VMMCDSP_MODULE *pModSig);

static
IFX_int32_t VMMC_TAPI_LL_ModuleConnect (
                        IFX_TAPI_LL_CH_t *pLLChannel,
                        IFX_TAPI_MAP_TYPE_t nType1,
                        unsigned char nCh2,
                        IFX_TAPI_MAP_TYPE_t nType2);

static
IFX_int32_t VMMC_TAPI_LL_ModuleDisconnect (
                        IFX_TAPI_LL_CH_t *pLLChannel,
                        IFX_TAPI_MAP_TYPE_t nType1,
                        unsigned char nCh2,
                        IFX_TAPI_MAP_TYPE_t nType2);

#ifdef TAPI_CID
static
IFX_int32_t VMMC_TAPI_LL_Data_Channel_Mute (
                        IFX_TAPI_LL_CH_t *pLLChannel,
                        IFX_boolean_t nMute);
#endif /* TAPI_CID */

static
IFX_int32_t VMMC_TAPI_LL_Data_Channel_Find_Connected_Module (
                        IFX_TAPI_LL_CH_t *pLLCh,
                        TAPI_CHANNEL **pTapiCh,
                        IFX_TAPI_MAP_TYPE_t *pModType);

static
IFX_int32_t VMMC_TAPI_LL_Module_Find_Connected_Data_Channel (
                        IFX_TAPI_LL_CH_t *pLLCh,
                        IFX_TAPI_MAP_TYPE_t pModType,
                        TAPI_CHANNEL **pTapiCh);

static
IFX_int32_t LOCAL_Module_Connect (
                        VMMC_CON_ACTION_t nAction,
                        VMMC_DEVICE *pDev,
                        IFX_uint32_t nCh1,
                        IFX_TAPI_MAP_TYPE_t nChType1,
                        IFX_uint32_t nCh2,
                        IFX_TAPI_MAP_TYPE_t nChType2);

static
IFX_int32_t LOCAL_ConnectPrepare (
                        VMMCDSP_MODULE_t* pSrc,
                        VMMCDSP_MODULE_t* pDst,
                        SIG_OUTPUT_SIDE nSide);
static
IFX_int32_t LOCAL_DisconnectPrepare (
                        VMMCDSP_MODULE_t* pSrc,
                        VMMCDSP_MODULE_t* pDst,
                        SIG_OUTPUT_SIDE nSide);

/* functions for wideband/narrowband detection and conference configuration */
static IFX_boolean_t vmmc_con_ConfMemList_ElemInList (
                        CONF_LIST_ELEMENT_t
                        *plist_elem);

static IFX_return_t  vmmc_con_ConfMemList_AppendElem (
                        CONF_LIST_ELEMENT_t
                        *plist_elem);

static IFX_return_t vmmc_con_GetInputMod (
                        VMMCDSP_MODULE_t* pMod,
                        IFX_uint8_t input_index,
                        CONF_LIST_ELEMENT_t* pModConn);

static IFX_return_t  vmmc_con_TraverseModules (
                        VMMCDSP_MODULE_t* pMod);

static CONF_LIST_ELEMENT_t* vmmc_con_ConfMemListCreate(
                        VMMC_CHANNEL *pCh,
                        VMMCDSP_MT start_module);

static IFX_return_t  vmmc_con_ConfMemListDelete (
                        void);

static IFX_int32_t   vmmc_con_ModSamplingMode (
                        SM_ACTION action,
                        VMMCDSP_MODULE_t *pMod,
                        OPMODE_SMPL      mode);

static
IFX_return_t vmmc_con_ConfMemList_MuteCtrlCoderChan (
                        IFX_operation_t mute);

/*
 * The conference member list is a single linked list of firmware modules
 * that are connected to each other. It is able to host all basic
 * firmware modules like COD, SIG, ALM and PCM. The list is built
 * on demand and deleted after use. The list is used to determine and configure
 * the necessary operation (8 or 16 kHz sampling rate, ISR) mode for all
 * conference members.
 */
/* conference member list created flag */
static IFX_boolean_t        conf_list_created = IFX_FALSE;
/* conference member list start pointer */
static CONF_LIST_ELEMENT_t* conf_listStart    = IFX_NULL;
/* conference member list end pointer */
static CONF_LIST_ELEMENT_t* conf_listEnd      = IFX_NULL;
/* number of conference members */
static IFX_uint8_t          conf_listMembers  = 0;
/* mutex protecting the conference member list */
static VMMC_OS_mutex_t mtxConfListAcc;

/* ============================= */
/* Function definitions          */
/* ============================= */

/** \defgroup VMMC_CONNECTIONS VMMC Connection Module */

/**
   Allocate data structure of the CON module in the given channel.

   \param  pCh             Pointer to the VMMC channel structure.

   \return
      - VMMC_statusOk if ok
      - VMMC_statusNoMem Memory allocation failed -> struct not created

   \remarks The channel parameter is no longer checked because the calling
   function assures correct values.
*/
IFX_int32_t VMMC_CON_Allocate_Ch_Structures (VMMC_CHANNEL *pCh)
{
   VMMC_CON_Free_Ch_Structures (pCh);

   pCh->pCON = VMMC_OS_Malloc (sizeof(VMMC_CON_t));
   if (pCh->pCON == IFX_NULL)
   {
      /* errmsg: Memory allocation failed */
      RETURN_STATUS (VMMC_statusNoMem);
   }

   memset(pCh->pCON, 0, sizeof(VMMC_CON_t));

   /* Initialise the semaphore to protect the conference member list. */
   VMMC_OS_MutexInit(&mtxConfListAcc);

   return VMMC_statusOk;
}

/**
   Free data structure of the CON module in the given channel.

   \param  pCh  Pointer to the VMMC channel structure.
*/
IFX_void_t VMMC_CON_Free_Ch_Structures (VMMC_CHANNEL *pCh)
{
   if (pCh->pCON != IFX_NULL)
   {
      VMMC_OS_Free (pCh->pCON);
      pCh->pCON = IFX_NULL;
      /* Delete the semaphore that protects the conference member list. */
      VMMC_OS_MutexDelete(&mtxConfListAcc);
   }
}

/**
   Function called during initialisation of channel, fills up
   the connection module related structure for ALM channel

   \param  pCh  Pointer to the VMMC channel structure.
*/
IFX_void_t VMMC_CON_Init_AlmCh (VMMC_CHANNEL *pCh)
{
   VMMCDSP_MODULE_t *pModAlm = &(pCh->pCON->modAlm);
   IFX_uint8_t ch = pCh->nChannel - 1;
   IFX_uint8_t i;

   pModAlm->nModType = VMMCDSP_MT_ALM;
   pModAlm->nSignal  = ECMD_IX_ALM_OUT0 + ch;
   pModAlm->pCh      = pCh;
   pModAlm->sampling_mode = VMMC_CON_SMPL_NB;

   for (i = 0; i < MAX_MODULE_SIGNAL_INPUTS; ++i)
      pModAlm->in[i].pParent = pModAlm;
}

#ifdef VMMC_FEAT_PCM
/**
   Function called during initialisation of channel, fills up
   the connection module related structure for PCM channel

   \param  pCh     Pointer to the VMMC channel structure.
   \param  pcmCh   PCM channel identifier.
*/
IFX_void_t VMMC_CON_Init_PcmCh (VMMC_CHANNEL *pCh, IFX_uint8_t pcmCh)
{
   VMMCDSP_MODULE_t *pModPcm = &(pCh->pCON->modPcm);
   IFX_uint8_t i;

   pModPcm->nModType = VMMCDSP_MT_PCM;
   pModPcm->nSignal  = ECMD_IX_PCM_OUT0 + pcmCh;
   pModPcm->pCh      = pCh;
   pModPcm->sampling_mode = VMMC_CON_SMPL_OFF;

   for (i = 0; i < MAX_MODULE_SIGNAL_INPUTS; ++i)
      pModPcm->in[i].pParent = pModPcm;
}
#endif /* VMMC_FEAT_PCM */

/**
   Function called during initialisation of channel, fills up
   the connection module related structure for coder channel

   \param  pCh  Pointer to the VMMC channel structure.
*/
IFX_void_t VMMC_CON_Init_CodCh (VMMC_CHANNEL *pCh)
{
   VMMCDSP_MODULE_t *pModCod = &(pCh->pCON->modCod);
   IFX_uint8_t ch = pCh->nChannel - 1;
   IFX_uint8_t i;
   IFX_uint8_t ch2sigarray[] =
   {
      ECMD_IX_COD_OUT0,    ECMD_IX_COD_OUT1,
      ECMD_IX_COD_OUT2,    ECMD_IX_COD_OUT3,
      ECMD_IX_COD_OUT4,    ECMD_IX_COD_OUT5,
      ECMD_IX_COD_OUT6,    ECMD_IX_COD_OUT7,
      ECMD_IX_COD_OUT8,    ECMD_IX_COD_OUT9,
      ECMD_IX_COD_OUT10,   ECMD_IX_COD_OUT11,
      ECMD_IX_COD_OUT12,   ECMD_IX_COD_OUT13
   };

   pModCod->nModType = VMMCDSP_MT_COD;
   if (ch >= sizeof(ch2sigarray)/sizeof(IFX_uint8_t))
   {
      /* This case must never happen. The number of COD channels must never
         exceed the number of COD outputs in the signalling array. */
      VMMC_ASSERT(0);
      pModCod->nSignal  = ECMD_IX_EMPTY;
   }
   else
   {
      pModCod->nSignal  = ch2sigarray[ch];
   }
   pModCod->pCh      = pCh;
   pModCod->sampling_mode = VMMC_CON_SMPL_OFF;

   for (i = 0; i < MAX_MODULE_SIGNAL_INPUTS; ++i)
      pModCod->in[i].pParent = pModCod;
}

/**
   Function called during initialisation of channel, fills up
   the connection module related structure for signaling channel

   \param  pCh  Pointer to the VMMC channel structure.
*/
IFX_void_t VMMC_CON_Init_SigCh (VMMC_CHANNEL *pCh)
{
   VMMCDSP_MODULE_t *pModSig = &(pCh->pCON->modSig);
   IFX_uint8_t ch = pCh->nChannel - 1;
   IFX_uint8_t i;
   IFX_uint8_t ch2sigarray[] =
   {
      ECMD_IX_SIG_OUTA0,   ECMD_IX_SIG_OUTA1,
      ECMD_IX_SIG_OUTA2,   ECMD_IX_SIG_OUTA3,
      ECMD_IX_SIG_OUTA4,   ECMD_IX_SIG_OUTA5,
      ECMD_IX_SIG_OUTA6,   ECMD_IX_SIG_OUTA7,
      ECMD_IX_SIG_OUTA8,   ECMD_IX_SIG_OUTA9,
      ECMD_IX_SIG_OUTA10,  ECMD_IX_SIG_OUTA11,
      ECMD_IX_SIG_OUTA12,  ECMD_IX_SIG_OUTA13
   };

   pModSig->nModType = VMMCDSP_MT_SIG;
   if (ch >= sizeof(ch2sigarray)/sizeof(IFX_uint8_t))
   {
      /* The FW reports more SIG modules than the lookup table above contains
         SIG module outputs. Check the FW signalling array defintion. */
      VMMC_ASSERT(0);
      pModSig->nSignal  = ECMD_IX_EMPTY;
      pModSig->nSignal2 = ECMD_IX_EMPTY;
   }
   else
   {
      pModSig->nSignal2 = ch2sigarray[ch];
      /* SIG output B is always SIG output A + 1 */
      pModSig->nSignal  = ch2sigarray[ch] + 1;
   }
   pModSig->pCh      = pCh;
   pModSig->sampling_mode = VMMC_CON_SMPL_NB;

   for (i = 0; i < MAX_MODULE_SIGNAL_INPUTS; ++i)
      pModSig->in[i].pParent = pModSig;
}

#ifdef DECT_SUPPORT
/**
   Function called during initialisation of channel, fills up
   the connection module related structure for the DECT channel

   \param  pCh  Pointer to the VMMC channel structure.
*/
IFX_void_t VMMC_CON_Init_DectCh (VMMC_CHANNEL *pCh)
{
   VMMCDSP_MODULE_t *pModDect = &(pCh->pCON->modDect);
   IFX_uint8_t ch = pCh->nChannel - 1;
   IFX_uint8_t i;

   pModDect->nModType = VMMCDSP_MT_DECT;
   pModDect->nSignal  = ECMD_IX_DECT_OUT0 + ch;
   pModDect->pCh      = pCh;
   pModDect->sampling_mode = VMMC_CON_SMPL_OFF;

   for (i = 0; i < MAX_MODULE_SIGNAL_INPUTS; ++i)
      pModDect->in[i].pParent = pModDect;
}
#endif /* DECT_SUPPORT */

IFX_uint8_t VMMC_CON_Get_ALM_SignalInput (VMMC_CHANNEL *pCh,
                                          IFX_uint8_t sig_index)
{
   VMMCDSP_MODULE_SIGNAL_t *pInput = &pCh->pCON->modAlm.in[sig_index];

   /* Return either the normal or the muted signal index */
   return pInput->mute ? pInput->i_mute : pInput->i;
}

#ifdef VMMC_FEAT_PCM
IFX_uint8_t VMMC_CON_Get_PCM_SignalInput (VMMC_CHANNEL *pCh,
                                          IFX_uint8_t sig_index)
{
   VMMCDSP_MODULE_SIGNAL_t *pInput = &pCh->pCON->modPcm.in[sig_index];

   /* Return either the normal or the muted signal index */
   return pInput->mute ? pInput->i_mute : pInput->i;
}
#endif /* VMMC_FEAT_PCM */

IFX_uint8_t VMMC_CON_Get_COD_SignalInput (VMMC_CHANNEL *pCh,
                                          IFX_uint8_t sig_index)
{
   VMMCDSP_MODULE_SIGNAL_t *pInput = &pCh->pCON->modCod.in[sig_index];

   /* Return either the normal or the muted signal index */
   return pInput->mute ? pInput->i_mute : pInput->i;
}

IFX_uint8_t VMMC_CON_Get_SIG_SignalInput (VMMC_CHANNEL *pCh,
                                          IFX_uint8_t sig_index)
{
   VMMCDSP_MODULE_SIGNAL_t *pInput = &pCh->pCON->modSig.in[sig_index];

   /* Return either the normal or the muted signal index */
   return pInput->mute ? pInput->i_mute : pInput->i;
}

#ifdef DECT_SUPPORT
IFX_uint8_t VMMC_CON_Get_DECT_SignalInput (VMMC_CHANNEL *pCh,
                                          IFX_uint8_t sig_index)
{
   VMMCDSP_MODULE_SIGNAL_t *pInput = &pCh->pCON->modDect.in[sig_index];

   /* Return either the normal or the muted signal index */
   return pInput->mute ? pInput->i_mute : pInput->i;
}
#endif /* DECT_SUPPORT */

#ifdef VMMC_USE_PROC
#ifdef LINUX
/**
   procfs output of the signalling array.

   \param   s        Pointer to seq_file struct.
   \return  0 on success
*/
int VMMC_proc_ConGet (struct seq_file *s)
{
   IFX_uint16_t         nDev, nCh;
   char                 sampling[] = { '-', 'N', 'W', 'A' };
   unsigned             sampling_mode;

   /* Print the connection arrays of all devices. */
   for (nDev=0; nDev < VMMC_MAX_DEVICES; nDev++)
   {
      VMMC_DEVICE *pDev;

      if (VMMC_GetDevice (nDev, &pDev) == IFX_SUCCESS)
      {
         /* Headline */
         seq_printf(s, "-- Device: #%d --\n", nDev);
         seq_printf(s, "   "
#ifdef VMMC_FEAT_PCM
                          "PCM channel        "
#endif /* VMMC_FEAT_PCM */
                          "Data Channel       "
                          "Phone channel      "
#ifdef DECT_SUPPORT
                          "DECT channel"
#endif /* DECT_SUPPORT */
                          "\n");
         seq_printf(s, "CH "
#ifdef VMMC_FEAT_PCM
                          "OUT I1 I2 I3 I4 I5 "
#endif /* VMMC_FEAT_PCM */
                          "OUT I1 I2 I3 I4 I5 "
                          "OUT I1 I2 I3 I4 I5 "
#ifdef DECT_SUPPORT
                          "OUT I1 I2 I3 I4 I5"
#endif /* DECT_SUPPORT */
                          "\n");

         /* Loop over all channels */
         for (nCh=0; nCh < VMMC_MAX_CH_NR; nCh++)
         {
            VMMC_CHANNEL *pCh = &pDev->pChannel[nCh];
            VMMC_CON_t *pChCon = pCh->pCON;

            /* Channel number */
            seq_printf(s, "%2d", nCh);

#ifdef VMMC_FEAT_PCM
            /* PCM channel */
            if (pCh->pPCM != IFX_NULL)
            {
               sampling_mode = pChCon->modPcm.sampling_mode - VMMC_CON_SMPL_OFF;
               VMMC_ASSERT(sampling_mode < sizeof(sampling));
               seq_printf(s, " %2d%c %2d %2d %2d %2d %2d",
                        pChCon->modPcm.nSignal, sampling[sampling_mode],
                        VMMC_CON_Get_PCM_SignalInput(pCh, 0),
                        VMMC_CON_Get_PCM_SignalInput(pCh, 1),
                        VMMC_CON_Get_PCM_SignalInput(pCh, 2),
                        VMMC_CON_Get_PCM_SignalInput(pCh, 3),
                        VMMC_CON_Get_PCM_SignalInput(pCh, 4));
            }
            else
               seq_printf(s, "                   ");
#endif /* VMMC_FEAT_PCM */

            /* Data channel */
            if ((pCh->pCOD != IFX_NULL) &&
                (pCh->pSIG != IFX_NULL) &&
                (pChCon->modCod.nSignal == pChCon->modSig.in[REMOTE_SIG_IN].i))
            {
               /* This is a data channel. */
               sampling_mode = pChCon->modSig.sampling_mode - VMMC_CON_SMPL_OFF;
               VMMC_ASSERT(sampling_mode < sizeof(sampling));
               seq_printf(s, " %2d%c %2d %2d %2d %2d %2d",
                        pChCon->modSig.nSignal, sampling[sampling_mode],
                        VMMC_CON_Get_SIG_SignalInput(pCh, LOCAL_SIG_IN),
                        VMMC_CON_Get_COD_SignalInput(pCh, 1),
                        VMMC_CON_Get_COD_SignalInput(pCh, 2),
                        VMMC_CON_Get_COD_SignalInput(pCh, 3),
                        VMMC_CON_Get_COD_SignalInput(pCh, 4));
            }
            else
               seq_printf(s, "                   ");

            /* ALM channel */
            if (pCh->pALM != IFX_NULL)
            {
               sampling_mode = pChCon->modAlm.sampling_mode - VMMC_CON_SMPL_OFF;
               VMMC_ASSERT(sampling_mode < sizeof(sampling));
               seq_printf(s, " %2d%c %2d %2d %2d %2d %2d",
                        pChCon->modAlm.nSignal, sampling[sampling_mode],
                        VMMC_CON_Get_ALM_SignalInput(pCh, 0),
                        VMMC_CON_Get_ALM_SignalInput(pCh, 1),
                        VMMC_CON_Get_ALM_SignalInput(pCh, 2),
                        VMMC_CON_Get_ALM_SignalInput(pCh, 3),
                        VMMC_CON_Get_ALM_SignalInput(pCh, 4));
            }
            else
               seq_printf(s, "                   ");

#ifdef DECT_SUPPORT
            /* DECT channel */
            if (pCh->pDECT != IFX_NULL)
            {
               sampling_mode = pChCon->modDect.sampling_mode - VMMC_CON_SMPL_OFF;
               VMMC_ASSERT(sampling_mode < sizeof(sampling));
               seq_printf(s, " %2d%c %2d %2d %2d %2d %2d",
                        pChCon->modDect.nSignal, sampling[sampling_mode],
                        VMMC_CON_Get_DECT_SignalInput(pCh, 0),
                        VMMC_CON_Get_DECT_SignalInput(pCh, 1),
                        VMMC_CON_Get_DECT_SignalInput(pCh, 2),
                        VMMC_CON_Get_DECT_SignalInput(pCh, 3),
                        VMMC_CON_Get_DECT_SignalInput(pCh, 4));
            }
            /* No placeholder needed for the last print. */
#endif /* DECT_SUPPORT */

            seq_printf(s, "\n");
         }
      }
   }

   return 0;
}
#endif /* LINUX */
#endif /* VMMC_USE_PROC */

/**
   Function that fills in the connection function pointers in the driver
   context structure which is passed to HL TAPI during registration.

   \param  pCON         Pointer to the CON part in the driver context struct.
*/
IFX_void_t VMMC_CON_Func_Register (IFX_TAPI_DRV_CTX_CON_t *pCON)
{
   pCON->Data_Channel_Add     = VMMC_TAPI_LL_Data_Channel_Add;
   pCON->Data_Channel_Remove  = VMMC_TAPI_LL_Data_Channel_Remove;
#ifdef TAPI_CID
   pCON->Data_Channel_Mute    = VMMC_TAPI_LL_Data_Channel_Mute;
#endif /* TAPI_CID */
   pCON->Data_Channel_Find_Connected_Module
                              = VMMC_TAPI_LL_Data_Channel_Find_Connected_Module;
   pCON->Module_Find_Connected_Data_Channel
                              = VMMC_TAPI_LL_Module_Find_Connected_Data_Channel;

   pCON->Module_Connect       = VMMC_TAPI_LL_ModuleConnect;
   pCON->Module_Disconnect    = VMMC_TAPI_LL_ModuleDisconnect;
}


/**
   Connect a data channel to an analog phone device or to PCM

   By definition a data channel consists of the COD-module and the SIG-module
   of the same TAPI connection.
   This function is used for basic channel setup (analog line to coder channel)
   and as well for conferencing.
   A conference is given, when two data channels are connected to one phone
   channel.

   Every channels configuration is only written once, when they are changed at
   the end of the signal array update. Therefore the connection is cached
   inside the driver in addition to the VoFW messages.
   ConnectPrepare may be called even if the input is already connected.

   The following steps are done:
   - connect signaling channel output to the destination (ALM or PCM) input,
     if not already done
   - Connect the destination to the data channel, if not already done. If the
     upstream input (I1) of the signalling module is already in use by another
     ALM or PCM output connect the new output to the next free upstream input
     on the coder of the same data channel instead.
   - Check if the destination is now connected to more than one data channel.
     If this is the case then connect this data channel to the conference in
     which the destination channel is involved.

   \param  pLLChannel  Pointer to the VMMC channel structure.
   \param  pMap        Handle to IFX_TAPI_MAP_DATA_t structure.

   \return
   - VMMC_statusCmdWr Writing the command failed
   - VMMC_statusNotSupported Requested action is not supported. Only types
      phone and PCM are supported.
   - VMMC_statusFuncParm Wrong parameters passed. This could be a wrong
   destination channel or this channel does not support the signaling
   module.
   - VMMC_statusWrongChMode The source channel is not is a data channel.
      It is verified by making sure SIG is getting it's input from the
      COD-module of the same channel.
   - VMMC_statusNoFreeInput No free input signal found or available
   - VMMC_statusOk if successful
*/
IFX_int32_t VMMC_TAPI_LL_Data_Channel_Add (
                        IFX_TAPI_LL_CH_t *pLLChannel,
                        IFX_TAPI_MAP_DATA_t const *pMap)
{
   VMMC_CHANNEL *pCh = (VMMC_CHANNEL *)pLLChannel,
             *pDstCh = IFX_NULL;
   VMMC_DEVICE *pDev = pCh->pParent;
   VMMCDSP_MODULE_t *pDstMod = IFX_NULL;
   VMMC_CON_t       *pThisChModules = pCh->pCON;
   IFX_int32_t ret = VMMC_statusOk;

   /* Validate channel parameter in the map struct given to us from outside. */
   if (pMap->nDstCh > VMMC_MAX_CH_NR)
   {
      /* errmsg: Destination channel number out of range */
      RETURN_STATUS (VMMC_statusDstChInvalid);
   }
   pDstCh = &pDev->pChannel[pMap->nDstCh];

   switch (pMap->nChType)
   {
#ifdef VMMC_FEAT_PCM
   case IFX_TAPI_MAP_TYPE_PCM:
      if (pDstCh->pPCM != IFX_NULL)
         pDstMod = &pDstCh->pCON->modPcm;
      break;
#endif /* VMMC_FEAT_PCM */
   case IFX_TAPI_MAP_TYPE_PHONE:
   case IFX_TAPI_MAP_TYPE_DEFAULT:
      if (pDstCh->pALM != IFX_NULL)
         pDstMod = &pDstCh->pCON->modAlm;
      break;
#ifdef DECT_SUPPORT
   case IFX_TAPI_MAP_TYPE_DECT:
      if (pDstCh->pDECT != IFX_NULL)
         pDstMod = &pDstCh->pCON->modDect;
      break;
#endif /* DECT_SUPPORT */
#ifndef VMMC_FEAT_AUTOMATIC_DATA_MAPPING
   case IFX_TAPI_MAP_TYPE_DATA:
      if ((pDstCh->pCOD != IFX_NULL) &&
          (pDstCh->pSIG != IFX_NULL) &&
          (pDstCh->pCON->modCod.nSignal ==
           pDstCh->pCON->modSig.in[REMOTE_SIG_IN].i))
      {
         /* This is a data channel; store the pointer to SIG module. */
         pDstMod = &pDstCh->pCON->modSig;
      }
      break;
#endif /* !VMMC_FEAT_AUTOMATIC_DATA_MAPPING */
   case IFX_TAPI_MAP_TYPE_CODER:
   default:
      /* errmsg: Module type specified as destination is not supported */
      RETURN_STATUS (VMMC_statusDstModuleTypeNotSupported);
   }
   if (pDstMod == IFX_NULL)
   {
      /* errmsg: The specified destination module does not exist */
      RETURN_STATUS (VMMC_statusDstModuleNotExisting);
   }
   /* Here pDstMod points to a valid module which should be connected to the
      data channel. It can either be an ALM, PCM or DECT module. */

   if ((pCh->pSIG == IFX_NULL) || (pCh->pCOD == IFX_NULL))
   {
      /* errmsg: The source channel does not contain a data channel */
      RETURN_STATUS (VMMC_statusSrcNoDataCh);
   }
   /* Verify that this is a data channel by making sure SIG is getting it's
      input from the COD-module of the same channel. */
   if (pThisChModules->modSig.in[REMOTE_SIG_IN].i !=
       pThisChModules->modCod.nSignal)
   {
      /* errmsg: The source channel is not initialised as data channel */
      RETURN_STATUS (VMMC_statusSrcWrongChMode);
   }

   /* Connect the destination to the data channel, if not already done.
      If the upstream input (I1) of the signalling module is already in
      use by another output connect the new output to the next free
      upstream input on the coder of the same data channel instead. */
   if (pThisChModules->modSig.in[LOCAL_SIG_IN].i == ECMD_IX_EMPTY)
   {
      /* connect destination output to SIG input (I1) */
      ret = LOCAL_ConnectPrepare (
               pDstMod, &pThisChModules->modSig, LOCAL_SIG_OUT);
   }
   else
   {
      /* Connect the destination output to COD input because the SIG
          input (I1) is already in use. This happens when adding more
         than one module to a data channel. But prevent connecting the
         destination to the COD when it is already connected to the SIG. */
      if (pThisChModules->modSig.in[LOCAL_SIG_IN].i != pDstMod->nSignal)
         ret = LOCAL_ConnectPrepare (
                  pDstMod, &pThisChModules->modCod, LOCAL_SIG_OUT);
   }

   if (VMMC_SUCCESS(ret))
   {
#ifndef VMMC_FEAT_AUTOMATIC_DATA_MAPPING
      /* When the destination is a SIG module which local input is already in
         use by some other module set as destination the connected COD module
         of the data channel. */
      if (pDstMod->nModType == VMMCDSP_MT_SIG)
      {
         if ((pDstMod->in[LOCAL_SIG_IN].i != ECMD_IX_EMPTY) &&
             (pDstMod->in[LOCAL_SIG_IN].i != pThisChModules->modSig.nSignal))
         {
            /* This modifies pDstMod so keep this entire part after the
               usage of the variable in the block above. */
            pDstMod = pDstMod->pInputs2->pParent;
         }
      }
#endif /* !VMMC_FEAT_AUTOMATIC_DATA_MAPPING */

      /* Connect the SIG output to the destination input. */
      ret = LOCAL_ConnectPrepare (
               &pThisChModules->modSig, pDstMod, LOCAL_SIG_OUT);
   }

#ifdef VMMC_FEAT_AUTOMATIC_DATA_MAPPING
   if (VMMC_SUCCESS(ret))
   {
      VMMCDSP_MODULE_t *tmpSigs[MAX_MODULE_SIGNAL_INPUTS] = {IFX_NULL};
      VMMCDSP_MODULE_SIGNAL_t *pInput = IFX_NULL;
      unsigned cnt = 0, i, j;

      /* Check if the Dst (ALM, PCM or DECT) is now connected to more than one
         data channel. If this is the case then connect these data channels
         so that all the data channels can talk to each other */

      /* Count to how many data channels the analog module is connected
         and store a pointer to each signalling module in a temporary list. */
      for (pInput = pDstMod->pInputs, cnt = 0;
           pInput != IFX_NULL; pInput = pInput->pNext )
      {
         if (pInput->pParent->nModType == VMMCDSP_MT_SIG)
            tmpSigs[cnt++] = pInput->pParent;
         if (pInput->pParent->nModType == VMMCDSP_MT_COD)
            tmpSigs[cnt++] = pInput->pParent->pInputs->pParent;
      }

      if (cnt > 1)
      {
         /* Conference with more than one data channel. So now connect each
            data channel to all the others that are stored in the list above.
            For this each signaling output is connected to one input on every
            coder taking part in the conference. ConnectPrepare may be called
            even if the input is already connected. */
         for (i = 0; VMMC_SUCCESS(ret) && (i <  cnt); i++)
            for (j = 0; VMMC_SUCCESS(ret) && (j < cnt); j++)
               if (i != j)
                  ret = LOCAL_ConnectPrepare (tmpSigs[i],
                           tmpSigs[j]->pInputs2->pParent, LOCAL_SIG_OUT);
         /* PS: The pointer is implicitly valid by a check done above */
      }
   }
#endif /* VMMC_FEAT_AUTOMATIC_DATA_MAPPING */

   /*
    * After the connections are prepared but before writing them to the FW
    * reevaluate the sampling mode of the new conference. If neccessary the
    * modules of the conference are configured to the other sampling rate.
    * Data channel's COD module is used as root module for conference traversal.
    */
   if (VMMC_SUCCESS(ret))
   {
      VMMC_CON_MatchConfSmplRate(pCh, VMMCDSP_MT_COD);
   }

   /*
    * Finally write down all FW configuration messages for new module connections
    */
   if (VMMC_SUCCESS(ret))
   {
      ret = VMMC_CON_ConnectConfigure (pDev);
   }

   RETURN_STATUS(ret);
}


/**
   Removes a data channel from an analog phone device

   Every channels configuration is only written once, when they are changed at
   the end of the signal array update. Therefore the connection is cached
   inside the driver in addition to the VoFW messages.

   The function performs the following steps:
   - disconnect signaling channel output from the destination (ALM or PCM) input
   - disconnect destination from either the coder channel or signaling channel
     depending on where the  output of the destination is connected to.
   - if this channel was involved in a conference, remove it from the conference
     and set the internal pointer to IFX_NULL
   - Finally all references inside the conferencing module to this channel are
     removed.

   \param  pLLChannel  Pointer to the VMMC channel structure.
   \param  pMap        Handle to IFX_TAPI_MAP_DATA_t structure.

   \return
   - VMMC_statusCmdWr Writing the command failed
   - VMMC_statusNotSupported Requested action is not supported. Only types
      phone and PCM are supported.
   - VMMC_statusFuncParm Wrong parameters passed. This could be a wrong
   destination channel or this channel does not support the signaling
   module.
   - VMMC_statusConInvalid The given connection is not valid
   - VMMC_statusOk if successful
*/
IFX_int32_t VMMC_TAPI_LL_Data_Channel_Remove (
                        IFX_TAPI_LL_CH_t *pLLChannel,
                        IFX_TAPI_MAP_DATA_t const *pMap)
{
   VMMC_CHANNEL *pCh = (VMMC_CHANNEL *)pLLChannel,
             *pDstCh = IFX_NULL;
   VMMC_DEVICE *pDev = pCh->pParent;
   IFX_int32_t ret = VMMC_statusOk;
   VMMCDSP_MODULE_t *pDstMod = IFX_NULL;
   VMMC_CON_t       *pThisChModules = pCh->pCON;
   IFX_boolean_t self_loop = IFX_FALSE;
#ifdef VMMC_FEAT_AUTOMATIC_DATA_MAPPING
   VMMCDSP_MODULE_t *tmpSigs[MAX_MODULE_SIGNAL_INPUTS] = {IFX_NULL};
   VMMCDSP_MODULE_SIGNAL_t *pInput = IFX_NULL, *pDstInput;
   IFX_int32_t cnt = 0, still_required, i;
#endif /* VMMC_FEAT_AUTOMATIC_DATA_MAPPING */

   /* Validate channel parameter in the map struct given to us from outside. */
   if (pMap->nDstCh > VMMC_MAX_CH_NR)
   {
      /* errmsg: Destination channel number out of range */
      RETURN_STATUS (VMMC_statusDstChInvalid);
   }
   pDstCh = &pDev->pChannel[pMap->nDstCh];

   switch (pMap->nChType)
   {
#ifdef VMMC_FEAT_PCM
   case IFX_TAPI_MAP_TYPE_PCM:
      if (pDstCh->pPCM != IFX_NULL)
         pDstMod = &pDstCh->pCON->modPcm;
      break;
#endif /* VMMC_FEAT_PCM */
   case IFX_TAPI_MAP_TYPE_PHONE:
   case IFX_TAPI_MAP_TYPE_DEFAULT:
      if (pDstCh->pALM != IFX_NULL)
         pDstMod = &pDstCh->pCON->modAlm;
      break;
#ifdef DECT_SUPPORT
   case IFX_TAPI_MAP_TYPE_DECT:
      if (pDstCh->pDECT != IFX_NULL)
         pDstMod = &pDstCh->pCON->modDect;
      break;
#endif /* DECT_SUPPORT */
#ifndef VMMC_FEAT_AUTOMATIC_DATA_MAPPING
   case IFX_TAPI_MAP_TYPE_DATA:
      if ((pDstCh->pCOD != IFX_NULL) &&
          (pDstCh->pSIG != IFX_NULL) &&
          (pDstCh->pCON->modCod.nSignal ==
           pDstCh->pCON->modSig.in[REMOTE_SIG_IN].i))
      {
         /* This is a data channel; store the pointer to SIG module. */
         pDstMod = &pDstCh->pCON->modSig;
         /* Set flag indicating whether the data channel loops to itself. */
         self_loop = (pDstCh == pCh) ? IFX_TRUE : IFX_FALSE;
      }
      break;
#endif /* !VMMC_FEAT_AUTOMATIC_DATA_MAPPING */
   case IFX_TAPI_MAP_TYPE_CODER:
   default:
      /* errmsg: Module type specified as destination is not supported */
      RETURN_STATUS (VMMC_statusDstModuleTypeNotSupported);
   }
   if (pDstMod == IFX_NULL)
   {
      /* errmsg: The specified destination module does not exist */
      RETURN_STATUS (VMMC_statusDstModuleNotExisting);
   }

   if ((pCh->pSIG == IFX_NULL) || (pCh->pCOD == IFX_NULL))
   {
      /* errmsg: The source channel does not contain a data channel */
      RETURN_STATUS (VMMC_statusSrcNoDataCh);
   }
   /* Verify that this is a data channel by making sure SIG is getting its
      input from the COD-module of the same channel. */
   if (pThisChModules->modSig.in[REMOTE_SIG_IN].i !=
       pThisChModules->modCod.nSignal)
   {
      /* errmsg: The source channel is not initialised as data channel */
      RETURN_STATUS (VMMC_statusSrcWrongChMode);
   }

#ifdef TAPI_CID
   /* Prevent disconnection of Dst from SIG while CID is running.
      All other connections can be disconnected. This minimises blocking.
      With this lockout we also prevent difficulties with the otherwise
      possible reroute of signals to the local signalling input below. */
   if (TAPI_Cid_IsActive(pCh->pTapiCh) &&
       (pThisChModules->modSig.in[LOCAL_SIG_IN].i == pDstMod->nSignal))
   {
      /* errmsg: Disconnect not possible while CID sequence is active */
      RETURN_STATUS (VMMC_statusCidActive);
   }
#endif /* TAPI_CID */

   /* Disconnect Src from either the COD or SIG module depending on where
      the output of the Src is connected to. */
   if ((pDstMod->nModType == VMMCDSP_MT_SIG) &&
       (pDstMod->in[LOCAL_SIG_IN].i != pThisChModules->modSig.nSignal))
   {
      /* Disconnect SIG output from destination (COD) input. */
      ret = LOCAL_DisconnectPrepare (&pThisChModules->modSig,
                                     pDstMod->pInputs2->pParent, LOCAL_SIG_OUT);
   }
   else
   {
      /* Disconnect SIG output from destination input. */
      ret = LOCAL_DisconnectPrepare (&pThisChModules->modSig,
                                     pDstMod, LOCAL_SIG_OUT);
   }

   /* Delete the reverse direction. But in case that the data channel loops
      to itself skip the deletion of the reverse direction.*/
   if (VMMC_SUCCESS(ret) && !self_loop)
   {
      /* Disconnect Dst from either the COD or SIG module depending on where
         the output of the Dst is connected to. */
      if( pThisChModules->modSig.in[LOCAL_SIG_IN].i == pDstMod->nSignal )
      {
         /* disconnect Dst (ALM or PCM) output from SIG input */
         ret = LOCAL_DisconnectPrepare (pDstMod,
                                        &pThisChModules->modSig, LOCAL_SIG_OUT);
      }
      else
      {
         /* disconnect Dst (ALM or PCM) output from COD input */
         ret = LOCAL_DisconnectPrepare (pDstMod,
                                        &pThisChModules->modCod, LOCAL_SIG_OUT);
      }
   }

#ifdef VMMC_FEAT_AUTOMATIC_DATA_MAPPING
   /* Finally the output of our signalling module may still be connected to
      the coder on other data channels because the channels have been
      connected to the Dst (ALM or PCM) module that we just disconnected.
      So now check all data channels that we are connected to because of the
      Dst and if the connection is still needed because we are still in a
      conference where another ALM or PCM module connects our data channel
      with another one. */

   /* Store a pointer to each signalling module in a data channel that
      the Dst (ALM or PCM) still connects to in a temporary list. */
   for (pInput = pDstMod->pInputs, cnt = 0;
        pInput != IFX_NULL; pInput = pInput->pNext )
   {
      if (pInput->pParent->nModType == VMMCDSP_MT_SIG)
         tmpSigs[cnt++] = pInput->pParent;
      if (pInput->pParent->nModType == VMMCDSP_MT_COD)
         tmpSigs[cnt++] = pInput->pParent->pInputs->pParent;
   }
   /* Loop over the list just built */
   for (i = 0, still_required = 0;
        VMMC_SUCCESS(ret) && i <  cnt && !still_required; i++)
   {
      /* Loop over all modules connecting to the data channel */
      for (pInput = tmpSigs[i]->pInputs;
           pInput != IFX_NULL; pInput = pInput->pNext)
      {
         /* Skip COD modules connecting to the data channel */
         if (pInput->pParent->nModType == VMMCDSP_MT_COD)
            continue;
         /* For remaining modules check if any of the inputs connecting to
            the output is the signalling module we started with. If so then
            mark the connection as still required. */
         for (pDstInput = pInput->pParent->pInputs;
              pDstInput != IFX_NULL; pDstInput = pDstInput->pNext)
            if (pDstInput->pParent == &pThisChModules->modSig ||
                pDstInput->pParent == &pThisChModules->modCod   )
            {
               still_required = 1;
               break;
            }
      }
      /* If no longer required disconnect the data channels */
      if (! still_required)
      {
         ret = LOCAL_DisconnectPrepare (&pThisChModules->modSig,
                                        tmpSigs[i]->pInputs2->pParent,
                                        LOCAL_SIG_OUT);
         if (VMMC_SUCCESS(ret))
            ret = LOCAL_DisconnectPrepare (tmpSigs[i],
                                           &pThisChModules->modCod,
                                           LOCAL_SIG_OUT);
      }
   }
#endif /* VMMC_FEAT_AUTOMATIC_DATA_MAPPING */

   /* Now the local input on the SIG within the data channel may be empty
      so reroute a signal from an input of the COD of the data channel if
      existing. */
   vmmc_con_DataChannelOrderInputs(&pThisChModules->modSig);
   if ((pDstMod->nModType == VMMCDSP_MT_SIG) && !self_loop)
      vmmc_con_DataChannelOrderInputs(pDstMod);

   /* write the configuration to the firmware */
   if (VMMC_SUCCESS(ret))
   {
      ret = VMMC_CON_ConnectConfigure (pDev);
   }

   /*
    * After FW modules are disconnected match the sampling modes of the
    * modules in each of the remaining two conferences.
    */
   VMMC_CON_MatchConfSmplRate(pCh, VMMCDSP_MT_COD);
   if (!self_loop)
   {
      VMMC_CON_MatchConfSmplRate(pDstCh, pDstMod->nModType);
   }

   RETURN_STATUS(ret);
}


/**
   Order the inputs of a data channel

   After removal of a module from a data channel the local input of the
   embedded SIG module may be empty. So this functions reroutes a signal
   from an input of the COD module of the data channel when possible.

   When compiled for automatic data channel mapping a signal originating
   from a SIG module is never rerouted.
   When compiled without automatic data channel mapping the following rules
   apply: Preferrably reroute a signal originating from a non SIG module.
   If only SIG modules are connected use the last one these instead. But
   never reroute the data channel internal connection coming from the remote
   output of the SIG module.

   \param  pModSig      Pointer to the SIG module struct of a data channel.

   \return
   - VMMC_statusOk if successful
   - error codes from LOCAL_DisconnectPrepare()
   - error codes from LOCAL_ConnectPrepare()
*/
static
IFX_int32_t vmmc_con_DataChannelOrderInputs(
                        struct VMMCDSP_MODULE *pModSig)
{
   VMMCDSP_MODULE_t *pModCod,
                    *pTmpMod;
#ifndef VMMC_FEAT_AUTOMATIC_DATA_MAPPING
   VMMCDSP_MODULE_t *p2ndSigMod = IFX_NULL;
#endif /* ! VMMC_FEAT_AUTOMATIC_DATA_MAPPING */
   IFX_int32_t i;
   IFX_int32_t ret = VMMC_statusOk;

   /* If the local input of the SIG is not empty nothing needs to be done. */
   if (pModSig->in[LOCAL_SIG_IN].i != ECMD_IX_EMPTY)
      return ret;

   pModCod = pModSig->pInputs2->pParent;

   /* Check all inputs of the COD module. */
   for (i = 0; i <  MAX_MODULE_SIGNAL_INPUTS; i++)
   {
      pTmpMod = pModCod->in[i].pOut;

      if (pTmpMod == IFX_NULL)
         continue;

      if (pTmpMod->nModType != VMMCDSP_MT_SIG)
         break;

#ifndef VMMC_FEAT_AUTOMATIC_DATA_MAPPING
      /* Here only inputs from SIG modules are left. */

      /* Skip the data channel internal connection which is coming from
        the remote output of the SIG module. */
      if (pTmpMod->nSignal2 == pModCod->in[i].i)
         continue;

      /* Remember the last SIG module. */
      p2ndSigMod = pTmpMod;
#endif /* ! VMMC_FEAT_AUTOMATIC_DATA_MAPPING */
   }

#ifndef VMMC_FEAT_AUTOMATIC_DATA_MAPPING
   /* If no input from an non SIG module is found choose a SIG module. */
   if (pTmpMod == IFX_NULL)
   {
      pTmpMod = p2ndSigMod;
   }
#endif /* ! VMMC_FEAT_AUTOMATIC_DATA_MAPPING */

   if (pTmpMod != IFX_NULL)
   {
      /* disconnect the found output from COD input */
      ret = LOCAL_DisconnectPrepare (pTmpMod, pModCod, LOCAL_SIG_OUT);
      if (VMMC_SUCCESS(ret))
         /* connect the found output to SIG input (I1) */
         ret = LOCAL_ConnectPrepare (pTmpMod, pModSig, LOCAL_SIG_OUT);
   }

   return ret;
}


/**
   Connect any two of the following modules: PCM, ALM or DECT.

   \param  pLLChannel   Pointer to the VMMC channel structure.
   \param  nType1       Module type in the first channel.
   \param  nCh2         Channel number of second channel
   \param  nType2       Module type in the second channel

   \return
   - VMMC_statusCmdWr Writing the command failed
   - VMMC_statusNotSupported Requested action is not supported. Only types
      phone and PCM are supported.
   - VMMC_statusFuncParm Wrong parameters passed. This could be a wrong
   destination channel or this channel does not support the signaling
   module.
   - VMMC_statusConInvalid The given connection is not valid
   - VMMC_statusOk if successful
*/
IFX_int32_t VMMC_TAPI_LL_ModuleConnect (
                        IFX_TAPI_LL_CH_t *pLLChannel,
                        IFX_TAPI_MAP_TYPE_t nType1,
                        unsigned char nCh2,
                        IFX_TAPI_MAP_TYPE_t nType2)
{
   VMMC_CHANNEL *pCh = (VMMC_CHANNEL *)pLLChannel;

   return LOCAL_Module_Connect (VMMC_CON_ACTION_CREATE,
                                pCh->pParent,
                                pCh->nChannel - 1, nType1,
                                nCh2,              nType2);
}


/**
   Disconnect any two of the following modules: PCM, ALM or DECT.

   \param  pLLChannel   Pointer to the VMMC channel structure.
   \param  nType1       Module type in the first channel.
   \param  nCh2         Channel number of second channel
   \param  nType2       Module type in the second channel

   \return
   - VMMC_statusCmdWr Writing the command failed
   - VMMC_statusNotSupported Requested action is not supported. Only types
      phone and PCM are supported.
   - VMMC_statusFuncParm Wrong parameters passed. This could be a wrong
   destination channel or this channel does not support the signaling
   module.
   - VMMC_statusConInvalid The given connection is not valid
   - VMMC_statusOk if successful
*/
IFX_int32_t VMMC_TAPI_LL_ModuleDisconnect (
                        IFX_TAPI_LL_CH_t *pLLChannel,
                        IFX_TAPI_MAP_TYPE_t nType1,
                        unsigned char nCh2,
                        IFX_TAPI_MAP_TYPE_t nType2)
{
   VMMC_CHANNEL *pCh = (VMMC_CHANNEL *)pLLChannel;

   return LOCAL_Module_Connect (VMMC_CON_ACTION_REMOVE,
                                pCh->pParent,
                                pCh->nChannel - 1, nType1,
                                nCh2,              nType2);
}


/**
   Create or remove a symetric connection between two local modules
   (PCM, ALM, DECT) on the same device.

   \param  nAction      Action to be performed VMMC_CON_ACTION_t
   \param  pDev         Pointer to the VMMC device structure
   \param  nCh1         Resource number of the first module
   \param  nChType1     Which module type to use from channel 1
   \param  nCh2         Resource number of the second module
   \param  nChType2     Which module type to use from channel 2

   \return
   - VMMC_statusOk if successful
   - error code if not ok
*/
static
IFX_int32_t LOCAL_Module_Connect (
                        VMMC_CON_ACTION_t nAction,
                        VMMC_DEVICE *pDev,
                        IFX_uint32_t nCh1,
                        IFX_TAPI_MAP_TYPE_t nChType1,
                        IFX_uint32_t nCh2,
                        IFX_TAPI_MAP_TYPE_t nChType2)
{
   VMMC_CHANNEL      *pCh     = IFX_NULL,
                     *pCh2    = IFX_NULL;
   VMMCDSP_MODULE_t  *pMod1   = IFX_NULL,
                     *pMod2   = IFX_NULL;
   IFX_int32_t       ret      = VMMC_statusOk;

   if (nCh1 >= VMMC_MAX_CH_NR)
   {
      /* errmsg: Resource not valid. Channel number out of range  */
      return VMMC_statusInvalCh;
   }
   if (nCh2 >= VMMC_MAX_CH_NR)
   {
      /* errmsg: Destination channel number out of range */
      return VMMC_statusDstChInvalid;
   }

   /* First channel (also used for RETURN_STATUS) */
   pCh = &pDev->pChannel[nCh1];

   /* get pointers to the signal struct of
      the selected modules in the channels */
   switch (nChType1)
   {
#ifdef VMMC_FEAT_PCM
   case IFX_TAPI_MAP_TYPE_PCM:
      if (pCh->pPCM != IFX_NULL)
         pMod1 = &pCh->pCON->modPcm;
      break;
#endif /* VMMC_FEAT_PCM */
   case IFX_TAPI_MAP_TYPE_PHONE:
   case IFX_TAPI_MAP_TYPE_DEFAULT:
      if (pCh->pALM != IFX_NULL)
         pMod1 = &pCh->pCON->modAlm;
      break;
#ifdef DECT_SUPPORT
   case IFX_TAPI_MAP_TYPE_DECT:
      if (pCh->pDECT != IFX_NULL)
         pMod1 = &pCh->pCON->modDect;
      break;
#endif /* DECT_SUPPORT */
   case IFX_TAPI_MAP_TYPE_CODER:
   default:
      /* errmsg: Module type specified as source is not supported */
      RETURN_STATUS (VMMC_statusSrcModuleTypeNotSupported);
   }

   /* Second channel */
   pCh2 = &pDev->pChannel[nCh2];

   switch (nChType2)
   {
#ifdef VMMC_FEAT_PCM
   case IFX_TAPI_MAP_TYPE_PCM:
      if (pCh2->pPCM != IFX_NULL)
         pMod2 = &pCh2->pCON->modPcm;
      break;
#endif /* VMMC_FEAT_PCM */
   case IFX_TAPI_MAP_TYPE_PHONE:
   case IFX_TAPI_MAP_TYPE_DEFAULT:
      if (pCh2->pALM != IFX_NULL)
         pMod2 = &pCh2->pCON->modAlm;
      break;
#ifdef DECT_SUPPORT
   case IFX_TAPI_MAP_TYPE_DECT:
      if (pCh2->pDECT != IFX_NULL)
         pMod2 = &pCh2->pCON->modDect;
      break;
#endif /* DECT_SUPPORT */
   case IFX_TAPI_MAP_TYPE_CODER:
   default:
      /* errmsg: Module type specified as destination is not supported */
      RETURN_STATUS (VMMC_statusDstModuleTypeNotSupported);
   }

   if (pMod1 == IFX_NULL)
   {
      /* errmsg: The specified source module does not exist */
      RETURN_STATUS (VMMC_statusSrcModuleNotExisting);
   }
   if (pMod2 == IFX_NULL)
   {
      /* errmsg: The specified destination module does not exist */
      RETURN_STATUS (VMMC_statusDstModuleNotExisting);
   }

   /* now pMod1 and pMod2 point to the modules to be connected/disconnected */

   if (nAction == VMMC_CON_ACTION_CREATE)
   {
      /* connect the forward direction */
      ret = LOCAL_ConnectPrepare (pMod1, pMod2, LOCAL_SIG_OUT);
      if (VMMC_SUCCESS(ret))
      {
         /* connect the reverse direction */
         ret = LOCAL_ConnectPrepare (pMod2, pMod1, LOCAL_SIG_OUT);
      }
   }
   if (nAction == VMMC_CON_ACTION_REMOVE)
   {
      /* disconnect the forward direction */
      ret = LOCAL_DisconnectPrepare (pMod1, pMod2, LOCAL_SIG_OUT);
      if (VMMC_SUCCESS(ret))
      {
         /* disconnect the reverse direction */
         ret = LOCAL_DisconnectPrepare (pMod2, pMod1, LOCAL_SIG_OUT);
      }
   }


   if (nAction == VMMC_CON_ACTION_CREATE)
   {
      /* After the connections are prepared but before writing them to the FW
       * reevaluate the sampling mode of the new conference. If neccessary the
       * modules of the conference are configured to the other sampling rate. */
      VMMC_CON_MatchConfSmplRate(pCh, pMod1->nModType);
   }

   /* finally write down FW commands to configure new module interconnections
      and calculated operation modes */
   if (VMMC_SUCCESS(ret))
      ret = VMMC_CON_ConnectConfigure (pDev);

   if (nAction == VMMC_CON_ACTION_REMOVE)
   {
      /* After FW modules are disconnected match the sampling modes of the
       * modules in each of the remaining two conferences. */
      VMMC_CON_MatchConfSmplRate(pCh,  pMod1->nModType);
      VMMC_CON_MatchConfSmplRate(pCh2, pMod2->nModType);
   }

   RETURN_STATUS(ret);
}


#ifdef TAPI_CID
/**
   Mute/Unmute all connections to modules which are attached to a data channel
   that is currently sending CID2 and should not listen to the CID.

   This function is used to temporarily mute connections in the signalling
   array. This function finds all connections which need to be muted and
   mutes/unmutes them so that modules taking part in a conference will not
   hear the CID signal which is not intended from them.

   \param  pLLChannel   Handle to IFX_TAPI_LL_CH_t structure.
   \param  nMute        IFX_TRUE: mute / IFX_FALSE: unmute.

   \return
   - VMMC_statusCmdWr Writing the command failed
   - VMMC_statusNotSupported Requested action is not supported. Only types
      phone and PCM are supported.
   - VMMC_statusFuncParm Wrong parameters passed. This could be a wrong
   destination channel or this channel does not support the signaling
   module.
   - VMMC_statusConInvalid The given connection is not valid
   - VMMC_statusOk if successful
*/
IFX_int32_t VMMC_TAPI_LL_Data_Channel_Mute (
                        IFX_TAPI_LL_CH_t *pLLChannel,
                        IFX_boolean_t nMute)
{
   VMMC_CHANNEL   *pCh = (VMMC_CHANNEL *)pLLChannel;
   VMMC_DEVICE   *pDev = pCh->pParent;
   VMMCDSP_MODULE_t *pSigMod = IFX_NULL,
                    *pAnaMod = IFX_NULL;
   VMMCDSP_MODULE_SIGNAL_t *pTmpSig;
   IFX_int8_t mute = nMute ? 1 : -1;
   unsigned i;
   IFX_int32_t ret;

   pSigMod = &pCh->pCON->modSig;
   /* find the module that is the one providing the input to SIG */
   /*lint -e{722} */
   for (pTmpSig = pSigMod->pInputs;
        pTmpSig != IFX_NULL && (pAnaMod = pTmpSig->pParent,
        pSigMod->in[LOCAL_SIG_IN].i != pAnaMod->nSignal);
        pTmpSig = pTmpSig->pNext, pAnaMod = IFX_NULL);
   if ( pAnaMod == IFX_NULL )
   {
      /* errmsg: No module is connected to the data channel */
      RETURN_STATUS (VMMC_statusSrcModuleNotExisting);
   }

   /* 1. Flag the outputs on SIG and Analog module as muted so that all
         connections created from now on are also created in muted mode. */
   pSigMod->nMute = /*lint --e(821)*/
   pSigMod->nMute2 = pAnaMod->nMute = nMute ? 1 : 0;
   pSigMod->modified = pAnaMod->modified = IFX_TRUE;

   /* 2. Find all inputs getting the local output of SIG and set them to muted
         mode but not the Analog module. Set then the  muted inputs to
         receive the input of the SIG module. */
   for (pTmpSig = pSigMod->pInputs;
        pTmpSig != IFX_NULL;
        pTmpSig = pTmpSig->pNext)
   {
      if (pTmpSig->pParent != pAnaMod)
      {
         pTmpSig->mute = (IFX_uint8_t)(pTmpSig->mute + mute);
         if (pTmpSig->mute == 1 && pTmpSig->pParent->nMute == 0)
            pTmpSig->i_mute = pSigMod->in[REMOTE_SIG_IN].i;
         pTmpSig->pParent->modified = IFX_TRUE;
      }
   }

   /* 3. Mute the remote input of the SIG module */
   pSigMod->in[REMOTE_SIG_IN].mute += /*lint --e(732)*/ mute;

   /* 4. Mute all inputs of the Analog module except the one connected to
         the SIG module. */
   for (i = 0; i < MAX_MODULE_SIGNAL_INPUTS; i++)
   {
      if (pAnaMod->in[i].i != pSigMod->nSignal)
      {
         pAnaMod->in[i].mute += /*lint --e(732)*/ mute;
         pAnaMod->in[i].i_mute = ECMD_IX_EMPTY;
      }
   }

   /* 5. Mute all inputs connected to the output of the Analog module but
         not the SIG module itself */
   for (pTmpSig = pAnaMod->pInputs;
        pTmpSig != IFX_NULL;
        pTmpSig = pTmpSig->pNext)
   {
      if (pTmpSig->pParent != pSigMod)
      {
         pTmpSig->mute += /*lint --e(732) */ mute;
         pTmpSig->pParent->modified = IFX_TRUE;
      }
   }

   /* 6. Mute all inputs that receive their input from SIG out remote */
   for (pTmpSig = pSigMod->pInputs2;
        pTmpSig != IFX_NULL;
        pTmpSig = pTmpSig->pNext)
   {
      pTmpSig->mute += /*lint --e(732)*/ mute;
      pTmpSig->pParent->modified = IFX_TRUE;
   }

   /* 7. For more comfort in the unmute case write the muted signal index on
         now only once muted inputs of analog modules that have been muted
         multiple times before. */
   for (i = 0; i < MAX_MODULE_SIGNAL_INPUTS; i++)
   {
      if ((pAnaMod->in[i].i != ECMD_IX_EMPTY) &&
          (pAnaMod->in[i].mute == 1) &&
          (pAnaMod->in[i].i_mute == ECMD_IX_EMPTY) &&
          (pAnaMod->in[i].pOut->nModType == VMMCDSP_MT_SIG) &&
          (pAnaMod->in[i].pOut->in[REMOTE_SIG_IN].mute != 0))
      {
         pAnaMod->in[i].i_mute = pAnaMod->in[i].pOut->in[REMOTE_SIG_IN].i;
      }
   }

   /* write the changed configuration to the chip */
   ret = VMMC_CON_ConnectConfigure (pDev);

   RETURN_STATUS(ret);
}
#endif /* TAPI_CID */


/**
   Find the module which provides it's input to the data channel main input
   (on the SIG module) and gets an input from this data channel.

   \param  pLLChannel   Pointer to the VMMC channel structure.
   \param  pTapiCh      Returns pointer to the TAPI channel the found module
                        belongs to or IFX_NULL if no module is connected.
   \param  pModType     Returns of which type the found module is.
                        If pTapiCh is IFX_NULL this is undefined.
   \return
   - VMMC_statusOk if successful
   - VMMC_statusParam if called on uninitialised channel
*/
IFX_int32_t VMMC_TAPI_LL_Data_Channel_Find_Connected_Module (
                        IFX_TAPI_LL_CH_t *pLLChannel,
                        TAPI_CHANNEL **pTapiCh,
                        IFX_TAPI_MAP_TYPE_t *pModType)
{
   VMMC_CHANNEL *pCh = (VMMC_CHANNEL *)pLLChannel;
   VMMCDSP_MODULE_t *pSigMod = IFX_NULL,
                    *pConMod = IFX_NULL;
   VMMCDSP_MODULE_SIGNAL_t *pTmpSig;

   /* start with a return value of fail */
   *pTapiCh = IFX_NULL;
   /* if nothing is found return something defined but meaningless */
   *pModType = IFX_TAPI_MAP_TYPE_DEFAULT;

   /* abort if called on an uninitialised channel */
   if (pCh->pCON == IFX_NULL)
   {
      RETURN_STATUS (VMMC_statusParam);
   }

   pSigMod = &pCh->pCON->modSig;

   /* find the module that is the one providing the input to the SIG module */
   /*lint -e{722} */
   for (pTmpSig = pSigMod->pInputs, pConMod = IFX_NULL;
        pTmpSig != IFX_NULL && (pConMod = pTmpSig->pParent,
        pSigMod->in[LOCAL_SIG_IN].i != pConMod->nSignal);
        pTmpSig = pTmpSig->pNext, pConMod = IFX_NULL);

   if ( pConMod != IFX_NULL )
   {
      /* check the reverse direction */
      /*lint -e{722} */
      for (pTmpSig = pConMod->pInputs;
           pTmpSig != IFX_NULL && (pTmpSig->pParent != pSigMod);
           pTmpSig = pTmpSig->pNext);

      if (pTmpSig != IFX_NULL)
      {
         /* found a match */
         *pTapiCh = pConMod->pCh->pTapiCh;
         switch (pConMod->nModType)
         {
         case VMMCDSP_MT_ALM:
            *pModType = IFX_TAPI_MAP_TYPE_PHONE;
            break;
         case VMMCDSP_MT_PCM:
            *pModType = IFX_TAPI_MAP_TYPE_PCM;
            break;
         case VMMCDSP_MT_DECT:
            *pModType = IFX_TAPI_MAP_TYPE_DECT;
            break;
         default:
            /* this is an error case - return with the default set above */
            break;
         }
      }
   }

   return VMMC_statusOk;
}


/**
   Find the data channel which gets it's main input from a given module
   and which provides it's input to the given module.

    There can be more than one data channel connected to a module. So the
    function acts as an iterator with pTapiCh as input and output parameter.

   \param  pLLCh        Pointer to Low-level channel structure.
   \param  nModType     Type of the module where to start search.
   \param  pTapiCh      On calling specifies which was the last channel found
                        by this function. For the first call use IFX_NULL.
                        For iteration calls use the last output.
                        Returns pointer to the TAPI channel the found module
                        belongs to or IFX_NULL if no module is connected.

   \return
   - VMMC_statusOk if successful
   - VMMC_statusSrcModuleNotExisting
   - VMMC_statusInvalCh
*/
IFX_int32_t VMMC_TAPI_LL_Module_Find_Connected_Data_Channel (
                        IFX_TAPI_LL_CH_t *pLLCh,
                        IFX_TAPI_MAP_TYPE_t nModType,
                        TAPI_CHANNEL **pTapiCh)
{
   VMMC_CHANNEL *pCh = (VMMC_CHANNEL *)pLLCh;
   VMMCDSP_MODULE_t *pSigMod = IFX_NULL,
                    *pConMod = IFX_NULL;
   VMMCDSP_MODULE_SIGNAL_t *pTmpSig;
   TAPI_CHANNEL *pLastTapiCh;
   IFX_boolean_t bSkip;

   switch (nModType)
   {
   default:
   case IFX_TAPI_MAP_TYPE_PHONE:
      pConMod = &pCh->pCON->modAlm;
      break;
#ifdef VMMC_FEAT_PCM
   case IFX_TAPI_MAP_TYPE_PCM:
      pConMod = &pCh->pCON->modPcm;
      break;
#endif /* VMMC_FEAT_PCM */
#ifdef DECT_SUPPORT
   case IFX_TAPI_MAP_TYPE_DECT:
      pConMod = &pCh->pCON->modDect;
      break;
#endif /* DECT_SUPPORT */
   }

   /* Make sure that we do not start with a non existing module. */
   if (pConMod == IFX_NULL /*lint --e(774) */)
   {
      /* The specified module does not exist on the specified channel. */
      VMMC_ASSERT(pConMod != IFX_NULL);
      *pTapiCh = IFX_NULL;
      /* errmsg: The specified source module does not exist */
      RETURN_STATUS (VMMC_statusSrcModuleNotExisting);
   }

   /* save the input and determine the iteration abort criteria */
   pLastTapiCh = *pTapiCh;
   bSkip = pLastTapiCh != IFX_NULL ? IFX_TRUE : IFX_FALSE;

   /* start with a return value of fail */
   *pTapiCh = IFX_NULL;

   /* find a SIG module that gets it local input from the given module */
   for (pTmpSig = pConMod->pInputs, pSigMod = IFX_NULL;
        pTmpSig != IFX_NULL;
        pTmpSig = pTmpSig->pNext, pSigMod = IFX_NULL)
   {
      pSigMod = pTmpSig->pParent;

      if (pSigMod->nModType == VMMCDSP_MT_SIG &&
          pSigMod->in[LOCAL_SIG_IN].i == pConMod->nSignal)
      {
         if (bSkip == IFX_FALSE)
         {
            break;
         }
         else
         {
            if (pSigMod->pCh->pTapiCh == pLastTapiCh)
               bSkip = IFX_FALSE;
         }
      }
   }

   if ( pSigMod != IFX_NULL )
   {
      /* TAPI allows only symetric connections so do not check for the
         reverse direction here. */

      /* set the return value */
      *pTapiCh = pSigMod->pCh->pTapiCh;
   }

   return VMMC_statusOk;
}


#ifdef VMMC_FEAT_RTCP_XR
/**
   If there is a one-to-one connection between a given module and a
   data channel returns the channel of the COD module.

   Any connections with other than a data channel or more than exactly only
   one data channel will return IFX_NULL.

   \param  pCh          Pointer to the VMMC channel structure.
   \param  mod          Module type where to start (PCM/ALM).

   \return
   Pointer to a VMMC channel with the COD module.
   IFX_NULL if an error occured or no COD was found.
*/
VMMC_CHANNEL *VMMC_CON_SingleDataChannelCodFind (
                        VMMC_CHANNEL *pCh,
                        VMMCDSP_MT mod)
{
   VMMCDSP_MODULE_t  *pMod = IFX_NULL,
                     *pSigMod = IFX_NULL,
                     *pCodMod = IFX_NULL;

   switch (mod)
   {
   case VMMCDSP_MT_ALM:
      pMod = &pCh->pCON->modAlm;
      break;
#ifdef VMMC_FEAT_PCM
   case VMMCDSP_MT_PCM:
      pMod = &pCh->pCON->modPcm;
      break;
#endif /* VMMC_FEAT_PCM */
   default:
      /* internal error: not an supported module type */
      VMMC_ASSERT(0);
      return IFX_NULL;
   }
   /* It is a coding error if the specified module does exist on the
      specified channel. */
   VMMC_ASSERT(pMod != IFX_NULL);

   /* TAPI allows only symetric connections so checking the output is
      sufficient. A loop over the inputs is not needed. */

   /* 1) If no module or more than one module is connected to the output
         this is not a one-to-one connection. */
   if ((pMod->pInputs == IFX_NULL) || pMod->pInputs->pNext != IFX_NULL)
   {
      return IFX_NULL;
   }

   /* 2) Assume that the module is connected to a data channel and check
         this in the next tests below. */

   /* If the module connected to the output is not the local side of
      a SIG module this cannot be a data channel. */
   pSigMod = pMod->pInputs->pParent;
   if ((pSigMod->nModType != VMMCDSP_MT_SIG) ||
       (pSigMod->in[LOCAL_SIG_IN].i != pMod->nSignal))
   {
      return IFX_NULL;
   }

   /* Verify that this is a data channel by making sure SIG is getting it's
      input from the COD-module of the same channel. */
   pCodMod = &(pSigMod->pCh->pCON->modCod);
   if (pSigMod->in[REMOTE_SIG_IN].i != pCodMod->nSignal)
   {
      return IFX_NULL;
   }

   /* 3) If more than one module is connected to the output of the SIG
         this is not a one-to-one connection. */
   if (pSigMod->pInputs->pNext != IFX_NULL)
   {
      return IFX_NULL;
   }

   return pCodMod->pCh;
}
#endif /* VMMC_FEAT_RTCP_XR */


/**
   Add a signal connection to the internal connection structures.
   Call Con_ConnectConfigure() to write all connections to the firmware.

   \param  pSrc         Pointer to the source module which provides the
                        output signal.
   \param  pDst         Pointer to the destination module where the output
                        is connected to.
   \param  nSide        Specifies which output to use when the source is a
                        signaling module, don't care for other module types
                        (set to 0).
                        If set to LOCAL_SIG_OUT then the output towards the
                        local side is used and with REMOTE_SIG_OUT the output
                        towards the remote side.

   \return
   - VMMC_statusOk if successful
   - VMMC_statusSigInputInUse
   - VMMC_statusNoFreeInput

   \remarks
   This function only connects two given modules if they are not already
   connected. The destination is set to modified if all was successful.
   The connection inherits a mute attribute from the module output that
   serves as the signal source for the connection. So when a connection
   is created on an output set to mute it will itself carry the mute
   attribute.
   As signalling modules have two sides (local/remote) to which other
   modules can be connected the side has to be determined first.
   When the signalling modules acts as an input rules are used to determine
   to which input side a module is connected:
   Local modules like ALM and PCM (not always) support auto suppression and
   get the event playout. They are connected to In 1.
   Remote modules like coder and PCM in case of advanced with DSP are
   connected to In 2.
   When the signalling module acts as an output nSignal is used to select
   which side of the signalling module is used for the connection. The value
   LOCAL_SIG_OUT is used for all analog channels. The value REMOTE_SIG_OUT
   is used for the coder only to connect to the Sig-OutA output.

   \verbatim
                          nSignal2
                          pInputs2
   Signalling module:      |----------------|
                       <-  | Out A     In 1 |  <-
                           |  upstream (tx) |
          remote <==       |  -  -  -  -  - |       ==> local
                           | downstream (rx)|
                       ->  | In 2     Out B |  ->
                           |----------------|
                                       nSignal
                                       pInputs
   \endverbatim
*/
static
IFX_int32_t LOCAL_ConnectPrepare (
                        VMMCDSP_MODULE_t* pSrc,
                        VMMCDSP_MODULE_t* pDst,
                        SIG_OUTPUT_SIDE nSide)
{
   int i, in = -1;
   VMMCDSP_MODULE_SIGNAL_t **pInput;
   IFX_uint8_t sig;
   IFX_uint8_t mute;
   VMMCDSP_MODULE_SIGNAL_t *pDstIn = IFX_NULL;

   /* get signal number and pointer to the output of the source module */
   if( nSide == REMOTE_SIG_OUT && pSrc->nModType == VMMCDSP_MT_SIG )
   {
      /* SIG module: use the remote output (Out A) of the signalling module */
      pInput = &pSrc->pInputs2;
      sig    = pSrc->nSignal2;
      mute   = pSrc->nMute2;
   }
   else
   {
      /* All modules: use the standard (local) output of given module */
      pInput = &pSrc->pInputs;
      sig    = pSrc->nSignal;
      mute   = pSrc->nMute;
   }

   /* *pInput now points to the first input signal of the list of input signals
      connected to this output. sig contains the number of the signal in the
      signal array */

   /* return now if the output is already connected to one of the inputs */
   for (i = 0; i < MAX_MODULE_SIGNAL_INPUTS; i++)
   {
      if (pDst->in[i].i == sig)
      {
         return VMMC_statusOk;
      }
   }

   /* find the index of a free input on the destination module */
   switch(pDst->nModType)
   {
      case VMMCDSP_MT_SIG:
         /* default input on signaling module is input 1 */
         in = LOCAL_SIG_IN;
         /* exception 1: coder connects always to input 2 */
         if (pSrc->nModType == VMMCDSP_MT_COD)
         {
            in = REMOTE_SIG_IN;
         }
         /* exception 2 for PCM: in advanced PCM it does not act as ALM, but as
            coder. This case is selected by using REMOTE_SIG_OUT on the PCM
            as it is connected to the SIG. This cannot be done when connecting
            PCM and SIG by user configuration. */
         if (pSrc->nModType == VMMCDSP_MT_PCM && nSide == REMOTE_SIG_OUT)
         {
            in = REMOTE_SIG_IN;
         }

         /* finally verify that the input selected is currently not in use */
         if (pDst->in[in].i != ECMD_IX_EMPTY)
         {
            /* errmsg: Input on SIG module already in use */
            return VMMC_statusSigInputInUse;
         }
         pDstIn = &(pDst->in[in]);
         break;

      default:
         /* find free input on the destination module (COD, ALM, PCM or DECT) */
         for (i = 0; i < MAX_MODULE_SIGNAL_INPUTS; i++)
         {
            if (pDst->in[i].i == ECMD_IX_EMPTY)
            {
               in = i;
               pDstIn = &(pDst->in[i]);
               break;
            }
         }
         break;
   }   /* switch(pDst->nModType) */

#ifdef DEBUG
   TRACE(VMMC, DBG_LEVEL_LOW,
         ("adding Signal %s-OUT to %s/input:%d %s\n",
          signalName[sig], signalName[pDst->nSignal], in, mute ? "muted" : ""));
#endif /* DEBUG */

   if( (pDstIn == IFX_NULL) || (in == -1) )
   {
      /* placed after the trace to get details on the modules (debug only) */
      TRACE(VMMC, DBG_LEVEL_NORMAL,
         ("No free input on destination module - stop\n"));
      /* errmsg: No free input on destination module */
      return VMMC_statusNoFreeInput;
   }

   /* Connect output to the input */
   if( (*pDstIn).i != sig )
   {
      pDst->modified = IFX_TRUE;
      (*pDstIn).i    = sig;
      (*pDstIn).mute += mute;
   }
   /* Set the pointer showing which output connects to this input */
   (*pDstIn).pOut  = pSrc;
   /* Add the input to the linked list of all inputs attached to one output.
      Always add the element to the head of the list.*/
   (*pDstIn).pNext = *pInput;
   *pInput = pDstIn;

   return VMMC_statusOk;
}


/**
   Removes a signal connection to the internal connection structures.
   Call Con_ConnectConfigure() to write all connections to the firmware.

   \param  pSrc         Pointer to the source module which provides the
                        output signal.
   \param  pDst         Pointer to the destination module where the output
                        is connected to.
   \param  nSide        Specifies which output to use when the source is a
                        signaling module, don't care for other module types
                        (set to 0).
                        If set to LOCAL_SIG_OUT then the output towards the
                        local side is used and with REMOTE_SIG_OUT the output
                        towards the remote side.

   \return
   - VMMC_statusOk if successful
   - error code otherwise

   \remarks
   To remove a signal connection the input in the destination module has to
   be unlinked from the list the source module keeps of all inputs connecting
   to it's output. Then the input struct in the destination can be blanked.
   If all was successful the destination is set to modified.
   Disconnecting a non existing connection returns also IFX_SUCCESS.
*/
static
IFX_int32_t LOCAL_DisconnectPrepare (
                        VMMCDSP_MODULE_t* pSrc,
                        VMMCDSP_MODULE_t* pDst,
                        SIG_OUTPUT_SIDE nSide)
{
   int i, in = -1;
   VMMCDSP_MODULE_SIGNAL_t **pBase,
                         *pTemp;
   IFX_uint8_t mute;

   if(pDst->nModType == VMMCDSP_MT_SIG)
   {
      /* For SIG as destination currently only a disconnect from the
         local side is supported. The driver never disconnects the data
         channel so the remote side is not implemented. */
      if (pDst->in[LOCAL_SIG_IN].pOut == pSrc)
         in = LOCAL_SIG_IN;
   }
   else
   {
      /* Find the input on the destination which is connected to the source. */
      for (i = 0; i < MAX_MODULE_SIGNAL_INPUTS; i++)
      {
         if (pDst->in[i].i == pSrc->nSignal)
         {
            in = i;
            break;
         }
      }
   }

#ifdef DEBUG
   TRACE(VMMC, DBG_LEVEL_LOW,
         ("removing Signal %s-OUT from %s/input:%d\n",
          in != -1 ? signalName[pDst->in[in].i] : "<empty>",
          signalName[pDst->nSignal], in));
#endif /* DEBUG */

   if (in == -1)
   {
     /* placed after the trace to get details on the modules (debug only) */
      TRACE(VMMC, DBG_LEVEL_NORMAL,("Given modules are not connected\n"));
      return VMMC_statusOk;
   }

   /* Get a pointer to the basepointer of the list of inputs connected. */
   if (nSide == REMOTE_SIG_OUT && pSrc->nModType == VMMCDSP_MT_SIG)
   {
      /* SIG-module only: remote side input */
      pBase = &pSrc->pInputs2;
      mute = pSrc->nMute2;
   }
   else
   {
      /* All modules: local side input */
      pBase = &pSrc->pInputs;
      mute = pSrc->nMute;
   }
   /* Get a pointer to the first element in the linked list of inputs. */
   pTemp = *pBase;
   /* pTemp now points to the first node in the list of input signals
      connected to this output. Now find the node of the input found
      above and remove this node from the linked list. */
   if (*pBase == &pDst->in[in])
   {
      /* special case: it is the first node so move the base pointer */
      *pBase = pDst->in[in].pNext;
   }
   else
   {
      /* Walk the list and try to find the node to be deleted. */
      while (pTemp != IFX_NULL && pTemp->pNext != &pDst->in[in])
         pTemp = pTemp->pNext;
      if (pTemp == IFX_NULL)
      {
         /* not found! The data structure is corrupt - this should not happen*/
         VMMC_ASSERT(0);
         return IFX_ERROR;
      }
      /* unlink the node from the list */
      pTemp->pNext = pTemp->pNext->pNext;
   }
   /* clear this input -> connection is now removed */
   pDst->in[in].pNext = IFX_NULL;
   pDst->in[in].i     = ECMD_IX_EMPTY;
   pDst->in[in].i_mute= ECMD_IX_EMPTY;
   pDst->in[in].mute -= mute;
   pDst->in[in].pOut  = IFX_NULL;
   pDst->modified     = IFX_TRUE;

   return VMMC_statusOk;
}


/** \addtogroup VMMC_CONNECTIONS */
/* @{ */

/**
   Add a signal connection to the internal shadow connection structure.
   Call Con_ConnectConfigure() to write the shadow structure to the chip.

   \param  pSrcCh       Pointer to the VMMC channel structure.
   \param  src          Source module which contains the output signal.
   \param  pDstCh       Pointer to the VMMC channel structure.
   \param  dst          Destination module where the output is connected to.
   \param  nSide        Specifies which output to use when the source is a
                        signaling module, don't care for other module types
                        (set to 0).
                        If set to LOCAL_SIG_OUT then the output towards the
                        local side is used and with REMOTE_SIG_OUT the output
                        towards the remote side.

   \return
   - VMMC_statusOk if successful
   - error code otherwise

   \remarks
   This function only connects two given modules if they are not already
   connected. The destination is set to modified if all was successful.
   The connection inherits a mute attribute from the module output that
   serves as the signal source for the connection. So when a connection
   is created on an output set to mute it will itself carry the mute
   attribute.
   As signalling modules have two sides (local/remote) to which other
   modules can be connected the side has to be determined first.
   When the signalling modules acts as an input rules are used to determine
   to which input side a module is connected:
   Local modules like ALM and PCM (not always) support auto suppression and
   get the event playout. They are connected to In 1.
   Remote modules like coder and PCM in case of advanced with DSP are
   connected to In 2.
   When the signalling module acts as an output nSide is used to select
   which side of the signalling module is used for the connection. The value
   LOCAL_SIG_OUT is used for all analog channels. The value REMOTE_SIG_OUT
   is used for the coder only to connect to the Sig-OutA output.

   \verbatim
                          nSignal2
                          pInputs2
   Signalling module:      |----------------|
                       <-  | Out A     In 1 |  <-
                           |  upstream (tx) |
          remote <==       |  -  -  -  -  - |       ==> local
                           | downstream (rx)|
                       ->  | In 2     Out B |  ->
                           |----------------|
                                       nSignal
                                       pInputs
   \endverbatim
*/
IFX_int32_t VMMC_CON_ConnectPrepare (
                        VMMC_CHANNEL *pSrcCh, VMMCDSP_MT src,
                        VMMC_CHANNEL *pDstCh, VMMCDSP_MT dst,
                        SIG_OUTPUT_SIDE nSide)
{
   VMMCDSP_MODULE_t *pSrc = IFX_NULL,
                    *pDst = IFX_NULL;

   switch (src)
   {
   case VMMCDSP_MT_ALM:
      pSrc = &pSrcCh->pCON->modAlm;
      break;
   case VMMCDSP_MT_SIG:
      pSrc = &pSrcCh->pCON->modSig;
      break;
   case VMMCDSP_MT_COD:
      pSrc = &pSrcCh->pCON->modCod;
      break;
#ifdef VMMC_FEAT_PCM
   case VMMCDSP_MT_PCM:
      pSrc = &pSrcCh->pCON->modPcm;
      break;
#endif /* VMMC_FEAT_PCM */
   default:
      /* internal error: not an supported module type */
      return IFX_ERROR;
   }

   switch (dst)
   {
   case VMMCDSP_MT_ALM:
      pDst = &pDstCh->pCON->modAlm;
      break;
   case VMMCDSP_MT_SIG:
      pDst = &pDstCh->pCON->modSig;
      break;
   case VMMCDSP_MT_COD:
      pDst = &pDstCh->pCON->modCod;
      break;
#ifdef VMMC_FEAT_PCM
   case VMMCDSP_MT_PCM:
      pDst = &pDstCh->pCON->modPcm;
      break;
#endif /* VMMC_FEAT_PCM */
   default:
      /* internal error: not an supported module type */
      return IFX_ERROR;
   }

   return LOCAL_ConnectPrepare (pSrc, pDst, nSide);
}


/**
   Write the configuration from the internal connection structures to the
   firmware.

   \param  pDev         Pointer to the VMMC device structure.

   \return
   - VMMC_statusOk if successful
   - error code otherwise
*/
IFX_int32_t VMMC_CON_ConnectConfigure (VMMC_DEVICE *pDev)
{
   VMMC_CHANNEL *pCh;
   IFX_int32_t ret = VMMC_statusOk;
   IFX_uint8_t  ch;

   /* Set preliminary to first channel for error reporting only. */
   pCh = &pDev->pChannel[0];

   VMMC_OS_MutexGet (&pDev->mtxMemberAcc);

   /* write ALM module inputs */
   for (ch = 0; VMMC_SUCCESS(ret) && (ch < pDev->caps.nALI); ++ch)
   {
      pCh = &pDev->pChannel[ch];

      /* call the module specific set function only if something was modified */
      if (pCh->pCON->modAlm.modified != IFX_FALSE)
      {
         /* write ALM module inputs */
         ret = VMMC_ALM_Set_Inputs (pCh);
         /* clear the modified flag now that we updated the cached fw message */
         pCh->pCON->modAlm.modified = IFX_FALSE;
      }
   }

   /* write CODer module inputs */
   for (ch = 0; VMMC_SUCCESS(ret) && (ch < pDev->caps.nCOD); ++ch)
   {
      pCh = &pDev->pChannel[ch];

      /* call the module specific set function only if something was modified */
      if (pCh->pCON->modCod.modified != IFX_FALSE)
      {
         /* write CODer module inputs */
         ret = VMMC_COD_Set_Inputs (pCh);
         /* clear the modified flag now that we updated the cached fw message */
         pCh->pCON->modCod.modified = IFX_FALSE;
      }
   }

#ifdef VMMC_FEAT_PCM
   /* write PCM module inputs */
   for (ch = 0; VMMC_SUCCESS(ret) && (ch < pDev->caps.nPCM); ++ch)
   {
      pCh = &pDev->pChannel[ch];

      /* call the module specific set function only if something was modified */
      if (pCh->pCON->modPcm.modified != IFX_FALSE)
      {
         /* write PCM module inputs */
         ret = VMMC_PCM_Set_Inputs (pCh);
         /* clear the modified flag now that we updated the cached fw message */
         pCh->pCON->modPcm.modified = IFX_FALSE;
      }
   }
#endif /* VMMC_FEAT_PCM */

   /* write SIGnalling module inputs */
   for (ch = 0; VMMC_SUCCESS(ret) && (ch < pDev->caps.nSIG); ++ch)
   {
      pCh = &pDev->pChannel[ch];

      /* call the module specific set function only if something was modified */
      if (pCh->pCON->modSig.modified != IFX_FALSE)
      {
         /* write SIG module inputs */
         ret = VMMC_SIG_Set_Inputs (pCh);
         /* clear the modified flag now that we updated the cached fw message */
         pCh->pCON->modSig.modified = IFX_FALSE;
      }
   }

#ifdef DECT_SUPPORT
   /* write DECT module inputs */
   for (ch = 0; VMMC_SUCCESS(ret) && (ch < pDev->caps.nDECT); ++ch)
   {
      pCh = &pDev->pChannel[ch];

      /* call the module specific set function only if something was modified */
      if (pCh->pCON->modDect.modified != IFX_FALSE)
      {
         /* write DECT module inputs */
         ret = VMMC_DECT_Set_Inputs (pCh);
         /* clear the modified flag now that we updated the cached fw message */
         pCh->pCON->modDect.modified = IFX_FALSE;
      }
   }
#endif /* DECT_SUPPORT */

   VMMC_OS_MutexRelease (&pDev->mtxMemberAcc);

   RETURN_STATUS(ret);
}

/* @} */


/* ==================================================== */
/* Functions for wideband/narrowband conference support */
/* ==================================================== */

/* The feature "Wideband" means that all signals are sampled with the double
   sampling frequency so instead of 8 kHz which is now called narrowband
   16 kHz are used. This directly leads to an increase of the bandwidth of
   the voice path.
   Wideband is realised distributed on the single modules in the system. So
   when connecting modules the conferencing code makes sure that all modules
   run in the same operation mode. All modules have the ability to select the
   operation mode 8/16 kHz of sampling from the signalling array. This ISR
   (internal sampling rate) switch must be set to the same rate on all modules
   which are connected. Apart from this most modules have an extra setting
   that allows them to run in 8 kHz mode while sampling with 16 kHz from the
   signalling array. This switch is done with either setting the appropriate
   coder or the the UD-bit (up-/down-sampling).

   The code here keeps an extra attribute in each channel which tells the
   current operation mode of each module. There are four states:
    -off: the module is turned off and does not provide any output to the
          signalling array.
    -NB:  the modules samples with 8 kHz to/from the signalling array
    -WB:  the modules samples with 16 kHz to/from the signalling array
    -AUTO: the module is able to run in NB or WB mode. Current mode is
          determined by the other modules in the conference. Modules with
          AUTO prefer to run in wideband mode if there are other AUTO modules
          in the conference. ALM modules are marked as AUTO.
   The state is updated by each module whenever it's operation mode changes.
   The following events in the modules trigger a reevaluation of the operation
   mode of the conference the module is part of:
    -Decoder changes between NB and WB
    -Encoder sampling mode is changed between NB and WB
    -Encoder or Decoder are started or stopped
    -Sampling mode of ALM is changed by IOCTL
    -Connection between modules is created
    -Connection between modules is deleted
*/

/**
   Set the module's sampling operation mode.

   \param  pCh             Pointer to the VMMC channel structure.
   \param  src             Module type.
   \param  sampling_mode   Sampling mode as defined in VMMC_CON_SAMPLING.
*/
IFX_void_t VMMC_CON_ModuleSamplingModeSet (VMMC_CHANNEL *pCh,
                                           VMMCDSP_MT src,
                                           VMMC_CON_SAMPLING sampling_mode)
{
   /* no parameter check because this function is called only by
      VMMC internal interfaces */
   switch (src)
   {
#ifdef VMMC_FEAT_PCM
   case VMMCDSP_MT_PCM:
      pCh->pCON->modPcm.sampling_mode = sampling_mode;
      break;
#endif /* VMMC_FEAT_PCM */
   case VMMCDSP_MT_ALM:
      pCh->pCON->modAlm.sampling_mode = sampling_mode;
      break;
   case VMMCDSP_MT_COD:
      pCh->pCON->modCod.sampling_mode = sampling_mode;
      break;
#ifdef DECT_SUPPORT
   case VMMCDSP_MT_DECT:
      pCh->pCON->modDect.sampling_mode = sampling_mode;
      break;
#endif /* DECT_SUPPORT */
   default:
      /* error: not an supported module type */
      /*lint -e(506, 774) */
      VMMC_ASSERT(0);
   }
}


/**
   Traverses the passed module's inputs for connected modules (recursively).

   The function scans all valid module inputs for connected modules.
   In case a module is found it is checked whether the module is already
   comprised in the conference member list.
   If not the module is added to the conference member list and the function
   calls itself pointing to the connected module found.

   \param  pMod         Pointer to firmware module structure where to start
                        the traversal.

   \return
   - IFX_SUCCESS if connected module was found at input input_index.
   - IFX_ERROR if no module was found at input or an invalid parameter was passed.
*/
static IFX_return_t vmmc_con_TraverseModules( VMMCDSP_MODULE_t* pMod )
{
   IFX_uint32_t        input_index;
   CONF_LIST_ELEMENT_t connected_module;

   if( pMod == IFX_NULL )
      return IFX_ERROR;

   memset( &connected_module, 0, sizeof(connected_module));

#if 0
   TRACE(VMMC, DBG_LEVEL_LOW,
      ("%s - pMod->nModType = %d\n", __FUNCTION__, pMod->nModType));
#endif

   for (input_index = 0; input_index < MAX_MODULE_SIGNAL_INPUTS; ++input_index)
   {
      if( (pMod->in[input_index].i != ECMD_IX_EMPTY) &&
          (pMod->in[input_index].pOut != IFX_NULL) )
      {
         /* get module connected to this input,
            add module to conference member list and proceed traversal of
            connected modules (recursive call of vmmc_con_TraverseModules) */
         if( vmmc_con_GetInputMod( pMod, input_index, &connected_module )
             == IFX_SUCCESS )
         {
            /* Connected Module found at input */
            if( vmmc_con_ConfMemList_ElemInList( &connected_module ) == IFX_FALSE )
            {
               /* connected module is not in the conf. member list
                  -> append it and proceed module traversal with
                     the connected module found                   */
               vmmc_con_ConfMemList_AppendElem( &connected_module );
               vmmc_con_TraverseModules( connected_module.pModule );
            }
         }
      }
   }

   return IFX_SUCCESS;
}


/**
   Returns the module connected to the module's input defined by input_index.

   \param  pMod         Pointer to firmware module structure.
   \param  input_index  Index of module input to be checked.
   \param  pModConn     Pointer to connected module found (already transformed
                        to list element structure type).

   \return
   - IFX_SUCCESS if connected module was found at input input_index.
   - IFX_ERROR if no module was found at input or an invalid parameter was passed.
*/
static IFX_return_t vmmc_con_GetInputMod( VMMCDSP_MODULE_t*    pMod,
                                          IFX_uint8_t          input_index,
                                          CONF_LIST_ELEMENT_t* pModConn )
{
   IFX_return_t retval=IFX_ERROR;

   if( input_index >= MAX_MODULE_SIGNAL_INPUTS ||
       pMod == IFX_NULL || pModConn == IFX_NULL)
   {
      return retval;
   }
   /* check if selected input is connected to other FW module's output */
   if( (pMod->in[input_index].i    != ECMD_IX_EMPTY) &&
       (pMod->in[input_index].pOut != IFX_NULL) )
   {
      /* fill the pModule pointer */
      pModConn->pModule = pMod->in[input_index].pOut;

#if 0
      TRACE(VMMC, DBG_LEVEL_LOW,
           ("Module 0x%08lX, type %u, input %u <- Module 0x%08lX, type %u\n",
             (IFX_uint32_t)pMod, pMod->nModType, input_index,
             (IFX_uint32_t)pMod->in[input_index].pOut, pModConn->pModule->nModType));
#endif
      retval = IFX_SUCCESS;
   }

   return retval;
}


/**
   Mutes or unmutes active coder channels in conference.

   \param  mute         Mute (IFX_ENABLE) or unmute (IFX_DISABLE) active
                        conference.

   \return
   - IFX_SUCCESS if all activer coder channels were (un)muted.
   - IFX_ERROR if the conf member list does not exist or
               coder channel (un)muting failed.
*/
static
IFX_return_t vmmc_con_ConfMemList_MuteCtrlCoderChan( IFX_operation_t mute )
{
   CONF_LIST_ELEMENT_t *pList;

   if( conf_list_created == IFX_FALSE )
   {
      TRACE(VMMC, DBG_LEVEL_HIGH,
         (__FILE__ ":%d: Conf. Member List not available\n", __LINE__));
      return IFX_ERROR;
   }

   /*
    * Beginning at the conf. member list start element the list is scanned
    * for active coder channel modules.
    * Every active coder channel found is then (un)muted according
    * to passed mute-parameter.
    */
   pList = conf_listStart;
   while( pList )
   {
      if((pList->pModule->nModType == VMMCDSP_MT_COD) &&
         (pList->pModule->sampling_mode != VMMC_CON_SMPL_OFF))
      {
         /*
          * active coder module found
          * mute or unmute this active coder module
          */
         TRACE(VMMC, DBG_LEVEL_LOW,
               ("-> %s coder channel %d\n",
               mute == IFX_ENABLE ? "mute" : "unmute",
               pList->pModule->pCh->nChannel-1));

         if( VMMC_COD_ENC_Hold( pList->pModule->pCh, mute) != IFX_SUCCESS )
         {
            TRACE(VMMC, DBG_LEVEL_HIGH,
                  (__FILE__ ":%d: failed to %s coder channel %d\n",
                  __LINE__, mute == IFX_ENABLE ? "mute" : "unmute",
                  pList->pModule->pCh->nChannel-1));
            return IFX_ERROR;
         }
      }

      /* proceed with next list element */
      pList=pList->next;
   }
   return IFX_SUCCESS;
}


/**
   Configures the module pointed to by pMod to the requested operation mode
   or checks if an action needs to be done.

   \param action Select setting or checking of sampling rate.
   \param pMod   Pointer to firmware module to be configured.
   \param mode   Sampling mode selector (8 KHz NB or 16KHz WB).
   \return
      - IFX_SUCCESS if passed module has been successfully configured.
      - IFX_ERROR in case of module configuration error.
*/
static IFX_int32_t vmmc_con_ModSamplingMode (SM_ACTION        action,
                                             VMMCDSP_MODULE_t *pMod,
                                             OPMODE_SMPL      mode)
{
   IFX_int32_t ret = IFX_ERROR;

   VMMC_ASSERT(pMod != IFX_NULL);
   /*lint -esym(613, pMod)*/

   switch( pMod->nModType )
   {
      case VMMCDSP_MT_ALM:
         ret = VMMC_ALM_SamplingMode(pMod->pCh, action, mode,
                  (((pMod->sampling_mode == VMMC_CON_SMPL_AUTO) && (mode == WB_16_KHZ)) ||
                    (pMod->sampling_mode == VMMC_CON_SMPL_WB)) ? WB_16_KHZ : NB_8_KHZ);
         break;
      case VMMCDSP_MT_SIG:
         ret = VMMC_SIG_SamplingMode(pMod->pCh, action, mode);
         break;
      case VMMCDSP_MT_COD:
         ret = VMMC_COD_SamplingMode(pMod->pCh, action, mode);
         break;
#ifdef DECT_SUPPORT
      case VMMCDSP_MT_DECT:
         ret = VMMC_DECT_SamplingMode(pMod->pCh, action, mode);
         break;
#endif /* DECT_SUPPORT */
#ifdef VMMC_FEAT_PCM
      case VMMCDSP_MT_PCM:
         /* PCM does not support automatic mode it only knows NB or WB */
         ret = VMMC_PCM_SamplingMode(pMod->pCh, action, mode,
                  (pMod->sampling_mode == VMMC_CON_SMPL_WB) ? WB_16_KHZ : NB_8_KHZ);
         break;
#endif /* VMMC_FEAT_PCM */
      default:
         TRACE(VMMC, DBG_LEVEL_HIGH,
            (__FILE__ ":%d: Invalid Mod. type %d\n", __LINE__, pMod->nModType));
         break;
   }

   return ret;
}


/**
   Match the internal sampling rate setting of all modules in the conference
   that is attached to the given module.

   First the conference members are looked up. Then the members are checked
   for their sampling mode. The conference is put into wideband mode when
   there are at least 2 ALM modules which are in automatic mode
   or one of the conference members is configured to work in wideband only.
   If wideband is not required the conference is put into narrowband mode.

   \param  pCh             VMMC Channel handle.
   \param  start_module    Channels FW module to start traversal with.

   \return
   - IFX_SUCCESS If conference has been successfully configured.
   - IFX_ERROR   In case of module configuration error.

*/
IFX_int32_t VMMC_CON_MatchConfSmplRate (VMMC_CHANNEL *pCh,
                                        VMMCDSP_MT start_module)
{
   IFX_int32_t wb_pref_cnt = 0,      /* WB prefered decisive modules counter */
               wb_only_cnt = 0;      /* wideband mode only counter */
   CONF_LIST_ELEMENT_t  *pList;      /* conference member list pointer */
   OPMODE_SMPL sampling_rate;
   IFX_int32_t retval=IFX_SUCCESS;
   IFX_boolean_t switching_needed;

   retval = VMMC_OS_MutexLockInterruptible(&mtxConfListAcc);
   if (retval != 0)
   {
      return VMMC_statusErr;
   }

   /* 1. create a list of conference members */
   vmmc_con_ConfMemListCreate(pCh, start_module);

   /* 2. walk the list of conference members and count the number of modules
         able to automatically switch to wideband and the number of modules
         configured to work in wideband only. */
   pList = conf_listStart;
   while( pList != IFX_NULL )
   {
      /* Search for WB capable FW modules that are set to automatic switching.
         These modules prefer to run in wideband if at least two can be found. */
      if (pList->pModule->sampling_mode == VMMC_CON_SMPL_AUTO)
      {
         wb_pref_cnt++;
      }

      /* Modules which are set fix to wideband force the signalling array
         to work in wideband mode. So the conference is wideband. */
      if (pList->pModule->sampling_mode == VMMC_CON_SMPL_WB)
      {
         wb_only_cnt++;
      }

      pList=pList->next;
   }

   TRACE(VMMC, DBG_LEVEL_LOW,
         ("WB pref cnt = %d   WB only cnt = %d\n",
          wb_pref_cnt, wb_only_cnt));

   /* 3. make decision which sampling mode is required for the signalling array
    * if at least 2 decisive WB capable modules or
    * an active wideband coder channel are/is found
    * configure conference for 16 KHz mode, otherwise for 8KHz
    */
   if( (wb_pref_cnt >= 2) || (wb_only_cnt > 0) )
   {
      sampling_rate = WB_16_KHZ;
   }
   else
   {
      sampling_rate = NB_8_KHZ;
   }

   TRACE(VMMC, DBG_LEVEL_LOW,
         ("Set sampling rate for conference: %s\n",
          sampling_rate == WB_16_KHZ ? "WIDEBAND" : "NARROWBAND"));

   /* 4. Determine if any conference member needs a sampling rate switch.
         This tells us if muting is needed. */
   switching_needed = IFX_FALSE;
   pList = conf_listStart;
   while( pList != IFX_NULL )
   {
      if( vmmc_con_ModSamplingMode( SM_CHECK,
                                    pList->pModule, sampling_rate) == IFX_TRUE )
      {
         switching_needed = IFX_TRUE;
         break;
      }
      pList=pList->next;
   }

   /* 5. mute the coder channels before changing the sampling mode */
   if (switching_needed)
   {
      if(vmmc_con_ConfMemList_MuteCtrlCoderChan (IFX_ENABLE) != IFX_SUCCESS)
      {
         TRACE(VMMC, DBG_LEVEL_HIGH,
               ("Muting coder channels failed\n"));
         retval = IFX_ERROR;
      }
   }

   /* 6. configure all conference members to the same sampling rate */
   /* This is needed even though we might not switch the sampling rate because
      the coders are configured in this step to use new types. */
   pList = conf_listStart;
   while( pList != IFX_NULL )
   {
      if( vmmc_con_ModSamplingMode( SM_SET, pList->pModule,
                                    sampling_rate ) != IFX_SUCCESS )
      {
         TRACE(VMMC, DBG_LEVEL_HIGH,
               ("Sampling Mode Configuration failed for Module 0x%p\n",
               pList->pModule));
         retval = IFX_ERROR;
      }
      pList=pList->next;
   }

   /* 7. unmute the coder channels again */
   /* UnMute coder channels after sampling rate change is done */
   if (switching_needed)
   {
      if( vmmc_con_ConfMemList_MuteCtrlCoderChan( IFX_DISABLE ) != IFX_SUCCESS)
      {
         TRACE(VMMC, DBG_LEVEL_HIGH,
               ("Unmuting coder channels failed\n"));
         retval = IFX_ERROR;
      }
   }

   /* 8. delete again the list of conference members */
   vmmc_con_ConfMemListDelete();

   VMMC_OS_MutexRelease(&mtxConfListAcc);

   return retval;
}


/**
   Creates list of firmware modules that are connected in a conference.

   \param  pCh             Pointer to VMMC channel structure.
   \param  start_module    Module type to start search.

   \return
   - != IFX_NULL pointer to the first element of the conference member list
   - IFX_NULL if list was not created.
*/
static
CONF_LIST_ELEMENT_t* vmmc_con_ConfMemListCreate( VMMC_CHANNEL *pCh,
                                                 VMMCDSP_MT start_module )
{
   VMMCDSP_MODULE_t    *root;
   CONF_LIST_ELEMENT_t list_elem;

   if( pCh == IFX_NULL )
   {
      return ((CONF_LIST_ELEMENT_t*)IFX_NULL);
   }

  /*
   * set the coder module of the passed channel as root element
   * for conference traversal
   */
   switch(start_module)
   {
      case VMMCDSP_MT_COD:
      case VMMCDSP_MT_SIG:
         root = &pCh->pCON->modCod;
         break;
#ifdef VMMC_FEAT_PCM
      case VMMCDSP_MT_PCM:
         root = &pCh->pCON->modPcm;
         break;
#endif /* VMMC_FEAT_PCM */
      case VMMCDSP_MT_ALM:
         root = &pCh->pCON->modAlm;
         break;
#ifdef DECT_SUPPORT
      case VMMCDSP_MT_DECT:
         root = &pCh->pCON->modDect;
         break;
#endif /* DECT_SUPPORT */
      default:
         /* error: not an supported module type */
         /*lint -e(506, 774) */
         VMMC_ASSERT(0);
         return ((CONF_LIST_ELEMENT_t*)IFX_NULL);
   }

   if(!conf_list_created)
   {
      /*
       * copy passed root to list element structure, add element to list and
       * start conference traversal in order to append all conference members
       * to list
       */
      list_elem.pModule = root;
      vmmc_con_ConfMemList_AppendElem( &list_elem );
      vmmc_con_TraverseModules( root );
      conf_list_created = IFX_TRUE;
      /* TRACE(VMMC, DBG_LEVEL_LOW,
         ("Created Conf. Member List with %d elements\n", conf_listMembers)); */
   }

   return conf_listStart;
}


/**
   Deletes list of firmware modules that are connected in a conference.

   \return
   - IFX_SUCCESS if conference member list has been deleted successfully.
   - IFX_ERROR if list or list start pointer does not exist.

   \remarks
   Deletion procedure uses the list element's next pointer to access
   following element.
*/
static IFX_return_t vmmc_con_ConfMemListDelete( void )
{
   CONF_LIST_ELEMENT_t  *pList, *next;

   /* sanity checks */
   if( !conf_list_created )
   {
      return IFX_ERROR;
   }

   if( conf_listStart==IFX_NULL )
   {
      return IFX_ERROR;
   }

   pList = conf_listStart;
   while(pList)
   {
      /* store next list element to be deleted, delete current element and
         increment list pointer */
      next = pList->next;
      VMMC_OS_Free (pList);
      conf_listMembers--;
      pList = next;
   }
   /* sanity check after removal of list elements */
   if( conf_listMembers != 0 )
   {
      TRACE(VMMC, DBG_LEVEL_HIGH,
           (__FILE__"%d: Invalid number of list members (%d) after deletion\n",
            __LINE__, conf_listMembers));
      return IFX_ERROR;
   }
   conf_listStart = conf_listEnd = IFX_NULL;
   conf_listMembers = 0;
   conf_list_created = IFX_FALSE;
   /* TRACE(VMMC, DBG_LEVEL_LOW,
         ("%s - Conf. Member list removed\n", __FUNCTION__)); */
   return IFX_SUCCESS;
}


/**
   Checks if the passed list element structure is already comprised
   in the conference member list.

   \param  plist_elem   Pointer to list element structure to be checked for
                    list membership.

   \return
   - IFX_TRUE if passed element is a list member already.
   - IFX_FALSE if passed element is not a list member.

   \remarks
   The list element's adr_id field is used for comparison.
*/
static
IFX_boolean_t vmmc_con_ConfMemList_ElemInList( CONF_LIST_ELEMENT_t* plist_elem )
{
   CONF_LIST_ELEMENT_t* pList = conf_listStart;
   IFX_boolean_t elem_found = IFX_FALSE;

   /* scan list for passed element,
      element identification is done by adr_id field comparison */
   while((pList != IFX_NULL) && (elem_found == IFX_FALSE))
   {
      if( pList->pModule == plist_elem->pModule )
          elem_found = IFX_TRUE;

      pList=pList->next;
   }
   return elem_found;
}


/**
   Append element to conference member list.

   \param  plist_elem   Pointer to list element structure to be checked for
                    list membership.

   \return
   - IFX_TRUE if passed element is a list member already.
   - IFX_FALSE if passed element is not a list member.
*/
static
IFX_return_t vmmc_con_ConfMemList_AppendElem( CONF_LIST_ELEMENT_t* plist_elem )
{
   CONF_LIST_ELEMENT_t *pNew;  /* pointer to new list element */

   /*
    * allocate memory for new list element
    */
   pNew = (CONF_LIST_ELEMENT_t*)VMMC_OS_Malloc (sizeof(CONF_LIST_ELEMENT_t));
   if( pNew == IFX_NULL )
   {
      TRACE(VMMC, DBG_LEVEL_HIGH,
            ("Could not allocate memory for new conf. member list element\n"));
      return IFX_ERROR;
   }

   /* copy the content from the passed pointer */
   *pNew = *plist_elem;

   /* append the new element to the end of the list */
   if( conf_listStart == IFX_NULL )
   {
      /* add first element to list -> set start pointer to first list element */
      conf_listStart = pNew;
   }
   else
   {
      /* add nth element to the list */
         conf_listEnd->next = pNew;
      }
   /* update list end pointer to new element and count added element */
   conf_listEnd = pNew;
   conf_listEnd->next = IFX_NULL;
   conf_listMembers++;

/* TRACE(VMMC, DBG_LEVEL_LOW,
         ("Added Module: Channel=%1d type=%1d ID=%p\n",
          plist_elem->pModule->pCh->nChannel - 1,
          plist_elem->pModule->nModType, plist_elem->pModule)); */

   return IFX_SUCCESS;
}
