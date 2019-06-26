/******************************************************************************

                          Copyright (c) 2007-2015
                     Lantiq Beteiligungs-GmbH & Co. KG

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/** \file
   Debug and Trace Interface - Basic Protocol Packet Handling.
*/

/* ============================================================================
   Includes
   ========================================================================= */
#include "dti_osmap.h"
#include "dsl_cpe_control.h"

#if defined(INCLUDE_DSL_CPE_API_VRX)

#include "cmv_message_format.h"

#include "drv_mei_cpe_interface.h"
#include "drv_dsl_cpe_api_ioctl.h"

#include "ifx_dti_protocol.h"
#include "ifx_dti_protocol_device.h"

#ifdef DTI_DONT_USE_PROTEXT
/*
   Dummy defines, not requried for the DTI Serial Proxy.
*/
typedef struct DTI_ProtocolServerCtx_s
{
	IFX_int_t dummy;
} DTI_ProtocolServerCtx_t;

typedef struct DTI_Connection_s
{
	IFX_int_t dummy;
} DTI_Connection_t;


/* sorry for this style - we want to avoid that the dti_protocol_ext.h is included */
#ifndef _DTI_PROTOCOL_EXT_H
#define _DTI_PROTOCOL_EXT_H
#endif
#endif /* DTI_DONT_USE_PROTEXT */

#include "dti_device.h"
#include "dsl_cpe_dti_vrx_ctx.h"

/* ============================================================================
   Defines
   ========================================================================= */
#ifdef DTI_STATIC
#  undef DTI_STATIC
#endif

#ifdef DTI_DEBUG
#  define DTI_STATIC
#else
#  define DTI_STATIC       static
#endif

#define VRX_IOCTL_ARG     unsigned long

/** DTI Device Interface Name */
#define DTI_DEV_VRX_IF_NAME                          "DSL-CPE"
#define DTI_DEV_VRX_BOARD_NAME                       "DSL CPE Board"

/** Max number of register accesses per packet */
#define DTI_DEV_VRX_MAX_REGISTER_ACCESS_NUM          0x44
/** Max number of debug accesses per packet */
#define DTI_DEV_VRX_MAX_DEBUG_ACCESS_NUM             0x44
/** max DTI buffer size for debug stream handling */
#define DTI_DEV_MEI_DBG_STREAM_MAX_BUFFER_SIZE       0x10000

/** default VRX device mailbox size */
#define DTI_DEV_VRX_DEFAULT_MBOX_SIZE                CMV_USED_PAYLOAD_8BIT_SIZE

#define DTI_VRX_ARRAY_LENGTH(array) ((sizeof(array)/sizeof((array)[0])))

/* message header structure */
#define VRX_DTI_MSG_IDX_MSDID    0
#define VRX_DTI_MSG_IDX_INDEX    1
#define VRX_DTI_MSG_IDX_LENGTH   2

/* ============================================================================
   Local Function Declaration
   ========================================================================= */

DTI_STATIC IFX_int_t DTI_DevOpenByName(
                        IFX_int_t      dev_num,
                        IFX_char_t     *pDevName);

DTI_STATIC IFX_int_t DTI_DevCloseByFd(
                        IFX_int_t devFd);
                        
DTI_STATIC IFX_int_t DTI_DevOpen(
                        DTI_DEV_VrxDriverCtx_t      *pVrxDevCtx,
                        DTI_PacketError_t           *pPacketError,
                        IFX_int_t                   lineNum);

DTI_STATIC IFX_int_t DTI_DevClose(
                        DTI_DEV_VrxDriverCtx_t      *pVrxDevCtx,
                        DTI_PacketError_t           *pPacketError,
                        IFX_int_t                   lineNum);
                        
DTI_STATIC IFX_int_t DTI_configSet_AutoMsg(
                        DTI_DEV_VrxDriverCtx_t  *pVrxDevCtx,
                        DTI_H2D_DeviceConfigSet_t  *pInCfgSet,
                        DTI_D2H_DeviceConfigSet_t  *pOutCfgSet,
                        IFX_int_t                  lineNum,
                        DTI_PacketError_t          *pPacketError);

DTI_STATIC IFX_int_t DTI_configGet_AutoMsg(
                        DTI_DEV_VrxDriverCtx_t  *pVrxDevCtx,
                        IFX_int_t                  lineNum,
                        DTI_PacketError_t          *pPacketError);

#ifndef DTI_DONT_USE_PROTEXT
DTI_STATIC IFX_int_t DTI_autoMsgRecv(
                        DTI_DEV_VrxDriverCtx_t  *pVrxDevCtx,
                        const DTI_Connection_t     *pDtiCon,
                        IFX_int_t                  devIfNum,
                        IFX_int_t                  lineNum,
                        IFX_char_t                 *pOutBuffer,
                        IFX_int_t                  outBufferSize_byte);
#endif /* !DTI_DONT_USE_PROTEXT */

#ifndef DTI_DONT_USE_PROTEXT
DTI_STATIC IFX_void_t DTI_devFdClear(
                        DTI_DEV_VrxDriverCtx_t  *pVrxDevCtx,
                        IFX_int_t                  lineNum);
#endif /* !DTI_DONT_USE_PROTEXT */

DTI_STATIC IFX_int_t DTI_configGet_MbSize(
                        DTI_DEV_VrxDriverCtx_t  *pVrxDevCtx,
                        IFX_int_t                  lineNum,
                        DTI_PacketError_t          *pPacketError);

/* ============================================================================
   Device Access Functions
   ========================================================================= */

DTI_STATIC IFX_int_t DTI_VRX_PrintoutLevelSet(
                        IFX_int_t            newDbgLevel);

DTI_STATIC IFX_int_t DTI_VRX_ModuleSetup(
                        DTI_DeviceSysInfo_t  *pDeviceSystemInfo,
                        DTI_DeviceCtx_t      **ppDtiDevCtx);

DTI_STATIC IFX_int_t DTI_VRX_ModuleDelete(
                        DTI_DeviceSysInfo_t  *pDeviceSystemInfo,
                        DTI_DeviceCtx_t      **ppDtiDevCtx);

DTI_STATIC IFX_int_t DTI_VRX_SystemInfoWrite(
                        DTI_DeviceSysInfo_t  *pDeviceSystemInfo,
                        IFX_char_t           *pSysInfoBuffer,
                        IFX_int_t            bufferSize);

DTI_STATIC IFX_int_t DTI_VRX_Reset(
                        DTI_DeviceCtx_t         *pDtiDevCtx,
                        DTI_H2D_DeviceReset_t   *pInDevReset,
                        DTI_D2H_DeviceReset_t   *pOutDevReset,
                        IFX_int_t               rstMaskSize_32,
                        DTI_PacketError_t       *pPacketError);

DTI_STATIC IFX_int_t DTI_VRX_Download(
                        DTI_DeviceCtx_t            *pDtiDevCtx,
                        DTI_ProtocolServerCtx_t    *pDtiProtServerCtx,
                        DTI_H2D_DeviceDownload_t   *pInDevDownload,
                        DTI_D2H_DeviceDownload_t   *pOutDevDownload,
                        DTI_PacketError_t          *pPacketError);

DTI_STATIC IFX_int_t DTI_VRX_DeviceOpen(
                        DTI_DeviceCtx_t      *pDtiDevCtx,
                        IFX_int_t            lineNum,
                        DTI_PacketError_t    *pPacketError);

DTI_STATIC IFX_int_t DTI_VRX_DeviceClose(
                        DTI_DeviceCtx_t      *pDtiDevCtx,
                        IFX_int_t            lineNum,
                        DTI_PacketError_t    *pPacketError);

DTI_STATIC IFX_int_t DTI_VRX_RegisterLock(
                        DTI_DeviceCtx_t      *pDtiDevCtx,
                        DTI_H2D_DeviceLock_t *pInLock,
                        DTI_D2H_DeviceLock_t *pOutLock,
                        IFX_int_t            lineNum,
                        DTI_PacketError_t    *pPacketError);

DTI_STATIC IFX_int_t DTI_VRX_RegisterGet(
                        DTI_DeviceCtx_t         *pDtiDevCtx,
                        DTI_H2D_RegisterGet_t   *pInRegGet,
                        DTI_D2H_RegisterGet_t   *pOutRegGet,
                        IFX_int_t               lineNum,
                        IFX_uint32_t            *pOutPaylSize_byte,
                        DTI_PacketError_t       *pPacketError);

DTI_STATIC IFX_int_t DTI_VRX_RegisterSet(
                        DTI_DeviceCtx_t         *pDtiDevCtx,
                        DTI_H2D_RegisterSet_t   *pInRegSet,
                        IFX_int_t               inRegSetSize_Byte,
                        IFX_int_t               lineNum,
                        DTI_PacketError_t       *pPacketError);

DTI_STATIC IFX_int_t DTI_VRX_ConfigSet(
                        DTI_DeviceCtx_t            *pDtiDevCtx,
                        DTI_H2D_DeviceConfigSet_t  *pInCfgSet,
                        DTI_D2H_DeviceConfigSet_t  *pOutCfgSet,
                        IFX_int_t                  lineNum,
                        DTI_PacketError_t          *pPacketError);

DTI_STATIC IFX_int_t DTI_VRX_ConfigGet(
                        DTI_DeviceCtx_t            *pDtiDevCtx,
                        DTI_H2D_DeviceConfigGet_t  *pInCfgGet,
                        DTI_D2H_DeviceConfigGet_t  *pOutCfgGet,
                        IFX_int_t                  lineNum,
                        DTI_PacketError_t          *pPacketError);
                        
#if (DTI_DEVICE_INTERFACE_VERSION) > (0x00000100)
DTI_STATIC IFX_int_t DTI_VRX_Message8Send(
                        DTI_DeviceCtx_t * pDtiDevCtx,
                        DTI_H2D_Message8_u *pInMsg8Send,
                        DTI_D2H_Message8_u *pOutMsg8Send, 
                        IFX_int_t devNum,
                        IFX_int_t inPaylSize_byte,
                        IFX_int_t * pOutPaylSize_byte,
                        DTI_PacketError_t *pPacketError);
#endif

DTI_STATIC IFX_int_t DTI_VRX_Message16Send(
                        DTI_DeviceCtx_t      *pDtiDevCtx,
                        DTI_H2D_Message16_u  *pInMsg16Send,
                        DTI_D2H_Message16_u  *pOutMsg16Send,
                        IFX_int_t            lineNum,
                        IFX_int_t            inPaylSize_byte,
                        IFX_int_t            *pOutPaylSize_byte,
                        DTI_PacketError_t    *pPacketError);

DTI_STATIC IFX_int_t DTI_VRX_Message32Send(
                        DTI_DeviceCtx_t      *pDtiDevCtx,
                        DTI_H2D_Message32_u  *pInMsg32Send,
                        DTI_D2H_Message32_u  *pOutMsg32Send,
                        IFX_int_t            lineNum,
                        IFX_int_t            inPaylSize_byte,
                        IFX_int_t            *pOutPaylSize_byte,
                        DTI_PacketError_t    *pPacketError);

DTI_STATIC IFX_int_t DTI_VRX_TraceBufferConfigSet(
                        DTI_DeviceCtx_t            *pDtiDevCtx,
                        DTI_H2D_TraceConfigSet_t   *pInTraceConfigSet,
                        DTI_D2H_TraceConfigSet_t   *pOutTraceConfigSet,
                        IFX_int_t                  lineNum,
                        DTI_PacketError_t          *pPacketError);

DTI_STATIC IFX_int_t DTI_VRX_TraceBufferReset(
                        DTI_DeviceCtx_t      *pDtiDevCtx,
                        IFX_int_t            lineNum,
                        DTI_PacketError_t    *pPacketError);

DTI_STATIC IFX_int_t DTI_VRX_TraceBufferStatusGet(
                        DTI_DeviceCtx_t            *pDtiDevCtx,
                        DTI_D2H_TraceStatusGet_t   *pOutTraceStatusGet,
                        IFX_int_t                  lineNum,
                        DTI_PacketError_t          *pPacketError);

DTI_STATIC IFX_int_t DTI_VRX_TraceBufferGet(
                        DTI_DeviceCtx_t            *pDtiDevCtx,
                        DTI_H2D_TraceBufferGet_t   *pInTraceBufferGet,
                        DTI_Packet_t               **ppUsedDtiPacketOut,
                        IFX_int_t                  *pUsedBufferOutSize,
                        IFX_int_t                  lineNum,
                        IFX_int_t                  *pTrBufReadSize_byte,
                        DTI_PacketError_t          *pPacketError);

DTI_STATIC IFX_int_t DTI_VRX_DebugRead(
                        DTI_DeviceCtx_t      *pDtiDevCtx,
                        DTI_H2D_DebugRead_t  *pInDbgGet,
                        DTI_D2H_DebugRead_t  *pOutDbgGet,
                        IFX_int_t            lineNum,
                        IFX_int_t            *pDbgReadCount,
                        DTI_PacketError_t    *pPacketError);

DTI_STATIC IFX_int_t DTI_VRX_DebugWrite(
                        DTI_DeviceCtx_t      *pDtiDevCtx,
                        DTI_H2D_DebugWrite_t *pInDbgSet,
                        IFX_uint32_t         *pOutDbgGet_nU,
                        IFX_int_t            lineNum,
                        IFX_int_t            *pDbgWriteCount,
                        DTI_PacketError_t    *pPacketError);

DTI_STATIC IFX_int_t DTI_VRX_AutoMsgProcess(
                        DTI_DeviceCtx_t         *pDtiDevCtx,
                        const DTI_Connection_t  *pDtiCon,
                        IFX_uint32_t            devSelectWait_ms,
                        IFX_char_t              *pOutBuffer,
                        IFX_int_t               outBufferSize_byte);

DTI_STATIC IFX_int_t DTI_VRX_WinEasyCiAccess(
                        DTI_DeviceCtx_t      *pDtiDevCtx,
                        IFX_int_t            lineNum,
                        const IFX_uint8_t    *pDataIn,
                        const IFX_uint32_t   sizeIn,
                        IFX_uint8_t          *pDataOut,
                        const IFX_uint32_t   sizeOut,
                        DTI_PacketError_t    *pPacketError);
                        
/* ============================================================================
   Variables
   ========================================================================= */

/* Create device debug module - user part */
IFXOS_PRN_USR_MODULE_CREATE(DTI_VRX, DTI_PRN_LEVEL_HIGH);

/**
   Collection of all available device access functions.
*/
DTI_DeviceAccessFct_t DTI_DeviceAccessFct_VRX =
{
   sizeof(DTI_DeviceAccessFct_t),         /* structure check */
   DTI_DEVICE_INTERFACE_VERSION,          /* device interface version */
   DTI_DEV_VRX_IF_NAME,                  /* device name */

   DTI_VRX_PrintoutLevelSet,
   DTI_VRX_ModuleSetup,
   DTI_VRX_ModuleDelete,
   DTI_VRX_SystemInfoWrite,

   DTI_VRX_Reset,
   DTI_VRX_Download,
   DTI_VRX_DeviceOpen,
   DTI_VRX_DeviceClose,
   DTI_VRX_RegisterLock,
   DTI_VRX_RegisterGet,
   DTI_VRX_RegisterSet,
   DTI_VRX_ConfigSet,
   DTI_VRX_ConfigGet,
   DTI_VRX_Message16Send,
   DTI_VRX_Message32Send,
   DTI_VRX_TraceBufferConfigSet,
   DTI_VRX_TraceBufferReset,
   DTI_VRX_TraceBufferStatusGet,
   DTI_VRX_TraceBufferGet,
   DTI_VRX_DebugRead,
   DTI_VRX_DebugWrite,
   DTI_VRX_AutoMsgProcess,
   DTI_VRX_WinEasyCiAccess,
#if (DTI_DEVICE_INTERFACE_VERSION) > (0x00000100)
   DTI_VRX_Message8Send,
#endif
};


/* ============================================================================
   Local Function
   ========================================================================= */

