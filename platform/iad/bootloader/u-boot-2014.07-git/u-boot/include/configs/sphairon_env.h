/*
 * Copyright (C) 2011-2018 Sphairon GmbH (a ZyXEL company)
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _SPHAIRON_ENV_H_
#define _SPHAIRON_ENV_H_

/* Enable library for Sphairon extensions */
#define CONFIG_LIB_SPHAIRON
#define CONFIG_LZMA
#define CONFIG_LZO

#if defined(CONFIG_LANTIQ_GPIO)
#define CONFIG_CMD_GPIO
#endif

#define CONFIG_CMD_PING
#define CONFIG_CMD_MISC
#define CONFIG_CMD_ECHO
#define CONFIG_CMD_TFTPPUT

#define CONFIG_MTD_DEVICE
#define CONFIG_CMD_MTD
#define CONFIG_CMD_MTDPARTS

#if defined(CONFIG_LTQ_SUPPORT_SPI_FLASH)
#define CONFIG_CMD_SPI
#endif

#if defined(CONFIG_SYS_BOOT_NANDSPL) || defined(CONFIG_SYS_BOOT_NANDHWSPL)
#define CONFIG_SPHAIRON_ETL_NAND
#endif

#if defined(CONFIG_SPHAIRON_ZYCLIP)
#define CONFIG_SPHAIRON_ETL_WRITE_DISABLE
#endif

#if defined(CONFIG_SPHAIRON_UBI_SUPPORT)
#define CONFIG_CMD_UBI
#define CONFIG_RBTREE
#define CONFIG_SPHAIRON_NO_UBOOT_UPDATE
#endif

/* Boot */
#define CONFIG_MIPS_BOOT_CMDLINE_LEGACY
#define CONFIG_MIPS_BOOT_FDT
#define CONFIG_FIT
#define CONFIG_OF_LIBFDT
#define CONFIG_MISC_INIT_R

/* Image booting */
#define CONFIG_SPHAIRON_SCAN_STEP_SIZE		0x2000
#define CONFIG_BOOTDELAY			3

#define CONFIG_TFTP_BLOCKSIZE			1400
#define CONFIG_NET_RETRY_COUNT			2

/* Environment */
#define CONFIG_IPADDR				192.168.100.1
#define CONFIG_SERVERIP				192.168.100.100
#define CONFIG_ETHADDR				00:1c:28:ff:00:10
#define CONFIG_LOADADDR				CONFIG_SYS_LOAD_ADDR

#define CONFIG_ENV_CONSOLEDEV					\
	"consoledev=ttyLTQ0\0"

#define CONFIG_ENV_ADDCONSOLE					\
	"addconsole=setenv bootargs $bootargs"			\
	" console=$consoledev,$baudrate\0"

#define CONFIG_ENV_NETDEV					\
	"netdev=eth0\0"

#define CONFIG_ENV_ADDIP					\
	"addip=setenv bootargs $bootargs"			\
	" ip=$ipaddr:$serverip::::$netdev:off\0"

#define CONFIG_ENV_ADDETH					\
	"addeth=setenv bootargs $bootargs"			\
	" ethaddr=$ethaddr\0"

#define CONFIG_ENV_MTDPARTS			\
	"mtdids="MTDIDS_DEFAULT"\0"		\
	"mtdparts="MTDPARTS_DEFAULT"\0"

#define CONFIG_ENV_SPHAIRON_GENERIC		\
	CONFIG_ENV_CONSOLEDEV			\
	CONFIG_ENV_ADDCONSOLE			\
	CONFIG_ENV_NETDEV			\
	CONFIG_ENV_ADDIP			\
	CONFIG_ENV_ADDETH			\
	CONFIG_ENV_MTDPARTS

#endif /* _SPHAIRON_ENV_H_ */
