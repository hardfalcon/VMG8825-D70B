/******************************************************************************

                              Copyright (c) 2014
                            Lantiq Deutschland GmbH

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/* ============================================================================
   Description : TAPI Driver, DEV_IO part
   ========================================================================= */

#if (defined(IFXOS_USE_DEV_IO) && (IFXOS_USE_DEV_IO == 1))
/* ============================================================================
   Inlcudes
   ========================================================================= */

#include <ifx_types.h>
#include <ifxos_std_defs.h>
#include <ifxos_select.h>
#include "drv_tapi.h"
#include "drv_tapi_errno.h"
#include "drv_tapi_ioctl.h"
#include <ifxos_device_access.h>

/* get at first the driver configuration */
#include <ifxos_device_io.h>
#if (TAPI_SUPPORT_IRQ == 1)
#include <ifxos_interrupt.h>
#if XAPI
#include <BspExport/board.h>
#endif
#endif

/* ============================================================================
   Local Macros & Definitions
   ========================================================================= */

#ifdef TAPI_STATIC
#undef TAPI_STATIC
#endif

#ifdef TAPI_DEBUG
#define TAPI_STATIC
#else
#define TAPI_STATIC   static
#endif

#define TAPI_DEFER_THREAD_STACK_SIZE  10000

/* ============================================================================
   typedefs DEV IO Control Structures
   ========================================================================= */

/** Generic structure for ioctl addressing. */
typedef struct
{
   /** Device index */
   IFX_uint16_t dev;
   /** Channel "module" index */
   IFX_uint16_t ch;
   /** Any parameter used by ioctls */
   IFX_uint32_t param;
} IFX_TAPI_IOCTL_t;

typedef struct TAPI_DEVIO_CtrlDevHandle_s TAPI_DEVIO_DevHandle_t;
/**
   DEV IO specific data for control a TAPI Control device
*/
struct TAPI_DEVIO_CtrlDevHandle_s
{
   /** driver device number */
   IFX_int_t         devNum;
   /* Major Driver Number     */
   IFX_int32_t         nMajorNum;
   /* Interface Semaphore      */
   TAPI_OS_mutex_t     oSemDrvIF;
   TAPI_DEVIO_DevHandle_t *pNext;
};

/* just used for cast safe return from open */
typedef union
{
   IFX_long_t nAdr;
   TAPI_DEVIO_DevHandle_t *pDevHandle;
}TAPI_DEVIO_Ctx_t;

/*
    Global device driver structure.
    IOS Device driver Context.
*/
typedef struct
{
   /* Device Driver Number     */
   IFX_int32_t         nDrvNum;
   /* Major Driver Number     */
   IFX_int32_t         nMajorNum;
   /* Max device               */
   IFX_int32_t         nMaxDev;
   /* Max device channel       */
   IFX_int32_t         nMaxChn;
   TAPI_DEVIO_DevHandle_t *pDevList;
}TAPI_DEVIO_DevCtx;

/** function type: ISR DEV IO */
/*lint -esym(751, TAPI_IFXInterruptRoutine_Fct) */
typedef void (*TAPI_IFXInterruptRoutine_Fct)(IFX_int_t);

/* ============================================================================
   Local variables
   ========================================================================= */

/* TAPI Control Dev Driver number */
TAPI_STATIC IFX_int_t TAPI_DevDrvNum = 0;
/* The Global device driver structure */
TAPI_DEVIO_DevCtx TAPI_devCtx;

/* ============================================================================
   Global Functions Declaration
   ========================================================================= */

IFX_int_t TAPI_ModuleDelete(void);

/* ============================================================================
   Local Functions Declaration
   ========================================================================= */

TAPI_STATIC IFX_long_t TAPI_DevOpen (void *device, const IFX_char_t *appendix);
TAPI_STATIC IFX_int_t TAPI_DevClose (void *pprivate);
TAPI_STATIC IFX_int_t TAPI_DevPoll  (void *pprivate);
TAPI_STATIC IFX_int_t TAPI_DevIoctl (void *pprivate, IFX_uint_t nCmd,
                                     IFX_ulong_t nArg);

