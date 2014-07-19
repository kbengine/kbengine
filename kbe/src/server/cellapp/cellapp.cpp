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
#include "profile.hpp"
#include "witness.hpp"
#include "coordinate_node.hpp"
#include "aoi_trigger.hpp"
#include "cellapp_interface.hpp"
#include "entity_remotemethod.hpp"
#include "forward_message_over_handler.hpp"
#include "network/tcp_packet.hpp"
#include "network/udp_packet.hpp"
#include "server/componentbridge.hpp"
#include "server/components.hpp"
#include "server/telnet_server.hpp"
#include "dbmgr/dbmgr_interface.hpp"
#include "navigation/navigation.hpp"

#include "../../server/baseappmgr/baseappmgr_interface.hpp"
#include "../../server/cellappmgr/cellappmgr_interface.hpp"
#include "../../server/baseapp/baseapp_interface.hpp"
#include "../../server/cellapp/cellapp_interface.hpp"
#include "../../server/dbmgr/dbmgr_interface.hpp"
#include "../../server/loginapp/loginapp_interface.hpp"

namespace KBEngine{
	
ServerConfig g_serverConfig;
KBE_SINGLETON_INIT(Cellapp);

Navigation g_navigation;

//-------------------------------------------------------------------------------------
Cellapp::Cellapp(Mercury::EventDispatcher& dispatcher, 
			 Mercury::NetworkInterface& ninterface, 
			 COMPONENT_TYPE componentType,
			 COMPONENT_ID componentID):
	EntityApp<Entity>(dispatcher, ninterface, componentType, componentID),
	pCellAppData_(NULL),
	forward_messagebuffer_(ninterface),
	cells_(),
	pTelnetServer_(NULL),
	pWitnessedTimeoutHandler_(NULL)
{
	KBEngine::Mercury::MessageHandlers::pMainMessageHandlers = &CellappInterface::messageHandlers;

	// hook mailboxcall
	static EntityMailbox::MailboxCallHookFunc mailboxCallHookFunc = std::tr1::bind(&Cellapp::createMailboxCallEntityRemoteMethod, this, 
		std::tr1::placeholders::_1, std::tr1::placeholders::_2);

	EntityMailbox::setMailboxCallHookFunc(&mailboxCallHookFunc);
}

//-------------------------------------------------------------------------------------
Cellapp::~Cellapp()
{
}

//-------------------------------------------------------------------------------------	
bool Cellapp::canShutdown()
{
	Entities<Entity>::ENTITYS_MAP& entities =  this->pEntities()->getEntities();
	Entities<Entity>::ENTITYS_MAP::iterator iter = entities.begin();
	for(; iter != entities.end(); iter++)
	{
		if(static_cast<Entity*>(iter->second.get())->getBaseMailbox() != NULL)
		{
			lastShutdownFailReason_ = "destroyHasBaseEntitys";
			return false;
		}
	}

	return true;
}

//-------------------------------------------------------------------------------------	
void Cellapp::onShutdown(bool first)
{
	EntityApp<Entity>::onShutdown(first);

	int count = g_serverConfig.getCellApp().perSecsDestroyEntitySize;
	Entities<Entity>::ENTITYS_MAP& entities =  this->pEntities()->getEntities();

	while(count > 0)
	{
		bool done = false;
		Entities<Entity>::ENTITYS_MAP::iterator iter = entities.begin();
		for(; iter != entities.end(); iter++)
		{
			if(static_cast<Entity*>(iter->second.get())->getBaseMailbox() != NULL && 
				static_cast<Entity*>(iter->second.get())->getScriptModule()->isPersistent())
			{
				this->destroyEntity(static_cast<Entity*>(iter->second.get())->getID(), true);

				count--;
				done = true;
				break;
			}
		}

		if(!done)
			break;
	}
}

//-------------------------------------------------------------------------------------		
bool Cellapp::initializeWatcher()
{
	ProfileVal::setWarningPeriod(stampsPerSecond() / g_kbeSrvConfig.gameUpdateHertz());

	WATCH_OBJECT("stats/runningTime", &runningTime);
	return EntityApp<Entity>::initializeWatcher();
}

//-------------------------------------------------------------------------------------
bool Cellapp::installPyModules()
{
	Entity::installScript(getScript().getModule());
	GlobalDataClient::installScript(getScript().getModule());

	registerScript(Entity::getScriptType());
	
	// 注册创建entity的方法到py
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(),		time,							__py_gametime,						METH_VARARGS,			0);
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(),		createEntity,					__py_createEntity,					METH_VARARGS,			0);
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(), 		executeRawDatabaseCommand,		__py_executeRawDatabaseCommand,		METH_VARARGS,			0);
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(),		reloadScript,					__py_reloadScript,					METH_VARARGS,			0);
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(),		addSpaceGeometryMapping,		Space::__py_AddSpaceGeometryMapping,METH_VARARGS,			0);
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(),		getSpaceGeometryMapping,		Space::__py_GetSpaceGeometryMapping,METH_VARARGS,			0);
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(),		setSpaceData,					Space::__py_SetSpaceData,			METH_VARARGS,			0);
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(),		getSpaceData,					Space::__py_GetSpaceData,			METH_VARARGS,			0);
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(),		delSpaceData,					Space::__py_DelSpaceData,			METH_VARARGS,			0);
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(),		isShuttingDown,					__py_isShuttingDown,				METH_VARARGS,			0);
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(),		address,						__py_address,						METH_VARARGS,			0);
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
	if(g_kbeSrvConfig.getBaseApp().profiles.open_pyprofile)
	{
		script::PyProfile::stop("kbengine");

		char buf[MAX_BUF];
		kbe_snprintf(buf, MAX_BUF, "baseapp%u.prof", startGroupOrder_);
		script::PyProfile::dump("kbengine", buf);
		script::PyProfile::remove("kbengine");
	}

	unregisterPyObjectToScript("cellAppData");
	S_RELEASE(pCellAppData_); 

	Entity::uninstallScript();
	return EntityApp<Entity>::uninstallPyModules();
}

