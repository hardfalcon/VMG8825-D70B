/******************************************************************************

                          Copyright (c) 2007-2015
                     Lantiq Beteiligungs-GmbH & Co. KG

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/* ==========================================================================
   Description : VRX Driver internal function interface
   ========================================================================== */

/* ============================================================================
   Includes
   ========================================================================= */

/* get at first the driver configuration */
#include "drv_mei_cpe_config.h"

#if (MEI_EXPORT_INTERNAL_API == 1) && (MEI_DRV_ATM_PTM_INTERFACE_ENABLE == 1)

#include "ifx_types.h"
#include "drv_mei_cpe_os.h"

#ifdef PPA_SUPPORTS_CALLBACKS
#ifdef LINUX
   #if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,12,59) && (MEI_SUPPORT_DEVICE_VR11 == 1))
      /* Nothing to include here anymore
         Definitions were moved to <net/dsl_tc.h> */
   #elif (LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0))
      #if (MEI_SUPPORT_DEVICE_VR10_320 != 1)
         #include <asm/ifx/ifx_atm.h>
      #else
         #include <linux/atm.h>
      #endif
   #else
      #if (MEI_SUPPORT_DEVICE_VR10_320 != 1)
         #include <lantiq_atm.h>
      #else
         #include <linux/lantiq_atm.h>
      #endif
   #endif
#else
   #error "ATM/PTM internal interface is only supported for Linux!"
#endif
#endif

/** get interface and configuration */
#include "drv_mei_cpe_interface.h"
#include "drv_mei_cpe_api.h"
#include "drv_mei_cpe_api_atm_ptm_intern.h"
#include "drv_mei_cpe_download.h"
/** get line state status */
#include "drv_mei_cpe_device_cntrl.h"

static unsigned int g_tx_link_rate[MEI_MAX_SUPPORTED_DFE_CHAN_DEVICES][2] = {{0}};
void *g_xdata_addr[MEI_MAX_SUPPORTED_DFE_CHAN_DEVICES] = {NULL};

/* ============================================================================
   Exported functions
   ========================================================================= */

IFX_int32_t MEI_InternalXtmSwhowtimeEntrySignal(
                              MEI_DYN_CNTRL_T         *pMeiDynCntrl,
                              MEI_XTM_ShowtimeEnter_t *pArgXtm)
{
   IFX_int32_t retVal = IFX_SUCCESS;
   IFX_uint8_t dslLineNum;
#ifdef PPA_SUPPORTS_CALLBACKS
   struct port_cell_info port_cell = {0};
   ltq_mei_atm_showtime_enter_t mei_showtime_enter = NULL;
#endif

   if (!pMeiDynCntrl || !pArgXtm)
      return -e_MEI_ERR_GET_ARG;

   /* Get line number*/
   dslLineNum = pMeiDynCntrl->pMeiDev->meiDrvCntrl.dslLineNum;

   if (pArgXtm->rate_fast)
   {
     g_tx_link_rate[dslLineNum][0] = pArgXtm->rate_fast / (53 * 8);
   }

   if (pArgXtm->rate_intl)
   {
      g_tx_link_rate[dslLineNum][1] = pArgXtm->rate_intl / (53 * 8);
   }

   if (g_tx_link_rate[dslLineNum][0] == 0 && g_tx_link_rate[dslLineNum][1] == 0)
   {
      retVal = -e_MEI_ERR_INVAL_CONFIG;
   }

#ifdef PPA_SUPPORTS_CALLBACKS
   /* get NULL or function pointer */
   mei_showtime_enter =
        (ltq_mei_atm_showtime_enter_t)ppa_callback_get(LTQ_MEI_SHOWTIME_ENTER);

   if (mei_showtime_enter)
   {
      port_cell.port_num = 2;
      port_cell.tx_link_rate[0] = g_tx_link_rate[dslLineNum][0];
      port_cell.tx_link_rate[1] = g_tx_link_rate[dslLineNum][1];

      PRN_DBG_USR_NL( MEI_NOTIFICATIONS, MEI_DRV_PRN_LEVEL_NORMAL,
            ("MEI_DRV[%02d]: ShowtimeEntrySignal, tx_link_rate(fast)=%d, "
             "tx_link_rate(intl)=%d" MEI_DRV_CRLF, dslLineNum,
             port_cell.tx_link_rate[0], port_cell.tx_link_rate[1]));

      mei_showtime_enter(dslLineNum, &port_cell, g_xdata_addr[dslLineNum]);
   }
   else
   {
      PRN_DBG_USR_NL( MEI_NOTIFICATIONS, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DRV[%02d]: ShowtimeEntrySignal failed, no PP callback defined!"
             MEI_DRV_CRLF, dslLineNum));

      retVal = -e_MEI_ERR_OP_FAILED;
   }

