#
#
#                              Copyright (c) 2014
#                            Lantiq Deutschland GmbH
#                             http://www.lantiq.com
#
#  For licensing information, see the file 'LICENSE' in the root folder of
#  this software module.
#
#


if KERNEL_2_6
bin_PROGRAMS = drv_vmmc.ko
# dummy file required for distcheck target
drv_vmmc_ko_SOURCES = @top_srcdir@/LICENSE
else
bin_PROGRAMS = drv_vmmc
endif

## Common VMMC driver sources
drv_vmmc_SOURCES = \
   drv_vmmc_alm.c \
   drv_vmmc_api.c \
   drv_vmmc_bbd.c \
   drv_vmmc_con.c \
   drv_vmmc_cod.c \
   drv_vmmc_event.c \
   drv_vmmc_gr909.c \
   drv_vmmc_init.c \
   drv_vmmc_init_cap.c \
   drv_vmmc_int.c \
   drv_vmmc_ioctl.c \
   drv_vmmc_linux.c \
   drv_vmmc_pcm.c \
   drv_vmmc_announcements.c \
   drv_vmmc_xrx100.c \
   drv_vmmc_res.c \
   drv_vmmc_sig.c \
   drv_vmmc_sig_cid.c \
   drv_vmmc_sig_cptd.c \
   drv_vmmc_sig_dtmfg.c \
   drv_vmmc_sig_mftd.c \
   drv_vmmc_sig_utg.c \
   drv_vmmc_stream.c \
   lib/lib_bbd/src/lib_bbd.c

if DECT_SUPPORT
drv_vmmc_SOURCES +=\
   drv_vmmc_dect.c
endif

if VMMC_WITH_MPS
drv_vmmc_SOURCES +=\
   mps/drv_mps_vmmc_linux.c \
   mps/drv_mps_vmmc_common.c \
   mps/drv_mps_vmmc_crc32.c

if AR9
drv_vmmc_SOURCES +=\
   mps/drv_mps_vmmc_ar9.c
endif

if VR9
drv_vmmc_SOURCES +=\
   mps/drv_mps_vmmc_ar9.c
endif

if XRX300
drv_vmmc_SOURCES +=\
   mps/drv_mps_vmmc_ar9.c
endif

if FALCON
drv_vmmc_SOURCES +=\
   mps/drv_mps_vmmc_falcon.c
endif

endif

if TAPI_PMC
drv_vmmc_SOURCES +=\
   drv_vmmc_pmc.c
endif

nodist_drv_vmmc_SOURCES =

if EVALUATION
nodist_drv_vmmc_SOURCES +=\
   drv_vmmc_eval.c
endif

nodist_EXTRA_drv_vmmc_SOURCES =\
   ../drv_config.h


EXTRA_DIST = \
   ../drv_version.h \
   ../include/vmmc_io.h \
   ../include/drv_vmmc_errno.h \
   ../include/drv_vmmc_strerrno.h \
   drv_api.h \
   drv_vmmc_access.h \
   drv_vmmc_api.h \
   drv_vmmc_bbd.h \
   drv_vmmc_fw_commands.h \
   drv_vmmc_fw_commands_sdd.h \
   drv_vmmc_fw_commands_voip.h \
   drv_vmmc_fw_data.h \
   drv_vmmc_hostapi.h \
   drv_vmmc_init.h \
   drv_vmmc_int.h \
   drv_vmmc_int_evt.h \
   drv_vmmc_linux.h \
   drv_vmmc_pcm.h \
   drv_vmmc_pcm_priv.h \
   drv_vmmc_announcements.h \
   drv_vmmc_alm.h \
   drv_vmmc_alm_priv.h \
   drv_vmmc_cod.h \
   drv_vmmc_cod_priv.h \
   drv_vmmc_res.h \
   drv_vmmc_res_priv.h \
   drv_vmmc_con.h \
   drv_vmmc_con_priv.h \
   drv_vmmc_sig.h \
   drv_vmmc_sig_cid.h \
   drv_vmmc_sig_priv.h \
   drv_vmmc_dect.h \
   drv_vmmc_dect_priv.h \
   drv_vmmc_dect.c \
   drv_vmmc_gr909.h \
   drv_vmmc_stream.h \
   drv_vmmc_ll_if_version.h\
   drv_vmmc_pmc.h \
   drv_vmmc_pmc.c \
   drv_vmmc_osmap.h \
   lib/lib_bbd/src/lib_bbd.h \
   mps/drv_mps_vmmc_linux.c \
   mps/drv_mps_vmmc_common.c \
   mps/drv_mps_vmmc_ar9.c \
   mps/drv_mps_vmmc_crc32.c \
   mps/drv_mps_version.h \
   mps/drv_mps_vmmc_device.h \
   mps/drv_mps_vmmc.h \
   mps/drv_mps_vmmc_dbg.h \
   mps/drv_mps_vmmc_crc32.h \
   mps/drv_mps_vmmc_bsp.h \
   mps/CHANGELOG

