AC_ARG_ENABLE([ssl],
              [AS_HELP_STRING([--enable-ssl=@<:@auto/yes/no@:>@],
                              [Use OpenSSL for TLS connections and SCRAM-SHA-1 authentication. NOTE: OpenSSL is required for authenticating to MongoDB 3.0 and later])],
              [],
              [enable_ssl=auto])

AS_IF([test "$enable_ssl" != "no"],[
  PKG_CHECK_MODULES(SSL, [openssl], [enable_ssl=yes], [
    AS_IF([test "$enable_ssl" != "no"],[
      AC_CHECK_LIB([ssl],[SSL_library_init],[have_ssl_lib=yes],[have_ssl_lib=no])
      AC_CHECK_LIB([crypto],[CRYPTO_set_locking_callback],[have_crypto_lib=yes],[have_crypto_lib=no])
      if test "$enable_ssl" = "yes"; then
        if test "$have_ssl_lib" = "no" -o "$have_crypto_lib" = "no" ; then
          AC_MSG_ERROR([You must install the OpenSSL libraries and development headers to enable SSL support.])
        fi
      fi

      AC_CHECK_HEADERS([openssl/bio.h openssl/ssl.h openssl/err.h openssl/crypto.h],
                       [have_ssl_headers=yes],
                       [have_ssl_headers=no])
      if test "$have_ssl_headers" = "no" -a "$enable_ssl" = "yes" ; then
        AC_MSG_ERROR([You must install the OpenSSL development headers to enable SSL support.])
      fi

      if test "$have_ssl_headers" = "yes" -a "$have_ssl_lib" = "yes" -a "$have_crypto_lib" = "yes"; then
        SSL_LIBS="-lssl -lcrypto"
        enable_ssl=yes
      else
        enable_ssl=no
      fi
    ])
  ])
])


AM_CONDITIONAL([ENABLE_SSL], [test "$enable_ssl" = "yes"])
AC_SUBST(SSL_CFLAGS)
AC_SUBST(SSL_LIBS)

dnl Let mongoc-config.h.in know about SSL status.
if test "$enable_ssl" = "yes" ; then
  AC_SUBST(MONGOC_ENABLE_SSL, 1)
else
  AC_SUBST(MONGOC_ENABLE_SSL, 0)
fi
