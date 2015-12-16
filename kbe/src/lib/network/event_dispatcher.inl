/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2016 KBEngine.

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
