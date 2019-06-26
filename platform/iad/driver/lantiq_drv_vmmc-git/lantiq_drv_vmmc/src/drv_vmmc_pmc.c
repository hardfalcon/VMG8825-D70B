/******************************************************************************

  Copyright (c) 2009-2015 Lantiq Deutschland GmbH
  Copyright (c) 2015 Lantiq Beteiligungs-GmbH & Co.KG
  Copyright 2016, Intel Corporation.

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/**
   \file drv_vmmc_pmc.c
   Implements the interface towards the system power management control unit.
*/

/* ============================= */
/* Includes                      */
/* ============================= */
#include "drv_api.h"
#include "drv_vmmc_pmc.h"
#include "drv_vmmc_fw_commands_voip.h"
#include "drv_vmmc_fw_commands_sdd.h"
#include "drv_vmmc_access.h"                    /* HOST_PROTECT... macros */
#include "drv_vmmc_init.h"                      /* VMMC_GetDevice() */
#include "drv_vmmc_alm.h"

#ifdef VMMC_FEAT_PMCU_IF
   #include <ifx_pmcu.h>                        /* PMCU driver interface */
#endif /* VMMC_FEAT_PMCU_IF */
#ifdef VMMC_FEAT_CPUFREQ_IF
   #include <linux/cpufreq.h>
   #include <cpufreq/ltq_cpufreq.h>
   #include <cpufreq/ltq_cpufreq_pmcu_compatible.h>
#endif /* VMMC_FEAT_CPUFREQ_IF */


/* ============================= */
/* Local Macros & Definitions    */
/* ============================= */

/* ============================= */
/* Type definitions              */
/* ============================= */

/* ============================= */
/* Function declaration          */
/* ============================= */
#ifdef VMMC_FEAT_PMCU_IF
static
IFX_PMCU_RETURN_t irq_vmmc_pmc_state_change(
                        IFX_PMCU_STATE_t pmcuState);
#endif /* VMMC_FEAT_PMCU_IF */

#ifdef VMMC_FEAT_CPUFREQ_IF
static int vmmc_cpufreq_notifier(
                        struct notifier_block *nb,
                        unsigned long val,
                        void *data);
#endif /* VMMC_FEAT_CPUFREQ_IF */

#ifdef VMMC_FEATURE_PMC_CORE
static
IFX_PMCU_RETURN_t irq_vmmc_pmc_state_get(
                        IFX_PMCU_STATE_t *pmcuState);

static
IFX_PMCU_RETURN_t irq_vmmc_pmc_prechange(
                        IFX_PMCU_MODULE_t pmcuModule,
                        IFX_PMCU_STATE_t newState,
                        IFX_PMCU_STATE_t oldState);

static
IFX_PMCU_RETURN_t irq_vmmc_pmc_postchange(
                        IFX_PMCU_MODULE_t pmcuModule,
                        IFX_PMCU_STATE_t newState,
                        IFX_PMCU_STATE_t oldState);

static IFX_boolean_t irq_vmmc_pmc_IsDspActive(
                        void);

static IFX_boolean_t irq_vmmc_pmc_IsSsiActive(
                        void);

static IFX_int32_t vmmc_pmc_RequestClockState(
                        VMMC_DEVICE *pDev,
                        IFX_PMCU_STATE_t clockState);
#endif /* VMMC_FEATURE_PMC_CORE */

#ifdef VMMC_FEAT_SLIC
static IFX_int32_t vmmc_pmc_SysClockHighThread(
                        VMMC_OS_ThreadParams_t *pThread);

static IFX_int32_t vmmc_pmc_SendClockHighAck(
                        VMMC_DEVICE *pDev);
#endif /* VMMC_FEAT_SLIC */


/* ============================= */
/* Global variables              */
/* ============================= */

/* Indicates if this driver is registered with the PMCU. */
static IFX_boolean_t      bRegisteredWithPmcu;
#ifdef VMMC_FEATURE_PMC_CORE
/* Status of the system clock. */
volatile IFX_PMCU_STATE_t nClockState,
                          nRequestedClockState;
#endif /* VMMC_FEATURE_PMC_CORE */
/* Semaphore to defer write until the system clock frequency is adjusted. */
static VMMC_OS_mutex_t    mtxDeferWrite;
#ifdef VMMC_FEAT_SLIC
/* mutex semaphore blocking the real time clock high thread */
static VMMC_OS_mutex_t s_mtxSysClkHighTask;
/* clock high thread handling structure */
static VMMC_OS_ThreadCtrl_t s_SysClkHighThread;
#endif /* VMMC_FEAT_SLIC */

#ifdef VMMC_FEAT_CPUFREQ_IF
/* Linux CPUFREQ support start */

struct ltq_cpufreq_module_info vmmc_cpufreq_feature_fss = {
   .name                            = "VOICE frequency scaling support",
   .pmcuModule                      = LTQ_CPUFREQ_MODULE_VE,
   .pmcuModuleNr                    = 0,
   .powerFeatureStat                = 1,
   .ltq_cpufreq_state_get           = irq_vmmc_pmc_state_get,
   .ltq_cpufreq_pwr_feature_switch  = NULL,
};

/* keep track of frequency transitions */
static int vmmc_cpufreq_notifier(
                        struct notifier_block *nb,
                        unsigned long val,
                        void *data)
{
   struct cpufreq_freqs *freq = data;
   IFX_PMCU_STATE_t new_State, old_State;
   IFX_PMCU_RETURN_t ret;

   new_State = ltq_cpufreq_get_ps_from_khz(freq->new);
   if(new_State == LTQ_CPUFREQ_PS_UNDEF)
   {
      return NOTIFY_STOP_MASK | (LTQ_CPUFREQ_MODULE_VE<<4);
   }
   old_State = ltq_cpufreq_get_ps_from_khz(freq->old);
   if(old_State == LTQ_CPUFREQ_PS_UNDEF)
   {
      return NOTIFY_STOP_MASK | (LTQ_CPUFREQ_MODULE_VE<<4);
   }
   if (val == CPUFREQ_PRECHANGE)
   {
      ret = irq_vmmc_pmc_prechange(LTQ_CPUFREQ_MODULE_VE, new_State, old_State);
      if (ret != IFX_PMCU_RETURN_SUCCESS)
      {
         return NOTIFY_STOP_MASK | (LTQ_CPUFREQ_MODULE_VE<<4);
      }
   }
   else if (val == CPUFREQ_POSTCHANGE)
   {
      ret = irq_vmmc_pmc_postchange(LTQ_CPUFREQ_MODULE_VE, new_State, old_State);
      if (ret != IFX_PMCU_RETURN_SUCCESS)
      {
         return NOTIFY_STOP_MASK | (LTQ_CPUFREQ_MODULE_VE<<4);
      }
   }
   else
   {
      return NOTIFY_OK | (LTQ_CPUFREQ_MODULE_VE<<4);
   }
   return NOTIFY_OK | (LTQ_CPUFREQ_MODULE_VE<<4);
}


