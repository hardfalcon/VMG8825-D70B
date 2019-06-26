/******************************************************************************

                              Copyright (c) 2014
                            Lantiq Deutschland GmbH

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/**
   \file drv_tapi_qos.c
   Implements the handling of the QOS driver registration and ioctls.
*/

/* ============================= */
/* Includes                      */
/* ============================= */
#include "drv_tapi.h"
#include <net/sock.h>

#ifdef TAPI_FEAT_QOS

#ifndef TAPI_FEAT_KPI
#error This feature requires KPI to be enabled.
#endif /* !TAPI_FEAT_KPI */

#include "drv_tapi_qos.h"
#include "drv_tapi_qos_io.h"
#include "drv_tapi_qos_ll_interface.h"
#include "drv_tapi_errno.h"

/* ============================= */
/* Local definitions             */
/* ============================= */

/* ============================= */
/* Local structures              */
/* ============================= */

/* ============================= */
/* Local variable definition     */
/* ============================= */
/** Pointer to registered QOS driver context. */
static IFX_TAPI_DRV_CTX_QOS_t *gpQosCtx = IFX_NULL;
/** QOS driver context protection */
static TAPI_OS_mutex_t        semProtectQosCtx;

/** global variable used to configure via insmod option */
extern IFX_int32_t block_egress_tasklet;

/* ============================= */
/* Global variable definition    */
/* ============================= */

/* ============================= */
/* Local function declaration    */
/* ============================= */

/* ============================= */
/* Local function definition     */
/* ============================= */

/* ============================= */
/* Global function definition    */
/* ============================= */

/** This service creates a new session on a channel.
    This version takes IPv4 addresses and ports to create an socket itself.

   \param  pChannel  Handle to TAPI_CHANNEL structure.
   \param  pInit     Handle to QOS_INIT_SESSION structure.

   \return Returns value as follows:
   - TAPI_statusOk: Starting QOS successful
   - TAPI_statusNoQosRegistered: No QOS driver registered - IOCTL aborted
   - TAPI_statusQosStartFailed: QOS start failed
*/
IFX_int32_t IFX_TAPI_QOS_Start (TAPI_CHANNEL* pChannel,
                                QOS_INIT_SESSION const *pInit)
{
   IFX_int16_t nKpiChannel;
   IFX_int32_t ret = TAPI_statusOk;
   IFX_int32_t retQOS = TAPI_statusOk;

   /* verify that caller of this function passes only safe parameters */
   TAPI_ASSERT(pChannel != IFX_NULL);
   TAPI_ASSERT(pInit != IFX_NULL);

   TRACE(TAPI_DRV, DBG_LEVEL_LOW, ("Qos_StartSession\n"));

   /* protect access to global variable to prevent deregistering while this
      ioctl is called */
   TAPI_OS_MutexGet (&semProtectQosCtx);

   if (gpQosCtx == IFX_NULL)
   {
      TAPI_OS_MutexRelease (&semProtectQosCtx);

      /* errmsg: No QOS driver registered - IOCTL aborted */
      RETURN_STATUS (TAPI_statusNoQosRegistered, 0);
   }

   /* protect channel-data from mutual access */
   TAPI_OS_MutexGet (&pChannel->semTapiChDataLock);

   nKpiChannel = IFX_TAPI_KPI_ChGet(pChannel, IFX_TAPI_KPI_STREAM_COD);

   /* release channel-data protection */
   TAPI_OS_MutexRelease (&pChannel->semTapiChDataLock);

   /* Is KPI already configured on this channel? */
   if ((nKpiChannel & 0xF000) != IFX_TAPI_KPI_UDP)
   {
      /* Get a free channel in the QOS driver. We will then map the traffic
         of this channel to this KPI channel in KPI group IFX_TAPI_KPI_UDP. */
      nKpiChannel = gpQosCtx->getFreeChannel();
      if (nKpiChannel >= 0)
      {
         IFX_TAPI_KPI_CH_CFG_t  cfg;

         /* Configure KPI */
         cfg.nStream = IFX_TAPI_KPI_STREAM_COD;
         cfg.nKpiCh = IFX_TAPI_KPI_UDP | nKpiChannel;
         ret = IFX_TAPI_KPI_ChCfgSet (pChannel, &cfg);
      }
      else
      {
         ret = IFX_ERROR;
      }
   }
   else
   {
      /* Remove KPI group from channel */
      nKpiChannel &= 0x0FFF;
   }

   if (TAPI_SUCCESS (ret))
   {
       if (pInit->family == AF_INET)
       {
      /* initiate a session */
            retQOS = gpQosCtx->start((IFX_TAPI_KPI_CH_t)nKpiChannel,
                                     pInit->family,
                                     (IFX_void_t*)&pInit->ipv4.srcAddr, pInit->srcPort,
                                     (IFX_void_t*)&pInit->ipv4.destAddr, pInit->destPort,

                                     pInit->do_srtp,
                                     pInit->ssrc,
                                     pInit->eSRTP,
                                     pInit->eSRTP_Auth,
                                     pInit->nSRTP_AuthFieldLength,
                                     pInit->localKey,
                                     pInit->localSalt,
                                     pInit->remoteKey,
                                     pInit->remoteSalt);
        }
        else if (pInit->family == AF_INET6)
        {
            /* initiate a session */
            retQOS = gpQosCtx->start((IFX_TAPI_KPI_CH_t)nKpiChannel,
                                     pInit->family,
                                     (IFX_void_t*)pInit->ipv6.srcAddr, pInit->srcPort,
                                     (IFX_void_t*)pInit->ipv6.destAddr, pInit->destPort,

                                     pInit->do_srtp,
                                     pInit->ssrc,
                                     pInit->eSRTP,
                                     pInit->eSRTP_Auth,
                                     pInit->nSRTP_AuthFieldLength,
                                     pInit->localKey,
                                     pInit->localSalt,
                                     pInit->remoteKey,
                                     pInit->remoteSalt);
        }
        else
        {
            retQOS = TAPI_statusParam;
        }

      if (!TAPI_SUCCESS (retQOS))
      {
         /* errmsg: QOS start failed */
         ret = TAPI_statusQosStartFailed;
      }
   }

   TAPI_OS_MutexRelease (&semProtectQosCtx);

   if (!TAPI_SUCCESS (ret))
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("QOS session NOT started.\n"));
   }

   RETURN_STATUS(ret, retQOS);
} /* Qos_StartSession() */


