/******************************************************************************

                              Copyright (c) 2014
                        Lantiq Beteiligungs-GmbH & Co.KG
                             http://www.lantiq.com

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/**
   \file drv_tapi_vxworks.c
   This file contains the implementation of High-Level TAPI Driver,
   VxWorks specific part.

   The implementation includes the following parts:
    -Registration part by which the low-level drivers register themselves.
     During the registration appropriate device nodes are created and
     registered with the IOS.
    -Device node operations (open, close, ioctl, read, write, select)
     are done here.
    -Timer abstraction layer.
    -Deferring of a function call for later execution.
*/

/* ============================= */
/* Includes                      */
/* ============================= */

#include "drv_tapi.h"
#include "drv_tapi_api.h"
#include "drv_tapi_event.h"
#include "drv_tapi_errno.h"
#include "drv_tapi_ioctl.h"
#include "drv_tapi_stream.h"
#include "drv_tapi_cid.h"
#include "drv_tapi_polling.h"

/* ============================= */
/* Defines                       */
/* ============================= */

/* VxWorks task priority of the task that executes the functions upon
   expiration of a timer. */
#define  TSK_PRIO_TIMER_HANDLER   51
/* VxWorks task priority of the task that processes all events */
#define  TSK_PRIO_EVENT_HANDLER   51

#ifdef VXWORKS

#ifndef OK
   /** return value on success */
   #define OK      0
#endif

#ifndef ERROR
   /** return value on failure */
   #define ERROR  (-1)
#endif

/*
    Device header passed to the IOS at the device driver creation during
    the system startup.
*/
typedef struct
{
   DEV_HDR           DevHdr;   /* VxWorks specific: IOS header               */
   /** \todo Which one to use Major or Driver number? */
   IFX_int32_t       nMajorNum; /* Major number */
   IFX_uint32_t      nDevNum;     /* Device number                              */
   IFX_uint32_t      nChNum;     /* Channel Number                             */
   IFX_int32_t       nInUse;    /* In Use counter                             */
   IFX_void_t*       pCtx;     /* context pointer: device or channel context */
} Y_S_dev_header;

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
   /* Interface Semaphore      */
   TAPI_OS_mutex_t     oSemDrvIF;
   /* device header pointer    */
   Y_S_dev_header**     pDevHdr;
   /* number of device header pointers */
   IFX_int32_t          nDevHdr;
} Y_S_dev_ctx;

/* ============================= */
/* Local variable definition     */
/* ============================= */

/* The Global device driver structure */
Y_S_dev_ctx TAPI_devCtx;

/** install parameter TAPI_debug_level: LOW (1), NORMAL (2), HIGH (3), OFF (4) */
#ifdef ENABLE_TRACE
extern IFX_uint32_t TAPI_debug_level;
#endif /* ENABLE_TRACE */


/** Protection for array of timer structs. */
static TAPI_OS_mutex_t semTimerArrDataLock;

/* ============================= */
/* Global variable definition    */
/* ============================= */

/** Message queue ID, holding events to be handled. */
static MSG_Q_ID nMsgQueue_Events = IFX_NULL;

/** Task ID of task for handling msg queue with events. */
static IFX_int32_t nTaskForEvents_ID = -1;

/** Size of events message queue, number of event messages that can be queued in. */
/** \todo make define MAX_MSG_CNT_EVENT_QUEUE dependent on number of channels available in the voice system */
enum { MAX_MSG_CNT_EVENT_QUEUE = 32 * 8};

/** Size of one message in the event queue. (Size of one pointer) */
const IFX_int32_t MSG_SIZE_EVENT_QUEUE = sizeof(IFX_int32_t);


/* -------------------------------------------------------------------------- */

/* ============================= */
/* Local function declaration    */
/* ============================= */

/* Functions handling the select/unselect driver system io calls */
static IFX_int32_t TAPI_OS_Select(IFX_TAPI_ioctlCtx_t* pCtx,
                                  IFX_int32_t nArgument);
static IFX_int32_t TAPI_OS_Unselect(IFX_TAPI_ioctlCtx_t* pCtx,
                                    IFX_int32_t nArgument);
static IFX_int32_t TAPI_OS_devNodeRegister (Y_S_dev_ctx* pDevCtx, int nMinor,
                                          IFX_char_t* buf);

static IFX_int32_t TAPI_OS_devCreate (Y_S_dev_ctx* pDevCtx,
                                         IFX_TAPI_DRV_CTX_t* pLLDrvCtx);
static IFX_int32_t TAPI_OS_devDelete(Y_S_dev_ctx* pDevCtx);

static IFX_return_t ifx_tapi_Event_StartMsgQueue(void);

/* ============================= */
/* Local function definition     */
/* ============================= */

/* ============================= */
/* Global function definition    */
/* ============================= */


/* --------------------------------------------------------------------------
                          REGISTRATION mechanisms  -->  BEGIN
   --------------------------------------------------------------------------
*/

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
   /* The "TAPI_devCtx" global area contains all the         */
   if (TAPI_devCtx.nDrvNum == ERROR)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("Unable to install the driver. "
            "(File: %s, line: %d)\n", __FILE__, __LINE__));
      return IFX_ERROR;
   }
   /* copy registration info from Low level driver */
   TAPI_devCtx.nMaxDev = pLLDrvCtx->maxDevs;
   TAPI_devCtx.nMaxChn = pLLDrvCtx->maxChannels;
   TAPI_devCtx.nMajorNum = pLLDrvCtx->majorNumber;

   if (TAPI_OS_devCreate(&TAPI_devCtx, pLLDrvCtx) == IFX_ERROR)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("Err creating device. "
            "(File: %s, line: %d)\n", __FILE__, __LINE__));
       return TAPI_statusErr;
   }

   TAPI_OS_MutexInit (&TAPI_devCtx.oSemDrvIF);
   /* Initialize mutex for timer struct array access */
   TAPI_OS_MutexInit (&semTimerArrDataLock);

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
   TAPI_OS_MutexDelete (&semTimerArrDataLock);
   TAPI_OS_MutexDelete (&TAPI_devCtx.oSemDrvIF);

   return TAPI_statusOk;

}


/* --------------------------------------------------------------------------
                          REGISTRATION mechanisms  -->  END
   --------------------------------------------------------------------------
*/


/* --------------------------------------------------------------------------
         DRIVER funcs (read/write/ioctl/release/open/poll)  -->  BEGIN
   --------------------------------------------------------------------------
*/


/**
   This function open a TAPI device previously registered to the OS during the
   device driver creation. For each Tapi channel, a device have been added in
   the device list with the device string "/dev/<device>DC":
      *<device> - vin, vmmc, ...
      - D: device number
      - C: channel number in the device

   The OS passes the Y_S_dev_header structure associated to the
   device "/dev/<device>DC":
      DevHdr: VxWorks specific header
      ddrvn : TAPI Device Driver Number (allocated by IOS)
      devn  : device number  (1 to TAPI_MAX_DEVICES)
      chnn  : channel number (0 to TAPI_MAX_CH_NR)
      pctx  : pointer to the channel context (will be allocated at the device open)

   The function will update the pctx field in the device header with the TAPI
   device channel (control/voice) selected structure.

   \param pDevHeader - device header pointer
   \param pAnnex - tail of device name / not used
   \param flags - open flags / not used

  \return pDevHeader - device header pointer
                <> IFX_NULL, the device is open without error
                == IFX_NULL, error
*/
static IFX_int32_t ifx_tapi_open(Y_S_dev_header* pDevHeader,
                                 INT8* pAnnex, IFX_int32_t flags)
{
   Y_S_dev_ctx             *pDevCtx    = IFX_NULL; /* device context pointer */
   IFX_TAPI_DRV_CTX_t      *pDrvCtx    = IFX_NULL;
   TAPI_DEV                *pTapiDev   = IFX_NULL;
   TAPI_CHANNEL            *pTapiCh    = IFX_NULL;
   IFX_TAPI_LL_CH_t        *pLLChDev   = IFX_NULL;
   IFX_uint32_t            nDev, nCh;

   if (pDevHeader == IFX_NULL)
   {
      /* The device driver is not installed */
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("pDevHeader is NULL. "
               "(File: %s, line: %d)\n", __FILE__, __LINE__));
      errno = -ENODEV;
      return (IFX_int32_t) IFX_NULL;
   }

   /* Get the Global device driver context. */
   pDevCtx = &TAPI_devCtx;
   /* Get the pointer to the device driver context based on the major number */
   pDrvCtx = IFX_TAPI_DeviceDriverContextGet(pDevHeader->nMajorNum);
   if (pDrvCtx == IFX_NULL)
   {
      /* This should actually never happen because the file descriptors are
         registered in TAPI_OS_RegisterLLDrv after a driver context is known. */
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("Err getting pDrvCtx."
               "(File: %s, line: %d)\n", __FILE__, __LINE__));
      errno = -ENODEV;
      return (IFX_int32_t) IFX_NULL;
   }

   /* protect against concurrent access */
   TAPI_OS_MutexGet (&pDevCtx->oSemDrvIF);

   nDev = pDevHeader->nDevNum;
#ifndef TAPI_ONE_DEVNODE
   /* for multi dev nodes the nDevNum starts at 0, otherwise there
    * is an offset of 1 */
   nDev -= 1;
