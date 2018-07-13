// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#include "baseapp.h"
#include "proxy.h"
#include "space.h"
#include "entity.h"
#include "baseapp_interface.h"
#include "entity_remotemethod.h"
#include "archiver.h"
#include "backuper.h"
#include "initprogress_handler.h"
#include "restore_entity_handler.h"
#include "entity_messages_forward_handler.h"
#include "forward_message_over_handler.h"
#include "sync_entitystreamtemplate_handler.h"
#include "common/timestamp.h"
#include "common/kbeversion.h"
#include "network/common.h"
#include "network/tcp_packet.h"
#include "network/udp_packet.h"
#include "network/fixed_messages.h"
#include "network/encryption_filter.h"
#include "server/components.h"
#include "server/telnet_server.h"
#include "server/py_file_descriptor.h"
#include "server/sendmail_threadtasks.h"
#include "math/math.h"
#include "pyscript/py_memorystream.h"
#include "client_lib/client_interface.h"

#include "../../server/baseappmgr/baseappmgr_interface.h"
#include "../../server/cellappmgr/cellappmgr_interface.h"
#include "../../server/baseapp/baseapp_interface.h"
#include "../../server/cellapp/cellapp_interface.h"
#include "../../server/dbmgr/dbmgr_interface.h"
#include "../../server/loginapp/loginapp_interface.h"

namespace KBEngine{
	
ServerConfig g_serverConfig;
KBE_SINGLETON_INIT(Baseapp);

// 创建一个用于生成实体的字典，包含了实体所有的持久化属性和数据
PyObject* createDictDataFromPersistentStream(MemoryStream& s, const char* entityType)
{
	PyObject* pyDict = PyDict_New();
	ScriptDefModule* pScriptModule = EntityDef::findScriptModule(entityType);

	if (!pScriptModule)
	{
		ERROR_MSG(fmt::format("Baseapp::createDictDataFromPersistentStream: not found script[{}]!\n",
			entityType));

		return pyDict;
	}

	// 先将celldata中的存储属性取出
	ScriptDefModule::PROPERTYDESCRIPTION_MAP& propertyDescrs = pScriptModule->getPersistentPropertyDescriptions();
	ScriptDefModule::PROPERTYDESCRIPTION_MAP::const_iterator iter = propertyDescrs.begin();

	try
	{
		for (; iter != propertyDescrs.end(); ++iter)
		{
			PropertyDescription* propertyDescription = iter->second;
			PyObject* pyVal = NULL;
			const char* attrname = propertyDescription->getName();

			if (propertyDescription->getDataType()->type() == DATA_TYPE_ENTITY_COMPONENT)
			{
				// 如果某个实体没有cell部分， 而组件属性没有base部分则忽略
				if (!pScriptModule->hasCell())
				{
					if (!propertyDescription->hasBase())
						continue;
				}

				EntityComponentType* pEntityComponentType = ((EntityComponentType*)propertyDescription->getDataType());

				// 和dbmgr判断保持一致，如果没有持久化属性dbmgr将不会传输数据过来
				if (pEntityComponentType->pScriptDefModule()->getPersistentPropertyDescriptions().size() == 0)
					continue;

				bool hasComponentData = false;
				s >> hasComponentData;

				if (hasComponentData)
				{
					if (!propertyDescription->hasBase())
					{
						pyVal = pEntityComponentType->createCellDataFromPersistentStream(&s);
					}
					else
					{
						pyVal = ((EntityComponentDescription*)propertyDescription)->createFromPersistentStream(pScriptModule, &s);

						if (!propertyDescription->isSameType(pyVal))
						{
							if (pyVal)
							{
								Py_DECREF(pyVal);
							}

							ERROR_MSG(fmt::format("Baseapp::createDictDataFromPersistentStream: {}.{} error, set to default!\n",
								entityType, attrname));

							pyVal = propertyDescription->parseDefaultStr("");
						}
					}
				}
				else
				{
					if (!propertyDescription->hasBase())
					{
						pyVal = ((EntityComponentType*)propertyDescription->getDataType())->createCellDataFromPersistentStream(NULL);
					}
					else
					{

						ERROR_MSG(fmt::format("Baseapp::createDictDataFromPersistentStream: {}.{} error, set to default!\n",
							entityType, attrname));

						pyVal = propertyDescription->parseDefaultStr("");
					}
				}
			}
			else
			{
				pyVal = propertyDescription->createFromPersistentStream(&s);

				if (!propertyDescription->isSameType(pyVal))
				{
					if (pyVal)
					{
						Py_DECREF(pyVal);
					}

					ERROR_MSG(fmt::format("Baseapp::createDictDataFromPersistentStream: {}.{} error, set to default!\n",
						entityType, attrname));

					pyVal = propertyDescription->parseDefaultStr("");
				}
			}

			PyDict_SetItemString(pyDict, attrname, pyVal);
			Py_DECREF(pyVal);
		}

		if (pScriptModule->hasCell())
		{
#ifdef CLIENT_NO_FLOAT
			int32 v1, v2, v3;
			int32 vv1, vv2, vv3;
#else
			float v1, v2, v3;
			float vv1, vv2, vv3;
#endif

			s >> v1 >> v2 >> v3;
			s >> vv1 >> vv2 >> vv3;

			PyObject* position = PyTuple_New(3);
			PyTuple_SET_ITEM(position, 0, PyFloat_FromDouble((float)v1));
			PyTuple_SET_ITEM(position, 1, PyFloat_FromDouble((float)v2));
			PyTuple_SET_ITEM(position, 2, PyFloat_FromDouble((float)v3));

			PyObject* direction = PyTuple_New(3);
			PyTuple_SET_ITEM(direction, 0, PyFloat_FromDouble((float)vv1));
			PyTuple_SET_ITEM(direction, 1, PyFloat_FromDouble((float)vv2));
			PyTuple_SET_ITEM(direction, 2, PyFloat_FromDouble((float)vv3));

			PyDict_SetItemString(pyDict, "position", position);
			PyDict_SetItemString(pyDict, "direction", direction);

			Py_DECREF(position);
			Py_DECREF(direction);
		}
	}
	catch (MemoryStreamException & e)
	{
		e.PrintPosError();

		for (; iter != propertyDescrs.end(); ++iter)
		{
			PropertyDescription* propertyDescription = iter->second;

			const char* attrname = propertyDescription->getName();

			ERROR_MSG(fmt::format("Baseapp::createDictDataFromPersistentStream: set({}.{}) to default!\n",
				entityType, attrname));

			PyObject* pyVal = propertyDescription->parseDefaultStr("");

			PyDict_SetItemString(pyDict, attrname, pyVal);
			Py_DECREF(pyVal);
		}

		PyObject* position = PyTuple_New(3);
		PyTuple_SET_ITEM(position, 0, PyFloat_FromDouble(0.f));
		PyTuple_SET_ITEM(position, 1, PyFloat_FromDouble(0.f));
		PyTuple_SET_ITEM(position, 2, PyFloat_FromDouble(0.f));

		PyObject* direction = PyTuple_New(3);
		PyTuple_SET_ITEM(direction, 0, PyFloat_FromDouble(0.f));
		PyTuple_SET_ITEM(direction, 1, PyFloat_FromDouble(0.f));
		PyTuple_SET_ITEM(direction, 2, PyFloat_FromDouble(0.f));

		PyDict_SetItemString(pyDict, "position", position);
		PyDict_SetItemString(pyDict, "direction", direction);

		Py_DECREF(position);
		Py_DECREF(direction);
	}

	return pyDict;
}

//-------------------------------------------------------------------------------------
Baseapp::Baseapp(Network::EventDispatcher& dispatcher, 
			 Network::NetworkInterface& ninterface, 
			 COMPONENT_TYPE componentType,
			 COMPONENT_ID componentID):
	EntityApp<Entity>(dispatcher, ninterface, componentType, componentID),
	loopCheckTimerHandle_(),
	pBaseAppData_(NULL),
	pendingLoginMgr_(ninterface),
	forward_messagebuffer_(ninterface),
	pBackuper_(),
	numProxices_(0),
	pTelnetServer_(NULL),
	pRestoreEntityHandlers_(),
	pResmgrTimerHandle_(),
	pInitProgressHandler_(NULL),
	flags_(APP_FLAGS_NONE),
	pBundleImportEntityDefDatas_(NULL)
{
	KBEngine::Network::MessageHandlers::pMainMessageHandlers = &BaseappInterface::messageHandlers;

	// hook entitycallcall
	static EntityCallAbstract::EntityCallCallHookFunc entitycallCallHookFunc = std::tr1::bind(&Baseapp::createEntityCallCallEntityRemoteMethod, this,
		std::tr1::placeholders::_1, std::tr1::placeholders::_2);

	EntityCallAbstract::setEntityCallCallHookFunc(&entitycallCallHookFunc);
}

//-------------------------------------------------------------------------------------
Baseapp::~Baseapp()
{
	// 不需要主动释放
	pInitProgressHandler_ = NULL;

	EntityCallAbstract::resetCallHooks();
}

//-------------------------------------------------------------------------------------	
bool Baseapp::canShutdown()
{
	if (getEntryScript().get() && PyObject_HasAttrString(getEntryScript().get(), "onReadyForShutDown") > 0)
	{
		// 所有脚本都加载完毕
		PyObject* pyResult = PyObject_CallMethod(getEntryScript().get(),
			const_cast<char*>("onReadyForShutDown"),
			const_cast<char*>(""));

		if (pyResult != NULL)
		{
			bool isReady = (pyResult == Py_True);
			Py_DECREF(pyResult);

			if (isReady)
				return true;
			else
				return false;
		}
		else
		{
			SCRIPT_ERROR_CHECK();
			return false;
		}
	}
	
	Components::COMPONENTS& cellapp_components = Components::getSingleton().getComponents(CELLAPP_TYPE);
	if(cellapp_components.size() > 0)
	{
		std::string s;
		for(size_t i=0; i<cellapp_components.size(); ++i)
		{
			s += fmt::format("{}, ", cellapp_components[i].cid);
		}

		INFO_MSG(fmt::format("Baseapp::canShutdown(): Waiting for cellapp[{}] destruction!\n", 
			s));

		return false;
	}

	int count = 0;
	Entities<Entity>::ENTITYS_MAP& entities =  this->pEntities()->getEntities();
	Entities<Entity>::ENTITYS_MAP::iterator iter = entities.begin();
	for(; iter != entities.end(); ++iter)
	{
		//if(static_cast<Entity*>(iter->second.get())->hasDB())
		{
			count++;
		}
	}

	if(count > 0)
	{
		lastShutdownFailReason_ = "destroyHasDBBases";
		INFO_MSG(fmt::format("Baseapp::canShutdown(): Wait for the entity's into the database! The remaining {}.\n", 
			count));

		return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------	
void Baseapp::onShutdownBegin()
{
	EntityApp<Entity>::onShutdownBegin();

	// 通知脚本
	SCOPED_PROFILE(SCRIPTCALL_PROFILE);
	SCRIPT_OBJECT_CALL_ARGS1(getEntryScript().get(), const_cast<char*>("onBaseAppShutDown"), 
		const_cast<char*>("i"), 0, false);

	pRestoreEntityHandlers_.clear();
}

//-------------------------------------------------------------------------------------	
void Baseapp::onShutdown(bool first)
{
	EntityApp<Entity>::onShutdown(first);

	if(first)
	{
		// 通知脚本
		SCOPED_PROFILE(SCRIPTCALL_PROFILE);
		SCRIPT_OBJECT_CALL_ARGS1(getEntryScript().get(), const_cast<char*>("onBaseAppShutDown"), 
			const_cast<char*>("i"), 1, false);
	}

	Components::COMPONENTS& cellapp_components = Components::getSingleton().getComponents(CELLAPP_TYPE);
	if(cellapp_components.size() == 0)
	{
		int count = g_serverConfig.getBaseApp().perSecsDestroyEntitySize;
		Entities<Entity>::ENTITYS_MAP& entities =  this->pEntities()->getEntities();

		while(count > 0 && entities.size() > 0)
		{
			std::vector<ENTITY_ID> vecs;
			
			Entities<Entity>::ENTITYS_MAP::iterator iter = entities.begin();
			for(; iter != entities.end(); ++iter)
			{
				//if(static_cast<Entity*>(iter->second.get())->hasDB() && 
				//	static_cast<Entity*>(iter->second.get())->cellEntityCall() == NULL)
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
	}
}

//-------------------------------------------------------------------------------------	
void Baseapp::onShutdownEnd()
{
	EntityApp<Entity>::onShutdownEnd();

	// 通知脚本
	SCOPED_PROFILE(SCRIPTCALL_PROFILE);
	SCRIPT_OBJECT_CALL_ARGS1(getEntryScript().get(), const_cast<char*>("onBaseAppShutDown"), 
		const_cast<char*>("i"), 2, false);
}

//-------------------------------------------------------------------------------------		
bool Baseapp::initializeWatcher()
{
	ProfileVal::setWarningPeriod(stampsPerSecond() / g_kbeSrvConfig.gameUpdateHertz());

	WATCH_OBJECT("numProxices", this, &Baseapp::numProxices);
	WATCH_OBJECT("numClients", this, &Baseapp::numClients);
	WATCH_OBJECT("load", this, &Baseapp::_getLoad);
	WATCH_OBJECT("stats/runningTime", &runningTime);
	return EntityApp<Entity>::initializeWatcher();
}

//-------------------------------------------------------------------------------------
bool Baseapp::installPyModules()
{
	Entity::installScript(getScript().getModule());
	Proxy::installScript(getScript().getModule());
	Space::installScript(getScript().getModule());
	EntityComponent::installScript(getScript().getModule());
	GlobalDataClient::installScript(getScript().getModule());

	registerScript(Entity::getScriptType());
	registerScript(Proxy::getScriptType());
	registerScript(EntityComponent::getScriptType());

	// 将app标记注册到脚本
	std::map<uint32, std::string> flagsmaps = createAppFlagsMaps();
	std::map<uint32, std::string>::iterator fiter = flagsmaps.begin();
	for (; fiter != flagsmaps.end(); ++fiter)
	{
		if (PyModule_AddIntConstant(getScript().getModule(), fiter->second.c_str(), fiter->first))
		{
			ERROR_MSG(fmt::format("Baseapp::onInstallPyModules: Unable to set KBEngine.{}.\n", fiter->second));
		}
	}

	// 注册创建entity的方法到py 
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(),		time,							__py_gametime,												METH_VARARGS,			0);
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(),		createEntity,					__py_createEntity,											METH_VARARGS,			0);
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(),		createEntityLocally,			__py_createEntity,											METH_VARARGS,			0);
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(), 		createEntityAnywhere,			__py_createEntityAnywhere,									METH_VARARGS,			0);
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(),		createEntityRemotely,			__py_createEntityRemotely,									METH_VARARGS,			0);
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(), 		createEntityFromDBID,			__py_createEntityFromDBID,									METH_VARARGS,			0);
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(), 		createEntityAnywhereFromDBID,	__py_createEntityAnywhereFromDBID,							METH_VARARGS,			0);
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(),		createEntityRemotelyFromDBID,	__py_createEntityRemotelyFromDBID,							METH_VARARGS,			0);
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(), 		executeRawDatabaseCommand,		__py_executeRawDatabaseCommand,								METH_VARARGS,			0);
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(), 		quantumPassedPercent,			__py_quantumPassedPercent,									METH_VARARGS,			0);
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(), 		charge,							__py_charge,												METH_VARARGS,			0);
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(), 		registerReadFileDescriptor,		PyFileDescriptor::__py_registerReadFileDescriptor,			METH_VARARGS,			0);
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(), 		registerWriteFileDescriptor,	PyFileDescriptor::__py_registerWriteFileDescriptor,			METH_VARARGS,			0);
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(), 		deregisterReadFileDescriptor,	PyFileDescriptor::__py_deregisterReadFileDescriptor,		METH_VARARGS,			0);
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(), 		deregisterWriteFileDescriptor,	PyFileDescriptor::__py_deregisterWriteFileDescriptor,		METH_VARARGS,			0);
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(),		reloadScript,					__py_reloadScript,											METH_VARARGS,			0);
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(),		isShuttingDown,					__py_isShuttingDown,										METH_VARARGS,			0);
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(),		address,						__py_address,												METH_VARARGS,			0);
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(),		deleteEntityByDBID,				__py_deleteEntityByDBID,									METH_VARARGS,			0);
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(),		lookUpEntityByDBID,				__py_lookUpEntityByDBID,									METH_VARARGS,			0);
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(), 		setAppFlags,					__py_setFlags,												METH_VARARGS,			0);
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(), 		getAppFlags,					__py_getFlags,												METH_VARARGS,			0);
		
	return EntityApp<Entity>::installPyModules();
}

//-------------------------------------------------------------------------------------
void Baseapp::onInstallPyModules()
{
	// 添加globalData, globalBases支持
	pBaseAppData_ = new GlobalDataClient(DBMGR_TYPE, GlobalDataServer::BASEAPP_DATA);
	registerPyObjectToScript("baseAppData", pBaseAppData_);

	if(PyModule_AddIntMacro(this->getScript().getModule(), LOG_ON_REJECT))
	{
		ERROR_MSG( "Baseapp::onInstallPyModules: Unable to set KBEngine.LOG_ON_REJECT.\n");
	}

	if(PyModule_AddIntMacro(this->getScript().getModule(), LOG_ON_ACCEPT))
	{
		ERROR_MSG( "Baseapp::onInstallPyModules: Unable to set KBEngine.LOG_ON_ACCEPT.\n");
	}

	if(PyModule_AddIntMacro(this->getScript().getModule(), LOG_ON_WAIT_FOR_DESTROY))
	{
		ERROR_MSG( "Baseapp::onInstallPyModules: Unable to set KBEngine.LOG_ON_WAIT_FOR_DESTROY.\n");
	}
}

//-------------------------------------------------------------------------------------
bool Baseapp::uninstallPyModules()
{	
	if(g_kbeSrvConfig.getBaseApp().profiles.open_pyprofile)
	{
		script::PyProfile::stop("kbengine");

		char buf[MAX_BUF];
		kbe_snprintf(buf, MAX_BUF, "baseapp%u.pyprofile", startGroupOrder_);
		script::PyProfile::dump("kbengine", buf);
		script::PyProfile::remove("kbengine");
	}

	unregisterPyObjectToScript("baseAppData");
	S_RELEASE(pBaseAppData_); 

	Entity::uninstallScript();
	Proxy::uninstallScript();
	EntityComponent::uninstallScript();
	GlobalDataClient::uninstallScript();

	return EntityApp<Entity>::uninstallPyModules();
}

//-------------------------------------------------------------------------------------
PyObject* Baseapp::__py_gametime(PyObject* self, PyObject* args)
{
	return PyLong_FromUnsignedLong(Baseapp::getSingleton().time());
}

//-------------------------------------------------------------------------------------
PyObject* Baseapp::__py_quantumPassedPercent(PyObject* self, PyObject* args)
{
	return PyLong_FromLong(Baseapp::getSingleton().tickPassedPercent());
}

//-------------------------------------------------------------------------------------
void Baseapp::onUpdateLoad()
{
	Network::Channel* pChannel = Components::getSingleton().getBaseappmgrChannel();
	if(pChannel != NULL)
	{
		Network::Bundle* pBundle = Network::Bundle::createPoolObject();
		(*pBundle).newMessage(BaseappmgrInterface::updateBaseapp);
		BaseappmgrInterface::updateBaseappArgs5::staticAddToBundle((*pBundle), 
			componentID_, (ENTITY_ID)(pEntities_->getEntities().size() - numProxices()), (ENTITY_ID)numProxices(), getLoad(), flags_);

		pChannel->send(pBundle);
	}
}

//-------------------------------------------------------------------------------------
bool Baseapp::run()
{
	return EntityApp<Entity>::run();
}

//-------------------------------------------------------------------------------------
void Baseapp::handleTimeout(TimerHandle handle, void * arg)
{
	switch (reinterpret_cast<uintptr>(arg))
	{
		case TIMEOUT_CHECK_STATUS:
			this->handleCheckStatusTick();
			return;
		default:
			break;
	}

	EntityApp<Entity>::handleTimeout(handle, arg);
}

//-------------------------------------------------------------------------------------
void Baseapp::handleCheckStatusTick()
{
	pendingLoginMgr_.process();
}

//-------------------------------------------------------------------------------------
void Baseapp::handleGameTick()
{
	AUTO_SCOPED_PROFILE("gameTick");

	// 一定要在最前面
	updateLoad();

	EntityApp<Entity>::handleGameTick();

	handleBackup();
	handleArchive();
}

//-------------------------------------------------------------------------------------
void Baseapp::handleBackup()
{
	AUTO_SCOPED_PROFILE("backup");
	pBackuper_->tick();
}

//-------------------------------------------------------------------------------------
void Baseapp::handleArchive()
{
	AUTO_SCOPED_PROFILE("archive");
	pArchiver_->tick();
}

//-------------------------------------------------------------------------------------
bool Baseapp::initializeBegin()
{
	return true;
}

//-------------------------------------------------------------------------------------
bool Baseapp::initializeEnd()
{
	// 添加一个timer， 每秒检查一些状态
	loopCheckTimerHandle_ = this->dispatcher().addTimer(1000000, this,
							reinterpret_cast<void *>(TIMEOUT_CHECK_STATUS));

	if(Resmgr::respool_checktick > 0)
	{
		pResmgrTimerHandle_ = this->dispatcher().addTimer(int(Resmgr::respool_checktick * 1000000),
			Resmgr::getSingletonPtr(), NULL);

		INFO_MSG(fmt::format("Baseapp::initializeEnd: started resmgr tick({}s)!\n", 
			Resmgr::respool_checktick));
	}

	pBackuper_.reset(new Backuper());
	pArchiver_.reset(new Archiver());

	new SyncEntityStreamTemplateHandler(this->networkInterface());

	// 如果需要pyprofile则在此处安装
	// 结束时卸载并输出结果
	if(g_kbeSrvConfig.getBaseApp().profiles.open_pyprofile)
	{
		script::PyProfile::start("kbengine");
	}

	pTelnetServer_ = new TelnetServer(&this->dispatcher(), &this->networkInterface());
	pTelnetServer_->pScript(&this->getScript());

	bool ret = pTelnetServer_->start(g_kbeSrvConfig.getBaseApp().telnet_passwd, 
		g_kbeSrvConfig.getBaseApp().telnet_deflayer, 
		g_kbeSrvConfig.getBaseApp().telnet_port);

	Components::getSingleton().extraData4(pTelnetServer_->port());
	return ret;
}

//-------------------------------------------------------------------------------------
void Baseapp::finalise()
{
	if(pTelnetServer_)
	{
		pTelnetServer_->stop();
		SAFE_RELEASE(pTelnetServer_);
	}

	pRestoreEntityHandlers_.clear();
	loopCheckTimerHandle_.cancel();
	pResmgrTimerHandle_.cancel();
	forward_messagebuffer_.clear();

	if (pBundleImportEntityDefDatas_)
	{
		Network::Bundle::reclaimPoolObject(pBundleImportEntityDefDatas_);
		pBundleImportEntityDefDatas_ = NULL;
	}

	EntityApp<Entity>::finalise();
}

