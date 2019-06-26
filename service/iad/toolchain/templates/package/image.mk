##############################################################################
# File:      image.mk                                                        #
# Purpose:   Build step image.                                               #
# Remarks:                                                                   #
#                                                                            #
# Copyright: Copyright (C) 2006, Sphairon Access Systems GmbH                #
#                                                                            #
# Author:    Frank Stebich                                                   #
# Created:   23.11.2006                                                      #
##############################################################################


include settings.mk


.PHONY: ${SAS_PROJECTS}


ifeq ($(SAS_CONFIG), skip)
image:
	@echo ">>> Skipping buildstep image <<<"
else
image:	${SAS_PROJECTS}
endif

${SAS_PROJECTS}:
	@+$(MAKE) -C $@/make -f image.mk SAS_CONFIG=${SAS_CONFIG} --no-print-directory
