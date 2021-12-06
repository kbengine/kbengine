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
	
namespace KBEngine{ namespace Thread{

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
	}
		
	virtual ~TPThread()
	{
		DEBUG_MSG(fmt::format("TPThread::~TPThread(): {}\n", (void*)this));
	}
	
	virtual void onStart(){}
	virtual void onEnd(){}

	virtual void onProcessTaskStart(TPTask* pTask) {}
	virtual void processTask(TPTask* pTask){ pTask->process(); }
	virtual void onProcessTaskEnd(TPTask* pTask) {}

	INLINE uint64_t id(void) const
	{
		std::stringstream ss;
		ss << tidp_->get_id();
		uint64_t id = std::stoull(ss.str());
		return id;
	};
	
	/**
		����һ���̣߳� �����Լ�����̰߳�
	*/
	std::thread::id createThread(void);

	virtual TPTask* tryGetTask(void);
	
	/**
		���������ź�
	*/
	void sendCondSignal(void)
	{
		std::lock_guard<std::mutex> lk(mutex_);
		cond_.notify_all();
	}
	
	/**
		�߳�֪ͨ �ȴ������ź�
	*/
	bool onWaitCondSignal(void);
	
	bool join(void);

	void detach() { tidp_->detach(); };

	/**
		��ȡ���߳�Ҫ���������
	*/
	INLINE TPTask* task(void) const;

	/**
		���ñ��߳�Ҫ���������
	*/
	INLINE void task(TPTask* tpt);

	INLINE int state(void) const;
	
	/**
		���߳�Ҫ����������Ѿ�������� ���Ǿ���ɾ���������������
	*/
	void onTaskCompleted(void);

	static void* threadFunc(TPThread* tptd);

	/**
		���ñ��߳�Ҫ���������
	*/
	INLINE ThreadPool* threadPool();

	/**
		����̹߳���״̬
		��Ҫ�ṩ��watcherʹ��
	*/
	virtual std::string printWorkState()
	{
		char buf[128];
		mutex_.lock();
		sprintf(buf, "%p,%u", currTask_, done_tasks_);
		mutex_.unlock();
		return buf;
	}

	/**
		�߳�����һ����δ�ı䵽����״̬������ִ�е��������
	*/
	void reset_done_tasks(){ done_tasks_ = 0; }
	void inc_done_tasks(){ ++done_tasks_; }

protected:
	std::condition_variable cond_;			// �߳��ź���
	std::mutex mutex_;			// �̻߳�����
	std::thread* tidp_;
	int threadWaitSecond_;			// �߳̿���״̬��������������߳��˳�, С��0Ϊ�����߳�(�뵥λ)
	TPTask * currTask_;				// ���̵߳ĵ�ǰִ�е�����
	ThreadPool* threadPool_;		// �̳߳�ָ��
	THREAD_STATE state_;			// �߳�״̬: -1��δ����, 0˯��, 1��æ��
	uint32 done_tasks_;				// �߳�����һ����δ�ı䵽����״̬������ִ�е��������
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
		��ȡ��ǰ�̳߳������߳�״̬(�ṩ��watch��)
	*/
	std::string printThreadWorks();

	/**
		��ȡ��ǰ�߳�����
	*/	
	INLINE uint32 currentThreadCount(void) const;
	
	/**
		��ȡ��ǰ�����߳�����
	*/		
	INLINE uint32 currentFreeThreadCount(void) const;
	
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
	bool _addTask(TPTask* tptask);
	INLINE bool addBackgroundTask(TPTask* tptask){ return addTask(tptask); }
	INLINE bool pushTask(TPTask* tptask){ return addTask(tptask); }

	/**
		�߳������Ƿ񵽴�������
	*/
	INLINE bool isThreadCountMax(void) const;
	
	/**
		�̳߳��Ƿ��ڷ�æ״̬
		δ���������Ƿ�ǳ���   ˵���̺ܷ߳�æ
	*/
	INLINE bool isBusy(void) const;
	
	/** 
		�̳߳��Ƿ��Ѿ�����ʼ�� 
	*/
	INLINE bool isInitialize(void) const;

	/**
		�����Ƿ��Ѿ�����
	*/
	INLINE bool isDestroyed() const;

	/**
		�����Ƿ��Ѿ�����
	*/
	INLINE void destroy();

	/** 
		��û������������
	*/
	INLINE uint32 bufferTaskSize() const;

	/** 
		��û��������
	*/
	INLINE std::queue<Thread::TPTask*>& bufferedTaskList();

	/** 
		����Ѿ���ɵ���������
	*/
	INLINE uint32 finiTaskSize() const;

	virtual std::string name() const { return "ThreadPool"; }

public:
	static int timeout;

	/**
		����һ���̳߳��߳�
	*/
	virtual TPThread* createThread(int threadWaitSecond = ThreadPool::timeout, bool threadStartsImmediately = true);

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
	size_t finiTaskList_count_;

	std::mutex bufferedTaskList_mutex_;							// ����bufferTaskList������
	std::mutex threadStateList_mutex_;							// ����bufferTaskList and freeThreadList_������
	std::mutex finiTaskList_mutex_;								// ����finiTaskList������
	
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
#include "threadpool.inl"
#endif
#endif // KBE_THREADPOOL_H
