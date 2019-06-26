ENV = $$(SAS_QT_ROOT)
isEmpty(ENV) {
	error (SAS_ROOT required)
}

ENV = $$(SAS_QT_BUILD)
isEmpty(ENV) {
	error (SAS_BUILD required)
}

TEMPLATE = app


ENV = $$(SAS_QT_INSTALL)
isEmpty(ENV) {
	error (SAS_INSTALL required)
}

ENV = $$(SAS_QT_SOURCES)
isEmpty(ENV) {
	error (SAS_SOURCES required)
}

ENV = $$(SAS_QT_TARGET)
isEmpty(ENV) {
	error (SAS_TARGET required)
}
	
TARGET = $$(SAS_QT_TARGET)

count(TARGET,1,">") {
	error (only a single entry allowed for SAS_TARGET)
}
	
DESTDIR  = $$(SAS_QT_BUILD)/bin


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
	DEPENDPATH  += $$(SAS_QT_ROOT)/include
}

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

CONFIG  += link_prl depend_prl

for(ENTRY, ENV) {
	!equals(ENTRY, core):!equals(ENTRY, network):!equals(ENTRY, xml):!equals(ENTRY, sql) {
		error ($${ENTRY} not allowd as value for SAS_MODULES list (core|xml|sql|network))
	}
	QT *= $${ENTRY}
}

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
	EXTRA_TARGET_NAME = $$member(MAP, 1)
        EXTRA_TARGET = extra_target_$${EXTRA_TARGET_NAME}

	!isEqual(EXTRA_TARGET_TYPE, CREATE_LINK_TO_TARGET) {
		error( $${EXTRA_TARGET_TYPE} is not allowed )
	}

	eval($${EXTRA_TARGET}.target = $${EXTRA_TARGET_NAME})
	eval($${EXTRA_TARGET}.depends = "$(TARGET)")
	eval($${EXTRA_TARGET}.commands = "$(SYMLINK)" "\"$(TARGET)\"" $$(SAS_QT_INSTALL)/$${EXTRA_TARGET_NAME})
	eval($${EXTRA_TARGET}.path = $$(SAS_QT_INSTALL))

	INSTALLS += $${EXTRA_TARGET}
}
