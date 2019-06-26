/******************************************************************************

                          Copyright (c) 2007-2015
                     Lantiq Beteiligungs-GmbH & Co. KG

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/* ==========================================================================
   Description : VR9/VR10/AR9 Digital Spectrum Management functions.
   ========================================================================== */


/* ============================================================================
   Inlcudes
   ========================================================================= */

/* get at first the driver configuration */
#include "drv_mei_cpe_config.h"

#if (MEI_SUPPORT_DSM == 1)

#include "ifx_types.h"
#include "drv_mei_cpe_os.h"
#include "drv_mei_cpe_dbg.h"

#include "drv_mei_cpe_dbg_driver.h"

#include "drv_mei_cpe_interface.h"
#include "drv_mei_cpe_mei_interface.h"
#include "drv_mei_cpe_api.h"
#include "drv_mei_cpe_mei_access.h"

/* CMV messages */
#include "cmv_message_format.h"

#include "drv_mei_cpe_download.h"
#include "drv_mei_cpe_msg_process.h"
#include "drv_mei_cpe_dsm.h"

/* ==========================================================================
   Global Variable Definitions
   ========================================================================== */
/** protection - lock to garantee unique access MEI driver entities to the
 *  exclusive PP driver call back function */
MEI_DRVOS_sema_t pCallBackFuncAccessLock;

/**
   Init DSM Vectoring ERB buf

\param
    pMeiDev       private device data
\param
   erb_buf_size   Error Reported Block size

\return
   IFX_SUCCESS
   IFX_ERROR
*/
IFX_int32_t MEI_VRX_DSM_ErbAlloc(MEI_DEV_T *pMeiDev, IFX_uint32_t erb_buf_size)
{
   IFX_int32_t ret = 0;
#if MEI_SUPPORT_DEVICE_VR11 != 1

   pMeiDev->meiERBbuf.pERB_virt = NULL;
   pMeiDev->meiERBbuf.pERB_phy = NULL;
   pMeiDev->meiERBbuf.nERBsize_byte = 0;
   pMeiDev->meiERBbuf.pCallBackFunc = NULL;

   /* allocate continuous memory */
   pMeiDev->meiERBbuf.pERB_virt = (IFX_uint8_t *)MEI_DRVOS_DMA_Malloc(
                                   MEI_DEVICE_CFG_VALUE_GET(dev), erb_buf_size,
                                   (MEI_DRVOS_DMA_T *)&pMeiDev->meiERBbuf.pERB_phy);

   if (!pMeiDev->meiERBbuf.pERB_virt)
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DRV[0x%02X]: no memory for Error Reported Block size %d" MEI_DRV_CRLF,
              MEI_DRV_LINENUM_GET(pMeiDev), erb_buf_size));
      return -e_MEI_ERR_NO_MEM;
   }
   pMeiDev->meiERBbuf.nERBsize_byte = erb_buf_size;

   PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL,
            ("MEI_DRV: ERB block addr = 0x%08X, size = %d [byte]" MEI_DRV_CRLF,
            (IFX_uint32_t)pMeiDev->meiERBbuf.pERB_virt, pMeiDev->meiERBbuf.nERBsize_byte));

   /* init the open instance mutex (semaphore) */
   MEI_DRVOS_SemaphoreInit(&pCallBackFuncAccessLock);
#endif

   return ret;
}

/**
   Release DSM Vectoring ERB buf

\param
    pMeiDev       private device data

\return
   NONE
*/
IFX_void_t MEI_VRX_DSM_ErbFree(MEI_DEV_T *pMeiDev)
{
#if MEI_SUPPORT_DEVICE_VR11 != 1
   if (pMeiDev->meiERBbuf.pERB_virt)
   {
      MEI_DRVOS_DMA_Free(MEI_DEVICE_CFG_VALUE_GET(dev),
                         pMeiDev->meiERBbuf.nERBsize_byte,
                         pMeiDev->meiERBbuf.pERB_virt,
                         (MEI_DRVOS_DMA_T)pMeiDev->meiERBbuf.pERB_phy);
   }

   pMeiDev->meiERBbuf.pERB_virt = NULL;
   pMeiDev->meiERBbuf.pERB_phy = NULL;
   pMeiDev->meiERBbuf.nERBsize_byte = 0;

   /* mutex exist */
   MEI_DRVOS_SemaphoreDelete(&pCallBackFuncAccessLock);
#endif /* (MEI_SUPPORT_DEVICE_VR11 != 1) */
}

