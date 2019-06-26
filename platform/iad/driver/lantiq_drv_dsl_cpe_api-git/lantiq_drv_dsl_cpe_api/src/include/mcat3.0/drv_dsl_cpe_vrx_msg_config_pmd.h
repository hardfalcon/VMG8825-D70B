/******************************************************************************

                            Copyright (c) 2007-2017
                       Lantiq Beteiligungs-GmbH & Co. KG

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/


#ifndef _DRV_DSL_CPE_VRX_MSG_CONFIG_PMD_H_
#define _DRV_DSL_CPE_VRX_MSG_CONFIG_PMD_H_

/** \file

*/

#ifndef __PACKED__
   #if defined (__GNUC__) || defined (__GNUG__)
      /* GNU C or C++ compiler */
      #define __PACKED__ __attribute__ ((packed))
   #else
      /* Byte alignment adjustment */
      #pragma pack(1)
      #define __PACKED__      /* nothing */
   #endif
   #define __PACKED_DEFINED__ 1
#endif


/** @defgroup _CONFIG_PMD_
 *  @{
 */

#ifdef __cplusplus
   extern "C" {
#endif

/* ----- Message Specific Constants Definition section ----- */
#define ACK_ModemFSM_StateGet_RESET_STATE 0
#define ACK_ModemFSM_StateGet_READY_STATE 1
#define ACK_ModemFSM_StateGet_FAIL_STATE 2
#define ACK_ModemFSM_StateGet_DIAG_COMPLETE_STATE 3
#define ACK_ModemFSM_StateGet_GHS_STATE 5
#define ACK_ModemFSM_StateGet_FULLINIT_STATE 6
#define ACK_ModemFSM_StateGet_STEADY_STATE_TC_NOSYNC 7
#define ACK_ModemFSM_StateGet_STEADY_STATE_TC_SYNC 8
#define ACK_ModemFSM_StateGet_DIAGMODE_STATE 9
#define ACK_ModemFSM_StateGet_T1413_STATE 14
#define ACK_ModemFSM_StateGet_PRE_FAIL_STATE 15
#define ACK_ModemFSM_StateGet_GHS_BONDING_CLR_STATE 16
#define ACK_ModemFSM_StateGet_MFD_STATE 18
#define ACK_ModemFSM_StateGet_MFD_COMPLETE_STATE 19
#define ACK_ModemFSM_StateGet_TEST_STATE 240
#define ACK_ModemFSM_StateGet_L0 0
#define ACK_ModemFSM_StateGet_L2 2
#define ACK_ModemFSM_StateGet_L3 3
#define EVT_ModemFSM_StateGet_RESET_STATE 0
#define EVT_ModemFSM_StateGet_READY_STATE 1
#define EVT_ModemFSM_StateGet_FAIL_STATE 2
#define EVT_ModemFSM_StateGet_DIAG_COMPLETE_STATE 3
#define EVT_ModemFSM_StateGet_GHS_STATE 5
#define EVT_ModemFSM_StateGet_FULLINIT_STATE 6
#define EVT_ModemFSM_StateGet_STEADY_STATE_TC_NOSYNC 7
#define EVT_ModemFSM_StateGet_STEADY_STATE_TC_SYNC 8
#define EVT_ModemFSM_StateGet_DIAGMODE_STATE 9
#define EVT_ModemFSM_StateGet_T1413_STATE 14
#define EVT_ModemFSM_StateGet_PRE_FAIL_STATE 15
#define EVT_ModemFSM_StateGet_GHS_BONDING_CLR_STATE 16
#define EVT_ModemFSM_StateGet_MFD_STATE 18
#define EVT_ModemFSM_StateGet_MFD_COMPLETE_STATE 19
#define EVT_ModemFSM_StateGet_TEST_STATE 240
#define EVT_ModemFSM_StateGet_L0 0
#define EVT_ModemFSM_StateGet_L2 2
#define EVT_ModemFSM_StateGet_L3 3
#define EVT_ModemReady_MRERR_OK 0x0
#define EVT_ModemReady_MRWARN_OK 0x0
#define CMD_ModemFSM_StateSet_LINKRES 0
#define CMD_ModemFSM_StateSet_LINKINI 2
#define CMD_ModemFSM_StateSet_TERMINATE 4
#define CMD_ModemFSM_StateSet_TESTSTA 8
#define CMD_ModemFSM_StateSet_MFD 9
#define ALM_ModemFSM_FailReasonGet_S_OK 0x0
#define ALM_ModemFSM_FailReasonGet_S_CODESWAP 0x1
#define ALM_ModemFSM_FailReasonGet_S_MINRATE_DS 0x4
#define ALM_ModemFSM_FailReasonGet_S_INIT 0x5
#define ALM_ModemFSM_FailReasonGet_S_MODE 0x6
#define ALM_ModemFSM_FailReasonGet_S_TIMEOUT 0x9
#define ALM_ModemFSM_FailReasonGet_S_XDSL_MODE 0xA
#define ALM_ModemFSM_FailReasonGet_S_PORT_MODE 0xB
#define ALM_ModemFSM_FailReasonGet_S_T1413_MODE 0xC
#define ALM_ModemFSM_FailReasonGet_S_GFAST_MODE 0xD
#define ALM_ModemFSM_FailReasonGet_S_FW_RETRY 0xE
#define ALM_ModemFSM_FailReasonGet_S_FW_HYBRID 0xF
#define ALM_ModemFSM_FailReasonGet_S_LOS 0x10
#define ALM_ModemFSM_FailReasonGet_S_LOF 0x11
#define ALM_ModemFSM_FailReasonGet_S_LPR 0x12
#define ALM_ModemFSM_FailReasonGet_S_LOM 0x13
#define ALM_ModemFSM_FailReasonGet_S_FAST_LOS 0x14
#define ALM_ModemFSM_FailReasonGet_S_ESE 0x15
#define ALM_ModemFSM_FailReasonGet_S_SESX 0x16
#define ALM_ModemFSM_FailReasonGet_S_ESX 0x17
#define ALM_ModemFSM_FailReasonGet_S_OOS0 0x18
#define ALM_ModemFSM_FailReasonGet_S_LCD0 0x20
#define ALM_ModemFSM_FailReasonGet_S_NCD0 0x22
#define ALM_ModemFSM_FailReasonGet_S_CRC_DS 0x24
#define ALM_ModemFSM_FailReasonGet_S_PP_DRIVER 0x25
#define ALM_ModemFSM_FailReasonGet_S_INTENDED_LOCAL_SHUTDOWN 0x26
#define ALM_ModemFSM_FailReasonGet_S_HW 0x27
#define ALM_ModemFSM_FailReasonGet_S_CALIBRATION 0x28
#define ALM_ModemFSM_FailReasonGet_S_PP_CLOCK_NEW 0x29
#define ALM_ModemFSM_FailReasonGet_E_OK 0x0
#define ALM_ModemFSM_FailReasonGet_E_CONFIG 0x1
#define ALM_ModemFSM_FailReasonGet_E_NOTFEASIBLE 0x2
#define ALM_ModemFSM_FailReasonGet_E_COMM 0x3
#define ALM_ModemFSM_FailReasonGet_E_COMMCRC 0x4
#define ALM_ModemFSM_FailReasonGet_E_NOPEER 0x5
#define ALM_ModemFSM_FailReasonGet_E_CHIPSET 0x6
#define ALM_ModemFSM_FailReasonGet_E_UNKNOWN 0x7
#define ALM_ModemFSM_FailReasonGet_E_RESERVED 0x8
#define ALM_ModemFSM_FailReasonGet_E_SHOWTIME_NE 0x10
#define ALM_ModemFSM_FailReasonGet_E_SHOWTIME_FE 0x11
#define ACK_ModemFSM_FailReasonGet_S_OK 0x0
#define ACK_ModemFSM_FailReasonGet_S_CODESWAP 0x1
#define ACK_ModemFSM_FailReasonGet_S_MINRATE_DS 0x4
#define ACK_ModemFSM_FailReasonGet_S_INIT 0x5
#define ACK_ModemFSM_FailReasonGet_S_MODE 0x6
#define ACK_ModemFSM_FailReasonGet_S_TIMEOUT 0x9
#define ACK_ModemFSM_FailReasonGet_S_XDSL_MODE 0xA
#define ACK_ModemFSM_FailReasonGet_S_PORT_MODE 0xB
#define ACK_ModemFSM_FailReasonGet_S_T1413_MODE 0xC
#define ACK_ModemFSM_FailReasonGet_S_GFAST_MODE 0xD
#define ACK_ModemFSM_FailReasonGet_S_FW_RETRY 0xE
#define ACK_ModemFSM_FailReasonGet_S_FW_HYBRID 0xF
#define ACK_ModemFSM_FailReasonGet_S_LOS 0x10
#define ACK_ModemFSM_FailReasonGet_S_LOF 0x11
#define ACK_ModemFSM_FailReasonGet_S_LPR 0x12
#define ACK_ModemFSM_FailReasonGet_S_LOM 0x13
#define ACK_ModemFSM_FailReasonGet_S_FAST_LOS 0x14
#define ACK_ModemFSM_FailReasonGet_S_ESE 0x15
#define ACK_ModemFSM_FailReasonGet_S_SESX 0x16
#define ACK_ModemFSM_FailReasonGet_S_ESX 0x17
#define ACK_ModemFSM_FailReasonGet_S_OOS0 0x18
#define ACK_ModemFSM_FailReasonGet_S_LCD0 0x20
#define ACK_ModemFSM_FailReasonGet_S_NCD0 0x22
#define ACK_ModemFSM_FailReasonGet_S_CRC_DS 0x24
#define ACK_ModemFSM_FailReasonGet_S_PP_DRIVER 0x25
#define ACK_ModemFSM_FailReasonGet_S_INTENDED_LOCAL_SHUTDOWN 0x26
#define ACK_ModemFSM_FailReasonGet_S_HW 0x27
#define ACK_ModemFSM_FailReasonGet_S_CALIBRATION 0x28
#define ACK_ModemFSM_FailReasonGet_S_PP_CLOCK_NEW 0x29
#define ACK_ModemFSM_FailReasonGet_E_OK 0x0
#define ACK_ModemFSM_FailReasonGet_E_CONFIG 0x1
#define ACK_ModemFSM_FailReasonGet_E_NOTFEASIBLE 0x2
#define ACK_ModemFSM_FailReasonGet_E_COMM 0x3
#define ACK_ModemFSM_FailReasonGet_E_COMMCRC 0x4
#define ACK_ModemFSM_FailReasonGet_E_NOPEER 0x5
#define ACK_ModemFSM_FailReasonGet_E_CHIPSET 0x6
#define ACK_ModemFSM_FailReasonGet_E_UNKNOWN 0x7
#define ACK_ModemFSM_FailReasonGet_E_RESERVED 0x8
#define ACK_ModemFSM_FailReasonGet_E_SHOWTIME_NE 0x10
#define ACK_ModemFSM_FailReasonGet_E_SHOWTIME_FE 0x11
#define ACK_TxL3RequestStatusGet_L3PENDING 0
#define ACK_TxL3RequestStatusGet_L3REJECTED 1
#define ACK_TxL3RequestStatusGet_L3ACCEPTED 2
#define ACK_TxL3RequestStatusGet_L3FAIL 3
#define ACK_TxL3ReqFailReasonGet_BUSY 0x01
#define ACK_TxL3ReqFailReasonGet_INVALID 0x02
#define ACK_TxL3ReqFailReasonGet_STATE_NOT_DESIRED 0x03
#define ACK_TxL3ReqFailReasonGet_NOT_L0 0x50
#define ACK_TxL3ReqFailReasonGet_TIMEOUT 0x90
#define CMD_ModemFSM_Freeze_FREEZE_OFF 0
#define CMD_ModemFSM_Freeze_PERIODIC 1
#define CMD_ModemFSM_Freeze_RP_CHDISC2 2
#define CMD_ModemFSM_Freeze_MEDLEY 4
#define CMD_ModemFSM_Freeze_SHOWTIME 8
#define CMD_ModemFSM_Freeze_SHOWTIME_GI1 16
#define ACK_FW_ImageInfoGet_STANDALONE 0x0
#define ACK_FW_ImageInfoGet_COMBINED 0x1
#define ACK_FW_ImageInfoGet_VDSL_FW 0x0
#define ACK_FW_ImageInfoGet_ADSL_FW 0x1
#define ACK_FW_ImageInfoGet_CPE_FW 0x1
#define CMD_APS_Status_Set_1PORT 1
#define CMD_APS_Status_Set_2PORT 2
#define CMD_APS_Status_Set_APIDEF 0
#define CMD_APS_Status_Set_ADSL 1
#define CMD_APS_Status_Set_VDSL 2
#define CMD_APS_Status_Set_FAST 3
#define CMD_APS_Status_Set_AUTO 0
#define CMD_APS_Status_Set_STANDARD 1
#define CMD_APS_Status_Set_NONSTANDARD 2
#define CMD_APS_Status_Set_GHSMODE 1
#define CMD_APS_Status_Set_T1413MODE 2
#define CMD_BearerCh0_DS_Set_FV 21
#define CMD_BearerCh0_DS_Set_MAX_BER3 0
#define CMD_BearerCh0_DS_Set_MAX_BER5 1
#define CMD_BearerCh0_DS_Set_MAX_BER7 2
#define ACK_BearerCh0_DS_Set_FV 21
#define CMD_BearerCh0_US_Set_FV 21
#define CMD_BearerCh0_US_Set_MAX_BER3 0
#define CMD_BearerCh0_US_Set_MAX_BER5 1
#define CMD_BearerCh0_US_Set_MAX_BER7 2
#define ACK_BearerCh0_US_Set_FV 21
#define CMD_InitPolicySet_MAX_NETRATE 0x0
#define CMD_InitPolicySet_MAX_INP 0x1
#define CMD_ErasureControlSet_MAX_NETRATE 0x0
#define CMD_ErasureControlSet_MAX_INP 0x1
#define CMD_Misc_ConfigSet_GHSMODE 0x0
#define CMD_Misc_ConfigSet_T1413MODE 0x1
#define CMD_OperatorSelect_TELCO_OFF 0
#define CMD_OperatorSelect_TELCO_DTAG 1
#define CMD_OperatorSelect_TELCO_BT 2
#define CMD_OperatorSelect_TELCO_TS 3
#define CMD_OperatorSelect_TELCO_FT 4
#define CMD_OperatorSelect_TELCO_KPN 5
#define CMD_OperatorSelect_TELCO_TELIA 6
#define CMD_OperatorSelect_TELCO_FIBERHOME 7
#define CMD_OperatorSelect_TELCO_CT 8
#define CMD_OperatorSelect_TELCO_TW 9
#define CMD_OperatorSelect_TELCO_VODAFONE 10
#define CMD_OperatorSelect_TELCO_CN 11
#define CMD_OperatorSelect_TELCO_OTE 12
#define CMD_OperatorSelect_TELCO_TELMEX 13
#define CMD_OperatorSelect_TELCO_ATT 14
#define CMD_OperatorSelect_TELCO_SWISSCOM 15
#define CMD_OperatorSelect_TELCO_NETGEAR 16
#define ACK_OperatorSelectionGet_TELCO_OFF 0
#define ACK_OperatorSelectionGet_TELCO_DTAG 1
#define ACK_OperatorSelectionGet_TELCO_BT 2
#define ACK_OperatorSelectionGet_TELCO_TS 3
#define ACK_OperatorSelectionGet_TELCO_FT 4
#define ACK_OperatorSelectionGet_TELCO_KPN 5
#define ACK_OperatorSelectionGet_TELCO_TELIA 6
#define ACK_OperatorSelectionGet_TELCO_FIBERHOME 7
#define ACK_OperatorSelectionGet_TELCO_CT 8
#define ACK_OperatorSelectionGet_TELCO_TW 9
#define ACK_OperatorSelectionGet_TELCO_VODAFONE 10
#define ACK_OperatorSelectionGet_TELCO_CN 11
#define ACK_OperatorSelectionGet_TELCO_OTE 12
#define ACK_OperatorSelectionGet_TELCO_TELMEX 13
#define ACK_OperatorSelectionGet_TELCO_ATT 14
#define ACK_OperatorSelectionGet_TELCO_SWISSCOM 15
#define ACK_OperatorSelectionGet_TELCO_NETGEAR 16
/* ----- Message Specific Constants Definition section (End) ----- */

/** Message ID for CMD_ModemFSM_StateGet */
#define CMD_MODEMFSM_STATEGET 0x0002

/**
   Requests information about the current state of the modem state-machine.The
   command can be sent in all states of the modem state machine (see Figure 2).
*/
typedef struct CMD_ModemFSM_StateGet CMD_ModemFSM_StateGet_t;

/** Message ID for ACK_ModemFSM_StateGet */
#define ACK_MODEMFSM_STATEGET 0x0002

/**
   Returns information about the current state of the modem state-machine.
*/
typedef struct ACK_ModemFSM_StateGet ACK_ModemFSM_StateGet_t;

/** Message ID for EVT_ModemFSM_StateGet */
#define EVT_MODEMFSM_STATEGET 0x0002

/**
   Returns information about the current state of the modem state-machine. This
   message is sent autonomously, if the modem enters a new state and reporting
   is enabled for that state (see CMD_ModemFSM_EventConfigure).
*/
typedef struct EVT_ModemFSM_StateGet EVT_ModemFSM_StateGet_t;

/** Message ID for EVT_ModemReady */
#define EVT_MODEMREADY 0xFF02

/**
   Autonomous message indicating that the modem online code was reached and that
   initialization was completed. The management entity may not perform any
   query-response messaging until the EVT_ModemReady message is received. Its
   generation cannot be disabled. If it is not received this indicates a
   problem, possibly with the boot sequence. If the message is received, it
   contains a result code that may also indicate that an error was detected by
   the firmware during initialization.
*/
typedef struct EVT_ModemReady EVT_ModemReady_t;

/** Message ID for CMD_ModemFSM_StateSet */
#define CMD_MODEMFSM_STATESET 0x0041

/**
   Controls state transitions of the modem state-machine.
*/
typedef struct CMD_ModemFSM_StateSet CMD_ModemFSM_StateSet_t;

/** Message ID for ACK_ModemFSM_StateSet */
#define ACK_MODEMFSM_STATESET 0x0041

/**
   Acknowledgement for message CMD_ModemFSM_StateSet
*/
typedef struct ACK_ModemFSM_StateSet ACK_ModemFSM_StateSet_t;

/** Message ID for ALM_ModemFSM_FailReasonGet */
#define ALM_MODEMFSM_FAILREASONGET 0x0502

/**
   Reports failure information after entering FAIL state. This message is sent
   autonomously without host request after entrance into FAIL was indicated by
   EVT_ModemFSM_StateGet. Both messages are generated only if enabled with
   CMD_ModemFSM_EventConfigure (common Bit E8 "FAIL").
*/
typedef struct ALM_ModemFSM_FailReasonGet ALM_ModemFSM_FailReasonGet_t;

/** Message ID for CMD_ModemFSM_FailReasonGet */
#define CMD_MODEMFSM_FAILREASONGET 0x0502

/**
   Requests failure information after entering FAIL state.
*/
typedef struct CMD_ModemFSM_FailReasonGet CMD_ModemFSM_FailReasonGet_t;

/** Message ID for ACK_ModemFSM_FailReasonGet */
#define ACK_MODEMFSM_FAILREASONGET 0x0502

/**
   Returns failure information after entering FAIL state.
*/
typedef struct ACK_ModemFSM_FailReasonGet ACK_ModemFSM_FailReasonGet_t;

/** Message ID for CMD_ModemFSM_OptionsSet */
#define CMD_MODEMFSM_OPTIONSSET 0x0449

/**
   Configuration of options for the modem state machine.
*/
typedef struct CMD_ModemFSM_OptionsSet CMD_ModemFSM_OptionsSet_t;

/** Message ID for ACK_ModemFSM_OptionsSet */
#define ACK_MODEMFSM_OPTIONSSET 0x0449

/**
   Acknowledgement to CMD_ModemFSM_OptionsSet.
*/
typedef struct ACK_ModemFSM_OptionsSet ACK_ModemFSM_OptionsSet_t;

/** Message ID for CMD_ModemOptionsSet */
#define CMD_MODEMOPTIONSSET 0x0062

/**
   Configuration of options for the modem state machine.
*/
typedef struct CMD_ModemOptionsSet CMD_ModemOptionsSet_t;

/** Message ID for ACK_ModemOptionsSet */
#define ACK_MODEMOPTIONSSET 0x0062

/**
   Acknowledgement to CMD_ModemOptionsSet.
*/
typedef struct ACK_ModemOptionsSet ACK_ModemOptionsSet_t;

/** Message ID for CMD_ModemFSM_EventConfigure */
#define CMD_MODEMFSM_EVENTCONFIGURE 0x0049

/**
   Sets options for the modem state-machine. Enables/Disables autonomous
   messages for specific state transitions (EVT_ModemFSM_StateGet). The EVT
   message is generated after entering the particular state listed in the
   parameter description of the enable bits.
*/
typedef struct CMD_ModemFSM_EventConfigure CMD_ModemFSM_EventConfigure_t;

/** Message ID for ACK_ModemFSM_EventConfigure */
#define ACK_MODEMFSM_EVENTCONFIGURE 0x0049

/**
   Acknowledgement for CMD_ModemFSM_EventConfigure. (Enabling/Disabling EVENT
   messages (EVTs) for specific state transitions.)
*/
typedef struct ACK_ModemFSM_EventConfigure ACK_ModemFSM_EventConfigure_t;

/** Message ID for CMD_FailuresNE_AlarmConfigure */
#define CMD_FAILURESNE_ALARMCONFIGURE 0x0149

/**
   Enables/Disables the generation of ALARM messages (ALM) for specific near-end
   line failures. If the corresponding Enable bit for a line failure is set,
   then the modem firmware will send an autonomous message ALM_LineFailureNE_Get
   if this failure occurs.
*/
typedef struct CMD_FailuresNE_AlarmConfigure CMD_FailuresNE_AlarmConfigure_t;

/** Message ID for ACK_FailuresNE_AlarmConfigure */
#define ACK_FAILURESNE_ALARMCONFIGURE 0x0149

/**
   Acknowledgement to CMD_FailuresNE_AlarmConfigure.
*/
typedef struct ACK_FailuresNE_AlarmConfigure ACK_FailuresNE_AlarmConfigure_t;

/** Message ID for CMD_FailuresFE_AlarmConfigure */
#define CMD_FAILURESFE_ALARMCONFIGURE 0x0249

/**
   Enables/Disables the generation of ALARM messages (ALM) for specific far-end
   line failures. If the corresponding Enable bit for a line failure is set,
   then the modem firmware will send an autonomous message ALM_LineFailureFE_Get
   if this failure occurs.
*/
typedef struct CMD_FailuresFE_AlarmConfigure CMD_FailuresFE_AlarmConfigure_t;

/** Message ID for ACK_FailuresFE_AlarmConfigure */
#define ACK_FAILURESFE_ALARMCONFIGURE 0x0249

/**
   Acknowledgement to CMD_FailuresFE_AlarmConfigure.
*/
typedef struct ACK_FailuresFE_AlarmConfigure ACK_FailuresFE_AlarmConfigure_t;

/** Message ID for CMD_ReInitNE_Configure */
#define CMD_REINITNE_CONFIGURE 0x0549

/**
   Configures re-initialization triggers for near-end failure conditions in
   modem state STEADY STATE transmission (see Figure 1). This command is
   accepted in RESET state only.
*/
typedef struct CMD_ReInitNE_Configure CMD_ReInitNE_Configure_t;

/** Message ID for ACK_ReInitNE_Configure */
#define ACK_REINITNE_CONFIGURE 0x0549

/**
   Acknowledgment to CMD_ReInitNE_Configure (Configuration of re-initialization
   triggers for near-end failures).
*/
typedef struct ACK_ReInitNE_Configure ACK_ReInitNE_Configure_t;

/** Message ID for CMD_ReinitThreshConfigure */
#define CMD_REINITTHRESHCONFIGURE 0x1E62

/**
   Configuration of reinitialization trigger definitions.
*/
typedef struct CMD_ReinitThreshConfigure CMD_ReinitThreshConfigure_t;

/** Message ID for ACK_ReinitThreshConfigure */
#define ACK_REINITTHRESHCONFIGURE 0x1E62

/**
   Acknowledgement for the message ACK_ReinitThreshConfigure.
*/
typedef struct ACK_ReinitThreshConfigure ACK_ReinitThreshConfigure_t;

/** Message ID for CMD_ShutdownRequest */
#define CMD_SHUTDOWNREQUEST 0x0341

/**
   Triggers a shutdown request, either as "L3 orderly shutdown" towards the
   remote side or as "locally forced shutdown" to the DSL-FW without sending a
   request to the remote side.After an "L3 orderly shutdown" request was
   accepted by the CO, the following shall happen: The CPE-Host forces L3 entry
   with transition to RESET state by applying CMD_ModemFSM_StateGet.A "locally
   forced shutdown" always results in exiting Showtime.
*/
typedef struct CMD_ShutdownRequest CMD_ShutdownRequest_t;

/** Message ID for ACK_ShutdownRequest */
#define ACK_SHUTDOWNREQUEST 0x0341

/**
   Acknowledgement for CMD_ShutdownRequest.
*/
typedef struct ACK_ShutdownRequest ACK_ShutdownRequest_t;

/** Message ID for CMD_RxL3RequestStatusGet */
#define CMD_RXL3REQUESTSTATUSGET 0x0402

/**
   Requests information about L3 orderly shutdown requests initiated from the
   remote side.Applying CMD_RxL3RequestStatusGet allows the host to determine
   whether a near-end LOS or LOM failure is due to an L3 shutdown or an
   unexpected line failure.
*/
typedef struct CMD_RxL3RequestStatusGet CMD_RxL3RequestStatusGet_t;

/** Message ID for ACK_RxL3RequestStatusGet */
#define ACK_RXL3REQUESTSTATUSGET 0x0402

/**
   Acknowledgement for CMD_RxL3RequestStatusGet.As long as the modem is in L0
   (Showtime), an ATU-R will automatically respond to any L3 request from the
   ATU-C by sending an accept message.  If the ATU-R is in L2 mode when it
   receives the request, it will send a reject message. Upon receiving an
   acceptance, the remote side should enter L3 state and shut off its
   transmitter.
*/
typedef struct ACK_RxL3RequestStatusGet ACK_RxL3RequestStatusGet_t;

/** Message ID for CMD_TxL3RequestStatusGet */
#define CMD_TXL3REQUESTSTATUSGET 0x1402

/**
   Requests the status of a near-end initiated L3 shutdown request. If the
   remote side accepted the request, in ADSL the host will force L3 entry using
   CMD_ModemFSM_StateSet.
*/
typedef struct CMD_TxL3RequestStatusGet CMD_TxL3RequestStatusGet_t;

/** Message ID for ACK_TxL3RequestStatusGet */
#define ACK_TXL3REQUESTSTATUSGET 0x1402

/**
   Acknowledgement for CMD_TxL3RequestStatusGet.
*/
typedef struct ACK_TxL3RequestStatusGet ACK_TxL3RequestStatusGet_t;

/** Message ID for CMD_TxL3ReqFailReasonGet */
#define CMD_TXL3REQFAILREASONGET 0x1502

/**
   Requests the fail reason of a failed near-end initiated L3 request. (see also
   ACK_TxL3RequestStatusGet)
*/
typedef struct CMD_TxL3ReqFailReasonGet CMD_TxL3ReqFailReasonGet_t;

/** Message ID for ACK_TxL3ReqFailReasonGet */
#define ACK_TXL3REQFAILREASONGET 0x1502

/**
   Reports the fail reason of a near-end initiated L3 request, as response to
   CMD_TxL3ReqFailReasonGet.
*/
typedef struct ACK_TxL3ReqFailReasonGet ACK_TxL3ReqFailReasonGet_t;

/** Message ID for CMD_ModemFSM_Freeze */
#define CMD_MODEMFSM_FREEZE 0x2C44

/**
   The message selects and enables one of several points during the
   initialization process where -after a normal link start had been triggered-
   the modem FSM "freezes". Freezing means the CO can be disconnected from the
   line and still the CPE continues transmitting the signal of the current
   phase. To exit from the freeze a state transition to RESET state has to be
   triggered. In case of a Showtime Freeze the reinit-triggers do not have any
   effect.
*/
typedef struct CMD_ModemFSM_Freeze CMD_ModemFSM_Freeze_t;

/** Message ID for ACK_ModemFSM_Freeze */
#define ACK_MODEMFSM_FREEZE 0x2C44

/**
   Acknowledgement for message CMD_ModemFSM_Freeze.
*/
typedef struct ACK_ModemFSM_Freeze ACK_ModemFSM_Freeze_t;

/** Message ID for CMD_FW_ImageInfoGet */
#define CMD_FW_IMAGEINFOGET 0xD103

/**
   Requests information about the loaded FW image. This info is used e.g. for
   swapping between ADSL and VDSL.
*/
typedef struct CMD_FW_ImageInfoGet CMD_FW_ImageInfoGet_t;

/** Message ID for ACK_FW_ImageInfoGet */
#define ACK_FW_IMAGEINFOGET 0xD103

/**
   Provides the information about the FW image requested by CMD_FW_ImageInfoGet
*/
typedef struct ACK_FW_ImageInfoGet ACK_FW_ImageInfoGet_t;

/** Message ID for CMD_APS_Status_Set */
#define CMD_APS_STATUS_SET 0x2062

/**
   The message informs the FW about the status of SW parameters relevant for the
   multimode state machine (APS). This can be used to trace the APS behaviour
   with FW tools. The message does not influence the APS behaviour.
*/
typedef struct CMD_APS_Status_Set CMD_APS_Status_Set_t;

/** Message ID for ACK_APS_Status_Set */
#define ACK_APS_STATUS_SET 0x2062

/**
   Acknowledgement for message CMD_APS_Status_Set.
*/
typedef struct ACK_APS_Status_Set ACK_APS_Status_Set_t;

/** Message ID for CMD_XTSE_Configure */
#define CMD_XTSE_CONFIGURE 0x0045

/**
   Configuration of the VTU Transmission System Enabling (XTSE).Configures the
   transmission system coding types to be supported on the line (VDSL flavour
   and annex support).(References: Section 7.3.1.1.1 of G.997.1 and G.994.1 Amd4
   [10] Tables "Standard information field - SPar(1) coding")
*/
typedef struct CMD_XTSE_Configure CMD_XTSE_Configure_t;

/** Message ID for ACK_XTSE_Configure */
#define ACK_XTSE_CONFIGURE 0x0045

/**
   Acknowledgement for message CMD_XTSE_Configure.
*/
typedef struct ACK_XTSE_Configure ACK_XTSE_Configure_t;

/** Message ID for CMD_BandControl_US_Set */
#define CMD_BANDCONTROL_US_SET 0x2548

/**
   Controls the upstream band usage.The message can only be used for DMTscope
   tests without handshake and training phases.
*/
typedef struct CMD_BandControl_US_Set CMD_BandControl_US_Set_t;

/** Message ID for ACK_BandControl_US_Set */
#define ACK_BANDCONTROL_US_SET 0x2548

/**
   Acknowledgement for message CMD_BandControl_US_Set.
*/
typedef struct ACK_BandControl_US_Set ACK_BandControl_US_Set_t;

/** Message ID for CMD_PSD_Set */
#define CMD_PSD_SET 0x2348

/**
   Configuration of PSD and Power parameters. Only used for DMTscope tests
   without handshake and training phases.
*/
typedef struct CMD_PSD_Set CMD_PSD_Set_t;

/** Message ID for ACK_PSD_Set */
#define ACK_PSD_SET 0x2348

/**
   Acknowledgement for the message CMD_PSD_Set.
*/
typedef struct ACK_PSD_Set ACK_PSD_Set_t;

/** Message ID for CMD_PSD_BreakpointsTxUS_Set */
#define CMD_PSD_BREAKPOINTSTXUS_SET 0x2848

/**
   Specifies the maximum upstream transmit PSD by means of breakpoints.To be
   used only for DMTscope tests without handshake and training phases.
*/
typedef struct CMD_PSD_BreakpointsTxUS_Set CMD_PSD_BreakpointsTxUS_Set_t;

/** Message ID for ACK_PSD_BreakpointsTxUS_Set */
#define ACK_PSD_BREAKPOINTSTXUS_SET 0x2848

/**
   Acknowledgement for the message CMD_PSD_BreakpointsTxUS_Set.
*/
typedef struct ACK_PSD_BreakpointsTxUS_Set ACK_PSD_BreakpointsTxUS_Set_t;

/** Message ID for  CMD_PSD_Calibration_DS_Set */
#define  CMD_PSD_CALIBRATION_DS_SET 0x5748

/**
   Sends downstream calibration information for "per-tone" test parameters. It
   is effective for Loop Diagnostic Mode (DELT) and Showtime. It is the
   responsibility of the manufacturer to provide the data.
*/
typedef struct  CMD_PSD_Calibration_DS_Set CMD_PSD_Calibration_DS_Set_t;

/** Message ID for ACK_PSD_Calibration_DS_Set */
#define ACK_PSD_CALIBRATION_DS_SET 0x5748

/**
   Acknowledgement for the message CMD_PSD_Calibration_DS_Set.
*/
typedef struct ACK_PSD_Calibration_DS_Set ACK_PSD_Calibration_DS_Set_t;

/** Message ID for  CMD_PSD_Calibration_US_Set */
#define  CMD_PSD_CALIBRATION_US_SET 0x5848

/**
   Sends upstream calibration information for the upstream transmit PSD. It is
   effective for loop diagnostic mode (DELT) and Showtime. It is the
   responsibility of the manufacturer to provide the data.
*/
typedef struct  CMD_PSD_Calibration_US_Set CMD_PSD_Calibration_US_Set_t;

/** Message ID for ACK_PSD_Calibration_US_Set */
#define ACK_PSD_CALIBRATION_US_SET 0x5848

/**
   Acknowledgement for the message CMD_PSD_Calibration_US_Set.
*/
typedef struct ACK_PSD_Calibration_US_Set ACK_PSD_Calibration_US_Set_t;

/** Message ID for CMD_PSD_OptionsSet */
#define CMD_PSD_OPTIONSSET 0x0262

/**
   Configuration of PSD related options.
*/
typedef struct CMD_PSD_OptionsSet CMD_PSD_OptionsSet_t;

/** Message ID for ACK_PSD_OptionsSet */
#define ACK_PSD_OPTIONSSET 0x0262

/**
   Acknowledgement to CMD_PSD_OptionsSet.
*/
typedef struct ACK_PSD_OptionsSet ACK_PSD_OptionsSet_t;

/** Message ID for CMD_UPBO_KL0Get */
#define CMD_UPBO_KL0GET 0xD603

/**
   Requests the electrical loop length estimate kl0.
*/
typedef struct CMD_UPBO_KL0Get CMD_UPBO_KL0Get_t;

/** Message ID for ACK_UPBO_KL0Get */
#define ACK_UPBO_KL0GET 0xD603

/**
   Delivers the data requested by CMD_UPBO_KL0Get.(Section 7.5.1.23 of G.997.1
   [11])
*/
typedef struct ACK_UPBO_KL0Get ACK_UPBO_KL0Get_t;

/** Message ID for CMD_PBO_AELEM_Status_Get */
#define CMD_PBO_AELEM_STATUS_GET 0xEA03

/**
   Requests status parameters for the Upstream  Power  Back-Off  (UPBO)
   alternative  electrical  length  estimation  method (AELEM).
*/
typedef struct CMD_PBO_AELEM_Status_Get CMD_PBO_AELEM_Status_Get_t;

/** Message ID for ACK_PBO_AELEM_Status_Get */
#define ACK_PBO_AELEM_STATUS_GET 0xEA03

/**
   Upstream  Power  Back-Off  (UPBO)  status  parameters  for  the  alternative
   electrical  length  estimation  method (AELEM) are reported.
*/
typedef struct ACK_PBO_AELEM_Status_Get ACK_PBO_AELEM_Status_Get_t;

/** Message ID for CMD_NoiseMarginDeltaSet */
#define CMD_NOISEMARGINDELTASET 0x1C45

/**
   Configuration of a target noise margin delta, which is added to the target
   noise margin value configured at the CO (and in case of the CPE received from
   there). The resulting value is then taken as target noise margin, e.g. for
   bit loading.
*/
typedef struct CMD_NoiseMarginDeltaSet CMD_NoiseMarginDeltaSet_t;

/** Message ID for ACK_NoiseMarginDeltaSet */
#define ACK_NOISEMARGINDELTASET 0x1C45

/**
   Acknowledgement for the message CMD_NoiseMarginDeltaSet.
*/
typedef struct ACK_NoiseMarginDeltaSet ACK_NoiseMarginDeltaSet_t;

/** Message ID for CMD_BearerCh0_DS_Set */
#define CMD_BEARERCH0_DS_SET 0x0048

/**
   Sets parameters for downstream bearer channel 0 (Chapters 7.3.2.1-5 of
   G.997.1 [11]). Bearer channel configuration at the CPE is optional and just
   used to further limit the parameters usually configured at the CO.The mode,
   ATM or PTM, is selected with the configuration controls "ATMControl" and
   "PTMControl" in Parameter 2. Exactly one of them must be enabled, not both!
*/
typedef struct CMD_BearerCh0_DS_Set CMD_BearerCh0_DS_Set_t;

/** Message ID for ACK_BearerCh0_DS_Set */
#define ACK_BEARERCH0_DS_SET 0x0048

/**
   Acknowledgement for CMD_BearerCh0_DS_Set. (Configuration of bearer channel
   0).
*/
typedef struct ACK_BearerCh0_DS_Set ACK_BearerCh0_DS_Set_t;

/** Message ID for CMD_BearerCh0_US_Set */
#define CMD_BEARERCH0_US_SET 0x0248

/**
   Sets parameters for upstream bearer channel 0 (Chapters 7.3.2.1-5 of G.997.1
   [11]). Bearer channel configuration at the CPE is optional and just used to
   further limit the parameters usually configured at the CO.The mode, ATM or
   PTM, is selected with the configuration controls "ATMControl" and
   "PTMControl" in Parameter 2. Exactly one of them must be enabled, not both!
*/
typedef struct CMD_BearerCh0_US_Set CMD_BearerCh0_US_Set_t;

/** Message ID for ACK_BearerCh0_US_Set */
#define ACK_BEARERCH0_US_SET 0x0248

/**
   Acknowledgement for CMD_BearerCh0_US_Set. (Configuration of bearer channel
   0).
*/
typedef struct ACK_BearerCh0_US_Set ACK_BearerCh0_US_Set_t;

/** Message ID for CMD_InitPolicySet */
#define CMD_INITPOLICYSET 0x1A45

/**
   The message selects the channel initialization policy (CIPOLICY) to be
   applied for the tranceiver configuration. (Section 7.3.2.10 of G.997.1)In
   ADSL mode, the message is applicable only for ADSL2/2+ (Section 7.10.3 of
   G.992.3 Amd 3). The standard defines the parameter for the CO only. Here, the
   message is applied at the ADSL-CPE, then it overrides the policy given by the
   CO!
*/
typedef struct CMD_InitPolicySet CMD_InitPolicySet_t;

/** Message ID for ACK_InitPolicySet */
#define ACK_INITPOLICYSET 0x1A45

/**
   Acknowledgment for message CMD_InitPolicySet.
*/
typedef struct ACK_InitPolicySet ACK_InitPolicySet_t;

/** Message ID for CMD_ErasureControlSet */
#define CMD_ERASURECONTROLSET 0x0162

/**
   Configuration of options for the modem state machine.
*/
typedef struct CMD_ErasureControlSet CMD_ErasureControlSet_t;

/** Message ID for ACK_ErasureControlSet */
#define ACK_ERASURECONTROLSET 0x0162

/**
   Acknowledgement to CMD_ErasureControlSet.
*/
typedef struct ACK_ErasureControlSet ACK_ErasureControlSet_t;

/** Message ID for CMD_Misc_ConfigSet */
#define CMD_MISC_CONFIGSET 0x3A48

/**
   Performs some miscellaneous chip set configurations.
*/
typedef struct CMD_Misc_ConfigSet CMD_Misc_ConfigSet_t;

/** Message ID for ACK_Misc_ConfigSet */
#define ACK_MISC_CONFIGSET 0x3A48

/**
   This is the acknowledgement for ACK_Misc_ConfigSet.
*/
typedef struct ACK_Misc_ConfigSet ACK_Misc_ConfigSet_t;

/** Message ID for CMD_OperationOptionsSet */
#define CMD_OPERATIONOPTIONSSET 0x0D62

/**
   Performs some miscellaneous chip set configurations.
*/
typedef struct CMD_OperationOptionsSet CMD_OperationOptionsSet_t;

/** Message ID for ACK_OperationOptionsSet */
#define ACK_OPERATIONOPTIONSSET 0x0D62

/**
   This is the acknowledgement for CMD_OperationOptionsSet.
*/
typedef struct ACK_OperationOptionsSet ACK_OperationOptionsSet_t;

/** Message ID for CMD_OperatorSelect */
#define CMD_OPERATORSELECT 0x1562

/**
   The message selects a DSL operator. The information is used to configure
   operator specific settings inside the DSL firmware.
*/
typedef struct CMD_OperatorSelect CMD_OperatorSelect_t;

/** Message ID for ACK_OperatorSelect */
#define ACK_OPERATORSELECT 0x1562

/**
   This is the acknowledgement for ACK_OperatorSelect.
*/
typedef struct ACK_OperatorSelect ACK_OperatorSelect_t;

/** Message ID for CMD_OperatorSelectionGet */
#define CMD_OPERATORSELECTIONGET 0x1522

/**
   The message reads back the selected DSL operator, as selected by
   ACK_OperatorSelect.
*/
typedef struct CMD_OperatorSelectionGet CMD_OperatorSelectionGet_t;

/** Message ID for ACK_OperatorSelectionGet */
#define ACK_OPERATORSELECTIONGET 0x1522

/**
   The message provides the selected DSL operator.
*/
typedef struct ACK_OperatorSelectionGet ACK_OperatorSelectionGet_t;

/** Message ID for CMD_TestOptionsSet */
#define CMD_TESTOPTIONSSET 0x1C44

/**
   The messages configures settings for test modes.
*/
typedef struct CMD_TestOptionsSet CMD_TestOptionsSet_t;

/** Message ID for ACK_TestOptionsSet */
#define ACK_TESTOPTIONSSET 0x1C44

/**
   This is the acknowledgement for CMD_TestOptionsSet.
*/
typedef struct ACK_TestOptionsSet ACK_TestOptionsSet_t;

/** Message ID for CMD_ClockSet */
#define CMD_CLOCKSET 0x0F62

/**
   Sets the PPE clock.
*/
typedef struct CMD_ClockSet CMD_ClockSet_t;

/** Message ID for ACK_ClockSet */
#define ACK_CLOCKSET 0x0F62

/**
   This is the acknowledgement for CMD_ClockSet.
*/
typedef struct ACK_ClockSet ACK_ClockSet_t;

/** Message ID for CMD_PLL_ClockSet */
#define CMD_PLL_CLOCKSET 0x1962

/**
   Sets an offset for the PLL frequency compared to the crystals rated nominal
   frequency of 36 MHz. This allows e.g. to finetune the handshake tone
   frequencies to exactly match the standard values.
*/
typedef struct CMD_PLL_ClockSet CMD_PLL_ClockSet_t;

/** Message ID for ACK_PLL_ClockSet */
#define ACK_PLL_CLOCKSET 0x1962

/**
   This is the acknowledgement for CMD_PLL_ClockSet.
*/
typedef struct ACK_PLL_ClockSet ACK_PLL_ClockSet_t;

/**
   Requests information about the current state of the modem state-machine.The
   command can be sent in all states of the modem state machine (see Figure 2).
*/
struct CMD_ModemFSM_StateGet
{
#if DSL_BYTE_ORDER == DSL_BIG_ENDIAN
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
#else
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
#endif
} __PACKED__ ;


/**
   Returns information about the current state of the modem state-machine.
*/
struct ACK_ModemFSM_StateGet
{
#if DSL_BYTE_ORDER == DSL_BIG_ENDIAN
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
   /** Modem Status */
   DSL_uint16_t ModemState;
   /** Reserved */
   DSL_uint16_t Res0 : 14;
   /** Line Power Management State */
   DSL_uint16_t LxState : 2;
   /** Last HS State or SOC Message */
   DSL_uint16_t State_HsOrSoc;
   /** RX Signal */
   DSL_uint16_t RxSignal;
   /** TX Signal */
   DSL_uint16_t TxSignal;
#else
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
   /** Modem Status */
   DSL_uint16_t ModemState;
   /** Line Power Management State */
   DSL_uint16_t LxState : 2;
   /** Reserved */
   DSL_uint16_t Res0 : 14;
   /** Last HS State or SOC Message */
   DSL_uint16_t State_HsOrSoc;
   /** RX Signal */
   DSL_uint16_t RxSignal;
   /** TX Signal */
   DSL_uint16_t TxSignal;
#endif
} __PACKED__ ;


/**
   Returns information about the current state of the modem state-machine. This
   message is sent autonomously, if the modem enters a new state and reporting
   is enabled for that state (see CMD_ModemFSM_EventConfigure).
*/
struct EVT_ModemFSM_StateGet
{
#if DSL_BYTE_ORDER == DSL_BIG_ENDIAN
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
   /** Modem Status */
   DSL_uint16_t ModemState;
   /** Reserved */
   DSL_uint16_t Res0 : 14;
   /** Line Power Management State */
   DSL_uint16_t LxState : 2;
   /** Last HS State or SOC Message */
   DSL_uint16_t State_HsOrSoc;
   /** RX Signal */
   DSL_uint16_t RxSignal;
   /** TX Signal */
   DSL_uint16_t TxSignal;
#else
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
   /** Modem Status */
   DSL_uint16_t ModemState;
   /** Line Power Management State */
   DSL_uint16_t LxState : 2;
   /** Reserved */
   DSL_uint16_t Res0 : 14;
   /** Last HS State or SOC Message */
   DSL_uint16_t State_HsOrSoc;
   /** RX Signal */
   DSL_uint16_t RxSignal;
   /** TX Signal */
   DSL_uint16_t TxSignal;
#endif
} __PACKED__ ;


/**
   Autonomous message indicating that the modem online code was reached and that
   initialization was completed. The management entity may not perform any
   query-response messaging until the EVT_ModemReady message is received. Its
   generation cannot be disabled. If it is not received this indicates a
   problem, possibly with the boot sequence. If the message is received, it
   contains a result code that may also indicate that an error was detected by
   the firmware during initialization.
*/
struct EVT_ModemReady
{
#if DSL_BYTE_ORDER == DSL_BIG_ENDIAN
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
   /** Error Code */
   DSL_uint8_t ErrorCode;
   /** Warning Code */
   DSL_uint8_t WarningCode;
#else
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
   /** Warning Code */
   DSL_uint8_t WarningCode;
   /** Error Code */
   DSL_uint8_t ErrorCode;
#endif
} __PACKED__ ;


/**
   Controls state transitions of the modem state-machine.
*/
struct CMD_ModemFSM_StateSet
{
#if DSL_BYTE_ORDER == DSL_BIG_ENDIAN
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
   /** Reserved */
   DSL_uint16_t Res0 : 12;
   /** Link Control */
   DSL_uint16_t LinkControl : 4;
#else
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
   /** Link Control */
   DSL_uint16_t LinkControl : 4;
   /** Reserved */
   DSL_uint16_t Res0 : 12;
#endif
} __PACKED__ ;


/**
   Acknowledgement for message CMD_ModemFSM_StateSet
*/
struct ACK_ModemFSM_StateSet
{
#if DSL_BYTE_ORDER == DSL_BIG_ENDIAN
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
#else
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
#endif
} __PACKED__ ;


/**
   Reports failure information after entering FAIL state. This message is sent
   autonomously without host request after entrance into FAIL was indicated by
   EVT_ModemFSM_StateGet. Both messages are generated only if enabled with
   CMD_ModemFSM_EventConfigure (common Bit E8 "FAIL").
*/
struct ALM_ModemFSM_FailReasonGet
{
#if DSL_BYTE_ORDER == DSL_BIG_ENDIAN
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
   /** System Sub Error Code */
   DSL_uint8_t SubErrorCode;
   /** System Error Code */
   DSL_uint8_t ErrorCode;
   /** Failure State Information */
   DSL_uint16_t FW_FailCode;
   /** Reserved */
   DSL_uint16_t Res0[8];
#else
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
   /** System Error Code */
   DSL_uint8_t ErrorCode;
   /** System Sub Error Code */
   DSL_uint8_t SubErrorCode;
   /** Failure State Information */
   DSL_uint16_t FW_FailCode;
   /** Reserved */
   DSL_uint16_t Res0[8];
#endif
} __PACKED__ ;


/**
   Requests failure information after entering FAIL state.
*/
struct CMD_ModemFSM_FailReasonGet
{
#if DSL_BYTE_ORDER == DSL_BIG_ENDIAN
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
#else
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
#endif
} __PACKED__ ;


/**
   Returns failure information after entering FAIL state.
*/
struct ACK_ModemFSM_FailReasonGet
{
#if DSL_BYTE_ORDER == DSL_BIG_ENDIAN
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
   /** System Sub Error Code */
   DSL_uint8_t SubErrorCode;
   /** System Error Code */
   DSL_uint8_t ErrorCode;
   /** Failure State Information */
   DSL_uint16_t FW_FailCode;
   /** Reserved */
   DSL_uint16_t Res0[8];
#else
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
   /** System Error Code */
   DSL_uint8_t ErrorCode;
   /** System Sub Error Code */
   DSL_uint8_t SubErrorCode;
   /** Failure State Information */
   DSL_uint16_t FW_FailCode;
   /** Reserved */
   DSL_uint16_t Res0[8];
#endif
} __PACKED__ ;


/**
   Configuration of options for the modem state machine.
*/
struct CMD_ModemFSM_OptionsSet
{
#if DSL_BYTE_ORDER == DSL_BIG_ENDIAN
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
   /** Reserved */
   DSL_uint16_t Res0 : 5;
   /** L2 Extensions of G.992.3 Amd4 (2011), (ADSL only), Bit 10 */
   DSL_uint16_t E10 : 1;
   /** L2 Automatic Exit (ADSL only), Bit 9 */
   DSL_uint16_t E9 : 1;
   /** L2 Low-Power Mode Enable (ADSL only), Bit 8 */
   DSL_uint16_t E8 : 1;
   /** Reserved */
   DSL_uint16_t Res1 : 5;
   /** Loop Diagnostic Mode Control, Bit 2 */
   DSL_uint16_t E2 : 1;
   /** Reserved 0 */
   DSL_uint16_t E1 : 1;
   /** Automatic Re-Start Control, Bit 0 */
   DSL_uint16_t E0 : 1;
#else
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
   /** Automatic Re-Start Control, Bit 0 */
   DSL_uint16_t E0 : 1;
   /** Reserved 0 */
   DSL_uint16_t E1 : 1;
   /** Loop Diagnostic Mode Control, Bit 2 */
   DSL_uint16_t E2 : 1;
   /** Reserved */
   DSL_uint16_t Res1 : 5;
   /** L2 Low-Power Mode Enable (ADSL only), Bit 8 */
   DSL_uint16_t E8 : 1;
   /** L2 Automatic Exit (ADSL only), Bit 9 */
   DSL_uint16_t E9 : 1;
   /** L2 Extensions of G.992.3 Amd4 (2011), (ADSL only), Bit 10 */
   DSL_uint16_t E10 : 1;
   /** Reserved */
   DSL_uint16_t Res0 : 5;
#endif
} __PACKED__ ;


/**
   Acknowledgement to CMD_ModemFSM_OptionsSet.
*/
struct ACK_ModemFSM_OptionsSet
{
#if DSL_BYTE_ORDER == DSL_BIG_ENDIAN
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
#else
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
#endif
} __PACKED__ ;


/**
   Configuration of options for the modem state machine.
*/
struct CMD_ModemOptionsSet
{
#if DSL_BYTE_ORDER == DSL_BIG_ENDIAN
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
   /** Reserved */
   DSL_uint16_t Res0 : 10;
   /** Steady-State Algorithm Control, Bit5 */
   DSL_uint16_t Trellis : 1;
   /** Reserved */
   DSL_uint16_t Res1 : 1;
   /** AELEM Control (VDSL only), Bit 3 */
   DSL_uint16_t enableAelem : 1;
   /** Short Init Control (ADSL only), Bit 2 */
   DSL_uint16_t shortInit : 1;
   /** US Virtual Noise Support, Bit 1 */
   DSL_uint16_t enableVN_US : 1;
   /** DS Virtual Noise Support, Bit 0 */
   DSL_uint16_t enableVN_DS : 1;
#else
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
   /** DS Virtual Noise Support, Bit 0 */
   DSL_uint16_t enableVN_DS : 1;
   /** US Virtual Noise Support, Bit 1 */
   DSL_uint16_t enableVN_US : 1;
   /** Short Init Control (ADSL only), Bit 2 */
   DSL_uint16_t shortInit : 1;
   /** AELEM Control (VDSL only), Bit 3 */
   DSL_uint16_t enableAelem : 1;
   /** Reserved */
   DSL_uint16_t Res1 : 1;
   /** Steady-State Algorithm Control, Bit5 */
   DSL_uint16_t Trellis : 1;
   /** Reserved */
   DSL_uint16_t Res0 : 10;
#endif
} __PACKED__ ;


/**
   Acknowledgement to CMD_ModemOptionsSet.
*/
struct ACK_ModemOptionsSet
{
#if DSL_BYTE_ORDER == DSL_BIG_ENDIAN
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
#else
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
#endif
} __PACKED__ ;


/**
   Sets options for the modem state-machine. Enables/Disables autonomous
   messages for specific state transitions (EVT_ModemFSM_StateGet). The EVT
   message is generated after entering the particular state listed in the
   parameter description of the enable bits.
*/
struct CMD_ModemFSM_EventConfigure
{
#if DSL_BYTE_ORDER == DSL_BIG_ENDIAN
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
   /** Enable Bit 15 */
   DSL_uint16_t E15 : 1;
   /** Enable Bit 14 */
   DSL_uint16_t E14 : 1;
   /** Enable Bit 13 */
   DSL_uint16_t E13 : 1;
   /** Enable Bit 12 */
   DSL_uint16_t E12 : 1;
   /** Enable Bit 11 */
   DSL_uint16_t E11 : 1;
   /** Enable Bit 10 */
   DSL_uint16_t E10 : 1;
   /** Enable Bit 9 */
   DSL_uint16_t E9 : 1;
   /** Enable Bit 8 */
   DSL_uint16_t E8 : 1;
   /** Enable Bit 7 */
   DSL_uint16_t E7 : 1;
   /** Enable Bit 6 */
   DSL_uint16_t E6 : 1;
   /** Enable Bit 5 */
   DSL_uint16_t E5 : 1;
   /** Enable Bit 4 */
   DSL_uint16_t E4 : 1;
   /** Enable Bit 3 */
   DSL_uint16_t E3 : 1;
   /** Enable Bit 2 */
   DSL_uint16_t E2 : 1;
   /** Enable Bit 1 (ADSL only) */
   DSL_uint16_t E1 : 1;
   /** Enable Bit 0 */
   DSL_uint16_t E0 : 1;
#else
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
   /** Enable Bit 0 */
   DSL_uint16_t E0 : 1;
   /** Enable Bit 1 (ADSL only) */
   DSL_uint16_t E1 : 1;
   /** Enable Bit 2 */
   DSL_uint16_t E2 : 1;
   /** Enable Bit 3 */
   DSL_uint16_t E3 : 1;
   /** Enable Bit 4 */
   DSL_uint16_t E4 : 1;
   /** Enable Bit 5 */
   DSL_uint16_t E5 : 1;
   /** Enable Bit 6 */
   DSL_uint16_t E6 : 1;
   /** Enable Bit 7 */
   DSL_uint16_t E7 : 1;
   /** Enable Bit 8 */
   DSL_uint16_t E8 : 1;
   /** Enable Bit 9 */
   DSL_uint16_t E9 : 1;
   /** Enable Bit 10 */
   DSL_uint16_t E10 : 1;
   /** Enable Bit 11 */
   DSL_uint16_t E11 : 1;
   /** Enable Bit 12 */
   DSL_uint16_t E12 : 1;
   /** Enable Bit 13 */
   DSL_uint16_t E13 : 1;
   /** Enable Bit 14 */
   DSL_uint16_t E14 : 1;
   /** Enable Bit 15 */
   DSL_uint16_t E15 : 1;
#endif
} __PACKED__ ;


/**
   Acknowledgement for CMD_ModemFSM_EventConfigure. (Enabling/Disabling EVENT
   messages (EVTs) for specific state transitions.)
*/
struct ACK_ModemFSM_EventConfigure
{
#if DSL_BYTE_ORDER == DSL_BIG_ENDIAN
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
#else
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
#endif
} __PACKED__ ;


/**
   Enables/Disables the generation of ALARM messages (ALM) for specific near-end
   line failures. If the corresponding Enable bit for a line failure is set,
   then the modem firmware will send an autonomous message ALM_LineFailureNE_Get
   if this failure occurs.
*/
struct CMD_FailuresNE_AlarmConfigure
{
#if DSL_BYTE_ORDER == DSL_BIG_ENDIAN
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
   /** Reserved */
   DSL_uint16_t Res0 : 2;
   /** Enable Bit 13 */
   DSL_uint16_t E13 : 1;
   /** Reserved */
   DSL_uint16_t Res1 : 9;
   /** Enable Bit3 */
   DSL_uint16_t E3 : 1;
   /** Reserved */
   DSL_uint16_t Res2 : 1;
   /** Enable Bit1 */
   DSL_uint16_t E1 : 1;
   /** Enable Bit0 */
   DSL_uint16_t E0 : 1;
#else
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
   /** Enable Bit0 */
   DSL_uint16_t E0 : 1;
   /** Enable Bit1 */
   DSL_uint16_t E1 : 1;
   /** Reserved */
   DSL_uint16_t Res2 : 1;
   /** Enable Bit3 */
   DSL_uint16_t E3 : 1;
   /** Reserved */
   DSL_uint16_t Res1 : 9;
   /** Enable Bit 13 */
   DSL_uint16_t E13 : 1;
   /** Reserved */
   DSL_uint16_t Res0 : 2;
#endif
} __PACKED__ ;


/**
   Acknowledgement to CMD_FailuresNE_AlarmConfigure.
*/
struct ACK_FailuresNE_AlarmConfigure
{
#if DSL_BYTE_ORDER == DSL_BIG_ENDIAN
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
#else
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
#endif
} __PACKED__ ;


/**
   Enables/Disables the generation of ALARM messages (ALM) for specific far-end
   line failures. If the corresponding Enable bit for a line failure is set,
   then the modem firmware will send an autonomous message ALM_LineFailureFE_Get
   if this failure occurs.
*/
struct CMD_FailuresFE_AlarmConfigure
{
#if DSL_BYTE_ORDER == DSL_BIG_ENDIAN
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
   /** Reserved */
   DSL_uint16_t Res0 : 12;
   /** Enable Bit3 */
   DSL_uint16_t E3 : 1;
   /** Enable Bit2 */
   DSL_uint16_t E2 : 1;
   /** Reserved */
   DSL_uint16_t Res1 : 1;
   /** Enable Bit0 */
   DSL_uint16_t E0 : 1;
#else
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
   /** Enable Bit0 */
   DSL_uint16_t E0 : 1;
   /** Reserved */
   DSL_uint16_t Res1 : 1;
   /** Enable Bit2 */
   DSL_uint16_t E2 : 1;
   /** Enable Bit3 */
   DSL_uint16_t E3 : 1;
   /** Reserved */
   DSL_uint16_t Res0 : 12;
#endif
} __PACKED__ ;


/**
   Acknowledgement to CMD_FailuresFE_AlarmConfigure.
*/
struct ACK_FailuresFE_AlarmConfigure
{
#if DSL_BYTE_ORDER == DSL_BIG_ENDIAN
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
#else
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
#endif
} __PACKED__ ;


/**
   Configures re-initialization triggers for near-end failure conditions in
   modem state STEADY STATE transmission (see Figure 1). This command is
   accepted in RESET state only.
*/
struct CMD_ReInitNE_Configure
{
#if DSL_BYTE_ORDER == DSL_BIG_ENDIAN
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
   /** Reserved */
   DSL_uint16_t Res0 : 2;
   /** ESE */
   DSL_uint16_t E13 : 1;
   /** Reserved */
   DSL_uint16_t Res1 : 2;
   /** OOS BC0 */
   DSL_uint16_t E10 : 1;
   /** Reserved */
   DSL_uint16_t Res2 : 1;
   /** NCD BC0 */
   DSL_uint16_t E8 : 1;
   /** Reserved */
   DSL_uint16_t Res3 : 1;
   /** LCD BC0 */
   DSL_uint16_t E6 : 1;
   /** Reserved */
   DSL_uint16_t Res4 : 2;
   /** LOM */
   DSL_uint16_t E3 : 1;
   /** Reserved */
   DSL_uint16_t Res5 : 1;
   /** LOF */
   DSL_uint16_t E1 : 1;
   /** LOS */
   DSL_uint16_t E0 : 1;
   /** Reserved */
   DSL_uint16_t Res6 : 14;
   /** ESx */
   DSL_uint16_t F1 : 1;
   /** SESx */
   DSL_uint16_t F0 : 1;
#else
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
   /** LOS */
   DSL_uint16_t E0 : 1;
   /** LOF */
   DSL_uint16_t E1 : 1;
   /** Reserved */
   DSL_uint16_t Res5 : 1;
   /** LOM */
   DSL_uint16_t E3 : 1;
   /** Reserved */
   DSL_uint16_t Res4 : 2;
   /** LCD BC0 */
   DSL_uint16_t E6 : 1;
   /** Reserved */
   DSL_uint16_t Res3 : 1;
   /** NCD BC0 */
   DSL_uint16_t E8 : 1;
   /** Reserved */
   DSL_uint16_t Res2 : 1;
   /** OOS BC0 */
   DSL_uint16_t E10 : 1;
   /** Reserved */
   DSL_uint16_t Res1 : 2;
   /** ESE */
   DSL_uint16_t E13 : 1;
   /** Reserved */
   DSL_uint16_t Res0 : 2;
   /** SESx */
   DSL_uint16_t F0 : 1;
   /** ESx */
   DSL_uint16_t F1 : 1;
   /** Reserved */
   DSL_uint16_t Res6 : 14;
#endif
} __PACKED__ ;


/**
   Acknowledgment to CMD_ReInitNE_Configure (Configuration of re-initialization
   triggers for near-end failures).
*/
struct ACK_ReInitNE_Configure
{
#if DSL_BYTE_ORDER == DSL_BIG_ENDIAN
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
#else
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
#endif
} __PACKED__ ;


/**
   Configuration of reinitialization trigger definitions.
*/
struct CMD_ReinitThreshConfigure
{
#if DSL_BYTE_ORDER == DSL_BIG_ENDIAN
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
   /** SESx Reinit Period */
   DSL_uint16_t SESxPeriod;
   /** LOM Persistency Time for Reinit */
   DSL_uint16_t LomRiPeriod;
   /** ESx Reinit Period */
   DSL_uint16_t ESxPeriod;
#else
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
   /** SESx Reinit Period */
   DSL_uint16_t SESxPeriod;
   /** LOM Persistency Time for Reinit */
   DSL_uint16_t LomRiPeriod;
   /** ESx Reinit Period */
   DSL_uint16_t ESxPeriod;
#endif
} __PACKED__ ;


/**
   Acknowledgement for the message ACK_ReinitThreshConfigure.
*/
struct ACK_ReinitThreshConfigure
{
#if DSL_BYTE_ORDER == DSL_BIG_ENDIAN
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
#else
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
#endif
} __PACKED__ ;


/**
   Triggers a shutdown request, either as "L3 orderly shutdown" towards the
   remote side or as "locally forced shutdown" to the DSL-FW without sending a
   request to the remote side.After an "L3 orderly shutdown" request was
   accepted by the CO, the following shall happen: The CPE-Host forces L3 entry
   with transition to RESET state by applying CMD_ModemFSM_StateGet.A "locally
   forced shutdown" always results in exiting Showtime.
*/
struct CMD_ShutdownRequest
{
#if DSL_BYTE_ORDER == DSL_BIG_ENDIAN
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
   /** Reserved */
   DSL_uint16_t Res0 : 14;
   /** Locally Forced Shutdown */
   DSL_uint16_t ForcedShutdown : 1;
   /** L3 Orderly Shutdown Request */
   DSL_uint16_t L3shutdown : 1;
#else
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
   /** L3 Orderly Shutdown Request */
   DSL_uint16_t L3shutdown : 1;
   /** Locally Forced Shutdown */
   DSL_uint16_t ForcedShutdown : 1;
   /** Reserved */
   DSL_uint16_t Res0 : 14;
#endif
} __PACKED__ ;


/**
   Acknowledgement for CMD_ShutdownRequest.
*/
struct ACK_ShutdownRequest
{
#if DSL_BYTE_ORDER == DSL_BIG_ENDIAN
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
#else
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
#endif
} __PACKED__ ;


/**
   Requests information about L3 orderly shutdown requests initiated from the
   remote side.Applying CMD_RxL3RequestStatusGet allows the host to determine
   whether a near-end LOS or LOM failure is due to an L3 shutdown or an
   unexpected line failure.
*/
struct CMD_RxL3RequestStatusGet
{
#if DSL_BYTE_ORDER == DSL_BIG_ENDIAN
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
#else
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
#endif
} __PACKED__ ;


/**
   Acknowledgement for CMD_RxL3RequestStatusGet.As long as the modem is in L0
   (Showtime), an ATU-R will automatically respond to any L3 request from the
   ATU-C by sending an accept message.  If the ATU-R is in L2 mode when it
   receives the request, it will send a reject message. Upon receiving an
   acceptance, the remote side should enter L3 state and shut off its
   transmitter.
*/
struct ACK_RxL3RequestStatusGet
{
#if DSL_BYTE_ORDER == DSL_BIG_ENDIAN
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
   /** L3 Request Response */
   DSL_uint16_t L3ReqResponse : 1;
   /** L3 Request Received */
   DSL_uint16_t L3ReqReceived : 1;
   /** Reserved */
   DSL_uint16_t Res0 : 14;
#else
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
   /** Reserved */
   DSL_uint16_t Res0 : 14;
   /** L3 Request Received */
   DSL_uint16_t L3ReqReceived : 1;
   /** L3 Request Response */
   DSL_uint16_t L3ReqResponse : 1;
#endif
} __PACKED__ ;


/**
   Requests the status of a near-end initiated L3 shutdown request. If the
   remote side accepted the request, in ADSL the host will force L3 entry using
   CMD_ModemFSM_StateSet.
*/
struct CMD_TxL3RequestStatusGet
{
#if DSL_BYTE_ORDER == DSL_BIG_ENDIAN
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
#else
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
#endif
} __PACKED__ ;


/**
   Acknowledgement for CMD_TxL3RequestStatusGet.
*/
struct ACK_TxL3RequestStatusGet
{
#if DSL_BYTE_ORDER == DSL_BIG_ENDIAN
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
   /** Reserved */
   DSL_uint16_t Res0 : 14;
   /** Status L3 Request */
   DSL_uint16_t L3ReqStatus : 2;
#else
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
   /** Status L3 Request */
   DSL_uint16_t L3ReqStatus : 2;
   /** Reserved */
   DSL_uint16_t Res0 : 14;
#endif
} __PACKED__ ;


/**
   Requests the fail reason of a failed near-end initiated L3 request. (see also
   ACK_TxL3RequestStatusGet)
*/
struct CMD_TxL3ReqFailReasonGet
{
#if DSL_BYTE_ORDER == DSL_BIG_ENDIAN
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
#else
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
#endif
} __PACKED__ ;


/**
   Reports the fail reason of a near-end initiated L3 request, as response to
   CMD_TxL3ReqFailReasonGet.
*/
struct ACK_TxL3ReqFailReasonGet
{
#if DSL_BYTE_ORDER == DSL_BIG_ENDIAN
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
   /** Reserved */
   DSL_uint16_t Res0 : 8;
   /** L3 Fail Reason */
   DSL_uint8_t L3FailReason;
#else
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
   /** L3 Fail Reason */
   DSL_uint8_t L3FailReason;
   /** Reserved */
   DSL_uint16_t Res0 : 8;
#endif
} __PACKED__ ;


/**
   The message selects and enables one of several points during the
   initialization process where -after a normal link start had been triggered-
   the modem FSM "freezes". Freezing means the CO can be disconnected from the
   line and still the CPE continues transmitting the signal of the current
   phase. To exit from the freeze a state transition to RESET state has to be
   triggered. In case of a Showtime Freeze the reinit-triggers do not have any
   effect.
*/
struct CMD_ModemFSM_Freeze
{
#if DSL_BYTE_ORDER == DSL_BIG_ENDIAN
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
   /** Freeze Points */
   DSL_uint16_t FreezePoint;
#else
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
   /** Freeze Points */
   DSL_uint16_t FreezePoint;
#endif
} __PACKED__ ;


/**
   Acknowledgement for message CMD_ModemFSM_Freeze.
*/
struct ACK_ModemFSM_Freeze
{
#if DSL_BYTE_ORDER == DSL_BIG_ENDIAN
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
#else
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
#endif
} __PACKED__ ;


/**
   Requests information about the loaded FW image. This info is used e.g. for
   swapping between ADSL and VDSL.
*/
struct CMD_FW_ImageInfoGet
{
#if DSL_BYTE_ORDER == DSL_BIG_ENDIAN
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
#else
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
#endif
} __PACKED__ ;


/**
   Provides the information about the FW image requested by CMD_FW_ImageInfoGet
*/
struct ACK_FW_ImageInfoGet
{
#if DSL_BYTE_ORDER == DSL_BIG_ENDIAN
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
   /** Reserved */
   DSL_uint16_t Res0 : 13;
   /** Site */
   DSL_uint16_t imageType : 1;
   /** DSL mode */
   DSL_uint16_t dslMode : 1;
   /** Site */
   DSL_uint16_t location : 1;
#else
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
   /** Site */
   DSL_uint16_t location : 1;
   /** DSL mode */
   DSL_uint16_t dslMode : 1;
   /** Site */
   DSL_uint16_t imageType : 1;
   /** Reserved */
   DSL_uint16_t Res0 : 13;
#endif
} __PACKED__ ;


/**
   The message informs the FW about the status of SW parameters relevant for the
   multimode state machine (APS). This can be used to trace the APS behaviour
   with FW tools. The message does not influence the APS behaviour.
*/
struct CMD_APS_Status_Set
{
#if DSL_BYTE_ORDER == DSL_BIG_ENDIAN
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
   /** Port Mode */
   DSL_uint16_t DualPortMode;
   /** NextMode */
   DSL_uint16_t NextMode;
   /** Remember Function */
   DSL_uint16_t bRemember;
   /** Activation Sequence */
   DSL_uint16_t ActSeq;
   /** Activation Start Mode (ADSL only) */
   DSL_uint16_t ActMode;
   /** nRetry  */
   DSL_uint16_t nRetry;
   /** nFwRetry  */
   DSL_uint16_t nFwRetry;
   /** nGhsRetry  */
   DSL_uint16_t nGhsRetry;
#else
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
   /** Port Mode */
   DSL_uint16_t DualPortMode;
   /** NextMode */
   DSL_uint16_t NextMode;
   /** Remember Function */
   DSL_uint16_t bRemember;
   /** Activation Sequence */
   DSL_uint16_t ActSeq;
   /** Activation Start Mode (ADSL only) */
   DSL_uint16_t ActMode;
   /** nRetry  */
   DSL_uint16_t nRetry;
   /** nFwRetry  */
   DSL_uint16_t nFwRetry;
   /** nGhsRetry  */
   DSL_uint16_t nGhsRetry;
#endif
} __PACKED__ ;


/**
   Acknowledgement for message CMD_APS_Status_Set.
*/
struct ACK_APS_Status_Set
{
#if DSL_BYTE_ORDER == DSL_BIG_ENDIAN
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
#else
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
#endif
} __PACKED__ ;


/**
   Configuration of the VTU Transmission System Enabling (XTSE).Configures the
   transmission system coding types to be supported on the line (VDSL flavour
   and annex support).(References: Section 7.3.1.1.1 of G.997.1 and G.994.1 Amd4
   [10] Tables "Standard information field - SPar(1) coding")
*/
struct CMD_XTSE_Configure
{
#if DSL_BYTE_ORDER == DSL_BIG_ENDIAN
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
   /** ADSL Mode-Bit15: G.992.5, Annex A */
   DSL_uint16_t A15 : 1;
   /** ADSL Mode-Bit14: G.992.5, Annex B */
   DSL_uint16_t A14 : 1;
   /** ADSL Mode-Bit13: G.992.3, Annex M */
   DSL_uint16_t A13 : 1;
   /** ADSL Mode-Bit12: G.992.3, Annex L, US Mask2 */
   DSL_uint16_t A12 : 1;
   /** ADSL Mode-Bit11: G.992.3, Annex L, US Mask1 */
   DSL_uint16_t A11 : 1;
   /** ADSL Mode-Bit10: G.992.2, Annex A */
   DSL_uint16_t A10 : 1;
   /** ADSL Mode-Bit9: G.992.3, Annex B */
   DSL_uint16_t A9 : 1;
   /** ADSL Mode-Bit8: G.992.3, Annex A */
   DSL_uint16_t A8 : 1;
   /** ADSL Mode-Bit7: G.992.5, Annex J */
   DSL_uint16_t A7 : 1;
   /** ADSL Mode-Bit6: G.992.5, Annex I */
   DSL_uint16_t A6 : 1;
   /** ADSL Mode-Bit5: G.992.3, Annex J */
   DSL_uint16_t A5 : 1;
   /** ADSL Mode-Bit4: G.992.3, Annex I */
   DSL_uint16_t A4 : 1;
   /** ADSL Mode-Bit3: G.992.1, Annex B */
   DSL_uint16_t A3 : 1;
   /** ADSL Mode-Bit2: G.992.1, Annex A */
   DSL_uint16_t A2 : 1;
   /** ADSL Mode-Bit1: G.992.5, Annex M */
   DSL_uint16_t A1 : 1;
   /** ADSL Mode-Bit0: T1.413 */
   DSL_uint16_t A0 : 1;
   /** Reserved */
   DSL_uint16_t Res0 : 7;
   /** Bit8: G.FAST */
   DSL_uint16_t F8 : 1;
   /** Reserved */
   DSL_uint16_t Res1 : 5;
   /** VDSL Mode-Bit2: VDSL2 */
   DSL_uint16_t V2 : 1;
   /** Reserved */
   DSL_uint16_t Res2 : 2;
#else
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
   /** ADSL Mode-Bit0: T1.413 */
   DSL_uint16_t A0 : 1;
   /** ADSL Mode-Bit1: G.992.5, Annex M */
   DSL_uint16_t A1 : 1;
   /** ADSL Mode-Bit2: G.992.1, Annex A */
   DSL_uint16_t A2 : 1;
   /** ADSL Mode-Bit3: G.992.1, Annex B */
   DSL_uint16_t A3 : 1;
   /** ADSL Mode-Bit4: G.992.3, Annex I */
   DSL_uint16_t A4 : 1;
   /** ADSL Mode-Bit5: G.992.3, Annex J */
   DSL_uint16_t A5 : 1;
   /** ADSL Mode-Bit6: G.992.5, Annex I */
   DSL_uint16_t A6 : 1;
   /** ADSL Mode-Bit7: G.992.5, Annex J */
   DSL_uint16_t A7 : 1;
   /** ADSL Mode-Bit8: G.992.3, Annex A */
   DSL_uint16_t A8 : 1;
   /** ADSL Mode-Bit9: G.992.3, Annex B */
   DSL_uint16_t A9 : 1;
   /** ADSL Mode-Bit10: G.992.2, Annex A */
   DSL_uint16_t A10 : 1;
   /** ADSL Mode-Bit11: G.992.3, Annex L, US Mask1 */
   DSL_uint16_t A11 : 1;
   /** ADSL Mode-Bit12: G.992.3, Annex L, US Mask2 */
   DSL_uint16_t A12 : 1;
   /** ADSL Mode-Bit13: G.992.3, Annex M */
   DSL_uint16_t A13 : 1;
   /** ADSL Mode-Bit14: G.992.5, Annex B */
   DSL_uint16_t A14 : 1;
   /** ADSL Mode-Bit15: G.992.5, Annex A */
   DSL_uint16_t A15 : 1;
   /** Reserved */
   DSL_uint16_t Res2 : 2;
   /** VDSL Mode-Bit2: VDSL2 */
   DSL_uint16_t V2 : 1;
   /** Reserved */
   DSL_uint16_t Res1 : 5;
   /** Bit8: G.FAST */
   DSL_uint16_t F8 : 1;
   /** Reserved */
   DSL_uint16_t Res0 : 7;
#endif
} __PACKED__ ;


/**
   Acknowledgement for message CMD_XTSE_Configure.
*/
struct ACK_XTSE_Configure
{
#if DSL_BYTE_ORDER == DSL_BIG_ENDIAN
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
#else
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
#endif
} __PACKED__ ;


/**
   Controls the upstream band usage.The message can only be used for DMTscope
   tests without handshake and training phases.
*/
struct CMD_BandControl_US_Set
{
#if DSL_BYTE_ORDER == DSL_BIG_ENDIAN
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
   /** Reserved */
   DSL_uint8_t Res0;
   /** Number of Upstream Bands */
   DSL_uint8_t NumBandsUS;
   /** Band Descriptor US */
   VRX_ToneIndex_t band[8];
#else
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
   /** Number of Upstream Bands */
   DSL_uint8_t NumBandsUS;
   /** Reserved */
   DSL_uint8_t Res0;
   /** Band Descriptor US */
   VRX_ToneIndex_t band[8];
#endif
} __PACKED__ ;


/**
   Acknowledgement for message CMD_BandControl_US_Set.
*/
struct ACK_BandControl_US_Set
{
#if DSL_BYTE_ORDER == DSL_BIG_ENDIAN
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
#else
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
#endif
} __PACKED__ ;


/**
   Configuration of PSD and Power parameters. Only used for DMTscope tests
   without handshake and training phases.
*/
struct CMD_PSD_Set
{
#if DSL_BYTE_ORDER == DSL_BIG_ENDIAN
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
   /** Reserved */
   DSL_uint16_t Res0;
   /** MAXNOMPSDus */
   DSL_uint16_t MAXNOMPSDus;
   /** Reserved */
   DSL_uint16_t Res1;
   /** MAXNOMATPus */
   DSL_uint16_t MAXNOMATPus;
   /** Reserved */
   DSL_uint16_t Res2;
#else
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
   /** Reserved */
   DSL_uint16_t Res0;
   /** MAXNOMPSDus */
   DSL_uint16_t MAXNOMPSDus;
   /** Reserved */
   DSL_uint16_t Res1;
   /** MAXNOMATPus */
   DSL_uint16_t MAXNOMATPus;
   /** Reserved */
   DSL_uint16_t Res2;
#endif
} __PACKED__ ;


/**
   Acknowledgement for the message CMD_PSD_Set.
*/
struct ACK_PSD_Set
{
#if DSL_BYTE_ORDER == DSL_BIG_ENDIAN
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
#else
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
#endif
} __PACKED__ ;


/**
   Specifies the maximum upstream transmit PSD by means of breakpoints.To be
   used only for DMTscope tests without handshake and training phases.
*/
struct CMD_PSD_BreakpointsTxUS_Set
{
#if DSL_BYTE_ORDER == DSL_BIG_ENDIAN
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
   /** Reserved */
   DSL_uint8_t Res0;
   /** Number of Breakpoints */
   DSL_uint8_t NumBreakPts;
   /** Breakpoint Descriptor TxPSDus */
   VRX_PSDbreak_t breakpoint[32];
#else
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
   /** Number of Breakpoints */
   DSL_uint8_t NumBreakPts;
   /** Reserved */
   DSL_uint8_t Res0;
   /** Breakpoint Descriptor TxPSDus */
   VRX_PSDbreak_t breakpoint[32];
#endif
} __PACKED__ ;


/**
   Acknowledgement for the message CMD_PSD_BreakpointsTxUS_Set.
*/
struct ACK_PSD_BreakpointsTxUS_Set
{
#if DSL_BYTE_ORDER == DSL_BIG_ENDIAN
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
#else
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
#endif
} __PACKED__ ;


/**
   Sends downstream calibration information for "per-tone" test parameters. It
   is effective for Loop Diagnostic Mode (DELT) and Showtime. It is the
   responsibility of the manufacturer to provide the data.
*/
struct  CMD_PSD_Calibration_DS_Set
{
#if DSL_BYTE_ORDER == DSL_BIG_ENDIAN
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
   /** Number of Breakpoints */
   DSL_uint16_t NumBreakPts;
   /** Breakpoint Descriptor RxPSD Compensation */
   VRX_PSD_CompBreak_t breakpoint[16];
#else
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
   /** Number of Breakpoints */
   DSL_uint16_t NumBreakPts;
   /** Breakpoint Descriptor RxPSD Compensation */
   VRX_PSD_CompBreak_t breakpoint[16];
#endif
} __PACKED__ ;


/**
   Acknowledgement for the message CMD_PSD_Calibration_DS_Set.
*/
struct ACK_PSD_Calibration_DS_Set
{
#if DSL_BYTE_ORDER == DSL_BIG_ENDIAN
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
#else
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
#endif
} __PACKED__ ;


/**
   Sends upstream calibration information for the upstream transmit PSD. It is
   effective for loop diagnostic mode (DELT) and Showtime. It is the
   responsibility of the manufacturer to provide the data.
*/
struct  CMD_PSD_Calibration_US_Set
{
#if DSL_BYTE_ORDER == DSL_BIG_ENDIAN
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
   /** Number of Breakpoints */
   DSL_uint16_t NumBreakPts;
   /** Breakpoint Descriptor TxPSD Compensation */
   VRX_PSD_CompBreak_t breakpoint[32];
#else
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
   /** Number of Breakpoints */
   DSL_uint16_t NumBreakPts;
   /** Breakpoint Descriptor TxPSD Compensation */
   VRX_PSD_CompBreak_t breakpoint[32];
#endif
} __PACKED__ ;


/**
   Acknowledgement for the message CMD_PSD_Calibration_US_Set.
*/
struct ACK_PSD_Calibration_US_Set
{
#if DSL_BYTE_ORDER == DSL_BIG_ENDIAN
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
#else
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
#endif
} __PACKED__ ;


/**
   Configuration of PSD related options.
*/
struct CMD_PSD_OptionsSet
{
#if DSL_BYTE_ORDER == DSL_BIG_ENDIAN
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
   /** Reserved */
   DSL_uint16_t Res0 : 15;
   /** US TSSI Control, Bit 0 */
   DSL_uint16_t useTssiUS : 1;
#else
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
   /** US TSSI Control, Bit 0 */
   DSL_uint16_t useTssiUS : 1;
   /** Reserved */
   DSL_uint16_t Res0 : 15;
#endif
} __PACKED__ ;


/**
   Acknowledgement to CMD_PSD_OptionsSet.
*/
struct ACK_PSD_OptionsSet
{
#if DSL_BYTE_ORDER == DSL_BIG_ENDIAN
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
#else
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
#endif
} __PACKED__ ;


/**
   Requests the electrical loop length estimate kl0.
*/
struct CMD_UPBO_KL0Get
{
#if DSL_BYTE_ORDER == DSL_BIG_ENDIAN
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
#else
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
#endif
} __PACKED__ ;


/**
   Delivers the data requested by CMD_UPBO_KL0Get.(Section 7.5.1.23 of G.997.1
   [11])
*/
struct ACK_UPBO_KL0Get
{
#if DSL_BYTE_ORDER == DSL_BIG_ENDIAN
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
   /** Kl0 Estimate VTU-R */
   DSL_uint16_t kl0_EstimR;
   /** Kl0 Estimate VTU-O */
   DSL_uint16_t kl0_EstimO;
#else
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
   /** Kl0 Estimate VTU-R */
   DSL_uint16_t kl0_EstimR;
   /** Kl0 Estimate VTU-O */
   DSL_uint16_t kl0_EstimO;
#endif
} __PACKED__ ;


/**
   Requests status parameters for the Upstream  Power  Back-Off  (UPBO)
   alternative  electrical  length  estimation  method (AELEM).
*/
struct CMD_PBO_AELEM_Status_Get
{
#if DSL_BYTE_ORDER == DSL_BIG_ENDIAN
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
#else
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
#endif
} __PACKED__ ;


/**
   Upstream  Power  Back-Off  (UPBO)  status  parameters  for  the  alternative
   electrical  length  estimation  method (AELEM) are reported.
*/
struct ACK_PBO_AELEM_Status_Get
{
#if DSL_BYTE_ORDER == DSL_BIG_ENDIAN
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
   /** AELE-MODE Used */
   DSL_uint16_t AeleMode;
   /** Kl0 Estimate per Band VTU-R */
   DSL_uint16_t Kl0EstimRPb[4];
   /** Final Kl0 per Band VTU-O */
   DSL_uint16_t Kl0EstimOPb[4];
   /** UPBOELMT */
   DSL_uint16_t UpboElmt;
   /** RXTHRSHDS */
   DSL_int16_t RxThreshDs;
#else
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
   /** AELE-MODE Used */
   DSL_uint16_t AeleMode;
   /** Kl0 Estimate per Band VTU-R */
   DSL_uint16_t Kl0EstimRPb[4];
   /** Final Kl0 per Band VTU-O */
   DSL_uint16_t Kl0EstimOPb[4];
   /** UPBOELMT */
   DSL_uint16_t UpboElmt;
   /** RXTHRSHDS */
   DSL_int16_t RxThreshDs;
#endif
} __PACKED__ ;


/**
   Configuration of a target noise margin delta, which is added to the target
   noise margin value configured at the CO (and in case of the CPE received from
   there). The resulting value is then taken as target noise margin, e.g. for
   bit loading.
*/
struct CMD_NoiseMarginDeltaSet
{
#if DSL_BYTE_ORDER == DSL_BIG_ENDIAN
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
   /** Target Noise Margin Delta DS */
   DSL_int16_t deltaTARSNRMds;
   /** Reserved (VDSL only) */
   DSL_uint16_t Res0 : 14;
   /** Reserved (VDSL only) */
   DSL_uint16_t Res1 : 1;
   /** Reserved (VDSL only) */
   DSL_uint16_t Res2 : 1;
#else
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
   /** Target Noise Margin Delta DS */
   DSL_int16_t deltaTARSNRMds;
   /** Reserved (VDSL only) */
   DSL_uint16_t Res2 : 1;
   /** Reserved (VDSL only) */
   DSL_uint16_t Res1 : 1;
   /** Reserved (VDSL only) */
   DSL_uint16_t Res0 : 14;
#endif
} __PACKED__ ;


/**
   Acknowledgement for the message CMD_NoiseMarginDeltaSet.
*/
struct ACK_NoiseMarginDeltaSet
{
#if DSL_BYTE_ORDER == DSL_BIG_ENDIAN
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
#else
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
#endif
} __PACKED__ ;


/**
   Sets parameters for downstream bearer channel 0 (Chapters 7.3.2.1-5 of
   G.997.1 [11]). Bearer channel configuration at the CPE is optional and just
   used to further limit the parameters usually configured at the CO.The mode,
   ATM or PTM, is selected with the configuration controls "ATMControl" and
   "PTMControl" in Parameter 2. Exactly one of them must be enabled, not both!
*/
struct CMD_BearerCh0_DS_Set
{
#if DSL_BYTE_ORDER == DSL_BIG_ENDIAN
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
   /** Reserved */
   DSL_uint16_t Res0 : 13;
   /** Reserved (STM) */
   DSL_uint16_t Res1 : 1;
   /** ATM Configuration Control */
   DSL_uint16_t ATMControl : 1;
   /** PTM Configuration Control */
   DSL_uint16_t PTMControl : 1;
   /** Reserved */
   DSL_uint16_t Res2;
   /** Minimum Data Rate BC0 PTM DS, LSW */
   DSL_uint16_t minRate0_PTMds_LSW;
   /** Minimum Data Rate BC0 PTM DS, MSW */
   DSL_uint16_t minRate0_PTMds_MSW;
   /** Maximum Data Rate BC0 PTM DS, LSW */
   DSL_uint16_t maxRate0_PTMds_LSW;
   /** Maximum Data Rate BC0 PTM DS, MSW */
   DSL_uint16_t maxRate0_PTMds_MSW;
   /** Minimum Reserved Data Rate BC0 PTM DS, LSW */
   DSL_uint16_t minResRate0_PTMds_LSW;
   /** Minimum Reserved Data Rate BC0 PTM DS, MSW */
   DSL_uint16_t minResRate0_PTMds_MSW;
   /** Reserved */
   DSL_uint8_t Res3;
   /** Maximum Interleaving Delay BC0 PTM DS */
   DSL_uint8_t maxDelay0_PTMds;
   /** Reserved */
   DSL_uint16_t Res4 : 4;
   /** OPTIONAL CIPOLICY 2, BC0 PTM DS */
   DSL_uint16_t CIPolicy2_PTMds : 1;
   /** OPTIONAL CIPOLICY 1, BC0 PTM DS */
   DSL_uint16_t CIPolicy1_PTMds : 1;
   /** "SHORT PACKETS" OPTION BC0 PTM DS */
   DSL_uint16_t ShortPacket_PTMds : 1;
   /** "PRE-EMPTION" OPTION BC0 PTM DS */
   DSL_uint16_t Preempt_PTMds : 1;
   /** Reserved */
   DSL_uint16_t Res5 : 6;
   /** Maximum BER BC0 PTM DS */
   DSL_uint16_t BER_PTMds : 2;
   /** Reserved */
   DSL_uint16_t Res6 : 4;
   /** Minimum INP BC0 PTM DS */
   DSL_uint16_t minINP_PTMds : 12;
   /** Reserved */
   DSL_uint16_t Res7;
   /** Minimum Data Rate BC0 ATM DS, LSW */
   DSL_uint16_t minRate0_ATMds_LSW;
   /** Minimum Data Rate BC0 ATM DS, MSW */
   DSL_uint16_t minRate0_ATMds_MSW;
   /** Maximum Data Rate BC0 ATM DS, LSW */
   DSL_uint16_t maxRate0_ATMds_LSW;
   /** Maximum Data Rate BC0 ATM DS, MSW */
   DSL_uint16_t maxRate0_ATMds_MSW;
   /** Minimum Reserved Data Rate BC0 ATM DS, LSW */
   DSL_uint16_t minResRate0_ATMds_LSW;
   /** Minimum Reserved Data Rate BC0 ATM DS, MSW */
   DSL_uint16_t minResRate0_ATMds_MSW;
   /** Reserved */
   DSL_uint8_t Res8;
   /** Maximum Interleaving Delay BC0 ATM DS */
   DSL_uint8_t maxDelay0_ATMds;
   /** Reserved */
   DSL_uint16_t Res9 : 4;
   /** OPTIONAL CIPOLICY 2, BC0 ATM DS */
   DSL_uint16_t CIPolicy2_ATMds : 1;
   /** OPTIONAL CIPOLICY 1, BC0 ATM DS */
   DSL_uint16_t CIPolicy1_ATMds : 1;
   /** Reserved */
   DSL_uint16_t Res10 : 8;
   /** Maximum BER BC0 ATM DS */
   DSL_uint16_t BER_ATMds : 2;
   /** Reserved */
   DSL_uint16_t Res11 : 4;
   /** Minimum INP BC0 ATM DS */
   DSL_uint16_t minINP_ATMds : 12;
#else
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
   /** PTM Configuration Control */
   DSL_uint16_t PTMControl : 1;
   /** ATM Configuration Control */
   DSL_uint16_t ATMControl : 1;
   /** Reserved (STM) */
   DSL_uint16_t Res1 : 1;
   /** Reserved */
   DSL_uint16_t Res0 : 13;
   /** Reserved */
   DSL_uint16_t Res2;
   /** Minimum Data Rate BC0 PTM DS, LSW */
   DSL_uint16_t minRate0_PTMds_LSW;
   /** Minimum Data Rate BC0 PTM DS, MSW */
   DSL_uint16_t minRate0_PTMds_MSW;
   /** Maximum Data Rate BC0 PTM DS, LSW */
   DSL_uint16_t maxRate0_PTMds_LSW;
   /** Maximum Data Rate BC0 PTM DS, MSW */
   DSL_uint16_t maxRate0_PTMds_MSW;
   /** Minimum Reserved Data Rate BC0 PTM DS, LSW */
   DSL_uint16_t minResRate0_PTMds_LSW;
   /** Minimum Reserved Data Rate BC0 PTM DS, MSW */
   DSL_uint16_t minResRate0_PTMds_MSW;
   /** Maximum Interleaving Delay BC0 PTM DS */
   DSL_uint8_t maxDelay0_PTMds;
   /** Reserved */
   DSL_uint8_t Res3;
   /** Maximum BER BC0 PTM DS */
   DSL_uint16_t BER_PTMds : 2;
   /** Reserved */
   DSL_uint16_t Res5 : 6;
   /** "PRE-EMPTION" OPTION BC0 PTM DS */
   DSL_uint16_t Preempt_PTMds : 1;
   /** "SHORT PACKETS" OPTION BC0 PTM DS */
   DSL_uint16_t ShortPacket_PTMds : 1;
   /** OPTIONAL CIPOLICY 1, BC0 PTM DS */
   DSL_uint16_t CIPolicy1_PTMds : 1;
   /** OPTIONAL CIPOLICY 2, BC0 PTM DS */
   DSL_uint16_t CIPolicy2_PTMds : 1;
   /** Reserved */
   DSL_uint16_t Res4 : 4;
   /** Minimum INP BC0 PTM DS */
   DSL_uint16_t minINP_PTMds : 12;
   /** Reserved */
   DSL_uint16_t Res6 : 4;
   /** Reserved */
   DSL_uint16_t Res7;
   /** Minimum Data Rate BC0 ATM DS, LSW */
   DSL_uint16_t minRate0_ATMds_LSW;
   /** Minimum Data Rate BC0 ATM DS, MSW */
   DSL_uint16_t minRate0_ATMds_MSW;
   /** Maximum Data Rate BC0 ATM DS, LSW */
   DSL_uint16_t maxRate0_ATMds_LSW;
   /** Maximum Data Rate BC0 ATM DS, MSW */
   DSL_uint16_t maxRate0_ATMds_MSW;
   /** Minimum Reserved Data Rate BC0 ATM DS, LSW */
   DSL_uint16_t minResRate0_ATMds_LSW;
   /** Minimum Reserved Data Rate BC0 ATM DS, MSW */
   DSL_uint16_t minResRate0_ATMds_MSW;
   /** Maximum Interleaving Delay BC0 ATM DS */
   DSL_uint8_t maxDelay0_ATMds;
   /** Reserved */
   DSL_uint8_t Res8;
   /** Maximum BER BC0 ATM DS */
   DSL_uint16_t BER_ATMds : 2;
   /** Reserved */
   DSL_uint16_t Res10 : 8;
   /** OPTIONAL CIPOLICY 1, BC0 ATM DS */
   DSL_uint16_t CIPolicy1_ATMds : 1;
   /** OPTIONAL CIPOLICY 2, BC0 ATM DS */
   DSL_uint16_t CIPolicy2_ATMds : 1;
   /** Reserved */
   DSL_uint16_t Res9 : 4;
   /** Minimum INP BC0 ATM DS */
   DSL_uint16_t minINP_ATMds : 12;
   /** Reserved */
   DSL_uint16_t Res11 : 4;
#endif
} __PACKED__ ;


/**
   Acknowledgement for CMD_BearerCh0_DS_Set. (Configuration of bearer channel
   0).
*/
struct ACK_BearerCh0_DS_Set
{
#if DSL_BYTE_ORDER == DSL_BIG_ENDIAN
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
#else
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
#endif
} __PACKED__ ;


/**
   Sets parameters for upstream bearer channel 0 (Chapters 7.3.2.1-5 of G.997.1
   [11]). Bearer channel configuration at the CPE is optional and just used to
   further limit the parameters usually configured at the CO.The mode, ATM or
   PTM, is selected with the configuration controls "ATMControl" and
   "PTMControl" in Parameter 2. Exactly one of them must be enabled, not both!
*/
struct CMD_BearerCh0_US_Set
{
#if DSL_BYTE_ORDER == DSL_BIG_ENDIAN
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
   /** Reserved */
   DSL_uint16_t Res0 : 13;
   /** Reserved (STM) */
   DSL_uint16_t Res1 : 1;
   /** ATM Configuration Control */
   DSL_uint16_t ATMControl : 1;
   /** PTM Configuration Control */
   DSL_uint16_t PTMControl : 1;
   /** Reserved */
   DSL_uint16_t Res2;
   /** Minimum Data Rate BC0 PTM US, LSW */
   DSL_uint16_t minRate0_PTMus_LSW;
   /** Minimum Data Rate BC0 PTM US, MSW */
   DSL_uint16_t minRate0_PTMus_MSW;
   /** Maximum Data Rate BC0 PTM US, LSW */
   DSL_uint16_t maxRate0_PTMus_LSW;
   /** Maximum Data Rate BC0 PTM US, MSW */
   DSL_uint16_t maxRate0_PTMus_MSW;
   /** Minimum Reserved Data Rate BC0 PTM US, LSW */
   DSL_uint16_t minResRate0_PTMus_LSW;
   /** Minimum Reserved Data Rate BC0 PTM US, MSW */
   DSL_uint16_t minResRate0_PTMus_MSW;
   /** Reserved */
   DSL_uint8_t Res3;
   /** Maximum Interleaving Delay BC0 PTM US */
   DSL_uint8_t maxDelay0_PTMus;
   /** Reserved */
   DSL_uint16_t Res4 : 4;
   /** OPTIONAL CIPOLICY 2, BC0 PTM US */
   DSL_uint16_t CIPolicy2_PTMus : 1;
   /** OPTIONAL CIPOLICY 1, BC0 PTM US */
   DSL_uint16_t CIPolicy1_PTMus : 1;
   /** "SHORT PACKETS" OPTION BC0 PTM US */
   DSL_uint16_t ShortPacket_PTMus : 1;
   /** "PRE-EMPTION" OPTION BC0 PTM US */
   DSL_uint16_t Preempt_PTMus : 1;
   /** Reserved */
   DSL_uint16_t Res5 : 6;
   /** Maximum BER BC0 PTM US */
   DSL_uint16_t BER_PTMus : 2;
   /** Reserved */
   DSL_uint16_t Res6 : 4;
   /** Minimum INP BC0 PTM US */
   DSL_uint16_t minINP_PTMus : 12;
   /** Reserved */
   DSL_uint16_t Res7;
   /** Minimum Data Rate BC0 ATM US, LSW */
   DSL_uint16_t minRate0_ATMus_LSW;
   /** Minimum Data Rate BC0 ATM US, MSW */
   DSL_uint16_t minRate0_ATMus_MSW;
   /** Maximum Data Rate BC0 ATM US, LSW */
   DSL_uint16_t maxRate0_ATMus_LSW;
   /** Maximum Data Rate BC0 ATM US, MSW */
   DSL_uint16_t maxRate0_ATMus_MSW;
   /** Minimum Reserved Data Rate BC0 ATM US, LSW */
   DSL_uint16_t minResRate0_ATMus_LSW;
   /** Minimum Reserved Data Rate BC0 ATM US, MSW */
   DSL_uint16_t minResRate0_ATMus_MSW;
   /** Reserved */
   DSL_uint8_t Res8;
   /** Maximum Interleaving Delay BC0 ATM US */
   DSL_uint8_t maxDelay0_ATMus;
   /** Reserved */
   DSL_uint16_t Res9 : 4;
   /** OPTIONAL CIPOLICY 1, BC0 ATM US */
   DSL_uint16_t CIPolicy2_ATMus : 1;
   /** OPTIONAL CIPOLICY 2, BC0 ATM US */
   DSL_uint16_t CIPolicy1_ATMus : 1;
   /** Reserved */
   DSL_uint16_t Res10 : 8;
   /** Maximum BER BC0 ATM US */
   DSL_uint16_t BER_ATMus : 2;
   /** Reserved */
   DSL_uint16_t Res11 : 4;
   /** Minimum INP BC0 ATM US */
   DSL_uint16_t minINP_ATMus : 12;
#else
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
   /** PTM Configuration Control */
   DSL_uint16_t PTMControl : 1;
   /** ATM Configuration Control */
   DSL_uint16_t ATMControl : 1;
   /** Reserved (STM) */
   DSL_uint16_t Res1 : 1;
   /** Reserved */
   DSL_uint16_t Res0 : 13;
   /** Reserved */
   DSL_uint16_t Res2;
   /** Minimum Data Rate BC0 PTM US, LSW */
   DSL_uint16_t minRate0_PTMus_LSW;
   /** Minimum Data Rate BC0 PTM US, MSW */
   DSL_uint16_t minRate0_PTMus_MSW;
   /** Maximum Data Rate BC0 PTM US, LSW */
   DSL_uint16_t maxRate0_PTMus_LSW;
   /** Maximum Data Rate BC0 PTM US, MSW */
   DSL_uint16_t maxRate0_PTMus_MSW;
   /** Minimum Reserved Data Rate BC0 PTM US, LSW */
   DSL_uint16_t minResRate0_PTMus_LSW;
   /** Minimum Reserved Data Rate BC0 PTM US, MSW */
   DSL_uint16_t minResRate0_PTMus_MSW;
   /** Maximum Interleaving Delay BC0 PTM US */
   DSL_uint8_t maxDelay0_PTMus;
   /** Reserved */
   DSL_uint8_t Res3;
   /** Maximum BER BC0 PTM US */
   DSL_uint16_t BER_PTMus : 2;
   /** Reserved */
   DSL_uint16_t Res5 : 6;
   /** "PRE-EMPTION" OPTION BC0 PTM US */
   DSL_uint16_t Preempt_PTMus : 1;
   /** "SHORT PACKETS" OPTION BC0 PTM US */
   DSL_uint16_t ShortPacket_PTMus : 1;
   /** OPTIONAL CIPOLICY 1, BC0 PTM US */
   DSL_uint16_t CIPolicy1_PTMus : 1;
   /** OPTIONAL CIPOLICY 2, BC0 PTM US */
   DSL_uint16_t CIPolicy2_PTMus : 1;
   /** Reserved */
   DSL_uint16_t Res4 : 4;
   /** Minimum INP BC0 PTM US */
   DSL_uint16_t minINP_PTMus : 12;
   /** Reserved */
   DSL_uint16_t Res6 : 4;
   /** Reserved */
   DSL_uint16_t Res7;
   /** Minimum Data Rate BC0 ATM US, LSW */
   DSL_uint16_t minRate0_ATMus_LSW;
   /** Minimum Data Rate BC0 ATM US, MSW */
   DSL_uint16_t minRate0_ATMus_MSW;
   /** Maximum Data Rate BC0 ATM US, LSW */
   DSL_uint16_t maxRate0_ATMus_LSW;
   /** Maximum Data Rate BC0 ATM US, MSW */
   DSL_uint16_t maxRate0_ATMus_MSW;
   /** Minimum Reserved Data Rate BC0 ATM US, LSW */
   DSL_uint16_t minResRate0_ATMus_LSW;
   /** Minimum Reserved Data Rate BC0 ATM US, MSW */
   DSL_uint16_t minResRate0_ATMus_MSW;
   /** Maximum Interleaving Delay BC0 ATM US */
   DSL_uint8_t maxDelay0_ATMus;
   /** Reserved */
   DSL_uint8_t Res8;
   /** Maximum BER BC0 ATM US */
   DSL_uint16_t BER_ATMus : 2;
   /** Reserved */
   DSL_uint16_t Res10 : 8;
   /** OPTIONAL CIPOLICY 2, BC0 ATM US */
   DSL_uint16_t CIPolicy1_ATMus : 1;
   /** OPTIONAL CIPOLICY 1, BC0 ATM US */
   DSL_uint16_t CIPolicy2_ATMus : 1;
   /** Reserved */
   DSL_uint16_t Res9 : 4;
   /** Minimum INP BC0 ATM US */
   DSL_uint16_t minINP_ATMus : 12;
   /** Reserved */
   DSL_uint16_t Res11 : 4;
#endif
} __PACKED__ ;


/**
   Acknowledgement for CMD_BearerCh0_US_Set. (Configuration of bearer channel
   0).
*/
struct ACK_BearerCh0_US_Set
{
#if DSL_BYTE_ORDER == DSL_BIG_ENDIAN
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
#else
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
#endif
} __PACKED__ ;


/**
   The message selects the channel initialization policy (CIPOLICY) to be
   applied for the tranceiver configuration. (Section 7.3.2.10 of G.997.1)In
   ADSL mode, the message is applicable only for ADSL2/2+ (Section 7.10.3 of
   G.992.3 Amd 3). The standard defines the parameter for the CO only. Here, the
   message is applied at the ADSL-CPE, then it overrides the policy given by the
   CO!
*/
struct CMD_InitPolicySet
{
#if DSL_BYTE_ORDER == DSL_BIG_ENDIAN
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
   /** Reserved */
   DSL_uint16_t Res0 : 14;
   /** Reserved for Init Policy BC1 */
   DSL_uint16_t Res1 : 1;
   /** Init Policy BC0 */
   DSL_uint16_t policy0 : 1;
#else
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
   /** Init Policy BC0 */
   DSL_uint16_t policy0 : 1;
   /** Reserved for Init Policy BC1 */
   DSL_uint16_t Res1 : 1;
   /** Reserved */
   DSL_uint16_t Res0 : 14;
#endif
} __PACKED__ ;


/**
   Acknowledgment for message CMD_InitPolicySet.
*/
struct ACK_InitPolicySet
{
#if DSL_BYTE_ORDER == DSL_BIG_ENDIAN
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
#else
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
#endif
} __PACKED__ ;


/**
   Configuration of options for the modem state machine.
*/
struct CMD_ErasureControlSet
{
#if DSL_BYTE_ORDER == DSL_BIG_ENDIAN
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
   /** Reserved */
   DSL_uint16_t Res0 : 13;
   /** "Trust-Me"-Bit, Bit 2 (ADSL only) */
   DSL_uint16_t trustMe : 1;
   /** Erasure Policy, Bit 1 */
   DSL_uint16_t erasurePolicy : 1;
   /** Reserved, Bit0 */
   DSL_uint16_t Res1 : 1;
#else
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
   /** Reserved, Bit0 */
   DSL_uint16_t Res1 : 1;
   /** Erasure Policy, Bit 1 */
   DSL_uint16_t erasurePolicy : 1;
   /** "Trust-Me"-Bit, Bit 2 (ADSL only) */
   DSL_uint16_t trustMe : 1;
   /** Reserved */
   DSL_uint16_t Res0 : 13;
#endif
} __PACKED__ ;


/**
   Acknowledgement to CMD_ErasureControlSet.
*/
struct ACK_ErasureControlSet
{
#if DSL_BYTE_ORDER == DSL_BIG_ENDIAN
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
#else
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
#endif
} __PACKED__ ;


/**
   Performs some miscellaneous chip set configurations.
*/
struct CMD_Misc_ConfigSet
{
#if DSL_BYTE_ORDER == DSL_BIG_ENDIAN
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
   /** Reserved */
   DSL_uint16_t Res0 : 7;
   /** TPS-TC Type Event Enable */
   DSL_uint16_t TcTypeEvt : 1;
   /** Hlog Invalidation */
   DSL_uint16_t HlogValidation : 1;
   /** Short CLR for ADSL2+ */
   DSL_uint16_t ShortClrA2p : 1;
   /** Short CLR for ADSL2 */
   DSL_uint16_t ShortClrA2 : 1;
   /** Short CLR for ADSL1 */
   DSL_uint16_t ShortClrA1 : 1;
   /** INP Protection Improvement (VDSL only) */
   DSL_uint16_t FrameParams : 1;
   /** Reserved */
   DSL_uint16_t Res1 : 1;
   /** Activation Start Mode (ADSL only) */
   DSL_uint16_t StartMode : 1;
   /** Non-standard GHS-ANSI Activation (ADSL only) */
   DSL_uint16_t GhsAnsiSeq : 1;
#else
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
   /** Non-standard GHS-ANSI Activation (ADSL only) */
   DSL_uint16_t GhsAnsiSeq : 1;
   /** Activation Start Mode (ADSL only) */
   DSL_uint16_t StartMode : 1;
   /** Reserved */
   DSL_uint16_t Res1 : 1;
   /** INP Protection Improvement (VDSL only) */
   DSL_uint16_t FrameParams : 1;
   /** Short CLR for ADSL1 */
   DSL_uint16_t ShortClrA1 : 1;
   /** Short CLR for ADSL2 */
   DSL_uint16_t ShortClrA2 : 1;
   /** Short CLR for ADSL2+ */
   DSL_uint16_t ShortClrA2p : 1;
   /** Hlog Invalidation */
   DSL_uint16_t HlogValidation : 1;
   /** TPS-TC Type Event Enable */
   DSL_uint16_t TcTypeEvt : 1;
   /** Reserved */
   DSL_uint16_t Res0 : 7;
#endif
} __PACKED__ ;


/**
   This is the acknowledgement for ACK_Misc_ConfigSet.
*/
struct ACK_Misc_ConfigSet
{
#if DSL_BYTE_ORDER == DSL_BIG_ENDIAN
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
#else
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
#endif
} __PACKED__ ;


/**
   Performs some miscellaneous chip set configurations.
*/
struct CMD_OperationOptionsSet
{
#if DSL_BYTE_ORDER == DSL_BIG_ENDIAN
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
   /** Reserved */
   DSL_uint16_t Res0 : 13;
   /** VRX_ENABLE, Fixed value */
   DSL_uint16_t Mfd : 1;
   /** Enhanced Upstream Framing (ADSL-Only) */
   DSL_uint16_t UsFramingExt : 1;
   /** NTR (currently VDSL-Only) */
   DSL_uint16_t Ntr : 1;
#else
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
   /** NTR (currently VDSL-Only) */
   DSL_uint16_t Ntr : 1;
   /** Enhanced Upstream Framing (ADSL-Only) */
   DSL_uint16_t UsFramingExt : 1;
   /** VRX_ENABLE, Fixed value */
   DSL_uint16_t Mfd : 1;
   /** Reserved */
   DSL_uint16_t Res0 : 13;
#endif
} __PACKED__ ;


/**
   This is the acknowledgement for CMD_OperationOptionsSet.
*/
struct ACK_OperationOptionsSet
{
#if DSL_BYTE_ORDER == DSL_BIG_ENDIAN
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
#else
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
#endif
} __PACKED__ ;


/**
   The message selects a DSL operator. The information is used to configure
   operator specific settings inside the DSL firmware.
*/
struct CMD_OperatorSelect
{
#if DSL_BYTE_ORDER == DSL_BIG_ENDIAN
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
   /** DSL Operator Selection */
   DSL_uint16_t DslOperator;
#else
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
   /** DSL Operator Selection */
   DSL_uint16_t DslOperator;
#endif
} __PACKED__ ;


/**
   This is the acknowledgement for ACK_OperatorSelect.
*/
struct ACK_OperatorSelect
{
#if DSL_BYTE_ORDER == DSL_BIG_ENDIAN
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
#else
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
#endif
} __PACKED__ ;


/**
   The message reads back the selected DSL operator, as selected by
   ACK_OperatorSelect.
*/
struct CMD_OperatorSelectionGet
{
#if DSL_BYTE_ORDER == DSL_BIG_ENDIAN
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
#else
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
#endif
} __PACKED__ ;


/**
   The message provides the selected DSL operator.
*/
struct ACK_OperatorSelectionGet
{
#if DSL_BYTE_ORDER == DSL_BIG_ENDIAN
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
   /** Selected DSL Operator */
   DSL_uint16_t DslOperator;
#else
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
   /** Selected DSL Operator */
   DSL_uint16_t DslOperator;
#endif
} __PACKED__ ;


/**
   The messages configures settings for test modes.
*/
struct CMD_TestOptionsSet
{
#if DSL_BYTE_ORDER == DSL_BIG_ENDIAN
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
   /** Reserved */
   DSL_uint16_t Res0 : 11;
   /** ATTNDR Improved Method Support (VDSL only) */
   DSL_uint16_t AttndrImproved : 1;
   /** Reserved */
   DSL_uint16_t Res1 : 2;
   /** US Finegain Control */
   DSL_uint16_t FgainControlUs : 1;
   /** DS Finegain Control */
   DSL_uint16_t FgainControlDs : 1;
#else
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
   /** DS Finegain Control */
   DSL_uint16_t FgainControlDs : 1;
   /** US Finegain Control */
   DSL_uint16_t FgainControlUs : 1;
   /** Reserved */
   DSL_uint16_t Res1 : 2;
   /** ATTNDR Improved Method Support (VDSL only) */
   DSL_uint16_t AttndrImproved : 1;
   /** Reserved */
   DSL_uint16_t Res0 : 11;
#endif
} __PACKED__ ;


/**
   This is the acknowledgement for CMD_TestOptionsSet.
*/
struct ACK_TestOptionsSet
{
#if DSL_BYTE_ORDER == DSL_BIG_ENDIAN
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
#else
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
#endif
} __PACKED__ ;


/**
   Sets the PPE clock.
*/
struct CMD_ClockSet
{
#if DSL_BYTE_ORDER == DSL_BIG_ENDIAN
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
   /** Reserved */
   DSL_uint16_t Res0 : 11;
   /** Clock Change Trigger */
   DSL_uint16_t ppeClkSet : 1;
   /** Reserved */
   DSL_uint16_t Res1 : 2;
   /** PPE Clock */
   DSL_uint16_t ppeClock : 2;
#else
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
   /** PPE Clock */
   DSL_uint16_t ppeClock : 2;
   /** Reserved */
   DSL_uint16_t Res1 : 2;
   /** Clock Change Trigger */
   DSL_uint16_t ppeClkSet : 1;
   /** Reserved */
   DSL_uint16_t Res0 : 11;
#endif
} __PACKED__ ;


/**
   This is the acknowledgement for CMD_ClockSet.
*/
struct ACK_ClockSet
{
#if DSL_BYTE_ORDER == DSL_BIG_ENDIAN
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
#else
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
#endif
} __PACKED__ ;


/**
   Sets an offset for the PLL frequency compared to the crystals rated nominal
   frequency of 36 MHz. This allows e.g. to finetune the handshake tone
   frequencies to exactly match the standard values.
*/
struct CMD_PLL_ClockSet
{
#if DSL_BYTE_ORDER == DSL_BIG_ENDIAN
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
   /** PLL Clock Offset in ppm */
   DSL_int16_t pllClockOffset;
#else
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
   /** PLL Clock Offset in ppm */
   DSL_int16_t pllClockOffset;
#endif
} __PACKED__ ;


/**
   This is the acknowledgement for CMD_PLL_ClockSet.
*/
struct ACK_PLL_ClockSet
{
#if DSL_BYTE_ORDER == DSL_BIG_ENDIAN
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
#else
   /** Index */
   DSL_uint16_t Index;
   /** Length */
   DSL_uint16_t Length;
#endif
} __PACKED__ ;




#ifdef __cplusplus
}
#endif

#ifdef __PACKED_DEFINED__
   #if !(defined (__GNUC__) || defined (__GNUG__))
      #pragma pack()
   #endif
   #undef __PACKED_DEFINED__
#endif /* __PACKED_DEFINED__ */

/** @} */

#endif /** _DRV_DSL_CPE_VRX_MSG_CONFIG_PMD_H_*/
