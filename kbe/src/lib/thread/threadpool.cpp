// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#include "threadpool.h"

#ifndef CODE_INLINE
#include "threadpool.inl"
#endif

#include "helper/watcher.h"

namespace KBEngine
{ 

KBE_SINGLETON_INIT(KBEngine::thread::ThreadPool);

namespace thread
{

int ThreadPool::timeout = 300;

//-------------------------------------------------------------------------------------
THREAD_ID TPThread::createThread(void)
{
#if KBE_PLATFORM == PLATFORM_WIN32
	tidp_ = (THREAD_ID)_beginthreadex(NULL, 0, 
		&TPThread::threadFunc, (void*)this, NULL, 0);
#else	
	if(pthread_create(&tidp_, NULL, TPThread::threadFunc, 
		(void*)this)!= 0)
	{
		ERROR_MSG("createThread error!");
	}
#endif
	return tidp_;
}

//-------------------------------------------------------------------------------------
bool TPThread::join(void)
{
#if KBE_PLATFORM == PLATFORM_WIN32
	int i = 0;

	while(true)
	{
		++i;
		DWORD dw = WaitForSingleObject(id(), 3000);  

		switch (dw)
		{
		case WAIT_OBJECT_0:
			return true;
		
		case WAIT_TIMEOUT:
			if(i > 20)
			{
				ERROR_MSG(fmt::format("TPThread::join: can't join thread({0:p})\n", (void*)this));
				return false;
			}
			else
			{
				WARNING_MSG(fmt::format("TPThread::join: waiting for thread({0:p}), try={1}\n", (void*)this, i));
			}
			break;
			
		case WAIT_FAILED:
		default:
			ERROR_MSG(fmt::format("TPThread::join: can't join thread({0:p})\n", (void*)this));
			return false;
		};
	}
#else
	void* status;
	if(pthread_join(id(), &status))
	{
		ERROR_MSG(fmt::format("TPThread::join: can't join thread({0:p})\n", (void*)this));
		return false;
	}
#endif

	return true;
}

//-------------------------------------------------------------------------------------
ThreadPool::ThreadPool():
isInitialize_(false),
bufferedTaskList_(),
finiTaskList_(),
finiTaskList_count_(0),
bufferedTaskList_mutex_(),
threadStateList_mutex_(),
finiTaskList_mutex_(),
busyThreadList_(),
freeThreadList_(),
allThreadList_(),
maxThreadCount_(0),
extraNewAddThreadCount_(0),
currentThreadCount_(0),
currentFreeThreadCount_(0),
normalThreadCount_(0),
isDestroyed_(false)
{		
	THREAD_MUTEX_INIT(threadStateList_mutex_);	
	THREAD_MUTEX_INIT(bufferedTaskList_mutex_);
	THREAD_MUTEX_INIT(finiTaskList_mutex_);
}

//-------------------------------------------------------------------------------------
ThreadPool::~ThreadPool()
{
	KBE_ASSERT(isDestroyed_ && allThreadList_.size() == 0);
}

//-------------------------------------------------------------------------------------
bool ThreadPool::initializeWatcher()
{
	WATCH_OBJECT((fmt::format("{}/maxThreadCount", name())).c_str(), this->maxThreadCount_);
	WATCH_OBJECT((fmt::format("{}/extraNewAddThreadCount", name())).c_str(), this->extraNewAddThreadCount_);
	WATCH_OBJECT((fmt::format("{}/currentFreeThreadCount", name())).c_str(), this->currentFreeThreadCount_);
	WATCH_OBJECT((fmt::format("{}/normalThreadCount", name())).c_str(), this->normalThreadCount_);
	WATCH_OBJECT((fmt::format("{}/bufferedTaskSize", name())).c_str(), this, &ThreadPool::bufferTaskSize);
	WATCH_OBJECT((fmt::format("{}/finiTaskSize", name()).c_str()), this, &ThreadPool::finiTaskSize);
	WATCH_OBJECT((fmt::format("{}/busyThreadStates", name())).c_str(), this, &ThreadPool::printThreadWorks);
	return true;
}

//-------------------------------------------------------------------------------------
std::string ThreadPool::printThreadWorks()
{
	std::string ret;

	THREAD_MUTEX_LOCK(threadStateList_mutex_);
	int i = 0;
	std::list<TPThread*>::iterator itr = busyThreadList_.begin();
	for(; itr != busyThreadList_.end(); ++itr)
	{
		ret += (fmt::format("{0:p}:({1}), ", (void*)(*itr), (*itr)->printWorkState()));
		i++;

		if(i > 1024)
			break;
	}

	THREAD_MUTEX_UNLOCK(threadStateList_mutex_);		
	return ret;
}

//-------------------------------------------------------------------------------------
void ThreadPool::finalise()
{
	destroy();
}

//-------------------------------------------------------------------------------------
void ThreadPool::destroy()
{
	isDestroyed_ = true;

	THREAD_MUTEX_LOCK(threadStateList_mutex_);

	DEBUG_MSG(fmt::format("ThreadPool::destroy(): starting size {0}.\n",
		allThreadList_.size()));
	
	THREAD_MUTEX_UNLOCK(threadStateList_mutex_);

	int itry = 0;
	while(true)
	{
		KBEngine::sleep(300);
		itry++;

		std::string taskaddrs = "";
		THREAD_MUTEX_LOCK(threadStateList_mutex_);

		int count = (int)allThreadList_.size();
		std::list<TPThread*>::iterator itr = allThreadList_.begin();
		for(; itr != allThreadList_.end(); ++itr)
		{
			if((*itr))
			{
				if((*itr)->state() != TPThread::THREAD_STATE_END)
				{
					(*itr)->sendCondSignal();
					taskaddrs += (fmt::format("{0:p},", (void*)(*itr)));
				}
				else
				{
					count--;
				}
			}
		}

		THREAD_MUTEX_UNLOCK(threadStateList_mutex_);
		
		if(count <= 0)
		{
			break;
		}
		else
		{
			WARNING_MSG(fmt::format("ThreadPool::destroy(): waiting for thread({0})[{1}], try={2}\n", 
				count, taskaddrs, itry));
		}
	}

	THREAD_MUTEX_LOCK(threadStateList_mutex_);

	KBEngine::sleep(100);

	std::list<TPThread*>::iterator itr = allThreadList_.begin();
	for(; itr != allThreadList_.end(); ++itr)
	{
		if((*itr))
		{
			delete (*itr);
			(*itr) = NULL;
		}
	}
	
	allThreadList_.clear();
	THREAD_MUTEX_UNLOCK(threadStateList_mutex_);

	THREAD_MUTEX_LOCK(finiTaskList_mutex_);
	
	if(finiTaskList_.size() > 0)
	{
		WARNING_MSG(fmt::format("ThreadPool::~ThreadPool(): Discarding {0} finished tasks.\n",
			finiTaskList_.size()));

		std::list<TPTask*>::iterator finiiter  = finiTaskList_.begin();
		for(; finiiter != finiTaskList_.end(); ++finiiter)
		{
			delete (*finiiter);
		}
	
		finiTaskList_.clear();
		finiTaskList_count_ = 0;
	}

	THREAD_MUTEX_UNLOCK(finiTaskList_mutex_);

	THREAD_MUTEX_LOCK(bufferedTaskList_mutex_);

	if(bufferedTaskList_.size() > 0)
	{
		WARNING_MSG(fmt::format("ThreadPool::~ThreadPool(): Discarding {0} buffered tasks.\n", 
			bufferedTaskList_.size()));

		while(bufferedTaskList_.size() > 0)
		{
			TPTask* tptask = bufferedTaskList_.front();
			bufferedTaskList_.pop();
			delete tptask;
		}
	}
	
	THREAD_MUTEX_UNLOCK(bufferedTaskList_mutex_);

	THREAD_MUTEX_DELETE(threadStateList_mutex_);
	THREAD_MUTEX_DELETE(bufferedTaskList_mutex_);
	THREAD_MUTEX_DELETE(finiTaskList_mutex_);

	DEBUG_MSG("ThreadPool::destroy(): successfully!\n");
}

//-------------------------------------------------------------------------------------
TPTask* ThreadPool::popbufferTask(void)
{
	TPTask* tptask = NULL;
	THREAD_MUTEX_LOCK(bufferedTaskList_mutex_);

	size_t size = bufferedTaskList_.size();
	if(size > 0)
	{
		tptask = bufferedTaskList_.front();
		bufferedTaskList_.pop();
	
		if(size > THREAD_BUSY_SIZE)
		{
			WARNING_MSG(fmt::format("ThreadPool::popbufferTask: task buffered({0})!\n", 
				size));
		}
	}

	THREAD_MUTEX_UNLOCK(bufferedTaskList_mutex_);	

	return tptask;
}

//-------------------------------------------------------------------------------------
void ThreadPool::addFiniTask(TPTask* tptask)
{ 
	THREAD_MUTEX_LOCK(finiTaskList_mutex_);
	finiTaskList_.push_back(tptask); 
	++finiTaskList_count_;
	THREAD_MUTEX_UNLOCK(finiTaskList_mutex_);	
}

//-------------------------------------------------------------------------------------
bool ThreadPool::createThreadPool(uint32 inewThreadCount, 
	uint32 inormalMaxThreadCount, uint32 imaxThreadCount)
{
	assert(!isInitialize_);
	INFO_MSG("ThreadPool::createThreadPool: creating  threadpool...\n");
	
	extraNewAddThreadCount_ = inewThreadCount;
	normalThreadCount_ = inormalMaxThreadCount;
	maxThreadCount_ = imaxThreadCount;
	
	for(uint32 i=0; i<normalThreadCount_; ++i)
	{
		TPThread* tptd = createThread(0);
		
		if(!tptd)
		{
			ERROR_MSG("ThreadPool::createThreadPool: error! \n");
			return false;
		}

		currentFreeThreadCount_++;	
		currentThreadCount_++;
		
		freeThreadList_.push_back(tptd);
		allThreadList_.push_back(tptd);
	}
	
	INFO_MSG(fmt::format("ThreadPool::createThreadPool: successfully({0}), "
		"newThreadCount={1}, normalMaxThreadCount={2}, maxThreadCount={3}\n",
			currentThreadCount_, extraNewAddThreadCount_, normalThreadCount_, maxThreadCount_));

	isInitialize_ = true;
	KBEngine::sleep(100);
	return true;
}

//-------------------------------------------------------------------------------------
void ThreadPool::onMainThreadTick()
{
	std::vector<TPTask*> finitasks;

	THREAD_MUTEX_LOCK(finiTaskList_mutex_);

	if(finiTaskList_.size() == 0)
	{
		THREAD_MUTEX_UNLOCK(finiTaskList_mutex_);	
		return;
	}

	std::copy(finiTaskList_.begin(), finiTaskList_.end(), std::back_inserter(finitasks));   
	finiTaskList_.clear();
	finiTaskList_count_ = 0;
	THREAD_MUTEX_UNLOCK(finiTaskList_mutex_);	

	std::vector<TPTask*>::iterator finiiter  = finitasks.begin();

	for(; finiiter != finitasks.end(); )
	{
		thread::TPTask::TPTaskState state = (*finiiter)->presentMainThread();

		switch(state)
		{
		case thread::TPTask::TPTASK_STATE_COMPLETED:
			delete (*finiiter);
			finiiter = finitasks.erase(finiiter);
			break;
			
		case thread::TPTask::TPTASK_STATE_CONTINUE_CHILDTHREAD:
			this->addTask((*finiiter));
			finiiter = finitasks.erase(finiiter);
			break;
			
		case thread::TPTask::TPTASK_STATE_CONTINUE_MAINTHREAD:
			addFiniTask((*finiiter));
			++finiiter;
			break;
			
		default:
			KBE_ASSERT(false);
			break;
		};
	}
}

//-------------------------------------------------------------------------------------
void ThreadPool::bufferTask(TPTask* tptask)
{
	THREAD_MUTEX_LOCK(bufferedTaskList_mutex_);

	bufferedTaskList_.push(tptask);

	size_t size = bufferedTaskList_.size();
	if(size > THREAD_BUSY_SIZE)
	{
		WARNING_MSG(fmt::format("ThreadPool::bufferTask: task buffered({0})!\n", 
			size));
	}

	THREAD_MUTEX_UNLOCK(bufferedTaskList_mutex_);
}

//-------------------------------------------------------------------------------------
TPThread* ThreadPool::createThread(int threadWaitSecond, bool threadStartsImmediately)
{
	TPThread* tptd = new TPThread(this, threadWaitSecond);

	if (threadStartsImmediately)
		tptd->createThread();

	return tptd;
}	

//-------------------------------------------------------------------------------------
bool ThreadPool::addFreeThread(TPThread* tptd)
{
	THREAD_MUTEX_LOCK(threadStateList_mutex_);
	std::list<TPThread*>::iterator itr;
	itr = find(busyThreadList_.begin(), busyThreadList_.end(), tptd);

	if(itr != busyThreadList_.end())
	{
		busyThreadList_.erase(itr);
	}
	else
	{
		THREAD_MUTEX_UNLOCK(threadStateList_mutex_);

		ERROR_MSG(fmt::format("ThreadPool::addFreeThread: busyThreadList_ not found thread.{0}\n",
		 (uint32)tptd->id()));
		
		delete tptd;
		return false;
	}
		
	freeThreadList_.push_back(tptd);
	currentFreeThreadCount_++;
	THREAD_MUTEX_UNLOCK(threadStateList_mutex_);
	return true;
}

//-------------------------------------------------------------------------------------
bool ThreadPool::addBusyThread(TPThread* tptd)
{
	THREAD_MUTEX_LOCK(threadStateList_mutex_);
	std::list<TPThread*>::iterator itr;
	itr = find(freeThreadList_.begin(), freeThreadList_.end(), tptd);
	
	if(itr != freeThreadList_.end())
	{
		freeThreadList_.erase(itr);
	}
	else
	{
		THREAD_MUTEX_UNLOCK(threadStateList_mutex_);
		ERROR_MSG(fmt::format("ThreadPool::addBusyThread: freeThreadList_ not "
				"found thread.{0}\n",
					(uint32)tptd->id()));
		
		delete tptd;
		return false;
	}
		
	busyThreadList_.push_back(tptd);
	--currentFreeThreadCount_;
	THREAD_MUTEX_UNLOCK(threadStateList_mutex_);		

	return true;
}

//-------------------------------------------------------------------------------------
bool ThreadPool::removeHangThread(TPThread* tptd)
{
	THREAD_MUTEX_LOCK(threadStateList_mutex_);
	std::list<TPThread*>::iterator itr, itr1;
	itr = find(freeThreadList_.begin(), freeThreadList_.end(), tptd);
	itr1 = find(allThreadList_.begin(), allThreadList_.end(), tptd);
	
	if(itr != freeThreadList_.end() && itr1 != allThreadList_.end())
	{
		freeThreadList_.erase(itr);
		allThreadList_.erase(itr1);
		--currentThreadCount_;
		--currentFreeThreadCount_;

		INFO_MSG(fmt::format("ThreadPool::removeHangThread: thread.{0} is destroy. "
			"currentFreeThreadCount:{1}, currentThreadCount:{2}\n",
		(uint32)tptd->id(), currentFreeThreadCount_, currentThreadCount_));
		
		SAFE_RELEASE(tptd);
	}
	else
	{
		THREAD_MUTEX_UNLOCK(threadStateList_mutex_);		
		
		ERROR_MSG(fmt::format("ThreadPool::removeHangThread: not found thread.{0}\n", 
			(uint32)tptd->id()));
		
		return false;
	}
	
	THREAD_MUTEX_UNLOCK(threadStateList_mutex_);
	return true;		
}

//-------------------------------------------------------------------------------------
bool ThreadPool::_addTask(TPTask* tptask)
{
	std::list<TPThread*>::iterator itr = freeThreadList_.begin();
	TPThread* tptd = (TPThread*)(*itr);
	freeThreadList_.erase(itr);
	busyThreadList_.push_back(tptd);
	--currentFreeThreadCount_;

	//INFO_MSG("ThreadPool::currFree:%d, currThreadCount:%d, busy:[%d]\n",
	//		 currentFreeThreadCount_, currentThreadCount_, busyThreadList_count_);

	tptd->task(tptask);

#if KBE_PLATFORM == PLATFORM_WIN32
	if (tptd->sendCondSignal() == 0) {
#else
	if (tptd->sendCondSignal() != 0) {
#endif
		ERROR_MSG("ThreadPool::addTask: pthread_cond_signal error!\n");
		return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool ThreadPool::addTask(TPTask* tptask)
{
	THREAD_MUTEX_LOCK(threadStateList_mutex_);
	if(currentFreeThreadCount_ > 0)
	{
		bool ret = _addTask(tptask);
		THREAD_MUTEX_UNLOCK(threadStateList_mutex_);

		return ret;
	}
	
	bufferTask(tptask);
	
	if(isThreadCountMax())
	{
		THREAD_MUTEX_UNLOCK(threadStateList_mutex_);

		//WARNING_MSG(fmt::format("ThreadPool::addTask: can't createthread, the poolsize is full({}).\n,
		//	maxThreadCount_));

		return false;
	}

	for(uint32 i=0; i<extraNewAddThreadCount_; ++i)
	{
		bool threadStartsImmediately = i > 0;

		// 设定5分钟未使用则退出的线程
		TPThread* tptd = createThread(ThreadPool::timeout, threadStartsImmediately);
		if(!tptd)
		{
#if KBE_PLATFORM == PLATFORM_WIN32		
			ERROR_MSG("ThreadPool::addTask: the ThreadPool create thread error! ... \n");
#else
			ERROR_MSG(fmt::format("ThreadPool::addTask: the ThreadPool create thread error:{0}\n", 
				kbe_strerror()));
#endif				
		}
		
		// 所有的线程列表
		allThreadList_.push_back(tptd);	
		
		if (threadStartsImmediately)
		{
			// 闲置的线程列表
			freeThreadList_.push_back(tptd);
			++currentFreeThreadCount_;
		}
		else
		{
			TPTask * pTask = tptd->tryGetTask();
			if (pTask)
			{
				busyThreadList_.push_back(tptd);
				tptd->task(pTask);
			}
			else
			{
				freeThreadList_.push_back(tptd);
				++currentFreeThreadCount_;
			}

			tptd->createThread();
		}

		++currentThreadCount_;
		
		
	}
	
	INFO_MSG(fmt::format("ThreadPool::addTask: new Thread, currThreadCount: {0}\n", 
		currentThreadCount_));

	THREAD_MUTEX_UNLOCK(threadStateList_mutex_);
	return true;
}

//-------------------------------------------------------------------------------------
bool ThreadPool::hasThread(TPThread* pTPThread)
{
	bool ret = true;

	THREAD_MUTEX_LOCK(threadStateList_mutex_);

	std::list<TPThread*>::iterator itr1 = find(allThreadList_.begin(), allThreadList_.end(), pTPThread);
	if(itr1 == allThreadList_.end())
		ret = false;

	THREAD_MUTEX_UNLOCK(threadStateList_mutex_);

	return ret;
}

//-------------------------------------------------------------------------------------
#if KBE_PLATFORM == PLATFORM_WIN32
unsigned __stdcall TPThread::threadFunc(void *arg)
#else	
void* TPThread::threadFunc(void* arg)
#endif
{
	TPThread * tptd = static_cast<TPThread*>(arg);
	ThreadPool* pThreadPool = tptd->threadPool();

	bool isRun = true;
	tptd->reset_done_tasks();

#if KBE_PLATFORM == PLATFORM_WIN32
#else			
	pthread_detach(pthread_self());
#endif

	tptd->onStart();

	while(isRun)
	{
		if(tptd->task() != NULL)
		{
			isRun = true;
		}
		else
		{
			tptd->reset_done_tasks();
			isRun = tptd->onWaitCondSignal();
		}

		if(!isRun || pThreadPool->isDestroyed())
		{
			if(!pThreadPool->hasThread(tptd))
				tptd = NULL;

			goto __THREAD_END__;
		}

		TPTask * task = tptd->task();
		if(task == NULL)
			continue;

		tptd->state_ = THREAD_STATE_BUSY;

		while(task && !tptd->threadPool()->isDestroyed())
		{
			tptd->inc_done_tasks();
			tptd->onProcessTaskStart(task);
			tptd->processTask(task);
			tptd->onProcessTaskEnd(task);
			
			// 尝试继续从任务队列里取出一个繁忙的未处理的任务
			TPTask * task1 = tptd->tryGetTask();

			if(!task1)
			{
				tptd->state_ = THREAD_STATE_PENDING;
				tptd->onTaskCompleted();
				break;
			}
			else
			{
				pThreadPool->addFiniTask(task);
				task = task1;
				tptd->task(task1);
			}
		}
	}

__THREAD_END__:
	if(tptd)
	{
		TPTask * task = tptd->task();
		if(task)
		{
			WARNING_MSG(fmt::format("TPThread::threadFunc: task {0:p} not finish, thread.{1:p} will exit.\n", 
				(void*)task, (void*)tptd));

			delete task;
		}

		tptd->onEnd();
		tptd->state_ = THREAD_STATE_END;
		tptd->reset_done_tasks();
	}

#if KBE_PLATFORM == PLATFORM_WIN32
	return 0;
#else	
	pthread_exit(NULL);
	return NULL;
#endif		
}

//-------------------------------------------------------------------------------------
bool TPThread::onWaitCondSignal(void)
{
#if KBE_PLATFORM == PLATFORM_WIN32
	if(threadWaitSecond_ <= 0)
	{
		state_ = THREAD_STATE_SLEEP;
		WaitForSingleObject(cond_, INFINITE); 
		ResetEvent(cond_);
	}
	else
	{
		state_ = THREAD_STATE_SLEEP;
		DWORD ret = WaitForSingleObject(cond_, threadWaitSecond_ * 1000);
		ResetEvent(cond_);

		// 如果是因为超时了， 说明这个线程很久没有被用到， 我们应该注销这个线程。
		// 通知ThreadPool注销自己
		if (ret == WAIT_TIMEOUT)
		{
			threadPool_->removeHangThread(this);
			return false;
		}
		else if(ret != WAIT_OBJECT_0)
		{
			ERROR_MSG(fmt::format("TPThread::onWaitCondSignal: WaitForSingleObject error, ret={0}\n", 
				ret));
		}
	}	
#else		
	if(threadWaitSecond_ <= 0)
	{
		lock();
		state_ = THREAD_STATE_SLEEP;
		pthread_cond_wait(&cond_, &mutex_);
		unlock();
	}
	else
	{
		struct timeval now;
		struct timespec timeout;			
		gettimeofday(&now, NULL);
		timeout.tv_sec = now.tv_sec + threadWaitSecond_;
		timeout.tv_nsec = now.tv_usec * 1000;
		
		lock();
		state_ = THREAD_STATE_SLEEP;
		int ret = pthread_cond_timedwait(&cond_, &mutex_, &timeout);
		unlock();
		
		// 如果是因为超时了， 说明这个线程很久没有被用到， 我们应该注销这个线程。
		if (ret == ETIMEDOUT)
		{
			// 通知ThreadPool注销自己
			threadPool_->removeHangThread(this);
			return false;
		}
		else if(ret != 0)
		{
			ERROR_MSG(fmt::format("TPThread::onWaitCondSignal: pthread_cond_wait error, {0}\n", 
				kbe_strerror()));
		}
	}
#endif
	return true;
}

//-------------------------------------------------------------------------------------
void TPThread::onTaskCompleted(void)
{
	threadPool_->addFiniTask(currTask_);
	currTask_ = NULL;
	threadPool_->addFreeThread(this);
}

//-------------------------------------------------------------------------------------
TPTask* TPThread::tryGetTask(void)
{
	return threadPool_->popbufferTask();
}

//-------------------------------------------------------------------------------------
}
}
