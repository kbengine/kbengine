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


#include "threadpool.hpp"
namespace KBEngine{ 
KBE_SINGLETON_INIT(KBEngine::thread::ThreadPool);
namespace thread{

// 全局数据的定义
ThreadPool g_stp;

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
		ERROR_MSG("createThread is error!");
	}
#endif
	return tidp_;
}

//-------------------------------------------------------------------------------------
ThreadPool::ThreadPool():
isInitialize_(false),
bufferedTaskList_(),
finiTaskList_()
{		
	extraNewAddThreadCount_ = currentThreadCount_ = 
	currentFreeThreadCount_ = normalThreadCount_ = 0;
	
	THREAD_MUTEX_INIT(threadStateList_mutex_);	
	THREAD_MUTEX_INIT(bufferedTaskList_mutex_);
	THREAD_MUTEX_INIT(finiTaskList_mutex_);
}

//-------------------------------------------------------------------------------------
ThreadPool::~ThreadPool()
{
	THREAD_MUTEX_DELETE(threadStateList_mutex_);
	THREAD_MUTEX_DELETE(bufferedTaskList_mutex_);
	THREAD_MUTEX_DELETE(finiTaskList_mutex_);
	
	std::list<TPThread*>::iterator itr = allThreadList_.begin();
	for(; itr != allThreadList_.end(); itr++)
	{
		if((*itr))
		{
			delete (*itr);
			(*itr) = NULL;
		}
	}
	
	std::list<TPTask*>::iterator finiiter  = finiTaskList_.begin();
	for(; finiiter != finiTaskList_.end(); finiiter++)
	{
		delete (*finiiter);
	}
	
	while(bufferedTaskList_.size() > 0)
	{
		TPTask* tptask = bufferedTaskList_.front();
		bufferedTaskList_.pop();
		delete tptask;
	}
}

//-------------------------------------------------------------------------------------
void ThreadPool::finalise()
{
	// 延时一下， 避免销毁时的极端情况导致出错
	KBEngine::sleep(100);
}

//-------------------------------------------------------------------------------------
TPTask* ThreadPool::popbufferTask(void)
{
	TPTask* tptask = NULL;
	THREAD_MUTEX_LOCK(bufferedTaskList_mutex_);

	if(bufferedTaskList_.size()> 0)
	{
		tptask = bufferedTaskList_.front();
		bufferedTaskList_.pop();
	}

	THREAD_MUTEX_UNLOCK(bufferedTaskList_mutex_);	
	return tptask;
}

//-------------------------------------------------------------------------------------
void ThreadPool::addFiniTask(TPTask* tptask)
{ 
	THREAD_MUTEX_LOCK(finiTaskList_mutex_);
	finiTaskList_.push_back(tptask); 
	THREAD_MUTEX_UNLOCK(finiTaskList_mutex_);	
}

//-------------------------------------------------------------------------------------
bool ThreadPool::createThreadPool(unsigned int inewThreadCount, 
	unsigned int inormalMaxThreadCount, unsigned int imaxThreadCount)
{
	assert(!isInitialize_ && "ThreadPool is exist!");
	INFO_MSG("ThreadPool::createThreadPool: creating  threadpool...\n");
	
	extraNewAddThreadCount_ = inewThreadCount;
	normalThreadCount_ = inormalMaxThreadCount;
	maxThreadCount_ = imaxThreadCount;
	
	for(unsigned int i=0; i<normalThreadCount_; i++)
	{
		TPThread* tptd = createThread();
		
		if(!tptd)
		{
			ERROR_MSG("ThreadPool::createThreadPool: create thread is error! \n");
		}

		currentFreeThreadCount_++;	
		currentThreadCount_++;
		freeThreadList_.push_back(tptd);										// 闲置的线程列表
		allThreadList_.push_back(tptd);										// 所有的线程列表
	}
	
	INFO_MSG(boost::format("ThreadPool::createThreadPool: is successfully(%1%), "
		"newThreadCount=%2%, normalMaxThreadCount=%3%, maxThreadCount=%4%\n") %
			currentThreadCount_ % extraNewAddThreadCount_ % normalThreadCount_ % maxThreadCount_);

	isInitialize_ = true;
	return true;
}

//-------------------------------------------------------------------------------------
void ThreadPool::onMainThreadTick()
{
	THREAD_MUTEX_LOCK(finiTaskList_mutex_);

	std::list<TPTask*>::iterator finiiter  = finiTaskList_.begin();

	for(; finiiter != finiTaskList_.end(); )
	{
		thread::TPTask::TPTaskState state = (*finiiter)->presentMainThread();

		switch(state)
		{
		case thread::TPTask::TPTASK_STATE_COMPLETED:
			delete (*finiiter);
			finiTaskList_.erase(finiiter++);
			break;
		case thread::TPTask::TPTASK_STATE_CONTINUE_CHILDTHREAD:
			this->addTask((*finiiter));
			finiTaskList_.erase(finiiter++);
			break;
		case thread::TPTask::TPTASK_STATE_CONTINUE_MAINTHREAD:
			++finiiter;
			break;
		default:
			KBE_ASSERT(false);
			break;
		};
	}

	THREAD_MUTEX_UNLOCK(finiTaskList_mutex_);	
}

//-------------------------------------------------------------------------------------
void ThreadPool::bufferTask(TPTask* tptask)
{
	THREAD_MUTEX_LOCK(bufferedTaskList_mutex_);
	bufferedTaskList_.push(tptask);

	WARNING_MSG(boost::format("ThreadPool::bufferTask: task buffered(%1%)!\n") % 
		(int)bufferedTaskList_.size());

	THREAD_MUTEX_UNLOCK(bufferedTaskList_mutex_);
}

