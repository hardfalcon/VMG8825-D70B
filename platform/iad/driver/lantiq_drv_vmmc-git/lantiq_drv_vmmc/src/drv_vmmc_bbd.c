/******************************************************************************

                              Copyright (c) 2013
                            Lantiq Deutschland GmbH
                             http://www.lantiq.com

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

*******************************************************************************/

/**
   \file drv_vmmc_bbd.c
*/

/* ============================= */
/* Includes                      */
/* ============================= */
#include "drv_vmmc_api.h"
#include "drv_vmmc_bbd.h"
#include "drv_vmmc_alm.h"
#include "drv_vmmc_alm_priv.h"      /* required to access bIdleExtNeedRestore */
#include "drv_mps_vmmc.h"

#include "ifxos_time.h"

/* ============================= */
/* Local defines                 */
/* ============================= */

#define VMMC_CMD_MOD_ECMD_MASK      0x1F00FF00
#define VMMC_WL_SDD_BASIC_CFG       0x04000400
#define VMMC_WL_SDD_RING_CFG        0x04000500
#define VMMC_WL_SDD_DCDC_CFG        0x04000C00
#define VMMC_WL_SDD_MWI_CFG         0x04000600

#define IDLE_EXT_TOGGLE_SLEEP_MS    5

/* maximum of downloads per BBD block,
   can be set also at compile time  */
#ifndef BBD_VMMC_BLOCK_MAXDWNLD
#define BBD_VMMC_BLOCK_MAXDWNLD              10
#endif /* BBD_VIN_BLOCK_MAXDWNLD */

/* ============================= */
/* BBD tag definitions           */
/* ============================= */
#define BBD_VMMC_MAGIC_XRX100                0x41523921 /* "AR9"  */
#define BBD_VMMC_MAGIC_XRX200                0x56523921 /* "VR9"  */
#define BBD_VMMC_MAGIC_XRX300                0x41523130 /* "AR10"  */
#define BBD_VMMC_MAGIC_FALCON                0x46414C43 /* "FALC"  */

/** BBD blocks tags */
/* 0x1XXX tags : DC/AC Coefficients */
#define BBD_COMPATIBILITY_BLOCK_TAG          0x000C
#define BBD_VMMC_CRAM_BLOCK                  0x1001
#define BBD_VMMC_SLIC_BLOCK                  0x1002
#define BBD_VMMC_RING_CFG_BLOCK              0x1003
#define BBD_VMMC_DC_THRESHOLDS_BLOCK         0x1004
#define BBD_VMMC_VOFW_COEFF                  0x1005

/** Transparent Download block */
#define BBD_VMMC_TRANSPARENT_BLOCK           0x1200

/** FXO CRAM block */
#define BBD_VMMC_FXO_CRAM_BLOCK              0x1010


/* ============================= */
/* Global Structures             */
/* ============================= */

/* ============================= */
/* Local variable definition     */
/* ============================= */

/* ============================= */
/* Global variable definition    */
/* ============================= */

/* registration of supported VMMC bbd blocks
   downloadable channelwise */
static const IFX_uint16_t VMMC_CH_BBD_Blocks[] =
{
   BBD_VMMC_CRAM_BLOCK,
   BBD_VMMC_VOFW_COEFF,
   BBD_VMMC_TRANSPARENT_BLOCK,
   BBD_VMMC_FXO_CRAM_BLOCK
};


/* ============================= */
/* Local function definition     */
/* ============================= */

static IFX_int32_t vmmc_BBD_CheckForSddDcDcConfig (
                        VMMC_CHANNEL *pCh,
                        VMMC_Msg_t *pMsg);

static IFX_int32_t vmmc_BBD_DownloadCh (
                        VMMC_CHANNEL *pCh,
                        bbd_format_t *pBBD);

static IFX_int32_t vmmc_BBD_VoFWCoeff (
                        VMMC_CHANNEL *pCh,
                        bbd_block_t *pBBDblock);

static IFX_int32_t vmmc_BBD_WhiteListedCmdWr (
                        VMMC_CHANNEL *pCh,
                        bbd_block_t *pBBDblock);

static IFX_int32_t vmmc_BBD_DownloadChCram  (
                        VMMC_CHANNEL *pCh,
                        bbd_block_t  *bbd_cram);

static IFX_int32_t vmmc_BBD_PrepareIdleExtBit (
                        VMMC_CHANNEL *pCh);

static IFX_int32_t vmmc_BBD_RestoreIdleExtBit (
                        VMMC_CHANNEL *pCh);

static IFX_int32_t vmmc_BBD_UpdateIdleExtBit (
                        VMMC_CHANNEL *pCh,
                        IFX_uint8_t val,
                        IFX_boolean_t *pbUpdated);

/* ============================= */
/* Debug functions               */
/* ============================= */

