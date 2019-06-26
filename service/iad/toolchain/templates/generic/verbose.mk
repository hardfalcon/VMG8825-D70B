##############################################################################
# File:      verbose.mk                                                      #
# Purpose:   Specific functions and rules for build-system messages.         #
#                                                                            #
# Copyright: Copyright (C) 2010, Sphairon Technologies GmbH                  #
#                                                                            #
# Author:    Daniel Schwierzeck                                              #
# Created:   01.12.2010                                                      #
##############################################################################

ifndef SAS_TEMPLATES_INCLUDE_VERBOSE
SAS_TEMPLATES_INCLUDE_VERBOSE := 1

ifndef SAS_TEMPLATES_VERBOSE
SAS_TEMPLATES_VERBOSE := 0
endif
ifeq ("$(origin V)", "command line")
SAS_TEMPLATES_VERBOSE := $(V)
endif

redirect := >

ifeq ($(SAS_TEMPLATES_VERBOSE),1)
quiet :=
Q :=
MAKE_SILENT :=
STDOUT_SILENT :=
else
quiet := quiet_
Q := @
MAKE_SILENT := -s
STDOUT_SILENT := $(redirect) /dev/null
endif

ifneq ($(findstring s,$(MAKEFLAGS)),)
quiet := silent_
endif

comma   := ,
squote  := '
empty   :=
space   := $(empty) $(empty)

escsq = $(subst $(squote),'\$(squote)',$1)

echo-cmd = $(if $($(quiet)cmd_$(1)),echo '  $(call escsq,$($(quiet)cmd_$(1)))$(echo-why)';)

cmd = @$(echo-cmd) $(cmd_$(1))

endif
