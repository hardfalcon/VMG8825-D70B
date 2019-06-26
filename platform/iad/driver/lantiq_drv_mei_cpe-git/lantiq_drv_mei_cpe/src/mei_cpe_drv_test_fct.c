/******************************************************************************

                          Copyright (c) 2007-2015
                     Lantiq Beteiligungs-GmbH & Co. KG

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/* ==========================================================================
   Description : Common test routines for the VRX driver test application.
   ========================================================================== */

/* ==========================================================================
   includes
   ========================================================================== */


#if 0
#ifdef VXWORKS
#include <vxworks.h>
#include <iolib.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>

/* open */
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#ifdef LINUX
#include <fcntl.h>
#include <unistd.h>
#include <sys/select.h>
#endif

#else
#include "mei_cpe_appl_osmap.h"
#endif



/* get common test routines */
#include "mei_cpe_drv_test_fct.h"

/* get interface and configuration */
#include "drv_mei_cpe_interface.h"

/* get the CMV message format */
#include "cmv_message_format.h"

/* ==========================================================================
   Macros
   ========================================================================== */

#ifdef LINUX
#define MEI_IOCTL_ARG unsigned long
#endif

#ifdef VXWORKS
#define MEI_IOCTL_ARG int
#endif


/* ==========================================================================
   Local variables
   ========================================================================== */

static unsigned int MEI_DumpBufWr[0x100];
static unsigned int MEI_DumpBufRd[0x100];

static unsigned int MEI_DmaRdBuf[0x0FF];
static unsigned int MEI_DmaWrBuf[0x0FF];


static unsigned char  *pVrxFileBuffer = NULL;
static IOCTL_MEI_fwDownLoad_t MEI_FwDl;

static IOCTL_MEI_dbgAccess_t        MEI_DbgAccess;
static IOCTL_MEI_GPA_accessInOut_t  MEI_GpaAccessIo;


/* ==========================================================================
   Global variables
   ========================================================================== */

/* mailbox write / read buffer */
CMV_MESSAGE_ALL_T MEI_WrCmvMsg;
CMV_MESSAGE_ALL_T MEI_RdCmvMsg;

unsigned char MEI_EtHRecvBuf[1580];
unsigned char MEI_EtHSendBuf[1580];

IOCTL_MEI_arg_t      MEI_IoctArgs;
IOCTL_MEI_regInOut_t MEI_RegIo;
IOCTL_MEI_devInit_t  MEI_DataDevInit =
   { {0}, TEST_MEI_IRQ_NUM, TEST_MEI_BASE_ADDR };


/* ==========================================================================
   Local Routines
   ========================================================================== */
static void MEI_log_cmv_hdr(MEIOS_File_t *streamOut, char *pLogStr, CMV_MESSAGE_ALL_T *pCmvMsg);

/* ==========================================================================
   Common test routines.
   ========================================================================== */

/**
   Print CMV Message Header.
*/
static void MEI_log_cmv_hdr(MEIOS_File_t *streamOut, char *pLogStr, CMV_MESSAGE_ALL_T *pCmvMsg)
{
   MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
           "%s, CMD-Hdr - 0x%04X  0x%04X 0x%04X 0x%04X 0x%04X 0x%04X" MEIOS_CRLF,
           (pLogStr) ? pLogStr : "Show",
           pCmvMsg->rawMsg[0],
           pCmvMsg->rawMsg[1],
           pCmvMsg->rawMsg[2],
           pCmvMsg->rawMsg[3],
           pCmvMsg->rawMsg[4],
           pCmvMsg->rawMsg[5] );

   return;
}


/**
   Print CMV Message Header.
*/
void MEI_log_ifx_msg(MEIOS_File_t *streamOut, char *pLogStr, IOCTL_MEI_message_t *pCntrl)
{
   int i;
   unsigned short *pPayl_16bit;

   MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
            "%s, IFX-Msg - Ctrl: 0x%08X, Class: 0x%04X, ID: 0x%04X, size: 0x%04X (%d)" MEIOS_CRLF,
            (pLogStr) ? pLogStr : "Show",
            pCntrl->msgCtrl, pCntrl->msgClassifier, pCntrl->msgId,
            pCntrl->paylSize_byte, pCntrl->paylSize_byte);

   switch (pCntrl->msgCtrl)
   {
      case MEI_MSG_CTRL_DRIVER_MSG:
         MEI_log_drv_msg(streamOut, pCntrl);
         break;

      case MEI_MSG_CTRL_ATMOAM_CELL_MSG:
         MEI_log_atmoam_cell_msg(streamOut, pCntrl);
         break;

      case MEI_MSG_CTRL_MODEM_MSG:
      default:
         if (pCntrl->paylSize_byte)
         {
            pPayl_16bit = (unsigned short *)pCntrl->pPayload;

            MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
                     "DATA - " MEIOS_CRLF);
            for (i=0; i < pCntrl->paylSize_byte/2;i++)
            {
               MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
                        "\t[%2d]: 0x%04X 0x%04X" MEIOS_CRLF,
                        i, pPayl_16bit[i],
                        ((pCntrl->paylSize_byte/2) > (i+1)) ? pPayl_16bit[i+1]:0);
               i++;
            }
         }
         break;
   }

   return;
}


/**
   Print Ethernet Frame.
*/
void MEI_log_eth_frame(
                  MEIOS_File_t  *streamOut,
                  char          *pLogStr,
                  unsigned char *pEthFrameStr,
                  int           frameSize_Byte)
{
   MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
            "Eth Frame %s (%d):" MEIOS_CRLF
            "%02X %02X %02X %02X %02X %02X  %02X %02X %02X %02X %02X %02X " MEIOS_CRLF
            "%02X %02X %02X %02X %02X %02X %02X %02X" MEIOS_CRLF,
            pLogStr, frameSize_Byte,
            pEthFrameStr[0], pEthFrameStr[1], pEthFrameStr[2], pEthFrameStr[3], pEthFrameStr[4], pEthFrameStr[5],
            pEthFrameStr[6], pEthFrameStr[7], pEthFrameStr[8], pEthFrameStr[9], pEthFrameStr[10], pEthFrameStr[11],
            pEthFrameStr[12], pEthFrameStr[13], pEthFrameStr[14], pEthFrameStr[15],
            pEthFrameStr[16], pEthFrameStr[17], pEthFrameStr[18], pEthFrameStr[19] );

   return;
}


/**
   Print CMV Message Header.
*/
void MEI_log_drv_msg(MEIOS_File_t *streamOut, IOCTL_MEI_message_t *pCntrl)
{
   MEI_DRV_MSG_all_t *pDrvMsg = (MEI_DRV_MSG_all_t *)pCntrl->pPayload;

   switch(pDrvMsg->hdr.id)
   {
      case MEI_DRV_MSG_IF_ROM_START_EVT:
      case MEI_DRV_MSG_IF_ROM_START_ALM:
         MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
            "DRV MSG: ROM_START[%s] - reason = 0x%X, state Change: %d --> %d" MEIOS_CRLF MEIOS_CRLF,
            (pDrvMsg->romStart.hdr.id & MEI_DRV_MSG_IF_FLAG_TYPE_ALARM) ? "ALM" : "EVT",
             pDrvMsg->romStart.reason,
             pDrvMsg->romStart.prevDrvState, pDrvMsg->romStart.newDrvState);

         break;
      case MEI_DRV_MSG_IF_REM_DL_DONE_EVT:
      case MEI_DRV_MSG_IF_REM_DL_DONE_ALM:
         MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
            "DRV MSG: REM_FW_DL[%s] - OP-Result = 0x%X, sessionId = 0x%X" MEIOS_CRLF
            "\tbytes = %d, time_ms = %d --> %s" MEIOS_CRLF MEIOS_CRLF,
            (pDrvMsg->remFwDl.hdr.id & MEI_DRV_MSG_IF_FLAG_TYPE_ALARM) ? "ALM" : "EVT",
            pDrvMsg->remFwDl.opResult, pDrvMsg->remFwDl.sessionId,
            pDrvMsg->remFwDl.sendBytes, pDrvMsg->remFwDl.elapsedTime_ms,
            pDrvMsg->remFwDl.resStr);
         break;
      default :
         MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
            "DRV MSG: UNKNOWN[???] - drv msg ID = 0x%08X" MEIOS_CRLF MEIOS_CRLF,
            pDrvMsg->hdr.id);
         break;
   }
   return;
/*
 **
 *
 *  FILENAME: F:\comacsd_driver\drv_vdsl2_dfe\src\vrx_drv_test_fct.c
 *
 *  PARAMETERS:
 *
 *  DESCRIPTION:
 *
 *  RETURNS:
 *
 */
}

/**
   Print VRX ATM OAM cell Message.
*/
void MEI_log_atmoam_cell_msg(MEIOS_File_t *streamOut, IOCTL_MEI_message_t *pCntrl)
{
   unsigned char  *pAtmCell;
   IOCTL_MEI_ATMOAM_drvAtmCells_t *pAtmOamUsrCells = (IOCTL_MEI_ATMOAM_drvAtmCells_t*)pCntrl->pPayload;
   int cellIdx, maxCellIdx = (pAtmOamUsrCells->cellCount <= 4) ? pAtmOamUsrCells->cellCount : 4;

   MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
      "MEI MSG: ATM OAM - cntrl = 0x%08X, size = %d, cellCnt = %d" MEIOS_CRLF,
      pCntrl->msgCtrl, pCntrl->paylSize_byte, pAtmOamUsrCells->cellCount);

   for (cellIdx = 0; cellIdx < maxCellIdx; cellIdx++)
   {
      pAtmCell = pAtmOamUsrCells->atmCells[cellIdx].ATMcell;
      MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
         "\tATM OAM - cell[%02d]: "
         "0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X" MEIOS_CRLF,
         cellIdx,
         pAtmCell[0], pAtmCell[1], pAtmCell[2], pAtmCell[4],
         pAtmCell[4], pAtmCell[5], pAtmCell[6], pAtmCell[7]);
   }

   return;
}

/**
   open an VRX device
*/
int MEI_open_dev(MEIOS_File_t *streamOut, int devNum, char *pPrefixName, char *pDevBaseName)
{
   int fd;
   char buf[128];

   if (devNum & 0x80)
   {
      MEIOS_SNPrintf( buf,
         sizeof(buf),
         "%s/%s/cntrl%d",
         pPrefixName, pDevBaseName, devNum & ~0x80 );
   }
   else
   {
      MEIOS_SNPrintf( buf,
         sizeof(buf),
         "%s/%s/%d",
         pPrefixName, pDevBaseName, devNum);
   }

   /* open device */
   MEIOS_Printf( TEST_MEI_DBG_PREFIX
      "open device: %s." MEIOS_CRLF,buf);

   fd = MEIOS_DeviceOpen(buf);
   if( fd < 0 )
   {
      MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
               "Cannot open device[%02d] %s." MEIOS_CRLF, devNum, buf);
      MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
               "Return = %d" MEIOS_CRLF, -1);
      return -1;
   }

   return fd;
}


