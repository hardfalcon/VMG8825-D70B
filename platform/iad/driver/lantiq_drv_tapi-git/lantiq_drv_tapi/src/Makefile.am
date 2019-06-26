#
#
#                              Copyright (c) 2014
#                            Lantiq Deutschland GmbH
#
#  For licensing information, see the file 'LICENSE' in the root folder of
#  this software module.
#
#
if LINUX
if WITH_KERNEL_MODULE
if KERNEL_2_6
bin_PROGRAMS = drv_tapi.ko
# dummy file required for distcheck target
drv_tapi_ko_SOURCES = @top_srcdir@/LICENSE
else
bin_PROGRAMS = drv_tapi
endif KERNEL_2_6_FALSE
else
lib_LIBRARIES = libdrvtapi.a
endif WITH_KERNEL_MODULE_FALSE
endif LINUX

## the driver itself
drv_tapi_common_SOURCES = \
    drv_tapi_dial.c\
    drv_tapi_event.c\
    drv_tapi_debug.c\
    drv_tapi_ioctl.c\
    drv_tapi_meter.c\
    drv_tapi_misc.c\
    drv_tapi_opcontrol.c\
    drv_tapi_ppd.c\
    drv_tapi_pcm.c\
    drv_tapi_ring.c\
    drv_tapi_tone.c\
    drv_tapi_cid.c\
    drv_tapi_voice.c\
    drv_tapi_init.c\
    drv_tapi_stream.c\
    drv_tapi_polling.c\
    drv_tapi_fxo.c\
    drv_tapi_stat.c\
    drv_tapi_srtp.c\
    lib/lib_fifo.c\
    lib/lib_bufferpool.c

if TAPI_SRTP
drv_tapi_common_SOURCES +=\
    drv_tapi_srtp.c
endif TAPI_SRTP

if LINUX
drv_tapi_common_SOURCES += \
    drv_tapi_linux.c
else
if WIN32
drv_tapi_common_SOURCES += \
    drv_tapi_win.c
endif WIN32
endif LINUX_FALSE

nodist_EXTRA_drv_tapi_SOURCES = \
    ../include/drv_tapi_autoconf.h

drv_tapi_SOURCES=$(drv_tapi_common_SOURCES)

if LINUX
if WITH_KERNEL_MODULE
# NOTE!
# build target specific source files must be added to the extra_dist
# section in addition !
if QOS_SUPPORT
drv_tapi_SOURCES +=\
    drv_tapi_qos.c
endif QOS_SUPPORT

if KPI_SUPPORT
drv_tapi_SOURCES +=\
    drv_tapi_kpi.c
endif KPI_SUPPORT
else
# library sources
libdrvtapi_a_SOURCES = \
    $(drv_tapi_common_SOURCES) \
    drv_tapi_dev_io.c
endif WITH_KERNEL_MODULE_FALSE
endif LINUX

EXTRA_DIST = \
    ../include/drv_tapi_io.h\
    ../include/drv_tapi_io_indexes.h\
    ../include/drv_tapi_kio.h\
    ../include/drv_tapi_dev_io.h\
    ../include/drv_tapi_event_io.h\
    ../include/drv_tapi_kpi_io.h\
    ../include/drv_tapi_qos_io.h\
    ../include/drv_tapi_qos_io_indexes.h\
    ../include/ifx_udp_redirect.h\
    ../include/drv_tapi_ll_interface.h\
    ../include/drv_tapi_fxo_ll_interface.h\
    ../include/drv_tapi_qos_ll_interface.h\
    ../include/drv_tapi_strerrno.h\
    ../include/drv_tapi_errno.h\
    ../include/lib_fifo.h\
    ../include/lib_bufferpool.h\
    ../include/drv_tapi_if_version.v3\
    ../include/drv_tapi_if_version.v4\
    ../include/drv_tapi_config.h\
    drv_tapi_osmap.h\
    drv_tapi.h\
    drv_tapi_ioctl.h\
    drv_tapi_ioctl_handlers.h\
    drv_tapi_api.h\
    drv_tapi_cid.h\
    drv_tapi_event.h\
    drv_tapi_linux.h\
    drv_tapi_stream.h\
    drv_tapi_polling.h\
    drv_tapi_vxworks.h\
    drv_tapi_win.h\
    drv_tapi_vxworks.c\
    drv_tapi_vxworks_bsp.c\
    drv_tapi_ll_if_version.h\
    drv_tapi_kpi.h\
    drv_tapi_kpi.c\
    drv_tapi_srtp.h\
    drv_tapi_srtp.c\
    drv_tapi_qos.h\
    drv_tapi_qos.c\
    drv_tapi_debug.h\
    drv_tapi_fxo.h\
    drv_tapi_ppd.h\
    drv_tapi_ppd.c\
    drv_tapi.wpj

if !LINUX
EXTRA_DIST += \
    drv_tapi_linux.c
endif !LINUX
if !WIN32
EXTRA_DIST += \
    drv_tapi_win.c
endif !WIN32

libdrvtapi_additional_cflags=
if LINUX
libdrvtapi_additional_cflags+=\
    -DLINUX
else
if WIN32
libdrvtapi_additional_cflags+=\
    -DWIN32 \
    -DWINDOWS
endif
endif