static struct notifier_block vmmc_cpufreq_notifier_block =
{
        .notifier_call  = vmmc_cpufreq_notifier
};
/* Linux CPUFREQ support end */
#endif /* VMMC_FEAT_CPUFREQ_IF */


/* ============================= */
/* Function definitions          */
/* ============================= */

/**
   Initialise Power management control.

   Create data structure in the device and set state variables to initial
   values.

   \param  pDev         Pointer to VMMC device structure.

   \return IFX_SUCCESS or error code
*/
IFX_int32_t VMMC_PMC_Init(VMMC_DEVICE *pDev)
{
   /* Power management control data array */
   pDev->pPMC = VMMC_OS_Malloc (sizeof (VMMC_PMC_CHANNEL_t) * VMMC_MAX_CH_NR);
   if (pDev->pPMC == IFX_NULL)
   {
      return IFX_ERROR;
   }
   memset (pDev->pPMC, 0, sizeof (VMMC_PMC_CHANNEL_t) * VMMC_MAX_CH_NR);

   /* Initially no request is pending. */
   pDev->bNeedSysClkHigh = IFX_FALSE;

   return IFX_SUCCESS;
}


/**
   Close down Power management control.

   Free data struct in the device.

   \param  pDev         Pointer to VMMC device structure.

   \return IFX_SUCCESS or error code
*/
IFX_void_t VMMC_PMC_Exit(VMMC_DEVICE *pDev)
{
   /* free Power Management Control data array */
   if (pDev->pPMC != IFX_NULL)
   {
      VMMC_OS_Free (pDev->pPMC);
      pDev->pPMC = IFX_NULL;
   }
}


/**
   PMC base configuration.

   Use this function where needed to set the base configuration of
   the Power Management Control interface.

   \param  pDev         Pointer to VMMC device structure.

   \return
   IFX_SUCCESS or error code
*/
IFX_int32_t VMMC_PMC_baseConf (VMMC_DEVICE *pDev)
{
   IFX_int32_t       ret = VMMC_statusOk;

#ifdef VMMC_FEAT_SLIC
   /* Write the Clock High Acknowledge message to start the handshake mechanism
      that allows the DART to sleep and wake up again. */
   ret = vmmc_pmc_SendClockHighAck(pDev);
#endif /* VMMC_FEAT_SLIC */

   return ret;
}


