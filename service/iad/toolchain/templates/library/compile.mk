##############################################################################
# File:      compile.mk                                                      #
# Purpose:   Library-specific template for build step compile.               #
# Remarks:                                                                   #
#                                                                            #
# Copyright: Copyright (C) 2006, Sphairon Access Systems GmbH                #
#                                                                            #
# Author:    Frank Stebich                                                   #
# Created:   17.10.2006                                                      #
##############################################################################

.SUFFIXES:

# get the current SAS workspace root folder (i.e. the folder above us
# containing the directory 'iad')
SAS_WORKSPACE_ROOT ?= $(firstword $(subst /iad/, ,$(CURDIR)))

include $(SAS_WORKSPACE_ROOT)/paths.mk
include $(SAS_WORKSPACE_ROOT)/exports.mk
include settings.mk
include $(SAS_TEMPLATES_PATH)/user/settings.mk
include $(SAS_TEMPLATES_PATH)/library/settings.mk
include $(SAS_TEMPLATES_PATH)/generic/verbose.mk

# Strip module root from filenames and use relative paths instead
SAS_BUILDDEPS	?= settings.mk compile.mk
PREFIXED_OFILES := $(patsubst %,$(SAS_BUILD)/%,$(OFILES))
PREFIXED_ODIRS	:= $(sort $(dir $(PREFIXED_OFILES)))

PREFIXED_CLANG_FILES := $(addprefix $(SAS_BUILD)/,$(CLANG_FILES))
PREFIXED_CLANG_DIRS := $(sort $(dir $(PREFIXED_CLANG_FILES)))

PREFIXED_CPPCHECK_FILES := $(addprefix $(SAS_BUILD)/,$(CPPCHECK_FILES))
PREFIXED_CPPCHECK_DIRS := $(sort $(dir $(PREFIXED_CPPCHECK_FILES)))

INSTALL_DIRS	:= $(sort $(abspath $(PREFIXED_ODIRS) $(PREFIXED_CLANG_DIRS) $(PREFIXED_CPPCHECK_DIRS) $(SAS_BUILD)))

.PHONY: compile

ifeq ($(SAS_CONFIG), skip)
compile:
	@echo "Skipping buildstep compile <<<"
else

COMPILE_TARGETS := $(SAS_BUILD)/$(TARGET)
ifeq ($(SAS_LIBTYPE),static)
COMPILE_TARGETS += $(SAS_ACTIVE-SDK_LIB_PATH)/$(TARGET)
else ifeq ($(SAS_LIBTYPE),shared)
COMPILE_TARGETS += $(SAS_ACTIVE-SDK_LIB_PATH)/$(TARGET)
ifneq ($(SAS_TARGET_LINK),)
COMPILE_TARGETS += $(SAS_ACTIVE-SDK_LIB_PATH)/$(TARGET_LINK)
endif
else ifeq ($(SAS_LIBTYPE),shared-soname)
COMPILE_TARGETS += $(SAS_ACTIVE-SDK_LIB_PATH)/$(TARGET_SONAME)
COMPILE_TARGETS += $(SAS_ACTIVE-SDK_LIB_PATH)/$(TARGET_VERSIONED)
COMPILE_TARGETS += $(SAS_ACTIVE-SDK_LIB_PATH)/$(TARGET_UNVERSIONED)
ifneq ($(SAS_TARGET_LINK),)
COMPILE_TARGETS += $(SAS_ACTIVE-SDK_LIB_PATH)/$(TARGET_LINK)
COMPILE_TARGETS += $(SAS_ACTIVE-SDK_LIB_PATH)/$(TARGET_SONAME_LINK)
endif
endif
ifeq ($(SAS_CLANG_ANALYZE),yes)
COMPILE_TARGETS += $(PREFIXED_CLANG_FILES)
endif
ifeq ($(SAS_CPPCHECK_ANALYZE),yes)
COMPILE_TARGETS += $(PREFIXED_CPPCHECK_FILES)
endif

compile: $(sort $(COMPILE_TARGETS))
ifeq ($(SAS_SKIP_UNIT_TESTS),no)
	@if [ -d ../test/make ]; then $(MAKE) -C ../test/make -f compile.mk; fi
endif
endif

quiet_cmd_compile_c = CC        $(@F)
cmd_compile_c = $(SAS_COMPILER) $(EXTRA) $(STANDARD) $(CFLAGS) \
	-DSAS_BASEFILE="\"$(<F)\"" -MMD -MP -MT $@ -MF $(@:%.o=%.d) -c -o $@ $<

quiet_cmd_compile_cpp = CPP       $(@F)
cmd_compile_cpp = $(SAS_COMPILER) $(EXTRA) $(STANDARD) $(CXXFLAGS) \
	-DSAS_BASEFILE="\"$(<F)\"" -MMD -MP -MT $@ -MF $(@:%.o=%.d) -c -o $@ $<

quiet_cmd_clang = CLANG     $(<F)
cmd_clang = clang --analyze $(CLANG_FLAGS) \
	-DSAS_BASEFILE="\"$(<F)\"" $< ;\
	touch $@

quiet_cmd_cppcheck = CPPCHECK  $(<F)
cmd_cppcheck = cppcheck $(CPPCHECK_FLAGS) \
	-DSAS_BASEFILE="\"$(<F)\"" $< 2> $@;

quiet_cmd_ar = AR        $(@F)
cmd_ar = $(SAS_CROSS_AR) cr $@  $^

