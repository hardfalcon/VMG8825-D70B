/******************************************************************************

                              Copyright (c) 2009
                            Lantiq Deutschland GmbH
                     Am Campeon 3; 85579 Neubiberg, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/
#ifndef _DTI_DEVICE_H
#define _DTI_DEVICE_H

#ifdef __cplusplus
   extern "C" {
#endif

/** \file
   Defines the definitions for the DTI Device Access.
   The device access is done via the correspondig device specific functions.
   The functions are kept within a handler function table and this table
   must be registered by the device specific implementation.
*/

/* ==========================================================================
   includes
   ========================================================================== */
#include "ifx_types.h"
#include "ifxos_debug.h"
#include "ifx_dti_protocol.h"
#include "ifx_dti_protocol_device.h"

#include "dti_protocol_ext.h"

/* ==========================================================================
   defines
   ========================================================================== */


/* ==========================================================================
   macro
   ========================================================================== */

/** current version of the device interface definition */
#define DTI_DEVICE_INTERFACE_VERSION            0x00010200

/** vendor name */
#define DTI_VENDOR_NAME_STR                     "Lantiq"

/** max length of the device interface name */
#define DTI_MAX_LEN_DEVICE_INTERFACE_NAME             16

/* ==========================================================================
   types
   ========================================================================== */

/** Forward declaration: Device access functions */
typedef struct DTI_DeviceAccessFct_s DTI_DeviceAccessFct_t;


/**
   Global Device System Info.
*/
typedef struct DTI_deviceSysInfo_s
{
   /** info is valid */
   IFX_boolean_t  bValid;
   /** number of devices */
   IFX_int_t      numOfDevs;
   /** ports per devices */
   IFX_int_t      portsPerDev;
   /** interfaces per device */
   IFX_int_t      ifPerDev;
   /** number of ports */
   IFX_int_t      numOfPorts;
   /** support auto msg handling */
   IFX_boolean_t  bControlAutoDevMsgSupport;
   /** optional parameter, could be used in the device specific agent implementation */
   IFX_int_t      param0;
} DTI_DeviceSysInfo_t;


/**
   Device specific Control Struct
*/
typedef struct DTI_DeviceCtx_s
{
   /** Interface number wihtin the Agent context */
   IFX_int_t               devIfNum;
   /** Device access functions */
   DTI_DeviceAccessFct_t   *pDeviceAccessFct;

   /** set if auto msg handling is required */
   IFX_boolean_t           bAutoDevMsgActive;
   /** device specific user context */
   IFX_void_t              *pDevice;

   /** reference to incoming Packet structure */
   DTI_Packet_t            *pPacketIn;
   /** reference to outgoing Packet structure */
   DTI_Packet_t            *pPacketOut;

} DTI_DeviceCtx_t;

/**
   Function signature to setup DTI Module

\param
   pDeviceSystemInfo - points to the agent system infos
\param
   ppDtiDevCtx       - pointer to return the allocated DTI Modul struct.

\return
   If success IFX_SUCCESS, the context is returned via the ppDtiVnxDrvCtx param
   else IFX_ERROR.

*/
typedef IFX_int_t (*DTI_DEV_ModuleSetup_f)(
                        DTI_DeviceSysInfo_t  *pDeviceSystemInfo,
                        DTI_DeviceCtx_t      **ppDtiDevCtx);

/** Function signature to delete a DTI Module.
   - close all open devices
   - free memory

\param
   pDeviceSystemInfo - points to the agent system infos
\param
   ppDtiDevCtx       - return pointer, points to the the DTI device context.
\return
   If success IFX_SUCCESS, the context is freed and the ptr is set to IFX_NULL.
   else IFX_ERROR.

*/
typedef IFX_int_t (*DTI_DEV_ModuleDelete_f)(
                        DTI_DeviceSysInfo_t  *pDeviceSystemInfo,
                        DTI_DeviceCtx_t      **ppDtiDevCtx);

/**
   Function signature to write the Sytem Info of the current DTI Agent instance
   to the given buffer.

\param
   pDeviceSystemInfo - points to the agent system infos
\param
   pSysInfoBuffer    - points to the Sytem Info char buffer.
\param
   bufferSize        - buffer size.

\return
   Number of written bytes.
*/
typedef IFX_int_t (*DTI_DEV_SystemInfoWrite_f)(
                        DTI_DeviceSysInfo_t  *pDeviceSysInfo,
                        IFX_char_t           *pSysInfoBuffer,
                        IFX_int_t            bufferSize);

