/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 kbegine Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
*/

#ifndef __KBE_PLATFORM_H__
#define __KBE_PLATFORM_H__
// common include	
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h> 
#include <iostream>  
#include <string>  
#include <cstring>  
#include <vector>
#include <map>
#include <list>
#include <set>
#include <assert.h> 
#include <algorithm>
// windows include	
#if defined( __WIN32__ ) || defined( WIN32 ) || defined( _WIN32 )
#include <time.h> 
#include <winsock2.h>		// 必须在windows.h之前包含， 否则网络模块编译会出错
#include <mswsock.h> 
#include <windows.h> 
#include <unordered_map>
#else
// linux include
#include <errno.h>
#include <pthread.h>	
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <netdb.h>
#include <tr1/unordered_map>
#include <langinfo.h>   /* CODESET */
#endif

/** 定义引擎名字空间 */
namespace KBEngine{ 
/** 定义引擎字节序 */
#define KBENGINE_LITTLE_ENDIAN							0
#define KBENGINE_BIG_ENDIAN								1
#if !defined(KBENGINE_ENDIAN)
#  if defined (ACE_BIG_ENDIAN)
#    define KBENGINE_ENDIAN KBENGINE_BIG_ENDIAN
#  else 
#    define KBENGINE_ENDIAN KBENGINE_LITTLE_ENDIAN
#  endif 
#endif


// current platform and compiler
#define PLATFORM_WIN32 0
#define PLATFORM_UNIX  1
#define PLATFORM_APPLE 2

#define UNIX_FLAVOUR_LINUX 1
#define UNIX_FLAVOUR_BSD 2
#define UNIX_FLAVOUR_OTHER 3
#define UNIX_FLAVOUR_OSX 4

#if defined( __WIN32__ ) || defined( WIN32 ) || defined( _WIN32 )
#  define KBE_PLATFORM PLATFORM_WIN32
#elif defined( __INTEL_COMPILER )
#  define KBE_PLATFORM PLATFORM_INTEL
#elif defined( __APPLE_CC__ )
#  define KBE_PLATFORM PLATFORM_APPLE
#else
#  define KBE_PLATFORM PLATFORM_UNIX
#endif

#define COMPILER_MICROSOFT 0
#define COMPILER_GNU	   1
#define COMPILER_BORLAND   2
#define COMPILER_INTEL     3

#ifdef _MSC_VER
#  define KBE_COMPILER COMPILER_MICROSOFT
#elif defined( __INTEL_COMPILER )
#  define KBE_COMPILER COMPILER_INTEL
#elif defined( __BORLANDC__ )
#  define KBE_COMPILER COMPILER_BORLAND
#elif defined( __GNUC__ )
#  define KBE_COMPILER COMPILER_GNU
#else
#  pragma error "FATAL ERROR: Unknown compiler."
#endif

#if KBE_PLATFORM == PLATFORM_UNIX || KBE_PLATFORM == PLATFORM_APPLE
#ifdef HAVE_DARWIN
#define KBE_PLATFORM_TEXT "MacOSX"
#define UNIX_FLAVOUR UNIX_FLAVOUR_OSX
#else
#ifdef USE_KQUEUE
#define KBE_PLATFORM_TEXT "FreeBSD"
#define UNIX_FLAVOUR UNIX_FLAVOUR_BSD
#else
#ifdef USE_KQUEUE_DFLY
#define KBE_PLATFORM_TEXT "DragonFlyBSD"
#define UNIX_FLAVOUR UNIX_FLAVOUR_BSD
#else
#define KBE_PLATFORM_TEXT "Linux"
#define UNIX_FLAVOUR UNIX_FLAVOUR_LINUX
#endif
#endif
#endif
#endif

#if KBE_PLATFORM == PLATFORM_WIN32
#define KBE_PLATFORM_TEXT "Win32"
#endif

#ifndef KBE_CONFIG
#ifdef _DEBUG
#define KBE_CONFIG "Debug"
#else
#define KBE_CONFIG "Release"
#endif
#endif

#ifdef X64
#define KBE_ARCH "X64"
#else
#define KBE_ARCH "X86"
#endif

/*---------------------------------------------------------------------------------
	类型定义
---------------------------------------------------------------------------------*/
/* Use correct types for x64 platforms, too */
#if KBE_COMPILER != COMPILER_GNU
typedef signed __int64											int64;
typedef signed __int32											int32;
typedef signed __int16											int16;
typedef signed __int8											int8;
typedef unsigned __int64										uint64;
typedef unsigned __int32										uint32;
typedef unsigned __int16										uint16;
typedef unsigned __int8											uint8;
#else
#include <stdint.h>
typedef int64_t													int64;
typedef int32_t													int32;
typedef int16_t													int16;
typedef int8_t													int8;
typedef uint64_t												uint64;
typedef uint32_t												uint32;
typedef uint16_t												uint16;
typedef uint8_t													uint8;
typedef uint16_t												WORD;
typedef uint32_t												DWORD;
#endif

typedef uint16													ENTITY_TYPE;											// entity的类别类型定义支持0-65535个类别
typedef int32													ENTITY_ID;												// entityID的类型
typedef uint32													SPACE_ID;												// 一个space的id
typedef uint32													CALLBACK_ID;											// 一个callback由CallbackMgr分配的id
typedef uint32													COMPONENT_ID;											// 一个服务器组件的id										
typedef	uint32													TIMER_ID;												// 一个timer的id类型
typedef uint8													MAIL_TYPE;												// mailbox 所投递的mail类别的类别


/*---------------------------------------------------------------------------------
	定会多种平台上的多线程相关
---------------------------------------------------------------------------------*/
#if KBE_PLATFORM == PLATFORM_WIN32
	#define THREAD_ID											HANDLE
	#define THREAD_SINGNAL										HANDLE
	#define THREAD_SINGNAL_INIT(x)								x = CreateEvent(NULL, TRUE, FALSE, NULL)
	#define THREAD_SINGNAL_DELETE(x)							CloseHandle(x)
	#define THREAD_SINGNAL_SET(x)								SetEvent(x)
	#define THREAD_MUTEX										CRITICAL_SECTION
	#define THREAD_MUTEX_INIT(x)								InitializeCriticalSection(&x)
	#define THREAD_MUTEX_DELETE(x)								DeleteCriticalSection(&x)
	#define THREAD_MUTEX_LOCK(x)								EnterCriticalSection(&x)
	#define THREAD_MUTEX_UNLOCK(x)								LeaveCriticalSection(&x)	
#else
	#define THREAD_ID											pthread_t
	#define THREAD_SINGNAL										pthread_cond_t
	#define THREAD_SINGNAL_INIT(x)								pthread_cond_init(&x, NULL)
	#define THREAD_SINGNAL_DELETE(x)							pthread_cond_destroy(&x)
	#define THREAD_SINGNAL_SET(x)								pthread_cond_signal(&x);
	#define THREAD_MUTEX										pthread_mutex_t
	#define THREAD_MUTEX_INIT(x)								pthread_mutex_init (&x, NULL)
	#define THREAD_MUTEX_DELETE(x)								pthread_mutex_destroy(&x)
	#define THREAD_MUTEX_LOCK(x)								pthread_mutex_lock(&x)
	#define THREAD_MUTEX_UNLOCK(x)								pthread_mutex_unlock(&x)		
#endif

/*---------------------------------------------------------------------------------
	跨平台接口定义
---------------------------------------------------------------------------------*/
/** 获取系统时间 */
#if KBE_PLATFORM == PLATFORM_WIN32
	inline uint32 getSystemTime() 
	{ 
		return ::GetTickCount(); 
	};
#else
	inline uint32 getSystemTime()
	{
		struct timeval tv;
		struct timezone tz;
		gettimeofday(&tv, &tz);
		return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
	};
#endif

/** 获取2个系统时间差 */
inline uint32 getSystemTimeDiff(uint32 oldTime, uint32 newTime)
{
    // getSystemTime() have limited data range and this is case when it overflow in this tick
    if (oldTime > newTime)
        return (0xFFFFFFFF - oldTime) + newTime;
    else
        return newTime - oldTime;
}

/** sleep 跨平台 */
#if KBE_PLATFORM == PLATFORM_WIN32
inline void sleep(uint32 ms){ ::Sleep(ms); }
#else
inline void sleep(uint32 ms){ usleep(1000 * ms);}	
#endif

/** 判断平台是否为小端字节序 */
inline bool isPlatformLittleEndian()
{
   int n = 1;
   return *((char*)&n)? true:false;
}

/** 浮点数计算优化 */
#if KBE_COMPILER == COMPILER_MICROSOFT && _MSC_VER >= 1400
#pragma float_control(push)
#pragma float_control(precise, on)
#endif

// fast int abs
static inline int int32abs( const int value )
{
	return (value ^ (value >> 31)) - (value >> 31);
}

// fast int abs and recast to unsigned
static inline uint32 int32abs2uint32( const int value )
{
	return (uint32)(value ^ (value >> 31)) - (value >> 31);
}

/// Fastest Method of float2int32
static inline int float2int32(const float value)
{
#if !defined(X64) && KBE_COMPILER == COMPILER_MICROSOFT 
	int i;
	__asm {
		fld value
		frndint
		fistp i
	}
	return i;
#else
	union { int asInt[2]; double asDouble; } n;
	n.asDouble = value + 6755399441055744.0;

	return n.asInt [0];
#endif
}

/// Fastest Method of long2int32
static inline int long2int32(const double value)
{
#if !defined(X64) && KBE_COMPILER == COMPILER_MICROSOFT
	int i;
	__asm {
		fld value
		frndint
		fistp i
	}
	return i;
#else
  union { int asInt[2]; double asDouble; } n;
  n.asDouble = value + 6755399441055744.0;
  return n.asInt [0];
#endif
}

/** 浮点数计算优化结束 */
#if KBE_COMPILER == COMPILER_MICROSOFT && _MSC_VER >= 1400
#pragma float_control(pop)
#endif


inline char* wchar2char(wchar_t* ts)
{
	char* ccattr =(char *)malloc(MB_CUR_MAX);
    wcstombs(ccattr, ts, MB_CUR_MAX);
	return ccattr;
};

inline wchar_t* char2wchar(char* cs)
{
	wchar_t* ccattr =(wchar_t *)malloc(MB_CUR_MAX);
    mbstowcs(ccattr, cs, MB_CUR_MAX);
	return ccattr;
};
}
#endif
