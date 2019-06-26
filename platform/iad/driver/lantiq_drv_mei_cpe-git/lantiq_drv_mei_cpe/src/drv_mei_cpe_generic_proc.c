/******************************************************************************

                          Copyright (c) 2007-2015
                     Lantiq Beteiligungs-GmbH & Co. KG

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/* ============================================================================
   Description : VRX Driver, proc file system replacement, generic part
   ========================================================================= */

/* ============================================================================
   Inlcudes
   ========================================================================= */

/* get at first the driver configuration */
#include "drv_mei_cpe_config.h"

#if !defined(LINUX) && defined(MEI_GENERIC_PROC_FS)

#include <stdio.h>

#include "ifx_types.h"
#include "drv_mei_cpe_os.h"
#include "drv_mei_cpe_dbg.h"

#include "drv_mei_cpe_dbg_driver.h"

/* project specific headers */
#include "drv_mei_cpe_interface.h"
#include "drv_mei_cpe_api.h"

#include "drv_mei_cpe_generic_proc.h"
#include "drv_mei_cpe_dbg_access.h"


#ifdef MEI_STATIC
#undef MEI_STATIC
#endif

#ifdef MEI_DEBUG
#define MEI_STATIC
#else
#define MEI_STATIC   static
#endif

/* ============================================================================
   Macros
   ========================================================================= */

#define MEI_CONFIG_PROC_COM_TRACE      't'
#define MEI_CONFIG_PROC_COM_TRACE_LOG  'T'
#define MEI_CONFIG_PROC_MEI_TRACE      'm'
#define MEI_CONFIG_PROC_MEI_TRACE_LOG  'M'
#define MEI_CONFIG_PROC_ROM_TRACE      'r'
#define MEI_CONFIG_PROC_ROM_TRACE_LOG  'R'

#define MEI_CONFIG_PROC_BOOT_DWIDTH    'D'
#define MEI_CONFIG_PROC_BOOT_DWIDTH_l  'd'
#define MEI_CONFIG_PROC_BOOT_WSTATES   'W'
#define MEI_CONFIG_PROC_BOOT_WSTATES_l 'w'
#define MEI_CONFIG_PROC_BLK_TOUT       'B'
#define MEI_CONFIG_PROC_BLK_TOUT_l     'b'


#if (MEI_SUPPORT_VDSL2_ADSL_SWAP == 1)
#define MEI_CONFIG_PROC_FW_MODE_SEL    'f'
#endif

#if (MEI_MSG_DUMP_ENABLE == 1)
#define MEI_CONFIG_PROC_MSG_DUMP_CNTRL     'c'
#define MEI_CONFIG_PROC_MSG_DUMP_WE_ID     'C'
#define MEI_CONFIG_PROC_MSG_DUMP_TRACE     's'
#define MEI_CONFIG_PROC_MSG_DUMP_TRACE_LOG 'S'
#endif

#define MEI_CONFIG_PROC_LAST           '\0'


/* ============================================================================
   Local variables
   ========================================================================= */

static struct MEI_CONFIG_PROC_s {char shortCut; char *pHelpStr; } VRXConfigProc[] =
{
   {MEI_CONFIG_PROC_COM_TRACE,      "Com Trace Level"},
   {MEI_CONFIG_PROC_COM_TRACE_LOG,  "Com Log Level"},
   {MEI_CONFIG_PROC_MEI_TRACE,      "MEI Trace/Log Level"},
#if (MEI_SUPPORT_ROM_CODE == 1)
   {MEI_CONFIG_PROC_ROM_TRACE,      "ROM Trace Level"},
   {MEI_CONFIG_PROC_ROM_TRACE_LOG,  "ROM Log Level"},
#endif

#if (MEI_SUPPORT_VDSL2_ADSL_SWAP == 1)
   {MEI_CONFIG_PROC_FW_MODE_SEL,    "FW Mode Select"},
#endif

