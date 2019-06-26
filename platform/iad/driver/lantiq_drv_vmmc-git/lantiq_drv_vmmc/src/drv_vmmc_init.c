/******************************************************************************

                            Copyright (c) 2006-2016
                        Lantiq Beteiligungs-GmbH & Co.KG
                             http://www.lantiq.com

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/**
   \file drv_vmmc_init.c
   This file implements the initialisation sequence.
*/

/* ============================= */
/* Includes                      */
/* ============================= */
#include "drv_api.h"
#include "drv_vmmc_init.h"
#include "drv_vmmc_api.h"
#include "drv_vmmc_bbd.h"
#include "drv_version.h"
#include "drv_vmmc_int.h"
#include "drv_vmmc_res.h"
#include "drv_vmmc_con.h"
#include "drv_vmmc_cod.h"
#include "drv_vmmc_sig.h"
#include "drv_vmmc_alm.h"
#ifdef VMMC_FEAT_PCM
#include "drv_vmmc_pcm.h"
#endif /* VMMC_FEAT_PCM */
#ifdef DECT_SUPPORT
#include "drv_vmmc_dect.h"
#endif /* DECT_SUPPORT */
#include "drv_vmmc_stream.h"

/* for ifx_mps_bufman_register we need */
#include "drv_mps_vmmc.h"
#include "drv_mps_vmmc_device.h"

#ifdef VMMC_FEAT_CLOCK_SCALING
#include "drv_vmmc_pmc.h"
#endif /* VMMC_FEAT_CLOCK_SCALING */

#include "drv_vmmc_announcements.h"


/* ============================= */
/* Local Macros & Definitions    */
/* ============================= */

/* Define trace output for vmmc
   OFF:    no output
   HIGH:   only important traces, as errors or some warnings
   NORMAL: including traces from high and general proceedings and
           possible problems
   LOW:    all traces and low level traces as basic chip access
           and interrupts, command data
   Traces can be completely switched off with the compiler switch
   ENABLE_TRACE to 0
*/
CREATE_TRACE_GROUP(VMMC);

#ifndef _MKSTR_1
#define _MKSTR_1(x)    #x
#define _MKSTR(x)      _MKSTR_1(x)
#endif

/** driver version string */
#define DRV_VMMC_VER_STR         _MKSTR(MAJORSTEP)   "."   \
                                 _MKSTR(MINORSTEP)   "."   \
                                 _MKSTR(VERSIONSTEP) "."   \
                                 _MKSTR(VERS_TYPE)

/** low-level API version string */
#define DRV_LL_INTERFACE_VER_STR _MKSTR(LL_IF_MAJORSTEP)   "." \
                                 _MKSTR(LL_IF_MINORSTEP)   "." \
                                 _MKSTR(LL_IF_VERSIONSTEP) "." \
                                 _MKSTR(LL_IF_VERS_TYPE)

/** what compatible driver version */
#define DRV_VMMC_WHAT_STR "@(#)Lantiq VMMC device driver, version " DRV_VMMC_VER_STR

#define MAX(x,y) ((x) > (y) ? (x) : (y))


/* ============================= */
/* Global variable definition    */
/* ============================= */
extern IFX_uint16_t major;
extern IFX_uint16_t minorBase;
extern IFX_char_t *devName;

/* Variable that is used to set the initial debug trace level.
   In some OS implementations it can be overwritten when starting the driver.
   debug_level: LOW (1), NORMAL (2), HIGH (3), OFF (4) */
#ifdef ENABLE_TRACE
#ifdef DEBUG
IFX_uint32_t debug_level = DBG_LEVEL_LOW;
#else
IFX_uint32_t debug_level = DBG_LEVEL_HIGH;
#endif /* DEBUG */
#endif /* ENABLE_TRACE */

/** what string support, driver version string */
const IFX_char_t DRV_VMMC_WHATVERSION[] = DRV_VMMC_WHAT_STR;
#ifdef HAVE_CONFIG_H
/** which configure options were set */
const IFX_char_t DRV_VMMC_WHICHCONFIG[] = VMMC_CONFIGURE_STR;
#endif /* HAVE_CONFIG_H */


/* ============================= */
/* Local variable definition     */
/* ============================= */
/* static variable of the driver context struct */
static IFX_TAPI_DRV_CTX_t DrvCtx;
/* static array of device structs */
static VMMC_DEVICE VDevices[VMMC_MAX_DEVICES];


/* ============================= */
/* Prototypes                    */
/* ============================= */
extern IFX_void_t Vmmc_IrqEnable (IFX_TAPI_LL_DEV_t* pLLDev);
extern IFX_void_t Vmmc_IrqDisable (IFX_TAPI_LL_DEV_t* pLLDev);
#ifdef VMMC_WITH_MPS
extern int  ifx_mps_init_module(void);
extern void ifx_mps_cleanup_module(void);
#endif
#ifdef VMMC_FEAT_VPE1_SW_WD
extern IFX_int32_t VMMC_WDT_Callback(IFX_uint32_t flags);
#endif /* VMMC_FEAT_VPE1_SW_WD */

extern IFX_int32_t VMMC_XRX100_PcmGpioRelease(
                              void);

static IFX_TAPI_LL_DEV_t*  VMMC_TAPI_LL_DevicePrepare (
                              TAPI_DEV* pTapiDev,
                              IFX_uint32_t devNum);

static IFX_int32_t         VMMC_TAPI_LL_DeviceInit (
                              IFX_TAPI_LL_DEV_t* pLLDev);

static IFX_void_t          VMMC_TAPI_LL_DeviceExit (
                              IFX_TAPI_LL_DEV_t *pLLDev,
                              IFX_boolean_t bChipAccess);

static IFX_void_t          vmmc_DeviceExit (
                              VMMC_DEVICE *pDev,
                              IFX_boolean_t bChipAccess);

static IFX_TAPI_LL_CH_t*   VMMC_TAPI_LL_ChannelPrepare(
                              TAPI_CHANNEL *pTapiCh,
                              IFX_TAPI_LL_DEV_t *pLLDev,
                              IFX_uint32_t chNum);

static IFX_int32_t         VMMC_TAPI_LL_ChannelInit (
                              IFX_TAPI_LL_CH_t *pLLCh);

static IFX_void_t          vmmc_ChannelStop (
                              VMMC_CHANNEL *pCh);

static IFX_void_t          vmmc_ChannelExit (
                              VMMC_CHANNEL *pCh);

static IFX_void_t  vmmc_ChipAccessExit  (VMMC_DEVICE *pDev);
static IFX_int32_t vmmc_ClearPendingInt (VMMC_DEVICE *pDev);

static IFX_int32_t VMMC_TAPI_LL_FW_Start(IFX_TAPI_LL_DEV_t *pLLDev,
                                         IFX_void_t const *pProc);
static IFX_int32_t VMMC_TAPI_LL_FW_Init (IFX_TAPI_LL_DEV_t *pLLDev,
                                         IFX_uint8_t nMode);
static IFX_int32_t vmmc_FW_VersionRead  (VMMC_DEVICE *pDev);
static IFX_int32_t vmmc_FW_DevDataInit  (VMMC_DEVICE *pDev);

static IFX_int32_t vmmc_Basic_VoIPConf  (VMMC_CHANNEL *pCh,
                                         IFX_uint8_t nMode);


/* ============================= */
/* Global function definition    */
/* ============================= */

/**
   Get a pointer to the device struct.

   \param  nr           Number of the device. Counting starts with zero.
   \param  pDev         Returns pointer to a device structure.

   \return
   IFX_SUCCESS if ok, otherwise IFX_ERROR
*/
IFX_int32_t VMMC_GetDevice (IFX_uint16_t nr, VMMC_DEVICE** pDev)
{
   if (nr >= VMMC_MAX_DEVICES)
   {
      *pDev = IFX_NULL;
      return IFX_ERROR;
   }

   *pDev = &VDevices[nr];
   return IFX_SUCCESS;
}


