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
#include "space.hpp"
#include "cellapp_interface.hpp"
#include "forward_message_over_handler.hpp"
#include "network/tcp_packet.hpp"
#include "network/udp_packet.hpp"
#include "server/componentbridge.hpp"
#include "server/components.hpp"
#include "dbmgr/dbmgr_interface.hpp"

#include "../../server/baseappmgr/baseappmgr_interface.hpp"
#include "../../server/cellappmgr/cellappmgr_interface.hpp"
#include "../../server/baseapp/baseapp_interface.hpp"
#include "../../server/cellapp/cellapp_interface.hpp"
#include "../../server/dbmgr/dbmgr_interface.hpp"
#include "../../server/loginapp/loginapp_interface.hpp"

namespace KBEngine{
	
ServerConfig g_serverConfig;
KBE_SINGLETON_INIT(Cellapp);

//-------------------------------------------------------------------------------------
Cellapp::Cellapp(Mercury::EventDispatcher& dispatcher, 
			 Mercury::NetworkInterface& ninterface, 
			 COMPONENT_TYPE componentType,
			 COMPONENT_ID componentID):
	EntityApp<Entity>(dispatcher, ninterface, componentType, componentID),
	pCellAppData_(NULL),
	forward_messagebuffer_(ninterface)
{
	KBEngine::Mercury::MessageHandlers::pMainMessageHandlers = &CellappInterface::messageHandlers;
}

//-------------------------------------------------------------------------------------
Cellapp::~Cellapp()
{
}

//-------------------------------------------------------------------------------------
bool Cellapp::installPyModules()
{
	Entity::installScript(getScript().getModule());
	registerScript(Entity::getScriptType());

	// 注册创建entity的方法到py
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(),		createEntity,			__py_createEntity,					METH_VARARGS,			0);

	return EntityApp<Entity>::installPyModules();
}

//-------------------------------------------------------------------------------------
void Cellapp::onInstallPyModules()
{
	// 添加globalData, cellAppData支持
	pCellAppData_ = new GlobalDataClient(DBMGR_TYPE, GlobalDataServer::CELLAPP_DATA);
	registerPyObjectToScript("cellAppData", pCellAppData_);
}

//-------------------------------------------------------------------------------------
bool Cellapp::uninstallPyModules()
{	
	unregisterPyObjectToScript("cellAppData");
	S_RELEASE(pCellAppData_); 

	Entity::uninstallScript();
	return EntityApp<Entity>::uninstallPyModules();
}

//-------------------------------------------------------------------------------------
bool Cellapp::run()
{
	return EntityApp<Entity>::run();
}

//-------------------------------------------------------------------------------------
void Cellapp::handleTimeout(TimerHandle handle, void * arg)
{
	switch (reinterpret_cast<uintptr>(arg))
	{
		case TIMEOUT_LOADING_TICK:
			break;
		default:
			break;
	}

	EntityApp<Entity>::handleTimeout(handle, arg);
}

//-------------------------------------------------------------------------------------
void Cellapp::handleGameTick()
{
	EntityApp<Entity>::handleGameTick();
}

//-------------------------------------------------------------------------------------
bool Cellapp::initializeBegin()
{
	return true;
}

//-------------------------------------------------------------------------------------
bool Cellapp::initializeEnd()
{
	return true;
}

//-------------------------------------------------------------------------------------
void Cellapp::finalise()
{
	EntityApp<Entity>::finalise();
}

