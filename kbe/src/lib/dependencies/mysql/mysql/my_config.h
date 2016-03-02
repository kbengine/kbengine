/* Copyright (c) 2009, 2015, Oracle and/or its affiliates. All rights reserved.
 
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; version 2 of the License.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA */

#ifndef MY_CONFIG_H
#define MY_CONFIG_H

#if defined( _WIN64 ) || defined( __x86_64__ ) || defined( __amd64 ) || defined( __LP64__ )


/*
 * From configure.cmake, in order of appearance 
 */
/* #undef HAVE_LLVM_LIBCPP */
#define _LARGEFILE_SOURCE 1

/* Libraries */
/* #undef HAVE_LIBM */
/* #undef HAVE_LIBNSL */
/* #undef HAVE_LIBCRYPT */
/* #undef HAVE_LIBSOCKET */
/* #undef HAVE_LIBDL */
/* #undef HAVE_LIBRT */
/* #undef HAVE_LIBWRAP */

/* Header files */
/* #undef HAVE_ALLOCA_H */
/* #undef HAVE_ARPA_INET_H */
/* #undef HAVE_DIRENT_H */
/* #undef HAVE_DLFCN_H */
/* #undef HAVE_EXECINFO_H */
/* #undef HAVE_FPU_CONTROL_H */
/* #undef HAVE_GRP_H */
/* #undef HAVE_IEEEFP_H */
/* #undef HAVE_LANGINFO_H */
#define HAVE_MALLOC_H 1
/* #undef HAVE_NETINET_IN_H */
/* #undef HAVE_POLL_H */
/* #undef HAVE_PWD_H */
/* #undef HAVE_STRINGS_H */
/* #undef HAVE_SYS_CDEFS_H */
/* #undef HAVE_SYS_IOCTL_H */
/* #undef HAVE_SYS_MMAN_H */
/* #undef HAVE_SYS_RESOURCE_H */
/* #undef HAVE_SYS_SELECT_H */
/* #undef HAVE_SYS_SOCKET_H */
/* #undef HAVE_TERM_H */
/* #undef HAVE_TERMIOS_H */
/* #undef HAVE_TERMIO_H */
/* #undef HAVE_UNISTD_H */
/* #undef HAVE_SYS_WAIT_H */
/* #undef HAVE_SYS_PARAM_H */
/* #undef HAVE_FNMATCH_H */
/* #undef HAVE_SYS_UN_H */
/* #undef HAVE_VIS_H */
/* #undef HAVE_SASL_SASL_H */

/* Libevent */
/* #undef HAVE_DEVPOLL */
/* #undef HAVE_SYS_DEVPOLL_H */
/* #undef HAVE_SYS_EPOLL_H */
/* #undef HAVE_TAILQFOREACH */

/* Functions */
#define HAVE_ALIGNED_MALLOC 1
/* #undef HAVE_BACKTRACE */
/* #undef HAVE_PRINTSTACK */
/* #undef HAVE_INDEX */
/* #undef HAVE_CLOCK_GETTIME */
/* #undef HAVE_CUSERID */
/* #undef HAVE_DIRECTIO */
/* #undef HAVE_FTRUNCATE */
#define HAVE_COMPRESS 1
/* #undef HAVE_CRYPT */
/* #undef HAVE_DLOPEN */
/* #undef HAVE_FCHMOD */
/* #undef HAVE_FCNTL */
/* #undef HAVE_FDATASYNC */
/* #undef HAVE_DECL_FDATASYNC */
/* #undef HAVE_FEDISABLEEXCEPT */
/* #undef HAVE_FSEEKO */
/* #undef HAVE_FSYNC */
/* #undef HAVE_GETHOSTBYADDR_R */
/* #undef HAVE_GETHRTIME */
#define HAVE_GETNAMEINFO 1
/* #undef HAVE_GETPASS */
/* #undef HAVE_GETPASSPHRASE */
/* #undef HAVE_GETPWNAM */
/* #undef HAVE_GETPWUID */
/* #undef HAVE_GETRLIMIT */
/* #undef HAVE_GETRUSAGE */
/* #undef HAVE_INITGROUPS */
/* #undef HAVE_ISSETUGID */
/* #undef HAVE_GETUID */
/* #undef HAVE_GETEUID */
/* #undef HAVE_GETGID */
/* #undef HAVE_GETEGID */
/* #undef HAVE_LSTAT */
/* #undef HAVE_MADVISE */
/* #undef HAVE_MALLOC_INFO */
/* #undef HAVE_MEMRCHR */
/* #undef HAVE_MLOCK */
/* #undef HAVE_MLOCKALL */
/* #undef HAVE_MMAP64 */
/* #undef HAVE_POLL */
/* #undef HAVE_POSIX_FALLOCATE */
/* #undef HAVE_POSIX_MEMALIGN */
/* #undef HAVE_PREAD */
/* #undef HAVE_PTHREAD_CONDATTR_SETCLOCK */
/* #undef HAVE_PTHREAD_SIGMASK */
/* #undef HAVE_READDIR_R */
/* #undef HAVE_READLINK */
/* #undef HAVE_REALPATH */
/* #undef HAVE_SETFD */
/* #undef HAVE_SIGACTION */
/* #undef HAVE_SLEEP */
/* #undef HAVE_STPCPY */
/* #undef HAVE_STPNCPY */
/* #undef HAVE_STRLCPY */
#define HAVE_STRNLEN 1
/* #undef HAVE_STRLCAT */
/* #undef HAVE_STRSIGNAL */
/* #undef HAVE_FGETLN */
/* #undef HAVE_STRSEP */
#define HAVE_TELL 1
/* #undef HAVE_VASPRINTF */
/* #undef HAVE_MEMALIGN */
/* #undef HAVE_NL_LANGINFO */
/* #undef HAVE_HTONLL */
/* #undef DNS_USE_CPU_CLOCK_FOR_ID */
/* #undef HAVE_EPOLL */
/* #undef HAVE_EVENT_PORTS */
#define HAVE_INET_NTOP 1
/* #undef HAVE_WORKING_KQUEUE */
/* #undef HAVE_TIMERADD */
/* #undef HAVE_TIMERCLEAR */
/* #undef HAVE_TIMERCMP */
/* #undef HAVE_TIMERISSET */

