/******************************************************************************

                              Copyright (c) 2009
                            Lantiq Deutschland GmbH
                     Am Campeon 3; 85579 Neubiberg, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

#include "dti_osmap.h"
#include "dti_connection_interface.h"
#include "dti_agent_interface.h"
#include "dti_agent_main.h"

#if defined(LINUX)
#  include <getopt.h>
#  define  DTI_GETOPT_DUMMY   NULL
#else
#  include "ifx_getopt.h"
#  define  DTI_GETOPT_DUMMY   0
#endif

#include "ifx_getopt_ext.h"

/* ==========================================================================
   Defines and types
   ========================================================================== */

#ifdef DTI_STATIC
#undef DTI_STATIC
#endif

#ifdef DTI_DEBUG
#define DTI_STATIC
#else
#define DTI_STATIC   static
#endif

/* for testing, scan for input to terminate */
#define DTI_TEST_MODE   1


#if defined(IFXOS_SUPPORT_GET_OPT_R) && (IFXOS_SUPPORT_GET_OPT_R == 1)

#define DTI_OPTIND	dti_optind
#define DTI_OPTARG	dti_optarg
#define DTI_GETOPT_LONG(ARGC, ARGV, L_OPT_STR, P_OPT, OPT_IDX, PP_OPTARG, P_OPTIND) \
		getopt_long_r(ARGC, ARGV, L_OPT_STR, P_OPT, OPT_IDX, PP_OPTARG, P_OPTIND)

#else

#define DTI_OPTIND	optind
#define DTI_OPTARG	optarg
#define DTI_GETOPT_LONG(ARGC, ARGV, L_OPT_STR, P_OPT, OPT_IDX, PP_OPTARG, P_OPTIND) \
		getopt_long(ARGC, ARGV, L_OPT_STR, P_OPT, OPT_IDX)

#endif


/*
   Local arguments
*/
typedef struct DTI_opArguments_s
{
   GetOptExt_IntArg_t   help;                /* h */
   GetOptExt_IntArg_t   dbgLevelDevice;      /* v */
   GetOptExt_IntArg_t   numOfWorker;         /* w */
   GetOptExt_IntArg_t   runInForground;      /* f */
   GetOptExt_IntArg_t   linesPerDevice;      /* l */
   GetOptExt_IntArg_t   devices;             /* d */
   GetOptExt_IntArg_t   dtiPort;             /* p */
   GetOptExt_IntArg_t   devAutoMsg;          /* D */
   GetOptExt_IntArg_t   cliAutoMsg;          /* C */
} DTI_opArguments_t;


#if defined(DEVICE_GENERIC)
extern DTI_DeviceAccessFct_t DTI_DeviceAccessFct_GENERIC;
#endif

/* ==========================================================================
   Local Function Declaration
   ========================================================================== */
DTI_STATIC IFX_void_t DTI_agentDoHelp(void);

DTI_STATIC IFX_int_t DTI_agent(
                           DTI_AgentCtx_t **ppDtiAgentCtx);

/* ==========================================================================
   Local Variables
   ========================================================================== */

#if defined(IFXOS_SUPPORT_GET_OPT_R) && (IFXOS_SUPPORT_GET_OPT_R == 1)
/* local definition of the optind and optarg variable, used by getopt_long_r(...) */
static int DTI_OPTIND = 0;
static char *DTI_OPTARG = IFX_NULL;
#endif

DTI_STATIC struct option DTI_longOptions[] =
{
/*  0 */   {"Help"    , no_argument,            DTI_GETOPT_DUMMY, 'h'},
/*  1 */   {"dev_dbg" , required_argument,      DTI_GETOPT_DUMMY, 'v'},
/*  2 */   {"num_wkr" , required_argument,      DTI_GETOPT_DUMMY, 'w'},
/*  3 */   {"fground" , no_argument,            DTI_GETOPT_DUMMY, 'f'},
/*  4 */   {"Lines"   , required_argument,      DTI_GETOPT_DUMMY, 'l'},
/*  5 */   {"Devices" , required_argument,      DTI_GETOPT_DUMMY, 'd'},
/*  6 */   {"Port"    , required_argument,      DTI_GETOPT_DUMMY, 'p'},
/*  7 */   {"DevAuto" , required_argument,      DTI_GETOPT_DUMMY, 'D'},
/*  8 */   {"CliAuto" , required_argument,      DTI_GETOPT_DUMMY, 'C'},
/*    */   {"ip_Addr" , required_argument,      DTI_GETOPT_DUMMY, 'a'},
           {NULL, 0, DTI_GETOPT_DUMMY, 0}
};

