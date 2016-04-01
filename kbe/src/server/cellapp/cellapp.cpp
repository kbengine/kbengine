/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2016 KBEngine.

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


#include "cellapp.h"
#include "space.h"
#include "profile.h"
#include "witness.h"
#include "coordinate_node.h"
#include "aoi_trigger.h"
#include "watch_obj_pools.h"
#include "cellapp_interface.h"
#include "entity_remotemethod.h"
#include "initprogress_handler.h"
#include "forward_message_over_handler.h"
#include "network/tcp_packet.h"
#include "network/udp_packet.h"
#include "server/components.h"
#include "server/telnet_server.h"
#include "dbmgr/dbmgr_interface.h"
#include "navigation/navigation.h"
#include "client_lib/client_interface.h"

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
	flags_(APP_FLAGS_NONE)
{
	KBEngine::Network::MessageHandlers::pMainMessageHandlers = &CellappInterface::messageHandlers;

	// hook mailboxcall
	static EntityMailbox::MailboxCallHookFunc mailboxCallHookFunc = std::tr1::bind(&Cellapp::createMailboxCallEntityRemoteMethod, this, 
		std::tr1::placeholders::_1, std::tr1::placeholders::_2);

	EntityMailbox::setMailboxCallHookFunc(&mailboxCallHookFunc);
}

//-------------------------------------------------------------------------------------
Cellapp::~Cellapp()
{
	EntityMailbox::resetCallHooks();
}