#if MEI_SUPPORT_DEVICE_VR11 == 1
   MEI_CGU_PPLOMCFG_print(&(pMeiDynCntrl->pMeiDev->meiDrvCntrl));
#endif /* MEI_SUPPORT_DEVICE_VR11 */
#else
   PRN_DBG_USR_NL( MEI_NOTIFICATIONS, MEI_DRV_PRN_LEVEL_ERR,
         ("MEI_DRV[%02d]: ShowtimeEntrySignal disabled by compile option, will not call it!"
          MEI_DRV_CRLF, dslLineNum));
#endif

   return retVal;
}

IFX_int32_t MEI_InternalXtmSwhowtimeExitSignal(
                              MEI_DYN_CNTRL_T        *pMeiDynCntrl,
                              MEI_XTM_ShowtimeExit_t *pArgXtm)
{
   IFX_int32_t retVal = IFX_SUCCESS;
   IFX_uint8_t dslLineNum;
#ifdef PPA_SUPPORTS_CALLBACKS
   ltq_mei_atm_showtime_exit_t mei_showtime_exit = NULL;
#endif

   if (!pMeiDynCntrl || !pArgXtm)
      return -e_MEI_ERR_GET_ARG;

   /* Get line number*/
   dslLineNum = pMeiDynCntrl->pMeiDev->meiDrvCntrl.dslLineNum;

#ifdef PPA_SUPPORTS_CALLBACKS
   /* get NULL or function pointer */
   mei_showtime_exit =
        (ltq_mei_atm_showtime_exit_t)ppa_callback_get(LTQ_MEI_SHOWTIME_EXIT);

   if (mei_showtime_exit)
   {
      PRN_DBG_USR_NL( MEI_NOTIFICATIONS, MEI_DRV_PRN_LEVEL_NORMAL,
            ("MEI_DRV[%02d]: ShowtimeExitSignal" MEI_DRV_CRLF, dslLineNum));

      mei_showtime_exit(dslLineNum);
   }
   else
   {
      PRN_DBG_USR_NL( MEI_NOTIFICATIONS, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DRV[%02d]: ShowtimeExitSignal failed, no PP callback defined!"
             MEI_DRV_CRLF, dslLineNum));

      retVal = -e_MEI_ERR_OP_FAILED;
   }
#else
   PRN_DBG_USR_NL( MEI_NOTIFICATIONS, MEI_DRV_PRN_LEVEL_ERR,
         ("MEI_DRV[%02d]: ShowtimeExitSignal disabled by compile option, will not call it!"
          MEI_DRV_CRLF, dslLineNum));
#endif /* PPA_SUPPORTS_CALLBACKS */

   return retVal;
}

