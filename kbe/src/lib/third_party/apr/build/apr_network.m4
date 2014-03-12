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
dnl apr_network.m4: APR's autoconf macros for testing network support
dnl

dnl
dnl check for type in_addr
dnl
AC_DEFUN(APR_TYPE_IN_ADDR,[
  AC_CACHE_CHECK(for type in_addr, ac_cv_type_in_addr,[
  AC_TRY_COMPILE([
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_WINSOCK2_H
#include <winsock2.h>
#endif
],[
 struct in_addr arg;
 arg.s_addr = htonl(INADDR_ANY);
], [ ac_cv_type_in_addr="yes"] , [
ac_cv_type_in_addr="no"])
])
])

dnl
dnl check for working getaddrinfo()
dnl
dnl Note that if the system doesn't have gai_strerror(), we
dnl can't use getaddrinfo() because we can't get strings
dnl describing the error codes.
dnl
AC_DEFUN([APR_CHECK_WORKING_GETADDRINFO], [
  AC_CACHE_CHECK(for working getaddrinfo, ac_cv_working_getaddrinfo,[
  AC_TRY_RUN( [
#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif

int main(void) {
    struct addrinfo hints, *ai;
    int error;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    error = getaddrinfo("127.0.0.1", NULL, &hints, &ai);
    if (error) {
        exit(1);
    }
    if (ai->ai_addr->sa_family != AF_INET) {
        exit(1);
    }
    exit(0);
}
],[
  ac_cv_working_getaddrinfo="yes"
],[
  ac_cv_working_getaddrinfo="no"
],[
  ac_cv_working_getaddrinfo="yes"
])])
if test "$ac_cv_working_getaddrinfo" = "yes"; then
  if test "$ac_cv_func_gai_strerror" != "yes"; then
    ac_cv_working_getaddrinfo="no"
  else
    AC_DEFINE(HAVE_GETADDRINFO, 1, [Define if getaddrinfo exists and works well enough for APR])
  fi
fi
])

dnl Check whether the AI_ADDRCONFIG flag can be used with getaddrinfo
AC_DEFUN([APR_CHECK_GETADDRINFO_ADDRCONFIG], [
  AC_CACHE_CHECK(for working AI_ADDRCONFIG, apr_cv_gai_addrconfig, [
  AC_TRY_RUN([
#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif

int main(int argc, char **argv) {
    struct addrinfo hints, *ai;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_ADDRCONFIG;
    return getaddrinfo("localhost", NULL, &hints, &ai) != 0;
}], [apr_cv_gai_addrconfig=yes], 
    [apr_cv_gai_addrconfig=no],
    [apr_cv_gai_addrconfig=no])])

if test $apr_cv_gai_addrconfig = yes; then
   AC_DEFINE(HAVE_GAI_ADDRCONFIG, 1, [Define if getaddrinfo accepts the AI_ADDRCONFIG flag])
fi
])

dnl
dnl check for working getnameinfo()
dnl
AC_DEFUN([APR_CHECK_WORKING_GETNAMEINFO], [
  AC_CACHE_CHECK(for working getnameinfo, ac_cv_working_getnameinfo,[
  AC_TRY_RUN( [
#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

int main(void) {
    struct sockaddr_in sa;
    char hbuf[256];
    int error;

    sa.sin_family = AF_INET;
    sa.sin_port = 0;
    sa.sin_addr.s_addr = inet_addr("127.0.0.1");
#ifdef SIN6_LEN
    sa.sin_len = sizeof(sa);
#endif

    error = getnameinfo((const struct sockaddr *)&sa, sizeof(sa),
                        hbuf, 256, NULL, 0,
                        NI_NUMERICHOST);
    if (error) {
        exit(1);
    } else {
        exit(0);
    }
}
],[
  ac_cv_working_getnameinfo="yes"
],[
  ac_cv_working_getnameinfo="no"
],[
  ac_cv_working_getnameinfo="yes"
])])
if test "$ac_cv_working_getnameinfo" = "yes"; then
  AC_DEFINE(HAVE_GETNAMEINFO, 1, [Define if getnameinfo exists])
fi
])

