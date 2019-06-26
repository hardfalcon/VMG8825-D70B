/******************************************************************************

                              Copyright (c) 2013
                            Lantiq Deutschland GmbH
                             http://www.lantiq.com

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

******************************************************************************/

/**
   \file drv_vmmc_ioctl.c
   Contains ioctl specific implementations according to io type.
*/

/* ============================= */
/* Includes                      */
/* ============================= */
#include "drv_api.h"
#include "drv_vmmc_api.h"
#include "drv_vmmc_bbd.h"

#ifdef VMMC_FEAT_GR909
#include "drv_vmmc_gr909.h"
#endif /* VMMC_FEAT_GR909  */

#include "drv_vmmc_access.h"
#include "drv_vmmc_fw_commands_sdd.h"
#include "drv_mps_vmmc.h"
#include "drv_mps_vmmc_device.h"
#include "drv_vmmc_bbd.h"
#include "drv_vmmc_alm.h"

#include "drv_version.h"

#ifdef VMMC_FEAT_CLOCK_SCALING
#include "drv_vmmc_pmc.h"
#endif /* VMMC_FEAT_CLOCK_SCALING */

/* ============================= */
/* Local Macros & Definitions    */
/* ============================= */

/* ============================= */
/* Type definitions              */
/* ============================= */

/* ============================= */
/* Function declaration          */
/* ============================= */
static IFX_int32_t VMMC_Read_Cmd (
                        VMMC_DEVICE *pDev,
                        VMMC_IO_MB_CMD* pCmd);
static IFX_int32_t VMMC_Write_Cmd (
                        VMMC_DEVICE *pDev,
                        VMMC_IO_MB_CMD* pCmd);
static IFX_int32_t VMMC_RegRd_Cmd (
                        VMMC_DEVICE *pDev,
                        VMMC_IO_REG_ACCESS *pCmd);
static IFX_int32_t VMMC_RegWr_Cmd (
                        VMMC_DEVICE *pDev,
                        VMMC_IO_REG_ACCESS *pCmd);
extern IFX_int32_t VMMC_ChipAccessInit (
                        VMMC_DEVICE *pDev);

/* ============================= */
/* Function definitions          */
/* ============================= */

/**
   Dispatches ioctrl commands and manages user space data handling in both
   directions.

\param   pContext - context pointer, may be device or channel pointer.
\param   msg      - ioctrl command id
\param   func     - function name
\param   arg      - structure identifier used as argument for the function
                    call and for copying user data
\remarks
   As an alternative use a transfer structure VINETIC_IO_USR and
   DoDataExchange for handling data exchange.

\code
   // call function VINETIC_Read_Cmd if id VINETIC_RMAIL received
   IFX_uint8_t IOcmd [660];

   switch (nCmd)
   {
      ON_IOCTL(VINETIC_RMAIL, VINETIC_Read_Cmd, IO_READ_CMD);
   }
\endcode
*/
#ifdef LINUX
#define ON_IOCTL(pContext,msg,func,arg)\
           case (msg):\
              {\
                 arg* p_arg = VMMC_OS_Malloc (sizeof(arg));\
                 if (p_arg != IFX_NULL) {\
                    copy_from_user (p_arg, (IFX_uint8_t*)ioarg, sizeof(arg));\
                    ret = func((pContext), p_arg);\
                    copy_to_user ((IFX_uint8_t*)ioarg, p_arg, sizeof(arg));\
                    VMMC_OS_Free (p_arg);\
                 }\
                 else\
                 {\
                    ret = VMMC_statusNoMem;\
                 }\
              }\
              break
#else
#define ON_IOCTL(pContext,msg,func,arg)\
           case (msg):\
              ret = func((pContext),(arg*)ioarg);\
              break
#endif /* LINUX */