/**
   Function signature to set the level for debug and error printous within
   the device module.

\note
   - Possible values are:
      - 1: Low       important/error, single/warnings and periodical info.
      - 2: Normal    important/error and single/warnings info.
      - 3: High      important/error info.
      - 0/4: OFF     no printouts.

\param
   newDbgLevel    - new trace and log level for debug printouts.

\return
   IFX_SUCCESS if new level has been set.
   IFX_ERROR   level not set, out of range or debug / error printout not enabled.
*/
typedef IFX_int_t (*DTI_DEV_PrintoutLevelSet_f)(
                        IFX_int_t            newDbgLevel);


/**
   Function signature to do a device reset on the selected device.

\param
   pDtiDevCtx     - points to the DTI Device context.
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
typedef IFX_int_t (*DTI_DEV_Reset_f)(
                        DTI_DeviceCtx_t         *pDtiDevCtx,
                        DTI_H2D_DeviceReset_t   *pInDevReset,
                        DTI_D2H_DeviceReset_t   *pOutDevReset,
                        IFX_int_t               rstMaskSize_32,
                        DTI_PacketError_t       *pPacketError);

/**
   Function signature to do a FW download on the selected devices

\param
   pDtiDevCtx        - points to the DTI Device context.
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
typedef IFX_int_t (*DTI_DEV_Download_f)(
                        DTI_DeviceCtx_t            *pDtiDevCtx,
                        DTI_ProtocolServerCtx_t    *pDtiProtServerCtx,
                        DTI_H2D_DeviceDownload_t   *pInDevDownload,
                        DTI_D2H_DeviceDownload_t   *pOutDevDownload,
                        DTI_PacketError_t          *pPacketError);

/**
   Function signature to open a given device (line/port).

\param
   pDtiDevCtx     - points to the DTI Device context.
\param
   lineNum        - line number.
\param
   pPacketError   - return ptr, return the DTI Packet Error.

\return
   IFX_SUCCESS and returns the DTI Packet Error code.
   IFX_ERROR   currently no (only in case of non-DTI related errors).
*/
typedef IFX_int_t (*DTI_DEV_DeviceOpen_f)(
                        DTI_DeviceCtx_t      *pDtiDevCtx,
                        IFX_int_t            lineNum,
                        DTI_PacketError_t    *pPacketError);

/**
   Function signature to close a given device (line/port).

\param
   pDtiDevCtx     - points to the DTI Device context.
\param
   lineNum        - line number.
\param
   pPacketError   - return ptr, return the DTI Packet Error.

\return
   IFX_SUCCESS and returns the DTI Packet Error code.
   IFX_ERROR   currently no (only in case of non-DTI related errors).
*/
typedef IFX_int_t (*DTI_DEV_DeviceClose_f)(
                        DTI_DeviceCtx_t      *pDtiDevCtx,
                        IFX_int_t            lineNum,
                        DTI_PacketError_t    *pPacketError);