/**
   Wrapper for the voice buffer get function that sets the FW as owner.
*/
static IFX_void_t* vmmc_WrapperVoiceBufferGet (void)
{
   return IFX_TAPI_VoiceBufferGetWithOwnerId (IFX_TAPI_BUFFER_OWNER_FW);
}


/**
   Wrapper for the voice buffer free all function freeing all buffers that
   are marked as owned by FW.
*/
static IFX_void_t vmmc_WrapperVoiceBufferFreeAll (void)
{
   IFX_TAPI_VoiceBufferFreeAllOwnerId (IFX_TAPI_BUFFER_OWNER_FW);
}


/**
   Initialise the access to the chip.

   This function allocates all resources needed for access to the chip.

   \param  pDev         Pointer to the device structure.

   \return
   - VMMC_statusOk      if successful
   - VMMC_statusReserveChipAccessFailed Reserving resources failed.

   \remarks
   Must be the first action after installing the driver and
   before any access to the hardware (corresponding vmmc chip).
*/
IFX_int32_t VMMC_ChipAccessInit(VMMC_DEVICE *pDev)
{
   IFX_int32_t ret = VMMC_statusOk;

   /* make sure that we really got a device structure */
   VMMC_ASSERT((pDev != NULL && pDev->nChannel == 0));

   /* Do not initialise the chip access more than one time. */
   if (pDev->nDevState & DS_BASIC_INIT)
   {
      return VMMC_statusOk;
   }

   /* Create semaphores that prevent concurrent chip access. */
   /* initialize the data mailbox protection semaphore */
   VMMC_OS_MutexInit (&pDev->mtxDataMbxAcc);
   /* initialize the command mailbox protection semaphore */
   VMMC_OS_MutexInit (&pDev->mtxCmdMbxAcc);
   /* initialize CmdRead mutex */
   VMMC_OS_MutexInit (&pDev->mtxCmdReadAcc);
   /* Init event that indicates that a command packet arrived in mailbox. */
   VMMC_OS_EventInit (&pDev->mpsCmdWakeUp);

   /* Note: This driver does not have an address range for the chip or an
            interrupt. So no address range or interrupt needs to be allocated
            here. The chip access is done via the MPS driver. */

   /* Open the MPS driver */

   /* Register the buffer handler. */
   ifx_mps_bufman_register((IFX_void_t* (*)(IFX_size_t, IFX_int32_t))
                           vmmc_WrapperVoiceBufferGet,
                           (IFX_void_t (*)(const IFX_void_t*))
                           IFX_TAPI_VoiceBufferPut,
                           sizeof(PACKET), POBX_BUFFER_THRESHOLD);
   ifx_mps_register_bufman_freeall_callback (vmmc_WrapperVoiceBufferFreeAll);

   /* Register interrupt handlers and open all channels to the MPS driver */
   ret = VMMC_Register_Callback(pDev);

#ifdef VMMC_FEAT_VPE1_SW_WD
   /* register watchdog timer callback */
   ifx_mps_register_wdog_callback(VMMC_WDT_Callback);
#endif /* VMMC_FEAT_VPE1_SW_WD */

   if (ret == VMMC_statusOk)
   {
      /* Device state: basic init is done */
      pDev->nDevState |= DS_BASIC_INIT;
   }
   else
   {
      TRACE(VMMC, DBG_LEVEL_HIGH,
            ("ERROR: Reserving the resources for chip access failed\n"));
      /* errmsg: Reserving the resources for chip access failed */
      ret = VMMC_statusReserveChipAccessFailed;
   }

   return ret;
}


/**
   Release the access to the chip.

   This function releases all resources needed for access to the chip.

   \param  pDev         Pointer to the device structure.
*/
static IFX_void_t vmmc_ChipAccessExit(VMMC_DEVICE *pDev)
{
   /* make sure that we really got a device structure */
   VMMC_ASSERT((pDev != NULL && pDev->nChannel == 0));

   /* Note: This driver does not have an address range for the chip or an
            interrupt. So no address range or interrupt needs to be released
            here. The chip access is done via the MPS driver. */

   if (pDev->nDevState & DS_BASIC_INIT)
   {
      /* Close all channels of the MPS driver */
      if (IFX_SUCCESS != VMMC_UnRegister_Callback())
      {
         TRACE (VMMC, DBG_LEVEL_HIGH,
               ("%s: VMMC_UnRegister_Callback failed\n", __FUNCTION__));
      }

#ifdef VMMC_FEAT_VPE1_SW_WD
      ifx_mps_register_wdog_callback(NULL);
#endif /* VMMC_FEAT_VPE1_SW_WD */

      /* Delete semaphores that prevent concurrent chip access. */
      VMMC_OS_MutexDelete (&pDev->mtxDataMbxAcc);
      VMMC_OS_MutexDelete (&pDev->mtxCmdMbxAcc);
      VMMC_OS_MutexDelete (&pDev->mtxCmdReadAcc);

      /* Device state: basic init is not done */
      pDev->nDevState &= ~DS_BASIC_INIT;
   }
}


/**
   Clears all pending interrupts.

   This function clears pending interrupts after a reset operation.

   \param  pDev              Pointer to the device structure.

   \return
   IFX_SUCCESS or IFX_ERROR

   \remarks
   This function must be save from interrupts.
*/
/*lint -e{715} currently there is just one device */
static IFX_int32_t vmmc_ClearPendingInt(VMMC_DEVICE *pDev)
{
   VMMC_UNUSED(pDev);

   *IFX_MPS_CAD0SR = 0xffffffff;

   return IFX_SUCCESS;
}


/**
   Prepare the low level device struct.

   This function clears the device struct, sets the device number and links it
   with the high-level device.

   \param  pTapiDev     Pointer to the high-level device struct.
   \param  devNum       Device number.

   \return
   Pointer to VMMC Device or IFX_NULL.
*/
IFX_TAPI_LL_DEV_t* VMMC_TAPI_LL_DevicePrepare(TAPI_DEV *pTapiDev,
                                              IFX_uint32_t devNum)
{
   VMMC_DEVICE* pDev;

   /* make sure a valid context is given */
   if (devNum >= VMMC_MAX_DEVICES)
   {
      TRACE(VMMC, DBG_LEVEL_HIGH,
            ("VMMC_LL_Prepare_Dev: VMMC device number out of range\n"));
      return IFX_NULL;
   }
   if (pTapiDev == IFX_NULL)
   {
      TRACE(VMMC, DBG_LEVEL_HIGH,
            ("VMMC_LL_Prepare_Dev: pTapiDev is NULL\n"));
      return IFX_NULL;
   }

   pDev = &VDevices[devNum];

   /* Clear the device struct (including the channels) */
   memset(pDev, 0, sizeof(VMMC_DEVICE));

#ifdef DEBUG
   /* Set the magic on the device and the included channel structures.
      This magic can help to locate the structs within a memory dump. */
   {
      IFX_uint16_t i;

      pDev->magic = VDEV_MAGIC;

      for (i = 0; i < VMMC_MAX_CH_NR; i++)
      {
         pDev->pChannel[i].magic = VCH_MAGIC;
      }
   }
#endif /* DEBUG */

   pDev->nDevNr = devNum;

   /* Store the corresponding HL pointer */
   pDev->pTapiDev = pTapiDev;

   VMMC_AddCaps (pDev);

   /* Return the pDev pointer which is stored in the HL device */
   return pDev;
}


