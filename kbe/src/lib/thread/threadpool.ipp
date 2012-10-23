namespace KBEngine { 
namespace thread
{

INLINE bool ThreadPool::isInitialize(void)const
{ 
	return isInitialize_; 
}

INLINE bool ThreadPool::isBusy(void)const
{
	return busyTaskList_.size() > THREAD_BUSY_SIZE;
}	

INLINE bool ThreadPool::isThreadCountMax(void)const
{
	return currentThreadCount_ >= maxThreadCount_;	
}

INLINE unsigned int ThreadPool::getCurrentThreadCount(void)const
{ 
	return currentThreadCount_; 
}

INLINE unsigned int ThreadPool::getCurrentFreeThreadCount(void)const
{ 
	return currentFreeThreadCount_; 
}

INLINE THREAD_ID TPThread::getID(void)const
{
	return tidp_;
}

INLINE void TPThread::setID(THREAD_ID tidp)
{
	tidp_ = tidp;
}

INLINE TPTask* TPThread::getTask(void)const
{
	return currTask_;
}

INLINE void TPThread::setTask(TPTask* tpt)
{
	currTask_ = tpt;
}

INLINE int TPThread::getState(void)const
{
	return state_;
}
}
}