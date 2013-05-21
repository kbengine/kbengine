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
#include "server/serverconfig.hpp"
#include "server/componentbridge.hpp"
#include "network/channel.hpp"
#include "network/bundle.hpp"
#include "network/common.hpp"
#include "cstdkbe/memorystream.hpp"
#include "helper/console_helper.hpp"
#include "resmgr/resmgr.hpp"

#include "../../server/baseappmgr/baseappmgr_interface.hpp"
#include "../../server/cellappmgr/cellappmgr_interface.hpp"
#include "../../server/baseapp/baseapp_interface.hpp"
#include "../../server/cellapp/cellapp_interface.hpp"
#include "../../server/dbmgr/dbmgr_interface.hpp"
#include "../../server/loginapp/loginapp_interface.hpp"
#include "../../server/resourcemgr/resourcemgr_interface.hpp"
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
Mercury::ChannelTimeOutHandler(),
Components::ComponentsNotificationHandler(),
componentType_(componentType),
componentID_(componentID),
mainDispatcher_(dispatcher),
networkInterface_(ninterface),
timers_(),
startGlobalOrder_(-1),
startGroupOrder_(-1),
pActiveTimerHandle_(),
threadPool_()
{
	networkInterface_.pExtensionData(this);
	networkInterface_.pChannelTimeOutHandler(this);
	networkInterface_.pChannelDeregisterHandler(this);

	startActiveTick(KBE_MAX(1.f, Mercury::g_channelInternalTimeout / 2.0f));

	// 默认所有app都设置为这个值， 如果需要调整则各自在派生类重新赋值
	ProfileVal::setWarningPeriod(stampsPerSecond() / g_kbeSrvConfig.gameUpdateHertz());
}

//-------------------------------------------------------------------------------------
ServerApp::~ServerApp()
{
	pActiveTimerHandle_.cancel();
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
	this->getMainDispatcher().addFrequentTask(&Componentbridge::getSingleton());

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
		threadPool_.initializeWatcher();
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
	bundle.send(getNetworkInterface(), pChannel);

	Mercury::Bundle bundle1;
	bundle1.newMessage(msgHandler);

	type = 1;
	bundle1 << type;
	bundle1.append(readStreamPtr1.get()->get());
	bundle1.send(getNetworkInterface(), pChannel);
}

//-------------------------------------------------------------------------------------		
bool ServerApp::initThreadPool()
{
	if(!threadPool_.isInitialize())
	{
		threadPool_.createThreadPool(1, 2, 8);
		return true;
	}

	return false;
}

//-------------------------------------------------------------------------------------		
void ServerApp::finalise(void)
{
	ProfileGroup::finalise();
	threadPool_.finalise();
	pActiveTimerHandle_.cancel();

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
	switch (reinterpret_cast<uintptr>(arg))
	{
		case TIMEOUT_ACTIVE_TICK:
		{
			int8 findComponentTypes[] = {BASEAPPMGR_TYPE, CELLAPPMGR_TYPE, DBMGR_TYPE, CELLAPP_TYPE, 
								BASEAPP_TYPE, LOGINAPP_TYPE, MESSAGELOG_TYPE, RESOURCEMGR_TYPE, UNKNOWN_COMPONENT_TYPE};
			
			int ifind = 0;
			while(findComponentTypes[ifind] != UNKNOWN_COMPONENT_TYPE)
			{
				COMPONENT_TYPE componentType = (COMPONENT_TYPE)findComponentTypes[ifind];

				Components::COMPONENTS& components = Components::getSingleton().getComponents(componentType);
				Components::COMPONENTS::iterator iter = components.begin();
				for(; iter != components.end(); iter++)
				{
					Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
					COMMON_MERCURY_MESSAGE(componentType, (*pBundle), onAppActiveTick);
					
					(*pBundle) << g_componentType;
					(*pBundle) << componentID_;
					if((*iter).pChannel != NULL)
						(*pBundle).send(getNetworkInterface(), (*iter).pChannel);

					Mercury::Bundle::ObjPool().reclaimObject(pBundle);
				}

				ifind++;
			}
			break;
		}
		default:
			break;
	}
}

