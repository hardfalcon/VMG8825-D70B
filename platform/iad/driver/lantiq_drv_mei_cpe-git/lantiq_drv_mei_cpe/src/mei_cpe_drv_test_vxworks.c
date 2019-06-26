/******************************************************************************

                          Copyright (c) 2007-2015
                     Lantiq Beteiligungs-GmbH & Co. KG

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/


/* ==========================================================================
   Description : Small test programm to test the VRX Driver
   ========================================================================== */

/* ==========================================================================
   includes
   ========================================================================== */

#ifdef VXWORKS

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>


#include "iolib.h"


/* get common test routines */
#include "mei_cpe_drv_test_fct.h"

/* get interface and configuration */
#include "drv_mei_cpe_interface.h"


/* ==========================================================================
   variables
   ========================================================================== */

#define MEI_NUM_OF_PARAMS   8
static int MEI_ParamTable[MEI_NUM_OF_PARAMS];


/* ==========================================================================
   test application - command table
   ========================================================================== */

/* common basic commands */
#define MEI_TEST_HELP      0
#define MEI_TEST_VER_GET   1

/* config commands */
#define MEI_TEST_CFG_GET  10
#define MEI_TEST_INI_DEV  11
#define MEI_TEST_FDL_DEV  12
#define MEI_TEST_RST_DEV  13
#define MEI_TEST_FW_SEL   14


/* debug commands */
#define MEI_TEST_MEI_GET  20
#define MEI_TEST_MEI_SET  21
#define MEI_TEST_MDBG_RD  22
#define MEI_TEST_MDBG_WR  23
#define MEI_TEST_GPA_RD   24
#define MEI_TEST_GPA_WR   25
#define MEI_TEST_DMA_RD   26
#define MEI_TEST_DMA_WR   27
#define MEI_TEST_DMA_WRRD 28

#define MEI_TEST_MSG_SEND 30
#define MEI_TEST_MSG_WR   31
#define MEI_TEST_ACK_RD   32
#define MEI_TEST_NFC_RD   33


#define MEI_PROC_VERSION 101
#define MEI_PROC_STATUS  102
#define MEI_PROC_SHOW    103
#define MEI_PROC_SET     104
#define MEI_PROC_NFC     105

#define MEI_TEST_INFO    999


struct sVrxDfeTestTable { int cmd; char *pHelpStr; } pVrxDfeTestTable[] =
{
   {MEI_TEST_HELP,    "Printout help info"},
   {MEI_TEST_VER_GET, "Get Driver Version  Args - none"},

   {MEI_TEST_CFG_GET, "Request dev config  Args - none"},
   {MEI_TEST_INI_DEV, "Init VRX Device   Args - 0: phy Addr,  1: IRQ"},
   {MEI_TEST_FDL_DEV, "Firmware Download   Args - 0: <file>"},
   {MEI_TEST_RST_DEV, "Reset VRX Device  Args - 0: Rst Mode"},
   {MEI_TEST_INFO,    "                              Rst Mode: 0=RESET, 1=ACT, 2=DEACT"},
   {MEI_TEST_FW_SEL,  "FW Select           Args - 0: <fw mode>"},
   {MEI_TEST_INFO,    "                              fw mode: 0=VDSL2, 1=ADSL"},

   {MEI_TEST_MEI_GET, "Get a MEI register  Args - 0: addr off [byte]"},
   {MEI_TEST_MEI_SET, "Set a MEI register  Args - 0: addr off [byte], 1: value [16bit]"},

   {MEI_TEST_MDBG_RD, "MEI Debug Read      Args - 0: Dest,  1: addr [32bit], 2: Count"},
   {MEI_TEST_MDBG_WR, "MEI Debug Write     Args - 0: Dest,  1: addr [32bit], 2: Count, 3: Value [32bit]"},
   {MEI_TEST_INFO,    "                              Dest: 0=AUX, 2=LDST, 3=CORE"},

   {MEI_TEST_GPA_RD,  "MEI GPA Read        Args - 0: Dest,  1: addr [32bit]"},
   {MEI_TEST_GPA_WR,  "MEI GPA Write       Args - 0: Dest,  1: addr [32bit], 2: Value [32bit]"},
   {MEI_TEST_INFO,    "                              Destination: 0=MEM, 1=AUX"},

   {MEI_TEST_DMA_RD,  "MEI DMA Read        Args - 0: addr [32bit]"},
   {MEI_TEST_DMA_WR,  "MEI DMA Write       Args - 0: addr [32bit],  1: Value [32bit]"},
   {MEI_TEST_DMA_WRRD,"MEI DMA Wr/Rd       Args - 0: addr [32bit],  1: Value [32bit], 3: Count"},

