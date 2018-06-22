// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include "baseapp.h"
#include "proxy.h"
#include "proxy_forwarder.h"
#include "server/serverconfig.h"

namespace KBEngine{	

//-------------------------------------------------------------------------------------
ProxyForwarder::ProxyForwarder(Proxy * pProxy) : 
pProxy_(pProxy)
{
	timerHandle_ = Baseapp::getSingleton().dispatcher().addTimer(1000000 / g_kbeSrvConfig.gameUpdateHertz(), this,
							NULL);
}

//-------------------------------------------------------------------------------------
ProxyForwarder::~ProxyForwarder()
{
	timerHandle_.cancel();
}

//-------------------------------------------------------------------------------------
void ProxyForwarder::handleTimeout(TimerHandle, void * arg)
{
	pProxy_->sendToClient(false);
}


}
