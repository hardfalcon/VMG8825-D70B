##############################################################################
# File:      clean.mk                                                        #
# Purpose:   Implementation of buildstep clean                               #
# Remarks:                                                                   #
#                                                                            #
# Copyright: Copyright (C) 2011, Sphairon Technologies GmbH                  #
#                                                                            #
# Author:    Martin Volkmer                                                  #
# Created:   26.06.2013                                                      #
##############################################################################

include settings.mk

.PHONY: clean

clean:
	-rm -rf $(SDK_TC_TARGET)
