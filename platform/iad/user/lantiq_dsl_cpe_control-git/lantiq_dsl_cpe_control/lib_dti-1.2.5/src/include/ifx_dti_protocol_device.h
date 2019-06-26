#ifndef _IFX_DTI_PROTOCOL_DEVICE_H
#define _IFX_DTI_PROTOCOL_DEVICE_H
/******************************************************************************

                              Copyright (c) 2009
                            Lantiq Deutschland GmbH
                     Am Campeon 3; 85579 Neubiberg, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/** \file
   Debug and Trace Interface - Protocol definitions and declarations (device part).
*/

/** \defgroup DTI_PROTOCOL_DEVICE DTI Protocol (Device Part)

   This Group contains definitions of the device specific part of the Debug and
   Trace Interface (DTI) Protocol.

   Wihtin this header the device specific definitions are defined, this are:
   - DTI Device specific message types and options.
   - DTI Device specific packet structures.

\ingroup DTI_PROTOCOL
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
   Types
   ========================================================================= */

/** \addtogroup DTI_PROTOCOL_DEVICE
@{ */

/**
   Possible device packet types
*/
typedef enum {
/*
   Packet Type: 0x0001xxxx - Low Level HW Access
*/
/** Device Reset */
   DTI_PacketType_eDeviceReset          = 0x00010001,
/** Device Download */
   DTI_PacketType_eDeviceDownload       = 0x00010002,

/** Device Open */
   DTI_PacketType_eDeviceOpen           = 0x00010003,
/** Device Close */
   DTI_PacketType_eDeviceClose          = 0x00010004,

/** Lock / Unlock Device */
   DTI_PacketType_eRegisterLock         = 0x00010020,
/** Read Device Register */
   DTI_PacketType_eRegisterGet          = 0x00010021,
/** Write Device Register */
   DTI_PacketType_eRegisterSet          = 0x00010022,

/*
   Packet Type: 0x0002xxxx - Control and Message Interface
*/
/** Set Configuration Option */
   DTI_PacketType_eConfigSet            = 0x00020001,
/** Get Configuration Option */
   DTI_PacketType_eConfigGet            = 0x00020002,
/** Message Send / Message Receive  */
   DTI_PacketType_eMessageSend          = 0x00020003,
/** Message Error */
   DTI_PacketType_eMessageError         = 0x00020004,

/*
   Packet Type: 0x0003xxxx - Trace Buffer Access
*/
/** Configure Trace Buffer */
   DTI_PacketType_eTraceBufferConfigSet = 0x00030001,
/** Reset Trace Buffer */
   DTI_PacketType_eTraceBufferReset     = 0x00030002,
/** Get Trace Buffer Status */
   DTI_PacketType_eTraceBufferStatusGet = 0x00030003,
/** Trace Buffer Read */
   DTI_PacketType_eTraceBufferGet       = 0x00030004,
/** Trace Buffer Notification */
   DTI_PacketType_eTraceBufferAvailable = 0x00030005,

/*
   Packet Type: 0x0004xxxx - Debug Register Access
*/
/** Debug Read Access */
   DTI_PacketType_eDebugGet             = 0x00040001,
/** Debug Write Access */
   DTI_PacketType_eDebugSet             = 0x00040002,

/*
   Packet Type: 0x0006xxxx - WinEasy Access
*/
/** WinEasy C/I Access */
   DTI_PacketType_eWinEasyCiAccess      = 0x00060001

} DTI_PacketTypeDevice_t;


/* ============================================================================
   Packet Type: 0x0001xxxx - Low Level HW Access
   ========================================================================= */

/**
   Device Reset - reset modes
*/
typedef enum {
   /** Default Reset Mode - do both soft and HW if available for the device */
   DTI_eRstDefault = 0,
   /** Soft Reset Mode */
   DTI_eRstSoftReset = 1,
   /** Soft Reset Mode */
   DTI_eRstHardReset = 2
} DTI_DeviceResetModes_t;

/** Device Reset.

   Packet type: \ref  DTI_PacketType_eDeviceReset .

   When this packet is send to the board the devices selected with a one in the masks will be reset.
   Depending on the mode there will be a hard reset or soft reset be performed.

   \remarks In case the requested reset mode is not supported, an error indication is returned.
   The default reset mode is the one which is usually used for a firmware download.
*/
typedef struct {
   /** The reset mode e.g. default(=0), softreset (=1) or hardreset (=2) */
   IFX_uint32_t mode;
   /** A mask value to select devices. If there are more than 32 devices additional mask value are
       appended. LSB corresponds with device 0. */
   IFX_uint32_t mask[1];
} DTI_H2D_DeviceReset_t;