//-------------------------------------------------------------------------------------
void Baseapp::onCellAppDeath(Network::Channel * pChannel)
{
	if(pChannel && pChannel->isExternal())
		return;
	
	if(shuttingdown_ != SHUTDOWN_STATE_STOP)
	{
		return;
	}
	
	PyObject* pyarg = PyTuple_New(1);

	PyObject* pyobj = PyTuple_New(2);

	const Network::Address& addr = pChannel->pEndPoint()->addr();
	PyTuple_SetItem(pyobj, 0, PyLong_FromUnsignedLong(addr.ip));
	PyTuple_SetItem(pyobj, 1, PyLong_FromUnsignedLong(addr.port));
	PyTuple_SetItem(pyarg, 0, pyobj);

	SCRIPT_ERROR_CHECK();

	{
		SCOPED_PROFILE(SCRIPTCALL_PROFILE);
		PyObject* pyResult = PyObject_CallMethod(getEntryScript().get(), 
											const_cast<char*>("onCellAppDeath"), 
											const_cast<char*>("O"), 
											pyarg);

		Py_DECREF(pyarg);

		if(pyResult != NULL)
			Py_DECREF(pyResult);
		else
			SCRIPT_ERROR_CHECK();
	}

	RestoreEntityHandler* pRestoreEntityHandler = new RestoreEntityHandler(pChannel->componentID(), this->networkInterface());
	Entities<Entity>::ENTITYS_MAP& entitiesMap = pEntities_->getEntities();
	Entities<Entity>::ENTITYS_MAP::const_iterator iter = entitiesMap.begin();
	while (iter != entitiesMap.end())
	{
		Entity* pEntity = static_cast<Entity*>(iter->second.get());
		
		EntityCall* cell = pEntity->cellEntityCall();
		if(cell && cell->componentID() == pChannel->componentID())
		{
			S_RELEASE(cell);
			pEntity->cellEntityCall(NULL);
			pEntity->installCellDataAttr(pEntity->getCellData());
			pEntity->onCellAppDeath();
			pRestoreEntityHandler->pushEntity(pEntity->id());
		}

		iter++;
	}

	pRestoreEntityHandlers_.push_back(KBEShared_ptr< RestoreEntityHandler >(pRestoreEntityHandler));
}

//-------------------------------------------------------------------------------------
void Baseapp::onRequestRestoreCB(Network::Channel* pChannel, KBEngine::MemoryStream& s)
{
	if(pChannel->isExternal())
		return;
	
	COMPONENT_ID cid, source_cid;
	bool canRestore = true;

	s >> cid >> source_cid >> canRestore;

	DEBUG_MSG(fmt::format("Baseapp::onRequestRestoreCB: cid={}, source_cid={}, canRestore={}, channel={}.\n", 
		cid, source_cid, canRestore, pChannel->c_str()));

	std::vector< KBEShared_ptr< RestoreEntityHandler > >::iterator resiter = pRestoreEntityHandlers_.begin();
	for(; resiter != pRestoreEntityHandlers_.end(); ++resiter)
	{
		if((*resiter)->cellappID() == source_cid)
		{
			(*resiter)->canRestore(canRestore);
			break;
		}
	}
}

//-------------------------------------------------------------------------------------
void Baseapp::onRestoreEntitiesOver(RestoreEntityHandler* pRestoreEntityHandler)
{
	std::vector< KBEShared_ptr< RestoreEntityHandler > >::iterator resiter = pRestoreEntityHandlers_.begin();
	for(; resiter != pRestoreEntityHandlers_.end(); ++resiter)
	{
		if((*resiter).get() == pRestoreEntityHandler)
		{
			pRestoreEntityHandlers_.erase(resiter);
			return;
		}
	}
}

//-------------------------------------------------------------------------------------
void Baseapp::onRestoreSpaceCellFromOtherBaseapp(Network::Channel* pChannel, KBEngine::MemoryStream& s)
{
	if(pChannel->isExternal())
		return;
	
	COMPONENT_ID baseappID = 0, cellappID = 0;
	SPACE_ID spaceID = 0;
	ENTITY_ID spaceEntityID = 0;
	bool destroyed = false;
	ENTITY_SCRIPT_UID utype = 0;

	s >> baseappID >> cellappID >> spaceID >> spaceEntityID >> utype >> destroyed;

	INFO_MSG(fmt::format("Baseapp::onRestoreSpaceCellFromOtherBaseapp: baseappID={0}, cellappID={5}, spaceID={1}, spaceEntityID={2}, destroyed={3}, "
		"restoreEntityHandlers({4})\n",
		baseappID, spaceID, spaceEntityID, destroyed, pRestoreEntityHandlers_.size(), cellappID));

	std::vector< KBEShared_ptr< RestoreEntityHandler > >::iterator resiter = pRestoreEntityHandlers_.begin();
	for(; resiter != pRestoreEntityHandlers_.end(); ++resiter)
	{
		(*resiter)->onRestoreSpaceCellFromOtherBaseapp(baseappID, cellappID, spaceID, spaceEntityID, utype, destroyed);
	}
}

//-------------------------------------------------------------------------------------
void Baseapp::onChannelDeregister(Network::Channel * pChannel)
{
	ENTITY_ID pid = pChannel->proxyID();

	// 如果是cellapp死亡了
	if(pChannel->isInternal())
	{
		Components::ComponentInfos* cinfo = Components::getSingleton().findComponent(pChannel);
		if(cinfo)
		{
			if(cinfo->componentType == CELLAPP_TYPE)
			{
				onCellAppDeath(pChannel);
			}
		}
	}

	EntityApp<Entity>::onChannelDeregister(pChannel);
	
	// 有关联entity的客户端退出则需要设置entity的client
	if(pid > 0)
	{
		Proxy* proxy = static_cast<Proxy*>(this->findEntity(pid));
		if(proxy)
		{
			proxy->onClientDeath();
		}
	}
}