/*
                                   01 2 34 5   */
#define DTI_GETOPT_LONG_OPTSTRING "hv:w:fl:d:p:D:C:a:"

DTI_STATIC const char *DTI_description[] =
{
/*  0 */   "help screen",
/*  1 */   "debug level device -v <new level>",
/*  2 */   "num of worker thr  -w <num of worker>",
/*  3 */   "run in forground   -f <none>",
/*  4 */   "lines per device   -l <lines>",
/*  5 */   "devices            -d <devices>",
/*  6 */   "server port        -p <port>",
/*  7 */   "devcie AutoMsg     -D 1",
/*  8 */   "CLI AutoMsg        -C 1",
/*    */   "server IP address  -a <ip addr (ASCII)>",
           IFX_NULL
};

/** getOpt option argument list */
DTI_STATIC DTI_opArguments_t   DTI_opArguments;
/*
                                    { {0, IFX_FALSE} };
*/

/** getOpt string argument list */
DTI_STATIC GetOptExt_StrArg_t DTI_strParams[GET_OPT_EXT_MAX_STR_PARAMS] =
                                    { {"\0", IFX_FALSE} };


#if defined(DTI_LIBRARY)

/** max argument string len */
#define DTI_OPTIONS_MAX_LEN      255
/** max number of arguments */
#define DTI_OPTIONS_MAX_NUM      32

/** keep a copy of the original argument string, each argument will be terminated */
DTI_STATIC IFX_char_t  DTI_controlArgStr[DTI_OPTIONS_MAX_LEN];
/** pointer array to point to the single arguments within the arg-string */
DTI_STATIC IFX_char_t  *DTI_controlOptions[DTI_OPTIONS_MAX_NUM] = {IFX_NULL};

#endif

/* ==========================================================================
   Global Variables
   ========================================================================== */
IFX_int_t   DTI_mainRun = 0;

/* ==========================================================================
   Local Function Definitons
   ========================================================================== */

/**
   Printout the "Help Info" for the available arguments.
*/
DTI_STATIC IFX_void_t DTI_agentDoHelp(void)
{
   struct option *ptr;
   const char **desc = &DTI_description[0];
   ptr = DTI_longOptions;

   DTI_Printf("%s" DTI_CRLF, DTI_AGENT_WHAT_STR);
   DTI_Printf("usage: dti agent [options]"DTI_CRLF);
   DTI_Printf("following options defined:"DTI_CRLF);

   while(ptr->name)
   {
      DTI_Printf(" --%s \t(-%c) - %s" DTI_CRLF, ptr->name, ptr->val, *desc);

      ptr++;
      desc++;
   }

   DTI_Printf(DTI_CRLF);

   return;
}

