/******************************************************************************

                            Copyright (c) 2006-2015
                        Lantiq Beteiligungs-GmbH & Co.KG
                             http://www.lantiq.com

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/**
   \file drv_vmmc_init_cap.c
   This file implements the capability reporting.
*/

/* ============================= */
/* includes                      */
/* ============================= */
#include "drv_api.h"
#include "drv_vmmc_init.h"
#include "drv_mps_vmmc.h"
#include "drv_mps_vmmc_device.h"
#include "drv_vmmc_announcements.h"

/* ============================= */
/* Configuration defintions      */
/* ============================= */
/* Maximum number of capabilities, used for allocating the array.
   Increase if needed, but keep as small as possible in order to save memory.
   Please note that 44 is the limit that fits into 4kB which is a usual memory
   page size on most platforms. */
#define MAX_CAPS              44

/* ============================= */
/* Local Macros & Definitions    */
/* ============================= */
#define MIN(x,y) ({ (x) < (y) ? (x) : (y); })

/* ============================= */
/* Local function declaration    */
/* ============================= */
static IFX_void_t AddCapability (IFX_TAPI_CAP_t* CapList,
                                 IFX_uint32_t *pnCap,
                                 IFX_char_t const * description,
                                 IFX_int32_t type, IFX_int32_t value);


/* ============================= */
/* Function definitions          */
/* ============================= */

