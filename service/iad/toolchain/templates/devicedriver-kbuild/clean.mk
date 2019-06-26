##############################################################################
# File:      clean.mk                                                        #
# Purpose:   devicedriver-kbuild implementation of build step clean.         #
# Remarks:                                                                   #
#                                                                            #
# Copyright: Copyright (C) 2010, Sphairon Technologies GmbH                  #
#                                                                            #
# Author:    Maik Dybek                                                      #
# Created:   28.04.2010                                                      #
##############################################################################

.SUFFIXES:
.PHONY: clean

# get the current SAS workspace root folder (i.e. the folder above us
# containing the directory 'iad')
SAS_WORKSPACE_ROOT ?= $(firstword $(subst /iad/, ,$(CURDIR)))

include $(SAS_WORKSPACE_ROOT)/paths.mk
include $(SAS_WORKSPACE_ROOT)/exports.mk
include settings.mk
include $(SAS_TEMPLATES_PATH)/devicedriver-kbuild/settings.mk


clean:
	-rm -rf $(SAS_BUILD)