/* ============================================================================
   TAPI driver  - interrupt handling
   ========================================================================= */

/* ============================================================================
   TAPI driver  - OS file device handling
   ========================================================================= */

/**
   Open a TAPI instance to control the TAPI device and its channels

\param
   device      private device data
\param
   appendix    remaing part of string used during open()

\return
   ERROR - on failure,
   non zero value on success
*/
TAPI_STATIC IFX_long_t TAPI_DevOpen (void *device, const IFX_char_t  *appendix)
{
   TAPI_DEVIO_DevHandle_t *pDevHandle = (TAPI_DEVIO_DevHandle_t *)device;
   IFX_TAPI_DRV_CTX_t      *pDrvCtx    = IFX_NULL;
   TAPI_DEVIO_Ctx_t ctx;

   pDevHandle = (TAPI_DEVIO_DevHandle_t *)device;
   /* Get the pointer to the device driver context based on the major number */
   pDrvCtx = IFX_TAPI_DeviceDriverContextGet(pDevHandle->nMajorNum);
   /* protect against concurrent access */
   TAPI_OS_MutexGet (&pDevHandle->oSemDrvIF);

   if (pDrvCtx == IFX_NULL)
   {
      /* This should actually never happen because the file descriptors are
         registered in TAPI_OS_RegisterLLDrv after a driver context is kn-own. */
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("Err getting pDrvCtx."
               "(File: %s, line: %d)\n", __FILE__, __LINE__));
      return (IFX_long_t) IFX_NULL;
   }
   /* store the first TAPI device pointer and return by open */
   ctx.pDevHandle = pDevHandle;

   /* Call the Low level Device specific open routine */
   if (IFX_TAPI_PtrChk (pDrvCtx->Open))
   {
      IFX_int32_t retLL;

      retLL = pDrvCtx->Open (pDrvCtx->pTapiDev->pLLDev);

      if (!TAPI_SUCCESS(retLL))
      {
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
            ("DRV_TAPI ERROR: Open LL channel failed.\n"));
         ctx.pDevHandle = IFX_NULL;
      }
   }

   /* release lock */
   TAPI_OS_MutexRelease (&pDevHandle->oSemDrvIF);
#ifndef TAPI_SINGLE_THREAD
   /* Single event wait queue per TAPI system */
   /*TAPI_OS_DrvSelectQueueInit(&ctx.pInstance->eventWakeupQueue);*/
#endif
   /* Return the dynamic control data block - assign to the new fd. */
   return ctx.nAdr;
}

/**
   Close a TAPI device.

\param
   pprivate    private device data

\return
   IFX_SUCCESS - on success.
   IFX_ERROR   - if privat dynamic data has be lost.
*/
TAPI_STATIC IFX_int_t TAPI_DevClose(void *device)
{
   TAPI_DEVIO_DevHandle_t *pDevHandle = (TAPI_DEVIO_DevHandle_t *)device;
   IFX_TAPI_DRV_CTX_t *pDrvCtx = IFX_NULL;
   /*TAPI_DEVIO_DevCtx* pDevCtx;*/

   if (pDevHandle == IFX_NULL)
   {
      return IFX_ERROR;
   }
   TAPI_OS_MutexGet (&pDevHandle->oSemDrvIF);
   /* Get the pointer to the device driver context based on the major number */
   pDrvCtx = IFX_TAPI_DeviceDriverContextGet(pDevHandle->nMajorNum);

   TAPI_OS_MutexRelease (&pDevHandle->oSemDrvIF);
   /* Call the Low-level Device specific release routine. */
   /* Not having such a function is not an error. */
   if (pDrvCtx && IFX_TAPI_PtrChk (pDrvCtx->Release))
   {
      IFX_int32_t retLL;

      retLL = pDrvCtx->Release (pDrvCtx->pTapiDev->pLLDev);

      if (!TAPI_SUCCESS (retLL))
      {
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
            ("DRV_TAPI ERROR: Release LL channel failed\n"));
         return IFX_ERROR;
      }
   }
   return IFX_SUCCESS;
}


