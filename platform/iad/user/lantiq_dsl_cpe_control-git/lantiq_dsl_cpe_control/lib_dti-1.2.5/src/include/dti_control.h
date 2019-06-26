#ifndef _DTI_CONTROL_H
#define _DTI_CONTROL_H
/******************************************************************************

                              Copyright (c) 2009
                            Lantiq Deutschland GmbH
                     Am Campeon 3; 85579 Neubiberg, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/** \file
   DTI internal definitions and declarations.
*/

#ifdef __cplusplus
   extern "C" {
#endif


/* ==========================================================================
   includes
   ========================================================================== */
#include "ifx_types.h"
#include "ifxos_lock.h"
#include "ifxos_socket.h"
#include "ifxos_thread.h"

#include "ifx_dti_protocol.h"
#include "dti_protocol_interface.h"

#include "dti_device.h"
#include "dti_agent_interface.h"
#include "dti_statistic.h"

#include "dti_cli.h"

/* ==========================================================================
   Defines
   ========================================================================== */

/** defines maximum lenght of packets that can be queued by the DTI agent */
#define DTI_MAX_PACKET_QUEUE_SIZE                     1

/** wait timer for new accept requests */
#define DTI_LISTENER_ACCEPT_WAIT_MS                   1000

/** wait timer for listener thread reaction  */
#define DTI_LISTENER_THREAD_WAIT_MS                   DTI_LISTENER_ACCEPT_WAIT_MS

/** wait timer for worker thread reaction  */
#define DTI_WORKER_THREAD_WAIT_MS                     100

/** used stack size */
#ifndef DTI_LISTENER_THREAD_STACK_SIZE
#  define DTI_LISTENER_THREAD_STACK_SIZE              8192
#endif
#ifndef DTI_WORKER_THREAD_STACK_SIZE
#  define DTI_WORKER_THREAD_STACK_SIZE                8192
#endif

/** used thread prio */
#ifndef DTI_LISTENER_THREAD_PRIO
#  define DTI_LISTENER_THREAD_PRIO                    0
#endif
#ifndef DTI_WORKER_THREAD_PRIO
#  define DTI_WORKER_THREAD_PRIO                      0
#endif

/* max number of images which can be stored */
#define DTI_MAX_NUM_OF_IMAGES                         3
/** max chunk size used for image transfere */
#define DTI_MAX_IMAGE_CHUNK_SIZE                      2048
/** max image size */
#define DTI_MAX_IMAGE_SIZE                            2097152

/** DTI Worker Thread State - init */
#define DTI_WORKER_THREAD_RESUME_INIT                    0
/** DTI Worker Thread State - in use */
#define DTI_WORKER_THREAD_RESUME_IN_USE                  1
/** DTI Worker Thread State - initialized, ready for resume */
#define DTI_WORKER_THREAD_RESUME_FREE                    2

/* ============================================================================
   CLI Control - Defines and Types
   ========================================================================= */

#define DTI_CLI_CNTRL_NUM(ENTRY)                ENTRY##_NUM
#define DTI_CLI_CNTRL_NAME(ENTRY)               ENTRY##_NAME
#define DTI_CLI_CNTRL_HELP(ENTRY)               ENTRY##_HELP

/* CLI Control - list of groups */
#define DTI_CLI_CNTRL_GROUP_CONFIG_NUM          0
#define DTI_CLI_CNTRL_GROUP_CONFIG_NAME         "config"
#define DTI_CLI_CNTRL_GROUP_CONFIG_HELP         "basic config command"

#define DTI_CLI_CNTRL_GROUP_DEVICE_NUM          1
#define DTI_CLI_CNTRL_GROUP_DEVICE_NAME         "device"
#define DTI_CLI_CNTRL_GROUP_DEVICE_HELP         "device commands"

#define DTI_CLI_CNTRL_GROUP_CLI_NUM             2
#define DTI_CLI_CNTRL_GROUP_CLI_NAME            "cli"
#define DTI_CLI_CNTRL_GROUP_CLI_HELP            "cli commands"

#define DTI_CLI_CNTRL_GROUP_HELP_NUM            3
#define DTI_CLI_CNTRL_GROUP_HELP_NAME           "help"
#define DTI_CLI_CNTRL_GROUP_HELP_HELP           "show help"

#define DTI_CLI_CNTRL_GROUP_CONFIG              { DTI_CLI_CNTRL_GROUP_CONFIG_NUM, \
                                                  DTI_CLI_CNTRL_GROUP_CONFIG_NAME, \
                                                  DTI_CLI_CNTRL_GROUP_CONFIG_HELP}