dnl
dnl check for negative error codes for getaddrinfo()
dnl
AC_DEFUN([APR_CHECK_NEGATIVE_EAI], [
  AC_CACHE_CHECK(for negative error codes for getaddrinfo, ac_cv_negative_eai,[
  AC_TRY_RUN( [
#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif

int main(void) {
    if (EAI_ADDRFAMILY < 0) {
        exit(0);
    }
    exit(1);
}
],[
  ac_cv_negative_eai="yes"
],[
  ac_cv_negative_eai="no"
],[
  ac_cv_negative_eai="no"
])])
if test "$ac_cv_negative_eai" = "yes"; then
  AC_DEFINE(NEGATIVE_EAI, 1, [Define if EAI_ error codes from getaddrinfo are negative])
fi
])

dnl
dnl Checks the definition of gethostbyname_r and gethostbyaddr_r
dnl which are different for glibc, solaris and assorted other operating
dnl systems
dnl
dnl Note that this test is executed too early to see if we have all of
dnl the headers.
AC_DEFUN([APR_CHECK_GETHOSTBYNAME_R_STYLE], [

dnl Try and compile a glibc2 gethostbyname_r piece of code, and set the
dnl style of the routines to glibc2 on success
AC_CACHE_CHECK([style of gethostbyname_r routine], ac_cv_gethostbyname_r_style,
APR_TRY_COMPILE_NO_WARNING([
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
],[
int tmp = gethostbyname_r((const char *) 0, (struct hostent *) 0, 
                          (char *) 0, 0, (struct hostent **) 0, &tmp);
/* use tmp to suppress the warning */
tmp=0;
], ac_cv_gethostbyname_r_style=glibc2, ac_cv_gethostbyname_r_style=none))

if test "$ac_cv_gethostbyname_r_style" = "glibc2"; then
    AC_DEFINE(GETHOSTBYNAME_R_GLIBC2, 1, [Define if gethostbyname_r has the glibc style])
fi

AC_CACHE_CHECK([3rd argument to the gethostbyname_r routines], ac_cv_gethostbyname_r_arg,
APR_TRY_COMPILE_NO_WARNING([
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
],[
int tmp = gethostbyname_r((const char *) 0, (struct hostent *) 0, 
                          (struct hostent_data *) 0);
/* use tmp to suppress the warning */
tmp=0;
], ac_cv_gethostbyname_r_arg=hostent_data, ac_cv_gethostbyname_r_arg=char))

if test "$ac_cv_gethostbyname_r_arg" = "hostent_data"; then
    AC_DEFINE(GETHOSTBYNAME_R_HOSTENT_DATA, 1, [Define if gethostbyname_r has the hostent_data for the third argument])
fi
])

dnl
dnl Checks the definition of getservbyname_r
dnl which are different for glibc, solaris and assorted other operating
dnl systems
dnl
dnl Note that this test is executed too early to see if we have all of
dnl the headers.
AC_DEFUN([APR_CHECK_GETSERVBYNAME_R_STYLE], [

dnl Try and compile a glibc2 getservbyname_r piece of code, and set the
dnl style of the routines to glibc2 on success
AC_CACHE_CHECK([style of getservbyname_r routine], ac_cv_getservbyname_r_style, [
APR_TRY_COMPILE_NO_WARNING([
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
],[
int tmp = getservbyname_r((const char *) 0, (const char *) 0,
                          (struct servent *) 0, (char *) 0, 0,
                          (struct servent **) 0);
/* use tmp to suppress the warning */
tmp=0;
], ac_cv_getservbyname_r_style=glibc2, ac_cv_getservbyname_r_style=none)

if test "$ac_cv_getservbyname_r_style" = "none"; then
    dnl Try and compile a Solaris getservbyname_r piece of code, and set the
    dnl style of the routines to solaris on success
    APR_TRY_COMPILE_NO_WARNING([
    #ifdef HAVE_SYS_TYPES_H
    #include <sys/types.h>
    #endif
    #ifdef HAVE_NETINET_IN_H
    #include <netinet/in.h>
    #endif
    #ifdef HAVE_ARPA_INET_H
    #include <arpa/inet.h>
    #endif
    #ifdef HAVE_NETDB_H
    #include <netdb.h>
    #endif
    #ifdef HAVE_STDLIB_H
    #include <stdlib.h>
    #endif
    ],[
    struct servent *tmp = getservbyname_r((const char *) 0, (const char *) 0,
                                          (struct servent *) 0, (char *) 0, 0);
    /* use tmp to suppress the warning */
    tmp=NULL;
    ], ac_cv_getservbyname_r_style=solaris, ac_cv_getservbyname_r_style=none)
fi

if test "$ac_cv_getservbyname_r_style" = "none"; then
    dnl Try and compile a OSF/1 getservbyname_r piece of code, and set the
    dnl style of the routines to osf1 on success
    APR_TRY_COMPILE_NO_WARNING([
    #ifdef HAVE_SYS_TYPES_H
    #include <sys/types.h>
    #endif
    #ifdef HAVE_NETINET_IN_H
    #include <netinet/in.h>
    #endif
    #ifdef HAVE_ARPA_INET_H
    #include <arpa/inet.h>
    #endif
    #ifdef HAVE_NETDB_H
    #include <netdb.h>
    #endif
    #ifdef HAVE_STDLIB_H
    #include <stdlib.h>
    #endif
    ],[
    int tmp = getservbyname_r((const char *) 0, (const char *) 0,
                              (struct servent *) 0, (struct servent_data *) 0);
    /* use tmp to suppress the warning */
    tmp=0;
    ], ac_cv_getservbyname_r_style=osf1, ac_cv_getservbyname_r_style=none)
fi
])

if test "$ac_cv_getservbyname_r_style" = "glibc2"; then
    AC_DEFINE(GETSERVBYNAME_R_GLIBC2, 1, [Define if getservbyname_r has the glibc style])
elif test "$ac_cv_getservbyname_r_style" = "solaris"; then
    AC_DEFINE(GETSERVBYNAME_R_SOLARIS, 1, [Define if getservbyname_r has the Solaris style])
elif test "$ac_cv_getservbyname_r_style" = "osf1"; then
    AC_DEFINE(GETSERVBYNAME_R_OSF1, 1, [Define if getservbyname_r has the OSF/1 style])
fi
])

