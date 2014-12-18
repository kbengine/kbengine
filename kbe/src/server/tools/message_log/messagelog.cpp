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
#include "server/components.hpp"
#include <sstream>

namespace KBEngine{
	
ServerConfig g_serverConfig;
KBE_SINGLETON_INIT(Messagelog);

//-------------------------------------------------------------------------------------
Messagelog::Messagelog(Network::EventDispatcher& dispatcher, 
				 Network::NetworkInterface& ninterface, 
				 COMPONENT_TYPE componentType,
				 COMPONENT_ID componentID):
	ServerApp(dispatcher, ninterface, componentType, componentID),
logWatchers_(),
buffered_logs_()

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
	Network::g_trace_packet = 0;
	return true;
}

//-------------------------------------------------------------------------------------
void Messagelog::finalise()
{
	ServerApp::finalise();
}

//-------------------------------------------------------------------------------------	
bool Messagelog::canShutdown()
{
	if(Components::getSingleton().getGameSrvComponentsSize() > 0)
	{
		INFO_MSG(fmt::format("Messagelog::canShutdown(): Waiting for components({}) destruction!\n", 
			Components::getSingleton().getGameSrvComponentsSize()));

		return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------
void Messagelog::writeLog(Network::Channel* pChannel, KBEngine::MemoryStream& s)
{
	LOG_ITEM* pLogItem = new LOG_ITEM();
	std::string str;

	s >> pLogItem->logtype;
	s >> pLogItem->componentType;
	s >> pLogItem->componentID;
	s >> pLogItem->componentGlobalOrder;
	s >> pLogItem->componentGroupOrder;
	s >> pLogItem->t >> pLogItem->kbetime;
	s >> str;

	time_t tt = static_cast<time_t>(pLogItem->t);	
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
		aTm->tm_mday, aTm->tm_hour, aTm->tm_min, aTm->tm_sec, pLogItem->kbetime);

	pLogItem->logstream << KBELOG_TYPE_NAME_EX(pLogItem->logtype);
	pLogItem->logstream << " ";
	pLogItem->logstream << COMPONENT_NAME_EX_1(pLogItem->componentType);
	pLogItem->logstream << " ";
	pLogItem->logstream << pLogItem->componentID;
	pLogItem->logstream << " ";
	pLogItem->logstream << (int)pLogItem->componentGlobalOrder;
	pLogItem->logstream << (int)pLogItem->componentGroupOrder;
	pLogItem->logstream << timebuf;
	pLogItem->logstream << "- ";
	pLogItem->logstream << str;
	DebugHelper::getSingleton().changeLogger(COMPONENT_NAME_EX(pLogItem->componentType));
	PRINT_MSG(pLogItem->logstream.str());
	DebugHelper::getSingleton().changeLogger("default");

	LOG_WATCHERS::iterator iter = logWatchers_.begin();
	for(; iter != logWatchers_.end(); iter++)
	{
		iter->second.onMessage(pLogItem);
	}

	// 缓存一部分log，提供工具查看log时能快速获取初始上下文
	buffered_logs_.push_back(pLogItem);
	if(buffered_logs_.size() > 64)
	{
		pLogItem = buffered_logs_.front();
		buffered_logs_.pop_front();
		delete pLogItem;
	}
}

//-------------------------------------------------------------------------------------
void Messagelog::sendInitLogs(LogWatcher& logWatcher)
{
	std::deque<LOG_ITEM*>::iterator iter = buffered_logs_.begin();
	for(; iter != buffered_logs_.end(); iter++)
	{
		logWatcher.onMessage((*iter));
	}
}

//-------------------------------------------------------------------------------------
void Messagelog::registerLogWatcher(Network::Channel* pChannel, KBEngine::MemoryStream& s)
{
	LogWatcher* pLogwatcher = &logWatchers_[pChannel->addr()];
	if(!pLogwatcher->createFromStream(&s))
	{
		ERROR_MSG(fmt::format("Messagelog::registerLogWatcher: addr={} is failed!\n",
			pChannel->addr().c_str()));

		logWatchers_.erase(pChannel->addr());
		s.done();
		return;
	}

	pLogwatcher->addr(pChannel->addr());

	INFO_MSG(fmt::format("Messagelog::registerLogWatcher: addr={0} is successfully!\n",
		pChannel->addr().c_str()));

	bool first;
	s >> first;

	if(first)
		sendInitLogs(*pLogwatcher);
}

//-------------------------------------------------------------------------------------
void Messagelog::deregisterLogWatcher(Network::Channel* pChannel, KBEngine::MemoryStream& s)
{
	logWatchers_.erase(pChannel->addr());

	INFO_MSG(fmt::format("Messagelog::deregisterLogWatcher: addr={0} is successfully!\n",
		pChannel->addr().c_str()));
}

//-------------------------------------------------------------------------------------
void Messagelog::updateLogWatcherSetting(Network::Channel* pChannel, KBEngine::MemoryStream& s)
{
	LogWatcher* pLogwatcher = &logWatchers_[pChannel->addr()];
	if(!pLogwatcher->updateSetting(&s))
	{
		ERROR_MSG(fmt::format("Messagelog::updateLogWatcherSetting: addr={} is failed!\n",
			pChannel->addr().c_str()));

		logWatchers_.erase(pChannel->addr());
	}

	s.done();
}

//-------------------------------------------------------------------------------------

}