/**
   Read VMMC command.

   \param  pDev         Pointer to device data.
   \param  pCmd         Pointer to command data.

   \return
   IFX_SUCCESS if no error, otherwise IFX_ERROR.
*/
static IFX_int32_t VMMC_Read_Cmd (VMMC_DEVICE *pDev, VMMC_IO_MB_CMD* pCmd)
{
   IFX_int32_t err;

   err = CmdRead (pDev, (IFX_uint32_t*)((IFX_void_t *)pCmd),
                  (IFX_uint32_t*)((IFX_void_t *)pCmd), pCmd->cmd & CMD_LEN);

   /* if read failed set the cmd length to 0 */
   if (err != IFX_SUCCESS)
      pCmd->cmd &= 0xffffff00;

   return err;
}


/**
   Write VMMC command.

   \param  pDev         Pointer to device data.
   \param  pCmd         Pointer to command data.

   \return
   IFX_SUCCESS if no error, otherwise IFX_ERROR (return value of CmdWrite).
*/
static IFX_int32_t VMMC_Write_Cmd (VMMC_DEVICE *pDev, VMMC_IO_MB_CMD* pCmd)
{
   IFX_int32_t  ret;
   IFX_uint32_t *pData = (IFX_uint32_t*)((IFX_void_t *)pCmd);

   ret = CmdWrite (pDev, pData, (pCmd->cmd & CMD_LEN));

   /* if read failed set the cmd length to 0 */
   if (ret != IFX_SUCCESS)
      pCmd->cmd &= 0xffffff00;

   return ret;
}


/**
   Ioctl handling function used to read a single host register.

   \param  pDev         Pointer to device data.
   \param  pCmd         Pointer to command data.

   \return
   Returns IFX_SUCCESS on success and IFX_ERROR if length in command exceeds
   the possible data length.
*/
/*lint -esym(715, pDev)
   Parameter not needed because currently just one device is supported. */
static IFX_int32_t VMMC_RegRd_Cmd(VMMC_DEVICE *pDev,
                                  VMMC_IO_REG_ACCESS *pCmd)
{
   int i;
   unsigned short offset = pCmd->offset;

   VMMC_UNUSED(pDev);

   if (pCmd->count > (sizeof(pCmd->pData) / sizeof(pCmd->pData[0])) )
   {
      return IFX_ERROR;
   }

   for (i=0; i < pCmd->count; i++)
   {
      pCmd->pData[i] = VMMC_READ_REG (offset);
      offset += 4;
   }

   return IFX_SUCCESS;
}


/**
   Ioctl handling function used to write a single host register.

   \param  pDev         Pointer to device data.
   \param  pCmd         Pointer to command data.

   \return
   Returns IFX_SUCCESS on success and IFX_ERROR if length in command exceeds
   the possible data length.
*/
static IFX_int32_t VMMC_RegWr_Cmd(VMMC_DEVICE *pDev,
                                  VMMC_IO_REG_ACCESS *pCmd)
{
   int i;
   unsigned short offset = pCmd->offset;

   VMMC_UNUSED(pDev);

   if (pCmd->count > (sizeof(pCmd->pData) / sizeof(pCmd->pData[0])) )
   {
      return IFX_ERROR;
   }

   for (i=0; (i < pCmd->count); i++)
   {
      VMMC_WRITE_REG (offset, pCmd->pData[i]);
      offset += 4;
   }

   return IFX_SUCCESS;
}
/*lint +esym(715, pDev)*/


