##############################################################################
# File:      prepare.mk                                                      #
# Purpose:   devicedriver-kbuild implementation of build step prepare.       #
# Remarks:                                                                   #
#                                                                            #
# Copyright: Copyright (C) 2010, Sphairon Technologies GmbH                  #
#                                                                            #
# Author:    Maik Dybek                                                      #
# Created:   28.04.2010                                                      #
##############################################################################

.SUFFIXES:
.PHONY: prepare

# get the current SAS workspace root folder (i.e. the folder above us
# containing the directory 'iad')
SAS_WORKSPACE_ROOT ?= $(firstword $(subst /iad/, ,$(CURDIR)))

include $(SAS_WORKSPACE_ROOT)/paths.mk
include $(SAS_WORKSPACE_ROOT)/exports.mk
include settings.mk
include $(SAS_TEMPLATES_PATH)/devicedriver-kbuild/settings.mk
include $(SAS_TEMPLATES_PATH)/generic/kmod.mk

BUILD_FILES	:= $(SAS_FILES:%=$(SAS_BUILD)/%)
BUILD_DIRS	:= $(sort $(abspath $(dir $(BUILD_FILES))))

prepare: $(KBUILD_FILE) $(BUILD_FILES)

$(KBUILD_FILE): $(SAS_TEMPLATES_PATH)/devicedriver-kbuild/prepare.mk \
		$(SAS_TEMPLATES_PATH)/devicedriver-kbuild/settings.mk \
		$(SAS_TEMPLATES_PATH)/generic/kmod.mk \
		settings.mk | $(SAS_BUILD)
	$(call SAS_Kmod_Create_Kbuild,$@)

$(BUILD_FILES): | $(BUILD_DIRS)

$(SAS_BUILD)/%.c: $(SAS_SOURCE)/%.c
	$(Q)if [ ! -L $@ ]; then rm -f $@; fi
	$(Q)ln -sf $< $@

$(sort $(SAS_BUILD) $(BUILD_DIRS)):
	$(Q)mkdir -p $@

