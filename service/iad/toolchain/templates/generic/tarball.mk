ifndef SAS_TEMPLATES_INCLUDE_TARBALL
SAS_TEMPLATES_INCLUDE_TARBALL := 1

include $(SAS_WORKSPACE_ROOT)/paths.mk
include $(SAS_WORKSPACE_ROOT)/exports.mk
include $(SAS_TEMPLATES_PATH)/generic/verbose.mk
include $(SAS_TEMPLATES_PATH)/generic/git-source.mk

PATCHES_DIR ?= $(ROOT)/patches
PATCH_SUFFIX ?= .diff
PATCH_LEVEL ?= 0
PATCHES := $(sort $(wildcard $(PATCHES_DIR)/*$(PATCH_SUFFIX)))

SAS_USE_GIT	:= $(call SAS_UseGit,$(dir $(CURDIR)))

define SAS_Tarball_Get_Extract_Flags
$(strip 
    $(if $(findstring .tar.gz,$(1)),-xz,
	$(if $(findstring .tar.bz2,$(1)),-xj,
	    $(if $(findstring .tar.,$(1)),
		    $(error ">>> Unknown/unsupported file extension in $(1)! <<<"),
		    $(if $(findstring .tar,$(1)),-x,
			$(error ">>> Unknown/unsupported file extension in $(1)! <<<"))))))
endef


define SAS_Tarball_Apply_Patches_GIT
	@echo ">>> Using git for patching <<<"
	$(Q)cd $(SAS_SOURCE) && $(GIT) init
	$(ifneq ($(GITIGNORE_FILE),)
	    @echo "Using $(GITIGNORE_FILE) as .gitignore"
	    $(Q)cp -f $(GITIGNORE_FILE) $(SAS_SOURCE)/.gitignore)
	$(Q)cd $(SAS_SOURCE) && $(GIT) add .
	$(Q)cd $(SAS_SOURCE) && $(GIT) commit -m "Initial Commit"
	$(Q)cd $(SAS_SOURCE) && $(GIT) tag -a "origin" -m "Initial Commit with original sources tag"
	$(Q)cd $(SAS_SOURCE) && if test -d $(PATCHES_DIR); then \
	    echo ">>> Applying patches <<<"; \
	    for p in $(PATCHES); do git am --reject --keep-cr $$p || exit 1; done \
	fi
endef


define SAS_Tarball_Apply_Patches_Plain
	@echo ">>> Using patch for patching <<<"
	$(Q)cd $(SAS_SOURCE) && if test -d $(PATCHES_DIR); then \
	    for p in $(PATCHES); do echo "Applying $$p"; patch -p$(PATCH_LEVEL) <$$p || exit 1; done \
	fi
endef


define SAS_Tarball_Use_Prepared_Config_Cache
	@if [ -f $(ROOT)/config.cache ]; then \
	    echo ">>> Bringing in prepared config.cache <<<"; \
	    cp $(ROOT)/config.cache $(SAS_SOURCE); \
	fi
endef


define SAS_Tarball_Clean_Source_Dir
	@echo ">>> Cleaning up source directory $(1) <<<"
	$(Q)-rm -rf $(1)
	$(Q)install -d $(1)
endef


define SAS_Tarball_Extract_Tarball
	@echo ">>> Unpacking source from tarball: $(1) <<<"
	$(Q)cd $(2) && tar $(call SAS_Tarball_Get_Extract_Flags,$(1))f $(1)
endef


ifeq ($(SAS_USE_GIT),y) 
GIT	:= $(shell which git)
ifeq ($(GIT),)
$(error ">>> USE_GIT is set, but git is not installed! <<<")
endif
define SAS_Tarball_Apply_Patches
	$(call SAS_Tarball_Apply_Patches_GIT)
	$(call SAS_Tarball_Use_Prepared_Config_Cache)
endef
else
define SAS_Tarball_Apply_Patches
	$(call SAS_Tarball_Apply_Patches_Plain)
	$(call SAS_Tarball_Use_Prepared_Config_Cache)
endef
endif

endif # !SAS_TEMPLATES_INCLUDE_TARBALL
