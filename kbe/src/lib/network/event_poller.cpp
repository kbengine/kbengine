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


#include "event_poller.hpp"

namespace KBEngine { 
namespace Mercury
{
	
#ifndef _WIN32
#define HAS_EPOLL
#endif

//-------------------------------------------------------------------------------------
EventPoller::EventPoller() : 
	fdReadHandlers_(), 
	fdWriteHandlers_(), 
	spareTime_(0)
{
}


//-------------------------------------------------------------------------------------
EventPoller::~EventPoller()
{
}


//-------------------------------------------------------------------------------------
bool EventPoller::registerForRead(int fd,
		InputNotificationHandler * handler)
{
	if (!this->doRegisterForRead(fd))
	{
		return false;
	}

	fdReadHandlers_[ fd ] = handler;

	return true;
}

//-------------------------------------------------------------------------------------
bool EventPoller::registerForWrite(int fd,
		InputNotificationHandler * handler)
{
	if (!this->doRegisterForWrite(fd))
	{
		return false;
	}

	fdWriteHandlers_[ fd ] = handler;

	return true;
}

//-------------------------------------------------------------------------------------
bool EventPoller::deregisterForRead(int fd)
{
	fdReadHandlers_.erase(fd);

	return this->doDeregisterForRead(fd);
}

//-------------------------------------------------------------------------------------
bool EventPoller::deregisterForWrite(int fd)
{
	fdWriteHandlers_.erase(fd);

	return this->doDeregisterForWrite(fd);
}

//-------------------------------------------------------------------------------------
bool EventPoller::trigger(int fd, FDHandlers & handlers)
{
	FDHandlers::iterator iter = handlers.find(fd);

	if (iter == handlers.end())
	{
		return false;
	}

	iter->second->handleInputNotification(fd);

	return true;
}

//-------------------------------------------------------------------------------------
bool EventPoller::triggerError(int fd)
{
	if (!this->triggerRead(fd))
	{
		return this->triggerWrite(fd);
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool EventPoller::isRegistered(int fd, bool isForRead) const
{
	const FDHandlers & handlers =
		isForRead ? fdReadHandlers_ : fdWriteHandlers_;

	return handlers.find(fd) != handlers.end();
}

//-------------------------------------------------------------------------------------
int EventPoller::handleInputNotification(int fd)
{

	this->processPendingEvents(0.0);

	return 0;
}

//-------------------------------------------------------------------------------------
int EventPoller::getFileDescriptor() const
{
	return -1;
}

//-------------------------------------------------------------------------------------
int EventPoller::maxFD() const
{
	int readMaxFD = EventPoller::maxFD(fdReadHandlers_);
	int writeMaxFD = EventPoller::maxFD(fdWriteHandlers_);
	return std::max(readMaxFD, writeMaxFD);
}

//-------------------------------------------------------------------------------------
int EventPoller::maxFD(const FDHandlers & handlerMap)
{
	int maxFD = -1;
	FDHandlers::const_iterator iFDHandler = handlerMap.begin();
	while (iFDHandler != handlerMap.end())
	{
		if (iFDHandler->first > maxFD)
		{
			maxFD = iFDHandler->first;
		}
		++iFDHandler;
	}
	return maxFD;
}


//-------------------------------------------------------------------------------------
class SelectPoller : public EventPoller
{
public:
	SelectPoller();

protected:
	virtual bool doRegisterForRead(int fd);
	virtual bool doRegisterForWrite(int fd);

	virtual bool doDeregisterForRead(int fd);
	virtual bool doDeregisterForWrite(int fd);

	virtual int processPendingEvents(double maxWait);

private:
	void handleInputNotifications(int &countReady,
		fd_set &readFDs, fd_set &writeFDs);

	void updateLargestFileDescriptor();

	// The file descriptor sets used in the select call.
	fd_set						fdReadSet_;
	fd_set						fdWriteSet_;

	// This is the largest registered file descriptor (read or write). It's used
	// in the call to select.
	int							fdLargest_;

	// This is the number of file descriptors registered for the write event.
	int							fdWriteCount_;
};


//-------------------------------------------------------------------------------------
SelectPoller::SelectPoller() :
	EventPoller(),
	fdReadSet_(),
	fdWriteSet_(),
	fdLargest_(-1),
	fdWriteCount_(0)
{
	// set up our list of file descriptors
	FD_ZERO(&fdReadSet_);
	FD_ZERO(&fdWriteSet_);
}

//-------------------------------------------------------------------------------------
void SelectPoller::updateLargestFileDescriptor()
{
	fdLargest_ = this->maxFD();
}

//-------------------------------------------------------------------------------------
void SelectPoller::handleInputNotifications(int &countReady,
	fd_set &readFDs, fd_set &writeFDs)
{
#ifdef _WIN32
	// X360 fd_sets don't look like POSIX ones, we know exactly what they are
	// and can just iterate over the provided FD arrays

	for (unsigned i=0; i < readFDs.fd_count; i++)
	{
		int fd = readFDs.fd_array[ i ];
		--countReady;
		this->triggerRead(fd);
	}

	for (unsigned i=0; i < writeFDs.fd_count; i++)
	{
		int fd = writeFDs.fd_array[ i ];
		--countReady;
		this->triggerWrite(fd);
	}

#else
	// POSIX fd_sets are more opaque and we just have to count up blindly until
	// we hit valid FD's with FD_ISSET

	for (int fd = 0; fd <= fdLargest_ && countReady > 0; ++fd)
	{
		if (FD_ISSET(fd, &readFDs))
		{
			--countReady;
			this->triggerRead(fd);
		}

		if (FD_ISSET(fd, &writeFDs))
		{
			--countReady;
			this->triggerWrite(fd);
		}
	}
#endif
}

//-------------------------------------------------------------------------------------
int SelectPoller::processPendingEvents(double maxWait)
{
	fd_set	readFDs;
	fd_set	writeFDs;
	struct timeval		nextTimeout;
	// Is this needed?
	FD_ZERO(&readFDs);
	FD_ZERO(&writeFDs);

	readFDs = fdReadSet_;
	writeFDs = fdWriteSet_;

	nextTimeout.tv_sec = (int)maxWait;
	nextTimeout.tv_usec =
		(int)((maxWait - (double)nextTimeout.tv_sec) * 1000000.0);

	uint64 startTime = timestamp();
	KBEConcurrency::startMainThreadIdling();

	int countReady = 0;

#ifdef _WIN32
	if (fdLargest_ == -1)
	{
		// Windows can't handle it if we don't have any FDs to select on, but
		// we have a non-NULL timeout.
		Sleep(int(maxWait * 1000.0));
	}
	else
#endif
	{
		countReady = select(fdLargest_+1, &readFDs,
				fdWriteCount_ ? &writeFDs : NULL, NULL, &nextTimeout);
	}

	KBEConcurrency::endMainThreadIdling();

	spareTime_ += timestamp() - startTime;

	if (countReady > 0)
	{
		this->handleInputNotifications(countReady, readFDs, writeFDs);
	}
	else if (countReady == -1)
	{
		// TODO: Clean this up on shutdown
		// if (!breakProcessing_)
		{
			WARNING_MSG(boost::format("EventDispatcher::processContinuously: "
				"error in select(): %1%\n") % kbe_strerror());
		}
	}

	return countReady;
}

//-------------------------------------------------------------------------------------
bool SelectPoller::doRegisterForRead(int fd)
{
#ifndef _WIN32
	if ((fd < 0) || (FD_SETSIZE <= fd))
	{
		ERROR_MSG(boost::format("EventDispatcher::registerFileDescriptor: "
			"Tried to register invalid fd %1%. FD_SETSIZE (%2%)\n") %
			fd % FD_SETSIZE);

		return false;
	}
#endif

	// Bail early if it's already in the read set
	if (FD_ISSET(fd, &fdReadSet_))
		return false;

	FD_SET(fd, &fdReadSet_);

	if (fd > fdLargest_)
	{
		fdLargest_ = fd;
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool SelectPoller::doRegisterForWrite(int fd)
{
#ifndef _WIN32
	if ((fd < 0) || (FD_SETSIZE <= fd))
	{
		ERROR_MSG(boost::format("EventDispatcher::registerWriteFileDescriptor: "
			"Tried to register invalid fd %1%. FD_SETSIZE (%2%)\n") %
			fd % FD_SETSIZE);

		return false;
	}
#endif

	if (FD_ISSET(fd, &fdWriteSet_))
	{
		return false;
	}

	FD_SET(fd, &fdWriteSet_);

	if (fd > fdLargest_)
	{
		fdLargest_ = fd;
	}

	++fdWriteCount_;
	return true;
}

//-------------------------------------------------------------------------------------
bool SelectPoller::doDeregisterForRead(int fd)
{
#ifndef _WIN32
	if ((fd < 0) || (FD_SETSIZE <= fd))
	{
		return false;
	}
#endif

	if (!FD_ISSET(fd, &fdReadSet_))
	{
		return false;
	}

	FD_CLR(fd, &fdReadSet_);

	if (fd == fdLargest_)
	{
		this->updateLargestFileDescriptor();
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool SelectPoller::doDeregisterForWrite(int fd)
{
#ifndef _WIN32
	if ((fd < 0) || (FD_SETSIZE <= fd))
	{
		return false;
	}
#endif

	if (!FD_ISSET(fd, &fdWriteSet_))
	{
		return false;
	}

	FD_CLR(fd, &fdWriteSet_);

	if (fd == fdLargest_)
	{
		this->updateLargestFileDescriptor();
	}

	--fdWriteCount_;
	return true;
}


//-------------------------------------------------------------------------------------
#ifdef HAS_EPOLL
#include <sys/epoll.h>


/**
 *	This class is an EventPoller that uses epoll.
 */
class EPoller : public EventPoller
{
public:
	EPoller(int expectedSize = 10);
	virtual ~EPoller();

	int getFileDescriptor() const { return epfd_; }

protected:
	virtual bool doRegisterForRead(int fd)
		{ return this->doRegister(fd, true, true); }

	virtual bool doRegisterForWrite(int fd)
		{ return this->doRegister(fd, false, true); }

	virtual bool doDeregisterForRead(int fd)
		{ return this->doRegister(fd, true, false); }

	virtual bool doDeregisterForWrite(int fd)
		{ return this->doRegister(fd, false, false); }

	virtual int processPendingEvents(double maxWait);

	bool doRegister(int fd, bool isRead, bool isRegister);

private:
	// epoll file descriptor
	int epfd_;
};

//-------------------------------------------------------------------------------------
EPoller::EPoller(int expectedSize) :
	epfd_(epoll_create(expectedSize))
{
	if (epfd_ == -1)
	{
		ERROR_MSG(boost::format("EPoller::EPoller: epoll_create failed: %1%\n") %
				kbe_strerror());
	}
};

//-------------------------------------------------------------------------------------
EPoller::~EPoller()
{
	if (epfd_ != -1)
	{
		close(epfd_);
	}
}

//-------------------------------------------------------------------------------------
bool EPoller::doRegister(int fd, bool isRead, bool isRegister)
{
	struct epoll_event ev;
	memset(&ev, 0, sizeof(ev)); // stop valgrind warning
	int op;

	ev.data.fd = fd;

	// Handle the case where the file is already registered for the opposite
	// action.
	if (this->isRegistered(fd, !isRead))
	{
		op = EPOLL_CTL_MOD;

		ev.events = isRegister ? EPOLLIN|EPOLLOUT :
					isRead ? EPOLLOUT : EPOLLIN;
	}
	else
	{
		// TODO: Could be good to use EPOLLET (leave like select for now).
		ev.events = isRead ? EPOLLIN : EPOLLOUT;
		op = isRegister ? EPOLL_CTL_ADD : EPOLL_CTL_DEL;
	}

	if (epoll_ctl(epfd_, op, fd, &ev) < 0)
	{
		const char* MESSAGE = "EPoller::doRegister: Failed to %s %s file "
				"descriptor %d (%s)\n";
		if (errno == EBADF)
		{
			WARNING_MSG(boost::format(MESSAGE) %
					(isRegister ? "add" : "remove") %
					(isRead ? "read" : "write") %
					fd %
					kbe_strerror());
		}
		else
		{
			ERROR_MSG(boost::format(MESSAGE) %
					(isRegister ? "add" : "remove") %
					(isRead ? "read" : "write") %
					fd %
					kbe_strerror());
		}

		return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------
int EPoller::processPendingEvents(double maxWait)
{
	const int MAX_EVENTS = 10;
	struct epoll_event events[ MAX_EVENTS ];
	int maxWaitInMilliseconds = int(ceil(maxWait * 1000));
	uint64 startTime = timestamp();

	KBEConcurrency::startMainThreadIdling();
	int nfds = epoll_wait(epfd_, events, MAX_EVENTS, maxWaitInMilliseconds);
	KBEConcurrency::endMainThreadIdling();


	spareTime_ += timestamp() - startTime;

	for (int i = 0; i < nfds; ++i)
	{
		if (events[i].events & (EPOLLERR|EPOLLHUP))
		{
			this->triggerError(events[i].data.fd);
		}
		else
		{
			if (events[i].events & EPOLLIN)
			{
				this->triggerRead(events[i].data.fd);
			}

			if (events[i].events & EPOLLOUT)
			{
				this->triggerWrite(events[i].data.fd);
			}
		}
	}

	return nfds;
}

#endif // HAS_EPOLL

//-------------------------------------------------------------------------------------
EventPoller * EventPoller::create()
{
#ifdef HAS_EPOLL
	return new EPoller();
#else
	return new SelectPoller();
#endif // HAS_EPOLL
}

}
}
