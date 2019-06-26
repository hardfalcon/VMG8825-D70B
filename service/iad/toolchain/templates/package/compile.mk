##############################################################################
# File:      compile.mk                                                      #
# Purpose:   Build step compile.                                             #
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
compile:
	@echo ">>> Skipping buildstep compile <<<"
else
compile:	${SAS_SUBSYS}
endif

${SAS_SUBSYS}:
	@+$(MAKE) -C $@/make -f compile.mk SAS_CONFIG=${SAS_CONFIG} --no-print-directory
