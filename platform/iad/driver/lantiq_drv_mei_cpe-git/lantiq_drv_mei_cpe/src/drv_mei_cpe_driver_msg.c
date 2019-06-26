/******************************************************************************

                          Copyright (c) 2007-2015
                     Lantiq Beteiligungs-GmbH & Co. KG

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/* ==========================================================================
   Description : Message Handling between the driver and the VRX device.
   ========================================================================== */


/* ============================================================================
   Includes
   ========================================================================= */

/* get at first the driver configuration */
#include "drv_mei_cpe_config.h"


#if (MEI_SUPPORT_DRIVER_MSG == 1)

#include "ifx_types.h"
#include "drv_mei_cpe_os.h"
#include "drv_mei_cpe_dbg.h"

#include "drv_mei_cpe_dbg_driver.h"

#include "drv_mei_cpe_interface.h"
#include "drv_mei_cpe_api.h"
#include "drv_mei_cpe_msg_process.h"
#include "drv_mei_cpe_driver_msg.h"

#if (MEI_DRV_ATM_OAM_ENABLE == 1)
#include "drv_mei_cpe_atmoam.h"
#endif


/* ============================================================================
   Local Macros & Definitions
   ========================================================================= */

#ifdef MEI_STATIC
#undef MEI_STATIC
#endif

#ifdef MEI_DEBUG
#define MEI_STATIC
#else
#define MEI_STATIC   static
#endif


/* ============================================================================
   Local Function Declarations
   ========================================================================= */

MEI_STATIC IFX_int32_t MEI_DrvMsg_RomStartSet(
                              MEI_DEV_T              *pMeiDev,
                              MEI_DRV_MSG_RomStart_t *pRomStart,
                              MEI_DRV_STATE_E        prevDrvState);

#if (MEI_DRV_ATM_OAM_ENABLE == 1)
MEI_STATIC IFX_int32_t MEI_DrvMsg_RemAtmOamSet(
                              MEI_DEV_T                *pMeiDev,
                              MEI_ATMOAM_DEV_CNTRL_T   *pAtmOamDevCntrl,
                              MEI_DRV_MSG_RemAtmOam_t  *pDrvMsgRemAtmOam,
                              IFX_uint32_t               linkNo,
                              IFX_uint32_t               extrFailedCnt);
#endif

/* ============================================================================
   Driver Message: ROM Start
   ========================================================================= */

/**
   This function setup a ROM Start VRX driver message.
   Depending on the previous state, the actual MEI interface state
   a ALARM or a EVENT message will be generated.

\param
   pMeiDev      private device data
\param
   pRomStart      points to the RomStart driver message struct.
\param
   prevDrvState   previous driver state


\attention
   - Called on int-level
*/

MEI_STATIC IFX_int32_t MEI_DrvMsg_RomStartSet(
                              MEI_DEV_T              *pMeiDev,
                              MEI_DRV_MSG_RomStart_t *pRomStart,
                              MEI_DRV_STATE_E        prevDrvState)
{

   pRomStart->hdr.lineNum = (unsigned int)MEI_DRV_LINENUM_GET(pMeiDev);

   /* check the reason */
   if ( (prevDrvState == e_MEI_DRV_STATE_SW_INIT_DONE) ||
        (prevDrvState == e_MEI_DRV_STATE_BOOT_WAIT_ROM_ALIVE) )
   {
      pRomStart->reason = (pMeiDev->bUsrRst) ?
                              MEI_DRV_MSG_IF_ROM_START_REASON_USR_RESET :
                              MEI_DRV_MSG_IF_ROM_START_REASON_STARTUP;
   }
   else
   {
      pRomStart->reason = MEI_DRV_MSG_IF_ROM_START_REASON_DEV_RESET;
   }

   pRomStart->prevDrvState = (unsigned int)prevDrvState;
   pRomStart->newDrvState = (unsigned int)(MEI_DRV_STATE_GET(pMeiDev));

   /* check for alarm */
   if (MEI_DRV_MEI_IF_STATE_GET(pMeiDev) != e_MEI_MEI_HW_STATE_UP)
   {
      pRomStart->hdr.id = MEI_DRV_MSG_ALM_ROM_START;
   }
   else
   {
      if(pRomStart->reason == MEI_DRV_MSG_IF_ROM_START_REASON_DEV_RESET)
      {
         pRomStart->hdr.id = MEI_DRV_MSG_ALM_ROM_START;
      }
      else
      {
         pRomStart->hdr.id = MEI_DRV_MSG_EVT_ROM_START;
      }
   }

   return IFX_SUCCESS;
}


/**
   This function setup and distribute a ROM Start VRX driver message.
   Depending on the previous state, the actual MEI interface state
   a ALARM or a EVENT message will be generated.

\param
   pMeiDev      private device data
\param
   prevDrvState   previous driver state

\attention
   - Called on int-level
*/

IFX_int32_t MEI_DrvMsg_RomStartDistribute(
                              MEI_DEV_T              *pMeiDev,
                              MEI_DRV_STATE_E        prevDrvState)
{
   IFX_uint32_t             distCount = 0;
   MEI_DRV_MSG_RomStart_t romStart;

   MEI_DrvMsg_RomStartSet( pMeiDev, &romStart, prevDrvState);

   distCount = MEI_DistributeAutoMsg(
                              pMeiDev,
                              pMeiDev->pRootNfcRecvFirst,
                              (IFX_uint8_t *)&romStart,
                              sizeof(MEI_DRV_MSG_RomStart_t),
                              MEI_RECV_BUF_CTRL_DRIVER_MSG);

   if (distCount <= 0)
   {
      PRN_DBG_INT( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL, MEI_DRV_LINENUM_GET(pMeiDev),
           ("MEI_DRV[%02d]: WARNING - no waiting user, discard ROM Start Drv msg!" MEI_DRV_CRLF,
             MEI_DRV_LINENUM_GET(pMeiDev)));
   }

#if (MEI_SUPPORT_STATISTICS == 1)
   PRN_DBG_INT( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL, MEI_DRV_LINENUM_GET(pMeiDev),
        ("MEI_DRV[%02d]: +++ ROM Start Msg GP1 stat = %d, dist count = %d +++" MEI_DRV_CRLF,
          MEI_DRV_LINENUM_GET(pMeiDev), pMeiDev->statistics.dfeGp1IntCount, distCount));
#endif

   return IFX_SUCCESS;
}


