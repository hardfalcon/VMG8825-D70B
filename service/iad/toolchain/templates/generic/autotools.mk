ifndef SAS_TEMPLATES_INCLUDE_AUTOTOOLS
SAS_TEMPLATES_INCLUDE_AUTOTOOLS := 1

include $(SAS_WORKSPACE_ROOT)/paths.mk
include $(SAS_WORKSPACE_ROOT)/exports.mk
include $(SAS_TEMPLATES_PATH)/generic/verbose.mk

ifeq ($(SAS_TEMPLATES_VERBOSE),1)
CONFIGURE_SILENT =
else
CONFIGURE_SILENT = --quiet
endif

SAS_LIBTOOLIZE	?= libtoolize

ifeq ($(SAS_GC_SECTIONS),y)
SAS_CFLAGS += -fdata-sections -ffunction-sections

ifneq ($(SAS_GC_SECTIONS_DISABLE),y)
SAS_LDFLAGS += -Wl,--gc-sections
endif
endif

define SAS_Autotools_Check_Prereq
	$(if $(SAS_BUILD),,@echo "error: SAS_BUILD not specified";exit 1)
	$(if $(SAS_SOURCE),,@echo "error: SAS_SOURCE not specified";exit 1)
endef

define SAS_Autotools_Configure_Tools
	CC=$(SAS_CROSS_GCC) \
	CXX=$(SAS_CROSS_GPP) \
	CPP=$(SAS_CROSS_CPP) \
	LD=$(SAS_CROSS_LD) \
	LDD=$(SAS_CROSS_LDD) \
	AR=$(SAS_CROSS_AR) \
	NM=$(SAS_CROSS_NM) \
	RANLIB=$(SAS_CROSS_RANLIB) \
	STRIP=$(SAS_CROSS_STRIP) \
	OBJDUMP=$(SAS_CROSS_OBJDUMP)
endef

define SAS_Autotools_Configure_SanitizeCache
	$(Q)touch $1
	$(Q)sed -i -e "s,^ac_cv_env_CFLAGS\(.*\),,g" \
		-e "s,^ac_cv_env_CXXFLAGS\(.*\),,g" \
		-e "s,^ac_cv_env_CPPFLAGS\(.*\),,g" \
		-e "s,^ac_cv_env_LDFLAGS\(.*\),,g" \
		-e "s,^ac_cv_env_LIBS\(.*\),,g" \
		-e "s,^ac_cv_lib_crypto_CRYPT\(.*\),,g" \
		-e "s,^ac_cv_env_\(.*\)_CFLAGS_\(.*\),,g"  \
		-e "s,^ac_cv_env_\(.*\)_LIBS_\(.*\),,g" \
		-e "s,^ac_cv_env_PKG_CONFIG\(.*\),,g" \
		$1
endef

# $(call SAS_Autotools_Configure_SetFlags,[cflags],[cxxflags],[ldflags])
define SAS_Autotools_Configure_SetFlags
CFLAGS=$(strip $1) CXXFLAGS=$(strip $2) LDFLAGS=$(strip $3)
endef

define SAS_Autotools_Configure_SetFlagsWithCpp
CFLAGS=$(strip $1) CXXFLAGS=$(strip $2) LDFLAGS=$(strip $3) CPPFLAGS=$(strip $4)
endef

define SAS_Autotools_Configure_SetHostUserFlags
$(call SAS_Autotools_Configure_SetFlags,\
	"$(strip $(SAS_CFLAGS))",\
	"$(strip $(SAS_CXXFLAGS))",\
	"$(strip $(SAS_LDFLAGS))")
endef

define SAS_Autotools_Configure_SetTargetUserNoSdkFlags
$(call SAS_Autotools_Configure_SetFlags,\
	"$(strip $(SAS_TOOLCHAIN_FLAGS) $(SAS_OPTIONS_OPTIMIZE) $(SAS_CFLAGS))",\
	"$(strip $(SAS_CXXFLAGS))",\
	"$(strip $(SAS_TOOLCHAIN_LDFLAGS) $(SAS_LDFLAGS))")
endef

define SAS_Autotools_Configure_SetTargetUserFlags
$(call SAS_Autotools_Configure_SetFlags,\
	"$(strip $(SAS_TOOLCHAIN_FLAGS) $(SAS_OPTIONS_OPTIMIZE) $(SAS_CFLAGS) $(SAS_PLATFORM-SDK_INCLUDE) $(SAS_SERVICE-SDK_INCLUDE))",\
	"$(strip $(SAS_CXXFLAGS))",\
	"$(strip $(SAS_TOOLCHAIN_LDFLAGS) $(SAS_LDFLAGS) $(if $(SAS_PLATFORM-SDK_LIB_PATH),-L$(SAS_PLATFORM-SDK_LIB_PATH),) $(if $(SAS_SERVICE-SDK_LIB_PATH),-L$(SAS_SERVICE-SDK_LIB_PATH),))")
endef