static DSL_int_t DTI_VrxDeviceStateCheck(
   DSL_int_t fd)
{
   IOCTL_MEI_reqCfg_t  Vdsl2_requCfg;

   memset(&Vdsl2_requCfg, 0x0, sizeof(IOCTL_MEI_reqCfg_t));

   if (DSL_CPE_Ioctl(fd, FIO_MEI_REQ_CONFIG, (DSL_int_t)&Vdsl2_requCfg) != 0)
   {
      DSL_CCA_DEBUG (DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
         "ioct(FIO_MEI_REQ_CONFIG): ERROR - cannot request config, retCode = %d!"DSL_CPE_CRLF,
         Vdsl2_requCfg.ictl.retCode));
      return -1;
   }

   if (Vdsl2_requCfg.currDrvState != 6)
   {
      DSL_CCA_DEBUG (DSL_CCA_DBG_ERR, (DSL_CPE_PREFIX
         "ioct(FIO_MEI_REQ_CONFIG): ERROR - device not available, state = %d!"DSL_CPE_CRLF,
         Vdsl2_requCfg.currDrvState));
      return -1;
   }

   return 0;
}

/**
   Open a line or control device of the VRX driver

\param
   num         - control or line device number
\param
   bCntrlDev   - if set open a control device
\param
   pDevName    - device base name

\return
   if success, a valid device fd
   else IFX_ERROR.
*/
DTI_STATIC IFX_int_t DTI_DevOpenByName(
                        IFX_int_t      dev_num,
                        IFX_char_t     *pDevName)
{
   DSL_char_t text[30];
   DSL_int_t mei_fd = -1;

   if (pDevName == DSL_NULL)
   {
      return -1;
   }

   snprintf(text, sizeof(text),"%s/%d", pDevName, dev_num);
   text[sizeof(text) - 1]='\0';

   mei_fd = DSL_CPE_Open(text);
   /*mei_fd = open(text, O_RDWR);*/

   if (mei_fd < 0)
   {
      DSL_CCA_DEBUG (DSL_CCA_DBG_MSG, (DSL_CPE_PREFIX
         "xDSL device driver cannot be opened!"DSL_CPE_CRLF));
      return -1;
   }

   /* Check VRX device driver state*/
   if (DTI_VrxDeviceStateCheck(mei_fd) < 0)
   {
      DSL_CPE_Close(mei_fd);
      DSL_CCA_DEBUG (DSL_CCA_DBG_MSG, (DSL_CPE_PREFIX
         "xDSL device driver not ready!"DSL_CPE_CRLF));
      return -1;
   }

   return mei_fd;
}

/**
   Close a line or control device of the VRX driver

\param
   devFd    - a valid device fd

\return
   If success IFX_SUCCESS else IFX_ERROR.

*/
DTI_STATIC IFX_int_t DTI_DevCloseByFd(
                        IFX_int_t devFd)
{
   IFX_int_t   retVal = IFX_ERROR;

   if (devFd > 0)
   {
      retVal = DSL_CPE_Close(devFd);
   }

   return retVal;
}

DTI_STATIC IFX_int_t DTI_DevOpen(
                        DTI_DEV_VrxDriverCtx_t      *pVrxDevCtx,
                        DTI_PacketError_t           *pPacketError,
                        IFX_int_t                   lineNum)
{
   *pPacketError = DTI_eNoError;

   if (pVrxDevCtx->pDevFds[lineNum] < 0)
   {
      pVrxDevCtx->pDevFds[lineNum] = 
         DTI_DevOpenByName(lineNum, DSL_CPE_DSL_LOW_DEV);
      if (pVrxDevCtx->pDevFds[lineNum] < 0)
      {
         *pPacketError = DTI_eErrPortOpen;
         return IFX_ERROR;
      }
   }

   return IFX_SUCCESS;
}                        

DTI_STATIC IFX_int_t DTI_DevClose(
                        DTI_DEV_VrxDriverCtx_t      *pVrxDevCtx,
                        DTI_PacketError_t           *pPacketError,
                        IFX_int_t                   lineNum)
{
   *pPacketError = DTI_eNoError;

   if (pVrxDevCtx->pDevFds[lineNum] >= 0)
   {
      (void)DTI_DevCloseByFd(pVrxDevCtx->pDevFds[lineNum]);
      pVrxDevCtx->pDevFds[lineNum] = -1;
   }

   return IFX_SUCCESS;
}

/*
   Config Set - enable / disable autonomous messages.

\param
   pVrxDevCtx    - points to the VRX Device context.
\param
   pInCfgSet      - points to the config data which will be set.
\param
   pOutCfgSet     - points to the data structure to return the actual config.
\param
   lineNum        - line number
\param
   pPacketError   - returns the DTI operation result

\return
   -1 if something went wrong, else
   size of the mailbox
*/
DTI_STATIC IFX_int_t DTI_configSet_AutoMsg(
                        DTI_DEV_VrxDriverCtx_t    *pVrxDevCtx,
                        DTI_H2D_DeviceConfigSet_t  *pInCfgSet,
                        DTI_D2H_DeviceConfigSet_t  *pOutCfgSet,
                        IFX_int_t                  lineNum,
                        DTI_PacketError_t          *pPacketError)
{
#ifndef DTI_DONT_USE_PROTEXT
   IFX_int_t            nfcChange = 1;
   IOCTL_MEI_ioctl_t drvIoCtl;

   pOutCfgSet->key = pInCfgSet->key;
   pOutCfgSet->value = 0;

   if ((pInCfgSet->value != 1) && (pInCfgSet->value != 0))
   {
      *pPacketError     = DTI_eErrInvalidParameters;

      return IFX_SUCCESS;
   }

   if (pVrxDevCtx->bAutoDevMsgSupport)
   {
      if ( (pVrxDevCtx->pDevFds[lineNum] == -1) && (pInCfgSet->value == 0) )
      {
         nfcChange = 0;
      }
      else
      {
         DTI_devFdClear(pVrxDevCtx, lineNum);
      }

      if (nfcChange == 1)
      {
         DTI_MemSet(&drvIoCtl, 0x00, sizeof(drvIoCtl)); 

         if (DTI_DevOpen(pVrxDevCtx, pPacketError, lineNum) != IFX_SUCCESS)
         {
            DTI_PRN_USR_ERR_NL(DTI_VRX, DTI_PRN_LEVEL_ERR,
               ("ERROR: Vrx Config Set AutoMsg - device open error." DTI_CRLF));

            *pPacketError = DTI_eErrPortOpen;
            return IFX_ERROR;
         }

         if ( (DSL_CPE_Ioctl(
                     pVrxDevCtx->pDevFds[lineNum],
                     (pInCfgSet->value == 0) ? FIO_MEI_MBOX_NFC_DISABLE : FIO_MEI_MBOX_NFC_ENABLE, 
                     (DSL_int_t)&drvIoCtl)) < 0 )
         {
            DTI_PRN_USR_ERR_NL(DTI_VRX, DTI_PRN_LEVEL_ERR,
               ("ERROR: Vrx Config Set AutoMsg - ioctl, NFC %s."DTI_CRLF,
                 (pInCfgSet->value == 0) ? "disable" : "enable"));

            *pPacketError = DTI_eErrPortOperation;
         }
         else
         {
            if (pInCfgSet->value == 1)
            {
               IFXOS_DevFdSet( (IFX_uint32_t)pVrxDevCtx->pDevFds[lineNum],
                               &pVrxDevCtx->nfcDevFds);

               if ((pVrxDevCtx->pDevFds[lineNum] + 1) > pVrxDevCtx->nfcMaxDevFd)
               {
                  pVrxDevCtx->nfcMaxDevFd = pVrxDevCtx->pDevFds[lineNum] + 1;
               }
               pOutCfgSet->value = 1;
            }
         }
      }
   }
   else
#endif /* !DTI_DONT_USE_PROTEXT */
   {   
      DTI_PRN_USR_ERR_NL(DTI_VRX, DTI_PRN_LEVEL_ERR,
         ("ERROR: Vrx Cfg Set - Auto msg not supported."DTI_CRLF));

      *pPacketError     = DTI_eErrConfiguration;
   }

   return IFX_SUCCESS;
}

/*
   Config Get - enable / disable status of autonous messages.

\param
   pVrxDevCtx    - points to the VRX Device context.
\param
   lineNum        - line number
\param
   pPacketError   - returns the DTI operation result

\return
   1 if auto msg is enabled
   0 if auto msg is disabled
*/
DTI_STATIC IFX_int_t DTI_configGet_AutoMsg(
                        DTI_DEV_VrxDriverCtx_t *pVrxDevCtx,
                        IFX_int_t               lineNum,
                        DTI_PacketError_t       *pPacketError)
{
   IFX_int_t keyValue = 0;

#ifndef DTI_DONT_USE_PROTEXT
   if (pVrxDevCtx->bAutoDevMsgSupport)
   {
      if ( (pVrxDevCtx->pDevFds[lineNum] >= 0) && (pVrxDevCtx->nfcMaxDevFd > 0))
      {
         keyValue = (IFXOS_DevFdIsSet( (IFX_uint32_t)pVrxDevCtx->pDevFds[lineNum],
                                       &pVrxDevCtx->nfcDevFds)) ? 1 : 0;
      }
      else
      {
         keyValue = 0;
      }

      *pPacketError = DTI_eNoError;
   }
   else
#endif /* !DTI_DONT_USE_PROTEXT */   
   {
      *pPacketError = DTI_eErrConfiguration;
   }

   return keyValue;
}

#ifndef DTI_DONT_USE_PROTEXT
/**
   Clear a device FD from the FD_SET and recalc new max FD.

\param
   pVrxDevCtx    - points to the VRX Device context.
\param
   lineNum        - line number

*/
DTI_STATIC IFX_void_t DTI_devFdClear(
                        DTI_DEV_VrxDriverCtx_t *pVrxDevCtx,
                        IFX_int_t               lineNum)
{
#if defined(IFXOS_HAVE_DEV_FD_CLEAR)
   IFX_int_t   i, maxDevFd = 0;

   DTI_DevFdClear(pVrxDevCtx->pDevFd[lineNum], &pVrxDevCtx->nfcDevFds);

   for (i = 0; i < pVrxDevCtx->numOfPorts; i++)
   {
      if (pVrxDevCtx->pDevFd[i] >= 0)
      {
         if ( (DTI_DevFdIsSet(pVrxDevCtx->pDevFds[i], &pVrxDevCtx->nfcDevFds)) &&
              (pVrxDevCtx->pDevFds[i] > maxDevFd) }
         {
            maxDevFd = pVrxDevCtx->pDevFds[i];
         }
      }
   }

   pVrxDevCtx->nfcMaxDevFd = maxDevFd + 1;

#else
   IFX_int_t            i, maxDevFd = 0;

   DTI_DevFdZero(&pVrxDevCtx->tmpDevFds);
   pVrxDevCtx->nfcMaxDevFd = 0;

   for (i = 0; i < pVrxDevCtx->numOfPorts; i++)
   {
      if ( (pVrxDevCtx->pDevFds[i] >= 0) && (i != lineNum))
      {
         if ( DTI_DevFdIsSet((IFX_uint32_t)pVrxDevCtx->pDevFds[i], &pVrxDevCtx->nfcDevFds) )
         {
            DTI_DevFdSet((IFX_uint32_t)pVrxDevCtx->pDevFds[i], &pVrxDevCtx->tmpDevFds);

            if (pVrxDevCtx->pDevFds[i] > maxDevFd)
            {
               maxDevFd = pVrxDevCtx->pDevFds[i];
            }
         }
      }
   }

   DTI_MemCpy(&pVrxDevCtx->nfcDevFds, &pVrxDevCtx->tmpDevFds, sizeof(IFXOS_devFd_set_t));
   pVrxDevCtx->nfcMaxDevFd = (maxDevFd) ? (maxDevFd + 1) : 0;

   DTI_DevFdZero(&pVrxDevCtx->tmpDevFds);

#endif
   return;
}
#endif /* !DTI_DONT_USE_PROTEXT */

#ifndef DTI_DONT_USE_PROTEXT
/**
   Check and process for xDSL Rev3 Device Autonomous Messages.
   The Auto Msg is read form the device (driver) and a corresponding
   DTI packet is created and sent to the upper DTI Client

\param
   pVrxDevCtx          - points to the VRX Device context.
\param
   pDtiCon              - points to the established DTI Connection data.
\param
   lineNum              - line number.
\param
   pOutBuffer           - points to the DTI packet out buffer to create a DTI packet.
\param
   outBufferSize_byte   - DTI out buffer size [byte]

\return
   IFX_SUCCESS if the DTI packet has been sent.
   IFX_ERROR   DTI packet has not been sent.

\remarks
   At first the device msg will be read out to free the internal driver buffer.
   This is done independant on the established connection.
*/
DTI_STATIC IFX_int_t DTI_autoMsgRecv(
                        DTI_DEV_VrxDriverCtx_t *pVrxDevCtx,
                        const DTI_Connection_t  *pDtiCon,
                        IFX_int_t               devIfNum,
                        IFX_int_t               lineNum,
                        IFX_char_t              *pOutBuffer,
                        IFX_int_t               outBufferSize_byte)
{
   IFX_int_t            paylBufferSize_byte = 0;
   DTI_PacketError_t    packetError     = DTI_eNoError;
   DTI_Packet_t         *pAutoMsgPacket = IFX_NULL;
   IFX_uint16_t         *pModemPayload  = IFX_NULL;
   DTI_D2H_Message16_u  *pMsg16;
   IOCTL_MEI_message_t  msgRead;
   DTI_PTR_U            uPayload;

   /*
      First: read out the msg to free the driver.
   */
   DTI_MemSet(&msgRead, 0x00, sizeof(msgRead));

   msgRead.pPayload      = (unsigned char*)pVrxDevCtx->pAutoMsgBuf;
   msgRead.paylSize_byte = (unsigned int)pVrxDevCtx->autoMsgBuf_byte;

   if ( DSL_CPE_Ioctl(
               pVrxDevCtx->pDevFds[lineNum],
               FIO_MEI_MBOX_NFC_RD,
               (DSL_int_t)&msgRead) < 0 )
   {
      DTI_PRN_USR_ERR_NL(DTI_VRX, DTI_PRN_LEVEL_ERR,
         ("ERROR: Vrx Auto Msg[0x04X%] - ioctl error)."DTI_CRLF,
           msgRead.msgId));

      DTI_PRN_USR_ERR_NL(DTI_VRX, DTI_PRN_LEVEL_ERR,
         ("\tNFC[0x04X%]."DTI_CRLF,msgRead.msgId));

      packetError = DTI_eErrPortAutoOperation;
   }
   
   /*
      Second: prepare and send the packet.
   */
   if (outBufferSize_byte < (sizeof(DTI_PacketHeader_t) + sizeof(IOCTL_MEI_message_t)))
   {
      DTI_PRN_USR_ERR_NL(DTI_VRX, DTI_PRN_LEVEL_ERR,
         ("ERROR: Vrx Dev Auto Msg - not enough out buffer."DTI_CRLF));

      return IFX_ERROR;
   }

   /* setup out packet */
   uPayload.pUInt8 = (IFX_uint8_t *)pOutBuffer;
   pAutoMsgPacket  = (DTI_Packet_t *)DTI_PTR_CAST_GET_ULONG(uPayload);
   if (pAutoMsgPacket == IFX_NULL)
   {
      DTI_PRN_USR_ERR_NL(DTI_VRX, DTI_PRN_LEVEL_ERR,
         ("ERROR: Vrx Dev Auto Msg - DTI Packet out buffer missaligned."DTI_CRLF));

      return IFX_ERROR;
   }

   /* setup out packet - payload */
   uPayload.pUInt8 = (IFX_uint8_t *)pAutoMsgPacket->payload;
   pMsg16     = (DTI_D2H_Message16_u *)DTI_PTR_CAST_GET_UINT16(uPayload);
   if (pMsg16 == IFX_NULL)
   {
      DTI_PRN_USR_ERR_NL(DTI_VRX, DTI_PRN_LEVEL_ERR,
         ("ERROR: Vrx Dev Auto Msg - DTI Payload out buffer missaligned."DTI_CRLF));

      packetError = (packetError == DTI_eNoError) ? DTI_eErrMalformedPacket : packetError;
   }

   if (packetError == DTI_eNoError)
   {
      /* modem msg payload = index[16Bit] + length[16Bit] + Payload Data[16/32 Bit] */
      uPayload.pUInt8 = (IFX_uint8_t *)msgRead.pPayload;
      pModemPayload   = (IFX_uint16_t *)DTI_PTR_CAST_GET_UINT16(uPayload);
      if (pModemPayload == IFX_NULL)
      {
         DTI_PRN_USR_ERR_NL(DTI_VRX, DTI_PRN_LEVEL_ERR,
            ("ERROR: Vrx Dev Auto Msg - DTI Payload modem buffer missaligned."DTI_CRLF));

         packetError = (packetError == DTI_eNoError) ? DTI_eErrMalformedPacket : packetError;
      }
   }

   if ( (packetError == DTI_eNoError) && (pModemPayload != IFX_NULL) )
   {
      /* DTI Packet Message [16/32 Bit]:
            DTI Packet Header + Payload Hdr [msg ID, index, length] + Payload Data [16/32 Bit] */
      paylBufferSize_byte = outBufferSize_byte - sizeof(DTI_PacketHeader_t);
      paylBufferSize_byte -= sizeof(pMsg16->raw.data[0])*4 + sizeof(pMsg16->raw.sendResult);

      if ( paylBufferSize_byte < ((IFX_int_t)msgRead.paylSize_byte) )
      {
         /* not enough buffer - cut payload and setup error packet */
         DTI_PRN_USR_ERR_NL(DTI_VRX, DTI_PRN_LEVEL_ERR,
            ("ERROR: Vrx Dev Auto Msg - cut Payload."DTI_CRLF));
         packetError = DTI_eErrInvalidPayloadSize;
      }
      else
      {
         paylBufferSize_byte = (IFX_int_t)msgRead.paylSize_byte;
         if (msgRead.paylSize_byte & 0x1)
         {
            /* add 1 byte in case of odd msg length */
            paylBufferSize_byte +=1;
         }
         /* Modem 16 Bit message */
         (void)DTI_headerPacketTypeSet(
               pAutoMsgPacket,
               (IFX_uint32_t)DTI_PacketType_eMessageSend,
               (IFX_uint32_t)DTI_e16Bit,
               (IFX_uint32_t)((sizeof(pMsg16->raw.data[VRX_DTI_MSG_IDX_MSDID]) + sizeof(pMsg16->raw.sendResult)) +
                              paylBufferSize_byte) );

         pMsg16->raw.sendResult = IFX_SUCCESS;
         pMsg16->raw.data[VRX_DTI_MSG_IDX_MSDID] = msgRead.msgId;
         if (paylBufferSize_byte > 0)
         {
            DTI_MemCpy(&pMsg16->raw.data[VRX_DTI_MSG_IDX_INDEX], msgRead.pPayload, paylBufferSize_byte);
         }
      }
   }

   if (packetError != DTI_eNoError)
   {
      (void)DTI_headerPacketTypeSet(
               pAutoMsgPacket,
               (IFX_uint32_t)DTI_PacketType_eMessageError, (IFX_uint32_t)DTI_e8Bit, 0);
   }

   /* complete header setup */
   pAutoMsgPacket->header.port   = DTI_HDR_PORT_PORT_NUM_SET(pAutoMsgPacket->header.port, lineNum);
   pAutoMsgPacket->header.port   = DTI_HDR_PORT_DEV_TYPE_NUM_SET(pAutoMsgPacket->header.port, devIfNum);
   pAutoMsgPacket->header.tan    = (IFX_uint32_t)0;
   pAutoMsgPacket->header.error  = (IFX_uint32_t)packetError;

   DTI_packetShow (
      pAutoMsgPacket, IFX_TRUE, IFX_FALSE, "Vrx AutoMsg",
      (pAutoMsgPacket->header.error == DTI_eNoError) ? IFXOS_PRN_LEVEL_LOW : DTI_PRN_LEVEL_HIGH);

   if (DTI_packetSend(pDtiCon, pAutoMsgPacket) != IFX_SUCCESS)
   {
      DTI_PRN_USR_ERR_NL(DTI_VRX, DTI_PRN_LEVEL_ERR,
         ("ERROR: Vrx Dev Auto Msg - send packet."DTI_CRLF));

      return IFX_ERROR;
   }

   return IFX_SUCCESS;
}
#endif /* !DTI_DONT_USE_PROTEXT */

