/******************************************************************************

                            Copyright (c) 2011-2016
                        Lantiq Beteiligungs-GmbH & Co.KG
                             http://www.lantiq.com

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/**
   \file drv_tapi_ppd.c
   Implementation of the FXS Phone Detection state machine.
*/
#include "drv_tapi.h"
#include "drv_tapi_errno.h"

#ifdef TAPI_FEAT_PMCU_IF
#include <ifx_pmcu.h>          /* PMCU driver interface */
#endif /* TAPI_FEAT_PMCU_IF */
#ifdef TAPI_FEAT_CPUFREQ_IF
   #include <linux/cpufreq.h>
   #include <cpufreq/ltq_cpufreq.h>
   #include <cpufreq/ltq_cpufreq_pmcu_compatible.h>
#endif /* TAPI_FEAT_CPUFREQ_IF */

#ifdef TAPI_FEAT_PHONE_DETECTION

/* ================================= */
/* Defines                           */
/* ================================= */
/* Indexes in Table with timeouts for timers. */
typedef enum
{
   TAPI_PPD_T1_IDX = 0,
   TAPI_PPD_T2_IDX = 1,
   TAPI_PPD_T3_IDX = 2,
   TAPI_PPD_T4_IDX = 3,
   TAPI_PPD_MAX_TIMEOUTS = 4
} TAPI_PPD_TIMEOUT_IDX_t;

/* States of state machine. */
typedef enum
{
   /* Line is disabled. */
   TAPI_PPD_STATE_LINE_DISABLED            = 0x0001,
   /* Phone is connected. Line is set to STANDBY. */
   TAPI_PPD_STATE_LINE_STANDBY             = 0x0002,
   /* Off-Hook detection procedure is ongoing. */
   TAPI_PPD_STATE_OFF_HOOK_DETECTION       = 0x0003,
   /* On-Hook detection procedure is ongoing. */
   TAPI_PPD_STATE_ON_HOOK_DETECTION        = 0x0004,
   /* Suspended because the neighbouring channel is in use. */
   TAPI_PPD_STATE_SUSPENDED                = 0x0005,
   /* Used to mark end of state tables */
   TAPI_PPD_STATE_END                      = 0xFFFF
} TAPI_PPD_SM_STATES_t;

/* Internal events */
typedef enum
{
   TAPI_PPD_IE_NONE = 0,
   /* Event generated when T1 timeout. */
   TAPI_PPD_IE_T1,
   /* T2 timeout. */
   TAPI_PPD_IE_T2,
   /* T3 timeout. */
   TAPI_PPD_IE_T3,
   /* T4 timeout. */
   TAPI_PPD_IE_T4,
   /* API change Line Feeding. */
   TAPI_PPD_IE_API_CHANGE_FEEDING,
   /* End of capacitance measurement. */
   TAPI_PPD_IE_LINE_MEASURE_CAPACITANCE_RDY,
   /* Stop the FXS Phone Detection state machine. */
   TAPI_PPD_IE_STOP,
   /* Services like ringing or CID transmission are started. */
   TAPI_PPD_IE_SERVICE,
   /* off-hook detected. */
   TAPI_PPD_IE_HOOKOFF,
   /* API started the capacitance measurement. */
   TAPI_PPD_IE_API_CAP_MEAS,
   /* Neighbour channel went active. */
   TAPI_PPD_IE_NB_ACTIVE,
   /* Neighbour channel went inactive. */
   TAPI_PPD_IE_NB_INACTIVE,
   /** this event is never generated,
      it is only used to mark end of TAPI_PPD_EVENT_ACTIONS_t table */
   TAPI_PPD_IE_END
} TAPI_PPD_INTERNAL_EVENTS_t;

/* Capacitance measurement flags. Indicate who is using the capacitance
   measurement. */
enum _TAPI_PPD_MEAS_FLAG
{
   /* Capacitance measurement used by the FXS Phone detection state machine. */
   TAPI_PPD_MEAS_FLAG_SM        = 0x01,
   /* Capacitance measurement used by user by calling API. */
   TAPI_PPD_MEAS_FLAG_API       = 0x02
};

/* Default timeout for T1 timer (in ms) */
#define TAPI_PPD_TIMEOUT_T1            3600000  /* 1h */

/* Default timeout for T2 timer (in ms) */
#define TAPI_PPD_TIMEOUT_T2            3000     /* 3s */

/* Default timeout for T3 timer (in ms) */
#define TAPI_PPD_TIMEOUT_T3            200

/* Default timeout for T4 timer (in ms). Default T4 timeout is set to 2s
   because firmware in worst case needs about 1.5s to swich line feeding
   to DISABLED and to finish capacity measurement. */
#define TAPI_PPD_TIMEOUT_T4            2000

/* Timeout for off-hook detection (in ms).
   The normal off-hook timeout is short to immediately react when the phone
   goes off-hook. But during off-hook detection is is set longer to filter
   out transient hook events. These happen with some electronic phones that
   draw their power from the line which causes transient hook events. */
#define TAPI_PPD_OFFHOOK_TIMEOUT       100 /* ms */

/* DUP counter setting during state offhook-detection (in ms).
   With a short time here phones drawing power from the line can be detected. */
#define TAPI_PPD_STDBY_DUP             2 /* ms */

/* Capacity threshold (nF) */
#define TAPI_PPD_CAPACITY_THRESHOLD    15

/* max enum ID */
#define TAPI_PPD_MAX_ENUM_ID  0xffffffff

/* String displayed when enum value is not found by ifx_tapi_ppd_Enum2Name. */
#define TAPI_PPD_ENUM_NOT_FOUND "(enum not found)"

/* Define for the capacitance measurement - do only tip to ring capacitance
   measurement. */
#define TAPI_PPD_T2R_CAP_ONLY   IFX_TRUE

/* ============================= */
/* Structures for PPM data        */
/* ============================= */
/* Structure to assigne enum name to its value (id) */
typedef struct _TAPI_PPD_ENUM_2_NAME_t
{
   IFX_int32_t nID;
   IFX_char_t* rgName;
} TAPI_PPD_ENUM_2_NAME_t;

/* structure with event and pointer to action that should be taken
   when this event ococcurs */
typedef struct _TAPI_PPD_EVENT_ACTIONS_t
{
   /* Event. */
   TAPI_PPD_INTERNAL_EVENTS_t    nEvent;
   /* Function that should be executed when nEvent ococcurs. */
   IFX_int32_t                   (*pAction)(TAPI_CHANNEL *);
} TAPI_PPD_EVENT_ACTIONS_t;

/* structure with state and pointer to table of TAPI_PPD_EVENT_ACTIONS_t
   with all handled events */
typedef struct _TAPI_PPD_STATE_COMBINATION_t
{
   /* State information. */
   TAPI_PPD_SM_STATES_t        nState;
   /* table with actions for nState. */
   TAPI_PPD_EVENT_ACTIONS_t*   pEventActions;
} TAPI_PPD_STATE_COMBINATION_t;

/* State machine data. */
typedef struct _TAPI_PPD_STATE_MACHINE_t
{
   /* IFX_TRUE - State Machine started. */
   IFX_boolean_t                   fSmStarted;
   /* Previous state machine start status. IFX_TRUE - state machine was
      started. Used for services like CID, Ringing - to restore previous status
      when given service is stopped. */
   IFX_boolean_t                   fPrevStartStatus;
   /* Current state. */
   TAPI_PPD_SM_STATES_t            nState;
   /* New internal event to handle */
   TAPI_PPD_INTERNAL_EVENTS_t      nIntEvent;
   /* Table with states and actions for each state. */
   TAPI_PPD_STATE_COMBINATION_t*   pStateCombination;
} TAPI_PPD_STATE_MACHINE_t;

/* Timer data. */
typedef struct _TAPI_PPD_TIMER_DATA_t
{
   /* Timer Id for PPD service. */
   Timer_ID                    PpdTimerId;
   /* Timeout index which is currently used for timer. Used only when timer
      is started. */
   TAPI_PPD_TIMEOUT_IDX_t      nActiveTimeout;
   /* IFX_TRUE - timer is started. */
   IFX_boolean_t               fTimerStarted;
   /* Table with timeouts for timer. */
   IFX_uint32_t                nTimeoutsTable[TAPI_PPD_MAX_TIMEOUTS];
} TAPI_PPD_TIMER_DATA_t;

struct TAPI_PPD_DATA
{
   /* IFX_TRUE - phone detected. */
   IFX_boolean_t               fPhoneDetected;
   /* Contains information who started the capacitance measurement:
      state machine or user by using API. */
   IFX_uint32_t                nMeasFlag;
   /* Mutex used to lock PPD state machine */
   TAPI_OS_mutex_t             hSemId;
   /* IFX_TRUE - Used to start C-measurement during initialization. Without
      this State Machine waits time configured for T1 until it starts
      C-measurement. This solution allows to save time during power
      measurement - no need to wait 1 hour (this is default time) until
      system detects no phone. */
   IFX_boolean_t               fFirstMeasurement;
   /* IFX_TRUE - state machine is suspended. Do not handle state transition.
      It used internally by PPD state machine, when line feeding mode must
      be changed during state transition. */
   IFX_boolean_t               fSuspend;
   /* IFX_TRUE - Phone Plug Detection functionality is enabled. */
   IFX_boolean_t               fIsEnabled;
   /* IFX_TRUE - Low level driver supports the capacitance measurement. */
   IFX_boolean_t               fLLSupport;
   /* Capacitance threshold for phone detection. */
   IFX_uint32_t                nCapacThreshold;
   /* Analog line capacitance measurement result. */
   IFX_TAPI_EVENT_LINE_MEASURE_CAPACITANCE_t  stCapacEventData;
   /* Timer data */
   TAPI_PPD_TIMER_DATA_t       TimerData;
   /* State Machine data */
   TAPI_PPD_STATE_MACHINE_t    StateMachineData;
};

/* ============================= */
/* Local function declarations   */
/* ============================= */
static IFX_void_t ifx_tapi_ppd_OnTimer(Timer_ID Timer, IFX_ulong_t nArg);
static IFX_char_t* ifx_tapi_ppd_Enum2Name(IFX_int32_t nEnum,
                                          TAPI_PPD_ENUM_2_NAME_t *pEnumName);
static IFX_int32_t ifx_tapi_ppd_HandleState(TAPI_CHANNEL *pChannel);
static IFX_int32_t ifx_tapi_ppd_Disabled_T2(TAPI_CHANNEL *pChannel);
static IFX_int32_t ifx_tapi_ppd_xxx_Service(TAPI_CHANNEL *pChannel);
static IFX_int32_t ifx_tapi_ppd_Standby_T1(TAPI_CHANNEL *pChannel);
static IFX_int32_t ifx_tapi_ppd_OffHookDet_T3(TAPI_CHANNEL *pChannel);
static IFX_int32_t ifx_tapi_ppd_OnHookDet_T4(TAPI_CHANNEL *pChannel);
static IFX_int32_t ifx_tapi_ppd_OnHookDet_CapRdy(TAPI_CHANNEL *pChannel);
static IFX_int32_t ifx_tapi_ppd_OnHookDet_Service(TAPI_CHANNEL *pChannel);
static IFX_int32_t ifx_tapi_ppd_OnHookDet_Stop(TAPI_CHANNEL *pChannel);
static IFX_int32_t ifx_tapi_ppd_NeighbourActive(
                        TAPI_CHANNEL *pChannel);
static IFX_int32_t ifx_tapi_ppd_NeighbourInactive(
                        TAPI_CHANNEL *pChannel);
static IFX_int32_t ifx_tapi_ppd_Ignore(
                        TAPI_CHANNEL *pChannel);
static void ifx_tapi_ppd_NbChannelsNotify(
                        TAPI_CHANNEL *pChannel,
                        IFX_operation_t nAction);
static IFX_boolean_t ifx_tapi_ppd_NbChannelsCheckActive(
                        TAPI_CHANNEL *pChannel);
static IFX_int32_t ifx_tapi_ppd_Set_Linefeed(TAPI_CHANNEL *pChannel,
                                             IFX_int32_t nMode);
static IFX_void_t ifx_tapi_ppd_StartTimer(TAPI_CHANNEL *pChannel,
                                          TAPI_PPD_TIMEOUT_IDX_t nTimeoutIdx);
static IFX_void_t ifx_tapi_ppd_StopTimer(TAPI_CHANNEL *pChannel);
static IFX_void_t ifx_tapi_ppd_UpdatePhoneDetectFlag(TAPI_CHANNEL *pChannel,
                                                 IFX_boolean_t fPhoneDetected);
static IFX_int32_t ifx_tapi_ppd_StopTimerAndMarkSM(TAPI_CHANNEL *pChannel);
static IFX_void_t ifx_tapi_ppd_StateTrans(TAPI_CHANNEL *pChannel,
                                          TAPI_PPD_SM_STATES_t nState);
static IFX_int32_t ifx_tapi_ppd_StartSM(TAPI_CHANNEL *pChannel);
static IFX_void_t ifx_tapi_ppd_Reset(TAPI_CHANNEL *pChannel);
#endif /* TAPI_FEAT_PHONE_DETECTION */

#ifdef TAPI_FEAT_POWER
static IFX_PMCU_RETURN_t tapi_pmc_state_get(
                              IFX_PMCU_STATE_t *pmcuState);
static IFX_PMCU_RETURN_t ifx_tapi_pmc_pwr_feature_switch(
                              IFX_PMCU_PWR_STATE_ENA_t pmcuPwrStateEna);
#endif /* TAPI_FEAT_POWER */

/* ============================= */
/* Local variable definition     */
/* ============================= */
#ifdef TAPI_FEAT_PHONE_DETECTION
/* Table of handled events with their handling functions.
   PPD state machine - for TAPI_PPD_STATE_LINE_DISABLED state. */
static TAPI_PPD_EVENT_ACTIONS_t rgLineDisabledStateData[] =
{
   { TAPI_PPD_IE_T2,                 ifx_tapi_ppd_Disabled_T2 },
   { TAPI_PPD_IE_API_CHANGE_FEEDING, ifx_tapi_ppd_StopTimerAndMarkSM },
   { TAPI_PPD_IE_SERVICE,            ifx_tapi_ppd_xxx_Service },
   { TAPI_PPD_IE_STOP,               ifx_tapi_ppd_StopTimerAndMarkSM },
   { TAPI_PPD_IE_HOOKOFF,            ifx_tapi_ppd_StopTimerAndMarkSM },
   { TAPI_PPD_IE_API_CAP_MEAS,       ifx_tapi_ppd_StopTimerAndMarkSM },
   { TAPI_PPD_IE_NB_ACTIVE,          ifx_tapi_ppd_NeighbourActive },
   { TAPI_PPD_IE_NB_INACTIVE,        ifx_tapi_ppd_Ignore },
   { TAPI_PPD_IE_END,                IFX_NULL}
};

/* Table of handled events with their handling functions.
   PPD state machine - for TAPI_PPD_STATE_LINE_STANDBY state. */
static TAPI_PPD_EVENT_ACTIONS_t rgLineStandbyStateData[] =
{
   { TAPI_PPD_IE_T1,                 ifx_tapi_ppd_Standby_T1 },
   { TAPI_PPD_IE_API_CHANGE_FEEDING, ifx_tapi_ppd_StopTimerAndMarkSM },
   { TAPI_PPD_IE_SERVICE,            ifx_tapi_ppd_xxx_Service },
   { TAPI_PPD_IE_STOP,               ifx_tapi_ppd_StopTimerAndMarkSM },
   { TAPI_PPD_IE_HOOKOFF,            ifx_tapi_ppd_StopTimerAndMarkSM },
   { TAPI_PPD_IE_API_CAP_MEAS,       ifx_tapi_ppd_StopTimerAndMarkSM },
   { TAPI_PPD_IE_NB_ACTIVE,          ifx_tapi_ppd_NeighbourActive },
   { TAPI_PPD_IE_NB_INACTIVE,        ifx_tapi_ppd_Ignore },
   { TAPI_PPD_IE_END,                IFX_NULL}
};

/* Table of handled events with their handling functions.
   PPD state machine - for TAPI_PPD_STATE_OFF_HOOK_DETECTION state. */