   {MEI_CONFIG_PROC_BLK_TOUT,       "Block Time Out"},

#if (MEI_MSG_DUMP_ENABLE == 1)
   {MEI_CONFIG_PROC_MSG_DUMP_CNTRL,     "Msg Dump Cntrl"},
   {MEI_CONFIG_PROC_MSG_DUMP_WE_ID,     "MSG Dump WE ID"},
   {MEI_CONFIG_PROC_MSG_DUMP_TRACE,     "Msg Dump Trace Level"},
   {MEI_CONFIG_PROC_MSG_DUMP_TRACE_LOG, "Msg Dump Log Level"},
#endif

   {MEI_CONFIG_PROC_LAST,           NULL}
};


/* ============================================================================
   Local functions - declaration
   ========================================================================= */
MEI_STATIC IFX_void_t MEI_ConfigProc(FILE *streamOut, IFX_char_t shortCut, IFX_int32_t procIndex, IFX_int32_t setVal);


/* ============================================================================
   Local functions - definition
   ========================================================================= */
/**
   Set/Show a proc entry
*/
MEI_STATIC IFX_void_t MEI_ConfigProc(FILE *streamOut, IFX_char_t shortCut, IFX_int32_t procIndex, IFX_int32_t setVal)
{
   switch(shortCut)
   {
      case MEI_CONFIG_PROC_COM_TRACE:
         if (setVal == -1)
         {
            /* Common Trace/Log */
            fprintf( streamOut, " %c = %d (%s)" MEI_DRV_CRLF,
                     VRXConfigProc[procIndex].shortCut,
                     MEI_DRV_PRN_USR_LEVEL_GET(MEI_DRV),
                     VRXConfigProc[procIndex].pHelpStr);
         }
         else
         {
            MEI_DRV_PRN_USR_LEVEL_SET(MEI_DRV, setVal);
         }
         break;

      case MEI_CONFIG_PROC_COM_TRACE_LOG:
         if (setVal == -1)
         {
            /* Common Trace/Log */
            fprintf( streamOut, " %c = %d (%s)" MEI_DRV_CRLF,
                     VRXConfigProc[procIndex].shortCut,
                     MEI_DRV_PRN_INT_LEVEL_GET(MEI_DRV),
                     VRXConfigProc[procIndex].pHelpStr);
         }
         else
         {
            MEI_DRV_PRN_INT_LEVEL_SET(MEI_DRV, setVal);
         }
         break;

      case MEI_CONFIG_PROC_MEI_TRACE:
      case MEI_CONFIG_PROC_MEI_TRACE_LOG:
         if (setVal == -1)
         {
            /* MEI Trace/Log */
            fprintf( streamOut, " %c = %d (%s)" MEI_DRV_CRLF,
                     VRXConfigProc[procIndex].shortCut,
                     MEI_DRV_PRN_USR_LEVEL_GET(MEI_MEI_ACCESS),
                     VRXConfigProc[procIndex].pHelpStr);
         }
         else
         {
            MEI_DRV_PRN_USR_LEVEL_SET(MEI_MEI_ACCESS, setVal);
         }

#if (MEI_SUPPORT_ROM_CODE == 1)
      case MEI_CONFIG_PROC_ROM_TRACE  :
         if (setVal == -1)
         {
            /* ROM Trace/Log */
            fprintf( streamOut, " %c = %d (%s)" MEI_DRV_CRLF,
                     VRXConfigProc[procIndex].shortCut,
                     MEI_DRV_PRN_USR_LEVEL_GET(MEI_ROM),
                     VRXConfigProc[procIndex].pHelpStr);
         }
         else
         {
            MEI_DRV_PRN_USR_LEVEL_SET(MEI_ROM, setVal);
         }
         break;

      case MEI_CONFIG_PROC_ROM_TRACE_LOG:
         if (setVal == -1)
         {
            /* ROM Trace/Log */
            fprintf( streamOut, " %c = %d (%s)" MEI_DRV_CRLF,
                     VRXConfigProc[procIndex].shortCut,
                     MEI_DRV_PRN_INT_LEVEL_GET(MEI_ROM),
                     VRXConfigProc[procIndex].pHelpStr);
         }
         else
         {
            MEI_DRV_PRN_INT_LEVEL_SET(MEI_ROM, setVal);
         }
         break;
#endif

#if (MEI_SUPPORT_VDSL2_ADSL_SWAP == 1)
      case MEI_CONFIG_PROC_FW_MODE_SEL:
         if (setVal == -1)
         {
            /* FW Mode Selection (VDSL2 / ADSL) */
            fprintf( streamOut, " %c = 0x%X (%s)" MEI_DRV_CRLF,
                     VRXConfigProc[procIndex].shortCut,
                     MEI_fwModeSelect,
                     VRXConfigProc[procIndex].pHelpStr);
         }
         else
         {
            MEI_fwModeSelect = (IFX_uint32_t)(setVal);
         }
         break;
#endif
      case MEI_CONFIG_PROC_BLK_TOUT  :
      case MEI_CONFIG_PROC_BLK_TOUT_l:
         if (setVal == -1)
         {
            /* Block Time Out */
            fprintf( streamOut, " %c = %d (%s)" MEI_DRV_CRLF,
                     VRXConfigProc[procIndex].shortCut,
                     MEI_BlockTimeout,
                     VRXConfigProc[procIndex].pHelpStr);
         }
         else
         {
            MEI_BlockTimeout = setVal;
         }
         break;

#if ( (MEI_MSG_DUMP_ENABLE == 1) && (MEI_DEBUG_PRINT == 1) )
      case MEI_CONFIG_PROC_MSG_DUMP_CNTRL:
         if (setVal == -1)
         {
            IFX_uint32_t tmpVal = ( MEI_msgDumpEnable | (MEI_msgDumpSetLabel ? 0x00000080 : 0x0) |
                                  (MEI_msgDumpOutCntrl << 8) |
                                  ( ((IFX_uint32_t)MEI_msgDumpLine) << 16) );

            /* Msg Dump Control */
            fprintf( streamOut, " %c = 0x%08X (%s)" MEI_DRV_CRLF,
                     VRXConfigProc[procIndex].shortCut,
                     tmpVal,
                     VRXConfigProc[procIndex].pHelpStr);
         }
         else
         {
            IFX_uint32_t tmpVal = (IFX_uint32_t)setVal;

            MEI_msgDumpEnable   = tmpVal & 0x0000007F;
            MEI_msgDumpSetLabel = (tmpVal & 0x00000080) ? IFX_TRUE : IFX_FALSE;
            MEI_msgDumpOutCntrl = (tmpVal & 0x0000FF00) >> 8;
            MEI_msgDumpLine     = (IFX_uint16_t)((tmpVal & 0xFFFF0000) >> 16);
         }
         break;

      case MEI_CONFIG_PROC_MSG_DUMP_WE_ID:
         if (setVal == -1)
         {
            /* Block Time Out */
            fprintf( streamOut, " %c = %d (%s)" MEI_DRV_CRLF,
                     VRXConfigProc[procIndex].shortCut,
                     MEI_msgDumpId,
                     VRXConfigProc[procIndex].pHelpStr);
         }
         else
         {
            MEI_msgDumpId = (IFX_int16_t)setVal;
         }
         break;

      case MEI_CONFIG_PROC_MSG_DUMP_TRACE:
         if (setVal == -1)
         {
            /* Common Trace/Log */
            fprintf( streamOut, " %c = %d (%s)" MEI_DRV_CRLF,
                     VRXConfigProc[procIndex].shortCut,
                     MEI_DRV_PRN_USR_LEVEL_GET(MEI_MSG_DUMP),
                     VRXConfigProc[procIndex].pHelpStr);
         }
         else
         {
            MEI_DRV_PRN_USR_LEVEL_SET(MEI_MSG_DUMP, setVal);
         }
         break;

      case MEI_CONFIG_PROC_MSG_DUMP_TRACE_LOG:
         if (setVal == -1)
         {
            /* Common Trace/Log */
            fprintf( streamOut, " %c = %d (%s)" MEI_DRV_CRLF,
                     VRXConfigProc[procIndex].shortCut,
                     MEI_DRV_PRN_INT_LEVEL_GET(MEI_MSG_DUMP),
                     VRXConfigProc[procIndex].pHelpStr);
         }
         else
         {
            MEI_DRV_PRN_INT_LEVEL_SET(MEI_MSG_DUMP, setVal);
         }
         break;
#endif

      default:
         fprintf( streamOut, "Error - unknown proc config shortCut[%d]" MEI_DRV_CRLF, shortCut);
         fprintf( streamOut, "Try one of this: 'T', 'R', 'M' ... " MEI_DRV_CRLF);
   }

   return;
}


