////////////////////////////////////////////////////////////////////////////////
//  $Id: vldapi.h,v 1.2.2.1 2005/08/03 23:14:26 dmouldin Exp $
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
//  Visual Leak Detector APIs
//

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// VLDEnable - Enables Visual Leak Detector's memory leak detection at runtime.
//   If memory leak detection is already enabled, which it is by default, then
//   calling this function has no effect.
//
//  Note: In multithreaded programs, this function operates on a per-thread
//    basis. In other words, if you call this function from one thread, then
//    memory leak detection is only enabled for that thread. If memory leak
//    detection is disabled for other threads, then it will remain disabled for
//    those other threads. It was designed to work this way to insulate you,
//    the programmer, from having to ensure thread synchronization when calling
//    VLDEnable() and VLDDisable(). Without this, calling these two functions
//    unsychronized could result in unpredictable and unintended behavior.
//    But this also means that if you want to enable memory leak detection
//    process-wide, then you need to call this function from every thread in
//    the process.
//
//  Return Value:
//
//    None.
//
void VLDEnable ();

// VLDDisable - Disables Visual Leak Detector's memory leak detection at
//   runtime. If memory leak detection is already disabled, then calling this
//   function has no effect.
//
//  Note: In multithreaded programs, this function operates on a per-thread
//    basis. In other words, if you call this function from one thread, then
//    memory leak detection is only disabled for that thread. If memory leak
//    detection is enabled for other threads, then it will remain enabled for
//    those other threads. It was designed to work this way to insulate you,
//    the programmer, from having to ensure thread synchronization when calling
//    VLDEnable() and VLDDisable(). Without this, calling these two functions
//    unsychronized could result in unpredictable and unintended behavior.
//    But this also means that if you want to disable memory leak detection
//    process-wide, then you need to call this function from every thread in
//    the process.
//
//  Return Value:
//
//    None.
//
void VLDDisable ();

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _DEBUG