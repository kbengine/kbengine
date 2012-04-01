/* pyconfig.h.  Generated from pyconfig.h.in by configure.  */
/* pyconfig.h.in.  Generated from configure.in by autoheader.  */


#ifndef Py_PYCONFIG_H
#define Py_PYCONFIG_H

#if defined( __WIN32__ ) || defined( WIN32 ) || defined( _WIN32 )
/* pyconfig.h.  NOT Generated automatically by configure.

This is a manually maintained version used for the Watcom,
Borland and Microsoft Visual C++ compilers.  It is a
standard part of the Python distribution.

WINDOWS DEFINES:
The code specific to Windows should be wrapped around one of
the following #defines

MS_WIN64 - Code specific to the MS Win64 API
MS_WIN32 - Code specific to the MS Win32 (and Win64) API (obsolete, this covers all supported APIs)
MS_WINDOWS - Code specific to Windows, but all versions.
MS_WINCE - Code specific to Windows CE
Py_ENABLE_SHARED - Code if the Python core is built as a DLL.

Also note that neither "_M_IX86" or "_MSC_VER" should be used for
any purpose other than "Windows Intel x86 specific" and "Microsoft
compiler specific".  Therefore, these should be very rare.


NOTE: The following symbols are deprecated:
NT, USE_DL_EXPORT, USE_DL_IMPORT, DL_EXPORT, DL_IMPORT
MS_CORE_DLL.

WIN32 is still required for the locale module.

*/

#ifdef _WIN32_WCE
#define MS_WINCE
#endif

/* Deprecated USE_DL_EXPORT macro - please use Py_BUILD_CORE */
#ifdef USE_DL_EXPORT
#	define Py_BUILD_CORE
#endif /* USE_DL_EXPORT */

/* Visual Studio 2005 introduces deprecation warnings for
   "insecure" and POSIX functions. The insecure functions should
   be replaced by *_s versions (according to Microsoft); the
   POSIX functions by _* versions (which, according to Microsoft,
   would be ISO C conforming). Neither renaming is feasible, so
   we just silence the warnings. */

#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE 1
#endif
#ifndef _CRT_NONSTDC_NO_DEPRECATE
#define _CRT_NONSTDC_NO_DEPRECATE 1
#endif

/* Windows CE does not have these */
#ifndef MS_WINCE
#define HAVE_IO_H
#define HAVE_SYS_UTIME_H
#define HAVE_TEMPNAM
#define HAVE_TMPFILE
#define HAVE_TMPNAM
#define HAVE_CLOCK
#define HAVE_STRERROR
#endif

#ifdef HAVE_IO_H
#include <io.h>
#endif

#define HAVE_HYPOT
#define HAVE_STRFTIME
#define DONT_HAVE_SIG_ALARM
#define DONT_HAVE_SIG_PAUSE
#define LONG_BIT	32
#define WORD_BIT 32
#define PREFIX ""
#define EXEC_PREFIX ""

#define MS_WIN32 /* only support win32 and greater. */
#define MS_WINDOWS
#ifndef PYTHONPATH
#	define PYTHONPATH L".\\DLLs;.\\lib"
#endif
#define NT_THREADS
#define WITH_THREAD
#ifndef NETSCAPE_PI
#define USE_SOCKET
#endif

/* CE6 doesn't have strdup() but _strdup(). Assume the same for earlier versions. */
#if defined(MS_WINCE)
#  include <stdlib.h>
#  define strdup _strdup
#endif

#ifdef MS_WINCE
/* Windows CE does not support environment variables */
#define getenv(v) (NULL)
#define environ (NULL)
#endif

/* Compiler specific defines */

/* ------------------------------------------------------------------------*/
/* Microsoft C defines _MSC_VER */
#ifdef _MSC_VER

/* We want COMPILER to expand to a string containing _MSC_VER's *value*.
 * This is horridly tricky, because the stringization operator only works
 * on macro arguments, and doesn't evaluate macros passed *as* arguments.
 * Attempts simpler than the following appear doomed to produce "_MSC_VER"
 * literally in the string.
 */
#define _Py_PASTE_VERSION(SUFFIX) \
	("[MSC v." _Py_STRINGIZE(_MSC_VER) " " SUFFIX "]")
/* e.g., this produces, after compile-time string catenation,
 * 	("[MSC v.1200 32 bit (Intel)]")
 *
 * _Py_STRINGIZE(_MSC_VER) expands to
 * _Py_STRINGIZE1((_MSC_VER)) expands to
 * _Py_STRINGIZE2(_MSC_VER) but as this call is the result of token-pasting
 *      it's scanned again for macros and so further expands to (under MSVC 6)
 * _Py_STRINGIZE2(1200) which then expands to
 * "1200"
 */
#define _Py_STRINGIZE(X) _Py_STRINGIZE1((X))
#define _Py_STRINGIZE1(X) _Py_STRINGIZE2 ## X
#define _Py_STRINGIZE2(X) #X

/* MSVC defines _WINxx to differentiate the windows platform types

   Note that for compatibility reasons _WIN32 is defined on Win32
   *and* on Win64. For the same reasons, in Python, MS_WIN32 is
   defined on Win32 *and* Win64. Win32 only code must therefore be
   guarded as follows:
   	#if defined(MS_WIN32) && !defined(MS_WIN64)
   Some modules are disabled on Itanium processors, therefore we
   have MS_WINI64 set for those targets, otherwise MS_WINX64
*/
#ifdef _WIN64
#define MS_WIN64
#endif

/* set the COMPILER */
#ifdef MS_WIN64
#if defined(_M_IA64)
#define COMPILER _Py_PASTE_VERSION("64 bit (Itanium)")
#define MS_WINI64
#elif defined(_M_X64) || defined(_M_AMD64)
#define COMPILER _Py_PASTE_VERSION("64 bit (AMD64)")
#define MS_WINX64
#else
#define COMPILER _Py_PASTE_VERSION("64 bit (Unknown)")
#endif
#endif /* MS_WIN64 */

/* set the version macros for the windows headers */
#ifdef MS_WINX64
/* 64 bit only runs on XP or greater */
#define Py_WINVER 0x0501 /* _WIN32_WINNT_WINXP */
#define Py_NTDDI NTDDI_WINXP
#else
/* Python 2.6+ requires Windows 2000 or greater */
#define Py_WINVER 0x0500 /* _WIN32_WINNT_WIN2K */
#define Py_NTDDI NTDDI_WIN2KSP4
#endif

/* We only set these values when building Python - we don't want to force
   these values on extensions, as that will affect the prototypes and
   structures exposed in the Windows headers. Even when building Python, we
   allow a single source file to override this - they may need access to
   structures etc so it can optionally use new Windows features if it
   determines at runtime they are available.
*/
#if defined(Py_BUILD_CORE) || defined(Py_BUILD_CORE_MODULE)
#ifndef NTDDI_VERSION
#define NTDDI_VERSION Py_NTDDI
#endif
#ifndef WINVER
#define WINVER Py_WINVER
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT Py_WINVER
#endif
#endif

/* _W64 is not defined for VC6 or eVC4 */
#ifndef _W64
#define _W64
#endif

/* Define like size_t, omitting the "unsigned" */
#ifdef MS_WIN64
typedef __int64 ssize_t;
#else
typedef _W64 int ssize_t;
#endif
#define HAVE_SSIZE_T 1

#if defined(MS_WIN32) && !defined(MS_WIN64)
#ifdef _M_IX86
#define COMPILER _Py_PASTE_VERSION("32 bit (Intel)")
#else
#define COMPILER _Py_PASTE_VERSION("32 bit (Unknown)")
#endif
#endif /* MS_WIN32 && !MS_WIN64 */

typedef int pid_t;

#include <float.h>
#define Py_IS_NAN _isnan
#define Py_IS_INFINITY(X) (!_finite(X) && !_isnan(X))
#define Py_IS_FINITE(X) _finite(X)
#define copysign _copysign
#define hypot _hypot

#endif /* _MSC_VER */

/* define some ANSI types that are not defined in earlier Win headers */
#if defined(_MSC_VER) && _MSC_VER >= 1200
/* This file only exists in VC 6.0 or higher */
#include <basetsd.h>
#endif

/* ------------------------------------------------------------------------*/
/* The Borland compiler defines __BORLANDC__ */
/* XXX These defines are likely incomplete, but should be easy to fix. */
#ifdef __BORLANDC__
#define COMPILER "[Borland]"

#ifdef _WIN32
/* tested with BCC 5.5 (__BORLANDC__ >= 0x0550)
 */

