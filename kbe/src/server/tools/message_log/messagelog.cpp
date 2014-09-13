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

	while(!this->mainDispatcher().isBreakProcessing())
	{
		threadPool_.onMainThreadTick();
		this->mainDispatcher().processOnce(false);
		networkInterface().processAllChannelPackets(&MessagelogInterface::messageHandlers);
		KBEngine::sleep(10);
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
	// 由于messagelog接收其他app的log，如果跟踪包输出将会非常卡。
	Mercury::g_trace_packet = 0;
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
	uint32 logtype;
	COMPONENT_TYPE componentType = UNKNOWN_COMPONENT_TYPE;
	COMPONENT_ID componentID = 0;
	COMPONENT_ORDER componentOrder = 0;
	int64 t;
	GAME_TIME kbetime = 0;
	std::string str;
	std::stringstream logstream;

	s >> logtype;
	s >> componentType;
	s >> componentID;
	s >> componentOrder;
	s >> t >> kbetime;
	s >> str;

	time_t tt = static_cast<time_t>(t);	
    tm* aTm = localtime(&tt);
    //       YYYY   year
    //       MM     month (2 digits 01-12)
    //       DD     day (2 digits 01-31)
    //       HH     hour (2 digits 00-23)
    //       MM     minutes (2 digits 00-59)
    //       SS     seconds (2 digits 00-59)

	if(aTm == NULL)
	{
		ERROR_MSG("Messagelog::writeLog: log is error!\n");
		return;
	}

	char timebuf[MAX_BUF];
    kbe_snprintf(timebuf, MAX_BUF, " [%-4d-%02d-%02d %02d:%02d:%02d %02d] ", aTm->tm_year+1900, aTm->tm_mon+1, 
		aTm->tm_mday, aTm->tm_hour, aTm->tm_min, aTm->tm_sec, kbetime);

	logstream << KBELOG_TYPE_NAME_EX(logtype);
	logstream << " ";
	logstream << COMPONENT_NAME_EX_1(componentType);
	logstream << " ";
	logstream << componentID;
	logstream << " ";
	logstream << (int)componentOrder;
	logstream << timebuf;
	logstream << "- ";
	logstream << str;
	DebugHelper::getSingleton().changeLogger(COMPONENT_NAME_EX(componentType));
	PRINT_MSG(logstream.str());
	DebugHelper::getSingleton().changeLogger("default");

	LOG_WATCHERS::iterator iter = logWatchers_.begin();
	for(; iter != logWatchers_.end(); iter++)
	{
		iter->second.onMessage(logtype, componentType, componentID, componentOrder, t, kbetime, str, logstream);
	}
}

//-------------------------------------------------------------------------------------
void Messagelog::registerLogWatcher(Mercury::Channel* pChannel, KBEngine::MemoryStream& s)
{
	LogWatcher* pLogwatcher = &logWatchers_[pChannel->addr()];
	if(!pLogwatcher->loadFromStream(&s))
	{
		ERROR_MSG(boost::format("Messagelog::registerLogWatcher: addr=%1% is failed!\n") %
			pChannel->addr().c_str());

		logWatchers_.erase(pChannel->addr());
		return;
	}

	pLogwatcher->addr(pChannel->addr());

	INFO_MSG(fmt::format("Messagelog::registerLogWatcher: addr={0} is successfully!\n",
		pChannel->addr().c_str()));
}

//-------------------------------------------------------------------------------------

}
