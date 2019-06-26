/*
 * (C) Copyright 2005-2017 Sphairon GmbH (a ZyXEL company)
 *
 * SPDX-License-Identifier: GPL-2.0
 */

#ifndef __ISDN_ISACSX_H__
#define __ISDN_ISACSX_H__

// Driver Version number (can be queried via ioctl GET_INFO)
#define ISACSX_DRV_VERSION     0x00040000	// Hi-Word = Major, Lo-Word = Minor
#define ISACSX_MAJ_VER         ((unsigned short)(ISACSX_DRV_VERSION >> 16))
#define ISACSX_MIN_VER         ((unsigned short)(ISACSX_DRV_VERSION & 0x0000FFFF))

/////////////////////////////////////////////////////////////////////////////
// Register definitions of the ISAC-SX PEB 3086

// Undefined Reset Value
#define URV   0x55

// Access Flag definitions
#define NA    0x00		// No Access
#define R_    0x01
#define _W    0x02
#define RW    0x03

// ISAC-SX Register Category Definition
typedef struct tagISACSXCAT {
	char *pszName;		// Category name
	char *pszDescription;	// Category description
	unsigned long dwStart;	// Start Index for this category
	unsigned long dwCount;	// Count of registers in this category
} ISACSXCAT, *LPISACSXCAT;

// ISAC-SX Register Definition
typedef struct tagISACSXREG {
	char *pszName;
	unsigned char ResMask;	// Reserved Bits Mask
	unsigned char ResVal;	// Reserved Bits Value (1 = set or undefined)
	unsigned char Addr;	// Address Offset
	unsigned char Length;	// Address Range Length
	unsigned char Access;	// Access (R, W, RW)
	unsigned char Reset;	// Reset Value
} ISACSXREG, *LPISACSXREG;

/////////////////////////////////////////////////////////////////////////////
// Register Adresses and Bit definitions of the ISAC-SX PEB 3086

/////////////////////////////////////////////////////////////////////////////
// D-Channel HDLC, C/I Handler Registers

// D-Channel FIFO Address-Range
// Note: The ISAC maintains an internal pointer for accessing the FIFOs, 
//       which is automatically incremented.
//       This means the address doesn't matter when accessing the FIFOs.

#define ISAC_PRESENT 1

// RFIFOD - D-Channel Receive FIFO (RD)
#define ISACSX_RFIFOD          0x00
#define ISACSX_RFIFOD_SIZE   0x20	// D-Channel Receive FIFO Size (32 byte)
// XFIFOD - D-Channel Transmit FIFO (WR)
#define ISACSX_XFIFOD          0x00
#define ISACSX_XFIFOD_SIZE   0x20	// D-Channel Transmit FIFO Size (32 byte)

// ISTAD - Interrupt Status Register D-Channel (RD, Default=0x10)
#define ISACSX_ISTAD           0x20
  // ISTAD Bit Flags
#define ISTAD_RME            0x80	// Receive Message End
#define ISTAD_RPF            0x40	// Receive Pool Full
#define ISTAD_RFO            0x20	// Receive Frame Overflow
#define ISTAD_XPR            0x10	// Transmit Pool Ready
#define ISTAD_XMR            0x08	// Transmit Message Repeat
#define ISTAD_XDU            0x04	// Transmit Data Underrun
#define ISTAD_RESERVED       0x03	// Reserved Mask Bits 0 and 1 are always 0

// MASKD - Interrupt Mask Register D-Channel (WR, Default=0xFF)
#define ISACSX_MASKD           0x20
  // MASKD Bit Flags
#define MASKD_RME            0x80	// Mask Receive Message End
#define MASKD_RPF            0x40	// Mask Receive Pool Full
#define MASKD_RFO            0x20	// Mask Receive Frame Overflow
#define MASKD_XPR            0x10	// Mask Transmit Pool Ready
#define MASKD_XMR            0x08	// Mask Transmit Message Repeat
#define MASKD_XDU            0x04	// Mask Transmit Data Underrun
#define MASKD_RESERVED       0x03	// Reserved Mask Bits 0 and 1 are always 1

// STARD - Status Register D-Channel (RD, Default=0x40)
#define ISACSX_STARD           0x21
  // STARD Bit Flags (unused bits are zero)
#define STARD_XDOV           0x80	// Transmit Data Overflow
#define STARD_XFW            0x40	// Transmit FIFO Write Enable
#define STARD_RACI           0x08	// Receiver Active Indication
#define STARD_XACI           0x02	// Transmiter Active Indication

// CMDRD - Command Register D-Channel (WR, Default=0x00)
#define ISACSX_CMDRD           0x21
  // CMDRD Bit Flags (unused bits are zero)
#define CMDRD_RMC            0x80	// Receive Message Complete
#define CMDRD_RRES           0x40	// Receiver Reset
#define CMDRD_STI            0x10	// Start Timer 1
#define CMDRD_XTF            0x08	// Transmit Transparent Frame
#define CMDRD_XME            0x02	// Transmit Message End
#define CMDRD_XRES           0x01	// Transmitter Reset

// MODED - Mode Register (RD/WR, Default=0xC0)
#define ISACSX_MODED           0x22
  // MODED Bit Flags (unused bits are zero)
  // MDS2-0, Mode Select
#define MODED_MDS_MASK       0xE0	// Mode Select Mask
#define MODED_MDS_NONAUTO1   0x40	// Non-Auto Mode, One-byte address compare
#define MODED_MDS_NONAUTO2   0x60	// Non-Auto Mode, Two-byte address compare
#define MODED_MDS_EXTTRANS   0x80	// Extended Transparent Mode
#define MODED_MDS_TRANS0     0xC0	// No address compare, all frames accepted
#define MODED_MDS_TRANS1     0xE0	// High-byte address compare
#define MODED_MDS_TRANS2     0xA0	// Low-byte address compare

#define MODED_RAC            0x08	// Receiver Active

  // DIM2-0, Digital Interface Modes
#define MODED_DIM_MASK       0x07	// DIM Mask
#define MODED_DIM2           0x04	// reserved
#define MODED_DIM1           0x02	// TIC bus enabled/disable
#define MODED_DIM0           0x01	// collision detection on/off

#define MODED_DIM_COLLOFF    0x00	// Transparent D-Channel, collision detection is disabled
#define MODED_DIM_COLLON     0x01	// Stop/go bit evaluated for D-Channel access handling
#define MODED_DIM_TICON      0x00	// Last octet of IOM channel 2 used for TIC bus access
#define MODED_DIM_TICOFF     0x02	// TIC bus access is disabled
  // Note the special modification rules when accessing the register:
  // DIM2 | DIM1 | DIM0
  // ------------------
  //  0   |      |  0         Transparent D-Channel, collision detection is disabled
  //  0   |      |  1         Stop/go bit evaluated for D-Channel access handling
  //  0   |  0   |            Last octet of IOM channel 2 used for TIC bus access
  //  0   |  1   |            TIC bus access is disabled
  //  1   |  x   |  x         Reserved

// EXMD1 - Extended Mode Register D-Channel 1 (RD/WR, Default=0x00)
#define ISACSX_EXMD1           0x23
  // EXMD1 Bit Flags (unused bits are zero)
#define EXMD1_XFBS_MASK      0x80	// Transmit FIFO Block Size Mask
#define EXMD1_XFBS_32        0x00	// Block size for Transmit FIFO Data is 32
#define EXMD1_XFBS_16        0x80	// Block size for Transmit FIFO Data is 16

#define EXMD1_RFBS_MASK      0x60	// Receive FIFO Block Size Mask
#define EXMD1_RFBS_32        0x00	// Block size for Receive FIFO Data is 32
#define EXMD1_RFBS_16        0x20	// Block size for Receive FIFO Data is 16
#define EXMD1_RFBS_8         0x40	// Block size for Receive FIFO Data is 8
#define EXMD1_RFBS_4         0x60	// Block size for Receive FIFO Data is 4

#define EXMD1_SRA            0x10	// Store Receive Address in RFIFOD (0=Don't Store, 1=Store)
#define EXMD1_XCRC           0x08	// Transmit CRC (0=Transmit, 1=Don't Transmit)
#define EXMD1_RCRC           0x04	// Store CRC in RFIFOD (0=Don't Store, 1=Store)
#define EXMD1_ITF            0x01	// Interframe Time Fill (0=idle cont. 1s, 1=flags seq. of patterns '01111110')

// TIMR1 - Timer 1 Register (RD/WR, Default=0x00)
#define ISACSX_TIMR1           0x24
  // TIMR1 Counter / Value
#define TIMR1_CNT_MASK			 0xE0	// CNT - Timer Counter Mask
  // CNT = 0..6: T = CNT x 2.048sec + T1, with T1 = (VALUE + 1) x 0.064sec
  // CNT = 7:    T = T1 = (VALUE + 1) x 0.064sec (generated periodically)
#define TIMR1_VAL_MASK			 0x1F	// VALUE - Timer Value Mask
  // VALUE = T1 = (VALUE + 1) x 0.064sec
  // (add macros that generate the correct values for a given period if we use this)

// SAP1 - SAPI1 Register (WR, Default=0xFC)
#define ISACSX_SAP1            0x25
  // SAP1 Bit Flags (unused bits are zero)