/**
   Write command to mailbox and send power management reports.

   This function records the status of all relevant algorithms used in the FW.
   It determines if the FW is idle or busy and reports this to PMCU. The PMCU
   can then implement power mangement on the device.

   \param  pDev         Pointer to VMMC device structure.
   \param  pMsg         Pointer to MPS struct with data to write.

   \return
   - IFX_SUCCESS on success.
   - IFX_ERROR on error. Error can occur if command-inbox space is not
                         sufficient or if low level function/macros fail.
   -VMMC_ERR_NO_FIBXMS  Not enough inbox space for writing command.

   \remarks
   The function will always and only write into the command mailbox. It must be
   made sure that only commands are passed to this function.
*/
IFX_int32_t VMMC_PMC_Write(VMMC_DEVICE *pDev, mps_message *pMsg)
{
   /* We only look at the common header of the FW command and the enable field.
      These parts are identical in all commands we are interested in. So we can
      use any of these FW cmd message structures to decode these fields. */
   PCM_CHAN_t *pFwCmd = /*lint --e(826)*/ (PCM_CHAN_t *)pMsg->pData;
   IFX_int32_t ret;
   IFX_boolean_t bCanRollback = IFX_FALSE;
   IFX_boolean_t bFwActiveBackup = IFX_FALSE;   /* init with dummy value */
   union VMMC_PMC_CHANNEL nBackup;
   IFX_uint8_t i;
   IFX_boolean_t bActivate = IFX_FALSE;
   IFX_boolean_t bDoPostprocessing = IFX_FALSE;
   /* Allowed number of functions that may still run in idle mode. */
   IFX_uint8_t nPcmChCnt = 1,
               nSigChCnt = 1,
               nSigMftdCnt = 1;

#ifdef VMMC_FEAT_SLIC
   /* This block handles only SDD messages. Other messages are handled below.
      Only when the FW supports the handshake to lower the clock SDD messages
      need to be looked at.
      Analyse only write messages and ignore read messages.
      Be paranoid and make sure that the channel field is in range.
      Do nothing before the device is initialised. */
   if ((pFwCmd->CMD == CMD_SDD) && (pFwCmd->MOD == MOD_SDD) &&
       (pDev->caps.bCLKHS) && (pFwCmd->RW == CMDWRITE) &&
       (pFwCmd->CHAN < MAX_ALM_NUM) && (pDev->nDevState & DS_DEV_INIT))
   {
      /* Any SDD Opmode changes as well as FXO hook-state changes require
         the system clock to be at full speed first. */
      if ((pFwCmd->ECMD == SDD_Opmode_ECMD) ||
          (pFwCmd->ECMD == SDD_FxoHookSwitch_ECMD))
      {
         /* When writing this command the DART will become active. */
         pDev->bDartActive = IFX_TRUE;
#ifdef VMMC_FEATURE_PMC_CORE
         /* Request maximum system clock speed. */
         ret = vmmc_pmc_RequestClockState(pDev, IFX_PMCU_STATE_D0);
         if (ret != VMMC_statusOk)
            return ret;
#endif /* VMMC_FEATURE_PMC_CORE */
      }
   }
   else
#endif /* VMMC_FEAT_SLIC */
#ifdef VMMC_FEATURE_PMC_CORE
   /* This block handles only the Clock High Acknowledge message.
      Other Messages are handled below. This block needs to stay above the
      checking of remaining EOP messages. */
   if ((pFwCmd->CMD == CMD_EOP) && (pFwCmd->MOD == MOD_SYSTEM) &&
       (pFwCmd->ECMD == SYS_CLOCK_HIGH_ECMD) && (pFwCmd->LENGTH == 0))
   {
      /* Make sure that the clock runs at full speed before writing this msg. */
      ret = vmmc_pmc_RequestClockState(pDev, IFX_PMCU_STATE_D0);
      if (ret != VMMC_statusOk)
         return ret;
   }
   else
#endif /* VMMC_FEATURE_PMC_CORE */
   /* Analyse only write messages and ignore read messages.
      Only messages with payload may contain an enable flag.
      Be paranoid and make sure that the channel field is in range.
      Do not access PMC structs before the device is initialised. */
   if ((pFwCmd->RW == CMDWRITE) && (pFwCmd->LENGTH > 0) &&
       (pFwCmd->CHAN < VMMC_MAX_CH_NR) && (pDev->nDevState & DS_DEV_INIT))
   {
      /* 0) Make a backup of the struct we are about to change so that in the
            case of an error we can do a rollback. */
      nBackup = pDev->pPMC[pFwCmd->CHAN];
      bFwActiveBackup = pDev->bFwActive;
      bCanRollback = IFX_TRUE;

      /* 1) Store the EN flag from selected FW command messages in an internal
            structure of the device. */
      switch (pFwCmd->CMD)
      {
      case CMD_EOP:
         /* Decode the relevant EOP commands */
         switch (pFwCmd->MOD)
         {
         case MOD_PCM:
            switch (pFwCmd->ECMD)
            {
            case PCM_CHAN_ECMD:
               pDev->pPMC[pFwCmd->CHAN].bits.pcm_ch = pFwCmd->EN;
               break;
            case PCM_LEC_ECMD:
               pDev->pPMC[pFwCmd->CHAN].bits.pcm_lec = pFwCmd->EN;
               break;
            case PCM_ES_ECMD:
               pDev->pPMC[pFwCmd->CHAN].bits.pcm_es = pFwCmd->EN;
               break;
            case PCM_DCHAN_ECMD:
               pDev->pPMC[pFwCmd->CHAN].bits.hdlc_ch = pFwCmd->EN;
               break;
            case PCM_SCHAN_ECMD:
               pDev->pPMC[pFwCmd->CHAN].bits.pcm_lb = pFwCmd->EN;
               break;
            default:
               /* struct was not changed so no need to rollback */
               bCanRollback = IFX_FALSE;
               break;
            }
            break;

         case MOD_ALI:
            switch (pFwCmd->ECMD)
            {
            case ALI_LEC_ECMD:
               pDev->pPMC[pFwCmd->CHAN].bits.alm_lec = pFwCmd->EN;
               break;
            case ALI_ES_ECMD:
               pDev->pPMC[pFwCmd->CHAN].bits.alm_es = pFwCmd->EN;
               break;
            default:
               /* struct was not changed so no need to rollback */
               bCanRollback = IFX_FALSE;
               break;
            }
            break;

         case MOD_SIGNALING:
            switch (pFwCmd->ECMD)
            {
            case SIG_CHAN_ECMD:
               pDev->pPMC[pFwCmd->CHAN].bits.sig_ch = pFwCmd->EN;
               break;
            case SIG_CIDS_CTRL_ECMD:
               pDev->pPMC[pFwCmd->CHAN].bits.sig_fskg = pFwCmd->EN;
               break;
            case SIG_CIDR_CTRL_ECMD:
               pDev->pPMC[pFwCmd->CHAN].bits.sig_fskd = pFwCmd->EN;
               break;
            case SIG_DTMFATG_CTRL_ECMD:
               pDev->pPMC[pFwCmd->CHAN].bits.sig_dtmfg = pFwCmd->EN;
               break;
            case SIG_DTMFR_CTRL_ECMD:
               pDev->pPMC[pFwCmd->CHAN].bits.sig_dtmfd = pFwCmd->EN;
               break;
            case SIG_UTG_CTRL_ECMD:
               pDev->pPMC[pFwCmd->CHAN].bits.sig_utg1 = pFwCmd->EN;
               break;
            case SIG_UTG2_CTRL_ECMD:
               pDev->pPMC[pFwCmd->CHAN].bits.sig_utg2 = pFwCmd->EN;
               break;
            case SIG_MFTD_CTRL_ECMD:
               pDev->pPMC[pFwCmd->CHAN].bits.sig_mftd = pFwCmd->EN;
               break;
            case SIG_CPTD_CTRL_ECMD:
               {
                  SIG_CPTD_CTRL_t *pCptdCtrl = (SIG_CPTD_CTRL_t *)pMsg->pData;
                  IFX_uint16_t j;

                  /* Multiple CPTDs are possible. Check if any of these is
                     enabled. If so then set the flag and abort. Disabled
                     means that not any CPTD is enabled. */
                  for (j=0, pDev->pPMC[pFwCmd->CHAN].bits.sig_cptd = 0;
                       pCptdCtrl->LENGTH >= ((j + 1) * SIG_CPTD_CTRL_LEN); j++)
                  {
                     if (pCptdCtrl->CPTD[j].EN)
                     {
                        pDev->pPMC[pFwCmd->CHAN].bits.sig_cptd = 1;
                        break;
                     }
                  } /* for j */
               }
               break;
            default:
               /* struct was not changed so no need to rollback */
               bCanRollback = IFX_FALSE;
               break;
            }
            break;

         case MOD_CODER:
            switch (pFwCmd->ECMD)
            {
            case COD_CHAN_SPEECH_ECMD:
               pDev->pPMC[pFwCmd->CHAN].bits.cod_ch = pFwCmd->EN;
               break;
            case COD_AGC_CTRL_ECMD:
               pDev->pPMC[pFwCmd->CHAN].bits.cod_agc = pFwCmd->EN;
               break;
            case COD_FAX_CTRL_ECMD:
               pDev->pPMC[pFwCmd->CHAN].bits.cod_fdp = pFwCmd->EN;
               break;
            default:
               /* struct was not changed so no need to rollback */
               bCanRollback = IFX_FALSE;
               break;
            }
            break;

         /* Ignore all the other modules. */
         default:
            /* struct was not changed so no need to rollback */
            bCanRollback = IFX_FALSE;
            break;
         }
         break;

      case CMD_DECT:
         /* Decode the relevant DECT commands */
         switch (pFwCmd->ECMD)
         {
         case COD_CHAN_SPEECH_ECMD:
            pDev->pPMC[pFwCmd->CHAN].bits.dect_ch = pFwCmd->EN;
            break;
         case DECT_UTG_CTRL_ECMD:
            pDev->pPMC[pFwCmd->CHAN].bits.dect_utg = pFwCmd->EN;
            break;
         default:
            /* struct was not changed so no need to rollback */
            bCanRollback = IFX_FALSE;
            break;
         }
         break;

      default:
         /* struct was not changed so no need to rollback */
         bCanRollback = IFX_FALSE;
         break;
      }

      /* 2) Check if the FW is busy or idle. Any set bit in the internal struct
            of the device indicates that the FW is busy. To allow the reception
            of CID sequences on FXO lines that are signaled by alert tones
            instead of line signals the bits for these functions are masked.
            This is possible because at low frequency still few algorithms may
            run. In this case the FW reports idle and requests low frequency. */
      for (i=0; i < VMMC_MAX_CH_NR ; i++)
      {
         union VMMC_PMC_CHANNEL nChState;

         /* Make a copy on which bits can be masked out. */
         nChState = pDev->pPMC[i];

         /* Mask funtions needed for the FXO scenario. Only a limited number
            of functions are masked. Exceeding the limit for a function of
            one type on different channels will trigger the busy indication
            below. */
         if ((nChState.bits.pcm_ch != 0) && (nPcmChCnt > 0))
         {
            nChState.bits.pcm_ch = 0;
            nPcmChCnt--;
         }
         if ((nChState.bits.sig_ch != 0) && (nSigChCnt > 0))
         {
            nChState.bits.sig_ch = 0;
            nSigChCnt--;
         }
         if ((nChState.bits.sig_mftd != 0) && (nSigMftdCnt > 0))
         {
            nChState.bits.sig_mftd = 0;
            nSigMftdCnt--;
         }

         /* By definition the FW is busy if any bits are set here. */
         if (nChState.value != 0)
         {
            bActivate = IFX_TRUE;
            break;
         }
      }

      /* 3) If there is a transistion from idle to active and the clock is
            low send a request that full performance is needed. */
      if ((bActivate == IFX_TRUE) && (pDev->bFwActive == IFX_FALSE))
      {
         TRACE(VMMC, DBG_LEVEL_LOW,
               ("VMMC dev %d: activate DSP\n", pDev->nDevNr));
         /* Mark the FW as active */
         pDev->bFwActive = IFX_TRUE;
#ifdef VMMC_FEATURE_PMC_CORE
         /* Request maximum system clock speed. */
         ret = vmmc_pmc_RequestClockState(pDev, IFX_PMCU_STATE_D0);
         if (ret != VMMC_statusOk)
            return ret;
#endif /* VMMC_FEATURE_PMC_CORE */
      }
      else
      {
         /* Remember to check after the write if anything needs to be done. */
         bDoPostprocessing = IFX_TRUE;
      }
   }

   /* 4) Send command to FW. */
   /* concurrent access protect by driver, but it should move to here.
       To cease interrupt and to use semaphore here is a good idea.*/
   VMMC_HOST_PROTECT(pDev);
   ret = ifx_mps_write_mailbox(command, pMsg);
   VMMC_HOST_RELEASE(pDev);

   /* 5) If the write failed we do a rollback of the status struct. */
   if ((ret != IFX_SUCCESS) && (bCanRollback == IFX_TRUE))
   {
      /*lint -e{644} bCanRollback makes sure that nBackup is initialised */
      pDev->pPMC[pFwCmd->CHAN] = nBackup;
      /* bFwActive is derived from the flags that we just restored. In order
         to not do the complete evaluation here just restore the status that
         was frozen together with the flags above. */
      pDev->bFwActive = bFwActiveBackup;
   }

   /* 6) If there is a transistion from active to idle note down the status
         so that we report the correct status when asked the next time. */
   if ((bDoPostprocessing == IFX_TRUE) && (ret == IFX_SUCCESS) &&
       (pDev->bFwActive == IFX_TRUE) && (bActivate == IFX_FALSE))
   {
      TRACE(VMMC, DBG_LEVEL_LOW,
            ("VMMC dev %d: deactivate DSP\n", pDev->nDevNr));

      /* Now FW is idle  */
      pDev->bFwActive = IFX_FALSE;

#ifdef VMMC_FEAT_CPUFREQ_IF
      /* If also the DART is idle notify that all frequencies are ok now. */
      if (pDev->bDartActive == IFX_FALSE)
      {
         /* Announce that any clock state is ok now. */
         (void) ltq_cpufreq_state_req(LTQ_CPUFREQ_MODULE_VE,
                                      1, IFX_PMCU_STATE_D0D3);
      }
#endif /* VMMC_FEAT_CPUFREQ_IF */
   }

   return ret;
}