#endif /* TAPI_ONE_DEVNODE */

   nCh  = pDevHeader->nChNum;

   /* check the device number */
   if (nDev >= pDrvCtx->maxDevs)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
           ("TAPI_DRV: max. device number exceed\n"));
      errno = -ENODEV;
      goto OPEN_ERROR;
   }

   /* check the channel number */
   if (nCh > pDrvCtx->maxChannels)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
          ("TAPI_DRV: max. channel number exceed\n"));
      errno = -ENODEV;
      goto OPEN_ERROR;
   }

   /*  check for device node */
   if (nCh == 0)
      nDev = 0;

   /* channel or multi dev node */
   pTapiDev = pDrvCtx->pTapiDev + nDev;

   if (nCh == 0)
   {
      /* Save the device pointer */
      pDevHeader->pCtx = pTapiDev;
      pLLChDev = pTapiDev->pLLDev;
   }
   else if (nCh <= pDrvCtx->maxChannels)
   {
      pTapiCh = pTapiDev->pChannel + nCh - 1;
      /* Save the channel pointer */
      pDevHeader->pCtx = pTapiCh;
      pLLChDev = pTapiCh->pLLChannel;
   }

   /* Call the Low level Device specific open routine */
   if (IFX_TAPI_PtrChk (pDrvCtx->Open))
   {
      IFX_int32_t retLL;

      retLL = pDrvCtx->Open (pLLChDev);

      if (!TAPI_SUCCESS(retLL))
      {
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
               ("Open LL channel failed. [dev:%d ch:%d]\n", nDev, nCh));
         goto OPEN_ERROR;
      }
   }

   /* Increment the Usage counter */
   pTapiDev->nInUse++;

   if (IFX_NULL != pTapiCh)
      pTapiCh->nInUse++;

   pDevHeader->nInUse++;

   /* release lock */
   TAPI_OS_MutexRelease (&pDevCtx->oSemDrvIF);

   return (IFX_int32_t) pDevHeader;

OPEN_ERROR:

   /* release lock */
   TAPI_OS_MutexRelease (&pDevCtx->oSemDrvIF);

   if (pDrvCtx != IFX_NULL && pDrvCtx->pTapiDev != IFX_NULL)
   {
      TAPI_OS_Free (pDrvCtx->pTapiDev);
   }

   errno = -ENODEV;
   return (IFX_int32_t) IFX_NULL;
} /* ifx_tapi_open() */

/**
   Configuration / Control for the device.

   \param pDevHeader - device header pointer
   \param nCmd - Configuration/Control command
   \param nArg - Configuration/Control arguments, optional

   \return IFX_SUCCESS    - no problems
           <> IFX_SUCCESS - Function is not implemented or other error

   \remarks
   This function does the following functions:
      - If the ioctl command is device specific, low-level driver's ioctl function
      - If the ioctl command is TAPI specific, it is handled at this level
*/
static STATUS ifx_tapi_ioctl(Y_S_dev_header* pDevHeader,
                             IFX_int32_t nCmd,
                             IFX_int32_t nArg)
{
   IFX_int32_t ret = IFX_SUCCESS;
   IFX_TAPI_DRV_CTX_t* pDrvCtx = IFX_NULL;
   IFX_TAPI_ioctlCtx_t  ctx;
   IFX_int32_t minor_num = 0;
   IFX_int32_t (*pfDoSelect)(IFX_TAPI_ioctlCtx_t*, IFX_int32_t);

   if (pDevHeader == IFX_NULL)
   {
      /* The device driver is not installed */
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("pDevHeader is NULL. "
            "(File: %s, line: %d)\n", __FILE__, __LINE__));
      errno = -ENODEV;
      return ERROR;
   }

   if (pDevHeader->pCtx == IFX_NULL)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("Device Driver not installed. "
            "(File: %s, line: %d)\n", __FILE__, __LINE__));
      /* The device driver is not installed */
      errno = ENODEV;
      return ERROR;
   }

   pDrvCtx = IFX_TAPI_DeviceDriverContextGet (pDevHeader->nMajorNum);
   if (pDrvCtx == IFX_NULL)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("Err getting device driver."
            "(File: %s, line: %d)\n", __FILE__, __LINE__));
      errno = ENODEV;
      return ERROR;
   }

   minor_num = pDevHeader->nDevNum * 10 + pDevHeader->nChNum;

   pfDoSelect = TAPI_OS_Select;
   switch (nCmd)
   {
      case FIOUNSELECT:
         pfDoSelect = TAPI_OS_Unselect;
      case FIOSELECT:
#ifdef TAPI_ONE_DEVNODE
         /* enumerate all devices */
         for (minor_num = 10; minor_num <= (pDrvCtx->maxDevs * 10);
            minor_num+=10)
#endif /* TAPI_ONE_DEVNODE */
         {
            /* get the ioctl context: channel, device etc. */
            ret = TAPI_ioctlContextGet (pDrvCtx, minor_num, nCmd, 0,
               IFX_FALSE, &ctx);
            if (IFX_SUCCESS == ret)
            {
               ret = pfDoSelect (&ctx, nArg);
            }

            TAPI_ioctlContextPut (0, &ctx);
         }
         break;
      default:
         /* get the ioctl context: channel, device etc. */
         ret = TAPI_ioctlContextGet (pDrvCtx, minor_num, nCmd, nArg,
            IFX_FALSE, &ctx);
         if (IFX_SUCCESS == ret)
         {
            ret = TAPI_Ioctl (&ctx);
         }

         TAPI_ioctlContextPut (nArg, &ctx);
         break;
   } /* switch */

   return ret;
} /* ifx_tapi_ioctl() */


/**
    Release the device. The in-use counter will be decreased

   \param pDevHeader - device header pointer

   \return OK - no error
           -ENODEV - MAX_SOC_CHANNELS is exceeded
           ERROR on error

   \remarks
      This function gets called when a close is called on the device.
      It decrements the usage count, free the FIFOs
*/
static STATUS ifx_tapi_release(Y_S_dev_header *pDevHeader)
{
   /* Current Tapi device structure */
   TAPI_DEV*            pTapiDev = IFX_NULL;
   /* Current Tapi device channel structure */
   TAPI_CHANNEL*        pTapiCh  = IFX_NULL;
   IFX_uint8_t          nCh;
   /* device context pointer */
   Y_S_dev_ctx*         pDevCtx = IFX_NULL;
   IFX_TAPI_DRV_CTX_t*  pDrvCtx = IFX_NULL;
   IFX_TAPI_LL_CH_t*    pLLChDev = IFX_NULL;

   if (IFX_NULL == pDevHeader)
   {
      /* The device driver is not installed */
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("pDevHeader is NULL. "
            "(File: %s, line: %d)\n", __FILE__, __LINE__));
      errno = ENODEV;
      return (ERROR);
   }

   /* Get the Global device driver context and check that the device driver
      is  already installed.  */
   pDevCtx = &TAPI_devCtx;
   if (IFX_NULL == pDevCtx)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("Device driver not installed. "
            "(File: %s, line: %d)\n", __FILE__, __LINE__));
       return ERROR;
   }

   /* pDevHeader: device header updated at the device opening      */
   /*     - pctx: points to the TAPI device channel structure   */
   if (pDevHeader->pCtx == IFX_NULL)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("Data context (ch, dev) missing. "
            "(File: %s, line: %d)\n", __FILE__, __LINE__));
      return ERROR;
   }

   TAPI_OS_MutexGet (&pDevCtx->oSemDrvIF);
   /* We first try to cast the context pointer to a device structure, just
      to test if it's a channel (TAPI_CHANNEL *) or a device (TAPI_DEVICE *)
      since nChannel is available in both structures this will work
      if pDevHeader->pctx is a channel, pCh is used instead of pDev. */
   pTapiDev = (TAPI_DEV *) pDevHeader->pCtx;
   nCh = pTapiDev->nChannel;

   /* Test the channel number in order to determine the
      Tapi device structure. */
   if (pTapiDev->nChannel != IFX_TAPI_DEVICE_CH_NUMBER)
   {
      pTapiCh = (TAPI_CHANNEL *) ((IFX_void_t *) pTapiDev);
      pTapiDev = pTapiCh->pTapiDevice;
      pDrvCtx = pTapiDev->pDevDrvCtx;
      pLLChDev = pTapiCh->pLLChannel;

      TRACE(TAPI_DRV, DBG_LEVEL_LOW,
            ("Closing device channel %d (%d)\n\r",
             pTapiCh->nChannel, pTapiCh->nInUse));
   }
   else
   {
      pDrvCtx = pTapiDev->pDevDrvCtx;
      pLLChDev = pTapiDev->pLLDev;
      /* Memory will be released when module is removed. */
   }

   TRACE(TAPI_DRV, DBG_LEVEL_LOW,
        ("closing device %d, Minor %d...\n\r",
         pDevHeader->nDevNum, pDevHeader->nDevNum));
   TAPI_OS_MutexRelease (&pDevCtx->oSemDrvIF);

   /* Call the Low-level Device specific release routine. */
   /* Not having such a function is not an error. */
   if (IFX_TAPI_PtrChk (pDrvCtx->Release))
   {
      IFX_int32_t retLL;

      retLL = pDrvCtx->Release (pLLChDev);

      if (!TAPI_SUCCESS (retLL))
      {
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
               ("Release LL channel failed for ch: %d\n", nCh));
         errno = EBUSY;
         return (ERROR);
      }
   }

   /* decrement the use counters */
   if (IFX_NULL != pTapiCh)
   {
      if (pTapiCh->nInUse > 0)
      {
         pTapiCh->nInUse--;
      }

#ifdef TAPI_FEAT_PACKET
      /* Free Fifo memory. */
      if (0 == pTapiCh->nInUse)
      {
         /* Call the Low level function to free the FIFO */
         /** \todo Do not free FIFO otherwise application could not start
             for the second time. Problem, in ifx_tapi_open only first time
             everything is initialized. */
      }
#endif /* TAPI_FEAT_PACKET */
   }

   /* decrement the use counter */
   if (pTapiDev->nInUse > 0)
   {
      pTapiDev->nInUse--;
   }

   pDevHeader->nInUse--;

   if (pDevHeader->nInUse == 0)
   {
      /* We erase the reference to the Tapi structure in
         header. */
      pDevHeader->pCtx = IFX_NULL;
   }

   return (OK);
} /* ifx_tapi_release() */