#define DTI_CLI_CNTRL_GROUP_DEVICE              { DTI_CLI_CNTRL_GROUP_DEVICE_NUM, \
                                                  DTI_CLI_CNTRL_GROUP_DEVICE_NAME, \
                                                  DTI_CLI_CNTRL_GROUP_DEVICE_HELP}

#define DTI_CLI_CNTRL_GROUP_CLI                 { DTI_CLI_CNTRL_GROUP_CLI_NUM, \
                                                  DTI_CLI_CNTRL_GROUP_CLI_NAME, \
                                                  DTI_CLI_CNTRL_GROUP_CLI_HELP}

#define DTI_CLI_CNTRL_GROUP_HELP                { DTI_CLI_CNTRL_GROUP_HELP_NUM, \
                                                  DTI_CLI_CNTRL_GROUP_HELP_NAME, \
                                                  DTI_CLI_CNTRL_GROUP_HELP_HELP}

/* CLI Control, Config Group:
   set new debug level: <0..4> (LOW / NORMAL / HIGH / OFF) */
#define DTI_CLI_CNTRL_GR_CONFIG_DBGLEVEL_NUM    0
#define DTI_CLI_CNTRL_GR_CONFIG_DBGLEVEL_NAME   "dbglevel"
#define DTI_CLI_CNTRL_GR_CONFIG_DBGLEVEL_HELP   "<value>  (set new dbg level, range 1..4)"

#define DTI_CLI_CNTRL_GR_CONFIG_DBGLEVEL        { DTI_CLI_CNTRL_GR_CONFIG_DBGLEVEL_NUM, \
                                                  DTI_CLI_CNTRL_GR_CONFIG_DBGLEVEL_NAME, \
                                                  DTI_CLI_CNTRL_GR_CONFIG_DBGLEVEL_HELP}



/* CLI Control, Config Group:
   image handling: write to file | release | show */
#define DTI_CLI_CNTRL_GR_CONFIG_IMAGE_NUM       1
#define DTI_CLI_CNTRL_GR_CONFIG_IMAGE_NAME      "image"
#define DTI_CLI_CNTRL_GR_CONFIG_IMAGE_HELP      "<show|release|write> <id> [name]"

#define DTI_CLI_CNTRL_GR_CONFIG_IMAGE           { DTI_CLI_CNTRL_GR_CONFIG_IMAGE_NUM, \
                                                  DTI_CLI_CNTRL_GR_CONFIG_IMAGE_NAME, \
                                                  DTI_CLI_CNTRL_GR_CONFIG_IMAGE_HELP}



/* all CLI Control Config Group commands */
#define DTI_CLI_CNTRL_GR_CONFIG_COMMANDS \
            DTI_CLI_CNTRL_GR_CONFIG_DBGLEVEL, \
            DTI_CLI_CNTRL_GR_CONFIG_IMAGE



/* CLI Control, Config Group Image Handling:
   show image informations */
#define DTI_CLI_CNTRL_GR_CONFIG_IMAGE_SHOW_NUM        0
#define DTI_CLI_CNTRL_GR_CONFIG_IMAGE_SHOW_NAME       "show"
#define DTI_CLI_CNTRL_GR_CONFIG_IMAGE_SHOW_HELP       "<imageId> (shows the DTI image informations)"

