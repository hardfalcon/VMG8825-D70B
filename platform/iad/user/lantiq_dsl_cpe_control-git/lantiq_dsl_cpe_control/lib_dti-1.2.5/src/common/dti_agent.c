/******************************************************************************

                              Copyright (c) 2009
                            Lantiq Deutschland GmbH
                     Am Campeon 3; 85579 Neubiberg, Germany

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/** \file
   Debug and Trace Interface - Server/Worker Thread Handling.
*/

/* ============================================================================
   Includes
   ========================================================================= */

#include "dti_osmap.h"

#include "dti_protocol_interface.h"
#include "ifx_dti_protocol.h"

#include "dti_device.h"
#include "dti_packet_device.h"

#include "dti_agent_interface.h"
#include "dti_control.h"

/* ============================================================================
   Defines
   ========================================================================= */

#ifdef DTI_STATIC
#  undef DTI_STATIC
#endif

#ifdef DTI_DEBUG
#  define DTI_STATIC
#else
#  define DTI_STATIC   static
#endif

/* ============================================================================
   Local Function Declaration
   ========================================================================= */

DTI_STATIC IFX_int_t DTI_AgentConvertStartupSettings(
                        DTI_AgentStartupSettings_t       *pAgentStartupSettings,
                        DTI_AgentStartupSettingsXDevs_t  *pAgentStartupSettingsXDevs);

DTI_STATIC IFX_int_t DTI_AgentCtxInit(
                        DTI_AgentCtx_t                   *pAgentCtx,
                        DTI_AgentStartupSettingsXDevs_t  *pAgentStartupSettingsXDevs);

DTI_STATIC IFX_int_t DTI_DeviceConfigSet(
                        DTI_AgentCtx_t                   *pAgentCtx,
                        DTI_AgentStartupSettingsXDevs_t  *pAgentStartupSettingsXDevs);

DTI_STATIC IFX_int_t DTI_ListenerIpConfigSet(
                        DTI_AgentCtx_t                   *pAgentCtx,
                        DTI_AgentStartupSettingsXDevs_t  *pAgentStartupSettingsXDevs);

DTI_STATIC IFX_int_t DTI_Agent_systemInfoWrite(
                        IFX_void_t        *pUserCtx,
                        IFX_char_t        *pSysInfoBuffer,
                        IFX_int_t         bufferSize);

DTI_STATIC IFX_int_t DTI_ModulesStart(
                        DTI_AgentCtx_t                   *pAgentCtx,
                        DTI_AgentStartupSettingsXDevs_t  *pAgentStartupSettingsXDevs);

DTI_STATIC IFX_int_t DTI_ModulesStop(
                        DTI_AgentCtx_t    *pAgentCtx);




/* ============================================================================
   Variables
   ========================================================================= */

/* ============================================================================
   Local Functions
   ========================================================================= */

/**
   DTI Agent - takeover current startup settings into the new multi-device structure.
*/
DTI_STATIC IFX_int_t DTI_AgentConvertStartupSettings(
                        DTI_AgentStartupSettings_t       *pAgentStartupSettings,
                        DTI_AgentStartupSettingsXDevs_t  *pAgentStartupSettingsXDevs)
{

   if ( (pAgentStartupSettings == IFX_NULL) || (pAgentStartupSettingsXDevs == IFX_NULL) )
   {
      return IFX_ERROR;
   }

   DTI_MemSet(pAgentStartupSettingsXDevs, 0x00, sizeof(DTI_AgentStartupSettingsXDevs_t));

   /*
      Takeover Common Settings
   */
   pAgentStartupSettingsXDevs->debugLevel                = pAgentStartupSettings->debugLevel;
   pAgentStartupSettingsXDevs->bStartupAutoCliMsgSupport = pAgentStartupSettings->bStartupAutoCliMsgSupport;
   pAgentStartupSettingsXDevs->numOfUsedWorker           = pAgentStartupSettings->numOfUsedWorker;
   pAgentStartupSettingsXDevs->bSingleThreadedMode       = pAgentStartupSettings->bSingleThreadedMode;
   pAgentStartupSettingsXDevs->listenPort                = pAgentStartupSettings->listenPort;
   DTI_MemCpy(pAgentStartupSettingsXDevs->serverIpAddr, pAgentStartupSettings->serverIpAddr, 16);

   return IFX_SUCCESS;
}