//-------------------------------------------------------------------------------------
void ServerApp::startActiveTick(float period)
{
	pActiveTimerHandle_.cancel();
	pActiveTimerHandle_ = getMainDispatcher().addTimer(int(period * 1000000),
									this, (void *)TIMEOUT_ACTIVE_TICK);
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
void ServerApp::shutDown()
{
	INFO_MSG( "ServerApp::shutDown: shutting down\n" );
	mainDispatcher_.breakProcessing();
}

//-------------------------------------------------------------------------------------	
void ServerApp::onSignalled(int sigNum)
{
	switch (sigNum)
	{
	case SIGINT:
	case SIGHUP:
		this->shutDown();
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
	INFO_MSG(boost::format("ServerApp::onChannelTimeOut: "
		"Channel %1% timed out.\n") % pChannel->c_str());

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
}

//-------------------------------------------------------------------------------------
void ServerApp::onRegisterNewApp(Mercury::Channel* pChannel, int32 uid, std::string& username, 
						int8 componentType, uint64 componentID, int8 globalorderID, int8 grouporderID,
						uint32 intaddr, uint16 intport, uint32 extaddr, uint16 extport)
{
	if(pChannel->isExternal())
		return;

	INFO_MSG(boost::format("ServerApp::onRegisterNewApp: uid:%1%, username:%2%, componentType:%3%, "
			"componentID:%4%, globalorderID=%10%, grouporderID=%11%, intaddr:%5%, intport:%6%, extaddr:%7%, extport:%8%,  from %9%.\n") %
			uid % 
			username.c_str() % 
			COMPONENT_NAME_EX((COMPONENT_TYPE)componentType) % 
			componentID %
			inet_ntoa((struct in_addr&)intaddr) %
			ntohs(intport) %
			(extaddr != 0 ? inet_ntoa((struct in_addr&)extaddr) : "nonsupport") %
			ntohs(extport) %
			pChannel->c_str() %
			((int32)globalorderID) % 
			((int32)grouporderID));

	Components::ComponentInfos* cinfos = Componentbridge::getComponents().findComponent((
		KBEngine::COMPONENT_TYPE)componentType, uid, componentID);

	pChannel->componentID(componentID);

	if(cinfos == NULL)
	{
		Componentbridge::getComponents().addComponent(uid, username.c_str(), 
			(KBEngine::COMPONENT_TYPE)componentType, componentID, globalorderID, grouporderID, intaddr, intport, extaddr, extport, pChannel);
	}
	else
	{
		KBE_ASSERT(cinfos->pIntAddr->ip == intaddr && cinfos->pIntAddr->port == intport);
		cinfos->pChannel = pChannel;
	}
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

		if(cinfos == NULL)
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
	// this->getNetworkInterface().deregisterChannel(pChannel);
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
	(*pBundle).send(getNetworkInterface(), pChannel);

	Mercury::Bundle::ObjPool().reclaimObject(pBundle);
}

//-------------------------------------------------------------------------------------
void ServerApp::reqCloseServer(Mercury::Channel* pChannel, MemoryStream& s)
{
	DEBUG_MSG(boost::format("ServerApp::reqCloseServer: %1%\n") % pChannel->c_str());

	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
	
	bool success = true;
	(*pBundle) << success;
	(*pBundle).send(getNetworkInterface(), pChannel);

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
	std::string verInfo, encryptedKey;

	s >> verInfo;
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

	INFO_MSG(boost::format("ServerApp::onHello: verInfo=%1%, encryptedKey=%2%, addr:%3%\n") % 
		verInfo % buf % pChannel->c_str());

	onHello(pChannel, verInfo, encryptedKey);
}

//-------------------------------------------------------------------------------------
void ServerApp::onHello(Mercury::Channel* pChannel, 
						const std::string& verInfo, 
						const std::string& encryptedKey)
{
}

//-------------------------------------------------------------------------------------		
}