//-------------------------------------------------------------------------------------
void Baseapp::onGetEntityAppFromDbmgr(Network::Channel* pChannel, int32 uid, std::string& username, 
						COMPONENT_TYPE componentType, COMPONENT_ID componentID, COMPONENT_ORDER globalorderID, COMPONENT_ORDER grouporderID,
						uint32 intaddr, uint16 intport, uint32 extaddr, uint16 extport, std::string& extaddrEx)
{
	if(pChannel->isExternal())
		return;

	Components::ComponentInfos* cinfos = Components::getSingleton().findComponent((
		KBEngine::COMPONENT_TYPE)componentType, uid, componentID);

	if(cinfos)
	{
		if(cinfos->pIntAddr->ip != intaddr || cinfos->pIntAddr->port != intport)
		{
			ERROR_MSG(fmt::format("Baseapp::onGetEntityAppFromDbmgr: Illegal app(uid:{0}, username:{1}, componentType:{2}, "
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
	KBE_ASSERT(Components::getSingleton().getDbmgr() != NULL);
	
	cinfos = 
		Components::getSingleton().findComponent(tcomponentType, uid, componentID);

	if (cinfos == NULL)
	{
		ERROR_MSG(fmt::format("Baseapp::onGetEntityAppFromDbmgr: Illegal app(uid:{0}, username:{1}, componentType:{2}, "
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
		BaseappInterface::onRegisterNewAppArgs11::staticAddToBundle((*pBundle), 
			getUserUID(), getUsername(), BASEAPP_TYPE, componentID_, startGlobalOrder_, startGroupOrder_,
			this->networkInterface().intTcpAddr().ip, this->networkInterface().intTcpAddr().port,
			this->networkInterface().extTcpAddr().ip, this->networkInterface().extTcpAddr().port, g_kbeSrvConfig.getConfig().externalAddress);
		break;
	case CELLAPP_TYPE:
		(*pBundle).newMessage(CellappInterface::onRegisterNewApp);
		CellappInterface::onRegisterNewAppArgs11::staticAddToBundle((*pBundle), 
			getUserUID(), getUsername(), BASEAPP_TYPE, componentID_, startGlobalOrder_, startGroupOrder_,
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
Entity* Baseapp::onCreateEntity(PyObject* pyEntity, ScriptDefModule* sm, ENTITY_ID eid)
{
	if(PyType_IsSubtype(sm->getScriptType(), Proxy::getScriptType()))
	{
		return new(pyEntity) Proxy(eid, sm);
	}
	else if (PyType_IsSubtype(sm->getScriptType(), Space::getScriptType()))
	{
		return new(pyEntity) Space(eid, sm);
	}

	return EntityApp<Entity>::onCreateEntity(pyEntity, sm, eid);
}

//-------------------------------------------------------------------------------------
PyObject* Baseapp::__py_createEntity(PyObject* self, PyObject* args)
{
	int argCount = (int)PyTuple_Size(args);
	PyObject* params = NULL;
	char* entityType = NULL;
	int ret = -1;

	if(argCount == 2)
		ret = PyArg_ParseTuple(args, "s|O", &entityType, &params);
	else
		ret = PyArg_ParseTuple(args, "s", &entityType);

	if(entityType == NULL || ret == -1)
	{
		PyErr_Format(PyExc_AssertionError, "Baseapp::createEntity: args error!");
		PyErr_PrintEx(0);
		return NULL;
	}
	
	PyObject* e = Baseapp::getSingleton().createEntity(entityType, params);
	if(e != NULL)
		Py_INCREF(e);

	return e;
}

//-------------------------------------------------------------------------------------
PyObject* Baseapp::__py_createEntityAnywhere(PyObject* self, PyObject* args)
{
	int argCount = (int)PyTuple_Size(args);
	PyObject* params = NULL, *pyCallback = NULL;
	char* entityType = NULL;
	int ret = -1;

	switch(argCount)
	{
	case 3:
		ret = PyArg_ParseTuple(args, "s|O|O", &entityType, &params, &pyCallback);
		break;
	case 2:
		ret = PyArg_ParseTuple(args, "s|O", &entityType, &params);
		break;
	default:
		ret = PyArg_ParseTuple(args, "s", &entityType);
	};


	if(entityType == NULL || ret == -1)
	{
		PyErr_Format(PyExc_AssertionError, "Baseapp::createEntityAnywhere: args error!");
		PyErr_PrintEx(0);
		return NULL;
	}

	if (EntityDef::findScriptModule(entityType) == NULL)
	{
		PyErr_Format(PyExc_AssertionError, "Baseapp::createEntityAnywhere: entityType(%s) error!", entityType);
		PyErr_PrintEx(0);
		return NULL;
	}

	if(!PyCallable_Check(pyCallback))
		pyCallback = NULL;

	Baseapp::getSingleton().createEntityAnywhere(entityType, params, pyCallback);
	S_Return;
}

//-------------------------------------------------------------------------------------
PyObject* Baseapp::__py_createEntityRemotely(PyObject* self, PyObject* args)
{
	int argCount = (int)PyTuple_Size(args);
	PyObject* params = NULL, *pyCallback = NULL, *pyEntityCall = NULL;
	char* entityType = NULL;
	int ret = -1;

	switch (argCount)
	{
	case 4:
		ret = PyArg_ParseTuple(args, "s|O|O|O", &entityType, &params, &pyEntityCall, &pyCallback);
		break;
	case 3:
		ret = PyArg_ParseTuple(args, "s|O|O", &entityType, &pyEntityCall, &params);
		break;
	default:
		ret = PyArg_ParseTuple(args, "s|O", &entityType, &pyEntityCall);
	};

	if (entityType == NULL || ret == -1)
	{
		PyErr_Format(PyExc_AssertionError, "Baseapp::createEntityRemotely: args error!");
		PyErr_PrintEx(0);
		return NULL;
	}

	if (EntityDef::findScriptModule(entityType) == NULL)
	{
		PyErr_Format(PyExc_AssertionError, "Baseapp::createEntityRemotely: entityType(%s) error!", entityType);
		PyErr_PrintEx(0);
		return NULL;
	}

	if (!PyCallable_Check(pyCallback))
		pyCallback = NULL;

	if (pyEntityCall == NULL || !PyObject_TypeCheck(pyEntityCall, EntityCall::getScriptType()))
	{
		PyErr_Format(PyExc_TypeError, "create %s arg2 is not baseEntityCall!",
			entityType);

		PyErr_PrintEx(0);
		return 0;
	}

	EntityCallAbstract* baseEntityCall = static_cast<EntityCallAbstract*>(pyEntityCall);
	if (baseEntityCall->type() != ENTITYCALL_TYPE_BASE)
	{
		PyErr_Format(PyExc_TypeError, "create %s args2 not is a direct baseEntityCall!",
			entityType);

		PyErr_PrintEx(0);
		return 0;
	}

	Baseapp::getSingleton().createEntityRemotely(entityType, baseEntityCall->componentID(), params, pyCallback);
	S_Return;
}

//-------------------------------------------------------------------------------------
PyObject* Baseapp::__py_createEntityFromDBID(PyObject* self, PyObject* args)
{
	int argCount = (int)PyTuple_Size(args);
	PyObject* pyCallback = NULL;
	wchar_t* wEntityType = NULL;
	char* entityType = NULL;
	int ret = -1;
	DBID dbid = 0;
	PyObject* pyEntityType = NULL;
	PyObject* pyDBInterfaceName = NULL;
	std::string dbInterfaceName = "default";

	switch(argCount)
	{
	case 4:
		{
			ret = PyArg_ParseTuple(args, "O|K|O|O", &pyEntityType, &dbid, &pyCallback, &pyDBInterfaceName);
			break;
		}
	case 3:
		{
			ret = PyArg_ParseTuple(args, "O|K|O", &pyEntityType, &dbid, &pyCallback);
			break;
		}
	case 2:
		{
			ret = PyArg_ParseTuple(args, "O|K", &pyEntityType, &dbid);
			break;
		}
	default:
		{
			PyErr_Format(PyExc_AssertionError, "%s: args require 2 or 3 args, gived %d!\n",
				__FUNCTION__, argCount);	
			PyErr_PrintEx(0);
			return NULL;
		}
	};

	if (ret == -1)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::createEntityFromDBID: args error!");
		PyErr_PrintEx(0);
		return NULL;
	}

	if (pyDBInterfaceName)
	{
		wchar_t* PyUnicode_AsWideCharStringRet0 = PyUnicode_AsWideCharString(pyDBInterfaceName, NULL);
		char* ccattr = strutil::wchar2char(PyUnicode_AsWideCharStringRet0);
		dbInterfaceName = ccattr;
		PyMem_Free(PyUnicode_AsWideCharStringRet0);
		free(ccattr);

		DBInterfaceInfo* pDBInterfaceInfo = g_kbeSrvConfig.dbInterface(dbInterfaceName);
		if (pDBInterfaceInfo->isPure)
		{
			ERROR_MSG(fmt::format("KBEngine::createEntityFromDBID: dbInterface({}) is a pure database does not support Entity! "
				"kbengine[_defs].xml->dbmgr->databaseInterfaces->*->pure\n",
				dbInterfaceName));

			return NULL;
		}

		int dbInterfaceIndex = pDBInterfaceInfo->index;
		if (dbInterfaceIndex < 0)
		{
			PyErr_Format(PyExc_TypeError, "Baseapp::createEntityFromDBID: not found dbInterface(%s)!",
				dbInterfaceName.c_str());

			PyErr_PrintEx(0);
			return NULL;
		}
	}

	if(pyEntityType)
	{
		wEntityType = PyUnicode_AsWideCharString(pyEntityType, NULL);	
		if(wEntityType)
		{
			entityType = strutil::wchar2char(wEntityType);									
			PyMem_Free(wEntityType);
		}
		else
		{
			SCRIPT_ERROR_CHECK();
		}
	}

	if(entityType == NULL || strlen(entityType) <= 0 || ret == -1)
	{
		PyErr_Format(PyExc_AssertionError, "Baseapp::createEntityFromDBID: args error, entityType=%s!", 
			(entityType ? entityType : "NULL"));

		PyErr_PrintEx(0);

		if(entityType)
			free(entityType);

		return NULL;
	}

	if (EntityDef::findScriptModule(entityType) == NULL)
	{
		PyErr_Format(PyExc_AssertionError, "Baseapp::createEntityFromDBID: entityType(%s) error!", entityType);
		PyErr_PrintEx(0);
		free(entityType);
		return NULL;
	}

	if(dbid <= 0)
	{
		PyErr_Format(PyExc_AssertionError, "Baseapp::createEntityFromDBID: dbid error!");
		PyErr_PrintEx(0);
		free(entityType);
		return NULL;
	}

	if(pyCallback && !PyCallable_Check(pyCallback))
	{
		pyCallback = NULL;

		PyErr_Format(PyExc_AssertionError, "Baseapp::createEntityFromDBID: callback error!");
		PyErr_PrintEx(0);
		free(entityType);
		return NULL;
	}

	Baseapp::getSingleton().createEntityFromDBID(entityType, dbid, pyCallback, dbInterfaceName);

	free(entityType);
	S_Return;
}

//-------------------------------------------------------------------------------------
void Baseapp::createEntityFromDBID(const char* entityType, DBID dbid, PyObject* pyCallback, const std::string& dbInterfaceName)
{
	Components::ComponentInfos* dbmgrinfos = Components::getSingleton().getDbmgr();
	if(dbmgrinfos == NULL || dbmgrinfos->pChannel == NULL || dbmgrinfos->cid == 0)
	{
		PyErr_Format(PyExc_AssertionError, "Baseapp::createEntityFromDBID: not found dbmgr!\n");
		PyErr_PrintEx(0);
		return;
	}

	DBInterfaceInfo* pDBInterfaceInfo = g_kbeSrvConfig.dbInterface(dbInterfaceName);
	if (pDBInterfaceInfo->isPure)
	{
		ERROR_MSG(fmt::format("Baseapp::createEntityFromDBID: dbInterface({}) is a pure database does not support Entity! "
			"kbengine[_defs].xml->dbmgr->databaseInterfaces->*->pure\n",
			dbInterfaceName));

		return;
	}

	int dbInterfaceIndex = pDBInterfaceInfo->index;
	if (dbInterfaceIndex < 0)
	{
		PyErr_Format(PyExc_TypeError, "Baseapp::createEntityFromDBID: not found dbInterface(%s)!", 
			dbInterfaceName.c_str());

		PyErr_PrintEx(0);
		return;
	}

	CALLBACK_ID callbackID = 0;
	if(pyCallback != NULL)
	{
		callbackID = callbackMgr().save(pyCallback);
	}

	Network::Bundle* pBundle = Network::Bundle::createPoolObject();
	pBundle->newMessage(DbmgrInterface::queryEntity);

	ENTITY_ID entityID = idClient_.alloc();
	KBE_ASSERT(entityID > 0);

	DbmgrInterface::queryEntityArgs7::staticAddToBundle((*pBundle), 
		dbInterfaceIndex, g_componentID, 0, dbid, entityType, callbackID, entityID);
	
	dbmgrinfos->pChannel->send(pBundle);
}

//-------------------------------------------------------------------------------------
void Baseapp::onCreateEntityFromDBIDCallback(Network::Channel* pChannel, KBEngine::MemoryStream& s)
{
	if(pChannel->isExternal())
		return;
	
	std::string entityType;
	DBID dbid;
	CALLBACK_ID callbackID;
	bool success = false;
	bool wasActive = false;
	ENTITY_ID entityID;
	COMPONENT_ID wasActiveCID = 0;
	ENTITY_ID wasActiveEntityID = 0;
	uint16 dbInterfaceIndex = 0;
	COMPONENT_ID createToComponentID = 0;

	s >> createToComponentID;
	s >> dbInterfaceIndex;
	s >> entityType;
	s >> dbid;
	s >> callbackID;
	s >> success;
	s >> entityID;
	s >> wasActive;

	if(wasActive)
	{
		s >> wasActiveCID;
		s >> wasActiveEntityID;
	}

	if (createToComponentID != g_componentID)
	{
		ERROR_MSG(fmt::format("Baseapp::onCreateEntityFromDBID: createToComponentID({}) != currComponentID({}), "
			"dbInterfaceIndex={}, entityType={}, dbid={}, callbackID={}, success={}, entityID={}, wasActive={}, wasActiveCID={}, wasActiveEntityID={}!\n",
			createToComponentID, g_componentID, dbInterfaceIndex, entityType, dbid, callbackID, success, entityID, wasActive, wasActiveCID, wasActiveEntityID));

		KBE_ASSERT(false);
	}

	if(!success)
	{
		if(callbackID > 0)
		{
			PyObject* baseEntityRef = NULL;

			if(wasActive && wasActiveCID > 0 && wasActiveEntityID > 0)
			{
				Entity* pEntity = this->findEntity(wasActiveEntityID);
				if(pEntity)
				{
					baseEntityRef = static_cast<PyObject*>(pEntity);
					Py_INCREF(baseEntityRef);
				}
				else
				{
					// 如果createEntityFromDBID类接口返回实体已经检出且在当前进程上，但是当前进程上无法找到实体时应该给出错误
					// 这种情况通常是异步的环境中从db查询到已经检出，但等回调时可能实体已经销毁了而造成的
					if(wasActiveCID != g_componentID)
					{
						baseEntityRef = static_cast<PyObject*>(new EntityCall(EntityDef::findScriptModule(entityType.c_str()), 
							NULL, wasActiveCID, wasActiveEntityID, ENTITYCALL_TYPE_BASE));
					}
					else
					{
						ERROR_MSG(fmt::format("Baseapp::onCreateEntityFromDBID: create {}({}) is failed! A local reference, But it has been destroyed!\n",
							entityType.c_str(), dbid));

						baseEntityRef = Py_None;
						Py_INCREF(baseEntityRef);
						wasActive = false;
					}
				}
			}
			else
			{
				baseEntityRef = Py_None;
				Py_INCREF(baseEntityRef);
				wasActive = false;

				ERROR_MSG(fmt::format("Baseapp::onCreateEntityFromDBID: create {}({}) is failed!\n",
					entityType.c_str(), dbid));
			}

			// baseEntityRef, dbid, wasActive
			PyObjectPtr pyfunc = pyCallbackMgr_.take(callbackID);
			if(pyfunc != NULL)
			{
				SCOPED_PROFILE(SCRIPTCALL_PROFILE);
				PyObject* pyResult = PyObject_CallFunction(pyfunc.get(), 
													const_cast<char*>("OKi"), 
													baseEntityRef, dbid, wasActive);

				if(pyResult != NULL)
					Py_DECREF(pyResult);
				else
					SCRIPT_ERROR_CHECK();
			}
			else
			{
				ERROR_MSG(fmt::format("Baseapp::onCreateEntityFromDBID: can't found callback:{}.\n",
					callbackID));
			}

			Py_DECREF(baseEntityRef);
		}
		
		s.done();
		return;
	}

	KBE_ASSERT(entityID > 0);
	EntityDef::context().currEntityID = entityID;
	EntityDef::context().currComponentType = BASEAPP_TYPE;

	PyObject* pyDict = createDictDataFromPersistentStream(s, entityType.c_str());
	PyObject* e = Baseapp::getSingleton().createEntity(entityType.c_str(), pyDict, false, entityID);
	if(e)
	{
		static_cast<Entity*>(e)->dbid(dbInterfaceIndex, dbid);
		static_cast<Entity*>(e)->initializeEntity(pyDict);
		Py_DECREF(pyDict);
	}
	else
	{
		ERROR_MSG(fmt::format("Baseapp::onCreateEntityFromDBID: create {}({}) is failed, e == NULL!\n", 
			entityType.c_str(), dbid));

		if(callbackID > 0)
		{
			PyObjectPtr pyfunc = pyCallbackMgr_.take(callbackID);
			if(pyfunc)
			{
				// 不需要通知脚本
			}
		}

		return;
	}

	if(callbackID > 0)
	{
		//if(e != NULL)
		//	Py_INCREF(e);

		// baseEntityRef, dbid, wasActive
		PyObjectPtr pyfunc = pyCallbackMgr_.take(callbackID);
		if(pyfunc != NULL)
		{
			SCOPED_PROFILE(SCRIPTCALL_PROFILE);
			PyObject* pyResult = PyObject_CallFunction(pyfunc.get(), 
												const_cast<char*>("OKi"), 
												e, dbid, wasActive);

			if(pyResult != NULL)
				Py_DECREF(pyResult);
			else
				SCRIPT_ERROR_CHECK();
		}
		else
		{
			ERROR_MSG(fmt::format("Baseapp::onCreateEntityFromDBID: can't found callback:{}.\n",
				callbackID));
		}
	}
}

//-------------------------------------------------------------------------------------
PyObject* Baseapp::__py_createEntityAnywhereFromDBID(PyObject* self, PyObject* args)
{
	int argCount = (int)PyTuple_Size(args);
	PyObject* pyCallback = NULL;
	wchar_t* wEntityType = NULL;
	char* entityType = NULL;
	int ret = -1;
	DBID dbid = 0;
	PyObject* pyEntityType = NULL;
	PyObject* pyDBInterfaceName = NULL;
	std::string dbInterfaceName = "default";

	switch(argCount)
	{
		case 4:
		{
			ret = PyArg_ParseTuple(args, "O|K|O|O", &pyEntityType, &dbid, &pyCallback, &pyDBInterfaceName);
			break;
		}
		case 3:
		{
				ret = PyArg_ParseTuple(args, "O|K|O", &pyEntityType, &dbid, &pyCallback);
				break;
		}
		case 2:
		{
				ret = PyArg_ParseTuple(args, "O|K", &pyEntityType, &dbid);
				break;
		}
		default:
		{
				PyErr_Format(PyExc_AssertionError, "%s: args require 2 or 3 args, gived %d!\n",
					__FUNCTION__, argCount);	
				PyErr_PrintEx(0);
				return NULL;
		}
	};

	if (ret == -1)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::createEntityAnywhereFromDBID: args error!");
		PyErr_PrintEx(0);
		return NULL;
	}

	if (pyDBInterfaceName)
	{
		wchar_t* PyUnicode_AsWideCharStringRet0 = PyUnicode_AsWideCharString(pyDBInterfaceName, NULL);
		char* ccattr = strutil::wchar2char(PyUnicode_AsWideCharStringRet0);
		dbInterfaceName = ccattr;
		PyMem_Free(PyUnicode_AsWideCharStringRet0);
		free(ccattr);

		DBInterfaceInfo* pDBInterfaceInfo = g_kbeSrvConfig.dbInterface(dbInterfaceName);
		if (pDBInterfaceInfo->isPure)
		{
			ERROR_MSG(fmt::format("Baseapp::createEntityAnywhereFromDBID: dbInterface({}) is a pure database does not support Entity! "
				"kbengine[_defs].xml->dbmgr->databaseInterfaces->*->pure\n",
				dbInterfaceName));

			return NULL;
		}

		int dbInterfaceIndex = pDBInterfaceInfo->index;
		if (dbInterfaceIndex < 0)
		{
			PyErr_Format(PyExc_TypeError, "Baseapp::createEntityAnywhereFromDBID: not found dbInterface(%s)!",
				dbInterfaceName.c_str());

			PyErr_PrintEx(0);
			return NULL;
		}
	}

	if(pyEntityType)
	{
		wEntityType = PyUnicode_AsWideCharString(pyEntityType, NULL);		
		if(wEntityType)
		{
			entityType = strutil::wchar2char(wEntityType);									
			PyMem_Free(wEntityType);	
		}
		else
		{
			SCRIPT_ERROR_CHECK();
		}
	}

	if(entityType == NULL || strlen(entityType) <= 0 || ret == -1)
	{
		PyErr_Format(PyExc_AssertionError, "Baseapp::createEntityAnywhereFromDBID: args error, entityType=%s!", 
			(entityType ? entityType : "NULL"));

		PyErr_PrintEx(0);

		if(entityType)
			free(entityType);

		return NULL;
	}

	if(EntityDef::findScriptModule(entityType) == NULL)
	{
		PyErr_Format(PyExc_AssertionError, "Baseapp::createEntityAnywhereFromDBID: entityType error!");
		PyErr_PrintEx(0);
		free(entityType);
		return NULL;
	}

	if(dbid <= 0)
	{
		PyErr_Format(PyExc_AssertionError, "Baseapp::createEntityAnywhereFromDBID: dbid error!");
		PyErr_PrintEx(0);
		free(entityType);
		return NULL;
	}

	if(pyCallback && !PyCallable_Check(pyCallback))
	{
		pyCallback = NULL;

		PyErr_Format(PyExc_AssertionError, "Baseapp::createEntityAnywhereFromDBID: callback error!");
		PyErr_PrintEx(0);
		free(entityType);
		return NULL;
	}

	Baseapp::getSingleton().createEntityAnywhereFromDBID(entityType, dbid, pyCallback, dbInterfaceName);

	free(entityType);
	S_Return;
}

//-------------------------------------------------------------------------------------
void Baseapp::createEntityAnywhereFromDBID(const char* entityType, DBID dbid, PyObject* pyCallback, const std::string& dbInterfaceName)
{
	Network::Channel* pBaseappmgrChannel = Components::getSingleton().getBaseappmgrChannel();
	if (pBaseappmgrChannel == NULL)
	{
		ERROR_MSG(fmt::format("Baseapp::createEntityAnywhereFromDBID: create {}({}) is error, not found baseappmgr!\n",
			entityType, dbid));

		return;
	}

	DBInterfaceInfo* pDBInterfaceInfo = g_kbeSrvConfig.dbInterface(dbInterfaceName);
	if (pDBInterfaceInfo->isPure)
	{
		ERROR_MSG(fmt::format("Baseapp::createEntityAnywhereFromDBID: dbInterface({}) is a pure database does not support Entity! "
			"kbengine[_defs].xml->dbmgr->databaseInterfaces->*->pure\n",
			dbInterfaceName));

		return;
	}

	int dbInterfaceIndex = pDBInterfaceInfo->index;
	if (dbInterfaceIndex < 0)
	{
		PyErr_Format(PyExc_TypeError, "Baseapp::createEntityAnywhereFromDBID: not found dbInterface(%s)!", 
			dbInterfaceName.c_str());

		PyErr_PrintEx(0);
		return;
	}

	uint16 udbInterfaceIndex = dbInterfaceIndex;

	CALLBACK_ID callbackID = 0;
	if(pyCallback != NULL)
	{
		callbackID = callbackMgr().save(pyCallback);
	}

	Network::Bundle* pBundle = Network::Bundle::createPoolObject();
	pBundle->newMessage(BaseappmgrInterface::reqCreateEntityAnywhereFromDBIDQueryBestBaseappID);
	(*pBundle) << entityType << dbid << callbackID << udbInterfaceIndex;
	pBaseappmgrChannel->send(pBundle);
}

//-------------------------------------------------------------------------------------
void Baseapp::onGetCreateEntityAnywhereFromDBIDBestBaseappID(Network::Channel* pChannel, KBEngine::MemoryStream& s)
{
	if(pChannel->isExternal())
		return;
	
	COMPONENT_ID targetComponentID;
	s >> targetComponentID;

	// 如果为0说明没有可用的，那么就用自己来创建好了
	if (targetComponentID == 0)
		targetComponentID = g_componentID;

	std::string entityType;
	DBID dbid;
	CALLBACK_ID callbackID;
	uint16 dbInterfaceIndex;

	s >> entityType;
	s >> dbid;
	s >> callbackID;
	s >> dbInterfaceIndex;

	Components::ComponentInfos* dbmgrinfos = Components::getSingleton().getDbmgr();
	if (dbmgrinfos == NULL || dbmgrinfos->pChannel == NULL || dbmgrinfos->cid == 0)
	{
		ERROR_MSG(fmt::format("Baseapp::createEntityAnywhereFromDBID: not found dbmgr!\n"));

		if (callbackID > 0)
		{
			PyObjectPtr pyfunc = pyCallbackMgr_.take(callbackID);
			if (pyfunc != NULL)
			{
			}
		}

		return;
	}

	Network::Bundle* pBundle = Network::Bundle::createPoolObject();
	pBundle->newMessage(DbmgrInterface::queryEntity);

	ENTITY_ID entityID = idClient_.alloc();
	KBE_ASSERT(entityID > 0);

	DbmgrInterface::queryEntityArgs7::staticAddToBundle((*pBundle),
		dbInterfaceIndex, targetComponentID, 1, dbid, entityType, callbackID, entityID);

	dbmgrinfos->pChannel->send(pBundle);
}

//-------------------------------------------------------------------------------------
void Baseapp::onCreateEntityAnywhereFromDBIDCallback(Network::Channel* pChannel, KBEngine::MemoryStream& s)
{
	if(pChannel->isExternal())
		return;
	
	size_t currpos = s.rpos();

	std::string entityType;
	DBID dbid;
	CALLBACK_ID callbackID;
	bool success = false;
	bool wasActive = false;
	ENTITY_ID entityID;
	COMPONENT_ID wasActiveCID;
	ENTITY_ID wasActiveEntityID;
	uint16 dbInterfaceIndex;
	COMPONENT_ID createToComponentID = 0;

	s >> createToComponentID;
	s >> dbInterfaceIndex;
	s >> entityType;
	s >> dbid;
	s >> callbackID;
	s >> success;
	s >> entityID;
	s >> wasActive;

	if(wasActive)
	{
		s >> wasActiveCID;
		s >> wasActiveEntityID;
	}

	if(!success)
	{
		if(callbackID > 0)
		{
			PyObject* baseEntityRef = NULL;

			if(wasActive && wasActiveCID > 0 && wasActiveEntityID > 0)
			{
				Entity* pEntity = this->findEntity(wasActiveEntityID);
				if(pEntity)
				{
					baseEntityRef = static_cast<PyObject*>(pEntity);
					Py_INCREF(baseEntityRef);
				}
				else
				{
					// 如果createEntityFromDBID类接口返回实体已经检出且在当前进程上，但是当前进程上无法找到实体时应该给出错误
					// 这种情况通常是异步的环境中从db查询到已经检出，但等回调时可能实体已经销毁了而造成的
					if(wasActiveCID != g_componentID)
					{
						baseEntityRef = static_cast<PyObject*>(new EntityCall(EntityDef::findScriptModule(entityType.c_str()), 
							NULL, wasActiveCID, wasActiveEntityID, ENTITYCALL_TYPE_BASE));
					}
					else
					{
						ERROR_MSG(fmt::format("Baseapp::createEntityAnywhereFromDBID: create {}({}) is failed! A local reference, But it has been destroyed!\n",
							entityType.c_str(), dbid));

						baseEntityRef = Py_None;
						Py_INCREF(baseEntityRef);
						wasActive = false;
					}
				}
			}
			else
			{
				ERROR_MSG(fmt::format("Baseapp::createEntityAnywhereFromDBID: create {}({}) is failed.\n", 
					entityType.c_str(), dbid));

				wasActive = false;
				baseEntityRef = Py_None;
				Py_INCREF(baseEntityRef);
			}

			// baseEntityRef, dbid, wasActive
			PyObjectPtr pyfunc = pyCallbackMgr_.take(callbackID);
			if(pyfunc != NULL)
			{
				SCOPED_PROFILE(SCRIPTCALL_PROFILE);
				PyObject* pyResult = PyObject_CallFunction(pyfunc.get(), 
													const_cast<char*>("OKi"), 
													baseEntityRef, dbid, wasActive);

				if(pyResult != NULL)
					Py_DECREF(pyResult);
				else
					SCRIPT_ERROR_CHECK();
			}
			else
			{
				ERROR_MSG(fmt::format("Baseapp::createEntityAnywhereFromDBID: can't found callback:{}.\n",
					callbackID));
			}

			Py_DECREF(baseEntityRef);
		}
		
		s.done();
		return;
	}

	Network::Channel* pBaseappmgrChannel = Components::getSingleton().getBaseappmgrChannel();
	if(pBaseappmgrChannel == NULL)
	{
		ERROR_MSG(fmt::format("Baseapp::createEntityAnywhereFromDBID: create {}({}) error, not found baseappmgr!\n", 
			entityType.c_str(), dbid));

		if (callbackID > 0)
		{
			PyObjectPtr pyfunc = pyCallbackMgr_.take(callbackID);
			if (pyfunc != NULL)
			{
			}
		}

		return;
	}

	s.rpos((int)currpos);

	MemoryStream* stream = MemoryStream::createPoolObject();
	(*stream) << createToComponentID << g_componentID;
	stream->append(s);
	s.done();

	// 通知baseappmgr在其他baseapp上创建entity
	Network::Bundle* pBundle = Network::Bundle::createPoolObject();
	pBundle->newMessage(BaseappmgrInterface::reqCreateEntityAnywhereFromDBID);
	pBundle->append((*stream));
	pBaseappmgrChannel->send(pBundle);
	MemoryStream::reclaimPoolObject(stream);
}

//-------------------------------------------------------------------------------------
void Baseapp::createEntityAnywhereFromDBIDOtherBaseapp(Network::Channel* pChannel, KBEngine::MemoryStream& s)
{
	if(pChannel->isExternal())
		return;
	
	std::string entityType;
	DBID dbid;
	CALLBACK_ID callbackID;
	bool success = false;
	bool wasActive = false;
	ENTITY_ID entityID;
	COMPONENT_ID sourceBaseappID;
	uint16 dbInterfaceIndex;
	COMPONENT_ID createToComponentID = 0;

	s >> sourceBaseappID;
	s >> createToComponentID;
	s >> dbInterfaceIndex;
	s >> entityType;
	s >> dbid;
	s >> callbackID;
	s >> success;
	s >> entityID;
	s >> wasActive;

	if (createToComponentID != g_componentID)
	{
		ERROR_MSG(fmt::format("Baseapp::createEntityAnywhereFromDBIDOtherBaseapp: createToComponentID({}) != currComponentID({}), "
			"sourceBaseappID={}, dbInterfaceIndex={}, entityType={}, dbid={}, callbackID={}, success={}, entityID={}, wasActive={}!\n",
			createToComponentID, g_componentID, sourceBaseappID, dbInterfaceIndex, entityType, dbid, callbackID, success, entityID, wasActive));

		KBE_ASSERT(false);
	}

	KBE_ASSERT(entityID > 0);
	EntityDef::context().currEntityID = entityID;
	EntityDef::context().currComponentType = BASEAPP_TYPE;
	PyObject* pyDict = createDictDataFromPersistentStream(s, entityType.c_str());
	PyObject* e = Baseapp::getSingleton().createEntity(entityType.c_str(), pyDict, false, entityID);

	if(e)
	{
		static_cast<Entity*>(e)->dbid(dbInterfaceIndex, dbid);
		static_cast<Entity*>(e)->initializeEntity(pyDict);
		Py_DECREF(pyDict);
	}
	else
	{
		ERROR_MSG(fmt::format("Baseapp::createEntityAnywhereFromDBIDOtherBaseapp: create {}({}) is failed, e == NULL!\n", 
			entityType.c_str(), dbid));

		if(callbackID > 0 && g_componentID == sourceBaseappID)
		{
			PyObjectPtr pyfunc = pyCallbackMgr_.take(callbackID);
			if(pyfunc)
			{
				// 不需要通知脚本
			}
		}

		return;
	}

	// 是否本地组件就是发起源， 如果是直接在本地调用回调
	if(g_componentID == sourceBaseappID)
	{
		onCreateEntityAnywhereFromDBIDOtherBaseappCallback(pChannel, g_componentID, entityType, static_cast<Entity*>(e)->id(), callbackID, dbid);
	}
	else
	{
		// 通知baseapp, 创建好了
		Network::Bundle* pBundle = Network::Bundle::createPoolObject();
		pBundle->newMessage(BaseappInterface::onCreateEntityAnywhereFromDBIDOtherBaseappCallback);


		BaseappInterface::onCreateEntityAnywhereFromDBIDOtherBaseappCallbackArgs5::staticAddToBundle((*pBundle), 
			g_componentID, entityType, static_cast<Entity*>(e)->id(), callbackID, dbid);

		Components::ComponentInfos* baseappinfos = Components::getSingleton().findComponent(BASEAPP_TYPE, sourceBaseappID);
		if(baseappinfos == NULL || baseappinfos->pChannel == NULL || baseappinfos->cid == 0)
		{
			ForwardItem* pFI = new ForwardItem();
			pFI->pHandler = NULL;
			pFI->pBundle = pBundle;
			forward_messagebuffer_.push(sourceBaseappID, pFI);
			WARNING_MSG(fmt::format("Baseapp::createEntityAnywhereFromDBID: not found sourceBaseapp({}), message is buffered.\n", sourceBaseappID));
			return;
		}
		
		baseappinfos->pChannel->send(pBundle);
	}
}

//-------------------------------------------------------------------------------------
void Baseapp::onCreateEntityAnywhereFromDBIDOtherBaseappCallback(Network::Channel* pChannel, COMPONENT_ID createByBaseappID, 
															   std::string entityType, ENTITY_ID createdEntityID, CALLBACK_ID callbackID, DBID dbid)
{
	if(pChannel->isExternal())
		return;
	
	if(callbackID > 0)
	{
		ScriptDefModule* sm = EntityDef::findScriptModule(entityType.c_str());
		if(sm == NULL)
		{
			ERROR_MSG(fmt::format("Baseapp::onCreateEntityAnywhereFromDBIDOtherBaseappCallback: not found entityType:{}.\n",
				entityType.c_str()));

			if (callbackID > 0)
			{
				PyObjectPtr pyfunc = pyCallbackMgr_.take(callbackID);
				if (pyfunc != NULL)
				{
				}
			}

			return;
		}

		// baseEntityRef, dbid, wasActive
		PyObjectPtr pyfunc = pyCallbackMgr_.take(callbackID);
		if(pyfunc != NULL)
		{
			Entity* pEntity = this->findEntity(createdEntityID);

			PyObject* pyResult = NULL;
			
			SCOPED_PROFILE(SCRIPTCALL_PROFILE);

			if(pEntity)
			{
				pyResult = PyObject_CallFunction(pyfunc.get(), 
												const_cast<char*>("OKi"), 
												pEntity, dbid, 0);
			}
			else
			{
				PyObject* mb = static_cast<PyObject*>(new EntityCall(sm, NULL, createByBaseappID, createdEntityID, ENTITYCALL_TYPE_BASE));
				pyResult = PyObject_CallFunction(pyfunc.get(), 
												const_cast<char*>("OKi"), 
												mb, dbid, 0);
				Py_DECREF(mb);
			}

			if(pyResult != NULL)
				Py_DECREF(pyResult);
			else
				SCRIPT_ERROR_CHECK();
		}
		else
		{
			ERROR_MSG(fmt::format("Baseapp::createEntityAnywhereFromDBID: not found callback:{}.\n",
				callbackID));
		}
	}
}

//-------------------------------------------------------------------------------------
PyObject* Baseapp::__py_createEntityRemotelyFromDBID(PyObject* self, PyObject* args)
{
	int argCount = (int)PyTuple_Size(args);
	PyObject* pyCallback = NULL, *pyEntityCall = NULL;
	wchar_t* wEntityType = NULL;
	char* entityType = NULL;
	int ret = -1;
	DBID dbid = 0;
	PyObject* pyEntityType = NULL;
	PyObject* pyDBInterfaceName = NULL;
	std::string dbInterfaceName = "default";

	switch(argCount)
	{
		case 5:
		{
			ret = PyArg_ParseTuple(args, "O|K|O|O|O", &pyEntityType, &dbid, &pyEntityCall, &pyCallback, &pyDBInterfaceName);
			break;
		}
		case 4:
		{
				ret = PyArg_ParseTuple(args, "O|K|O|O", &pyEntityType, &dbid, &pyEntityCall, &pyCallback);
				break;
		}
		case 3:
		{
				ret = PyArg_ParseTuple(args, "O|K|O", &pyEntityType, &dbid, &pyEntityCall);
				break;
		}
		default:
		{
				PyErr_Format(PyExc_AssertionError, "%s: args require 3 ~ 5 args, gived %d!\n",
					__FUNCTION__, argCount);	
				PyErr_PrintEx(0);
				return NULL;
		}
	};

	if (ret == -1)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::createEntityRemotelyFromDBID: args error!");
		PyErr_PrintEx(0);
		return NULL;
	}

	if (pyDBInterfaceName)
	{
		wchar_t* PyUnicode_AsWideCharStringRet0 = PyUnicode_AsWideCharString(pyDBInterfaceName, NULL);
		char* ccattr = strutil::wchar2char(PyUnicode_AsWideCharStringRet0);
		dbInterfaceName = ccattr;
		PyMem_Free(PyUnicode_AsWideCharStringRet0);
		free(ccattr);

		DBInterfaceInfo* pDBInterfaceInfo = g_kbeSrvConfig.dbInterface(dbInterfaceName);
		if (pDBInterfaceInfo->isPure)
		{
			ERROR_MSG(fmt::format("Baseapp::createEntityRemotelyFromDBID: dbInterface({}) is a pure database does not support Entity! "
				"kbengine[_defs].xml->dbmgr->databaseInterfaces->*->pure\n",
				dbInterfaceName));

			return NULL;
		}

		int dbInterfaceIndex = pDBInterfaceInfo->index;
		if (dbInterfaceIndex < 0)
		{
			PyErr_Format(PyExc_TypeError, "Baseapp::createEntityRemotelyFromDBID: not found dbInterface(%s)!",
				dbInterfaceName.c_str());

			PyErr_PrintEx(0);
			return NULL;
		}
	}

	if(pyEntityType)
	{
		wEntityType = PyUnicode_AsWideCharString(pyEntityType, NULL);		
		if(wEntityType)
		{
			entityType = strutil::wchar2char(wEntityType);									
			PyMem_Free(wEntityType);	
		}
		else
		{
			SCRIPT_ERROR_CHECK();
		}
	}

	if(entityType == NULL || strlen(entityType) <= 0 || ret == -1)
	{
		PyErr_Format(PyExc_AssertionError, "Baseapp::createEntityRemotelyFromDBID: args error, entityType=%s!", 
			(entityType ? entityType : "NULL"));

		PyErr_PrintEx(0);

		if(entityType)
			free(entityType);

		return NULL;
	}

	if(EntityDef::findScriptModule(entityType) == NULL)
	{
		PyErr_Format(PyExc_AssertionError, "Baseapp::createEntityRemotelyFromDBID: entityType error!");
		PyErr_PrintEx(0);
		free(entityType);
		return NULL;
	}

	if(dbid <= 0)
	{
		PyErr_Format(PyExc_AssertionError, "Baseapp::createEntityRemotelyFromDBID: dbid error!");
		PyErr_PrintEx(0);
		free(entityType);
		return NULL;
	}

	if(pyCallback && !PyCallable_Check(pyCallback))
	{
		pyCallback = NULL;

		PyErr_Format(PyExc_AssertionError, "Baseapp::createEntityRemotelyFromDBID: callback error!");
		PyErr_PrintEx(0);
		free(entityType);
		return NULL;
	}

	if (pyEntityCall == NULL || !PyObject_TypeCheck(pyEntityCall, EntityCall::getScriptType()))
	{
		PyErr_Format(PyExc_TypeError, "create %s arg2 is not baseEntityCall!",
			entityType);

		PyErr_PrintEx(0);
		return 0;
	}

	EntityCallAbstract* baseEntityCall = static_cast<EntityCallAbstract*>(pyEntityCall);
	if (baseEntityCall->type() != ENTITYCALL_TYPE_BASE)
	{
		PyErr_Format(PyExc_TypeError, "create %s args2 not is a direct baseEntityCall!",
			entityType);

		PyErr_PrintEx(0);
		return 0;
	}

	Baseapp::getSingleton().createEntityRemotelyFromDBID(entityType, dbid, baseEntityCall->componentID(), pyCallback, dbInterfaceName);

	free(entityType);
	S_Return;
}

//-------------------------------------------------------------------------------------
void Baseapp::createEntityRemotelyFromDBID(const char* entityType, DBID dbid, COMPONENT_ID createToComponentID, PyObject* pyCallback, const std::string& dbInterfaceName)
{
	Components::ComponentInfos* dbmgrinfos = Components::getSingleton().getDbmgr();
	if (dbmgrinfos == NULL || dbmgrinfos->pChannel == NULL || dbmgrinfos->cid == 0)
	{
		ERROR_MSG(fmt::format("Baseapp::createEntityRemotelyFromDBID: not found dbmgr!\n"));
		return;
	}

	DBInterfaceInfo* pDBInterfaceInfo = g_kbeSrvConfig.dbInterface(dbInterfaceName);
	if (pDBInterfaceInfo->isPure)
	{
		ERROR_MSG(fmt::format("Baseapp::createEntityRemotelyFromDBID: dbInterface({}) is a pure database does not support Entity! "
			"kbengine[_defs].xml->dbmgr->databaseInterfaces->*->pure\n",
			dbInterfaceName));

		return;
	}

	int dbInterfaceIndex = pDBInterfaceInfo->index;
	if (dbInterfaceIndex < 0)
	{
		PyErr_Format(PyExc_TypeError, "Baseapp::createEntityRemotelyFromDBID: not found dbInterface(%s)!", 
			dbInterfaceName.c_str());

		PyErr_PrintEx(0);
		return;
	}

	CALLBACK_ID callbackID = 0;
	if(pyCallback != NULL)
	{
		callbackID = callbackMgr().save(pyCallback);
	}

	Network::Bundle* pBundle = Network::Bundle::createPoolObject();
	pBundle->newMessage(DbmgrInterface::queryEntity);

	ENTITY_ID entityID = idClient_.alloc();
	KBE_ASSERT(entityID > 0);

	DbmgrInterface::queryEntityArgs7::staticAddToBundle((*pBundle),
		dbInterfaceIndex, createToComponentID, 2, dbid, entityType, callbackID, entityID);

	dbmgrinfos->pChannel->send(pBundle);
}

//-------------------------------------------------------------------------------------
void Baseapp::onCreateEntityRemotelyFromDBIDCallback(Network::Channel* pChannel, KBEngine::MemoryStream& s)
{
	if(pChannel->isExternal())
		return;
	
	size_t currpos = s.rpos();

	std::string entityType;
	DBID dbid;
	CALLBACK_ID callbackID;
	bool success = false;
	bool wasActive = false;
	ENTITY_ID entityID;
	COMPONENT_ID wasActiveCID;
	ENTITY_ID wasActiveEntityID;
	uint16 dbInterfaceIndex;
	COMPONENT_ID createToComponentID = 0;

	s >> createToComponentID;
	s >> dbInterfaceIndex;
	s >> entityType;
	s >> dbid;
	s >> callbackID;
	s >> success;
	s >> entityID;
	s >> wasActive;

	if(wasActive)
	{
		s >> wasActiveCID;
		s >> wasActiveEntityID;
	}

	if(!success)
	{
		if(callbackID > 0)
		{
			PyObject* baseEntityRef = NULL;

			if(wasActive && wasActiveCID > 0 && wasActiveEntityID > 0)
			{
				Entity* pEntity = this->findEntity(wasActiveEntityID);
				if(pEntity)
				{
					baseEntityRef = static_cast<PyObject*>(pEntity);
					Py_INCREF(baseEntityRef);
				}
				else
				{
					// 如果createEntityFromDBID类接口返回实体已经检出且在当前进程上，但是当前进程上无法找到实体时应该给出错误
					// 这种情况通常是异步的环境中从db查询到已经检出，但等回调时可能实体已经销毁了而造成的
					if(wasActiveCID != g_componentID)
					{
						baseEntityRef = static_cast<PyObject*>(new EntityCall(EntityDef::findScriptModule(entityType.c_str()), 
							NULL, wasActiveCID, wasActiveEntityID, ENTITYCALL_TYPE_BASE));
					}
					else
					{
						ERROR_MSG(fmt::format("Baseapp::createEntityRemotelyFromDBID: create {}({}) is failed! A local reference, But it has been destroyed!\n",
							entityType.c_str(), dbid));

						baseEntityRef = Py_None;
						Py_INCREF(baseEntityRef);
						wasActive = false;
					}
				}
			}
			else
			{
				ERROR_MSG(fmt::format("Baseapp::createEntityRemotelyFromDBID: create {}({}) is failed.\n", 
					entityType.c_str(), dbid));

				wasActive = false;
				baseEntityRef = Py_None;
				Py_INCREF(baseEntityRef);
			}

			// baseEntityRef, dbid, wasActive
			PyObjectPtr pyfunc = pyCallbackMgr_.take(callbackID);
			if(pyfunc != NULL)
			{
				SCOPED_PROFILE(SCRIPTCALL_PROFILE);
				PyObject* pyResult = PyObject_CallFunction(pyfunc.get(), 
													const_cast<char*>("OKi"), 
													baseEntityRef, dbid, wasActive);

				if(pyResult != NULL)
					Py_DECREF(pyResult);
				else
					SCRIPT_ERROR_CHECK();
			}
			else
			{
				ERROR_MSG(fmt::format("Baseapp::createEntityRemotelyFromDBID: can't found callback:{}.\n",
					callbackID));
			}

			Py_DECREF(baseEntityRef);
		}
		
		s.done();
		return;
	}

	Network::Channel* pBaseappmgrChannel = Components::getSingleton().getBaseappmgrChannel();
	if(pBaseappmgrChannel == NULL)
	{
		ERROR_MSG(fmt::format("Baseapp::createEntityRemotelyFromDBID: create {}({}) error, not found baseappmgr!\n", 
			entityType.c_str(), dbid));

		if (callbackID > 0)
		{
			PyObjectPtr pyfunc = pyCallbackMgr_.take(callbackID);
			if (pyfunc != NULL)
			{
			}
		}

		return;
	}

	s.rpos((int)currpos);

	MemoryStream* stream = MemoryStream::createPoolObject();
	(*stream) << createToComponentID << g_componentID;
	stream->append(s);
	s.done();

	// 通知baseappmgr在其他baseapp上创建entity
	Network::Bundle* pBundle = Network::Bundle::createPoolObject();
	pBundle->newMessage(BaseappmgrInterface::reqCreateEntityRemotelyFromDBID);
	pBundle->append((*stream));
	pBaseappmgrChannel->send(pBundle);
	MemoryStream::reclaimPoolObject(stream);
}

//-------------------------------------------------------------------------------------
void Baseapp::createEntityRemotelyFromDBIDOtherBaseapp(Network::Channel* pChannel, KBEngine::MemoryStream& s)
{
	if(pChannel->isExternal())
		return;
	
	std::string entityType;
	DBID dbid;
	CALLBACK_ID callbackID;
	bool success = false;
	bool wasActive = false;
	ENTITY_ID entityID;
	COMPONENT_ID sourceBaseappID;
	uint16 dbInterfaceIndex;
	COMPONENT_ID createToComponentID = 0;

	s >> sourceBaseappID;
	s >> createToComponentID;
	s >> dbInterfaceIndex;
	s >> entityType;
	s >> dbid;
	s >> callbackID;
	s >> success;
	s >> entityID;
	s >> wasActive;

	if (createToComponentID != g_componentID)
	{
		ERROR_MSG(fmt::format("Baseapp::createEntityRemotelyFromDBIDOtherBaseapp: createToComponentID({}) != currComponentID({}), "
			"sourceBaseappID={}, dbInterfaceIndex={}, entityType={}, dbid={}, callbackID={}, success={}, entityID={}, wasActive={}!\n",
			createToComponentID, g_componentID, sourceBaseappID, dbInterfaceIndex, entityType, dbid, callbackID, success, entityID, wasActive));

		KBE_ASSERT(false);
	}

	KBE_ASSERT(entityID > 0);
	EntityDef::context().currEntityID = entityID;
	EntityDef::context().currComponentType = BASEAPP_TYPE;
	PyObject* pyDict = createDictDataFromPersistentStream(s, entityType.c_str());
	PyObject* e = Baseapp::getSingleton().createEntity(entityType.c_str(), pyDict, false, entityID);

	if(e)
	{
		static_cast<Entity*>(e)->dbid(dbInterfaceIndex, dbid);
		static_cast<Entity*>(e)->initializeEntity(pyDict);
		Py_DECREF(pyDict);
	}
	else
	{
		ERROR_MSG(fmt::format("Baseapp::createEntityRemotelyFromDBIDOtherBaseapp: create {}({}) is failed, e == NULL!\n", 
			entityType.c_str(), dbid));

		if(callbackID > 0 && g_componentID == sourceBaseappID)
		{
			PyObjectPtr pyfunc = pyCallbackMgr_.take(callbackID);
			if(pyfunc)
			{
				// 不需要通知脚本
			}
		}

		return;
	}

	// 是否本地组件就是发起源， 如果是直接在本地调用回调
	if(g_componentID == sourceBaseappID)
	{
		onCreateEntityRemotelyFromDBIDOtherBaseappCallback(pChannel, g_componentID, entityType, static_cast<Entity*>(e)->id(), callbackID, dbid);
	}
	else
	{
		// 通知baseapp, 创建好了
		Network::Bundle* pBundle = Network::Bundle::createPoolObject();
		pBundle->newMessage(BaseappInterface::onCreateEntityRemotelyFromDBIDOtherBaseappCallback);


		BaseappInterface::onCreateEntityRemotelyFromDBIDOtherBaseappCallbackArgs5::staticAddToBundle((*pBundle), 
			g_componentID, entityType, static_cast<Entity*>(e)->id(), callbackID, dbid);

		Components::ComponentInfos* baseappinfos = Components::getSingleton().findComponent(BASEAPP_TYPE, sourceBaseappID);
		if(baseappinfos == NULL || baseappinfos->pChannel == NULL || baseappinfos->cid == 0)
		{
			ForwardItem* pFI = new ForwardItem();
			pFI->pHandler = NULL;
			pFI->pBundle = pBundle;
			forward_messagebuffer_.push(sourceBaseappID, pFI);
			WARNING_MSG(fmt::format("Baseapp::createEntityRemotelyFromDBID: not found sourceBaseapp({}), message is buffered.\n", sourceBaseappID));
			return;
		}
		
		baseappinfos->pChannel->send(pBundle);
	}
}

//-------------------------------------------------------------------------------------
void Baseapp::onCreateEntityRemotelyFromDBIDOtherBaseappCallback(Network::Channel* pChannel, COMPONENT_ID createByBaseappID, 
															   std::string entityType, ENTITY_ID createdEntityID, CALLBACK_ID callbackID, DBID dbid)
{
	if(pChannel->isExternal())
		return;
	
	if(callbackID > 0)
	{
		ScriptDefModule* sm = EntityDef::findScriptModule(entityType.c_str());
		if(sm == NULL)
		{
			ERROR_MSG(fmt::format("Baseapp::onCreateEntityRemotelyFromDBIDOtherBaseappCallback: not found entityType:{}.\n",
				entityType.c_str()));

			if (callbackID > 0)
			{
				PyObjectPtr pyfunc = pyCallbackMgr_.take(callbackID);
				if (pyfunc != NULL)
				{
				}
			}

			return;
		}

		// baseEntityRef, dbid, wasActive
		PyObjectPtr pyfunc = pyCallbackMgr_.take(callbackID);
		if(pyfunc != NULL)
		{
			Entity* pEntity = this->findEntity(createdEntityID);

			PyObject* pyResult = NULL;
			
			SCOPED_PROFILE(SCRIPTCALL_PROFILE);

			if(pEntity)
			{
				pyResult = PyObject_CallFunction(pyfunc.get(), 
												const_cast<char*>("OKi"), 
												pEntity, dbid, 0);
			}
			else
			{
				PyObject* mb = static_cast<PyObject*>(new EntityCall(sm, NULL, createByBaseappID, createdEntityID, ENTITYCALL_TYPE_BASE));
				pyResult = PyObject_CallFunction(pyfunc.get(), 
												const_cast<char*>("OKi"), 
												mb, dbid, 0);
				Py_DECREF(mb);
			}

			if(pyResult != NULL)
				Py_DECREF(pyResult);
			else
				SCRIPT_ERROR_CHECK();
		}
		else
		{
			ERROR_MSG(fmt::format("Baseapp::createEntityRemotelyFromDBID: not found callback:{}.\n",
				callbackID));
		}
	}
}

//-------------------------------------------------------------------------------------
void Baseapp::createCellEntityInNewSpace(Entity* pEntity, PyObject* pyCellappIndex)
{
	ScriptDefModule* pScriptModule = pEntity->pScriptModule();
	if (!pScriptModule || !pScriptModule->hasCell())
	{
		ERROR_MSG(fmt::format("{}::createCellEntityInNewSpace: cannot find the cellapp script({})!\n",
			pScriptModule->getName(), pScriptModule->getName()));

		return;
	}

	// 如果cellappIndex为0，则代表不强制指定cellapp
	// 非0的情况下，选择的cellapp可以用1,2,3,4来代替
	// 假如预期有4个cellapp， 假如不够4个， 只有3个， 那么4代表1
	uint32 cellappIndex = 0;

	if (PyLong_Check(pyCellappIndex))
		cellappIndex = (uint32)PyLong_AsUnsignedLong(pyCellappIndex);

	ENTITY_ID id = pEntity->id();
	std::string entityType = pEntity->ob_type->tp_name;

	Network::Bundle* pBundle = Network::Bundle::createPoolObject();

	(*pBundle).newMessage(CellappmgrInterface::reqCreateCellEntityInNewSpace);

	(*pBundle) << entityType;
	(*pBundle) << id;
	(*pBundle) << cellappIndex;
	(*pBundle) << componentID_;

	EntityCall* clientEntityCall = pEntity->clientEntityCall();
	bool hasClient = (clientEntityCall != NULL);
	(*pBundle) << hasClient;

	MemoryStream* s = MemoryStream::createPoolObject();
	pEntity->addCellDataToStream(CELLAPP_TYPE, ED_FLAG_ALL, s);
	(*pBundle).append(*s);
	MemoryStream::reclaimPoolObject(s);
	
	Components::ComponentInfos* pComponents = Components::getSingleton().getCellappmgr();
	if(pComponents)
	{
		if(pComponents->pChannel != NULL)
		{
			pComponents->pChannel->send(pBundle);
		}
		else
		{
			ERROR_MSG("Baseapp::createCellEntityInNewSpace: cellappmgr channel is NULL.\n");
			Network::Bundle::reclaimPoolObject(pBundle);
		}
		
		return;
	}

	Network::Bundle::reclaimPoolObject(pBundle);
	ERROR_MSG("Baseapp::createCellEntityInNewSpace: not found cellappmgr.\n");
}

//-------------------------------------------------------------------------------------
void Baseapp::restoreSpaceInCell(Entity* pEntity)
{
	ENTITY_ID id = pEntity->id();
	std::string entityType = pEntity->ob_type->tp_name;

	Network::Bundle* pBundle = Network::Bundle::createPoolObject();

	(*pBundle).newMessage(CellappmgrInterface::reqRestoreSpaceInCell);

	(*pBundle) << entityType;
	(*pBundle) << id;
	(*pBundle) << componentID_;
	(*pBundle) << pEntity->spaceID();

	EntityCall* clientEntityCall = pEntity->clientEntityCall();
	bool hasClient = (clientEntityCall != NULL);
	(*pBundle) << hasClient;

	MemoryStream* s = MemoryStream::createPoolObject();
	pEntity->addCellDataToStream(CELLAPP_TYPE, ED_FLAG_ALL, s);
	(*pBundle).append(*s);
	MemoryStream::reclaimPoolObject(s);
	
	Components::ComponentInfos* pComponents = Components::getSingleton().getCellappmgr();
	if(pComponents)
	{
		if(pComponents->pChannel != NULL)
		{
			pComponents->pChannel->send(pBundle);
		}
		else
		{
			ERROR_MSG("Baseapp::restoreSpaceInCell: cellappmgr channel is NULL.\n");
			Network::Bundle::reclaimPoolObject(pBundle);
		}
		
		return;
	}
	
	Network::Bundle::reclaimPoolObject(pBundle);
	ERROR_MSG("Baseapp::restoreSpaceInCell: not found cellappmgr.\n");
}

//-------------------------------------------------------------------------------------
void Baseapp::createEntityAnywhere(const char* entityType, PyObject* params, PyObject* pyCallback)
{
	std::string strInitData = "";
	uint32 initDataLength = 0;
	if(params != NULL && PyDict_Check(params))
	{
		strInitData = script::Pickler::pickle(params);
		initDataLength = (uint32)strInitData.length();
	}

	Network::Bundle* pBundle = Network::Bundle::createPoolObject();
	(*pBundle).newMessage(BaseappmgrInterface::reqCreateEntityAnywhere);

	(*pBundle) << entityType;
	(*pBundle) << initDataLength;
	if(initDataLength > 0)
		(*pBundle).append(strInitData.data(), initDataLength);

	(*pBundle) << componentID_;

	CALLBACK_ID callbackID = 0;
	if(pyCallback != NULL)
	{
		callbackID = callbackMgr().save(pyCallback);
	}

	(*pBundle) << callbackID;

	Components::ComponentInfos* pComponents = Components::getSingleton().getBaseappmgr();
	if(pComponents)
	{
		if(pComponents->pChannel != NULL)
		{
			pComponents->pChannel->send(pBundle);
		}
		else
		{
			ERROR_MSG("Baseapp::createEntityAnywhere: baseappmgr channel is NULL.\n");
			Network::Bundle::reclaimPoolObject(pBundle);
		}
		
		return;
	}

	Network::Bundle::reclaimPoolObject(pBundle);
	ERROR_MSG("Baseapp::createEntityAnywhere: not found baseappmgr.\n");
}

//-------------------------------------------------------------------------------------
void Baseapp::onCreateEntityAnywhere(Network::Channel* pChannel, MemoryStream& s)
{
	if(pChannel->isExternal())
		return;

	std::string strInitData = "";
	PyObject* params = NULL;
	std::string entityType;
	COMPONENT_ID componentID;
	CALLBACK_ID callbackID;

	s >> entityType;
	s.readBlob(strInitData);

	s >> componentID;
	s >> callbackID;

	if(strInitData.size() > 0)
		params = script::Pickler::unpickle(strInitData);

	Entity* pEntity = createEntity(entityType.c_str(), params);
	Py_XDECREF(params);

	if(pEntity == NULL)
	{
		ERROR_MSG(fmt::format("Baseapp::onCreateEntityAnywhere: create error! entityType={}, componentID={}, callbackID={}\n", 
			entityType, componentID, callbackID));

		return;
	}

	// 如果不是在发起创建entity的baseapp上创建则需要转发回调到发起方
	if(componentID != componentID_)
	{
		Components::ComponentInfos* cinfos = Components::getSingleton().findComponent(componentID);
		if(cinfos == NULL || cinfos->pChannel == NULL)
		{
			Network::Bundle* pBundle = Network::Bundle::createPoolObject();
			ForwardItem* pFI = new ForwardItem();
			pFI->pHandler = NULL;
			pFI->pBundle = pBundle;

			(*pBundle).newMessage(BaseappInterface::onCreateEntityAnywhereCallback);
			(*pBundle) << callbackID;
			(*pBundle) << entityType;
			(*pBundle) << pEntity->id();
			(*pBundle) << componentID_;
			forward_messagebuffer_.push(componentID, pFI);
			WARNING_MSG("Baseapp::onCreateEntityAnywhere: not found baseapp, message is buffered.\n");
			return;
		}

		Network::Channel* lpChannel = cinfos->pChannel;

		// 需要baseappmgr转发给目的baseapp
		Network::Bundle* pForwardbundle = Network::Bundle::createPoolObject();
		(*pForwardbundle).newMessage(BaseappInterface::onCreateEntityAnywhereCallback);
		(*pForwardbundle) << callbackID;
		(*pForwardbundle) << entityType;
		(*pForwardbundle) << pEntity->id();
		(*pForwardbundle) << componentID_;
		lpChannel->send(pForwardbundle);
	}
	else
	{
		ENTITY_ID eid = pEntity->id();
		_onCreateEntityAnywhereCallback(NULL, callbackID, entityType, eid, componentID_);
	}
}

//-------------------------------------------------------------------------------------
void Baseapp::onCreateEntityAnywhereCallback(Network::Channel* pChannel, KBEngine::MemoryStream& s)
{
	if(pChannel->isExternal())
		return;

	CALLBACK_ID callbackID = 0;
	std::string entityType;
	ENTITY_ID eid = 0;
	COMPONENT_ID componentID = 0;
	
	s >> callbackID;
	s >> entityType;
	s >> eid;
	s >> componentID;
	_onCreateEntityAnywhereCallback(pChannel, callbackID, entityType, eid, componentID);
}

//-------------------------------------------------------------------------------------
void Baseapp::_onCreateEntityAnywhereCallback(Network::Channel* pChannel, CALLBACK_ID callbackID, 
	std::string& entityType, ENTITY_ID eid, COMPONENT_ID componentID)
{
	if(callbackID == 0)
	{
		// 没有设定回调
		//ERROR_MSG(fmt::format("Baseapp::_onCreateEntityAnywhereCallback: error(callbackID == 0)! entityType={}, componentID={}\n", 
		//	entityType, componentID));

		return;
	}

	PyObjectPtr pyCallback = callbackMgr().take(callbackID);
	PyObject* pyargs = PyTuple_New(1);

	if(pChannel != NULL)
	{
		ScriptDefModule* sm = EntityDef::findScriptModule(entityType.c_str());
		if(sm == NULL)
		{
			ERROR_MSG(fmt::format("Baseapp::onCreateEntityAnywhereCallback: can't found entityType:{}.\n",
				entityType.c_str()));

			Py_DECREF(pyargs);
			return;
		}
		
		// 如果entity属于另一个baseapp创建则设置它的entityCall
		Network::Channel* pOtherBaseappChannel = Components::getSingleton().findComponent(componentID)->pChannel;
		KBE_ASSERT(pOtherBaseappChannel != NULL);
		PyObject* mb = static_cast<EntityCall*>(new EntityCall(sm, NULL, componentID, eid, ENTITYCALL_TYPE_BASE));
		PyTuple_SET_ITEM(pyargs, 0, mb);
		
		if(pyCallback != NULL)
		{
			PyObject* pyRet = PyObject_CallObject(pyCallback.get(), pyargs);
			if(pyRet == NULL)
			{
				SCRIPT_ERROR_CHECK();
			}
			else
			{
				Py_DECREF(pyRet);
			}
		}
		else
		{
			ERROR_MSG(fmt::format("Baseapp::onCreateEntityAnywhereCallback: can't found callback:{}.\n",
				callbackID));
		}

		//Py_DECREF(mb);
	}
	else
	{
		Entity* pEntity = pEntities_->find(eid);
		if(pEntity == NULL)
		{
			ERROR_MSG(fmt::format("Baseapp::onCreateEntityAnywhereCallback: can't found entity:{}.\n", eid));
			Py_DECREF(pyargs);
			return;
		}

		Py_INCREF(pEntity);
		PyTuple_SET_ITEM(pyargs, 0, pEntity);

		SCOPED_PROFILE(SCRIPTCALL_PROFILE);

		if(pyCallback != NULL)
		{
			PyObject* pyRet = PyObject_CallObject(pyCallback.get(), pyargs);
			if(pyRet == NULL)
			{
				SCRIPT_ERROR_CHECK();
			}
			else
			{
				Py_DECREF(pyRet);
			}
		}
		else
		{
			ERROR_MSG(fmt::format("Baseapp::onCreateEntityAnywhereCallback: can't found callback:{}.\n",
				callbackID));
		}
	}

	SCRIPT_ERROR_CHECK();
	Py_DECREF(pyargs);
}

//-------------------------------------------------------------------------------------
void Baseapp::createEntityRemotely(const char* entityType, COMPONENT_ID componentID, PyObject* params, PyObject* pyCallback)
{
	std::string strInitData = "";
	uint32 initDataLength = 0;
	if (params != NULL && PyDict_Check(params))
	{
		strInitData = script::Pickler::pickle(params);
		initDataLength = (uint32)strInitData.length();
	}

	Network::Bundle* pBundle = Network::Bundle::createPoolObject();
	(*pBundle).newMessage(BaseappmgrInterface::reqCreateEntityRemotely);

	// 创建到这个组件上
	(*pBundle) << componentID;

	(*pBundle) << entityType;
	(*pBundle) << initDataLength;

	if (initDataLength > 0)
		(*pBundle).append(strInitData.data(), initDataLength);

	(*pBundle) << componentID_;

	CALLBACK_ID callbackID = 0;
	if (pyCallback != NULL)
	{
		callbackID = callbackMgr().save(pyCallback);
	}

	(*pBundle) << callbackID;

	Components::ComponentInfos* pComponents = Components::getSingleton().getBaseappmgr();
	if (pComponents)
	{
		if (pComponents->pChannel != NULL)
		{
			pComponents->pChannel->send(pBundle);
		}
		else
		{
			ERROR_MSG("Baseapp::createEntityRemotely: baseappmgr channel is NULL.\n");
			Network::Bundle::reclaimPoolObject(pBundle);
		}

		return;
	}

	Network::Bundle::reclaimPoolObject(pBundle);
	ERROR_MSG("Baseapp::createEntityRemotely: not found baseappmgr.\n");
}

//-------------------------------------------------------------------------------------
void Baseapp::onCreateEntityRemotely(Network::Channel* pChannel, MemoryStream& s)
{
	if (pChannel->isExternal())
		return;

	std::string strInitData = "";
	PyObject* params = NULL;
	std::string entityType;
	COMPONENT_ID reqComponentID;
	CALLBACK_ID callbackID;

	s >> entityType;
	s.readBlob(strInitData);

	s >> reqComponentID;
	s >> callbackID;

	if (strInitData.size() > 0)
		params = script::Pickler::unpickle(strInitData);

	Entity* pEntity = createEntity(entityType.c_str(), params);
	Py_XDECREF(params);

	if (pEntity == NULL)
	{
		ERROR_MSG(fmt::format("Baseapp::onCreateEntityRemotely: create error! entityType={}, reqComponentID={}, callbackID={}\n",
			entityType, reqComponentID, callbackID));

		return;
	}

	// 如果不是在发起创建entity的baseapp上创建则需要转发回调到发起方
	if (reqComponentID != componentID_)
	{
		Components::ComponentInfos* cinfos = Components::getSingleton().findComponent(reqComponentID);
		if (cinfos == NULL || cinfos->pChannel == NULL)
		{
			Network::Bundle* pBundle = Network::Bundle::createPoolObject();
			ForwardItem* pFI = new ForwardItem();
			pFI->pHandler = NULL;
			pFI->pBundle = pBundle;

			(*pBundle).newMessage(BaseappInterface::onCreateEntityAnywhereCallback);
			(*pBundle) << callbackID;
			(*pBundle) << entityType;
			(*pBundle) << pEntity->id();
			(*pBundle) << componentID_;
			forward_messagebuffer_.push(reqComponentID, pFI);
			WARNING_MSG("Baseapp::onCreateEntityRemotely: not found baseapp, message is buffered.\n");
			return;
		}

		Network::Channel* lpChannel = cinfos->pChannel;

		// 需要baseappmgr转发给目的baseapp
		Network::Bundle* pForwardbundle = Network::Bundle::createPoolObject();
		(*pForwardbundle).newMessage(BaseappInterface::onCreateEntityRemotelyCallback);
		(*pForwardbundle) << callbackID;
		(*pForwardbundle) << entityType;
		(*pForwardbundle) << pEntity->id();
		(*pForwardbundle) << componentID_;
		lpChannel->send(pForwardbundle);
	}
	else
	{
		ENTITY_ID eid = pEntity->id();
		_onCreateEntityAnywhereCallback(NULL, callbackID, entityType, eid, componentID_);
	}
}

//-------------------------------------------------------------------------------------
void Baseapp::onCreateEntityRemotelyCallback(Network::Channel* pChannel, KBEngine::MemoryStream& s)
{
	if (pChannel->isExternal())
		return;

	CALLBACK_ID callbackID = 0;
	std::string entityType;
	ENTITY_ID eid = 0;
	COMPONENT_ID componentID = 0;

	s >> callbackID;
	s >> entityType;
	s >> eid;
	s >> componentID;
	_onCreateEntityRemotelyCallback(pChannel, callbackID, entityType, eid, componentID);
}

//-------------------------------------------------------------------------------------
void Baseapp::_onCreateEntityRemotelyCallback(Network::Channel* pChannel, CALLBACK_ID callbackID,
	std::string& entityType, ENTITY_ID eid, COMPONENT_ID componentID)
{
	if (callbackID == 0)
	{
		// 没有设定回调
		//ERROR_MSG(fmt::format("Baseapp::_onCreateEntityRemotelyCallback: error(callbackID == 0)! entityType={}, componentID={}\n", 
		//	entityType, componentID));

		return;
	}

	PyObjectPtr pyCallback = callbackMgr().take(callbackID);
	PyObject* pyargs = PyTuple_New(1);

	if (pChannel != NULL)
	{
		ScriptDefModule* sm = EntityDef::findScriptModule(entityType.c_str());
		if (sm == NULL)
		{
			ERROR_MSG(fmt::format("Baseapp::onCreateEntityRemotelyCallback: can't found entityType:{}.\n",
				entityType.c_str()));

			Py_DECREF(pyargs);
			return;
		}

		// 如果entity属于另一个baseapp创建则设置它的entityCall
		Network::Channel* pOtherBaseappChannel = Components::getSingleton().findComponent(componentID)->pChannel;
		KBE_ASSERT(pOtherBaseappChannel != NULL);
		PyObject* mb = static_cast<EntityCall*>(new EntityCall(sm, NULL, componentID, eid, ENTITYCALL_TYPE_BASE));
		PyTuple_SET_ITEM(pyargs, 0, mb);

		if (pyCallback != NULL)
		{
			PyObject* pyRet = PyObject_CallObject(pyCallback.get(), pyargs);
			if (pyRet == NULL)
			{
				SCRIPT_ERROR_CHECK();
			}
			else
			{
				Py_DECREF(pyRet);
			}
		}
		else
		{
			ERROR_MSG(fmt::format("Baseapp::onCreateEntityRemotelyCallback: can't found callback:{}.\n",
				callbackID));
		}

		//Py_DECREF(mb);
	}
	else
	{
		Entity* pEntity = pEntities_->find(eid);
		if (pEntity == NULL)
		{
			ERROR_MSG(fmt::format("Baseapp::onCreateEntityRemotelyCallback: can't found entity:{}.\n", eid));
			Py_DECREF(pyargs);
			return;
		}

		Py_INCREF(pEntity);
		PyTuple_SET_ITEM(pyargs, 0, pEntity);

		SCOPED_PROFILE(SCRIPTCALL_PROFILE);

		if (pyCallback != NULL)
		{
			PyObject* pyRet = PyObject_CallObject(pyCallback.get(), pyargs);
			if (pyRet == NULL)
			{
				SCRIPT_ERROR_CHECK();
			}
			else
			{
				Py_DECREF(pyRet);
			}
		}
		else
		{
			ERROR_MSG(fmt::format("Baseapp::onCreateEntityRemotelyCallback: can't found callback:{}.\n",
				callbackID));
		}
	}

	SCRIPT_ERROR_CHECK();
	Py_DECREF(pyargs);
}

//-------------------------------------------------------------------------------------
void Baseapp::createCellEntity(EntityCallAbstract* createToCellEntityCall, Entity* pEntity)
{
	if(pEntity->cellEntityCall())
	{
		ERROR_MSG(fmt::format("Baseapp::createCellEntity: {} {} has a cell!\n",
			pEntity->scriptName(), pEntity->id()));

		return;
	}

	Network::Bundle* pBundle = Network::Bundle::createPoolObject();
	(*pBundle).newMessage(CellappInterface::onCreateCellEntityFromBaseapp);

	ENTITY_ID id = pEntity->id();
	std::string entityType = pEntity->ob_type->tp_name;

	EntityCall* clientEntityCall = pEntity->clientEntityCall();
	bool hasClient = (clientEntityCall != NULL);
	
	(*pBundle) << createToCellEntityCall->id();				// 在这个entityCall所在的cellspace上创建
	(*pBundle) << entityType;
	(*pBundle) << id;
	(*pBundle) << componentID_;
	(*pBundle) << hasClient;
	(*pBundle) << pEntity->inRestore();

	MemoryStream* s = MemoryStream::createPoolObject();
	pEntity->addCellDataToStream(CELLAPP_TYPE, ED_FLAG_ALL, s);
	(*pBundle).append(*s);
	MemoryStream::reclaimPoolObject(s);
	
	if(createToCellEntityCall->getChannel() == NULL)
	{
		ERROR_MSG(fmt::format("Baseapp::createCellEntity: not found cellapp(createToCellEntityCall:"
			"componentID={}, entityID={}), create error!\n",
			createToCellEntityCall->componentID(), createToCellEntityCall->id()));

		pEntity->onCreateCellFailure();
		Network::Bundle::reclaimPoolObject(pBundle);
		return;
	}

	createToCellEntityCall->getChannel()->send(pBundle);
}

//-------------------------------------------------------------------------------------
void Baseapp::onCreateCellFailure(Network::Channel* pChannel, ENTITY_ID entityID)
{
	if(pChannel->isExternal())
		return;

	Entity* pEntity = pEntities_->find(entityID);

	// 可能客户端在期间掉线了
	if(pEntity == NULL)
	{
		ERROR_MSG(fmt::format("Baseapp::onCreateCellFailure: not found entity({})!\n", entityID));
		return;
	}

	pEntity->onCreateCellFailure();
}

//-------------------------------------------------------------------------------------
void Baseapp::onEntityGetCell(Network::Channel* pChannel, ENTITY_ID id, 
							  COMPONENT_ID componentID, SPACE_ID spaceID)
{
	if(pChannel->isExternal())
		return;

	Entity* pEntity = pEntities_->find(id);

	// DEBUG_MSG("Baseapp::onEntityGetCell: entityID %d.\n", id);
	
	// 可能客户端在期间掉线了
	if(pEntity == NULL)
	{
		Network::Bundle* pBundle = Network::Bundle::createPoolObject();

		(*pBundle).newMessage(CellappInterface::onDestroyCellEntityFromBaseapp);
		(*pBundle) << id;
		pChannel->send(pBundle);
		ERROR_MSG(fmt::format("Baseapp::onEntityGetCell: not found entity({}), I will destroyEntityCell!\n", id));
		return;
	}

	if(pEntity->spaceID() != spaceID)
		pEntity->spaceID(spaceID);

	// 如果是有客户端的entity则需要告知客户端， 自身entity已经进入世界了。
	if(pEntity->clientEntityCall() != NULL)
	{
		onClientEntityEnterWorld(static_cast<Proxy*>(pEntity), componentID);
	}

	pEntity->onGetCell(pChannel, componentID);
}

//-------------------------------------------------------------------------------------
void Baseapp::onClientEntityEnterWorld(Proxy* pEntity, COMPONENT_ID componentID)
{
	Py_INCREF(pEntity);
	pEntity->initClientCellPropertys();
	pEntity->onClientGetCell(NULL, componentID);
	Py_DECREF(pEntity);
}

//-------------------------------------------------------------------------------------
bool Baseapp::createClientProxies(Proxy* pEntity, bool reload)
{
	Py_INCREF(pEntity);
	
	// 将通道代理的关系与该entity绑定， 在后面通信中可提供身份合法性识别
	Network::Channel* pChannel = pEntity->clientEntityCall()->getChannel();
	pChannel->proxyID(pEntity->id());
	pEntity->addr(pChannel->addr());

	// 重新生成一个ID
	if(reload)
		pEntity->rndUUID(genUUID64());
	
	// 一些数据必须在实体创建后立即访问
	pEntity->initClientBasePropertys();

	// 让客户端知道已经创建了proxices, 并初始化一部分属性
	Network::Bundle* pBundle = Network::Bundle::createPoolObject();
	(*pBundle).newMessage(ClientInterface::onCreatedProxies);
	(*pBundle) << pEntity->rndUUID();
	(*pBundle) << pEntity->id();
	(*pBundle) << pEntity->ob_type->tp_name;
	//pEntity->clientEntityCall()->sendCall((*pBundle));
	pEntity->sendToClient(ClientInterface::onCreatedProxies, pBundle);

	// 本应该由客户端告知已经创建好entity后调用这个接口。
	//if(!reload)
	pEntity->onClientEnabled();
	Py_DECREF(pEntity);
	return true;
}

//-------------------------------------------------------------------------------------
PyObject* Baseapp::__py_executeRawDatabaseCommand(PyObject* self, PyObject* args)
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
	else if(argCount == 3)
		ret = PyArg_ParseTuple(args, "s#|O|i", &data, &size, &pycallback, &eid);
	else if(argCount == 2)
		ret = PyArg_ParseTuple(args, "s#|O", &data, &size, &pycallback);
	else if(argCount == 1)
		ret = PyArg_ParseTuple(args, "s#", &data, &size);

	if(ret == -1)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::executeRawDatabaseCommand: args error!");
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

	Baseapp::getSingleton().executeRawDatabaseCommand(data, (uint32)size, pycallback, eid, dbInterfaceName);
	S_Return;
}

//-------------------------------------------------------------------------------------
void Baseapp::executeRawDatabaseCommand(const char* datas, uint32 size, PyObject* pycallback, ENTITY_ID eid, const std::string& dbInterfaceName)
{
	if(datas == NULL)
	{
		ERROR_MSG("KBEngine::executeRawDatabaseCommand: execute error!\n");
		return;
	}

	Components::ComponentInfos* dbmgrinfos = Components::getSingleton().getDbmgr();
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

	//INFO_MSG(fmt::format("KBEngine::executeRawDatabaseCommand{}:{}.\n", 
	//	(eid > 0 ? (fmt::format("(entityID={})", eid)) : ""), datas));

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
void Baseapp::onExecuteRawDatabaseCommandCB(Network::Channel* pChannel, KBEngine::MemoryStream& s)
{
	if(pChannel->isExternal())
		return;
	
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

	//DEBUG_MSG(fmt::format("Baseapp::onExecuteRawDatabaseCommandCB: nrows={}, nfields={}, err={}.\n", 
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
			ERROR_MSG(fmt::format("Baseapp::onExecuteRawDatabaseCommandCB: can't found callback:{}.\n",
				callbackID));
		}
	}

	Py_XDECREF(pResultSet);
	Py_XDECREF(pAffectedRows);
	Py_XDECREF(pLastInsertID);
	Py_XDECREF(pErrorMsg);
}

//-------------------------------------------------------------------------------------
PyObject* Baseapp::__py_charge(PyObject* self, PyObject* args)
{
	if(PyTuple_Size(args) != 4)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::charge: args != (ordersID, dbid, byteDatas[bytes], pycallback)!");
		PyErr_PrintEx(0);
		return NULL;
	}

	PyObject* pyDatas = NULL, *pycallback = NULL;
	char* pChargeID = NULL;
	DBID dbid = 0;

	if(PyArg_ParseTuple(args, "s|K|O|O", &pChargeID, &dbid, &pyDatas, &pycallback) == -1)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::charge: args error!");
		PyErr_PrintEx(0);
		return NULL;
	}
	
	if (pChargeID == NULL)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::charge: ordersID not is string!");
		PyErr_PrintEx(0);
		return NULL;
	}

	if(strlen(pChargeID) <= 0)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::charge: ordersID is NULL!");
		PyErr_PrintEx(0);
		return NULL;
	}

	if(dbid == 0)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::charge: dbid is 0!");
		PyErr_PrintEx(0);
		return NULL;
	}

	if (!PyBytes_Check(pyDatas))
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::charge: byteDatas != bytes!");
		PyErr_PrintEx(0);
		return NULL;
	}
	
	if(!PyCallable_Check(pycallback))
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::charge: invalid pycallback!");
		PyErr_PrintEx(0);
		return NULL;
	}

	std::string datas;

	char *buffer;
	Py_ssize_t length;

	if(PyBytes_AsStringAndSize(pyDatas, &buffer, &length) < 0)
	{
		SCRIPT_ERROR_CHECK();
		return NULL;
	}

	datas.assign(buffer, length);

	if(Baseapp::getSingleton().isShuttingdown())
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::charge(%s): shuttingdown, operation not allowed! dbid=%" PRIu64, 
			pChargeID, dbid);

		PyErr_PrintEx(0);
		return NULL;
	}

	Baseapp::getSingleton().charge(pChargeID, dbid, datas, pycallback);
	S_Return;
}

