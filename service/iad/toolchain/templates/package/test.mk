##############################################################################
# File:      test.mk                                                         #
# Purpose:   Build step test.                                                #
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
test:
	@echo ">>> Skipping buildstep test <<<"
else
test:	${SAS_SUBSYS}
endif

${SAS_SUBSYS}:
	@+$(MAKE) -C $@/make -f test.mk SAS_CONFIG=${SAS_CONFIG} --no-print-directory