define SAS_Autotools_Configure_SetTargetUserWithCppFlags
$(call SAS_Autotools_Configure_SetFlagsWithCpp,\
	"$(strip $(SAS_TOOLCHAIN_FLAGS) $(SAS_OPTIONS_OPTIMIZE) $(SAS_CFLAGS))",\
	"$(strip $(SAS_CXXFLAGS))",\
	"$(strip $(SAS_TOOLCHAIN_LDFLAGS) $(SAS_LDFLAGS) $(if $(SAS_PLATFORM-SDK_LIB_PATH),-L$(SAS_PLATFORM-SDK_LIB_PATH),) $(if $(SAS_SERVICE-SDK_LIB_PATH),-L$(SAS_SERVICE-SDK_LIB_PATH),))",\
	"$(strip $(SAS_PLATFORM-SDK_INCLUDE) $(SAS_SERVICE-SDK_INCLUDE))")
endef

define SAS_Autotools_Configure_SetDriverUserFlags
$(call SAS_Autotools_Configure_SetFlags,\
	"$(strip $(SAS_CFLAGS))",\
	"$(strip $(SAS_CXXFLAGS))")
endef

define SAS_Autotools_Configure_SetTargetTools
--build=$(SAS_HOST_TOOLCHAIN) \
--host=$(SAS_TOOLCHAIN) \
--target=$(SAS_TOOLCHAIN)
endef

define SAS_Autotools_Libtoolize_Raw
	@echo ">>> Libtoolize: $1"
	$(Q)cd $(abspath $1) && $(SAS_LIBTOOLIZE) --copy --force
endef

define SAS_Autotools_Aclocal_Raw
	@echo ">>> Aclocal: $1"
	$(Q)cd $(abspath $1) && aclocal
endef

define SAS_Autotools_Autoheader_Raw
	@echo ">>> Autoheader: $1"
	$(Q)cd $(abspath $1) && autoheader
endef

define SAS_Autotools_Autoconf_Raw
	@echo ">>> Autoconf: $1"
	$(Q)cd $(abspath $1) && autoconf
endef

define SAS_Autotools_Autoreconf_Raw
	@echo ">>> Autoreconf: $1"
	$(Q)cd $(abspath $1) && autoreconf -i -f
endef

# Run automake
define SAS_Autotools_Automake_Raw
	@echo ">>> Automake: $1"
	$(Q)cd $(abspath $1) && automake -a -c
endef

define SAS_Autotools_Generate_Raw
	$(call SAS_Autotools_Libtoolize_Raw, $1)
	$(call SAS_Autotools_Aclocal_Raw, $1)
	$(call SAS_Autotools_Autoheader_Raw, $1)
	$(call SAS_Autotools_Autoconf_Raw, $1)
	$(call SAS_Autotools_Automake_Raw, $1)
endef

define SAS_Autotools_Configure_Raw
	@echo ">>> Configuring: $2"
	$(Q)if [ "$(abspath $1)" != "$(abspath $2)" ]; then \
		rm -rf $(abspath $2); \
		mkdir -p $(abspath $2); \
	else \
		rm -f $(abspath $1)/Makefile; \
		rm -f $(abspath $1)/config.status; \
		rm -f $(abspath $1)/config.log; \
	fi
	$(Q)cd $(abspath $2) && $(abspath $1)/configure \
		--prefix=$(SAS_CONFIGURE_PREFIX) \
		$(CONFIGURE_SILENT) \
		$(strip $3)
endef

# Run autotools configure for host
define SAS_Autotools_Configure_Host
	$(call SAS_Autotools_Check_Prereq)
	$(call SAS_Autotools_Configure_Raw,$(SAS_SOURCE),$1,\
		$(SAS_CONFIGURE_OPTS) \
		$(call SAS_Autotools_Configure_SetHostUserFlags))
endef

# Run autotools configure for host with config cache
define SAS_Autotools_Configure_Host_Cached
	$(call SAS_Autotools_Check_Prereq)
	$(call SAS_Autotools_Configure_SanitizeCache,$(SAS_WORKSPACE_ROOT)/host.config.cache)
	$(call SAS_Autotools_Configure_Raw,$(SAS_SOURCE),$1,\
		--cache-file=$(SAS_WORKSPACE_ROOT)/host.config.cache \
		$(SAS_CONFIGURE_OPTS) \
		$(call SAS_Autotools_Configure_SetHostUserFlags))
endef

# Run autotools configure for target
define SAS_Autotools_Configure_Target
	$(call SAS_Autotools_Check_Prereq)
	$(call SAS_Autotools_Configure_Raw,$(SAS_SOURCE),$1,\
		$(call SAS_Autotools_Configure_SetTargetTools) \
		$(SAS_CONFIGURE_OPTS) \
		$(SAS_Autotools_Configure_Tools) \
		$(call SAS_Autotools_Configure_SetTargetUserFlags))
endef