/*
   Get the Line Cfg from the driver and return the mailbox size.

\param
   pVrxDevCtx    - points to the VRX Device context.
\param
   lineNum        - line number
\param
   pPacketError   - returns the DTI operation result

\return
   -1 if something went wrong, else
   size of the mailbox
*/
DTI_STATIC IFX_int_t DTI_configGet_MbSize(
                        DTI_DEV_VrxDriverCtx_t *pVrxDevCtx,
                        IFX_int_t               lineNum,
                        DTI_PacketError_t       *pPacketError)
{
   IOCTL_MEI_reqCfg_t reqCfg;

   if (DTI_DevOpen(pVrxDevCtx, pPacketError, lineNum) != IFX_SUCCESS)
   {
      DTI_PRN_USR_ERR_NL(DTI_VRX, DTI_PRN_LEVEL_ERR,
         ("ERROR: Vrx Config Get MB Size - device open error."DTI_CRLF));

      return -1;
   }

   DTI_MemSet(&reqCfg, 0x00, sizeof(IOCTL_MEI_reqCfg_t));

   if ( (DSL_CPE_Ioctl(
            pVrxDevCtx->pDevFds[lineNum],
            FIO_MEI_REQ_CONFIG,
            (DSL_int_t)&reqCfg)) < 0 )
   {
      DTI_PRN_USR_ERR_NL(DTI_VRX, DTI_PRN_LEVEL_ERR,
         ("ERROR: Vrx Config Get MB Size - request config error."DTI_CRLF));

      *pPacketError = DTI_eErrPortOperation;
      return -1;
   }
   else
   {
      DTI_PRN_USR_DBG_NL(DTI_VRX, DTI_PRN_LEVEL_NORMAL,
         ("Vrx Config MB Size: ME2ARC = 0x%X, ARC2ME = 0x%X."DTI_CRLF,
           reqCfg.Me2ArcOnlineMbSize, reqCfg.Arc2MeOnlineMbSize));

      *pPacketError = DTI_eNoError;
      return (IFX_int_t)reqCfg.Me2ArcOnlineMbSize;
   }
}

/**
   xDSL Debug Stream setup
      - request current config
      - if still not init do init if data are given.

\param
   priv_ctx - points to the VRX Device context.
\param
   line_num        - line number
\param
   cfg_get        - incoming configuration
\param
   cfg_set        - outgoing configuration

\return
   -1 if something went wrong, else
   size of the mailbox
*/
DTI_STATIC IFX_int_t
DTI_traceBufferDbgStrmCfg(DTI_DEV_VrxDriverCtx_t *pVrxDevCtx,
                          IFX_int_t lineNum,
                          IOCTL_MEI_DEBUG_STREAM_configGet_t *pCfgGet,
                          IOCTL_MEI_DEBUG_STREAM_configSet_t *pCfgSet)
{
   /* get current config */
   DTI_MemSet(pCfgGet, 0x00, sizeof(IOCTL_MEI_DEBUG_STREAM_configGet_t));

   if (IFXOS_DeviceControl(pVrxDevCtx->pDevFds[lineNum],
      FIO_MEI_DEBUG_STREAM_CONFIG_GET,
      (IFX_int_t)pCfgGet) < 0) 
   {
      if (pCfgSet != IFX_NULL) 
      {
         if (IFXOS_DeviceControl(pVrxDevCtx->pDevFds[lineNum],
            FIO_MEI_DEBUG_STREAM_CONFIG_SET,
            (IFX_int_t)pCfgSet) < 0)
         {
            DTI_PRN_USR_ERR_NL(
            DTI_VRX, DTI_PRN_LEVEL_ERR,
            ("ERROR: Mei dbgStreamCfgGet - "\
            "line %d, init set error."DTI_CRLF,
            lineNum));

            return IFX_ERROR;
         }
         else
         {
            DTI_MemSet(pCfgGet, 0x00, sizeof(IOCTL_MEI_DEBUG_STREAM_configGet_t));
            if (IFXOS_DeviceControl(
               pVrxDevCtx->pDevFds[lineNum],
               FIO_MEI_DEBUG_STREAM_CONFIG_GET,
               (IFX_int_t)pCfgGet) < 0)
            {
               DTI_PRN_USR_ERR_NL(DTI_VRX,
               DTI_PRN_LEVEL_ERR,
               ("ERROR: Mei dbgStreamCfgGet "\
               "- line %d, re-init get error"\
               DTI_CRLF, lineNum));

               return IFX_ERROR;
            }
            else
            {
               return IFX_SUCCESS;
            }
         }
      }
   }

   return IFX_SUCCESS;
}

DTI_STATIC IFX_int_t DTI_FwFeaturesGet(
   IFX_char_t *pFirmware,
   const IFX_uint32_t nFwLength,
   DSL_FirmwareFeatures_t *pFwFeatures)
{
   DSL_uint32_t i = 0, nValue = 0;
   IFX_uint32_t str_len = MAX_WHAT_STRING_LEN - 1;
   IFX_uint8_t *ptr = IFX_NULL;
   IFX_uint32_t rd_sz, bin_len;
   IFX_boolean_t bFound = IFX_FALSE, bEnd = IFX_FALSE;
   DSL_char_t pString[MAX_WHAT_STRING_LEN] = {0};
   IFX_int_t nFwApplication = -1;
   IFX_uint8_t nPlatformId = 0;
   DSL_char_t *pWhat = DSL_NULL;

   if((pFirmware == IFX_NULL) || (nFwLength < 4))
   {
      return IFX_ERROR;
   }

   bin_len = nFwLength;
   rd_sz   = bin_len - 4;

   if (bin_len < 4)
   {
      return DSL_ERROR;
   }

   ptr = (IFX_uint8_t*)pFirmware;

   while (bin_len > 4)
   {
      while(rd_sz)
      {
         if(*ptr == '@' && !bFound)
         {
            if (ptr[1] == '(' && ptr[2] == '#' && ptr[3] == ')')
            {
               bFound = IFX_TRUE;
            }
         }

         if (bFound)
         {
            ptr   += 4;
            rd_sz -= 4;

            /* found what string */
            while(str_len && *ptr && (rd_sz))
            {
               if(*ptr != ' ')
               {
                  pString[i++] = *ptr;
                  str_len--;
               }

               ptr++;
               rd_sz--;
            }

            bEnd = IFX_TRUE;
            break;
         }

         ptr++;
         rd_sz--;
         bin_len--;
      }

      if (bEnd)
      {
         break;
      }
   }

   pString[i] = 0;

   if(i)
   {
      DTI_PRN_USR_ERR_NL(DTI_VRX, DTI_PRN_LEVEL_ERR,
         ("Firmware WHAT String: %s"DTI_CRLF, pString));
   }
   else
   {
      DTI_PRN_USR_ERR_NL(DTI_VRX, DTI_PRN_LEVEL_ERR,
         ("Firmware WHAT String not detected"DTI_CRLF));
   }

   pWhat = pString;

   for (i=0; i<6; i++)
   {
      if (*pWhat == '\0')
      {
         break;
      }

      /* expect a hex value */
      nValue = strtoul(pWhat, &pWhat, 16);

      if (i == 0)
      {
         nPlatformId = (IFX_uint8_t) nValue;
      }

      /* step to the next position after a '.' */
      pWhat = strpbrk(pWhat, ".");
      pWhat++;
   }

   pFwFeatures->nPlatformId = nPlatformId;

   if (i == 6)
   {
      nFwApplication = nValue;
   }

   /* Check for Annex A (default) mode*/
   if (nFwApplication == 1)
   {
      pFwFeatures->nFirmwareXdslModes |= DSL_FW_XDSLMODE_ADSL_A;
   }
   else if ((nFwApplication == 0) || (nFwApplication == 2))
   {
      pFwFeatures->nFirmwareXdslModes |= DSL_FW_XDSLMODE_ADSL_B;
   }

   /* Check for the VDSL FW capability */
   if ((nFwApplication == 5) || (nFwApplication == 6))
   {
      pFwFeatures->nFirmwareXdslModes |= DSL_FW_XDSLMODE_VDSL2;
   }

   /* Check for the Vectoring FW capability */
   if (nFwApplication == 7)
   {
      pFwFeatures->nFirmwareXdslModes |= DSL_FW_XDSLMODE_VDSL2_VECTOR;
   }

   return IFX_SUCCESS;
}

/* ============================================================================
   Device Access Functions
   ========================================================================= */

/**
   Set a new debug level.
*/
DTI_STATIC IFX_int_t DTI_VRX_PrintoutLevelSet(
                        IFX_int_t            newDbgLevel)
{
   IFXOS_PRN_USR_LEVEL_SET(DTI_VRX, newDbgLevel);

   return IFX_SUCCESS;
}

