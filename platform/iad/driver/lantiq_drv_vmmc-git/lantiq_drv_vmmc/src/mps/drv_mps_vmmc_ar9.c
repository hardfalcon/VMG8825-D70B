/******************************************************************************

                              Copyright (c) 2014
                            Lantiq Deutschland GmbH
                             http://www.lantiq.com

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

****************************************************************************
   Module      : drv_mps_vmmc_ar9.c
   Description : This file contains the implementation of the AR9/VR9 specific
                 driver functions.
*******************************************************************************/

/* ============================= */
/* Includes                      */
/* ============================= */
#include "drv_config.h"

#ifdef VMMC_WITH_MPS
   #include "drv_api.h"
#endif

#ifdef LINUX
#include "drv_mps_vmmc_bsp.h"
#endif /* LINUX */

#if (defined(GENERIC_OS) && defined(GREENHILLS_CHECK))
   #include "drv_vmmc_ghs.h"
#endif /* (defined(GENERIC_OS) && defined(GREENHILLS_CHECK)) */

/* lib_ifxos headers */
#include "ifx_types.h"
#include "ifxos_linux_drv.h"
#include "ifxos_copy_user_space.h"
#include "ifxos_event.h"
#include "ifxos_lock.h"
#include "ifxos_select.h"
#include "ifxos_interrupt.h"

#ifdef LINUX
/* board specific headers */
#if (BSP_API_VERSION < 3)
   #include <asm/ifx_vpe.h>
   #ifdef VMMC_FEAT_SLIC
      #if defined(CONFIG_IFX_GPIO)
         #include <asm/ifx/ifx_gpio.h>
      #else
         #include <linux/gpio.h>
      #endif
   #endif /* VMMC_FEAT_SLIC */
#elif (BSP_API_VERSION < 5)
   #include <asm/ltq_vpe.h>
   #ifdef VMMC_FEAT_SLIC
      #include <ltq_gpio.h>
   #endif /* VMMC_FEAT_SLIC */
#else
   #include <asm/ltq_vpe.h>
   #ifdef VMMC_FEAT_SLIC
      #include <linux/gpio.h>
   #endif /* VMMC_FEAT_SLIC */
#endif

#ifdef PMU_SUPPORTED
   #if (BSP_API_VERSION <= 3)
      #include "ifx_pmu.h"
   #elif (BSP_API_VERSION <= 4)
      #include "ltq_pmu.h"
   #endif
#endif /* PMU_SUPPORTED */
#endif /* LINUX */

/* device specific headers */
#include "drv_mps_vmmc.h"
#include "drv_mps_vmmc_dbg.h"
#include "drv_mps_vmmc_device.h"

/* ============================= */
/* Local Macros & Definitions    */
/* ============================= */

/* xRX platform runtime flags */
#define VPE1_STARTED        (1 << 0)
#define SSLIC_GPIO_RESERVED (1 << 1)

/* Firmware watchdog timer counter address */
#define VPE1_WDOG_CTR_ADDR (ifx_mps_ram_base + 432)

/* Firmware watchdog timeout range, values in ms */
#define VPE1_WDOG_TMOUT_MIN 20
#define VPE1_WDOG_TMOUT_MAX 5000

#define IFX_MPS_UNUSED(var) ((IFX_void_t)(var))

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
IFX_int32_t ifx_mps_wdog_callback (IFX_uint32_t wdog_cleared_ok_count);
extern IFXOS_event_t fw_ready_evt;

/* ============================= */
/* Local function declaration    */
/* ============================= */
static IFX_int32_t ifx_mps_fw_wdog_start_ar9(void);

/* ============================= */
/* Local variable definition     */
/* ============================= */
static IFX_uint32_t lq_mps_xRX_flags = 0;

#ifdef VMMC_FEAT_SLIC
int lq_mps_slic_reset_gpio;  /* BSP API version >= 5 */
#endif /* VMMC_FEAT_SLIC */

/* VMMC watchdog timer callback */
static IFX_int32_t (*ifx_wdog_callback)(IFX_uint32_t flags) = IFX_NULL;

/* FW decrypt function in SOC ROM. */
static void (*ifx_mps_chip_decrypt_fn)(unsigned int addr, int n) = IFX_NULL;

