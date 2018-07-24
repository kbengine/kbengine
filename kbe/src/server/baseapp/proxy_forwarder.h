// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_PROXY_FORWARDER_H
#define KBE_PROXY_FORWARDER_H

#include "helper/debug_helper.h"
#include "common/common.h"

namespace KBEngine{

class ProxyForwarder : public TimerHandler
{
public:
	ProxyForwarder(Proxy * pProxy);
	virtual ~ProxyForwarder();

protected:
	virtual void handleTimeout(TimerHandle, void * arg);
	virtual void onRelease(TimerHandle handle, void  * pUser) {}

protected:
	Proxy * pProxy_;
	TimerHandle timerHandle_;
};

}

#endif // KBE_PROXY_FORWARDER_H
