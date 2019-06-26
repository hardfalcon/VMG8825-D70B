/******************************************************************************

                          Copyright (c) 2007-2015
                     Lantiq Beteiligungs-GmbH & Co. KG

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/* ==========================================================================
   Description : Small test programm to test the VRX Driver
   ========================================================================== */

/* ============================= */
/* Includes                      */
/* ============================= */

#ifdef LINUX
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>

/* open */
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>

#include <unistd.h>
#include <getopt.h>
#include <sys/select.h>


/** get interface and configuration */
#include "drv_vrx_interface.h"
#include "test_internal/drv_test_vrx_interface.h"
#include "cmv_message_format.h"

/* ==========================================================================
   defines
   ========================================================================== */

#define TEST_INTERN_PREFIX ">>> "

#define MEI_TEST_INTERN_MAX_STR_PARAMS      5
#define MEI_TEST_INTERN_STR_PARAM_LEN       32
#define MEI_TEST_INTERN_MAX_OPT_PARAMS      8

#define MEI_TEST_INTERN_IOCTL_ARG unsigned long
#define MEI_TEST_INTERN_DEV_NAME  "/dev/mei_test"


/* ==========================================================================
   Local Function Declaration
   ========================================================================== */

static void GetOptArg_RequDigit(char *pOptArg, int *pFlag, char *pDesc);
static void GetOptArg_RequStr(char *pOptArg, char *pDest, char *pDesc);

static void parseArgs(int argc, char *argv[]);
static void doHelp(void);
static void MEI_TEST_Intern(FILE *streamOut);
static int MEI_TEST_OpenDev(FILE *streamOut, int devNum, char *pDevBase);
static int MEI_TEST_GetVers(FILE *streamOut, int fd, IOCTL_MEI_TEST_internArg_t *pArgs);
static int MEI_TEST_DbgLevelSet(FILE *streamOut, int fd, IOCTL_MEI_TEST_internArg_t *pArgs);
static int MEI_TEST_InitVrxDev(FILE *streamOut, int fd, IOCTL_MEI_TEST_internArg_t *pArgs);
static int MEI_TEST_Reset(FILE *streamOut, int fd, IOCTL_MEI_TEST_internArg_t *pArgs);
static int MEI_TEST_FwDl(FILE *streamOut, int fd, IOCTL_MEI_TEST_internArg_t *pArgs);
static int MEI_TEST_FwSwap(FILE *streamOut, int fd, IOCTL_MEI_TEST_internArg_t *pArgs);
static int MEI_TEST_ReqCfg(FILE *streamOut, int fd, IOCTL_MEI_TEST_internArg_t *pArgs);
static int MEI_TEST_SetMeiReg(FILE *streamOut, int fd, IOCTL_MEI_TEST_internArg_t *pArgs);
static int MEI_TEST_GetMeiReg(FILE *streamOut, int fd, IOCTL_MEI_TEST_internArg_t *pArgs);
static int MEI_TEST_SetMeiDbg(FILE *streamOut, int fd, IOCTL_MEI_TEST_internArg_t *pArgs);
static int MEI_TEST_GetMeiDbg(FILE *streamOut, int fd, IOCTL_MEI_TEST_internArg_t *pArgs);
static int MEI_TEST_SetGpa(FILE *streamOut, int fd, IOCTL_MEI_TEST_internArg_t *pArgs);
static int MEI_TEST_GetGpa(FILE *streamOut, int fd, IOCTL_MEI_TEST_internArg_t *pArgs);
static int MEI_TEST_SendMsg(FILE *streamOut, int fd, IOCTL_MEI_TEST_internArg_t *pArgs);
void MEI_TEST_LogIfxMsg(FILE *streamOut, char *pLogStr, IOCTL_MEI_message_t *pCntrl);
static void MEI_TEST_SetupSendMsg(
                     IOCTL_MEI_message_t *pMsg,
                     unsigned char *pBuf,
                     unsigned short msgID, int *pParamArr);

/* ==========================================================================
   Local Variables (used for arguments)
   ========================================================================== */

static int bTestInternHelp          = -1;    /* h */
static int bTestInternTestGetVer    = -1;    /* v */
static int bTestInternTestDbgLvl    = -1;    /* l */
static int bTestInternVrxLine       = -1;    /* n */

static int bTestInternInitDev       = -1;    /* i */
static int bTestInternDrvReset      = -1;    /* R */

static int bTestInternFwDl          = -1;    /* F */
static int bTestInternFwSwap        = -1;    /* f */

static int bTestInternReqCfg        = -1;    /* c */
static int bTestInternTargetDest    = -1;    /* d */

static int bTestInternSetMeiReg     = -1;    /* S */
static int bTestInternGetMeiReg     = -1;    /* s */

static int bTestInternSetMeiDbg   = -1;      /* M */
static int bTestInternGetMeiDbg   = -1;      /* m */

static int bTestInternSetGpaReg     = -1;    /* A */
static int bTestInternGetGpaReg     = -1;    /* a */

static int bTestInternSendMsg       = -1;    /* W */

static char bTestInternStrParam[MEI_TEST_INTERN_MAX_STR_PARAMS][MEI_TEST_INTERN_STR_PARAM_LEN] =
   { {"\0"}, {"\0"}, {"\0"}, {"\0"}, {"\0"} };

static int bTestInternParams[MEI_TEST_INTERN_MAX_OPT_PARAMS] =
   {-1, -1, -1, -1, -1, -1, -1, -1};  /* 0, 1, 2, 3, 4, 5, 6, 7 */


/* ==========================================================================
   Local variables for parse and interprete arguments
   ========================================================================== */


/*
   0 no_argument
   1 required_argument
   2 optional_argument
*/