/**
   Poll/Select support

\param
   pprivate    private dynamic control data (per open instance)

\return
   0:    no data available
   1:    data available
   -1:   ERROR
*/
TAPI_STATIC IFX_int_t TAPI_DevPoll (void  *device)
{
   TAPI_DEVIO_DevHandle_t *pDevHandle = (TAPI_DEVIO_DevHandle_t *)device;
   IFX_TAPI_DRV_CTX_t* pDrvCtx = IFX_NULL;
   TAPI_DEV* pTapiDev;
   IFX_int_t i = 0, j = 0;

   pDrvCtx = IFX_TAPI_DeviceDriverContextGet (pDevHandle->nMajorNum);
   if (pDrvCtx == IFX_NULL)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("Err getting device driver."
            "(File: %s, line: %d)\n", __FILE__, __LINE__));
      return IFX_ERROR;
   }


   for (j = 0;j < pDrvCtx->maxDevs; ++j)
   {
      pTapiDev = &pDrvCtx->pTapiDev[j];
      pTapiDev->bNeedWakeup = IFX_TRUE;

      if (pTapiDev->pChannel == IFX_NULL)
         continue;

      for (i = 0; i < pTapiDev->nMaxChannel; i++)
      {
         TAPI_CHANNEL *pTapiCh = &(pTapiDev->pChannel[i]);
         if (pTapiCh != IFX_NULL && !IFX_TAPI_EventFifoEmpty(pTapiCh))
         {
            /* at least one event available so return action */
            pTapiDev->bNeedWakeup = IFX_FALSE;
            return 1;
         }
      }
   }
   return 0;
}

/**
   Configuration / Control for the device.

\param
   pprivate - private dynamic control data (per open instance)
\param
   cmd      - function id's
\param
   arg      - optional argument

\return
   OK and positive values - success,
   negative value - ioctl failed
*/
TAPI_STATIC IFX_int_t TAPI_DevIoctl (void *device,
                              IFX_uint_t  nCmd, IFX_ulong_t nArg)
{
   TAPI_DEVIO_DevHandle_t *pDevHandle = (TAPI_DEVIO_DevHandle_t *)device;
   IFX_TAPI_DRV_CTX_t* pDrvCtx = IFX_NULL;
   IFX_TAPI_ioctlCtx_t ctx;
   IFX_int32_t ret;

   pDrvCtx = IFX_TAPI_DeviceDriverContextGet (pDevHandle->nMajorNum);
   if (pDrvCtx == IFX_NULL)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("Err getting device driver."
            "(File: %s, line: %d)\n", __FILE__, __LINE__));
      return IFX_ERROR;
   }

   ret = TAPI_ioctlContextGet (pDrvCtx, 0, nCmd, nArg, IFX_FALSE, &ctx);
   if (IFX_SUCCESS == ret)
   {
      ret = TAPI_Ioctl (&ctx);
   }

   TAPI_ioctlContextPut (nArg, &ctx);

   return ret;
}


/* ============================================================================
   TAPI driver - module setup.
   ========================================================================= */

