// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

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
