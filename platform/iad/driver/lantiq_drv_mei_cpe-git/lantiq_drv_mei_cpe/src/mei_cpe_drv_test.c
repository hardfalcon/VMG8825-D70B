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
#include "mei_cpe_appl_osmap.h"

#ifdef LINUX

#include <unistd.h>
#define _GNU_SOURCE
#include <getopt.h>


/** get interface and configuration */
#include "drv_mei_cpe_interface.h"

/** get the CMV message format */
#include "cmv_message_format.h"

/* get common test routines */
#include "mei_cpe_drv_test_fct.h"


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
void MEI_drv_test(void);

/* ============================================================== */
/* Local Variables (used for arguments)                           */
/* ============================================================== */
static int bTestSetGpaReg     = -1;    /* a */
static int bTestGetGpaReg     = -1;    /* A */

static int bTestDisplBuf      = -1;    /* b/B */

static int bReqCfg            = -1;    /* c */
static int bBusMasterChipId   = -1;    /* C */

static int bTestDisplMeiRegs  = -1;    /* d */
static int bMeiDbgDest        = -1;    /* D */

static int bTestFwDl          = -1;    /* F */
static int bTestFwSwap        = -1;    /* f */
static int bTestSetModeXDSL   = -1;    /* e */
static int bTestGetMeiReg     = -1;    /* g */

static int bTestHelp          = -1;    /* h */
static int bTestInitDev       = -1;    /* i */
static int bTestInitDrv       = -1;    /* T */

/* -1: not set, 0: disable, 1: enable */
static int bTestMbLoop        = -1;    /* l/L */

static int bMeiDbgWrOff       = -1;    /* m */
static int bMeiDbgRdOff       = -1;    /* M */

static int bTestDfeChannel    = -1;    /* n */
static int bTestNfcInitDev    = -1;    /* N */

static int bOptValTest        = -1;    /* o */
static int bTestMbRead        = -1;    /* r */
static int bDrvReset          = -1;    /* R */

static int bDrvPrintOutTr     = -1;    /* p */

static int bTestSetMeiReg     = -1;    /* s */
static int bTestSetMeiValue   = -1;    /* S */

static int bTroubleTest       = -1;    /* t */

static int bTestGetVer        = -1;    /* v */
static int bTestMbSend        = -1;    /* w */
static int bTestSendMsg       = -1;    /* W */
static int testValue          = -1;    /* x */
static int bTestSetGpaDest    = -1;    /* X */

static int bDMAReadTest       = -1;    /* y */
static int bDMAWriteTest      = -1;    /* Y */

static int bTestDsmCnfGet     = -1;    /* k */
static int bTestDsmCnfSet     = -1;    /* K */
static int bTestDsmStsGet     = -1;    /* j */
static int bTestDsmStcGet     = -1;    /* J */
static int bTestDsmMacGet     = -1;    /* q */
static int bTestDsmMacSet     = -1;    /* Q */

static int bDbgLevelSet       = -1;    /* G */

static int bPllOffsetSet      = -1;    /* P */
static int bPllOffsetGet      = -1;    /* O */

static int bParams[MEI_TEST_MAX_OPT_PARAMS] =
   {-1, -1, -1, -1, -1, -1, -1, -1};  /* 0, 1, 2, 3, 4, 5, 6, 7 */

#define MEI_TEST_MAX_STR_PARAMS   1
#define MEI_TEST_STR_PARAM_LEN    256

static char argStrParam[MEI_TEST_MAX_STR_PARAMS][MEI_TEST_STR_PARAM_LEN] =
   { {"\0"} };

/* ============================================================== */
/* Local variables for parse and interprete arguments             */
/* ============================================================== */
static void GetOptArg_RequStr(char *pOptArg, char *pDest, char *pDesc);
static void parseArgs(int argc, char *argv[]);
static void doHelp(void);

/*
   0 no_argument
   1 required_argument
   2 optional_argument
*/

static struct option long_options[] =
{
/*  0 */   {"help    ", no_argument,        NULL, 'h'},
/*  1 */   {"version ", no_argument,        NULL, 'v'},
/*  2 */   {"Config  ", no_argument,        NULL, 'c'},
/*  3 */   {"trace ch", required_argument,  NULL, 'p'},
/*  4 */   {"disp_mei", no_argument,        NULL, 'd'},
/*  5 */   {"init_dev", required_argument,  NULL, 'i'},
/*  6 */   {"init_drv", required_argument,  NULL, 'T'},
/*  7 */   {"dfe_num ", required_argument,  NULL, 'n'},
/*  8 */   {"nfc_idev", required_argument,  NULL, 'N'},
/*  9 */   {"set_reg ", required_argument,  NULL, 's'},
/* 10 */   {"set_val ", required_argument,  NULL, 'S'},
/* 11 */   {"get_reg ", required_argument,  NULL, 'g'},
/* 12 */   {"loop_on ", no_argument,        NULL, 'L'},
/* 13 */   {"loop_off", no_argument,        NULL, 'l'},
/* 14 */   {"mbraw_send", optional_argument, NULL, 'w'},
/* 15 */   {"mbraw_rd", optional_argument,  NULL, 'r'},
/* 16 */   {"value   ", required_argument,  NULL, 'x'},
/* 17 */   {"disp_rd ", required_argument,  NULL, 'b'},
/* 18 */   {"disp_wr ", no_argument,        NULL, 'B'},
/* 19 */   {"dbg_wr  ", required_argument,  NULL, 'm'},
/* 20 */   {"dbg_rd  ", required_argument,  NULL, 'M'},
/* 21 */   {"dbg_dest", required_argument,  NULL, 'D'},
/* 22 */   {"drv_reset", required_argument, NULL, 'R'},
/* 23 */   {"fw_downl", no_argument,        NULL, 'F'},
/* 24 */   {"fw_swap ", required_argument,  NULL, 'f'},
/* 25 */   {"set_xdsl", required_argument,  NULL, 'e'},
/* 26 */   {"chip_id ", required_argument,  NULL, 'C'},
/* 27 */   {"gpa_set ", required_argument,  NULL, 'a'},
/* 28 */   {"gpa_get ", required_argument,  NULL, 'A'},
/* 29 */   {"gpa_dest", required_argument,  NULL, 'X'},
/* 30 */   {"mb_send ", optional_argument,  NULL, 'W'},
/* 31 */   {"dma_stress", required_argument,NULL, 't'},
/* 32 */   {"dma_read",  required_argument, NULL, 'y'},
/* 33 */   {"dma_write", required_argument, NULL, 'Y'},
/* 34 */   {"opt_val  ", required_argument, NULL, 'o'},
/* 35 */   {"dsm_cnfg_get ", no_argument, NULL, 'k'},
/* 36 */   {"dsm_cnfg_set ", no_argument, NULL, 'K'},
/* 37 */   {"dsm_status   ", no_argument, NULL, 'j'},
/* 38 */   {"dsm_statistics", no_argument, NULL, 'J'},
/* 39 */   {"MAC_get  ", no_argument, NULL, 'q'},
/* 40 */   {"MAC_set  ", no_argument, NULL, 'Q'},
/* 41 */   {"dbg_lvl_set", no_argument, NULL, 'G'},
/* 42 */   {"pll_offset_set", required_argument, NULL, 'P'},
/* 43 */   {"pll_offset_get", no_argument, NULL, 'O'},
/* 44 */   {"param_0  ", required_argument, NULL, '0'},
/* 45 */   {"param_1  ", required_argument, NULL, '1'},
/* 46 */   {"param_2  ", required_argument, NULL, '2'},
/* 47 */   {"param_3  ", required_argument, NULL, '3'},
/* 48 */   {"param_4  ", required_argument, NULL, '4'},
/* 49 */   {"param_5  ", required_argument, NULL, '5'},
/* 50 */   {"param_6  ", required_argument, NULL, '6'},
/* 51 */   {"param_7  ", required_argument, NULL, '7'},
/* 52 */   {"str_z    ", required_argument, NULL, 'z'},
           {NULL, 0, NULL, 0}
};


