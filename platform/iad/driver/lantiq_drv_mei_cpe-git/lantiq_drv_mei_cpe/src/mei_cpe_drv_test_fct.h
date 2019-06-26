#ifndef _MEI_CPE_drv_test_fct_h
#define _MEI_CPE_drv_test_fct_h
/******************************************************************************

                          Copyright (c) 2007-2015
                     Lantiq Beteiligungs-GmbH & Co. KG

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/* ==========================================================================
   Description : Common test routines for the VRX driver test application.
   ========================================================================== */
#ifdef __cplusplus
extern "C"
{
#endif

/* ==========================================================================
   includes
   ========================================================================== */
#include "mei_cpe_appl_osmap.h"

#include "drv_mei_cpe_interface.h"
#include "cmv_message_format.h"


/* ==========================================================================
   global macros
   ========================================================================== */

#define SWAP_32_TO_16BIT_PAYLOAD(x)  ( (((x)&0xFFFF0000)>>16)  \
                                     | (((x)&0x0000FFFF)<<16) )



#define TEST_MEI_DBG_PREFIX   "T >> "

/*
   Device location like

      /dev/VRX4/0/1
       ___ __________ _ _
        |      |    | | |
        |      |    | | +- VRX number (relative channel)
        |      |    | +--- VRXx entity (chip)
        |      |    +-- VRXx entity type (VRX1 or VRX4)
        |      +------- VRXx entity type (VRX1 or VRX4)
        +----------- prefix
*/
#define TEST_MEI_DEV_PREFIX      "/dev"
#define TEST_MEI_DEVICE_NAME     "mei_cpe"

#define TEST_MEI_USED_DFEX_NUM   0
#define TEST_MEI_USED_DFE_NUM    0

#define TEST_MEI_BASE_ADDR       0xC0200000
#define TEST_MEI_IRQ_NUM         0x00000000
/* #define TEST_MEI_IRQ_NUM         99 */

#define MEI_FW_DL_FILE_BASE_NAME   "firmware"

/* optional parameter for send message */
#define MEI_TEST_MAX_OPT_PARAMS   8

/* ==========================================================================
   export global variables
   ========================================================================== */

extern IOCTL_MEI_arg_t          MEI_IoctArgs;
extern IOCTL_MEI_regInOut_t    MEI_RegIo;
extern IOCTL_MEI_devInit_t       MEI_DataDevInit;


/* ============================================================================
   export global functions
   ========================================================================= */
int MEI_open_dev(MEIOS_File_t *streamOut, int devNum, char *pPrefixName, char *pDevBaseName);
int MEI_init_dev(MEIOS_File_t *streamOut, int fd, IOCTL_MEI_devInit_t *pDevInit);
int MEI_init_drv(MEIOS_File_t *streamOut, int fd, IOCTL_MEI_drvInit_t *pDrvInit);
int MEI_req_cfg(MEIOS_File_t *streamOut, int fd);
int MEI_req_stat(MEIOS_File_t *streamOut, int fd);
int MEI_drv_reset(MEIOS_File_t *streamOut, int fd, unsigned int resetMode);

int MEI_x_drv_set_trace(MEIOS_File_t *streamOut, int fd, unsigned int debugArgs, int *pParamArr);

int MEI_mailbox_loop(MEIOS_File_t *streamOut, int fd, int on_off);

int MEI_GetVersion(MEIOS_File_t *streamOut, int fd);

int MEI_get_reg(MEIOS_File_t *streamOut, int fd, IOCTL_MEI_regInOut_t *pRegIO);
int MEI_set_reg(MEIOS_File_t *streamOut, int fd, IOCTL_MEI_regInOut_t *pRegIO);

int MEI_ReadRawAck(MEIOS_File_t *streamOut, int fd);
int MEI_SendRawMsg(MEIOS_File_t *streamOut, int fd, int payload_count_16bit, int start);

void MEI_SetupSendMsg( IOCTL_MEI_message_t *pMsgCntrl,
                         unsigned char *pBuf, int bufSize,
                         unsigned short msgID);

void MEI_SetupSendMsgArg( IOCTL_MEI_message_t *pMsgCntrl,
                            unsigned char *pBuf,
                            unsigned short msgID, int *pParamArr);

