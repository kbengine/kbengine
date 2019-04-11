// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#include "serverapp.h"
#include "server/component_active_report_handler.h"
#include "server/shutdowner.h"
#include "server/serverconfig.h"
#include "server/components.h"
#include "network/channel.h"
#include "network/bundle.h"
#include "network/common.h"
#include "common/memorystream.h"
#include "helper/console_helper.h"
#include "helper/sys_info.h"
#include "helper/watch_pools.h"
#include "resmgr/resmgr.h"

#include "../../server/baseappmgr/baseappmgr_interface.h"
#include "../../server/cellappmgr/cellappmgr_interface.h"
#include "../../server/baseapp/baseapp_interface.h"
#include "../../server/cellapp/cellapp_interface.h"
#include "../../server/dbmgr/dbmgr_interface.h"
#include "../../server/loginapp/loginapp_interface.h"
#include "../../server/tools/logger/logger_interface.h"
#include "../../server/tools/interfaces/interfaces_interface.h"

namespace KBEngine{
COMPONENT_TYPE g_componentType = UNKNOWN_COMPONENT_TYPE;
COMPONENT_ID g_componentID = 0;
COMPONENT_ORDER g_componentGlobalOrder = -1;
COMPONENT_ORDER g_componentGroupOrder = -1;
COMPONENT_GUS g_genuuid_sections = -1;

GAME_TIME g_kbetime = 0;

//-------------------------------------------------------------------------------------
ServerApp::ServerApp(Network::EventDispatcher& dispatcher, 
					 Network::NetworkInterface& ninterface, 
					 COMPONENT_TYPE componentType,
					 COMPONENT_ID componentID):
SignalHandler(),
TimerHandler(),
ShutdownHandler(),
Network::ChannelTimeOutHandler(),
Components::ComponentsNotificationHandler(),
componentType_(componentType),
componentID_(componentID),
dispatcher_(dispatcher),
networkInterface_(ninterface),
timers_(),
startGlobalOrder_(-1),
startGroupOrder_(-1),
pShutdowner_(NULL),
pActiveTimerHandle_(NULL),
threadPool_()
{
	networkInterface_.pChannelTimeOutHandler(this);
	networkInterface_.pChannelDeregisterHandler(this);

	// 广播自己的地址给网上上的所有kbemachine
	// 并且从kbemachine获取basappmgr和cellappmgr以及dbmgr地址
	Components::getSingleton().pHandler(this);
	this->dispatcher().addTask(&Components::getSingleton());
	
	pActiveTimerHandle_ = new ComponentActiveReportHandler(this);
	pActiveTimerHandle_->startActiveTick(KBE_MAX(1.f, Network::g_channelInternalTimeout / 2.0f));

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
	{
		pShutdowner_ = new Shutdowner(this);
	}
	else
	{
		WARNING_MSG(fmt::format("ServerApp::shutDown:  In shuttingdown!\n"));
		return;
	}
	
	pShutdowner_->shutdown(shutdowntime < 0.f ? g_kbeSrvConfig.shutdowntime() : shutdowntime, 
		g_kbeSrvConfig.shutdownWaitTickTime(), dispatcher_);
}

//-------------------------------------------------------------------------------------
void ServerApp::onShutdownBegin()
{
#if KBE_PLATFORM == PLATFORM_WIN32
	printf("[INFO]: shutdown begin.\n");
#endif

	dispatcher_.setWaitBreakProcessing();
}

//-------------------------------------------------------------------------------------
void ServerApp::onShutdown(bool first)
{
}

//-------------------------------------------------------------------------------------
void ServerApp::onShutdownEnd()
{
	dispatcher_.breakProcessing();
}

//-------------------------------------------------------------------------------------
bool ServerApp::loadConfig()
{
	return true;
}

//-------------------------------------------------------------------------------------		
bool ServerApp::installSignals()
{
	g_kbeSignalHandlers.attachApp(this);
	g_kbeSignalHandlers.ignoreSignal(SIGPIPE);
	g_kbeSignalHandlers.addSignal(SIGINT, this);
	g_kbeSignalHandlers.addSignal(SIGHUP, this);
	return true;
}

//-------------------------------------------------------------------------------------		
bool ServerApp::initialize()
{
	if (!installSignals())
		return false;

	if(!initThreadPool())
		return false;
	
	if(!loadConfig())
		return false;
	
	if(!initializeBegin())
		return false;
	
	if(!inInitialize())
		return false;

	bool ret = initializeEnd();

	// 最后仍然需要设置一次，避免期间被其他第三方库修改
	if (!installSignals())
		return false;

#ifdef ENABLE_WATCHERS
	return ret && Network::initialize() && initializeWatcher();
#else
	return ret && Network::initialize();
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

	return Network::initializeWatcher() && Resmgr::getSingleton().initializeWatcher() &&
		threadPool_.initializeWatcher() && WatchPool::initWatchPools();
}

//-------------------------------------------------------------------------------------		
void ServerApp::queryWatcher(Network::Channel* pChannel, MemoryStream& s)
{
	if(pChannel->isExternal())
		return;
	
	AUTO_SCOPED_PROFILE("watchers");

	std::string path;
	s >> path;

	MemoryStream::SmartPoolObjectPtr readStreamPtr = MemoryStream::createSmartPoolObj(OBJECTPOOL_POINT);
	WatcherPaths::root().readWatchers(path, readStreamPtr.get()->get());

	MemoryStream::SmartPoolObjectPtr readStreamPtr1 = MemoryStream::createSmartPoolObj(OBJECTPOOL_POINT);
	WatcherPaths::root().readChildPaths(path, path, readStreamPtr1.get()->get());

	Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);
	ConsoleInterface::ConsoleWatcherCBMessageHandler msgHandler;
	(*pBundle).newMessage(msgHandler);

