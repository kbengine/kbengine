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


#include "serverapp.hpp"
#include "server/component_active_report_handler.hpp"
#include "server/shutdowner.hpp"
#include "server/serverconfig.hpp"
#include "server/componentbridge.hpp"
#include "network/channel.hpp"
#include "network/bundle.hpp"
#include "network/common.hpp"
#include "cstdkbe/memorystream.hpp"
#include "helper/console_helper.hpp"
#include "helper/sys_info.hpp"
#include "helper/watch_pools.hpp"
#include "resmgr/resmgr.hpp"

#include "../../server/baseappmgr/baseappmgr_interface.hpp"
#include "../../server/cellappmgr/cellappmgr_interface.hpp"
#include "../../server/baseapp/baseapp_interface.hpp"
#include "../../server/cellapp/cellapp_interface.hpp"
#include "../../server/dbmgr/dbmgr_interface.hpp"
#include "../../server/loginapp/loginapp_interface.hpp"
#include "../../server/tools/message_log/messagelog_interface.hpp"
#include "../../server/tools/billing_system/billingsystem_interface.hpp"

namespace KBEngine{
COMPONENT_TYPE g_componentType = UNKNOWN_COMPONENT_TYPE;
COMPONENT_ID g_componentID = 0;
COMPONENT_ORDER g_componentGlobalOrder = -1;
COMPONENT_ORDER g_componentGroupOrder = -1;
GAME_TIME g_kbetime = 0;

//-------------------------------------------------------------------------------------
ServerApp::ServerApp(Mercury::EventDispatcher& dispatcher, 
					 Mercury::NetworkInterface& ninterface, 
					 COMPONENT_TYPE componentType,
					 COMPONENT_ID componentID):
SignalHandler(),
TimerHandler(),
ShutdownHandler(),
Mercury::ChannelTimeOutHandler(),
Components::ComponentsNotificationHandler(),
componentType_(componentType),
componentID_(componentID),
mainDispatcher_(dispatcher),
networkInterface_(ninterface),
timers_(),
startGlobalOrder_(-1),
startGroupOrder_(-1),
pShutdowner_(NULL),
pActiveTimerHandle_(NULL),
threadPool_()
{
	networkInterface_.pExtensionData(this);
	networkInterface_.pChannelTimeOutHandler(this);
	networkInterface_.pChannelDeregisterHandler(this);

	pActiveTimerHandle_ = new ComponentActiveReportHandler(this);
	pActiveTimerHandle_->startActiveTick(KBE_MAX(1.f, Mercury::g_channelInternalTimeout / 2.0f));

	// 默认所有app都设置为这个值， 如果需要调整则各自在派生类重新赋值
	ProfileVal::setWarningPeriod(stampsPerSecond() / g_kbeSrvConfig.gameUpdateHertz());
}

//-------------------------------------------------------------------------------------
ServerApp::~ServerApp()
{
	SAFE_RELEASE(pActiveTimerHandle_);
	SAFE_RELEASE(pShutdowner_);
}

//-------------------------------------------------------------------------------------	
void ServerApp::shutDown(float shutdowntime)
{
	if(pShutdowner_ == NULL)
		pShutdowner_ = new Shutdowner(this);

	pShutdowner_->shutdown(shutdowntime < 0.f ? g_kbeSrvConfig.shutdowntime() : shutdowntime, 
		g_kbeSrvConfig.shutdownWaitTickTime(), mainDispatcher_);
}

//-------------------------------------------------------------------------------------
void ServerApp::onShutdownBegin()
{
#if KBE_PLATFORM == PLATFORM_WIN32
	printf("[INFO]: shutdown begin.\n");
#endif

	mainDispatcher_.setWaitBreakProcessing();
	networkInterface_.dispatcher().setWaitBreakProcessing();
}

//-------------------------------------------------------------------------------------
void ServerApp::onShutdown(bool first)
{
}

//-------------------------------------------------------------------------------------
void ServerApp::onShutdownEnd()
{
	mainDispatcher_.breakProcessing();
	networkInterface_.dispatcher().breakProcessing();
}

//-------------------------------------------------------------------------------------
bool ServerApp::loadConfig()
{
	return true;
}

//-------------------------------------------------------------------------------------		
bool ServerApp::installSingnals()
{
	g_kbeSignalHandlers.attachApp(this);
	g_kbeSignalHandlers.addSignal(SIGINT, this);
	g_kbeSignalHandlers.addSignal(SIGPIPE, this);
	g_kbeSignalHandlers.addSignal(SIGHUP, this);
	return true;
}

//-------------------------------------------------------------------------------------		
bool ServerApp::initialize()
{
	if(!initThreadPool())
		return false;

	if(!installSingnals())
		return false;
	
	if(!loadConfig())
		return false;
	
	if(!initializeBegin())
		return false;
	
	if(!inInitialize())
		return false;
	
	// 广播自己的地址给网上上的所有kbemachine
	// 并且从kbemachine获取basappmgr和cellappmgr以及dbmgr地址
	Componentbridge::getSingleton().getComponents().pHandler(this);
	this->mainDispatcher().addFrequentTask(&Componentbridge::getSingleton());

	bool ret = initializeEnd();

#ifdef ENABLE_WATCHERS
	return ret && initializeWatcher();
#else
	return ret;
#endif
}

//-------------------------------------------------------------------------------------		
bool ServerApp::initializeWatcher()
{
	WATCH_OBJECT("stats/stampsPerSecond", &KBEngine::stampsPerSecond);
	WATCH_OBJECT("uid", &KBEngine::getUserUID);
	WATCH_OBJECT("username", &KBEngine::getUsername);
	WATCH_OBJECT("componentType", componentType_);
	WATCH_OBJECT("componentID", componentID_);
	WATCH_OBJECT("globalOrder", this, &ServerApp::globalOrder);
	WATCH_OBJECT("groupOrder", this, &ServerApp::groupOrder);
	WATCH_OBJECT("gametime", this, &ServerApp::time);

	return Mercury::initializeWatcher() && Resmgr::getSingleton().initializeWatcher() &&
		threadPool_.initializeWatcher() && WatchPool::initWatchPools();
}

//-------------------------------------------------------------------------------------		
void ServerApp::queryWatcher(Mercury::Channel* pChannel, MemoryStream& s)
{
	AUTO_SCOPED_PROFILE("watchers");

	std::string path;
	s >> path;

	MemoryStream::SmartPoolObjectPtr readStreamPtr = MemoryStream::createSmartPoolObj();
	WatcherPaths::root().readWatchers(path, readStreamPtr.get()->get());

	MemoryStream::SmartPoolObjectPtr readStreamPtr1 = MemoryStream::createSmartPoolObj();
	WatcherPaths::root().readChildPaths(path, path, readStreamPtr1.get()->get());

	Mercury::Bundle bundle;
	ConsoleInterface::ConsoleWatcherCBMessageHandler msgHandler;
	bundle.newMessage(msgHandler);

	uint8 type = 0;
	bundle << type;
	bundle.append(readStreamPtr.get()->get());
	bundle.send(networkInterface(), pChannel);

	Mercury::Bundle bundle1;
	bundle1.newMessage(msgHandler);

	type = 1;
	bundle1 << type;
	bundle1.append(readStreamPtr1.get()->get());
	bundle1.send(networkInterface(), pChannel);
}

//-------------------------------------------------------------------------------------		
bool ServerApp::initThreadPool()
{
	if(!threadPool_.isInitialize())
	{
		thread::ThreadPool::timeout = int(g_kbeSrvConfig.thread_timeout_);
		threadPool_.createThreadPool(g_kbeSrvConfig.thread_init_create_, 
			g_kbeSrvConfig.thread_pre_create_, g_kbeSrvConfig.thread_max_create_);

		return true;
	}

	return false;
}

//-------------------------------------------------------------------------------------		
void ServerApp::finalise(void)
{
	ProfileGroup::finalise();
	threadPool_.finalise();
	Mercury::finalise();
}

//-------------------------------------------------------------------------------------		
double ServerApp::gameTimeInSeconds() const
{
	return double(g_kbetime) / g_kbeSrvConfig.gameUpdateHertz();
}

//-------------------------------------------------------------------------------------
void ServerApp::handleTimeout(TimerHandle, void * arg)
{
}

//-------------------------------------------------------------------------------------
void ServerApp::handleTimers()
{
	AUTO_SCOPED_PROFILE("callTimers");
	timers().process(g_kbetime);
}

//-------------------------------------------------------------------------------------		
bool ServerApp::run(void)
{
	mainDispatcher_.processUntilBreak();
	return true;
}

//-------------------------------------------------------------------------------------	
void ServerApp::onSignalled(int sigNum)
{
	switch (sigNum)
	{
	case SIGINT:
	case SIGHUP:
		this->shutDown(1.f);
	default:
		break;
	}
}

//-------------------------------------------------------------------------------------	
void ServerApp::onChannelDeregister(Mercury::Channel * pChannel)
{
	if(pChannel->isInternal())
	{
		Componentbridge::getSingleton().onChannelDeregister(pChannel);
	}
}

//-------------------------------------------------------------------------------------	
void ServerApp::onChannelTimeOut(Mercury::Channel * pChannel)
{
	INFO_MSG(fmt::format("ServerApp::onChannelTimeOut: "
		"Channel {0} timed out.\n", pChannel->c_str()));

	networkInterface_.deregisterChannel(pChannel);
	pChannel->destroy();
}

//-------------------------------------------------------------------------------------
void ServerApp::onAddComponent(const Components::ComponentInfos* pInfos)
{
	if(pInfos->componentType == MESSAGELOG_TYPE)
	{
		DebugHelper::getSingleton().registerMessagelog(MessagelogInterface::writeLog.msgID, pInfos->pIntAddr.get());
	}
}

//-------------------------------------------------------------------------------------
void ServerApp::onRemoveComponent(const Components::ComponentInfos* pInfos)
{
	if(pInfos->componentType == MESSAGELOG_TYPE)
	{
		DebugHelper::getSingleton().unregisterMessagelog(MessagelogInterface::writeLog.msgID, pInfos->pIntAddr.get());
	}
	else if(pInfos->componentType == DBMGR_TYPE)
	{
		this->shutDown(0.f);
	}
}

//-------------------------------------------------------------------------------------
void ServerApp::onRegisterNewApp(Mercury::Channel* pChannel, int32 uid, std::string& username, 
						int8 componentType, uint64 componentID, int8 globalorderID, int8 grouporderID,
						uint32 intaddr, uint16 intport, uint32 extaddr, uint16 extport, std::string& extaddrEx)
{
	if(pChannel->isExternal())
		return;

	INFO_MSG(fmt::format("ServerApp::onRegisterNewApp: uid:{0}, username:{1}, componentType:{2}, "
			"componentID:{3}, globalorderID={9}, grouporderID={10}, intaddr:{4}, intport:{5}, extaddr:{6}, extport:{7},  from {8}.\n",
			uid,
			username.c_str(),
			COMPONENT_NAME_EX((COMPONENT_TYPE)componentType), 
			componentID,
			inet_ntoa((struct in_addr&)intaddr),
			ntohs(intport),
			(extaddr != 0 ? inet_ntoa((struct in_addr&)extaddr) : "nonsupport"),
			ntohs(extport),
			pChannel->c_str(),
			((int32)globalorderID),
			((int32)grouporderID)));

	Components::ComponentInfos* cinfos = Componentbridge::getComponents().findComponent((
		KBEngine::COMPONENT_TYPE)componentType, uid, componentID);

	pChannel->componentID(componentID);

	if(cinfos == NULL)
	{
		Componentbridge::getComponents().addComponent(uid, username.c_str(), 
			(KBEngine::COMPONENT_TYPE)componentType, componentID, globalorderID, grouporderID, intaddr, intport, extaddr, extport, extaddrEx, 0,
			0.f, 0.f, 0, 0, 0, 0, 0, pChannel);
	}
	else
	{
		KBE_ASSERT(cinfos->pIntAddr->ip == intaddr && cinfos->pIntAddr->port == intport);
		cinfos->pChannel = pChannel;
	}
}

//-------------------------------------------------------------------------------------
void ServerApp::reqKillServer(Mercury::Channel* pChannel, MemoryStream& s)
{
	if(pChannel->isExternal())
		return;

	COMPONENT_ID componentID;
	COMPONENT_TYPE componentType;
	std::string username;
	int32 uid;
	std::string reason;

	s >> componentID >> componentType >> username >> uid >> reason;

	INFO_MSG(boost::format("ServerApp::reqKillServer: requester(uid:%1%, username:%2%, componentType:%3%, "
				"componentID:%4%, reason:%5%, from %6%)\n") %
				uid % 
				username % 
				COMPONENT_NAME_EX((COMPONENT_TYPE)componentType) % 
				componentID %
				reason %
				pChannel->c_str());

	CRITICAL_MSG("The application was killed!\n");
}

//-------------------------------------------------------------------------------------
void ServerApp::onAppActiveTick(Mercury::Channel* pChannel, COMPONENT_TYPE componentType, COMPONENT_ID componentID)
{
	if(componentType != CLIENT_TYPE)
		if(pChannel->isExternal())
			return;
	
	Mercury::Channel* pTargetChannel = NULL;
	if(componentType != CONSOLE_TYPE && componentType != CLIENT_TYPE)
	{
		Components::ComponentInfos* cinfos = 
			Componentbridge::getComponents().findComponent(componentType, KBEngine::getUserUID(), componentID);

		if(cinfos == NULL || cinfos->pChannel == NULL)
		{
			ERROR_MSG(boost::format("ServerApp::onAppActiveTick[%1%]: %2%:%3% not found.\n") % 
				pChannel % COMPONENT_NAME_EX(componentType) % componentID);

			return;
		}

		pTargetChannel = cinfos->pChannel;
		pTargetChannel->updateLastReceivedTime();
	}
	else
	{
		pChannel->updateLastReceivedTime();
		pTargetChannel = pChannel;
	}

	//DEBUG_MSG("ServerApp::onAppActiveTick[%x]: %s:%"PRAppID" lastReceivedTime:%"PRIu64" at %s.\n", 
	//	pChannel, COMPONENT_NAME_EX(componentType), componentID, pChannel->lastReceivedTime(), pTargetChannel->c_str());
}

//-------------------------------------------------------------------------------------
void ServerApp::reqClose(Mercury::Channel* pChannel)
{
	DEBUG_MSG(boost::format("ServerApp::reqClose: %1%\n") % pChannel->c_str());
	// this->networkInterface().deregisterChannel(pChannel);
	// pChannel->destroy();
}

//-------------------------------------------------------------------------------------
void ServerApp::lookApp(Mercury::Channel* pChannel)
{
	if(pChannel->isExternal())
		return;

	DEBUG_MSG(boost::format("ServerApp::lookApp: %1%\n") % pChannel->c_str());

	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
	
	(*pBundle) << g_componentType;
	(*pBundle) << componentID_;

	ShutdownHandler::SHUTDOWN_STATE state = shuttingdown();
	int8 istate = int8(state);
	(*pBundle) << istate;

	(*pBundle).send(networkInterface(), pChannel);

	Mercury::Bundle::ObjPool().reclaimObject(pBundle);
}

//-------------------------------------------------------------------------------------
void ServerApp::reqCloseServer(Mercury::Channel* pChannel, MemoryStream& s)
{
	DEBUG_MSG(boost::format("ServerApp::reqCloseServer: %1%\n") % pChannel->c_str());

	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
	
	bool success = true;
	(*pBundle) << success;
	(*pBundle).send(networkInterface(), pChannel);

	Mercury::Bundle::ObjPool().reclaimObject(pBundle);

	this->shutDown();
}

//-------------------------------------------------------------------------------------
void ServerApp::queryLoad(Mercury::Channel* pChannel)
{
}

//-------------------------------------------------------------------------------------
void ServerApp::hello(Mercury::Channel* pChannel, MemoryStream& s)
{
	std::string verInfo, scriptVerInfo, encryptedKey;

	s >> verInfo >> scriptVerInfo;
	s.readBlob(encryptedKey);

	char buf[1024];

	if(encryptedKey.size() > 3)
	{
		char *c = buf;

		for (int i=0; i < (int)encryptedKey.size(); i++)
		{
			c += sprintf(c, "%02hhX ", (unsigned char)encryptedKey.data()[i]);
		}

		c[-1] = '\0';
	}
	else
	{
		encryptedKey = "";
		sprintf(buf, "None");
		buf[4] = '\0';
	}

	INFO_MSG(boost::format("ServerApp::onHello: verInfo=%1%, scriptVerInfo=%2%, encryptedKey=%3%, addr:%4%\n") % 
		verInfo % scriptVerInfo % buf % pChannel->c_str());

	if(verInfo != KBEVersion::versionString())
		onVersionNotMatch(pChannel);
	else if(scriptVerInfo != KBEVersion::scriptVersionString())
		onScriptVersionNotMatch(pChannel);
	else
		onHello(pChannel, verInfo, scriptVerInfo, encryptedKey);
}

//-------------------------------------------------------------------------------------
void ServerApp::onHello(Mercury::Channel* pChannel, 
						const std::string& verInfo, 
						const std::string& scriptVerInfo, 
						const std::string& encryptedKey)
{
}

//-------------------------------------------------------------------------------------
void ServerApp::onVersionNotMatch(Mercury::Channel* pChannel)
{
}

//-------------------------------------------------------------------------------------
void ServerApp::onScriptVersionNotMatch(Mercury::Channel* pChannel)
{
}

//-------------------------------------------------------------------------------------
void ServerApp::startProfile(Mercury::Channel* pChannel, KBEngine::MemoryStream& s)
{
	std::string profileName;
	int8 profileType;
	uint32 timelen;

	s >> profileName >> profileType >> timelen;

	startProfile_(pChannel, profileName, profileType, timelen);
}

//-------------------------------------------------------------------------------------
void ServerApp::startProfile_(Mercury::Channel* pChannel, std::string profileName, int8 profileType, uint32 timelen)
{
	switch(profileType)
	{
	case 1:	// cprofile
		new CProfileHandler(this->networkInterface(), timelen, profileName, pChannel->addr());
		break;
	case 2:	// eventprofile
		new EventProfileHandler(this->networkInterface(), timelen, profileName, pChannel->addr());
		break;
	case 3:	// mercuryprofile
		new MercuryProfileHandler(this->networkInterface(), timelen, profileName, pChannel->addr());
		break;
	default:
		ERROR_MSG(boost::format("ServerApp::startProfile_: type(%1%:%2%) not support!\n") % 
			profileType % profileName);

		break;
	};
}

//-------------------------------------------------------------------------------------		
}