//-------------------------------------------------------------------------------------	
bool Cellapp::canShutdown()
{
	Entities<Entity>::ENTITYS_MAP& entities =  this->pEntities()->getEntities();
	Entities<Entity>::ENTITYS_MAP::iterator iter = entities.begin();
	for(; iter != entities.end(); ++iter)
	{
		//Entity* pEntity = static_cast<Entity*>(iter->second.get());
		//if(pEntity->baseMailbox() != NULL && 
		//		pEntity->pScriptModule()->isPersistent())
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

	uint32 count = g_serverConfig.getCellApp().perSecsDestroyEntitySize;
	Entities<Entity>::ENTITYS_MAP& entities =  this->pEntities()->getEntities();

	while(count > 0)
	{
		std::vector<Entity*> vecs;

		bool done = false;
		Entities<Entity>::ENTITYS_MAP::iterator iter = entities.begin();
		for(; iter != entities.end(); ++iter)
		{
			//Entity* pEntity = static_cast<Entity*>(iter->second.get());
			//if(pEntity->baseMailbox() != NULL && 
			//	pEntity->pScriptModule()->isPersistent())
			{
				this->destroyEntity(static_cast<Entity*>(iter->second.get())->id(), true);

				count--;
				done = true;
				break;
			}
		}

		// 如果count等于perSecsDestroyEntitySize说明上面已经没有可处理的东西了
		// 剩下的应该都是space，可以开始销毁了
		Spaces::finalise();

		if(!done)
			break;
	}
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
	GlobalDataClient::installScript(getScript().getModule());

	registerScript(Entity::getScriptType());
	
	// 将app标记注册到脚本
	std::map<uint32, std::string> flagsmaps = createAppFlagsMaps();
	std::map<uint32, std::string>::iterator fiter = flagsmaps.begin();
	for (; fiter != flagsmaps.end(); ++fiter)
	{
		if (PyModule_AddIntConstant(getScript().getModule(), fiter->second.c_str(), fiter->first))
		{
			ERROR_MSG(fmt::format("Cellapp::onInstallPyModules: Unable to set KBEngine.{}.\n", fiter->second));
		}
	}

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
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(),		raycast,						__py_raycast,						METH_VARARGS,			0);
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(), 		setAppFlags,					__py_setFlags,						METH_VARARGS,			0);
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(), 		getAppFlags,					__py_getFlags,						METH_VARARGS,			0);
	
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

	// 一定要在最前面
	updateLoad();

	EntityApp<Entity>::handleGameTick();

	updatables_.update();
	Spaces::update();
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
	if(g_kbeSrvConfig.getCellApp().profiles.open_pyprofile)
	{
		script::PyProfile::start("kbengine");
	}

	pWitnessedTimeoutHandler_ = new WitnessedTimeoutHandler();

	// 是否管理Y轴
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
	SAFE_RELEASE(pGhostManager_);
	SAFE_RELEASE(pWitnessedTimeoutHandler_);

	if(pTelnetServer_)
	{
		pTelnetServer_->stop();
		SAFE_RELEASE(pTelnetServer_);
	}

	Spaces::finalise();
	Navigation::getSingleton().finalise();
	forward_messagebuffer_.clear();

	EntityApp<Entity>::finalise();
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

			Network::Bundle* pBundle = Network::Bundle::createPoolObject();
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
	KBE_ASSERT(ret != -1);

	Network::Bundle* pBundle = Network::Bundle::createPoolObject();

	switch(tcomponentType)
	{
	case BASEAPP_TYPE:
		(*pBundle).newMessage(BaseappInterface::onRegisterNewApp);
		BaseappInterface::onRegisterNewAppArgs11::staticAddToBundle((*pBundle), getUserUID(), getUsername(), 
			CELLAPP_TYPE, componentID_, startGlobalOrder_, startGroupOrder_,
			this->networkInterface().intaddr().ip, this->networkInterface().intaddr().port, 
			this->networkInterface().extaddr().ip, this->networkInterface().extaddr().port, g_kbeSrvConfig.getConfig().externalAddress);
		break;
	case CELLAPP_TYPE:
		(*pBundle).newMessage(CellappInterface::onRegisterNewApp);
		CellappInterface::onRegisterNewAppArgs11::staticAddToBundle((*pBundle), getUserUID(), getUsername(), 
			CELLAPP_TYPE, componentID_, startGlobalOrder_, startGroupOrder_,
			this->networkInterface().intaddr().ip, this->networkInterface().intaddr().port, 
			this->networkInterface().extaddr().ip, this->networkInterface().extaddr().port, g_kbeSrvConfig.getConfig().externalAddress);
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
		Network::Bundle* pBundle = Network::Bundle::createPoolObject();
		(*pBundle).newMessage(CellappmgrInterface::updateCellapp);
		CellappmgrInterface::updateCellappArgs4::staticAddToBundle((*pBundle), 
			componentID_, (ENTITY_ID)pEntities_->getEntities().size(), getLoad(), flags_);

		pChannel->send(pBundle);
	}
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
	if(space == NULL || !space->isGood())
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::createEntity: spaceID %ld not found.", spaceID);
		PyErr_PrintEx(0);
		return 0;
	}
	
	// 创建entity
	Entity* pEntity = Cellapp::getSingleton().createEntity(entityType, params, false, 0);

	if(pEntity != NULL)
	{
		Py_INCREF(pEntity);
		pEntity->spaceID(space->id());
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
	int argCount = (int)PyTuple_Size(args);
	PyObject* pycallback = NULL;
	PyObject* pyDBInterfaceName = NULL;
	int ret = -1;
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

	if(ret == -1)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::executeRawDatabaseCommand: args is error!");
		PyErr_PrintEx(0);
		S_Return;
	}
	
	std::string dbInterfaceName = "default";
	if (pyDBInterfaceName)
	{
		wchar_t* PyUnicode_AsWideCharStringRet0 = PyUnicode_AsWideCharString(pyDBInterfaceName, NULL);
		char* ccattr = strutil::wchar2char(PyUnicode_AsWideCharStringRet0);
		dbInterfaceName = ccattr;
		PyMem_Free(PyUnicode_AsWideCharStringRet0);
		free(ccattr);
		
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

	int dbInterfaceIndex = g_kbeSrvConfig.dbInterfaceName2dbInterfaceIndex(dbInterfaceName);
	if (dbInterfaceIndex < 0)
	{
		ERROR_MSG(fmt::format("KBEngine::executeRawDatabaseCommand: not found dbInterface({})!\n",
			dbInterfaceName));

		return;
	}

	//INFO_MSG(fmt::format("KBEngine::executeRawDatabaseCommand{}:{}.\n", (eid > 0 ? fmt::format("(entityID={})", eid) : ""), datas));

	Network::Bundle* pBundle = Network::Bundle::createPoolObject();
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
												const_cast<char*>("OOO"), 
												pResultSet, pAffectedRows, pErrorMsg);

			if(pyResult != NULL)
				Py_DECREF(pyResult);
			else
				SCRIPT_ERROR_CHECK();
		}
		else
		{
			ERROR_MSG(fmt::format("Cellapp::onExecuteRawDatabaseCommandCB: can't found callback:{}.\n",
				callbackID));
		}
	}

	Py_XDECREF(pResultSet);
	Py_XDECREF(pAffectedRows);
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

	new InitProgressHandler(this->networkInterface());
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
void Cellapp::onCreateInNewSpaceFromBaseapp(Network::Channel* pChannel, KBEngine::MemoryStream& s)
{
	std::string entityType;
	ENTITY_ID mailboxEntityID;
	COMPONENT_ID componentID;
	SPACE_ID spaceID = 1;
	bool hasClient;

	s >> entityType;
	s >> mailboxEntityID;
	s >> spaceID;
	s >> componentID;
	s >> hasClient;

	// DEBUG_MSG("Cellapp::onCreateInNewSpaceFromBaseapp: spaceID=%u, entityType=%s, entityID=%d, componentID=%"PRAppID".\n", 
	//	spaceID, entityType.c_str(), mailboxEntityID, componentID);

	Space* space = Spaces::createNewSpace(spaceID);
	if(space != NULL)
	{
		// 创建entity
		Entity* e = createEntity(entityType.c_str(), NULL, false, mailboxEntityID, false);
		
		if(e == NULL)
		{
			s.done();

			ERROR_MSG("Cellapp::onCreateInNewSpaceFromBaseapp: createEntity error!\n");

			/* 目前来说除非内存或者系统问题，否则不会出现这个错误
			Network::Bundle* pBundle = Network::Bundle::createPoolObject();
			pBundle->newMessage(BaseappInterface::onCreateCellFailure);
			BaseappInterface::onCreateCellFailureArgs1::staticAddToBundle(*pBundle, mailboxEntityID);
			cinfos->pChannel->send(pBundle);
			*/
			return;
		}

		PyObject* cellData = e->createCellDataFromStream(&s);

		// 设置entity的baseMailbox
		EntityMailbox* mailbox = new EntityMailbox(e->pScriptModule(), NULL, componentID, mailboxEntityID, MAILBOX_TYPE_BASE);
		e->baseMailbox(mailbox);
		
		if (hasClient)
		{
			KBE_ASSERT(e->baseMailbox() != NULL && !e->hasWitness());
			PyObject* clientMailbox = PyObject_GetAttrString(e->baseMailbox(), "client");
			KBE_ASSERT(clientMailbox != Py_None);

			EntityMailbox* client = static_cast<EntityMailbox*>(clientMailbox);
			// Py_INCREF(clientMailbox); 这里不需要增加引用， 因为每次都会产生一个新的对象

			// 为了能够让entity.__init__中能够修改属性立刻能广播到客户端我们需要提前设置这些
			e->clientMailbox(client);
			e->setWitness(Witness::createPoolObject());
		}

		// 此处baseapp可能还有没初始化过来， 所以有一定概率是为None的
		Components::ComponentInfos* cinfos = Components::getSingleton().findComponent(BASEAPP_TYPE, componentID);
		if(cinfos == NULL || cinfos->pChannel == NULL)
		{
			Network::Bundle* pBundle = Network::Bundle::createPoolObject();
			ForwardItem* pFI = new ForwardItem();
			pFI->pHandler = new FMH_Baseapp_onEntityGetCellFrom_onCreateInNewSpaceFromBaseapp(e, spaceID, cellData);
			//Py_XDECREF(cellData);
			pFI->pBundle = pBundle;
			(*pBundle).newMessage(BaseappInterface::onEntityGetCell);
			BaseappInterface::onEntityGetCellArgs3::staticAddToBundle((*pBundle), mailboxEntityID, componentID_, spaceID);
			forward_messagebuffer_.push(componentID, pFI);
			
			WARNING_MSG(fmt::format("Cellapp::onCreateInNewSpaceFromBaseapp: not found baseapp({}), message is buffered.\n",
				componentID));
			
			return;
		}

		space->addEntity(e);
		e->spaceID(space->id());
		e->initializeEntity(cellData);
		Py_XDECREF(cellData);

		// 添加到space
		space->addEntityToNode(e);

		if (hasClient)
		{
			e->onGetWitness();
		}
		else
		{
			space->onEnterWorld(e);
		}

		Network::Bundle* pBundle = Network::Bundle::createPoolObject();
		(*pBundle).newMessage(BaseappInterface::onEntityGetCell);
		BaseappInterface::onEntityGetCellArgs3::staticAddToBundle((*pBundle), mailboxEntityID, componentID_, spaceID);
		cinfos->pChannel->send(pBundle);

		return;
	}
	
	ERROR_MSG(fmt::format("Cellapp::onCreateInNewSpaceFromBaseapp: not found baseapp[{}], entityID={}, spaceID={}.\n",
		componentID, mailboxEntityID, spaceID));
}