static TAPI_PPD_EVENT_ACTIONS_t rgOffHookDetectionStateData[] =
{
   { TAPI_PPD_IE_T3,                 ifx_tapi_ppd_OffHookDet_T3 },
   { TAPI_PPD_IE_API_CHANGE_FEEDING, ifx_tapi_ppd_StopTimerAndMarkSM },
   { TAPI_PPD_IE_HOOKOFF,            ifx_tapi_ppd_StopTimerAndMarkSM },
   { TAPI_PPD_IE_SERVICE,            ifx_tapi_ppd_xxx_Service },
   { TAPI_PPD_IE_STOP,               ifx_tapi_ppd_StopTimerAndMarkSM },
   { TAPI_PPD_IE_API_CAP_MEAS,       ifx_tapi_ppd_StopTimerAndMarkSM },
   { TAPI_PPD_IE_NB_ACTIVE,          ifx_tapi_ppd_NeighbourActive },
   { TAPI_PPD_IE_NB_INACTIVE,        ifx_tapi_ppd_Ignore },
   { TAPI_PPD_IE_END,                IFX_NULL}
};

/* Table of handled events with their handling functions.
   PPD state machine - for TAPI_PPD_STATE_ON_HOOK_DETECTION state. */
static TAPI_PPD_EVENT_ACTIONS_t rgOnHookDetectionStateData[] =
{
   { TAPI_PPD_IE_T4,                 ifx_tapi_ppd_OnHookDet_T4 },
   { TAPI_PPD_IE_API_CHANGE_FEEDING, ifx_tapi_ppd_OnHookDet_Stop },
   { TAPI_PPD_IE_LINE_MEASURE_CAPACITANCE_RDY, ifx_tapi_ppd_OnHookDet_CapRdy },
   { TAPI_PPD_IE_SERVICE,            ifx_tapi_ppd_OnHookDet_Service },
   { TAPI_PPD_IE_STOP,               ifx_tapi_ppd_OnHookDet_Stop },
   { TAPI_PPD_IE_HOOKOFF,            ifx_tapi_ppd_OnHookDet_Stop },
   { TAPI_PPD_IE_API_CAP_MEAS,       ifx_tapi_ppd_OnHookDet_Stop },
   { TAPI_PPD_IE_NB_ACTIVE,          ifx_tapi_ppd_NeighbourActive },
   { TAPI_PPD_IE_NB_INACTIVE,        ifx_tapi_ppd_Ignore },
   { TAPI_PPD_IE_END,                IFX_NULL}
};

/* Table of handled events with their handling functions.
   PPD state machine - for TAPI_PPD_STATE_LINE_SUSPENDED state. */
static TAPI_PPD_EVENT_ACTIONS_t rgSuspendedStateData[] =
{
   { TAPI_PPD_IE_HOOKOFF,            ifx_tapi_ppd_StopTimerAndMarkSM },
   { TAPI_PPD_IE_API_CHANGE_FEEDING, ifx_tapi_ppd_StopTimerAndMarkSM },
   { TAPI_PPD_IE_API_CAP_MEAS,       ifx_tapi_ppd_StopTimerAndMarkSM },
   { TAPI_PPD_IE_STOP,               ifx_tapi_ppd_StopTimerAndMarkSM },
   { TAPI_PPD_IE_SERVICE,            ifx_tapi_ppd_xxx_Service },
   { TAPI_PPD_IE_NB_ACTIVE,          ifx_tapi_ppd_Ignore },
   { TAPI_PPD_IE_NB_INACTIVE,        ifx_tapi_ppd_NeighbourInactive },
   { TAPI_PPD_IE_END,                IFX_NULL}
};

/* Table of available states with their TAPI_PPD_EVENT_ACTIONS_t tables
   for state machine. */
TAPI_PPD_STATE_COMBINATION_t rgPPDStatesData[] =
{
   { TAPI_PPD_STATE_LINE_DISABLED,            rgLineDisabledStateData },
   { TAPI_PPD_STATE_LINE_STANDBY,             rgLineStandbyStateData },
   { TAPI_PPD_STATE_OFF_HOOK_DETECTION,       rgOffHookDetectionStateData },
   { TAPI_PPD_STATE_ON_HOOK_DETECTION,        rgOnHookDetectionStateData },
   { TAPI_PPD_STATE_SUSPENDED,                rgSuspendedStateData },
   { TAPI_PPD_STATE_END,                      IFX_NULL }
};

/** names of internal events */
TAPI_PPD_ENUM_2_NAME_t rgInternalEventsName[] =
{
   {TAPI_PPD_IE_NONE, "TAPI_PPD_IE_NONE"},
   {TAPI_PPD_IE_T1, "TAPI_PPD_IE_T1"},
   {TAPI_PPD_IE_T2, "TAPI_PPD_IE_T2"},
   {TAPI_PPD_IE_T3, "TAPI_PPD_IE_T3"},
   {TAPI_PPD_IE_T4, "TAPI_PPD_IE_T4"},
   {TAPI_PPD_IE_API_CHANGE_FEEDING, "TAPI_PPD_IE_API_CHANGE_FEEDING"},
   {TAPI_PPD_IE_LINE_MEASURE_CAPACITANCE_RDY, "TAPI_PPD_IE_LINE_MEASURE_CAPACITANCE_RDY"},
   {TAPI_PPD_IE_STOP, "TAPI_PPD_IE_STOP"},
   {TAPI_PPD_IE_SERVICE, "TAPI_PPD_IE_SERVICE"},
   {TAPI_PPD_IE_HOOKOFF, "TAPI_PPD_IE_HOOKOFF"},
   {TAPI_PPD_IE_NB_ACTIVE, "TAPI_PPD_IE_NB_ACTIVE"},
   {TAPI_PPD_IE_NB_INACTIVE, "TAPI_PPD_IE_NB_INACTIVE"},
   {TAPI_PPD_IE_API_CAP_MEAS, "TAPI_PPD_IE_API_CAP_MEAS"},
   /** this event is never generated,
      it is only used to mark end of EVENT_ACTIONS_t table */
   {TAPI_PPD_IE_END, "TAPI_PPD_IE_END"},
   {TAPI_PPD_MAX_ENUM_ID, "TAPI_PPD_MAX_ENUM_ID"}
};

/** names of states */
TAPI_PPD_ENUM_2_NAME_t rgStateName[] =
{
   {TAPI_PPD_STATE_LINE_DISABLED, "LINE_DISABLED"},
   {TAPI_PPD_STATE_LINE_STANDBY, "LINE_STANDBY"},
   {TAPI_PPD_STATE_OFF_HOOK_DETECTION, "OFF_HOOK_DETECTION"},
   {TAPI_PPD_STATE_ON_HOOK_DETECTION, "ON_HOOK_DETECTION"},
   {TAPI_PPD_STATE_SUSPENDED, "SUSPENDED"},
   {TAPI_PPD_MAX_ENUM_ID, "END OF TABLE"}
};
#endif /* TAPI_FEAT_PHONE_DETECTION */

#ifdef TAPI_FEAT_POWER
/** IFX_TRUE - Phone Detection feature is enabled by PMCU. Otherwise it
    is disabled. */
static IFX_boolean_t fPmcu_FeatureEnabled = IFX_TRUE;

#ifdef TAPI_FEAT_CPUFREQ_IF
/* Linux CPUFREQ support start */
struct ltq_cpufreq_module_info ppd_cpufreq_feature_pds = {
   .name                            = "VOICE Phone detection support",
   .pmcuModule                      = LTQ_CPUFREQ_MODULE_VE,
   .pmcuModuleNr                    = 0,
   .powerFeatureStat                = 1,
   .ltq_cpufreq_state_get           = tapi_pmc_state_get,
   .ltq_cpufreq_pwr_feature_switch  = ifx_tapi_pmc_pwr_feature_switch
};
/* Linux CPUFREQ support end */
#endif /* TAPI_FEAT_CPUFREQ_IF */
#endif /* TAPI_FEAT_POWER */


#ifdef TAPI_FEAT_PHONE_DETECTION
/* ============================= */
/* Local function definition     */
/* ============================= */
/**
   Function to return the name assigned to a enum value

   \param   nEnum  -  enum value (id)
   \param   pEnumName -  pointer to array with enum name

   \return  pointer to enum name. If enum value is not found or in case of
            error, pointer to string with error description is returned.
*/
static IFX_char_t* ifx_tapi_ppd_Enum2Name(IFX_int32_t nEnum,
                                          TAPI_PPD_ENUM_2_NAME_t *pEnumName)
{
   IFX_char_t *pName = IFX_NULL;

   TAPI_ASSERT (pEnumName);
   /* check pointer, on error return pointer to error string */
/*    TD_PTR_CHECK(pEnumName, TD_rgEnum2NameErrorCode[ENUM_NULL_POINTER]); */

   /* search for nEnum value */
   do
   {
      /* check if value is the same */
      if (pEnumName->nID == nEnum)
      {
         /* set enum name string */
         pName = pEnumName->rgName;
      }
      pEnumName++;
   } while ((pName == IFX_NULL) && (pEnumName->nID != TAPI_PPD_MAX_ENUM_ID));

   if (pName == IFX_NULL)
   {
      /* if pName is IFX_NULL, it means that pEnumName points to last
         element in the table. Assign string with info. */
      pName = TAPI_PPD_ENUM_NOT_FOUND;
   }
   return pName;
}

/**
   State transition.

   \param pChannel      Pointer to TAPI_CHANNEL structure.
   \param nState        New state.
*/
static IFX_void_t ifx_tapi_ppd_StateTrans(TAPI_CHANNEL *pChannel,
                                          TAPI_PPD_SM_STATES_t nState)
{
   TAPI_PPD_STATE_MACHINE_t *pStateMachineData = IFX_NULL;

   TAPI_ASSERT (pChannel);
   pStateMachineData = &pChannel->pTapiPpdData->StateMachineData;

   pStateMachineData->nState = nState;
/*    TRACE(TAPI_DRV, DBG_LEVEL_LOW,
         ("TAPI PPD, %s:%d ch%d: state changed to: %s\n",
          pChannel->pTapiDevice->pDevDrvCtx->drvName,
          pChannel->pTapiDevice->nDev,
          pChannel->nChannel,
          ifx_tapi_ppd_Enum2Name(pStateMachineData->nState, rgStateName))); */
}

/**
   Start Timer.

   \param pChannel      Pointer to TAPI_CHANNEL structure.
   \param nTimeoutIdx   Index to table with timeout value.
*/
static IFX_void_t ifx_tapi_ppd_StartTimer(TAPI_CHANNEL *pChannel,
                                          TAPI_PPD_TIMEOUT_IDX_t nTimeoutIdx)
{
   TAPI_PPD_TIMER_DATA_t *pTimerData = IFX_NULL;
   IFX_uint32_t nTime;

   TAPI_ASSERT (pChannel);
   pTimerData = &pChannel->pTapiPpdData->TimerData;

   /* Lookup delay value. */
   nTime = pTimerData->nTimeoutsTable[nTimeoutIdx];

   /* Start timer. If timer was already started then it is stopped and
      again started but with new timeout value. */
   TAPI_SetTime_Timer (pTimerData->PpdTimerId, nTime, IFX_FALSE, IFX_FALSE);
   /* Set current timeout index. */
   pTimerData->nActiveTimeout = nTimeoutIdx;
   /* Mark timer as started. */
   pTimerData->fTimerStarted = IFX_TRUE;

/*    TRACE(TAPI_DRV, DBG_LEVEL_LOW,
         ("CTAPI PPD, %s:%d ch%d: Start T%d \n",
          pChannel->pTapiDevice->pDevDrvCtx->drvName,
          pChannel->pTapiDevice->nDev,
          pChannel->nChannel, (nTimeoutIdx+1))); */
}

/**
   Stop Timer.

   \param pChannel      Pointer to TAPI_CHANNEL structure.
*/
static IFX_void_t ifx_tapi_ppd_StopTimer(TAPI_CHANNEL *pChannel)
{
   TAPI_PPD_TIMER_DATA_t *pTimerData = IFX_NULL;
   /* IFX_uint32_t nTimeoutIdx; */

   TAPI_ASSERT (pChannel);
   pTimerData = &pChannel->pTapiPpdData->TimerData;

   if (pTimerData->fTimerStarted == IFX_FALSE)
   {
      /* Timer is already stopped. Nothing to do. */
      return;
   }
   /* Stop timer. */
   TAPI_Stop_Timer (pTimerData->PpdTimerId);
   /* Get timeout index which was used for timer. Needed for debug trace. */
   /* nTimeoutIdx = pTimerData->nActiveTimeout; */
   /* Mark timer as stopped. */
   pTimerData->fTimerStarted = IFX_FALSE;

/*    TRACE(TAPI_DRV, DBG_LEVEL_LOW,
         ("TAPI PPD, %s:%d ch%d: Stop T%d \n",
          pChannel->pTapiDevice->pDevDrvCtx->drvName,
          pChannel->pTapiDevice->nDev,
          pChannel->nChannel, (nTimeoutIdx+1))); */
}

/**
   Stores information about phone detect state.

   \param pChannel             Pointer to TAPI_CHANNEL structure.
   \param fPhoneDetected       IFX_TRUE - phone detected
                               IFX_FALSE - phone not detected
*/
static IFX_void_t ifx_tapi_ppd_UpdatePhoneDetectFlag(TAPI_CHANNEL *pChannel,
                                                 IFX_boolean_t fPhoneDetected)
{
   TAPI_PPD_DATA_t *pTapiPpdData = IFX_NULL;

   TAPI_ASSERT (pChannel);
   pTapiPpdData = pChannel->pTapiPpdData;
   /* Update phone detect flag and print debug information if phone detection
      state is changed. */
   if (pTapiPpdData->fPhoneDetected != fPhoneDetected)
   {
      pTapiPpdData->fPhoneDetected = fPhoneDetected;
      if (fPhoneDetected == IFX_TRUE)
      {
         TRACE(TAPI_DRV, DBG_LEVEL_LOW,
               ("TAPI PPD, %s:%d ch%d: Phone detected.\n",
                pChannel->pTapiDevice->pDevDrvCtx->drvName,
                pChannel->pTapiDevice->nDev,
                pChannel->nChannel));
      }
      else
      {
         TRACE(TAPI_DRV, DBG_LEVEL_LOW,
               ("TAPI PPD, %s:%d ch%d: No phone detected.\n",
                pChannel->pTapiDevice->pDevDrvCtx->drvName,
                pChannel->pTapiDevice->nDev,
                pChannel->nChannel));
      }
   }
}

/**
   Stops timer and marks state machine as not started.

   \param pChannel             Pointer to TAPI_CHANNEL structure.

   \return TAPI_statusOk
*/
static IFX_int32_t ifx_tapi_ppd_StopTimerAndMarkSM(TAPI_CHANNEL *pChannel)
{
   TAPI_PPD_STATE_MACHINE_t *pStateMachineData = IFX_NULL;
   IFX_TAPI_DRV_CTX_t* pDrvCtx = IFX_NULL;

   TAPI_ASSERT (pChannel);
   pStateMachineData = &pChannel->pTapiPpdData->StateMachineData;
   pDrvCtx = pChannel->pTapiDevice->pDevDrvCtx;
   /* Stop timer. */
   ifx_tapi_ppd_StopTimer(pChannel);
   /* Restore the DUP time. */
   if (pDrvCtx->ALM.CfgDupTimer_Override)
   {
      pDrvCtx->ALM.CfgDupTimer_Override(pChannel->pLLChannel, DUP_OVERRIDE_OFF);
   }
#ifdef TAPI_FEAT_DIAL
   /* Turn off overwrite. This works only because the PPD SM event handling
      is executed after the dial SM and there the time was already started. */
   IFX_TAPI_Dial_OffhookTime_Override(pChannel, IFX_TAPI_DIAL_TIME_NORMAL);
#endif /* TAPI_FEAT_DIAL */
   /* Mark that state machine is stopped. */
   pStateMachineData->fSmStarted = IFX_FALSE;
   return TAPI_statusOk;
}

