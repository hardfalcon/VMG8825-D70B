##############################################################################
# File:      prepare.mk                                                      #
# Purpose:   BATS unit-test buildstep prepare                                #
#                                                                            #
# Copyright: Copyright (C) 2018, Sphairon GmbH (a ZyXEL company)             #
##############################################################################

.SUFFIXES:
.PHONY: prepare

# get the current SAS workspace root folder (i.e. the folder above us
# containing the directory 'iad')
SAS_WORKSPACE_ROOT ?= $(firstword $(subst /iad/, ,$(CURDIR)))

include $(SAS_WORKSPACE_ROOT)/paths.mk
include $(SAS_WORKSPACE_ROOT)/exports.mk
include settings.mk
include $(SAS_TEMPLATES_PATH)/user/settings.mk
include $(SAS_TEMPLATES_PATH)/bats-unittest/settings.mk

$(call SAS_TraceVariable,BATS_TOOL)

ifeq ($(SAS_BUILD),)
$(error "SAS_BUILD required")
endif

ifeq ($(SAS_SOURCE),)
$(error "SAS_SOURCE required")
endif

ifeq ($(BATS_TOOL),)
$(error "bats tool required")
endif

prepare:
