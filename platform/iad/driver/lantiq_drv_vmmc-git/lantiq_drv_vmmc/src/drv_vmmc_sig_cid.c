/******************************************************************************

                              Copyright (c) 2012
                            Lantiq Deutschland GmbH
                             http://www.lantiq.com

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/*
   \file drv_vinetic_cid.c
   This file contains the implementation of the functions for CID operations.
*/

#include "drv_api.h"
/* ============================= */
/* Check if feature is enabled   */
/* ============================= */
#ifdef TAPI_CID

/* ============================= */
/* Includes                      */
/* ============================= */
#include "drv_vmmc_sig_priv.h"
#include "drv_vmmc_sig_cid.h"
#include "drv_vmmc_api.h"
#include "drv_vmmc_sig.h"

/* ============================= */
/* Local Macros & Definitions    */
/* ============================= */
/* DTMF max digt/inter digit time */
#define MAX_DIGIT_TIME        127 /* ms */
#define MAX_INTERDIGIT_TIME   127 /* ms */

/* ============================= */
/* Global variable definition    */
/* ============================= */

/* ============================= */
/* Global function declaration   */
/* ============================= */

/* ============================= */
/* Extern function declaration   */
/* ============================= */
extern IFX_TAPI_CID_RX_DATA_t *TAPI_Phone_GetCidRxBuf (TAPI_CHANNEL *pChannel,
                                                       IFX_uint32_t nLen);

/* ============================= */
/* Local function declaration    */
/* ============================= */
static IFX_int32_t vmmc_sig_SetCidSender (VMMC_CHANNEL *pCh,
                                         IFX_boolean_t bEn);
static IFX_int32_t vmmc_sig_SetCidRec   (VMMC_CHANNEL const *pCh,
                                         IFX_TAPI_CID_HOOK_MODE_t  cidHookMode,
                                         IFX_TAPI_CID_FSK_CFG_t *pFskConf,
                                         IFX_TAPI_CID_STD_t nStandard,
                                         IFX_boolean_t bEn);
static IFX_int32_t vmmc_sig_SetCidCoeff (VMMC_CHANNEL *pCh,
                                         IFX_TAPI_CID_HOOK_MODE_t cidHookMode,
                                         IFX_TAPI_CID_FSK_CFG_t *pFskConf,
                                         IFX_uint8_t nSize);
static IFX_int32_t vmmc_sig_FSKG_Start   (VMMC_CHANNEL *pCh);
static IFX_int32_t vmmc_sig_FSKG_Stop    (VMMC_CHANNEL *pCh);
static IFX_int32_t vmmc_sig_FSKG_Request (VMMC_CHANNEL *pCh);

/* ============================= */
/* Local variable definition     */
/* ============================= */
/* Level table   internal dB values
   val     0,     5,    10,    15,    20,    25,    30,    35,    40,    45,
          50,    55,    60,    65,    70,    75,    80,    85,    90,    95,
         100,   105,   110,   115,   120,   125,   130,   135,   140,   145,
         150,   155,   160,   165,   170,   175,   180,   185,   190,   195,
         200,   205,   210,   215,   220,   225,   230,   235,   240,   245,
         250,   255,   260,   265,   270,   275,   280,   285,   290,   295,
         300,   305,   310,   315,   320,   325,   330,   335,   340,   345,
         350,   355,   360,   365,   370,   375,   380,   385,   390,   395,
         400,   405,   410,   415,   420,   425,   430,   435,   440,   445,
         450,   455,   460,   465,   470,   475,   480,   485,   490,   495,
         500,   505,   510,   515,   520,   525,   530,   535,   540,   545,
         550,   555,   560,   565,   570,   575,   580,   585,   590,   595,
         600,   605,   610,   615,   620,   625,   630,   635,   640,   645,
         650,   655,   660,   665,   670,   675,   680,   685,   685,   695,
         700,   705,   710,   715,   720,   725,   730,   735,   740,   745,
         750,   755,   760,   765,   770,   775,   780,   785,   790,   795,
         800,   805,   810,   815,   820,   825,   830,   835,   840,   845,
         850,   855,   860,   865,   870,   875,   880,   885,   890,   895,
         900,   905,   910,   915,   920,   925,   930,   935,   940,   945,
         950,   955,   960 */
/* Level table   dB values
   dB      0,  -0.5,  -1.0,  -1.5,  -2.0,  -2.5,  -3.0,  -3.5,  -4.0,  -4.5,
        -5.0,  -5.5,  -6.0,  -6.5,  -7.0,  -7.5,  -8.0,  -8.5,  -9.0,  -9.5,
       -10.0, -10.5, -11.0, -11.5, -12.0, -12.5, -13.0, -13.5, -14.0, -14.5,
       -15.0, -15.5, -16.0, -16.5, -17.0, -17.5, -18.0, -18.5, -19.0, -19.5,
       -20.0, -20.5, -21.0, -21.5, -22.0, -22.5, -23.0, -23.5, -24.0, -24.5,
       -25.0, -25.5, -26.0, -26.5, -27.0, -27.5, -28.0, -28.5, -29.0, -29.5,
       -30.0, -30.5, -31.0, -31.5, -32.0, -32.5, -33.0, -33.5, -34.0, -34.5,
       -35.0, -35.5, -36.0, -36.5, -37.0, -37.5, -38.0, -38.5, -39.0, -39.5,
       -40.0, -40.5, -41.0, -41.5, -42.0, -42.5, -43.0, -43.5, -44.0, -44.5,
       -45.0, -45.5, -46.0, -46.5, -47.0, -47.5, -48.0, -48.5, -49.0, -49.5,
       -50.0, -50.5, -51.0, -51.5, -52.0, -52.5, -53.0, -53.5, -54.0, -54.5,
       -55.0, -55.5, -56.0, -56.5, -57.0, -57.5, -58.0, -58.5, -59.0, -59.5,
       -60.0, -60.5, -61.0, -61.5, -62.0, -62.5, -63.0, -63.5, -64.0, -64.5,
       -65.0, -65.5, -66.0, -66.5, -67.0, -67.5, -68.0, -68.5, -69.0, -69.5,
       -70.0, -70.5, -71.0, -71.5, -72.0, -72.5, -73.0, -73.5, -74.0, -74.5,
       -75.0, -75.5, -76.0, -76.5, -77.0, -77.5, -78.0, -78.5, -79.0, -79.5,
       -80.0, -80.5, -81.0, -81.5, -82.0, -82.5, -83.0, -83.5, -84.0, -84.5,
       -85.0, -85.5, -86.0, -86.5, -87.0, -87.5, -88.0, -88.5, -89.0, -89.5,
       -90.0, -90.5, -91.0, -91.5, -92.0, -92.5, -93.0, -93.5, -94.0, -94.5,
       -95.0, -95.5, -96.0 */

