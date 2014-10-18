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
dnl apr_hints.m4: APR's autoconf macros for platform-specific hints
dnl
dnl  We preload various configure settings depending
dnl  on previously obtained platform knowledge.
dnl  We allow all settings to be overridden from
dnl  the command-line.
dnl
dnl  We maintain the "format" that we've used
dnl  under 1.3.x, so we don't exactly follow
dnl  what is "recommended" by autoconf.

dnl
dnl APR_PRELOAD
dnl
dnl  Preload various ENV/makefile params such as CC, CFLAGS, etc
dnl  based on outside knowledge
dnl
dnl  Generally, we force the setting of CC, and add flags
dnl  to CFLAGS, CPPFLAGS, LIBS and LDFLAGS. 
dnl
AC_DEFUN(APR_PRELOAD, [
if test "x$apr_preload_done" != "xyes" ; then

  apr_preload_done="yes"

  echo "Applying APR hints file rules for $host"

  case "$host" in
    *mint)
	APR_ADDTO(CPPFLAGS, [-DMINT -D_GNU_SOURCE])
	;;
    *MPE/iX*)
	APR_ADDTO(CPPFLAGS, [-DMPE -D_POSIX_SOURCE -D_SOCKET_SOURCE])
	APR_ADDTO(LIBS, [-lsvipc -lcurses])
	APR_ADDTO(LDFLAGS, [-Xlinker \"-WL,cap=ia,ba,ph;nmstack=1024000\"])
	;;
    *-apple-aux3*)
	APR_ADDTO(CPPFLAGS, [-DAUX3 -D_POSIX_SOURCE])
	APR_ADDTO(LIBS, [-lposix -lbsd])
	APR_ADDTO(LDFLAGS, [-s])
	APR_SETVAR(SHELL, [/bin/ksh])
	;;
    *-ibm-aix*)
	APR_ADDTO(CPPFLAGS, [-U__STR__ -D_THREAD_SAFE])
        dnl _USR_IRS gets us the hstrerror() proto in netdb.h
        case $host in
            *-ibm-aix4.3)
	        APR_ADDTO(CPPFLAGS, [-D_USE_IRS])
	        ;;
            *-ibm-aix5*)
	        APR_ADDTO(CPPFLAGS, [-D_USE_IRS])
	        ;;
            *-ibm-aix4.3.*)
                APR_ADDTO(CPPFLAGS, [-D_USE_IRS])
                ;;
        esac
        dnl If using xlc, remember it, and give it the right options.
        if $CC 2>&1 | grep 'xlc' > /dev/null; then
          APR_SETIFNULL(AIX_XLC, [yes])
          APR_ADDTO(CFLAGS, [-qHALT=E])
        fi
	APR_SETIFNULL(apr_sysvsem_is_global, [yes])
	APR_SETIFNULL(apr_lock_method, [USE_SYSVSEM_SERIALIZE])
        case $host in
            *-ibm-aix3* | *-ibm-aix4.1.*)
                ;;
            *)
                APR_ADDTO(LDFLAGS, [-Wl,-brtl])
                ;;
	esac
        ;;
    *-apollo-*)
	APR_ADDTO(CPPFLAGS, [-DAPOLLO])
	;;
    *-dg-dgux*)
	APR_ADDTO(CPPFLAGS, [-DDGUX])
	;;
    *-os2*)
	APR_SETVAR(SHELL, [sh])
	APR_SETIFNULL(apr_gethostbyname_is_thread_safe, [yes])
	APR_SETIFNULL(apr_gethostbyaddr_is_thread_safe, [yes])
	APR_SETIFNULL(apr_getservbyname_is_thread_safe, [yes])
	;;
    *-hi-hiux)
	APR_ADDTO(CPPFLAGS, [-DHIUX])
	;;
    *-hp-hpux11.*)
	APR_ADDTO(CPPFLAGS, [-DHPUX11 -D_REENTRANT -D_HPUX_SOURCE])
	;;
    *-hp-hpux10.*)
 	case $host in
 	  *-hp-hpux10.01)
