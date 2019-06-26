##############################################################################
# File:      test.mk                                                         #
# Purpose:   cppunit-test template of build step test.                       #
# Remarks:                                                                   #
#                                                                            #
# Copyright: Copyright (C) 2006, Sphairon Access Systems GmbH                #
#                                                                            #
# Author:    Ullrich Meyer                                                   #
# Created:   29.11.2006                                                      #
##############################################################################

.SUFFIXES:
.PHONY: test

# get the current SAS workspace root folder (i.e. the folder above us
# containing the directory 'iad')
SAS_WORKSPACE_ROOT ?= $(firstword $(subst /iad/, ,$(CURDIR)))

include $(SAS_WORKSPACE_ROOT)/paths.mk
include $(SAS_WORKSPACE_ROOT)/exports.mk
include settings.mk
include $(SAS_TEMPLATES_PATH)/user/settings.mk
include $(SAS_TEMPLATES_PATH)/cppunittest/settings.mk

ifeq ($(SAS_CONFIG), skip)

test:
	@echo ">>> Skipping buildstep test <<<"

else

test:
	@echo ">>> Running unit test: $(SAS_TARGET) <<<"
	$(SAS_BUILD)/$(SAS_TARGET)

endif