/* formula: level=32768*10^(dBval / 20) */
static const IFX_uint16_t cid_rx_levels[] = {
0x8000, 0x78D6, 0x7214, 0x6BB2, 0x65AC, 0x5FFC, 0x5A9D, 0x558C, 0x50C3, 0x4C3E,
0x47FA, 0x43F4, 0x4026, 0x3C90, 0x392C, 0x35FA, 0x32F5, 0x301B, 0x2D6A, 0x2AE0,
0x287A, 0x2636, 0x2413, 0x220E, 0x2026, 0x1E5A, 0x1CA7, 0x1B0D, 0x198A, 0x181C,
0x16C3, 0x157D, 0x1449, 0x1326, 0x1214, 0x1111, 0x101D, 0x0F36, 0x0E5C, 0x0D8E,
0x0CCC, 0x0C15, 0x0B68, 0x0AC5, 0x0A2A, 0x0999, 0x090F, 0x088E, 0x0813, 0x079F,
0x0732, 0x06CB, 0x066A, 0x060E, 0x05B7, 0x0565, 0x0518, 0x04CF, 0x048A, 0x0449,
0x040C, 0x03D2, 0x039B, 0x0367, 0x0337, 0x0309, 0x02DD, 0x02B4, 0x028D, 0x0269,
0x0246, 0x0226, 0x0207, 0x01EA, 0x01CE, 0x01B4, 0x019C, 0x0185, 0x016F, 0x015B,
0x0147, 0x0135, 0x0124, 0x0113, 0x0104, 0x00F5, 0x00E7, 0x00DB, 0x00CE, 0x00C3,
0x00B8, 0x00AD, 0x00A4, 0x009B, 0x0092, 0x008A, 0x0082, 0x007B, 0x0074, 0x006D,
0x0067, 0x0061, 0x005C, 0x0057, 0x0052, 0x004D, 0x0049, 0x0045, 0x0041, 0x003D,
0x003A, 0x0037, 0x0033, 0x0031, 0x002E, 0x002B, 0x0029, 0x0026, 0x0024, 0x0022,
0x0020, 0x001E, 0x001D, 0x001B, 0x001A, 0x0018, 0x0017, 0x0015, 0x0014, 0x0013,
0x0012, 0x0011, 0x0010, 0x000F, 0x000E, 0x000D, 0x000D, 0x000C, 0x000B, 0x000A,
0x000A, 0x0009, 0x0009, 0x0008, 0x0008, 0x0007, 0x0007, 0x0006, 0x0006, 0x0006,
0x0005, 0x0005, 0x0005, 0x0004, 0x0004, 0x0004, 0x0004, 0x0003, 0x0003, 0x0003,
0x0003, 0x0003, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0001,
0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001,
0x0001, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
0x0000, 0x0000, 0x0000
};

static const IFX_uint16_t cid_tx_levels[] = {
0xBCAC, 0xB21E, 0xA827, 0x9EBF, 0x95DE, 0x8D7C, 0x8592, 0x7E19, 0x770B, 0x7062,
0x6A19, 0x6429, 0x5E8F, 0x5945, 0x5446, 0x4F90, 0x4B1C, 0x46E9, 0x42F1, 0x3F32,
0x3BA9, 0x3853, 0x352C, 0x3233, 0x2F64, 0x2CBD, 0x2A3D, 0x27E0, 0x25A5, 0x238A,
0x218D, 0x1FAC, 0x1DE7, 0x1C3A, 0x1AA6, 0x1928, 0x17C0, 0x166C, 0x152B, 0x13FC,
0x12DE, 0x11CF, 0x10D0, 0x0FDF, 0x0EFC, 0x0E26, 0x0D5B, 0x0C9C, 0x0BE7, 0x0B3D,
0x0A9C, 0x0A04, 0x0974, 0x08ED, 0x086D, 0x07F4, 0x0782, 0x0717, 0x06B1, 0x0651,
0x05F7, 0x05A1, 0x0551, 0x0505, 0x04BD, 0x0479, 0x0439, 0x03FC, 0x03C3, 0x038D,
0x035A, 0x032A, 0x02FD, 0x02D2, 0x02AA, 0x0284, 0x0260, 0x023E, 0x021D, 0x01FF,
0x01E3, 0x01C7, 0x01AE, 0x0196, 0x017F, 0x016A, 0x0155, 0x0142, 0x0130, 0x011F,
0x010F, 0x0100, 0x00F2, 0x00E4, 0x00D7, 0x00CB, 0x00C0, 0x00B5, 0x00AB, 0x00A1,
0x0098, 0x0090, 0x0088, 0x0080, 0x0079, 0x0072, 0x006C, 0x0066, 0x0060, 0x005A,
0x0055, 0x0051, 0x004C, 0x0048, 0x0044, 0x0040, 0x003C, 0x0039, 0x0036, 0x0033,
0x0030, 0x002D, 0x002B, 0x0028, 0x0026, 0x0024, 0x0022, 0x0020, 0x001E, 0x001C,
0x001B, 0x0019, 0x0018, 0x0016, 0x0015, 0x0014, 0x0013, 0x0012, 0x0011, 0x0010,
0x000F, 0x000E, 0x000D, 0x000C, 0x000C, 0x000B, 0x000A, 0x000A, 0x0009, 0x0009,
0x0008, 0x0008, 0x0007, 0x0007, 0x0006, 0x0006, 0x0006, 0x0005, 0x0005, 0x0005,
0x0004, 0x0004, 0x0004, 0x0004, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0002,
0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0001, 0x0001, 0x0001, 0x0001,
0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0000, 0x0000,
0x0000, 0x0000, 0x0000
};

/* ============================= */
/* Local function definition     */
/* ============================= */

