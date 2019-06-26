#ifndef _DTI_CLI_H
#define _DTI_CLI_H
/******************************************************************************

                              Copyright (c) 2009
                            Lantiq Deutschland GmbH
                     Am Campeon 3; 85579 Neubiberg, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/** \file
   Debug and Trace Interface - Command Line Interface (CLI) definitions and declarations.
*/

#ifdef __cplusplus
   extern "C" {
#endif


/* ============================================================================
   Includes
   ========================================================================= */
#include "ifx_types.h"
#include "ifxos_lock.h"
#include "ifx_fifo.h"

#include "ifx_dti_protocol.h"
#include "ifx_dti_protocol_cli.h"
#include "dti_cli_interface.h"

#include "dti_protocol_ext.h"
#include "dti_agent_interface.h"

/* ============================================================================
   Defines
   ========================================================================= */

#if defined(INCLUDE_CLI_SUPPORT) && (INCLUDE_CLI_SUPPORT == 1)
#  ifndef DTI_CLI_EVENT_SUPPORT
#     define DTI_CLI_EVENT_SUPPORT           1
#  endif
#else
#  ifdef DTI_CLI_EVENT_SUPPORT
#     undef DTI_CLI_EVENT_SUPPORT
#  endif
#  define DTI_CLI_EVENT_SUPPORT              0
#endif

#define DTI_CLI_MAX_CLI_INTERFACES           4
#define DTI_CLI_EVENT_MAX_BUFFER             0x10000

#define DTI_CLI_TRACE_CMD_WR_SEND_FLAG       0x01
#define DTI_CLI_TRACE_CMD_RD_SEND_FLAG       0x02

#define DTI_CLI_TRACE_CMD_SEND_FLAGS         (DTI_CLI_TRACE_CMD_WR_SEND_FLAG | DTI_CLI_TRACE_CMD_RD_SEND_FLAG)


/* ============================================================================
   CLI Control Defines
   ========================================================================= */

/* CLI Control, CLI Group:
   size of the CLI buffer: <0/1/2> (Send / Recv / Event) */
#define DTI_CLI_CNTRL_GR_CLI_BUFSIZE_NUM        0
#define DTI_CLI_CNTRL_GR_CLI_BUFSIZE_NAME       "bufSize"
#define DTI_CLI_CNTRL_GR_CLI_BUFSIZE_HELP       "<IFNum> <0/1/2>  (show tx/rx/evt buffer size)"

#define DTI_CLI_CNTRL_GR_CLI_BUFSIZE            { DTI_CLI_CNTRL_GR_CLI_BUFSIZE_NUM, \
                                                  DTI_CLI_CNTRL_GR_CLI_BUFSIZE_NAME, \
                                                  DTI_CLI_CNTRL_GR_CLI_BUFSIZE_HELP}

/* CLI Control, CLI Group:
   create event handling: <evt buffersize> <max evt size> */
#define DTI_CLI_CNTRL_GR_CLI_EVT_CREATE_NUM     1
#define DTI_CLI_CNTRL_GR_CLI_EVT_CREATE_NAME    "evtCreate"
#define DTI_CLI_CNTRL_GR_CLI_EVT_CREATE_HELP    "<evt buffersize> <max evt size>  (create event handling)"

#define DTI_CLI_CNTRL_GR_CLI_EVT_CREATE         { DTI_CLI_CNTRL_GR_CLI_EVT_CREATE_NUM, \
                                                  DTI_CLI_CNTRL_GR_CLI_EVT_CREATE_NAME, \
                                                  DTI_CLI_CNTRL_GR_CLI_EVT_CREATE_HELP}

/* CLI Control, CLI Group:
   close event handling: <evt buffersize> <max evt size> */
#define DTI_CLI_CNTRL_GR_CLI_EVT_CLOSE_NUM      2
#define DTI_CLI_CNTRL_GR_CLI_EVT_CLOSE_NAME     "evtClose"
#define DTI_CLI_CNTRL_GR_CLI_EVT_CLOSE_HELP     "<none>  (close event handling)"

#define DTI_CLI_CNTRL_GR_CLI_EVT_CLOSE          { DTI_CLI_CNTRL_GR_CLI_EVT_CLOSE_NUM, \
                                                  DTI_CLI_CNTRL_GR_CLI_EVT_CLOSE_NAME, \
                                                  DTI_CLI_CNTRL_GR_CLI_EVT_CLOSE_HELP}


/* CLI Control, CLI Group:
   event control: <IF Num> <0/1> (ON / OFF) */