	uint8 type = 0;
	(*pBundle) << type;
	(*pBundle).append(readStreamPtr.get()->get());
	pChannel->send(pBundle);

	Network::Bundle* pBundle1 = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);
	(*pBundle1).newMessage(msgHandler);

	type = 1;
	(*pBundle1) << type;
	(*pBundle1).append(readStreamPtr1.get()->get());
	pChannel->send(pBundle1);
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
	Network::finalise();
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
	AUTO_SCOPED_PROFILE("callScriptTimers");
	timers().process(g_kbetime);
}

//-------------------------------------------------------------------------------------		
bool ServerApp::run(void)
{
	dispatcher_.processUntilBreak();
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
void ServerApp::onChannelDeregister(Network::Channel * pChannel)
{
	if(pChannel->isInternal())
	{
		Components::getSingleton().onChannelDeregister(pChannel, this->isShuttingdown());
	}
}

//-------------------------------------------------------------------------------------	
void ServerApp::onChannelTimeOut(Network::Channel * pChannel)
{
	INFO_MSG(fmt::format("ServerApp::onChannelTimeOut: "
		"Channel {0} timeout!\n", pChannel->c_str()));

	pChannel->condemn("timedout");
	networkInterface_.deregisterChannel(pChannel);
	pChannel->destroy();
	Network::Channel::reclaimPoolObject(pChannel);
}

//-------------------------------------------------------------------------------------
void ServerApp::onAddComponent(const Components::ComponentInfos* pInfos)
{
	if(pInfos->componentType == LOGGER_TYPE)
	{
		DebugHelper::getSingleton().registerLogger(LoggerInterface::writeLog.msgID, pInfos->pIntAddr.get());
	}
}

//-------------------------------------------------------------------------------------
void ServerApp::onIdentityillegal(COMPONENT_TYPE componentType, COMPONENT_ID componentID, uint32 pid, const char* pAddr)
{
	ERROR_MSG(fmt::format("ServerApp::onIdentityillegal: The current process and {}(componentID={} ->conflicted???, pid={}, addr={}) conflict, the process will exit!\n"
			"Can modify the components-CID and UID to avoid conflict.\n",
		COMPONENT_NAME_EX((COMPONENT_TYPE)componentType), componentID, pid, pAddr));

	this->shutDown(1.f);
}

//-------------------------------------------------------------------------------------
void ServerApp::onRemoveComponent(const Components::ComponentInfos* pInfos)
{
	if(pInfos->componentType == LOGGER_TYPE)
	{
		DebugHelper::getSingleton().unregisterLogger(LoggerInterface::writeLog.msgID, pInfos->pIntAddr.get());
	}
	else if(pInfos->componentType == DBMGR_TYPE)
	{
		if(g_componentType != MACHINE_TYPE && 
			g_componentType != LOGGER_TYPE && 
			g_componentType != INTERFACES_TYPE &&
			g_componentType != BOTS_TYPE &&
			g_componentType != WATCHER_TYPE)
			this->shutDown(0.f);
	}
	else if (pInfos->componentType == CELLAPPMGR_TYPE)
	{
		if (g_componentType == CELLAPP_TYPE)
			this->shutDown(1.f);
	}
	else if (pInfos->componentType == BASEAPPMGR_TYPE)
	{
		if (g_componentType == BASEAPP_TYPE)
			this->shutDown(1.f);
	}
}

//-------------------------------------------------------------------------------------
void ServerApp::onRegisterNewApp(Network::Channel* pChannel, int32 uid, std::string& username, 
						COMPONENT_TYPE componentType, COMPONENT_ID componentID, COMPONENT_ORDER globalorderID, COMPONENT_ORDER grouporderID,
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

	Components::ComponentInfos* cinfos = Components::getSingleton().findComponent((
		KBEngine::COMPONENT_TYPE)componentType, uid, componentID);

	pChannel->componentID(componentID);

	if(cinfos == NULL)
	{
		Components::getSingleton().addComponent(uid, username.c_str(), 
			(KBEngine::COMPONENT_TYPE)componentType, componentID, globalorderID, grouporderID, 0, intaddr, intport, extaddr, extport, extaddrEx, 0,
			0.f, 0.f, 0, 0, 0, 0, 0, pChannel);
	}
	else
	{
		if (!(cinfos->pIntAddr->ip == intaddr && cinfos->pIntAddr->port == intport))
		{
			ERROR_MSG(fmt::format("ServerApp::onRegisterNewApp: error component(uid:{}, username:{}, componentType:{}, componentID:{}, from {})!\n",
				uid,
				username.c_str(),
				COMPONENT_NAME_EX((COMPONENT_TYPE)componentType), componentID, pChannel->c_str()));

			return;
		}

		cinfos->pChannel = pChannel;
	}
}

//-------------------------------------------------------------------------------------
void ServerApp::reqKillServer(Network::Channel* pChannel, MemoryStream& s)
{
	if(pChannel->isExternal())
		return;

	COMPONENT_ID componentID;
	COMPONENT_TYPE componentType;
	std::string username;
	int32 uid;
	std::string reason;

	s >> componentID >> componentType >> username >> uid >> reason;

	INFO_MSG(fmt::format("ServerApp::reqKillServer: requester(uid:{}, username:{}, componentType:{}, "
				"componentID:{}, reason:{}, from {})\n",
				uid, 
				username, 
				COMPONENT_NAME_EX((COMPONENT_TYPE)componentType),
				componentID,
				reason,
				pChannel->c_str()));

	CRITICAL_MSG("The application was killed!\n");
}

//-------------------------------------------------------------------------------------
void ServerApp::onAppActiveTick(Network::Channel* pChannel, COMPONENT_TYPE componentType, COMPONENT_ID componentID)
{
	if(componentType != CLIENT_TYPE)
		if(pChannel->isExternal())
			return;
	
	pChannel->updateLastReceivedTime();
	
	if(componentType != CONSOLE_TYPE && componentType != CLIENT_TYPE)
	{
		Components::ComponentInfos* cinfos = 
			Components::getSingleton().findComponent(componentType, KBEngine::getUserUID(), componentID);

		if(cinfos == NULL || cinfos->pChannel == NULL)
		{
			ERROR_MSG(fmt::format("ServerApp::onAppActiveTick[{:p}]: {}:{} not found.\n", 
				(void*)pChannel, COMPONENT_NAME_EX(componentType), componentID));

			return;
		}

		cinfos->pChannel->updateLastReceivedTime();
	}

	//DEBUG_MSG(fmt::format("ServerApp::onAppActiveTick[{:p}]: {}:{} lastReceivedTime:{} at {}.\n",
	//	(void*)pChannel, COMPONENT_NAME_EX(componentType), componentID, pChannel->lastReceivedTime(), pChannel->c_str()));
}

//-------------------------------------------------------------------------------------
void ServerApp::reqClose(Network::Channel* pChannel)
{
	if(pChannel->isExternal())
		return;
	
	DEBUG_MSG(fmt::format("ServerApp::reqClose: {}\n", pChannel->c_str()));
	// this->networkInterface().deregisterChannel(pChannel);
	// pChannel->destroy();
}

//-------------------------------------------------------------------------------------
void ServerApp::lookApp(Network::Channel* pChannel)
{
	if(pChannel->isExternal())
		return;

	//DEBUG_MSG(fmt::format("ServerApp::lookApp: {}, componentID={}\n", pChannel->c_str(), g_componentID));

	Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);
	
	(*pBundle) << g_componentType;
	(*pBundle) << componentID_;

	ShutdownHandler::SHUTDOWN_STATE state = shuttingdown();
	int8 istate = int8(state);
	(*pBundle) << istate;

	pChannel->send(pBundle);
	//DEBUG_MSG(fmt::format("ServerApp::lookApp: response! componentID={}\n", g_componentID));
}