/**
   Setup Vrx Module

\param
   pDeviceSystemInfo - points to the system infos
\param
   ppDtiDevCtx       - pointer to return the allocated Vrx Modul struct.

\return
   If success IFX_SUCCESS, the context is returned via the ppDtiDevCtx param
   else IFX_ERROR.

*/
DTI_STATIC IFX_int_t DTI_VRX_ModuleSetup(
                        DTI_DeviceSysInfo_t  *pDeviceSystemInfo,
                        DTI_DeviceCtx_t      **ppDtiDevCtx)
{
   IFX_int_t   i, ctxSize = 0;
   DTI_DeviceCtx_t            *pDtiDevCtx = IFX_NULL;
   DTI_DEV_VrxDriverCtx_t  *pVrxDevCtx = IFX_NULL;
   DTI_PTR_U                  uDtiDevCtx;
   IFX_int_t nDevNum;

   if ( (ppDtiDevCtx == IFX_NULL) || (pDeviceSystemInfo == IFX_NULL))
   {
      DTI_PRN_USR_ERR_NL(DTI_VRX, DTI_PRN_LEVEL_ERR,
         ("ERROR: Vrx Dev Module Setup - NULL ptr args."DTI_CRLF));

      return IFX_ERROR;
   }

   if (*ppDtiDevCtx != IFX_NULL)
   {
      pDtiDevCtx = *ppDtiDevCtx;
      pVrxDevCtx = (DTI_DEV_VrxDriverCtx_t *)pDtiDevCtx->pDevice;
   }

   if (pDtiDevCtx != IFX_NULL)
   {
      pDtiDevCtx = *ppDtiDevCtx;

      DTI_PRN_USR_ERR_NL(DTI_VRX, IFXOS_PRN_LEVEL_WRN,
         ("WARNING: Vrx Dev Module Setup - already done (devs = %d, ports = %d)."DTI_CRLF,
         (pVrxDevCtx != IFX_NULL) ? pVrxDevCtx->numOfDevs : -1, 
         (pVrxDevCtx != IFX_NULL) ? pVrxDevCtx->numOfPorts : -1));

      return IFX_ERROR;
   }

   pDeviceSystemInfo->numOfDevs   = (IFX_int_t)1;
   pDeviceSystemInfo->portsPerDev = (IFX_int_t)1;
   pDeviceSystemInfo->ifPerDev    = (IFX_int_t)1;
   pDeviceSystemInfo->numOfPorts  = pDeviceSystemInfo->numOfDevs * pDeviceSystemInfo->portsPerDev;
   pDeviceSystemInfo->bValid      = IFX_FALSE;
#ifndef DTI_DONT_USE_PROTEXT
   pDeviceSystemInfo->bControlAutoDevMsgSupport = IFX_TRUE;
#else
   pDeviceSystemInfo->bControlAutoDevMsgSupport = IFX_FALSE;
#endif /* !DTI_DONT_USE_PROTEXT */

   /* Calculate the number of devices */
   for (nDevNum = 0; nDevNum <= DTI_DEV_MAX_LINE_NUMBER; nDevNum++)
   {
      IFX_int_t fd = -1;
      fd = DTI_DevOpenByName(nDevNum, DSL_CPE_IFX_LOW_DEV);
      if (fd < 0)
      {
         break;
      }
      DTI_DevCloseByFd(fd);
   }

   if (nDevNum > 0)
   {
      pDeviceSystemInfo->numOfDevs   = (IFX_int_t)nDevNum;
      pDeviceSystemInfo->numOfPorts  = pDeviceSystemInfo->numOfDevs *
         pDeviceSystemInfo->ifPerDev;
      pDeviceSystemInfo->bValid      = IFX_TRUE;
   } 

   DTI_PRN_USR_DBG_NL(DTI_VRX, DTI_PRN_LEVEL_HIGH,
      ("Vrx Dev Module Setup - update driver config (devs = %d, ports per dev = %d, total lines = %d, auto msg support = %d)."
      DTI_CRLF, pDeviceSystemInfo->numOfDevs, pDeviceSystemInfo->portsPerDev, pDeviceSystemInfo->numOfPorts, 
                pDeviceSystemInfo->bControlAutoDevMsgSupport));

   ctxSize = sizeof(DTI_DeviceCtx_t) + sizeof(DTI_DEV_VrxDriverCtx_t) +
             (sizeof(IFX_int_t) * pDeviceSystemInfo->numOfPorts);
   ctxSize += ((pDeviceSystemInfo->bControlAutoDevMsgSupport == IFX_TRUE) ? (DTI_DEV_VRX_DEFAULT_MBOX_SIZE) : 0);

   uDtiDevCtx.pUInt8 = DTI_Malloc((IFX_uint_t)ctxSize);
   if (uDtiDevCtx.pUInt8 == IFX_NULL)
   {
      DTI_PRN_USR_ERR_NL(DTI_VRX, DTI_PRN_LEVEL_ERR,
         ("ERROR: Vrx Dev Module Setup - Dev Struct alloc."DTI_CRLF));

      return IFX_ERROR;
   }
   memset(uDtiDevCtx.pUInt8, 0x0, ctxSize);
   /* set context pointer */
   pDtiDevCtx = (DTI_DeviceCtx_t *)DTI_PTR_CAST_GET_ULONG(uDtiDevCtx);
   if (pDtiDevCtx == IFX_NULL)
   {
      DTI_Free(uDtiDevCtx.pUInt8);

      DTI_PRN_USR_ERR_NL(DTI_VRX, DTI_PRN_LEVEL_ERR,
         ("ERROR: Vrx Dev Module Setup - Dev Struct miss-aligned."DTI_CRLF));

      return IFX_ERROR;
   }

   /* set pDevice pointer*/
   uDtiDevCtx.pUInt8  += sizeof(DTI_DeviceCtx_t);
   pVrxDevCtx = (DTI_DEV_VrxDriverCtx_t*)DTI_PTR_CAST_GET_UINT32(uDtiDevCtx);

   if (pVrxDevCtx == IFX_NULL)
   {
      DTI_Free(pDtiDevCtx);

      DTI_PRN_USR_ERR_NL(DTI_VRX, DTI_PRN_LEVEL_ERR,
         ("ERROR: Vrx Dev Module Setup - pDevice miss-aligned."DTI_CRLF));

      return IFX_ERROR;
   }

   /* set devFd pointer */
   uDtiDevCtx.pUInt8  += sizeof(DTI_DEV_VrxDriverCtx_t);
   pVrxDevCtx->pDevFds = (IFX_int_t *)DTI_PTR_CAST_GET_UINT32(uDtiDevCtx);

   if (pVrxDevCtx->pDevFds == IFX_NULL)
   {
      DTI_Free(pDtiDevCtx);

      DTI_PRN_USR_ERR_NL(DTI_VRX, DTI_PRN_LEVEL_ERR,
         ("ERROR: Vrx Dev Module Setup - Vnx devFds miss-aligned."DTI_CRLF));

      return IFX_ERROR;
   }

   /* set auto msg buffer pointer */
   if (pDeviceSystemInfo->bControlAutoDevMsgSupport == IFX_TRUE)
   {
      uDtiDevCtx.pUInt8  += (sizeof(IFX_int_t) * pDeviceSystemInfo->numOfPorts);
      pVrxDevCtx->pAutoMsgBuf = (IFX_ulong_t *)DTI_PTR_CAST_GET_ULONG(uDtiDevCtx);

      if (pVrxDevCtx->pAutoMsgBuf == IFX_NULL)
      {
         DTI_Free(pDtiDevCtx);

         DTI_PRN_USR_ERR_NL(DTI_VRX, DTI_PRN_LEVEL_ERR,
            ("ERROR: Vrx Dev Module Setup - Vnx Auto MSg Buffer miss-aligned."DTI_CRLF));

         return IFX_ERROR;
      }
      pVrxDevCtx->autoMsgBuf_byte = DTI_DEV_VRX_DEFAULT_MBOX_SIZE;
   }
   else
   {
      pVrxDevCtx->pAutoMsgBuf     = IFX_NULL;
      pVrxDevCtx->autoMsgBuf_byte = 0;
   }

   for (i = 0; i < pDeviceSystemInfo->numOfPorts; i++)
   {
      pVrxDevCtx->pDevFds[i] = -1;
   }

   pVrxDevCtx->numOfRegAccess     = DTI_DEV_VRX_MAX_REGISTER_ACCESS_NUM;
   pVrxDevCtx->numOfDebugAccess   = DTI_DEV_VRX_MAX_DEBUG_ACCESS_NUM;

   pDtiDevCtx->bAutoDevMsgActive  = IFX_FALSE;

   pVrxDevCtx->numOfDevs          = pDeviceSystemInfo->numOfDevs;
   pVrxDevCtx->portsPerDev        = pDeviceSystemInfo->portsPerDev;
   pVrxDevCtx->numOfPorts         = pDeviceSystemInfo->numOfPorts;
#ifndef DTI_DONT_USE_PROTEXT
   pVrxDevCtx->bAutoDevMsgSupport = pDeviceSystemInfo->bControlAutoDevMsgSupport;
#else
   pVrxDevCtx->bAutoDevMsgSupport = IFX_FALSE;
#endif /* !DTI_DONT_USE_PROTEXT */
   pDtiDevCtx->pDevice = (IFX_void_t *)pVrxDevCtx;
   *ppDtiDevCtx = pDtiDevCtx;

   return IFX_SUCCESS;
}


/**
   Delete a Vrx Module.
   - close all open devices
   - free memory

\param
   ppDtiDevCtx - contains the modul context.

\return
   If success IFX_SUCCESS, the context is freed and the ptr is set to IFX_NULL.
   else IFX_ERROR.

*/
DTI_STATIC IFX_int_t DTI_VRX_ModuleDelete(
                        DTI_DeviceSysInfo_t  *pDeviceSystemInfo,
                        DTI_DeviceCtx_t      **ppDtiDevCtx)
{
   IFX_int_t   i;
   DTI_DeviceCtx_t         *pDtiDevCtx = IFX_NULL;
   DTI_DEV_VrxDriverCtx_t *pVrxDevCtx = IFX_NULL;

   if (ppDtiDevCtx == IFX_NULL)
   {
      DTI_PRN_USR_ERR_NL(DTI_VRX, DTI_PRN_LEVEL_ERR,
         ("ERROR: Vrx Dev Module Delete - NULL ptr args."DTI_CRLF));

      return IFX_ERROR;
   }

   if (*ppDtiDevCtx == IFX_NULL)
   {
      DTI_PRN_USR_ERR_NL(DTI_VRX, DTI_PRN_LEVEL_ERR,
         ("ERROR: Vrx Dev Module Delete - NULL ptr modul ctx."DTI_CRLF));

      return IFX_ERROR;
   }

   pDtiDevCtx = *ppDtiDevCtx;
   *ppDtiDevCtx = IFX_NULL;

   pVrxDevCtx = (DTI_DEV_VrxDriverCtx_t *)pDtiDevCtx->pDevice;
   for (i = 0; i < pVrxDevCtx->numOfPorts; i++)
   {
      if (pVrxDevCtx->pDevFds[i] >= 0)
      {
         (void)DTI_DevCloseByFd(pVrxDevCtx->pDevFds[i]);
         pVrxDevCtx->pDevFds[i] = -1;
      }
   }

   DTI_Free(pDtiDevCtx);

   return IFX_SUCCESS;
}

/**
   Write the Sytem Info of the current DTI Agent instance to the given buffer.

\param
   pDeviceSystemInfo     - points to Sytem Info struct of the current agent instance.
\param
   pSysInfoBuffer    - points to the Sytem Info char buffer.
\param
   bufferSize        - buffer size.

\return
   Number of written bytes.
*/
DTI_STATIC IFX_int_t DTI_VRX_SystemInfoWrite(
                        DTI_DeviceSysInfo_t  *pDeviceSystemInfo,
                        IFX_char_t           *pSysInfoBuffer,
                        IFX_int_t            bufferSize)
{
   IFX_int_t writtenLen = 0;
   const IFX_char_t *pBoardName = DTI_DEV_VRX_BOARD_NAME;

   (void)DTI_snprintf(&pSysInfoBuffer[writtenLen], bufferSize - writtenLen,
      "VendorName=%s", DTI_VENDOR_NAME_STR);
   writtenLen = (IFX_int_t)DTI_StrLen(&pSysInfoBuffer[writtenLen]) + 1;

   (void)DTI_snprintf(&pSysInfoBuffer[writtenLen], bufferSize - writtenLen,
      "BoardName=%s", pBoardName);
   writtenLen += (IFX_int_t)DTI_StrLen(&pSysInfoBuffer[writtenLen]) + 1;

   (void)DTI_snprintf(&pSysInfoBuffer[writtenLen], bufferSize - writtenLen,
      "BoardRevision=0.0");
   writtenLen += (IFX_int_t)DTI_StrLen(&pSysInfoBuffer[writtenLen]) + 1;

   if (pDeviceSystemInfo->bValid == IFX_TRUE)
   {
      (void)DTI_snprintf(&pSysInfoBuffer[writtenLen], bufferSize - writtenLen,
         "NumOfDevices=%d", pDeviceSystemInfo->numOfDevs);
      writtenLen += (IFX_int_t)DTI_StrLen(&pSysInfoBuffer[writtenLen]) + 1;

      (void)DTI_snprintf(&pSysInfoBuffer[writtenLen], bufferSize - writtenLen,
         "MaxChannel=%d", pDeviceSystemInfo->numOfPorts);
      writtenLen += (IFX_int_t)DTI_StrLen(&pSysInfoBuffer[writtenLen]) + 1;
   }
   else
   {
      (void)DTI_snprintf(&pSysInfoBuffer[writtenLen], bufferSize - writtenLen,
         "NumOfDevices=<na>");
      writtenLen += (IFX_int_t)DTI_StrLen(&pSysInfoBuffer[writtenLen]) + 1;

      (void)DTI_snprintf(&pSysInfoBuffer[writtenLen], bufferSize - writtenLen,
         "MaxChannel=<na>");
      writtenLen += (IFX_int_t)DTI_StrLen(&pSysInfoBuffer[writtenLen]) + 1;
   }

   return writtenLen;
}

/**
   Do an device reset on the selected device.

\param
   pDtiDevCtx     - points to the VRX Device context.
\param
   pInDevReset    - points to the DTI Host2Dev Reset struct.
\param
   pOutDevReset   - points to the DTI Dev2Host Reset struct.
\param
   rstMaskSize_32 - number of mask elements (32 Bit).
\param
   pPacketError   - return ptr, return the DTI Packet Error.

\return
   IFX_SUCCESS and returns the DTI Packet Error code.
   IFX_ERROR   currently no (only in case of non-DTI related errors).
*/
DTI_STATIC IFX_int_t DTI_VRX_Reset(
                        DTI_DeviceCtx_t         *pDtiDevCtx,
                        DTI_H2D_DeviceReset_t   *pInDevReset,
                        DTI_D2H_DeviceReset_t   *pOutDevReset,
                        IFX_int_t               rstMaskSize_32,
                        DTI_PacketError_t       *pPacketError)
{
   DTI_PRN_USR_ERR_NL(DTI_VRX, DTI_PRN_LEVEL_ERR,
      ("ERROR: Vrx Device Reset - not supported."DTI_CRLF));

   *pPacketError = DTI_eErrUnknown;
   return IFX_SUCCESS;
}