/**
   Initalize the CID module and the cached firmware messages.

  \param  pCh           Pointer to the VMMC channel structure.
*/
IFX_void_t VMMC_SIG_CID_InitCh (VMMC_CHANNEL *pCh)
{
   VMMC_DEVICE   *pDev = pCh->pParent;
   VMMC_SIGCH_t  *pSig = pCh->pSIG;
   IFX_uint8_t   ch = pCh->nChannel - 1;

   /* FSK sender control */
   memset(&pSig->fw_sig_cidsend, 0, sizeof(pSig->fw_sig_cidsend));
   pSig->fw_sig_cidsend.CMD  = CMD_EOP;
   pSig->fw_sig_cidsend.CHAN = ch;
   pSig->fw_sig_cidsend.MOD = MOD_SIGNALING;
   pSig->fw_sig_cidsend.ECMD = SIG_CIDS_CTRL_ECMD;
   pSig->fw_sig_cidsend.CISNR  = ch;
   if (pDev->caps.bEventMailboxSupported)
   {
      pSig->fw_sig_cidsend.EVM   = SIG_CIDS_CTRL_EVM_RDY |
                                   SIG_CIDS_CTRL_EVM_BUF_REQ |
                                   SIG_CIDS_CTRL_EVM_BUF_UNDERFLOW;
   }

   /* FSK sender coefficients */
   memset(&pSig->fw_sig_cids_coef, 0, sizeof(pSig->fw_sig_cids_coef));
   pSig->fw_sig_cids_coef.CMD  = CMD_EOP;
   pSig->fw_sig_cids_coef.CHAN = ch;
   pSig->fw_sig_cids_coef.MOD  = MOD_RESOURCE;
   pSig->fw_sig_cids_coef.ECMD = RES_CIDS_COEF_ECMD;

   /* FSK sender data */
   memset(&pSig->fw_sig_cids_data, 0, sizeof(pSig->fw_sig_cids_data));
   pSig->fw_sig_cids_data.CMD  = CMD_EOP;
   pSig->fw_sig_cids_data.CHAN = ch;
   pSig->fw_sig_cids_data.MOD = MOD_RESOURCE;
   pSig->fw_sig_cids_data.ECMD = RES_CIDS_DATA_ECMD;

   /* FSK receiver control */
   memset(&pSig->fw_sig_cidrcv, 0, sizeof(pSig->fw_sig_cidrcv));
   pSig->fw_sig_cidrcv.CMD = CMD_EOP;
   pSig->fw_sig_cidrcv.CHAN = ch;
   pSig->fw_sig_cidrcv.MOD = MOD_SIGNALING;
   pSig->fw_sig_cidrcv.ECMD = SIG_CIDR_CTRL_ECMD;
   pSig->fw_sig_cidrcv.CIDRNR = ch;

   /* FSK receiver coefficients */
   memset(&pSig->fw_sig_cidr_coef, 0, sizeof(pSig->fw_sig_cidr_coef));
   pSig->fw_sig_cidr_coef.CMD = CMD_EOP;
   pSig->fw_sig_cidr_coef.CHAN = ch;
   pSig->fw_sig_cidr_coef.MOD = MOD_RESOURCE;
   pSig->fw_sig_cidr_coef.ECMD = RES_CIDR_COEF_ECMD;
}


/**
   Basic CID Module configuration.

   \param  pCh          Pointer to the VMMC channel structure.

   \return VMMC_statusOk if no error, otherwise error code.
*/
IFX_int32_t VMMC_SIG_CID_BaseConf (VMMC_CHANNEL *pCh)
{
   VMMC_DEVICE  *pDev    = pCh->pParent;
   VMMC_SIGCH_t *pSIG    = pCh->pSIG;
   IFX_int32_t   ret;

   /* FSK Generator ********************************************************* */
   /* EN = 0 , AD = 1, HLEV = 1, V23 = 1, A1 = 1, A2 = 1 */
   pSIG->fw_sig_cidsend.EN = SIG_CIDS_CTRL_DISABLE;
   pSIG->fw_sig_cidsend.AD = SIG_CIDS_CTRL_AD_ON;
   pSIG->fw_sig_cidsend.HLEV = SIG_CIDS_CTRL_HLEV_HIGH;
   pSIG->fw_sig_cidsend.V23  = SIG_CIDS_CTRL_V23_ITU_T;
   pSIG->fw_sig_cidsend.ADD_1 = 0;
   pSIG->fw_sig_cidsend.ADD_2 = SIG_CIDS_CTRL_ADD2_ON;

   ret = CmdWrite (pDev, (IFX_uint32_t *) &pSIG->fw_sig_cidsend,
                   sizeof (pSIG->fw_sig_cidsend) - CMD_HDR_CNT);
   if (ret != VMMC_statusOk)
      RETURN_STATUS (ret);
   if (pDev->err == VMMC_ERR_CERR)
      RETURN_STATUS (VMMC_statusCerr);

   /* FSK Receiver ********************************************************** */
   pSIG->fw_sig_cidrcv.EN = 0;
   ret = CmdWrite (pDev, (IFX_uint32_t *) &pSIG->fw_sig_cidrcv,
                   sizeof (pSIG->fw_sig_cidrcv) - CMD_HDR_CNT);
   if (ret != VMMC_statusOk)
      RETURN_STATUS (ret);

   return VMMC_statusOk;
}


/**
   Disables or Enables CID Sender according to bEn

   \param  pCh       Pointer to VMMC channel structure.
   \param  bEn       IFX_TRUE : enable / IFX_FALSE : disable.

   \return
      IFX_SUCCESS or IFX_ERROR
*/
static IFX_int32_t vmmc_sig_SetCidSender (VMMC_CHANNEL *pCh, IFX_boolean_t bEn)
{
   IFX_int32_t     ret   = IFX_SUCCESS;
   VMMC_DEVICE     *pDev = pCh->pParent;
   IFX_uint32_t    *pCmd, nCount;
   SIG_CIDS_CTRL_t *pCidSenderCmd = &pCh->pSIG->fw_sig_cidsend;

   pCmd = (IFX_uint32_t*) pCidSenderCmd;
   nCount = sizeof (*pCidSenderCmd) - CMD_HDR_CNT;

   if ((bEn == IFX_TRUE) && !(pCidSenderCmd->EN & SIG_CIDS_CTRL_ENABLE))
   {
      pCidSenderCmd->EN = SIG_CIDS_CTRL_ENABLE;
      pCidSenderCmd->ADD_2 = SIG_CIDS_CTRL_ADD2_ON;

      ret = CmdWrite (pDev, pCmd, nCount);
   }
   else if ((bEn == IFX_FALSE) && (pCidSenderCmd->EN & SIG_CIDS_CTRL_ENABLE))
   {
      pCidSenderCmd->EN &= ~SIG_CIDS_CTRL_ENABLE;

      ret = CmdWrite (pDev, pCmd, nCount);
   }

   return ret;
}