/* ============================================================================
   Global functions - definition
   ========================================================================= */

/**
   Show proc config settings.
*/
IFX_void_t MEI_ShowConfigProc(FILE *streamOut)
{
   IFX_int32_t procIndex = 0;

   fprintf(streamOut, MEI_DRV_CRLF "VRX VRX proc config" MEI_DRV_CRLF MEI_DRV_CRLF);

   while (1)
   {
      if (VRXConfigProc[procIndex].pHelpStr == NULL)
         break;

      MEI_ConfigProc(streamOut, VRXConfigProc[procIndex].shortCut, procIndex, -1);
      procIndex++;
   }

   return;
}

/**
   Set proc config settings.
*/
IFX_void_t MEI_SetConfigProc(FILE *streamOut, IFX_char_t shortCut, IFX_int32_t setVal)
{
   MEI_ConfigProc(streamOut, shortCut, 0, setVal);

   MEI_ShowConfigProc(streamOut);

   return;
}


/**
   Read the version information from the driver.

\return
   none
*/
IFX_void_t MEI_ShowVersionProc(FILE *streamOut)
{

   fprintf(streamOut, "%s" MEI_DRV_CRLF, &MEI_WHATVERSION[4]);
   return;
}


/**
   Read the status information from the driver.

\return
  none
*/
IFX_void_t MEI_ShowStatusProc(FILE *streamOut)
{
   int i, j;

   for (i=0; i<MEI_DFEX_ENTITIES; i++)
   {
      fprintf(streamOut, "********************************" MEI_DRV_CRLF);
      fprintf(streamOut, "pDFEX(%d) = 0x%08X" MEI_DRV_CRLF, i, (int)MEIX_Cntrl[i]);
      fprintf(streamOut, "++++++++++++++++++++++++++++++++" MEI_DRV_CRLF);
      if (MEIX_Cntrl[i] != NULL)
      {
         fprintf(streamOut, "IRQ Count = %d" MEI_DRV_CRLF, MEIX_Cntrl[i]->IRQ_Count);

         for (j=0; j<MEI_DFE_INSTANCE_PER_ENTITY; j++)
         {
            fprintf(streamOut, "pDFECh[%d] = 0x%08X" MEI_DRV_CRLF,
                    (i*MEI_DFE_INSTANCE_PER_ENTITY) +j,
                    (int)MEIX_Cntrl[i]->MeiDevice[j]);
            fprintf(streamOut, "--------------------------------" MEI_DRV_CRLF);

            if (MEIX_Cntrl[i]->MeiDevice[j] != NULL)
            {
               fprintf( streamOut, "Hw Vers    = %5d\tMEI State   = %s" MEI_DRV_CRLF,
                        MEIX_Cntrl[i]->MeiDevice[j]->modemData.hwVersion,
                        (MEI_DRV_MEI_IF_STATE_GET(MEIX_Cntrl[i]->MeiDevice[j]))?
                        ( (MEI_DRV_MEI_IF_STATE_GET(MEIX_Cntrl[i]->MeiDevice[j]) == e_MEI_MEI_HW_STATE_UP)?
                        "UP":"DOWN") : "unknown");

               fprintf( streamOut, "bOpen      = %5d\t,Drv State  = %5d\tModemFSM = %d" MEI_DRV_CRLF,
                        MEIX_Cntrl[i]->MeiDevice[j]->openCount,
                        MEI_DRV_STATE_GET(MEIX_Cntrl[i]->MeiDevice[j]),
                        MEI_DRV_MODEM_STATE_GET(MEIX_Cntrl[i]->MeiDevice[j]) );

#if (MEI_SUPPORT_STATISTICS == 1)
               fprintf( streamOut, "DrvSwRst   = %5d\tMeiHwRst    = %5d" MEI_DRV_CRLF,
                        MEIX_Cntrl[i]->MeiDevice[j]->statistics.drvSwRstCount,
                        MEIX_Cntrl[i]->MeiDevice[j]->statistics.meiHwRstCount);
               fprintf( streamOut, "GP1 Int    = %5d\tMsgAv Int   = %5d" MEI_DRV_CRLF,
                        MEIX_Cntrl[i]->MeiDevice[j]->statistics.dfeGp1IntCount,
                        MEIX_Cntrl[i]->MeiDevice[j]->statistics.dfeMsgAvIntCount);
               fprintf( streamOut, "FwDownl    = %5d\tCodeSwap    = %5d" MEI_DRV_CRLF,
                        MEIX_Cntrl[i]->MeiDevice[j]->statistics.fwDlCount,
                        MEIX_Cntrl[i]->MeiDevice[j]->statistics.dfeCodeSwapCount);
               fprintf( streamOut, "FwDownlErr = %5d" MEI_DRV_CRLF,
                        MEIX_Cntrl[i]->MeiDevice[j]->statistics.fwDlErrCount);

#if ((MEI_SUPPORT_TIME_TRACE == 1) && (MEI_SUPPORT_ROM_CODE == 1) && (MEI_SUPPORT_DL_DMA_CS == 1))
               fprintf( streamOut, "FwDownlErr = %5d\tCS max Time = %5d [ms]" MEI_DRV_CRLF,
                        MEIX_Cntrl[i]->MeiDevice[j]->statistics.fwDlErrCount,
                        MEIX_Cntrl[i]->MeiDevice[j]->timeStat.processCsMax_ms);

#else
               fprintf( streamOut, "FwDownlErr = %5d" MEI_DRV_CRLF,
                        MEIX_Cntrl[i]->MeiDevice[j]->statistics.fwDlErrCount);
#endif

#if (MEI_SUPPORT_OPTIMIZED_FW_DL == 1)
               fprintf( streamOuts, "FwDownlOpt = %5d" MEI_DRV_CRLF,
                              pMeiDev->statistics.fwDlOptSuccessCount);
               fprintf( streamOuts, "FwDOptFail = %5d" MEI_DRV_CRLF,
                              pMeiDev->statistics.fwDlOptFailedCount);
#endif
               fprintf( streamOut, "TxMsg      = %5d\tRxAck       = %5d" MEI_DRV_CRLF,
                        MEIX_Cntrl[i]->MeiDevice[j]->statistics.sendMsgCount,
                        MEIX_Cntrl[i]->MeiDevice[j]->statistics.recvAckCount);
               fprintf( streamOut, "RxMsg      = %5d\tRxMsgDisc   = %5d" MEI_DRV_CRLF,
                        MEIX_Cntrl[i]->MeiDevice[j]->statistics.recvMsgCount,
                        MEIX_Cntrl[i]->MeiDevice[j]->statistics.recvMsgDiscardCount);
               fprintf( streamOut, "RxMsgErr   = %5d\tTxMsgErr    = %5d" MEI_DRV_CRLF,
                        MEIX_Cntrl[i]->MeiDevice[j]->statistics.recvMsgErrCount,
                        MEIX_Cntrl[i]->MeiDevice[j]->statistics.errorCount);
               fprintf( streamOut, "Nfc        = %5d\tNfcDisc     = %5d" MEI_DRV_CRLF,
                        MEIX_Cntrl[i]->MeiDevice[j]->statistics.recvNfcCount,
                        MEIX_Cntrl[i]->MeiDevice[j]->statistics.recvNfcDiscardCount);
               fprintf( streamOut, "NfcDist    = %5d\tNfcDistDisc = %5d" MEI_DRV_CRLF MEI_DRV_CRLF,
                        MEIX_Cntrl[i]->MeiDevice[j]->statistics.recvNfcDistCount,
                        MEIX_Cntrl[i]->MeiDevice[j]->statistics.recvNfcDistDiscardCount);

#endif   /* #if (MEI_SUPPORT_STATISTICS == 1) */

#if (MEI_SUPPORT_TIME_TRACE == 1)
               fprintf( streamOut, "WSendMin   = %5d\tWSendMax    = %5d [ms]" MEI_DRV_CRLF,
                        MEIX_Cntrl[i]->MeiDevice[j]->timeStat.waitSendMin_ms,
                        MEIX_Cntrl[i]->MeiDevice[j]->timeStat.waitSendMax_ms);
               fprintf( streamOut, "WAckMin    = %5d\tWAckMax     = %5d [ms]" MEI_DRV_CRLF,
                        MEIX_Cntrl[i]->MeiDevice[j]->timeStat.waitAckMin_ms,
                        MEIX_Cntrl[i]->MeiDevice[j]->timeStat.waitAckMax_ms);
#endif   /* #if (MEI_SUPPORT_TIME_TRACE == 1) */

            }
         }        /* for ( ; j<MEI_DFE_INSTANCE_PER_ENTITY; ) {...} */
      }        /* if (MEIX_Cntrl[i] != NULL) {...} */
   }        /* for ( ; i<MEI_MAX_DFEX_ENTITIES; ) {...} */

   return;
}


