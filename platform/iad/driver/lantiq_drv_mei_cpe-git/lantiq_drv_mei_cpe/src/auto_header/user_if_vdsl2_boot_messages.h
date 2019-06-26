/******************************************************************************

                          Copyright (c) 2007-2015
                     Lantiq Beteiligungs-GmbH & Co. KG

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#ifndef _BOOT_MESSAGES_H_
#define _BOOT_MESSAGES_H_

/** \file
   This section describes the basic data types used for the boot messages.
*/

#ifndef __PACKED__
   #if defined (__GNUC__) || defined (__GNUG__)
      /* GNU C or C++ compiler */
      #define __PACKED__ __attribute__ ((packed))
   #elif !defined (__PACKED__)
      #define __PACKED__      /* nothing */
   #endif
#endif


/** @defgroup _BOOT_MESSAGES_ Basic Data Types
 *  @{
 */

#ifdef __cplusplus
   extern "C" {
#endif


#if 1

#include "ifx_types.h"

#else


/* ----- Include section ----- */
/* ----- Include section (End) ----- */

/* ----- Define section ----- */
/* ----- Define section (End) ----- */

/** This is an unsigned 32-bit data type */
typedef unsigned long IFX_uint32_t;


/** This is an unsigned 16-bit data type */
typedef unsigned short IFX_uint16_t;


/** This is an unsigned 8-bit data type */
typedef unsigned char IFX_uint8_t;


/** This is an 1-bit data type */
typedef unsigned int IFX_bit_t;
#endif

/**
   This structure defines the boot messages.
*/
typedef struct BootMessage BootMessage_t;

/** Message ID for EVT_InitDone */
#define EVT_INITDONE 0x681

/**
   This message is sent to the host when the boot initialization is complete. This
   event triggers the host to select the master.
*/
typedef struct EVT_InitDone EVT_InitDone_t;

/** Message ID for EVT_SRAM_Invalid */
#define EVT_SRAM_INVALID 0x682

/**
   This message is sent from the master modem to the host when the contents of the SRAM
   is not valid and notifies to the host to download the Firmware image into the SRAM.
   The host is required to download the Firmware image into the SRAM. The firmware
   download is started by sending CMD_FW_DownloadInit to initialize the download.
   .
*/
typedef struct EVT_SRAM_Invalid EVT_SRAM_Invalid_t;

/** Message ID for EVT_SRAM_Valid */
#define EVT_SRAM_VALID 0x683

/**
   This message is sent to the host when the master modem detects a valid Firmware
   image in the SRAM. The host then sends the message CMD_BusArbitrationStart to start
   the bus arbitration or decides to override the SRAM by sending CMD_FW_DownloadInit.
*/
typedef struct EVT_SRAM_Valid EVT_SRAM_Valid_t;

/** Message ID for EVT_BootComplete */
#define EVT_BOOTCOMPLETE 0x685

/**
   This message is sent to the host notifying the status of boot completeness and going
   into online mode if the status indicates SUCCESS.
*/
typedef struct EVT_BootComplete EVT_BootComplete_t;

/** Message ID for CMD_FW_DownloadInit */
#define CMD_FW_DOWNLOADINIT 0x101

/**
   This message is sent from the host to the master modem in case of not a valid
   Firmware image loaded into the SRAM or when the Firmware image is valid and a new
   Firmware is to be loaded. This message contains the configuration parameters for
   accessing the SRAM (data width, wait states, bus arbitration parameters, etc.).
*/
typedef struct CMD_FW_DownloadInit CMD_FW_DownloadInit_t;

/** Message ID for ACK_FW_DownloadInit */
#define ACK_FW_DOWNLOADINIT 0x301

/**
   This message is sent to the host as an acknowledgement to CMD_FW_DownloadInit.
*/
typedef struct ACK_FW_DownloadInit ACK_FW_DownloadInit_t;

/** Message ID for CMD_FW_BlockDownload */
#define CMD_FW_BLOCKDOWNLOAD 0x102

/**
   This message is sent from the host to the master modem transferring the next
   Firmware image block to the specified bulk memory address and indicates that this
   Firmware block is ready to be transferred from the bulk memory into the SRAM by the
   boot code. The boot code then copies the corresponding Firmware block from bulk
   memory into the SRAM.
*/
typedef struct CMD_FW_BlockDownload CMD_FW_BlockDownload_t;

/** Message ID for ACK_FW_BlockDownload */
#define ACK_FW_BLOCKDOWNLOAD 0x302

/**
   This message is sent to the host as an acknowledgement to CMD_FW_BlockDownload.
*/
typedef struct ACK_FW_BlockDownload ACK_FW_BlockDownload_t;

/** Message ID for CMD_FW_DownloadComplete */
#define CMD_FW_DOWNLOADCOMPLETE 0x103

/**
   This message is sent from the host to the master modem when all the Firmware image
   blocks have been downloaded into SRAM.
*/
typedef struct CMD_FW_DownloadComplete CMD_FW_DownloadComplete_t;

/** Message ID for ACK_FW_DownloadComplete */
#define ACK_FW_DOWNLOADCOMPLETE 0x303

/**
   This message is sent to the host as an acknowledgement to CMD_FW_DownloadComplete.
*/
typedef struct ACK_FW_DownloadComplete ACK_FW_DownloadComplete_t;

/** Message ID for CMD_BusArbitrationStart */
#define CMD_BUSARBITRATIONSTART 0x104

/**
   This message is sent from the host to the master chip after receiving
   ACK_FW_DownloadComplete. With this message, the master chip controller starts the
   bus arbitration and gives the bus grant free to the slaves. Then it waits for its
   turn to get the bus grant. The bus arbitration in this case means that the SRAM
   access has to be arbitrated between the different chips wherein the master is always
   responsible for giving the first bus grant to the first slave.
*/
typedef struct CMD_BusArbitrationStart CMD_BusArbitrationStart_t;

/** Message ID for ACK_BusArbitrationStart */
#define ACK_BUSARBITRATIONSTART 0x304

/**
   This message is sent to the host as an acknowledgement to CMD_BusArbitrationStart.
*/
typedef struct ACK_BusArbitrationStart ACK_BusArbitrationStart_t;

/** Message ID for CMD_OnlineStateActivate */
#define CMD_ONLINESTATEACTIVATE 0x105

/**
   This message is sent from the host to trigger the boot code to start the modem code.
   This message is only sent when the boot mode is set to 7 (pinstrap BTCFG=0111B),
   assuming that the host has written the first page of modem code into the instruction
   RAM of the chip controller.
*/
typedef struct CMD_OnlineStateActivate CMD_OnlineStateActivate_t;

/** Message ID for ACK_OnlineStateActivate */
#define ACK_ONLINESTATEACTIVATE 0x305

/**
   This message is sent to the host as an acknowledgement to CMD_OnlineStateActivate.
*/
typedef struct ACK_OnlineStateActivate ACK_OnlineStateActivate_t;

/** Message ID for CMD_BootCodeVersionGet */
#define CMD_BOOTCODEVERSIONGET 0x106

/**
   This message is sent from the host to request for the version number of the boot
   code.
*/
typedef struct CMD_BootCodeVersionGet CMD_BootCodeVersionGet_t;

/** Message ID for ACK_BootCodeVersionGet */
#define ACK_BOOTCODEVERSIONGET 0x306

/**
   This message is sent to the host as an acknowledgement to CMD_BootCodeVersionGet and
   it contains the version number of the boot code.
*/
typedef struct ACK_BootCodeVersionGet ACK_BootCodeVersionGet_t;

/** Message ID for CMD_MemoryMappedRead */
#define CMD_MEMORYMAPPEDREAD 0x110

/**
   This message is sent from the host to read a value from any memory mapped region in
   the controller subsystem.
*/
typedef struct CMD_MemoryMappedRead CMD_MemoryMappedRead_t;

/** Message ID for ACK_MemoryMappedRead */
#define ACK_MEMORYMAPPEDREAD 0x310

/**
   This message is sent to the host as an acknowledgement to CMD_MemoryMappedRead.
*/
typedef struct ACK_MemoryMappedRead ACK_MemoryMappedRead_t;

/** Message ID for CMD_MemoryMappedWrite */
#define CMD_MEMORYMAPPEDWRITE 0x111

/**
   This message is sent from the host to write a value into any memory mapped region in
   the controller subsystem.
*/
typedef struct CMD_MemoryMappedWrite CMD_MemoryMappedWrite_t;

/** Message ID for ACK_MemoryMappedWrite */
#define ACK_MEMORYMAPPEDWRITE 0x311

/**
   This message is sent to the host as an acknowledgement to CMD_MemoryMappedWrite.
*/
typedef struct ACK_MemoryMappedWrite ACK_MemoryMappedWrite_t;

/** Message ID for CMD_AuxRegisterRead */
#define CMD_AUXREGISTERREAD 0x112

/**
   This message is sent from the host to read any auxiliary register in the controller
   subsystem.
*/
typedef struct CMD_AuxRegisterRead CMD_AuxRegisterRead_t;

/** Message ID for ACK_AuxRegisterRead */
#define ACK_AUXREGISTERREAD 0x312

/**
   This message is sent to the host as an acknowledgement to CMD_AuxRegisterRead.
*/
typedef struct ACK_AuxRegisterRead ACK_AuxRegisterRead_t;

/** Message ID for CMD_AuxRegisterWrite */
#define CMD_AUXREGISTERWRITE 0x113

/**
   This message is sent from the host to write to any auxiliary register in the
   controller subsystem.
*/
typedef struct CMD_AuxRegisterWrite CMD_AuxRegisterWrite_t;

/** Message ID for ACK_AuxRegisterWrite */
#define ACK_AUXREGISTERWRITE 0x313

/**
   This message is sent to the host as an acknowledgement to CMD_AuxRegisterWrite.
*/
typedef struct ACK_AuxRegisterWrite ACK_AuxRegisterWrite_t;

/**
   This structure defines the boot messages.
*/
struct BootMessage
{
   /** Mailbox Code */
   IFX_uint16_t MbxCode;
   /** Function/Direction Opcode */
   IFX_uint16_t FctCode;
   /** Number of parameters */
   IFX_uint16_t Size;
   /** Message Identifier */
   IFX_uint16_t MessageID;
   /** Reserved */
   IFX_uint16_t ResWord4;
   /** Reserved */
   IFX_uint16_t ResWord5;
   /** Message parameters */
   IFX_uint32_t Params[8];
} ;


/**
   This message is sent to the host when the boot initialization is complete. This
   event triggers the host to select the master.
*/
struct EVT_InitDone
{
   /** Boot Configuration */
   IFX_uint32_t BTCFG;
} ;


/**
   This message is sent to the host notifying the status of boot completeness and going
   into online mode if the status indicates SUCCESS.
*/
struct EVT_BootComplete
{
   /** Indicates the status of boot completeness. */
   IFX_uint32_t Status;
} ;


/**
   This message is sent from the host to the master modem in case of not a valid
   Firmware image loaded into the SRAM or when the Firmware image is valid and a new
   Firmware is to be loaded. This message contains the configuration parameters for
   accessing the SRAM (data width, wait states, bus arbitration parameters, etc.).
*/
struct CMD_FW_DownloadInit
{
   /** Data Width */
   IFX_uint32_t DataWidth;
   /** Wait States */
   IFX_uint32_t WaitStates;
   /** Arbitration Period */
   IFX_uint32_t ArbitrationPeriod;
   /** Watchdog Timer Master */
   IFX_uint32_t WD_TimerMaster;
   /** Watchdog Timer Slave */
   IFX_uint32_t WD_TimerSlave;
} ;


/**
   This message is sent to the host as an acknowledgement to CMD_FW_DownloadInit.
*/
struct ACK_FW_DownloadInit
{
   /** Bulk Memory Transfer Address */
   IFX_uint32_t TransferAddress;
   /** Size */
   IFX_uint32_t Size;
} ;


/**
   This message is sent from the host to the master modem transferring the next
   Firmware image block to the specified bulk memory address and indicates that this
   Firmware block is ready to be transferred from the bulk memory into the SRAM by the
   boot code. The boot code then copies the corresponding Firmware block from bulk
   memory into the SRAM.
*/
struct CMD_FW_BlockDownload
{
   /** Bulk Memory Transfer Address */
   IFX_uint32_t BM_Address;
   /** Transfer Size */
   IFX_uint32_t Size;
} ;


/**
   This message is sent to the host as an acknowledgement to CMD_BootCodeVersionGet and
   it contains the version number of the boot code.
*/
struct ACK_BootCodeVersionGet
{
   /** Version Number */
   IFX_uint32_t Version;
} ;


/**
   This message is sent from the host to read a value from any memory mapped region in
   the controller subsystem.
*/
struct CMD_MemoryMappedRead
{
   /** Address */
   IFX_uint32_t Address;
} ;


/**
   This message is sent to the host as an acknowledgement to CMD_MemoryMappedRead.
*/
struct ACK_MemoryMappedRead
{
   /** Address */
   IFX_uint32_t Address;
   /** Value */
   IFX_uint32_t Value;
} ;


/**
   This message is sent from the host to write a value into any memory mapped region in
   the controller subsystem.
*/
struct CMD_MemoryMappedWrite
{
   /** Address */
   IFX_uint32_t Address;
   /** Value */
   IFX_uint32_t Value;
} ;


/**
   This message is sent from the host to read any auxiliary register in the controller
   subsystem.
*/
struct CMD_AuxRegisterRead
{
   /** Register Number */
   IFX_uint32_t RegNo;
} ;


/**
   This message is sent to the host as an acknowledgement to CMD_AuxRegisterRead.
*/
struct ACK_AuxRegisterRead
{
   /** Register Number */
   IFX_uint32_t RegNo;
   /** Value */
   IFX_uint32_t Value;
} ;


/**
   This message is sent from the host to write to any auxiliary register in the
   controller subsystem.
*/
struct CMD_AuxRegisterWrite
{
   /** Register Number */
   IFX_uint32_t RegNo;
   /** Value */
   IFX_uint32_t Value;
} ;


#ifdef __cplusplus
}
#endif

/** @} */
#endif