/**
   Sets the linefeeding mode of the device

   \param pChannel      Pointer to TAPI_CHANNEL structure.
   \param nMode         Line mode.

   \return
     - TAPI_statusOk: if successful
     - IFX_ERROR: in case of an error
*/
static IFX_int32_t ifx_tapi_ppd_Set_Linefeed(TAPI_CHANNEL *pChannel,
                                             IFX_int32_t nMode)
{
   TAPI_PPD_DATA_t *pTapiPpdData = IFX_NULL;
   IFX_int32_t     ret = TAPI_statusOk;

   TAPI_ASSERT (pChannel);
   pTapiPpdData = pChannel->pTapiPpdData;
   /* Suspend the FXS PPD state machine. Calling TAPI_Phone_Set_Linefeed()
      won't influence on state machine. */
   pTapiPpdData->fSuspend = IFX_TRUE;
   ret = TAPI_Phone_Set_Linefeed (pChannel, nMode);
   /* Release state machine. */
   pTapiPpdData->fSuspend = IFX_FALSE;
   return ret;
}

/**
   Handles a state transitions of the Phone Detection state machine.

   \param   pChannel    Pointer to TAPI_CHANNEL structure.

   \return
     - TAPI_statusOk: if successful
     - IFX_ERROR: in case of an error
*/
static IFX_int32_t ifx_tapi_ppd_HandleState(TAPI_CHANNEL *pChannel)
{
   int i=0, j=0;
   IFX_int32_t ret = TAPI_statusOk;
   IFX_boolean_t fContinue = IFX_TRUE;
   TAPI_PPD_EVENT_ACTIONS_t* pActionsList;
   TAPI_PPD_DATA_t *pTapiPpdData = IFX_NULL;
   TAPI_PPD_STATE_MACHINE_t *pStateMachineData = IFX_NULL;
   IFX_TAPI_DRV_CTX_t* pDrvCtx = IFX_NULL;

   TAPI_ASSERT (pChannel);
   pTapiPpdData = pChannel->pTapiPpdData;
   pStateMachineData = &pTapiPpdData->StateMachineData;

   if ((pTapiPpdData->fIsEnabled == IFX_FALSE) ||
       (pStateMachineData->fSmStarted == IFX_FALSE))
   {
      /* Phone detection functionality is disabled or State machine is stopped.
      This is not an error. */
      return TAPI_statusOk;
   }

      TRACE(TAPI_DRV, DBG_LEVEL_LOW,
         ("TAPI PPD, %s:%d ch%d: state: %s, event: %s.\n",
          pChannel->pTapiDevice->pDevDrvCtx->drvName,
          pChannel->pTapiDevice->nDev,
          pChannel->nChannel,
          ifx_tapi_ppd_Enum2Name(pStateMachineData->nState, rgStateName),
          ifx_tapi_ppd_Enum2Name(pStateMachineData->nIntEvent,
                                 rgInternalEventsName)));

   /* Looking for selected state */
   while (TAPI_PPD_STATE_END !=  pStateMachineData->pStateCombination[i].nState)
   {
      if (pStateMachineData->pStateCombination[i].nState ==
          pStateMachineData->nState)
      {
         /* Select proper array with events */
         pActionsList = pStateMachineData->pStateCombination[i].pEventActions;

         /* Looking for selected event for particular state */
         while(IFX_TRUE == fContinue)
         {
            /* check if end of TAPI_PPD_EVENT_ACTIONS_t table */
            if (pActionsList[j].nEvent == TAPI_PPD_IE_END)
            {
               TRACE(TAPI_DRV, DBG_LEVEL_LOW,
                 ("\nTAPI PPD, %s:%d ch%d: event %s not handled by state %s.\n",
                     pChannel->pTapiDevice->pDevDrvCtx->drvName,
                     pChannel->pTapiDevice->nDev,
                     pChannel->nChannel,
                     ifx_tapi_ppd_Enum2Name(pStateMachineData->nIntEvent,
                                            rgInternalEventsName),
                     ifx_tapi_ppd_Enum2Name(pStateMachineData->nState,
                                            rgStateName)
                     ));
               /* Didn't find selected event for particular state */
               pStateMachineData->nIntEvent = TAPI_PPD_IE_NONE;
               /* leave while loop */
               fContinue = IFX_FALSE;
            }
            else if (pActionsList[j].nEvent == pStateMachineData->nIntEvent)
            {
               /* Execute action for selected event */
               ret = (*pActionsList[j].pAction)(pChannel);
               /* clear event. */
               pStateMachineData->nIntEvent = TAPI_PPD_IE_NONE;
               if (!TAPI_SUCCESS(ret))
               {
                  TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
                        ("\nDRV_ERROR: TAPI PPD, action for event %s failed "
                         "(%s:%d ch %d).\n",
                         ifx_tapi_ppd_Enum2Name(pActionsList[j].nEvent,
                                                rgInternalEventsName),
                         pChannel->pTapiDevice->pDevDrvCtx->drvName,
                         pChannel->pTapiDevice->nDev,
                         pChannel->nChannel));
                  return ret;
               }
               /* leave while loop */
               fContinue = IFX_FALSE;
            } /* if (pActionsList[j].nEvent == ..) */
            j++;
         } /* while(IFX_TRUE == fContinue) */
         /* state found - leave for loop */
         break;
      }
      i++;
   }
   /* if fContinue value is set to IFX_TRUE then
      pStateMachineData->nState was not found in
      pStateMachineData->pStateCombination state list - should not happend. */
   if (IFX_TRUE == fContinue)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
            ("\nDRV_ERROR: TAPI PPD, state %d not found (%s:%d ch %d, event %s ).\n",
             pStateMachineData->nState,
             pChannel->pTapiDevice->pDevDrvCtx->drvName,
             pChannel->pTapiDevice->nDev,
             pChannel->nChannel,
             ifx_tapi_ppd_Enum2Name(pStateMachineData->nIntEvent,
                                    rgInternalEventsName)));
      /* Should not happend. Stop state machine, stop capacitance measurement
         if started by state machine and set line to STANDBY. */
      pStateMachineData->fSmStarted = IFX_FALSE;
      pDrvCtx = pChannel->pTapiDevice->pDevDrvCtx;
      if (pChannel->pTapiPpdData->nMeasFlag & TAPI_PPD_MEAS_FLAG_SM)
      {
         pTapiPpdData->nMeasFlag &= ~TAPI_PPD_MEAS_FLAG_SM;
         if (!(pChannel->pTapiPpdData->nMeasFlag & TAPI_PPD_MEAS_FLAG_API) &&
          (pDrvCtx->ALM.CapMeasStop != IFX_NULL))
      {
         pDrvCtx->ALM.CapMeasStop (pChannel->pLLChannel);
      }
      }
      if (!(pChannel->pTapiPpdData->nMeasFlag & TAPI_PPD_MEAS_FLAG_API))
      {
         ifx_tapi_ppd_Set_Linefeed(pChannel, IFX_TAPI_LINE_FEED_STANDBY);
      }
      /* errmsg: PPD state machine failed */
      RETURN_STATUS (TAPI_statusInitPpdStateMachineFailed, 0);
   } /* if (IFX_TRUE == fContinue) */

   return TAPI_statusOk;
}

/**
   Resets the FXS Phone Detection state machine variables.

   \param pChannel      Pointer to TAPI_CHANNEL structure.
*/
static IFX_void_t ifx_tapi_ppd_Reset(TAPI_CHANNEL *pChannel)
{
   TAPI_PPD_STATE_MACHINE_t *pStateMachineData = IFX_NULL;

   TAPI_ASSERT (pChannel);
   pStateMachineData = &pChannel->pTapiPpdData->StateMachineData;
   pStateMachineData->nIntEvent = TAPI_PPD_IE_NONE;
   pStateMachineData->fSmStarted = IFX_TRUE;
   pStateMachineData->fPrevStartStatus = IFX_FALSE;
   pChannel->pTapiPpdData->nMeasFlag &= ~TAPI_PPD_MEAS_FLAG_SM;
   pChannel->pTapiPpdData->fSuspend = IFX_FALSE;
}

/**
   Starts the FXS Phone Detection state machine.

   \param pChannel      Pointer to TAPI_CHANNEL structure.

   \return
     - TAPI_statusOk: if successful
     - IFX_ERROR: in case of an error
*/
static IFX_int32_t ifx_tapi_ppd_StartSM(TAPI_CHANNEL *pChannel)
{
   TAPI_PPD_DATA_t *pTapiPpdData = IFX_NULL;
   IFX_int32_t ret = TAPI_statusOk;

   TAPI_ASSERT (pChannel);
   pTapiPpdData = pChannel->pTapiPpdData;

   /* Set line feeding to STANDBY. */
   ret = ifx_tapi_ppd_Set_Linefeed(pChannel, IFX_TAPI_LINE_FEED_STANDBY);
   if (!TAPI_SUCCESS(ret))
   {
      /* Mark that state machine is stopped. */
      pTapiPpdData->StateMachineData.fSmStarted = IFX_FALSE;
      return ret;
   }

   /* If any neighbour channel is active change directly to SUSPENDED state. */
   if (ifx_tapi_ppd_NbChannelsCheckActive(pChannel) == IFX_TRUE)
   {
      ifx_tapi_ppd_StateTrans(pChannel, TAPI_PPD_STATE_SUSPENDED);
      return ret;
   }

   if (pTapiPpdData->fFirstMeasurement == IFX_FALSE)
   {
      ret = ifx_tapi_ppd_Standby_T1(pChannel);
      /* Mark that first C-measurement was executed. Next C-measurements
         will be executed according to T1. */
      pTapiPpdData->fFirstMeasurement = IFX_TRUE;
      return ret;
   }

   /* Check stored "detect state". */
   if (pTapiPpdData->fPhoneDetected)
   {
      /* Set the TAPI_PPD_STATE_LINE_STANDBY state in the PPD State Machine. */
      ifx_tapi_ppd_StateTrans(pChannel, TAPI_PPD_STATE_LINE_STANDBY);
      /* Start Timer - T1. */
      ifx_tapi_ppd_StartTimer(pChannel, TAPI_PPD_T1_IDX);
   }
   else
   {
      IFX_TAPI_DRV_CTX_t* pDrvCtx = pChannel->pTapiDevice->pDevDrvCtx;

      /* Reducing the standby DUP time will help with the detection of
         phones which drain power but are slow with the loop current. */
      if (pDrvCtx->ALM.CfgDupTimer_Override)
      {
         pDrvCtx->ALM.CfgDupTimer_Override(pChannel->pLLChannel,
                                           TAPI_PPD_STDBY_DUP);
      }
#ifdef TAPI_FEAT_DIAL
      /* Override the off-hook timeout of the state machine. */
      IFX_TAPI_Dial_OffhookTime_Override(pChannel, TAPI_PPD_OFFHOOK_TIMEOUT);
#endif /* TAPI_FEAT_DIAL */

      /* Set the TAPI_PPD_STATE_OFF_HOOK_DETECTION state in the
         PPD State Machine. */
      ifx_tapi_ppd_StateTrans(pChannel, TAPI_PPD_STATE_OFF_HOOK_DETECTION);

      /* Start Timer - T3. */
      ifx_tapi_ppd_StartTimer(pChannel, TAPI_PPD_T3_IDX);
   }
   return ret;
}

/**
   Action when detected TAPI_PPD_IE_T2 event for TAPI_PPD_STATE_LINE_DISABLED
   state.

   Set Standby, change to state off-hook detection, and start timer T3.

   \param   pChannel    Pointer to TAPI_CHANNEL structure.

   \return
     - TAPI_statusOk: if successful
     - IFX_ERROR: in case of an error
*/
static IFX_int32_t ifx_tapi_ppd_Disabled_T2(TAPI_CHANNEL *pChannel)
{
   IFX_int32_t ret = TAPI_statusOk;

   TAPI_ASSERT (pChannel);

   /* Set line feeding to STANDBY. */
   ret = ifx_tapi_ppd_Set_Linefeed(pChannel, IFX_TAPI_LINE_FEED_STANDBY);

   if (TAPI_SUCCESS(ret))
   {
      /* Set the TAPI_PPD_STATE_OFF_HOOK_DETECTION state in the
         PPD State Machine. */
      ifx_tapi_ppd_StateTrans(pChannel, TAPI_PPD_STATE_OFF_HOOK_DETECTION);

      /* Start Timer - T3. */
      ifx_tapi_ppd_StartTimer(pChannel, TAPI_PPD_T3_IDX);
   }
   else
   {
      /* Mark that state machine is stopped. */
      pChannel->pTapiPpdData->StateMachineData.fSmStarted = IFX_FALSE;
      return ret;
   }

   return TAPI_statusOk;
}

/**
   Action when detected TAPI_PPD_IE_SERVICE event.
   Stop T2 and set line feeding to STANDBY. Stop state machine.

   \param   pChannel    Pointer to TAPI_CHANNEL structure.

   \return
     - TAPI_statusOk: if successful
     - IFX_ERROR: in case of an error
*/
static IFX_int32_t ifx_tapi_ppd_xxx_Service(TAPI_CHANNEL *pChannel)
{
   TAPI_PPD_STATE_MACHINE_t *pStateMachineData = IFX_NULL;
   IFX_TAPI_DRV_CTX_t* pDrvCtx = IFX_NULL;

   TAPI_ASSERT (pChannel);
   pStateMachineData = &pChannel->pTapiPpdData->StateMachineData;
   pDrvCtx = pChannel->pTapiDevice->pDevDrvCtx;
   /* Stop timer. */
   ifx_tapi_ppd_StopTimer(pChannel);
   /* Restore the DUP time. */
   if (pDrvCtx->ALM.CfgDupTimer_Override)
   {
      pDrvCtx->ALM.CfgDupTimer_Override(pChannel->pLLChannel, DUP_OVERRIDE_OFF);
   }
#ifdef TAPI_FEAT_DIAL
   /* Turn off overwrite. This works only because the PPD SM event handling
      is executed after the dial SM and there the time was already started. */
   IFX_TAPI_Dial_OffhookTime_Override(pChannel, IFX_TAPI_DIAL_TIME_NORMAL);
#endif /* TAPI_FEAT_DIAL */
   /* Stop state machine. */
   pStateMachineData->fSmStarted = IFX_FALSE;
   /* Set line feeding to STANDBY. */
   return ifx_tapi_ppd_Set_Linefeed(pChannel, IFX_TAPI_LINE_FEED_STANDBY);
}

/**
   Action when detected TAPI_PPD_IE_T1 event for
   TAPI_PPD_STATE_LINE_STANDBY state.
   Start GR909 phone detect measurement and start T4. Transition to
   TAPI_PPD_STATE_ON_HOOK_DETECTION state.

   \param   pChannel    Pointer to TAPI_CHANNEL structure.

   \return
     - TAPI_statusOk: if successful
     - IFX_ERROR: in case of an error
*/
static IFX_int32_t ifx_tapi_ppd_Standby_T1(TAPI_CHANNEL *pChannel)
{
   TAPI_PPD_DATA_t *pTapiPpdData = IFX_NULL;
   TAPI_PPD_STATE_MACHINE_t *pStateMachineData = IFX_NULL;
   IFX_TAPI_DRV_CTX_t* pDrvCtx = IFX_NULL;
   IFX_int32_t ret = TAPI_statusOk;

   TAPI_ASSERT (pChannel);
   pTapiPpdData = pChannel->pTapiPpdData;
   pStateMachineData = &pTapiPpdData->StateMachineData;
   pDrvCtx = pChannel->pTapiDevice->pDevDrvCtx;

   pStateMachineData->nIntEvent = TAPI_PPD_IE_NONE;

   /* Start GR909 phone detect measurement. */
   if (pDrvCtx->ALM.CapMeasStart == IFX_NULL)
   {
   /* TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
           ("\nDRV_ERROR, TAPI PPD: Capacitance measurement not supported by "
            "low level driver (%s:%d ch %d).\n",
            pChannel->pTapiDevice->pDevDrvCtx->drvName,
            pChannel->pTapiDevice->nDev,
            pChannel->nChannel)); */
      /* Stop state machine. */
      pStateMachineData->fSmStarted = IFX_FALSE;
      RETURN_STATUS (TAPI_statusLLNotSupp, 0);
   }

   /* Do not start capacitance measurement if it is already started by user.
      Measurement result will be reused. */
   if (!(pTapiPpdData->nMeasFlag & TAPI_PPD_MEAS_FLAG_API))
   {
      ret = pDrvCtx->ALM.CapMeasStart (pChannel->pLLChannel,
                                       TAPI_PPD_T2R_CAP_ONLY);
      if (!TAPI_SUCCESS(ret))
      {
         /* Set line feeding to STANDBY. */
         ifx_tapi_ppd_Set_Linefeed(pChannel, IFX_TAPI_LINE_FEED_STANDBY);
         /* Transition to state TAPI_PPD_STATE_LINE_STANDBY. */
         ifx_tapi_ppd_StateTrans(pChannel, TAPI_PPD_STATE_LINE_STANDBY);
         /* Start Timer - T1. */
         ifx_tapi_ppd_StartTimer(pChannel, TAPI_PPD_T1_IDX);
         /* errmsg: LL driver returned an error */
         RETURN_STATUS (TAPI_statusLLFailed, ret);
      }
   }

   /* Mark that C-measurement is active. */
   pTapiPpdData->nMeasFlag |= TAPI_PPD_MEAS_FLAG_SM;
   /* Transition to state TAPI_PPD_STATE_ON_HOOK_DETECTION. */
   ifx_tapi_ppd_StateTrans(pChannel, TAPI_PPD_STATE_ON_HOOK_DETECTION);
   /* Start Timer - T4. */
   ifx_tapi_ppd_StartTimer(pChannel, TAPI_PPD_T4_IDX);
   return ret;
}

