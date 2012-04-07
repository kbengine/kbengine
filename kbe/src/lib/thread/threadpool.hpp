/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 kbegine Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
*/
/*
	线程池
	
	使用用法:
	if(ThreadPool::getSingletonPtr() && !ThreadPool::getSingleton().isInitialize())
		ThreadPool::getSingleton().createThreadPool(inewThreadCount, inormalMaxThreadCount, imaxThreadCount);

	@param inewThreadCount			: 当系统繁忙时线程池会新增加这么多线程（临时）
	@param inormalMaxThreadCount	: 线程池会一直保持这么多个数的线程
	@param imaxThreadCount			: 线程池最多只能有这么多个线程
	当线程池线程到达最大数量时， 一些没有处理的任务会被暂时保存到队列里等待空闲线程来处理
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
#include "helper/debug_helper.hpp"
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#include <windows.h>          // for HANDLE
#include <process.h>          // for _beginthread()	
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

		
/*
	线程池的任务抽象类
*/
class TPTask
{
protected:	
public:	
	TPTask(){}
	virtual ~TPTask(){}

	/*
		具体任务处理接口，  需要继承类完成具体实现
	*/
	virtual void onHandle(void) = 0;
};

/*
	线程池的线程基类
*/
class ThreadPool;
class TPThread
{
protected:
	THREAD_SINGNAL m_cond;			// 线程信号量
	THREAD_MUTEX m_mutex;			// 线程互诉体
	int m_threadWaitSecond;			// 线程空闲状态超过这个秒数则线程退出， 小于0为永久线程 秒单位
	TPTask * m_currTask;			// 该线程的当前执行的任务
	THREAD_ID m_tidp;				// 本线程的ID
	ThreadPool* m_threadPool;		// 线程池指针
	int m_state;					// 线程状态 -1还未启动, 0睡眠， 1繁忙中
public:
	TPThread(ThreadPool* threadPool, int threadWaitSecond = 0) : \
	m_threadWaitSecond(threadWaitSecond), 
	m_currTask(NULL), 
	m_threadPool(threadPool)
	{
		m_state = 0;
		initCond();
		initMutex();
	}
		
	virtual ~TPThread()
	{
		uninitCond();
		uninitMutex();
	}
	
	THREAD_ID getID(void)const{
		return m_tidp;
	}
	
	void setID(THREAD_ID tidp){
		m_tidp = tidp;
	}
	
	/**创建一个线程， 并将自己与该线程绑定*/
	THREAD_ID createThread(void);
	
	virtual void initCond(void){
		THREAD_SINGNAL_INIT(m_cond);
	}

	virtual void initMutex(void){
		THREAD_MUTEX_INIT(m_mutex);	
	}

	virtual void uninitCond(void){
		THREAD_SINGNAL_DELETE(m_cond);
	}
	
	virtual void uninitMutex(void){
		THREAD_MUTEX_DELETE(m_mutex);
	}

	virtual void lock(void){
		THREAD_MUTEX_LOCK(m_mutex); 
	}
	
	virtual void unlock(void){
		THREAD_MUTEX_UNLOCK(m_mutex); 
	}	

	virtual TPTask* tryGetTask(void);
	
	/**发送条件信号*/
	int sendCondSignal(void){
		return THREAD_SINGNAL_SET(m_cond);
	}
	
	/**线程通知 等待条件信号*/
	bool onWaitCondSignal(void);

	/**获取本线程要处理的任务*/
	TPTask* getTask(void)const{
		return m_currTask;
	}

	/**设置本线程要处理的任务*/
	void setTask(TPTask* tpt){
		m_currTask = tpt;
	}

	int getState(void)const{
		return m_state;
	}
	
	/**本线程要处理的任务已经处理完毕 我们决定删除这个废弃的任务*/
	void onTaskComplete(void);

#if KBE_PLATFORM == PLATFORM_WIN32
	static unsigned __stdcall threadFunc(void *arg)
#else	
	static void* threadFunc(void* arg)
#endif
	{
		TPThread * tptd = (TPThread*) arg;
		bool isRun = true;
#if KBE_PLATFORM == PLATFORM_WIN32
		THREAD_TRY_EXECUTION;
#else			
		pthread_detach(pthread_self());
#endif
		while(isRun)
		{
			isRun = tptd->onWaitCondSignal();
			tptd->m_state = 1;
			TPTask * task = tptd->getTask();
			
			while(task)
			{
				task->onHandle();									// 处理该任务
				TPTask * task1 = tptd->tryGetTask();				// 尝试继续从任务队列里取出一个繁忙的未处理的任务

				if(!task1){
					tptd->onTaskComplete();
					break;
				}
				else
				{
					delete task;
					task = task1;
					tptd->setTask(task1);
				}
			}
		}

#if KBE_PLATFORM == PLATFORM_WIN32
		THREAD_HANDLE_CRASH;
		return 0;
#else	
		pthread_exit(NULL);
		return NULL;
#endif		
	}
	
};


