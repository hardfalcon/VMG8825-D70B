/******************************************************************************

                              Copyright (c) 2012
                            Lantiq Deutschland GmbH
                             http://www.lantiq.com

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/**
   \file drv_vmmc_announcements.c
   This file contains the implementation of the functions for the announcement
   feature.
*/

/* ============================= */
/* Check if feature is enabled   */
/* ============================= */
#include "drv_api.h"
#ifdef VMMC_FEAT_ANNOUNCEMENTS

/* ============================= */
/* Includes                      */
/* ============================= */

#include "drv_vmmc_cod_priv.h"
#include "drv_vmmc_init.h"
#include "drv_mps_vmmc.h"
#include "drv_vmmc_api.h"
#include "drv_vmmc_cod.h"
#include "drv_vmmc_errno.h"
#include "drv_vmmc_announcements.h"


/* ============================= */
/* Global constant definition    */
/* ============================= */

/* ============================= */
/* Global macro definition       */
/* ============================= */

/* ============================= */
/* Type declarations             */
/* ============================= */

struct VMMC_ANNOUNCEMENT_HEADER
{
   /** Encoder type as used in the coder channel speech compression command */
   IFX_uint32_t ENC : 5;
   /** Announcement ID */
   IFX_uint32_t ANNID : 8;
   /** Length of the payload in byte */
   IFX_uint32_t LENGTH : 19;
} __PACKED__ ;

typedef struct VMMC_ANNOUNCEMENT_HEADER VMMC_ANNOUNCEMENT_HEADER_t;

struct VMMC_ANN_s
{
   /* Handle to the used channel */
   VMMC_CHANNEL *pCh;
   /* pointer to announcement data */
   VMMC_ANNOUNCEMENT_HEADER_t *pAddress;
   /* size of announcemet payload */
   IFX_uint32_t nSize;
   /** Prohibits access an announcement is played and
      freeing of currently played announcement. */
   VMMC_OS_mutex_t semProtectAnn;
};

/* ============================= */
/* Local variable definition     */
/* ============================= */

/* ============================= */
/* Local function declaration    */
/* ============================= */

/* ============================= */
/* Global functions declaration  */
/* ============================= */

/* ============================= */
/* Global function definition    */
/* ============================= */

/**
   Initialize the announcement service

   \return
   - VMMC_statusOk - if successful
   - VMMC_statusNoMem - no free memory
*/
IFX_int32_t VMMC_Ann_Init (VMMC_DEVICE* pDev)
{
   VMMC_ANN_t *pAnn;

   VMMC_ASSERT (pDev);

   if (IFX_NULL != pDev->pAnn)
   {
      /* during initialization of unsupported feature no any error generation */
      return VMMC_statusOk;
   }

   /* allocate announcement resources */
   pDev->pAnn = VMMC_OS_Malloc (sizeof (*pDev->pAnn) * VMMC_ANNOUNCEMENTS_MAX);
   if (IFX_NULL == pDev->pAnn)
      RETURN_DEVSTATUS (VMMC_statusNoMem);

   memset (pDev->pAnn, 0, sizeof (*pDev->pAnn) * VMMC_ANNOUNCEMENTS_MAX);

   for (pAnn = pDev->pAnn; pAnn < pDev->pAnn + VMMC_ANNOUNCEMENTS_MAX; pAnn++)
   {
      /* create announcement protection semaphore */
      VMMC_OS_MutexInit (&pAnn->semProtectAnn);
   }

   return VMMC_statusOk;
}