/**

   Read data from the Tapi.

   \param pDevHeader - device header pointer
   \param pDest - data destination pointer
   \param nLength - data length to read

   \return len - data length
*/
static IFX_int32_t ifx_tapi_read(Y_S_dev_header* pDevHeader,
                                 IFX_uint8_t* pDest,
                                 IFX_int32_t nLength)
{
#ifdef TAPI_FEAT_PACKET
   TAPI_DEV* pTapiDev = IFX_NULL;
   TAPI_CHANNEL* pTapiCh = IFX_NULL;
   IFX_TAPI_DRV_CTX_t* pDrvCtx = IFX_NULL;
   IFX_TAPI_LL_CH_t* pCh = IFX_NULL;
   IFX_int32_t ch_idx = 0;
   IFX_void_t const    *pPacket   = NULL;
   IFX_uint32_t         nOffset;
   IFX_uint32_t         size = 0;

   if (IFX_NULL == pDevHeader)
   {
      /* The device driver is not installed */
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("pDevHeader is NULL. "
            "(File: %s, line: %d)\n", __FILE__, __LINE__));
      errno = -ENODEV;
      return 0;
   }

   pTapiCh = (TAPI_CHANNEL *) pDevHeader->pCtx;
   pTapiDev = pTapiCh->pTapiDevice;

   if (pTapiDev->bInitialized == IFX_FALSE)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_LOW,
            ("TAPI_DRV: read failed because device is not initialised\n"));
      return -EFAULT;
   }

   ch_idx = pTapiCh->nChannel;
   if (0 > ch_idx)
   {
      ch_idx = 0;
   }
   pTapiCh = &(pTapiDev->pChannel[ch_idx]);

   pDrvCtx = pTapiCh->pTapiDevice->pDevDrvCtx;
   pCh = pTapiCh->pLLChannel;

   /* All packets to be read are already in the channel specific upstream
      fifo of the driver. They are put there by whoever receives packets.
      So all that is done here operates on this fifo and does not need to call
      any ll-driver function any more. */

   /* Lock access to the fifo. */
   TAPI_OS_MutexGet (&pTapiCh->semTapiChDataLock);
   if (IFX_TAPI_PtrChk (pDrvCtx->IRQ.LockDevice))
   {
      pDrvCtx->IRQ.LockDevice (pTapiDev->pLLDev);
   }

   /* If FIFO is empty or does not exist we cannot read anything. In this
      case we either wait until data arrives or return if non-blocking.
      If the FIFO does not exist no data will ever arrive and the possible
      wait will last forever. */
   TAPI_OS_SPIN_LOCK_IRQSAVE(&pTapiCh->sl_up_stream_fifo[fifo_idx]);
   if ((pTapiCh->pUpStreamFifo[IFX_TAPI_STREAM_COD] == IFX_NULL) ||
       (fifoEmpty(pTapiCh->pUpStreamFifo[IFX_TAPI_STREAM_COD]) == IFX_TRUE))
   {
      /* Unlock access to the fifo. */
      TAPI_OS_SPIN_UNLOCK_IRQRESTORE(&pTapiCh->sl_up_stream_fifo[fifo_idx]);
      if (pDrvCtx->IRQ.UnlockDevice != IFX_NULL)
      {
         pDrvCtx->IRQ.UnlockDevice (pTapiDev->pLLDev);
      }
      TAPI_OS_MutexRelease (&pTapiCh->semTapiChDataLock);

      /* check the non-blocking flag */
      if (pTapiCh->nFlags & CF_NONBLOCK)
      {
         /* this is a non blocking read call - return immediately */
         return 0;
      }

      /* this is a blocking read call so go to sleep now */
      if (TAPI_OS_EventWait (&pTapiCh->semReadBlock,
                             TAPI_OS_WAIT_FOREVER, IFX_NULL) == IFX_ERROR)
      {
         /* timeout has expired without arrival of data */
         return 0;
      }

      /* we have been woken because data has arrived */

      /* before accessing the mailbox - lock again. */
      TAPI_OS_MutexGet (&pTapiCh->semTapiChDataLock);
      if (IFX_TAPI_PtrChk (pDrvCtx->IRQ.LockDevice))
      {
         pDrvCtx->IRQ.LockDevice (pTapiDev->pLLDev);
      }
      /*
      Do not get the spinlock here, because the get function is protected inside
      TAPI_OS_SPIN_LOCK_IRQSAVE(&pTapiCh->sl_up_stream_fifo[fifo_idx]);
      */
   }

   /* read data from the fifo */
   TAPI_ASSERT (
      fifoEmpty(pTapiCh->pUpStreamFifo[IFX_TAPI_STREAM_COD]) == IFX_FALSE);
   pPacket = IFX_TAPI_UpStreamFifo_Get(pTapiCh,
                                       IFX_TAPI_STREAM_COD, &size, &nOffset);

   /* Unlock access to the fifo. */
   if (pDrvCtx->IRQ.UnlockDevice != IFX_NULL)
   {
      pDrvCtx->IRQ.UnlockDevice (pTapiDev->pLLDev);
   }
   TAPI_OS_MutexRelease (&pTapiCh->semTapiChDataLock);

   /* sanity check on the received packet */
   if (pPacket == NULL)
   {
      /* packet may not be NULL - internal error */
      TRACE (TAPI_DRV, DBG_LEVEL_LOW, ("INFO: pPacket Null\n"));
      return 0;
   }
   if (size > (IFX_uint32_t)nLength)
   {
      /* output buffer not large enough for data */
      /* drop the packet */
      IFX_TAPI_VoiceBufferPut((IFX_void_t *)pPacket);
      /* return linux error code to the application */
      return -EINVAL;
   }

   /* unmap data */
   TAPI_OS_CpyKern2Usr ((void *)pDest,
                        (void *)((char *)pPacket + nOffset),
                        (IFX_uint32_t)size);

   /* return buffer back to the pool now that is has been copied */
   IFX_TAPI_VoiceBufferPut((IFX_void_t *)pPacket);

   return (IFX_int32_t)size;
#else

   return (IFX_int32_t)0;
#endif /* TAPI_FEAT_PACKET */

}


/**
   Writes data to the device.

   \param pDevHeader - device header pointer
   \param pSrc - data source pointer
   \param nLength - data length to write

   \return nLength - 0 if failure else the length
*/
static IFX_int32_t ifx_tapi_write(Y_S_dev_header* pDevHeader,
                                  IFX_uint8_t* pSrc,
                                  IFX_int32_t nLength)
{
#ifdef TAPI_FEAT_PACKET
   TAPI_DEV* pTapiDev = IFX_NULL;
   TAPI_CHANNEL* pTapiCh = IFX_NULL;
   IFX_TAPI_DRV_CTX_t* pDrvCtx = IFX_NULL;
   IFX_int32_t ch_idx = 0;
   IFX_uint8_t         *pData;
   IFX_size_t           buf_size;
   IFX_ssize_t          size = 0;

   if (IFX_NULL == pDevHeader)
   {
      /* The device driver is not installed */
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("pDevHeader is NULL. "
            "(File: %s, line: %d)\n", __FILE__, __LINE__));
      errno = -ENODEV;
      return 0;
   }

   pTapiCh = (TAPI_CHANNEL *) pDevHeader->pCtx;
   pTapiDev = pTapiCh->pTapiDevice;

   if (pTapiDev->bInitialized == IFX_FALSE)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_LOW,
            ("TAPI_DRV: read failed because device is not initialised\n"));
      return -EFAULT;
   }

   ch_idx = pTapiCh->nChannel;
   if (0 > ch_idx)
   {
      ch_idx = 0;
   }
   pTapiCh = &(pTapiDev->pChannel[ch_idx]);

   pDrvCtx = pTapiCh->pTapiDevice->pDevDrvCtx;

   if (IFX_TAPI_PtrChk (pDrvCtx->Write))
   {
      /* Truncate data to the size of our voice buffer. */
      buf_size = IFX_TAPI_VoiceBufferPool_ElementSizeGet();
      if (nLength > (buf_size - pDrvCtx->pktBufPrependSpace))
      {
         nLength = (buf_size - pDrvCtx->pktBufPrependSpace);
      }

      /* Get a buffer from the voice buffer pool. */
      /* The vinetic ll-write function requires a TAPI voice buffer
         to have some space in which it can add the mailbox header. */
      if ((pData = IFX_TAPI_VoiceBufferGet()) == IFX_NULL)
      {
         /* voice buffer pool is depleted */
         return -EFAULT;
      }

      /* if you have time and want clean buffers you might like this */
      /* memset(pData, 0, buf_size); */

      /* Copy data into buffer. */
      TAPI_OS_CpyUsr2Kern ((void *)(pData + pDrvCtx->pktBufPrependSpace),
                           (void *)pSrc, (IFX_uint32_t)nLength);

      /* Call the low-level driver's write function. */
      size = pDrvCtx->Write (pTapiCh->pLLChannel, pData, nLength,
                             (IFX_int32_t*)IFX_NULL, IFX_TAPI_STREAM_COD);
   }
   else
   {
      TRACE(TAPI_DRV, DBG_LEVEL_LOW,
            ("TAPI_DRV: LL-driver does not provide packet write\n"));
      return -EFAULT;
   }

   return (IFX_int32_t)size;