/**
   DTI Agent - context init
*/
DTI_STATIC IFX_int_t DTI_AgentCtxInit(
                        DTI_AgentCtx_t                   *pAgentCtx,
                        DTI_AgentStartupSettingsXDevs_t  *pAgentStartupSettingsXDevs)
{
   IFX_int_t imgNum;

   if (pAgentCtx == IFX_NULL)
   {
      return IFX_ERROR;
   }

   /* prepare context structure */
   DTI_MemSet(pAgentCtx, 0, sizeof(DTI_AgentCtx_t));
   DTI_LockInit(&pAgentCtx->dataLock);

   if (pAgentStartupSettingsXDevs->bSingleThreadedMode == 0)
   {
      pAgentStartupSettingsXDevs->bSingleThreadedMode =
            (pAgentStartupSettingsXDevs->numOfUsedWorker == 1) ? IFX_TRUE : IFX_FALSE;
   }
   else
   {
      pAgentStartupSettingsXDevs->numOfUsedWorker = 1;
   }

   if ( (pAgentStartupSettingsXDevs->numOfUsedWorker >= 0) &&
        (pAgentStartupSettingsXDevs->numOfUsedWorker <= DTI_MAX_NUM_OF_WORKER) )
   {
      pAgentCtx->numOfUsedWorker =
         (pAgentStartupSettingsXDevs->numOfUsedWorker == 0) ?
               DTI_MAX_NUM_OF_WORKER : pAgentStartupSettingsXDevs->numOfUsedWorker;
   }
   else
   {
      DTI_PRN_USR_ERR_NL(DTI_DBG, DTI_PRN_LEVEL_ERR,
         ("ERROR: DTI Agent Ctx Init - num of worker threads = %d, max = %d)" DTI_CRLF,
          pAgentStartupSettingsXDevs->numOfUsedWorker, DTI_MAX_NUM_OF_WORKER));

      return IFX_ERROR;
   }

   pAgentCtx->bSingleThreadedMode =
      (pAgentStartupSettingsXDevs->bSingleThreadedMode) ? IFX_TRUE : IFX_FALSE;

   for (imgNum = 0; imgNum < DTI_MAX_NUM_OF_IMAGES; imgNum++)
   {
      pAgentCtx->imageCntrl[imgNum].imageId = imgNum;
      pAgentCtx->imageCntrl[imgNum].maxChunkSize = DTI_MAX_IMAGE_CHUNK_SIZE;
      pAgentCtx->imageCntrl[imgNum].maxImageSize = DTI_MAX_IMAGE_SIZE;
   }

   return IFX_SUCCESS;
}

/**
   Set the device configuration

\remark
   Allow setup only if the DTI is still not started
*/
DTI_STATIC IFX_int_t DTI_DeviceConfigSet(
                        DTI_AgentCtx_t                   *pAgentCtx,
                        DTI_AgentStartupSettingsXDevs_t  *pAgentStartupSettingsXDevs)
{
   IFX_int_t devIdx = 0;

   pAgentCtx->numOfDevIf = 0;

   /*
      Setup the available device interfaces.
   */
   for (devIdx = 0; devIdx < DTI_MAX_DEVICE_INTERFACES; devIdx++)
   {
      if ( (pAgentStartupSettingsXDevs->devIfSettings[devIdx].pDeviceAccessFct != IFX_NULL) &&
           (pAgentStartupSettingsXDevs->devIfSettings[devIdx].numOfDevices     >  0) &&
           (pAgentStartupSettingsXDevs->devIfSettings[devIdx].linesPerDevice   >  0) )
      {
         (void)DTI_DeviceConfigAdd(
                        pAgentCtx, devIdx,
                        &pAgentStartupSettingsXDevs->devIfSettings[devIdx]);
      }
   }     /* for (devIdx = 0; devIdx < DTI_MAX_DEVICE_INTERFACES; devIdx++) */

   return IFX_SUCCESS;
}

