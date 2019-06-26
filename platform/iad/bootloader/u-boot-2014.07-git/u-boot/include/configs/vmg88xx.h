/*
 * Copyright (C) 2015 Daniel Schwierzeck, daniel.schwierzeck@gmail.com
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define CONFIG_IDENT_STRING	" vmg8847"
#define CONFIG_BOARD_NAME	"Zyxel vmg8847 series [510, 511, 512, 513, 514, 515]"

/* Configure SoC */
#define CONFIG_LTQ_SUPPORT_UART		/* Enable ASC and UART */

#define CONFIG_LTQ_SUPPORT_ETHERNET	/* Enable ethernet */
#define CONFIG_FW_XRX300_PHY11G_A21

#define CONFIG_LTQ_SSIO_SHIFT_REGS	3
#define CONFIG_LTQ_SSIO_EDGE_FALLING

#define CONFIG_LTQ_SSIO_GPHY1_MODE	0x2
#define CONFIG_LTQ_SSIO_GPHY2_MODE	0x2
#define CONFIG_LTQ_SSIO_GPHY3_MODE	0x2
#define CONFIG_LTQ_SSIO_GPHY4_MODE	0x2
#define CONFIG_LTQ_SSIO_INIT_VALUE	0xc00	/* red pwr LED, PCIE0 reset */

#define CONFIG_LTQ_SUPPORT_HSNAND_FLASH
#define CONFIG_LTQ_NAND_CS1
#define CONFIG_LTQ_NAND_ECC_REED_SOLOMON
#define CONFIG_LTQ_NAND_BITFLIP_DETECT

#define CONFIG_LTQ_SUPPORT_SPL_NAND_FLASH	/* Build NAND flash SPL */
#define CONFIG_SYS_NAND_PAGE_SIZE	0x800
#define CONFIG_SYS_NAND_BLOCK_SIZE	0x20000
#define CONFIG_LTQ_SPL_SWAP
#define CONFIG_LTQ_SPL_REDUND_IMAGES	0x1

#define CONFIG_LTQ_SPL_COMP_LZO
#define CONFIG_LTQ_SPL_CONSOLE
#define CONFIG_LTQ_SPL_MC_TUNE
#define CONFIG_SYS_DRAM_PROBE

/* MTD devices */
#define CONFIG_MTD_DEVICE
#define CONFIG_MTD_PARTITIONS
#define CONFIG_CMD_MTD
#define CONFIG_CMD_MTDPARTS
#define CONFIG_MTD_UBI_BEB_LIMIT	30
#define MTDIDS_DEFAULT			"nand0=nand-xway"
#define MTDPARTS_DEFAULT		\
	"mtdparts=nand-xway:1024k(uboot_fix),-(ubi)"

#if defined(CONFIG_SYS_BOOT_NANDHWSPL)
#define CONFIG_SPL_MC_TUNE_OFFS		0x1a000
#define CONFIG_SPL_U_BOOT_OFFS		0x20000
#define CONFIG_SPL_U_BOOT_SIZE		0x60000

#define CONFIG_ENV_IS_IN_UBI
#define CONFIG_ENV_OVERWRITE
#define CONFIG_ENV_UBI_PART		"ubi"
#define CONFIG_ENV_UBI_VOLUME		"uboot_env"
#define CONFIG_ENV_UBI_VID_HDR_OFF	"2048"
#define CONFIG_SPHAIRON_UBI_SUPPORT
#define CONFIG_SPHAIRON_FLASHLAYOUT	"ubisq"
#else
#define CONFIG_ENV_IS_NOWHERE
#define CONFIG_SPHAIRON_ETL_NAND
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
