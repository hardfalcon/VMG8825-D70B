##############################################################################
# File:      coverage.mk                                                     #
# Purpose:   Library specific template of build step coverage                #
# Remarks:                                                                   #
#                                                                            #
# Copyright: Copyright (C) 2007, Sphairon Access Systems GmbH                #
#                                                                            #
# Author:    Thomas Haak                                                     #
# Created:   27.11.2007                                                      #
##############################################################################

.SUFFIXES:
.PHONY: coverage

# This is a delegating make file for analysing test coverage at module level

ifeq ($(SAS_CONFIG), skip)
coverage:
	@echo ">>> Skipping buildstep coverage <<<"
else
coverage:
	if [ -d ../test/make ]; then ${MAKE} -C ../test/make -f coverage.mk; fi
endif