typedef int pid_t;
/* BCC55 seems to understand __declspec(dllimport), it is used in its
   own header files (winnt.h, ...) - so we can do nothing and get the default*/

#undef HAVE_SYS_UTIME_H
#define HAVE_UTIME_H
#define HAVE_DIRENT_H

/* rename a few functions for the Borland compiler */
#include <io.h>
#define _chsize chsize
#define _setmode setmode

#else /* !_WIN32 */
#error "Only Win32 and later are supported"
#endif /* !_WIN32 */

#endif /* BORLANDC */

/* ------------------------------------------------------------------------*/
/* egcs/gnu-win32 defines __GNUC__ and _WIN32 */
#if defined(__GNUC__) && defined(_WIN32)
/* XXX These defines are likely incomplete, but should be easy to fix.
   They should be complete enough to build extension modules. */
/* Suggested by Rene Liebscher <R.Liebscher@gmx.de> to avoid a GCC 2.91.*
   bug that requires structure imports.  More recent versions of the
   compiler don't exhibit this bug.
*/
#if (__GNUC__==2) && (__GNUC_MINOR__<=91)
#warning "Please use an up-to-date version of gcc! (>2.91 recommended)"
#endif

#define COMPILER "[gcc]"
#define hypot _hypot
#define PY_LONG_LONG long long
#define PY_LLONG_MIN LLONG_MIN
#define PY_LLONG_MAX LLONG_MAX
#define PY_ULLONG_MAX ULLONG_MAX
#endif /* GNUC */

/* ------------------------------------------------------------------------*/
/* lcc-win32 defines __LCC__ */
#if defined(__LCC__)
/* XXX These defines are likely incomplete, but should be easy to fix.
   They should be complete enough to build extension modules. */

#define COMPILER "[lcc-win32]"
typedef int pid_t;
/* __declspec() is supported here too - do nothing to get the defaults */

#endif /* LCC */

/* ------------------------------------------------------------------------*/
/* End of compilers - finish up */

#ifndef NO_STDIO_H
#	include <stdio.h>
#endif

/* 64 bit ints are usually spelt __int64 unless compiler has overridden */
#define HAVE_LONG_LONG 1
#ifndef PY_LONG_LONG
#	define PY_LONG_LONG __int64
#	define PY_LLONG_MAX _I64_MAX
#	define PY_LLONG_MIN _I64_MIN
#	define PY_ULLONG_MAX _UI64_MAX
#endif

/* For Windows the Python core is in a DLL by default.  Test
Py_NO_ENABLE_SHARED to find out.  Also support MS_NO_COREDLL for b/w compat */
#if !defined(MS_NO_COREDLL) && !defined(Py_NO_ENABLE_SHARED)
#	define Py_ENABLE_SHARED 1 /* standard symbol for shared library */
#	define MS_COREDLL	/* deprecated old symbol */
#endif /* !MS_NO_COREDLL && ... */

/*  All windows compilers that use this header support __declspec */
#define HAVE_DECLSPEC_DLL

/* For an MSVC DLL, we can nominate the .lib files used by extensions */
#ifdef MS_COREDLL
#	ifndef Py_BUILD_CORE /* not building the core - must be an ext */
#		if defined(_MSC_VER)
			/* So MSVC users need not specify the .lib file in
			their Makefile (other compilers are generally
			taken care of by distutils.) */
#			if defined(_DEBUG)
#				pragma comment(lib,"python32_d.lib")
#			elif defined(Py_LIMITED_API)
#				pragma comment(lib,"python3.lib")
#			else
#				pragma comment(lib,"python32.lib")
#			endif /* _DEBUG */
#		endif /* _MSC_VER */
#	endif /* Py_BUILD_CORE */
#endif /* MS_COREDLL */

#if defined(MS_WIN64)
/* maintain "win32" sys.platform for backward compatibility of Python code,
   the Win64 API should be close enough to the Win32 API to make this
   preferable */
#	define PLATFORM "win32"
#	define SIZEOF_VOID_P 8
#	define SIZEOF_TIME_T 8
#	define SIZEOF_OFF_T 4
#	define SIZEOF_FPOS_T 8
#	define SIZEOF_HKEY 8
#	define SIZEOF_SIZE_T 8
/* configure.in defines HAVE_LARGEFILE_SUPPORT iff HAVE_LONG_LONG,
   sizeof(off_t) > sizeof(long), and sizeof(PY_LONG_LONG) >= sizeof(off_t).
   On Win64 the second condition is not true, but if fpos_t replaces off_t
   then this is true. The uses of HAVE_LARGEFILE_SUPPORT imply that Win64
   should define this. */
#	define HAVE_LARGEFILE_SUPPORT
#elif defined(MS_WIN32)
#	define PLATFORM "win32"
#	define HAVE_LARGEFILE_SUPPORT
#	define SIZEOF_VOID_P 4
#	define SIZEOF_OFF_T 4
#	define SIZEOF_FPOS_T 8
#	define SIZEOF_HKEY 4
#	define SIZEOF_SIZE_T 4
	/* MS VS2005 changes time_t to an 64-bit type on all platforms */
#	if defined(_MSC_VER) && _MSC_VER >= 1400
#	define SIZEOF_TIME_T 8
#	else
#	define SIZEOF_TIME_T 4
#	endif
#endif

#ifdef _DEBUG
#	define Py_DEBUG
#endif


#ifdef MS_WIN32

#define SIZEOF_SHORT 2
#define SIZEOF_INT 4
#define SIZEOF_LONG 4
#define SIZEOF_LONG_LONG 8
#define SIZEOF_DOUBLE 8
#define SIZEOF_FLOAT 4

/* VC 7.1 has them and VC 6.0 does not.  VC 6.0 has a version number of 1200.
   Microsoft eMbedded Visual C++ 4.0 has a version number of 1201 and doesn't
   define these.
   If some compiler does not provide them, modify the #if appropriately. */
#if defined(_MSC_VER)
#if _MSC_VER > 1300
#define HAVE_UINTPTR_T 1
#define HAVE_INTPTR_T 1
#else
/* VC6, VS 2002 and eVC4 don't support the C99 LL suffix for 64-bit integer literals */
#define Py_LL(x) x##I64
#endif  /* _MSC_VER > 1200  */
#endif  /* _MSC_VER */

#endif

/* define signed and unsigned exact-width 32-bit and 64-bit types, used in the
   implementation of Python long integers. */
#ifndef PY_UINT32_T
#if SIZEOF_INT == 4
#define HAVE_UINT32_T 1
#define PY_UINT32_T unsigned int
#elif SIZEOF_LONG == 4
#define HAVE_UINT32_T 1
#define PY_UINT32_T unsigned long
#endif
#endif

#ifndef PY_UINT64_T
#if SIZEOF_LONG_LONG == 8
#define HAVE_UINT64_T 1
#define PY_UINT64_T unsigned PY_LONG_LONG
#endif
#endif

#ifndef PY_INT32_T
#if SIZEOF_INT == 4
#define HAVE_INT32_T 1
#define PY_INT32_T int
#elif SIZEOF_LONG == 4
#define HAVE_INT32_T 1
#define PY_INT32_T long
#endif
#endif

#ifndef PY_INT64_T
#if SIZEOF_LONG_LONG == 8
#define HAVE_INT64_T 1
#define PY_INT64_T PY_LONG_LONG
#endif
#endif

/* Fairly standard from here! */

/* Define to 1 if you have the `copysign' function. */
#define HAVE_COPYSIGN 1

/* Define to 1 if you have the `isinf' macro. */
#define HAVE_DECL_ISINF 1

/* Define to 1 if you have the `isnan' function. */
#define HAVE_DECL_ISNAN 1

/* Define if on AIX 3.
   System headers sometimes define this.
   We just want to avoid a redefinition error message.  */
#ifndef _ALL_SOURCE
/* #undef _ALL_SOURCE */
#endif

/* Define to empty if the keyword does not work.  */
/* #define const  */

/* Define to 1 if you have the <conio.h> header file. */
#ifndef MS_WINCE
#define HAVE_CONIO_H 1
#endif

/* Define to 1 if you have the <direct.h> header file. */
#ifndef MS_WINCE
#define HAVE_DIRECT_H 1
#endif

/* Define if you have dirent.h.  */
/* #define DIRENT 1 */

/* Define to the type of elements in the array set by `getgroups'.
   Usually this is either `int' or `gid_t'.  */
/* #undef GETGROUPS_T */

/* Define to `int' if <sys/types.h> doesn't define.  */
/* #undef gid_t */

/* Define if your struct tm has tm_zone.  */
/* #undef HAVE_TM_ZONE */

/* Define if you don't have tm_zone but do have the external array
   tzname.  */