/**
   Action when detected TAPI_PPD_IE_T3 event for
   TAPI_PPD_STATE_OFF_HOOK_DETECTION state.
   Set line feeding to DISABLED, start T2 and do transition to
   TAPI_PPD_STATE_LINE_DISABLED state.

   \param   pChannel    Pointer to TAPI_CHANNEL structure.

   \return
     - TAPI_statusOk: if successful
     - IFX_ERROR: in case of an error
*/
static IFX_int32_t ifx_tapi_ppd_OffHookDet_T3(TAPI_CHANNEL *pChannel)
{
   TAPI_PPD_STATE_MACHINE_t *pStateMachineData = IFX_NULL;
   IFX_int32_t ret = TAPI_statusOk;

   TAPI_ASSERT (pChannel);
   pStateMachineData = &pChannel->pTapiPpdData->StateMachineData;

   pStateMachineData->nIntEvent = TAPI_PPD_IE_NONE;

   /* Set line feeding to DISABLED. */
   ret = ifx_tapi_ppd_Set_Linefeed(pChannel, IFX_TAPI_LINE_FEED_DISABLED);
   if (!TAPI_SUCCESS(ret))
   {
      /* Mark that state machine is stopped. */
      pStateMachineData->fSmStarted = IFX_FALSE;
      return ret;
   }

   /* Transition to state TAPI_PPD_STATE_LINE_DISABLED. */
   ifx_tapi_ppd_StateTrans(pChannel, TAPI_PPD_STATE_LINE_DISABLED);
   /* Start Timer - T2. */
   ifx_tapi_ppd_StartTimer(pChannel, TAPI_PPD_T2_IDX);
   return ret;
}

/**
   Action when detected TAPI_PPD_IE_T4 event for
   TAPI_PPD_STATE_ON_HOOK_DETECTION state.
   Stop GR909 phone detect measurement, mark internal data that phone is not
   detected, set line feeding to STANDBY and start T3. Transition to
   TAPI_PPD_STATE_OFF_HOOK_DETECTION state.

   \param   pChannel    Pointer to TAPI_CHANNEL structure.

   \return
     - TAPI_statusOk: if successful
     - IFX_ERROR: in case of an error
*/
static IFX_int32_t ifx_tapi_ppd_OnHookDet_T4(TAPI_CHANNEL *pChannel)
{
   TAPI_PPD_STATE_MACHINE_t *pStateMachineData = IFX_NULL;
   IFX_TAPI_DRV_CTX_t* pDrvCtx = IFX_NULL;
   IFX_int32_t ret = TAPI_statusOk;

   TAPI_ASSERT (pChannel);
   pStateMachineData = &pChannel->pTapiPpdData->StateMachineData;
   pDrvCtx = pChannel->pTapiDevice->pDevDrvCtx;

   pStateMachineData->nIntEvent = TAPI_PPD_IE_NONE;

   /* Without a result from the capacitance measurement there is no information
      about the phone. So in this state of uncertainty the preference is on
      providing service and not power saving. So we define that the phone is
      still plugged. This is done by keeping the phone detection flag set as
      it was when we came here. */

   /* Stop GR909 phone detect measurement. */
   if (pDrvCtx->ALM.CapMeasStop == IFX_NULL)
   {
   /* TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
           ("\nDRV_ERROR, TAPI PPD: Capacitance measurement not supported by "
            "low level driver (%s:%d ch %d).\n",
            pChannel->pTapiDevice->pDevDrvCtx->drvName,
            pChannel->pTapiDevice->nDev,
            pChannel->nChannel)); */
      /* Stop state machine. */
      pStateMachineData->fSmStarted = IFX_FALSE;
      RETURN_STATUS (TAPI_statusLLNotSupp, 0);
   }

   /* Mark that C-measurement is stopped. */
   pChannel->pTapiPpdData->nMeasFlag = 0;
   ret = pDrvCtx->ALM.CapMeasStop (pChannel->pLLChannel);
   if (!TAPI_SUCCESS(ret))
   {
      /* Stop state machine. */
      pStateMachineData->fSmStarted = IFX_FALSE;
      /* errmsg: LL driver returned an error */
      RETURN_STATUS (TAPI_statusLLFailed, ret);
   }

   /* Finally a warning that this state means that the capacitance measurement
      has unexpectedly failed. */
   TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
        ("\nTAPI PPD: Capacitance measurement failed unexpectedly! "
         "(%s:%d ch %d)\n",
         pChannel->pTapiDevice->pDevDrvCtx->drvName,
         pChannel->pTapiDevice->nDev,
         pChannel->nChannel));

   return ifx_tapi_ppd_StartSM(pChannel);
}

/**
   Action when detected TAPI_PPD_IE_LINE_MEASURE_CAPACITANCE_RDY event for
   TAPI_PPD_STATE_ON_HOOK_DETECTION state.
   Stop T4 and check measured capacity.
   - Capacity above threshold - phone detected, set line feeding to STANDBY,
                                start T1 and do transition to
                                TAPI_PPD_STATE_LINE_STANDBY state.
   - Capacity below threshold - phone not detected, mark internal data that
                                phone is not detected set line feeding to
                                STANDBY, start T3 and do transition to
                                TAPI_PPD_STATE_OFF_HOOK_DETECTION state.

   \param   pChannel    Pointer to TAPI_CHANNEL structure.

   \return
     - TAPI_statusOk: if successful
     - IFX_ERROR: in case of an error
*/
static IFX_int32_t ifx_tapi_ppd_OnHookDet_CapRdy(TAPI_CHANNEL *pChannel)
{
   TAPI_PPD_DATA_t *pTapiPpdData = IFX_NULL;
   TAPI_PPD_STATE_MACHINE_t *pStateMachineData = IFX_NULL;
   IFX_int32_t ret = TAPI_statusOk;

   TAPI_ASSERT (pChannel);
   pTapiPpdData = pChannel->pTapiPpdData;
   pStateMachineData = &pChannel->pTapiPpdData->StateMachineData;

   pStateMachineData->nIntEvent = TAPI_PPD_IE_NONE;
   /* Stop timer */
   ifx_tapi_ppd_StopTimer(pChannel);

   if (pTapiPpdData->stCapacEventData.nReturnCode == IFX_SUCCESS)
   {
/*       TRACE(TAPI_DRV, DBG_LEVEL_LOW,
         ("TAPI PPD, %s:%d ch%d: Measured capacitance: %d.\n",
          pChannel->pTapiDevice->pDevDrvCtx->drvName,
          pChannel->pTapiDevice->nDev,
          pChannel->nChannel, pTapiPpdData->stCapacEventData.nCapacitance)); */
      /* Structure contains correct capacitance. */
      if (pTapiPpdData->stCapacEventData.nCapacitance <=
          pTapiPpdData->nCapacThreshold)
      {
         /* Capacitance is below threshold, so mark that phone is
            not detected. */
         ifx_tapi_ppd_UpdatePhoneDetectFlag(pChannel, IFX_FALSE);
      }
      else
      {
         /* Mark that phone is detected. */
         ifx_tapi_ppd_UpdatePhoneDetectFlag(pChannel, IFX_TRUE);
      }
   }
   else
   {
      /* There was an error during capacitance measurement. */
      /* TRACE(TAPI_DRV, DBG_LEVEL_NORMAL,
            ("\nDRV_WARNING, TAPI PPD, %s:%d ch%d: Capacity measurement result is "
             "invalid, return code: 0x%x. \n",
             pChannel->pTapiDevice->pDevDrvCtx->drvName,
             pChannel->pTapiDevice->nDev,
             pChannel->nChannel, pTapiPpdData->stCapacEventData.nReturnCode)); */
   }

   ret = pChannel->pTapiDevice->pDevDrvCtx->ALM.CapMeasStop (pChannel->pLLChannel);
   if (!TAPI_SUCCESS(ret))
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
             ("\nDRV_ERROR, TAPI PPD: stop of cap. measurement failed on %s:%d "
              "ch %d.\n",
              pChannel->pTapiDevice->pDevDrvCtx->drvName,
              pChannel->pTapiDevice->nDev,
              pChannel->nChannel));
      ifx_tapi_ppd_StartSM(pChannel);
      /* errmsg: LL driver returned an error */
      RETURN_STATUS (TAPI_statusLLFailed, ret);
   }
   return ifx_tapi_ppd_StartSM(pChannel);
}

/**
   Action when detected TAPI_PPD_IE_SERVICE event for
   TAPI_PPD_STATE_ON_HOOK_DETECTION state.
   Stop T4, Stop GR909 phone detect measurement and set line feeding to STANDBY.
   Stop state machine.

   \param   pChannel    Pointer to TAPI_CHANNEL structure.

   \return
     - TAPI_statusOk: if successful
     - IFX_ERROR: in case of an error
*/
static IFX_int32_t ifx_tapi_ppd_OnHookDet_Service(TAPI_CHANNEL *pChannel)
{
   IFX_int32_t ret = TAPI_statusOk;

   TAPI_ASSERT (pChannel);
   ret = ifx_tapi_ppd_OnHookDet_Stop(pChannel);
   /* Set line feeding to STANDBY. */
   if (TAPI_SUCCESS(ret))
   {
      ret = ifx_tapi_ppd_Set_Linefeed(pChannel, IFX_TAPI_LINE_FEED_STANDBY);
   }
   return ret;
}

/**
   Action when detected TAPI_PPD_IE_STOP event for
   TAPI_PPD_STATE_ON_HOOK_DETECTION state.
   Stop T4, Stop GR909 phone detect measurement.
   Stop state machine.

   \param   pChannel    Pointer to TAPI_CHANNEL structure.

   \return
     - TAPI_statusOk: if successful
     - IFX_ERROR: in case of an error
*/
static IFX_int32_t ifx_tapi_ppd_OnHookDet_Stop(TAPI_CHANNEL *pChannel)
{
   TAPI_PPD_STATE_MACHINE_t *pStateMachineData = IFX_NULL;
   IFX_TAPI_DRV_CTX_t* pDrvCtx = IFX_NULL;
   IFX_int32_t ret = TAPI_statusOk;

   TAPI_ASSERT (pChannel);
   pStateMachineData = &pChannel->pTapiPpdData->StateMachineData;
   pDrvCtx = pChannel->pTapiDevice->pDevDrvCtx;

   /* Stop GR909 phone detect measurement. */
   if (pDrvCtx->ALM.CapMeasStop == IFX_NULL)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
           ("\nDRV_ERROR, TAPI PPD: Capacitance measurement not supported by "
            "low level driver (%s:%d ch %d).\n",
            pChannel->pTapiDevice->pDevDrvCtx->drvName,
            pChannel->pTapiDevice->nDev,
            pChannel->nChannel));
      /* Stop state machine. */
      pStateMachineData->fSmStarted = IFX_FALSE;
      RETURN_STATUS (TAPI_statusLLNotSupp, 0);
   }

   ret = pDrvCtx->ALM.CapMeasStop (pChannel->pLLChannel);
   /* Mark that C-measurement is stopped. */
   pChannel->pTapiPpdData->nMeasFlag = 0;
   /* Stop timer */
   ifx_tapi_ppd_StopTimer(pChannel);
   /* Stop state machine. */
   pStateMachineData->fSmStarted = IFX_FALSE;
   if (!TAPI_SUCCESS(ret))
   {
      /* errmsg: LL driver returned an error */
      RETURN_STATUS (TAPI_statusLLFailed, ret);
   }
   return ret;
}

/**
   PPD-timer callback function.

   \param   Timer       TimerID of timer that expired.
   \param   nArg        Argument of timer. This argument is a pointer to the
                        TAPI_CHANNEL structure.

   \return  none.
*/
static IFX_void_t ifx_tapi_ppd_OnTimer(Timer_ID Timer, IFX_ulong_t nArg)
{
   TAPI_CHANNEL *pChannel  = (TAPI_CHANNEL *) nArg;
   TAPI_PPD_DATA_t *pTapiPpdData = IFX_NULL;
   TAPI_PPD_TIMER_DATA_t *pTimerData = IFX_NULL;

   IFX_UNUSED (Timer);
   TAPI_ASSERT (pChannel);
   pTapiPpdData = pChannel->pTapiPpdData;
   pTimerData = &pTapiPpdData->TimerData;

   /* lock PPD data */
   TAPI_OS_MutexGet (&pTapiPpdData->hSemId);
   /* Mark timer as stopped. */
   pTimerData->fTimerStarted = IFX_FALSE;
   /* Set event for PPD State Machine. */
   switch (pTimerData->nActiveTimeout)
   {
      case TAPI_PPD_T1_IDX:
         pTapiPpdData->StateMachineData.nIntEvent = TAPI_PPD_IE_T1;
         break;

      case TAPI_PPD_T2_IDX:
         pTapiPpdData->StateMachineData.nIntEvent = TAPI_PPD_IE_T2;
         break;

      case TAPI_PPD_T3_IDX:
         pTapiPpdData->StateMachineData.nIntEvent = TAPI_PPD_IE_T3;
         break;

      case TAPI_PPD_T4_IDX:
         pTapiPpdData->StateMachineData.nIntEvent = TAPI_PPD_IE_T4;
         break;

      default:
         /* Should not happen. */
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
               ("\nDRV_ERROR, TAPI PPD: ifx_tapi_ppd_OnTimer received wrong Timeout Index, "
                "Timeout Idx %d, %s:%d ch%d.\n",
                pTimerData->nActiveTimeout,
                pChannel->pTapiDevice->pDevDrvCtx->drvName,
                pChannel->pTapiDevice->nDev, pChannel->nChannel));
         /* unlock PPD data */
         TAPI_OS_MutexRelease (&pTapiPpdData->hSemId);
         return;
   }

   /* Do state transition. */
   ifx_tapi_ppd_HandleState(pChannel);
   /* unlock PPD data */
   TAPI_OS_MutexRelease (&pTapiPpdData->hSemId);
}


/**
   Action when detected TAPI_PPD_IE_NB_ACTIVE event.

   Stop Timer, capacitance measurement and go to silent state.

   \param  pChannel     Pointer to TAPI_CHANNEL structure.

   \return
   - TAPI_statusOk: if successful
   - IFX_ERROR: in case of an error
*/
static IFX_int32_t ifx_tapi_ppd_NeighbourActive(
                        TAPI_CHANNEL *pChannel)
{
   IFX_TAPI_DRV_CTX_t* pDrvCtx = IFX_NULL;
   IFX_int32_t ret = TAPI_statusOk;

   TAPI_ASSERT (pChannel);
   pDrvCtx = pChannel->pTapiDevice->pDevDrvCtx;

   /* Stop GR909 phone detect measurement. */
   if (pDrvCtx->ALM.CapMeasStop != IFX_NULL)
   {
      ret = pDrvCtx->ALM.CapMeasStop (pChannel->pLLChannel);
      /* Mark that C-measurement is stopped. */
      pChannel->pTapiPpdData->nMeasFlag = 0;
   }

   /* Stop timer. */
   ifx_tapi_ppd_StopTimer(pChannel);
   /* Restore the DUP time. */
   if (pDrvCtx->ALM.CfgDupTimer_Override)
   {
      pDrvCtx->ALM.CfgDupTimer_Override(pChannel->pLLChannel, DUP_OVERRIDE_OFF);
   }
#ifdef TAPI_FEAT_DIAL
   /* Turn off overwrite. This works only because the PPD SM event handling
      is executed after the dial SM and there the time was already started. */
   IFX_TAPI_Dial_OffhookTime_Override(pChannel, IFX_TAPI_DIAL_TIME_NORMAL);
#endif /* TAPI_FEAT_DIAL */

   /* Set line feeding to STANDBY. */
   ifx_tapi_ppd_Set_Linefeed(pChannel, IFX_TAPI_LINE_FEED_STANDBY);

   /* Transition to state TAPI_PPD_STATE_LINE_STANDBY. */
   ifx_tapi_ppd_StateTrans(pChannel, TAPI_PPD_STATE_SUSPENDED);

   if (!TAPI_SUCCESS(ret))
   {
      /* errmsg: LL driver returned an error */
      RETURN_STATUS (TAPI_statusLLFailed, ret);
   }
   return ret;
}