IFX_int32_t MEI_TcRequest(void *data)
{
   MEI_TcLayerRequestCallback_t *args = data;
   MEI_DEV_T *pMeiDev = args->pMeiDev;
   IFX_uint8_t dslLineNum = args->line;
   IFX_int32_t retVal = IFX_SUCCESS;

#if defined(PPA_SUPPORTS_CALLBACKS) && defined(PPA_SUPPORTS_TC_CALLBACKS)
   IFX_uint32_t nTcLayer = args->nTcLayer;
   mei_tc_request_type request_type = MEI_TC_REQUEST_OFF;
   MEI_DYN_CNTRL_T *pMeiDynCntrl = NULL;
   MEI_TC_Request_t argsTcRequest;
   IFX_uint8_t counter;

   MEI_DRV_RELEASE_UNIQUE_CALLBACK_ACCESS(pMeiDev);

   switch (nTcLayer)
   {
      case DSL_TC_ATM:
         request_type = MEI_TC_REQUEST_ATM;
         break;

      case DSL_TC_EFM:
         request_type = MEI_TC_REQUEST_PTM;
         break;

      default:
         request_type = MEI_TC_REQUEST_OFF;
         break;
   }

   MEI_DynCntrlStructAlloc(dslLineNum, &pMeiDynCntrl);

   if (pMeiDynCntrl != NULL)
   {
      pMeiDynCntrl->openInstance = 0xFF;
      pMeiDynCntrl->pDfeX        = NULL;
      pMeiDynCntrl->pMeiDev      = pMeiDev;

      argsTcRequest.request_type = request_type;
      argsTcRequest.is_bonding = MEI_PAF_EnableGet(pMeiDynCntrl);

      MEI_InternalTcRequest(pMeiDynCntrl, &argsTcRequest);

#if MEI_SUPPORT_DEVICE_VR11 == 1
      if (nTcLayer == DSL_TC_EFM)
      {
         for(counter = 0; counter < MEI_DEV_ERB_ATTEMPTS; ++counter)
         {
            MEI_DRVOS_Wait_ms(MEI_DEV_ERB_TIMEOUT);

            PRN_DBG_USR_NL( MEI_NOTIFICATIONS, MEI_DRV_PRN_LEVEL_MSG,
               ("MEI_DRV[%02d - %02d]: TcRequest attempt #%u." MEI_DRV_CRLF,
               dslLineNum, pMeiDynCntrl->openInstance, counter+1));

            retVal = MEI_InternalErbAddrUpdate(pMeiDynCntrl);
            MEI_VRX_ModemStateUpdate(pMeiDynCntrl);

            if ((retVal == -e_MEI_ERR_DEV_BUSY || retVal == -e_MEI_ERR_DEV_INVAL_RESP) &&
                  MEI_DRV_MODEM_STATE_GET(pMeiDev) >= MEI_DRV_FSM_STATE_FULLINIT)
            {
               continue;
            }
            else
            {
               break;
            }
         }
      }
#endif /* (MEI_SUPPORT_DEVICE_VR11 == 1) */

      MEI_DynCntrlStructFree(&pMeiDynCntrl);
   }

#ifdef IRQ_POLLING_FORCE 
   do_exit(retVal);
#endif /* IRQ_POLLING_FORCE */

#else
   MEI_DRV_RELEASE_UNIQUE_CALLBACK_ACCESS(pMeiDev);

   PRN_DBG_USR_NL( MEI_NOTIFICATIONS, MEI_DRV_PRN_LEVEL_ERR,
      ("MEI_DRV[%02d]: TcRequest disabled by compile option, will not call it!"
      MEI_DRV_CRLF, dslLineNum));

#endif /* defined(PPA_SUPPORTS_CALLBACKS) && defined(PPA_SUPPORTS_TC_CALLBACKS) */

   return retVal;
}

IFX_int32_t MEI_InternalTcRequest(
                              MEI_DYN_CNTRL_T        *pMeiDynCntrl,
                              MEI_TC_Request_t       *pArgTcRequest)
{
   IFX_int32_t retVal = IFX_SUCCESS;
   IFX_uint8_t dslLineNum = 0;
#if defined(PPA_SUPPORTS_CALLBACKS) && defined(PPA_SUPPORTS_TC_CALLBACKS)
   mei_tc_request_t mei_tc_request = NULL;
   mei_tc_request_type request_type = MEI_TC_REQUEST_OFF;
   IFX_int32_t is_bonding = 0;
#endif

   if (!pMeiDynCntrl || !pArgTcRequest)
      return -e_MEI_ERR_GET_ARG;

   /* get line number*/
   dslLineNum = pMeiDynCntrl->pMeiDev->meiDrvCntrl.dslLineNum;

#if defined(PPA_SUPPORTS_CALLBACKS) && defined(PPA_SUPPORTS_TC_CALLBACKS)
   /* get request type */
   request_type = pArgTcRequest->request_type;

   /* get bonding flag */
   is_bonding = pArgTcRequest->is_bonding;

   /* get NULL or function pointer */
   mei_tc_request =
      (mei_tc_request_t)ppa_callback_get(LTQ_MEI_TC_REQUEST);

   if (mei_tc_request)
   {
      PRN_DBG_USR_NL( MEI_NOTIFICATIONS, MEI_DRV_PRN_LEVEL_NORMAL,
            ("MEI_DRV[%02d]: TcRequest %d, is_bonding=%d" MEI_DRV_CRLF,
                              dslLineNum, request_type, is_bonding));

      mei_tc_request(dslLineNum, request_type, is_bonding);
   }
   else
   {
      PRN_DBG_USR_NL( MEI_NOTIFICATIONS, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DRV[%02d]: TcRequest failed, no PP callback defined!"
             MEI_DRV_CRLF, dslLineNum));

      retVal = -e_MEI_ERR_OP_FAILED;
   }