static struct option long_options[] =
{
/*  0 */   {"help", no_argument,        NULL, 'h'},
/*  1 */   {"version", no_argument,        NULL, 'v'},
/*  2 */   {"level", no_argument,        NULL, 'l'},
/*  3 */   {"nLine", required_argument,  NULL, 'n'},
/*  4 */   {"init_dev", required_argument,  NULL, 'i'},
/*  5 */   {"MEI_rst", required_argument,  NULL, 'R'},
/*  6 */   {"fw_downl", no_argument,        NULL, 'F'},
/*  7 */   {"fw_swap", required_argument,  NULL, 'f'},
/*  8 */   {"Config", no_argument,        NULL, 'c'},
/*  9 */   {"dbg_dest", required_argument,  NULL, 'd'},
/* 10 */   {"set_reg", required_argument,  NULL, 'S'},
/* 11 */   {"get_reg", required_argument,  NULL, 's'},
/* 12 */   {"dbg_set", required_argument,  NULL, 'M'},
/* 13 */   {"dbg_get", required_argument,  NULL, 'm'},
/* 13 */   {"gpa_set", required_argument,  NULL, 'A'},
/* 15 */   {"gpa_get", required_argument,  NULL, 'a'},
/* 16 */   {"mb_send", required_argument,  NULL, 'W'},
/*    */   {"param_0", required_argument,  NULL, '0'},
/*    */   {"param_1", required_argument,  NULL, '1'},
/*    */   {"param_2", required_argument,  NULL, '2'},
/*    */   {"param_3", required_argument,  NULL, '3'},
/*    */   {"param_4", required_argument,  NULL, '4'},
/*    */   {"param_5", required_argument,  NULL, '5'},
/*    */   {"param_6", required_argument,  NULL, '6'},
/*    */   {"param_7", required_argument,  NULL, '7'},
/*    */   {"str_w", required_argument,  NULL, 'w'},
/*    */   {"str_x", required_argument,  NULL, 'x'},
/*    */   {"str_y", required_argument,  NULL, 'y'},
/*    */   {"str_z", required_argument,  NULL, 'z'},
           {NULL, 0, NULL, 0}
};


#define GETOPT_LONG_OPTSTRING "hvl:n:i:R:Ff:cd:S:s:M:m:A:a:W:0:1:2:3:4:5:6:7:w:x:y:z:"

static char description[][64] =
{
/*  0 */   {"help screen"},
/*  1 */   {"drv version"},
/*  2 */   {"Debug Level (-l [1..3]"},
/*  3 */   {"nLine (-n <line>, default: 0)"},
/*  4 */   {"init the MEI dev (base Addr, -0 IRQ)"},
/*  5 */   {"MEI Rst (-R <0/1/2 rst/act/deact> [cntrl<-0 MEI sel mask>]"},
/*  6 */   {"FW Download [-w filename]"},
/*  7 */   {"FW Swap -f <mode> (0: VDSL2, 1: ADSL)"},
/*  8 */   {"MEI Config"},
/*  9 */   {"dbg dest AUX=0, MEM=2, CORE=3"},
/* 10 */   {"set MEI register (-S offset, -0 value)"},
/* 11 */   {"get MEI register (-s offset)"},
/* 12 */   {"mei dbg set (-M <adr> -d <dest> -0 <val>)"},
/* 13 */   {"mei dbg get (-m <adr> -d <dest>)"},
/* 13 */   {"GPA access set (-A <addr> -d <dest>) -0 <value>"},
/* 15 */   {"GPA access get (-a <addr> -d <dest>)"},
/* 16 */   {"mbox send (-W msgID, [-0 <param> -1 <param> ...] )"},
/*    */   {"param_0"},
/*    */   {"param_1"},
/*    */   {"param_2"},
/*    */   {"param_3"},
/*    */   {"param_4"},
/*    */   {"param_5"},
/*    */   {"param_6"},
/*    */   {"param_7"},
/*    */   {"str_w"},
/*    */   {"str_x"},
/*    */   {"str_y"},
/*    */   {"str_z"},
           {0}
};


/* ============================================================== */
/* Local Function to parse and interprete arguments               */
/* ============================================================== */

/*
   get an argument
*/
static void GetOptArg_RequDigit(char *pOptArg, int *pFlag, char *pDesc)
{
   unsigned long temp;
   char *pEndPtr;

   if (optarg)
   {
      temp = strtoul(optarg, &pEndPtr, 0);
      if (errno)
      {
         printf( TEST_INTERN_PREFIX
                 "%s: invalid argument = 0x%08X\n\r", pDesc, (unsigned int)temp);
      }
      else
      {
         printf( TEST_INTERN_PREFIX
                 "%s: 0x%08X\n\r", pDesc, (unsigned int)temp);
         *pFlag = temp;
      }
   }
   else
   {
      printf( TEST_INTERN_PREFIX
              "%s: missing argument\n\r", pDesc);
   }

   return;
}

/*
   get an argument
*/
static void GetOptArg_RequStr(char *pOptArg, char *pDest, char *pDesc)
{
   unsigned long strLength;

   if (optarg)
   {
      if ( (strLength = strlen(optarg)) > 0)
      {
         memset(pDest, 0x00, MEI_TEST_INTERN_STR_PARAM_LEN);
         strncpy(pDest, (char *)optarg,
                 (strLength > (MEI_TEST_INTERN_STR_PARAM_LEN -1)) ?
                        (MEI_TEST_INTERN_STR_PARAM_LEN -1) : strLength);
         printf( TEST_INTERN_PREFIX
                 "%s: %s\n\r", pDesc, pDest);
      }
   }
   else
   {
      printf( TEST_INTERN_PREFIX
              "%s: missing argument\n\r", pDesc);
   }

   return;
}


