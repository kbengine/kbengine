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

#ifndef KBE_EPOLL_POLLER_H
#define KBE_EPOLL_POLLER_H

#include "event_poller.h"

#if KBE_PLATFORM != PLATFORM_WIN32
#define HAS_EPOLL
#endif

namespace KBEngine { 
namespace Network
{

#ifdef HAS_EPOLL
class EpollPoller : public EventPoller
{
public:
	EpollPoller(int expectedSize = 10);
	virtual ~EpollPoller();

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

	int epfd_;
};
#endif // HAS_EPOLL

}
}
#endif // KBE_EPOLL_POLLER_H