/* WL2373 */
/* #undef HAVE_SYS_TIME_H */
/* #undef HAVE_SYS_TIMES_H */
/* #undef HAVE_TIMES */
/* #undef HAVE_GETTIMEOFDAY */

/* Symbols */
/* #undef HAVE_LRAND48 */
/* #undef GWINSZ_IN_SYS_IOCTL */
/* #undef FIONREAD_IN_SYS_IOCTL */
/* #undef FIONREAD_IN_SYS_FILIO */
/* #undef HAVE_SIGEV_THREAD_ID */
/* #undef HAVE_SIGEV_PORT */
#define HAVE_LOG2 1

#define HAVE_ISINF 1

/* #undef HAVE_KQUEUE_TIMERS */
/* #undef HAVE_POSIX_TIMERS */

/* Endianess */
/* #undef WORDS_BIGENDIAN */

/* Type sizes */
#define SIZEOF_VOIDP     8
#define SIZEOF_CHARP     8
#define SIZEOF_LONG      4
#define SIZEOF_SHORT     2
#define SIZEOF_INT       4
#define SIZEOF_LONG_LONG 8
#define SIZEOF_OFF_T     4
#define SIZEOF_TIME_T    8
/* #undef HAVE_UINT */
/* #undef HAVE_ULONG */
/* #undef HAVE_U_INT32_T */
/* #undef HAVE_STRUCT_TIMESPEC */

/* Code tests*/
#define STACK_DIRECTION -1
/* #undef TIME_WITH_SYS_TIME */
#define NO_FCNTL_NONBLOCK 1
/* #undef HAVE_PAUSE_INSTRUCTION */
/* #undef HAVE_FAKE_PAUSE_INSTRUCTION */
/* #undef HAVE_HMT_PRIORITY_INSTRUCTION */
/* #undef HAVE_ABI_CXA_DEMANGLE */
/* #undef HAVE_BSS_START */
/* #undef HAVE_BUILTIN_UNREACHABLE */
/* #undef HAVE_BUILTIN_EXPECT */
/* #undef HAVE_BUILTIN_STPCPY */
/* #undef HAVE_GCC_ATOMIC_BUILTINS */
/* #undef HAVE_VALGRIND */

/* IPV6 */
/* #undef HAVE_NETINET_IN6_H */
#define HAVE_STRUCT_SOCKADDR_IN6 1
#define HAVE_STRUCT_IN6_ADDR 1
#define HAVE_IPV6 1

/* #undef ss_family */
/* #undef HAVE_SOCKADDR_IN_SIN_LEN */
/* #undef HAVE_SOCKADDR_IN6_SIN6_LEN */

/*
 * Platform specific CMake files
 */