//-------------------------------------------------------------------------------------
void Cellapp::onRestoreSpaceInCellFromBaseapp(Network::Channel* pChannel, KBEngine::MemoryStream& s)
{
	std::string entityType;
	ENTITY_ID mailboxEntityID;
	COMPONENT_ID componentID;
	SPACE_ID spaceID = 1;
	bool hasClient;

	s >> entityType;
	s >> mailboxEntityID;
	s >> spaceID;
	s >> componentID;
	s >> hasClient;

	// DEBUG_MSG("Cellapp::onRestoreSpaceInCellFromBaseapp: spaceID=%u, entityType=%s, entityID=%d, componentID=%"PRAppID".\n", 
	//	spaceID, entityType.c_str(), mailboxEntityID, componentID);

	Space* space = Spaces::createNewSpace(spaceID);
	if(space != NULL)
	{
		// 创建entity
		Entity* e = createEntity(entityType.c_str(), NULL, false, mailboxEntityID, false);
		
		if(e == NULL)
		{
			s.done();
			return;
		}

		PyObject* cellData = e->createCellDataFromStream(&s);

		// 设置entity的baseMailbox
		EntityMailbox* mailbox = new EntityMailbox(e->pScriptModule(), NULL, componentID, mailboxEntityID, MAILBOX_TYPE_BASE);
		e->baseMailbox(mailbox);
		
		if (hasClient)
		{
			KBE_ASSERT(e->baseMailbox() != NULL && !e->hasWitness());
			PyObject* clientMailbox = PyObject_GetAttrString(e->baseMailbox(), "client");
			KBE_ASSERT(clientMailbox != Py_None);

			EntityMailbox* client = static_cast<EntityMailbox*>(clientMailbox);
			// Py_INCREF(clientMailbox); 这里不需要增加引用， 因为每次都会产生一个新的对象

			// 为了能够让entity.__init__中能够修改属性立刻能广播到客户端我们需要提前设置这些
			e->clientMailbox(client);
			e->setWitness(Witness::createPoolObject());
		}

		// 此处baseapp可能还有没初始化过来， 所以有一定概率是为None的
		Components::ComponentInfos* cinfos = Components::getSingleton().findComponent(BASEAPP_TYPE, componentID);
		if(cinfos == NULL || cinfos->pChannel == NULL)
		{
			Network::Bundle* pBundle = Network::Bundle::createPoolObject();
			ForwardItem* pFI = new ForwardItem();
			pFI->pHandler = new FMH_Baseapp_onEntityGetCellFrom_onCreateInNewSpaceFromBaseapp(e, spaceID, cellData);
			//Py_XDECREF(cellData);
			pFI->pBundle = pBundle;
			(*pBundle).newMessage(BaseappInterface::onEntityGetCell);
			BaseappInterface::onEntityGetCellArgs3::staticAddToBundle((*pBundle), mailboxEntityID, componentID_, spaceID);
			forward_messagebuffer_.push(componentID, pFI);
			
			WARNING_MSG(fmt::format("Cellapp::onRestoreSpaceInCellFromBaseapp: not found baseapp({}), message has been buffered.\n",
				componentID));
			
			return;
		}
		
		e->spaceID(space->id());
		e->createNamespace(cellData);
		Py_XDECREF(cellData);

		// 添加到space
		e->onRestore();

		space->addEntityAndEnterWorld(e, true);

		Network::Bundle* pBundle = Network::Bundle::createPoolObject();
		(*pBundle).newMessage(BaseappInterface::onEntityGetCell);
		BaseappInterface::onEntityGetCellArgs3::staticAddToBundle((*pBundle), mailboxEntityID, componentID_, spaceID);
		cinfos->pChannel->send(pBundle);
		return;
	}
	
	ERROR_MSG(fmt::format("Cellapp::onRestoreSpaceInCellFromBaseapp: not found baseapp[{}], entityID={}, spaceID={}.\n",
		componentID, mailboxEntityID, spaceID));
}

