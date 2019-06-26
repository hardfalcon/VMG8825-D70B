##############################################################################
# File:      git-source.mk                                                   #
# Purpose:   Git source helper functions                                     #
#                                                                            #
# Copyright: Copyright (C) 2011, Sphairon Technologies GmbH                  #
#                                                                            #
# Author:    Daniel Schwierzeck                                              #
# Created:   31.08.2011                                                      #
##############################################################################

ifndef SAS_TEMPLATES_INCLUDE_GIT_SOURCE
SAS_TEMPLATES_INCLUDE_GIT_SOURCE := 1

# $(call SAS_UseGit,<module-root>)
define SAS_UseGit
$(if $(filter y,$(USE_GIT)),y,$(shell [ -e $1/use_git ] && echo y || echo n))
endef

endif # SAS_TEMPLATES_INCLUDE_GIT_SOURCE