/**
   Set all capabilities.

   \param  pDev         Pointer to the device structure.

   \return
   IFX_SUCCESS or IFX_ERROR if no memory is available

   \remarks
   Macro MAX_CAPS must match with the capabilities number. So adapt this macro
   accordingly if new capabilities are added.
*/
IFX_int32_t VMMC_AddCaps (VMMC_DEVICE *pDev)
{
   /* capability list */
   IFX_TAPI_CAP_t *CapList;

   /* count the number of entries */
   IFX_uint32_t nCap = 0;

   if (pDev->CapList == NULL)
   {
      pDev->CapList = (IFX_TAPI_CAP_t*)
                      VMMC_OS_Malloc (MAX_CAPS * sizeof(IFX_TAPI_CAP_t));
      if (pDev->CapList == NULL)
      {
         SET_DEV_ERROR (VMMC_ERR_NO_MEM);
         return IFX_ERROR;
      }
   }

   CapList = pDev->CapList;

   AddCapability (CapList, &nCap, "LANTIQ",
                  IFX_TAPI_CAP_TYPE_VENDOR, 0);
   AddCapability (CapList, &nCap, "VMMC",
                  IFX_TAPI_CAP_TYPE_DEVICE, 6);
   AddCapability (CapList, &nCap, "POTS",
                  IFX_TAPI_CAP_TYPE_PORT, IFX_TAPI_CAP_PORT_POTS);
   AddCapability (CapList, &nCap, "PSTN",
                  IFX_TAPI_CAP_TYPE_PORT, IFX_TAPI_CAP_PORT_PSTN);
   AddCapability (CapList, &nCap, "DEVICE TYPE",
                  IFX_TAPI_CAP_TYPE_DEVTYPE, VMMC_DEV_TYPE);
   AddCapability (CapList, &nCap, "DEVICE VERSION", IFX_TAPI_CAP_TYPE_DEVVERS,
                  0x0100 + IFX_MPS_CHIPID_VERSION_GET(*IFX_MPS_CHIPID));

   if (pDev->bCapsRead)
   {
      AddCapability (CapList, &nCap, "DSP", IFX_TAPI_CAP_TYPE_DSP, 0);

      if (pDev->caps.CODECS & CODEC_G726)
      {
         AddCapability (CapList, &nCap, "G.726 16 kbps",
                        IFX_TAPI_CAP_TYPE_CODEC, IFX_TAPI_ENC_TYPE_G726_16);
         AddCapability (CapList, &nCap, "G.726 24 kbps",
                        IFX_TAPI_CAP_TYPE_CODEC, IFX_TAPI_ENC_TYPE_G726_24);
         AddCapability (CapList, &nCap, "G.726 32 kbps",
                        IFX_TAPI_CAP_TYPE_CODEC, IFX_TAPI_ENC_TYPE_G726_32);
         AddCapability (CapList, &nCap, "G.726 40 kbps",
                        IFX_TAPI_CAP_TYPE_CODEC, IFX_TAPI_ENC_TYPE_G726_40);
      }
      if (pDev->caps.CODECS & CODEC_G711)
      {
         AddCapability (CapList, &nCap, "u-LAW",
                        IFX_TAPI_CAP_TYPE_CODEC, IFX_TAPI_ENC_TYPE_MLAW);
         AddCapability (CapList, &nCap, "A-LAW",
                        IFX_TAPI_CAP_TYPE_CODEC, IFX_TAPI_ENC_TYPE_ALAW);
         AddCapability (CapList, &nCap, "u-LAW VBD",
                        IFX_TAPI_CAP_TYPE_CODEC, IFX_TAPI_COD_TYPE_MLAW_VBD);
         AddCapability (CapList, &nCap, "A-LAW VBD",
                        IFX_TAPI_CAP_TYPE_CODEC, IFX_TAPI_COD_TYPE_ALAW_VBD);
      }
      if (pDev->caps.CODECS & CODEC_G723_1)
      {
         AddCapability (CapList, &nCap, "G.723 6.3kbps",
                        IFX_TAPI_CAP_TYPE_CODEC, IFX_TAPI_ENC_TYPE_G723_63);
         AddCapability (CapList, &nCap, "G.723 5.3kbps",
                        IFX_TAPI_CAP_TYPE_CODEC, IFX_TAPI_ENC_TYPE_G723_53);
      }
      if (pDev->caps.CODECS & CODEC_G729AB)
      {
         AddCapability (CapList, &nCap, "G.729",
                        IFX_TAPI_CAP_TYPE_CODEC, IFX_TAPI_ENC_TYPE_G729);
      }
      if (pDev->caps.CODECS & CODEC_G729E)
      {
         AddCapability (CapList, &nCap, "G.729E",
                        IFX_TAPI_CAP_TYPE_CODEC, IFX_TAPI_ENC_TYPE_G729_E);
      }
      if (pDev->caps.CODECS & CODEC_ILBC)
      {
         AddCapability (CapList, &nCap, "iLBC 13.3 kbps",
                        IFX_TAPI_CAP_TYPE_CODEC, IFX_TAPI_ENC_TYPE_ILBC_133);
         AddCapability (CapList, &nCap, "iLBC 15.2 kbps",
                        IFX_TAPI_CAP_TYPE_CODEC, IFX_TAPI_ENC_TYPE_ILBC_152);
      }
      if (pDev->caps.CODECS & CODEC_G722)
      {
         AddCapability (CapList, &nCap, "G.722 64 kbps",
                        IFX_TAPI_CAP_TYPE_CODEC, IFX_TAPI_ENC_TYPE_G722_64);
      }
      if (pDev->caps.CODECS & CODEC_G722_1)
      {
         AddCapability (CapList, &nCap, "G.722 24 kbps",
                        IFX_TAPI_CAP_TYPE_CODEC, IFX_TAPI_ENC_TYPE_G7221_24);
         AddCapability (CapList, &nCap, "G.722 32 kbps",
                        IFX_TAPI_CAP_TYPE_CODEC, IFX_TAPI_ENC_TYPE_G7221_32);
      }
      if (pDev->caps.CODECS & CODEC_AMR_NB)
      {
         AddCapability (CapList, &nCap, "AMR-NB",
                        IFX_TAPI_CAP_TYPE_CODEC, IFX_TAPI_ENC_TYPE_AMR_NB);
      }
      if (pDev->caps.CODECS & CODEC_AMR_WB)
      {
         AddCapability (CapList, &nCap, "G722.2, AMR-WB",
                        IFX_TAPI_CAP_TYPE_CODEC, IFX_TAPI_ENC_TYPE_AMR_WB);
      }
      if (pDev->caps.CODECS & CODEC_L16)
      {
         AddCapability (CapList, &nCap, "Lin 16 Bit 8 KHz",
                        IFX_TAPI_CAP_TYPE_CODEC, IFX_TAPI_ENC_TYPE_LIN16_8);
      }
      if (pDev->caps.CODECS & CODEC_L16_16)
      {
         AddCapability (CapList, &nCap, "Lin 16 Bit 16 KHz",
                        IFX_TAPI_CAP_TYPE_CODEC, IFX_TAPI_ENC_TYPE_LIN16_16);
      }
      if (pDev->caps.nFAX != 0)
      {
         AddCapability (CapList, &nCap, "T.38",
                        IFX_TAPI_CAP_TYPE_T38, pDev->caps.nFAX);
      }
      if (pDev->caps.CODECS & CODEC_G728)
      {
         AddCapability (CapList, &nCap, "G.728",
                        IFX_TAPI_CAP_TYPE_CODEC, IFX_TAPI_ENC_TYPE_G728);
      }

      AddCapability (CapList, &nCap, "Coder",
                     IFX_TAPI_CAP_TYPE_CODECS, pDev->caps.nCOD);
      AddCapability (CapList, &nCap, "PCM",
                     IFX_TAPI_CAP_TYPE_PCM,
#ifdef VMMC_FEAT_PCM
                     pDev->caps.nPCM);
#else
                     0);
#endif /* VMMC_FEAT_PCM */
      AddCapability (CapList, &nCap, "Phones",
                     IFX_TAPI_CAP_TYPE_PHONES, pDev->caps.nFXS);
      AddCapability (CapList, &nCap, "SIG",
                     IFX_TAPI_CAP_TYPE_SIGDETECT, pDev->caps.nSIG);
      AddCapability (CapList, &nCap, "DECT",
                     IFX_TAPI_CAP_TYPE_DECT,
#ifdef DECT_SUPPORT
                     pDev->caps.nDECT);
#else
                     0);