/**
   Do a FW download on the selected devices

\param
   pDtiDevCtx        - points to the VRX Device context.
\param
   pDtiProtServerCtx - points to the DTI protocol server context.
\param
   pInDevDownload    - points to the DTI Host2Dev Download struct.
\param
   pOutDevDownload   - points to the DTI Dev2Host Download struct.
\param
   pPacketError      - return ptr, return the DTI Packet Error.

\return
   IFX_SUCCESS and returns the DTI Packet Error code.
   IFX_ERROR   currently no (only in case of non-DTI related errors).
*/
DTI_STATIC IFX_int_t DTI_VRX_Download(
                        DTI_DeviceCtx_t            *pDtiDevCtx,
                        DTI_ProtocolServerCtx_t    *pDtiProtServerCtx,
                        DTI_H2D_DeviceDownload_t   *pInDevDownload,
                        DTI_D2H_DeviceDownload_t   *pOutDevDownload,
                        DTI_PacketError_t          *pPacketError)
{
#ifndef DTI_DONT_USE_PROTEXT
   IFX_int_t   fd = -1;
   IFX_int_t   nDevNum, nDevMax = -1, nDevMin = -1, nDevCount = 0;
   DTI_PacketError_t lockPacketError = DTI_eNoError;
   DTI_DEV_VrxDriverCtx_t *pVrxDevCtx = (DTI_DEV_VrxDriverCtx_t *)pDtiDevCtx->pDevice;
   DTI_ImageControl_t *imgCtrl = IFX_NULL;
   IOCTL_MEI_reset_t         MEI_ResetArgs;
   IOCTL_MEI_fwDownLoad_t    MEI_FwDl;
   IOCTL_MEI_FwModeCtrlSet_t MEI_FwMode;
   DSL_FirmwareFeatures_t    FwFeatures = {0};

   pOutDevDownload->errorMask = pInDevDownload->loadMask;

   /*
      Check args
   */
   for (nDevNum = 0; nDevNum < 32; nDevNum++)
   {
      if (pInDevDownload->loadMask & (0x1 << nDevNum))
      {
         if (nDevMin == -1)
         {
            nDevMin = nDevNum;
         }
         nDevMax = nDevNum;
         nDevCount++;
      }
   }

   if ((nDevMin > pVrxDevCtx->numOfDevs) ||
       (nDevMax > pVrxDevCtx->numOfDevs) || (nDevCount == 0))
   {
      DTI_PRN_USR_ERR_NL(DTI_VRX, DTI_PRN_LEVEL_ERR,
         ("ERROR: MEI Device Download - mask 0x%08X, max devs %d (nDevMin=%d, "
         "nDevMax=%d, nDevCount=%d), truncate."DTI_CRLF, 
         pInDevDownload->loadMask, pVrxDevCtx->numOfDevs, nDevMin,
         nDevMax, nDevCount));
   }

   /*
      Get Image from the DTI Agent Context.
   */
   if (pDtiProtServerCtx->pCbFctImageCntrlLock(
                              pDtiProtServerCtx->pCbCtxAgent,
                              (IFX_int_t)pInDevDownload->imageNum,
                              1, &imgCtrl, pPacketError) != IFX_SUCCESS)
   {
      DTI_PRN_USR_ERR_NL(DTI_VRX, DTI_PRN_LEVEL_ERR,
         ("ERROR: MEI Device Download - image %d get."DTI_CRLF,
         pInDevDownload->imageNum));

      *pPacketError = DTI_eErrPortOperation;
      return IFX_ERROR;
   }

   if (imgCtrl)
   {
      if ((imgCtrl->pData == IFX_NULL) || (imgCtrl->imageSize == 0))
      {
         DTI_PRN_USR_ERR_NL(DTI_VRX, DTI_PRN_LEVEL_ERR,
            ("ERROR: MEI Device Download - image %d empty."DTI_CRLF,
               pInDevDownload->imageNum));

         *pPacketError = DTI_eErrPortOperation;
         goto DTI_AGENT_DOWNLOAD_EXIT;
      }
   }
   else
   {
      DTI_PRN_USR_ERR_NL(DTI_VRX, DTI_PRN_LEVEL_ERR,
         ("ERROR: MEI Device Download - failed to get image."DTI_CRLF));
      goto DTI_AGENT_DOWNLOAD_EXIT;
   }
   nDevCount=0;
   for (nDevNum=0; nDevNum < 32; nDevNum++)
   {
      if (nDevCount >= pVrxDevCtx->numOfDevs)
      {
         DTI_PRN_USR_DBG_NL(DTI_VRX, DTI_PRN_LEVEL_LOW,
            ("MEI Device Download - break, nDevCount(%d) >= numOfDevs(%d). "
            DTI_CRLF, nDevCount, pVrxDevCtx->numOfDevs));
         break;
      }

      if (!(pInDevDownload->loadMask & (0x1 << nDevNum)))
      {
         DTI_PRN_USR_DBG_NL(DTI_VRX, DTI_PRN_LEVEL_LOW,
            ("MEI Device Download - skip FW download on device %d."DTI_CRLF,
            nDevNum));
         continue;
      }
      nDevCount++;

      /*
         open device
      */
      fd = DTI_DevOpenByName(nDevNum, DSL_CPE_IFX_LOW_DEV);
      if (fd < 0)
      {
         DTI_PRN_USR_ERR_NL(DTI_VRX, DTI_PRN_LEVEL_ERR,
            ("ERROR: Mei Device Download - open control device %d."DTI_CRLF,
               nDevNum));

            *pPacketError = DTI_eErrPortOperation;
            goto DTI_AGENT_DOWNLOAD_EXIT;
      }

      /*
         reset MEI driver
      */
      DTI_MemSet(&MEI_ResetArgs, 0x00, sizeof(IOCTL_MEI_reset_t));

      MEI_ResetArgs.rstMode = (IOCTL_MEI_resetMode_e)0;
      MEI_ResetArgs.rstSelMask = (unsigned int)0x1F;

      if (IFXOS_DeviceControl(fd, FIO_MEI_RESET, (IFX_int_t)&MEI_ResetArgs) < 0)
      {
         DTI_PRN_USR_ERR_NL(DTI_VRX, DTI_PRN_LEVEL_ERR,
            ("ERROR: MEI Device Download - ioctl MEI_RESET "
            "retCode = %d."DTI_CRLF, MEI_ResetArgs.ictl.retCode));

         *pPacketError = DTI_eErrPortOperation;
         goto DTI_AGENT_DOWNLOAD_EXIT;
      }

      /*
         configure FW mode
      */
      DTI_MemSet(&MEI_FwMode, 0x00, sizeof(IOCTL_MEI_FwModeCtrlSet_t));

      DTI_FwFeaturesGet((DSL_char_t*)imgCtrl->pData,
                        (IFX_uint32_t)imgCtrl->imageSize,
                        &FwFeatures);

      MEI_FwMode.bMultiLineModeLock                  = IFX_TRUE;
      MEI_FwMode.bXdslModeLock                       = IFX_TRUE;
      MEI_FwMode.eXdslModeCurrent                    = 0;/* VDSL only */
      MEI_FwMode.firmwareFeatures.nPlatformId        = FwFeatures.nPlatformId;
      MEI_FwMode.firmwareFeatures.eFirmwareXdslModes =
                                                FwFeatures.nFirmwareXdslModes;

      if (IFXOS_DeviceControl(fd, FIO_MEI_FW_MODE_CTRL_SET,
          (IFX_int_t)&MEI_FwMode) < 0)
      {
         DTI_PRN_USR_ERR_NL(DTI_VRX, DTI_PRN_LEVEL_ERR,
            ("ERROR: MEI Device Download - ioctl FIO_MEI_FW_MODE_CTRL_SET "
            " %d, retCode = %d."DTI_CRLF, MEI_FwMode.ictl.retCode));

         *pPacketError = DTI_eErrPortOperation;
         goto DTI_AGENT_DOWNLOAD_EXIT;
      }

      /*
         do FW download
      */
      DTI_PRN_USR_DBG_NL(DTI_VRX, DTI_PRN_LEVEL_LOW,
         ("MEI Device Download - start FW download on device %d."DTI_CRLF,
         nDevNum));

      DTI_MemSet(&MEI_FwDl, 0x00, sizeof(MEI_FwDl));
      MEI_FwDl.pFwImage = (unsigned char *)imgCtrl->pData;
      MEI_FwDl.size_byte = (IFX_uint32_t)imgCtrl->imageSize;

      if (IFXOS_DeviceControl(fd, FIO_MEI_FW_DL, (IFX_int_t)&MEI_FwDl) < 0)
      {
         DTI_PRN_USR_ERR_NL(DTI_VRX, DTI_PRN_LEVEL_ERR,
            ("ERROR: MEI Device Download - ioctl FW download."DTI_CRLF));

         *pPacketError = DTI_eErrPortOperation;
         goto DTI_AGENT_DOWNLOAD_EXIT;
      }
      else
      {
         DTI_PRN_USR_DBG_NL(DTI_VRX, DTI_PRN_LEVEL_LOW,
            ("MEI Device Download - FW download on device %d successful ."
            DTI_CRLF, nDevNum));
      }

      if (fd != -1)
      {
         (void)IFXOS_DeviceClose(fd);
         fd = -1;
      }
   }

DTI_AGENT_DOWNLOAD_EXIT:

   if (fd != -1)
   {
      (void)DTI_DevCloseByFd(fd);
   }

   if (imgCtrl != IFX_NULL)
   {
      (void)pDtiProtServerCtx->pCbFctImageCntrlLock(
                                 pDtiProtServerCtx->pCbCtxAgent,
                                 (IFX_int_t)pInDevDownload->imageNum,
                                 0, IFX_NULL, &lockPacketError);

      if (*pPacketError == DTI_eNoError)
      {
         /* return lock error only if no previous error occured */
         *pPacketError = lockPacketError;
      }
   }

   if (*pPacketError == DTI_eNoError)
   {
      pOutDevDownload->errorMask = 0;
   }

   return (*pPacketError == DTI_eNoError) ? IFX_SUCCESS : IFX_ERROR;
#else
   DTI_PRN_USR_ERR_NL(DTI_VRX, DTI_PRN_LEVEL_ERR,
      ("ERROR: Vrx Device Download - not supported."DTI_CRLF));

   *pPacketError = DTI_eErrUnknown;
   return IFX_SUCCESS;
#endif /* !DTI_DONT_USE_PROTEXT */ 
}


/**
   Open a given line device.

\param
   pDtiDevCtx     - points to the VRX Device context.
\param
   lineNum        - line number.
\param
   pPacketError   - return ptr, return the DTI Packet Error.

\return
   IFX_SUCCESS and returns the DTI Packet Error code.
   IFX_ERROR   currently no (only in case of non-DTI related errors).
*/
DTI_STATIC IFX_int_t DTI_VRX_DeviceOpen(
                        DTI_DeviceCtx_t      *pDtiDevCtx,
                        IFX_int_t            lineNum,
                        DTI_PacketError_t    *pPacketError)
{
   DTI_DEV_VrxDriverCtx_t  *pVrxDevCtx = (DTI_DEV_VrxDriverCtx_t *)pDtiDevCtx->pDevice;

   *pPacketError = DTI_eNoError;

   if (lineNum > DTI_DEV_MAX_LINE_NUMBER)
   {
      DTI_PRN_USR_ERR_NL(DTI_VRX, DTI_PRN_LEVEL_ERR,
         ("ERROR: Vrx Device Open - line number %d is different from 0."DTI_CRLF,
          lineNum));
          
      *pPacketError = DTI_eErrPortOutOfRange;          

      return IFX_ERROR;
   }

   return DTI_DevOpen(pVrxDevCtx, pPacketError, lineNum);
}


/**
   Close a given line device.

\param
   pDtiDevCtx     - points to the VRX Device context.
\param
   lineNum        - line number.
\param
   pPacketError   - return ptr, return the DTI Packet Error.

\return
   IFX_SUCCESS and returns the DTI Packet Error code.
   IFX_ERROR   currently no (only in case of non-DTI related errors).
*/
DTI_STATIC IFX_int_t DTI_VRX_DeviceClose(
                        DTI_DeviceCtx_t      *pDtiDevCtx,
                        IFX_int_t            lineNum,
                        DTI_PacketError_t    *pPacketError)
{
   DTI_DEV_VrxDriverCtx_t  *pVrxDevCtx = (DTI_DEV_VrxDriverCtx_t *)pDtiDevCtx->pDevice;

   *pPacketError = DTI_eNoError;

   if (lineNum > DTI_DEV_MAX_LINE_NUMBER)
   {
      DTI_PRN_USR_ERR_NL(DTI_VRX, DTI_PRN_LEVEL_ERR,
         ("ERROR: Vrx Device Close - line number %d is different from 0."DTI_CRLF,
          lineNum));
          
      *pPacketError = DTI_eErrPortOutOfRange;

      return IFX_ERROR;
   }

   return DTI_DevClose(pVrxDevCtx, pPacketError, lineNum);
}

/**
   Get / release the device lock.

\param
   pDtiDevCtx     - points to the VRX Device context.
\param
   pInLock        - points to the DTI Host2Dev Lock struct.
\param
   pOutLock       - points to the DTI Dev2Host Lock struct.
\param
   lineNum        - line number.
\param
   pPacketError   - return ptr, return the DTI Packet Error.

\return
   IFX_SUCCESS and returns the DTI Packet Error code.
   IFX_ERROR   currently no (only in case of non-DTI related errors).
*/
DTI_STATIC IFX_int_t DTI_VRX_RegisterLock(
                        DTI_DeviceCtx_t      *pDtiDevCtx,
                        DTI_H2D_DeviceLock_t *pInLock,
                        DTI_D2H_DeviceLock_t *pOutLock,
                        IFX_int_t            lineNum,
                        DTI_PacketError_t    *pPacketError)
{
   DTI_PRN_USR_ERR_NL(DTI_VRX, DTI_PRN_LEVEL_ERR,
      ("ERROR: Vrx Lock - not supported."DTI_CRLF));

   pOutLock->lock = 0;
   *pPacketError  = DTI_eErrUnknown;

   return IFX_SUCCESS;
}

/**
   Get a device register (MEI interface).

\param
   pDtiDevCtx     - points to the VRX Device context.
\param
   pInRegGet      - points to the DTI Host2Dev RegisterGet struct.
\param
   pOutRegGet     - points to the DTI Dev2Host RegisterGet struct.
\param
   lineNum        - line number.
\param
   pOutPaylSize_byte - return ptr, return the size of the read registers.
\param
   pPacketError   - return ptr, return the DTI Packet Error.

\return
   IFX_SUCCESS and returns the DTI Packet Error code.
   IFX_ERROR   currently no (only in case of non-DTI related errors).
*/
DTI_STATIC IFX_int_t DTI_VRX_RegisterGet(
                        DTI_DeviceCtx_t         *pDtiDevCtx,
                        DTI_H2D_RegisterGet_t   *pInRegGet,
                        DTI_D2H_RegisterGet_t   *pOutRegGet,
                        IFX_int_t               lineNum,
                        IFX_uint32_t            *pOutPaylSize_byte,
                        DTI_PacketError_t       *pPacketError)
{
   IFX_int_t   count, regCount;
   DTI_DEV_VrxDriverCtx_t     *pVrxDevCtx = (DTI_DEV_VrxDriverCtx_t *)pDtiDevCtx->pDevice;
   IOCTL_MEI_regInOut_t   regIO;

   regCount = (IFX_int_t)pInRegGet->count;

   *pOutPaylSize_byte = 0;

   if (regCount > pVrxDevCtx->numOfRegAccess)
   {
      DTI_PRN_USR_ERR_NL(DTI_VRX, DTI_PRN_LEVEL_ERR,
         ("ERROR: Vrx RegisterGet - line %d, count %d > max %d."DTI_CRLF,
          lineNum, regCount, pVrxDevCtx->numOfRegAccess));

      *pPacketError      = DTI_eErrConfiguration;

      return IFX_SUCCESS;
   }

   if (DTI_DevOpen(pVrxDevCtx, pPacketError, lineNum) != IFX_SUCCESS)
   {
      DTI_PRN_USR_ERR_NL(DTI_VRX, DTI_PRN_LEVEL_ERR,
         ("ERROR: Vrx RegisterGet - device open error." DTI_CRLF));

      return IFX_SUCCESS;
   }

   for (count = 0; count < regCount; count++)
   {
      DTI_MemSet(&regIO, 0x00, sizeof(regIO));
      regIO.addr = (unsigned int)pInRegGet->address;

      if ( (DSL_CPE_Ioctl(
                  pVrxDevCtx->pDevFds[lineNum], FIO_MEI_REG_GET, (DSL_int_t)&regIO)) < 0 )
      {
         DTI_PRN_USR_ERR_NL(DTI_VRX, DTI_PRN_LEVEL_ERR,
            ("ERROR: Vrx RegisterGet - ioctl error."DTI_CRLF));

         break;
      }
      else
      {
         pOutRegGet->data[count] = (IFX_uint32_t)regIO.value;
      }
   }

   if (count < regCount)
   {
      *pOutPaylSize_byte = 0;
      *pPacketError      = DTI_eErrPortOperation;
   }
   else
   {
      *pOutPaylSize_byte = (IFX_uint32_t)(regCount * sizeof(IFX_uint32_t));
      *pPacketError      = DTI_eNoError;
   }

   return IFX_SUCCESS;
}


/**
   Get a device register (MEI interface).

\param
   pDtiDevCtx        - points to the VRX Device context.
\param
   pInRegSet         - points to the DTI Host2Dev RegisterSet struct.
\param
   inRegSetSize_Byte - size of the given IN data [byte].
\param
   lineNum           - line number.
\param
   pPacketError      - return ptr, return the DTI Packet Error.

\return
   IFX_SUCCESS DTI Packet Error code is "no error".
   IFX_ERROR   if the register read operation fails.
*/
DTI_STATIC IFX_int_t DTI_VRX_RegisterSet(
                        DTI_DeviceCtx_t         *pDtiDevCtx,
                        DTI_H2D_RegisterSet_t   *pInRegSet,
                        IFX_int_t               inRegSetSize_Byte,
                        IFX_int_t               lineNum,
                        DTI_PacketError_t       *pPacketError)
{
   IFX_int_t   count, regCount;
   DTI_DEV_VrxDriverCtx_t     *pVrxDevCtx = (DTI_DEV_VrxDriverCtx_t *)pDtiDevCtx->pDevice;
   IOCTL_MEI_regInOut_t   regIO;

   regCount = (inRegSetSize_Byte - sizeof(pInRegSet->address)) / sizeof(IFX_uint32_t);

   if ( (regCount > pVrxDevCtx->numOfRegAccess) || (regCount <= 0) )
   {
      DTI_PRN_USR_ERR_NL(DTI_VRX, DTI_PRN_LEVEL_ERR,
         ("ERROR: Vrx RegisterSet - line %d, count %d (max %d)."DTI_CRLF,
          lineNum, regCount, pVrxDevCtx->numOfRegAccess));

      *pPacketError = DTI_eErrInvalidPayloadSize;

      return IFX_SUCCESS;
   }

   if (DTI_DevOpen(pVrxDevCtx, pPacketError, lineNum) != IFX_SUCCESS)
   {
      DTI_PRN_USR_ERR_NL(DTI_VRX, DTI_PRN_LEVEL_ERR,
         ("ERROR: Vrx RegisterSet - device open error." DTI_CRLF));

      return IFX_SUCCESS;
   }

   *pPacketError = DTI_eNoError;

   for (count = 0; count < regCount; count++)
   {
      DTI_MemSet(&regIO, 0x00, sizeof(regIO));
      regIO.addr = (unsigned int)pInRegSet->address;
      regIO.value  = (unsigned int)pInRegSet->data[count];

      if ( (DSL_CPE_Ioctl(
                  pVrxDevCtx->pDevFds[lineNum], FIO_MEI_REG_SET, (DSL_int_t)&regIO)) < 0 )
      {
         DTI_PRN_USR_ERR_NL(DTI_VRX, DTI_PRN_LEVEL_ERR,
            ("ERROR: Vrx RegisterSet - ioctl error."DTI_CRLF));

         *pPacketError = DTI_eErrPortOperation;

         break;
      }
   }

   return IFX_SUCCESS;
}