/**
   Init DSM related data

\param
    pMeiDev       private device data

\return
   NONE
*/
IFX_void_t MEI_VRX_DSM_DataInit(MEI_DEV_T *pMeiDev)
{
   memset((IFX_uint8_t *)&pMeiDev->meiDsmStatistic, 0x00, sizeof(IOCTL_MEI_dsmStatistics_t));
   memset((IFX_uint8_t *)&pMeiDev->meiDsmConfig, 0x00, sizeof(IOCTL_MEI_dsmConfig_t));
   pMeiDev->bDsmConfigInit = IFX_FALSE;
   memset((IFX_uint8_t *)&pMeiDev->meiMacConfig, 0x00, sizeof(IOCTL_MEI_MacConfig_t));
#if MEI_SUPPORT_DEVICE_VR11 != 1
   memset((IFX_uint8_t *)(pMeiDev->meiERBbuf.pERB_virt), 0x00, 4);
#endif
#if (MEI_DBG_DSM_PROFILING == 1)
   memset((IFX_uint8_t *)pMeiDev->meiDbgProfilingData, 0x00, sizeof(pMeiDev->meiDbgProfilingData));
   pMeiDev->bErbReset = IFX_TRUE;
#endif

   pMeiDev->nFwVectorSupport = e_MEI_DSM_VECTOR_FW_SUPPORT_MODE_NONE;
   memset((IFX_uint8_t *)&pMeiDev->firmwareFeatures, 0x00, sizeof(IOCTL_MEI_firmwareFeatures_t));
   pMeiDev->meiFwDlCount = 0;

#if MEI_SUPPORT_DEVICE_VR11 != 1
   pMeiDev->meiERBbuf.pCallBackFunc = mei_dsm_cb_func;
   PRN_DBG_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL,
            ("MEI_DRV: PP callback function addr = 0x%08X" MEI_DRV_CRLF,
            (IFX_uint32_t)(pMeiDev->meiERBbuf.pCallBackFunc)));
#endif
}

/**
   Send firmware message with DsmConfig options

\param
   pMeiDynCntrl   - private dynamic device data (per open instance)

\param
   pDsmConfig     - configure options

\return
   IFX_SUCCESS    Success
   IFX_ERROR      in case of error
*/
IFX_int32_t MEI_VRX_DSM_ControlSet(MEI_DYN_CNTRL_T *pMeiDynCntrl, IOCTL_MEI_dsmConfig_t *pDsmConfig)
{
   IFX_int32_t ret = 0;
   CMD_DSM_Control_t sCmd;
   ACK_DSM_Control_t sAck;

   memset(&sCmd, 0x00, sizeof(sCmd));
   memset(&sAck, 0x00, sizeof(sAck));
   sCmd.Length = 1;

   sCmd.VectoringMode = (IFX_uint16_t)(pDsmConfig->eVectorControl);

   ret = MEI_InternalSendMessage(pMeiDynCntrl, CMD_DSM_CONTROL,
                                 sizeof(sCmd), (unsigned char *)&sCmd,
                                 sizeof(sAck), (unsigned char *)&sAck);
   return ret;
}