/*
   Start the DTI agent with the given startup parameters
*/
DTI_STATIC IFX_int_t DTI_agent(
                           DTI_AgentCtx_t **ppDtiAgentCtx)
{
   IFX_int_t         devIfNum = 0, numOfDevices = 0, linesPerDevice = 0, bAutoDevMsg = 0;
   DTI_AgentCtx_t             *pDtiAgentCtx;
   DTI_AgentStartupSettingsXDevs_t  AgentStartupSettingsXDevs;
   DTI_DeviceAccessFct_t            *pDeviceAccessFct = IFX_NULL;

   DTI_MemSet(&AgentStartupSettingsXDevs, 0x00, sizeof(DTI_AgentStartupSettingsXDevs_t));

   AgentStartupSettingsXDevs.debugLevel =
         (DTI_opArguments.dbgLevelDevice.bSet == IFX_TRUE) ? DTI_opArguments.dbgLevelDevice.intValue : 4;

   AgentStartupSettingsXDevs.numOfUsedWorker =
         (DTI_opArguments.numOfWorker.bSet == IFX_TRUE) ? DTI_opArguments.numOfWorker.intValue : 0;

   AgentStartupSettingsXDevs.bStartupAutoCliMsgSupport =
         (DTI_opArguments.cliAutoMsg.bSet == IFX_TRUE) ? DTI_opArguments.cliAutoMsg.intValue : 0;

   AgentStartupSettingsXDevs.listenPort = (IFX_uint16_t)
         ((DTI_opArguments.dtiPort.bSet == IFX_TRUE) ? DTI_opArguments.dtiPort.intValue : 0);

   if (DTI_strParams[0].bSet == IFX_TRUE)
   {
      DTI_MemCpy(AgentStartupSettingsXDevs.serverIpAddr, DTI_strParams[0].strValue, 16);
      AgentStartupSettingsXDevs.serverIpAddr[15] = '\0';
   }

   numOfDevices   = (DTI_opArguments.devices.bSet == IFX_TRUE) ? DTI_opArguments.devices.intValue : 0;
   linesPerDevice = (DTI_opArguments.linesPerDevice.bSet == IFX_TRUE) ? DTI_opArguments.linesPerDevice.intValue : 0;
   bAutoDevMsg    = (DTI_opArguments.devAutoMsg.bSet == IFX_TRUE) ? DTI_opArguments.devAutoMsg.intValue : 0;

#if defined(DEVICE_GENERIC)
   if (devIfNum < DTI_MAX_DEVICE_INTERFACES)
   {
      pDeviceAccessFct = &DTI_DeviceAccessFct_GENERIC;
      if ( (devIfNum > 0) && (pDeviceAccessFct != IFX_NULL) )
      {
         /* workaround (for testing multiple devices)
            this is not the first device, use default values for setup */
         numOfDevices   = 1;
         linesPerDevice = 1;
         bAutoDevMsg    = 0;
      }

      if ( (pDeviceAccessFct != IFX_NULL) &&
           (numOfDevices > 0) && (linesPerDevice > 0) )
      {
         DTI_Printf("Device[%d]: Add GENERIC - devs = %d lines per devs = %d\n",
            devIfNum, numOfDevices, linesPerDevice);

         AgentStartupSettingsXDevs.devIfSettings[devIfNum].numOfDevices     = numOfDevices;
         AgentStartupSettingsXDevs.devIfSettings[devIfNum].linesPerDevice   = linesPerDevice;
         AgentStartupSettingsXDevs.devIfSettings[devIfNum].pDeviceAccessFct = pDeviceAccessFct;
         AgentStartupSettingsXDevs.devIfSettings[devIfNum].bStartupAutoDevMsgSupport = bAutoDevMsg;

         devIfNum++;
      }
      pDeviceAccessFct = IFX_NULL;
   }
#endif
   AgentStartupSettingsXDevs.numOfUsedDevIf = devIfNum;

   if (DTI_opArguments.dbgLevelDevice.bSet == IFX_TRUE)
   {
      DTI_DebugLevelSet((IFX_uint32_t)AgentStartupSettingsXDevs.debugLevel);
   }

   if ( DTI_AgentStartXDevs(&pDtiAgentCtx, &AgentStartupSettingsXDevs) == IFX_SUCCESS)
   {
      DTI_Printf("Agent started, press any key and Enter to exit\n");

      DTI_mainRun = 1;
      *ppDtiAgentCtx = pDtiAgentCtx;

      return IFX_SUCCESS;
   }

   DTI_mainRun = 0;

   return IFX_ERROR;
}

/* ==========================================================================
   Global Part
   ========================================================================== */