/**
   Set the Listener IP configuration
*/
DTI_STATIC IFX_int_t DTI_ListenerIpConfigSet(
                        DTI_AgentCtx_t                   *pAgentCtx,
                        DTI_AgentStartupSettingsXDevs_t  *pAgentStartupSettingsXDevs)
{
   if (pAgentCtx)
   {
      DTI_conCntrlStructInit(&pAgentCtx->listenConCntrl);

      if ((DTI_StrLen(pAgentStartupSettingsXDevs->serverIpAddr) > 0))
      {
         DTI_StrCpy(pAgentCtx->listenConCntrl.ipAddr, pAgentStartupSettingsXDevs->serverIpAddr);
      }

      if (pAgentStartupSettingsXDevs->listenPort != 0)
      {
         pAgentCtx->listenConCntrl.sockPort = pAgentStartupSettingsXDevs->listenPort;
      }
      else
      {
         pAgentCtx->listenConCntrl.sockPort = DTI_TCP_PORT;
      }

      return IFX_SUCCESS;
   }

   return IFX_ERROR;
}

/**
   DTI Agent Call back to fill the buffer with the system specific informations.
*/
DTI_STATIC IFX_int_t DTI_Agent_systemInfoWrite(
                        IFX_void_t  *pUserCtx,
                        IFX_char_t  *pSysInfoBuffer,
                        IFX_int_t   bufferSize)
{
   IFX_int_t writtenLen = 0, devIdx;
   DTI_AgentCtx_t *pAgentCtx = (DTI_AgentCtx_t *)pUserCtx;

   (void)DTI_snprintf(&pSysInfoBuffer[writtenLen], bufferSize - writtenLen,
      "PacketQueueSize=%d", DTI_MAX_PACKET_QUEUE_SIZE);
   writtenLen += (IFX_int_t)DTI_StrLen(&pSysInfoBuffer[writtenLen]) + 1;

   if (pAgentCtx != IFX_NULL)
   {
      (void)DTI_snprintf(&pSysInfoBuffer[writtenLen], bufferSize - writtenLen,
         "maxClientConnections=%d", pAgentCtx->numOfUsedWorker);
      writtenLen += (IFX_int_t)DTI_StrLen(&pSysInfoBuffer[writtenLen]) + 1;

      for (devIdx = 0; devIdx < DTI_MAX_DEVICE_INTERFACES; devIdx++)
      {
         if (pAgentCtx->deviceInterface[devIdx].bConfigured == IFX_TRUE)
         {
            writtenLen += DTI_device_systemInfoWrite(
                              pAgentCtx->deviceInterface[devIdx].pDeviceAccessFct,
                              &pAgentCtx->deviceInterface[devIdx].deviceSysInfo,
                              &pSysInfoBuffer[writtenLen],
                              bufferSize - writtenLen);
         }
      }

      (void)DTI_snprintf(&pSysInfoBuffer[writtenLen], bufferSize - writtenLen,
         "NUM_DEV_IF=%d", pAgentCtx->numOfDevIf);
      writtenLen += (IFX_int_t)DTI_StrLen(&pSysInfoBuffer[writtenLen]) + 1;

      for (devIdx = 0; devIdx < DTI_MAX_DEVICE_INTERFACES; devIdx++)
      {
         if (pAgentCtx->deviceInterface[devIdx].bConfigured == IFX_TRUE)
         {
            (void)DTI_snprintf(&pSysInfoBuffer[writtenLen], bufferSize - writtenLen,
               "DEV_IF_%02d_NAME=%s",
               devIdx, pAgentCtx->deviceInterface[devIdx].ifName);
            writtenLen += (IFX_int_t)DTI_StrLen(&pSysInfoBuffer[writtenLen]) + 1;
         }
      }

#if defined(INCLUDE_CLI_SUPPORT) && (INCLUDE_CLI_SUPPORT == 1)
      writtenLen += DTI_CLI_configStrWrite(
                        pAgentCtx,
                        &pSysInfoBuffer[writtenLen],
                        bufferSize - writtenLen);
#endif
   }
   else
   {
      (void)DTI_snprintf(&pSysInfoBuffer[writtenLen], bufferSize - writtenLen,
         "Error=User Context lost");
      writtenLen += (IFX_int_t)DTI_StrLen(&pSysInfoBuffer[writtenLen]) + 1;
   }

   (void)DTI_snprintf(&pSysInfoBuffer[writtenLen], bufferSize - writtenLen,
      "DTIAgentVersion=%s", DTI_AGENT_VER_STR);
   writtenLen += (IFX_int_t)DTI_StrLen(&pSysInfoBuffer[writtenLen]) + 1;

   /* terminate */
   pSysInfoBuffer[writtenLen++] = '\0';

   return writtenLen;

}

/* Initialization of modules, calls each present module
   initialization routine */
