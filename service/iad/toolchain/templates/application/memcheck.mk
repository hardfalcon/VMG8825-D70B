##############################################################################
# File:      memcheck.mk                                                     #
# Purpose:   Application template for build step memcheck                    #
# Remarks:                                                                   #
#                                                                            #
# Copyright: Copyright (C) 2007, Sphairon Access Systems GmbH                #
#                                                                            #
# Author:    Thomas Haak                                                     #
# Created:   21.12.2007                                                      #
##############################################################################

.SUFFIXES:
.PHONY: memcheck

# get the current SAS workspace root folder (i.e. the folder above us
# containing the directory 'iad')
SAS_WORKSPACE_ROOT ?= $(firstword $(subst /iad/, ,$(CURDIR)))

include $(SAS_WORKSPACE_ROOT)/paths.mk
include $(SAS_WORKSPACE_ROOT)/exports.mk
include settings.mk
include $(SAS_TEMPLATES_PATH)/user/settings.mk
include $(SAS_TEMPLATES_PATH)/application/settings.mk

# Check if valgrind is available
VALGRIND_AVAILABLE	:= $(shell which valgrind)
# Check which option for XML output is supported
ifneq ($(VALGRIND_AVAILABLE),)
VALGRIND_XML_OLD	:= $(shell valgrind --help | grep '\"--log-file-exactly\"')
ifneq ($(VALGRIND_XML_OLD),)
VALGRIND_XML_OPTION	:= '--log-file-exactly=memcheck.xml'
else
VALGRIND_XML_OPTION	:=
endif
endif

ifeq ($(SAS_MEMCHECK_REPORT_TYPE),XML)
SAS_MEMCHECK = valgrind --xml=yes --leak-check=full $(VALGRIND_XML_OPTION)
else
SAS_MEMCHECK = valgrind --leak-check=full
endif

SAS_MEMCHECK_REPORT = $(SAS_TOOLS_BIN)/memcheck-report.sh
SAS_MODULE_DIR ?= $(shell pwd)/../..

ifeq ($(SAS_CONFIG),skip)
memcheck:
	@echo ">>> Skipping buildstep memcheck <<<"
else
ifeq ($(SAS_TOOLCHAIN),$(SAS_HOST_TOOLCHAIN))

memcheck:
	# In order to generate information from the memory check we need to
	# execute the target together with the memory checker, but we are not
	# interested in its return code (and therefore ignore it).
ifneq ($(VALGRIND_AVAILABLE),)
ifneq ($(VALGRIND_XML_OLD),)
	$(SAS_MEMCHECK) $(SAS_BUILD)/$(SAS_TARGET)
else
	$(SAS_MEMCHECK) $(SAS_BUILD)/$(SAS_TARGET) > memcheck.xml 2>&1
endif
	$(SAS_MEMCHECK_REPORT) $(SAS_MODULE_DIR)
endif

else

memcheck:

endif
endif