/**
   Parse the given argument array into the option control struct.
*/
IFX_void_t DTI_agentOptArgParse(
                              IFX_int_t   argc,
                              IFX_char_t  *argv[])
{
   int parm_no=0;

   /*
      For library call
      If the getopt is used while normal operation, the static and global
      variables have to be reset before each parse. */
   DTI_MemSet(&DTI_opArguments, 0x00, sizeof(DTI_opArguments_t));
   DTI_MemSet(DTI_strParams, 0x00, sizeof(DTI_strParams));
   DTI_OPTIND = 0;

   while(1)
   {
      int option_index = DTI_OPTIND ? DTI_OPTIND : 1;
      int c;

      /* 1 colon means there is a required parameter */
      /* 2 colons means there is an optional parameter */
      c = DTI_GETOPT_LONG(
             argc, argv, DTI_GETOPT_LONG_OPTSTRING,
             DTI_longOptions, &option_index,
             &DTI_OPTARG, &DTI_OPTIND);

      if(c == -1)
      {
         if (parm_no==0)
         {
            DTI_opArguments.help.bSet = IFX_TRUE;
            DTI_opArguments.help.intValue = 1;
         }
         break;
      }

      parm_no++;

      switch(c)
      {
         case 'h':
            DTI_opArguments.help.intValue = 1;
            DTI_opArguments.help.bSet     = IFX_TRUE;
            break;
         case 'v':
            GetOptExt_RequiredDigit(DTI_OPTARG, &DTI_opArguments.dbgLevelDevice, "dbg device");
            break;
         case 'w':
            GetOptExt_RequiredDigit(DTI_OPTARG, &DTI_opArguments.numOfWorker, "num of worker");
            break;
         case 'f':
            DTI_opArguments.runInForground.intValue = 1;
            DTI_opArguments.runInForground.bSet     = IFX_TRUE;
            break;

         case 'l':
            GetOptExt_RequiredDigit(DTI_OPTARG, &DTI_opArguments.linesPerDevice, "lines");
            break;
         case 'd':
            GetOptExt_RequiredDigit(DTI_OPTARG, &DTI_opArguments.devices, "devices");
            break;
         case 'p':
            GetOptExt_RequiredDigit(DTI_OPTARG, &DTI_opArguments.dtiPort, "port");
            break;

         case 'D':
            GetOptExt_RequiredDigit(DTI_OPTARG, &DTI_opArguments.devAutoMsg, "DevAuto");
            break;
         case 'C':
            GetOptExt_RequiredDigit(DTI_OPTARG, &DTI_opArguments.cliAutoMsg, "CliAuto");
            break;

         case 'a':
            GetOptExt_RequiredStr(DTI_OPTARG, &DTI_strParams[0], "IP Addr");
            break;
         default:
            break;
      }        /* switch(c) {...} */
   }        /* while(1) {...} */

   if (DTI_OPTIND < (argc -1))
   {
      DTI_Printf("Sorry, there are unrecognized options: ");
      while (DTI_OPTIND < argc)
         DTI_Printf("%s ", argv[DTI_OPTIND++]);

      DTI_Printf("\n");
   }
}

/*
   Check if only the help is requested
*/
IFX_int_t DTI_agentParseHelp(void)
{
   if( DTI_opArguments.help.bSet == 1)
   {
      DTI_agentDoHelp();

      return IFX_ERROR;
   }

   return IFX_SUCCESS;
}


#if defined(DTI_LIBRARY)

DTI_AgentCtx_t *pGlobalDtiAgentCtx = IFX_NULL;

IFX_int_t DTI_agent_libmain(
                     const IFX_char_t  *pArgString)
{
   IFX_int_t      retVal = IFX_SUCCESS, argCnt = 0;
   DTI_AgentCtx_t *pDtiAgentCtx;

   if (!pArgString)
   {
      DTI_agentDoHelp();

      return IFX_SUCCESS;
   }

   DTI_MemSet(DTI_controlOptions, 0x00, sizeof(DTI_controlOptions));
   DTI_MemSet(DTI_controlArgStr, 0x00, sizeof(DTI_controlArgStr));

   DTI_snprintf(DTI_controlArgStr, DTI_OPTIONS_MAX_LEN, "dti %s", pArgString);

   argCnt = GetOptExt_ParseArgString(
                  DTI_controlArgStr,
                  DTI_controlOptions,
                  DTI_OPTIONS_MAX_NUM);

   if (argCnt <= 0)
   {
      DTI_Printf("Error: DTI Lib - get options <%s>\n", pArgString);

      return IFX_ERROR;
   }

   DTI_agentOptArgParse(argCnt, DTI_controlOptions);
   if( DTI_agentParseHelp() != IFX_SUCCESS)
   {
      return IFX_ERROR;
   }

   if ((retVal = DTI_agent(&pDtiAgentCtx)) == IFX_SUCCESS)
   {
      pGlobalDtiAgentCtx = pDtiAgentCtx;

      while (DTI_mainRun == 1)
      {
         DTI_SecSleep(1);
      }

      DTI_AgentStop(&pDtiAgentCtx);
   }

   return retVal;
}
#endif   /* #if defined(DTI_LIBRARY) */


