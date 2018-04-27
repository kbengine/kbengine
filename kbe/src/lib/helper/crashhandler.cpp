// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include "crashhandler.h"
namespace KBEngine{ namespace exception {
static wchar_t _g_fileName[512] = {0};

//-------------------------------------------------------------------------------------
void installCrashHandler(int svnVer, const char* dumpType)
{
	wchar_t* wsz = strutil::char2wchar(const_cast<char*>(dumpType));
	wsprintf(_g_fileName, L"CrashDumps\\%ls_v%d.dmp", wsz, svnVer);
	free(wsz);
}

//-------------------------------------------------------------------------------------
void createMiniDump(EXCEPTION_POINTERS* pep) 
{
	if(pep == 0)
	{
		// Raise an exception :P
		__try
		{
			RaiseException(EXCEPTION_BREAKPOINT, 0, 0, 0);
		}
		__except(KBEngine::exception::createMiniDump(GetExceptionInformation()), EXCEPTION_CONTINUE_EXECUTION)
		{
		}		
	}
	
	// 每次都尝试创建一个存放CrashDump目录
	CreateDirectory(L"CrashDumps", 0);

	// Open the file 
	HANDLE hFile = CreateFile(_g_fileName, GENERIC_READ | GENERIC_WRITE, 
		0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL ); 

	if( ( hFile != NULL ) && ( hFile != INVALID_HANDLE_VALUE ) ) 
	{
		// Create the minidump 
		MINIDUMP_EXCEPTION_INFORMATION mdei; 

		mdei.ThreadId           = GetCurrentThreadId(); 
		mdei.ExceptionPointers  = pep; 
		mdei.ClientPointers     = FALSE; 

		MINIDUMP_CALLBACK_INFORMATION mci; 

		mci.CallbackRoutine     = (MINIDUMP_CALLBACK_ROUTINE)dumpCallback; 
		mci.CallbackParam       = 0; 

		MINIDUMP_TYPE mdt       = (MINIDUMP_TYPE)(MiniDumpWithIndirectlyReferencedMemory | MiniDumpScanMemory); 

		BOOL rv = MiniDumpWriteDump( GetCurrentProcess(), GetCurrentProcessId(), 
			hFile, mdt, (pep != 0) ? &mdei : 0, 0, &mci ); 

		if( !rv ) 
			_tprintf( _T("DumpWriteDump failed. Error: %u \n"), GetLastError() ); 
		else 
			_tprintf( _T("dump created.\n") ); 

		// Close the file 
		CloseHandle( hFile ); 

	}
	else 
	{
		_tprintf( _T("CreateFile failed. Error: %u \n"), GetLastError() ); 
	}

}

//-------------------------------------------------------------------------------------
BOOL CALLBACK dumpCallback(
	PVOID                            pParam, 
	const PMINIDUMP_CALLBACK_INPUT   pInput, 
	PMINIDUMP_CALLBACK_OUTPUT        pOutput 
) 
{
	BOOL bRet = FALSE; 


	// Check parameters 
	if( pInput == 0 ) 
		return FALSE; 

	if( pOutput == 0 ) 
		return FALSE; 

	// Process the callbacks 
	switch( pInput->CallbackType ) 
	{
		case IncludeModuleCallback: 
		{
			// Include the module into the dump 
			bRet = TRUE; 
		}
		break; 

		case IncludeThreadCallback: 
		{
			// Include the thread into the dump 
			bRet = TRUE; 
		}
		break; 

		case ModuleCallback: 
		{
			// Does the module have ModuleReferencedByMemory flag set ? 

			if( !(pOutput->ModuleWriteFlags & ModuleReferencedByMemory) ) 
			{
				// No, it does not - exclude it 

				wprintf( L"Excluding module: %s \n", pInput->Module.FullPath ); 
				pOutput->ModuleWriteFlags &= (~ModuleWriteModule); 
			}

			bRet = TRUE; 
		}
		break; 

		case ThreadCallback: 
		{
			// Include all thread information into the minidump 
			bRet = TRUE;  
		}
		break; 

		case ThreadExCallback: 
		{
			// Include this information 
			bRet = TRUE;  
		}
		break; 

		case MemoryCallback: 
		{
			// We do not include any information here -> return FALSE 
			bRet = FALSE; 
		}
		break; 

		case CancelCallback: 
			break; 
	}

	return bRet; 
}

//-------------------------------------------------------------------------------------
}
}
