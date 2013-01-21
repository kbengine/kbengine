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

#include "pybots.hpp"
#include "bots.hpp"
#include "entity.hpp"
#include "clientobject.hpp"
#include "bots_interface.hpp"
#include "resmgr/resmgr.hpp"
#include "network/common.hpp"
#include "network/tcp_packet.hpp"
#include "network/udp_packet.hpp"
#include "network/message_handler.hpp"
#include "thread/threadpool.hpp"
#include "server/componentbridge.hpp"
#include "server/serverconfig.hpp"
#include "helper/console_helper.hpp"

#include "../../../server/baseapp/baseapp_interface.hpp"
#include "../../../server/loginapp/loginapp_interface.hpp"

namespace KBEngine{
ServerConfig g_serverConfig;
Componentbridge* g_pComponentbridge = NULL;

//-------------------------------------------------------------------------------------
Bots::Bots(Mercury::EventDispatcher& dispatcher, 
			 Mercury::NetworkInterface& ninterface, 
			 COMPONENT_TYPE componentType,
			 COMPONENT_ID componentID):
ClientApp(dispatcher, ninterface, componentType, componentID),
script_(),
scriptBaseTypes_(),
gameTimer_(),
pPyBots_(NULL),
clients_(),
reqCreateAndLoginTotalCount_(g_serverConfig.getBots().defaultAddBots_totalCount),
reqCreateAndLoginTickCount_(g_serverConfig.getBots().defaultAddBots_tickCount),
reqCreateAndLoginTickTime_(g_serverConfig.getBots().defaultAddBots_tickTime),
pCreateAndLoginHandler_(NULL),
pEventPoller_(Mercury::EventPoller::create())
{
	KBEngine::Mercury::MessageHandlers::pMainMessageHandlers = &BotsInterface::messageHandlers;
	g_pComponentbridge = new Componentbridge(ninterface, componentType, componentID);
}

//-------------------------------------------------------------------------------------
Bots::~Bots()
{
	SAFE_RELEASE(g_pComponentbridge);
	SAFE_RELEASE(pEventPoller_);
}

//-------------------------------------------------------------------------------------
bool Bots::initialize()
{
	// 广播自己的地址给网上上的所有kbemachine
	this->getMainDispatcher().addFrequentTask(&Componentbridge::getSingleton());

	gameTimer_ = this->getMainDispatcher().addTimer(1000000 / g_kbeSrvConfig.gameUpdateHertz(), this,
							reinterpret_cast<void *>(TIMEOUT_GAME_TICK));

	if(!installPyScript())
		return false;

	if(!installPyModules())
		return false;
	
	ProfileVal::setWarningPeriod(stampsPerSecond() / g_kbeSrvConfig.gameUpdateHertz());

	return installEntityDef();
}

//-------------------------------------------------------------------------------------
void Bots::finalise()
{
	gameTimer_.cancel();
	
	reqCreateAndLoginTotalCount_ = 0;
	SAFE_RELEASE(pCreateAndLoginHandler_);
	ClientApp::finalise();
}

//-------------------------------------------------------------------------------------
bool Bots::installEntityDef()
{
	if(!EntityDef::installScript(getScript().getModule()))
		return false;

	// 初始化数据类别
	// demo/res/scripts/entity_defs/alias.xml
	if(!DataTypes::initialize("scripts/entity_defs/alias.xml"))
		return false;

	// 初始化所有扩展模块
	// demo/res/scripts/
	if(!EntityDef::initialize(Resmgr::getSingleton().respaths()[1] + "res/scripts/", scriptBaseTypes_, BOTS_TYPE)){
		return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------
int Bots::registerPyObjectToScript(const char* attrName, PyObject* pyObj)
{ 
	return script_.registerToModule(attrName, pyObj); 
}

//-------------------------------------------------------------------------------------
int Bots::unregisterPyObjectToScript(const char* attrName)
{ 
	return script_.unregisterToModule(attrName); 
}

//-------------------------------------------------------------------------------------
bool Bots::installPyScript()
{
	if(Resmgr::getSingleton().respaths().size() <= 0)
	{
		ERROR_MSG("EntityApp::installPyScript: KBE_RES_PATH is error!\n");
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
	pyPaths += root_path + L"res/scripts/bots;";
	
	std::string kbe_res_path = Resmgr::getSingleton().respaths()[0].c_str();
	kbe_res_path += "scripts/common";

	tbuf = KBEngine::strutil::char2wchar(const_cast<char*>(kbe_res_path.c_str()));
	bool ret = getScript().install(tbuf, pyPaths, "KBEngine", componentType_);
	// 此处经测试传入python之后被python释放了
	// free(tbuf);
	return ret;
}

//-------------------------------------------------------------------------------------
void Bots::registerScript(PyTypeObject* pto)
{
	scriptBaseTypes_.push_back(pto);
}

//-------------------------------------------------------------------------------------
bool Bots::uninstallPyScript()
{
	return uninstallPyModules() && getScript().uninstall();
}

//-------------------------------------------------------------------------------------
bool Bots::installPyModules()
{
	EntityDef::installScript(getScript().getModule());
	Entity::installScript(getScript().getModule());
	Entities<Entity>::installScript(NULL);
	registerScript(Entity::getScriptType());
	
	ClientObject::installScript(NULL);
	PyBots::installScript(NULL);

	pPyBots_ = new PyBots();
	registerPyObjectToScript("bots", pPyBots_);
	
	onInstallPyModules();
	return true;
}

//-------------------------------------------------------------------------------------
bool Bots::uninstallPyModules()
{
	EntityDef::uninstallScript();

	Py_DECREF(pPyBots_);
	pPyBots_ = NULL;

	Entity::uninstallScript();
	Entities<Entity>::uninstallScript();
	ClientObject::uninstallScript();
	PyBots::uninstallScript();
	EntityDef::uninstallScript();
	return true;
}

//-------------------------------------------------------------------------------------
bool Bots::run(void)
{
	pCreateAndLoginHandler_ = new CreateAndLoginHandler();
	return ClientApp::run();
}

//-------------------------------------------------------------------------------------
void Bots::handleTimeout(TimerHandle handle, void * arg)
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

	ClientApp::handleTimeout(handle, arg);
}

//-------------------------------------------------------------------------------------
void Bots::handleGameTick()
{
	// time_t t = ::time(NULL);
	// DEBUG_MSG("EntityApp::handleGameTick[%"PRTime"]:%u\n", t, time_);

	g_kbetime++;
	threadPool_.onMainThreadTick();
	handleTimers();

	getNetworkInterface().handleChannels(KBEngine::Mercury::MessageHandlers::pMainMessageHandlers);
	pEventPoller_->processPendingEvents(0.0);

	CLIENTS::iterator iter = clients().begin();
	for(;iter != clients().end(); iter++)
		iter->second.get()->gameTick();
}

//-------------------------------------------------------------------------------------
Mercury::Channel* Bots::findChannelByMailbox(EntityMailbox& mailbox)
{
	int32 appID = (int32)mailbox.getComponentID();
	ClientObject* pClient = findClientByAppID(appID);
	if(pClient)
	{
		return pClient->pChannel();
	}

	return NULL;
}

//-------------------------------------------------------------------------------------
void Bots::lookApp(Mercury::Channel* pChannel)
{
	DEBUG_MSG(boost::format("Bots::lookApp: %1%\n") % pChannel->c_str());

	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
	
	(*pBundle) << g_componentType;
	(*pBundle) << componentID_;
	(*pBundle).send(getNetworkInterface(), pChannel);

	Mercury::Bundle::ObjPool().reclaimObject(pBundle);
}

//-------------------------------------------------------------------------------------
void Bots::reqCloseServer(Mercury::Channel* pChannel, MemoryStream& s)
{
	DEBUG_MSG(boost::format("Bots::reqCloseServer: %1%\n") % pChannel->c_str());

	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
	
	bool success = true;
	(*pBundle) << success;
	(*pBundle).send(getNetworkInterface(), pChannel);

	Mercury::Bundle::ObjPool().reclaimObject(pBundle);

	this->getMainDispatcher().breakProcessing();
}

//-------------------------------------------------------------------------------------
void Bots::onExecScriptCommand(Mercury::Channel* pChannel, KBEngine::MemoryStream& s)
{
	std::string cmd;
	s.readBlob(cmd);

	PyObject* pycmd = PyUnicode_DecodeUTF8(cmd.data(), cmd.size(), NULL);
	if(pycmd == NULL)
	{
		SCRIPT_ERROR_CHECK();
		return;
	}

	DEBUG_MSG(boost::format("EntityApp::onExecScriptCommand: size(%1%), command=%2%.\n") % 
		cmd.size() % cmd);

	std::string retbuf = "";
	PyObject* pycmd1 = PyUnicode_AsEncodedString(pycmd, "utf-8", NULL);

	if(script_.run_simpleString(PyBytes_AsString(pycmd1), &retbuf) == 0)
	{
		// 将结果返回给客户端
		Mercury::Bundle bundle;
		ConsoleInterface::ConsoleExecCommandCBMessageHandler msgHandler;
		bundle.newMessage(msgHandler);
		ConsoleInterface::ConsoleExecCommandCBMessageHandlerArgs1::staticAddToBundle(bundle, retbuf);
		bundle.send(this->getNetworkInterface(), pChannel);
	}

	Py_DECREF(pycmd);
	Py_DECREF(pycmd1);
}

//-------------------------------------------------------------------------------------
bool Bots::addClient(ClientObject* pClient)
{
	clients().insert(std::make_pair< Mercury::Channel*, ClientObjectPtr >(pClient->pChannel(), 
		ClientObjectPtr(pClient)));

	return true;
}

//-------------------------------------------------------------------------------------
bool Bots::delClient(ClientObject* pClient)
{
	clients().erase(pClient->pChannel());
	return true;
}

//-------------------------------------------------------------------------------------
ClientObject* Bots::findClient(Mercury::Channel * pChannel)
{
	CLIENTS::iterator iter = clients().find(pChannel);
	if(iter != clients().end())
	{
		return iter->second.get();
	}

	return NULL;
}

//-------------------------------------------------------------------------------------
ClientObject* Bots::findClientByAppID(int32 appID)
{
	CLIENTS::iterator iter = clients().begin();
	for(; iter != clients().end(); iter++)
	{
		if(iter->second.get()->appID() == appID)
			return iter->second.get();
	}

	return NULL;
}

//-------------------------------------------------------------------------------------
void Bots::onAppActiveTick(Mercury::Channel* pChannel, COMPONENT_TYPE componentType, COMPONENT_ID componentID)
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
			ERROR_MSG(boost::format("Bots::onAppActiveTick[%1%]: %2%:%3% not found.\n") % 
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
void Bots::onCreateAccountResult(Mercury::Channel * pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onCreateAccountResult(s);
	}
}

//-------------------------------------------------------------------------------------	
void Bots::onLoginSuccessfully(Mercury::Channel * pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onLoginSuccessfully(s);
	}
}

//-------------------------------------------------------------------------------------	
void Bots::onLoginFailed(Mercury::Channel * pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onLoginFailed(s);
	}
}

//-------------------------------------------------------------------------------------	
void Bots::onLoginGatewayFailed(Mercury::Channel * pChannel, SERVER_ERROR_CODE failedcode)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onLoginGatewayFailed(failedcode);
	}
}