#define MACHINE_TYPE "x86_64"
/* #undef HAVE_LINUX_LARGE_PAGES */
/* #undef HAVE_SOLARIS_LARGE_PAGES */
/* #undef HAVE_SOLARIS_ATOMIC */
/* #undef HAVE_SOLARIS_STYLE_GETHOST */
#define SYSTEM_TYPE "Win64"
/* Windows stuff, mostly functions, that have Posix analogs but named differently */
/* #undef IPPROTO_IPV6 */
/* #undef IPV6_V6ONLY */
/* This should mean case insensitive file system */
#define FN_NO_CASE_SENSE 1

/*
 * From main CMakeLists.txt
 */
#define MAX_INDEXES 64U
/* #undef WITH_INNODB_MEMCACHED */
/* #undef ENABLE_MEMCACHED_SASL */
/* #undef ENABLE_MEMCACHED_SASL_PWDB */
#define ENABLED_PROFILING 1
/* #undef HAVE_ASAN */
#define ENABLED_LOCAL_INFILE 1
#define OPTIMIZER_TRACE 1
#define DEFAULT_MYSQL_HOME "C:/Program Files/MySQL/MySQL Server 5.7"
#define SHAREDIR "share"
#define DEFAULT_BASEDIR "C:/Program Files/MySQL/MySQL Server 5.7"
#define MYSQL_DATADIR "C:/Program Files/MySQL/MySQL Server 5.7/data"
#define DEFAULT_CHARSET_HOME "C:/Program Files/MySQL/MySQL Server 5.7"
#define PLUGINDIR "C:/Program Files/MySQL/MySQL Server 5.7/lib/plugin"
/* #undef DEFAULT_SYSCONFDIR */
#define DEFAULT_TMPDIR ""
#define INSTALL_SBINDIR "/bin"
#define INSTALL_BINDIR "/bin"
#define INSTALL_MYSQLSHAREDIR "/share"
#define INSTALL_SHAREDIR "/share"
#define INSTALL_PLUGINDIR "/lib/plugin"
#define INSTALL_INCLUDEDIR "/include"
#define INSTALL_SCRIPTDIR "/scripts"
#define INSTALL_MYSQLDATADIR "/data"
/* #undef INSTALL_PLUGINTESTDIR */
#define INSTALL_INFODIR "/docs"
#define INSTALL_MYSQLTESTDIR "/mysql-test"
#define INSTALL_DOCREADMEDIR "/."
#define INSTALL_DOCDIR "/docs"
#define INSTALL_MANDIR "/man"
#define INSTALL_SUPPORTFILESDIR "/support-files"
#define INSTALL_LIBDIR "/lib"

/*
 * Readline
 */
/* #undef HAVE_MBSTATE_T */
/* #undef HAVE_LANGINFO_CODESET */
/* #undef HAVE_WCSDUP */
/* #undef HAVE_WCHAR_T */
/* #undef HAVE_WINT_T */
/* #undef HAVE_CURSES_H */
/* #undef HAVE_NCURSES_H */
/* #undef USE_LIBEDIT_INTERFACE */
/* #undef HAVE_HIST_ENTRY */

/*
 * Libedit
 */
/* #undef HAVE_DECL_TGOTO */

/*
 * DTrace
 */
/* #undef HAVE_DTRACE */

/*
 * Character sets
 */
#define MYSQL_DEFAULT_CHARSET_NAME "latin1"
#define MYSQL_DEFAULT_COLLATION_NAME "latin1_swedish_ci"
#define HAVE_CHARSET_armscii8 1
#define HAVE_CHARSET_ascii 1
#define HAVE_CHARSET_big5 1
#define HAVE_CHARSET_cp1250 1
#define HAVE_CHARSET_cp1251 1
#define HAVE_CHARSET_cp1256 1
#define HAVE_CHARSET_cp1257 1
#define HAVE_CHARSET_cp850 1
#define HAVE_CHARSET_cp852 1 
#define HAVE_CHARSET_cp866 1
#define HAVE_CHARSET_cp932 1
#define HAVE_CHARSET_dec8 1
#define HAVE_CHARSET_eucjpms 1
#define HAVE_CHARSET_euckr 1
#define HAVE_CHARSET_gb2312 1
#define HAVE_CHARSET_gbk 1
#define HAVE_CHARSET_gb18030 1
#define HAVE_CHARSET_geostd8 1
#define HAVE_CHARSET_greek 1
#define HAVE_CHARSET_hebrew 1
#define HAVE_CHARSET_hp8 1
#define HAVE_CHARSET_keybcs2 1
#define HAVE_CHARSET_koi8r 1
#define HAVE_CHARSET_koi8u 1
#define HAVE_CHARSET_latin1 1
#define HAVE_CHARSET_latin2 1
#define HAVE_CHARSET_latin5 1
#define HAVE_CHARSET_latin7 1
#define HAVE_CHARSET_macce 1
#define HAVE_CHARSET_macroman 1
#define HAVE_CHARSET_sjis 1
#define HAVE_CHARSET_swe7 1
#define HAVE_CHARSET_tis620 1
#define HAVE_CHARSET_ucs2 1
#define HAVE_CHARSET_ujis 1
#define HAVE_CHARSET_utf8mb4 1
/* #undef HAVE_CHARSET_utf8mb3 */
#define HAVE_CHARSET_utf8 1
#define HAVE_CHARSET_utf16 1
#define HAVE_CHARSET_utf32 1
#define HAVE_UCA_COLLATIONS 1