/**
   Get VRX driver configuration.
   - ioctl(..., FIO_MEI_REQ_CONFIG, ...);

*/
int MEI_req_cfg(MEIOS_File_t *streamOut, int fd)
{
   int ret;
   IOCTL_MEI_reqCfg_t *pReqCfg = &MEI_IoctArgs.req_cfg;

   if ( (ret = MEIOS_DeviceControl(fd, FIO_MEI_REQ_CONFIG, (MEI_IOCTL_ARG)&MEI_IoctArgs.req_cfg)) < 0 )
   {
      MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
              "ERROR - request config: ret = %d, retCode = %d" MEIOS_CRLF,
              ret, pReqCfg->ictl.retCode);
      return -1;
   }

   MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
           "ioct(FIO_MEI_REQ_CONFIG) ===================" MEIOS_CRLF);

   MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
           "REQ_CONFIG[%02d-%02d]: phy addr 0x%08X (virt 0x%08X), IRQ %d" MEIOS_CRLF,
           pReqCfg->devNum, pReqCfg->currOpenInst,
           pReqCfg->phyBaseAddr, pReqCfg->virtBaseAddr, pReqCfg->usedIRQ);

   MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
           "REQ_CONFIG[%02d-%02d]: phy PDBRAM addr 0x%08X (virt 0x%08X) - VR10 only" MEIOS_CRLF,
           pReqCfg->devNum, pReqCfg->currOpenInst,
           pReqCfg->phyPDBRAMaddr, pReqCfg->virtPDBRAMaddr);

   MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
           "REQ_CONFIG[%02d-%02d]: curr DrvState = %d, curr ModemFSM = %d" MEIOS_CRLF,
           pReqCfg->devNum, pReqCfg->currOpenInst,
           pReqCfg->currDrvState, pReqCfg->currModemFsmState);

   MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
           "REQ_CONFIG[%02d-%02d]: BOOT ARC2ME - addr 0x%08X size 0x%X (drv size 0x%X)" MEIOS_CRLF,
           pReqCfg->devNum, pReqCfg->currOpenInst,
           pReqCfg->Arc2MeBootMbAddr, pReqCfg->Arc2MeBootMbSize, pReqCfg->drvArc2MeMbSize);

   MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
           "REQ_CONFIG[%02d-%02d]: BOOT ME2ARC - addr 0x%08X size 0x%X (drv size 0x%X)" MEIOS_CRLF,
           pReqCfg->devNum, pReqCfg->currOpenInst,
           pReqCfg->Me2ArcBootMbAddr, pReqCfg->Me2ArcBootMbSize, pReqCfg->drvMe2ArcMbSize);

   MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
           "REQ_CONFIG[%02d-%02d]: BootMode %d, ChipId %d" MEIOS_CRLF,
           pReqCfg->devNum, pReqCfg->currOpenInst,
           pReqCfg->bootMode, pReqCfg->chipId);

   MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
           "REQ_CONFIG[%02d-%02d]: Online ARC2ME - addr 0x%08X size 0x%X (drv size 0x%X)" MEIOS_CRLF,
           pReqCfg->devNum, pReqCfg->currOpenInst,
           pReqCfg->Arc2MeOnlineMbAddr, pReqCfg->Arc2MeOnlineMbSize, pReqCfg->drvArc2MeMbSize);

   MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
           "REQ_CONFIG[%02d-%02d]: Online ME2ARC - addr 0x%08X size 0x%X (drv size 0x%X)" MEIOS_CRLF,
           pReqCfg->devNum, pReqCfg->currOpenInst,
           pReqCfg->Me2ArcOnlineMbAddr, pReqCfg->Me2ArcOnlineMbSize, pReqCfg->drvMe2ArcMbSize);

   return 0;
}


/**
   Get VRX driver configuration.
   - ioctl(..., FIO_MEI_REQ_STAT, ...);

*/
int MEI_req_stat(MEIOS_File_t *streamOut, int fd)
{
   int ret;
   IOCTL_MEI_statistic_t *pReqStat = &MEI_IoctArgs.req_stat;

   if ( (ret = MEIOS_DeviceControl(fd, FIO_MEI_REQ_STAT, (MEI_IOCTL_ARG)&MEI_IoctArgs.req_stat)) < 0 )
   {
      MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
         "ERROR - request statistics: ret = %d, retCode = %d" MEIOS_CRLF,
         ret, pReqStat->ictl.retCode);
      return -1;
   }

   MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
            "ioct(FIO_MEI_REQ_STAT) ===================" MEIOS_CRLF);

   MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
            "DrvSwRst   = %d\tMeiHwRst    = %d" MEIOS_CRLF,
            pReqStat->drvSwRstCount, pReqStat->meiHwRstCount);
   MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
            "GP1 Int    = %d\tMsgAv Int   = %d" MEIOS_CRLF,
            pReqStat->dfeGp1IntCount, pReqStat->dfeMsgAvIntCount);
   MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
            "FwDownl    = %d\tCodeSwap    = %d" MEIOS_CRLF,
            pReqStat->fwDlCount, pReqStat->dfeCodeSwapCount);
   MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
            "FwDownlErr = %d" MEIOS_CRLF,
            pReqStat->fwDlErrCount);
   MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
            "TxMsg      = %d\tRxAck       = %d" MEIOS_CRLF,
            pReqStat->sendMsgCount, pReqStat->recvAckCount);
   MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
            "RxMsg      = %d\tRxMsgDisc   = %d" MEIOS_CRLF,
            pReqStat->recvMsgCount, pReqStat->recvMsgDiscardCount);
   MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
            "RxMsgErr   = %d\tTxMsgErr    = %d" MEIOS_CRLF,
            pReqStat->recvMsgErrCount, pReqStat->errorCount);
   MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
            "Nfc        = %d\tNfcDisc     = %d" MEIOS_CRLF,
            pReqStat->recvNfcCount, pReqStat->recvNfcDiscardCount);
   MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
            "NfcDist    = %d\tNfcDistDisc = %d" MEIOS_CRLF MEIOS_CRLF,
            pReqStat->recvNfcDistCount, pReqStat->recvNfcDistDiscardCount);

   return 0;
}


/**
   Do the VRX initialisation.
   - set base address
   - set IRQ (IRQ = 0 --> poll mode)
*/
int MEI_init_dev(MEIOS_File_t *streamOut, int fd, IOCTL_MEI_devInit_t *pDevInit)
{
   int ret;

   MEIOS_Printf( TEST_MEI_DBG_PREFIX
           "ioct(FIO_MEI_DEV_INIT): baseaddr = 0x%08X, IRQ = %d" MEIOS_CRLF,
           (unsigned int)pDevInit->meiBaseAddr, (unsigned int)pDevInit->usedIRQ);

   if ( (ret = MEIOS_DeviceControl(fd, FIO_MEI_DEV_INIT, (MEI_IOCTL_ARG)pDevInit)) < 0 )
   {
      MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
         "ERROR - device init: errno = %d, ret= %d, retCode = %d" MEIOS_CRLF,
         errno, ret, pDevInit->ictl.retCode);
      return -1;
   }
   else
   {
      if (pDevInit->ictl.retCode == e_MEI_ERR_ALREADY_DONE)
      {
         MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
            "WARNING - device init: already done (device tree data used), retCode = %d"
            MEIOS_CRLF, pDevInit->ictl.retCode);
      }
   }

   return 0;
}

/**
   Do the VRX initialisation.
   - set base address
   - set IRQ (IRQ = 0 --> poll mode)
*/
int MEI_init_drv(MEIOS_File_t *streamOut, int fd, IOCTL_MEI_drvInit_t *pDrvInit)
{
   int ret;

   if ( (ret = MEIOS_DeviceControl(fd, FIO_MEI_DRV_INIT, (MEI_IOCTL_ARG)pDrvInit)) < 0 )
   {
      MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
         "ERROR - driver init: errno = %d, ret= %d, retCode = %d" MEIOS_CRLF,
         errno, ret, pDrvInit->ictl.retCode);
      return -1;
   }

   /* current settings */
   MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
           "ioct(FIO_MEI_DRV_INIT): COMMON" MEIOS_CRLF
           "blockTimeout = %d, waitModemMsg_ms = %d, waitFirstResp_ms = %d" MEIOS_CRLF,
           pDrvInit->blockTimeout, pDrvInit->waitModemMsg_ms,
           pDrvInit->waitFirstResp_ms & 0x7FFFFFFF);

   MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
           "ioct(FIO_MEI_DRV_INIT): Bootmode 8" MEIOS_CRLF
           "bmWaitForDl_ms = %d, bmWaitDlInit_ms = %d, bmWaitNextBlk_ms = %d" MEIOS_CRLF
           "bmDatawidth = %d, bmWaitStates = %d" MEIOS_CRLF,
           pDrvInit->bmWaitForDl_ms, pDrvInit->bmWaitDlInit_ms, pDrvInit->bmWaitNextBlk_ms,
           pDrvInit->bmDatawidth, pDrvInit->bmWaitStates  );

   return 0;
}

/**
   Reset VRX driver.
*/
int MEI_drv_reset(MEIOS_File_t *streamOut, int fd, unsigned int resetMode)
{
   int ret;
   IOCTL_MEI_reset_t reset_args;

   MEIOS_MemSet(&reset_args, 0x00, sizeof(IOCTL_MEI_reset_t));

   reset_args.rstMode = (IOCTL_MEI_resetMode_e)(resetMode & 0x0000FFFF);
   reset_args.rstSelMask = (unsigned int)((resetMode & 0xFFFF0000) >> 16);

   MEIOS_Printf( TEST_MEI_DBG_PREFIX
           "ioct(FIO_MEI_RESET): mode = 0x%08X" MEIOS_CRLF,
           (unsigned int)resetMode);


   if ( (ret = MEIOS_DeviceControl(fd, FIO_MEI_RESET, (MEI_IOCTL_ARG)&reset_args)) < 0 )
   {
      MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
               "ERROR - ioct(MEI_RESET) - ret = %d, retCode = %d" MEIOS_CRLF,
               ret, reset_args.ictl.retCode);
      return -1;
   }

   return 0;
}

/**
   Set VRX driver debug level.
*/
int MEI_x_drv_set_trace(MEIOS_File_t *streamOut, int fd, unsigned int debugArgs, int *pParamArr)
{
   int i, ret;
   IOCTL_MEIX_debugSet_t dbgTrace_args;


   MEIOS_Printf( TEST_MEI_DBG_PREFIX
           "ioct(FIO_MEIX_CH_TRACE_SET): mode = 0x%08X, mask[0] = 0x%08X" MEIOS_CRLF,
            debugArgs, (unsigned int)pParamArr[0]);

   MEIOS_MemSet(&dbgTrace_args, 0x00, sizeof(IOCTL_MEIX_debugSet_t));

   /** new trace/log level */
   /** debug block mask (01 = com, 02 = MEI, 04 = rom, 08 = boot) */
   dbgTrace_args.level     = (debugArgs & 0x0000FFFF);
   dbgTrace_args.blockMask = (unsigned int)((debugArgs & 0xFFFF0000) >> 16);

   for(i=0; i<4; i++)
   {
      if(pParamArr[i] == -1)
         dbgTrace_args.dfeMask[i] = 0;
      else
         dbgTrace_args.dfeMask[i] = pParamArr[i];
   }

   if ( ( ret = MEIOS_DeviceControl(fd, FIO_MEIX_CH_TRACE_SET, (MEI_IOCTL_ARG)&dbgTrace_args)) != 0 )
   {
      MEIOS_Printf( TEST_MEI_DBG_PREFIX
              "ERROR - ioct(FIO_MEIX_CH_TRACE_SET) - mask[0] = 0x%08X, ret = %d, retCode = %d" MEIOS_CRLF,
              dbgTrace_args.dfeMask[0], ret, dbgTrace_args.xIctl.retCode );
      return -1;
   }

   return 0;
}

