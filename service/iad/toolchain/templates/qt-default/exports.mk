##############################################################################
# File:      qt-exports.mk                                                   #
# Purpose:   exports all symbol's for pro file.                              #
# Remarks:                                                                   #
#                                                                            #
# Copyright: Copyright (C) 2009, Sphairon Access Systems GmbH                #
#                                                                            #
# Author:    Mario Witkowski                                                 #
# Created:   07.07.2009                                                      #
##############################################################################

ifeq (${SAS_TEMPLATES_PATH},)
$(error "SAS_TEMPLATES_PATH required")
endif

ifeq (${SAS_PLATFORM-SDK_ROOT},)
$(error "SAS_PLATFORM-SDK_ROOT required")
endif

include ${SAS_TEMPLATES_PATH}/qt-default/../user/settings.mk

ifeq (${SAS_BUILD},)
$(error "SAS_BUILD required")
endif


ifeq ($(SAS_PEDANTIC),yes)
SAS_CXXFLAGS	+= -pedantic
SAS_CFLAGS	+= -pedantic
endif

SAS_CXXFLAGS	+= -std=gnu++98
SAS_CXXFLAGS	+= -mtune=4kc
SAS_CFLAGS	+= -std=gnu99
SAS_CFLAGS	+= -mtune=4kc

export SAS_QT_ROOT=$(SAS_ROOT)
export SAS_QT_BUILD=$(SAS_BUILD)
export SAS_QT_INSTALL=$(SAS_INSTALL)
export SAS_QT_TARGET=$(SAS_TARGET)
export SAS_QT_SOURCES=$(SAS_SOURCES)
export SAS_QT_HEADERS=$(SAS_HEADERS)
export SAS_QT_DEFINES=$(SAS_DEFINES)
export SAS_QT_CXXFLAGS=$(SAS_CXXFLAGS)
export SAS_QT_CFLAGS=$(SAS_CFLAGS)
export SAS_QT_MODULES=$(SAS_MODULES)
export SAS_QT_INCLUDE=$(SAS_INCLUDE) $(SAS_PLATFORM-SDK_INCLUDE_PATH) $(SAS_PLATFORM-SDK_INCLUDE_PATH)/sas
export SAS_QT_LIBS=$(SAS_LIBS) -L$(SAS_PLATFORM-SDK_LIB_PATH)
export SAS_QT_EXTRA_TARGET=$(SAS_EXTRA_TARGET)
export SAS_QT_PREDEPENDS=$(SAS_PREDEPENDS)
export SAS_QT_RPATHDIR=$(SAS_RPATHDIR) $(SAS_PLATFORM-SDK_LIB_PATH) $(SAS_QT_SDK_LIBPATH)


export SAS_QT_LIBTYPE=$(SAS_LIBTYPE)
export SAS_QT_VERSION=$(SAS_VERSION)
export SAS_QT_HEADERS_INSTALL=$(SAS_HEADERS_INSTALL)

export SAS_QT_OPTIONS_OPTIMIZE=$(SAS_OPTIONS_OPTIMIZE)
