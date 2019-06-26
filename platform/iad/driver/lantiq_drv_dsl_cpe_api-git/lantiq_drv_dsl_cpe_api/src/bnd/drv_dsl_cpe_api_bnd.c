/******************************************************************************

                          Copyright (c) 2007-2015
                     Lantiq Beteiligungs-GmbH & Co. KG

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/
#define DSL_INTERN

#include "drv_dsl_cpe_api.h"

#ifdef __cplusplus
   extern "C" {
#endif


#undef DSL_DBG_BLOCK
#define DSL_DBG_BLOCK DSL_DBG_CPE_API

/*
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_BND_HW_INIT
*/
DSL_Error_t DSL_DRV_BND_HwInit(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN DSL_BND_HwInit_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_CHECK_POINTER(pContext, pData);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, SYS_DBG_MSG"DSL[%02d]: IN - DSL_DRV_BND_HwInit"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Call device specific implementation*/
   nErrCode = DSL_DRV_BND_DEV_HwInit(pContext, pData);

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, SYS_DBG_MSG"DSL[%02d]: OUT - DSL_DRV_BND_HwInit, retCode=%d"
      DSL_DRV_CRLF, DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

/*
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_BND_CONFIG_SET
*/
DSL_Error_t DSL_DRV_BND_ConfigSet(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN DSL_BND_ConfigSet_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS, nRetCode = DSL_SUCCESS;
   DSL_uint8_t nLine;
   DSL_Context_t *pCurrCtx;
#if defined(INCLUDE_DSL_CPE_API_VRX)
   DSL_int_t nAutoTcCount = 0, nAtmTcCount = 0;
   DSL_TcLayerSelection_t nTcMode = DSL_TC_UNKNOWN;
#endif

   DSL_CHECK_POINTER(pContext, pData);
   DSL_CHECK_ERR_CODE();

   DSL_CHECK_DSLMODE(pData->nDslMode);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, SYS_DBG_MSG"DSL[%02d]: IN - DSL_DRV_BND_ConfigSet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

#if defined(INCLUDE_DSL_CPE_API_VRX)
   if (pData->data.bPafEnable == DSL_TRUE)
   {
      if (DSL_DRV_LINES_PER_DEVICE == 2 &&  DSL_DRV_DEV_FirmwareFeatureCheck(pContext,
         DSL_FW_XDSLFEATURE_DUALPORT) == DSL_FALSE)
      {
         DSL_DEBUG(DSL_DBG_WRN, (pContext, SYS_DBG_WRN
            "DSL[%02d]: Bonding could not be enabled because of firmware "
            "doesn't support it"DSL_DRV_CRLF, DSL_DEV_NUM(pContext)));

         return DSL_WRN_BND_NOT_SUPPORTED_BY_FIRMWARE;
      }
      else
      {
         /* Check if one of the following configurations are done for one of
            the available devices/lines. If yes, return appropriate warning.
            - Auto TC-Layer is configured for VDSL/ADSL
            - ATM TC-Layer is configured for VDSL/ADSL */
         for (nLine = 0;
              nLine < DSL_DRV_LINES_PER_DEVICE * DSL_DRV_DEVICE_NUMBER;
              nLine++)
         {
            pCurrCtx = pContext->pXDev[nLine].pContext;

            DSL_CTX_READ_SCALAR(pCurrCtx, nRetCode,
               pDevCtx->data.deviceCfg.sysCIF[pData->nDslMode].nTcLayer,
               nTcMode);
            DSL_SET_ERROR_CODE(nRetCode, nErrCode);

            if (nTcMode == DSL_TC_AUTO)
            {
               nAutoTcCount++;
            }
            if (nTcMode == DSL_TC_ATM)
            {
               nAtmTcCount++;
            }
         }

         /* Set error/warning return codes in order of priority */
         if (nAtmTcCount > 0)
         {
            /* Reject bonding enable in this case */
            pData->data.bPafEnable = DSL_FALSE;
            DSL_SET_ERROR_CODE(DSL_ERR_CONFIG_BND_VS_TCLAYER, nErrCode);
         }
         else if (nAutoTcCount > 0)
         {
            /* Accept bonding enable in this case (do not reset user enable) */
            DSL_SET_ERROR_CODE(DSL_WRN_CONFIG_BND_VS_TCLAYER, nErrCode);
         }
      }
   }
#endif

   if (DSL_DRV_BONDING_ENABLED)
   {
      for (nLine = 0; nLine < DSL_DRV_LINES_PER_DEVICE * DSL_DRV_DEVICE_NUMBER; nLine++)
      {
         pCurrCtx = pContext->pXDev[nLine].pContext;

         /* Set Bonding configuration within CPE API Context*/
         DSL_CTX_WRITE(pCurrCtx, nRetCode, BndConfig[pData->nDslMode], pData->data);
         DSL_SET_ERROR_CODE(nRetCode, nErrCode);
      }
   }
   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, SYS_DBG_MSG"DSL[%02d]: OUT - DSL_DRV_BND_ConfigSet, retCode=%d"
      DSL_DRV_CRLF, DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

/*
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_BND_CONFIG_GET
*/
#ifdef INCLUDE_DSL_CONFIG_GET
DSL_Error_t DSL_DRV_BND_ConfigGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_OUT DSL_BND_ConfigGet_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_CHECK_POINTER(pContext, pData);
   DSL_CHECK_ERR_CODE();

   DSL_CHECK_DSLMODE(pData->nDslMode);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, SYS_DBG_MSG"DSL[%02d]: IN - DSL_DRV_BND_ConfigGet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Get Bonding configuration from the CPE API Context*/
   DSL_CTX_READ(pContext, nErrCode, BndConfig[pData->nDslMode], pData->data);

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, SYS_DBG_MSG"DSL[%02d]: OUT - DSL_DRV_BND_ConfigGet, retCode=%d"
      DSL_DRV_CRLF, DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}
#endif /* INCLUDE_DSL_CONFIG_GET*/

/*
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_BND_HS_STATUS_GET
*/
DSL_Error_t DSL_DRV_BND_HsStatusGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_OUT DSL_BND_HsStatusGet_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_CHECK_POINTER(pContext, pData);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, SYS_DBG_MSG"DSL[%02d]: IN - DSL_DRV_BND_HsStatusGet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   if (DSL_DRV_LINES_PER_DEVICE == 2 && DSL_DRV_DEV_FirmwareFeatureCheck(pContext,
      DSL_FW_XDSLFEATURE_DUALPORT) == DSL_FALSE)
   {
      /* on-chip bonding: ignore if feature is not supported */
      DSL_DEBUG(DSL_DBG_WRN, (pContext, SYS_DBG_WRN"DSL[%02d]: "
         "WARNING - DSL_DRV_BND_HsStatusGet(): Firmware does not support bonding"
         DSL_DRV_CRLF, DSL_DEV_NUM(pContext)));
   }
   else
   {
      /* Call VRX device specific implementation*/
      nErrCode = DSL_DRV_BND_DEV_HsStatusGet(pContext, pData);
   }

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, SYS_DBG_MSG"DSL[%02d]: OUT - DSL_DRV_BND_HsStatusGet, retCode=%d"
      DSL_DRV_CRLF, DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