//-------------------------------------------------------------------------------------
void ServerApp::reqCloseServer(Network::Channel* pChannel, MemoryStream& s)
{
	if(pChannel->isExternal())
		return;
	
	DEBUG_MSG(fmt::format("ServerApp::reqCloseServer: {}\n", pChannel->c_str()));

	Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);
	
	bool success = true;
	(*pBundle) << success;
	pChannel->send(pBundle);
	this->shutDown();
}

//-------------------------------------------------------------------------------------
void ServerApp::queryLoad(Network::Channel* pChannel)
{
	if(pChannel->isExternal())
		return;
}

//-------------------------------------------------------------------------------------
void ServerApp::hello(Network::Channel* pChannel, MemoryStream& s)
{
	std::string verInfo, scriptVerInfo, encryptedKey;

	s >> verInfo >> scriptVerInfo;
	s.readBlob(encryptedKey);

	char buf[MAX_BUF];
	std::string encryptedKey_str;

	if (encryptedKey.size() > 3 && encryptedKey.size() <= 65535)
	{
		for (int i = 0; i < (int)encryptedKey.size(); ++i)
		{
			memset(buf, 0, MAX_BUF);
			kbe_snprintf(buf, MAX_BUF / 2, "%02hhX ", (unsigned char)encryptedKey.data()[i]);
			encryptedKey_str += buf;
		}
	}
	else
	{
		encryptedKey = "";
		encryptedKey_str = "None";
	}

	INFO_MSG(fmt::format("ServerApp::onHello: verInfo={}, scriptVerInfo={}, encryptedKey={}, addr:{}\n", 
		verInfo, scriptVerInfo, encryptedKey_str, pChannel->c_str()));

	if(verInfo != KBEVersion::versionString())
		onVersionNotMatch(pChannel);
	else if(scriptVerInfo != KBEVersion::scriptVersionString())
		onScriptVersionNotMatch(pChannel);
	else
		onHello(pChannel, verInfo, scriptVerInfo, encryptedKey);
}

