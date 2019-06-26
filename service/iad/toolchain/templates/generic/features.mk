ifndef SAS_TEMPLATES_INCLUDE_FEATURES
SAS_TEMPLATES_INCLUDE_FEATURES := 1

# Generate an option string for autotools configure scripts dependent on SAS_FEATURES
# $(call SAS_Feature_SetConfigureOption,FEATURE,CONFIGURE_FLAG)
define SAS_Feature_SetConfigureOption
$(if $(filter $(strip $(1)),$(SAS_FEATURES)),--enable-$(strip $(2)),--disable-$(strip $(2)))
endef

# Generate an SAS_ENABLE_xxx CFLAG string dependent on SAS_FEATURES
# $(call SAS_Feature_SetEnableCFLAG,FEATURE,CFLAG)
define SAS_Feature_SetEnableCFLAG
$(if $(filter $(strip $(1)),$(SAS_FEATURES)),-DSAS_ENABLE_$(strip $(2)),)
endef

# Generate a SAS_DISABLE_xxx CFLAG string dependent on SAS_FEATURES
# $(call SAS_Feature_SetDisableCFLAG,FEATURE,CFLAG)
define SAS_Feature_SetDisableCFLAG
$(if $(filter $(strip $(1)),$(SAS_FEATURES)),-DSAS_DISABLE_$(strip $(2)),)
endef

# Generate a HAVE_xxx CFLAG string dependent on SAS_FEATURES
# $(call SAS_Feature_SetCFLAG,FEATURE,CFLAG)
define SAS_Feature_SetHaveCFLAG
$(if $(filter $(strip $(1)),$(SAS_FEATURES)),-DHAVE_$(strip $(2)),)
endef

endif
