##############################################################################
# File:      prepare.mk                                                      #
# Purpose:   Default implementation of build step prepare.                   #
# Remarks:                                                                   #
#                                                                            #
# Copyright: Copyright (C) 2010, Sphairon Access Systems GmbH                #
#                                                                            #
# Author:    Daniel Schwierzeck                                              #
# Created:   11.03.2010                                                      #
##############################################################################

.SUFFIXES:
.NOTPARALLEL:

# get the current SAS workspace root folder (i.e. the folder above us
# containing the directory 'iad')
SAS_WORKSPACE_ROOT ?= $(firstword $(subst /iad/, ,$(CURDIR)))

include $(SAS_WORKSPACE_ROOT)/paths.mk
include $(SAS_WORKSPACE_ROOT)/exports.mk
include settings.mk
include $(SAS_TEMPLATES_PATH)/generic/verbose.mk
include $(SAS_TEMPLATES_PATH)/generic/debug.mk
include $(SAS_TEMPLATES_PATH)/generic/git-source.mk

$(call SAS_AssertNotNull,SAS_TARBALL)
$(call SAS_AssertNotNull,SAS_TARBALL_DIR)
$(call SAS_AssertNotNull,SAS_GIT_PATCH_DIR)
$(call SAS_AssertNotNull,SAS_SOURCE)
$(call SAS_AssertNotNull,SAS_EXTRACT_DIR)
$(call SAS_AssertNotNull,SAS_GIT_BRANCH)
$(call SAS_AssertNotNull,SAS_GIT_TREE)

SAS_BUILD	?= $(dir $(CURDIR))/build
SAS_STAMPS	:= $(SAS_BUILD)/stamps

SAS_USE_GIT	:= $(call SAS_UseGit,$(dir $(CURDIR)))

ifeq ($(SAS_USE_GIT),y)
GIT_STAMP	:= $(SAS_STAMPS)/git-$(subst /,_,$(SAS_GIT_BRANCH)).stamp
GIT_STAMPS	:= $(wildcard $(SAS_STAMPS)/git-*)
PREPARE		:= $(GIT_STAMP)
GIT		:= $(shell which git)
$(call SAS_AssertNotNull,GIT)
GIT_SOURCE	:= $(wildcard $(dir $(CURDIR))meta/git-source.conf)
$(call SAS_AssertNotNull,GIT_SOURCE)
else
PREPARE		:= $(SAS_STAMPS)/patch.stamp
endif

PATCH_FILES	:= $(sort $(wildcard $(SAS_GIT_PATCH_DIR)/*.patch))

ifeq ($(PATCH_FILES),)
$(info >>> Patches: none found in $(SAS_GIT_PATCH_DIR))
endif


.PHONY: prepare
prepare: $(SAS_BUILD)/prepare.stamp

$(SAS_BUILD)/prepare.stamp: $(PREPARE) | $(SAS_STAMPS)
ifneq ($(SAS_SOURCE_LINK),)
	@echo ">>> Creating source link: $(notdir $(abspath $(SAS_SOURCE_LINK))) <<<"
	$(Q)rm -f $(SAS_SOURCE_LINK)
	$(Q)cd $(dir $(abspath $(SAS_SOURCE_LINK))) && ln -s $(SAS_SOURCE) $(notdir $(abspath $(SAS_SOURCE_LINK)))
endif
	@touch $@

$(SAS_STAMPS)/extract.stamp: $(SAS_TARBALL_DIR)/$(SAS_TARBALL) $(PATCH_FILES) prepare.mk | $(SAS_STAMPS)
	$(Q)if [ -d $(SAS_EXTRACT_DIR) ]; then \
		echo ">>> Cleaning source directory <<<"; \
		chmod +w -R $(SAS_EXTRACT_DIR); \
		rm -rf $(SAS_EXTRACT_DIR); \
	fi
	@echo ">>> Extracting tarball: $(SAS_TARBALL) <<<"
	$(Q)install -d $(SAS_EXTRACT_DIR)
	$(Q)tar -axf $(SAS_TARBALL_DIR)/$(SAS_TARBALL) -C $(SAS_EXTRACT_DIR)
	@touch $@

$(SAS_STAMPS)/patch.stamp: $(SAS_STAMPS)/extract.stamp | $(SAS_STAMPS)
ifneq ($(PATCH_FILES),)
	$(Q)for p in $(PATCH_FILES); do \
		echo ">>> Applying patch: $$p <<<"; \
		patch -f -d $(SAS_SOURCE) -p 1 -i $$p || exit 1; \
	done
else
	@echo ">>> No patches to apply <<<"
endif
	@touch $@

$(GIT_STAMP): prepare.mk $(GIT_SOURCE) | $(SAS_STAMPS)
	$(Q)$(if $(GIT_STAMPS),rm -f $(GIT_STAMPS),)
	@echo ">>> Checking out branch $(SAS_GIT_BRANCH) <<<"
	$(Q)test -d $(SAS_GIT_TREE)/.git
	$(Q)cd $(SAS_GIT_TREE) && LANG=C $(GIT) checkout $(SAS_GIT_BRANCH)
	@touch $@

$(SAS_BUILD) $(SAS_STAMPS):
	$(Q)mkdir -p $@