/** Device Reset.

   Packet type: \ref  DTI_PacketType_eDeviceReset .

   The board will reply the status of the reset. A successful device reset will be marked with 1 in the mask field.
*/
typedef DTI_H2D_DeviceReset_t DTI_D2H_DeviceReset_t;

/**
   Device Download Command - performs a download on the selected devices with the
   selected and previous provided image.


*/
typedef struct {
   /** Identify the image used for this download command */
   IFX_uint32_t imageNum;
   /** The download type. Could be eFirmware(=0), eCache(=1) */
   IFX_uint32_t mode;

   /** Bitmask to reset devices before the firmware is loaded.
       Will be ignored if not supported by target device.
       A one selects a device. The first device (0) is located in the LSB. */
   IFX_uint32_t resetMask;

   /** Bitmask to select devices for image load. A one selects a device.
       The first device (0) is located in the LSB. */
   IFX_uint32_t loadMask;

} DTI_H2D_DeviceDownload_t;

/** Device Download Command - informs the host about a failed download operation
    per device.

*/
typedef struct {
   /** Bitmask with a one for every device which failed in the download procedure.
       The first device (=0) is located in the LSB. */
   IFX_uint32_t errorMask;
} DTI_D2H_DeviceDownload_t;

/** Device Lock / Unlock.

   Packet type: \ref  DTI_PacketType_eRegisterLock .

   This command is used to restrict any other instance from communication to the device. This is necessary to avoid
   any corruption of the register accesses by a different instance.
*/
typedef struct {
   /** Unlock(=0) or Lock(=1) the device */
   IFX_uint32_t lock;
} DTI_H2D_DeviceLock_t;

/** Device Lock / Unlock.

   Packet type: \ref  DTI_PacketType_eRegisterLock .

   The lock element will indicate the status of the device (locked / unlocked).
*/
typedef DTI_H2D_DeviceLock_t DTI_D2H_DeviceLock_t;

/** Read Device Register.

   Packet type: \ref  DTI_PacketType_eRegisterGet .

   This command reads a device register a number of times. Reading the same register a number of times is used for   certain device register which have an internal automatic address increment.
*/
typedef struct {
   /** Register address of the device */
   IFX_uint32_t address;
   /** Number of  times the registers should be read.
       If count is greater than the maximum number of reads supported by the DTI implementation an error is generated. */
   IFX_uint32_t count;
} DTI_H2D_RegisterGet_t;

/** Read Device Register.

   Packet type: \ref  DTI_PacketType_eRegisterGet .

   The requested data. If the register width is lower than 32 bit, the register content is stored in the LSBs of data and
   the upper bits are set to zero.
*/
typedef struct {
   IFX_uint32_t data[1];
} DTI_D2H_RegisterGet_t;

/** Write Device Register.

   Packet type: \ref  DTI_PacketType_eRegisterSet .

   This command writes a device register a number of times. Writing the same register a number of times is used for certain
   device register which have an internal automatic address increment.
*/
typedef struct {
   /** Register address of the device */
   IFX_uint32_t address;
   /** The data to write.
        If the number of  the data items exceeds the supported value of the implementation, none of the writes is executed and
        an error indication is generated. If the register width is lower than 32 bit, the LSBs of data are written to the register
        and the upper bits are discarded. */
   IFX_uint32_t data[1];
} DTI_H2D_RegisterSet_t;




/* ============================================================================
   Packet Type: 0x0002xxxx - Control and Message Interface
   ========================================================================= */

/**
   Possible configuration options
*/
typedef enum {
   /** Configures a new timeout value in ms. R/W */
   DTI_eTimeout = 0,
   /** Disables (0) or enables(1) the autonomous message transport. R/W */
   DTI_eAutonousMessages = 1,
   /** The total mailbox size in number of bytes. */
   DTI_eMailboxSize = 2,
   /** Maximum number of register read/writes within one package. */
   DTI_eMaxRegAccess = 3,
   /** Maximum number of debug read/writes within one package. */
   DTI_eMaxDebugAccess = 4
} DTI_DeviceConfigType_t;


/** Device Configuration Set.

   Packet type: \ref  DTI_PacketType_eConfigSet .

   This packet changes a configuration option.
*/
typedef struct {
   /** The parameter to be configured (DTI_DeviceConfigType_t). */
   IFX_uint32_t key;
   /** The new value. */
   IFX_uint32_t value;
} DTI_H2D_DeviceConfigSet_t;

/** Configuration Set.

   Packet type: \ref  DTI_PacketType_eConfigSet .

   This packet changes a configuration option.
 */
