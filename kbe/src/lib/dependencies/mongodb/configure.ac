AC_PREREQ(2.60)

# Set version from contents of "VERSION_CURRENT" file.
AC_INIT([mongo-c-driver],
        m4_esyscmd_s(cat VERSION_CURRENT),
        [https://jira.mongodb.org/browse/CDRIVER])

m4_include([build/autotools/Versions.m4])

AC_CONFIG_MACRO_DIR([build/autotools/m4])
AC_CONFIG_AUX_DIR([build/autotools])
AC_SUBST(ACLOCAL_AMFLAGS, "-I build/autotools")

MONGOC_API_VERSION=1.0
AC_SUBST(MONGOC_API_VERSION)

m4_include([build/autotools/CheckCompiler.m4])
m4_include([build/autotools/CheckProgs.m4])
m4_include([build/autotools/CheckHost.m4])
m4_include([build/autotools/CheckTarget.m4])

m4_include([build/autotools/SetupLibtool.m4])
LT_INIT

m4_include([build/autotools/ReadCommandLineArguments.m4])
m4_include([build/autotools/CheckSasl.m4])
m4_include([build/autotools/CheckSSL.m4])
m4_include([build/autotools/FindDependencies.m4])
m4_include([build/autotools/AutoHarden.m4])
m4_include([build/autotools/PlatformFlags.m4])
m4_include([build/autotools/MaintainerFlags.m4])
m4_include([build/autotools/Optimizations.m4])
m4_include([build/autotools/Coverage.m4])
m4_include([build/autotools/LDVersionScript.m4])
m4_include([build/autotools/WeakSymbols.m4])
m4_include([build/autotools/AutomaticInitAndCleanup.m4])

# We would put AM_INIT_AUTOMAKE into SetupAutomake.m4, but seems to cause
# autoconf errors.
AM_INIT_AUTOMAKE([foreign subdir-objects tar-ustar])
AM_MAINTAINER_MODE([enable])
m4_include([build/autotools/SetupAutomake.m4])

AC_CONFIG_FILES([
    Makefile
    build/version
    src/libmongoc-1.0.pc
    src/libmongoc-ssl-1.0.pc
    src/libmongoc-priv.pc
    src/mongoc/mongoc-config.h
    src/mongoc/mongoc-version.h
    doc/installing.page
    doc/mongoc_version.page
])

m4_include([build/autotools/Libbson.m4])

m4_include([build/autotools/PrintBuildConfiguration.m4])
