##############################################################################
# File:      settings.mk						     #
# Purpose:   Common settings for userspace libraries and applications.       #
# Remarks:      							     #
#       								     #
# Copyright: Copyright (C) 2006, Sphairon Access Systems GmbH   	     #
#       								     #
# Author:    Frank Stebich      					     #
# Created:   17.10.2006 						     #
##############################################################################

SAS_ROOT		:= $(dir $(CURDIR))
SAS_INCLUDE		+= -I$(SAS_ROOT)/include
SAS_SOURCE		?= $(SAS_ROOT)/source
SAS_BUILD		?= $(SAS_ROOT)/build
SAS_DOC_EXPORT		:= $(SAS_ROOT)/doc/extern
SAS_OPTIMIZE		?= $(SAS_OPTIONS_OPTIMIZE)
SAS_DEBUG		?= $(SAS_OPTIONS_DEBUG)
SAS_SOURCE_LANG		?= C
ifeq ($(SAS_SOURCE_LANG),C)
SAS_COMPILER		?= $(SAS_CROSS_GCC)
SAS_STANDARD		?= -std=gnu99
endif
ifeq ($(SAS_SOURCE_LANG),C++)
SAS_COMPILER		?= $(SAS_CROSS_GPP)
ifeq ($(SAS_SUPPORT_CPLUSPLUS_11),y)
SAS_STANDARD		?= -std=gnu++11
ifeq ($(SAS_SUPPORT_GLIBCXX_USE_C99),y)
SAS_DEFINE		+= -D_GLIBCXX_USE_C99=1
endif
endif
ifeq ($(SAS_SUPPORT_CPLUSPLUS_0X),y)
SAS_STANDARD		?= -std=gnu++0x
endif
SAS_STANDARD		?= -std=gnu++98
endif
SAS_LIBTYPE		?= static
SAS_PEDANTIC		?= no

SAS_COV			?= $(SAS_ROOT)/cov
SAS_MC			?= $(SAS_ROOT)/memcheck
SAS_TEST_COVERAGE	?= no
SAS_LINKTYPE		?= dynamic
SAS_VISIBILTY_HIDDEN	?= no

export SAS_COV
export SAS_TEST_COVERAGE

export SAS_MC

VPATH		:= $(SAS_SOURCE)

PLATFORM	:= $(SAS_CROSS_FLAGS)
OPTIMIZE	:= $(SAS_OPTIMIZE)
STANDARD	:= $(SAS_STANDARD)
DEBUG		:= $(SAS_DEBUG)
DEFINE		:= $(SAS_DEFINE)
DEFINE		+= $(SAS_OPTIONS_DEFINE)

WARNINGS	:= $(SAS_WARNINGS)
WARNINGS	+= -Wall -Wextra
WARNINGS	+= -Wno-long-long -Wno-unused-parameter
WARNINGS-$(SAS_SUPPORT_WNOERROR)	+= -Wno-error=unused-variable
WARNINGS-$(SAS_SUPPORT_WNOERROR)	+= -Wno-error=unused-function
WARNINGS-$(SAS_SUPPORT_WNOERROR)	+= -Wno-error=deprecated-declarations
WARNINGS	+= $(WARNINGS-y)

ifeq ($(SAS_PEDANTIC),yes)
EXTRA		+= -pedantic
#EXTRA		+= -pedantic-errors
endif

CFLAGS		+= $(DEBUG)
CFLAGS		+= $(OPTIMIZE)
CFLAGS		+= $(SAS_TOOLCHAIN_FLAGS)
CFLAGS		+= $(SAS_TOOLCHAIN_DEFINES)
CFLAGS		+= $(PLATFORM)
CFLAGS		+= $(FLAGS)
CFLAGS		+= $(WARNINGS)
CFLAGS		+= $(DEFINE)
CFLAGSV		:= $(CFLAGS)
CFLAGS		+= -include $(SAS_SUPPORT_HEADER)
CFLAGS		+= $(SAS_INCLUDE)

CXXFLAGS	+= $(CFLAGS)
CXXFLAGSV	+= $(CFLAGSV)

CLANG_FLAGS	+= $(WARNINGS)
CLANG_FLAGS	+= $(STANDARD)
CLANG_FLAGS	+= $(DEFINE)
CLANG_FLAGS	+= -include $(SAS_SUPPORT_HEADER)
CLANG_FLAGS	+= $(SAS_INCLUDE)
CLANG_FLAGS	+= -fdiagnostics-show-category=name

CPPCHECK_FLAGS	+= $(DEFINE)
CPPCHECK_FLAGS	+= -include $(SAS_SUPPORT_HEADER)
CPPCHECK_FLAGS	+= $(SAS_INCLUDE)
CPPCHECK_FLAGS	+= --xml --xml-version=2
CPPCHECK_FLAGS	+= --enable=warning --enable=style --enable=performance
CPPCHECK_FLAGS	+= --enable=portability

LDFLAGS		+= $(SAS_TOOLCHAIN_LDFLAGS)

DTEMP		:= $(SAS_FILES:.c=.d)
DFILES		+= $(DTEMP:.cpp=.d)
OTEMP		:= $(SAS_FILES:.c=.o)
OFILES		+= $(OTEMP:.cpp=.o)

COVFILES	:= *.cov *.gcov *.gcda *.gcno

MCFILES		:= memcheck.xml

CLANG_FILES	:= $(SAS_FILES:%=%.clang)
CPPCHECK_FILES	:= $(SAS_FILES:%=%.cppcheck.xml)
