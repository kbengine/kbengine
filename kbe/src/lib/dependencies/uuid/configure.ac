# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([libuuid], [1.0.3], [sloowfranklin@gmail.com])
AC_CONFIG_SRCDIR([compare.c])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([foreign])
LT_INIT

# Checks for programs.
AC_PROG_CC


# Checks for libraries.
SOCKET_LIBS=
AC_SEARCH_LIBS([socket], [socket],
    [if test x"$ac_cv_search_socket" != x"none required"; then
        SOCKET_LIBS="$SOCKET_LIBS -lsocket";
    fi])
AC_SUBST([SOCKET_LIBS])

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h inttypes.h limits.h netinet/in.h stdlib.h string.h sys/file.h sys/ioctl.h sys/socket.h sys/time.h unistd.h])
AC_CHECK_DECLS([_SC_HOST_NAME_MAX])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_INT32_T
AC_TYPE_MODE_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_CHECK_FUNCS([ftruncate gettimeofday memset socket strtoul usleep srandom])

dnl version details from <major>.<minor>.<release>
PACKAGE_VERSION_MAJOR=$(echo $PACKAGE_VERSION | awk -F. '{print $1}')
PACKAGE_VERSION_MINOR=$(echo $PACKAGE_VERSION | awk -F. '{print $2}')
PACKAGE_VERSION_RELEASE=$(echo $PACKAGE_VERSION | awk -F. '{print $3}')

LIBUUID_VERSION="$PACKAGE_VERSION_MAJOR.$PACKAGE_VERSION_MINOR.$PACKAGE_VERSION_RELEASE"
LIBUUID_LT_MAJOR=1
LIBUUID_LT_MINOR=0
LIBUUID_LT_MICRO=0
LIBUUID_VERSION_INFO=`expr $LIBUUID_LT_MAJOR + $LIBUUID_LT_MINOR`:$LIBUUID_LT_MICRO:$LIBUUID_LT_MINOR
AC_SUBST([LIBUUID_VERSION])
AC_SUBST([LIBUUID_VERSION_INFO])

AC_CONFIG_FILES(
  [Makefile]
  [uuid.pc]
)
AC_OUTPUT
