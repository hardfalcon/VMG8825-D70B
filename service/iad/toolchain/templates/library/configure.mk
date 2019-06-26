##############################################################################
# File:      configure.mk                                                    #
# Purpose:   Library specific template of build step configure.              #
# Remarks:                                                                   #
#                                                                            #
# Copyright: Copyright (C) 2006, Sphairon Access Systems GmbH                #
#                                                                            #
# Author:    Frank Stebich                                                   #
# Created:   29.09.2006                                                      #
##############################################################################

.SUFFIXES:

# get the current SAS workspace root folder (i.e. the folder above us
# containing the directory 'iad')
SAS_WORKSPACE_ROOT ?= $(firstword $(subst /iad/, ,$(CURDIR)))

include $(SAS_WORKSPACE_ROOT)/paths.mk
include $(SAS_WORKSPACE_ROOT)/exports.mk
include settings.mk
include $(SAS_TEMPLATES_PATH)/user/settings.mk
include $(SAS_TEMPLATES_PATH)/library/settings.mk
include $(SAS_TEMPLATES_PATH)/generic/verbose.mk

.PHONY: configure installheaders $(SAS_EXPORT_HEADER_PATH)/%

configure: installheaders
ifeq ($(SAS_SKIP_UNIT_TESTS),no)
	@if [ -d ../test/make ]; then $(MAKE) -C ../test/make -f configure.mk; fi
endif


ifeq ($(SAS_EXPORT_HEADER_PATH),)

installheaders:

else

SRC_HEADER_FILES	:= $(shell find $(SAS_EXPORT_HEADER_PATH) -name "*.h" -o -name "*.hpp" -o -name "*.inl")
DEST_HEADER_FILES	:= $(subst $(SAS_EXPORT_HEADER_PATH),$(SAS_ACTIVE-SDK_INCLUDE_PATH),$(SRC_HEADER_FILES))
DEST_HEADER_DIRS	:= $(sort $(dir $(DEST_HEADER_FILES)))

installheaders: $(DEST_HEADER_FILES)

$(DEST_HEADER_FILES): | $(DEST_HEADER_DIRS)
$(DEST_HEADER_DIRS):
	$(Q)mkdir -p $@

$(SAS_ACTIVE-SDK_INCLUDE_PATH)/%: $(SAS_EXPORT_HEADER_PATH)/% settings.mk configure.mk
	$(Q)cp -fL $< $@

endif
