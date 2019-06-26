#
#
#                              Copyright (c) 2013
#                            Lantiq Deutschland GmbH
#                             http://www.lantiq.com
#
#  For licensing information, see the file 'LICENSE' in the root folder of
#  this software module.
#
#


AUTOMAKE_OPTIONS = foreign 1.7 nostdinc
ACLOCAL_AMFLAGS = -I m4

SUBDIRS = src

# set distcheck options - see configure.in for list of modifications
DISTCHECK_CONFIGURE_FLAGS=@CONFIGURE_OPTIONS@
CONFIG_CLEAN_FILES=src/modules.order
drv_vmmc_docdir = ${prefix}/doc/drv_vmmc

drv_vmmc_doc_DATA = \
	README\
	AUTHORS\
	LICENSE

EXTRA_DIST = $(drv_vmmc_doc_DATA)\
	CHANGELOG\
	doc/lantiq,mps-xrx100.txt\
	doc/doxyconfig\
	doc/footer.html\
	doc/header.html\
	doc/logo.jpg\
	doc/html/logo.jpg\
	doc/stylesheet.css

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
	makeself.sh --gzip --notemp $(distdir) drv_vmmc-${PACKAGE_VERSION}.sh \
	"${PACKAGE_NAME}"

distvob:
	cleartool mkact -nc distvob_drv_vmmc_${PACKAGE_VERSION}_`date +"%H%M"`
	@if test `\ls /var/vob/comacsd/comacsd_dist/ifx-voice-vmmc/drv_vmmc*.tar.gz` != "/var/vob/comacsd/comacsd_dist/ifx-voice-vmmc/drv_vmmc-${PACKAGE_VERSION}.tar.gz"; then\
	   cleartool co -nc /var/vob/comacsd/comacsd_dist/ifx-voice-vmmc; \
	   cleartool mv `\ls /var/vob/comacsd/comacsd_dist/ifx-voice-vmmc/drv_vmmc*.tar.gz` /var/vob/comacsd/comacsd_dist/ifx-voice-vmmc/drv_vmmc-${PACKAGE_VERSION}.tar.gz; \
	   cleartool ci -nc /var/vob/comacsd/comacsd_dist/ifx-voice-vmmc; \
	fi;
	cleartool co -nc /var/vob/comacsd/comacsd_dist/ifx-voice-vmmc/drv_vmmc-${PACKAGE_VERSION}.tar.gz
	cp drv_vmmc-${PACKAGE_VERSION}.tar.gz /var/vob/comacsd/comacsd_dist/ifx-voice-vmmc
	cleartool ci -nc /var/vob/comacsd/comacsd_dist/ifx-voice-vmmc/drv_vmmc-${PACKAGE_VERSION}.tar.gz

ghs:
	cd src && make ghs

lint:
	@target=lint; \
    list='$(SUBDIRS)'; \
    for subdir in $$list; do \
       echo "Making $$target in $$subdir"; \
       if test "$$subdir" = "."; then \
          dot_seen=yes; \
          local_target="$$target-am"; \
       else \
          local_target="$$target"; \
       fi; \
       (cd $$subdir && $(MAKE) $(AM_MAKEFLAGS) $$local_target) \
       || eval $$failcom; \
     done;

rsm:
	@target=rsm; \
    list='$(SUBDIRS)'; \
    for subdir in $$list; do \
       echo "Making $$target in $$subdir"; \
       if test "$$subdir" = "."; then \
          dot_seen=yes; \
          local_target="$$target-am"; \
       else \
          local_target="$$target"; \
       fi; \
       (cd $$subdir && $(MAKE) $(AM_MAKEFLAGS) $$local_target) \
       || eval $$failcom; \
     done;

errno:
	@top_srcdir@/script/updateErrno.pl @top_srcdir@/include/drv_vmmc_errno.h @top_srcdir@/include/drv_vmmc_strerrno.h @top_srcdir@/src