/* ============================================================================
   Driver Message: Remote Access ATM OAM
   ========================================================================= */
#if (MEI_DRV_ATM_OAM_ENABLE == 1)

/**
   This function setup a Remote Access ATM OAM driver message.
   Depending on the result of the TFTP operation a EVENT or a ALARM message
   will be generated.

\param
   pMeiDev         privat device data.
\param
   pAtmOamDevCntrl   privat ATM OAM driver data.
\param
   pDrvMsgRemAtmOam  points to the Remote ATM OAM driver message struct.
\param
   linkNo            link no (bearer channel).
\param
   extrFailedCnt     current number of extract cell failures of the incoming alarm.

\attention
   - Called on int-level.
*/
MEI_STATIC IFX_int32_t MEI_DrvMsg_RemAtmOamSet(
                              MEI_DEV_T                *pMeiDev,
                              MEI_ATMOAM_DEV_CNTRL_T   *pAtmOamDevCntrl,
                              MEI_DRV_MSG_RemAtmOam_t  *pDrvMsgRemAtmOam,
                              IFX_uint32_t               linkNo,
                              IFX_uint32_t               extrFailedCnt )
{
   /* IFX_uint32_t            currTick = MEI_DRVOS_GetElapsedTime_ms(0); */

   pDrvMsgRemAtmOam->hdr.lineNum          = (unsigned int)MEI_DRV_LINENUM_GET(pMeiDev);
   pDrvMsgRemAtmOam->hdr.id               = MEI_DRV_MSG_ALM_REM_ATM_OAM_EXTR;
   pDrvMsgRemAtmOam->linkNo               = (unsigned int)linkNo;
   pDrvMsgRemAtmOam->extrCellFailedCnt    = (unsigned int)extrFailedCnt;

#if (MEI_SUPPORT_ATM_OAM_STATISTICS == 1)
   pDrvMsgRemAtmOam->extrCellFailedAllCnt = (unsigned int)pAtmOamDevCntrl->statistics.extrCellErrCnt;
   pDrvMsgRemAtmOam->extrFailedAlmAllCnt  = (unsigned int)pAtmOamDevCntrl->statistics.extrMsgErrCnt;
#else
   pDrvMsgRemAtmOam->extrCellFailedAllCnt = 0;
   pDrvMsgRemAtmOam->extrFailedAlmAllCnt  = 0;
#endif

   return IFX_SUCCESS;
}


/**
   This function setup a REM ATM OAM Alarm driver message.
   This event depends on the ATM OAM driver config and is generated if a
   modem alarm comes up.

\param
   pMeiDev      private device data
\param
   linkNo            link no (bearer channel).
\param
   extrFailedCnt     current number of extract cell failures of the incoming alarm.

\attention
   - Called on int-level
*/
IFX_int32_t MEI_DrvMsg_RemAtmOamDistribute(
                              MEI_DEV_T    *pMeiDev,
                              IFX_uint32_t   linkNo,
                              IFX_uint32_t   extrFailedCnt)
{
   IFX_uint32_t               distCount = 0;
   MEI_ATMOAM_DEV_CNTRL_T   *pAtmOamDevCntrl;
   MEI_DRV_MSG_RemAtmOam_t  drvMsgRemAtmOam;

   if (pMeiDev->pAtmOamDevCntrl)
   {
      pAtmOamDevCntrl = pMeiDev->pAtmOamDevCntrl;
   }
   else
   {
      PRN_ERR_INT_NL( MEI_DRV, MEI_DRV_PRN_LEVEL_ERR,
             ("MEI_DRV[%02d]: ERROR - dist drv msg ATM OAM Alarm, module not exist" MEI_DRV_CRLF
               , MEI_DRV_LINENUM_GET(pMeiDev)));

      return IFX_ERROR;
   }

   MEI_DrvMsg_RemAtmOamSet(pMeiDev, pAtmOamDevCntrl,
                             &drvMsgRemAtmOam, linkNo, extrFailedCnt);

   distCount = MEI_DistributeAutoMsg(
                              pMeiDev,
                              pMeiDev->pRootNfcRecvFirst,
                              (IFX_uint8_t *)&drvMsgRemAtmOam,
                              sizeof(MEI_DRV_MSG_RemAtmOam_t),
                              MEI_RECV_BUF_CTRL_DRIVER_MSG);

   if (distCount <= 0)
   {
      PRN_DBG_USR( MEI_DRV, MEI_DRV_PRN_LEVEL_NORMAL, MEI_DRV_LINENUM_GET(pMeiDev),
             ("MEI_DRV[%02d]: WARNING - no waiting user, discard driver Rem ATM OAM auto. msg!" MEI_DRV_CRLF,
               MEI_DRV_LINENUM_GET(pMeiDev)));
   }

   return IFX_SUCCESS;
}

#endif      /* #if (MEI_DRV_ATM_OAM_ENABLE == 1) */

#endif      /* #if (MEI_SUPPORT_DRIVER_MSG == 1) */



