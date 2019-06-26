##############################################################################
# File:      configure.mk                                                    #
# Purpose:   Build step configure.                                           #
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

configure:	${SAS_SUBSYS}

${SAS_SUBSYS}:
	@+$(MAKE) -C $@/make -f configure.mk SAS_CONFIG=${SAS_CONFIG} --no-print-directory