#else
   return (IFX_int32_t)0;
#endif /* TAPI_FEAT_PACKET */
}

/* --------------------------------------------------------------------------
             DRIVER funcs (read/write/ioctl/release/open)  -->  END
   --------------------------------------------------------------------------
*/


/* --------------------------------------------------------------------------
                          CREATE/DELETE driver  -->  BEGIN
   --------------------------------------------------------------------------
*/


/**

   TAPI device driver initialization.
   This is the device driver initialization function to call at the system
   startup prior any access to the TAPI device driver.
   After the initialization the device driver is ready to be accessed by
   the application. The global structure "TAPI_devCtx" contains all the data
   handling the interface (open, close, ioctl,...).

   \arguments None

   \return OK or ERROR
*/
IFX_int32_t Tapi_DeviceDriverInit(void)
{
   /* The "TAPI_devCtx" global area contains all the         */
   /* data related to the device driver installation */
   /* The area is used during all the life of the VINETIC    */
   /* device driver. The area is updated during the creation */
   /* and the access to the device driver.                   */
   /* Reset the memory area.                                 */
   memset(&TAPI_devCtx, 0x00, sizeof(Y_S_dev_ctx));
   /* IOS device driver creation. We add the TAPI device driver to the
      IOS device list providing the device driver function interface.
      The IOS returns the Device Driver Number which will be used later on.
      Maybe done only once */
   TAPI_devCtx.nDrvNum = iosDrvInstall(IFX_NULL, IFX_NULL,
                                   (FUNCPTR) ifx_tapi_open,
                                   (FUNCPTR) ifx_tapi_release,
                                   (FUNCPTR) ifx_tapi_read,
                                   (FUNCPTR) ifx_tapi_write,
                                    (FUNCPTR) ifx_tapi_ioctl);
   if (TAPI_devCtx.nDrvNum == ERROR)
      return ERROR;

   printf("%s, (c) 2001-2014 Lantiq Deutschland GmbH\n", &TAPI_WHATVERSION[4]);

   ifx_tapi_Event_StartMsgQueue();

   IFX_TAPI_Driver_Start();

   /* Very first call used for calibration  of the self-calibrating hard
      delay routines library "DelayLib.c" For IDES3300 this calibration
      is done before, so this leads only to an additonal affordable
      wait of 1us. */
   TAPI_OS_USecSleep(1);

   return IFX_SUCCESS;
} /* Tapi_DeviceDriverInit() */


/**

   TAPI device driver shutdown.
   This is the device driver shutdown function. This function is called by the
   system when the TAPI device driver is no longer needed. Prior to shutdown
   the device driver all the device channels should be closed.
   This function releases all the resources granted by the device driver.

   \param None

   \return OK or ERROR

   \remarks
*/
IFX_int32_t Tapi_DeviceDriverStop(VOID)
{
   IFX_int32_t i = 0;

   printf("Removing Highlevel TAPI module\n");

   if (TAPI_OS_devDelete(&TAPI_devCtx) == IFX_ERROR)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("Err deleting device. "
            "(File: %s, line: %d)\n", __FILE__, __LINE__));

      return IFX_ERROR;
   }

   TAPI_OS_MutexDelete (&TAPI_devCtx.oSemDrvIF);

   /* Delete mutex for timer struct array */
   TAPI_OS_MutexDelete (&semTimerArrDataLock);

   /* Actually all LL drivers should have unregistered here. Being careful
      we force unregister of any drivers which may still be registered. */
   for (i = 0; i < TAPI_MAX_LL_DRIVERS; i++)
   {
      if (gHLDrvCtx [i].pDrvCtx != IFX_NULL)
      {
         IFX_TAPI_Unregister_LL_Drv (gHLDrvCtx [i].pDrvCtx->majorNumber);
      }
   }

   IFX_TAPI_Driver_Stop();

   TRACE(TAPI_DRV,DBG_LEVEL_NORMAL,("TAPI_DRV: cleanup successful\n"));

   return IFX_SUCCESS;
} /* Tapi_DeviceDriverStop() */


/**

   TAPI device driver creation.
   This function is called at the system startup (Tapi_DeviceDriverInit)
   in order to create and install the TAPI device driver in the target OS (VxWorks).

   Device driver Creation: the device driver is declared to the IOS system with
   "iosDrvInstall" OS function. We pass to the IOS the device driver interface
   functions (open, close, ioctl, ...), the IOS returns a Device Driver Number which
   will be used for adding the devices to the device driver.

   Device addition: Once the Device driver have been declared to the IOS system,
   we gone add the devices to the device driver list. Each device is identified by
   a unique string which will be used by the application for opening a device
   (the application will get in return a unique file descriptor (fd) allocated by
   the system and will be used for further access to the selected device).

   Device string selection: "/dev/vinDC" with,

        - "/dev/vin", this is a VINETIC device
        - "D",        designate the device number
                        "1",    VINETIC device 1
                        "2",    VINETIC device 2
                        ...
                        "VINETIC_MAX_DEVICES" VINETIC device VINETIC_MAX_DEVICES
        - "C",        designate a channel (C) in the designated device (D)
                        "0",    Control channel (device wise)
                        "1",    voice channel 1 (channel wise)
                        ...,
                        "VINETIC_MAX_CH_NR", voice channel VINETIC_MAX_CH_NR (channel wise)


   VINETIC_MAX_DEVICES, number maximum of VINETIC device
   VINETIC_MAX_CH_NR,   number maximum of voice channels

   The TAPI device driver is a multi-device driver, for each device, a device is
   accessed through channels. We distinghish two types of channels:
    - Control channel, used for control and configuration purpose. It's device wise and
    is always the channel number ZERO (0).
    - Voice channel, used to access a device voice channel. It's channel wise and the
    channel is <> to ZERO (0).

    Each device channel (Control/Voice) is represented by a device in the IOS system.
    The device is added to the device list for the VINETIC device driver, the number
    of devices per VINETIC device is:

          VINETIC_MAX_CH_NR (number of Voice channels) + 1 (Control Channel).


    The "Y_S_dev_header" structure is passed to the IOS sytem:
        DevHdr: VxWorks specific header
        ddrvn : VINETIC Device Driver Number (allocated by IOS)
        devn  : device number  (1 to VINETIC_MAX_DEVICES)
        chnn  : channel number (0 to VINETIC_MAX_CH_NR)
        pctx  : pointer to the channel context (will be allocated at the device open)


   \param pDevCtx - The Global device driver structure address

   \return OK or ERROR
*/
static IFX_int32_t TAPI_OS_devCreate (Y_S_dev_ctx* pDevCtx,
                                         IFX_TAPI_DRV_CTX_t* pLLDrvCtx)
{
#ifndef TAPI_ONE_DEVNODE
   IFX_uint8_t nDev, nCh;
#endif
   IFX_uint8_t nMinor, maxFds;
   IFX_char_t  buf[64];
   IFX_char_t* pzsDeviceName = IFX_NULL;

   if ((IFX_NULL == pDevCtx) || (IFX_NULL == pLLDrvCtx) || (IFX_NULL == pLLDrvCtx->devNodeName))
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("Wrong input arguments. "
            "(File: %s, line: %d)\n", __FILE__, __LINE__));
      return IFX_ERROR;
   }

   pzsDeviceName = pLLDrvCtx->devNodeName;

   /* max file descriptors per device /dev/fd10 + /dev/fd11..1x
      at the moment most of the ll driver report a wrong value for
      pLLDrvCtx->maxChannels of 4 which doesn't reflect the 8 PCM channels
      Therefore in we use pLLDrvCtx->maxChannels only in case of
      DuSLIC-xT (2). For all others we stick to 8 till cleanup.
      This is no limitation for the system. */
   maxFds      = (pLLDrvCtx->maxChannels == 2) ?
                  pLLDrvCtx->maxChannels : 8;
   /* limit to one digit , more than 10 is not possible with the multi device
      node concept fd10 .. fd19, fd20 .. fd29 */
   if (maxFds > 9)
      maxFds = 8;

   /* The device driver is declared to the IOS: Now we add to the device
      driver IOS list the devices for each TAPI device channel. */
   nMinor = pDevCtx->nDevHdr;
   pDevCtx->nDevHdr += (pDevCtx->nMaxDev + 1) * maxFds;
   pDevCtx->pDevHdr = realloc(pDevCtx->pDevHdr,
      pDevCtx->nDevHdr * sizeof(Y_S_dev_header*));
   memset (buf, 0, sizeof(buf));