/**
   Set a device configuration.

\param
   pDtiConCtx     - points to the DTI connection setup.
\param
   pDtiDevCtx     - points to the VRX Device context.
\param
   pInCfgSet      - points to the DTI Host2Dev ConfigSet struct.
\param
   pOutCfgSet     - points to the DTI Dev2Host ConfigSet struct.
\param
   lineNum        - line number.
\param
   pPacketError   - return ptr, return the DTI Packet Error.

\return
   IFX_SUCCESS and returns the DTI Packet Error code.
   IFX_ERROR   currently no (only in case of non-DTI related errors).

\remark
   Changes of the autonomous message handling may have influence to the
   connection settings (select wait time)
*/
DTI_STATIC IFX_int_t DTI_VRX_ConfigSet(
                        DTI_DeviceCtx_t            *pDtiDevCtx,
                        DTI_H2D_DeviceConfigSet_t  *pInCfgSet,
                        DTI_D2H_DeviceConfigSet_t  *pOutCfgSet,
                        IFX_int_t                  lineNum,
                        DTI_PacketError_t          *pPacketError)
{
   DTI_DEV_VrxDriverCtx_t  *pVrxDevCtx = (DTI_DEV_VrxDriverCtx_t *)pDtiDevCtx->pDevice;

   memset(pOutCfgSet, 0x00, sizeof(DTI_D2H_DeviceConfigSet_t));
   pOutCfgSet->key = pInCfgSet->key;

   switch (pInCfgSet->key)
   {
      case DTI_eTimeout:
         pOutCfgSet->value = 0;
         *pPacketError = DTI_eErrUnknown;
         break;

      case DTI_eAutonousMessages:
         (void)DTI_configSet_AutoMsg(
                        pVrxDevCtx, pInCfgSet, pOutCfgSet,
                        lineNum, pPacketError);

         /* NFC is enabled - change conncetion settings */
         if (pVrxDevCtx->nfcMaxDevFd)
         {
            pDtiDevCtx->bAutoDevMsgActive = IFX_TRUE;
         }
         else
         {
            pDtiDevCtx->bAutoDevMsgActive = IFX_FALSE;
         }
         break;

      case DTI_eMaxRegAccess:
         if (pInCfgSet->value > DTI_DEV_VRX_MAX_REGISTER_ACCESS_NUM)
         {
            pVrxDevCtx->numOfRegAccess = DTI_DEV_VRX_MAX_REGISTER_ACCESS_NUM;
            *pPacketError = DTI_eErrConfiguration;
         }
         else
         {
            pVrxDevCtx->numOfRegAccess = (IFX_int_t)pInCfgSet->value;
            *pPacketError = DTI_eNoError;
         }
         pOutCfgSet->value = (IFX_uint32_t)pVrxDevCtx->numOfRegAccess;
         break;

      case DTI_eMaxDebugAccess:
         if (pInCfgSet->value > DTI_DEV_VRX_MAX_DEBUG_ACCESS_NUM)
         {
            pVrxDevCtx->numOfDebugAccess = DTI_DEV_VRX_MAX_DEBUG_ACCESS_NUM;
            *pPacketError = DTI_eErrConfiguration;
         }
         else
         {
            pVrxDevCtx->numOfDebugAccess = (IFX_int_t)pInCfgSet->value;
            *pPacketError = DTI_eNoError;
         }
         pOutCfgSet->value = (IFX_uint32_t)pVrxDevCtx->numOfDebugAccess;
         break;

      /* R/O */
      case DTI_eMailboxSize:
      default:
         pOutCfgSet->value = 0;
         *pPacketError = DTI_eErrUnknown;
         break;
   }

   return IFX_SUCCESS;
}


/**
   Get the device configuration of the given line.

\param
   pDtiDevCtx     - points to the VRX Device context.
\param
   pInCfgSet      - points to the DTI Host2Dev ConfigSet struct.
\param
   pOutCfgSet     - points to the DTI Dev2Host ConfigSet struct.
\param
   lineNum        - line number.
\param
   pPacketError   - return ptr, return the DTI Packet Error.

\return
   IFX_SUCCESS and returns the DTI Packet Error code.
   IFX_ERROR   currently no (only in case of non-DTI related errors).
*/
DTI_STATIC IFX_int_t DTI_VRX_ConfigGet(
                        DTI_DeviceCtx_t            *pDtiDevCtx,
                        DTI_H2D_DeviceConfigGet_t  *pInCfgGet,
                        DTI_D2H_DeviceConfigGet_t  *pOutCfgGet,
                        IFX_int_t                  lineNum,
                        DTI_PacketError_t          *pPacketError)
{
   DTI_DEV_VrxDriverCtx_t  *pVrxDevCtx = (DTI_DEV_VrxDriverCtx_t *)pDtiDevCtx->pDevice;

   memset(pOutCfgGet, 0x00, sizeof(DTI_D2H_DeviceConfigGet_t));
   pOutCfgGet->key = pInCfgGet->key;

   switch (pInCfgGet->key)
   {
      case DTI_eTimeout:
         *pPacketError = DTI_eErrUnknown;
         break;

      case DTI_eAutonousMessages:
         pOutCfgGet->value = (IFX_uint32_t)DTI_configGet_AutoMsg(
                                                pVrxDevCtx, lineNum, pPacketError);
         break;

      case DTI_eMailboxSize:
         pOutCfgGet->value = (IFX_uint32_t)DTI_configGet_MbSize(
                                                pVrxDevCtx, lineNum, pPacketError);
         break;

      case DTI_eMaxRegAccess:
         pOutCfgGet->value = (IFX_uint32_t)pVrxDevCtx->numOfRegAccess;
         *pPacketError = DTI_eNoError;
         break;

      case DTI_eMaxDebugAccess:
         pOutCfgGet->value = (IFX_uint32_t)pVrxDevCtx->numOfDebugAccess;
         *pPacketError = DTI_eNoError;
         break;

      default:
         *pPacketError = DTI_eErrUnknown;
         break;
   }

   return IFX_SUCCESS;
}


/**
   Send a 16 Bit message to the device and wait for the responce.

\param
   pDtiDevCtx        - points to the VRX Device context.
\param
   pInMsg16Send      - points to the DTI 16 bit message send struct.
\param
   pOutMsg16Send     - points to the DTI 16 bit message responce struct.
\param
   lineNum           - line number.
\param
   inPaylSize_byte   - payload size of the DTI packet [byte].
\param
   pOutPaylSize_byte - return ptr, return the OUT payload size [byte].
\param
   pPacketError      - return ptr, return the DTI Packet Error.

\return
   IFX_SUCCESS and returns the DTI Packet Error code.
   IFX_ERROR   currently no (only in case of non-DTI related errors).
*/
DTI_STATIC IFX_int_t DTI_VRX_Message16Send(
                        DTI_DeviceCtx_t      *pDtiDevCtx,
                        DTI_H2D_Message16_u  *pInMsg16Send,
                        DTI_D2H_Message16_u  *pOutMsg16Send,
                        IFX_int_t            lineNum,
                        IFX_int_t            inPaylSize_byte,
                        IFX_int_t            *pOutPaylSize_byte,
                        DTI_PacketError_t    *pPacketError)
{
   IOCTL_MEI_messageSend_t messageSend;

   DTI_DEV_VrxDriverCtx_t  *pVrxDevCtx = (DTI_DEV_VrxDriverCtx_t *)pDtiDevCtx->pDevice;

   if (DTI_DevOpen(pVrxDevCtx, pPacketError, lineNum) != IFX_SUCCESS)
   {
      DTI_PRN_USR_ERR_NL(DTI_VRX, DTI_PRN_LEVEL_ERR,
         ("ERROR: Vrx msg16 Send - device open error" DTI_CRLF));
         
      *pOutPaylSize_byte = 0;

      return IFX_SUCCESS;
   }

   memset(&messageSend, 0x00, sizeof(IOCTL_MEI_messageSend_t));

   /* setup message send */
   messageSend.write_msg.msgId         = (unsigned short)pInMsg16Send->raw.data[VRX_DTI_MSG_IDX_MSDID];
   messageSend.write_msg.paylSize_byte = (unsigned short)inPaylSize_byte - sizeof(pInMsg16Send->raw.data[VRX_DTI_MSG_IDX_MSDID]);
   messageSend.write_msg.pPayload      = (unsigned char*)&pInMsg16Send->raw.data[VRX_DTI_MSG_IDX_INDEX]; 
   
   messageSend.ack_msg.paylSize_byte = CMV_USED_PAYLOAD_8BIT_SIZE;
   messageSend.ack_msg.pPayload      = (unsigned char *)&pOutMsg16Send->raw.data[VRX_DTI_MSG_IDX_INDEX];    

   if ( (DSL_CPE_Ioctl(pVrxDevCtx->pDevFds[lineNum], FIO_MEI_MBOX_MSG_SEND, (DSL_int_t)&messageSend)) < 0 )
   {
      /* in case of error loop back input message */
      pOutMsg16Send->raw.data[VRX_DTI_MSG_IDX_MSDID] = pInMsg16Send->raw.data[VRX_DTI_MSG_IDX_MSDID];
      pOutMsg16Send->raw.data[VRX_DTI_MSG_IDX_INDEX] = pInMsg16Send->raw.data[VRX_DTI_MSG_IDX_INDEX];
      pOutMsg16Send->raw.data[VRX_DTI_MSG_IDX_LENGTH] = pInMsg16Send->raw.data[VRX_DTI_MSG_IDX_LENGTH];
      *pOutPaylSize_byte = (sizeof(pOutMsg16Send->raw.data[VRX_DTI_MSG_IDX_MSDID]) * 3) + sizeof(pOutMsg16Send->raw.sendResult);
      pOutMsg16Send->raw.sendResult = IFX_ERROR;
      *pPacketError      = DTI_eErrPortOperation;
   }
   else
   {
      pOutMsg16Send->raw.data[VRX_DTI_MSG_IDX_MSDID] = messageSend.ack_msg.msgId;
      if (messageSend.ack_msg.paylSize_byte & 0x1)
      {
         /*in case of odd message add 1 byte */
         messageSend.ack_msg.paylSize_byte += 1;
      }
      *pOutPaylSize_byte = (IFX_int_t)(messageSend.ack_msg.paylSize_byte + sizeof(pOutMsg16Send->raw.data[VRX_DTI_MSG_IDX_MSDID]) +
                            sizeof(pOutMsg16Send->raw.sendResult));
      pOutMsg16Send->raw.sendResult = IFX_SUCCESS;
      *pPacketError = DTI_eNoError;
   }
   
   return IFX_SUCCESS;
}


/**
   Send a 32 Bit message to the device and wait for the responce.

\param
   pDtiDevCtx        - points to the VRX Device context.
\param
   pInMsg32Send      - points to the DTI 32 bit message send struct.
\param
   pOutMsg32Send     - points to the DTI 32 bit message responce struct.
\param
   lineNum           - line number.
\param
   inPaylSize_byte   - payload size of the DTI packet [byte].
\param
   pOutPaylSize_byte - return ptr, return the OUT payload size.
\param
   pPacketError      - return ptr, return the DTI Packet Error.

\return
   IFX_SUCCESS and returns the DTI Packet Error code.
   IFX_ERROR   currently no (only in case of non-DTI related errors).
*/
DTI_STATIC IFX_int_t DTI_VRX_Message32Send(
                        DTI_DeviceCtx_t      *pDtiDevCtx,
                        DTI_H2D_Message32_u  *pInMsg32Send,
                        DTI_D2H_Message32_u  *pOutMsg32Send,
                        IFX_int_t            lineNum,
                        IFX_int_t            inPaylSize_byte,
                        IFX_int_t            *pOutPaylSize_byte,
                        DTI_PacketError_t    *pPacketError)
{
   IOCTL_MEI_messageSend_t messageSend;

   DTI_DEV_VrxDriverCtx_t  *pVrxDevCtx = (DTI_DEV_VrxDriverCtx_t *)pDtiDevCtx->pDevice;

   if (DTI_DevOpen(pVrxDevCtx, pPacketError, lineNum) != IFX_SUCCESS)
   {
      DTI_PRN_USR_ERR_NL(DTI_VRX, DTI_PRN_LEVEL_ERR,
         ("ERROR: Vrx msg32 Send - line %d %s."DTI_CRLF,
          lineNum, (*pPacketError == DTI_eErrPortOutOfRange) ? "range error" : "open error"));

      *pOutPaylSize_byte = 0;

      return IFX_SUCCESS;
   }

   memset(&messageSend, 0x00, sizeof(IOCTL_MEI_messageSend_t));

   /* setup message send */
   messageSend.write_msg.msgId         = (unsigned short)pInMsg32Send->raw.data[VRX_DTI_MSG_IDX_MSDID];
   messageSend.write_msg.paylSize_byte = (unsigned short)inPaylSize_byte - (sizeof(IFX_uint32_t) << 1);
   pInMsg32Send->raw.data[VRX_DTI_MSG_IDX_LENGTH] |= (pInMsg32Send->raw.data[VRX_DTI_MSG_IDX_INDEX] << 16);
   messageSend.write_msg.pPayload      = (unsigned char*)&pInMsg32Send->raw.data[VRX_DTI_MSG_IDX_LENGTH]; 

   messageSend.ack_msg.paylSize_byte = CMV_USED_PAYLOAD_8BIT_SIZE;
   messageSend.ack_msg.pPayload      = (unsigned char *)&pOutMsg32Send->raw.data[VRX_DTI_MSG_IDX_LENGTH];

   if ( (DSL_CPE_Ioctl(pVrxDevCtx->pDevFds[lineNum], FIO_MEI_MBOX_MSG_SEND, (DSL_int_t)&messageSend)) < 0 )
   {
      /* in case of error loop back input message */
      pOutMsg32Send->raw.data[VRX_DTI_MSG_IDX_MSDID] = pInMsg32Send->raw.data[VRX_DTI_MSG_IDX_MSDID];
      pOutMsg32Send->raw.data[VRX_DTI_MSG_IDX_INDEX] = pInMsg32Send->raw.data[VRX_DTI_MSG_IDX_INDEX];
      pOutMsg32Send->raw.data[VRX_DTI_MSG_IDX_LENGTH] = pInMsg32Send->raw.data[VRX_DTI_MSG_IDX_LENGTH];
      *pOutPaylSize_byte = sizeof(pOutMsg32Send->raw.data[VRX_DTI_MSG_IDX_MSDID]) * 3 + sizeof(pOutMsg32Send->raw.sendResult);
      pOutMsg32Send->raw.sendResult = IFX_ERROR;
      *pPacketError      = DTI_eErrPortOperation;
   }
   else
   {
      pOutMsg32Send->raw.data[VRX_DTI_MSG_IDX_MSDID] = (IFX_uint32_t)messageSend.ack_msg.msgId;
      pOutMsg32Send->raw.data[VRX_DTI_MSG_IDX_INDEX] |= (pOutMsg32Send->raw.data[VRX_DTI_MSG_IDX_LENGTH] >> 16);
      *pOutPaylSize_byte = (IFX_int_t)(messageSend.ack_msg.paylSize_byte + sizeof(pOutMsg32Send->raw.data[VRX_DTI_MSG_IDX_MSDID]) +
                            sizeof(pOutMsg32Send->raw.sendResult));
      pOutMsg32Send->raw.sendResult = IFX_SUCCESS;
      *pPacketError = DTI_eNoError;
   }

   return IFX_SUCCESS;
}

#if (DTI_DEVICE_INTERFACE_VERSION) > (0x00000100)
/**
   Send a 8 Bit message to the device and wait for the responce.

\param
   pDtiDevCtx        - points to the VRX Device context.
\param
   pInMsg32Send      - points to the DTI 32 bit message send struct.
\param
   pOutMsg32Send     - points to the DTI 32 bit message responce struct.
\param
   lineNum           - line number.
\param
   inPaylSize_byte   - payload size of the DTI packet [byte].
\param
   pOutPaylSize_byte - return ptr, return the OUT payload size.
\param
   pPacketError      - return ptr, return the DTI Packet Error.

\return
   IFX_SUCCESS and returns the DTI Packet Error code.
   IFX_ERROR   currently no (only in case of non-DTI related errors).
*/
DTI_STATIC IFX_int_t DTI_VRX_Message8Send(
                        DTI_DeviceCtx_t * pDtiDevCtx,
                        DTI_H2D_Message8_u *pInMsg8Send,
                        DTI_D2H_Message8_u *pOutMsg8Send, 
                        IFX_int_t devNum,
                        IFX_int_t inPaylSize_byte,
                        IFX_int_t * pOutPaylSize_byte,
                        DTI_PacketError_t *pPacketError)
{
   DTI_PRN_USR_ERR_NL(DTI_VRX, DTI_PRN_LEVEL_ERR,
      ("ERROR: Vrx Msg8 Send - not necessary (supported)."DTI_CRLF));

   *pPacketError  = DTI_eErrUnknown;

   return IFX_SUCCESS;
}
#endif /*(DTI_DEVICE_INTERFACE_VERSION) > (0x00000100)*/

