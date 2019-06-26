/*
 * Copyright (C) 2011-2015 Daniel Schwierzeck, daniel.schwierzeck@gmail.com
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define CONFIG_IDENT_STRING	" vmg5304"
#define CONFIG_BOARD_NAME	"Zyxel vmg5304 series [440, 441]"

/* Configure SoC */
#define CONFIG_LTQ_SUPPORT_UART		/* Enable ASC and UART */

#define CONFIG_LTQ_SUPPORT_ETHERNET	/* Enable ethernet */
#define CONFIG_FW_VRX200_PHY11G_A2X

#define CONFIG_LTQ_SSIO_SHIFT_REGS	1
#define CONFIG_LTQ_SSIO_EDGE_FALLING
#define CONFIG_LTQ_SSIO_GPHY1_MODE	0x3
#define CONFIG_LTQ_SSIO_GPHY2_MODE	0x3
#define CONFIG_LTQ_SSIO_INIT_VALUE	0x0

#define CONFIG_LTQ_SUPPORT_SPI_FLASH
#define CONFIG_SPI_FLASH_SPANSION	/* Supports Spansion serial flash */

#define CONFIG_LTQ_SUPPORT_NAND_FLASH
#define CONFIG_SYS_NAND_USE_FLASH_BBT

#define CONFIG_LTQ_SUPPORT_SPL_SPI_FLASH	/* Build SPI flash SPL */
#define CONFIG_SPL_SPI_BUS		0
#define CONFIG_SPL_SPI_CS		4
#define CONFIG_SPL_SPI_MAX_HZ		25000000
#define CONFIG_SPL_SPI_MODE		0

#define CONFIG_LTQ_SUPPORT_SPL_NAND_FLASH	/* Build NAND flash SPL */
#define CONFIG_SYS_NAND_PAGE_COUNT	64
#define CONFIG_SYS_NAND_PAGE_SIZE	0x800
#define CONFIG_SYS_NAND_OOBSIZE		64
#define CONFIG_SYS_NAND_BLOCK_SIZE	0x20000
#define CONFIG_SYS_NAND_5_ADDR_CYCLE
#define CONFIG_SYS_NAND_ECC_NONE

#define CONFIG_LTQ_SPL_COMP_LZO
#define CONFIG_LTQ_SPL_CONSOLE
#define CONFIG_LTQ_SPL_MC_TUNE

/* MTD devices */
#define CONFIG_MTD_DEVICE
#define CONFIG_MTD_PARTITIONS
#define CONFIG_SPI_FLASH_MTD
#define CONFIG_CMD_MTD
#define CONFIG_CMD_MTDPARTS

/* Environment */
#define CONFIG_ENV_SPI_BUS		CONFIG_SPL_SPI_BUS
#define CONFIG_ENV_SPI_CS		CONFIG_SPL_SPI_CS
#define CONFIG_ENV_SPI_MAX_HZ		CONFIG_SPL_SPI_MAX_HZ
#define CONFIG_ENV_SPI_MODE		CONFIG_SPL_SPI_MODE

#if defined(CONFIG_SYS_BOOT_SFSPL)
/*
 *  0x6800 @ 0x00000: SPL
 *   0x800 @ 0x06800: MC tune data
 * 0x77000 @ 0x07000: U-Boot
 *  0x1000 @ 0x7e000: Calibration data
 *  0x7000 @ 0x7f000: ETL data
 */
#define CONFIG_SPL_U_BOOT_OFFS		0x7000
#define CONFIG_SPL_U_BOOT_SIZE		0x44000
#define CONFIG_SPL_MC_TUNE_OFFS		0x6800

#define CONFIG_ENV_IS_IN_SPI_FLASH
#define CONFIG_ENV_OVERWRITE
#define CONFIG_ENV_OFFSET		(512 * 1024)
#define CONFIG_ENV_SECT_SIZE		(256 * 1024)

#define MTDIDS_DEFAULT			"nor0=spi0.4"
#define MTDPARTS_DEFAULT		\
	"mtdparts=spi0.4:512k(uboot_fix),256k(uboot_cfg)"

#define CONFIG_SPHAIRON_FLASHLAYOUT	"psmd"
#elif defined(CONFIG_SYS_BOOT_NANDSPL)
#define CONFIG_SPL_TPL_OFFS		0x800
#define CONFIG_SPL_TPL_SIZE		0x5000
#define CONFIG_SPL_MC_TUNE_OFFS		0x1a000
#define CONFIG_SPL_U_BOOT_OFFS		0x20000
#define CONFIG_SPL_U_BOOT_SIZE		0x60000
#define CONFIG_LTQ_SPL_REDUND_IMAGES	0x1

#define CONFIG_ENV_IS_IN_UBI
#define CONFIG_ENV_OVERWRITE
#define CONFIG_ENV_UBI_PART		"ubi"
#define CONFIG_ENV_UBI_VOLUME		"uboot_env"
#define CONFIG_ENV_UBI_VID_HDR_OFF	"2048"

#define MTDIDS_DEFAULT			"nand0=nand-xway"
#define MTDPARTS_DEFAULT		\
	"mtdparts=nand-xway:1024k(uboot_fix),-(ubi)"

#define CONFIG_SPHAIRON_UBI_SUPPORT
#define CONFIG_SPHAIRON_FLASHLAYOUT	"ubisq"
#else
#define CONFIG_ENV_IS_NOWHERE

#define MTDIDS_DEFAULT			"nor0=spi0.4"
#define MTDPARTS_DEFAULT		"mtdparts="
#endif

#define CONFIG_ENV_SIZE			(8 * 1024)

/* Console */
#define CONFIG_LTQ_ADVANCED_CONSOLE
#define CONFIG_BAUDRATE			115200
#define CONFIG_CONSOLE_ASC		1

/* Pull in default board configs for Lantiq XWAY VRX200 */
#include <asm/lantiq/config.h>
#include <asm/arch/config.h>

/* Pull in Sphairon env */
#include <configs/sphairon_env.h>

#define CONFIG_EXTRA_ENV_SETTINGS	\
	CONFIG_ENV_LANTIQ_DEFAULTS	\
	CONFIG_ENV_SPHAIRON_GENERIC

#endif /* __CONFIG_H */
