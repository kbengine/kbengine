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


#include "event_dispatcher.hpp"
#include "network/event_poller.hpp"
#include "network/error_reporter.hpp"

#ifndef CODE_INLINE
#include "event_dispatcher.ipp"
#endif

namespace KBEngine { 
namespace Mercury
{

EventDispatcher::EventDispatcher() :
	breakProcessing_(false),
	maxWait_(0.1),
	numTimerCalls_(0),
	accSpareTime_(0),
	oldSpareTime_(0),
	totSpareTime_(0),
	lastStatisticsGathered_(0),
	pFrequentTasks_(new Tasks),
	pErrorReporter_(NULL),
	pTimers_(new Timers64),
	pCouplingToParent_(NULL)
	
{
	pPoller_ = EventPoller::create();
	pErrorReporter_ = new ErrorReporter(*this);
}

//-------------------------------------------------------------------------------------
EventDispatcher::~EventDispatcher()
{
	SAFE_RELEASE(pErrorReporter_);
	SAFE_RELEASE(pFrequentTasks_);
	SAFE_RELEASE(pPoller_);
	
	if (!pTimers_->empty())
	{
//		INFO_MSG(boost::format("EventDispatcher()::~EventDispatcher: Num timers = %1%\n") %
//			pTimers_->size());
	}

	pTimers_->clear(false);
	SAFE_RELEASE(pTimers_);
}

//-------------------------------------------------------------------------------------
EventPoller* EventDispatcher::createPoller()
{
	pPoller_ = EventPoller::create();
	return pPoller_;
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
	KBE_ASSERT(pCouplingToParent_ == NULL);
	pCouplingToParent_ = new DispatcherCoupling(parentDispatcher, *this);

	int fd = pPoller_->getFileDescriptor();

	if (fd != -1)
	{
		parentDispatcher.registerFileDescriptor(fd, pPoller_);
		parentDispatcher.registerWriteFileDescriptor(fd, pPoller_);
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
	int fd = pPoller_->getFileDescriptor();

	if (fd != -1)
	{
		parentDispatcher.deregisterFileDescriptor(fd);
		parentDispatcher.deregisterWriteFileDescriptor(fd);
	}

	KBE_ASSERT(pCouplingToParent_ != NULL);
	delete pCouplingToParent_;
	pCouplingToParent_ = NULL;
}

//-------------------------------------------------------------------------------------
bool EventDispatcher::registerFileDescriptor(int fd,
	InputNotificationHandler * handler)
{
	return pPoller_->registerForRead(fd, handler);
}

//-------------------------------------------------------------------------------------
bool EventDispatcher::registerWriteFileDescriptor(int fd,
	InputNotificationHandler * handler)
{
	return pPoller_->registerForWrite(fd, handler);
}

//-------------------------------------------------------------------------------------
bool EventDispatcher::deregisterFileDescriptor(int fd)
{
	return pPoller_->deregisterForRead(fd);
}

//-------------------------------------------------------------------------------------
bool EventDispatcher::deregisterWriteFileDescriptor(int fd)
{
	return pPoller_->deregisterForWrite(fd);
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

	TimerHandle handle = pTimers_->add(timestamp() + interval,
			recurrent ? interval : 0,
			handler, arg);

	return handle;
}

//-------------------------------------------------------------------------------------
uint64 EventDispatcher::getSpareTime() const
{
	return pPoller_->spareTime();
}

//-------------------------------------------------------------------------------------
void EventDispatcher::clearSpareTime()
{
	accSpareTime_ += pPoller_->spareTime();
	pPoller_->clearSpareTime();
}

uint64 EventDispatcher::timerDeliveryTime(TimerHandle handle) const
{
	return pTimers_->timerDeliveryTime(handle);
}

//-------------------------------------------------------------------------------------
uint64 EventDispatcher::timerIntervalTime(TimerHandle handle) const
{
	return pTimers_->timerIntervalTime(handle);
}

//-------------------------------------------------------------------------------------
uint64 & EventDispatcher::timerIntervalTime(TimerHandle handle)
{
	return pTimers_->timerIntervalTime(handle);
}

//-------------------------------------------------------------------------------------
double EventDispatcher::proportionalSpareTime() const
{
	double ret = (double)(int64)(totSpareTime_ - oldSpareTime_);
	return ret / stampsPerSecondD();
	return 0;
}

//-------------------------------------------------------------------------------------
void EventDispatcher::addFrequentTask(Task * pTask)
{
	pFrequentTasks_->add(pTask);
}

//-------------------------------------------------------------------------------------
bool EventDispatcher::cancelFrequentTask(Task * pTask)
{
	return pFrequentTasks_->cancel(pTask);
}

//-------------------------------------------------------------------------------------
double EventDispatcher::calculateWait() const
{
	double maxWait = maxWait_;

	if (!pTimers_->empty())
	{
		maxWait = std::min(maxWait,
			pTimers_->nextExp(timestamp()) / stampsPerSecondD());
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
	pFrequentTasks_->process();
}

//-------------------------------------------------------------------------------------
void EventDispatcher::processTimers()
{
	numTimerCalls_ += pTimers_->process(timestamp());
}

//-------------------------------------------------------------------------------------
void EventDispatcher::processStats()
{
	if (timestamp() - lastStatisticsGathered_ >= stampsPerSecond())
	{
		oldSpareTime_ = totSpareTime_;
		totSpareTime_ = accSpareTime_ + pPoller_->spareTime();

		lastStatisticsGathered_ = timestamp();
	}
}

//-------------------------------------------------------------------------------------
int EventDispatcher::processNetwork(bool shouldIdle)
{
	double maxWait = shouldIdle ? this->calculateWait() : 0.0;
	return pPoller_->processPendingEvents(maxWait);
}

//-------------------------------------------------------------------------------------
void EventDispatcher::processUntilBreak()
{
	this->processContinuously();
}

//-------------------------------------------------------------------------------------
void EventDispatcher::processContinuously()
{
	breakProcessing_ = EVENT_DISPATCHER_STATUS_RUNNING;
	while(breakProcessing_ != EVENT_DISPATCHER_STATUS_BREAK_PROCESSING)
	{
		this->processOnce(true);
	}
}

//-------------------------------------------------------------------------------------
int EventDispatcher::processOnce(bool shouldIdle)
{
	breakProcessing_ = EVENT_DISPATCHER_STATUS_RUNNING;
	this->processFrequentTasks();
	if(breakProcessing_ != EVENT_DISPATCHER_STATUS_BREAK_PROCESSING){
		this->processTimers();
	}

	this->processStats();
	
	if(breakProcessing_ != EVENT_DISPATCHER_STATUS_BREAK_PROCESSING){
		return this->processNetwork(shouldIdle);
	}

	return 0;
}

}
}
