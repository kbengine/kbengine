/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 kbegine Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
*/
/*
	实现c++单列模式。 参考ogre, 支持跨平台
	用法:
		class A:public Singleton<A>
		{
		};
		在cpp文件中:
		template<> A* Singleton<A>::singleton_ = 0;
*/
#ifndef __SINGLETON_H__
#define __SINGLETON_H__
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// common include
#include "cstdkbe/platform.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>	
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#include <errno.h>
#endif

namespace KBEngine{
	
template <typename T> 
class Singleton
{
protected:
	static T* singleton_;
public:
	Singleton(void)
	{
		assert(!singleton_);
#if defined(_MSC_VER) && _MSC_VER < 1200	 
		int offset = (int)(T*)1 - (int)(Singleton <T>*)(T*)1;
		singleton_ = (T*)((int)this + offset);
#else
		singleton_ = static_cast< T* >(this);
#endif
	}
	
	
	~Singleton(void){  assert(singleton_);  singleton_ = 0; }
	static T& getSingleton(void) { assert(singleton_);  return (*singleton_); }
	static T* getSingletonPtr(void){ return singleton_; }
};

#define KBE_SINGLETON_INIT( TYPE )							\
template <>	 TYPE * Singleton< TYPE >::singleton_ = 0;	\
	
}
#endif // __SINGLETON_H__