#if MEI_SUPPORT_DEVICE_VR11 == 1
   MEI_CGU_PPLOMCFG_print(&(pMeiDynCntrl->pMeiDev->meiDrvCntrl));
#endif /* MEI_SUPPORT_DEVICE_VR11 */
#else
   PRN_DBG_USR_NL( MEI_NOTIFICATIONS, MEI_DRV_PRN_LEVEL_ERR,
         ("MEI_DRV[%02d]: TcRequest disabled by compile option, will not call it!"
          MEI_DRV_CRLF, dslLineNum));
#endif /* PPA_SUPPORTS_CALLBACKS && PPA_SUPPORTS_TC_CALLBACKS */

   return retVal;
}

#if MEI_SUPPORT_DEVICE_VR11 == 1
IFX_int32_t MEI_InternalErbAddrUpdate(
                                  MEI_DYN_CNTRL_T        *pMeiDynCntrl)
{
   IFX_int32_t retVal = IFX_SUCCESS;
   IFX_uint8_t dslLineNum = 0;
#if defined(PPA_SUPPORTS_CALLBACKS) && defined(PPA_SUPPORTS_TC_CALLBACKS)
   mei_erb_addr_get_t mei_erb_addr_get = NULL;
   unsigned int data_addr, desc_addr;
   int ppaRetVal = 0;
#endif

   /* get line number */
   dslLineNum = pMeiDynCntrl->pMeiDev->meiDrvCntrl.dslLineNum;

#if (MEI_SUPPORT_DSM == 1)
   if (!pMeiDynCntrl->pMeiDev->bIsSetErb)
   {
#endif /* MEI_SUPPORT_DSM == 1 */
#if defined(PPA_SUPPORTS_CALLBACKS) && defined(PPA_SUPPORTS_TC_CALLBACKS)
      /* get NULL or function pointer */
      mei_erb_addr_get = (mei_erb_addr_get_t)ppa_callback_get(LTQ_MEI_ERB_ADDR_GET);
      if (mei_erb_addr_get)
      {
         ppaRetVal = mei_erb_addr_get(dslLineNum, &data_addr, &desc_addr);

         PRN_DBG_USR_NL( MEI_NOTIFICATIONS, MEI_DRV_PRN_LEVEL_MSG,
               ("MEI_DRV[%02d]: mei_erb_addr_get() ->  ppaRetVal=%d, "
                "data_addr=0x%08X, desc_addr=0x%08X"
                MEI_DRV_CRLF, dslLineNum, ppaRetVal, data_addr, desc_addr));

         if (ppaRetVal < 0)
         {
            PRN_DBG_USR_NL( MEI_NOTIFICATIONS, MEI_DRV_PRN_LEVEL_ERR,
                            ("MEI_DRV[%02d]: MEI_InternalErbAddrUpdate: mei_erb_addr_get failed ret=0x%x" MEI_DRV_CRLF,
                             dslLineNum, ppaRetVal));

            retVal = -e_MEI_ERR_DEV_BUSY;
         }
         else if (data_addr == 0 || desc_addr == 0)
         {
            PRN_DBG_USR_NL( MEI_NOTIFICATIONS, MEI_DRV_PRN_LEVEL_ERR,
               ("MEI_DRV[%02d]: MEI_InternalErbAddrUpdate:  Invalid value, data=0x%x, descriptor=0x%x, ret=0x%x" MEI_DRV_CRLF,
                dslLineNum, data_addr, desc_addr, ppaRetVal));
            retVal = -e_MEI_ERR_DEV_INVAL_RESP;
         }
         else
         {
            MEI_VR11_ErbBarSet(pMeiDynCntrl->pMeiDev, data_addr, desc_addr);

#if (MEI_SUPPORT_DSM == 1)
            pMeiDynCntrl->pMeiDev->bar14 = data_addr;
            pMeiDynCntrl->pMeiDev->bar17 = desc_addr;
            pMeiDynCntrl->pMeiDev->bIsSetErb = IFX_TRUE;
#endif /* MEI_SUPPORT_DSM == 1  */
         }
      }
      else
      {
         PRN_DBG_USR_NL( MEI_NOTIFICATIONS, MEI_DRV_PRN_LEVEL_WRN,
               ("MEI_DRV[%02d]: MEI_InternalErbAddrUpdate failed, no PP callback defined!"
                MEI_DRV_CRLF, dslLineNum));

         retVal = -e_MEI_ERR_OP_FAILED;
      }
#else
      PRN_DBG_USR_NL( MEI_NOTIFICATIONS, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DRV[%02d]: MEI_InternalErbAddrUpdate disabled by compile option, will not call it!"
             MEI_DRV_CRLF, dslLineNum));
#endif /* PPA_SUPPORTS_CALLBACKS && PPA_SUPPORTS_TC_CALLBACKS */
#if (MEI_SUPPORT_DSM == 1)
   }
   else
   {
         PRN_DBG_USR_NL( MEI_NOTIFICATIONS, MEI_DRV_PRN_LEVEL_MSG,
               ("MEI_DRV[%02d]: MEI_InternalErbAddrUpdate reusing previously stored values"MEI_DRV_CRLF,
                dslLineNum));
   }