/**
   Clean-up the announcement service
*/
IFX_void_t VMMC_Ann_Cleanup (VMMC_DEVICE* pDev,
                             IFX_boolean_t bChipAccess)
{
   VMMC_ANN_t *pAnn;

   VMMC_ASSERT (pDev);

   if (IFX_NULL == pDev->pAnn)
      return;

   for (pAnn = pDev->pAnn; pAnn < pDev->pAnn + VMMC_ANNOUNCEMENTS_MAX; pAnn++)
   {
      if ((IFX_NULL != pAnn->pCh) && (bChipAccess != IFX_FALSE))
      {
         /* always expect successful result */
         (IFX_void_t) IFX_TAPI_LL_Ann_Stop (pAnn->pCh);
      }

      if (IFX_NULL != pAnn->pAddress)
      {
         VMMC_OS_Free(pAnn->pAddress);
         pAnn->pAddress = IFX_NULL;
         pAnn->nSize = 0;
      }

      /* delete announcement protection semaphore */
      VMMC_OS_MutexDelete (&pAnn->semProtectAnn);
   }

   VMMC_OS_Free (pDev->pAnn);
   pDev->pAnn = IFX_NULL;
}

/**
   Process announcement end event.

   \param   pCh   Handle to VMMC_CHANNEL structure.
*/
IFX_void_t VMMC_AnnEndEventServe (VMMC_CHANNEL* pCh)
{
   VMMC_ASSERT (pCh);

   if (IFX_NULL == pCh->pAnn)
   {
      /* announcement are not started on that channel */
      return;
   }

   VMMC_OS_MutexRelease (&pCh->pAnn->semProtectAnn);
}

/**
   The command adds (or downloads) a new announcement to the VMMC. The VMMC
   allocates a contiguous buffer and copies the data to that buffer.

   \param pLLDev     Handle to low-level device
   \param pCfg       Pointer to the IFX_TAPI_COD_ANNOUNCE_CFG_t structure

   \return
   - VMMC_statusOk - if successful
   - VMMC_statusParam - invalid parameter
   - VMMC_statusAnnInUse - announcement ID in use
   - VMMC_statusErrKernCpy - VMMC_OS_CpyUsr2Kern failed
*/
IFX_int32_t IFX_TAPI_LL_Ann_Cfg (
   IFX_TAPI_LL_DEV_t *pLLDev,
   IFX_TAPI_COD_ANNOUNCE_CFG_t const *pCfg)
{
   VMMC_DEVICE *pDev = (VMMC_DEVICE*)pLLDev;
   IFX_TAPI_CAP_t codCap;
   VMMC_ANN_t *pAnn = IFX_NULL;

   if ((IFX_NULL == pLLDev) || (IFX_NULL == pCfg))
      return VMMC_statusParam;

   if (IFX_NULL == pDev->pAnn)
      RETURN_DEVSTATUS (VMMC_statusNotSupported);

   /* check announcement resource number */
   if (pCfg->nAnnIdx >= VMMC_ANNOUNCEMENTS_MAX)
   {
      TRACE(VMMC,DBG_LEVEL_NORMAL,
         ("Invalid announcement index %d, valid range [0; %d)\n",
            pCfg->nAnnIdx, VMMC_ANNOUNCEMENTS_MAX));
      RETURN_DEVSTATUS (VMMC_statusParam);
   }
   /* check announcement message size, should be <= 64 kB */
   if (pCfg->nAnnSize > VMMC_ANNOUNCEMENTS_SIZE_MAX)
   {
      TRACE(VMMC,DBG_LEVEL_NORMAL,
         ("Invalid announcement payload size %d, valid range [1; %d]\n",
            pCfg->nAnnSize, VMMC_ANNOUNCEMENTS_SIZE_MAX));
      RETURN_DEVSTATUS (VMMC_statusParam);
   }

   /* retrieve corresponding resource */
   pAnn = pDev->pAnn + pCfg->nAnnIdx;

   /* check if announcement is already configured */
   if (IFX_NULL != pAnn->pAddress)
   {
      TRACE(VMMC,DBG_LEVEL_NORMAL,
         ("Announcement %d already in use\n", pCfg->nAnnIdx));
      /* errmsg: Announcement ID already in use */
      RETURN_DEVSTATUS (VMMC_statusAnnInUse);
   }

   pAnn->nSize = pCfg->nAnnSize + sizeof(VMMC_ANNOUNCEMENT_HEADER_t);

   /* allocate memory in VMMC driver for announcement storage */
   pAnn->pAddress = (VMMC_ANNOUNCEMENT_HEADER_t*) VMMC_OS_Malloc (pAnn->nSize);
   if (IFX_NULL == pAnn->pAddress)
   {
      RETURN_DEVSTATUS (VMMC_statusNoMem);
   }

   /* store announcement in VMMC driver */
   if (VMMC_OS_CpyUsr2Kern (
         (IFX_void_t*)pAnn->pAddress,
         (IFX_void_t*)pCfg->pAnn,
         pAnn->nSize) == 0)
   {
      /* errmsg: Copy to of from user space not successful (Linux only) */
      RETURN_DEVSTATUS (VMMC_statusErrKernCpy);
   }

   memset (&codCap, 0, sizeof (codCap));
   codCap.captype = IFX_TAPI_CAP_TYPE_CODEC;
   codCap.cap = (*(IFX_int32_t*)pAnn->pAddress);

   /* check for supported codec */
   if (1 != TAPI_LL_Phone_Check_Capability (pLLDev, &codCap))
   {
      VMMC_OS_Free(pAnn->pAddress);

      /* errmsg: Encoder not supported by the firmware */
      RETURN_DEVSTATUS (VMMC_statusEncoderNotSupported);
   }

   if (pAnn->pAddress->ENC == 0)
   {
      /* correct encoder field */
      pAnn->pAddress->ENC =
         VMMC_COD_trans_cod_tapi2fw ((IFX_TAPI_COD_TYPE_t)codCap.cap,
                                     (IFX_TAPI_COD_BITRATE_t) 0,
                                     pDev->caps.bAMRE);
      /* only G.711 and G.729 are supported by anouncements feature */
      if ((pAnn->pAddress->ENC != COD_CHAN_SPEECH_ENC_G711_ALAW) &&
          (pAnn->pAddress->ENC != COD_CHAN_SPEECH_ENC_G711_MLAW) &&
          (pAnn->pAddress->ENC != COD_CHAN_SPEECH_ENC_G729AB_8) &&
          (pAnn->pAddress->ENC != COD_CHAN_SPEECH_ENC_G729E_11_8))
      {
         /* errmsg: Requested encoder type not supported */
         RETURN_DEVSTATUS (VMMC_statusCodconfNotValid);
      }
      /* fill in announcement ID field */
      pAnn->pAddress->ANNID = pCfg->nAnnIdx;
      /* fill in length field */
      pAnn->pAddress->LENGTH = pCfg->nAnnSize;
   }

   /* invalidate cache */
   ifx_mps_cache_inv ((IFX_ulong_t)pAnn->pAddress, pAnn->nSize);

   return VMMC_statusOk;
}