DSL_Error_t DSL_DRV_BND_RemotePafAvailableCheck(
   DSL_IN DSL_Context_t *pContext,
   DSL_OUT DSL_uint32_t *pRemotePafAvailable)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;
   DSL_BND_HsStatusGet_t status;

   DSL_DRV_MemSet(&status, 0x0, sizeof(DSL_BND_HsStatusGet_t));
   nErrCode = DSL_DRV_BND_HsStatusGet(pContext, &status);
   if (nErrCode != DSL_SUCCESS)
   {
      DSL_DEBUG(DSL_DBG_ERR,
         (pContext, SYS_DBG_ERR "DSL[%02d]: ERROR - PAF status get failed!"
         DSL_DRV_CRLF, DSL_DEV_NUM(pContext)));

      return nErrCode;
   }

   /* Set RemotePafAvailable */
   *pRemotePafAvailable = (DSL_uint32_t)status.data.nRemotePafSupported;

   return DSL_SUCCESS;
}

/*
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_BND_HS_CONTINUE
*/
DSL_Error_t DSL_DRV_BND_HsContinue(
   DSL_IN DSL_Context_t *pContext,
   DSL_IN DSL_BND_HsContinue_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_CHECK_POINTER(pContext, pData);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, SYS_DBG_MSG"DSL[%02d]: IN - DSL_DRV_BND_HsContinue"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Call device specific implementation*/
   nErrCode = DSL_DRV_BND_DEV_HsContinue(pContext, pData);

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, SYS_DBG_MSG"DSL[%02d]: OUT - DSL_DRV_BND_HsContinue, retCode=%d"
      DSL_DRV_CRLF, DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

/*
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_BND_ETH_DBG_COUNTERS_GET
*/
DSL_Error_t DSL_DRV_BND_EthDbgCountersGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_OUT DSL_BND_EthDbgCounters_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_CHECK_POINTER(pContext, pData);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, SYS_DBG_MSG"DSL[%02d]: IN - DSL_DRV_BND_EthDbgCountersGet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Call device specific implementation*/
   nErrCode = DSL_DRV_BND_DEV_EthDbgCountersGet(pContext, pData);

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, SYS_DBG_MSG"DSL[%02d]: OUT - DSL_DRV_BND_EthDbgCountersGet, retCode=%d"
      DSL_DRV_CRLF, DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

/*
   For a detailed description please refer to the equivalent ioctl
   \ref DSL_FIO_BND_ETH_COUNTERS_GET
*/
DSL_Error_t DSL_DRV_BND_EthCountersGet(
   DSL_IN DSL_Context_t *pContext,
   DSL_OUT DSL_BND_EthCounters_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_CHECK_POINTER(pContext, pData);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, SYS_DBG_MSG"DSL[%02d]: IN - DSL_DRV_BND_EthCountersGet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   /* Call device specific implementation*/
   nErrCode = DSL_DRV_BND_DEV_EthCountersGet(pContext, pData);

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, SYS_DBG_MSG"DSL[%02d]: OUT - DSL_DRV_BND_EthCountersGet, retCode=%d"
      DSL_DRV_CRLF, DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;
}

DSL_Error_t DSL_DRV_BND_PortModeSyncSet(
   DSL_IN DSL_Context_t *pContext,
   DSL_OUT DSL_BND_PortModeSync_t *pData)
{
   DSL_Error_t nErrCode = DSL_SUCCESS;

   DSL_CHECK_POINTER(pContext, pData);
   DSL_CHECK_ERR_CODE();

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, SYS_DBG_MSG"DSL[%02d]: IN - DSL_DRV_BND_PortModeSyncSet"DSL_DRV_CRLF,
      DSL_DEV_NUM(pContext)));

   if (pData->data.nPortMode < DSL_PORT_MODE_NA ||
       pData->data.nPortMode >= DSL_PORT_MODE_LAST)
   {
      nErrCode = DSL_ERR_PARAM_RANGE;
   }
   else
   {
      DSL_CTX_WRITE(pContext, nErrCode, BndPortModeSync, pData->data.nPortMode);
   }

   DSL_DEBUG(DSL_DBG_MSG,
      (pContext, SYS_DBG_MSG"DSL[%02d]: OUT - DSL_DRV_BND_PortModeSyncSet, retCode=%d"
      DSL_DRV_CRLF, DSL_DEV_NUM(pContext), nErrCode));

   return nErrCode;

}


#ifdef __cplusplus
}
#endif
