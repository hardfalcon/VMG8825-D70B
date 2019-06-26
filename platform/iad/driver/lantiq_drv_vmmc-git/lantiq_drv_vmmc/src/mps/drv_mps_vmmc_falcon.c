/******************************************************************************

                              Copyright (c) 2014
                            Lantiq Deutschland GmbH
                             http://www.lantiq.com

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

****************************************************************************
   Module      : drv_mps_vmmc_falcon.c
   Description : This file contains the implementation of the FALC-ON specific
                 driver functions.
*******************************************************************************/

/* ============================= */
/* Includes                      */
/* ============================= */
#include "drv_config.h"

#if defined(SYSTEM_FALCON) /* defined in drv_config.h */

/* lib_ifxos headers */
#include "ifx_types.h"
#include "ifxos_linux_drv.h"
#include "ifxos_copy_user_space.h"
#include "ifxos_event.h"
#include "ifxos_lock.h"
#include "ifxos_select.h"
#include "ifxos_interrupt.h"
#include <linux/gpio.h>
#if   (LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0))
#include <sys1_reg.h>
#include <gpon_reg_base.h>
#else
#define GPON_SYS1_BASE     (KSEG1 | 0x1EF00000)
#define GPON_SYS_BASE      (KSEG1 | 0x1DF00000)
#define ACTS_MPS 0x02000000
#define ACTS_GPTC 0x04000000
#endif
#include <falcon_irq.h>
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,5,0))
#include <vpe.h>
#else
#include <asm/vpe.h>
#include <softdog_vpe.h>
#endif
#include <sysctrl.h>

void (*ifx_bsp_basic_mps_decrypt)(unsigned int addr, int n) = (void (*)(unsigned int, int))0xbf000290;


/*#define USE_PLAIN_VOICE_FIRMWARE*/
/* board specific headers */

/* device specific headers */
#include "drv_mps_vmmc.h"
#include "drv_mps_vmmc_dbg.h"
#include "drv_mps_vmmc_device.h"

/* ============================= */
/* Local Macros & Definitions    */
/* ============================= */
#define OPTIC_PMA_BASE (KSEG1 | 0x1DD00000)
   /** reset and power down control for PMD
       Not Specified */
#define GPON_PMD_RESETCONTROL (OPTIC_PMA_BASE + 0x00000280)

#ifndef PMA_PMD_RESETCONTROL_DLL_PD
/* Fields of "reset and power down control for PMD" */
/** dll pmd power down
    0 .. no pd, 1 .. pd */
#define PMA_PMD_RESETCONTROL_DLL_PD 0x00000800
#endif
#ifndef PMA_PMD_RESETCONTROL_DLL_RSTN
/** dll pmd reset
    0 .. reset, 1 .. no reset */
#define PMA_PMD_RESETCONTROL_DLL_RSTN 0x00000400
#endif

/* ============================= */
/* Global variable definition    */
/* ============================= */
extern mps_comm_dev *pMPSDev;

/* ============================= */
/* Global function declaration   */
/* ============================= */
IFX_void_t ifx_mps_release (void);
extern IFX_uint32_t ifx_mps_reset_structures (mps_comm_dev * pMPSDev);
extern IFX_int32_t ifx_mps_bufman_close (void);
extern IFXOS_event_t fw_ready_evt;
/* ============================= */
/* Local function declaration    */
/* ============================= */

/* ============================= */
/* Local variable definition     */
/* ============================= */
static IFX_int32_t vpe1_started = 0;

/* ============================= */
/* Local function definition     */
/* ============================= */

/******************************************************************************
 * AR9 Specific Routines
 ******************************************************************************/

/**
 * Firmware download to Voice CPU
 * This function performs a firmware download to the coprocessor.
 *
 * \param   pMBDev    Pointer to mailbox device structure
 * \param   pFWDwnld  Pointer to firmware structure
 * \return  0         IFX_SUCCESS, firmware ready
 * \return  -1        IFX_ERROR,   firmware not downloaded.
 * \ingroup Internal
 */