/**
   Initialise the low level device struct.

   Initialise the member variables of the device structure.

   \param  pLLDev       Pointer to the device structure.

   \return
      VMMC_statusOk if successful else device specific return code.
*/
IFX_int32_t VMMC_TAPI_LL_DeviceInit(IFX_TAPI_LL_DEV_t* pLLDev)
{
   VMMC_DEVICE *pDev = (VMMC_DEVICE *)pLLDev;
#ifdef VMMC_FEAT_CLOCK_SCALING
   IFX_int32_t ret = IFX_SUCCESS;
#endif /* VMMC_FEAT_CLOCK_SCALING */

   /* OS and board independent initializations, resets all values */

   /* initialize share variables protection semaphore */
   VMMC_OS_MutexInit (&pDev->mtxMemberAcc);

#ifdef VMMC_FEAT_CLOCK_SCALING
   ret = VMMC_PMC_Init(pDev);
   if (!VMMC_SUCCESS (ret))
   {
       TRACE(VMMC, DBG_LEVEL_HIGH,
            ("ERROR: PMC initialization of Low level failed\n"));
      return ret;
   }
#endif /* VMMC_FEAT_CLOCK_SCALING */

   /* default DC/DC type */
   pDev->sdd.nAllowedDcDcType = VMMC_DCDC_TYPE_IBB;
   /* dedicated DC/DC is the default */
   pDev->sdd.bDcDcHwCombined = IFX_FALSE;

   /* intially there is no error */
   pDev->err = VMMC_statusOk;

   return VMMC_statusOk;
}


/**
   Frees all resources of the device.

   This calls also exit on all channels because they are members of the device.
   Called when the device is released.

   \param  pDev         Pointer to the device structure.
   \param  bChipAccess  Allow or deny chip access in this function.
*/
static IFX_void_t vmmc_DeviceExit(VMMC_DEVICE *pDev,
                                  IFX_boolean_t bChipAccess)
{
   IFX_uint16_t i;

   if (bChipAccess != IFX_FALSE)
   {
      /* call stop on all channels of the device */
      for (i = 0; i < VMMC_MAX_CH_NR; i++)
      {
         vmmc_ChannelStop(&pDev->pChannel[i]);
      }

#ifdef VMMC_FEAT_PCM
      /* The PCM interface is device global and can only be stopped after all
         PCM channels are deactivated. */
      VMMC_PCM_IF_Stop(pDev);
#endif /* VMMC_FEAT_PCM */
   }

#ifdef VMMC_FEAT_ANNOUNCEMENTS
   VMMC_Ann_Cleanup (pDev, bChipAccess);
#endif /* VMMC_FEAT_ANNOUNCEMENTS */

   /* release chip access */
   vmmc_ChipAccessExit(pDev);

   /* call exit on all channels of the device */
   for (i = 0; i < VMMC_MAX_CH_NR; i++)
   {
      vmmc_ChannelExit(&pDev->pChannel[i]);
   }

#ifdef VMMC_FEAT_PCM
   /* reset PCM timeslot management flags */
   memset(pDev->PcmRxTs, 0, sizeof(pDev->PcmRxTs));
   memset(pDev->PcmTxTs, 0, sizeof(pDev->PcmTxTs));
#endif /* VMMC_FEAT_PCM */

   /* clear statistics */
   pDev->nMipsOl = 0;

#ifdef VMMC_FEAT_CLOCK_SCALING
   VMMC_PMC_Exit(pDev);
#endif /* VMMC_FEAT_CLOCK_SCALING */

   /* Free the resources allocated on the device. */
   VMMC_RES_StructuresFree(pDev);

   /* Fill the capabilities again with initial values.
      To have capabilities available during cleanup this is done last. */
   pDev->bCapsRead = IFX_FALSE;
   VMMC_AddCaps (pDev);

   /* delete device mutex */
   VMMC_OS_MutexDelete (&pDev->mtxMemberAcc);

   /* reset the states variable but keep the GPIO flag */
   pDev->nDevState &= DS_GPIO_RESERVED;
}


/**
   Stop the VMMC device and free all allocated resources.

   \param  pLLDev       Pointer to the device structure.
   \param  bChipAccess  Allow or deny chip access in this function.
*/
IFX_void_t VMMC_TAPI_LL_DeviceExit (IFX_TAPI_LL_DEV_t *pLLDev,
                                    IFX_boolean_t bChipAccess)
{
   VMMC_DEVICE  *pDev = (VMMC_DEVICE *)pLLDev;

   /* resource cleanup */
   vmmc_DeviceExit(pDev, bChipAccess);
}


/**
   Prepare the low level channel struct.

   \param  pTapiCh      High level channel pointer.
   \param  pLLDev       Pointer to the device structure.
   \param  chNum        Channel number.

   \return
   Pointer to VMMC Channel or IFX_NULL.
*/
IFX_TAPI_LL_CH_t* VMMC_TAPI_LL_ChannelPrepare(TAPI_CHANNEL *pTapiCh,
                                              IFX_TAPI_LL_DEV_t *pLLDev,
                                              IFX_uint32_t chNum)
{
   VMMC_DEVICE  *pDev = (VMMC_DEVICE *)pLLDev;
   VMMC_CHANNEL *pCh;

   VMMC_ASSERT(pDev != IFX_NULL);

   /* Make sure we have a VMMC channel that we can bind to the TAPI channel. */
   if (chNum >= VMMC_MAX_CH_NR)
   {
      TRACE(VMMC, DBG_LEVEL_HIGH,
            ("No VMMC channel available to bind to TAPI channel %d\n", chNum));
      return IFX_NULL;
   }
   if (pTapiCh == IFX_NULL)
   {
      TRACE(VMMC, DBG_LEVEL_HIGH,
            ("VMMC_TAPI_LL_ChannelPrepare: pTapiCh is NULL\n"));
      return IFX_NULL;
   }

   /* TAPI channel number is identical to the channel index in this driver. */
   pCh  = &(pDev->pChannel[chNum]);

   pCh->nChannel = (IFX_uint8_t)chNum + 1;
   pCh->pParent  = pDev;

   /* Store the corresponding HL TAPI channel pointer */
   pCh->pTapiCh = pTapiCh;

   /* Should return the pCh which should be stored by the HL */
   return pCh;
}


/**
   Initialise the low level channel struct.

   \param  pLLCh        Pointer to the channel structure.

   \return
   IFX_SUCCESS
*/
IFX_int32_t VMMC_TAPI_LL_ChannelInit (IFX_TAPI_LL_CH_t *pLLCh)
{
   VMMC_CHANNEL *pCh = (VMMC_CHANNEL *)pLLCh;

   VMMC_ASSERT(pCh != IFX_NULL);

   /* init channel lock */
   VMMC_OS_MutexInit (&pCh->chAcc);

   /* Defaut Payload type */
   pCh->nEvtPT   = DEFAULT_EVTPT;

   /* allocate timer and status for tone resources */
#if 0
   pCh->pToneRes = VMMC_OS_Malloc (sizeof (VMMC_TONERES) * TAPI_TONE_MAXRES);
   memset (pCh->pToneRes, 0, sizeof (VMMC_TONERES) * TAPI_TONE_MAXRES);
#endif /* 0 */

   pCh->nNoKpiPathError = 0;

   return IFX_SUCCESS;
}