#define SAP1_SAPI1_MASK      0xFC	// Mask for Value of the first Service Access Point Identifier (SAPI)
#define SAP1_MHA             0x01	// Mask High Address (SAPI address of incoming frame is compared)
  // (add macros that generate the correct register value if we use this)

// SAP2 - SAPI2 Register (WR, Default=0xFC)
#define ISACSX_SAP2            0x26
  // SAP2 Bit Flags (unused bits are zero)
#define SAP2_SAPI2_MASK      0xFC	// Mask for Value of the first Service Access Point Identifier (SAPI)
#define SAP2_MLA             0x01	// Mask Low Address (TEI address of incoming frame is compared)
  // (add macros that generate the correct register value if we use this)

// RBCLD - Receive Frame Byte Count Low D-Channel Register (RD, Default=0x00)
#define ISACSX_RBCLD           0x26
  // RBC7-0 : Receive Byte Count
  // Eight least significant bits of the total number of bytes in a received message (see RBCHD register).

// RBCHD - Receive Frame Byte Count High D-Channel Register (RD, Default=0x00)
#define ISACSX_RBCHD           0x27
  // RBCHD Bit Flags (unused bits are zero)
#define RBCHD_OV             0x10	// Overflow - A '1' in this bit position indicates
					// a message longer than (212 - 1) = 4095 bytes
#define RBCHD_RBCHI_MASK     0x0F	// Mask for Hi-Value of RBC (Receive Byte Count)
  // Note: Normally RBCHD and RBCLD should be read by the microcontroller after an
  // RME-interrupt in order to determine the number of bytes to be read from the
  // RFIFOD, and the total message length. The contents of the registers are valid only
  // after an RME or RPF interrupt, and remain so until the frame is acknowledged via
  // the RMC bit or RRES.
  // (add macros that generate the correct register value if we use this)

// TEI1 - TEI1 Register 1 (WR, Default=0xFF)
#define ISACSX_TEI1            0x27
  // TEI1 Bit Flags (unused bits are zero)
#define TEI1_TEI_MASK        0xFE	// Mask for Value of the TEI 1 - Terminal Endpoint Indentifier
#define TEI1_EA1             0x01	// Address field Extension bit - set to '1' according to HDLC/LAPD.
  // (add macros that generate the correct register value if we use this)

// TEI2 - TEI2 Register 1 (WR, Default=0xFF)
#define ISACSX_TEI2            0x28
  // TEI2 Bit Flags (unused bits are zero)
#define TEI2_TEI_MASK        0xFE	// Mask for Value of the TEI 2 - Terminal Endpoint Indentifier
#define TEI2_EA1             0x01	// Address field Extension bit - set to '1' according to HDLC/LAPD.
  // (add macros that generate the correct register value if we use this)

// RSTAD - Receive Status Register D-Channel (RD, Default=0x0F)
#define ISACSX_RSTAD           0x28
  // RSTAD Bit Flags
#define RSTAD_VFR            0x80	// Valid Frame - The received frame is valid (1) or invalid (0).
#define RSTAD_RDO            0x40	// Receive Data Overflow - If RDO=1, at least one byte of the frame has been lost.
#define RSTAD_CRC            0x20	// CRC Check - The CRC is correct (1) or incorrect (0).
#define RSTAD_RAB            0x10	// Receive Message Aborted - The receive message was aborted by the remote station (1).
#define RSTAD_VALID_MASK     (RSTAD_VFR | RSTAD_RDO | RSTAD_CRC | RSTAD_RAB)
#define RSTAD_SA1            0x08	// SAPI Address Identification 1 - See documentation for details
#define RSTAD_SA0            0x04	// SAPI Address Identification 0 - See documentation for details
#define RSTAD_SA_MASK        (RSTAD_SA1 | RASTAD_SA0)	// Mask for SA1..SA0
#define RSTAD_CR             0x02	// Command/Response - contains the received frame's C/R bit (Bit1 in SAPI address)
#define RSTAD_TA             0x01	// TEI Address Identification - See documentation for details
  // (add macros that generate the correct register value if we use SA1-0, TA)

// TMD - Test Mode Register D-Channel (RD/WR, Default=0x00)
#define ISACSX_TMD             0x29
  // TMD Bit Flags (unused bits are zero)
#define TMD_TLP              0x01	// Test Loop - The setting of TLP is only valid if the IOM interface is active.

// CIR0 - Command/Indication Receive 0 Register (RD, Default=0xF3)
#define ISACSX_CIR0            0x2E
  // CIR0 Bit Flags
#define CIR0_CODR0           0xF0	// C/I Code 0 Receive - Value of the received Command/Indication code.
#define CIR0_CIC0            0x08	// C/I Code 0 Change - A change in the received C/I code has been recognized.
#define CIR0_CIC1            0x04	// C/I Code 1 Change - A change in the received C/I code in IOM-ch-1 has been recognized.
#define CIR0_SG              0x02	// Stop/Go Bit Monitoring - avail. upstream D-channel on the S/T interface, 1: Stop, 0: Go.
#define CIR0_BAS             0x01	// Bus Access Status - State of the TIC-bus: 0: the ISAC-SX itself occupies the D- and C/I-channel
					//                                           1: another device occupies the D- and C/I-channel
  // (add macros to access register value if we use CODR0)

// CIX0 - Command/Indication Transmit 0 Register (WR, Default=0xFE)
#define ISACSX_CIX0            0x2E
  // CIX0 Bit Flags
#define CIX0_CODX0           0xF0	// C/I-Code 0 Transmit - Code to be transmitted in the C/I-channel 0.
#define CIX0_TBA             0x06	// TIC Bus Address, Defines the individual address for the ISAC-SX on the IOM bus.
#define CIX0_BAC             0x01	// Bus Access Control- Only valid if the TIC-bus feature is enabled (MODED.DIM2-0).
  // (add macros to access register value if we use CODX0, TBA)

// CIR1 - Command/Indication Receive 1 Register (RD, Default=0xFE)
#define ISACSX_CIR1            0x2F
  // CIR1 Bit Flags
#define CIR1_CODR1           0xFC	// C/I Code 1 Receive - Value of the received Command/Indication code.
#define CIR1_CICW            0x02	// C/I-Channel Width  (read back from CIX1, see below)
#define CIR1_CL1E            0x01	// C/I-Channel 1 Interrupt Enable (read back from CIX1, see below)
  // (add macros to access register value if we use CODR0)

// CIX1 - Command/Indication Transmit 0 Register (WR, Default=0xFE)
#define ISACSX_CIX1            0x2F
  // CIX1 Bit Flags
#define CIX1_CODX1           0xF0	// C/I-Code 1 Transmit - Code to be transmitted in the C/I-channel 1.
#define CIX1_CICW            0x02	// C/I-Channel Width - selects a 4 bit ('0') or 6 bit ('1') C/I1 channel width.
#define CIX1_CL1E            0x01	// C/I-Channel 1 Interrupt Enable - Interrupt ISTA.CIC of CIR0.CIC1 is enabled (1) or masked (0).
  // (add macros to access register value if we use CODX1)

/////////////////////////////////////////////////////////////////////////////
// Transceiver Registers

// TR_CONF0 - Transceiver Configuration Register 0 (RD/WR, Default=0x01)
#define ISACSX_TR_CONF0        0x30
  // TR_CONF0 Bit Flags (unused bits are zero)
#define TR_CONF0_DIS_TR      0x80	// Disable Transceiver - Setting DIS_TR to '1' disables the transceiver.
					// The transceiver must not be reenabled by setting DIS_TR from '1' to '0'.
#define TR_CONF0_BUS         0x40	// Point-to-Point / Bus Selection (NT, LT-S and Int. NT mode only)
					// 0: Adaptive Timing (Point-t-Point, extended passive bus).
					// 1: Fixed Timing (Short passive bus).
#define TR_CONF0_EN_ICV      0x20	// Enable Illegal Code Violation
					// 0: normal operation
					// 1: ICV enabled. The receipt of at least one illegal code violation within one multi-frame 
					//    is indicated by the C/I indication '1011' (CVR) in two consecutive IOM frames.
#define TR_CONF0_L1SW        0x08	// Enable Layer 1 State Machine in Software -  0: Hardware SM, 1: Software SM
#define TR_CONF0_EXLP        0x02	// External loop
					// 0: internal loop next to the line pins
					// 1: external loop which has to be closed between SR1/2 and SX1/SX2
#define TR_CONF0_LDD         0x01	// Level Detection Discard
					// 0: Automatic clock generation after detection of any signal on the line in power down state
					// 1: No clock generation after detection of any signal on the line in power down state
					// Note: If an interrupt by the level detect circuitry is generated, the microcontroller
					//       has to set this bit to '0' for an activation of the S/T interface.

// TR_CONF1 - Transceiver Configuration Register 1 (RD/WR, Default=0x??)
#define ISACSX_TR_CONF1        0x31
  // TR_CONF1 Bit Flags (unused bits are zero, except Bist 2-0, see note below)
#define TR_CONF1_RPLL_ADJ    0x40	// Receive PLL Adjustment
					// 0: DPLL tracking step is 0.5 XTAL period per S-frame
					// 1: DPLL tracking step is 1 XTAL period per S-frame
#define TR_CONF1_EN_SFSC     0x20	// Enable Short FSC
					// 0: No short FSC is generated
					// 1: A short FSC is generated once per multi-frame (every 40th IOM frame)
  // IMPORTANT NOTE on the Undefined Bits 2-0:
  // The value of these bits depends on the selected mode. It is important to note that these
  // bits MUST NOT be overwritten to a different value when accessing this register.

