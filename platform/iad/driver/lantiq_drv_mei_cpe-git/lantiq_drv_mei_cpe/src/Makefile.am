## Process this file with automake to produce Makefile.in

if ENABLE_ECOS

# for eCos build only a library
noinst_LIBRARIES = libmeicpe.a

else

# Target definition depending on kernel version:
if KERNEL_2_6
#noinst_PROGRAMS = drv_mei_cpe
bin_PROGRAMS = drv_mei_cpe.ko
if ADD_INTERNAL_IF_SET
#noinst_PROGRAMS += drv_mei_cpe_intern
bin_PROGRAMS += drv_mei_cpe_intern.ko
endif
else
bin_PROGRAMS = drv_mei_cpe
if ADD_INTERNAL_IF_SET
bin_PROGRAMS += drv_mei_cpe_intern
endif
endif

if MEI_TEST_APPL_ENABLE
bin_PROGRAMS += mei_cpe_drv_test
endif

if ADD_INTERNAL_IF_SET
bin_PROGRAMS += test_mei_cpe_intern
endif

if ADD_SUPPORT_DEBUG_STREAMS
bin_PROGRAMS += mei_cpe_drv_dbg_strm_dmp
endif

bin_SCRIPTS = inst_drv_mei_cpe.sh alias_drv_mei_cpe.sh

# ENABLE_ECOS
endif

if MEI_TEST_APPL_ENABLE
## this is a test program for the MEI CPE driver
mei_cpe_drv_test_SOURCES = \
	mei_cpe_drv_test.c\
	mei_cpe_appl_osmap.h\
	$(mei_cpe_test_appl_os_sources)\
	mei_cpe_drv_test_fct.c\
	mei_cpe_drv_test_fct.h

test_mei_cpe_intern_SOURCES = \
	test_internal/mei_cpe_drv_test_intern.c
endif

if ADD_SUPPORT_DEBUG_STREAMS
mei_cpe_drv_dbg_strm_dmp_SOURCES = \
	debug_stream_dump/mei_cpe_drv_dbg_strm_dmp.c\
	mei_cpe_appl_osmap.h
endif

if IFXOS_ENABLE
AM_CPPFLAGS = \
	-I@srcdir@\
	-I@srcdir@/auto_header\
	-I@KERNEL_INCL_PATH@ $(IFXOS_INCLUDE_PATH)
else
AM_CPPFLAGS = \
	-I@srcdir@\
	-I@srcdir@/auto_header\
	-I@KERNEL_INCL_PATH@
endif

## listed files are to be distributed
drv_mei_cpe_vrx_includedir = $(includedir)
drv_mei_cpe_vrx_include_HEADERS = \
drv_mei_cpe_api_intern.h \
drv_mei_cpe_api_atm_ptm_intern.h \
drv_mei_cpe_interface.h \
drv_mei_cpe_config.h \
cmv_message_format.h