//-------------------------------------------------------------------------------------
void Baseapp::charge(std::string chargeID, DBID dbid, const std::string& datas, PyObject* pycallback)
{
	CALLBACK_ID callbackID = callbackMgr().save(pycallback, uint64(g_kbeSrvConfig.interfaces_orders_timeout_ + 
		g_kbeSrvConfig.callback_timeout_));

	INFO_MSG(fmt::format("Baseapp::charge: chargeID={0}, dbid={3}, datas={1}, pycallback={2}.\n", 
		chargeID,
		datas,
		callbackID,
		dbid));

	Network::Bundle* pBundle = Network::Bundle::createPoolObject();

	(*pBundle).newMessage(DbmgrInterface::charge);
	(*pBundle) << chargeID;
	(*pBundle) << dbid;
	(*pBundle).appendBlob(datas);
	(*pBundle) << callbackID;

	Network::Channel* pChannel = Components::getSingleton().getDbmgrChannel();

	if(pChannel == NULL)
	{
		ERROR_MSG("Baseapp::charge: not found dbmgr!\n");
		Network::Bundle::reclaimPoolObject(pBundle);
		return;
	}

	pChannel->send(pBundle);
}

//-------------------------------------------------------------------------------------
void Baseapp::onChargeCB(Network::Channel* pChannel, KBEngine::MemoryStream& s)
{
	if(pChannel->isExternal())
		return;
	
	std::string chargeID;
	CALLBACK_ID callbackID;
	std::string datas;
	DBID dbid;
	SERVER_ERROR_CODE retcode;

	s >> chargeID;
	s >> dbid;
	s.readBlob(datas);
	s >> callbackID;
	s >> retcode;

	INFO_MSG(fmt::format("Baseapp::onChargeCB: chargeID={0}, dbid={3}, datas={1}, pycallback={2}.\n", 
		chargeID,
		datas,
		callbackID,
		dbid));

	PyObject* pyOrder = PyUnicode_FromString(chargeID.c_str());
	PyObject* pydbid = PyLong_FromUnsignedLongLong(dbid);
	PyObject* pySuccess = PyBool_FromLong((retcode == SERVER_SUCCESS));
	PyObject* pyBytes = PyBytes_FromStringAndSize(datas.data(), datas.length());

	if(callbackID > 0)
	{
		PyObjectPtr pycallback = callbackMgr().take(callbackID);

		if(pycallback != NULL)
		{
			SCOPED_PROFILE(SCRIPTCALL_PROFILE);
			PyObject* pyResult = PyObject_CallFunction(pycallback.get(), 
												const_cast<char*>("OOOO"), 
												pyOrder, pydbid, pySuccess, pyBytes);

			if(pyResult != NULL)
				Py_DECREF(pyResult);
			else
				SCRIPT_ERROR_CHECK();
		}
		else
		{
			ERROR_MSG(fmt::format("Baseapp::onChargeCB: can't found callback:{}.\n",
				callbackID));
		}
	}
	else
	{
		SCOPED_PROFILE(SCRIPTCALL_PROFILE);
		PyObject* pyResult = PyObject_CallMethod(getEntryScript().get(), 
										const_cast<char*>("onLoseChargeCB"), 
										const_cast<char*>("OOOO"), 
										pyOrder, pydbid, pySuccess, pyBytes);

		if(pyResult != NULL)
			Py_DECREF(pyResult);
		else
			SCRIPT_ERROR_CHECK();
	}

	Py_DECREF(pyOrder);
	Py_DECREF(pydbid);
	Py_DECREF(pySuccess);
	Py_DECREF(pyBytes);
}