dnl
dnl see if TCP_NODELAY setting is inherited from listening sockets
dnl
AC_DEFUN([APR_CHECK_TCP_NODELAY_INHERITED], [
  AC_CACHE_CHECK(if TCP_NODELAY setting is inherited from listening sockets, ac_cv_tcp_nodelay_inherited,[
  AC_TRY_RUN( [
#include <stdio.h>
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_NETINET_TCP_H
#include <netinet/tcp.h>
#endif
#ifndef HAVE_SOCKLEN_T
typedef int socklen_t;
#endif
int main(void) {
    int listen_s, connected_s, client_s;
    int listen_port, rc;
    struct sockaddr_in sa;
    socklen_t sa_len;
    socklen_t option_len;
    int option;

    listen_s = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_s < 0) {
        perror("socket");
        exit(1);
    }
    option = 1;
    rc = setsockopt(listen_s, IPPROTO_TCP, TCP_NODELAY, &option, sizeof option);
    if (rc < 0) {
        perror("setsockopt TCP_NODELAY");
        exit(1);
    }
    memset(&sa, 0, sizeof sa);
    sa.sin_family = AF_INET;
#ifdef BEOS
    sa.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
#endif
    /* leave port 0 to get ephemeral */
    rc = bind(listen_s, (struct sockaddr *)&sa, sizeof sa);
    if (rc < 0) {
        perror("bind for ephemeral port");
        exit(1);
    }
    /* find ephemeral port */
    sa_len = sizeof(sa);
    rc = getsockname(listen_s, (struct sockaddr *)&sa, &sa_len);
    if (rc < 0) {
        perror("getsockname");
        exit(1);
    }
    listen_port = sa.sin_port;
    rc = listen(listen_s, 5);
    if (rc < 0) {
        perror("listen");
        exit(1);
    }
    client_s = socket(AF_INET, SOCK_STREAM, 0);
    if (client_s < 0) {
        perror("socket");
        exit(1);
    }
    memset(&sa, 0, sizeof sa);
    sa.sin_family = AF_INET;
    sa.sin_port   = listen_port;
#ifdef BEOS
    sa.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
#endif
    /* leave sin_addr all zeros to use loopback */
    rc = connect(client_s, (struct sockaddr *)&sa, sizeof sa);
    if (rc < 0) {
        perror("connect");
        exit(1);
    }
    sa_len = sizeof sa;
    connected_s = accept(listen_s, (struct sockaddr *)&sa, &sa_len);
    if (connected_s < 0) {
        perror("accept");
        exit(1);
    }
    option_len = sizeof option;
    rc = getsockopt(connected_s, IPPROTO_TCP, TCP_NODELAY, &option, &option_len);
    if (rc < 0) {
        perror("getsockopt");
        exit(1);
    }
    if (!option) {
        fprintf(stderr, "TCP_NODELAY is not set in the child.\n");
        exit(1);
    }
    return 0;
}
],[
    ac_cv_tcp_nodelay_inherited="yes"
],[
    ac_cv_tcp_nodelay_inherited="no"
],[
    ac_cv_tcp_nodelay_inherited="yes"
])])
if test "$ac_cv_tcp_nodelay_inherited" = "yes"; then
    tcp_nodelay_inherited=1
else
    tcp_nodelay_inherited=0
fi
])

