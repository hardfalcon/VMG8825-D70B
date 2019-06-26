##############################################################################
# File:      install.mk                                                      #
# Purpose:   Makefile for libraries.                                         #
# Remarks:                                                                   #
#                                                                            #
# Copyright: Copyright (C) 2006, Sphairon Access Systems GmbH                #
#                                                                            #
# Author:    Frank Stebich                                                   #
# Created:   17.10.2006                                                      #
##############################################################################

.SUFFIXES:

# get the current SAS workspace root folder (i.e. the folder above us
# containing the directory 'iad')
SAS_WORKSPACE_ROOT ?= $(firstword $(subst /iad/, ,$(CURDIR)))

include $(SAS_WORKSPACE_ROOT)/paths.mk
include $(SAS_WORKSPACE_ROOT)/exports.mk
include settings.mk
include $(SAS_TEMPLATES_PATH)/user/settings.mk
include $(SAS_TEMPLATES_PATH)/library/settings.mk
include $(SAS_TEMPLATES_PATH)/generic/verbose.mk

.PHONY: install

ifeq ($(SAS_LIBTYPE),shared)
INSTALL_TARGETS += $(SAS_INSTALL)/$(TARGET)
ifneq ($(SAS_TARGET_LINK),)
INSTALL_TARGETS += $(SAS_INSTALL)/$(TARGET_LINK)
endif
else ifeq ($(SAS_LIBTYPE),shared-soname)
INSTALL_TARGETS += $(SAS_INSTALL)/$(TARGET_SONAME)
INSTALL_TARGETS += $(SAS_INSTALL)/$(TARGET_VERSIONED)
INSTALL_TARGETS += $(SAS_INSTALL)/$(TARGET_UNVERSIONED)
ifneq ($(SAS_TARGET_LINK),)
INSTALL_TARGETS += $(SAS_INSTALL)/$(TARGET_LINK)
INSTALL_TARGETS += $(SAS_INSTALL)/$(TARGET_SONAME_LINK)
endif
endif


ifeq ($(SAS_CONFIG), skip)
install:
	@echo ">>> Skipping buildstep install <<<"
else
install: $(INSTALL_TARGETS)
endif

$(SAS_INSTALL):
	$(Q)mkdir -p $@

quiet_cmd_install = INSTALL   $(<F) -> $(@:$(SAS_WORKSPACE_ROOT)/%=%)
cmd_install = cp -f $< $@

ifeq ($(SAS_LIBTYPE),shared-soname)
# Rule for installing a symlink without any version info
$(SAS_INSTALL)/$(TARGET_UNVERSIONED): $(SAS_INSTALL)/$(TARGET_SONAME)
	$(Q)rm -f $@
	$(Q)cd $(@D) && ln -s $(<F) $(@F)
else
# Rule for native installing of target binary
$(SAS_INSTALL)/$(TARGET): $(SAS_BUILD)/$(TARGET) | $(SAS_INSTALL)
	$(Q)rm -f $(SAS_INSTALL)/$(TARGET)*
	$(call cmd,install)
endif

# Rule for installing a symlink with alternative name to target binary
ifneq ($(TARGET_LINK),)
$(SAS_INSTALL)/$(TARGET_LINK): $(SAS_INSTALL)/$(TARGET)
	$(Q)rm -f $@
	$(Q)cd $(@D) && ln -s $(<F) $(@F)
endif

# Rule for installing the binary target by extending its name with version info
ifneq ($(TARGET_VERSIONED),)
$(SAS_INSTALL)/$(TARGET_VERSIONED): $(SAS_BUILD)/$(TARGET) | $(SAS_INSTALL)
	$(Q)rm -f $(SAS_INSTALL)/$(TARGET)*
	$(call cmd,install)
endif

# Rule for installing a symlink with only major version number
ifneq ($(TARGET_SONAME),)
$(SAS_INSTALL)/$(TARGET_SONAME): $(SAS_INSTALL)/$(TARGET_VERSIONED)
	$(Q)rm -f $@
	$(Q)cd $(@D) && ln -s $(<F) $(@F)
endif

ifneq ($(TARGET_SONAME_LINK),)
# Rule for installing a symlink with alternative name to versioned target binary
$(SAS_INSTALL)/$(TARGET_SONAME_LINK): $(SAS_INSTALL)/$(TARGET_SONAME)
	$(Q)rm -f $@
	$(Q)cd $(@D) && ln -s $(<F) $(@F)
endif
