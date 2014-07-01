#pragma once

//#ifdef _WIN32
//#define WIN32_LEAN_AND_MEAN
////http://www.cnblogs.com/tonyyang132/archive/2009/10/14/1583110.html
//// LEAN_AND_MEAN can avoid winsock issue. e.g:  error C2011: 'sockaddr' : 'struct' type redefinition
//#include <Windows.h>
//#endif
//
//* Use correct types for x64 platforms, too */

//#if KBE_COMPILER != COMPILER_GNU
//typedef signed __int64											int64;
//typedef signed __int32											int32;
//typedef signed __int16											int16;
//typedef signed __int8											int8;
//typedef unsigned __int64										uint64;
//typedef unsigned __int32										uint32;
//typedef unsigned __int16										uint16;
//typedef unsigned __int8											uint8;
//typedef INT_PTR													intptr;
//typedef UINT_PTR        										uintptr;
//#define PRI64													"lld"
//#define PRIu64													"llu"
//#define PRIx64													"llx"
//#define PRIX64													"llX"
//#define PRIzu													"lu"
//#define PRIzd													"ld"
//#define PRTime													PRI64
//#else
//typedef int64_t													int64;
//typedef int32_t													int32;
//typedef int16_t													int16;
//typedef int8_t													int8;
//typedef uint64_t												uint64;
//typedef uint32_t												uint32;
//typedef uint16_t												uint16;
//typedef uint8_t													uint8;
//typedef uint16_t												WORD;
//typedef uint32_t												DWORD;
//#endif
//

//end define common data type. 

//#include "cocos2d.h"

#include <stdint.h>


#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID) ||  (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
#include "ios/KBEClientcore.h"
#else

#include "KBEClientCoreMacros.h"
#include "net/base_sock.h"
#include "net/net_client.h"
#include "util/basic_types.h"

#endif


USING_NS_GC;