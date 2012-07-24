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


#include "baseappmgr.hpp"
#include "baseappmgr_interface.hpp"
#include "network/common.hpp"
#include "network/tcp_packet.hpp"
#include "network/udp_packet.hpp"
#include "network/message_handler.hpp"
#include "thread/threadpool.hpp"
#include "server/componentbridge.hpp"

namespace KBEngine{
	
ServerConfig g_serverConfig;
KBE_SINGLETON_INIT(Baseappmgr);

//-------------------------------------------------------------------------------------
Baseappmgr::Baseappmgr(Mercury::EventDispatcher& dispatcher, 
			 Mercury::NetworkInterface& ninterface, 
			 COMPONENT_TYPE componentType,
			 COMPONENT_ID componentID):
	ServerApp(dispatcher, ninterface, componentType, componentID),
	gameTimer_()
{
}

//-------------------------------------------------------------------------------------
Baseappmgr::~Baseappmgr()
{
}

//-------------------------------------------------------------------------------------
bool Baseappmgr::run()
{
	return ServerApp::run();
}

//-------------------------------------------------------------------------------------
void Baseappmgr::handleTimeout(TimerHandle handle, void * arg)
{
	switch (reinterpret_cast<uintptr>(arg))
	{
		case TIMEOUT_GAME_TICK:
			this->handleGameTick();
			break;
		default:
			break;
	}

	ServerApp::handleTimeout(handle, arg);
}

//-------------------------------------------------------------------------------------
void Baseappmgr::handleGameTick()
{
	 //time_t t = ::time(NULL);
	 //DEBUG_MSG("CellApp::handleGameTick[%"PRTime"]:%u\n", t, time_);
	
	time_++;
	getNetworkInterface().handleChannels(&BaseappmgrInterface::messageHandlers);
}

//-------------------------------------------------------------------------------------
bool Baseappmgr::initializeBegin()
{
	return true;
}

//-------------------------------------------------------------------------------------
bool Baseappmgr::inInitialize()
{
	return true;
}

//-------------------------------------------------------------------------------------
bool Baseappmgr::initializeEnd()
{
	gameTimer_ = this->getMainDispatcher().addTimer(1000000 / g_kbeSrvConfig.gameUpdateHertz(), this,
							reinterpret_cast<void *>(TIMEOUT_GAME_TICK));
	return true;
}

//-------------------------------------------------------------------------------------
void Baseappmgr::finalise()
{
	gameTimer_.cancel();
	ServerApp::finalise();
}

//-------------------------------------------------------------------------------------
void Baseappmgr::reqCreateBaseAnywhere(Mercury::Channel* pChannel, MemoryStream& s) 
{
	DEBUG_MSG("Baseappmgr::reqCreateBaseAnywhere: %s opsize=%d\n", pChannel->c_str(), s.opsize());
	Components::COMPONENTS& components = Components::getSingleton().getComponents(BASEAPP_TYPE);
	size_t componentSize = components.size();
	if(componentSize == 0)
		return;
	
	static uint32 currentBaseappIndex = 0;
	if(currentBaseappIndex > componentSize - 1)
		currentBaseappIndex = 0;
	
	Components::COMPONENTS::iterator iter = components.begin();
	std::advance(iter, currentBaseappIndex++);
	Mercury::Channel* lpChannel = (*iter).pChannel;

	Mercury::Bundle bundle;
	bundle.append((char*)s.data(), s.opsize());
	bundle.send(this->getNetworkInterface(), lpChannel);
}

//-------------------------------------------------------------------------------------

}
