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


#include "resourcemgr.hpp"
#include "resourcemgr_interface.hpp"
#include "network/common.hpp"
#include "network/tcp_packet.hpp"
#include "network/udp_packet.hpp"
#include "network/message_handler.hpp"
#include "network/bundle_broadcast.hpp"
#include "thread/threadpool.hpp"
#include "server/componentbridge.hpp"

namespace KBEngine{
	
ServerConfig g_serverConfig;
KBE_SINGLETON_INIT(Resourcemgr);

//-------------------------------------------------------------------------------------
Resourcemgr::Resourcemgr(Mercury::EventDispatcher& dispatcher, 
				 Mercury::NetworkInterface& ninterface, 
				 COMPONENT_TYPE componentType,
				 COMPONENT_ID componentID):
	ServerApp(dispatcher, ninterface, componentType, componentID),
	pResmgrTimerHandle_()
{
}

//-------------------------------------------------------------------------------------
Resourcemgr::~Resourcemgr()
{
}

//-------------------------------------------------------------------------------------
bool Resourcemgr::run()
{
	bool ret = true;

	while(!this->getMainDispatcher().isBreakProcessing())
	{
		threadPool_.onMainThreadTick();
		this->getMainDispatcher().processOnce(false);
		getNetworkInterface().processAllChannelPackets(&ResourcemgrInterface::messageHandlers);
		KBEngine::sleep(100);
	};

	return ret;
}

//-------------------------------------------------------------------------------------
void Resourcemgr::handleTimeout(TimerHandle handle, void * arg)
{
	ServerApp::handleTimeout(handle, arg);
}

//-------------------------------------------------------------------------------------
bool Resourcemgr::initializeBegin()
{
	return true;
}

//-------------------------------------------------------------------------------------
bool Resourcemgr::inInitialize()
{
	return true;
}

//-------------------------------------------------------------------------------------
bool Resourcemgr::initializeEnd()
{
	if(Resmgr::respool_checktick > 0)
	{
		pResmgrTimerHandle_ = this->getMainDispatcher().addTimer(int(Resmgr::respool_checktick * 1000000),
			Resmgr::getSingletonPtr(), NULL);

		INFO_MSG(boost::format("Resourcemgr::initializeEnd: started resmgr tick(%1%s)!\n") % 
			Resmgr::respool_checktick);
	}

	return true;
}

//-------------------------------------------------------------------------------------
void Resourcemgr::finalise()
{
	pResmgrTimerHandle_.cancel();
	ServerApp::finalise();
}

//-------------------------------------------------------------------------------------

}