IFX_int32_t ifx_mps_download_firmware (mps_mbx_dev *pMBDev, mps_fw *pFWDwnld)
{
   IFX_uint32_t mem, cksum;
   IFX_uint8_t crc;
   IFX_boolean_t bMemReqNotPresent = IFX_FALSE;

   /* VCC register */
   /* dummy accesss on GTC for GPONC-55, otherwise upper bits are random on read */
   ltq_r32 ((u32 *)((KSEG1 | 0x1DC000B0)));
   /* NTR Frequency Select 1536 kHz per default or take existing,
      NTR Output Enable and NTR8K Output Enable  */
   if ((ltq_r32 ((u32 *)(GPON_SYS_BASE + 0xBC)) & 7) == 0)
      ltq_w32_mask (0x10187, 0x183, (u32 *)(GPON_SYS_BASE + 0xBC));
   else
      ltq_w32_mask (0x10180, 0x180, (u32 *)(GPON_SYS_BASE + 0xBC));
#if 0
   /* BIU-ICU1-IM1_ISR - IM1:FSCT_CMP1=1 and FSC_ROOT=1
      (0x1f880328 = 0x00002800) */
   ltq_w32 (0x00002800, (u32 *)(GPON_ICU1_BASE + 0x30));
#endif
   /* copy FW footer from user space */
   if (IFX_NULL == IFXOS_CpyFromUser(pFW_img_data,
                           pFWDwnld->data+pFWDwnld->length/4-sizeof(*pFW_img_data)/4,
                           sizeof(*pFW_img_data)))
   {
      TRACE (MPS, DBG_LEVEL_HIGH,
                  (KERN_ERR "[%s %s %d]: copy_from_user error\r\n",
                   __FILE__, __func__, __LINE__));
      return IFX_ERROR;
   }

   mem = pFW_img_data->mem;

   /* memory requirement sanity check */
   if ((crc = ~((mem >> 16) + (mem >> 8) + mem)) != (mem >> 24))
   {
      TRACE (MPS, DBG_LEVEL_HIGH,
          ("[%s %s %d]: warning, image does not contain size - assuming 1MB!\n",
           __FILE__, __func__, __LINE__));
      mem = 1 * 1024 * 1024;
      bMemReqNotPresent = IFX_TRUE;
   }
   else
   {
      mem &= 0x00FFFFFF;
   }

   /* check if FW image fits in available memory space */
   if (mem > vpe1_get_max_mem(0))
   {
      TRACE (MPS, DBG_LEVEL_HIGH,
      ("[%s %s %d]: error, firmware memory exceeds reserved space (%i > %i)!\n",
                 __FILE__, __func__, __LINE__, mem, vpe1_get_max_mem(0)));
      return IFX_ERROR;
   }

   /* reset the driver */
   ifx_mps_reset ();

   /* call BSP to get cpu1 base address */
   cpu1_base_addr = (IFX_uint32_t *)vpe1_get_load_addr(0);

   /* check if CPU1 base address is sane
      \todo: check if address is 1MB aligned,
      also make it visible in a /proc fs */
   if (!cpu1_base_addr)
   {
      TRACE (MPS, DBG_LEVEL_HIGH,
             (KERN_ERR "IFX_MPS: CPU1 base address is invalid!\r\n"));
      return IFX_ERROR;
   }
   /* further use uncached value */
   cpu1_base_addr = (IFX_uint32_t *)KSEG1ADDR(cpu1_base_addr);

   /* free all data buffers that might be currently used by FW */
   if (IFX_NULL != ifx_mps_bufman_freeall)
   {
      ifx_mps_bufman_freeall();
   }

   if(FW_FORMAT_NEW)
   {
      /* adjust download length */
      pFWDwnld->length -= (sizeof(*pFW_img_data)-sizeof(IFX_uint32_t));
   }
   else
   {
      pFWDwnld->length -= sizeof(IFX_uint32_t);

      /* handle unlikely case if FW image does not contain memory requirement -
         assumed for old format only */
      if (IFX_TRUE == bMemReqNotPresent)
         pFWDwnld->length += sizeof(IFX_uint32_t);

      /* in case of old FW format always assume that FW is encrypted;
         use compile switch USE_PLAIN_VOICE_FIRMWARE for plain FW */
#ifndef USE_PLAIN_VOICE_FIRMWARE
      pFW_img_data->enc = FW_ENC_ENCRYPT;
#else
#warning Using unencrypted firmware!
      pFW_img_data->enc = 0;
#endif /* USE_PLAIN_VOICE_FIRMWARE */
      /* initializations for the old format */
      pFW_img_data->st_addr_crc = 2*sizeof(IFX_uint32_t) +
                                  FW_AR9_OLD_FMT_XCPT_AREA_SZ;
      pFW_img_data->en_addr_crc = pFWDwnld->length;
      pFW_img_data->fw_vers = 0;
      pFW_img_data->magic = 0;
   }

   if (ifx_mps_init_mbox_setup(
         pMPSDev, (pFW_img_data->enc & FW_ENC_MBOX_LAYOUT_2) ?
            LTQ_VOICE_MPS_MBOX_TYPE_L2 : LTQ_VOICE_MPS_MBOX_TYPE_L1) != IFX_SUCCESS)
   {
      TRACE (MPS, DBG_LEVEL_HIGH,
            (KERN_ERR "IFX_MPS: FW Download - MBox Setup(L%d) failed!\r\n",
            (pFW_img_data->enc & FW_ENC_MBOX_LAYOUT_2) ? LTQ_VOICE_MPS_MBOX_TYPE_L2 : LTQ_VOICE_MPS_MBOX_TYPE_L1));
      return IFX_ERROR;
   }
   else
   {
      TRACE (MPS, DBG_LEVEL_HIGH,
          ("IFX_MPS: Info - use FW MBox Layout %d\n", pMPSDev->mbx_descr));
      /* default setup fastbuffer config - depends on the MBox layout / FW */
      (void)ifx_mps_fastbuf_cfg_setup(0, 0, 0, 0, 0, 0);
   }

   /* copy FW image to base address of CPU1 */
   if (IFX_NULL ==
       IFXOS_CpyFromUser ((IFX_void_t *)cpu1_base_addr,
                          (IFX_void_t *)pFWDwnld->data, pFWDwnld->length))
   {
      TRACE (MPS, DBG_LEVEL_HIGH,
             (KERN_ERR "[%s %s %d]: copy_from_user error\r\n", __FILE__,
              __func__, __LINE__));
      return IFX_ERROR;
   }

   /* process firmware decryption */
   if ((pFW_img_data->enc & FW_ENC_ENCRYPT) == FW_ENC_ENCRYPT)
   {
      if(FW_FORMAT_NEW)
      {
         /* adjust decryption length (avoid decrypting CRC32 checksum) */
         pFWDwnld->length -= sizeof(IFX_uint32_t);
      }
      /* BootROM actually decrypts n+4 bytes if n bytes were passed for
         decryption. Subtract sizeof(u32) from length to avoid decryption
         of data beyond the FW image code */
      pFWDwnld->length -= sizeof(IFX_uint32_t);
      ifx_bsp_basic_mps_decrypt((unsigned int)cpu1_base_addr, pFWDwnld->length);
   }

   /* calculate CRC32 checksum over downloaded image */
   cksum = ifx_mps_fw_crc32(cpu1_base_addr, pFW_img_data);

   /* verify the checksum */
   if(FW_FORMAT_NEW)
   {
      if (cksum != pFW_img_data->crc32)
      {
         TRACE (MPS, DBG_LEVEL_HIGH,
                ("MPS: FW checksum error: img=0x%08x calc=0x%08x\r\n",
                pFW_img_data->crc32, cksum));
         /*return IFX_ERROR;*/
      }
   }
   else
   {
      /* just store self-calculated checksum */
      pFW_img_data->crc32 = cksum;
   }

   /* start VPE1 */
   ifx_mps_release ();

   /* get FW version */
   return ifx_mps_get_fw_version (0);
}


