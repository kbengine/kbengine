#include "threadPool.hpp"
namespace KBEngine{ 

template<> KBEngine::thread::ThreadPool* Singleton<KBEngine::thread::ThreadPool>::m_singleton_ = 0;
namespace thread{

// 全局数据的定义
ThreadPool g_stp;

//-------------------------------------------------------------------------------------
THREAD_ID TPThread::createThread(void)
{
#if KBE_PLATFORM == PLATFORM_WIN32
	m_tidp = (THREAD_ID)_beginthreadex(NULL, 0, &TPThread::threadFunc, (void*)this, NULL, 0);
#else	
	assert(pthread_create(&m_tidp, NULL, TPThread::threadFunc, (void*)this)== 0 && "createThread is error!");
#endif
	return m_tidp;
}
	
//-------------------------------------------------------------------------------------
TPTask* ThreadPool::popBusyTaskList(void)
{
	TPTask* tptask = NULL;
	THREAD_MUTEX_LOCK(busyTaskList_mutex);

	if(m_busyTaskList.size()> 0){
		tptask = m_busyTaskList.front();
		m_busyTaskList.pop();
	}

	THREAD_MUTEX_UNLOCK(busyTaskList_mutex);	
	return tptask;
}

//-------------------------------------------------------------------------------------
bool ThreadPool::createThreadPool(unsigned int inewThreadCount, unsigned int inormalMaxThreadCount, unsigned int imaxThreadCount)
{
	assert(!m_isInitialize && "ThreadPool is exist initialize!");
	INFO_MSG("ThreadPool::start to create the pool of Thread...\n");
	
	m_extraNewAddThreadCount = inewThreadCount;
	m_normalThreadCount = inormalMaxThreadCount;
	m_maxThreadCount = imaxThreadCount;
	
	for(unsigned int i=0; i<m_normalThreadCount; i++)
	{
		TPThread* tptd = createThread();
		if(!tptd)
			ERROR_MSG("ThreadPool:: create Thread error! \n");
		
		m_currentFreeThreadCount++;	
		m_currentThreadCount++;
		m_freeThreadList.push_back(tptd);										// 闲置的线程列表
		m_allThreadList.push_back(tptd);										// 所有的线程列表
	}
	
	INFO_MSG("ThreadPool::createThreadPool is successfully(%d), newThreadCount=%d, normalMaxThreadCount=%d, maxThreadCount=%d\n", \
			m_currentThreadCount, m_extraNewAddThreadCount, m_normalThreadCount, m_maxThreadCount);

	m_isInitialize = true;
	return true;
}

//-------------------------------------------------------------------------------------
void ThreadPool::saveToBusyTaskList(TPTask* tptask)
{
	THREAD_MUTEX_LOCK(busyTaskList_mutex);
	m_busyTaskList.push(tptask);
	THREAD_MUTEX_UNLOCK(busyTaskList_mutex);
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
	THREAD_MUTEX_LOCK(threadStateList_mutex);
	std::list<TPThread*>::iterator itr;
	itr = find(m_busyThreadList.begin(), m_busyThreadList.end(), tptd);

	if(itr != m_busyThreadList.end()){
		m_busyThreadList.erase(itr);
	}
	else
	{
		THREAD_MUTEX_UNLOCK(threadStateList_mutex);
		ERROR_MSG("ThreadPool::moveThreadToFreeList: m_busyThreadList not found thread.%u\n", (unsigned int)tptd->getID());
		return false;
	}
		
	m_freeThreadList.push_back(tptd);
	m_currentFreeThreadCount++;
	THREAD_MUTEX_UNLOCK(threadStateList_mutex);
	return true;
}

//-------------------------------------------------------------------------------------
bool ThreadPool::moveThreadToBusyList(TPThread* tptd)
{
	THREAD_MUTEX_LOCK(threadStateList_mutex);
	std::list<TPThread*>::iterator itr;
	itr = find(m_freeThreadList.begin(), m_freeThreadList.end(), tptd);
	if(itr != m_freeThreadList.end()){
		m_freeThreadList.erase(itr);
	}
	else
	{
		THREAD_MUTEX_UNLOCK(threadStateList_mutex);
		ERROR_MSG("ThreadPool::moveThreadToBusyList: m_freeThreadList not found thread.%u\n", (unsigned int)tptd->getID());
		return false;
	}
		
	m_busyThreadList.push_back(tptd);
	m_currentFreeThreadCount--;
	THREAD_MUTEX_UNLOCK(threadStateList_mutex);		

	return true;
}

//-------------------------------------------------------------------------------------
bool ThreadPool::removeHangThread(TPThread* tptd)
{
	THREAD_MUTEX_LOCK(threadStateList_mutex);
	std::list<TPThread*>::iterator itr, itr1;
	itr = find(m_freeThreadList.begin(), m_freeThreadList.end(), tptd);
	itr1 = find(m_allThreadList.begin(), m_allThreadList.end(), tptd);
	
	if(itr != m_freeThreadList.end()&& itr1 != m_allThreadList.end()){
		m_freeThreadList.erase(itr);
		m_allThreadList.erase(itr1);
		m_currentThreadCount--;
		m_currentFreeThreadCount--;
		INFO_MSG("ThreadPool::removeHangThread: thread.%u is destroy. currentFreeThreadCount:%d, currentThreadCount:%d\n", \
		(unsigned int)tptd->getID(), m_currentFreeThreadCount, m_currentThreadCount);
		SAFE_RELEASE(tptd);
	}
	else
	{
		THREAD_MUTEX_UNLOCK(threadStateList_mutex);		
		ERROR_MSG("ThreadPool::removeHangThread: not found thread.%u\n", (unsigned int)tptd->getID());
		return false;
	}
	
	THREAD_MUTEX_UNLOCK(threadStateList_mutex);		
	return true;		
}

//-------------------------------------------------------------------------------------
bool ThreadPool::addTask(TPTask* tptask)
{
	THREAD_MUTEX_LOCK(threadStateList_mutex);
	if(m_currentFreeThreadCount > 0){
		std::list<TPThread*>::iterator itr = m_freeThreadList.begin();
		TPThread* tptd = (TPThread*)(*itr);
		m_freeThreadList.erase(itr);
		m_busyThreadList.push_back(tptd);
		m_currentFreeThreadCount--;
		INFO_MSG("ThreadPool::currFree:%d, currThreadCount:%d, busy:[%d]\n", m_currentFreeThreadCount, m_currentThreadCount, m_busyThreadList.size());
		THREAD_MUTEX_UNLOCK(threadStateList_mutex);
		
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
		THREAD_MUTEX_UNLOCK(threadStateList_mutex);
		WARNING_MSG("thread create is failed, count is full(%d).\n", m_maxThreadCount);
		return false;
	}

	for(unsigned int i=0; i<m_extraNewAddThreadCount; i++)
	{
		TPThread* tptd = createThread(300);									// 设定5分钟未使用则退出的线程
		if(!tptd){
#if KBE_PLATFORM == PLATFORM_WIN32		
			ERROR_MSG(">>ThreadPool create new Thread error! ... \n");
#else
			ERROR_MSG(">>ThreadPool create new Thread error:%s ... \n", strerror(errno));
#endif				
		}

		m_allThreadList.push_back(tptd);									// 所有的线程列表
		m_freeThreadList.push_back(tptd);									// 闲置的线程列表
		m_currentThreadCount++;
		m_currentFreeThreadCount++;	
		
	}
	
	INFO_MSG(">>ThreadPool:createNewThread-> currThreadCount: %d\n", m_currentThreadCount);
	THREAD_MUTEX_UNLOCK(threadStateList_mutex);
	return true;
}

//-------------------------------------------------------------------------------------
bool TPThread::onWaitCondSignal(void)
{
#if KBE_PLATFORM == PLATFORM_WIN32
	if(m_threadWaitSecond <= 0){
		m_state = 0;
		WaitForSingleObject(m_cond, INFINITE); 
		ResetEvent(m_cond);
	}
	else
	{
		m_state = 0;
		DWORD ret = WaitForSingleObject(m_cond, m_threadWaitSecond * 1000);
		ResetEvent(m_cond);

		if (ret == WAIT_TIMEOUT){											// 如果是因为超时了， 说明这个线程很久没有被用到， 我们应该注销这个线程。
			m_threadPool->removeHangThread(this);							// 通知ThreadPool注销自己
			return false;
		}
		else if(ret != WAIT_OBJECT_0){
			ERROR_MSG("WaitForSingleObject is error, ret=%d\n", ret);
		}
	}	
#else		
	if(m_threadWaitSecond <= 0){
		lock();
		m_state = 0;
		pthread_cond_wait(&cond, &mutex);
		unlock();
	}
	else
	{
		struct timeval now;
		struct timespec timeout;			
		gettimeofday(&now, NULL);
		timeout.tv_sec = now.tv_sec + m_threadWaitSecond;
		timeout.tv_nsec = now.tv_usec * 1000;
		
		lock();
		m_state = 0;
		int ret = pthread_cond_timedwait(&cond, &mutex, &timeout);
		unlock();
		
		// 如果是因为超时了， 说明这个线程很久没有被用到， 我们应该注销这个线程。
		if (ret == ETIMEDOUT){
			// 通知ThreadPool注销自己
			m_threadPool->removeHangThread(this);
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
	SAFE_RELEASE(m_currTask);
	m_threadPool->moveThreadToFreeList(this);
}

//-------------------------------------------------------------------------------------
TPTask* TPThread::tryGetTask(void)
{
	return m_threadPool->popBusyTaskList();
}

//-------------------------------------------------------------------------------------
}
}