/**
   This command tells the VMMC to release the buffer, which was allocated for
   an announcement. If the announcement with this ID is currently played back
   by the firmware, the VMMC will stop the announcement and wait for the
   "announcement end" event from the firmware. Only after reception of this
   event, the VMMC will actually free the buffer and mark the announcement ID
   as free. (For this buffer handling, the VMMC will have to implement a simple
   state machine).

   \param pLLDev     Handle to low-level device
   \param pFree      Pointer to the IFX_TAPI_COD_ANNOUNCE_BUFFER_FREE_t
                     structure

   \return
   - VMMC_statusOk - if successful
   - VMMC_statusParam - invalid parameter
   - VMMC_statusAnnNotConfigured - no data for announcement have been downloaded
*/
IFX_int32_t IFX_TAPI_LL_Ann_Free (
   IFX_TAPI_LL_DEV_t *pLLDev,
   IFX_TAPI_COD_ANNOUNCE_BUFFER_FREE_t const *pFree)
{
   VMMC_DEVICE *pDev = (VMMC_DEVICE*)pLLDev;
   VMMC_ANN_t *pAnn = IFX_NULL;
   IFX_int32_t ret;

   if ((IFX_NULL == pLLDev) || (IFX_NULL == pFree))
      return VMMC_statusParam;

   if (IFX_NULL == pDev->pAnn)
      RETURN_DEVSTATUS (VMMC_statusNotSupported);

   /* check announcement resource number */
   if (pFree->nAnnIdx >= VMMC_ANNOUNCEMENTS_MAX)
   {
      TRACE(VMMC,DBG_LEVEL_NORMAL,
         ("Invalid announcement index %d, valid range [0; %d)\n",
            pFree->nAnnIdx, VMMC_ANNOUNCEMENTS_MAX));
      RETURN_DEVSTATUS (VMMC_statusParam);
   }

   /* retrieve corresponding resource */
   pAnn = pDev->pAnn + pFree->nAnnIdx;

   /* check if announcement has been configured */
   if (IFX_NULL == pAnn->pAddress)
   {
      TRACE(VMMC,DBG_LEVEL_NORMAL,
         ("No data for announcement %d\n", pFree->nAnnIdx));
      /* errmsg: Announcement not configured */
      RETURN_DEVSTATUS (VMMC_statusAnnNotConfigured);
   }

   if (IFX_NULL != pAnn->pCh)
   {
      /* It is not allowed to free announcement that is currently playing. */
      ret = IFX_TAPI_LL_Ann_Stop ((IFX_TAPI_LL_CH_t*) pAnn->pCh);
      if (!VMMC_SUCCESS (ret))
         RETURN_DEVSTATUS (ret);
   }

   VMMC_OS_Free(pAnn->pAddress);

   pAnn->pAddress = IFX_NULL;
   pAnn->nSize = 0;

   return VMMC_statusOk;
}

