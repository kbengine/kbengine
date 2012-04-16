#include "event_dispatcher.hpp"

namespace KBEngine { 
namespace Mercury
{

EventDispatcher::EventDispatcher() :
	m_breakProcessing_(false),
	m_maxWait_(0.1),
	m_numTimerCalls_(0),
	m_accSpareTime_(0),
	m_oldSpareTime_(0),
	m_totSpareTime_(0),
	m_lastStatisticsGathered_(0),
	m_pFrequentTasks_(new Tasks),
	m_pErrorReporter_(NULL),
	m_pTimes_(new Times64),
	m_pCouplingToParent_(NULL)
	
{
	m_pPoller_ = EventPoller::create();
	m_pErrorReporter_ = new ErrorReporter(*this);
}

//-------------------------------------------------------------------------------------
EventDispatcher::~EventDispatcher()
{
	delete m_pFrequentTasks_;
	delete m_pPoller_;
	
	if (!m_pTimes_->empty())
	{
		INFO_MSG("EventDispatcher()::~EventDispatcher: Num timers = %d\n",
			m_pTimes_->size());
	}

	m_pTimes_->clear(false);
	delete m_pTimes_;
}

//-------------------------------------------------------------------------------------
EventPoller* EventDispatcher::createPoller()
{
	m_pPoller_ = EventPoller::create();
	return m_pPoller_;
}

//-------------------------------------------------------------------------------------
void EventDispatcher::attach(EventDispatcher & childDispatcher)
{
	childDispatcher.attachTo(*this);
	childDispatchers_.push_back(&childDispatcher);
}

//-------------------------------------------------------------------------------------
void EventDispatcher::attachTo(EventDispatcher & parentDispatcher)
{
	KBE_ASSERT(m_pCouplingToParent_ == NULL);
	m_pCouplingToParent_ = new DispatcherCoupling(parentDispatcher, *this);

	int fd = m_pPoller_->getFileDescriptor();

	if (fd != -1)
	{
		parentDispatcher.registerFileDescriptor(fd, m_pPoller_);
		parentDispatcher.registerWriteFileDescriptor(fd, m_pPoller_);
	}
}

//-------------------------------------------------------------------------------------
void EventDispatcher::detach(EventDispatcher & childDispatcher)
{
	childDispatcher.detachFrom(*this);

	ChildDispatchers & d = childDispatchers_;
	d.erase(std::remove(d.begin(), d.end(), &childDispatcher), d.end());
}

//-------------------------------------------------------------------------------------
void EventDispatcher::detachFrom(EventDispatcher & parentDispatcher)
{
	int fd = m_pPoller_->getFileDescriptor();

	if (fd != -1)
	{
		parentDispatcher.deregisterFileDescriptor(fd);
		parentDispatcher.deregisterWriteFileDescriptor(fd);
	}

	KBE_ASSERT(m_pCouplingToParent_ != NULL);
	delete m_pCouplingToParent_;
	m_pCouplingToParent_ = NULL;
}

//-------------------------------------------------------------------------------------
bool EventDispatcher::registerFileDescriptor(int fd,
	InputNotificationHandler * handler)
{
	return m_pPoller_->registerForRead(fd, handler);
}

//-------------------------------------------------------------------------------------
bool EventDispatcher::registerWriteFileDescriptor(int fd,
	InputNotificationHandler * handler)
{
	return m_pPoller_->registerForWrite(fd, handler);
}

//-------------------------------------------------------------------------------------
bool EventDispatcher::deregisterFileDescriptor(int fd)
{
	return m_pPoller_->deregisterForRead(fd);
}

//-------------------------------------------------------------------------------------
bool EventDispatcher::deregisterWriteFileDescriptor(int fd)
{
	return m_pPoller_->deregisterForWrite(fd);
}

//-------------------------------------------------------------------------------------
TimerHandle EventDispatcher::addTimerCommon(int64 microseconds,
	TimerHandler * handler,
	void * arg,
	bool recurrent)
{
	KBE_ASSERT(handler);

	if (microseconds <= 0)
		return TimerHandle();

	uint64 interval = int64(
		(((double)microseconds)/1000000.0) * stampsPerSecondD());

	TimerHandle handle = m_pTimes_->add(timestamp() + interval,
			recurrent ? interval : 0,
			handler, arg);

	return handle;
}

//-------------------------------------------------------------------------------------
uint64 EventDispatcher::getSpareTime() const
{
	return m_pPoller_->spareTime();
}

//-------------------------------------------------------------------------------------
void EventDispatcher::clearSpareTime()
{
	m_accSpareTime_ += m_pPoller_->spareTime();
	m_pPoller_->clearSpareTime();
}

uint64 EventDispatcher::timerDeliveryTime(TimerHandle handle) const
{
	return m_pTimes_->timerDeliveryTime(handle);
}

//-------------------------------------------------------------------------------------
uint64 EventDispatcher::timerIntervalTime(TimerHandle handle) const
{
	return m_pTimes_->timerIntervalTime(handle);
}

//-------------------------------------------------------------------------------------
uint64 & EventDispatcher::timerIntervalTime(TimerHandle handle)
{
	return m_pTimes_->timerIntervalTime(handle);
}

//-------------------------------------------------------------------------------------
double EventDispatcher::proportionalSpareTime() const
{
	double ret = (double)(int64)(m_totSpareTime_ - m_oldSpareTime_);
	return ret / stampsPerSecondD();
	return 0;
}

//-------------------------------------------------------------------------------------
void EventDispatcher::addFrequentTask(Task * pTask)
{
	m_pFrequentTasks_->add(pTask);
}

//-------------------------------------------------------------------------------------
bool EventDispatcher::cancelFrequentTask(Task * pTask)
{
	return m_pFrequentTasks_->cancel(pTask);
}

//-------------------------------------------------------------------------------------
double EventDispatcher::calculateWait() const
{
	double maxWait = m_maxWait_;

	if (!m_pTimes_->empty())
	{
		maxWait = std::min(maxWait,
			m_pTimes_->nextExp(timestamp()) / stampsPerSecondD());
	}

	ChildDispatchers::const_iterator iter = childDispatchers_.begin();

	while (iter != childDispatchers_.end())
	{
		maxWait = std::min(maxWait, (*iter)->calculateWait());
		++iter;
	}

	return maxWait;
}

//-------------------------------------------------------------------------------------
void EventDispatcher::processFrequentTasks()
{
	m_pFrequentTasks_->process();
}

//-------------------------------------------------------------------------------------
void EventDispatcher::processTimers()
{
	m_numTimerCalls_ += m_pTimes_->process(timestamp());
}

//-------------------------------------------------------------------------------------
void EventDispatcher::processStats()
{
	if (timestamp() - m_lastStatisticsGathered_ >= stampsPerSecond())
	{
		m_oldSpareTime_ = m_totSpareTime_;
		m_totSpareTime_ = m_accSpareTime_ + m_pPoller_->spareTime();

		m_lastStatisticsGathered_ = timestamp();
	}
}

//-------------------------------------------------------------------------------------
int EventDispatcher::processNetwork(bool shouldIdle)
{
	double maxWait = shouldIdle ? this->calculateWait() : 0.0;
	return m_pPoller_->processPendingEvents(maxWait);
}

//-------------------------------------------------------------------------------------
void EventDispatcher::processUntilBreak()
{
	this->processContinuously();
}

//-------------------------------------------------------------------------------------
void EventDispatcher::processContinuously()
{
	m_breakProcessing_ = false;
	while(!m_breakProcessing_)
	{
		this->processOnce(true);
	}
}

//-------------------------------------------------------------------------------------
int EventDispatcher::processOnce(bool shouldIdle)
{
	m_breakProcessing_ = false;
	this->processFrequentTasks();
	if(!m_breakProcessing_){
		this->processTimers();
	}

	this->processStats();
	
	if(!m_breakProcessing_){
		return this->processNetwork(shouldIdle);
	}

	return 0;
}

}
}