//-------------------------------------------------------------------------------------
void Cellapp::requestRestore(Network::Channel* pChannel, KBEngine::MemoryStream& s)
{
	COMPONENT_ID cid;
	s >> cid;

	bool canRestore = idClient_.size() > 0;

	DEBUG_MSG(fmt::format("Cellapp::requestRestore: cid={}, canRestore={}, channel={}.\n", 
		cid, canRestore, pChannel->c_str()));

	Network::Bundle* pBundle = Network::Bundle::createPoolObject();
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

	// 此处baseapp可能还有没初始化过来， 所以有一定概率是为None的
	Components::ComponentInfos* cinfos = Components::getSingleton().findComponent(BASEAPP_TYPE, componentID);
	if(cinfos == NULL || cinfos->pChannel == NULL)
	{
		MemoryStream* pCellData = MemoryStream::createPoolObject();
		pCellData->append(s);

		Network::Bundle* pBundle = Network::Bundle::createPoolObject();
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
	// 注意：此处理论不会找不到组件， 因为onCreateCellEntityFromBaseapp中已经进行过一次消息缓存判断
	Components::ComponentInfos* cinfos = Components::getSingleton().findComponent(BASEAPP_TYPE, componentID);
	KBE_ASSERT(cinfos != NULL && cinfos->pChannel != NULL);

	Entity* pCreateToEntity = pEntities_->find(createToEntityID);

	// 可能spaceEntity已经销毁了， 但还未来得及通知到baseapp时
	// base部分在向这个space创建entity
	if(pCreateToEntity == NULL)
	{
		ERROR_MSG("Cellapp::_onCreateCellEntityFromBaseapp: not fount spaceEntity. may have been destroyed!\n");

		Network::Bundle* pBundle = Network::Bundle::createPoolObject();
		pBundle->newMessage(BaseappInterface::onCreateCellFailure);
		BaseappInterface::onCreateCellFailureArgs1::staticAddToBundle(*pBundle, entityID);
		cinfos->pChannel->send(pBundle);
		return;
	}

	spaceID = pCreateToEntity->spaceID();

	//DEBUG_MSG("Cellapp::onCreateCellEntityFromBaseapp: spaceID=%u, entityType=%s, entityID=%d, componentID=%"PRAppID".\n", 
	//	spaceID, entityType.c_str(), entityID, componentID);

	Space* space = Spaces::findSpace(spaceID);
	if(space != NULL && space->isGood())
	{
		// 告知baseapp， entity的cell创建了
		Network::Bundle* pBundle = Network::Bundle::createPoolObject();
		pBundle->newMessage(BaseappInterface::onEntityGetCell);
		BaseappInterface::onEntityGetCellArgs3::staticAddToBundle(*pBundle, entityID, componentID_, spaceID);
		cinfos->pChannel->send(pBundle);

		// 解包cellData信息.
		PyObject* cellData = NULL;
	
		// 创建entity
		Entity* e = createEntity(entityType.c_str(), cellData, false, entityID, false);
		
		if(e == NULL)
		{
			Py_XDECREF(cellData);
			return;
		}

		// 设置entity的baseMailbox
		EntityMailbox* mailbox = new EntityMailbox(e->pScriptModule(), NULL, componentID, entityID, MAILBOX_TYPE_BASE);
		e->baseMailbox(mailbox);
		
		cellData = e->createCellDataFromStream(pCellData);
		e->createNamespace(cellData);

		if(hasClient)
		{
			KBE_ASSERT(e->baseMailbox() != NULL && !e->hasWitness());
			PyObject* clientMailbox = PyObject_GetAttrString(e->baseMailbox(), "client");
			KBE_ASSERT(clientMailbox != Py_None);

			EntityMailbox* client = static_cast<EntityMailbox*>(clientMailbox);	
			// Py_INCREF(clientMailbox); 这里不需要增加引用， 因为每次都会产生一个新的对象

			// 为了能够让entity.__init__中能够修改属性立刻能广播到客户端我们需要提前设置这些
			e->clientMailbox(client);
			e->setWitness(Witness::createPoolObject());
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
			e->onGetWitness();
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
void Cellapp::onDestroyCellEntityFromBaseapp(Network::Channel* pChannel, ENTITY_ID eid)
{
	// DEBUG_MSG("Cellapp::onDestroyCellEntityFromBaseapp:entityID=%d.\n", eid);
	destroyEntity(eid, true);
}

//-------------------------------------------------------------------------------------
RemoteEntityMethod* Cellapp::createMailboxCallEntityRemoteMethod(MethodDescription* pMethodDescription, EntityMailbox* pMailbox)
{
	return new EntityRemoteMethod(pMethodDescription, pMailbox);
}

//-------------------------------------------------------------------------------------
void Cellapp::onEntityMail(Network::Channel* pChannel, KBEngine::MemoryStream& s)
{
	ENTITY_ID eid;
	s >> eid;

	ENTITY_MAILBOX_TYPE	mailtype;
	s >> mailtype;

	// 在本地区尝试查找该收件人信息， 看收件人是否属于本区域
	Entity* entity = pEntities_->find(eid);
	if(entity == NULL)
	{
		if(mailtype == MAILBOX_TYPE_CELL)
		{
			GhostManager* gm = Cellapp::getSingleton().pGhostManager();
			COMPONENT_ID cellID = gm->getRoute(eid);
			if(gm && cellID > 0)
			{
				Network::Bundle* pBundle = Network::Bundle::createPoolObject();
				(*pBundle).newMessage(CellappInterface::onEntityMail);
				(*pBundle) << eid << mailtype;
				(*pBundle).append(s);
				gm->pushMessage(cellID, pBundle);
				s.done();
				return;
			}
		}

		WARNING_MSG(fmt::format("Cellapp::onEntityMail: entityID {} not found.\n", eid));
		s.done();
		return;
	}
	
	Network::Bundle* pBundle = Network::Bundle::createPoolObject();
	Network::Bundle& bundle = *pBundle;
	bool reclaim = true;

	switch(mailtype)
	{
		// 本组件是cellapp，那么确认邮件的目的地是这里， 那么执行最终操作
		case MAILBOX_TYPE_CELL:	
			{
				if(!entity->isReal())
				{
					GhostManager* gm = Cellapp::getSingleton().pGhostManager();
					if(gm)
					{
						bundle.newMessage(CellappInterface::onEntityMail);
						bundle << eid << mailtype;
						bundle.append(s);
						gm->pushMessage(entity->realCell(), pBundle);
						reclaim = false;
					}
				}
				else
				{
					entity->onRemoteMethodCall(pChannel, s);
				}
			}

			break;

		// entity.base.cell.xxx
		case MAILBOX_TYPE_BASE_VIA_CELL: 
			{
				EntityMailboxAbstract* mailbox = static_cast<EntityMailboxAbstract*>(entity->baseMailbox());
				if(mailbox == NULL)
				{
					WARNING_MSG(fmt::format("Cellapp::onEntityMail: not found baseMailbox! mailboxType={}, entityID={}.\n",
						mailtype, eid));

					break;
				}
				
				mailbox->newMail(bundle);
				bundle.append(s);
				mailbox->postMail(pBundle);
				reclaim = false;
			}
			break;
		
		// entity.cell.client
		case MAILBOX_TYPE_CLIENT_VIA_CELL: 
			{
				EntityMailboxAbstract* mailbox = static_cast<EntityMailboxAbstract*>(entity->clientMailbox());
				if(mailbox == NULL)
				{
					WARNING_MSG(fmt::format("Cellapp::onEntityMail: not found clientMailbox! mailboxType={}, entityID={}.\n",
						mailtype, eid));

					break;
				}
				
				mailbox->newMail(bundle);
				bundle.append(s);
				s.done();
				mailbox->postMail(pBundle);
				reclaim = false;
			}
			break;
		default:
			{
				ERROR_MSG(fmt::format("Cellapp::onEntityMail: mailboxType {} is error! must a cellType. entityID={}.\n",
					mailtype, eid));
			}
	};

	if(reclaim)
		Network::Bundle::reclaimPoolObject(pBundle);

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

	// 这个方法呼叫如果不是这个proxy自己的方法则必须呼叫的entity和proxy的cellEntity在一个space中。
	try
	{
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
		ERROR_MSG(fmt::format("Cellapp::onUpdateDataFromClient: not found entity {}!\n", srcEntityID));
		
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
				Network::Bundle* pForwardBundle = Network::Bundle::createPoolObject();
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
				Network::Bundle* pForwardBundle = Network::Bundle::createPoolObject();
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
				Network::Bundle* pForwardBundle = Network::Bundle::createPoolObject();
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

	// 检查是否是entity消息， 否则不合法.
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

		if((pMsgHandler->msgLen == NETWORK_VARIABLE_MESSAGE) || Network::g_packetAlwaysContainLength)
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

		// 临时设置有效读取位， 防止接口中溢出操作
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

		// 防止handle中没有将数据导出获取非法操作
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

	DEBUG_MSG(fmt::format("Cellapp::lookApp: {}\n", pChannel->c_str()));

	Network::Bundle* pBundle = Network::Bundle::createPoolObject();
	
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

	pChannel->send(pBundle);
}

//-------------------------------------------------------------------------------------
PyObject* Cellapp::__py_reloadScript(PyObject* self, PyObject* args)
{
	bool fullReload = true;
	int argCount = (int)PyTuple_Size(args);
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
	
	bool success = false;

	Entity* refEntity = Cellapp::getSingleton().findEntity(nearbyMBRefID);
	if(refEntity == NULL || refEntity->isDestroyed())
	{
		s.rpos((int)rpos);

		Network::Bundle* pBundle = Network::Bundle::createPoolObject();
		(*pBundle).newMessage(CellappInterface::reqTeleportToCellAppCB);
		(*pBundle) << g_componentID << entityBaseappID;
		(*pBundle) << teleportEntityID;
		(*pBundle) << success;
		(*pBundle).append(&s);
		pChannel->send(pBundle);

		ERROR_MSG(fmt::format("Cellapp::reqTeleportToCellApp: not found refEntity({}), spaceID({}), reqTeleportEntity({})!\n", 
			nearbyMBRefID, spaceID, teleportEntityID));

		s.done();
		return;
	}

	Space* space = Spaces::findSpace(refEntity->spaceID());
	if(space == NULL || !space->isGood())
	{
		s.rpos((int)rpos);

		Network::Bundle* pBundle = Network::Bundle::createPoolObject();
		(*pBundle).newMessage(CellappInterface::reqTeleportToCellAppCB);
		(*pBundle) << g_componentID << entityBaseappID;
		(*pBundle) << teleportEntityID;
		(*pBundle) << success;
		(*pBundle).append(&s);
		pChannel->send(pBundle);

		ERROR_MSG(fmt::format("Cellapp::reqTeleportToCellApp: not found space({}),  reqTeleportEntity({})!\n", spaceID, teleportEntityID));
		s.done();
		return;
	}

	// 创建entity
	Entity* e = createEntity(EntityDef::findScriptModule(entityType)->getName(), NULL, false, teleportEntityID, false);
	if(e == NULL)
	{
		s.rpos((int)rpos);

		Network::Bundle* pBundle = Network::Bundle::createPoolObject();
		(*pBundle).newMessage(CellappInterface::reqTeleportToCellAppCB);
		(*pBundle) << g_componentID << entityBaseappID;
		(*pBundle) << teleportEntityID;
		(*pBundle) << success;
		(*pBundle).append(&s);
		pChannel->send(pBundle);

		ERROR_MSG(fmt::format("Cellapp::reqTeleportToCellApp: create reqTeleportEntity({}) error!\n", teleportEntityID));
		s.done();
		return;
	}
	
	e->createFromStream(s);

	// 有可能序列化过来的ghost内容包含移动控制器，之所以序列化过来是为了
	// 在传送失败时可以用于恢复现场, 那么传送成功了我们应该停止以前的移动行为
	e->stopMove();

	COMPONENT_ID ghostCell;
	s >> ghostCell;

	// 对于传送操作来说，实体传送过来就不会有ghost部分了
	// 当前实体做出的任何改变不需要同步到原有cell，这可能会产生网络消息死循环
	ghostCell = 0;

	e->ghostCell(ghostCell);
	e->spaceID(space->id());
	e->setPositionAndDirection(pos, dir);
	
	// 进入新space之前必须通知客户端leaveSpace
	if(e->clientMailbox())
	{
		Network::Bundle* pSendBundle = Network::Bundle::createPoolObject();
		Network::Bundle* pForwardBundle = Network::Bundle::createPoolObject();
		(*pForwardBundle).newMessage(ClientInterface::onEntityLeaveSpace);
		(*pForwardBundle) << e->id();
		NETWORK_ENTITY_MESSAGE_FORWARD_CLIENT(e->id(), (*pSendBundle), (*pForwardBundle));
		e->clientMailbox()->postMail(pSendBundle);
		Network::Bundle::reclaimPoolObject(pForwardBundle);
	}

	// 进入新的space中
	space->addEntityAndEnterWorld(e);

	Entity* nearbyMBRef = Cellapp::getSingleton().findEntity(nearbyMBRefID);
	e->onTeleportSuccess(nearbyMBRef, space->id());
	
	success = true;

	// 如果是有base的实体，需要将baseappID填入，以便在reqTeleportToCellAppCB中回调给baseapp传输结束状态
	if(e->baseMailbox())
		entityBaseappID = e->baseMailbox()->componentID();

	{
		Network::Bundle* pBundle = Network::Bundle::createPoolObject();
		(*pBundle).newMessage(CellappInterface::reqTeleportToCellAppCB);
		(*pBundle) << g_componentID << entityBaseappID;
		(*pBundle) << teleportEntityID;
		(*pBundle) << success;
		pChannel->send(pBundle);
	}
}

//-------------------------------------------------------------------------------------
void Cellapp::reqTeleportToCellAppCB(Network::Channel* pChannel, MemoryStream& s)
{
	bool success;
	ENTITY_ID nearbyMBRefID = 0, teleportEntityID = 0;
	COMPONENT_ID targetCellappID, entityBaseappID;

	s >> targetCellappID >> entityBaseappID >> teleportEntityID >> success;

	// 实体可能没有base部分，那么不需要通知baseapp
	if(entityBaseappID > 0)
	{
		Components::ComponentInfos* pInfos = Components::getSingleton().findComponent(entityBaseappID);
		if(pInfos && pInfos->pChannel)
		{
			Network::Bundle* pBundle = Network::Bundle::createPoolObject();
			(*pBundle).newMessage(BaseappInterface::onMigrationCellappEnd);
			(*pBundle) << teleportEntityID;

			if(success)
				(*pBundle) << targetCellappID;
			else
				(*pBundle) << g_componentID;

			pInfos->pChannel->send(pBundle);
		}
		else
		{
			ERROR_MSG(fmt::format("Cellapp::reqTeleportToCellAppCB: not found baseapp({}), entity({})!\n", 
				entityBaseappID, teleportEntityID));
		}
	}

	// 传送成功，我们销毁这个entity
	if(success)
	{
		destroyEntity(teleportEntityID, false);
		return;
	}

	// 某些情况下实体可能此时找不到了，例如：副本销毁了
	Entity* entity = Cellapp::getSingleton().findEntity(teleportEntityID);
	if(entity == NULL)
	{
		ERROR_MSG(fmt::format("Cellapp::reqTeleportToCellAppCB: not found reqTeleportEntity({}), lose entity!\n", 
			teleportEntityID));

		s.done();
		return;
	}

	// 传送失败了，我们需要重恢复entity
	Position3D pos;
	Direction3D dir;
	ENTITY_SCRIPT_UID entityType;
	SPACE_ID lastSpaceID = 0;

	s >> teleportEntityID >> nearbyMBRefID >> lastSpaceID;
	s >> entityType;
	s >> pos.x >> pos.y >> pos.z;
	s >> dir.dir.x >> dir.dir.y >> dir.dir.z;

	entity->changeToReal(0, s);
	entity->onTeleportFailure();

	s.done();
}

//-------------------------------------------------------------------------------------
int Cellapp::raycast(SPACE_ID spaceID, int layer, const Position3D& start, const Position3D& end, std::vector<Position3D>& hitPos)
{
	Space* pSpace = Spaces::findSpace(spaceID);
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
		if(PyArg_ParseTuple(args, "IOO", &spaceID, &pyStartPos, &pyEndPos) == -1)
		{
			PyErr_Format(PyExc_TypeError, "Cellapp::raycast: args is error!");
			PyErr_PrintEx(0);
			return 0;
		}
	}
	else if(currargsSize == 4)
	{
		if(PyArg_ParseTuple(args, "IiOO", &spaceID, &layer, &pyStartPos, &pyEndPos) == -1)
		{
			PyErr_Format(PyExc_TypeError, "Cellapp::raycast: args is error!");
			PyErr_PrintEx(0);
			return 0;
		}
	}
	else
	{
		PyErr_Format(PyExc_TypeError, "Cellapp::raycast: args is error!");
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

	if(PyArg_ParseTuple(args, "I", &flags) == -1)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::setFlags: args is error!");
		PyErr_PrintEx(0);
		return NULL;
	}

	Cellapp::getSingleton().flags(flags);
	S_Return;
}

//-------------------------------------------------------------------------------------

}