//-------------------------------------------------------------------------------------
PyObject* Cellapp::__py_gametime(PyObject* self, PyObject* args)
{
	return PyLong_FromUnsignedLong(Cellapp::getSingleton().time());
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
	AUTO_SCOPED_PROFILE("gameTick");

	updateLoad();

	EntityApp<Entity>::handleGameTick();

	updatables_.update();
}

//-------------------------------------------------------------------------------------
bool Cellapp::initializeBegin()
{
	return true;
}

//-------------------------------------------------------------------------------------
bool Cellapp::initializeEnd()
{
	// 如果需要pyprofile则在此处安装
	// 结束时卸载并输出结果
	if(g_kbeSrvConfig.getBaseApp().profiles.open_pyprofile)
	{
		script::PyProfile::start("kbengine");
	}

	pWitnessedTimeoutHandler_ = new WitnessedTimeoutHandler();

	// 是否管理Y轴
	CoordinateSystem::hasY = g_kbeSrvConfig.getCellApp().coordinateSystem_hasY;

	mainDispatcher_.clearSpareTime();

	pTelnetServer_ = new TelnetServer(&this->getMainDispatcher(), &this->getNetworkInterface());
	pTelnetServer_->pScript(&this->getScript());
	return pTelnetServer_->start(g_kbeSrvConfig.getCellApp().telnet_passwd, 
		g_kbeSrvConfig.getCellApp().telnet_deflayer, 
		g_kbeSrvConfig.getCellApp().telnet_port);
}

//-------------------------------------------------------------------------------------
void Cellapp::finalise()
{
	SAFE_RELEASE(pWitnessedTimeoutHandler_);

	if(pTelnetServer_)
	{
		pTelnetServer_->stop();
		SAFE_RELEASE(pTelnetServer_);
	}

	Spaces::finalise();
	EntityApp<Entity>::finalise();
}

//-------------------------------------------------------------------------------------
void Cellapp::onGetEntityAppFromDbmgr(Mercury::Channel* pChannel, int32 uid, std::string& username, 
						int8 componentType, uint64 componentID, int8 globalorderID, int8 grouporderID,
						uint32 intaddr, uint16 intport, uint32 extaddr, uint16 extport, std::string& extaddrEx)
{
	EntityApp<Entity>::onRegisterNewApp(pChannel, uid, username, componentType, componentID, globalorderID, grouporderID,
									intaddr, intport, extaddr, extport, extaddrEx);

	KBEngine::COMPONENT_TYPE tcomponentType = (KBEngine::COMPONENT_TYPE)componentType;

	Components::COMPONENTS& cts = Componentbridge::getComponents().getComponents(DBMGR_TYPE);
	KBE_ASSERT(cts.size() >= 1);
	
	Components::ComponentInfos* cinfos = Componentbridge::getComponents().findComponent(tcomponentType, uid, componentID);
	cinfos->pChannel = NULL;

	int ret = Components::getSingleton().connectComponent(tcomponentType, uid, componentID);
	KBE_ASSERT(ret != -1);

	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();

	switch(tcomponentType)
	{
	case BASEAPP_TYPE:
		(*pBundle).newMessage(BaseappInterface::onRegisterNewApp);
		BaseappInterface::onRegisterNewAppArgs11::staticAddToBundle((*pBundle), getUserUID(), getUsername(), 
			CELLAPP_TYPE, componentID_, startGlobalOrder_, startGroupOrder_,
			this->getNetworkInterface().intaddr().ip, this->getNetworkInterface().intaddr().port, 
			this->getNetworkInterface().extaddr().ip, this->getNetworkInterface().extaddr().port, g_kbeSrvConfig.getConfig().externalAddress);
		break;
	case CELLAPP_TYPE:
		(*pBundle).newMessage(CellappInterface::onRegisterNewApp);
		CellappInterface::onRegisterNewAppArgs11::staticAddToBundle((*pBundle), getUserUID(), getUsername(), 
			CELLAPP_TYPE, componentID_, startGlobalOrder_, startGroupOrder_,
			this->getNetworkInterface().intaddr().ip, this->getNetworkInterface().intaddr().port, 
			this->getNetworkInterface().extaddr().ip, this->getNetworkInterface().extaddr().port, g_kbeSrvConfig.getConfig().externalAddress);
		break;
	default:
		KBE_ASSERT(false && "no support!\n");
		break;
	};
	
	(*pBundle).send(this->getNetworkInterface(), cinfos->pChannel);
	Mercury::Bundle::ObjPool().reclaimObject(pBundle);
}

//-------------------------------------------------------------------------------------
void Cellapp::updateLoad()
{
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
		return 0;
	}
	
	if(entityType == NULL || strlen(entityType) == 0)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::createEntity: entityType is NULL.");
		PyErr_PrintEx(0);
		return 0;
	}

	Space* space = Spaces::findSpace(spaceID);
	if(space == NULL)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::createEntity: spaceID %ld not found.", spaceID);
		PyErr_PrintEx(0);
		return 0;
	}
	
	// 创建entity
	Entity* pEntity = Cellapp::getSingleton().createEntityCommon(entityType, params, false, 0);

	if(pEntity != NULL)
	{
		Py_INCREF(pEntity);
		pEntity->setSpaceID(space->getID());
		pEntity->initializeEntity(params);
		pEntity->pySetPosition(position);
		pEntity->pySetDirection(direction);	
		
		// 添加到space
		space->addEntityAndEnterWorld(pEntity);

		// 有可能在addEntityAndEnterWorld中被销毁了
		// 是否能在创建过程中被销毁还需要考虑
		if(pEntity->isDestroyed())
		{
			Py_DECREF(pEntity);
			return NULL;
		}
	}

	//Py_XDECREF(params);
	return pEntity;
}

//-------------------------------------------------------------------------------------
PyObject* Cellapp::__py_executeRawDatabaseCommand(PyObject* self, PyObject* args)
{
	int argCount = PyTuple_Size(args);
	PyObject* pycallback = NULL;
	int ret = -1;
	ENTITY_ID eid = -1;

	char* data = NULL;
	Py_ssize_t size;
	
	if(argCount == 3)
		ret = PyArg_ParseTuple(args, "s#|O|i", &data, &size, &pycallback, &eid);
	else if(argCount == 2)
		ret = PyArg_ParseTuple(args, "s#|O", &data, &size, &pycallback);
	else if(argCount == 1)
		ret = PyArg_ParseTuple(args, "s#", &data, &size);

	if(ret == -1)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::executeRawDatabaseCommand: args is error!");
		PyErr_PrintEx(0);
	}
	
	Cellapp::getSingleton().executeRawDatabaseCommand(data, size, pycallback, eid);
	S_Return;
}

