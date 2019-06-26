ENV = $$(SAS_QT_ROOT)
isEmpty(ENV) {
	error (SAS_ROOT required)
}

ENV = $$(SAS_QT_BUILD)
isEmpty(ENV) {
	error (SAS_BUILD required)
}

TEMPLATE = lib

ENV = $$(SAS_QT_INSTALL)
isEmpty(ENV) {
	error (SAS_INSTALL required)
}

ENV = $$(SAS_QT_SOURCES)
isEmpty(ENV) {
	error (SAS_SOURCES required)
}

ENV = $$(SAS_QT_LIBTYPE)
isEmpty(ENV) {
	error (SAS_LIBTYPE required)
}

ENV = $$(SAS_QT_VERSION)
isEmpty(ENV) {
	error (SAS_VERSION required)
}

VERSION = $$(SAS_QT_VERSION)

ENV = $$(SAS_QT_TARGET)
isEmpty(ENV) {
	error (SAS_TARGET required)
}
	
TARGET = $$(SAS_QT_TARGET)

count(TARGET,1,">") {
	error (only a single entry allowed for SAS_TARGET)
}
	
DESTDIR  = $$(SAS_QT_BUILD)/lib

LIBTYPE = $$(SAS_QT_LIBTYPE)

count(LIBTYPE,1,">") {
	error (only a single entry allowed for SAS_LIBTYPE)
}

!equals(LIBTYPE,shared):!equals(LIBTYPE,static) {
	error ($$(LIBTYPE) not allowd as value for SAS_LIBTYPE required (static|shared))
}

ENV = $$(SAS_QT_SOURCES)
for(ENTRY, ENV) {
	FILE = $$(SAS_QT_ROOT)/source/$${ENTRY}
	!exists($${FILE}) {
		error( $${FILE} not exists)
	}
	SOURCES += $${FILE}
}

ENV = $$(SAS_QT_HEADERS)
for(ENTRY, ENV) {
	FILE = $$(SAS_QT_ROOT)/include/$${ENTRY}
	!exists($${FILE}) {
		error( $${FILE} not exists)
	}

	HEADERS += $${FILE}
}

ENV = $$(SAS_QT_HEADERS)
!isEmpty(ENV) {
	INCLUDEPATH += $$(SAS_QT_ROOT)/include

	ENV = $$(SAS_QT_HEADERS_INSTALL)
	isEmpty(ENV) {
		error (SAS_HEADERS_INSTALL required when defines SAS_HEADERS)
	}

	headers.path = $$(SAS_QT_HEADERS_INSTALL)
	headers.files = $$(SAS_QT_HEADERS)

	INSTALLS += headers
}

DEPENDPATH  += $$INCLUDEPATH
DEPENDPATH  += $$(SAS_QT_ROOT)/source
	
ENV = $$(SAS_QT_DEFINES)
!isEmpty(ENV) {
	DEFINES += $$(SAS_QT_DEFINES)
}
	
ENV = $$(SAS_QT_CXXFLAGS)
!isEmpty(ENV) {
	QMAKE_CXXFLAGS += $$(SAS_QT_CXXFLAGS)
}

ENV = $$(SAS_QT_CFLAGS)
!isEmpty(ENV) {
	QMAKE_CFLAGS += $$(SAS_QT_CFLAGS)
}

ENV = $$(SAS_QT_RPATHDIR)
!isEmpty(ENV) {
	QMAKE_RPATH     = -Wl,-rpath-link,
	QMAKE_RPATHDIR += $$(SAS_QT_RPATHDIR)
}

QT = 
ENV = $$(SAS_QT_MODULES)

isEmpty(ENV) {
	CONFIG  =
}

CONFIG  += create_prl explicitlib depend_prl

for(ENTRY, ENV) {
	!equals(ENTRY, core):!equals(ENTRY, network):!equals(ENTRY, xml):!equals(ENTRY, sql) {
		error ($${ENTRY} not allowd as value for SAS_MODULES list (core|xml|sql|network))
	}
	QT *= $${ENTRY}
}

CONFIG  += $$(SAS_QT_LIBTYPE)

ENV = $$(SAS_QT_INCLUDE)
for(ENTRY, ENV) {
	INCLUDEPATH += $${ENTRY}
}

ENV = $$(SAS_QT_LIBS)
for(ENTRY, ENV) {
	LIBS += $${ENTRY}
}

MOC_DIR     = $$(SAS_QT_BUILD)/moc
RCC_DIR     = $$(SAS_QT_BUILD)/rcc
OBJECTS_DIR = $$(SAS_QT_BUILD)/obj
	
target.path = $$(SAS_QT_INSTALL)

INSTALLS += target




ENV = $$(SAS_QT_EXTRA_TARGET)
for(ENTRY, ENV) {
	MAP = $$split(ENTRY, "@")

	!count(MAP,2,"="){
		error( $${ENTRY} is not a valid deklaration SAS_EXTRA_TARGET )
	}
	
	EXTRA_TARGET_TYPE = $$member(MAP, 0)
	EXTRA_TARGET_PATH = $$member(MAP, 1)

	!isEqual(EXTRA_TARGET_TYPE, INSTALL_TO_FS) {
		error( $${EXTRA_TARGET_TYPE} is not allowed )
	}

	isEqual(LIBTYPE, shared) {
		FS_COPY_INFO.target   = fs_info
		FS_COPY_INFO.commands = @echo "\">>> copy libs to $${EXTRA_TARGET_PATH} <<<\""
	
		FS_COPY_TARGET_PATH.target   = fs_target_path
		FS_COPY_TARGET_PATH.commands = "@$(CHK_DIR_EXISTS)" $${EXTRA_TARGET_PATH} || "$(MKDIR)" $${EXTRA_TARGET_PATH}
		FS_COPY_TARGET_PATH.depends  = FS_COPY_INFO

		FS_COPY_TARGET.target   = fs_target
		FS_COPY_TARGET.commands = "$(INSTALL_FILE)" $$(SAS_QT_INSTALL)/$(TARGET)" $${EXTRA_TARGET_PATH}
		FS_COPY_TARGET.depends  = FS_COPY_TARGET_PATH
	
		FS_COPY_TARGET0.target   = fs_target0
		FS_COPY_TARGET0.commands = "$(SYMLINK)" "\"$(TARGET)\"" $${EXTRA_TARGET_PATH}/"$(TARGET0)"
		FS_COPY_TARGET0.depends  = FS_COPY_TARGET
	
		FS_COPY_TARGET1.target   = fs_target1
		FS_COPY_TARGET1.commands = "$(SYMLINK)" "\"$(TARGET)\"" $${EXTRA_TARGET_PATH}/"$(TARGET1)"
		FS_COPY_TARGET1.depends  = FS_COPY_TARGET0
	
		FS_COPY_TARGET2.target   = fs_target2
		FS_COPY_TARGET2.commands = "$(SYMLINK)" "\"$(TARGET)\"" $${EXTRA_TARGET_PATH}/"$(TARGET2)"
		FS_COPY_TARGET2.depends  = FS_COPY_TARGET1
	
		FS_COPY_EXTRA.target = fs_copy
		FS_COPY_EXTRA.depends = "$(DESTDIR)/$(TARGET)"
		FS_COPY_EXTRA.depends += FS_COPY_TARGET2
		FS_COPY_EXTRA.commands = @echo -n ""
		FS_COPY_EXTRA.path = $${EXTRA_TARGET_PATH}
	
		QMAKE_EXTRA_TARGETS += FS_COPY_EXTRA FS_COPY_TARGET2 FS_COPY_TARGET1 FS_COPY_TARGET0 FS_COPY_TARGET FS_COPY_TARGET_PATH FS_COPY_INFO
	
		INSTALLS += FS_COPY_EXTRA
	}
	else {
		FS_COPY_EXTRA.target = fs_copy
		FS_COPY_EXTRA.depends = "$(DESTDIR)/$(TARGETA)"
		FS_COPY_EXTRA.commands = @echo "\">>> skip copy libs to $${EXTRA_TARGET_PATH} for static libs <<<\""
		FS_COPY_EXTRA.path = /

		INSTALLS += FS_COPY_EXTRA
	}
}

ENV = $$(SAS_QT_PREDEPENDS)
for(ENTRY, ENV) {
	PRE_TARGETDEPS *= $${ENTRY}
}