#ifdef VMMC_BBD_DEBUG
static IFX_void_t print_bbd_format(bbd_format_t *pBBD)
{
   IFX_uint32_t i;

   TRACE(VMMC, DBG_LEVEL_HIGH, ("pBBD->buf  = 0x%08X\n", pBBD->buf));
   TRACE(VMMC, DBG_LEVEL_HIGH, ("pBBD->size = %d byte\n", pBBD->size));

   for(i=0; i<pBBD->size; i++)
   {
      if( i!=0 && (i%8 == 0))
         TRACE(VMMC, DBG_LEVEL_HIGH, ("\n"));
      TRACE(VMMC, DBG_LEVEL_HIGH, ("%02X", pBBD->buf[i]));
   }
   return;
}

static IFX_void_t print_bbd_block(bbd_block_t *pBBD)
{
   IFX_uint32_t i;

   TRACE(VMMC, DBG_LEVEL_HIGH, ("pBBD->identifier = 0x%08X\n", pBBD->identifier));
   TRACE(VMMC, DBG_LEVEL_HIGH, ("pBBD->tag        = 0x%04X\n", pBBD->tag));
   TRACE(VMMC, DBG_LEVEL_HIGH, ("pBBD->version    = 0x%04X\n", pBBD->version));
   TRACE(VMMC, DBG_LEVEL_HIGH, ("pBBD->index      = 0x%04X\n", pBBD->index));
   TRACE(VMMC, DBG_LEVEL_HIGH, ("pBBD->size       = 0x%08X\n", pBBD->size));
   TRACE(VMMC, DBG_LEVEL_HIGH, ("pBBD->pData      = 0x%08X\n", pBBD->pData));
   for(i=0; i<pBBD->size; i++)
   {
      if( i!=0 && (i%8 == 0))
         TRACE(VMMC, DBG_LEVEL_HIGH, ("\n"));
      TRACE(VMMC, DBG_LEVEL_HIGH, ("%02X", pBBD->pData[i]));
   }
   TRACE(VMMC, DBG_LEVEL_HIGH, ("\n"));
   return;
}
#endif /* VMMC_BBD_DEBUG */


/**
   Check for the SDD DC/DC config message and analyse it.

   In combined DC/DC operation mode the DCcontrol of channel 0 supplies also
   channel 1.
   The DC/DC operation mode is stored on a per channel basis. Line operations
   are only possible after configuration of the channel.

   \param  pCh          Pointer to the VMMC channel structure.
   \param  pMsg         Pointer to FW command.

   \return
   - VMMC_statusOk if successful
   - VMMC_statusBbdDcDcReconfig
*/
static IFX_int32_t vmmc_BBD_CheckForSddDcDcConfig (VMMC_CHANNEL *pCh,
                                                   VMMC_Msg_t *pMsg)
{
   VMMC_DEVICE *pDev = pCh->pParent;
   IFX_int32_t ret = VMMC_statusOk;

   /* Analyse only SDD DC/DC config commands. */
   if ( (pMsg->cmd.CMD == CMD_SDD) &&
        (pMsg->cmd.MOD == MOD_SDD) &&
        (pMsg->cmd.ECMD == SDD_DcDcConfig_ECMD) &&
        (pMsg->cmd.LENGTH >= SDD_DcDcConfig_LEN))
   {
      VMMC_SDD_DcDcConfig_t *pSddDcDcCfg = (VMMC_SDD_DcDcConfig_t*)pMsg;
      enum VMMC_DCDC_TYPE nNewDcDcType;

      /* Writing of SDD DC/DC config must not be done more than once. So abort
         when configuration was already done. */
      if (pCh->pALM->nDcDcType != VMMC_DCDC_TYPE_DEFAULT_IBB)
      {
         TRACE(VMMC, DBG_LEVEL_LOW,
               ("VMMC INFO: Skipping SDD DC/DC reconfiguration\n"));
         /* errmsg: Ignoring already set SDD DC/DC config in BBD download. */
         return VMMC_statusBbdDcDcReconfig;
      }

      /* Block DC/DC type settings which do not match the configured HW type. */
      nNewDcDcType = (enum VMMC_DCDC_TYPE)
         (pSddDcDcCfg->DcDcHw + VMMC_DCDC_TYPE_IBB);
      if (nNewDcDcType != pDev->sdd.nAllowedDcDcType)
      {
         /* errmsg: DC/DC converter type in the BBD download must match the
            DC/DC HW type. */
         return VMMC_statusBbdDcDcHwDiffers;
      }
      /* Remember the type of DC/DC operation mode. In dedicated mode the
         DCcontrol works per channel so the type is stored per channel. */
      pCh->pALM->nDcDcType = nNewDcDcType;
      /* Set flag to start an automatic calibration after BBD download. */
      pCh->pALM->bCalibrationNeeded = IFX_TRUE;

      /* In case of combined mode set the type of DC/DC operation mode
         from channel A also to channel B. */
      if ((pDev->sdd.bDcDcHwCombined == IFX_TRUE) &&
          (pSddDcDcCfg->CHAN == VMMC_ALM_CMD_CHAN_A))
      {
         /* The combined DC/DC flag is only set on 2 channel systems.
            So there is always a channel 1. */
         pDev->pChannel[1].pALM->nDcDcType = pCh->pALM->nDcDcType;
         /* Set flag to start an automatic calibration after BBD download. */
         pDev->pChannel[1].pALM->bCalibrationNeeded = IFX_TRUE;

         /* Copy this SDD_DcDcConfig message to channel B to have
            a useful configuration there. */
         pSddDcDcCfg->CHAN = VMMC_ALM_CMD_CHAN_B;
         ret = CmdWrite (pCh->pParent, pMsg->val, pMsg->cmd.LENGTH);
         /* Correct the channel back to A. */
         pSddDcDcCfg->CHAN = VMMC_ALM_CMD_CHAN_A;
      }
   }
   return ret;
}

