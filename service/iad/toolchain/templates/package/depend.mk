##############################################################################
# File:      depend.mk                                                       #
# Purpose:   Build step depend.                                              #
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
depend:
	@echo ">>> Skipping buildstep depend <<<"
else
depend:	${SAS_SUBSYS}
endif

${SAS_SUBSYS}:
	@+$(MAKE) -C $@/make -f depend.mk SAS_CONFIG=${SAS_CONFIG} --no-print-directory