/*
 * Feature set
 */
#define WITH_PARTITION_STORAGE_ENGINE 1

/*
 * Performance schema
 */
#define WITH_PERFSCHEMA_STORAGE_ENGINE 1
/* #undef DISABLE_PSI_THREAD */
/* #undef DISABLE_PSI_MUTEX */
/* #undef DISABLE_PSI_RWLOCK */
/* #undef DISABLE_PSI_COND */
/* #undef DISABLE_PSI_FILE */
/* #undef DISABLE_PSI_TABLE */
/* #undef DISABLE_PSI_SOCKET */
/* #undef DISABLE_PSI_STAGE */
/* #undef DISABLE_PSI_STATEMENT */
/* #undef DISABLE_PSI_SP */
/* #undef DISABLE_PSI_PS */
/* #undef DISABLE_PSI_IDLE */
/* #undef DISABLE_PSI_STATEMENT_DIGEST */
/* #undef DISABLE_PSI_METADATA */
/* #undef DISABLE_PSI_MEMORY */
/* #undef DISABLE_PSI_TRANSACTION */

/*
 * syscall
*/
/* #undef HAVE_SYS_THREAD_SELFID */
/* #undef HAVE_SYS_GETTID */
/* #undef HAVE_PTHREAD_GETTHREADID_NP */
/* #undef HAVE_INTEGER_PTHREAD_SELF */

/* Platform-specific C++ compiler behaviors we rely upon */

/*
  This macro defines whether the compiler in use needs a 'typename' keyword
  to access the types defined inside a class template, such types are called
  dependent types. Some compilers require it, some others forbid it, and some
  others may work with or without it. For example, GCC requires the 'typename'
  keyword whenever needing to access a type inside a template, but msvc
  forbids it.
 */
#define HAVE_IMPLICIT_DEPENDENT_NAME_TYPING 1


/*
 * MySQL version
 */
#define DOT_FRM_VERSION 6
#define MYSQL_VERSION_MAJOR 5
#define MYSQL_VERSION_MINOR 7
#define MYSQL_VERSION_PATCH 10
#define MYSQL_VERSION_EXTRA ""
#define PACKAGE "mysql"
#define PACKAGE_BUGREPORT ""
#define PACKAGE_NAME "MySQL Server"
#define PACKAGE_STRING "MySQL Server 5.7.10"
#define PACKAGE_TARNAME "mysql"
#define PACKAGE_VERSION "5.7.10"
#define VERSION "5.7.10"
#define PROTOCOL_VERSION 10

/*
 * CPU info
 */
#define CPU_LEVEL1_DCACHE_LINESIZE 64

/*
 * NDB
 */
/* #undef WITH_NDBCLUSTER_STORAGE_ENGINE */
/* #undef HAVE_PTHREAD_SETSCHEDPARAM */

/*
 * Other
 */
/* #undef EXTRA_DEBUG */
/* #undef HAVE_CHOWN */

/*
 * Hardcoded values needed by libevent/NDB/memcached
 */
#define HAVE_FCNTL_H 1
#define HAVE_GETADDRINFO 1
#define HAVE_INTTYPES_H 1
#define HAVE_SELECT 1
#define HAVE_SIGNAL_H 1
#define HAVE_STDARG_H 1
#define HAVE_STDINT_H 1
#define HAVE_STDLIB_H 1
#define HAVE_STRDUP 1
#define HAVE_STRTOK_R 1
#define HAVE_STRTOLL 1
#define HAVE_SYS_STAT_H 1
#define HAVE_SYS_TYPES_H 1
#define SIZEOF_CHAR 1