libdrvtapi_additional_cflags+=\
    -DRW_MULTI_THREAD \
    -D_REENTRANT \
    -D_THREAD_SAFE \
    -D_POSIX_SOURCE -D_GNU_SOURCE \
    -DIFXOS_USE_DEV_IO=1

INCLUDES  = -I@abs_srcdir@ \
            -I@abs_top_srcdir@ \
            -I@abs_top_builddir@ \
            -I@abs_top_srcdir@/include \
            -I@abs_top_builddir@/include \
            -I@IFXOS_INCL_PATH@

if WIN32
INCLUDES += -I@abs_top_srcdir@/build_win32
endif

if EVENT_LOGGER_DEBUG
INCLUDES += -I@EVENT_LOGGER_INCL_PATH@
endif

if SRTP_LIB_INCL
INCLUDES += -I@SRTP_INCL_PATH@
endif

if LINUX
if WITH_KERNEL_MODULE

if KERNEL_2_6
INCLUDES += -I@KERNEL_INCL_PATH@ \
            -I@KERNEL_INCL_PATH@/asm/mach-generic \
            -I@KERNEL_BUILD_PATH@/include \
            -I@KERNEL_BUILD_PATH@/include2

else
INCLUDES += -I@KERNEL_INCL_PATH@
endif KERNEL_2_6_FALSE

else
## provide additional includes for library
endif WITH_KERNEL_MODULE_FALSE
endif LINUX

if WARNINGS
AM_CFLAGS = -Wall -Wimplicit -Wreturn-type -Wunused -Wswitch -Wcomment -Wparentheses
# -Wpointer-arith
else
AM_CFLAGS = -Wall
endif

if LINUX
if WITH_KERNEL_MODULE
## flags for the driver
drv_tapi_CFLAGS = -DLINUX -D__KERNEL__  -DEXPORT_SYMTAB \
	$(AM_CFLAGS) -DTAPI_HAVE_TIMERS -DMODULE

if KERNEL_2_6
drv_tapi_CFLAGS += -fno-common
endif KERNEL_2_6

drv_tapi_LDFLAGS = -r -nostdlib
endif WITH_KERNEL_MODULE
endif LINUX
# -H
ghs:
	ccppc $(addprefix @srcdir@/,$(drv_tapi_SOURCES)) $(INCLUDES) \
	-c -gcc -Wall -bigendian -D__BYTE_ORDER=__BIG_ENDIAN \
	-DHAVE_CONFIG_H -DGENERIC_OS -DGREENHILLS_CHECK -DTAPI_HAVE_TIMERS

lint:
	@flint @top_srcdir@/std_lx.lnt $(INCLUDES) \
	-I$(shell dirname `$(CC) -print-file-name=include`)/include \
	$(drv_tapi_SOURCES)

rsm:
	rsm -fa -n -s -Ta $(addprefix @srcdir@/,$(drv_tapi_common_SOURCES)) \
	$(addprefix @srcdir@/,$(EXTRA_DIST))

install-exec-hook: $(bin_PROGRAMS)
	@for p in $(bin_PROGRAMS); do \
		$(STRIP) --strip-debug $(DESTDIR)$(bindir)/$$p; \
		echo "  $(STRIP) --strip-debug $(DESTDIR)$(bindir)/$$p"; \
	done

if LINUX
if WITH_KERNEL_MODULE

# Extra rule for linux-2.6 kernel object
if KERNEL_2_6
drv_tapi_OBJS = "$(subst .c,.o, $(drv_tapi_SOURCES))"

drv_tapi.ko: $(nodist_EXTRA_drv_tapi_SOURCES) $(drv_tapi_SOURCES) $(EXTRA_DIST)
	@echo "Making Linux 2.6.x kernel object"
	@for f in $(drv_tapi_SOURCES) ; do \
		if test ! -e $(PWD)/$$f; then \
			echo "  LN      $$f" ; \
			mkdir -p $(PWD)/`dirname $$f`/; \
			ln -s @abs_srcdir@/$$f $(PWD)/$$f; \
		fi; \
	done;
	@echo "# drv_tapi: Generated to build Linux 2.6.x kernel object" > $(PWD)/Kbuild
	@echo "obj-m := $(subst .ko,.o,$@)"			>> $(PWD)/Kbuild
	@echo "$(subst .ko,,$@)-y := $(drv_tapi_OBJS)"	>> $(PWD)/Kbuild
	@echo "EXTRA_CFLAGS := -DHAVE_CONFIG_H  $(drv_tapi_CFLAGS) $(INCLUDES)"	>> $(PWD)/Kbuild

clean-generic:
	@echo "drv_tapi: Cleanup Linux 2.6.x kernel object build"
	@- find . -name ".*.cmd" | xargs rm -f
	@- find . -name "*.o" | xargs rm -f
	@- rm -f Module.symvers Kbuild modules.order
	@- rm -rf .tmp_versions drv_tapi.mod.c
endif KERNEL_2_6
else
# library flags
libdrvtapi_a_CFLAGS = -DTAPI_LIBRARY -DLINUX -DTAPI_HAVE_TIMERS \
    $(AM_CFLAGS) \
    $(libdrvtapi_additional_cflags)

endif WITH_KERNEL_MODULE_FALSE
endif LINUX