//-------------------------------------------------------------------------------------
void Baseapp::onDbmgrInitCompleted(Network::Channel* pChannel, 
		GAME_TIME gametime, ENTITY_ID startID, ENTITY_ID endID, 
		COMPONENT_ORDER startGlobalOrder, COMPONENT_ORDER startGroupOrder, 
		const std::string& digest)
{
	if(pChannel->isExternal())
		return;

	EntityApp<Entity>::onDbmgrInitCompleted(pChannel, gametime, startID, endID,
		startGlobalOrder, startGroupOrder, digest);

	// 再次同步自己的新信息(startGlobalOrder, startGroupOrder等)到machine
	Components::getSingleton().broadcastSelf();

	// 这里需要更新一下python的环境变量
	this->getScript().setenv("KBE_BOOTIDX_GLOBAL", getenv("KBE_BOOTIDX_GLOBAL"));
	this->getScript().setenv("KBE_BOOTIDX_GROUP", getenv("KBE_BOOTIDX_GROUP"));

	SCOPED_PROFILE(SCRIPTCALL_PROFILE);

	PyObject* pyResult = PyObject_CallMethod(getEntryScript().get(), 
										const_cast<char*>("onInit"), 
										const_cast<char*>("i"), 
										0);

	if(pyResult != NULL)
		Py_DECREF(pyResult);
	else
		SCRIPT_ERROR_CHECK();

	pInitProgressHandler_ = new InitProgressHandler(this->networkInterface());
}

//-------------------------------------------------------------------------------------
void Baseapp::onBroadcastBaseAppDataChanged(Network::Channel* pChannel, KBEngine::MemoryStream& s)
{
	if(pChannel->isExternal())
		return;

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
		ERROR_MSG("Baseapp::onBroadcastBaseAppDataChanged: no has key!\n");
		return;
	}

	if(isDelete)
	{
		if(pBaseAppData_->del(pyKey))
		{
			SCOPED_PROFILE(SCRIPTCALL_PROFILE);

			// 通知脚本
			SCRIPT_OBJECT_CALL_ARGS1(getEntryScript().get(), const_cast<char*>("onBaseAppDataDel"), 
				const_cast<char*>("O"), pyKey, false);
		}
	}
	else
	{
		PyObject * pyValue = script::Pickler::unpickle(value);
		if(pyValue == NULL)
		{
			ERROR_MSG("Baseapp::onBroadcastBaseAppDataChanged: no has value!\n");
			Py_DECREF(pyKey);
			return;
		}

		if(pBaseAppData_->write(pyKey, pyValue))
		{
			SCOPED_PROFILE(SCRIPTCALL_PROFILE);

			// 通知脚本
			SCRIPT_OBJECT_CALL_ARGS2(getEntryScript().get(), const_cast<char*>("onBaseAppData"), 
				const_cast<char*>("OO"), pyKey, pyValue, false);
		}

		Py_DECREF(pyValue);
	}

	Py_DECREF(pyKey);
}

//-------------------------------------------------------------------------------------
void Baseapp::registerPendingLogin(Network::Channel* pChannel, KBEngine::MemoryStream& s)
{
	if(pChannel->isExternal())
	{
		s.done();
		return;
	}

	std::string									loginName; 
	std::string									accountName;
	std::string									password;
	std::string									datas;
	ENTITY_ID									entityID;
	DBID										entityDBID;
	uint32										flags;
	uint64										deadline;
	int											clientType;
	bool										forceInternalLogin;
	bool										needCheckPassword;

	s >> loginName >> accountName >> password >> needCheckPassword >> entityID >> entityDBID >> flags >> deadline >> clientType >> forceInternalLogin;
	s.readBlob(datas);

	Network::Bundle* pBundle = Network::Bundle::createPoolObject();
	(*pBundle).newMessage(BaseappmgrInterface::onPendingAccountGetBaseappAddr);

	(*pBundle) << loginName;
	(*pBundle) << accountName;
	
	if (!forceInternalLogin && strlen((const char*)&g_kbeSrvConfig.getBaseApp().externalAddress) > 0)
	{
		(*pBundle) << g_kbeSrvConfig.getBaseApp().externalAddress;
	}
	else
	{
		(*pBundle) << inet_ntoa((struct in_addr&)networkInterface().extTcpAddr().ip);
	}

	(*pBundle) << this->networkInterface().extTcpAddr().port;
	(*pBundle) << this->networkInterface().extUdpAddr().port;

	pChannel->send(pBundle);

	PendingLoginMgr::PLInfos* ptinfos = new PendingLoginMgr::PLInfos;
	ptinfos->accountName = accountName;
	ptinfos->password = password;
	ptinfos->entityID = entityID;
	ptinfos->entityDBID = entityDBID;
	ptinfos->flags = flags;
	ptinfos->deadline = deadline;
	ptinfos->ctype = (COMPONENT_CLIENT_TYPE)clientType;
	ptinfos->datas = datas;
	ptinfos->needCheckPassword = needCheckPassword;
	pendingLoginMgr_.add(ptinfos);
}

//-------------------------------------------------------------------------------------
void Baseapp::loginBaseappFailed(Network::Channel* pChannel, std::string& accountName, 
								 SERVER_ERROR_CODE failedcode, bool relogin)
{
	if(failedcode == SERVER_ERR_NAME)
	{
		DEBUG_MSG(fmt::format("Baseapp::login: not found user[{}], login is failed!\n",
			accountName.c_str()));

		failedcode = SERVER_ERR_NAME_PASSWORD;
	}
	else if(failedcode == SERVER_ERR_PASSWORD)
	{
		DEBUG_MSG(fmt::format("Baseapp::login: user[{}] password error, login is failed!\n",
			accountName.c_str()));

		failedcode = SERVER_ERR_NAME_PASSWORD;
	}

	if(pChannel == NULL)
		return;

	Network::Bundle* pBundle = Network::Bundle::createPoolObject();

	if(relogin)
		(*pBundle).newMessage(ClientInterface::onReloginBaseappFailed);
	else
		(*pBundle).newMessage(ClientInterface::onLoginBaseappFailed);

	ClientInterface::onLoginBaseappFailedArgs1::staticAddToBundle((*pBundle), failedcode);
	pChannel->send(pBundle);
}