#endif /* MEI_SUPPORT_DSM == 1 */

   return retVal;
}
#endif /* (MEI_SUPPORT_DEVICE_VR11 == 1) */

IFX_int32_t MEI_InternalTcReset(
                              MEI_DYN_CNTRL_T        *pMeiDynCntrl,
                              MEI_TC_Reset_t         *pArgTcReset)
{
   IFX_int32_t retVal = IFX_SUCCESS;
   IFX_uint8_t dslLineNum = 0;
#if defined(PPA_SUPPORTS_CALLBACKS) && defined(PPA_SUPPORTS_TC_CALLBACKS) && (MEI_SUPPORT_DEVICE_VR11 != 1)
   mei_tc_reset_t mei_tc_reset = NULL;
   mei_tc_reset_type reset_type = MEI_TC_RESET_CLEAN;
#endif

   if (!pMeiDynCntrl || !pArgTcReset)
      return -e_MEI_ERR_GET_ARG;

   /* get line number*/
   dslLineNum = pMeiDynCntrl->pMeiDev->meiDrvCntrl.dslLineNum;

#if (MEI_SUPPORT_DEVICE_VR11 != 1)
#if defined(PPA_SUPPORTS_CALLBACKS) && defined(PPA_SUPPORTS_TC_CALLBACKS)
   /* get reset type */
   reset_type = pArgTcReset->reset_type;
   /* get NULL or function pointer */
   mei_tc_reset = (mei_tc_reset_t)ppa_callback_get(LTQ_MEI_TC_RESET);

   if (mei_tc_reset)
   {
      PRN_DBG_USR_NL( MEI_NOTIFICATIONS, MEI_DRV_PRN_LEVEL_NORMAL,
         ("MEI_DRV[%02d]: TcReset %d" MEI_DRV_CRLF, dslLineNum, reset_type));

      mei_tc_reset(dslLineNum, reset_type);
   }
   else
   {
      PRN_DBG_USR_NL( MEI_NOTIFICATIONS, MEI_DRV_PRN_LEVEL_ERR,
            ("MEI_DRV[%02d]: TcReset failed, no PP callback defined!"
             MEI_DRV_CRLF, dslLineNum));

      retVal = -e_MEI_ERR_OP_FAILED;
   }
#else
   PRN_DBG_USR_NL( MEI_NOTIFICATIONS, MEI_DRV_PRN_LEVEL_ERR,
         ("MEI_DRV[%02d]: TcReset disabled by compile option, will not call it!"
          MEI_DRV_CRLF, dslLineNum));
#endif /* PPA_SUPPORTS_CALLBACKS && PPA_SUPPORTS_TC_CALLBACKS */
#endif /* (MEI_SUPPORT_DEVICE_VR11 != 1) */
   return retVal;
}

