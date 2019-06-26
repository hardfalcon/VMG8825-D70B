#ifndef _DRV_ERRNO_H
#define _DRV_ERRNO_H
/******************************************************************************

                            Copyright (c) 2006-2015
                        Lantiq Beteiligungs-GmbH & Co.KG
                             http://www.lantiq.com

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

*******************************************************************************
   Module      : drv_vmmc_errno.h
******************************************************************************/

/** \file drv_vmmc_errno.h This file contains error number definitions
 and macros for setting the error code.
\note The macros must be used for reporting errors */

/* ============================= */
/* Global Defines                */
/* ============================= */


/** macro to signal and set an error. The error may also generate a
   trigger signal */
#ifndef SET_ERROR

#ifdef TESTING
/** Set non device or driver specific error.
\remarks The macro also reports the error to the high level TAPI
\param no error number */
#define SET_ERROR(no)                     \
   do {                                   \
      pCh->pParent->err = (IFX_int32_t) (no);      \
      pCh->pParent->nErrLine = __LINE__;           \
      strcpy (pCh->pParent->sErrFile, __FILE__);   \
      VMMC_ChErrorEvent (pCh, no, __LINE__, __FILE__,IFX_NULL, 0);  \
   } while(0)

/** Set device or driver specific error
\param no error number */
#define SET_DEV_ERROR(no)                 \
   do {                                   \
      pDev->err = (IFX_int32_t) (no);     \
      pDev->nErrLine = __LINE__;          \
      strcpy (pDev->sErrFile, __FILE__);  \
      VMMC_DevErrorEvent (pDev, no, __LINE__, __FILE__,IFX_NULL, 0);   \
   } while(0)
#else /* #ifdef TESTING */
   /* Customer specific definition.
      This solution has to be a provisional solution
      and has to be changed */
#define SET_ERROR(no)                     \
   do {                                   \
      pCh->pParent->err = (IFX_int32_t) (no);     \
      VMMC_ChErrorEvent (pCh, no, __LINE__, __FILE__,IFX_NULL, 0);        \
      /*lint -e(506, 774) */              \
      /* Parameter is deliberately false to cause an assertion */  \
      VMMC_ASSERT(IFX_FALSE);            \
   } while(0)

#define SET_DEV_ERROR(no)                 \
   do {                                   \
      pDev->err = (IFX_int32_t) (no);     \
      VMMC_DevErrorEvent (pDev, no, __LINE__, __FILE__,IFX_NULL, 0);      \
      /*lint -e(506, 774) */              \
      /* Parameter is deliberately false to cause an assertion */  \
      VMMC_ASSERT(IFX_FALSE);            \
   } while(0)
#endif /* #else #ifdef TESTING */

#endif /* SET_ERROR */

/** \defgroup ErrorCodes Driver and Chip Error Codes
 * @{
 */

/*Errorblock*/
/** Enumeration for function return status. The upper four bits are reserved for
    error clasification */