/**
   Action when detected TAPI_PPD_IE_NB_INACTIVE event.

   Depending on the phone detect state set the linefeeding and change the
   state.

   \param  pChannel     Pointer to TAPI_CHANNEL structure.

   \return
   - TAPI_statusOk: if successful
*/
static IFX_int32_t ifx_tapi_ppd_NeighbourInactive(
                        TAPI_CHANNEL *pChannel)
{
   TAPI_PPD_DATA_t *pTapiPpdData = IFX_NULL;
   IFX_TAPI_DRV_CTX_t* pDrvCtx = IFX_NULL;

   TAPI_ASSERT (pChannel);
   pTapiPpdData = pChannel->pTapiPpdData;
   pDrvCtx = pChannel->pTapiDevice->pDevDrvCtx;

   /* Check stored "detect state". */
   if (pTapiPpdData->fPhoneDetected)
   {
      /* Set line feeding to STANDBY. */
      ifx_tapi_ppd_Set_Linefeed(pChannel, IFX_TAPI_LINE_FEED_STANDBY);
      /* Next state: LINE STANDBY. */
      ifx_tapi_ppd_StateTrans(pChannel, TAPI_PPD_STATE_LINE_STANDBY);
      /* Start Timer - T1. */
      ifx_tapi_ppd_StartTimer(pChannel, TAPI_PPD_T1_IDX);
   }
   else
   {
      /* Set line feeding to DISABLED. */
      ifx_tapi_ppd_Set_Linefeed(pChannel, IFX_TAPI_LINE_FEED_DISABLED);
      /* Next state: LINE DISABLED. */
      ifx_tapi_ppd_StateTrans(pChannel, TAPI_PPD_STATE_LINE_DISABLED);

      /* Reducing the standby DUP time will help with the detection of
         phones which drain power but are slow with the loop current. */
      if (pDrvCtx->ALM.CfgDupTimer_Override)
      {
         pDrvCtx->ALM.CfgDupTimer_Override(pChannel->pLLChannel,
                                           TAPI_PPD_STDBY_DUP);
      }
#ifdef TAPI_FEAT_DIAL
      /* Override the off-hook timeout of the state machine. */
      IFX_TAPI_Dial_OffhookTime_Override(pChannel, TAPI_PPD_OFFHOOK_TIMEOUT);
#endif /* TAPI_FEAT_DIAL */

      /* Start Timer - T2. */
      ifx_tapi_ppd_StartTimer(pChannel, TAPI_PPD_T2_IDX);
   }

   return TAPI_statusOk;
}


/**
   Action to silently ignore an event.

   \param  pChannel     Pointer to TAPI_CHANNEL structure.

   \return
   - TAPI_statusOk: if successful
*/
static IFX_int32_t ifx_tapi_ppd_Ignore(
                        TAPI_CHANNEL *pChannel)
{
   IFX_UNUSED (pChannel);

   return TAPI_statusOk;
}
#endif /* TAPI_FEAT_PHONE_DETECTION */


#ifdef TAPI_FEAT_CAP_MEAS
/**
   Start an analog line capacitance measurement.
   PPD State Machine is stopped.

   \param pChannel      Pointer to TAPI_CHANNEL structure.
   \param fObsoleteApi  IFX_TRUE - IFX_TAPI_LINE_MEASURE_CAPACITANCE_START used
                        IFX_FALSE - IFX_TAPI_NLT_CAPACITANCE_START used.

   \return
     - TAPI_statusOk: if successful
     - IFX_ERROR: in case of an error
*/
static IFX_int32_t ifx_tapi_ppd_CapMeasStart(TAPI_CHANNEL *pChannel,
                                             IFX_boolean_t fObsoleteApi)
{
   IFX_TAPI_DRV_CTX_t* pDrvCtx = IFX_NULL;
#ifdef TAPI_FEAT_PHONE_DETECTION
   TAPI_PPD_DATA_t *pTapiPpdData = IFX_NULL;
#endif /* TAPI_FEAT_PHONE_DETECTION */
   IFX_TAPI_EVENT_t Event;
   IFX_int32_t ret = TAPI_statusOk;

   TAPI_ASSERT (pChannel);
   pDrvCtx = pChannel->pTapiDevice->pDevDrvCtx;
#ifdef TAPI_FEAT_PHONE_DETECTION
   pTapiPpdData = pChannel->pTapiPpdData;
   /* Make sure the ppd data struct exists on this channel. */
   if (pTapiPpdData == IFX_NULL)
   {
      /** Service not supported on called channel context */
      RETURN_STATUS (TAPI_statusInvalidCh, 0);
   }
   /* lock PPD data */
   TAPI_OS_MutexGet (&pTapiPpdData->hSemId);
#endif /* TAPI_FEAT_PHONE_DETECTION */
   if (pDrvCtx->ALM.CapMeasStart == IFX_NULL)
   {
   /* TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
           ("\nDRV_ERROR, TAPI PPD: Capacitance measurement not supported by "
            "low level driver (%s:%d ch %d).\n",
            pChannel->pTapiDevice->pDevDrvCtx->drvName,
            pChannel->pTapiDevice->nDev,
            pChannel->nChannel)); */
      /* unlock PPD data */
#ifdef TAPI_FEAT_PHONE_DETECTION
      TAPI_OS_MutexRelease (&pTapiPpdData->hSemId);
#endif /* TAPI_FEAT_PHONE_DETECTION */
      RETURN_STATUS (TAPI_statusLLNotSupp, 0);
   }

#ifdef TAPI_FEAT_PHONE_DETECTION
   if (pTapiPpdData->nMeasFlag == TAPI_PPD_MEAS_FLAG_API)
   {
   /* TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
           ("\nDRV_ERROR, TAPI PPD: Capacitance measurement is already active "
            "(%s:%d ch %d).\n",
            pChannel->pTapiDevice->pDevDrvCtx->drvName,
            pChannel->pTapiDevice->nDev,
            pChannel->nChannel)); */
      /* unlock PPD data */
      TAPI_OS_MutexRelease (&pTapiPpdData->hSemId);
      /* errmsg: Capacitance measurement is already active */
      RETURN_STATUS (TAPI_statusCapMeasStartWhileActive, 0);
   }

   /* Inform state machine that user starts capacity measurement. */
   pTapiPpdData->StateMachineData.nIntEvent = TAPI_PPD_IE_API_CAP_MEAS;
   ret = ifx_tapi_ppd_HandleState(pChannel);
   if (!TAPI_SUCCESS(ret))
   {
      /* unlock PPD data */
      TAPI_OS_MutexRelease (&pTapiPpdData->hSemId);
      return ret;
   }

   pTapiPpdData->nMeasFlag = TAPI_PPD_MEAS_FLAG_API;
#endif /* TAPI_FEAT_PHONE_DETECTION */
   /* For the IFX_TAPI_LINE_MEASURE_CAPACITANCE_START command, the
      IFX_TAPI_EVENT_LINE_MEASURE_CAPACITANCE_RDY event is used to inform user
      about the end of capacitance measurement and to send results.
      When the IFX_TAPI_NLT_CAPACITANCE_START command is used, user is informed
      that capacitance measurement is finished by IFX_TAPI_EVENT_NLT_END. In
      this case IFX_TAPI_EVENT_LINE_MEASURE_CAPACITANCE_RDY should not be send
      to the user application.
      Store current TAPI events mask configuration and enable/disable proper
      events. */
   if (fObsoleteApi == IFX_TRUE)
   {
      Event.id = IFX_TAPI_EVENT_LINE_MEASURE_CAPACITANCE_RDY;
      IFX_TAPI_Event_SetMask(pChannel, &Event, IFX_EVENT_ENABLE);
      Event.id = IFX_TAPI_EVENT_LINE_MEASURE_CAPACITANCE_GND_RDY;
      IFX_TAPI_Event_SetMask(pChannel, &Event, IFX_EVENT_ENABLE);
      Event.id = IFX_TAPI_EVENT_NLT_END;
      IFX_TAPI_Event_SetMask(pChannel, &Event, IFX_EVENT_DISABLE);
   }
   else
   {
      Event.id = IFX_TAPI_EVENT_LINE_MEASURE_CAPACITANCE_RDY;
      IFX_TAPI_Event_SetMask(pChannel, &Event, IFX_EVENT_DISABLE);
      Event.id = IFX_TAPI_EVENT_LINE_MEASURE_CAPACITANCE_GND_RDY;
      IFX_TAPI_Event_SetMask(pChannel, &Event, IFX_EVENT_DISABLE);
      Event.id = IFX_TAPI_EVENT_NLT_END;
      IFX_TAPI_Event_SetMask(pChannel, &Event, IFX_EVENT_ENABLE);
   }
   /* Start the capacity measurement. Measure tip to ring and line to ground
      capacitances. */
   ret = pDrvCtx->ALM.CapMeasStart (pChannel->pLLChannel, IFX_FALSE);
   if (!TAPI_SUCCESS(ret))
   {
#ifdef TAPI_FEAT_PHONE_DETECTION
      pTapiPpdData->nMeasFlag &= ~TAPI_PPD_MEAS_FLAG_API;
      /* unlock PPD data */
      TAPI_OS_MutexRelease (&pTapiPpdData->hSemId);
#endif /* TAPI_FEAT_PHONE_DETECTION */
      RETURN_STATUS (TAPI_statusLLFailed, ret);
   }

#ifdef TAPI_FEAT_PHONE_DETECTION
   /* unlock PPD data */
   TAPI_OS_MutexRelease (&pTapiPpdData->hSemId);
#endif /* TAPI_FEAT_PHONE_DETECTION */
   return TAPI_statusOk;
}
#endif /* TAPI_FEAT_CAP_MEAS */

#ifdef TAPI_FEAT_CAP_MEAS
/**
   Stop an analog line capacitance measurement.

   \param pChannel      Pointer to TAPI_CHANNEL structure.

   \return
     - TAPI_statusOk: if successful
     - IFX_ERROR: in case of an error

   \remark
   This function can't be used by the FXS Phone Detection state machine.
*/
static IFX_int32_t ifx_tapi_ppd_CapMeasStop(TAPI_CHANNEL *pChannel)
{
   IFX_TAPI_DRV_CTX_t* pDrvCtx = IFX_NULL;
#ifdef TAPI_FEAT_PHONE_DETECTION
   TAPI_PPD_DATA_t *pTapiPpdData = IFX_NULL;
#endif /* TAPI_FEAT_PHONE_DETECTION */
   IFX_int32_t ret = TAPI_statusOk;

   TAPI_ASSERT (pChannel);
   pDrvCtx = pChannel->pTapiDevice->pDevDrvCtx;
#ifdef TAPI_FEAT_PHONE_DETECTION
   pTapiPpdData = pChannel->pTapiPpdData;
   /* lock PPD data */
   TAPI_OS_MutexGet (&pTapiPpdData->hSemId);
#endif /* TAPI_FEAT_PHONE_DETECTION */
   if (pDrvCtx->ALM.CapMeasStop == IFX_NULL)
   {
   /* TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
           ("\nDRV_ERROR, TAPI PPD: Capacitance measurement not supported by "
            "low level driver (%s:%d ch %d).\n",
            pChannel->pTapiDevice->pDevDrvCtx->drvName,
            pChannel->pTapiDevice->nDev,
            pChannel->nChannel)); */
#ifdef TAPI_FEAT_PHONE_DETECTION
      /* unlock PPD data */
      TAPI_OS_MutexRelease (&pTapiPpdData->hSemId);
#endif /* TAPI_FEAT_PHONE_DETECTION */
      RETURN_STATUS (TAPI_statusLLNotSupp, 0);
   }

#ifdef TAPI_FEAT_PHONE_DETECTION
   pTapiPpdData->nMeasFlag &= ~TAPI_PPD_MEAS_FLAG_API;
#endif /* TAPI_FEAT_PHONE_DETECTION */
   ret = pDrvCtx->ALM.CapMeasStop (pChannel->pLLChannel);
   if (!TAPI_SUCCESS(ret))
   {
      /* unlock PPD data */
#ifdef TAPI_FEAT_PHONE_DETECTION
      TAPI_OS_MutexRelease (&pTapiPpdData->hSemId);
#endif /* TAPI_FEAT_PHONE_DETECTION */
      RETURN_STATUS (TAPI_statusLLFailed, ret);
   }

#ifdef TAPI_FEAT_PHONE_DETECTION
   /* unlock PPD data */
   TAPI_OS_MutexRelease (&pTapiPpdData->hSemId);
#endif /* TAPI_FEAT_PHONE_DETECTION */
   return TAPI_statusOk;
}
#endif /* TAPI_FEAT_CAP_MEAS */