## the driver itself
drv_mei_cpe_all_sources = \
	drv_mei_cpe_common.c\
	drv_mei_cpe_config.h\
	drv_mei_cpe_api.h\
	drv_mei_cpe_interface.h\
	drv_mei_cpe_api_intern.h\
	drv_mei_cpe_api_atm_ptm_intern.h\
	drv_mei_cpe_api_intern.c\
	drv_mei_cpe_api_atm_ptm_intern.c\
	drv_mei_cpe_mei_vrx.h\
	drv_mei_cpe_mei_interface.h\
	drv_mei_cpe_mailbox.h\
	drv_mei_cpe_msg_process.c\
	drv_mei_cpe_msg_process.h\
	drv_mei_cpe_msg_process_ar9.c\
	drv_mei_cpe_msg_map_ar9.c \
	drv_mei_cpe_msg_map_ar9.h \
	drv_mei_cpe_dbg_access.c\
	drv_mei_cpe_dbg_access.h\
	drv_mei_cpe_os.h\
	drv_mei_cpe_dbg.c\
	drv_mei_cpe_dbg.h\
	drv_mei_cpe_dbg_driver.c\
	drv_mei_cpe_dbg_driver.h\
	drv_mei_cpe_linux.c\
	drv_mei_cpe_linux.h\
	drv_mei_cpe_linux_proc_config.c\
	drv_mei_cpe_linux_proc_config.h\
	drv_mei_cpe_vxworks.c\
	drv_mei_cpe_vxworks.h\
	drv_mei_cpe_vxworks_bsp.c\
	drv_mei_cpe_ecos.c\
	drv_mei_cpe_ecos.h\
	drv_mei_cpe_nucleus.c\
	drv_mei_cpe_nucleus.h\
	drv_mei_cpe_mei_access_vrx.c \
	drv_mei_cpe_mei_access_ar9.c \
	drv_mei_cpe_device_vrx.c \
	drv_mei_cpe_rom_handler.c\
	drv_mei_cpe_rom_handler.h\
	drv_mei_cpe_rom_handler_if.h\
	drv_mei_cpe_download.c\
	drv_mei_cpe_download.h\
	drv_mei_cpe_download_vrx.c\
	drv_mei_cpe_download_vrx.h\
	drv_mei_cpe_download_vrx_common.h\
	drv_mei_cpe_dsm.c\
	drv_mei_cpe_dsm.h\
	drv_mei_cpe_dsm_common.h\
	drv_mei_cpe_dbg_streams.c\
	drv_mei_cpe_dbg_streams.h\
	drv_mei_cpe_dbg_streams_common.h\
	cmv_message_format.h\
	drv_mei_cpe_device_cntrl.c\
	drv_mei_cpe_device_cntrl.h\
	drv_mei_cpe_driver_msg.c\
	drv_mei_cpe_driver_msg.h\
	auto_header/user_if_vdsl2_boot_messages.h\
	drv_mei_cpe_config_auto.h.in

drv_mei_cpe_atmoam_access_sources = \
	drv_mei_cpe_atmoam.h\
	drv_mei_cpe_atmoam_common.c\
	drv_mei_cpe_atmoam_common.h\
	drv_if_vdsl2_atm_oam.h

drv_mei_cpe_cleareoc_access_sources = \
	drv_mei_cpe_clear_eoc.h\
	drv_mei_cpe_clear_eoc_common.c\
	drv_mei_cpe_clear_eoc_common.h\
	drv_if_vdsl2_clear_eoc.h

drv_mei_cpe_mei_access_sources = \
	drv_mei_cpe_mei_access.h

drv_mei_cpe_mei_access_ikos_sources = \
	drv_mei_cpe_mei_access_ikos.c \
	drv_mei_cpe_mei_access_ikos.h

drv_mei_cpe_mei_access_sw_emul_sources = \
	drv_mei_cpe_mei_access_sw_emul.c \
	drv_mei_cpe_mei_access_sw_emul.h

drv_mei_cpe_modem_test_sources = \
	drv_mei_cpe_modem_test.c \
	drv_mei_cpe_modem_test.h

## without any emulation
drv_mei_cpe_SOURCES = $(drv_mei_cpe_all_sources) \
				$(drv_mei_cpe_mei_access_sources)\
				$(drv_mei_cpe_atmoam_access_sources)\
				$(drv_mei_cpe_cleareoc_access_sources)

libmeicpe_a_SOURCES = $(drv_mei_cpe_all_sources) \
				$(drv_mei_cpe_mei_access_sources)\
				$(drv_mei_cpe_atmoam_access_sources)\
				$(drv_mei_cpe_cleareoc_access_sources)

if ADD_DRV_MODEM_TESTS_SET
nodist_drv_mei_cpe_SOURCES = $(drv_mei_cpe_modem_test_sources)
endif


## seperate module for internal interface testing
drv_mei_cpe_intern_sources = \
	drv_mei_cpe_dbg.h\
	drv_mei_cpe_dbg.c\
	test_internal/drv_test_mei_cpe_linux.c\
	test_internal/drv_test_mei_cpe_linux.h\
	test_internal/drv_test_mei_cpe_interface.h

drv_mei_cpe_intern_SOURCES = $(drv_mei_cpe_intern_sources)

AM_CFLAGS = -Wall -Wimplicit -Wunused -Wswitch -Wcomment \
		-Wuninitialized -Wparentheses -Wreturn-type

if IFXOS_ENABLE
AM_LDFLAGS= \
	-Bstatic -dn -static @IFXOS_LIBRARY_PATH@
else
AM_LDFLAGS= \
	-Bstatic -dn -static
endif

#
# common driver cflags
if ENABLE_ECOS

