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
#include "thread/threadpool.hpp"
#include "entitydef/entity_mailbox.hpp"
#include "entitydef/entitydef.hpp"
#include "server/componentbridge.hpp"
#include "server/serverconfig.hpp"
#include "helper/profile.hpp"

#include "../../server/baseapp/baseapp_interface.hpp"
#include "../../server/loginapp/loginapp_interface.hpp"

namespace KBEngine{

COMPONENT_TYPE g_componentType = UNKNOWN_COMPONENT_TYPE;
COMPONENT_ID g_componentID = 0;
COMPONENT_ORDER g_componentOrder = 1;
GAME_TIME g_kbetime = 0;

KBE_SINGLETON_INIT(ClientApp);
//-------------------------------------------------------------------------------------
ClientApp::ClientApp(Mercury::EventDispatcher& dispatcher, 
					 Mercury::NetworkInterface& ninterface, 
					 COMPONENT_TYPE componentType,
					 COMPONENT_ID componentID):
TimerHandler(),
Mercury::ChannelTimeOutHandler(),
script_(),
scriptBaseTypes_(),
gameTimer_(),
componentType_(componentType),
componentID_(componentID),
mainDispatcher_(dispatcher),
networkInterface_(ninterface),
timers_(),
threadPool_(),
serverChannel_(NULL),
pyCallbackMgr_(),
ip_(),
port_(),
lastSentActiveTickTime_(timestamp()),
connectedGateway_(false)
{
	networkInterface_.pExtensionData(this);
	networkInterface_.pChannelTimeOutHandler(this);
	networkInterface_.pChannelDeregisterHandler(this);

	// 初始化mailbox模块获取channel函数地址
	EntityMailbox::setFindChannelFunc(std::tr1::bind(&ClientApp::findChannelByMailbox, this, 
		std::tr1::placeholders::_1));
}

//-------------------------------------------------------------------------------------
ClientApp::~ClientApp()
{
	serverChannel_ = NULL;
}

//-------------------------------------------------------------------------------------
bool ClientApp::loadConfig()
{
	return true;
}

//-------------------------------------------------------------------------------------
int ClientApp::registerPyObjectToScript(const char* attrName, PyObject* pyObj)
{ 
	return script_.registerToModule(attrName, pyObj); 
}

//-------------------------------------------------------------------------------------
int ClientApp::unregisterPyObjectToScript(const char* attrName)
{ 
	return script_.unregisterToModule(attrName); 
}

//-------------------------------------------------------------------------------------		
bool ClientApp::installSingnals()
{
	return true;
}

//-------------------------------------------------------------------------------------		
bool ClientApp::initialize()
{
	if(!threadPool_.isInitialize())
		threadPool_.createThreadPool(4, 4, 256);

	if(!installSingnals())
		return false;
	
	if(!loadConfig())
		return false;
	
	if(!initializeBegin())
		return false;

	gameTimer_ = this->getMainDispatcher().addTimer(1000000 / g_kbeSrvConfig.gameUpdateHertz(), this,
							reinterpret_cast<void *>(TIMEOUT_GAME_TICK));

	if(!installPyScript())
		return false;

	if(!installPyModules())
		return false;
	
	if(!installEntityDef())
		return false;

	ProfileVal::setWarningPeriod(stampsPerSecond() / g_kbeSrvConfig.gameUpdateHertz());

	if(!inInitialize())
		return false;

	return initializeEnd();
}


//-------------------------------------------------------------------------------------
bool ClientApp::installEntityDef()
{
	if(!EntityDef::installScript(getScript().getModule()))
		return false;

	// 初始化数据类别
	// demo/res/scripts/entity_defs/alias.xml
	if(!DataTypes::initialize("scripts/entity_defs/alias.xml"))
		return false;

	// 初始化所有扩展模块
	// demo/res/scripts/
	if(!EntityDef::initialize(Resmgr::getSingleton().respaths()[1] + "res/scripts/", scriptBaseTypes_, g_componentType)){
		return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool ClientApp::installPyScript()
{
	if(Resmgr::getSingleton().respaths().size() <= 0)
	{
		ERROR_MSG("ClientApp::installPyScript: KBE_RES_PATH is error!\n");
		return false;
	}

	std::wstring root_path = L"";
	wchar_t* tbuf = KBEngine::strutil::char2wchar(const_cast<char*>(Resmgr::getSingleton().respaths()[1].c_str()));
	if(tbuf != NULL)
	{
		root_path += tbuf;
		free(tbuf);
	}
	else
	{
		return false;
	}

	std::wstring pyPaths = root_path + L"res/scripts/common;";
	pyPaths += root_path + L"res/scripts/data;";
	pyPaths += root_path + L"res/scripts/user_type;";
	pyPaths += root_path + L"res/scripts/client;";
	
	std::string kbe_res_path = Resmgr::getSingleton().respaths()[0].c_str();
	kbe_res_path += "scripts/common";

	tbuf = KBEngine::strutil::char2wchar(const_cast<char*>(kbe_res_path.c_str()));
	bool ret = getScript().install(tbuf, pyPaths, "KBEngine", componentType_);
	// 此处经测试传入python之后被python释放了
	// free(tbuf);
	return ret;
}

//-------------------------------------------------------------------------------------
void ClientApp::registerScript(PyTypeObject* pto)
{
	scriptBaseTypes_.push_back(pto);
}

//-------------------------------------------------------------------------------------
bool ClientApp::uninstallPyScript()
{
	return uninstallPyModules() && getScript().uninstall();
}

//-------------------------------------------------------------------------------------
bool ClientApp::installPyModules()
{
	EntityDef::installScript(getScript().getModule());
//	Entity::installScript(getScript().getModule());
//	Entities<Entity>::installScript(NULL);
//	registerScript(Entity::getScriptType());

	onInstallPyModules();
	return true;
}

//-------------------------------------------------------------------------------------
bool ClientApp::uninstallPyModules()
{
	EntityDef::uninstallScript();

//	Entity::uninstallScript();
//	Entities<Entity>::uninstallScript();
	return true;
}

//-------------------------------------------------------------------------------------		
void ClientApp::finalise(void)
{
	gameTimer_.cancel();
	threadPool_.finalise();
}

//-------------------------------------------------------------------------------------		
double ClientApp::gameTimeInSeconds() const
{
	return double(g_kbetime) / 10;
}

//-------------------------------------------------------------------------------------
void ClientApp::handleTimeout(TimerHandle, void * arg)
{
	switch (reinterpret_cast<uintptr>(arg))
	{
		case TIMEOUT_GAME_TICK:
		{
			handleGameTick();
		}
		default:
			break;
	}
}

//-------------------------------------------------------------------------------------
void ClientApp::handleGameTick()
{
	g_kbetime++;
	threadPool_.onMainThreadTick();
	handleTimers();

	getNetworkInterface().handleChannels(KBEngine::Mercury::MessageHandlers::pMainMessageHandlers);

	if(serverChannel_)
	{
		sendTick();
		serverChannel_->send();
	}
}

//-------------------------------------------------------------------------------------
void ClientApp::handleTimers()
{
	timers().process(g_kbetime);
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
	INFO_MSG(boost::format("ClientApp::onChannelTimeOut: "
		"Channel %1% timed out.\n") % pChannel->c_str());

	networkInterface_.deregisterChannel(pChannel);
	pChannel->destroy();
}

//-------------------------------------------------------------------------------------
void ClientApp::sendTick()
{
	if(serverChannel_)
		return;

	// 向服务器发送tick
	uint64 check = uint64( Mercury::g_channelExternalTimeout * stampsPerSecond() ) / 2;
	if (timestamp() - lastSentActiveTickTime_ > check)
	{
		lastSentActiveTickTime_ = timestamp();

		Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
		if(connectedGateway_)
			(*pBundle).newMessage(BaseappInterface::onClientActiveTick);
		else
			(*pBundle).newMessage(LoginappInterface::onClientActiveTick);

		serverChannel_->bundles().push_back(pBundle);
	}
}

//-------------------------------------------------------------------------------------	
Mercury::Channel* ClientApp::findChannelByMailbox(EntityMailbox& mailbox)
{
	return serverChannel_;
}

//-------------------------------------------------------------------------------------	
void ClientApp::onCreateAccountResult(Mercury::Channel * pChannel, MemoryStream& s)
{
	SERVER_ERROR_CODE failedcode;
	std::string datas;

	s >> failedcode;
	s.readBlob(datas);
}

//-------------------------------------------------------------------------------------	
void ClientApp::onLoginSuccessfully(Mercury::Channel * pChannel, MemoryStream& s)
{
}

//-------------------------------------------------------------------------------------	
void ClientApp::onLoginFailed(Mercury::Channel * pChannel, MemoryStream& s)
{
}

//-------------------------------------------------------------------------------------	
void ClientApp::onLoginGatewayFailed(Mercury::Channel * pChannel, SERVER_ERROR_CODE failedcode)
{
}

//-------------------------------------------------------------------------------------	
void ClientApp::onCreatedProxies(Mercury::Channel * pChannel, 
								 uint64 rndUUID, ENTITY_ID eid, std::string& entityType)
{
}

//-------------------------------------------------------------------------------------	
void ClientApp::onEntityEnterWorld(Mercury::Channel * pChannel, ENTITY_ID eid, 
								   ENTITY_SCRIPT_UID scriptType, SPACE_ID spaceID)
{
}

//-------------------------------------------------------------------------------------	
void ClientApp::onEntityLeaveWorld(Mercury::Channel * pChannel, ENTITY_ID eid, SPACE_ID spaceID)
{
}

//-------------------------------------------------------------------------------------	
void ClientApp::onEntityEnterSpace(Mercury::Channel * pChannel, SPACE_ID spaceID, ENTITY_ID eid)
{
}

//-------------------------------------------------------------------------------------	
void ClientApp::onEntityLeaveSpace(Mercury::Channel * pChannel, SPACE_ID spaceID, ENTITY_ID eid)
{
}

//-------------------------------------------------------------------------------------	
void ClientApp::onEntityDestroyed(Mercury::Channel * pChannel, ENTITY_ID eid)
{
}

//-------------------------------------------------------------------------------------
void ClientApp::onRemoteMethodCall(Mercury::Channel* pChannel, KBEngine::MemoryStream& s)
{
}

//-------------------------------------------------------------------------------------
void ClientApp::onUpdatePropertys(Mercury::Channel* pChannel, MemoryStream& s)
{
}

//-------------------------------------------------------------------------------------
void ClientApp::onStreamDataStarted(Mercury::Channel* pChannel, int16 id, uint32 datasize, std::string& descr)
{
}

//-------------------------------------------------------------------------------------
void ClientApp::onStreamDataRecv(Mercury::Channel* pChannel, MemoryStream& s)
{
}

//-------------------------------------------------------------------------------------
void ClientApp::onStreamDataCompleted(Mercury::Channel* pChannel, int16 id)
{
}

//-------------------------------------------------------------------------------------		
}
