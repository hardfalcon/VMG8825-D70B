/*
 * (C) Copyright 2005-2017 Sphairon GmbH (a ZyXEL company)
 *
 * SPDX-License-Identifier: GPL-2.0
 */

#ifndef __ISDN_USER_H__
#define __ISDN_USER_H__

#define MAX_LAYER1_DEVICES 4

#define L1_REQUEST      0
#define L1_CONFIRM      1
#define L1_INDICATION   2
#define L1_RESPONSE     3

#define PH_ACTIVATE   0x0100
#define PH_DEACTIVATE 0x0110
#define PH_DATA       0x0120

#define IXF_NONE        0x00000000  // No flags
#define IXF_TE_MODE     0x00000001  // MUST NOT be combined with LTS Mode (default)
#define IXF_LTS_MODE    0x00000002  // MUST NOT be combined with TE Mode
#define IXF_SRES_WAR    0x00000004
#define IXF_SWAP_TRBC   0x00000008  // only swap the B-channel transciever
#define IXF_NO_SRES     0x00000010
#define IXF_NO_PWR_SRC1 0x00000020
#define IXF_LTT_MODE    0x00000040
#define IXF_FXO_OP_MODE 0x00008000  // if set ISDN_FXO is detected

// Configuration Flags
#define ICF_NONE            0x00000000  // No flags
#define ICF_TX_ENABLE       0x00000001  // Configure Transmitter enable/disable

// Maximum length of a D-Channel Message (longer messages are discarded)
#define MAX_DFRAME_LEN_L1   300 // changed from 150 to 300 to support full frames

// Device filenames as they appear in /dev/<devicename>
#define LAYER1_DEV0_NAME   "/dev/isdn0"
#define LAYER1_DEV1_NAME   "/dev/isdn1"
#define LAYER1_DEV2_NAME   "/dev/isdn2"
#define LAYER1_DEV3_NAME   "/dev/isdn3"

// ISDN Layer 1 Exchange Structure
typedef struct tagL1MSG {
//  unsigned long dwSize;                       // Size of Exchange structure
    unsigned long dwStatus; // Layer 1 Status
    unsigned long dwCount;  // Count of HDLC Data
    unsigned char Data[MAX_DFRAME_LEN_L1];  // HDLC Frame Data In/Out (maybe allocated dynamically later!)
} L1MSG, *LPL1MSG;

// T-SMINT Information Structure
typedef struct tagL1DEV_INFO {
    unsigned long dwSize;   // Size of the structure (used to handle extensions)
    unsigned long dwVersion;    // Driver Version Information
    unsigned long dwDevNum; // T-SMINT Minor Device Number (index of the chip)
    unsigned long dwDevCount;   // Number of Configured Devices (since version 2.6)
    unsigned long dwTimeslotA;  // Timeslot A
    unsigned long dwTimeslotB;  // Timeslot B
    unsigned long dwTimeslotD;  // Timeslot D
    unsigned long dwFlags;  // I/O flags for the queried device
} L1DEV_INFO, *LPL1DEV_INFO;

// T-SMINT Configuration Structure
typedef struct tagL1DEV_CONFIG_CONFIG {
    unsigned long dwSize;   // Size of the structure (used to handle extensions)
    unsigned long dwFlags;  // Configuration Flags (refer to ICF_XXX flags above)
    // ICF_TX_ENABLE configuration variable:
    bool bEnableTX;     // Enable/Disable Transmitter
    unsigned long dwBChannel;   // B-Channel to enable/disable transmitter for (can be 0 or 1)
} L1DEV_CONFIG, *LPL1DEV_CONFIG;

// T-SMINT Hardware Test Structure
typedef struct tagL1DEV_HWTEST {
    unsigned long dwSize;   // Size of the structure (used to handle extensions)
    unsigned long dwCmd;    // Test command to be enabled / disabled
    unsigned long dwFlags;
} L1DEV_HWTEST, *LPL1DEV_HWTEST;

// T-SMINT Led control Structure
typedef struct tagL1DEV_SETLED {
    unsigned long dwSize;   // Size of the structure (used to handle extensions)
    unsigned long dwState;  // Control state of Led
} L1DEV_SETLED, *LPL1DEV_SETLED;

