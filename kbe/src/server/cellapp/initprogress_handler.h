// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_CELLAPP_INIT_PROGRESS_HANDLER_H
#define KBE_CELLAPP_INIT_PROGRESS_HANDLER_H

// common include
#include "helper/debug_helper.h"
#include "common/common.h"

namespace KBEngine{

class InitProgressHandler : public Task
{
public:
	InitProgressHandler(Network::NetworkInterface & networkInterface);
	~InitProgressHandler();
	
	bool process();

private:
	Network::NetworkInterface & networkInterface_;
	int delayTicks_;
};


}

#endif // KBE_CELLAPP_INIT_PROGRESS_HANDLER_H