#define HAVE_TZNAME

/* Define to `int' if <sys/types.h> doesn't define.  */
/* #undef mode_t */

/* Define if you don't have dirent.h, but have ndir.h.  */
/* #undef NDIR */

/* Define to `long' if <sys/types.h> doesn't define.  */
/* #undef off_t */

/* Define to `int' if <sys/types.h> doesn't define.  */
/* #undef pid_t */

/* Define if the system does not provide POSIX.1 features except
   with this defined.  */
/* #undef _POSIX_1_SOURCE */

/* Define if you need to in order for stat and other things to work.  */
/* #undef _POSIX_SOURCE */

/* Define as the return type of signal handlers (int or void).  */
#define RETSIGTYPE void

/* Define to `unsigned' if <sys/types.h> doesn't define.  */
/* #undef size_t */

/* Define if you have the ANSI C header files.  */
#define STDC_HEADERS 1

/* Define if you don't have dirent.h, but have sys/dir.h.  */
/* #undef SYSDIR */

/* Define if you don't have dirent.h, but have sys/ndir.h.  */
/* #undef SYSNDIR */

/* Define if you can safely include both <sys/time.h> and <time.h>.  */
/* #undef TIME_WITH_SYS_TIME */

/* Define if your <sys/time.h> declares struct tm.  */
/* #define TM_IN_SYS_TIME 1 */

/* Define to `int' if <sys/types.h> doesn't define.  */
/* #undef uid_t */

/* Define if the closedir function returns void instead of int.  */
/* #undef VOID_CLOSEDIR */

/* Define if getpgrp() must be called as getpgrp(0)
   and (consequently) setpgrp() as setpgrp(0, 0). */
/* #undef GETPGRP_HAVE_ARGS */

/* Define this if your time.h defines altzone */
/* #define HAVE_ALTZONE */

/* Define if you have the putenv function.  */
#ifndef MS_WINCE
#define HAVE_PUTENV
#endif

/* Define if your compiler supports function prototypes */
#define HAVE_PROTOTYPES

/* Define if  you can safely include both <sys/select.h> and <sys/time.h>
   (which you can't on SCO ODT 3.0). */
/* #undef SYS_SELECT_WITH_SYS_TIME */

/* Define if you want documentation strings in extension modules */
#define WITH_DOC_STRINGS 1

/* Define if you want to compile in rudimentary thread support */
/* #undef WITH_THREAD */

/* Define if you want to use the GNU readline library */
/* #define WITH_READLINE 1 */

/* Define as the size of the unicode type. */
/* This is enough for unicodeobject.h to do the "right thing" on Windows. */
#define Py_UNICODE_SIZE 2

/* Use Python's own small-block memory-allocator. */
#define WITH_PYMALLOC 1

/* Define if you have clock.  */
/* #define HAVE_CLOCK */

/* Define when any dynamic module loading is enabled */
#define HAVE_DYNAMIC_LOADING

/* Define if you have ftime.  */
#ifndef MS_WINCE
#define HAVE_FTIME
#endif

/* Define if you have getpeername.  */
#define HAVE_GETPEERNAME

/* Define if you have getpgrp.  */
/* #undef HAVE_GETPGRP */

/* Define if you have getpid.  */
#ifndef MS_WINCE
#define HAVE_GETPID
#endif

/* Define if you have gettimeofday.  */
/* #undef HAVE_GETTIMEOFDAY */

/* Define if you have getwd.  */
/* #undef HAVE_GETWD */

/* Define if you have lstat.  */
/* #undef HAVE_LSTAT */

/* Define if you have the mktime function.  */
#define HAVE_MKTIME

/* Define if you have nice.  */
/* #undef HAVE_NICE */

/* Define if you have readlink.  */
/* #undef HAVE_READLINK */

/* Define if you have select.  */
/* #undef HAVE_SELECT */

/* Define if you have setpgid.  */
/* #undef HAVE_SETPGID */

/* Define if you have setpgrp.  */
/* #undef HAVE_SETPGRP */

/* Define if you have setsid.  */
/* #undef HAVE_SETSID */

/* Define if you have setvbuf.  */
#define HAVE_SETVBUF

/* Define if you have siginterrupt.  */
/* #undef HAVE_SIGINTERRUPT */

/* Define if you have symlink.  */
/* #undef HAVE_SYMLINK */

/* Define if you have tcgetpgrp.  */
/* #undef HAVE_TCGETPGRP */

/* Define if you have tcsetpgrp.  */
/* #undef HAVE_TCSETPGRP */

/* Define if you have times.  */
/* #undef HAVE_TIMES */

/* Define if you have uname.  */
/* #undef HAVE_UNAME */

/* Define if you have waitpid.  */
/* #undef HAVE_WAITPID */

/* Define to 1 if you have the `wcsftime' function. */
#if defined(_MSC_VER) && _MSC_VER >= 1310
#define HAVE_WCSFTIME 1
#endif

/* Define to 1 if you have the `wcscoll' function. */
#ifndef MS_WINCE
#define HAVE_WCSCOLL 1
#endif

/* Define to 1 if you have the `wcsxfrm' function. */
#ifndef MS_WINCE
#define HAVE_WCSXFRM 1
#endif

/* Define if you have the <dlfcn.h> header file.  */
/* #undef HAVE_DLFCN_H */

/* Define to 1 if you have the <errno.h> header file. */
#ifndef MS_WINCE
#define HAVE_ERRNO_H 1
#endif

/* Define if you have the <fcntl.h> header file.  */
#ifndef MS_WINCE
#define HAVE_FCNTL_H 1
#endif

/* Define to 1 if you have the <process.h> header file. */
#ifndef MS_WINCE
#define HAVE_PROCESS_H 1
#endif

/* Define to 1 if you have the <signal.h> header file. */
#ifndef MS_WINCE
#define HAVE_SIGNAL_H 1
#endif

/* Define if you have the <stdarg.h> prototypes.  */
#define HAVE_STDARG_PROTOTYPES

/* Define if you have the <stddef.h> header file.  */
#define HAVE_STDDEF_H 1

/* Define if you have the <sys/audioio.h> header file.  */
/* #undef HAVE_SYS_AUDIOIO_H */

/* Define if you have the <sys/param.h> header file.  */
/* #define HAVE_SYS_PARAM_H 1 */

/* Define if you have the <sys/select.h> header file.  */
/* #define HAVE_SYS_SELECT_H 1 */

/* Define to 1 if you have the <sys/stat.h> header file.  */
#ifndef MS_WINCE
#define HAVE_SYS_STAT_H 1
#endif

/* Define if you have the <sys/time.h> header file.  */
/* #define HAVE_SYS_TIME_H 1 */

/* Define if you have the <sys/times.h> header file.  */
/* #define HAVE_SYS_TIMES_H 1 */

/* Define to 1 if you have the <sys/types.h> header file.  */
#ifndef MS_WINCE
#define HAVE_SYS_TYPES_H 1
#endif

/* Define if you have the <sys/un.h> header file.  */
/* #define HAVE_SYS_UN_H 1 */

/* Define if you have the <sys/utime.h> header file.  */
/* #define HAVE_SYS_UTIME_H 1 */

/* Define if you have the <sys/utsname.h> header file.  */
/* #define HAVE_SYS_UTSNAME_H 1 */

/* Define if you have the <thread.h> header file.  */
/* #undef HAVE_THREAD_H */

/* Define if you have the <unistd.h> header file.  */
/* #define HAVE_UNISTD_H 1 */

/* Define if you have the <utime.h> header file.  */
/* #define HAVE_UTIME_H 1 */

/* Define if the compiler provides a wchar.h header file. */
#define HAVE_WCHAR_H 1

/* The size of `wchar_t', as computed by sizeof. */
#define SIZEOF_WCHAR_T 2

/* Define if you have the dl library (-ldl).  */
/* #undef HAVE_LIBDL */

/* Define if you have the mpc library (-lmpc).  */
/* #undef HAVE_LIBMPC */

/* Define if you have the nsl library (-lnsl).  */
#define HAVE_LIBNSL 1

/* Define if you have the seq library (-lseq).  */
/* #undef HAVE_LIBSEQ */

/* Define if you have the socket library (-lsocket).  */
#define HAVE_LIBSOCKET 1

/* Define if you have the sun library (-lsun).  */
/* #undef HAVE_LIBSUN */

/* Define if you have the termcap library (-ltermcap).  */
/* #undef HAVE_LIBTERMCAP */

/* Define if you have the termlib library (-ltermlib).  */
/* #undef HAVE_LIBTERMLIB */

