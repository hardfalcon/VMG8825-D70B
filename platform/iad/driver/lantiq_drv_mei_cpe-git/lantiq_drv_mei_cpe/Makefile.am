## Process this file with automake to produce Makefile.in

## Following varibales are set externally:
## DISTCHECK_CONFIGURE_PARAMS:  exported by the configure.in script via subst
##                              from "DISTCHECK_PAR_DRV"
## NOTE: "DISTCHECK_PAR_DRV" is set by the build_all.sh script.

AUTOMAKE_OPTIONS = foreign 1.6 dist-zip nostdinc

SUBDIRS = src

DISTCHECK_CONFIGURE_FLAGS=@CONFIGURE_OPTIONS@

drv_vrx_docdir = ${prefix}/doc/drv_vrx

drv_vrx_doc_DATA = \
	ChangeLog

EXTRA_DIST = $(drv_vrx_doc_DATA)\
	README \
	AUTHORS \
	NEWS \
	TODO \
	LICENSE \
	doc/doxyconfig \
	doc/footer.html \
	doc/header.html \
	doc/mei_cpe_drv.h \
	doc/mei_cpe_drv_SW_structure_overview.jpg \
	doc/html/logo.gif \
	doc/html/spacer.gif \
	doc/html/stylesheet.css


# Copy all the spec files. Of cource, only one is actually used.
dist-hook:
	for specfile in *.spec; do \
		if test -f $$specfile; then \
			cp -p $$specfile $(distdir); \
		fi \
	done

# Check for correct line ends
distcheck-hook:
	chmod a+w $(distdir)
	echo "Checking line ends ...!!!"; \
	find $(distdir) -type f -exec file {} \; | grep -e "CRLF" -e "Non-ISO" && exit 1; \
	echo "Checking line ends ... done."
