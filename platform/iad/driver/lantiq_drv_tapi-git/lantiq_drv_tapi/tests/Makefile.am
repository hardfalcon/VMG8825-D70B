#
#
#                              Copyright (c) 2014
#                            Lantiq Deutschland GmbH
#
#  For licensing information, see the file 'LICENSE' in the root folder of
#  this software module.
#
#

AUTOMAKE_OPTIONS = foreign

EXTRA_PROGRAMS = test_lib_fifo test_lib_bufferpool

test_lib_fifo_SOURCES = \
    ${top_srcdir}/src/lib/lib_fifo.c \
    lib_fifotest.c

test_lib_bufferpool_SOURCES = \
    ${top_srcdir}/src/lib/lib_bufferpool.c \
    lib_buffertest.c

LDADD = \
    ${IFXOS_LIBRARY_PATH}/libifxos.a

# force overwrite CFLAGS
CFLAGS = \
	-DLINUX \
	-DDEBUG \
	-D_POSIX_SOURCE \
	-D_GNU_SOURCE

INCLUDES  = -I@abs_srcdir@ \
            -I@abs_top_srcdir@ \
            -I@abs_top_builddir@ \
            -I@abs_top_srcdir@/include \
            -I@IFXOS_INCL_PATH@

test-lib_fifo: test_lib_fifo
	@echo "+---------------------------------------------------------------------+"
	@echo "| for lib_fifo testing you can use 'test_lib_fifo' application        |"
	@echo "+---------------------------------------------------------------------+"

test-lib_bufferpool: test_lib_bufferpool
	@echo "+---------------------------------------------------------------------+"
	@echo "| for lib_fifo testing you can use 'test_lib_bufferpool' application  |"
	@echo "+---------------------------------------------------------------------+"

.PHONY: test-lib_fifo test-lib_bufferpool
