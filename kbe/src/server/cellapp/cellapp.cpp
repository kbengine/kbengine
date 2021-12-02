// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#include "cellapp.h"
#include "space.h"
#include "spacememory.h"
#include "profile.h"
#include "witness.h"
#include "coordinate_node.h"
#include "view_trigger.h"
#include "watch_obj_pools.h"
#include "cellapp_interface.h"
#include "entity_remotemethod.h"
#include "initprogress_handler.h"
#include "forward_message_over_handler.h"
#include "network/tcp_packet.h"
#include "network/udp_packet.h"
#include "network/network_stats.h"
#include "server/components.h"
#include "server/telnet_server.h"
#include "server/py_file_descriptor.h"
#include "dbmgr/dbmgr_interface.h"
#include "navigation/navigation.h"
#include "client_lib/client_interface.h"
#include "common/sha1.h"

#include "../../server/baseappmgr/baseappmgr_interface.h"
#include "../../server/cellappmgr/cellappmgr_interface.h"
#include "../../server/baseapp/baseapp_interface.h"
#include "../../server/cellapp/cellapp_interface.h"
#include "../../server/dbmgr/dbmgr_interface.h"
#include "../../server/loginapp/loginapp_interface.h"

namespace KBEngine{
	
ServerConfig g_serverConfig;
KBE_SINGLETON_INIT(Cellapp);

Navigation g_navigation;

//-------------------------------------------------------------------------------------
Cellapp::Cellapp(Network::EventDispatcher& dispatcher, 
			 Network::NetworkInterface& ninterface, 
			 COMPONENT_TYPE componentType,
			 COMPONENT_ID componentID):
	EntityApp<Entity>(dispatcher, ninterface, componentType, componentID),
	pCellAppData_(NULL),
	forward_messagebuffer_(ninterface),
	cells_(),
	pTelnetServer_(NULL),
	pWitnessedTimeoutHandler_(NULL),
	pGhostManager_(NULL),
	flags_(APP_FLAGS_NONE),
	spaceViewers_(),
	pInitProgressHandler_(NULL)
{
	KBEngine::Network::MessageHandlers::pMainMessageHandlers = &CellappInterface::messageHandlers;

	// hook entitycallcall
	static EntityCallAbstract::EntityCallCallHookFunc entitycallCallHookFunc = std::tr1::bind(&Cellapp::createEntityCallCallEntityRemoteMethod, this,
		std::tr1::placeholders::_1, std::tr1::placeholders::_2);

	EntityCallAbstract::setEntityCallCallHookFunc(&entitycallCallHookFunc);
}

//-------------------------------------------------------------------------------------
Cellapp::~Cellapp()
{
	pInitProgressHandler_ = NULL;
	EntityCallAbstract::resetCallHooks();
}

//-------------------------------------------------------------------------------------	
ShutdownHandler::CAN_SHUTDOWN_STATE Cellapp::canShutdown()
{
	Entities<Entity>::ENTITYS_MAP& entities =  this->pEntities()->getEntities();
	Entities<Entity>::ENTITYS_MAP::iterator iter = entities.begin();
	for(; iter != entities.end(); ++iter)
	{
		//Entity* pEntity = static_cast<Entity*>(iter->second.get());
		//if(pEntity->baseEntityCall() != NULL && 
		//		pEntity->pScriptModule()->isPersistent())
		{
			INFO_MSG(fmt::format("Cellapp::canShutdown(): Wait for the entity's into the database! The remaining {}.\n",
				entities.size()));

			lastShutdownFailReason_ = "destroyHasBaseEntitys";
			return ShutdownHandler::CAN_SHUTDOWN_STATE_FALSE;
		}
	}

	return ShutdownHandler::CAN_SHUTDOWN_STATE_TRUE;
}

//-------------------------------------------------------------------------------------	
void Cellapp::onShutdown(bool first)
{
	EntityApp<Entity>::onShutdown(first);

	uint32 count = g_serverConfig.getCellApp().perSecsDestroyEntitySize;
	Entities<Entity>::ENTITYS_MAP& entities =  this->pEntities()->getEntities();

	while(count > 0 && entities.size() > 0)
	{
		std::vector<ENTITY_ID> vecs;
		
		Entities<Entity>::ENTITYS_MAP::iterator iter = entities.begin();
		for(; iter != entities.end(); ++iter)
		{
			//Entity* pEntity = static_cast<Entity*>(iter->second.get());
			//if(pEntity->baseEntityCall() != NULL && 
			//	pEntity->pScriptModule()->isPersistent())
			{
				vecs.push_back(static_cast<Entity*>(iter->second.get())->id());

				if(--count == 0)
					break;
			}
		}

		std::vector<ENTITY_ID>::iterator iter1 = vecs.begin();
		for(; iter1 != vecs.end(); ++iter1)
		{
			Entity* e = this->findEntity((*iter1));
			if(!e)
				continue;
			
			this->destroyEntity((*iter1), true);
		}
	}

	// ���count����perSecsDestroyEntitySize˵�������Ѿ�û�пɴ���Ķ�����
	// ʣ�µ�Ӧ�ö���space�����Կ�ʼ������
	if(count == g_serverConfig.getCellApp().perSecsDestroyEntitySize)
		SpaceMemorys::finalise();
}

//-------------------------------------------------------------------------------------		
bool Cellapp::initializeWatcher()
{
	ProfileVal::setWarningPeriod(stampsPerSecond() / g_kbeSrvConfig.gameUpdateHertz());

	WATCH_OBJECT("load", this, &Cellapp::_getLoad);
	WATCH_OBJECT("spaceSize", &KBEngine::getUsername);
	WATCH_OBJECT("stats/runningTime", &runningTime);
	return EntityApp<Entity>::initializeWatcher() && WatchObjectPool::initWatchPools();
}

//-------------------------------------------------------------------------------------
bool Cellapp::installPyModules()
{
	Entity::installScript(getScript().getModule());
	Space::installScript(getScript().getModule());
	EntityComponent::installScript(getScript().getModule());
	GlobalDataClient::installScript(getScript().getModule());

	registerScript(Entity::getScriptType());
	registerScript(EntityComponent::getScriptType());

	// ��app���ע�ᵽ�ű�
	std::map<uint32, std::string> flagsmaps = createAppFlagsMaps();
	std::map<uint32, std::string>::iterator fiter = flagsmaps.begin();
	for (; fiter != flagsmaps.end(); ++fiter)
	{
		if (PyModule_AddIntConstant(getScript().getModule(), fiter->second.c_str(), fiter->first))
		{
			ERROR_MSG(fmt::format("Cellapp::onInstallPyModules: Unable to set KBEngine.{}.\n", fiter->second));
		}
	}

	// ע�ᴴ��entity�ķ�����py
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(),		time,							__py_gametime,											METH_VARARGS,			0);
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(),		createEntity,					__py_createEntity,										METH_VARARGS,			0);
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(), 		executeRawDatabaseCommand,		__py_executeRawDatabaseCommand,							METH_VARARGS,			0);
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(),		reloadScript,					__py_reloadScript,										METH_VARARGS,			0);
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(),		registerReadFileDescriptor,		PyFileDescriptor::__py_registerReadFileDescriptor,		METH_VARARGS,			0);
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(),		registerWriteFileDescriptor,	PyFileDescriptor::__py_registerWriteFileDescriptor,		METH_VARARGS,			0);
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(),		deregisterReadFileDescriptor,	PyFileDescriptor::__py_deregisterReadFileDescriptor,	METH_VARARGS,			0);
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(),		deregisterWriteFileDescriptor,	PyFileDescriptor::__py_deregisterWriteFileDescriptor,	METH_VARARGS,			0);
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(),		addSpaceGeometryMapping,		SpaceMemory::__py_AddSpaceGeometryMapping,				METH_VARARGS,			0);
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(),		getSpaceGeometryMapping,		SpaceMemory::__py_GetSpaceGeometryMapping,				METH_VARARGS,			0);
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(),		setSpaceData,					SpaceMemory::__py_SetSpaceData,							METH_VARARGS,			0);
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(),		getSpaceData,					SpaceMemory::__py_GetSpaceData,							METH_VARARGS,			0);
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(),		delSpaceData,					SpaceMemory::__py_DelSpaceData,							METH_VARARGS,			0);
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(),		isShuttingDown,					__py_isShuttingDown,									METH_VARARGS,			0);
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(),		address,						__py_address,											METH_VARARGS,			0);
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(),		raycast,						__py_raycast,											METH_VARARGS,			0);
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(), 		setAppFlags,					__py_setFlags,											METH_VARARGS,			0);
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(), 		getAppFlags,					__py_getFlags,											METH_VARARGS,			0);
	
	return EntityApp<Entity>::installPyModules();
}

//-------------------------------------------------------------------------------------
void Cellapp::onInstallPyModules()
{
	// ���globalData, cellAppData֧��
	pCellAppData_ = new GlobalDataClient(DBMGR_TYPE, GlobalDataServer::CELLAPP_DATA);
	registerPyObjectToScript("cellAppData", pCellAppData_);
}

//-------------------------------------------------------------------------------------
bool Cellapp::uninstallPyModules()
{
	if(g_kbeSrvConfig.getCellApp().profiles.open_pyprofile)
	{
		script::PyProfile::stop("kbengine");

		char buf[MAX_BUF];
		kbe_snprintf(buf, MAX_BUF, "cellapp%u.pyprofile", startGroupOrder_);
		script::PyProfile::dump("kbengine", buf);
		script::PyProfile::remove("kbengine");
	}

	unregisterPyObjectToScript("cellAppData");
	S_RELEASE(pCellAppData_); 

	Entity::uninstallScript();
	EntityComponent::uninstallScript();
	GlobalDataClient::uninstallScript();
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

	// һ��Ҫ����ǰ��
	updateLoad();

	EntityApp<Entity>::handleGameTick();

	updatables_.update();
	SpaceMemorys::update();
}