#endif /* DECT_SUPPORT */
      AddCapability (CapList, &nCap, "FXO",
                     IFX_TAPI_CAP_TYPE_FXO, pDev->caps.nFXO);

      /* DC/DC converter type */
      AddCapability (CapList, &nCap, "DC/DC type",
                     IFX_TAPI_CAP_TYPE_DCDC,
                     pDev->sdd.nAllowedDcDcType - VMMC_DCDC_TYPE_IBB);
   }

#ifdef VMMC_FEAT_ANNOUNCEMENTS
    AddCapability (CapList, &nCap, "Number of announcements",
        IFX_TAPI_CAP_TYPE_NAN, VMMC_ANNOUNCEMENTS_MAX);
    AddCapability (CapList, &nCap, "Max announcement size",
        IFX_TAPI_CAP_TYPE_ANS, VMMC_ANNOUNCEMENTS_SIZE_MAX);
#endif

   /* check that array is not out of bounds */
   VMMC_ASSERT (nCap <= MAX_CAPS);
   pDev->nMaxCaps = (IFX_uint8_t)nCap;

   return IFX_SUCCESS;
}


/**
   Add capability to given list.

   Writes a capability to the capability list entry that is adressed with
   nCap and increments nCap afterwards. Previous data in this slot is
   overwritten.

   \param  CapList      Pointer to the capability list.
   \param  pnCap        Pointer to an integer used as index to the list.
                        The integer is automatically postincremented by one.
   \param  description  C-string with a text describing the capability.
   \param  type         Type ID of the capability
   \param  value        Value (amount) of the capability.
*/
static IFX_void_t AddCapability (IFX_TAPI_CAP_t* CapList,
                                 IFX_uint32_t *pnCap,
                                 IFX_char_t const * description,
                                 IFX_int32_t type, IFX_int32_t value)
{
   IFX_uint32_t capnr;

   if (pnCap  == NULL)
      return;

   if (*pnCap >= MAX_CAPS)
      return;

   capnr = (*pnCap);
   /* Note: strncpy will terminate the target string with \0 if the source
      string is longer than length parameter */
   strncpy (CapList[capnr].desc, description, sizeof (CapList[0].desc));
   CapList[capnr].captype = (IFX_TAPI_CAP_TYPE_t)type;
   CapList[capnr].cap = value;
   CapList[capnr].handle = (int)capnr;
   (*pnCap) = capnr + 1;
   TRACE (VMMC, DBG_LEVEL_LOW, ("Cap: %s , type %d = %d size=%d\n",
          description, type, value, strlen(description)));
}