# Run autotools configure for target with config cache
define SAS_Autotools_Configure_Target_Cached
	$(call SAS_Autotools_Check_Prereq)
	$(call SAS_Autotools_Configure_SanitizeCache,$(SAS_WORKSPACE_ROOT)/target.config.cache)
	$(call SAS_Autotools_Configure_Raw,$(SAS_SOURCE),$1,\
		--cache-file=$(SAS_WORKSPACE_ROOT)/target.config.cache \
		$(call SAS_Autotools_Configure_SetTargetTools) \
		$(SAS_CONFIGURE_OPTS) \
		$(SAS_Autotools_Configure_Tools) \
		$(call SAS_Autotools_Configure_SetTargetUserFlags))
endef

# Run autotools configure for target with config cache and with include paths in CPPFLAGS
define SAS_Autotools_Configure_Target_Cached_CPPFlags
	$(call SAS_Autotools_Check_Prereq)
	$(call SAS_Autotools_Configure_SanitizeCache,$(SAS_WORKSPACE_ROOT)/target.config.cache)
	$(call SAS_Autotools_Configure_Raw,$(SAS_SOURCE),$1,\
		--cache-file=$(SAS_WORKSPACE_ROOT)/target.config.cache \
		$(call SAS_Autotools_Configure_SetTargetTools) \
		$(SAS_CONFIGURE_OPTS) \
		$(SAS_Autotools_Configure_Tools) \
		$(call SAS_Autotools_Configure_SetTargetUserWithCppFlags))
endef

# Run autotools configure for target without SAS SDK set
define SAS_Autotools_Configure_TargetNoSdk
	$(call SAS_Autotools_Check_Prereq)
	$(call SAS_Autotools_Configure_Raw,$(SAS_SOURCE),$1,\
		$(call SAS_Autotools_Configure_SetTargetTools) \
		$(SAS_CONFIGURE_OPTS) \
		$(SAS_Autotools_Configure_Tools) \
		$(call SAS_Autotools_Configure_SetTargetUserNoSdkFlags))
endef

# Run autotools configure for target with config cache without SAS SDK set
define SAS_Autotools_Configure_TargetNoSdk_Cached
	$(call SAS_Autotools_Check_Prereq)
	$(call SAS_Autotools_Configure_SanitizeCache,$(SAS_WORKSPACE_ROOT)/target.config.cache)
	$(call SAS_Autotools_Configure_Raw,$(SAS_SOURCE),$1,\
		--cache-file=$(SAS_WORKSPACE_ROOT)/target.config.cache \
		$(call SAS_Autotools_Configure_SetTargetTools) \
		$(SAS_CONFIGURE_OPTS) \
		$(SAS_Autotools_Configure_Tools) \
		$(call SAS_Autotools_Configure_SetTargetUserNoSdkFlags))
endef

# Run autotools configure for target drivers
define SAS_Autotools_Configure_TargetDriver
	$(call SAS_Autotools_Check_Prereq)
	$(call SAS_Autotools_Configure_Raw,$(SAS_SOURCE),$1,\
		$(call SAS_Autotools_Configure_SetTargetTools) \
		$(SAS_CONFIGURE_OPTS) \
		$(SAS_Autotools_Configure_Tools) \
		$(call SAS_Autotools_Configure_SetDriverUserFlags))
endef

# Run autotools configure for target drivers with config cache
define SAS_Autotools_Configure_TargetDriver_Cached
	$(call SAS_Autotools_Check_Prereq)
	$(call SAS_Autotools_Configure_SanitizeCache,$(SAS_WORKSPACE_ROOT)/target.config.cache)
	$(call SAS_Autotools_Configure_Raw,$(SAS_SOURCE),$1,\
		--cache-file=$(SAS_WORKSPACE_ROOT)/target.config.cache \
		$(call SAS_Autotools_Configure_SetTargetTools) \
		$(SAS_CONFIGURE_OPTS) \
		$(SAS_Autotools_Configure_Tools) \
		$(call SAS_Autotools_Configure_SetDriverUserFlags))
endef

# Run autotools compile for target
define SAS_Autotools_Compile
	@echo ">>> Compiling: $1"
	$(Q)+$(MAKE) $(MAKE_SILENT) -C $(abspath $1)
endef

# Run autotools compile for target with build tools set
define SAS_Autotools_CompileWithTools
	@echo ">>> Compiling: $1"
	$(Q)+$(MAKE) $(MAKE_SILENT) -C $(abspath $1) \
		$(SAS_Autotools_Configure_Tools)
endef

# Run autotools install for target in local build dir
define SAS_Autotools_InstallLocal
	@echo ">>> Installing local: $1"
	$(Q)+$(MAKE) $(MAKE_SILENT) -C $(abspath $1) install DESTDIR=$(SAS_BUILD)
endef

# Run autotools install for target in local build dir
define SAS_Autotools_InstallSpecific
	@echo ">>> Installing: $1 -> $2"
	$(Q)mkdir -p $(abspath $2)
	$(Q)+$(MAKE) $(MAKE_SILENT) -C $(abspath $1) install DESTDIR=$(abspath $2)
endef

endif
