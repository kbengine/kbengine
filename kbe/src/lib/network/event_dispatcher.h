// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_EVENT_DISPATCHER_H
#define KBE_EVENT_DISPATCHER_H

#include <map>
#include "common/tasks.h"
#include "common/timer.h"
#include "network/interfaces.h"
#include "network/common.h"

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
	
	int  processOnce(bool shouldIdle = false);
	void processUntilBreak();

	INLINE bool hasBreakProcessing() const;
	INLINE bool waitingBreakProcessing() const;

	void breakProcessing(bool breakState = true);
	INLINE void setWaitBreakProcessing();
	
	void addTask(Task * pTask);
	bool cancelTask(Task * pTask);
	
	INLINE double maxWait() const;
	INLINE void maxWait(double seconds);

	bool registerReadFileDescriptor(int fd, InputNotificationHandler * handler);
	bool deregisterReadFileDescriptor(int fd);
	bool registerWriteFileDescriptor(int fd, OutputNotificationHandler * handler);
	bool deregisterWriteFileDescriptor(int fd);

	INLINE TimerHandle addTimer(int64 microseconds,
					TimerHandler * handler, void* arg = NULL);

	uint64 getSpareTime() const;
	void clearSpareTime();

	ErrorReporter & errorReporter()	{ return *pErrorReporter_; }

	INLINE EventPoller* createPoller();
	EventPoller* pPoller(){ return pPoller_; }

	int processNetwork(bool shouldIdle);
private:
	TimerHandle addTimerCommon(int64 microseconds,
		TimerHandler * handler,
		void * arg,
		bool recurrent);

	void processTasks();
	void processTimers();
	void processStats();
	
	double calculateWait() const;
	
protected:
	int8 breakProcessing_;

	double maxWait_;
	uint32 numTimerCalls_;
	
	// Statistics
	TimeStamp		accSpareTime_;
	TimeStamp		oldSpareTime_;
	TimeStamp		totSpareTime_;
	TimeStamp		lastStatisticsGathered_;
	
	Tasks* pTasks_;
	ErrorReporter * pErrorReporter_;
	Timers64* pTimers_;
	EventPoller* pPoller_;
};


}
}

#ifdef CODE_INLINE
#include "event_dispatcher.inl"
#endif
#endif // KBE_EVENT_DISPATCHER_H
