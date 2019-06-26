## Process this file with automake to produce Makefile.in

# static libraries
noinst_LIBRARIES = \
	libdti_protocol.a \
	libdti_client.a \
	libdti_agent.a

# dynamic libraries
lib_LTLIBRARIES = \
	libdti_agent.la

#	libdti_protocol.la

if BUILD_DTI_CLIENT
lib_LTLIBRARIES += \
	libdti_client.la
endif

if DEVICE_GENERIC
noinst_LIBRARIES += \
	libdti_device_generic.a
lib_LTLIBRARIES += \
	libdti_device_generic.la
endif


AM_CPPFLAGS = \
	-I@srcdir@\
	-I@srcdir@/include\
	$(TARGET_INCLUDE_PATH) \
	$(IFXOS_INCLUDE_PATH) $(DEV_DRIVER_INCL_PATH)

AM_LDFLAGS = @IFXOS_LIBRARY_PATH@

if HAVE_GCC

AM_CFLAGS = \
	-Wall -Wimplicit -Wunused -Wundef -Wswitch -Wcomment \
	-Wuninitialized -Wparentheses -Wreturn-type
# AM_LDFLAGS += \
# 	-Bstatic -dn -static

else !HAVE_GCC

AM_CFLAGS =
# AM_LDFLAGS += \
# 	-dn -Bstatic

endif !HAVE_GCC

# additional flags depending on the OS settings
additional_flags = -DADD_ON_FLAGS

if ENABLE_DEBUG
optimization_flags = -O1 -g
else
optimization_flags = -O1
endif


if ENABLE_LINUX
additional_flags += \
	-DLINUX -D_POSIX_SOURCE -D_GNU_SOURCE \
	-DRW_MULTI_THREAD -D_REENTRANT -D_THREAD_SAFE \
	-Wstrict-aliasing

endif ENABLE_LINUX

if ENABLE_ECOS
additional_flags += \
	-DECOS -DIFXOS_USE_DEV_IO=1

endif ENABLE_ECOS

if ENABLE_GENERIC
additional_flags += \
	-DGENERIC_OS -DIFXOS_USE_DEV_IO=1

lint_file = std_generic.lnt

endif ENABLE_GENERIC

## add compile flags for Lauterbach Debugger Support
## lauterbach_cflags = -DLAUTERBACH_COMPILE -Wall -Wstrict-prototypes -Wno-trigraphs -O1 -g3 \
## 	-fno-schedule-insns -fno-schedule-insns2 -fno-strict-aliasing -fno-common -DDTI_DEBUG
## AM_CFLAGS += $(lauterbach_cflags)

#
# DTI Protocol Library
lib_dti_protocol_sources = \
	common/dti_connection.c\
	common/dti_protocol.c

lib_dti_protocol_extra_sources = \
	include/dti_connection_interface.h\
	include/ifx_dti_protocol.h\
	include/ifx_dti_protocol_device.h\
	include/ifx_dti_protocol_cli.h\
	include/dti_protocol_interface.h

lib_dti_protocol_a_cflags = $(DTI_TARGET_OPTIONS)

libdti_protocol_a_SOURCES = \
	$(lib_dti_protocol_sources)

libdti_protocol_a_CFLAGS = \
	$(AM_CFLAGS)\
	$(additional_flags)\
	$(optimization_flags)\
	$(lib_dti_protocol_a_cflags)

libdti_protocol_la_SOURCES = \
	$(lib_dti_protocol_sources)

libdti_protocol_la_CFLAGS = \
	$(AM_CFLAGS)\
	$(additional_flags)\
	$(optimization_flags)\
	$(lib_dti_protocol_a_cflags)

#
# Never try to set the interface numbers so that they correspond to the release
# number of your package (see readme).
libdti_protocol_la_LDFLAGS = -version-info 0:0:0

#
# DTI Client Library
lib_dti_client_sources = \
	client_lib/dti_client_packet_protocol.c\
	client_lib/dti_client_packet_device.c\
	client_lib/dti_client_packet_cli.c

lib_dti_client_extra_sources = \
	include/dti_osmap.h\
	include/dti_client_packet_protocol.h\
	include/dti_client_packet_device.h\
	include/dti_client_packet_cli.h

lib_dti_client_a_cflags = $(DTI_TARGET_OPTIONS)

libdti_client_a_SOURCES = \
	$(lib_dti_client_sources) \
	$(lib_dti_protocol_sources)

libdti_client_a_CFLAGS = \
	$(AM_CFLAGS)\
	$(additional_flags)\
	$(optimization_flags)\
	$(lib_dti_client_a_cflags)