/** This service creates a new session on a channel.
    This version is used to create a session via an existing socket fd.

   \param  pChannel  Handle to TAPI_CHANNEL structure.
   \param  pInit     Handle to QOS_INIT_SESSION_ON_SOCKET structure.

   \return Returns value as follows:
   - TAPI_statusOk: Starting QOS successful
   - TAPI_statusNoQosRegistered: No QOS driver registered - IOCTL aborted
   - TAPI_statusQosServiceNotSupported: Service not supported by QOS driver
   - TAPI_statusQosStartFdFailed: QOS start on existing socket failed
*/
IFX_int32_t IFX_TAPI_QOS_SessionOnSocketStart(TAPI_CHANNEL* pChannel,
   QOS_INIT_SESSION_ON_SOCKET const *pInit)
{
   IFX_int16_t nKpiChannel;
   IFX_int32_t ret = TAPI_statusOk;
   IFX_int32_t retQOS = TAPI_statusOk;

   /* verify that caller of this function passes only safe parameters */
   TAPI_ASSERT(pChannel != IFX_NULL);
   TAPI_ASSERT(pInit != IFX_NULL);

   TRACE(TAPI_DRV, DBG_LEVEL_LOW, ("Qos_StartSessionOnSocket\n"));

   /* protect access to global variable to prevent deregistering while this
      ioctl is called */
   TAPI_OS_MutexGet (&semProtectQosCtx);

   if (gpQosCtx == IFX_NULL)
   {
      TAPI_OS_MutexRelease (&semProtectQosCtx);

      /* errmsg: No QOS driver registered - IOCTL aborted */
      RETURN_STATUS (TAPI_statusNoQosRegistered, 0);
   }

   /* This is an extension so check if this service is supported. */
   if (gpQosCtx->start_fd == IFX_NULL)
   {
      TAPI_OS_MutexRelease (&semProtectQosCtx);

      /*errmsg: QOS: Service not supported by QOS driver - IOCTL failed */
      RETURN_STATUS(TAPI_statusQosServiceNotSupported, 0);
   }

   /* protect channel-data from mutual access */
   TAPI_OS_MutexGet (&pChannel->semTapiChDataLock);

   nKpiChannel = IFX_TAPI_KPI_ChGet(pChannel, IFX_TAPI_KPI_STREAM_COD);

   /* release channel-data protection */
   TAPI_OS_MutexRelease (&pChannel->semTapiChDataLock);

   /* Is KPI already configured on this channel? */
   if ((nKpiChannel & 0xF000) != IFX_TAPI_KPI_UDP)
   {
      /* Get a free channel in the QOS driver. We will then map the traffic
         of this channel to this KPI channel in KPI group IFX_TAPI_KPI_UDP. */
      nKpiChannel = gpQosCtx->getFreeChannel();
      if (nKpiChannel >= 0)
      {
         IFX_TAPI_KPI_CH_CFG_t  cfg;

         /* Configure KPI */
         cfg.nStream = IFX_TAPI_KPI_STREAM_COD;
         cfg.nKpiCh = IFX_TAPI_KPI_UDP | nKpiChannel;
         ret = IFX_TAPI_KPI_ChCfgSet (pChannel, &cfg);
      }
      else
      {
         ret = IFX_ERROR;
      }
   }
   else
   {
      /* Remove KPI group from channel */
      nKpiChannel &= 0x0FFF;
   }

   if (TAPI_SUCCESS (ret))
   {
      /* initiate a session */
      retQOS = gpQosCtx->start_fd((IFX_TAPI_KPI_CH_t)nKpiChannel, pInit->fd);

      if (!TAPI_SUCCESS (retQOS))
      {
         /* errmsg: QOS start on existing socket failed */
         ret = TAPI_statusQosStartFdFailed;
      }
   }

   TAPI_OS_MutexRelease (&semProtectQosCtx);

   if (!TAPI_SUCCESS (ret))
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("QOS session NOT started.\n"));
   }

   RETURN_STATUS(ret, retQOS);
} /* Qos_StartSession() */