/*
   Parse the arguments.
*/
static void parseArgs(int argc, char *argv[])
{
   int parm_no=0;

   while(1)
   {
      int option_index = 0;
      int c;

      /* 1 colon means there is a required parameter */
      /* 2 colons means there is an optional parameter */
      c = getopt_long (argc, argv, GETOPT_LONG_OPTSTRING, long_options, &option_index);

      if(c == -1)
      {
         if (parm_no==0)
            bTestInternHelp = 1;
         return;
      }

      parm_no++;

      switch(c)
      {
         case 'h':
            bTestInternHelp = 1;
            break;
         case 'v':
            bTestInternTestGetVer = 1;
            break;
         case 'l':
            GetOptArg_RequDigit(optarg, &bTestInternTestDbgLvl, "DbgLvl");
            break;
         case 'n':
            GetOptArg_RequDigit(optarg, &bTestInternVrxLine, "line");
            break;
         case 'i':
            GetOptArg_RequDigit(optarg, &bTestInternInitDev, "Init");
            break;
         case 'f':
            GetOptArg_RequDigit(optarg, &bTestInternFwSwap, "Swap");
            break;
         case 'F':
            bTestInternFwDl = 1;
            break;
         case 'R':
            GetOptArg_RequDigit(optarg, &bTestInternDrvReset, "Rst");
            break;
         case 'c':
            bTestInternReqCfg = 1;
            break;
         case 'd':
            GetOptArg_RequDigit(optarg, &bTestInternTargetDest, "Dest");
            break;
         case 'S':
            GetOptArg_RequDigit(optarg, &bTestInternSetMeiReg, "RegSet");
            break;
         case 's':
            GetOptArg_RequDigit(optarg, &bTestInternGetMeiReg, "RegGet");
            break;
         case 'M':
            GetOptArg_RequDigit(optarg, &bTestInternSetMeiDbg, "DBGSet");
            break;
         case 'm':
            GetOptArg_RequDigit(optarg, &bTestInternGetMeiDbg, "DBGGet");
            break;
         case 'A':
            GetOptArg_RequDigit(optarg, &bTestInternSetGpaReg, "GPASet");
            break;
         case 'a':
            GetOptArg_RequDigit(optarg, &bTestInternGetGpaReg, "GPAGet");
            break;
         case 'W':
            GetOptArg_RequDigit(optarg, &bTestInternSendMsg, "Msg Send");
            break;

         case '0':
            GetOptArg_RequDigit(optarg, &bTestInternParams[0], "opt Arg[0]");
            break;
         case '1':
            GetOptArg_RequDigit(optarg, &bTestInternParams[1], "opt Arg[1]");
            break;
         case '2':
            GetOptArg_RequDigit(optarg, &bTestInternParams[2], "opt Arg[2]");
            break;
         case '3':
            GetOptArg_RequDigit(optarg, &bTestInternParams[3], "opt Arg[3]");
            break;
         case '4':
            GetOptArg_RequDigit(optarg, &bTestInternParams[4], "opt Arg[4]");
            break;
         case '5':
            GetOptArg_RequDigit(optarg, &bTestInternParams[5], "opt Arg[5]");
            break;
         case '6':
            GetOptArg_RequDigit(optarg, &bTestInternParams[6], "opt Arg[6]");
            break;
         case '7':
            GetOptArg_RequDigit(optarg, &bTestInternParams[7], "opt Arg[7]");
            break;

         case 'w':
            GetOptArg_RequStr(optarg, &bTestInternStrParam[0][0], "opt Str[0]");
            break;
         case 'x':
            GetOptArg_RequStr(optarg, &bTestInternStrParam[1][0], "opt Str[1]");
            break;
         case 'y':
            GetOptArg_RequStr(optarg, &bTestInternStrParam[2][0], "opt Str[2]");
            break;
         case 'z':
            GetOptArg_RequStr(optarg, &bTestInternStrParam[3][0], "opt Str[3]");
            break;


      }        /* switch(c) {...} */
   }        /* while(1) {...} */

   if (optind < argc)
   {
      printf( TEST_INTERN_PREFIX
              "Sorry, there are unrecognized options: ");
      while (optind < argc)
         printf ("%s ", argv[optind++]);
      printf ("\n");
   }

   return;
}


static void doHelp(void)
{
   struct option *ptr;
   char *desc = description[0];
   ptr = long_options;

   printf("usage: test_mei_intern [options]\n\r");
   printf("following options defined:\n\r");

   while(ptr->name)
   {
      printf(" --%s \t(-%c)- %s\n\r", ptr->name, ptr->val, desc);
      ptr++;
      desc += sizeof(description[0]);
   }

   printf("\n\r");
   return;
}


