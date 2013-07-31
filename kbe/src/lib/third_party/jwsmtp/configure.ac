dnl jwSMTP library configure.ac file
dnl --------------------------------

dnl --------------------------------
dnl Do mi checks 'ere
dnl --------------------------------

AC_INIT(jwsmtp/jwsmtp/jwsmtp.h)
AM_CONFIG_HEADER(config.h)

dnl -----------------------------------------------
dnl Package name and version number
dnl -----------------------------------------------

JWSMTP_LIBRARY_NAME=jwsmtp

#release versioning
JWSMTP_MAJOR_VERSION=1
JWSMTP_MINOR_VERSION=32
JWSMTP_MICRO_VERSION=15

#  API version
JWSMTP_API_VERSION=1.32
AC_SUBST(JWSMTP_API_VERSION)

#shared library versioning
JWSMTP_LIBRARY_VERSION=2:8:2

dnl --------------------------------
dnl Package name and version number
dnl --------------------------------

AC_SUBST(JWSMTP_LIBRARY_VERSION)

PACKAGE=$JWSMTP_LIBRARY_NAME
AC_SUBST(JWSMTP_LIBRARY_NAME)

JWSMTP_VERSION=$JWSMTP_MAJOR_VERSION.$JWSMTP_MINOR_VERSION.$JWSMTP_MICRO_VERSION
JWSMTP_RELEASE=$JWSMTP_MAJOR_VERSION.$JWSMTP_MINOR_VERSION
AC_SUBST(JWSMTP_RELEASE)
AC_SUBST(JWSMTP_VERSION)

VERSION=$JWSMTP_VERSION

AM_INIT_AUTOMAKE($PACKAGE, $VERSION, no-define)

dnl -----------------------------------------------
dnl Checks for programs.
dnl -----------------------------------------------

AC_PROG_CC
AC_PROG_CXX
AM_PROG_LIBTOOL
AM_SANITY_CHECK
AC_LANG_CPLUSPLUS

dnl -----------------------------------------------
dnl The Makefiles
dnl
dnl update the pc filename to whatever version is now
dnl
dnl -----------------------------------------------

AC_OUTPUT(Makefile \
          jwsmtp-1.32.pc \
          jwsmtp/Makefile \
          jwsmtp/jwsmtp/Makefile \
          docs/Makefile \
)

AC_MSG_NOTICE([
to compile and install ${JWSMTP_LIBRARY_NAME}
  make
  make install])
