// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_UPDATE_DB_SERVER_LOG_HANDLER_H
#define KBE_UPDATE_DB_SERVER_LOG_HANDLER_H

// common include
#include "helper/debug_helper.h"
#include "common/common.h"

namespace KBEngine{

class UpdateDBServerLogHandler : public TimerHandler
{	
public:	
	UpdateDBServerLogHandler();

	~UpdateDBServerLogHandler();

private:
	virtual void handleTimeout(TimerHandle handle, void * pUser);

	virtual void onRelease( TimerHandle handle, void * /*pUser*/ ){};

	void cancel();
	
	TimerHandle* pTimerHandle_;
};	


}

#endif // KBE_UPDATE_DB_SERVER_LOG_HANDLER_H