/**
   Switch on/off the VRX driver loop
   --> ioctl(..., FIO_MEI_DRV_LOOP, ...)
   - 0: loop off
   - 1: loop on
*/
int MEI_mailbox_loop(MEIOS_File_t *streamOut, int fd, int on_off)
{
   int ret;
   IOCTL_MEI_drvLoop_t drvLoops;

   MEIOS_Printf( TEST_MEI_DBG_PREFIX
           "ioct(FIO_MEI_DRV_LOOP): %s" MEIOS_CRLF,
           (on_off) ? "ON":"OFF");

   MEIOS_MemSet(&drvLoops, 0x00, sizeof(IOCTL_MEI_drvLoop_t));
   drvLoops.loopEnDis = (on_off) ? 1 : 0;

   if ( (ret = MEIOS_DeviceControl(fd, FIO_MEI_DRV_LOOP, (MEI_IOCTL_ARG)&drvLoops)) < 0 )
   {
      MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
               "ERROR - ioct(.., FIO_MEI_DRV_LOOP, ..) - ret = %d, retCode = %d" MEIOS_CRLF,
               ret, drvLoops.ictl.retCode);
      return -1;
   }
   return 0;
}


/**
   Print board driver version to console.

   Arguments passed to ioctl(),
   - nCmd:       FIO_MEI_VERSION_GET
   - nArgument:  (char *) buffer for version string (size: 80)

   \return
   -1 on error
   otherwise return code of ioctl() call
*/
int MEI_GetVersion(MEIOS_File_t *streamOut, int fd)
{
   int ret ;
   char verBuf[128] ;
   IOCTL_MEI_drvVersion_t drvVersion;

   MEIOS_MemSet(&verBuf, 0x0, sizeof(verBuf));
   MEIOS_MemSet(&drvVersion, 0x00, sizeof(IOCTL_MEI_drvVersion_t));

   drvVersion.strSize     = 127;
   drvVersion.pVersionStr = verBuf;


   ret = MEIOS_DeviceControl(fd, FIO_MEI_VERSION_GET, (MEI_IOCTL_ARG)&drvVersion) ;
   if (ret < 0)
   {
      /* ERROR while ioctl */
      MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
               "ERROR - request version, ret = %d, retCode = %d" MEIOS_CRLF,
                ret, drvVersion.ictl.retCode);
      return -1;
   }
   else
   {
      verBuf[127] = '\0';
      MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
               "Driver version = 0x%08X \"%s\"." MEIOS_CRLF,
               drvVersion.versionId ,verBuf);
      return 0;
   }
}

/**
   Get a MEI register.
   --> ioctl(..., FIO_MEI_REG_GET, ...)
*/
int MEI_get_reg(MEIOS_File_t *streamOut, int fd, IOCTL_MEI_regInOut_t *pRegIO)
{
   int ret;

   MEIOS_Printf( TEST_MEI_DBG_PREFIX
           "ioct(FIO_MEI_REG_GET): MEI[0x%X]" MEIOS_CRLF, pRegIO->addr);

   if ( (ret = MEIOS_DeviceControl(fd, FIO_MEI_REG_GET, (MEI_IOCTL_ARG)pRegIO)) < 0 )
   {
      MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
               "ERROR - ioct(.., FIO_MEI_REG_GET, ..) - ret = %d, retCode = %d" MEIOS_CRLF,
               ret, pRegIO->ictl.retCode);
      return -1;
   }

   MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
           "Get: MEI Reg[0x%X] = 0x%08X" MEIOS_CRLF, pRegIO->addr, pRegIO->value);

   return 0;
}

/**
   Set a MEI register.
   --> ioctl(..., FIO_MEI_REG_SET, ...)
*/
int MEI_set_reg(MEIOS_File_t *streamOut, int fd, IOCTL_MEI_regInOut_t *pRegIO)
{
   int ret;

   MEIOS_Printf( TEST_MEI_DBG_PREFIX
           "ioct(FIO_MEI_REG_SET): MEI[0x%X] = 0x%08X" MEIOS_CRLF,
           pRegIO->addr, pRegIO->value);

   if ( (ret = MEIOS_DeviceControl(fd, FIO_MEI_REG_SET, (MEI_IOCTL_ARG)pRegIO)) < 0 )
   {
      MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
              "ERROR - ioct(.., FIO_MEI_REG_SET, ..) - ret = %d, retCode = %d" MEIOS_CRLF,
              ret, pRegIO->ictl.retCode);
      return -1;
   }

   MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
           "Set MEI Reg[0x%X] = 0x%08X" MEIOS_CRLF, pRegIO->addr, pRegIO->value);

   return 0;
}


/**
   read an ACK from the mailbox.
*/
int MEI_ReadRawAck(MEIOS_File_t *streamOut, int fd)
{
   int ret;
   IOCTL_MEI_mboxSend_t ioctlArg;

   /* setup message */
   MEIOS_MemSet(&MEI_RdCmvMsg, 0x00, sizeof(CMV_MESSAGE_ALL_T));

   MEIOS_MemSet(&ioctlArg, 0x00, sizeof(IOCTL_MEI_mboxSend_t));

   MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
           "Start - read ACK (count=%d / 2)" MEIOS_CRLF, sizeof(CMV_MESSAGE_ALL_T));

   ioctlArg.ack_msg.count_16bit = sizeof(CMV_MESSAGE_ALL_T);
   ioctlArg.ack_msg.pData_16 = (unsigned short *)&MEI_RdCmvMsg;
   ret = MEIOS_DeviceControl(fd, FIO_MEI_MBOX_ACK_RAW_RD, (MEI_IOCTL_ARG)&ioctlArg.ack_msg);

   if (ret < 0)
      MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
              "ERROR - read ACK (ret = %d, exp = %d), retCode = %d" MEIOS_CRLF,
              ret, sizeof(CMV_MESSAGE_ALL_T), ioctlArg.ack_msg.ictl.retCode);
   else
   {
      MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
              "Success - read ACK (read = %d, exp = %d)" MEIOS_CRLF,
              ret, sizeof(CMV_MESSAGE_ALL_T));

      MEI_log_cmv_hdr(streamOut, "read", &MEI_RdCmvMsg);
   }

   return ret;
}


/**
   Send a raw CMV message.
*/
int MEI_SendRawMsg(MEIOS_File_t *streamOut, int fd, int payload_count_16bit, int start)
{

   int ret, i, msgCount_16bit;
   CMV_STD_MESSAGE_T *pCmvMsg = &MEI_WrCmvMsg.cmv;
   IOCTL_MEI_mboxSend_t ioctlArg;

   MEIOS_Printf( TEST_MEI_DBG_PREFIX
           "Start - send Msg (count=%d / 2)" MEIOS_CRLF, payload_count_16bit);

   /* setup write message */
   MEIOS_MemSet(pCmvMsg, 0x00, sizeof(CMV_STD_MESSAGE_T));

   MEIOS_MemSet(&ioctlArg, 0x00, sizeof(IOCTL_MEI_mboxSend_t));

   if (payload_count_16bit > CMV_USED_PAYLOAD_16BIT_SIZE)
      payload_count_16bit = CMV_USED_PAYLOAD_16BIT_SIZE;

   start = (start == -1) ? 0 : start;

   /* setup header */
   pCmvMsg->header.mbxCode = 0x00;
   P_CMV_MSGHDR_FCT_OPCODE_DIR_SET(pCmvMsg, 0x00);
   P_CMV_MSGHDR_FCT_OPCODE_TYPE_SET(pCmvMsg, 0x03);
   P_CMV_MSGHDR_BIT_SIZE_SET(pCmvMsg, CMV_MSG_BIT_SIZE_16BIT);
   P_CMV_MSGHDR_PAYLOAD_SIZE_SET(pCmvMsg, payload_count_16bit);

   for (i=0; i < payload_count_16bit; i++)
   {
      pCmvMsg->payload.params_16Bit[i] = (unsigned short)start++;
   }

   msgCount_16bit = payload_count_16bit + CMV_HEADER_16BIT_SIZE;
   ioctlArg.write_msg.pData_16 = (unsigned short *)pCmvMsg;
   ioctlArg.write_msg.count_16bit = msgCount_16bit;

   MEIOS_Printf( TEST_MEI_DBG_PREFIX
           "Start - write to mailbox (payload count 16bit %d)" MEIOS_CRLF,
           payload_count_16bit);

   MEI_log_cmv_hdr(streamOut, "write", (CMV_MESSAGE_ALL_T *)pCmvMsg);


   /* setup receive message buffer */
   MEIOS_MemSet(&MEI_RdCmvMsg, 0x00, sizeof(CMV_MESSAGE_ALL_T));

   MEIOS_Printf( TEST_MEI_DBG_PREFIX
           "Start - read ACK (count=%d / 2)" MEIOS_CRLF,
           sizeof(CMV_MESSAGE_ALL_T));

   ioctlArg.ack_msg.count_16bit = sizeof(CMV_MESSAGE_ALL_T);
   ioctlArg.ack_msg.pData_16 = (unsigned short *)&MEI_RdCmvMsg;

   ret = MEIOS_DeviceControl(fd, FIO_MEI_MBOX_MSG_RAW_SEND, (MEI_IOCTL_ARG)&ioctlArg);
   if (ret < 0)
   {
      MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
               "Error - send msg (read = %d, exp = %d), retCode = %d" MEIOS_CRLF,
               ret, msgCount_16bit, ioctlArg.ictl.retCode);
   }
   else
   {
      MEIOS_Printf( TEST_MEI_DBG_PREFIX
              "DONE - send Msg (read = %d, exp = %d)" MEIOS_CRLF,
              ret, sizeof(CMV_MESSAGE_ALL_T));

      MEI_log_cmv_hdr(streamOut, "read", &MEI_RdCmvMsg);
   }

   return ret;
}


/*
   Set the msg for send message
*/
void MEI_SetupSendMsg( IOCTL_MEI_message_t *pMsgCntrl,
                         unsigned char *pBuf, int bufSize,
                         unsigned short msgID)
{
   pMsgCntrl->msgId = msgID;
   pMsgCntrl->msgClassifier = 0x00;
   pMsgCntrl->paylSize_byte = bufSize;
   pMsgCntrl->pPayload = pBuf;

   return;
}


