##############################################################################
# File:      test.mk                                                         #
# Purpose:   BATS unit-test buildstep test                                   #
#                                                                            #
# Copyright: Copyright (C) 2018, Sphairon GmbH (a ZyXEL company)             #
##############################################################################

.SUFFIXES:
.PHONY: test

# get the current SAS workspace root folder (i.e. the folder above us
# containing the directory 'iad')
SAS_WORKSPACE_ROOT ?= $(firstword $(subst /iad/, ,$(CURDIR)))

include $(SAS_WORKSPACE_ROOT)/paths.mk
include $(SAS_WORKSPACE_ROOT)/exports.mk
include settings.mk
include $(SAS_TEMPLATES_PATH)/bats-unittest/settings.mk

ifeq ($(SAS_CONFIG), skip)

test:
	@echo ">>> Skipping buildstep test <<<"

else

test: | $(SAS_BUILD)
ifneq ($(BATS_TESTS),)
	@echo ">>> Running BATS unit tests in: $(SAS_SOURCE) <<<"
	@$(BATS_TOOL) -t $(BATS_TESTS) | tee $(SAS_BUILD)/$(BATS_RESULTS)
endif

$(SAS_BUILD):
	@mkdir -p $@

endif