   {MEI_TEST_INFO,    " "},
   {MEI_TEST_MSG_SEND,"MEI MSG Send        Args - 0: IFX msg ID,  1: Count, 2: Payl[0], 3: Payl[1], ..."},
   {MEI_TEST_MSG_WR,  "MEI Msg Write       Args - 0: IFX msg ID,  1: Count, 2: Payl[0], 3: Payl[1], ..."},
   {MEI_TEST_ACK_RD,  "MEI ACK Read        Args - none"},
   {MEI_TEST_NFC_RD,  "MEI NFC READ        Args - none"},

   {-1, NULL}
};


/* ==========================================================================
   test application - local routines
   ========================================================================== */

/**
   Print Help to specified stream
*/
static int doVrxDrvTestHelp_fd(MEIOS_File_t *streamOut)
{
   int testIndex = 0;

   if(streamOut == NULL)
   {
      streamOut = stdout;
   }

   MEIOS_FPrintf(streamOut,
      MEIOS_CRLF "VRX driver - Low Level Tests" MEIOS_CRLF MEIOS_CRLF);

   while (1)
   {
      if (pVrxDfeTestTable[testIndex].cmd == -1)
         break;

      if (pVrxDfeTestTable[testIndex].cmd != MEI_TEST_INFO)
      {
         MEIOS_FPrintf(streamOut,
            "%3d  %s" MEIOS_CRLF,
            pVrxDfeTestTable[testIndex].cmd, pVrxDfeTestTable[testIndex].pHelpStr);
      }
      else
      {
         MEIOS_FPrintf(streamOut,
            "     %s" MEIOS_CRLF,
            pVrxDfeTestTable[testIndex].pHelpStr);
      }

      testIndex++;
   }

   MEIOS_FPrintf(streamOut,
      MEIOS_CRLF "Usage (device access):" MEIOS_CRLF
      "\tdoVrxDrvTest <cmd>, <dev num>, [<arg 0>, <arg 1>, ..., <arg 4>]" MEIOS_CRLF);

   MEIOS_FPrintf(streamOut,
      MEIOS_CRLF "Usage (device access):" MEIOS_CRLF
      "\tdoVrxDrvTest 30, 0, 0x0010, 2, 0, 3  --> request FW version" MEIOS_CRLF MEIOS_CRLF);

   MEIOS_FPrintf(streamOut,
      MEIOS_CRLF "For VxWorks Proc FS use: " MEIOS_CRLF
      "\tdoVrxProcFs <cmd>, [arg 0], [arg 1]" MEIOS_CRLF MEIOS_CRLF);

   return 0;
}

/**
   Help
*/
static int doVrxDrvTestHelp()
{
   return doVrxDrvTestHelp_fd(stdout);
}

/* ==========================================================================
   test application - routines
   ========================================================================== */