/*
   Set the msg arguments for send message
*/
void MEI_SetupSendMsgArg( IOCTL_MEI_message_t *pMsgCntrl,
                            unsigned char *pBuf,
                            unsigned short msgID, int *pParamArr)
{
   int i;

   unsigned int   *pUInt32Buf = (unsigned int *)pBuf;
   unsigned short *pUInt16Buf = (unsigned short *)pBuf;

   /* index + length */
   pUInt16Buf[0] = (unsigned short)pParamArr[0];
   pUInt16Buf[1] = (unsigned short)pParamArr[1];


   /* ifx msg (params without index and length) */
   for (i = 0; i < MEI_TEST_MAX_OPT_PARAMS - 2; i++)
   {
      if(pParamArr[i+2] == -1)
         break;

      if ( (msgID & 0x0010) )
      {
         pUInt32Buf[i+1] = (unsigned int)pParamArr[i+2];
      }
      else
         pUInt16Buf[i+2] = (unsigned short)pParamArr[i+2];
   }

   pMsgCntrl->msgId = msgID;
   pMsgCntrl->msgClassifier = 0;

   /* Add index + length field */
   pMsgCntrl->paylSize_byte = 2 * sizeof(unsigned short);

   /* Add payload data (16/32 bit) */
   if ( (msgID & 0x0010) )
      pMsgCntrl->paylSize_byte += (i * sizeof(unsigned int));
   else
      pMsgCntrl->paylSize_byte += (i * sizeof(unsigned short));

   pMsgCntrl->pPayload = pBuf;

   return;
}


/**
   send a message (write the msg and wait for ack)
   - message ID
*/
int MEI_SendMessage( MEIOS_File_t *streamOut, int fd, IOCTL_MEI_messageSend_t *pMsgSend,
                       unsigned short msgID, int *pParamArr)
{
   int ret;

   /* setup the send msg args */
   MEI_SetupSendMsgArg( &pMsgSend->write_msg, (unsigned char *)MEI_WrCmvMsg.rawMsg,
                          msgID, pParamArr);

   /* setup the ack buffer */
   pMsgSend->ack_msg.pPayload = (unsigned char *)MEI_RdCmvMsg.rawMsg;
   pMsgSend->ack_msg.paylSize_byte = sizeof(MEI_RdCmvMsg.rawMsg);

   MEI_log_ifx_msg(streamOut, "MSG send", &pMsgSend->write_msg);

   ret = MEIOS_DeviceControl(fd, FIO_MEI_MBOX_MSG_SEND, (MEI_IOCTL_ARG)pMsgSend);

   if (ret < 0)
   {
      MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
               "ERROR - send msg (ret = %d), retCode = %d" MEIOS_CRLF,
               ret, pMsgSend->ictl.retCode);

      MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
               "ACK: msgId = 0x%04X, MbCode = 0x%02X, FctOp = 0x%02X" MEIOS_CRLF,
               pMsgSend->ack_msg.msgId,
               pMsgSend->ack_msg.msgClassifier & 0x00FF,
               (pMsgSend->ack_msg.msgClassifier & 0xFF00) >> 8 );
   }
   else
   {
      MEI_log_ifx_msg(streamOut, "ACK recv", &pMsgSend->ack_msg);
   }

   return ret;
}


/**
   write a message (write the msg)
   - message ID
*/
int MEI_WriteMessage(MEIOS_File_t *streamOut, int fd, IOCTL_MEI_message_t *pMsgWrite,
                           unsigned short msgID, int *pParamArr)
{
   int ret;

   /* setup the send msg args */
   MEI_SetupSendMsgArg( pMsgWrite, (unsigned char *)MEI_WrCmvMsg.rawMsg,
                              msgID, pParamArr);

   MEI_log_ifx_msg(streamOut, "MSG write", pMsgWrite);

   ret = MEIOS_DeviceControl(fd, FIO_MEI_MBOX_MSG_WR, (MEI_IOCTL_ARG)pMsgWrite);
   if (ret < 0)
   {
      MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
               "ERROR - write msg (ret = %d), retCode = %d" MEIOS_CRLF,
               ret, pMsgWrite->ictl.retCode);
   }

   return ret;
}


/**
   read a ACK message
*/
int MEI_ReadAck(MEIOS_File_t *streamOut, int fd, IOCTL_MEI_message_t *pMsgAck)
{
   int ret;

   /* setup the ack buffer */
   pMsgAck->pPayload = (unsigned char *)MEI_RdCmvMsg.rawMsg;
   pMsgAck->paylSize_byte = sizeof(MEI_RdCmvMsg.rawMsg);

   ret = MEIOS_DeviceControl(fd, FIO_MEI_MBOX_ACK_RD, (MEI_IOCTL_ARG)pMsgAck);
   if (ret < 0)
   {
      MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
               "ERROR - Read Ack (ret = %d), retCode = %d" MEIOS_CRLF,
               ret, pMsgAck->ictl.retCode);
      MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
               "ACK: msgId = 0x%04X, FctOp = 0x%02X, MbCode = 0x%02X" MEIOS_CRLF,
               pMsgAck->msgId,
               pMsgAck->msgClassifier & 0x00FF,
               (pMsgAck->msgClassifier & 0xFF00) >> 8 );
   }
   else
   {
      MEI_log_ifx_msg(streamOut, "ACK read", pMsgAck);
   }

   return ret;
}


/**
   read a NFC message
*/
int MEI_ReadNfc(MEIOS_File_t *streamOut, int fd, IOCTL_MEI_message_t *pMsgNfc)
{
   int ret;

   /* setup the ack buffer */
   pMsgNfc->pPayload = (unsigned char *)MEI_RdCmvMsg.rawMsg;
   pMsgNfc->paylSize_byte = sizeof(MEI_RdCmvMsg.rawMsg);

   ret = MEIOS_DeviceControl(fd, FIO_MEI_MBOX_NFC_RD, (MEI_IOCTL_ARG)pMsgNfc);
   if (ret < 0)
   {
      MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
               "ERROR - Read NFC (ret = %d), retCode = %d" MEIOS_CRLF,
               ret, pMsgNfc->ictl.retCode);
      MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
               "NFC: msgId = 0x%04X, FctOp = 0x%02X, MbCode = 0x%02X" MEIOS_CRLF,
               pMsgNfc->msgId,
               pMsgNfc->msgClassifier & 0x00FF,
               (pMsgNfc->msgClassifier & 0xFF00) >> 8 );
   }
   else
   {
      MEI_log_ifx_msg(streamOut, "NFC read", pMsgNfc);
   }

   return ret;
}




/**
   Switch on/off the VRX NFC handling
   --> ioctl(..., FIO_MEI_MBOX_NFC_ENABLE, ...)
   --> ioctl(..., FIO_MEI_MBOX_NFC_DISABLE, ...)
*/
int MEI_ReceiveNfcOnOff(MEIOS_File_t *streamOut, int fd, int on_off)
{
   IOCTL_MEI_ioctl_t argIoctl;

   MEIOS_Printf( TEST_MEI_DBG_PREFIX
           "ioct(%s)" MEIOS_CRLF,
           (on_off) ? "FIO_MEI_MBOX_NFC_ENABLE":"FIO_MEI_MBOX_NFC_DISABLE");

   MEIOS_MemSet(&argIoctl, 0x00, sizeof(IOCTL_MEI_ioctl_t));

   if ( ( MEIOS_DeviceControl(fd,
                 (on_off)?(FIO_MEI_MBOX_NFC_ENABLE):(FIO_MEI_MBOX_NFC_DISABLE),
                 (MEI_IOCTL_ARG)&argIoctl) ) < 0 )
   {
      MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
               "ERROR - ioct(.., %s, ..), retCode = %d" MEIOS_CRLF,
               (on_off) ? "FIO_MEI_MBOX_NFC_ENABLE":"FIO_MEI_MBOX_NFC_DISABLE",
               argIoctl.retCode);
      return -1;
   }
   return 0;
}


/**
   Get/Set the autonomous message control (NFC)
   --> ioctl(..., FIO_MEI_AUTO_MSG_CTRL_GET, ...)
   --> ioctl(..., FIO_MEI_AUTO_MSG_CTRL_SET, ...)
*/
int MEI_AutoMsgCntl(MEIOS_File_t *streamOut, int fd, int set_get,
                      unsigned int modemCtrl, unsigned int driverCtrl)
{
   IOCTL_MEI_autoMsgCtrl_t autoMsgCtrl;

   MEIOS_MemSet(&autoMsgCtrl, 0x00, sizeof(IOCTL_MEI_autoMsgCtrl_t));

   if (set_get)
   {
      autoMsgCtrl.modemMsgMask = modemCtrl;
      autoMsgCtrl.driverMsgMask = driverCtrl;
      MEIOS_Printf( TEST_MEI_DBG_PREFIX
              "ioct(FIO_MEI_AUTO_MSG_CTRL_SET) - modemCtrl = 0x%X driverCtrl = 0x%X" MEIOS_CRLF,
              modemCtrl, driverCtrl);
   }
   else
   {
      MEIOS_Printf( TEST_MEI_DBG_PREFIX
              "ioct(FIO_MEI_AUTO_MSG_CTRL_GET)" MEIOS_CRLF);
   }

   if ( ( MEIOS_DeviceControl(fd,
                 (set_get)?(FIO_MEI_AUTO_MSG_CTRL_SET):(FIO_MEI_AUTO_MSG_CTRL_GET),
                 (MEI_IOCTL_ARG)&autoMsgCtrl) ) < 0 )
   {
      MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
               "ERROR - ioct(.., %s, ..), retCode = %d" MEIOS_CRLF,
               (set_get) ? "FIO_MEI_AUTO_MSG_CTRL_SET":"FIO_MEI_AUTO_MSG_CTRL_GET",
               autoMsgCtrl.ictl.retCode);
      return -1;
   }

   MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
            "ioct(.., %s, ..) done - modemCtrl = 0x%X, driverCtrl = 0x%X" MEIOS_CRLF,
            (set_get) ? "FIO_MEI_AUTO_MSG_CTRL_SET":"FIO_MEI_AUTO_MSG_CTRL_GET",
            autoMsgCtrl.modemMsgMask, autoMsgCtrl.driverMsgMask);

   return 0;
}




/**
   read an NFC from the mailbox.
*/
int MEI_ReadRawNfc(MEIOS_File_t *streamOut, int fd)
{
   int ret;
   CMV_STD_MESSAGE_T *pCmvMsg = &MEI_RdCmvMsg.cmv;
   IOCTL_MEI_mboxMsg_t ioctlArg;

   /* enable the NFC message handling */
   MEI_ReceiveNfcOnOff(streamOut, fd, 1);

   /* setup message */
   MEIOS_MemSet(pCmvMsg, 0x00, sizeof(CMV_STD_MESSAGE_T));

   MEIOS_MemSet(&ioctlArg, 0x00, sizeof(IOCTL_MEI_mboxMsg_t));

   MEIOS_Printf( TEST_MEI_DBG_PREFIX
           "Start - read RAW NFC (count=%d / 2)" MEIOS_CRLF,
           sizeof(CMV_STD_MESSAGE_T));

   ioctlArg.count_16bit = sizeof(CMV_STD_MESSAGE_T);
   ioctlArg.pData_16 = (unsigned short *)pCmvMsg;
   ret = MEIOS_DeviceControl(fd, FIO_MEI_MBOX_NFC_RAW_RD, (MEI_IOCTL_ARG)&ioctlArg);

   if (ret < 0)
      MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
               "ERROR - read RAW NFC (ret = %d, exp = %d), retCode = %d" MEIOS_CRLF,
               ret, sizeof(CMV_STD_MESSAGE_T), ioctlArg.ictl.retCode);
   else
   {
      MEIOS_Printf( TEST_MEI_DBG_PREFIX
              "Success - read RAW NFC (read = %d, exp = %d)" MEIOS_CRLF,
              ret, sizeof(CMV_STD_MESSAGE_T));

      MEI_log_cmv_hdr(streamOut, "read RAW", (CMV_MESSAGE_ALL_T *)pCmvMsg);
   }

   return ret;
}


