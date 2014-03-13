
dnl if $2 contains '@dd', links against mingw symbols
dnl otherwise calls AC_CHECK_LIB
AC_DEFUN([APR_CHECK_DLL_FUNC],[
m4_define($1_function_name,m4_substr($2,0,m4_index($2,[@])))
m4_define($1_function_arglength,m4_substr($2,m4_incr(m4_index($2,[@]))))
m4_define($1_[function_name]_arglength,m4_substr($2,m4_incr(m4_index($2,[@]))))
dnl m4_define(apr_check_dll_id,$1_m4_defn($1_function_name))

AC_CACHE_CHECK([for $2 in $1],[ac_cv_lib_$1_]$1_function_name,[

ac_func_search_save_LIBS=$LIBS
LIBS="$LIBS -l$1"

AC_TRY_LINK([
#pragma pack(1)
struct x {
]m4_for([byte_id], 1, m4_defn([$1_function_name_arglength]), 1,[[ char c]]byte_id;
)[};
__stdcall ]$1_function_name[(]struct x[);],[
struct x s = {0};
]$1_function_name[(s)],
[ac_cv_lib_$1_]$1_function_name[=yes],[ac_cv_lib_$1_]$1_function_name[=no])
LIBS=$ac_func_search_save_LIBS
])dnl AC_CACHE_CHECK

AS_IF([test $ac_cv_lib_$1_]$1_function_name[ = yes],
      [m4_default([$3], [AC_DEFINE_UNQUOTED(AS_TR_CPP(HAVE_LIB$1),,Enable if this library is available)
  LIBS="-l$1 $LIBS"
])],
    [$4])dnl
])