#ifdef TAPI_ONE_DEVNODE
   sprintf(buf, "/dev/%s", pzsDeviceName);
   if (TAPI_OS_devNodeRegister (pDevCtx, nMinor, buf) == IFX_ERROR)
   {
      return IFX_ERROR;
   }
   pDevCtx->pDevHdr[nMinor]->nDevNum = 0;
   pDevCtx->pDevHdr[nMinor]->nChNum = 0;
   nMinor++;
#else /* TAPI_ONE_DEVNODE */
   for (nDev = 1; nDev < pLLDrvCtx->maxDevs + 1; nDev++)
   {
      /* Allocate memory for channels and control device. */
      for (nCh = 0; nCh < maxFds; nCh++)
      {
         sprintf(buf, "/dev/%s%d%d", pzsDeviceName, nDev, nCh);
         if (TAPI_OS_devNodeRegister (pDevCtx, nMinor, buf) == IFX_ERROR)
         {
            return IFX_ERROR;
         }
         pDevCtx->pDevHdr[nMinor]->nDevNum = nDev;
         pDevCtx->pDevHdr[nMinor]->nChNum = nCh;
         nMinor++;
      }
   }
#endif /* TAPI_ONE_DEVNODE */

   return IFX_SUCCESS;
} /* TAPI_OS_devCreate() */


static IFX_int32_t TAPI_OS_devNodeRegister (Y_S_dev_ctx* pDevCtx, int nMinor,
                                          IFX_char_t* buf)
{
   Y_S_dev_header* pDevHeader = IFX_NULL;

   /* Allocate memory for the device header. Reset the memory. */
   pDevHeader = (Y_S_dev_header*) TAPI_OS_Malloc (sizeof(Y_S_dev_header));
   if (IFX_NULL == pDevHeader)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("Err get mem pDevHeader. "
            "(File: %s, line: %d)\n", __FILE__, __LINE__));
      return IFX_ERROR;
   }
   memset(pDevHeader, 0x00, sizeof(Y_S_dev_header));

   /* Build the device string "/dev/vinij" and add the device to the
      IOS device list. */
   if (iosDevAdd(&pDevHeader->DevHdr, buf, pDevCtx->nDrvNum) == ERROR)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("Unable to add the device %s "
            "(File: %s, line: %d)\n", buf, __FILE__, __LINE__));
      TAPI_OS_Free (pDevHeader);
      return (IFX_ERROR);
   }

/* Fill the device header with intial values. */
   pDevHeader->nMajorNum = pDevCtx->nMajorNum;
#if 0
   pDevHeader->nDrvNum = pDevCtx->nDrvNum;
#endif /* 0 */

   pDevCtx->pDevHdr[nMinor] = pDevHeader;

   return IFX_SUCCESS;
}

/**

   TAPI device driver deletion.
   This function is called at the device driver shutdown
   (Tapi_DeviceDriverStop) in order to release and uninstall the OS-target
   resources (VxWorks) granted by the TAPI device driver.

   \param pDevCtx - The Global device driver structure address

   \return
   OK or ERROR
*/
static IFX_int32_t TAPI_OS_devDelete(Y_S_dev_ctx* pDevCtx)
{
   IFX_uint8_t nDev = 0;


   /* Check that we have a valid Global structure address      */
   /* and that a device is installed (OS-target point of view) */

   if ((pDevCtx == IFX_NULL) || (pDevCtx->nDrvNum == ERROR))
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("Wrong input arguments. "
            "(File: %s, line: %d)\n", __FILE__, __LINE__));

      return (ERROR);
   }

   for (nDev = 0; nDev < pDevCtx->nMaxDev; nDev++)
   {

      /* Delete the device from the IOS link list */
      /* free the device header                   */

      if (pDevCtx->pDevHdr[nDev])
      {
         iosDevDelete((DEV_HDR *)pDevCtx->pDevHdr[nDev]);
         TAPI_OS_Free (pDevCtx->pDevHdr[nDev]);
      }
   }

   /* We remove the TAPI device driver from the IOS. */
   /* We free the Tapi device structures.            */

   iosDrvRemove(pDevCtx->nDrvNum, IFX_TRUE);

   /* Reset to 0 the Global device driver structure */
   /* in order to reset all the pointers to NULL    */
   /* marking that the device driver is no longer   */
   /* installed.                                    */

   memset(pDevCtx, 0x00, sizeof(Y_S_dev_ctx));
   return (OK);
} /* TAPI_OS_devDelete() */


/* --------------------------------------------------------------------------
                          CREATE/DELETE driver  -->  END
   --------------------------------------------------------------------------
*/


/* --------------------------------------------------------------------------
                             SELECT mechanism  -->  BEGIN
   --------------------------------------------------------------------------
*/

/**
   Executes the select for the channel fd.

   \param pTapiCh  - handle to channel control structure
   \param node - node list
   \param opt - optional argument, which contains needed information for
                TAPI_OS_DrvSelectQueueAddTask

   \return System event qualifier. Either 0 or TAPI_OS_SYSREAD.
           IFX_ERROR fo error.

   \remarks
   This function needs operating system services, that are hidden by
   IFXOS macros.
*/
IFX_int32_t TAPI_SelectCh(TAPI_CHANNEL* pTapiCh,
                          IFX_int32_t node,
                          IFX_int32_t opt)
{
   IFX_TAPI_DRV_CTX_t* pDrvCtx = IFX_NULL;
   IFX_TAPI_LL_DEV_t* pLLDev = IFX_NULL;
   IFX_uint32_t  flags = 0;
   IFX_int32_t   ret = 0;
   IFX_TAPI_T38_STATUS_t TapiFaxStatus;


   if (IFX_NULL == pTapiCh)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("Wrong input arguments. "
            "(File: %s, line: %d)\n", __FILE__, __LINE__));
      return IFX_ERROR;
   }

   pDrvCtx = (IFX_TAPI_DRV_CTX_t*) pTapiCh->pTapiDevice->pDevDrvCtx;
   pLLDev = pTapiCh->pTapiDevice->pLLDev;

   /* Get the Status from the low level driver. */
   if (IFX_TAPI_PtrChk (pDrvCtx->COD.T38_Status_Get))
   {
      pDrvCtx->COD.T38_Status_Get(pTapiCh->pLLChannel, &TapiFaxStatus);
   }

   selNodeAdd ((SEL_WAKEUP_LIST *)&pTapiCh->wqRead,  (SEL_WAKEUP_NODE *)node);

#ifdef TAPI_FEAT_FAX_T38
   selNodeAdd ((SEL_WAKEUP_LIST *)&pTapiCh->wqWrite,  (SEL_WAKEUP_NODE *)node);

   if ((TapiFaxStatus.nStatus & IFX_TAPI_FAX_T38_TX_ON)
      && (IFX_TRUE == pTapiCh->bFaxDataRequest))
   {
      /* Task should write a new packet now. */
      ret |= TAPI_OS_SYSWRITE;
   }
#endif /* TAPI_FEAT_FAX_T38 */
   /* Select on a voice channel -- only implemented for TAPI. */
   flags |= CF_NEED_WAKEUP;
   /* Clear flags first, then apply new flags. */

   if ((IFX_NULL == pDrvCtx->IRQ.LockDevice)
       || (IFX_NULL == pDrvCtx->IRQ.UnlockDevice))
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("IRQ missing for locking/unlocking"
            " the device. (File: %s, line: %d)\n", __FILE__, __LINE__));
      return IFX_ERROR;
   }

   pDrvCtx->IRQ.LockDevice (pLLDev);

#ifdef TAPI_FEAT_PACKET
   TAPI_OS_SPIN_LOCK_IRQSAVE(&pTapiCh->sl_up_stream_fifo[IFX_TAPI_STREAM_COD]);
   if (!fifoEmpty(pTapiCh->pUpStreamFifo[IFX_TAPI_STREAM_COD]))
   {
      flags |= CF_WAKEUPSRC_STREAM;
      flags &= ~CF_NEED_WAKEUP;
      ret |= TAPI_OS_SYSREAD;
   }
   TAPI_OS_SPIN_UNLOCK_IRQRESTORE(&pTapiCh->sl_up_stream_fifo[IFX_TAPI_STREAM_COD]);
#endif /* TAPI_FEAT_PACKET */

   pTapiCh->nFlags &= ~(CF_WAKEUPSRC_GR909
                        | CF_WAKEUPSRC_STREAM
                        | CF_WAKEUPSRC_TAPI
                        | CF_NEED_WAKEUP);
   pTapiCh->nFlags |= flags;

   pDrvCtx->IRQ.UnlockDevice(pLLDev);

   return ret;
} /* TAPI_SelectCh() */