static void MEI_TEST_Intern(FILE *streamOut)
{
   int fd, ret=0;
   IOCTL_MEI_TEST_internArg_t localArgs;


   bTestInternVrxLine = (bTestInternVrxLine == -1) ? 0 : bTestInternVrxLine;

   fd = MEI_TEST_OpenDev(streamOut, bTestInternVrxLine, MEI_TEST_INTERN_DEV_NAME);
   if (fd <= 0)
   {
      fprintf( streamOut, TEST_INTERN_PREFIX
               "Error = %d - open Dev <%s/%d>\n\r",
               errno, MEI_TEST_INTERN_DEV_NAME, bTestInternVrxLine);
      return;
   }

   memset(&localArgs, 0x00, sizeof(IOCTL_MEI_TEST_internArg_t));

   /* test case: request Version */
   if (bTestInternTestGetVer == 1)
   {
      MEI_TEST_GetVers(streamOut, fd, &localArgs);
      goto MEI_TEST_INTERN_END;
   }

   /* test case: request Version */
   if (bTestInternTestDbgLvl != -1)
   {
      MEI_TEST_DbgLevelSet(streamOut, fd, &localArgs);
      goto MEI_TEST_INTERN_END;
   }

   /* test case: basic init VRX Driver
                 "-i base Addr, -0 IRQ"
   */
   if (bTestInternInitDev != -1)
   {
      ret = MEI_TEST_InitVrxDev(streamOut, fd, &localArgs);
      goto MEI_TEST_INTERN_END;
   }

   /* test case: reset VRX Drv and Dev
                 "MEI Rst (-R <0/1/2 rst/act/deact> [cntrl<-0 MEI sel mask>]"
   */
   if (bTestInternDrvReset != -1)
   {
      ret = MEI_TEST_Reset(streamOut, fd, &localArgs);
      goto MEI_TEST_INTERN_END;
   }

   /* test case: MEI FW download*/
   if (bTestInternFwDl != -1)
   {
      ret = MEI_TEST_FwDl(streamOut, fd, &localArgs);
      goto MEI_TEST_INTERN_END;
   }

   /* test case: swap ADSL / VDSL FW */
   if (bTestInternFwSwap != -1)
   {
      ret = MEI_TEST_FwSwap(streamOut, fd, &localArgs);
      goto MEI_TEST_INTERN_END;
   }

   /* test case: Request MEI Config */
   if (bTestInternReqCfg != -1)
   {
      ret = MEI_TEST_ReqCfg(streamOut, fd, &localArgs);
      goto MEI_TEST_INTERN_END;
   }

   /* test case: Set MEI Reg */
   if (bTestInternSetMeiReg != -1)
   {
      ret = MEI_TEST_SetMeiReg(streamOut, fd, &localArgs);
      goto MEI_TEST_INTERN_END;
   }

   /* test case: Get MEI Reg */
   if (bTestInternGetMeiReg != -1)
   {
      ret = MEI_TEST_GetMeiReg(streamOut, fd, &localArgs);
      goto MEI_TEST_INTERN_END;
   }

   /* test case: Set MEI Debug */
   if (bTestInternSetMeiDbg != -1)
   {
      ret = MEI_TEST_SetMeiDbg(streamOut, fd, &localArgs);
      goto MEI_TEST_INTERN_END;
   }

   /* test case: Get MEI Debug */
   if (bTestInternGetMeiDbg != -1)
   {
      ret = MEI_TEST_GetMeiDbg(streamOut, fd, &localArgs);
      goto MEI_TEST_INTERN_END;
   }

   /* test case: Set GPA */
   if (bTestInternSetGpaReg != -1)
   {
      ret = MEI_TEST_SetGpa(streamOut, fd, &localArgs);
      goto MEI_TEST_INTERN_END;
   }

   /* test case: Get GPA */
   if (bTestInternGetGpaReg != -1)
   {
      ret = MEI_TEST_GetGpa(streamOut, fd, &localArgs);
      goto MEI_TEST_INTERN_END;
   }

   /* test case: Send Vrx Msg */
   if (bTestInternSendMsg != -1)
   {
      ret = MEI_TEST_SendMsg(streamOut, fd, &localArgs);
      goto MEI_TEST_INTERN_END;
   }

MEI_TEST_INTERN_END:

   close(fd);
   fprintf( streamOut, TEST_INTERN_PREFIX
            "Line = %02d Return = %d\n\r", bTestInternVrxLine, ret);
   return;
}


/**
   open an VRX device
*/
static int MEI_TEST_OpenDev(FILE *streamOut, int devNum, char *pDevBase)
{
   int fd;
   char buf[128];

   sprintf( buf, "%s/%d", pDevBase, devNum);

   /* open device */
   fprintf( streamOut, TEST_INTERN_PREFIX "open device: %s.\n\r",buf);

   fd = open(buf, O_RDWR, 0644);
   if( fd < 0 )
   {
      fprintf( streamOut, TEST_INTERN_PREFIX
               "Cannot open device[%02d] %s.\n\r", devNum, buf);
      fprintf( streamOut, TEST_INTERN_PREFIX
               "Return = %d\n\r", -1);
      return -1;
   }

   return fd;
}


/**
   test case: request VRX Driver Version
*/
static int MEI_TEST_GetVers(FILE *streamOut, int fd, IOCTL_MEI_TEST_internArg_t *pArgs)
{
   int retVal = -1;
   char verBuf[128];

   memset(&verBuf, 0x0, sizeof(verBuf));
   pArgs->tst_version.strSize     = 127;
   pArgs->tst_version.pVersionStr = verBuf;

   retVal = ioctl(fd, FIO_MEI_TEST_VERS_GET, (MEI_TEST_INTERN_IOCTL_ARG)pArgs);
   if (retVal < 0)
   {
      /* ERROR while ioctl */
      fprintf( streamOut, TEST_INTERN_PREFIX
               "ERROR - request TEST Mod version, ret = %d, retCode = %d\n\r",
                retVal, pArgs->tst_version.ictl.retCode);
   }
   else
   {
      verBuf[127] = '\0';
      fprintf( streamOut, TEST_INTERN_PREFIX
               "TEST Mod version = 0x%08X \"%s\".\n\r"
               ,pArgs->tst_version.versionId ,verBuf);
   }


   memset(&verBuf, 0x0, sizeof(verBuf));
   pArgs->intern_drvVers.strSize     = 127;
   pArgs->intern_drvVers.pVersionStr = verBuf;

   retVal = ioctl(fd, FIO_MEI_INTERN_VERS_GET, (MEI_TEST_INTERN_IOCTL_ARG)pArgs);
   if (retVal < 0)
   {
      /* ERROR while ioctl */
      fprintf( streamOut, TEST_INTERN_PREFIX
               "ERROR - request MEI version, ret = %d, retCode = %d\n\r",
                retVal, pArgs->intern_drvVers.ictl.retCode);
   }
   else
   {
      verBuf[127] = '\0';
      fprintf( streamOut, TEST_INTERN_PREFIX
               "Driver version = 0x%08X \"%s\".\n\r"
               ,pArgs->intern_drvVers.versionId ,verBuf);
   }

   return 0;
}



/**
   test case: Debug Level Set
*/
static int MEI_TEST_DbgLevelSet(FILE *streamOut, int fd, IOCTL_MEI_TEST_internArg_t *pArgs)
{
   int retVal = -1;

   pArgs->tst_dbgLevel.valLevel = (unsigned int)bTestInternTestDbgLvl;

   retVal = ioctl(fd, FIO_MEI_TEST_DBG_LEVEL, (MEI_TEST_INTERN_IOCTL_ARG)pArgs);
   if (retVal < 0)
   {
      /* ERROR while ioctl */
      fprintf( streamOut, TEST_INTERN_PREFIX
               "ERROR - debug level set, ret = %d, retCode = %d\n\r",
                retVal, pArgs->tst_dbgLevel.ictl.retCode);
      return -1;
   }

   return 0;
}