/**
   Setup the trace buffer configuration (xDSL Rev3 debug streams).

\remark
   This function releases current configuration if the Debug Streams are
   already configured before the new config is set.

\param
   pDtiDevCtx           - points to the VRX Device context.
\param
   pInTraceConfigSet    - points to the DTI Host2Dev TraceConfigSet struct.
\param
   pOutTraceConfigSet   - points to the DTI Dev2Host TraceConfigSet struct.
\param
   lineNum              - line number.
\param
   pPacketError         - return ptr, return the DTI Packet Error.

\return
   IFX_SUCCESS and returns the DTI Packet Error code.
   IFX_ERROR   debug stream setup failed.
*/
DTI_STATIC IFX_int_t DTI_VRX_TraceBufferConfigSet(
                        DTI_DeviceCtx_t            *pDtiDevCtx,
                        DTI_H2D_TraceConfigSet_t   *pInTraceConfigSet,
                        DTI_D2H_TraceConfigSet_t   *pOutTraceConfigSet,
                        IFX_int_t                  lineNum,
                        DTI_PacketError_t          *pPacketError)
{
   IFX_int_t   nRet = IFX_SUCCESS;
   IFX_uint_t  nOpMode = 0;
   IOCTL_MEI_DEBUG_STREAM_configGet_t pCfgGet;
   DTI_DEV_VrxDriverCtx_t *pVrxDevCtx = pDtiDevCtx->pDevice;
   IOCTL_MEI_DEBUG_STREAM_release_t release;
   union {
      IOCTL_MEI_DEBUG_STREAM_configSet_t cfg;
      IOCTL_MEI_DEBUG_STREAM_control_t ctrl;
   } ioCmd;

   if (DTI_DevOpen(pVrxDevCtx, pPacketError, lineNum) != IFX_SUCCESS) 
   {
      DTI_PRN_USR_ERR_NL(DTI_VRX, DTI_PRN_LEVEL_ERR,
         ("ERROR: Mei TraceBufferConfigSet - line %d %s"DTI_CRLF,
         lineNum, (*pPacketError == DTI_eErrPortOutOfRange) ?
         "range error" : "open error"));
      pOutTraceConfigSet->size = 0;
      return IFX_SUCCESS;
   }

   *pPacketError  = DTI_eNoError;

   nRet = DTI_traceBufferDbgStrmCfg(pVrxDevCtx, lineNum, &pCfgGet, IFX_NULL);
   if (nRet != IFX_SUCCESS)
   {
      /* return, still not initialized, nothing to do for switch off*/
      pOutTraceConfigSet->size = 0;
      return IFX_SUCCESS;
   }

   switch ((DTI_TraceBufferMode_t)pInTraceConfigSet->mode)
   {
      case DTI_eDisabled:
         /* switch off - and return */
         pOutTraceConfigSet->size = 0;
         nOpMode = e_MEI_DBG_STREAM_DEFAULT_RING;
         break;

      case DTI_eLinear:
         nOpMode = e_MEI_DBG_STREAM_FILL;
         break;

      case DTI_eCircular:
         nOpMode = e_MEI_DBG_STREAM_USER_RING;
         break;

      case DTI_eFifo:
         nOpMode = e_MEI_DBG_STREAM_FIFO;
         break;

      default:
         /* invalid config request */
         *pPacketError = DTI_eErrInvalidParameters;
         pOutTraceConfigSet->size = 0;
         return IFX_SUCCESS;
   }

   /* debug streams already initialized */
   if (((DTI_TraceBufferMode_t)pInTraceConfigSet->mode == DTI_eDisabled)
      || ((unsigned int)nOpMode != pCfgGet.operationMode))
   {
      /* release current config */
      memset(&release, 0, sizeof(release));
      release.releaseMode = e_MEI_DBG_STREAM_RELEASE_COMPLETELY;
      (void)IFXOS_DeviceControl(pVrxDevCtx->pDevFds[lineNum],
         FIO_MEI_DEBUG_STREAM_RELEASE,
         (IFX_int_t)&release);

      if ((DTI_TraceBufferMode_t)pInTraceConfigSet->mode ==
         DTI_eDisabled)
      {
         /* return - disabled */
         pOutTraceConfigSet->size = 0;
         return IFX_SUCCESS;
      }
   }

   DTI_MemSet(&ioCmd.cfg, 0, sizeof(IOCTL_MEI_DEBUG_STREAM_configSet_t));
   ioCmd.cfg.operationMode = (MEI_DBG_STREAM_BUF_OPMODE_E)nOpMode;
   ioCmd.cfg.bufferSize = (unsigned int) pInTraceConfigSet->size;

   /* new init */
   nRet = DTI_traceBufferDbgStrmCfg(pVrxDevCtx, lineNum,
      &pCfgGet, &ioCmd.cfg);
   if (nRet != IFX_SUCCESS)
   {
      pOutTraceConfigSet->size = 0;
      *pPacketError = DTI_eErrPortOperation;
   }

   DTI_MemSet(&ioCmd.ctrl, 0, sizeof(IOCTL_MEI_DEBUG_STREAM_control_t));
   ioCmd.ctrl.operationMode = (MEI_DBG_STREAM_BUF_OPMODE_E)nOpMode;
   ioCmd.ctrl.onOff = 1;

   if (IFXOS_DeviceControl(pVrxDevCtx->pDevFds[lineNum],
      FIO_MEI_DEBUG_STREAM_CONTROL,
      (IFX_int_t)&ioCmd.ctrl) < 0)
   {
      /* error - by enabling debug streams, release it completely */
      memset(&release, 0, sizeof(release));
      release.releaseMode = e_MEI_DBG_STREAM_RELEASE_COMPLETELY;
      (void)IFXOS_DeviceControl(pVrxDevCtx->pDevFds[lineNum],
         FIO_MEI_DEBUG_STREAM_RELEASE,
         (IFX_int_t)&release);

      *pPacketError = DTI_eErrPortOperation;
      pOutTraceConfigSet->size = 0;

      return IFX_SUCCESS;
   }

   pOutTraceConfigSet->size = (IFX_uint32_t)pCfgGet.bufferSize;

   return IFX_SUCCESS;
}


/**
   Reset the current trace buffer (xDSL Rev3 debug streams).

\param
   pDtiDevCtx     - points to the VRX Device context.
\param
   lineNum        - line number.
\param
   pPacketError   - return ptr, return the DTI Packet Error.

\return
   IFX_SUCCESS and returns the DTI Packet Error code.
   IFX_ERROR   debug stream setup failed.
*/
DTI_STATIC IFX_int_t DTI_VRX_TraceBufferReset(
                        DTI_DeviceCtx_t      *pDtiDevCtx,
                        IFX_int_t            lineNum,
                        DTI_PacketError_t    *pPacketError)
{
   IOCTL_MEI_DEBUG_STREAM_release_t release;
   DTI_DEV_VrxDriverCtx_t *pVrxDevCtx = pDtiDevCtx->pDevice;

   if (DTI_DevOpen(pVrxDevCtx, pPacketError, lineNum) != IFX_SUCCESS)
   {
      DTI_PRN_USR_ERR_NL(DTI_VRX, DTI_PRN_LEVEL_ERR,
         ("ERROR: Mei trace buffer reset - line %d %s."DTI_CRLF,
         lineNum, (*pPacketError == DTI_eErrPortOutOfRange) ?
         "range error" : "open error"));

      return IFX_SUCCESS;
   }

   memset(&release, 0, sizeof(release));
   release.releaseMode = e_MEI_DBG_STREAM_DELETE_DATA_AND_STATISTIC;

   if (IFXOS_DeviceControl(pVrxDevCtx->pDevFds[lineNum],
      FIO_MEI_DEBUG_STREAM_RELEASE,
      (IFX_int_t)&release) < 0 )
   {
      *pPacketError = DTI_eErrPortOperation;
      return IFX_SUCCESS;
   }
   *pPacketError  = DTI_eNoError;
   return IFX_SUCCESS;
}

/**
   Setup the trace buffer configuration (xDSL Rev3 debug streams).

\param
   pDtiDevCtx           - points to the VRX Device context.
\param
   pOutTraceStatusGet   - points to the DTI Dev2Host TraceStatusGet struct.
\param
   lineNum              - line number.
\param
   pPacketError         - return ptr, return the DTI Packet Error.

\return
   IFX_SUCCESS and returns the DTI Packet Error code.
   IFX_ERROR   debug stream config request failed.
*/
DTI_STATIC IFX_int_t DTI_VRX_TraceBufferStatusGet(
                        DTI_DeviceCtx_t            *pDtiDevCtx,
                        DTI_D2H_TraceStatusGet_t   *pOutTraceStatusGet,
                        IFX_int_t                  lineNum,
                        DTI_PacketError_t          *pPacketError)
{
   IFX_int_t nRet = IFX_SUCCESS;
   DTI_DEV_VrxDriverCtx_t *pVrxDevCtx = pDtiDevCtx->pDevice;
   union {
      IOCTL_MEI_DEBUG_STREAM_configGet_t pCfgGet;
      IOCTL_MEI_DEBUG_STREAM_statistic_t statistics;
   } ioCmd;

   if (DTI_DevOpen(pVrxDevCtx, pPacketError, lineNum) != IFX_SUCCESS)
   {
      DTI_PRN_USR_ERR_NL(DTI_VRX, DTI_PRN_LEVEL_ERR,
         ("ERROR: Mei TraceBufferStatusGet - line %d %s."DTI_CRLF,
         lineNum,
         (*pPacketError == DTI_eErrPortOutOfRange) ?
         "range error" : "open error"));

      goto DTI_DEV_TRACE_BUFFER_STATUS_GET_ERR;
   }

   *pPacketError = DTI_eNoError;

   nRet = DTI_traceBufferDbgStrmCfg(pVrxDevCtx, lineNum,
      &ioCmd.pCfgGet, IFX_NULL);
   if (nRet != IFX_SUCCESS)
   {
      *pPacketError = DTI_eErrConfiguration;

      goto DTI_DEV_TRACE_BUFFER_STATUS_GET_ERR;
   }

   if (ioCmd.pCfgGet.onOff == 0)
   {
      pOutTraceStatusGet->mode = DTI_eDisabled;
      pOutTraceStatusGet->size = 0;
      pOutTraceStatusGet->fill = 0;
   }
   else
   {
      switch (ioCmd.pCfgGet.operationMode)
      {
         case e_MEI_DBG_STREAM_FILL:
            pOutTraceStatusGet->mode = DTI_eLinear;
            break;
         case e_MEI_DBG_STREAM_USER_RING:
            pOutTraceStatusGet->mode = DTI_eCircular;
            break;
         case e_MEI_DBG_STREAM_FIFO:
            pOutTraceStatusGet->mode = DTI_eFifo;
            break;
         default:
            pOutTraceStatusGet->mode =
            (IFX_uint32_t)ioCmd.pCfgGet.operationMode;
      }

      pOutTraceStatusGet->size = (IFX_uint32_t)ioCmd.pCfgGet.bufferSize;

      DTI_MemSet(&ioCmd.statistics, 0x00,
         sizeof(IOCTL_MEI_DEBUG_STREAM_statistic_t));

      if (IFXOS_DeviceControl(pVrxDevCtx->pDevFds[lineNum],
         FIO_MEI_DEBUG_STREAM_STATISTIC_GET,
         (IFX_int_t)&ioCmd.statistics) < 0)
      {
         /* error - get statistics */
         *pPacketError = DTI_eErrPortOperation;
         pOutTraceStatusGet->fill = 0;
         return IFX_SUCCESS;
      }

      pOutTraceStatusGet->fill =
      (IFX_uint32_t)(ioCmd.statistics.dataH2BufSize_byte -
      ioCmd.statistics.dataBuf2UsrSize_byte);
   }

   return IFX_SUCCESS;

DTI_DEV_TRACE_BUFFER_STATUS_GET_ERR:

   pOutTraceStatusGet->mode = 0;
   pOutTraceStatusGet->size = 0;
   pOutTraceStatusGet->fill = 0;

   return IFX_SUCCESS;
}

