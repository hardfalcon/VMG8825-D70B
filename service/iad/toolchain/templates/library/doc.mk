##############################################################################
# File:      doc.mk                                                          #
# Purpose:   Library-specific template for build step doc.                   #
# Remarks:                                                                   #
#                                                                            #
# Copyright: Copyright (C) 2006, Sphairon Access Systems GmbH                #
#                                                                            #
# Author:    Christian Schroeder                                             #
# Created:   12.07.2007                                                      #
##############################################################################

.SUFFIXES:

.PHONY: doc doc-html doc-pdf doc-html-install doc-pdf-install

# get the current SAS workspace root folder (i.e. the folder above us
# containing the directory 'iad')
SAS_WORKSPACE_ROOT ?= $(firstword $(subst /iad/, ,$(CURDIR)))

include $(SAS_WORKSPACE_ROOT)/paths.mk
include $(SAS_WORKSPACE_ROOT)/exports.mk
include settings.mk
include $(SAS_TEMPLATES_PATH)/user/settings.mk
include $(SAS_TEMPLATES_PATH)/library/settings.mk
include $(SAS_TEMPLATES_PATH)/generic/verbose.mk
include $(SAS_TEMPLATES_PATH)/generic/debug.mk


SAS_DOC_OUTPUT		:= $(abspath $(SAS_BUILD)/doc)
SAS_DOC_CLANG		?= English
SAS_DOC_SLANG		?= C++
ifneq ($(SAS_LIB_MAJOR),)
SAS_DOC_PNUM		:= $(SAS_LIB_MAJOR).$(SAS_LIB_MINOR).$(SAS_LIB_PATCH)
else
SAS_DOC_PNUM		:= 0
endif
SAS_DOC_PDF		?= no
SAS_DOC_RTF		?= no
SAS_DOC_INPUT		?= $(abspath $(SAS_ROOT)/include)

ifeq ($(SAS_TARGET),)
SAS_DOC_TARGET		:= $(lastword $(subst /, ,$(dir $(CURDIR))))
else
SAS_DOC_TARGET		:= $(SAS_TARGET)
endif

define build_doxy_tag
$1/tagfiles/$2.tag=$1/$2/html
endef

