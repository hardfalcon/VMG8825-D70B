##############################################################################
# File:      configure.mk                                                    #
# Purpose:   Linux kernel implementation of buildstep configure              #
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

LINUX_SDK_HEADER_DIRS	:= linux asm asm-generic
LINUX_SDK_HEADER_DIRS	:= $(addprefix $(SAS_PLATFORM-SDK_INCLUDE_PATH)/,$(LINUX_SDK_HEADER_DIRS))
SAS_SOURCE_STAMP	:= $(SAS_BUILD)/source-$(subst /,_,$(SAS_SOURCE)).stamp
SAS_CONFIG_STAMP	:= $(SAS_BUILD)/config-$(subst /,_,$(SAS_CONFIG)).stamp

$(call SAS_TraceVariable,SAS_CROSS_COMPILE)

.PHONY: configure
configure: $(SAS_BUILD)/configure.stamp

$(SAS_BUILD)/configure.stamp:	$(SAS_BUILD)/oldconfig.stamp $(SAS_SOURCE_STAMP)
	@touch $@

$(SAS_CONFIG_STAMP):	$(SAS_TEMPLATES_PATH)/linux-kernel/settings.mk \
			$(SAS_TEMPLATES_PATH)/linux-kernel/configure.mk \
			| $(SAS_BUILD)
	@echo ">>> Using SAS_CONFIG '$(SAS_CONFIG)' <<<"
	@rm -f $(SAS_BUILD)/config-*
	@touch $@

$(SAS_SOURCE_STAMP): | $(SAS_BUILD)
	rm -f $(SAS_BUILD)/source-*
	rm -f $(SAS_SOURCE_LINK)
	ln -s $(SAS_SOURCE) $(SAS_SOURCE_LINK)
	@touch $@

$(SAS_KBUILD_DIR)/kconfig-script.config:	$(SAS_KCONFIG_SCRIPT) \
						$(SAS_CONFIG_STAMP) \
						$(SAS_KCONFIG_DEPS) \
						| $(SAS_KBUILD_DIR)
	$(Q)chmod u+x $(SAS_KCONFIG_SCRIPT)
	$(Q)$(SAS_KCONFIG_SCRIPT) $(SAS_KCONFIG_ARGS) $(SAS_KCONFIG_OPTS) > $@

$(SAS_KBUILD_DIR)/allno.config:	$(SAS_KBUILD_DIR)/kconfig-script.config
	$(Q)sort -u $< > $@

$(SAS_BUILD)/oldconfig.stamp:	$(SAS_KBUILD_DIR)/allno.config \
				$(SAS_SOURCE_STAMP)
	@echo ">>> Configuring kernel <<<"
	$(Q)+$(MAKE) -C $(SAS_SOURCE) allnoconfig \
		ARCH=$(SAS_KERNEL_ARCH) \
		CROSS_COMPILE=$(SAS_CROSS_COMPILE) \
		KBUILD_OUTPUT=$(SAS_KBUILD_DIR) \
		KCONFIG_ALLCONFIG=$<
	$(Q)cp -f $(SAS_KBUILD_DIR)/.config $(SAS_KBUILD_DIR)/.config.compare
	@echo ">>> Preparing module build <<<"
	$(Q)+$(MAKE) -C $(SAS_KBUILD_DIR) modules_prepare \
		ARCH=$(SAS_KERNEL_ARCH) \
		CROSS_COMPILE=$(SAS_CROSS_COMPILE) \
		INSTALL_HDR_PATH=$(SAS_BUILD)
	@echo ">>> Installing kernel headers <<<"
	$(Q)+$(MAKE) -C $(SAS_KBUILD_DIR) headers_install \
		ARCH=$(SAS_KERNEL_ARCH) \
		CROSS_COMPILE=$(SAS_CROSS_COMPILE) \
		INSTALL_HDR_PATH=$(SAS_BUILD)
	@touch $@

$(SAS_KBUILD_DIR):
	$(Q)mkdir -p $@