/* Define if you have the thread library (-lthread).  */
/* #undef HAVE_LIBTHREAD */

/* WinSock does not use a bitmask in select, and uses
   socket handles greater than FD_SETSIZE */
#define Py_SOCKET_FD_CAN_BE_GE_FD_SETSIZE

/* Define if C doubles are 64-bit IEEE 754 binary format, stored with the
   least significant byte first */
#define DOUBLE_IS_LITTLE_ENDIAN_IEEE754 1

#else
/* Define if building universal (internal helper macro) */
/* #undef AC_APPLE_UNIVERSAL_BUILD */

/* Define for AIX if your compiler is a genuine IBM xlC/xlC_r and you want
   support for AIX C++ shared extension modules. */
/* #undef AIX_GENUINE_CPLUSPLUS */

/* Define if you have the Mach cthreads package */
/* #undef C_THREADS */

/* Define if C doubles are 64-bit IEEE 754 binary format, stored in ARM
   mixed-endian order (byte order 45670123) */
/* #undef DOUBLE_IS_ARM_MIXED_ENDIAN_IEEE754 */

/* Define if C doubles are 64-bit IEEE 754 binary format, stored with the most
   significant byte first */
/* #undef DOUBLE_IS_BIG_ENDIAN_IEEE754 */

/* Define if C doubles are 64-bit IEEE 754 binary format, stored with the
   least significant byte first */
#define DOUBLE_IS_LITTLE_ENDIAN_IEEE754 1

/* Define if --enable-ipv6 is specified */
#define ENABLE_IPV6 1

/* Define if flock needs to be linked with bsd library. */
/* #undef FLOCK_NEEDS_LIBBSD */

/* Define if getpgrp() must be called as getpgrp(0). */
/* #undef GETPGRP_HAVE_ARG */

/* Define if gettimeofday() does not have second (timezone) argument This is
   the case on Motorola V4 (R40V4.2) */
/* #undef GETTIMEOFDAY_NO_TZ */

/* Define to 1 if you have the `accept4' function. */
#define HAVE_ACCEPT4 1

/* Define to 1 if you have the `acosh' function. */
#define HAVE_ACOSH 1

/* struct addrinfo (netdb.h) */
#define HAVE_ADDRINFO 1

/* Define to 1 if you have the `alarm' function. */
#define HAVE_ALARM 1

/* Define this if your time.h defines altzone. */
/* #undef HAVE_ALTZONE */

/* Define to 1 if you have the `asinh' function. */
#define HAVE_ASINH 1

/* Define to 1 if you have the <asm/types.h> header file. */
#define HAVE_ASM_TYPES_H 1

/* Define to 1 if you have the `atanh' function. */
#define HAVE_ATANH 1

/* Define if GCC supports __attribute__((format(PyArg_ParseTuple, 2, 3))) */
/* #undef HAVE_ATTRIBUTE_FORMAT_PARSETUPLE */

/* Define to 1 if you have the `bind_textdomain_codeset' function. */
#define HAVE_BIND_TEXTDOMAIN_CODESET 1

/* Define to 1 if you have the <bluetooth/bluetooth.h> header file. */
/* #undef HAVE_BLUETOOTH_BLUETOOTH_H */

/* Define to 1 if you have the <bluetooth.h> header file. */
/* #undef HAVE_BLUETOOTH_H */

/* Define if mbstowcs(NULL, "text", 0) does not return the number of wide
   chars that would be converted. */
/* #undef HAVE_BROKEN_MBSTOWCS */

/* Define if nice() returns success/failure instead of the new priority. */
/* #undef HAVE_BROKEN_NICE */

/* Define if the system reports an invalid PIPE_BUF value. */
/* #undef HAVE_BROKEN_PIPE_BUF */

/* Define if poll() sets errno on invalid file descriptors. */
/* #undef HAVE_BROKEN_POLL */

/* Define if the Posix semaphores do not work on your system */
/* #undef HAVE_BROKEN_POSIX_SEMAPHORES */

/* Define if pthread_sigmask() does not work on your system. */
/* #undef HAVE_BROKEN_PTHREAD_SIGMASK */

/* define to 1 if your sem_getvalue is broken. */
/* #undef HAVE_BROKEN_SEM_GETVALUE */

/* Define this if you have the type _Bool. */
#define HAVE_C99_BOOL 1

/* Define to 1 if you have the 'chflags' function. */
/* #undef HAVE_CHFLAGS */

/* Define to 1 if you have the `chown' function. */
#define HAVE_CHOWN 1

/* Define if you have the 'chroot' function. */
#define HAVE_CHROOT 1

/* Define to 1 if you have the `clock' function. */
#define HAVE_CLOCK 1

/* Define if the C compiler supports computed gotos. */
#define HAVE_COMPUTED_GOTOS 1

/* Define to 1 if you have the `confstr' function. */
#define HAVE_CONFSTR 1

/* Define to 1 if you have the <conio.h> header file. */
/* #undef HAVE_CONIO_H */

/* Define to 1 if you have the `copysign' function. */
#define HAVE_COPYSIGN 1

/* Define to 1 if you have the `ctermid' function. */
#define HAVE_CTERMID 1

/* Define if you have the 'ctermid_r' function. */
/* #undef HAVE_CTERMID_R */

/* Define to 1 if you have the <curses.h> header file. */
/* #undef HAVE_CURSES_H */

/* Define if you have the 'is_term_resized' function. */
/* #undef HAVE_CURSES_IS_TERM_RESIZED */

/* Define if you have the 'resizeterm' function. */
/* #undef HAVE_CURSES_RESIZETERM */

/* Define if you have the 'resize_term' function. */
/* #undef HAVE_CURSES_RESIZE_TERM */

/* Define to 1 if you have the declaration of `isfinite', and to 0 if you
   don't. */
#define HAVE_DECL_ISFINITE 1

/* Define to 1 if you have the declaration of `isinf', and to 0 if you don't.
   */
#define HAVE_DECL_ISINF 1

/* Define to 1 if you have the declaration of `isnan', and to 0 if you don't.
   */
#define HAVE_DECL_ISNAN 1

/* Define to 1 if you have the declaration of `tzname', and to 0 if you don't.
   */
/* #undef HAVE_DECL_TZNAME */

/* Define to 1 if you have the device macros. */
#define HAVE_DEVICE_MACROS 1

/* Define if we have /dev/ptc. */
/* #undef HAVE_DEV_PTC */

/* Define if we have /dev/ptmx. */
#define HAVE_DEV_PTMX 1

/* Define to 1 if you have the <direct.h> header file. */
/* #undef HAVE_DIRECT_H */

/* Define to 1 if you have the <dirent.h> header file, and it defines `DIR'.
   */
#define HAVE_DIRENT_H 1

/* Define to 1 if you have the <dlfcn.h> header file. */
#define HAVE_DLFCN_H 1

/* Define to 1 if you have the `dlopen' function. */
#define HAVE_DLOPEN 1

/* Define to 1 if you have the `dup2' function. */
#define HAVE_DUP2 1

/* Defined when any dynamic module loading is enabled. */
#define HAVE_DYNAMIC_LOADING 1

/* Define if you have the 'epoll' functions. */
#define HAVE_EPOLL 1

/* Define to 1 if you have the `erf' function. */
#define HAVE_ERF 1

/* Define to 1 if you have the `erfc' function. */
#define HAVE_ERFC 1

/* Define to 1 if you have the <errno.h> header file. */
#define HAVE_ERRNO_H 1

/* Define to 1 if you have the `execv' function. */
#define HAVE_EXECV 1

/* Define to 1 if you have the `expm1' function. */
#define HAVE_EXPM1 1

/* Define if you have the 'fchdir' function. */
#define HAVE_FCHDIR 1

/* Define to 1 if you have the `fchmod' function. */
#define HAVE_FCHMOD 1

/* Define to 1 if you have the `fchown' function. */
#define HAVE_FCHOWN 1

/* Define to 1 if you have the <fcntl.h> header file. */
#define HAVE_FCNTL_H 1

/* Define if you have the 'fdatasync' function. */
#define HAVE_FDATASYNC 1

/* Define to 1 if you have the `finite' function. */
#define HAVE_FINITE 1

/* Define to 1 if you have the `flock' function. */
#define HAVE_FLOCK 1

/* Define to 1 if you have the `fork' function. */
#define HAVE_FORK 1

/* Define to 1 if you have the `forkpty' function. */
#define HAVE_FORKPTY 1

/* Define to 1 if you have the `fpathconf' function. */
#define HAVE_FPATHCONF 1

/* Define to 1 if you have the `fseek64' function. */
/* #undef HAVE_FSEEK64 */