/**
   Read the status information from the driver.

\return
  none
*/
IFX_void_t MEI_ShowNfcProc(FILE *streamOut)
{
#if (MEI_SUPPORT_DRV_NFC_DEBUG == 1)
   int i, j, len = 0;

   char nfcDisplay[MEI_NFC_DISPLAY_BUFFER_SIZE];

   for (i=0; i<MEI_DFEX_ENTITIES; i++)
   {
      fprintf(streamOut, "********************************" MEI_DRV_CRLF);
      fprintf(streamOut, "pDFEX(%d) = 0x%08X" MEI_DRV_CRLF, i, (int)MEIX_Cntrl[i]);
      fprintf(streamOut, "++++++++++++++++++++++++++++++++" MEI_DRV_CRLF);
      if (MEIX_Cntrl[i] != NULL)
      {
         for (j=0; j<MEI_DFE_INSTANCE_PER_ENTITY; j++)
         {
            nfcDisplay[0] = '\0';
            len = 0;
            if (MEIX_Cntrl[i]->MeiDevice[j] != NULL)
            {
               len += MEI_ShowNfcData( MEIX_Cntrl[i]->MeiDevice[j],
                                         nfcDisplay, MEI_NFC_DISPLAY_BUFFER_SIZE);

               fprintf(streamOut, "%s" MEI_DRV_CRLF, nfcDisplay);
            }
         }        /* for ( ; j<MEI_DFE_INSTANCE_PER_ENTITY; ) {...} */
      }        /* if (MEIX_Cntrl[i] != NULL) {...} */
   }        /* for ( ; i<MEI_MAX_DFEX_ENTITIES; ) {...} */
#else
   fprintf(streamOut, "MEI: NFC proc not enabled" MEI_DRV_CRLF);
#endif
   return;
}