/**
   VMMC device specific ioctl handling

   \param  pLLDummyCh   Pointer to either device or channel struct.
   \param  iocmd        IOCTL identifier.
   \param  ioarg        IOCTL argument.

   \return
   - VMMC_statusOk      if successful
   - VMMC_statusNoMem   if no memory for temporary structs
   - VMMC_statusInvalidIoctl this IOCTL is not handled
*/
IFX_int32_t VMMC_Dev_Spec_Ioctl (IFX_TAPI_LL_CH_t *pLLDummyCh,
                                 IFX_uint32_t iocmd,
                                 IFX_ulong_t ioarg)
{
   IFX_int32_t     ret   = VMMC_statusOk;
   VMMC_CHANNEL   *pCh;
   VMMC_DEVICE    *pDev;

#ifndef TAPI_ONE_DEVNODE
   pCh   = (VMMC_CHANNEL *) pLLDummyCh;

   /* distinguish device / channel context */
   if (pCh->nChannel == 0)
   {
      /* initial assumption is wrong - we received a device pointer */
      pDev = (VMMC_DEVICE *) pLLDummyCh;
      pCh  = IFX_NULL;
   }
   else
   {
      /* initial assumption was correct so we only need to get the
         corresponding device pointer */
      pDev = pCh->pParent;
   }
#else
   VMMC_GetDevice (0, &pDev);
   pCh = IFX_NULL;
#endif

   switch(iocmd)
   {
      case FIO_GET_VERS:
      {
         VMMC_IO_VERSION *pVers;
         SYS_VER_t *pCmd;

         pVers = VMMC_OS_Malloc (sizeof(VMMC_IO_VERSION));
         if (pVers == IFX_NULL)
         {
            ret = VMMC_statusNoMem;
            break;
         }

         pCmd = VMMC_OS_Malloc (sizeof(SYS_VER_t));
         if (pCmd == IFX_NULL)
         {
            VMMC_OS_Free (pVers);
            ret = VMMC_statusNoMem;
            break;
         }

         memset((IFX_void_t *)pVers, 0, sizeof(VMMC_IO_VERSION));
         if (pDev->nDevState & DS_FW_DLD)
         {
            IFX_uint16_t length;

            memset((IFX_void_t *)pCmd, 0, sizeof(SYS_VER_t));
            /* Command Header */
            pCmd->CMD = CMD_EOP;
            pCmd->MOD = MOD_SYSTEM;
            pCmd->ECMD = SYS_VER_ECMD;

            length = SYS_VER_LEN_BASIC;
            if (pDev->bCapsRead && pDev->caps.bVerExtTs)
            {
               length = SYS_VER_LEN_WITH_TIME;
            }

            /* Read Cmd */
            ret = CmdRead(pDev, (IFX_uint32_t *)pCmd,
                                (IFX_uint32_t *)pCmd, length);
            if (ret != IFX_SUCCESS)
            {
               TRACE (VMMC, DBG_LEVEL_HIGH,
                      ("VMMC Error reading firmware version!\n"));
               /* memset done above */
            }
            else
            {
               pVers->nEdspVers   = pCmd->MAJ;
               pVers->nEdspIntern = pCmd->MIN;
               pVers->nEDSPHotFix = pCmd->HF;
               pVers->nEDSPVariant = pCmd->VAR;
               pVers->nEDSPTimestamp = pCmd->LXDATE;
            }
            pVers->nChannel    = pDev->caps.nALI;
            pVers->nType       = pCmd->PLA; /* 0:INCA2, 1:Danube, 4: AR9 */
            pVers->nChip       = 0;
            pVers->nTapiVers   = 3;
            pVers->nDrvVers    = MAJORSTEP << 24 | MINORSTEP << 16 |
                                 VERSIONSTEP << 8 | VERS_TYPE;
#ifdef VMMC_FEAT_SLIC
            /* in case of SmartSLIC based systems, we can give some more
               versions.*/
            if (VMMC_ALM_SmartSLIC_IsConnected(pDev))
            {
               pVers->nDCCtrlVers         = pDev->sdd.nFirmwareVers;
               pVers->nDCCtrlStep         = pDev->sdd.nFirmwareStep;
               pVers->nDCCtrlHotFix       = pDev->sdd.nFirmwareHotFix;
               pVers->nASDSPVers          = pDev->sdd.nASDSPVers;
               pVers->nASDSPStep          = pDev->sdd.nASDSPStep;
               pVers->nSmartSLIC_HW_rev   = pDev->sdd.nSlicRevision;
               pVers->nSmartSLIC_DevId    = pDev->sdd.nSlicDevId;
            }
#endif /* VMMC_FEAT_SLIC */
         }
         VMMC_OS_CpyKern2Usr ((IFX_uint8_t*)ioarg,
                              pVers, sizeof(VMMC_IO_VERSION));
         VMMC_OS_Free (pVers);
         VMMC_OS_Free (pCmd);
         break;
      }

      case FIO_READ_CMD:
      {
         VMMC_IO_MB_CMD *pCmd = VMMC_OS_Malloc (sizeof(VMMC_IO_MB_CMD));
         if (pCmd == IFX_NULL)
         {
            ret = VMMC_statusNoMem;
            break;
         }
         VMMC_OS_CpyUsr2Kern ((char*)pCmd,
                              (IFX_uint8_t*)ioarg, sizeof (VMMC_IO_MB_CMD));
         ret = VMMC_Read_Cmd (pDev, pCmd);
         VMMC_OS_CpyKern2Usr ((IFX_uint8_t*)ioarg,
                              pCmd, sizeof (VMMC_IO_MB_CMD));
         VMMC_OS_Free (pCmd);
         break;
      }
      case FIO_WRITE_CMD:
      {
         VMMC_IO_MB_CMD *pCmd = VMMC_OS_Malloc (sizeof(VMMC_IO_MB_CMD));
         if (pCmd == IFX_NULL)
         {
            ret = VMMC_statusNoMem;
            break;
         }
         VMMC_OS_CpyUsr2Kern ((char*)pCmd,
                              (IFX_uint8_t*)ioarg, sizeof (VMMC_IO_MB_CMD));
         ret = VMMC_Write_Cmd (pDev, pCmd);
         VMMC_OS_Free (pCmd);
         break;
      }
      case FIO_RDREG:
      {
         VMMC_IO_REG_ACCESS *pCmd = VMMC_OS_Malloc (sizeof(VMMC_IO_REG_ACCESS));
         if (pCmd == IFX_NULL)
         {
            ret = VMMC_statusNoMem;
            break;
         }
         VMMC_OS_CpyUsr2Kern ((char*)pCmd,
                              (IFX_uint8_t*)ioarg, sizeof (VMMC_IO_REG_ACCESS));
         ret = VMMC_RegRd_Cmd (pDev, pCmd);
         VMMC_OS_CpyKern2Usr ((IFX_uint8_t*)ioarg,
                              pCmd, sizeof (VMMC_IO_REG_ACCESS));
         VMMC_OS_Free (pCmd);
         break;
      }
      case FIO_WRREG:
      {
         VMMC_IO_REG_ACCESS *pCmd = VMMC_OS_Malloc (sizeof(VMMC_IO_REG_ACCESS));
         if (pCmd == IFX_NULL)
         {
            ret = VMMC_statusNoMem;
            break;
         }
         VMMC_OS_CpyUsr2Kern ((char*)pCmd,
                              (IFX_uint8_t*)ioarg, sizeof (VMMC_IO_REG_ACCESS));
         ret = VMMC_RegWr_Cmd (pDev, pCmd);
         VMMC_OS_Free (pCmd);
         break;
      }
      case FIO_FW_DOWNLOAD:
      {
         VMMC_IO_INIT IoInit;
         mps_fw dwnld_struct;

         if (pDev->nDevState & DS_BASIC_INIT)
         {
            /* Chip access was already initialised. So this is a
               reconfiguration.
               Reset all driver internal states and do a chip access exit. */
            IFX_TAPI_DeviceReset (pDev->pTapiDev);
         }

         /* Init of chip access */
         ret = VMMC_ChipAccessInit(pDev);
         if (ret != IFX_SUCCESS)
         {
             break;
         }

#ifdef VMMC_FEAT_CLOCK_SCALING
         /* Indicate that FW is about to boot and needs full system clock. */
         ret = VMMC_PMC_FwBoot(pDev, IFX_TRUE);
         if (ret != IFX_SUCCESS)
         {
             break;
         }
#endif /* VMMC_FEAT_CLOCK_SCALING */

         /* Before FW start reset the flags indicating FW error conditions. */
         pDev->bCmdReadError = IFX_FALSE;
         pDev->bSSIcrash = IFX_FALSE;

         VMMC_OS_CpyUsr2Kern ((char*)&IoInit, (char*)ioarg, sizeof(IoInit));

         memset (&dwnld_struct, 0, sizeof (dwnld_struct));
         dwnld_struct.length = IoInit.pram_size;
         /* MPS driver will do the USR2KERN so just pass on the pointer. */
         dwnld_struct.data = (IFX_void_t *)IoInit.pPRAMfw;

         ret =
#ifdef LINUX
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36))
            ifx_mps_ioctl((IFX_void_t *)command, IFX_NULL,
                             FIO_MPS_DOWNLOAD, (IFX_uint32_t) &dwnld_struct);
#else
            ifx_mps_ioctl((IFX_void_t *)command,
                             FIO_MPS_DOWNLOAD, (IFX_uint32_t) &dwnld_struct);
#endif
#else /* LINUX */
            ifx_mps_ioctl((IFX_void_t *)command,
                             FIO_MPS_DOWNLOAD, (IFX_uint32_t) &dwnld_struct);
#endif /* LINUX */
#ifdef VMMC_FEAT_CLOCK_SCALING
         /* Indicate that FW boot is done. */
         (void)VMMC_PMC_FwBoot(pDev, IFX_FALSE);
#endif /* VMMC_FEAT_CLOCK_SCALING */
         break;
      }
      case FIO_DEV_RESET:
      {
         ret =
#ifdef LINUX
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36))
            ifx_mps_ioctl((IFX_void_t *)command, IFX_NULL, FIO_MPS_RESET, 0);
