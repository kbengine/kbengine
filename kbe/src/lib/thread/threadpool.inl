namespace KBEngine { 
namespace thread
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

INLINE std::queue<thread::TPTask*>& ThreadPool::bufferedTaskList()
{
	return bufferedTaskList_;
}

INLINE void ThreadPool::lockBufferedTaskList()
{
	THREAD_MUTEX_LOCK(bufferedTaskList_mutex_);
}

INLINE void ThreadPool::unlockBufferedTaskList()
{
	THREAD_MUTEX_UNLOCK(bufferedTaskList_mutex_);
}
	
INLINE uint32 ThreadPool::finiTaskSize() const
{
	return (uint32)finiTaskList_count_;
}

INLINE THREAD_ID TPThread::id(void) const
{
	return tidp_;
}

INLINE void TPThread::id(THREAD_ID tidp)
{
	tidp_ = tidp;
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