/* For --secure-file-priv */
#define DEFAULT_SECURE_FILE_PRIV_DIR ""
#define DEFAULT_SECURE_FILE_PRIV_EMBEDDED_DIR "NULL"
/* #undef HAVE_LIBNUMA */

#else //-----------------------------32λ-----------------------------------------------------------------

/*
 * From configure.cmake, in order of appearance 
 */
/* #undef HAVE_LLVM_LIBCPP */
#define _LARGEFILE_SOURCE 1

/* Libraries */
/* #undef HAVE_LIBM */
/* #undef HAVE_LIBNSL */
/* #undef HAVE_LIBCRYPT */
/* #undef HAVE_LIBSOCKET */
/* #undef HAVE_LIBDL */
/* #undef HAVE_LIBRT */
/* #undef HAVE_LIBWRAP */

/* Header files */
/* #undef HAVE_ALLOCA_H */
/* #undef HAVE_ARPA_INET_H */
/* #undef HAVE_DIRENT_H */
/* #undef HAVE_DLFCN_H */
/* #undef HAVE_EXECINFO_H */
/* #undef HAVE_FPU_CONTROL_H */
/* #undef HAVE_GRP_H */
/* #undef HAVE_IEEEFP_H */
/* #undef HAVE_LANGINFO_H */
#define HAVE_MALLOC_H 1
/* #undef HAVE_NETINET_IN_H */
/* #undef HAVE_POLL_H */
/* #undef HAVE_PWD_H */
/* #undef HAVE_STRINGS_H */
/* #undef HAVE_SYS_CDEFS_H */
/* #undef HAVE_SYS_IOCTL_H */
/* #undef HAVE_SYS_MMAN_H */
/* #undef HAVE_SYS_RESOURCE_H */
/* #undef HAVE_SYS_SELECT_H */
/* #undef HAVE_SYS_SOCKET_H */
/* #undef HAVE_TERM_H */
/* #undef HAVE_TERMIOS_H */
/* #undef HAVE_TERMIO_H */
/* #undef HAVE_UNISTD_H */
/* #undef HAVE_SYS_WAIT_H */
/* #undef HAVE_SYS_PARAM_H */
/* #undef HAVE_FNMATCH_H */
/* #undef HAVE_SYS_UN_H */
/* #undef HAVE_VIS_H */
/* #undef HAVE_SASL_SASL_H */

/* Libevent */
/* #undef HAVE_DEVPOLL */
/* #undef HAVE_SYS_DEVPOLL_H */
/* #undef HAVE_SYS_EPOLL_H */
/* #undef HAVE_TAILQFOREACH */

/* Functions */
#define HAVE_ALIGNED_MALLOC 1
/* #undef HAVE_BACKTRACE */
/* #undef HAVE_PRINTSTACK */
/* #undef HAVE_INDEX */
/* #undef HAVE_CLOCK_GETTIME */
/* #undef HAVE_CUSERID */
/* #undef HAVE_DIRECTIO */
/* #undef HAVE_FTRUNCATE */
#define HAVE_COMPRESS 1
/* #undef HAVE_CRYPT */
/* #undef HAVE_DLOPEN */
/* #undef HAVE_FCHMOD */
/* #undef HAVE_FCNTL */
/* #undef HAVE_FDATASYNC */
/* #undef HAVE_DECL_FDATASYNC */
/* #undef HAVE_FEDISABLEEXCEPT */
/* #undef HAVE_FSEEKO */
/* #undef HAVE_FSYNC */
/* #undef HAVE_GETHOSTBYADDR_R */
/* #undef HAVE_GETHRTIME */
/* #undef HAVE_GETNAMEINFO */
/* #undef HAVE_GETPASS */
/* #undef HAVE_GETPASSPHRASE */
/* #undef HAVE_GETPWNAM */
/* #undef HAVE_GETPWUID */
/* #undef HAVE_GETRLIMIT */
/* #undef HAVE_GETRUSAGE */
/* #undef HAVE_INITGROUPS */
/* #undef HAVE_ISSETUGID */
/* #undef HAVE_GETUID */
/* #undef HAVE_GETEUID */
/* #undef HAVE_GETGID */
/* #undef HAVE_GETEGID */
/* #undef HAVE_LSTAT */
/* #undef HAVE_MADVISE */
/* #undef HAVE_MALLOC_INFO */
/* #undef HAVE_MEMRCHR */
/* #undef HAVE_MLOCK */
/* #undef HAVE_MLOCKALL */
/* #undef HAVE_MMAP64 */
/* #undef HAVE_POLL */
/* #undef HAVE_POSIX_FALLOCATE */
/* #undef HAVE_POSIX_MEMALIGN */
/* #undef HAVE_PREAD */
/* #undef HAVE_PTHREAD_CONDATTR_SETCLOCK */
/* #undef HAVE_PTHREAD_SIGMASK */
/* #undef HAVE_READDIR_R */
/* #undef HAVE_READLINK */
/* #undef HAVE_REALPATH */
/* #undef HAVE_SETFD */
/* #undef HAVE_SIGACTION */
/* #undef HAVE_SLEEP */
/* #undef HAVE_STPCPY */
/* #undef HAVE_STPNCPY */
/* #undef HAVE_STRLCPY */
#define HAVE_STRNLEN 1
/* #undef HAVE_STRLCAT */
/* #undef HAVE_STRSIGNAL */
/* #undef HAVE_FGETLN */
/* #undef HAVE_STRSEP */
#define HAVE_TELL 1
/* #undef HAVE_VASPRINTF */
/* #undef HAVE_MEMALIGN */
/* #undef HAVE_NL_LANGINFO */
/* #undef HAVE_HTONLL */
/* #undef DNS_USE_CPU_CLOCK_FOR_ID */
/* #undef HAVE_EPOLL */
/* #undef HAVE_EVENT_PORTS */
/* #undef HAVE_INET_NTOP */
/* #undef HAVE_WORKING_KQUEUE */
/* #undef HAVE_TIMERADD */
/* #undef HAVE_TIMERCLEAR */
/* #undef HAVE_TIMERCMP */
/* #undef HAVE_TIMERISSET */