/**
   Stopps all resources of the channel.

   Called when the channel is released.

   \param  pCh          Pointer to the channel.
*/
static IFX_void_t vmmc_ChannelStop(VMMC_CHANNEL *pCh)
{
   /* Deactivate all still running algorithms in the FW */
   VMMC_ALM_ChStop(pCh);
   VMMC_COD_ChStop(pCh);
#ifdef VMMC_FEAT_PCM
   VMMC_PCM_ChStop(pCh);
#endif /* VMMC_FEAT_PCM */
   VMMC_SIG_ChStop(pCh);
#ifdef DECT_SUPPORT
   VMMC_DECT_ChStop(pCh);
#endif /* DECT_SUPPORT */
}


/**
   Frees all resources of the channel.

   Called when the channel is released.

   \param  pCh          Pointer to the channel.
*/
static IFX_void_t vmmc_ChannelExit(VMMC_CHANNEL *pCh)
{
   /* delete channel mutex */
   VMMC_OS_MutexDelete (&pCh->chAcc);

   /* free the modules */
   VMMC_CON_Free_Ch_Structures (pCh);
   VMMC_ALM_Free_Ch_Structures (pCh);
   VMMC_COD_Free_Ch_Structures (pCh);
#ifdef VMMC_FEAT_PCM
   VMMC_PCM_Free_Ch_Structures (pCh);
#endif /* VMMC_FEAT_PCM */
   VMMC_SIG_Free_Ch_Structures (pCh);
#ifdef DECT_SUPPORT
   VMMC_DECT_Free_Ch_Structures (pCh);
#endif /* DECT_SUPPORT */

#if 0
   VMMC_OS_Free (pCh->pToneRes);
#endif /* 0 */
}


/**
   Firmware start function.

   \param  pLLDev       Pointer to the device structure.
   \param  pProc        Pointer to low-level device initialization structure.

   \return
   - VMMC_statusOk if successful
*/
IFX_int32_t VMMC_TAPI_LL_FW_Start(IFX_TAPI_LL_DEV_t *pLLDev,
                                  IFX_void_t const *pProc)
{
   VMMC_DEVICE *pDev = (VMMC_DEVICE*)pLLDev;
   VMMC_IO_INIT IoInit;
   IFX_int32_t ret = VMMC_statusOk;

   ret = VMMC_ChipAccessInit(pDev);
   if (ret != VMMC_statusOk)
   {
      RETURN_DEVSTATUS (ret);
   }

   /* Do not start the firmware again after it was initialised. */
   if (pDev->nDevState & DS_DEV_INIT)
   {
      return VMMC_statusOk;
   }

   /* clear interrupt register and interupt status registers */
   Vmmc_IrqLockDevice (pDev);
   vmmc_ClearPendingInt(pDev);
   Vmmc_IrqUnlockDevice(pDev);

   if (pProc == IFX_NULL)
   {
      /* reset all init pointers and init flag */
      memset (&IoInit, 0, sizeof (IoInit));
      /* set additional default flags */
      IoInit.nFlags = FW_AUTODWLD;
   }
   else
   {
      /* The init struct is specific for the LL driver and only known here.
         Because HL does not know the struct it cannot copy it and so it is
         copied here in the LL driver. */
      VMMC_OS_CpyUsr2Kern (&IoInit, pProc, sizeof (VMMC_IO_INIT));
   }

   if ((IoInit.pram_size > 0) && (IoInit.pPRAMfw != IFX_NULL))
   {
      mps_fw fwDwld;

      /* Before FW start reset the flags indicating FW error conditions. */
      pDev->bCmdReadError = IFX_FALSE;
      pDev->bSSIcrash = IFX_FALSE;
      pDev->nSlicRecoveryCnt = 0;

      /* provide the FW image details directly to the MPS driver, the
         MPS driver will copy the FW to the RAM area of Voice CPU */

      /* set download pointers */
      fwDwld.data = /*lint --e(826)*/(IFX_uint32_t* )IoInit.pPRAMfw;
      fwDwld.length = IoInit.pram_size;

     /* download firmware */
      ret =
#ifdef LINUX
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36))
         ifx_mps_ioctl((IFX_void_t *) command, IFX_NULL, FIO_MPS_DOWNLOAD,
                          (IFX_uint32_t) &fwDwld);
#else
         ifx_mps_ioctl((IFX_void_t *) command, FIO_MPS_DOWNLOAD,
                          (IFX_uint32_t) &fwDwld);
#endif /* LINUX < 2.6.36 */
#else /* LINUX */
         ifx_mps_ioctl((IFX_void_t *) command, FIO_MPS_DOWNLOAD,
                          (IFX_uint32_t) &fwDwld);
#endif /* LINUX */
   }

   if (VMMC_SUCCESS(ret))
   {
      pDev->nDevState |= DS_FW_DLD;
   }

   RETURN_DEVSTATUS (ret);
}