/**
   Disables or Enables Cid Receiver according to bEn.

   \param  pCh       Pointer to VMMC channel structure.
   \param  cidHookMode  CID hook mode as specified in IFX_TAPI_CID_HOOK_MODE_t.
   \param  pFskConf  Pointer to IFX_TAPI_CID_FSK_CFG_t structure.
   \param  nStandard CID standard selected determines the frequencies.
   \param  bEn       IFX_TRUE : enable / IFX_FALSE : disable cid receiver.

   \return
   IFX_SUCCESS or IFX_ERROR

   \remark
   The signalling channel must be enabled before this command is issued.
*/
static IFX_int32_t vmmc_sig_SetCidRec (VMMC_CHANNEL const *pCh,
                                       IFX_TAPI_CID_HOOK_MODE_t cidHookMode,
                                       IFX_TAPI_CID_FSK_CFG_t *pFskConf,
                                       IFX_TAPI_CID_STD_t nStandard,
                                       IFX_boolean_t bEn)
{
   IFX_int32_t     ret      = IFX_SUCCESS;
   IFX_uint32_t   *pCmd, nCount;
   VMMC_DEVICE    *pDev     = pCh->pParent;

   /* No configuration allowed while receiver is running -> stop first  */
   if (bEn == IFX_TRUE &&
       pCh->pSIG->fw_sig_cidrcv.EN == SIG_CIDR_CTRL_ENABLE)
   {
      pCh->pSIG->fw_sig_cidrcv.EN = SIG_CIDR_CTRL_DISABLE;
      pCmd = (IFX_uint32_t *) &pCh->pSIG->fw_sig_cidrcv;
      nCount = sizeof (pCh->pSIG->fw_sig_cidrcv) - CMD_HDR_CNT;
      ret = CmdWrite (pDev, pCmd, nCount);
   }

      /* Please keep indentation so code can be compared to drv_vinetic */
      /*lint -save -e539 */

      if (bEn == IFX_TRUE)
      {
         /* Set CID receiver coefficients */
         pCh->pSIG->fw_sig_cidr_coef.LEVEL = cid_rx_levels[ (-1)*pFskConf->levelRX / 5 ];
         /* set seizure and mark according to CID type */
         switch (cidHookMode)
         {
         case  IFX_TAPI_CID_HM_OFFHOOK:
            pCh->pSIG->fw_sig_cidr_coef.SEIZURE = 0;
            pCh->pSIG->fw_sig_cidr_coef.MARK = pFskConf->markRXOffhook;
            break;
         default:
            /* fallthrough to onhook case */
         case IFX_TAPI_CID_HM_ONHOOK:
            pCh->pSIG->fw_sig_cidr_coef.SEIZURE = pFskConf->seizureRX;
            pCh->pSIG->fw_sig_cidr_coef.MARK = pFskConf->markRXOnhook;
            break;
         }
         pCmd = (IFX_uint32_t *) &pCh->pSIG->fw_sig_cidr_coef;
         nCount = 8; /* level, seizure and mark with two byte each */
         ret = CmdWrite (pDev, pCmd, nCount);

         /* Select the frequency plan to use ITU-T V.23 or Bell 202 (Telcordia) */
         if (nStandard == IFX_TAPI_CID_STD_TELCORDIA)
         {
            pCh->pSIG->fw_sig_cidrcv.CM = 1;
         }
         else
         {
            pCh->pSIG->fw_sig_cidrcv.CM = 0;
         }
      }

      if (ret == IFX_SUCCESS)
      {
         /* Start or stop CID receiver with control command */
         pCh->pSIG->fw_sig_cidrcv.EN =
            bEn ? SIG_CIDR_CTRL_ENABLE : SIG_CIDR_CTRL_DISABLE;

         pCmd = (IFX_uint32_t *) &pCh->pSIG->fw_sig_cidrcv;
         nCount = sizeof (pCh->pSIG->fw_sig_cidrcv) - CMD_HDR_CNT;
         ret = CmdWrite (pDev, pCmd, nCount);
      }

   /*lint -restore*/

   return ret;
}