/* WL2373 */
/* #undef HAVE_SYS_TIME_H */
/* #undef HAVE_SYS_TIMES_H */
/* #undef HAVE_TIMES */
/* #undef HAVE_GETTIMEOFDAY */

/* Symbols */
/* #undef HAVE_LRAND48 */
/* #undef GWINSZ_IN_SYS_IOCTL */
/* #undef FIONREAD_IN_SYS_IOCTL */
/* #undef FIONREAD_IN_SYS_FILIO */
/* #undef HAVE_SIGEV_THREAD_ID */
/* #undef HAVE_SIGEV_PORT */
#define HAVE_LOG2 1

#define HAVE_ISINF 1

/* #undef HAVE_KQUEUE_TIMERS */
/* #undef HAVE_POSIX_TIMERS */

/* Endianess */
/* #undef WORDS_BIGENDIAN */

/* Type sizes */
#define SIZEOF_VOIDP     4
#define SIZEOF_CHARP     4
#define SIZEOF_LONG      4
#define SIZEOF_SHORT     2
#define SIZEOF_INT       4
#define SIZEOF_LONG_LONG 8
#define SIZEOF_OFF_T     4
#define SIZEOF_TIME_T    8
/* #undef HAVE_UINT */
/* #undef HAVE_ULONG */
/* #undef HAVE_U_INT32_T */
/* #undef HAVE_STRUCT_TIMESPEC */

/* Code tests*/
#define STACK_DIRECTION -1
/* #undef TIME_WITH_SYS_TIME */
#define NO_FCNTL_NONBLOCK 1
/* #undef HAVE_PAUSE_INSTRUCTION */
/* #undef HAVE_FAKE_PAUSE_INSTRUCTION */
/* #undef HAVE_HMT_PRIORITY_INSTRUCTION */
/* #undef HAVE_ABI_CXA_DEMANGLE */
/* #undef HAVE_BSS_START */
/* #undef HAVE_BUILTIN_UNREACHABLE */
/* #undef HAVE_BUILTIN_EXPECT */
/* #undef HAVE_BUILTIN_STPCPY */
/* #undef HAVE_GCC_ATOMIC_BUILTINS */
/* #undef HAVE_VALGRIND */

/* IPV6 */
/* #undef HAVE_NETINET_IN6_H */
#define HAVE_STRUCT_SOCKADDR_IN6 1
#define HAVE_STRUCT_IN6_ADDR 1
#define HAVE_IPV6 1

/* #undef ss_family */
/* #undef HAVE_SOCKADDR_IN_SIN_LEN */
/* #undef HAVE_SOCKADDR_IN6_SIN6_LEN */

/*
 * Platform specific CMake files
 */
