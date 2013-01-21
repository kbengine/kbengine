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


#include "clientapp.hpp"
#include "network/channel.hpp"
#include "thread/threadpool.hpp"
#include "entitydef/entity_mailbox.hpp"

#include "../../server/baseapp/baseapp_interface.hpp"
#include "../../server/loginapp/loginapp_interface.hpp"

namespace KBEngine{

COMPONENT_TYPE g_componentType = UNKNOWN_COMPONENT_TYPE;
COMPONENT_ID g_componentID = 0;
COMPONENT_ORDER g_componentOrder = 1;
GAME_TIME g_kbetime = 0;

KBE_SINGLETON_INIT(ClientApp);
//-------------------------------------------------------------------------------------
ClientApp::ClientApp(Mercury::EventDispatcher& dispatcher, 
					 Mercury::NetworkInterface& ninterface, 
					 COMPONENT_TYPE componentType,
					 COMPONENT_ID componentID):
TimerHandler(),
Mercury::ChannelTimeOutHandler(),
componentType_(componentType),
componentID_(componentID),
mainDispatcher_(dispatcher),
networkInterface_(ninterface),
timers_(),
threadPool_(),
serverChannel_(NULL)
{
	networkInterface_.pExtensionData(this);
	networkInterface_.pChannelTimeOutHandler(this);
	networkInterface_.pChannelDeregisterHandler(this);

	// 初始化mailbox模块获取channel函数地址
	EntityMailbox::setFindChannelFunc(std::tr1::bind(&ClientApp::findChannelByMailbox, this, 
		std::tr1::placeholders::_1));
}

//-------------------------------------------------------------------------------------
ClientApp::~ClientApp()
{
	serverChannel_ = NULL;
}

//-------------------------------------------------------------------------------------
bool ClientApp::loadConfig()
{
	return true;
}

//-------------------------------------------------------------------------------------		
bool ClientApp::installSingnals()
{
	return true;
}

//-------------------------------------------------------------------------------------		
bool ClientApp::initialize()
{
	if(!threadPool_.isInitialize())
		threadPool_.createThreadPool(4, 4, 256);

	if(!installSingnals())
		return false;
	
	if(!loadConfig())
		return false;
	
	if(!initializeBegin())
		return false;
	
	if(!inInitialize())
		return false;

	return initializeEnd();
}

//-------------------------------------------------------------------------------------		
void ClientApp::finalise(void)
{
	threadPool_.finalise();
}

//-------------------------------------------------------------------------------------		
double ClientApp::gameTimeInSeconds() const
{
	return double(g_kbetime) / 10;
}

//-------------------------------------------------------------------------------------
void ClientApp::handleTimeout(TimerHandle, void * arg)
{
	switch (reinterpret_cast<uintptr>(arg))
	{
		case TIMEOUT_GAME_TICK:
		{
		}
		default:
			break;
	}
}

//-------------------------------------------------------------------------------------
void ClientApp::handleTimers()
{
	timers().process(g_kbetime);
}

//-------------------------------------------------------------------------------------		
bool ClientApp::run(void)
{
	mainDispatcher_.processUntilBreak();
	return true;
}

//-------------------------------------------------------------------------------------	
void ClientApp::shutDown()
{
	INFO_MSG( "ClientApp::shutDown: shutting down\n" );
	mainDispatcher_.breakProcessing();
}

//-------------------------------------------------------------------------------------	
void ClientApp::onChannelDeregister(Mercury::Channel * pChannel)
{
}

//-------------------------------------------------------------------------------------	
void ClientApp::onChannelTimeOut(Mercury::Channel * pChannel)
{
	INFO_MSG(boost::format("ClientApp::onChannelTimeOut: "
		"Channel %1% timed out.\n") % pChannel->c_str());

	networkInterface_.deregisterChannel(pChannel);
	pChannel->destroy();
}

//-------------------------------------------------------------------------------------	
Mercury::Channel* ClientApp::findChannelByMailbox(EntityMailbox& mailbox)
{
	return serverChannel_;
}

//-------------------------------------------------------------------------------------	
void ClientApp::onCreateAccountResult(Mercury::Channel * pChannel, MemoryStream& s)
{
	SERVER_ERROR_CODE failedcode;
	std::string datas;

	s >> failedcode;
	s.readBlob(datas);
}

//-------------------------------------------------------------------------------------	
void ClientApp::onLoginSuccessfully(Mercury::Channel * pChannel, MemoryStream& s)
{
}

//-------------------------------------------------------------------------------------	
void ClientApp::onLoginFailed(Mercury::Channel * pChannel, MemoryStream& s)
{
}

//-------------------------------------------------------------------------------------	
void ClientApp::onLoginGatewayFailed(Mercury::Channel * pChannel, SERVER_ERROR_CODE failedcode)
{
}

//-------------------------------------------------------------------------------------	
void ClientApp::onCreatedProxies(Mercury::Channel * pChannel, 
								 uint64 rndUUID, ENTITY_ID eid, std::string& entityType)
{
}

//-------------------------------------------------------------------------------------	
void ClientApp::onEntityEnterWorld(Mercury::Channel * pChannel, ENTITY_ID eid, 
								   ENTITY_SCRIPT_UID scriptType, SPACE_ID spaceID)
{
}

//-------------------------------------------------------------------------------------	
void ClientApp::onEntityLeaveWorld(Mercury::Channel * pChannel, ENTITY_ID eid, SPACE_ID spaceID)
{
}

//-------------------------------------------------------------------------------------	
void ClientApp::onEntityEnterSpace(Mercury::Channel * pChannel, SPACE_ID spaceID, ENTITY_ID eid)
{
}

//-------------------------------------------------------------------------------------	
void ClientApp::onEntityLeaveSpace(Mercury::Channel * pChannel, SPACE_ID spaceID, ENTITY_ID eid)
{
}

//-------------------------------------------------------------------------------------	
void ClientApp::onEntityDestroyed(Mercury::Channel * pChannel, ENTITY_ID eid)
{
}

//-------------------------------------------------------------------------------------
void ClientApp::onRemoteMethodCall(Mercury::Channel* pChannel, KBEngine::MemoryStream& s)
{
}

//-------------------------------------------------------------------------------------
void ClientApp::onUpdatePropertys(Mercury::Channel* pChannel, MemoryStream& s)
{
}

//-------------------------------------------------------------------------------------
void ClientApp::onStreamDataStarted(Mercury::Channel* pChannel, int16 id, uint32 datasize, std::string& descr)
{
}

//-------------------------------------------------------------------------------------
void ClientApp::onStreamDataRecv(Mercury::Channel* pChannel, MemoryStream& s)
{
}

//-------------------------------------------------------------------------------------
void ClientApp::onStreamDataCompleted(Mercury::Channel* pChannel, int16 id)
{
}

//-------------------------------------------------------------------------------------		
}