/**
   Set the CID sender coefficients

   \param  pCh       Pointer to VMMC channel structure.
   \param  cidHookMode  CID hook mode as specified in IFX_TAPI_CID_HOOK_MODE_t.
   \param  pFskConf  Pointer to IFX_TAPI_CID_FSK_CFG_t structure.
   \param  nSize     Size of CID data to send, used to set the BRS level.

   \return
   IFX_SUCCESS or Error Code

   \remark
   The CID sender must be disabled before programming the coefficients, what
   is done in this function.
*/
static IFX_int32_t vmmc_sig_SetCidCoeff (VMMC_CHANNEL *pCh,
                                         IFX_TAPI_CID_HOOK_MODE_t cidHookMode,
                                         IFX_TAPI_CID_FSK_CFG_t *pFskConf,
                                         IFX_uint8_t nSize)
{
   IFX_int32_t    ret = IFX_SUCCESS;
   VMMC_DEVICE    *pDev = pCh->pParent;
   IFX_uint32_t   *pCmd, nCount;

   /* not allowed while cid sender is active */
   ret = vmmc_sig_SetCidSender (pCh, IFX_FALSE);
   if (ret == IFX_SUCCESS)
   {
      IFX_uint32_t   nStopBits = 0;

      /* set the new coefficients */

      /* calculate level value from pFskConf->levelTX */
      pCh->pSIG->fw_sig_cids_coef.LEVEL =
         cid_tx_levels[ (-1)*pFskConf->levelTX / 5 ];
      /* set seizure, mark and stop according to CID type */
      switch (cidHookMode)
      {
      /* offhook CID, called CID type 2 */
      case  IFX_TAPI_CID_HM_OFFHOOK:
         /* set Seizure - off hook CID has always 0 seizure bits */
         pCh->pSIG->fw_sig_cids_coef.SEIZURE = 0;
         /* set Mark */
         pCh->pSIG->fw_sig_cids_coef.MARK = pFskConf->markTXOffhook;
         /* set Stop */
         nStopBits = pFskConf->stopTXOffhook;
         break;
      /* onhook CID, called CID type 1 */
      case IFX_TAPI_CID_HM_ONHOOK:
         /* set Seizure */
         pCh->pSIG->fw_sig_cids_coef.SEIZURE = pFskConf->seizureTX;
         /* set Mark */
         pCh->pSIG->fw_sig_cids_coef.MARK = pFskConf->markTXOnhook;
         /* set Stop */
         nStopBits = pFskConf->stopTXOnhook;
         break;
      }
      /* Stop: The API value is the number of additional stop bits to be sent.
         The range is 0-10 where for compatibility 0 is converted to 1 stop bit.
         The FW value is also the number of additional stop bits to be sent. */
      if (nStopBits == 0)
         nStopBits = 1;
      if (nStopBits > 10)
         nStopBits = 10;
      pCh->pSIG->fw_sig_cids_coef.STOP = nStopBits;
      /* BRS <= MAX_CID_LOOP_DATA*/
      if (nSize >= MAX_CID_LOOP_DATA)
         pCh->pSIG->fw_sig_cids_coef.BRS = MAX_CID_LOOP_DATA;
      else
         pCh->pSIG->fw_sig_cids_coef.BRS = nSize;

      pCmd = (IFX_uint32_t *) &pCh->pSIG->fw_sig_cids_coef;
      nCount = sizeof (pCh->pSIG->fw_sig_cids_coef) - CMD_HDR_CNT;

      ret = CmdWrite (pDev, pCmd, nCount);
   }
   if (ret != IFX_SUCCESS)
      TRACE (VMMC,DBG_LEVEL_HIGH, ("CID Coefficients setting failed\n"));

   return ret;
}


/**
   Do remaining setup and start the FSK generator transmission

   \param  pCh       Pointer to VMMC channel structure.

   \return
   IFX_SUCCESS or IFX_ERROR

   \remarks
   It is assumed that the CID coefficients (mark, seizure, brs, level) were set
   already accordingly by the caller of this function.
*/
static IFX_int32_t vmmc_sig_FSKG_Start (VMMC_CHANNEL *pCh)
{
   IFX_int32_t     ret  = IFX_SUCCESS;
   VMMC_DEVICE    *pDev = pCh->pParent;

   /* Reset the Tx data position */
   pCh->pSIG->cidSend.nPos     = 0;

   /* Switch on the signalling channel */
   VMMC_OS_MutexGet (&pDev->mtxMemberAcc);
   ret = VMMC_SIG_AutoChStop (pCh, IFX_TRUE);
   VMMC_OS_MutexRelease (&pDev->mtxMemberAcc);

   /* Set adders and enable CID Sender */
   if (ret == IFX_SUCCESS)
   {
      ret = vmmc_sig_SetCidSender(pCh, IFX_TRUE);
   }

   return ret;
}


/**
   Stop the FSK generator

   Does the needed settings at end of CID due to an error, an offhook event
   or the end of transmission.

   \param  pCh       Pointer to VMMC channel structure.

   \return
   IFX_SUCCESS or IFX_ERROR

   \remarks
   This function can be called from Interrupt routine and therefore should not
   be blocking.
   This function does not deactivate the global signaling channel, because
   otherwise the switching function VMMC_SIG_AutoChStop must be protected
   against interrupts. Switching of signaling channel is not a must, but
   only a recommendation. In a typical scenario more sig resources would be
   used anyway, that the switching is not mandatory.
*/
static IFX_int32_t vmmc_sig_FSKG_Stop (VMMC_CHANNEL *pCh)
{
   IFX_int32_t      ret  = IFX_SUCCESS;
   VMMC_DEVICE     *pDev = pCh->pParent;
   SIG_CIDS_CTRL_t *pCidSenderCmd = &pCh->pSIG->fw_sig_cidsend;

   /* Disable the CID sender immediately: EN = 0, AD = 0 */
   pCidSenderCmd->EN = 0;
   pCidSenderCmd->AD = 0;
   ret = CmdWriteIsr (pDev, (IFX_uint32_t*) pCidSenderCmd,
                      sizeof (*pCidSenderCmd) - CMD_HDR_CNT);

   return ret;
}


/**
   Handles requests for data from the FSK generator

   \param  pCh       Pointer to VMMC channel structure.

   \return
   IFX_SUCCESS or IFX_ERROR

   \remarks
   This function is called from Interrupt routine and therefore must not
   use blocking functions.
*/
static IFX_int32_t vmmc_sig_FSKG_Request (VMMC_CHANNEL *pCh)
{
   VMMC_DEVICE *pDev = pCh->pParent;
   VMMC_CID    *pChCid = &pCh->pSIG->cidSend;
   RES_CIDS_DATA_t *pCidSenderData = &pCh->pSIG->fw_sig_cids_data;
   IFX_uint8_t    i, len;
   IFX_int32_t    ret;
   IFX_uint32_t   *pCmd;

   /* clear the data area of the message */
   memset (pCidSenderData->DATA, 0x0, sizeof (pCidSenderData->DATA));

   /* determine how much we can send with this message */
   if ((pChCid->nCidCnt - pChCid->nPos) > MAX_CID_LOOP_DATA)
   {
      len = MAX_CID_LOOP_DATA;
   }
   else
   {
      /* last block of CID data */
      len = (pChCid->nCidCnt - pChCid->nPos);
   }
   if (len == 0)
   {
      /* no more data to send so ignore this interrupt */
      return IFX_SUCCESS;
   }

   /* copy new data to the CID sender data command */
   for (i = 0; i < len; i++)
   {
      ((IFX_uint8_t *)pCidSenderData->DATA)[i] = pChCid->pCid [pChCid->nPos + i];
   }

   /* The length will be set in the CmdWriteIsr() function. The value in the
      command length field is set to the exact length given here. The number
      of bytes written in the mailbox is the next higher multiple of four. */

   /* write command into the mailbox */
   pCmd = (IFX_uint32_t *) pCidSenderData;
   ret = CmdWriteIsr (pDev, pCmd, len);

   if (ret == IFX_SUCCESS)
   {
      /* advance the pos variable */
      pChCid->nPos += len;
      /* what to do at end of CID. */
      if (pChCid->nPos >= pChCid->nCidCnt)
      {
         SIG_CIDS_CTRL_t *pCidSenderCmd = &pCh->pSIG->fw_sig_cidsend;

         /* Disable the CID sender but playout what is still in the buffer:
            EN = 0, AD = 1 */
         pCidSenderCmd->EN = 0;
         pCidSenderCmd->AD = 1;
         ret = CmdWriteIsr (pDev, (IFX_uint32_t*) pCidSenderCmd,
                            sizeof (*pCidSenderCmd) - CMD_HDR_CNT);
      }
   }

   return ret;
}


