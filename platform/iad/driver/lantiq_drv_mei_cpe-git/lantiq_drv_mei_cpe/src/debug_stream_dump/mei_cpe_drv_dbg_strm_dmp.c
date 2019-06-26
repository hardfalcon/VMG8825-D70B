/******************************************************************************

               Copyright (c) 2007-2015
            Lantiq Beteiligungs-GmbH & Co. KG

   For licensing information, see the file 'LICENSE' in the root folder of
   this software module.

******************************************************************************/

/* ==========================================================================
   Description : Small programm to store debug stream dump from the VRX Driver
   to the file system
   ========================================================================== */

/* ============================= */
/* Includes                      */
/* ============================= */
#include "mei_cpe_appl_osmap.h"

#ifdef LINUX

#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#define _GNU_SOURCE
#include <getopt.h>
#include <sys/time.h>
#include <time.h>

/** get interface and configuration */
#include "drv_mei_cpe_interface.h"

/** get the CMV message format */
#include "cmv_message_format.h"

/* ============================================================== */
/* Local Variables                                                */
/* ============================================================== */

#if (MEI_SUPPORT_DEVICE_VR10_320 == 1)
/* Dummy functions to avoid linker complaints */
void __aeabi_unwind_cpp_pr0(void)
{
};

void __aeabi_unwind_cpp_pr1(void)
{
};

void __aeabi_unwind_cpp_pr2(void)
{
};
#endif

/* ============================================================== */
/* Local Function Declaration                                     */
/* ============================================================== */
int MEI_debug_stream_dump_daemon(void);

/* ============================================================== */
/* Local Variables (used for arguments)                           */
/* ============================================================== */
static int bHelp              = -1;    /* h */
static int bFile              = -1;    /* f */
static int bOperationMode     = -1;    /* o */
static int bBufferSize        = -1;    /* b */
static int bStreamIdFilter    = -1;    /* s */
static int bEventIdFilter     = -1;    /* e */
static int bMaskFilter        = -1;    /* m */
static int bPatternFilter     = -1;    /* p */
static int bText              = -1;    /* t */
static int bLength            = -1;    /* l */
static int bInternalMask      = -1;    /* i */
static int bSnapshotMode      = -1;    /* T */
static int bInterval          = -1;    /* I */
static int bDfeChannel        = -1;    /* c */

static int fd = -1;

static int bRun               = 1;
static int bCheckFileExt      = 1;

#define DBG_STRM_DMP_MEI_BUFFER_SIZE_BYTES (256 * 20)
#define DBG_STRM_DMP_MEI_DBG_PREFIX        "DBG_STREAM >> "
#define DBG_STRM_DMP_MEI_DEV_PATH          "/dev/mei_cpe/"
#define DBG_STRM_DMP_MEI_DEFAULT_DFE_NUM   0
#define DBG_STRM_DMP_MEI_EVENT_NUM         0x7603 /* EVT_DBG_DEBUG_STREAM */
#define DBG_STRM_DMP_MEI_MINIMUM_FILE_SIZE 256
#define DBG_STRM_DMP_MEI_DEFAULT_FILE_SIZE 1048576 /* 1 Mbyte */
#define DBG_STRM_DMP_MEI_INT_MASK_MAX_NUM  5
#define DBG_STRM_DMP_MEI_STD_EXTENSION     ".bin"
#define DBG_STRM_DMP_MEI_SNAPSHOT_LEN_SEC  120
#define DBG_STRM_DMP_MEI_INACTIVE_MS       5000
#define DBG_STRM_DMP_MEI_BYTES_TO_FLUSH DBG_STRM_DMP_MEI_BUFFER_SIZE_BYTES / 10

static unsigned long  g_nOperationMode =
   (unsigned long)e_MEI_DBG_STREAM_DEFAULT_RING;
static unsigned int   g_nBufferSize = DBG_STRM_DMP_MEI_BUFFER_SIZE_BYTES;
static unsigned short g_nStartStreamId = 0;
static unsigned short g_nStopStreamId = 0;
static unsigned short g_nStartEventId = 0;
static unsigned short g_nStopEventId = 0;
static unsigned short g_nStartMask[MEI_DBG_STREAM_MAX_COMPARE_SIZE_IN_WORDS]
   = { 0 };
static unsigned short g_nStopMask[MEI_DBG_STREAM_MAX_COMPARE_SIZE_IN_WORDS]
   = { 0 };
static unsigned short g_nStartPattern[MEI_DBG_STREAM_MAX_COMPARE_SIZE_IN_WORDS]
   = { 0 };
static unsigned short g_nStopPattern[MEI_DBG_STREAM_MAX_COMPARE_SIZE_IN_WORDS]
   = { 0 };
static unsigned long  g_nFileLength = DBG_STRM_DMP_MEI_DEFAULT_FILE_SIZE;
static char dumpFileName[256];
static unsigned int   g_nInternalMask[DBG_STRM_DMP_MEI_INT_MASK_MAX_NUM]={ 0 };
static unsigned short g_nSnapshotLengthInSec=DBG_STRM_DMP_MEI_SNAPSHOT_LEN_SEC;
static unsigned int   g_nInterval = DBG_STRM_DMP_MEI_INACTIVE_MS;

#ifdef LINUX
   #define MEI_IOCTL_ARG unsigned long
#endif

#ifdef VXWORKS
   #define MEI_IOCTL_ARG int
#endif

/* ============================================================== */
/* Local variables for parse and interprete arguments             */
/* ============================================================== */
static void parseArgs(int argc, char *argv[]);
static void doHelp(void);

/*
   0 no_argument
   1 required_argument
   2 optional_argument
*/