#define MACHINE_TYPE "AMD64"
/* #undef HAVE_LINUX_LARGE_PAGES */
/* #undef HAVE_SOLARIS_LARGE_PAGES */
/* #undef HAVE_SOLARIS_ATOMIC */
/* #undef HAVE_SOLARIS_STYLE_GETHOST */
#define SYSTEM_TYPE "Win32"
/* Windows stuff, mostly functions, that have Posix analogs but named differently */
/* #undef IPPROTO_IPV6 */
/* #undef IPV6_V6ONLY */
/* This should mean case insensitive file system */
#define FN_NO_CASE_SENSE 1

/*
 * From main CMakeLists.txt
 */
#define MAX_INDEXES 64U
/* #undef WITH_INNODB_MEMCACHED */
/* #undef ENABLE_MEMCACHED_SASL */
/* #undef ENABLE_MEMCACHED_SASL_PWDB */
#define ENABLED_PROFILING 1
/* #undef HAVE_ASAN */
#define ENABLED_LOCAL_INFILE 1
#define OPTIMIZER_TRACE 1
#define DEFAULT_MYSQL_HOME "C:/Program Files/MySQL/MySQL Server 5.7"
#define SHAREDIR "share"
#define DEFAULT_BASEDIR "C:/Program Files/MySQL/MySQL Server 5.7"
#define MYSQL_DATADIR "C:/Program Files/MySQL/MySQL Server 5.7/data"
#define DEFAULT_CHARSET_HOME "C:/Program Files/MySQL/MySQL Server 5.7"
#define PLUGINDIR "C:/Program Files/MySQL/MySQL Server 5.7/lib/plugin"
/* #undef DEFAULT_SYSCONFDIR */
#define DEFAULT_TMPDIR ""
#define INSTALL_SBINDIR "/bin"
#define INSTALL_BINDIR "/bin"
#define INSTALL_MYSQLSHAREDIR "/share"
#define INSTALL_SHAREDIR "/share"
#define INSTALL_PLUGINDIR "/lib/plugin"
#define INSTALL_INCLUDEDIR "/include"
#define INSTALL_SCRIPTDIR "/scripts"
#define INSTALL_MYSQLDATADIR "/data"
/* #undef INSTALL_PLUGINTESTDIR */
#define INSTALL_INFODIR "/docs"
#define INSTALL_MYSQLTESTDIR "/mysql-test"
#define INSTALL_DOCREADMEDIR "/."
#define INSTALL_DOCDIR "/docs"
#define INSTALL_MANDIR "/man"
#define INSTALL_SUPPORTFILESDIR "/support-files"
#define INSTALL_LIBDIR "/lib"

/*
 * Readline
 */
/* #undef HAVE_MBSTATE_T */
/* #undef HAVE_LANGINFO_CODESET */
/* #undef HAVE_WCSDUP */
/* #undef HAVE_WCHAR_T */
/* #undef HAVE_WINT_T */
/* #undef HAVE_CURSES_H */
/* #undef HAVE_NCURSES_H */
/* #undef USE_LIBEDIT_INTERFACE */
/* #undef HAVE_HIST_ENTRY */

/*
 * Libedit
 */
/* #undef HAVE_DECL_TGOTO */

/*
 * DTrace
 */
/* #undef HAVE_DTRACE */

/*
 * Character sets
 */
#define MYSQL_DEFAULT_CHARSET_NAME "latin1"
#define MYSQL_DEFAULT_COLLATION_NAME "latin1_swedish_ci"
#define HAVE_CHARSET_armscii8 1
#define HAVE_CHARSET_ascii 1
#define HAVE_CHARSET_big5 1
#define HAVE_CHARSET_cp1250 1
#define HAVE_CHARSET_cp1251 1
#define HAVE_CHARSET_cp1256 1
#define HAVE_CHARSET_cp1257 1
#define HAVE_CHARSET_cp850 1
#define HAVE_CHARSET_cp852 1 
#define HAVE_CHARSET_cp866 1
#define HAVE_CHARSET_cp932 1
#define HAVE_CHARSET_dec8 1
#define HAVE_CHARSET_eucjpms 1
#define HAVE_CHARSET_euckr 1
#define HAVE_CHARSET_gb2312 1
#define HAVE_CHARSET_gbk 1
#define HAVE_CHARSET_gb18030 1
#define HAVE_CHARSET_geostd8 1
#define HAVE_CHARSET_greek 1
#define HAVE_CHARSET_hebrew 1
#define HAVE_CHARSET_hp8 1
#define HAVE_CHARSET_keybcs2 1
#define HAVE_CHARSET_koi8r 1
#define HAVE_CHARSET_koi8u 1
#define HAVE_CHARSET_latin1 1
#define HAVE_CHARSET_latin2 1
#define HAVE_CHARSET_latin5 1
#define HAVE_CHARSET_latin7 1
#define HAVE_CHARSET_macce 1
#define HAVE_CHARSET_macroman 1
#define HAVE_CHARSET_sjis 1
#define HAVE_CHARSET_swe7 1
#define HAVE_CHARSET_tis620 1
#define HAVE_CHARSET_ucs2 1
#define HAVE_CHARSET_ujis 1
#define HAVE_CHARSET_utf8mb4 1
/* #undef HAVE_CHARSET_utf8mb3 */
#define HAVE_CHARSET_utf8 1
#define HAVE_CHARSET_utf16 1
#define HAVE_CHARSET_utf32 1
#define HAVE_UCA_COLLATIONS 1

