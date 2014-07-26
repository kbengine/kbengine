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


#ifndef __THREADPOOL_H__
#define __THREADPOOL_H__

// common include	
// #define NDEBUG
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <iostream>	
#include <list>
#include <queue>
#include <algorithm>
#include "cstdkbe/cstdkbe.hpp"
#include "cstdkbe/tasks.hpp"
#include "helper/debug_helper.hpp"
#include "thread/threadtask.hpp"
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#include <windows.h>          // for HANDLE
#include <process.h>          // for _beginthread()	
#include "helper/crashhandler.hpp"
#else
// linux include
#include <errno.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <pthread.h>	
#endif
	
namespace KBEngine{ namespace thread{

// �̳߳ػ�̴߳��������Ŀ���ڷ�æ״̬
#define THREAD_BUSY_SIZE 32

/*
	�̳߳ص��̻߳���
*/
class ThreadPool;
class TPThread
{
public:
	friend class ThreadPool;

	// �߳�״̬ -1��δ����, 0˯�ߣ� 1��æ��
	enum THREAD_STATE
	{
		THREAD_STATE_STOP = -1,
		THREAD_STATE_SLEEP = 0,
		THREAD_STATE_BUSY = 1,
		THREAD_STATE_END = 2
	};

public:
	TPThread(ThreadPool* threadPool, int threadWaitSecond = 0):
	threadWaitSecond_(threadWaitSecond), 
	currTask_(NULL), 
	threadPool_(threadPool)
	{
		state_ = THREAD_STATE_SLEEP;
		initCond();
		initMutex();
	}
		
	virtual ~TPThread()
	{
		deleteCond();
		deleteMutex();

		DEBUG_MSG(boost::format("TPThread::~TPThread(): %1%\n") % this);
	}
	
	virtual void onStart(){}
	virtual void onEnd(){}

	virtual void onProcessTaskStart(TPTask* pTask){}
	virtual void processTask(TPTask* pTask){ pTask->process(); }
	virtual void onProcessTaskEnd(TPTask* pTask){}

	INLINE THREAD_ID getID(void)const;
	
	INLINE void setID(THREAD_ID tidp);
	
	/**
		����һ���̣߳� �����Լ�����̰߳�
	*/
	THREAD_ID createThread(void);
	
	virtual void initCond(void)
	{
		THREAD_SINGNAL_INIT(cond_);
	}

	virtual void initMutex(void)
	{
		THREAD_MUTEX_INIT(mutex_);	
	}

	virtual void deleteCond(void)
	{
		THREAD_SINGNAL_DELETE(cond_);
	}
	
	virtual void deleteMutex(void)
	{
		THREAD_MUTEX_DELETE(mutex_);
	}

	virtual void lock(void)
	{
		THREAD_MUTEX_LOCK(mutex_); 
	}
	
	virtual void unlock(void)
	{
		THREAD_MUTEX_UNLOCK(mutex_); 
	}	

	virtual TPTask* tryGetTask(void);
	
	/**
		���������ź�
	*/
	int sendCondSignal(void)
	{
		return THREAD_SINGNAL_SET(cond_);
	}
	
	/**
		�߳�֪ͨ �ȴ������ź�
	*/
	bool onWaitCondSignal(void);
	
	bool join(void);

	/**
		��ȡ���߳�Ҫ���������
	*/
	INLINE TPTask* getTask(void)const;

	/**
		���ñ��߳�Ҫ���������
	*/
	INLINE void setTask(TPTask* tpt);

	INLINE int getState(void)const;
	
	/**
		���߳�Ҫ����������Ѿ�������� ���Ǿ���ɾ���������������
	*/
	void onTaskCompleted(void);

#if KBE_PLATFORM == PLATFORM_WIN32
	static unsigned __stdcall threadFunc(void *arg);
#else	
	static void* threadFunc(void* arg);
#endif

	/**
		���ñ��߳�Ҫ���������
	*/
	INLINE ThreadPool* threadPool();
protected:
	THREAD_SINGNAL cond_;			// �߳��ź���
	THREAD_MUTEX mutex_;			// �̻߳�����
	int threadWaitSecond_;			// �߳̿���״̬��������������߳��˳��� С��0Ϊ�����߳� �뵥λ
	TPTask * currTask_;				// ���̵߳ĵ�ǰִ�е�����
	THREAD_ID tidp_;				// ���̵߳�ID
	ThreadPool* threadPool_;		// �̳߳�ָ��
	THREAD_STATE state_;			// �߳�״̬ -1��δ����, 0˯�ߣ� 1��æ��
};


class ThreadPool
{
public:		
	
	ThreadPool();
	virtual ~ThreadPool();
	
	void finalise();

	virtual void onMainThreadTick();
	
