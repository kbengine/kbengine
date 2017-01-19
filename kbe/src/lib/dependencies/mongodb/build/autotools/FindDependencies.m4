
# Solaris needs to link against socket libs.
if test "$os_solaris" = "yes"; then
    CFLAGS="$CFLAGS -D__EXTENSIONS__"
    CFLAGS="$CFLAGS -D_XOPEN_SOURCE=1"
    CFLAGS="$CFLAGS -D_XOPEN_SOURCE_EXTENDED=1"
    LDFLAGS="$LDFLAGS -lsocket -lnsl"
fi

# Check if we should enable the bundled libbson.
if test "$with_libbson" = "auto"; then
   PKG_CHECK_MODULES(BSON, libbson-1.0 >= libbson_required_version,
                     [with_libbson=system], [with_libbson=bundled])
fi
AM_CONDITIONAL(ENABLE_LIBBSON, [test "$with_libbson" = "bundled"])

# Check for shm functions.
AC_CHECK_FUNCS([shm_open], [SHM_LIB=],
               [AC_CHECK_LIB([rt], [shm_open], [SHM_LIB=-lrt], [SHM_LIB=])])
AC_SUBST([SHM_LIB])

# Check for sched_getcpu
AC_CHECK_FUNCS([sched_getcpu])

# Check for clock_gettime
AC_SEARCH_LIBS([clock_gettime], [rt], [
    AC_DEFINE(HAVE_CLOCK_GETTIME, 1, [Have clock_gettime])
])
AS_IF([test "$ac_cv_search_clock_gettime" = "-lrt"],
      [LDFLAGS="$LDFLAGS -lrt"])

AS_IF([test "$enable_rdtscp" = "yes"],
      [CPPFLAGS="$CPPFLAGS -DENABLE_RDTSCP"])

AS_IF([test "$enable_shm_counters" = "yes"],
      [CPPFLAGS="$CPPFLAGS -DMONGOC_ENABLE_SHM_COUNTERS"])

AX_PTHREAD
