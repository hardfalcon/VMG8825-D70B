##############################################################################
# File:      memcheck.mk                                                     #
# Purpose:   Library specific template of build step coverage                #
# Remarks:                                                                   #
#                                                                            #
# Copyright: Copyright (C) 2007, Sphairon Access Systems GmbH                #
#                                                                            #
# Author:    Thomas Haak                                                     #
# Created:   21.12.2007                                                      #
##############################################################################

.SUFFIXES:
.PHONY: memcheck

# This is a delegating make file for checking memory leakage at module level

ifeq ($(SAS_CONFIG), skip)
memcheck:
	@echo ">>> Skipping buildstep memcheck <<<"
else
memcheck:
	@if [ -d ../test/make ]; then ${MAKE} -C ../test/make -f memcheck.mk; fi
endif