static struct option long_options[] =
{
/*  0 */   {"help",      no_argument,        NULL, 'h'},
/*  1 */   {"file",      required_argument,  NULL, 'f'},
/*  2 */   {"operation", required_argument,  NULL, 'o'},
/*  3 */   {"buffer",    required_argument,  NULL, 'b'},
/*  4 */   {"stream",    required_argument,  NULL, 's'},
/*  5 */   {"event",     required_argument,  NULL, 'e'},
/*  6 */   {"mask",      required_argument,  NULL, 'm'},
/*  7 */   {"pattern",   required_argument,  NULL, 'p'},
/*  8 */   {"text",      no_argument,        NULL, 't'},
/*  9 */   {"length",    required_argument,  NULL, 'l'},
/* 10 */   {"intcfg",    required_argument,  NULL, 'i'},
/* 11 */   {"snapshot",  optional_argument,  NULL, 'T'},
/* 12 */   {"interval",  required_argument,  NULL, 'I'},
/* 13 */   {"channel",   required_argument,  NULL, 'c'},
{NULL,        0,                  NULL,  0}
};

#define GETOPT_LONG_OPTSTRING "hf:o:b:s:e:m:p:tl:i:T::I:c:"

static char description[][100] =
{
   /*  0 */   {"help screen"},
   /*  1 */   {"file name"},
   /*  2 */   {"debug stream buffer operation mode. 0 - default ring (default),"
   " 1 - fill, 2 - user ring, 3 - fifo"},
   /*  3 */   {"buffer size in bytes"},
   /*  4 */   {"stream ID to trigger to start or to stop the copy of the "
   "current log, default values are nulls"},
   /*  5 */   {"event ID to trigger to start or to stop the copy of the "
   "current log, default values are nulls"},
   /*  6 */   {"mask to trigger  to trigger to start the copy of the "
   "current log, default values are nulls"},
   /*  7 */   {"pattern to trigger to start or to stop the copy of the "
   "current log, default values are nulls"},
   /*  8 */   {"ASCII text output format is used for the output file"},
   /*  9 */   {"maximum length of a dump file, length = 1Mb is used if the "
   "option is skipped"},
   /* 10 */   {"bit mask to configure which messages are output in the debug "
   "stream (FW internal filter)"},
   /* 11 */   {"snapshot feature with optional debug stream dump length in "
   "seconds"},
   /* 12 */   {"time interval in milliseconds between two snapshots"},
   /* 13 */   {"number of DFE channel"},
   {0}
};

/* ============================================================== */
/* Local Function to parse and interprete arguments               */
/* ============================================================== */

static void MEI_debug_stream_dump_termination_handler(int sig)
{
   /* ignore the signal, we'll handle by ourself */
   signal (sig, SIG_IGN);
   if (sig == SIGINT || sig == SIGTERM)
   {
      MEIOS_Printf( DBG_STRM_DMP_MEI_DBG_PREFIX
        "Bye from the MEI Debug Stream Dump Application." MEIOS_CRLF);

      bRun = 0;
   }
}

