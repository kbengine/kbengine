// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#include "threadpool.h"

#ifndef CODE_INLINE
#include "threadpool.inl"
#endif

#include "helper/watcher.h"

namespace KBEngine
{ 

KBE_SINGLETON_INIT(KBEngine::Thread::ThreadPool);

namespace Thread
{

int ThreadPool::timeout = 300;

//-------------------------------------------------------------------------------------
std::thread::id TPThread::createThread(void)
{
	tidp_ = new std::thread(TPThread::threadFunc, this);
	return tidp_->get_id();
}

//-------------------------------------------------------------------------------------
bool TPThread::join(void)
{
	if (!tidp_->joinable())
	{
		ERROR_MSG(fmt::format("TPThread::join: can't join Thread({0:p})\n", (void*)this));
		return false;
	}
	tidp_->join();
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

	threadStateList_mutex_.lock();
	int i = 0;
	std::list<TPThread*>::iterator itr = busyThreadList_.begin();
	for(; itr != busyThreadList_.end(); ++itr)
	{
		ret += (fmt::format("{0:p}:({1}), ", (void*)(*itr), (*itr)->printWorkState()));
		i++;

		if(i > 1024)
			break;
	}

	threadStateList_mutex_.unlock();
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

	threadStateList_mutex_.lock();

	DEBUG_MSG(fmt::format("ThreadPool::destroy(): starting size {0}.\n",
		allThreadList_.size()));
	
	threadStateList_mutex_.unlock();

	int itry = 0;
	while(true)
	{
		KBEngine::sleep(300);
		itry++;

		std::string taskaddrs = "";
		threadStateList_mutex_.lock();

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

		threadStateList_mutex_.unlock();
		
		if(count <= 0)
		{
			break;
		}
		else
		{
			WARNING_MSG(fmt::format("ThreadPool::destroy(): waiting for Thread({0})[{1}], try={2}\n", 
				count, taskaddrs, itry));
		}
	}

	threadStateList_mutex_.lock();

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
	threadStateList_mutex_.unlock();

	finiTaskList_mutex_.lock();
	
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

	finiTaskList_mutex_.unlock();

	bufferedTaskList_mutex_.lock();

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
	
	bufferedTaskList_mutex_.unlock();

	DEBUG_MSG("ThreadPool::destroy(): successfully!\n");
}

//-------------------------------------------------------------------------------------
TPTask* ThreadPool::popbufferTask(void)
{
	TPTask* tptask = NULL;
	bufferedTaskList_mutex_.lock();

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

	bufferedTaskList_mutex_.unlock();

	return tptask;
}

//-------------------------------------------------------------------------------------
void ThreadPool::addFiniTask(TPTask* tptask)
{ 
	finiTaskList_mutex_.lock();
	finiTaskList_.push_back(tptask); 
	++finiTaskList_count_;
	finiTaskList_mutex_.unlock();

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

	finiTaskList_mutex_.lock();

	if(finiTaskList_.size() == 0)
	{
		finiTaskList_mutex_.unlock();
		return;
	}

	std::copy(finiTaskList_.begin(), finiTaskList_.end(), std::back_inserter(finitasks));   
	finiTaskList_.clear();
	finiTaskList_count_ = 0;
	finiTaskList_mutex_.unlock();

	std::vector<TPTask*>::iterator finiiter  = finitasks.begin();

	for(; finiiter != finitasks.end(); )
	{
		Thread::TPTask::TPTaskState state = (*finiiter)->presentMainThread();

		switch(state)
		{
		case Thread::TPTask::TPTASK_STATE_COMPLETED:
			delete (*finiiter);
			finiiter = finitasks.erase(finiiter);
			break;
			
		case Thread::TPTask::TPTASK_STATE_CONTINUE_CHILDTHREAD:
			this->addTask((*finiiter));
			finiiter = finitasks.erase(finiiter);
			break;
			
		case Thread::TPTask::TPTASK_STATE_CONTINUE_MAINTHREAD:
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
	bufferedTaskList_mutex_.lock();

	bufferedTaskList_.push(tptask);

	size_t size = bufferedTaskList_.size();
	if(size > THREAD_BUSY_SIZE)
	{
		WARNING_MSG(fmt::format("ThreadPool::bufferTask: task buffered({0})!\n", 
			size));
	}

	bufferedTaskList_mutex_.unlock();
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
	threadStateList_mutex_.lock();
	std::list<TPThread*>::iterator itr;
	itr = find(busyThreadList_.begin(), busyThreadList_.end(), tptd);

	if(itr != busyThreadList_.end())
	{
		busyThreadList_.erase(itr);
	}
	else
	{
		threadStateList_mutex_.unlock();

		ERROR_MSG(fmt::format("ThreadPool::addFreeThread: busyThreadList_ not found Thread.{0}\n", tptd->id()));
		
		delete tptd;
		return false;
	}
		
	freeThreadList_.push_back(tptd);
	currentFreeThreadCount_++;
	threadStateList_mutex_.unlock();
	return true;
}

//-------------------------------------------------------------------------------------
bool ThreadPool::addBusyThread(TPThread* tptd)
{
	threadStateList_mutex_.lock();
	std::list<TPThread*>::iterator itr;
	itr = find(freeThreadList_.begin(), freeThreadList_.end(), tptd);
	
	if(itr != freeThreadList_.end())
	{
		freeThreadList_.erase(itr);
	}
	else
	{
		threadStateList_mutex_.unlock();
		ERROR_MSG(fmt::format("ThreadPool::addBusyThread: freeThreadList_ not found Thread.{0}\n", tptd->id()));
		delete tptd;
		return false;
	}
		
	busyThreadList_.push_back(tptd);
	--currentFreeThreadCount_;
	threadStateList_mutex_.unlock();

	return true;
}

//-------------------------------------------------------------------------------------
bool ThreadPool::removeHangThread(TPThread* tptd)
{
	threadStateList_mutex_.lock();
	std::list<TPThread*>::iterator itr, itr1;
	itr = find(freeThreadList_.begin(), freeThreadList_.end(), tptd);
	itr1 = find(allThreadList_.begin(), allThreadList_.end(), tptd);
	
	if(itr != freeThreadList_.end() && itr1 != allThreadList_.end())
	{
		freeThreadList_.erase(itr);
		allThreadList_.erase(itr1);
		--currentThreadCount_;
		--currentFreeThreadCount_;

		INFO_MSG(fmt::format("ThreadPool::removeHangThread: Thread.{0} is destroy. "
			"currentFreeThreadCount:{1}, currentThreadCount:{2}\n",	tptd->id(), currentFreeThreadCount_, currentThreadCount_));
		
		SAFE_RELEASE(tptd);
	}
	else
	{
		threadStateList_mutex_.unlock();
		
		ERROR_MSG(fmt::format("ThreadPool::removeHangThread: not found Thread.{0}\n", tptd->id()));
		
		return false;
	}
	
	threadStateList_mutex_.unlock();
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

//#if KBE_PLATFORM == PLATFORM_WIN32
//	if (tptd->sendCondSignal() == 0) {
//#else
//	if (tptd->sendCondSignal() != 0) {
//#endif
//		ERROR_MSG("ThreadPool::addTask: pthread_cond_signal error!\n");
//		return false;
//	}
	tptd->sendCondSignal();
	return true;
}

//-------------------------------------------------------------------------------------
bool ThreadPool::addTask(TPTask* tptask)
{
	threadStateList_mutex_.lock();
	if(currentFreeThreadCount_ > 0)
	{
		bool ret = _addTask(tptask);
		threadStateList_mutex_.unlock();

		return ret;
	}
	
	bufferTask(tptask);
	
	if(isThreadCountMax())
	{
		threadStateList_mutex_.unlock();

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
			ERROR_MSG("ThreadPool::addTask: the ThreadPool create Thread error! ... \n");
#else
			ERROR_MSG(fmt::format("ThreadPool::addTask: the ThreadPool create Thread error:{0}\n", 
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

	threadStateList_mutex_.unlock();
	return true;
}

//-------------------------------------------------------------------------------------
bool ThreadPool::hasThread(TPThread* pTPThread)
{
	bool ret = true;

	threadStateList_mutex_.lock();

	std::list<TPThread*>::iterator itr1 = find(allThreadList_.begin(), allThreadList_.end(), pTPThread);
	if(itr1 == allThreadList_.end())
		ret = false;

	threadStateList_mutex_.unlock();

	return ret;
}

//-------------------------------------------------------------------------------------
void* TPThread::threadFunc(TPThread* tptd)
{
	if (!tptd)
	{
		return nullptr;
	}

	ThreadPool* pThreadPool = tptd->threadPool();

	bool isRun = true;
	tptd->reset_done_tasks();
	tptd->detach();
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
			WARNING_MSG(fmt::format("TPThread::threadFunc: task {0:p} not finish, Thread.{1:p} will exit.\n", 
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
	std::unique_lock<std::mutex> lk(mutex_);
	if(threadWaitSecond_ <= 0)
	{
		state_ = THREAD_STATE_SLEEP;
		cond_.wait(lk);
	}
	else
	{
		state_ = THREAD_STATE_SLEEP;
		auto const timeout = std::chrono::steady_clock::now() + std::chrono::microseconds(1000);
		auto ret = cond_.wait_until(lk, timeout);
		
		// 如果是因为超时了， 说明这个线程很久没有被用到， 我们应该注销这个线程。
		if (ret == std::cv_status::timeout)
		{
			// 通知ThreadPool注销自己
			threadPool_->removeHangThread(this);
			return false;
		}
		else if(ret != std::cv_status::no_timeout)
		{
			ERROR_MSG(fmt::format("TPThread::onWaitCondSignal: pthread_cond_wait error, {0}\n", kbe_strerror()));
		}
	}
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