/** Deactivate and delete a session on a channel.

   \param  pChannel       Handle to TAPI_CHANNEL structure.

   \return Returns value as follows:
   - TAPI_statusOk: Starting QOS successful
   - TAPI_statusNoQosRegistered: No QOS driver registered - IOCTL aborted
   - TAPI_statusQosStopFailed: QOS stop failed
*/
IFX_int32_t IFX_TAPI_QOS_Stop (TAPI_CHANNEL *pChannel)
{
   IFX_TAPI_KPI_CH_t nKpiChannel;
   IFX_int32_t ret = TAPI_statusOk;
   IFX_int32_t retQOS = TAPI_statusOk;

   /* verify that caller of this function passes only safe parameters */
   TAPI_ASSERT(pChannel != IFX_NULL);

   TRACE(TAPI_DRV, DBG_LEVEL_LOW, ("Qos_StopSession()\n"));

   /* protect access to global variable to prevent deregistering while this
      ioctl is called */
   TAPI_OS_MutexGet (&semProtectQosCtx);

   if (gpQosCtx == IFX_NULL)
   {
      TAPI_OS_MutexRelease (&semProtectQosCtx);

      /* errmsg: No QOS driver registered - IOCTL aborted */
      RETURN_STATUS (TAPI_statusNoQosRegistered, 0);
   }

   /* protect channel-data from mutual access */
   TAPI_OS_MutexGet (&pChannel->semTapiChDataLock);

   nKpiChannel = IFX_TAPI_KPI_ChGet(pChannel, IFX_TAPI_KPI_STREAM_COD);

   /* release channel-data protection */
   TAPI_OS_MutexRelease (&pChannel->semTapiChDataLock);

   /* Is KPI configured for UDP group on this channel? */
   if ((nKpiChannel & 0xF000) == IFX_TAPI_KPI_UDP)
   {
      IFX_TAPI_KPI_CH_CFG_t  cfg;

      /* Remove KPI group from channel */
      nKpiChannel &= 0x0FFF;
      /* deactivate the session */
      retQOS = gpQosCtx->stop(nKpiChannel);

      if (!TAPI_SUCCESS (retQOS))
      {
         /* errmsg: QOS stop failed */
         ret = TAPI_statusQosStopFailed;
      }
      else
      {
         /* Unconfigure KPI */
         cfg.nStream = IFX_TAPI_KPI_STREAM_COD;
         cfg.nKpiCh = IFX_TAPI_KPI_GROUP0;
         ret = IFX_TAPI_KPI_ChCfgSet (pChannel, &cfg);
      }
   }

   TAPI_OS_MutexRelease (&semProtectQosCtx);

   if (!TAPI_SUCCESS (ret))
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("QOS session NOT stopped.\n"));
   }

   RETURN_STATUS(ret, retQOS);
}