dnl	       # We know this is a problem in 10.01.
dnl	       # Not a problem in 10.20.  Otherwise, who knows?
	       APR_ADDTO(CPPFLAGS, [-DSELECT_NEEDS_CAST])
	       ;;	     
 	esac
	APR_ADDTO(CPPFLAGS, [-D_REENTRANT])
	;;
    *-hp-hpux*)
	APR_ADDTO(CPPFLAGS, [-DHPUX -D_REENTRANT])
	;;
    *-linux*)
	APR_ADDTO(CPPFLAGS, [-DLINUX -D_REENTRANT -D_GNU_SOURCE])
	;;
    *-lynx-lynxos)
	APR_ADDTO(CPPFLAGS, [-D__NO_INCLUDE_WARN__ -DLYNXOS])
	APR_ADDTO(LIBS, [-lbsd])
	;;
    *486-*-bsdi*)
	APR_ADDTO(CFLAGS, [-m486])
	;;
    *-*-bsdi*)
        case $host in
            *bsdi4.1)
                APR_ADDTO(CFLAGS, [-D_REENTRANT])
                ;;
        esac
        ;;
    *-openbsd*)
	APR_ADDTO(CPPFLAGS, [-D_POSIX_THREADS])
        # binding to an ephemeral port fails on OpenBSD so override
        # the test for O_NONBLOCK inheritance across accept().
        APR_SETIFNULL(ac_cv_o_nonblock_inherited, [yes])
	;;
    *-netbsd*)
	APR_ADDTO(CPPFLAGS, [-DNETBSD])
        # fcntl() lies about O_NONBLOCK on an accept()ed socket (PR kern/26950)
        APR_SETIFNULL(ac_cv_o_nonblock_inherited, [yes])
	;;
    *-freebsd*)
        APR_SETIFNULL(apr_lock_method, [USE_FLOCK_SERIALIZE])
        if test -x /sbin/sysctl; then
            os_version=`/sbin/sysctl -n kern.osreldate`
        else
            os_version=000000
        fi
        # 502102 is when libc_r switched to libpthread (aka libkse).
        if test $os_version -ge "502102"; then
          apr_cv_pthreads_cflags="none"
          apr_cv_pthreads_lib="-lpthread"
        else
          APR_ADDTO(CPPFLAGS, [-D_THREAD_SAFE -D_REENTRANT])
          APR_SETIFNULL(enable_threads, [no])
        fi
        # prevent use of KQueue before FreeBSD 4.8
        if test $os_version -lt "480000"; then
          APR_SETIFNULL(ac_cv_func_kqueue, no)
        fi
	;;
    *-k*bsd*-gnu)
        APR_ADDTO(CPPFLAGS, [-D_REENTRANT -D_GNU_SOURCE])
        ;;
    *-gnu*|*-GNU*)
        APR_ADDTO(CPPFLAGS, [-D_REENTRANT -D_GNU_SOURCE -DHURD])
        ;;
    *-next-nextstep*)
	APR_SETIFNULL(CFLAGS, [-O])
	APR_ADDTO(CPPFLAGS, [-DNEXT])
	;;
    *-next-openstep*)
	APR_SETIFNULL(CFLAGS, [-O])
	APR_ADDTO(CPPFLAGS, [-DNEXT])
	;;
    *-apple-rhapsody*)
	APR_ADDTO(CPPFLAGS, [-DRHAPSODY])
	;;
    *-apple-darwin*)
        APR_ADDTO(CPPFLAGS, [-DDARWIN -DSIGPROCMASK_SETS_THREAD_MASK -no-cpp-precomp])
        APR_SETIFNULL(apr_posixsem_is_global, [yes])
        case $host in
            *-apple-darwin[[1-9]].*)
                # APR's use of kqueue has triggered kernel panics for some
                # 10.5.x (Darwin 9.x) users when running the entire test suite.
                # In 10.4.x, use of kqueue would cause the socket tests to hang.
                # 10.6+ (Darwin 10.x is supposed to fix the KQueue issues
                APR_SETIFNULL(ac_cv_func_kqueue, [no]) 
                APR_SETIFNULL(ac_cv_func_poll, [no]) # See issue 34332
            ;;
            *-apple-darwin1?.*)
                APR_ADDTO(CPPFLAGS, [-DDARWIN_10])
            ;;
        esac
	;;
    *-dec-osf*)
	APR_ADDTO(CPPFLAGS, [-DOSF1])
        # process-shared mutexes don't seem to work in Tru64 5.0
        APR_SETIFNULL(apr_cv_process_shared_works, [no])
	;;
    *-nto-qnx*)
	;;
    *-qnx)
	APR_ADDTO(CPPFLAGS, [-DQNX])
	APR_ADDTO(LIBS, [-N128k -lunix])
	;;
    *-qnx32)
	APR_ADDTO(CPPFLAGS, [-DQNX])
	APR_ADDTO(CFLAGS, [-mf -3])
	APR_ADDTO(LIBS, [-N128k -lunix])
	;;
    *-isc4*)
	APR_ADDTO(CPPFLAGS, [-posix -DISC])
	APR_ADDTO(LDFLAGS, [-posix])
	APR_ADDTO(LIBS, [-linet])
	;;
    *-sco3.2v[[234]]*)
	APR_ADDTO(CPPFLAGS, [-DSCO -D_REENTRANT])
	if test "$GCC" = "no"; then
	    APR_ADDTO(CFLAGS, [-Oacgiltz])
	fi
	APR_ADDTO(LIBS, [-lPW -lmalloc])
	;;
    *-sco3.2v5*)
	APR_ADDTO(CPPFLAGS, [-DSCO5 -D_REENTRANT])
	;;
    *-sco_sv*|*-SCO_SV*)
	APR_ADDTO(CPPFLAGS, [-DSCO -D_REENTRANT])
	APR_ADDTO(LIBS, [-lPW -lmalloc])
	;;
    *-solaris2*)
    	PLATOSVERS=`echo $host | sed 's/^.*solaris2.//'`
	APR_ADDTO(CPPFLAGS, [-DSOLARIS2=$PLATOSVERS -D_POSIX_PTHREAD_SEMANTICS -D_REENTRANT])
        if test $PLATOSVERS -ge 10; then
            APR_SETIFNULL(apr_lock_method, [USE_PROC_PTHREAD_SERIALIZE])
        else
            APR_SETIFNULL(apr_lock_method, [USE_FCNTL_SERIALIZE])
        fi
        # readdir64_r error handling seems broken on Solaris (at least
        # up till 2.8) -- it will return -1 at end-of-directory.
        APR_SETIFNULL(ac_cv_func_readdir64_r, [no])
	;;
    *-sunos4*)
	APR_ADDTO(CPPFLAGS, [-DSUNOS4])
	;;
    *-unixware1)
	APR_ADDTO(CPPFLAGS, [-DUW=100])
	;;
    *-unixware2)
	APR_ADDTO(CPPFLAGS, [-DUW=200])
	APR_ADDTO(LIBS, [-lgen])
	;;
    *-unixware211)
	APR_ADDTO(CPPFLAGS, [-DUW=211])
	APR_ADDTO(LIBS, [-lgen])
	;;
    *-unixware212)
	APR_ADDTO(CPPFLAGS, [-DUW=212])
	APR_ADDTO(LIBS, [-lgen])
	;;
    *-unixware7)
	APR_ADDTO(CPPFLAGS, [-DUW=700])
	APR_ADDTO(LIBS, [-lgen])
	;;
    maxion-*-sysv4*)
	APR_ADDTO(CPPFLAGS, [-DSVR4])
	APR_ADDTO(LIBS, [-lc -lgen])
	;;
    *-*-powermax*)
	APR_ADDTO(CPPFLAGS, [-DSVR4])
	APR_ADDTO(LIBS, [-lgen])
	;;
    TPF)
       APR_ADDTO(CPPFLAGS, [-DTPF -D_POSIX_SOURCE])
       ;;
    bs2000*-siemens-sysv*)
	APR_SETIFNULL(CFLAGS, [-O])
	APR_ADDTO(CPPFLAGS, [-DSVR4 -D_XPG_IV -D_KMEMUSER])
	APR_ADDTO(LIBS, [-lsocket])
	APR_SETIFNULL(enable_threads, [no])
	;;
    *-siemens-sysv4*)
	APR_ADDTO(CPPFLAGS, [-DSVR4 -D_XPG_IV -DHAS_DLFCN -DUSE_MMAP_FILES -DUSE_SYSVSEM_SERIALIZED_ACCEPT])
	APR_ADDTO(LIBS, [-lc])
	;;
    pyramid-pyramid-svr4)
	APR_ADDTO(CPPFLAGS, [-DSVR4 -DNO_LONG_DOUBLE])
	APR_ADDTO(LIBS, [-lc])
	;;
    DS/90\ 7000-*-sysv4*)
	APR_ADDTO(CPPFLAGS, [-DUXPDS])
	;;
    *-tandem-sysv4*)
	APR_ADDTO(CPPFLAGS, [-DSVR4])
	;;
    *-ncr-sysv4)
	APR_ADDTO(CPPFLAGS, [-DSVR4 -DMPRAS])
	APR_ADDTO(LIBS, [-lc -L/usr/ucblib -lucb])
	;;
    *-sysv4*)
	APR_ADDTO(CPPFLAGS, [-DSVR4])
	APR_ADDTO(LIBS, [-lc])
	;;
    88k-encore-sysv4)
	APR_ADDTO(CPPFLAGS, [-DSVR4 -DENCORE])
	APR_ADDTO(LIBS, [-lPW])
	;;
    *-uts*)
	PLATOSVERS=`echo $host | sed 's/^.*,//'`
	case $PLATOSVERS in
	    2*) APR_ADDTO(CPPFLAGS, [-DUTS21])
	        APR_ADDTO(CFLAGS, [-Xa -eft])
	        APR_ADDTO(LIBS, [-lbsd -la])
	        ;;
	    *)  APR_ADDTO(CPPFLAGS, [-DSVR4])
	        APR_ADDTO(CFLAGS, [-Xa])
	        ;;
	esac
	;;
    *-ultrix)
	APR_ADDTO(CPPFLAGS, [-DULTRIX])
	APR_SETVAR(SHELL, [/bin/sh5])
	;;
    *powerpc-tenon-machten*)
	APR_ADDTO(LDFLAGS, [-Xlstack=0x14000 -Xldelcsect])
	;;
    *-machten*)
	APR_ADDTO(LDFLAGS, [-stack 0x14000])
	;;
    *convex-v11*)
	APR_ADDTO(CPPFLAGS, [-DCONVEXOS11])
	APR_SETIFNULL(CFLAGS, [-O1])
	APR_ADDTO(CFLAGS, [-ext])
	;;
    i860-intel-osf1)
	APR_ADDTO(CPPFLAGS, [-DPARAGON])
	;;
    *-sequent-ptx2.*.*)
	APR_ADDTO(CPPFLAGS, [-DSEQUENT=20])
	APR_ADDTO(CFLAGS, [-Wc,-pw])
	APR_ADDTO(LIBS, [-linet -lc -lseq])
	;;
    *-sequent-ptx4.0.*)
	APR_ADDTO(CPPFLAGS, [-DSEQUENT=40])
	APR_ADDTO(CFLAGS, [-Wc,-pw])
	APR_ADDTO(LIBS, [-linet -lc])
	;;
    *-sequent-ptx4.[[123]].*)
	APR_ADDTO(CPPFLAGS, [-DSEQUENT=41])
	APR_ADDTO(CFLAGS, [-Wc,-pw])
	APR_ADDTO(LIBS, [-lc])
	;;
    *-sequent-ptx4.4.*)
	APR_ADDTO(CPPFLAGS, [-DSEQUENT=44])
	APR_ADDTO(CFLAGS, [-Wc,-pw])
	APR_ADDTO(LIBS, [-lc])
	;;
    *-sequent-ptx4.5.*)
	APR_ADDTO(CPPFLAGS, [-DSEQUENT=45])
	APR_ADDTO(CFLAGS, [-Wc,-pw])
	APR_ADDTO(LIBS, [-lc])
	;;
    *-sequent-ptx5.0.*)
	APR_ADDTO(CPPFLAGS, [-DSEQUENT=50])
	APR_ADDTO(CFLAGS, [-Wc,-pw])
	APR_ADDTO(LIBS, [-lc])
	;;
    *NEWS-OS*)
	APR_ADDTO(CPPFLAGS, [-DNEWSOS])
	;;
    *-riscix)
	APR_ADDTO(CPPFLAGS, [-DRISCIX])
	APR_SETIFNULL(CFLAGS, [-O])
	;;
    *-irix*)
	APR_ADDTO(CPPFLAGS, [-D_POSIX_THREAD_SAFE_FUNCTIONS])
	;;
    *beos*)
        APR_ADDTO(CPPFLAGS, [-DBEOS])
        PLATOSVERS=`uname -r`
        APR_SETIFNULL(apr_process_lock_is_global, [yes])
        case $PLATOSVERS in
            5.0.4)
                APR_ADDTO(LDFLAGS, [-L/boot/beos/system/lib])
                APR_ADDTO(LIBS, [-lbind -lsocket])
                APR_ADDTO(CPPFLAGS,[-DBONE7])
                ;;
            5.1)
                APR_ADDTO(LDFLAGS, [-L/boot/beos/system/lib])
                APR_ADDTO(LIBS, [-lbind -lsocket])
                ;;
	esac
	APR_ADDTO(CPPFLAGS, [-DSIGPROCMASK_SETS_THREAD_MASK])
        ;;
    4850-*.*)
	APR_ADDTO(CPPFLAGS, [-DSVR4 -DMPRAS])
	APR_ADDTO(LIBS, [-lc -L/usr/ucblib -lucb])
	;;
    drs6000*)
	APR_ADDTO(CPPFLAGS, [-DSVR4])
	APR_ADDTO(LIBS, [-lc -L/usr/ucblib -lucb])
	;;
    m88k-*-CX/SX|CYBER)
	APR_ADDTO(CPPFLAGS, [-D_CX_SX])
	APR_ADDTO(CFLAGS, [-Xa])
	;;
    *-tandem-oss)
	APR_ADDTO(CPPFLAGS, [-D_TANDEM_SOURCE -D_XOPEN_SOURCE_EXTENDED=1])
	;;
    *-ibm-os390)
        APR_SETIFNULL(apr_lock_method, [USE_SYSVSEM_SERIALIZE])
        APR_SETIFNULL(apr_sysvsem_is_global, [yes])
        APR_SETIFNULL(apr_gethostbyname_is_thread_safe, [yes])
        APR_SETIFNULL(apr_gethostbyaddr_is_thread_safe, [yes])
        APR_SETIFNULL(apr_getservbyname_is_thread_safe, [yes])
        AC_DEFINE(HAVE_ZOS_PTHREADS, 1, [Define for z/OS pthread API nuances])
        APR_ADDTO(CPPFLAGS, [-U_NO_PROTO -DSIGPROCMASK_SETS_THREAD_MASK -DTCP_NODELAY=1])
        ;;
    *-ibm-as400)
        APR_SETIFNULL(apr_lock_method, [USE_SYSVSEM_SERIALIZE])
        APR_SETIFNULL(apr_process_lock_is_global, [yes])
        APR_SETIFNULL(apr_gethostbyname_is_thread_safe, [yes])
        APR_SETIFNULL(apr_gethostbyaddr_is_thread_safe, [yes])
        APR_SETIFNULL(apr_getservbyname_is_thread_safe, [yes])
        ;;
    *mingw*)
        APR_ADDTO(INTERNAL_CPPFLAGS, -DBINPATH=$apr_builddir/test/.libs)
        APR_ADDTO(CPPFLAGS, [-DWIN32 -D__MSVCRT__])
        APR_ADDTO(LDFLAGS, [-Wl,--enable-auto-import,--subsystem,console])
        APR_SETIFNULL(have_unicode_fs, [1])
        APR_SETIFNULL(have_proc_invoked, [1])
        APR_SETIFNULL(apr_lock_method, [win32])
        APR_SETIFNULL(apr_process_lock_is_global, [yes])
        APR_SETIFNULL(apr_cv_use_lfs64, [yes])
        APR_SETIFNULL(apr_cv_osuuid, [yes])
        APR_SETIFNULL(apr_cv_tcp_nodelay_with_cork, [no])
        APR_SETIFNULL(apr_thread_func, [__stdcall])
        APR_SETIFNULL(ac_cv_o_nonblock_inherited, [yes])
        APR_SETIFNULL(ac_cv_tcp_nodelay_inherited, [yes])
        APR_SETIFNULL(ac_cv_file__dev_zero, [no])
        APR_SETIFNULL(ac_cv_func_setpgrp_void, [no])
        APR_SETIFNULL(ac_cv_func_mmap, [yes])
        APR_SETIFNULL(ac_cv_define_sockaddr_in6, [yes])
        APR_SETIFNULL(ac_cv_working_getaddrinfo, [yes])
        APR_SETIFNULL(ac_cv_working_getnameinfo, [yes])
        APR_SETIFNULL(ac_cv_func_gai_strerror, [yes])
        case $host in
            *mingw32*)
                APR_SETIFNULL(apr_has_xthread_files, [1])
                APR_SETIFNULL(apr_has_user, [1])
                APR_SETIFNULL(apr_procattr_user_set_requires_password, [1])
                dnl The real function is TransmitFile(), not sendfile(), but
                dnl this bypasses the Linux/Solaris/AIX/etc. test and enables
                dnl the TransmitFile() implementation.
                APR_SETIFNULL(ac_cv_func_sendfile, [yes])
                ;;
            *mingwce)
                APR_SETIFNULL(apr_has_xthread_files, [0])
                APR_SETIFNULL(apr_has_user, [0])
                APR_SETIFNULL(apr_procattr_user_set_requires_password, [0])
                APR_SETIFNULL(ac_cv_func_sendfile, [no])
                ;;
        esac
        ;;
  esac

