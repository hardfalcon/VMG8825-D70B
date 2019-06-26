/******************************************************************************

                              Copyright (c) 2014
                            Lantiq Deutschland GmbH
                             http://www.lantiq.com

  For licensing information, see the file 'LICENSE' in the root folder of
  this software module.

*******************************************************************************/

/**
   \file drv_vmmc_xrx100.c
   This file contains implementations specific to the xRX platforms.
   Current supported are xRX100, xRX200, xRX300.
*/

#include "drv_vmmc_api.h"
#include "drv_mps_vmmc.h"

#ifdef LINUX
   #if (BSP_API_VERSION < 3)
      #if defined(CONFIG_IFX_GPIO)
         #include <asm/ifx/ifx_gpio.h>
      #else
         #include <linux/gpio.h>
      #endif
   #elif (BSP_API_VERSION < 5)
      #include <ltq_gpio.h>
   #endif /* BSP_API_VERSION */
#endif /* LINUX */

#define VMMC_TAPI_GPIO_MODULE_ID       IFX_GPIO_MODULE_TAPI_VMMC


/**
   Reserve and configure the GPIOs of the PCM interface.

   \param  mode         PCM interface operation: Master / Slave.
   \param  GPIOreserved Indicates if the GPIOs still have to be reserved.

   \return
   0 if successful or errorcode otherwise.
*/
IFX_int32_t VMMC_XRX100_PcmGpioReserve(
                        IFX_uint32_t mode,
                        IFX_boolean_t GPIOreserved)
{
   IFX_int32_t ret = 0;

#if (BSP_API_VERSION < 5)
   if ((ifx_mps_chip_family == MPS_CHIP_XRX100) ||
       (ifx_mps_chip_family == MPS_CHIP_XRX200))
   {
#ifdef CONFIG_IFX_GPIO
      /* Reserve P0.0 as TDM/FSC */
      if (!GPIOreserved)
         ret |= ifx_gpio_pin_reserve(IFX_GPIO_PIN_ID(0, 0), VMMC_TAPI_GPIO_MODULE_ID);
      ret |= ifx_gpio_altsel0_set(IFX_GPIO_PIN_ID(0, 0), VMMC_TAPI_GPIO_MODULE_ID);
      ret |= ifx_gpio_altsel1_set(IFX_GPIO_PIN_ID(0, 0), VMMC_TAPI_GPIO_MODULE_ID);
      ret |= ifx_gpio_open_drain_set(IFX_GPIO_PIN_ID(0, 0), VMMC_TAPI_GPIO_MODULE_ID);

      /* Reserve P1.9 as TDM/DO */
      if (!GPIOreserved)
         ret |= ifx_gpio_pin_reserve(IFX_GPIO_PIN_ID(1, 9), VMMC_TAPI_GPIO_MODULE_ID);
      ret |= ifx_gpio_altsel0_set(IFX_GPIO_PIN_ID(1, 9), VMMC_TAPI_GPIO_MODULE_ID);
      ret |= ifx_gpio_altsel1_clear(IFX_GPIO_PIN_ID(1, 9), VMMC_TAPI_GPIO_MODULE_ID);
      ret |= ifx_gpio_dir_out_set(IFX_GPIO_PIN_ID(1, 9), VMMC_TAPI_GPIO_MODULE_ID);
      ret |= ifx_gpio_open_drain_set(IFX_GPIO_PIN_ID(1, 9), VMMC_TAPI_GPIO_MODULE_ID);

      /* Reserve P2.9 as TDM/DI */
      if (!GPIOreserved)
         ret |= ifx_gpio_pin_reserve(IFX_GPIO_PIN_ID(2, 9), VMMC_TAPI_GPIO_MODULE_ID);
      ret |= ifx_gpio_altsel0_clear(IFX_GPIO_PIN_ID(2, 9), VMMC_TAPI_GPIO_MODULE_ID);
      ret |= ifx_gpio_altsel1_set(IFX_GPIO_PIN_ID(2, 9), VMMC_TAPI_GPIO_MODULE_ID);
      ret |= ifx_gpio_dir_in_set(IFX_GPIO_PIN_ID(2, 9), VMMC_TAPI_GPIO_MODULE_ID);

      /* Reserve P2.8 as TDM/DCL */
      if (!GPIOreserved)
         ret |= ifx_gpio_pin_reserve(IFX_GPIO_PIN_ID(2, 8), VMMC_TAPI_GPIO_MODULE_ID);
      ret |= ifx_gpio_altsel0_clear(IFX_GPIO_PIN_ID(2, 8), VMMC_TAPI_GPIO_MODULE_ID);
      ret |= ifx_gpio_altsel1_set(IFX_GPIO_PIN_ID(2, 8), VMMC_TAPI_GPIO_MODULE_ID);
      ret |= ifx_gpio_open_drain_set(IFX_GPIO_PIN_ID(2, 8), VMMC_TAPI_GPIO_MODULE_ID);

      if (mode == 2) {
         /* TDM/FSC+DCL Master */
         ret |= ifx_gpio_dir_out_set(IFX_GPIO_PIN_ID(0, 0), VMMC_TAPI_GPIO_MODULE_ID);
         ret |= ifx_gpio_dir_out_set(IFX_GPIO_PIN_ID(2, 8), VMMC_TAPI_GPIO_MODULE_ID);
      } else {
         /* TDM/FSC+DCL Slave */
         ret |= ifx_gpio_dir_in_set(IFX_GPIO_PIN_ID(0, 0), VMMC_TAPI_GPIO_MODULE_ID);
         ret |= ifx_gpio_dir_in_set(IFX_GPIO_PIN_ID(2, 8), VMMC_TAPI_GPIO_MODULE_ID);
      }
#else
      if (!GPIOreserved) {
         ret = gpio_request(0, "tdm-fsc");
         if (ret)
            return IFX_ERROR;

         ret = gpio_request(25, "tdm-do");
         if (ret) {
            gpio_free(0);
            return IFX_ERROR;
         }

         ret = gpio_request(26, "tdm-di");
         if (ret) {
            gpio_free(25);
            gpio_free(0);
            return IFX_ERROR;
         }

         ret = gpio_request(40, "tdm-dcl");
         if (ret) {
            gpio_free(26);
            gpio_free(25);
            gpio_free(0);
            return IFX_ERROR;
         }
      }

      gpio_direction_output(25, 1);
      gpio_set_altfunc(25, 1, 0, 1);
      gpio_direction_input(26);
      gpio_set_altfunc(26, 0, 1, 0);

      if (mode == 2) {
         gpio_direction_output(0, 0);
         gpio_set_altfunc(0, 1, 1, 1);
         gpio_direction_output(40, 0);
         gpio_set_altfunc(40, 0, 1, 1);
      } else {
         gpio_direction_input(0);
         gpio_set_altfunc(0, 1, 1, 0);
         gpio_set_pull(0, LTQ_GPIO_PULL_UP);
         gpio_direction_input(40);
         gpio_set_altfunc(40, 0, 1, 0);
         gpio_set_pull(40, LTQ_GPIO_PULL_UP);
      }
#endif
   } /* XRX100, XRX200 */

#ifdef CONFIG_IFX_GPIO
   if (ifx_mps_chip_family == MPS_CHIP_XRX300)
   {
      /* Reserve P1.9 as TDM/DO */
      if (!GPIOreserved)
         ret |= ifx_gpio_pin_reserve(IFX_GPIO_PIN_ID(1, 9), VMMC_TAPI_GPIO_MODULE_ID);
      ret |= ifx_gpio_altsel0_set(IFX_GPIO_PIN_ID(1, 9), VMMC_TAPI_GPIO_MODULE_ID);
      ret |= ifx_gpio_altsel1_clear(IFX_GPIO_PIN_ID(1, 9), VMMC_TAPI_GPIO_MODULE_ID);
      ret |= ifx_gpio_dir_out_set(IFX_GPIO_PIN_ID(1, 9), VMMC_TAPI_GPIO_MODULE_ID);
      ret |= ifx_gpio_open_drain_set(IFX_GPIO_PIN_ID(1, 9), VMMC_TAPI_GPIO_MODULE_ID);

      /* Reserve P1.10 as TDM/DI */
      if (!GPIOreserved)
         ret |= ifx_gpio_pin_reserve(IFX_GPIO_PIN_ID(1, 10), VMMC_TAPI_GPIO_MODULE_ID);
      ret |= ifx_gpio_altsel0_clear(IFX_GPIO_PIN_ID(1, 10), VMMC_TAPI_GPIO_MODULE_ID);
      ret |= ifx_gpio_altsel1_set(IFX_GPIO_PIN_ID(1, 10), VMMC_TAPI_GPIO_MODULE_ID);
      ret |= ifx_gpio_dir_in_set(IFX_GPIO_PIN_ID(1, 10), VMMC_TAPI_GPIO_MODULE_ID);

      /* Reserve P1.11 as TDM/DCL */
      if (!GPIOreserved)
         ret |= ifx_gpio_pin_reserve(IFX_GPIO_PIN_ID(1, 11), VMMC_TAPI_GPIO_MODULE_ID);
      ret |= ifx_gpio_altsel0_set(IFX_GPIO_PIN_ID(1, 11), VMMC_TAPI_GPIO_MODULE_ID);
      ret |= ifx_gpio_altsel1_clear(IFX_GPIO_PIN_ID(1, 11), VMMC_TAPI_GPIO_MODULE_ID);
      ret |= ifx_gpio_open_drain_set(IFX_GPIO_PIN_ID(1, 11), VMMC_TAPI_GPIO_MODULE_ID);

      /* Reserve P3.10 as TDM/FSC */
      if (!GPIOreserved)
         ret |= ifx_gpio_pin_reserve(IFX_GPIO_PIN_ID(3, 10), VMMC_TAPI_GPIO_MODULE_ID);
      ret |= ifx_gpio_altsel0_clear(IFX_GPIO_PIN_ID(3, 10), VMMC_TAPI_GPIO_MODULE_ID);
      ret |= ifx_gpio_altsel1_set(IFX_GPIO_PIN_ID(3, 10), VMMC_TAPI_GPIO_MODULE_ID);
      ret |= ifx_gpio_open_drain_set(IFX_GPIO_PIN_ID(3, 10), VMMC_TAPI_GPIO_MODULE_ID);

      if (mode == 2) {
         /* TDM/FSC+DCL Master */
         ret |= ifx_gpio_dir_out_set(IFX_GPIO_PIN_ID(1, 11), VMMC_TAPI_GPIO_MODULE_ID);
         ret |= ifx_gpio_dir_out_set(IFX_GPIO_PIN_ID(3, 10), VMMC_TAPI_GPIO_MODULE_ID);
      } else {
         /* TDM/FSC+DCL Slave */
         ret |= ifx_gpio_dir_in_set(IFX_GPIO_PIN_ID(1, 11), VMMC_TAPI_GPIO_MODULE_ID);
         ret |= ifx_gpio_dir_in_set(IFX_GPIO_PIN_ID(3, 10), VMMC_TAPI_GPIO_MODULE_ID);
      }
   }
#endif
#endif /* BSP_API_VERSION */

   return ret;
}


