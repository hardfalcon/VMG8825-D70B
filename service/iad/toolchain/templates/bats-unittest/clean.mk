##############################################################################
# File:      clean.mk                                                        #
# Purpose:   BATS unit-test buildstep clean                                  #
#                                                                            #
# Copyright: Copyright (C) 2018, Sphairon GmbH (a ZyXEL company)             #
##############################################################################

.SUFFIXES:
.PHONY: clean

# get the current SAS workspace root folder (i.e. the folder above us
# containing the directory 'iad')
SAS_WORKSPACE_ROOT ?= $(firstword $(subst /iad/, ,$(CURDIR)))

include $(SAS_WORKSPACE_ROOT)/paths.mk
include $(SAS_WORKSPACE_ROOT)/exports.mk
include settings.mk
include $(SAS_TEMPLATES_PATH)/user/settings.mk
include $(SAS_TEMPLATES_PATH)/bats-unittest/settings.mk

clean:
	-rm -rf $(SAS_BUILD)
