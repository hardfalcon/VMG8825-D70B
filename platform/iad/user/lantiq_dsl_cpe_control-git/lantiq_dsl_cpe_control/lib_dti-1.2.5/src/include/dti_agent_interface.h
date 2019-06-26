#ifndef _DTI_AGENT_INTERFACE_H
#define _DTI_AGENT_INTERFACE_H
/******************************************************************************

                              Copyright (c) 2009
                            Lantiq Deutschland GmbH
                     Am Campeon 3; 85579 Neubiberg, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/** \file
   Debug and Trace Interface definitions and declarations.
*/


#ifdef __cplusplus
   extern "C" {
#endif

/* ==========================================================================
   includes
   ========================================================================== */
#include "ifx_types.h"

#include "dti_device.h"

/* ==========================================================================
   Defines
   ========================================================================== */

#ifndef _MKSTR_1
#define _MKSTR_1(x)    #x
#define _MKSTR(x)      _MKSTR_1(x)
#endif

/** DTI version, major number */
#define DTI_AGENT_VER_MAJOR         1
/** DTI version, minor number */
#define DTI_AGENT_VER_MINOR         2
/** DTI version, build number */
#define DTI_AGENT_VER_STEP          5

#ifndef DTI_AGENT_VER_BUILD
    /** DTI Agent, internal build number */
#   define DTI_AGENT_VER_BUILD      0
#endif

#define DTI_AGENT_VER \
		(  (DTI_AGENT_VER_MAJOR << 24) \
		 | (DTI_AGENT_VER_MINOR << 16) \
		 | (DTI_AGENT_VER_STEP  << 8) \
		 | (DTI_AGENT_VER_BUILD) )


/** DTI version string */
#if (DTI_AGENT_VER_BUILD == 0)
#define DTI_AGENT_VER_STR       _MKSTR(DTI_AGENT_VER_MAJOR)"."_MKSTR(DTI_AGENT_VER_MINOR)"."_MKSTR(DTI_AGENT_VER_STEP)
#else
#define DTI_AGENT_VER_STR       _MKSTR(DTI_AGENT_VER_MAJOR)"."_MKSTR(DTI_AGENT_VER_MINOR)"."_MKSTR(DTI_AGENT_VER_STEP)"."_MKSTR(DTI_AGENT_VER_BUILD)
#endif

/** DTI what string */
#define DTI_AGENT_WHAT_STR      "@(#)Debug and Trace Interface, DTI Agent " DTI_AGENT_VER_STR

/* single threaded mode is supported by this version */
#define DTI_SUPPORT_SINGLE_THREADED_MODE              1

/* DTI Statistic and Status Information */
#define DTI_SUPPORT_STATISTIC_STATUS                  1

/** DTI Agent device config remove function available */
#define DTI_SUPPORT_DEV_CONFIG_REMOVE                 1


/** max number of client (worker) threads */
#define DTI_MAX_NUM_OF_WORKER                         4

/** max number of supported device interfaces */
#define DTI_MAX_DEVICE_INTERFACES                     4

#define DTI_CLI_CONTROL_PREFIX                        "@DTI# "

/** returns the number of used worker threads */
#define DTI_NUM_OF_USED_WORKER_GET(p_dti_agent_ctx) \
            (((p_dti_agent_ctx) != IFX_NULL) ? (p_dti_agent_ctx)->numOfUsedWorker : -1)

/* ==========================================================================
   Types
   ========================================================================== */

/** Forward definition of main DTI Agent Context structure */
typedef struct DTI_AgentCtx_s    DTI_AgentCtx_t;

/* DTI error codes */
enum DTI_Error_e
{
   /** Operation failed because of invalid / missing config */
   DTI_ERR_CONFIGURATION   = -5,

   /** The remote connection cannot established or has dropped */
   DTI_ERR_CONNECTION      = -4,

   /** There is not enough memory to finish current operation */
   DTI_ERR_NO_MEMORY       = -3,
   /** A passed argument pointer is invalid */
   DTI_ERR_NULL_PTR_ARG    = -2,

   /** Internal error */
   DTI_ERR_INTERNAL        = IFX_ERROR,
   /** No error */
   DTI_SUCCESS             = IFX_SUCCESS
};


/**
   Startup settings for the DTI agent.
*/
typedef struct DTI_AgentStartupSettings_s
{
   /** user given number of devices */
   IFX_int_t      numOfDevices;
   /** user given lines per device */
   IFX_int_t      linesPerDevice;
   /** enable autonomous device message handling */
   IFX_int_t      bStartupAutoDevMsgSupport;
   /** enable autonomous CLI message handling */
   IFX_int_t      bStartupAutoCliMsgSupport;

   /** user given debug level */
   IFX_int_t      debugLevel;

   /** number of used worker threads */
   IFX_int_t      numOfUsedWorker;

   /** only one thread for listen and work */
   IFX_int_t      bSingleThreadedMode;

   /**  used listen port for the DTI agent (listen) */
   IFX_uint16_t   listenPort;
   /** used IP address of the DTI agent (listen) */
   IFX_char_t     serverIpAddr[16];

} DTI_AgentStartupSettings_t;

/**
   Startup settings for a device interfaces.
*/
typedef struct DTI_AgentStartupDevIfSettings_s
{
   /** user given number of devices */
   IFX_int_t      numOfDevices;
   /** user given lines per device */
   IFX_int_t      linesPerDevice;
   /** enable autonomous device message handling */
   IFX_int_t      bStartupAutoDevMsgSupport;
   /* points to the corresponding device access functions */
   DTI_DeviceAccessFct_t *pDeviceAccessFct;
   /** Optional parameter, could be used in the device specific agent implementation */
   IFX_int_t      param0;
} DTI_DeviceIfSettings_t;

/**
   Startup settings for the DTI agent.
*/
typedef struct DTI_AgentStartupSettingsXDevs_s
{
   /** number of used device interfaces */
   IFX_int_t      numOfUsedDevIf;
   /** user given number of devices */
   DTI_DeviceIfSettings_t devIfSettings[DTI_MAX_DEVICE_INTERFACES];

   /** user given debug level */
   IFX_int_t      debugLevel;

   /** enable autonomous CLI message handling */
   IFX_int_t      bStartupAutoCliMsgSupport;

   /** number of used worker threads */
   IFX_int_t      numOfUsedWorker;

   /** only one thread for listen and work */
   IFX_int_t      bSingleThreadedMode;

   /**  used listen port for the DTI agent (listen) */
   IFX_uint16_t   listenPort;
   /** used IP address of the DTI agent (listen) */
   IFX_char_t     serverIpAddr[16];

} DTI_AgentStartupSettingsXDevs_t;


/* ==========================================================================
   Exports
   ========================================================================== */

/** Start the DTI agent. */
extern IFX_int_t DTI_AgentStart(
                        DTI_AgentCtx_t             **ppDtiAgentCtx,
                        DTI_AgentStartupSettings_t *pAgentStartupSettings);

extern IFX_int_t DTI_AgentStartXDevs(
                        DTI_AgentCtx_t                   **ppDtiAgentCtx,
                        DTI_AgentStartupSettingsXDevs_t  *pAgentStartupSettingsXDevs);

/** Start the DTI agent. */
extern IFX_int_t DTI_AgentStop(
                        DTI_AgentCtx_t  **ppDtiAgentCtx);

/** Add a DTI Device interface to the given agent */
extern IFX_int_t DTI_DeviceConfigAdd(
                        DTI_AgentCtx_t          *pAgentCtx,
                        IFX_int_t               devIfNumber,
                        DTI_DeviceIfSettings_t  *pDeviceIfSettings);

/** Remove a DTI Device interface from the given agent */
extern IFX_int_t DTI_DeviceConfigRemove(
                        DTI_AgentCtx_t  *pAgentCtx,
                        IFX_int_t       devIfNumber,
                        IFX_char_t      *pDeviceIfName);

/** Set the debug level of the DTI interface. */
extern IFX_void_t DTI_DebugLevelSet(
                        IFX_uint32_t newLevel);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* _DTI_AGENT_INTERFACE_H */