// T-SMINT Status Structure
typedef struct tagL1DEV_STATUS {
    unsigned long dwSize;   // (IN)  Size of the structure (used to handle extensions)
    unsigned long dwMask;   // (IN)  Mask to indicate which status flags to query
    unsigned long dwStatus; // (OUT) Status flags returned for the requested mask
} L1DEV_STATUS, *LPL1DEV_STATUS;

// T-SMINT Register Manipulation structure (used by read reg and write reg)
typedef struct tagISDN_REG {
    unsigned long dwSize;   // Size of the structure (used to handle extensions)
    unsigned long dwRegOfs;
    unsigned long dwRegVal;
} L1DEV_REG, *LPL1DEV_REG;

#define STG_IOCTL_MAGIC              0xbf
// I/O Control (ioctl) definitions
#define IOCTL_L1DEV_GET_INFO     _IOR(STG_IOCTL_MAGIC, 0, LPL1DEV_INFO)
#define IOCTL_L1DEV_SET_CONFIG   _IOW(STG_IOCTL_MAGIC, 1, LPL1DEV_CONFIG)
#define IOCTL_L1DEV_GET_CONFIG   _IOWR(STG_IOCTL_MAGIC, 2, LPL1DEV_CONFIG)
#define IOCTL_L1DEV_SET_L1STATE  _IOR(STG_IOCTL_MAGIC, 3, bool)
#define IOCTL_L1DEV_GET_L1STATE  _IOR(STG_IOCTL_MAGIC, 4, bool)
#define IOCTL_L1DEV_SET_HWTEST   _IOW(STG_IOCTL_MAGIC, 5, LPL1DEV_HWTEST)
#define IOCTL_L1DEV_GET_STATUS   _IOR(STG_IOCTL_MAGIC, 6, LPL1DEV_STATUS)
#define IOCTL_L1DEV_SET_LEDSTATE _IOW(STG_IOCTL_MAGIC, 10, LPL1DEV_SETLED)
#define IOCTL_L1DEV_ENABLE       _IOW(STG_IOCTL_MAGIC, 11, unsigned long)
#define IOCTL_L1DEV_DISABLE      _IOW(STG_IOCTL_MAGIC, 12, unsigned long)
#define IOCTL_L1DEV_SET_BUSMODE  _IOW(STG_IOCTL_MAGIC, 13, bool)

// actually implemented for testing purpose only
#define IOCTL_L1DEV_READ_REG     _IOWR(STG_IOCTL_MAGIC, 101, LPL1DEV_REG)
#define IOCTL_L1DEV_WRITE_REG    _IOWR(STG_IOCTL_MAGIC, 102, LPL1DEV_REG)

// defines for ACT-LED control
#define HW_LED_ON       1       // Switch Led ON
#define HW_LED_OFF      0       // Switch Led OFF
#define HW_LED_1HZ      2       // Switch Led 1HZ
#define HW_LED_2HZ      3       // Switch Led 2HZ
#define HW_LED_HWCTL    4       // Switch Led Hardwarecontrol
#define HW_LED_B1_ON    5       // Switch 1. B-Channel Led ON
#define HW_LED_B2_ON    6       // Switch 2. B-Channel Led ON
#define HW_LED_B1_OFF   7       // Switch 1. B-Channel Led OFF
#define HW_LED_B2_OFF   8       // Switch 2. B-Channel Led OFF

//  Status flag/mask definitions
#define ISF_NONE                    0x00000000  // No flag
#define ISF_ONLINE              0x00000001  // set if line is feed (TE mode only)
#define ISF_P2P                     0x00000002  // set if in p2p mode    (LTS mode only)

//  Hardware Test Commands
enum {
	HWTEST_CMD_NONE = 0,
	HWTEST_CMD_LOOP,
	HWTEST_CMD_SCP,
	HWTEST_CMD_SSP,
	HWTEST_CMD_RESET
};

enum {
	HWTEST_LOOP_SIDE_NONE = (1 << 0),
	HWTEST_LOOP_SIDE_TR = (1 << 1),
	HWTEST_LOOP_SIDE_IOM = (1 << 2),
	HWTEST_LOOP_TYPE_NONE = (1 << 3),
	HWTEST_LOOP_TYPE_B1 = (1 << 4),
	HWTEST_LOOP_TYPE_B2 = (1 << 5),
	HWTEST_LOOP_TYPE_B1B2 = (1 << 6)
};

#endif /* __ISDN_USER_H__ */