/*
                                             1 1              2 2
                               01234 5 6 789 0 1 234 5 6 7 89 0 1 2 34 5 6 7 8 9  */
#define GETOPT_LONG_OPTSTRING "hvcp:di:Tn:N:s:S:g:Llw:r:x:b:Bm:M:D:R:Ff:eC:a:A:X:W:t:y:Y:o:kK:jJqQGP:O0:1:2:3:4:5:6:7:z:"

static char description[][64] =
{
/*  0 */   {"help screen"},
/*  1 */   {"driver version"},
/*  2 */   {"MEI configuration"},
/*  3 */   {"MEI trace level"},
/*  4 */   {"display MEI Register"},
/*  5 */   {"init the device (MEI base Addr, -o IRQ), not sup. VR10"},
/*  6 */   {"init the driver (blk tout, msg tout, MR tout)"},
/*  7 */   {"MEI number (-n <devNum>, default: 0)"},
/*  8 */   {"init the devive for NFC (base Addr, -o IRQ)"},
/*  9 */   {"set MEI register (-s offset, -S value)"},
/* 10 */   {"set MEI value (see -s)"},
/* 11 */   {"get MEI register (-g offset)"},
/* 12 */   {"switch loop ON"},
/* 13 */   {"switch loop OFF"},
/* 14 */   {"mbox raw send (count, none=default)"},
/* 15 */   {"mbox raw read ack (count, none=default"},
/* 16 */   {"common used value (write start pattern)"},
/* 17 */   {"display read buffer (buffer number)"},
/* 18 */   {"display write buffer"},
/* 19 */   {"mei dbg access write (-m <adr> -D <dest> -x <cnt> -0 <val>)"},
/* 20 */   {"mei dbg access read  (-M <adr> -D <dest> -x <cnt>)"},
/* 21 */   {"mei dbg access destination AUX=0, LDST=2, CORE=3"},
/* 22 */   {"Drv Reset -R <MMMMRRRR> (RstMask:MMMM,RstMode:RRRR)"},
/* 23 */   {"FW Download [-o <0/1>] firmware<#>.bin) [-z <name>/<null>]"},
/* 24 */   {"FW Swap -f <mode> (0: VDSL2, 1: ADSL)"},
/* 25 */   {"Set mode (xdsl<0/1>, id<5/7>, fw_xdsl<0|1|2|4|8>) [-z <...>]"},
/* 26 */   {"Set BusMaster -C <chipId> -0 <MEI sel mask>"},
/* 27 */   {"GPA access set (-a <addr> -x <value> -X <dest>)"},
/* 28 */   {"GPA access get (-A <addr> -X <dest>)"},
/* 29 */   {"GPA access destination MEM=0, AUX=1"},
/* 30 */   {"mbox send (-W msgID, [-0 <param> -1 <param> ...] )"},
/* 31 */   {"DMA Stress test (-t <addr>, -x <range>, -D <loop>)"},
/* 32 */   {"DMA Read  (32Bit) (-y <addr>, -X <count 32 bit>)"},
/* 33 */   {"DMA Write (32Bit) (-Y <addr>, -x <value> -X <count 32 bit>)"},
/* 34 */   {"optional value (-o irq | entity)"},
/* 35 */   {"Get DSM config (vector_control)"},
/* 36 */   {"Set DSM config (vector_control<0/1/2>)"},
/* 37 */   {"Get DSM status (status, friendly_status)"},
/* 38 */   {"Get DSM statistic (processed, dropped)"},
/* 39 */   {"Get MAC address (XX:XX:XX:XX:XX:XX)"},
/* 40 */   {"Set MAC address (XX:XX:XX:XX:XX:XX) [-z <address>]"},
/* 41 */   {"Set debug level (module <1/2/3>, level <1/2/3/4>) -z <...>"},
/* 42 */   {"set PLL offset (dec) [-32768..32767], 32768 (ignore PLL)"},
/* 43 */   {"get PLL offset (dec)"},
/* 44 */   {"message param 0"},
/* 45 */   {"message param 1"},
/* 46 */   {"message param 2"},
/* 47 */   {"message param 3"},
/* 48 */   {"message param 4"},
/* 49 */   {"message param 5"},
/* 50 */   {"message param 6"},
/* 51 */   {"message param 7"},
/* 52 */   {"arg string z"},
           {0}
};


/* ============================================================== */
/* Local Function to parse and interprete arguments               */
/* ============================================================== */

