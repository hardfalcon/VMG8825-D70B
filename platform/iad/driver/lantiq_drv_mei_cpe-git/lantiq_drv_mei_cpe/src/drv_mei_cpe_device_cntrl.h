#ifndef _DRV_MEI_CPE_DEVICE_CNTRL_H
#define _DRV_MEI_CPE_DEVICE_CNTRL_H
/******************************************************************************

                          Copyright (c) 2007-2015
                     Lantiq Beteiligungs-GmbH & Co. KG

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/* ==========================================================================
   Description : VRX device specific insert / extract defines.
   ========================================================================== */



/* ==========================================================================
   includes
   ========================================================================== */

/* get at first the driver configuration */
#include "drv_mei_cpe_config.h"

#include "ifx_types.h"
#include "drv_mei_cpe_api.h"

/* ==========================================================================
   VRX Register - Macro defs
   ========================================================================== */

/** Base Address of the TIMER memory mapping area */
#define MEI_DEV_MEM_MAP_BASE_ADDR_TIMER    0x000E8000

/** Base Address of the GPIO memory mapping area */
#define MEI_DEV_MEM_MAP_BASE_ADDR_GPIO     0x000EA000

/** address of the device OnChip Memory Control register */
#define MEI_DEV_REG_ADDR_OC_MEM_CTRL       (MEI_DEV_MEM_MAP_BASE_ADDR_TIMER + 0x20)

/** address of the device AFSEL register */
#define MEI_DEV_REG_ADDR_AFSEL             (MEI_DEV_MEM_MAP_BASE_ADDR_GPIO + 0x00)


/* ============================================================================
   Common Message handling macros
   ========================================================================= */

/** Default state after reset */
#define MEI_DRV_FSM_STATE_RESET                  0x0000
/** First State after a link initiation has been triggered. */
#define MEI_DRV_FSM_STATE_READY                  0x0001
/** Entered upon an initialization or STEADYSTATE failure ... */
#define MEI_DRV_FSM_STATE_FAIL                   0x0002
/** Following a successful Diagnostic Mode */
#define MEI_DRV_FSM_STATE_DIAG_COMPL             0x0003
/** Ddetection of a far-end GHS signal */
#define MEI_DRV_FSM_STATE_GHS                    0x0005
/** Training phase of initialization following GHS start-up */
#define MEI_DRV_FSM_STATE_FULLINIT               0x0006
/** STEADY-STATE TC not synchronized */
#define MEI_DRV_FSM_STATE_STEADY_NSYNC           0x0007
/** STEADY-STATE TC synchronized - Modem is fully operational */
#define MEI_DRV_FSM_STATE_STEADY_SYNC            0x0008
/** From GHS into a Diagnostic Mode initialization sequence */
#define MEI_DRV_FSM_STATE_DIAG_MODE              0x0009
/** TEST state */
#define MEI_DRV_FSM_STATE_TEST                   0x00F0

/** The driver don't take care about the current modem state */
#define MEI_DRV_FSM_STATE_DON_T_CARE             0xFFFF


/** return the modem message ID out from a driver msg ID definition */
#define MEI_DRV_MODEM_MSG_ID_GET(drvMsgDefine) \
                        ((IFX_uint16_t)(((IFX_uint32_t)drvMsgDefine) & 0x0000FFFF))

/** return the availability state out from a driver msg ID definition */
#define MEI_DRV_MODEM_AVAIL_STATE_GET(drvMsgDefine) \
                        ((IFX_uint16_t)((((IFX_uint32_t)drvMsgDefine) & 0xFFFF0000) >> 16))

/** check the availablility of the given message for the current state */
#define MEI_DRV_MSG_AVAILABLE(pMeiDev, drvMsgDefine) \
         ( ( ( MEI_DRV_MODEM_AVAIL_STATE_GET(drvMsgDefine) == MEI_DRV_FSM_STATE_DON_T_CARE ) || \
             ( MEI_DRV_MODEM_AVAIL_STATE_GET(drvMsgDefine) == MEI_DRV_MODEM_STATE_GET(pMeiDev) ) ) ? \
                IFX_TRUE : IFX_FALSE )


/* ==========================================================================
   VRX Device common functions.
   ========================================================================== */
extern IFX_int32_t MEI_DevCntlMsgAvailable(
                              MEI_DYN_CNTRL_T *pMeiDynCntrl,
                              IFX_uint32_t      drvMsgId);


/* ==========================================================================
   VRX Device Init functions.
   ========================================================================== */
#if (MEI_DRV_POWERSAVING_ENABLED == 1)
extern IFX_int32_t MEI_DevSetup_PowerSave(
                              MEI_DEV_T *pMeiDev);
