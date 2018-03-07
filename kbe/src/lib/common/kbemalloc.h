/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2018 KBEngine.

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

/*
*/
#ifndef KBE_MEMORY_MALLOC_H
#define KBE_MEMORY_MALLOC_H

#ifdef USE_JEMALLOC
#include "jemalloc/jemalloc.h"
#endif

//#define USE_KBE_MALLOC				
//#define KBE_MALLOC_ALIGN			16

#ifdef USE_KBE_MALLOC
#include "nedmalloc/nedmalloc.h"

#if KBE_PLATFORM == PLATFORM_WIN32
#ifdef _DEBUG
#pragma comment (lib, "nedmalloc_d.lib")
#else
#pragma comment (lib, "nedmalloc.lib")
#endif
#endif

inline void* operator new(size_t size)
{
#if KBE_MALLOC_ALIGN == 0
    return nedalloc::nedpmalloc(0, size);
#else
    return nedalloc::nedpmemalign(0, KBE_MALLOC_ALIGN, size);
#endif
}

inline void* operator new[](size_t size)
{
#if KBE_MALLOC_ALIGN == 0
    return nedalloc::nedpmalloc(0, size);
#else
    return nedalloc::nedpmemalign(0, KBE_MALLOC_ALIGN, size);
#endif
}

inline void  operator delete(void* p)
{
    nedalloc::nedpfree(0, p);
}

inline void  operator delete[](void* p)
{
    nedalloc::nedpfree(0, p);
}

#endif
#endif