//-------------------------------------------------------------------------------------
void Cellapp::executeRawDatabaseCommand(const char* datas, uint32 size, PyObject* pycallback, ENTITY_ID eid)
{
	if(datas == NULL)
	{
		ERROR_MSG("KBEngine::executeRawDatabaseCommand: execute is error!\n");
		return;
	}

	Components::COMPONENTS& cts = Components::getSingleton().getComponents(DBMGR_TYPE);
	Components::ComponentInfos* dbmgrinfos = NULL;

	if(cts.size() > 0)
		dbmgrinfos = &(*cts.begin());

	if(dbmgrinfos == NULL || dbmgrinfos->pChannel == NULL || dbmgrinfos->cid == 0)
	{
		ERROR_MSG("KBEngine::executeRawDatabaseCommand: not found dbmgr!\n");
		return;
	}

	INFO_MSG(boost::format("KBEngine::executeRawDatabaseCommand%1%:%2%.\n") % (eid > 0 ? (boost::format("(entityID=%1%)") % eid).str() : "") % datas);

	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
	(*pBundle).newMessage(DbmgrInterface::executeRawDatabaseCommand);
	(*pBundle) << eid;
	(*pBundle) << componentID_ << componentType_;

	CALLBACK_ID callbackID = 0;

	if(pycallback && PyCallable_Check(pycallback))
		callbackID = callbackMgr().save(pycallback);

	(*pBundle) << callbackID;
	(*pBundle) << size;
	(*pBundle).append(datas, size);
	(*pBundle).send(this->getNetworkInterface(), dbmgrinfos->pChannel);
	Mercury::Bundle::ObjPool().reclaimObject(pBundle);
}

//-------------------------------------------------------------------------------------
void Cellapp::onExecuteRawDatabaseCommandCB(Mercury::Channel* pChannel, KBEngine::MemoryStream& s)
{
	std::string err;
	CALLBACK_ID callbackID = 0;
	uint32 nrows = 0;
	uint32 nfields = 0;
	uint64 affectedRows = 0;

	PyObject* pResultSet = NULL;
	PyObject* pAffectedRows = NULL;
	PyObject* pErrorMsg = NULL;

	s >> callbackID;
	s >> err;

	if(err.size() <= 0)
	{
		s >> nfields;

		pErrorMsg = Py_None;
		Py_INCREF(pErrorMsg);

		if(nfields > 0)
		{
			pAffectedRows = Py_None;
			Py_INCREF(pAffectedRows);

			s >> nrows;

			pResultSet = PyList_New(nrows);
			for (uint32 i = 0; i < nrows; ++i)
			{
				PyObject* pRow = PyList_New(nfields);
				for(uint32 j = 0; j < nfields; ++j)
				{
					std::string cell;
					s.readBlob(cell);

					PyObject* pCell = NULL;
						
					if(cell == "NULL")
					{
						Py_INCREF(Py_None);
						pCell = Py_None;
					}
					else
					{
						pCell = PyBytes_FromStringAndSize(cell.data(), cell.length());
					}

					PyList_SET_ITEM(pRow, j, pCell);
				}

				PyList_SET_ITEM(pResultSet, i, pRow);
			}
		}
		else
		{
			pResultSet = Py_None;
			Py_INCREF(pResultSet);

			pErrorMsg = Py_None;
			Py_INCREF(pErrorMsg);

			s >> affectedRows;

			pAffectedRows = PyLong_FromUnsignedLongLong(affectedRows);
		}
	}
	else
	{
			pResultSet = Py_None;
			Py_INCREF(pResultSet);

			pErrorMsg = PyUnicode_FromString(err.c_str());

			pAffectedRows = Py_None;
			Py_INCREF(pAffectedRows);
	}

	DEBUG_MSG(boost::format("Cellapp::onExecuteRawDatabaseCommandCB: nrows=%1%, nfields=%2%, err=%3%.\n") % 
		nrows % nfields % err.c_str());

	if(callbackID > 0)
	{
		SCOPED_PROFILE(SCRIPTCALL_PROFILE);

		PyObjectPtr pyfunc = pyCallbackMgr_.take(callbackID);
		if(pyfunc != NULL)
		{
			PyObject* pyResult = PyObject_CallFunction(pyfunc.get(), 
												const_cast<char*>("OOO"), 
												pResultSet, pAffectedRows, pErrorMsg);

			if(pyResult != NULL)
				Py_DECREF(pyResult);
			else
				SCRIPT_ERROR_CHECK();
		}
		else
		{
			ERROR_MSG(boost::format("Cellapp::onExecuteRawDatabaseCommandCB: can't found callback:%1%.\n") %
				callbackID);
		}
	}

	Py_XDECREF(pResultSet);
	Py_XDECREF(pAffectedRows);
	Py_XDECREF(pErrorMsg);
}

//-------------------------------------------------------------------------------------
void Cellapp::reqBackupEntityCellData(Mercury::Channel* pChannel, KBEngine::MemoryStream& s)
{
	ENTITY_ID entityID = 0;
	s >> entityID;

	Entity* e = this->findEntity(entityID);
	if(!e)
	{
		WARNING_MSG(boost::format("Cellapp::reqBackupEntityCellData: not found entity %1%.\n") % entityID);
		return;
	}

	e->backupCellData();
}

//-------------------------------------------------------------------------------------
void Cellapp::reqWriteToDBFromBaseapp(Mercury::Channel* pChannel, KBEngine::MemoryStream& s)
{
	ENTITY_ID entityID = 0;
	CALLBACK_ID callbackID = 0;

	s >> entityID;
	s >> callbackID;

	Entity* e = this->findEntity(entityID);
	if(!e)
	{
		WARNING_MSG(boost::format("Cellapp::reqWriteToDBFromBaseapp: not found entity %1%.\n") % entityID);
		return;
	}

	e->writeToDB(&callbackID);
}

