##############################################################################
# File:      install.mk                                                      #
# Purpose:   Default implementation of build step install.                   #
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

.PHONY: install

install:
	$(MAKE) -C ${SAS_BUILD} install
