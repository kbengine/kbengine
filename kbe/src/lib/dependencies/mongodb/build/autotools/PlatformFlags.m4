dnl check_cc_cxx_flag is defined in build/autotools/AutoHarden.m4
dnl Ignore OpenSSL deprecation warnings on OSX
AS_IF([test "$os_darwin" = "yes"],
      [check_cc_cxx_flag([-Wno-deprecated-declarations], [CFLAGS="$CFLAGS -Wno-deprecated-declarations"])])
dnl We know there are some cast-align issues on OSX
AS_IF([test "$os_darwin" = "yes"],
      [check_cc_cxx_flag([-Wno-cast-align], [CFLAGS="$CFLAGS -Wno-cast-align"])])
AS_IF([test "$os_darwin" = "yes"],
      [check_cc_cxx_flag([-Wno-unneeded-internal-declaration], [CFLAGS="$CFLAGS -Wno-unneeded-internal-declaration"])])
dnl We know there are some cast-align issues on Solaris
AS_IF([test "$os_solaris" = "yes"],
      [check_cc_cxx_flag([-Wno-cast-align], [CFLAGS="$CFLAGS -Wno-cast-align"])])


