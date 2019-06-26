##############################################################################
# File:      prepare.mk                                                      #
# Purpose:   Default implementation of build step prepare.                   #
# Remarks:                                                                   #
#                                                                            #
# Copyright: Copyright (C) 2009, Sphairon Access Systems GmbH                #
#                                                                            #
# Author:    Mario Witkowski                                                 #
# Created:   07.07.2009                                                      #
##############################################################################

include settings.mk

ifeq (${SAS_TEMPLATES_PATH},)
$(error "SAS_TEMPLATES_PATH required")
endif

include ${SAS_TEMPLATES_PATH}/qt-default/exports.mk

.PHONY: prepare

prepare: ${SAS_BUILD}/prepare.stamp

${SAS_BUILD}/prepare.stamp: settings.mk
	@echo ">>> Prepare build directory <<<"
	@-rm -rf $(SAS_BUILD)
	@install -d $(SAS_BUILD)
	@touch $@