/**
   Checks in the capability list if a specific capability is supported.

   \param  pLLDev       Pointer to VMMC_DEVICE structure.
   \param  pCapList     Pointer to IFX_TAPI_CAP_t structure.

   \return Support status of the capability
   - 0 if not supported
   - 1 if supported

   \remarks
   This function compares only the captype and the cap members of the given
   IFX_TAPI_CAP_t structure with the ones of the VMMC_CAP_Table.
*/
IFX_int32_t TAPI_LL_Phone_Check_Capability (IFX_TAPI_LL_DEV_t *pLLDev,
                                            IFX_TAPI_CAP_t *pCapList)
{
   VMMC_DEVICE *pDev = pLLDev;
   IFX_int32_t cnt;
   IFX_int32_t ret = 0;

   /* do checks */
   for (cnt = 0; cnt < pDev->nMaxCaps; cnt++)
   {
      if (pCapList->captype == pDev->CapList[cnt].captype)
      {
         switch (pCapList->captype)
         {
         /* Handle number counters, cap is returned */
         case IFX_TAPI_CAP_TYPE_PCM:
         case IFX_TAPI_CAP_TYPE_CODECS:
         case IFX_TAPI_CAP_TYPE_PHONES:
         case IFX_TAPI_CAP_TYPE_SIGDETECT:
         case IFX_TAPI_CAP_TYPE_DECT:
         case IFX_TAPI_CAP_TYPE_T38:
         case IFX_TAPI_CAP_TYPE_DEVVERS:
         case IFX_TAPI_CAP_TYPE_DEVTYPE:
         case IFX_TAPI_CAP_TYPE_FXO:
         case IFX_TAPI_CAP_TYPE_DCDC:
            pCapList->cap = pDev->CapList[cnt].cap;
            ret = 1;
            break;
         /* strings are returned in the "desc" field */
         case IFX_TAPI_CAP_TYPE_VENDOR:
         case IFX_TAPI_CAP_TYPE_DEVICE:
            strncpy (pCapList->desc, pDev->CapList[cnt].desc, sizeof (pCapList->desc));
            ret = 1;
            break;
         default:
            /* default IFX_TRUE or IFX_FALSE capabilities */
            if (pCapList->cap == pDev->CapList[cnt].cap)
               ret = 1;
            break;
         }
      }
   }
   return ret;
}


/**
   Returns the number of entries in the capability list.

   \param  pLLDev       Pointer to VMMC_DEVICE structure.

   \return The number of capability entries.
*/
IFX_int32_t TAPI_LL_Phone_Get_Capabilities (IFX_TAPI_LL_DEV_t *pLLDev)
{
   return (((VMMC_DEVICE *) pLLDev)->nMaxCaps);
}


/**
   Returns Vmmc's capability lists.

   \param  pLLDev       Pointer to VMMC_DEVICE structure.
   \param  pCapList     Pointer to IFX_TAPI_CAP_LIST_t structure with details
                        where to copy the data to. No more capabilities than
                        specified in the element nCap will be copied into the
                        memory given in this structure.

   \return Return value according to IFX_return_t:
   - IFX_SUCCESS        Always successful.
*/
IFX_int32_t TAPI_LL_Phone_Get_Capability_List (
                        IFX_TAPI_LL_DEV_t *pLLDev,
                        IFX_TAPI_CAP_LIST_t *pCapList)
{
   VMMC_DEVICE *pDev = pLLDev;

   /* The count of entries given in the parameter is the limit. Should the
      internal list have less entries than this reduce the number of entries
      to be copied to this smaller number. */
   if (pDev->nMaxCaps < pCapList->nCap)
   {
      pCapList->nCap = pDev->nMaxCaps;
   }

   VMMC_OS_CpyKern2Usr (
      pCapList->pList,
      pDev->CapList, /*lint !e64 */
      pCapList->nCap * sizeof (*pDev->CapList));

   return IFX_SUCCESS;
}


