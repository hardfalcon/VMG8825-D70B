##############################################################################
# File:      prepare.mk                                                      #
# Purpose:   Default implementation of build step prepare.                   #
# Remarks:                                                                   #
#                                                                            #
# Copyright: Copyright (C) 2008, Sphairon Access Systems GmbH                #
#                                                                            #
# Author:    Daniel Egger                                                    #
# Created:   22.12.2008                                                      #
##############################################################################

.SUFFIXES:

.PHONY: patch

.NOTPARALLEL:

include settings.mk
include $(SAS_TEMPLATES_PATH)/generic/pkg-requirements.mk
include $(SAS_TEMPLATES_PATH)/generic/debug.mk

$(call SAS_AssertNotNull,SAS_SOURCE)
$(call SAS_AssertNotNull,SAS_BUILD)
$(call SAS_AssertNotNull,TARBALL)

EXTRACT_DIR ?= $(SAS_BUILD)/source/

include $(SAS_TEMPLATES_PATH)/generic/tarball.mk

ifeq ($(SAS_CONFIG),pre-built)
patch:
	@echo ">>> Using pre-built binaries. Nothing to prepare. <<<"
else
patch: $(EXTRACT_DIR)/patch.stamp $(EXTRA_FILES)
endif

$(SAS_BUILD)/prepare.stamp: $(EXTRACT_DIR)/patch.stamp

$(EXTRACT_DIR)/patch.stamp: $(EXTRACT_DIR)/extract.stamp
	$(call SAS_Tarball_Apply_Patches)
	@touch $@


$(EXTRACT_DIR)/extract.stamp: settings.mk prepare.mk $(ROOT)/$(TARBALL) $(PATCHES)
	$(call SAS_Tarball_Clean_Source_Dir,$(EXTRACT_DIR))
	$(call SAS_Tarball_Extract_Tarball,$(ROOT)/$(TARBALL),$(EXTRACT_DIR))
	@touch $@

include $(SAS_TEMPLATES_PATH)/generic/pkg-implicit-target.mk
