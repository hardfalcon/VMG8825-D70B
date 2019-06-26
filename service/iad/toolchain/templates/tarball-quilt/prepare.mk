##############################################################################
# File:      prepare.mk                                                      #
# Purpose:   Default implementation of build step prepare.                   #
# Remarks:                                                                   #
#                                                                            #
# Copyright: Copyright (C) 2009,2010 Sphairon Access Systems GmbH            #
# Copyright: Copyright (C) 2010, Sphairon Technologies GmbH                  #
#                                                                            #
# Author:    Daniel Schwierzeck                                              #
# Created:   19.10.2009                                                      #
##############################################################################

.SUFFIXES:
.NOTPARALLEL:
.PHONY: prepare preparedirs apply-patches-quilt

include settings.mk
include $(SAS_TEMPLATES_PATH)/generic/pkg-requirements.mk
include $(SAS_TEMPLATES_PATH)/generic/debug.mk

$(call SAS_AssertNotNull,SAS_BUILD)
$(call SAS_AssertNotNull,SAS_TARBALL_NAME)
$(call SAS_AssertNotNull,SAS_TARBALL_VERSION)
$(call SAS_AssertNotNull,SAS_TARBALL_SUFFIX)
$(call SAS_AssertNotNull,SAS_TARBALL_DIR)
$(call SAS_AssertNotNull,SAS_PATCHES_DIR)
$(call SAS_AssertNotNull,SAS_EXTRACT_DIR)
$(call SAS_AssertNotNull,SAS_SERIES_DIR)
$(call SAS_AssertNotNull,SAS_SERIES_FILES)
$(call SAS_AssertNotNull,SAS_SOURCE)
$(call SAS_AssertNotNull,SAS_TARBALL)

QUILT		:= $(shell which quilt)
GIT		:= $(shell which git)

ifneq ($(USE_GIT),)
ifeq ($(GIT),)
 $(error ">>> USE_GIT is set, but git is not installed! <<<")
else
 QUILT 		:= 
 ifneq ($(SAS_PATCHES_AUTHOR),)
	GIT_QUILTPATCHES_AUTHOR	:= --author $(SAS_PATCHES_AUTHOR)
 endif
endif
endif

PATCH_FILES	:= $(shell find $(SAS_PATCHES_DIR) -iname "*.patch" ! -type d)
SERIES_PATCHES	= $(foreach patch,$(shell cat $(SAS_SOURCE)/series),$(patsubst %,"%",$(patch)))
SERIES_FILES	:= $(patsubst %,$(SAS_SERIES_DIR)/%,$(SAS_SERIES_FILES))

IGNORE_UPDATES	:= $(shell [ -e $(SAS_BUILD)/ignore-updates ] && echo 1 || echo 0)

ifeq ($(IGNORE_UPDATES),1)
prepare:
else
prepare: $(SAS_BUILD)/patch.stamp
endif

$(SAS_BUILD)/extract.stamp: settings.mk prepare.mk $(SAS_TARBALL_DIR)/$(SAS_TARBALL) $(PATCH_FILES) $(SERIES_FILES) $(EXTRA_FILES)
	@if [ -d $(SAS_EXTRACT_DIR) ]; then \
		echo ">>> Cleaning source directory <<<"; \
		chmod +w -R $(SAS_EXTRACT_DIR); \
		rm -rf $(SAS_EXTRACT_DIR); \
	fi
	@echo ">>> Extracting tarball: $(SAS_TARBALL) <<<"
	@install -d $(SAS_EXTRACT_DIR)
ifeq ($(SAS_TARBALL_SUFFIX),tar.bz2)
	@tar -xjf $(SAS_TARBALL_DIR)/$(SAS_TARBALL) -C $(SAS_EXTRACT_DIR)
endif
ifeq ($(SAS_TARBALL_SUFFIX),tar.gz)
	@tar -xzf $(SAS_TARBALL_DIR)/$(SAS_TARBALL) -C $(SAS_EXTRACT_DIR)
endif
	@touch $@

$(SAS_SOURCE)/series: $(SAS_BUILD)/extract.stamp $(SERIES_FILES)
	@echo -n > $@
	@for seriesfile in $(SAS_SERIES_FILES); do \
		echo ">>> Adding to series file: $$seriesfile <<<"; \
		cat $(SAS_SERIES_DIR)/$$seriesfile >> $@; \
	done

$(SAS_BUILD)/patch.stamp: $(SAS_SOURCE)/series
	@if [ -d $(SAS_SOURCE)/patches ]; then \
		rm -rf $(SAS_SOURCE)/patches; \
	fi
	@if [ ! -L $(SAS_SOURCE)/patches ]; then \
		ln -sf $(SAS_PATCHES_DIR) $(SAS_SOURCE)/patches; \
	fi
ifneq ($(USE_GIT),)
	cp -av $(SAS_SOURCE)/series $(SAS_SOURCE)/patches
endif

ifneq ($(PATCH_FILES),)
ifneq ($(USE_GIT),)
	@echo "Setting up inital GIT repository"
	@cd $(SAS_SOURCE) && $(GIT) init
ifneq ($(GITIGNORE_FILE),)
	@echo "Using $(GITIGNORE_FILE) as .gitignore"
	cp -f $(GITIGNORE_FILE) $(SAS_SOURCE)/.gitignore
else
	@echo "Create default .gitignore file with patches, since we won't track them"
	echo "patches" >> $(SAS_SOURCE)/.gitignore
endif
	@cd $(SAS_SOURCE) && $(GIT) add .
	@cd $(SAS_SOURCE) && $(GIT) commit -m "Initial Commit"
	@cd $(SAS_SOURCE) && $(GIT) tag -a "origin" -m "Initial Commit with original sources tag"
	@echo "Importing Quilt Patches"
	@cd $(SAS_SOURCE) && $(GIT) quiltimport $(GIT_QUILTPATCHES_AUTHOR) --patches patches
	@echo "Finished Import of Quilt Patches"
	@echo "Cleanup series file"
	rm $(SAS_SOURCE)/patches
else
ifeq ($(QUILT),)
	@echo ">>> Applying patches manually <<<"
	@for i in $(SERIES_PATCHES); do \
		( echo $$i | grep -q "^#" ) && continue; \
		( echo $$i | grep -q "^ " ) && continue; \
		( echo $$i | grep -q "^$$" ) && continue; \
		NAME="$${i% \-p*}"; \
		DEPTH="$${i#*patch }"; \
		if [ "$$NAME" = "$$DEPTH" ]; then \
			DEPTH="-p1"; \
		fi; \
		if [ -e $(SAS_SOURCE)/patches/$$NAME ]; then \
			echo "Applying patch: $$NAME"; \
			cd $(SAS_SOURCE) && patch $$DEPTH < patches/$$NAME; \
			if [ $$? -ne 0 ]; then \
				echo "Failed to apply patch: $$NAME"; \
				exit 1; \
			fi; \
		fi; \
	done
else
	@echo ">>> Applying patches with quilt <<<"
	@cd $(SAS_SOURCE) && $(QUILT) push -aq
endif
endif
else
	@echo ">>> No patches in series file <<<"
endif
	@touch $@

include $(SAS_TEMPLATES_PATH)/generic/pkg-implicit-target.mk
