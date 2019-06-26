##############################################################################
# File:      install.mk                                                      #
# Purpose:   Linux kernel implementation of buildstep install                #
#                                                                            #
# Copyright: Copyright (C) 2011, Sphairon Technologies GmbH                  #
#                                                                            #
# Author:    Daniel Schwierzeck                                              #
# Created:   20.09.2011                                                      #
##############################################################################

.SUFFIXES:
MAKEFLAGS := -rR --no-print-directory

# get the current SAS workspace root folder (i.e. the folder above us
# containing the directory 'iad')
SAS_WORKSPACE_ROOT ?= $(firstword $(subst /iad/, ,$(CURDIR)))

include $(SAS_WORKSPACE_ROOT)/paths.mk
include $(SAS_WORKSPACE_ROOT)/exports.mk
include settings.mk
include $(SAS_TEMPLATES_PATH)/generic/verbose.mk
include $(SAS_TEMPLATES_PATH)/generic/debug.mk
include $(SAS_TEMPLATES_PATH)/linux-kernel/settings.mk

SDK_IMAGES_PATH		:= $(SAS_PLATFORM-SDK_IMAGES_PATH)/kernel

INSTALL_FILES		+= $(TARGET_VMLINUX)
INSTALL_FILES		+= vmlinux System.map Module.symvers .missing-syscalls.d
INSTALL_FILES		+= $(notdir $(wildcard $(SAS_BUILD)/linux.*))
INSTALL_FILES_SDK	:= $(INSTALL_FILES:%=$(SDK_IMAGES_PATH)/%)

INSTALL-TARGETS-y			+= $(INSTALL_FILES_SDK)
INSTALL-TARGETS-$(SAS_COMPILE_MODULES)	+= modules-install

.PHONY: install
install: $(INSTALL-TARGETS-y)

# Target for installation of kernel modules
.PHONY: modules-install
modules-install:
	$(Q)+$(MAKE) -C $(SAS_KBUILD_DIR) modules_install \
		ARCH=$(SAS_KERNEL_ARCH) \
		CROSS_COMPILE=$(SAS_CROSS_COMPILE) \
		INSTALL_MOD_PATH=$(KMOD_INSTALL_PATH)
	$(Q)rm -f $(KMOD_INSTALL_PATH)/lib/modules/*/build
	$(Q)rm -f $(KMOD_INSTALL_PATH)/lib/modules/*/source

# Implicit rules to install all images to platform SDK
$(SDK_IMAGES_PATH)/%: $(SAS_BUILD)/% | $(SDK_IMAGES_PATH)
	$(Q)install -m644 $< $@

$(SDK_IMAGES_PATH):
	$(Q)mkdir -p $@
