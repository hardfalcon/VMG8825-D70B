ifndef SAS_TEMPLATES_INCLUDE_KMOD
SAS_TEMPLATES_INCLUDE_KMOD := 1

include $(SAS_WORKSPACE_ROOT)/paths.mk
include $(SAS_WORKSPACE_ROOT)/exports.mk
include $(SAS_TEMPLATES_PATH)/generic/verbose.mk
include $(SAS_TEMPLATES_PATH)/generic/debug.mk

$(call SAS_AssertNotNull,SAS_TOOLCHAIN_KERNEL_PREFIX)
$(call SAS_AssertNotNull,SAS_INCLUDE)
$(call SAS_AssertNotNull,SAS_SOURCE)
$(call SAS_AssertNotNull,SAS_BUILD)
$(call SAS_AssertNotNull,SAS_TARGET)
$(call SAS_AssertNotNull,SAS_MOD_DIR)

define SAS_Kmod_Create_Kbuild
	@echo "Generating Kbuild"
	@echo "# Auto-generated Kbuild file for $(KBUILD_TARGET). Do not edit!" > $(1)
	@echo "obj-m := $(KBUILD_TARGET)" >> $(1)
	@if [ "$(KBUILD_OBJECTS)" != "" ] ; then \
		for obj in $(KBUILD_OBJECTS) ; do \
			echo "$(SAS_TARGET)-y += $$obj" >> $(1); \
		done; \
	fi
	@for cflag in $(CFLAGS) ; do \
		echo "ccflags-y += $$cflag" >> $(1); \
	done
endef

endif # !SAS_TEMPLATES_INCLUDE_KMOD

