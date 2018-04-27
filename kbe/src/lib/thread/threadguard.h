// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

/*
	线程守护体：
		避免线程之间恶意竞争产生死锁问题。
	用法:
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
#ifndef KBE_THREADGUARD_H
#define KBE_THREADGUARD_H
	
#include "thread/threadmutex.h"
#include <assert.h>
	
namespace KBEngine{ namespace thread{

class ThreadGuard
{
public:
	explicit ThreadGuard(ThreadMutex* mutexPtr):mutexPtr_(mutexPtr)
	{
		mutexPtr_->lockMutex();
	}

	virtual ~ThreadGuard(void) 
	{ 
		mutexPtr_->unlockMutex();
	}	
	
protected:
	ThreadMutex* mutexPtr_;
};

}
}

#endif // KBE_THREADGUARD_H