dnl
dnl Determine whether TCP_NODELAY and TCP_CORK can both be set
dnl on a TCP socket.
dnl
AC_DEFUN([APR_CHECK_TCP_NODELAY_WITH_CORK], [
AC_CACHE_CHECK([whether TCP_NODELAY and TCP_CORK can both be enabled],
[apr_cv_tcp_nodelay_with_cork],
[AC_RUN_IFELSE([AC_LANG_PROGRAM([[
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_NETINET_TCP_H
#include <netinet/tcp.h>
#endif
#include <stdio.h>
#include <stdlib.h>
]], [[
    int fd, flag, rc;

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
       exit(1);
    }

    flag = 1;
    rc = setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof flag);
    if (rc < 0) {
        perror("setsockopt TCP_NODELAY");
        exit(2);
    }

    flag = 1;
    rc = setsockopt(fd, IPPROTO_TCP, TCP_CORK, &flag, sizeof flag);
    if (rc < 0) {
        perror("setsockopt TCP_CORK");
        exit(3);
    }

    exit(0);
]])], [apr_cv_tcp_nodelay_with_cork=yes], [apr_cv_tcp_nodelay_with_cork=no])])

if test "$apr_cv_tcp_nodelay_with_cork" = "yes"; then
  AC_DEFINE([HAVE_TCP_NODELAY_WITH_CORK], 1,
            [Define if TCP_NODELAY and TCP_CORK can be enabled at the same time])
fi
])


dnl
dnl see if O_NONBLOCK setting is inherited from listening sockets
dnl
AC_DEFUN([APR_CHECK_O_NONBLOCK_INHERITED], [
  AC_CACHE_CHECK(if O_NONBLOCK setting is inherited from listening sockets, ac_cv_o_nonblock_inherited,[
  AC_TRY_RUN( [
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_NETINET_TCP_H
#include <netinet/tcp.h>
#endif
#ifndef HAVE_SOCKLEN_T
typedef int socklen_t;
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
int main(void) {
    int listen_s, connected_s, client_s;
    int listen_port, rc;
    struct sockaddr_in sa;
    socklen_t sa_len;
    fd_set fds;
    struct timeval tv;

    listen_s = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_s < 0) {
        perror("socket");
        exit(1);
    }
    memset(&sa, 0, sizeof sa);
    sa.sin_family = AF_INET;
#ifdef BEOS
    sa.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
#endif
    /* leave port 0 to get ephemeral */
    rc = bind(listen_s, (struct sockaddr *)&sa, sizeof sa);
    if (rc < 0) {
        perror("bind for ephemeral port");
        exit(1);
    }
    /* find ephemeral port */
    sa_len = sizeof(sa);
    rc = getsockname(listen_s, (struct sockaddr *)&sa, &sa_len);
    if (rc < 0) {
        perror("getsockname");
        exit(1);
    }
    listen_port = sa.sin_port;
    rc = listen(listen_s, 5);
    if (rc < 0) {
        perror("listen");
        exit(1);
    }
    rc = fcntl(listen_s, F_SETFL, O_NONBLOCK);
    if (rc < 0) {
        perror("fcntl(F_SETFL)");
        exit(1);
    }
    client_s = socket(AF_INET, SOCK_STREAM, 0);
    if (client_s < 0) {
        perror("socket");
        exit(1);
    }
    memset(&sa, 0, sizeof sa);
    sa.sin_family = AF_INET;
    sa.sin_port   = listen_port;
#ifdef BEOS
    sa.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
#endif
    /* leave sin_addr all zeros to use loopback */
    rc = connect(client_s, (struct sockaddr *)&sa, sizeof sa);
    if (rc < 0) {
        perror("connect");
        exit(1);
    }
    sa_len = sizeof sa;
    /* 1 second select timeout */
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    /* Set up fd set */
    FD_ZERO(&fds);
    FD_SET(listen_s, &fds);
    /* Wait for socket to become readable */
    rc = select(listen_s + 1, &fds, NULL, NULL, &tv);
    if (rc < 0) {
        perror("select");
        exit(1);
    }
    if (rc == 0) {
        fprintf(stderr, "Socket failed to become readable (timeout)\n");
        exit(1);
    }
    if (!FD_ISSET(listen_s, &fds)) {
        fprintf(stderr, "Socket failed to become readable (selected another fd)\n");
        exit(1);
    }
    connected_s = accept(listen_s, (struct sockaddr *)&sa, &sa_len);
    if (connected_s < 0) {
        perror("accept");
        exit(1);
    }
    rc = fcntl(connected_s, F_GETFL, 0);
    if (rc < 0) {
        perror("fcntl(F_GETFL)");
        exit(1);
    }
    if (!(rc & O_NONBLOCK)) {
        fprintf(stderr, "O_NONBLOCK is not set in the child.\n");
        exit(1);
    }
    return 0;
}
],[
    ac_cv_o_nonblock_inherited="yes"
],[
    ac_cv_o_nonblock_inherited="no"
],[
    ac_cv_o_nonblock_inherited="yes"
])])
if test "$ac_cv_o_nonblock_inherited" = "yes"; then
    o_nonblock_inherited=1
else
    o_nonblock_inherited=0
fi
])

