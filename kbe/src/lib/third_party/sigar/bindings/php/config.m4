PHP_ARG_ENABLE(sigar, whether to enable Sigar support,
[ --enable-sigar   Enable Sigar support])

if test "$PHP_SIGAR" = "yes"; then
  AC_DEFINE(HAVE_SIGAR, 1, [Whether you have Sigar])
  dnl hardwired for the moment
  SIGAR_LIBNAME=sigar-universal-macosx
  SIGAR_INSTALLDIR=../java/sigar-bin
  SIGAR_LIB_DIR=$SIGAR_INSTALLDIR/lib
  SIGAR_INC_DIR=$SIGAR_INSTALLDIR/include
  PHP_ADD_LIBRARY_WITH_PATH($SIGAR_LIBNAME, $SIGAR_LIB_DIR, SIGAR_SHARED_LIBADD)
  PHP_ADD_INCLUDE($SIGAR_INC_DIR)
  PHP_NEW_EXTENSION(sigar, php_sigar.c, $ext_shared)
  PHP_SUBST(SIGAR_SHARED_LIBADD)
fi