/* ============================================================================
   Proc-FS User Access
   ========================================================================= */

/*
   Proc FS - Commands
*/
#define MEI_GENERIC_TEST_FS_HELP      0
#define MEI_GENERIC_PROC_FS_VERSION   101
#define MEI_GENERIC_PROC_FS_STATUS    102
#define MEI_GENERIC_PROC_FS_SHOW      103
#define MEI_GENERIC_PROC_FS_SET       104
#define MEI_GENERIC_PROC_FS_NFC       105
#define MEI_MEI_TEST_FS_INFO      999


/*
   Contains the command and help info for the Proc-Fs
*/
struct MEI_proc_fs_access_table_s { int cmd; char *pHelpStr; } pVrxProcFsAccessTable[] =
{
   {MEI_GENERIC_TEST_FS_HELP,    "Print ProcFs help info"},
   {MEI_MEI_TEST_FS_INFO,    " "},
   {MEI_GENERIC_PROC_FS_VERSION, "Proc Get Version    Args - none"},
   {MEI_GENERIC_PROC_FS_STATUS,  "Proc Get Status     Args - none"},
   {MEI_GENERIC_PROC_FS_SHOW,    "Proc Show Config    Args - none"},
   {MEI_GENERIC_PROC_FS_SET,     "Proc Set Config     Args - 0: Entry, 1: New Value"},
   {MEI_GENERIC_PROC_FS_NFC,     "Proc Get NFC        Args - none"},
   {-1, NULL}
};