//-------------------------------------------------------------------------------------
bool Cellapp::initializeBegin()
{
	return true;
}

//-------------------------------------------------------------------------------------
bool Cellapp::initializeEnd()
{
	// �����Ҫpyprofile���ڴ˴���װ
	// ����ʱж�ز�������
	if(g_kbeSrvConfig.getCellApp().profiles.open_pyprofile)
	{
		script::PyProfile::start("kbengine");
	}

	pWitnessedTimeoutHandler_ = new WitnessedTimeoutHandler();

	// �Ƿ����Y��
	CoordinateSystem::hasY = g_kbeSrvConfig.getCellApp().coordinateSystem_hasY;

	dispatcher_.clearSpareTime();

	pGhostManager_ = new GhostManager();

	pTelnetServer_ = new TelnetServer(&this->dispatcher(), &this->networkInterface());
	pTelnetServer_->pScript(&this->getScript());

	bool ret = pTelnetServer_->start(g_kbeSrvConfig.getCellApp().telnet_passwd, 
		g_kbeSrvConfig.getCellApp().telnet_deflayer, 
		g_kbeSrvConfig.getCellApp().telnet_port);

	Components::getSingleton().extraData4(pTelnetServer_->port());
	return ret;
}

//-------------------------------------------------------------------------------------
void Cellapp::finalise()
{
	spaceViewers_.finalise();

	SAFE_RELEASE(pGhostManager_);
	SAFE_RELEASE(pWitnessedTimeoutHandler_);

	if(pTelnetServer_)
	{
		pTelnetServer_->stop();
		SAFE_RELEASE(pTelnetServer_);
	}

	SpaceMemorys::finalise();
	Navigation::getSingleton().finalise();
	forward_messagebuffer_.clear();
	updatables_.clear();

	destroyObjPool();
	EntityApp<Entity>::finalise();
}

//-------------------------------------------------------------------------------------
void Cellapp::destroyObjPool()
{
	EntityRef::destroyObjPool();
	Witness::destroyObjPool();
}

//-------------------------------------------------------------------------------------
void Cellapp::onGetEntityAppFromDbmgr(Network::Channel* pChannel, int32 uid, std::string& username, 
						COMPONENT_TYPE componentType, COMPONENT_ID componentID, COMPONENT_ORDER globalorderID, COMPONENT_ORDER grouporderID,
						uint32 intaddr, uint16 intport, uint32 extaddr, uint16 extport, std::string& extaddrEx)
{
	Components::ComponentInfos* cinfos = Components::getSingleton().findComponent((
		KBEngine::COMPONENT_TYPE)componentType, uid, componentID);

	if(cinfos)
	{
		if(cinfos->pIntAddr->ip != intaddr || cinfos->pIntAddr->port != intport)
		{
			ERROR_MSG(fmt::format("Cellapp::onGetEntityAppFromDbmgr: Illegal app(uid:{0}, username:{1}, componentType:{2}, "
					"componentID:{3}, globalorderID={9}, grouporderID={10}, intaddr:{4}, intport:{5}, extaddr:{6}, extport:{7},  from {8})\n",
					uid, 
					username,
					COMPONENT_NAME_EX((COMPONENT_TYPE)componentType), 
					componentID,
					inet_ntoa((struct in_addr&)intaddr),
					ntohs(intport),
					(extaddr != 0 ? inet_ntoa((struct in_addr&)extaddr) : "nonsupport"),
					ntohs(extport),
					pChannel->c_str(),
					((int32)globalorderID), 
					((int32)grouporderID)));

			Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);
			(*pBundle).newMessage(DbmgrInterface::reqKillServer);
			(*pBundle) << g_componentID << g_componentType << KBEngine::getUsername() << KBEngine::getUserUID() << "Duplicate app-id.";
			pChannel->send(pBundle);
		}
	}

	EntityApp<Entity>::onRegisterNewApp(pChannel, uid, username, componentType, componentID, globalorderID, grouporderID,
									intaddr, intport, extaddr, extport, extaddrEx);

	KBEngine::COMPONENT_TYPE tcomponentType = (KBEngine::COMPONENT_TYPE)componentType;

	Components::COMPONENTS& cts = Components::getSingleton().getComponents(DBMGR_TYPE);
	KBE_ASSERT(cts.size() >= 1);
	
	cinfos = Components::getSingleton().findComponent(tcomponentType, uid, componentID);
	
	if (cinfos == NULL)
	{
		ERROR_MSG(fmt::format("Cellapp::onGetEntityAppFromDbmgr: Illegal app(uid:{0}, username:{1}, componentType:{2}, "
				"componentID:{3}, globalorderID={9}, grouporderID={10}, intaddr:{4}, intport:{5}, extaddr:{6}, extport:{7},  from {8})\n",
				uid, 
				username,
				COMPONENT_NAME_EX((COMPONENT_TYPE)componentType), 
				componentID,
				inet_ntoa((struct in_addr&)intaddr),
				ntohs(intport),
				(extaddr != 0 ? inet_ntoa((struct in_addr&)extaddr) : "nonsupport"),
				ntohs(extport),
				pChannel->c_str(),
				((int32)globalorderID), 
				((int32)grouporderID)));

		return;
	}
	
	cinfos->pChannel = NULL;

	int ret = Components::getSingleton().connectComponent(tcomponentType, uid, componentID);

	if (ret == -1)
	{
		if (!pInitProgressHandler_)
			pInitProgressHandler_ = new InitProgressHandler(this->networkInterface());

		pInitProgressHandler_->updateInfos(componentID_, startGlobalOrder_, startGroupOrder_);

		InitProgressHandler::PendingConnectEntityApp appInfos;
		appInfos.componentID = componentID;
		appInfos.componentType = tcomponentType;
		appInfos.uid = uid;
		appInfos.count = 0;
		pInitProgressHandler_->addPendingConnectEntityApps(appInfos);

		ERROR_MSG(fmt::format("Cellapp::onGetEntityAppFromDbmgr: Add to the pending list and try connecting later! uid:{}, componentType:{}, componentID:{}\n",
			uid,
			COMPONENT_NAME_EX((COMPONENT_TYPE)tcomponentType),
			componentID));

		return;
	}

	Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);

	switch(tcomponentType)
	{
	case BASEAPP_TYPE:
		(*pBundle).newMessage(BaseappInterface::onRegisterNewApp);
		BaseappInterface::onRegisterNewAppArgs11::staticAddToBundle((*pBundle), getUserUID(), getUsername(), 
			CELLAPP_TYPE, componentID_, startGlobalOrder_, startGroupOrder_,
			this->networkInterface().intTcpAddr().ip, this->networkInterface().intTcpAddr().port,
			this->networkInterface().extTcpAddr().ip, this->networkInterface().extTcpAddr().port, g_kbeSrvConfig.getConfig().externalAddress);
		break;
	case CELLAPP_TYPE:
		(*pBundle).newMessage(CellappInterface::onRegisterNewApp);
		CellappInterface::onRegisterNewAppArgs11::staticAddToBundle((*pBundle), getUserUID(), getUsername(), 
			CELLAPP_TYPE, componentID_, startGlobalOrder_, startGroupOrder_,
			this->networkInterface().intTcpAddr().ip, this->networkInterface().intTcpAddr().port,
			this->networkInterface().extTcpAddr().ip, this->networkInterface().extTcpAddr().port, g_kbeSrvConfig.getConfig().externalAddress);
		break;
	default:
		KBE_ASSERT(false && "no support!\n");
		break;
	};
	
	cinfos->pChannel->send(pBundle);
}

//-------------------------------------------------------------------------------------
void Cellapp::onUpdateLoad()
{
	Network::Channel* pChannel = Components::getSingleton().getCellappmgrChannel();
	if(pChannel != NULL)
	{
		Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);
		(*pBundle).newMessage(CellappmgrInterface::updateCellapp);
		CellappmgrInterface::updateCellappArgs4::staticAddToBundle((*pBundle), 
			componentID_, (ENTITY_ID)pEntities_->getEntities().size(), getLoad(), flags_);

		pChannel->send(pBundle);
	}
}

//-------------------------------------------------------------------------------------
Entity* Cellapp::onCreateEntity(PyObject* pyEntity, ScriptDefModule* sm, ENTITY_ID eid)
{
	if (PyType_IsSubtype(sm->getScriptType(), Space::getScriptType()))
	{
		return new(pyEntity) Space(eid, sm);
	}

	return EntityApp<Entity>::onCreateEntity(pyEntity, sm, eid);
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
			"KBEngine::createEntity: args error! args[scriptName, spaceID, position, direction, states].");
		PyErr_PrintEx(0);
		return 0;
	}
	
	if(entityType == NULL || strlen(entityType) == 0)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::createEntity: entityType is NULL.");
		PyErr_PrintEx(0);
		return 0;
	}

	SpaceMemory* space = SpaceMemorys::findSpace(spaceID);
	if(space == NULL || !space->isGood())
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::createEntity: spaceID %ld not found.", spaceID);
		PyErr_PrintEx(0);
		return 0;
	}
	
	if(Cellapp::getSingleton().isShuttingdown())
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::createEntity: shutting down! entityType=%s", entityType);
		PyErr_PrintEx(0);
		return 0;
	}
	
	// ����entity
	Entity* pEntity = Cellapp::getSingleton().createEntity(entityType, params, false, 0);

	if(pEntity != NULL)
	{
		Py_INCREF(pEntity);
		pEntity->spaceID(space->id());
		pEntity->createNamespace(params);
		pEntity->pySetPosition(position);
		pEntity->pySetDirection(direction);	
		pEntity->initializeScript();

		// ��ӵ�space
		space->addEntityAndEnterWorld(pEntity);

		// �п�����addEntityAndEnterWorld�б�������
		// ������Ҫ��ʵ�巵�ظ��ű���ֻ����ʵ��ΪisDestroyed = true״̬
		//if(pEntity->isDestroyed())
		//{
		//	Py_DECREF(pEntity);
		//	return NULL;
		//}
	}

	//Py_XDECREF(params);
	return pEntity;
}

