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


#include "cellapp.hpp"
#include "entity.hpp"
#include "cellapp_interface.hpp"
#include "network/tcp_packet.hpp"
#include "network/udp_packet.hpp"
#include "network/message_handler.hpp"
#include "entitydef/entity_mailbox.hpp"
#include "thread/threadpool.hpp"
#include "server/componentbridge.hpp"

namespace KBEngine{
	
ServerConfig g_serverConfig;
KBE_SINGLETON_INIT(CellApp);

//-------------------------------------------------------------------------------------
CellApp::CellApp(Mercury::EventDispatcher& dispatcher, 
			 Mercury::NetworkInterface& ninterface, 
			 COMPONENT_TYPE componentType,
			 COMPONENT_ID componentID):
	EntityApp(dispatcher, ninterface, componentType, componentID),
    idClient_(NULL),
	pEntities_(NULL),
    gameTimer_()
{
	// KBEngine::Mercury::MessageHandlers::pMainMessageHandlers = &CellAppInterface::messageHandlers;	

	// 初始化mailbox模块获取entity实体函数地址
	EntityMailbox::setGetEntityFunc(std::tr1::bind(&CellApp::tryGetEntityByMailbox, this, std::tr1::placeholders::_1, std::tr1::placeholders::_2));
}

//-------------------------------------------------------------------------------------
CellApp::~CellApp()
{
}

//-------------------------------------------------------------------------------------
bool CellApp::installPyModules()
{
	Entities<Entity>::installScript(NULL);
	Entity::installScript(getScript().getModule());

	registerScript(Entity::getScriptType());
	
	pEntities_ = new Entities<Entity>();
	registerPyObjectToScript("entities", pEntities_);
	return EntityApp::installPyModules();
}

//-------------------------------------------------------------------------------------
bool CellApp::uninstallPyModules()
{	
	S_RELEASE(pEntities_);
	unregisterPyObjectToScript("entities");
	Entities<Entity>::uninstallScript();
	Entity::uninstallScript();
	return EntityApp::uninstallPyModules();
}

//-------------------------------------------------------------------------------------
bool CellApp::run()
{
	idClient_->onAddRange(1, 500);
	Entity* e = createEntity("Avatar", NULL);
	registerPyObjectToScript("avatar", e);
	PyRun_SimpleString("print (dir(KBEngine.avatar), KBEngine.entities.has_key(1))");
	PyRun_SimpleString("print ('888888888888888888888', KBEngine.avatar.id, KBEngine.avatar.name)");
	DEBUG_MSG("kbe:python is init successfully!!! %d\n", 88);
	SmartPointer<PyObject> testsmartpointer(::PyBytes_FromString("test"));
	testsmartpointer.clear();

	CRITICAL_MSG("hahahah %d\n", 1111);
	unregisterPyObjectToScript("avatar");
	destroyEntity(e->getID());
	return ServerApp::run();
}

//-------------------------------------------------------------------------------------
void CellApp::handleTimeout(TimerHandle handle, void * arg)
{
	switch (reinterpret_cast<uintptr>(arg))
	{
		case TIMEOUT_GAME_TICK:
			this->handleGameTick();
			break;
		case TIMEOUT_LOADING_TICK:
			break;
		default:
			break;
	}
}

//-------------------------------------------------------------------------------------
void CellApp::handleTimers()
{
	timers().process(time_);
}

//-------------------------------------------------------------------------------------
void CellApp::handleGameTick()
{
	// time_t t = ::time(NULL);
	// DEBUG_MSG("CellApp::handleGameTick[%"PRTime"]:%u\n", t, time_);
	
	time_++;
	handleTimers();
	getNetworkInterface().handleChannels(&CellappInterface::messageHandlers);
}

//-------------------------------------------------------------------------------------
bool CellApp::initializeBegin()
{
	if(thread::ThreadPool::getSingletonPtr() && 
		!thread::ThreadPool::getSingleton().isInitialize())
		thread::ThreadPool::getSingleton().createThreadPool(16, 16, 256);

	return true;
}

//-------------------------------------------------------------------------------------
bool CellApp::initializeEnd()
{
	idClient_ = new IDClient<ENTITY_ID>;
	
	gameTimer_ = this->getMainDispatcher().addTimer(1000000 / g_kbeSrvConfig.gameUpdateHertz(), this,
							reinterpret_cast<void *>(TIMEOUT_GAME_TICK));
	
	return true;
}

//-------------------------------------------------------------------------------------
void CellApp::finalise()
{
	SAFE_RELEASE(idClient_);
	gameTimer_.cancel();
	uninstallPyModules();
}

//-------------------------------------------------------------------------------------
Entity* CellApp::createEntity(const char* entityType, PyObject* params, bool isInitializeScript, ENTITY_ID eid)
{
	// 检查ID是否足够, 不足返回NULL
	if(eid <= 0 && idClient_->getSize() == 0)
	{
		PyErr_SetString(PyExc_SystemError, "App::createEntity: is Failed. not enough entityIDs.");
		PyErr_PrintEx(0);
		return NULL;
	}
	
	ScriptModule* sm = EntityDef::findScriptModule(entityType);
	if(sm == NULL || !sm->hasCell())
	{
		PyErr_Format(PyExc_TypeError, "App::createEntity: entityType [%s] not found.\n", entityType);
		PyErr_PrintEx(0);
		return NULL;
	}

	PyObject* obj = sm->createObject();

	// 判断是否要分配一个新的id
	ENTITY_ID id = eid;
	if(id <= 0)
		id = idClient_->alloc();

	// 执行Entity的构造函数
	Entity* entity = new(obj) Entity(id, sm);

	// 创建名字空间
	entity->createNamespace(params);

	// 将entity加入entities
	pEntities_->add(id, entity); 
	
	// 检查ID的足够性，不足则申请
	//checkEntityIDEnough();

	// 初始化脚本
	if(isInitializeScript)
		entity->initializeScript();

	SCRIPT_ERROR_CHECK();
	INFO_MSG("App::createEntity: new %s (%ld).\n", entityType, id);
	return entity;
}

//-------------------------------------------------------------------------------------
Entity* CellApp::findEntity(ENTITY_ID eid)
{
	return pEntities_->find(eid);
}

//-------------------------------------------------------------------------------------
PyObject* CellApp::tryGetEntityByMailbox(COMPONENT_ID componentID, ENTITY_ID eid)
{
	if(componentID != componentID_)
		return NULL;
	
	Entity* entity = pEntities_->find(eid);
	if(entity == NULL){
		ERROR_MSG("CellApp::tryGetEntityByMailbox: can't found entity:%ld.\n", eid);
		return NULL;
	}

	return entity;
}

//-------------------------------------------------------------------------------------
bool CellApp::destroyEntity(ENTITY_ID entityID)
{
	Entity* entity = pEntities_->erase(entityID);
	if(entity != NULL)
	{
		entity->destroy();
		return true;
	}

	return false;
}

//-------------------------------------------------------------------------------------

}