/* Define to 1 if you have the `fseeko' function. */
#define HAVE_FSEEKO 1

/* Define to 1 if you have the `fstatvfs' function. */
#define HAVE_FSTATVFS 1

/* Define if you have the 'fsync' function. */
#define HAVE_FSYNC 1

/* Define to 1 if you have the `ftell64' function. */
/* #undef HAVE_FTELL64 */

/* Define to 1 if you have the `ftello' function. */
#define HAVE_FTELLO 1

/* Define to 1 if you have the `ftime' function. */
#define HAVE_FTIME 1

/* Define to 1 if you have the `ftruncate' function. */
#define HAVE_FTRUNCATE 1

/* Define to 1 if you have the `gai_strerror' function. */
#define HAVE_GAI_STRERROR 1

/* Define to 1 if you have the `gamma' function. */
#define HAVE_GAMMA 1

/* Define if we can use gcc inline assembler to get and set x87 control word
   */
#define HAVE_GCC_ASM_FOR_X87 1

/* Define if you have the getaddrinfo function. */
#define HAVE_GETADDRINFO 1

/* Define to 1 if you have the `getcwd' function. */
#define HAVE_GETCWD 1

/* Define this if you have flockfile(), getc_unlocked(), and funlockfile() */
#define HAVE_GETC_UNLOCKED 1

/* Define to 1 if you have the `getgroups' function. */
#define HAVE_GETGROUPS 1

/* Define to 1 if you have the `gethostbyname' function. */
/* #undef HAVE_GETHOSTBYNAME */

/* Define this if you have some version of gethostbyname_r() */
#define HAVE_GETHOSTBYNAME_R 1

/* Define this if you have the 3-arg version of gethostbyname_r(). */
/* #undef HAVE_GETHOSTBYNAME_R_3_ARG */

/* Define this if you have the 5-arg version of gethostbyname_r(). */
/* #undef HAVE_GETHOSTBYNAME_R_5_ARG */

/* Define this if you have the 6-arg version of gethostbyname_r(). */
#define HAVE_GETHOSTBYNAME_R_6_ARG 1

/* Define to 1 if you have the `getitimer' function. */
#define HAVE_GETITIMER 1

/* Define to 1 if you have the `getloadavg' function. */
#define HAVE_GETLOADAVG 1

/* Define to 1 if you have the `getlogin' function. */
#define HAVE_GETLOGIN 1

/* Define to 1 if you have the `getnameinfo' function. */
#define HAVE_GETNAMEINFO 1

/* Define if you have the 'getpagesize' function. */
#define HAVE_GETPAGESIZE 1

/* Define to 1 if you have the `getpeername' function. */
#define HAVE_GETPEERNAME 1

/* Define to 1 if you have the `getpgid' function. */
#define HAVE_GETPGID 1

/* Define to 1 if you have the `getpgrp' function. */
#define HAVE_GETPGRP 1

/* Define to 1 if you have the `getpid' function. */
#define HAVE_GETPID 1

/* Define to 1 if you have the `getpriority' function. */
#define HAVE_GETPRIORITY 1

/* Define to 1 if you have the `getpwent' function. */
#define HAVE_GETPWENT 1

/* Define to 1 if you have the `getresgid' function. */
#define HAVE_GETRESGID 1

/* Define to 1 if you have the `getresuid' function. */
#define HAVE_GETRESUID 1

/* Define to 1 if you have the `getsid' function. */
#define HAVE_GETSID 1

/* Define to 1 if you have the `getspent' function. */
#define HAVE_GETSPENT 1

/* Define to 1 if you have the `getspnam' function. */
#define HAVE_GETSPNAM 1

/* Define to 1 if you have the `gettimeofday' function. */
#define HAVE_GETTIMEOFDAY 1

/* Define to 1 if you have the `getwd' function. */
#define HAVE_GETWD 1

/* Define to 1 if you have the <grp.h> header file. */
#define HAVE_GRP_H 1

/* Define if you have the 'hstrerror' function. */
#define HAVE_HSTRERROR 1

/* Define to 1 if you have the `hypot' function. */
#define HAVE_HYPOT 1

/* Define to 1 if you have the <ieeefp.h> header file. */
/* #undef HAVE_IEEEFP_H */

/* Define if you have the 'inet_aton' function. */
#define HAVE_INET_ATON 1

/* Define if you have the 'inet_pton' function. */
#define HAVE_INET_PTON 1

/* Define to 1 if you have the `initgroups' function. */
#define HAVE_INITGROUPS 1

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if you have the <io.h> header file. */
/* #undef HAVE_IO_H */

/* Define to 1 if you have the `kill' function. */
#define HAVE_KILL 1

/* Define to 1 if you have the `killpg' function. */
#define HAVE_KILLPG 1

/* Define if you have the 'kqueue' functions. */
/* #undef HAVE_KQUEUE */

/* Define to 1 if you have the <langinfo.h> header file. */
#define HAVE_LANGINFO_H 1

/* Defined to enable large file support when an off_t is bigger than a long
   and long long is available and at least as big as an off_t. You may need to
   add some flags for configuration and compilation to enable this mode. (For
   Solaris and Linux, the necessary defines are already defined.) */
#define HAVE_LARGEFILE_SUPPORT 1

/* Define to 1 if you have the 'lchflags' function. */
/* #undef HAVE_LCHFLAGS */

/* Define to 1 if you have the `lchmod' function. */
/* #undef HAVE_LCHMOD */

/* Define to 1 if you have the `lchown' function. */
#define HAVE_LCHOWN 1

/* Define to 1 if you have the `lgamma' function. */
#define HAVE_LGAMMA 1

/* Define to 1 if you have the `dl' library (-ldl). */
#define HAVE_LIBDL 1

/* Define to 1 if you have the `dld' library (-ldld). */
/* #undef HAVE_LIBDLD */

/* Define to 1 if you have the `ieee' library (-lieee). */
/* #undef HAVE_LIBIEEE */

/* Define to 1 if you have the <libintl.h> header file. */
#define HAVE_LIBINTL_H 1

/* Define if you have the readline library (-lreadline). */
/* #undef HAVE_LIBREADLINE */

/* Define to 1 if you have the `resolv' library (-lresolv). */
/* #undef HAVE_LIBRESOLV */

/* Define to 1 if you have the <libutil.h> header file. */
/* #undef HAVE_LIBUTIL_H */

/* Define if you have the 'link' function. */
#define HAVE_LINK 1

/* Define to 1 if you have the <linux/netlink.h> header file. */
#define HAVE_LINUX_NETLINK_H 1

/* Define to 1 if you have the <linux/tipc.h> header file. */
#define HAVE_LINUX_TIPC_H 1

/* Define to 1 if you have the `log1p' function. */
#define HAVE_LOG1P 1

/* Define this if you have the type long double. */
#define HAVE_LONG_DOUBLE 1

/* Define this if you have the type long long. */
#define HAVE_LONG_LONG 1

/* Define to 1 if you have the `lstat' function. */
#define HAVE_LSTAT 1

/* Define this if you have the makedev macro. */
#define HAVE_MAKEDEV 1

/* Define to 1 if you have the `mbrtowc' function. */
#define HAVE_MBRTOWC 1

/* Define to 1 if you have the `memmove' function. */
#define HAVE_MEMMOVE 1

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if you have the `mkfifo' function. */
#define HAVE_MKFIFO 1

/* Define to 1 if you have the `mknod' function. */
#define HAVE_MKNOD 1

/* Define to 1 if you have the `mktime' function. */
#define HAVE_MKTIME 1

/* Define to 1 if you have the `mremap' function. */
#define HAVE_MREMAP 1

/* Define to 1 if you have the <ncurses.h> header file. */
/* #undef HAVE_NCURSES_H */

/* Define to 1 if you have the <ndir.h> header file, and it defines `DIR'. */
/* #undef HAVE_NDIR_H */

/* Define to 1 if you have the <netpacket/packet.h> header file. */
#define HAVE_NETPACKET_PACKET_H 1

/* Define to 1 if you have the `nice' function. */
#define HAVE_NICE 1

/* Define to 1 if you have the `openpty' function. */
#define HAVE_OPENPTY 1

/* Define if compiling using MacOS X 10.5 SDK or later. */
/* #undef HAVE_OSX105_SDK */

/* Define to 1 if you have the `pathconf' function. */
#define HAVE_PATHCONF 1

/* Define to 1 if you have the `pause' function. */
#define HAVE_PAUSE 1

/* Define if the OS supports pipe2() */
#define HAVE_PIPE2 1

