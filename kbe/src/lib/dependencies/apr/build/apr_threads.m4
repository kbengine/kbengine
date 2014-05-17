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

dnl -----------------------------------------------------------------
dnl apr_threads.m4: APR's autoconf macros for testing thread support
dnl

dnl
dnl APR_CHECK_PTHREADS_H([ ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
dnl
dnl gcc issues warnings when parsing AIX 4.3.3's pthread.h
dnl which causes autoconf to incorrectly conclude that
dnl pthreads is not available.
dnl Turn off warnings if we're using gcc.
dnl
AC_DEFUN(APR_CHECK_PTHREADS_H, [
  if test "$GCC" = "yes"; then
    SAVE_FL="$CPPFLAGS"
    CPPFLAGS="$CPPFLAGS -w"
    AC_CHECK_HEADERS(pthread.h, [ $1 ] , [ $2 ] )
    CPPFLAGS="$SAVE_FL"
  else
    AC_CHECK_HEADERS(pthread.h, [ $1 ] , [ $2 ] )
  fi
])dnl


dnl
dnl APR_CHECK_PTHREAD_GETSPECIFIC_TWO_ARGS
dnl
AC_DEFUN(APR_CHECK_PTHREAD_GETSPECIFIC_TWO_ARGS, [
AC_CACHE_CHECK(whether pthread_getspecific takes two arguments, ac_cv_pthread_getspecific_two_args,[
AC_TRY_COMPILE([
#include <pthread.h>
],[
pthread_key_t key;
void *tmp;
pthread_getspecific(key,&tmp);
],[
    ac_cv_pthread_getspecific_two_args=yes
],[
    ac_cv_pthread_getspecific_two_args=no
])
])

if test "$ac_cv_pthread_getspecific_two_args" = "yes"; then
  AC_DEFINE(PTHREAD_GETSPECIFIC_TAKES_TWO_ARGS, 1, [Define if pthread_getspecific() has two args])
fi
])dnl


dnl
dnl APR_CHECK_PTHREAD_ATTR_GETDETACHSTATE_ONE_ARG
dnl
AC_DEFUN(APR_CHECK_PTHREAD_ATTR_GETDETACHSTATE_ONE_ARG, [
AC_CACHE_CHECK(whether pthread_attr_getdetachstate takes one argument, ac_cv_pthread_attr_getdetachstate_one_arg,[
AC_TRY_COMPILE([
#include <pthread.h>
],[
pthread_attr_t *attr;
pthread_attr_getdetachstate(attr);
],[
    ac_cv_pthread_attr_getdetachstate_one_arg=yes
],[
    ac_cv_pthread_attr_getdetachstate_one_arg=no
])
])

if test "$ac_cv_pthread_attr_getdetachstate_one_arg" = "yes"; then
  AC_DEFINE(PTHREAD_ATTR_GETDETACHSTATE_TAKES_ONE_ARG, 1, [Define if pthread_attr_getdetachstate() has one arg])
fi
])dnl


dnl
dnl APR_PTHREADS_TRY_RUN(actions-if-success)
dnl
dnl Try running a program which uses pthreads, executing the
dnl actions-if-success commands on success.
dnl
AC_DEFUN(APR_PTHREADS_TRY_RUN, [
AC_TRY_RUN( [
#include <pthread.h>
#include <stddef.h>

void *thread_routine(void *data) {
    return data;
}

int main() {
    pthread_t thd;
    pthread_mutexattr_t mattr;
    pthread_once_t once_init = PTHREAD_ONCE_INIT;
    int data = 1;
    pthread_mutexattr_init(&mattr);
    return pthread_create(&thd, NULL, thread_routine, &data);
} ], [apr_p_t_r=yes], [apr_p_t_r=no], [apr_p_t_r=no])

if test $apr_p_t_r = yes; then
  $1
fi

])dnl


dnl
dnl APR_PTHREADS_CHECK()
dnl
dnl Try to find a way to enable POSIX threads.  Sets the 
dnl pthreads_working variable to "yes" on success.
dnl
AC_DEFUN([APR_PTHREADS_CHECK], [

AC_CACHE_CHECK([for CFLAGS needed for pthreads], [apr_cv_pthreads_cflags],
[apr_ptc_cflags=$CFLAGS
 for flag in none -kthread -pthread -pthreads -mt -mthreads -Kthread -threads; do 
    CFLAGS=$apr_ptc_cflags
    test "x$flag" != "xnone" && CFLAGS="$CFLAGS $flag"
    APR_PTHREADS_TRY_RUN([
      apr_cv_pthreads_cflags="$flag"
      break
    ])
 done
 CFLAGS=$apr_ptc_cflags
])

if test -n "$apr_cv_pthreads_cflags"; then
   pthreads_working=yes
   if test "x$apr_cv_pthreads_cflags" != "xnone"; then
     APR_ADDTO(CFLAGS,[$apr_cv_pthreads_cflags])
   fi
fi

# The CFLAGS may or may not be sufficient to ensure that libapr
# depends on the pthreads library: some versions of libtool
# drop -pthread when passed on the link line; some versions of
# gcc ignore -pthread when linking a shared object.  So always
# try and add the relevant library to LIBS too.

AC_CACHE_CHECK([for LIBS needed for pthreads], [apr_cv_pthreads_lib], [
  apr_ptc_libs=$LIBS
  for lib in -lpthread -lpthreads -lc_r; do
    LIBS="$apr_ptc_libs $lib"
    APR_PTHREADS_TRY_RUN([
      apr_cv_pthreads_lib=$lib
      break
    ])
  done
  LIBS=$apr_ptc_libs
])

if test -n "$apr_cv_pthreads_lib"; then
   pthreads_working=yes
   APR_ADDTO(LIBS,[$apr_cv_pthreads_lib])
fi

if test "$pthreads_working" = "yes"; then
  threads_result="POSIX Threads found"
else
  threads_result="POSIX Threads not found"
fi
])dnl

dnl
dnl APR_PTHREADS_CHECK_SAVE
dnl APR_PTHREADS_CHECK_RESTORE
dnl
dnl Save the global environment variables that might be modified during
dnl the checks for threading support so that they can restored if the
dnl result is not what the caller wanted.
dnl
AC_DEFUN(APR_PTHREADS_CHECK_SAVE, [
  apr_pthsv_CFLAGS="$CFLAGS"
  apr_pthsv_LIBS="$LIBS"
])dnl

AC_DEFUN(APR_PTHREADS_CHECK_RESTORE, [
  CFLAGS="$apr_pthsv_CFLAGS"
  LIBS="$apr_pthsv_LIBS"
])dnl

dnl
dnl APR_CHECK_SIGWAIT_ONE_ARG
dnl
AC_DEFUN([APR_CHECK_SIGWAIT_ONE_ARG], [
  AC_CACHE_CHECK(whether sigwait takes one argument,ac_cv_sigwait_one_arg,[
  AC_TRY_COMPILE([
#if defined(__NETBSD__) || defined(DARWIN)
    /* When using the unproven-pthreads package, we need to pull in this
     * header to get a prototype for sigwait().  Else things will fail later
     * on.  XXX Should probably be fixed in the unproven-pthreads package.
     * Darwin is declaring sigwait() in the wrong place as well.
     */
#include <pthread.h>
#endif
#include <signal.h>
],[
  sigset_t set;
 
  sigwait(&set);
],[
  ac_cv_sigwait_one_arg=yes
],[
  ac_cv_sigwait_one_arg=no
])])
  if test "$ac_cv_sigwait_one_arg" = "yes"; then
    AC_DEFINE(SIGWAIT_TAKES_ONE_ARG,1,[ ])
  fi
])

dnl Check for recursive mutex support (per SUSv3).
AC_DEFUN([APR_CHECK_PTHREAD_RECURSIVE_MUTEX], [
  AC_CACHE_CHECK([for recursive mutex support], [apr_cv_mutex_recursive],
[AC_TRY_RUN([#include <sys/types.h>
#include <pthread.h>
#include <stdlib.h>

int main() {
    pthread_mutexattr_t attr;
    pthread_mutex_t m;

    exit (pthread_mutexattr_init(&attr) 
          || pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE)
          || pthread_mutex_init(&m, &attr));
}], [apr_cv_mutex_recursive=yes], [apr_cv_mutex_recursive=no], 
[apr_cv_mutex_recursive=no])])

if test "$apr_cv_mutex_recursive" = "yes"; then
   AC_DEFINE([HAVE_PTHREAD_MUTEX_RECURSIVE], 1,
             [Define if recursive pthread mutexes are available])
fi
])

dnl Check for robust process-shared mutex support
AC_DEFUN([APR_CHECK_PTHREAD_ROBUST_SHARED_MUTEX], [
AC_CACHE_CHECK([for robust cross-process mutex support], 
[apr_cv_mutex_robust_shared],
[AC_TRY_RUN([
#include <sys/types.h>
#include <pthread.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    pthread_mutex_t mutex;
    pthread_mutexattr_t attr;

    if (pthread_mutexattr_init(&attr))
        exit(1);
    if (pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED))
        exit(2);
    if (pthread_mutexattr_setrobust_np(&attr, PTHREAD_MUTEX_ROBUST_NP))
        exit(3);
    if (pthread_mutexattr_setprotocol(&attr, PTHREAD_PRIO_INHERIT))
        exit(4);
    if (pthread_mutex_init(&mutex, &attr))
        exit(5);
    if (pthread_mutexattr_destroy(&attr))
        exit(6);
    if (pthread_mutex_destroy(&mutex))
        exit(7);

    exit(0);
}], [apr_cv_mutex_robust_shared=yes], [apr_cv_mutex_robust_shared=no])])

if test "$apr_cv_mutex_robust_shared" = "yes"; then
   AC_DEFINE([HAVE_PTHREAD_MUTEX_ROBUST], 1,
             [Define if cross-process robust mutexes are available])
fi
])
