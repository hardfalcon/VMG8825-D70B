##############################################################################
# File:      update-patches.mk                                               #
# Purpose:   Generic creation of patch series from git repository.           #
# Remarks:                                                                   #
#                                                                            #
# Copyright: Copyright (C) 2010, Sphairon Technologies GmbH                  #
#                                                                            #
# Author:    Daniel Schwierzeck                                              #
# Created:   19.07.2010                                                      #
##############################################################################

.SUFFIXES:
.NOTPARALLEL:

# get the current SAS workspace root folder (i.e. the folder above us
# containing the directory 'iad')
SAS_WORKSPACE_ROOT ?= $(firstword $(subst /iad/, ,$(CURDIR)))

include $(SAS_WORKSPACE_ROOT)/paths.mk
include $(SAS_WORKSPACE_ROOT)/exports.mk
include settings.mk

SHELL := $(shell if [ -x "$$BASH" ]; then echo $$BASH; \
	  else if [ -x /bin/bash ]; then echo /bin/bash; \
	  else echo sh; fi ; fi)

GIT := $(shell which git)
ifeq ($(GIT),)
$(error "No git binary found")
endif

ifeq ("$(origin V)", "command line")
SAS_VERBOSE = $(V)
endif
ifndef SAS_VERBOSE
  SAS_VERBOSE = 0
endif
ifeq ($(SAS_VERBOSE),1)
  Q =
else
  Q = @
endif

SOURCE		?= $(SAS_SOURCE)
EXTRACT_DIR	?= $(ROOT)/build/source/
SOURCE_DIR	?= $(EXTRACT_DIR)
SAS_PATCH_DIR	?= $(ROOT)/patches
SAS_SED_SCRIPT	:= $(SAS_TEMPLATES_PATH)/git-source/signature-filter.sed


.PHONY: update-patches

update-patches: | $(SAS_PATCH_DIR)
	$(Q)if [ ! -d $(SOURCE) ]; then \
		echo ">>> Git source directory not found: $(SOURCE) <<<"; \
		exit 1; \
	fi
	$(Q)pushd $(SOURCE) > /dev/null; \
	echo ">>> Creating patches from origin..master in $(SAS_PATCH_DIR) <<<"; \
	rm -rf $(SAS_PATCH_DIR)/*; \
	LANG=C $(GIT) format-patch -k --no-renames -o $(SAS_PATCH_DIR) origin..master; \
	popd > /dev/null
	$(Q)for p in $$(find $(SAS_PATCH_DIR) -iname "*.patch"); do \
		sed -r -f $(SAS_SED_SCRIPT) -i $$p; \
		sed -e "/^From \([a-z0-9]*\)\(.*\)/d" -i $$p; \
	done

$(SAS_PATCH_DIR):
	$(Q)install -d $@
