##############################################################################
# File:      clean.mk                                                        #
# Purpose:   Library specific template for build step clean.                 #
# Remarks:                                                                   #
#                                                                            #
# Copyright: Copyright (C) 2006, Sphairon Access Systems GmbH                #
#                                                                            #
# Author:    Frank Stebich                                                   #
# Created:   17.10.2006                                                      #
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
include $(SAS_TEMPLATES_PATH)/library/settings.mk

clean:
	-rm -rf $(SAS_BUILD)
ifeq ($(SAS_SKIP_UNIT_TESTS),no)
	-if [ -d ../test/make ]; then $(MAKE) -C ../test/make -f clean.mk; fi
endif
