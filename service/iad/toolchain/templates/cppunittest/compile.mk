##############################################################################
# File:      compile.mk                                                      #
# Purpose:   cppunit-test specific template for build step compile.          #
# Remarks:                                                                   #
#                                                                            #
# Copyright: Copyright (C) 2006, Sphairon Access Systems GmbH                #
#                                                                            #
# Author:    Ullrich Meyer                                                   #
# Created:   29.11.2006                                                      #
##############################################################################

.SUFFIXES:

# get the current SAS workspace root folder (i.e. the folder above us
# containing the directory 'iad')
SAS_WORKSPACE_ROOT ?= $(firstword $(subst /iad/, ,$(CURDIR)))

include $(SAS_WORKSPACE_ROOT)/paths.mk
include $(SAS_WORKSPACE_ROOT)/exports.mk
include settings.mk
SAS_SOURCE_LANG	?= C++
include $(SAS_TEMPLATES_PATH)/user/settings.mk
include $(SAS_TEMPLATES_PATH)/cppunittest/settings.mk
include $(SAS_TEMPLATES_PATH)/generic/verbose.mk

# Strip module root from filenames and use relative paths instead
PREFIXED_OFILES := $(patsubst %,$(SAS_BUILD)/%,$(OFILES))
PREFIXED_ODIRS	:= $(sort $(dir $(PREFIXED_OFILES)))
INSTALL_DIRS	:= $(sort $(abspath $(PREFIXED_ODIRS) $(SAS_BUILD)))

.PHONY: compile $(TARGET)

ifeq ($(SAS_CONFIG), skip)
compile:
	@echo ">>> Skipping buildstep compile <<<"
else

compile:	$(SAS_BUILD)/$(TARGET)

quiet_cmd_compile_cpp = CPP       $(@F)
cmd_compile_cpp = $(SAS_COMPILER) $(EXTRA) $(STANDARD) $(CXXFLAGS) \
	-DSAS_BASEFILE="\"$(<F)\"" -MMD -MP -MT $@ -MF $(@:%.o=%.d) -c -o $@ $<

quiet_cmd_link = LD        $(@F)
cmd_link = $(SAS_COMPILER) -o $@ $(PREFIXED_OFILES) -Wl,-Map -Wl,$@.map $(LDFLAGS) $(LIBS)

$(INSTALL_DIRS):
	$(Q)mkdir -p $@

$(PREFIXED_OFILES): | $(INSTALL_DIRS)

$(SAS_BUILD)/$(TARGET):	$(PREFIXED_OFILES) $(SAS_OBJECTS) $(DEP_LIBS)
	$(call cmd,link)
	$(Q)$(SAS_CROSS_NM) -C $@ > $@.sym
	$(Q)$(SAS_CROSS_READELF) -d $@ > $@.dyn

$(SAS_BUILD)/%.o: %.cpp settings.mk compile.mk $(SAS_TEMPLATES_PATH)/cppunittest/settings.mk $(SAS_TEMPLATES_PATH)/user/settings.mk
	$(call cmd,compile_cpp)

include $(wildcard $(PREFIXED_OFILES:%.o=%.d))

endif
