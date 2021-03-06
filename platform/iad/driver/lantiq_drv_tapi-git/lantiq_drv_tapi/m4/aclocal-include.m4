# aclocal-include.m4
# 
# This macro adds the name macrodir to the set of directories
# that `aclocal' searches for macros.  

dnl AM_ACLOCAL_INCLUDE(macrodir)
AC_DEFUN([AM_ACLOCAL_INCLUDE],
[
   test -n "$ACLOCAL_FLAGS" && ACLOCAL="$ACLOCAL $ACLOCAL_FLAGS"

   for k in $1 ; do ACLOCAL="$ACLOCAL -I $k" ; done
])dnl
