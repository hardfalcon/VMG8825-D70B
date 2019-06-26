/*
 * Copyright (C) 2007-2010 Lantiq Deutschland GmbH
 * Copyright (C) 2011-2013 Daniel Schwierzeck, daniel.schwierzeck@gmail.com
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 * Common board configuration for Lantiq XWAY Danube family
 *
 * Use following defines in your board config to enable specific features
 * and drivers for this SoC:
 *
 * CONFIG_LTQ_SUPPORT_UART
 * - support the Danube ASC/UART interface and console
 *
 * CONFIG_LTQ_SUPPORT_NOR_FLASH
 * - support a parallel NOR flash via the CFI interface in flash bank 0
 *
 * CONFIG_LTQ_SUPPORT_ETHERNET
 * - support the Danube ETOP and MAC interface
 *
 * CONFIG_LTQ_SUPPORT_SPI_FLASH
 * - support the Danube SPI interface and serial flash drivers
 * - specific SPI flash drivers must be configured separately
 */

#ifndef __DANUBE_CONFIG_H__
#define __DANUBE_CONFIG_H__

/* CPU and SoC type */
#define CONFIG_SOC_LANTIQ
#define CONFIG_SOC_XWAY_DANUBE

/* Cache configuration */
#define CONFIG_SYS_MIPS_CACHE_MODE	CONF_CM_CACHABLE_NONCOHERENT
#define CONFIG_SYS_MIPS_CACHE_BASE	0x9fc00000
#define CONFIG_SYS_MIPS_CACHE_INIT_RAM_LOAD

#define CONFIG_SYS_MALLOC_LEN		1 * 1024 * 1024

/*
 * Supported clock modes
 * PLL0 clock output is 333 MHz
 * PLL1 clock output is 262.144 MHz
 */
#define LTQ_CLK_CPU_333_DDR_167		0	/* Base PLL0, OCP 2 */
#define LTQ_CLK_CPU_111_DDR_111		1	/* Base PLL0, OCP 1 */

/* CPU speed */
#define CONFIG_SYS_CLOCK_MODE		LTQ_CLK_CPU_333_DDR_167
#define CONFIG_SYS_HZ			1000

/* RAM */
#define CONFIG_SYS_SDRAM_BASE		0x80000000
#define CONFIG_SYS_SDRAM_BASE_UC	0xa0000000
#define CONFIG_SYS_MEMTEST_START	0x81000000
#define CONFIG_SYS_MEMTEST_END		0x82000000
#define CONFIG_SYS_LOAD_ADDR		0x81000000
#define CONFIG_SYS_LOAD_SIZE		(2 * 1024 * 1024)
#define CONFIG_SYS_INIT_SP_OFFSET	0x4000
#define CONFIG_RAM_TEXT_BASE		0xA0100000

/* SRAM */
#define CONFIG_SYS_SRAM_BASE		0xBE1A0000
#define CONFIG_SYS_SRAM_SIZE		0x10000

/* ASC/UART driver and console */
#define CONFIG_LANTIQ_SERIAL
#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }

/* GPIO */
#define CONFIG_LANTIQ_GPIO
#define CONFIG_LTQ_GPIO_MAX_BANKS	2

/* FLASH driver */
#if defined(CONFIG_LTQ_SUPPORT_NOR_FLASH)
#define CONFIG_SYS_MAX_FLASH_BANKS	1
#define CONFIG_SYS_MAX_FLASH_SECT	256
#define CONFIG_SYS_FLASH_BASE		0xB0000000
#define CONFIG_FLASH_16BIT
#define CONFIG_SYS_FLASH_CFI
#define CONFIG_FLASH_CFI_DRIVER
#define CONFIG_SYS_FLASH_CFI_WIDTH	FLASH_CFI_16BIT
#define CONFIG_SYS_FLASH_USE_BUFFER_WRITE
#define CONFIG_FLASH_SHOW_PROGRESS	50
#define CONFIG_SYS_FLASH_PROTECTION
#define CONFIG_CFI_FLASH_USE_WEAK_ADDR_SWAP

#define CONFIG_CMD_FLASH
#else
#define CONFIG_SYS_NO_FLASH
#endif /* CONFIG_NOR_FLASH */

#if defined(CONFIG_LTQ_SUPPORT_SPI_FLASH)
#define CONFIG_LANTIQ_SPI
#define CONFIG_SPI_FLASH

#define CONFIG_CMD_SF
#define CONFIG_CMD_SPI
#endif

#if defined(CONFIG_LTQ_SUPPORT_NAND_FLASH)
#define CONFIG_NAND_LANTIQ
#define CONFIG_SYS_NAND_ONFI_DETECTION
#define CONFIG_SYS_NAND_SELF_INIT
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#define CONFIG_SYS_NAND_BASE		0xB4000000

#define CONFIG_CMD_NAND
#endif

#if defined(CONFIG_LTQ_SUPPORT_ETHERNET)
#define CONFIG_LANTIQ_DMA
#define CONFIG_LANTIQ_DANUBE_ETOP

#define CONFIG_PHYLIB
#define CONFIG_MII

#define CONFIG_CMD_MII
#define CONFIG_CMD_NET
#endif

#define CONFIG_SPL_MAX_SIZE		(32 * 1024)
#define CONFIG_SPL_BSS_SIZE		(4 * 1024)
#define CONFIG_SPL_STACK_SIZE		(4 * 1024)
#define CONFIG_SPL_STACK_BASE		(CONFIG_SYS_SRAM_BASE + \
					CONFIG_SPL_MAX_SIZE + \
					CONFIG_SPL_STACK_SIZE - 1)
#define CONFIG_SPL_BSS_BASE		(CONFIG_SPL_STACK_BASE + 1)

#define CONFIG_SPL_MC_TUNE_BASE		(CONFIG_SYS_SDRAM_BASE_UC + \
					CONFIG_SYS_INIT_SP_OFFSET)

#if defined(CONFIG_SYS_BOOT_RAM)
#define CONFIG_SYS_TEXT_BASE		0xA0100000
#define CONFIG_SKIP_LOWLEVEL_INIT
#define CONFIG_SYS_DISABLE_CACHE
#endif

#if defined(CONFIG_SYS_BOOT_NOR)
#define CONFIG_SYS_TEXT_BASE		0xB0000000
#endif

#if defined(CONFIG_SYS_BOOT_NORSPL)
#define CONFIG_SYS_TEXT_BASE		0x80100000
#if defined(CONFIG_TPL_BUILD)
#define CONFIG_SPL_TEXT_BASE		0xBE1A0000
#else
#define CONFIG_SPL_TEXT_BASE		0xB0000000
#endif
#define CONFIG_TPL_TEXT_BASE		0xBE1A0000
#endif

#if defined(CONFIG_SYS_BOOT_NOR) || defined(CONFIG_SYS_BOOT_NORSPL)
#define CONFIG_SYS_XWAY_EBU_BOOTCFG	0x688C688C
#define CONFIG_XWAY_SWAP_BYTES
#endif

#define	CONFIG_SYS_MONITOR_BASE		CONFIG_SYS_TEXT_BASE

#endif /* __DANUBE_CONFIG_H__ */