DTI_STATIC IFX_int_t DTI_ModulesStart(
                        DTI_AgentCtx_t                   *pAgentCtx,
                        DTI_AgentStartupSettingsXDevs_t  *pAgentStartupSettingsXDevs)
{
   if (pAgentCtx == IFX_NULL)
   {
      return IFX_ERROR;
   }

   if (pDTI_systemInfoGetFunction == IFX_NULL)
   {
      /* set callback function to get the system info */
      pDTI_systemInfoGetFunction = DTI_Agent_systemInfoWrite;
   }

#if defined(INCLUDE_CLI_SUPPORT) && (INCLUDE_CLI_SUPPORT == 1)
   pAgentCtx->bControlAutoCliMsgSupport =
            (pAgentStartupSettingsXDevs->bStartupAutoCliMsgSupport) ? IFX_TRUE : IFX_FALSE;

   (void)DTI_CLI_moduleControlInit(pAgentCtx);
#endif

   return IFX_SUCCESS;
}

/*
   Finalization of modules, calls each present module its
   finalization routine */
DTI_STATIC IFX_int_t DTI_ModulesStop(
                        DTI_AgentCtx_t *pAgentCtx)
{

   if (pAgentCtx == IFX_NULL)
   {
      return IFX_ERROR;
   }

   return IFX_SUCCESS;
}


/* ============================================================================
   Global Functions
   ========================================================================= */
/**
   Start the DTI agent.
*/
IFX_int_t DTI_AgentStart(
                        DTI_AgentCtx_t             **ppDtiAgentCtx,
                        DTI_AgentStartupSettings_t *pAgentStartupSettings)
{
   IFX_int_t      retVal = IFX_SUCCESS;
   DTI_AgentStartupSettingsXDevs_t  *pAgentStartupSettingsXDevs = IFX_NULL;

   pAgentStartupSettingsXDevs = DTI_Malloc(sizeof(DTI_AgentStartupSettingsXDevs_t));
   if (pAgentStartupSettingsXDevs == IFX_NULL)
   {
      DTI_PRN_USR_ERR_NL(DTI_DBG, DTI_PRN_LEVEL_ERR,
         ("ERROR: DTI Agent Start - startup settings, not enough mem" DTI_CRLF));

      return (IFX_int_t)DTI_ERR_NO_MEMORY;
   }
   (void)DTI_AgentConvertStartupSettings(pAgentStartupSettings, pAgentStartupSettingsXDevs);

   retVal = DTI_AgentStartXDevs(ppDtiAgentCtx, pAgentStartupSettingsXDevs);

   if (pAgentStartupSettingsXDevs != IFX_NULL)
   {
      DTI_Free(pAgentStartupSettingsXDevs);
   }

   return retVal;
}