//-------------------------------------------------------------------------------------
PyObject* Cellapp::__py_executeRawDatabaseCommand(PyObject* self, PyObject* args)
{
	int argCount = (int)PyTuple_Size(args);
	PyObject* pycallback = NULL;
	PyObject* pyDBInterfaceName = NULL;
	int ret = 0;
	ENTITY_ID eid = -1;

	char* data = NULL;
	Py_ssize_t size;
	
	if (argCount == 4)
		ret = PyArg_ParseTuple(args, "s#|O|i|O", &data, &size, &pycallback, &eid, &pyDBInterfaceName);
	else if (argCount == 3)
		ret = PyArg_ParseTuple(args, "s#|O|i", &data, &size, &pycallback, &eid);
	else if(argCount == 2)
		ret = PyArg_ParseTuple(args, "s#|O", &data, &size, &pycallback);
	else if(argCount == 1)
		ret = PyArg_ParseTuple(args, "s#", &data, &size);

	if(!ret)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::executeRawDatabaseCommand: args error!");
		PyErr_PrintEx(0);
		S_Return;
	}
	
	std::string dbInterfaceName = "default";
	if (pyDBInterfaceName)
	{
		dbInterfaceName = PyUnicode_AsUTF8AndSize(pyDBInterfaceName, NULL);
		
		if (!g_kbeSrvConfig.dbInterface(dbInterfaceName))
		{
			PyErr_Format(PyExc_TypeError, "KBEngine::executeRawDatabaseCommand: args4, incorrect dbInterfaceName(%s)!", 
				dbInterfaceName.c_str());
			
			PyErr_PrintEx(0);
			S_Return;
		}
	}

	Cellapp::getSingleton().executeRawDatabaseCommand(data, (uint32)size, pycallback, eid, dbInterfaceName);
	S_Return;
}

//-------------------------------------------------------------------------------------
void Cellapp::executeRawDatabaseCommand(const char* datas, uint32 size, PyObject* pycallback, ENTITY_ID eid, const std::string& dbInterfaceName)
{
	if(datas == NULL)
	{
		ERROR_MSG("KBEngine::executeRawDatabaseCommand: execute error!\n");
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

	int dbInterfaceIndex = g_kbeSrvConfig.dbInterfaceName2dbInterfaceIndex(dbInterfaceName);
	if (dbInterfaceIndex < 0)
	{
		ERROR_MSG(fmt::format("KBEngine::executeRawDatabaseCommand: not found dbInterface({})!\n",
			dbInterfaceName));

		return;
	}

	//INFO_MSG(fmt::format("KBEngine::executeRawDatabaseCommand{}:{}.\n", (eid > 0 ? fmt::format("(entityID={})", eid) : ""), datas));

	Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);
	(*pBundle).newMessage(DbmgrInterface::executeRawDatabaseCommand);
	(*pBundle) << eid;
	(*pBundle) << (uint16)dbInterfaceIndex;
	(*pBundle) << componentID_ << componentType_;

	CALLBACK_ID callbackID = 0;

	if(pycallback && PyCallable_Check(pycallback))
		callbackID = callbackMgr().save(pycallback);

	(*pBundle) << callbackID;
	(*pBundle) << size;
	(*pBundle).append(datas, size);
	dbmgrinfos->pChannel->send(pBundle);
}

//-------------------------------------------------------------------------------------
void Cellapp::onExecuteRawDatabaseCommandCB(Network::Channel* pChannel, KBEngine::MemoryStream& s)
{
	std::string err;
	CALLBACK_ID callbackID = 0;
	uint32 nrows = 0;
	uint32 nfields = 0;
	uint64 affectedRows = 0;
	uint64 lastInsertID = 0;

	PyObject* pResultSet = NULL;
	PyObject* pAffectedRows = NULL;
	PyObject* pLastInsertID = NULL;
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

			pLastInsertID = Py_None;
			Py_INCREF(pLastInsertID);

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
						
					if(cell == "KBE_QUERY_DB_NULL")
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

			s >> lastInsertID;
			pLastInsertID = PyLong_FromUnsignedLongLong(lastInsertID);
		}
	}
	else
	{
			pResultSet = Py_None;
			Py_INCREF(pResultSet);

			pErrorMsg = PyUnicode_FromString(err.c_str());

			pAffectedRows = Py_None;
			Py_INCREF(pAffectedRows);

			pLastInsertID = Py_None;
			Py_INCREF(pLastInsertID);
	}

	s.done();

	//DEBUG_MSG(fmt::format("Cellapp::onExecuteRawDatabaseCommandCB: nrows={}, nfields={}, err={}.\n", 
	//	nrows, nfields, err.c_str()));

	if(callbackID > 0)
	{
		SCOPED_PROFILE(SCRIPTCALL_PROFILE);

		PyObjectPtr pyfunc = pyCallbackMgr_.take(callbackID);
		if(pyfunc != NULL)
		{
			PyObject* pyResult = PyObject_CallFunction(pyfunc.get(), 
												const_cast<char*>("OOOO"), 
												pResultSet, pAffectedRows, pLastInsertID, pErrorMsg);

			if(pyResult != NULL)
				Py_DECREF(pyResult);
			else
				SCRIPT_ERROR_CHECK();
		}
		else
		{
			ERROR_MSG(fmt::format("Cellapp::onExecuteRawDatabaseCommandCB: not found callback:{}.\n",
				callbackID));
		}
	}

	Py_XDECREF(pResultSet);
	Py_XDECREF(pAffectedRows);
	Py_XDECREF(pLastInsertID);
	Py_XDECREF(pErrorMsg);
}

//-------------------------------------------------------------------------------------
void Cellapp::reqBackupEntityCellData(Network::Channel* pChannel, KBEngine::MemoryStream& s)
{
	ENTITY_ID entityID = 0;
	s >> entityID;

	Entity* e = this->findEntity(entityID);
	if(!e)
	{
		WARNING_MSG(fmt::format("Cellapp::reqBackupEntityCellData: not found entity {}.\n", entityID));
		return;
	}

	e->backupCellData();
}

//-------------------------------------------------------------------------------------
void Cellapp::reqWriteToDBFromBaseapp(Network::Channel* pChannel, KBEngine::MemoryStream& s)
{
	ENTITY_ID entityID = 0;
	CALLBACK_ID callbackID = 0;
	int8 shouldAutoLoad = -1;

	s >> entityID;
	s >> callbackID;
	s >> shouldAutoLoad;

	Entity* e = this->findEntity(entityID);
	if(!e)
	{
		WARNING_MSG(fmt::format("Cellapp::reqWriteToDBFromBaseapp: not found entity {}.\n", entityID));
		return;
	}

	e->writeToDB(&callbackID, &shouldAutoLoad, NULL);
}

//-------------------------------------------------------------------------------------
void Cellapp::onDbmgrInitCompleted(Network::Channel* pChannel, 
		GAME_TIME gametime, ENTITY_ID startID, ENTITY_ID endID, COMPONENT_ORDER 
		startGlobalOrder, COMPONENT_ORDER startGroupOrder, const std::string& digest)
{
	EntityApp<Entity>::onDbmgrInitCompleted(pChannel, gametime, startID, endID, startGlobalOrder, startGroupOrder, digest);
	
	// �ٴ�ͬ���Լ�������Ϣ(startGlobalOrder, startGroupOrder��)��machine
	Components::getSingleton().broadcastSelf();

	// ������Ҫ����һ��python�Ļ�������
	this->getScript().setenv("KBE_BOOTIDX_GLOBAL", getenv("KBE_BOOTIDX_GLOBAL"));
	this->getScript().setenv("KBE_BOOTIDX_GROUP", getenv("KBE_BOOTIDX_GROUP"));

	if (!pInitProgressHandler_)
		pInitProgressHandler_ = new InitProgressHandler(this->networkInterface());

	pInitProgressHandler_->start();
}

//-------------------------------------------------------------------------------------
void Cellapp::onBroadcastCellAppDataChanged(Network::Channel* pChannel, KBEngine::MemoryStream& s)
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

	if(isDelete)
	{
		if(pCellAppData_->del(pyKey))
		{
			SCOPED_PROFILE(SCRIPTCALL_PROFILE);

			// ֪ͨ�ű�
			SCRIPT_OBJECT_CALL_ARGS1(getEntryScript().get(), const_cast<char*>("onCellAppDataDel"), 
				const_cast<char*>("O"), pyKey, false);
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

		if(pCellAppData_->write(pyKey, pyValue))
		{
			SCOPED_PROFILE(SCRIPTCALL_PROFILE);

			// ֪ͨ�ű�
			SCRIPT_OBJECT_CALL_ARGS2(getEntryScript().get(), const_cast<char*>("onCellAppData"), 
				const_cast<char*>("OO"), pyKey, pyValue, false);
		}

		Py_DECREF(pyValue);
	}

	Py_DECREF(pyKey);
}