/* Define to 1 if you have the `plock' function. */
/* #undef HAVE_PLOCK */

/* Define to 1 if you have the `poll' function. */
#define HAVE_POLL 1

/* Define to 1 if you have the <poll.h> header file. */
#define HAVE_POLL_H 1

/* Define to 1 if you have the <process.h> header file. */
/* #undef HAVE_PROCESS_H */

/* Define if your compiler supports function prototype */
#define HAVE_PROTOTYPES 1

/* Defined for Solaris 2.6 bug in pthread header. */
/* #undef HAVE_PTHREAD_DESTRUCTOR */

/* Define to 1 if you have the <pthread.h> header file. */
#define HAVE_PTHREAD_H 1

/* Define to 1 if you have the `pthread_init' function. */
/* #undef HAVE_PTHREAD_INIT */

/* Define to 1 if you have the `pthread_sigmask' function. */
#define HAVE_PTHREAD_SIGMASK 1

/* Define to 1 if you have the <pty.h> header file. */
#define HAVE_PTY_H 1

/* Define to 1 if you have the `putenv' function. */
#define HAVE_PUTENV 1

/* Define to 1 if you have the `readlink' function. */
#define HAVE_READLINK 1

/* Define to 1 if you have the `realpath' function. */
#define HAVE_REALPATH 1

/* Define if you have readline 2.1 */
/* #undef HAVE_RL_CALLBACK */

/* Define if you can turn off readline's signal handling. */
/* #undef HAVE_RL_CATCH_SIGNAL */

/* Define if you have readline 2.2 */
/* #undef HAVE_RL_COMPLETION_APPEND_CHARACTER */

/* Define if you have readline 4.0 */
/* #undef HAVE_RL_COMPLETION_DISPLAY_MATCHES_HOOK */

/* Define if you have readline 4.2 */
/* #undef HAVE_RL_COMPLETION_MATCHES */

/* Define if you have rl_completion_suppress_append */
/* #undef HAVE_RL_COMPLETION_SUPPRESS_APPEND */

/* Define if you have readline 4.0 */
/* #undef HAVE_RL_PRE_INPUT_HOOK */

/* Define to 1 if you have the `round' function. */
#define HAVE_ROUND 1

/* Define to 1 if you have the `select' function. */
#define HAVE_SELECT 1

/* Define to 1 if you have the `sem_getvalue' function. */
#define HAVE_SEM_GETVALUE 1

/* Define to 1 if you have the `sem_open' function. */
#define HAVE_SEM_OPEN 1

/* Define to 1 if you have the `sem_timedwait' function. */
#define HAVE_SEM_TIMEDWAIT 1

/* Define to 1 if you have the `sem_unlink' function. */
#define HAVE_SEM_UNLINK 1

/* Define to 1 if you have the `setegid' function. */
#define HAVE_SETEGID 1

/* Define to 1 if you have the `seteuid' function. */
#define HAVE_SETEUID 1

/* Define to 1 if you have the `setgid' function. */
#define HAVE_SETGID 1

/* Define if you have the 'setgroups' function. */
#define HAVE_SETGROUPS 1

/* Define to 1 if you have the `setitimer' function. */
#define HAVE_SETITIMER 1

/* Define to 1 if you have the `setlocale' function. */
#define HAVE_SETLOCALE 1

/* Define to 1 if you have the `setpgid' function. */
#define HAVE_SETPGID 1

/* Define to 1 if you have the `setpgrp' function. */
#define HAVE_SETPGRP 1

/* Define to 1 if you have the `setregid' function. */
#define HAVE_SETREGID 1

/* Define to 1 if you have the `setresgid' function. */
#define HAVE_SETRESGID 1

/* Define to 1 if you have the `setresuid' function. */
#define HAVE_SETRESUID 1

/* Define to 1 if you have the `setreuid' function. */
#define HAVE_SETREUID 1

/* Define to 1 if you have the `setsid' function. */
#define HAVE_SETSID 1

/* Define to 1 if you have the `setuid' function. */
#define HAVE_SETUID 1

/* Define to 1 if you have the `setvbuf' function. */
#define HAVE_SETVBUF 1

/* Define to 1 if you have the <shadow.h> header file. */
#define HAVE_SHADOW_H 1

/* Define to 1 if you have the `sigaction' function. */
#define HAVE_SIGACTION 1

/* Define to 1 if you have the `siginterrupt' function. */
#define HAVE_SIGINTERRUPT 1

/* Define to 1 if you have the <signal.h> header file. */
#define HAVE_SIGNAL_H 1

/* Define to 1 if you have the `sigrelse' function. */
#define HAVE_SIGRELSE 1

/* Define to 1 if you have the `snprintf' function. */
#define HAVE_SNPRINTF 1

/* Define if sockaddr has sa_len member */
/* #undef HAVE_SOCKADDR_SA_LEN */

/* struct sockaddr_storage (sys/socket.h) */
#define HAVE_SOCKADDR_STORAGE 1

/* Define if you have the 'socketpair' function. */
#define HAVE_SOCKETPAIR 1

/* Define to 1 if you have the <spawn.h> header file. */
#define HAVE_SPAWN_H 1

/* Define if your compiler provides ssize_t */
#define HAVE_SSIZE_T 1

/* Define to 1 if you have the `statvfs' function. */
#define HAVE_STATVFS 1

/* Define if you have struct stat.st_mtim.tv_nsec */
#define HAVE_STAT_TV_NSEC 1

/* Define if you have struct stat.st_mtimensec */
/* #undef HAVE_STAT_TV_NSEC2 */

/* Define if your compiler supports variable length function prototypes (e.g.
   void fprintf(FILE *, char *, ...);) *and* <stdarg.h> */
#define HAVE_STDARG_PROTOTYPES 1

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the `strdup' function. */
#define HAVE_STRDUP 1

/* Define to 1 if you have the `strftime' function. */
#define HAVE_STRFTIME 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the `strlcpy' function. */
/* #undef HAVE_STRLCPY */

/* Define to 1 if you have the <stropts.h> header file. */
#define HAVE_STROPTS_H 1

/* Define to 1 if `st_birthtime' is a member of `struct stat'. */
/* #undef HAVE_STRUCT_STAT_ST_BIRTHTIME */

/* Define to 1 if `st_blksize' is a member of `struct stat'. */
#define HAVE_STRUCT_STAT_ST_BLKSIZE 1

/* Define to 1 if `st_blocks' is a member of `struct stat'. */
#define HAVE_STRUCT_STAT_ST_BLOCKS 1

/* Define to 1 if `st_flags' is a member of `struct stat'. */
/* #undef HAVE_STRUCT_STAT_ST_FLAGS */

/* Define to 1 if `st_gen' is a member of `struct stat'. */
/* #undef HAVE_STRUCT_STAT_ST_GEN */

/* Define to 1 if `st_rdev' is a member of `struct stat'. */
#define HAVE_STRUCT_STAT_ST_RDEV 1

/* Define to 1 if `tm_zone' is a member of `struct tm'. */
#define HAVE_STRUCT_TM_TM_ZONE 1

/* Define to 1 if your `struct stat' has `st_blocks'. Deprecated, use
   `HAVE_STRUCT_STAT_ST_BLOCKS' instead. */
#define HAVE_ST_BLOCKS 1

/* Define if you have the 'symlink' function. */
#define HAVE_SYMLINK 1

/* Define to 1 if you have the `sysconf' function. */
#define HAVE_SYSCONF 1

/* Define to 1 if you have the <sysexits.h> header file. */
#define HAVE_SYSEXITS_H 1

/* Define to 1 if you have the <sys/audioio.h> header file. */
/* #undef HAVE_SYS_AUDIOIO_H */

/* Define to 1 if you have the <sys/bsdtty.h> header file. */
/* #undef HAVE_SYS_BSDTTY_H */

/* Define to 1 if you have the <sys/dir.h> header file, and it defines `DIR'.
   */
/* #undef HAVE_SYS_DIR_H */

/* Define to 1 if you have the <sys/epoll.h> header file. */
#define HAVE_SYS_EPOLL_H 1

/* Define to 1 if you have the <sys/event.h> header file. */
/* #undef HAVE_SYS_EVENT_H */

/* Define to 1 if you have the <sys/file.h> header file. */
#define HAVE_SYS_FILE_H 1

/* Define to 1 if you have the <sys/loadavg.h> header file. */
/* #undef HAVE_SYS_LOADAVG_H */

/* Define to 1 if you have the <sys/lock.h> header file. */
/* #undef HAVE_SYS_LOCK_H */

/* Define to 1 if you have the <sys/mkdev.h> header file. */
/* #undef HAVE_SYS_MKDEV_H */