/**
   Handles the FIOSELECT system call.

   \param  pCtx         Pointer to IOCTL context struct.
   \param  nArgument    Node argument of FIOSELECT call.

   \return IFX_SUCCESS / IFX_ERROR

   \remarks
   The set of wake up queues are different for the VINETIC configuration/control
   channel and for the voice channel.
*/
static IFX_int32_t TAPI_OS_Select(IFX_TAPI_ioctlCtx_t* pCtx,
                                  IFX_int32_t nArgument)
{
   IFX_int32_t ret = 0;
   TAPI_DEV* pTapiDev = IFX_NULL;
   IFX_uint8_t  i, needWakeup = 0;

   if (pCtx == IFX_NULL)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("Wrong input arguments. "
            "(File: %s, line: %d)\n", __FILE__, __LINE__));
      return IFX_ERROR;
   }

   if (TAPI_IOC_CTX_CH == pCtx->nContext)
   {
      if (pCtx->pTapiCh->bInitialized == IFX_TRUE)
      {
         /* handle channel file descriptors */
         ret = TAPI_SelectCh(pCtx->pTapiCh, nArgument, 0);

         if ((ret & TAPI_OS_SYSREAD) &&
             (selWakeupType((SEL_WAKEUP_NODE*) nArgument) == SELREAD))
         {
            selWakeup((SEL_WAKEUP_NODE*) nArgument);
         }

         if ((ret & TAPI_OS_SYSWRITE) &&
             (selWakeupType((SEL_WAKEUP_NODE*) nArgument) == SELWRITE))
         {
            selWakeup((SEL_WAKEUP_NODE*) nArgument);
         }
      }

      /* Select() requires a return value of zero (OK, IFX_SUCCESS). */
   }
   else
   {
      /* handle device file descriptor */
      pTapiDev = pCtx->pTapiCh->pTapiDevice;

      /* add node to the wake up list */
      selNodeAdd(&(pTapiDev->wqEvent), (SEL_WAKEUP_NODE *) nArgument);
      pTapiDev->bNeedWakeup = IFX_TRUE;

      /* check if any data channels have events available */
      for (i=0, needWakeup=0;
           (i < pTapiDev->nMaxChannel) && (needWakeup == 0); i++)
      {
         TAPI_CHANNEL *pTapiCh = &pTapiDev->pChannel[i];

         if ((IFX_TRUE == pTapiCh->bInitialized) &&
             (IFX_FALSE == IFX_TAPI_EventFifoEmpty(pTapiCh)))
         {
            needWakeup++;
         }
      }

      if (needWakeup != 0)
      {
         /* events are pending - wake up the task */
         selWakeup((SEL_WAKEUP_NODE *) nArgument);
         pTapiDev->bNeedWakeup = IFX_FALSE;
      }
   }

   return IFX_SUCCESS;
} /* TAPI_OS_select() */

/**
   Handles the FIOUNSELECT system call.

   \param  pCtx         Pointer to IOCTL context struct.
   \param  nArgument    Node argument of FIOUNSELECT call.

   \return IFX_SUCCESS / IFX_ERROR

   \remarks
   The set of wake up queues are different for the VINETIC configuration/control
   channel and for the voice channel.
*/
static IFX_int32_t TAPI_OS_Unselect(IFX_TAPI_ioctlCtx_t* pCtx,
                                    IFX_int32_t nArgument)
{
   if (pCtx == IFX_NULL)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("Wrong input arguments. "
            "(File: %s, line: %d)\n", __FILE__, __LINE__));
      return IFX_ERROR;
   }

   if (TAPI_IOC_CTX_CH == pCtx->nContext)
   {
      /* handle channel file descriptors */

      /* Voice Streaming */
      selNodeDelete(&(pCtx->pTapiCh->wqRead), (SEL_WAKEUP_NODE *) nArgument);
#ifdef TAPI_FEAT_FAX_T38
      selNodeDelete(&(pCtx->pTapiCh->wqWrite), (SEL_WAKEUP_NODE *) nArgument);
#endif /* TAPI_FEAT_FAX_T38 */
      /* clear */
      pCtx->pTapiCh->nFlags &= ~CF_NEED_WAKEUP;
   }
   else
   {
      selNodeDelete(&(pCtx->pTapiCh->pTapiDevice->wqEvent), (SEL_WAKEUP_NODE *) nArgument);
   }

   return IFX_SUCCESS;
}


/* --------------------------------------------------------------------------
                             SELECT mechanism  -->  END
   --------------------------------------------------------------------------
*/

/* --------------------------------------------------------------------------
                             TIMER mechanisms  -->  BEGIN
   --------------------------------------------------------------------------
*/

/* New usage of timers. We have high priority task which waits for message
   pointer to timer struct or timer struct. When he gets it it starts func with
   parameters (both arguments located in timer struct).
   Timer are used as before (create it, start it, stop it, ..) BUT when timer
   elapses message with usefull data is send.
   When timer is created also array of timer struct is created.
 */

/** Message queue ID, holding funcs with arguments to be handled
    after timer has elapsed. */
static MSG_Q_ID nTimerMsgQueue = 0;

/** Task ID for high priority task handling timer messages */
static IFX_int32_t nTimerMsgHandler_ID = -1;

/** Max. messages in queue. */
enum { MSG_CNT_TIMER_QUEUE = 32 * 8 };

/** Message size in queue. */
const IFX_int32_t MSG_SIZE_TIMER_QUEUE = sizeof(IFX_int32_t);

/** Initial size of array in element count when created. */
static IFX_int32_t START_ELEM_CNT = 10;

/** Increase number of elements for array if full. */
enum { INCREASE_ELEM_CNT = 5 };

/** Holding timer information. When timer will elapse function
    with following prototype will be called : func(timer, arguments) */
typedef struct _TIMER_STRUCT_t
{
   /** Pointer to the function that is executed when the timer elapses. */
   TIMER_ENTRY pFunc;
   /** 1st Argument to the function that is executed when the timer elapses. */
   timer_t Timer;
   /** 2nd Argument to the function that is executed when the timer elapses. */
   IFX_int32_t nArg;
} TIMER_STRUCT_t;

/** Array of timer struct. */
static TIMER_STRUCT_t* rgTimers = IFX_NULL;

/** Number of timer struct in array. */
static IFX_int32_t nTimersCnt = 0;

/** Number of used timer struct in array. */
static IFX_int32_t nUsedTimers = 0;

#ifdef SYS_TIMERLIB_VXWORKS_H
   extern timer_t InstallTimer(
                        TIMER_ENTRY pTimerEntry,
                        IFX_uint32_t nTime,
                        IFX_int32_t nArgument,
                        IFX_boolean_t bEnableTimer);

   extern BOOL StopTimer(
                        timer_t Timer);

   extern BOOL DeleteTimer(
                        timer_t Timer);

   extern BOOL SetTimeTimer(
                        timer_t Timer,
                        IFX_uint32_t nTime,
                        IFX_boolean_t bPeriodically,
                        IFX_boolean_t bRestart);
#endif /* SYS_DRV_TIMERLIB_VXWORKS_H */

/**
   When timer elapses this function is called and will send message
   to message queue.

   \param  Timer        Timer ID (unused).
   \param  nArg         index in the timer array.
 */
static IFX_void_t tapi_timer_SendMsg(timer_t Timer, IFX_uint32_t nArg)
{
   IFX_return_t ret = IFX_SUCCESS;

   /* Check if message queue exists */
   if (nTimerMsgQueue == IFX_NULL)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("Message queue is missing.\n"));
      return;
   }

   /* Just make copy of ptr and send it. */
   ret = msgQSend(nTimerMsgQueue, /* Message queue ID */
                  (IFX_char_t *) &nArg, /* Message, just pointer to it */
                   MSG_SIZE_TIMER_QUEUE, /* Message len */
                   NO_WAIT, MSG_PRI_NORMAL);

   if (ret != IFX_SUCCESS)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
            ("%s: Error sending message, errno %d.\n", __FUNCTION__, errno));
   }
}


/**
   This function will read message queue and start func.

   After the timer has elapsed message was send with msgQSend()
   this handler is waiting for it and handle it.
 */
static IFX_void_t tapi_timer_HandleTimerMsg(void)
{
   IFX_int32_t ret = IFX_SUCCESS;
   TIMER_STRUCT_t* pParam = IFX_NULL;
   IFX_int32_t timer_idx = 0;


   /* Check if message queue exists */
   if (nTimerMsgQueue == IFX_NULL)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("Message queue is missing.\n"));
      return;
   }

   for(;;)
   {
      /* Wait for message, got number of bytes read or error (-1). */
      ret = msgQReceive(nTimerMsgQueue,
                        (IFX_char_t *) &timer_idx,
                        MSG_SIZE_TIMER_QUEUE,
                        WAIT_FOREVER);

      if ((ret == ERROR) || (ret < MSG_SIZE_TIMER_QUEUE))
      {
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("Error receiving message %d.\n", ret));
      }
      else
      {
         if ((0 > timer_idx) || (nTimersCnt < timer_idx))
         {
            TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("Timer message with wrong value received %d\n", timer_idx));
            return;
         }

         /* Call function to handle this event. This one will also
            free buffer, put them back to bufferpool. */
         pParam = &rgTimers[timer_idx - 1];

         pParam->pFunc(pParam->Timer, (IFX_int32_t) pParam->nArg);
      }
   }
}


