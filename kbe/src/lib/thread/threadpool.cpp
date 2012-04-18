#include "threadPool.hpp"
namespace KBEngine{ 

template<> KBEngine::thread::ThreadPool* Singleton<KBEngine::thread::ThreadPool>::singleton_ = 0;
namespace thread{

// 全局数据的定义
ThreadPool g_stp;

//-------------------------------------------------------------------------------------
THREAD_ID TPThread::createThread(void)
{
#if KBE_PLATFORM == PLATFORM_WIN32
	tidp_ = (THREAD_ID)_beginthreadex(NULL, 0, &TPThread::threadFunc, (void*)this, NULL, 0);
#else	
	assert(pthread_create(&tidp_, NULL, TPThread::threadFunc, (void*)this)== 0 && "createThread is error!");
#endif
	return tidp_;
}
	
//-------------------------------------------------------------------------------------
Task* ThreadPool::popBusyTaskList(void)
{
	Task* tptask = NULL;
	THREAD_MUTEX_LOCK(busyTaskList_mutex_);

	if(busyTaskList_.size()> 0){
		tptask = busyTaskList_.front();
		busyTaskList_.pop();
	}

	THREAD_MUTEX_UNLOCK(busyTaskList_mutex_);	
	return tptask;
}

//-------------------------------------------------------------------------------------
bool ThreadPool::createThreadPool(unsigned int inewThreadCount, unsigned int inormalMaxThreadCount, unsigned int imaxThreadCount)
{
	assert(!isInitialize_ && "ThreadPool is exist initialize!");
	INFO_MSG("ThreadPool::start to create the pool of Thread...\n");
	
	extraNewAddThreadCount_ = inewThreadCount;
	normalThreadCount_ = inormalMaxThreadCount;
	maxThreadCount_ = imaxThreadCount;
	
	for(unsigned int i=0; i<normalThreadCount_; i++)
	{
		TPThread* tptd = createThread();
		if(!tptd)
			ERROR_MSG("ThreadPool:: create Thread error! \n");
		
		currentFreeThreadCount_++;	
		currentThreadCount_++;
		freeThreadList_.push_back(tptd);										// 闲置的线程列表
		allThreadList_.push_back(tptd);										// 所有的线程列表
	}
	
	INFO_MSG("ThreadPool::createThreadPool is successfully(%d), newThreadCount=%d, normalMaxThreadCount=%d, maxThreadCount=%d\n", \
			currentThreadCount_, extraNewAddThreadCount_, normalThreadCount_, maxThreadCount_);

	isInitialize_ = true;
	return true;
}

//-------------------------------------------------------------------------------------
void ThreadPool::saveToBusyTaskList(Task* tptask)
{
	THREAD_MUTEX_LOCK(busyTaskList_mutex_);
	busyTaskList_.push(tptask);
	THREAD_MUTEX_UNLOCK(busyTaskList_mutex_);
	WARNING_MSG("ThreadPool::save BusyTask to list!\n");
}

//-------------------------------------------------------------------------------------
TPThread* ThreadPool::createThread(int threadWaitSecond)
{
	TPThread* tptd = new TPThread(this, threadWaitSecond);
	tptd->createThread();
	return tptd;
}	

//-------------------------------------------------------------------------------------
bool ThreadPool::moveThreadToFreeList(TPThread* tptd)
{
	THREAD_MUTEX_LOCK(threadStateList_mutex_);
	std::list<TPThread*>::iterator itr;
	itr = find(busyThreadList_.begin(), busyThreadList_.end(), tptd);

	if(itr != busyThreadList_.end()){
		busyThreadList_.erase(itr);
	}
	else
	{
		THREAD_MUTEX_UNLOCK(threadStateList_mutex_);
		ERROR_MSG("ThreadPool::moveThreadToFreeList: busyThreadList_ not found thread.%u\n", (unsigned int)tptd->getID());
		return false;
	}
		
	freeThreadList_.push_back(tptd);
	currentFreeThreadCount_++;
	THREAD_MUTEX_UNLOCK(threadStateList_mutex_);
	return true;
}

//-------------------------------------------------------------------------------------
bool ThreadPool::moveThreadToBusyList(TPThread* tptd)
{
	THREAD_MUTEX_LOCK(threadStateList_mutex_);
	std::list<TPThread*>::iterator itr;
	itr = find(freeThreadList_.begin(), freeThreadList_.end(), tptd);
	if(itr != freeThreadList_.end()){
		freeThreadList_.erase(itr);
	}
	else
	{
		THREAD_MUTEX_UNLOCK(threadStateList_mutex_);
		ERROR_MSG("ThreadPool::moveThreadToBusyList: freeThreadList_ not found thread.%u\n", (unsigned int)tptd->getID());
		return false;
	}
		
	busyThreadList_.push_back(tptd);
	currentFreeThreadCount_--;
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
	
	if(itr != freeThreadList_.end()&& itr1 != allThreadList_.end()){
		freeThreadList_.erase(itr);
		allThreadList_.erase(itr1);
		currentThreadCount_--;
		currentFreeThreadCount_--;
		INFO_MSG("ThreadPool::removeHangThread: thread.%u is destroy. currentFreeThreadCount:%d, currentThreadCount:%d\n", \
		(unsigned int)tptd->getID(), currentFreeThreadCount_, currentThreadCount_);
		SAFE_RELEASE(tptd);
	}
	else
	{
		THREAD_MUTEX_UNLOCK(threadStateList_mutex_);		
		ERROR_MSG("ThreadPool::removeHangThread: not found thread.%u\n", (unsigned int)tptd->getID());
		return false;
	}
	
	THREAD_MUTEX_UNLOCK(threadStateList_mutex_);		
	return true;		
}

//-------------------------------------------------------------------------------------
bool ThreadPool::addTask(Task* tptask)
{
	THREAD_MUTEX_LOCK(threadStateList_mutex_);
	if(currentFreeThreadCount_ > 0){
		std::list<TPThread*>::iterator itr = freeThreadList_.begin();
		TPThread* tptd = (TPThread*)(*itr);
		freeThreadList_.erase(itr);
		busyThreadList_.push_back(tptd);
		currentFreeThreadCount_--;
		INFO_MSG("ThreadPool::currFree:%d, currThreadCount:%d, busy:[%d]\n", currentFreeThreadCount_, currentThreadCount_, busyThreadList_.size());
		THREAD_MUTEX_UNLOCK(threadStateList_mutex_);
		
		tptd->setTask(tptask);												// 给线程设置新任务	
		
#if KBE_PLATFORM == PLATFORM_WIN32
		if(tptd->sendCondSignal()== 0){
#else
		if(tptd->sendCondSignal()!= 0){
#endif
			ERROR_MSG("ThreadPool:pthread_cond_signal is error!\n");
			return false;
		}
		
		return true;
	}
	
	saveToBusyTaskList(tptask);
	
	if(isThreadCountMax()){
		THREAD_MUTEX_UNLOCK(threadStateList_mutex_);
		WARNING_MSG("thread create is failed, count is full(%d).\n", maxThreadCount_);
		return false;
	}

	for(unsigned int i=0; i<extraNewAddThreadCount_; i++)
	{
		TPThread* tptd = createThread(300);									// 设定5分钟未使用则退出的线程
		if(!tptd){
#if KBE_PLATFORM == PLATFORM_WIN32		
			ERROR_MSG(">>ThreadPool create new Thread error! ... \n");
#else
			ERROR_MSG(">>ThreadPool create new Thread error:%s ... \n", strerror(errno));
#endif				
		}

		allThreadList_.push_back(tptd);									// 所有的线程列表
		freeThreadList_.push_back(tptd);									// 闲置的线程列表
		currentThreadCount_++;
		currentFreeThreadCount_++;	
		
	}
	
	INFO_MSG(">>ThreadPool:createNewThread-> currThreadCount: %d\n", currentThreadCount_);
	THREAD_MUTEX_UNLOCK(threadStateList_mutex_);
	return true;
}

//-------------------------------------------------------------------------------------
bool TPThread::onWaitCondSignal(void)
{
#if KBE_PLATFORM == PLATFORM_WIN32
	if(threadWaitSecond_ <= 0){
		state_ = 0;
		WaitForSingleObject(cond_, INFINITE); 
		ResetEvent(cond_);
	}
	else
	{
		state_ = 0;
		DWORD ret = WaitForSingleObject(cond_, threadWaitSecond_ * 1000);
		ResetEvent(cond_);

		if (ret == WAIT_TIMEOUT){											// 如果是因为超时了， 说明这个线程很久没有被用到， 我们应该注销这个线程。
			threadPool_->removeHangThread(this);							// 通知ThreadPool注销自己
			return false;
		}
		else if(ret != WAIT_OBJECT_0){
			ERROR_MSG("WaitForSingleObject is error, ret=%d\n", ret);
		}
	}	
#else		
	if(threadWaitSecond_ <= 0){
		lock();
		state_ = 0;
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
		state_ = 0;
		int ret = pthread_cond_timedwait(&cond_, &mutex_, &timeout);
		unlock();
		
		// 如果是因为超时了， 说明这个线程很久没有被用到， 我们应该注销这个线程。
		if (ret == ETIMEDOUT){
			// 通知ThreadPool注销自己
			threadPool_->removeHangThread(this);
			return false;
		}
		else if(ret != 0){
			ERROR_MSG("pthread_cond_timedwait is error, %s\n", strerror(errno));
		}
	}
#endif
	return true;
}

//-------------------------------------------------------------------------------------
void TPThread::onTaskComplete(void)
{
	SAFE_RELEASE(currTask_);
	threadPool_->moveThreadToFreeList(this);
}

//-------------------------------------------------------------------------------------
Task* TPThread::tryGetTask(void)
{
	return threadPool_->popBusyTaskList();
}

//-------------------------------------------------------------------------------------
}
}