/* Define to 1 if you have the <sys/modem.h> header file. */
/* #undef HAVE_SYS_MODEM_H */

/* Define to 1 if you have the <sys/ndir.h> header file, and it defines `DIR'.
   */
/* #undef HAVE_SYS_NDIR_H */

/* Define to 1 if you have the <sys/param.h> header file. */
#define HAVE_SYS_PARAM_H 1

/* Define to 1 if you have the <sys/poll.h> header file. */
#define HAVE_SYS_POLL_H 1

/* Define to 1 if you have the <sys/resource.h> header file. */
#define HAVE_SYS_RESOURCE_H 1

/* Define to 1 if you have the <sys/select.h> header file. */
#define HAVE_SYS_SELECT_H 1

/* Define to 1 if you have the <sys/socket.h> header file. */
#define HAVE_SYS_SOCKET_H 1

/* Define to 1 if you have the <sys/statvfs.h> header file. */
#define HAVE_SYS_STATVFS_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/termio.h> header file. */
/* #undef HAVE_SYS_TERMIO_H */

/* Define to 1 if you have the <sys/times.h> header file. */
#define HAVE_SYS_TIMES_H 1

/* Define to 1 if you have the <sys/time.h> header file. */
#define HAVE_SYS_TIME_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <sys/un.h> header file. */
#define HAVE_SYS_UN_H 1

/* Define to 1 if you have the <sys/utsname.h> header file. */
#define HAVE_SYS_UTSNAME_H 1

/* Define to 1 if you have the <sys/wait.h> header file. */
#define HAVE_SYS_WAIT_H 1

/* Define to 1 if you have the `tcgetpgrp' function. */
#define HAVE_TCGETPGRP 1

/* Define to 1 if you have the `tcsetpgrp' function. */
#define HAVE_TCSETPGRP 1

/* Define to 1 if you have the `tempnam' function. */
#define HAVE_TEMPNAM 1

/* Define to 1 if you have the <termios.h> header file. */
#define HAVE_TERMIOS_H 1

/* Define to 1 if you have the <term.h> header file. */
/* #undef HAVE_TERM_H */

/* Define to 1 if you have the `tgamma' function. */
#define HAVE_TGAMMA 1

/* Define to 1 if you have the <thread.h> header file. */
/* #undef HAVE_THREAD_H */

/* Define to 1 if you have the `timegm' function. */
#define HAVE_TIMEGM 1

/* Define to 1 if you have the `times' function. */
#define HAVE_TIMES 1

/* Define to 1 if you have the `tmpfile' function. */
#define HAVE_TMPFILE 1

/* Define to 1 if you have the `tmpnam' function. */
#define HAVE_TMPNAM 1

/* Define to 1 if you have the `tmpnam_r' function. */
#define HAVE_TMPNAM_R 1

/* Define to 1 if your `struct tm' has `tm_zone'. Deprecated, use
   `HAVE_STRUCT_TM_TM_ZONE' instead. */
#define HAVE_TM_ZONE 1

/* Define to 1 if you have the `truncate' function. */
#define HAVE_TRUNCATE 1

/* Define to 1 if you don't have `tm_zone' but do have the external array
   `tzname'. */
/* #undef HAVE_TZNAME */

/* Define this if you have tcl and TCL_UTF_MAX==6 */
/* #undef HAVE_UCS4_TCL */

/* Define to 1 if the system has the type `uintptr_t'. */
#define HAVE_UINTPTR_T 1

/* Define to 1 if you have the `uname' function. */
#define HAVE_UNAME 1

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* Define to 1 if you have the `unsetenv' function. */
#define HAVE_UNSETENV 1

/* Define if you have a useable wchar_t type defined in wchar.h; useable means
   wchar_t must be an unsigned type with at least 16 bits. (see
   Include/unicodeobject.h). */
/* #undef HAVE_USABLE_WCHAR_T */

/* Define to 1 if you have the <util.h> header file. */
/* #undef HAVE_UTIL_H */

/* Define to 1 if you have the `utimes' function. */
#define HAVE_UTIMES 1

/* Define to 1 if you have the <utime.h> header file. */
#define HAVE_UTIME_H 1

/* Define to 1 if you have the `wait3' function. */
#define HAVE_WAIT3 1

/* Define to 1 if you have the `wait4' function. */
#define HAVE_WAIT4 1

/* Define to 1 if you have the `waitpid' function. */
#define HAVE_WAITPID 1

/* Define if the compiler provides a wchar.h header file. */
#define HAVE_WCHAR_H 1

/* Define to 1 if you have the `wcscoll' function. */
#define HAVE_WCSCOLL 1

/* Define to 1 if you have the `wcsftime' function. */
#define HAVE_WCSFTIME 1

/* Define to 1 if you have the `wcsxfrm' function. */
#define HAVE_WCSXFRM 1

/* Define if tzset() actually switches the local timezone in a meaningful way.
   */
#define HAVE_WORKING_TZSET 1

/* Define if the zlib library has inflateCopy */
/* #undef HAVE_ZLIB_COPY */

/* Define to 1 if you have the `_getpty' function. */
/* #undef HAVE__GETPTY */

/* Define if you are using Mach cthreads directly under /include */
/* #undef HURD_C_THREADS */

/* Define if log1p(-0.) is 0. rather than -0. */
/* #undef LOG1P_DROPS_ZERO_SIGN */

/* Define if you are using Mach cthreads under mach / */
/* #undef MACH_C_THREADS */

/* Define to 1 if `major', `minor', and `makedev' are declared in <mkdev.h>.
   */
/* #undef MAJOR_IN_MKDEV */

/* Define to 1 if `major', `minor', and `makedev' are declared in
   <sysmacros.h>. */
/* #undef MAJOR_IN_SYSMACROS */

/* Define if mvwdelch in curses.h is an expression. */
/* #undef MVWDELCH_IS_EXPRESSION */

/* Define to the address where bug reports for this package should be sent. */
/* #undef PACKAGE_BUGREPORT */

/* Define to the full name of this package. */
/* #undef PACKAGE_NAME */

/* Define to the full name and version of this package. */
/* #undef PACKAGE_STRING */

/* Define to the one symbol short name of this package. */
/* #undef PACKAGE_TARNAME */

/* Define to the home page for this package. */
/* #undef PACKAGE_URL */

/* Define to the version of this package. */
/* #undef PACKAGE_VERSION */

/* Define if POSIX semaphores aren't enabled on your system */
/* #undef POSIX_SEMAPHORES_NOT_ENABLED */

/* Defined if PTHREAD_SCOPE_SYSTEM supported. */
#define PTHREAD_SYSTEM_SCHED_SUPPORTED 1

/* Define as the preferred size in bits of long digits */
/* #undef PYLONG_BITS_IN_DIGIT */

/* Define to printf format modifier for long long type */
#define PY_FORMAT_LONG_LONG "ll"

/* Define to printf format modifier for Py_ssize_t */
#define PY_FORMAT_SIZE_T "z"

/* Define as the integral type used for Unicode representation. */
#define PY_UNICODE_TYPE unsigned short

/* Define if you want to build an interpreter with many run-time checks. */
/* #undef Py_DEBUG */

/* Defined if Python is built as a shared library. */
/* #undef Py_ENABLE_SHARED */

/* Define as the size of the unicode type. */
#define Py_UNICODE_SIZE 2

/* assume C89 semantics that RETSIGTYPE is always void */
#define RETSIGTYPE void

/* Define if setpgrp() must be called as setpgrp(0, 0). */
/* #undef SETPGRP_HAVE_ARG */

/* Define this to be extension of shared libraries (including the dot!). */
#define SHLIB_EXT ""

/* Define if i>>j for signed int i does not extend the sign bit when i < 0 */
/* #undef SIGNED_RIGHT_SHIFT_ZERO_FILLS */

/* The size of `double', as computed by sizeof. */
#define SIZEOF_DOUBLE 8

/* The size of `float', as computed by sizeof. */
#define SIZEOF_FLOAT 4

/* The size of `fpos_t', as computed by sizeof. */
#define SIZEOF_FPOS_T 16

/* The size of `int', as computed by sizeof. */
#define SIZEOF_INT 4

/* The size of `long', as computed by sizeof. */
#define SIZEOF_LONG 4

/* The size of `long double', as computed by sizeof. */
#define SIZEOF_LONG_DOUBLE 12

/* The size of `long long', as computed by sizeof. */
#define SIZEOF_LONG_LONG 8

/* The size of `off_t', as computed by sizeof. */
#define SIZEOF_OFF_T 8