//-------------------------------------------------------------------------------------
void Baseapp::loginBaseapp(Network::Channel* pChannel, 
						   std::string& accountName, 
						   std::string& password)
{
	accountName = KBEngine::strutil::kbe_trim(accountName);
	if(accountName.size() > ACCOUNT_NAME_MAX_LENGTH)
	{
		ERROR_MSG(fmt::format("Baseapp::loginBaseapp: accountName too big, size={}, limit={}.\n",
			accountName.size(), ACCOUNT_NAME_MAX_LENGTH));

		return;
	}

	if(password.size() > ACCOUNT_PASSWD_MAX_LENGTH)
	{
		ERROR_MSG(fmt::format("Baseapp::loginBaseapp: password too big, size={}, limit={}.\n",
			password.size(), ACCOUNT_PASSWD_MAX_LENGTH));

		return;
	}

	INFO_MSG(fmt::format("Baseapp::loginBaseapp: new user[{0}], channel[{1}].\n", 
		accountName, pChannel->c_str()));

	Components::ComponentInfos* dbmgrinfos = Components::getSingleton().getDbmgr();
	if(dbmgrinfos == NULL || dbmgrinfos->pChannel == NULL || dbmgrinfos->cid == 0)
	{
		loginBaseappFailed(pChannel, accountName, SERVER_ERR_SRV_NO_READY);
		return;
	}

	PendingLoginMgr::PLInfos* ptinfos = pendingLoginMgr_.find(accountName);
	if(ptinfos == NULL)
	{
		loginBaseappFailed(pChannel, accountName, SERVER_ERR_ILLEGAL_LOGIN);
		return;
	}
	else if (!ptinfos->addr.isNone() && ptinfos->addr != pChannel->addr())
	{
		loginBaseappFailed(pChannel, accountName, SERVER_ERR_ILLEGAL_LOGIN);
		return;
	}

	if(ptinfos->password != password)
	{
		loginBaseappFailed(pChannel, accountName, SERVER_ERR_PASSWORD);
		pendingLoginMgr_.removeNextTick(accountName);
		return;
	}

	if((ptinfos->flags & ACCOUNT_FLAG_LOCK) > 0)
	{
		loginBaseappFailed(pChannel, accountName, SERVER_ERR_ACCOUNT_LOCK);
		pendingLoginMgr_.removeNextTick(accountName);
		return;
	}

	if((ptinfos->flags & ACCOUNT_FLAG_NOT_ACTIVATED) > 0)
	{
		loginBaseappFailed(pChannel, accountName, SERVER_ERR_ACCOUNT_NOT_ACTIVATED);
		pendingLoginMgr_.removeNextTick(accountName);
		return;
	}

	if(ptinfos->deadline > 0 && ::time(NULL) - ptinfos->deadline <= 0)
	{
		loginBaseappFailed(pChannel, accountName, SERVER_ERR_ACCOUNT_DEADLINE);
		pendingLoginMgr_.removeNextTick(accountName);
		return;
	}

	if(idClient_.size() == 0)
	{
		ERROR_MSG("Baseapp::loginBaseapp: idClient size is 0.\n");
		loginBaseappFailed(pChannel, accountName, SERVER_ERR_SRV_NO_READY);
		pendingLoginMgr_.removeNextTick(accountName);
		return;
	}

	// 虽然接入第三方dbmgr不检查密码，但至少在loginapp时提交的password应该跟本次提交的能匹配上
	// 否则容易被其他连接攻击式的试探登陆
	if (!ptinfos->needCheckPassword && ptinfos->password != password)
	{
		loginBaseappFailed(pChannel, accountName, SERVER_ERR_NAME_PASSWORD);
		pendingLoginMgr_.removeNextTick(accountName);
		return;
	}

	// 如果entityID大于0则说明此entity是存活状态登录
	if(ptinfos->entityID > 0)
	{
		INFO_MSG(fmt::format("Baseapp::loginBaseapp: user[{}] has entity({}).\n",
			accountName.c_str(), ptinfos->entityID));

		pendingLoginMgr_.removeNextTick(accountName);

		Proxy* pEntity = static_cast<Proxy*>(findEntity(ptinfos->entityID));
		if(pEntity == NULL || pEntity->isDestroyed())
		{
			loginBaseappFailed(pChannel, accountName, SERVER_ERR_BUSY);
			return;
		}

		// 防止在onLogOnAttempt中销毁了
		Py_INCREF(pEntity);

		// 通知脚本异常登录请求有脚本决定是否允许这个通道强制登录
		int32 ret = pEntity->onLogOnAttempt(pChannel->addr().ipAsString(), 
			ntohs(pChannel->addr().port), password.c_str());

		if (pEntity->isDestroyed())
		{
			Py_DECREF(pEntity);

			loginBaseappFailed(pChannel, accountName, SERVER_ERR_OP_FAILED);
			return;
		}

		switch(ret)
		{
		case LOG_ON_ACCEPT:
			if(pEntity->clientEntityCall() != NULL)
			{
				// 通告在别处登录
				Network::Channel* pOldClientChannel = pEntity->clientEntityCall()->getChannel();
				if(pOldClientChannel != NULL)
				{
					INFO_MSG(fmt::format("Baseapp::loginBaseapp: script LOG_ON_ACCEPT. oldClientChannel={}\n",
						pOldClientChannel->c_str()));
					
					kickChannel(pOldClientChannel, SERVER_ERR_ACCOUNT_LOGIN_ANOTHER);
				}
				else
				{
					INFO_MSG("Baseapp::loginBaseapp: script LOG_ON_ACCEPT.\n");
				}
				
				pEntity->clientEntityCall()->addr(pChannel->addr());
				pEntity->addr(pChannel->addr());
				pEntity->setClientType(ptinfos->ctype);
				pEntity->setLoginDatas(ptinfos->datas);
				createClientProxies(pEntity, true);
				pEntity->onGetWitness();
			}
			else
			{
				// 创建entity的客户端entityCall
				EntityCall* entityClientEntityCall = new EntityCall(pEntity->pScriptModule(), 
					&pChannel->addr(), 0, pEntity->id(), ENTITYCALL_TYPE_CLIENT);

				pEntity->clientEntityCall(entityClientEntityCall);
				pEntity->addr(pChannel->addr());
				pEntity->setClientType(ptinfos->ctype);
				pEntity->setLoginDatas(ptinfos->datas);

				// 将通道代理的关系与该entity绑定， 在后面通信中可提供身份合法性识别
				entityClientEntityCall->getChannel()->proxyID(pEntity->id());
				createClientProxies(pEntity, true);
				pEntity->onGetWitness();
			}
			break;
		case LOG_ON_WAIT_FOR_DESTROY:
		default:
			INFO_MSG("Baseapp::loginBaseapp: script LOG_ON_REJECT.\n");
			loginBaseappFailed(pChannel, accountName, SERVER_ERR_ACCOUNT_IS_ONLINE);
			Py_DECREF(pEntity);
			return;
		};

		Py_DECREF(pEntity);
	}
	else
	{
		Network::Bundle* pBundle = Network::Bundle::createPoolObject();
		(*pBundle).newMessage(DbmgrInterface::queryAccount);

		ENTITY_ID entityID = idClient_.alloc();
		KBE_ASSERT(entityID > 0);

		DbmgrInterface::queryAccountArgs8::staticAddToBundle((*pBundle), accountName, password, ptinfos->needCheckPassword, g_componentID,
			entityID, ptinfos->entityDBID, pChannel->addr().ip, pChannel->addr().port);

		dbmgrinfos->pChannel->send(pBundle);
	}

	// 记录客户端地址
	ptinfos->addr = pChannel->addr();
}

//-------------------------------------------------------------------------------------
void Baseapp::reloginBaseapp(Network::Channel* pChannel, std::string& accountName, 
							 std::string& password, uint64 key, ENTITY_ID entityID)
{
	accountName = KBEngine::strutil::kbe_trim(accountName);
	INFO_MSG(fmt::format("Baseapp::reloginBaseapp: accountName={}, key={}, entityID={}.\n",
		accountName, key, entityID));

	Entity* pEntity = findEntity(entityID);
	if(pEntity == NULL || !PyObject_TypeCheck(pEntity, Proxy::getScriptType()) || pEntity->isDestroyed())
	{
		loginBaseappFailed(pChannel, accountName, SERVER_ERR_ILLEGAL_LOGIN, true);
		return;
	}
	
	Proxy* proxy = static_cast<Proxy*>(pEntity);
	
	if(key == 0 || proxy->rndUUID() != key)
	{
		loginBaseappFailed(pChannel, accountName, SERVER_ERR_ILLEGAL_LOGIN, true);
		return;
	}

	EntityCall* entityClientEntityCall = proxy->clientEntityCall();
	if(entityClientEntityCall != NULL)
	{
		Network::Channel* pMBChannel = entityClientEntityCall->getChannel();

		WARNING_MSG(fmt::format("Baseapp::reloginBaseapp: accountName={}, key={}, "
			"entityID={}, ClientEntityCall({}) is exist, will be kicked out!\n",
			accountName, key, entityID, 
			(pMBChannel ? pMBChannel->c_str() : "unknown")));
		
		if(pMBChannel)
		{
			pMBChannel->proxyID(0);
			pMBChannel->condemn();
		}

		entityClientEntityCall->addr(pChannel->addr());
	}
	else
	{
		// 创建entity的客户端entityCall
		entityClientEntityCall = new EntityCall(proxy->pScriptModule(), 
			&pChannel->addr(), 0, proxy->id(), ENTITYCALL_TYPE_CLIENT);

		proxy->clientEntityCall(entityClientEntityCall);
	}

	// 将通道代理的关系与该entity绑定， 在后面通信中可提供身份合法性识别
	proxy->addr(pChannel->addr());
	pChannel->proxyID(proxy->id());
	proxy->rndUUID(KBEngine::genUUID64());

	// 客户端重连也需要将完整的数据重发给客户端， 相当于登录之后获得的数据。
	// 因为断线期间不能确保包括场景等数据已发生变化
	// 客户端需要重建所有数据
	Py_INCREF(proxy);
	createClientProxies(proxy, true);
	proxy->onGetWitness();
	Py_DECREF(proxy);
	// proxy->onClientEnabled();

	Network::Bundle* pBundle = Network::Bundle::createPoolObject();
	(*pBundle).newMessage(ClientInterface::onReloginBaseappSuccessfully);
	(*pBundle) << proxy->rndUUID();
	pChannel->send(pBundle);
}

//-------------------------------------------------------------------------------------
void Baseapp::kickChannel(Network::Channel* pChannel, SERVER_ERROR_CODE failedcode)
{
	if(pChannel == NULL)
		return;

	INFO_MSG(fmt::format("Baseapp::kickChannel: pChannel={}, failedcode={}, proxyID={}.\n",
		pChannel->c_str(), failedcode, pChannel->proxyID()));

	Network::Bundle* pBundle = Network::Bundle::createPoolObject();
	(*pBundle).newMessage(ClientInterface::onKicked);
	ClientInterface::onKickedArgs1::staticAddToBundle((*pBundle), failedcode);
	pChannel->send(pBundle);

	pChannel->proxyID(0);
	pChannel->condemn();
}

//-------------------------------------------------------------------------------------
void Baseapp::onQueryAccountCBFromDbmgr(Network::Channel* pChannel, KBEngine::MemoryStream& s)
{
	if(pChannel->isExternal())
		return;

	std::string accountName;
	std::string password;
	bool success = false;
	DBID dbid;
	ENTITY_ID entityID;
	uint32 flags;
	uint64 deadline;
	uint16 dbInterfaceIndex;

	s >> dbInterfaceIndex >> accountName >> password >> dbid >> success >> entityID >> flags >> deadline;

	PendingLoginMgr::PLInfos* ptinfos = pendingLoginMgr_.remove(accountName);
	if(ptinfos == NULL)
	{
		ERROR_MSG(fmt::format("Baseapp::onQueryAccountCBFromDbmgr: PendingLoginMgr not found({})\n",
			accountName.c_str()));

		s.done();
		return;
	}

	Network::Channel* pClientChannel = this->networkInterface().findChannel(ptinfos->addr);

	if (!success)
	{
		std::string error;
		s >> error;
		ERROR_MSG(fmt::format("Baseapp::onQueryAccountCBFromDbmgr: query {} is failed! error({})\n",
			accountName.c_str(), error));

		s.done();
		loginBaseappFailed(pClientChannel, accountName, SERVER_ERR_SRV_NO_READY);
		return;
	}

	std::string bindatas;
	s.readBlob(bindatas);

	Proxy* pEntity = static_cast<Proxy*>(createEntity(g_serverConfig.getDBMgr().dbAccountEntityScriptType,
		NULL, false, entityID));

	if(!pEntity)
	{
		ERROR_MSG(fmt::format("Baseapp::onQueryAccountCBFromDbmgr: create {} is failed! error(baseEntity == NULL)\n",
			accountName.c_str()));
		
		s.done();
		
		loginBaseappFailed(pClientChannel, accountName, SERVER_ERR_SRV_NO_READY);
		return;
	}

	KBE_ASSERT(pEntity != NULL);
	pEntity->hasDB(true);
	pEntity->dbid(dbInterfaceIndex, dbid);
	pEntity->setClientType(ptinfos->ctype);
	pEntity->setLoginDatas(ptinfos->datas);
	pEntity->setCreateDatas(bindatas);

	KBE_ASSERT(entityID > 0);
	EntityDef::context().currEntityID = entityID;
	EntityDef::context().currComponentType = BASEAPP_TYPE;
	PyObject* pyDict = createDictDataFromPersistentStream(s, g_serverConfig.getDBMgr().dbAccountEntityScriptType);

	PyObject* py__ACCOUNT_NAME__ = PyUnicode_FromString(accountName.c_str());
	PyDict_SetItemString(pyDict, "__ACCOUNT_NAME__", py__ACCOUNT_NAME__);
	Py_DECREF(py__ACCOUNT_NAME__);

	PyObject* py__ACCOUNT_PASSWD__ = PyUnicode_FromString(KBE_MD5::getDigest(password.data(), (int)password.length()).c_str());
	PyDict_SetItemString(pyDict, "__ACCOUNT_PASSWORD__", py__ACCOUNT_PASSWD__);
	Py_DECREF(py__ACCOUNT_PASSWD__);

	Py_INCREF(pEntity);
	pEntity->initializeEntity(pyDict);
	Py_DECREF(pyDict);

	if(pClientChannel != NULL)
	{
		// 创建entity的客户端entityCall
		EntityCall* entityClientEntityCall = new EntityCall(pEntity->pScriptModule(), 
			&pClientChannel->addr(), 0, pEntity->id(), ENTITYCALL_TYPE_CLIENT);

		pEntity->clientEntityCall(entityClientEntityCall);
		pEntity->addr(pClientChannel->addr());

		createClientProxies(pEntity);
		
		/*
		Network::Bundle* pBundle = Network::Bundle::createPoolObject();
		(*pBundle).newMessage(DbmgrInterface::onAccountOnline);

		DbmgrInterface::onAccountOnlineArgs3::staticAddToBundle((*pBundle), accountName, 
			componentID_, pEntity->id());

		pChannel->send(pBundle);
		*/
	}

	INFO_MSG(fmt::format("Baseapp::onQueryAccountCBFromDbmgr: user={}, uuid={}, entityID={}, flags={}, deadline={}.\n",
		accountName, pEntity->rndUUID(), pEntity->id(), flags, deadline));

	SAFE_RELEASE(ptinfos);
	Py_DECREF(pEntity);
}

//-------------------------------------------------------------------------------------
void Baseapp::forwardMessageToClientFromCellapp(Network::Channel* pChannel, 
												KBEngine::MemoryStream& s)
{
	AUTO_SCOPED_PROFILE("forwardMessageToClientFromCellapp");
	
	if(pChannel->isExternal())
		return;
	
	ENTITY_ID eid;
	s >> eid;

	Entity* pEntity = pEntities_->find(eid);
	if(pEntity == NULL)
	{
		if(s.length() > 0)
		{
			if(Network::g_trace_packet > 0 && s.length() >= sizeof(Network::MessageID))
			{
				Network::MessageID fmsgid = 0;
				s >> fmsgid;

				Network::MessageHandler* pMessageHandler = ClientInterface::messageHandlers.find(fmsgid);
				bool isprint = true;

				if(pMessageHandler)
				{
					std::vector<std::string>::iterator iter = std::find(Network::g_trace_packet_disables.begin(),
															Network::g_trace_packet_disables.end(),
																pMessageHandler->name);	

					if(iter != Network::g_trace_packet_disables.end())
					{
						isprint = false;
					}
				}

				if(isprint)
				{
					WARNING_MSG(fmt::format("Baseapp::forwardMessageToClientFromCellapp: entityID {} not found, {}(msgid={}).\n", 
						eid, (pMessageHandler == NULL ? "unknown" : pMessageHandler->name), fmsgid));
				}
				else
				{
					WARNING_MSG(fmt::format("Baseapp::forwardMessageToClientFromCellapp: entityID {} not found.\n", eid));
				}
			}
			else
			{
				// ERROR_MSG(fmt::format("Baseapp::forwardMessageToClientFromCellapp: entityID {} not found.\n", eid));
			}
		}

		s.done();
		return;
	}

	EntityCallAbstract* entityCall = static_cast<EntityCallAbstract*>(pEntity->clientEntityCall());
	if(entityCall == NULL)
	{
		if(s.length() > 0)
		{
			if(Network::g_trace_packet > 0 && s.length() >= sizeof(Network::MessageID))
			{
				Network::MessageID fmsgid = 0;
				s >> fmsgid;

				Network::MessageHandler* pMessageHandler = ClientInterface::messageHandlers.find(fmsgid);
				bool isprint = true;

				if(pMessageHandler)
				{
					std::vector<std::string>::iterator iter = std::find(Network::g_trace_packet_disables.begin(),
															Network::g_trace_packet_disables.end(),
																pMessageHandler->name);

					if(iter != Network::g_trace_packet_disables.end())
					{
						isprint = false;
					}
				}

				if(isprint)
				{
					ERROR_MSG(fmt::format("Baseapp::forwardMessageToClientFromCellapp: "
						"error(not found clientEntityCall)! entityID({}), {}(msgid={}).\n", 
						eid,(pMessageHandler == NULL ? "unknown" : pMessageHandler->name), fmsgid));
				}
				else
				{
					ERROR_MSG(fmt::format("Baseapp::forwardMessageToClientFromCellapp: "
						"error(not found clientEntityCall)! entityID({}).\n",
						eid));
				}
			}
			else
			{
				/*
				ERROR_MSG(fmt::format("Baseapp::forwardMessageToClientFromCellapp: "
					"error(not found clientEntityCall)! entityID({}).\n",
					eid));
				*/
			}
		}

		s.done();
		return;
	}
	
	if(s.length() <= 0)
		return;

	BaseMessagesForwardClientHandler* pBufferedSendToClientMessages = pEntity->pBufferedSendToClientMessages();

	Network::Channel* pClientChannel = entityCall->getChannel();
	Network::Bundle* pSendBundle = NULL;
	
	static Network::MessageHandler* pMessageHandler = NULL;

	int rpos = s.rpos();
	Network::MessageID fmsgid = 0;
	s >> fmsgid;

	if (!pMessageHandler || pMessageHandler->msgID != fmsgid)
		pMessageHandler = ClientInterface::messageHandlers.find(fmsgid);

	s.rpos(rpos);
		
	if (!pClientChannel || pBufferedSendToClientMessages)
		pSendBundle = Network::Bundle::createPoolObject();
	else
		pSendBundle = pClientChannel->createSendBundle();

	(*pSendBundle).append(s);
	pSendBundle->pCurrMsgHandler(pMessageHandler);

	if (!pBufferedSendToClientMessages)
		static_cast<Proxy*>(pEntity)->sendToClient(pSendBundle);
	else
		pBufferedSendToClientMessages->pushMessages(pSendBundle);

	if(Network::g_trace_packet > 0 && s.length() >= sizeof(Network::MessageID))
	{
		bool isprint = true;

		if(pMessageHandler)
		{
			std::vector<std::string>::iterator iter = std::find(Network::g_trace_packet_disables.begin(),
													Network::g_trace_packet_disables.end(),
														pMessageHandler->name);

			if(iter != Network::g_trace_packet_disables.end())
			{
				isprint = false;
			}
		}

		if(isprint)
		{
			DEBUG_MSG(fmt::format("Baseapp::forwardMessageToClientFromCellapp: {}(msgid={}).\n",
				(pMessageHandler == NULL ? "unknown" : pMessageHandler->name), fmsgid));
		}
	}

	s.done();
}

//-------------------------------------------------------------------------------------
void Baseapp::forwardMessageToCellappFromCellapp(Network::Channel* pChannel, 
												KBEngine::MemoryStream& s)
{
	if(pChannel->isExternal())
		return;
	
	ENTITY_ID eid;
	s >> eid;

	Entity* pEntity = pEntities_->find(eid);
	if(pEntity == NULL)
	{
		ERROR_MSG(fmt::format("Baseapp::forwardMessageToCellappFromCellapp: entityID {} not found.\n", eid));
		s.done();
		return;
	}

	EntityCallAbstract* entityCall = static_cast<EntityCallAbstract*>(pEntity->cellEntityCall());
	if(entityCall == NULL)
	{
		ERROR_MSG(fmt::format("Baseapp::forwardMessageToCellappFromCellapp: "
			"error(not found cellEntityCall)! entityID={}.\n", 
			eid));

		s.done();
		return;
	}
	
	if(s.length() <= 0)
		return;

	Network::Channel* pClientChannel = entityCall->getChannel();
	Network::Bundle* pSendBundle = NULL;
	
	if(!pChannel)
		pSendBundle = Network::Bundle::createPoolObject();
	else
		pSendBundle = pClientChannel->createSendBundle();
	
	(*pSendBundle).append(s);
	pEntity->sendToCellapp(pSendBundle);
	
	if(Network::g_trace_packet > 0 && s.length() >= sizeof(Network::MessageID))
	{
		Network::MessageID fmsgid = 0;
		s >> fmsgid;

		Network::MessageHandler* pMessageHandler = CellappInterface::messageHandlers.find(fmsgid);
		bool isprint = true;

		if(pMessageHandler)
		{
			std::vector<std::string>::iterator iter = std::find(Network::g_trace_packet_disables.begin(),
													Network::g_trace_packet_disables.end(),
														pMessageHandler->name);

			if(iter != Network::g_trace_packet_disables.end())
			{
				isprint = false;
			}
		}

		if(isprint)
		{
			DEBUG_MSG(fmt::format("Baseapp::forwardMessageToCellappFromCellapp: {}(msgid={}).\n",
				(pMessageHandler == NULL ? "unknown" : pMessageHandler->name), fmsgid));
		}
	}

	s.done();
}

//-------------------------------------------------------------------------------------
RemoteEntityMethod* Baseapp::createEntityCallCallEntityRemoteMethod(MethodDescription* pMethodDescription, EntityCallAbstract* pEntityCall)
{
	return new EntityRemoteMethod(pMethodDescription, pEntityCall);
}

