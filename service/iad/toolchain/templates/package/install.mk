##############################################################################
# File:      install.mk                                                      #
# Purpose:   Build step install.                                             #
# Remarks:                                                                   #
#                                                                            #
# Copyright: Copyright (C) 2006, Sphairon Access Systems GmbH                #
#                                                                            #
# Author:    Frank Stebich                                                   #
# Created:   23.11.2006                                                      #
##############################################################################


include settings.mk


.PHONY: ${SAS_SUBSYS}
.NOTPARALLEL:

ifeq ($(SAS_CONFIG), skip)
install:
	@echo ">>> Skipping buildstep install <<<"
else
install:	${SAS_SUBSYS}
endif

${SAS_SUBSYS}:
	@+$(MAKE) -C $@/make -f install.mk SAS_CONFIG=${SAS_CONFIG} --no-print-directory