//-------------------------------------------------------------------------------------
void Cellapp::onCreateCellEntityInNewSpaceFromBaseapp(Network::Channel* pChannel, KBEngine::MemoryStream& s)
{
	std::string entityType;
	ENTITY_ID entitycallEntityID;
	COMPONENT_ID componentID;
	SPACE_ID spaceID = 1;
	bool hasClient;

	s >> entityType;
	s >> entitycallEntityID;
	s >> spaceID;
	s >> componentID;
	s >> hasClient;

	// DEBUG_MSG("Cellapp::onCreateCellEntityInNewSpaceFromBaseapp: spaceID=%u, entityType=%s, entityID=%d, componentID=%"PRAppID".\n", 
	//	spaceID, entityType.c_str(), entitycallEntityID, componentID);

	SpaceMemory* space = SpaceMemorys::createNewSpace(spaceID, entityType);
	if(space != NULL)
	{
		// ����entity
		Entity* e = createEntity(entityType.c_str(), NULL, false, entitycallEntityID, false);
		
		if(e == NULL)
		{
			s.done();

			ERROR_MSG("Cellapp::onCreateCellEntityInNewSpaceFromBaseapp: createEntity error!\n");

			/* Ŀǰ��˵�����ڴ����ϵͳ���⣬���򲻻�����������
			Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);
			pBundle->newMessage(BaseappInterface::onCreateCellFailure);
			BaseappInterface::onCreateCellFailureArgs1::staticAddToBundle(*pBundle, entitycallEntityID);
			cinfos->pChannel->send(pBundle);
			*/
			return;
		}

		PyObject* cellData = e->createCellDataFromStream(&s);

		// ����entity��baseEntityCall
		EntityCall* entityCall = new EntityCall(e->pScriptModule(), NULL, componentID, entitycallEntityID, ENTITYCALL_TYPE_BASE);
		e->baseEntityCall(entityCall);
		
		if (hasClient)
		{
			KBE_ASSERT(e->baseEntityCall() != NULL && !e->hasWitness());
			PyObject* clientEntityCall = PyObject_GetAttrString(e->baseEntityCall(), "client");
			KBE_ASSERT(clientEntityCall != Py_None);

			EntityCall* client = static_cast<EntityCall*>(clientEntityCall);
			// Py_INCREF(clientEntityCall); ���ﲻ��Ҫ�������ã� ��Ϊÿ�ζ������һ���µĶ���

			// Ϊ���ܹ���entity.__init__���ܹ��޸����������ܹ㲥���ͻ���������Ҫ��ǰ������Щ
			e->clientEntityCall(client);
			e->setWitness(Witness::createPoolObject(OBJECTPOOL_POINT));
		}

		// �˴�baseapp���ܻ���û��ʼ�������� ������һ��������ΪNone��
		Components::ComponentInfos* cinfos = Components::getSingleton().findComponent(BASEAPP_TYPE, componentID);
		if(cinfos == NULL || cinfos->pChannel == NULL)
		{
			Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);
			ForwardItem* pFI = new ForwardItem();
			pFI->pHandler = new FMH_Baseapp_onEntityGetCellFrom_onCreateCellEntityInNewSpaceFromBaseapp(e, spaceID, cellData);
			//Py_XDECREF(cellData);
			pFI->pBundle = pBundle;
			(*pBundle).newMessage(BaseappInterface::onEntityGetCell);
			BaseappInterface::onEntityGetCellArgs3::staticAddToBundle((*pBundle), entitycallEntityID, componentID_, spaceID);
			forward_messagebuffer_.push(componentID, pFI);
			
			WARNING_MSG(fmt::format("Cellapp::onCreateCellEntityInNewSpaceFromBaseapp: not found baseapp({}), message is buffered.\n",
				componentID));
			
			return;
		}

		KBE_SHA1 sha;
		uint32 digest[5];
		sha.Input(s.data(), s.length());
		sha.Result(digest);
		e->setDirty((uint32*)&digest[0]);

		space->addEntity(e);
		e->spaceID(space->id());
		e->initializeEntity(cellData, true);
		Py_XDECREF(cellData);

		// ��ӵ�space
		space->addEntityToNode(e);

		if (hasClient)
		{
			e->onGetWitness();
		}
		else
		{
			space->onEnterWorld(e);
		}

		Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);
		(*pBundle).newMessage(BaseappInterface::onEntityGetCell);
		BaseappInterface::onEntityGetCellArgs3::staticAddToBundle((*pBundle), entitycallEntityID, componentID_, spaceID);
		cinfos->pChannel->send(pBundle);

		return;
	}
	
	ERROR_MSG(fmt::format("Cellapp::onCreateCellEntityInNewSpaceFromBaseapp: not found baseapp[{}], entityID={}, spaceID={}.\n",
		componentID, entitycallEntityID, spaceID));
}

//-------------------------------------------------------------------------------------
void Cellapp::onRestoreSpaceInCellFromBaseapp(Network::Channel* pChannel, KBEngine::MemoryStream& s)
{
	std::string entityType;
	ENTITY_ID entitycallEntityID;
	COMPONENT_ID componentID;
	SPACE_ID spaceID = 1;
	bool hasClient;

	s >> entityType;
	s >> entitycallEntityID;
	s >> spaceID;
	s >> componentID;
	s >> hasClient;

	// DEBUG_MSG("Cellapp::onRestoreSpaceInCellFromBaseapp: spaceID=%u, entityType=%s, entityID=%d, componentID=%"PRAppID".\n", 
	//	spaceID, entityType.c_str(), entitycallEntityID, componentID);

	SpaceMemory* space = SpaceMemorys::createNewSpace(spaceID, entityType);
	if(space != NULL)
	{
		// ����entity
		Entity* e = createEntity(entityType.c_str(), NULL, false, entitycallEntityID, false);
		
		if(e == NULL)
		{
			s.done();
			return;
		}

		PyObject* cellData = e->createCellDataFromStream(&s);

		// ����entity��baseEntityCall
		EntityCall* entityCall = new EntityCall(e->pScriptModule(), NULL, componentID, entitycallEntityID, ENTITYCALL_TYPE_BASE);
		e->baseEntityCall(entityCall);
		
		if (hasClient)
		{
			KBE_ASSERT(e->baseEntityCall() != NULL && !e->hasWitness());
			PyObject* clientEntityCall = PyObject_GetAttrString(e->baseEntityCall(), "client");
			KBE_ASSERT(clientEntityCall != Py_None);

			EntityCall* client = static_cast<EntityCall*>(clientEntityCall);
			// Py_INCREF(clientEntityCall); ���ﲻ��Ҫ�������ã� ��Ϊÿ�ζ������һ���µĶ���

			// Ϊ���ܹ���entity.__init__���ܹ��޸����������ܹ㲥���ͻ���������Ҫ��ǰ������Щ
			e->clientEntityCall(client);
			e->setWitness(Witness::createPoolObject(OBJECTPOOL_POINT));
		}

		// �˴�baseapp���ܻ���û��ʼ�������� ������һ��������ΪNone��
		Components::ComponentInfos* cinfos = Components::getSingleton().findComponent(BASEAPP_TYPE, componentID);
		if(cinfos == NULL || cinfos->pChannel == NULL)
		{
			Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);
			ForwardItem* pFI = new ForwardItem();
			pFI->pHandler = new FMH_Baseapp_onEntityGetCellFrom_onCreateCellEntityInNewSpaceFromBaseapp(e, spaceID, cellData);
			//Py_XDECREF(cellData);
			pFI->pBundle = pBundle;
			(*pBundle).newMessage(BaseappInterface::onEntityGetCell);
			BaseappInterface::onEntityGetCellArgs3::staticAddToBundle((*pBundle), entitycallEntityID, componentID_, spaceID);
			forward_messagebuffer_.push(componentID, pFI);
			
			WARNING_MSG(fmt::format("Cellapp::onRestoreSpaceInCellFromBaseapp: not found baseapp({}), message has been buffered.\n",
				componentID));
			
			return;
		}
		
		KBE_SHA1 sha;
		uint32 digest[5];
		sha.Input(s.data(), s.length());
		sha.Result(digest);
		e->setDirty((uint32*)&digest[0]);

		e->spaceID(space->id());
		e->createNamespace(cellData);
		Py_XDECREF(cellData);

		// ��ӵ�space
		e->onRestore();

		space->addEntityAndEnterWorld(e, true);

		Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);
		(*pBundle).newMessage(BaseappInterface::onEntityGetCell);
		BaseappInterface::onEntityGetCellArgs3::staticAddToBundle((*pBundle), entitycallEntityID, componentID_, spaceID);
		cinfos->pChannel->send(pBundle);
		return;
	}
	
	ERROR_MSG(fmt::format("Cellapp::onRestoreSpaceInCellFromBaseapp: not found baseapp[{}], entityID={}, spaceID={}.\n",
		componentID, entitycallEntityID, spaceID));
}

//-------------------------------------------------------------------------------------
void Cellapp::requestRestore(Network::Channel* pChannel, KBEngine::MemoryStream& s)
{
	COMPONENT_ID cid;
	s >> cid;

	bool canRestore = idClient_.size() > 0;

	DEBUG_MSG(fmt::format("Cellapp::requestRestore: cid={}, canRestore={}, channel={}.\n", 
		cid, canRestore, pChannel->c_str()));

	Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);
	(*pBundle).newMessage(BaseappInterface::onRequestRestoreCB);
	(*pBundle) << g_componentID << cid << canRestore;
	pChannel->send(pBundle);
}