//-------------------------------------------------------------------------------------
void Cellapp::onGetEntityAppFromDbmgr(Mercury::Channel* pChannel, int32 uid, std::string& username, 
						int8 componentType, uint64 componentID, 
						uint32 intaddr, uint16 intport, uint32 extaddr, uint16 extport)
{
	EntityApp<Entity>::onRegisterNewApp(pChannel, uid, username, componentType, componentID, 
									intaddr, intport, extaddr, extport);

	KBEngine::COMPONENT_TYPE tcomponentType = (KBEngine::COMPONENT_TYPE)componentType;

	Components::COMPONENTS cts = Componentbridge::getComponents().getComponents(DBMGR_TYPE);
	KBE_ASSERT(cts.size() >= 1);
	
	Components::ComponentInfos* cinfos = Componentbridge::getComponents().findComponent(tcomponentType, uid, componentID);
	cinfos->pChannel = NULL;

	int ret = Components::getSingleton().connectComponent(tcomponentType, uid, componentID);
	KBE_ASSERT(ret != -1);

	Mercury::Bundle bundle;

	switch(tcomponentType)
	{
	case BASEAPP_TYPE:
		bundle.newMessage(BaseappInterface::onRegisterNewApp);
		BaseappInterface::onRegisterNewAppArgs8::staticAddToBundle(bundle, getUserUID(), getUsername(), CELLAPP_TYPE, componentID_, 
			this->getNetworkInterface().intaddr().ip, this->getNetworkInterface().intaddr().port, 
			this->getNetworkInterface().extaddr().ip, this->getNetworkInterface().extaddr().port);
		break;
	case CELLAPP_TYPE:
		bundle.newMessage(CellappInterface::onRegisterNewApp);
		CellappInterface::onRegisterNewAppArgs8::staticAddToBundle(bundle, getUserUID(), getUsername(), CELLAPP_TYPE, componentID_, 
			this->getNetworkInterface().intaddr().ip, this->getNetworkInterface().intaddr().port, 
			this->getNetworkInterface().extaddr().ip, this->getNetworkInterface().extaddr().port);
		break;
	default:
		KBE_ASSERT(false && "no support!\n");
		break;
	};
	
	bundle.send(this->getNetworkInterface(), cinfos->pChannel);
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
	
	if(entityType == NULL || strlen(entityType) == 0)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::createEntity: entityType is NULL.");
		PyErr_PrintEx(0);
		S_Return;
	}

	Space* space = Spaces::findSpace(spaceID);
	if(space == NULL)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::createEntity: spaceID %ld not found.", spaceID);
		PyErr_PrintEx(0);
		S_Return;
	}
	
	// 创建entity
	Entity* pEntity = Cellapp::getSingleton().createEntityCommon(entityType, params, false, 0);

	if(pEntity != NULL)
	{
		Py_INCREF(pEntity);
		pEntity->pySetPosition(position);
		pEntity->pySetDirection(direction);	
		pEntity->initializeScript();

		// 添加到space
		space->addEntity(pEntity);
	}
	
	return pEntity;
}

//-------------------------------------------------------------------------------------
void Cellapp::onDbmgrInitCompleted(Mercury::Channel* pChannel, 
		ENTITY_ID startID, ENTITY_ID endID, int32 startGlobalOrder, int32 startGroupOrder)
{
	EntityApp<Entity>::onDbmgrInitCompleted(pChannel, startID, endID, startGlobalOrder, startGroupOrder);
	
	// 所有脚本都加载完毕
	PyObject* pyResult = PyObject_CallMethod(getEntryScript().get(), 
										const_cast<char*>("onInit"), 
										const_cast<char*>("i"), 
										0);

	if(pyResult != NULL)
		Py_DECREF(pyResult);
	else
		SCRIPT_ERROR_CHECK();
}

//-------------------------------------------------------------------------------------
void Cellapp::onBroadcastCellAppDataChange(Mercury::Channel* pChannel, KBEngine::MemoryStream& s)
{
	int32 slen;
	std::string key, value;
	bool isDelete;
	
	s >> isDelete;
	s >> slen;
	key.assign((char*)(s.data() + s.rpos()), slen);
	s.read_skip(slen);

	if(!isDelete)
	{
		s >> slen;
		value.assign((char*)(s.data() + s.rpos()), slen);
		s.read_skip(slen);
	}

	PyObject * pyKey = script::Pickler::unpickle(key);

	if(isDelete)
	{
		if(pCellAppData_->del(pyKey))
		{
			// 通知脚本
			PyObject* pyResult = PyObject_CallMethod(getEntryScript().get(), 
												const_cast<char*>("onCellAppDataDel"), 
												const_cast<char*>("O"), 
												pyKey);

			if(pyResult != NULL)
				Py_DECREF(pyResult);
			else
				SCRIPT_ERROR_CHECK();
		}
	}
	else
	{
		PyObject * pyValue = script::Pickler::unpickle(value);
		if(pCellAppData_->write(pyKey, pyValue))
		{
			// 通知脚本
			PyObject* pyResult = PyObject_CallMethod(getEntryScript().get(), 
												const_cast<char*>("onCellAppData"), 
												const_cast<char*>("OO"), 
												pyKey, pyValue);

			if(pyResult != NULL)
				Py_DECREF(pyResult);
			else
				SCRIPT_ERROR_CHECK();
		}
	}

}