/* ============================= */
/* Local function definition     */
/* ============================= */

/******************************************************************************
 * AR9 Specific Routines
 ******************************************************************************/

#ifdef VMMC_FEAT_SLIC
/**
 * Reserve GPIO lines used by SmartSLIC.
 * Called every time before VPE1 startup.
 *
 * \param    none
 * \return   0         IFX_SUCCESS
 * \return   -1        IFX_ERROR
 * \ingroup  Internal
 * \remarks  Reservation is necessary to protect GPIO lines used
 *           by SmartSLIC from being seized by other SW modules.
 */
static IFX_int32_t lq_mps_sslic_gpio_reserve(void)
{
   IFX_int32_t ret = IFX_SUCCESS;

#if (BSP_API_VERSION < 5) && defined(CONFIG_IFX_GPIO)
   if (!(lq_mps_xRX_flags & SSLIC_GPIO_RESERVED))
   {
      /* SmartSLIC reset */
      ret = ifx_gpio_pin_reserve(
               IFX_GPIO_PIN_ID(lq_mps_slic_reset_port, lq_mps_slic_reset_pin),
               IFX_GPIO_MODULE_TAPI_VMMC);
      if (ret)
         return ret;

      /* P1.15_ALTSEL0 = 0 */
      ret = ifx_gpio_altsel0_clear(
               IFX_GPIO_PIN_ID(lq_mps_slic_reset_port, lq_mps_slic_reset_pin),
               IFX_GPIO_MODULE_TAPI_VMMC);
      if (ret)
         return ret;

      /* P1.15_ALTSEL1 = 0 */
      ret = ifx_gpio_altsel1_clear(
               IFX_GPIO_PIN_ID(lq_mps_slic_reset_port, lq_mps_slic_reset_pin),
               IFX_GPIO_MODULE_TAPI_VMMC);
      if (ret)
         return ret;

      /* P1.15_DIR = 1 */
      ret = ifx_gpio_dir_out_set(
               IFX_GPIO_PIN_ID(lq_mps_slic_reset_port, lq_mps_slic_reset_pin),
               IFX_GPIO_MODULE_TAPI_VMMC);
      if (ret)
         return ret;

      /* P1.15_OD = 1 */
      ret = ifx_gpio_open_drain_set(
               IFX_GPIO_PIN_ID(lq_mps_slic_reset_port, lq_mps_slic_reset_pin),
               IFX_GPIO_MODULE_TAPI_VMMC);

      /* SmartSLIC clock (GPIO8, P0.8) */
      ret = ifx_gpio_pin_reserve(IFX_GPIO_PIN_ID (0, 8),
                                 IFX_GPIO_MODULE_TAPI_VMMC);
      if (ret)
         return ret;

      /* SmartSLIC interface SSI0 (GPIO34, P2.2) */
      ret = ifx_gpio_pin_reserve(IFX_GPIO_PIN_ID (2, 2),
                                 IFX_GPIO_MODULE_TAPI_VMMC);
      if (ret)
         return ret;

      /* SmartSLIC interface SSI0 (GPIO35, P2.3) */
      ret = ifx_gpio_pin_reserve(IFX_GPIO_PIN_ID (2, 3),
                                 IFX_GPIO_MODULE_TAPI_VMMC);
      if (ret)
         return ret;

      /* SmartSLIC interface SSI0 (GPIO36, P2.4) */
      ret = ifx_gpio_pin_reserve(IFX_GPIO_PIN_ID (2, 4),
                                 IFX_GPIO_MODULE_TAPI_VMMC);
      if (ret)
         return ret;

      lq_mps_xRX_flags |= SSLIC_GPIO_RESERVED;
   }
#else /* BSP_API_VERSION */
   if (!(lq_mps_xRX_flags & SSLIC_GPIO_RESERVED))
   {
      ret = gpio_request(lq_mps_slic_reset_gpio, "slic-reset");
      if (ret)
         return ret;

      gpio_direction_output(lq_mps_slic_reset_gpio, 1);
      lq_mps_xRX_flags |= SSLIC_GPIO_RESERVED;
   }
#endif /* BSP_API_VERSION */

   return ret;
}
#endif /* VMMC_FEAT_SLIC */

