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

/*
	线程池的线程基类
*/
class ThreadPool;
class TPThread
{
public:
	TPThread(ThreadPool* threadPool, int threadWaitSecond = 0):
	threadWaitSecond_(threadWaitSecond), 
	currTask_(NULL), 
	threadPool_(threadPool)
	{
		state_ = 0;
		initCond();
		initMutex();
	}
		
	virtual ~TPThread()
	{
		uninitCond();
		uninitMutex();
	}
	
	THREAD_ID getID(void)const
	{
		return tidp_;
	}
	
	void setID(THREAD_ID tidp)
	{
		tidp_ = tidp;
	}
	
	/**创建一个线程， 并将自己与该线程绑定*/
	THREAD_ID createThread(void);
	
	virtual void initCond(void)
	{
		THREAD_SINGNAL_INIT(cond_);
	}

	virtual void initMutex(void)
	{
		THREAD_MUTEX_INIT(mutex_);	
	}

	virtual void uninitCond(void)
	{
		THREAD_SINGNAL_DELETE(cond_);
	}
	
	virtual void uninitMutex(void)
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
	
	void deleteFiniTask(TPTask* tpTask);
	
	/**发送条件信号*/
	int sendCondSignal(void)
	{
		return THREAD_SINGNAL_SET(cond_);
	}
	
	/**线程通知 等待条件信号*/
	bool onWaitCondSignal(void);

	/**获取本线程要处理的任务*/
	TPTask* getTask(void)const
	{
		return currTask_;
	}

	/**设置本线程要处理的任务*/
	void setTask(TPTask* tpt)
	{
		currTask_ = tpt;
	}

	int getState(void)const
	{
		return state_;
	}
	
	/**本线程要处理的任务已经处理完毕 我们决定删除这个废弃的任务*/
	void onTaskComplete(void);

#if KBE_PLATFORM == PLATFORM_WIN32
	static unsigned __stdcall threadFunc(void *arg)
#else	
	static void* threadFunc(void* arg)
#endif
	{
		TPThread * tptd = static_cast<TPThread*>(arg);
		bool isRun = true;
#if KBE_PLATFORM == PLATFORM_WIN32
		THREAD_TRY_EXECUTION;
#else			
		pthread_detach(pthread_self());
#endif
		while(isRun)
		{
			isRun = tptd->onWaitCondSignal();
			tptd->state_ = 1;
			TPTask * task = tptd->getTask();
			
			while(task)
			{
				task->process();									// 处理该任务
				TPTask * task1 = tptd->tryGetTask();				// 尝试继续从任务队列里取出一个繁忙的未处理的任务

				if(!task1)
				{
					tptd->onTaskComplete();
					break;
				}
				else
				{
					tptd->deleteFiniTask(task);
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
protected:
	THREAD_SINGNAL cond_;			// 线程信号量
	THREAD_MUTEX mutex_;			// 线程互诉体
	int threadWaitSecond_;			// 线程空闲状态超过这个秒数则线程退出， 小于0为永久线程 秒单位
	TPTask * currTask_;				// 该线程的当前执行的任务
	THREAD_ID tidp_;				// 本线程的ID
	ThreadPool* threadPool_;		// 线程池指针
	int state_;						// 线程状态 -1还未启动, 0睡眠， 1繁忙中
};


class ThreadPool : public Singleton<ThreadPool>
{
protected:
	/**创建一个线程池线程*/
	TPThread* createThread(int threadWaitSecond = 0);
public:		
	
	ThreadPool();
	virtual ~ThreadPool();
	
	virtual void onMainThreadTick();
	
	/**
		获取当前线程总数
	*/	
	unsigned int getCurrentThreadCount(void)const
	{ 
		return currentThreadCount_; 
	}
	
	/**
		获取当前空闲线程总数
	*/		
	unsigned int getCurrentFreeThreadCount(void)const
	{ 
		return currentFreeThreadCount_; 
	}
	
	/**
		创建线程池
		@param inewThreadCount			: 当系统繁忙时线程池会新增加这么多线程（临时）
		@param inormalMaxThreadCount	: 线程池会一直保持这么多个数的线程
		@param imaxThreadCount			: 线程池最多只能有这么多个线程
	*/
	bool createThreadPool(unsigned int inewThreadCount, 
	unsigned int inormalMaxThreadCount, unsigned int imaxThreadCount);

	/**
		移动一个线程到空闲列表
	*/
	bool moveThreadToFreeList(TPThread* tptd);
	
	/**
		移动一个线程到繁忙列表
	*/	
	bool moveThreadToBusyList(TPThread* tptd);
	
	/**
		添加一个已经完成的任务到列表
	*/	
	void addFiniTask(TPTask* tptask);
	
	/**
		删除一个挂起(超时)线程
	*/	
	bool removeHangThread(TPThread* tptd);
	
	/**
		向线程池添加一个任务
	*/		
	bool addTask(TPTask* tptask);
	
	/**
		线程数量是否到达最大个数
	*/
	bool isThreadCountMax(void)const
	{
		return currentThreadCount_ >= maxThreadCount_;	
	}
	
	/**
		线程池是否处于繁忙状态
		未处理任务是否非常多   说明线程很繁忙
	*/
	bool isBusy(void)const
	{
		return busyTaskList_.size() > 32;
	}	

	/**
		将某个任务保存到未处理列表
	*/
	void saveToBusyTaskList(TPTask* tptask);

	/**
		从未处理列表取出一个任务 并从列表中删除
	*/
	TPTask* popBusyTaskList(void);
	
	/** 
		线程池是否已经被初始化 
	*/
	bool isInitialize(void)const
	{ 
		return isInitialize_; 
	}
protected:
	bool isInitialize_;												// 线程池是否被初始化过
	
	std::queue<TPTask*> busyTaskList_;								// 系统处于繁忙时还未处理的任务列表
	std::vector<TPTask*> finiTaskList_;								// 已经完成的任务列表
	
	THREAD_MUTEX busyTaskList_mutex_;								// 处理busyTaskList_互斥锁
	THREAD_MUTEX threadStateList_mutex_;							// 处理busyThreadList_ and freeThreadList_互斥锁
	THREAD_MUTEX finiTaskList_mutex_;								// 处理finiTaskList互斥锁
	
	std::list<TPThread*> busyThreadList_;							// 繁忙的线程列表
	std::list<TPThread*> freeThreadList_;							// 闲置的线程列表
	std::list<TPThread*> allThreadList_;							// 所有的线程列表
	
	unsigned int maxThreadCount_;									// 最大线程总数
	unsigned int extraNewAddThreadCount_;							// 如果normalThreadCount_不足够使用则会新创建这么多线程
	unsigned int currentThreadCount_;								// 当前线程数
	unsigned int currentFreeThreadCount_;							// 当前闲置的线程数
	unsigned int normalThreadCount_;								// 标准状态下的线程总数 即：默认情况下一启动服务器就开启这么多线程
																	// 如果线程不足够，则会新创建一些线程， 最大能够到maxThreadNum.
};

}
}
#endif