// TR_CONF2 - Transceiver Configuration Register 2 (RD/WR, Default=0x80)
#define ISACSX_TR_CONF2        0x32
  // TR_CONF2 Bit Flags (unused bits are zero)
#define TR_CONF2_DIS_TX      0x80	// Disable Line Driver, 0: Transmitter is enabled, 1: Transmitter is disabled
#define TR_CONF2_PDS         0x40	// Phase Deviation Select - Defines the phase deviation of the S-transmitter.
					// 0: The phase deviation is 2 S-bits minus 7 oscillator periods plus 
					//    analog delay plus delay of the external circuitry.
					// 1: The phase deviation is 2 S-bits minus 9 oscillator periods plus 
					//    analog delay plus delay of the external circuitry.
#define TR_CONF2_RLP         0x10	// Remote Line Loop, 0: Remote Loop open, 1: Remote Loop closed
#define TR_CONF2_SGP         0x02	// Stop/Go Bit Polarity - Defines the polarity of the S/G bit output on pin SGO.
					// 0: low active (SGO=0 means 'go'; SGO=1 means 'stop')
					// 1: high active (SGO=1 means 'go'; SGO=0 means 'stop')
#define TR_CONF2_SGD         0x01	// Stop/Go Bit Duration - Defines the duration of the S/G bit output on pin SGO.
					// 0: active during the D-channel timeslot
					// 1: active during the whole corresponding IOM frame 
					//   (starts and ends with the beginning of the D-channel timeslot)

// TR_STA - Transceiver Status Register (RD, Default=0x00)
#define ISACSX_TR_STA          0x33
  // TR_STA Bit Flags (unused bits are zero)
#define TR_STA_RINF          0x80	// Receiver INFO
					// 00: Received INFO 0
					// 01: Received any signal except INFO 0,2,3,4
					// 10: Reserved (NT mode) or INFO 2 (TE mode)
					// 11: Received INFO 3 (NT mode) or INFO 4 (TE mode)    
#define TR_STA_SLIP          0x40	// SLIP Detected
					// A '1' in this bit position indicates that a SLIP 
					// is detected in the receive or transmit path.    
#define TR_STA_ICV           0x20	// Illegal Code Violation
					// 0: No illegal code violation is detected
					// 1: Illegal code violation (ANSI T1.605) in data stream is detected
#define TR_STA_FSYN          0x04	// Frame Synchronization State
					// 0: The S/T receiver is not synchronized
					// 1: The S/T receiver has synchronized to the framing bit F
#define TR_STA_LD            0x01	// Level Detection
					// 0: No receive signal has been detected on the line.
					// 1: Any receive signal has been detected on the line.

// TR_CMD - Transceiver Command Register (RD/WR, Default=0x08)
#define ISACSX_TR_CMD          0x34
  // TR_CMD Bit Flags (unused bits are zero)
#define TR_CMD_XINF          0xE0	// Transmit INFO
					// 000: Transmit INFO 0
					// 001: reserved
					// 010: Transmit INFO 1 (TE mode) or INFO 2 (NT mode)
					// 011: Transmit INFO 3 (TE mode) or INFO 4 (NT mode)
					// 100: Send continous pulses at 192 kbit/s alternating or 96 kHz rectangular, respectively (SCP)
					// 101: Send single pulses at 4 kbit/s with alternating polarity corresponding to 2 kHz fundamental mode (SSP)
					// 11x: reserved
#define TR_CMD_DPRIO         0x10	// D-Channel Priority (always writable in Int. NT mode)
					// 0: Priority Class 1for D channel access on IOM (Int. NT) or on S interface (TE/LT-T)
					// 1: Priority Class 2 for D channel access on IOM (Int. NT) or on S interface (TE/LT-T)
#define TR_CMD_TDDIS         0x08	// TDDIS ... Transmit Data Disabled (TE mode)
					// 0: The B and D channel data are transparently transmitted on the S/T interface if INFO 3 is being transmitted
					// 1: The B and D channel data are set to logical '1' on the S/T interface if INFO 3 is being transmitted
#define TR_CMD_PD            0x04	// Power Down
					// 0: The transceiver is set to operational mode
					// 1: The transceiver is set to power down mode
#define TR_CMD_LD_A          0x02	// Loop Analog - The setting of this bit corresponds to the C/I command ARL.
					// 0: Analog loop is open
					// 1: Analog loop is closed internally or externally according to the EXLP bit in the TR_CONF0 register

// SQRR1 - S/Q-Channel Receive Register 1 (RD, Default=0x40)
#define ISACSX_SQRR1           0x35
  // SQRR1 Bit Flags (unused bits are zero)
#define SQRR1_MSYN           0x80	// Multi-frame Synchronization State
					// 0: The S/T receiver has not synchronized to the received FA and M bits
					// 1: The S/T receiver has synchronized to the received FA and M bits
#define SQRR1_MFEN           0x40	// Multiframe Enable
					// Read-back of the MFEN bit of the SQXR register
#define SQRR1_SQR            0x0F	// Received S Bits
					// Received S bits in frames 1, 6, 11 and 16 (TE mode)
					// Received Q bits in frames 1, 6, 11 and 16 (NT mode)

// SQXR1 - S/Q-Channel Transmit Register 1 (WR, Default=0x4F)
#define ISACSX_SQXR1           0x35
  // SQXR1 Bit Flags (unused bits are zero)
#define SQXR1_MFEN           0x40	// Multiframe Enable - Used to enable or disable the multiframe structure (Readback value in SQRR1)
					// 0: S/T multiframe is disabled
					// 1: S/T multiframe is enabled
#define SQXR1_SQX            0x0F	// Transmitted S/Q Bits
					// Transmitted Q bits (FA bit position) in frames 1, 6, 11 and 16 (TE mode)
					// Transmitted S bits (FA bit position) in frames 1, 6, 11 and 16 (NT mode)

// SQRR2 - S/Q-Channel Receive Register 2 (RD, Default=0x00)
#define ISACSX_SQRR2           0x36
  // SQR21-24, SQR31-34 - Received S Bits (TE mode only)
  // Received S bits in frames 2, 7, 12 and 17 (SQR21-24, subchannel 2),
  // and in frames 3, 8, 13 and 18 (SQR31-34, subchannel 3).

// SQXR2 - S/Q-Channel Transmit Register 2 (WR, Default=0x00)
#define ISACSX_SQXR2           0x36
  // SQX21-24, SQX31-34 - Transmitted S Bits (NT mode only)
  // Transmitted S bits in frames 2, 7, 12 and 17 (SQX21-24, subchannel 2),
  // and in frames 3, 8, 13 and 18 (SQX31-34, subchannel 3).

// SQRR3 - S/Q-Channel Receive Register 3 (RD, Default=0x00)
#define ISACSX_SQRR3           0x37
  // SQR41-44, SQR51-54 - Received S Bits (TE mode only)
  // Received S bits in frames 4, 9, 14 and 19 (SQR41-44, subchannel 4),
  // and in frames 5, 10, 15 and 20 (SQR51-54, subchannel 5).

// SQXR3 - S/Q-Channel Transmit Register 3 (WR, Default=0x00)
#define ISACSX_SQXR3           0x37
  // SQX41-44, SQX51-54 - Transmitted S Bits (NT mode only)
  // Transmitted S bits in frames 4, 9, 14 and 19 (SQX41-44, subchannel 4),
  // and in frames 5, 10, 15 and 20 (SQX51-54, subchannel 5).

// ISTATR - Interrupt Status Register Transceiver (RD, Default=0x00)
#define ISACSX_ISTATR          0x38
  // ISTATR Bit Flags
  // For all interrupts in the ISTATR register the following logical states are defined:
  // 0: Interrupt is not acitvated
  // 1: Interrupt is acitvated
  // Bits 7-4 are reserved - Bits set to '1' in this bit position must be ignored.
#define ISTATR_LD            0x08	// Level Detection - Any receive signal has been detected on the line.
					// This bit is set to '1' (i.e. an interrupt is generated if not masked) 
					// as long as any receiver signal is detected on the line.
#define ISTATR_RIC           0x04	// Receiver INFO Change
					// RIC is activated if one of the TR_STA bits RINF or ICV has changed.
					// This bit is reset by reading the register TR_STA.
#define ISTATR_SQC           0x02	// S/Q-Channel Change
					// A change in the received S-channel (TE) or Q-channel (NT) has been detected. The new
					// code can be read from the SQRxx bits of registers SQRR1-3 within the next multiframe
					// (5 ms). This bit is reset by a read access to the corresponding SQRRx register.
#define ISTATR_SQW           0x01	// S/Q-Channel Writable
					// The S/Q channel data for the next multiframe is writable.
					// The register for the Q (S) bits to be transmitted (received) has to be written (read) within
					// the next multiframes (5 ms). This bit is reset by writing register SQXRx.

// MASKTR - Mask Transceiver Interrupt Register (RD/WR, Default=0xFF)
#define ISACSX_MASKTR          0x39
  // MASKTR Bit Flags (unused bits are '1')
