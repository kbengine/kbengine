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


#include "baseapp.hpp"
#include "base.hpp"
#include "proxy.hpp"
#include "baseapp_interface.hpp"
#include "network/common.hpp"
#include "network/tcp_packet.hpp"
#include "network/udp_packet.hpp"
#include "network/message_handler.hpp"
#include "entitydef/entity_mailbox.hpp"
#include "thread/threadpool.hpp"
#include "server/componentbridge.hpp"

namespace KBEngine{
	
ServerConfig g_serverConfig;
KBE_SINGLETON_INIT(Baseapp);

//-------------------------------------------------------------------------------------
Baseapp::Baseapp(Mercury::EventDispatcher& dispatcher, 
			 Mercury::NetworkInterface& ninterface, 
			 COMPONENT_TYPE componentType,
			 COMPONENT_ID componentID):
	EntityApp(dispatcher, ninterface, componentType, componentID),
    idClient_(),
	pBases_(NULL),
    gameTimer_(),
	pGlobalData_(NULL),
	pGlobalBases_(NULL)
{
	// KBEngine::Mercury::MessageHandlers::pMainMessageHandlers = &BaseappInterface::messageHandlers;	

	// 初始化mailbox模块获取entity实体函数地址
	// EntityMailbox::setGetEntityFunc(std::tr1::bind(&CellApp::tryGetEntityByMailbox, this, std::tr1::placeholders::_1, std::tr1::placeholders::_2));

	idClient_.pApp(this);
}

//-------------------------------------------------------------------------------------
Baseapp::~Baseapp()
{
}

//-------------------------------------------------------------------------------------
bool Baseapp::installPyModules()
{
	Entities<Base>::installScript(NULL);
	Base::installScript(getScript().getModule());
	Proxy::installScript(getScript().getModule());
	
	registerScript(Base::getScriptType());
	registerScript(Proxy::getScriptType());
	
	pBases_ = new Entities<Base>();
	registerPyObjectToScript("entities", pBases_);

	// 添加globalData, globalBases支持
	pGlobalData_ = new GlobalDataClient(BASEAPPMGR_TYPE, GlobalDataServer::GLOBAL_DATA);
	pGlobalBases_ = new GlobalDataClient(BASEAPPMGR_TYPE, GlobalDataServer::GLOBAL_BASES);
	registerPyObjectToScript("globalData", pGlobalData_);
	registerPyObjectToScript("globalBases", pGlobalBases_);

	return EntityApp::installPyModules();
}

//-------------------------------------------------------------------------------------
bool Baseapp::uninstallPyModules()
{	
	unregisterPyObjectToScript("globalData");
	unregisterPyObjectToScript("globalBases");
	S_RELEASE(pGlobalData_); 
	S_RELEASE(pGlobalBases_); 

	S_RELEASE(pBases_);
	unregisterPyObjectToScript("entities");
	Entities<Base>::uninstallScript();
	Base::uninstallScript();
	Proxy::uninstallScript();
	return EntityApp::uninstallPyModules();
}

//-------------------------------------------------------------------------------------
bool Baseapp::run()
{
	return ServerApp::run();
}

//-------------------------------------------------------------------------------------
void Baseapp::handleTimeout(TimerHandle handle, void * arg)
{
	switch (reinterpret_cast<uintptr>(arg))
	{
		case TIMEOUT_GAME_TICK:
			this->handleGameTick();
			break;
		default:
			break;
	}
}

//-------------------------------------------------------------------------------------
void Baseapp::handleTimers()
{
	timers().process(time_);
}

//-------------------------------------------------------------------------------------
void Baseapp::handleGameTick()
{
	// time_t t = ::time(NULL);
	// DEBUG_MSG("CellApp::handleGameTick[%"PRTime"]:%u\n", t, time_);
	
	time_++;
	handleTimers();
	getNetworkInterface().handleChannels(&BaseappInterface::messageHandlers);
}

//-------------------------------------------------------------------------------------
bool Baseapp::initializeBegin()
{
	if(thread::ThreadPool::getSingletonPtr() && 
		!thread::ThreadPool::getSingleton().isInitialize())
		thread::ThreadPool::getSingleton().createThreadPool(16, 16, 256);

	return true;
}

//-------------------------------------------------------------------------------------
bool Baseapp::initializeEnd()
{
	gameTimer_ = this->getMainDispatcher().addTimer(1000000 / g_kbeSrvConfig.gameUpdateHertz(), this,
							reinterpret_cast<void *>(TIMEOUT_GAME_TICK));
	
	return true;
}

//-------------------------------------------------------------------------------------
void Baseapp::finalise()
{
	gameTimer_.cancel();
	uninstallPyModules();
}

//-------------------------------------------------------------------------------------
bool Baseapp::destroyEntity(ENTITY_ID entityID)
{
	return true;
}

//-------------------------------------------------------------------------------------
void Baseapp::onReqAllocEntityID(Mercury::Channel* pChannel, ENTITY_ID startID, ENTITY_ID endID)
{
	idClient_.onAddRange(startID, endID);
}

//-------------------------------------------------------------------------------------
void Baseapp::onDbmgrInitCompleted(Mercury::Channel* pChannel, 
		ENTITY_ID startID, ENTITY_ID endID, int32 startGlobalOrder, int32 startGroupOrder)
{
	INFO_MSG("Baseapp::onDbmgrInitCompleted: entityID alloc(%d-%d), startGlobalOrder=%d, startGroupOrder=%d.\n",
		startID, endID, startGlobalOrder, startGroupOrder);

	startGlobalOrder_ = startGlobalOrder;
	startGroupOrder_ = startGroupOrder;

	idClient_.onAddRange(startID, endID);

	// 所有脚本都加载完毕
	PyObject* pyResult = PyObject_CallMethod(getEntryScript().get(), 
										const_cast<char*>("onBaseAppReady"), 
										const_cast<char*>("i"), 
										startGroupOrder == 1 ? 1 : 0);

	if(pyResult != NULL)
		Py_DECREF(pyResult);
	else
		SCRIPT_ERROR_CHECK();
}

//-------------------------------------------------------------------------------------
void Baseapp::onBroadcastGlobalDataChange(Mercury::Channel* pChannel, std::string& key, std::string& value, bool isDelete)
{
	if(isDelete)
		pGlobalData_->del(key);
	else
		pGlobalData_->write(key, value);
}

//-------------------------------------------------------------------------------------
void Baseapp::onBroadcastGlobalBasesChange(Mercury::Channel* pChannel, std::string& key, std::string& value, bool isDelete)
{
	if(isDelete)
		pGlobalBases_->del(key);
	else
		pGlobalBases_->write(key, value);
}

//-------------------------------------------------------------------------------------

}