/* ============================= */
/* Global function definition    */
/* ============================= */

/**
   CID FSK State Machine

   This function manages all CID states :
   - CID_SETUP        : generator stopped waiting for setup
   - CID_TRANSMIT     : transmission of FSK data

   \param  pCh       Pointer to VMMC channel structure.
   \param  nAction   Enum from VMMC_CID_ACTION that indicates the action.

   \return
   IFX_SUCCESS or IFX_ERROR

   \remark
   This function is called from Interrupt routine and therefore must not
   use any blocking call.
   \remark
   CID baudrate = 150 Bytes/sec without Pause.
   1 CID Word = ~ 20 ms
*/
IFX_int32_t VMMC_CidFskMachine (VMMC_CHANNEL *pCh,
                                enum VMMC_CID_ACTION nAction)
{
   IFX_int32_t    ret    = IFX_SUCCESS;
   VMMC_CID      *pCidData;

   if (pCh->pSIG == NULL)
   {
      return IFX_ERROR;
   }
   pCidData = &pCh->pSIG->cidSend;

   switch (nAction)
   {
   default:
      /*lint -fallthrough */
   case VMMC_CID_ACTION_STOP:
      if (pCidData->nState == VMMC_CID_STATE_TRANSMIT)
      {
         ret = vmmc_sig_FSKG_Stop (pCh);
      }
      /* reset CID state */
      if (ret == IFX_SUCCESS)
      {
         pCidData->nState = VMMC_CID_STATE_SETUP;
         pCidData->nCidCnt = 0;
      }
      break;

   case VMMC_CID_ACTION_START:
      /* prepare for cid only when new cid data was set */
      if ((pCidData->nState == VMMC_CID_STATE_SETUP) &&
          (pCidData->nCidCnt != 0))
      {
         ret = vmmc_sig_FSKG_Start (pCh);

         if (ret == IFX_SUCCESS)
         {
            pCidData->nState = VMMC_CID_STATE_TRANSMIT;
         }
         /* the error case is handled below */
      }

      break;

   case VMMC_CID_ACTION_REQ_DATA:
      if (pCidData->nState == VMMC_CID_STATE_TRANSMIT)
      {
         ret = vmmc_sig_FSKG_Request (pCh);
      }
      break;
   } /* switch */

   if (ret != IFX_SUCCESS)
   {
      SET_ERROR (VMMC_ERR_CID_TRANSMIT);
      /* allow CID setup on the next run */
      pCidData->nState = VMMC_CID_STATE_SETUP;
      pCidData->nCidCnt = 0;
      /* don't allow forthcoming ringing */
      TAPI_Cid_Abort (pCh->pTapiCh);
   }

   return ret;
}


/**
   DTMF CID state machine

   \param  pCh       Pointer to VMMC channel structure.

   \return
   IFX_SUCCESS or IFX_ERROR

   \remarks
   This function translates the DTMF to CID states. This function is called
   from DTMF module in ISR context.
   In case the DTMF transmission is finished or aborted, the cid flag must be
   reset for an possible subsequent sending of CID FSK on this channel.
*/
IFX_void_t VMMC_CidDtmfMachine (VMMC_CHANNEL *pCh)
{
   IFX_TAPI_EVENT_t tapiEvent;

   switch (pCh->pSIG->dtmfSend.state)
   {
   case DTMF_READY:
      memset(&tapiEvent, 0, sizeof(tapiEvent));
      tapiEvent.id = IFX_TAPI_EVENT_CID_TX_END;
      IFX_TAPI_Event_Dispatch (pCh->pTapiCh, &tapiEvent);
      break;
   case DTMF_START:
   case DTMF_TRANSMIT:
      break;
   case DTMF_ABORT:
      pCh->pSIG->dtmfSend.state = DTMF_READY;
      /* don't allow forthcoming ringing */
      TAPI_Cid_Abort (pCh->pTapiCh);
      break;
   default:
      break;
   }
}


/**
   Start the CID FSK receiver.

   \param  pLLChannel   Pointer to VMMC channel structure.
   \param  pCidData     Pointer to CID RX configuration structure.

   \return
   - VMMC_statusCmdWr Writing the command failed
   - VMMC_statusOk if successful
*/
IFX_int32_t VMMC_TAPI_LL_SIG_CID_RX_Start (IFX_TAPI_LL_CH_t *pLLChannel,
                                           IFX_TAPI_CID_RX_t const *pCidData)
{
   VMMC_CHANNEL *pCh = (VMMC_CHANNEL *) pLLChannel;
   IFX_int32_t ret;

   /* Make sure the SIG channel is active before activating the receiver. */
   ret = VMMC_SIG_AutoChStop (pCh, IFX_TRUE);

   if (VMMC_SUCCESS (ret))
   {
      /* Reset the received bytes counter */
      pCh->pSIG->nRxCount = 0;
      ret = vmmc_sig_SetCidRec (pCh, pCidData->txHookMode,
                                pCidData->pFskConf,
                                pCidData->nStandard, IFX_TRUE);
   }

   RETURN_STATUS (ret);
}