typedef enum
{
   /** Success, no error occured */
   VMMC_statusOk = TAPI_statusClassSuccess,
   /** Success, no change in the configuration found and no message send */
   VMMC_statusNoChg,

   /******************************************************** Device warnings */

   /******************************************************* Channel warnings */
   /** No data currently available */
   VMMC_statusNoData = TAPI_statusClassCh | TAPI_statusClassWarn | 0x1,
   /** No update of the requested information currently available */
   VMMC_statusNoUpdate,

   /********************************************************** Device errors */
   /** Firmware download failed */
   VMMC_statusFwDwldFail = TAPI_statusClassErr | 0x10,
   /** MIPS overload reported by the VoFW */
   VMMC_statusMipsOL,
   /** Command error occured */
   VMMC_statusCmdErr,

   /********************************************************* Channel errors */
   /** Unknown error in channel */
   VMMC_statusChErr = TAPI_statusClassErr | TAPI_statusClassCh,
   /** Packet time encoding time mismatch. An atempt to
   set the packet time is not valid for the current coder setting. */
   VMMC_statusPte,
   /** Writing a command failed */
   VMMC_statusCmdWr,
   /** The source channel is not is a data channel. */
   VMMC_statusWrongChMode,
   /** No free input signal found or available */
   VMMC_statusNoFreeInput,
   /** The requested resource is not available. */
   VMMC_statusNoRes,
   /** Signaling module is not activated. Internal error. */
   VMMC_statusSigNotAct,
   /** Action not possible when coder is running */
   VMMC_statusCodRun,
   /** Coder activation is not possible while the T.38 data pump is running. */
   VMMC_statusT38Run,
   /** Timeout while waiting for read data */
   VMMC_statusCmdRdTimeout,
   /** Timeout while waiting for free mailbox space in VoFW */
   VMMC_statusCmdWrTimeout,
   /** Timeout while waiting for mailbox messages acknowledge */
   VMMC_statusCmdAckTimeout,
   /** Resource not valid. Channel number out of range  */
   VMMC_statusInvalCh,
   /** Line mode switch is invalid. Not every transition is valid.  */
   VMMC_statusInvalLMSwitch,
   /** At least one parameter is wrong  */
   VMMC_statusParam,
   /** Feature or combination not supported   */
   VMMC_statusNotSupported,
   /** Analog line intialization failed, due command write error  */
   VMMC_statusAlmInit,

   /** The BBD content for VoFW coefficients is invalid.
       Only resource coefficients are allowed  */
   VMMC_statusBBDviolation,
   /** Event payload type mismatch */
   VMMC_statusWrongEvpt,
   /** Invalid state for switching off signaling modules. Internal error */
   VMMC_statusInvalSigState,
   /** Signaling module not enabled */
   VMMC_statusSigModNotEn,
   /** Command error occured before  */
   VMMC_statusCerr,
   /** No tone resource available for this channel  */
   VMMC_statusNoToneRes,
   /** parameters are out of the supported range  */
   VMMC_statusParamRange,
   /** UTG already active */
   VMMC_statusSigUtgAlreadyActive,
   /** UTG not active */
   VMMC_statusSigUtgNotActive,
   /** UTG bad resource number */
   VMMC_statusSigUtgBadRes,
   /** MFTD not active */
   VMMC_statusSigMftdNotActive,
   /** ToneAPI tone's cadence can not be zero */
   VMMC_statusSigUtgBadCadence,
   /** No more CPTD resources avaliable */
   VMMC_statusSigCptdNoRes,
   /** Max number of CPTDs used */
   VMMC_statusSigCptdMaxRes,
   /** Invalid line type  */
   VMMC_statusInvalAlmTypeSmpl,
   /** At least one other resource is in use, which is overlayed with the CID sender */
   VMMC_statusOverlay,
   /** Parameter is out of range */
   VMMC_statusFuncParm,
   /** A CID transmission is already active. */
   VMMC_statusCidAct,
   /** A DTMF transmission is active. */
   VMMC_statusDtmfAct,
   /** Initiating a CID sequence failed  */
   VMMC_statusCidStartSeqErr,
   /** CID Tx could not be stopped. */
   VMMC_statusCidTxStopErr,
   /** PCM timeslot given out of range  */
   VMMC_statusPcmTsInvalid,
   /** Number of PCM Highway out of range  */
   VMMC_statusPcmHwInvalid,
   /** Rx timeslot for PCM channel activation not available  */
   VMMC_statusPcmNoRx,
   /** Tx timeslot for PCM channel activation not available  */
   VMMC_statusPcmNoTx,
   /** PCM setting failed because PCM channel is disabled  */
   VMMC_statusPcmDeact,
   /** Configuration not possible. PCM channel not activated  */
   VMMC_statusPcmChNotEn,
   /** payloadtype out of range  */
   VMMC_statusRtpPtOutOfRange,
   /** payloadtype redefinition  */
   VMMC_statusRtpEvtPtRedefinition,
   /** Echo suppressor not supported.  */
   VMMC_statusEsNotSupported,
   /** Cannot start DECT UTG when DECT channel is not active  */
   VMMC_statusDectUtgFailedChNotActive,
   /** DECT UTG resource parameter invalid  */
   VMMC_statusDectUtgBadRes,
   /** DECT UTG tone parameter invalid  */
   VMMC_statusDectUtgBadTone,
   /** GR909 line is not in "disabled" state  */
   VMMC_statusGR909LineNotDisabled,
   /** Coder Channel must be active to read JB statistics  */
   VMMC_statusCodNotActiveOnJbRead,
   /** Coder Channel must be active to reset JB statistics  */
   VMMC_statusCodNotActiveOnJbReset,
   /** Coder Channel must be active to read RTCP statistics  */
   VMMC_statusCodNotActiveOnRtcpRead,
   /** Coder Channel must be active to reset RTCP statistics  */
   VMMC_statusCodNotActiveOnRtcpReset,
   /** Requested encoder type not supported */
   VMMC_statusCodconfNotValid,
   /** Coder module is not activated, internal driver error. */
   VMMC_statusCodModEn,
   /** Coder time, frame length not supported */
   VMMC_statusCodTime,
   /** Invalid slave mode with MCTS  */
   VMMC_statusPcmSlaveCfg,
   /** VoFW intermodule connection error. Connection between
       two modules not done  */
   VMMC_statusModCon,
   /** Invalid VAD parameter specified  */
   VMMC_statusCodInvalVad,
   /** Enabling AGC on the coder is only allowed if the coder
       channel is activated  */
   VMMC_statusCodAgc,
   /** Setting the frame length is only allowed when the coder is activated */
   VMMC_statusCodNotActive,
   /** PCM interface cannot be configured while any PCM channel
       is active */
   VMMC_statusPcmIfCfgWhileActive,
   /** PCM interface configuration invalid,
       Shift Edge for Double clocking only */
   VMMC_statusPcmIfCfgInvalidShift,
   /** PCM interface configuration invalid,
       Driving Mode for Single clocking only */
   VMMC_statusPcmIfCfgInvalidDrvHalf,
   /** PCM channel cfg cannot be changed while channel is active */
   VMMC_statusPcmChEn,
   /** PCM interface configuration failed to configure the GPIOs */
   VMMC_statusPcmIfCfgGpioFailed,
   /** PCM channel coder not available */
   VMMC_statusPcmChCoderNotAvail,
   /** Current line mode is CALIBRATE */
   VMMC_statusCalInProgress,
   /** Current line mode is not DISABLED */
   VMMC_statusCalLineNotDisabled,
   /** Action not possible when fax channel already running */
   VMMC_statusT38Restart,
   /** Action not possible when coder channel not mapped */
   VMMC_statusT38NotMapped,
   /** Action not possible when fax channel not active */
   VMMC_statusT38NotActive,
   /** Resource not available. Channel already in use for another tasks */
   VMMC_statusPcmInUse,
   /** Internal HDLC fifo overflow */
   VMMC_statusHdlcFifoOverflow,
   /** Failed to write data in to the MPS */
   VMMC_statusMpsWriteFail,
   /** PCM interface cannot be stopped while any PCM channel
       is active */
   VMMC_statusPcmIfStopWhileActive,
   /** Invalid IOCTL call */
   VMMC_statusInvalidIoctl,
   /** Reserving the resources for chip access failed */
   VMMC_statusReserveChipAccessFailed,
   /** FXS service called on FXO channel */
   VMMC_statusFXSCallOnFXO,
   /** FXO service called, line is not FXO */
   VMMC_statusLineNotFXO,
   /** FXO line disabled */
   VMMC_statusFXOLineDisabled,
   /** Coder channel not active */
   VMMC_statusCodAnnCodChanNotActive,
   /** Announcement playout not active */
   VMMC_statusCodAnnNotActive,
   /** Line type change not allowed */
   VMMC_statusLineTypChNotAll,
   /** Cannot start DECT Echo Canceller when DECT channel is not active  */
   VMMC_statusDectEcStartFailedChNotActive,
   /** Cannot stop DECT channel when DECT Echo Canceller is active  */
   VMMC_statusDectStopFailedEcActive,
   /** Action not possible when capacitance measurement is active  */
   VMMC_statusFailCapMeasActive,
   /** Capacitance measurement is already active  */
   VMMC_statusCapMeasStartWhileActive,
  /** Invalid DECT coder or length parameter values  */
   VMMC_statusDectInvCodLen,
   /** DECT coder not supported by firmware  */
   VMMC_statusDectCodNotSupported,
   /** Cannot set DECT coder while UTG is running  */
   VMMC_statusDectCodSetFailedUtg,
   /** Cannot set DECT PTE while channel is running  */
   VMMC_statusDectCodPteSetFail,
   /** DECT encoder delay out of range */
   VMMC_statusDectEncDelayOutOfRange,
   /** DECT decoder delay out of range */
   VMMC_statusDectDecDelayOutOfRange,
   /** DECT codec delays cannot be set while channel is active */
   VMMC_statusDectCfgWhileActive,
   /** DECT invalid frame length parameter */
   VMMC_statusDectInvalidFrameLength,
   /** DECT invalid coder parameter */
   VMMC_statusDectInvalidCoder,
   /** DECT selected coder not supported by firmware */
   VMMC_statusDectCoderNotInFw,
   /** DECT frame length cannot be changed while the channel is
       active */
   VMMC_statusDectFrameLengthWhileActive,
   /** DECT gain out of supported range */
   VMMC_statusDectGainOutOfRange,
   /** Announcement ID already in use */
   VMMC_statusAnnInUse,
   /** Encoder not supported by the firmware */
   VMMC_statusEncoderNotSupported,
   /** Announcement not configured */
   VMMC_statusAnnNotConfigured,
   /** Announcement playout ongoing */
   VMMC_statusAnnActive,
   /** Copy KERN2USR or USR2KERN failed */
   VMMC_statusErrKernCpy,
   /** DTMF detector bad input direction selection */
   VMMC_statusSigDtmfBadInputSelect,
   /** MWL might be started only in linemode standby */
   VMMC_statusMwlNotStandby,
   /** Reading the basic configuration failed */
   VMMC_statusBasicCfgRdErr,
   /** Reading the ring configuration failed */
   VMMC_statusRingCfgRdErr,
   /** Writing the basic configuration failed */
   VMMC_statusBasicCfgWrErr,
   /** Writing the ring configuration failed */
   VMMC_statusRingCfgWrErr,
   /** Writing the operation mode failed. */
   VMMC_statusOpModeWrErr,
   /** Reading the operation mode failed. */
   VMMC_statusOpModeRdErr,
   /** MOS configuration failed because coder channel is active */
   VMMC_statusCodActiveMosCfgFailed,
   /** MOS result get failed because decoder is not active */
   VMMC_statusCodMosGetFailedDecNotActive,
   /** Invalid Rmes value. */
   VMMC_statusInvalidRmes,
   /** Timeout waiting for an event from SDD. */
   VMMC_statusSddEvtWaitTmout,
   /** Waiting for an event from SDD interrupted by a signal. */
   VMMC_statusSddEvtWaitInterrupt,
   /** Failed to send sys clock change request to PMCU. */
   VMMC_statusReqMaxClockFailed,
   /** Operation blocked until a BBD with DC/DC configuration
       matching the connected DC/DC hardware type is downloaded */
   VMMC_statusBlockedDcDcTypeMissmatch,
   /** Neighbour line mode is not DISABLED */
   VMMC_statusCalNeighbourLineNotDisabled,
   /** Linefeed change not allowed while neighbour line is doing
       linetesting. */
   VMMC_statusNeighbourLineBlocksLMSwitch,
   /** MWL not possible on combined DC/DC hardware */
   VMMC_statusNoMwlAndCombinedDcDc,
   /** Ignoring already set SDD DC/DC config in BBD download. */
   VMMC_statusBbdDcDcReconfig,
   /** DC/DC converter type in the BBD download must match the
       DC/DC HW type. */
   VMMC_statusBbdDcDcHwDiffers,
   /** Neighbour line mode is not DISABLED */
   VMMC_statusGR909NeighbourLineNotDisabled,
   /** Coder redundancy not supported for the selected
       packetization length. */
   VMMC_statusCodRedUnsupportedFrameLength,
   /** Coder redundancy value invalid. */
   VMMC_statusCodRedundancyInvalid,
   /** Automatic calibration after BBD download failed. */
   VMMC_statusAutomaticCalibrationFailed,
   /** Requested PCM resolution not supported. */
   VMMC_statusPcmResolutionNotSupported,
   /** Destination channel number out of range */
   VMMC_statusDstChInvalid,
   /** Module type specified as destination is not supported */
   VMMC_statusDstModuleTypeNotSupported,
   /** The specified destination module does not exist */
   VMMC_statusDstModuleNotExisting,
   /** The source channel does not contain a data channel */
   VMMC_statusSrcNoDataCh,
   /** The source channel is not initialised as data channel */
   VMMC_statusSrcWrongChMode,
   /** Disconnect not possible while CID sequence is active */
   VMMC_statusCidActive,
   /** Module type specified as source is not supported */
   VMMC_statusSrcModuleTypeNotSupported,
   /** The specified source module does not exist */
   VMMC_statusSrcModuleNotExisting,
   /** Input on SIG module already in use */
   VMMC_statusSigInputInUse,
   /******************************************************** Critical errors */
   /** Generic or unknown error occured */
   VMMC_statusErr = TAPI_statusClassCritical,
   /** driver initialization failed */
   VMMC_statusDrvInitFail,
   /** basic device initialization failed */
   VMMC_statusDevInitFail,
   /** no memory by memory allocation */
   VMMC_statusNoMem,
   /** Basic communication socket initialization failed */
   VMMC_statusDevSockInit
}VMMC_status_t;