/**
   Report clock frequency requirement for FW boot to the PMCU.

   \param  pDev         Pointer to VMMC device structure.
   \param  bBoot        Indicates if FW boots or has finished booting:
                        - IFX_TRUE: FW boots
                        - IFX_FALSE: FW boot has finished.

   \return
   - IFX_SUCCESS on success.
   - errors from PMCU when requesting clock speed
*/
IFX_int32_t VMMC_PMC_FwBoot(VMMC_DEVICE *pDev, IFX_boolean_t bBoot)
{
   IFX_int32_t ret = IFX_SUCCESS;

   if (bBoot == IFX_TRUE)
   {
      /* FW is booting. */
      pDev->bFwStartup = IFX_TRUE;
      /* Initially the DSP is idle but the DART is active. */
      pDev->bFwActive = IFX_FALSE;
      pDev->bDartActive = IFX_TRUE;

      /* The flags above are checked in the prechange call and need to be set
         before requesting the clock state below. */

#ifdef VMMC_FEATURE_PMC_CORE
      /* Request maximum system clock speed. */
      ret = vmmc_pmc_RequestClockState(pDev, IFX_PMCU_STATE_D0);
#endif /* VMMC_FEATURE_PMC_CORE */

      TRACE(VMMC, DBG_LEVEL_LOW,
            ("VMMC dev %d: firmware booting\n", pDev->nDevNr));
   }
   else
   {
      /* Reset the flag that indicates that the FW is booting. */
      pDev->bFwStartup = IFX_FALSE;
      /* Update of the flags for DSP and DART is done by the other procedures
         in this module. */
      TRACE(VMMC, DBG_LEVEL_LOW,
            ("VMMC dev %d: firmware boot finished\n", pDev->nDevNr));
   }

   return ret;
}