/**
   Stop the CID FSK receiver.

   \param  pLLChannel   Pointer to VINETIC channel structure.
   \param  pCid         CID stop configuration

   \return
   - VMMC_statusCmdWr Writing the command failed
   - VMMC_statusOk if successful
*/
IFX_int32_t VMMC_TAPI_LL_SIG_CID_RX_Stop (IFX_TAPI_LL_CH_t *pLLChannel,
                                          IFX_TAPI_CID_RX_CFG_t const *pCid)
{
   VMMC_CHANNEL *pCh = (VMMC_CHANNEL *) pLLChannel;
   IFX_int32_t  ret;

   VMMC_UNUSED (pCid);

   /* hookmode, pCidFskCfg and Standard are ignored but must be given */
   ret = vmmc_sig_SetCidRec (pCh, IFX_TAPI_CID_HM_ONHOOK, IFX_NULL,
                             IFX_TAPI_CID_STD_ETSI_FSK, IFX_FALSE);

   if (VMMC_SUCCESS (ret))
   {
      /* Check if the signalling channel can be deactivated */
      ret = VMMC_SIG_AutoChStop (pCh, IFX_FALSE);
   }

   RETURN_STATUS (ret);
}


/**
   Start CID data transmission

   This function is non blocking. It handles all necessary steps to transmit
   CID data by either DTMF or FSK. It returns an error if a CID transmission
   is already running.

   After triggering the DTMF transmission, the transmission will be handled
   automatically. The DTMF data will be sent on interrupt request
   and stopped on end of transmission, error or hook event.
   The callback cbDtmfStatus can be used to track the status of the DTMF
   transmission.
   The driver will convert the IFX_char_t data to DTMF words.
   This Mode only supports restricted DTMF signs 0 to D (no alert tones or pause).
   Only supports DTMF generator high level timing mode.

   \param  pLLChannel   Pointer to VMMC channel structure.
   \param  pCidData     Pointer to CID TX configuration structure.

   \return
   - VMMC_statusFuncParm Wrong parameters passed. This code is returned
     when one of the parameters has an invalid value.
   - VMMC_statusOverlay At least one other resource is in use, which is
      overlayed with the CID sender (UTG for example).
   - VMMC_statusDtmfAct
   - VMMC_statusNotSupported Requested action is not supported. This code
     is returned when an unknown CID standard shall be used.
   - VMMC_statusCidStartSeqErr   The CID sequence could not be started
   - VMMC_statusOk if successful
*/
IFX_int32_t VMMC_TAPI_LL_SIG_CID_TX_Start (IFX_TAPI_LL_CH_t *pLLChannel,
                                           IFX_TAPI_CID_TX_t const *pCidData)
{
   VMMC_CHANNEL *pCh = (VMMC_CHANNEL *) pLLChannel;
   IFX_int32_t   ret = IFX_SUCCESS;

   /* prevent data buffer overrun */
   if ( pCidData->nCidParamLen > IFX_TAPI_CID_TX_SIZE_MAX )
   {
      TRACE (VMMC, DBG_LEVEL_LOW, ("\nDRV_ERROR: "
               "Too much data for CID sender buffer - aborting\n"));
      /* errmsg: Parameter is out of range*/
      RETURN_STATUS (VMMC_statusFuncParm);
   }

   /* remember the data type for stopping the correct machine */
   pCh->pSIG->cidSend.nCidDataType = pCidData->cidDataType;

   switch (pCidData->cidDataType)
   {
   case IFX_TAPI_CID_DATA_TYPE_FSK_BEL202:
   case IFX_TAPI_CID_DATA_TYPE_FSK_V23:
      /* For event reporting in status bitfield only: Check whether UTG2
         is active, because the status bits for CIS and UTG2 are overlaid.
         => CIS cannot be used while UTG2 is active! */
      if ((pCh->pParent->caps.bEventMailboxSupported == IFX_FALSE) &&
          (pCh->pSIG->fw_utg[1].EN == SIG_CIDS_CTRL_ENABLE))
      {
         TRACE (VMMC, DBG_LEVEL_LOW, ("\nDRV_ERROR: Cid Sender "
                "cannot be used while UTG2 is active! \n"));
         /* errmsg: At least one other resource is in use,
                    which is overlayed with the CID sender*/
         RETURN_STATUS (VMMC_statusOverlay);
      }

      if (pCh->pSIG->cidSend.nState == VMMC_CID_STATE_SETUP)
      {
         /* length in bytes */
         pCh->pSIG->cidSend.nCidCnt  = pCidData->nCidParamLen;
         memcpy (pCh->pSIG->cidSend.pCid,
                 pCidData->pCidParam, pCidData->nCidParamLen);

         ret = vmmc_sig_SetCidCoeff(pCh, pCidData->txHookMode,
                                    pCidData->pFskConf,
                                    pCh->pSIG->cidSend.nCidCnt);
         /* now start non blocking state machine */
         if (ret == IFX_SUCCESS)
         {
            /* ITU-T V.23 specification shall be used */
            if (pCidData->cidDataType == IFX_TAPI_CID_DATA_TYPE_FSK_V23)
            {
               pCh->pSIG->fw_sig_cidsend.V23 = SIG_CIDS_CTRL_V23_ITU_T;
            }
            else
            /* Bellcore specification shall be used */
            {
               pCh->pSIG->fw_sig_cidsend.V23 = 0;
            }
            /* start the FSK transmission */
            ret = VMMC_CidFskMachine (pCh, VMMC_CID_ACTION_START);
         }
      }
      else
      {
         /* errmsg: A CID transmission is already active.*/
         RETURN_STATUS (VMMC_statusCidAct);
      }
      break;

   case IFX_TAPI_CID_DATA_TYPE_DTMF:
      if ((pCh->pSIG->dtmfSend.state == DTMF_READY) ||
          (pCh->pSIG->dtmfSend.state == DTMF_ABORT)    )
      {
         IFX_uint16_t  i;

         /* check digit and interdigit times */
         if ((pCidData->pDtmfConf->digitTime > MAX_DIGIT_TIME) ||
             (pCidData->pDtmfConf->interDigitTime > MAX_INTERDIGIT_TIME))
         {
            TRACE (VMMC, DBG_LEVEL_LOW, ("\nDRV_ERROR: Digit time or "
                 "inter digit time exceeds limit of 127 ms\n"));
            /* errmsg: Parameter is out of range*/
            RETURN_STATUS (VMMC_statusFuncParm);
         }
         /* Write DTMF/AT generator coefficients */
         ret = VMMC_SIG_DTMFG_CoeffSet(pCh,
                                       (pCidData->pDtmfConf->digitTime * 2),
                                       (pCidData->pDtmfConf->interDigitTime * 2));
         if (ret == IFX_SUCCESS)
         {
            /* not using cpb2w, byteorder is corrected in VMMC_CidDtmfMachine */
            memcpy (pCh->pSIG->cidSend.pCid,
                    pCidData->pCidParam, pCidData->nCidParamLen);
            /* transcode Characters A-D, # and * to FW specific setting */
            /* errors may occur if input string contains invalid characters */
            for (i=0; ret == IFX_SUCCESS && i<pCidData->nCidParamLen; i++)
            {
               ret = VMMC_SIG_DTMF_encode_ascii2fw((IFX_char_t)pCh->pSIG->cidSend.pCid[i],
                                                   pCh->pSIG->cidSend.pCid+i);
            }
            /* finally start the DTMF generator */
            if (ret == IFX_SUCCESS)
            {
               ret = VMMC_SIG_DTMFG_Start (pCh,
                                           (IFX_uint16_t *)pCh->pSIG->cidSend.pCid,
                                           pCidData->nCidParamLen, 1,
                                           VMMC_CidDtmfMachine, IFX_TRUE);
            }
         }
      }
      else
      {
         TRACE (VMMC, DBG_LEVEL_LOW,
              ("\nDRV_ERROR: Cannot start DTMF transmission while DTMF "
               "generator is already in use on this channel\n"));
         /* errmsg: A DTMF transmission is active.*/
         RETURN_STATUS (VMMC_statusDtmfAct);
      }
      break;
   default:
      /* errmsg: Feature or combination not supported*/
      RETURN_STATUS (VMMC_statusNotSupported);
   }

   if (VMMC_SUCCESS (ret))
   {
      return VMMC_statusOk;
   }
   else
   {
      /* errmsg: Initiating a CID sequence failed */
      RETURN_STATUS (VMMC_statusCidStartSeqErr);
   }
}