SAS_DOC_TAGFILES	:= $(wildcard $(SAS_PLATFORM-SDK_DOC_PATH)/tagfiles/*.tag)
SAS_DOC_TARGETS		:= $(SAS_DOC_TAGFILES:$(SAS_PLATFORM-SDK_DOC_PATH)/tagfiles/%.tag=%)
SAS_DOC_TAG_TARGETS	:= $(filter-out $(SAS_DOC_TARGET),$(SAS_DOC_TARGETS))
SAS_DOC_EXT_TAGFILES	:= $(foreach tag,$(SAS_DOC_TAG_TARGETS),\
				$(call build_doxy_tag,$(SAS_PLATFORM-SDK_DOC_PATH),$(tag)))

SAS_DOC_TAGFILES	:= $(wildcard $(SAS_SERVICE-SDK_DOC_PATH)/tagfiles/*.tag)
SAS_DOC_TARGETS		:= $(SAS_DOC_TAGFILES:$(SAS_SERVICE-SDK_DOC_PATH)/tagfiles/%.tag=%)
SAS_DOC_TAG_TARGETS	:= $(filter-out $(SAS_DOC_TARGET),$(SAS_DOC_TARGETS))
SAS_DOC_EXT_TAGFILES	+= $(foreach tag,$(SAS_DOC_TAG_TARGETS),\
				$(call build_doxy_tag,$(SAS_SERVICE-SDK_DOC_PATH),$(tag)))

$(call SAS_TraceVariable,SAS_DOC_EXT_TAGFILES)

SAS_DOC_ENABLED_SECTIONS ?= no

SAS_DOC_INSTALL_PATH	:= $(SAS_ACTIVE-SDK_DOC_PATH)/$(SAS_DOC_TARGET)
SAS_DOC_TAGFILE		:= $(SAS_DOC_OUTPUT)/$(SAS_DOC_TARGET).tag

HTMLDOC := $(SAS_DOC_OUTPUT)/html
RTFDOC  := $(SAS_DOC_OUTPUT)/rtf
PDFDOC  := $(SAS_DOC_OUTPUT)/latex

DOXYFILE_DEPS := $(SAS_TEMPLATES_PATH)/library/settings.mk
DOXYFILE_DEPS += $(SAS_TEMPLATES_PATH)/library/doc.mk
DOXYFILE_DEPS += $(SAS_TEMPLATES_PATH)/user/settings.mk

ifeq ($(SAS_DOC_SLANG),C)
  CONFIG_FILE := $(SAS_TEMPLATES_PATH)/user/Doxyfile_c.tmpl
else
ifeq ($(SAS_DOC_SLANG),C++)
  CONFIG_FILE := $(SAS_TEMPLATES_PATH)/user/Doxyfile_cpp.tmpl
else
  $(error $(SAS_DOC_SLANG) language currently not supported.)
endif
endif

DOXYGEN := $(shell which doxygen)

ifneq ($(DOXYGEN),)
DOC_TARGETS	+= doc-html
ifneq ($(SAS_ACTIVE-SDK_DOC_PATH),)
DOC_INSTALL_TARGETS += doc-install-html
endif
ifeq ($(SAS_DOC_PDF),yes)
DOC_TARGETS	+= doc-pdf
ifneq ($(SAS_ACTIVE-SDK_DOC_PATH),)
DOC_INSTALL_TARGETS += doc-install-pdf
endif
endif
endif

doc: $(DOC_TARGETS) $(DOC_INSTALL_TARGETS)

doc-install-html: | doc-html $(SAS_ACTIVE-SDK_DOC_PATH)/tagfiles
	@echo ">>> Installing doxygen HTML doc for $(SAS_DOC_TARGET) to $(SAS_DOC_INSTALL_PATH) <<<"
	$(Q)rm -rf $(SAS_DOC_INSTALL_PATH)/html
	$(Q)mkdir -p $(SAS_DOC_INSTALL_PATH)/html
	$(Q)cp -frp $(HTMLDOC) $(SAS_DOC_INSTALL_PATH)
	$(Q)cp -fp $(SAS_DOC_TAGFILE) $(SAS_ACTIVE-SDK_DOC_PATH)/tagfiles/

doc-install-pdf: doc-pdf
	@echo ">>> Installing doxygen PDF doc for $(SAS_DOC_TARGET) $(PDFDOC) <<<"
	$(Q)rm -rf $(SAS_DOC_INSTALL_PATH)/*.pdf
	$(Q)mkdir -p $(SAS_DOC_INSTALL_PATH)
	$(Q)cp -fp $(PDFDOC)/refman.pdf $(SAS_ACTIVE-SDK_DOC_PATH)/$(SAS_DOC_INSTALL_PATH)/$(SAS_DOC_TARGET).pdf

doc-html: $(SAS_DOC_OUTPUT)/Doxyfile
	@echo "$(SAS_ACTIVE-SDK_DOC_PATH)"
	@echo ">>> Generating doxygen HTML doc <<<"
	$(Q)rm -rf $(HTMLDOC) $(PDFDOC) $(RTFDOC)
	$(Q)$(DOXYGEN) $<
	$(Q)sed -i -e 's,$(abspath $(SAS_ROOT)),,g' $(SAS_DOC_TAGFILE)

doc-pdf: doc-html
	@echo ">>> Generating doxygen PDF doc <<<"
	$(Q)$(MAKE) -C $(PDFDOC) refman.pdf > /dev/null

$(SAS_DOC_OUTPUT)/Doxyfile: $(CONFIG_FILE) $(DOXYFILE_DEPS) | $(SAS_DOC_OUTPUT)
	$(Q)sed -e 's,SAS_DOC_TARGET,$(SAS_DOC_TARGET),g' \
		-e 's,SAS_DOC_PNUM,$(SAS_DOC_PNUM),g' \
		-e 's,SAS_DOC_INPUT,$(SAS_DOC_INPUT),g' \
		-e 's,SAS_DOC_OUTPUT,$(SAS_DOC_OUTPUT),g' \
		-e 's,SAS_DOC_TAGFILE,$(SAS_DOC_TAGFILE),g' \
		-e 's,SAS_DOC_EXT_TAGFILES,$(SAS_DOC_EXT_TAGFILES),g' \
		-e 's,SAS_DOC_CLANG,$(SAS_DOC_CLANG),g' \
		-e 's,SAS_DOC_ENABLED_SECTIONS,$(SAS_DOC_ENABLED_SECTIONS),g' \
		-e 's,SAS_DOC_PDF,$(SAS_DOC_PDF),g' \
		-e 's,SAS_DOC_RTF,$(SAS_DOC_RTF),g' \
		$< > $@
	$(Q)$(DOXYGEN) -u $@

$(SAS_DOC_OUTPUT) $(SAS_ACTIVE-SDK_DOC_PATH) $(SAS_ACTIVE-SDK_DOC_PATH)/tagfiles:
	$(Q)install -d $@