/**
   Read out the Capabilities from firmware.

   \param  pDev         Pointer to the device structure.

   \return
   IFX_SUCCESS or IFX_ERROR.
*/
IFX_return_t VMMC_Get_FwCap(VMMC_DEVICE *pDev)
{
   IFX_int32_t ret = IFX_ERROR;
   SYS_CAP_t    capCmd;

   pDev->bCapsRead = IFX_FALSE;

   /* Initialise the firmware command for reading the capabilities */
   memset((IFX_void_t *)&capCmd, 0, sizeof(SYS_CAP_t));

   /* Command Header */
   capCmd.CMD    = CMD_EOP;
   capCmd.MOD    = MOD_SYSTEM;
   capCmd.ECMD   = SYS_CAP_ECMD;
   capCmd.LENGTH = 4;

   /* First read only first 4 bytes which contain the current version
      and the actual length of the message. */
   ret = CmdRead(pDev, (IFX_uint32_t *)&capCmd,
                       (IFX_uint32_t *)&capCmd, capCmd.LENGTH);

   if (ret == IFX_SUCCESS)
   {
      /* Read the maximum length of the message that we can interpret but
         not more than the firmware can provide. */
      /* BLEN is the length including the header that the fw reports. */
      capCmd.LENGTH = MIN (capCmd.BLEN, sizeof(SYS_CAP_t));
      /* Subtract the length of the header. */
      capCmd.LENGTH -= CMD_HDR_CNT;
      /* Read capability once more - this time with maximum length. */
      ret = CmdRead(pDev, (IFX_uint32_t *)&capCmd,
                          (IFX_uint32_t *)&capCmd, capCmd.LENGTH);
   }

   if (ret != IFX_SUCCESS)
   {
      TRACE (VMMC, DBG_LEVEL_LOW,
            ("INFO: Firmware capabilities-message not supported\n"));
   }
   else /* Store the capabilities*/
   {
      pDev->bCapsRead = IFX_TRUE;

      /* Our internal structure is different from the firmware message to
         be more flexible. So when the firmware message changes we can
         handle different versions of the message by first looking to the
         version field before copying the values. Also some different
         interpretation of values is possible. */

      /* Number of PCM Channels */
      pDev->caps.nPCM = capCmd.NPCM;
      /* Number of Analog Line Channels */
      pDev->caps.nALI = capCmd.NALI;
      /* Number of Signaling Channels */
      pDev->caps.nSIG = capCmd.NSIG;
      /* Number of Coder Channels */
      pDev->caps.nCOD = capCmd.NCOD;
      /* Number of AGCs */
      pDev->caps.nAGC = capCmd.NAGC;
      /* Number of Equalizers */
      pDev->caps.nEQ = capCmd.NEQ;
      /* Number of Near-End LECs */
      pDev->caps.nNLEC = capCmd.NNLEC;
      /* Number of Combined Near-End/Far-End LECs */
      pDev->caps.nWLEC = capCmd.NWLEC;
      /* Number of Near-End Wideband LECs */
      pDev->caps.nNWLEC = capCmd.NNWLEC;
      /* Number of Combined Near-End/Far-End Wideband LECs */
      pDev->caps.nWWLEC = capCmd.NWWLEC;
      /* Number of Universal Tone Generators */
      pDev->caps.nUTG = capCmd.NUTG;
      /* Number of DTMF Generators */
      pDev->caps.nDTMFG = capCmd.NDTMFG;
      /* Number of Caller ID Senders */
      pDev->caps.nCIDS = capCmd.NCIDS;
      /* Number of Caller ID Receivers */
      pDev->caps.nCIDR = capCmd.NCIDR;
      /* Number of Call Progress Tone Detectors */
      pDev->caps.nCPTD = capCmd.NCPTD;
      /* Number of Call Progress Tone Detectors per channel */
      pDev->caps.nCptdPerCh = capCmd.CPTD3 ? 3 : 1;
      /* Number of Modem and Fax Tone Discriminators (MFTDs) */
      pDev->caps.nMFTD = capCmd.NMFTD;
      /* Number of FAX Channels with FAX Relay (T.38) Support */
      pDev->caps.nFAX = capCmd.NFAX;
      /** Number of DTMF Detectors */
      pDev->caps.nDTMFD = capCmd.NDTMFD;
      /* Number of "PCM shortcuts" */
      pDev->caps.nPCMS = capCmd.PCMS;
      /* Number of HDLC framers for D-channel access */
      pDev->caps.nHDLC = capCmd.DCHAN;
      /* Codecs */
      pDev->caps.CODECS = capCmd.CODECS;
      /* Maximum Number of Low Complexity Coders for the Coder Channel */
      pDev->caps.CLOW = capCmd.CLOW;
      /* Maximum Number of Mid Complexity Coders for the Coder Channel */
      pDev->caps.CMID = capCmd.CLOW;
      /* Maximum Number of High Complexity Coders for the Coder Channel*/
      pDev->caps.CMAX = capCmd.CMAX;
      /* PCM Channel Coders */
      pDev->caps.PCOD = capCmd.PCOD;
      /* MFTD Version */
      pDev->caps.MFTDV = capCmd.MFTDV;
      /** Number of DECT Channels */
      pDev->caps.nDECT = capCmd.NDECT;
      /** DECT codecs */
      pDev->caps.DECT_CODECS = capCmd.DECT_CODECS;
      /** Echo Suppressor in analog line channel */
      pDev->caps.ES = capCmd.ES;
      /* Tone Detection Capabilities */
      pDev->caps.TONES = capCmd.TONES;
      /* Features */
      pDev->caps.FEAT = capCmd.FEAT;
      /* Number of UTG resources per channel (== SIG module), either 1 or 2 */
      pDev->caps.nUtgPerCh = (capCmd.FEAT & EDSP_CAP_FEAT_UTGUD) ? 2 : 1;
      pDev->caps.nUtgPerCh = MIN(pDev->caps.nUtgPerCh, LL_TAPI_TONE_MAXRES);
      /* Number of AudioChannels */
      pDev->caps.nAudioCnt = (pDev->caps.FEAT & EDSP_CAP_FEAT_CHAUD) ? 1 : 0;
      /* Overlays */
      pDev->caps.OVL = capCmd.OV;
      /* Event Playout Capabilities */
      pDev->caps.EPC = capCmd.EPC;
      /* Event Transmission Capabilities */
      pDev->caps.ETC = capCmd.ETC;

      /* Support for extended jitter buffer statistics is yes by default */
      pDev->caps.bExtendedJBsupported = IFX_TRUE;
      /* Support for jitter buffer enhancements */
      pDev->caps.bEnhancedJB = capCmd.JB1 ? 1 : 0;

      /** DTMF receiver enhancements: 0=no 1=yes */
      pDev->caps.bDT1 = capCmd.DT1 ? 1 : 0;
      /** DTMF receiver enhancements, step 1: 0=no 1=yes */
      pDev->caps.bDT2 = capCmd.DT2 ? 1 : 0;
      /** Support of RFC 4040 clearmode: 0=no 1=yes */
      pDev->caps.bRFC4040supported = capCmd.RFC4040 ? 1 : 0;
      /** Support of Echo Suppressor enhancements: 0=no 1=yes */
      pDev->caps.bEnhancedES = capCmd.ESE ? 1 : 0;

      /* set the number of NLEC equal to the number of WLEC when supported */
      if (pDev->caps.nWLEC > 0)
      {
         pDev->caps.nNLEC = pDev->caps.nWLEC;
      }

      /* Number of echo suppressor ressources */
      pDev->caps.nES = capCmd.NES;
      if (pDev->caps.nES > 0)
      {
         /* Versions with the NES field support ES on both ALM and PCM. */
         pDev->caps.bESonALM = 1;
         pDev->caps.bESonPCM = 1;
      }
      /* The number of echo suppressor resources is only availabe when the
         message length is 44 bytes or more. In older versions we look for
         the ES bit which indicates that there is one ES for every ALM.
         This code is actually never used and just for compatibility. */
      if ((capCmd.BLEN < 44) && (capCmd.ES))
      {
         pDev->caps.nES = capCmd.NALI;
         /* ES support was availabe only on ALM in these old versions. */
         pDev->caps.bESonALM = 1;
      }
      /* Number of LIN Channels */
      pDev->caps.nLIN = capCmd.NLIN;
      /** T.38 stack is implemented in firmware */
      pDev->caps.bT38FW = capCmd.T38FW;
      /** Number of PCM codec resources */
      pDev->caps.nPCMCOD = capCmd.PCMCOD;
      /** Support of the idle pattern in the D-Channel */
      pDev->caps.bDIP = capCmd.DIP;
      /** Support of the DECT channel Echo Suppressor */
      pDev->caps.bESonDECT = capCmd.DES;
      /** Support of jitter adaptation during silence */
      pDev->caps.bJAS = capCmd.JAS;
      /** Support of Echo canceler additions step 1 (Coefficient freeze) */
      pDev->caps.bECA1 = capCmd.ECA1;
      /** Support of EMOS calculation in FW */
      pDev->caps.bEMOS = capCmd.EMOS;
      /** Support of MWI (SDD feature) */
      pDev->caps.bMWI = capCmd.MWI;
      /** Support of extended firmware version */
      pDev->caps.bVerExtTs = capCmd.EVER;
      /** Support of handshake mechanism for Clock Scaling */
      pDev->caps.bCLKHS = capCmd.CLKHS;
      /** Support of RTP Control Protocol Extended Reports (RTCP XR) */
      pDev->caps.bRtcpXR = capCmd.XR;
      /** AMR Encoder
          0 - old style,
          1 - bit rate can be set in a separate bitfield */
      pDev->caps.bAMRE = capCmd.AMRE;
      /** Support of PCM split timeslots */
      pDev->caps.bPcmSplitTs = capCmd.PSTS;
      /* Support of timestamps in hook events */
      pDev->caps.bHTS = capCmd.HTS;
      /* Support for burst number extension in RTCP XR VoIP Metrics Report. */
      pDev->caps.bXR_BN = capCmd.XR_BN;
   }

   return  (IFX_SUCCESS == ret) ? IFX_SUCCESS : IFX_ERROR;
}