/**
   Show MEI register.
   --> ioctl(..., FIO_MEI_MEI_REGS_SHOW, ...)
*/
void MEI_show_mei_regs(int fd)
{
   IOCTL_MEI_ioctl_t argIoctl;

   MEIOS_Printf( TEST_MEI_DBG_PREFIX
           "ioct(FIO_MEI_MEI_REGS_SHOW)" MEIOS_CRLF);

   MEIOS_MemSet(&argIoctl, 0x00, sizeof(IOCTL_MEI_ioctl_t));

   if ( (MEIOS_DeviceControl(fd, FIO_MEI_MEI_REGS_SHOW, (MEI_IOCTL_ARG)&argIoctl)) < 0 )
   {
      MEIOS_Printf( TEST_MEI_DBG_PREFIX
              "ERROR - ioct(.., FIO_MEI_MEI_REGS_SHOW, ..), retCode = %d" MEIOS_CRLF,
              argIoctl.retCode);
      return;
   }

   return;

}

/**
   Show MEI register.
   --> ioctl(..., FIO_MEI_DRV_BUF_SHOW, ...)
*/
void MEI_show_drv_buffer(int fd, unsigned char bufNum, unsigned int count)
{
   IOCTL_MEI_drvBufShow_t MEI_ShowDrvBuf;

   MEIOS_Printf( TEST_MEI_DBG_PREFIX "ioct(FIO_MEI_DRV_BUF_SHOW)" MEIOS_CRLF);

   MEIOS_MemSet(&MEI_ShowDrvBuf, 0x00, sizeof(IOCTL_MEI_drvBufShow_t));

   if (count)
   {
      MEI_ShowDrvBuf.count = (count > CMV_MESSAGE_SIZE) ? CMV_MESSAGE_SIZE : count;
   }
   else
   {
      MEI_ShowDrvBuf.count = 16;
   }

   MEI_ShowDrvBuf.bufNum = (unsigned int)(bufNum & 0xFF);

   if ( (MEIOS_DeviceControl(fd, FIO_MEI_DRV_BUF_SHOW, (MEI_IOCTL_ARG)&MEI_ShowDrvBuf)) < 0 )
   {
      MEIOS_Printf( TEST_MEI_DBG_PREFIX
              "ERROR - ioct(.., FIO_MEI_DRV_BUF_SHOW, ..), retCode = %d" MEIOS_CRLF,
              MEI_ShowDrvBuf.ictl.retCode);
      return;
   }

   return;
}


/**
   Write to the VRX via MEI debug functionality.
*/
int MEI_mei_dbg_write(MEIOS_File_t *streamOut, int fd, int offset, int des, int count, int *pParamArr)
{
   int idx;

   count = (count < 0) ? 0x100: count;

   MEIOS_Printf( TEST_MEI_DBG_PREFIX
           "ioct(.., FIO_MEI_DBG_WRITE, ..): off=0x%08X, des=%d, count=%d" MEIOS_CRLF,
           offset, des, count);

   MEI_DbgAccess.count = count;
   MEI_DbgAccess.dbgAddr = offset;
   MEI_DbgAccess.dbgDest = des;

   if (count == 1)
   {
      if (pParamArr[0] != -1)
         MEI_DumpBufWr[0] = pParamArr[0];
      else
         MEI_DumpBufWr[0] = 0xDEADBEEF;

      MEI_DbgAccess.pData_32 = (unsigned int *)MEI_DumpBufWr;
      MEIOS_Printf( TEST_MEI_DBG_PREFIX
              "DBG Dest[%d] Write[0x%08X] = 0x%08X" MEIOS_CRLF,
              des, offset, (unsigned int)MEI_DbgAccess.pData_32 );
   }
   else
   {
      for (idx=0; idx < count; idx++)
      {
         MEI_DumpBufWr[idx] = idx + 0xa500;
         MEIOS_Printf( TEST_MEI_DBG_PREFIX
                 "DBG Dest[%d] Write[0x%08X] = 0x%08X" MEIOS_CRLF,
                 des, offset+(idx << 2), idx + 0xa500 );
      }
      MEI_DbgAccess.pData_32 = (unsigned int *)MEI_DumpBufWr;
   }

   if ( (MEIOS_DeviceControl(fd, FIO_MEI_DBG_WRITE, (MEI_IOCTL_ARG)&MEI_DbgAccess)) < 0 )
   {
      MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
              "ERROR - ioct(.., FIO_MEI_DBG_WRITE, ..) - wrCount %d, retCode = %d" MEIOS_CRLF,
               (unsigned int)MEI_DbgAccess.count, MEI_DbgAccess.ictl.retCode);
      return -1;
   }

   MEIOS_Printf( TEST_MEI_DBG_PREFIX
           "DONE - ioct(.., FIO_MEI_DBG_WRITE, ..)" MEIOS_CRLF);

   return 0;
}


/**
   Read from the VRX via MEI debug functionality.
*/
int MEI_mei_dbg_read(MEIOS_File_t *streamOut, int fd, int offset, int des, int count)
{
   int idx;

   count = (count < 0) ? 0x100: count;
   MEI_DbgAccess.count = count;
   MEI_DbgAccess.dbgAddr = offset;
   MEI_DbgAccess.dbgDest = des;
   MEI_DbgAccess.pData_32 = (unsigned int *)MEI_DumpBufRd;

   MEIOS_Printf( TEST_MEI_DBG_PREFIX
           "ioct(.., FIO_MEI_DBG_READ, ..): off=0x%08X, des=%d, count=%d" MEIOS_CRLF,
           (unsigned int)MEI_DbgAccess.dbgAddr,
           (unsigned int)MEI_DbgAccess.dbgDest,
           (unsigned int)MEI_DbgAccess.count);


   if ( (MEIOS_DeviceControl(fd, FIO_MEI_DBG_READ, (MEI_IOCTL_ARG)&MEI_DbgAccess)) < 0 )
   {
      MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
              "ERROR - ioct(.., FIO_MEI_DBG_READ, ..) - rdCount %d, retCode = %d" MEIOS_CRLF,
               (unsigned int)MEI_DbgAccess.count, MEI_DbgAccess.ictl.retCode);
      return -1;
   }

   MEIOS_Printf( TEST_MEI_DBG_PREFIX
           "DONE - ioct(.., FIO_MEI_DBG_READ, ..)" MEIOS_CRLF);


   for (idx=0; idx < count; idx++)
   {
      MEIOS_FPrintf(streamOut,
               "--> DBG Dest[%d] Read[0x%08X] = 0x%08X" MEIOS_CRLF,
               des, offset+(idx << 2), MEI_DumpBufRd[idx] );
   }

   return 0;
}

/**
   Write to the VRX via General Purpose Access (GPA) functionality
*/
int MEI_gpa_write(MEIOS_File_t *streamOut, int fd, int addr, int dest, int value)
{
   /* check params */
   MEI_GpaAccessIo.addr = (unsigned int)addr;
   MEI_GpaAccessIo.value = (unsigned int)value;

   if (dest != -1)
      MEI_GpaAccessIo.dest = (dest) ? 1 : 0;
   else
   {
      /* invalid destination */
      MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
               "ioct(.., FIO_MEI_GPA_WRITE, ..): invalid destination" MEIOS_CRLF);
      return -1;
   }

   if ( (MEIOS_DeviceControl(fd, FIO_MEI_GPA_WRITE, (MEI_IOCTL_ARG)&MEI_GpaAccessIo)) < 0 )
   {
      MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
               "ERROR - ioct(.., FIO_MEI_GPA_WRITE, ..), retCode = %d" MEIOS_CRLF,
               MEI_GpaAccessIo.ictl.retCode);
      return -1;
   }

   MEIOS_Printf( TEST_MEI_DBG_PREFIX
           "DONE - ioct(.., FIO_MEI_GPA_WRITE, ..): addr/off[0x%X] = 0x%08X (%s)" MEIOS_CRLF,
           MEI_GpaAccessIo.addr,
           MEI_GpaAccessIo.value,
           (MEI_GpaAccessIo.dest)?"AUX" : "MEM" );

   return 0;
}


/**
   Read from the VRX via General Purpose Access (GPA) functionality
*/
int MEI_gpa_read(MEIOS_File_t *streamOut, int fd, int addr, int dest)
{
   /* check params */
   MEI_GpaAccessIo.addr = (unsigned int)addr;

   if (dest != -1)
      MEI_GpaAccessIo.dest = (dest) ? 1 : 0;
   else
   {
      /* invalid destination */
      MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
               "ioct(.., FIO_MEI_GPA_READ, ..): invalid destination" MEIOS_CRLF);
      return -1;
   }

   if ( (MEIOS_DeviceControl(fd, FIO_MEI_GPA_READ, (MEI_IOCTL_ARG)&MEI_GpaAccessIo)) < 0 )
   {
      MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
               "ERROR - ioct(.., FIO_MEI_GPA_READ, ..), retCode = %d" MEIOS_CRLF,
               MEI_GpaAccessIo.ictl.retCode);
      return -1;
   }

   MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
           "GPA Dest(%s) Read[0x%08X] = 0x%08X" MEIOS_CRLF,
            (MEI_GpaAccessIo.dest)?"AUX" : "MEM",
            MEI_GpaAccessIo.addr,
            MEI_GpaAccessIo.value);

   return 0;
}

#if 0
/*
   write  a file.
*/
int MEI_write_file(MEIOS_File_t *streamOut, char *pFileName, unsigned char *pFileBuf, int fileSize)
{

   int filefd, ret;

   /* Open file for input: */
   if( (filefd = open( pFileName, O_CREAT | O_RDWR /* | O_BINARY */, 0644)) == -1 )
   {
      MEIOS_FPrintf(streamOut,
         "ERROR: %s - create file <%s> failed" MEIOS_CRLF,
         MEIOS_StrError(errno), pFileName);
      return -1;
   }

   if ( (ret = write(filefd, pFileBuf, fileSize)) != fileSize)
   {
      MEIOS_FPrintf(streamOut,
         "error write file: exp = %d != write = %d - %s" MEIOS_CRLF,
         fileSize, ret, MEIOS_StrError(errno));
      close(filefd);
      return -1;
   }

   close(filefd);
   return ret;
}

#endif

/*
   do an firmware download.
*/
int MEI_fw_download(MEIOS_File_t *streamOut, int fd, int fileNum, char *pFileName)
{
   int ret;
   char bufFileName[80+1];

   MEIOS_MemSet(bufFileName, 0x00, 80+1);

   if (pFileName && MEIOS_StrLen(pFileName))
   {
      MEIOS_StrNCpy(bufFileName, pFileName, 80);
   }
   else
   {
      if (fileNum == -1)
      {
         MEIOS_SPrintf(bufFileName,"%s.bin", MEI_FW_DL_FILE_BASE_NAME);
      }
      else
      {
         MEIOS_SPrintf(bufFileName,"%s%d.bin", MEI_FW_DL_FILE_BASE_NAME, fileNum);
      }
   }

   ret = MEI_fw_download_name(streamOut, fd, bufFileName);

   return ret;
}