static void parseArgs(int argc, char *argv[])
{
   int parm_no      = 0;
   char *pEndPtr    = "\0";
   unsigned short i = 0;
   char string[40]  = { 0 };
   char seps[]      = ":";
   char *token;
   unsigned int nVal = 0;

   while(1)
   {
      int option_index = 0;
      int c;

      /* 1 colon means there is a required parameter */
      /* 2 colons means there is an optional parameter */
      c = getopt_long (argc,
                       argv,
                       GETOPT_LONG_OPTSTRING,
                       long_options,
                       &option_index);
      if(c == -1)
      {
         break;
      }

      parm_no++;

      switch(c)
      {
         case 'h':
            bHelp = 1;
            break;
         case 'f':
            if (optarg)
            {
               strncpy(dumpFileName, optarg,sizeof(dumpFileName));
               dumpFileName[sizeof(dumpFileName)-1] = 0;
               bFile = 1;
            }
            break;
         case 'o':
            if (optarg)
            {
               g_nOperationMode=(unsigned long)(strtoul (optarg, &pEndPtr, 0));
               bOperationMode = 1;
            }
            break;
         case 'b':
            if (optarg)
            {
               g_nBufferSize = (unsigned long)(strtoul (optarg, &pEndPtr, 0));
               bBufferSize = 1;
            }
            break;
         case 's':
            if (optarg)
            {
               strncpy (string, optarg, sizeof(string)-1);
               string[sizeof(string)-1] = 0;
               token = strtok (string, seps);
               if (token != NULL)
               {
                  for (i = 0; i < 2; i++)
                  {
                     sscanf (token, "%x", &nVal);
                     if (i == 0)
                     {
                        g_nStartStreamId = (unsigned short) nVal;
                     }
                     else
                     {
                        g_nStopStreamId = (unsigned short) nVal;
                     }
                     /* Get next token */
                     token = strtok(NULL, seps);
                     /* Exit scanning if no further information is included */
                     if (token == NULL)
                     {
                        break;
                     }
                  }
               }
               if (i > 0) bStreamIdFilter = 1;
            }
            break;
         case 'e':
            if (optarg)
            {
               strncpy (string, optarg, sizeof(string)-1);
               string[sizeof(string)-1] = 0;
               token = strtok (string, seps);
               if (token != NULL)
               {
                  for (i = 0; i < 2; i++)
                  {
                     sscanf (token, "%x", &nVal);
                     if (i == 0)
                     {
                        g_nStartEventId = (unsigned short) nVal;
                     }
                     else
                     {
                        g_nStopEventId = (unsigned short) nVal;
                     }
                     /* Get next token */
                     token = strtok(NULL, seps);
                     /* Exit scanning if no further information is included */
                     if (token == NULL)
                     {
                        break;
                     }
                  }
               }
               if (i > 0)
               {
                  bEventIdFilter = 1;
               }
            }
            break;
         case 'm':
            if (optarg)
            {
               strncpy (string, optarg, sizeof(string)-1);
               string[sizeof(string)-1] = 0;
               token = strtok (string, seps);
               if (token != NULL)
               {
                  for (i = 0;
                       i < (MEI_DBG_STREAM_MAX_COMPARE_SIZE_IN_WORDS << 1);
                       i++)
                  {
                     sscanf (token, "%x", &nVal);
                     if (i < MEI_DBG_STREAM_MAX_COMPARE_SIZE_IN_WORDS)
                     {
                        g_nStartMask[i] = (unsigned short) nVal;
                     }
                     else
                     {
                        g_nStopMask[i -
                                    MEI_DBG_STREAM_MAX_COMPARE_SIZE_IN_WORDS] =
                           (unsigned short) nVal;
                     }
                     /* Get next token */
                     token = strtok(NULL, seps);
                     /* Exit scanning if no further information is included */
                     if (token == NULL)
                     {
                        break;
                     }
                  }
               }
               if (i > 0)
               {
                  bMaskFilter = 1;
               }
            }
            break;
         case 'p':
            if (optarg)
            {
               strncpy (string, optarg, sizeof(string)-1);
               string[sizeof(string)-1] = 0;
               token = strtok (string, seps);
               if (token != NULL)
               {
                  for (i = 0;
                       i < (MEI_DBG_STREAM_MAX_COMPARE_SIZE_IN_WORDS << 1);
                       i++)
                  {
                     sscanf (token, "%x", &nVal);
                     if (i < MEI_DBG_STREAM_MAX_COMPARE_SIZE_IN_WORDS)
                     {
                        g_nStartPattern[i] = (unsigned short) nVal;
                     }
                     else
                     {
                        g_nStopPattern[i -
                           MEI_DBG_STREAM_MAX_COMPARE_SIZE_IN_WORDS] =
                           (unsigned short) nVal;
                     }
                     /* Get next token */
                     token = strtok(NULL, seps);
                     /* Exit scanning if no further information is included */
                     if (token == NULL)
                     {
                        break;
                     }
                  }
               }
               if (i > 0)
               {
                  bPatternFilter = 1;
               }
            }
            break;
         case 't':
            bText = 1;
            break;
         case 'l':
            if (optarg)
            {
               g_nFileLength = (unsigned long)(strtoul (optarg, &pEndPtr, 0));
               if (g_nFileLength < DBG_STRM_DMP_MEI_MINIMUM_FILE_SIZE)
               {
                  g_nFileLength = DBG_STRM_DMP_MEI_DEFAULT_FILE_SIZE;
                  MEIOS_Printf(DBG_STRM_DMP_MEI_DBG_PREFIX
                     "Defined file size too small. "
                     "Default file size is used." MEIOS_CRLF);
               }
               bLength = 1;
            }
            break;
         case 'i':
            if (optarg)
            {
               strncpy (string, optarg, sizeof(string)-1);
               string[sizeof(string)-1] = 0;
               token = strtok (string, seps);
               if (token != NULL)
               {
                  for (i = 0; i < DBG_STRM_DMP_MEI_INT_MASK_MAX_NUM; i++)
                  {
                     sscanf (token, "%x", &nVal);
                     g_nInternalMask[i] = (unsigned short) nVal;

                     /* Get next token */
                     token = strtok(NULL, seps);
                     /* Exit scanning if no further information is included */
                     if (token == NULL)
                     {
                        break;
                     }
                  }
               }
               if (i > 0)
               {
                  bInternalMask = 1;
               }
            }
            break;
         case 'T':
            if (optarg)
            {
               g_nSnapshotLengthInSec =
                  (unsigned short)(strtoul (optarg, &pEndPtr, 0));
               if (g_nSnapshotLengthInSec == 0)
               {
                  g_nSnapshotLengthInSec = DBG_STRM_DMP_MEI_SNAPSHOT_LEN_SEC;
               }
            }
            else
            {
               g_nSnapshotLengthInSec = DBG_STRM_DMP_MEI_SNAPSHOT_LEN_SEC;
            }
            bSnapshotMode = 1;
            break;
         case 'I':
            if (optarg)
            {
               g_nInterval = (unsigned int)(strtoul (optarg, &pEndPtr, 0));
               bInterval = 1;
            }
            break;
         case 'c':
            if (optarg)
            {
               bDfeChannel = (int)(strtoul (optarg, &pEndPtr, 0));
            }
            break;
      }        /* switch(c) {...} */
   }        /* while(1) {...} */

   if (optind < argc)
   {
      MEIOS_Printf (DBG_STRM_DMP_MEI_DBG_PREFIX"Sorry, there are unrecognized "
         "options: ");
      while (optind < argc)
         MEIOS_Printf ("%s ", argv[optind++]);
      MEIOS_Printf ("\n");
   }
}


