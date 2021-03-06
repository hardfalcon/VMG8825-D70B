##############################################################################
# File:      generate.mk                                                     #
# Purpose:   Default implementation of build step generate.                  #
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

.PHONY: generate

generate: ${SAS_BUILD}/Makefile

${SAS_BUILD}/Makefile: ${SAS_TEMPLATES_PATH}/qt-application/application.pro
	cd ${SAS_TEMPLATES_PATH}/qt-application && $(SAS_PLATFORM-SDK_ROOT)/qt/bin/qmake -o ${SAS_BUILD}/Makefile