#ifdef TAPI_FEAT_PHONE_DETECTION
/* ============================= */
/* Global function definition    */
/* ============================= */
/**
   Initialise FXS Phone Detection state machine on the given channel.

   Initialise the data structures and resources needed for the FXS Phone
   Detection state machine.

   \param   pChannel    Pointer to TAPI_CHANNEL structure.

   \return
     - TAPI_statusOk: if successful
     - IFX_ERROR: in case of an error
*/
IFX_int32_t IFX_TAPI_PPD_Initialise_Unprot(TAPI_CHANNEL *pChannel)
{
   TAPI_PPD_DATA_t *pTapiPpdData = IFX_NULL;
   TAPI_PPD_TIMER_DATA_t *pTimerData = IFX_NULL;
   IFX_TAPI_DRV_CTX_t* pDrvCtx = IFX_NULL;

   TAPI_ASSERT (pChannel);
   TAPI_ASSERT (pChannel->pTapiDevice);
   TAPI_ASSERT (pChannel->pTapiDevice->pDevDrvCtx);
   pDrvCtx = pChannel->pTapiDevice->pDevDrvCtx;
   pTapiPpdData = pChannel->pTapiPpdData;

   if ((pDrvCtx->ALM.CapMeasStop == IFX_NULL) ||
       (pDrvCtx->ALM.CapMeasStart == IFX_NULL))
   {
      /* Low level driver doesn't support phone detection. */
      return TAPI_statusOk;
   }

   /* check if channel has the required analog module */
   if ((pChannel->nChannel >=  pChannel->pTapiDevice->nResource.AlmCount) ||
       !((pChannel->TapiOpControlData.nLineType == IFX_TAPI_LINE_TYPE_FXS_NB) ||
         (pChannel->TapiOpControlData.nLineType == IFX_TAPI_LINE_TYPE_FXS_WB) ||
         (pChannel->TapiOpControlData.nLineType == IFX_TAPI_LINE_TYPE_FXS_AUTO)
        ))
   {
      /* no analog module -> nothing to do  --  this is not an error */
      return TAPI_statusOk;
   }

   if (pTapiPpdData != IFX_NULL)
   {
      /* It is already initialized. */
      return TAPI_statusOk;
   }

   /* allocate data storage */
   if ((pTapiPpdData = TAPI_OS_Malloc (sizeof(*pTapiPpdData))) == IFX_NULL)
   {
      RETURN_STATUS (TAPI_statusNoMem, 0);
   }
   /* Store pointer to data in the channel before we lose it on exit. */
   pChannel->pTapiPpdData = pTapiPpdData;
   memset (pTapiPpdData, 0x00, sizeof(*pTapiPpdData));

   if (pDrvCtx->ALM.CheckCapMeasSup == IFX_NULL)
   {
      /* Low level driver doesn't support this function. It is not error.
         To be compliant with older versions we assume that driver supports
         the capacitance measurment. */
      pTapiPpdData->fLLSupport = IFX_TRUE;
   }
   else
   {
      pDrvCtx->ALM.CheckCapMeasSup(pChannel->pLLChannel,
                                   &pTapiPpdData->fLLSupport);
   }

   pTimerData = &pTapiPpdData->TimerData;
   /* initialize (create) timer */
   pTimerData->PpdTimerId =
      TAPI_Create_Timer((TIMER_ENTRY)ifx_tapi_ppd_OnTimer,
                        (IFX_uintptr_t)pChannel);
   if(pTimerData->PpdTimerId == 0)
   {
      /** errmsg: Timer creation failed */
      RETURN_STATUS (TAPI_statusTimerFail, 0);
   }

   /* Initialise the PPD access mutex */
   TAPI_OS_MutexInit (&pTapiPpdData->hSemId);
   /* Default reset value is IFX_TRUE - phone detected. */
   pTapiPpdData->fPhoneDetected = IFX_TRUE;
   pTapiPpdData->nCapacThreshold = TAPI_PPD_CAPACITY_THRESHOLD;
   /* Feature is enabled unless global flag is set otherwise. */
   pTapiPpdData->fIsEnabled = IFX_TRUE;
#ifdef TAPI_FEAT_POWER
   if(fPmcu_FeatureEnabled == IFX_FALSE)
      pTapiPpdData->fIsEnabled = IFX_FALSE;
#endif /* TAPI_FEAT_POWER */
   /* Mark that first C-measurement was not executed yet. */
   pTapiPpdData->fFirstMeasurement = IFX_FALSE;
   /* Initialize timeouts. */
   pTimerData->nTimeoutsTable[TAPI_PPD_T1_IDX] = TAPI_PPD_TIMEOUT_T1;
   pTimerData->nTimeoutsTable[TAPI_PPD_T2_IDX] = TAPI_PPD_TIMEOUT_T2;
   pTimerData->nTimeoutsTable[TAPI_PPD_T3_IDX] = TAPI_PPD_TIMEOUT_T3;
   pTimerData->nTimeoutsTable[TAPI_PPD_T4_IDX] = TAPI_PPD_TIMEOUT_T4;
   /* Mark timer as stopped. */
   pTimerData->fTimerStarted = IFX_FALSE;

   pTapiPpdData->stCapacEventData.nReturnCode = IFX_SUCCESS;
   /* State machine is stopped. */
   pTapiPpdData->StateMachineData.fSmStarted = IFX_FALSE;
   pTapiPpdData->StateMachineData.pStateCombination = rgPPDStatesData;
   pTapiPpdData->StateMachineData.nIntEvent = TAPI_PPD_IE_NONE;
   pTapiPpdData->nMeasFlag = 0;

   return TAPI_statusOk;
}

/**
   Cleanup  FXS Phone Detection State Machine on the given channel.

   Free the resources needed for the FXS PPD state machine.

   \param   pChannel    Pointer to TAPI_CHANNEL structure.
*/
IFX_void_t IFX_TAPI_PPD_Cleanup(TAPI_CHANNEL *pChannel)
{
   TAPI_ASSERT (pChannel);
   /* lock channel */
   TAPI_OS_MutexGet (&pChannel->semTapiChDataLock);

   if (pChannel->pTapiPpdData != IFX_NULL)
   {
      TAPI_PPD_DATA_t *pTapiPpdData = pChannel->pTapiPpdData;

      /* unconditionally destruct the timer if existing */
      if (pTapiPpdData->TimerData.PpdTimerId != 0)
      {
         TAPI_Delete_Timer (pTapiPpdData->TimerData.PpdTimerId);
         pTapiPpdData->TimerData.PpdTimerId = 0;
      }

      /* Delete the PPD access mutex */
      TAPI_OS_MutexDelete (&pTapiPpdData->hSemId);
      /* free the data storage on the channel */
      TAPI_OS_Free (pChannel->pTapiPpdData);
      pChannel->pTapiPpdData = IFX_NULL;
   }
   /* unlock channel */
   TAPI_OS_MutexRelease (&pChannel->semTapiChDataLock);
}

/**
   Handles line feeding.
   IFX_TAPI_FEED_PHONE_DETECT - FXS Phone Detection state machine is
   started. Other line feeding modes stop PPD state machine.

   \param pChannel      Pointer to TAPI_CHANNEL structure.
   \param pMode         pointer to line mode.

   \return
     - TAPI_statusOk: if successful
     - IFX_ERROR: in case of an error
*/
IFX_int32_t IFX_TAPI_PPD_HandleLineFeeding(TAPI_CHANNEL *pChannel,
                                           IFX_uint8_t *pMode)
{
   TAPI_PPD_DATA_t *pTapiPpdData = IFX_NULL;
   TAPI_PPD_STATE_MACHINE_t *pStateMachineData = IFX_NULL;
   IFX_TAPI_DRV_CTX_t* pDrvCtx = IFX_NULL;
   IFX_int32_t ret = TAPI_statusOk;

   TAPI_ASSERT (pChannel);
   TAPI_ASSERT (pChannel->pTapiDevice);
   TAPI_ASSERT (pChannel->pTapiDevice->pDevDrvCtx);
   TAPI_ASSERT (pMode);
   pDrvCtx = pChannel->pTapiDevice->pDevDrvCtx;
   pTapiPpdData = pChannel->pTapiPpdData;

   /* Notify neighbouring channel state machines first. */
   if ((pTapiPpdData == IFX_NULL) ||
       (pTapiPpdData->fSuspend == IFX_FALSE))
   {
      /* Analyse all linemode changes not caused by phone detection itself. */
      if ((*pMode == IFX_TAPI_LINE_FEED_DISABLED) ||
          (*pMode == IFX_TAPI_LINE_FEED_STANDBY) ||
          (*pMode == IFX_TAPI_LINE_FEED_PHONE_DETECT))
      {
         /* In all these modes allow phone detection on the other channel. */
         ifx_tapi_ppd_NbChannelsNotify(pChannel, IFX_ENABLE);
      }
      else
      {
         /* In all other modes disable phone detection on the other channel. */
         ifx_tapi_ppd_NbChannelsNotify(pChannel, IFX_DISABLE);
      }
   }

   /* Handle this channel. */

   if (pTapiPpdData == IFX_NULL)
   {
      /* The FXS Phone Detection state machine is not initialized
         for this channel. */
      return TAPI_statusOk;
   }

   /* Ignore the linefeed changes caused by this statemachine itself. */
   if (IFX_TRUE == pTapiPpdData->fSuspend)
   {
      return TAPI_statusOk;
   }

   /* Exit here should the low level driver not support the capacitance
      measurement. */
   if (pDrvCtx->ALM.CheckCapMeasSup != IFX_NULL)
   {
      /* Always update because the capability can change after an BBD download.
      */
      pDrvCtx->ALM.CheckCapMeasSup(pChannel->pLLChannel,
                                   &pTapiPpdData->fLLSupport);
   }
   if (pTapiPpdData->fLLSupport == IFX_FALSE)
   {
      if (*pMode == IFX_TAPI_LINE_FEED_PHONE_DETECT)
      {
         /* Modify the linefeeding from "detect" to regular standby. */
         *pMode = IFX_TAPI_LINE_FEED_STANDBY;

         TRACE(TAPI_DRV, DBG_LEVEL_LOW,
              ("\nDRV_WARNING: Phone Detection is not supported. "
               "STANDBY used instead of DETECT (%s:%d ch %d).\n",
               pChannel->pTapiDevice->pDevDrvCtx->drvName,
               pChannel->pTapiDevice->nDev,
               pChannel->nChannel));
      }

      return TAPI_statusOk;
   }

   /* Exit here when the phone detection is disabled by administration. */
   if (pTapiPpdData->fIsEnabled == IFX_FALSE)
   {
      /* The FXS Phone Detection functionality is disabled. */
      if (*pMode == IFX_TAPI_LINE_FEED_PHONE_DETECT)
      {
         /* Modify the linefeeding from "detect" to regular standby. */
         *pMode = IFX_TAPI_LINE_FEED_STANDBY;

         TRACE(TAPI_DRV, DBG_LEVEL_LOW,
              ("\nDRV_WARNING: Phone Detection is not enabled. "
               "STANDBY used instead of DETECT (%s:%d ch %d).\n",
               pChannel->pTapiDevice->pDevDrvCtx->drvName,
               pChannel->pTapiDevice->nDev,
               pChannel->nChannel));
      }

      return TAPI_statusOk;
   }

   /* lock PPD data */
   TAPI_OS_MutexGet (&pTapiPpdData->hSemId);

   pStateMachineData = &pTapiPpdData->StateMachineData;

   if (*pMode == IFX_TAPI_LINE_FEED_PHONE_DETECT)
   {
      /* Start FXS PPD state machine.*/
      if (pStateMachineData->fSmStarted == IFX_TRUE)
      {
         /* State machine is already started. Nothing to do. */
         /* unlock PPD data */
         TAPI_OS_MutexRelease (&pTapiPpdData->hSemId);
         return TAPI_statusOk;
      }
      TRACE(TAPI_DRV, DBG_LEVEL_LOW,
           ("TAPI PPD, %s:%d ch%d: Start state machine\n",
            pChannel->pTapiDevice->pDevDrvCtx->drvName,
            pChannel->pTapiDevice->nDev,
            pChannel->nChannel));
      ifx_tapi_ppd_Reset(pChannel);
      ret = ifx_tapi_ppd_StartSM(pChannel);
      if (!TAPI_SUCCESS(ret))
      {
         /* unlock PPD data */
         TAPI_OS_MutexRelease (&pTapiPpdData->hSemId);
         return ret;
      }
   }
   else
   {
      /* line mode other than IFX_TAPI_FEED_PHONE_DETECT - stop the FXS PPD
         state machine. */
      pStateMachineData->nIntEvent = TAPI_PPD_IE_API_CHANGE_FEEDING;
      ret = ifx_tapi_ppd_HandleState(pChannel);
   }
   /* unlock PPD data */
   TAPI_OS_MutexRelease (&pTapiPpdData->hSemId);

   return ret;
}

/**
   Stops FXS Phone Detection state machine and sets line mode to
   IFX_TAPI_LINE_FEED_STANDBY

   \param pChannel      Pointer to TAPI_CHANNEL structure.

   \return
     - TAPI_statusOk: if successful
     - IFX_ERROR: in case of an error

   \remark
   This function should be used before ringing or CID transmission are started.
*/
IFX_int32_t IFX_TAPI_PPD_ServiceStart(TAPI_CHANNEL *pChannel)
{
   TAPI_PPD_STATE_MACHINE_t *pStateMachineData = IFX_NULL;
   TAPI_PPD_DATA_t *pTapiPpdData = IFX_NULL;
   IFX_int32_t ret = TAPI_statusOk;

   TAPI_ASSERT (pChannel);
   pTapiPpdData = pChannel->pTapiPpdData;
   if (pTapiPpdData == IFX_NULL)
   {
      /* The FXS Phone Detection state machine is not initialized
         for this channel. */
      return TAPI_statusOk;
   }
   pStateMachineData = &pTapiPpdData->StateMachineData;
   /* lock PPD data */
   TAPI_OS_MutexGet (&pTapiPpdData->hSemId);
   if (pTapiPpdData->fIsEnabled == IFX_FALSE)
   {
      /* Phone Detection functionality is disabled - nothing to do. */
      TAPI_OS_MutexRelease (&pTapiPpdData->hSemId);
      return TAPI_statusOk;
   }

   if (pStateMachineData->fSmStarted == IFX_TRUE)
   {
      pStateMachineData->fPrevStartStatus = IFX_TRUE;
      pStateMachineData->nIntEvent = TAPI_PPD_IE_SERVICE;
      ret = ifx_tapi_ppd_HandleState(pChannel);
   }
   else
   {
      pStateMachineData->fPrevStartStatus = IFX_FALSE;
   }
   /* unlock PPD data */
   TAPI_OS_MutexRelease (&pTapiPpdData->hSemId);
   return ret;
}

/**
   Restore previous status of the FXS Phone Detection state machine.

   \param pChannel      Pointer to TAPI_CHANNEL structure.

   \return
     - TAPI_statusOk: if successful
     - IFX_ERROR: in case of an error

   \remark
   This function should be used when ringing or CID transmission are stopped.
*/
IFX_int32_t IFX_TAPI_PPD_ServiceStop(TAPI_CHANNEL *pChannel)
{
   TAPI_PPD_STATE_MACHINE_t *pStateMachineData = IFX_NULL;
   TAPI_PPD_DATA_t *pTapiPpdData = IFX_NULL;
   IFX_int32_t ret = TAPI_statusOk;

   TAPI_ASSERT (pChannel);
   pTapiPpdData = pChannel->pTapiPpdData;
   if (pTapiPpdData == IFX_NULL)
   {
      /* The FXS Phone Detection state machine is not initialized
         for this channel. */
      return TAPI_statusOk;
   }
   pStateMachineData = &pTapiPpdData->StateMachineData;
   /* lock PPD data */
   TAPI_OS_MutexGet (&pTapiPpdData->hSemId);
   if (pTapiPpdData->fIsEnabled == IFX_FALSE)
   {
      /* Phone Detection functionality is disabled - nothing to do. */
      TAPI_OS_MutexRelease (&pTapiPpdData->hSemId);
      return TAPI_statusOk;
   }
   if (pStateMachineData->fPrevStartStatus == IFX_TRUE)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_LOW,
            ("TAPI PPD, %s:%d ch%d: Start state machine\n",
             pChannel->pTapiDevice->pDevDrvCtx->drvName,
             pChannel->pTapiDevice->nDev,
             pChannel->nChannel));
      ifx_tapi_ppd_Reset(pChannel);
      ret = ifx_tapi_ppd_StartSM(pChannel);
   }
   /* unlock PPD data */
   TAPI_OS_MutexRelease (&pTapiPpdData->hSemId);
   return ret;
}

/**
   Handle events.

   \param pChannel      Pointer to TAPI_CHANNEL structure.
   \param pEvent        Pointer to TAPI Event.

   \return
     - TAPI_statusOk: if successful
     - IFX_ERROR: in case of an error
*/
IFX_int32_t IFX_TAPI_PPD_HandleEvent(TAPI_CHANNEL *pChannel,
                                     IFX_TAPI_EVENT_t *pEvent)
{
   TAPI_PPD_DATA_t *pTapiPpdData = IFX_NULL;
   TAPI_PPD_STATE_MACHINE_t *pStateMachineData = IFX_NULL;
   IFX_int32_t ret = TAPI_statusOk;

   TAPI_ASSERT (pChannel);
   TAPI_ASSERT (pEvent);
   pTapiPpdData = pChannel->pTapiPpdData;
   if (pTapiPpdData == IFX_NULL)
   {
      /* The FXS Phone Detection state machine is not initialized
         for this channel. */
      return TAPI_statusOk;
   }
   pStateMachineData = &pTapiPpdData->StateMachineData;
   /* lock PPD data */
   TAPI_OS_MutexGet (&pTapiPpdData->hSemId);
   if (pTapiPpdData->fIsEnabled == IFX_FALSE)
   {
      /* Phone Detection functionality is disabled - nothing to do.*/
      TAPI_OS_MutexRelease (&pTapiPpdData->hSemId);
      return TAPI_statusOk;
   }

   switch (pEvent->id)
   {
      case IFX_TAPI_EVENT_FXS_OFFHOOK_INT:
      {
         ifx_tapi_ppd_NbChannelsNotify(pChannel, IFX_DISABLE);
         /* Mark that phone is detected. */
         ifx_tapi_ppd_UpdatePhoneDetectFlag(pChannel, IFX_TRUE);
         pStateMachineData->nIntEvent = TAPI_PPD_IE_HOOKOFF;
         break;
      }

      case IFX_TAPI_EVENT_LINE_MEASURE_CAPACITANCE_RDY:
      {
         if (!(pChannel->pTapiPpdData->nMeasFlag & TAPI_PPD_MEAS_FLAG_SM))
         {
            /* user application started C-measurement. Do not handle
               this event. */
            TAPI_OS_MutexRelease (&pTapiPpdData->hSemId);
            return TAPI_statusOk;
         }
         /* Mark that C-measurement is stopped. */
         pTapiPpdData->nMeasFlag = 0;
         pStateMachineData->nIntEvent = TAPI_PPD_IE_LINE_MEASURE_CAPACITANCE_RDY;
         memcpy((void *) &pTapiPpdData->stCapacEventData,
                (void*) &pEvent->data.lcap,
                sizeof(IFX_TAPI_EVENT_LINE_MEASURE_CAPACITANCE_t));
         break;
      }

      case IFX_TAPI_EVENT_FAULT_LINE_OVERTEMP:
      {
         /* Low level driver sets line mode to DISABLED. PPD state machine
            must be stopped without changing line mode. */
         pStateMachineData->nIntEvent = TAPI_PPD_IE_STOP;
         break;
      }

      case IFX_TAPI_EVENT_NLT_END:
      {
         /* Mark that C-measurement is stopped. */
         pTapiPpdData->nMeasFlag = 0;
         break;
      }

      default:
         /* unlock PPD data */
         TAPI_OS_MutexRelease (&pTapiPpdData->hSemId);
         return TAPI_statusOk;
   }

   ret = ifx_tapi_ppd_HandleState(pChannel);
   /* unlock PPD data */
   TAPI_OS_MutexRelease (&pTapiPpdData->hSemId);
   return ret;
}