//-------------------------------------------------------------------------------------
void Cellapp::onCreateCellEntityFromBaseapp(Network::Channel* pChannel, KBEngine::MemoryStream& s)
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

	// �˴�baseapp���ܻ���û��ʼ�������� ������һ��������ΪNone��
	Components::ComponentInfos* cinfos = Components::getSingleton().findComponent(BASEAPP_TYPE, componentID);
	if(cinfos == NULL || cinfos->pChannel == NULL)
	{
		MemoryStream* pCellData = MemoryStream::createPoolObject(OBJECTPOOL_POINT);
		pCellData->append(s);

		Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);
		ForwardItem* pFI = new ForwardItem();
		pFI->pHandler = new FMH_Baseapp_onEntityGetCellFrom_onCreateCellEntityFromBaseapp(entityType, createToEntityID, 
			entityID, pCellData, hasClient, inRescore, componentID, spaceID);

		pFI->pBundle = pBundle;
		(*pBundle).newMessage(BaseappInterface::onEntityGetCell);
		BaseappInterface::onEntityGetCellArgs3::staticAddToBundle((*pBundle), entityID, componentID_, spaceID);
		forward_messagebuffer_.push(componentID, pFI);

		WARNING_MSG(fmt::format("Cellapp::onCreateCellEntityFromBaseapp: not found baseapp({}), message is buffered.\n",
			componentID));
			
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
	// ע�⣺�˴����۲����Ҳ�������� ��ΪonCreateCellEntityFromBaseapp���Ѿ����й�һ����Ϣ�����ж�
	Components::ComponentInfos* cinfos = Components::getSingleton().findComponent(BASEAPP_TYPE, componentID);
	KBE_ASSERT(cinfos != NULL && cinfos->pChannel != NULL);

	Entity* pCreateToEntity = pEntities_->find(createToEntityID);

	// ����spaceEntity�Ѿ������ˣ� ����δ���ü�֪ͨ��baseappʱ
	// base�����������space����entity
	if(pCreateToEntity == NULL)
	{
		ERROR_MSG("Cellapp::_onCreateCellEntityFromBaseapp: not fount spaceEntity. may have been destroyed!\n");

		Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);
		pBundle->newMessage(BaseappInterface::onCreateCellFailure);
		BaseappInterface::onCreateCellFailureArgs1::staticAddToBundle(*pBundle, entityID);
		cinfos->pChannel->send(pBundle);
		return;
	}

	spaceID = pCreateToEntity->spaceID();

	//DEBUG_MSG("Cellapp::onCreateCellEntityFromBaseapp: spaceID=%u, entityType=%s, entityID=%d, componentID=%"PRAppID".\n", 
	//	spaceID, entityType.c_str(), entityID, componentID);

	SpaceMemory* space = SpaceMemorys::findSpace(spaceID);
	if(space != NULL && space->isGood())
	{
		// ��֪baseapp�� entity��cell������
		Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);
		pBundle->newMessage(BaseappInterface::onEntityGetCell);
		BaseappInterface::onEntityGetCellArgs3::staticAddToBundle(*pBundle, entityID, componentID_, spaceID);
		cinfos->pChannel->send(pBundle);

		// ���cellData��Ϣ.
		PyObject* cellData = NULL;
	
		// ����entity
		Entity* e = createEntity(entityType.c_str(), cellData, false, entityID, false);
		
		if(e == NULL)
		{
			Py_XDECREF(cellData);
			return;
		}

		// ����entity��baseEntityCall
		EntityCall* entityCall = new EntityCall(e->pScriptModule(), NULL, componentID, entityID, ENTITYCALL_TYPE_BASE);
		e->baseEntityCall(entityCall);
		
		KBE_SHA1 sha;
		uint32 digest[5];
		sha.Input(pCellData->data(), pCellData->length());
		sha.Result(digest);
		e->setDirty((uint32*)&digest[0]);

		cellData = e->createCellDataFromStream(pCellData);

		e->createNamespace(cellData, true);

		if(hasClient)
		{
			KBE_ASSERT(e->baseEntityCall() != NULL && !e->hasWitness());
			PyObject* clientEntityCall = PyObject_GetAttrString(e->baseEntityCall(), "client");
			KBE_ASSERT(clientEntityCall != Py_None);

			EntityCall* client = static_cast<EntityCall*>(clientEntityCall);	
			// Py_INCREF(clientEntityCall); ���ﲻ��Ҫ�������ã� ��Ϊÿ�ζ������һ���µĶ���

			// Ϊ���ܹ���entity.__init__���ܹ��޸����������ܹ㲥���ͻ���������Ҫ��ǰ������Щ
			e->clientEntityCall(client);
			e->setWitness(Witness::createPoolObject(OBJECTPOOL_POINT));
		}

		space->addEntity(e);

		if(!inRescore)
		{
			e->initializeScript();
		}
		else
		{
			e->onRestore();
		}

		Py_XDECREF(cellData);
		
		// ��������һ�����ã� ��Ϊ�����ڽ���ʱ������
		Py_INCREF(e);

		space->addEntityToNode(e);
		bool isDestroyed = e->isDestroyed();
		Py_DECREF(e);

		if(isDestroyed == true)
			return;

		// �������client��entity����������cliententitycall, baseapp���ֵ�onEntityGetCell���֪�ͻ���enterworld.
		if(hasClient)
		{
			e->onGetWitness();
		}
		else
		{
			space->onEnterWorld(e);
		}

		return;
	}

	KBE_ASSERT(false && "Cellapp::onCreateCellEntityFromBaseapp: error!\n");
}

//-------------------------------------------------------------------------------------
void Cellapp::onDestroyCellEntityFromBaseapp(Network::Channel* pChannel, ENTITY_ID eid)
{
	// DEBUG_MSG("Cellapp::onDestroyCellEntityFromBaseapp:entityID=%d.\n", eid);

	KBEngine::Entity* e = KBEngine::Cellapp::getSingleton().findEntity(eid);

	if (e == NULL)
	{
		WARNING_MSG(fmt::format("Cellapp::onDestroyCellEntityFromBaseapp: not found entityID:{}.\n",
			eid));

		return;
	}

	if (!e->isReal())
	{
		// ��Ҫ����ת
		GhostManager* gm = Cellapp::getSingleton().pGhostManager();
		if (gm)
		{
			Network::Bundle* pBundle = gm->createSendBundle(e->realCell());
			pBundle->newMessage(CellappInterface::onDestroyCellEntityFromBaseapp);
			(*pBundle) << eid;
			gm->pushMessage(e->realCell(), pBundle);
		}

		return;
	}

	destroyEntity(eid, true);
}

//-------------------------------------------------------------------------------------
RemoteEntityMethod* Cellapp::createEntityCallCallEntityRemoteMethod(MethodDescription* pMethodDescription, EntityCallAbstract* pEntityCall)
{
	return new EntityRemoteMethod(pMethodDescription, pEntityCall);
}

//-------------------------------------------------------------------------------------
void Cellapp::onEntityCall(Network::Channel* pChannel, KBEngine::MemoryStream& s)
{
	ENTITY_ID eid;
	s >> eid;

	ENTITYCALL_TYPE calltype;
	s >> calltype;

	// �ڱ��������Բ��Ҹ��ռ�����Ϣ�� ���ռ����Ƿ����ڱ�����
	Entity* entity = pEntities_->find(eid);
	if(entity == NULL)
	{
		if(calltype == ENTITYCALL_TYPE_CELL)
		{
			GhostManager* gm = Cellapp::getSingleton().pGhostManager();
			COMPONENT_ID cellID = gm ? gm->getRoute(eid) : 0;
			if(gm && cellID > 0)
			{
				Network::Bundle* pBundle = gm->createSendBundle(cellID);
				(*pBundle).newMessage(CellappInterface::onEntityCall);
				(*pBundle) << eid << calltype;
				(*pBundle).append(s);
				gm->pushMessage(cellID, pBundle);
				s.done();
				return;
			}
		}

		WARNING_MSG(fmt::format("Cellapp::onEntityCall: entityID {} not found.\n", eid));
		s.done();
		return;
	}

	switch(calltype)
	{
		// �������cellapp����ôȷ���ʼ���Ŀ�ĵ������ ��ôִ�����ղ���
		case ENTITYCALL_TYPE_CELL:	
			{
				if(!entity->isReal())
				{
					GhostManager* gm = Cellapp::getSingleton().pGhostManager();
					if(gm)
					{
						Network::Bundle* pBundle = gm->createSendBundle(entity->realCell());
						pBundle->newMessage(CellappInterface::onEntityCall);
						(*pBundle) << eid << calltype;
						pBundle->append(s);
						gm->pushMessage(entity->realCell(), pBundle);
					}
				}
				else
				{
					entity->onRemoteMethodCall(pChannel, s);
				}
			}

			break;

		// entity.base.cell.xxx
		case ENTITYCALL_TYPE_BASE_VIA_CELL: 
			{
				EntityCallAbstract* entityCall = static_cast<EntityCallAbstract*>(entity->baseEntityCall());
				if(entityCall == NULL)
				{
					//WARNING_MSG(fmt::format("Cellapp::onEntityCall: not found baseEntityCall! entitycallType={}, entityID={}.\n",
					//	calltype, eid));

					break;
				}
				
				Network::Channel* pChannel = entityCall->getChannel();
				if (pChannel)
				{
					Network::Bundle* pBundle = pChannel->createSendBundle();
					entityCall->newCall_(*pBundle);
					pBundle->append(s);
					pChannel->send(pBundle);
				}
			}
			break;
		
		// entity.cell.client
		case ENTITYCALL_TYPE_CLIENT_VIA_CELL: 
			{
				EntityCallAbstract* entityCall = static_cast<EntityCallAbstract*>(entity->clientEntityCall());
				if(entityCall == NULL)
				{
					//WARNING_MSG(fmt::format("Cellapp::onEntityCall: not found clientEntityCall! entitycallType={}, entityID={}.\n",
					//	calltype, eid));

					break;
				}
				
				Network::Channel* pChannel = entityCall->getChannel();
				if (pChannel)
				{
					Network::Bundle* pBundle = pChannel->createSendBundle();
					entityCall->newCall_(*pBundle);
					pBundle->append(s);
					pChannel->send(pBundle);
				}
			}
			break;
		default:
			{
				ERROR_MSG(fmt::format("Cellapp::onEntityCall: entitycallType {} error! must a cellType. entityID={}.\n",
					calltype, eid));
			}
	};

	s.done();
}

