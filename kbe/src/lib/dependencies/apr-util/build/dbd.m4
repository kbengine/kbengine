dnl -------------------------------------------------------- -*- autoconf -*-
dnl Licensed to the Apache Software Foundation (ASF) under one or more
dnl contributor license agreements.  See the NOTICE file distributed with
dnl this work for additional information regarding copyright ownership.
dnl The ASF licenses this file to You under the Apache License, Version 2.0
dnl (the "License"); you may not use this file except in compliance with
dnl the License.  You may obtain a copy of the License at
dnl
dnl     http://www.apache.org/licenses/LICENSE-2.0
dnl
dnl Unless required by applicable law or agreed to in writing, software
dnl distributed under the License is distributed on an "AS IS" BASIS,
dnl WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
dnl See the License for the specific language governing permissions and
dnl limitations under the License.

dnl
dnl DBD module
dnl

dnl
dnl APU_CHECK_DBD: compile backends for apr_dbd.
dnl
AC_DEFUN([APU_CHECK_DBD], [
  apu_have_pgsql=0

  old_libs="$LIBS"
  old_cppflags="$CPPFLAGS"
  old_ldflags="$LDFLAGS"

  AC_ARG_WITH([pgsql], APR_HELP_STRING([--with-pgsql=DIR], [specify PostgreSQL location]),
  [
    if test "$withval" = "yes"; then
      AC_PATH_PROG([PGSQL_CONFIG],[pg_config])
      if test "x$PGSQL_CONFIG" != 'x'; then
        pgsql_CPPFLAGS="-I`$PGSQL_CONFIG --includedir`"
        pgsql_LDFLAGS="-L`$PGSQL_CONFIG --libdir`"

        APR_ADDTO(CPPFLAGS, [$pgsql_CPPFLAGS])
        APR_ADDTO(LDFLAGS, [$pgsql_LDFLAGS])
      fi

      AC_CHECK_HEADERS(libpq-fe.h, [
        AC_CHECK_LIB(pq, PQsendQueryPrepared, [apu_have_pgsql=1],[
        if test "x$PGSQL_CONFIG" != 'x'; then
          unset ac_cv_lib_pq_PQsendQueryPrepared
          pgsql_LIBS="`$PGSQL_CONFIG --libs`"
          APR_ADDTO(LIBS, [$pgsql_LIBS])
          AC_CHECK_LIB(pq, PQsendQueryPrepared, [apu_have_pgsql=1])
        fi
        ])
      ])
      if test "$apu_have_pgsql" = "0"; then
        AC_CHECK_HEADERS(postgresql/libpq-fe.h, [
          AC_CHECK_LIB(pq, PQsendQueryPrepared, [apu_have_pgsql=1],[
          if test "x$PGSQL_CONFIG" != 'x'; then
            unset ac_cv_lib_pq_PQsendQueryPrepared
            pgsql_LIBS="`$PGSQL_CONFIG --libs`"
            APR_ADDTO(LIBS, [$pgsql_LIBS])
            AC_CHECK_LIB(pq, PQsendQueryPrepared, [apu_have_pgsql=1])
          fi
          ])
        ])
      fi
      if test "$apu_have_pgsql" != "0" && test "x$PGSQL_CONFIG" != 'x'; then
        APR_ADDTO(APRUTIL_PRIV_INCLUDES, [$pgsql_CPPFLAGS])
      fi
    elif test "$withval" = "no"; then
      :
    else
      AC_PATH_PROG([PGSQL_CONFIG],[pg_config],,[$withval/bin])
      if test "x$PGSQL_CONFIG" != 'x'; then
        pgsql_CPPFLAGS="-I`$PGSQL_CONFIG --includedir`"
        pgsql_LDFLAGS="-L`$PGSQL_CONFIG --libdir`"
      else
        pgsql_CPPFLAGS="-I$withval/include"
        pgsql_LDFLAGS="-L$withval/lib "
      fi

      APR_ADDTO(CPPFLAGS, [$pgsql_CPPFLAGS])
      APR_ADDTO(LDFLAGS, [$pgsql_LDFLAGS])

      AC_MSG_NOTICE(checking for pgsql in $withval)
      AC_CHECK_HEADERS(libpq-fe.h, [
        AC_CHECK_LIB(pq, PQsendQueryPrepared, [apu_have_pgsql=1],[
        if test "x$PGSQL_CONFIG" != 'x'; then
          unset ac_cv_lib_pq_PQsendQueryPrepared
          pgsql_LIBS="`$PGSQL_CONFIG --libs`"
          APR_ADDTO(LIBS, [$pgsql_LIBS])
          AC_CHECK_LIB(pq, PQsendQueryPrepared, [apu_have_pgsql=1])
        fi
        ])
      ])
      if test "$apu_have_pgsql" != "1"; then
        AC_CHECK_HEADERS(postgresql/libpq-fe.h, [
          AC_CHECK_LIB(pq, PQsendQueryPrepared, [apu_have_pgsql=1],[
          if test "x$PGSQL_CONFIG" != 'x'; then
            unset ac_cv_lib_pq_PQsendQueryPrepared
            pgsql_LIBS="`$PGSQL_CONFIG --libs`"
            APR_ADDTO(LIBS, [$pgsql_LIBS])
            AC_CHECK_LIB(pq, PQsendQueryPrepared, [apu_have_pgsql=1])
          fi
          ])
        ])
      fi
      if test "$apu_have_pgsql" != "0"; then
        APR_ADDTO(APRUTIL_PRIV_INCLUDES, [$pgsql_CPPFLAGS])
      fi
    fi
  ], [
    AC_PATH_PROG([PGSQL_CONFIG],[pg_config])
    if test "x$PGSQL_CONFIG" != 'x'; then
      pgsql_CPPFLAGS="-I`$PGSQL_CONFIG --includedir`"
      pgsql_LDFLAGS="-L`$PGSQL_CONFIG --libdir`"

      APR_ADDTO(CPPFLAGS, [$pgsql_CPPFLAGS])
      APR_ADDTO(LDFLAGS, [$pgsql_LDFLAGS])
    fi

    AC_CHECK_HEADERS(libpq-fe.h, [
      AC_CHECK_LIB(pq, PQsendQueryPrepared, [apu_have_pgsql=1],[
      if test "x$PGSQL_CONFIG" != 'x'; then
        unset ac_cv_lib_pq_PQsendQueryPrepared
        pgsql_LIBS="`$PGSQL_CONFIG --libs`"
        APR_ADDTO(LIBS, [$pgsql_LIBS])
        AC_CHECK_LIB(pq, PQsendQueryPrepared, [apu_have_pgsql=1])
      fi
      ])
    ])
    if test "$apu_have_pgsql" = "0"; then
      AC_CHECK_HEADERS(postgresql/libpq-fe.h, [
        AC_CHECK_LIB(pq, PQsendQueryPrepared, [apu_have_pgsql=1],[
        if test "x$PGSQL_CONFIG" != 'x'; then
          unset ac_cv_lib_pq_PQsendQueryPrepared
          pgsql_LIBS="`$PGSQL_CONFIG --libs`"
          APR_ADDTO(LIBS, [$pgsql_LIBS])
          AC_CHECK_LIB(pq, PQsendQueryPrepared, [apu_have_pgsql=1])
        fi
        ])
      ])
    fi
    if test "$apu_have_pgsql" != "0" && test "x$PGSQL_CONFIG" != 'x'; then
      APR_ADDTO(APRUTIL_PRIV_INCLUDES, [$pgsql_CPPFLAGS])
    fi
  ])
  AC_SUBST(apu_have_pgsql)
  dnl Since we have already done the AC_CHECK_LIB tests, if we have it, 
  dnl we know the library is there.
  if test "$apu_have_pgsql" = "1"; then
    APR_ADDTO(LDADD_dbd_pgsql, [$pgsql_LDFLAGS -lpq $pgsql_LIBS])
  fi
  AC_SUBST(LDADD_dbd_pgsql)

  LIBS="$old_libs"
  CPPFLAGS="$old_cppflags"
  LDFLAGS="$old_ldflags"
])
dnl
AC_DEFUN([APU_CHECK_DBD_MYSQL], [
  apu_have_mysql=0

  old_libs="$LIBS"
  old_cppflags="$CPPFLAGS"
  old_ldflags="$LDFLAGS"

  AC_ARG_WITH([mysql], APR_HELP_STRING([--with-mysql=DIR], [enable MySQL DBD driver]),
  [
    if test "$withval" = "yes"; then
      AC_PATH_PROG([MYSQL_CONFIG],[mysql_config])
      if test "x$MYSQL_CONFIG" != 'x'; then
        mysql_CPPFLAGS="`$MYSQL_CONFIG --include`"
        mysql_LDFLAGS="`$MYSQL_CONFIG --libs_r | sed -e 's/-l[[^ ]]\+//g'`"
        mysql_LIBS="`$MYSQL_CONFIG --libs_r`"

        APR_ADDTO(CPPFLAGS, [$mysql_CPPFLAGS])
        APR_ADDTO(LIBS, [$mysql_LIBS])
      fi

      AC_CHECK_HEADERS([mysql.h my_global.h my_sys.h],
                       AC_CHECK_LIB(mysqlclient_r, mysql_init, [apu_have_mysql=1]),
                       [apu_have_mysql=0; break],
                       [#include <my_global.h>])
      if test "$apu_have_mysql" = "0"; then
        AC_CHECK_HEADERS([mysql/mysql.h mysql/my_global.h mysql/my_sys.h],
                         AC_CHECK_LIB(mysqlclient_r, mysql_init, [apu_have_mysql=1]),
                         [apu_have_mysql=0; break],
                         [#include <mysql/my_global.h>])
      fi
      if test "$apu_have_mysql" != "0" && test "x$MYSQL_CONFIG" != 'x'; then
        APR_ADDTO(APRUTIL_PRIV_INCLUDES, [$mysql_CPPFLAGS])
      fi
    elif test "$withval" = "no"; then
      :
    else
      AC_PATH_PROG([MYSQL_CONFIG],[mysql_config],,[$withval/bin])
      if test "x$MYSQL_CONFIG" != 'x'; then
        mysql_CPPFLAGS="`$MYSQL_CONFIG --include`"
        mysql_LDFLAGS="`$MYSQL_CONFIG --libs_r | sed -e 's/-l[[^ ]]\+//g'`"
        mysql_LIBS="`$MYSQL_CONFIG --libs_r`"
      else
        mysql_CPPFLAGS="-I$withval/include"
        mysql_LDFLAGS="-L$withval/lib "
      fi

      APR_ADDTO(CPPFLAGS, [$mysql_CPPFLAGS])
      APR_ADDTO(LDFLAGS, [$mysql_LDFLAGS])
      APR_ADDTO(LIBS, [$mysql_LIBS])

      AC_MSG_NOTICE(checking for mysql in $withval)
      AC_CHECK_HEADERS([mysql.h my_global.h my_sys.h],
                       AC_CHECK_LIB(mysqlclient_r, mysql_init, [apu_have_mysql=1]),
                       [apu_have_mysql=0; break],
                       [#include <my_global.h>])

      if test "$apu_have_mysql" != "1"; then
        AC_CHECK_HEADERS([mysql/mysql.h mysql/my_global.h mysql/my_sys.h],
                         AC_CHECK_LIB(mysqlclient_r, mysql_init, [apu_have_mysql=1]),
                         [apu_have_mysql=0; break],
                         [#include <mysql/my_global.h>])
      fi
      if test "$apu_have_mysql" != "0"; then
        APR_ADDTO(APRUTIL_PRIV_INCLUDES, [$mysql_CPPFLAGS])
      fi
    fi
  ])

  AC_SUBST(apu_have_mysql)

  dnl Since we have already done the AC_CHECK_LIB tests, if we have it, 
  dnl we know the library is there.
  if test "$apu_have_mysql" = "1"; then
    APR_ADDTO(LDADD_dbd_mysql, [$mysql_LDFLAGS -lmysqlclient_r $mysql_LIBS])
  fi
  AC_SUBST(LDADD_dbd_mysql)

  LIBS="$old_libs"
  CPPFLAGS="$old_cppflags"
  LDFLAGS="$old_ldflags"
])
dnl
AC_DEFUN([APU_CHECK_DBD_SQLITE3], [
  apu_have_sqlite3=0

  old_libs="$LIBS"
  old_cppflags="$CPPFLAGS"
  old_ldflags="$LDFLAGS"

  AC_ARG_WITH([sqlite3], APR_HELP_STRING([--with-sqlite3=DIR], [enable sqlite3 DBD driver]),
  [
    if test "$withval" = "yes"; then
      AC_CHECK_HEADERS(sqlite3.h, AC_CHECK_LIB(sqlite3, sqlite3_open, [apu_have_sqlite3=1]))
    elif test "$withval" = "no"; then
      :
    else
      sqlite3_CPPFLAGS="-I$withval/include"
      sqlite3_LDFLAGS="-L$withval/lib "

      APR_ADDTO(CPPFLAGS, [$sqlite3_CPPFLAGS])
      APR_ADDTO(LDFLAGS, [$sqlite3_LDFLAGS])

      AC_MSG_NOTICE(checking for sqlite3 in $withval)
      AC_CHECK_HEADERS(sqlite3.h, AC_CHECK_LIB(sqlite3, sqlite3_open, [apu_have_sqlite3=1]))
      if test "$apu_have_sqlite3" != "0"; then
        APR_ADDTO(APRUTIL_PRIV_INCLUDES, [-I$withval/include])
      fi
    fi
  ], [
    AC_CHECK_HEADERS(sqlite3.h, AC_CHECK_LIB(sqlite3, sqlite3_open, [apu_have_sqlite3=1]))
  ])

  AC_SUBST(apu_have_sqlite3)

  dnl Since we have already done the AC_CHECK_LIB tests, if we have it, 
  dnl we know the library is there.
  if test "$apu_have_sqlite3" = "1"; then
    APR_ADDTO(LDADD_dbd_sqlite3, [$sqlite3_LDFLAGS -lsqlite3])
  fi
  AC_SUBST(LDADD_dbd_sqlite3)

  LIBS="$old_libs"
  CPPFLAGS="$old_cppflags"
  LDFLAGS="$old_ldflags"
])
dnl
AC_DEFUN([APU_CHECK_DBD_SQLITE2], [
  apu_have_sqlite2=0

  old_libs="$LIBS"
  old_cppflags="$CPPFLAGS"
  old_ldflags="$LDFLAGS"

  AC_ARG_WITH([sqlite2], APR_HELP_STRING([--with-sqlite2=DIR], [enable sqlite2 DBD driver]),
  [
    if test "$withval" = "yes"; then
      AC_CHECK_HEADERS(sqlite.h, AC_CHECK_LIB(sqlite, sqlite_open, [apu_have_sqlite2=1]))
    elif test "$withval" = "no"; then
      :
    else
      sqlite2_CPPFLAGS="-I$withval/include"
      sqlite2_LDFLAGS="-L$withval/lib "

      APR_ADDTO(CPPFLAGS, [$sqlite2_CPPFLAGS])
      APR_ADDTO(LDFLAGS, [$sqlite2_LDFLAGS])

      AC_MSG_NOTICE(checking for sqlite2 in $withval)
      AC_CHECK_HEADERS(sqlite.h, AC_CHECK_LIB(sqlite, sqlite_open, [apu_have_sqlite2=1]))
      if test "$apu_have_sqlite2" != "0"; then
        APR_ADDTO(APRUTIL_PRIV_INCLUDES, [-I$withval/include])
      fi
    fi
  ], [
    AC_CHECK_HEADERS(sqlite.h, AC_CHECK_LIB(sqlite, sqlite_open, [apu_have_sqlite2=1]))
  ])

  AC_SUBST(apu_have_sqlite2)

  dnl Since we have already done the AC_CHECK_LIB tests, if we have it, 
  dnl we know the library is there.
  if test "$apu_have_sqlite2" = "1"; then
    APR_ADDTO(LDADD_dbd_sqlite2, [$sqlite2_LDFLAGS -lsqlite])
  fi
  AC_SUBST(LDADD_dbd_sqlite2)

  LIBS="$old_libs"
  CPPFLAGS="$old_cppflags"
  LDFLAGS="$old_ldflags"
])
dnl
AC_DEFUN([APU_CHECK_DBD_ORACLE], [
  apu_have_oracle=0

  old_libs="$LIBS"
  old_cppflags="$CPPFLAGS"
  old_ldflags="$LDFLAGS"

  AC_ARG_WITH([oracle-include],
    APR_HELP_STRING([--with-oracle-include=DIR], [path to Oracle include files]))
  AC_ARG_WITH([oracle], 
    APR_HELP_STRING([--with-oracle=DIR], [enable Oracle DBD driver; giving ORACLE_HOME as DIR]),
  [
    if test "$withval" = "yes"; then
      if test -n "$with_oracle_include"; then
        oracle_CPPFLAGS="$CPPFLAGS -I$with_oracle_include"
        APR_ADDTO(APRUTIL_PRIV_INCLUDES, [-I$with_oracle_include])
      fi

      APR_ADDTO(CPPFLAGS, [$oracle_CPPFLAGS])

      AC_CHECK_HEADERS(oci.h, AC_CHECK_LIB(clntsh, OCIEnvCreate, [apu_have_oracle=1],[
        unset ac_cv_lib_clntsh_OCIEnvCreate
        oracle_LIBS="-lnnz11"
        APR_ADDTO(LIBS, [$oracle_LIBS])
        AC_CHECK_LIB(clntsh, OCIEnvCreate, [apu_have_oracle=1],[
          unset ac_cv_lib_clntsh_OCIEnvCreate
          APR_REMOVEFROM(LIBS, [$oracle_LIBS])
          oracle_LIBS="-lnnz10"
          APR_ADDTO(LIBS, [$oracle_LIBS])
          AC_CHECK_LIB(clntsh, OCIEnvCreate, [apu_have_oracle=1])
        ])
      ]))
    elif test "$withval" = "no"; then
      :
    else
      if test -n "$with_oracle_include"; then
        oracle_CPPFLAGS="$CPPFLAGS -I$with_oracle_include"
        APR_ADDTO(APRUTIL_PRIV_INCLUDES, [-I$with_oracle_include])
      else
        oracle_CPPFLAGS="-I$withval/rdbms/demo -I$withval/rdbms/public"
      fi
      oracle_LDFLAGS="-L$withval/lib "

      APR_ADDTO(CPPFLAGS, [$oracle_CPPFLAGS])
      APR_ADDTO(LDFLAGS, [$oracle_LDFLAGS])

      AC_MSG_NOTICE(checking for oracle in $withval)
      AC_CHECK_HEADERS(oci.h, AC_CHECK_LIB(clntsh, OCIEnvCreate, [apu_have_oracle=1],[
        unset ac_cv_lib_clntsh_OCIEnvCreate
        oracle_LIBS="-lnnz11"
        APR_ADDTO(LIBS, [$oracle_LIBS])
        AC_CHECK_LIB(clntsh, OCIEnvCreate, [apu_have_oracle=1],[
          unset ac_cv_lib_clntsh_OCIEnvCreate
          APR_REMOVEFROM(LIBS, [$oracle_LIBS])
          oracle_LIBS="-lnnz10"
          APR_ADDTO(LIBS, [$oracle_LIBS])
          AC_CHECK_LIB(clntsh, OCIEnvCreate, [apu_have_oracle=1])
        ])
      ]))
      if test "$apu_have_oracle" != "0"; then
        oracle_LDFLAGS="$oracle_LDFLAGS -R$withval/lib"
        if test -z "$with_oracle_include"; then
          APR_ADDTO(APRUTIL_PRIV_INCLUDES, [-I$withval/rdbms/demo])
          APR_ADDTO(APRUTIL_PRIV_INCLUDES, [-I$withval/rdbms/public])
        fi
      fi
    fi
  ])

  AC_SUBST(apu_have_oracle)

  dnl Since we have already done the AC_CHECK_LIB tests, if we have it, 
  dnl we know the library is there.
  if test "$apu_have_oracle" = "1"; then
    APR_ADDTO(LDADD_dbd_oracle, [$oracle_LDFLAGS -lclntsh $oracle_LIBS])
  fi
  AC_SUBST(LDADD_dbd_oracle)

  LIBS="$old_libs"
  CPPFLAGS="$old_cppflags"
  LDFLAGS="$old_ldflags"
])

dnl
AC_DEFUN([APU_CHECK_DBD_FREETDS], [
  apu_have_freetds=0

  old_libs="$LIBS"
  old_cppflags="$CPPFLAGS"
  old_ldflags="$LDFLAGS"

  AC_ARG_WITH([freetds], 
    APR_HELP_STRING([--with-freetds=DIR], [specify FreeTDS location]),
  [
    if test "$withval" = "yes"; then
      AC_CHECK_HEADERS(sybdb.h, AC_CHECK_LIB(sybdb, tdsdbopen, [apu_have_freetds=1]))
      if test "$apu_have_freetds" = "0"; then
        AC_CHECK_HEADERS(freetds/sybdb.h, AC_CHECK_LIB(sybdb, tdsdbopen, [apu_have_freetds=1]))
      fi
    elif test "$withval" = "no"; then
      :
    else
      sybdb_CPPFLAGS="-I$withval/include"
      sybdb_LDFLAGS="-L$withval/lib "

      APR_ADDTO(CPPFLAGS, [$sybdb_CPPFLAGS])
      APR_ADDTO(LDFLAGS, [$sybdb_LDFLAGS])

      AC_MSG_NOTICE(checking for freetds in $withval)
      AC_CHECK_HEADERS(sybdb.h, AC_CHECK_LIB(sybdb, tdsdbopen, [apu_have_freetds=1]))
      if test "$apu_have_freetds" = "0"; then
        AC_CHECK_HEADERS(freetds/sybdb.h, AC_CHECK_LIB(sybdb, tdsdbopen, [apu_have_freetds=1]))
      fi
      if test "$apu_have_freetds" != "0"; then
        APR_ADDTO(APRUTIL_PRIV_INCLUDES, [-I$withval/include])
      fi
    fi
  ], [
    AC_CHECK_HEADERS(sybdb.h, AC_CHECK_LIB(sybdb, tdsdbopen, [apu_have_freetds=1]))
    if test "$apu_have_freetds" = "0"; then
      AC_CHECK_HEADERS(freetds/sybdb.h, AC_CHECK_LIB(sybdb, tdsdbopen, [apu_have_freetds=1]))
    fi
  ])

  AC_SUBST(apu_have_freetds)

  dnl Since we have already done the AC_CHECK_LIB tests, if we have it, 
  dnl we know the library is there.
  if test "$apu_have_freetds" = "1"; then
    APR_ADDTO(LDADD_dbd_freetds, [$sybdb_LDFLAGS -lsybdb])
    dnl Erm, I needed pcreposix, but I think that dependency has gone
    dnl from the current code
    dnl APR_ADDTO(LDADD_dbd_freetds, [-lpcreposix])
  fi
  AC_SUBST(LDADD_dbd_freetds)

  LIBS="$old_libs"
  CPPFLAGS="$old_cppflags"
  LDFLAGS="$old_ldflags"
])
dnl

AC_DEFUN([APU_CHECK_DBD_ODBC], [
  apu_have_odbc=0

  old_libs="$LIBS"
  old_cppflags="$CPPFLAGS"
  old_ldflags="$LDFLAGS"

  AC_ARG_WITH([odbc], APR_HELP_STRING([--with-odbc=DIR], [specify ODBC location]),
  [
    if test "$withval" = "yes"; then
      AC_PATH_PROG([ODBC_CONFIG],[odbc_config])
      if test "x$ODBC_CONFIG" != 'x'; then
        odbc_CPPFLAGS="-I`$ODBC_CONFIG --include-prefix`"
        odbc_LDFLAGS="-L`$ODBC_CONFIG --lib-prefix`"
        odbc_LIBS="`$ODBC_CONFIG --libs`"

        APR_ADDTO(CPPFLAGS, [$odbc_CPPFLAGS])
        APR_ADDTO(LDFLAGS, [$odbc_LDFLAGS])
        APR_ADDTO(LIBS, [$odbc_LIBS])
      fi

      AC_CHECK_HEADERS(sql.h, AC_CHECK_LIB(odbc, SQLAllocHandle, [apu_have_odbc=1]))
      if test "$apu_have_odbc" = "0"; then
        AC_CHECK_HEADERS(odbc/sql.h, AC_CHECK_LIB(odbc, SQLAllocHandle, [apu_have_odbc=1]))
      fi
      if test "$apu_have_odbc" != "0" && test "x$ODBC_CONFIG" != 'x'; then
        APR_ADDTO(APRUTIL_PRIV_INCLUDES, [$odbc_CPPFLAGS])
      fi
    elif test "$withval" = "no"; then
      :
    else
      AC_PATH_PROG([ODBC_CONFIG],[odbc_config],,[$withval/bin])
      if test "x$ODBC_CONFIG" != 'x'; then
        odbc_CPPFLAGS="-I`$ODBC_CONFIG --include-prefix`"
        odbc_LDFLAGS="-L`$ODBC_CONFIG --lib-prefix`"
        odbc_LIBS="`$ODBC_CONFIG --libs`"
      else
        if test -f "$withval" && test -x "$withval"; then 
          odbc_CPPFLAGS="-I`$withval --include-prefix`"
          odbc_LDFLAGS="-L`$withval --lib-prefix`"
          odbc_LIBS="`$withval --libs`"
        else
          odbc_CPPFLAGS="-I$withval/include"
          odbc_LDFLAGS="-L$withval/lib "
        fi
      fi

      APR_ADDTO(CPPFLAGS, [$odbc_CPPFLAGS])
      APR_ADDTO(LDFLAGS, [$odbc_LDFLAGS])
      APR_ADDTO(LIBS, [$odbc_LIBS])

      AC_MSG_NOTICE(checking for odbc in $withval)
      AC_CHECK_HEADERS(sql.h, AC_CHECK_LIB(odbc, SQLAllocHandle, [apu_have_odbc=1]))
      if test "$apu_have_odbc" = "0"; then
        AC_CHECK_HEADERS(odbc/sql.h, AC_CHECK_LIB(odbc, SQLAllocHandle, [apu_have_odbc=1]))
      fi
      if test "$apu_have_odbc" != "0" && test "x$ODBC_CONFIG" != 'x'; then
        APR_ADDTO(APRUTIL_PRIV_INCLUDES, [$odbc_CPPFLAGS])
      fi
    fi
  ], [
    AC_PATH_PROG([ODBC_CONFIG],[odbc_config])
    if test "x$ODBC_CONFIG" != 'x'; then
      odbc_CPPFLAGS="-I`$ODBC_CONFIG --include-prefix`"
      odbc_LDFLAGS="-L`$ODBC_CONFIG --lib-prefix`"
      odbc_LIBS="`$ODBC_CONFIG --libs`"

      APR_ADDTO(CPPFLAGS, [$odbc_CPPFLAGS])
      APR_ADDTO(LDFLAGS, [$odbc_LDFLAGS])
      APR_ADDTO(LIBS, [$odbc_LIBS])
    fi

    AC_CHECK_HEADERS(sql.h, AC_CHECK_LIB(odbc, SQLAllocHandle, [apu_have_odbc=1]))
    if test "$apu_have_odbc" = "0"; then
      AC_CHECK_HEADERS(odbc/sql.h, AC_CHECK_LIB(odbc, SQLAllocHandle, [apu_have_odbc=1]))
    fi
    if test "$apu_have_odbc" != "0" && test "x$ODBC_CONFIG" != 'x'; then
      APR_ADDTO(APRUTIL_PRIV_INCLUDES, [$odbc_CPPFLAGS])
    fi
  ])
  AC_SUBST(apu_have_odbc)
  dnl Since we have already done the AC_CHECK_LIB tests, if we have it, 
  dnl we know the library is there.
  if test "$apu_have_odbc" = "1"; then
    APR_ADDTO(LDADD_dbd_odbc, [$odbc_LDFLAGS -lodbc $odbc_LIBS])
  fi
  AC_SUBST(LDADD_dbd_odbc)

  LIBS="$old_libs"
  CPPFLAGS="$old_cppflags"
  LDFLAGS="$old_ldflags"

  apu_dbd_tests=""
  test $apu_have_oracle = 1 &&  apu_dbd_tests="$apu_dbd_tests oracle"
  test $apu_have_pgsql = 1 &&   apu_dbd_tests="$apu_dbd_tests pgsql"
  test $apu_have_mysql = 1 &&   apu_dbd_tests="$apu_dbd_tests mysql"
  test $apu_have_sqlite2 = 1 && apu_dbd_tests="$apu_dbd_tests sqlite2"
  test $apu_have_sqlite3 = 1 && apu_dbd_tests="$apu_dbd_tests sqlite3"
  test $apu_have_freetds = 1 && apu_dbd_tests="$apu_dbd_tests freetds"
  test $apu_have_odbc = 1 &&    apu_dbd_tests="$apu_dbd_tests odbc"
  AC_SUBST(apu_dbd_tests)
])