/* ============================= */
/* Global function definition    */
/* ============================= */

/**
   Download a BBD file.

   \param  pLLDev       Pointer to the VMMC device structure.
   \param  pProc        Pointer to low-level device initialization structure.

   \return
   - VMMC_statusOk if successful
*/
IFX_int32_t VMMC_TAPI_LL_BBD_Dnld(IFX_TAPI_LL_DEV_t *pLLDev,
                                  IFX_void_t const *pProc)
{
   VMMC_DEVICE *pDev = (VMMC_DEVICE*)pLLDev;
   VMMC_IO_INIT IoInit;
   IFX_int32_t ret = VMMC_statusOk;

   if (pProc == IFX_NULL)
   {
      /* reset all init pointers and init flag */
      memset (&IoInit, 0, sizeof (IoInit));
      /* set additional default flags */
      IoInit.nFlags = FW_AUTODWLD;
   }
   else
   {
      /* The init struct is specific for the LL driver and only known here.
         Because HL does not know the struct it cannot copy it and so it is
         copied here in the LL driver. */
      VMMC_OS_CpyUsr2Kern (&IoInit, pProc, sizeof (IoInit));
   }

   /* BBD Download */
   if(IoInit.pBBDbuf != IFX_NULL)
   {
      bbd_format_t bbdDwld;

      bbdDwld.buf  = IoInit.pBBDbuf;
      bbdDwld.size = IoInit.bbd_size;
      ret = VMMC_BBD_Download((VMMC_CHANNEL *) pDev, &bbdDwld);
      if (ret != IFX_SUCCESS)
      {
         TRACE(VMMC,DBG_LEVEL_HIGH, ("INFO: BBD Buffer Download failed.\n"));
      }
      else
      {
         TRACE(VMMC,DBG_LEVEL_LOW, ("INFO: BBD Buffer provided successfully.\n"));
      }
   }
   else
   {
      TRACE(VMMC,DBG_LEVEL_HIGH, ("INFO: No BBD Buffer provided.\n"));
   }

   RETURN_DEVSTATUS (ret);
}


/**
   Handle a BBD block.

   Internally this function dispatches processing of the blocks depending on
   the block type. This function is used within the context of a single channel.

   \param  pCh          Pointer to VMMC channel structure.
   \param  pBBDblock    Pointer to a kernel level copy of the bbd block.

   \return
   IFX_SUCCESS or IFX_ERROR
*/
static IFX_int32_t VMMC_BBD_BlockHandler(
                        VMMC_CHANNEL *pCh,
                        bbd_block_t *pBBDblock)
{
   IFX_int32_t                ret = IFX_SUCCESS;

   TRACE(VMMC, DBG_LEVEL_LOW,
         ("bbd block with tag 0x%04X passed\n", pBBDblock->tag));

   if (pCh->pALM->line_type_fxs != IFX_TRUE)
   {
      /* FXO line */

      /* For FXO line the only allowed blocks are FXO_CRAM and TRANSPARENT. */
      switch (pBBDblock->tag)
      {
         case  BBD_VMMC_FXO_CRAM_BLOCK:
            ret = vmmc_BBD_DownloadChCram (pCh, pBBDblock);
            break;
         case BBD_VMMC_TRANSPARENT_BLOCK:
            ret = vmmc_BBD_WhiteListedCmdWr(pCh, pBBDblock);
            break;
         default:
            TRACE (VMMC, DBG_LEVEL_LOW, ("VMMC device driver WARNING:"
                " unsupported block tag 0x%04X for FXO line%d. Skipped.\n",
                pBBDblock->tag, pCh->nChannel-1));
            break;
      }
   }
   else
   {
      /* FXS line */

      switch (pBBDblock->tag)
      {
         case  BBD_VMMC_CRAM_BLOCK:
            ret = vmmc_BBD_DownloadChCram (pCh, pBBDblock);
            break;

         case BBD_VMMC_VOFW_COEFF:
            vmmc_BBD_VoFWCoeff (pCh, pBBDblock);
            break;

         case BBD_VMMC_TRANSPARENT_BLOCK:
            ret = vmmc_BBD_WhiteListedCmdWr(pCh, pBBDblock);
            break;

         default:
            TRACE (VMMC, DBG_LEVEL_LOW, ("VMMC device driver WARNING:"
                " unsupported block tag 0x%04X for FXS line%d. Skipped.\n",
                pBBDblock->tag, pCh->nChannel-1));
            /* parsing continues */
            break;
      }
   } /* if */
   return ret;
}