static void doHelp(void)
{
   struct option *ptr;
   char *desc = description[0];
   ptr = long_options;
   char* operationMode = NULL;

   MEIOS_Printf("usage: mei_cpe_dbg_strm_dmp [options]" MEIOS_CRLF);
   MEIOS_Printf("following options defined:" MEIOS_CRLF);

   switch(g_nOperationMode)
   {
      case e_MEI_DBG_STREAM_DEFAULT_RING:
         operationMode = "Default Ring";
         break;
      case e_MEI_DBG_STREAM_FILL:
         operationMode = "Fill";
         break;
      case e_MEI_DBG_STREAM_USER_RING:
         operationMode = "User Ring";
         break;
      case e_MEI_DBG_STREAM_FIFO:
         operationMode = "Fifo";
         break;
      default:
         operationMode = "Unknown";
         break;
   }

   while(ptr->name)
   {
      MEIOS_Printf(" --%s (-%c)\t- %s" MEIOS_CRLF, ptr->name, ptr->val, desc);
      ptr++;
      desc += sizeof(description[0]);
   }
   MEIOS_Printf("Note: All the options are optional. If you run application "
      "without options the stdout will be used for the output. The following "
      "configuration values will be used by default:" MEIOS_CRLF);
   MEIOS_Printf("   - Binary output file format " MEIOS_CRLF);
   MEIOS_Printf("   - Operation Mode: %s " MEIOS_CRLF, operationMode);
   MEIOS_Printf("   - Buffer Size = %d bytes " MEIOS_CRLF, g_nBufferSize);
   MEIOS_Printf("   - 'Tracee like trigger' is off by default" MEIOS_CRLF);
   MEIOS_Printf("Use CTRL-C combination to stop application in forground or "
      "'killall' in background." MEIOS_CRLF );
   MEIOS_Printf("How to use 'Tracee like trigger' configuration parameters "
      "(-s, -e, -m, -p):" MEIOS_CRLF );
   MEIOS_Printf("   All the values must be defined in hexadecimal 16-bit "
      "numbers without 0x prefix." MEIOS_CRLF );
   MEIOS_Printf("   Format for -s and -e options:" MEIOS_CRLF );
   MEIOS_Printf("   -s XXXX:YYYY" MEIOS_CRLF );
   MEIOS_Printf("   where XXXX start values in HEX, YYYY - stop values in HEX."
      MEIOS_CRLF );
   MEIOS_Printf("   Format for -m and -p options:" MEIOS_CRLF );
   MEIOS_Printf("   -m XXXX:XXXX:XXXX:XXXX:YYYY:YYYY:YYYY:YYYY" MEIOS_CRLF );
   MEIOS_Printf("   where XXXX start values in HEX, YYYY - stop values in HEX."
      MEIOS_CRLF);
   MEIOS_Printf("   All the 'Tracee like trigger' configuration parameters "
      "can be used together or separately."MEIOS_CRLF );
   MEIOS_Printf("   The start values define conditions to start the event "
      "handling." MEIOS_CRLF );
   MEIOS_Printf("   The stop values define conditions to stop the event "
      "handling." MEIOS_CRLF );
   MEIOS_Printf("   If only stop condition is defined then the event handling "
      "is started immediately after the application has been run." MEIOS_CRLF );
   MEIOS_Printf("   If only start conditions or both start/stop conditions "
      "are defined then the event handling is started after reaching the first "
      " starting condition (stream ID, event ID or mask-pattern combination)."
      MEIOS_CRLF MEIOS_CRLF);
   MEIOS_Printf("Examples:" MEIOS_CRLF );
   MEIOS_Printf("   - send debug stream events into the stdout:" MEIOS_CRLF );
   MEIOS_Printf("./mei_cpe_drv_dbg_strm_dmp" MEIOS_CRLF MEIOS_CRLF);
   MEIOS_Printf("   - minimum required options to save dump into the file:"
      MEIOS_CRLF );
   MEIOS_Printf("./mei_cpe_drv_dbg_strm_dmp -f ./dump.bin"
      MEIOS_CRLF MEIOS_CRLF);
   MEIOS_Printf("   - text file output format used, started as a daemon:"
      MEIOS_CRLF );
   MEIOS_Printf("./mei_cpe_drv_dbg_strm_dmp -f ./dump.txt -t &" MEIOS_CRLF
      MEIOS_CRLF);
   MEIOS_Printf("   - internal mask filter configured, started as a daemon:"
      MEIOS_CRLF );
   MEIOS_Printf("./mei_cpe_drv_dbg_strm_dmp -f ./dump.bin -o 1 -b 40000 "
      "-i 8:0:0:0:0 &" MEIOS_CRLF MEIOS_CRLF);
   MEIOS_Printf("   - 'Tracee like trigger' configuration to start event "
      "handling since the first GHS message:" MEIOS_CRLF );
   MEIOS_Printf("./mei_cpe_drv_dbg_strm_dmp -f ./dump.bin -o 0 -b 40000 "
      "-e B:0 &" MEIOS_CRLF MEIOS_CRLF);
   MEIOS_Printf("   - 'Tracee like trigger' configuration to start event "
      "handling since the first GHS message and stop on the Stream ID = 0x83:"
      MEIOS_CRLF );
   MEIOS_Printf("./mei_cpe_drv_dbg_strm_dmp -f ./dump.bin -o 0 -b 40000 "
      "-s 0:83 -e B:0 &" MEIOS_CRLF MEIOS_CRLF);
   MEIOS_Printf("   - 'Tracee like trigger' configuration to start event "
      "handling since the first GHS message and stop on the Stream ID = 0x1 "
      "or on the data payload with 0xf in the last bits of each of four "
      "starting words:" MEIOS_CRLF );
   MEIOS_Printf("./mei_cpe_drv_dbg_strm_dmp -f ./dbg_strm_dump.bin -e B:0 -s "
      "0:1 -m 0:0:0:0:f:f:f:f -p 0:0:0:0:f:f:f:f &" MEIOS_CRLF MEIOS_CRLF);
   MEIOS_Printf("   - full set of configuration options are defined, started "
      "as a daemon:" MEIOS_CRLF );
   MEIOS_Printf("./mei_cpe_drv_dbg_strm_dmp -f ./dump.bin -o 1 -b 40000 "
      "-s 0:2 -e 2:0 -m F:0:0:0:F:0:0:0 -p 2:0:0:0:4:0:0:0 -i 8:0:0:0:0 &"
      MEIOS_CRLF MEIOS_CRLF);
   MEIOS_Printf("   - snapshot feature activated:" MEIOS_CRLF );
   MEIOS_Printf("./mei_cpe_drv_dbg_strm_dmp -f ./dump.bin -T -s 0 -e 2 "
      "-m F:0:0:0 -p 2:0:0:0 &"
      MEIOS_CRLF MEIOS_CRLF);

   return;
}

