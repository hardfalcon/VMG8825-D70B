##############################################################################
# File:      settings.mk                                                     #
# Purpose:   Linux kernel settings                                           #
#                                                                            #
# Copyright: Copyright (C) 2011, Sphairon Technologies GmbH                  #
#                                                                            #
# Author:    Daniel Schwierzeck                                              #
# Created:   20.09.2011                                                      #
##############################################################################

$(call SAS_AssertNotNull,SAS_BUILD)
$(call SAS_AssertNotNull,SAS_KBUILD_DIR)
$(call SAS_AssertNotNull,SAS_DEVICETREE_DIR)
$(call SAS_AssertNotNull,SAS_SOURCE_LINK)
$(call SAS_AssertNotNull,SAS_KERNEL_ARCH)
$(call SAS_AssertNotNull,SAS_KERNEL_LOADADDRESS)

$(call SAS_AssertNotNull,SAS_TOOLCHAIN_KERNEL_PREFIX)
SAS_CROSS_COMPILE	:= $(SAS_TOOLCHAIN_KERNEL_PREFIX)

SAS_INITRAMFS_UID	:= $(shell id -u)
SAS_INITRAMFS_GID	:= $(shell id -g)
SAS_INITRAMFS_SOURCE	:= $(SAS_INITRAMFS_INSTALL_PATH) $(SAS_INITRAMFS_FILE_LIST)

SAS_COMPILE_ALWAYS	?= y
SAS_COMPILE_MODULES	?= y
SAS_COMPILE_CHECKSTACK	?= n
SAS_COMPILE_CHECKSECTIONS ?= n
SAS_COMPILE_EXTRA	?= n

SAS_BUILTIN_INITRAMFS	?= y

KBUILD_VMLINUX		:= vmlinux.bin
TARGET_VMLINUX		:= vmlinux.bin

KMOD_INSTALL_PATH	?= $(SAS_ROOTFS_INSTALL_PATH)
KMOD_EXTRA_INSTALL_PATH	?= $(SAS_INITRAMFS_INSTALL_PATH)/modules

SAS_SCRIPT_PERM_FIX_FILES	:= $(wildcard $(addprefix $(SAS_SOURCE)/scripts/,$(SAS_SCRIPT_PERM_FIX)))
SAS_SCRIPT_PERM_FIX_STAMPS	:= $(SAS_SCRIPT_PERM_FIX_FILES:$(SAS_SOURCE)/scripts/%=$(SAS_BUILD)/permfix.%.stamp)

$(call SAS_AssertFileExists,$(SAS_KCONFIG_SCRIPT))
$(call SAS_TraceVariable,SAS_KCONFIG_SCRIPT)
$(call SAS_TraceVariable,SAS_KCONFIG_ARGS)
$(call SAS_TraceVariable,SAS_KCONFIG_OPTS)
$(call SAS_TraceVariable,SAS_KCONFIG_DEPS)