fi
])

dnl
dnl APR_CC_HINTS
dnl
dnl  Allows us to provide a default choice of compiler which
dnl  the user can override.
AC_DEFUN(APR_CC_HINTS, [
case "$host" in
  *-apple-aux3*)
      APR_SETIFNULL(CC, [gcc])
      ;;
  bs2000*-siemens-sysv*)
      APR_SETIFNULL(CC, [c89 -XLLML -XLLMK -XL -Kno_integer_overflow])
      ;;
  *convex-v11*)
      APR_SETIFNULL(CC, [cc])
      ;;
  *-ibm-os390)
      APR_SETIFNULL(CC, [cc])
      ;;
  *-ibm-as400)
      APR_SETIFNULL(CC, [icc])
      ;;
  *-isc4*)
      APR_SETIFNULL(CC, [gcc])
      ;;
  m88k-*-CX/SX|CYBER)
      APR_SETIFNULL(CC, [cc])
      ;;
  *-next-openstep*)
      APR_SETIFNULL(CC, [cc])
      ;;
  *-qnx32)
      APR_SETIFNULL(CC, [cc -F])
      ;;
  *-tandem-oss)
      APR_SETIFNULL(CC, [c89])
      ;;
  TPF)
      APR_SETIFNULL(CC, [c89])
      ;;
esac
])
