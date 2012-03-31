/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 kbegine Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
*/

#ifndef G3D_FORMAT_H
#define G3D_FORMAT_H

#include "platform.h"
#include <string>
#include <stdio.h>
#include <cstdarg>
#include <assert.h>
#ifndef G3D_WIN32
    // Don't include varargs.h for some random
    // gcc reason
    //#include <varargs.h>
    #include <stdarg.h>
#endif

#ifndef _MSC_VER
    #ifndef __cdecl
        #define __cdecl __attribute__((cdecl))
    #endif
#endif

namespace G3D {

/**
  Produces a string from arguments of the style of printf.  This avoids
  problems with buffer overflows when using sprintf and makes it easy
  to use the result functionally.  This function is fast when the resulting
  string is under 160 characters (not including terminator) and slower
  when the string is longer.
 */
std::string format(
    const char*                 fmt
    ...) G3D_CHECK_PRINTF_ARGS;

/**
  Like format, but can be called with the argument list from a ... function.
 */
std::string vformat(
    const char*                 fmt,
    va_list                     argPtr) G3D_CHECK_VPRINTF_ARGS;


}; // namespace

#endif