/* The size of `pid_t', as computed by sizeof. */
#define SIZEOF_PID_T 4

/* The size of `pthread_t', as computed by sizeof. */
#define SIZEOF_PTHREAD_T 4

/* The size of `short', as computed by sizeof. */
#define SIZEOF_SHORT 2

/* The size of `size_t', as computed by sizeof. */
#define SIZEOF_SIZE_T 4

/* The size of `time_t', as computed by sizeof. */
#define SIZEOF_TIME_T 4

/* The size of `uintptr_t', as computed by sizeof. */
#define SIZEOF_UINTPTR_T 4

/* The size of `void *', as computed by sizeof. */
#define SIZEOF_VOID_P 4

/* The size of `wchar_t', as computed by sizeof. */
#define SIZEOF_WCHAR_T 4

/* The size of `_Bool', as computed by sizeof. */
#define SIZEOF__BOOL 1

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Define if you can safely include both <sys/select.h> and <sys/time.h>
   (which you can't on SCO ODT 3.0). */
#define SYS_SELECT_WITH_SYS_TIME 1

/* Define if tanh(-0.) is -0., or if platform doesn't have signed zeros */
#define TANH_PRESERVES_ZERO_SIGN 1

/* Define to 1 if you can safely include both <sys/time.h> and <time.h>. */
#define TIME_WITH_SYS_TIME 1

/* Define to 1 if your <sys/time.h> declares `struct tm'. */
/* #undef TM_IN_SYS_TIME */

/* Define if you want to use computed gotos in ceval.c. */
/* #undef USE_COMPUTED_GOTOS */

/* Define to use the C99 inline keyword. */
#define USE_INLINE 1

/* Enable extensions on AIX 3, Interix.  */
#ifndef _ALL_SOURCE
# define _ALL_SOURCE 1
#endif
/* Enable GNU extensions on systems that have them.  */
#ifndef _GNU_SOURCE
# define _GNU_SOURCE 1
#endif
/* Enable threading extensions on Solaris.  */
#ifndef _POSIX_PTHREAD_SEMANTICS
# define _POSIX_PTHREAD_SEMANTICS 1
#endif
/* Enable extensions on HP NonStop.  */
#ifndef _TANDEM_SOURCE
# define _TANDEM_SOURCE 1
#endif
/* Enable general extensions on Solaris.  */
#ifndef __EXTENSIONS__
# define __EXTENSIONS__ 1
#endif


/* Define if a va_list is an array of some kind */
/* #undef VA_LIST_IS_ARRAY */

/* Define if you want SIGFPE handled (see Include/pyfpe.h). */
/* #undef WANT_SIGFPE_HANDLER */

/* Define if WINDOW in curses.h offers a field _flags. */
/* #undef WINDOW_HAS_FLAGS */

/* Define if you want documentation strings in extension modules */
#define WITH_DOC_STRINGS 1

/* Define if you want to use the new-style (Openstep, Rhapsody, MacOS) dynamic
   linker (dyld) instead of the old-style (NextStep) dynamic linker (rld).
   Dyld is necessary to support frameworks. */
/* #undef WITH_DYLD */

/* Define to 1 if libintl is needed for locale functions. */
/* #undef WITH_LIBINTL */

/* Define if you want to produce an OpenStep/Rhapsody framework (shared
   library plus accessory files). */
/* #undef WITH_NEXT_FRAMEWORK */

/* Define if you want to compile in Python-specific mallocs */
#define WITH_PYMALLOC 1

/* Define if you want to compile in rudimentary thread support */
#define WITH_THREAD 1

/* Define to profile with the Pentium timestamp counter */
/* #undef WITH_TSC */

/* Define if you want pymalloc to be disabled when running under valgrind */
/* #undef WITH_VALGRIND */

/* Define WORDS_BIGENDIAN to 1 if your processor stores words with the most
   significant byte first (like Motorola and SPARC, unlike Intel). */
#if defined AC_APPLE_UNIVERSAL_BUILD
# if defined __BIG_ENDIAN__
#  define WORDS_BIGENDIAN 1
# endif
#else
# ifndef WORDS_BIGENDIAN
/* #  undef WORDS_BIGENDIAN */
# endif
#endif

/* Define if arithmetic is subject to x87-style double rounding issue */
#define X87_DOUBLE_ROUNDING 1

/* Define on OpenBSD to activate all library features */
/* #undef _BSD_SOURCE */

/* Define on Irix to enable u_int */
#define _BSD_TYPES 1

/* Define on Darwin to activate all library features */
#define _DARWIN_C_SOURCE 1

/* This must be set to 64 on some systems to enable large file support. */
#define _FILE_OFFSET_BITS 64

/* Define on Linux to activate all library features */
#define _GNU_SOURCE 1

/* This must be defined on some systems to enable large file support. */
#define _LARGEFILE_SOURCE 1

/* This must be defined on AIX systems to enable large file support. */
/* #undef _LARGE_FILES */

/* Define to 1 if on MINIX. */
/* #undef _MINIX */

/* Define on NetBSD to activate all library features */
#define _NETBSD_SOURCE 1

/* Define _OSF_SOURCE to get the makedev macro. */
/* #undef _OSF_SOURCE */

/* Define to 2 if the system does not provide POSIX.1 features except with
   this defined. */
/* #undef _POSIX_1_SOURCE */

/* Define to activate features from IEEE Stds 1003.1-2001 */
//#define _POSIX_C_SOURCE 200112L

/* Define to 1 if you need to in order for `stat' and other things to work. */
/* #undef _POSIX_SOURCE */

/* Define if you have POSIX threads, and your system does not define that. */
/* #undef _POSIX_THREADS */

/* Define to force use of thread-safe errno, h_errno, and other functions */
/* #undef _REENTRANT */

/* Define for Solaris 2.5.1 so the uint32_t typedef from <sys/synch.h>,
   <pthread.h>, or <semaphore.h> is not used. If the typedef were allowed, the
   #define below would cause a syntax error. */
/* #undef _UINT32_T */

/* Define for Solaris 2.5.1 so the uint64_t typedef from <sys/synch.h>,
   <pthread.h>, or <semaphore.h> is not used. If the typedef were allowed, the
   #define below would cause a syntax error. */
/* #undef _UINT64_T */

/* Define to the level of X/Open that your system supports */
//#define _XOPEN_SOURCE 600

/* Define to activate Unix95-and-earlier features */
#define _XOPEN_SOURCE_EXTENDED 1

/* Define on FreeBSD to activate all library features */
#define __BSD_VISIBLE 1

/* Define to 1 if type `char' is unsigned and you are not using gcc.  */
#ifndef __CHAR_UNSIGNED__
/* # undef __CHAR_UNSIGNED__ */
#endif

/* Define to 'long' if <time.h> doesn't define. */
/* #undef clock_t */

/* Define to empty if `const' does not conform to ANSI C. */
/* #undef const */

/* Define to `int' if <sys/types.h> doesn't define. */
/* #undef gid_t */

/* Define to `__inline__' or `__inline' if that's what the C compiler
   calls it, or to nothing if 'inline' is not supported under any name.  */
#ifndef __cplusplus
/* #undef inline */
#endif

/* Define to the type of a signed integer type of width exactly 32 bits if
   such a type exists and the standard includes do not define it. */
/* #undef int32_t */

/* Define to the type of a signed integer type of width exactly 64 bits if
   such a type exists and the standard includes do not define it. */
/* #undef int64_t */

/* Define to `int' if <sys/types.h> does not define. */
/* #undef mode_t */

/* Define to `long int' if <sys/types.h> does not define. */
/* #undef off_t */

/* Define to `int' if <sys/types.h> does not define. */
/* #undef pid_t */

/* Define to empty if the keyword does not work. */
/* #undef signed */

/* Define to `unsigned int' if <sys/types.h> does not define. */
/* #undef size_t */

/* Define to `int' if <sys/socket.h> does not define. */
/* #undef socklen_t */

/* Define to `int' if <sys/types.h> doesn't define. */
/* #undef uid_t */

/* Define to the type of an unsigned integer type of width exactly 32 bits if
   such a type exists and the standard includes do not define it. */
/* #undef uint32_t */

/* Define to the type of an unsigned integer type of width exactly 64 bits if
   such a type exists and the standard includes do not define it. */
/* #undef uint64_t */

/* Define to empty if the keyword does not work. */
/* #undef volatile */


/* Define the macros needed if on a UnixWare 7.x system. */
#if defined(__USLC__) && defined(__SCO_VERSION__)
#define STRICT_SYSV_CURSES /* Don't use ncurses extensions */
#endif
#endif
#endif /*Py_PYCONFIG_H*/