#else
            ifx_mps_ioctl((IFX_void_t *)command, FIO_MPS_RESET, 0);
#endif
#else /* LINUX */
            ifx_mps_ioctl((IFX_void_t *)command, FIO_MPS_RESET, 0);
#endif /* LINUX */
         if (ret == VMMC_statusOk)
         {
            pDev->bSSIcrash = IFX_FALSE;
         }
#ifdef VMMC_FEAT_CLOCK_SCALING
         /* Indicate that FW boot is done. */
         (void)VMMC_PMC_FwBoot(pDev, IFX_FALSE);
#endif /* VMMC_FEAT_CLOCK_SCALING */
         break;
      }
      case FIO_DEV_RESTART:
      {
         ret =
#ifdef LINUX
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36))
            ifx_mps_ioctl((IFX_void_t *)command, IFX_NULL, FIO_MPS_RESTART, 0);
#else
            ifx_mps_ioctl((IFX_void_t *)command, FIO_MPS_RESTART, 0);
#endif
#else /* LINUX */
            ifx_mps_ioctl((IFX_void_t *)command, FIO_MPS_RESTART, 0);
#endif /* LINUX */
         if (ret == VMMC_statusOk)
         {
            pDev->bSSIcrash = IFX_FALSE;
         }
         break;
      }
      case FIO_LASTERR:
      {
         VMMC_OS_CpyKern2Usr ((IFX_void_t*)ioarg, &pDev->err, sizeof(IFX_int32_t));
         pDev->err = VMMC_ERR_OK;
         break;
      }
      case FIO_REPORT_SET:
      {
         if ((ioarg > DBG_LEVEL_HIGH) || (ioarg < DBG_LEVEL_LOW))
         {
            SetTraceLevel(VMMC, DBG_LEVEL_OFF);
         }
         else
         {
            SetTraceLevel(VMMC, ioarg);
         }
         break;
      }
      ON_IOCTL((VMMC_CHANNEL *) pLLDummyCh,
               FIO_BBD_DOWNLOAD, VMMC_BBD_Download, bbd_format_t);

      default:
         /* errmsg: Invalid IOCTL call */
         ret = VMMC_statusInvalidIoctl;
         break;
   }

   RETURN_DEVSTATUS(ret);
}