/*
   swap the FW from VDSL2 <--> ADSL
*/
int MEI_fw_swap(MEIOS_File_t *streamOut, int fd, int bTestFwSwap)
{
   IOCTL_MEI_fwMode_t fwMode;

   MEIOS_MemSet(&fwMode, 0x00, sizeof(IOCTL_MEI_fwMode_t));

   fwMode.fwMode = (bTestFwSwap) ? 1 : 0;

   if ( (MEIOS_DeviceControl(fd, FIO_MEI_FW_MODE_SELECT, (MEI_IOCTL_ARG)&fwMode)) < 0 )
   {
      MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
               "ERROR - ioct(FIO_MEI_FW_MODE_SELECT), %s, retCode = %d" MEIOS_CRLF,
               MEIOS_StrError(errno), fwMode.ictl.retCode);

      return -1;
   }

   return 0;
}

/*
   Set Current XDSL mode in port mode control structure
*/
int MEI_fw_set_mode(MEIOS_File_t *streamOut, int fd, char *pConfig)
{
   IOCTL_MEI_FwModeCtrlSet_t fwMode;

   memset(&fwMode, 0x00, sizeof(IOCTL_MEI_FwModeCtrlSet_t));

   fwMode.bMultiLineModeLock = IFX_TRUE;
   fwMode.bXdslModeLock      = IFX_TRUE;

   sscanf(pConfig, "%u %hhu %u",
          (unsigned int *)&fwMode.eXdslModeCurrent,
          &fwMode.firmwareFeatures.nPlatformId,
          (unsigned int *)&fwMode.firmwareFeatures.eFirmwareXdslModes);

   if (strlen(pConfig) == 0)
   {
      MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
               "ERROR - empty parameters, use -z option!" MEIOS_CRLF);

      return -1;
   }

   switch (fwMode.eXdslModeCurrent)
   {
      case e_MEI_XDSLMODE_VDSL:
         MEIOS_Printf(TEST_MEI_DBG_PREFIX"XDSL mode: set VDSL mode" MEIOS_CRLF);
         break;

      case e_MEI_XDSLMODE_ADSL:
         MEIOS_Printf(TEST_MEI_DBG_PREFIX"XDSL mode: set ADSL mode" MEIOS_CRLF);
         break;

      default:
         MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
               "ERROR - inconsistent parameters, invalid xdsl mode %u!" MEIOS_CRLF,
                                        (unsigned int)fwMode.eXdslModeCurrent);
         return -1;
   }

   switch (fwMode.firmwareFeatures.nPlatformId)
   {
      case 0x5:
         MEIOS_Printf(TEST_MEI_DBG_PREFIX"FW platform id: set VRX200 platform" MEIOS_CRLF);
         break;

      case 0x7:
         MEIOS_Printf(TEST_MEI_DBG_PREFIX"FW platform id: set VRX300 platform" MEIOS_CRLF);
         break;

      case 0x8:
         MEIOS_Printf(TEST_MEI_DBG_PREFIX"FW platform id: set VRX500 platform" MEIOS_CRLF);
         break;

      default:
         MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
               "ERROR - inconsistent parameters, invalid platform id %u!" MEIOS_CRLF,
                                (unsigned int)fwMode.firmwareFeatures.nPlatformId);
         return -1;
   }

   if (fwMode.firmwareFeatures.eFirmwareXdslModes == e_MEI_FW_XDSLMODE_CLEANED)
   {
      MEIOS_Printf(TEST_MEI_DBG_PREFIX"FW XDSL mode: set CLEANED mode" MEIOS_CRLF);
   }

   if (fwMode.firmwareFeatures.eFirmwareXdslModes & e_MEI_FW_XDSLMODE_ADSL_A)
   {
      MEIOS_Printf(TEST_MEI_DBG_PREFIX"FW XDSL mode: set ADSL_A mode" MEIOS_CRLF);
   }

   if (fwMode.firmwareFeatures.eFirmwareXdslModes & e_MEI_FW_XDSLMODE_ADSL_B)
   {
      MEIOS_Printf(TEST_MEI_DBG_PREFIX"FW XDSL mode: set ADSL_B mode" MEIOS_CRLF);
   }

   if (fwMode.firmwareFeatures.eFirmwareXdslModes & e_MEI_FW_XDSLMODE_VDSL2)
   {
      MEIOS_Printf(TEST_MEI_DBG_PREFIX"FW XDSL mode: set VDSL2 mode" MEIOS_CRLF);
   }

   if (fwMode.firmwareFeatures.eFirmwareXdslModes & e_MEI_FW_XDSLMODE_VDSL2_VECTOR)
   {
      MEIOS_Printf(TEST_MEI_DBG_PREFIX"FW XDSL mode: set VDSL2_VECTOR mode" MEIOS_CRLF);
   }

   MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
      "xdsl = 0x%X, id = 0x%X, fw xdsl = 0x%X" MEIOS_CRLF,
          fwMode.eXdslModeCurrent,
          fwMode.firmwareFeatures.nPlatformId,
          fwMode.firmwareFeatures.eFirmwareXdslModes);

   if ( (MEIOS_DeviceControl(fd, FIO_MEI_FW_MODE_CTRL_SET, (MEI_IOCTL_ARG)&fwMode)) < 0 )
   {
      MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
               "ERROR - ioct(FIO_MEI_FW_MODE_CTRL_SET), %s, retCode = %d" MEIOS_CRLF,
               MEIOS_StrError(errno), fwMode.ictl.retCode);

      return -1;
   }

   return 0;
}

/*
   do an firmware download.
*/
int MEI_fw_download_name(MEIOS_File_t *streamOut, int fd, char *pFileName)
{
   IFX_size_t filesize = 0;

   if ((MEIOS_StrCaseCmp(pFileName, "null", MEIOS_StrLen("null")) == 0))
   {
         MEIOS_Printf( TEST_MEI_DBG_PREFIX
                 "No fw binary (expected chunks reuse)" MEIOS_CRLF);
         MEI_FwDl.pFwImage = NULL;
         MEI_FwDl.size_byte = 0;
   }
   else
   {
      if (MEIOS_FileLoad (pFileName, &pVrxFileBuffer, &filesize) != 0)
      {
         MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
                  "ERROR: FW Download - load FW image file <%s> failed", pFileName);
         return -1;
      }

      /* Open file for input: */
      if(filesize <= 0)
      {
         MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
                  "ERROR: FW Download - load FW image file <%s> failed (size)", pFileName);
         return -1;
      }

      MEIOS_Printf( TEST_MEI_DBG_PREFIX
              "Start Download <%s>, size: %lu" MEIOS_CRLF, pFileName, filesize);

      MEI_FwDl.pFwImage = pVrxFileBuffer;
      MEI_FwDl.size_byte = (IFX_uint32_t)filesize;
   }

   if ( (MEIOS_DeviceControl(fd, FIO_MEI_FW_DL, (MEI_IOCTL_ARG)&MEI_FwDl)) < 0 )
   {
      MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
               "ERROR - ioct(FIO_MEI_FW_DL), %s, retCode = %d" MEIOS_CRLF,
               MEIOS_StrError(errno), MEI_FwDl.ictl.retCode);

      return -1;
   }

   return 0;
}


/**
   Read from the VRX via DMA functionality
*/
int MEI_dma_read(MEIOS_File_t *streamOut, int fd, unsigned int addr, unsigned int count)
{
   int i;

   count = (count > 0xFF) ? 0xFF : count;

   MEI_IoctArgs.dma_access.dmaAddr = addr;
   MEI_IoctArgs.dma_access.count_32bit = count;
   MEI_IoctArgs.dma_access.pData_32 = MEI_DmaRdBuf;

   MEIOS_Printf( TEST_MEI_DBG_PREFIX
           "ioct(.., FIO_MEI_DMA_READ, ..): addr=0x%08X, count=%d" MEIOS_CRLF,
           addr, count);

   if ( (MEIOS_DeviceControl(fd, FIO_MEI_DMA_READ, (MEI_IOCTL_ARG)&MEI_IoctArgs)) < 0 )
   {
      MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
              "ERROR - ioct(.., FIO_MEI_DMA_READ, ..), retCode = %d" MEIOS_CRLF,
               MEI_IoctArgs.drv_ioctl.retCode);
      return -1;
   }

   MEIOS_Printf( TEST_MEI_DBG_PREFIX "DMA Read: " MEIOS_CRLF);
   for (i = 0; i < MEI_IoctArgs.dma_access.count_32bit; i++)
   {
      MEIOS_FPrintf(streamOut,
               "--> DMA Read[0x%08X]: 0x%08X" MEIOS_CRLF,
               (addr + i * 4), (unsigned int)MEI_DmaRdBuf[i]);
   }

   return 0;
}

/**
   Write to the VRX via DMA functionality
*/
int MEI_dma_write(MEIOS_File_t *streamOut, int fd, unsigned int addr, unsigned int count, unsigned int start)
{
   int i;

   count = (count > 0xFF) ? 0xFF : count;

   MEI_IoctArgs.dma_access.dmaAddr = addr;
   MEI_IoctArgs.dma_access.count_32bit = count;
   MEI_IoctArgs.dma_access.pData_32 = &MEI_DmaWrBuf[0];

   MEIOS_Printf( TEST_MEI_DBG_PREFIX
           "ioct(.., FIO_MEI_DMA_WRITE, ..): addr=0x%08X, count=%d, start=0x%X, buf 0x%08X" MEIOS_CRLF,
           addr, count, start, (unsigned int)&MEI_DmaWrBuf[0]);


   for (i = 0; i < count; i++)
   {
      MEI_DmaWrBuf[i] = start + i;
      MEIOS_Printf( TEST_MEI_DBG_PREFIX
              "--> DMA Write[0x%08X]: 0x%08X" MEIOS_CRLF,
              (addr + i * 4), (unsigned int)MEI_DmaWrBuf[i]);
   }

   if ( (MEIOS_DeviceControl(fd, FIO_MEI_DMA_WRITE, (MEI_IOCTL_ARG)&MEI_IoctArgs)) < 0 )
   {
      MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
               "ERROR - ioct(.., FIO_MEI_DMA_WRITE, ..), retCode = %d" MEIOS_CRLF,
               MEI_IoctArgs.dma_access.ictl.retCode);
      return -1;
   }

   return 0;
}


int MEI_dma_wr_rd(MEIOS_File_t *streamOut, int fd, unsigned int addr, unsigned int count, unsigned int start)
{
   int i, ret = 0;

   MEIOS_Printf( TEST_MEI_DBG_PREFIX
           "Start DMA Test - WRITE/READ: addr = 0x%08X, Count = %d, Start = 0x%04X" MEIOS_CRLF,
           addr, count, start);

   if ( (ret = MEI_dma_write(streamOut, fd, addr, count, start)) < 0)
   {
      MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
              "DMA Test - WRITE/READ: write failed" MEIOS_CRLF);
      return -1;
   }

   if ( (ret = MEI_dma_read(streamOut, fd, addr, count)) < 0)
   {
      MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
              "DMA Test - WRITE/READ: read failed" MEIOS_CRLF);
      return -1;
   }

   for (i = 0; i < count; i++)
   {
      if ( MEI_DmaWrBuf[i] != MEI_DmaRdBuf[i] )
      {
         /* ERROR DMA mismatch */
         MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
                  "END DMA Test - WRITE/READ ERROR at offset %d: read = 0x%04X expect = 0x%04X" MEIOS_CRLF,
                  i << 2, (unsigned int)MEI_DmaRdBuf[i], (unsigned int)MEI_DmaWrBuf[i]);
         return -1;
      }
   }

   MEIOS_Printf( TEST_MEI_DBG_PREFIX
           "End DMA Test - WRITE/READ: SUCCESS" MEIOS_CRLF);

   return 0;
}