#define MASKTR_RESERVED      0xF0	// Reserved Mask Bits 7-4 are always 1
#define MASKTR_LD            0x08	// Mask Transceiver Interrupt for LD
#define MASKTR_RIC           0x04	// Mask Transceiver Interrupt for RIC
#define MASKTR_SQC           0x02	// Mask Transceiver Interrupt for SQC
#define MASKTR_SQW           0x01	// Mask Transceiver Interrupt for SQW

// TR_MODE - Transceiver Mode Register 1 (RD/WR, Default=000000xxb)
#define ISACSX_TR_MODE         0x3A
  // TR_MODE Bit Flags (unused bits are zero)
#define TR_MODE_DCH_INH      0x08	// D-Channel Inhibit
					// Setting this bit to '1' has the effect that the S-transceiver blocks 
					// the access to the Dchannel on S by inverting the E-bits.
#define TR_MODE_MODE         0x07	// Transceiver Mode
					// 000: TE mode
					// 001: LT-T mode
					// 010: NT mode (without D-channel handler)
					// 011: LT-S mode (without D-channel handler)
					// 110: Intelligent NT mode (with NT state machine and with D-channel handler)
					// 111: Intelligent NT mode (with LT-S state machine and with D-channel handler)
					// 100: reserved
					// 101: reserved
					// Note: The three modes TE, LT-T and LT-S can be selected by pin strapping 
					// (reset values for bits TR_MODE.MODE0,1 loaded from pins MODE0,1), 
					// all other modes are programmable only.

/////////////////////////////////////////////////////////////////////////////
// Auxiliary Interface Registers

// ACFG1 - Auxiliary Configuration Register 1 (RD/WR, Default=0x00)
#define ISACSX_ACFG1           0x3C
  // ACFG1 Bit Flags
  // OD7-0 - Output Driver Select for AUX7 - AUX0
  // 0: output is open drain
  // 1: output is push/pull
  // See also comment in documentation.

// ACFG2 - Auxiliary Configuration Register 2 (RD/WR, Default=0x00)
#define ISACSX_ACFG2           0x3D
  // ACFG2 Bit Flags
#define ACFG2_A7SEL          0x80	// AUX7 Function Select
					// 0: pin AUX7 provides normal I/O functionality.
					// 1: pin AUX7 provides the S/G bit output (SGO) from the IOM DD-line. 
#define ACFG2_A5SEL          0x40	// AUX5 Function Select
					// 0: pin AUX5 provides normal I/O functionality.
					// 1: pin AUX5 provides an FSC or BCL signal output (FBOUT) which is selected in ACFG2.FBS.
#define ACFG2_FBS            0x20	// FSC/BCL Output Select
					// 0: FSC is output on pin AUX5.
					// 1: BCL (single bit clock) is output on pin AUX5.
#define ACFG2_A4SEL          0x10	// AUX4 Function Select
					// 0: pin AUX4 provides normal I/O functionality.
					// 1: pin AUX4 supports multiframe synchronization and is used as M-bit input in Int. NT/NT/LT-S modes or as M-bit output in TE/LT-T modes
#define ACFG2_ACL            0x08	// ACL Function Select
					// 0: Pin ACL automatically indicates the S-bus activation status by a LOW level.
					// 1: The output state of ACL is programmable by the host in bit LED.
#define ACFG2_LED            0x04	// LED Control - If enabled (ACL=1) the LED connected between VDD and ACL is switched ...
					// 0: Off (high level on pin ACL)
					// 1: On (low level on pin ACL)
#define ACFG2_EL1            0x02	// Edge/Level Triggered Interrupt Input for INT1, 0: negative level, 1: negative edge
#define ACFG2_EL0            0x01	// Edge/Level Triggered Interrupt Input for INT0, 0: negative level, 1: negative edge

// AOE - Auxiliary Output Enable Register (RD/WR, Default=0xFF)
#define ISACSX_AOE             0x3E
  // AOE Bit Flags
  // OE7-0 - Output Enable for AUX7 - AUX0
  // 0: Pin AUX7-0 is configured as output.
  // 1: Pin AUX7-0 is configured as input.
  // See also comment in documentation.
#define AOE_AUX0             0x01
#define AOE_AUX1             0x02
#define AOE_AUX2             0x04
#define AOE_AUX3             0x08
#define AOE_AUX4             0x10
#define AOE_AUX5             0x20
#define AOE_AUX6             0x40
#define AOE_AUX7             0x80

// ARX - Auxiliary Interface Receive Register (RD, Default=undefined)
#define ISACSX_ARX             0x3F
  // ARX Bit Flags
  // AR7-0 - Auxiliary Receive
  // The value of AR7-0 always reflects the level at pin AUX7-0 at the time when ARX is read
  // by the host even if a pin is configured as output. If the mask bit for AUX7, 6 is set in the
  // MASKA register, no interrupt is generated to the ISAC-SX, however, the current state at
  // pin AUX7,6 can be read from AR7,6
  // See also comment in documentation.
#define ARX_AUX0             0x01
#define ARX_AUX1             0x02
#define ARX_AUX2             0x04
#define ARX_AUX3             0x08
#define ARX_AUX4             0x10
#define ARX_AUX5             0x20
#define ARX_AUX6             0x40
#define ARX_AUX7             0x80

// ATX - Auxiliary Interface Transmit Register (WR, Default=0x00)
#define ISACSX_ATX             0x3F
  // ARX Bit Flags
  // AT7-0 - Auxiliary Transmit
  // A '0' or '1' in AT7-0 will drive a low or a high level at pin AUX7-0 if the corresponding
  // output is enabled in the AOE register.
  // See also comment in documentation.
#define ATX_AUX0             0x01
#define ATX_AUX1             0x02
#define ATX_AUX2             0x04
#define ATX_AUX3             0x08
#define ATX_AUX4             0x10
#define ATX_AUX5             0x20
#define ATX_AUX6             0x40
#define ATX_AUX7             0x80

/////////////////////////////////////////////////////////////////////////////
// IOM-2 and MONITOR Handler

// CDAxy - Controller Data Access Register xy (RD/WR, Default=0xFF)
#define ISACSX_CDA10           0x40
#define ISACSX_CDA11           0x41
#define ISACSX_CDA20           0x42
#define ISACSX_CDA21           0x43

// XXX_TSDPxy - Time Slot and Data Port Selection for CHxy (RD/WR)
#define ISACSX_CDA_TSDP10             0x44	// Default=0x00 ( = output on B1-DD)
#define ISACSX_CDA_TSDP11             0x45	// Default=0x01 ( = output on B2-DD)
#define ISACSX_CDA_TSDP20             0x46	// Default=0x80 ( = output on B1-DU)
#define ISACSX_CDA_TSDP21             0x47	// Default=0x81 ( = output on B2-DU)
#define ISACSX_BCH_TSDP_BC1           0x48	// Default=0x80 ( = output on B1-DU)
#define ISACSX_BCH_TSDP_BC2           0x49	// Default=0x81 ( = output on B2-DU)
#define ISACSX_TR_TSDP_BC1            0x4C	// Default=0x00 ( = transceiver output on B1-DD), see note
#define ISACSX_TR_TSDP_BC2            0x4D	// Default=0x01 ( = transceiver output on B2-DD), see note
  // Also refer to documentation for details.

  // XXX_TSDPxy Bit Flags (unused bits are zero)
#define TSDP_DPS             0x80	// Data Port Selection
					 // 0:The data channel xy of the functional unit XXX is output on DD.
					 //   The data channel xy of the functional unit XXX is input from DU.
					 // 1:The data channel xy of the functional unit XXX is output on DU.
					 //   The data channel xy of the functional unit XXX is input from DD.

#define TSDP_TSS_MASK        0x1F	// Timeslot Selection
					 // Selects one of 32 timeslots (0...31) on the IOM-2 interface for the data channels.
					 // Note: The TSS reset values for TR_TSDP_BC1/2 are determined by the channel select
					 // pins CH2-0 which are mapped to the corresponding bits TSS4-2.

// CDAx_CR - Control Register Controller Data Access CH1x (RD/WR, Default=0x00)
#define ISACSX_CDA1_CR            0x4E
#define ISACSX_CDA2_CR            0x4F
  // CDAx_CR Bit Flags
#define ISACSX_CDAx_CR_EN_TBM   0x20	// Enable TIC Bus Monitoring
					 // 0: The TIC bus monitoring is disabled
					 // 1: The TIC bus monitoring with the CDAx0 register is enabled. 
					 // The TSDPx0 register must be set to 08H for monitoring from DU or 88H for monitoring from DD, respectively
					 // (This selection is only valid if IOM_CR.TIC_DIS = 0).

#define ISACSX_CDAx_CR_EN_I1    0x10	// Enable Input CDAx1
					 // 0: The input of the CDAx1 register is disabled
					 // 1: The input of the CDAx1 register is enabled

#define ISACSX_CDAx_CR_EN_I0    0x08	// Enable Input CDAx0
					 // 0: The input of the CDAx0 register is disabled
					 // 1: The input of the CDAx0 register is enabled

#define ISACSX_CDAx_CR_EN_O1    0x04	// Enable Output CDAx1
					 // 0: The output of the CDAx1 register is disabled
					 // 1: The output of the CDAx1 register is enabled

#define ISACSX_CDAx_CR_EN_O0    0x02	// Enable Output CDAx0
					 // 0: The output of the CDAx0 register is disabled
					 // 1: The output of the CDAx0 register is enabled

