##############################################################################
# File:      generate.mk                                                     #
# Purpose:   Library-specific template of build step generate.               #
# Remarks:                                                                   #
#                                                                            #
# Copyright: Copyright (C) 2006, Sphairon Access Systems GmbH                #
#                                                                            #
# Author:    Frank Stebich                                                   #
# Created:   29.09.2006                                                      #
##############################################################################

.SUFFIXES:
.PHONY: generate

generate:
ifeq ($(SAS_SKIP_UNIT_TESTS),no)
	if [ -d ../test/make ]; then ${MAKE} -C ../test/make -f generate.mk; fi
endif