/**
   Test routine to wrap the user params to the corresponding function.
   Selection of the output stream for printouts added.
*/
int doVrxDrvTest_fd(
                   MEIOS_File_t *streamOut,
                   int cmd, int devNum,
                   int param0, int param1, int param2, int param3, int param4,
                   int param5, int param6, int param7, int param8, int param9,
                   int param10, int param11)
{
   int ret = -1;
   int fd = -1;

   if(streamOut == NULL)
   {
      streamOut = stdout;
   }

   if ( (cmd == 0) || (cmd == -1) )
   {
      doVrxDrvTestHelp_fd(streamOut);
      return 0;
   }

   if ( (fd = MEI_open_dev( streamOut,
                              devNum,
                              (char *)TEST_MEI_DEV_PREFIX,
                              (char *)TEST_MEI_DEVICE_NAME) ) <= 0)
   {
      return -1;
   }

   switch(cmd)
   {
      case MEI_TEST_VER_GET:
         ret = MEI_GetVersion(streamOut, fd);
         break;

      case MEI_TEST_CFG_GET:
         {
            int tempRet;

            ret = 0;
            tempRet = MEI_req_cfg(streamOut, fd);
            ret = (tempRet < 0) ? tempRet : ret;
            tempRet = MEI_req_stat(streamOut, fd);
            ret = (tempRet < 0) ? tempRet : ret;
         }
         break;

      case MEI_TEST_INI_DEV:
         MEI_DataDevInit.meiBaseAddr = (unsigned int)param0;
         MEI_DataDevInit.usedIRQ     = (unsigned int)param1;
         ret = MEI_init_dev(streamOut, fd, &MEI_DataDevInit);
         break;

      case MEI_TEST_FDL_DEV:
         ret = MEI_fw_download_name(streamOut, fd, (char *)param0);
         break;

      case MEI_TEST_RST_DEV:
         ret = MEI_drv_reset(streamOut, fd, (unsigned int)param0);
         break;

      case MEI_TEST_FW_SEL:
         ret = MEI_fw_swap(streamOut, fd, (int)param0);
         break;

      case MEI_TEST_MEI_GET:
         MEI_RegIo.addr  = (unsigned int)param0;
         MEI_RegIo.value = (unsigned int)param1;
         ret = MEI_get_reg(streamOut, fd, &MEI_RegIo);
         break;

      case MEI_TEST_MEI_SET:
         MEI_RegIo.addr  = (unsigned int)param0;
         MEI_RegIo.value = (unsigned int)param1;
         ret = MEI_set_reg(streamOut, fd, &MEI_RegIo);
         break;

      case MEI_TEST_MDBG_RD:
         ret = MEI_mei_dbg_read(streamOut, fd, param1, param0, param2);
         break;

      case MEI_TEST_MDBG_WR:
         ret = MEI_mei_dbg_write(streamOut, fd, param1, param0, param2, &param3);
         break;

      case MEI_TEST_GPA_RD:
         ret = MEI_gpa_read(streamOut, fd, param1, param0);
         break;

      case MEI_TEST_GPA_WR:
         ret = MEI_gpa_write(streamOut, fd, param1, param0, param2);
         break;

      case MEI_TEST_DMA_RD:
         /* MEI DMA Read        Args - 0: addr [32bit] */
         ret = MEI_dma_read(streamOut, fd, (unsigned int) param0, (unsigned int)1);
         break;

      case MEI_TEST_DMA_WR:
         /* MEI DMA Write       Args - 0: addr [32bit],  1: Value [32bit] */
         ret = MEI_dma_write(streamOut, fd, (unsigned int) param0, (unsigned int)1, (unsigned int)param1);
         break;

      case MEI_TEST_DMA_WRRD:
         /* MEI DMA Wr/Rd       Args - 0: addr [32bit],  1: Value [32bit], 3: Count */
         ret = MEI_dma_wr_rd(streamOut, fd, (unsigned int) param0, (unsigned int)param2, (unsigned int)param1);
         break;

      case MEI_TEST_MSG_SEND:
         /* set all to -1 */
         {
            int paramCount, *pParam = &param2;

            param1 = (param1 > MEI_NUM_OF_PARAMS) ? MEI_NUM_OF_PARAMS : param1;
            MEIOS_MemSet(&MEI_ParamTable, 0xFF, sizeof(MEI_ParamTable));
            for (paramCount = 0; paramCount < param1; paramCount++)
            {
               MEI_ParamTable[paramCount] = pParam[paramCount];
            }

            ret = MEI_SendMessage( streamOut, fd, &MEI_IoctArgs.ifx_msg_send,
                                     (unsigned short)param0, MEI_ParamTable);
         }
         break;

      case MEI_TEST_MSG_WR:
         /* set all to -1 */
         {
            int paramCount, *pParam = &param2;

            param1 = (param1 > MEI_NUM_OF_PARAMS) ? MEI_NUM_OF_PARAMS : param1;
            MEIOS_MemSet(&MEI_ParamTable, 0xFF, sizeof(MEI_ParamTable));
            for (paramCount = 0; paramCount < param1; paramCount++)
            {
               MEI_ParamTable[paramCount] = pParam[paramCount];
            }

            ret = MEI_WriteMessage(streamOut, fd, &MEI_IoctArgs.ifx_msg,
                                         (unsigned short)param0, MEI_ParamTable);
         }
         break;

      case MEI_TEST_ACK_RD:
         ret = MEI_ReadAck(streamOut, fd, &MEI_IoctArgs.ifx_msg);
         break;

      case MEI_TEST_NFC_RD:
         ret = MEI_ReadNfc(streamOut, fd, &MEI_IoctArgs.ifx_msg);
         break;

      default:
         MEIOS_FPrintf(streamOut,
            MEIOS_CRLF "Error - unknown command %d" MEIOS_CRLF, cmd);
         doVrxDrvTestHelp_fd(streamOut);
         ret = -1;
   }

   if (fd > 0)
      MEIOS_DeviceClose(fd);

   MEIOS_FPrintf(streamOut, TEST_MEI_DBG_PREFIX
            "Line = %02d Return = %d" MEIOS_CRLF, devNum, ret);

   return ret;
}

/**
   Test routine to wrap the user params to the corresponding function
*/
int doVrxDrvTest(int cmd, int devNum,
                   int param0, int param1, int param2, int param3, int param4,
                   int param5, int param6, int param7, int param8, int param9,
                   int param10, int param11 )
{
   return doVrxDrvTest_fd(
            stdout,
            cmd, devNum,
            param0, param1, param2, param3, param4,  param5,
            param6, param7, param8, param9, param10, param11);
}

#endif