/**
   test case: basic init VRX Driver
              "-i base Addr, -0 IRQ"
*/
static int MEI_TEST_InitVrxDev(FILE *streamOut, int fd, IOCTL_MEI_TEST_internArg_t *pArgs)
{
   int retVal = -1;

   if (bTestInternParams[0] == -1)
   {
      /* ERROR while ioctl */
      fprintf( streamOut, TEST_INTERN_PREFIX
               "ERROR - init dev, missing IRQ\n\r");
      return -1;
   }

   pArgs->intern_devInit.meiBaseAddr = (unsigned int)bTestInternInitDev;
   pArgs->intern_devInit.usedIRQ     = (unsigned int)bTestInternParams[0];

   retVal = ioctl(fd, FIO_MEI_INTERN_INIT, (MEI_TEST_INTERN_IOCTL_ARG)pArgs);
   if (retVal < 0)
   {
      /* ERROR while ioctl */
      fprintf( streamOut, TEST_INTERN_PREFIX
               "ERROR - init dev, ret = %d, retCode = %d\n\r",
                retVal, pArgs->intern_devInit.ictl.retCode);
      return -1;
   }

   return 0;
}


/**
   test case: reset VRX Drv and Dev
              "MEI Rst (-R <0/1/2 rst/act/deact> [cntrl<-0 MEI sel mask>]"
*/
static int MEI_TEST_Reset(FILE *streamOut, int fd, IOCTL_MEI_TEST_internArg_t *pArgs)
{
   int retVal = -1;

   pArgs->intern_devRst.rstMode    = (unsigned int)(((unsigned int)bTestInternDrvReset) & 0xFFFF);
   pArgs->intern_devRst.rstSelMask = (unsigned int)((((unsigned int)bTestInternDrvReset) & 0xFFFF0000) >> 16);

   retVal = ioctl(fd, FIO_MEI_INTERN_RESET, (MEI_TEST_INTERN_IOCTL_ARG)pArgs);
   if (retVal < 0)
   {
      /* ERROR while ioctl */
      fprintf( streamOut, TEST_INTERN_PREFIX
               "ERROR - reset MEI dev, ret = %d, retCode = %d\n\r",
                retVal, pArgs->intern_devRst.ictl.retCode);
      return -1;
   }

   return 0;
}

/**
   test case: MEI FW download
*/
static int MEI_TEST_FwDl(FILE *streamOut, int fd, IOCTL_MEI_TEST_internArg_t *pArgs)
{

   return 0;
}

/**
   test case: swap ADSL / VDSL FW
*/
static int MEI_TEST_FwSwap(FILE *streamOut, int fd, IOCTL_MEI_TEST_internArg_t *pArgs)
{

   return 0;
}