/**
   Firmware initialisation function.

   \param  pLLDev       Pointer to the device structure.
   \param  nMode        Enum from IFX_TAPI_INIT_MODE_t specifying the setup.

   \return
   - VMMC_statusOk if successful
*/
static IFX_int32_t VMMC_TAPI_LL_FW_Init(IFX_TAPI_LL_DEV_t *pLLDev,
                                        IFX_uint8_t nMode)
{
   VMMC_DEVICE          *pDev = (VMMC_DEVICE*)pLLDev;
   IFX_uint8_t          nMinEdspVers[3] = {0, 0, 0};
   IFX_TAPI_RESOURCE    nResource;
   IFX_int32_t          ret = VMMC_statusOk;

   /* do nothing if device initialization was already done */
   if (pDev->nDevState & DS_DEV_INIT)
   {
      return VMMC_statusOk;
   }

   ret = VMMC_ChipAccessInit(pDev);
   if (ret != VMMC_statusOk)
   {
      RETURN_DEVSTATUS (ret);
   }

   /* Read FW version and store in pDev */
   ret = vmmc_FW_VersionRead(pDev);

   if (VMMC_SUCCESS(ret))
   {
      /* Successfully reading the version implies that the FW is downloaded. */
      pDev->nDevState |= DS_FW_DLD;

      TRACE (VMMC, DBG_LEVEL_NORMAL,
             ("\nFWVERS: %u.%u.%u.%u.%u\n",
              pDev->nEdspVers[0], pDev->nEdspVers[1], pDev->nEdspVers[2],
              pDev->nEdspVers[3], pDev->nEdspVers[4]));

      switch (ifx_mps_chip_family)
      {
         case MPS_CHIP_XRX100:
            nMinEdspVers[0] = XRX100_MIN_FW_MAJORSTEP;
            nMinEdspVers[1] = XRX100_MIN_FW_MINORSTEP;
            nMinEdspVers[2] = XRX100_MIN_FW_HOTFIXSTEP;
            break;
         case MPS_CHIP_XRX200:
            nMinEdspVers[0] = XRX200_MIN_FW_MAJORSTEP;
            nMinEdspVers[1] = XRX200_MIN_FW_MINORSTEP;
            nMinEdspVers[2] = XRX200_MIN_FW_HOTFIXSTEP;
            break;
         case MPS_CHIP_XRX300:
            nMinEdspVers[0] = XRX300_MIN_FW_MAJORSTEP;
            nMinEdspVers[1] = XRX300_MIN_FW_MINORSTEP;
            nMinEdspVers[2] = XRX300_MIN_FW_HOTFIXSTEP;
            break;
         case MPS_CHIP_FALCON:
            nMinEdspVers[0] = FALCON_MIN_FW_MAJORSTEP;
            nMinEdspVers[1] = FALCON_MIN_FW_MINORSTEP;
            nMinEdspVers[2] = FALCON_MIN_FW_HOTFIXSTEP;
            break;
         default:
            TRACE (VMMC, DBG_LEVEL_HIGH,
                   ("\nVMMC WARNING: unknown platform - no version check\n"));
            break;
      }

      /* Warn if the FW version is older than a defined minimum version */
      if (   (pDev->nEdspVers[0] < nMinEdspVers[0]) ||
             ((pDev->nEdspVers[0] == nMinEdspVers[0]) &&
              ((pDev->nEdspVers[1] < nMinEdspVers[1]) ||
               ((pDev->nEdspVers[1] == nMinEdspVers[1]) &&
                ((pDev->nEdspVers[2] < nMinEdspVers[2]))))))
      {
         TRACE (VMMC, DBG_LEVEL_HIGH,
                ("\nWARNING: FW version %u.%u.%u too old.  "
                 "Minimum required FW version is %u.%u.%u\n",
                 pDev->nEdspVers[0], pDev->nEdspVers[1], pDev->nEdspVers[2],
                 nMinEdspVers[0], nMinEdspVers[1], nMinEdspVers[2]));
      }
   }

   /* Use the FW capability command to fill the capability struct in pDev */
   if (VMMC_SUCCESS(ret))
   {
      ret = VMMC_Get_FwCap(pDev);
   }

#ifdef VMMC_FEAT_SLIC
   /* Read the SDD version information and store it in the device structure. */
   if (VMMC_SUCCESS(ret))
   {
      ret = VMMC_SDD_VersionRead (pDev);

      if (VMMC_SUCCESS(ret))
      {
         IFX_uint8_t          nChannels, nFXOChannels;

         /* Reduce the number of ALM channels in the capabilities if the SLIC
            reports fewer channels. */
         if (!VMMC_ALM_SmartSLIC_ChGet (pDev, &nChannels, &nFXOChannels))
         {
            if (nChannels == 0)
            {
               TRACE (VMMC, DBG_LEVEL_HIGH,
                      ("\nWARNING: ** UNFUSED SLIC DEVICE **\n"));
               nChannels = 2; /* assume 2 channels for unfused SLIC devices */
            }
            if (pDev->caps.nALI > nChannels)
            {
               pDev->caps.nALI = nChannels;
            }
         }
         else
         {
            TRACE (VMMC, DBG_LEVEL_HIGH,
                   ("\nWARNING: ** ACCESSING SLIC DEVICE FAILED **\n"));
            pDev->caps.nALI = 0;
         }

         /* count the number of FXO lines */
         pDev->caps.nFXO = nFXOChannels;
      }
   }

   /* count the number of FXS lines */
   pDev->caps.nFXS = pDev->caps.nALI - pDev->caps.nFXO;
#else /* VMMC_FEAT_SLIC */
   pDev->caps.nALI = 0;
   pDev->caps.nFXO = 0;
   pDev->caps.nFXS = 0;
#endif /* VMMC_FEAT_SLIC */

   if (VMMC_SUCCESS(ret))
   {
      TRACE (VMMC, DBG_LEVEL_LOW, ("\n"));
      TRACE (VMMC, DBG_LEVEL_LOW,
             ("FWCAP: nPCM:%2u  nALM:%2u  nSIG:%2u  nCOD:%2u  nDECT:%2u\n",
              pDev->caps.nPCM, pDev->caps.nALI,
              pDev->caps.nSIG, pDev->caps.nCOD, pDev->caps.nDECT));
      TRACE (VMMC, DBG_LEVEL_LOW,
             ("FWCAP: nNLEC:%2u nWLEC:%2u nES:%2u   nAGC:%2u  nFAX:%2u\n",
              pDev->caps.nNLEC, pDev->caps.nWLEC, pDev->caps.nES,
              pDev->caps.nAGC, pDev->caps.nFAX));
      TRACE (VMMC, DBG_LEVEL_LOW,
             ("FWCAP: nUTG:%2u  UTG/CH:%2u nMFTD:%2u nLIN:%2u\n",
              pDev->caps.nUTG, pDev->caps.nUtgPerCh,
              pDev->caps.nMFTD, pDev->caps.nLIN));
      TRACE (VMMC, DBG_LEVEL_LOW,
             ("FWCAP: nDTMFR:%2u nDTMFG:%2u nCIDS:%2u nCIDR:%2u\n",
             pDev->caps.nDTMFD, pDev->caps.nDTMFG,
             pDev->caps.nCIDS, pDev->caps.nCIDR));

      /* Maximum number of resources, is the maximum of:
         nALI, nPCM, nCOD, nFAX, nSIG, nDECT and nLIN. */
      pDev->caps.nMaxRes = MAX(pDev->caps.nALI, pDev->caps.nCOD);
      pDev->caps.nMaxRes = MAX(pDev->caps.nMaxRes, pDev->caps.nFAX);
      pDev->caps.nMaxRes = MAX(pDev->caps.nMaxRes, pDev->caps.nSIG);
#ifdef VMMC_FEAT_PCM
      pDev->caps.nMaxRes = MAX(pDev->caps.nMaxRes, pDev->caps.nPCM);
#endif /* VMMC_FEAT_PCM */
#ifdef DECT_SUPPORT
      pDev->caps.nMaxRes = MAX(pDev->caps.nMaxRes, pDev->caps.nDECT);
#endif /* DECT_SUPPORT */

      if (pDev->caps.nMaxRes > VMMC_MAX_CH_NR)
      {
         /* Warn about incorrect definition then limit the counters so that we
            do not crash when trying to initialise non existing resources. */
         TRACE(VMMC,DBG_LEVEL_HIGH,
               ("WARNING: Detected maximum resources %d but defined only %d. "
                "Some resources will not be usable\n",
                pDev->caps.nMaxRes, VMMC_MAX_CH_NR));
         if (pDev->caps.nALI > VMMC_MAX_CH_NR)
            pDev->caps.nALI = VMMC_MAX_CH_NR;
         if (pDev->caps.nCOD > VMMC_MAX_CH_NR)
            pDev->caps.nCOD = VMMC_MAX_CH_NR;
         if (pDev->caps.nFAX > VMMC_MAX_CH_NR)
            pDev->caps.nFAX = VMMC_MAX_CH_NR;
         if (pDev->caps.nSIG > VMMC_MAX_CH_NR)
            pDev->caps.nSIG = VMMC_MAX_CH_NR;
         if (pDev->caps.nPCM > VMMC_MAX_CH_NR)
            pDev->caps.nPCM = VMMC_MAX_CH_NR;
         if (pDev->caps.nDECT > VMMC_MAX_CH_NR)
            pDev->caps.nDECT = VMMC_MAX_CH_NR;
         if (pDev->caps.nLIN > VMMC_MAX_CH_NR)
            pDev->caps.nLIN = VMMC_MAX_CH_NR;
         if (pDev->caps.nAudioCnt > VMMC_MAX_CH_NR)
            pDev->caps.nAudioCnt = VMMC_MAX_CH_NR;
         pDev->caps.nMaxRes = VMMC_MAX_CH_NR;
      }

      /* report the resource counts to HL TAPI */
      memset (&nResource, 0x00, sizeof(nResource));
      nResource.AlmCount   = pDev->caps.nALI;
      nResource.SigCount   = pDev->caps.nSIG;
      nResource.CodCount   = pDev->caps.nCOD;
      nResource.PcmCount   = pDev->caps.nPCM;
      nResource.DectCount  = pDev->caps.nDECT;
      nResource.DTMFGCount = pDev->caps.nDTMFG;
      nResource.DTMFRCount = pDev->caps.nDTMFD;
      nResource.FSKGCount  = pDev->caps.nCIDS;
      nResource.FSKRCount  = pDev->caps.nCIDR;
      nResource.ToneCount  = MAX(pDev->caps.nSIG, pDev->caps.nDECT);
      nResource.HdlcCount  = pDev->caps.nHDLC;
      IFX_TAPI_ReportResources (pDev->pTapiDev,&nResource);

      /* use event mailbox per default - if _not_
         Danube/Twinpass FW < 10.x or INCA-IP2 FW < 9.x  */
      if ( !( ((pDev->nEdspVers[3] == 1) && (pDev->nEdspVers[0] < 10)) ||
              ((pDev->nEdspVers[3] == 0) && (pDev->nEdspVers[0] < 9))
            ))
      {
         pDev->caps.bEventMailboxSupported = 1;
      }
      else
      {
         pDev->caps.bEventMailboxSupported = 0;
      }
   }

   if (VMMC_SUCCESS(ret))
   {
      ret = VMMC_AddCaps(pDev);
   }

   /* set default firmware cache values now, required to allow storage of
      coefficients from BBD Download... */
   if (VMMC_SUCCESS(ret))
   {
      ret = vmmc_FW_DevDataInit(pDev);
   }

   pDev->nDevState |= DS_DEV_INIT;

   /* Do appropriate initialization for all channels and the device. */
   if (nMode != IFX_TAPI_INIT_MODE_NONE)
   {
      IFX_uint16_t nCh;

      for (nCh = 0; VMMC_SUCCESS(ret) && (nCh < VMMC_MAX_CH_NR); nCh++)
      {
         ret = vmmc_Basic_VoIPConf (&pDev->pChannel[nCh], nMode);
      }

#ifdef VMMC_FEAT_CLOCK_SCALING
      if (VMMC_SUCCESS(ret))
      {
         ret = VMMC_PMC_baseConf(pDev);
      }
#endif /* VMMC_FEAT_CLOCK_SCALING */
   }

   RETURN_DEVSTATUS (ret);
}


