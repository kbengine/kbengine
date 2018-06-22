// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_COMPONENT_ACTIVE_REPORT_HANDLER_H
#define KBE_COMPONENT_ACTIVE_REPORT_HANDLER_H

#include "common/common.h"
#include "common/tasks.h"
#include "common/timer.h"
#include "helper/debug_helper.h"

namespace KBEngine { 

class ServerApp;

class ComponentActiveReportHandler : public TimerHandler
{
public:
	enum TimeOutType
	{
		TIMEOUT_ACTIVE_TICK,
		TIMEOUT_MAX
	};
	
	ComponentActiveReportHandler(ServerApp* pApp);
	virtual ~ComponentActiveReportHandler();
	
	void startActiveTick(float period);

	void cancel();

protected:
	virtual void handleTimeout(TimerHandle handle, void * arg);

	ServerApp* pApp_;
	TimerHandle pActiveTimerHandle_;

};

}

#endif // KBE_COMPONENT_ACTIVE_REPORT_HANDLER_H
