# Check for strnlen()
AC_SUBST(BSON_HAVE_STRNLEN, 0)
AC_CHECK_FUNC(strnlen, [AC_SUBST(BSON_HAVE_STRNLEN, 1)])

# Check for snprintf()
AC_SUBST(BSON_HAVE_SNPRINTF, 0)
AC_CHECK_FUNC(snprintf, [AC_SUBST(BSON_HAVE_SNPRINTF, 1)])

# Check for _set_output_format (unlikely, only Visual Studio 2013 and older)
AC_SUBST(BSON_NEEDS_SET_OUTPUT_FORMAT, 0)
AC_CHECK_FUNCS(_set_output_format, [AC_SUBST(BSON_NEEDS_SET_OUTPUT_FORMAT, 1)])

# Check for struct timespec
AC_SUBST(BSON_HAVE_TIMESPEC, 0)
AC_CHECK_TYPES([struct timespec], [AC_SUBST(BSON_HAVE_TIMESPEC, 1)])

# Check for clock_gettime and if it needs -lrt
AC_SUBST(BSON_HAVE_CLOCK_GETTIME, 0)
AC_SEARCH_LIBS([clock_gettime], [rt], [AC_SUBST(BSON_HAVE_CLOCK_GETTIME, 1)])


# Check for pthreads. We might need to make this better to handle mingw,
# but I actually think it is okay to just check for it even though we will
# use win32 primatives.
AX_PTHREAD([],
           [AC_MSG_ERROR([libbson requires pthreads on non-Windows platforms.])])


# The following is borrowed from the guile configure script.
#
# On past versions of Solaris, believe 8 through 10 at least, you
# had to write "pthread_once_t foo = { PTHREAD_ONCE_INIT };".
# This is contrary to POSIX:
# http://www.opengroup.org/onlinepubs/000095399/functions/pthread_once.html
# Check here if this style is required.
#
# glibc (2.3.6 at least) works both with or without braces, so the
# test checks whether it works without.
#
AC_SUBST(BSON_PTHREAD_ONCE_INIT_NEEDS_BRACES, 0)
AC_CACHE_CHECK([whether PTHREAD_ONCE_INIT needs braces],
  bson_cv_need_braces_on_pthread_once_init,
  [AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <pthread.h>
     pthread_once_t foo = PTHREAD_ONCE_INIT;]])],
    [bson_cv_need_braces_on_pthread_once_init=no],
    [bson_cv_need_braces_on_pthread_once_init=yes])])
if test "$bson_cv_need_braces_on_pthread_once_init" = yes; then
    AC_SUBST(BSON_PTHREAD_ONCE_INIT_NEEDS_BRACES, 1)
fi

# Solaris needs to link against socket libs.
# This is only used in our streaming bson examples
if test "$os_solaris" = "yes"; then
    SOCKET_CFLAGS="$CFLAGS -D__EXTENSIONS__"
    SOCKET_CFLAGS="$CFLAGS -D_XOPEN_SOURCE=1"
    SOCKET_CFLAGS="$CFLAGS -D_XOPEN_SOURCE_EXTENDED=1"
    SOCKET_LDFLAGS="$LDFLAGS -lsocket -lnsl"
    AC_SUBST(SOCKET_CFLAGS)
    AC_SUBST(SOCKET_LDFLAGS)
fi
