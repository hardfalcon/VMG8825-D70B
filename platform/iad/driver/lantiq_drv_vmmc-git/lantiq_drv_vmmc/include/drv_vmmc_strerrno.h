#ifndef _DRV_VMMC_STRERRNO_H
#define _DRV_VMMC_STRERRNO_H
/******************************************************************************

                            Copyright (c) 2006-2015
                        Lantiq Beteiligungs-GmbH & Co.KG
                             http://www.lantiq.com

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/**
   \file drv_vmmc_strerrno.h  Contains the error codes and textual descriptions
   for use by the application which uses TAPI.
*/

/* ========================================================================== */
/*                             Macro definitions                              */
/* ========================================================================== */
#define VMMC_ERRNO_CNT 152

/* ========================================================================== */
/*                             Global variables                               */
/* ========================================================================== */
const IFX_uint32_t VMMC_drvErrnos[VMMC_ERRNO_CNT] =
{
   /* VMMC_statusOk */
   0,
   /* VMMC_statusNoChg */
   0x1,
   /* VMMC_statusNoData */
   0x5001,
   /* VMMC_statusNoUpdate */
   0x5002,
   /* VMMC_statusFwDwldFail */
   0x6010,
   /* VMMC_statusMipsOL */
   0x6011,
   /* VMMC_statusCmdErr */
   0x6012,
   /* VMMC_statusChErr */
   0x7000,
   /* VMMC_statusPte */
   0x7001,
   /* VMMC_statusCmdWr */
   0x7002,
   /* VMMC_statusWrongChMode */
   0x7003,
   /* VMMC_statusNoFreeInput */
   0x7004,
   /* VMMC_statusNoRes */
   0x7005,
   /* VMMC_statusSigNotAct */
   0x7006,
   /* VMMC_statusCodRun */
   0x7007,
   /* VMMC_statusT38Run */
   0x7008,
   /* VMMC_statusCmdRdTimeout */
   0x7009,
   /* VMMC_statusCmdWrTimeout */
   0x700a,
   /* VMMC_statusCmdAckTimeout */
   0x700b,
   /* VMMC_statusInvalCh */
   0x700c,
   /* VMMC_statusInvalLMSwitch */
   0x700d,
   /* VMMC_statusParam */
   0x700e,
   /* VMMC_statusNotSupported */
   0x700f,
   /* VMMC_statusAlmInit */
   0x7010,
   /* VMMC_statusBBDviolation */
   0x7011,
   /* VMMC_statusWrongEvpt */
   0x7012,
   /* VMMC_statusInvalSigState */
   0x7013,
   /* VMMC_statusSigModNotEn */
   0x7014,
   /* VMMC_statusCerr */
   0x7015,
   /* VMMC_statusNoToneRes */
   0x7016,
   /* VMMC_statusParamRange */
   0x7017,
   /* VMMC_statusSigUtgAlreadyActive */
   0x7018,
   /* VMMC_statusSigUtgNotActive */
   0x7019,
   /* VMMC_statusSigUtgBadRes */
   0x701a,
   /* VMMC_statusSigMftdNotActive */
   0x701b,
   /* VMMC_statusSigUtgBadCadence */
   0x701c,
   /* VMMC_statusSigCptdNoRes */
   0x701d,
   /* VMMC_statusSigCptdMaxRes */
   0x701e,
   /* VMMC_statusInvalAlmTypeSmpl */
   0x701f,
   /* VMMC_statusOverlay */
   0x7020,
   /* VMMC_statusFuncParm */
   0x7021,
   /* VMMC_statusCidAct */
   0x7022,
   /* VMMC_statusDtmfAct */
   0x7023,
   /* VMMC_statusCidStartSeqErr */
   0x7024,
   /* VMMC_statusCidTxStopErr */
   0x7025,
   /* VMMC_statusPcmTsInvalid */
   0x7026,
   /* VMMC_statusPcmHwInvalid */
   0x7027,
   /* VMMC_statusPcmNoRx */
   0x7028,
   /* VMMC_statusPcmNoTx */
   0x7029,
   /* VMMC_statusPcmDeact */
   0x702a,
   /* VMMC_statusPcmChNotEn */
   0x702b,
   /* VMMC_statusRtpPtOutOfRange */
   0x702c,
   /* VMMC_statusRtpEvtPtRedefinition */
   0x702d,
   /* VMMC_statusEsNotSupported */
   0x702e,
   /* VMMC_statusDectUtgFailedChNotActive */
   0x702f,
   /* VMMC_statusDectUtgBadRes */
   0x7030,
   /* VMMC_statusDectUtgBadTone */
   0x7031,
   /* VMMC_statusGR909LineNotDisabled */
   0x7032,
   /* VMMC_statusCodNotActiveOnJbRead */
   0x7033,
   /* VMMC_statusCodNotActiveOnJbReset */
   0x7034,
   /* VMMC_statusCodNotActiveOnRtcpRead */
   0x7035,
   /* VMMC_statusCodNotActiveOnRtcpReset */
   0x7036,
   /* VMMC_statusCodconfNotValid */
   0x7037,
   /* VMMC_statusCodModEn */
   0x7038,
   /* VMMC_statusCodTime */
   0x7039,
   /* VMMC_statusPcmSlaveCfg */
   0x703a,
   /* VMMC_statusModCon */
   0x703b,
   /* VMMC_statusCodInvalVad */
   0x703c,
   /* VMMC_statusCodAgc */
   0x703d,
   /* VMMC_statusCodNotActive */
   0x703e,
   /* VMMC_statusPcmIfCfgWhileActive */
   0x703f,
   /* VMMC_statusPcmIfCfgInvalidShift */
   0x7040,
   /* VMMC_statusPcmIfCfgInvalidDrvHalf */
   0x7041,
   /* VMMC_statusPcmChEn */
   0x7042,
   /* VMMC_statusPcmIfCfgGpioFailed */
   0x7043,
   /* VMMC_statusPcmChCoderNotAvail */
   0x7044,
   /* VMMC_statusCalInProgress */
   0x7045,
   /* VMMC_statusCalLineNotDisabled */
   0x7046,
   /* VMMC_statusT38Restart */
   0x7047,
   /* VMMC_statusT38NotMapped */
   0x7048,
   /* VMMC_statusT38NotActive */
   0x7049,
   /* VMMC_statusPcmInUse */
   0x704a,
   /* VMMC_statusHdlcFifoOverflow */
   0x704b,
   /* VMMC_statusMpsWriteFail */
   0x704c,
   /* VMMC_statusPcmIfStopWhileActive */
   0x704d,
   /* VMMC_statusInvalidIoctl */
   0x704e,
   /* VMMC_statusReserveChipAccessFailed */
   0x704f,
   /* VMMC_statusFXSCallOnFXO */
   0x7050,
   /* VMMC_statusLineNotFXO */
   0x7051,
   /* VMMC_statusFXOLineDisabled */
   0x7052,
   /* VMMC_statusCodAnnCodChanNotActive */
   0x7053,
   /* VMMC_statusCodAnnNotActive */
   0x7054,
   /* VMMC_statusLineTypChNotAll */
   0x7055,
   /* VMMC_statusDectEcStartFailedChNotActive */
   0x7056,
   /* VMMC_statusDectStopFailedEcActive */
   0x7057,
   /* VMMC_statusFailCapMeasActive */
   0x7058,
   /* VMMC_statusCapMeasStartWhileActive */
   0x7059,
   /* VMMC_statusDectInvCodLen */
   0x705a,
   /* VMMC_statusDectCodNotSupported */
   0x705b,
   /* VMMC_statusDectCodSetFailedUtg */
   0x705c,
   /* VMMC_statusDectCodPteSetFail */
   0x705d,
   /* VMMC_statusDectEncDelayOutOfRange */
   0x705e,
   /* VMMC_statusDectDecDelayOutOfRange */
   0x705f,
   /* VMMC_statusDectCfgWhileActive */
   0x7060,
   /* VMMC_statusDectInvalidFrameLength */
   0x7061,
   /* VMMC_statusDectInvalidCoder */
   0x7062,
   /* VMMC_statusDectCoderNotInFw */
   0x7063,
   /* VMMC_statusDectFrameLengthWhileActive */
   0x7064,
   /* VMMC_statusDectGainOutOfRange */
   0x7065,
   /* VMMC_statusAnnInUse */
   0x7066,
   /* VMMC_statusEncoderNotSupported */
   0x7067,
   /* VMMC_statusAnnNotConfigured */
   0x7068,
   /* VMMC_statusAnnActive */
   0x7069,
   /* VMMC_statusErrKernCpy */
   0x706a,
   /* VMMC_statusSigDtmfBadInputSelect */
   0x706b,
   /* VMMC_statusMwlNotStandby */
   0x706c,
   /* VMMC_statusBasicCfgRdErr */
   0x706d,
   /* VMMC_statusRingCfgRdErr */
   0x706e,
   /* VMMC_statusBasicCfgWrErr */
   0x706f,
   /* VMMC_statusRingCfgWrErr */
   0x7070,
   /* VMMC_statusOpModeWrErr */
   0x7071,
   /* VMMC_statusOpModeRdErr */
   0x7072,
   /* VMMC_statusCodActiveMosCfgFailed */
   0x7073,
   /* VMMC_statusCodMosGetFailedDecNotActive */
   0x7074,
   /* VMMC_statusInvalidRmes */
   0x7075,
   /* VMMC_statusSddEvtWaitTmout */
   0x7076,
   /* VMMC_statusSddEvtWaitInterrupt */
   0x7077,
   /* VMMC_statusReqMaxClockFailed */
   0x7078,
   /* VMMC_statusBlockedDcDcTypeMissmatch */
   0x7079,
   /* VMMC_statusCalNeighbourLineNotDisabled */
   0x707a,
   /* VMMC_statusNeighbourLineBlocksLMSwitch */
   0x707b,
   /* VMMC_statusNoMwlAndCombinedDcDc */
   0x707c,
   /* VMMC_statusBbdDcDcReconfig */
   0x707d,
   /* VMMC_statusBbdDcDcHwDiffers */
   0x707e,
   /* VMMC_statusGR909NeighbourLineNotDisabled */
   0x707f,
   /* VMMC_statusCodRedUnsupportedFrameLength */
   0x7080,
   /* VMMC_statusCodRedundancyInvalid */
   0x7081,
   /* VMMC_statusAutomaticCalibrationFailed */
   0x7082,
   /* VMMC_statusPcmResolutionNotSupported */
   0x7083,
   /* VMMC_statusDstChInvalid */
   0x7084,
   /* VMMC_statusDstModuleTypeNotSupported */
   0x7085,
   /* VMMC_statusDstModuleNotExisting */
   0x7086,
   /* VMMC_statusSrcNoDataCh */
   0x7087,
   /* VMMC_statusSrcWrongChMode */
   0x7088,
   /* VMMC_statusCidActive */
   0x7089,
   /* VMMC_statusSrcModuleTypeNotSupported */
   0x708a,
   /* VMMC_statusSrcModuleNotExisting */
   0x708b,
   /* VMMC_statusSigInputInUse */
   0x708c,
   /* VMMC_statusErr */
   0x8000,
   /* VMMC_statusDrvInitFail */
   0x8001,
   /* VMMC_statusDevInitFail */
   0x8002,
   /* VMMC_statusNoMem */
   0x8003
};