/**
   Function signature to get / release the device lock.

\param
   pDtiDevCtx     - points to the DTI Device context.
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
typedef IFX_int_t (*DTI_DEV_RegisterLock_f)(
                        DTI_DeviceCtx_t      *pDtiDevCtx,
                        DTI_H2D_DeviceLock_t *pInLock,
                        DTI_D2H_DeviceLock_t *pOutLock,
                        IFX_int_t            lineNum,
                        DTI_PacketError_t    *pPacketError);

/**
   Function signature to get a device register (MEI interface).

\param
   pDtiDevCtx     - points to the DTI Device context.
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
typedef IFX_int_t (*DTI_DEV_RegisterGet_f)(
                        DTI_DeviceCtx_t         *pDtiDevCtx,
                        DTI_H2D_RegisterGet_t   *pInRegGet,
                        DTI_D2H_RegisterGet_t   *pOutRegGet,
                        IFX_int_t               lineNum,
                        IFX_uint32_t            *pOutPaylSize_byte,
                        DTI_PacketError_t       *pPacketError);

/**
   Function signature to set a device register (MEI interface).

\param
   pDtiDevCtx        - points to the DTI Device context.
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
typedef IFX_int_t (*DTI_DEV_RegisterSet_f)(
                        DTI_DeviceCtx_t         *pDtiDevCtx,
                        DTI_H2D_RegisterSet_t   *pInRegSet,
                        IFX_int_t               inRegSetSize_Byte,
                        IFX_int_t               lineNum,
                        DTI_PacketError_t       *pPacketError);

/**
   Function signature to set a device configuration.

\param
   pDtiConCtx     - points to the DTI connection setup.
\param
   pDtiDevCtx     - points to the DTI Device context.
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
typedef IFX_int_t (*DTI_DEV_ConfigSet_f)(
                        DTI_DeviceCtx_t            *pDtiDevCtx,
                        DTI_H2D_DeviceConfigSet_t  *pInCfgSet,
                        DTI_D2H_DeviceConfigSet_t  *pOutCfgSet,
                        IFX_int_t                  lineNum,
                        DTI_PacketError_t          *pPacketError);

/**
   Function signature to get the device configuration of the given line.

\param
   pDtiDevCtx     - points to the DTI Device context.
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
typedef IFX_int_t (*DTI_DEV_ConfigGet_f)(
                        DTI_DeviceCtx_t            *pDtiDevCtx,
                        DTI_H2D_DeviceConfigGet_t  *pInCfgGet,
                        DTI_D2H_DeviceConfigGet_t  *pOutCfgGet,
                        IFX_int_t                  lineNum,
                        DTI_PacketError_t          *pPacketError);

/**
   Function signature to send a 8 Bit message to the device and wait for the responce.

\param
   pDtiDevCtx        - points to the DTI Device context.
\param
   pInMsg8Send      - points to the DTI 8 bit message send struct.
\param
   pOutMsg8Send     - points to the DTI 8 bit message responce struct.
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
typedef IFX_int_t (*DTI_DEV_Message8Send_f)(
                        DTI_DeviceCtx_t      *pDtiDevCtx,
                        DTI_H2D_Message8_u   *pInMsg16Send,
                        DTI_D2H_Message8_u   *pOutMsg16Send,
                        IFX_int_t            lineNum,
                        IFX_int_t            inPaylSize_byte,
                        IFX_int_t            *pOutPaylSize_byte,
                        DTI_PacketError_t    *pPacketError);

/**
   Function signature to send a 16 Bit message to the device and wait for the responce.

\param
   pDtiDevCtx        - points to the DTI Device context.
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
typedef IFX_int_t (*DTI_DEV_Message16Send_f)(
                        DTI_DeviceCtx_t      *pDtiDevCtx,
                        DTI_H2D_Message16_u  *pInMsg16Send,
                        DTI_D2H_Message16_u  *pOutMsg16Send,
                        IFX_int_t            lineNum,
                        IFX_int_t            inPaylSize_byte,
                        IFX_int_t            *pOutPaylSize_byte,
                        DTI_PacketError_t    *pPacketError);

/**
   Function signature to send a 32 Bit message to the device and wait for the responce.

\param
   pDtiDevCtx        - points to the DTI Device context.
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
typedef IFX_int_t (*DTI_DEV_Message32Send_f)(
                        DTI_DeviceCtx_t      *pDtiDevCtx,
                        DTI_H2D_Message32_u  *pInMsg32Send,
                        DTI_D2H_Message32_u  *pOutMsg32Send,
                        IFX_int_t            lineNum,
                        IFX_int_t            inPaylSize_byte,
                        IFX_int_t            *pOutPaylSize_byte,
                        DTI_PacketError_t    *pPacketError);

/**
   Function signature to setup the trace buffer configuration

\remark
   This function releases current configuration if the Debug Streams are
   already configured before the new config is set.

\param
   pDtiDevCtx           - points to the DTI Device context.
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
typedef IFX_int_t (*DTI_DEV_TraceBufferConfigSet_f)(
                        DTI_DeviceCtx_t            *pDtiDevCtx,
                        DTI_H2D_TraceConfigSet_t   *pInTraceConfigSet,
                        DTI_D2H_TraceConfigSet_t   *pOutTraceConfigSet,
                        IFX_int_t                  lineNum,
                        DTI_PacketError_t          *pPacketError);

/**
   Function signature to reset the current trace buffer

\param
   pDtiDevCtx     - points to the DTI Device context.
\param
   lineNum        - line number.
\param
   pPacketError   - return ptr, return the DTI Packet Error.

\return
   IFX_SUCCESS and returns the DTI Packet Error code.
   IFX_ERROR   debug stream setup failed.
*/
typedef IFX_int_t (*DTI_DEV_TraceBufferReset_f)(
                        DTI_DeviceCtx_t      *pDtiDevCtx,
                        IFX_int_t            lineNum,
                        DTI_PacketError_t    *pPacketError);

