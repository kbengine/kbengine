/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2018 KBEngine.

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

#ifndef KBE_EVENT_POLLER_H
#define KBE_EVENT_POLLER_H

#include "common/common.h"
#include "common/timestamp.h"
#include "network/interfaces.h"
#include "thread/concurrency.h"
#include "network/common.h"
#include <map>

namespace KBEngine { 
namespace Network
{
	
class InputNotificationHandler;
typedef std::map<int, InputNotificationHandler *> FDReadHandlers;
typedef std::map<int, OutputNotificationHandler *> FDWriteHandlers;

class EventPoller
{
public:
	EventPoller();
	virtual ~EventPoller();

	bool registerForRead(int fd, InputNotificationHandler * handler);
	bool registerForWrite(int fd, OutputNotificationHandler * handler);

	bool deregisterForRead(int fd);
	bool deregisterForWrite(int fd);


	virtual int processPendingEvents(double maxWait) = 0;
	virtual int getFileDescriptor() const;

	void clearSpareTime()		{spareTime_ = 0;}
	uint64 spareTime() const	{return spareTime_;}

	static EventPoller * create();

	InputNotificationHandler* findForRead(int fd);
	OutputNotificationHandler* findForWrite(int fd);

protected:
	virtual bool doRegisterForRead(int fd) = 0;
	virtual bool doRegisterForWrite(int fd) = 0;

	virtual bool doDeregisterForRead(int fd) = 0;
	virtual bool doDeregisterForWrite(int fd) = 0;

	bool triggerRead(int fd);
	bool triggerWrite(int fd);
	bool triggerError(int fd);
	
	bool isRegistered(int fd, bool isForRead) const;

	int maxFD() const;

private:
	FDReadHandlers fdReadHandlers_;
	FDWriteHandlers fdWriteHandlers_;

protected:
	uint64 spareTime_;
};

}
}
#endif // KBE_EVENT_POLLER_H
