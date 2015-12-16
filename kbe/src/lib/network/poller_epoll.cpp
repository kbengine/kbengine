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


#include "poller_epoll.h"
#include "helper/profile.h"

namespace KBEngine { 

#ifdef HAS_EPOLL
#include <sys/epoll.h>
ProfileVal g_idleProfile("Idle");

namespace Network
{
	
//-------------------------------------------------------------------------------------
EpollPoller::EpollPoller(int expectedSize) :
	epfd_(epoll_create(expectedSize))
{
	if (epfd_ == -1)
	{
		ERROR_MSG(fmt::format("EpollPoller::EpollPoller: epoll_create failed: {}\n",
				kbe_strerror()));
	}
};

//-------------------------------------------------------------------------------------
EpollPoller::~EpollPoller()
{
	if (epfd_ != -1)
	{
		close(epfd_);
	}
}

//-------------------------------------------------------------------------------------
bool EpollPoller::doRegister(int fd, bool isRead, bool isRegister)
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
		const char* MESSAGE = "EpollPoller::doRegister: Failed to {} {} file "
				"descriptor {} ({})\n";
		if (errno == EBADF)
		{
			WARNING_MSG(fmt::format(MESSAGE,
					(isRegister ? "add" : "remove"),
					(isRead ? "read" : "write"),
					fd,
					kbe_strerror()));
		}
		else
		{
			ERROR_MSG(fmt::format(MESSAGE,
					(isRegister ? "add" : "remove"),
					(isRead ? "read" : "write"),
					fd,
					kbe_strerror()));
		}

		return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------
int EpollPoller::processPendingEvents(double maxWait)
{
	const int MAX_EVENTS = 10;
	struct epoll_event events[ MAX_EVENTS ];
	int maxWaitInMilliseconds = int(ceil(maxWait * 1000));

#if ENABLE_WATCHERS
	g_idleProfile.start();
#else
	uint64 startTime = timestamp();
#endif

	KBEConcurrency::onStartMainThreadIdling();
	int nfds = epoll_wait(epfd_, events, MAX_EVENTS, maxWaitInMilliseconds);
	KBEConcurrency::onEndMainThreadIdling();


#if ENABLE_WATCHERS
	g_idleProfile.stop();
	spareTime_ += g_idleProfile.lastTime_;
#else
	spareTime_ += timestamp() - startTime;
#endif

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

}

#endif // HAS_EPOLL

}