//-------------------------------------------------------------------------------------
void ServerApp::onHello(Network::Channel* pChannel, 
						const std::string& verInfo, 
						const std::string& scriptVerInfo, 
						const std::string& encryptedKey)
{
}

//-------------------------------------------------------------------------------------
void ServerApp::onVersionNotMatch(Network::Channel* pChannel)
{
}

//-------------------------------------------------------------------------------------
void ServerApp::onScriptVersionNotMatch(Network::Channel* pChannel)
{
}

//-------------------------------------------------------------------------------------
void ServerApp::startProfile(Network::Channel* pChannel, KBEngine::MemoryStream& s)
{
	if(pChannel->isExternal())
		return;
	
	std::string profileName;
	int8 profileType;
	uint32 timelen;

	s >> profileName >> profileType >> timelen;

	startProfile_(pChannel, profileName, profileType, timelen);
}

//-------------------------------------------------------------------------------------
void ServerApp::startProfile_(Network::Channel* pChannel, std::string profileName, int8 profileType, uint32 timelen)
{
	if(pChannel->isExternal())
		return;
	
	switch(profileType)
	{
	case 1:	// cprofile
		new CProfileHandler(this->networkInterface(), timelen, profileName, pChannel->addr());
		break;
	case 2:	// eventprofile
		new EventProfileHandler(this->networkInterface(), timelen, profileName, pChannel->addr());
		break;
	case 3:	// networkprofile
		new NetworkProfileHandler(this->networkInterface(), timelen, profileName, pChannel->addr());
		break;
	default:
		ERROR_MSG(fmt::format("ServerApp::startProfile_: type({}:{}) not support!\n", 
			profileType, profileName));

		break;
	};
}

//-------------------------------------------------------------------------------------		
}