/**
   Initialize mesage queue and starts message queue handler.

   \return IFX_SUCESS on ok, otherwise IFX_ERROR.
*/
static IFX_return_t tapi_timer_StartMsgQueue(void)
{
   IFX_return_t ret = IFX_SUCCESS;

   /* Create message queue for dispatching events */
   nTimerMsgQueue = msgQCreate(MSG_CNT_TIMER_QUEUE,
                               MSG_SIZE_TIMER_QUEUE,
                               0 /* MSG_Q_FIFO */);

   if (nTimerMsgQueue == IFX_NULL)
   {
      /* Error creating mesage queue. */
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("Error creating message queue.\n"));

      ret = IFX_ERROR;
   }

   if ((ret != IFX_ERROR) && (nTimerMsgHandler_ID == -1))
   {
      /* Create task which will read events from message queue and
         call func to handle them */
      nTimerMsgHandler_ID = taskSpawn("tTimerMsgHandler",
                                      TSK_PRIO_TIMER_HANDLER,
                                      0, /* Options */
                                      8192, /* Stack size */
                                      (FUNCPTR) tapi_timer_HandleTimerMsg,
                                      /* 10 arguments */
                                      0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

      if (nTimerMsgHandler_ID == IFX_ERROR)
      {
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("Error creating task.\n"));
         ret = IFX_ERROR;
      }
   }

   return ret;
}


/**
   Search for free timer struct in array.

   \param nTimerIdx - array index of free element in array

   \return pointer to timer structure, otherwise IFX_NULL.
 */
static TIMER_STRUCT_t* tapi_timer_GetFreeElem(IFX_int32_t* nTimerIdx)
{
   TIMER_STRUCT_t* free_struct = IFX_NULL;
   IFX_int32_t i = 0;
   IFX_void_t* increased_part = IFX_NULL;


   if (nTimerIdx == IFX_NULL)
   {
      /* Wrong input arguments */
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
           ("Invalid input argument(s). (File: %s, line: %d)\n",
            __FILE__, __LINE__));
      return IFX_NULL;
   }

   if (rgTimers == IFX_NULL)
   {
      nTimersCnt = START_ELEM_CNT;

      /* Allocate memory for timers. */
      /*          dev * ch * max_timers elements, SIZE - size of struct */
      rgTimers = calloc(nTimersCnt, sizeof(TIMER_STRUCT_t));
      if (rgTimers == IFX_NULL)
      {
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
               ("Error creating array of timer structs."
                "(File: %s, line: %d)\n", __FILE__, __LINE__));
         return IFX_NULL;
      }
   }

   free_struct = IFX_NULL;

   /* Search for free struct */
   for (i = 0; i < nTimersCnt; i++)
   {
      if (rgTimers[i].pFunc == IFX_NULL)
      {
         /* Got free struct */
         free_struct = &rgTimers[i];
         *nTimerIdx = i + 1;
         break;
      }
   }

   if (free_struct == IFX_NULL)
   {
      /* Array of timer struct is full, try to increase it */
      increased_part = realloc(rgTimers, (nTimersCnt + INCREASE_ELEM_CNT)
                                          * sizeof(TIMER_STRUCT_t));
      if (increased_part == IFX_NULL)
      {
         TRACE(TAPI_DRV, DBG_LEVEL_NORMAL,
               ("Error increasing array of timer structs."
                " (File: %s, line: %d)\n", __FILE__, __LINE__));
         return IFX_NULL;
      }
      else
      {
         rgTimers = increased_part;
      }

      /* Set to zero increased memory. */
      memset(&rgTimers[nTimersCnt], 0,
             INCREASE_ELEM_CNT * sizeof(TIMER_STRUCT_t));

      /* Set free_struct to first timer in mem which was increased */
      free_struct = &rgTimers[nTimersCnt];
      *nTimerIdx = nTimersCnt + 1;

      /* Increase count of timers */
      nTimersCnt += INCREASE_ELEM_CNT;
   }

   /* Also increase number of used timer structs. */
   nUsedTimers++;

   return free_struct;
}


/**
   Create a timer.

   \param  pTimerEntry  Functionpointer to the call-back function.
   \param  nArgument    Argument to be passed to the call-back function.

   \return Generic Pointer to timer.
*/
Timer_ID TAPI_Create_Timer(TIMER_ENTRY pTimerEntry, IFX_ulong_t nArgument)
{
   /* Use common timer functions if sys_timerlib_vxworks.c is included
      (done in BSPProj.c) */
   TIMER_STRUCT_t* free_struct = IFX_NULL;
   IFX_int32_t timer_idx = 0;

   TAPI_OS_MutexGet (&semTimerArrDataLock);

   if (nTimerMsgQueue == 0)
   {
      tapi_timer_StartMsgQueue();
   }

   free_struct = tapi_timer_GetFreeElem(&timer_idx);
   if (free_struct == IFX_NULL)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("Err getting free timer struct elem"
            " from array. (File: %s, line: %d)\n", __FILE__, __LINE__));

      TAPI_OS_MutexRelease (&semTimerArrDataLock);

      return (Timer_ID)(0);
   }

   TAPI_OS_MutexRelease (&semTimerArrDataLock);

   free_struct->nArg = nArgument;
   free_struct->pFunc = pTimerEntry;

#ifndef SYS_TIMERLIB_VXWORKS_H
   /* Derive timer from CLK realtimer, do not use signal handler. */
   if (timer_create(CLOCK_REALTIME, NULL, &free_struct->Timer) == IFX_ERROR)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("Err creating timer. "
            "(File: %s, line: %d)\n", __FILE__, __LINE__));

      /* Set also that this timer struct is free, because could not create timer. */
      free_struct->pFunc = IFX_NULL;

      return (Timer_ID)(0);
   }

   /* Connect timer to function, which will send message with arguments. */
   if (timer_connect(free_struct->Timer, tapi_timer_SendMsg,
                     (IFX_int32_t) timer_idx) == IFX_ERROR)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("Err connecting to timer. "
            "(File: %s, line: %d)\n", __FILE__, __LINE__));

      /* Set also that this timer struct is free, because could not create timer. */
      free_struct->pFunc = IFX_NULL;

      return (Timer_ID)(0);
   }
#else
   free_struct->Timer = InstallTimer(tapi_timer_SendMsg, 0, timer_idx,
      IFX_FALSE);
#endif

   return (Timer_ID)(free_struct->Timer);
}


/**
   Sets a timer to the specified time and starts it.

   \param  Timer        Generic Pointer to timer.
   \param  nTime        Time in ms.
   \param  bPeriodically Starts the timer periodically or not.
   \param  bRestart     Restart the timer or normal start.

   \return Returns an error code: IFX_TRUE / IFX_FALSE
*/
IFX_boolean_t TAPI_SetTime_Timer(Timer_ID timer_id,
                                 IFX_uint32_t nTime,
                                 IFX_boolean_t bPeriodically,
                                 IFX_boolean_t bRestart)
{
   timer_t Timer = (timer_t)timer_id;

   /* Use common timer functions if sys_timerlib_vxworks.c is included (done in BSPProj.c). */
#ifndef SYS_TIMERLIB_VXWORKS_H
   struct itimerspec   timeToSet;        /* time to be set */
   struct timespec     timeValue;        /* timer expiration value */
   struct timespec     timeInterval;     /* timer period */
   int flags = 0;


   /* Stop timer. */
   if (bRestart == IFX_TRUE)
   {
      if (timer_cancel(Timer) == IFX_ERROR)
      {
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("Err cancelling timer. "
               "(File: %s, line: %d)\n", __FILE__, __LINE__));

         return (IFX_FALSE);
      }
   }

   /* With timeout of 0 timer should fire immediately */
   if (nTime == 0)
   {
      flags = TIMER_ABSTIME;
      nTime = 1;
   }
   /* Initialize timer expiration value. */
   timeValue.tv_sec = (nTime / 1000);
   timeValue.tv_nsec = (nTime % 1000) * 1000 * 1000;

   /* Initialize timer period */
   if (bPeriodically == IFX_TRUE)
   {
      timeInterval.tv_sec = (nTime / 1000);
      timeInterval.tv_nsec = (nTime % 1000) * 1000 * 1000;
   }
   else
   {
      timeInterval.tv_sec = 0;
      timeInterval.tv_nsec = 0;
   }

   /* Reset timer structure. */
   memset((IFX_char_t *) &timeToSet, 0, sizeof (struct itimerspec));

   /* Set the time to be set value. */
   /* NOTICE: Copy all parameter in simple steps. This is a workaround to avoid
      crashes on the CheckBoard, because of incompatiple compiler versions. */
   timeToSet.it_value.tv_sec = timeValue.tv_sec;
   timeToSet.it_value.tv_nsec = timeValue.tv_nsec;
   timeToSet.it_interval.tv_sec = timeInterval.tv_sec;
   timeToSet.it_interval.tv_nsec = timeInterval.tv_nsec;

   /* Pass timer value & reload value. */
   if (timer_settime(Timer, flags, &timeToSet, IFX_NULL) == IFX_ERROR)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("Err setting time for timer. "
            "(File: %s, line: %d)\n", __FILE__, __LINE__));

      return (IFX_FALSE);
   }

   return (IFX_TRUE);
