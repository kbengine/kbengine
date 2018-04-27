// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#ifndef KBE_THREADPOOL_H
#define KBE_THREADPOOL_H

#include "common/common.h"
#include "common/tasks.h"
#include "helper/debug_helper.h"
#include "thread/threadtask.h"
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#include <windows.h>          // for HANDLE
#include <process.h>          // for _beginthread()	
#include "helper/crashhandler.h"
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

// 线程池活动线程大于这个数目则处于繁忙状态
#define THREAD_BUSY_SIZE 32

/*
	线程池的线程基类
*/
class ThreadPool;
class TPThread
{
public:
	friend class ThreadPool;

	// 线程状态 -1还未启动, 0睡眠， 1繁忙中
	enum THREAD_STATE
	{
		THREAD_STATE_STOP = -1,
		THREAD_STATE_SLEEP = 0,
		THREAD_STATE_BUSY = 1,
		THREAD_STATE_END = 2,
		THREAD_STATE_PENDING = 3
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

		DEBUG_MSG(fmt::format("TPThread::~TPThread(): {}\n", (void*)this));
	}
	
	virtual void onStart(){}
	virtual void onEnd(){}

	virtual void onProcessTaskStart(TPTask* pTask) {}
	virtual void processTask(TPTask* pTask){ pTask->process(); }
	virtual void onProcessTaskEnd(TPTask* pTask) {}

	INLINE THREAD_ID id(void) const;
	
	INLINE void id(THREAD_ID tidp);
	
	/**
		创建一个线程， 并将自己与该线程绑定
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
		发送条件信号
	*/
	int sendCondSignal(void)
	{
#if KBE_PLATFORM == PLATFORM_WIN32
		return THREAD_SINGNAL_SET(cond_);
#else
REATTEMPT:

		lock();

		if (state_ == THREAD_STATE_PENDING)
		{       
			unlock();
			goto REATTEMPT;
		}

		int ret = THREAD_SINGNAL_SET(cond_);
		unlock();
		return ret;
#endif
	}
	
	/**
		线程通知 等待条件信号
	*/
	bool onWaitCondSignal(void);
	
	bool join(void);

	/**
		获取本线程要处理的任务
	*/
	INLINE TPTask* task(void) const;

	/**
		设置本线程要处理的任务
	*/
	INLINE void task(TPTask* tpt);

	INLINE int state(void) const;
	
	/**
		本线程要处理的任务已经处理完毕 我们决定删除这个废弃的任务
	*/
	void onTaskCompleted(void);

#if KBE_PLATFORM == PLATFORM_WIN32
	static unsigned __stdcall threadFunc(void *arg);
#else	
	static void* threadFunc(void* arg);
#endif

	/**
		设置本线程要处理的任务
	*/
	INLINE ThreadPool* threadPool();

	/**
		输出线程工作状态
		主要提供给watcher使用
	*/
	virtual std::string printWorkState()
	{
		char buf[128];
		lock();
		sprintf(buf, "%p,%u", currTask_, done_tasks_);
		unlock();
		return buf;
	}

	/**
		线程启动一次在未改变到闲置状态下连续执行的任务计数
	*/
	void reset_done_tasks(){ done_tasks_ = 0; }
	void inc_done_tasks(){ ++done_tasks_; }

protected:
	THREAD_SINGNAL cond_;			// 线程信号量
	THREAD_MUTEX mutex_;			// 线程互诉体
	int threadWaitSecond_;			// 线程空闲状态超过这个秒数则线程退出, 小于0为永久线程(秒单位)
	TPTask * currTask_;				// 该线程的当前执行的任务
	THREAD_ID tidp_;				// 本线程的ID
	ThreadPool* threadPool_;		// 线程池指针
	THREAD_STATE state_;			// 线程状态: -1还未启动, 0睡眠, 1繁忙中
	uint32 done_tasks_;				// 线程启动一次在未改变到闲置状态下连续执行的任务计数
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
		获取当前线程池所有线程状态(提供给watch用)
	*/
	std::string printThreadWorks();

	/**
		获取当前线程总数
	*/	
	INLINE uint32 currentThreadCount(void) const;
	
	/**
		获取当前空闲线程总数
	*/		
	INLINE uint32 currentFreeThreadCount(void) const;
	
