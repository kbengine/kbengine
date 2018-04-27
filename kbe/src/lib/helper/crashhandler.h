// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_CRASHHANDLER_H
#define KBE_CRASHHANDLER_H
	
// common include	
//#define NDEBUG
// windows include	
#ifdef WIN32
#include "common/common.h"
#include <windows.h>
#include <tchar.h>
#include <dbghelp.h>
#include <stdio.h>
#include <crtdbg.h>
#include <time.h> 
#pragma comment (lib, "dbghelp.lib")
#else
// linux include
#include <errno.h>
#endif
	
namespace KBEngine{ namespace exception {
/** 安装 */
void installCrashHandler(int svnVer, const char* dumpType);

/** 创建dump文件函数 */
void createMiniDump(EXCEPTION_POINTERS* pep ); 

/**  自定义的 minidump callback */
BOOL CALLBACK dumpCallback(
	PVOID                            pParam, 
	const PMINIDUMP_CALLBACK_INPUT   pInput, 
	PMINIDUMP_CALLBACK_OUTPUT        pOutput 
); 

#ifndef _DEBUG
	/** 在要截获crash的代码最开始的地方写上这个宏 */
	#define THREAD_TRY_EXECUTION int exceptionCode = 0;																												\
								__try{
		
	/** 在要截获crash的代码最末尾的地方写上这个宏 */
	#define THREAD_HANDLE_CRASH  }__except(exceptionCode = GetExceptionCode(), KBEngine::exception::createMiniDump(GetExceptionInformation()),						\
															EXCEPTION_EXECUTE_HANDLER) {																			\
									printf("%x\n", exceptionCode);																									\
									wchar_t msg[512];																												\
									wsprintf(msg, L"Exception happened. Exception code is %x.", exceptionCode);														\
									MessageBox(NULL, msg, L"Exception", MB_OK);																						\
								}
#else
	#define THREAD_TRY_EXECUTION 
	#define THREAD_HANDLE_CRASH 
#endif

}
}
#endif
