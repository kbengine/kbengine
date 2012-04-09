////////////////////////////////////////////////////////////////////////////////
//  $Id: vld.h,v 1.15.2.1 2005/08/03 23:13:47 dmouldin Exp $
//
//  Visual Leak Detector (Version 1.0)
//  Copyright (c) 2005 Dan Moulding
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU Lesser General Public License as published by
//  the Free Software Foundation; either version 2.1 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  See COPYING.txt for the full terms of the GNU Lesser General Public License.
//
////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifdef _DEBUG

////////////////////////////////////////////////////////////////////////////////
//
//  Configuration Options
//

// Configuration flags
#define VLD_CONFIG_AGGREGATE_DUPLICATES 0x1
#define VLD_CONFIG_SELF_TEST            0x2
#define VLD_CONFIG_SHOW_USELESS_FRAMES  0x4
#define VLD_CONFIG_START_DISABLED       0x8

#ifndef VLDBUILD

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// If VLD_AGGREGATE_DUPLICATES is defined, then duplicated leaks (those that
// have the same size and call stack as a previously identified leak) are not
// shown in detail in the memory leak report. Instead, only a count of the
// number of matching duplicate leaks is shown along with the detailed
// information from the first such leak encountered.
#ifdef VLD_AGGREGATE_DUPLICATES
#define VLD_FLAG_AGGREGATE_DUPLICATES VLD_CONFIG_AGGREGATE_DUPLICATES
#else
#define VLD_FLAG_AGGREGATE_DUPLICATES 0x0
#endif // VLD_AGGREGATE_DUPLICATES

// If VLD_MAX_DATA_DUMP is defined, then the amount of data shown in user-data
// memory dumps will be limited to the specified number of bytes.
#ifdef VLD_MAX_DATA_DUMP
unsigned long _VLD_maxdatadump = VLD_MAX_DATA_DUMP;
#else
unsigned long _VLD_maxdatadump = 0xffffffff;
#endif // VLD_MAX_DATA_DUMP

// If VLD_MAX_TRACE_FRAMES is defined, then the number of frames traced for each
// allocated memory block when walking the stack will be limited to the
// specified number of frames.
#ifdef VLD_MAX_TRACE_FRAMES
unsigned long _VLD_maxtraceframes = VLD_MAX_TRACE_FRAMES;
#else
unsigned long _VLD_maxtraceframes = 0xffffffff;
#endif // VLD_MAX_TRACE_FRAMES

// If VLD_SELF_TEST is defined, then Visual Leak Detector will perform a memory
// leak self-test, by intentionally leaking memory, to ensure that it is able to
// correctly detect memory leaks internal to Visual Leak Detector.
#ifdef VLD_SELF_TEST
#define VLD_FLAG_SELF_TEST VLD_CONFIG_SELF_TEST
#else
#define VLD_FLAG_SELF_TEST 0x0
#endif // VLD_SELF_TEST

// If VLD_SHOW_USELESS_FRAMES is defined, then all frames traced will be
// displayed, even frames internal to the heap and Visual Leak Detector.
#ifdef VLD_SHOW_USELESS_FRAMES
#define VLD_FLAG_SHOW_USELESS_FRAMES VLD_CONFIG_SHOW_USELESS_FRAMES
#else
#define VLD_FLAG_SHOW_USELESS_FRAMES 0x0
#endif // VLD_SHOW_USELESS_FRAMES

// If VLD_START_DISABLED is defined, then Visual Leak Detector will initially
// be disabled for all threads.
#ifdef VLD_START_DISABLED
#define VLD_FLAG_START_DISABLED VLD_CONFIG_START_DISABLED
#else
#define VLD_FLAG_START_DISABLED 0x0
#endif // VLD_START_DISABLED

// Initialize the configuration flags based on defined preprocessor macros.
unsigned _VLD_configflags = VLD_FLAG_AGGREGATE_DUPLICATES | VLD_FLAG_SELF_TEST |
                            VLD_FLAG_SHOW_USELESS_FRAMES | VLD_FLAG_START_DISABLED;

#ifdef __cplusplus
}
#endif // __cplusplus

////////////////////////////////////////////////////////////////////////////////
//
//  Linker Directives
//

// Link with the appropriate Visual Leak Detector library. One of: multithreaded
// DLL, multithreaded static, or single threaded. All three link with debug
// versions of the CRT.
#ifdef _DLL
#pragma comment (lib, "vldmtdll.lib")
#else
#ifdef _MT
#pragma comment (lib, "vldmt.lib")
#else
#pragma comment (lib, "vld.lib")
#endif // _MT 
#endif // _DLL

// Force a symbolic reference to the global VisualLeakDetector class object from
// the library. This enusres that the object is linked with the program, even
// though nobody directly references it outside of the library.
#pragma comment(linker, "/include:?visualleakdetector@@3VVisualLeakDetector@@A")

#endif // VLDBUILD

#endif // _DEBUG