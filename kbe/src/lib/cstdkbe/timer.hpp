/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 kbegine Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
*/

#ifndef __TIMER__
#define __TIMER__
#include "cstdkbe/cstdkbe.hpp"
#include "helper/debug_helper.hpp"

namespace KBEngine
{
class TimersBase;
class TimerBase;

class TimerHandle
{
public:
	explicit TimerHandle(TimerBase * pTimer = NULL) : m_pTimer_( pTimer ) {}

	void cancel();
	void clearWithoutCancel()	{ m_pTimer_ = NULL; }

	bool isSet() const		{ return m_pTimer_ != NULL; }

	friend bool operator==( TimerHandle h1, TimerHandle h2 );
	TimerBase * getTimer() const	{ return m_pTimer_; }

private:
	TimerBase * m_pTimer_;
};

inline bool operator==( TimerHandle h1, TimerHandle h2 )
{
	return h1.m_pTimer_ == h2.m_pTimer_;
}


/**
 *	This is an interface which must be derived from in order to
 *	receive time queue events.
 */
class TimerHandler
{
public:
	TimerHandler() : m_numTimesRegistered_( 0 ) {}
	virtual ~TimerHandler()
	{
		KBE_ASSERT( m_numTimesRegistered_ == 0 );
	};

	virtual void handleTimeout(TimerHandle handle, void * pUser) = 0;

protected:
	virtual void onRelease( TimerHandle handle, void * pUser ) {}

private:
	friend class TimerBase;
	void incTimerRegisterCount() { ++m_numTimesRegistered_; }
	void decTimerRegisterCount() { --m_numTimesRegistered_; }
	void release( TimerHandle handle, void * pUser )
	{
		this->decTimerRegisterCount();
		this->onRelease( handle, pUser );
	}

	int m_numTimesRegistered_;
};

class TimerBase
{
public:
	TimerBase(TimersBase &owner, void * pUserData):m_pUserData_(pUserData)
	{
	}
	
	void cancel();
	void * getUserData()const	{ return m_pUserData_; }
	bool isCancelled()const{ return m_state_ == TIMER_CANCELLED; }
	bool isExecuting()const{ return m_state_ == TIMER_EXECUTING; }
protected:
	enum TimerState
	{
		TIMER_PENDING,
		TIMER_EXECUTING,
		TIMER_CANCELLED
	};
	
	void *m_pUserData_;
	TimerState m_state_;
};

class TimersBase
{
public:
	virtual void onCancel() = 0;
};

template<class TIME_STAMP>
class TimersT : public TimersBase
{
public:
	typedef TIME_STAMP TimeStamp;

	TimersT();
	~TimersT();

private:
	
	typedef std::vector<KBEngine::TimerBase *> Container;
	Container container_;
};

}

#endif // __TASKS__
