/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 kbegine Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
*/
namespace KBEngine { 

template<class TIME_STAMP>
TimesT<TIME_STAMP>::TimesT():
	m_timeQueue_(),
	m_pProcessingNode_( NULL ),
	m_lastProcessTime_( 0 ),
	m_numCancelled_( 0 )
{
}

template<class TIME_STAMP>
TimesT<TIME_STAMP>::~TimesT()
{
	this->clear();
}

template <class TIME_STAMP>
TimerHandle TimesT< TIME_STAMP >::add( TimeStamp startTime,
		TimeStamp interval, TimerHandler * pHandler, void * pUser )
{
	Time * pTime = new Time( *this, startTime, interval, pHandler, pUser );
	m_timeQueue_.push( pTime );
	return TimerHandle( pTime );
}

template <class TIME_STAMP>
void TimesT< TIME_STAMP >::onCancel()
{
	++m_numCancelled_;

	// If there are too many cancelled timers in the queue (more than half),
	// these are flushed from the queue immediately.

	if (m_numCancelled_ * 2 > int(m_timeQueue_.size()))
	{
		this->purgeCancelledTimes();
	}
}

template <class TIME_STAMP>
void TimesT< TIME_STAMP >::clear(bool shouldCallCancel)
{
	int maxLoopCount = m_timeQueue_.size();

	while (!m_timeQueue_.empty())
	{
		Time * pTime = m_timeQueue_.unsafePopBack();
		if (!pTime->isCancelled() && shouldCallCancel)
		{
			--m_numCancelled_;
			pTime->cancel();

			if (--maxLoopCount == 0)
			{
				shouldCallCancel = false;
			}
		}
		else if (pTime->isCancelled())
		{
			--m_numCancelled_;
		}

		delete pTime;
	}

	m_numCancelled_ = 0;
	m_timeQueue_ = PriorityQueue();
}

template <class TIME>
class IsNotCancelled
{
public:
	bool operator()( const TIME * pTime )
	{
		return !pTime->isCancelled();
	}
};

template <class TIME_STAMP>
void TimesT< TIME_STAMP >::purgeCancelledTimes()
{
	typename PriorityQueue::Container & container = m_timeQueue_.container();
	typename PriorityQueue::Container::iterator newEnd =
		std::partition( container.begin(), container.end(),
			IsNotCancelled< Time >() );

	for (typename PriorityQueue::Container::iterator iter = newEnd;
		iter != container.end();
		++iter)
	{
		delete *iter;
	}

	const int numPurged = (container.end() - newEnd);
	m_numCancelled_ -= numPurged;
	KBE_ASSERT( (m_numCancelled_ == 0) || (m_numCancelled_ == 1) );
	
	container.erase( newEnd, container.end() );
	m_timeQueue_.heapify();
}

template <class TIME_STAMP>
int TimesT< TIME_STAMP >::process(TimeStamp now)
{
	int numFired = 0;

	while ((!m_timeQueue_.empty()) && (
		m_timeQueue_.top()->time() <= now ||
		m_timeQueue_.top()->isCancelled()))
	{
		Time * pTime = m_pProcessingNode_ = m_timeQueue_.top();
		m_timeQueue_.pop();

		if (!pTime->isCancelled())
		{
			++numFired;
			pTime->triggerTimer();
		}

		if (!pTime->isCancelled())
		{
			m_timeQueue_.push( pTime );
		}
		else
		{
			delete pTime;

			KBE_ASSERT( m_numCancelled_ > 0 );
			--m_numCancelled_;
		}
	}

	m_pProcessingNode_ = NULL;
	m_lastProcessTime_ = now;
	return numFired;
}

template <class TIME_STAMP>
bool TimesT< TIME_STAMP >::legal(TimerHandle handle) const
{
	typedef Time * const * TimeIter;
	Time * pTime = static_cast< Time* >( handle.time() );

	if (pTime == NULL)
	{
		return false;
	}

	if (pTime == m_pProcessingNode_)
	{
		return true;
	}

	TimeIter begin = &m_timeQueue_.top();
	TimeIter end = begin + m_timeQueue_.size();

	for (TimeIter it = begin; it != end; it++)
	{
		if (*it == pTime)
		{
			return true;
		}
	}

	return false;
}

template <class TIME_STAMP>
TIME_STAMP TimesT< TIME_STAMP >::nextExp(TimeStamp now) const
{
	if (m_timeQueue_.empty() ||
		now > m_timeQueue_.top()->time())
	{
		return 0;
	}

	return m_timeQueue_.top()->time() - now;
}

template <class TIME_STAMP>
bool TimesT< TIME_STAMP >::getTimerInfo( TimerHandle handle,
					TimeStamp &			time,
					TimeStamp &			interval,
					void * &			pUser ) const
{
	Time * pTime = static_cast< Time * >( handle.time() );

	if (!pTime->isCancelled())
	{
		time = pTime->time();
		interval = pTime->interval();
		pUser = pTime->pUserData();

		return true;
	}

	return false;
}

template <class TIME_STAMP>
TIME_STAMP
	TimesT< TIME_STAMP >::timerDeliveryTime(TimerHandle handle) const
{
	Time * pTime = static_cast< Time * >( handle.time() );
	return pTime->deliveryTime();
}

template <class TIME_STAMP>
TIME_STAMP
	TimesT< TIME_STAMP >::timerIntervalTime(TimerHandle handle) const
{
	Time * pTime = static_cast< Time * >( handle.time() );
	return pTime->interval();
}

template <class TIME_STAMP>
TIME_STAMP & TimesT< TIME_STAMP >::timerIntervalTime( TimerHandle handle )
{
	Time * pTime = static_cast< Time * >( handle.time() );
	return pTime->intervalRef();
}


inline TimeBase::TimeBase(TimesBase & owner, TimerHandler * pHandler, void * pUserData) :
	m_owner_( owner ),
	m_pHandler_( pHandler ),
	m_pUserData_(pUserData),
	m_state_(TIME_PENDING )
{
	pHandler->incTimerRegisterCount();
}

inline void TimeBase::cancel()
{
	if (this->isCancelled()){
		return;
	}

	KBE_ASSERT( (m_state_ == TIME_PENDING) || (m_state_ == TIME_EXECUTING) );
	m_state_ = TIME_CANCELLED;

	if (m_pHandler_){
		m_pHandler_->release(TimerHandle(this), m_pUserData_);
		m_pHandler_ = NULL;
	}

	m_owner_.onCancel();
}


template <class TIME_STAMP>
TimesT< TIME_STAMP >::Time::Time( TimesBase & owner,
		TimeStamp startTime, TimeStamp interval,
		TimerHandler * _pHandler, void * _pUser ) :
	TimeBase( owner, _pHandler, _pUser ),
	m_time_( startTime ),
	m_interval_( interval )
{
}

template <class TIME_STAMP>
TIME_STAMP TimesT< TIME_STAMP >::Time::deliveryTime() const
{
	return this->isExecuting() ?  (m_time_ + m_interval_) : m_time_;
}


template <class TIME_STAMP>
void TimesT< TIME_STAMP >::Time::triggerTimer()
{
	if (!this->isCancelled())
	{
		m_state_ = TIME_EXECUTING;

		m_pHandler_->handleTimeout( TimerHandle( this ), m_pUserData_ );

		if ((m_interval_ == 0) && !this->isCancelled())
		{
			this->cancel();
		}
	}

	if (!this->isCancelled())
	{
		m_time_ += m_interval_;
		m_state_ = TIME_PENDING;
	}
}

}