//-------------------------------------------------------------------------------------
TPThread* ThreadPool::createThread(int threadWaitSecond)
{
	TPThread* tptd = new TPThread(this, threadWaitSecond);
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

		ERROR_MSG(boost::format("ThreadPool::addFreeThread: busyThreadList_ not found thread.%1%\n") %
		 (unsigned int)tptd->getID());

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
		ERROR_MSG(boost::format("ThreadPool::addBusyThread: freeThreadList_ not "
					"found thread.%1%\n") %
					(unsigned int)tptd->getID());
		
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
	
	if(itr != freeThreadList_.end() && itr1 != allThreadList_.end())
	{
		freeThreadList_.erase(itr);
		allThreadList_.erase(itr1);
		currentThreadCount_--;
		currentFreeThreadCount_--;

		INFO_MSG(boost::format("ThreadPool::removeHangThread: thread.%1% is destroy. "
			"currentFreeThreadCount:%2%, currentThreadCount:%3%\n") %
		(unsigned int)tptd->getID() % currentFreeThreadCount_ % currentThreadCount_);
		
		SAFE_RELEASE(tptd);
	}
	else
	{
		THREAD_MUTEX_UNLOCK(threadStateList_mutex_);		
		
		ERROR_MSG(boost::format("ThreadPool::removeHangThread: not found thread.%1%\n") % 
			(unsigned int)tptd->getID());
		
		return false;
	}
	
	THREAD_MUTEX_UNLOCK(threadStateList_mutex_);		
	return true;		
}

//-------------------------------------------------------------------------------------
bool ThreadPool::addTask(TPTask* tptask)
{
	THREAD_MUTEX_LOCK(threadStateList_mutex_);
	if(currentFreeThreadCount_ > 0)
	{
		std::list<TPThread*>::iterator itr = freeThreadList_.begin();
		TPThread* tptd = (TPThread*)(*itr);
		freeThreadList_.erase(itr);
		busyThreadList_.push_back(tptd);
		currentFreeThreadCount_--;
		
		//INFO_MSG("ThreadPool::currFree:%d, currThreadCount:%d, busy:[%d]\n",
		//		 currentFreeThreadCount_, currentThreadCount_, busyThreadList_.size());
		
		tptd->setTask(tptask);												// 给线程设置新任务	
		
#if KBE_PLATFORM == PLATFORM_WIN32
		if(tptd->sendCondSignal()== 0){
#else
		if(tptd->sendCondSignal()!= 0){
#endif
			ERROR_MSG("ThreadPool::addTask: pthread_cond_signal is error!\n");
			THREAD_MUTEX_UNLOCK(threadStateList_mutex_);
			return false;
		}
		
		THREAD_MUTEX_UNLOCK(threadStateList_mutex_);
		return true;
	}
	
	bufferTask(tptask);
	
	if(isThreadCountMax())
	{
		THREAD_MUTEX_UNLOCK(threadStateList_mutex_);

		WARNING_MSG(boost::format("ThreadPool::addTask: thread create is failed, count is full(%1%).\n") % 
			maxThreadCount_);

		return false;
	}

	for(unsigned int i=0; i<extraNewAddThreadCount_; i++)
	{
		TPThread* tptd = createThread(300);									// 设定5分钟未使用则退出的线程
		if(!tptd)
		{
#if KBE_PLATFORM == PLATFORM_WIN32		
			ERROR_MSG("ThreadPool::addTask: ThreadPool create new Thread error! ... \n");
#else
			ERROR_MSG(boost::format("boost::format(ThreadPool::addTask: ThreadPool create new Thread error:%1% ... \n") % 
				kbe_strerror());
#endif				
		}

		allThreadList_.push_back(tptd);										// 所有的线程列表
		freeThreadList_.push_back(tptd);									// 闲置的线程列表
		currentThreadCount_++;
		currentFreeThreadCount_++;	
		
	}
	
	INFO_MSG(boost::format("ThreadPool::addTask: createNewThread-> currThreadCount: %1%\n") % 
		currentThreadCount_);

	THREAD_MUTEX_UNLOCK(threadStateList_mutex_);
	return true;
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

		if (ret == WAIT_TIMEOUT)
		{																	// 如果是因为超时了， 说明这个线程很久没有被用到， 我们应该注销这个线程。
			threadPool_->removeHangThread(this);							// 通知ThreadPool注销自己
			return false;
		}
		else if(ret != WAIT_OBJECT_0)
		{
			ERROR_MSG(boost::format("TPThread::onWaitCondSignal: WaitForSingleObject is error, ret=%1%\n") % ret);
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
			ERROR_MSG(boost::format("TPThread::onWaitCondSignal: pthread_cond_timedwait is error, %1%\n") % 
				kbe_strerror());
		}
	}
#endif
	return true;
}

//-------------------------------------------------------------------------------------
void TPThread::onTaskComplete(void)
{
	deleteFiniTask(currTask_);
	currTask_ = NULL;
	threadPool_->addFreeThread(this);
}

//-------------------------------------------------------------------------------------
TPTask* TPThread::tryGetTask(void)
{
	return threadPool_->popbufferTask();
}

//-------------------------------------------------------------------------------------
void TPThread::deleteFiniTask(TPTask* tpTask)
{
	threadPool_->addFiniTask(tpTask);
}

//-------------------------------------------------------------------------------------
}
}
