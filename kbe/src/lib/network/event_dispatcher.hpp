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

#ifndef KBE_EVENT_DISPATCHER_HPP
#define KBE_EVENT_DISPATCHER_HPP

#include <map>
#include "cstdkbe/tasks.hpp"
#include "cstdkbe/timer.hpp"
#include "network/interfaces.hpp"
#include "network/common.hpp"

namespace KBEngine { 
namespace Network
{
class DispatcherCoupling;
class ErrorReporter;
class EventPoller;

class EventDispatcher
{
public:
	enum EVENT_DISPATCHER_STATUS
	{
		EVENT_DISPATCHER_STATUS_RUNNING = 0,
		EVENT_DISPATCHER_STATUS_WAITING_BREAK_PROCESSING = 1,
		EVENT_DISPATCHER_STATUS_BREAK_PROCESSING = 2
	};

	EventDispatcher();
	virtual ~EventDispatcher();
	
	void processContinuously();
	int  processOnce(bool shouldIdle = false);
	void processUntilBreak();

	bool isBreakProcessing()const { return breakProcessing_ == EVENT_DISPATCHER_STATUS_BREAK_PROCESSING; }
	bool isWaitBreakProcessing()const { return breakProcessing_ == EVENT_DISPATCHER_STATUS_WAITING_BREAK_PROCESSING; }

	void breakProcessing(bool breakState = true);
	INLINE void setWaitBreakProcessing();
	bool processingBroken()const;
	
	void addFrequentTask(Task * pTask);
	bool cancelFrequentTask(Task * pTask);
	
	INLINE double maxWait() const;
	INLINE void maxWait(double seconds);

	bool registerFileDescriptor(int fd, InputNotificationHandler * handler);
	bool deregisterFileDescriptor(int fd);
	bool registerWriteFileDescriptor(int fd, InputNotificationHandler * handler);
	bool deregisterWriteFileDescriptor(int fd);

	INLINE TimerHandle addTimer(int64 microseconds,
					TimerHandler * handler, void* arg = NULL);
	INLINE TimerHandle addOnceOffTimer(int64 microseconds,
					TimerHandler * handler, void * arg = NULL);

	uint64 timerDeliveryTime(TimerHandle handle) const;
	uint64 timerIntervalTime(TimerHandle handle) const;
	uint64 & timerIntervalTime(TimerHandle handle);
	
	uint64 getSpareTime() const;
	void clearSpareTime();
	double proportionalSpareTime() const;

	ErrorReporter & errorReporter()	{ return *pErrorReporter_; }

	INLINE EventPoller* createPoller();
	EventPoller* pPoller(){ return pPoller_; }

	int processNetwork(bool shouldIdle);
private:
	TimerHandle addTimerCommon(int64 microseconds,
		TimerHandler * handler,
		void * arg,
		bool recurrent);

	void processFrequentTasks();
	void processTimers();
	void processStats();
	
	double calculateWait() const;
	
	void attachTo(EventDispatcher & parentDispatcher);
	void detachFrom(EventDispatcher & parentDispatcher);
	
	typedef std::vector<EventDispatcher *> ChildDispatchers;
	ChildDispatchers childDispatchers_;

protected:
	int8 breakProcessing_;

	double maxWait_;
	uint32 numTimerCalls_;
	
	// Statistics
	TimeStamp		accSpareTime_;
	TimeStamp		oldSpareTime_;
	TimeStamp		totSpareTime_;
	TimeStamp		lastStatisticsGathered_;
	
	Tasks* pFrequentTasks_;
	ErrorReporter * pErrorReporter_;
	Timers64* pTimers_;
	EventPoller* pPoller_;
};


}
}

#ifdef CODE_INLINE
#include "event_dispatcher.ipp"
#endif
#endif // KBE_EVENT_DISPATCHER_HPP
