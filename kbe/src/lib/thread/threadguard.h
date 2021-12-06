// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

/*
	�߳��ػ��壺
		�����߳�֮����⾺�������������⡣
	�÷�:
		��һ�����ж��廥�����Ա
		ThreadMutex tm;
		����Ҫ�����ĵط�:
		void XXCLASS::foo(void)
		{
			ThreadGuard tg(this->tm);
			����Ĵ��붼�ǰ�ȫ��
			...
		}
*/
#ifndef KBE_THREADGUARD_H
#define KBE_THREADGUARD_H
	
#include "thread/threadmutex.h"
#include <assert.h>
	
namespace KBEngine{ namespace Thread{

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