#ifdef VMMC_FEAT_PMCU_IF
/**
   Callback used to change module's power state.

   For voice the power state cannot be changed with this interface.

   \param  pmcuState    Structure with power management state.

   \return
   - IFX_PMCU_RETURN_SUCCESS  successful in all cases.
*/
static IFX_PMCU_RETURN_t irq_vmmc_pmc_state_change(IFX_PMCU_STATE_t pmcuState)
{
   /* For voice the power state cannot be changed with this interface. */
   /* The power state is always accepted as given. */

   return IFX_PMCU_RETURN_SUCCESS;
}
#endif /* VMMC_FEAT_PMCU_IF */


#ifdef VMMC_FEATURE_PMC_CORE
/**
   Returns the status that was set by the PMCU.

   \param  pmcuState    Pointer to return power state.

   \return
   - IFX_PMCU_RETURN_SUCCESS Get power-state successfully.
*/
static IFX_PMCU_RETURN_t irq_vmmc_pmc_state_get(IFX_PMCU_STATE_t *pmcuState)
{
   *pmcuState = nClockState;

   return IFX_PMCU_RETURN_SUCCESS;
}


/**
   Callback to be called before module changes its state to new.

   \param  pmcuModule   Module
   \param  newState     New state
   \param  oldState     Old state

   \return
   - IFX_SUCCESS on success.
   - IFX_ERROR on error.
*/
static
IFX_PMCU_RETURN_t irq_vmmc_pmc_prechange(
                        IFX_PMCU_MODULE_t pmcuModule,
                        IFX_PMCU_STATE_t newState,
                        IFX_PMCU_STATE_t oldState)
{
   /* If the DSP part of the FW is active D0 is the only allowed state. */
   if ((newState != IFX_PMCU_STATE_D0) &&
       (irq_vmmc_pmc_IsDspActive() == IFX_TRUE))
   {
#if 0
      /* Never ever use TRACE in IRQ context (except for debugging). */
      TRACE(VMMC, DBG_LEVEL_HIGH,
      ("VMMC: Deny lower clock - FW is active (module:%d, newState:%d,"
       " oldState:%d)\n", pmcuModule, newState, oldState));
#endif
      return IFX_PMCU_RETURN_DENIED;
   }

   /* If the SSI interface is active only D0 and D1 state are allowed. */
   if ((newState != IFX_PMCU_STATE_D0) &&
       (newState != IFX_PMCU_STATE_D1) &&
       (irq_vmmc_pmc_IsSsiActive() == IFX_TRUE))
   {
#if 0
      /* Never ever use TRACE in IRQ context (except for debugging). */
      TRACE(VMMC, DBG_LEVEL_HIGH,
      ("VMMC: Deny lower clock - SSI is active (module:%d, newState:%d,"
       " oldState:%d)\n", pmcuModule, newState, oldState));
#endif
      return IFX_PMCU_RETURN_DENIED;
   }

   return IFX_PMCU_RETURN_SUCCESS;
}


/**
   Callback to be called after module changes its state to new state.

   \param   pmcuModule      Module
   \param   newState        New state
   \param   oldState        Old state

   \return
   - IFX_SUCCESS on success.
   - IFX_ERROR on error.
*/
static
IFX_PMCU_RETURN_t irq_vmmc_pmc_postchange(
                        IFX_PMCU_MODULE_t pmcuModule,
                        IFX_PMCU_STATE_t newState,
                        IFX_PMCU_STATE_t oldState)
{
   VMMC_UNUSED(newState);
   VMMC_UNUSED(oldState);

   /* Remember the clock state for checking it in the request function. */
   nClockState = newState;

   if (newState == nRequestedClockState)
   {
      /* Request is done. Reset the variable for the next request. */
      nRequestedClockState = IFX_PMCU_STATE_INVALID;
      /* Unblock the waiting task now that clock speed has changed. */
      VMMC_OS_MutexRelease(&mtxDeferWrite);
   }

#if 0
   /* Never ever use TRACE in IRQ context (except for debugging). */
   TRACE(VMMC, DBG_LEVEL_LOW,
         ("VMMC clock speed D%d\n", newState-1));
#endif

   return IFX_PMCU_RETURN_SUCCESS;
}


/**
   Check if the DSP part of the voice-FW is active.

   \return
   - IFX_TRUE if FW is active.
   - IFX_FALSE if FW is not active.
*/
static IFX_boolean_t irq_vmmc_pmc_IsDspActive(void)
{
   IFX_uint16_t     dev;
   VMMC_DEVICE      *pDev = IFX_NULL;

   for (dev = 0; dev < VMMC_MAX_DEVICES; dev++)
   {
      if (VMMC_GetDevice (dev, &pDev) == IFX_SUCCESS)
      {
         /* The flags are initialised in ...DevicePrepare() and can be used
            even before the device was started. */
         if ((pDev->bFwActive == IFX_TRUE) ||
             (pDev->bFwStartup == IFX_TRUE))
         {
            /* FW is active. There is no need to check other devices. */
            return IFX_TRUE;
         }
      }
   }
   return IFX_FALSE;
}


