##############################################################################
# File:      compile.mk                                                      #
# Purpose:   Linux kernel implementation of buildstep compile                #
#                                                                            #
# Copyright: Copyright (C) 2011, Sphairon Technologies GmbH                  #
#                                                                            #
# Author:    Daniel Schwierzeck                                              #
# Created:   20.09.2011                                                      #
##############################################################################

.SUFFIXES:
MAKEFLAGS := -rR --no-print-directory

# get the current SAS workspace root folder (i.e. the folder above us
# containing the directory 'iad')
SAS_WORKSPACE_ROOT ?= $(firstword $(subst /iad/, ,$(CURDIR)))

include $(SAS_WORKSPACE_ROOT)/paths.mk
include $(SAS_WORKSPACE_ROOT)/exports.mk
include settings.mk
include $(SAS_TEMPLATES_PATH)/generic/verbose.mk
include $(SAS_TEMPLATES_PATH)/generic/debug.mk
include $(SAS_TEMPLATES_PATH)/linux-kernel/settings.mk

META_FILES					+= linux.entryaddress
META_FILES					+= linux.loadaddress
META_FILES					+= linux.version
META_FILES					+= linux.arch
META_FILES					+= linux.image
META_FILES					+= linux.config
META_FILES					+= linux.squashfs_comp

COMPILE_DEPS-y					+= $(SAS_TEMPLATES_PATH)/linux-kernel/settings.mk
COMPILE_DEPS-y					+= $(SAS_TEMPLATES_PATH)/linux-kernel/compile.mk
COMPILE_DEPS-y					+= $(SAS_BUILD)/configure.stamp
COMPILE_DEPS-$(SAS_COMPILE_EXTRA)		+= $(KMOD_EXTRA_INSTALL_DIRS)

ifeq ($(SAS_COMPILE_KERNELIMAGE),)
SAS_COMPILE_KERNELIMAGE				:= zImage
endif
COMPILE_TARGETS-y				+= $(SAS_BUILD)/$(TARGET_VMLINUX)
COMPILE_TARGETS-y				+= $(SAS_BUILD)/vmlinux
COMPILE_TARGETS-y				+= $(SAS_BUILD)/System.map
COMPILE_TARGETS-y				+= $(SAS_BUILD)/Module.symvers
COMPILE_TARGETS-y				+= $(SAS_BUILD)/.missing-syscalls.d
COMPILE_TARGETS-y				+= $(META_FILES:%=$(SAS_BUILD)/%)
COMPILE_TARGETS-y				+= $(SAS_SCRIPT_PERM_FIX_STAMPS)
COMPILE_TARGETS-$(SAS_COMPILE_CHECKSTACK)	+= $(SAS_BUILD)/checkstack.stamp
COMPILE_TARGETS-y				+= $(SAS_BUILD)/gen_init_cpio
COMPILE_TARGETS-y				+= $(SAS_BUILD)/gen_initramfs_list.sh

COMPILE_ENV-$(SAS_COMPILE_CHECKSECTIONS)	+= CONFIG_DEBUG_SECTION_MISMATCH=y

# Always run kernel compile if SAS_COMPILE_ALWAYS = y otherwise build only once
FORCE_TARGET-$(SAS_COMPILE_ALWAYS)		:= FORCE

# Generic kbuild invocation function
define Linux/Build/Target
$(Q)+$(MAKE) -C $(SAS_KBUILD_DIR) $(1) \
	ARCH=$(SAS_KERNEL_ARCH) \
	CROSS_COMPILE=$(SAS_CROSS_COMPILE) \
	SAS_INITRAMFS_SOURCE=$(SAS_INITRAMFS_SOURCE) \
	$(COMPILE_ENV-y)
endef

define Linux/Build/InstallModulesExtra
	@echo ">>> Installing extra modules to: $(KMOD_EXTRA_INSTALL_PATH) <<<"
	$(Q)for f in $(SAS_KMOD_EXTRA_OBJECTS); do \
		echo ">>> Installing extra module: $$f <<<"; \
		mod_src=$(SAS_KBUILD_DIR)/$$f; \
		mod_dst=$(KMOD_EXTRA_INSTALL_PATH)/$$(basename $$f); \
		mkdir -p $(KMOD_EXTRA_INSTALL_PATH); \
		cp -af $$mod_src $$mod_dst; \
		$(SAS_CROSS_STRIP) --strip-unneeded $$mod_dst; \
	done
endef

define Linux/Build/CleanInitramfs
	$(Q)rm -rf $(SAS_KBUILD_DIR)/usr/initramfs_data.*
endef

.PHONY: compile
compile: $(SAS_BUILD)/compile.stamp
$(SAS_BUILD)/compile.stamp: $(COMPILE_TARGETS-y)
	$(Q)touch $@

# Common target if arch specific kbuild generate a raw binary image
$(SAS_BUILD)/vmlinux.bin: $(COMPILE_DEPS-y) $(FORCE_TARGET-y)
ifeq ($(SAS_COMPILE_EXTRA),y)
	+$(call Linux/Build/Target,modules)
	+$(call Linux/Build/InstallModulesExtra)
	$(call Linux/Build/CleanInitramfs)
endif
	+$(call Linux/Build/Target,vmlinux.bin)
ifeq ($(SAS_COMPILE_EXTRA),n)
	+$(call Linux/Build/Target,modules)
	+$(call Linux/Build/InstallModulesExtra)