/*
 * Feature set
 */
#define WITH_PARTITION_STORAGE_ENGINE 1

/*
 * Performance schema
 */
#define WITH_PERFSCHEMA_STORAGE_ENGINE 1
/* #undef DISABLE_PSI_THREAD */
/* #undef DISABLE_PSI_MUTEX */
/* #undef DISABLE_PSI_RWLOCK */
/* #undef DISABLE_PSI_COND */
/* #undef DISABLE_PSI_FILE */
/* #undef DISABLE_PSI_TABLE */
/* #undef DISABLE_PSI_SOCKET */
/* #undef DISABLE_PSI_STAGE */
/* #undef DISABLE_PSI_STATEMENT */
/* #undef DISABLE_PSI_SP */
/* #undef DISABLE_PSI_PS */
/* #undef DISABLE_PSI_IDLE */
/* #undef DISABLE_PSI_STATEMENT_DIGEST */
/* #undef DISABLE_PSI_METADATA */
/* #undef DISABLE_PSI_MEMORY */
/* #undef DISABLE_PSI_TRANSACTION */

/*
 * syscall
*/
/* #undef HAVE_SYS_THREAD_SELFID */
/* #undef HAVE_SYS_GETTID */
/* #undef HAVE_PTHREAD_GETTHREADID_NP */
/* #undef HAVE_INTEGER_PTHREAD_SELF */

/* Platform-specific C++ compiler behaviors we rely upon */

/*
  This macro defines whether the compiler in use needs a 'typename' keyword
  to access the types defined inside a class template, such types are called
  dependent types. Some compilers require it, some others forbid it, and some
  others may work with or without it. For example, GCC requires the 'typename'
  keyword whenever needing to access a type inside a template, but msvc
  forbids it.
 */
#define HAVE_IMPLICIT_DEPENDENT_NAME_TYPING 1


/*
 * MySQL version
 */
#define DOT_FRM_VERSION 6
#define MYSQL_VERSION_MAJOR 5
#define MYSQL_VERSION_MINOR 7
#define MYSQL_VERSION_PATCH 10
#define MYSQL_VERSION_EXTRA ""
#define PACKAGE "mysql"
#define PACKAGE_BUGREPORT ""
#define PACKAGE_NAME "MySQL Server"
#define PACKAGE_STRING "MySQL Server 5.7.10"
#define PACKAGE_TARNAME "mysql"
#define PACKAGE_VERSION "5.7.10"
#define VERSION "5.7.10"
#define PROTOCOL_VERSION 10

/*
 * CPU info
 */
#define CPU_LEVEL1_DCACHE_LINESIZE 64

/*
 * NDB
 */
/* #undef WITH_NDBCLUSTER_STORAGE_ENGINE */
/* #undef HAVE_PTHREAD_SETSCHEDPARAM */

/*
 * Other
 */
/* #undef EXTRA_DEBUG */
/* #undef HAVE_CHOWN */

/*
 * Hardcoded values needed by libevent/NDB/memcached
 */
#define HAVE_FCNTL_H 1
#define HAVE_GETADDRINFO 1
#define HAVE_INTTYPES_H 1
#define HAVE_SELECT 1
#define HAVE_SIGNAL_H 1
#define HAVE_STDARG_H 1
#define HAVE_STDINT_H 1
#define HAVE_STDLIB_H 1
#define HAVE_STRDUP 1
#define HAVE_STRTOK_R 1
#define HAVE_STRTOLL 1
#define HAVE_SYS_STAT_H 1
#define HAVE_SYS_TYPES_H 1
#define SIZEOF_CHAR 1

/* For --secure-file-priv */
#define DEFAULT_SECURE_FILE_PRIV_DIR ""
#define DEFAULT_SECURE_FILE_PRIV_EMBEDDED_DIR "NULL"
/* #undef HAVE_LIBNUMA */

#endif

#endif
