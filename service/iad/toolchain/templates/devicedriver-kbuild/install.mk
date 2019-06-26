##############################################################################
# File:      install.mk                                                      #
# Purpose:   devicedriver-kbuild implementation of build step install.       #
# Remarks:                                                                   #
#                                                                            #
# Copyright: Copyright (C) 2010, Sphairon Technologies GmbH                  #
#                                                                            #
# Author:    Maik Dybek                                                      #
# Created:   28.04.2010                                                      #
##############################################################################

.SUFFIXES:
.PHONY: install

# get the current SAS workspace root folder (i.e. the folder above us
# containing the directory 'iad')
SAS_WORKSPACE_ROOT ?= $(firstword $(subst /iad/, ,$(CURDIR)))

include $(SAS_WORKSPACE_ROOT)/paths.mk
include $(SAS_WORKSPACE_ROOT)/exports.mk
include settings.mk
include $(SAS_TEMPLATES_PATH)/generic/verbose.mk
include $(SAS_TEMPLATES_PATH)/devicedriver-kbuild/settings.mk

install:
	$(Q)$(MAKE) -C $(SAS_LINUX_BUILD_PATH) modules_install \
		INSTALL_MOD_DIR=$(KBUILD_MOD_DIR) \
		INSTALL_MOD_PATH=$(SAS_ROOTFS_INSTALL_PATH) \
		KBUILD_VERBOSE=$(KBUILD_VERBOSE) \
		KBUILD_EXTMOD=$(SAS_BUILD) \
		ARCH=$(SAS_LINUX_26_ARCH) \
		CROSS_COMPILE=$(SAS_CROSS_COMPILE)
