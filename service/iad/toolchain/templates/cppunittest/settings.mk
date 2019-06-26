##############################################################################
# File:      settings.mk                                                     #
# Purpose:   Common settings for cppunit-test applications.                  #
# Remarks:                                                                   #
#                                                                            #
# Copyright: Copyright (C) 2006, Sphairon Access Systems GmbH                #
#                                                                            #
# Author:    Ullrich Meyer                                                   #
# Created:   29.11.2006                                                      #
##############################################################################

# Put functions and data into separate sections for section garbage collect
ifeq ($(SAS_GC_SECTIONS),y)
EXTRA		+= -fdata-sections -ffunction-sections
endif

# Enable stack usage statistics available since gcc-4.6.x
ifeq ($(SAS_SUPPORT_STACKUSAGE),y)
EXTRA		+= -fstack-usage
endif

TARGET		:= $(SAS_TARGET)

SAS_CPPUNIT_LOGFILENAME := test_results.cppunit.xml
EXTRA		+= -D'SAS_CPPUNIT_LOGFILENAME="$(SAS_CPPUNIT_LOGFILENAME)"'

# Add SDK directories to include search path
CFLAGS		+= $(SAS_PLATFORM-SDK_INCLUDE)
CFLAGS		+= -I$(SAS_PLATFORM-SDK_ROOT)/toolchain/cppunit/include
ifneq ($(SAS_SERVICE-SDK_ROOT),)
CFLAGS		+= $(SAS_SERVICE-SDK_INCLUDE)
endif

# Add include path of user specific module to include search path
CFLAGS		+= -I$(dir $(CURDIR))/../include

# Allow enable/disable of --as-needed via config option
ifeq ($(SAS_SUPPORT_LD_AS_NEEDED),y)
SAS_LIBS	:= $(SAS_LIBS:SAS_LD_AS_NEEDED_DISABLE=-Wl,--no-as-needed)
SAS_LIBS	:= $(SAS_LIBS:SAS_LD_AS_NEEDED_ENABLE=-Wl,--as-needed)
else
SAS_LIBS	:= $(SAS_LIBS:SAS_LD_AS_NEEDED_DISABLE=)
SAS_LIBS	:= $(SAS_LIBS:SAS_LD_AS_NEEDED_ENABLE=)
endif

# Add dependency handling to force relinking of binaries if a linked
# library has been updated
LIBS_SHARED	:= $(SAS_LIBS:-l%=lib%.so)
LIBS_STATIC	:= $(SAS_LIBS:-l%=lib%.a)
DEP_LIBS	+= $(wildcard $(LIBS_SHARED:%=$(SAS_PLATFORM-SDK_LIB_PATH)/%))
DEP_LIBS	+= $(wildcard $(LIBS_STATIC:%=$(SAS_PLATFORM-SDK_LIB_PATH)/%))
ifneq ($(SAS_SERVICE-SDK_ROOT),)
DEP_LIBS	+= $(wildcard $(LIBS_SHARED:%=$(SAS_SERVICE-SDK_LIB_PATH)/%))
DEP_LIBS	+= $(wildcard $(LIBS_STATIC:%=$(SAS_SERVICE-SDK_LIB_PATH)/%))
endif
DEP_LIBS	+= $(wildcard $(SAS_PLATFORM-SDK_ROOT)/toolchain/cppunit/lib/libcppunit*.so*)

# Allow function backtrace for host compiled applications
ifeq ($(SAS_TOOLCHAIN),$(SAS_HOST_TOOLCHAIN))
LDFLAGS		+= -rdynamic
endif

# Add SDK directories to library search path
LDFLAGS		+= -L$(SAS_PLATFORM-SDK_LIB_PATH)
SAS_LIB_PATH	+= $(SAS_PLATFORM-SDK_LIB_PATH)
ifneq ($(SAS_SERVICE-SDK_ROOT),)
LDFLAGS		+= -L$(SAS_SERVICE-SDK_LIB_PATH)
SAS_LIB_PATH	+= $(SAS_SERVICE-SDK_LIB_PATH)
endif
LDFLAGS		+= -L$(SAS_PLATFORM-SDK_ROOT)/toolchain/cppunit/lib
SAS_LIB_PATH	+= $(SAS_PLATFORM-SDK_ROOT)/toolchain/cppunit/lib

# Add library search path directly to ELF header for host applications
# For target applications only use library search path for linker.
ifeq ($(SAS_TOOLCHAIN),$(SAS_HOST_TOOLCHAIN))
RPATH_OPT	:= --rpath
else
RPATH_OPT	:= --rpath-link
endif

RPATH		:= $(shell echo $(SAS_LIB_PATH) | sed 's/ /:/g')
LDFLAGS		+= -Wl,$(RPATH_OPT) -Wl,$(RPATH)

# Garbage collect sections
ifeq ($(SAS_GC_SECTIONS),y)
ifneq ($(SAS_GC_SECTIONS_DISABLE),y)
LDFLAGS		+= -Wl,--gc-sections
endif
endif

# Pull in user specific linker flags
LDFLAGS		+= $(SAS_LDFLAGS)

# Wrap a predefinex symbols
comma = ,
LDFLAGS         += $(addprefix -Wl$(comma)-wrap$(comma),$(SAS_WRAP_SYMBOL))

# Only add libraries to ELF header that are referenced
ifeq ($(SAS_SUPPORT_LD_AS_NEEDED),y)
LIBS		+= -Wl,--as-needed
endif

# Pull in module under test objects
LIBS		+= $(SAS_OBJECTS)

# Pull in user specific libraries
LIBS		+= $(SAS_LIBS)

# Always link CppUnit library
LIBS		+= -lcppunit

# Disable --as-needed for remaining libraries
ifeq ($(SAS_SUPPORT_LD_AS_NEEDED),y)
LIBS		+= -Wl,--no-as-needed
endif

# Always link libdl
ifeq ($(SAS_LINKTYPE),dynamic)
LIBS		+= -ldl
endif

# Always link libgcov on host if coverage statistics are enabled by user
ifeq ($(SAS_TEST_COVERAGE),yes)
LIBS		+= -lgcov
endif
