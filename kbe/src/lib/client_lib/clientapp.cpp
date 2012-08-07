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

#include "../../server/baseapp/baseapp_interface.hpp"
#include "../../server/loginapp/loginapp_interface.hpp"

namespace KBEngine{

COMPONENT_TYPE g_componentType = UNKNOWN_COMPONENT_TYPE;
COMPONENT_ID g_componentID = 0;
COMPONENT_ORDER g_componentOrder = 1;

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
time_(0),
timers_()
{
	networkInterface_.pExtensionData(this);
	networkInterface_.pChannelTimeOutHandler(this);
	networkInterface_.pChannelDeregisterHandler(this);
}

//-------------------------------------------------------------------------------------
ClientApp::~ClientApp()
{
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
}

//-------------------------------------------------------------------------------------		
double ClientApp::gameTimeInSeconds() const
{
	return double(time_) / 10;
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
	timers().process(time_);
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
	INFO_MSG( "ClientApp::onChannelTimeOut: "
		"Channel %s timed out.\n", pChannel->c_str());

	networkInterface_.deregisterChannel(pChannel);
	pChannel->destroy();
}

//-------------------------------------------------------------------------------------	
void ClientApp::onCreateAccountResult(Mercury::Channel * pChannel, MERCURY_ERROR_CODE failedcode)
{
}

//-------------------------------------------------------------------------------------	
void ClientApp::onLoginSuccessfully(Mercury::Channel * pChannel, MemoryStream& s)
{
}

//-------------------------------------------------------------------------------------	
void ClientApp::onLoginFailed(Mercury::Channel * pChannel, MERCURY_ERROR_CODE failedcode)
{
}

//-------------------------------------------------------------------------------------	
void ClientApp::onLoginGatewayFailed(Mercury::Channel * pChannel, MERCURY_ERROR_CODE failedcode)
{
}

//-------------------------------------------------------------------------------------	
void ClientApp::onLoginGatewaySuccessfully(Mercury::Channel * pChannel, uint64 rndUUID, ENTITY_ID eid)
{
}

//-------------------------------------------------------------------------------------		
}