typedef DTI_H2D_DeviceConfigSet_t DTI_D2H_DeviceConfigSet_t;

/** Configuration Retrieval.

   Packet type: \ref  DTI_PacketType_eConfigGet .

*/
typedef struct {
   /** The parameter to be read. (DTI_DeviceConfigType_t) */
   IFX_uint32_t key;
} DTI_H2D_DeviceConfigGet_t;

/** Configuration Retrieval.

   Packet type: \ref  DTI_PacketType_eConfigGet .

   This packet read a configuration option.
 */
typedef struct {
   /** The parameter to be read. (DTI_ConfigType_t) */
   IFX_uint32_t key;
   /** The configured value. */
   IFX_uint32_t value;
} DTI_D2H_DeviceConfigGet_t;



/** RAW 8 Bit Message Send from the host to the device.

   Packet type: \ref  DTI_PacketType_eMessageSend .
*/
typedef struct {
   /** Message payload data. */
   IFX_uint8_t data[1];
} DTI_H2D_rawMessage8_t;

/** RAW 8 Bit Message Send Responce from the device to the host.

   Packet type: \ref  DTI_PacketType_eMessageSend .
*/
typedef struct {
   /** The result code of the send operation */
   IFX_uint32_t sendResult;
   /** Message payload data. */
   IFX_uint8_t data[1];
} DTI_D2H_rawMessage8_t;


/** VINAX 16 Bit Message Send from the host to the device.

   Packet type: \ref  DTI_PacketType_eMessageSend .
*/
typedef struct {
   /** The message identifier */
   IFX_uint16_t msgid;
   /** Parameter index for the data. */
   IFX_uint16_t index;
   /** number of 16bit payload elements. */
   IFX_uint16_t length;
   /** Message payload data. */
   IFX_uint16_t data[1];
} DTI_H2D_vnxMessage16_t;

/** GEMINAX 16 Bit Message Send from the host to the device.

   Packet type: \ref  DTI_PacketType_eMessageSend .
*/
typedef struct {
   /** The message identifier */
   IFX_uint16_t cmdWord;
   /** Parameter index for the data. */
   IFX_uint16_t lenField;
   /** Message payload data. */
   IFX_uint16_t data[1];
} DTI_H2D_gmxMessage16_t;

/** RAW 16 Bit Message Send from the host to the device.

   Packet type: \ref  DTI_PacketType_eMessageSend .
*/
typedef struct {
   /** Message payload data. */
   IFX_uint16_t data[1];
} DTI_H2D_rawMessage16_t;

/** VINAX 16 Bit Message Send Responce from the device to the host.

   Packet type: \ref  DTI_PacketType_eMessageSend .
*/
typedef struct {
   /** The result code of the send operation */
   IFX_uint16_t sendResult;
   /** The message identifier */
   IFX_uint16_t msgid;

   /* !! take care that the the following data are 32 Bit aligned !! */

   /** Parameter index for the data. */
   IFX_uint16_t index;
   /** number of 16bit payload elements. */
   IFX_uint16_t length;
   /** Message payload data. */
   IFX_uint16_t data[1];
} DTI_D2H_vnxMessage16_t;

/** GEMINAX 16 Bit Message Send Responce from the device to the host.

   Packet type: \ref  DTI_PacketType_eMessageSend .
*/
typedef struct {
   /** The result code of the send operation */
   IFX_uint16_t sendResult;
   /** The message identifier */
   IFX_uint16_t cmdWord;
   /** Parameter index for the data. */
   IFX_uint16_t lenField;
   /** Message payload data. */
   IFX_uint16_t data[1];
} DTI_D2H_gmxMessage16_t;

/** RAW 16 Bit Message Send Responce from the device to the host.

   Packet type: \ref  DTI_PacketType_eMessageSend .
*/
typedef struct {
   /** The result code of the send operation */
   IFX_uint16_t sendResult;
   /** Message payload data. */
   IFX_uint16_t data[1];
} DTI_D2H_rawMessage16_t;

/** VINAX 32 Bit Message Send from the host to the device.

   Packet type: \ref  DTI_PacketType_eMessageSend .
*/
typedef struct {
   /** The message identifier */
   IFX_uint32_t msgid;
   /** Parameter index for the data. */
   IFX_uint32_t index;
   /** Size of data in number of bytes. */
   IFX_uint32_t length;
   /** Message payload data. */
   IFX_uint32_t data[1];
} DTI_H2D_vnxMessage32_t;

/** RAW 32 Bit Message Send from the host to the device.

   Packet type: \ref  DTI_PacketType_eMessageSend .
*/
typedef struct {
   /** Message payload data. */
   IFX_uint32_t data[1];
} DTI_H2D_rawMessage32_t;

