ifneq ($(GEN_PKG),)
ifeq ($(PKG_NAME),)
ifeq ($(NAME),)
 $(error ">>> PKG_NAME/NAME not defined <<<")
else
 PKG_NAME 	?= $(NAME)
endif
endif

ifeq ($(PKG_VERSION),)
ifeq ($(VERSION),)
 $(error ">>> PKG_VERSION/VERSION not defined <<<")
else
 $(info "Version: :$(VERSION): PKG_VERSION: :$(PKG_VERSION):")
 PKG_VERSION 	?= $(VERSION)
endif
endif

ifeq ($(PKG_LICENSE),)
ifeq ($(LICENSE),)
 $(error ">>> PKG_LICENSE/LICENSE package license not defined <<<")
else
 PKG_LICENSE 	?= $(LICENSE)
endif
endif

ifeq ($(PKG_DESC),)
ifeq ($(DESC),)
 $(error ">>> PKG_DESC/DESC package description not defined <<<")
else
 PKG_DESC 	?= $(DESC)
endif
endif

ifeq ($(PKG_DEPENDS),)
 $(error ">>> PKG_DEPENDS not defined <<<")
endif

ifeq ($(PKG_CFLAGS),)
 $(error ">>> PKG_CFLAGS not defined <<<")
 $(error ">>> a include directive -I<PATH> parameter shall be set or "" <<<")
endif

ifeq ($(PKG_LIBS),)
 $(error ">>> PKG_LIBS not defined <<<")
 $(error ">>> a link library path -L<PATH> and the library name -l<name> should be set or "" <<<")
endif

EXTRA_FILES	+= $(SAS_ACTIVE-SDK_ROOT)/pkg/$(PKG_NAME).pc
$(info generate $(PKG_FILES))
endif