/**
   Start the DTI agent (multiple devices).
*/
IFX_int_t DTI_AgentStartXDevs(
                        DTI_AgentCtx_t                   **ppDtiAgentCtx,
                        DTI_AgentStartupSettingsXDevs_t  *pAgentStartupSettingsXDevs)
{
   IFX_int_t      retVal = IFX_SUCCESS;
   DTI_AgentCtx_t *pAgentCtx = IFX_NULL;

   if ( (ppDtiAgentCtx == IFX_NULL) || (pAgentStartupSettingsXDevs == IFX_NULL) )
   {
      DTI_PRN_USR_ERR_NL(DTI_DBG, DTI_PRN_LEVEL_ERR,
         ("ERROR: DTI Agent Start - missing context/startup pointer" DTI_CRLF));

      return (IFX_int_t)DTI_ERR_NULL_PTR_ARG;
   }
   else
   {
      DTI_PRN_USR_DBG_NL(DTI_DBG, DTI_PRN_LEVEL_HIGH,
         ("DTI Agent Start - %s" DTI_CRLF, DTI_AGENT_WHAT_STR));
   }

   if (pAgentStartupSettingsXDevs->debugLevel != 0)
   {
      DTI_DebugLevelSet((IFX_uint32_t)pAgentStartupSettingsXDevs->debugLevel);
   }

   pAgentCtx = DTI_Malloc(sizeof(DTI_AgentCtx_t));
   if (pAgentCtx == IFX_NULL)
   {
      DTI_PRN_USR_ERR_NL(DTI_DBG, DTI_PRN_LEVEL_ERR,
         ("ERROR: DTI Agent Start - not enough mem" DTI_CRLF));

      return (IFX_int_t)DTI_ERR_NO_MEMORY;
   }

   /* do basic context init */
   if (DTI_AgentCtxInit(
         pAgentCtx, pAgentStartupSettingsXDevs) != IFX_SUCCESS)
   {
      DTI_PRN_USR_ERR_NL(DTI_DBG, DTI_PRN_LEVEL_ERR,
         ("ERROR: DTI Agent Start - ctx init failed" DTI_CRLF));

      retVal = (IFX_int_t)DTI_ERR_INTERNAL;
      goto DTI_Agent_Start_ERROR;
   }

   /*
      do basic setup
   */
   (void)DTI_DeviceConfigSet(pAgentCtx, pAgentStartupSettingsXDevs);
   (void)DTI_ListenerIpConfigSet(pAgentCtx, pAgentStartupSettingsXDevs);

   if (DTI_ModulesStart(pAgentCtx, pAgentStartupSettingsXDevs) != IFX_SUCCESS)
   {
      DTI_PRN_USR_ERR_NL(DTI_DBG, DTI_PRN_LEVEL_ERR,
         ("ERROR: DTI Agent Start - start modules" DTI_CRLF));

      retVal = (IFX_int_t)DTI_ERR_INTERNAL;
      goto DTI_Agent_Start_ERROR;
   }

   if (DTI_ListenerThread_Startup (pAgentCtx) != IFX_SUCCESS)
   {
      DTI_PRN_USR_ERR_NL(DTI_DBG, DTI_PRN_LEVEL_ERR,
         ("ERROR: DTI Agent Start - start listener" DTI_CRLF));

      retVal = (IFX_int_t)DTI_ERR_CONNECTION;
      goto DTI_Agent_Start_ERROR;
   }

   *ppDtiAgentCtx = pAgentCtx;

   return IFX_SUCCESS;

DTI_Agent_Start_ERROR:

   if (IFXOS_LOCK_INIT_VALID(&pAgentCtx->dataLock))
   {
      DTI_LockDelete(&pAgentCtx->dataLock);
   }

   DTI_Free(pAgentCtx);
   *ppDtiAgentCtx = IFX_NULL;

   return retVal;
}

/**
   Stop the DTI agent.
*/
IFX_int_t DTI_AgentStop(
                        DTI_AgentCtx_t  **ppDtiAgentCtx)
{
   IFX_int_t      retVal = IFX_SUCCESS;
   DTI_AgentCtx_t *pAgentCtx = IFX_NULL;

   if (!ppDtiAgentCtx)
   {
      DTI_PRN_USR_ERR_NL(DTI_DBG, DTI_PRN_LEVEL_ERR,
         ("ERROR: DTI Agent Stop - null pointer argument" DTI_CRLF));

      return DTI_ERR_NULL_PTR_ARG;
   }

   pAgentCtx = *ppDtiAgentCtx;
   if (pAgentCtx == IFX_NULL)
   {
      DTI_PRN_USR_ERR_NL(DTI_DBG, DTI_PRN_LEVEL_ERR,
         ("ERROR: DTI Agent Stop - missing context pointer" DTI_CRLF));

      return DTI_ERR_NULL_PTR_ARG;
   }

   /*
      Here stop the Listener Thread
   */
   DTI_PRN_USR_DBG_NL(DTI_DBG, DTI_PRN_LEVEL_HIGH,
      ("DTI Agent Stop - stop listener" DTI_CRLF));

   if (DTI_ListenerThread_Stop (pAgentCtx) != IFX_SUCCESS)
   {
      DTI_PRN_USR_ERR_NL(DTI_DBG, DTI_PRN_LEVEL_ERR,
         ("ERROR: DTI Agent Stop - stop listener" DTI_CRLF));

      retVal = DTI_ERR_CONNECTION;
   }

   DTI_PRN_USR_DBG_NL(DTI_DBG, DTI_PRN_LEVEL_HIGH,
      ("DTI Agent Stop - stop module" DTI_CRLF));

   if (DTI_ModulesStop(pAgentCtx) != IFX_SUCCESS)
   {
      DTI_PRN_USR_ERR_NL(DTI_DBG, DTI_PRN_LEVEL_ERR,
         ("ERROR: DTI Agent Stop - stop module" DTI_CRLF));

      retVal = DTI_ERR_INTERNAL;
   }

   if (IFXOS_LOCK_INIT_VALID(&pAgentCtx->dataLock))
   {
      DTI_LockDelete(&pAgentCtx->dataLock);
   }

   if (retVal == IFX_SUCCESS)
   {
      DTI_Free(pAgentCtx);
      *ppDtiAgentCtx = IFX_NULL;
   }

   DTI_PRN_USR_DBG_NL(DTI_DBG, DTI_PRN_LEVEL_HIGH,
      ("DTI Agent Stop - stop done" DTI_CRLF));

   return retVal;
}

