##############################################################################
# File:      settings.mk                                                     #
# Purpose:   Common settings for userspace libraries.                        #
# Remarks:                                                                   #
#                                                                            #
# Copyright: Copyright (C) 2006, Sphairon Access Systems GmbH                #
#                                                                            #
# Author:    Frank Stebich                                                   #
# Created:   17.10.2006                                                      #
##############################################################################

# Put functions and data into separate sections for section garbage collect
ifeq ($(SAS_GC_SECTIONS),y)
EXTRA		+= -fdata-sections -ffunction-sections
endif

# Mark all symbols as hidden if enabled by user
ifeq ($(SAS_SUPPORT_VISIBILITY),y)
ifeq ($(SAS_VISIBILTY_HIDDEN),yes)
EXTRA		+= -fvisibility=hidden
endif
endif

# Enable stack usage statistics available since gcc-4.6.x
ifeq ($(SAS_SUPPORT_STACKUSAGE),y)
EXTRA		+= -fstack-usage
endif

ifeq ($(SAS_LIBTYPE),shared)

TARGET		:= lib$(SAS_TARGET).so
EXTRA		+= -fpic
CFLAGS		+= -DSAS_DSO_COMPILE
LDFLAGS		+= -fpic

ifneq ($(SAS_TARGET_LINK),)
TARGET_LINK	:= lib$(SAS_TARGET_LINK).so
endif

else ifeq ($(SAS_LIBTYPE),static)

TARGET		:= lib$(SAS_TARGET).a
EXTRA		+= -fpic

else ifeq ($(SAS_LIBTYPE),shared-soname)

ifeq ($(SAS_LIB_MAJOR),)
$(error "library Major number is not given but needed!")
endif
ifeq ($(SAS_LIB_MINOR),)
$(error "library Minor number is not given but needed!")
endif
ifeq ($(SAS_LIB_PATCH),)
$(error "library Patch-Level is not given but needed!")
endif

TARGET			:= lib$(SAS_TARGET).so
EXTRA			+= -fpic
CFLAGS			+= -DSAS_DSO_COMPILE
LDFLAGS			+= -fpic
LDFLAGS			+= -Wl,-soname,$(TARGET_SONAME)

CLANG_FLAGS		+= -DSAS_DSO_COMPILE

ifneq ($(SAS_TARGET_LINK),)
TARGET_LINK		:= lib$(SAS_TARGET_LINK).so
TARGET_SONAME_LINK	:= lib$(SAS_TARGET_LINK).so.$(SAS_LIB_MAJOR)
endif
TARGET_UNVERSIONED	:= lib$(SAS_TARGET).so
TARGET_VERSIONED	:= lib$(SAS_TARGET).so.$(SAS_LIB_MAJOR).$(SAS_LIB_MINOR).$(SAS_LIB_PATCH)
TARGET_SONAME		:= lib$(SAS_TARGET).so.$(SAS_LIB_MAJOR)

else

$(error "library type required: shared | shared-soname | static")

endif

ifneq ($(SAS_LIBTYPE),static)

# Add dependency handling to force relinking of binaries if a linked
# library has been updated
ifeq ($(SAS_SUPPORT_LD_AS_NEEDED),y)
SAS_LIBS	:= $(SAS_LIBS:SAS_LD_AS_NEEDED_DISABLE=-Wl,--no-as-needed)
SAS_LIBS	:= $(SAS_LIBS:SAS_LD_AS_NEEDED_ENABLE=-Wl,--as-needed)
else
SAS_LIBS	:= $(SAS_LIBS:SAS_LD_AS_NEEDED_DISABLE=)
SAS_LIBS	:= $(SAS_LIBS:SAS_LD_AS_NEEDED_ENABLE=)
endif

LIBS_SHARED	:= $(SAS_LIBS:-l%=lib%.so)
LIBS_STATIC	:= $(SAS_LIBS:-l%=lib%.a)

DEP_LIBS	+= $(wildcard $(LIBS_SHARED:%=$(SAS_PLATFORM-SDK_LIB_PATH)/%))
DEP_LIBS	+= $(wildcard $(LIBS_STATIC:%=$(SAS_PLATFORM-SDK_LIB_PATH)/%))
ifneq ($(SAS_SERVICE-SDK_ROOT),)
DEP_LIBS	+= $(wildcard $(LIBS_SHARED:%=$(SAS_SERVICE-SDK_LIB_PATH)/%))
DEP_LIBS	+= $(wildcard $(LIBS_STATIC:%=$(SAS_SERVICE-SDK_LIB_PATH)/%))
endif

# Add SDK directories to library search path
LDFLAGS		+= -L$(SAS_PLATFORM-SDK_LIB_PATH)
SAS_LIB_PATH	+= $(SAS_PLATFORM-SDK_LIB_PATH)
ifneq ($(SAS_SERVICE-SDK_ROOT),)
LDFLAGS		+= -L$(SAS_SERVICE-SDK_LIB_PATH)
SAS_LIB_PATH	+= $(SAS_SERVICE-SDK_LIB_PATH)
endif

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

# Always link libdl
LIBS		+= -ldl

# Always link libgcov on host if coverage statistics are enabled by user
ifeq ($(SAS_TOOLCHAIN),$(SAS_HOST_TOOLCHAIN))
ifeq ($(SAS_TEST_COVERAGE),yes)
LIBS		+= -lgcov
endif
endif

else	# ($(SAS_LIBTYPE),static)

ifneq ($(SAS_LIBS),)
$(error "setting of SAS_LIBS makes no sense for static libraries")
endif

endif	# ($(SAS_LIBTYPE),static)

ifeq ($(SAS_TOOLCHAIN),$(SAS_HOST_TOOLCHAIN))
 ifeq ($(SAS_TEST_COVERAGE),yes)
  # library shall be prepared for usage in unit testing and test coverage analysis
  EXTRA += -fprofile-arcs -ftest-coverage
 endif
endif

CFLAGS += $(SAS_PLATFORM-SDK_INCLUDE)
CLANG_FLAGS += $(SAS_PLATFORM-SDK_INCLUDE)
CPPCHECK_FLAGS += $(SAS_PLATFORM-SDK_INCLUDE)
ifneq ($(SAS_SERVICE-SDK_ROOT),)
 CFLAGS += $(SAS_SERVICE-SDK_INCLUDE)
 CLANG_FLAGS += $(SAS_SERVICE-SDK_INCLUDE)
 CPPCHECK_FLAGS += $(SAS_SERVICE-SDK_INCLUDE)
endif

SAS_INSTALL            ?= $(SAS_ROOTFS_LIB_INSTALL_PATH)
SAS_EXPORT_HEADER_PATH ?= $(dir $(CURDIR))include
