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

#include "dbmgr/dbmgr_interface.hpp"

namespace KBEngine{
	
ServerConfig g_serverConfig;
KBE_SINGLETON_INIT(Cellapp);

//-------------------------------------------------------------------------------------
Cellapp::Cellapp(Mercury::EventDispatcher& dispatcher, 
			 Mercury::NetworkInterface& ninterface, 
			 COMPONENT_TYPE componentType,
			 COMPONENT_ID componentID):
	EntityApp(dispatcher, ninterface, componentType, componentID),
	idClient_(),
	pEntities_(NULL),
    gameTimer_(),
	pGlobalData_(NULL),
	pCellAppData_(NULL)
{
	// KBEngine::Mercury::MessageHandlers::pMainMessageHandlers = &CellAppInterface::messageHandlers;	

	// 初始化mailbox模块获取entity实体函数地址
	EntityMailbox::setGetEntityFunc(std::tr1::bind(&Cellapp::tryGetEntityByMailbox, this, std::tr1::placeholders::_1, std::tr1::placeholders::_2));

	idClient_.pApp(this);
}

//-------------------------------------------------------------------------------------
Cellapp::~Cellapp()
{
}

//-------------------------------------------------------------------------------------
bool Cellapp::installPyModules()
{
	Entities<Entity>::installScript(NULL);
	Entity::installScript(getScript().getModule());

	registerScript(Entity::getScriptType());
	
	pEntities_ = new Entities<Entity>();
	registerPyObjectToScript("entities", pEntities_);

	// 添加globalData, cellAppData支持
	pGlobalData_	= new GlobalDataClient(BASEAPPMGR_TYPE);
	pCellAppData_	= new GlobalDataClient(CELLAPPMGR_TYPE);
	registerPyObjectToScript("globalData", pGlobalData_);
	registerPyObjectToScript("cellAppData", pCellAppData_);


	// 注册创建entity的方法到py
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(),		createEntity,			__py_createEntity,					METH_VARARGS,			0);

	return EntityApp::installPyModules();
}

//-------------------------------------------------------------------------------------
bool Cellapp::uninstallPyModules()
{	
	unregisterPyObjectToScript("globalData");
	unregisterPyObjectToScript("cellAppData");
	S_RELEASE(pGlobalData_); 
	S_RELEASE(pCellAppData_); 

	S_RELEASE(pEntities_);
	unregisterPyObjectToScript("entities");
	Entities<Entity>::uninstallScript();
	Entity::uninstallScript();
	return EntityApp::uninstallPyModules();
}

//-------------------------------------------------------------------------------------
bool Cellapp::run()
{
	return ServerApp::run();
}

//-------------------------------------------------------------------------------------
void Cellapp::handleTimeout(TimerHandle handle, void * arg)
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
void Cellapp::handleTimers()
{
	timers().process(time_);
}

//-------------------------------------------------------------------------------------
void Cellapp::handleGameTick()
{
	// time_t t = ::time(NULL);
	// DEBUG_MSG("Cellapp::handleGameTick[%"PRTime"]:%u\n", t, time_);
	
	time_++;
	handleTimers();
	getNetworkInterface().handleChannels(&CellappInterface::messageHandlers);
}

//-------------------------------------------------------------------------------------
bool Cellapp::initializeBegin()
{
	if(thread::ThreadPool::getSingletonPtr() && 
		!thread::ThreadPool::getSingleton().isInitialize())
		thread::ThreadPool::getSingleton().createThreadPool(16, 16, 256);

	return true;
}

//-------------------------------------------------------------------------------------
bool Cellapp::initializeEnd()
{
	gameTimer_ = this->getMainDispatcher().addTimer(1000000 / g_kbeSrvConfig.gameUpdateHertz(), this,
							reinterpret_cast<void *>(TIMEOUT_GAME_TICK));
	
	return true;
}

//-------------------------------------------------------------------------------------
void Cellapp::finalise()
{
	gameTimer_.cancel();
	uninstallPyModules();
}

