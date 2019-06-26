##############################################################################
# File:      prepare.mk                                                      #
# Purpose:   Default implementation of build step prepare.                   #
# Remarks:   based on git-source prepare template                            #
#                                                                            #
# Copyright: Copyright (C) 2015, Sphairon GmbH (A ZyXEL Company)             #
#                                                                            #
# Author:    Maik Dybek                                                      #
# Created:   07.07.2015                                                      #
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

$(call SAS_AssertNotNull,SAS_GIT_TREE)

SAS_BUILD	?= $(dir $(CURDIR))/build
SAS_STAMPS	:= $(SAS_BUILD)/stamps

GIT_STAMP	:= $(SAS_STAMPS)/git-$(subst /,_,$(SAS_GIT_BRANCH)).stamp
GIT_STAMPS	:= $(wildcard $(SAS_STAMPS)/git-*)
GIT		:= $(shell which git)
GIT_SOURCE	:= $(wildcard $(dir $(CURDIR))meta/git-source.conf)

$(call SAS_AssertNotNull,GIT)
$(call SAS_AssertNotNull,GIT_SOURCE)

.PHONY: prepare showbranch
prepare: $(SAS_BUILD)/prepare.stamp showbranch

$(SAS_BUILD)/prepare.stamp: $(GIT_STAMP)
ifneq ($(SAS_SOURCE_LINK),)
	@echo ">>> Creating source link: $(notdir $(abspath $(SAS_SOURCE_LINK))) <<<"
	$(Q)rm -f $(SAS_SOURCE_LINK)
	$(Q)cd $(dir $(abspath $(SAS_SOURCE_LINK))) && ln -s $(SAS_GIT_TREE) $(notdir $(abspath $(SAS_SOURCE_LINK)))
endif
	@touch $@

$(GIT_STAMP): prepare.mk $(GIT_SOURCE) | $(SAS_STAMPS)
	$(Q)$(if $(GIT_STAMPS),rm -f $(GIT_STAMPS),)
ifneq ($(SAS_GIT_BRANCH),)
	@echo ">>> Checking out branch $(SAS_GIT_BRANCH) <<<"
	$(Q)test -d $(SAS_GIT_TREE)/.git
	$(Q)cd $(SAS_GIT_TREE) && LANG=C $(GIT) checkout $(SAS_GIT_BRANCH)
endif
	@touch $@

showbranch:
	@echo ">>> Branch state after prepare.mk <<<"
	$(Q)cd $(SAS_GIT_TREE) && LANG=C $(GIT) status -v

$(SAS_BUILD) $(SAS_STAMPS):
	$(Q)mkdir -p $@