/**
   Does a BBD download.

   Handles the IOCTL FIO_VMMC_BBD_DOWNLOAD to download a BBD buffer on a
   given channel or a given device. For a device the download is done on
   each of the channels.

   \param  pCh          Pointer to VMMC channel or VMMC device structure.
   \param  pBBD         Pointer to user BBD buffer.

   \return
   IFX_SUCCESS or error code.
*/
IFX_int32_t VMMC_BBD_Download (VMMC_CHANNEL *pCh, bbd_format_t *pBBD)
{
   IFX_int32_t       ret = IFX_SUCCESS;
   bbd_error_t       bbd_err;
   bbd_format_t      bbd;

   /* initializations */
   memset (&bbd, 0, sizeof(bbd));

   /* make a local copy of the BBD buffer */
   if (pBBD->buf != IFX_NULL)
   {
      /* get local memory for bbd buffer */
      bbd.buf  = VMMC_OS_MapBuffer(pBBD->buf, pBBD->size);
      if (bbd.buf == NULL)
         return IFX_ERROR;
      /* set size */
      bbd.size = pBBD->size;
      /* check BBD Buffer integrity */
      switch (ifx_mps_chip_family)
      {
         case MPS_CHIP_XRX100:
            bbd_err = bbd_check_integrity (&bbd, BBD_VMMC_MAGIC_XRX100);
            break;
         case MPS_CHIP_XRX200:
            bbd_err = bbd_check_integrity (&bbd, BBD_VMMC_MAGIC_XRX200);
            break;
         case MPS_CHIP_XRX300:
            bbd_err = bbd_check_integrity (&bbd, BBD_VMMC_MAGIC_XRX300);
            break;
         case MPS_CHIP_FALCON:
            bbd_err = bbd_check_integrity (&bbd, BBD_VMMC_MAGIC_FALCON);
            break;
         default:
            VMMC_OS_UnmapBuffer(bbd.buf);
            TRACE (VMMC, DBG_LEVEL_HIGH,
                   ("VMMC: BBD ID does not match the platform ID.\n"));
            return IFX_ERROR;
      }
      if (bbd_err != BBD_INTG_OK)
      {
         VMMC_OS_UnmapBuffer(bbd.buf);
         TRACE (VMMC, DBG_LEVEL_HIGH,
                ("VMMC: BBD download fail (0x%X)\n", bbd_err));
         return IFX_ERROR;
      }
   }
   else
   {
      /* errmsg: At least one parameter is wrong  */
      RETURN_STATUS(VMMC_statusParam);
   }

   /* detect channel or device context */
   if (pCh->nChannel != 0)
   {
      /* channel context - download on one channel only */
      ret = vmmc_BBD_DownloadCh (pCh, &bbd);
   }
   else
   {
      /* device context - downloads on all _analog_ channels */
      VMMC_DEVICE  *pDev = /*lint --e(826)*/ (VMMC_DEVICE *) pCh;
      IFX_uint16_t nCh;

      for (nCh = 0; VMMC_SUCCESS(ret) && (nCh < VMMC_MAX_CH_NR); nCh++)
      {
         pCh = &pDev->pChannel[nCh];
         if (pCh->pALM != IFX_NULL)
         {
            ret = vmmc_BBD_DownloadCh (pCh, &bbd);
         }
      }
   }

   VMMC_OS_UnmapBuffer(bbd.buf);

   return ret;
}