//-------------------------------------------------------------------------------------
void Baseapp::onEntityCall(Network::Channel* pChannel, KBEngine::MemoryStream& s)
{
	if(pChannel->isExternal())
		return;

	ENTITY_ID eid;
	s >> eid;

	ENTITYCALL_TYPE calltype;
	s >> calltype;

	// 在本地区尝试查找该收件人信息， 看收件人是否属于本区域
	Entity* pEntity = pEntities_->find(eid);
	if(pEntity == NULL)
	{
		WARNING_MSG(fmt::format("Baseapp::onEntityCall: entityID {} not found.\n", eid));
		s.done();
		return;
	}
	

	switch(calltype)
	{
		// 本组件是baseapp，那么确认邮件的目的地是这里， 那么执行最终操作
		case ENTITYCALL_TYPE_BASE:		
			pEntity->onRemoteMethodCall(pChannel, s);
			break;

		// entity.cell.base.xxx
		case ENTITYCALL_TYPE_CELL_VIA_BASE: 
			{
				EntityCallAbstract* entityCall = static_cast<EntityCallAbstract*>(pEntity->cellEntityCall());
				if(entityCall == NULL)
				{
					WARNING_MSG(fmt::format("Baseapp::onEntityCall: not found cellEntityCall! "
						"entitycallType={}, entityID={}.\n", calltype, eid));

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

		case ENTITYCALL_TYPE_CLIENT_VIA_BASE: // entity.base.client
			{
				EntityCallAbstract* entityCall = static_cast<EntityCallAbstract*>(pEntity->clientEntityCall());
				if(entityCall == NULL)
				{
					WARNING_MSG(fmt::format("Baseapp::onEntityCall: not found clientEntityCall! "
						"entitycallType={}, entityID={}.\n", 
						calltype, eid));

					break;
				}
				
				Network::Channel* pChannel = entityCall->getChannel();
				if (pChannel)
				{
					Network::Bundle* pBundle = pChannel->createSendBundle();
					entityCall->newCall_(*pBundle);
					pBundle->append(s);

					if(Network::g_trace_packet > 0 && s.length() >= sizeof(ENTITY_METHOD_UID))
					{
						ENTITY_METHOD_UID utype = 0;
						s >> utype;

						DEBUG_MSG(fmt::format("Baseapp::onEntityCall: onRemoteMethodCall(entityID={}, method={}).\n",
							eid, utype));
					}

					static_cast<Proxy*>(pEntity)->sendToClient(pBundle);
				}
			}
			break;

		default:
			{
				ERROR_MSG(fmt::format("Baseapp::onEntityCall: entitycallType {} error! must a baseType. entityID={}.\n",
					calltype, eid));
			}
	};

	s.done();
}

//-------------------------------------------------------------------------------------
void Baseapp::onRemoteCallCellMethodFromClient(Network::Channel* pChannel, KBEngine::MemoryStream& s)
{
	if(pChannel->isInternal())
		return;

	ENTITY_ID srcEntityID = pChannel->proxyID();
	if(srcEntityID <= 0)
	{
		ERROR_MSG(fmt::format("Baseapp::onRemoteCallCellMethodFromClient: pChannel does not bind proxy! addr={}\n",
			pChannel->c_str()));
				
		pChannel->condemn();
		s.done();
		return;
	}
	
	if(s.length() <= 0)
		return;

	KBEngine::Proxy* e = static_cast<KBEngine::Proxy*>
			(KBEngine::Baseapp::getSingleton().findEntity(srcEntityID));		

	if(e == NULL || e->cellEntityCall() == NULL)
	{
		WARNING_MSG(fmt::format("Baseapp::onRemoteCallCellMethodFromClient: {} {} no cell.\n",
			(e == NULL ? "unknown" : e->scriptName()), srcEntityID));
		
		s.done();
		return;
	}

	Network::Bundle* pBundle = Network::Bundle::createPoolObject();
	(*pBundle).newMessage(CellappInterface::onRemoteCallMethodFromClient);
	(*pBundle) << srcEntityID;
	(*pBundle).append(s);
	
	e->sendToCellapp(pBundle);
	s.done();
}

//-------------------------------------------------------------------------------------
void Baseapp::onUpdateDataFromClient(Network::Channel* pChannel, KBEngine::MemoryStream& s)
{
	if(shuttingdown_ != SHUTDOWN_STATE_STOP)
	{
		s.done();
		return;
	}

	AUTO_SCOPED_PROFILE("onUpdateDataFromClient");

	ENTITY_ID srcEntityID = pChannel->proxyID();
	if(srcEntityID <= 0)
	{
		ERROR_MSG(fmt::format("Baseapp::onUpdateDataFromClient: pChannel does not bind proxy! addr={}\n",
			pChannel->c_str()));
				
		pChannel->condemn();
		s.done();
		return;
	}
	
	static size_t datasize = (sizeof(float) * 6 + sizeof(uint8) + sizeof(uint32));
	if(s.length() <= 0 || s.length() != datasize)
	{
		ERROR_MSG(fmt::format("Baseapp::onUpdateDataFromClient: invalid data, size({} != {}), srcEntityID={}.\n",
			datasize, s.length(), srcEntityID));

		s.done();
		return;
	}

	KBEngine::Proxy* e = static_cast<KBEngine::Proxy*>
			(KBEngine::Baseapp::getSingleton().findEntity(srcEntityID));	

	if(e == NULL || e->cellEntityCall() == NULL)
	{
		ERROR_MSG(fmt::format("Baseapp::onUpdateDataFromClient: {} {} no cell.\n",
			(e == NULL ? "unknown" : e->scriptName()), srcEntityID));
		
		s.done();
		return;
	}

	Network::Bundle* pBundle = Network::Bundle::createPoolObject();
	(*pBundle).newMessage(CellappInterface::onUpdateDataFromClient);
	(*pBundle) << srcEntityID;
	(*pBundle).append(s);
	
	e->sendToCellapp(pBundle);
	s.done();
}

//------------------------------------------------------------------------------------- 
void Baseapp::onUpdateDataFromClientForControlledEntity(Network::Channel* pChannel, KBEngine::MemoryStream& s)
{
	if(shuttingdown_ != SHUTDOWN_STATE_STOP)
	{
		s.done();
		return;
	}
	
	ENTITY_ID srcEntityID = pChannel->proxyID();
	if(srcEntityID <= 0)
	{
		s.done();
		return;
	}
	
	static size_t datasize = (sizeof(int32) + sizeof(float) * 6 + sizeof(uint8) + sizeof(uint32));
	if(s.length() <= 0 || s.length() != datasize)
	{
		ERROR_MSG(fmt::format("Baseapp::onUpdateDataFromClientForControlledEntity: invalid data, size({} != {}), srcEntityID={}.\n",
			datasize, s.length(), srcEntityID));

		s.done();
		return;
	}

	KBEngine::Proxy* e = static_cast<KBEngine::Proxy*>
			(KBEngine::Baseapp::getSingleton().findEntity(srcEntityID));	

	if(e == NULL || e->cellEntityCall() == NULL)
	{
		ERROR_MSG(fmt::format("Baseapp::onUpdateDataFromClientForControlledEntity: {} {} has no cell.\n",
			(e == NULL ? "unknown" : e->scriptName()), srcEntityID));
		
		s.done();
		return;
	}

	Network::Bundle* pBundle = Network::Bundle::ObjPool().createObject();
	(*pBundle).newMessage(CellappInterface::onUpdateDataFromClientForControlledEntity);
	(*pBundle) << srcEntityID;
	(*pBundle).append(s);
	
	e->sendToCellapp(pBundle);
	s.done();
}

//-------------------------------------------------------------------------------------
void Baseapp::onBackupEntityCellData(Network::Channel* pChannel, KBEngine::MemoryStream& s)
{
	if(pChannel->isExternal())
		return;

	ENTITY_ID entityID = 0;
	s >> entityID;

	Entity* pEntity = this->findEntity(entityID);

	if(pEntity)
	{
		INFO_MSG(fmt::format("Baseapp::onBackupEntityCellData: {}({}), {} bytes.\n",
			pEntity->scriptName(), entityID, s.length()));

		pEntity->onBackupCellData(pChannel, s);
	}
	else
	{
		ERROR_MSG(fmt::format("Baseapp::onBackupEntityCellData: not found entityID={}\n", entityID));
		s.done();
	}
}

//-------------------------------------------------------------------------------------
void Baseapp::onCellWriteToDBCompleted(Network::Channel* pChannel, KBEngine::MemoryStream& s)
{
	if(pChannel->isExternal())
		return;

	ENTITY_ID entityID = 0;
	CALLBACK_ID callbackID = 0;
	int8 shouldAutoLoad = -1;
	int dbInterfaceIndex = -1;

	s >> entityID;
	s >> callbackID;
	s >> shouldAutoLoad;
	s >> dbInterfaceIndex;

	Entity* pEntity = this->findEntity(entityID);

	if(pEntity)
	{

		INFO_MSG(fmt::format("Baseapp::onCellWriteToDBCompleted: {}({}).\n",
			pEntity->scriptName(), entityID));

		pEntity->onCellWriteToDBCompleted(callbackID, shouldAutoLoad, dbInterfaceIndex);
	}
	else
	{
		ERROR_MSG(fmt::format("Baseapp::onCellWriteToDBCompleted: not found entityID={}\n",
			entityID));
	}
}

//-------------------------------------------------------------------------------------
void Baseapp::onWriteToDBCallback(Network::Channel* pChannel, ENTITY_ID eid, 
	DBID entityDBID, uint16 dbInterfaceIndex, CALLBACK_ID callbackID, bool success)
{
	if(pChannel->isExternal())
		return;

	Entity* pEntity = pEntities_->find(eid);
	if(pEntity == NULL)
	{
		// ERROR_MSG("Baseapp::onWriteToDBCallback: can't found entity:%d.\n", eid);
		return;
	}

	pEntity->onWriteToDBCallback(eid, entityDBID, dbInterfaceIndex, callbackID, -1, success);
}

//-------------------------------------------------------------------------------------
void Baseapp::onClientActiveTick(Network::Channel* pChannel)
{
	if(!pChannel->isExternal())
		return;

	onAppActiveTick(pChannel, CLIENT_TYPE, 0);

	Network::Bundle* pBundle = Network::Bundle::createPoolObject();
	pBundle->newMessage(ClientInterface::onAppActiveTickCB);
	pChannel->send(pBundle);
}

//-------------------------------------------------------------------------------------
void Baseapp::onEntityAutoLoadCBFromDBMgr(Network::Channel* pChannel, MemoryStream& s)
{
	if(pChannel->isExternal())
		return;

	pInitProgressHandler_->onEntityAutoLoadCBFromDBMgr(pChannel, s);
}

//-------------------------------------------------------------------------------------
void Baseapp::onHello(Network::Channel* pChannel, 
						const std::string& verInfo, 
						const std::string& scriptVerInfo,
						const std::string& encryptedKey)
{
	Network::Bundle* pBundle = Network::Bundle::createPoolObject();
	
	pBundle->newMessage(ClientInterface::onHelloCB);
	(*pBundle) << KBEVersion::versionString();
	(*pBundle) << KBEVersion::scriptVersionString();
	(*pBundle) << Network::MessageHandlers::getDigestStr();
	(*pBundle) << EntityDef::md5().getDigestStr();
	(*pBundle) << g_componentType;

	// 此消息不允许加密，所以设定已加密忽略再次加密，当第一次send消息不是立即发生而是交由epoll通知时会出现这种情况（一般用于测试，正规环境不会出现）
	// web协议必须要加密，所以不能设置为true
	if (pChannel->type() != KBEngine::Network::Channel::CHANNEL_WEB)
		pBundle->pCurrPacket()->encrypted(true);

	pChannel->send(pBundle);
	
	if(Network::g_channelExternalEncryptType > 0)
	{
		if(encryptedKey.size() > 3)
		{
			// 替换为一个加密的过滤器
			pChannel->pFilter(Network::createEncryptionFilter(Network::g_channelExternalEncryptType, encryptedKey));
		}
		else
		{
			WARNING_MSG(fmt::format("Baseapp::onHello: client is not encrypted, addr={}\n"
				, pChannel->c_str()));
		}
	}
}

//-------------------------------------------------------------------------------------
void Baseapp::lookApp(Network::Channel* pChannel)
{
	if(pChannel->isExternal())
		return;

	DEBUG_MSG(fmt::format("Baseapp::lookApp: {}\n", pChannel->c_str()));

	Network::Bundle* pBundle = Network::Bundle::createPoolObject();
	
	(*pBundle) << g_componentType;
	(*pBundle) << componentID_;

	ShutdownHandler::SHUTDOWN_STATE state = shuttingdown();
	int8 istate = int8(state);
	(*pBundle) << istate;
	(*pBundle) << this->entitiesSize();
	(*pBundle) << numClients();
	(*pBundle) << numProxices();

	uint32 port = 0;
	if(pTelnetServer_)
		port = pTelnetServer_->port();

	(*pBundle) << port;

	pChannel->send(pBundle);
}

//-------------------------------------------------------------------------------------
void Baseapp::importClientMessages(Network::Channel* pChannel)
{
	static Network::Bundle bundle;

	if(bundle.empty())
	{
		std::map< Network::MessageID, Network::ExposedMessageInfo > messages;

		{
			const Network::MessageHandlers::MessageHandlerMap& msgHandlers = BaseappInterface::messageHandlers.msgHandlers();
			Network::MessageHandlers::MessageHandlerMap::const_iterator iter = msgHandlers.begin();
			for(; iter != msgHandlers.end(); ++iter)
			{
				Network::MessageHandler* pMessageHandler = iter->second;
				if(!iter->second->exposed)
					continue;

				Network::ExposedMessageInfo& info = messages[iter->first];
				info.id = iter->first;
				info.name = pMessageHandler->name;
				info.msgLen = pMessageHandler->msgLen;
				info.argsType = (int8)pMessageHandler->pArgs->type();

				KBEngine::strutil::kbe_replace(info.name, "::", "_");
				std::vector<std::string>::iterator iter1 = pMessageHandler->pArgs->strArgsTypes.begin();
				for(; iter1 !=  pMessageHandler->pArgs->strArgsTypes.end(); ++iter1)
				{
					info.argsTypes.push_back((uint8)datatype2id((*iter1)));
				}
			}
		}

		bundle.newMessage(ClientInterface::onImportClientMessages);
		uint16 size = (uint16)messages.size();
		bundle << size;

		std::map< Network::MessageID, Network::ExposedMessageInfo >::iterator iter = messages.begin();
		for(; iter != messages.end(); ++iter)
		{
			uint8 argsize = (uint8)iter->second.argsTypes.size();
			bundle << iter->second.id << iter->second.msgLen << iter->second.name << iter->second.argsType << argsize;

			std::vector<uint8>::iterator argiter = iter->second.argsTypes.begin();
			for(; argiter != iter->second.argsTypes.end(); ++argiter)
			{
				bundle << (*argiter);
			}
		}
	}

	pChannel->send(new Network::Bundle(bundle));
}

//-------------------------------------------------------------------------------------
void Baseapp::importClientEntityDef(Network::Channel* pChannel)
{
	if (!pBundleImportEntityDefDatas_)
	{
		pBundleImportEntityDefDatas_ = Network::Bundle::createPoolObject();

		ENTITY_PROPERTY_UID posuid = ENTITY_BASE_PROPERTY_UTYPE_POSITION_XYZ;
		ENTITY_PROPERTY_UID diruid = ENTITY_BASE_PROPERTY_UTYPE_DIRECTION_ROLL_PITCH_YAW;
		ENTITY_PROPERTY_UID spaceuid = ENTITY_BASE_PROPERTY_UTYPE_SPACEID;

		Network::FixedMessages::MSGInfo* msgInfo =
					Network::FixedMessages::getSingleton().isFixed("Property::position");

		if(msgInfo != NULL)
			posuid = msgInfo->msgid;

		msgInfo = Network::FixedMessages::getSingleton().isFixed("Property::direction");
		if(msgInfo != NULL)
			diruid = msgInfo->msgid;

		msgInfo = Network::FixedMessages::getSingleton().isFixed("Property::spaceID");
		if(msgInfo != NULL)
			spaceuid = msgInfo->msgid;

		pBundleImportEntityDefDatas_->newMessage(ClientInterface::onImportClientEntityDef);
		
		const DataTypes::UID_DATATYPE_MAP& dataTypes = DataTypes::uid_dataTypes();
		uint16 aliassize = (uint16)dataTypes.size();
		(*pBundleImportEntityDefDatas_) << aliassize;

		DataTypes::UID_DATATYPE_MAP::const_iterator dtiter = dataTypes.begin();
		for(; dtiter != dataTypes.end(); ++dtiter)
		{
			const DataType* datatype = dtiter->second;

			(*pBundleImportEntityDefDatas_) << datatype->id();
			(*pBundleImportEntityDefDatas_) << datatype->getName();
			(*pBundleImportEntityDefDatas_) << datatype->aliasName();

			if(strcmp(datatype->getName(), "FIXED_DICT") == 0)
			{
				FixedDictType* dictdatatype = const_cast<FixedDictType*>(static_cast<const FixedDictType*>(datatype));
				
				FixedDictType::FIXEDDICT_KEYTYPE_MAP& keys = dictdatatype->getKeyTypes();

				uint8 keysize = (uint8)keys.size();
				(*pBundleImportEntityDefDatas_) << keysize;
				(*pBundleImportEntityDefDatas_) << dictdatatype->moduleName();

				FixedDictType::FIXEDDICT_KEYTYPE_MAP::const_iterator keyiter = keys.begin();
				for(; keyiter != keys.end(); ++keyiter)
				{
					(*pBundleImportEntityDefDatas_) << keyiter->first;
					(*pBundleImportEntityDefDatas_) << keyiter->second->dataType->id();
				}
			}
			else if(strcmp(datatype->getName(), "ARRAY") == 0)
			{
				(*pBundleImportEntityDefDatas_) << const_cast<FixedArrayType*>(static_cast<const FixedArrayType*>(datatype))->getDataType()->id();
			}
		}

		const EntityDef::SCRIPT_MODULES& modules = EntityDef::getScriptModules();
		EntityDef::SCRIPT_MODULES::const_iterator iter = modules.begin();
		for(; iter != modules.end(); ++iter)
		{
			const ScriptDefModule::PROPERTYDESCRIPTION_MAP& propers = iter->get()->getClientPropertyDescriptions();
			const ScriptDefModule::METHODDESCRIPTION_MAP& methods = iter->get()->getClientMethodDescriptions();
			const ScriptDefModule::METHODDESCRIPTION_MAP& methods1 = iter->get()->getBaseExposedMethodDescriptions();
			const ScriptDefModule::METHODDESCRIPTION_MAP& methods2 = iter->get()->getCellExposedMethodDescriptions();

			if(!iter->get()->hasClient())
				continue;

			uint16 size = (uint16)propers.size() + 3 /* pos, dir, spaceID */;
			uint16 size1 = (uint16)methods.size();
			uint16 size2 = (uint16)methods1.size();
			uint16 size3 = (uint16)methods2.size();

			(*pBundleImportEntityDefDatas_) << iter->get()->getName() << iter->get()->getUType() << size << size1 << size2 << size3;
			
			int16 aliasID = ENTITY_BASE_PROPERTY_ALIASID_POSITION_XYZ;
			if (!iter->get()->usePropertyDescrAlias())
				aliasID = -1;
			(*pBundleImportEntityDefDatas_) << posuid << ((uint32)ED_FLAG_ALL_CLIENTS) << aliasID << "position" << "" << DataTypes::getDataType("VECTOR3")->id();

			aliasID = ENTITY_BASE_PROPERTY_ALIASID_DIRECTION_ROLL_PITCH_YAW;
			if (!iter->get()->usePropertyDescrAlias())
				aliasID = -1;
			(*pBundleImportEntityDefDatas_) << diruid << ((uint32)ED_FLAG_ALL_CLIENTS) << aliasID << "direction" << "" << DataTypes::getDataType("VECTOR3")->id();

			aliasID = ENTITY_BASE_PROPERTY_ALIASID_SPACEID;
			if (!iter->get()->usePropertyDescrAlias())
				aliasID = -1;
			(*pBundleImportEntityDefDatas_) << spaceuid << ((uint32)ED_FLAG_CELL_PRIVATE) << aliasID << "spaceID" << "" << DataTypes::getDataType("UINT32")->id();

			ScriptDefModule::PROPERTYDESCRIPTION_MAP::const_iterator piter = propers.begin();
			for(; piter != propers.end(); ++piter)
			{
				ENTITY_PROPERTY_UID	properUtype = piter->second->getUType();
				int16 aliasID = piter->second->aliasID();
				std::string	name = piter->second->getName();
				std::string	defaultValStr = piter->second->getDefaultValStr();
				uint32 flags = piter->second->getFlags();
				(*pBundleImportEntityDefDatas_) << properUtype << flags << aliasID << name << defaultValStr << piter->second->getDataType()->id();
			}
			
			ScriptDefModule::METHODDESCRIPTION_MAP::const_iterator miter = methods.begin();
			for(; miter != methods.end(); ++miter)
			{
				ENTITY_METHOD_UID methodUtype = miter->second->getUType();
				int16 aliasID = miter->second->aliasID();

				std::string	name = miter->second->getName();
				
				const std::vector<DataType*>& args = miter->second->getArgTypes();
				uint8 argssize = (uint8)args.size();

				(*pBundleImportEntityDefDatas_) << methodUtype << aliasID << name << argssize;
				
				std::vector<DataType*>::const_iterator argiter = args.begin();
				for(; argiter != args.end(); ++argiter)
				{
					(*pBundleImportEntityDefDatas_) << (*argiter)->id();
				}
			}

			miter = methods1.begin();
			for(; miter != methods1.end(); ++miter)
			{
				ENTITY_METHOD_UID methodUtype = miter->second->getUType();
				int16 aliasID = miter->second->aliasID();

				std::string	name = miter->second->getName();
				
				const std::vector<DataType*>& args = miter->second->getArgTypes();
				uint8 argssize = (uint8)args.size();

				(*pBundleImportEntityDefDatas_) << methodUtype << aliasID << name << argssize;
				
				std::vector<DataType*>::const_iterator argiter = args.begin();
				for(; argiter != args.end(); ++argiter)
				{
					(*pBundleImportEntityDefDatas_) << (*argiter)->id();
				}
			}

			miter = methods2.begin();
			for(; miter != methods2.end(); ++miter)
			{
				ENTITY_METHOD_UID methodUtype = miter->second->getUType();
				int16 aliasID = miter->second->aliasID();

				std::string	name = miter->second->getName();
				
				const std::vector<DataType*>& args = miter->second->getArgTypes();
				uint8 argssize = (uint8)args.size();

				(*pBundleImportEntityDefDatas_) << methodUtype << aliasID << name << argssize;
				
				std::vector<DataType*>::const_iterator argiter = args.begin();
				for(; argiter != args.end(); ++argiter)
				{
					(*pBundleImportEntityDefDatas_) << (*argiter)->id();
				}
			}
		}
	}

	pChannel->send(new Network::Bundle((*pBundleImportEntityDefDatas_)));
}

//-------------------------------------------------------------------------------------
PyObject* Baseapp::__py_reloadScript(PyObject* self, PyObject* args)
{
	bool fullReload = true;
	int argCount = (int)PyTuple_Size(args);
	if(argCount == 1)
	{
		if(PyArg_ParseTuple(args, "b", &fullReload) == -1)
		{
			PyErr_Format(PyExc_TypeError, "KBEngine::reloadScript(fullReload): args error!");
			PyErr_PrintEx(0);
			return 0;
		}
	}

	Baseapp::getSingleton().reloadScript(fullReload);
	S_Return;
}

//-------------------------------------------------------------------------------------
void Baseapp::reloadScript(bool fullReload)
{
	if (pBundleImportEntityDefDatas_)
	{
		Network::Bundle::reclaimPoolObject(pBundleImportEntityDefDatas_);
		pBundleImportEntityDefDatas_ = NULL;
	}

	EntityApp<Entity>::reloadScript(fullReload);
}

//-------------------------------------------------------------------------------------
void Baseapp::onReloadScript(bool fullReload)
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
PyObject* Baseapp::__py_isShuttingDown(PyObject* self, PyObject* args)
{
	return PyBool_FromLong(Baseapp::getSingleton().isShuttingdown() ? 1 : 0);
}

//-------------------------------------------------------------------------------------
PyObject* Baseapp::__py_address(PyObject* self, PyObject* args)
{
	PyObject* pyobj = PyTuple_New(2);
	const Network::Address& addr = Baseapp::getSingleton().networkInterface().intEndpoint().addr();
	PyTuple_SetItem(pyobj, 0,  PyLong_FromUnsignedLong(addr.ip));
	PyTuple_SetItem(pyobj, 1,  PyLong_FromUnsignedLong(addr.port));
	return pyobj;
}

//-------------------------------------------------------------------------------------
PyObject* Baseapp::__py_deleteEntityByDBID(PyObject* self, PyObject* args)
{
	uint16 currargsSize = (uint16)PyTuple_Size(args);
	if (currargsSize < 3 || currargsSize > 4)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::deleteEntityByDBID: args != (entityType, dbID, pycallback, dbInterfaceName)!");
		PyErr_PrintEx(0);
		return NULL;
	}
	
	char* entityType = NULL;
	PyObject* pycallback = NULL;
	PyObject* pyDBInterfaceName = NULL;
	DBID dbid = 0;
	std::string dbInterfaceName = "default";

	if (currargsSize == 3)
	{
		if (PyArg_ParseTuple(args, "s|K|O", &entityType, &dbid, &pycallback) == -1)
		{
			PyErr_Format(PyExc_TypeError, "KBEngine::deleteEntityByDBID: args error!");
			PyErr_PrintEx(0);
			return NULL;
		}
	}
	else if (currargsSize == 4)
	{
		if (PyArg_ParseTuple(args, "s|K|O|O", &entityType, &dbid, &pycallback, &pyDBInterfaceName) == -1)
		{
			PyErr_Format(PyExc_TypeError, "KBEngine::deleteEntityByDBID: args error!");
			PyErr_PrintEx(0);
			return NULL;
		}

		wchar_t* PyUnicode_AsWideCharStringRet0 = PyUnicode_AsWideCharString(pyDBInterfaceName, NULL);
		char* ccattr = strutil::wchar2char(PyUnicode_AsWideCharStringRet0);
		dbInterfaceName = ccattr;
		PyMem_Free(PyUnicode_AsWideCharStringRet0);
		free(ccattr);
	}

	ScriptDefModule* sm = EntityDef::findScriptModule(entityType);
	if(sm == NULL)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::deleteEntityByDBID: entityType(%s) not found!", entityType);
		PyErr_PrintEx(0);
		return NULL;
	}

	if(dbid == 0)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::deleteEntityByDBID: dbid is 0!");
		PyErr_PrintEx(0);
		return NULL;
	}
	
	if(!PyCallable_Check(pycallback))
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::deleteEntityByDBID: invalid pycallback!");
		PyErr_PrintEx(0);
		return NULL;
	}

	Components::ComponentInfos* dbmgrinfos = Components::getSingleton().getDbmgr();
	if(dbmgrinfos == NULL || dbmgrinfos->pChannel == NULL || dbmgrinfos->cid == 0)
	{
		ERROR_MSG("KBEngine::deleteEntityByDBID({}): not found dbmgr!\n");
		return NULL;
	}

	DBInterfaceInfo* pDBInterfaceInfo = g_kbeSrvConfig.dbInterface(dbInterfaceName);
	if (pDBInterfaceInfo->isPure)
	{
		ERROR_MSG(fmt::format("Baseapp::deleteEntityByDBID: dbInterface({}) is a pure database does not support Entity! "
			"kbengine[_defs].xml->dbmgr->databaseInterfaces->*->pure\n",
			dbInterfaceName));

		return NULL;
	}

	int dbInterfaceIndex = pDBInterfaceInfo->index;
	if (dbInterfaceIndex < 0)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::deleteEntityByDBID: not found dbInterface(%s)!", dbInterfaceName.c_str());
		PyErr_PrintEx(0);
		return NULL;
	}

	CALLBACK_ID callbackID = Baseapp::getSingleton().callbackMgr().save(pycallback);

	Network::Bundle* pBundle = Network::Bundle::createPoolObject();
	(*pBundle).newMessage(DbmgrInterface::deleteEntityByDBID);
	(*pBundle) << (uint16)dbInterfaceIndex;
	(*pBundle) << g_componentID;
	(*pBundle) << dbid;
	(*pBundle) << callbackID;
	(*pBundle) << sm->getUType();
	dbmgrinfos->pChannel->send(pBundle);

	S_Return;
}