#define ISACSX_CDAx_CR_SWAP     0x01	// Swap Inputs
					 // 0:The time slot and data port for the input of the CDAxy register is defined by its own
					 //   TSDPxy register. The data port for the CDAxy input is vice versa to the output setting for CDAxy.
					 // 1:The input (time slot and data port) of the CDAx0 is defined by the TSDP register of
					 //   CDAx1 and the input of CDAx1 is defined by the TSDP register of CDAx0. The data
					 //   port for the CDAx0 input is vice versa to the output setting for CDAx1. The data port
					 //   for the CDAx1 input is vice versa to the output setting for CDAx0. The input definition
					 //   for time slot and data port CDAx0 are thus swapped to CDAx1 and for CDAx1 to
					 //   CDAx0. The outputs are not affected by the SWAP bit.

// TR_CR - Control Register Transceiver Data (IOM_CR.CI_CS=0) (RD/WR, Default=0xF8)
// Note:  Read and write access to this register is only possible if IOM_CR.CI_CS = 0.
#define ISACSX_TR_CR           0x50
  // TR_CR Bit Flags
  // EN_xxx: This register is used to individually enable/disable the D-channel (both RX and TX direction)
  //         and the receive/transmit paths for the B-channel of the S-transceiver.
#define TR_CR_EN_D           0x80	// Enable Transceiver D-Channel Data
#define TR_CR_EN_B2R         0x40	// Enable Transceiver B2 Receive Data
#define TR_CR_EN_B1R         0x20	// Enable Transceiver B1 Receive Data
#define TR_CR_EN_B2X         0x10	// Enable Transceiver B2 Transmit Data
#define TR_CR_EN_B1X         0x08	// Enable Transceiver B1 Transmit Data
  // 0: The corresponding data path to the transceiver is disabled.
  // 1: The corresponding data path to the transceiver is enabled.
  // Note: Receive data corresponds to downstream direction, and transmit data corresponds to upstream direction.

  // CS2-0: This register is used to select one of eight IOM channels 
  //        to which the transceiver D-channel data is related to.
#define TR_CR_CS_MASK        0x07	// Channel Select for Transceiver D-channel
  // Note: The reset value is determined by the channel select pins CH2-0 which are directly
  // mapped to CS2-0. It should be noted that writing TR_CR.CS2-0 will also write to
  // TRC_CR.CS2-0 and therefore modify the channel selection for the transceiver C/I0 data.

// TRC_CR - Control Register Transceiver C/I0 (IOM_CR.CI_CS=1) (RD/WR, Default=0x00)
// Note:  Write access to this register is possible if IOM_CR.CI_CS = 0 or IOM_CR.CI_CS = 1.
//        Read access to this register is possible only if IOM_CR.CI_CS = 1.
#define ISACSX_TRC_CR          0x50
  // TRC_CR Bit Flags
  // CS2-0: This register is used to select one of eight IOM channels to which the transceiver C/I0
  //        channel data is related to. The reset value is determined by the MODE2-bit and the
  //        channel select pins CH2-0 which are mapped to CS2-0.
#define TRC_CR_CS_MASK       0x07	// Channel Select for the Transceiver C/I0 Channel

// BCHx_CR - Control Register B-Channel Controller Data (RD/WR, Default=0x08)
#define ISACSX_BCHA_CR          0x51
#define ISACSX_BCHB_CR          0x52
  // BCH_CR Bit Flags
#define BCH_CR_DPS_D         0x80	// Data Port Selection for D-Channel Timeslot access
					// 0: The B-channel controller data is output on DD.
					//    The B-channel controller data is input from DU.
					// 1: The B-channel controller data is output on DU.
					//    The B-channel controller data is input from DD.
#define BCH_CR_EN_D          0x20	// Enable D-Channel Timeslot (2-bit) for B-Channel controller access
#define BCH_CR_EN_BC2        0x10	// Enable B2-Channel Timeslot (8-bit) for B-Channel controller access
#define BCH_CR_EN_BC1        0x08	// Enable B1-Channel Timeslot (8-bit) for B-Channel controller access
					// 0: B-channel B/A does not access timeslot data B1, B2 or D, respectively.
					// 1: B-channel B/A does access timeslot data B1, B2 or D, respectively.
  // Note: The terms B1/B2 should not imply that the 8-bit timeslots must be located in the
  // first/second IOM-2 timeslots, it's simply a placeholder for the 8-bit timeslot position
  // selected in the registers BCH_TSDP_BC1/2.

  // CS2-0: This register is used to select one of eight IOM channels. If enabled (EN_D=1), the
  //        B-channel controller is connected to the 2-bit D-channel timeslot of that IOM channel.
#define BCH_CR_CS_MASK       0x07	// Channel Select for D-Channel Timeslot access
  // Note: The reset value is determined by the channel select pins CH2-0 which are directly mapped to CS2-0.

// DCI_CR - Control Register for D and CI1 Handler (IOM_CR.CI_CS=0) (RD/WR, Default=0xA0)
// Note: Read and write access to this register is only possible if IOM_CR.CI_CS = 0.
#define ISACSX_DCI_CR          0x53
  // DCI_CR Bit Flags
#define DCI_DPS_CL1          0x80	// Data Port Selection CI1 Handler Data
					// 0: The CI1 handler data is output on DD and input from DU
					// 1: The CI1 handler data is output on DU and input from DD
#define DCI_CR_EN_CL1        0x40	// Enable CI1 Handler Data
					// 0: CI1 handler data access is disabled
					// 1: CI1 handler data access is enabled
					// Note: The timeslot for the C/I1 handler is fixed to IOM channel 1.
#define DCI_CR_D_EN_D        0x20	// Enable D-timeslot for D-channel controller
#define DCI_CR_D_EN_B2       0x10	// Enable B2-timeslot for D-channel controller
#define DCI_CR_D_EN_B1       0x08	// Enable B1-timeslot for D-channel controller
					// 0: D-channel controller does not access timeslot data B1, B2 or D, respectively
					// 1: D-channel controller does access timeslot data B1, B2 or D, respectively

  // CS2-0: This register is used to select one of eight IOM channels. If enabled,
  //        the D-channel data is connected to the corresponding timeslots of that IOM channel,
#define DCI_CR_CS_MASK       0x07	// Channel Select for D-channel controller
  // Note: The reset value is determined by the channel select pins CH2-0 which are directly
  //       mapped to CS2-0. It should be noted that writing DCI_CR.CS2-0 will also write to
  //       DCIC_CR.CS2-0 and therefore modify the channel selection for the data of the C/I0 handler.

// DCIC_CR - Control Register for CI0 Handler (IOM_CR.CI_CS=1) (RD/WR, Default=0x00)
// Note:  Write access to this register is possible if IOM_CR.CI_CS = 0 or IOM_CR.CI_CS = 1.
//        Read access to this register is possible only if IOM_CR.CI_CS = 1.
#define ISACSX_DCIC_CR         0x13
  // DCIC_CR Bit Flags
  // CS2-0: This register is used to select one of eight IOM channels. If enabled, the data of the
  //        C/I0 handler is connected to the corresponding C/I0 timeslot of that IOM channel. The
  //        reset value is determined by the channel select pins CH2-0 which are mapped to CS2-0.
#define DCIC_CR_CS_MASK      0x07	// Channel Select for C/I0 Handler

// MON_CR - Control Register Monitor Data (RD/WR, Default=0x40)
// Note: Read and write access to this register is only possible if IOM_CR.CI_CS = 0.
#define ISACSX_MON_CR          0x54
  // MON_CR Bit Flags
#define MON_CR_DPS           0x80	// Data Port Selection
					// 0: The Monitor data is output on DD and input from DU
					// 1: The Monitor data is output on DU and input from DD
#define MON_CR_EN_MON        0x40	// Enable Output
					// 0: The Monitor data input and output is disabled
					// 1: The Monitor data input and output is enabled
#define MON_CR_CS_MASK       0x07	// CS2-0, MONITOR Channel Selection
  // 000: The MONITOR data is input/output on MON0 (3rd timeslot on IOM-2)
  // 001: The MONITOR data is input/output on MON1 (7th timeslot on IOM-2)
  // 010: The MONITOR data is input/output on MON2 (11th timeslot on IOM-2)
  // :
  // 111: The MONITOR data is input/output on MON7 (31st timeslot on IOM-2)
  // Note: The reset value is determined by the channel select pins CH2-0 which are directly
  // mapped to CS2-0.

// SDSx_CR - Control Register Serial Data Strobe x (RD/WR)
#define ISACSX_SDS1_CR                0x55	// Default=0x00
#define ISACSX_SDS2_CR                0x56	// Default=0x00
  // Also refer to documentation for details.

  // SDSx_CR Bit Flags (unused bits are zero)
#define ISACSX_SDSx_CR_ENS_TSS      0x80	// Enable Serial Data Strobe of timeslot TSS
#define ISACSX_SDSx_CR_ENS_TSS1     0x40	// Enable Serial Data Strobe of timeslot TSS+1
					// 0: The serial data strobe signal SDSx is inactive during TSS, TSS+1
					// 1: The serial data strobe signal SDSx is active during TSS, TSS+1
#define ISACSX_SDSx_CR_ENS_TSS3     0x20	// Enable Serial Data Strobe of timeslot TSS+3 (D-Channel)
					// 0: The serial data strobe signal SDSx is inactive during the D-channel (bit7, 6) of TSS+3
					// 1: The serial data strobe signal SDSx is active during the D-channel (bit7, 6) of TSS+3