drv_mei_cpe_common_cflags = $(AM_CFLAGS) -DECOS -DIFXOS_USE_DEV_IO=1

else

drv_mei_cpe_common_cflags = \
	$(AM_CFLAGS) -DLINUX -D__linux__ \
	-Wall -Wstrict-aliasing -Wno-undef

endif


#
# target cflags
if ADD_DRV_CFLAGS_SET

drv_mei_cpe_target_cflags= $(ADD_DRV_CFLAGS)

else

if TARGET_ADM5120_MIPSEL

#
# cflags for the IFX VDSL2 ADM5120 target
#
drv_mei_cpe_target_cflags = -O2 -mlong-calls \
	-fomit-frame-pointer -fno-strict-aliasing \
	-fno-common -finline-limit=5000 \
	-G 0 -mno-abicalls -fno-pic -mcpu=r4600  -mips2 -Wa,--trap \
	-D_MIPS_ISA=6 -D_MIPS_SZLONG=32 -D_MIPS_SZPTR=32 -D_MIPS_SZINT=32 -D__MIPSEL__

else

#
# default cflags for the IFX VDSL2 targets
#
drv_mei_cpe_target_cflags = -O1 -s -Winline

# Lauterbach
# drv_mei_cpe_target_cflags = -Wall -Wstrict-prototypes -Wno-trigraphs -O1 -g3 -fno-strict-aliasing -fno-common -DMEI_DEBUG

if !KERNEL_2_6
# the headerfile of linux kernels 2.6.x contain to much arithmetic
# with void pointers (which is allowed for gcc!)
drv_mei_cpe_common_cflags += -Wpointer-arith
endif
endif
endif

if CONFIG_USE_MEI_ADD_EXT_DRV_CFLAGS
drv_mei_cpe_external_cflags= $(MEI_ADD_EXT_DRV_CFLAGS)
endif

drv_mei_cpe_compile_flags = -DMEI_INTERN -DMEI_MEI_SW_EMUL=0 -DIKOS_EMULATION=0 -DIKOS_BUSY_TIMEOUT=0\
	$(MEI_DRV_TARGET_OPTIONS)

drv_mei_cpe_CFLAGS =  $(drv_mei_cpe_common_cflags) \
				$(drv_mei_cpe_target_cflags) \
				$(drv_mei_cpe_compile_flags) \
				$(drv_mei_cpe_external_cflags)

libmeicpe_a_CFLAGS = $(drv_mei_cpe_common_cflags) \
				$(drv_mei_cpe_target_cflags) \
				$(drv_mei_cpe_compile_flags) \
				$(drv_mei_cpe_external_cflags)

drv_mei_cpe_LDFLAGS = -r -nostdlib

drv_mei_cpe_intern_CFLAGS = $(drv_mei_cpe_common_cflags) \
				$(drv_mei_cpe_target_cflags) \
				$(drv_mei_cpe_compile_flags) \
				$(drv_mei_cpe_external_cflags)

drv_mei_cpe_intern_LDFLAGS = -r -nostdlib


#
# Common flags for compiling a application
#
mei_cpe_app_common_cflags = -DLINUX -ansi -Wall -Wimplicit -Wreturn-type -Wunused -Wswitch -Wcomment -Wuninitialized -s

if ADD_APPL_CFLAGS_SET
mei_cpe_appl_cflags= $(ADD_APPL_CFLAGS)
else
if TARGET_ADM5120_MIPSEL
mei_cpe_appl_cflags =  -O1 -g
else
mei_cpe_appl_cflags =  -DPPC
endif
endif

if ADD_APPL_LDFLAGS_SET
mei_cpe_appl_ldflags= $(ADD_APPL_LDFLAGS)
else
if TARGET_ADM5120_MIPSEL
mei_cpe_appl_cflags =  -O1 -g
mei_cpe_appl_ldflags = -static
else
mei_cpe_appl_cflags =  -DPPC
endif
endif

if MEI_TEST_APPL_ENABLE
mei_cpe_drv_test_CPPFLAGS = -I@srcdir@\
				-I@srcdir@/auto_header\
				-I@KERNEL_INCL_PATH@ $(IFXOS_INCLUDE_PATH)

