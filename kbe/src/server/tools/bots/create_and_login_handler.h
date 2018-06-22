// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_CREATE_AND_LOGIN_HANDLER_H
#define KBE_CREATE_AND_LOGIN_HANDLER_H

#include "common/common.h"
#include "common/tasks.h"
#include "common/timer.h"
#include "helper/debug_helper.h"

namespace KBEngine { 

class CreateAndLoginHandler : public TimerHandler
{
public:
	CreateAndLoginHandler();
	virtual ~CreateAndLoginHandler();

protected:
	virtual void handleTimeout(TimerHandle handle, void * arg);

	TimerHandle timerHandle_;

};



}

#endif // KBE_CREATE_AND_LOGIN_HANDLER_H