/**
   Function signature to setup the trace buffer configuration

\param
   pDtiDevCtx           - points to the DTI Device context.
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
typedef IFX_int_t (*DTI_DEV_TraceBufferStatusGet_f)(
                        DTI_DeviceCtx_t            *pDtiDevCtx,
                        DTI_D2H_TraceStatusGet_t   *pOutTraceStatusGet,
                        IFX_int_t                  lineNum,
                        DTI_PacketError_t          *pPacketError);

/**
   Function signature to read the trace buffer from the device

\param
   pDtiDevCtx           - points to the DTI Device context.
\param
   pInTraceBufferGet    - points to the DTI InTraceBufferFet struct.
\param
   ppUsedDtiPacketOut   - return pointer points to the location of the out buffer.
\param
   pUsedBufferOutSize   - size of the used out buffer.
\param
   lineNum              - line number.
\param
   pTrBufReadSize_byte  - number of read bytes.
\param
   pPacketError         - return ptr, return the DTI Packet Error.

\return
   IFX_SUCCESS and returns the DTI Packet Error code.
   IFX_ERROR   debug stream config request failed.
*/
typedef IFX_int_t (*DTI_DEV_TraceBufferGet_f)(
                        DTI_DeviceCtx_t            *pDtiDevCtx,
                        DTI_H2D_TraceBufferGet_t   *pInTraceBufferGet,
                        DTI_Packet_t               **ppUsedDtiPacketOut,
                        IFX_int_t                  *pUsedBufferOutSize,
                        IFX_int_t                  lineNum,
                        IFX_int_t                  *pTrBufReadSize_byte,
                        DTI_PacketError_t          *pPacketError);

/**
   Function signature to read data from the device via Debug Access.

\param
   pDtiDevCtx     - points to the DTI Device context.
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
typedef IFX_int_t (*DTI_DEV_DebugRead_f)(
                        DTI_DeviceCtx_t      *pDtiDevCtx,
                        DTI_H2D_DebugRead_t  *pInDbgGet,
                        DTI_D2H_DebugRead_t  *pOutDbgGet,
                        IFX_int_t            lineNum,
                        IFX_int_t            *pDbgReadCount,
                        DTI_PacketError_t    *pPacketError);

/**
   Function signature to write data to the device via Debug Access.

\param
   pDtiDevCtx     - points to the DTI Device context.
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
typedef IFX_int_t (*DTI_DEV_DebugWrite_f)(
                        DTI_DeviceCtx_t      *pDtiDevCtx,
                        DTI_H2D_DebugWrite_t *pInDbgSet,
                        IFX_uint32_t         *pOutDbgGet_nU,
                        IFX_int_t            lineNum,
                        IFX_int_t            *pDbgWriteCount,
                        DTI_PacketError_t    *pPacketError);

/**
   Function signature to check and process for Device Autonomous Messages.
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
typedef IFX_int_t (*DTI_DEV_AutoMsgProcess_f)(
                        DTI_DeviceCtx_t         *pDtiDevCtx,
                        const DTI_Connection_t  *pDtiCon,
                        IFX_uint32_t            devSelectWait_ms,
                        IFX_char_t              *pOutBuffer,
                        IFX_int_t               outBufferSize_byte);

/**
   Function signature to send a WinEasy packet.
   This is a wrapper function to allow WinEasy Debug access.

\param
   pDtiDevCtx     - points to the Device context.
\param
   lineNum        - line number of the WinEasy Device access.
\param
   pDataIn        - points to the IN buffer.
\param
   sizeIn         - size of the IN buffer.
\param
   pDataOut       - points to the OUT buffer.
\param
   sizeOut        - size of the OUT buffer.
\param
   pPacketError   - return ptr, return the DTI Packet Error.

\return
   IFX_SUCCESS if the DTI packet has been sent.
   IFX_ERROR   DTI packet has not been sent.
*/
typedef IFX_int_t (*DTI_DEV_WinEasyCiAccess_f)(
                        DTI_DeviceCtx_t      *pDtiDevCtx,
                        IFX_int_t            lineNum,
                        const IFX_uint8_t    *pDataIn,
                        const IFX_uint32_t   sizeIn,
                        IFX_uint8_t          *pDataOut,
                        const IFX_uint32_t   sizeOut,
                        DTI_PacketError_t    *pPacketError);