const IFX_char_t * const VMMC_drvErrStrings [VMMC_ERRNO_CNT] =
{
   "Success, no error occured",
   "Success, no change in the configuration found and no message send",
   "No data currently available",
   "No update of the requested information currently available",
   "Firmware download failed",
   "MIPS overload reported by the VoFW",
   "Command error occured",
   "Unknown error in channel",
   "Packet time encoding time mismatch. An atempt to set the packet time is not valid for the current coder setting.",
   "Writing a command failed",
   "The source channel is not is a data channel.",
   "No free input signal found or available",
   "The requested resource is not available.",
   "Signaling module is not activated. Internal error.",
   "Action not possible when coder is running",
   "Coder activation is not possible while the T.38 data pump is running.",
   "Timeout while waiting for read data",
   "Timeout while waiting for free mailbox space in VoFW",
   "Timeout while waiting for mailbox messages acknowledge",
   "Resource not valid. Channel number out of range",
   "Line mode switch is invalid. Not every transition is valid.",
   "At least one parameter is wrong",
   "Feature or combination not supported",
   "Analog line intialization failed, due command write error",
   "The BBD content for VoFW coefficients is invalid. Only resource coefficients are allowed",
   "Event payload type mismatch",
   "Invalid state for switching off signaling modules. Internal error",
   "Signaling module not enabled",
   "Command error occured before",
   "No tone resource available for this channel",
   "parameters are out of the supported range",
   "UTG already active",
   "UTG not active",
   "UTG bad resource number",
   "MFTD not active",
   "ToneAPI tone's cadence can not be zero",
   "No more CPTD resources avaliable",
   "Max number of CPTDs used",
   "Invalid line type",
   "At least one other resource is in use, which is overlayed with the CID sender",
   "Parameter is out of range",
   "A CID transmission is already active.",
   "A DTMF transmission is active.",
   "Initiating a CID sequence failed",
   "CID Tx could not be stopped.",
   "PCM timeslot given out of range",
   "Number of PCM Highway out of range",
   "Rx timeslot for PCM channel activation not available",
   "Tx timeslot for PCM channel activation not available",
   "PCM setting failed because PCM channel is disabled",
   "Configuration not possible. PCM channel not activated",
   "payloadtype out of range",
   "payloadtype redefinition",
   "Echo suppressor not supported.",
   "Cannot start DECT UTG when DECT channel is not active",
   "DECT UTG resource parameter invalid",
   "DECT UTG tone parameter invalid",
   "GR909 line is not in \"disabled\" state",
   "Coder Channel must be active to read JB statistics",
   "Coder Channel must be active to reset JB statistics",
   "Coder Channel must be active to read RTCP statistics",
   "Coder Channel must be active to reset RTCP statistics",
   "Requested encoder type not supported",
   "Coder module is not activated, internal driver error.",
   "Coder time, frame length not supported",
   "Invalid slave mode with MCTS",
   "VoFW intermodule connection error. Connection between two modules not done",
   "Invalid VAD parameter specified",
   "Enabling AGC on the coder is only allowed if the coder channel is activated",
   "Setting the frame length is only allowed when the coder is activated",
   "PCM interface cannot be configured while any PCM channel is active",
   "PCM interface configuration invalid, Shift Edge for Double clocking only",
   "PCM interface configuration invalid, Driving Mode for Single clocking only",
   "PCM channel cfg cannot be changed while channel is active",
   "PCM interface configuration failed to configure the GPIOs",
   "PCM channel coder not available",
   "Current line mode is CALIBRATE",
   "Current line mode is not DISABLED",
   "Action not possible when fax channel already running",
   "Action not possible when coder channel not mapped",
   "Action not possible when fax channel not active",
   "Resource not available. Channel already in use for another tasks",
   "Internal HDLC fifo overflow",
   "Failed to write data in to the MPS",
   "PCM interface cannot be stopped while any PCM channel is active",
   "Invalid IOCTL call",
   "Reserving the resources for chip access failed",
   "FXS service called on FXO channel",
   "FXO service called, line is not FXO",
   "FXO line disabled",
   "Coder channel not active",
   "Announcement playout not active",
   "Line type change not allowed",
   "Cannot start DECT Echo Canceller when DECT channel is not active",
   "Cannot stop DECT channel when DECT Echo Canceller is active",
   "Action not possible when capacitance measurement is active",
   "Capacitance measurement is already active",
   "Invalid DECT coder or length parameter values",
   "DECT coder not supported by firmware",
   "Cannot set DECT coder while UTG is running",
   "Cannot set DECT PTE while channel is running",
   "DECT encoder delay out of range",
   "DECT decoder delay out of range",
   "DECT codec delays cannot be set while channel is active",
   "DECT invalid frame length parameter",
   "DECT invalid coder parameter",
   "DECT selected coder not supported by firmware",
   "DECT frame length cannot be changed while the channel is active",
   "DECT gain out of supported range",
   "Announcement ID already in use",
   "Encoder not supported by the firmware",
   "Announcement not configured",
   "Announcement playout ongoing",
   "Copy KERN2USR or USR2KERN failed",
   "DTMF detector bad input direction selection",
   "MWL might be started only in linemode standby",
   "Reading the basic configuration failed",
   "Reading the ring configuration failed",
   "Writing the basic configuration failed",
   "Writing the ring configuration failed",
   "Writing the operation mode failed.",
   "Reading the operation mode failed.",
   "MOS configuration failed because coder channel is active",
   "MOS result get failed because decoder is not active",
   "Invalid Rmes value.",
   "Timeout waiting for an event from SDD.",
   "Waiting for an event from SDD interrupted by a signal.",
   "Failed to send sys clock change request to PMCU.",
   "Operation blocked until a BBD with DC/DC configuration matching the connected DC/DC hardware type is downloaded",
   "Neighbour line mode is not DISABLED",
   "Linefeed change not allowed while neighbour line is doing linetesting.",
   "MWL not possible on combined DC/DC hardware",
   "Ignoring already set SDD DC/DC config in BBD download.",
   "DC/DC converter type in the BBD download must match the DC/DC HW type.",
   "Neighbour line mode is not DISABLED",
   "Coder redundancy not supported for the selected packetization length.",
   "Coder redundancy value invalid.",
   "Automatic calibration after BBD download failed.",
   "Requested PCM resolution not supported.",
   "Destination channel number out of range",
   "Module type specified as destination is not supported",
   "The specified destination module does not exist",
   "The source channel does not contain a data channel",
   "The source channel is not initialised as data channel",
   "Disconnect not possible while CID sequence is active",
   "Module type specified as source is not supported",
   "The specified source module does not exist",
   "Input on SIG module already in use",
   "Generic or unknown error occured",
   "driver initialization failed",
   "basic device initialization failed",
   "no memory by memory allocation"
};

#endif /* _DRV_VMMC_STRERRNO_H */
