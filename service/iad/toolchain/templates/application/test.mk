##############################################################################
# File:      test.mk                                                         #
# Purpose:   Application-specific template of build step test.               #
# Remarks:                                                                   #
#                                                                            #
# Copyright: Copyright (C) 2006, Sphairon Access Systems GmbH                #
#                                                                            #
# Author:    Frank Stebich                                                   #
# Created:   29.09.2006                                                      #
##############################################################################

.SUFFIXES:
.PHONY: test

# get the current SAS workspace root folder (i.e. the folder above us
# containing the directory 'iad')
SAS_WORKSPACE_ROOT ?= $(firstword $(subst /iad/, ,$(CURDIR)))

include $(SAS_WORKSPACE_ROOT)/paths.mk
include $(SAS_WORKSPACE_ROOT)/exports.mk
include settings.mk

ifeq ($(SAS_CONFIG), skip)
test:
	@echo ">>> Skipping buildstep test <<<"
else
test:
ifeq ($(SAS_SKIP_UNIT_TESTS),no)
	@if [ -d ../test/make ]; then $(MAKE) -C ../test/make -f test.mk; fi
endif
endif
