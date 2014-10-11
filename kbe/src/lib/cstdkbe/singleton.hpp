/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 KBEngine.

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
	�÷�:
		class A:public Singleton<A>
		{
		};
		��cpp�ļ���:
		template<> A* Singleton<A>::singleton_ = 0;
*/
#ifndef KBE_SINGLETON_HPP
#define KBE_SINGLETON_HPP

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
#endif // KBE_SINGLETON_HPP