/**
   Does a BBD download on the given channel

   This function just handles one channel only. The channel must have an ALM
   or this function will return with parameter error.

   \param  pCh          Pointer to VMMC channel or VMMC device structure.
   \param  pBBD         Pointer to user BBD buffer.

   \return
   IFX_SUCCESS or IFX_ERROR
*/
IFX_int32_t vmmc_BBD_DownloadCh (VMMC_CHANNEL *pCh, bbd_format_t *pBBD)
{
   IFX_int32_t       ret = IFX_SUCCESS;
   IFX_uint32_t      i, j, block_num;
   bbd_block_t       bbd_vmmc_block;
   VMMC_DEVICE       *pDev = IFX_NULL;

   /* currently the download only covers ALM channels - if the
      application tries to download on a channel which doesn't
      have an ALM we return with error. */
   if (pCh->pALM == IFX_NULL)
   {
      /* errmsg: Resource not valid. Channel number out of range */
      RETURN_STATUS(VMMC_statusInvalCh);
   }
   pDev = pCh->pParent;

   /* Save the setting of the idleExt bit and clear it to wake the SDD. */
   ret = vmmc_BBD_UpdateBasicConfigCache (pCh);
   if (VMMC_SUCCESS(ret))
   {
      ret = vmmc_BBD_PrepareIdleExtBit (pCh);
   }
   if (ret != IFX_SUCCESS)
   {
      TRACE (VMMC, DBG_LEVEL_HIGH,
            ("VMMC: BBD download: IdleExt bit reset fail\n"));
      return IFX_ERROR;
   }
   /* sleep at least 5ms */
   IFXOS_MSecSleep(IDLE_EXT_TOGGLE_SLEEP_MS);

   /* initializations */
   memset (&bbd_vmmc_block, 0, sizeof (bbd_vmmc_block));

   /* Go through the bbd buffer and download any blocks of relevance found.
      VMMC_CH_BBD_Blocks is a constant array of 16 bit entries defining
      the different supported block types. */
   block_num  = (sizeof (VMMC_CH_BBD_Blocks) / sizeof (IFX_uint16_t));
   switch (ifx_mps_chip_family)
   {
      case MPS_CHIP_XRX100:
         bbd_vmmc_block.identifier = BBD_VMMC_MAGIC_XRX100;
         break;
      case MPS_CHIP_XRX200:
         bbd_vmmc_block.identifier = BBD_VMMC_MAGIC_XRX200;
         break;
      case MPS_CHIP_XRX300:
         bbd_vmmc_block.identifier = BBD_VMMC_MAGIC_XRX300;
         break;
      case MPS_CHIP_FALCON:
         bbd_vmmc_block.identifier = BBD_VMMC_MAGIC_FALCON;
         break;
      default:
         /* The error is already caught above in the download function. */
         break;
   }
   for (i = 0; i < block_num; i++)
   {
      /* The order of block search and handling is defined in
         const VMMC_CH_BBD_Blocks array. */
      bbd_vmmc_block.tag = VMMC_CH_BBD_Blocks[i];

      /* sniff blocks of this tag up to maximum allowed number */
      for (j = 0; j < BBD_VMMC_BLOCK_MAXDWNLD; j++)
      {
         /* look at block of this index and download it if available */
         bbd_vmmc_block.index = j;
         bbd_get_block (pBBD, &bbd_vmmc_block);
         if ((bbd_vmmc_block.pData == NULL) || (bbd_vmmc_block.size == 0))
         {
            /* skip further search for this blocktype */
            break;
         }

         /* do the download on a single channel */
         ret = VMMC_BBD_BlockHandler(pCh, &bbd_vmmc_block);

         /* stop everything if the previous download went wrong */
         if (ret != IFX_SUCCESS)
            break;
      }
      /* stop everything if the previous download went wrong */
      if (ret != IFX_SUCCESS)
         break;
   }

   /* Restore the idleExt bit. */
   if (VMMC_SUCCESS(ret))
   {
      ret = vmmc_BBD_UpdateBasicConfigCache (pCh);
   }
   if (VMMC_SUCCESS(ret))
   {
      ret = vmmc_BBD_RestoreIdleExtBit (pCh);
   }
   if (ret != IFX_SUCCESS)
   {
      TRACE (VMMC, DBG_LEVEL_HIGH,
                   ("VMMC: BBD download: IdleExt bit restore fail\n"));
      return IFX_ERROR;
   }

   if (VMMC_SUCCESS(ret))
   {
      /* Do automatically a calibration on the channel on which the BBD was
         downloaded to adapt to new coefficients. */
      if (pDev->sdd.bDcDcHwCombined != IFX_TRUE)
      {
         if (pCh->pALM->bCalibrationNeeded == IFX_TRUE)
         {
            ret = VMMC_ALM_Calibration (pCh);
         }
      }
      else
      {
         for (i=0; (i < pDev->caps.nALI) && VMMC_SUCCESS(ret); i++)
         {
            pCh = &pDev->pChannel[i];

            if (pCh->pALM->bCalibrationNeeded == IFX_TRUE)
            {
               ret = VMMC_ALM_Calibration (pCh);
            }
         }
      }

      if (!VMMC_SUCCESS(ret))
      {
         /* errmsg: Automatic calibration after BBD download failed. */
         ret = VMMC_statusAutomaticCalibrationFailed;
      }
   }

   return ret;
}