/**
   Send firmware message with DsmConfig options

\param
   pMeiDynCntrl   - private dynamic device data (per open instance)

\param
   pDsmStatus     - configure options

\return
   IFX_SUCCESS    Success
   IFX_ERROR      in case of error
*/
IFX_int32_t MEI_VRX_DSM_StatusGet(MEI_DYN_CNTRL_T *pMeiDynCntrl, IOCTL_MEI_dsmStatus_t *pDsmStatus)
{
   IFX_int32_t ret = 0;
   CMD_HS_SelectedProfileVDSL2Get_t sCmd;
   ACK_HS_SelectedProfileVDSL2Get_t sAck;

   memset(&sCmd, 0x00, sizeof(sCmd));
   memset(&sAck, 0x00, sizeof(sAck));
   sCmd.Length = 1;

   ret = MEI_InternalSendMessage(pMeiDynCntrl, CMD_HS_SELECTEDPROFILEVDSL2GET,
                                 sizeof(sCmd), (unsigned char *)&sCmd,
                                 sizeof(sAck), (unsigned char *)&sAck);
   if (ret < 0)
   {
      return ret;
   }

   /* Downstream and upstream Vectoring */
   if (sAck.dsmSel0)
   {
      pDsmStatus->eVectorStatus = e_MEI_VECTOR_STAT_ON_DS_US;
   }
   /* Downstream-only Vectoring. */
   else if (sAck.dsmSel1)
   {
      pDsmStatus->eVectorStatus = e_MEI_VECTOR_STAT_ON_DS;
   }
   else
   {
      pDsmStatus->eVectorStatus = e_MEI_VECTOR_STAT_OFF;
   }

   /* Vector-friendly downstream + upstream operation */
   if (sAck.dsmSel2)
   {
      pDsmStatus->eVectorFriendlyStatus = e_MEI_VECTOR_FRIENDLY_STAT_ON_DS_US;
   }
   /* Vector-friendly downstream (only) operation */
   else if (sAck.dsmSel3)
   {
      pDsmStatus->eVectorFriendlyStatus = e_MEI_VECTOR_FRIENDLY_STAT_ON_DS;
   }
   else
   {
      pDsmStatus->eVectorFriendlyStatus = e_MEI_VECTOR_FRIENDLY_STAT_OFF;
   }

   return ret;
}

/**
   Send firmware message with DsmConfig options

\param
   pMeiDynCntrl   - private dynamic device data (per open instance)

\param
   pMacConfig     - configure options

\return
   IFX_SUCCESS    Success
   IFX_ERROR      in case of error
*/
IFX_int32_t MEI_VRX_DSM_MacConfigSet(MEI_DYN_CNTRL_T *pMeiDynCntrl, IOCTL_MEI_MacConfig_t *pMacConfig)
{
   IFX_int32_t ret = 0;
   CMD_MAC_FrameConfigure_t sCmd;
   ACK_MAC_FrameConfigure_t sAck;

   memset(&sCmd, 0x00, sizeof(sCmd));
   memset(&sAck, 0x00, sizeof(sAck));
   sCmd.Length = 3;

   sCmd.SrcMacAddrB0_1 = pMacConfig->nMacAddress[1] | (IFX_uint16_t)(pMacConfig->nMacAddress[0]) << 8;
   sCmd.SrcMacAddrB2_3 = pMacConfig->nMacAddress[3] | (IFX_uint16_t)(pMacConfig->nMacAddress[2]) << 8;
   sCmd.SrcMacAddrB4_5 = pMacConfig->nMacAddress[5] | (IFX_uint16_t)(pMacConfig->nMacAddress[4]) << 8;

   ret = MEI_InternalSendMessage(pMeiDynCntrl, CMD_MAC_FRAMECONFIGURE,
                                 sizeof(sCmd), (unsigned char *)&sCmd,
                                 sizeof(sAck), (unsigned char *)&sAck);

   return ret;
}

/**
   Get dsm statistics via firmware message

\param
   pMeiDynCntrl         - private dynamic device data (per open instance)

\param
   pDsmStatistics       - pointer to DSM stat structure

\return
   IFX_SUCCESS    Success
   IFX_ERROR      in case of error
*/
IFX_int32_t MEI_VRX_DSM_StatsGet(MEI_DYN_CNTRL_T *pMeiDynCntrl,
                                 IOCTL_MEI_dsmStatistics_t *pDsmStatistics)
{
   IFX_int32_t ret = 0;
   CMD_DSM_StatsGet_t sCmd;
   ACK_DSM_StatsGet_t sAck;

   memset(&sCmd, 0x00, sizeof(sCmd));
   memset(&sAck, 0x00, sizeof(sAck));

   if (MEI_DEVICE_CFG_IS_PLATFORM(e_MEI_DEV_PLATFORM_CONFIG_VR11))
   {
      sCmd.Length = 6;
   }
   else
   {
      sCmd.Length = 2;
   }

   ret = MEI_InternalSendMessage(pMeiDynCntrl, CMD_DSM_STATSGET,
                                 sizeof(sCmd), (unsigned char *)&sCmd,
                                 sizeof(sAck), (unsigned char *)&sAck);

   if (ret < 0)
   {
      return ret;
   }

   if (pDsmStatistics != NULL)
   {
      pDsmStatistics->n_fw_dropped_size = (sAck.ErrVecDiscard_MSW << 16) + sAck.ErrVecDiscard_LSW;

      if(MEI_DEVICE_CFG_IS_PLATFORM(e_MEI_DEV_PLATFORM_CONFIG_VR11))
      {
         pDsmStatistics->n_processed = (sAck.ErrVecTransmitted_MSW << 16) + sAck.ErrVecTransmitted_LSW;
         pDsmStatistics->n_fw_total = (sAck.ErrVecTotal_MSW << 16) + sAck.ErrVecTotal_LSW;
      }
   }

   return ret;
}

