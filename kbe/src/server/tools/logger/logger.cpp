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


#include "logger.h"
#include "logger_interface.h"
#include "network/common.h"
#include "network/tcp_packet.h"
#include "network/udp_packet.h"
#include "network/message_handler.h"
#include "network/bundle_broadcast.h"
#include "thread/threadpool.h"
#include "server/components.h"
#include <sstream>

namespace KBEngine{
	
ServerConfig g_serverConfig;
KBE_SINGLETON_INIT(Logger);

//-------------------------------------------------------------------------------------
Logger::Logger(Network::EventDispatcher& dispatcher, 
				 Network::NetworkInterface& ninterface, 
				 COMPONENT_TYPE componentType,
				 COMPONENT_ID componentID):
	ServerApp(dispatcher, ninterface, componentType, componentID),
logWatchers_(),
buffered_logs_(),
timer_()
{
}

//-------------------------------------------------------------------------------------
Logger::~Logger()
{
}

//-------------------------------------------------------------------------------------
bool Logger::run()
{
	dispatcher_.processUntilBreak();
	return true;
}

//-------------------------------------------------------------------------------------
void Logger::handleTimeout(TimerHandle handle, void * arg)
{
	switch (reinterpret_cast<uintptr>(arg))
	{
		case TIMEOUT_TICK:
			this->handleTick();
			break;
		default:
			break;
	}

	ServerApp::handleTimeout(handle, arg);
}

//-------------------------------------------------------------------------------------
void Logger::handleTick()
{
	threadPool_.onMainThreadTick();
	networkInterface().processAllChannelPackets(&LoggerInterface::messageHandlers);
}

//-------------------------------------------------------------------------------------
bool Logger::initializeBegin()
{
	return true;
}

//-------------------------------------------------------------------------------------
bool Logger::inInitialize()
{
	return true;
}

//-------------------------------------------------------------------------------------
bool Logger::initializeEnd()
{
	// 由于logger接收其他app的log，如果跟踪包输出将会非常卡。
	Network::g_trace_packet = 0;

	timer_ = this->dispatcher().addTimer(1000000 / 50, this,
							reinterpret_cast<void *>(TIMEOUT_TICK));
	return true;
}

//-------------------------------------------------------------------------------------
void Logger::finalise()
{
	timer_.cancel();
	ServerApp::finalise();
}

//-------------------------------------------------------------------------------------	
bool Logger::canShutdown()
{
	if(Components::getSingleton().getGameSrvComponentsSize() > 0)
	{
		INFO_MSG(fmt::format("Logger::canShutdown(): Waiting for components({}) destruction!\n", 
			Components::getSingleton().getGameSrvComponentsSize()));

		return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------
void Logger::writeLog(Network::Channel* pChannel, KBEngine::MemoryStream& s)
{
	LOG_ITEM* pLogItem = new LOG_ITEM();
	std::string str;

	s >> pLogItem->uid;
	s >> pLogItem->logtype;
	s >> pLogItem->componentType;
	s >> pLogItem->componentID;
	s >> pLogItem->componentGlobalOrder;
	s >> pLogItem->componentGroupOrder;
	s >> pLogItem->t;
	s >> pLogItem->kbetime;
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
		ERROR_MSG("Logger::writeLog: log is error!\n");
		return;
	}

	char timebuf[MAX_BUF];

	pLogItem->logstream << KBELOG_TYPE_NAME_EX(pLogItem->logtype);
	pLogItem->logstream << " ";
	pLogItem->logstream << COMPONENT_NAME_EX_2(pLogItem->componentType);

    kbe_snprintf(timebuf, MAX_BUF, "%02d", (int)pLogItem->componentGroupOrder);
	pLogItem->logstream << timebuf;
	pLogItem->logstream << " ";
	pLogItem->logstream << pLogItem->uid;
	pLogItem->logstream << " ";
	pLogItem->logstream << pLogItem->componentID;
	pLogItem->logstream << " ";

    kbe_snprintf(timebuf, MAX_BUF, " [%-4d-%02d-%02d %02d:%02d:%02d %03d] ", aTm->tm_year+1900, aTm->tm_mon+1, 
		aTm->tm_mday, aTm->tm_hour, aTm->tm_min, aTm->tm_sec, pLogItem->kbetime);
	pLogItem->logstream << timebuf;

	pLogItem->logstream << "- ";
	pLogItem->logstream << str;


	DebugHelper::getSingleton().changeLogger(COMPONENT_NAME_EX(pLogItem->componentType));
	PRINT_MSG(pLogItem->logstream.str());
	DebugHelper::getSingleton().changeLogger("default");

	LOG_WATCHERS::iterator iter = logWatchers_.begin();
	for(; iter != logWatchers_.end(); ++iter)
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
void Logger::sendInitLogs(LogWatcher& logWatcher)
{
	std::deque<LOG_ITEM*>::iterator iter = buffered_logs_.begin();
	for(; iter != buffered_logs_.end(); ++iter)
	{
		logWatcher.onMessage((*iter));
	}
}

//-------------------------------------------------------------------------------------
void Logger::registerLogWatcher(Network::Channel* pChannel, KBEngine::MemoryStream& s)
{
	LogWatcher* pLogwatcher = &logWatchers_[pChannel->addr()];
	if(!pLogwatcher->createFromStream(&s))
	{
		ERROR_MSG(fmt::format("Logger::registerLogWatcher: addr={} is failed!\n",
			pChannel->addr().c_str()));

		logWatchers_.erase(pChannel->addr());
		s.done();
		return;
	}

	pLogwatcher->addr(pChannel->addr());

	INFO_MSG(fmt::format("Logger::registerLogWatcher: addr={0} is successfully!\n",
		pChannel->addr().c_str()));

	bool first;
	s >> first;

	if(first)
		sendInitLogs(*pLogwatcher);
}

//-------------------------------------------------------------------------------------
void Logger::deregisterLogWatcher(Network::Channel* pChannel, KBEngine::MemoryStream& s)
{
	logWatchers_.erase(pChannel->addr());

	INFO_MSG(fmt::format("Logger::deregisterLogWatcher: addr={0} is successfully!\n",
		pChannel->addr().c_str()));
}

//-------------------------------------------------------------------------------------
void Logger::updateLogWatcherSetting(Network::Channel* pChannel, KBEngine::MemoryStream& s)
{
	LogWatcher* pLogwatcher = &logWatchers_[pChannel->addr()];
	if(!pLogwatcher->updateSetting(&s))
	{
		ERROR_MSG(fmt::format("Logger::updateLogWatcherSetting: addr={} is failed!\n",
			pChannel->addr().c_str()));

		logWatchers_.erase(pChannel->addr());
	}

	s.done();
}

//-------------------------------------------------------------------------------------

}