/**
   Download VoFW Coefficients on channel

   \param  pCh          Pointer to VMMC channel structure.
   \param  pBBDblock    Pointer to BBD coefficient block.

   \return
   - VMMC_statusBBDError Writing the command of one parameter of the
     BBD file failed.
   - VMMC_statusOk if successful.
*/
IFX_int32_t vmmc_BBD_VoFWCoeff (VMMC_CHANNEL *pCh, bbd_block_t *pBBDblock)
{
   VMMC_Msg_t Msg;
   IFX_int32_t  ret = VMMC_statusOk;
   IFX_uint32_t cnt = 0, j;

   /* handle all messages in the buffer */
   do
   {
      Msg.val[0] = ntohl (*((IFX_uint32_t*)/*lint --e(826)*/&pBBDblock->pData[cnt]));
      for (j=4; j < Msg.cmd.LENGTH + CMD_HDR_CNT; j+= 4)
      {
         Msg.val[j/4] = ntohl (*((IFX_uint32_t*)/*lint --e(826)*/&pBBDblock->pData[cnt + j]));
      }
      /* go to the next message in 32 bit steps */
      cnt += Msg.cmd.LENGTH + CMD_HDR_CNT;
      if (cnt > pBBDblock->size)
      {
         return VMMC_statusOk;
      }
      if (Msg.cmd.MOD != MOD_RESOURCE)
      {
         /* errmsg: The BBD content for VoFW coefficients is invalid.
            Only resource coefficients are allowed */
         RETURN_STATUS (VMMC_statusBBDviolation);
      }
      /* Store LEC coefficients */
      if (Msg.cmd.ECMD == RES_LEC_COEF_ECMD)
      {
         /* VMMC_ALM_LecCoeffUpdate (pCh, &Msg.res_LecCoef); */
      }
      else
      {
         ret = CmdWrite (pCh->pParent, Msg.val, Msg.cmd.LENGTH);
      }
   }
   while (VMMC_SUCCESS(ret) && (cnt < pBBDblock->size));

   RETURN_STATUS (ret);
}


/**
   Transparent "white-listed" CmdWrite

   \param  pCh          Pointer to VMMC channel structure.
   \param  pBBDblock    Pointer to BBD coefficient block.

   \return
   - VMMC_statusBBDError Writing the command of one parameter of the
     BBD file failed.
   - VMMC_statusOk if successful.
*/
static IFX_int32_t vmmc_BBD_WhiteListedCmdWr(VMMC_CHANNEL *pCh,
                                             bbd_block_t *pBBDblock)
{
   VMMC_Msg_t   Msg;
   IFX_int32_t  ret = VMMC_statusOk;
   IFX_uint32_t i;

   /* endianess conform header */
   Msg.val[0]   = ntohl (*((IFX_uint32_t*)/*lint --e(826)*/&pBBDblock->pData[0]));
   Msg.cmd.CHAN = pCh->nChannel - 1;

   /* check pBBDblock->size */

   /* endianess conform data */
   for (i=4; i < Msg.cmd.LENGTH + 4; i+= 4)
   {
      Msg.val[i/4] = ntohl (*((IFX_uint32_t*)/*lint --e(826)*/&pBBDblock->pData[i]));
   }

   /* check if the block contains a FW message from the "whitelist",
      which is defined by the msg header fields CMD, MOD and ECMD */
   switch (Msg.val[0] & VMMC_CMD_MOD_ECMD_MASK)
   {
      case VMMC_WL_SDD_BASIC_CFG:
         {
            {
               VMMC_SDD_BasicConfig_t *pSDD_BasicConfig =
                                      (VMMC_SDD_BasicConfig_t *)&Msg;
               /* patch IdleExt bit */
               if(1 == pSDD_BasicConfig->IdleExt)
               {
                  pCh->pALM->bIdleExtNeedRestore = IFX_TRUE;
                  pSDD_BasicConfig->IdleExt = 0;
               }
               else
               {
                  pCh->pALM->bIdleExtNeedRestore = IFX_FALSE;
               }
            }
         }
      case VMMC_WL_SDD_RING_CFG:
      case VMMC_WL_SDD_MWI_CFG:
         ret = CmdWrite (pCh->pParent, Msg.val, Msg.cmd.LENGTH);
         break;

      case VMMC_WL_SDD_DCDC_CFG:
         if (pCh->pALM->line_type_fxs == IFX_TRUE)
         {
            /* Only on FXS lines analyse the command and if it contains a
               SDD DC/DC message certain actions are executed. The command
               may be modified in this function. */
            ret = vmmc_BBD_CheckForSddDcDcConfig (pCh, &Msg);
         }

         if (ret == VMMC_statusOk)
         {
            ret = CmdWrite (pCh->pParent, Msg.val, Msg.cmd.LENGTH);
         }

         /* This is not a real error. Just the CmdWrite needed to be skipped.
            But for the entire BBD download this is success. */
         if (ret == VMMC_statusBbdDcDcReconfig)
            ret = VMMC_statusOk;
         break;

      default:
         /* if command is not whitelisted, ignore it */
         TRACE(VMMC, DBG_LEVEL_HIGH,
              ("VMMC BBD ignoring non whitelisted block:\n"));
         for (i=0; i<= Msg.cmd.LENGTH/4; i++)
         {
            TRACE(VMMC, DBG_LEVEL_HIGH,("VMMC BBD %% %08X\n", Msg.val[i]));
         }
   }

   RETURN_STATUS (ret);
}