//-------------------------------------------------------------------------------------
void Cellapp::onCreateInNewSpaceFromBaseapp(Mercury::Channel* pChannel, KBEngine::MemoryStream& s)
{
	std::string entityType;
	ENTITY_ID mailboxEntityID;
	uint32 cellDataLength;
	std::string strEntityCellData;
	COMPONENT_ID componentID;
	SPACE_ID spaceID = 1;

	s >> entityType;
	s >> mailboxEntityID;
	s >> spaceID;
	s >> componentID;
	s >> cellDataLength;

	if(cellDataLength > 0)
	{
		strEntityCellData.assign((char*)(s.data() + s.rpos()), cellDataLength);
		s.read_skip(cellDataLength);
	}

	// DEBUG_MSG("Cellapp::onCreateInNewSpaceFromBaseapp: spaceID=%u, entityType=%s, entityID=%d, componentID=%"PRAppID".\n", 
	//	spaceID, entityType.c_str(), mailboxEntityID, componentID);

	Space* space = Spaces::createNewSpace(spaceID);
	if(space != NULL)
	{
		// 解包cellData信息.
		PyObject* params = NULL;
		if(strEntityCellData.size() > 0)
			params = script::Pickler::unpickle(strEntityCellData);
	
		// 创建entity
		Entity* e = createEntityCommon(entityType.c_str(), params, false, mailboxEntityID);
		Py_XDECREF(params);
		
		if(e == NULL)
			return;

		// 设置entity的baseMailbox
		EntityMailbox* mailbox = new EntityMailbox(e->getScriptModule(), NULL, componentID, mailboxEntityID, MAILBOX_TYPE_BASE);
		e->setBaseMailbox(mailbox);
		
		// 此处baseapp可能还有没初始化过来， 所以有一定概率是为None的
		Components::ComponentInfos* cinfos = Components::getSingleton().findComponent(BASEAPP_TYPE, componentID);
		if(cinfos == NULL || cinfos->pChannel == NULL)
		{
			ForwardItem* pFI = new ForwardItem();
			pFI->pHandler = new FMH_Baseapp_onEntityGetCellFrom_onCreateInNewSpaceFromBaseapp(e, spaceID);
			pFI->bundle.newMessage(BaseappInterface::onEntityGetCell);
			BaseappInterface::onEntityGetCellArgs2::staticAddToBundle(pFI->bundle, mailboxEntityID, componentID_);
			forward_messagebuffer_.push(componentID, pFI);
			WARNING_MSG("Cellapp::onCreateInNewSpaceFromBaseapp: not found baseapp, message is buffered.\n");
			return;
		}

		// 添加到space
		space->addEntity(e);
		e->initializeScript();

		Mercury::Bundle bundle;
		bundle.newMessage(BaseappInterface::onEntityGetCell);
		BaseappInterface::onEntityGetCellArgs2::staticAddToBundle(bundle, mailboxEntityID, componentID_);
		bundle.send(this->getNetworkInterface(), cinfos->pChannel);
		return;
	}
	
	ERROR_MSG("Cellapp::onCreateInNewSpaceFromBaseapp: not found baseapp[%"PRAppID"], entityID=%d, spaceID=%u.\n", 
		componentID, mailboxEntityID, spaceID);
}