/**
   Add a device configuration

\remark
   Allow setup only if the DTI is still not started
*/
IFX_int_t DTI_DeviceConfigAdd(
                        DTI_AgentCtx_t          *pAgentCtx,
                        IFX_int_t               devIfNumber,
                        DTI_DeviceIfSettings_t  *pDeviceIfSettings)
{
   IFX_int_t   devIdx, usedDevIfNum;
   DTI_deviceInterface_t *pDeviceInterface = IFX_NULL;

   if ( (pAgentCtx == IFX_NULL) || (pDeviceIfSettings == IFX_NULL) )
   {
      DTI_PRN_USR_ERR_NL(DTI_DBG, DTI_PRN_LEVEL_ERR,
         ("ERROR: DTI Device Config Add - null pointer argument" DTI_CRLF));

      return DTI_ERR_NULL_PTR_ARG;
   }

   if (pDeviceIfSettings->pDeviceAccessFct == IFX_NULL)
   {
      DTI_PRN_USR_ERR_NL(DTI_DBG, DTI_PRN_LEVEL_ERR,
         ("ERROR: DTI Device Config Add - missing access handler" DTI_CRLF));

      return DTI_ERR_CONFIGURATION;
   }


   if (devIfNumber != -1)
   {
      if ( (devIfNumber < 0) || (devIfNumber >= DTI_MAX_DEVICE_INTERFACES) )
      {
         DTI_PRN_USR_ERR_NL(DTI_DBG, DTI_PRN_LEVEL_ERR,
            ("ERROR: DTI Device Config Add - devIfNum out of range (max = %d)" DTI_CRLF,
            DTI_MAX_DEVICE_INTERFACES));

         return DTI_ERR_CONFIGURATION;
      }
   }

   if ( (pDeviceIfSettings->linesPerDevice == 0) || (pDeviceIfSettings->numOfDevices == 0) )
   {
      DTI_PRN_USR_ERR_NL(DTI_DBG, DTI_PRN_LEVEL_ERR,
         ("ERROR: DTI Device Config Add - device not configured (num of devices / lines)" DTI_CRLF));

      return DTI_ERR_CONFIGURATION;
   }

   if ( (pDeviceIfSettings->pDeviceAccessFct->structSize_byte        != sizeof(DTI_DeviceAccessFct_t)) ||
        (pDeviceIfSettings->pDeviceAccessFct->deviceInterfaceVersion != DTI_DEVICE_INTERFACE_VERSION) )
   {
      DTI_PRN_USR_ERR_NL(DTI_DBG, DTI_PRN_LEVEL_ERR,
         ("ERROR: DTI Device Config Add - device IF missmatch" DTI_CRLF));
      DTI_PRN_USR_ERR_NL(DTI_DBG, DTI_PRN_LEVEL_ERR,
         ("\tSize IF Struct:    internal =     %8d, user =    %8d" DTI_CRLF,
          sizeof(DTI_DeviceAccessFct_t), pDeviceIfSettings->pDeviceAccessFct->structSize_byte));
      DTI_PRN_USR_ERR_NL(DTI_DBG, DTI_PRN_LEVEL_ERR,
         ("\tVersion IF Struct: internal = 0x08%X, user = 0x08%X" DTI_CRLF,
         DTI_DEVICE_INTERFACE_VERSION, pDeviceIfSettings->pDeviceAccessFct->deviceInterfaceVersion));

      return DTI_ERR_CONFIGURATION;
   }

   if (DTI_LockGet(&pAgentCtx->dataLock) != IFX_SUCCESS)
   {
      DTI_PRN_USR_ERR_NL(DTI_DBG, DTI_PRN_LEVEL_ERR,
         ("ERROR: DTI Device Config Add - lock."DTI_CRLF));

      return DTI_ERR_INTERNAL;
   }

   usedDevIfNum = devIfNumber;
   if (usedDevIfNum == -1)
   {
      for (devIdx = 0; devIdx < DTI_MAX_DEVICE_INTERFACES; devIdx++)
      {
         if (pAgentCtx->deviceInterface[devIdx].bConfigured == IFX_FALSE)
         {
            usedDevIfNum = devIdx;
            break;
         }
      }
   }

   if (usedDevIfNum == -1)
   {
      DTI_PRN_USR_ERR_NL(DTI_DBG, DTI_PRN_LEVEL_ERR,
         ("ERROR: DTI Device Config Add - all device IF's busy" DTI_CRLF));

      return DTI_ERR_CONFIGURATION;
   }
   else
   {
      pDeviceInterface = &pAgentCtx->deviceInterface[usedDevIfNum];

      if (pDeviceInterface->bConfigured == IFX_TRUE)
      {
         DTI_PRN_USR_ERR_NL(DTI_DBG, DTI_PRN_LEVEL_ERR,
            ("WARNING: DTI Device Config Add - overwrite device IF[%d] = <%s>" DTI_CRLF,
            usedDevIfNum, pDeviceInterface->ifName));

         pAgentCtx->numOfDevIf--;
      }

      DTI_MemSet(pDeviceInterface, 0x00, sizeof(DTI_deviceInterface_t));
      DTI_StrNCpy( pDeviceInterface->ifName,
                   (pDeviceIfSettings->pDeviceAccessFct->pDevIfName != IFX_NULL) ?
                     pDeviceIfSettings->pDeviceAccessFct->pDevIfName : "unknown",
                   DTI_MAX_LEN_DEVICE_INTERFACE_NAME);

      pDeviceInterface->pDeviceAccessFct = pDeviceIfSettings->pDeviceAccessFct;

      pDeviceInterface->deviceSysInfo.numOfDevs   = pDeviceIfSettings->numOfDevices;
      pDeviceInterface->deviceSysInfo.portsPerDev = pDeviceIfSettings->linesPerDevice;
      pDeviceInterface->deviceSysInfo.numOfPorts  =
         (pDeviceIfSettings->numOfDevices * pDeviceIfSettings->linesPerDevice);

      pDeviceInterface->deviceSysInfo.bControlAutoDevMsgSupport = (IFX_boolean_t)(pDeviceIfSettings->bStartupAutoDevMsgSupport);
      pDeviceInterface->deviceSysInfo.param0 = pDeviceIfSettings->param0;
      pDeviceInterface->deviceSysInfo.bValid = IFX_TRUE;
      pDeviceInterface->bConfigured = IFX_TRUE;
      pAgentCtx->numOfDevIf++;
   }

   DTI_LockRelease(&pAgentCtx->dataLock);

   return DTI_SUCCESS;
}