#define DTI_CLI_CNTRL_GR_CONFIG_IMAGE_SHOW      { DTI_CLI_CNTRL_GR_CONFIG_IMAGE_SHOW_NUM, \
                                                  DTI_CLI_CNTRL_GR_CONFIG_IMAGE_SHOW_NAME, \
                                                  DTI_CLI_CNTRL_GR_CONFIG_IMAGE_SHOW_HELP}


/* CLI Control, Config Group Image Handling:
   release an image from the DTI context */
#define DTI_CLI_CNTRL_GR_CONFIG_IMAGE_RELEASE_NUM     1
#define DTI_CLI_CNTRL_GR_CONFIG_IMAGE_RELEASE_NAME    "release"
#define DTI_CLI_CNTRL_GR_CONFIG_IMAGE_RELEASE_HELP    "<imageId> (release the image from DTI context)"

#define DTI_CLI_CNTRL_GR_CONFIG_IMAGE_RELEASE   { DTI_CLI_CNTRL_GR_CONFIG_IMAGE_RELEASE_NUM, \
                                                  DTI_CLI_CNTRL_GR_CONFIG_IMAGE_RELEASE_NAME, \
                                                  DTI_CLI_CNTRL_GR_CONFIG_IMAGE_RELEASE_HELP}

/* CLI Control, Config Group Image Handling:
   write an image to the target file system */
#define DTI_CLI_CNTRL_GR_CONFIG_IMAGE_WRITE_NUM       2
#define DTI_CLI_CNTRL_GR_CONFIG_IMAGE_WRITE_NAME      "write"
#define DTI_CLI_CNTRL_GR_CONFIG_IMAGE_WRITE_HELP      "<imageId> (write an image to the target filesystem)"

#define DTI_CLI_CNTRL_GR_CONFIG_IMAGE_WRITE     { DTI_CLI_CNTRL_GR_CONFIG_IMAGE_WRITE_NUM, \
                                                  DTI_CLI_CNTRL_GR_CONFIG_IMAGE_WRITE_NAME, \
                                                  DTI_CLI_CNTRL_GR_CONFIG_IMAGE_WRITE_HELP}

/* all CLI Control Config Image commands */
#define DTI_CLI_CNTRL_GR_CONFIG_IMAGE_COMMANDS \
            DTI_CLI_CNTRL_GR_CONFIG_IMAGE_SHOW, \
            DTI_CLI_CNTRL_GR_CONFIG_IMAGE_RELEASE, \
            DTI_CLI_CNTRL_GR_CONFIG_IMAGE_WRITE





/* ==========================================================================
   Types
   ========================================================================== */

/*
   CLI Config - Keyword
*/
typedef struct
{
   IFX_uint8_t       keywordIdx;
   const IFX_char_t  *pKeywordName;
   const IFX_char_t  *pKeywordHelp;
} DTI_cliConfigKeyword_t;


/**
   Contains the device interface specific informations
*/
typedef struct DTI_deviceInterface_s
{
   /** TRUE if the device info is set */
   IFX_boolean_t           bConfigured;
   /** device configuration */
   DTI_DeviceSysInfo_t     deviceSysInfo;

   /** keeps the name the given interface handler */
   IFX_char_t              ifName[DTI_MAX_LEN_DEVICE_INTERFACE_NAME];

   /** device interface access functions */
   DTI_DeviceAccessFct_t   *pDeviceAccessFct;

} DTI_deviceInterface_t;

