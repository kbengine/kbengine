namespace KBEngine { 
namespace Thread
{

INLINE bool ThreadPool::isInitialize(void) const
{ 
	return isInitialize_; 
}

INLINE bool ThreadPool::isBusy(void) const
{
	return bufferedTaskList_.size() > THREAD_BUSY_SIZE;
}	

INLINE bool ThreadPool::isThreadCountMax(void) const
{
	return currentThreadCount_ >= maxThreadCount_;	
}

INLINE uint32 ThreadPool::currentThreadCount(void) const
{ 
	return currentThreadCount_; 
}

INLINE uint32 ThreadPool::currentFreeThreadCount(void) const
{ 
	return currentFreeThreadCount_; 
}

INLINE bool ThreadPool::isDestroyed() const 
{ 
	return isDestroyed_; 
}

ThreadPool* TPThread::threadPool()
{
	return threadPool_;
}

INLINE uint32 ThreadPool::bufferTaskSize() const
{
	return (uint32)bufferedTaskList_.size();
}

INLINE std::queue<Thread::TPTask*>& ThreadPool::bufferedTaskList()
{
	return bufferedTaskList_;
}

INLINE uint32 ThreadPool::finiTaskSize() const
{
	return (uint32)finiTaskList_count_;
}

INLINE TPTask* TPThread::task(void) const
{
	return currTask_;
}

INLINE void TPThread::task(TPTask* tpt)
{
	currTask_ = tpt;
}

INLINE int TPThread::state(void) const
{
	return state_;
}


}
}