/**
   Check if the SSI interface of the voice-FW is active.

   \return
   - IFX_TRUE if FW is active.
   - IFX_FALSE if FW is not active or FW does not support clock scaling.
*/
static IFX_boolean_t irq_vmmc_pmc_IsSsiActive(void)
{
   IFX_uint16_t     dev;
   VMMC_DEVICE      *pDev = IFX_NULL;

   for (dev = 0; dev < VMMC_MAX_DEVICES; dev++)
   {
      if (VMMC_GetDevice (dev, &pDev) == IFX_SUCCESS)
      {
         /* The flag is initialised in ...DevicePrepare() and can be used
            even before the device was started. */
         if (pDev->bDartActive == IFX_TRUE)
         {
            /* SSI is active. There is no need to check other devices */
            return IFX_TRUE;
         }
      }
   }

   return IFX_FALSE;
}
#endif /* VMMC_FEATURE_PMC_CORE */


#ifdef VMMC_FEAT_SLIC
/**
   Notifies about the DART_IN_SLEEP event.

   This function is used directly from interrupt context to notify about
   the DART_IN_SLEEP event from the FW.

   \param  pDev         Pointer to VMMC device structure.
*/
IFX_void_t irq_VMMC_PMC_DartInSleepEvent (VMMC_DEVICE *pDev)
{
   IFX_uint16_t nCh;

   VMMC_ASSERT (pDev != IFX_NULL);

   /* Any ALM reporting that the DART cannot sleep blocks the reset of the
      flag below. Only when all channels were tested negative the flag is
      reset. */

   for (nCh = 0; nCh < MAX_ALM_NUM; nCh++)
   {
      if (! irq_VMMC_ALM_DartCanSleep(&pDev->pChannel[nCh]))
         return;
   }

   pDev->bDartActive = IFX_FALSE;

#ifdef VMMC_FEAT_CPUFREQ_IF
   /* If also the DSP is idle notify that all frequencies are ok now. */
   if (pDev->bFwActive == IFX_FALSE)
   {
      /* Announce that any clock state is ok now. */
      (void) ltq_cpufreq_state_req(LTQ_CPUFREQ_MODULE_VE,
                                   1, IFX_PMCU_STATE_D0D3);
   }
#endif /* VMMC_FEAT_CPUFREQ_IF */
}
#endif /* VMMC_FEAT_SLIC */


#ifdef VMMC_FEAT_SLIC
/**
   Handle the DART wakeup request of the FW.

   On receiving this event the system clock frequency needs to be increased
   and when this is stable the SYS_CLOCK_HIGH command needs to be sent.
   As both of this includes possible delays this function schedules this
   into task context.

   \param  pDev         Pointer to VMMC device structure.
*/
IFX_void_t irq_VMMC_PMC_DartWakeupReqEvent(VMMC_DEVICE *pDev)
{

   VMMC_ASSERT (pDev != IFX_NULL);

   if (pDev->bNeedSysClkHigh == IFX_FALSE)
   {
      /* set flag in pDev context */
      pDev->bNeedSysClkHigh = IFX_TRUE;
      /* wakeup the handler thread */
      VMMC_OS_MutexRelease(&s_mtxSysClkHighTask);
   }
}
#endif /* VMMC_FEAT_SLIC */


#ifdef VMMC_FEATURE_PMC_CORE
/**
   Request a clock frequency state.

   In case that the system clock frequency needs to be changed this function
   is blocking until the change is reported back by the PMCU.

   \param  pDev         Pointer to VMMC device structure.
   \param  clockState   Clock status to request.

   \return
   - VMMC_statusOk      Sending request successfull.
   - VMMC_statusErr     Sending request failed.
*/
static IFX_int32_t vmmc_pmc_RequestClockState(
                        VMMC_DEVICE *pDev,
                        IFX_PMCU_STATE_t clockState)
{
   IFX_int32_t ret = VMMC_statusOk;

   /* If the requested clock state is the one that was reported last nothing
      needs to be done here. Checking the last clock state and storing the
      requested state needs to be done in an atomic operation as the requested
      clock state acts also as a flag to signal that here we start blocking
      after the request and wait for a wake-up from the post function. */
   Vmmc_IrqLockDevice(pDev);

#ifdef VMMC_FEAT_CPUFREQ_IF
   if (nClockState == IFX_PMCU_STATE_INVALID)
   {
      struct cpufreq_policy *policy = cpufreq_cpu_get(smp_processor_id());
      nClockState = IFX_PMCU_STATE_D0;
      if (policy)
      {
         nClockState = ltq_cpufreq_get_ps_from_khz(policy->cur);
      }
   }
#endif /* VMMC_FEAT_CPUFREQ_IF */

   if (nClockState != clockState)
   {
      nRequestedClockState = clockState;
      Vmmc_IrqUnlockDevice(pDev);

      {
         IFX_PMCU_RETURN_t err = IFX_PMCU_RETURN_ERROR;

#ifdef VMMC_FEAT_PMCU_IF
         /* Request clock state via the PMCU */
         err = ifx_pmcu_state_req(IFX_PMCU_MODULE_VE, 1, clockState);

         if (err == IFX_PMCU_RETURN_SUCCESS)
         {
            /* Wait on semaphore until the clock speed was increased. */
            VMMC_OS_MutexGet(&mtxDeferWrite);
         }
         else
#if defined(PMCU_VERSION_CODE)
         /* Denied is returned when accepting of requests is turned off.
            It implies that the clock runs at maximum speed. */
         if (err != IFX_PMCU_RETURN_DENIED)
#endif /* PMCU_VERSION_CODE */
         {
            TRACE(VMMC, DBG_LEVEL_HIGH,
                 ("VMMC dev %d: PMCU failed to request clock state %d\n",
                  pDev->nDevNr, clockState));
            /*errmsg: Failed to send sys clock change request to PMCU. */
            ret = VMMC_statusReqMaxClockFailed;
         }
#endif /* VMMC_FEAT_PMCU_IF */
#ifdef VMMC_FEAT_CPUFREQ_IF
         /* Request clock state */
         err = ltq_cpufreq_state_req(LTQ_CPUFREQ_MODULE_VE, 1, clockState);
         if (err == IFX_PMCU_RETURN_SUCCESS)
         {
            /* Wait on semaphore until the clock speed was increased. */
            VMMC_OS_MutexGet(&mtxDeferWrite);
         }
         else
         /* Denied is returned when accepting of requests is turned off.
            It implies that the clock runs at maximum speed. */
         if (err == IFX_PMCU_RETURN_DENIED)
         {
            nClockState = clockState;
            nRequestedClockState = IFX_PMCU_STATE_INVALID;
         }
         else
         {
            TRACE(VMMC, DBG_LEVEL_HIGH,
                 ("VMMC dev %d: PMCU failed to request clock state %d\n",
                  pDev->nDevNr, clockState));
            /*errmsg: Failed to send sys clock change request to PMCU. */
            ret = VMMC_statusReqMaxClockFailed;
         }
#endif /* VMMC_FEAT_CPUFREQ_IF */
      }
   }
   else
   {
      /* Release protection also in the case that nothing needs to be done. */
      Vmmc_IrqUnlockDevice(pDev);
   }

   return ret;
}
#endif /* VMMC_FEATURE_PMC_CORE */


