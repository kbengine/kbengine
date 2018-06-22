// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

namespace KBEngine { 

template<class TIME_STAMP>
TimersT<TIME_STAMP>::TimersT():
	timeQueue_(),
	pProcessingNode_( NULL ),
	lastProcessTime_( 0 ),
	numCancelled_( 0 )
{
}

template<class TIME_STAMP>
TimersT<TIME_STAMP>::~TimersT()
{
	this->clear();
}

template <class TIME_STAMP>
TimerHandle TimersT< TIME_STAMP >::add( TimeStamp startTime,
		TimeStamp interval, TimerHandler * pHandler, void * pUser )
{
	Time * pTime = new Time( *this, startTime, interval, pHandler, pUser );
	timeQueue_.push( pTime );
	return TimerHandle( pTime );
}

template <class TIME_STAMP>
void TimersT< TIME_STAMP >::onCancel()
{
	++numCancelled_;

	// If there are too many cancelled timers in the queue (more than half),
	// these are flushed from the queue immediately.

	if (numCancelled_ * 2 > int(timeQueue_.size()))
	{
		this->purgeCancelledTimes();
	}
}

template <class TIME_STAMP>
void TimersT< TIME_STAMP >::clear(bool shouldCallCancel)
{
	int maxLoopCount = (int)timeQueue_.size();

	while (!timeQueue_.empty())
	{
		Time * pTime = timeQueue_.unsafePopBack();
		if (!pTime->isCancelled() && shouldCallCancel)
		{
			--numCancelled_;
			pTime->cancel();

			if (--maxLoopCount == 0)
			{
				shouldCallCancel = false;
			}
		}
		else if (pTime->isCancelled())
		{
			--numCancelled_;
		}

		delete pTime;
	}

	numCancelled_ = 0;
	timeQueue_ = PriorityQueue();
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
void TimersT< TIME_STAMP >::purgeCancelledTimes()
{
	typename PriorityQueue::Container & container = timeQueue_.container();
	typename PriorityQueue::Container::iterator newEnd =
		std::partition( container.begin(), container.end(),
			IsNotCancelled< Time >() );

	for (typename PriorityQueue::Container::iterator iter = newEnd;
		iter != container.end();
		++iter)
	{
		delete *iter;
	}

	const int numPurged = (int)(container.end() - newEnd);
	numCancelled_ -= numPurged;
	KBE_ASSERT( (numCancelled_ == 0) || (numCancelled_ == 1) );
	
	container.erase( newEnd, container.end() );
	timeQueue_.make_heap();
}

template <class TIME_STAMP>
int TimersT< TIME_STAMP >::process(TimeStamp now)
{
	int numFired = 0;

	while ((!timeQueue_.empty()) && (
		timeQueue_.top()->time() <= now ||
		timeQueue_.top()->isCancelled()))
	{
		Time * pTime = pProcessingNode_ = timeQueue_.top();
		timeQueue_.pop();

		if (!pTime->isCancelled())
		{
			++numFired;
			pTime->triggerTimer();
		}

		if (!pTime->isCancelled())
		{
			timeQueue_.push( pTime );
		}
		else
		{
			delete pTime;

			KBE_ASSERT( numCancelled_ > 0 );
			--numCancelled_;
		}
	}

	pProcessingNode_ = NULL;
	lastProcessTime_ = now;
	return numFired;
}

template <class TIME_STAMP>
bool TimersT< TIME_STAMP >::legal(TimerHandle handle) const
{
	typedef Time * const * TimeIter;
	Time * pTime = static_cast< Time* >( handle.time() );

	if (pTime == NULL)
	{
		return false;
	}

	if (pTime == pProcessingNode_)
	{
		return true;
	}

	TimeIter begin = &timeQueue_.top();
	TimeIter end = begin + timeQueue_.size();

	for (TimeIter it = begin; it != end; ++it)
	{
		if (*it == pTime)
		{
			return true;
		}
	}

	return false;
}

template <class TIME_STAMP>
TIME_STAMP TimersT< TIME_STAMP >::nextExp(TimeStamp now) const
{
	if (timeQueue_.empty() ||
		now > timeQueue_.top()->time())
	{
		return 0;
	}

	return timeQueue_.top()->time() - now;
}

template <class TIME_STAMP>
bool TimersT< TIME_STAMP >::getTimerInfo( TimerHandle handle,
					TimeStamp &			time,
					TimeStamp &			interval,
					void * &			pUser ) const
{
	Time * pTime = static_cast< Time * >( handle.time() );

	if (!pTime->isCancelled())
	{
		time = pTime->time();
		interval = pTime->interval();
		pUser = pTime->getUserData();

		return true;
	}

	return false;
}


inline TimeBase::TimeBase(TimersBase & owner, TimerHandler * pHandler, void * pUserData) :
	owner_(owner),
	pHandler_(pHandler),
	pUserData_(pUserData),
	state_(TIME_PENDING)
{
	pHandler->incTimerRegisterCount();
}

inline void TimeBase::cancel()
{
	if (this->isCancelled()){
		return;
	}

	KBE_ASSERT((state_ == TIME_PENDING) || (state_ == TIME_EXECUTING));
	state_ = TIME_CANCELLED;

	if (pHandler_){
		pHandler_->release(TimerHandle(this), pUserData_);
		pHandler_ = NULL;
	}

	owner_.onCancel();
}


template <class TIME_STAMP>
TimersT< TIME_STAMP >::Time::Time( TimersBase & owner,
		TimeStamp startTime, TimeStamp interval,
		TimerHandler * _pHandler, void * _pUser ) :
	TimeBase(owner, _pHandler, _pUser),
	time_(startTime),
	interval_(interval)
{
}

template <class TIME_STAMP>
void TimersT< TIME_STAMP >::Time::triggerTimer()
{
	if (!this->isCancelled())
	{
		state_ = TIME_EXECUTING;

		pHandler_->handleTimeout( TimerHandle( this ), pUserData_ );

		if ((interval_ == 0) && !this->isCancelled())
		{
			this->cancel();
		}
	}

	if (!this->isCancelled())
	{
		time_ += interval_;
		state_ = TIME_PENDING;
	}
}

}