/**
   Read the firmware version.

   \param  pDev         Pointer to the device structure.

   \return
   IFX_SUCCESS or IFX_ERROR, if read failed or FW not downloaded

   \remarks
   Result is stored in nEdspVersion in pDev structure. Assigment is:
     nEdspVers[0] : Major FW version number
     nEdspVers[1] : Minor FW version number
     nEdspVers[2] : Hotfix number FW version
     nEdspVers[3] : Platform id (0=INCA IP2, 1=Danube, ...)
     nEdspVers[4] : Customer variant (0=for all customers)
*/
IFX_int32_t vmmc_FW_VersionRead (VMMC_DEVICE *pDev)
{
   SYS_VER_t   pCmd;
   IFX_int32_t ret;

   memset((IFX_void_t *)&pCmd, 0, sizeof(SYS_VER_t));
   /* Command Header */
   pCmd.CMD = CMD_EOP;
   pCmd.MOD = MOD_SYSTEM;
   pCmd.ECMD = SYS_VER_ECMD;

   /* Read Cmd */
   ret = CmdRead(pDev, (IFX_uint32_t *)&pCmd,
                       (IFX_uint32_t *)&pCmd, SYS_VER_LEN_BASIC);
   if (VMMC_SUCCESS(ret))
   {
      pDev->nEdspVers[0] = pCmd.MAJ;
      pDev->nEdspVers[1] = pCmd.MIN;
      pDev->nEdspVers[2] = pCmd.HF;
      pDev->nEdspVers[3] = pCmd.PLA;
      pDev->nEdspVers[4] = pCmd.VAR;
   }

   RETURN_DEVSTATUS (ret);
}


/**
   Create and initalise the structs handling the firmware modules.

   \param  pDev         Pointer to the device structure.

   \return
   IFX_SUCCESS or IFX_ERROR, if no memory is available
*/
IFX_int32_t  vmmc_FW_DevDataInit(VMMC_DEVICE *pDev)
{
   IFX_uint8_t i;
   IFX_int32_t ret = IFX_SUCCESS;

   /* RES module */
   ret = VMMC_RES_StructuresAllocate (pDev);
   if (ret != IFX_SUCCESS)
   {
      SET_DEV_ERROR (VMMC_ERR_NO_MEM);
   }
   else
   {
      VMMC_RES_StructuresInit (pDev);
   }

   /* CON module */
   for (i=0; (ret == IFX_SUCCESS) && (i < pDev->caps.nMaxRes); ++i)
   {
      VMMC_CHANNEL *pCh = &pDev->pChannel[i];

      ret = VMMC_CON_Allocate_Ch_Structures (pCh);
      if (ret != IFX_SUCCESS)
      {
         SET_DEV_ERROR (VMMC_ERR_NO_MEM);
      }
   }

   /* ALM module */
   for (i = 0; (ret == IFX_SUCCESS) && (i < pDev->caps.nALI); ++i)
   {
      VMMC_CHANNEL *pCh = &pDev->pChannel[i];

      ret = VMMC_ALM_Allocate_Ch_Structures (pCh);
      if (ret != IFX_SUCCESS)
      {
         SET_DEV_ERROR (VMMC_ERR_NO_MEM);
      }
      else
      {
         VMMC_ALM_InitCh (pCh);
      }
   }

   /* COD module */
   for (i = 0; (ret == IFX_SUCCESS) && (i < pDev->caps.nCOD); ++i)
   {
      VMMC_CHANNEL *pCh = &pDev->pChannel[i];

      ret = VMMC_COD_Allocate_Ch_Structures (pCh);
      if (ret != IFX_SUCCESS)
      {
         SET_DEV_ERROR (VMMC_ERR_NO_MEM);
      }
      else
      {
         ret = VMMC_COD_InitCh (pCh);
      }
   }

#ifdef VMMC_FEAT_PCM
   /* PCM module */
   for (i = 0; (ret == IFX_SUCCESS) && (i < pDev->caps.nPCM); ++i)
   {
      VMMC_CHANNEL *pCh = &pDev->pChannel[i];

      ret = VMMC_PCM_Allocate_Ch_Structures (pCh);
      if (ret != IFX_SUCCESS)
      {
         SET_DEV_ERROR (VMMC_ERR_NO_MEM);
      }
      else
      {
         /* set pcm ch equal to channel (linear pcm resource mapping) */
         VMMC_PCM_InitCh (pCh, i);
      }
   }
#endif /* VMMC_FEAT_PCM */

   /* SIG module */
   for (i = 0; (ret == IFX_SUCCESS) && (i < pDev->caps.nSIG); ++i)
   {
      VMMC_CHANNEL *pCh = &pDev->pChannel[i];

      ret = VMMC_SIG_Allocate_Ch_Structures (pCh);
      if (ret != IFX_SUCCESS)
      {
         SET_DEV_ERROR (VMMC_ERR_NO_MEM);
      }
      else
      {
         VMMC_SIG_InitCh (pCh);
      }
   }

#ifdef DECT_SUPPORT
   /* DECT module */
   for (i = 0; (ret == IFX_SUCCESS) && (i < pDev->caps.nDECT); ++i)
   {
      VMMC_CHANNEL *pCh = &pDev->pChannel[i];

      ret = VMMC_DECT_Allocate_Ch_Structures (pCh);
      if (ret != IFX_SUCCESS)
      {
         SET_DEV_ERROR (VMMC_ERR_NO_MEM);
      }
      else
      {
         VMMC_DECT_InitCh (pCh);
      }
   }
#endif /* DECT_SUPPORT */

#ifdef VMMC_FEAT_ANNOUNCEMENTS
   if ((ret == IFX_SUCCESS) && (pDev->caps.nCOD > 0))
   {
      /* Initialize the announcement service */
      ret = VMMC_Ann_Init (pDev);
   }
#endif /* VMMC_FEAT_ANNOUNCEMENTS */

   for (i = 0; (ret == IFX_SUCCESS) && (i < pDev->caps.nALI); ++i)
   {
      VMMC_CHANNEL *pCh = &pDev->pChannel[i];

      /* default MWI configuration */
      ret = VMMC_ALM_MWL_DefaultConf (pCh);
   }

   return ret;
}


