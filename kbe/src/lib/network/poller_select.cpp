// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#include "poller_select.h"
#include "helper/profile.h"

namespace KBEngine { 

#ifndef HAS_EPOLL

ProfileVal g_idleProfile("Idle");

namespace Network
{
	
//-------------------------------------------------------------------------------------
SelectPoller::SelectPoller() :
	EventPoller(),
	fdReadSet_(),
	fdWriteSet_(),
	fdLargest_(-1),
	fdWriteCount_(0)
{
	FD_ZERO(&fdReadSet_);
	FD_ZERO(&fdWriteSet_);
}

//-------------------------------------------------------------------------------------
void SelectPoller::handleNotifications(int &countReady,
	fd_set &readFDs, fd_set &writeFDs)
{
#if KBE_PLATFORM == PLATFORM_WIN32
	for (unsigned i=0; i < readFDs.fd_count; ++i)
	{
		int fd = readFDs.fd_array[ i ];
		--countReady;
		this->triggerRead(fd);
	}

	for (unsigned i=0; i < writeFDs.fd_count; ++i)
	{
		int fd = writeFDs.fd_array[ i ];
		--countReady;
		this->triggerWrite(fd);
	}

#else
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

	FD_ZERO(&readFDs);
	FD_ZERO(&writeFDs);

	readFDs = fdReadSet_;
	writeFDs = fdWriteSet_;

	nextTimeout.tv_sec = (int)maxWait;
	nextTimeout.tv_usec =
		(int)((maxWait - (double)nextTimeout.tv_sec) * 1000000.0);

#if ENABLE_WATCHERS
	g_idleProfile.start();
#else
	uint64 startTime = timestamp();
#endif

	KBEConcurrency::onStartMainThreadIdling();

	int countReady = 0;

#if KBE_PLATFORM == PLATFORM_WIN32
	if (fdLargest_ == -1)
	{
		Sleep(int(maxWait * 1000.0));
	}
	else
#endif
	{
		countReady = select(fdLargest_+1, &readFDs,
				fdWriteCount_ ? &writeFDs : NULL, NULL, &nextTimeout);
	}

	KBEConcurrency::onEndMainThreadIdling();

#if ENABLE_WATCHERS
	g_idleProfile.stop();
	spareTime_ += g_idleProfile.lastTime_;
#else
	spareTime_ += timestamp() - startTime;
#endif
	
	if (countReady > 0)
	{
		this->handleNotifications(countReady, readFDs, writeFDs);
	}
	else if (countReady == -1)
	{
		WARNING_MSG(fmt::format("EventDispatcher::processContinuously: "
			"error in select(): {}\n", kbe_strerror()));
	}

	return countReady;
}

//-------------------------------------------------------------------------------------
bool SelectPoller::doRegisterForRead(int fd)
{
#if KBE_PLATFORM != PLATFORM_WIN32
	if ((fd < 0) || (FD_SETSIZE <= fd))
	{
		ERROR_MSG(fmt::format("SelectPoller::doRegisterForRead: "
			"Tried to register invalid fd {}. FD_SETSIZE ({})\n",
			fd, FD_SETSIZE));

		return false;
	}
#else
	if (fdReadSet_.fd_count >= FD_SETSIZE)
	{
		ERROR_MSG(fmt::format("SelectPoller::doRegisterForRead: "
			"Tried to register invalid fd {}. FD_SETSIZE ({})\n",
			fd, FD_SETSIZE));

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
#if KBE_PLATFORM != PLATFORM_WIN32
	if ((fd < 0) || (FD_SETSIZE <= fd))
	{
		ERROR_MSG(fmt::format("SelectPoller::doRegisterForWrite: "
			"Tried to register invalid fd {}. FD_SETSIZE ({})\n",
			fd, FD_SETSIZE));

		return false;
	}
#else
	if (fdWriteSet_.fd_count >= FD_SETSIZE)
	{
		ERROR_MSG(fmt::format("SelectPoller::doRegisterForWrite: "
			"Tried to register invalid fd {}. FD_SETSIZE ({})\n",
			fd, FD_SETSIZE));

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
#if KBE_PLATFORM != PLATFORM_WIN32
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
		fdLargest_ = this->maxFD();
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool SelectPoller::doDeregisterForWrite(int fd)
{
#if KBE_PLATFORM != PLATFORM_WIN32
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
		fdLargest_ = this->maxFD();
	}

	--fdWriteCount_;
	return true;
}

#endif // HAS_EPOLL

}
}
