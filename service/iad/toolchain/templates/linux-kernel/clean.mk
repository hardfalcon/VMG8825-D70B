##############################################################################
# File:      clean.mk                                                        #
# Purpose:   Linux kernel implementation of buildstep clean                  #
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

$(call SAS_AssertNotNull,SAS_BUILD)

.PHONY: clean
clean:
	rm -rf $(SAS_BUILD)