#endif



#if (MEI_SUPPORT_DFE_GPA_ACCESS == 1)
/* ==========================================================================
   VRX Device - Debug messages (General Purpose Access - GPA).
   ========================================================================== */

/*
   General Purpose Access message ID's (online).
*/

#define CMD_DBG_MEM_MAP_WRITE          0xA173
#define ACK_DBG_MEM_MAP_WIRTE          CMD_DBG_MEM_MAP_WRITE
#define CMD_DBG_AUX_REGISTER_WRITE     0xA373
#define ACK_DBG_AUX_REGISTER_WRITE     CMD_DBG_AUX_REGISTER_WRITE

#define CMD_DBG_MEM_MAP_READ           0xA033
#define ACK_DBG_MEM_MAP_READ           CMD_DBG_MEM_MAP_READ
#define CMD_DBG_AUX_REGISTER_READ      0xA233
#define ACK_DBG_AUX_REGISTER_READ      CMD_DBG_AUX_REGISTER_READ


extern IFX_int32_t MEI_OnlineGpaWr(
                              MEI_DYN_CNTRL_T *pMeiDynCntrl,
                              IFX_boolean_t     aux,
                              IFX_uint32_t      addr,
                              IFX_uint32_t      val);

extern IFX_int32_t MEI_OnlineGpaRd(
                              MEI_DYN_CNTRL_T *pMeiDynCntrl,
                              IFX_boolean_t     aux,
                              IFX_uint32_t      addr,
                              IFX_uint32_t      *val);

extern IFX_int32_t MEI_OnlineOnGpaAckRecv(
                              MEI_DEV_T *pMeiDev,
                              CMV_STD_MESSAGE_T *pGpaMsg );

#endif   /* #if (MEI_SUPPORT_DFE_GPA_ACCESS == 1) */


#if (MEI_SUPPORT_VDSL2_ADSL_SWAP == 1)
/* ============================================================================
   SWAP VDSL2 - ADSL firmware
   ========================================================================= */


/* ==========================================================================
   SWAP: Macro defs
   ========================================================================== */

/** VRX HW register[PDSCR]: used for selecting VDSL2 ADSL mode */
#define MEI_DEV_CFG_FW_MODE_SELECT_REG        0xE208C

/** VRX HW register[PDSCR]: bit 29 is used for selecting VDSL2 ADSL mode */
#define MEI_DEV_CFG_FW_MODE_SELECT_BIT_POS    29

/** Local definition FW restart cmd - VDSL2/ADSL swap */
#define MEI_DEV_CMD_DSL_MODE_MODIFY           0x0141


/* ==========================================================================
   SWAP: typedefs
   ========================================================================== */

/**
   Firmware modes for startup
*/
typedef enum
{
   /** start the FW in VDSL2 mode */
   e_MEI_DEV_CFG_MODE_VDSL2 = 0,
   /** start the FW in ADSL mode */
   e_MEI_DEV_CFG_MODE_ADSL,
} MEI_DEV_CFG_FW_MODE_E;


/* ==========================================================================
   SWAP: device control functions for VDSL2 ADSL swap.
   ========================================================================== */

extern IFX_int32_t MEI_CMD_DSL_ModeModify(
                              MEI_DYN_CNTRL_T    *pMeiDynCntrl,
                              MEI_DEV_CFG_FW_MODE_E eFwMode);

extern IFX_int32_t MEI_DevCfgFwModeSelect(
                              MEI_DEV_T             *pMeiDev,
                              MEI_DEV_CFG_FW_MODE_E eFwMode);

extern IFX_int32_t MEI_DevCfgFwModeSwap(
                              MEI_DYN_CNTRL_T       *pMeiDynCntrl,
                              MEI_DEV_CFG_FW_MODE_E eFwMode);

#endif      /* #if (MEI_SUPPORT_VDSL2_ADSL_SWAP == 1) */


#if (MEI_DRV_ATM_OAM_ENABLE == 1)

/* ==========================================================================
   ATM OAM part
   ========================================================================== */
#include "drv_mei_cpe_atmoam.h"


/* ==========================================================================
   ATM OAM: Macro defs
   ========================================================================== */

/**
   Local definition of the line extract message ID
   This must match with the official one from the message cataloge!
*/


#define MEI_MSG_EVT_ATMCELLLINEEXTRACT           0x5211
#define MEI_DRV_EVT_ATMCELLLINEEXTRACT           MEI_MSG_EVT_ATMCELLLINEEXTRACT

#define MEI_MSG_ALM_ATMCELLEXTRACTFAILED         0x5311
#define MEI_DRV_ALM_ATMCELLEXTRACTFAILED         MEI_MSG_ALM_ATMCELLEXTRACTFAILED

#define MEI_MSG_EVT_ATMCELLLINEINSERTSTATUSGET   0x5511
#define MEI_DRV_EVT_ATMCELLLINEINSERTSTATUSGET   MEI_MSG_EVT_ATMCELLLINEINSERTSTATUSGET

#define MEI_MSG_CMD_ATMINSERTEXTRACT_CONTROL     0x5051
#define MEI_DRV_CMD_ATMINSERTEXTRACT_CONTROL \
         (MEI_MSG_CMD_ATMINSERTEXTRACT_CONTROL | (MEI_DRV_FSM_STATE_RESET << 16))

#define MEI_MSG_CMD_ATMCELLLINEINSERT            0x5151
#define MEI_DRV_CMD_ATMCELLLINEINSERT \
         (MEI_MSG_CMD_ATMCELLLINEINSERT | (MEI_DRV_FSM_STATE_STEADY_SYNC << 16))

#define MEI_MSG_CMD_ATMCELLLINEINSERTSTATUSGET   0x5511
#define MEI_DRV_CMD_ATMCELLLINEINSERTSTATUSGET \
         (MEI_MSG_CMD_ATMCELLLINEINSERTSTATUSGET | (MEI_DRV_FSM_STATE_STEADY_SYNC << 16))

#define MEI_MSG_CMD_ATMINSERTEXTRACTSTATSGET     0x5411
#define MEI_DRV_CMD_ATMINSERTEXTRACTSTATSGET \
         (MEI_MSG_CMD_ATMINSERTEXTRACTSTATSGET | (MEI_DRV_FSM_STATE_STEADY_SYNC << 16))


/* ==========================================================================
   ATM OAM: device control functions for ATM OAM insert extract.
   ========================================================================== */

extern IFX_int32_t MEI_ATMOAM_CMD_InsExtControl(
                              MEI_DYN_CNTRL_T        *pMeiDynCntrl,
                              MEI_ATMOAM_DEV_CNTRL_T *pAtmOamDevCntrl);

extern IFX_int32_t MEI_ATMOAM_CMD_InsExtStatsGet(
                              MEI_DYN_CNTRL_T             *pMeiDynCntrl,
                              MEI_ATMOAM_DEV_CNTRL_T      *pAtmOamDevCntrl,
                              IOCTL_MEI_ATMOAM_counter_t  *pAtmOamStats);

extern IFX_int32_t MEI_ATMOAM_CMD_InsStatusGet(
                              MEI_DYN_CNTRL_T             *pMeiDynCntrl,
                              MEI_ATMOAM_DEV_CNTRL_T      *pAtmOamDevCntrl,
                              IFX_uint32_t                  *pInsStatus);

extern IFX_int32_t MEI_ATMOAM_CMD_AtmCellLineInsert(
                              MEI_DYN_CNTRL_T             *pMeiDynCntrl,
                              MEI_ATMOAM_DEV_CNTRL_T      *pAtmOamDevCntrl,
                              IOCTL_MEI_ATMOAM_rawCell_t  *pRawAtmCellBlock,
                              IFX_uint32_t                  cellCnt);


extern IFX_int32_t MEI_ATMOAM_doEVT_AtmCellLineInsertStatus(
                              MEI_DEV_T                *pMeiDev,
                              MEI_ATMOAM_DEV_CNTRL_T   *pAtmOamDevCntrl,
                              CMV_STD_MESSAGE_T          *pModemMsg);

extern IFX_int32_t MEI_ATMOAM_doALM_AtmCellExtractFailed(
                              MEI_DEV_T                *pMeiDev,
                              MEI_ATMOAM_DEV_CNTRL_T   *pAtmOamDevCntrl,
                              CMV_STD_MESSAGE_T          *pModemMsg,
                              IFX_uint32_t               *pLinkNo,
                              IFX_uint32_t               *pFailCount);

extern IFX_int32_t MEI_ATMOAM_doEVT_AtmCellExtract(
                              MEI_DEV_T                   *pMeiDev,
                              MEI_ATMOAM_DEV_CNTRL_T      *pAtmOamDevCntrl,
                              CMV_STD_MESSAGE_T             *pModemMsg,
                              MEI_ATMOAM_CELL_BUFFER_T    *pRxAtmCells,
                              IFX_uint8_t                   bufferSize_Cells);

#endif      /* #if (MEI_DRV_ATM_OAM_ENABLE == 1) */


/* ============================================================================
   Clear EOC Insert/Extract part
   ========================================================================= */
#if (MEI_DRV_CLEAR_EOC_ENABLE == 1)

