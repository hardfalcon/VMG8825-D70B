##############################################################################
# File:      coverage.mk                                                     #
# Purpose:   cppunit-test template for build step coverage                   #
# Remarks:                                                                   #
#                                                                            #
# Copyright: Copyright (C) 2007, Sphairon Access Systems GmbH                #
#                                                                            #
# Author:    Thomas Haak                                                     #
# Created:   27.11.2007                                                      #
##############################################################################

.SUFFIXES:
.PHONY: coverage

# get the current SAS workspace root folder (i.e. the folder above us
# containing the directory 'iad')
SAS_WORKSPACE_ROOT ?= $(firstword $(subst /iad/, ,$(CURDIR)))

include $(SAS_WORKSPACE_ROOT)/paths.mk
include $(SAS_WORKSPACE_ROOT)/exports.mk
include settings.mk
include $(SAS_TEMPLATES_PATH)/user/settings.mk
include $(SAS_TEMPLATES_PATH)/application/settings.mk

SAS_MODULE_DIR	?= $(shell pwd)/../..
SAS_MODULE_DIR	:= $(abspath $(SAS_MODULE_DIR))
SAS_COV_TMPDIR	:= $(SAS_MODULE_DIR)/build
SAS_COV_DIR	:= $(SAS_MODULE_DIR)/cov

ifeq ($(SAS_TOOLCHAIN),$(SAS_HOST_TOOLCHAIN))

coverage: | $(SAS_COV_DIR)
	rm -rf $(SAS_COV_DIR)/*
	lcov -c -d $(SAS_COV_TMPDIR) -o $(SAS_COV_TMPDIR)/coverage.info \
		-b $(SAS_MODULE_DIR) -t $(SAS_TARGET)
	lcov --remove $(SAS_COV_TMPDIR)/coverage.info "/usr*" \
		-o $(SAS_COV_TMPDIR)/coverage.info
	genhtml -o $(SAS_COV_DIR) -s -k --legend --num-spaces 4 \
		--prefix $(SAS_WORKSPACE_ROOT) \
		--title "Unit-level test coverage for $(SAS_TARGET)" \
		$(SAS_COV_TMPDIR)/coverage.info

else

coverage:
	@echo ">>> Buildstep coverage only supported for i386-linux toolchain <<<"
endif

$(SAS_COV_DIR):
	mkdir -p $@