#define DTI_CLI_CNTRL_GR_CLI_EVENT_NUM          3
#define DTI_CLI_CNTRL_GR_CLI_EVENT_NAME         "event"
#define DTI_CLI_CNTRL_GR_CLI_EVENT_HELP         "<IFNum> <0/1>  (switch events ON/OFF)"

#define DTI_CLI_CNTRL_GR_CLI_EVENT              { DTI_CLI_CNTRL_GR_CLI_EVENT_NUM, \
                                                  DTI_CLI_CNTRL_GR_CLI_EVENT_NAME, \
                                                  DTI_CLI_CNTRL_GR_CLI_EVENT_HELP}

/* CLI Control, CLI Group:
enable CLI Command Msg Log: <IF Num> <0/1> (ON / OFF) */
#define DTI_CLI_CNTRL_GR_CLI_TR_CMD_NUM         4
#define DTI_CLI_CNTRL_GR_CLI_TR_CMD_NAME        "traceCmd"
#define DTI_CLI_CNTRL_GR_CLI_TR_CMD_HELP        "<IFNum> <0/1>  (cmd msg log OFF/ON)"

#define DTI_CLI_CNTRL_GR_CLI_TR_CMD             { DTI_CLI_CNTRL_GR_CLI_TR_CMD_NUM, \
                                                  DTI_CLI_CNTRL_GR_CLI_TR_CMD_NAME, \
                                                  DTI_CLI_CNTRL_GR_CLI_TR_CMD_HELP}

/* CLI Control, CLI Group:
enable CLI Event Msg Log: <IF Num> <0/1> (ON / OFF) */
#define DTI_CLI_CNTRL_GR_CLI_TR_EVT_NUM         5
#define DTI_CLI_CNTRL_GR_CLI_TR_EVT_NAME        "traceEvt"
#define DTI_CLI_CNTRL_GR_CLI_TR_EVT_HELP        "<IFNum> <0/1>  (event msg log OFF/ON)"

#define DTI_CLI_CNTRL_GR_CLI_TR_EVT             { DTI_CLI_CNTRL_GR_CLI_TR_EVT_NUM, \
                                                  DTI_CLI_CNTRL_GR_CLI_TR_EVT_NAME, \
                                                  DTI_CLI_CNTRL_GR_CLI_TR_EVT_HELP}

/* CLI Control, CLI Group:
enable CLI Event Distribution: <IF Num> <0/1> (ON / OFF) */
#define DTI_CLI_CNTRL_GR_CLI_DIST_EVT_NUM         6
#define DTI_CLI_CNTRL_GR_CLI_DIST_EVT_NAME        "distEvent"
#define DTI_CLI_CNTRL_GR_CLI_DIST_EVT_HELP        "<IFNum> <0/1>  (event distribution OFF/ON)"

#define DTI_CLI_CNTRL_GR_CLI_DIST_EVT           { DTI_CLI_CNTRL_GR_CLI_DIST_EVT_NUM, \
                                                  DTI_CLI_CNTRL_GR_CLI_DIST_EVT_NAME, \
                                                  DTI_CLI_CNTRL_GR_CLI_DIST_EVT_HELP}

/* all CLI Control CLI Group commands */
#define DTI_CLI_CNTRL_GR_CLI_COMMANDS  \
            DTI_CLI_CNTRL_GR_CLI_BUFSIZE, \
            DTI_CLI_CNTRL_GR_CLI_EVT_CREATE, \
            DTI_CLI_CNTRL_GR_CLI_EVT_CLOSE, \
            DTI_CLI_CNTRL_GR_CLI_EVENT, \
            DTI_CLI_CNTRL_GR_CLI_TR_CMD, \
            DTI_CLI_CNTRL_GR_CLI_TR_EVT, \
            DTI_CLI_CNTRL_GR_CLI_DIST_EVT


/* ============================================================================
   Typedefs
   ========================================================================= */

/**
   CLI specific Control Struct for CLI Event Handling.
*/
typedef struct DTI_CliEventCtx_s
{
   /** set if auto msg handling is required */
   IFX_boolean_t        bAutoCliMsgActive;

   /** event FIFO */
   IFXOS_lock_t         eventFifoLock;
   /** points to the event Var FIFO */
   IFX_VFIFO            eventFifo;

   /** max size of a FIFO element */
   IFX_uint_t           maxEventSize_byte;
   /** size of the used FIFO buffer */
   IFX_uint_t           eventFifoBufferSize_byte;
   /** keeps the the used FIFO buffer */
   IFX_ulong_t          *pEventFifoBuffer;

} DTI_CliEventCtx_t;