/**
   Register the low level driver with the operating system

   This function will be called when the low-level driver is added to the
   system. Any OS specific registration or set-up should be done here.

   \param  pLLDrvCtx    Pointer to device driver context created by LL-driver.
   \param  pHLDrvCtx    Pointer to high-level driver context.

   \return
   TAPI_statusOk

   \remarks
*/
IFX_int32_t TAPI_OS_RegisterLLDrv (IFX_TAPI_DRV_CTX_t* pLLDrvCtx,
                                    IFX_TAPI_HL_DRV_CTX_t* pHLDrvCtx)
{
   IFX_char_t     buf[64];
   TAPI_DEVIO_DevCtx* pDevCtx;
   TAPI_DEVIO_DevHandle_t *pDevHandle;

   pDevCtx = &TAPI_devCtx;
   if (TAPI_devCtx.pDevList == IFX_NULL)
   {
      /* special case for first entry */
      pDevCtx->pDevList = (TAPI_DEVIO_DevHandle_t *)TAPI_OS_Malloc(sizeof(TAPI_DEVIO_DevHandle_t));
      pDevHandle = pDevCtx->pDevList;
      pDevHandle->pNext = IFX_NULL;
   }
   else
   {
      pDevHandle = TAPI_devCtx.pDevList;
      while (pDevHandle->pNext != IFX_NULL)
         pDevHandle = pDevHandle->pNext;
      /* now pNext is free (=null) and we can use it */
      pDevHandle->pNext = (TAPI_DEVIO_DevHandle_t *)TAPI_OS_Malloc(sizeof(TAPI_DEVIO_DevHandle_t));
      pDevHandle = pDevHandle->pNext;
      pDevHandle->pNext = IFX_NULL;
   }
   pDevHandle->nMajorNum = pLLDrvCtx->majorNumber;
   TAPI_OS_MutexInit (&pDevHandle->oSemDrvIF);

   /* The device driver is declared to the IOS: Now we add to the device
      driver IOS list the devices */
   memset (buf, 0, sizeof(buf));
#ifdef TAPI_ONE_DEVNODE
   sprintf(buf, "/dev/%s", pLLDrvCtx->devNodeName);

   /* add control device 0 (only cntrl0 !!!) to the OS */
   if (DEVIO_device_add (pDevHandle, buf,
         (unsigned int)TAPI_DevDrvNum ) == (unsigned int)-1)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
            ("TAPI_DRV: ERROR - ModuleCreate, basic register - add control <%s>" TAPI_CRLF,
               buf));
      return IFX_ERROR;
   }
   TRACE(TAPI_DRV, DBG_LEVEL_NORMAL,
            ("TAPI_DRV: ModuleCreate, basic register - add <%s> done" TAPI_CRLF,
              buf));
#endif /* TAPI_ONE_DEVNODE */

   return TAPI_statusOk;
}

/**
   UnRegister the low level driver from the operating system

   This function will be called when the low-level driver is removed from the
   system. Any OS specific deregistration should be done here.

   \param  pLLDrvCtx    Pointer to device driver context created by LL-driver.
   \param  pHLDrvCtx    Pointer to high-level driver context.

   \return
   TAPI_statusErr on error, otherwise TAPI_statusOk
*/
IFX_int32_t TAPI_OS_UnregisterLLDrv (IFX_TAPI_DRV_CTX_t* pLLDrvCtx,
                                      IFX_TAPI_HL_DRV_CTX_t* pHLDrvCtx)
{
   TAPI_DEVIO_DevCtx* pDevCtx;
   TAPI_DEVIO_DevHandle_t *pDevHandle, *pThis;

   pDevCtx = &TAPI_devCtx;
   if (pDevCtx->pDevList != IFX_NULL)
   {
      pDevHandle = pDevCtx->pDevList;
      if (pDevHandle->nMajorNum == pLLDrvCtx->majorNumber)
      {
         pThis = pDevHandle;
         pDevCtx->pDevList = pDevHandle->pNext;
      }
      else
      {
         pThis = IFX_NULL;
         while (pDevHandle->pNext != IFX_NULL)
         {
            if (pDevHandle->pNext->nMajorNum == pLLDrvCtx->majorNumber)
            {
               pThis = pDevHandle->pNext;
               /* remove this pointer from the list */
               pDevHandle->pNext = pThis->pNext;
            }
         }
      }
      if (pThis != IFX_NULL)
      {
         TAPI_OS_MutexDelete (&pThis->oSemDrvIF);
         DEVIO_device_delete (pThis);
         TAPI_OS_Free (pThis);
      }
   }

   return TAPI_statusOk;

}

