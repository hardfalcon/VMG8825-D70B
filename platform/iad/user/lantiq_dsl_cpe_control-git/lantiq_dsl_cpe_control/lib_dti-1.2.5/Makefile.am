## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign 1.6 nostdinc

SUBDIRS = src application/dti_agent/src
LINT_SUBDIRS = src 

DISTCHECK_CONFIGURE_FLAGS=@CONFIGURE_OPTIONS@

dti_docdir = ${prefix}/doc/dti

dti_doc_DATA = \
	ChangeLog

EXTRA_DIST = $(dti_doc_DATA) \
   LICENSE\
	README\
	AUTHORS\
	NEWS\
	TODO\
	doc/dti.dxcfg\
	doc/footer.html\
	doc/header.html\
	doc/logo.gif\
	doc/stylesheet.css
	

# Copy all the spec files. Of cource, only one is actually used.
dist-hook:
	for specfile in *.spec; do \
		if test -f $$specfile; then \
			cp -p $$specfile $(distdir); \
		fi \
	done

lint:
	@target=lint; \
	list='$(LINT_SUBDIRS)'; \
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