INCLUDES = \
   -I@abs_top_srcdir@ \
   -I@abs_top_srcdir@/include \
   -I@abs_top_srcdir@/src \
   -I@abs_top_builddir@ \
   -I@abs_top_srcdir@/src/lib/lib_bbd/src \
   -I@IFXOS_INCL_PATH@

if VMMC_WITH_MPS
INCLUDES +=\
   -I@abs_top_srcdir@/src/mps
endif

if ! KERNEL_2_6
INCLUDES += -I@KERNEL_INCL_PATH@
endif

if HL_TAPI_INCL_PATH_SET
INCLUDES += -I@HL_TAPI_INCL_PATH@ -I@HL_TAPI_INCL_PATH@/include
else
INCLUDES += -I@abs_top_srcdir@/src/drv_tapi/include
endif

if MPS_INCL_PATH_SET
INCLUDES += -I@MPS_INCL_PATH@ -I@MPS_INCL_PATH@/include
else
INCLUDES += -I@abs_top_srcdir@/../drv_mps_vmmc/include
endif

if EVENT_LOGGER_DEBUG
INCLUDES += -I@EVENT_LOGGER_INCL_PATH@
endif

if WARNINGS
AM_CFLAGS = -Wall -Wimplicit -Wreturn-type -Wunused -Wswitch -Wcomment -Wparentheses
# -Wpointer-arith
else
AM_CFLAGS = -Wall
endif

if USE_MODULE
drv_vmmc_CFLAGS =-D__LINUX__ -DLINUX -D__KERNEL__ -DMODULE -DEXPORT_SYMTAB $(AM_CFLAGS)
else
drv_vmmc_CFLAGS =-D__LINUX__ -DLINUX -D__KERNEL__ -DEXPORT_SYMTAB $(AM_CFLAGS)
endif

drv_vmmc_LDFLAGS = -r -nostdlib

# GHS
ghs:
	ccppc $(addprefix @srcdir@/,$(drv_vmmc_SOURCES)) $(INCLUDES) -I@KERNEL_INCL_PATH@ -I@KERNEL_INCL_PATH@/asm/ifx \
	-c -gcc -Wall -bigendian -D__BYTE_ORDER=__BIG_ENDIAN -DBSP_API_VERSION=2 \
	-DHAVE_CONFIG_H -DGENERIC_OS -DGREENHILLS_CHECK

lint:
	@flint ../../std_lx.lnt $(INCLUDES) \
	-I$(shell dirname `$(CC) -print-file-name=include`)/include \
	$(drv_vmmc_SOURCES) $(nodist_drv_vmmc_SOURCES)

rsm:
	rsm -fa -n -s -Ta $(addprefix @abs_srcdir@/,$(drv_vmmc_SOURCES)) $(addprefix @abs_srcdir@/,$(EXTRA_DIST))

install-exec-hook: $(bin_PROGRAMS)
	@for p in $(bin_PROGRAMS); do \
		$(STRIP) --strip-debug $(DESTDIR)$(bindir)/$$p; \
		echo "  $(STRIP) --strip-debug $(DESTDIR)$(bindir)/$$p"; \
	done

# Extra rule for linux-2.6 kernel object
if KERNEL_2_6
drv_vmmc_CFLAGS += -fno-common
drv_vmmc_OBJS = "$(subst .c,.o, $(drv_vmmc_SOURCES) $(nodist_drv_vmmc_SOURCES))"

drv_vmmc.ko: $(drv_vmmc_SOURCES) $(nodist_drv_vmmc_SOURCES) $(nodist_EXTRA_drv_vmmc_SOURCES) $(EXTRA_DIST)
	@echo "Making Linux 2.6.x kernel object"
	@for f in $(drv_vmmc_SOURCES) $(nodist_drv_vmmc_SOURCES) ; do \
		if test ! -e $(PWD)/$$f; then \
			echo "  LN      $$f" ; \
			mkdir -p $(PWD)/`dirname $$f`/; \
			ln -s @abs_srcdir@/$$f $(PWD)/$$f; \
		fi; \
	done;
	@echo "# drv_vmmc: Generated to build Linux 2.6.x kernel object" > $(PWD)/Kbuild
	@echo "obj-m := $(subst .ko,.o,$@)"			>> $(PWD)/Kbuild
	@echo "$(subst .ko,,$@)-y := $(drv_vmmc_OBJS)"	>> $(PWD)/Kbuild
	@echo "EXTRA_CFLAGS := -DHAVE_CONFIG_H  $(drv_vmmc_CFLAGS) $(INCLUDES)"	>> $(PWD)/Kbuild

clean-generic:
	@echo "drv_vmmc: Cleanup Linux 2.6.x kernel object build"
	@- find . -name ".*.cmd" | xargs rm -f
	@- find . -name "*.o" | xargs rm -f
	@- rm -f Module.symvers Kbuild modules.order
	@- rm -rf .tmp_versions drv_vmmc.mod.c

endif