//-------------------------------------------------------------------------------------	
void Bots::onCreatedProxies(Mercury::Channel * pChannel, 
								 uint64 rndUUID, ENTITY_ID eid, std::string& entityType)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onCreatedProxies(rndUUID, eid, entityType);
	}
}

//-------------------------------------------------------------------------------------	
void Bots::onEntityEnterWorld(Mercury::Channel * pChannel, ENTITY_ID eid, 
							  ENTITY_SCRIPT_UID scriptType, SPACE_ID spaceID)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onEntityEnterWorld(eid, scriptType, spaceID);
	}
}

//-------------------------------------------------------------------------------------	
void Bots::onEntityLeaveWorld(Mercury::Channel * pChannel, ENTITY_ID eid, SPACE_ID spaceID)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onEntityLeaveWorld(eid, spaceID);
	}
}

//-------------------------------------------------------------------------------------	
void Bots::onEntityEnterSpace(Mercury::Channel * pChannel, SPACE_ID spaceID, ENTITY_ID eid)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onEntityEnterSpace(eid, spaceID);
	}
}

//-------------------------------------------------------------------------------------	
void Bots::onEntityLeaveSpace(Mercury::Channel * pChannel, SPACE_ID spaceID, ENTITY_ID eid)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onEntityLeaveSpace(eid, spaceID);
	}
}

//-------------------------------------------------------------------------------------	
void Bots::onEntityDestroyed(Mercury::Channel * pChannel, ENTITY_ID eid)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onEntityDestroyed(eid);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onRemoteMethodCall(Mercury::Channel* pChannel, KBEngine::MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onRemoteMethodCall(s);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onUpdatePropertys(Mercury::Channel* pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onUpdatePropertys(s);
	}
}

//-------------------------------------------------------------------------------------

}