#ifdef VMMC_FEAT_SLIC
/**
 * Free GPIO lines used by SmartSLIC.
 * Called every time after VPE1 stopping.
 *
 * \param    none
 * \return   0         IFX_SUCCESS
 * \return   -1        IFX_ERROR
 * \ingroup  Internal
 */
static IFX_int32_t lq_mps_sslic_gpio_free(void)
{
   IFX_int32_t ret = IFX_SUCCESS;

#if (BSP_API_VERSION < 5) && defined(CONFIG_IFX_GPIO)
   if (lq_mps_xRX_flags & SSLIC_GPIO_RESERVED)
   {
      /* SmartSLIC reset */
      ret = ifx_gpio_pin_free(
               IFX_GPIO_PIN_ID(lq_mps_slic_reset_port, lq_mps_slic_reset_pin),
               IFX_GPIO_MODULE_TAPI_VMMC);
      if (ret)
         return ret;

      /* SmartSLIC clock (GPIO8, P0.8) */
      ret = ifx_gpio_pin_free(IFX_GPIO_PIN_ID (0, 8),
                              IFX_GPIO_MODULE_TAPI_VMMC);
      if (ret)
         return ret;

      /* SmartSLIC interface SSI0 (GPIO34, P2.2) */
      ret = ifx_gpio_pin_free(IFX_GPIO_PIN_ID (2, 2),
                              IFX_GPIO_MODULE_TAPI_VMMC);
      if (ret)
         return ret;

      /* SmartSLIC interface SSI0 (GPIO35, P2.3) */
      ret = ifx_gpio_pin_free(IFX_GPIO_PIN_ID (2, 3),
                              IFX_GPIO_MODULE_TAPI_VMMC);
      if (ret)
         return ret;

      /* SmartSLIC interface SSI0 (GPIO36, P2.4) */
      ret = ifx_gpio_pin_free(IFX_GPIO_PIN_ID (2, 4),
                              IFX_GPIO_MODULE_TAPI_VMMC);
      if (ret)
         return ret;

      lq_mps_xRX_flags &= ~SSLIC_GPIO_RESERVED;
   }

#else /* BSP_API_VERSION */
   if (lq_mps_xRX_flags & SSLIC_GPIO_RESERVED)
   {
      gpio_free(lq_mps_slic_reset_gpio);
      lq_mps_xRX_flags &= ~SSLIC_GPIO_RESERVED;
   }
#endif /* BSP_API_VERSION */

   return ret;
}
#endif /* VMMC_FEAT_SLIC */

/**
 * Start AR9 EDSP firmware watchdog mechanism.
 * Called after download and startup of VPE1.
 *
 * \param   none
 * \return  0         IFX_SUCCESS
 * \return  -1        IFX_ERROR
 * \ingroup Internal
 */
