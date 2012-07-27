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


#include "bots.hpp"
#include "bots_interface.hpp"
#include "network/common.hpp"
#include "network/tcp_packet.hpp"
#include "network/udp_packet.hpp"
#include "network/message_handler.hpp"
#include "thread/threadpool.hpp"

#include "../../../server/baseapp/baseapp_interface.hpp"
#include "../../../server/loginapp/loginapp_interface.hpp"

namespace KBEngine{
	
ServerConfig g_serverConfig;
KBE_SINGLETON_INIT(Bots);

//-------------------------------------------------------------------------------------
Bots::Bots(Mercury::EventDispatcher& dispatcher, 
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
timers_(),
gameTimer_()
{
}

//-------------------------------------------------------------------------------------
Bots::~Bots()
{
}

//-------------------------------------------------------------------------------------
bool Bots::run()
{
	return true;
}

//-------------------------------------------------------------------------------------
void Bots::handleTimeout(TimerHandle handle, void * arg)
{
	switch (reinterpret_cast<uintptr>(arg))
	{
		case TIMEOUT_GAME_TICK:
			this->handleGameTick();
			break;
		default:
			break;
	}

}

//-------------------------------------------------------------------------------------
void Bots::handleGameTick()
{
	time_++;
	getNetworkInterface().handleChannels(&BotsInterface::messageHandlers);
}

//-------------------------------------------------------------------------------------
bool Bots::initializeBegin()
{
	return true;
}

//-------------------------------------------------------------------------------------
bool Bots::inInitialize()
{
	return true;
}

//-------------------------------------------------------------------------------------
bool Bots::initializeEnd()
{
	gameTimer_ = this->getMainDispatcher().addTimer(1000000 / g_kbeSrvConfig.gameUpdateHertz(), this,
							reinterpret_cast<void *>(TIMEOUT_GAME_TICK));
	return true;
}

//-------------------------------------------------------------------------------------
void Bots::finalise()
{
	gameTimer_.cancel();
}

//-------------------------------------------------------------------------------------

}
