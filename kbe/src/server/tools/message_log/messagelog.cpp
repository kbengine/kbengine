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


#include "messagelog.hpp"
#include "messagelog_interface.hpp"
#include "network/common.hpp"
#include "network/tcp_packet.hpp"
#include "network/udp_packet.hpp"
#include "network/message_handler.hpp"
#include "network/bundle_broadcast.hpp"
#include "thread/threadpool.hpp"
#include "server/componentbridge.hpp"
#include <sstream>

namespace KBEngine{
	
ServerConfig g_serverConfig;
KBE_SINGLETON_INIT(Messagelog);

//-------------------------------------------------------------------------------------
Messagelog::Messagelog(Mercury::EventDispatcher& dispatcher, 
				 Mercury::NetworkInterface& ninterface, 
				 COMPONENT_TYPE componentType,
				 COMPONENT_ID componentID):
	ServerApp(dispatcher, ninterface, componentType, componentID)

{
}

//-------------------------------------------------------------------------------------
Messagelog::~Messagelog()
{
}

//-------------------------------------------------------------------------------------
bool Messagelog::run()
{
	bool ret = true;

	while(!this->getMainDispatcher().isBreakProcessing())
	{
		this->getMainDispatcher().processOnce(false);
		getNetworkInterface().handleChannels(&MessagelogInterface::messageHandlers);
		KBEngine::sleep(100);
	};

	return ret;
}

//-------------------------------------------------------------------------------------
void Messagelog::handleTimeout(TimerHandle handle, void * arg)
{
	ServerApp::handleTimeout(handle, arg);
}

//-------------------------------------------------------------------------------------
bool Messagelog::initializeBegin()
{
	return true;
}

//-------------------------------------------------------------------------------------
bool Messagelog::inInitialize()
{
	return true;
}

//-------------------------------------------------------------------------------------
bool Messagelog::initializeEnd()
{
	return true;
}

//-------------------------------------------------------------------------------------
void Messagelog::finalise()
{
	ServerApp::finalise();
}

//-------------------------------------------------------------------------------------
void Messagelog::writeLog(Mercury::Channel* pChannel, KBEngine::MemoryStream& s)
{
	int8 ilogtype;
	LOG_TYPE logType;
	COMPONENT_TYPE componentType = UNKNOWN_COMPONENT_TYPE;
	COMPONENT_ID componentID = 0;
	COMPONENT_ORDER componentOrder = 0;
	int64 t;
	GAME_TIME kbetime = 0;
	std::string str;
	std::stringstream logstream;

	s >> ilogtype;
	logType = static_cast<LOG_TYPE>(ilogtype);
	s >> componentType;
	s >> componentID;
	s >> componentOrder;
	s >> t >> kbetime;
	s >> str;
	
    tm* aTm = localtime(&t);
    //       YYYY   year
    //       MM     month (2 digits 01-12)
    //       DD     day (2 digits 01-31)
    //       HH     hour (2 digits 00-23)
    //       MM     minutes (2 digits 00-59)
    //       SS     seconds (2 digits 00-59)
	char timebuf[MAX_BUF];
    kbe_snprintf(timebuf, MAX_BUF, " [%-4d-%02d-%02d %02d:%02d:%02d %02d] ", aTm->tm_year+1900, aTm->tm_mon+1, 
		aTm->tm_mday, aTm->tm_hour, aTm->tm_min, aTm->tm_sec, kbetime);

	logstream << LOG_TYPE_NAME_EX(logType);
	logstream << " ";
	logstream << COMPONENT_NAME_EX(componentType);
	logstream << " ";
	logstream << componentID;
	logstream << " ";
	logstream << (int)componentOrder;
	logstream << timebuf;
	logstream << "- ";
	logstream << str;
	DebugHelper::getSingleton().changeLogger(COMPONENT_NAME_EX(componentType));
	PRINT_MSG(logstream.str().c_str());
	DebugHelper::getSingleton().changeLogger("default");
}

//-------------------------------------------------------------------------------------

}