	/**
		创建线程池
		@param inewThreadCount			: 当系统繁忙时线程池会新增加这么多线程（临时）
		@param inormalMaxThreadCount	: 线程池会一直保持这么多个数的线程
		@param imaxThreadCount			: 线程池最多只能有这么多个线程
	*/
	bool createThreadPool(uint32 inewThreadCount, 
			uint32 inormalMaxThreadCount, uint32 imaxThreadCount);
	
	/**
		向线程池添加一个任务
	*/		
	bool addTask(TPTask* tptask);
	bool _addTask(TPTask* tptask);
	INLINE bool addBackgroundTask(TPTask* tptask){ return addTask(tptask); }
	INLINE bool pushTask(TPTask* tptask){ return addTask(tptask); }

	/**
		线程数量是否到达最大个数
	*/
	INLINE bool isThreadCountMax(void) const;
	
	/**
		线程池是否处于繁忙状态
		未处理任务是否非常多   说明线程很繁忙
	*/
	INLINE bool isBusy(void) const;
	
	/** 
		线程池是否已经被初始化 
	*/
	INLINE bool isInitialize(void) const;

	/**
		返回是否已经销毁
	*/
	INLINE bool isDestroyed() const;

	/**
		返回是否已经销毁
	*/
	INLINE void destroy();

	/** 
		获得缓存的任务数量
	*/
	INLINE uint32 bufferTaskSize() const;

	/** 
		获得缓存的任务
	*/
	INLINE std::queue<thread::TPTask*>& bufferedTaskList();

	/** 
		操作缓存的任务锁
	*/
	INLINE void lockBufferedTaskList();
	INLINE void unlockBufferedTaskList();

	/** 
		获得已经完成的任务数量
	*/
	INLINE uint32 finiTaskSize() const;

	virtual std::string name() const { return "ThreadPool"; }

public:
	static int timeout;

	/**
		创建一个线程池线程
	*/
	virtual TPThread* createThread(int threadWaitSecond = ThreadPool::timeout, bool threadStartsImmediately = true);

	/**
		将某个任务保存到未处理列表
	*/
	void bufferTask(TPTask* tptask);

	/**
		从未处理列表取出一个任务 并从列表中删除
	*/
	TPTask* popbufferTask(void);

	/**
		移动一个线程到空闲列表
	*/
	bool addFreeThread(TPThread* tptd);
	
	/**
		移动一个线程到繁忙列表
	*/	
	bool addBusyThread(TPThread* tptd);
	
	/**
		添加一个已经完成的任务到列表
	*/	
	void addFiniTask(TPTask* tptask);
	
	/**
		删除一个挂起(超时)线程
	*/	
	bool removeHangThread(TPThread* tptd);

	bool initializeWatcher();

protected:
	bool isInitialize_;												// 线程池是否被初始化过
	
	std::queue<TPTask*> bufferedTaskList_;							// 系统处于繁忙时还未处理的任务列表
	std::list<TPTask*> finiTaskList_;								// 已经完成的任务列表
	size_t finiTaskList_count_;

	THREAD_MUTEX bufferedTaskList_mutex_;							// 处理bufferTaskList互斥锁
	THREAD_MUTEX threadStateList_mutex_;							// 处理bufferTaskList and freeThreadList_互斥锁
	THREAD_MUTEX finiTaskList_mutex_;								// 处理finiTaskList互斥锁
	
	std::list<TPThread*> busyThreadList_;							// 繁忙的线程列表
	std::list<TPThread*> freeThreadList_;							// 闲置的线程列表
	std::list<TPThread*> allThreadList_;							// 所有的线程列表

	uint32 maxThreadCount_;											// 最大线程总数
	uint32 extraNewAddThreadCount_;									// 如果normalThreadCount_不足够使用则会新创建这么多线程
	uint32 currentThreadCount_;										// 当前线程数
	uint32 currentFreeThreadCount_;									// 当前闲置的线程数
	uint32 normalThreadCount_;										// 标准状态下的线程总数 即：默认情况下一启动服务器就开启这么多线程
																	// 如果线程不足够，则会新创建一些线程， 最大能够到maxThreadNum.

	bool isDestroyed_;
};

}


}

#ifdef CODE_INLINE
#include "threadpool.inl"
#endif
#endif // KBE_THREADPOOL_H