#include "drv_mei_cpe_clear_eoc.h"


/* ==========================================================================
   Clear EOC: Macro defs
   ========================================================================== */

/**
   Local definition of the used message ID's
   This must match with the official one from the message cataloge!
*/
#define MEI_MSG_CMD_CLEAREOC_CONFIGURE           0x0A49
#define MEI_DRV_CMD_CLEAREOC_CONFIGURE \
         (MEI_MSG_CMD_CLEAREOC_CONFIGURE | (MEI_DRV_FSM_STATE_RESET << 16))

#define MEI_MSG_CMD_CLEAREOC_TXTRIGGER           0x0949
#define MEI_DRV_CMD_CLEAREOC_TXTRIGGER \
         (MEI_MSG_CMD_CLEAREOC_TXTRIGGER | (MEI_DRV_FSM_STATE_STEADY_SYNC << 16))

#define MEI_MSG_CMD_CLEAREOC_WRITE               0x5143
#define MEI_DRV_CMD_CLEAREOC_WRITE \
         (MEI_MSG_CMD_CLEAREOC_WRITE | (MEI_DRV_FSM_STATE_STEADY_SYNC << 16))

#define MEI_MSG_CMD_CLEAREOC_READ                0x5203
#define MEI_DRV_CMD_CLEAREOC_READ \
         (MEI_MSG_CMD_CLEAREOC_READ | (MEI_DRV_FSM_STATE_STEADY_SYNC << 16))

#define MEI_MSG_CMD_CLEAREOCSTATUSGET            0x0B09
#define MEI_DRV_CMD_CLEAREOCSTATUSGET \
         (MEI_MSG_CMD_CLEAREOCSTATUSGET | (MEI_DRV_FSM_STATE_STEADY_SYNC << 16))

#define MEI_MSG_EVT_CLEAREOC_READ                0x5203
#define MEI_DRV_EVT_CLEAREOC_READ                MEI_MSG_EVT_CLEAREOC_READ

#define MEI_MSG_EVT_CLEAREOCSTATUSGET            0x0B09
#define MEI_DRV_EVT_CLEAREOCSTATUSGET            MEI_MSG_EVT_CLEAREOCSTATUSGET


/* ==========================================================================
   Clear EOC: device control functions for Clear EOC.
   ========================================================================== */

extern IFX_int32_t MEI_CEOC_CMD_ClearEOC_Configure(
                              MEI_DYN_CNTRL_T      *pMeiDynCntrl,
                              MEI_CEOC_DEV_CNTRL_T *pCEocDevCntrl);

extern IFX_int32_t MEI_CEOC_CMD_ClearEOC_TxTrigger(
                              MEI_DYN_CNTRL_T      *pMeiDynCntrl,
                              MEI_CEOC_DEV_CNTRL_T *pCEocDevCntrl);

extern IFX_int32_t MEI_CEOC_CMD_ClearEOC_Write(
                              MEI_DYN_CNTRL_T         *pMeiDynCntrl,
                              MEI_CEOC_DEV_CNTRL_T    *pCEocDevCntrl,
                              MEI_CEOC_FRAME_BUFFER_T *pCEocFrameBuf);

extern IFX_int32_t MEI_CEOC_CMD_ClearEOC_Read(
                              MEI_DYN_CNTRL_T         *pMeiDynCntrl,
                              MEI_CEOC_DEV_CNTRL_T    *pCEocDevCntrl,
                              MEI_CEOC_FRAME_BUFFER_T *pCEocFrameBuf);

extern IFX_int32_t MEI_CEOC_CMD_ClearEOCStatusGet(
                              MEI_DYN_CNTRL_T       *pMeiDynCntrl,
                              MEI_CEOC_DEV_CNTRL_T  *pCEocDevCntrl);

extern IFX_int32_t MEI_CEOC_doEVT_ClearEOC_Read(
                              MEI_DEV_T               *pMeiDev,
                              MEI_CEOC_DEV_CNTRL_T    *pCEocDevCntrl,
                              CMV_STD_MESSAGE_T         *pModemMsg,
                              MEI_CEOC_FRAME_BUFFER_T *pCEocFrameBuf);

extern IFX_int32_t MEI_CEOC_doEVT_ClearEOCStatusGet(
                              MEI_DEV_T            *pMeiDev,
                              MEI_CEOC_DEV_CNTRL_T *pCEocDevCntrl,
                              CMV_STD_MESSAGE_T      *pModemMsg);


#endif      /* #if (MEI_DRV_CLEAR_EOC_ENABLE == 1) */

#endif      /* #ifndef _DRV_MEI_CPE_DEVICE_CNTRL_H */


