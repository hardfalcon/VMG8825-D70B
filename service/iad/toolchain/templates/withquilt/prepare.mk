##############################################################################
# File:      prepare.mk                                                      #
# Purpose:   Default implementation of build step prepare.                   #
# Remarks:                                                                   #
#                                                                            #
# Copyright: Copyright (C) 2009, Sphairon Access Systems GmbH                #
#                                                                            #
# Author:    Frank Seidel                                                    #
# Created:   30.06.2009                                                      #
##############################################################################

.SUFFIXES:

.PHONY: patch

include settings.mk

SOURCE		?= $(SAS_SOURCE)
EXTRACT_DIR	?= $(SOURCE)
PATCHES_DIR	?= $(ROOT)/patches.sas
PATCHES		:= $(wildcard $(PATCHES_DIR)/*)

patch: $(EXTRACT_DIR)/patch.stamp
$(EXTRACT_DIR)/patch.stamp: settings.mk prepare.mk $(PATCHES)
	@echo ">>> Patching with quilt in patches.sas <<<"
	@rm -rf $(SAS_BUILD)
	@mkdir -p $(SAS_BUILD)
	@ln -s $(ROOT)/scripts   $(SAS_BUILD)/
	@ln -s $(ROOT)/patches.* $(SAS_BUILD)/
	@ln -s $(ROOT)/*series*  $(SAS_BUILD)/
	@ln -s $(ROOT)/*.tar.*   $(SAS_BUILD)/
	@cd $(SAS_BUILD) && ./scripts/setup_all.sh $(SAS_CONFIG)
	@cd $(EXTRACT_DIR) && ../scripts/quiltpusha.sh
	@if [ -f $(ROOT)/config.cache ]; then \
	    echo ">>> Bringing in prepared config.cache <<<"; \
	    cp $(ROOT)/config.cache $(SOURCE); \
	fi
	@touch $@
