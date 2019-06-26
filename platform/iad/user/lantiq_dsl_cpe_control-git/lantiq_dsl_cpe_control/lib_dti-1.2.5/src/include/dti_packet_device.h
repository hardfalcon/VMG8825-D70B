#ifndef _DTI_PACKET_DEVICE_H
#define _DTI_PACKET_DEVICE_H
/******************************************************************************

                              Copyright (c) 2009
                            Lantiq Deutschland GmbH
                     Am Campeon 3; 85579 Neubiberg, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#ifdef __cplusplus
   extern "C" {
#endif

/** \file
   DTI agent device packet handling.
*/


/* ==========================================================================
   includes
   ========================================================================== */
#include "ifx_types.h"
#include "ifx_dti_protocol.h"
#include "ifx_dti_protocol_device.h"

#include "dti_device.h"



/* ==========================================================================
   CLI Control Defines
   ========================================================================== */

/* CLI Control, Device Group : 
   size of config settings: <lineNum> <0/1/2> (MBox / Max Reg Acc / Max Dbg Acc) */
#define DTI_CLI_CNTRL_GR_DEV_CFGSIZE_NUM        0
#define DTI_CLI_CNTRL_GR_DEV_CFGSIZE_NAME       "cfgsize"
#define DTI_CLI_CNTRL_GR_DEV_CFGSIZE_HELP       "<lineNum> <2/3/4>  (get size info, MBox/Reg/Dbg)"

#define DTI_CLI_CNTRL_GR_DEV_CFGSIZE            { DTI_CLI_CNTRL_GR_DEV_CFGSIZE_NUM, \
                                                  DTI_CLI_CNTRL_GR_DEV_CFGSIZE_NAME, \
                                                  DTI_CLI_CNTRL_GR_DEV_CFGSIZE_HELP}

/* CLI Control, Device Group: 
   enable NFC: <Line Num> <0/1> (ON / OFF) */
#define DTI_CLI_CNTRL_GR_DEV_NFC_NUM            1
#define DTI_CLI_CNTRL_GR_DEV_NFC_NAME           "nfc"
#define DTI_CLI_CNTRL_GR_DEV_NFC_HELP           "<lineNum> <0/1>    (switch NFC OFF/ON)"

#define DTI_CLI_CNTRL_GR_DEV_NFC                { DTI_CLI_CNTRL_GR_DEV_NFC_NUM, \
                                                  DTI_CLI_CNTRL_GR_DEV_NFC_NAME, \
                                                  DTI_CLI_CNTRL_GR_DEV_NFC_HELP}

/* CLI Control, Device Group: 
   line close <Line Num> <0/1> */
#define DTI_CLI_CNTRL_GR_DEV_LINE_CLOSE_NUM     2
#define DTI_CLI_CNTRL_GR_DEV_LINE_CLOSE_NAME    "lclose"
#define DTI_CLI_CNTRL_GR_DEV_LINE_CLOSE_HELP    "<lineNum>  (close given device)"

#define DTI_CLI_CNTRL_GR_DEV_LINE_CLOSE         { DTI_CLI_CNTRL_GR_DEV_LINE_CLOSE_NUM, \
                                                  DTI_CLI_CNTRL_GR_DEV_LINE_CLOSE_NAME, \
                                                  DTI_CLI_CNTRL_GR_DEV_LINE_CLOSE_HELP}

/* CLI Control, Device Group: 
   line open <Line Num> */
#define DTI_CLI_CNTRL_GR_DEV_LINE_OPEN_NUM      3
#define DTI_CLI_CNTRL_GR_DEV_LINE_OPEN_NAME     "lopen"
#define DTI_CLI_CNTRL_GR_DEV_LINE_OPEN_HELP     "<lineNum>  (open given device)"

#define DTI_CLI_CNTRL_GR_DEV_LINE_OPEN          { DTI_CLI_CNTRL_GR_DEV_LINE_OPEN_NUM, \
                                                  DTI_CLI_CNTRL_GR_DEV_LINE_OPEN_NAME, \
                                                  DTI_CLI_CNTRL_GR_DEV_LINE_OPEN_HELP}

/* all CLI Control Device Group commands */
#define DTI_CLI_CNTRL_GR_DEVICE_COMMANDS \
            DTI_CLI_CNTRL_GR_DEV_CFGSIZE, \
            DTI_CLI_CNTRL_GR_DEV_NFC, \
            DTI_CLI_CNTRL_GR_DEV_LINE_CLOSE, \
            DTI_CLI_CNTRL_GR_DEV_LINE_OPEN


/* ==========================================================================
   types
   ========================================================================== */

/* ==========================================================================
   exports
   ========================================================================== */

IFXOS_PRN_USR_MODULE_DECL(DTI_DEV);

extern IFX_int_t DTI_packetHandler_device(
                        DTI_DeviceCtx_t         *pDtiDevCtx,
                        DTI_ProtocolServerCtx_t *pDtiProtServerCtx,
                        const DTI_Packet_t      *pDtiPacketIn,
                        DTI_Packet_t            *pDtiPacketOut,
                        IFX_uint_t              dtiBufOutLen);

extern IFX_int_t DTI_device_configSet(
                        DTI_DeviceCtx_t            *pDtiDevCtx,
                        DTI_H2D_DeviceConfigSet_t  *pInCfgSet,
                        DTI_D2H_DeviceConfigSet_t  *pOutCfgSet,
                        IFX_int_t                  lineNum,
                        DTI_PacketError_t          *pPacketError);

extern IFX_int_t DTI_device_configGet(
                        DTI_DeviceCtx_t            *pDtiDevCtx,
                        DTI_H2D_DeviceConfigGet_t  *pInCfgGet,
                        DTI_D2H_DeviceConfigGet_t  *pOutCfgGet,
                        IFX_int_t                  lineNum,
                        DTI_PacketError_t          *pPacketError);

extern IFX_int_t DTI_device_devClose(
                        DTI_DeviceCtx_t            *pDtiDevCtx,
                        IFX_int_t                  lineNum,
                        DTI_PacketError_t          *pPacketError);

extern IFX_int_t DTI_device_devOpen(
                        DTI_DeviceCtx_t            *pDtiDevCtx,
                        IFX_int_t                  lineNum,
                        DTI_PacketError_t          *pPacketError);

extern IFX_int_t DTI_device_autoMsgProcess(
                        DTI_DeviceCtx_t         *pDtiDevCtx,
                        const DTI_Connection_t  *pDtiCon,
                        IFX_uint32_t            devSelectWait_ms,
                        IFX_char_t              *pOutBuffer, 
                        IFX_int_t               outBufferSize_byte);

extern IFX_int_t DTI_device_moduleStart(
                        DTI_DeviceAccessFct_t   *pDeviceAccessFct,
                        DTI_DeviceSysInfo_t     *pDeviceSystemInfo,
                        DTI_DeviceCtx_t         **ppDtiDevCtx);

extern IFX_int_t DTI_device_moduleStop(
                        DTI_DeviceAccessFct_t   *pDeviceAccessFct,
                        DTI_DeviceSysInfo_t     *pDeviceSystemInfo,
                        DTI_DeviceCtx_t         **ppDtiDevCtx);


extern IFX_int_t DTI_device_systemInfoWrite(
                        DTI_DeviceAccessFct_t   *pDeviceAccessFct,
                        DTI_DeviceSysInfo_t     *pDeviceSystemInfo,
                        IFX_char_t              *pSysInfoBuffer,
                        IFX_int_t               bufferSize);


#ifdef __cplusplus
}
#endif

#endif /* _DTI_PACKET_DEVICE_H */