/*
   get an argument
*/
static void GetOptArg_RequStr(char *pOptArg, char *pDest, char *pDesc)
{
   unsigned long strLength;

   if (optarg)
   {
      if ( (strLength = MEIOS_StrLen(optarg)) > 0)
      {
         MEIOS_MemSet(pDest, 0x00, MEI_TEST_STR_PARAM_LEN);
         MEIOS_StrNCpy(pDest, (char *)optarg,
                 (strLength > (MEI_TEST_STR_PARAM_LEN -1)) ?
                        (MEI_TEST_STR_PARAM_LEN -1) : strLength);
         MEIOS_Printf( TEST_MEI_DBG_PREFIX
                 "%s: %s" MEIOS_CRLF, pDesc, pDest);
      }
   }
   else
   {
      MEIOS_Printf( TEST_MEI_DBG_PREFIX
              "%s: missing argument" MEIOS_CRLF, pDesc);
   }

   return;
}


static void parseArgs(int argc, char *argv[])
{
   unsigned long temp;
   int parm_no=0;
   char *pEndPtr ;

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
            bTestHelp = 1;
         break;
      }

      parm_no++;

      switch(c)
      {
         case 'h':
            bTestHelp = 1;
            break;
         case 'v':
            bTestGetVer = 1;
            break;
         case 'c':
            bReqCfg = 1;
            break;
         case 'd':
            bTestDisplMeiRegs = 1;
            break;
         case 'i':
            bTestInitDev = 1;

            if (optarg)
            {
               MEI_DataDevInit.meiBaseAddr = MEIOS_StrToUl(optarg, &pEndPtr, 0x10);
               if (MEI_DataDevInit.meiBaseAddr)
               {
                  MEIOS_Printf(TEST_MEI_DBG_PREFIX"init dev: base address = 0x%08X" MEIOS_CRLF,
                               (unsigned int)MEI_DataDevInit.meiBaseAddr);
               }
            }
            break;
         case 'T':
            bTestInitDrv = 1;
            break;

         case 'n':
            if (optarg)
            {
               temp = MEIOS_StrToUl(optarg, &pEndPtr, 0x10);
               if (errno)
               {
                  MEIOS_Printf(TEST_MEI_DBG_PREFIX"MEI Channel: invalid,  0x%08X" MEIOS_CRLF, (unsigned int)temp);
               }
               else
               {
                  bTestDfeChannel = (int)temp;
                  /* MEIOS_Printf(TEST_MEI_DBG_PREFIX"MEI Channel: 0x%08X" MEIOS_CRLF, (unsigned int)temp); */
               }
            }
            break;

         case 'N':
            bTestNfcInitDev = 1;

            if (optarg)
            {
               MEI_DataDevInit.meiBaseAddr = MEIOS_StrToUl(optarg, &pEndPtr, 0x10);
               if (MEI_DataDevInit.meiBaseAddr)
               {
                  MEIOS_Printf(TEST_MEI_DBG_PREFIX"NFC init dev: base address = 0x%08X, IRQ = 0x%X (%d)" MEIOS_CRLF,
                               (unsigned int)MEI_DataDevInit.meiBaseAddr,
                               (unsigned int)MEI_DataDevInit.usedIRQ,
                               (unsigned int)MEI_DataDevInit.usedIRQ);
               }
            }
            break;

         case 'L':
            bTestMbLoop = 1;
            break;

         case 'l':
            bTestMbLoop = 0;
            break;
         case 's':
            if (optarg)
            {
               temp = MEIOS_StrToUl(optarg, &pEndPtr, 0x10);
               if (errno)
               {
                  MEIOS_Printf(TEST_MEI_DBG_PREFIX"MEI Offset Addr: invalid,  0x%08X" MEIOS_CRLF, (unsigned int)temp);
               }
               else
               {
                  bTestSetMeiReg = (int)temp;
                  MEIOS_Printf(TEST_MEI_DBG_PREFIX"MEI Offset Addr: 0x%08X" MEIOS_CRLF, (unsigned int)temp);
               }
            }
            break;

         case 'S':
            if (optarg)
            {
               temp = MEIOS_StrToUl(optarg, &pEndPtr, 0x10);
               if (errno)
               {
                  MEIOS_Printf(TEST_MEI_DBG_PREFIX"MEI Set Value: invalid,  0x%08X" MEIOS_CRLF,(unsigned int)temp);
               }
               else
               {
                  bTestSetMeiValue = (int)temp;
                  MEIOS_Printf(TEST_MEI_DBG_PREFIX"MEI Set Value: 0x%08X" MEIOS_CRLF, (unsigned int)temp);
               }
            }
            break;

         case 'g':
            if (optarg)
            {
               temp = MEIOS_StrToUl(optarg, &pEndPtr, 0x10);
               if (errno)
               {
                  MEIOS_Printf(TEST_MEI_DBG_PREFIX"MEI Offset Addr: invalid,  0x%08X" MEIOS_CRLF, (unsigned int)temp);
               }
               else
               {
                  bTestGetMeiReg = (int)temp;
                  MEIOS_Printf(TEST_MEI_DBG_PREFIX"MEI Offset Addr: 0x%08X" MEIOS_CRLF, (unsigned int)temp);
               }
            }
            break;

         case 'w':
            if (optarg)
            {
               temp = MEIOS_StrToUl(optarg, &pEndPtr, 0x10);
               if (temp)
               {
                  MEIOS_Printf(TEST_MEI_DBG_PREFIX"parse mb send: mailbox count = %d" MEIOS_CRLF, (unsigned int)temp);
                  bTestMbSend = temp;
               }
               else
               {
                  MEIOS_Printf(TEST_MEI_DBG_PREFIX"parse  mb send: invalid mailbox count = %d" MEIOS_CRLF, (unsigned int)temp);
                  bTestMbSend = CMV_MESSAGE_SIZE;
               }
            }
            else
            {
               bTestMbSend = CMV_MESSAGE_SIZE;
               MEIOS_Printf(TEST_MEI_DBG_PREFIX"parse  mb send: missing mailbox count = %d" MEIOS_CRLF, bTestMbSend);
            }
            break;

         case 'r':
            if (optarg)
            {
               temp = MEIOS_StrToUl(optarg, &pEndPtr, 0x10);
               if (temp)
               {
                  MEIOS_Printf(TEST_MEI_DBG_PREFIX"parse read: mailbox count = %d" MEIOS_CRLF, (unsigned int)temp);
                  bTestMbRead = temp;
               }
               else
               {
                  MEIOS_Printf("parse read: invalid mailbox count = %d" MEIOS_CRLF, (unsigned int)temp);
                  bTestMbRead = CMV_MESSAGE_SIZE;
               }

            }
            else
            {
               bTestMbRead = CMV_MESSAGE_SIZE;
               MEIOS_Printf(TEST_MEI_DBG_PREFIX"parse read: missing mailbox count = %d" MEIOS_CRLF, bTestMbRead);
            }
            break;


         case 'o':
            if (optarg)
            {
               temp = MEIOS_StrToUl(optarg, &pEndPtr, 0x10);
               if (errno != ERANGE)
               {
                  MEIOS_Printf(TEST_MEI_DBG_PREFIX"parse: opt value = %d" MEIOS_CRLF, (unsigned int)temp);
                  bOptValTest = temp;
               }
               else
               {
                  MEIOS_Printf(TEST_MEI_DBG_PREFIX"parse: opt value = %d <invalid> " MEIOS_CRLF, (unsigned int)temp);
               }

            }
            break;

         case 'x':
            if (optarg)
            {
               temp = MEIOS_StrToUl(optarg, &pEndPtr, 0x10);
               if (errno)
               {
                  MEIOS_Printf(TEST_MEI_DBG_PREFIX"Value: invalid,  0x%08X" MEIOS_CRLF, (unsigned int)temp);
               }
               else
               {
                  testValue = (int)temp;
                  MEIOS_Printf(TEST_MEI_DBG_PREFIX"Value: 0x%08X" MEIOS_CRLF, (unsigned int)temp);
               }
            }
            break;

         case 'b':
            if (optarg)
            {
               temp = MEIOS_StrToUl(optarg, &pEndPtr, 0x10);
               if (errno)
               {
                  MEIOS_Printf(TEST_MEI_DBG_PREFIX"Value: invalid,  0x%08X" MEIOS_CRLF, (unsigned int)temp);
               }
               else
               {
                  bTestDisplBuf = (unsigned char)(temp & 0x000000007F);
                  MEIOS_Printf(TEST_MEI_DBG_PREFIX"Value: 0x%08X" MEIOS_CRLF, (unsigned int)temp);
               }
            }
            break;
         case 'B':
            bTestDisplBuf = 0x80;
            break;
         case 'm':
            if (optarg)
            {
               temp = MEIOS_StrToUl(optarg, &pEndPtr, 0x10);
               if (errno)
               {
                  MEIOS_Printf(TEST_MEI_DBG_PREFIX"MEI DBG WrOff: invalid,  0x%08X" MEIOS_CRLF, (unsigned int)temp);
               }
               else
               {
                  bMeiDbgWrOff = (int)temp;
                  MEIOS_Printf(TEST_MEI_DBG_PREFIX"MEI DBG WrOff: 0x%08X" MEIOS_CRLF, (unsigned int)temp);
               }
            }
            break;

         case 'M':
            if (optarg)
            {
               temp = MEIOS_StrToUl(optarg, &pEndPtr, 0x10);
               if (errno)
               {
                  MEIOS_Printf(TEST_MEI_DBG_PREFIX"MEI DBG RdOff: invalid,  0x%08X" MEIOS_CRLF, (unsigned int)temp);
               }
               else
               {
                  bMeiDbgRdOff = (int)temp;
                  MEIOS_Printf("MEI DBG RdOff: 0x%08X" MEIOS_CRLF, (unsigned int)temp);
               }
            }
            break;

         case 'D':
            if (optarg)
            {
               temp = MEIOS_StrToUl(optarg, &pEndPtr, 0x10);
               if (errno)
               {
                  MEIOS_Printf(TEST_MEI_DBG_PREFIX"MEI DBG Dest: invalid,  0x%08X" MEIOS_CRLF, (unsigned int)temp);
               }
               else
               {
                  bMeiDbgDest = (int)temp;
                  MEIOS_Printf(TEST_MEI_DBG_PREFIX"MEI DBG Dest: 0x%08X" MEIOS_CRLF, (unsigned int)temp);
               }
            }
            break;

         case 'R':
            if (optarg)
            {
               temp = MEIOS_StrToUl(optarg, &pEndPtr, 0x10);
               if (errno)
               {
                  MEIOS_Printf(TEST_MEI_DBG_PREFIX"Drv Reset: invalid,  0x%08X" MEIOS_CRLF, (unsigned int)temp);
               }
               else
               {
                  bDrvReset = (int)temp;
                  MEIOS_Printf(TEST_MEI_DBG_PREFIX"Drv Reset: 0x%08X" MEIOS_CRLF, (unsigned int)temp);
               }
            }
            break;

         case 'p':
            if (optarg)
            {
               temp = MEIOS_StrToUl(optarg, &pEndPtr, 0x10);
               if (errno)
               {
                  MEIOS_Printf(TEST_MEI_DBG_PREFIX"Drv PrintOut: invalid,  0x%08X" MEIOS_CRLF, (unsigned int)temp);
               }
               else
               {
                  bDrvPrintOutTr = (int)temp;
                  MEIOS_Printf(TEST_MEI_DBG_PREFIX"Drv PrintOut (trace): 0x%08X" MEIOS_CRLF, (unsigned int)temp);
               }
            }
            break;

         case 'C':
            if (optarg)
            {
               temp = MEIOS_StrToUl(optarg, &pEndPtr, 0x10);
               if (errno)
               {
                  MEIOS_Printf(TEST_MEI_DBG_PREFIX"BusMaster set: invalid,  0x%08X" MEIOS_CRLF,
                         (unsigned int)temp);
               }
               else
               {
                  bBusMasterChipId = (int)temp;
                  /* MEIOS_Printf( TEST_MEI_DBG_PREFIX"BusMaster set: chipId = 0x%08X, mask = 0x%08X" MEIOS_CRLF,
                          (unsigned int)temp, bParams[0]); */
               }
            }
            break;



         case 'A':
            if (optarg)
            {
               temp = MEIOS_StrToUl(optarg, &pEndPtr, 0x10);
               if (errno)
               {
                  MEIOS_Printf(TEST_MEI_DBG_PREFIX"GPA access get: invalid,  0x%08X" MEIOS_CRLF, (unsigned int)temp);
               }
               else
               {
                  bTestGetGpaReg = (int)temp;
                  MEIOS_Printf(TEST_MEI_DBG_PREFIX"GPA access get: 0x%08X" MEIOS_CRLF, (unsigned int)temp);
               }
            }
            break;

         case 'a':
            if (optarg)
            {
               temp = MEIOS_StrToUl(optarg, &pEndPtr, 0x10);
               if (errno)
               {
                  MEIOS_Printf(TEST_MEI_DBG_PREFIX"GPA access set: invalid,  0x%08X" MEIOS_CRLF, (unsigned int)temp);
               }
               else
               {
                  bTestSetGpaReg = (int)temp;
                  MEIOS_Printf(TEST_MEI_DBG_PREFIX"GPA access set: 0x%08X" MEIOS_CRLF, (unsigned int)temp);
               }
            }
            break;

         case 'X':
            if (optarg)
            {
               temp = MEIOS_StrToUl(optarg, &pEndPtr, 0x10);
               if (errno)
               {
                  MEIOS_Printf(TEST_MEI_DBG_PREFIX"GPA access dest: invalid,  0x%08X" MEIOS_CRLF, (unsigned int)temp);
               }
               else
               {
                  bTestSetGpaDest = (int)temp;
                  MEIOS_Printf(TEST_MEI_DBG_PREFIX"GPA access dest: 0x%08X" MEIOS_CRLF, (unsigned int)temp);
               }
            }
            break;

         case 'W':
            if (optarg)
            {
               temp = MEIOS_StrToUl(optarg, &pEndPtr, 0x10);
               if (errno)
               {
                  MEIOS_Printf(TEST_MEI_DBG_PREFIX"Send Msg: invalid,  0x%08X" MEIOS_CRLF, (unsigned int)temp);
               }
               else
               {
                  bTestSendMsg = (int)temp;
                  MEIOS_Printf(TEST_MEI_DBG_PREFIX"Send Msg: msg ID 0x%04X" MEIOS_CRLF, (unsigned short)bTestSendMsg);
               }
            }
            break;

         case '0':
            if (optarg)
            {
               temp = MEIOS_StrToUl(optarg, &pEndPtr, 0x10);
               if (errno)
                  MEIOS_Printf(TEST_MEI_DBG_PREFIX"param[0]: invalid,  0x%08X" MEIOS_CRLF, (unsigned int)temp);
               else
                  bParams[0] = (int)temp;
            }
            break;
         case '1':
            if (optarg)
            {
               temp = MEIOS_StrToUl(optarg, &pEndPtr, 0x10);
               if (errno)
                  MEIOS_Printf(TEST_MEI_DBG_PREFIX"param[1]: invalid,  0x%08X" MEIOS_CRLF, (unsigned int)temp);
               else
                  bParams[1] = (int)temp;
            }
            break;
         case '2':
            if (optarg)
            {
               temp = MEIOS_StrToUl(optarg, &pEndPtr, 0x10);
               if (errno)
                  MEIOS_Printf(TEST_MEI_DBG_PREFIX"param[2]: invalid,  0x%08X" MEIOS_CRLF, (unsigned int)temp);
               else
                  bParams[2] = (int)temp;
            }
            break;
         case '3':
            if (optarg)
            {
               temp = MEIOS_StrToUl(optarg, &pEndPtr, 0x10);
               if (errno)
                  MEIOS_Printf(TEST_MEI_DBG_PREFIX"param[3]: invalid,  0x%08X" MEIOS_CRLF, (unsigned int)temp);
               else
                  bParams[3] = (int)temp;
            }
            break;
         case '4':
            if (optarg)
            {
               temp = MEIOS_StrToUl(optarg, &pEndPtr, 0x10);
               if (errno)
                  MEIOS_Printf(TEST_MEI_DBG_PREFIX"param[4]: invalid,  0x%08X" MEIOS_CRLF, (unsigned int)temp);
               else
                  bParams[4] = (int)temp;
            }
            break;
         case '5':
            if (optarg)
            {
               temp = MEIOS_StrToUl(optarg, &pEndPtr, 0x10);
               if (errno)
                  MEIOS_Printf(TEST_MEI_DBG_PREFIX"param[5]: invalid,  0x%08X" MEIOS_CRLF, (unsigned int)temp);
               else
                  bParams[5] = (int)temp;
            }
            break;
         case '6':
            if (optarg)
            {
               temp = MEIOS_StrToUl(optarg, &pEndPtr, 0x10);
               if (errno)
                  MEIOS_Printf(TEST_MEI_DBG_PREFIX"param[6]: invalid,  0x%08X" MEIOS_CRLF, (unsigned int)temp);
               else
                  bParams[6] = (int)temp;
            }
            break;
         case '7':
            if (optarg)
            {
               temp = MEIOS_StrToUl(optarg, &pEndPtr, 0x10);
               if (errno)
                  MEIOS_Printf(TEST_MEI_DBG_PREFIX"param[7]: invalid,  0x%08X" MEIOS_CRLF, (unsigned int)temp);
               else
                  bParams[7] = (int)temp;
            }
            break;
         case 't':
            if (optarg)
            {
               temp = MEIOS_StrToUl(optarg, &pEndPtr, 0x10);
               if (errno)
               {
                  MEIOS_Printf(TEST_MEI_DBG_PREFIX"DMA stress test: invalid,  0x%08X" MEIOS_CRLF, (unsigned int)temp);
               }
               else
               {
                  bTroubleTest = (int)temp;
               }
            }
            break;

         case 'y':
            if (optarg)
            {
               temp = MEIOS_StrToUl(optarg, &pEndPtr, 0x10);
               if (errno)
               {
                  MEIOS_Printf(TEST_MEI_DBG_PREFIX"DMA read: invalid,  0x%08X" MEIOS_CRLF, (unsigned int)temp);
               }
               else
               {
                  bDMAReadTest = (int)temp;
               }
            }
            break;

         case 'Y':
            if (optarg)
            {
               temp = MEIOS_StrToUl(optarg, &pEndPtr, 0x10);
               if (errno)
               {
                  MEIOS_Printf(TEST_MEI_DBG_PREFIX"DMA write: invalid,  0x%08X" MEIOS_CRLF, (unsigned int)temp);
               }
               else
               {
                  bDMAWriteTest = (int)temp;
               }
            }
            break;

         case 'F':
            bTestFwDl = 1;
            break;

         case 'f':
            if (optarg)
            {
               temp = MEIOS_StrToUl(optarg, &pEndPtr, 0x10);
               if (errno)
               {
                  MEIOS_Printf(TEST_MEI_DBG_PREFIX"FW Swap: invalid,  0x%08X" MEIOS_CRLF, (unsigned int)temp);
               }
               else
               {
                  bTestFwSwap = (int)temp;
               }
            }
            break;

         case 'e':
            bTestSetModeXDSL = 1;
            break;

         case 'k':
            bTestDsmCnfGet = 1;
            break;

         case 'K':
            if (optarg)
            {
               temp = MEIOS_StrToUl(optarg, &pEndPtr, 0x10);
               if (errno)
               {
                  MEIOS_Printf(TEST_MEI_DBG_PREFIX"DSM config: invalid value 0x%08X" MEIOS_CRLF, (unsigned int)temp);
               }
               else
               {
                  bTestDsmCnfSet = (int)temp;
               }
            }
            break;

         case 'j':
            bTestDsmStsGet = 1;
            break;

         case 'J':
            bTestDsmStcGet = 1;
            break;

         case 'q':
            bTestDsmMacGet = 1;
            break;

         case 'Q':
            bTestDsmMacSet = 1;
            break;

         case 'G':
            bDbgLevelSet = 1;
            break;

         case 'O':
            bPllOffsetGet = 1;
            break;

         case 'P':
            if (optarg)
            {
               /*temp = MEIOS_StrToUl(optarg, &pEndPtr, 0x10);*/
               temp = MEIOS_StrToUl(optarg, &pEndPtr, 10);
               /*printf("\n---> errno %d, P %d <----\n", errno, temp);*/
               if (errno)
               {
                  MEIOS_Printf(TEST_MEI_DBG_PREFIX"MEI Offset Addr: invalid,  0x%08X" MEIOS_CRLF, (unsigned int)temp);
               }
               else
               {
                  bPllOffsetSet = (int)temp;
                  /*EIOS_Printf(TEST_MEI_DBG_PREFIX"MEI Offset Addr: 0x%08X" MEIOS_CRLF, (unsigned int)temp);*/
                  MEIOS_Printf(TEST_MEI_DBG_PREFIX"MEI Offset Addr: %d" MEIOS_CRLF, (unsigned int)temp);
               }
            }
            break;

         case 'z':
            GetOptArg_RequStr(optarg, &argStrParam[0][0], "opt Str[0]");
            break;
            
      }        /* switch(c) {...} */
   }        /* while(1) {...} */

   if (optind < argc)
   {
      MEIOS_Printf (TEST_MEI_DBG_PREFIX"Sorry, there are unrecognized options: ");
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

   MEIOS_Printf("usage: MEI_drv_test [options]" MEIOS_CRLF);
   MEIOS_Printf("following options defined:" MEIOS_CRLF);

   while(ptr->name)
   {
      MEIOS_Printf(" --%s (-%c)\t- %s" MEIOS_CRLF, ptr->name, ptr->val, desc);
      ptr++;
      desc += sizeof(description[0]);
   }

   MEIOS_Printf("" MEIOS_CRLF);
   return;
}