/**
   Check DSM Vectoring statisitcs stored in firmware (after
   fw download)

\param
   pMeiDynCntrl   - private dynamic device data (per open instance)

\return
   NONE
*/
IFX_void_t MEI_VRX_DSM_FwStatsCheck(MEI_DYN_CNTRL_T *pMeiDynCntrl)
{
   MEI_DEV_T *pMeiDev = pMeiDynCntrl->pMeiDev;
   IOCTL_MEI_dsmStatistics_t dsmStatistics;

   memset(&dsmStatistics, 0, sizeof(dsmStatistics));

   if (pMeiDev->nFwVectorSupport == e_MEI_DSM_VECTOR_FW_SUPPORT_MODE_FULL)
   {
      /* Increment successfull fw downloads counter */
      pMeiDev->meiFwDlCount++;

      /* read n_fw_dropped_size from firmware via msg */
      MEI_VRX_DSM_FwStatsUpdate(pMeiDynCntrl, &dsmStatistics);

      if (dsmStatistics.n_fw_dropped_size != 0)
      {
         PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_WRN,
            ("MEI_DRV[0x%02X]: DSM statistics (fw_dropped counter) not cleared (%i)!"
             MEI_DRV_CRLF, MEI_DRV_LINENUM_GET(pMeiDev), dsmStatistics.n_fw_dropped_size));
      }
   }
}

/**
   Update DSM Vectoring statisitcs stored in firmware

\param
   pMeiDynCntrl         - private dynamic device data (per open instance)

\param
   pMeiDsmStatistics    - read internal FW stat values of DSM

\return
   NONE
*/
IFX_void_t MEI_VRX_DSM_FwStatsUpdate(MEI_DYN_CNTRL_T *pMeiDynCntrl,
                                     IOCTL_MEI_dsmStatistics_t *pDsmStatistics)
{
   MEI_DEV_T *pMeiDev = pMeiDynCntrl->pMeiDev;

   if (MEI_VRX_DSM_StatsGet(pMeiDynCntrl, pDsmStatistics) < 0)
   {
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
         ("MEI_DRV[0x%02X]: fail to get DSM statistics!" MEI_DRV_CRLF,
         MEI_DRV_LINENUM_GET(pMeiDev)));
   }
}

/**
   Set necessary fw confirure options for DSM Vectoring

\param
   pMeiDynCntrl   - private dynamic device data (per open instance)

\return
   IFX_SUCCESS    Success
   IFX_ERROR      in case of error
*/
IFX_int32_t MEI_VRX_DSM_FwConfigSet(MEI_DYN_CNTRL_T *pMeiDynCntrl)
{
   IFX_int32_t ret = 0;
   MEI_DEV_T *pMeiDev = pMeiDynCntrl->pMeiDev;

   if (pMeiDev->nFwVectorSupport >= e_MEI_DSM_VECTOR_FW_SUPPORT_MODE_REDUCE)
   {
      if ((ret = MEI_VRX_DSM_ControlSet(pMeiDynCntrl, &pMeiDev->meiDsmConfig)) < 0)
      {
         PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DRV[0x%02X]: fail to set DSM config!" MEI_DRV_CRLF,
            MEI_DRV_LINENUM_GET(pMeiDev)));
      }
   }

   if (pMeiDev->nFwVectorSupport == e_MEI_DSM_VECTOR_FW_SUPPORT_MODE_FULL)
   {
      if (ret == 0)
      {
         if ((ret = MEI_VRX_DSM_MacConfigSet(pMeiDynCntrl, &pMeiDev->meiMacConfig)) < 0)
         {
            PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
               ("MEI_DRV[0x%02X]: fail to set MAC config!" MEI_DRV_CRLF,
               MEI_DRV_LINENUM_GET(pMeiDev)));
         }
      }
   }

   return IFX_SUCCESS;
}

