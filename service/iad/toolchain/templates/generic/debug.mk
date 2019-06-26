##############################################################################
# File:      debug.mk                                                        #
# Purpose:   Debugging and assertion functions                               #
#                                                                            #
# Copyright: Copyright (C) 2011, Sphairon Technologies GmbH                  #
#                                                                            #
# Author:    Daniel Schwierzeck                                              #
# Created:   31.08.2011                                                      #
##############################################################################

ifndef SAS_TEMPLATES_INCLUDE_DEBUG
SAS_TEMPLATES_INCLUDE_DEBUG := 1

# $(call SAS_Assert,condition,message)
define SAS_Assert
$(if $1,,$(error Assertion failed: $2))
endef

# $(call SAS_AssertFileExists,wildcard-pattern)
define SAS_AssertFileExists
$(call SAS_Assert,$(wildcard $1),$1 does not exist)
endef

# $(call SAS_AssertDirectoryExists,directory-name)
define SAS_AssertDirectoryExists
$(call SAS_Assert,$(shell test -d $1 && echo $1 || echo ""),$1 does not exist)
endef

# $(call SAS_AssertNotNull,make-variable)
define SAS_AssertNotNull
$(call SAS_Assert,$(strip $($1)),The variable "$1" is null)
endef

# $(call SAS_AssertIsEqual,make-variable,value)
define SAS_AssertIsEqual
$(call SAS_Assert,$(filter $2,$($1)),The variable "$1" is not equal to "$2")
endef

# $(call SAS_ConfigError,<error-msg>)
define SAS_ConfigError
$(error SAS_CONFIG required$(if $1, ($1),))
endef

space	:=
space 	+=
or 	:= |
comma   := ,

# $(eval $(call SAS_ConfigMakefileError,<config-dir>))
define SAS_ConfigMakefileError
$(error $(call SAS_ConfigError,($(subst $(space),$(or),$(basename $(notdir $(wildcard $1/*.mk)))))))
endef

# $(eval (call SAS_CheckMakefile,<config-dir>,<makefile-name>))
define SAS_CheckMakefile
$(if $(strip $2),\
	$(if $(wildcard $1/$(strip $2).mk),,\
		$(call SAS_ConfigMakefileError,$1)),\
	$(call SAS_ConfigMakefileError,$1))
endef

# $(eval (call SAS_CheckMakefiles,<config-dir>,<makefile name1 name2 name3..>))
define SAS_CheckMakefiles
$(foreach file,$2,$(eval $(call SAS_CheckMakefile,$1,$(file))))
endef

# $(eval (call SAS_CheckConfigMakefile,<config-dir>))
define SAS_CheckConfigMakefile
$(if $(strip $(SAS_CONFIG)),\
	$(if $(wildcard $1/$(strip $(SAS_CONFIG)).mk),,\
		$(call SAS_ConfigMakefileError,$1)),\
	$(call SAS_ConfigMakefileError,$1))
endef

# $(call SAS_GetConfigSubstring,<number>,[error-msg])
define SAS_GetConfigSubstring
$(strip $(if $(SAS_CONFIG),\
	$(word $1,$(subst -, ,$(strip $(SAS_CONFIG)))),\
	$(call SAS_ConfigError,$2)))
endef

# $(call SAS_TraceVariable,<make-variable>)
define SAS_TraceVariable
$(if $(filter 1,$(SAS_TEMPLATES_VERBOSE)),$(warning >>> $1 = $($1) <<<))
endef

# $(call SAS_FileExists,<file-path>)
define SAS_FileExists
$(shell [ -e $1 ] && echo y || echo n)
endef

# $(eval (call SAS_IncludeMakefiles,<dir>,<makefile name1 name2 name3..>))
define SAS_IncludeMakefiles
include $(foreach file,$(subst $(comma),$(space),$(2)),$1/$(file).mk)
endef

# $(call SAS_StripComma,<string>)
define SAS_StripComma
$(subst $(comma),$(space),$1)
endef

# $(call SAS_getConfigSpecs,<git-branch>)
define SAS_getConfigSpecs
$(call SAS_StripComma,$(strip $(patsubst %-$1,%,$(SAS_CONFIG))))
endef

endif # SAS_TEMPLATES_INCLUDE_DEBUG