/**
   Global Control struct for handling a Command Line Interface
*/
typedef struct DTI_cli_control_s
{
   /** protect CLI responce buffer */
   IFXOS_lock_t      cliResponceBufLock;

   /** interface is ready for CLI Send */
   IFX_boolean_t     bRdyForCliSend;

   /** enable / disable CLI command msg tracer (0x1: wr cmd's, 0x2: responce) */
   IFX_uint_t        bCmdMsgTrace;
   /** enable / disable CLI event msg tracer */
   IFX_boolean_t     bEvtMsgTrace;
   /** test: distribute as an event */
   IFX_boolean_t     bTestDistEvt;

   /** interface number */
   IFX_uint8_t       cliIfNum;
   /** registered user interface name */
   IFX_char_t        cliIfName[DTI_CLI_MAX_NAME_LEN];

   /** points to the used resonce buffer for the CLI Send command */
   IFX_uint8_t       *pCliResponceBuffer;
   /** length of the responce buffer */
   IFX_uint_t        responceBufferSize;

   /** callback context pointer */
   IFX_void_t                          *pCallbackContext;
   /** registered send function */
   DTI_CliSendCommandFunc_t             pSendFunct;
   /** registered send function with partial response support*/
   DTI_CliSendFragmentedCommandFunc_t   pSendFragFunct;

   /** current client context to return events */
   IFX_void_t                 *pEventClientContext;

   /** if set, the corresponding worker thread accepts events form this CLI */
   IFX_uint8_t       bAcceptEvents[DTI_MAX_NUM_OF_WORKER];

} DTI_cli_control_t;

/* ============================================================================
   Exports
   ========================================================================= */

IFXOS_PRN_USR_MODULE_DECL(DTI_CLI);

extern IFX_int_t DTI_CLI_configStrWrite(
                        DTI_AgentCtx_t          *pAgentCtx,
                        IFX_char_t              *pSysInfoBuffer,
                        IFX_int_t               bufferSize);

extern IFX_int_t DTI_packetHandler_cli(
                        DTI_AgentCtx_t          *pAgentCtx,
                        DTI_ProtocolServerCtx_t *pDtiProtServerCtx,
                        IFX_int_t               workerNum,
                        const DTI_Packet_t      *pDtiPacketIn,
                        IFX_uint_t              dtiBufInLen,
                        DTI_Packet_t            *pDtiPacketOut,
                        IFX_uint_t              dtiBufOutLen);

extern IFX_int_t DTI_CLI_workerEventCtxInit(
                        DTI_AgentCtx_t          *pAgentCtx,
                        IFX_int_t               workerNum);

extern IFX_int_t DTI_CLI_workerEventCtxDelete(
                        DTI_AgentCtx_t          *pAgentCtx,
                        IFX_int_t               workerNum);

extern IFX_int_t DTI_CLI_workerEventCreate(
                        DTI_AgentCtx_t          *pAgentCtx,
                        IFX_int_t               workerNum,
                        IFX_uint_t              eventBufferSize_byte,
                        IFX_uint_t              maxEventSize_byte);

extern IFX_int_t DTI_CLI_workerEventRelease(
                        DTI_AgentCtx_t          *pAgentCtx,
                        IFX_int_t               workerNum);

extern IFX_int_t DTI_CLI_workerEventControl(
                        DTI_AgentCtx_t          *pAgentCtx,
                        IFX_int_t               workerNum,
                        IFX_uint_t              cliIfNum,
                        IFX_boolean_t           bEnable);

extern IFX_int_t DTI_CLI_autoMsgProcess(
                        DTI_AgentCtx_t          *pAgentCtx,
                        IFX_int_t               workerNum,
                        DTI_ProtocolServerCtx_t *pDtiProtServerCtx);

extern IFX_int_t DTI_CLI_moduleControlInit(
                        DTI_AgentCtx_t          *pAgentCtx);

extern IFX_int_t DTI_CLI_moduleStop(
                        DTI_AgentCtx_t          *pAgentCtx);

extern IFX_int_t DTI_CLI_cliControlGrCliProcess(
                        DTI_AgentCtx_t          *pAgentCtx,
                        IFX_int_t               workerNum,
                        const IFX_char_t        *pArgs,
                        IFX_uint_t              argLen,
                        IFX_int_t               commandNum,
                        IFX_char_t              *pResponseBuffer,
                        IFX_uint32_t            responseBufferSize_byte);

#ifdef __cplusplus
}
#endif

#endif   /* #ifndef _DTI_CLI_H */