dnl 
dnl check for socklen_t, fall back to unsigned int
dnl
AC_DEFUN([APR_CHECK_SOCKLEN_T], [
AC_CACHE_CHECK(for socklen_t, ac_cv_socklen_t,[
AC_TRY_COMPILE([
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
],[
socklen_t foo = (socklen_t) 0;
],[
    ac_cv_socklen_t=yes
],[
    ac_cv_socklen_t=no
])
])

if test "$ac_cv_socklen_t" = "yes"; then
  AC_DEFINE(HAVE_SOCKLEN_T, 1, [Whether you have socklen_t])
fi
])


AC_DEFUN([APR_CHECK_INET_ADDR], [
AC_CACHE_CHECK(for inet_addr, ac_cv_func_inet_addr,[
AC_TRY_COMPILE([
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
],[
inet_addr("127.0.0.1");
],[
    ac_cv_func_inet_addr=yes
],[
    ac_cv_func_inet_addr=no
])
])

if test "$ac_cv_func_inet_addr" = "yes"; then
  have_inet_addr=1
else
  have_inet_addr=0
fi
])


AC_DEFUN([APR_CHECK_INET_NETWORK], [
AC_CACHE_CHECK(for inet_network, ac_cv_func_inet_network,[
AC_TRY_COMPILE([
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
],[
inet_network("127.0.0.1");
],[
    ac_cv_func_inet_network=yes
],[
    ac_cv_func_inet_network=no
])
])

if test "$ac_cv_func_inet_network" = "yes"; then
  have_inet_network=1
else
  have_inet_network=0
fi
])

dnl Check for presence of struct sockaddr_storage.
AC_DEFUN([APR_CHECK_SOCKADDR_STORAGE], [
AC_CACHE_CHECK(for sockaddr_storage, apr_cv_define_sockaddr_storage,[
AC_TRY_COMPILE([
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
],[struct sockaddr_storage sa;],
[apr_cv_define_sockaddr_storage=yes],
[apr_cv_define_sockaddr_storage=no])])

if test "$apr_cv_define_sockaddr_storage" = "yes"; then
  have_sa_storage=1
else
  have_sa_storage=0
fi
AC_SUBST(have_sa_storage)
])

dnl Check for presence of struct sockaddr_in6.
AC_DEFUN([APR_CHECK_SOCKADDR_IN6], [
AC_CACHE_CHECK(for sockaddr_in6, ac_cv_define_sockaddr_in6,[
AC_TRY_COMPILE([
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
],[
struct sockaddr_in6 sa;
],[
    ac_cv_define_sockaddr_in6=yes
],[
    ac_cv_define_sockaddr_in6=no
])
])

if test "$ac_cv_define_sockaddr_in6" = "yes"; then
  have_sockaddr_in6=1
else
  have_sockaddr_in6=0
fi
])