//-------------------------------------------------------------------------------------
void Cellapp::onCreateCellEntityFromBaseapp(Mercury::Channel* pChannel, KBEngine::MemoryStream& s)
{
	std::string entityType;
	ENTITY_ID createToEntityID, entityID;
	uint32 cellDataLength;
	std::string strEntityCellData;
	COMPONENT_ID componentID;
	SPACE_ID spaceID = 1;
	bool hasClient;

	s >> createToEntityID;
	s >> entityType;
	s >> entityID;
	s >> componentID;
	s >> hasClient;
	s >> cellDataLength;

	if(cellDataLength > 0)
	{
		strEntityCellData.assign((char*)(s.data() + s.rpos()), cellDataLength);
		s.read_skip(cellDataLength);
	}

		// 此处baseapp可能还有没初始化过来， 所以有一定概率是为None的
		Components::ComponentInfos* cinfos = Components::getSingleton().findComponent(BASEAPP_TYPE, componentID);
		if(cinfos == NULL || cinfos->pChannel == NULL)
		{
			ForwardItem* pFI = new ForwardItem();
			pFI->pHandler = new FMH_Baseapp_onEntityGetCellFrom_onCreateCellEntityFromBaseapp(entityType, createToEntityID, 
				entityID, cellDataLength, strEntityCellData, hasClient, componentID, spaceID);
			pFI->bundle.newMessage(BaseappInterface::onEntityGetCell);
			BaseappInterface::onEntityGetCellArgs2::staticAddToBundle(pFI->bundle, entityID, componentID_);
			forward_messagebuffer_.push(componentID, pFI);
			WARNING_MSG("Cellapp::onCreateCellEntityFromBaseapp: not found baseapp, message is buffered.\n");
			return;
		}

	_onCreateCellEntityFromBaseapp(entityType, createToEntityID, entityID, cellDataLength, strEntityCellData, hasClient, componentID, spaceID);

	Mercury::Bundle bundle;
	bundle.newMessage(BaseappInterface::onEntityGetCell);
	BaseappInterface::onEntityGetCellArgs2::staticAddToBundle(bundle, entityID, componentID_);
	bundle.send(this->getNetworkInterface(), cinfos->pChannel);
}

//-------------------------------------------------------------------------------------
void Cellapp::_onCreateCellEntityFromBaseapp(std::string& entityType, ENTITY_ID createToEntityID, ENTITY_ID entityID, uint32 cellDataLength, 
		std::string& strEntityCellData, bool hasClient, COMPONENT_ID componentID, SPACE_ID spaceID)
{
	Entity* pCreateToEntity = pEntities_->find(createToEntityID);
	spaceID = pCreateToEntity->getSpaceID();

	//DEBUG_MSG("Cellapp::onCreateCellEntityFromBaseapp: spaceID=%u, entityType=%s, entityID=%d, componentID=%"PRAppID".\n", 
	//	spaceID, entityType.c_str(), entityID, componentID);

	Space* space = Spaces::findSpace(spaceID);
	if(space != NULL)
	{
		// 解包cellData信息.
		PyObject* params = NULL;
		if(strEntityCellData.size() > 0)
			params = script::Pickler::unpickle(strEntityCellData);
	
		// 创建entity
		Entity* e = createEntityCommon(entityType.c_str(), params, false, entityID);
		Py_XDECREF(params);
		
		if(e == NULL)
			return;
		
		// 注意：此处理论不会找不到组件， 因为onCreateCellEntityFromBaseapp中已经进行过一次消息缓存判断
		Components::ComponentInfos* cinfos = Components::getSingleton().findComponent(BASEAPP_TYPE, componentID);
		KBE_ASSERT(cinfos != NULL && cinfos->pChannel != NULL);

		// 设置entity的baseMailbox
		EntityMailbox* mailbox = new EntityMailbox(e->getScriptModule(), NULL, componentID, entityID, MAILBOX_TYPE_BASE);
		e->setBaseMailbox(mailbox);

		// 添加到space
		space->addEntity(e);
		e->initializeScript();
		
		// 如果是有client的entity则需要告知client， cell部分创建了
		if(hasClient)
		{
			PyObject* clientMailbox = PyObject_GetAttrString(mailbox, "client");
			KBE_ASSERT(clientMailbox != Py_None);

			EntityMailbox* client = static_cast<EntityMailbox*>(clientMailbox);	
			// Py_INCREF(clientMailbox); 这里不需要增加引用， 因为每次都会产生一个新的对象
			e->setClientMailbox(client);
			
			// 初始化默认AOI范围
			ENGINE_COMPONENT_INFO& ecinfo = ServerConfig::getSingleton().getCellApp();
			e->setAoiRadius(ecinfo.defaultAoIRadius, ecinfo.defaultAoIHysteresisArea);
		}

		return;
	}

	KBE_ASSERT(false && "Cellapp::onCreateCellEntityFromBaseapp: is error!\n");
}

//-------------------------------------------------------------------------------------
void Cellapp::onDestroyCellEntityFromBaseapp(Mercury::Channel* pChannel, ENTITY_ID eid)
{
	// DEBUG_MSG("Cellapp::onDestroyCellEntityFromBaseapp:entityID=%d.\n", eid);
	destroyEntity(eid);
}

//-------------------------------------------------------------------------------------
void Cellapp::onEntityMail(Mercury::Channel* pChannel, KBEngine::MemoryStream& s)
{
}

//-------------------------------------------------------------------------------------

}