/**
   test case: Request MEI Config
*/
static int MEI_TEST_ReqCfg(FILE *streamOut, int fd, IOCTL_MEI_TEST_internArg_t *pArgs)
{
   int retVal = -1;
   IOCTL_MEI_reqCfg_t    *pReqCfg  = (IOCTL_MEI_reqCfg_t *)pArgs;
   IOCTL_MEI_statistic_t *pReqStat = (IOCTL_MEI_statistic_t *)pArgs;

   /*
      reguest config data
   */
   retVal = ioctl(fd, FIO_MEI_INTERN_REQ_CONFIG, (MEI_TEST_INTERN_IOCTL_ARG)pReqCfg);
   if (retVal < 0)
   {
      /* ERROR while ioctl */
      fprintf( streamOut, TEST_INTERN_PREFIX
               "ERROR - request MEI config, ret = %d, retCode = %d\n\r",
                retVal, pReqCfg->ictl.retCode);
      return -1;
   }

   fprintf(streamOut, TEST_INTERN_PREFIX
           "ioct(FIO_MEI_REQ_CONFIG) ===================\n\r");

   fprintf(streamOut, TEST_INTERN_PREFIX
           "REQ_CONFIG[%02d-%02d]: phy addr 0x%08X (virt 0x%08X), IRQ %d\r\n",
           pReqCfg->devNum, pReqCfg->currOpenInst,
           pReqCfg->phyBaseAddr, pReqCfg->virtBaseAddr, pReqCfg->usedIRQ);

   fprintf(streamOut, TEST_INTERN_PREFIX
           "REQ_CONFIG[%02d-%02d]: curr DrvState = %d, curr ModemFSM = %d\n\r",
           pReqCfg->devNum, pReqCfg->currOpenInst,
           pReqCfg->currDrvState, pReqCfg->currModemFsmState);

   fprintf(streamOut, TEST_INTERN_PREFIX
           "REQ_CONFIG[%02d-%02d]: BOOT ARC2ME - addr 0x%08X size 0x%X (drv size 0x%X)\n\r",
           pReqCfg->devNum, pReqCfg->currOpenInst,
           pReqCfg->Arc2MeBootMbAddr, pReqCfg->Arc2MeBootMbSize, pReqCfg->drvArc2MeMbSize);

   fprintf(streamOut, TEST_INTERN_PREFIX
           "REQ_CONFIG[%02d-%02d]: BOOT ME2ARC - addr 0x%08X size 0x%X (drv size 0x%X)\n\r",
           pReqCfg->devNum, pReqCfg->currOpenInst,
           pReqCfg->Me2ArcBootMbAddr, pReqCfg->Me2ArcBootMbSize, pReqCfg->drvMe2ArcMbSize);

   fprintf(streamOut, TEST_INTERN_PREFIX
           "REQ_CONFIG[%02d-%02d]: BootMode %d, ChipId %d\n\r",
           pReqCfg->devNum, pReqCfg->currOpenInst,
           pReqCfg->bootMode, pReqCfg->chipId);

   fprintf(streamOut, TEST_INTERN_PREFIX
           "REQ_CONFIG[%02d-%02d]: Online ARC2ME - addr 0x%08X size 0x%X (drv size 0x%X)\n\r",
           pReqCfg->devNum, pReqCfg->currOpenInst,
           pReqCfg->Arc2MeOnlineMbAddr, pReqCfg->Arc2MeOnlineMbSize, pReqCfg->drvArc2MeMbSize);

   fprintf(streamOut, TEST_INTERN_PREFIX
           "REQ_CONFIG[%02d-%02d]: Online ME2ARC - addr 0x%08X size 0x%X (drv size 0x%X)\n\r",
           pReqCfg->devNum, pReqCfg->currOpenInst,
           pReqCfg->Me2ArcOnlineMbAddr, pReqCfg->Me2ArcOnlineMbSize, pReqCfg->drvMe2ArcMbSize);


   /*
      request statistic data
   */
   memset(pArgs, 0x00, sizeof(IOCTL_MEI_TEST_internArg_t));

   retVal = ioctl(fd, FIO_MEI_INTERN_REQ_STAT, (MEI_TEST_INTERN_IOCTL_ARG)pReqStat);
   if (retVal < 0)
   {
      /* ERROR while ioctl */
      fprintf( streamOut, TEST_INTERN_PREFIX
               "ERROR - request MEI statistics, ret = %d, retCode = %d\n\r",
                retVal, pReqStat->ictl.retCode);
      return -1;
   }

   fprintf( streamOut, TEST_INTERN_PREFIX
            "ioct(FIO_MEI_REQ_STAT) ===================\n\r");

   fprintf( streamOut, TEST_INTERN_PREFIX
            "DrvSwRst   = %d\tMeiHwRst    = %d\n\r",
            pReqStat->drvSwRstCount, pReqStat->meiHwRstCount);
   fprintf( streamOut, TEST_INTERN_PREFIX
            "GP1 Int    = %d\tMsgAv Int   = %d\n\r",
            pReqStat->dfeGp1IntCount, pReqStat->dfeMsgAvIntCount);
   fprintf( streamOut, TEST_INTERN_PREFIX
            "FwDownl    = %d\tCodeSwap    = %d\n\r",
            pReqStat->fwDlCount, pReqStat->dfeCodeSwapCount);
   fprintf( streamOut, TEST_INTERN_PREFIX
            "FwDownlErr = %d\n\r",
            pReqStat->fwDlErrCount);
   fprintf( streamOut, TEST_INTERN_PREFIX
            "TxMsg      = %d\tRxAck       = %d\n\r",
            pReqStat->sendMsgCount, pReqStat->recvAckCount);
   fprintf( streamOut, TEST_INTERN_PREFIX
            "RxMsg      = %d\tRxMsgDisc   = %d\n\r",
            pReqStat->recvMsgCount, pReqStat->recvMsgDiscardCount);
   fprintf( streamOut, TEST_INTERN_PREFIX
            "RxMsgErr   = %d\tTxMsgErr    = %d\n\r",
            pReqStat->recvMsgErrCount, pReqStat->errorCount);
   fprintf( streamOut, TEST_INTERN_PREFIX
            "Nfc        = %d\tNfcDisc     = %d\n\r",
            pReqStat->recvNfcCount, pReqStat->recvNfcDiscardCount);
   fprintf( streamOut, TEST_INTERN_PREFIX
            "NfcDist    = %d\tNfcDistDisc = %d\n\n\r",
            pReqStat->recvNfcDistCount, pReqStat->recvNfcDistDiscardCount);


   return 0;
}

/**
   test case: Set MEI Reg
              "(-S offset, -0 value)"
*/
static int MEI_TEST_SetMeiReg(FILE *streamOut, int fd, IOCTL_MEI_TEST_internArg_t *pArgs)
{
   int retVal = -1;

   pArgs->intern_regIo.addr  = (unsigned int)bTestInternSetMeiReg;
   pArgs->intern_regIo.value = (unsigned int)bTestInternParams[0];

   fprintf( streamOut, TEST_INTERN_PREFIX
            "Set Byte Off - MEI Reg[0x%04X] = 0x%04X\n\r",
             pArgs->intern_regIo.addr, pArgs->intern_regIo.value);

   retVal = ioctl(fd, FIO_MEI_INTERN_REG_SET, (MEI_TEST_INTERN_IOCTL_ARG)pArgs);
   if (retVal < 0)
   {
      /* ERROR while ioctl */
      fprintf( streamOut, TEST_INTERN_PREFIX
               "ERROR - set MEI Reg, ret = %d, retCode = %d\n\r",
                retVal, pArgs->intern_regIo.ictl.retCode);
      return -1;
   }

   return 0;
}

/**
   test case: Get MEI Reg
              "(-s offset)"
*/
static int MEI_TEST_GetMeiReg(FILE *streamOut, int fd, IOCTL_MEI_TEST_internArg_t *pArgs)
{
   int retVal = -1;

   pArgs->intern_regIo.addr  = (unsigned int)bTestInternGetMeiReg;

   retVal = ioctl(fd, FIO_MEI_INTERN_REG_GET, (MEI_TEST_INTERN_IOCTL_ARG)pArgs);
   if (retVal < 0)
   {
      /* ERROR while ioctl */
      fprintf( streamOut, TEST_INTERN_PREFIX
               "ERROR - Get MEI Reg, ret = %d, retCode = %d\n\r",
                retVal, pArgs->intern_regIo.ictl.retCode);
      return -1;
   }

   fprintf( streamOut, TEST_INTERN_PREFIX
            "Get Byte Off - MEI Reg[0x%04X] = 0x%04X\n\r",
             pArgs->intern_regIo.addr, pArgs->intern_regIo.value);

   return 0;
}

