/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2016 KBEngine.

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
#include "server/telnet_server.h"
#include "profile.h"	

namespace KBEngine{
	
ServerConfig g_serverConfig;
KBE_SINGLETON_INIT(Logger);

static uint64 g_totalNumlogs = 0;
static uint32 g_secsNumlogs = 0;
static uint64 g_lastCalcsecsNumlogsTime = 0;

uint64 totalNumlogs()
{
	return g_totalNumlogs;
}

uint64 secsNumlogs()
{
	return g_secsNumlogs;
}

//-------------------------------------------------------------------------------------
Logger::Logger(Network::EventDispatcher& dispatcher, 
				 Network::NetworkInterface& ninterface, 
				 COMPONENT_TYPE componentType,
				 COMPONENT_ID componentID):
	PythonApp(dispatcher, ninterface, componentType, componentID),
logWatchers_(),
buffered_logs_(),
timer_(),
pTelnetServer_(NULL)
{
}

//-------------------------------------------------------------------------------------
Logger::~Logger()
{
}

//-------------------------------------------------------------------------------------		
bool Logger::initializeWatcher()
{
	ProfileVal::setWarningPeriod(stampsPerSecond() / g_kbeSrvConfig.gameUpdateHertz());

	WATCH_OBJECT("stats/totalNumlogs", &totalNumlogs);
	WATCH_OBJECT("stats/secsNumlogs", &secsNumlogs);
	WATCH_OBJECT("stats/bufferedLogsSize", this, &Logger::bufferedLogsSize);
	return true;
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
	PythonApp::handleTimeout(handle, arg);

	switch (reinterpret_cast<uintptr>(arg))
	{
		case TIMEOUT_TICK:
			this->handleTick();
			break;
		default:
			break;
	}

}

//-------------------------------------------------------------------------------------
void Logger::handleTick()
{
	if(timestamp() - g_lastCalcsecsNumlogsTime > uint64( stampsPerSecond() ))
	{
		g_lastCalcsecsNumlogsTime = timestamp();
		g_secsNumlogs = 0;
	}

	threadPool_.onMainThreadTick();
	networkInterface().processChannels(&LoggerInterface::messageHandlers);
}

//-------------------------------------------------------------------------------------
bool Logger::initializeBegin()
{
	return true;
}

//-------------------------------------------------------------------------------------
bool Logger::inInitialize()
{
	PythonApp::inInitialize();
	return true;
}

//-------------------------------------------------------------------------------------
bool Logger::initializeEnd()
{
	PythonApp::initializeEnd();

	// ����logger��������app��log��������ٰ��������ǳ�����
	Network::g_trace_packet = 0;

	timer_ = this->dispatcher().addTimer(1000000 / 50, this,
							reinterpret_cast<void *>(TIMEOUT_TICK));

	SCOPED_PROFILE(SCRIPTCALL_PROFILE);

	// ���нű����������
	if (getEntryScript().get())
	{
		PyObject* pyResult = PyObject_CallMethod(getEntryScript().get(),
			const_cast<char*>("onLoggerAppReady"),
			const_cast<char*>(""));

		if (pyResult != NULL)
			Py_DECREF(pyResult);
		else
			SCRIPT_ERROR_CHECK();
	}

	pTelnetServer_ = new TelnetServer(&this->dispatcher(), &this->networkInterface());
	pTelnetServer_->pScript(&this->getScript());

	bool ret = pTelnetServer_->start(g_kbeSrvConfig.getLogger().telnet_passwd,
		g_kbeSrvConfig.getLogger().telnet_deflayer,
		g_kbeSrvConfig.getLogger().telnet_port);

	Components::getSingleton().extraData4(pTelnetServer_->port());
	return ret;
}

//-------------------------------------------------------------------------------------
void Logger::finalise()
{
	std::deque<LOG_ITEM*>::iterator iter = buffered_logs_.begin();
	for(; iter != buffered_logs_.end(); ++iter)
	{
		delete (*iter);
	}

	buffered_logs_.clear();

	timer_.cancel();
	PythonApp::finalise();
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

	if (getEntryScript().get() && PyObject_HasAttrString(getEntryScript().get(), "onReadyForShutDown") > 0)
	{
		// ���нű����������
		PyObject* pyResult = PyObject_CallMethod(getEntryScript().get(),
			const_cast<char*>("onReadyForShutDown"),
			const_cast<char*>(""));

		if (pyResult != NULL)
		{
			bool isReady = (pyResult == Py_True);
			Py_DECREF(pyResult);

			if (isReady)
				return true;
			else
				return false;
		}
		else
		{
			SCRIPT_ERROR_CHECK();
			return false;
		}
	}

	return true;
}

//-------------------------------------------------------------------------------------	
void Logger::onShutdownBegin()
{
	PythonApp::onShutdownBegin();

	// ֪ͨ�ű�
	if (getEntryScript().get())
	{
		SCOPED_PROFILE(SCRIPTCALL_PROFILE);
		SCRIPT_OBJECT_CALL_ARGS0(getEntryScript().get(), const_cast<char*>("onLoggerAppShutDown"));
	}
}

//-------------------------------------------------------------------------------------	
void Logger::onShutdownEnd()
{
	PythonApp::onShutdownEnd();
}

//-------------------------------------------------------------------------------------
void Logger::writeLog(Network::Channel* pChannel, KBEngine::MemoryStream& s)
{
	++g_secsNumlogs;
	++g_totalNumlogs;

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
	s.readBlob(str);

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
		ERROR_MSG("Logger::writeLog: log error!\n");
		delete pLogItem;
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

	static bool notificationScript = getEntryScript().get() && PyObject_HasAttrString(getEntryScript().get(), "onLogWrote") > 0;
	if(notificationScript)
	{
		// ��¼����������־�����ڽű��ص�ʱʹ��
		std::string sLog = pLogItem->logstream.str();
		
		PyObject* pyResult = PyObject_CallMethod(getEntryScript().get(),
			const_cast<char*>("onLogWrote"),
			const_cast<char*>("y#"),
			sLog.c_str(),
			sLog.length());

		if (pyResult != NULL)
		{
			Py_DECREF(pyResult);
		}
		else
		{
			SCRIPT_ERROR_CHECK();
		}
	}

	// ����һ����log���ṩ���߲鿴logʱ�ܿ��ٻ�ȡ��ʼ������
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
