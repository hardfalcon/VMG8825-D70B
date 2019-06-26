##############################################################################
# File:      coverage.mk                                                     #
# Purpose:   Build step coverage.                                            #
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
coverage:
	@echo ">>> Skipping buildstep coverage <<<"
else
coverage:	${SAS_SUBSYS}
endif

${SAS_SUBSYS}:
	@+$(MAKE) -C $@/make -f coverage.mk SAS_CONFIG=${SAS_CONFIG} --no-print-directory
