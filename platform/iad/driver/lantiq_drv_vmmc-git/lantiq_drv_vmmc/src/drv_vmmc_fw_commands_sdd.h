#ifndef _DRV_VMMC_FW_COMMANDS_SDD_H
#define _DRV_VMMC_FW_COMMANDS_SDD_H
/******************************************************************************

                              Copyright (c) 2013
                            Lantiq Deutschland GmbH
                             http://www.lantiq.com

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/**
   \file drv_vmmc_fw_commands_sdd.h
   This file contains the definitions of the SDD-FW command interface.
*/

/* ============================= */
/* Includes                      */
/* ============================= */
#include "drv_vmmc_fw_commands.h"


#ifdef __cplusplus
   extern "C" {
#endif

/* ============================= */
/* Global Defines                */
/* ============================= */

/* Command channel values */
#define VMMC_ALM_CMD_CHAN_A 0
#define VMMC_ALM_CMD_CHAN_B 1

/* ============================= */
/* Global Types                  */
/* ============================= */

/** @defgroup _VMMC_FW_SPEC_COMMANDS_ Command Messages
 *  @{
 */
/* ----- enums for special parameters ----- */

enum /* VMMC_SDD_OPMODE */
{
   VMMC_SDD_OPMODE_DISABLED           = 0x0,
   VMMC_SDD_OPMODE_RING_BURST         = 0x1,
   VMMC_SDD_OPMODE_STANDBY            = 0x2,
   VMMC_SDD_OPMODE_ACTIVE             = 0x3,
   VMMC_SDD_OPMODE_FXO                = 0x4,
   VMMC_SDD_OPMODE_CALIBRATE          = 0x6,
   VMMC_SDD_OPMODE_GR909              = 0x7
};

enum /* VMMC_SDD_OPMODE_POL */
{
   VMMC_OPMOD_POL_NORMAL              = 0x0,
   VMMC_OPMOD_POL_REVERSE             = 0x1
};

enum /* VMMC_SDD_OPMODE_HOWLER */
{
   VMMC_OPMOD_HOWLER_OFF              = 0x0,
   VMMC_OPMOD_HOWLER_ON               = 0x1
};

enum /* VMMC_SDD_OPMODE_TTX_BURST */
{
   VMMC_OPMOD_TTX_BURST_OFF           = 0x0,
   VMMC_OPMOD_TTX_BURST_ON            = 0x1
};

enum /* VMMC_SDD_OPMODE_MWI_LAMP */
{
   VMMC_OPMOD_MWI_LAMP_OFF            = 0x0,
   VMMC_OPMOD_MWI_LAMP_ON             = 0x1
};

/* ----- definition of FW SDD commands ----- */

typedef struct VMMC_SDD_REVISION_READ
{
   CMD_HEAD_BE;
   /** DC converter type which is indicated on the GPIO pins of the SmartSLIC.*/
   IFX_uint32_t DcDcType : 4;
   /** SmartSLIC HW Rev */
   IFX_uint32_t SmartSLIC_HW_Rev : 4;
   /** DCCtrl FW Version */
   IFX_uint32_t DCCtrlVers : 8;
   /** Reserved */
   IFX_uint32_t Res01 : 1;
   /** SmartSLIC Device ID */
   IFX_uint32_t SmartSLIC_DevID : 7;
   /** Channel Of Device */
   IFX_uint32_t SmartSLIC_CH : 3;
   /** Channel 1 Type */
   IFX_uint32_t SmartSLIC_FXO1 : 1;
   /** Channel 0 Type */
   IFX_uint32_t SmartSLIC_FXO0 : 1;
   /** SLIC Type of Device */
   IFX_uint32_t SmartSLIC_SLIC : 2;
   /** Reserved */
   IFX_uint32_t Res02 : 1;
   /** ASDSP Version */
   IFX_uint32_t ASDSPVers : 8;
   /** ASDSP Version Step */
   IFX_uint32_t ASDSPStep : 8;
   /** DCCtrl FW Hotfix Revision */
   IFX_uint32_t DCCtrlHotFixRevision : 8;
   /** DCCtrl FW Version Step */
   IFX_uint32_t DCCtrlVersStep : 8;
} __PACKED__ VMMC_SDD_REVISION_READ_t;

#define  SDD_REVISION_READ_ECMD  0
#define  SDD_REVISION_READ_LEN  8
#define  SDD_REVISION_READ_DCTYPE_COMBINED 1


/** SDD_Calibrate */
typedef struct VMMC_SDD_Calibrate
{
   CMD_HEAD_BE;
   /** TX path offset */
   IFX_uint32_t TxOffset : 16;
   /** Measurement equipment offset */
   IFX_uint32_t MeOffset : 16;
   /** RX DC path offset */
   IFX_uint32_t UlimOffset30 : 16;
   /** RX DC path offset */
   IFX_uint32_t UlimOffset60 : 16;
   /** IDAC gain correction */
   IFX_uint32_t IdacGain : 16;
   /** Reserved */
   IFX_uint32_t Res01 : 16;
   /** Reserved */
   IFX_uint32_t Res02 : 16;
   /** Combined DC/DC Voltage Correction */
   IFX_uint32_t Vdcdc : 16;
} __PACKED__ VMMC_SDD_Calibrate_t;

#define SDD_Calibrate_ECMD 3
#define SDD_Calibrate_LEN 16


/** SDD_BasicConfig */
typedef struct VMMC_SDD_BasicConfig
{
   CMD_HEAD_BE;
   /** DUP Time for Hook Debouncing in ONHOOK Mode */
   IFX_uint32_t OnhookDup : 4;
   /** DUP Time for Hook Debouncing in ACT Mode */
   IFX_uint32_t ActiveDup : 4;
   /** n.a. */
   IFX_uint32_t GsDup : 4;
   /** n.a. */
   IFX_uint32_t SoftDup : 4;
   /** n.a. */
   IFX_uint32_t GndkDup : 4;
   /** DUP Time for Excessive Temperature Detection Debouncing */
   IFX_uint32_t OvtDup : 4;
   /** n.a. */
   IFX_uint32_t GsEn : 1;
   /** Reserved */
   IFX_uint32_t Res01 : 1;
   /** Automatic Sense Bias Enable */
   IFX_uint32_t AutoBiasEn : 1;
   /** n.a. */
   IFX_uint32_t RpSoft : 1;
   /** Idle Extension Bit */
   IFX_uint32_t IdleExt : 1;
   /** Reserved */
   IFX_uint32_t Res02 : 3;
   /** Voltage Reduction for Standby Low Power */
   IFX_uint32_t LPVoltageOffset : 4;
   /** DC/DC Overhead Voltage */
   IFX_uint32_t DcDcOvh : 4;
   /** n.a. */
   IFX_uint32_t T2GDup : 4;
   /** n.a. */
   IFX_uint32_t GrFaultDisEn : 1;
   /** n.a. */
   IFX_uint32_t RaTimEn : 1;
   /** n.a. */
   IFX_uint32_t RiTimEn : 1;
   /** Low Power Bias */
   IFX_uint32_t LpBias : 1;
   /** n.a. */
   IFX_uint32_t TtxBurstLength : 16;
   /** n.a. */
   IFX_uint32_t OnhookThresh : 16;
   /** On-Hook Threshold in ACT Mode */
   IFX_uint32_t ActOnhookThresh : 16;
   /** Off-Hook Threshold in ACT Mode */
   IFX_uint32_t ActOffhookThresh : 16;
   /** n.a. */
   IFX_uint32_t GsThresh : 16;
   /** Open Loop Voltage Limit */
   IFX_uint32_t VoltageLimit : 16;
   /** Closed Loop Current Limit */
   IFX_uint32_t CurrentLimit : 16;
   /** n.a. */
   IFX_uint32_t GkLowThresh : 16;
   /** n.a. */
   IFX_uint32_t GkHighThresh : 16;
} __PACKED__ VMMC_SDD_BasicConfig_t;

#define SDD_BasicConfig_ECMD          4
#define SDD_BasicConfig_LEN           24
#define SDD_BasicConfig_ENABLE        1
#define SDD_BasicConfig_DISABLE       0


/** SDD_RingConfig */
typedef struct VMMC_SDD_RingConfig
{
   CMD_HEAD_BE;
   /** Ringing MODE */
   IFX_uint32_t RingMode : 2;
   /** Ring Trip Type */
   IFX_uint32_t RingTripType : 2;
   /** Ringing Signal Form */
   IFX_uint32_t RingSignal : 1;
   /** n.a. */
   IFX_uint32_t RingCrestFact : 3;
   /** n.a. */
   IFX_uint32_t RingDelay : 8;
   /**  Ringing Frequency */
   IFX_uint32_t RingFrequency : 16;
   /** Ringing Amplitude */
   IFX_uint32_t RingAmplitude : 16;
   /** Ringing Hook Threshold */
   IFX_uint32_t RingThresh : 16;
   /** Ringing DC Offset */
   IFX_uint32_t RingDcOffset : 16;
   /** Maximum Ring Current */
   IFX_uint32_t RingImax : 16;
   /** Ringing Regulation Coefficient */
   IFX_uint32_t RingCoeff : 16;
   /** Minimum Ringing Voltage (peak) */
   IFX_uint32_t RingVmin : 16;
   /** n.a. */
   IFX_uint32_t BurstLength : 8;
   /** n.a. */
   IFX_uint32_t PauseLength : 8;
   /** Fast Hook Threshold */
   IFX_uint32_t FastThresh : 16;
} __PACKED__ VMMC_SDD_RingConfig_t;

#define SDD_RingConfig_ECMD          5
#define SDD_RingConfig_LEN           20


/** SDD_ContMeasRead */
typedef struct VMMC_SDD_ContMeasRead
{
   CMD_HEAD_BE;
   /** Line Voltage on RING Wire */
   IFX_uint32_t VlineRing : 16;
   /** Line Voltage on TIP Wire */
   IFX_uint32_t VlineTip : 16;
   /** Desired Line Voltage */
   IFX_uint32_t Vlim : 16;
   /** Line Current */
   IFX_uint32_t Itrans : 16;
   /** Longitudinal Current */
   IFX_uint32_t Ilong : 16;
   /** Reserved */
   IFX_uint32_t SlicTemp : 16;
   /** RING Current */
   IFX_uint32_t Iring : 16;
   /** RING Voltage */
   IFX_uint32_t Vring : 16;
   /** n.a. */
   IFX_uint32_t TtxReal : 16;
   /** n.a. */
   IFX_uint32_t TtxImag : 16;
   /** n.a. */
   IFX_uint32_t TtxLen : 16;
   /** n.a. */
   IFX_uint32_t Ittx : 16;
   /** n.a. */
   IFX_uint32_t Vttx : 16;
   /** Battery Voltage */
   IFX_uint32_t Vbat : 16;
} __PACKED__ VMMC_SDD_ContMeasRead_t;

#define SDD_ContMeasRead_ECMD 2
#define SDD_ContMeasRead_LEN  28


/** SDD_ContMeasClear */
typedef struct VMMC_SDD_ContMeasClear
{
   CMD_HEAD_BE;
   /** Reserved */
   IFX_uint32_t Res01 : 15;
   /** Clear Continuous Measurement Data */
   IFX_uint32_t ClearCMD : 1;
   /** Reserved */
   IFX_uint32_t Res02 : 16;
} __PACKED__ VMMC_SDD_ContMeasClear_t;

#define SDD_ContMeasClear_ECMD 1
#define SDD_ContMeasClear_LEN  4


/** SDD_FxoHookSwitch */
typedef struct VMMC_SDD_FxoHookSwitch
{
   CMD_HEAD_BE;
   /** HOOK switch */
   IFX_uint32_t HF : 2;
   /** Reserved */
   IFX_uint32_t Res01 : 30;
} __PACKED__ VMMC_SDD_FxoHookSwitch_t;

#define SDD_FxoHookSwitch_ECMD 0x1B
#define SDD_FxoHookSwitch_LEN  4
#define SDD_FxoHookSwitch_HF_ON  0
#define SDD_FxoHookSwitch_HF_OFF  1
#define SDD_FxoHookSwitch_HF_FLASH  2


/** SDD_FxoConfig */
typedef struct VMMC_SDD_FxoConfig
{
   CMD_HEAD_BE;
   /** Reserved */
   IFX_uint32_t Res01 : 3;
   /** Timeout for transition to low power idle state */
   IFX_uint32_t TACT : 5;
   /** Debounce timer 2 */
   IFX_uint32_t DEB2 : 8;
   /** Maximum detection time for an open switching interval */
   IFX_uint32_t OSIT : 8;
   /** Flash time */
   IFX_uint32_t FLASHT : 8;
   /** Debounce timer 1 */
   IFX_uint32_t DEB1 : 8;
   /** Reserved */
   IFX_uint32_t Res02 : 24;
} __PACKED__ VMMC_SDD_FxoConfig_t;

#define SDD_FxoConfig_ECMD 0x1A
#define SDD_FxoConfig_LEN  8


/** SDD_DcDcConfig */
typedef struct VMMC_SDD_DcDcConfig
{
   CMD_HEAD_BE;
   /** Reserved */
   IFX_uint32_t Res01 : 8;
   /** DC/DC Hardware Option */
   IFX_uint32_t DcDcHw : 4;
   /** DC/DC Output Voltage Determined by Vcm DAC */
   IFX_uint32_t DcDcVconst : 1;
   /** DC/DC Switch Output Enable */
   IFX_uint32_t DcDcSwEn : 1;
   /** DC/DC Clock Every Second Cycle Low */
   IFX_uint32_t DcDcClkLow : 1;
   /** DC/DC Switch Output Inverted */
   IFX_uint32_t DcDcSwInv : 1;
   /** Reserved */
   IFX_uint32_t Res02 : 5;
   /** DC/DC Saw Tooth Generator Slope */
   IFX_uint32_t DcDcSaw : 3;
   /** DC/DC Frequency Counter */
   IFX_uint32_t DcDcFreq : 8;
   /** DC/DC Duty Cycle Counter for Idle Low Power */
   IFX_uint32_t DcDcDutyLp : 8;
   /** DC/DC Duty Cycle Counter */
   IFX_uint32_t DcDcDuty : 8;
   /** DC/DC Output Voltage (When DcDcVconst = 1) */
   IFX_uint32_t DcDcVoltage : 16;
} __PACKED__ VMMC_SDD_DcDcConfig_t;

#define SDD_DcDcConfig_ECMD 0x0C
#define SDD_DcDcConfig_LEN  8
#define SDD_DcDcConfig_DcDcHw_IBB12  0
#define SDD_DcDcConfig_DcDcHw_CIBB12  1


/** SDD_Opmode */
typedef struct VMMC_SDD_Opmode
{
   CMD_HEAD_BE;
   /** Polarity */
   IFX_uint32_t Polarity : 1;
   /** Howler Tone Configuration */
   IFX_uint32_t Howler : 1;
   /** Start a TTX Burst / TTX Burst Running */
   IFX_uint32_t TtxBurst : 1;
   /** n.a. */
   IFX_uint32_t SelParty : 1;
   /** Message Waiting Indication */
   IFX_uint32_t MwiLamp : 1;
   /** n.a. */
   IFX_uint32_t Res5kEn : 1;
   /** Set TIP wire to high impedance state */
   IFX_uint32_t HighImpTip : 1;
   /** Set RING wire to high impedance state */
   IFX_uint32_t HighImpRing : 1;
   /** Current Operating Mode (Read Only) */
   IFX_uint32_t CurOpMode : 4;
   /** Select Operating Mode */
   IFX_uint32_t OperatingMode : 4;
   /** Reserved */
   IFX_uint32_t Res01 : 16;
} __PACKED__ VMMC_SDD_Opmode_t;

#define SDD_Opmode_ECMD      7
#define SDD_Opmode_LEN       4
#define SDD_Opmode_TTX_BURST 1


/** SDD_GR909Config */
typedef struct VMMC_SDD_GR909Config
{
   CMD_HEAD_BE;
   /** Reserved */
   IFX_uint32_t Res01 : 6;
   /** Phone Detection Feature */
   IFX_uint32_t Pdf : 1;
   /** n.a. */
   IFX_uint32_t Cou : 1;
   /** Reserved */
   IFX_uint32_t Res02 : 3;
   /** Hazardous Potential Test */
   IFX_uint32_t Hpt : 1;
   /** Foreign ElectroMotive Forces Test */
   IFX_uint32_t Femf : 1;
   /** Resistive Faults Test */
   IFX_uint32_t Rft : 1;
   /** Receiver Off-Hook Test */
   IFX_uint32_t Roh : 1;
   /** Ringer Impedance Test */
   IFX_uint32_t Rit : 1;
   /** HPT Wire to GND AC Limit */
   IFX_uint32_t HptW2gAcLim : 16;
   /** HPT Wire to Wire AC Limit */
   IFX_uint32_t HptW2wAcLim : 16;
   /** HPT Wire to GND DC Limit */
   IFX_uint32_t HptW2gDcLim : 16;
   /** HPT Wire to Wire DC Limit */
   IFX_uint32_t HptW2wDcLim : 16;
   /** FEMF Wire to GND AC Limit */
   IFX_uint32_t FemfW2gAcLim : 16;
   /** FEMF Wire to Wire AC Limit */
   IFX_uint32_t FemfW2wAcLim : 16;
   /** FEMF Wire to GND DC Limit */
   IFX_uint32_t FemfW2gDcLim : 16;
   /** FEMF Wire to Wire DC Limit */
   IFX_uint32_t FemfW2wDcLim : 16;
   /** RFT Resistance Limit */
   IFX_uint32_t RftResLim : 16;
   /** ROH Linearity Limit */
   IFX_uint32_t RohLinLim : 16;
   /** RIT Lower Limit */
   IFX_uint32_t RitLowLim : 16;
   /** RIT Higher Limit */
   IFX_uint32_t RitHighLim : 16;
   /** Reserved */
   IFX_uint32_t Res03 : 16;
} __PACKED__ VMMC_SDD_GR909Config_t;

#define SDD_GR909Config_ECMD 8
#define SDD_GR909Config_LEN  28


/** SDD_GR909PhoneDetection */
typedef struct VMMC_SDD_GR909PhoneDetection
{
   CMD_HEAD_BE;
   /** Capacitance of the phone [nF] */
   IFX_uint32_t Capacitance : 32;
} __PACKED__ VMMC_SDD_GR909PhoneDetection_t;

#define SDD_GR909PhoneDetection_ECMD 0
#define SDD_GR909PhoneDetection_LEN  4


/** SDD_GR909ResultsRead */
typedef struct VMMC_SDD_GR909ResultsRead
{
   CMD_HEAD_BE;
   /** HPT Test Valid */
   IFX_uint32_t HptValid : 1;
   /** FEMF Test Valid */
   IFX_uint32_t FemfValid : 1;
   /** RFT Test Valid */
   IFX_uint32_t RftValid : 1;
   /** ROH Test Valid */
   IFX_uint32_t RohValid : 1;
   /** RIT Test Valid */
   IFX_uint32_t RitValid : 1;
   /** Reserved */
   IFX_uint32_t Res01 : 6;
   /** GR909 Busy */
   IFX_uint32_t Busy : 1;
   /** Reserved */
   IFX_uint32_t Res02 : 4;
   /** HPT Test Passed */
   IFX_uint32_t HptPass   : 1;
   /** FEMF Test Passed */
   IFX_uint32_t FemfPass : 1;
   /** RFT Test Passed */
   IFX_uint32_t RftPass : 1;
   /** ROH Test Passed */
   IFX_uint32_t RohPass : 1;
   /** RIT Test Passed */
   IFX_uint32_t RitPass : 1;
   /** Reserved */
   IFX_uint32_t Res03 : 11;
   /** Test Result HPT or FEMF AC Ring to GND */
   IFX_uint32_t HptAcR2g : 16;
   /** Test Result HPT or FEMF AC Tip to GND */
   IFX_uint32_t HptAcT2g : 16;
   /** Test Result HPT or FEMF AC Tip to Ring */
   IFX_uint32_t HptAcT2r : 16;
   /** Test Result HPT or FEMF DC Ring to GND */
   IFX_uint32_t HptDcR2g : 16;
   /** Test Result HPT or FEMF DC Tip to GND */
   IFX_uint32_t HptDcT2g : 16;
   /** Test Result HPT or FEMF DC Tip to Ring */
   IFX_uint32_t HptDcT2r : 16;
   /** Test Result RFT Ring to GND */
   IFX_uint32_t RftR2g : 16;
   /** Test Result RFT Tip to GND */
   IFX_uint32_t RftT2g : 16;
   /** Test Result RFT Tip to Ring */
   IFX_uint32_t RftT2r : 16;
   /** Test Result ROH Low */
   IFX_uint32_t RohLow : 16;
   /** Test Result ROH High */
   IFX_uint32_t RohHigh : 16;
   /** Test Result RIT */
   IFX_uint32_t RitRes : 16;
} __PACKED__ VMMC_SDD_GR909ResultsRead_t;

#define SDD_GR909ResultsRead_ECMD  0x9
#define SDD_GR909ResultsRead_LEN   0x1C


/** SDD_MwiConfig */
typedef struct VMMC_SDD_MwiConfig
{
   CMD_HEAD_BE;
   /** Message Waiting Voltage */
   IFX_uint32_t MwiVoltage : 16;
   /** Message Waiting Hook Threshold */
   IFX_uint32_t MwiThresh : 16;
   /** Message Waiting Slope */
   IFX_uint32_t MwiSlope : 16;
   /** Message Waiting On-time (Lamp ON) */
   IFX_uint32_t MwiOnTime : 8;
   /** Message Waiting Off-time (Lamp OFF) */
   IFX_uint32_t MwiOffTime : 8;
} __PACKED__ VMMC_SDD_MwiConfig_t;

#define SDD_MwiConfig_ECMD 6
#define SDD_MwiConfig_LEN  8
#define SDD_MwiVoltageDefault 0x5555
#define SDD_MwiThreshDefault  0x3333
#define SDD_MwiSlopeDefault   0x001B
#define SDD_MwiOnTimeDefault  0x14
#define SDD_MwiOffTimeDefault 0x28


/** SDD_DartDebug */
struct VMMC_SDD_DartDebug
{
   CMD_HEAD_BE;
   /** Write/Read Debug Data */
   IFX_uint32_t RWD : 1;
   /** Reserved */
   IFX_uint32_t Res01 : 27;
   /** Debug Bus Type */
   IFX_uint32_t DebugType : 4;
   /** Reserved */
   IFX_uint32_t Res02 : 32;
   /** Reserved */
   IFX_uint32_t Res03 : 32;
} __PACKED__;

#define SDD_DartDebug_ECMD 31
#define SDD_DartDebug_LEN  12
#define SDD_DartDebug_RWD_WRITE 0
#define SDD_DartDebug_RWD_READ 1
#define SDD_DartDebug_DebugType_SSI 2

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* _DRV_VMMC_FW_COMMANDS_SDD_H */
