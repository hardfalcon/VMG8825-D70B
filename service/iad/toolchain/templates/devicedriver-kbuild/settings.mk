##############################################################################
# File:      settings.mk                                                     #
# Purpose:   devicedriver-kbuild implementation generic settings             #
# Remarks:                                                                   #
#                                                                            #
# Copyright: Copyright (C) 2010, Sphairon Technologies GmbH                  #
#                                                                            #
# Author:    Maik Dybek                                                      #
# Created:   28.04.2010                                                      #
##############################################################################

SAS_ROOT		:= $(dir $(CURDIR))
SAS_INCLUDE		+= -I$(SAS_ROOT)include
SAS_SOURCE		?= $(SAS_ROOT)source
SAS_BUILD		?= $(SAS_ROOT)build
SAS_COMPLEX		?= no
SAS_VERBOSE		?= n
SAS_DEBUG		?= n

SAS_CROSS_COMPILE	:= $(SAS_TOOLCHAIN_KERNEL_PREFIX)

WARNINGS		:= $(SAS_WARNINGS)
WARNINGS		+= -Wall
WARNINGS		+= -Wno-unused-parameter

INCLUDE			+= $(SAS_INCLUDE)

DEFINE-y		:= $(SAS_DEFINE)
DEFINE-$(SAS_DEBUG)	+= -DDEBUG

CFLAGS			+= $(FLAGS)
CFLAGS			+= $(WARNINGS)
CFLAGS			+= $(DEFINE-y)
CFLAGS			+= $(INCLUDE)

KBUILD_TARGET 		:= $(SAS_TARGET).o
KBUILD_OBJECTS 		:= $(strip $(filter-out $(KBUILD_TARGET),$(SAS_FILES:.c=.o)))
KBUILD_FILE 		:= $(SAS_BUILD)/Kbuild

KBUILD_ENV-$(SAS_DEBUG)	+= CONFIG_DEBUG_SECTION_MISMATCH=y

ifeq ($(SAS_VERBOSE),y)
KBUILD_VERBOSE		:= 1
else
KBUILD_VERBOSE		:= $(SAS_TEMPLATES_VERBOSE)
endif

KBUILD_MOD_DIR		:= extra/$(SAS_MOD_DIR)