#define ISACSX_SDSx_CR_TSS_MASK     0x1F	// Timeslot Selection
					// Selects one of 32 timeslots on the IOM-2 interface (with respect to FSC) during which
					// SDSx is active high or provides a strobed BCL clock output (see SDS_CONF.SDS1/2_BCL). 
					// The data strobe signal allows standard data devices to access a programmable channel.

// IOM_CR - Control Register IOM Data (RD/WR, Default=0x08)
#define ISACSX_IOM_CR            0x57
  // IOM_CR Bit Flags
#define ISACSX_IOM_CR_SPU      0x80	// Software Power Up
					// 0: The DU line is normally used for transmitting data
					// 1: Setting this bit to '1' will pull the DU line to low.
					//    This will enforce connected layer 1 devices to deliver IOM-clocking
#define ISACSX_IOM_CR_DIS_AW   0x40	// Disable Asynchronous Awake (NT, LT-S, Int. NT mode only)
					// Setting this bit to '1' disables the Asynchronous Awake function of the transceiver.
#define ISACSX_IOM_CR_CI_CS    0x20	// C/I Channel Selection
					// 0: A write access to CS2-0 has effect on the configuration of D- and C/I-channel,
					// whereas a read access delivers the D-channel configuration only.
					// 1: A write access to CS2-0 has effect on the configuration of the C/I-channel only,
					// whereas a read access delivers the C/I-channel configuration only.
#define ISACSX_IOM_CR_TIC_DIS  0x10	// TIC Bus Disable
					// 0: The last octet of IOM channel 2 (12th timeslot) is used as TIC bus 
					//    (in a frame timing with 12 timeslots only).
					// 1: The TIC bus is disabled. The last octet of the last IOM time slot
					//    (TS 11) can be used as every time slot.
#define ISACSX_IOM_CR_EN_BCL   0x08	// Enable Bit Clock BCL/SCLK, 0: The BCL/SCLK clock is disabled, 1: The BCL/SCLK clock is enabled.
#define ISACSX_IOM_CR_CLKM     0x04	// Clock Mode, 0: A double bit clock is connected to DCL, 1: A single bit clock is connected to DCL
#define ISACSX_IOM_CR_DIS_OD   0x02	// Disable Open Drain Drivers, 0: DU/DD are open drain drivers, 1: DU/DD are push pull drivers
#define ISACSX_IOM_CR_DIS_IOM  0x01	// Disable IOM, 0: The IOM interface is enabled, 1: The IOM interface is disabled

// STI - Synchronous Transfer Interrupt (RD, Default=0x00)
#define ISACSX_STI               0x58
  // STI Bit Flags
  // For all interrupts in the STI register the following logical states are applied:
  // 0: Interrupt is not activated
  // 1: Interrupt is activated
  // Note: The interrupts are automatically reset by reading the STI register.
  // For more information refer to the documentation
#define ISACSX_STI_STOVxy_MASK 0xF0	// Synchronous Transfer Overflow Interrupt
#define ISACSX_STI_STIxy_MASK  0x0F	// Synchronous Transfer Interrupt

// ASTI - Acknowledge Synchronous Transfer Interrupt (RD, Default=0x00)
#define ISACSX_ASTI              0x58
  // ASTI Bit Flags
  // After an STIxy interrupt the microcontroller has to acknowledge the interrupt 
  // by setting the corresponding ACKxy bit to '1'.  
  // For more information refer to the documentation
#define ISACSX_ASTI_ACKxy_MASK 0x0F	// ACKxy ... Acknowledge Synchronous Transfer Interrupt

// MSTI - Mask Synchronous Transfer Interrupt (RD/WR, Default=0xFF)
#define ISACSX_MSTI               0x59
  // MSTI Bit Flags
  // For the MSTI register the following logical states are applied:
  // 0: Interrupt is not masked
  // 1: Interrupt is masked  
  // For more information refer to the documentation
#define ISACSX_MSTI_STOVxy_MASK 0xF0	// Synchronous Transfer Overflow for STIxy
#define ISACSX_MSTI_STIxy_MASK  0x0F	// Synchronous Transfer Interrupt xy

// SDS_CONF - Configuration Register for Serial Data Strobes (RD/WR, Default=0x00)
#define ISACSX_SDS_CONF           0x5A
  // SDS_CONF Bit Flags
#define ISACSX_SDS_CONF_DIOM_INV 0x08	// DU/DD on IOM Timeslot Inverted
					// 0: DU/DD are active during SDS1 HIGH phase and inactive during the LOW phase.
					// 1: DU/DD are active during SDS1 LOW phase and inactive during the HIGH phase.
					// Note: This bit has only effect if DIOM_SDS is set to '1' otherwise DIOM_INV is don't care.
#define ISACSX_SDS_CONF_DIOM_SDS 0x04	// DU/DD on IOM Controlled via SDS1
					// 0: The pin SDS1 and its configuration settings are used for serial data strobe only.
					//    The IOM-2 data lines are not affected.
					// 1: The DU/DD lines are deactivated during the during High/Low phase (selected via DIOM_INV) 
					//    of the SDS1 signal. The SDS1 timeslot is selected in SDS1_CR.
#define ISACSX_SDS_CONF_SDS2_BCL 0x02	// Enable IOM Bit Clock for SDS2
					// 0: The serial data strobe is generated in the programmed timeslot.
					// 1: The IOM bit clock is generated in the programmed timeslot.
#define ISACSX_SDS_CONF_SDS1_BCL 0x01	// Enable IOM Bit Clock for SDS1
					// 0: The serial data strobe is generated in the programmed timeslot.
					// 1: The IOM bit clock is generated in the programmed timeslot.

// MCDA - Monitoring CDA Bits (RD, Default=0xFF)
#define ISACSX_MCDA                0x5B
  // MCDA Bit Flags
#define ISACSX_MCDA_MCDA21       0xC0	// Monitoring CDA21 Bits
#define ISACSX_MCDA_MCDA20       0x30	// Monitoring CDA20 Bits
#define ISACSX_MCDA_MCDA11       0x0C	// Monitoring CDA11 Bits
#define ISACSX_MCDA_MCDA10       0x03	// Monitoring CDA10 Bits
  // Note: Bit 7 and Bit 6 of the CDAxy registers are mapped into the MCDA register.
  //       This can be used for monitoring the D-channel bits on DU and DD and the 
  //       'Echo bits' on the TIC bus with the same register

// MOR - MONITOR Receive Channel (RD, Default=0xFF)
#define ISACSX_MOR                 0x5C
  // Note: Contains the MONITOR data received in the IOM-2 MONITOR channel according to the
  //       MONITOR channel protocol. The MONITOR channel (0-7) can be selected by setting the
  //       monitor channel select bit MON_CR.MCS.

// MOX - MONITOR Transmit Channel (WR, Default=0xFF)
#define ISACSX_MOX                 0x5C
  // Note: Contains the MONITOR data to be transmitted in IOM-2 MONITOR channel according
  //       to the MONITOR channel protocol.The MONITOR channel (0-7) can be selected by
  //       setting the monitor channel select bit MON_CR.MCS

// MOSR - MONITOR Interrupt Status Register (RD, Default=0x00)
#define ISACSX_MOSR               0x5D
  // MOSR Bit Flags
#define ISACSX_MOSR_MDR         0x80	// MONITOR channel Data Received
#define ISACSX_MOSR_MER         0x40	// MONITOR channel End of Reception
#define ISACSX_MOSR_MDA         0x20	// MONITOR channel Data Acknowledged
#define ISACSX_MOSR_MAB         0x10	// MONITOR channel Data Abort

// MOCR - MONITOR Control Register (RD/WR, Default=0x00)
#define ISACSX_MOCR               0x5E
  // MOCR Bit Flags
#define ISACSX_MOSR_MRE         0x80	// MONITOR Receive Interrupt Enable
					// 0: MONITOR interrupt status MDR generation is masked
					// 1: MONITOR interrupt status MDR generation is enabled
#define ISACSX_MOSR_MRC         0x40	// MR Bit Control, Determines the value of the MR bit:
					// 0: MR is always '1'. In addition, the MDR interrupt is blocked, except for the first byte of a packet (if MRE = 1).
					// 1: MR is internally controlled by the ISAC-SX according to MONITOR channel protocol.
					//    In addition, the MDR interrupt is enabled for all received bytes according to the MONITOR channel protocol (if MRE = 1).
#define ISACSX_MOSR_MIE         0x20	// MONITOR Interrupt Enable
					// MONITOR interrupt status MER, MDA, MAB generation is enabled (1) or masked (0).
#define ISACSX_MOSR_MXC         0x10	// MX Bit Control, Determines the value of the MX bit:
					// 0: The MX bit is always '1'.
					// 1: The MX bit is internally controlled by the ISAC-SX according to MONITOR channel protocol.

// MSTA - MONITOR Status Register (RD, Default=0x00)
#define ISACSX_MSTA               0x5F
  // MSTA Bit Flags
#define ISACSX_MSTA_MAC         0x04	// MONITOR Transmit Channel Active (The data transmisson in the MONITOR channel is in progress)
#define ISACSX_MSTA_TOUT        0x01	// Time-Out (Read-back value of the TOUT bit)

