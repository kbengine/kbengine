COVERAGE_CFLAGS=""
COVERAGE_LDFLAGS=""

if test "$enable_coverage" = "yes"; then
    COVERAGE_CFLAGS="--coverage -g"
    COVERAGE_LDFLAGS="-lgcov"
fi

AC_SUBST(COVERAGE_CFLAGS)
AC_SUBST(COVERAGE_LDFLAGS)
