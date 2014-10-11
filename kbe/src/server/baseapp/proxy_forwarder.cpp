/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 KBEngine.

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

#include "baseapp.hpp"
#include "proxy.hpp"
#include "proxy_forwarder.hpp"
#include "server/serverconfig.hpp"

namespace KBEngine{	

//-------------------------------------------------------------------------------------
ProxyForwarder::ProxyForwarder(Proxy * pProxy) : 
pProxy_(pProxy)
{
	timerHandle_ = Baseapp::getSingleton().mainDispatcher().addTimer(1000000 / g_kbeSrvConfig.gameUpdateHertz(), this,
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