/**
   Configure FW according to Tapi settings

   \param  pCh          Pointer to the channel structure.
   \param  nMode        Operating mode determines the preconfigured connections.

   \return IFX_SUCCESS or IFX_ERROR
*/
static IFX_int32_t vmmc_Basic_VoIPConf (VMMC_CHANNEL *pCh, IFX_uint8_t nMode)
{
   VMMC_DEVICE  *pDev = pCh->pParent;
   IFX_int32_t  ret = IFX_SUCCESS;

   /* protect fwmsgs against concurrent tasks */
   VMMC_OS_MutexGet (&pDev->mtxMemberAcc);

   /* create connections between the modules depending on the selected mode */
   switch (nMode)
   {
      default:
      case IFX_TAPI_INIT_MODE_DEFAULT:
      case IFX_TAPI_INIT_MODE_VOICE_CODER:
         /* connect COD to SIG to create the "data channel" */
         if ((pCh->pCOD != IFX_NULL) && (pCh->pSIG != IFX_NULL))
         {
            /* attach COD on the network side of SIG */
            ret = VMMC_CON_ConnectPrepare (pCh, VMMCDSP_MT_SIG,
                                           pCh, VMMCDSP_MT_COD, REMOTE_SIG_OUT);
            if (!VMMC_SUCCESS(ret))
               goto exit;
            ret = VMMC_CON_ConnectPrepare (pCh, VMMCDSP_MT_COD,
                                           pCh, VMMCDSP_MT_SIG, REMOTE_SIG_OUT);
            if (!VMMC_SUCCESS(ret))
               goto exit;
         }
#ifdef ENABLE_OBSOLETE_PREMAPPING
#ifdef VMMC_ALM_EN
         if ((pCh->pSIG != IFX_NULL) && (pCh->pALM != IFX_NULL))
         {
            /* connect SIG to ALM */
            ret = VMMC_CON_ConnectPrepare (pCh, VMMCDSP_MT_SIG,
                                           pCh, VMMCDSP_MT_ALM, LOCAL_SIG_OUT);
            if(!VMMC_SUCCESS(ret))
               goto exit;
            ret = VMMC_CON_ConnectPrepare (pCh, VMMCDSP_MT_ALM,
                                           pCh, VMMCDSP_MT_SIG, LOCAL_SIG_OUT);
            if(!VMMC_SUCCESS(ret))
               goto exit;
         }
#endif /* VMMC_ALM_EN */
#endif /* ENABLE_OBSOLETE_PREMAPPING */
         break;

      case IFX_TAPI_INIT_MODE_PCM_DSP:
         /* connect PCM <-> SIG <-> ALM */
         if ((pCh->pPCM != IFX_NULL) && (pCh->pSIG != IFX_NULL))
         {
            /* attach PCM on the network side of SIG */
            ret = VMMC_CON_ConnectPrepare (pCh, VMMCDSP_MT_SIG,
                                           pCh, VMMCDSP_MT_PCM, REMOTE_SIG_OUT);
            if (!VMMC_SUCCESS(ret))
               goto exit;
            ret = VMMC_CON_ConnectPrepare (pCh, VMMCDSP_MT_PCM,
                                           pCh, VMMCDSP_MT_SIG, REMOTE_SIG_OUT);
            if (!VMMC_SUCCESS(ret))
               goto exit;
         }
#ifdef ENABLE_OBSOLETE_PREMAPPING
#ifdef VMMC_ALM_EN
         if ((pCh->pSIG != IFX_NULL) && (pCh->pALM != IFX_NULL))
         {
            /* connect SIG to ALM */
            ret = VMMC_CON_ConnectPrepare (pCh, VMMCDSP_MT_SIG,
                                           pCh, VMMCDSP_MT_ALM, LOCAL_SIG_OUT);
            if(!VMMC_SUCCESS(ret))
               goto exit;
            ret = VMMC_CON_ConnectPrepare (pCh, VMMCDSP_MT_ALM,
                                           pCh, VMMCDSP_MT_SIG, LOCAL_SIG_OUT);
            if(!VMMC_SUCCESS(ret))
               goto exit;
         }
#endif /* VMMC_ALM_EN */
#endif /* ENABLE_OBSOLETE_PREMAPPING */
         break;

      case IFX_TAPI_INIT_MODE_PCM_PHONE:
         /* connect PCM <-> ALM */
         if ((pCh->pPCM != IFX_NULL) && (pCh->pALM != IFX_NULL))
         {
            ret = VMMC_CON_ConnectPrepare (pCh, VMMCDSP_MT_PCM,
                                           pCh, VMMCDSP_MT_ALM, LOCAL_SIG_OUT);
            if(!VMMC_SUCCESS(ret))
               goto exit;
            ret = VMMC_CON_ConnectPrepare (pCh, VMMCDSP_MT_ALM,
                                           pCh, VMMCDSP_MT_PCM, LOCAL_SIG_OUT);
            if(!VMMC_SUCCESS(ret))
               goto exit;
         }
         break;

      case IFX_TAPI_INIT_MODE_TEST_COD2ALM:
         /* internal testmode: COD <-> ALM */
#ifdef VMMC_ALM_EN
         if ((pCh->pCOD != IFX_NULL) && (pCh->pALM != IFX_NULL))
         {
            /* connect COD to ALM */
            ret = VMMC_CON_ConnectPrepare (pCh, VMMCDSP_MT_COD,
                                           pCh, VMMCDSP_MT_ALM, LOCAL_SIG_OUT);
            if(!VMMC_SUCCESS(ret))
               goto exit;
            ret = VMMC_CON_ConnectPrepare (pCh, VMMCDSP_MT_ALM,
                                           pCh, VMMCDSP_MT_COD, LOCAL_SIG_OUT);
            if(!VMMC_SUCCESS(ret))
               goto exit;
         }
#endif /* VMMC_ALM_EN */
         break;

      case IFX_TAPI_INIT_MODE_NONE:
         /* internal testmode: no connection between the modules */
         break;
   }

#ifdef VMMC_ALM_EN
   /* configure ALI for VoIP */
   if (pCh->pALM != IFX_NULL)
   {
      ret = VMMC_ALM_baseConf (pCh);
      pDev->nDevState |= DS_ALM_EN;
   }
#endif /* VMMC_ALM_EN */

   /* configure Coder  */
   if (ret == IFX_SUCCESS && pCh->pCOD != IFX_NULL)
   {
      ret = VMMC_COD_baseConf (pCh);
      pDev->nDevState |= DS_COD_EN;
   }

   /* Configure Signalling module */
   if (ret == IFX_SUCCESS && pCh->pSIG != IFX_NULL)
   {
      ret = VMMC_SIG_Base_Conf (pCh);
      pDev->nDevState |= DS_SIG_EN;
   }

exit:
   /* unlock */
   VMMC_OS_MutexRelease (&pDev->mtxMemberAcc);

   if(VMMC_SUCCESS(ret))
      ret = VMMC_CON_ConnectConfigure (pDev);

   if(VMMC_SUCCESS(ret))
      pCh->nState |= CS_INIT;

   return ret;
}


