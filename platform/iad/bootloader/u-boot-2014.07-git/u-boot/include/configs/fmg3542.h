/*
 * Copyright (C) 2018-2019 Sphairon GmbH (a ZyXEL company)
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define CONFIG_IDENT_STRING		" fmg3542"
#define CONFIG_BOARD_NAME		"Zyxel fmg3542 series [520]"

/* Configure SoC */
#define CONFIG_LTQ_SUPPORT_UART		/* Enable ASC and UART */

#define CONFIG_LTQ_SUPPORT_ETHERNET	/* Enable ethernet */
#define CONFIG_FW_XRX300_PHY11G_A21
#define CONFIG_PHY_REALTEK		/* for RTL8211FS ethernet PHY */

/* FIXME: support boot cfg 0x6 also with HSNAND driver */
#if defined(CONFIG_SYS_BOOT_NANDSPL)
#define CONFIG_LTQ_SUPPORT_NAND_FLASH
#define CONFIG_SYS_NAND_USE_FLASH_BBT
#else
#define CONFIG_LTQ_SUPPORT_HSNAND_FLASH
#define CONFIG_LTQ_NAND_FORCE_NO_ECC
#endif
#define CONFIG_LTQ_NAND_CS1

#define CONFIG_LTQ_SUPPORT_SPL_NAND_FLASH	/* Build NAND flash SPL in conjunction with CONFIG_LTQ_SUPPORT_NAND_FLASH */
#define CONFIG_SYS_NAND_PAGE_COUNT	64
#define CONFIG_SYS_NAND_PAGE_SIZE	0x800
#define CONFIG_SYS_NAND_OOBSIZE		64
#define CONFIG_SYS_NAND_BLOCK_SIZE	0x20000
#define CONFIG_SYS_NAND_5_ADDR_CYCLE
#define CONFIG_SYS_NAND_ECC_NONE

#define CONFIG_LTQ_SPL_COMP_LZO
#define CONFIG_LTQ_SPL_CONSOLE

/* MTD devices */
#define CONFIG_MTD_DEVICE
#define CONFIG_MTD_PARTITIONS
#define CONFIG_CMD_MTD
#define CONFIG_CMD_MTDPARTS
#define CONFIG_MTD_UBI_BEB_LIMIT	30
#define MTDIDS_DEFAULT			"nand0=nand-xway"

#if defined(CONFIG_SYS_BOOT_NANDSPL)
#define CONFIG_SPL_TPL_OFFS		0x800
#define CONFIG_SPL_TPL_SIZE		0x5000
#define CONFIG_SPL_MC_TUNE_OFFS		0x1a000
#define CONFIG_SPL_U_BOOT_OFFS		0x20000
#define CONFIG_SPL_U_BOOT_SIZE		0x60000
#define CONFIG_LTQ_SPL_REDUND_IMAGES	0x1


#define CONFIG_LTQ_SPL_MC_TUNE
#define CONFIG_SYS_DRAM_PROBE

#define CONFIG_ENV_IS_IN_UBI
#define CONFIG_ENV_OVERWRITE
#define CONFIG_ENV_UBI_PART		"ubi"
#define CONFIG_ENV_UBI_VOLUME		"uboot_env"
#define CONFIG_ENV_UBI_VID_HDR_OFF	"2048"

#define CONFIG_SPHAIRON_UBI_SUPPORT
#define CONFIG_SPHAIRON_ZYCLIP
#define CONFIG_SPHAIRON_FLASHLAYOUT	"ubisqd"
#define CONFIG_SPHAIRON_MSTC_UBISQD

#define MTDPARTS_DEFAULT		\
	"mtdparts=nand-xway:1024k(uboot_fix),61312k(ubi),256k(reserved),256k(calibration),56064k(data),10880k(syscfg),256k(romd),256k(mrd_cert1),256k(mrd_cert2)"
#elif defined(CONFIG_SYS_BOOT_UBISPL)
#define CONFIG_SPL_U_BOOT_OFFS		0x0
#define CONFIG_SPL_U_BOOT_SIZE		0x0

#define CONFIG_SPL_UBI_MAX_VOL_LEBS	400
#define CONFIG_SPL_UBI_MAX_PEB_SIZE	(CONFIG_SYS_NAND_BLOCK_SIZE)
#define CONFIG_SPL_UBI_VOL_IDS		8
#define CONFIG_SPL_UBI_INFO_ADDR	(CONFIG_SPL_TEXT_BASE + \
					CONFIG_SPL_MAX_SIZE + \
					CONFIG_SPL_BSS_SIZE)
#define CONFIG_SPL_UBI_PEB_OFFSET	491	/* start PEB of firmware partition: 0x03d60000 / 0x20000 */
#define CONFIG_SPL_UBI_MAX_PEBS		438	/* number of PEBs in firmware partition: (0x07420000 - 0x03d60000) / 0x20000 */
#define CONFIG_SPL_UBI_LOAD_MONITOR_ID	6	/* U-Boot in 7th volume with ID 6, only compatible with ubisq */

#define CONFIG_ENV_IS_IN_UBI
#define CONFIG_ENV_OVERWRITE
#define CONFIG_ENV_UBI_PART		"ubi"
#define CONFIG_ENV_UBI_VOLUME		"uboot_env"
#define CONFIG_ENV_UBI_VID_HDR_OFF	"2048"

#define CONFIG_SPHAIRON_UBI_SUPPORT
#define CONFIG_SPHAIRON_ETL_NAND
#define CONFIG_SPHAIRON_ZYCLIP
#define CONFIG_SPHAIRON_FLASHLAYOUT	"ubisq"
#define CONFIG_SPHAIRON_MSTC_UBISQ
#define CONFIG_ZYCLIP_IMAGE

#define MTDPARTS_DEFAULT		\
	"mtdparts=nand-xway:896k(uboot_fix),61440k(data),256k(reserved),256k(calibration),66944k(ubi),256k(romd),256k(mrd_cert1),256k(mrd_cert2)"
#else
#define CONFIG_ENV_IS_NOWHERE
#define CONFIG_SPHAIRON_ETL_NAND
#define CONFIG_SPHAIRON_ZYCLIP

#define MTDPARTS_DEFAULT		\
	"mtdparts=nand-xway:896k(uboot_fix),61440k(data),256k(reserved),256k(calibration),66944k(ubi),256k(romd),256k(mrd_cert1),256k(mrd_cert2)"
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