/** VINAX 32 Bit Message Send Responce from the device to the host.

   Packet type: \ref  DTI_PacketType_eMessageSend .
*/
typedef struct {
   /** The result code of the send operation */
   IFX_uint32_t sendResult;
   /** The message identifier */
   IFX_uint32_t msgid;
   /** Parameter index for the data. */
   IFX_uint32_t index;
   /** Size of data in number of bytes. */
   IFX_uint32_t length;
   /** Message payload data.
   */
   IFX_uint32_t data[1];
} DTI_D2H_vnxMessage32_t;

/** RAW 32 Bit Message Send Responce from the device to the host.

   Packet type: \ref  DTI_PacketType_eMessageSend .
*/
typedef struct {
   /** The result code of the send operation */
   IFX_uint32_t sendResult;
   /** Message payload data. */
   IFX_uint32_t data[1];
} DTI_D2H_rawMessage32_t;


/** 8 Bit Message Union, Send from the host to the device.

   Packet type: \ref  DTI_PacketType_eMessageSend .

*/
typedef union {
   /** RAW message struct */
   DTI_H2D_rawMessage8_t raw;
} DTI_H2D_Message8_u;

/** 8 Bit Message Union, Send Responce from the device to the host.

   Packet type: \ref  DTI_PacketType_eMessageSend .
*/
typedef union {
   /** RAW message struct */
   DTI_D2H_rawMessage8_t raw;
} DTI_D2H_Message8_u;

/** 16 Bit Message Union, Send from the host to the device.

   Packet type: \ref  DTI_PacketType_eMessageSend .
*/
typedef union {
   /** RAW message struct */
   DTI_H2D_rawMessage16_t raw;
   /** VINAX message struct */
   DTI_H2D_vnxMessage16_t vnx;
   /** GEMINAX message struct */
   DTI_H2D_gmxMessage16_t gmx;
} DTI_H2D_Message16_u;

/** 16 Bit Message Union, Send Responce from the device to the host.

   Packet type: \ref  DTI_PacketType_eMessageSend .
*/
typedef union {
   /** RAW message struct */
   DTI_D2H_rawMessage16_t raw;
   /** VINAX message struct */
   DTI_D2H_vnxMessage16_t vnx;
   /** GEMINAX message struct */
   DTI_D2H_gmxMessage16_t gmx;
} DTI_D2H_Message16_u;

/** 32 Bit Message Union, Send from the host to the device.

   Packet type: \ref  DTI_PacketType_eMessageSend .
*/
typedef union {
   /** RAW message struct */
   DTI_H2D_rawMessage32_t raw;
   /** VINAX message struct */
   DTI_H2D_vnxMessage32_t vnx;
} DTI_H2D_Message32_u;

/** 32 Bit Message Union, Send Responce from the device to the host.

   Packet type: \ref  DTI_PacketType_eMessageSend .
*/
typedef union {
   /** RAW message struct */
   DTI_D2H_rawMessage32_t raw;
   /** VINAX message struct */
   DTI_D2H_vnxMessage32_t vnx;
} DTI_D2H_Message32_u;


/* ============================================================================
   Packet Type: 0x0003xxxx - Trace Buffer Access
   ========================================================================= */

/**
   Possible trace buffer modes
*/
typedef enum {
   /** trace buffer disabled */
   DTI_eDisabled = 0,
   /** trace buffer filled linear */
   DTI_eLinear = 1,
   /** trace buffer filled in circular way */
   DTI_eCircular = 2,
   /** trace buffer behaves like a FIFO */
   DTI_eFifo = 3
} DTI_TraceBufferMode_t;

/** Trace Configuration Set.

   Packet type: \ref  DTI_PacketType_eTraceBufferConfigSet .

   This packet is used to configure the trace buffer. A linear buffer is filled up with the debug data.
   Once the first message does not fit to the debug buffer, any further messages are discarded.
   A �ResetBuffer� has to be performed to reset this condition. A circular buffer will overwrite old data.
   An efficient implementation should be done to avoid modulo addressing on byte level. At least a message
   level modulo implementation is assumed. Further details depend on the target processor. During reading
   of the trace buffer content any further messages may be discarded. FiFo mode is similar to circular mode.
   But in case of an overflow an error indication is written to the FIFO.
   If a message does not fit to the buffer, the complete message is discarded. There are no partial messages
   in the buffer. The last trace buffer configuration remains active even if the debugging connection is closed.
*/
typedef struct {
   /** Buffer mode. DTI_TraceBufferMode_t can be eDisabled, eLinear, eCircular or eFifo */
   IFX_uint32_t mode;
   /** The wanted buffer size in bytes.  */
   IFX_uint32_t size;
   /** Notification threshold in bytes.
   If the trace buffer contains at least NfcThreshold bytes, a notification message is send once until
   the buffer is reset or read.
   0xffffffff turns off this feature. */
   IFX_uint32_t nfcThreshold;
} DTI_H2D_TraceConfigSet_t;