/**
   Print Proc FS Help info

\param
   streamOut   points to the output stream


*/
static int doVrxProcFsHelp_fd(FILE *streamOut)
{
   int testIndex = 0;

   if(streamOut == NULL)
   {
      streamOut = stdout;
   }

   printf(MEI_DRV_CRLF "VRX driver - Proc FS" MEI_DRV_CRLF MEI_DRV_CRLF);

   while (1)
   {
      if (pVrxProcFsAccessTable[testIndex].cmd == -1)
         break;

      if (pVrxProcFsAccessTable[testIndex].cmd != MEI_MEI_TEST_FS_INFO)
         fprintf( streamOut,
                  "%3d  %s" MEI_DRV_CRLF,
                  pVrxProcFsAccessTable[testIndex].cmd,
                  pVrxProcFsAccessTable[testIndex].pHelpStr);
      else
         fprintf( streamOut,
                  "     %s" MEI_DRV_CRLF,
                  pVxProcFsAccessTable[testIndex].pHelpStr);

      testIndex++;
   }

   fprintf(streamOut,
      MEI_DRV_CRLF "Usage (Proc FS access):" MEI_DRV_CRLF
      "\tdoVrxProcFs <cmd>, <proc entry>, [<arg 0>, <arg 1>]" MEI_DRV_CRLF MEI_DRV_CRLF);

   fprintf(streamOut,
      "Example (Proc FS access):" MEI_DRV_CRLF
      "\tdoVrxProcFs 104, 't', 1  --> change trace level" MEI_DRV_CRLF MEI_DRV_CRLF);

   return 0;
}