/**
   Cleanup every ressource used by QOS.

   \return Returns value as follows:
   - TAPI_statusNoQosRegistered: No QOS driver registered - IOCTL aborted
*/
IFX_int32_t IFX_TAPI_QOS_Clean (void)
{
   IFX_int32_t ret = TAPI_statusOk;
   IFX_int32_t retQOS = TAPI_statusOk;

   TRACE(TAPI_DRV, DBG_LEVEL_LOW, ("Qos_Cleanup()\n"));

   /* protect access to global variable to prevent deregistering while this
      ioctl is called */
   TAPI_OS_MutexGet (&semProtectQosCtx);

   if (gpQosCtx == IFX_NULL)
   {
      TAPI_OS_MutexRelease (&semProtectQosCtx);

      /* errmsg: No QOS driver registered - IOCTL aborted */
      return TAPI_statusNoQosRegistered;
   }

   /* Clear all filter tables. */
   retQOS = gpQosCtx->clean();

   TAPI_OS_MutexRelease (&semProtectQosCtx);

   if (!TAPI_SUCCESS (retQOS))
   {
      /* errmsg: QOS cleanup failed */
      ret = TAPI_statusQosCleanupFailed;
   }

   return ret;
}


/**
   Initialise the QOS service

   \return
   - IFX_SUCCESS  in all cases
*/
IFX_int32_t IFX_TAPI_QOS_Init (void)
{
   /* create QOS driver context protection semaphore */
   TAPI_OS_MutexInit (&semProtectQosCtx);

   return IFX_SUCCESS;
}


/**
   Clean-up the QOS service

   \return none
*/
IFX_void_t IFX_TAPI_QOS_Cleanup (void)
{
   /* delete QOS driver context protection semaphore */
   TAPI_OS_MutexDelete (&semProtectQosCtx);
}


/**
   Register a QOS driver in TAPI

   This function is exported to be called by the QOS driver.

   \param  pQosCtx      Pointer to QOS driver context. IFX_NULL to unregister.

   \return IFX_SUCCESS or IFX_ERROR.
*/
IFX_return_t IFX_TAPI_QOS_DrvRegister (IFX_TAPI_DRV_CTX_QOS_t *pQosCtx)
{

   if (pQosCtx != IFX_NULL)
   {
      IFX_size_t l = strlen(pQosCtx->InterfaceVersion);

      /* Make sure the interface versions are matching before registering. */
      if ((l != strlen(DRV_QOS_INTERFACE_VER_STR)) ||
         strncmp (pQosCtx->InterfaceVersion, DRV_QOS_INTERFACE_VER_STR, l))
      {
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
            ("TAPI: ATTENTION - mismatch of QOS driver Interface\n"));
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
            ("TAPI: please check that drv_tapi and drv_%s driver match.\n",
             pQosCtx->drvName));
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
            ("Version set in QOS Driver = %.10s\n", pQosCtx->InterfaceVersion));
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
            ("Version expected by TAPI  = %.10s\n", DRV_QOS_INTERFACE_VER_STR));
         return IFX_ERROR;
      }
   }

   /* protect access to global variable to prevent deregistering while an
      ioctl is called */
   TAPI_OS_MutexGet (&semProtectQosCtx);

   /* On unregistration clear the tasklet address. As long as the address
      exists it will be called when a packet arrives in the KPI group. */
   if (pQosCtx == IFX_NULL)
   {
      IFX_TAPI_KPI_EgressTaskletRegister(IFX_TAPI_KPI_UDP, IFX_NULL);
   }

   /* store the driver context */
   gpQosCtx = pQosCtx;

   /* optional: register KPI egress tasklet - if implemented */
   if (pQosCtx != IFX_NULL && pQosCtx->pQosEgressTasklet)
   {
      if (!block_egress_tasklet)
         IFX_TAPI_KPI_EgressTaskletRegister(IFX_TAPI_KPI_UDP,
                                            pQosCtx->pQosEgressTasklet);
   }

   TAPI_OS_MutexRelease (&semProtectQosCtx);

   return IFX_SUCCESS;
}


#ifdef TAPI_FEAT_PROCFS
/**
   Linux proc filesystem support to print the driver registration information.

   This function is exported to be called by the linux proc handler.

   \param  s

   \return
   none
*/
extern IFX_void_t IFX_TAPI_QOS_proc_read_registration (struct seq_file *s)
{
   /* protect access to global variable to prevent deregistering while data
      is printed */
   TAPI_OS_MutexGet (&semProtectQosCtx);

   if (gpQosCtx != IFX_NULL)
   {
      seq_printf(s,
         "Name: %s  IF-version: %s  Use egress tasklet: %s\n",
         gpQosCtx->drvName, gpQosCtx->InterfaceVersion,
         (gpQosCtx->pQosEgressTasklet && !block_egress_tasklet) ?
         "yes" : "NO!");
   }
   else
   {
      seq_printf(s, "- no QOS driver registered -\n");
   }

   TAPI_OS_MutexRelease (&semProtectQosCtx);
}
#endif /* TAPI_FEAT_PROCFS */

#endif /* TAPI_FEAT_QOS */