/**
   Release the GPIOs of the PCM interface.

   \return
   0 if successful or errorcode otherwise.
*/
IFX_int32_t VMMC_XRX100_PcmGpioRelease(void)
{
   IFX_int32_t ret = 0;

#if (BSP_API_VERSION < 5)
   if ((ifx_mps_chip_family == MPS_CHIP_XRX100) ||
       (ifx_mps_chip_family == MPS_CHIP_XRX200))
   {
#ifdef CONFIG_IFX_GPIO
      ret |= ifx_gpio_pin_free(IFX_GPIO_PIN_ID(0, 0), VMMC_TAPI_GPIO_MODULE_ID);
      ret |= ifx_gpio_pin_free(IFX_GPIO_PIN_ID(1, 9), VMMC_TAPI_GPIO_MODULE_ID);
      ret |= ifx_gpio_pin_free(IFX_GPIO_PIN_ID(2, 9), VMMC_TAPI_GPIO_MODULE_ID);
      ret |= ifx_gpio_pin_free(IFX_GPIO_PIN_ID(2, 8), VMMC_TAPI_GPIO_MODULE_ID);
#else
      gpio_free(40);
      gpio_free(26);
      gpio_free(25);
      gpio_free(0);
#endif
   }
#ifdef CONFIG_IFX_GPIO
   if (ifx_mps_chip_family == MPS_CHIP_XRX300)
   {
      ret |= ifx_gpio_pin_free(IFX_GPIO_PIN_ID(1,  9), VMMC_TAPI_GPIO_MODULE_ID);
      ret |= ifx_gpio_pin_free(IFX_GPIO_PIN_ID(1, 10), VMMC_TAPI_GPIO_MODULE_ID);
      ret |= ifx_gpio_pin_free(IFX_GPIO_PIN_ID(1, 11), VMMC_TAPI_GPIO_MODULE_ID);
      ret |= ifx_gpio_pin_free(IFX_GPIO_PIN_ID(3, 10), VMMC_TAPI_GPIO_MODULE_ID);
   }
#endif
#endif /* BSP_API_VERSION */

   return ret;
}
