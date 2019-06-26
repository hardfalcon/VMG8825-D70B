##############################################################################
# File:      doc.mk                                                          #
# Purpose:   Build step doc.                                                 #
# Remarks:                                                                   #
#                                                                            #
# Copyright: Copyright (C) 2006, Sphairon Access Systems GmbH                #
#                                                                            #
# Author:    Christian Schroeder                                             #
# Created:   12.07.2007                                                      #
##############################################################################


include settings.mk


.PHONY: ${SAS_SUBSYS}
.NOTPARALLEL:

doc:	${SAS_SUBSYS}

${SAS_SUBSYS}:
	@+$(MAKE) -C $@/make -f doc.mk SAS_CONFIG=${SAS_CONFIG} --no-print-directory SAS_DOC_PACKAGE=$(SAS_DOC_PACKAGE)