/**
   Description:
      Debug daemon which configures, starts and collect debug information
      (so called debug streams) from the DSL Firmware.
   Arguments:
      argc - number of parameters
      argv - array of parameter strings
   Return Value:
      None.
   Remarks:
      None.
*/
int main(int argc, char *argv[])
{
   parseArgs(argc,argv);

   if( bHelp == 1)
   {
      doHelp();
      return EXIT_SUCCESS;
   }

   /* Options combination check */
   if ((bSnapshotMode == 1)   &&
       ((bOperationMode == 1) ||
        (bBufferSize == 1)    ||
        (bLength == 1)))
   {
      MEIOS_Printf(DBG_STRM_DMP_MEI_DBG_PREFIX
         "WARNING: At least one incompatible option is used in the snapshot "
         "mode. Options -o, -b, -l cannot be used in this mode and will be "
         "discarded." MEIOS_CRLF);

      bOperationMode = 0;
      bBufferSize    = 0;
      bLength        = 0;
   }

   MEI_debug_stream_dump_daemon();

   return 0;
}

/* ============================================================== */
/* Local Function Definitions                                     */
/* ============================================================== */

static int MEI_debug_stream_dump_event_wait()
{
   MEIOS_File_t *streamOut = stdout;
   int ret;
   fd_set rfds;
   struct timeval tv;
   IOCTL_MEI_message_t message;
   CMV_MESSAGE_ALL_T MEI_RdCmvMsg;

   MEIOS_MemSet(&message, 0x00, sizeof(IOCTL_MEI_message_t));

   do
   {
      tv.tv_sec = 1;
      tv.tv_usec = 0;
      FD_ZERO(&rfds);
      FD_SET(fd, &rfds);
      ret = select(fd + 1, &rfds, NULL, NULL, &tv);
      if (ret == -1)
      {
         MEIOS_FPrintf(streamOut, DBG_STRM_DMP_MEI_DBG_PREFIX
            "ERROR - select error." MEIOS_CRLF);
         return -1;
      }
      if (ret == 0)
      {
         /* no data within timeout */
         return 1;
      }

      if (FD_ISSET(fd, &rfds) == 0)
      {
      /* not for us */
         return 1;
      }

      /* setup the ack buffer */
      MEIOS_MemSet(&MEI_RdCmvMsg, 0x00, sizeof(MEI_RdCmvMsg));
      message.pPayload = (unsigned char *)MEI_RdCmvMsg.rawMsg;
      message.paylSize_byte = sizeof(MEI_RdCmvMsg.rawMsg);

      if ( (MEIOS_DeviceControl(fd, FIO_MEI_MBOX_NFC_RD,
                                (MEI_IOCTL_ARG)&message)) < 0 )
      {
         MEIOS_FPrintf(streamOut, DBG_STRM_DMP_MEI_DBG_PREFIX
            "ERROR - can't read from device." MEIOS_CRLF);
         return -1;
      }
   } while((message.msgId != DBG_STRM_DMP_MEI_EVENT_NUM) && (bRun == 1));

   return 0;
}

static MEIOS_File_t* MEI_debug_stream_dump_new_file(MEIOS_File_t *dumpFd)
{
   #define DBG_LOG_DATE_STRING_LENGTH 16
   char dmpDate[DBG_LOG_DATE_STRING_LENGTH];
   char fileName[256];
   char *fileNamePtr = NULL;
   static char *fileExt = NULL;
   struct tm *tmpTm = NULL;
   time_t timeNow = 0;

   if (dumpFd == stdout)
   {
      return NULL;
   }

   if (dumpFd != NULL)
   {
      fflush(dumpFd);
      fclose(dumpFd);
   }

   timeNow = time(NULL);
   tmpTm = localtime(&timeNow);
   if (tmpTm != NULL)
   {
      strftime(dmpDate, DBG_LOG_DATE_STRING_LENGTH, "%Y%m%d_%H%M%S", tmpTm);

      if (bCheckFileExt == 1)
      {
         /* search for the standard file extension */
         fileExt = strstr(dumpFileName, DBG_STRM_DMP_MEI_STD_EXTENSION);
         if ((fileExt != NULL) &&
            (*(fileExt + strlen(DBG_STRM_DMP_MEI_STD_EXTENSION)) == '\0'))
         {
            /* cut off standard file extension */
            *fileExt = '\0';
         }
         else
         {
            /* the file extension is different from the standard one */
            fileExt = NULL;
         }
         bCheckFileExt = 0;
      }

      if (fileExt != NULL)
      {
         /* in case of the standard file extension insert the timestamp
            before the file extension */
         MEIOS_SNPrintf(fileName, sizeof(fileName), "%s-%s"
            DBG_STRM_DMP_MEI_STD_EXTENSION, dumpFileName, dmpDate);
      }
      else
      {
         /* in all other cases insert timestamp after full file name */
         MEIOS_SNPrintf(fileName, sizeof(fileName), "%s-%s", dumpFileName, dmpDate);
      }
      fileNamePtr = fileName;
   }
   else
   {
      MEIOS_Printf(DBG_STRM_DMP_MEI_DBG_PREFIX
         "Could not attach date to filename." MEIOS_CRLF);
      fileNamePtr = dumpFileName;
   }

   dumpFd = fopen(fileNamePtr, "w");
   if(dumpFd == NULL)
   {
      MEIOS_Printf(DBG_STRM_DMP_MEI_DBG_PREFIX
         "Could not open new file." MEIOS_CRLF);
   }

   return dumpFd;
}

