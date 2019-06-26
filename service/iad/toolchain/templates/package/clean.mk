##############################################################################
# File:      clean.mk                                                        #
# Purpose:   Build step clean.                                               #
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


clean:	${SAS_SUBSYS}

${SAS_SUBSYS}:
	@+$(MAKE) -C $@/make -f clean.mk SAS_CONFIG=${SAS_CONFIG} --no-print-directory
