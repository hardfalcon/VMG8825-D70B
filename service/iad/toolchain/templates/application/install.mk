##############################################################################
# File:      install.mk                                                      #
# Purpose:   Application-specific template for build step install.           #
# Remarks:                                                                   #
#                                                                            #
# Copyright: Copyright (C) 2006, Sphairon Access Systems GmbH                #
#                                                                            #
# Author:    Frank Stecbich                                                  #
# Created:   13.10.2006                                                      #
##############################################################################

.SUFFIXES:
.PHONY: install

# get the current SAS workspace root folder (i.e. the folder above us
# containing the directory 'iad')
SAS_WORKSPACE_ROOT ?= $(firstword $(subst /iad/, ,$(CURDIR)))

include $(SAS_WORKSPACE_ROOT)/paths.mk
include $(SAS_WORKSPACE_ROOT)/exports.mk
include settings.mk
include $(SAS_TEMPLATES_PATH)/user/settings.mk
include $(SAS_TEMPLATES_PATH)/application/settings.mk
include $(SAS_TEMPLATES_PATH)/generic/verbose.mk

ifeq ($(SAS_CONFIG), skip)
install:
	@echo ">>> Skipping buildstep install <<<"
else
install: $(SAS_INSTALL)/$(TARGET) $(MULTIBIN_TARGETS)

$(SAS_INSTALL):
	$(Q)mkdir -p $@

quiet_cmd_install = INSTALL   $(<F) -> $(@:$(SAS_WORKSPACE_ROOT)/%=%)
cmd_install = cp -f $< $@

quiet_cmd_multibin = MULTIBIN  $(<:$(SAS_WORKSPACE_ROOT)/%=%) -> $(@:$(SAS_WORKSPACE_ROOT)/%=%)
cmd_multibin = rm -f $@; cd $(@D) && ln -s $(<F) $(@F)

$(SAS_INSTALL)/$(TARGET): $(SAS_BUILD)/$(TARGET) settings.mk install.mk | $(SAS_INSTALL)
	$(call cmd,install)

$(MULTIBIN_TARGETS): $(SAS_INSTALL)/$(TARGET)
	$(call cmd,multibin)

endif