IFX_int32_t ifx_mps_fw_wdog_start_ar9()
{
   /* vpe1_wdog_ctr should be set up in u-boot as
      "vpe1_wdog_ctr_addr=0xBF2001B0"; protection from incorrect or missing
      setting */
   if (vpe1_wdog_ctr != VPE1_WDOG_CTR_ADDR)
   {
      vpe1_wdog_ctr = VPE1_WDOG_CTR_ADDR;
   }

   /* vpe1_wdog_timeout should be set up in u-boot as "vpe1_wdog_timeout =
      <value in ms>"; protection from insane setting */
   if (vpe1_wdog_timeout < VPE1_WDOG_TMOUT_MIN)
   {
      vpe1_wdog_timeout = VPE1_WDOG_TMOUT_MIN;
   }
   if (vpe1_wdog_timeout > VPE1_WDOG_TMOUT_MAX)
   {
      vpe1_wdog_timeout = VPE1_WDOG_TMOUT_MAX;
   }

   /* convert into jiffies */
   vpe1_wdog_timeout = vpe1_wdog_timeout * HZ / 1000;

   /* register BSP callback function */
   if (IFX_SUCCESS !=
       vpe1_sw_wdog_register_reset_handler (ifx_mps_wdog_callback))
   {
      TRACE (MPS, DBG_LEVEL_HIGH,
             (KERN_ERR "[%s %s %d]: Unable to register WDT callback.\r\n",
              __FILE__, __func__, __LINE__));
      return IFX_ERROR;;
   }

   /* start software watchdog timer */
   if (IFX_SUCCESS != vpe1_sw_wdog_start (0))
   {
      TRACE (MPS, DBG_LEVEL_HIGH,
             (KERN_ERR
              "[%s %s %d]: Error starting software watchdog timer.\r\n",
              __FILE__, __func__, __LINE__));
      return IFX_ERROR;
   }

   return IFX_SUCCESS;
}

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
   IFX_int32_t retval = 0;
   IFX_uint32_t mem, cksum;
   IFX_boolean_t bMemReqNotPresent = IFX_FALSE;

   IFX_MPS_UNUSED(pMBDev);
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

   if(FW_FORMAT_NEW)
   {
      IFX_uint32_t plt = pFW_img_data->fw_vers >> 8 & 0xf;

      /* platform check */
      if (((ifx_mps_chip_family == MPS_CHIP_XRX100) && (plt != FW_PLT_XRX100)) ||
          ((ifx_mps_chip_family == MPS_CHIP_XRX200) && (plt != FW_PLT_XRX200)) ||
          ((ifx_mps_chip_family == MPS_CHIP_XRX300) && (plt != FW_PLT_XRX300)))
      {
         TRACE (MPS, DBG_LEVEL_HIGH,
                ("Firmware does not match to this platform.\n"));
         return IFX_ERROR;
      }
   }

   mem = pFW_img_data->mem;

   /* memory requirement sanity check - crc check */
   if ((0xFF & ~((mem >> 16) + (mem >> 8) + mem)) != (mem >> 24))
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

   /* check if CPU1 base address is sane */
   if (cpu1_base_addr == IFX_NULL || !cpu1_base_addr)
   {
      TRACE (MPS, DBG_LEVEL_HIGH,
             (KERN_ERR "IFX_MPS: CPU1 base address is invalid!\r\n"));
      return IFX_ERROR;
   }
   else
   {
      /* check if CPU1 address is 1MB aligned */
      if ((IFX_uint32_t)cpu1_base_addr & 0xfffff)
      {
         TRACE (MPS, DBG_LEVEL_HIGH,
               (KERN_ERR "IFX_MPS: CPU1 base address is not 1MB aligned!\r\n"));
         return IFX_ERROR;
      }
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

      if (ifx_mps_chip_decrypt_fn == IFX_NULL)
      {
         TRACE (MPS, DBG_LEVEL_HIGH,
         ("MPS loader: Abort - platform identification failed!\n"));
         return IFX_ERROR;
      }

      /* Call the decrypt function in on-chip ROM. */
      ifx_mps_chip_decrypt_fn((IFX_uint32_t)cpu1_base_addr, pFWDwnld->length);
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
         return IFX_ERROR;
      }
   }
   else
   {
      /* just store self-calculated checksum */
      pFW_img_data->crc32 = cksum;
   }

   /* start VPE1 */
   ifx_mps_release ();
   /* start FW watchdog mechanism */
   ifx_mps_fw_wdog_start_ar9();
   /* get FW version */
   retval = ifx_mps_get_fw_version (0);

   return retval;
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
   /* start FW watchdog mechanism */
   ifx_mps_fw_wdog_start_ar9();
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
   if (lq_mps_xRX_flags & VPE1_STARTED)
   {
      /* stop software watchdog timer */
      vpe1_sw_wdog_stop (0);
      /* clean up the BSP callback function */
      vpe1_sw_wdog_register_reset_handler (IFX_NULL);
      /* stop VPE1 */
      vpe1_sw_stop (0);
      lq_mps_xRX_flags &= ~VPE1_STARTED;
#ifdef VMMC_FEAT_SLIC
      /* release SmartSLIC GPIO lines */
      if (lq_mps_sslic_gpio_free())
      {
         TRACE (MPS, DBG_LEVEL_HIGH,
                  ("IFX_MPS: error freeing SSLIC GPIO lines!\n"));
      }
#endif /* VMMC_FEAT_SLIC */
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
   if (lq_mps_xRX_flags & VPE1_STARTED)
   {
      /* stop software watchdog timer first */
      vpe1_sw_wdog_stop (0);
      vpe1_sw_stop (0);
      lq_mps_xRX_flags &= ~VPE1_STARTED;
#ifdef VMMC_FEAT_SLIC
      /* release SmartSLIC GPIO lines */
      if (lq_mps_sslic_gpio_free())
      {
         TRACE (MPS, DBG_LEVEL_HIGH,
                  ("IFX_MPS: error freeing SSLIC GPIO lines!\n"));
      }
#endif /* VMMC_FEAT_SLIC */
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

#ifdef VMMC_FEAT_SLIC
   /* reserve SmartSLIC GPIO pins */
   if (lq_mps_sslic_gpio_reserve())
   {
      TRACE (MPS, DBG_LEVEL_HIGH,
               ("IFX_MPS: cannot reserve SSLIC GPIO lines!\n"));
      return;
   }
#endif /* VMMC_FEAT_SLIC */

   /* Start VPE1 */
   if (IFX_SUCCESS !=
       vpe1_sw_start ((IFX_void_t *)cpu1_base_addr, 0, 0))
   {
      TRACE (MPS, DBG_LEVEL_HIGH, (KERN_ERR "Error starting VPE1\r\n"));
      return;
   }

   lq_mps_xRX_flags |= VPE1_STARTED;

   /* Sleep until FW is ready or a timeout after 3 seconds occured */
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

   return;
}

/**
 * WDT callback.
 * This function is called by BSP (module softdog_vpe) in case if software
 * watchdog timer expiration is detected by BSP.
 * This function needs to be registered at BSP as WDT callback using
 * vpe1_sw_wdog_register_reset_handler() API.
 *
 * \return  0        IFX_SUCCESS, cannot fail
 * \ingroup Internal
 */
IFX_int32_t ifx_mps_wdog_callback (IFX_uint32_t wdog_cleared_ok_count)
{
#ifdef DEBUG
   TRACE (MPS, DBG_LEVEL_HIGH,
          ("MPS: watchdog callback! arg=0x%08x\r\n", wdog_cleared_ok_count));
#endif /* DEBUG */

#ifdef VMMC_FEAT_SLIC
   /* activate SmartSLIC RESET */
   if (lq_mps_xRX_flags & SSLIC_GPIO_RESERVED)
   {
      IFXOS_INTSTAT flags;

      IFXOS_LOCKINT (flags);
#if (BSP_API_VERSION < 5) && defined(CONFIG_IFX_GPIO)
      /* P1.15_OUT = 0 */
      if (ifx_gpio_output_clear
          (IFX_GPIO_PIN_ID (1, 15), IFX_GPIO_MODULE_TAPI_VMMC))
      {
         TRACE (MPS, DBG_LEVEL_HIGH,
                (KERN_ERR "[%s %s %d]: Error resetting SLIC (GPIO error).\r\n",
                __FILE__, __func__, __LINE__));
      }
#else /* BSP_API_VERSION */
      gpio_set_value(lq_mps_slic_reset_gpio, 0);
#endif /* BSP_API_VERSION */
      IFXOS_UNLOCKINT (flags);
   }
#endif /* VMMC_FEAT_SLIC */

   /* recalculate and compare the firmware checksum */
   ifx_mps_fw_crc_compare(cpu1_base_addr, pFW_img_data);

   /* dump exception area on a console */
   ifx_mps_dump_fw_xcpt(cpu1_base_addr, pFW_img_data);

   if (IFX_NULL != ifx_wdog_callback)
   {
      /* call VMMC driver */
      ifx_wdog_callback (wdog_cleared_ok_count);
   }
   else
   {
      TRACE (MPS, DBG_LEVEL_HIGH,
             (KERN_WARNING "MPS: VMMC watchdog timer callback is NULL.\r\n"));
   }
   return 0;
}

/**
 * Register WDT callback.
 * This function is called by VMMC driver to register its callback in
 * the MPS driver.
 *
 * \return  0        IFX_SUCCESS, cannot fail
 * \ingroup Internal
 */
IFX_int32_t
ifx_mps_register_wdog_callback (IFX_int32_t (*pfn) (IFX_uint32_t flags))
{
   ifx_wdog_callback = pfn;
   return 0;
}

#ifndef VMMC_WITH_MPS
EXPORT_SYMBOL (ifx_mps_register_wdog_callback);
#endif /* !VMMC_WITH_MPS */

/**
   Enable or disable the Voice-HW on XRX100 (and compatible) devices.

   Switch on/off power and clock for DFEV and TDM HW blocks. The HW blocks
   on the SOC are switched off during the system boot-up in the very early
   stage. Every individual module has to clock on / power on it's own block
   before accessing the register space. As a last thing when removing the
   driver the HW blocks need to be disabled again.

   \param  bEnable  Enable or disable HW - switch on/off power and clock.
                    IFX_ENABLE - enable the PMU
                    IFX_DISABLE - disable the PMU

*/
IFX_void_t ifx_mps_HwSetupXRX100(IFX_enDis_t bEnable)
{
#ifdef PMU_SUPPORTED

   #if (BSP_API_VERSION <= 4)

      if (ifx_mps_chip_family == MPS_CHIP_XRX200)
      {
         if (bEnable != IFX_DISABLE)
         {
            /* Turn on the power domain for TDM. */
            if (ifx_pmu_pg_slic_tdm_enable() != 0)
            {
               TRACE(MPS, DBG_LEVEL_HIGH,
                     ("VMMC failed to power on SLIC TDM power domain\n"));
            }
         }
      }

      /* Turn on or off the DFEV and TDM clock domains. */
      DFEV0_PMU_SETUP(bEnable ? IFX_PMU_ENABLE : IFX_PMU_DISABLE);
      DFEV1_PMU_SETUP(bEnable ? IFX_PMU_ENABLE : IFX_PMU_DISABLE);
      TDM_PMU_SETUP(bEnable ? IFX_PMU_ENABLE : IFX_PMU_DISABLE);

      if (ifx_mps_chip_family == MPS_CHIP_XRX200)
      {
         if (bEnable == IFX_DISABLE)
         {
            /* Turn off the power domain for TDM. */
            if (ifx_pmu_pg_slic_tdm_disable() != 0)
            {
               TRACE(MPS, DBG_LEVEL_HIGH,
                     ("VMMC failed to power off SLIC TDM power domain\n"));
            }
         }
      }

   #else /* (BSP_API_VERSION <= 4) */

      IFX_MPS_UNUSED(bEnable);

   #endif /* (BSP_API_VERSION <= 4) */

#else  /* PMU_SUPPORTED */
   IFX_MPS_UNUSED(bEnable);
#endif /* PMU_SUPPORTED */
}

/**
   Probe (test) the Voice-HW on XRX100 (and compatible) devices.

   Access the MPS CHIP ID register and try to identify the chip family.
   The result is stored for later use in other functions. Additionally the
   entrypoint of the FW decrypt function in the SOC ROM is stored. It depends
   in the silicon revision.
   The GPIO for SLIC reset is given by system definition and depends on the
   platform. Currently it is hardcoded in the voice-FW and cannot be changed.
   So also here in the driver it is hardcoded depending on the platform.
   GPIO for SLIC Reset AR9, VR9: (GPIO31, P1.15) / 231
   GPIO for SLIC Reset AR10: (GPIO 0, P0.0) / 194
   As the linear number depends on the GPIO framework implementation the
   hardcode should better be configurable at least with a base address.

   \return
   0 on success or errorcode
*/
IFX_int32_t ifx_mps_HwProbeXRX100(void)
{
   u32 part_number;
   u32 chip_version;

   part_number = IFX_MPS_CHIPID_PARTNUM_GET(*IFX_MPS_CHIPID);

   /* Please note that the order of the tests below matters. */
   /* xRX1xx */
   if (((part_number & 0xFFF0) == 0x0160) ||
       ((part_number & 0xFFF0) == 0x0170))
   {
      ifx_mps_chip_family = MPS_CHIP_XRX100;
      ifx_mps_chip_decrypt_fn = (void (*)(unsigned int, int))0xbf0017c4;
      return 0;
   }

   /* xRX2xx */
   if ((part_number & 0xFFF0) == 0x01C0)
   {
      ifx_mps_chip_family = MPS_CHIP_XRX200;
      ifx_mps_chip_decrypt_fn = (void (*)(unsigned int, int))0xbf001ea4;
      return 0;
   }

   if ((part_number == 0x000B) ||
       (part_number == 0x000C) ||
       (part_number == 0x000D) ||
       (part_number == 0x000E))
   {
      ifx_mps_chip_family = MPS_CHIP_XRX200;
      ifx_mps_chip_decrypt_fn = (void (*)(unsigned int, int))0xbf001f38;
      return 0;
   }

   /* xRX3xx */
   if (part_number <= 0x0013)
   {
      if (*IFX_MPS_GRX330_CHIPID)
      {
         ifx_mps_chip_family = MPS_CHIP_XRX300;
         ifx_mps_chip_decrypt_fn = (void (*)(unsigned int, int))0xbfc00030;
         return 0;
      }

      chip_version = IFX_MPS_CHIPID_VERSION_GET(*IFX_MPS_CHIPID);
      if ((chip_version == 0) || (chip_version == 1))
      {
         ifx_mps_chip_family = MPS_CHIP_XRX300;
         ifx_mps_chip_decrypt_fn = (void (*)(unsigned int, int))0xbfc01918;
         return 0;
      }
      else
      {
         ifx_mps_chip_family = MPS_CHIP_XRX300;
         ifx_mps_chip_decrypt_fn = (void (*)(unsigned int, int))0xbfc012c0;
         return 0;
      }
   }

   /* xRX5xx*/
   /* Reserved values:
      0x0014 (G562), 0x0015 (G582), 0x0016 (G583), 0x0017 (G580) */

   /* unknown silicon */
   TRACE (MPS, DBG_LEVEL_HIGH,
          ("MPS: Failed to identify chip\n"));
   return -1;
}

#if defined(SYSTEM_XRX300)
#define PCM_REG_BASE    0x1f100000
#define PCM_REG_SIZE    0x1000
#define PCM_EN          0x18
#define PCM_CFG14       0x60
#define PCM_CFG15       0x64
#define PCM_DATA14      0xb8
#define PCM_DATA15      0xbc

static inline void pcm_reg_writel(u32 offset, u32 val)
{
    void *pcm_reg_base = ioremap_nocache(PCM_REG_BASE, PCM_REG_SIZE);
    __raw_writel(val, pcm_reg_base + offset);
}

static inline u32 pcm_reg_readl(u32 offset)
{
    const void *pcm_reg_base = ioremap_nocache(PCM_REG_BASE, PCM_REG_SIZE);
    return __raw_readl(pcm_reg_base + offset);
}

static inline void pcm_reg_setbits32(u32 offset, u32 set)
{
    void *pcm_reg_base = ioremap_nocache(PCM_REG_BASE, PCM_REG_SIZE);
    u32 val = __raw_readl(pcm_reg_base + offset);
    val |= set;
    __raw_writel(val, pcm_reg_base + offset);
}

void ifx_mps_pcm_if_war(void)
{
    int i;
    const u32 pcm_en = BIT(15) | BIT(14);

    TRACE(MPS, DBG_LEVEL_HIGH, ("PCM xRX330 TDM pin workaround\n"));

    for (i = 0; i < 500; i++) {
        /* Enable channels 14 and 15 */
        pcm_reg_setbits32(PCM_EN, pcm_en);
        asm("sync");

        if (pcm_reg_readl(PCM_EN) == pcm_en)
            break;

        TRACE(MPS, DBG_LEVEL_HIGH, ("PCM xRX330 PCM_EN still not correctly set! Retrying ...\n"));
    }

    /* Assign timeslot 2 to channel 14 and timeslot 3 to channel 15 */
    pcm_reg_writel(PCM_CFG14, 2 << 16 | 2);
    pcm_reg_writel(PCM_CFG15, 3 << 16 | 3);
    /* Permanently write 0xFF to timeslot data */
    pcm_reg_writel(PCM_DATA14, 0xff);
    pcm_reg_writel(PCM_DATA15, 0xff);
    asm("sync");
}
#endif
