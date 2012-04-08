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
inline void EventDispatcher::breakProcessing( bool breakState )
{
	m_breakProcessing_ = breakState;
}

inline bool EventDispatcher::processingBroken() const
{
	return m_breakProcessing_;
}

inline double EventDispatcher::maxWait() const
{
	return m_maxWait_;
}

inline void EventDispatcher::maxWait(double seconds)
{
	m_maxWait_ = seconds;
}

} // namespace Mercury
}
// event_dispatcher.ipp