/**
   test case: Set MEI Debug
              "(-M <adr> -d <dest> -0 <val>)"
*/
static int MEI_TEST_SetMeiDbg(FILE *streamOut, int fd, IOCTL_MEI_TEST_internArg_t *pArgs)
{
   int retVal = -1;
   unsigned int tempData;

   switch(bTestInternTargetDest)
   {
      case 0:
         pArgs->intern_DbgAccess.dbgDest = 0;
         break;
      case 2:
         pArgs->intern_DbgAccess.dbgDest = 2;
         break;
      case 3:
         pArgs->intern_DbgAccess.dbgDest = 3;
         break;
      default:
         return -1;
   }

   tempData = (unsigned int)bTestInternParams[0];

   pArgs->intern_DbgAccess.dbgAddr  = (unsigned int)bTestInternSetMeiDbg;
   pArgs->intern_DbgAccess.count    = 1;
   pArgs->intern_DbgAccess.pData_32 = &tempData;

   fprintf( streamOut, TEST_INTERN_PREFIX
            "Set MEI Dbg[0x%08X] = 0x%08X (%s)\n\r",
             (unsigned int)pArgs->intern_DbgAccess.dbgAddr,
             (unsigned int)(*(pArgs->intern_DbgAccess.pData_32)),
             (bTestInternTargetDest = 0) ? "AUX" : (bTestInternTargetDest = 2) ? "MEM" : "CORE"  );

   retVal = ioctl(fd, FIO_MEI_INTERN_DBG_WRITE, (MEI_TEST_INTERN_IOCTL_ARG)pArgs);
   if (retVal < 0)
   {
      /* ERROR while ioctl */
      fprintf( streamOut, TEST_INTERN_PREFIX
               "ERROR - Set MEI Dbg, ret = %d, retCode = %d\n\r",
                retVal, pArgs->intern_regIo.ictl.retCode);
      return -1;
   }

   return 0;
}

/**
   test case: Get MEI Debug
              "(-m <adr> -d <dest>)"
*/
static int MEI_TEST_GetMeiDbg(FILE *streamOut, int fd, IOCTL_MEI_TEST_internArg_t *pArgs)
{
   int retVal = -1;
   unsigned int tempData = 0xA5A5A5A5;

   switch(bTestInternTargetDest)
   {
      case 0:
         pArgs->intern_DbgAccess.dbgDest = 0;
         break;
      case 2:
         pArgs->intern_DbgAccess.dbgDest = 2;
         break;
      case 3:
         pArgs->intern_DbgAccess.dbgDest = 3;
         break;
      default:
         return -1;
   }

   pArgs->intern_DbgAccess.dbgAddr  = (unsigned int)bTestInternGetMeiDbg;
   pArgs->intern_DbgAccess.count    = 1;
   pArgs->intern_DbgAccess.pData_32 = &tempData;

   retVal = ioctl(fd, FIO_MEI_INTERN_DBG_READ, (MEI_TEST_INTERN_IOCTL_ARG)pArgs);
   if (retVal < 0)
   {
      /* ERROR while ioctl */
      fprintf( streamOut, TEST_INTERN_PREFIX
               "ERROR - Get MEI Dbg, ret = %d, retCode = %d\n\r",
                retVal, pArgs->intern_regIo.ictl.retCode);
      return -1;
   }

   fprintf( streamOut, TEST_INTERN_PREFIX
            "Get MEI Dbg[0x%08X] = 0x%08X (%s)\n\r",
             (unsigned int)pArgs->intern_DbgAccess.dbgAddr,
             (unsigned int)(*(pArgs->intern_DbgAccess.pData_32)),
             (bTestInternTargetDest = 0) ? "AUX" : (bTestInternTargetDest = 2) ? "MEM" : "CORE"  );

   return 0;
}


/**
   test case: Set GPA
              "(-A <addr> -d <dest>) -0 <value>"
*/
static int MEI_TEST_SetGpa(FILE *streamOut, int fd, IOCTL_MEI_TEST_internArg_t *pArgs)
{
   int retVal = -1;

   switch(bTestInternTargetDest)
   {
      case 0:
         pArgs->intern_GpaAccess.dest = MEI_IOCTL_GPA_DEST_AUX;
         break;
      case 2:
         pArgs->intern_GpaAccess.dest = MEI_IOCTL_GPA_DEST_MEM;
         break;
      default:
         return -1;
   }

   pArgs->intern_GpaAccess.addr  = (unsigned int)bTestInternSetGpaReg;
   pArgs->intern_GpaAccess.value = (unsigned int)bTestInternParams[0];

   fprintf( streamOut, TEST_INTERN_PREFIX
            "Set MEI Dbg[0x%08X] = 0x%08X (%s)\n\r",
             pArgs->intern_GpaAccess.addr, pArgs->intern_GpaAccess.value,
             (bTestInternTargetDest = 0) ? "AUX" : "MEM" );

   retVal = ioctl(fd, FIO_MEI_INTERN_GPA_WRITE, (MEI_TEST_INTERN_IOCTL_ARG)pArgs);
   if (retVal < 0)
   {
      /* ERROR while ioctl */
      fprintf( streamOut, TEST_INTERN_PREFIX
               "ERROR - Set GPA, ret = %d, retCode = %d\n\r",
                retVal, pArgs->intern_GpaAccess.ictl.retCode);
      return -1;
   }

   return 0;
}

/**
   test case: Get GPA
              "(-a <addr> -d <dest>)"
*/
static int MEI_TEST_GetGpa(FILE *streamOut, int fd, IOCTL_MEI_TEST_internArg_t *pArgs)
{
   int retVal = -1;

   switch(bTestInternTargetDest)
   {
      case 0:
         pArgs->intern_GpaAccess.dest = MEI_IOCTL_GPA_DEST_AUX;
         break;
      case 2:
         pArgs->intern_GpaAccess.dest = MEI_IOCTL_GPA_DEST_MEM;
         break;
      default:
         return -1;
   }

   pArgs->intern_GpaAccess.addr  = (unsigned int)bTestInternGetGpaReg;

   retVal = ioctl(fd, FIO_MEI_INTERN_GPA_READ, (MEI_TEST_INTERN_IOCTL_ARG)pArgs);
   if (retVal < 0)
   {
      /* ERROR while ioctl */
      fprintf( streamOut, TEST_INTERN_PREFIX
               "ERROR - Get GPA, ret = %d, retCode = %d\n\r",
                retVal, pArgs->intern_GpaAccess.ictl.retCode);
      return -1;
   }

   fprintf( streamOut, TEST_INTERN_PREFIX
            "Get MEI Dbg[0x%08X] = 0x%08X (%s)\n\r",
             pArgs->intern_GpaAccess.addr, pArgs->intern_GpaAccess.value,
             (bTestInternTargetDest = 0) ? "AUX" : "MEM" );

   return 0;
}


