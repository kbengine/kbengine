/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _LOG4CXX_HELPERS_TCHAR_H
#define _LOG4CXX_HELPERS_TCHAR_H

#error log4cxx/helpers/tchar.h is obsolete, see details following this line.

/**
* A short history of log4cxx's tchar.h
*
* The previous log4cxx/helpers/tchar.h contained macros that
* attempted to replicate macros and functions defined by
* the Microsoft SDK's tchar.h and related header files
* such as _T() and TCHAR.
*
* When building apps using both log4cxx and Microsoft SDK's tchar.h,
* these definitions could conflict and, for example, the code generated
* by _T("Foo") would depend on the include order of the two
* tchar.h's.
*
* The motivation of tchar.h in the Microsoft SDK was to
* support presenting either a wide-char or multi-byte char
* facade to a C API depending on the presence of
* the _UNICODE or _MBCS preprocessor macros.  When _UNICODE
* was set then tchar was typedef'd as wchar_t and, for example,
* the CreateProcess macro was defined to be CreateProcessW,  If
* _MBCS was defined, then tchar was typedef'd as char
* and CreateProcess macro was defined to be CreateProcessA.
*
* In either case, the setting of _UNICODE or _MBCS
* didn't affect the implementation of the operating system.
* If you were running the Windows NT family, all the multi-byte
* methods delegated to a wide-char implementation.
* In the Windows 9x family, most wide-char methods delegated
* to a multi-byte implementation.
*
* In practice, most Microsoft Windows executables were either
* wide-char or multi-byte centric.  However, they did not
* have to be exclusively so.  An application built with
* _UNICODE, could still call multi-byte API functions,
* they would just need to explicitly call CreateProcessA
* instead of using the facade macro.  An executable could
* also use both a multi-byte centric and wide-char centric
* DLL's since all the calls eventually hit the same
* underlying implementation be it a wide-char on in
* Windows NT or multi-char in Windows 9x.
*
* The use of log4cxx/helpers/tchar.h in log4cxx 0.9.7 was
* undesirable because it made log4cxx either exclusively
* wide-char or exclusively multi-byte and had to be consistant
* with the character model of the calling executable.
* This would make it extremely difficult to use
* log4cxx when DLL's with different character models
* where called by the same application.  Since log4cxx
* was C++, not C, function overloading could be
* used instead of the CreateProcess et al macros
* used in the Windows headers.
*
* In the rework before the 0.9.8, the following changes
* were made to log4cxx:
*
* 1. All inclusions of log4cxx/helpers/tchar.h
*       and use of TCHAR, log4cxx::String and _T
*       were removed from log4cxx.
* 2. log4cxx/logstring.h was added to define the
*     implementation character model using the log4cxx::logchar
*     and log4cxx::LogString typedefs and LOG4CXX_STR macro.
* 3. Methods commonly used by calling applications were defined
*    in both wide-char and multi-byte and both pointer and string
*    forms with conversion to the implementation character
*    model delayed as long as possible.
* 4. Use of Standard Template Library streams within
*    log4cxx was substantially reduced (but not totally
*    elminated).
* 5. The LOG4CXX_DEBUG and similar macros were simplified
*    and now only take arguments that evaluate to
*    character pointers or strings and no longer take
*    the right hand side of an insertion operation:
*
*    //   This used to work, but no longer
*    LOG4CXX_DEBUG(logger, "foo" << i);
*
*    If you extensively used this idiom, please consider
*    migrating to stream-like API defined in log4cxx/stream.h.
*
* 6. The LOG4CXX_DEBUG and similar use the LOG4CXX_LOCATION
*    macro to define the log statement location instead of
*    using __FILE__ and __LINE__.  Logger::debug and
*    similar now take const LocationInfo& instead of
*    separate const char* and int arguments.  This allows
*    class and method names to appear in location info.
* 7. log4cxx include files no longer include config.h
*    or related files.  config.h and related files
*    may be used by log4cxx implementation, but have
*    no effect on the exposed API.
*
* It is expected that the default implementation character
* model will be wchar_t.  However this may vary by platform
* and may be changed based on feedback.
*
* Developers using log4cxx should seldom be concerned
* with the internal character model of log4cxx unless
* writing custom appenders or layouts.  An application
* should not be using log4cxx::logchar, log4cxx::LogString
* or LOG4CXX_STR unless dealing with something that is
* clearly a log4cxx internal.  If you find something
* defined as using or returning LogString that you
* don't consider a log4cxx internal, please file a
* bug report or post a message to one of the mailing lists.
*
* wchar_t literals should be preferred in log requests since
* since they eliminate potential encoding confusion
* when the development and deployment encodings are different.
*
* Migration strategies:
*
* If you followed the examples in the previous log4cxx versions,
* you may have _T() macros littered through your code
* and inclusions of this file.  If you are on the Microsoft
* platform, the simplest solution is to just include
* the Platform SDK's tchar.h which would result your log
* statements matching the character model of your application.
*
* If you targetting another platform and your only use of
* _T() in related to log4cxx, then I would recommend replacing
*  all _T() with another macro (say MYAPP_LOGSTR())
* and defining that macro in a commonly included header file
* or defining _T() in a commonly included header file.
*
* I would first try defining these macros as
*
* #define _T(str) L ## str
*
* If that results in too many compilation errors, then try:
*
* #define _T(str) str
*
* Using the first form will result in wchar_t literals which
* will avoid potential encoding confusion and is expected
* to result in slightly better performance when logging.
*
*  Since the best choice for _T() depends on the application,
*  there is not a definition within log4cxx.
*
* Use encoding conversion macros A2T, W2T, et al should
* not longer be necessary.  If you are doing a lot of
* work converting between encodings, you might consider
* using the stream-like interface in log4cxx/stream.h
* which defines insertion operators for multi-byte
* strings in addition to exposing all the
* insertion operations defined for
* std::basic_ostream<wchar_t>.
*
*/

#endif //_LOG4CXX_HELPERS_TCHAR_H
