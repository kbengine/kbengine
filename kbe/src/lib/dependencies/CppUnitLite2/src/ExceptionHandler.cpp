#include "ExceptionHandler.h"
#include "TestResult.h"
#include "Failure.h"
#include "TestException.h"
#include <string.h>
#include <stdio.h>

// BigWorld Modification - allow access to c string on both server and client,
// avoiding depreciated methods on client.
//
// Note we don't want dependency on BigWorld bw_print type functions here.
#ifdef WIN32

#define snprintf _snprintf_s
#define strncat   strncat_s

#endif

namespace ExceptionHandler {


namespace {
    bool g_bHandleExceptions = true;
}


bool IsOn ()
{
    return g_bHandleExceptions;
}

void TurnOn (bool bOn)
{
    g_bHandleExceptions = bOn;
}


void Handle (TestResult& result, const TestException& exception, 
             const char* testname, const char* filename, int linenumber )
{
	const size_t size = 4096;
    char msg[ size ];
    snprintf( msg, size, "Raised exception %s from:\n  %s(%i)", 
		exception.message, exception.file, exception.line );
    result.AddFailure (Failure (msg, testname, filename, linenumber));
}

void Handle (TestResult& result, const char* condition, 
             const char* testname, const char* filename, int linenumber)
{
    if (!g_bHandleExceptions) 
        throw;
    
	const size_t size = 1024;
    char msg[size] = "Unhandled exception ";
	strncat( msg, condition, size );
    result.AddFailure (Failure (msg, testname, filename, linenumber));
}



}