/**


*/
void PrintoutMem(unsigned int value)
{
   union {
      unsigned char  pChar[4];
      unsigned short pShort[2];
      unsigned int   pInt[1];
   } temp;

   temp.pInt[0] = value;

   MEIOS_Printf(
      MEIOS_CRLF "          00010203" MEIOS_CRLF);
   MEIOS_Printf("---------------------------" MEIOS_CRLF);
   MEIOS_Printf("uChar:  0x%02X%02X%02X%02X" MEIOS_CRLF, temp.pChar[0], temp.pChar[1], temp.pChar[2], temp.pChar[3]);
   MEIOS_Printf("uShort: 0x%04X%04X" MEIOS_CRLF, temp.pShort[0], temp.pShort[1]);
   MEIOS_Printf("uInt:   0x%08X" MEIOS_CRLF MEIOS_CRLF, temp.pInt[0]);

   temp.pInt[0] = SWAP_32_TO_16BIT_PAYLOAD(value);

   MEIOS_Printf(
      MEIOS_CRLF "- Swap 32Bit Payload ------" MEIOS_CRLF);
   MEIOS_Printf(
      "uChar:  0x%02X%02X%02X%02X" MEIOS_CRLF, temp.pChar[0], temp.pChar[1], temp.pChar[2], temp.pChar[3]);
   MEIOS_Printf(
      "uShort: 0x%04X%04X" MEIOS_CRLF, temp.pShort[0], temp.pShort[1]);
   MEIOS_Printf(
      "uInt:   0x%08X" MEIOS_CRLF MEIOS_CRLF, temp.pInt[0]);

   temp.pInt[0] = value;
   temp.pInt[0] = SWAP_32_TO_16BIT_PAYLOAD(temp.pInt[0]);

   MEIOS_Printf(
      MEIOS_CRLF "- Swap to itself ----------" MEIOS_CRLF);
   MEIOS_Printf(
      "uChar:  0x%02X%02X%02X%02X" MEIOS_CRLF, temp.pChar[0], temp.pChar[1], temp.pChar[2], temp.pChar[3]);
   MEIOS_Printf(
      "uShort: 0x%04X%04X" MEIOS_CRLF, temp.pShort[0], temp.pShort[1]);
   MEIOS_Printf(
      "uInt:   0x%08X" MEIOS_CRLF MEIOS_CRLF, temp.pInt[0]);

   /* ARC little endian --> host:
      MEM 78 56 34 12  --> 12 34 56 78
      MB (16 bit):
         --> 0x5678
         --> 0x1234
   */

   temp.pInt[0] = (unsigned int) (0x5678 & 0x0000FFFF);

   MEIOS_Printf(
      MEIOS_CRLF "- MB first word ---------" MEIOS_CRLF);
   MEIOS_Printf("uChar:  0x%02X%02X%02X%02X" MEIOS_CRLF, temp.pChar[0], temp.pChar[1], temp.pChar[2],temp. pChar[3]);
   MEIOS_Printf("uShort: 0x%04X%04X" MEIOS_CRLF, temp.pShort[0], temp.pShort[1]);
   MEIOS_Printf("uInt:   0x%08X" MEIOS_CRLF MEIOS_CRLF, temp.pInt[0]);

   temp.pInt[0] = (unsigned int) (((0x1234 & 0x0000FFFF) << 16) | temp.pInt[0]);
   MEIOS_Printf(
      MEIOS_CRLF "- MB 2. word ------------" MEIOS_CRLF);
   MEIOS_Printf("uChar:  0x%02X%02X%02X%02X" MEIOS_CRLF, temp.pChar[0], temp.pChar[1], temp.pChar[2], temp.pChar[3]);
   MEIOS_Printf("uShort: 0x%04X%04X" MEIOS_CRLF, temp.pShort[0], temp.pShort[1]);
   MEIOS_Printf("uInt:   0x%08X" MEIOS_CRLF MEIOS_CRLF, temp.pInt[0]);
   MEIOS_Printf("---------------------------" MEIOS_CRLF MEIOS_CRLF);

   return;
}