#else
   return ( SetTimeTimer(Timer, nTime, bPeriodically, bRestart) );
#endif
}


/**
   Stop a timer.

   \param  Timer        Generic Pointer to timer.

   \return Returns an error code: IFX_TRUE / IFX_FALSE
*/
IFX_boolean_t TAPI_Stop_Timer(Timer_ID timer_id)
{
   timer_t Timer = (timer_t)timer_id;

#ifndef SYS_TIMERLIB_VXWORKS_H
   /* Stop timer. */
   if (timer_cancel(Timer) == IFX_ERROR)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("Err cancelling a timer. "
            "(File: %s, line: %d)\n", __FILE__, __LINE__));

      return (IFX_FALSE);
   }

   return (IFX_TRUE);
#else
   return ( StopTimer(Timer) );
#endif
}


/**
   Search for used struct and free it (its not used by this timer anymore).

   \param  Timer        Timer ID handle for vxWorks.
*/
static IFX_void_t tapi_timer_RemoveStruct(timer_t* Timer)
{
   IFX_int32_t i = 0;

   if (Timer == IFX_NULL)
   {
      /* Wrong input arguments */
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
           ("Invalid input argument(s). (File: %s, line: %d)\n",
            __FILE__, __LINE__));
      return;
   }

   TAPI_OS_MutexGet (&semTimerArrDataLock);

   /* Search for free struct */
   for (i = 0; i < nTimersCnt; i++)
   {
      if (*Timer == rgTimers[i].Timer)
      {
         /* Set to unused */
         memset(&rgTimers[i], 0, sizeof(TIMER_STRUCT_t));

         /* Decrease number of used timer structs. */
         nUsedTimers--;

         if (nUsedTimers == 0)
         {
            /* Release array of timer structs, because its empty. */
            free(rgTimers);
            rgTimers = IFX_NULL;
         }

         break;
      }
   }

   TAPI_OS_MutexRelease (&semTimerArrDataLock);
}


/**
   Delete a timer.

   \param  Timer        Generic Pointer to timer.

   \return Returns an error code: IFX_TRUE / IFX_FALSE
*/
IFX_boolean_t TAPI_Delete_Timer(Timer_ID timer_id)
{
   timer_t Timer = (timer_t)timer_id;

#ifndef SYS_TIMERLIB_VXWORKS_H
   /* Stop timer. */
   if (timer_cancel(Timer) == IFX_ERROR)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("Err cancelling a timer. "
            "(File: %s, line: %d)\n", __FILE__, __LINE__));
      return (IFX_FALSE);
   }

   /* Delete timer. */
   if (timer_delete(Timer) == IFX_ERROR)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("Err deleting timer. "
            "(File: %s, line: %d)\n", __FILE__, __LINE__));
      return (IFX_FALSE);
   }

   tapi_timer_RemoveStruct(&Timer);

   return (IFX_TRUE);
#else
   tapi_timer_RemoveStruct(&Timer);

   return ( DeleteTimer(Timer) );
#endif
}


/* --------------------------------------------------------------------------
                             TIMER mechanisms  -->  END
   --------------------------------------------------------------------------
*/


/* --------------------------------------------------------------------------
                             EVENT handling  -->  BEGIN
   --------------------------------------------------------------------------
*/

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

   if (pParam == IFX_NULL)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("Wrong input arguments."
            "(File: %s, line: %d)\n", __FILE__, __LINE__));
      return IFX_ERROR;
   }

   if (nMsgQueue_Events != IFX_NULL)
   {
      /* We need a variable that we can take the address of. */
      IFX_void_t *tmp_param = pParam;

      /* Copy the pointer value into the queue. */
      ret = msgQSend(nMsgQueue_Events, /* Message queue ID */
                     (IFX_char_t *) &tmp_param, /* Message */
                     MSG_SIZE_EVENT_QUEUE, /* Message len */
                     NO_WAIT,
                     MSG_PRI_NORMAL);
      if (ret != IFX_SUCCESS)
      {
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
               ("TAPI_EVENT: Error sending messsage, errno %d.\n", errno));
      }
   }
   else
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
            ("Defer TAPI event: No message queue, so won't send message.\n"));

      ret = IFX_ERROR;
   }

   return ret;
}

/**
   This function will read message queue for events.

   \remarks
   When message is send with msgQSend() this handler is waiting for it
   and dispatch it to event handler.
*/
static IFX_void_t IFX_TAPI_Event_Dispatch_Queue(void)
{
   IFX_return_t ret = IFX_SUCCESS;
   IFX_TAPI_EXT_EVENT_PARAM_t* pParam = IFX_NULL;

   /* Check if message queue exists */
   if (nMsgQueue_Events == IFX_NULL)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("Message queue is missing.\n"));
      return;
   }

   for (;;)
   {
      /* Wait for message */
      ret = msgQReceive(nMsgQueue_Events,
                        (IFX_char_t *) &pParam,
                        MSG_SIZE_EVENT_QUEUE,
                        WAIT_FOREVER);
      if ((ret == ERROR) || (ret < MSG_SIZE_EVENT_QUEUE))
      {
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
               ("Error receiving message %d. (errno=0x%08x)\n", ret, errno));
      }
      else
      {
         if (IFX_TAPI_Event_Dispatch_ProcessCtx(pParam) != IFX_SUCCESS)
         {
            TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("Error when processing event.\n"));
         }
      }
   }
}


/**
   Initialize mesage queue and starts message queue handler.

   \param none

   \return IFX_SUCESS on ok, otherwise IFX_ERROR.
*/
static IFX_return_t ifx_tapi_Event_StartMsgQueue(void)
{
   IFX_return_t ret = IFX_SUCCESS;

   if (nMsgQueue_Events != IFX_NULL)
   {
      /* message queue already created */
      return IFX_SUCCESS;
   }

   /* Create message queue for dispatching events */
   nMsgQueue_Events = msgQCreate(MAX_MSG_CNT_EVENT_QUEUE,
                                 MSG_SIZE_EVENT_QUEUE,
                                 0 /* MSG_Q_FIFO */);
   if (IFX_NULL == nMsgQueue_Events)
   {
      /* Error creating mesage queue. */
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("Error creating message queue.\n"));
      ret = IFX_ERROR;
   }

   if ((ret != IFX_ERROR) && (nTaskForEvents_ID == -1))
   {
      TRACE(TAPI_DRV, DBG_LEVEL_NORMAL,
            ("Start task for handling msg queue with events.\n"));

      /* Create task which will read events from message queue and
         calls functions to handle them before putting it into the
         fifo towards the application */
      /* NOTE: spawned event handling task names were united accross
               AN Voice drivers, please keep the practice. */
      nTaskForEvents_ID = taskSpawn("tTAPI_EventHandler",
                                    TSK_PRIO_EVENT_HANDLER,
                                    0, /* Options */
                                    8192, /* Stack size */
                                    (FUNCPTR) IFX_TAPI_Event_Dispatch_Queue,
                                    /* 10 arguments */
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

      if (nTaskForEvents_ID == IFX_ERROR)
      {
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("Error creating task.\n"));
         ret = IFX_ERROR;
      }
   }

   return ret;
}


#if 0 /* For future use */
/**
   Deletes mesage queue and stops message queue handler.

   \pararm none

   \return IFX_SUCCESS on ok, otherwise IFX_ERROR.
*/
IFX_int32_t IFX_TAPI_Event_StopMsgQueue(void)
{
   IFX_int32_t ret = IFX_SUCCESS;

   /* Delete task */
   if (taskDelete(nTaskForEvents_ID) != IFX_SUCCESS)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("Error deleting task.\n"));
      ret = IFX_ERROR;
   }
   /* set to inital state even if kill failed */
   nTaskForEvents_ID = -1;

   /* Delete message queue (try even if task delete failed) */
   if (msgQDelete(nMsgQueue_Events) != IFX_SUCCESS)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH, ("Error deleting message queue.\n"));
      ret = IFX_ERROR;
   }
   /* set to inital state even if delete failed */
   nMsgQueue_Events = 0;

   return ret;
}
#endif /* if 0 */

int tapiDrvShow ()
{
   IFX_TAPI_DRV_CTX_t *pDrvCtx = IFX_NULL;
   int i, len = 0;

   for (i = 0; i < TAPI_MAX_LL_DRIVERS; i++)
   {
      if (gHLDrvCtx [i].pDrvCtx != IFX_NULL)
      {
         pDrvCtx = gHLDrvCtx [i].pDrvCtx;
         printf ("Driver \t  version \t major \t devices devname \n\r");
         printf ("==================================================\n\r");
         printf ("%s    %s \t %d \t %d \t /dev/%s\n\r",
              pDrvCtx->drvName, pDrvCtx->drvVersion,
              pDrvCtx->majorNumber,pDrvCtx->maxDevs, pDrvCtx->devNodeName);
      }
   }

   return len;
}

#endif /* VXWORKS */