/**
   Access the Proc Filesystem.
*/
int doVrxProcFs_fd(
                   FILE *streamOut,
                   int cmd, int procEntry, int param0, int param1, int param2)
{
   if(streamOut == NULL)
   {
      streamOut = stdout;
   }

   if ( (cmd == 0) || (cmd == -1) )
   {
      doVrxProcFsHelp_fd(streamOut);
      return 0;
   }

   /* direct commands - no device open required */
   switch(cmd)
   {
      case MEI_GENERIC_PROC_FS_VERSION:
         MEI_ShowVersionProc(streamOut);
         break;

      case MEI_GENERIC_PROC_FS_STATUS:
         MEI_ShowStatusProc(streamOut);
         break;

      case MEI_GENERIC_PROC_FS_SHOW:
         MEI_ShowConfigProc(streamOut);
         break;

      case MEI_GENERIC_PROC_FS_SET:
         MEI_SetConfigProc(streamOut, (IFX_char_t)procEntry, (IFX_int32_t)param0);
         break;

      case MEI_GENERIC_PROC_FS_NFC:
         MEI_ShowNfcProc(streamOut);
         break;

      default:
         break;
   }

   switch(cmd)
   {

      case MEI_GENERIC_PROC_FS_VERSION:
      case MEI_GENERIC_PROC_FS_STATUS:
      case MEI_GENERIC_PROC_FS_SHOW:
      case MEI_GENERIC_PROC_FS_SET:
      case MEI_GENERIC_PROC_FS_NFC:
         fprintf( streamOut,
                  "Return = %d" MEI_DRV_CRLF, 0);
         return 0;
      default:
         fprintf( streamOut,
                  "ProcFs: Unkown Command %d" MEI_DRV_CRLF, cmd);

         fprintf( streamOut,
                  "Return = %d" MEI_DRV_CRLF, -1);
         return -1;
   }

   return 0;
}


/**
   Wrapper Proc FS.
   - use default stdout stream.
*/
int doVrxProcFs(int cmd, int procEntry, int param0, int param1, int param2)
{
   return doVrxProcFs_fd(stdout, cmd, procEntry, param0, param1, param2);
}


#endif      /* #if !defined(LINUX) && defined(MEI_GENERIC_PROC_FS) */