//-------------------------------------------------------------------------------------
void Cellapp::onRemoteCallMethodFromClient(Network::Channel* pChannel, KBEngine::MemoryStream& s)
{
	ENTITY_ID srcEntityID, targetID;

	s >> srcEntityID >> targetID;

	KBEngine::Entity* e = KBEngine::Cellapp::getSingleton().findEntity(targetID);		

	if(e == NULL)
	{	
		WARNING_MSG(fmt::format("Cellapp::onRemoteCallMethodFromClient: not found entityID:{}, srcEntityID:{}.\n", 
			targetID, srcEntityID));
		
		s.done();
		return;
	}

	// ���������������������proxy�Լ��ķ����������е�entity��proxy��cellEntity��һ��space�С�
	try
	{
		if (!e->isReal())
		{
			// ��Ҫ����ת
			GhostManager* gm = Cellapp::getSingleton().pGhostManager();
			if (gm)
			{
				Network::Bundle* pBundle = gm->createSendBundle(e->realCell());
				pBundle->newMessage(CellappInterface::onRemoteCallMethodFromClient);
				(*pBundle) << srcEntityID;
				(*pBundle) << targetID;
				pBundle->append(s);
				gm->pushMessage(e->realCell(), pBundle);
			}

			return;
		}

		e->onRemoteCallMethodFromClient(pChannel, srcEntityID, s);
	}catch(MemoryStreamException &)
	{
		ERROR_MSG(fmt::format("Cellapp::onRemoteCallMethodFromClient: message error! entityID:{}.\n", 
			targetID));

		s.done();
		return;
	}
}

//-------------------------------------------------------------------------------------
void Cellapp::onUpdateDataFromClient(Network::Channel* pChannel, KBEngine::MemoryStream& s)
{
	ENTITY_ID srcEntityID = 0;

	s >> srcEntityID;
	if(srcEntityID <= 0)
		return;
	
	if(s.length() <= 0)
		return;

	KBEngine::Entity* e = findEntity(srcEntityID);	

	if(e == NULL)
	{
		WARNING_MSG(fmt::format("Cellapp::onUpdateDataFromClient: not found entity {}!\n", srcEntityID));
		
		s.done();
		return;
	}

	// ����Ǳ�ϵͳ�����ˣ��ֻ򱻱��˿����ˣ�����������Լ��ͻ��˵ĸ�����Ϣ
	if (e->controlledBy() == NULL || e->controlledBy()->id() != srcEntityID)
	{
		// phw: �����Է��֣�����controlledBy�ı�ʱ֪ͨ�ͻ��˴���һ����ʱ��
		//      ���Կͻ����յ���Ϣǰ��Ȼ����λ����Ϣ����ʹ������Ĵ�����־����е�࣬
		//      ���ע�͵������־���Լ��ٲ���Ҫ����־�����
		//ERROR_MSG(fmt::format("Cellapp::onUpdateDataFromClientForControlledEntity: entity {} has no permission to control entity {}!\n", proxiesEntityID, srcEntityID));

		s.done();
		return;
	}

	e->onUpdateDataFromClient(s);
	s.done();
}

//-------------------------------------------------------------------------------------
void Cellapp::onUpdateDataFromClientForControlledEntity(Network::Channel* pChannel, KBEngine::MemoryStream& s)
{
	ENTITY_ID proxiesEntityID = 0;
	s >> proxiesEntityID;
	if(proxiesEntityID <= 0)
		return;

	if(s.length() <= 0)
		return;

	ENTITY_ID srcEntityID = 0;

	s >> srcEntityID;
	if(srcEntityID <= 0)
		return;
	
	if(s.length() <= 0)
		return;

	KBEngine::Entity* e = findEntity(srcEntityID);	

	if(e == NULL)
	{
		ERROR_MSG(fmt::format("Cellapp::onUpdateDataFromClientForControlledEntity: not found entity {}!\n", srcEntityID));
		
		s.done();
		return;
	}

	if (e->controlledBy() == NULL || e->controlledBy()->id() != proxiesEntityID)
	{
		// phw: �����Է��֣�����controlledBy�ı�ʱ֪ͨ�ͻ��˴���һ����ʱ��
		//      ���Կͻ����յ���Ϣǰ��Ȼ����λ����Ϣ����ʹ������Ĵ�����־����е�࣬
		//      ���ע�͵������־���Լ��ٲ���Ҫ����־�����
		//ERROR_MSG(fmt::format("Cellapp::onUpdateDataFromClientForControlledEntity: entity {} has no permission to control entity {}!\n", proxiesEntityID, srcEntityID));
		
		s.done();
		return;
	}

	e->onUpdateDataFromClient(s);
	s.done();
}

//-------------------------------------------------------------------------------------
void Cellapp::onUpdateGhostPropertys(Network::Channel* pChannel, KBEngine::MemoryStream& s)
{
	ENTITY_ID entityID;
	
	s >> entityID;

	Entity* entity = findEntity(entityID);
	if(entity == NULL)
	{
		GhostManager* gm = Cellapp::getSingleton().pGhostManager();
		if(gm)
		{
			COMPONENT_ID targetCell = gm->getRoute(entityID);
			if(targetCell > 0)
			{
				Network::Bundle* pForwardBundle = gm->createSendBundle(targetCell);
				(*pForwardBundle).newMessage(CellappInterface::onUpdateGhostPropertys);
				(*pForwardBundle) << entityID;
				pForwardBundle->append(s);

				gm->pushRouteMessage(entityID, targetCell, pForwardBundle);
				s.done();
				return;
			}
		}

		ERROR_MSG(fmt::format("Cellapp::onUpdateGhostPropertys: not found entity({})\n", 
			entityID));

		s.done();
		return;
	}

	entity->onUpdateGhostPropertys(s);
}

//-------------------------------------------------------------------------------------
void Cellapp::onRemoteRealMethodCall(Network::Channel* pChannel, KBEngine::MemoryStream& s)
{
	ENTITY_ID entityID;
	
	s >> entityID;

	Entity* entity = findEntity(entityID);
	if(entity == NULL)
	{
		GhostManager* gm = Cellapp::getSingleton().pGhostManager();
		if(gm)
		{
			COMPONENT_ID targetCell = gm->getRoute(entityID);
			if(targetCell > 0)
			{
				Network::Bundle* pForwardBundle = gm->createSendBundle(targetCell);
				(*pForwardBundle).newMessage(CellappInterface::onRemoteRealMethodCall);
				(*pForwardBundle) << entityID;
				pForwardBundle->append(s);

				gm->pushRouteMessage(entityID, targetCell, pForwardBundle);
				s.done();
				return;
			}
		}

		ERROR_MSG(fmt::format("Cellapp::onRemoteRealMethodCall: not found entity({})\n", 
			entityID));

		s.done();
		return;
	}

	entity->onRemoteRealMethodCall(s);
}

//-------------------------------------------------------------------------------------
void Cellapp::onUpdateGhostVolatileData(Network::Channel* pChannel, KBEngine::MemoryStream& s)
{
	ENTITY_ID entityID;
	
	s >> entityID;

	Entity* entity = findEntity(entityID);
	if(entity == NULL)
	{
		GhostManager* gm = Cellapp::getSingleton().pGhostManager();
		if(gm)
		{
			COMPONENT_ID targetCell = gm->getRoute(entityID);
			if(targetCell > 0)
			{
				Network::Bundle* pForwardBundle = gm->createSendBundle(targetCell);
				(*pForwardBundle).newMessage(CellappInterface::onUpdateGhostVolatileData);
				(*pForwardBundle) << entityID;
				pForwardBundle->append(s);

				gm->pushRouteMessage(entityID, targetCell, pForwardBundle);
				s.done();
				return;
			}
		}

		ERROR_MSG(fmt::format("Cellapp::onUpdateGhostVolatileData: not found entity({})\n", 
			entityID));

		s.done();
		return;
	}

	entity->onUpdateGhostVolatileData(s);
}