/**
   Reads out the current PPD State Machine parameters

   \param pChannel      Pointer to TAPI_CHANNEL structure.
   \param pChannel      Pointer to IFX_TAPI_LINE_PHONE_DETECT_CFG_t structure.

   \return
     - TAPI_statusOk: if successful
     - IFX_ERROR: in case of an error
*/
IFX_int32_t IFX_TAPI_PPD_Cfg_Get(TAPI_CHANNEL *pChannel,
                                  IFX_TAPI_LINE_PHONE_DETECT_CFG_t *pPpdConf)
{
   TAPI_PPD_DATA_t *pTapiPpdData = IFX_NULL;

   TAPI_ASSERT (pChannel);
   TAPI_ASSERT (pPpdConf);
   pTapiPpdData = pChannel->pTapiPpdData;
   if (pTapiPpdData == IFX_NULL)
   {
      /* The FXS Phone Detection state machine is not initialized
         for this channel. */
      RETURN_STATUS (TAPI_statusInvalidCh, 0);
   }

   /* lock PPD data */
   TAPI_OS_MutexGet (&pTapiPpdData->hSemId);
   pPpdConf->nCapacitance = pTapiPpdData->nCapacThreshold;
   pPpdConf->nFindPeriod = pTapiPpdData->TimerData.nTimeoutsTable[TAPI_PPD_T2_IDX];
   pPpdConf->nLostPeriod = pTapiPpdData->TimerData.nTimeoutsTable[TAPI_PPD_T1_IDX]/1000;
   pPpdConf->nOffHookTime = pTapiPpdData->TimerData.nTimeoutsTable[TAPI_PPD_T3_IDX];;

   /* unlock PPD data */
   TAPI_OS_MutexRelease (&pTapiPpdData->hSemId);
   return TAPI_statusOk;
}

/**
   Configures the PPD State Machine parameters

   \param pChannel      Pointer to TAPI_CHANNEL structure.
   \param pChannel      Pointer to IFX_TAPI_LINE_PHONE_DETECT_CFG_t structure.

   \return
     - TAPI_statusOk: if successful
     - IFX_ERROR: in case of an error
*/
IFX_int32_t IFX_TAPI_PPD_Cfg_Set(TAPI_CHANNEL *pChannel,
                               IFX_TAPI_LINE_PHONE_DETECT_CFG_t const *pPpdConf)
{
   TAPI_PPD_DATA_t *pTapiPpdData = IFX_NULL;
   IFX_int32_t ret = TAPI_statusOk;

   TAPI_ASSERT (pChannel);
   TAPI_ASSERT (pPpdConf);
   pTapiPpdData = pChannel->pTapiPpdData;
   if (pTapiPpdData == IFX_NULL)
   {
      /* The FXS Phone Detection state machine is not initialized
         for this channel. */
      RETURN_STATUS (TAPI_statusInvalidCh, 0);
   }

   /* lock PPD data */
   TAPI_OS_MutexGet (&pTapiPpdData->hSemId);
   pTapiPpdData->nCapacThreshold = pPpdConf->nCapacitance;
   pTapiPpdData->TimerData.nTimeoutsTable[TAPI_PPD_T2_IDX] = pPpdConf->nFindPeriod;
   pTapiPpdData->TimerData.nTimeoutsTable[TAPI_PPD_T1_IDX] = pPpdConf->nLostPeriod * 1000;
   pTapiPpdData->TimerData.nTimeoutsTable[TAPI_PPD_T3_IDX]= pPpdConf->nOffHookTime;
   /* Apply new settings. */
   if ((pTapiPpdData->fIsEnabled == IFX_TRUE) &&
       (pTapiPpdData->StateMachineData.fSmStarted == IFX_TRUE))
   {
      /* To apply new settings state machine must be stopped and started
         again. */
      pTapiPpdData->StateMachineData.nIntEvent = TAPI_PPD_IE_STOP;
      ret = ifx_tapi_ppd_HandleState(pChannel);
      if (TAPI_SUCCESS(ret))
      {
         TRACE(TAPI_DRV, DBG_LEVEL_LOW,
               ("TAPI PPD, %s:%d ch%d: Start state machine\n",
                pChannel->pTapiDevice->pDevDrvCtx->drvName,
                pChannel->pTapiDevice->nDev,
                pChannel->nChannel));
         ifx_tapi_ppd_Reset(pChannel);
         ret = ifx_tapi_ppd_StartSM(pChannel);
      }
   }
   /* unlock PPD data */
   TAPI_OS_MutexRelease (&pTapiPpdData->hSemId);
   return ret;
}
#endif /* TAPI_FEAT_PHONE_DETECTION */

#ifdef TAPI_FEAT_CAP_MEAS
/**
   Stop an analog line capacitance measurement. It is used by the
   IFX_TAPI_LINE_MEASURE_CAPACITANCE_STOP command.

   \param pChannel      Pointer to TAPI_CHANNEL structure.

   \return
     - TAPI_statusOk: if successful
     - IFX_ERROR: in case of an error

   \remark
   This function can't be used by the FXS Phone Detection state machine.
*/
IFX_int32_t IOCTL_TAPI_PPD_CapMeasStop(TAPI_CHANNEL *pChannel)
{
   return ifx_tapi_ppd_CapMeasStop(pChannel);
}
#endif /* TAPI_FEAT_CAP_MEAS */

#ifdef TAPI_FEAT_CAP_MEAS
/**
   Stop an analog line capacitance measurement. It is used by the
   IFX_TAPI_NLT_CAPACITANCE_STOP command.

   \param pChannel      Pointer to TAPI_CHANNEL structure.
   \param pArg          not used

   \return
     - TAPI_statusOk: if successful
     - IFX_ERROR: in case of an error

   \remark
   This function can't be used by the FXS Phone Detection state machine.
*/
IFX_int32_t IOCTL_TAPI_PPD_NLTCapMeasStop(TAPI_CHANNEL *pChannel,
                                          IFX_TAPI_NLT_CAPACITANCE_STOP_t *pArg)
{
   IFX_UNUSED(pArg);

   return ifx_tapi_ppd_CapMeasStop(pChannel);
}
#endif /* TAPI_FEAT_CAP_MEAS */

#ifdef TAPI_FEAT_CAP_MEAS
/**
   Start an analog line capacitance measurement. It is used by the
   IFX_TAPI_LINE_MEASURE_CAPACITANCE_START command.
   PPD State Machine is stopped.

   \param pChannel      Pointer to TAPI_CHANNEL structure.

   \return
     - TAPI_statusOk: if successful
     - IFX_ERROR: in case of an error
*/
IFX_int32_t IOCTL_TAPI_PPD_CapMeasStart(TAPI_CHANNEL *pChannel)
{
   return ifx_tapi_ppd_CapMeasStart(pChannel, IFX_TRUE);
}
#endif /* TAPI_FEAT_CAP_MEAS */

#ifdef TAPI_FEAT_CAP_MEAS
/**
   Start an analog line capacitance measurement. It is used by the
   IFX_TAPI_NLT_CAPACITANCE_START command.
   PPD State Machine is stopped.

   \param pChannel      Pointer to TAPI_CHANNEL structure.
   \param pArg          not used

   \return
     - TAPI_statusOk: if successful
     - IFX_ERROR: in case of an error
*/
IFX_int32_t IOCTL_TAPI_PPD_NLTCapMeasStart(TAPI_CHANNEL *pChannel,
                                         IFX_TAPI_NLT_CAPACITANCE_START_t *pArg)
{
   IFX_UNUSED(pArg);

   return ifx_tapi_ppd_CapMeasStart(pChannel, IFX_FALSE);
}
#endif /* TAPI_FEAT_CAP_MEAS */

#ifdef TAPI_FEAT_PHONE_DETECTION
/**
   Checks who started the capacitance measurement. If user application
   didn't start it, event with measurement result should not be generated.

   \param pChannel      Pointer to TAPI_CHANNEL structure.

   \return
     - IFX_TRUE - event should be disabled
     - IFX_FALSE - event shouldn't be disabled
*/
IFX_boolean_t IFX_TAPI_PPD_DisableEvent(TAPI_CHANNEL *pChannel)
{
   TAPI_ASSERT (pChannel);

   if (pChannel->pTapiPpdData == IFX_NULL)
   {
      return IFX_FALSE;
   }
   if (pChannel->pTapiPpdData->nMeasFlag & TAPI_PPD_MEAS_FLAG_API)
   {
      return IFX_FALSE;
   }
   /* Capacitance measurement was not started by the user - disable event
      generation. */
   return IFX_TRUE;
}

/**
   Enable Phone Detection functionality on channel.

   \param pChannel      Pointer to TAPI_CHANNEL structure.

   \return
     - TAPI_statusOk: if successful
     - IFX_ERROR: in case of an error

   \remark
   This function doesn't start the PPD State Machine. To start it,
   line mode must be set to IFX_TAPI_FEED_PHONE_DETECT as a next step.
*/
IFX_int32_t IFX_TAPI_PPD_EnPhoneDetOnCh(TAPI_CHANNEL *pChannel)
{
   TAPI_PPD_DATA_t *pTapiPpdData = IFX_NULL;
   IFX_int32_t ret = TAPI_statusOk;

   TAPI_ASSERT (pChannel);
   pTapiPpdData = pChannel->pTapiPpdData;
   if (pTapiPpdData == IFX_NULL)
   {
      /* The FXS Phone Detection state machine is not initialized
         for this channel. */
      RETURN_STATUS (TAPI_statusInvalidCh, 0);
   }

   /* lock PPD data */
   TAPI_OS_MutexGet (&pTapiPpdData->hSemId);

   /* Mark that functionality is enabled. */
   pTapiPpdData->fIsEnabled = IFX_TRUE;
   TRACE(TAPI_DRV, DBG_LEVEL_LOW,
        ("TAPI PPD, Enable Phone Detection on %s:%d ch%d\n",
         pChannel->pTapiDevice->pDevDrvCtx->drvName,
         pChannel->pTapiDevice->nDev,
         pChannel->nChannel));
   TAPI_OS_MutexRelease (&pTapiPpdData->hSemId);
   return ret;
}

/**
   Disable Phone Detection functionality on channel.

   \param pChannel      Pointer to TAPI_CHANNEL structure.

   \return
     - TAPI_statusOk: if successful
     - IFX_ERROR: in case of an error

*/
IFX_int32_t IFX_TAPI_PPD_DisPhoneDetOnCh(TAPI_CHANNEL *pChannel)
{
   TAPI_PPD_DATA_t *pTapiPpdData = IFX_NULL;
   IFX_int32_t ret = TAPI_statusOk;

   TAPI_ASSERT (pChannel);
   pTapiPpdData = pChannel->pTapiPpdData;
   if (pTapiPpdData == IFX_NULL)
   {
      /* The FXS Phone Detection state machine is not initialized
         for this channel. */
      RETURN_STATUS (TAPI_statusInvalidCh, 0);
   }

   /* lock PPD data */
   TAPI_OS_MutexGet (&pTapiPpdData->hSemId);
   if (pTapiPpdData->fIsEnabled == IFX_FALSE)
   {
      /* Phone Detection functionality is already disabled -
         nothing to do. */
      TAPI_OS_MutexRelease (&pTapiPpdData->hSemId);
      return TAPI_statusOk;
   }

   /* If State Machine is started - stop it and set line feeding to STANDBY.
      If State Machine is stopped, e.g. active call - do nothing with state
      machine and line feeding.  */
   pTapiPpdData->StateMachineData.nIntEvent = TAPI_PPD_IE_SERVICE;
   ret = ifx_tapi_ppd_HandleState(pChannel);

   /* Mark that Phone Detection functionality is disabled. */
   pTapiPpdData->fIsEnabled = IFX_FALSE;
   TRACE(TAPI_DRV, DBG_LEVEL_LOW,
        ("TAPI PPD, Disable Phone Detection on %s:%d ch%d\n",
         pChannel->pTapiDevice->pDevDrvCtx->drvName,
         pChannel->pTapiDevice->nDev,
         pChannel->nChannel));
   TAPI_OS_MutexRelease (&pTapiPpdData->hSemId);
   return ret;
}

#ifdef TAPI_FEAT_PHONE_DETECTION_PROCFS
/**
   Reads the FXS Phone Detection state machine information.

   \param s
   \param pChannel      Pointer to TAPI_CHANNEL structure.
   \param nDev          device number
*/
IFX_void_t IFX_TAPI_PPD_proc_read(struct seq_file *s, TAPI_CHANNEL *pChannel,
   IFX_uint32_t nDev)
{
   TAPI_PPD_DATA_t *pTapiPpdData = IFX_NULL;
   TAPI_PPD_STATE_MACHINE_t *pSmData = IFX_NULL;
   TAPI_PPD_TIMER_DATA_t *pTimerData = IFX_NULL;
   IFX_char_t tempBuf[30];

   TAPI_ASSERT (s);
   TAPI_ASSERT (pChannel);

   pTapiPpdData = pChannel->pTapiPpdData;
   if (pTapiPpdData == IFX_NULL)
   {
      /* The FXS Phone Detection state machine is not initialized
         for this channel. */
      return;
   }
   /* lock PPD data */
   TAPI_OS_MutexGet (&pTapiPpdData->hSemId);
   pSmData = &pTapiPpdData->StateMachineData;
   pTimerData = &pTapiPpdData->TimerData;
   /* Device */
   memset (tempBuf, 0, sizeof(tempBuf));
   snprintf(tempBuf, sizeof(tempBuf)-1,
            "%s:%d", pChannel->pTapiDevice->pDevDrvCtx->drvName, nDev);
   seq_printf(s, "%10s|", tempBuf);
   /* Channel number. */
   seq_printf(s, "%3d|", pChannel->nChannel);
   /* status - Phone detection Functionality Disabled/Enabled */
   seq_printf(s, "%6s|",
                 (pTapiPpdData->fIsEnabled == IFX_TRUE)? "en":"dis");
   /* State machine status. */
   seq_printf(s, "%4s|",
                 (pTapiPpdData->fIsEnabled == IFX_TRUE) ?
                 ((pSmData->fSmStarted == IFX_TRUE)? "run":"stop") : "");
   /* Phone detection status. */
   seq_printf(s, "%5s|",
                 ((pTapiPpdData->fIsEnabled == IFX_TRUE) &&
                  (pSmData->fSmStarted == IFX_TRUE))?
                 ((pTapiPpdData->fPhoneDetected == IFX_TRUE)? "yes":"no") : "");
   /* Current state. */
   seq_printf(s, "%18s|",
                 ((pTapiPpdData->fIsEnabled == IFX_TRUE) &&
                  (pSmData->fSmStarted == IFX_TRUE))?
                  ifx_tapi_ppd_Enum2Name(pSmData->nState, rgStateName):"");
   /* Active timer */
   memset (tempBuf, 0, sizeof(tempBuf));
   if (pTimerData->fTimerStarted == IFX_TRUE)
   {
      snprintf(tempBuf, sizeof(tempBuf)-1,
               "T%d", (pTimerData->nActiveTimeout + 1));
   }
   seq_printf(s, "%5s|", tempBuf);
   /* Last capacity */
   memset (tempBuf, 0, sizeof(tempBuf));
   if (pTapiPpdData->stCapacEventData.nReturnCode == IFX_SUCCESS)
   {
      snprintf(tempBuf, sizeof(tempBuf)-1,
               "%d", pTapiPpdData->stCapacEventData.nCapacitance);
   }
   else
   {
      snprintf(tempBuf, sizeof(tempBuf)-1,
               "err code: 0x%X", pTapiPpdData->stCapacEventData.nReturnCode);
   }
   seq_printf(s, "%21s|", tempBuf);
   /* Capacity threshold */
   seq_printf(s, "%6d|", pTapiPpdData->nCapacThreshold);
   /* T1 timeout */
   seq_printf(s, "%6d|", pTimerData->nTimeoutsTable[TAPI_PPD_T1_IDX]/1000);
   /* T2 timeout */
   seq_printf(s, "%6d|", pTimerData->nTimeoutsTable[TAPI_PPD_T2_IDX]);
   /* T3 timeout */
   seq_printf(s, "%6d|\n", pTimerData->nTimeoutsTable[TAPI_PPD_T3_IDX]);

   /* unlock PPD data */
   TAPI_OS_MutexRelease (&pTapiPpdData->hSemId);
}
#endif /* TAPI_FEAT_PHONE_DETECTION_PROCFS */