#ifdef VMMC_FEAT_SLIC
/**
   Thead to send Clock High Acknowledge to FW.

   This thread is triggered from IRQ context via semaphore to write the clock
   high acknowledge message to the FW. Because this may sleep this thread is
   needed.

   \param  pThread      Reference to the thread context.

   \return
   - 0                  In all cases.
*/
static IFX_int32_t vmmc_pmc_SysClockHighThread(VMMC_OS_ThreadParams_t *pThread)
{
   IFX_int32_t mtx_ret, ret;

   /* Acquire realtime priority. */
   VMMC_OS_THREAD_PRIORITY_MODIFY (VMMC_OS_THREAD_PRIO_HIGH);

   /* The main loop is waiting for an event. */
   while ((pThread->bShutDown == IFX_FALSE) &&
          (mtx_ret = VMMC_OS_MutexLockInterruptible(&s_mtxSysClkHighTask),
           (mtx_ret == 0)) &&
          (pThread->bShutDown == IFX_FALSE))
   {
      VMMC_DEVICE *pDev;
      IFX_int32_t i;

      /* Loop over all devices. */
      for (i=0; i < VMMC_MAX_DEVICES; i++)
      {
         if ((VMMC_GetDevice(i, &pDev) == VMMC_statusOk) &&
             (pDev->bNeedSysClkHigh))
         {
            /* When writing this command the DART will be active. */
            pDev->bDartActive = IFX_TRUE;
            /* Unset flag in pDev context. */
            pDev->bNeedSysClkHigh = IFX_FALSE;
            /* Send the Clock High Acknowledge message. This will internally
               request the maximum system clock before writing the command. */
            ret = vmmc_pmc_SendClockHighAck(pDev);
            if (!VMMC_SUCCESS(ret))
            {
               IFX_TAPI_EVENT_t tapiEvent;

               /* This specific event is being reported on channel 0. */
               memset(&tapiEvent, 0, sizeof(IFX_TAPI_EVENT_t));
               tapiEvent.id = IFX_TAPI_EVENT_FAULT_SW_WAKE_DART;
               IFX_TAPI_Event_Dispatch(pDev->pChannel[0].pTapiCh, &tapiEvent);
            }
         }
      } /* for all devices */
   } /* main loop */

   return 0;
}
#endif /* VMMC_FEAT_SLIC */


#ifdef VMMC_FEAT_SLIC
/**
   Send a Clock High Acknowledge command to the FW.

   The writing of this message may trigger a request to increase the system
   clock frequency and waiting for completion of this. This is handled in the
   write function above.

   \param  pDev         Pointer to VMMC device structure.

   \return
   Value from CmdWrite.
*/
static IFX_int32_t vmmc_pmc_SendClockHighAck(VMMC_DEVICE *pDev)
{
   struct SYS_CLOCK_HIGH  fw_sys_clock_high;

   VMMC_ASSERT(pDev != IFX_NULL);

   /* This command must not be sent when the FW is unaware of clock scaling. */
   if (!(pDev->caps.bCLKHS))
      return VMMC_statusOk;
   /* When no analog lines are available do not send this command. */
   if (pDev->caps.nFXS == 0 && pDev->caps.nFXO == 0)
      return VMMC_statusOk;

   memset (&fw_sys_clock_high, 0, sizeof (fw_sys_clock_high));
   fw_sys_clock_high.CMD   = CMD_EOP;
   fw_sys_clock_high.MOD   = MOD_SYSTEM;
   fw_sys_clock_high.ECMD  = SYS_CLOCK_HIGH_ECMD;

   return CmdWrite(pDev,
                   (IFX_uint32_t *)&fw_sys_clock_high, SYS_CLOCK_HIGH_LEN);
}
#endif /* VMMC_FEAT_SLIC */


/**
   Report the current clock frequency status.

   \param  pClockState  Pointer to variable where to return the clock state.
   \param  pRequestedClockState Pointer to variable where to return the
                        requested clock state.
*/
IFX_void_t VMMC_PMC_ClockStateGet(IFX_int32_t *pClockState,
                                  IFX_int32_t *pRequestedClockState)
{
#ifdef VMMC_FEATURE_PMC_CORE
   *pClockState =
      (IFX_int32_t)(nClockState - IFX_PMCU_STATE_D0);
   *pRequestedClockState =
      (IFX_int32_t)(nRequestedClockState - IFX_PMCU_STATE_D0);
#else
   *pClockState = *pRequestedClockState = -1;
#endif /* VMMC_FEATURE_PMC_CORE */
}