// MCONF - MONITOR Configuration Register (WR, Default=0x00)
#define ISACSX_MCONF              0x5F
  // MCONF Bit Flags
#define ISACSX_MCONF_TOUT       0x01	// Time-Out
					// 0: The monitor time-out function is disabled
					// 1: The monitor time-out function is enabled

/////////////////////////////////////////////////////////////////////////////
// Interrupt and General Configuration

// ISTA - Interrupt Status Register (RD, Default=0x00)
#define ISACSX_ISTA               0x60
  // ISTA Bit Flags
  // For all interrupts in the ISTA register following logical states are applied:
  // 0: Interrupt is not acitvated
  // 1: Interrupt is acitvated
#define ISACSX_ISTA_ICB                0x80	// HDLC Interrupt from B-channel has been recognized
#define ISACSX_ISTA_ST                 0x20	// Synchronous Transfer
#define ISACSX_ISTA_CIC                0x10	// C/I Channel Change
#define ISACSX_ISTA_AUX                0x08	// Auxiliary Interrupts
#define ISACSX_ISTA_TRAN               0x04	// Transceiver Interrupt
#define ISACSX_ISTA_MOS                0x02	// MONITOR Status
#define ISACSX_ISTA_ICD                0x01	// HDLC Interrupt from D-channel has been recognized
  // Bit 6 is zero
  // Note: A read of the ISTA register clears none of the interrupts.
  //       They are only cleared by reading the corresponding status register.

// MASK - Mask Register (WR, Default=0xFF)
#define ISACSX_MASK               0x60
  // MASK Bit Flags
  // For the MASK register following logical states are applied:
  // 0: Interrupt is enabled
  // 1: Interrupt is disabled  
#define ISACSX_MASK_ICB                0x80	// Mask HDLC Interrupt from B-channel
#define ISACSX_MASK_ST                 0x20	// Mask Synchronous Transfer
#define ISACSX_MASK_CIC                0x10	// Mask C/I Channel Change
#define ISACSX_MASK_AUX                0x08	// Mask Auxiliary Interrupts
#define ISACSX_MASK_TRAN               0x04	// Mask Transceiver Interrupt
#define ISACSX_MASK_MOS                0x02	// Mask MONITOR Status
#define ISACSX_MASK_ICD                0x01	// Mask HDLC Interrupt from D-channel
  // Bit 6 is one
  // Note: In the event of a C/I channel change, CIC is set in ISTA even if the 
  //       corresponding mask bit in MASK is set, but no interrupt is generated.

// AUXI - Auxiliary Interrupt Status Register (RD, Default=0x00)
#define ISACSX_AUXI               0x61
  // AUXI Bit Flags
  // For all interrupts in the ISTA register following logical states are applied:
  // 0: Interrupt is not acitvated
  // 1: Interrupt is acitvated
#define AUXI_EAW                0x20	// External Awake Interrupt, An interrupt from the EAW pin has been detected.
#define AUXI_WOV                0x10	// Watchdog Timer Overflow
#define AUXI_TIN2               0x08	// Timer Interrupt 2
#define AUXI_TIN1               0x04	// Timer Interrupt 1
#define AUXI_INT1               0x02	// Auxiliary Interrupt from external devices 1
#define AUXI_INT0               0x01	// Auxiliary Interrupt from external devices 0
  // Bits 7 and 6 are zero

// AUXM - Auxiliary Mask Register (WR, Default=0xFF)
#define ISACSX_AUXM               0x61
  // AUXM Bit Flags
  // For the MASK register following logical states are applied:
  // 0: Interrupt is enabled
  // 1: Interrupt is disabled  
#define AUXM_EAW                0x20	// External Awake Interrupt, An interrupt from the EAW pin has been detected.
#define AUXM_WOV                0x10	// Watchdog Timer Overflow
#define AUXM_TIN2               0x08	// Timer Interrupt 2
#define AUXM_TIN1               0x04	// Timer Interrupt 1
#define AUXM_INT1               0x02	// Auxiliary Interrupt from external devices 1
#define AUXM_INT0               0x01	// Auxiliary Interrupt from external devices 0
  // Bits 7 and 6 are one

// MODE1 - Mode1 Register (RD/WR, Default=0x00)
#define ISACSX_MODE1              0x62
  // MODE1 Bit Flags
#define MODE1_WTC1              0x10	// Watchdog Timer Control 1
#define MODE1_WTC2              0x08	// Watchdog Timer Control 2
#define MODE1_CFS               0x04	// Configuration Select
					// 0: The IOM interface clock and frame signals are always active, "Power Down" state included.
					// 1: The IOM interface clock and frame signals are normally inactive ("Power Down").
#define MODE1_RSS2              0x02	// Reset Source Selection 2
#define MODE1_RSS1              0x01	// Reset Source Selection 1
  // Bits 7-5 are zero
  // Refer to documentation for details.

// MODE2 - Mode2 Register (RD/WR, Default=0x00)
#define ISACSX_MODE2              0x63
  // MODE2 Bit Flags
#define ISACSX_MODE2_INT_POL    0x08	// Interrupt Polarity, Selects the polarity of the interrupt pin INT.
					// 0: low active with open drain characteristic (default)
					// 1: high active with push pull characteristic
#define ISACSX_MODE2_PPSDX      0x01	// Push/Pull Output for SDX (SCI Interface)
					// 0: The SDX pin has open drain characteristic
					// 1: The SDX pin has push/pull characteristic

// Supported Isac Chip Version definition
#define ISAC_CHIP_VERSION_14      0x01

// ID - Identification Register (RD, Default=0x01)
#define ISACSX_ID                 0x64
  // ID Bit Flags
#define ISACSX_ID_DESIGN_MASK   0x3F	// Design Number
  // The design number allows to identify different hardware designs of the ISAC-SX by software.
  // 01H: V 1.4 (all other codes reserved)

// SRES - Software Reset Register (WR, Default=0x00)
#define ISACSX_SRES               0x64
  // ID Bit Flags
#define ISACSX_SRES_RES_CI      0x80	// Reset C/I-handler
#define ISACSX_SRES_RES_BCH     0x40	// Reset B-channel
#define ISACSX_SRES_RES_MON     0x10	// Reset Monitor channel
#define ISACSX_SRES_RES_DCH     0x08	// Reset D-channel
#define ISACSX_SRES_RES_IOM     0x04	// Reset IOM handler
#define ISACSX_SRES_RES_TR      0x02	// Reset S-transceiver
#define ISACSX_SRES_RES_RSTO    0x01	// Reset RSTO
// Setting one of these bits to '1' causes the corresponding block to be reset 
// for a duration of 4 BCL clock cycles, except RES_RSTO which is activated 
// for a duration of 125 ... 250 µs. The bits are automatically reset to '0' again.

// TIMR2 - Timer 2 Register (RD/WR, Default=0x00)
#define ISACSX_TIMR2           0x65
  // TIMR2 Bit Definitions
#define TIMR2_TMD     			 0x80	// Timer Mode
					// 0: Count Down Timer. An interrupt is generated only once after a time period of 1 ... 63 ms.
					// 1: Periodic Timer. An interrupt is periodically generated every 1 ... 63 ms (see CNT).
#define TIMR2_CNT_MASK			 0x3F	// Timer Counter
					// 0:    Timer off.
					// 1-63: Timer period = 1 - 63 ms
  // By writing '0' to CNT the timer is immediately stopped. 
  // A value different from that determines the time period after which an interrupt will be generated.
  // If the timer is already started with a certain CNT value and is written again before an
  // interrupt has been released, the timer will be reset to the new value and restarted again.
  // An interrupt is indicated to the host in AUXI.TIN2.

  // Note: Reading back this value delivers back the current counter value which may differ
  //       from the programmed value if the counter is running.

/////////////////////////////////////////////////////////////////////////////
// B-Channel Registers

// ISTAB - Interrupt Status Register B-Channel (RD, Default=0x10)
#define ISACSX_ISTAB           0x70
  // ISTAB Bit Flags
#define ISTAB_RME            0x80	// Receive Message End
#define ISTAB_RPF            0x40	// Receive Pool Full
#define ISTAB_RFO            0x20	// Receive Frame Overflow
#define ISTAB_XPR            0x10	// Transmit Pool Ready
#define ISTAB_XDU            0x04	// Transmit Data Underrun
  // Bits 0,1 and 3 are zero

// MASKB - Interrupt Mask Register B-Channel (WR, Default=0xFF)
#define ISACSX_MASKB           0x70
  // MASKB Bit Flags
#define MASKB_RME            0x80	// Mask Receive Message End
#define MASKB_RPF            0x40	// Mask Receive Pool Full
#define MASKB_RFO            0x20	// Mask Receive Frame Overflow
#define MASKB_XPR            0x10	// Mask Transmit Pool Ready
#define MASKB_XDU            0x04	// Mask Transmit Data Underrun
#define MASKB_RESERVED       0x0B	// Reserved Mask Bits 0,1 and 3 are always 1

// STARB - Status Register B-Channel (RD, Default=0x40)
#define ISACSX_STARB           0x71
  // STARB Bit Flags (unused bits are zero)
#define STARB_XDOV           0x80	// Transmit Data Overflow
#define STARB_XFW            0x40	// Transmit FIFO Write Enable
#define STARB_RACI           0x08	// Receiver Active Indication
#define STARB_XACI           0x02	// Transmiter Active Indication

