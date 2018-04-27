// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_SELECT_POLLER_H
#define KBE_SELECT_POLLER_H

#include "event_poller.h"

namespace KBEngine { 
namespace Network
{

#ifndef HAS_EPOLL

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
	void handleNotifications(int &countReady,
			fd_set &readFDs, fd_set &writeFDs);

	fd_set						fdReadSet_;
	fd_set						fdWriteSet_;

	// 最后注册的socket描述符 （读或写）
	int							fdLargest_;

	// 注册写的socket描述符数量
	int							fdWriteCount_;
};


#endif // HAS_EPOLL

}
}
#endif // KBE_SELECT_POLLER_H