static int MEI_debug_stream_dump_save_to_file(MEIOS_File_t   *dumpFd,
                                              unsigned short *pBuffer,
                                              unsigned long  *pCurLength,
                                              MEIOS_File_t   **pDumpFd)
{
   unsigned short i = 0;
   static unsigned long nWrittenBytesToFlush = 0;
   IOCTL_MEI_DEBUG_STREAM_data_t data;

   MEIOS_MemSet(&data, 0x00, sizeof(IOCTL_MEI_DEBUG_STREAM_data_t));

   data.dataBufferSize_byte = DBG_STRM_DMP_MEI_BUFFER_SIZE_BYTES;
   data.maxStreamEntries = 20;
   data.pData = (unsigned char*)pBuffer;
   data.timeout_ms = 0;
   if ((MEIOS_DeviceControl(fd,
                            FIO_MEI_DEBUG_STREAM_DATA_GET,
                            (MEI_IOCTL_ARG)&data)) < 0 )
   {
      MEIOS_FPrintf(stdout, DBG_STRM_DMP_MEI_DBG_PREFIX
         "ERROR - ioct(FIO_MEI_DEBUG_STREAM_DATA_GET), %s, retCode = %d"
         MEIOS_CRLF, MEIOS_StrError(errno), data.ictl.retCode);
      return -1;
   }

   if ((bText != 1) && (bFile == 1))
   {
      if (data.dataBufferSize_byte >= g_nFileLength)
      {
         for (; data.dataBufferSize_byte >= g_nFileLength;
                data.dataBufferSize_byte -= g_nFileLength)
         {
            *pCurLength = (data.dataBufferSize_byte < g_nFileLength)?
               data.dataBufferSize_byte :
               g_nFileLength;
            fwrite(data.pData, sizeof(char), *pCurLength, dumpFd);
            data.pData += *pCurLength;
            dumpFd = MEI_debug_stream_dump_new_file(dumpFd);
            if (dumpFd == NULL)
            {
               MEIOS_FPrintf(stdout, DBG_STRM_DMP_MEI_DBG_PREFIX
                  "ERROR - could not open new dump file" MEIOS_CRLF);
               return -1;
            }
         }
         *pCurLength = 0;
      }
      else
      {
         *pCurLength += data.dataBufferSize_byte;
         if (*pCurLength >= g_nFileLength)
         {
            dumpFd = MEI_debug_stream_dump_new_file(dumpFd);
            if (dumpFd == NULL)
            {
               MEIOS_FPrintf(stdout, DBG_STRM_DMP_MEI_DBG_PREFIX
                  "ERROR - could not open new dump file" MEIOS_CRLF);
               return -1;
            }
            *pCurLength = data.dataBufferSize_byte;
         }

         if (dumpFd != NULL)
         {
            fwrite(data.pData, sizeof(char), data.dataBufferSize_byte, dumpFd);
            nWrittenBytesToFlush += data.dataBufferSize_byte;
            if (nWrittenBytesToFlush >= DBG_STRM_DMP_MEI_BYTES_TO_FLUSH)
            {
                fflush(dumpFd);
                nWrittenBytesToFlush = 0;
            }
         }
      }
   }
   else
   {
      unsigned long nWrittenBytes = 0;

      for (i = 0; i < data.dataBufferSize_byte / 2; i++)
      {
         /* Reset cycle byte counter */
         nWrittenBytes = 0;

         if((dumpFd == stdout) && (dumpFd != NULL) &&
           ((((unsigned short*)data.pData)[i]) == DBG_STRM_DMP_MEI_EVENT_NUM))
         {
            nWrittenBytes += MEIOS_FPrintf(dumpFd, "\n");
         }
         if((((unsigned short*)data.pData)[i]) != DBG_STRM_DMP_MEI_EVENT_NUM)
         {
            if (dumpFd != NULL)
            {
               nWrittenBytes += MEIOS_FPrintf(dumpFd, "%04X ",
                  ((unsigned short*)data.pData)[i]);
            }
         }
         else
         {
            nWrittenBytes += 5;
         }

         *pCurLength += nWrittenBytes;
         nWrittenBytesToFlush += nWrittenBytes;

         if ((dumpFd != NULL) &&
             (nWrittenBytesToFlush >= DBG_STRM_DMP_MEI_BYTES_TO_FLUSH))
         {
            fflush(dumpFd);
            nWrittenBytesToFlush = 0;
         }

         if((((unsigned short*)data.pData)[i]) == DBG_STRM_DMP_MEI_EVENT_NUM)
         {
            if(*pCurLength >= g_nFileLength)
            {
               dumpFd = MEI_debug_stream_dump_new_file(dumpFd);
               if (dumpFd == NULL)
               {
                  MEIOS_FPrintf(stdout, DBG_STRM_DMP_MEI_DBG_PREFIX
                     "ERROR - could not open new dump file" MEIOS_CRLF);
                  return -1;
               }
               *pCurLength = 0;
               nWrittenBytes = 0;
               nWrittenBytesToFlush = 0;
            }
            if (dumpFd != NULL)
            {
               MEIOS_FPrintf(dumpFd, "%04X ", DBG_STRM_DMP_MEI_EVENT_NUM);
            }
         }
      }
   }

   if (data.dataBufferSize_byte == 0)
   {
      /* The data from the buffer was written fully */
      if (dumpFd != NULL)
      {
         if (pDumpFd != NULL)
         {
            *pDumpFd = dumpFd;
         }
         fflush(dumpFd);
         nWrittenBytesToFlush = 0;
      }

      return 1;
   }
   else
   {
      /* The data from the buffer was written partially */
      return 0;
   }
}