libdti_client_la_SOURCES = \
	$(lib_dti_client_sources) \
	$(lib_dti_protocol_sources)

libdti_client_la_CFLAGS = \
	$(AM_CFLAGS)\
	$(additional_flags)\
	$(optimization_flags)\
	$(lib_dti_client_a_cflags)

#
# Never try to set the interface numbers so that they correspond to the release
# number of your package (see readme).
libdti_client_la_LDFLAGS = -version-info 0:0:0

#
# CLI module
dti_cli_sources = \
	cli/dti_cli.c

dti_cli_extra_sources = \
	include/dti_cli_interface.h \
	include/dti_cli.h

#
# DTI agent library
lib_dti_agent_sources = \
	common/dti_agent.c\
	common/dti_control.c\
	common/dti_protocol_ext.c\
	common/dti_packet_device.c\
	common/dti_statistic.c

lib_dti_agent_extra_sources = \
	include/dti_osmap.h\
	include/dti_agent_interface.h\
	include/dti_protocol_ext.h\
	include/dti_device.h\
	include/dti_control.h\
	include/dti_packet_device.h\
	include/dti_statistic.h\
	include/dti_statistic_if.h

lib_dti_agent_a_cflags = $(DTI_TARGET_OPTIONS)

lib_dti_agent_a_sources = \
	$(lib_dti_agent_sources)

if INCLUDE_CLI_SUPPORT
lib_dti_agent_a_cflags += -DINCLUDE_CLI_SUPPORT
lib_dti_agent_a_sources += $(dti_cli_sources)
endif

libdti_agent_a_SOURCES = \
	$(lib_dti_agent_a_sources) \
	$(lib_dti_protocol_sources)

libdti_agent_a_CFLAGS = \
	$(AM_CFLAGS)\
	$(additional_flags)\
	$(optimization_flags)\
	$(lib_dti_agent_a_cflags)

libdti_agent_la_SOURCES = \
	$(lib_dti_agent_a_sources) \
	$(lib_dti_protocol_sources)

libdti_agent_la_CFLAGS = \
	$(AM_CFLAGS)\
	$(additional_flags)\
	$(optimization_flags)\
	$(lib_dti_agent_a_cflags)

#
# Never try to set the interface numbers so that they correspond to the release
# number of your package (see readme).
libdti_agent_la_LDFLAGS = -version-info 0:0:0


dti_device_options =
dti_device_ldadd =

#
# Generic device module
dti_dev_generic_sources = \
	device_generic/dti_dev_generic.c

dti_dev_generic_extra_sources = \
	include/dti_device_generic_ctx.h

lib_dti_device_generic_a_cflags = $(DTI_TARGET_OPTIONS) -DDEVICE_GENERIC

if DEVICE_GENERIC
dti_device_ldadd += -ldti_device_generic
dti_device_options += -DDEVICE_GENERIC

libdti_device_generic_a_SOURCES = \
	$(dti_dev_generic_sources)

libdti_device_generic_a_CFLAGS = \
	$(AM_CFLAGS)\
	$(additional_flags)\
	$(optimization_flags)\
	$(lib_dti_device_generic_a_cflags)

libdti_device_generic_la_SOURCES = \
	$(dti_dev_generic_sources)

libdti_device_generic_la_CFLAGS = \
	$(AM_CFLAGS)\
	$(additional_flags)\
	$(optimization_flags)\
	$(lib_dti_device_generic_a_cflags)

#
# Never try to set the interface numbers so that they correspond to the release
# number of your package (see readme).
libdti_device_generic_la_LDFLAGS = -version-info 0:0:0
endif DEVICE_GENERIC

#
# LINT: collect device specific sources
dti_device_sources = \
	$(dti_dev_generic_sources)

dti_device_extra = \
	$(dti_dev_generic_extra_sources)

dti_device_all_cflags = -DDEVICE_GENERIC


# all sources for the current library distribution
EXTRA_DIST = \
	$(lib_dti_protocol_extra_sources)\
	$(lib_dti_client_extra_sources)\
	$(lib_dti_agent_extra_sources)\
	$(dti_device_sources)\
	$(dti_device_extra)\
	$(dti_cli_extra_sources)

lint:
	flint +b -vm \
	-i$(shell dirname `$(CC) -print-file-name=include`)/include \
	$(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) $(DTI_TARGET_OPTIONS) \
	-i@top_srcdir@ -i@srcdir@ $(lint_file) \
	$(dti_device_sources) \
	$(lib_dti_protocol_sources) \
	$(lib_dti_client_sources) \
	$(lib_dti_agent_sources) \
	$(dti_cli_sources)