//-------------------------------------------------------------------------------------
void Cellapp::forwardEntityMessageToCellappFromClient(Network::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID srcEntityID;

	s >> srcEntityID;

	KBEngine::Entity* e = KBEngine::Cellapp::getSingleton().findEntity(srcEntityID);		

	if(e == NULL)
	{	
		WARNING_MSG(fmt::format("Cellapp::forwardEntityMessageToCellappFromClient: not found entityID:{}.\n",
			srcEntityID));
		
		s.done();
		return;
	}

	if(e->isDestroyed())
	{
		ERROR_MSG(fmt::format("{}::forwardEntityMessageToCellappFromClient: {} is destroyed!\n",	
			e->scriptName(), e->id()));

		s.done();
		return;
	}

	if (!e->isReal())
	{
		// ��Ҫ����ת
		GhostManager* gm = Cellapp::getSingleton().pGhostManager();
		if (gm)
		{
			Network::Bundle* pBundle = gm->createSendBundle(e->realCell());
			pBundle->newMessage(CellappInterface::forwardEntityMessageToCellappFromClient);
			(*pBundle) << srcEntityID;
			pBundle->append(s);
			gm->pushMessage(e->realCell(), pBundle);
		}

		return;
	}

	// ����Ƿ���entity��Ϣ�� ���򲻺Ϸ�.
	while(s.length() > 0 && !e->isDestroyed())
	{
		Network::MessageID currMsgID;
		Network::MessageLength currMsgLen;

		s >> currMsgID;

		Network::MessageHandler* pMsgHandler = CellappInterface::messageHandlers.find(currMsgID);

		if(pMsgHandler == NULL)
		{
			ERROR_MSG(fmt::format("Cellapp::forwardEntityMessageToCellappFromClient: invalide msgID={}, msglen={}, from {}.\n", 
				currMsgID, s.wpos(), pChannel->c_str()));

			s.done();
			return;
		}

		if(pMsgHandler->type() != Network::NETWORK_MESSAGE_TYPE_ENTITY)
		{
			WARNING_MSG(fmt::format("Cellapp::forwardEntityMessageToCellappFromClient: msgID={} not is entitymsg.\n",
				currMsgID));

			s.done();
			return;
		}

		if(pMsgHandler->msgLen == NETWORK_VARIABLE_MESSAGE)
			s >> currMsgLen;
		else
			currMsgLen = pMsgHandler->msgLen;

		if(s.length() < currMsgLen || currMsgLen >  NETWORK_MESSAGE_MAX_SIZE / 2)
		{
			ERROR_MSG(fmt::format("Cellapp::forwardEntityMessageToCellappFromClient: msgID={}, invalide msglen={}, from {}.\n", 
				currMsgID, s.wpos(), pChannel->c_str()));

			s.done();
			return;
		}

		// ��ʱ������Ч��ȡλ�� ��ֹ�ӿ����������
		size_t wpos = s.wpos();
		// size_t rpos = s.rpos();
		size_t frpos = s.rpos() + currMsgLen;
		s.wpos((int)frpos);

		try
		{
			pMsgHandler->handle(pChannel, s);
		}catch(MemoryStreamException &)
		{
			ERROR_MSG(fmt::format("Cellapp::forwardEntityMessageToCellappFromClient: message({}) error! entityID:{}.\n", 
				pMsgHandler->name.c_str(), srcEntityID));

			s.done();
			return;
		}

		// ��ֹhandle��û�н����ݵ�����ȡ�Ƿ�����
		if(currMsgLen > 0)
		{
			if(frpos != s.rpos())
			{
				CRITICAL_MSG(fmt::format("Cellapp::forwardEntityMessageToCellappFromClient[{}]: rpos({}) invalid, expect={}. msgID={}, msglen={}.\n",
					pMsgHandler->name.c_str(), s.rpos(), frpos, currMsgID, currMsgLen));

				s.rpos((int)frpos);
			}
		}

		s.wpos((int)wpos);
	}
}

//-------------------------------------------------------------------------------------
void Cellapp::reqSetFlags(Network::Channel* pChannel, MemoryStream& s)
{
	if (pChannel->isExternal())
		return;
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
void Cellapp::lookApp(Network::Channel* pChannel)
{
	if(pChannel->isExternal())
		return;

	//DEBUG_MSG(fmt::format("Cellapp::lookApp: {}\n", pChannel->c_str()));

	Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);
	
	(*pBundle) << g_componentType;
	(*pBundle) << componentID_;

	ShutdownHandler::SHUTDOWN_STATE state = shuttingdown();
	int8 istate = int8(state);
	(*pBundle) << istate;
	(*pBundle) << this->entitiesSize();
	(*pBundle) << (int32)SpaceMemorys::size();

	uint32 port = 0;
	if(pTelnetServer_)
		port = pTelnetServer_->port();

	(*pBundle) << port;

	pChannel->send(pBundle);
}

//-------------------------------------------------------------------------------------
PyObject* Cellapp::__py_reloadScript(PyObject* self, PyObject* args)
{
	bool fullReload = true;
	int argCount = (int)PyTuple_Size(args);
	if(argCount == 1)
	{
		if(!PyArg_ParseTuple(args, "b", &fullReload))
		{
			PyErr_Format(PyExc_TypeError, "KBEngine::reloadScript(fullReload): args error!");
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
	for(; eiter != entities.end(); ++eiter)
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
	const Network::Address& addr = Cellapp::getSingleton().networkInterface().intEndpoint().addr();
	PyTuple_SetItem(pyobj, 0,  PyLong_FromUnsignedLong(addr.ip));
	PyTuple_SetItem(pyobj, 1,  PyLong_FromUnsignedLong(addr.port));
	return pyobj;
}

//-------------------------------------------------------------------------------------
void Cellapp::reqTeleportToCellApp(Network::Channel* pChannel, MemoryStream& s)
{
	size_t rpos = s.rpos();

	ENTITY_ID nearbyMBRefID = 0, teleportEntityID = 0;
	Position3D pos;
	Direction3D dir;
	ENTITY_SCRIPT_UID entityType;
	SPACE_ID spaceID = 0;
	COMPONENT_ID entityBaseappID = 0;

	s >> teleportEntityID >> nearbyMBRefID >> spaceID;
	s >> entityType;
	s >> pos.x >> pos.y >> pos.z;
	s >> dir.dir.x >> dir.dir.y >> dir.dir.z;

	COMPONENT_ID ghostCell;
	s >> ghostCell;

	bool success = false;

	Entity* refEntity = Cellapp::getSingleton().findEntity(nearbyMBRefID);
	if (refEntity == NULL || refEntity->isDestroyed())
	{
		s.rpos((int)rpos);

		Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);
		(*pBundle).newMessage(CellappInterface::reqTeleportToCellAppCB);
		(*pBundle) << ghostCell << g_componentID << entityBaseappID;
		(*pBundle) << teleportEntityID;
		(*pBundle) << success;
		(*pBundle).append(&s);
		pChannel->send(pBundle);

		ERROR_MSG(fmt::format("Cellapp::reqTeleportToCellApp: not found refEntity({}), spaceID({}), reqTeleportEntity({})!\n",
			nearbyMBRefID, spaceID, teleportEntityID));

		s.done();
		return;
	}

	SpaceMemory* space = SpaceMemorys::findSpace(refEntity->spaceID());
	if (space == NULL || !space->isGood())
	{
		s.rpos((int)rpos);

		Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);
		(*pBundle).newMessage(CellappInterface::reqTeleportToCellAppCB);
		(*pBundle) << ghostCell << g_componentID << entityBaseappID;
		(*pBundle) << teleportEntityID;
		(*pBundle) << success;
		(*pBundle).append(&s);
		pChannel->send(pBundle);

		ERROR_MSG(fmt::format("Cellapp::reqTeleportToCellApp: not found space({}),  reqTeleportEntity({})!\n", spaceID, teleportEntityID));
		s.done();
		return;
	}

	// ����entity
	Entity* e = createEntity(EntityDef::findScriptModule(entityType)->getName(), NULL, false, teleportEntityID, false);
	if (e == NULL)
	{
		s.rpos((int)rpos);

		Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);
		(*pBundle).newMessage(CellappInterface::reqTeleportToCellAppCB);
		(*pBundle) << ghostCell << g_componentID << entityBaseappID;
		(*pBundle) << teleportEntityID;
		(*pBundle) << success;
		(*pBundle).append(&s);
		pChannel->send(pBundle);

		ERROR_MSG(fmt::format("Cellapp::reqTeleportToCellApp: create reqTeleportEntity({}) error!\n", teleportEntityID));
		s.done();
		return;
	}
	
	Py_INCREF(e);
	e->createFromStream(s);

	// �п������л�������ghost���ݰ����ƶ���������֮�������л�������Ϊ��
	// �ڴ���ʧ��ʱ�������ڻָ��ֳ�, ��ô���ͳɹ�������Ӧ��ֹͣ��ǰ���ƶ���Ϊ
	e->stopMove();

	// ���ڴ��Ͳ�����˵��ʵ�崫�͹����Ͳ�����ghost������
	// ��ǰʵ���������κθı䲻��Ҫͬ����ԭ��cell������ܻ����������Ϣ��ѭ��
	//ghostCell = 0;
	e->ghostCell(0);

	e->spaceID(space->id());
	e->setPositionAndDirection(pos, dir);

	if (e->baseEntityCall())
	{
		e->addFlags(ENTITY_FLAGS_TELEPORT_START);
		
		// �������base��ʵ�壬��Ҫ��baseappID���룬�Ա���reqTeleportToCellAppCB�лص���baseapp�������״̬
		entityBaseappID = e->baseEntityCall()->componentID();

		// ��baseapp���ʹ��͵���֪ͨ
		Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);
		(*pBundle).newMessage(BaseappInterface::onMigrationCellappEnd);
		(*pBundle) << e->id();
		(*pBundle) << ghostCell << g_componentID;
		e->baseEntityCall()->sendCall(pBundle);
	}

	// ������space֮ǰ����֪ͨ�ͻ���leaveSpace
	if (e->clientEntityCall())
	{
		Network::Bundle* pSendBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);
		NETWORK_ENTITY_MESSAGE_FORWARD_CLIENT_BEGIN(e->id(), (*pSendBundle));
		ENTITY_MESSAGE_FORWARD_CLIENT_BEGIN(pSendBundle, ClientInterface::onEntityLeaveSpace, leaveSpace);
		(*pSendBundle) << e->id();
		ENTITY_MESSAGE_FORWARD_CLIENT_END(pSendBundle, ClientInterface::onEntityLeaveSpace, leaveSpace);
		e->clientEntityCall()->sendCall(pSendBundle);
	}

	// �����µ�space��
	space->addEntityAndEnterWorld(e);

	Entity* nearbyMBRef = Cellapp::getSingleton().findEntity(nearbyMBRefID);
	e->onTeleportSuccess(nearbyMBRef, space->id());

	success = true;

	{
		Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);
		(*pBundle).newMessage(CellappInterface::reqTeleportToCellAppCB);
		(*pBundle) << ghostCell << g_componentID << entityBaseappID;
		(*pBundle) << teleportEntityID;
		(*pBundle) << success;
		pChannel->send(pBundle);
	}
	
	Py_DECREF(e);
}