/**
   Create a TAPI driver module.

\return
   IFX_SUCCESS or IFX_ERROR

*/
IFX_int_t TAPI_ModuleCreate(void)
{
   IFX_int_t ret = IFX_SUCCESS;

   TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
          ("%s (c) Copyright 2010-2014, Lantiq GmbH"
          TAPI_CRLF, &TAPI_WHATVERSION[4]));

   if (TAPI_DevDrvNum != 0)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
            ("TAPI Dev Module Create already done" TAPI_CRLF));
      return ret;
   }
   memset(&TAPI_devCtx, 0x00, sizeof(TAPI_DEVIO_DevCtx));

   SetTraceLevel (TAPI_DRV, DBG_LEVEL_HIGH);
   if (TAPI_DevDrvNum <= 0)
   {
      TAPI_DevDrvNum = (IFX_int_t)DEVIO_driver_install(
                                   TAPI_DevOpen,
                                   TAPI_DevClose,
                                   IFX_NULL,
                                   IFX_NULL,
                                   TAPI_DevIoctl,
                                   TAPI_DevPoll);

      if (TAPI_DevDrvNum == IFX_ERROR)
      {
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
               ("TAPI_DRV[--]: ERROR - ModuleCreate, basic register - "
                "install line dev driver" TAPI_CRLF));

         return IFX_ERROR;
      }
   }
   else
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
             ("TAPI_DRV[--]: line device driver already installed." TAPI_CRLF));
      return (IFX_ERROR);
   }

   if (IFX_TAPI_Driver_Start() != IFX_SUCCESS)
   {
      TAPI_ModuleDelete();
      return ret;
   }

   return ret;
}


/**
   Remove the TAPI Control Driver .

\return
   IFX_SUCCESS successful, all freed and driver removed
   IFX_ERROR   not successful, devices still busy
*/
IFX_int_t TAPI_ModuleDelete(void)
{

   if ( (TAPI_DevDrvNum != -1) )
   {
      /*
         Release device from OS
      */
   }
   /* remove driver from OS */
   DEVIO_driver_remove ( (unsigned int) TAPI_DevDrvNum, 1 );

   return IFX_SUCCESS;
}

/* --------------------------------------------------------------------------
                             EVENT handling  -->  BEGIN
   --------------------------------------------------------------------------
*/

#ifndef TAPI_SINGLE_THREAD

/**
   Function to be executed from a thread to serve deferred work.

   This function calls deferred function.

   \param  *pThread     Pointer to thread parameters
   \return  IFX_SUCCESS
*/
static IFX_int32_t ifx_tapi_defer_thread (TAPI_OS_ThreadParams_t *pThread)
{
   IFX_int32_t (*Func)(IFX_void_t*) = (IFX_void_t*)pThread->nArg1;

   Func((IFX_void_t*)pThread->nArg2);
   /* Auto-remove of thread,  delete thread will updates variables
    * to allow start it again */
   IFXOS_ThreadDelete(&((IFX_TAPI_EXT_EVENT_PARAM_t *)
         (pThread->nArg2))->defferThread, 0);
   return IFX_SUCCESS;
}
#endif

/**
   Defer work to process context

   Puts the pointer into a msgQ from where it will be processed by a task.

   \param pFunc - pointer to function to be called (not needed in VxWorks)
   \param pParam - parameter passed to the function

   \return IFX_SUCCESS or IFX_ERROR in case of an error

   \remarks
    In VxWorks taskSpawn() is not working when we are in interrupt context,
    so message queue will be used.
*/
IFX_return_t TAPI_DeferWork(IFX_void_t* pFunc, IFX_void_t* pParam)
{
   IFX_return_t ret = IFX_ERROR;
#ifndef TAPI_SINGLE_THREAD
   IFX_TAPI_EXT_EVENT_PARAM_t *pEvParam = (IFX_TAPI_EXT_EVENT_PARAM_t *) pParam;
#else
   IFX_int32_t (*Func)(IFX_void_t*) = pFunc;
#endif

   if (pParam == IFX_NULL)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("Wrong input arguments."
            "(File: %s, line: %d)\n", __FILE__, __LINE__));
      return IFX_ERROR;
   }

#ifdef TAPI_SINGLE_THREAD
   /* Defer work not supported in single thread */
   ret = Func(pParam);
#else
   /* start a thread */
   ret = TAPI_OS_ThreadInit (&pEvParam->defferThread, "TAPIdefer",
         ifx_tapi_defer_thread,
         TAPI_DEFER_THREAD_STACK_SIZE, TAPI_OS_THREAD_PRIO_HIGHEST,
         (IFX_ulong_t)pFunc, (IFX_ulong_t)pParam);
#endif
   return ret;
}

#endif /* #if (defined(IFXOS_USE_DEV_IO) && (IFXOS_USE_DEV_IO == 1)) */