/**
   Remove a device configuration

\remark
   Allow setup only if the DTI is still not started
*/
IFX_int_t DTI_DeviceConfigRemove(
                        DTI_AgentCtx_t  *pAgentCtx,
                        IFX_int_t       devIfNumber,
                        IFX_char_t      *pDeviceIfName)
{
   IFX_int_t   i, usedDevIfNum;
   DTI_deviceInterface_t *pDeviceInterface = IFX_NULL;

   if (pAgentCtx == IFX_NULL)
   {
      DTI_PRN_USR_ERR_NL(DTI_DBG, DTI_PRN_LEVEL_ERR,
         ("ERROR: DTI Device Config Remove - null pointer argument" DTI_CRLF));

      return DTI_ERR_NULL_PTR_ARG;
   }

   usedDevIfNum = -1;
   if (pDeviceIfName != IFX_NULL)
   {
      for (i = 0; i < DTI_MAX_DEVICE_INTERFACES; i++)
      {
         if (pAgentCtx->deviceInterface[i].bConfigured == IFX_FALSE)
            {continue;}

         if (DTI_StrNCmp( pAgentCtx->deviceInterface[i].ifName,
                          pDeviceIfName,
                          DTI_StrLen(pAgentCtx->deviceInterface[i].ifName)) == 0)
         {
            usedDevIfNum = i;
            break;
         }
      }
   }
   else
   {
      if ((devIfNumber < 0) || (devIfNumber >= DTI_MAX_DEVICE_INTERFACES))
      {
         DTI_PRN_USR_ERR_NL(DTI_DBG, DTI_PRN_LEVEL_ERR,
            ("ERROR: DTI Device Config Remove - devIfNum out of range (max = %d)" DTI_CRLF,
            DTI_MAX_DEVICE_INTERFACES));

         return DTI_ERR_CONFIGURATION;
      }

      if (pAgentCtx->deviceInterface[devIfNumber].bConfigured == IFX_TRUE)
         {usedDevIfNum = devIfNumber;}
   }

   if (usedDevIfNum == -1)
   {
      DTI_PRN_USR_ERR_NL(DTI_DBG, DTI_PRN_LEVEL_ERR,
         ("ERROR: DTI Device Config Remove - device IF[%d] (%s) not valid or not found" DTI_CRLF,
          devIfNumber, (pDeviceIfName != IFX_NULL) ? pDeviceIfName : "unknown"));

      return DTI_ERR_CONFIGURATION;
   }


   if (DTI_LockGet(&pAgentCtx->dataLock) != IFX_SUCCESS)
   {
      DTI_PRN_USR_ERR_NL(DTI_DBG, DTI_PRN_LEVEL_ERR,
         ("ERROR: DTI Device Config Add - lock."DTI_CRLF));

      return DTI_ERR_INTERNAL;
   }

   for (i = 0; i < DTI_MAX_NUM_OF_WORKER; i++)
   {
      if (pAgentCtx->pWorker[i] != IFX_NULL)
      {
         if (pAgentCtx->pWorker[i]->thrResumeState == DTI_WORKER_THREAD_RESUME_IN_USE)
         {
            usedDevIfNum = -1;
            break;
         }
      }
   }

   if (usedDevIfNum == -1)
   {
      DTI_LockRelease(&pAgentCtx->dataLock);

      DTI_PRN_USR_ERR_NL(DTI_DBG, DTI_PRN_LEVEL_ERR,
         ("ERROR: DTI Device Config Remove - device IF[%d] (%s) in use" DTI_CRLF,
          devIfNumber, (pDeviceIfName != IFX_NULL) ? pDeviceIfName : "unknown"));

      return DTI_ERR_CONFIGURATION;
   }


   pDeviceInterface = &pAgentCtx->deviceInterface[usedDevIfNum];
   DTI_PRN_USR_ERR_NL(DTI_DBG, DTI_PRN_LEVEL_ERR,
      ("WARNING: DTI Device Config Remove - remove device IF[%d] = <%s>" DTI_CRLF,
      usedDevIfNum, pDeviceInterface->ifName));

   pAgentCtx->numOfDevIf--;
   pDeviceInterface->bConfigured = IFX_FALSE;

   DTI_MemSet(pDeviceInterface, 0x00, sizeof(DTI_deviceInterface_t));

   DTI_LockRelease(&pAgentCtx->dataLock);

   return DTI_SUCCESS;
}