/** Trace Configuration Set.

   Packet type: \ref  DTI_PacketType_eTraceBufferConfigSet .
*/
typedef struct {
   /** The actual buffer size in bytes. This can be smaller than the requested one. */
   IFX_uint32_t size;
} DTI_D2H_TraceConfigSet_t;

/** Read Trace Status

   Packet type: \ref  DTI_PacketType_eTraceBufferStatusGet .
*/
typedef struct {
   /** Buffer mode. DTI_TraceBufferMode_t (see Configure Trace Buffer) */
   IFX_uint32_t mode;
   /** The actual buffer size in bytes. */
   IFX_uint32_t size;
   /** Number of pending bytes in trace buffer */
   IFX_uint32_t fill;
} DTI_D2H_TraceStatusGet_t;

/** Read Trace Buffer Content.

   Packet type: \ref  DTI_PacketType_eTraceBufferGet .

   This message allows reading of the trace buffer. The whole content is streamed.
   The streamed content is removed from the buffer.
*/
typedef struct {
   /** If error code is zero the complete debug buffer content is sent. */
   IFX_uint8_t data[1];
} DTI_D2H_TraceRead_t /* obsulate */;


/** Request the Trace Buffer Content.

   Packet type: \ref  DTI_PacketType_eTraceBufferGet .

   This message allows to requests the trace buffer content.
   The whole content is streamed. The streamed content is removed from the buffer.

\remark
   This call is none-blocking, means the call returns if the
   - requested size has been read
   - no further data are available (trace buffer empty).
*/
typedef struct {
   /** requested content size */
   IFX_uint32_t size;
} DTI_H2D_TraceBufferGet_t;


/** Returns the Trace Buffer Content.

   Packet type: \ref  DTI_PacketType_eTraceBufferGet .

   This message returns the requested trace buffer content.
   The whole content is streamed. The streamed content is removed from the buffer.
*/
typedef struct {
   /** If error code is zero the complete debug buffer content is sent. */
   IFX_uint8_t data[1];
} DTI_D2H_TraceBufferGet_t;




/** Read Trace Buffer Status.

   Packet type: \ref  DTI_PacketType_eTraceBufferAvailable .

   This message is send once autonomously if the trace buffer contains at least the configured number of bytes.
*/
typedef DTI_D2H_TraceStatusGet_t DTI_D2H_TraceNotification_t;

/* ============================================================================
   Packet Type: 0x0004xxxx - Debug Register Access
   ========================================================================= */

/**
    Access types for DTI_H2D_DebugRead/Write packets
*/
typedef enum {
    DTI_VNX_eAux_t  = 0,
    DTI_VNX_eCore_t = 1,
    DTI_VNX_eMem_t  = 2
} DTI_VnxAccess_t;


/** Debug Read .

   Packet type: \ref  DTI_PacketType_eDebugGet .

*/
typedef struct {
   /** The register / memory type (eAux, eCore, eMem) */
   IFX_uint32_t type;
   /** The address offset into the specified register / memory bank */
   IFX_uint32_t offset;
   /** Number of 32 bit values to read.
       If count is greater than the maximum number of reads supported by the DTI implementation an error is generated. */
   IFX_uint32_t count;
} DTI_H2D_DebugRead_t;

/** Debug Read.

   Packet type: \ref  DTI_PacketType_eDebugGet .

   This packet is used to read from the debug port.

   \remarks Vinax only
*/
typedef struct {
   IFX_uint32_t data[1];
} DTI_D2H_DebugRead_t;

/** Debug Write .

   Packet type: \ref  DTI_PacketType_eDebugSet .

   This packet is used to write to the debug port.

   \remarks Vinax only
*/
typedef struct {
   /** The register / memory type (eAux, eCore, eMem) */
   IFX_uint32_t type;
   /** The address offset into the specified register / memory bank */
   IFX_uint32_t offset;
   /** 32bit values to write
       If the number of  the data items exceeds the supported value of the implementation, none of the writes is executed
       and a error indication is generated. */
   IFX_uint32_t data[1];
} DTI_H2D_DebugWrite_t;

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* _IFX_DTI_PROTOCOL_DEVICE_H */
