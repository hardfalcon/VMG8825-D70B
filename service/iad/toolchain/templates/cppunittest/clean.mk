##############################################################################
# File:      clean.mk                                                        #
# Purpose:   CppUnit test specific template for build step clean             #
# Remarks:                                                                   #
#                                                                            #
# Copyright: Copyright (C) 2007, Sphairon Access Systems GmbH                #
#                                                                            #
# Author:    Thomas Haak                                                     #
# Created:   27.11.2007                                                      #
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
include $(SAS_TEMPLATES_PATH)/cppunittest/settings.mk

clean:
	-rm -rf $(SAS_BUILD)
	-rm -rf $(SAS_MC)
	-rm -rf $(SAS_COV)
	-rm -f $(SAS_CPPUNIT_LOGFILENAME)