/**
   Download CRAM Coefficients on channel

   \param  pCh          Pointer to VMMC channel structure.
   \param  pBBDblock    Pointer to BBD CRAM block.

   \return
   IFX_SUCCESS or IFX_ERROR

   \remarks
   - It is assumed that the given CRAM bbd block is valid and that the data
     representation in the bbd block is according to BBD specification,
     as follows:

     ofset_16 : 0xXX, 0xXX,
     data_16[]: 0xXX, 0xXX,
                0xXX, 0xXX,
                ...
     crc_16   : 0xXX, 0xXX
*/
static IFX_int32_t vmmc_BBD_DownloadChCram (VMMC_CHANNEL *pCh,
                                            bbd_block_t  *bbd_cram)
{
   IFX_int32_t  ret = IFX_ERROR;
   IFX_uint32_t countWords;
   IFX_uint32_t posBytes = 0;
   IFX_uint8_t  lenBytes, *pByte;
   IFX_uint8_t  padBytes = 0;
   IFX_uint16_t cram_offset, cram_crc,
                pCmd [MAX_CMD_WORD]
#if defined (__GNUC__) || defined (__GNUG__)
                   __attribute__ ((aligned(4)))
#endif
                   = {0};

   /* read offset */
   cpb2w (&cram_offset, &bbd_cram->pData[0], sizeof (IFX_uint16_t));
   /* set CRAM payload pointer */
   pByte = &bbd_cram->pData[2];
   /* set CRAM data size in words, removing offset and crc bytes */
   countWords = (bbd_cram->size - 4) >> 1 ;

   /* SDD_Coef command */
   pCmd[0] = (0x0400) | (pCh->nChannel - 1);
   pCmd[1] = (0x0D00);

   /* write CRAM data */
   while (countWords > 0)
   {
      /* calculate length to download (in words = 16bit),
         maximum allowed length for this message is 56 Bytes = 28 Words */
      if (countWords > ((MAX_CMD_WORD - CMD_HDR_CNT - 1)))
         lenBytes = (MAX_CMD_WORD - CMD_HDR_CNT - 1) << 1;
      else
         lenBytes = countWords << 1;
      /* set CRAM offset in CMD2 */
      pCmd[2] = ((cram_offset + (posBytes >> 1)) << 5) | lenBytes>>1;
      /* set CRAM data while taking care of endianess  */
      cpb2w (&pCmd[3], &pByte[posBytes], lenBytes);
      /* the download length is a multiple of 16 bit, nevertheless we
         have to be 32bit aligned */
      if ((lenBytes%4))
      {
         padBytes = 0;
      }
      else
      {
         /* add padding word */
         pCmd[3+(lenBytes>>1)] = 0x0000;
         padBytes = 2;
      }
      /*printk("o=%d l=%d p=%d\n", (cram_offset + (posBytes >> 1)), lenBytes>>1, padBytes>>1);*/

      /* write Data */
#if 1
      /* lenBytes + 2 bytes for block offset/length which are not calculated
         in the download progress */
      ret = CmdWrite (pCh->pParent, (IFX_uint32_t *) pCmd, lenBytes+padBytes+2);
#else
#warning temp CRAM download just traces
   pCmd[1] &= ~0x00FF;
   pCmd[1] |= lenBytes+2;
   printk(KERN_DEBUG "cram ");
   {int i;
   for (i=0; i<= (4 /*header*/+lenBytes+padBytes)/2; i++) /* i in dwords */
   {
      printk(KERN_DEBUG "%04X", pCmd[i]);
      if (i%2 && i!=0) printk(KERN_DEBUG " ");
   }
   printk(KERN_DEBUG "\n");
   }
   ret = IFX_SUCCESS;
#endif /* 0 */

      if (ret != IFX_SUCCESS)
         break;
      /* \todo eventually call crc calculation routine here */
      /* increment position */
      posBytes += lenBytes;
      /* decrement count */
      countWords -= (lenBytes >> 1);
   }
   /* In case the download went through, read CRC from buffer and do checks */
   if (ret == IFX_SUCCESS)
   {
      cpb2w (&cram_crc, &pByte [posBytes], sizeof (IFX_uint16_t));
      /* \todo Check download CRAM CRC here, either by comparison or by
               reading it back from chip */
   }
   return ret;
}

