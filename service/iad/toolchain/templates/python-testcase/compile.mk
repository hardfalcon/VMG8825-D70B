##############################################################################
# File:      compile.mk                                                      #
# Purpose:   Implementation of buildstep compile.                            #
# Remarks:                                                                   #
#                                                                            #
# Copyright: Copyright (C) 2013, Sphairon GmbH (a ZyXEL company)             #
#                                                                            #
# Author:    Martin Volkmer                                                  #
# Created:   26.06.2013                                                      #
##############################################################################

include settings.mk

.PHONY: compile pylint

pylint:
	(export PYTHONPATH=$(FRAMEWORK_SRC_PATH) && cd $(SDK_QA_PATH) && ./run_pylint_for_errors.sh $(TC_SRC_PATH) 1)

compile:  pylint
	
	