#if !defined(DTI_LIBRARY)
/* ==========================================================================
   main part
   ========================================================================== */


/* ==========================================================================
   main part - local function declaration
   ========================================================================== */

DTI_STATIC IFX_boolean_t DTI_AgentKeyGet(
                           IFX_char_t  *pInKey, IFX_char_t  *pInKey_num);

/* ==========================================================================
   main part - variables
   ========================================================================== */

static IFX_char_t DTI_main_inKey = 'c';
static IFX_char_t DTI_main_inKey_num = '0';

/* ==========================================================================
   main part - local function definiton
   ========================================================================== */

/**
   Check for for keypressed and terminate
*/
DTI_STATIC IFX_boolean_t DTI_AgentKeyGet(
                           IFX_char_t  *pInKey, IFX_char_t  *pInKey_num)
{
   IFX_int_t  inKey = (IFX_int_t)'c';

   while(1)
   {
      inKey = (char)DTI_GetChar();

      if (DTI_IsAlNum(inKey))
         break;
   }

   if (DTI_IsDigit(inKey))
   {
      if (pInKey_num)
         *pInKey_num = (IFX_char_t)inKey;
   }

   if (DTI_IsAlpha(inKey))
   {
      if (pInKey)
         *pInKey = (IFX_char_t)inKey;
   }

   if ( (IFX_char_t)inKey == 'q')
      return IFX_TRUE;
   else
      return IFX_FALSE;
}

/* ==========================================================================
   main
   ========================================================================== */

int main(int argc, char * argv[])
{
   IFX_int_t      retVal = IFX_SUCCESS;
   DTI_AgentCtx_t *pDtiAgentCtx;

   DTI_agentOptArgParse(argc,argv);

   if( DTI_agentParseHelp() != IFX_SUCCESS)
   {
      return -1;
   }

   if (DTI_conInit() != IFX_SUCCESS)
   {
      DTI_Printf("Error: DTI - Start Agent, Socket init\n");
      return -1;
   }

   if ((retVal = DTI_agent(&pDtiAgentCtx)) == IFX_SUCCESS)
   {
      DTI_Printf("DTI - press <q> for quit" DTI_CRLF);

      while (DTI_mainRun == 1)
      {
         if (DTI_opArguments.runInForground.bSet == IFX_FALSE)
         {
            DTI_SecSleep(1);
         }
         else
         {
            (void)DTI_AgentKeyGet(&DTI_main_inKey, &DTI_main_inKey_num);

            switch (DTI_main_inKey)
            {
               case 'l':
                  /* update debug level */
                  {
                     IFX_uint8_t dbgLevel = 0;
                     dbgLevel = (IFX_uint8_t)(DTI_main_inKey_num - '0');
                     if ( (dbgLevel > 0) && (dbgLevel < 5) )
                     {
                        DTI_Printf("DTI - change: debug level %d" DTI_CRLF, dbgLevel);
                        DTI_DebugLevelSet((IFX_uint32_t)dbgLevel);
                     }
                  }
                  break;

               case 'c':
                  /* continue */
                  break;

#if defined(DEVICE_GENERIC)
               case 'g':
                  {
                     /* add generic interface for testing */
                     DTI_DeviceIfSettings_t devIfGeneric;

                     devIfGeneric.numOfDevices   = 1;
                     devIfGeneric.linesPerDevice = 1;
                     devIfGeneric.bStartupAutoDevMsgSupport = IFX_FALSE;
                     devIfGeneric.pDeviceAccessFct = &DTI_DeviceAccessFct_GENERIC;

                     DTI_Printf("DTI - Add Generic Device" DTI_CRLF);
                     (void)DTI_DeviceConfigAdd(pDtiAgentCtx, -1, &devIfGeneric);
                  }
                  break;
#endif

               case 'b':
                  DTI_Printf("DTI - Switch to Background Mode" DTI_CRLF);
                  DTI_opArguments.runInForground.bSet = IFX_TRUE;
                  break;

               case 'q':
                  DTI_Printf("DTI - Stop Agent" DTI_CRLF);
                  DTI_mainRun = 0;
                  break;

                default:
                  break;
            }

         }
      }

      (void)DTI_AgentStop(&pDtiAgentCtx);
   }

   (void)DTI_conCleanup();

   return retVal;
}

#endif      /* #if !defined(DTI_LIBRARY) */