int MEI_dma_stress(MEIOS_File_t *streamOut, int fd, unsigned int addr, unsigned int range, unsigned int loop)
{
   if ( ((int)addr == -1) ||
        ((int)range == -1) ||
        ((int)loop == -1) )
   {
      MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
              "DMA Stress Test: invalid param - \"mei_test -t 0x%X -x 0x%X -D 0x%X\"" MEIOS_CRLF,
               addr, range, loop);
      return -1;
   }

   MEI_IoctArgs.dbg_access.dbgAddr = addr;
   MEI_IoctArgs.dbg_access.count = range;
   MEI_IoctArgs.dbg_access.dbgDest = loop;

   MEIOS_Printf( TEST_MEI_DBG_PREFIX
           "ioct(.., FIO_MEI_DMA_TEST, ..): addr=0x%08X, range=%d, loop=0x04%X" MEIOS_CRLF,
           addr, range, loop);

   if ( (MEIOS_DeviceControl(fd, FIO_MEI_DMA_TEST, (MEI_IOCTL_ARG)&MEI_IoctArgs)) < 0 )
   {
      MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
              "ERROR - ioct(.., FIO_MEI_DMA_TEST, ..), retCode = %d" MEIOS_CRLF,
              MEI_IoctArgs.dbg_access.ictl.retCode);
      return -1;
   }

   MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
            "End DMA Test - Stress: SUCCESS" MEIOS_CRLF);

   return 0;
}


#ifdef LINUX

/*
   open an device and wait for incoming nfc's
*/
int MEI_nfc_wait_for_nfcs(MEIOS_File_t *streamOut, int max_wait_count, int dfeNum, int *pParams)
{
   int fd1, fdMax, ret, retNfc, tempCount = 0;
   char check = 'c';

   MEIOS_devFd_set_t   readfds_in, readfds_out;
   IOCTL_MEI_message_t ioctlArg_nfc_msg;

   fd1 = MEI_open_dev( stdout,
                         (dfeNum == -1) ?  TEST_MEI_USED_DFE_NUM : dfeNum,
                         (char *)TEST_MEI_DEV_PREFIX,
                         (char *)TEST_MEI_DEVICE_NAME);
   if( fd1 < 0 )
   {
      MEIOS_Printf(TEST_MEI_DBG_PREFIX "Error open device NFC[1]" MEIOS_CRLF);
      return -1;
   }

   fdMax = fd1 +1;


   if ( (pParams[0] == -1) && (pParams[1] == -1) )
   {
      /* enable nfc */
      ret = MEI_ReceiveNfcOnOff(stdout, fd1, 1);
      pParams[0] = MEI_DRV_MSG_CTRL_IF_MODEM_ALL_ON;    /* use default setup for NFC handling */
      pParams[1] = 0;   /* no driver message handling */
   }
   else
   {
      /* use auto msg ctrl for NFC setup */
      ret = MEI_AutoMsgCntl(stdout, fd1, 1,
                      ((pParams[0] == -1) ? 0 : (unsigned int)pParams[0] ),
                      ((pParams[1] == -1) ? 0 : (unsigned int)pParams[1] ));
   }

   if (ret != 0)
   {
      MEIOS_DeviceClose(fd1);
      return -1;
   }

   MEIOS_Printf(MEIOS_CRLF MEIOS_CRLF "Enter: " MEIOS_CRLF
      "\t<q> for end" MEIOS_CRLF
      "\t<digit> for poll" MEIOS_CRLF
      "\t<c> for continious processing" MEIOS_CRLF MEIOS_CRLF);


   /* wait for messages - select*/
   while (max_wait_count > 0)
   {
      /* set receive buffer */
      MEIOS_MemSet(&ioctlArg_nfc_msg, 0x00, sizeof(ioctlArg_nfc_msg));
      if ( pParams[0] & (MEI_DRV_MSG_CTRL_IF_MODEM_ETH_FRAME_ON) )
      {
         unsigned char MEI_EtHRecvBuf[1580];
         MEIOS_MemSet(&MEI_EtHRecvBuf, 0x00, 1580);
         ioctlArg_nfc_msg.pPayload = (unsigned char *)MEI_EtHRecvBuf;
         ioctlArg_nfc_msg.paylSize_byte = 1580;
      }
      else
      {
         MEIOS_MemSet(&MEI_RdCmvMsg, 0x00, sizeof(MEI_RdCmvMsg));
         ioctlArg_nfc_msg.pPayload = (unsigned char *)&MEI_RdCmvMsg.rawMsg;
         ioctlArg_nfc_msg.paylSize_byte = sizeof(MEI_RdCmvMsg);
      }

      if (isdigit(check))
      {
         tempCount = (int)check - (int)'0';
         check = 'p';
         MEIOS_Printf("---> poll for %d NFC's (mode %c)" MEIOS_CRLF, tempCount, check);
      }

      MEIOS_MemSet(&readfds_in,  0, sizeof(MEIOS_devFd_set_t));
      MEIOS_MemSet(&readfds_out, 0, sizeof(MEIOS_devFd_set_t));

      MEIOS_DevFdSet(0, &readfds_in);

      switch (check)
      {
         case 'c':
            /* add device fd for continous receive of NFC's */
            MEIOS_DevFdSet(fd1, &readfds_in);
            break;
         case 'p':
            /* add device only for a number of NFC's */
            if (tempCount > 0)
            {
               MEIOS_DevFdSet(fd1, &readfds_in);
            }
            break;
         default:
            check = 'c';
            MEIOS_DevFdSet(fd1, &readfds_in);
      }

      ret = MEIOS_DeviceSelect(fdMax, &readfds_in, &readfds_out, 500);
      if ( ret < 0 )
      {
         MEIOS_Printf(TEST_MEI_DBG_PREFIX "ERROR select - %s" MEIOS_CRLF, MEIOS_StrError(errno));
         break;
      }

      /* check data */
      if (ret == 0)
      {
         /* no data - timeout */
         continue;
      }

      /* something received */
      if ( MEIOS_DevFdIsSet(fd1, &readfds_out) )
      {
         if (tempCount > 0)
         {
            MEIOS_Printf( TEST_MEI_DBG_PREFIX "select - read NFC msg[%d] (mode %c)" MEIOS_CRLF,
                    tempCount, check);
            tempCount--;
         }
         else
         {
            MEIOS_Printf( TEST_MEI_DBG_PREFIX "select - read NFC msg (mode %c)" MEIOS_CRLF, check);
         }

         retNfc = MEIOS_DeviceControl(fd1, FIO_MEI_MBOX_NFC_RD, (MEI_IOCTL_ARG)&ioctlArg_nfc_msg);
         if (retNfc < 0)
         {
            MEIOS_Printf( TEST_MEI_DBG_PREFIX"ERROR ioctl(.., nfc, ..) retCode = %d - %s" MEIOS_CRLF,
                    ioctlArg_nfc_msg.ictl.retCode, MEIOS_StrError(errno));
            MEIOS_Printf( TEST_MEI_DBG_PREFIX"NFC: msgId = 0x%04X, FctOp = 0x%02X, MbCode = 0x%02X" MEIOS_CRLF,
                    ioctlArg_nfc_msg.msgId,
                    ioctlArg_nfc_msg.msgClassifier & 0x00FF,
                    (ioctlArg_nfc_msg.msgClassifier & 0xFF00) >> 8 );
            /*
               Go On also if you receive errors
            break;
            */
         }
         else
         {
            if ( pParams[0] & (MEI_DRV_MSG_CTRL_IF_MODEM_ETH_FRAME_ON) )
            {
               MEI_log_eth_frame(stdout, "Extract", ioctlArg_nfc_msg.pPayload, ioctlArg_nfc_msg.paylSize_byte  );
            }
            else
            {
               MEI_log_ifx_msg(stdout, "IFX NFC:", &ioctlArg_nfc_msg);
            }
         }
      }

      if ( MEIOS_DevFdIsSet(0, &readfds_out) )
      {
         check = (char)MEIOS_GetChar();
         if ( check == 'q')
            break;
      }
   }

   /* disable nfc */
   if ( (ret = MEI_ReceiveNfcOnOff(stdout, fd1, 0)) != 0)
   {
      MEIOS_Printf(TEST_MEI_DBG_PREFIX "ERROR disable NFC" MEIOS_CRLF);
   }

   /* close devices */
   MEIOS_DeviceClose(fd1);
   MEIOS_Printf( TEST_MEI_DBG_PREFIX
           "close(%d) device NFC[1]: /%s/%s/%d." MEIOS_CRLF, fd1,
           TEST_MEI_DEV_PREFIX, TEST_MEI_DEVICE_NAME,
           (dfeNum == -1) ? TEST_MEI_USED_DFE_NUM : dfeNum);

   return 0;
}

/*
   DSM config get
*/
int MEI_dsm_config_get(MEIOS_File_t *streamOut, int fd)
{
   IOCTL_MEI_dsmConfig_t dsm_config;

   memset(&dsm_config, 0x0, sizeof(IOCTL_MEI_dsmConfig_t));

   if ( (MEIOS_DeviceControl(fd, FIO_MEI_DSM_CONFIG_GET, (MEI_IOCTL_ARG)&dsm_config)) < 0 )
   {
      MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
               "ERROR - ioct(FIO_MEI_DSM_CONFIG_GET), %s, retCode = %d" MEIOS_CRLF,
               MEIOS_StrError(errno), dsm_config.ictl.retCode);

      return -1;
   }

   MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
      "VectorControl = %i" MEIOS_CRLF, dsm_config.eVectorControl);

   return 0;
}

/*
   DSM config set
*/
int MEI_dsm_config_set(MEIOS_File_t *streamOut, int fd, int vector_control)
{
   IOCTL_MEI_dsmConfig_t dsm_config;

   memset(&dsm_config, 0x0, sizeof(IOCTL_MEI_dsmConfig_t));

   dsm_config.eVectorControl = vector_control;
   switch (dsm_config.eVectorControl)
   {
      case e_MEI_VECTOR_CTRL_OFF:
         MEIOS_Printf(TEST_MEI_DBG_PREFIX
                         "DSM config: set VECTOR_CTRL_OFF" MEIOS_CRLF);
         break;

      case e_MEI_VECTOR_CTRL_ON:
         MEIOS_Printf(TEST_MEI_DBG_PREFIX
                         "DSM config: set VECTOR_CTRL_ON" MEIOS_CRLF);
         break;

      case e_MEI_VECTOR_CTRL_FRIENDLY_ON:
         MEIOS_Printf(TEST_MEI_DBG_PREFIX
                         "DSM config: set VECTOR_CTRL_FRIENDLY_ON" MEIOS_CRLF);
         break;

      default:
         MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
               "ERROR - inconsistent parameter %i" MEIOS_CRLF, vector_control);
         return -1;
   }

   if ( (MEIOS_DeviceControl(fd, FIO_MEI_DSM_CONFIG_SET, (MEI_IOCTL_ARG)&dsm_config)) < 0 )
   {
      MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
               "ERROR - ioct(FIO_MEI_DSM_CONFIG_SET), %s, retCode = %d" MEIOS_CRLF,
               MEIOS_StrError(errno), dsm_config.ictl.retCode);

      return -1;
   }

   return 0;
}