dnl
dnl APR_H_ERRNO_COMPILE_CHECK
dnl
AC_DEFUN([APR_H_ERRNO_COMPILE_CHECK], [
  if test x$1 != x; then
    CPPFLAGS="-D$1 $CPPFLAGS"
  fi
  AC_TRY_COMPILE([
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif
],[
int h_e = h_errno;
],[
  if test x$1 != x; then
    ac_cv_h_errno_cppflags="$1"
  else
    ac_cv_h_errno_cppflags=yes
  fi
],[
  ac_cv_h_errno_cppflags=no
])])


dnl
dnl APR_CHECK_SCTP
dnl
dnl check for presence of SCTP protocol support
dnl
AC_DEFUN([APR_CHECK_SCTP],
[
  AC_CACHE_CHECK([whether SCTP is supported], [apr_cv_sctp], [
  AC_TRY_RUN([
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_NETINET_SCTP_H
#include <netinet/sctp.h>
#endif
#ifdef HAVE_NETINET_SCTP_UIO_H
#include <netinet/sctp_uio.h>
#endif
#include <stdlib.h>
int main(void) {
    int s, opt = 1;
    if ((s = socket(AF_INET, SOCK_STREAM, IPPROTO_SCTP)) < 0)
       exit(1);
    if (setsockopt(s, IPPROTO_SCTP, SCTP_NODELAY, &opt, sizeof(int)) < 0)
       exit(2);
    exit(0);
}], [apr_cv_sctp=yes], [apr_cv_sctp=no], [apr_cv_sctp=no])])

if test "$apr_cv_sctp" = "yes"; then
    have_sctp=1
else
    have_sctp=0
fi
])

dnl APR_CHECK_MCAST: check for multicast interfaces
AC_DEFUN([APR_CHECK_MCAST], [
AC_CACHE_CHECK([for struct ip_mreq], [apr_cv_struct_ipmreq], [
AC_TRY_COMPILE([
#include <sys/types.h>
#include <netinet/in.h>
], [
    struct ip_mreq mip;
    mip.imr_interface.s_addr = INADDR_ANY;
], [apr_cv_struct_ipmreq=yes], [apr_cv_struct_ipmreq=no], [apr_cv_struct_ipmreq=yes])])

if test $apr_cv_struct_ipmreq = yes; then
   AC_DEFINE([HAVE_STRUCT_IPMREQ], 1, [Define if struct impreq was found])
fi
])

dnl
dnl APR_CHECK_H_ERRNO_FLAG
dnl
dnl checks which flags are necessary for <netdb.h> to define h_errno
dnl
AC_DEFUN([APR_CHECK_H_ERRNO_FLAG], [
  AC_MSG_CHECKING([for h_errno in netdb.h])
  AC_CACHE_VAL(ac_cv_h_errno_cppflags,[
    APR_H_ERRNO_COMPILE_CHECK
    if test "$ac_cv_h_errno_cppflags" = "no"; then
      ac_save="$CPPFLAGS"
      for flag in _XOPEN_SOURCE_EXTENDED; do
        APR_H_ERRNO_COMPILE_CHECK($flag)
        if test "$ac_cv_h_errno_cppflags" != "no"; then
          break
        fi
      done
      CPPFLAGS="$ac_save"
    fi
  ])
  if test "$ac_cv_h_errno_cppflags" != "no"; then
    if test "$ac_cv_h_errno_cppflags" != "yes"; then
      CPPFLAGS="-D$ac_cv_h_errno_cppflags $CPPFLAGS"
      AC_MSG_RESULT([yes, with -D$ac_cv_h_errno_cppflags])
    else
      AC_MSG_RESULT([$ac_cv_h_errno_cppflags])
    fi
  else
    AC_MSG_RESULT([$ac_cv_h_errno_cppflags])
  fi
])


AC_DEFUN([APR_EBCDIC], [
  AC_CACHE_CHECK([whether system uses EBCDIC],ac_cv_ebcdic,[
  AC_TRY_RUN( [
int main(void) { 
  return (unsigned char)'A' != (unsigned char)0xC1; 
} 
],[
  ac_cv_ebcdic="yes"
],[
  ac_cv_ebcdic="no"
],[
  ac_cv_ebcdic="no"
])])
  if test "$ac_cv_ebcdic" = "yes"; then
    apr_charset_ebcdic=1
  else
    apr_charset_ebcdic=0
  fi
  AC_SUBST(apr_charset_ebcdic)
])