/**
   Collection of all available device access functions and device data.
*/
struct DTI_DeviceAccessFct_s
{
   /** for structure check */
   IFX_int_t         structSize_byte;
   /** device interface version */
   IFX_uint_t        deviceInterfaceVersion;
   /** Interface Name of this device interface */
   IFX_char_t        *pDevIfName;

   /** Fct Pointer: Set the printout level for debug and error printouts */
   DTI_DEV_PrintoutLevelSet_f       pPrintoutLevelSet;

   /** Fct Pointer: Setup the device specific context data */
   DTI_DEV_ModuleSetup_f            pModuleSetupFct;
   /** Fct Pointer: Delete the device specific context data */
   DTI_DEV_ModuleDelete_f           pModuleDeleteFct;
   /** Fct Pointer: Add the device specific system info */
   DTI_DEV_SystemInfoWrite_f        pSystemInfoWrite_deviceFct;

   /** Fct Pointer: Device Reset operation */
   DTI_DEV_Reset_f                  pResetFct;
   /** Fct Pointer: Device Download operation */
   DTI_DEV_Download_f               pDownloadFct;
   /** Fct Pointer: Device Open operation */
   DTI_DEV_DeviceOpen_f             pDeviceOpenFct;
   /** Fct Pointer: Device Close operation */
   DTI_DEV_DeviceClose_f            pDeviceCloseFct;
   /** Fct Pointer: Device Register Lock operation */
   DTI_DEV_RegisterLock_f           pRegisterLockFct;
   /** Fct Pointer: Device Register Get operation */
   DTI_DEV_RegisterGet_f            pRegisterGetFct;
   /** Fct Pointer: Device Register Set operation */
   DTI_DEV_RegisterSet_f            pRegisterSetFct;
   /** Fct Pointer: Device Config Set operation */
   DTI_DEV_ConfigSet_f              pConfigSetFct;
   /** Fct Pointer: Device Config Get operation */
   DTI_DEV_ConfigGet_f              pConfigGetFct;

   /* shiftet to the end:
   DTI_DEV_Message8Send_f           pMessage8SendFct; */

   /** Fct Pointer: Device 16 Bit Message Send operation */
   DTI_DEV_Message16Send_f          pMessage16SendFct;
   /** Fct Pointer: Device 32 Bit Message Send operation */
   DTI_DEV_Message32Send_f          pMessage32SendFct;
   /** Fct Pointer: Device Trace Buffer Config Set operation */
   DTI_DEV_TraceBufferConfigSet_f   pTraceBufferConfigSetFct;
   /** Fct Pointer: Device Trace Buffer Reset operation */
   DTI_DEV_TraceBufferReset_f       pTraceBufferResetFct;
   /** Fct Pointer: Device Trace Buffer Status Get operation */
   DTI_DEV_TraceBufferStatusGet_f   pTraceBufferStatusGetFct;
   /** Fct Pointer: Device Trace Buffer Get operation */
   DTI_DEV_TraceBufferGet_f         pTraceBufferGetFct;
   /** Fct Pointer: Device Debug Read operation */
   DTI_DEV_DebugRead_f              pDebugReadFct;
   /** Fct Pointer: Device Debug Write operation */
   DTI_DEV_DebugWrite_f             pDebugWriteFct;
   /** Fct Pointer: Device Autonomous Message Process operation */
   DTI_DEV_AutoMsgProcess_f         pAutoMsgProcessFct;
   /** Fct Pointer: Device Debug WinEasy Command Interface Access operation */
   DTI_DEV_WinEasyCiAccess_f        pWinEasyCiAccessFct;

   /** Fct Pointer: Device 8 Bit Message Send operation */
   DTI_DEV_Message8Send_f           pMessage8SendFct;
};


/* ==========================================================================
   exports
   ========================================================================== */

#ifdef __cplusplus
}
#endif

#endif /* _DTI_DEVICE_H */