/**
   Starts playing announcement

   \param pLLCh         Pointer to Low-level channel structure
   \param pStart        Pointer to the IFX_TAPI_COD_ANNOUNCE_START_t structure

   \return
   VMMC_statusOk if successful else device specific return code.
*/
IFX_int32_t IFX_TAPI_LL_Ann_Start (
   IFX_TAPI_LL_CH_t *pLLCh,
   IFX_TAPI_COD_ANNOUNCE_START_t const *pStart)
{
   IFX_int32_t ret = VMMC_statusOk;
   VMMC_CHANNEL *pCh = (VMMC_CHANNEL *)pLLCh;
   VMMC_ANN_t *pAnn = IFX_NULL;

   if ((IFX_NULL == pLLCh) || (IFX_NULL == pStart))
      return VMMC_statusParam;

   if (IFX_NULL == pCh->pParent->pAnn)
      RETURN_STATUS (VMMC_statusNotSupported);

   /* check announcement resource number */
   if (pStart->nAnnIdx >= VMMC_ANNOUNCEMENTS_MAX)
   {
      TRACE(VMMC, DBG_LEVEL_NORMAL,
         ("Invalid announcement index %d, valid range [0; %d)\n",
            pStart->nAnnIdx, VMMC_ANNOUNCEMENTS_MAX));
      RETURN_STATUS (VMMC_statusParam);
   }

   /* retrieve corresponding resource */
   pAnn = pCh->pParent->pAnn + pStart->nAnnIdx;

   /* check if announcement has been configured */
   if (IFX_NULL == pAnn->pAddress)
   {
      TRACE(VMMC,DBG_LEVEL_NORMAL,
         ("No data for announcement %d\n", pStart->nAnnIdx));
      /* errmsg: Announcement not configured */
      RETURN_STATUS (VMMC_statusAnnNotConfigured);
   }

   /* check if announcement is already playing on that channel */
   if (IFX_NULL != pCh->pAnn)
   {
      TRACE(VMMC,DBG_LEVEL_NORMAL,
         ("Announcement already playing on channel %d\n", pCh->nChannel));
      /* errmsg: Announcement playout ongoing */
      RETURN_STATUS (VMMC_statusAnnActive);
   }

   /* check if announcement is already playing */
   if (IFX_NULL != pAnn->pCh)
   {
      TRACE(VMMC,DBG_LEVEL_NORMAL,
         ("Announcement %d already playing on channel %d\n",
         pStart->nAnnIdx, pAnn->pCh->nChannel));
      /* errmsg: Announcement playout ongoing */
      RETURN_STATUS (VMMC_statusAnnActive);
   }

   /* Check if coder is active */
   if (pCh->pCOD->fw_cod_ch_speech.EN != COD_CHAN_SPEECH_ENABLE)
   {
      /* errmsg: Coder channel not active */
      RETURN_STATUS (VMMC_statusCodAnnCodChanNotActive);
   }

   /* samaphore will be released by IFX_TAPI_EVENT_COD_ANNOUNCE_END event */
   VMMC_OS_MutexGet (&pAnn->semProtectAnn);
   /* markup the announcement resources as used */
   pAnn->pCh = pCh;
   /* store the handle of used announcement */
   pCh->pAnn = pAnn;

   /* start announcement */
   pCh->pCOD->fw_cod_ann_ctrl.E = COD_ANN_CTRL_E_START;
   pCh->pCOD->fw_cod_ann_ctrl.L =
      pStart->bLoop ? COD_ANN_CTRL_L_ON : COD_ANN_CTRL_L_OFF;
   pCh->pCOD->fw_cod_ann_ctrl.ADDR =
      (IFX_int32_t) KSEG1ADDR(pAnn->pAddress);

   ret = CmdWrite (pCh->pParent,
                   (IFX_uint32_t *)&pCh->pCOD->fw_cod_ann_ctrl,
                   COD_ANN_CTRL_LEN);
   if (!VMMC_SUCCESS(ret))
   {
      /* restore previous state */
      pCh->pCOD->fw_cod_ann_ctrl.E = COD_ANN_CTRL_E_STOP;
      pCh->pAnn = IFX_NULL;
      pAnn->pCh = IFX_NULL;
      VMMC_OS_MutexRelease (&pAnn->semProtectAnn);
      RETURN_STATUS (ret);
   }

   return VMMC_statusOk;
}