#if MEI_SUPPORT_DEVICE_VR11 != 1
/**
   Init DSM related data

\param
    pMeiDev       private device data

\param
    pDsmErbParams Error Reported Block parameters passing by FW

\return
   NONE
*/
IFX_void_t MEI_VRX_DSM_EvtErbHandler(MEI_DEV_T *pMeiDev, EVT_DSM_ErrorVectorReady_t *pDsmErbParams)
{
   IFX_uint32_t *pMeiErrVecSize = (IFX_uint32_t *)pMeiDev->meiERBbuf.pERB_virt;
   IFX_int32_t pp_err_code;
#if (MEI_DBG_DSM_PROFILING == 1)
   u64 count_start, count_end;
#endif

#if (MEI_DBG_DSM_PROFILING == 1)
   /* [TD, 2012-11-19] Reset ERB data word (32 bit) - index: 9, offset: 0x24
      For FW debugging: Before PP driver is called. */
   if (pMeiDev->bErbReset == IFX_TRUE)
   {
      *(pMeiErrVecSize + 9) = 0x0;
   }
#endif

   /* Check if length read from buf are equal to length passing via event */
   if (*pMeiErrVecSize != pDsmErbParams->ErrVecSize)
   {
      pMeiDev->meiDsmStatistic.n_mei_dropped_size++;
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
         ("MEI_DRV[0x%02X]: ERB length 0x%x are not equal to EVT parameter (0x%x)!" MEI_DRV_CRLF,
         MEI_DRV_LINENUM_GET(pMeiDev), *pMeiErrVecSize, pDsmErbParams->ErrVecSize));

      return;
   }

   /* Check for non zero length */
   if (*pMeiErrVecSize == 0)
   {
      pMeiDev->meiDsmStatistic.n_mei_dropped_size++;
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
         ("MEI_DRV[0x%02X]: Zero ERB length!" MEI_DRV_CRLF, MEI_DRV_LINENUM_GET(pMeiDev)));

      return;
   }

   /* Check callback function was initialized by PP driver */
   if (pMeiDev->meiERBbuf.pCallBackFunc == NULL)
   {
      pMeiDev->meiDsmStatistic.n_mei_dropped_no_pp_cb++;
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
         ("MEI_DRV[0x%02X]: DSM callback function are not initialized!" MEI_DRV_CRLF,
         MEI_DRV_LINENUM_GET(pMeiDev)));

      return;
   }

#if (MEI_DBG_DSM_PROFILING == 1)
   count_start = MEI_Count0_read(pMeiDev);
#endif

   MEI_DRVOS_SemaphoreLock(&pCallBackFuncAccessLock);

   /* Call PP driver callback function */
   if ((pp_err_code = pMeiDev->meiERBbuf.pCallBackFunc(
      (IFX_uint32_t *)pMeiDev->meiERBbuf.pERB_virt)) < 0)
   {
#if (MEI_DBG_DSM_PROFILING == 1)
      count_end = MEI_Count0_read(pMeiDev);
      /* [TD, 2012-11-19] Reset ERB data word (32 bit) - index: 10, offset: 0x28
         For FW debugging: After PP driver has processed the data. */
      if (pMeiDev->bErbReset == IFX_TRUE)
      {
         *(pMeiErrVecSize + 10) = 0x0;
      }
#endif
      pMeiDev->meiDsmStatistic.n_pp_dropped++;
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
         ("MEI_DRV[0x%02X]: ERB dropped by PP driver (err code %i)!" MEI_DRV_CRLF,
         MEI_DRV_LINENUM_GET(pMeiDev), pp_err_code));
   }
   else
   {
#if (MEI_DBG_DSM_PROFILING == 1)
      count_end = MEI_Count0_read(pMeiDev);
      /* [TD, 2012-11-19] Reset ERB data word (32 bit) - index: 10, offset: 0x28
         For FW debugging: After PP driver has processed the data. */
      if (pMeiDev->bErbReset == IFX_TRUE)
      {
         *(pMeiErrVecSize + 10) = 0x0;
      }
#endif
      pMeiDev->meiDsmStatistic.n_processed++;
      PRN_ERR_USR_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL,
         ("MEI_DRV[0x%02X]: ERB processed successfully by PP driver" MEI_DRV_CRLF,
         MEI_DRV_LINENUM_GET(pMeiDev)));
   }

   MEI_DRVOS_SemaphoreUnlock(&pCallBackFuncAccessLock);

