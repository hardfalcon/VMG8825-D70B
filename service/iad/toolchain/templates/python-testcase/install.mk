##############################################################################
# File:      install.mk          	                                     #
# Purpose:   Default implementation of build step install	             #
# Remarks:                                                                   #
#                                                                            #
# Copyright: Copyright (C) 2013, Sphairon Access Systems GmbH                #
#                                                                            #
# Author:    Martin Volkmer	                                             #
# Created:   04.04.2014                                                      #
##############################################################################

include settings.mk

.PHONY: install

install:
	mkdir -p $(SDK_TC_TARGET)
	#copy build source to test-sdk, excluding pyc files 
	(rsync -a --include=*.py --include=*.xml --include=system/*.sh --include=*/ --exclude=* --prune-empty-dirs $(TC_SRC_PATH)/ $(SDK_TC_TARGET))