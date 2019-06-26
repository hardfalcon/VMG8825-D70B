##############################################################################
# File:      prepare.mk                                                      #
# Purpose:   cppunit-test specific template for build step prepare.          #
# Remarks:   Checks for the existance of mandatory make variables.           #
#                                                                            #
# Copyright: Copyright (C) 2006, Sphairon Access Systems GmbH                #
#                                                                            #
# Author:    Ullrich Meyer                                                   #
# Created:   29.11.2006                                                      #
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
include $(SAS_TEMPLATES_PATH)/cppunittest/settings.mk


#ifeq ($(SAS_INCLUDE),)
#$(error "SAS_INCLUDE required")
#endif

ifeq ($(SAS_SOURCE),)
$(error "SAS_SOURCE required")
endif

ifeq ($(SAS_BUILD),)
$(error "SAS_BUILD required")
endif

ifeq ($(SAS_TARGET),)
$(error "SAS_TARGET required")
endif

prepare:

