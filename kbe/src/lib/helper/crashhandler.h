/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2017 KBEngine.

KBEngine is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

KBEngine is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.
 
You should have received a copy of the GNU Lesser General Public License
along with KBEngine.  If not, see <http://www.gnu.org/licenses/>.
*/

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
/** ��װ */
void installCrashHandler(int svnVer, const char* dumpType);

/** ����dump�ļ����� */
void createMiniDump(EXCEPTION_POINTERS* pep ); 

/**  �Զ���� minidump callback */
BOOL CALLBACK dumpCallback(
	PVOID                            pParam, 
	const PMINIDUMP_CALLBACK_INPUT   pInput, 
	PMINIDUMP_CALLBACK_OUTPUT        pOutput 
); 

#ifndef _DEBUG
	/** ��Ҫ�ػ�crash�Ĵ����ʼ�ĵط�д������� */
	#define THREAD_TRY_EXECUTION int exceptionCode = 0;																												\
								__try{
		
	/** ��Ҫ�ػ�crash�Ĵ�����ĩβ�ĵط�д������� */
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
