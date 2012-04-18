/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 kbegine Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
*/
/*
	线程互诉体：
		避免线程之间恶意竞争产生死锁问题。
	用法:
		ThreadMutex tm;
		tm.lockMutex();
		....安全代码
		tm.unlockMutex();
		
		最好是配合ThreadGuard来使用
		在一个类中定义互诉体成员
		ThreadMutex tm;
		在需要保护的地方:
		void XXCLASS::foo(void)
		{
			ThreadGuard tg(this->tm);
			下面的代码都是安全的
			...
		}
*/
#ifndef __THREADMUTEX_H__
#define __THREADMUTEX_H__
	
// common include
#include "cstdkbe/cstdkbe.hpp"
//#define NDEBUG
#include <assert.h>

// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#include <errno.h>
#endif
	
namespace KBEngine{ namespace thread{

class ThreadMutex 
{
public:
	ThreadMutex(void)
	{
		THREAD_MUTEX_INIT(mutex_);
	}

	virtual ~ThreadMutex(void) 
	{ 
		THREAD_MUTEX_DELETE(mutex_);
	}	
	
	void lockMutex(void)
	{
		THREAD_MUTEX_LOCK(mutex_);
	}

	void unlockMutex(void)
	{
		THREAD_MUTEX_UNLOCK(mutex_);
	}
protected:
	THREAD_MUTEX mutex_;
};

}
}
#endif
