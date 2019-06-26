## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign 1.6 nostdinc

SUBDIRS = src

DISTCHECK_CONFIGURE_FLAGS=@CONFIGURE_OPTIONS@

lib_ifxos_docdir = ${prefix}/doc/lib_ifxos

lib_ifxos_doc_DATA = \
	ChangeLog

EXTRA_DIST = $(lib_ifxos_doc_DATA) \
	LICENSE\
	build_vxworks/lib_ifxos.wpj\
	build_vxworks/lib_ifxos_xapi.wpj\
	build_win32/vc7/lib_ifxos.vcproj\
	build_win32/eclipse_juno/.cproject\
	build_win32/eclipse_juno/.project\
	build_win32/eclipse_juno/makefile.win32\
   zlib_license.txt \
	autogen.sh\
	doc/ifxos_all.dxcfg\
	doc/ifxos_interface.dxcfg\
	doc/ifxos_linux.dxcfg\
	doc/ifxos_vxworks.dxcfg\
	doc/ifxos_ecos.dxcfg\
	doc/ifxos_nucleus.dxcfg\
	doc/footer.html\
	doc/header.html\
	doc/logo.gif\
	doc/html/logo.gif\
	doc/stylesheet.css \
	vxworks_6.4/.project \
	vxworks_6.4/.wrmakefile \
	vxworks_6.4/.wrproject \
	vxworks_6.6/.cproject \
	vxworks_6.6/.project \
	vxworks_6.6/.wrmakefile \
	vxworks_6.6/.wrproject

# Copy all the spec files. Of cource, only one is actually used.
dist-hook:
	for specfile in *.spec; do \
		if test -f $$specfile; then \
			cp -p $$specfile $(distdir); \
		fi \
	done

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