/**
   Context of a worker thread.
*/
typedef struct DTI_WorkerCtx_s
{
   /** number of current worker */
   IFX_int_t               workerNum;

   /** controls resume of a already running thread */
   IFX_int_t               thrResumeState;

   /** thread control of current worker */
   IFXOS_ThreadCtrl_t      thr;

   /** DTI protocol control and connection data (server side) */
   DTI_ProtocolServerCtx_t dtiProtocolServerCtx;

   /** Used Device interfaces within the worker environment */
   DTI_deviceInterface_t   *pUsedDevInterfaces[DTI_MAX_DEVICE_INTERFACES];

   /** Device context of current worker */
   DTI_DeviceCtx_t         *pDtiDevCtx[DTI_MAX_DEVICE_INTERFACES];

#if    defined(INCLUDE_CLI_SUPPORT) && (INCLUDE_CLI_SUPPORT == 1) \
    && defined(DTI_CLI_EVENT_SUPPORT) && (DTI_CLI_EVENT_SUPPORT == 1)
   /** CLI Event handling */
   DTI_CliEventCtx_t       cliEventCtx;
#endif

} DTI_WorkerCtx_t;


/**
   DTI Agent Context.
*/
struct DTI_AgentCtx_s
{
   /** protect context struct */
   IFXOS_lock_t            dataLock;

   /*
      Listener configuration
   */
   /** DTI Agent is running */
   IFX_boolean_t           bListenRun;

   /** listen Connection */
   DTI_Connection_t        listenCon;
   /** connection control data */
   DTI_ConnectionCntrl_t   listenConCntrl;

   /*+ listener thread */
   IFXOS_ThreadCtrl_t      listenerThr;

   /** number of used worker threads */
   IFX_int_t               numOfUsedWorker;

   /** only one thread for listen and work */
   IFX_boolean_t           bSingleThreadedMode;

   /* current number of worker threads */
   IFX_int_t               nWorkers;

   /* worker threads */
   DTI_WorkerCtx_t         *pWorker[DTI_MAX_NUM_OF_WORKER];

   /** number of available device interfaces */
   IFX_int_t               numOfDevIf;

   /** available device interfaces */
   DTI_deviceInterface_t   deviceInterface[DTI_MAX_DEVICE_INTERFACES];

#if defined(INCLUDE_CLI_SUPPORT) && (INCLUDE_CLI_SUPPORT == 1)
   /** enable autonomous CLI message handling */
   IFX_boolean_t           bControlAutoCliMsgSupport;
   /** control struct for the CLI interfaces */
   DTI_cli_control_t       cliControl[DTI_CLI_MAX_CLI_INTERFACES];
#endif

   DTI_ImageControl_t      imageCntrl[DTI_MAX_NUM_OF_IMAGES];

#if defined(DTI_INCLUDE_CORE_STATISTICS) && (DTI_INCLUDE_CORE_STATISTICS == 1)
   struct ltq_dti_statistic_agent_s stat_agent;
   struct ltq_dti_statistic_worker_thr_s stat_worker_thr[DTI_MAX_NUM_OF_WORKER];
   struct ltq_dti_statistic_connection_s stat_conn[DTI_MAX_NUM_OF_WORKER];
#endif
};


/* ==========================================================================
   Exports
   ========================================================================== */
IFXOS_PRN_USR_MODULE_DECL(DTI_DBG);

extern DTI_cliConfigKeyword_t DTI_cliControlGroupEntries[];
extern DTI_cliConfigKeyword_t DTI_cliControlGrConfigEntries[];
extern DTI_cliConfigKeyword_t DTI_cliControlGrDeviceEntries[];
extern DTI_cliConfigKeyword_t DTI_cliControlGrCliEntries[];

extern IFX_int_t DTI_cliControlGetNextDigitVal(
                        IFX_char_t        **ppArgStr,
                        IFX_uint_t        *pArgLen,
                        IFX_uint_t        *pULVal,
                        IFX_uint_t        base);

extern IFX_int_t DTI_ListenerThread_Startup (
                        DTI_AgentCtx_t    *pAgentCtx);

extern IFX_int_t DTI_ListenerThread_Stop (
                        DTI_AgentCtx_t    *pAgentCtx);


#ifdef __cplusplus
}
#endif

#endif /* _DTI_CONTROL_H */

