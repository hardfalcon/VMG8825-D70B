##############################################################################
# File:      settings.mk                                                     #
# Purpose:   Common settings for userspace applications.                     #
# Remarks:                                                                   #
#                                                                            #
# Copyright: Copyright (C) 2006, Sphairon Access Systems GmbH                #
#                                                                            #
# Author:    Frank Stebich                                                   #
# Created:   08.11.2006                                                      #
##############################################################################

# Put functions and data into separate sections for section garbage collect
ifeq ($(SAS_GC_SECTIONS),y)
EXTRA		+= -fdata-sections -ffunction-sections
endif

# Enable stack usage statistics available since gcc-4.6.x
ifeq ($(SAS_SUPPORT_STACKUSAGE),y)
EXTRA		+= -fstack-usage
endif

ifeq ($(SAS_POSITION_INDEPENDENT_EXECUTABLE),y)
EXTRA		+= -fpie
LDFLAGS		+= -pie
endif

TARGET		:= $(SAS_TARGET)

MULTIBIN_TARGETS:= $(addprefix $(SAS_INSTALL)/,$(SAS_MULTI_BIN_LINK))

# Add SDK directories to include search path
CFLAGS		+= $(SAS_PLATFORM-SDK_INCLUDE)
CLANG_FLAGS	+= $(SAS_PLATFORM-SDK_INCLUDE)
ifneq ($(SAS_SERVICE-SDK_ROOT),)
CFLAGS		+= $(SAS_SERVICE-SDK_INCLUDE)
CLANG_FLAGS	+= $(SAS_SERVICE-SDK_INCLUDE)
endif

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

# Only add libraries to ELF header that are referenced
ifeq ($(SAS_SUPPORT_LD_AS_NEEDED),y)
LIBS		+= -Wl,--as-needed
endif

# Pull in user specific libraries
LIBS		+= $(SAS_LIBS)

# Disable --as-needed for remaining libraries
ifeq ($(SAS_SUPPORT_LD_AS_NEEDED),y)
LIBS		+= -Wl,--no-as-needed
endif

# Always link libdl
ifeq ($(SAS_LINKTYPE),dynamic)
LIBS		+= -ldl
endif

# Always link libgcov on host if coverage statistics are enabled by user
ifeq ($(SAS_TOOLCHAIN),$(SAS_HOST_TOOLCHAIN))
ifeq ($(SAS_TEST_COVERAGE),yes)
LIBS		+= -lgcov
endif
endif

# Force static linking if required by user
ifeq ($(SAS_LINKTYPE),static)
LDFLAGS		+= -static
endif


ifeq ($(SAS_TOOLCHAIN),$(SAS_HOST_TOOLCHAIN))
ifeq ($(SAS_TEST_COVERAGE),yes)
# Always link libgcov on host if coverage statistics are enabled by user
LIBS		+= -lgcov

# library shall be prepared for usage in unit testing and test coverage analysis
EXTRA		+= -fprofile-arcs -ftest-coverage
endif
endif
