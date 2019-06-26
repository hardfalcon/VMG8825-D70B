## Process this file with automake to produce Makefile.in

# static libraries
noinst_LIBRARIES = \
	libdti_agent_main.a

if ENABLE_LINUX
if BUILD_DTI_AGENT
bin_PROGRAMS = dti_agent_main
endif
endif


AM_CPPFLAGS = \
	-I@srcdir@\
	$(TARGET_INCLUDE_PATH) \
	$(IFXOS_INCLUDE_PATH) $(DTI_INCLUDE_PATH)

AM_LDFLAGS = @IFXOS_LIBRARY_PATH@ -L@top_builddir@/src

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

if DEVICE_GENERIC
dti_device_ldadd = -ldti_device_generic
dti_device_options = -DDEVICE_GENERIC
endif DEVICE_GENERIC


#
# DTI Agent main library
libdti_agent_main_sources = \
	dti_agent_main.c

libdti_agent_main_extra_sources = \
	dti_agent_main.h

libdti_agent_main_a_SOURCES = \
	$(libdti_agent_main_sources)

libdti_agent_main_a_cflags = $(DTI_TARGET_OPTIONS) $(dti_device_options) -DDTI_LIBRARY

libdti_agent_main_a_CFLAGS = \
	$(AM_CFLAGS)\
	$(additional_flags)\
	$(optimization_flags)\
	$(libdti_agent_main_a_cflags)



if ENABLE_LINUX
if BUILD_DTI_AGENT

# DTI Agent Standalone
dti_agent_main_cflags = $(DTI_TARGET_OPTIONS) $(dti_device_options)

if HAVE_IFXOS_SYSOBJ_SUPPORT
dti_agent_main_cflags += -DHAVE_IFXOS_SYSOBJ_SUPPORT=1
endif

dti_agent_main_CFLAGS = \
	$(AM_CFLAGS)\
	$(additional_flags)\
	$(optimization_flags)\
	$(dti_agent_main_cflags)


dti_agent_main_LDADD = \
	$(dti_device_ldadd) \
	-ldti_agent \
	-lifxos \
	-lpthread \
	-lrt

dti_agent_main_SOURCES = \
	dti_agent_main.c

endif BUILD_DTI_AGENT
endif ENABLE_LINUX


# all sources for the current library distribution
EXTRA_DIST = \
	$(libdti_agent_main_extra_sources)

lint:
	flint +b -vm \
	-i$(shell dirname `$(CC) -print-file-name=include`)/include \
	$(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) $(DTI_TARGET_OPTIONS) \
	-i@top_srcdir@ -i@srcdir@ $(lint_file) \
	$(dti_agent_main_SOURCES)

