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
//	pEntities_(NULL),
    gameTimer_()
{
	// KBEngine::Mercury::MessageHandlers::pMainMessageHandlers = &CellAppInterface::messageHandlers;	

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
	//Entities<Entity>::installScript(NULL);
	//Entity::installScript(getScript().getModule());

	//registerScript(Entity::getScriptType());
	
	//pEntities_ = new Entities<Entity>();
	//registerPyObjectToScript("entities", pEntities_);
	return EntityApp::installPyModules();
}

//-------------------------------------------------------------------------------------
bool Baseapp::uninstallPyModules()
{	
	//S_RELEASE(pEntities_);
	//unregisterPyObjectToScript("entities");
	//Entities<Entity>::uninstallScript();
	//Entity::uninstallScript();
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

}
