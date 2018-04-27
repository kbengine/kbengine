// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

/*
	线程互诉体：
	用法:
		ThreadMutex tm;
		tm.lockMutex();
		....安全代码
		tm.unlockMutex();
		
		最好是配合ThreadGuard来使用
		在一个类中定义互诉体成员
		ThreadMutex tm;
		在需要保护的地方:
		void XXCLASS::func(void)
		{
			ThreadGuard tg(this->tm);
			下面的代码都是安全的
			...
		}
*/
#ifndef __THREADMUTEX_H__
#define __THREADMUTEX_H__
	
#include "common/common.h"


namespace KBEngine{ namespace thread{

class ThreadMutexNull
{
public:
	ThreadMutexNull(void)
	{
	}

	virtual ~ThreadMutexNull(void)
	{
	}

	virtual void lockMutex(void)
	{
	}

	virtual void unlockMutex(void)
	{
	}
};

class ThreadMutex : public ThreadMutexNull
{
public:
	ThreadMutex(void)
	{
		THREAD_MUTEX_INIT(mutex_);
	}

	ThreadMutex(const ThreadMutex& v)
	{
		// 这里不允许拷贝构造mutex_，这是非常危险的
		// 会造成多次THREAD_MUTEX_DELETE
		THREAD_MUTEX_INIT(mutex_);
	}

	virtual ~ThreadMutex(void)
	{ 
		THREAD_MUTEX_DELETE(mutex_);
	}	
	
	virtual void lockMutex(void)
	{
		THREAD_MUTEX_LOCK(mutex_);
	}

	virtual void unlockMutex(void)
	{
		THREAD_MUTEX_UNLOCK(mutex_);
	}

protected:
	THREAD_MUTEX mutex_;
};

}
}
#endif