mei_cpe_drv_test_CFLAGS = $(mei_cpe_app_common_cflags) \
				$(mei_cpe_appl_cflags) $(MEI_DRV_TARGET_OPTIONS)

mei_cpe_drv_test_LDFLAGS = $(mei_cpe_appl_ldflags) -Bstatic -dn -static @IFXOS_LIBRARY_PATH@

mei_cpe_drv_test_LDADD = -lifxos
endif

test_mei_cpe_intern_CFLAGS = $(mei_cpe_app_common_cflags) \
					$(mei_cpe_appl_cflags)
test_mei_cpe_intern_LDFLAGS =$(mei_cpe_appl_ldflags)
test_mei_cpe_intern_LDADD = -lifxos

mei_cpe_drv_dbg_strm_dmp_CPPFLAGS = -I@srcdir@\
				-I@srcdir@/auto_header\
				-I@KERNEL_INCL_PATH@ $(IFXOS_INCLUDE_PATH)
mei_cpe_drv_dbg_strm_dmp_CFLAGS = $(mei_cpe_app_common_cflags) \
				$(mei_cpe_appl_cflags) $(MEI_DRV_TARGET_OPTIONS)
mei_cpe_drv_dbg_strm_dmp_LDFLAGS = $(mei_cpe_appl_ldflags) -Bstatic -dn -static @IFXOS_LIBRARY_PATH@
mei_cpe_drv_dbg_strm_dmp_LDADD = -lifxos

# linux 2.6 kernel object - dummy to force dependencies
drv_mei_cpe_ko_SOURCES = ../ChangeLog
drv_mei_cpe_intern_ko_SOURCES = ../ChangeLog

# Extra rule for linux-2.6 kernel object
if KERNEL_2_6
drv_mei_cpe_OBJS = "$(subst .c,.o,$(filter %.c,$(drv_mei_cpe_SOURCES)))"

drv_mei_cpe.ko: $(drv_mei_cpe_SOURCES)
	@echo -e "drv_mei_cpe: Making Linux 2.6.x kernel object"
	@if test ! -e drv_mei_cpe_common.c ; then \
		echo "copy source files (as links only!)"; \
		cp -s $(filter %.c,$(addprefix @abs_srcdir@/,$(drv_mei_cpe_SOURCES))) $(PWD)/ ; \
	fi
	@echo -e "# drv_mei_cpe: Generated to build Linux 2.6.x kernel object" > $(PWD)/Kbuild
	@echo -e "obj-m := $(subst .ko,.o,$@)"			>> $(PWD)/Kbuild
	@echo -e "$(subst .ko,,$@)-y := $(drv_mei_cpe_OBJS)"	>> $(PWD)/Kbuild
	@echo -e "EXTRA_CFLAGS := $(drv_mei_cpe_CFLAGS) -I@abs_srcdir@ -I@abs_srcdir@/auto_header $(IFXOS_INCLUDE_PATH)" >> $(PWD)/Kbuild

clean-generic:
	@echo "drv_mei_cpe: Cleanup Linux 2.6.x kernel object build"
	@- find . -name ".*.cmd" | xargs rm -f
	@- find . -name "*.o" | xargs rm -f
	@- rm -f Module.symvers Kbuild
	@- rm -rf .tmp_versions *.mod.c *.order

endif

## add extra files to distribution
EXTRA_DIST = \
	drv_mei_cpe_vxworks.c\
	drv_mei_cpe_vxworks.h\
	drv_mei_cpe_vxworks_bsp.c\
	drv_mei_cpe_generic_proc.c\
	drv_mei_cpe_generic_proc.h\
	drv_mei_cpe_ecos.c\
	drv_mei_cpe_ecos.h\
	drv_mei_cpe_nucleus.c\
	drv_mei_cpe_nucleus.h\
	mei_cpe_drv_test_vxworks.c\
	mei_cpe_drv_examp_int_wrap_vxw.c\
	mei_cpe_drv_examp_int_wrap_vxw.h\
	../build_vxworks/drv_mei_cpe.wpj\
	inst_drv_mei_cpe.sh\
	alias_drv_mei_cpe.sh\
	../scripts/enable_ARC_JTAG.sh\
	../scripts/inst_drv_mei_cpe.sh\
	../scripts/ifx_mei_cpe_drv_init.sh\
   ../scripts/alias_drv_mei_cpe.sh