/**
 * Restart CPU1
 * This function restarts CPU1 by accessing the reset request register and
 * reinitializes the mailbox.
 *
 * \return  0        IFX_SUCCESS, successful restart
 * \return  -1       IFX_ERROR, if reset failed
 * \ingroup Internal
 */
IFX_int32_t ifx_mps_restart (void)
{
   /* raise reset request for CPU1 and reset driver structures */
   ifx_mps_reset ();
   /* let CPU1 run */
   ifx_mps_release ();
   TRACE (MPS, DBG_LEVEL_HIGH, ("IFX_MPS: Restarting firmware..."));
   return ifx_mps_get_fw_version (0);
}

/**
 * Shutdown MPS - stop VPE1
 * This function stops VPE1
 *
 * \ingroup Internal
 */
IFX_void_t ifx_mps_shutdown (void)
{
   if (vpe1_started)
   {
      /* stop software watchdog timer */
      vpe1_sw_wdog_stop (0);
      /* clean up the BSP callback function */
      vpe1_sw_wdog_register_reset_handler (IFX_NULL);
      /* stop VPE1 */
      vpe1_sw_stop (0);
      vpe1_started = 0;
   }
}

/**
 * Reset CPU1
 * This function causes a reset of CPU1 by clearing the CPU0 boot ready bit
 * in the reset request register RCU_RST_REQ.
 * It does not change the boot configuration registers for CPU0 or CPU1.
 *
 * \return  0        IFX_SUCCESS, cannot fail
 * \ingroup Internal
 */