/**
   Initialisation of Power management control to be done on driver start.

   This creates a thread for sending the SysClockHigh message to the FW.
   Register with the PMCU driver.

   \return     VMMC_statusOk
*/
IFX_int32_t VMMC_PMC_OnDriverStart(void)
{
#ifdef VMMC_FEATURE_PMC_CORE
   /* Initially the system clock state is unknown. */
   nClockState = nRequestedClockState = IFX_PMCU_STATE_INVALID;
#endif /* VMMC_FEATURE_PMC_CORE */
   /* Initialise the command write defer semaphore. (locked) */
   VMMC_OS_MutexInit(&mtxDeferWrite);
   VMMC_OS_MutexGet(&mtxDeferWrite);

#ifdef VMMC_FEAT_SLIC
   /* Initialize mutex the sys-clock-high thread waits on. (locked) */
   VMMC_OS_MutexInit(&s_mtxSysClkHighTask);
   VMMC_OS_MutexGet(&s_mtxSysClkHighTask);
   /* Start a thread to send the SysClockHigh message in task context. */
   VMMC_OS_ThreadInit (&s_SysClkHighThread, "VMMCsysclk",
            (VMMC_OS_ThreadFunction_t)vmmc_pmc_SysClockHighThread,
            /* stack size */ 5000,
            /* nPriority */ VMMC_OS_THREAD_PRIO_HIGH,
            0, 0);
#endif /* VMMC_FEAT_SLIC */

#ifdef VMMC_FEAT_PMCU_IF
   {
      IFX_PMCU_REGISTER_t pmcuRegister;
      /* Dependency list. */
      /* must be static to let the gcc accept the static initialisation */
      static IFX_PMCU_MODULE_DEP_t depList =
      {
         /* nDepth */
         1,
         {
            {  /* module id */
               IFX_PMCU_MODULE_CPU,
               /* Device number */
               0,
               /* onState */
               IFX_PMCU_STATE_D0,
               /* standBy */
               IFX_PMCU_STATE_D0D3,
               /* lpStandBy */
               IFX_PMCU_STATE_D0D3,
               /* offState */
               IFX_PMCU_STATE_D0D3
            }
         }
      };

      /* Register with the PMCU driver */
      memset (&pmcuRegister, 0, sizeof(IFX_PMCU_REGISTER_t));
      pmcuRegister.pmcuModule = IFX_PMCU_MODULE_VE;
      /* The device number is stored here for use in the get function.
         Note: Number 0 is reserved for TAPI HL driver. VMMC LL-driver
         uses Number 1. */
      pmcuRegister.pmcuModuleNr = 1;
      pmcuRegister.pmcuModuleDep = &depList;
      pmcuRegister.ifx_pmcu_state_get = irq_vmmc_pmc_state_get;
      pmcuRegister.ifx_pmcu_state_change = irq_vmmc_pmc_state_change;
      pmcuRegister.pre = irq_vmmc_pmc_prechange;
      pmcuRegister.post = irq_vmmc_pmc_postchange;

      if (IFX_PMCU_RETURN_SUCCESS != ifx_pmcu_register ( &pmcuRegister ))
      {
         TRACE(VMMC, DBG_LEVEL_HIGH,
              ("ERROR, VMMC Registration to PMCU failed.\n"));
         return IFX_ERROR;
      }

      TRACE(VMMC, DBG_LEVEL_LOW,
           ("VMMC: successfully registered to the PMCU\n"));
      /* Set flag that this device is registered with the PMCU. */
      bRegisteredWithPmcu = IFX_TRUE;
   }
#endif /* VMMC_FEAT_PMCU_IF */

#ifdef VMMC_FEAT_CPUFREQ_IF
   if ( cpufreq_register_notifier(&vmmc_cpufreq_notifier_block,
                                  CPUFREQ_TRANSITION_NOTIFIER) )
   {
      TRACE(VMMC, DBG_LEVEL_HIGH,
           ("ERROR, VMMC Registration to PMCU failed.\n"));
      return IFX_ERROR;
   }
   else
   {
      struct ltq_cpufreq* ppd_cpufreq_p;
      ppd_cpufreq_p = ltq_cpufreq_get();
      if(ppd_cpufreq_p != NULL){
         list_add_tail(&vmmc_cpufreq_feature_fss.list,
                       &ppd_cpufreq_p->list_head_module);
      }

      TRACE(VMMC, DBG_LEVEL_LOW,
           ("VMMC: successfully registered to the PMCU\n"));
      /* Set flag that this device is registered with the PMCU. */
      bRegisteredWithPmcu = IFX_TRUE;
   }
#endif /* VMMC_FEAT_CPUFREQ_IF */

   return VMMC_statusOk;
}


/**
   Termination of Power management control to be done on driver stop.

   Terminates the thread for sending the SysClockHigh message to the FW.
   Unregister from the PMCU driver.

   \return     VMMC_statusOk
*/
IFX_int32_t VMMC_PMC_OnDriverStop(void)
{
   if (bRegisteredWithPmcu)
   {
#ifdef VMMC_FEAT_PMCU_IF
      IFX_PMCU_REGISTER_t pmcuRegister;

      memset (&pmcuRegister, 0, sizeof(pmcuRegister));
      pmcuRegister.pmcuModule = IFX_PMCU_MODULE_VE;
      pmcuRegister.pmcuModuleNr = 1;
      ifx_pmcu_unregister ( &pmcuRegister );
#endif /* VMMC_FEAT_PMCU_IF */

#ifdef VMMC_FEAT_CPUFREQ_IF
      if ( cpufreq_unregister_notifier(&vmmc_cpufreq_notifier_block,
                                       CPUFREQ_TRANSITION_NOTIFIER) )
      {
         TRACE(VMMC, DBG_LEVEL_HIGH,
              ("ERROR, VMMC: Fail in unregistering VMMC from CPUFREQ.\n"));
      }
      list_del(&vmmc_cpufreq_feature_fss.list);
#endif /* VMMC_FEAT_CPUFREQ_IF */

      /* Unconditionally reset the flag that this driver is registered. */
      bRegisteredWithPmcu = IFX_FALSE;
   }

#ifdef VMMC_FEAT_SLIC
   VMMC_OS_THREAD_KILL (&s_SysClkHighThread);
   VMMC_OS_MutexDelete(&s_mtxSysClkHighTask);
#endif /* VMMC_FEAT_SLIC */

   /* delete the cmd write defer semaphore */
   VMMC_OS_MutexRelease(&mtxDeferWrite);
   VMMC_OS_MutexDelete(&mtxDeferWrite);

   return VMMC_statusOk;
}