#if (MEI_DBG_DSM_PROFILING == 1)
   if (*pMeiErrVecSize == 0)
   {
      MEI_VRX_DSM_DbgUpdateProfiling(pMeiDev, count_start, count_end);
   }
#endif
}

#endif /* (MEI_SUPPORT_DEVICE_VR11 != 1) */

#if (MEI_DBG_DSM_PROFILING == 1)
#define COUNT0_MAX 0xFFFFFFFF
#include <asm/ifx/ifx_clk.h>
#include <linux/delay.h>
extern unsigned int ifx_get_cpu_hz(void);

IFX_void_t MEI_VRX_DSM_DbgPrintProfiling(MEI_DEV_T *pMeiDev)
{
   IFX_uint32_t i;

   printk ("MEI_DRV[0x%02X]: meiDbgProfilingData:" MEI_DRV_CRLF,
      MEI_DRV_LINENUM_GET(pMeiDev));
   for (i = 0; i < 32; i++)
   {
      printk ("%02d: %d" MEI_DRV_CRLF, i, pMeiDev->meiDbgProfilingData[i]);
   }
}

IFX_void_t MEI_VRX_DSM_DbgUpdateProfiling(MEI_DEV_T *pMeiDev,
   u64 count_start, u64 count_end)
{
   IFX_int32_t cpu_freq_kHz, idx;
   u64 count_diff, time_diff;

   cpu_freq_kHz=ifx_get_cpu_hz()/1000;

   if (count_start >= count_end)
   {
      count_diff = (COUNT0_MAX - count_start) + count_end;
   }
   else
   {
      count_diff = count_end - count_start;
   }

   time_diff = 2 * count_diff;            /* count runs with cpu/2 */
   do_div(time_diff, cpu_freq_kHz);       /* time_diff in milli seconds */
   /*printk ("time_diff[%d/%d]=%d [ms]" MEI_DRV_CRLF, i, j, (IFX_uint32_t)time_diff);*/
   idx = (IFX_uint32_t)(time_diff >> 2);
   if (idx > 31) idx = 31;
   /*printk ("idx[%d/%d]=%d" MEI_DRV_CRLF, i, j, idx);*/
   pMeiDev->meiDbgProfilingData[idx]++;

}

IFX_void_t MEI_VRX_DSM_DbgTestProfiling(MEI_DEV_T *pMeiDev)
{
   IFX_uint32_t i, j;
   IFX_int32_t cpu_freq_kHz;
   u64 count_start, count_end;

   cpu_freq_kHz=ifx_get_cpu_hz()/1000;
   for (i = 0; i < 32; i++)
   {
      for (j = 0; j < (i+1) ; j++)
      {
         count_start = MEI_Count0_read(pMeiDev);
         /* Using interruptible timout with 'minimum' delay gives unpredictable
            delays! */
         /*MEI_DRVOS_Wait_ms(4*i + 2);*/
         udelay((4*i + 2)*1000);
         count_end = MEI_Count0_read(pMeiDev);

         MEI_VRX_DSM_DbgUpdateProfiling(pMeiDev, count_start, count_end);
      }
   }
   printk ("MEI_DRV[0x%02X]: meiDbgProfilingData:" MEI_DRV_CRLF,
      MEI_DRV_LINENUM_GET(pMeiDev));
   for (i = 0; i < 32; i++)
   {
      printk ("%02d: %d" MEI_DRV_CRLF, i, pMeiDev->meiDbgProfilingData[i]);
   }
   memset((IFX_uint8_t *)pMeiDev->meiDbgProfilingData, 0x00, sizeof(pMeiDev->meiDbgProfilingData));
}

IFX_uint32_t MEI_Count0_read(MEI_DEV_T *pMeiDev)
{
   IFX_int32_t cpu_freq_MHz = 0;

   cpu_freq_MHz=ifx_get_cpu_hz()/1000000;

   return read_c0_count();
}
#endif

#endif /* (MEI_SUPPORT_DSM == 1) */

