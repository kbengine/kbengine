// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

/*
	�̻߳����壺
	�÷�:
		ThreadMutex tm;
		tm.lockMutex();
		....��ȫ����
		tm.unlockMutex();
		
		��������ThreadGuard��ʹ��
		��һ�����ж��廥�����Ա
		ThreadMutex tm;
		����Ҫ�����ĵط�:
		void XXCLASS::func(void)
		{
			ThreadGuard tg(this->tm);
			����Ĵ��붼�ǰ�ȫ��
			...
		}
*/
#ifndef __THREADMUTEX_H__
#define __THREADMUTEX_H__
	
#include "common/common.h"


namespace KBEngine{ namespace Thread{

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
	}

	ThreadMutex(const ThreadMutex& v)
	{
		// ���ﲻ����������mutex_�����Ƿǳ�Σ�յ�
		// ����ɶ��THREAD_MUTEX_DELETE
	}

	virtual ~ThreadMutex(void)
	{ 
	}	
	
	virtual void lockMutex(void)
	{
		mutex_.lock();
	}

	virtual void unlockMutex(void)
	{
		mutex_.unlock();
	}

protected:
	std::mutex mutex_;
};

}
}
#endif
