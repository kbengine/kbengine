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

#ifndef CODE_INLINE
#include "threadpool.ipp"
#endif

#include "helper/watcher.hpp"

namespace KBEngine{ 
KBE_SINGLETON_INIT(KBEngine::thread::ThreadPool);
namespace thread{

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
		ERROR_MSG("createThread is error!");
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
		DWORD dw = WaitForSingleObject(getID(), 3000);  
		switch (dw)
		{
		case WAIT_OBJECT_0:
			return true;
		case WAIT_TIMEOUT:
			if(i > 20)
			{
				ERROR_MSG(boost::format("TPThread::join: can't join thread(%1%)\n") % this);
				return false;
			}
			else
			{
				WARNING_MSG(boost::format("TPThread::join: waiting for thread(%1%), try=%2%\n") % this % i);
			}
			break;
		case WAIT_FAILED:
		default:
			ERROR_MSG(boost::format("TPThread::join: can't join thread(%1%)\n") % this);
			return false;
		};
	}
#else
	void* status;
	if(pthread_join(getID(), &status))
	{
		ERROR_MSG(boost::format("TPThread::join: can't join thread(%1%)\n") % this);
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
isDestroyed_(false)
{		
	extraNewAddThreadCount_ =  0;
	currentThreadCount_ =  0;
	currentFreeThreadCount_ =  0;
	normalThreadCount_ = 0;
	
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
	WATCH_OBJECT("threadpool/maxThreadCount", this->maxThreadCount_);
	WATCH_OBJECT("threadpool/extraNewAddThreadCount", this->extraNewAddThreadCount_);
	WATCH_OBJECT("threadpool/currentFreeThreadCount", this->currentFreeThreadCount_);
	WATCH_OBJECT("threadpool/normalThreadCount", this->normalThreadCount_);
	WATCH_OBJECT("threadpool/bufferedTaskSize", this, &ThreadPool::bufferTaskSize);
	WATCH_OBJECT("threadpool/finiTaskSize", this, &ThreadPool::finiTaskSize);
	return true;
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

	DEBUG_MSG(boost::format("ThreadPool::destroy(): starting size %1%.\n") % 
		allThreadList_.size());
	
	THREAD_MUTEX_UNLOCK(threadStateList_mutex_);

	int itry = 0;
	while(true)
	{
		KBEngine::sleep(300);
		itry++;

		THREAD_MUTEX_LOCK(threadStateList_mutex_);
		int count = allThreadList_.size();
		std::list<TPThread*>::iterator itr = allThreadList_.begin();
		for(; itr != allThreadList_.end(); itr++)
		{
			if((*itr))
			{
				if((*itr)->getState() != TPThread::THREAD_STATE_END)
					(*itr)->sendCondSignal();
				else
					count--;
			}
		}

		THREAD_MUTEX_UNLOCK(threadStateList_mutex_);

		if(count <= 0)
		{
			break;
		}
		else
		{
			WARNING_MSG(boost::format("ThreadPool::destroy(): waiting for thread(%1%), try=%2%\n") % count % itry);
		}
	}

	THREAD_MUTEX_LOCK(threadStateList_mutex_);

	KBEngine::sleep(100);

	std::list<TPThread*>::iterator itr = allThreadList_.begin();
	for(; itr != allThreadList_.end(); itr++)
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
		WARNING_MSG(boost::format("ThreadPool::~ThreadPool(): Discarding %1% finished tasks.\n") % 
			finiTaskList_.size());

		std::list<TPTask*>::iterator finiiter  = finiTaskList_.begin();
		for(; finiiter != finiTaskList_.end(); finiiter++)
		{
			delete (*finiiter);
		}
	
		finiTaskList_.clear();
	}

	THREAD_MUTEX_UNLOCK(finiTaskList_mutex_);

	THREAD_MUTEX_LOCK(bufferedTaskList_mutex_);

	if(bufferedTaskList_.size() > 0)
	{
		WARNING_MSG(boost::format("ThreadPool::~ThreadPool(): Discarding %1% buffered tasks.\n") % 
			bufferedTaskList_.size());

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

	if(bufferedTaskList_.size() > 0)
	{
		tptask = bufferedTaskList_.front();
		bufferedTaskList_.pop();
	}
	
	if(bufferedTaskList_.size() > THREAD_BUSY_SIZE)
	{
		WARNING_MSG(boost::format("ThreadPool::popbufferTask: task buffered(%1%)!\n") % 
			bufferedTaskList_.size());
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
bool ThreadPool::createThreadPool(uint32 inewThreadCount, 
	uint32 inormalMaxThreadCount, uint32 imaxThreadCount)
{
	assert(!isInitialize_ && "ThreadPool is exist!");
	INFO_MSG("ThreadPool::createThreadPool: creating  threadpool...\n");
	
	extraNewAddThreadCount_ = inewThreadCount;
	normalThreadCount_ = inormalMaxThreadCount;
	maxThreadCount_ = imaxThreadCount;
	
	for(uint32 i=0; i<normalThreadCount_; i++)
	{
		TPThread* tptd = createThread(0);
		
		if(!tptd)
		{
			ERROR_MSG("ThreadPool::createThreadPool: create thread is error! \n");
		}

		currentFreeThreadCount_++;	
		currentThreadCount_++;
		freeThreadList_.push_back(tptd);										// 闲置的线程列表
		allThreadList_.push_back(tptd);										// 所有的线程列表
	}
	
	INFO_MSG(boost::format("ThreadPool::createThreadPool: successfully(%1%), "
		"newThreadCount=%2%, normalMaxThreadCount=%3%, maxThreadCount=%4%\n") %
			currentThreadCount_ % extraNewAddThreadCount_ % normalThreadCount_ % maxThreadCount_);

	isInitialize_ = true;
	KBEngine::sleep(100);
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

	if(bufferedTaskList_.size() > THREAD_BUSY_SIZE)
	{
		WARNING_MSG(boost::format("ThreadPool::bufferTask: task buffered(%1%)!\n") % 
			bufferedTaskList_.size());
	}

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
		 (uint32)tptd->getID());
		
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
		ERROR_MSG(boost::format("ThreadPool::addBusyThread: freeThreadList_ not "
					"found thread.%1%\n") %
					(uint32)tptd->getID());
		
		delete tptd;
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
		(uint32)tptd->getID() % currentFreeThreadCount_ % currentThreadCount_);
		
		SAFE_RELEASE(tptd);
	}
	else
	{
		THREAD_MUTEX_UNLOCK(threadStateList_mutex_);		
		
		ERROR_MSG(boost::format("ThreadPool::removeHangThread: not found thread.%1%\n") % 
			(uint32)tptd->getID());
		
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

		WARNING_MSG(boost::format("ThreadPool::addTask: can't createthread, the poolsize is full(%1%).\n") % 
			maxThreadCount_);

		return false;
	}

	for(uint32 i=0; i<extraNewAddThreadCount_; i++)
	{
		TPThread* tptd = createThread(300);									// 设定5分钟未使用则退出的线程
		if(!tptd)
		{
#if KBE_PLATFORM == PLATFORM_WIN32		
			ERROR_MSG("ThreadPool::addTask: the ThreadPool create thread is error! ... \n");
#else
			ERROR_MSG(boost::format("boost::format(ThreadPool::addTask: the ThreadPool create thread is error:%1%\n") % 
				kbe_strerror());
#endif				
		}

		allThreadList_.push_back(tptd);										// 所有的线程列表
		freeThreadList_.push_back(tptd);									// 闲置的线程列表
		currentThreadCount_++;
		currentFreeThreadCount_++;	
		
	}
	
	INFO_MSG(boost::format("ThreadPool::addTask: new Thread, currThreadCount: %1%\n") % 
		currentThreadCount_);

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

#if KBE_PLATFORM == PLATFORM_WIN32
#else			
	pthread_detach(pthread_self());
#endif

	tptd->onStart();

	while(isRun)
	{
		if(tptd->getTask() != NULL)
		{
			isRun = true;
		}
		else
		{
			isRun = tptd->onWaitCondSignal();
		}

		if(!isRun || pThreadPool->isDestroyed())
		{
			if(!pThreadPool->hasThread(tptd))
				tptd = NULL;

			goto __THREAD_END__;
		}

		TPTask * task = tptd->getTask();
		if(task == NULL)
			continue;

		tptd->state_ = THREAD_STATE_BUSY;
		while(task && !tptd->threadPool()->isDestroyed())
		{
			tptd->onProcessTaskStart(task);
			tptd->processTask(task);							// 处理该任务								
			tptd->onProcessTaskEnd(task);

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

__THREAD_END__:
	if(tptd)
	{
		TPTask * task = tptd->getTask();
		if(task)
		{
			WARNING_MSG(boost::format("TPThread::threadFunc: task %1% not finish, thread.%2% will exit.\n") % 
				task % tptd);

			delete task;
		}

		tptd->onEnd();
		tptd->state_ = THREAD_STATE_END;
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
			ERROR_MSG(boost::format("TPThread::onWaitCondSignal: WaitForSingleObject is error, ret=%1%\n") 
				% ret);
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