/**
Description:
   Test program to test the VRX driver.
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

   if( bTestHelp == 1)
   {
      doHelp();

      /* PrintoutMem(0x12345678); */
      return EXIT_SUCCESS;
   }

   MEI_drv_test();

   return 0;

}


/* ============================================================== */
/* Local Function Definitions                                     */
/* ============================================================== */

void MEI_drv_test(void)
{
   int fd, ret=-1;   
   MEIOS_File_t *streamOut = stdout;

   /* open default device or given device num */
   bTestDfeChannel = (bTestDfeChannel == -1) ? TEST_MEI_USED_DFE_NUM : bTestDfeChannel;

   fd = MEI_open_dev(streamOut, bTestDfeChannel,
               (char *)TEST_MEI_DEV_PREFIX,
               (char *)TEST_MEI_DEVICE_NAME );
   if( fd < 0 )
   {
      return;
   }

   /* == Begin Test cases ==================================================
      here do the test cases
      ====================================================================== */

   /* test case: init device */
   if (bTestInitDev == 1)
   {
      if (bOptValTest != -1)
      {
         MEI_DataDevInit.usedIRQ = (unsigned int)bOptValTest;
      }
      ret = MEI_init_dev(streamOut, fd, &MEI_DataDevInit);
   }

   /* test case: init device */
   if (bTestInitDrv == 1)
   {
      IOCTL_MEI_drvInit_t  MEI_DataDrvInit;
      MEIOS_MemSet(&MEI_DataDrvInit, 0x00, sizeof(IOCTL_MEI_drvInit_t));

      if (bParams[0] != -1)
         MEI_DataDrvInit.blockTimeout = (bParams[0]) ? 1 : 0;

      if (bParams[1] != -1)
         MEI_DataDrvInit.waitModemMsg_ms = bParams[1];

      if (bParams[2] != -1)
         MEI_DataDrvInit.waitFirstResp_ms = bParams[2];

      ret = MEI_init_drv(streamOut, fd, &MEI_DataDrvInit);
   }

   /* test case: request VRX configuration */
   if (bReqCfg == 1)
   {
      int tempRet;

      ret = 0;
      tempRet = MEI_req_cfg(streamOut, fd);
      ret = (tempRet < 0) ? tempRet : ret;
      tempRet = MEI_req_stat(streamOut, fd);
      ret = (tempRet < 0) ? tempRet : ret;
   }

   /* test case: request version */
   if (bTestGetVer == 1)
   {
      ret = MEI_GetVersion(streamOut, fd );
   }

   /* test case: reset chip HW */
   if (bDrvReset != -1)
   {
      ret = MEI_drv_reset(streamOut, fd, bDrvReset);
   }

   /* test case: set debug trace output */
   if (bDrvPrintOutTr != -1)
   {
      if (bTestDfeChannel & 0x80)
      {
         ret = MEI_x_drv_set_trace(streamOut, fd, bDrvPrintOutTr, bParams);
      }
      else
      {
         ret = -1;
         MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
                  "Error Trace Level set - device[%d] is no control device" MEIOS_CRLF,
                  bTestDfeChannel );
      }
   }

   /* test case: show MEI register set */
   if (bTestDisplMeiRegs == 1)
      MEI_show_mei_regs(fd);

   /* test case: set MEI register */
   if (bTestSetMeiReg != -1)
   {
      if (bTestSetMeiValue != -1)
      {
         MEI_RegIo.addr  = (unsigned int)bTestSetMeiReg;
         MEI_RegIo.value = (unsigned int)bTestSetMeiValue;
         ret = MEI_set_reg(streamOut, fd, &MEI_RegIo);
      }
      else
      {
         ret = -1;
         MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
                 "Set MEI Reg[0x%02X]: missing value" MEIOS_CRLF,bTestSetMeiReg);
      }
   }

   /* test case: get MEI register */
   if (bTestGetMeiReg != -1)
   {
      MEI_RegIo.addr  = (unsigned int)bTestGetMeiReg;
      MEI_RegIo.value = (unsigned int)0;
      ret = MEI_get_reg(streamOut, fd, &MEI_RegIo);
   }

   /* test case: swtich loop ON/OFF */
   if (bTestMbLoop != -1)
   {
      ret = MEI_mailbox_loop(streamOut, fd, bTestMbLoop);
   }

   /* test case: write to mailbox */
   if (bTestMbSend != -1)
   {
      ret = MEI_SendRawMsg(streamOut, fd, bTestMbSend, testValue);
   }

   /* test case: read from mailbox */

   if (bTestMbRead != -1)
   {
      /* ret = MEI_ReadRawAck(fd); */
      ret = MEI_ReadRawNfc(streamOut, fd);
   }

   /* test case: read from mailbox */
   if (bTestDisplBuf != -1)
      MEI_show_drv_buffer(fd, bTestDisplBuf, (testValue == -1) ? 0x16 : testValue);


   /* test case: MEI DBG write */
   if (bMeiDbgWrOff != -1)
   {
      bMeiDbgDest = (bMeiDbgDest != -1) ? bMeiDbgDest : 0;
      testValue = (testValue != -1) ? testValue : 0x10;
      ret = MEI_mei_dbg_write(streamOut, fd, bMeiDbgWrOff, bMeiDbgDest, testValue, bParams);
   }

   /* test case: MEI DBG read */
   if (bMeiDbgRdOff != -1)
   {
      bMeiDbgDest = (bMeiDbgDest != -1) ? bMeiDbgDest : 0;
      testValue = (testValue != -1) ? testValue : 0x10;
      ret = MEI_mei_dbg_read(streamOut, fd, bMeiDbgRdOff, bMeiDbgDest, testValue);
   }

   /* test case: do FW download */
   if (bTestFwDl != -1)
   {
      ret = MEI_fw_download(streamOut, fd, bOptValTest, argStrParam[0]);
   }

   if (bTestFwSwap != -1)
   {
      ret = MEI_fw_swap(streamOut, fd, bTestFwSwap);
   }

   if (bTestSetModeXDSL != -1)
   {
      ret = MEI_fw_set_mode(streamOut, fd, argStrParam[0]);
   }

   /* test case: GPA write */
   if (bTestSetGpaReg != -1)
   {
      ret = MEI_gpa_write(streamOut, fd, bTestSetGpaReg, bTestSetGpaDest, testValue);
   }

   /* test case: GPA read */
   if (bTestGetGpaReg != -1)
   {
      ret = MEI_gpa_read(streamOut, fd, bTestGetGpaReg, bTestSetGpaDest);
   }

   /* test case: send MSG */
   if (bTestSendMsg != -1)
   {
      ret = MEI_SendMessage(streamOut, fd, &MEI_IoctArgs.ifx_msg_send, bTestSendMsg, bParams);
   }

   if (bTroubleTest != -1)
   {
      ret = MEI_dma_stress(streamOut, fd, bTroubleTest, testValue, bMeiDbgDest);
   }

   if (bDMAReadTest != -1)
   {
      ret = MEI_dma_read(streamOut, fd, bDMAReadTest, bTestSetGpaDest);
   }

   if (bDMAWriteTest != -1)
   {
      ret = MEI_dma_write(streamOut, fd, bDMAWriteTest, bTestSetGpaDest, testValue);
   }


   /* test case: NFC init device */
   if (bTestNfcInitDev == 1)
   {
      if (bOptValTest != -1)
      {
         MEI_DataDevInit.usedIRQ = bOptValTest;
      }
      ret = MEI_init_dev(streamOut, fd, &MEI_DataDevInit);
      ret = MEI_nfc_wait_for_nfcs(streamOut, 100, bTestDfeChannel, bParams);
   }

   /* test case: get DSM config */
   if (bTestDsmCnfGet != -1)
   {
      ret = MEI_dsm_config_get(streamOut, fd);
   }

   /* test case: set DSM config */
   if (bTestDsmCnfSet != -1)
   {
      ret = MEI_dsm_config_set(streamOut, fd, bTestDsmCnfSet);
   }

   /* test case: get DSM status */
   if (bTestDsmStsGet != -1)
   {
      ret = MEI_dsm_status_get(streamOut, fd);
   }

   /* test case: get DSM statistic */
   if (bTestDsmStcGet != -1)
   {
      ret = MEI_dsm_statistics_get(streamOut, fd);
   }

   /* test case: get MAC address */
   if (bTestDsmMacGet != -1)
   {
      ret = MEI_mac_get(streamOut, fd);
   }

   /* test case: set MAC address */
   if (bTestDsmMacSet != -1)
   {
      ret = MEI_mac_set(streamOut, fd, argStrParam[0]);
   }

   /* test case: set debug level */
   if (bDbgLevelSet != -1)
   {
      ret = MEI_dbg_lvl_set(streamOut, fd, argStrParam[0]);
   }

   /* test case: get PLL offset */
   if (bPllOffsetGet != -1)
   {
      ret = MEI_pll_offset_get(streamOut, fd);
   }

   /* test case: set PLL offset */
   if (bPllOffsetSet != -1)
   {
      ret = MEI_pll_offset_set(streamOut, fd, bPllOffsetSet);
   }
   
   /* == END Test cases ==================================================== */
   /*
   MEIOS_Printf(TEST_MEI_DBG_PREFIX"close(%d) device: /%s/%s/%d." MEIOS_CRLF, fd,
          TEST_MEI_DEV_PREFIX, TEST_MEI_DEVICE_NAME,
          (bTestDfeChannel == -1) ? TEST_MEI_USED_DFE_NUM : bTestDfeChannel);
   */
   MEIOS_DeviceClose(fd);

   MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
            "Line = %02d Return = %d" MEIOS_CRLF, bTestDfeChannel, ret);

   return;
}