class ThreadPool : public Singleton<ThreadPool>
{
protected:
	bool m_isInitialize;											// 线程池是否被初始化过
	std::queue<TPTask*> m_busyTaskList;								// 系统处于繁忙时还未处理的任务列表
	THREAD_MUTEX busyTaskList_mutex;								// 处理m_busyTaskList互斥锁
	THREAD_MUTEX threadStateList_mutex;								// 处理m_busyThreadList and m_freeThreadList互斥锁
	std::list<TPThread*> m_busyThreadList;							// 繁忙的线程列表
	std::list<TPThread*> m_freeThreadList;							// 闲置的线程列表
	std::list<TPThread*> m_allThreadList;							// 所有的线程列表
	unsigned int m_maxThreadCount;									// 最大线程总数
	unsigned int m_extraNewAddThreadCount;							// 如果m_normalThreadCount不足够使用则会新创建这么多线程
	unsigned int m_currentThreadCount;								// 当前线程数
	unsigned int m_currentFreeThreadCount;							// 当前闲置的线程数
	unsigned int m_normalThreadCount;								// 标准状态下的线程总数 即：默认情况下一启动服务器就开启这么多线程
																	// 如果线程不足够，则会新创建一些线程， 最大能够到maxThreadNum.
																	
protected:
	/**创建一个线程池线程*/														
	TPThread* createThread(int threadWaitSecond = 0);													
public:		
	
	ThreadPool()
	{		
		m_extraNewAddThreadCount = m_currentThreadCount = m_currentFreeThreadCount = m_normalThreadCount = 0;
		THREAD_MUTEX_INIT(threadStateList_mutex);	
		THREAD_MUTEX_INIT(busyTaskList_mutex);
	}

	virtual ~ThreadPool()
	{
		THREAD_MUTEX_DELETE(threadStateList_mutex);
		THREAD_MUTEX_DELETE(busyTaskList_mutex);
		std::list<TPThread*>::iterator itr = m_allThreadList.begin();
		for(; itr != m_allThreadList.end(); itr++)
		{
			if((*itr))
			{
				delete (*itr);
				(*itr) = NULL;
			}
		}
	}
	
	/**获取当前线程总数*/	
	unsigned int getCurrentThreadCount(void)const{ 
		return m_currentThreadCount; 
	}
	
	/**获取当前空闲线程总数*/		
	unsigned int getCurrentFreeThreadCount(void)const{ 
		return m_currentFreeThreadCount; 
	}
	
	/**
		创建线程池
		@param inewThreadCount			: 当系统繁忙时线程池会新增加这么多线程（临时）
		@param inormalMaxThreadCount	: 线程池会一直保持这么多个数的线程
		@param imaxThreadCount			: 线程池最多只能有这么多个线程
	*/
	bool createThreadPool(unsigned int inewThreadCount, unsigned int inormalMaxThreadCount, unsigned int imaxThreadCount);

	/**移动一个线程到空闲列表*/
	bool moveThreadToFreeList(TPThread* tptd);
	
	/**移动一个线程到繁忙列表*/	
	bool moveThreadToBusyList(TPThread* tptd);
	
	/**删除一个挂起(超时)线程*/	
	bool removeHangThread(TPThread* tptd);
	
	/**向线程池添加一个任务*/		
	bool addTask(TPTask* tptask);
	
	/**线程数量是否到达最大个数*/
	bool isThreadCountMax(void)const{
		return m_currentThreadCount >= m_maxThreadCount;	
	}
	
	/**
		线程池是否处于繁忙状态
		未处理任务是否非常多   说明线程很繁忙
	*/
	bool isBusy(void)const{
		return m_busyTaskList.size() > 32;
	}	

	/**将某个任务保存到未处理列表*/
	void saveToBusyTaskList(TPTask* tptask);

	/**从未处理列表取出一个任务 并从列表中删除*/
	TPTask* popBusyTaskList(void);
	
	/** 线程池是否已经被初始化 */
	bool isInitialize(void)const{ 
		return m_isInitialize; 
	}
};

}
}
#endif
