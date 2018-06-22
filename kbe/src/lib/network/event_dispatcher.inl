// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

namespace KBEngine { 
namespace Network
{
INLINE TimerHandle EventDispatcher::addTimer(int64 microseconds,
	TimerHandler * handler, void * arg)
{
	return this->addTimerCommon(microseconds, handler, arg, true);
}

INLINE void EventDispatcher::breakProcessing(bool breakState)
{
	if(breakState)
		breakProcessing_ = EVENT_DISPATCHER_STATUS_BREAK_PROCESSING;
	else
		breakProcessing_ = EVENT_DISPATCHER_STATUS_RUNNING;
}

INLINE void EventDispatcher::setWaitBreakProcessing()
{
	breakProcessing_ = EVENT_DISPATCHER_STATUS_WAITING_BREAK_PROCESSING;
}

INLINE bool EventDispatcher::hasBreakProcessing() const 
{ 
	return breakProcessing_ == EVENT_DISPATCHER_STATUS_BREAK_PROCESSING; 
}

INLINE bool EventDispatcher::waitingBreakProcessing() const 
{ 
	return breakProcessing_ == EVENT_DISPATCHER_STATUS_WAITING_BREAK_PROCESSING; 
}

INLINE double EventDispatcher::maxWait() const
{
	return maxWait_;
}

INLINE void EventDispatcher::maxWait(double seconds)
{
	maxWait_ = seconds;
}

} // namespace Network
}
// event_dispatcher.inl