#endif /* TAPI_FEAT_PHONE_DETECTION */

/**
   Enable Phone Detection functionality on device.

   \param pTapiDev      Pointer to TAPI_DEV structure.

   \return
     - TAPI_statusOk: if successful
     - IFX_ERROR: in case of an error

*/
IFX_int32_t IFX_TAPI_PPD_EnPhoneDet(TAPI_DEV *pTapiDev)
{
   IFX_int32_t ret = TAPI_statusOk;
#ifdef TAPI_FEAT_PHONE_DETECTION
   TAPI_CHANNEL *pChannel = IFX_NULL;
   TAPI_PPD_DATA_t *pTapiPpdData = IFX_NULL;
   IFX_int32_t i;

   TAPI_ASSERT (pTapiDev);
   /* for the moment two channels are enabled until the real number of channels
      is known internally. */
   for (i = 0; i < pTapiDev->nMaxChannel; i++)
   {
      pChannel = pTapiDev->pChannel + i;
      pTapiPpdData = pChannel->pTapiPpdData;

      if (pTapiPpdData == IFX_NULL)
      {
         continue;
      }
      ret = IFX_TAPI_PPD_EnPhoneDetOnCh(pChannel);
      if (!TAPI_SUCCESS(ret))
      {
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
              ("\nDRV_ERROR, TAPI PPD: enabling of Phone Detection on %s:%d "
               "ch %d failed.\n",
               pChannel->pTapiDevice->pDevDrvCtx->drvName,
               pChannel->pTapiDevice->nDev,
               pChannel->nChannel));
         return ret;
      }
   }
#else
   IFX_UNUSED (pTapiDev);
#endif /* TAPI_FEAT_PHONE_DETECTION */
   return ret;
}

/**
   Disable Phone Detection functionality on device.

   \param pTapiDev      Pointer to TAPI_DEV structure.

   \return
     - TAPI_statusOk: if successful
     - IFX_ERROR: in case of an error

*/
IFX_int32_t IFX_TAPI_PPD_DisPhoneDet(TAPI_DEV *pTapiDev)
{
   IFX_int32_t ret = TAPI_statusOk;
#ifdef TAPI_FEAT_PHONE_DETECTION
   TAPI_CHANNEL *pChannel = IFX_NULL;
   TAPI_PPD_DATA_t *pTapiPpdData = IFX_NULL;
   IFX_int32_t i;

   TAPI_ASSERT (pTapiDev);
   for (i = 0; i < pTapiDev->nMaxChannel; i++)
   {
      pChannel = pTapiDev->pChannel + i;
      pTapiPpdData = pChannel->pTapiPpdData;

      if (pTapiPpdData == IFX_NULL)
      {
         continue;
      }
      ret = IFX_TAPI_PPD_DisPhoneDetOnCh(pChannel);
      if (!TAPI_SUCCESS(ret))
      {
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
              ("\nDRV_ERROR, TAPI PPD: disabling of Phone Detection on %s:%d "
               "ch %d failed.\n",
               pChannel->pTapiDevice->pDevDrvCtx->drvName,
               pChannel->pTapiDevice->nDev,
               pChannel->nChannel));
         return ret;
      }
   }
#else
   IFX_UNUSED (pTapiDev);
#endif /* TAPI_FEAT_PHONE_DETECTION */
   return ret;
}


#ifdef TAPI_FEAT_PHONE_DETECTION
/**
   Switch the phone detection on the neighbouring channels on or off.

   This function finds from the given channel all neighbouring channels and
   switches the phone detection on these according to the parameter.

   \param  pChannel     Pointer to TAPI_CHANNEL structure.
   \param  nAction      -IFX_ENABLE to turn phone detection on
                        -IFX_DISABLE to turn phone detection off
   \return
   - TAPI_statusOk: if successful
*/
static void ifx_tapi_ppd_NbChannelsNotify(
                        TAPI_CHANNEL *pChannel,
                        IFX_operation_t nAction)
{
   TAPI_DEV *pTapiDev;
   IFX_int32_t i;

   TAPI_ASSERT (pChannel);
   pTapiDev = pChannel->pTapiDevice;

   /* If this channel indicates enable check also the other channels. */
   if (nAction == IFX_ENABLE)
   {
      /* nAction is updated before the loop where it is checked. */
      nAction =
         (ifx_tapi_ppd_NbChannelsCheckActive(pChannel) == IFX_FALSE) ?
         IFX_ENABLE : IFX_DISABLE;
   }

   for (i = 0; i < pTapiDev->nMaxChannel; i++)
   {
      TAPI_CHANNEL *pNbChannel = pTapiDev->pChannel + i;
      TAPI_PPD_DATA_t *pTapiNbPpdData = pNbChannel->pTapiPpdData;

      if (pNbChannel == pChannel)
      {
         /* Skip the channel from which this is called. */
         continue;
      }
      if (pTapiNbPpdData == IFX_NULL)
      {
         /* Skip neighbour channels without phone detection state machine. */
         continue;
      }

      if (nAction == IFX_ENABLE)
      {
         /* Mark the neighbour as inactive => turn on phone detection. */
         pTapiNbPpdData->StateMachineData.nIntEvent = TAPI_PPD_IE_NB_INACTIVE;
         ifx_tapi_ppd_HandleState(pNbChannel);
      }
      else
      {
         /* Mark the neighbour as active => turn off phone detection. */
         pTapiNbPpdData->StateMachineData.nIntEvent = TAPI_PPD_IE_NB_ACTIVE;
         ifx_tapi_ppd_HandleState(pNbChannel);
      }
   }
}


/**
   Check if neighbouring channels allow to run phone detection on this channel.

   Phone detection must be stopped or suspended while any neighbouring channel
   is in an active call, so for any linemode other than disable, standby
   or detect.
   This function finds from the given channel all neighbouring channels and
   returns if any is active or all are inactive.

   \param  pChannel     Pointer to TAPI_CHANNEL structure.
   \param  nAction      -IFX_ENABLE to turn phone detection on.
                        -IFX_DISABLE to turn phone detection off.
   \return
   - IFX_true: Some neighbouring channel is active.
   - IFX_true: No neighbouring channel is active.
*/
static IFX_boolean_t ifx_tapi_ppd_NbChannelsCheckActive(
                        TAPI_CHANNEL *pChannel)
{
   TAPI_DEV *pTapiDev;
   IFX_int32_t nDevice;
   IFX_boolean_t bActive = IFX_FALSE;

   TAPI_ASSERT (pChannel);
   pTapiDev = pChannel->pTapiDevice;

   for (nDevice = 0; nDevice < pTapiDev->nMaxChannel; nDevice++)
   {
      TAPI_CHANNEL *pNbChannel = pTapiDev->pChannel + nDevice;
      IFX_uint8_t nLineMode;

      if (pNbChannel->pTapiPpdData == IFX_NULL)
      {
         /* Skip channels whithout phone detection. */
         continue;
      }
      if (pNbChannel == pChannel)
      {
         /* Skip the channel from which this is called. */
         continue;
      }

      TAPI_Phone_Linefeed_Get(pNbChannel, &nLineMode);

      if ( !( (nLineMode == IFX_TAPI_LINE_FEED_DISABLED) ||
              (nLineMode == IFX_TAPI_LINE_FEED_STANDBY) ||
              (nLineMode == IFX_TAPI_LINE_FEED_PHONE_DETECT)))
      {
         /* Channel is active. Stop search and return immediately. */
         bActive = IFX_TRUE;
         break;
      }
   }

   return bActive;
}
#endif /* TAPI_FEAT_PHONE_DETECTION */


#ifdef TAPI_FEAT_POWER
/**
   Callback used to enable/disable the power features.

   \param  pmcuPwrStateEna    IFX_PMCU_PWR_STATE_ON - enable the power feature
                              IFX_PMCU_PWR_STATE_OFF - disable the power feature.

   \return
   - IFX_PMCU_RETURN_SUCCESS Power feature switched successfully.
   - IFX_PMCU_RETURN_ERROR   Error occured during the switch of power feature.
*/
static IFX_PMCU_RETURN_t ifx_tapi_pmc_pwr_feature_switch(
                                       IFX_PMCU_PWR_STATE_ENA_t pmcuPwrStateEna)
{
   IFX_int32_t ret;
   IFX_TAPI_DRV_CTX_t *pDrvCtx = IFX_NULL;
   TAPI_DEV *pTapiDev = IFX_NULL;
   IFX_uint16_t i, j;

   /* Loop over all registered LL-drivers */
   for (i = 0; i < TAPI_MAX_LL_DRIVERS; i++)
   {
      pDrvCtx = gHLDrvCtx[i].pDrvCtx;
      if (pDrvCtx == IFX_NULL)
         continue; /* next driver */

      /* set pTapiDev to point to the first element of an array of all
         TAPI devices of this driver's context */
      pTapiDev = pDrvCtx->pTapiDev;

      for (j=0; j < pDrvCtx->maxDevs; j++)
      {
         pTapiDev = &(pDrvCtx->pTapiDev[j]);

         if (pTapiDev->bInitialized != IFX_TRUE)
         {
            continue; /* next device */
         }

         switch (pmcuPwrStateEna)
         {
            case IFX_PMCU_PWR_STATE_ON:
               ret = IFX_TAPI_PPD_EnPhoneDet(pTapiDev);
               if (!TAPI_SUCCESS(ret))
                  return IFX_PMCU_RETURN_ERROR;
               break;
            case IFX_PMCU_PWR_STATE_OFF:
               ret = IFX_TAPI_PPD_DisPhoneDet(pTapiDev);
               if (!TAPI_SUCCESS(ret))
                  return IFX_PMCU_RETURN_ERROR;
               break;
            default:
               TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
                    ("DRV_ERROR, Invalid argument to switch the power feature "
                    "(pmcuPwrStateEna: %d).\n", pmcuPwrStateEna));
               return IFX_PMCU_RETURN_ERROR;
         } /* switch */
      } /* for devs */
   } /* for drvs */

   /* Remember the power state. It is necesssary to call it in this place
      to have it also in situation when devices are not initialized. */
   switch (pmcuPwrStateEna)
   {
      case IFX_PMCU_PWR_STATE_ON:
         fPmcu_FeatureEnabled = IFX_TRUE;
         break;
      case IFX_PMCU_PWR_STATE_OFF:
         fPmcu_FeatureEnabled = IFX_FALSE;
         break;
      default:
         TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
              ("DRV_ERROR, Invalid argument to switch the power feature "
              "(pmcuPwrStateEna: %d).\n", pmcuPwrStateEna));
         return IFX_PMCU_RETURN_ERROR;
   } /* switch */

   return IFX_PMCU_RETURN_SUCCESS;
}

#ifdef TAPI_FEAT_PMCU_IF
/**
   Dummy function for callbacks used before and after module's power state.

   \param  pmcuModule   Module
   \param  newState     New state
   \param  oldState     Old state

   \return
      - IFX_PMCU_RETURN_NOACTIVITY - always.
*/
static IFX_PMCU_RETURN_t tapi_pmc_pre_post(IFX_PMCU_MODULE_t pmcuModule,
                                            IFX_PMCU_STATE_t newState,
                                            IFX_PMCU_STATE_t oldState)
{
   return IFX_PMCU_RETURN_NOACTIVITY;
}
#endif /* TAPI_FEAT_PMCU_IF */

/**
   Dummy function for callbacks used to get module's power state.

   \param  pmcuState    Pointer to return power state.

   \return
      - IFX_PMCU_RETURN_SUCCESS - always.
*/
static IFX_PMCU_RETURN_t tapi_pmc_state_get(IFX_PMCU_STATE_t *pmcuState)
{
   *pmcuState = IFX_PMCU_STATE_D0;
   return IFX_PMCU_RETURN_SUCCESS;
}

/**
   Initialise Power management control.

   Create data structure in the device and register with the PMCU driver.

   \return
     - TAPI_statusOk: if successful
     - IFX_ERROR: in case of an error
*/
IFX_int32_t IFX_TAPI_PMC_Init(void)
{
#ifdef TAPI_FEAT_PMCU_IF
   IFX_PMCU_REGISTER_t pmcuRegister;
   IFX_PMCU_RETURN_t ret;

   /* Register with the PMCU driver */
   memset (&pmcuRegister, 0, sizeof(IFX_PMCU_REGISTER_t));
   pmcuRegister.pmcuModule = IFX_PMCU_MODULE_VE;
   /* TAPI has module number 0. Low Level VE modules like vmmc will start
      from 1. */
   pmcuRegister.pmcuModuleNr = 0;
   pmcuRegister.post = tapi_pmc_pre_post;
   pmcuRegister.pre = tapi_pmc_pre_post;
   pmcuRegister.ifx_pmcu_state_get = tapi_pmc_state_get;
   pmcuRegister.ifx_pmcu_pwr_feature_switch = ifx_tapi_pmc_pwr_feature_switch;
   ret = ifx_pmcu_register ( &pmcuRegister );
   if (ret != IFX_PMCU_RETURN_SUCCESS)
   {
      TRACE(TAPI_DRV, DBG_LEVEL_HIGH,
           ("\nDRV_ERROR, TAPI registration to PMCU failed.\n"));
      /* errmsg: Registration to PMCU failed. */
      return TAPI_statusRegisterToPmcuFailed;
   }
   TRACE(TAPI_DRV, DBG_LEVEL_LOW,
           ("TAPI successfully registered to the PMCU.\n"));
#endif /* TAPI_FEAT_PMCU_IF */

#ifdef TAPI_FEAT_CPUFREQ_IF
   struct ltq_cpufreq* ppd_cpufreq_p;
   ppd_cpufreq_p = ltq_cpufreq_get();
   if(ppd_cpufreq_p != NULL){
      list_add_tail(&ppd_cpufreq_feature_pds.list,
                    &ppd_cpufreq_p->list_head_module);
   }
#endif /* TAPI_FEAT_CPUFREQ_IF */

   fPmcu_FeatureEnabled = IFX_TRUE;

   return TAPI_statusOk;
}

/**
   Close down Power management control.

   Unregister from the PMCU driver and free data struct in the device.
*/
IFX_void_t IFX_TAPI_PMC_Exit(void)
{
#ifdef TAPI_FEAT_PMCU_IF
   IFX_PMCU_REGISTER_t pmcuRegister;

   memset (&pmcuRegister, 0, sizeof(pmcuRegister));
   pmcuRegister.pmcuModule = IFX_PMCU_MODULE_VE;
   pmcuRegister.pmcuModuleNr = 0;
   ifx_pmcu_unregister ( &pmcuRegister );
#endif /* TAPI_FEAT_PMCU_IF */

#ifdef TAPI_FEAT_CPUFREQ_IF
   list_del(&ppd_cpufreq_feature_pds.list);
#endif /* TAPI_FEAT_CPUFREQ_IF */
}
#endif /* TAPI_FEAT_POWER */