/*
   DSM status get
*/
int MEI_dsm_status_get(MEIOS_File_t *streamOut, int fd)
{
   IOCTL_MEI_dsmStatus_t dsm_status;

   memset(&dsm_status, 0x0, sizeof(IOCTL_MEI_dsmStatus_t));

   if ( (MEIOS_DeviceControl(fd, FIO_MEI_DSM_STATUS_GET, (MEI_IOCTL_ARG)&dsm_status)) < 0 )
   {
      MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
               "ERROR - ioct(FIO_MEI_DSM_STATUS_GET), %s, retCode = %d" MEIOS_CRLF,
               MEIOS_StrError(errno), dsm_status.ictl.retCode);

      return -1;
   }

   MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
      "VectorStatus = %i, VectorFriendlyStatus = %i" MEIOS_CRLF,
               dsm_status.eVectorStatus, dsm_status.eVectorFriendlyStatus);

   return 0;
}

/*
   DSM statistic get
*/
int MEI_dsm_statistics_get(MEIOS_File_t *streamOut, int fd)
{
   IOCTL_MEI_dsmStatistics_t dsm_statistics;

   memset(&dsm_statistics, 0x0, sizeof(IOCTL_MEI_dsmStatistics_t));

   if ( (MEIOS_DeviceControl(fd, FIO_MEI_DSM_STATISTICS_GET, (MEI_IOCTL_ARG)&dsm_statistics)) < 0 )
   {
      MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
               "ERROR - ioct(FIO_MEI_DSM_STATISTICS_GET), %s, retCode = %d" MEIOS_CRLF,
               MEIOS_StrError(errno), dsm_statistics.ictl.retCode);

      return -1;
   }

   MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX "n_processed = %i" MEIOS_CRLF
                            TEST_MEI_DBG_PREFIX "n_fw_dropped_size = %i" MEIOS_CRLF
                            TEST_MEI_DBG_PREFIX "n_mei_dropped_size = %i" MEIOS_CRLF
                            TEST_MEI_DBG_PREFIX "n_mei_dropped_no_pp_cb = %i" MEIOS_CRLF
                            TEST_MEI_DBG_PREFIX "n_pp_dropped = %i" MEIOS_CRLF,
      dsm_statistics.n_processed, dsm_statistics.n_fw_dropped_size,
      dsm_statistics.n_mei_dropped_size, dsm_statistics.n_mei_dropped_no_pp_cb,
      dsm_statistics.n_pp_dropped);

   return 0;
}

/*
   MAC address get
*/
int MEI_mac_get(MEIOS_File_t *streamOut, int fd)
{
   IOCTL_MEI_MacConfig_t mac_config;

   memset(&mac_config, 0x0, sizeof(IOCTL_MEI_MacConfig_t));

   if ( (MEIOS_DeviceControl(fd, FIO_MEI_MAC_CONFIG_GET, (MEI_IOCTL_ARG)&mac_config)) < 0 )
   {
      MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
               "ERROR - ioct(FIO_MEI_MAC_CONFIG_GET), %s, retCode = %d" MEIOS_CRLF,
               MEIOS_StrError(errno), mac_config.ictl.retCode);

      return -1;
   }

   MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
      "MAC address = %02X:%02X:%02X:%02X:%02X:%02X" MEIOS_CRLF,
               mac_config.nMacAddress[0], mac_config.nMacAddress[1],
               mac_config.nMacAddress[2], mac_config.nMacAddress[3],
               mac_config.nMacAddress[4], mac_config.nMacAddress[5]);

   return 0;
}

/*
   MAC address set
*/
int MEI_mac_set(MEIOS_File_t *streamOut, int fd, char *pMAC)
{
   IOCTL_MEI_MacConfig_t mac_config;

   memset(&mac_config, 0x0, sizeof(IOCTL_MEI_MacConfig_t));

   if (strlen(pMAC) == 0)
   {
      MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
               "ERROR - empty parameters, use -z option!" MEIOS_CRLF);
      return -1;
   }

   if (MEI_get_mac_addr((unsigned char *)pMAC, &mac_config) < 0)
   {
      MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
               "ERROR - inconsistent MAC address!" MEIOS_CRLF);

      return -1;
   }
   else
   {
      MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
         "MAC address = %02X:%02X:%02X:%02X:%02X:%02X" MEIOS_CRLF,
                  mac_config.nMacAddress[0], mac_config.nMacAddress[1],
                  mac_config.nMacAddress[2], mac_config.nMacAddress[3],
                  mac_config.nMacAddress[4], mac_config.nMacAddress[5]);
   }

   if ( (MEIOS_DeviceControl(fd, FIO_MEI_MAC_CONFIG_SET, (MEI_IOCTL_ARG)&mac_config)) < 0 )
   {
      MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
               "ERROR - ioct(FIO_MEI_MAC_CONFIG_SET), %s, retCode = %d" MEIOS_CRLF,
               MEIOS_StrError(errno), mac_config.ictl.retCode);

      return -1;
   }

   return 0;
}

/*
   Debug level set
*/
int MEI_dbg_lvl_set(MEIOS_File_t *streamOut, int fd, char *pConfig)
{
   IOCTL_MEI_dbgLevel_t dbg_lvl;
   char* dbg_module[3] = {"MEI_DRV", "MEI_MSG_DUMP_API", "MEI_NOTIFICATIONS"};
   char* dbg_level[4] = {"LOW", "NORMAL", "HIGH", "OFF"};

   memset(&dbg_lvl, 0x0, sizeof(IOCTL_MEI_dbgLevel_t));

   if (strlen(pConfig) == 0)
   {
      MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
               "ERROR - empty parameters, use -z option!" MEIOS_CRLF);
      return -1;
   }

   sscanf(pConfig, "%u %u", (unsigned int *)&dbg_lvl.eDbgModule, &dbg_lvl.valLevel);
   if ((dbg_lvl.eDbgModule < e_MEI_DBGMOD_MEI_DRV) ||
       (dbg_lvl.eDbgModule >= e_MEI_DBGMOD_LAST)   ||
       (dbg_lvl.valLevel < MEI_DBG_LEVEL_LOW)      ||
       (dbg_lvl.valLevel > MEI_DBG_LEVEL_OFF))
   {
      MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
               "ERROR - inconsistent parameters!" MEIOS_CRLF);
      return -1;
   }

   MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
      "DebugModule = %i (%s), DebugLevel = %i (%s)" MEIOS_CRLF,
               dbg_lvl.eDbgModule, dbg_module[dbg_lvl.eDbgModule - 1],
               dbg_lvl.valLevel, dbg_level[dbg_lvl.valLevel - 1]);

   if ( (MEIOS_DeviceControl(fd, FIO_MEI_DEBUGLEVEL, (MEI_IOCTL_ARG)&dbg_lvl)) < 0 )
   {
      MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
               "ERROR - ioct(FIO_MEI_DEBUGLEVEL), %s, retCode = %d" MEIOS_CRLF,
               MEIOS_StrError(errno), dbg_lvl.ictl.retCode);

      return -1;
   }

   return 0;
}

/*
   PLL offset get
*/
int MEI_pll_offset_get(MEIOS_File_t *streamOut, int fd)
{
   IOCTL_MEI_pllOffsetConfig_t pll_config;

   memset(&pll_config, 0x0, sizeof(IOCTL_MEI_pllOffsetConfig_t));

   if ( (MEIOS_DeviceControl(fd, FIO_MEI_PLL_OFFSET_CONFIG_GET, (MEI_IOCTL_ARG)&pll_config)) < 0 )
   {
      MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
               "ERROR - ioct(FIO_MEI_PLL_OFFSET_CONFIG_GET), %s, retCode = %d" MEIOS_CRLF,
               MEIOS_StrError(errno), pll_config.ictl.retCode);

      return -1;
   }

   MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
      "PLL offset get %i" MEIOS_CRLF, pll_config.nPllOffset);

   return 0;
}

/*
   PLL offset set
*/
int MEI_pll_offset_set(MEIOS_File_t *streamOut, int fd, int pll_offset)
{
   IOCTL_MEI_pllOffsetConfig_t pll_config;

   memset(&pll_config, 0x0, sizeof(IOCTL_MEI_pllOffsetConfig_t));

   pll_config.nPllOffset = pll_offset;

   MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
      "PLL offset set %i" MEIOS_CRLF, pll_config.nPllOffset);

   if ( (MEIOS_DeviceControl(fd, FIO_MEI_PLL_OFFSET_CONFIG_SET, (MEI_IOCTL_ARG)&pll_config)) < 0 )
   {
      MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
               "ERROR - ioct(FIO_MEI_PLL_OFFSET_CONFIG_SET), %s, retCode = %d" MEIOS_CRLF,
               MEIOS_StrError(errno), pll_config.ictl.retCode);

      return -1;
   }

   return 0;
}

#ifdef MEI_CURRENT_NOT_USED
static MEIOS_File_t *openMsgFile(char *pFileName);
static int getNextMsgLine(MEIOS_File_t *pFd, unsigned int *pParams, int maxParams);
static int scanMsgLine(char *pLineBuf, unsigned int *pParams, int maxParams);
#endif

#ifdef MEI_CURRENT_NOT_USED
/**
Open a VRX Msg file
*/
static MEIOS_File_t *openMsgFile(char *pFileName)
{
   MEIOS_File_t *pFd = NULL;

   if( (pFd = fopen( pFileName, "r")) == NULL )
   {
      /* error open given file */
      MEIOS_Printf("ERROR - Open Msg file %s" MEIOS_CRLF, pFileName);
      return NULL;
   }

   return pFd;
}


static int getNextMsgLine(MEIOS_File_t *pFd, unsigned int *pParams, int maxParams)
{
   char msgLine[255];

   if (fgets(msgLine, sizeof(msgLine), pFd) == NULL)
      return -1;


   return scanMsgLine(msgLine, pParams, maxParams);
}


static int scanMsgLine(char *pLineBuf, unsigned int *pParams, int maxParams)
{
   char *pCurrPos;
   int run = 1, index = 0;

   pCurrPos = pLineBuf;

   /* process the line */
   while( (index < maxParams) && run)
   {
      if ( sscanf(pCurrPos, "%X", &pParams[index]) == 1)
      {
         /* set next param */
         index++;

         /* find the end of the current param */
         while(1)
         {
            pCurrPos++;
            if ( (*pCurrPos == ' ') || (*pCurrPos == ';') ||
                 (*pCurrPos == '\n') || (*pCurrPos == '\r') || (*pCurrPos == '\0') )
            {
               break;
            }
         }

         /* find begin of the next param */
         while(1)
         {
            if (*pCurrPos != ' ')
            {
               if ( (*pCurrPos == '\n') || (*pCurrPos == '\r') || (*pCurrPos == '\0') )
               {
                  run = 0;
               }
               if (*pCurrPos == ';')
               {
                  run = 0;
               }
               break;
            }
            pCurrPos++;
         }
      }
      else
      {
         break;
      }
   }

   return index;
}
#endif      /* #ifdef MEI_CURRENT_NOT_USED */

#endif /* #ifdef LINUX */

