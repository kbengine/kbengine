AC_ARG_ENABLE([sasl],
              [AS_HELP_STRING([--enable-sasl=@<:@auto/yes/no@:>@],
                              [Use libsasl for Kerberos.])],
              [],
              [enable_sasl=auto])

sasl_mode=no

AS_IF([test "$enable_sasl" != "no"],[
  PKG_CHECK_MODULES(SASL, [libsasl2], [sasl_mode=sasl2], [
    AS_IF([test "$enable_sasl" != "no"],[
      AC_CHECK_LIB([sasl2],[sasl_client_init],[have_sasl2_lib=yes],[have_sasl2_lib=no])
      AC_CHECK_LIB([sasl],[sasl_client_init],[have_sasl_lib=yes],[have_sasl_lib=no])
      if test "$have_sasl_lib" = "no" -a "$have_sasl2_lib" = "no" -a "$enable_sasl" = "yes" ; then
        AC_MSG_ERROR([You must install the Cyrus SASL libraries and development headers to enable SASL support.])
      fi

      AC_CHECK_HEADER([sasl/sasl.h],[have_sasl_headers=yes],[have_sasl_headers=no])
      if test "$have_sasl_headers" = "no" -a "$enable_sasl" = "yes" ; then
        AC_MSG_ERROR([You must install the Cyrus SASL development headers to enable SASL support.])
      fi

      if test "$have_sasl_headers" -a "$have_sasl2_lib" = "yes" ; then
        sasl_mode=sasl2
        SASL_LIBS=-lsasl2
      fi

      if test "$have_sasl_headers" -a "$have_sasl_lib" = "yes" ; then
        sasl_mode=sasl
        SASL_LIBS=-lsasl
      fi
    ])
  ])
])

AM_CONDITIONAL([ENABLE_SASL], [test "$sasl_mode" != "no"])
AC_SUBST(SASL_CFLAGS)
AC_SUBST(SASL_LIBS)

dnl Let mongoc-config.h.in know about SASL status.
if test "$sasl_mode" != "no" ; then
  AC_SUBST(MONGOC_ENABLE_SASL, 1)
  
  AC_CHECK_LIB([sasl2],[sasl_client_done],
               [have_sasl_client_done=yes],
               [have_sasl_client_done=no])

  if test "$have_sasl_client_done" = "yes" ; then
    AC_SUBST(MONGOC_HAVE_SASL_CLIENT_DONE, 1)
  else
    AC_SUBST(MONGOC_HAVE_SASL_CLIENT_DONE, 0)
  fi

else
  AC_SUBST(MONGOC_ENABLE_SASL, 0)
  AC_SUBST(MONGOC_HAVE_SASL_CLIENT_DONE, 0)
fi