//-------------------------------------------------------------------------------------
void Baseapp::deleteEntityByDBIDCB(Network::Channel* pChannel, KBEngine::MemoryStream& s)
{
	if(pChannel->isExternal())
		return;
	
	ENTITY_ID entityID = 0;
	COMPONENT_ID entityInAppID = 0;
	bool success = false;
	CALLBACK_ID callbackID;
	DBID entityDBID;
	ENTITY_SCRIPT_UID sid;

	s >> success >> entityID >> entityInAppID >> callbackID >> sid >> entityDBID;

	ScriptDefModule* sm = EntityDef::findScriptModule(sid);
	if(sm == NULL)
	{
		ERROR_MSG(fmt::format("Baseapp::deleteEntityByDBIDCB: entityUType({}) not found!\n", sid));
		return;
	}

	if(callbackID > 0)
	{
		// true or false or entityCall
		PyObjectPtr pyfunc = pyCallbackMgr_.take(callbackID);
		if(pyfunc != NULL)
		{
			PyObject* pyval = NULL;
			if(success)
			{
				pyval = Py_True;
				Py_INCREF(pyval);
			}
			else if(entityID > 0 && entityInAppID > 0)
			{
				Entity* e = static_cast<Entity*>(this->findEntity(entityID));
				if(e != NULL)
				{
					pyval = e;
					Py_INCREF(pyval);
				}
				else
				{
					pyval = static_cast<EntityCall*>(new EntityCall(sm, NULL, entityInAppID, entityID, ENTITYCALL_TYPE_BASE));
				}
			}
			else
			{
				pyval = Py_False;
				Py_INCREF(pyval);
			}

			SCOPED_PROFILE(SCRIPTCALL_PROFILE);
			PyObject* pyResult = PyObject_CallFunction(pyfunc.get(), 
												const_cast<char*>("O"), 
												pyval);

			if(pyResult != NULL)
				Py_DECREF(pyResult);
			else
				SCRIPT_ERROR_CHECK();

			Py_DECREF(pyval);
		}
		else
		{
			ERROR_MSG(fmt::format("Baseapp::deleteEntityByDBIDCB: can't found callback:{}.\n",
				callbackID));
		}
	}
}

//-------------------------------------------------------------------------------------
PyObject* Baseapp::__py_lookUpEntityByDBID(PyObject* self, PyObject* args)
{
	uint16 currargsSize = (uint16)PyTuple_Size(args);
	if (currargsSize < 3 || currargsSize > 4)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::lookUpEntityByDBID: args != (entityType, dbID, pycallback, dbInterfaceName)!");
		PyErr_PrintEx(0);
		return NULL;
	}
	
	char* entityType = NULL;
	PyObject* pycallback = NULL;
	DBID dbid = 0;
	std::string dbInterfaceName = "default";

	if (currargsSize == 3)
	{
		if (PyArg_ParseTuple(args, "s|K|O", &entityType, &dbid, &pycallback) == -1)
		{
			PyErr_Format(PyExc_TypeError, "KBEngine::lookUpEntityByDBID: args error!");
			PyErr_PrintEx(0);
			return NULL;
		}
	}
	else if (currargsSize == 4)
	{
		PyObject* pyDBInterfaceName = NULL;

		if (PyArg_ParseTuple(args, "s|K|O|O", &entityType, &dbid, &pycallback, &pyDBInterfaceName) == -1)
		{
			PyErr_Format(PyExc_TypeError, "KBEngine::lookUpEntityByDBID: args error!");
			PyErr_PrintEx(0);
			return NULL;
		}

		wchar_t* PyUnicode_AsWideCharStringRet0 = PyUnicode_AsWideCharString(pyDBInterfaceName, NULL);
		char* ccattr = strutil::wchar2char(PyUnicode_AsWideCharStringRet0);
		dbInterfaceName = ccattr;
		PyMem_Free(PyUnicode_AsWideCharStringRet0);
		free(ccattr);
	}
	else
	{
		KBE_ASSERT(false);
	}

	ScriptDefModule* sm = EntityDef::findScriptModule(entityType);
	if(sm == NULL)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::lookUpEntityByDBID: entityType(%s) not found!", entityType);
		PyErr_PrintEx(0);
		return NULL;
	}

	if(dbid == 0)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::lookUpEntityByDBID: dbid is 0!");
		PyErr_PrintEx(0);
		return NULL;
	}
	
	if(!PyCallable_Check(pycallback))
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::lookUpEntityByDBID: invalid pycallback!");
		PyErr_PrintEx(0);
		return NULL;
	}
	
	DBInterfaceInfo* pDBInterfaceInfo = g_kbeSrvConfig.dbInterface(dbInterfaceName);
	if (pDBInterfaceInfo->isPure)
	{
		ERROR_MSG(fmt::format("Baseapp::lookUpEntityByDBID: dbInterface({}) is a pure database does not support Entity! "
			"kbengine[_defs].xml->dbmgr->databaseInterfaces->*->pure\n",
			dbInterfaceName));

		return NULL;
	}

	int dbInterfaceIndex = pDBInterfaceInfo->index;
	if (dbInterfaceIndex < 0)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::lookUpEntityByDBID: not found dbInterface(%s)!", dbInterfaceName.c_str());
		PyErr_PrintEx(0);
		return NULL;
	}

	Components::ComponentInfos* dbmgrinfos = Components::getSingleton().getDbmgr();
	if(dbmgrinfos == NULL || dbmgrinfos->pChannel == NULL || dbmgrinfos->cid == 0)
	{
		ERROR_MSG("KBEngine::lookUpEntityByDBID({}): not found dbmgr!\n");
		return NULL;
	}

	CALLBACK_ID callbackID = Baseapp::getSingleton().callbackMgr().save(pycallback);

	Network::Bundle* pBundle = Network::Bundle::createPoolObject();
	(*pBundle).newMessage(DbmgrInterface::lookUpEntityByDBID);
	(*pBundle) << (uint16)dbInterfaceIndex;
	(*pBundle) << g_componentID;
	(*pBundle) << dbid;
	(*pBundle) << callbackID;
	(*pBundle) << sm->getUType();
	dbmgrinfos->pChannel->send(pBundle);

	S_Return;
}

//-------------------------------------------------------------------------------------
void Baseapp::lookUpEntityByDBIDCB(Network::Channel* pChannel, KBEngine::MemoryStream& s)
{
	if(pChannel->isExternal())
		return;
	
	ENTITY_ID entityID = 0;
	COMPONENT_ID entityInAppID = 0;
	bool success = false;
	CALLBACK_ID callbackID;
	DBID entityDBID;
	ENTITY_SCRIPT_UID sid;

	s >> success >> entityID >> entityInAppID >> callbackID >> sid >> entityDBID;

	ScriptDefModule* sm = EntityDef::findScriptModule(sid);
	if(sm == NULL)
	{
		ERROR_MSG(fmt::format("Baseapp::lookUpEntityByDBIDCB: entityUType({}) not found!\n", sid));
		return;
	}

	if(callbackID > 0)
	{
		// true or false or entityCall
		PyObjectPtr pyfunc = pyCallbackMgr_.take(callbackID);
		if(pyfunc != NULL)
		{
			PyObject* pyval = NULL;

			if(entityID > 0 && entityInAppID > 0)
			{
				Entity* e = static_cast<Entity*>(this->findEntity(entityID));
				if(e != NULL)
				{
					pyval = e;
					Py_INCREF(pyval);
				}
				else
				{
					pyval = static_cast<EntityCall*>(new EntityCall(sm, NULL, entityInAppID, entityID, ENTITYCALL_TYPE_BASE));
				}
			}
			else if(success)
			{
				pyval = Py_True;
				Py_INCREF(pyval);
			}
			else
			{
				pyval = Py_False;
				Py_INCREF(pyval);
			}

			SCOPED_PROFILE(SCRIPTCALL_PROFILE);
			PyObject* pyResult = PyObject_CallFunction(pyfunc.get(), 
												const_cast<char*>("O"), 
												pyval);

			if(pyResult != NULL)
				Py_DECREF(pyResult);
			else
				SCRIPT_ERROR_CHECK();

			Py_DECREF(pyval);
		}
		else
		{
			ERROR_MSG(fmt::format("Baseapp::lookUpEntityByDBIDCB: can't found callback:{}.\n",
				callbackID));
		}
	}
}

//-------------------------------------------------------------------------------------
void Baseapp::reqAccountBindEmail(Network::Channel* pChannel, ENTITY_ID entityID, std::string& password, std::string& email)
{
	Entity* pEntity = pEntities_->find(entityID);
	if(pEntity == NULL)
	{
		ERROR_MSG(fmt::format("Baseapp::reqAccountBindEmail: can't found entity:{}.\n", entityID));
		return;
	}
	
	PyObject* py__ACCOUNT_NAME__ = PyObject_GetAttrString(pEntity, "__ACCOUNT_NAME__");
	if(py__ACCOUNT_NAME__ == NULL)
	{
		DEBUG_MSG(fmt::format("Baseapp::reqAccountBindEmail: {}({}) not found __ACCOUNT_NAME__!\n", pEntity->scriptName(), entityID));
		PyErr_Clear();
		return;
	}

	wchar_t* wname = PyUnicode_AsWideCharString(py__ACCOUNT_NAME__, NULL);					
	char* name = strutil::wchar2char(wname);									
	PyMem_Free(wname);
	
	std::string accountName = name;
	free(name);

	Py_DECREF(py__ACCOUNT_NAME__);

	if(accountName.size() == 0)
	{
		DEBUG_MSG(fmt::format("Baseapp::reqAccountBindEmail: {}({}) __ACCOUNT_NAME__ is NULL!\n", pEntity->scriptName(), entityID));
		return;
	}

	password = KBEngine::strutil::kbe_trim(password);
	email = KBEngine::strutil::kbe_trim(email);

	if (!email_isvalid(email.c_str()))
	{
		ERROR_MSG(fmt::format("Baseapp::reqAccountBindEmail(): invalid email({})! accountName={}\n", 
			email, accountName));

		Network::Bundle* pBundle = Network::Bundle::createPoolObject();
		(*pBundle).newMessage(ClientInterface::onReqAccountBindEmailCB);
		SERVER_ERROR_CODE retcode = SERVER_ERR_NAME_MAIL;
		(*pBundle) << retcode;
		pChannel->send(pBundle);
		return;
	}
			
	INFO_MSG(fmt::format("Baseapp::reqAccountBindEmail: accountName={}, entityID={}, email={}!\n", accountName, entityID, email));

	Components::ComponentInfos* dbmgrinfos = Components::getSingleton().getDbmgr();
	if(dbmgrinfos == NULL || dbmgrinfos->pChannel == NULL || dbmgrinfos->cid == 0)
	{
		ERROR_MSG(fmt::format("Baseapp::reqAccountBindEmail: accountName({}), not found dbmgr!\n", 
			accountName));

		Network::Bundle* pBundle = Network::Bundle::createPoolObject();
		(*pBundle).newMessage(ClientInterface::onReqAccountBindEmailCB);
		SERVER_ERROR_CODE retcode = SERVER_ERR_SRV_NO_READY;
		(*pBundle) << retcode;
		pChannel->send(pBundle);
		return;
	}

	Network::Bundle* pBundle = Network::Bundle::createPoolObject();
	(*pBundle).newMessage(DbmgrInterface::accountReqBindMail);
	(*pBundle) << entityID << accountName << password << email;
	dbmgrinfos->pChannel->send(pBundle);
}

//-------------------------------------------------------------------------------------
void Baseapp::onReqAccountBindEmailCBFromDBMgr(Network::Channel* pChannel, ENTITY_ID entityID, std::string& accountName, std::string& email,
	SERVER_ERROR_CODE failedcode, std::string& code)
{
	if(pChannel->isExternal())
		return;
	
	INFO_MSG(fmt::format("Baseapp::onReqAccountBindEmailCBFromDBMgr: {}({}) failedcode={}!\n", 
		accountName, entityID, failedcode));

	if (failedcode != SERVER_SUCCESS)
	{
		Entity* pEntity = pEntities_->find(entityID);
		if (pEntity == NULL || pEntity->clientEntityCall() == NULL || pEntity->clientEntityCall()->getChannel() == NULL)
		{
			ERROR_MSG(fmt::format("Baseapp::onReqAccountBindEmailCBFromDBMgr: entity:{}, channel is NULL.\n", entityID));
			return;
		}

		Network::Bundle* pBundle = Network::Bundle::createPoolObject();
		(*pBundle).newMessage(ClientInterface::onReqAccountBindEmailCB);
		(*pBundle) << failedcode;
		pEntity->clientEntityCall()->getChannel()->send(pBundle);
	}
	else
	{
		Network::Channel* pBaseappmgrChannel = Components::getSingleton().getBaseappmgrChannel();
		if (pBaseappmgrChannel != NULL)
		{
			Network::Bundle* pBundle = Network::Bundle::createPoolObject();
			(*pBundle).newMessage(BaseappmgrInterface::reqAccountBindEmailAllocCallbackLoginapp);
			BaseappmgrInterface::reqAccountBindEmailAllocCallbackLoginappArgs6::staticAddToBundle((*pBundle), g_componentID,
				entityID, accountName, email, failedcode, code);

			pBaseappmgrChannel->send(pBundle);
		}
		else
		{
			ERROR_MSG(fmt::format("Baseapp::onReqAccountBindEmailCBFromDBMgr: entity:{}, pBaseappmgrChannel is NULL.\n", entityID));
			return;
		}
	}
}

//-------------------------------------------------------------------------------------
void Baseapp::onReqAccountBindEmailCBFromBaseappmgr(Network::Channel* pChannel, ENTITY_ID entityID, std::string& accountName, std::string& email,
	SERVER_ERROR_CODE failedcode, std::string& code, std::string& loginappCBHost, uint16 loginappCBPort)
{
	if (pChannel->isExternal())
		return;

	INFO_MSG(fmt::format("Baseapp::onReqAccountBindEmailCBFromBaseappmgr: {}({}) failedcode={}!\n",
		accountName, entityID, failedcode));

	if (failedcode == SERVER_SUCCESS)
	{
		threadPool_.addTask(new SendBindEMailTask(email, code,
			loginappCBHost,
			loginappCBPort));
	}

	Entity* pEntity = pEntities_->find(entityID);
	if (pEntity == NULL || pEntity->clientEntityCall() == NULL || pEntity->clientEntityCall()->getChannel() == NULL)
	{
		ERROR_MSG(fmt::format("Baseapp::onReqAccountBindEmailCBFromBaseappmgr: entity:{}, channel is NULL.\n", entityID));
		return;
	}

	Network::Bundle* pBundle = Network::Bundle::createPoolObject();
	(*pBundle).newMessage(ClientInterface::onReqAccountBindEmailCB);
	(*pBundle) << failedcode;
	pEntity->clientEntityCall()->getChannel()->send(pBundle);
}

//-------------------------------------------------------------------------------------
void Baseapp::reqAccountNewPassword(Network::Channel* pChannel, ENTITY_ID entityID, 
									std::string& oldpassworld, std::string& newpassword)
{
	Entity* pEntity = pEntities_->find(entityID);
	if(pEntity == NULL)
	{
		ERROR_MSG(fmt::format("Baseapp::reqAccountNewPassword: can't found entity:{}.\n", entityID));
		return;
	}
	
	PyObject* py__ACCOUNT_NAME__ = PyObject_GetAttrString(pEntity, "__ACCOUNT_NAME__");
	if(py__ACCOUNT_NAME__ == NULL)
	{
		DEBUG_MSG(fmt::format("Baseapp::reqAccountNewPassword: {}({}) not found __ACCOUNT_NAME__!\n", pEntity->scriptName(), entityID));
		PyErr_Clear();
		return;
	}

	wchar_t* wname = PyUnicode_AsWideCharString(py__ACCOUNT_NAME__, NULL);					
	char* name = strutil::wchar2char(wname);									
	PyMem_Free(wname);
	
	std::string accountName = name;
	free(name);

	Py_DECREF(py__ACCOUNT_NAME__);

	if(accountName.size() == 0)
	{
		DEBUG_MSG(fmt::format("Baseapp::reqAccountNewPassword: {}({}) __ACCOUNT_NAME__ is NULL!\n", pEntity->scriptName(), entityID));
		return;
	}

	oldpassworld = KBEngine::strutil::kbe_trim(oldpassworld);
	newpassword = KBEngine::strutil::kbe_trim(newpassword);

	INFO_MSG(fmt::format("Baseapp::reqAccountNewPassword: {}({})!\n", 
		accountName, entityID));

	Components::ComponentInfos* dbmgrinfos = Components::getSingleton().getDbmgr();
	if(dbmgrinfos == NULL || dbmgrinfos->pChannel == NULL || dbmgrinfos->cid == 0)
	{
		ERROR_MSG(fmt::format("Baseapp::reqAccountNewPassword: accountName({}), not found dbmgr!\n", 
			accountName));

		Network::Bundle* pBundle = Network::Bundle::createPoolObject();
		(*pBundle).newMessage(ClientInterface::onReqAccountNewPasswordCB);
		SERVER_ERROR_CODE retcode = SERVER_ERR_SRV_NO_READY;
		(*pBundle) << retcode;
		pChannel->send(pBundle);
		return;
	}

	Network::Bundle* pBundle = Network::Bundle::createPoolObject();
	(*pBundle).newMessage(DbmgrInterface::accountNewPassword);
	(*pBundle) << entityID << accountName << oldpassworld << newpassword;
	dbmgrinfos->pChannel->send(pBundle);
}

//-------------------------------------------------------------------------------------
void Baseapp::onReqAccountNewPasswordCB(Network::Channel* pChannel, ENTITY_ID entityID, std::string& accountName,
	SERVER_ERROR_CODE failedcode)
{
	if(pChannel->isExternal())
		return;
	
	INFO_MSG(fmt::format("Baseapp::onReqAccountNewPasswordCB: {}({}) failedcode={}!\n", 
		accountName, entityID, failedcode));

	Entity* pEntity = pEntities_->find(entityID);
	if(pEntity == NULL || pEntity->clientEntityCall() == NULL || pEntity->clientEntityCall()->getChannel() == NULL)
	{
		ERROR_MSG(fmt::format("Baseapp::onReqAccountNewPasswordCB: entity:{}, channel is NULL.\n", entityID));
		return;
	}

	Network::Bundle* pBundle = Network::Bundle::createPoolObject();
	(*pBundle).newMessage(ClientInterface::onReqAccountNewPasswordCB);
	(*pBundle) << failedcode;
	pEntity->clientEntityCall()->getChannel()->send(pBundle);
}

//-------------------------------------------------------------------------------------
void Baseapp::onVersionNotMatch(Network::Channel* pChannel)
{
	INFO_MSG(fmt::format("Baseapp::onVersionNotMatch: {}.\n", pChannel->c_str()));

	Network::Bundle* pBundle = Network::Bundle::createPoolObject();
	pBundle->newMessage(ClientInterface::onVersionNotMatch);
	(*pBundle) << KBEVersion::versionString();
	pChannel->send(pBundle);
}

//-------------------------------------------------------------------------------------
void Baseapp::onScriptVersionNotMatch(Network::Channel* pChannel)
{
	INFO_MSG(fmt::format("Baseapp::onScriptVersionNotMatch: {}.\n", pChannel->c_str()));

	Network::Bundle* pBundle = Network::Bundle::createPoolObject();
	pBundle->newMessage(ClientInterface::onScriptVersionNotMatch);
	(*pBundle) << KBEVersion::scriptVersionString();
	pChannel->send(pBundle);
}

//-------------------------------------------------------------------------------------
PyObject* Baseapp::__py_getFlags(PyObject* self, PyObject* args)
{
	return PyLong_FromUnsignedLong(Baseapp::getSingleton().flags());
}

//-------------------------------------------------------------------------------------
PyObject* Baseapp::__py_setFlags(PyObject* self, PyObject* args)
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
		PyErr_Format(PyExc_TypeError, "KBEngine::setFlags: args error!");
		PyErr_PrintEx(0);
		return NULL;
	}

	Baseapp::getSingleton().flags(flags);
	S_Return;
}

//-------------------------------------------------------------------------------------

}
