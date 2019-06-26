#ifndef _DRV_MEI_CPE_LINUX_PROC_CONFIG_H
#define _DRV_MEI_CPE_LINUX_PROC_CONFIG_H
/******************************************************************************

                          Copyright (c) 2007-2015
                     Lantiq Beteiligungs-GmbH & Co. KG

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/* ==========================================================================
   Description : Linux proc fs config read write part
   ========================================================================== */

#ifdef LINUX

#ifdef __cplusplus
extern "C"
{
#endif


/* ==========================================================================
   Includes
   ========================================================================== */
/* get the driver configuration */
#include "drv_mei_cpe_config.h"


#if CONFIG_PROC_FS
/* ==========================================================================
   procfs configuration definitons
   ========================================================================== */

#if (MEI_DEBUG_PRINT == 1)
#define MEI_PROCFS_CONFIG_G_DBG_CONFIG_ENTRY \
            {"GDL", MEI_ProcReadConfigGlobalDbgConfig, MEI_ProcWriteConfigGlobalDbgConfig}
#define MEI_PROCFS_CONFIG_G_DBG_CONFIG \
            GET_FIELD_2(PROCFS_CONFIG_G_DBG_CONFIG, MEI_PROCFS_CONFIG_G_DBG_CONFIG_ENTRY, SEPARATOR)
#else
#define MEI_PROCFS_CONFIG_G_DBG_CONFIG
#endif

#define MEI_PROCFS_CONFIG_LOG_ENTRY \
            {"LOG", MEI_ProcReadConfigLog, MEI_ProcWriteConfigLog}
#define MEI_PROCFS_CONFIG_LOG \
            GET_FIELD_2(PROCFS_CONFIG_LOG, MEI_PROCFS_CONFIG_LOG_ENTRY, SEPARATOR)

#define MEI_PROCFS_CONFIG_TRACE_ENTRY \
            {"TRACE", MEI_ProcReadConfigTrace, MEI_ProcWriteConfigTrace}
#define MEI_PROCFS_CONFIG_TRACE \
            GET_FIELD_2(PROCFS_CONFIG_TRACE, MEI_PROCFS_CONFIG_TRACE_ENTRY, SEPARATOR)

#define MEI_PROCFS_CONFIG_LOG_MEI_ENTRY \
            {"L_MEI", MEI_ProcReadConfigLogMei, MEI_ProcWriteConfigLogMei}
#define MEI_PROCFS_CONFIG_LOG_MEI \
            GET_FIELD_2(PROCFS_CONFIG_LOG_MEI, MEI_PROCFS_CONFIG_LOG_MEI_ENTRY, SEPARATOR)

#define MEI_PROCFS_CONFIG_TRACE_MEI_ENTRY \
            {"T_MEI", MEI_ProcReadConfigTraceMei, MEI_ProcWriteConfigTraceMei}
#define MEI_PROCFS_CONFIG_TRACE_MEI \
            GET_FIELD_2(PROCFS_CONFIG_TRACE_MEI, MEI_PROCFS_CONFIG_TRACE_MEI_ENTRY, SEPARATOR)

#define MEI_PROCFS_CONFIG_MB_ME2ARC_ENTRY \
            {"MB_ME2ARC", MEI_ProcReadConfigMailboxME2ARC, MEI_ProcWriteConfigMailboxME2ARC}
#define MEI_PROCFS_CONFIG_MB_ME2ARC \
            GET_FIELD_2(PROCFS_CONFIG_MB_ME2ARC, MEI_PROCFS_CONFIG_MB_ME2ARC_ENTRY, SEPARATOR)

#define MEI_PROCFS_CONFIG_MB_ARC2ME_ENTRY \
            {"MB_ARC2ME", MEI_ProcReadConfigMailboxARC2ME, MEI_ProcWriteConfigMailboxARC2ME}
#define MEI_PROCFS_CONFIG_MB_ARC2ME \
            GET_FIELD_2(PROCFS_CONFIG_MB_ARC2ME, MEI_PROCFS_CONFIG_MB_ARC2ME_ENTRY, SEPARATOR)

#if (MEI_SUPPORT_ROM_CODE == 1)

#define MEI_PROCFS_CONFIG_TRACE_DL_ROM_ENTRY \
            {"T_ROM", MEI_ProcReadConfigTraceRom, MEI_ProcWriteConfigTraceRom}
#define MEI_PROCFS_CONFIG_TRACE_DL_ROM \
            GET_FIELD_2(PROCFS_CONFIG_TRACE_DL_ROM, MEI_PROCFS_CONFIG_TRACE_DL_ROM_ENTRY, SEPARATOR)
#else

#define MEI_PROCFS_CONFIG_TRACE_DL_ROM

#endif


#define MEI_PROCFS_CONFIG_RAM_DATA_WIDTH
#define MEI_PROCFS_CONFIG_WAITSTATES


#define MEI_PROCFS_CONFIG_MAX_WAIT_RDY_FOR_DL
#define MEI_PROCFS_CONFIG_MAX_WAIT_ACK_NEXT_BLOCK
#define MEI_PROCFS_CONFIG_MAX_WAIT_DL_START

#define MEI_PROCFS_CONFIG_BLOCK_TOUT_ENTRY \
            {"DL_BLOCK_TOUT", MEI_ProcReadConfigBlockTimeout, MEI_ProcWriteConfigBlockTimeout}
#define MEI_PROCFS_CONFIG_BLOCK_TOUT \
            GET_FIELD_2(PROCFS_CONFIG_BLOCK_TOUT, MEI_PROCFS_CONFIG_BLOCK_TOUT_ENTRY, SEPARATOR)

#define MEI_PROCFS_CONFIG_MAX_WAIT_MODEM_ONLINE_ENTRY \
            {"W_MODEM_ONLINE", MEI_ProcReadConfigMaxWaitModemOnline, MEI_ProcWriteConfigMaxWaitModemOnline}
#define MEI_PROCFS_CONFIG_MAX_WAIT_MODEM_ONLINE \
            GET_FIELD_2(PROCFS_CONFIG_W_MODEM_ONLINE, MEI_PROCFS_CONFIG_MAX_WAIT_MODEM_ONLINE_ENTRY, SEPARATOR)

#define MEI_PROCFS_CONFIG_W_DFE_RESP_ENTRY \
            {"W_DFE_RESP_MS", MEI_ProcReadConfigMaxWaitDfeResp, MEI_ProcWriteConfigMaxWaitDfeResp}
#define MEI_PROCFS_CONFIG_W_DFE_RESP \
            GET_FIELD_2(PROCFS_CONFIG_W_DFE_RESP, MEI_PROCFS_CONFIG_W_DFE_RESP_ENTRY, SEPARATOR)

#define MEI_PROCFS_CONFIG_BOOTMODE

#if (MEI_SUPPORT_VDSL2_ADSL_SWAP == 1)

#define MEI_PROCFS_CONFIG_FW_SELECT_ENTRY \
            {"FW_SEL", MEI_ProcReadConfigFwSelect, MEI_ProcWriteConfigFwSelect}
#define MEI_PROCFS_CONFIG_FW_SELECT \
            GET_FIELD_2(PROCFS_CONFIG_FW_SELECT, MEI_PROCFS_CONFIG_FW_SELECT_ENTRY, SEPARATOR)

#else

#define MEI_PROCFS_CONFIG_FW_SELECT

#endif      /* #if (MEI_SUPPORT_VDSL2_ADSL_SWAP == 1) */



#define MEI_PROCFS_CONFIG_FSM_SET_PRE_ACT_ENTRY \
            {"FSM_PRE", MEI_ProcReadConfigFsmSetPreAct, MEI_ProcWriteConfigFsmSetPreAct}
#define MEI_PROCFS_CONFIG_FSM_SET_PRE_ACT \
            GET_FIELD_2(PROCFS_CONFIG_FSM_SET_PRE_ACT, MEI_PROCFS_CONFIG_FSM_SET_PRE_ACT_ENTRY, SEPARATOR)

#if ( (MEI_MSG_DUMP_ENABLE == 1) && (MEI_DEBUG_PRINT == 1) )

#define MEI_PROCFS_CONFIG_MDMP_ENABLE_ENTRY \
            {"MDMP_ENABLE", MEI_ProcReadConfigMsgDumpEnable, MEI_ProcWriteConfigMsgDumpEnable}
#define MEI_PROCFS_CONFIG_MDMP_ENABLE \
            GET_FIELD_2(PROCFS_CONFIG_MDMP_ENABLE, MEI_PROCFS_CONFIG_MDMP_ENABLE_ENTRY, SEPARATOR)

#define MEI_PROCFS_CONFIG_MDMP_ID_ENTRY \
            {"MDMP_ID", MEI_ProcReadConfigMsgDumpId, MEI_ProcWriteConfigMsgDumpId}
#define MEI_PROCFS_CONFIG_MDMP_ID \
            GET_FIELD_2(PROCFS_CONFIG_MDMP_ID, MEI_PROCFS_CONFIG_MDMP_ID_ENTRY, SEPARATOR)

#else

#define MEI_PROCFS_CONFIG_MDMP_ENABLE
#define MEI_PROCFS_CONFIG_MDMP_ID

#endif


#if (MEI_EXT_MEI_ACCESS_ADD_CSE_MIPS == 1) || (MEI_EXT_MEI_ACCESS_ADD_CSE == 1)

#define MEI_PROCFS_CONFIG_MEI_ACCESS_ADD_CSE_ENTRY \
            {"MEI_ADD_CSE", MEI_ProcReadConfigMeiAccCse, MEI_ProcWriteConfigMeiAccCse}
#define MEI_PROCFS_CONFIG_MEI_ACCESS_ADD_CSE \
            GET_FIELD_2(PROCFS_CONFIG_MEI_ACCESS_ADD_CSE, MEI_PROCFS_CONFIG_MEI_ACCESS_ADD_CSE_ENTRY, SEPARATOR)
#else
#define MEI_PROCFS_CONFIG_MEI_ACCESS_ADD_CSE
#endif


#if (MEI_SUPPORT_DRV_MODEM_TESTS == 1)

#define MEI_PROCFS_CONFIG_DTEST_CNTRL_ENTRY \
            {"DTEST_CNTRL", MEI_ProcReadConfigDTestCntrl, MEI_ProcWriteConfigDTestCntrl}
#define MEI_PROCFS_CONFIG_DTEST_CNTRL \
            GET_FIELD_2(PROCFS_CONFIG_DTEST_CNTRL, MEI_PROCFS_CONFIG_DTEST_CNTRL_ENTRY, SEPARATOR)
#else

#define MEI_PROCFS_CONFIG_DTEST_CNTRL
#endif

#define MEI_PROCFS_CONFIG_DBG_FLAGS_ENTRY \
            {"DBG_FLAGS", MEI_ProcReadConfigDebugFlags, MEI_ProcWriteConfigDebugFlags}
#define MEI_PROCFS_CONFIG_DBG_FLAGS \
            GET_FIELD_2(PROCFS_CONFIG_DBG_FLAGS, MEI_PROCFS_CONFIG_DBG_FLAGS_ENTRY, SEPARATOR)
            
#if (MEI_SUPPORT_DEBUG_LOGGER == 1)
#define MEI_PROCFS_CONFIG_DBG_LOGGER_ENTRY \
            {"DBG_LOGGER", MEI_ProcReadConfigDebugLogger, MEI_ProcWriteConfigDebugLogger}
#define MEI_PROCFS_CONFIG_DBG_LOGGER \
            GET_FIELD_2(PROCFS_CONFIG_DBG_LOGGER, MEI_PROCFS_CONFIG_DBG_LOGGER_ENTRY, SEPARATOR)            
#endif /* (MEI_SUPPORT_DEBUG_LOGGER == 1) */

#define MEI_PROCFS_CONFIG_LAST_ENTRY \
            {"LAST", NULL, NULL}
#define MEI_PROCFS_CONFIG_LAST \
            GET_FIELD_2(PROCFS_CONFIG_LAST, MEI_PROCFS_CONFIG_LAST_ENTRY, NO_SEPARATOR)




/* ==========================================================================
   types and enums
   ========================================================================== */

/* Test cases:
                        enum   | description | test function
*/
#if (MEI_SUPPORT_DEBUG_LOGGER == 1)
#define MEI_PROCFS_CONFIG_TABLE \
                     MEI_PROCFS_CONFIG_LOG \
                     MEI_PROCFS_CONFIG_TRACE \
                     MEI_PROCFS_CONFIG_LOG_MEI \
                     MEI_PROCFS_CONFIG_TRACE_MEI \
                     MEI_PROCFS_CONFIG_TRACE_DL_ROM \
                     MEI_PROCFS_CONFIG_MB_ME2ARC \
                     MEI_PROCFS_CONFIG_MB_ARC2ME \
                     MEI_PROCFS_CONFIG_RAM_DATA_WIDTH \
                     MEI_PROCFS_CONFIG_WAITSTATES \
                     MEI_PROCFS_CONFIG_MAX_WAIT_RDY_FOR_DL \
                     MEI_PROCFS_CONFIG_MAX_WAIT_ACK_NEXT_BLOCK \
                     MEI_PROCFS_CONFIG_MAX_WAIT_DL_START \
                     MEI_PROCFS_CONFIG_BOOTMODE \
                     MEI_PROCFS_CONFIG_BLOCK_TOUT \
                     MEI_PROCFS_CONFIG_MAX_WAIT_MODEM_ONLINE\
                     MEI_PROCFS_CONFIG_W_DFE_RESP \
                     MEI_PROCFS_CONFIG_FW_SELECT \
                     MEI_PROCFS_CONFIG_FSM_SET_PRE_ACT \
                     MEI_PROCFS_CONFIG_MDMP_ENABLE \
                     MEI_PROCFS_CONFIG_MDMP_ID \
                     MEI_PROCFS_CONFIG_MEI_ACCESS_ADD_CSE \
                     MEI_PROCFS_CONFIG_DTEST_CNTRL \
                     MEI_PROCFS_CONFIG_G_DBG_CONFIG \
                     MEI_PROCFS_CONFIG_DBG_FLAGS \
                     MEI_PROCFS_CONFIG_DBG_LOGGER \
                     MEI_PROCFS_CONFIG_LAST
#else /* (MEI_SUPPORT_DEBUG_LOGGER == 1) */
#define MEI_PROCFS_CONFIG_TABLE \
                     MEI_PROCFS_CONFIG_LOG \
                     MEI_PROCFS_CONFIG_TRACE \
                     MEI_PROCFS_CONFIG_LOG_MEI \
                     MEI_PROCFS_CONFIG_TRACE_MEI \
                     MEI_PROCFS_CONFIG_TRACE_DL_ROM \
                     MEI_PROCFS_CONFIG_MB_ME2ARC \
                     MEI_PROCFS_CONFIG_MB_ARC2ME \
                     MEI_PROCFS_CONFIG_RAM_DATA_WIDTH \
                     MEI_PROCFS_CONFIG_WAITSTATES \
                     MEI_PROCFS_CONFIG_MAX_WAIT_RDY_FOR_DL \
                     MEI_PROCFS_CONFIG_MAX_WAIT_ACK_NEXT_BLOCK \
                     MEI_PROCFS_CONFIG_MAX_WAIT_DL_START \
                     MEI_PROCFS_CONFIG_BOOTMODE \
                     MEI_PROCFS_CONFIG_BLOCK_TOUT \
                     MEI_PROCFS_CONFIG_MAX_WAIT_MODEM_ONLINE\
                     MEI_PROCFS_CONFIG_W_DFE_RESP \
                     MEI_PROCFS_CONFIG_FW_SELECT \
                     MEI_PROCFS_CONFIG_FSM_SET_PRE_ACT \
                     MEI_PROCFS_CONFIG_MDMP_ENABLE \
                     MEI_PROCFS_CONFIG_MDMP_ID \
                     MEI_PROCFS_CONFIG_MEI_ACCESS_ADD_CSE \
                     MEI_PROCFS_CONFIG_DTEST_CNTRL \
                     MEI_PROCFS_CONFIG_G_DBG_CONFIG \
                     MEI_PROCFS_CONFIG_DBG_FLAGS \
                     MEI_PROCFS_CONFIG_LAST
#endif /* (MEI_SUPPORT_DEBUG_LOGGER == 1) */

/* specify no separator for the last table entry */
#ifdef NO_SEPARATOR
#undef NO_SEPARATOR
#endif
#define NO_SEPARATOR

/* separator for typedef enum is ',' */
#ifdef SEPARATOR
#undef SEPARATOR
#endif
#define SEPARATOR ,

/* prepare the macro table expansion for typedef enum */
#ifdef GET_FIELD_2
#undef GET_FIELD_2
#endif
#define GET_FIELD_2(y,z,sep) e_##y sep

/**
   Enumeration type for procfs config parameter
*/
typedef enum
{
   MEI_PROCFS_CONFIG_TABLE
} E_MEI_PROCFS_CONFIG;


/**
   Function type of the procfs write config function.
   - param: points to the argument of the corresponding write procfs call.
*/
typedef int(*MEI_PROCFS_WRITE_CONFIG_FCT)(char *);

/**
   Function type of the procfs read config function.
   - param: points to the current page location.
*/
typedef void(*MEI_PROCFS_READ_CONFIG_FCT)(struct seq_file *s);


/**
   Type of procfs config entries.
*/
typedef struct MEI_config_proc_entry_s
{
   /** corresponding config parameter name */
   char *pName;
   /** corresponding read  config function */
   MEI_PROCFS_READ_CONFIG_FCT  pRdFct;
   /** corresponding write config function */
   MEI_PROCFS_WRITE_CONFIG_FCT pWrFct;
} MEI_CONFIG_PROC_ENTRY_T;


/* ==========================================================================
   Global procfs config functions
   ========================================================================== */

extern int MEI_InstallProcEntryConfig(struct proc_dir_entry *driver_proc_node);

#endif      /* #if CONFIG_PROC_FS */

#ifdef __cplusplus
/* extern "C" */
}
#endif

#endif      /* #ifdef LINUX */

#endif      /* #ifndef _DRV_MEI_CPE_LINUX_PROC_CONFIG_H */