void MEI_log_ifx_msg(
                  MEIOS_File_t   *streamOut,
                  char           *pLogStr,
                  IOCTL_MEI_message_t *pCntrl);

void MEI_log_eth_frame(
                  MEIOS_File_t  *streamOut,
                  char          *pLogStr,
                  unsigned char *pEthFrameStr,
                  int           frameSize_Byte);

void MEI_log_atmoam_cell_msg(MEIOS_File_t *streamOut, IOCTL_MEI_message_t *pCntrl);
void MEI_log_drv_msg(MEIOS_File_t *streamOut, IOCTL_MEI_message_t *pCntrl);

int MEI_SendMessage(MEIOS_File_t *streamOut, int fd, IOCTL_MEI_messageSend_t *pMsgSend,
                          unsigned short msgID, int *pParamArr);
int MEI_WriteMessage(MEIOS_File_t *streamOut, int fd, IOCTL_MEI_message_t *pMsgWrite,
                           unsigned short msgID, int *pParamArr);
int MEI_ReadAck(MEIOS_File_t *streamOut, int fd, IOCTL_MEI_message_t *pMsgAck);
int MEI_ReadNfc(MEIOS_File_t *streamOut, int fd, IOCTL_MEI_message_t *pMsgNfc);


int MEI_ReceiveNfcOnOff(MEIOS_File_t *streamOut, int fd, int on_off);
int MEI_ReadRawNfc(MEIOS_File_t *streamOut, int fd);

int MEI_AutoMsgCntl(MEIOS_File_t *streamOut, int fd, int set_get,
                      unsigned int modemCtrl, unsigned int driverCtrl);

void MEI_show_mei_regs(int fd);
void MEI_show_drv_buffer(int fd, unsigned char bufNum, unsigned int count);

int MEI_mei_dbg_read(MEIOS_File_t *streamOut, int fd, int offset, int des, int count);
int MEI_mei_dbg_write(MEIOS_File_t *streamOut, int fd, int offset, int des, int count, int *pParamArr);

int MEI_gpa_read(MEIOS_File_t *streamOut, int fd, int addr, int dest);
int MEI_gpa_write(MEIOS_File_t *streamOut, int fd, int addr, int dest, int value);


int MEI_fw_download_name(MEIOS_File_t *streamOut, int fd, char *pFileName);
int MEI_fw_download(MEIOS_File_t *streamOut, int fd, int fileNum, char *pFileName);
int MEI_fw_swap(MEIOS_File_t *streamOut, int fd, int bTestFwSwap);
int MEI_fw_set_mode(MEIOS_File_t *streamOut, int fd, char *pConfig);

int MEI_dma_stress(MEIOS_File_t *streamOut, int fd, unsigned int addr, unsigned int range, unsigned int loop);
int MEI_dma_wr_rd(MEIOS_File_t *streamOut, int fd, unsigned int addr, unsigned int count, unsigned int start);
int MEI_dma_write(MEIOS_File_t *streamOut, int fd, unsigned int addr, unsigned int count, unsigned int start);
int MEI_dma_read(MEIOS_File_t *streamOut, int fd, unsigned int addr, unsigned int count);

int MEI_dsm_config_get(MEIOS_File_t *streamOut, int fd);
int MEI_dsm_config_set(MEIOS_File_t *streamOut, int fd, int vector_control);
int MEI_dsm_status_get(MEIOS_File_t *streamOut, int fd);
int MEI_dsm_statistics_get(MEIOS_File_t *streamOut, int fd);
int MEI_mac_get(MEIOS_File_t *streamOut, int fd);
int MEI_mac_set(MEIOS_File_t *streamOut, int fd, char *pMAC);
int MEI_dbg_lvl_set(MEIOS_File_t *streamOut, int fd, char *pConfig);
int MEI_pll_offset_get(MEIOS_File_t *streamOut, int fd);
int MEI_pll_offset_set(MEIOS_File_t *streamOut, int fd, int pll_offset);

#ifdef LINUX
int MEI_nfc_wait_for_nfcs(MEIOS_File_t *streamOut, int max_wait_count, int dfeNum, int *pParams);
#endif

int MEI_get_mac_addr (unsigned char *pString, IOCTL_MEI_MacConfig_t *pMacAdr);

#ifdef __cplusplus
/* extern "C" */
}
#endif

#endif      /* #define _MEI_CPE_drv_test_fct_h */