IFX_void_t ifx_mps_reset (void)
{
   /* if VPE1 is already started, stop it */
   if (vpe1_started)
   {
      /* stop software watchdog timer first */
      vpe1_sw_wdog_stop (0);
      vpe1_sw_stop (0);
      vpe1_started = 0;
   }

   /* reset driver */
   ifx_mps_reset_structures (pMPSDev);
   ifx_mps_bufman_close ();
   return;
}

/**
 * Let CPU1 run
 * This function starts VPE1
 *
 * \return  none
 * \ingroup Internal
 */
IFX_void_t ifx_mps_release (void)
{
   IFX_int_t ret;
   IFX_int32_t RetCode = 0;

   /* Start VPE1 */
   if (IFX_SUCCESS !=
       vpe1_sw_start ((IFX_void_t *)cpu1_base_addr, 0, 0))
   {
      TRACE (MPS, DBG_LEVEL_HIGH, (KERN_ERR "Error starting VPE1\r\n"));
      return;
   }
   vpe1_started = 1;

   /* sleep 3 seconds until FW is ready */
   ret = IFXOS_EventWait (&fw_ready_evt, 3000, &RetCode);
   if ((ret == IFX_ERROR) && (RetCode == 1))
   {
      /* timeout */
      TRACE (MPS, DBG_LEVEL_HIGH,
             (KERN_ERR "[%s %s %d]: Timeout waiting for firmware ready.\r\n",
              __FILE__, __func__, __LINE__));
      /* recalculate and compare the firmware checksum */
      ifx_mps_fw_crc_compare(cpu1_base_addr, pFW_img_data);
      /* dump exception area on a console */
      ifx_mps_dump_fw_xcpt(cpu1_base_addr, pFW_img_data);
   }
}

/**
   Hardware setup on FALC ON
*/
void sys_hw_setup (void)
{
#if   (LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0))
   /* Set INFRAC register bit 1: clock enable of the GPE primary clock.  */
   sys_gpe_hw_activate (0);
   /* enable 1.5 V */
   ltq_w32_mask (0xf, 0x0b, (u32 *)(GPON_SYS1_BASE | 0xbc));
   /* SYS1-CLKEN:GPTC = 1 and MPS, no longer FSCT = 1 */
   sys1_hw_activate (ACTS_MPS | ACTS_GPTC);
#else
   ltq_sysctl_ldo1v5_cfg_set(1, SYSCTRL_LDO1V5_1V50);
   ltq_sysctl_sys1_activate (SYSCTRL_SYS1_MPS | SYSCTRL_SYS1_GPTC);
#endif
   /* GPTC:CLC:RMC = 1 */
   ltq_w32 (0x00000100, (u32 *)(KSEG1 | 0x1E100E00));
   /* enable voice pll: clear powerdown and deactivate reset by settin RSTN */
   ltq_w32_mask (PMA_PMD_RESETCONTROL_DLL_RSTN | PMA_PMD_RESETCONTROL_DLL_PD,
   PMA_PMD_RESETCONTROL_DLL_RSTN, (u32 *)(GPON_PMD_RESETCONTROL));
}

#endif /* SYSTEM_FALCON */