/**
   Read data from the device via Debug Access (MEI Debug).

\param
   pDtiDevCtx           - points to the VRX Device context.
\param
   pInTraceBufferGet    - points to the DTI Host2Dev RegisterGet struct.
\param
   ppUsedDtiPacketOut   - return ptr, points to the DTI Out packet.
\param
   pUsedBufferOutSize   - return value, points to the DTI Out packet size.
\param
   lineNum              - line number.
\param
   pTrBufReadSize_byte  - return ptr, return the number of read registers.
\param
   pPacketError         - return ptr, return the DTI Packet Error.

\remark
   The function is called with the standard out packet. If the requested size
   does not fit into this buffer, a corresponding buffer is allocated and used
   for the DSL_CPE_Ioctl call. The pointer to this out packet will be returned.

\return
   IFX_SUCCESS  debug stream data read successful.
      - returns the DTI Packet Error code.
      - pointer to the used out package.
      - size of the used out package.
   IFX_ERROR   data read failed.
*/
DTI_STATIC IFX_int_t DTI_VRX_TraceBufferGet(
                        DTI_DeviceCtx_t            *pDtiDevCtx,
                        DTI_H2D_TraceBufferGet_t   *pInTraceBufferGet,
                        DTI_Packet_t               **ppUsedDtiPacketOut,
                        IFX_int_t                  *pUsedBufferOutSize,
                        IFX_int_t                  lineNum,
                        IFX_int_t                  *pTrBufReadSize_byte,
                        DTI_PacketError_t          *pPacketError)
{
   IFX_int_t nRet = (IFX_int_t)IFX_SUCCESS, nBufferOutSize;
   DTI_PTR_U uPtr;
   IOCTL_MEI_DEBUG_STREAM_data_t stream;
   DTI_DEV_VrxDriverCtx_t *pVrxDevCtx = pDtiDevCtx->pDevice;
   DTI_Packet_t *pDtiPacketOut;
   DTI_D2H_TraceBufferGet_t *pOutTraceBufferGet;

   /* check if we use the local or default buffer for responce */
   if (pInTraceBufferGet->size > (IFX_uint32_t)(*pUsedBufferOutSize -
      sizeof(DTI_PacketHeader_t)))
   {
      /* use local user buffer */
      if (pVrxDevCtx->pDbgStreamUserBuf != IFX_NULL)
      {
         if (pInTraceBufferGet->size >
            (IFX_uint32_t) (pVrxDevCtx->dbgStreamUserBuf_byte -
            sizeof(DTI_PacketHeader_t)) )
         {
            DTI_Free(pVrxDevCtx->pDbgStreamUserBuf);
            pVrxDevCtx->pDbgStreamUserBuf = IFX_NULL;
            pVrxDevCtx->dbgStreamUserBuf_byte = 0;
         }
      }

      if (pVrxDevCtx->pDbgStreamUserBuf == IFX_NULL)
      {
         nBufferOutSize = (IFX_int_t)(((sizeof(DTI_PacketHeader_t) +
            pInTraceBufferGet->size) & ~0xFFF) + 0x1000);

         if (nBufferOutSize > DTI_DEV_MEI_DBG_STREAM_MAX_BUFFER_SIZE)
         {
            nBufferOutSize = DTI_DEV_MEI_DBG_STREAM_MAX_BUFFER_SIZE;
         }

         pVrxDevCtx->dbgStreamUserBuf_byte = nBufferOutSize;
         pVrxDevCtx->pDbgStreamUserBuf = DTI_Malloc((IFX_size_t)nBufferOutSize);

         if (pVrxDevCtx->pDbgStreamUserBuf == IFX_NULL)
         {
            pVrxDevCtx->dbgStreamUserBuf_byte = 0;

            *pPacketError = DTI_eErrMalformedPacket;
            *pTrBufReadSize_byte = 0;

            return IFX_SUCCESS;
         }
      }

      uPtr.pUInt8 = pVrxDevCtx->pDbgStreamUserBuf;
      pDtiPacketOut = (DTI_Packet_t *)DTI_PTR_CAST_GET_ULONG(uPtr);

      if (pDtiPacketOut == IFX_NULL)
      {
         if (pVrxDevCtx->pDbgStreamUserBuf != IFX_NULL)
         {
            DTI_Free(pVrxDevCtx->pDbgStreamUserBuf);
            pVrxDevCtx->pDbgStreamUserBuf = IFX_NULL;
            pVrxDevCtx->dbgStreamUserBuf_byte = 0;
         }

         *pPacketError = DTI_eErrMalformedPacket;
         *pTrBufReadSize_byte = 0;

         DTI_PRN_USR_ERR_NL(DTI_VRX, DTI_PRN_LEVEL_ERR,
            ("Error: Mei Packet Trace Buffer Get - "\
            "missaligned user stream buffer."DTI_CRLF));

         return IFX_SUCCESS;
      }
      nBufferOutSize = pVrxDevCtx->dbgStreamUserBuf_byte;
   }
   else
   {
      /* use default buffer */
      nBufferOutSize = *pUsedBufferOutSize;
      pDtiPacketOut = *ppUsedDtiPacketOut;
   }

   /* asign packet payload */
   uPtr.pUInt8 = pDtiPacketOut->payload;
   pOutTraceBufferGet =(DTI_D2H_TraceBufferGet_t *)DTI_PTR_CAST_GET_ULONG(uPtr);

   if (pOutTraceBufferGet == IFX_NULL)
   {
      if (pVrxDevCtx->pDbgStreamUserBuf != IFX_NULL)
      {
         DTI_Free(pVrxDevCtx->pDbgStreamUserBuf);
         pVrxDevCtx->pDbgStreamUserBuf = IFX_NULL;
         pVrxDevCtx->dbgStreamUserBuf_byte = 0;
      }

      *pPacketError = DTI_eErrMalformedPacket;
      *pTrBufReadSize_byte = 0;

      DTI_PRN_USR_ERR_NL(DTI_VRX, DTI_PRN_LEVEL_ERR,
         ("Error: Mei Packet Trace Buffer Get - "\
         "missaligned payload."DTI_CRLF));

      return IFX_SUCCESS;
   }

   if (DTI_DevOpen(pVrxDevCtx, pPacketError, lineNum) != IFX_SUCCESS)
   {
      *pTrBufReadSize_byte = 0;

      DTI_PRN_USR_ERR_NL(DTI_VRX, DTI_PRN_LEVEL_ERR,
         ("ERROR: Mei Packet Trace Buffer Get - "\
         "line %d %s." DTI_CRLF,
         lineNum,
         (*pPacketError == DTI_eErrPortOutOfRange) ?
         "range error" : "open error"));

      return IFX_SUCCESS;
   }

   /* read until buffer full or no more data available */
   DTI_MemSet(&stream, 0x00, sizeof(IOCTL_MEI_DEBUG_STREAM_data_t));
   stream.dataBufferSize_byte = (unsigned int)pInTraceBufferGet->size;
   stream.pData = (unsigned char *)pOutTraceBufferGet->data;

   DTI_PRN_USR_ERR_NL(DTI_VRX, DTI_PRN_LEVEL_LOW,
      ("ERROR: Mei Packet Trace Buffer Get - line %d, "\
      "InTraceBufferGet->size=%d, nBufferOutSize=%d."\
      DTI_CRLF,
      lineNum, pInTraceBufferGet->size, nBufferOutSize));

   if ((stream.dataBufferSize_byte == 0) || (stream.pData == IFX_NULL))
   {
      *pPacketError = DTI_eErrPortOperation;
      *pTrBufReadSize_byte = 0;
      return IFX_ERROR;
   }
   nRet = IFXOS_DeviceControl(pVrxDevCtx->pDevFds[lineNum],
      FIO_MEI_DEBUG_STREAM_DATA_GET,
      (IFX_int_t)&stream);
   if (nRet  < 0)
   {
      *pPacketError = DTI_eErrPortOperation;
      *pTrBufReadSize_byte = 0;
      return IFX_SUCCESS;
   }

   if (((nBufferOutSize - sizeof(DTI_PacketHeader_t)) <
      (IFX_int_t)stream.dataBufferSize_byte) ||
      (pInTraceBufferGet->size < (IFX_uint32_t)stream.dataBufferSize_byte) )
   {
      DTI_PRN_USR_ERR_NL(DTI_VRX, DTI_PRN_LEVEL_ERR,
         ("ERROR: Mei Packet Trace Buffer Get - "\
         "line %d, ioctl error recv = %d > "\
         "ioctlreq = %d / dtireq = %d."DTI_CRLF,
         lineNum, stream.dataBufferSize_byte,
         (nBufferOutSize -
         sizeof(DTI_PacketHeader_t)),
         pInTraceBufferGet->size ));

      *pTrBufReadSize_byte = (IFX_int_t)pInTraceBufferGet->size;
   }
   else
   {
      *pTrBufReadSize_byte = (IFX_int_t)stream.dataBufferSize_byte;
   }

   *pPacketError = DTI_eNoError;
   *pTrBufReadSize_byte  = (IFX_int_t)stream.dataBufferSize_byte;
   *pUsedBufferOutSize = nBufferOutSize;
   *ppUsedDtiPacketOut  = pDtiPacketOut;

   return IFX_SUCCESS;
}

/**
   Read data from the device via Debug Access (MEI Debug).

\param
   pDtiDevCtx     - points to the VRX Device context.
\param
   pInDbgGet      - points to the DTI Host2Dev RegisterGet struct.
\param
   pOutDbgGet     - points to the DTI Dev2Host RegisterGet struct.
\param
   lineNum        - line number.
\param
   pDbgReadCount  - return ptr, return the number of read registers.
\param
   pPacketError   - return ptr, return the DTI Packet Error.

\return
   IFX_SUCCESS and returns the DTI Packet Error code.
   IFX_ERROR   currently no (only in case of non-DTI related errors).
*/
DTI_STATIC IFX_int_t DTI_VRX_DebugRead(
                        DTI_DeviceCtx_t      *pDtiDevCtx,
                        DTI_H2D_DebugRead_t  *pInDbgGet,
                        DTI_D2H_DebugRead_t  *pOutDbgGet,
                        IFX_int_t            lineNum,
                        IFX_int_t            *pDbgReadCount,
                        DTI_PacketError_t    *pPacketError)
{
   IOCTL_MEI_dbgAccess_t    dbgAcc;
   DTI_DEV_VrxDriverCtx_t  *pVrxDevCtx = (DTI_DEV_VrxDriverCtx_t *)pDtiDevCtx->pDevice;

   *pDbgReadCount = 0;
   *pPacketError  = DTI_eNoError;

   if (pInDbgGet->count > (IFX_uint32_t)pVrxDevCtx->numOfRegAccess)
   {
      DTI_PRN_USR_ERR_NL(DTI_VRX, DTI_PRN_LEVEL_ERR,
         ("ERROR: Vrx DebugRead - line %d, count %d > max %d."DTI_CRLF,
          lineNum, pInDbgGet->count, pVrxDevCtx->numOfDebugAccess));

      *pPacketError  = DTI_eErrConfiguration;

      return IFX_SUCCESS;
   }

   if (DTI_DevOpen(pVrxDevCtx, pPacketError, lineNum) != IFX_SUCCESS)
   {
      DTI_PRN_USR_ERR_NL(DTI_VRX, DTI_PRN_LEVEL_ERR,
         ("ERROR: Vrx DebugRead - device open error." DTI_CRLF));

      return IFX_SUCCESS;
   }

   DTI_MemSet(&dbgAcc, 0x00, sizeof(IOCTL_MEI_dbgAccess_t));
   dbgAcc.count    = (unsigned int)((pInDbgGet->count) ? pInDbgGet->count : 1);
   dbgAcc.dbgAddr  = (unsigned int)pInDbgGet->offset;
   dbgAcc.dbgDest  = (unsigned int) pInDbgGet->type;
   dbgAcc.pData_32 = (unsigned int *) pOutDbgGet->data;

   if ( (DSL_CPE_Ioctl(
               pVrxDevCtx->pDevFds[lineNum],
               FIO_MEI_DBG_READ,
               (DSL_int_t)&dbgAcc)) < 0 )
   {
      DTI_PRN_USR_ERR_NL(DTI_VRX, DTI_PRN_LEVEL_ERR,
         ("ERROR: Vrx DebugRead - ioctl error)."DTI_CRLF));

      *pPacketError = DTI_eErrPortOperation;
   }
   else
   {
      *pDbgReadCount = (IFX_int_t)dbgAcc.count;
   }

   return IFX_SUCCESS;
}


/**
   Write data to the device via Debug Access (MEI Debug).

\param
   pDtiDevCtx     - points to the VRX Device context.
\param
   pInDbgSet      - points to the DTI Host2Dev RegisterGet struct.
\param
   pOutDbgGet_nU  - not used.
\param
   lineNum        - line number.
\param
   pDbgWriteCount - return ptr, return the number of read registers.
\param
   pPacketError   - return ptr, return the DTI Packet Error.

\return
   IFX_SUCCESS and returns the DTI Packet Error code.
   IFX_ERROR   currently no (only in case of non-DTI related errors).
*/
DTI_STATIC IFX_int_t DTI_VRX_DebugWrite(
                        DTI_DeviceCtx_t      *pDtiDevCtx,
                        DTI_H2D_DebugWrite_t *pInDbgSet,
                        IFX_uint32_t         *pOutDbgGet_nU,
                        IFX_int_t            lineNum,
                        IFX_int_t            *pDbgWriteCount,
                        DTI_PacketError_t    *pPacketError)
{
   IOCTL_MEI_dbgAccess_t    dbgAcc;
   DTI_DEV_VrxDriverCtx_t  *pVrxDevCtx = (DTI_DEV_VrxDriverCtx_t *)pDtiDevCtx->pDevice;

   *pPacketError   = DTI_eNoError;

   if (*pDbgWriteCount > pVrxDevCtx->numOfRegAccess)
   {
      DTI_PRN_USR_ERR_NL(DTI_VRX, DTI_PRN_LEVEL_ERR,
         ("ERROR: Vrx DebugWrite - line %d, count %d > max %d."DTI_CRLF,
          lineNum, *pDbgWriteCount, pVrxDevCtx->numOfDebugAccess));

      *pDbgWriteCount = 0;
      *pPacketError   = DTI_eErrConfiguration;

      return IFX_SUCCESS;
   }

   if (DTI_DevOpen(pVrxDevCtx, pPacketError, lineNum) != IFX_SUCCESS)
   {
      DTI_PRN_USR_ERR_NL(DTI_VRX, DTI_PRN_LEVEL_ERR,
         ("ERROR: Vrx DebugRead - device open error." DTI_CRLF));

      *pDbgWriteCount = 0;
      return IFX_SUCCESS;
   }

   DTI_MemSet(&dbgAcc, 0x00, sizeof(IOCTL_MEI_dbgAccess_t));
   dbgAcc.count    = (*pDbgWriteCount > 0) ? ((unsigned int)*pDbgWriteCount) : 0;
   dbgAcc.dbgAddr  = (unsigned int) pInDbgSet->offset;
   dbgAcc.dbgDest  = (unsigned int) pInDbgSet->type;
   dbgAcc.pData_32 = (unsigned int *) pInDbgSet->data;

   if ( (DSL_CPE_Ioctl(
               pVrxDevCtx->pDevFds[lineNum],
               FIO_MEI_DBG_WRITE,
               (DSL_int_t)&dbgAcc)) < 0 )
   {
      DTI_PRN_USR_ERR_NL(DTI_VRX, DTI_PRN_LEVEL_ERR,
         ("ERROR: Vrx DebugWrite - ioctl error."DTI_CRLF));

      *pDbgWriteCount = 0;
      *pPacketError = DTI_eErrPortOperation;
   }
   else
   {
      *pDbgWriteCount = (IFX_int_t)dbgAcc.count;
   }

   return IFX_SUCCESS;
}

DTI_STATIC IFX_int_t DTI_VRX_WinEasyCiAccess(
                        DTI_DeviceCtx_t      *pDtiDevCtx,
                        IFX_int_t            lineNum,
                        const IFX_uint8_t    *pDataIn,
                        const IFX_uint32_t   sizeIn,
                        IFX_uint8_t          *pDataOut,
                        const IFX_uint32_t   sizeOut,
                        DTI_PacketError_t    *pPacketError)
{

   DTI_PRN_USR_ERR_NL(DTI_VRX, DTI_PRN_LEVEL_ERR,
      ("ERROR: Vrx WinEasy Access - not supported."DTI_CRLF));

   *pPacketError = DTI_eErrUnknown;
   return IFX_SUCCESS;
}

/**
   Check and process for Device Autonomous Messages.
   The Auto Msg is read form the device (driver) and a corresponding
   DTI packet is created and sent to the upper DTI Client

\param
   pDtiDevCtx           - points to the Device context.
\param
   pDtiCon              - points to the established DTI Connection data.
\param
   devSelectWait_ms     - wait time [ms] for the device select operation.
\param
   pOutBuffer           - points to the DTI packet out buffer to create a DTI packet.
\param
   outBufferSize_byte   - DTI out buffer size [byte]

\return
   IFX_SUCCESS if the DTI packet has been sent.
   IFX_ERROR   DTI packet has not been sent.

\remarks
   At first the device msg will be read out to free the internal driver buffer.
   This is done independant on the established connection.
*/
DTI_STATIC IFX_int_t DTI_VRX_AutoMsgProcess(
                        DTI_DeviceCtx_t         *pDtiDevCtx,
                        const DTI_Connection_t  *pDtiCon,
                        IFX_uint32_t            devSelectWait_ms,
                        IFX_char_t              *pOutBuffer,
                        IFX_int_t               outBufferSize_byte)
{
#ifndef DTI_DONT_USE_PROTEXT
   IFX_int_t      lineNum = 0, retVal = IFX_SUCCESS;
   DTI_DEV_VrxDriverCtx_t  *pVrxDevCtx = (DTI_DEV_VrxDriverCtx_t *)pDtiDevCtx->pDevice;

   if (pVrxDevCtx->nfcMaxDevFd == 0)
   {
      /* min DevFd = 0 --> maxFD = 0 +1, no device enabled */
      return IFX_SUCCESS;
   }

   DTI_DevFdZero(&pVrxDevCtx->tmpDevFds);
   retVal = DTI_DeviceSelect(
               (IFX_uint32_t)pVrxDevCtx->nfcMaxDevFd,
               &pVrxDevCtx->nfcDevFds, &pVrxDevCtx->tmpDevFds, devSelectWait_ms);

   if ( retVal < 0 )
   {
      return IFX_ERROR;
   }

   if (retVal == 0)
   {
      return IFX_SUCCESS;
   }

   for (lineNum = 0; lineNum < pVrxDevCtx->numOfPorts; lineNum++)
   {
      if (pVrxDevCtx->pDevFds[lineNum] != -1)
      {
         if ( DTI_DevFdIsSet((IFX_uint32_t)pVrxDevCtx->pDevFds[lineNum], &pVrxDevCtx->tmpDevFds) )
         {
            retVal--;
            (void)DTI_autoMsgRecv( 
                        pVrxDevCtx, pDtiCon, 
                        pDtiDevCtx->devIfNum, lineNum,
                        pOutBuffer, outBufferSize_byte);
         }
      }

      if (retVal == 0)
         break;
   }
#else
   DTI_PRN_USR_ERR_NL(DTI_VRX, DTI_PRN_LEVEL_ERR,
      ("ERROR: Vrx Auto msg - not supported."DTI_CRLF));
#endif /* !DTI_DONT_USE_PROTEXT */

   return IFX_SUCCESS;
} 

#endif /* INCLUDE_DSL_CPE_API_VRX*/


