%.pc: settings.mk prepare.mk
	mkdir -p $(dir $@)
	echo "Name: "$(PKG_NAME) > $@
	echo "Version:" $(PKG_VERSION) >> $@
	echo "License:" $(PKG_LICENSE) >> $@
	echo "Description:" $(PKG_DESC) >> $@
	echo "Requires: "$(PKG_DEPENDS) >> $@
	echo "Requires.private: "$(PKG_DEPENDS_STATIC) >> $@
	echo "Cflags: "$(PKG_CFLAGS) >> $@
	echo "Libs: "$(PKG_LIBS) >> $@
