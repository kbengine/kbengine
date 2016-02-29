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
/*
Windows Platform Support I/O Completion Port
written by Yu-T,2016-2-24
*/
#ifndef KBE_IOCP_POLLER_H
#define KBE_IOCP_POLLER_H

#include "event_poller.h"

namespace KBEngine {
	namespace Network
	{

#ifndef HAS_EPOLL
#ifdef USE_IOCP
		class IocpPoller : public EventPoller
		{
		public:
			IocpPoller();

		protected:
			virtual bool doRegisterForRead(int fd);
			virtual bool doRegisterForWrite(int fd);

			virtual bool doDeregisterForRead(int fd);
			virtual bool doDeregisterForWrite(int fd);

			virtual int processPendingEvents(double maxWait);

		private:
			void handleNotifications(int &countReady,
				fd_set &readFDs, fd_set &writeFDs);

			fd_set						fdReadSet_;
			fd_set						fdWriteSet_;

			// ���ע���socket������ ������д��
			int							fdLargest_;

			// ע��д��socket����������
			int							fdWriteCount_;
		};

#endif // USE_IOCP
#endif // HAS_EPOLL

	}
}
#endif // KBE_IOCP_POLLER_H
#pragma once