int MEI_get_mac_addr(unsigned char *pString, IOCTL_MEI_MacConfig_t *pMacAdr)
{
   char string[20] = {0};
   char sMacAddr[3] = {0};
   char seps[] = ":";
   char *token;
   int i = 0;

   strncpy (string, (char *)pString, sizeof(string)-1);
   string[sizeof(string)-1] = 0;

   /* Get first token */
   token = strtok (string, seps);
   if (token != NULL)
   {
      for (i = 0; i < MEI_MAC_ADDRESS_OCTETS; i++)
      {
         sscanf (token, "%2s", (unsigned char *)sMacAddr);
         sscanf (token, "%hhx", (unsigned char *)&(pMacAdr->nMacAddress[i]));

         if ( ((strcmp (&sMacAddr[1], "0") != 0) && ( (pMacAdr->nMacAddress[i] & 0xF) == 0)) )
         {
            i=0;
            break;
         }

         sMacAddr[1] = '\0';
         if ( ((strcmp (&sMacAddr[0], "0") != 0) && ( ((pMacAdr->nMacAddress[i] >> 4) & 0xF) == 0)) )
         {
            i=0;
            break;
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

   if (i < (MEI_MAC_ADDRESS_OCTETS - 1))
   {
      return -1;
   }
   else
   {
      return 0;
   }

}

#endif /* LINUX */