/*
   Set the msg arguments for send message
*/
static void MEI_TEST_SetupSendMsg(
                     IOCTL_MEI_message_t *pMsg,
                     unsigned char *pBuf,
                     unsigned short msgID, int *pParamArr)
{
   int i;

   unsigned int   *pUInt32Buf = (unsigned int *)pBuf;
   unsigned short *pUInt16Buf = (unsigned short *)pBuf;

   /* index + length */
   pUInt16Buf[0] = (unsigned short)pParamArr[0];
   pUInt16Buf[1] = (unsigned short)pParamArr[1];


   /* ifx msg (params without index and length) */
   for (i = 0; i < MEI_TEST_INTERN_MAX_OPT_PARAMS - 2; i++)
   {
      if(pParamArr[i+2] == -1)
         break;

      if ( (msgID & 0x0010) )
      {
         pUInt32Buf[i+1] = (unsigned int)pParamArr[i+2];
      }
      else
         pUInt16Buf[i+2] = (unsigned short)pParamArr[i+2];
   }

   pMsg->msgId = msgID;
   pMsg->msgClassifier = 0;

   /* Add index + length field */
   pMsg->paylSize_byte = 2 * sizeof(unsigned short);

   /* Add payload data (16/32 bit) */
   if ( (msgID & 0x0010) )
      pMsg->paylSize_byte += (i * sizeof(unsigned int));
   else
      pMsg->paylSize_byte += (i * sizeof(unsigned short));

   pMsg->pPayload = pBuf;

   return;
}


/**
   Print CMV Message Header.
*/
void MEI_TEST_LogIfxMsg(FILE *streamOut, char *pLogStr, IOCTL_MEI_message_t *pCntrl)
{
   int i;
   unsigned short *pPayl_16bit;

   fprintf( streamOut, TEST_INTERN_PREFIX
            "%s, IFX-Msg - Ctrl: 0x%08X, Class: 0x%04X, ID: 0x%04X, size: 0x%04X (%d)\n\r",
            (pLogStr) ? pLogStr : "Show",
            pCntrl->msgCtrl, pCntrl->msgClassifier, pCntrl->msgId,
            pCntrl->paylSize_byte, pCntrl->paylSize_byte);

   switch (pCntrl->msgCtrl)
   {
      case MEI_MSG_CTRL_DRIVER_MSG:
         fprintf( streamOut, TEST_INTERN_PREFIX
                  "Driver Message not supported\n\r");
         break;

      case MEI_MSG_CTRL_MODEM_MSG:
      default:
         if (pCntrl->paylSize_byte)
         {
            pPayl_16bit = (unsigned short *)pCntrl->pPayload;

            fprintf( streamOut, TEST_INTERN_PREFIX
                     "DATA - \n\r");
            for (i=0; i < pCntrl->paylSize_byte/2;i++)
            {
               fprintf( streamOut, TEST_INTERN_PREFIX
                        "\t[%2d]: 0x%04X 0x%04X\n\r",
                        i, pPayl_16bit[i],
                        ((pCntrl->paylSize_byte/2) > (i+1)) ? pPayl_16bit[i+1]:0);
               i++;
            }
         }
         break;
   }

   return;
}


/* mailbox write / read buffer */
CMV_MESSAGE_ALL_T MEI_TEST_CmdMsg;
CMV_MESSAGE_ALL_T MEI_TEST_AckMsg;


/**
   test case: Send Vrx Msg
*/
static int MEI_TEST_SendMsg(FILE *streamOut, int fd, IOCTL_MEI_TEST_internArg_t *pArgs)
{
   int retVal = -1;
   unsigned short msgID = (unsigned short)bTestInternSendMsg;

   IOCTL_MEI_messageSend_t *pMsg    = &pArgs->intern_ifxMsgSend;
   IOCTL_MEI_message_t     *pMsgCmd = &pMsg->write_msg;
   IOCTL_MEI_message_t     *pMsgAck = &pMsg->ack_msg;


   /* setup the send msg args */
   MEI_TEST_SetupSendMsg( pMsgCmd, (unsigned char *)MEI_TEST_CmdMsg.rawMsg,
                          msgID, bTestInternParams);

   /* setup the ack buffer */
   pMsgAck->pPayload      = (unsigned char *)MEI_TEST_AckMsg.rawMsg;
   pMsgAck->paylSize_byte = sizeof(MEI_TEST_AckMsg.rawMsg);

   MEI_TEST_LogIfxMsg(streamOut, "Send", pMsgCmd);

   retVal = ioctl(fd, FIO_MEI_INTERN_MBOX_MSG_SEND, (MEI_TEST_INTERN_IOCTL_ARG)pMsg);
   if (retVal < 0)
   {
      /* ERROR while ioctl */
      fprintf( streamOut, TEST_INTERN_PREFIX
               "ERROR - send Message, ret = %d, retCode = %d\n\r",
                retVal, pMsg->ictl.retCode);
      return -1;
   }

   MEI_TEST_LogIfxMsg(streamOut, "ACK ", pMsgAck);

   return 0;
}


int main(int argc, char *argv[])
{
   FILE *streamOut = stdout;

   parseArgs(argc,argv);

   if( bTestInternHelp == 1)
   {
      doHelp();

      /* PrintoutMem(0x12345678); */
      return EXIT_SUCCESS;
   }

   MEI_TEST_Intern(streamOut);

   return 0;

}



#endif /* LINUX */


