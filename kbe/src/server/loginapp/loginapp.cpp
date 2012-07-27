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


#include "loginapp.hpp"
#include "loginapp_interface.hpp"
#include "network/common.hpp"
#include "network/tcp_packet.hpp"
#include "network/udp_packet.hpp"
#include "network/message_handler.hpp"
#include "thread/threadpool.hpp"
#include "server/componentbridge.hpp"
#include "server/components.hpp"

namespace KBEngine{
	
ServerConfig g_serverConfig;
KBE_SINGLETON_INIT(Loginapp);

//-------------------------------------------------------------------------------------
Loginapp::Loginapp(Mercury::EventDispatcher& dispatcher, 
			 Mercury::NetworkInterface& ninterface, 
			 COMPONENT_TYPE componentType,
			 COMPONENT_ID componentID):
	ServerApp(dispatcher, ninterface, componentType, componentID)
{
}

//-------------------------------------------------------------------------------------
Loginapp::~Loginapp()
{
}

//-------------------------------------------------------------------------------------
bool Loginapp::run()
{
	bool ret = true;

	while(!this->getMainDispatcher().isBreakProcessing())
	{
		this->getMainDispatcher().processOnce(false);
		getNetworkInterface().handleChannels(&LoginappInterface::messageHandlers);
		KBEngine::sleep(100);
	};

	return ret;
}

//-------------------------------------------------------------------------------------
void Loginapp::handleTimeout(TimerHandle handle, void * arg)
{
	ServerApp::handleTimeout(handle, arg);
}

//-------------------------------------------------------------------------------------
bool Loginapp::initializeBegin()
{
	return true;
}

//-------------------------------------------------------------------------------------
bool Loginapp::inInitialize()
{
	return true;
}

//-------------------------------------------------------------------------------------
bool Loginapp::initializeEnd()
{
	return true;
}

//-------------------------------------------------------------------------------------
void Loginapp::finalise()
{
	ServerApp::finalise();
}

//-------------------------------------------------------------------------------------
void Loginapp::login(Mercury::Channel* pChannel, MemoryStream& s)
{
	Components::ComponentInfos* cinfos = Components::getSingleton().findComponent(BASEAPPMGR_TYPE);
	if(cinfos == NULL || cinfos->pChannel == NULL || cinfos->cid == 0)
	{
		_loginFailed(pChannel);
		return;
	}
}

//-------------------------------------------------------------------------------------
void Loginapp::_loginFailed(Mercury::Channel* pChannel)
{
}

//-------------------------------------------------------------------------------------
void Loginapp::onLoginAccountQueryResultFromDbmgr(Mercury::Channel* pChannel, MemoryStream& s)
{
}

//-------------------------------------------------------------------------------------
void Loginapp::onLoginAccountQueryBaseappAddrFromBaseappmgr(Mercury::Channel* pChannel, MemoryStream& s)
{
}

//-------------------------------------------------------------------------------------
void Loginapp::onDbmgrInitCompleted(Mercury::Channel* pChannel, int32 startGlobalOrder, int32 startGroupOrder)
{
	INFO_MSG("Loginapp::onDbmgrInitCompleted:startGlobalOrder=%d, startGroupOrder=%d.\n",
		startGlobalOrder, startGroupOrder);

	startGlobalOrder_ = startGlobalOrder;
	startGroupOrder_ = startGroupOrder;
}

//-------------------------------------------------------------------------------------

}