// CMDRB - Command Register B-Channel (WR, Default=0x00)
#define ISACSX_CMDRB           0x71
  // CMDRD Bit Flags (unused bits are zero)
#define CMDRB_RMC            0x80	// Receive Message Complete
#define CMDRB_RRES           0x40	// Receiver Reset
#define CMDRB_XTF            0x08	// Transmit Transparent Frame
#define CMDRB_XME            0x02	// Transmit Message End
#define CMDRB_XRES           0x01	// Transmitter Reset

// MODEB - Mode Register (RD/WR, Default=0xC0)
#define ISACSX_MODEB           0x72
  // MODEB Bit Flags (unused bits are zero)
  // MDS2-0, Mode Select
#define MODEB_MDS_MASK       0xE0	// Mode Select Mask
#define MODEB_MDS_NONAUTO1   0x40	// Non-Auto Mode, One-byte address compare
#define MODEB_MDS_NONAUTO2   0x60	// Non-Auto Mode, Two-byte address compare
#define MODEB_MDS_EXTTRANS   0x80	// Extended Transparent Mode
#define MODEB_MDS_TRANS0     0xC0	// No address compare, all frames accepted
#define MODEB_MDS_TRANS1     0xE0	// High-byte address compare
#define MODEB_MDS_TRANS2     0xA0	// Low-byte address compare

#define MODEB_RAC            0x08	// Receiver Active

// EXMB - Extended Mode Register B-Channel (RD/WR, Default=0xC0)
#define ISACSX_EXMB            0x73
  // EXMB Bit Flags
#define EXMB_RESERVED1       0xC0	// Reserved '1' bits
#define EXMB_RFBS            0x20	// Receive FIFO Block Size, 0: Block size is 16 byte, 1: Block size is 8 byte
#define EXMB_SRA             0x10	// Store Receive Address in RFIFOB (0=Don't Store, 1=Store)
#define EXMB_XCRC            0x08	// Transmit CRC (0=Transmit, 1=Don't Transmit)
#define EXMB_RCRC            0x04	// Store CRC in RFIFOB (0=Don't Store, 1=Store)
#define EXMB_RESERVED0       0x02	// Reserved '0' bit
#define EXMB_ITF             0x01	// Interframe Time Fill (0=idle cont. 1s, 1=flags seq. of patterns '01111110')

// RAH1 - RAH1 Register (WR, Default=0x00)
#define ISACSX_RAH1            0x75
  // RAH1 Bit Flags (unused bits are zero)
#define RAH1_RAH1_MASK       0xFC	// Mask for Value of the first individual programmable high address byte
#define RAH1_MHA             0x01	// Mask High Address

// RAH2 - RAH2 Register (WR, Default=0x00)
#define ISACSX_RAH2            0x76
  // RAH2 Bit Flags (unused bits are zero)
#define RAH2_RAH2_MASK       0xFC	// Mask for Value of the second individual programmable high address byte
#define RAH2_MLA             0x01	// Mask Low Address

// RBCLB - Receive Frame Byte Count Low B-Channel Register (RD, Default=0x00)
#define ISACSX_RBCLB           0x76
  // RBC7-0 : Receive Byte Count
  // Eight least significant bits of the total number of bytes in a received message (see RBCHB register).

// RBCHB - Receive Frame Byte Count High B-Channel Register (RD, Default=0x00)
#define ISACSX_RBCHB           0x77
  // RBCHB Bit Flags (unused bits are zero)
#define RBCHB_OV             0x10	// Overflow - A '1' in this bit position indicates
					// a message longer than (212 - 1) = 4095 bytes
#define RBCHB_RBCHI_MASK     0x0F	// Mask for Hi-Value of RBC (Receive Byte Count)
  // Note: Normally RBCHB and RBCLB should be read by the microcontroller after an RMEinterrupt
  // in order to determine the number of bytes to be read from the RFIFOB,
  // and the total message length. The contents of the registers are valid only after an
  // RME or RPF interrupt, and remain so until the frame is acknowledged via the RMC
  // bit or RRES.
  // (add macros that generate the correct register value if we use this)

// RAL1 - RAL1 Register 1 (WR, Default=0x00)
#define ISACSX_RAL1            0x77
  // Receive Address Byte Low Register 1
  // The general function (READ/WRITE) and the meaning or contents of this register
  // depends on the selected operating mode:
  // - Non-auto mode (16-bit address):
  // RAL1 can be programmed with the value of the first individual low address byte.
  // - Non-auto mode (8-bit address):
  // According to X.25 LAPB protocol, the address in RAL1 is recognized as COMMAND address.

// RAL2 - RAL2 Register (WR, Default=0x00)
#define ISACSX_RAL2            0x78
  // Receive Address Byte Low Register 2
  // Value of the second individual programmable low address byte. If a one byte address
  // field is selected, RAL2 is recognized as RESPONSE according to X.25 LAPB protocol.

// RSTAB - Receive Status Register B-Channel (RD, Default=0x0E)
#define ISACSX_RSTAB           0x78
  // RSTAB Bit Flags
#define RSTAB_VFR            0x80	// Valid Frame - The received frame is valid (1) or invalid (0).
#define RSTAB_RDO            0x40	// Receive Data Overflow - If RDO=1, at least one byte of the frame has been lost.
#define RSTAB_CRC            0x20	// CRC Check - The CRC is correct (1) or incorrect (0).
#define RSTAB_RAB            0x10	// Receive Message Aborted - The receive message was aborted by the remote station (1).
#define RSTAB_HA1            0x08	// High Byte Address Compare - See documentation for details
#define RSTAB_HA0            0x04	// High Byte Address Compare - See documentation for details
#define RSTAB_HA_MASK        (RSTAB_HA1 | RASTAB_HA0)	// Mask for HA1..HA0
					// 10 - RAH1 has been recognized
					// 00 - RAH2 has been recognized
					// 01 - group address has been recognized
#define RSTAB_CR             0x02	// Command/Response - contains the received frame's C/R bit (Bit1 in SAPI address)
#define RSTAB_LA             0x01	// Low Byte Address Compare; significant only in non automodes 8 and 16 and in transparent mode 2
					// 0: Group address has been recognized
					// 1: RAL1 or RAL2 has been recognized
  //  See documentation for details
  // (add macros that generate the correct register value if we use HA1-0, LA)

// TMB - Test Mode Register B-Channel (RD/WR, Default=0x00)
#define ISACSX_TMB             0x79
  // TMB Bit Flags (unused bits are zero)
#define TMB_TLP              0x01	// Test Loop
					// The TX path of layer-2 is internally connected with the RX path of layer-2. 
					// Data coming from the layer 1 controller will not be forwarded to the layer 2 controller.

// B-Channel FIFO Address-Range
// Note: The ISAC maintains an internal pointer for accessing the FIFOs, 
//       which is automatically incremented.
//       This means the address is doesn't matter when accessing the FIFOs.

// RFIFOB - B-Channel Receive FIFO (RD)
#define ISACSX_RFIFOB          0x7A
// XFIFOB - B-Channel Transmit FIFO (WR)
#define ISACSX_XFIFOB          0x7A

struct isdnl1;
void isacsx_hwinit(struct isdnl1 *isdnl1);

int isacsx_ioctl_get_info(struct isdnl1 *isdnl1, unsigned long arg);
int isacsx_ioctl_set_config(struct isdnl1 *isdnl1, unsigned long arg);
int isacsx_ioctl_get_config(struct isdnl1 *isdnl1, unsigned long arg);
int isacsx_ioctl_set_l1state(struct isdnl1 *isdnl1, unsigned long arg);
int isacsx_ioctl_get_l1state(struct isdnl1 *isdnl1, unsigned long arg);
int isacsx_ioctl_set_hwtest(struct isdnl1 *isdnl1, unsigned long arg);
int isacsx_ioctl_get_status(struct isdnl1 *isdnl1, unsigned long arg);
int isacsx_ioctl_set_ledstatus(struct isdnl1 *isdnl1, unsigned long arg);
int isacsx_ioctl_read_reg(struct isdnl1 *isdnl1, unsigned long arg);
int isacsx_ioctl_write_reg(struct isdnl1 *isdnl1, unsigned long arg);
int isacsx_ioctl_enable(struct isdnl1 *isdnl1);
int isacsx_ioctl_disable(struct isdnl1 *isdnl1);
int isacsx_ioctl_set_busmode(struct isdnl1 *isdnl1, unsigned long arg);

void isacsx_reg_write(const struct isdnl1 *isdnl1, u8 offset, u8 value);
u8 isacsx_reg_read(const struct isdnl1 *isdnl1, u8 offset);
void isacsx_reg_mask(const struct isdnl1 *isdnl1, u8 offset, u8 set, u8 clr);
void isacsx_fill_fifo(struct isdnl1 *isdnl1);
void isacsx_release(struct isdnl1 *isdnl1);
int isacsx_irq_dispatch(struct isdnl1 *isdnl1);

bool isacsx_has_power_src1(const struct isdnl1 *isdnl1);
void isacsx_send_tx_frame(struct isdnl1 *isdnl1);
void isacsx_hwcmd(struct isdnl1 *isdnl1, unsigned int cmd);

int isacsx_pcm_clk_cb(struct notifier_block *nb, unsigned long ev,
				void *data);

/////////////////////////////////////////////////////////////////////////////
#endif				/* __ISDN_ISACSX_H__ */