quiet_cmd_link = LD        $(@F)
cmd_link = $(SAS_COMPILER) -shared -o $@ $(PREFIXED_OFILES) -Wl,-Map -Wl,$@.map $(LDFLAGS) $(LIBS)

# Create build and install directories
$(INSTALL_DIRS) $(SAS_ACTIVE-SDK_LIB_PATH):
	$(Q)mkdir -p $@

# Object files depend on build directories
$(PREFIXED_OFILES): | $(INSTALL_DIRS)
$(PREFIXED_CLANG_FILES): | $(INSTALL_DIRS)
$(PREFIXED_CPPCHECK_FILES): | $(INSTALL_DIRS)

# Implicit rules for object files
$(SAS_BUILD)/%.o: %.c $(SAS_TEMPLATES_PATH)/library/settings.mk $(SAS_TEMPLATES_PATH)/user/settings.mk $(SAS_BUILDDEPS)
	$(call cmd,compile_c)

$(SAS_BUILD)/%.o: %.cpp $(SAS_TEMPLATES_PATH)/library/settings.mk $(SAS_TEMPLATES_PATH)/user/settings.mk $(SAS_BUILDDEPS)
	$(call cmd,compile_cpp)

include $(wildcard $(PREFIXED_OFILES:%.o=%.d))

$(SAS_BUILD)/%.c.clang: %.c $(SAS_TEMPLATES_PATH)/library/settings.mk $(SAS_TEMPLATES_PATH)/user/settings.mk $(SAS_BUILDDEPS)
	$(call cmd,clang)

$(SAS_BUILD)/%.cpp.clang: %.cpp $(SAS_TEMPLATES_PATH)/library/settings.mk $(SAS_TEMPLATES_PATH)/user/settings.mk $(SAS_BUILDDEPS)
	$(call cmd,clang)

$(SAS_BUILD)/%.c.cppcheck.xml: %.c $(SAS_TEMPLATES_PATH)/library/settings.mk $(SAS_TEMPLATES_PATH)/user/settings.mk $(SAS_BUILDDEPS)
	$(call cmd,cppcheck)

$(SAS_BUILD)/%.cpp.cppcheck.xml: %.cpp $(SAS_TEMPLATES_PATH)/library/settings.mk $(SAS_TEMPLATES_PATH)/user/settings.mk $(SAS_BUILDDEPS)
	$(call cmd,cppcheck)

# Target rule for static libraries
$(SAS_BUILD)/lib$(SAS_TARGET).a: $(PREFIXED_OFILES)
	$(call cmd,ar)

# Target rule for shared libraries
$(SAS_BUILD)/lib$(SAS_TARGET).so: $(PREFIXED_OFILES) $(DEP_LIBS)
	$(call cmd,link)
	$(Q)$(SAS_CROSS_NM) -CD $@ > $@.sym
	$(Q)$(SAS_CROSS_READELF) -d $@ > $@.dyn

ifeq ($(SAS_LIBTYPE),shared-soname)
# Rule for installing a symlink without any version info
$(SAS_ACTIVE-SDK_LIB_PATH)/$(TARGET_UNVERSIONED): $(SAS_ACTIVE-SDK_LIB_PATH)/$(TARGET_SONAME)
	$(Q)rm -f $@
	$(Q)cd $(@D) && ln -s $(<F) $(@F)
else
# Rule for native installing of target binary
$(SAS_ACTIVE-SDK_LIB_PATH)/$(TARGET): $(SAS_BUILD)/$(TARGET) | $(SAS_ACTIVE-SDK_LIB_PATH)
	$(Q)rm -f $(SAS_ACTIVE-SDK_LIB_PATH)/$(TARGET)*
	$(Q)cp -f $< $@
endif

# Rule for installing a symlink with alternative name to target binary
ifneq ($(SAS_TARGET_LINK),)
$(SAS_ACTIVE-SDK_LIB_PATH)/$(TARGET_LINK): $(SAS_ACTIVE-SDK_LIB_PATH)/$(TARGET)
	$(Q)rm -f $@
	$(Q)cd $(@D) && ln -s $(<F) $(@F)
endif

# Rule for installing the binary target by extending its name with version info
ifneq ($(TARGET_VERSIONED),)
$(SAS_ACTIVE-SDK_LIB_PATH)/$(TARGET_VERSIONED): $(SAS_BUILD)/$(TARGET) | $(SAS_ACTIVE-SDK_LIB_PATH)
	$(Q)rm -f $(SAS_ACTIVE-SDK_LIB_PATH)/$(TARGET)*
	$(Q)cp -f $< $@
endif

# Rule for installing a symlink with only major version number
ifneq ($(TARGET_SONAME),)
$(SAS_ACTIVE-SDK_LIB_PATH)/$(TARGET_SONAME): $(SAS_ACTIVE-SDK_LIB_PATH)/$(TARGET_VERSIONED)
	$(Q)rm -f $@
	$(Q)cd $(@D) && ln -s $(<F) $(@F)
endif

ifneq ($(TARGET_SONAME_LINK),)
# Rule for installing a symlink with alternative name to versioned target binary
$(SAS_ACTIVE-SDK_LIB_PATH)/$(TARGET_SONAME_LINK): $(SAS_ACTIVE-SDK_LIB_PATH)/$(TARGET_SONAME)
	$(Q)rm -f $@
	$(Q)cd $(@D) && ln -s $(<F) $(@F)
endif