endif
	$(Q)install -m644 $(SAS_KBUILD_DIR)/arch/$(SAS_KERNEL_ARCH)/boot/vmlinux.bin $@

# Special target if arch specific kbuild only generate a zip compressed binary image
$(SAS_BUILD)/vmlinux.bin.gz: $(COMPILE_DEPS-y) $(FORCE_TARGET-y)
ifeq ($(SAS_COMPILE_EXTRA),y)
	+$(call Linux/Build/Target,modules)
	+$(call Linux/Build/InstallModulesExtra)
	$(call Linux/Build/CleanInitramfs)
endif
	+$(call Linux/Build/Target,$(SAS_COMPILE_KERNELIMAGE))
ifeq ($(SAS_COMPILE_EXTRA),n)
	+$(call Linux/Build/Target,modules)
	+$(call Linux/Build/InstallModulesExtra)
endif
	$(Q)install -m644 $(SAS_KBUILD_DIR)/vmlinux.bin.gz $@

# Target to manually rebuild modules
.PHONY: modules
modules: $(COMPILE_DEPS-y) $(FORCE_TARGET-y)
	+$(call Linux/Build/Target,modules)
	+$(call Linux/Build/Target,modules_install) \
		INSTALL_MOD_PATH=$(KMOD_INSTALL_PATH)

# Target for installing initramfs tools
$(SAS_KBUILD_DIR)/usr/gen_init_cpio: | $(SAS_BUILD)/$(KBUILD_VMLINUX)
$(SAS_BUILD)/gen_init_cpio: $(SAS_KBUILD_DIR)/usr/gen_init_cpio
	$(Q)install -m755 $< $@

$(SAS_BUILD)/gen_initramfs_list.sh: $(SAS_SOURCE)/scripts/gen_initramfs_list.sh
	$(Q)install -m755 $< $@

# Create install dirs for extra kernel modules
$(KMOD_EXTRA_INSTALL_DIRS):
	$(Q)mkdir -p $@

# Run make checkstack to generate a list of stack hogs
$(SAS_BUILD)/checkstack.stamp: $(SAS_BUILD)/$(KBUILD_VMLINUX)
	+$(call Linux/Build/Target,checkstack)
	$(Q)touch $@

# Install vmlinux ELF binary
$(SAS_KBUILD_DIR)/vmlinux: | $(SAS_BUILD)/$(KBUILD_VMLINUX)
$(SAS_BUILD)/vmlinux: $(SAS_KBUILD_DIR)/vmlinux
	$(Q)install -m644 $< $@

# Install System.map
$(SAS_KBUILD_DIR)/System.map: | $(SAS_BUILD)/$(KBUILD_VMLINUX)
$(SAS_BUILD)/System.map: $(SAS_KBUILD_DIR)/System.map
	$(Q)install -m644 $< $@

# Install Module.symvers
$(SAS_KBUILD_DIR)/Module.symvers: | $(SAS_BUILD)/$(KBUILD_VMLINUX)
$(SAS_BUILD)/Module.symvers: $(SAS_KBUILD_DIR)/Module.symvers
	$(Q)install -m644 $< $@

# Install .missing-syscalls.d
$(SAS_KBUILD_DIR)/.missing-syscalls.d: | $(SAS_BUILD)/$(KBUILD_VMLINUX)
$(SAS_BUILD)/.missing-syscalls.d: $(SAS_KBUILD_DIR)/.missing-syscalls.d
	$(Q)install -m644 $< $@

# Determine and install kernel entry address
$(SAS_BUILD)/linux.entryaddress: $(SAS_KBUILD_DIR)/$(SAS_KERNEL_ARCH).linux.entryaddress
	$(Q)install -m644 $< $@

$(SAS_KBUILD_DIR)/mips.linux.entryaddress: $(SAS_BUILD)/System.map
	$(Q)awk '/T kernel_entry/ { print $$1 }' $< > $@

$(SAS_KBUILD_DIR)/powerpc.linux.entryaddress: $(SAS_BUILD)/configure.stamp
	$(Q)echo 0 > $@

# Determine and install kernel load address
$(SAS_BUILD)/linux.loadaddress: $(SAS_BUILD)/System.map
	$(Q)awk '/[AT] _text/ { print $$1 }' $< > $@

# Install additional meta information
$(SAS_BUILD)/linux.version: $(SAS_BUILD)/configure.stamp
	$(Q)cat $(SAS_KBUILD_DIR)/include/config/kernel.release > $@

$(SAS_BUILD)/linux.arch: $(SAS_BUILD)/configure.stamp
	$(Q)echo $(SAS_KERNEL_ARCH) > $@

$(SAS_BUILD)/linux.image: $(SAS_BUILD)/configure.stamp
	$(Q)echo $(TARGET_VMLINUX) > $@

$(SAS_BUILD)/linux.config: $(SAS_KBUILD_DIR)/.config | $(SAS_BUILD)/configure.stamp
	$(Q)cp $< $@ > $@

$(SAS_BUILD)/linux.squashfs_comp: $(SAS_BUILD)/configure.stamp
	$(Q)echo $(if $(SAS_SQUASHFS_COMP),$(SAS_SQUASHFS_COMP),lzma) > $@

# Fixup script permissions if added by patches
$(SAS_BUILD)/permfix.%.stamp: $(SAS_SOURCE)/scripts/% | $(SAS_BUILD)/configure.stamp
	$(Q)chmod u+x $<
	$(Q)touch $@

.PHONY: FORCE
FORCE: ;

