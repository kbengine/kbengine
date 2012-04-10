/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 kbegine Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
*/
namespace KBEngine { 
namespace Mercury
{
INLINE TimerHandle EventDispatcher::addTimer(int64 microseconds,
	TimerHandler * handler, void * arg)
{
	return this->addTimerCommon(microseconds, handler, arg, true);
}

INLINE TimerHandle EventDispatcher::addOnceOffTimer(int64 microseconds,
	TimerHandler * handler, void * arg)
{
	return this->addTimerCommon(microseconds, handler, arg, false);
}

INLINE void EventDispatcher::breakProcessing(bool breakState)
{
	m_breakProcessing_ = breakState;
}

INLINE bool EventDispatcher::processingBroken() const
{
	return m_breakProcessing_;
}

INLINE double EventDispatcher::maxWait() const
{
	return m_maxWait_;
}

INLINE void EventDispatcher::maxWait(double seconds)
{
	m_maxWait_ = seconds;
}

} // namespace Mercury
}
// event_dispatcher.ipp