/**
   Set the debug level of the DTI interface.
\remarks
   debug level = 0x------LL: use level for all
   debug level = 0xCCDDBBPP: each byte for the corresponding modules
*/
IFX_void_t DTI_DebugLevelSet(
                        IFX_uint32_t newLevel)
{
   if (newLevel & 0xFFFFFF00)
   {
      IFXOS_PRN_USR_LEVEL_SET(DTI_PROTOCOL, (newLevel & 0xF));
      IFXOS_PRN_USR_LEVEL_SET(DTI_CON, ((newLevel >> 4) & 0xF));

      IFXOS_PRN_USR_LEVEL_SET(DTI_DBG, ((newLevel >> 8) & 0xFF));
      IFXOS_PRN_USR_LEVEL_SET(DTI_DEV, ((newLevel >> 16) & 0xFF));
#  if defined(INCLUDE_CLI_SUPPORT) && (INCLUDE_CLI_SUPPORT == 1)
      IFXOS_PRN_USR_LEVEL_SET(DTI_CLI, ((newLevel >> 24) & 0xFF));
#  endif
   }
   else
   {
      IFXOS_PRN_USR_LEVEL_SET(DTI_PROTOCOL, (newLevel & 0xFF));
      IFXOS_PRN_USR_LEVEL_SET(DTI_DBG, (newLevel & 0xFF));
      IFXOS_PRN_USR_LEVEL_SET(DTI_DEV, (newLevel & 0xFF));
#  if defined(INCLUDE_CLI_SUPPORT) && (INCLUDE_CLI_SUPPORT == 1)
      IFXOS_PRN_USR_LEVEL_SET(DTI_CLI, (newLevel & 0xFF));
#  endif
   }

   return;
}

