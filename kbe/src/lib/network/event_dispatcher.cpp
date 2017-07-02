/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2017 KBEngine.

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


#include "event_dispatcher.h"
#include "network/event_poller.h"
#include "network/error_reporter.h"
#include "helper/profile.h"

#ifndef CODE_INLINE
#include "event_dispatcher.inl"
#endif

namespace KBEngine { 
namespace Network
{

EventDispatcher::EventDispatcher() :
	breakProcessing_(EVENT_DISPATCHER_STATUS_RUNNING),
	maxWait_(0.1),
	numTimerCalls_(0),
	accSpareTime_(0),
	oldSpareTime_(0),
	totSpareTime_(0),
	lastStatisticsGathered_(0),
	pTasks_(new Tasks),
	pErrorReporter_(NULL),
	pTimers_(new Timers64)
	
{
	pPoller_ = EventPoller::create();
	pErrorReporter_ = new ErrorReporter(*this);
}

//-------------------------------------------------------------------------------------
EventDispatcher::~EventDispatcher()
{
	SAFE_RELEASE(pErrorReporter_);
	SAFE_RELEASE(pTasks_);
	SAFE_RELEASE(pPoller_);
	
	if (!pTimers_->empty())
	{
		INFO_MSG(fmt::format("EventDispatcher()::~EventDispatcher: Num timers = {}\n",
			pTimers_->size()));
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
bool EventDispatcher::registerReadFileDescriptor(int fd,
	InputNotificationHandler * handler)
{
	return pPoller_->registerForRead(fd, handler);
}

//-------------------------------------------------------------------------------------
bool EventDispatcher::registerWriteFileDescriptor(int fd,
	OutputNotificationHandler * handler)
{
	return pPoller_->registerForWrite(fd, handler);
}

//-------------------------------------------------------------------------------------
bool EventDispatcher::deregisterReadFileDescriptor(int fd)
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

//-------------------------------------------------------------------------------------
void EventDispatcher::addTask(Task * pTask)
{
	pTasks_->add(pTask);
}

//-------------------------------------------------------------------------------------
bool EventDispatcher::cancelTask(Task * pTask)
{
	return pTasks_->cancel(pTask);
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

	return maxWait;
}

//-------------------------------------------------------------------------------------
void EventDispatcher::processTasks()
{
	pTasks_->process();
}

//-------------------------------------------------------------------------------------
void EventDispatcher::processTimers()
{
	AUTO_SCOPED_PROFILE("callSystemTimers")
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
	if(breakProcessing_ != EVENT_DISPATCHER_STATUS_BREAK_PROCESSING)
		breakProcessing_ = EVENT_DISPATCHER_STATUS_RUNNING;

	while(breakProcessing_ != EVENT_DISPATCHER_STATUS_BREAK_PROCESSING)
	{
		this->processOnce(true);
	}
}

//-------------------------------------------------------------------------------------
int EventDispatcher::processOnce(bool shouldIdle)
{
	if(breakProcessing_ != EVENT_DISPATCHER_STATUS_BREAK_PROCESSING)
		breakProcessing_ = EVENT_DISPATCHER_STATUS_RUNNING;

	this->processTasks();

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