	bool hasThread(TPThread* pTPThread);

	/**
		��ȡ��ǰ�߳�����
	*/	
	INLINE uint32 getCurrentThreadCount(void)const;
	
	/**
		��ȡ��ǰ�����߳�����
	*/		
	INLINE uint32 getCurrentFreeThreadCount(void)const;
	
	/**
		�����̳߳�
		@param inewThreadCount			: ��ϵͳ��æʱ�̳߳ػ���������ô���̣߳���ʱ��
		@param inormalMaxThreadCount	: �̳߳ػ�һֱ������ô��������߳�
		@param imaxThreadCount			: �̳߳����ֻ������ô����߳�
	*/
	bool createThreadPool(uint32 inewThreadCount, 
			uint32 inormalMaxThreadCount, uint32 imaxThreadCount);
	
	/**
		���̳߳����һ������
	*/		
	bool addTask(TPTask* tptask);
	INLINE bool addBackgroundTask(TPTask* tptask){ return addTask(tptask); }
	INLINE bool pushTask(TPTask* tptask){ return addTask(tptask); }

	/**
		�߳������Ƿ񵽴�������
	*/
	INLINE bool isThreadCountMax(void)const;
	
	/**
		�̳߳��Ƿ��ڷ�æ״̬
		δ���������Ƿ�ǳ���   ˵���̺ܷ߳�æ
	*/
	INLINE bool isBusy(void)const;
	
	/** 
		�̳߳��Ƿ��Ѿ�����ʼ�� 
	*/
	INLINE bool isInitialize(void)const;

	/**
		�����Ƿ��Ѿ�����
	*/
	INLINE bool isDestroyed()const;

	/**
		�����Ƿ��Ѿ�����
	*/
	INLINE void destroy();

	/** 
		��û������������
	*/
	INLINE uint32 bufferTaskSize()const;

	/** 
		��û��������
	*/
	INLINE std::queue<thread::TPTask*>& bufferedTaskList();

	/** 
		���������������
	*/
	INLINE void lockBufferedTaskList();
	INLINE void unlockBufferedTaskList();

	/** 
		����Ѿ���ɵ���������
	*/
	INLINE uint32 finiTaskSize()const;
public:
	static int timeout;

	/**
		����һ���̳߳��߳�
	*/
	virtual TPThread* createThread(int threadWaitSecond = ThreadPool::timeout);

	/**
		��ĳ�����񱣴浽δ�����б�
	*/
	void bufferTask(TPTask* tptask);

	/**
		��δ�����б�ȡ��һ������ �����б���ɾ��
	*/
	TPTask* popbufferTask(void);

	/**
		�ƶ�һ���̵߳������б�
	*/
	bool addFreeThread(TPThread* tptd);
	
	/**
		�ƶ�һ���̵߳���æ�б�
	*/	
	bool addBusyThread(TPThread* tptd);
	
	/**
		���һ���Ѿ���ɵ������б�
	*/	
	void addFiniTask(TPTask* tptask);
	
	/**
		ɾ��һ������(��ʱ)�߳�
	*/	
	bool removeHangThread(TPThread* tptd);

	bool initializeWatcher();
protected:
	bool isInitialize_;												// �̳߳��Ƿ񱻳�ʼ����
	
	std::queue<TPTask*> bufferedTaskList_;							// ϵͳ���ڷ�æʱ��δ����������б�
	std::list<TPTask*> finiTaskList_;								// �Ѿ���ɵ������б�
	
	THREAD_MUTEX bufferedTaskList_mutex_;							// ����bufferTaskList������
	THREAD_MUTEX threadStateList_mutex_;							// ����bufferTaskList and freeThreadList_������
	THREAD_MUTEX finiTaskList_mutex_;								// ����finiTaskList������
	
	std::list<TPThread*> busyThreadList_;							// ��æ���߳��б�
	std::list<TPThread*> freeThreadList_;							// ���õ��߳��б�
	std::list<TPThread*> allThreadList_;							// ���е��߳��б�
	
	uint32 maxThreadCount_;											// ����߳�����
	uint32 extraNewAddThreadCount_;									// ���normalThreadCount_���㹻ʹ������´�����ô���߳�
	uint32 currentThreadCount_;										// ��ǰ�߳���
	uint32 currentFreeThreadCount_;									// ��ǰ���õ��߳���
	uint32 normalThreadCount_;										// ��׼״̬�µ��߳����� ����Ĭ�������һ�����������Ϳ�����ô���߳�
																	// ����̲߳��㹻������´���һЩ�̣߳� ����ܹ���maxThreadNum.

	bool isDestroyed_;
};

}


}

#ifdef CODE_INLINE
#include "threadpool.ipp"
#endif
#endif