enum VMMC_DEV_ERR {
   /** 0x0: no error */
   VMMC_ERR_OK = 0,
   /** command error reported by vmmc, see last command */
   VMMC_ERR_CERR = 0x01,
   /** command inbox overflow reported by vmmc */
   VMMC_ERR_CIBX_OF = 0x2,
   /** host error reported by vmmc */
   VMMC_ERR_HOST = 0x3,
   /** MIPS overload */
   VMMC_ERR_MIPS_OL = 0x4,
   /** no command data received event within timeout.
       This error is obsolete, since the driver used a polling mode */
   VMMC_ERR_NO_COBX = 0x5,
   /** no command data received within timeout */
   VMMC_ERR_NO_DATA = 0x6,
   /** not enough inbox space for writing command */
   VMMC_ERR_NO_FIBXMS = 0x7,
   /** more data then expected in outbox */
   VMMC_ERR_MORE_DATA = 0x8,
   /** Mailbox was not empty after timeout. This error occurs while the
       driver tries to switch the mailbox sizes before and after the
       firmware download. The timeout is given in the constant WAIT_MBX_EMPTY */
   VMMC_ERR_NO_MBXEMPTY = 0x9,
   /** download ready event has not occured */
   VMMC_ERR_NO_DLRDY = 0xA,
   /** register read: expected values do not match */
   VMMC_ERR_WRONGDATA = 0xB,
   /** OBXML is zero after COBX-DATA event, wrong behaviour of VMMC
       After event OBXML must indicate data. This error is obsolete,
       since the driver used a polling mode */
   VMMC_ERR_OBXML_ZERO = 0xC,
   /** Test chip access failed */
   VMMC_ERR_TEST_FAIL = 0xD,
   /** Internal EDSP hardware error reported by
       VMMC in HWSR1:HW-ERR */
   VMMC_ERR_HW_ERR = 0xE,
   /** Mailbox Overflow Error */
   VMMC_ERR_PIBX_OF = 0xF,
   /** invalid parameter in function call */
   VMMC_ERR_FUNC_PARM = 0x10,
   /** timeout while waiting on channel status change */
   VMMC_ERR_TO_CHSTATE = 0x11,
   /** buffer underrun in evaluation downstreaming */
   VMMC_ERR_BUF_UN = 0x12,
   /** no memory by memory allocation */
   VMMC_ERR_NO_MEM = 0x13,
   /** board previously not initialized  */
   VMMC_ERR_NOINIT = 0x14,
   /** interrupts can not be cleared  */
   VMMC_ERR_INTSTUCK = 0x15,
   /** line testing measurement is running */
   VMMC_ERR_LT_ON = 0x16,
   /** PHI patch wasn't successfully downloaded. The problem was an chip
       access problem */
   VMMC_ERR_NOPHI = 0x17,
   /** EDSP Failures */
   VMMC_ERR_EDSP_FAIL = 0x18,
   /** CRC Fail while download */
   VMMC_ERR_FWCRC_FAIL = 0x19,
   /** TAPI not initialized */
   VMMC_ERR_NO_TAPI = 0x1A,
   /** Error while using SPI Inteface  */
   VMMC_ERR_SPI = 0x1B,
   /** inconsistent or invalid parameters were provided  */
   VMMC_ERR_INVALID = 0x1C,
   /** no Data to copy to user space for GR909 measurement */
   VMMC_ERR_GR909 = 0x1D,
   /** CRC Fail while ac download for V1.4 */
   VMMC_ERR_ACCRC_FAIL = 0x1E,
   /** couldn't read out chip version */
   VMMC_ERR_NO_VERSION = 0x1F,
   /** CRC Fail in DCCTRL download */
   VMMC_ERR_DCCRC_FAIL = 0x20,
   /** unknown chip version */
   VMMC_ERR_UNKNOWN_VERSION = 0x21,
   /** Linetesting, line is in Power Down High Impedance, measurement not possible */
   VMMC_ERR_LT_LINE_IS_PDNH = 0x22,
   /** Linetesting, unknown Parameter */
   VMMC_ERR_LT_UNKNOWN_PARAM = 0x23,
   /** Error while sending CID */
   VMMC_ERR_CID_TRANSMIT = 0x24,
   /** Linetesting, timeout waiting for LM_OK */
   VMMC_ERR_LT_TIMEOUT_LM_OK = 0x25,
   /** linetesting, timeout waiting for RAMP_RDY */
   VMMC_ERR_LT_TIMEOUT_LM_RAMP_RDY = 0x26,
   /** PRAM firmware not ok */
   VMMC_ERR_PRAM_FW = 0x27,
   /** no firmware specified and not included in driver  */
   VMMC_ERR_NOFW = 0x28,
   /** PHI CRC is zero */
   VMMC_ERR_PHICRC0 = 0x29,
   /** Embedded Controller download failed */
   VMMC_ERR_EMBDCTRL_DWLD_FAIL = 0x2A,
   /** Embedded Controller boot failed after download */
   VMMC_ERR_EMBDCTRL_DWLD_BOOT = 0x2B,
   /** Firmware binary is invalid  */
   VMMC_ERR_FWINVALID = 0x2C,
   /** Firmware version could not be read, no answer to command  */
   VMMC_ERR_NOFWVERS = 0x2D,
   /** Maximize mailbox failed  */
   VMMC_ERR_NOMAXCBX = 0x2E,
   /** Signaling channel not enabled */
   VMMC_ERR_SIGCH_NOTEN = 0x30,
   /** coder configuration not valid */
   VMMC_ERR_CODCONF_NOTVALID = 0x31,
   /** Linetesting, optimum result routine failed */
   VMMC_ERR_LT_OPTRES_FAILED = 0x32,
   /** No free input found while connecting cod, sig and alm modules */
   VMMC_ERR_NO_FREE_INPUT_SLOT = 0x33,
   /** feature or combination not supported  */
   VMMC_ERR_NOTSUPPORTED = 0x34,
   /** resource not available  */
   VMMC_ERR_NORESOURCE = 0x35,
   /** connection not valid on remove */
   VMMC_ERR_CON_INVALID = 0x37,
   /** host register access failure [2CPE] */
   VMMC_ERR_HOSTREG_ACCESS = 0x38,
   /** no packet buffers available */
   VMMC_ERR_NOPKT_BUFF = 0x39,
   /** At least one parameter is not possible to apply when the coder is
       running. Event payload types can not be changed on the fly. */
   VMMC_ERR_COD_RUNNING = 0x3A,
   /* Tone is already played out on this channel */
   VMMC_ERR_TONE_PLAYING = 0x3B,
   /** Tone resource is not capable playing out a certain tone. This
       error should not occur -> internal mismatch */
   VMMC_ERR_INVALID_TONERES = 0x3C,
   /** Invalid state for switching off signaling modules. Internal error */
   VMMC_ERR_INVALID_SIGSTATE = 0x3D,
   VMMC_ERR_INVALID_UTGSTATE = 0x3E,
   /** Cid sending is ongoing in this channel */
   VMMC_ERR_CID_RUNNING = 0x3F,
   /** Some internal state occured, that could not be handled. This error
       should never occur */
   VMMC_ERR_UNKNOWN = 0x40,
   /** Action not supported with this TAPI initialisation mode */
   VMMC_ERR_WRONG_CHANNEL_MODE = 0x41,
   /** No acutal signaling channel found. Could be an internal initialization
       problem or the resource is not available on this channel */
   VMMC_ERR_NO_SIGCH = 0x42,
   /** bufferpool buffer free error */
   VMMC_ERR_BUFPUT = 0x43,
   /** mailbox write error */
   VMMC_ERR_MBXWRITE = 0x44,
   /** Coder activation is not possible while the T.38 data pump is running. */
   VMMC_ERR_T38_RUNNING = 0x45,
   /* add here ^ */
   /*---------------------------- severe errors ------------------------------*/
   /** driver initialization failed */
   VMMC_ERR_DRVINIT_FAIL = 0x80,
   /** general access error, RDQ bit is always 1 */
   VMMC_ERR_DEV_ERR = 0x81
};

/*@}*/
#endif /* _DRV_ERRNO_H */