#ifdef PPA_SUPPORTS_CALLBACKS
/**
   Function that is used by the PP subsystem to get some showtime relevant data
   from the DSL subsystem (MEI Driver).

   \param line_idx
      Defines the line index for which showtime data is requested.
   \param is_showtime[out]
      Retuns current (showtime) state of the given line
         - 0: Line is currently *not* in showtime
         - 1: Line is currently in showtime
   \param port_cell[out]
      Returns the cell rate for the given line
   \param xdata_addr[out]
      Returns a pointer to the consistent DSL FW data memory

   \return
      0 if successful and -1 in case of an error/warning
*/
int ltq_mei_showtime_check(
                              const unsigned char line_idx,
                              int *is_showtime,
                              struct port_cell_info *port_cell,
                              void **xdata_addr)
{
   unsigned int i;

   if (line_idx >= MEI_DFE_CHAN_DEVICES)
   {
      return -1;
   }

   PRN_DBG_USR_NL( MEI_NOTIFICATIONS, MEI_DRV_PRN_LEVEL_NORMAL,
         ("MEI_DRV[%02d]: ShowtimeCheck" MEI_DRV_CRLF, line_idx));

   if (is_showtime)
   {
     *is_showtime = (g_tx_link_rate[line_idx][0] == 0) &&
                    (g_tx_link_rate[line_idx][1] == 0) ? 0 : 1;
     PRN_DBG_USR_NL( MEI_NOTIFICATIONS, MEI_DRV_PRN_LEVEL_NORMAL,
           ("MEI_DRV[%02d]: is_showtime=%d" MEI_DRV_CRLF, line_idx, *is_showtime));
   }

   if (port_cell)
   {
      for ( i = 0; i < port_cell->port_num && i < 2; i++ )
      {
         port_cell->tx_link_rate[i] = g_tx_link_rate[line_idx][i];

         PRN_DBG_USR_NL( MEI_NOTIFICATIONS, MEI_DRV_PRN_LEVEL_NORMAL,
               ("MEI_DRV[%02d]: tx_link_rate[%d]=%d" MEI_DRV_CRLF, line_idx, i,
                port_cell->tx_link_rate[i]));
      }
   }

   if (xdata_addr)
   {
      if ((g_tx_link_rate[line_idx][0] == 0) &&
          (g_tx_link_rate[line_idx][1] == 0))
      {
         *xdata_addr = NULL;
      }
      else
      {
         *xdata_addr = g_xdata_addr[line_idx];
      }

      PRN_DBG_USR_NL( MEI_NOTIFICATIONS, MEI_DRV_PRN_LEVEL_NORMAL,
            ("MEI_DRV[%02d]: *xdata_addr=%p" MEI_DRV_CRLF, line_idx, *xdata_addr));
   }

   return 0;
}
#endif /* #ifdef PPA_SUPPORTS_CALLBACKS */

int ifx_mei_atm_led_blink(void)
{
    return IFX_SUCCESS;
}

EXPORT_SYMBOL (MEI_InternalXtmSwhowtimeEntrySignal);
EXPORT_SYMBOL (MEI_InternalXtmSwhowtimeExitSignal);
EXPORT_SYMBOL (MEI_InternalTcRequest);
EXPORT_SYMBOL (MEI_InternalTcReset);
#if MEI_SUPPORT_DEVICE_VR11 == 1
EXPORT_SYMBOL (MEI_InternalErbAddrUpdate);
#endif /* (MEI_SUPPORT_DEVICE_VR11 == 1) */
EXPORT_SYMBOL(ifx_mei_atm_led_blink);

#endif      /* #if (MEI_EXPORT_INTERNAL_API == 1) && (MEI_DRV_ATM_PTM_INTERFACE_ENABLE == 1) */