/**
   Stop CID data transmission

   \param  pLLChannel   Pointer to VMMC channel structure.

   \return
   - VMMC_statusNotSupported
   - VMMC_statusCidTxStopErr
   - VMMC_statusOk if successful
*/
IFX_int32_t VMMC_TAPI_LL_SIG_CID_TX_Stop (IFX_TAPI_LL_CH_t *pLLChannel)
{
   IFX_int32_t ret = VMMC_statusOk;
   VMMC_CHANNEL *pCh = (VMMC_CHANNEL *) pLLChannel;

   switch (pCh->pSIG->cidSend.nCidDataType)
   {
   case IFX_TAPI_CID_DATA_TYPE_FSK_BEL202:
   case IFX_TAPI_CID_DATA_TYPE_FSK_V23:
      ret = VMMC_CidFskMachine (pCh, VMMC_CID_ACTION_STOP);
      break;

   case IFX_TAPI_CID_DATA_TYPE_DTMF:
      if ((pCh->pSIG->dtmfSend.state == DTMF_START) ||
          (pCh->pSIG->dtmfSend.state == DTMF_TRANSMIT))
      {
         ret = irq_VMMC_SIG_DtmfStop(pCh);
      }
      break;
   default:
      /* errmsg: Feature or combination not supported*/
      RETURN_STATUS (VMMC_statusNotSupported);
   }

   if (VMMC_SUCCESS (ret))
   {
      return ret;
   }
   else
   {
      /* errmsg: CID Tx could not be stopped.*/
      RETURN_STATUS (VMMC_statusCidTxStopErr);
   }
}


/**
   CID Receiver data collection.

   \param  pChannel     Pointer to TAPI_CHANNEL structure.
   \param  pPacket      Pointer to data to be stored.
   \param  nLength      Length of data to be stored in number of bytes.

   \remarks
   This function is called by the function handling data packets in
   interrupt level. It collects the data given and stores it in a buffer
   which it gets from TAPI.
*/
IFX_return_t irq_VMMC_SIG_CID_RX_Data_Collect (TAPI_CHANNEL *pChannel,
                                               IFX_uint16_t *pPacket,
                                               IFX_uint32_t nLength)
{
   VMMC_CHANNEL *pCh = (VMMC_CHANNEL *) pChannel->pLLChannel;
   IFX_TAPI_CID_RX_DATA_t *pBuf = NULL;
   IFX_uint32_t nLen;

   /* stop processing on illegal packet length */
   if (nLength == 0 || nLength > 4)
      return IFX_ERROR;

   /* packet contains fsk payload data if length is bigger than 2 */
   if (nLength > 2)
   {
      /* packet contains fsk payload data */

      /* determine the length of data in bytes */
      nLen = nLength - 2;

      /* get a buffer with at least the length to store the received data
         This implicitely sets the HL status to IFX_TAPI_CID_RX_STATE_ONGOING
         and on no buffers sets the HL error to IFX_TAPI_CID_RX_ERROR_READ */
      pBuf = (IFX_TAPI_CID_RX_DATA_t *)TAPI_Phone_GetCidRxBuf (pChannel, nLen);
      if (pBuf != NULL)
      {
         /* at least the high byte is valid */
         pBuf->data [pBuf->nSize++] = HIGHBYTE (ntohs(pPacket[1]));
         /* if length is 4 the low byte is the second data */
         if (nLength == 4)
         {
            pBuf->data [pBuf->nSize++] = LOWBYTE (ntohs(pPacket[1]));
         }
         pCh->pSIG->nRxCount += nLen;
      }
   }

   /* evaluate from the carrier detect flag if receiver has just finished  */
   /* if CD = 1, caller id receiving is still running */
   /* if CD = 0, carrier lost. Reception of block finished */
   if ( !(ntohs(pPacket[0]) & 0x8000) )
   {
      /* event for the user : fsk reception ended */
      IFX_TAPI_EVENT_t tapiEvent;

      memset(&tapiEvent, 0, sizeof(tapiEvent));
      tapiEvent.id = IFX_TAPI_EVENT_CID_RX_END;
      /* always local due to coding */
      tapiEvent.data.cid_rx_end.local = 1;
      tapiEvent.data.cid_rx_end.number = pCh->pSIG->nRxCount;
      IFX_TAPI_Event_Dispatch(pChannel,&tapiEvent);
      pCh->pSIG->nRxCount = 0;
   }

   return IFX_SUCCESS;
}

#endif /* TAPI_CID */
