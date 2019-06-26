##############################################################################
# File:      prepare.mk                                                      #
# Purpose:   Library-specific template for build step prepare.               #
# Remarks:   Checks for the existance of mandatory make variables.           #
#                                                                            #
# Copyright: Copyright (C) 2006, Sphairon Access Systems GmbH                #
#                                                                            #
# Author:    Frank Stebich                                                   #
# Created:   23.11.2006                                                      #
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
include $(SAS_TEMPLATES_PATH)/library/settings.mk
include $(SAS_TEMPLATES_PATH)/generic/pkg-requirements.mk

ifeq (${SAS_ACTIVE-SDK_ROOT},)
$(error "SAS_ACTIVE-SDK_ROOT required")
endif

ifeq ($(SAS_INCLUDE),)
$(error "SAS_INCLUDE required")
endif

ifeq ($(SAS_SOURCE),)
$(error "SAS_SOURCE required")
endif

ifeq ($(SAS_BUILD),)
$(error "SAS_BUILD required")
endif

ifeq ($(SAS_LIBTYPE),)
$(error "SAS_LIBTYPE required");
endif

ifeq ($(SAS_TARGET),)
$(error "SAS_TARGET required")
endif

ifeq (${SAS_LIBTYPE},shared)
ifeq ($(SAS_INSTALL),)
$(error "SAS_INSTALL required")
endif
endif

prepare: $(EXTRA_FILES)
ifeq ($(SAS_SKIP_UNIT_TESTS),no)
	@if [ -d ../test/make ]; then ${MAKE} -C ../test/make -f prepare.mk; fi
endif

include $(SAS_TEMPLATES_PATH)/generic/pkg-implicit-target.mk