/**
   Update IdleExt bit of SDD_BasicConfig on the given channel.

   \param pCh           Pointer to the VMMC channel structure.
   \param val           Value of IdleExtBit of SDD_BasicConfig command.
   \param pbUpdated     Completion indicator

   \return
   IFX_SUCCESS or IFX_ERROR
*/
static IFX_int32_t vmmc_BBD_UpdateIdleExtBit (VMMC_CHANNEL *pCh,
                                              IFX_uint8_t val,
                                              IFX_boolean_t *pbUpdated)
{
   IFX_int32_t            ret = IFX_SUCCESS;
   VMMC_DEVICE            *pDev;
   VMMC_SDD_BasicConfig_t *pSDD_BasicConfig;

   /* sanity check */
   if (pCh == IFX_NULL || pCh->pALM == IFX_NULL || val > 1)
   {
      return IFX_ERROR;
   }
   pDev = pCh->pParent;
   pSDD_BasicConfig = &pCh->pALM->fw_sdd_basic_config;

   /* default result init */
   if (IFX_NULL != pbUpdated)
   {
      *pbUpdated = IFX_FALSE;
   }

   /* if update is required - write down to FW */
   if (val != pSDD_BasicConfig->IdleExt)
   {
      VMMC_OS_MutexGet (&pCh->chAcc);
      pSDD_BasicConfig->IdleExt = val;
      ret = CmdWrite (pDev, (IFX_uint32_t *)((IFX_void_t *)pSDD_BasicConfig),
                      SDD_BasicConfig_LEN);
      VMMC_OS_MutexRelease (&pCh->chAcc);

      if (ret == IFX_SUCCESS && IFX_NULL != pbUpdated)
      {
         *pbUpdated = IFX_TRUE;
      }
   }

   return ret;
}

/**
   Prepare IdleExt bit of SDD_BasicConfig on given channel.

   This function clears the IdleExt bit in the SDD_BasicConfig command. This is
   required to ensure that SLIC is not in a sleep mode when being programmed.

   \param pCh           Pointer to the VMMC channel structure.

\return
   IFX_SUCCESS or IFX_ERROR
*/
static IFX_int32_t vmmc_BBD_PrepareIdleExtBit (VMMC_CHANNEL *pCh)
{
   IFX_int32_t ret;
   IFX_boolean_t bUpdated;

   /* reset IdleExt bit of SDD_BasicConfig to ensure that SLIC
      is not in a sleep mode */
   ret = vmmc_BBD_UpdateIdleExtBit (pCh, 0, &bUpdated);
   if (ret == IFX_SUCCESS && bUpdated == IFX_TRUE)
   {
      /* IdleExt bit is now reset to 0.
         It may need or may not need restoration to 1 after BBD
         download finish. This depends on BBD file content, whether
         the BBD file has SDD_BasicConfig command(s) with IdleExt==0.
         The below boolean indicates whether the restoration is needed.
         This indicator can be altered by transparent BBD block
         processing routine. */
      pCh->pALM->bIdleExtNeedRestore = IFX_TRUE;
   }

   return ret;
}

/**
   Restore IdleExt bit of SDD_BasicConfig on given channel.

   This functions restores IdleExt bit to 1 if bIdleExtNeedRestore is true.

   \param pCh           Pointer to the VMMC channel structure.

\return
   IFX_SUCCESS or IFX_ERROR
*/
static IFX_int32_t vmmc_BBD_RestoreIdleExtBit (VMMC_CHANNEL *pCh)
{
   IFX_int32_t ret = IFX_SUCCESS;

   if (IFX_TRUE == pCh->pALM->bIdleExtNeedRestore)
   {
      ret = vmmc_BBD_UpdateIdleExtBit (pCh, 1, IFX_NULL);
   }

   return ret;
}

/**
   Update SDD_BasicConfig command cache on given channel.

   \param pCh           Pointer to the VMMC channel structure.

\return
   IFX_SUCCESS or IFX_ERROR
*/
IFX_int32_t vmmc_BBD_UpdateBasicConfigCache (VMMC_CHANNEL *pCh)
{
   IFX_int32_t            ret;
   VMMC_DEVICE            *pDev = pCh->pParent;
   VMMC_SDD_BasicConfig_t *pSDD_BasicConfig = &pCh->pALM->fw_sdd_basic_config;

   /* protect fw msg */
   VMMC_OS_MutexGet (&pCh->chAcc);
   ret = CmdRead (pDev, (IFX_uint32_t *)((IFX_void_t *)pSDD_BasicConfig),
                        (IFX_uint32_t *)((IFX_void_t *)pSDD_BasicConfig),
                        SDD_BasicConfig_LEN);
   VMMC_OS_MutexRelease (&pCh->chAcc);

   return ret;
}