//-------------------------------------------------------------------------------------
PyObject* Cellapp::__py_createEntity(PyObject* self, PyObject* args)
{
	PyObject* params = NULL;
	char* entityType = NULL;
	SPACE_ID spaceID;
	PyObject* position, *direction;
	
	if(!PyArg_ParseTuple(args, "s|I|O|O|O", &entityType, &spaceID, &position, &direction, &params))
	{
		PyErr_Format(PyExc_TypeError, 
			"KBEngine::createEntity: args is error! args[scriptName, spaceID, position, direction, states].");
		PyErr_PrintEx(0);
		return NULL;
	}

	
	//Space* space = SpaceManager::findSpace(spaceID);
	//if(space == NULL)
	//{
	//	PyErr_Format(PyExc_TypeError, "KBEngine::createEntity: spaceID %ld not found.", spaceID);
	//	PyErr_PrintEx(0);
	//	S_Return;
	//}
	
	// 创建entity
	Entity* pEntity = Cellapp::getSingleton().createEntity(entityType, params, false, 0);

	if(pEntity != NULL)
	{
		Py_INCREF(pEntity);
		pEntity->pySetPosition(position);
		pEntity->pySetDirection(direction);	
		pEntity->initializeScript();

		// 添加到space
		//space->addEntity(pEntity);
	}
	
	return pEntity;
}

Entity* Cellapp::createEntity(const char* entityType, PyObject* params, bool isInitializeScript, ENTITY_ID eid)
{
	// 检查ID是否足够, 不足返回NULL
	if(eid <= 0 && idClient_.getSize() == 0)
	{
		PyErr_SetString(PyExc_SystemError, "Cellapp::createEntity: is Failed. not enough entityIDs.");
		PyErr_PrintEx(0);
		return NULL;
	}
	
	ScriptModule* sm = EntityDef::findScriptModule(entityType);
	if(sm == NULL || !sm->hasCell())
	{
		PyErr_Format(PyExc_TypeError, "Cellapp::createEntity: entity [%s] not found.\n", entityType);
		PyErr_PrintEx(0);
		return NULL;
	}

	PyObject* obj = sm->createObject();

	// 判断是否要分配一个新的id
	ENTITY_ID id = eid;
	if(id <= 0)
		id = idClient_.alloc();

	// 执行Entity的构造函数
	Entity* entity = new(obj) Entity(id, sm);

	// 创建名字空间
	entity->createNamespace(params);

	// 将entity加入entities
	pEntities_->add(id, entity); 

	// 初始化脚本
	if(isInitializeScript)
		entity->initializeScript();

	SCRIPT_ERROR_CHECK();
	INFO_MSG("Cellapp::createEntity: new %s (%ld).\n", entityType, id);
	return entity;
}

//-------------------------------------------------------------------------------------
Entity* Cellapp::findEntity(ENTITY_ID eid)
{
	return pEntities_->find(eid);
}

//-------------------------------------------------------------------------------------
PyObject* Cellapp::tryGetEntityByMailbox(COMPONENT_ID componentID, ENTITY_ID eid)
{
	if(componentID != componentID_)
		return NULL;
	
	Entity* entity = pEntities_->find(eid);
	if(entity == NULL){
		ERROR_MSG("Cellapp::tryGetEntityByMailbox: can't found entity:%ld.\n", eid);
		return NULL;
	}

	return entity;
}

//-------------------------------------------------------------------------------------
bool Cellapp::destroyEntity(ENTITY_ID entityID)
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
void Cellapp::onReqAllocEntityID(Mercury::Channel* pChannel, ENTITY_ID startID, ENTITY_ID endID)
{
	idClient_.onAddRange(startID, endID);
}

//-------------------------------------------------------------------------------------
void Cellapp::onDbmgrInit(Mercury::Channel* pChannel, 
		ENTITY_ID startID, ENTITY_ID endID, int32 startGlobalOrder, int32 startGroupOrder)
{
	INFO_MSG("Cellapp::onDbmgrInit: entityID alloc(%d-%d), startGlobalOrder=%d, startGroupOrder=%d.\n",
		startID, endID, startGlobalOrder, startGroupOrder);

	startGlobalOrder_ = startGlobalOrder;
	startGroupOrder_ = startGroupOrder;

	idClient_.onAddRange(startID, endID);
	
	// 所有脚本都加载完毕
	PyObject* pyResult = PyObject_CallMethod(getEntryScript().get(), 
										const_cast<char*>("onInitialize"), 
										const_cast<char*>("i"), 
										0);

	if(pyResult != NULL)
		Py_DECREF(pyResult);
	else
		SCRIPT_ERROR_CHECK();
}

//-------------------------------------------------------------------------------------
void Cellapp::onBroadcastGlobalDataChange(Mercury::Channel* pChannel, std::string& key, std::string& value, bool isDelete)
{
	if(isDelete)
		pGlobalData_->del(key);
	else
		pGlobalData_->write(key, value);
}

//-------------------------------------------------------------------------------------
void Cellapp::onBroadcastCellAppDataChange(Mercury::Channel* pChannel, std::string& key, std::string& value, bool isDelete)
{
	if(isDelete)
		pCellAppData_->del(key);
	else
		pCellAppData_->write(key, value);
}

//-------------------------------------------------------------------------------------

}