/**
   Stops playing announcment

   \param pLLCh         Pointer to Low-level channel structure

   \return
   VMMC_statusOk if successful else device specific return code.
   */
IFX_int32_t IFX_TAPI_LL_Ann_Stop (IFX_TAPI_LL_CH_t *pLLCh)
{
   VMMC_CHANNEL *pCh = (VMMC_CHANNEL *)pLLCh;
   IFX_int32_t ret = VMMC_statusOk;

   if (IFX_NULL == pLLCh)
      return VMMC_statusParam;

   if (IFX_NULL == pCh->pAnn)
   {
      /* Deactivated because this is not really an error. */
      return VMMC_statusOk;
   }

   /* stop announcement */
   pCh->pCOD->fw_cod_ann_ctrl.E = COD_ANN_CTRL_E_STOP;
   ret = CmdWrite (pCh->pParent,
      (IFX_uint32_t *)&pCh->pCOD->fw_cod_ann_ctrl, COD_ANN_CTRL_LEN);
   if (!VMMC_SUCCESS (ret))
   {
      /* restore real state, is not stopped */
      pCh->pCOD->fw_cod_ann_ctrl.E = COD_ANN_CTRL_E_START;
      RETURN_STATUS (ret);
   }

   /* mutex signals reception of annoucement end event, now it's possible
    * to free the annoucement */
   VMMC_OS_MutexGet (&pCh->pAnn->semProtectAnn);
   /* release the mutex, otherwise annoucement playout won't be possible */
   VMMC_OS_MutexRelease (&pCh->pAnn->semProtectAnn);

   /* after received event we are sure that announcement are not used */
   pCh->pAnn->pCh = IFX_NULL;

   /* markup channel as not active announcement client */
   pCh->pAnn = IFX_NULL;

   return VMMC_statusOk;
}

#endif /* VMMC_FEAT_ANNOUNCEMENTS */
