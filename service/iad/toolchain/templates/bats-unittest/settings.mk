##############################################################################
# File:      settings.mk                                                     #
# Purpose:   BATS unit-test settings                                         #
#                                                                            #
# Copyright: Copyright (C) 2018, Sphairon GmbH (a ZyXEL company)             #
##############################################################################

SAS_ROOT	:= $(dir $(CURDIR))
SAS_SOURCE	?= $(SAS_ROOT)/source
SAS_BUILD	?= $(SAS_ROOT)/build

BATS_TOOL	:= $(shell which bats)
BATS_RESULTS	:= test_results.tap

BATS_DIRS	:= $(SAS_SOURCE)
BATS_DIRS	+= $(addprefix $(SAS_SOURCE)/,$(BATS_TEST_DIRS))
BATS_TESTS	:= $(foreach dir,$(abspath $(BATS_DIRS)),$(wildcard $(dir)/*.bats))