/**
   VMMC device driver startup.

   This is the device driver initialization function to call at the system
   startup prior to any access to the VMMC device driver.

   \return IFX_SUCCESS or error code
*/
IFX_int32_t VMMC_DeviceDriverStart(void)
{
   IFX_int32_t result = 0;
   IFX_TAPI_DRV_CTX_t *pDrvCtx = &DrvCtx;

#ifdef ENABLE_TRACE
   /* Sanity check for cases where the variable is set from the outside. */
   if ((debug_level < DBG_LEVEL_LOW) || (debug_level > DBG_LEVEL_OFF))
   {
      debug_level = DBG_LEVEL_HIGH;
   }
   /* Set the default trace level */
   SetTraceLevel(VMMC, debug_level);
#endif /* ENABLE_TRACE */

#ifdef VMMC_WITH_MPS
   /* load MPS driver */
   ifx_mps_init_module();
#endif

   memset(pDrvCtx, 0, sizeof(*pDrvCtx));

   /* Initialize the function pointers structure and register
      with the High Level TAPI */
   pDrvCtx->majorNumber = major;
   pDrvCtx->minorBase = minorBase;

   pDrvCtx->devNodeName = devName;
   pDrvCtx->maxDevs = VMMC_MAX_DEVICES;
   pDrvCtx->maxChannels = VMMC_MAX_CH_NR;

   /* procfs info */
   pDrvCtx->drvName = DEV_NAME;
   pDrvCtx->drvVersion = DRV_VMMC_VER_STR;
   pDrvCtx->hlLLInterfaceVersion = DRV_LL_INTERFACE_VER_STR;

   /* Generic functions  */
   pDrvCtx->Prepare_Dev = VMMC_TAPI_LL_DevicePrepare;
   pDrvCtx->Init_Dev = VMMC_TAPI_LL_DeviceInit;
   pDrvCtx->Exit_Dev = VMMC_TAPI_LL_DeviceExit;

   pDrvCtx->Prepare_Ch = VMMC_TAPI_LL_ChannelPrepare;
   pDrvCtx->Init_Ch = VMMC_TAPI_LL_ChannelInit;

   pDrvCtx->FW_Start = VMMC_TAPI_LL_FW_Start;
   pDrvCtx->FW_Init = VMMC_TAPI_LL_FW_Init;
   pDrvCtx->BBD_Dnld = VMMC_TAPI_LL_BBD_Dnld;

   pDrvCtx->Open = VMMC_LL_Open;
   pDrvCtx->Release = VMMC_LL_Close;

#ifdef VMMC_FEAT_PACKET
   pDrvCtx->Write = VMMC_LL_Write;
   pDrvCtx->pktBufPrependSpace = 0;
   pDrvCtx->bProvidePktRead = IFX_TRUE;
   pDrvCtx->readChannels[IFX_TAPI_STREAM_COD] = VMMC_MAX_CH_NR;
#endif /* VMMC_FEAT_PACKET */

   pDrvCtx->Ioctl = VMMC_Dev_Spec_Ioctl;

   pDrvCtx->CAP_Number_Get = TAPI_LL_Phone_Get_Capabilities;
   pDrvCtx->CAP_List_Get = TAPI_LL_Phone_Get_Capability_List;
   pDrvCtx->CAP_Check = TAPI_LL_Phone_Check_Capability;

   /* IRQ information */
   pDrvCtx->IRQ.LockDevice = Vmmc_IrqLockDevice;
   pDrvCtx->IRQ.UnlockDevice = Vmmc_IrqUnlockDevice;
   pDrvCtx->IRQ.IrqEnable = Vmmc_IrqEnable;
   pDrvCtx->IRQ.IrqDisable = Vmmc_IrqDisable;

   /* Debug related functions */
   pDrvCtx->Dbg_CErr_Handler = VMMC_CmdErr_Handler;

   /* CODer Module */
   VMMC_COD_Func_Register (&pDrvCtx->COD);

   /* CONnection module */
   VMMC_CON_Func_Register (&pDrvCtx->CON);

#ifdef VMMC_FEAT_PCM
   /* PCM related */
   VMMC_PCM_Func_Register (&pDrvCtx->PCM);
#endif /* VMMC_FEAT_PCM */

   /* SIGnalling related */
   VMMC_SIG_Func_Register (&pDrvCtx->SIG);

   /* ALM specific */
   VMMC_ALM_Func_Register (&pDrvCtx->ALM);

#ifdef DECT_SUPPORT
   /* DECT specific */
   VMMC_DECT_Func_Register (&pDrvCtx->DECT);
#endif /* DECT_SUPPORT */

   /* Register this driver with the HL-TAPI driver
      this also registers the driver context for use by the HL-TAPI */
   result = IFX_TAPI_Register_LL_Drv (pDrvCtx);
   if (result != IFX_SUCCESS)
   {
      TRACE(VMMC, DBG_LEVEL_HIGH, ("VMMC driver start: registration failed\n"));
      return result;
   }

#ifdef LINUX
   /* copyright trace shall not be prefixed with KERN_INFO */
   printk("%s, (c) 2006-2016 Lantiq Beteiligungs-GmbH & Co.KG\n",
          &DRV_VMMC_WHATVERSION [4]);
#else
   printf("%s, (c) 2006-2016 Lantiq Beteiligungs-GmbH & Co.KG\n\r",
          &DRV_VMMC_WHATVERSION [4]);
#endif /* LINUX */

#ifdef EVENT_LOGGER_DEBUG
   {
      IFX_int32_t i = 0;

      /* Register the driver with device name, type and number
         to the eventlogger */
      for (i = 0; i < pDrvCtx->maxDevs; i++)
      {
         EL_REG_Register(DEV_NAME, DEV_TYPE_VOICE_MACRO,
                         i /* dev num */, IFX_NULL /* cb func */);
      }
   }
#endif /* EVENT_LOGGER_DEBUG */

#ifdef VMMC_FEAT_CLOCK_SCALING
   /* Start thread for system clock frequency change. */
   VMMC_PMC_OnDriverStart();
#endif /* VMMC_FEAT_CLOCK_SCALING */

   return IFX_SUCCESS;
}


/**
   VMMC device driver shutdown.
*/
IFX_void_t VMMC_DeviceDriverStop(void)
{
   IFX_uint8_t    i;

#ifdef VMMC_FEAT_CLOCK_SCALING
   /* Stop thread for system clock frequency change. */
   VMMC_PMC_OnDriverStop();
#endif /* VMMC_FEAT_CLOCK_SCALING */

   /* Unregister this driver from the HL-TAPI driver
      this also unregisters the device driver context */
   IFX_TAPI_Unregister_LL_Drv ((&DrvCtx)->majorNumber);

   for (i=0; i < VMMC_MAX_DEVICES; i++)
   {
      /* resource cleanup */
      vmmc_DeviceExit(&VDevices[i], IFX_TRUE);

      /* Free the capabilities list */
      if (VDevices[i].CapList != IFX_NULL)
      {
         VMMC_OS_Free (VDevices[i].CapList);
         VDevices[i].CapList = IFX_NULL;
      }
   }

#if !defined(SYSTEM_FALCON)
   if (VDevices[0].nDevState & DS_GPIO_RESERVED)
   {
      IFX_int32_t ret = 0;
      ret = VMMC_XRX100_PcmGpioRelease();
      if (!VMMC_SUCCESS(ret))
      {
          TRACE(VMMC, DBG_LEVEL_HIGH,
                ("ERROR: Unregistering the GPIOs of PCM interface failed.\n"));
      }
   }
#endif /* !defined(SYSTEM_FALCON) */

   /* Note: the device struct array is a static variable so no free is needed.*/

#ifdef EVENT_LOGGER_DEBUG
   /* Unregister the driver with device name, type and number
      from the eventlogger */
   EL_REG_Unregister(DEV_NAME, DEV_TYPE_VOICE_MACRO, -1 /* dev num (-1 all) */);
#endif /* EVENT_LOGGER_DEBUG */

#ifdef VMMC_WITH_MPS
   /* unload MPS driver */
   ifx_mps_cleanup_module();
#endif
}