//-------------------------------------------------------------------------------------
void Cellapp::onDbmgrInitCompleted(Mercury::Channel* pChannel, 
		GAME_TIME gametime, ENTITY_ID startID, ENTITY_ID endID, int32 startGlobalOrder, int32 startGroupOrder, const std::string& digest)
{
	EntityApp<Entity>::onDbmgrInitCompleted(pChannel, gametime, startID, endID, startGlobalOrder, startGroupOrder, digest);
	
	SCOPED_PROFILE(SCRIPTCALL_PROFILE);

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
void Cellapp::onBroadcastCellAppDataChanged(Mercury::Channel* pChannel, KBEngine::MemoryStream& s)
{

	std::string key, value;
	bool isDelete;
	
	s >> isDelete;
	s.readBlob(key);

	if(!isDelete)
	{
		s.readBlob(value);
	}

	PyObject * pyKey = script::Pickler::unpickle(key);
	if(pyKey == NULL)
	{
		ERROR_MSG("Cellapp::onBroadcastCellAppDataChanged: no has key!\n");
		return;
	}

	Py_INCREF(pyKey);

	if(isDelete)
	{
		if(pCellAppData_->del(pyKey))
		{
			SCOPED_PROFILE(SCRIPTCALL_PROFILE);

			// 通知脚本
			SCRIPT_OBJECT_CALL_ARGS1(getEntryScript().get(), const_cast<char*>("onCellAppDataDel"), 
				const_cast<char*>("O"), pyKey);
		}
	}
	else
	{
		PyObject * pyValue = script::Pickler::unpickle(value);

		if(pyValue == NULL)
		{
			ERROR_MSG("Cellapp::onBroadcastCellAppDataChanged: no has value!\n");
			Py_DECREF(pyKey);
			return;
		}

		Py_INCREF(pyValue);

		if(pCellAppData_->write(pyKey, pyValue))
		{
			SCOPED_PROFILE(SCRIPTCALL_PROFILE);

			// 通知脚本
			SCRIPT_OBJECT_CALL_ARGS2(getEntryScript().get(), const_cast<char*>("onCellAppData"), 
				const_cast<char*>("OO"), pyKey, pyValue);
		}

		Py_DECREF(pyValue);
	}

	Py_DECREF(pyKey);
}

//-------------------------------------------------------------------------------------
void Cellapp::onCreateInNewSpaceFromBaseapp(Mercury::Channel* pChannel, KBEngine::MemoryStream& s)
{
	std::string entityType;
	ENTITY_ID mailboxEntityID;
	COMPONENT_ID componentID;
	SPACE_ID spaceID = 1;

	s >> entityType;
	s >> mailboxEntityID;
	s >> spaceID;
	s >> componentID;

	// DEBUG_MSG("Cellapp::onCreateInNewSpaceFromBaseapp: spaceID=%u, entityType=%s, entityID=%d, componentID=%"PRAppID".\n", 
	//	spaceID, entityType.c_str(), mailboxEntityID, componentID);

	Space* space = Spaces::createNewSpace(spaceID);
	if(space != NULL)
	{
		// 创建entity
		Entity* e = createEntityCommon(entityType.c_str(), NULL, false, mailboxEntityID, false);
		
		if(e == NULL)
		{
			s.opfini();
			return;
		}

		PyObject* cellData = e->createCellDataFromStream(&s);

		// 设置entity的baseMailbox
		EntityMailbox* mailbox = new EntityMailbox(e->getScriptModule(), NULL, componentID, mailboxEntityID, MAILBOX_TYPE_BASE);
		e->setBaseMailbox(mailbox);
		
		// 此处baseapp可能还有没初始化过来， 所以有一定概率是为None的
		Components::ComponentInfos* cinfos = Components::getSingleton().findComponent(BASEAPP_TYPE, componentID);
		if(cinfos == NULL || cinfos->pChannel == NULL)
		{
			Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
			ForwardItem* pFI = new ForwardItem();
			pFI->pHandler = new FMH_Baseapp_onEntityGetCellFrom_onCreateInNewSpaceFromBaseapp(e, spaceID, cellData);
			//Py_XDECREF(cellData);
			pFI->pBundle = pBundle;
			(*pBundle).newMessage(BaseappInterface::onEntityGetCell);
			BaseappInterface::onEntityGetCellArgs3::staticAddToBundle((*pBundle), mailboxEntityID, componentID_, spaceID);
			forward_messagebuffer_.push(componentID, pFI);
			WARNING_MSG(boost::format("Cellapp::onCreateInNewSpaceFromBaseapp: not found baseapp(%1%), message is buffered.\n") %
				componentID);
			return;
		}
		
		e->setSpaceID(space->getID());
		e->initializeEntity(cellData);
		Py_XDECREF(cellData);

		// 添加到space
		space->creatorID(e->getID());
		space->addEntityAndEnterWorld(e);

		Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
		(*pBundle).newMessage(BaseappInterface::onEntityGetCell);
		BaseappInterface::onEntityGetCellArgs3::staticAddToBundle((*pBundle), mailboxEntityID, componentID_, spaceID);
		(*pBundle).send(this->getNetworkInterface(), cinfos->pChannel);
		Mercury::Bundle::ObjPool().reclaimObject(pBundle);
		return;
	}
	
	ERROR_MSG(boost::format("Cellapp::onCreateInNewSpaceFromBaseapp: not found baseapp[%1%], entityID=%2%, spaceID=%3%.\n") %
		componentID % mailboxEntityID % spaceID);
}

//-------------------------------------------------------------------------------------
void Cellapp::onRestoreSpaceInCellFromBaseapp(Mercury::Channel* pChannel, KBEngine::MemoryStream& s)
{
	std::string entityType;
	ENTITY_ID mailboxEntityID;
	COMPONENT_ID componentID;
	SPACE_ID spaceID = 1;

	s >> entityType;
	s >> mailboxEntityID;
	s >> spaceID;
	s >> componentID;

	// DEBUG_MSG("Cellapp::onRestoreSpaceInCellFromBaseapp: spaceID=%u, entityType=%s, entityID=%d, componentID=%"PRAppID".\n", 
	//	spaceID, entityType.c_str(), mailboxEntityID, componentID);

	Space* space = Spaces::createNewSpace(spaceID);
	if(space != NULL)
	{
		// 创建entity
		Entity* e = createEntityCommon(entityType.c_str(), NULL, false, mailboxEntityID, false);
		
		if(e == NULL)
		{
			s.opfini();
			return;
		}

		PyObject* cellData = e->createCellDataFromStream(&s);

		// 设置entity的baseMailbox
		EntityMailbox* mailbox = new EntityMailbox(e->getScriptModule(), NULL, componentID, mailboxEntityID, MAILBOX_TYPE_BASE);
		e->setBaseMailbox(mailbox);
		
		// 此处baseapp可能还有没初始化过来， 所以有一定概率是为None的
		Components::ComponentInfos* cinfos = Components::getSingleton().findComponent(BASEAPP_TYPE, componentID);
		if(cinfos == NULL || cinfos->pChannel == NULL)
		{
			Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
			ForwardItem* pFI = new ForwardItem();
			pFI->pHandler = new FMH_Baseapp_onEntityGetCellFrom_onCreateInNewSpaceFromBaseapp(e, spaceID, cellData);
			//Py_XDECREF(cellData);
			pFI->pBundle = pBundle;
			(*pBundle).newMessage(BaseappInterface::onEntityGetCell);
			BaseappInterface::onEntityGetCellArgs3::staticAddToBundle((*pBundle), mailboxEntityID, componentID_, spaceID);
			forward_messagebuffer_.push(componentID, pFI);
			WARNING_MSG(boost::format("Cellapp::onRestoreSpaceInCellFromBaseapp: not found baseapp(%1%), message is buffered.\n") %
				componentID);
			return;
		}
		
		e->setSpaceID(space->getID());
		e->createNamespace(cellData);
		Py_XDECREF(cellData);

		// 添加到space
		space->creatorID(e->getID());
		e->onRestore();

		space->addEntityAndEnterWorld(e, true);

		Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
		(*pBundle).newMessage(BaseappInterface::onEntityGetCell);
		BaseappInterface::onEntityGetCellArgs3::staticAddToBundle((*pBundle), mailboxEntityID, componentID_, spaceID);
		(*pBundle).send(this->getNetworkInterface(), cinfos->pChannel);
		Mercury::Bundle::ObjPool().reclaimObject(pBundle);
		return;
	}
	
	ERROR_MSG(boost::format("Cellapp::onRestoreSpaceInCellFromBaseapp: not found baseapp[%1%], entityID=%2%, spaceID=%3%.\n") %
		componentID % mailboxEntityID % spaceID);
}

//-------------------------------------------------------------------------------------
void Cellapp::onCreateCellEntityFromBaseapp(Mercury::Channel* pChannel, KBEngine::MemoryStream& s)
{
	std::string entityType;
	ENTITY_ID createToEntityID, entityID;
	
	COMPONENT_ID componentID;
	SPACE_ID spaceID = 1;
	bool hasClient;
	bool inRescore = false;

	s >> createToEntityID;
	s >> entityType;
	s >> entityID;
	s >> componentID;
	s >> hasClient;
	s >> inRescore;

	// 此处baseapp可能还有没初始化过来， 所以有一定概率是为None的
	Components::ComponentInfos* cinfos = Components::getSingleton().findComponent(BASEAPP_TYPE, componentID);
	if(cinfos == NULL || cinfos->pChannel == NULL)
	{
		MemoryStream* pCellData = MemoryStream::ObjPool().createObject();
		pCellData->append(s);

		Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
		ForwardItem* pFI = new ForwardItem();
		pFI->pHandler = new FMH_Baseapp_onEntityGetCellFrom_onCreateCellEntityFromBaseapp(entityType, createToEntityID, 
			entityID, pCellData, hasClient, inRescore, componentID, spaceID);

		pFI->pBundle = pBundle;
		(*pBundle).newMessage(BaseappInterface::onEntityGetCell);
		BaseappInterface::onEntityGetCellArgs3::staticAddToBundle((*pBundle), entityID, componentID_, spaceID);
		forward_messagebuffer_.push(componentID, pFI);

		WARNING_MSG(boost::format("Cellapp::onCreateCellEntityFromBaseapp: not found baseapp(%1%), message is buffered.\n") %
			componentID);
		return;
	}

	_onCreateCellEntityFromBaseapp(entityType, createToEntityID, entityID, 
					&s, hasClient, inRescore, componentID, spaceID);

}

//-------------------------------------------------------------------------------------
void Cellapp::_onCreateCellEntityFromBaseapp(std::string& entityType, ENTITY_ID createToEntityID, ENTITY_ID entityID,
											MemoryStream* pCellData, bool hasClient, bool inRescore, COMPONENT_ID componentID, 
											SPACE_ID spaceID)
{
	// 注意：此处理论不会找不到组件， 因为onCreateCellEntityFromBaseapp中已经进行过一次消息缓存判断
	Components::ComponentInfos* cinfos = Components::getSingleton().findComponent(BASEAPP_TYPE, componentID);
	KBE_ASSERT(cinfos != NULL && cinfos->pChannel != NULL);

	Entity* pCreateToEntity = pEntities_->find(createToEntityID);

	// 可能spaceEntity已经销毁了， 但还未来得及通知到baseapp时
	// base部分在向这个space创建entity
	if(pCreateToEntity == NULL)
	{
		ERROR_MSG("Cellapp::_onCreateCellEntityFromBaseapp: not fount spaceEntity. may have been destroyed!\n");

		Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
		pBundle->newMessage(BaseappInterface::onCreateCellFailure);
		BaseappInterface::onCreateCellFailureArgs1::staticAddToBundle(*pBundle, entityID);
		pBundle->send(this->getNetworkInterface(), cinfos->pChannel);
		Mercury::Bundle::ObjPool().reclaimObject(pBundle);
		return;
	}

	spaceID = pCreateToEntity->getSpaceID();

	//DEBUG_MSG("Cellapp::onCreateCellEntityFromBaseapp: spaceID=%u, entityType=%s, entityID=%d, componentID=%"PRAppID".\n", 
	//	spaceID, entityType.c_str(), entityID, componentID);

	Space* space = Spaces::findSpace(spaceID);
	if(space != NULL)
	{
		// 告知baseapp， entity的cell创建了
		Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
		pBundle->newMessage(BaseappInterface::onEntityGetCell);
		BaseappInterface::onEntityGetCellArgs3::staticAddToBundle(*pBundle, entityID, componentID_, spaceID);
		pBundle->send(this->getNetworkInterface(), cinfos->pChannel);
		Mercury::Bundle::ObjPool().reclaimObject(pBundle);

		// 解包cellData信息.
		PyObject* cellData = NULL;
	
		// 创建entity
		Entity* e = createEntityCommon(entityType.c_str(), cellData, false, entityID, false);
		
		if(e == NULL)
		{
			Py_XDECREF(cellData);
			return;
		}

		// 设置entity的baseMailbox
		EntityMailbox* mailbox = new EntityMailbox(e->getScriptModule(), NULL, componentID, entityID, MAILBOX_TYPE_BASE);
		e->setBaseMailbox(mailbox);
		
		cellData = e->createCellDataFromStream(pCellData);

		if(hasClient)
		{
			KBE_ASSERT(e->getBaseMailbox() != NULL && !e->hasWitness());
			PyObject* clientMailbox = PyObject_GetAttrString(e->getBaseMailbox(), "client");
			KBE_ASSERT(clientMailbox != Py_None);

			EntityMailbox* client = static_cast<EntityMailbox*>(clientMailbox);	
			// Py_INCREF(clientMailbox); 这里不需要增加引用， 因为每次都会产生一个新的对象

			// 为了能够让entity.__init__中能够修改属性立刻能广播到客户端我们需要提前设置这些
			e->setClientMailbox(client);
			e->setWitness(Witness::ObjPool().createObject());
		}

		space->addEntity(e);

		if(!inRescore)
		{
			e->initializeEntity(cellData);
		}
		else
		{
			e->createNamespace(cellData);
			e->onRestore();
		}

		Py_XDECREF(cellData);
		
		// 这里增加一个引用， 因为可能在进入时被销毁
		Py_INCREF(e);
		space->addEntityToNode(e);
		bool isDestroyed = e->isDestroyed();
		Py_DECREF(e);

		if(isDestroyed == true)
			return;

		// 如果是有client的entity则设置它的clientmailbox, baseapp部分的onEntityGetCell会告知客户端enterworld.
		if(hasClient)
		{
			e->onGetWitness(cinfos->pChannel);
		}
		else
		{
			space->onEnterWorld(e);
		}

		return;
	}

	KBE_ASSERT(false && "Cellapp::onCreateCellEntityFromBaseapp: is error!\n");
}

//-------------------------------------------------------------------------------------
void Cellapp::onDestroyCellEntityFromBaseapp(Mercury::Channel* pChannel, ENTITY_ID eid)
{
	// DEBUG_MSG("Cellapp::onDestroyCellEntityFromBaseapp:entityID=%d.\n", eid);
	destroyEntity(eid, true);
}

//-------------------------------------------------------------------------------------
RemoteEntityMethod* Cellapp::createMailboxCallEntityRemoteMethod(MethodDescription* md, EntityMailbox* pMailbox)
{
	return new EntityRemoteMethod(md, pMailbox);
}

//-------------------------------------------------------------------------------------
void Cellapp::onEntityMail(Mercury::Channel* pChannel, KBEngine::MemoryStream& s)
{
	ENTITY_ID eid;
	s >> eid;

	ENTITY_MAILBOX_TYPE	mailtype;
	s >> mailtype;

	// 在本地区尝试查找该收件人信息， 看收件人是否属于本区域
	Entity* entity = pEntities_->find(eid);
	if(entity == NULL)
	{
		ERROR_MSG(boost::format("Cellapp::onEntityMail: entityID %1% not found.\n") % eid);
		s.opfini();
		return;
	}
	
	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
	Mercury::Bundle& bundle = *pBundle;

	switch(mailtype)
	{
		case MAILBOX_TYPE_CELL:																		// 本组件是baseapp，那么确认邮件的目的地是这里， 那么执行最终操作
			entity->onRemoteMethodCall(pChannel, s);
			break;
		case MAILBOX_TYPE_BASE_VIA_CELL: // entity.base.cell.xxx
			{
				EntityMailboxAbstract* mailbox = static_cast<EntityMailboxAbstract*>(entity->getBaseMailbox());
				if(mailbox == NULL)
				{
					ERROR_MSG(boost::format("Cellapp::onEntityMail: occur a error(can't found baseMailbox)! mailboxType=%1%, entityID=%2%.\n") %
						mailtype % eid);

					break;
				}
				
				mailbox->newMail(bundle);
				bundle.append(s);
				mailbox->postMail(bundle);
			}
			break;
		case MAILBOX_TYPE_CLIENT_VIA_CELL: // entity.cell.client
			{
				EntityMailboxAbstract* mailbox = static_cast<EntityMailboxAbstract*>(entity->getClientMailbox());
				if(mailbox == NULL)
				{
					ERROR_MSG(boost::format("Cellapp::onEntityMail: occur a error(can't found clientMailbox)! mailboxType=%1%, entityID=%2%.\n") %
						mailtype % eid);

					break;
				}
				
				mailbox->newMail(bundle);
				bundle.append(s);
				s.read_skip(s.opsize());
				mailbox->postMail(bundle);
			}
			break;
		default:
			{
				ERROR_MSG(boost::format("Cellapp::onEntityMail: mailboxType %1% is error! must a cellType. entityID=%2%.\n") %
					mailtype % eid);
			}
	};

	Mercury::Bundle::ObjPool().reclaimObject(pBundle);
	s.opfini();
}

//-------------------------------------------------------------------------------------
void Cellapp::onRemoteCallMethodFromClient(Mercury::Channel* pChannel, KBEngine::MemoryStream& s)
{
	ENTITY_ID srcEntityID, targetID;

	s >> srcEntityID >> targetID;

	KBEngine::Entity* e = KBEngine::Cellapp::getSingleton().findEntity(targetID);		

	if(e == NULL)
	{	
		WARNING_MSG(boost::format("Cellapp::onRemoteCallMethodFromClient: can't found entityID:%1%, by srcEntityID:%2%.\n") % 
			targetID % srcEntityID);
		
		s.read_skip(s.opsize());
		return;
	}

	// 这个方法呼叫如果不是这个proxy自己的方法则必须呼叫的entity和proxy的cellEntity在一个space中。
	try
	{
		e->onRemoteCallMethodFromClient(pChannel, s);
	}catch(MemoryStreamException &)
	{
		ERROR_MSG(boost::format("Cellapp::onRemoteCallMethodFromClient: message is error! entityID:%1%.\n") % 
			targetID);

		s.read_skip(s.opsize());
		return;
	}
}

//-------------------------------------------------------------------------------------
void Cellapp::onUpdateDataFromClient(Mercury::Channel* pChannel, KBEngine::MemoryStream& s)
{
	ENTITY_ID srcEntityID = 0;

	s >> srcEntityID;
	if(srcEntityID <= 0)
		return;
	
	if(s.opsize() <= 0)
		return;

	KBEngine::Entity* e = findEntity(srcEntityID);	

	if(e == NULL)
	{
		ERROR_MSG(boost::format("Cellapp::onUpdateDataFromClient: not found entity %1%!\n") % srcEntityID);
		
		s.read_skip(s.opsize());
		return;
	}

	e->onUpdateDataFromClient(s);
	s.read_skip(s.opsize());
}

//-------------------------------------------------------------------------------------
void Cellapp::forwardEntityMessageToCellappFromClient(Mercury::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID srcEntityID;

	s >> srcEntityID;

	KBEngine::Entity* e = KBEngine::Cellapp::getSingleton().findEntity(srcEntityID);		

	if(e == NULL)
	{	
		WARNING_MSG(boost::format("Cellapp::forwardEntityMessageToCellappFromClient: can't found entityID:%1%.\n") %
			srcEntityID);
		
		s.read_skip(s.opsize());
		return;
	}

	if(e->isDestroyed())																				
	{																										
		ERROR_MSG(boost::format("%1%::forwardEntityMessageToCellappFromClient: %2% is destroyed!\n") %										
			e->getScriptName() % e->getID());

		s.read_skip(s.opsize());
		return;																							
	}

	// 检查是否是entity消息， 否则不合法.
	while(s.opsize() > 0 && !e->isDestroyed())
	{
		Mercury::MessageID			currMsgID;
		Mercury::MessageLength		currMsgLen;

		s >> currMsgID;

		Mercury::MessageHandler* pMsgHandler = CellappInterface::messageHandlers.find(currMsgID);

		if(pMsgHandler == NULL)
		{
			ERROR_MSG(boost::format("Cellapp::forwardEntityMessageToCellappFromClient: invalide msgID=%1%, msglen=%2%, from %3%.\n") % 
				currMsgID % s.wpos() % pChannel->c_str());

			s.read_skip(s.opsize());
			return;
		}

		if(pMsgHandler->type() != Mercury::MERCURY_MESSAGE_TYPE_ENTITY)
		{
			WARNING_MSG(boost::format("Cellapp::forwardEntityMessageToCellappFromClient: msgID=%1% not is entitymsg.\n") %
				currMsgID);

			s.read_skip(s.opsize());
			return;
		}

		if((pMsgHandler->msgLen == MERCURY_VARIABLE_MESSAGE) || Mercury::g_packetAlwaysContainLength)
			s >> currMsgLen;
		else
			currMsgLen = pMsgHandler->msgLen;

		if(s.opsize() < currMsgLen || currMsgLen >  MERCURY_MESSAGE_MAX_SIZE / 2)
		{
			ERROR_MSG(boost::format("Cellapp::forwardEntityMessageToCellappFromClient: msgID=%1%, invalide msglen=%2%, from %3%.\n") % 
				currMsgID % s.wpos() % pChannel->c_str());

			s.read_skip(s.opsize());
			return;
		}

		// 临时设置有效读取位， 防止接口中溢出操作
		size_t wpos = s.wpos();
		// size_t rpos = s.rpos();
		size_t frpos = s.rpos() + currMsgLen;
		s.wpos(frpos);

		try
		{
			pMsgHandler->handle(pChannel, s);
		}catch(MemoryStreamException &)
		{
			ERROR_MSG(boost::format("Cellapp::forwardEntityMessageToCellappFromClient: message is error! entityID:%1%.\n") % 
				srcEntityID);

			s.read_skip(s.opsize());
			return;
		}

		// 防止handle中没有将数据导出获取非法操作
		if(currMsgLen > 0)
		{
			if(frpos != s.rpos())
			{
				CRITICAL_MSG(boost::format("Cellapp::forwardEntityMessageToCellappFromClient[%1%]: rpos(%2%) invalid, expect=%3%. msgID=%4%, msglen=%5%.\n") %
					pMsgHandler->name.c_str() % s.rpos() % frpos % currMsgID % currMsgLen);

				s.rpos(frpos);
			}
		}

		s.wpos(wpos);
	}
}

//-------------------------------------------------------------------------------------
bool Cellapp::addUpdatable(Updatable* pObject)
{
	return updatables_.add(pObject);
}

//-------------------------------------------------------------------------------------
bool Cellapp::removeUpdatable(Updatable* pObject)
{
	return updatables_.remove(pObject);
}

//-------------------------------------------------------------------------------------
void Cellapp::lookApp(Mercury::Channel* pChannel)
{
	if(pChannel->isExternal())
		return;

	DEBUG_MSG(boost::format("Cellapp::lookApp: %1%\n") % pChannel->c_str());

	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
	
	(*pBundle) << g_componentType;
	(*pBundle) << componentID_;

	ShutdownHandler::SHUTDOWN_STATE state = shuttingdown();
	int8 istate = int8(state);
	(*pBundle) << istate;
	(*pBundle) << this->entitiesSize();
	(*pBundle) << cells_.size();

	uint32 port = 0;
	if(pTelnetServer_)
		port = pTelnetServer_->port();

	(*pBundle) << port;

	(*pBundle).send(getNetworkInterface(), pChannel);

	Mercury::Bundle::ObjPool().reclaimObject(pBundle);
}

//-------------------------------------------------------------------------------------
PyObject* Cellapp::__py_reloadScript(PyObject* self, PyObject* args)
{
	bool fullReload = true;
	int argCount = PyTuple_Size(args);
	if(argCount == 1)
	{
		if(PyArg_ParseTuple(args, "b", &fullReload) == -1)
		{
			PyErr_Format(PyExc_TypeError, "KBEngine::reloadScript(fullReload): args is error!");
			PyErr_PrintEx(0);
			return 0;
		}
	}

	Cellapp::getSingleton().reloadScript(fullReload);
	S_Return;
}

//-------------------------------------------------------------------------------------
void Cellapp::reloadScript(bool fullReload)
{
	EntityApp<Entity>::reloadScript(fullReload);
}

//-------------------------------------------------------------------------------------
void Cellapp::onReloadScript(bool fullReload)
{
	Entities<Entity>::ENTITYS_MAP& entities = pEntities_->getEntities();
	Entities<Entity>::ENTITYS_MAP::iterator eiter = entities.begin();
	for(; eiter != entities.end(); eiter++)
	{
		static_cast<Entity*>(eiter->second.get())->reload(fullReload);
	}

	EntityApp<Entity>::onReloadScript(fullReload);
}

//-------------------------------------------------------------------------------------
PyObject* Cellapp::__py_isShuttingDown(PyObject* self, PyObject* args)
{
	return PyBool_FromLong(Cellapp::getSingleton().isShuttingdown() ? 1 : 0);
}

//-------------------------------------------------------------------------------------
PyObject* Cellapp::__py_address(PyObject* self, PyObject* args)
{
	PyObject* pyobj = PyTuple_New(2);
	const Mercury::Address& addr = Cellapp::getSingleton().getNetworkInterface().intEndpoint().addr();
	PyTuple_SetItem(pyobj, 0,  PyLong_FromUnsignedLong(addr.ip));
	PyTuple_SetItem(pyobj, 1,  PyLong_FromUnsignedLong(addr.port));
	return pyobj;
}

//-------------------------------------------------------------------------------------
void Cellapp::reqTeleportOtherValidation(Mercury::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID nearbyMBRefID = 0, teleportEntityID = 0;
	COMPONENT_ID cellComponentID = 0;
	s >> teleportEntityID >> nearbyMBRefID >> cellComponentID;

	Entity* entity = Cellapp::getSingleton().findEntity(nearbyMBRefID);
	SPACE_ID spaceID = 0;

	if(entity)
	{
		Space* space = Spaces::findSpace(entity->getSpaceID());
		if(space == NULL || (space->creatorID() == entity->getID() && entity->isDestroyed()))
		{
			spaceID = 0;
		}
		else
		{
			spaceID = space->getID();
		}
	}

	Mercury::Channel* pCellappChannel = Components::getSingleton().findComponent(cellComponentID)->pChannel;

	if(pCellappChannel == NULL)
	{
		ERROR_MSG(boost::format("Cellapp::reqTeleportOtherValidation: not found cellapp(%1%)!\n") % cellComponentID);
		return;
	}

	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
	(*pBundle).newMessage(CellappInterface::reqTeleportOtherAck);
	(*pBundle) << teleportEntityID;
	(*pBundle) << nearbyMBRefID;
	(*pBundle) << spaceID;

	pBundle->send(this->getNetworkInterface(), pCellappChannel);
	Mercury::Bundle::ObjPool().reclaimObject(pBundle);
}

//-------------------------------------------------------------------------------------
void Cellapp::reqTeleportOtherAck(Mercury::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID nearbyMBRefID = 0, teleportEntityID = 0;
	SPACE_ID spaceID = 0;

	s >> teleportEntityID >> nearbyMBRefID >> spaceID;
	
	Entity* entity = Cellapp::getSingleton().findEntity(teleportEntityID);

	if(entity)
	{
		entity->onReqTeleportOtherAck(pChannel, nearbyMBRefID, spaceID);
	}
	else
	{
		ERROR_MSG(boost::format("Cellapp::reqTeleportOtherAck: not found reqTeleportEntity(%1%)!\n") % teleportEntityID);
	}
}

//-------------------------------------------------------------------------------------
void Cellapp::reqTeleportOther(Mercury::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID nearbyMBRefID = 0, teleportEntityID = 0;
	Position3D pos;
	Direction3D dir;
	std::string entityType;
	SPACE_ID spaceID = 0, lastSpaceID = 0;

	s >> teleportEntityID >> nearbyMBRefID >> lastSpaceID >> spaceID;
	s >> entityType;
	s >> pos.x >> pos.y >> pos.z;
	s >> dir.dir.x >> dir.dir.y >> dir.dir.z;

	Space* space = Spaces::findSpace(spaceID);
	if(space == NULL)
	{
		ERROR_MSG(boost::format("Cellapp::reqTeleportOther: not found space(%1%),  reqTeleportEntity(%2%)!\n") % spaceID % teleportEntityID);
		s.opfini();
		return;
	}

	// 创建entity
	Entity* e = createEntityCommon(entityType.c_str(), NULL, false, teleportEntityID, false);
	if(e == NULL)
	{
		ERROR_MSG(boost::format("Cellapp::reqTeleportOther: create reqTeleportEntity(%1%) is error!\n") % teleportEntityID);
		s.opfini();
		return;
	}

	e->setSpaceID(space->getID());
	e->setPositionAndDirection(pos, dir);

	// 读取entity的celldata并且创建entity
	PyObject* cellData = e->createCellDataFromStream(&s);
	e->createNamespace(cellData);
	Py_XDECREF(cellData);
	
	space->addEntityAndEnterWorld(e);

	Entity* nearbyMBRef = Cellapp::getSingleton().findEntity(nearbyMBRefID);
	e->onTeleportSuccess(nearbyMBRef, lastSpaceID);
}

//-------------------------------------------------------------------------------------

}
