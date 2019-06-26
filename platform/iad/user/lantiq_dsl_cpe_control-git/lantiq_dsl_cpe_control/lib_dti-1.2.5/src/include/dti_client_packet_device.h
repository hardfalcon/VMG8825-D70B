#ifndef _DTI_CLIENT_PACKET_DEVICE_H
#define _DTI_CLIENT_PACKET_DEVICE_H
/******************************************************************************

                              Copyright (c) 2009
                            Lantiq Deutschland GmbH
                     Am Campeon 3; 85579 Neubiberg, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/** \file
   DTI Client Protocol library API.
*/

/** \defgroup DTI_CLIENT_DEVICE_IF DTI Client Device Interface

   This Group contains the definitions and functions of the DTI device Layer
   used on client side.

   For the device access this module provides functions to setup the
   corresponding DTI packet for
   - device control and management (like reset, FW download, configuration)
   - Device Register access
   - Message handling
   - Device Debug access
   - Trace buffer handling (VINAX only)
*/

#ifdef __cplusplus
   extern "C" {
#endif

/* ============================================================================
   Includes
   ========================================================================= */

#include "ifx_types.h"
#include "ifx_dti_protocol.h"

/* ============================================================================
   Macro Defines
   ========================================================================= */

/** \addtogroup DTI_CLIENT_DEVICE_IF
   *{ */

#define DTI_VINAX_IFX_MSG_INDICATION   0x0010


/* ============================================================================
   Type Defines
   ========================================================================= */


/* ============================================================================
   Exports
   ========================================================================= */
/**
   Setup Device Packet "DTI_PacketType_eDeviceReset"

\param
   pDtiPacketSend    - points to the IN packet.
\param
   paylBufferSize    - payload size of the IN packet.
\param
   mode              - reset mode (0: default, 1: soft reset, 2: hard reset).
\param
   mask              - device mask contains selected devices for reset.

\return
   IFX_SUCCESS setup done.
   IFX_ERROR   missing args or missaligned payload.
*/
extern IFX_int_t DTI_CLIENT_packetDeviceReset_Set(
                        DTI_Packet_t   *pDtiPacketSend,
                        IFX_uint_t     paylBufferSize,
                        IFX_uint32_t   mode,
                        IFX_uint32_t   mask);

/**
   Setup Device Packet "DTI_PacketType_eDeviceDownload"

\param
   pDtiPacketSend    - points to the IN packet.
\param
   paylBufferSize    - payload size of the IN packet.
\param
   imageNum          - identify the image on the target.
\param
   mode              - dl mode (0: firmware, 1: cache) - not used on VINAX Systems
\param
   resetMask         - device mask contains selected devices for reset.
\param
   loadMask         - device mask contains selected devices for download.

\return
   IFX_SUCCESS setup done.
   IFX_ERROR   missing args or missaligned payload.
*/
extern IFX_int_t DTI_CLIENT_packetDeviceDownload_Set(
                        DTI_Packet_t   *pDtiPacketSend,
                        IFX_uint_t     paylBufferSize,
                        IFX_uint32_t   imageNum,
                        IFX_uint32_t   mode,
                        IFX_uint32_t   resetMask,
                        IFX_uint32_t   loadMask);

/**
   Setup Device Packet "DTI_PacketType_eDeviceOpen"

\param
   pDtiPacketSend   - points to the IN packet.
\param
   paylBufferSize    - payload size of the IN packet.

\return
   IFX_SUCCESS setup done.
   IFX_ERROR   missing args or missaligned payload.
*/
extern IFX_int_t DTI_CLIENT_packetDeviceOpen_Set(
                        DTI_Packet_t   *pDtiPacketSend,
                        IFX_uint_t     paylBufferSize);

/**
   Setup Device Packet "DTI_PacketType_eDeviceClose"

\param
   pDtiPacketSend   - points to the IN packet.
\param
   paylBufferSize    - payload size of the IN packet.

\return
   IFX_SUCCESS setup done.
   IFX_ERROR   missing args or missaligned payload.
*/
extern IFX_int_t DTI_CLIENT_packetDeviceClose_Set(
                        DTI_Packet_t   *pDtiPacketSend,
                        IFX_uint_t     paylBufferSize);

/**
   Setup Device Packet "DTI_PacketType_eRegisterLock"

\param
   pDtiPacketSend   - points to the IN packet.
\param
   paylBufferSize    - payload size of the IN packet.
\param
   lockOnOff         - 1: lock the driver, 0: unlock the driver.

\return
   IFX_SUCCESS setup done.
   IFX_ERROR   missing args or missaligned payload.
*/
extern IFX_int_t DTI_CLIENT_packetLock_Set(
                        DTI_Packet_t   *pDtiPacketSend,
                        IFX_uint_t     paylBufferSize,
                        IFX_uint_t     lockOnOff);

/**
   Setup Device Packet "DTI_PacketType_eRegisterSet"

\param
   pDtiPacketSend   - points to the IN packet.
\param
   paylBufferSize    - payload size of the IN packet.
\param
   regAddr           - register address for get.
\param
   pRegData          - register data to write.
\param
   regCount_32       - number of register to write.

\remark:
   There are data registers which provide an read/write access to underlaying
   HW resources like DMA. Read from / write to such registers also triggers an
   auto-increment of the corresponding address.
   On such registers a multiple read from / write to perform a HW supported
   access to the internal resources.

\return
   IFX_SUCCESS setup done.
   IFX_ERROR   missing args or missaligned payload.
*/
extern IFX_int_t DTI_CLIENT_packetRegisterSet_Set(
                        DTI_Packet_t   *pDtiPacketSend,
                        IFX_uint_t     paylBufferSize,
                        IFX_uint32_t   regAddr,
                        IFX_uint32_t   *pRegData,
                        IFX_uint_t     regCount_32);

/**
   Setup Device Packet "DTI_PacketType_eRegisterGet"

\param
   pDtiPacketSend    - points to the IN packet.
\param
   paylBufferSize    - payload size of the IN packet.
\param
   regAddr           - register address for get.
\param
   regCount          - number of register read counts.

\remark:
   There are data registers which provide an read/write access to underlaying
   HW resources like DMA. Read from / write to such registers also triggers an
   auto-increment of the corresponding address.
   On such registers a multiple read from / write to perform a HW supported
   access to the internal resources.

\return
   IFX_SUCCESS setup done.
   IFX_ERROR   missing args or missaligned payload.
*/
extern IFX_int_t DTI_CLIENT_packetRegisterGet_Set(
                        DTI_Packet_t   *pDtiPacketSend,
                        IFX_uint_t     paylBufferSize,
                        IFX_uint32_t   regAddr,
                        IFX_uint_t     regCount);

/**
   Setup Device Packet "DTI_PacketType_eConfigSet"

\param
   pDtiPacketSend   - points to the IN packet.
\param
   paylBufferSize    - payload size of the IN packet.
\param
   key               - indentifies the config setting to set on the target.
\param
   keyValue          - new config value

\return
   IFX_SUCCESS setup done.
   IFX_ERROR   missing args or missaligned payload.
*/
extern IFX_int_t DTI_CLIENT_packetConfigSet_Set(
                        DTI_Packet_t   *pDtiPacketSend,
                        IFX_uint_t     paylBufferSize,
                        IFX_uint32_t   key,
                        IFX_uint32_t   keyValue);

/**
   Setup Device Packet "DTI_PacketType_eConfigGet"

\param
   pDtiPacketSend   - points to the IN packet.
\param
   paylBufferSize    - payload size of the IN packet.
\param
   key               - indentifies the config setting to get from the target.

\return
   IFX_SUCCESS setup done.
   IFX_ERROR   missing args or missaligned payload.
*/
extern IFX_int_t DTI_CLIENT_packetConfigGet_Set(
                        DTI_Packet_t   *pDtiPacketSend,
                        IFX_uint_t     paylBufferSize,
                        IFX_uint32_t   key);

/**
   Setup Device Packet "DTI_PacketType_eMessageSend" (VINAX)

\param
   pDtiPacketSend   - points to the IN packet.
\param
   paylBufferSize    - payload size of the IN packet.
\param
   msgId             - msg ID, unique msg identifier.
\param
   msgIndex          - msg index field, VINAX message payload (see msg cat)
\param
   msgLength         - msg length field, VINAX message payload (see msg cat)
\param
   pUsrMsgData       - points to the user data, further payload arguments
\param
   usrMsgDataSize_byte  - size of the user data.

\return
   IFX_SUCCESS setup done.
   IFX_ERROR   missing args or missaligned payload.
*/
extern IFX_int_t DTI_CLIENT_packetVnxMessageSend_Set(
                        DTI_Packet_t   *pDtiPacketSend,
                        IFX_uint_t     paylBufferSize,
                        IFX_uint16_t   msgId,
                        IFX_uint16_t   msgIndex,
                        IFX_uint16_t   msgLength,
                        IFX_uint8_t    *pUsrMsgData,
                        IFX_uint_t     usrMsgDataSize_byte);

/**
   Setup Device Packet "DTI_PacketType_eMessageSend"

\param
   pDtiPacketSend   - points to the IN packet.
\param
   paylBufferSize    - payload size of the IN packet.
\param
   cmdWord           - msg ID, unique msg identifier.
\param
   lenField          - msg index field, VINAX message payload (see msg cat)
\param
   pUsrMsgData       - points to the user data, further payload arguments
\param
   usrMsgDataSize_byte  - size of the user data.

\return
   IFX_SUCCESS setup done.
   IFX_ERROR   missing args or missaligned payload.
*/
extern IFX_int_t DTI_CLIENT_packetGmxMessageSend_Set(
                        DTI_Packet_t   *pDtiPacketSend,
                        IFX_uint_t     paylBufferSize,
                        IFX_uint16_t   cmdWord,
                        IFX_uint16_t   lenField,
                        IFX_uint8_t    *pUsrMsgData,
                        IFX_uint_t     usrMsgDataSize_byte);


/**
   Setup Device Packet "DTI_PacketType_eMessageSend"

\param
   pDtiPacketSend   - points to the IN packet.
\param
   paylBufferSize    - payload size of the IN packet.
\param
   payloadType       - payload type - mixed, 8, 16, 32 bit message.
\param
   pUsrMsgData       - points to the user data, further payload arguments
\param
   usrMsgDataSize_byte  - size of the user data.

\return
   IFX_SUCCESS setup done.
   IFX_ERROR   missing args or missaligned payload.
*/
extern IFX_int_t DTI_CLIENT_packetMessageSend_Set(
                        DTI_Packet_t   *pDtiPacketSend,
                        IFX_uint_t     paylBufferSize,
                        IFX_uint_t     payloadType,
                        IFX_uint8_t    *pUsrMsgData,
                        IFX_uint_t     usrMsgDataSize_byte);

/**
   Setup Device Packet "DTI_PacketType_eTraceBufferConfigSet"

\param
   pDtiPacketSend    - points to the IN packet.
\param
   paylBufferSize    - payload size of the IN packet.
\param
   operationMode     - operation mode of the underlaying debug stream feature
                       0: disable
                       1: linear (fill), 2: circular (ring), 3: fifo
\param
   bufferSize_byte   - used buffer size the underlaying debug stream feature (driver).
\param
   nfcThreshold      - currently not used.

\return
   IFX_SUCCESS setup done.
   IFX_ERROR   missing args or missaligned payload.
*/
extern IFX_int_t DTI_CLIENT_packetTraceBufferCfgSet_Set(
                        DTI_Packet_t   *pDtiPacketSend,
                        IFX_uint_t     paylBufferSize,
                        IFX_uint32_t   operationMode,
                        IFX_uint32_t   bufferSize_byte,
                        IFX_uint32_t   nfcThreshold);

/**
   Setup Device Packet "DTI_PacketType_eTraceBufferReset"

\param
   pDtiPacketSend   - points to the IN packet.
\param
   paylBufferSize    - payload size of the IN packet.

\return
   IFX_SUCCESS setup done.
   IFX_ERROR   missing args or missaligned payload.
*/
extern IFX_int_t DTI_CLIENT_packetTraceBufferReset_Set(
                        DTI_Packet_t   *pDtiPacketSend,
                        IFX_uint_t     paylBufferSize);

/**
   Setup Device Packet "DTI_PacketType_eTraceBufferStatusGet"

\param
   pDtiPacketSend   - points to the IN packet.
\param
   paylBufferSize    - payload size of the IN packet.

\return
   IFX_SUCCESS setup done.
   IFX_ERROR   missing args or missaligned payload.
*/
extern IFX_int_t DTI_CLIENT_packetTraceBufferStatGet_Set(
                        DTI_Packet_t   *pDtiPacketSend,
                        IFX_uint_t     paylBufferSize);

/**
   Setup Device Packet "DTI_PacketType_eTraceBufferGet"

\param
   pDtiPacketSend   - points to the IN packet.
\param
   paylBufferSize    - payload size of the IN packet.
\param
   readSize_byte     - number of bytes to read.

\return
   IFX_SUCCESS setup done.
   IFX_ERROR   missing args or missaligned payload.
*/
extern IFX_int_t DTI_CLIENT_packetTraceBufferGet_Set(
                        DTI_Packet_t   *pDtiPacketSend,
                        IFX_uint_t     paylBufferSize,
                        IFX_uint32_t   readSize_byte);

/**
   Setup Device Packet "DTI_PacketType_eDebugGet"

\param
   pDtiPacketSend   - points to the IN packet.
\param
   paylBufferSize    - payload size of the IN packet.
\param
   offset            - offset / destination address
\param
   destType          - destination type
                       there are different destinations like AUX, CORE, MEM)
\param
   count_32          - number of 32 bit units to read.

\return
   IFX_SUCCESS setup done.
   IFX_ERROR   missing args or missaligned payload.
*/
extern IFX_int_t DTI_CLIENT_packetDebugGet_Set(
                        DTI_Packet_t   *pDtiPacketSend,
                        IFX_uint_t     paylBufferSize,
                        IFX_uint32_t   offset,
                        IFX_uint_t     destType,
                        IFX_uint_t     count_32);

/**
   Setup Device Packet "DTI_PacketType_eDebugSet"

\param
   pDtiPacketSend   - points to the IN packet.
\param
   paylBufferSize    - payload size of the IN packet.
\param
   offset            - offset / destination address
\param
   destType          - destination type
                       there are different destinations like AUX, CORE, MEM)
\param
   pDbgData          - points to the data to write.
\param
   count_32          - number of 32 bit units to write.

\return
   IFX_SUCCESS setup done.
   IFX_ERROR   missing args or missaligned payload.
*/
extern IFX_int_t DTI_CLIENT_packetDebugSet_Set(
                        DTI_Packet_t   *pDtiPacketSend,
                        IFX_uint_t     paylBufferSize,
                        IFX_uint32_t   offset,
                        IFX_uint_t     destType,
                        IFX_uint32_t   *pDbgData,
                        IFX_uint_t     count_32);
/** *} */

#ifdef __cplusplus
}
#endif

#endif /* _DTI_CLIENT_PACKET_DEVICE_H */
