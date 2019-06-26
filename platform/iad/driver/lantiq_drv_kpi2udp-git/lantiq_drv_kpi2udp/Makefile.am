#
#
#                              Copyright (c) 2013
#                            Lantiq Deutschland GmbH
#                     Am Campeon 3; 85579 Neubiberg, Germany
#
#  For licensing information, see the file 'LICENSE' in the root folder of
#  this software module.
#
#

AUTOMAKE_OPTIONS = foreign 1.7 nostdinc
ACLOCAL_AMFLAGS = -I m4

# set distcheck options - see configure.in for list of modifications
DISTCHECK_CONFIGURE_FLAGS=@CONFIGURE_OPTIONS@
CONFIG_CLEAN_FILES=modules.order
drv_kpi2udp_docdir = ${prefix}/doc/drv_kpi2udp

drv_kpi2udp_doc_DATA = \
	README\
	AUTHORS\
	LICENSE

EXTRA_DIST = $(drv_kpi2udp_doc_DATA)\
   CHANGELOG \
   drv_version.h

if KERNEL_2_6
bin_PROGRAMS = drv_kpi2udp.ko
# dummy file required for distcheck target
drv_kpi2udp_ko_SOURCES = @top_srcdir@/LICENSE
else
bin_PROGRAMS = drv_kpi2udp
endif

## Common KPI2UDP driver sources
drv_kpi2udp_SOURCES = \
   ifx_udp_redirect.c

nodist_EXTRA_drv_kpi2udp_SOURCES =\
   drv_config.h

INCLUDES = \
   -I@abs_top_srcdir@ \
   -I@abs_top_builddir@ \
	-I@IFXOS_INCL_PATH@

if ! KERNEL_2_6
INCLUDES += -I@KERNEL_INCL_PATH@
endif

if HL_TAPI_INCL_PATH_SET
INCLUDES += -I@HL_TAPI_INCL_PATH@ -I@HL_TAPI_INCL_PATH@/include
else
INCLUDES += -I@abs_top_srcdir@/../drv_tapi/include
endif

if WARNINGS
AM_CFLAGS = -Wall -Wimplicit -Wreturn-type -Wunused -Wswitch -Wcomment -Wparentheses
else
AM_CFLAGS = -Wall
endif

drv_kpi2udp_CFLAGS =-DLINUX $(CFLAGS) $(AM_CFLAGS)

drv_kpi2udp_LDFLAGS = -r -nostdlib

install-exec-hook: $(bin_PROGRAMS)
	@for p in $(bin_PROGRAMS); do \
		$(STRIP) --strip-debug $(DESTDIR)$(bindir)/$$p; \
		echo "  $(STRIP) --strip-debug $(DESTDIR)$(bindir)/$$p"; \
	done

# Extra rule for linux-2.6 kernel object
if KERNEL_2_6
drv_kpi2udp_CFLAGS += -fno-common
drv_kpi2udp_OBJS = "$(subst .c,.o, $(drv_kpi2udp_SOURCES))"

drv_kpi2udp.ko: $(drv_kpi2udp_SOURCES) $(nodist_EXTRA_drv_kpi2udp_SOURCES) $(EXTRA_DIST)
	@echo -e "Making Linux 2.6.x kernel object"
	@for f in $(drv_kpi2udp_SOURCES) ; do \
		if test ! -e $(top_builddir)/$$f; then \
			echo "  LN      $$f" ; \
			mkdir -p $(PWD)/`dirname $$f`/; \
			ln -s @abs_srcdir@/$$f $(top_builddir)/$$f; \
		fi; \
	done;
	@echo -e "# drv_kpi2udp: Generated to build Linux 2.6.x kernel object" > $(top_builddir)/Kbuild
	@echo -e "obj-m := $(subst .ko,.o,$@)"			>> $(top_builddir)/Kbuild
	@echo -e "$(subst .ko,,$@)-y := $(drv_kpi2udp_OBJS)"	>> $(top_builddir)/Kbuild
	@echo -e "EXTRA_CFLAGS := -DHAVE_CONFIG_H  $(drv_kpi2udp_CFLAGS) $(INCLUDES)"	>> $(top_builddir)/Kbuild

clean-generic:
	@echo "drv_kpi2udp: Cleanup Linux 2.6.x kernel object build"
	@- find . -name ".*.cmd" | xargs rm -f
	@- find . -name "*.o" | xargs rm -f
	@- rm -f Module.symvers Kbuild modules.order
	@- rm -rf .tmp_versions drv_kpi2udp.mod.c

endif

# Make subdirectories inside tar.gz writable for everyone
# (required for 2.6 kbuild)
dist-hook:
	find $(distdir) -type d -exec chmod 777 {} \; -print

# Create self extracting linux distribution
distcheck-hook:
	@echo -n "Checking for the line ending style... "; \
	failure_list="`find $(distdir) -path '$(distdir)/doc' -prune -o -type f -exec grep -Pqsc $$'\r\n' {} \; -printf '\tDOS line ending style: %p\n'`"; \
	if test -z "$$failure_list"; then \
		echo "Ok"; \
	else \
		echo "Fail"; \
		echo "$$failure_list"; \
		exit -1; \
	fi
	makeself.sh --gzip --notemp $(distdir) drv_kpi2udp-${PACKAGE_VERSION}.sh \
	"${PACKAGE_NAME}"

distvob:
	cleartool mkact -nc distvob_drv_tapi_kpi2udp_${PACKAGE_VERSION}_`date +"%H%M"`
	@if test `\ls /var/vob/comacsd/comacsd_dist/ifx-voice-tapi-kpi2udp/*.tar.gz` != "/var/vob/comacsd/comacsd_dist/ifx-voice-tapi-kpi2udp/drv_kpi2udp-${PACKAGE_VERSION}.tar.gz"; then\
	   cleartool co -nc /var/vob/comacsd/comacsd_dist/ifx-voice-tapi-kpi2udp; \
	   cleartool mv `\ls /var/vob/comacsd/comacsd_dist/ifx-voice-tapi-kpi2udp/*.tar.gz` /var/vob/comacsd/comacsd_dist/ifx-voice-tapi-kpi2udp/drv_kpi2udp-${PACKAGE_VERSION}.tar.gz; \
	   cleartool ci -nc /var/vob/comacsd/comacsd_dist/ifx-voice-tapi-kpi2udp; \
	fi;
	cleartool co -nc /var/vob/comacsd/comacsd_dist/ifx-voice-tapi-kpi2udp/drv_kpi2udp-${PACKAGE_VERSION}.tar.gz
	cp drv_kpi2udp-${PACKAGE_VERSION}.tar.gz /var/vob/comacsd/comacsd_dist/ifx-voice-tapi-kpi2udp
	cleartool ci -nc /var/vob/comacsd/comacsd_dist/ifx-voice-tapi-kpi2udp/drv_kpi2udp-${PACKAGE_VERSION}.tar.gz