int MEI_debug_stream_dump_daemon(void)
{
   int          ret     = -1;
   MEIOS_File_t *dumpFd = NULL;
   char         buf[128];
   IOCTL_MEI_DEBUG_STREAM_configSet_t config;
   IOCTL_MEI_DEBUG_STREAM_mask_set_t  mask;
   IOCTL_MEI_DEBUG_STREAM_control_t   control;
   IOCTL_MEI_ioctl_t                  drvIoCtl;
   unsigned short *pBuffer       = NULL;
   unsigned long  nCurrentLength = 0;
   unsigned short i              = 0;
   unsigned long  nRestartTimer  = 0;/* msec */
   struct timeval tv;

   signal(SIGINT, MEI_debug_stream_dump_termination_handler);
   signal(SIGTERM, MEI_debug_stream_dump_termination_handler);

   MEIOS_MemSet(&config, 0x00, sizeof(IOCTL_MEI_DEBUG_STREAM_configSet_t));

   /* open default device or given device num */
   bDfeChannel = (bDfeChannel == -1) ?
                     DBG_STRM_DMP_MEI_DEFAULT_DFE_NUM :
                     bDfeChannel;
   if (bDfeChannel & 0x80)
   {
      MEIOS_SPrintf( buf, DBG_STRM_DMP_MEI_DEV_PATH "cntrl%d",
         bDfeChannel & ~0x80 );
   }
   else
   {
      MEIOS_SPrintf( buf, DBG_STRM_DMP_MEI_DEV_PATH "%d", bDfeChannel);
   }

   /* open device */
   MEIOS_Printf( DBG_STRM_DMP_MEI_DBG_PREFIX
      "open device: %s." MEIOS_CRLF,buf);

   fd = MEIOS_DeviceOpen(buf);
   if( fd < 0 )
   {
      MEIOS_FPrintf(stdout, DBG_STRM_DMP_MEI_DBG_PREFIX
         "Cannot open device[%02d] %s." MEIOS_CRLF, bDfeChannel, buf);
      ret = -1;
      goto MEI_DBG_STRM_DMP_EXIT;
   }

   if ( (MEIOS_DeviceControl(fd,
                             FIO_MEI_MBOX_NFC_ENABLE,
                             (MEI_IOCTL_ARG)&drvIoCtl)) < 0 )
   {
      MEIOS_FPrintf(stdout, DBG_STRM_DMP_MEI_DBG_PREFIX
         "ERROR - ioct(FIO_MEI_MBOX_NFC_ENABLE), %s, retCode = %d"
         MEIOS_CRLF, MEIOS_StrError(errno), config.ictl.retCode);

      ret = -1;
      goto MEI_DBG_STRM_DMP_EXIT;
   }

   /* now configure new stream*/
   config.onOff = 1;
   config.notificationEnabled = 1;

   if (bSnapshotMode == IFX_TRUE)
   {
      /*
         The symbol counter increments with 4,3125KHz, so a symbol count delta
         of 4313 is equal to 1 second. So, the user defines -a in seconds,
         e.g. 120. Once the trigger is hit at symbol count x, the amount of
         debug streams from symbol count x until x-120*4313 is stored as an
         extra file.
      */

      config.filterMode    = e_MEI_DBG_STREAM_SNAPSHOT;
      config.bufferSize    = g_nSnapshotLengthInSec * 4313;
      config.operationMode = e_MEI_DBG_STREAM_DEFAULT_RING;

      if (bText == 1)
      {
         /* _HHHH => 2 bytes are coded as 5 ASCII bytes */
         g_nFileLength = (config.bufferSize * 5) >> 1;
      }
      else
      {
         g_nFileLength = config.bufferSize;
      }
   }
   else
   {
      config.filterMode    = e_MEI_DBG_STREAM_START_STOP;
      config.operationMode = (MEI_DBG_STREAM_BUF_OPMODE_E)g_nOperationMode;
      config.bufferSize    = g_nBufferSize;
   }

   config.startStreamId = g_nStartStreamId;
   config.stopStreamId = g_nStopStreamId;
   config.startEventId = g_nStartEventId;
   config.stopEventId = g_nStopEventId;
   for (i = 0; i < MEI_DBG_STREAM_MAX_COMPARE_SIZE_IN_WORDS; i++)
   {
      config.startMask[i] = g_nStartMask[i];
      config.stopMask[i] = g_nStopMask[i];
      config.startPattern[i] = g_nStartPattern[i];
      config.stopPattern[i] = g_nStopPattern[i];
   }

   if ((MEIOS_DeviceControl(fd,
                            FIO_MEI_DEBUG_STREAM_CONFIG_SET,
                            (MEI_IOCTL_ARG)&config)) < 0 )
   {
      MEIOS_FPrintf(stdout, DBG_STRM_DMP_MEI_DBG_PREFIX
         "ERROR - ioct(FIO_MEI_DEBUG_STREAM_CONFIG_SET), %s, retCode = %d"
         MEIOS_CRLF, MEIOS_StrError(errno), config.ictl.retCode);
      ret = -1;
      goto MEI_DBG_STRM_DMP_EXIT;
   }

   pBuffer=(unsigned short*)MEIOS_MemAlloc(DBG_STRM_DMP_MEI_BUFFER_SIZE_BYTES);
   if (pBuffer == NULL)
   {
      MEIOS_FPrintf(stdout, DBG_STRM_DMP_MEI_DBG_PREFIX
         "ERROR - could not allocate buffer %d bytes." MEIOS_CRLF,
         DBG_STRM_DMP_MEI_BUFFER_SIZE_BYTES);
      ret = -1;
      goto MEI_DBG_STRM_DMP_EXIT;
   }

   if( bFile == 1)
   {
      if( (dumpFd = fopen(dumpFileName, "w")) == NULL )
      {
         /* error open given file */
         MEIOS_FPrintf(stdout, DBG_STRM_DMP_MEI_DBG_PREFIX
            "ERROR - Open Msg file %s" MEIOS_CRLF, dumpFileName);
         ret = -1;
         goto MEI_DBG_STRM_DMP_EXIT;
      }
   }
   else
   {
      /* if the output file name is not defined use stdout as output */
      dumpFd = stdout;
   }

   control.onOff = 1;
   control.operationMode = config.operationMode;
   MEIOS_DeviceControl(fd,
                       FIO_MEI_DEBUG_STREAM_CONTROL,
                       (MEI_IOCTL_ARG)&control);
   MEIOS_FPrintf(stdout, DBG_STRM_DMP_MEI_DBG_PREFIX
      "Debug stream logging started." MEIOS_CRLF
      "Use CTRL-C combination to stop application in forground or "
      "'killall' in background." MEIOS_CRLF );


   if (bInternalMask == 1)
   {
      MEIOS_MemSet(&mask, 0x00, sizeof(IOCTL_MEI_DEBUG_STREAM_mask_set_t));

      mask.mask1 = g_nInternalMask[0];
      mask.mask2 = g_nInternalMask[1];
      mask.mask3 = g_nInternalMask[2];
      mask.mask4 = g_nInternalMask[3];
      mask.mask5 = g_nInternalMask[4];

      if ((MEIOS_DeviceControl(fd,
                               FIO_MEI_DEBUG_STREAM_MASK_SET,
                               (MEI_IOCTL_ARG)&mask)) < 0 )
      {
         MEIOS_FPrintf(stdout, DBG_STRM_DMP_MEI_DBG_PREFIX
            "ERROR - ioct(FIO_MEI_DEBUG_STREAM_MASK_SET), %s, retCode = %d"
            MEIOS_CRLF, MEIOS_StrError(errno), config.ictl.retCode);
         ret = -1;
         goto MEI_DBG_STRM_DMP_EXIT;
      }
   }

  while (bRun)
   {
      if ((bSnapshotMode == IFX_TRUE) && (nRestartTimer != 0))
      {
         MEIOS_MemSet(&tv, 0, sizeof(tv));
         gettimeofday(&tv, NULL);

         if ((unsigned long)(tv.tv_sec*1000+(tv.tv_usec)/1000) >=
             (nRestartTimer + g_nInterval))
         {
            MEIOS_MemSet(&control,
                         0x00,
                         sizeof(IOCTL_MEI_DEBUG_STREAM_control_t));

            /* Start debug event generation */
            control.onOff = 1;
            MEIOS_DeviceControl(fd,
                                FIO_MEI_DEBUG_STREAM_CONTROL,
                                (MEI_IOCTL_ARG)&control);

            /* Create a file for a new snapshot */
            nCurrentLength = 0;
            dumpFd = MEI_debug_stream_dump_new_file(dumpFd);
            if (dumpFd == NULL)
            {
               goto MEI_DBG_STRM_DMP_EXIT;
            }

            /* Stop the 'inactive' timer */
            nRestartTimer = 0;
         }
      }

      ret = MEI_debug_stream_dump_event_wait();
      if((ret < 0) && (bRun == 1))
      {
         /* break the loop immediately
            if the application has not terminated */
         break;
      }
      if((ret > 0) && (bRun == 1))
      {
         /* continue to wait data accessibility
            if the application has not terminated */
         sleep(1);
         continue;
      }

      if ((bSnapshotMode == IFX_TRUE) && (nRestartTimer == 0))
      {
         MEIOS_MemSet(&tv, 0, sizeof(tv));
         MEIOS_MemSet(&control,
                      0x00,
                      sizeof(IOCTL_MEI_DEBUG_STREAM_control_t));

         /* Stop debug event generation */
         control.onOff = 0;
         MEIOS_DeviceControl(fd,
                             FIO_MEI_DEBUG_STREAM_CONTROL,
                             (MEI_IOCTL_ARG)&control);

         /* Set the restart timer */
         gettimeofday(&tv, NULL);
         nRestartTimer = (unsigned long)(tv.tv_sec*1000+(tv.tv_usec)/1000);
      }

      do
      {
         ret = MEI_debug_stream_dump_save_to_file(dumpFd,
                                                  pBuffer,
                                                  &nCurrentLength,
                                                  &dumpFd);
      } while (ret == 0);

      if (ret == -1)
      {
         MEIOS_FPrintf(stdout, DBG_STRM_DMP_MEI_DBG_PREFIX
            "File saving operation was interrupted by error." MEIOS_CRLF);

         break;
      }
   }

MEI_DBG_STRM_DMP_EXIT:

   if (fd > 0)
   {
      MEIOS_DeviceClose(fd);
   }

   if (dumpFd != NULL)
   {
      fflush(dumpFd);
      fclose(dumpFd);
   }

   if (pBuffer != NULL)
   {
      IFXOS_MemFree(pBuffer);
   }

   MEIOS_FPrintf(stdout, DBG_STRM_DMP_MEI_DBG_PREFIX
      "Line = %02d Return = %d" MEIOS_CRLF, bDfeChannel, ret);

   return ret;
}

#endif /* LINUX */