//-------------------------------------------------------------------------------------
void Cellapp::reqTeleportToCellAppCB(Network::Channel* pChannel, MemoryStream& s)
{
	bool success;
	COMPONENT_ID sourceCellappID, targetCellappID, entityBaseappID;
	ENTITY_ID teleportEntityID = 0;

	s >> sourceCellappID >> targetCellappID >> entityBaseappID >> teleportEntityID >> success;

	// ��������£� Ӧ�ô��ͽ������ʱ����ǰ��ʵ��Ӧ���ڵ�ǰcell�ϣ� ���������cellapp���ˣ� ˵���ڴ��ڼ䱻Ǩ������
	// ��ʱ��Ǩ�ƺܿ��ܻ�������
	if (sourceCellappID != g_componentID)
	{
		ERROR_MSG(fmt::format("Cellapp::reqTeleportToCellAppCB(): sourceCellappID={} != currCellappID={}, targetCellappID={}\n", 
			sourceCellappID, g_componentID, targetCellappID));
	}

	// ���ͳɹ��������������entity
	if(success)
	{
		destroyEntity(teleportEntityID, false);
		return;
	}

	// ʵ�����û��base���֣���ô����Ҫ֪ͨbaseapp
	if (entityBaseappID > 0)
	{
		Components::ComponentInfos* pInfos = Components::getSingleton().findComponent(entityBaseappID);
		if (pInfos && pInfos->pChannel)
		{
			Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);
			(*pBundle).newMessage(BaseappInterface::onMigrationCellappEnd);
			(*pBundle) << teleportEntityID;
			(*pBundle) << sourceCellappID << sourceCellappID;
			pInfos->pChannel->send(pBundle);
		}
		else
		{
			ERROR_MSG(fmt::format("Cellapp::reqTeleportToCellAppCB: not found baseapp({}), entity({})!\n",
				entityBaseappID, teleportEntityID));
		}
	}

	// ĳЩ�����ʵ����ܴ�ʱ�Ҳ����ˣ����磺����������
	Entity* entity = Cellapp::getSingleton().findEntity(teleportEntityID);
	if(entity == NULL)
	{
		ERROR_MSG(fmt::format("Cellapp::reqTeleportToCellAppCB: not found reqTeleportEntity({}), lose entity!\n", 
			teleportEntityID));

		s.done();
		return;
	}

	// ����ʧ���ˣ�������Ҫ�ػָ�entity
	ENTITY_ID nearbyMBRefID = 0;
	Position3D pos;
	Direction3D dir;
	ENTITY_SCRIPT_UID entityType;
	SPACE_ID lastSpaceID = 0;
	COMPONENT_ID cid;

	s >> teleportEntityID >> nearbyMBRefID >> lastSpaceID;
	s >> entityType;
	s >> pos.x >> pos.y >> pos.z;
	s >> dir.dir.x >> dir.dir.y >> dir.dir.z;
	s >> cid;

	Py_INCREF(entity);
	entity->changeToReal(0, s);
	entity->onTeleportFailure();
	Py_DECREF(entity);
	
	s.done();
}

//-------------------------------------------------------------------------------------
void Cellapp::reqTeleportToCellAppOver(Network::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID teleportEntityID = 0;

	s >> teleportEntityID;
	
	// ĳЩ�����ʵ����ܴ�ʱ�Ҳ����ˣ����磺����������
	Entity* entity = Cellapp::getSingleton().findEntity(teleportEntityID);
	if(entity == NULL)
	{
		ERROR_MSG(fmt::format("Cellapp::reqTeleportToCellAppOver: not found reqTeleportEntity({}), lose entity!\n", 
			teleportEntityID));

		s.done();
		return;
	}

	entity->removeFlags(ENTITY_FLAGS_TELEPORT_START);
}

//-------------------------------------------------------------------------------------
int Cellapp::raycast(SPACE_ID spaceID, int layer, const Position3D& start, const Position3D& end, std::vector<Position3D>& hitPos)
{
	SpaceMemory* pSpace = SpaceMemorys::findSpace(spaceID);
	if(pSpace == NULL)
	{
		ERROR_MSG(fmt::format("Cellapp::raycast: not found space({})!\n", 
			spaceID));

		return -1;
	}
	
	if(pSpace->pNavHandle() == NULL)
	{
		ERROR_MSG(fmt::format("Cellapp::raycast: space({}) not addSpaceGeometryMapping! layer={}\n", 
			spaceID, layer));

		return -1;
	}

	return pSpace->pNavHandle()->raycast(layer, start, end, hitPos);
}

//-------------------------------------------------------------------------------------
PyObject* Cellapp::__py_raycast(PyObject* self, PyObject* args)
{
	uint16 currargsSize = (uint16)PyTuple_Size(args);

	int layer = 0;
	SPACE_ID spaceID = 0;

	PyObject* pyStartPos = NULL;
	PyObject* pyEndPos = NULL;

	if(currargsSize == 3)
	{
		if(!PyArg_ParseTuple(args, "IOO", &spaceID, &pyStartPos, &pyEndPos))
		{
			PyErr_Format(PyExc_TypeError, "Cellapp::raycast: args error!");
			PyErr_PrintEx(0);
			return 0;
		}
	}
	else if(currargsSize == 4)
	{
		if(!PyArg_ParseTuple(args, "IiOO", &spaceID, &layer, &pyStartPos, &pyEndPos))
		{
			PyErr_Format(PyExc_TypeError, "Cellapp::raycast: args error!");
			PyErr_PrintEx(0);
			return 0;
		}
	}
	else
	{
		PyErr_Format(PyExc_TypeError, "Cellapp::raycast: args error!");
		PyErr_PrintEx(0);
		return 0;
	}

	if(!PySequence_Check(pyStartPos))
	{
		PyErr_Format(PyExc_TypeError, "Cellapp::raycast: args1(startPos) not is PySequence!");
		PyErr_PrintEx(0);
		return 0;
	}

	if(!PySequence_Check(pyEndPos))
	{
		PyErr_Format(PyExc_TypeError, "Cellapp::raycast: args2(endPos) not is PySequence!");
		PyErr_PrintEx(0);
		return 0;
	}

	if(PySequence_Size(pyStartPos) != 3)
	{
		PyErr_Format(PyExc_TypeError, "Cellapp::raycast: args1(startPos) invalid!");
		PyErr_PrintEx(0);
		return 0;
	}

	if(PySequence_Size(pyEndPos) != 3)
	{
		PyErr_Format(PyExc_TypeError, "Cellapp::raycast: args2(endPos) invalid!");
		PyErr_PrintEx(0);
		return 0;
	}

	Position3D startPos;
	Position3D endPos;
	std::vector<Position3D> hitPosVec;
	//float hitPos[3];

	script::ScriptVector3::convertPyObjectToVector3(startPos, pyStartPos);
	script::ScriptVector3::convertPyObjectToVector3(endPos, pyEndPos);
	if(Cellapp::getSingleton().raycast(spaceID, layer, startPos, endPos, hitPosVec) <= 0)
	{
		S_Return;
	}

	int idx = 0;
	PyObject* pyHitpos = PyTuple_New(hitPosVec.size());
	for(std::vector<Position3D>::iterator iter = hitPosVec.begin(); iter != hitPosVec.end(); ++iter)
	{
		PyObject* pyHitposItem = PyTuple_New(3);
		PyTuple_SetItem(pyHitposItem, 0, ::PyFloat_FromDouble((*iter).x));
		PyTuple_SetItem(pyHitposItem, 1, ::PyFloat_FromDouble((*iter).y));
		PyTuple_SetItem(pyHitposItem, 2, ::PyFloat_FromDouble((*iter).z));

		PyTuple_SetItem(pyHitpos, idx++, pyHitposItem);
	}

	return pyHitpos;
}

//-------------------------------------------------------------------------------------
PyObject* Cellapp::__py_getFlags(PyObject* self, PyObject* args)
{
	return PyLong_FromUnsignedLong(Cellapp::getSingleton().flags());
}

//-------------------------------------------------------------------------------------
PyObject* Cellapp::__py_setFlags(PyObject* self, PyObject* args)
{
	if(PyTuple_Size(args) != 1)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::setFlags: argsSize != 1!");
		PyErr_PrintEx(0);
		return NULL;
	}

	uint32 flags;

	if(!PyArg_ParseTuple(args, "I", &flags))
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::setFlags: args error!");
		PyErr_PrintEx(0);
		return NULL;
	}

	Cellapp::getSingleton().flags(flags);
	S_Return;
}

//-------------------------------------------------------------------------------------
void Cellapp::setSpaceViewer(Network::Channel* pChannel, MemoryStream& s)
{
	bool del = false;
	s >> del;

	SPACE_ID spaceID;
	s >> spaceID;

	// ���Ϊ0����鿴����cell
	CELL_ID cellID;
	s >> cellID;

	spaceViewers_.updateSpaceViewer(pChannel->addr(), spaceID, cellID, del);
}

//-------------------------------------------------------------------------------------

}
