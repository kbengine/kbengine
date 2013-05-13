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
#include "proxy.hpp"
#include "base.hpp"
#include "baseapp_interface.hpp"
#include "base_remotemethod.hpp"
#include "archiver.hpp"
#include "backup_sender.hpp"
#include "restore_entity_handler.hpp"
#include "forward_message_over_handler.hpp"
#include "sync_entitystreamtemplate_handler.hpp"
#include "cstdkbe/timestamp.hpp"
#include "cstdkbe/kbeversion.hpp"
#include "network/common.hpp"
#include "network/tcp_packet.hpp"
#include "network/udp_packet.hpp"
#include "network/fixed_messages.hpp"
#include "network/encryption_filter.hpp"
#include "server/componentbridge.hpp"
#include "server/components.hpp"
#include "server/telnet_server.hpp"
#include "math/math.hpp"
#include "entitydef/blob.hpp"
#include "client_lib/client_interface.hpp"

#include "../../server/baseappmgr/baseappmgr_interface.hpp"
#include "../../server/cellappmgr/cellappmgr_interface.hpp"
#include "../../server/baseapp/baseapp_interface.hpp"
#include "../../server/cellapp/cellapp_interface.hpp"
#include "../../server/dbmgr/dbmgr_interface.hpp"
#include "../../server/loginapp/loginapp_interface.hpp"

namespace KBEngine{
	
ServerConfig g_serverConfig;
KBE_SINGLETON_INIT(Baseapp);

uint64 Baseapp::_g_lastTimestamp = timestamp();

PyObject* create_celldatadict_from_stream(MemoryStream& s, const char* entityType)
{
	PyObject* pyDict = PyDict_New();
	ScriptDefModule* scriptModule = EntityDef::findScriptModule(entityType);

	// 先将celldata中的存储属性取出
	ScriptDefModule::PROPERTYDESCRIPTION_MAP& propertyDescrs = scriptModule->getPersistentPropertyDescriptions();
	ScriptDefModule::PROPERTYDESCRIPTION_MAP::const_iterator iter = propertyDescrs.begin();

	for(; iter != propertyDescrs.end(); iter++)
	{
		PropertyDescription* propertyDescription = iter->second;

		const char* attrname = propertyDescription->getName();
		PyObject* pyVal = propertyDescription->createFromStream(&s);
		PyDict_SetItemString(pyDict, attrname, pyVal);
		Py_DECREF(pyVal);
	}
	
	if(scriptModule->hasCell())
	{
		ArraySize size = 0;
#ifdef CLIENT_NO_FLOAT
		int32 v1, v2, v3;
		int32 vv1, vv2, vv3;
#else
		float v1, v2, v3;
		float vv1, vv2, vv3;
#endif
		
		s >> size >> v1 >> v2 >> v3;
		s >> size >> vv1 >> vv2 >> vv3;

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

	return pyDict;
}

//-------------------------------------------------------------------------------------
Baseapp::Baseapp(Mercury::EventDispatcher& dispatcher, 
			 Mercury::NetworkInterface& ninterface, 
			 COMPONENT_TYPE componentType,
			 COMPONENT_ID componentID):
	EntityApp<Base>(dispatcher, ninterface, componentType, componentID),
	loopCheckTimerHandle_(),
	pGlobalBases_(NULL),
	pendingLoginMgr_(ninterface),
	forward_messagebuffer_(ninterface),
	pBackupSender_(),
	load_(0.f),
	numProxices_(0),
	pTelnetServer_(NULL),
	pRestoreEntityHandlers_()
{
	KBEngine::Mercury::MessageHandlers::pMainMessageHandlers = &BaseappInterface::messageHandlers;

	// hook mailboxcall
	static EntityMailbox::MailboxCallHookFunc mailboxCallHookFunc = std::tr1::bind(&Baseapp::createMailboxCallEntityRemoteMethod, this, 
		std::tr1::placeholders::_1, std::tr1::placeholders::_2);

	EntityMailbox::setMailboxCallHookFunc(&mailboxCallHookFunc);
}

//-------------------------------------------------------------------------------------
Baseapp::~Baseapp()
{
}

//-------------------------------------------------------------------------------------		
bool Baseapp::initializeWatcher()
{
	ProfileVal::setWarningPeriod(stampsPerSecond() / g_kbeSrvConfig.gameUpdateHertz());

	WATCH_OBJECT("numProxices", this, &Baseapp::numProxices);
	WATCH_OBJECT("load", this, &Baseapp::getLoad);
	WATCH_OBJECT("stats/runningTime", &runningTime);
	return EntityApp<Base>::initializeWatcher();
}

//-------------------------------------------------------------------------------------
bool Baseapp::installPyModules()
{
	Base::installScript(getScript().getModule());
	Proxy::installScript(getScript().getModule());
	GlobalDataClient::installScript(getScript().getModule());

	registerScript(Base::getScriptType());
	registerScript(Proxy::getScriptType());

	// 注册创建entity的方法到py
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(),		time,							__py_gametime,						METH_VARARGS,			0);
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(),		createBase,						__py_createBase,					METH_VARARGS,			0);
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(),		createBaseLocally,				__py_createBase,					METH_VARARGS,			0);
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(),		createEntity,					__py_createBase,					METH_VARARGS,			0);
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(), 		createBaseAnywhere,				__py_createBaseAnywhere,			METH_VARARGS,			0);
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(), 		createBaseFromDBID,				__py_createBaseFromDBID,			METH_VARARGS,			0);
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(), 		executeRawDatabaseCommand,		__py_executeRawDatabaseCommand,		METH_VARARGS,			0);
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(), 		quantumPassedPercent,			__py_quantumPassedPercent,			METH_VARARGS,			0);
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(), 		charge,							__py_charge,						METH_VARARGS,			0);

	return EntityApp<Base>::installPyModules();
}

//-------------------------------------------------------------------------------------
void Baseapp::onInstallPyModules()
{
	// 添加globalData, globalBases支持
	pGlobalBases_ = new GlobalDataClient(DBMGR_TYPE, GlobalDataServer::GLOBAL_BASES);
	registerPyObjectToScript("globalBases", pGlobalBases_);

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
		kbe_snprintf(buf, MAX_BUF, "baseapp%u.prof", startGroupOrder_);
		script::PyProfile::dump("kbengine", buf);
		script::PyProfile::remove("kbengine");
	}

	unregisterPyObjectToScript("globalBases");
	S_RELEASE(pGlobalBases_); 

	Base::uninstallScript();
	Proxy::uninstallScript();
	return EntityApp<Base>::uninstallPyModules();
}

//-------------------------------------------------------------------------------------
PyObject* Baseapp::__py_gametime(PyObject* self, PyObject* args)
{
	return PyLong_FromUnsignedLong(Baseapp::getSingleton().time());
}

//-------------------------------------------------------------------------------------
PyObject* Baseapp::__py_quantumPassedPercent(PyObject* self, PyObject* args)
{
	return PyLong_FromLong(quantumPassedPercent());
}

//-------------------------------------------------------------------------------------
int Baseapp::quantumPassedPercent(uint64 curr)
{
	// 取得间隔差转为微妙级别 / 每秒cpu-stamps = 一秒内所过去的cpu-stamps
	uint64 pass_stamps = (curr - _g_lastTimestamp) * uint64(1000) / stampsPerSecond();

	static int ms_expected = (1000 / g_kbeSrvConfig.gameUpdateHertz());

	// 得到一个当前tick-pass_stamps占一个时钟周期的的百分比
	return int(pass_stamps) * 100 / ms_expected;
}

//-------------------------------------------------------------------------------------
uint64 Baseapp::checkTickPeriod()
{
	uint64 curr = timestamp();
	int percent = quantumPassedPercent(curr);

	if (percent > 200)
	{
		WARNING_MSG(boost::format("Baseapp::handleGameTick: tick took %d%% (%.2f seconds)!\n") %
			percent % (float(percent)/1000.f));
	}

	uint64 elapsed = curr - _g_lastTimestamp;
	_g_lastTimestamp = curr;
	return elapsed;
}

//-------------------------------------------------------------------------------------
bool Baseapp::run()
{
	return EntityApp<Base>::run();
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

	EntityApp<Base>::handleTimeout(handle, arg);
}

//-------------------------------------------------------------------------------------
void Baseapp::handleCheckStatusTick()
{
	pendingLoginMgr_.process();
}

//-------------------------------------------------------------------------------------
void Baseapp::updateLoad()
{
	uint64 lastTickInStamps = checkTickPeriod();

	// 获得空闲时间比例
	double spareTime = 1.0;
	if (lastTickInStamps != 0)
	{
		spareTime = double(mainDispatcher_.getSpareTime())/double(lastTickInStamps);
	}

	mainDispatcher_.clearSpareTime();

	// 如果空闲时间比例小于0 或者大于1则表明计时不准确
	if ((spareTime < 0.f) || (1.f < spareTime))
	{
		if (g_timingMethod == RDTSC_TIMING_METHOD)
		{
			CRITICAL_MSG(boost::format("Baseapp::handleGameTick: "
				"Invalid timing result %.3f.\n"
				"please  to change for timingMethod(curr = RDTSC_TIMING_METHOD)!") %
				spareTime );
		}
		else
		{
			CRITICAL_MSG(boost::format("Baseapp::handleGameTick: Invalid timing result %.3f.\n") %
				spareTime);
		}
	}

	// 负载的值为1.0 - 空闲时间比例, 必须在0-1.f之间
	float load = KBEClamp(1.f - float(spareTime), 0.f, 1.f);

	// 此处算法看server_operations_guide.pdf介绍loadSmoothingBias处
	// loadSmoothingBias 决定本次负载取最后一次负载的loadSmoothingBias剩余比例 + 当前负载的loadSmoothingBias比例
	static float loadSmoothingBias = g_kbeSrvConfig.getBaseApp().loadSmoothingBias;
	load_ = (1 - loadSmoothingBias) * load_ + loadSmoothingBias * load;

	Mercury::Channel* pChannel = Componentbridge::getComponents().getBaseappmgrChannel();
	
	if(pChannel != NULL)
	{
		Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
		(*pBundle).newMessage(BaseappmgrInterface::updateBaseapp);
		BaseappmgrInterface::updateBaseappArgs4::staticAddToBundle((*pBundle), 
			componentID_, pEntities_->getEntities().size() - numProxices(), numProxices(), this->getLoad());

		(*pBundle).send(this->getNetworkInterface(), pChannel);
		Mercury::Bundle::ObjPool().reclaimObject(pBundle);
	}
}

//-------------------------------------------------------------------------------------
void Baseapp::handleGameTick()
{
	// 一定要在最前面
	updateLoad();

	EntityApp<Base>::handleGameTick();

	handleBackup();
	handleArchive();
}

//-------------------------------------------------------------------------------------
void Baseapp::handleBackup()
{
	pBackupSender_->tick();
}

//-------------------------------------------------------------------------------------
void Baseapp::handleArchive()
{
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
	loopCheckTimerHandle_ = this->getMainDispatcher().addTimer(1000000, this,
							reinterpret_cast<void *>(TIMEOUT_CHECK_STATUS));

	pBackupSender_.reset(new BackupSender());
	pArchiver_.reset(new Archiver());

	new SyncEntityStreamTemplateHandler(this->getNetworkInterface());

	_g_lastTimestamp = timestamp();

	// 如果需要pyprofile则在此处安装
	// 结束时卸载并输出结果
	if(g_kbeSrvConfig.getBaseApp().profiles.open_pyprofile)
	{
		script::PyProfile::start("kbengine");
	}

	pTelnetServer_ = new TelnetServer(&this->getMainDispatcher(), &this->getNetworkInterface());
	pTelnetServer_->pScript(&this->getScript());
	return pTelnetServer_->start(g_kbeSrvConfig.getBaseApp().telnet_passwd, 
		g_kbeSrvConfig.getBaseApp().telnet_deflayer, 
		g_kbeSrvConfig.getBaseApp().telnet_port);
}

//-------------------------------------------------------------------------------------
void Baseapp::finalise()
{
	pTelnetServer_->stop();
	SAFE_RELEASE(pTelnetServer_);

	pRestoreEntityHandlers_.clear();
	loopCheckTimerHandle_.cancel();
	EntityApp<Base>::finalise();
}

//-------------------------------------------------------------------------------------
void Baseapp::onCellAppDeath(Mercury::Channel * pChannel)
{
	PyObject* pyarg = PyTuple_New(1);

	PyObject* pyobj = PyTuple_New(2);

	const Mercury::Address& addr = pChannel->endpoint()->addr();
	PyTuple_SetItem(pyobj, 0, PyLong_FromUnsignedLong(addr.ip));
	PyTuple_SetItem(pyobj, 1, PyLong_FromUnsignedLong(addr.port));
	PyTuple_SetItem(pyarg, 0, pyobj);

	SCRIPT_ERROR_CHECK();
	PyObject* pyResult = PyObject_CallMethod(getEntryScript().get(), 
										const_cast<char*>("onCellAppDeath"), 
										const_cast<char*>("O"), 
										pyarg);

	Py_DECREF(pyarg);

	if(pyResult != NULL)
		Py_DECREF(pyResult);
	else
		SCRIPT_ERROR_CHECK();


	RestoreEntityHandler* pRestoreEntityHandler = new RestoreEntityHandler(this->getNetworkInterface());
	Entities<Base>::ENTITYS_MAP& entitiesMap = pEntities_->getEntities();
	Entities<Base>::ENTITYS_MAP::const_iterator iter = entitiesMap.begin();
	while (iter != entitiesMap.end())
	{
		Base* pBase = static_cast<Base*>(iter->second.get());
		
		EntityMailbox* cell = pBase->getCellMailbox();
		if(cell && cell->getComponentID() == pChannel->componentID())
		{
			S_RELEASE(cell);
			pBase->setCellMailbox(NULL);
			pBase->installCellDataAttr(pBase->getCellData());
			pRestoreEntityHandler->pushEntity(pBase->getID());
		}

		iter++;
	}

	pRestoreEntityHandlers_.push_back(KBEShared_ptr< RestoreEntityHandler >(pRestoreEntityHandler));
}

//-------------------------------------------------------------------------------------
void Baseapp::onResoreEntitiesOver(RestoreEntityHandler* pRestoreEntityHandler)
{
	std::vector< KBEShared_ptr< RestoreEntityHandler > >::iterator resiter = pRestoreEntityHandlers_.begin();
	for(; resiter != pRestoreEntityHandlers_.end(); resiter++)
	{
		if((*resiter).get() == pRestoreEntityHandler)
		{
			pRestoreEntityHandlers_.erase(resiter);
			return;
		}
	}
}

//-------------------------------------------------------------------------------------
void Baseapp::onChannelDeregister(Mercury::Channel * pChannel)
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

	EntityApp<Base>::onChannelDeregister(pChannel);
	
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
void Baseapp::onGetEntityAppFromDbmgr(Mercury::Channel* pChannel, int32 uid, std::string& username, 
						int8 componentType, uint64 componentID, 
						uint32 intaddr, uint16 intport, uint32 extaddr, uint16 extport)
{
	if(pChannel->isExternal())
		return;

	EntityApp<Base>::onRegisterNewApp(pChannel, uid, username, componentType, componentID, 
									intaddr, intport, extaddr, extport);

	KBEngine::COMPONENT_TYPE tcomponentType = (KBEngine::COMPONENT_TYPE)componentType;

	Components::COMPONENTS cts = Componentbridge::getComponents().getComponents(DBMGR_TYPE);
	KBE_ASSERT(cts.size() >= 1);
	
	Components::ComponentInfos* cinfos = 
		Componentbridge::getComponents().findComponent(tcomponentType, uid, componentID);

	cinfos->pChannel = NULL;

	int ret = Components::getSingleton().connectComponent(tcomponentType, uid, componentID);
	KBE_ASSERT(ret != -1);

	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();

	switch(tcomponentType)
	{
	case BASEAPP_TYPE:
		(*pBundle).newMessage(BaseappInterface::onRegisterNewApp);
		BaseappInterface::onRegisterNewAppArgs8::staticAddToBundle((*pBundle), 
			getUserUID(), getUsername(), BASEAPP_TYPE, componentID_, 

			this->getNetworkInterface().intaddr().ip, this->getNetworkInterface().intaddr().port, 
			this->getNetworkInterface().extaddr().ip, this->getNetworkInterface().extaddr().port);
		break;
	case CELLAPP_TYPE:
		(*pBundle).newMessage(CellappInterface::onRegisterNewApp);
		CellappInterface::onRegisterNewAppArgs8::staticAddToBundle((*pBundle), 
			getUserUID(), getUsername(), BASEAPP_TYPE, componentID_, 

			this->getNetworkInterface().intaddr().ip, this->getNetworkInterface().intaddr().port, 
			this->getNetworkInterface().extaddr().ip, this->getNetworkInterface().extaddr().port);
		break;
	default:
		KBE_ASSERT(false && "no support!\n");
		break;
	};
	
	(*pBundle).send(this->getNetworkInterface(), cinfos->pChannel);
	Mercury::Bundle::ObjPool().reclaimObject(pBundle);
}

//-------------------------------------------------------------------------------------
Base* Baseapp::onCreateEntityCommon(PyObject* pyEntity, ScriptDefModule* sm, ENTITY_ID eid)
{
	if(PyType_IsSubtype(sm->getScriptType(), Proxy::getScriptType()))
	{
		return new(pyEntity) Proxy(eid, sm);
	}

	return EntityApp<Base>::onCreateEntityCommon(pyEntity, sm, eid);
}

//-------------------------------------------------------------------------------------
PyObject* Baseapp::__py_createBase(PyObject* self, PyObject* args)
{
	int argCount = PyTuple_Size(args);
	PyObject* params = NULL;
	char* entityType = NULL;
	int ret = -1;

	if(argCount == 2)
		ret = PyArg_ParseTuple(args, "s|O", &entityType, &params);
	else
		ret = PyArg_ParseTuple(args, "s", &entityType);

	if(entityType == NULL || ret == -1)
	{
		PyErr_Format(PyExc_AssertionError, "Baseapp::createBase: args is error!");
		PyErr_PrintEx(0);
		return NULL;
	}
	
	PyObject* e = Baseapp::getSingleton().createEntityCommon(entityType, params);
	if(e != NULL)
		Py_INCREF(e);

	return e;
}

//-------------------------------------------------------------------------------------
PyObject* Baseapp::__py_createBaseAnywhere(PyObject* self, PyObject* args)
{
	int argCount = PyTuple_Size(args);
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
		PyErr_Format(PyExc_AssertionError, "Baseapp::createBaseAnywhere: args is error!");
		PyErr_PrintEx(0);
		return NULL;
	}

	if(!PyCallable_Check(pyCallback))
		pyCallback = NULL;

	Baseapp::getSingleton().createBaseAnywhere(entityType, params, pyCallback);
	S_Return;
}

//-------------------------------------------------------------------------------------
PyObject* Baseapp::__py_createBaseFromDBID(PyObject* self, PyObject* args)
{
	int argCount = PyTuple_Size(args);
	PyObject* pyCallback = NULL;
	wchar_t* wEntityType = NULL;
	char* entityType = NULL;
	int ret = -1;
	DBID dbid;
	PyObject* pyEntityType = NULL;

	switch(argCount)
	{
	case 3:
		ret = PyArg_ParseTuple(args, "O|K|O", &pyEntityType, &dbid, &pyCallback);
		break;
	case 2:
		ret = PyArg_ParseTuple(args, "O|K", &pyEntityType, &dbid);
		break;
	default:
		{
			PyErr_Format(PyExc_AssertionError, "%s: args require 2 or 3 args, gived %d!\n",
				__FUNCTION__, argCount);	
			PyErr_PrintEx(0);
			return NULL;
		}
	};

	if(pyEntityType)
	{
		wEntityType = PyUnicode_AsWideCharString(pyEntityType, NULL);					
		entityType = strutil::wchar2char(wEntityType);									
		PyMem_Free(wEntityType);		
	}

	if(entityType == NULL || strlen(entityType) <= 0 || ret == -1)
	{
		PyErr_Format(PyExc_AssertionError, "Baseapp::createBaseFromDBID: args is error, entityType=%s!", entityType);
		PyErr_PrintEx(0);

		if(entityType)
			free(entityType);

		return NULL;
	}

	if(EntityDef::findScriptModule(entityType) == NULL)
	{
		PyErr_Format(PyExc_AssertionError, "Baseapp::createBaseFromDBID: entityType is error!");
		PyErr_PrintEx(0);
		free(entityType);
		return NULL;
	}

	if(dbid <= 0)
	{
		PyErr_Format(PyExc_AssertionError, "Baseapp::createBaseFromDBID: dbid is error!");
		PyErr_PrintEx(0);
		free(entityType);
		return NULL;
	}

	if(pyCallback && !PyCallable_Check(pyCallback))
	{
		pyCallback = NULL;

		PyErr_Format(PyExc_AssertionError, "Baseapp::createBaseFromDBID: callback is error!");
		PyErr_PrintEx(0);
		free(entityType);
		return NULL;
	}

	Baseapp::getSingleton().createBaseFromDBID(entityType, dbid, pyCallback);

	free(entityType);
	S_Return;
}

//-------------------------------------------------------------------------------------
void Baseapp::createBaseFromDBID(const char* entityType, DBID dbid, PyObject* pyCallback)
{
	Components::COMPONENTS cts = Components::getSingleton().getComponents(DBMGR_TYPE);
	Components::ComponentInfos* dbmgrinfos = NULL;

	if(cts.size() > 0)
		dbmgrinfos = &(*cts.begin());

	if(dbmgrinfos == NULL || dbmgrinfos->pChannel == NULL || dbmgrinfos->cid == 0)
	{
		PyErr_Format(PyExc_AssertionError, "Baseapp::createBaseFromDBID: not found dbmgr!\n");
		PyErr_PrintEx(0);
		return;
	}

	CALLBACK_ID callbackID = 0;
	if(pyCallback != NULL)
	{
		callbackID = callbackMgr().save(pyCallback);
	}

	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
	pBundle->newMessage(DbmgrInterface::queryEntity);

	ENTITY_ID entityID = idClient_.alloc();
	KBE_ASSERT(entityID > 0);

	DbmgrInterface::queryEntityArgs5::staticAddToBundle((*pBundle), 
		g_componentID, dbid, entityType, callbackID, entityID);
	
	pBundle->send(this->getNetworkInterface(), dbmgrinfos->pChannel);
	Mercury::Bundle::ObjPool().reclaimObject(pBundle);
}

//-------------------------------------------------------------------------------------
void Baseapp::onCreateBaseFromDBIDCallback(Mercury::Channel* pChannel, KBEngine::MemoryStream& s)
{
	std::string entityType;
	DBID dbid;
	CALLBACK_ID callbackID;
	bool success = false;
	bool wasActive = false;
	ENTITY_ID entityID;

	s >> entityType;
	s >> dbid;
	s >> callbackID;
	s >> success;
	s >> entityID;
	s >> wasActive;

	if(!success)
	{
		ERROR_MSG(boost::format("Baseapp::onCreateBaseFromDBID: create %1%(%2%) is failed.\n") % 
			entityType.c_str() % dbid);

		if(callbackID > 0)
		{
			SCOPED_PROFILE(SCRIPTCALL_PROFILE);

			Py_INCREF(Py_None);
			// baseRef, dbid, wasActive
			PyObjectPtr pyfunc = pyCallbackMgr_.take(callbackID);
			PyObject* pyResult = PyObject_CallFunction(pyfunc.get(), 
												const_cast<char*>("OKi"), 
												Py_None, dbid, wasActive);

			if(pyResult != NULL)
				Py_DECREF(pyResult);
			else
				SCRIPT_ERROR_CHECK();
		}

		return;
	}

	PyObject* pyDict = create_celldatadict_from_stream(s, entityType.c_str());
	PyObject* e = Baseapp::getSingleton().createEntityCommon(entityType.c_str(), pyDict, false, entityID);
	if(e)
	{
		static_cast<Base*>(e)->setDBID(dbid);
		static_cast<Base*>(e)->initializeEntity(pyDict);
		Py_DECREF(pyDict);
	}

	if(callbackID > 0)
	{
		//if(e != NULL)
		//	Py_INCREF(e);

		SCOPED_PROFILE(SCRIPTCALL_PROFILE);

		// baseRef, dbid, wasActive
		PyObjectPtr pyfunc = pyCallbackMgr_.take(callbackID);
		PyObject* pyResult = PyObject_CallFunction(pyfunc.get(), 
											const_cast<char*>("OKi"), 
											e, dbid, wasActive);

		if(pyResult != NULL)
			Py_DECREF(pyResult);
		else
			SCRIPT_ERROR_CHECK();
	}
}

//-------------------------------------------------------------------------------------
void Baseapp::createInNewSpace(Base* base, PyObject* cell)
{
	ENTITY_ID id = base->getID();
	std::string entityType = base->ob_type->tp_name;

	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();

	(*pBundle).newMessage(CellappmgrInterface::reqCreateInNewSpace);

	(*pBundle) << entityType;
	(*pBundle) << id;
	(*pBundle) << componentID_;

	MemoryStream* s = MemoryStream::ObjPool().createObject();
	base->addPositionAndDirectionToStream(*s);
	(*pBundle).append(s);
	MemoryStream::ObjPool().reclaimObject(s);

	s = MemoryStream::ObjPool().createObject();
	base->addCellDataToStream(ED_FLAG_ALL, s);
	(*pBundle).append(*s);
	MemoryStream::ObjPool().reclaimObject(s);
	
	Components::COMPONENTS& components = Components::getSingleton().getComponents(CELLAPPMGR_TYPE);
	Components::COMPONENTS::iterator iter = components.begin();
	if(iter != components.end())
	{
		if((*iter).pChannel != NULL)
		{
			(*pBundle).send(this->getNetworkInterface(), (*iter).pChannel);
		}
		else
		{
			ERROR_MSG("Baseapp::createInNewSpace: cellappmgr channel is NULL.\n");
		}
		
		Mercury::Bundle::ObjPool().reclaimObject(pBundle);
		return;
	}
	
	Mercury::Bundle::ObjPool().reclaimObject(pBundle);
	ERROR_MSG("Baseapp::createInNewSpace: not found cellappmgr.\n");
}

//-------------------------------------------------------------------------------------
void Baseapp::restoreSpaceInCell(Base* base)
{
	ENTITY_ID id = base->getID();
	std::string entityType = base->ob_type->tp_name;

	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();

	(*pBundle).newMessage(CellappmgrInterface::reqRestoreSpaceInCell);

	(*pBundle) << entityType;
	(*pBundle) << id;
	(*pBundle) << componentID_;
	(*pBundle) << base->getSpaceID();

	MemoryStream* s = MemoryStream::ObjPool().createObject();
	base->addPositionAndDirectionToStream(*s);
	(*pBundle).append(s);
	MemoryStream::ObjPool().reclaimObject(s);

	s = MemoryStream::ObjPool().createObject();
	base->addCellDataToStream(ED_FLAG_ALL, s);
	(*pBundle).append(*s);
	MemoryStream::ObjPool().reclaimObject(s);
	
	Components::COMPONENTS& components = Components::getSingleton().getComponents(CELLAPPMGR_TYPE);
	Components::COMPONENTS::iterator iter = components.begin();
	if(iter != components.end())
	{
		if((*iter).pChannel != NULL)
		{
			(*pBundle).send(this->getNetworkInterface(), (*iter).pChannel);
		}
		else
		{
			ERROR_MSG("Baseapp::createInNewSpace: cellappmgr channel is NULL.\n");
		}
		
		Mercury::Bundle::ObjPool().reclaimObject(pBundle);
		return;
	}
	
	Mercury::Bundle::ObjPool().reclaimObject(pBundle);
	ERROR_MSG("Baseapp::createInNewSpace: not found cellappmgr.\n");
}

//-------------------------------------------------------------------------------------
void Baseapp::createBaseAnywhere(const char* entityType, PyObject* params, PyObject* pyCallback)
{
	std::string strInitData = "";
	uint32 initDataLength = 0;
	if(params != NULL && PyDict_Check(params))
	{
		strInitData = script::Pickler::pickle(params);
		initDataLength = strInitData.length();
	}

	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
	(*pBundle).newMessage(BaseappmgrInterface::reqCreateBaseAnywhere);

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

	Components::COMPONENTS& components = Components::getSingleton().getComponents(BASEAPPMGR_TYPE);
	Components::COMPONENTS::iterator iter = components.begin();
	if(iter != components.end())
	{
		if((*iter).pChannel != NULL)
		{
			(*pBundle).send(this->getNetworkInterface(), (*iter).pChannel);
		}
		else
		{
			ERROR_MSG("Baseapp::createInNewSpace: baseappmgr channel is NULL.\n");
		}
		
		Mercury::Bundle::ObjPool().reclaimObject(pBundle);
		return;
	}

	Mercury::Bundle::ObjPool().reclaimObject(pBundle);
	ERROR_MSG("Baseapp::createBaseAnywhere: not found baseappmgr.\n");
}

//-------------------------------------------------------------------------------------
void Baseapp::onCreateBaseAnywhere(Mercury::Channel* pChannel, MemoryStream& s)
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

	Base* base = createEntityCommon(entityType.c_str(), params);
	Py_XDECREF(params);

	if(base == NULL)
		return;

	// 如果不是在发起创建entity的baseapp上创建则需要转发回调到发起方
	if(componentID != componentID_)
	{
		Components::ComponentInfos* cinfos = Components::getSingleton().findComponent(componentID);
		if(cinfos == NULL || cinfos->pChannel == NULL)
		{
			Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
			ForwardItem* pFI = new ForwardItem();
			pFI->pHandler = NULL;
			pFI->pBundle = pBundle;

			(*pBundle).newMessage(BaseappInterface::onCreateBaseAnywhereCallback);
			(*pBundle) << callbackID;
			(*pBundle) << entityType;
			(*pBundle) << base->getID();
			(*pBundle) << componentID_;
			forward_messagebuffer_.push(componentID, pFI);
			WARNING_MSG("Baseapp::onCreateBaseAnywhere: not found baseapp, message is buffered.\n");
			return;
		}

		Mercury::Channel* lpChannel = cinfos->pChannel;

		// 需要baseappmgr转发给目的baseapp
		Mercury::Bundle* pForwardbundle = Mercury::Bundle::ObjPool().createObject();
		(*pForwardbundle).newMessage(BaseappInterface::onCreateBaseAnywhereCallback);
		(*pForwardbundle) << callbackID;
		(*pForwardbundle) << entityType;
		(*pForwardbundle) << base->getID();
		(*pForwardbundle) << componentID_;
		(*pForwardbundle).send(this->getNetworkInterface(), lpChannel);
		Mercury::Bundle::ObjPool().reclaimObject(pForwardbundle);
	}
	else
	{
		ENTITY_ID eid = base->getID();
		_onCreateBaseAnywhereCallback(NULL, callbackID, entityType, eid, componentID_);
	}
}

//-------------------------------------------------------------------------------------
void Baseapp::onCreateBaseAnywhereCallback(Mercury::Channel* pChannel, KBEngine::MemoryStream& s)
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
	_onCreateBaseAnywhereCallback(pChannel, callbackID, entityType, eid, componentID);
}

//-------------------------------------------------------------------------------------
void Baseapp::_onCreateBaseAnywhereCallback(Mercury::Channel* pChannel, CALLBACK_ID callbackID, 
	std::string& entityType, ENTITY_ID eid, COMPONENT_ID componentID)
{
	if(callbackID == 0)
		return;

	PyObjectPtr pyCallback = callbackMgr().take(callbackID);
	PyObject* pyargs = PyTuple_New(1);

	if(pChannel != NULL)
	{
		ScriptDefModule* sm = EntityDef::findScriptModule(entityType.c_str());
		if(sm == NULL)
		{
			ERROR_MSG(boost::format("Baseapp::onCreateBaseAnywhereCallback: can't found entityType:%1%.\n") %
				entityType.c_str());

			Py_DECREF(pyargs);
			return;
		}
		
		// 如果entity属于另一个baseapp创建则设置它的mailbox
		Mercury::Channel* pOtherBaseappChannel = Components::getSingleton().findComponent(componentID)->pChannel;
		KBE_ASSERT(pOtherBaseappChannel != NULL);
		PyObject* mb = static_cast<EntityMailbox*>(new EntityMailbox(sm, NULL, componentID, eid, MAILBOX_TYPE_BASE));
		PyTuple_SET_ITEM(pyargs, 0, mb);

		PyObject* pyRet = PyObject_CallObject(pyCallback.get(), pyargs);
		if(pyRet == NULL)
		{
			SCRIPT_ERROR_CHECK();
		}
		else
		{
			Py_DECREF(pyRet);
		}

		//Py_DECREF(mb);
		int i=0;
		i++;
	}
	else
	{
		Base* base = pEntities_->find(eid);
		if(base == NULL)
		{
			ERROR_MSG(boost::format("Baseapp::onCreateBaseAnywhereCallback: can't found entity:%1%.\n") % eid);
			Py_DECREF(pyargs);
			return;
		}

		Py_INCREF(base);
		PyTuple_SET_ITEM(pyargs, 0, base);

		SCOPED_PROFILE(SCRIPTCALL_PROFILE);

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

	SCRIPT_ERROR_CHECK();
	Py_DECREF(pyargs);
}

//-------------------------------------------------------------------------------------
void Baseapp::createCellEntity(EntityMailboxAbstract* createToCellMailbox, Base* base)
{
	if(base->getCellMailbox())
	{
		ERROR_MSG(boost::format("Baseapp::createCellEntity: %1% %2% has a cell!\n") %
			base->getScriptName() % base->getID());

		return;
	}

	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
	(*pBundle).newMessage(CellappInterface::onCreateCellEntityFromBaseapp);

	ENTITY_ID id = base->getID();
	std::string entityType = base->ob_type->tp_name;

	EntityMailbox* clientMailbox = base->getClientMailbox();
	bool hasClient = (clientMailbox != NULL);
	
	(*pBundle) << createToCellMailbox->getID();				// 在这个mailbox所在的cellspace上创建
	(*pBundle) << entityType;
	(*pBundle) << id;
	(*pBundle) << componentID_;
	(*pBundle) << hasClient;
	(*pBundle) << base->inRestore();

	MemoryStream* s = MemoryStream::ObjPool().createObject();
	base->addPositionAndDirectionToStream(*s);
	(*pBundle).append(s);
	MemoryStream::ObjPool().reclaimObject(s);

	s = MemoryStream::ObjPool().createObject();
	base->addCellDataToStream(ED_FLAG_ALL, s);
	(*pBundle).append(*s);
	MemoryStream::ObjPool().reclaimObject(s);
	
	if(createToCellMailbox->getChannel() == NULL)
	{
		ERROR_MSG(boost::format("Baseapp::createCellEntity: not found cellapp(createToCellMailbox:"
			"componentID=%1%, entityID=%2%), create is error!\n") %
			createToCellMailbox->getComponentID() % createToCellMailbox->getID());

		base->onCreateCellFailure();
		Mercury::Bundle::ObjPool().reclaimObject(pBundle);
		return;
	}

	(*pBundle).send(this->getNetworkInterface(), createToCellMailbox->getChannel());
	Mercury::Bundle::ObjPool().reclaimObject(pBundle);
}

//-------------------------------------------------------------------------------------
void Baseapp::onCreateCellFailure(Mercury::Channel* pChannel, ENTITY_ID entityID)
{
	if(pChannel->isExternal())
		return;

	Base* base = pEntities_->find(entityID);
	KBE_ASSERT(base != NULL);

	base->onCreateCellFailure();
}

//-------------------------------------------------------------------------------------
void Baseapp::onEntityGetCell(Mercury::Channel* pChannel, ENTITY_ID id, 
							  COMPONENT_ID componentID, SPACE_ID spaceID)
{
	if(pChannel->isExternal())
		return;

	Base* base = pEntities_->find(id);

	// DEBUG_MSG("Baseapp::onEntityGetCell: entityID %d.\n", id);
	KBE_ASSERT(base != NULL);
	
	if(base->getSpaceID() != spaceID)
		base->setSpaceID(spaceID);

	// 如果是有客户端的entity则需要告知客户端， 自身entity已经进入世界了。
	if(base->getClientMailbox() != NULL)
	{
		onClientEntityEnterWorld(static_cast<Proxy*>(base));
	}

	base->onGetCell(pChannel, componentID);
}

//-------------------------------------------------------------------------------------
void Baseapp::onClientEntityEnterWorld(Proxy* base)
{
	base->initClientCellPropertys();

	/*
	Mercury::Bundle bundle;
	bundle.newMessage(ClientInterface::onEntityEnterWorld);
	bundle << base->getID();
	bundle << base->getScriptModule()->getUType();
	bundle << base->getSpaceID();
	base->getClientMailbox()->postMail(bundle);
	*/
}

//-------------------------------------------------------------------------------------
bool Baseapp::createClientProxies(Proxy* base, bool reload)
{
	// 将通道代理的关系与该entity绑定， 在后面通信中可提供身份合法性识别
	Mercury::Channel* pChannel = base->getClientMailbox()->getChannel();
	pChannel->proxyID(base->getID());
	base->addr(pChannel->addr());

	// 重新生成一个ID
	if(reload)
		base->rndUUID(genUUID64());

	// 让客户端知道已经创建了proxices, 并初始化一部分属性
	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
	(*pBundle).newMessage(ClientInterface::onCreatedProxies);
	(*pBundle) << base->rndUUID();
	(*pBundle) << base->getID();
	(*pBundle) << base->ob_type->tp_name;
	base->getClientMailbox()->postMail((*pBundle));
	Mercury::Bundle::ObjPool().reclaimObject(pBundle);

	base->initClientBasePropertys();

	// 本应该由客户端告知已经创建好entity后调用这个接口。
	//if(!reload)
	base->onEntitiesEnabled();

	return true;
}

//-------------------------------------------------------------------------------------
PyObject* Baseapp::__py_executeRawDatabaseCommand(PyObject* self, PyObject* args)
{
	int argCount = PyTuple_Size(args);
	PyObject* pycallback = NULL;
	int ret = -1;

	char* data = NULL;
	Py_ssize_t size;
	
	if(argCount == 2)
		ret = PyArg_ParseTuple(args, "s#|O", &data, &size, &pycallback);
	else if(argCount == 1)
		ret = PyArg_ParseTuple(args, "s#", &data, &size);

	if(ret == -1)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::executeRawDatabaseCommand: args is error!");
		PyErr_PrintEx(0);
	}
	
	Baseapp::getSingleton().executeRawDatabaseCommand(data, size, pycallback);
	S_Return;
}

//-------------------------------------------------------------------------------------
void Baseapp::executeRawDatabaseCommand(const char* datas, uint32 size, PyObject* pycallback)
{
	if(datas == NULL)
	{
		ERROR_MSG("Baseapp::executeRawDatabaseCommand: execute is error!\n");
		return;
	}

	Components::COMPONENTS cts = Components::getSingleton().getComponents(DBMGR_TYPE);
	Components::ComponentInfos* dbmgrinfos = NULL;

	if(cts.size() > 0)
		dbmgrinfos = &(*cts.begin());

	if(dbmgrinfos == NULL || dbmgrinfos->pChannel == NULL || dbmgrinfos->cid == 0)
	{
		ERROR_MSG("Baseapp::executeRawDatabaseCommand: not found dbmgr!\n");
		return;
	}

	INFO_MSG(boost::format("KBEngine::executeRawDatabaseCommand:%1%.\n") % datas);

	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
	(*pBundle).newMessage(DbmgrInterface::executeRawDatabaseCommand);
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
void Baseapp::onExecuteRawDatabaseCommandCB(Mercury::Channel* pChannel, KBEngine::MemoryStream& s)
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

	DEBUG_MSG(boost::format("Baseapp::onExecuteRawDatabaseCommandCB: nrows=%1%, nfields=%2%, err=%3%.\n") % 
		nrows % nfields % err.c_str());

	if(callbackID > 0)
	{
		SCOPED_PROFILE(SCRIPTCALL_PROFILE);

		PyObjectPtr pyfunc = pyCallbackMgr_.take(callbackID);
		PyObject* pyResult = PyObject_CallFunction(pyfunc.get(), 
											const_cast<char*>("OOO"), 
											pResultSet, pAffectedRows, pErrorMsg);

		if(pyResult != NULL)
			Py_DECREF(pyResult);
		else
			SCRIPT_ERROR_CHECK();
	}

	Py_XDECREF(pResultSet);
	Py_XDECREF(pAffectedRows);
	Py_XDECREF(pErrorMsg);
}

//-------------------------------------------------------------------------------------
PyObject* Baseapp::__py_charge(PyObject* self, PyObject* args)
{
	if(PyTuple_Size(args) != 4)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::charge: args != (ordersID, dbid, byteDatas[bytes|BLOB], pycallback)!");
		PyErr_PrintEx(0);
		return NULL;
	}

	PyObject* pyDatas = NULL, *pycallback = NULL;
	char* pChargeID;
	DBID dbid;

	if(PyArg_ParseTuple(args, "s|K|O|O", &pChargeID, &dbid, &pyDatas, &pycallback) == -1)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::charge: args is error!");
		PyErr_PrintEx(0);
		return NULL;
	}
	
	if(pChargeID == NULL || strlen(pChargeID) <= 0)
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

	if(!PyBytes_Check(pyDatas) && !PyObject_TypeCheck(pyDatas, Blob::getScriptType()))
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::charge: byteDatas != bytes|BLOB!");
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

	if(!PyBytes_Check(pyDatas))
	{
		Blob* pBlob = static_cast<Blob*>(pyDatas);
		pBlob->stream().readBlob(datas);
	}
	else
	{
		char *buffer;
		Py_ssize_t length;

		if(PyBytes_AsStringAndSize(pyDatas, &buffer, &length) < 0)
		{
			SCRIPT_ERROR_CHECK();
			return NULL;
		}

		datas.assign(buffer, length);
	}

	Baseapp::getSingleton().charge(pChargeID, dbid, datas, pycallback);
	S_Return;
}

//-------------------------------------------------------------------------------------
void Baseapp::charge(std::string chargeID, DBID dbid, const std::string& datas, PyObject* pycallback)
{
	CALLBACK_ID callbackID = callbackMgr().save(pycallback);

	INFO_MSG(boost::format("Baseapp::charge: chargeID=%1%, dbid=%4%, datas=%2%, pycallback=%3%.\n") % 
		chargeID %
		datas %
		callbackID %
		dbid);

	Mercury::Bundle::SmartPoolObjectPtr pBundle = Mercury::Bundle::createSmartPoolObj();

	(*(*pBundle)).newMessage(DbmgrInterface::charge);
	(*(*pBundle)) << chargeID;
	(*(*pBundle)) << dbid;
	(*(*pBundle)).appendBlob(datas);
	(*(*pBundle)) << callbackID;

	Mercury::Channel* pChannel = Components::getSingleton().getDbmgrChannel();

	if(pChannel == NULL)
	{
		ERROR_MSG("Baseapp::charge: not found dbmgr!\n");
		return;
	}

	(*(*pBundle)).send(this->getNetworkInterface(), pChannel);
}

//-------------------------------------------------------------------------------------
void Baseapp::onChargeCB(Mercury::Channel* pChannel, KBEngine::MemoryStream& s)
{
	std::string chargeID;
	CALLBACK_ID callbackID;
	std::string datas;
	DBID dbid;
	bool success;

	s >> chargeID;
	s >> dbid;
	s.readBlob(datas);
	s >> callbackID;
	s >> success;

	INFO_MSG(boost::format("Baseapp::onChargeCB: chargeID=%1%, dbid=%4%, datas=%2%, pycallback=%3%.\n") % 
		chargeID %
		datas %
		callbackID %
		dbid);

	PyObject* pyOrder = PyUnicode_FromString(chargeID.c_str());
	PyObject* pydbid = PyLong_FromUnsignedLongLong(dbid);
	PyObject* pySuccess = PyBool_FromLong(success);
	Blob* pBlob = new Blob(datas);

	SCOPED_PROFILE(SCRIPTCALL_PROFILE);

	if(callbackID > 0)
	{
		PyObjectPtr pycallback = callbackMgr().take(callbackID);

		PyObject* pyResult = PyObject_CallFunction(pycallback.get(), 
											const_cast<char*>("OOOO"), 
											pyOrder, pydbid, pySuccess, static_cast<PyObject*>(pBlob));

		if(pyResult != NULL)
			Py_DECREF(pyResult);
		else
			SCRIPT_ERROR_CHECK();
	}
	else
	{
		PyObject* pyResult = PyObject_CallMethod(getEntryScript().get(), 
										const_cast<char*>("onLoseChargeCB"), 
										const_cast<char*>("OOOO"), 
										pyOrder, pydbid, pySuccess, static_cast<PyObject*>(pBlob));

		if(pyResult != NULL)
			Py_DECREF(pyResult);
		else
			SCRIPT_ERROR_CHECK();
	}

	Py_DECREF(pyOrder);
	Py_DECREF(pydbid);
	Py_DECREF(pySuccess);
	Py_DECREF(pBlob);
}

//-------------------------------------------------------------------------------------
void Baseapp::onDbmgrInitCompleted(Mercury::Channel* pChannel, 
		GAME_TIME gametime, ENTITY_ID startID, ENTITY_ID endID, int32 startGlobalOrder, int32 startGroupOrder, const std::string& digest)
{
	if(pChannel->isExternal())
		return;

	EntityApp<Base>::onDbmgrInitCompleted(pChannel, gametime, startID, endID, 
		startGlobalOrder, startGroupOrder, digest);

	SCOPED_PROFILE(SCRIPTCALL_PROFILE);

	PyObject* pyResult = PyObject_CallMethod(getEntryScript().get(), 
										const_cast<char*>("onInit"), 
										const_cast<char*>("i"), 
										0);

	if(pyResult != NULL)
		Py_DECREF(pyResult);
	else
		SCRIPT_ERROR_CHECK();

	// 所有脚本都加载完毕
	pyResult = PyObject_CallMethod(getEntryScript().get(), 
										const_cast<char*>("onBaseAppReady"), 
										const_cast<char*>("i"), 
										startGroupOrder);

	if(pyResult != NULL)
		Py_DECREF(pyResult);
	else
		SCRIPT_ERROR_CHECK();
}

//-------------------------------------------------------------------------------------
void Baseapp::onBroadcastGlobalBasesChange(Mercury::Channel* pChannel, KBEngine::MemoryStream& s)
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
		ERROR_MSG("Baseapp::onBroadcastCellAppDataChange: no has key!\n");
		return;
	}

	Py_INCREF(pyKey);

	if(isDelete)
	{
		if(pGlobalBases_->del(pyKey))
		{
			SCOPED_PROFILE(SCRIPTCALL_PROFILE);

			// 通知脚本
			SCRIPT_OBJECT_CALL_ARGS1(getEntryScript().get(), const_cast<char*>("onGlobalBasesDel"), 
				const_cast<char*>("O"), pyKey);
		}
	}
	else
	{
		PyObject * pyValue = script::Pickler::unpickle(value);
		if(pyValue == NULL)
		{
			ERROR_MSG("Baseapp::onBroadcastCellAppDataChange: no has value!\n");
			Py_DECREF(pyKey);
			return;
		}

		Py_INCREF(pyValue);

		if(pGlobalBases_->write(pyKey, pyValue))
		{
			SCOPED_PROFILE(SCRIPTCALL_PROFILE);

			// 通知脚本
			SCRIPT_OBJECT_CALL_ARGS2(getEntryScript().get(), const_cast<char*>("onGlobalBases"), 
				const_cast<char*>("OO"), pyKey, pyValue);
		}

		Py_DECREF(pyValue);
	}

	Py_DECREF(pyKey);
}

//-------------------------------------------------------------------------------------
void Baseapp::registerPendingLogin(Mercury::Channel* pChannel, std::string& loginName, std::string& accountName, 
								   std::string& password, ENTITY_ID entityID, DBID entityDBID)
{
	if(pChannel->isExternal())
		return;

	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
	(*pBundle).newMessage(BaseappmgrInterface::onPendingAccountGetBaseappAddr);

	(*pBundle) << loginName;
	(*pBundle) << accountName;
	(*pBundle) << this->getNetworkInterface().extaddr().ip;
	(*pBundle) << this->getNetworkInterface().extaddr().port;
	(*pBundle).send(this->getNetworkInterface(), pChannel);

	PendingLoginMgr::PLInfos* ptinfos = new PendingLoginMgr::PLInfos;
	ptinfos->accountName = accountName;
	ptinfos->password = password;
	ptinfos->entityID = entityID;
	ptinfos->entityDBID = entityDBID;
	pendingLoginMgr_.add(ptinfos);

	Mercury::Bundle::ObjPool().reclaimObject(pBundle);
}

//-------------------------------------------------------------------------------------
void Baseapp::loginGatewayFailed(Mercury::Channel* pChannel, std::string& accountName, 
								 SERVER_ERROR_CODE failedcode)
{
	if(failedcode == SERVER_ERR_NAME)
	{
		DEBUG_MSG(boost::format("Baseapp::login: not found user[%1%], login is failed!\n") %
			accountName.c_str());

		failedcode = SERVER_ERR_NAME_PASSWORD;
	}
	else if(failedcode == SERVER_ERR_PASSWORD)
	{
		DEBUG_MSG(boost::format("Baseapp::login: user[%1%] password is error, login is failed!\n") %
			accountName.c_str());

		failedcode = SERVER_ERR_NAME_PASSWORD;
	}

	if(pChannel == NULL)
		return;

	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
	(*pBundle).newMessage(ClientInterface::onLoginGatewayFailed);
	ClientInterface::onLoginGatewayFailedArgs1::staticAddToBundle((*pBundle), failedcode);
	(*pBundle).send(this->getNetworkInterface(), pChannel);
	Mercury::Bundle::ObjPool().reclaimObject(pBundle);
}

//-------------------------------------------------------------------------------------
void Baseapp::loginGateway(Mercury::Channel* pChannel, 
						   std::string& accountName, 
						   std::string& password)
{
	if(accountName.size() > ACCOUNT_NAME_MAX_LENGTH)
	{
		ERROR_MSG(boost::format("Baseapp::loginGateway: accountName too big, size=%1%, limit=%2%.\n") %
			accountName.size() % ACCOUNT_NAME_MAX_LENGTH);

		return;
	}

	if(password.size() > ACCOUNT_PASSWD_MAX_LENGTH)
	{
		ERROR_MSG(boost::format("Baseapp::loginGateway: password too big, size=%1%, limit=%2%.\n") %
			password.size() % ACCOUNT_PASSWD_MAX_LENGTH);

		return;
	}

	INFO_MSG(boost::format("Baseapp::loginGateway: new user[%1%], channel[%2%].\n") % 
		accountName.c_str() % pChannel->c_str());

	Components::COMPONENTS cts = Components::getSingleton().getComponents(DBMGR_TYPE);
	Components::ComponentInfos* dbmgrinfos = NULL;

	if(cts.size() > 0)
		dbmgrinfos = &(*cts.begin());

	if(dbmgrinfos == NULL || dbmgrinfos->pChannel == NULL || dbmgrinfos->cid == 0)
	{
		loginGatewayFailed(pChannel, accountName, SERVER_ERR_SRV_NO_READY);
		return;
	}

	PendingLoginMgr::PLInfos* ptinfos = pendingLoginMgr_.find(accountName);
	if(ptinfos == NULL)
	{
		loginGatewayFailed(pChannel, accountName, SERVER_ERR_NAME);
		return;
	}

	if(ptinfos->password != password)
	{
		loginGatewayFailed(pChannel, accountName, SERVER_ERR_PASSWORD);
		return;
	}

	// 如果entityID大于0则说明此entity是存活状态登录
	if(ptinfos->entityID > 0)
	{
		INFO_MSG(boost::format("Baseapp::loginGateway: user[%1%] has entity(%2%).\n") %
			accountName.c_str() % ptinfos->entityID);

		Proxy* base = static_cast<Proxy*>(findEntity(ptinfos->entityID));
		if(base == NULL)
		{
			loginGatewayFailed(pChannel, accountName, SERVER_ERR_SRV_NO_READY);
			return;
		}
		
		// 通知脚本异常登录请求有脚本决定是否允许这个通道强制登录
		int32 ret = base->onLogOnAttempt(inet_ntoa((struct in_addr&)pChannel->addr().ip), 
			ntohs(pChannel->addr().port), password.c_str());

		switch(ret)
		{
		case LOG_ON_ACCEPT:
			if(base->getClientMailbox() != NULL)
			{
				// 通告在别处登录
				Mercury::Channel* pOldClientChannel = base->getClientMailbox()->getChannel();
				if(pOldClientChannel != NULL)
				{
					INFO_MSG(boost::format("Baseapp::loginGateway: script LOG_ON_ACCEPT. oldClientChannel=%1%\n") %
						pOldClientChannel->c_str());
					
					loginGatewayFailed(pOldClientChannel, accountName, SERVER_ERR_ANOTHER_LOGON);
					pOldClientChannel->proxyID(0);
					pOldClientChannel->condemn();
				}
				else
				{
					INFO_MSG("Baseapp::loginGateway: script LOG_ON_ACCEPT.\n");
				}
				
				base->getClientMailbox()->addr(pChannel->addr());
				base->addr(pChannel->addr());
				createClientProxies(base, true);
			}
			else
			{
				// 创建entity的客户端mailbox
				EntityMailbox* entityClientMailbox = new EntityMailbox(base->getScriptModule(), 
					&pChannel->addr(), 0, base->getID(), MAILBOX_TYPE_CLIENT);

				base->setClientMailbox(entityClientMailbox);
				base->addr(pChannel->addr());

				// 将通道代理的关系与该entity绑定， 在后面通信中可提供身份合法性识别
				entityClientMailbox->getChannel()->proxyID(base->getID());
				createClientProxies(base, true);
			}
			break;
		case LOG_ON_WAIT_FOR_DESTROY:
		default:
			INFO_MSG("Baseapp::loginGateway: script LOG_ON_REJECT.\n");
			loginGatewayFailed(pChannel, accountName, SERVER_ERR_ACCOUNT_ONLINE);
			return;
		};
	}
	else
	{
		Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
		(*pBundle).newMessage(DbmgrInterface::queryAccount);

		ENTITY_ID entityID = idClient_.alloc();
		KBE_ASSERT(entityID > 0);

		DbmgrInterface::queryAccountArgs5::staticAddToBundle((*pBundle), accountName, password, g_componentID, 
			entityID, ptinfos->entityDBID);

		(*pBundle).send(this->getNetworkInterface(), dbmgrinfos->pChannel);
		Mercury::Bundle::ObjPool().reclaimObject(pBundle);
	}

	// 记录客户端地址
	ptinfos->addr = pChannel->addr();
}

//-------------------------------------------------------------------------------------
void Baseapp::reLoginGateway(Mercury::Channel* pChannel, std::string& accountName, 
							 std::string& password, uint64 key, ENTITY_ID entityID)
{
	INFO_MSG(boost::format("Baseapp::reLoginGateway: accountName=%1%, key=%2%, entityID=%3%.\n") %
		accountName % key % entityID);

	Base* base = findEntity(entityID);
	if(base == NULL || !PyObject_TypeCheck(base, Proxy::getScriptType()))
	{
		loginGatewayFailed(pChannel, accountName, SERVER_ERR_ILLEGAL_LOGIN);
		return;
	}
	
	Proxy* proxy = static_cast<Proxy*>(base);
	
	if(proxy->rndUUID() != key)
	{
		loginGatewayFailed(pChannel, accountName, SERVER_ERR_ILLEGAL_LOGIN);
		return;
	}

	if(proxy->getClientMailbox() == NULL)
	{
		// 创建entity的客户端mailbox
		EntityMailbox* entityClientMailbox = new EntityMailbox(proxy->getScriptModule(), 
			&pChannel->addr(), 0, proxy->getID(), MAILBOX_TYPE_CLIENT);

		proxy->setClientMailbox(entityClientMailbox);
		proxy->addr(pChannel->addr());

		// 将通道代理的关系与该entity绑定， 在后面通信中可提供身份合法性识别
		entityClientMailbox->getChannel()->proxyID(proxy->getID());
		createClientProxies(proxy, true);
	}
	else
	{
		WARNING_MSG(boost::format("Baseapp::reLoginGateway: accountName=%1%, key=%2%, "
			"entityID=%3%, ClientMailbox is exist.\n") %
			accountName.c_str() % key % entityID);
	}
}

//-------------------------------------------------------------------------------------
void Baseapp::onQueryAccountCBFromDbmgr(Mercury::Channel* pChannel, KBEngine::MemoryStream& s)
{
	if(pChannel->isExternal())
		return;

	std::string accountName;
	std::string password;
	bool success = false;
	DBID dbid;
	ENTITY_ID entityID;

	s >> accountName >> password >> dbid >> success >> entityID;

	PendingLoginMgr::PLInfos* ptinfos = pendingLoginMgr_.remove(accountName);
	if(ptinfos == NULL)
		return;

	if(!success)
	{
		ERROR_MSG(boost::format("Baseapp::onQueryAccountCBFromDbmgr: query %1% is failed!\n") %
			accountName.c_str());

		return;
	}

	Proxy* base = static_cast<Proxy*>(createEntityCommon(g_serverConfig.getDBMgr().dbAccountEntityScriptType, 
		NULL, false, entityID));

	Mercury::Channel* pClientChannel = this->getNetworkInterface().findChannel(ptinfos->addr);

	KBE_ASSERT(base != NULL);
	base->hasDB(true);
	base->setDBID(dbid);

	PyObject* pyDict = create_celldatadict_from_stream(s, g_serverConfig.getDBMgr().dbAccountEntityScriptType);

	PyObject* py__ACCOUNT_NAME__ = PyUnicode_FromString(accountName.c_str());
	PyDict_SetItemString(pyDict, "__account_name__", py__ACCOUNT_NAME__);
	Py_DECREF(py__ACCOUNT_NAME__);

	base->initializeEntity(pyDict);
	Py_DECREF(pyDict);

	if(pClientChannel != NULL)
	{
		// 创建entity的客户端mailbox
		EntityMailbox* entityClientMailbox = new EntityMailbox(base->getScriptModule(), 
			&pClientChannel->addr(), 0, base->getID(), MAILBOX_TYPE_CLIENT);

		base->setClientMailbox(entityClientMailbox);
		base->addr(pClientChannel->addr());

		createClientProxies(base);

		Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
		(*pBundle).newMessage(DbmgrInterface::onAccountOnline);

		DbmgrInterface::onAccountOnlineArgs3::staticAddToBundle((*pBundle), accountName, 
			componentID_, base->getID());

		(*pBundle).send(this->getNetworkInterface(), pChannel);
		Mercury::Bundle::ObjPool().reclaimObject(pBundle);
	}

	INFO_MSG(boost::format("Baseapp::onQueryAccountCBFromDbmgr: user[%1%], uuid[%2%], entityID=%3%.\n") %
		accountName % base->rndUUID() % base->getID());

	SAFE_RELEASE(ptinfos);
}

//-------------------------------------------------------------------------------------
void Baseapp::onEntityEnterWorldFromCellapp(Mercury::Channel* pChannel, ENTITY_ID entityID)
{
	if(pChannel->isExternal())
		return;

	Proxy* base = static_cast<Proxy*>(pEntities_->find(entityID));
	// DEBUG_MSG("Baseapp::onEntityEnterWorldFromCellapp: entityID %d.\n", entityID);
	KBE_ASSERT(base != NULL);

	Mercury::Channel* pClientChannel = this->getNetworkInterface().findChannel(base->addr());
	if(pClientChannel)
	{
		Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
		(*pBundle).newMessage(ClientInterface::onEntityEnterWorld);

		ClientInterface::onEntityEnterWorldArgs3::staticAddToBundle((*pBundle), entityID, base->getScriptModule()->getUType(),
			base->getSpaceID());

		(*pBundle).send(this->getNetworkInterface(), pClientChannel);
		Mercury::Bundle::ObjPool().reclaimObject(pBundle);
	}
}

//-------------------------------------------------------------------------------------
void Baseapp::onEntityLeaveWorldFromCellapp(Mercury::Channel* pChannel, 
											ENTITY_ID entityID)
{
	if(pChannel->isExternal())
		return;

	Proxy* base = static_cast<Proxy*>(pEntities_->find(entityID));
	// DEBUG_MSG("Baseapp::onEntityEnterWorldFromCellapp: entityID %d.\n", entityID);
	KBE_ASSERT(base != NULL);

	Mercury::Channel* pClientChannel = this->getNetworkInterface().findChannel(base->addr());
	if(pClientChannel)
	{
		Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
		(*pBundle).newMessage(ClientInterface::onEntityLeaveWorld);

		ClientInterface::onEntityLeaveWorldArgs2::staticAddToBundle((*pBundle), entityID, 
			base->getSpaceID());

		(*pBundle).send(this->getNetworkInterface(), pClientChannel);
		Mercury::Bundle::ObjPool().reclaimObject(pBundle);
	}
}

//-------------------------------------------------------------------------------------
void Baseapp::onEntityEnterSpaceFromCellapp(Mercury::Channel* pChannel, 
											ENTITY_ID entityID, SPACE_ID spaceID)
{
	if(pChannel->isExternal())
		return;
}

//-------------------------------------------------------------------------------------
void Baseapp::onEntityLeaveSpaceFromCellapp(Mercury::Channel* pChannel, 
											ENTITY_ID entityID, SPACE_ID spaceID)
{
	if(pChannel->isExternal())
		return;
}

//-------------------------------------------------------------------------------------
void Baseapp::forwardMessageToClientFromCellapp(Mercury::Channel* pChannel, 
												KBEngine::MemoryStream& s)
{
	if(pChannel->isExternal())
		return;
	
	ENTITY_ID eid;
	s >> eid;

	Base* base = pEntities_->find(eid);
	if(base == NULL)
	{
		ERROR_MSG(boost::format("Baseapp::forwardMessageToClientFromCellapp: entityID %1% not found.\n") % eid);
		s.opfini();
		return;
	}

	EntityMailboxAbstract* mailbox = static_cast<EntityMailboxAbstract*>(base->getClientMailbox());
	if(mailbox == NULL)
	{
		ERROR_MSG(boost::format("Baseapp::forwardMessageToClientFromCellapp: "
			"occur a error(can't found clientMailbox)! entityID=%1%.\n") % 
			eid);

		s.opfini();
		return;
	}
	
	if(s.opsize() <= 0)
		return;

	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
	(*pBundle).append(s);
	mailbox->postMail((*pBundle));
	Mercury::Bundle::ObjPool().reclaimObject(pBundle);
	
	if(Mercury::g_trace_packet > 0 && s.opsize() >= sizeof(Mercury::MessageID))
	{
		Mercury::MessageID fmsgid = 0;
		s >> fmsgid;
		Mercury::MessageHandler* pMessageHandler = ClientInterface::messageHandlers.find(fmsgid);
		DEBUG_MSG(boost::format("Baseapp::forwardMessageToClientFromCellapp: %1%(msgid=%2%).\n") %
			(pMessageHandler == NULL ? "unknown" : pMessageHandler->name) % fmsgid);
	}

	s.read_skip(s.opsize());
}

//-------------------------------------------------------------------------------------
RemoteEntityMethod* Baseapp::createMailboxCallEntityRemoteMethod(MethodDescription* md, EntityMailbox* pMailbox)
{
	return new BaseRemoteMethod(md, pMailbox);
}

//-------------------------------------------------------------------------------------
void Baseapp::onEntityMail(Mercury::Channel* pChannel, KBEngine::MemoryStream& s)
{
	if(pChannel->isExternal())
		return;

	ENTITY_ID eid;
	s >> eid;

	ENTITY_MAILBOX_TYPE	mailtype;
	s >> mailtype;

	// 在本地区尝试查找该收件人信息， 看收件人是否属于本区域
	Base* base = pEntities_->find(eid);
	if(base == NULL)
	{
		ERROR_MSG(boost::format("Baseapp::onEntityMail: entityID %1% not found.\n") % eid);
		s.opfini();
		return;
	}
	
	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
	Mercury::Bundle& bundle = *pBundle;

	switch(mailtype)
	{
		case MAILBOX_TYPE_BASE:		// 本组件是baseapp，那么确认邮件的目的地是这里， 那么执行最终操作
			base->onRemoteMethodCall(pChannel, s);
			break;
		case MAILBOX_TYPE_CELL_VIA_BASE: // entity.cell.base.xxx
			{
				EntityMailboxAbstract* mailbox = static_cast<EntityMailboxAbstract*>(base->getCellMailbox());
				if(mailbox == NULL)
				{
					ERROR_MSG(boost::format("Baseapp::onEntityMail: occur a error(can't found cellMailbox)! "
						"mailboxType=%1%, entityID=%2%.\n") % mailtype %  eid);

					break;
				}
				
				mailbox->newMail(bundle);
				bundle.append(s);
				mailbox->postMail(bundle);
			}
			break;
		case MAILBOX_TYPE_CLIENT_VIA_BASE: // entity.base.client
			{
				EntityMailboxAbstract* mailbox = static_cast<EntityMailboxAbstract*>(base->getClientMailbox());
				if(mailbox == NULL)
				{
					ERROR_MSG(boost::format("Baseapp::onEntityMail: occur a error(can't found clientMailbox)! "
						"mailboxType=%1%, entityID=%2%.\n") % 
						mailtype % eid);

					break;
				}
				
				mailbox->newMail(bundle);
				bundle.append(s);

				if(Mercury::g_trace_packet > 0 && s.opsize() >= sizeof(ENTITY_METHOD_UID))
				{
					ENTITY_METHOD_UID utype = 0;
					s >> utype;
					DEBUG_MSG(boost::format("Baseapp::onEntityMail: onRemoteMethodCall(entityID=%1%, method=%2%).\n") %
						eid % utype);
				}

				s.read_skip(s.opsize());
				mailbox->postMail(bundle);
			}
			break;
		default:
			{
				ERROR_MSG(boost::format("Baseapp::onEntityMail: mailboxType %1% is error! must a baseType. entityID=%2%.\n") %
					mailtype % eid);
			}
	};

	Mercury::Bundle::ObjPool().reclaimObject(pBundle);
	s.opfini();
}

//-------------------------------------------------------------------------------------
void Baseapp::onRemoteCallCellMethodFromClient(Mercury::Channel* pChannel, KBEngine::MemoryStream& s)
{
	if(pChannel->isInternal())
		return;

	ENTITY_ID srcEntityID = pChannel->proxyID();
	if(srcEntityID <= 0)
		return;
	
	if(s.opsize() <= 0)
		return;

	KBEngine::Proxy* e = static_cast<KBEngine::Proxy*>
			(KBEngine::Baseapp::getSingleton().findEntity(srcEntityID));		

	if(e == NULL || e->getCellMailbox() == NULL)
	{
		ERROR_MSG(boost::format("Baseapp::onRemoteCallCellMethodFromClient: %1% %2% has no cell.\n") %
			e->getScriptName() % srcEntityID);
		
		s.read_skip(s.opsize());
		return;
	}

	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
	(*pBundle).newMessage(CellappInterface::onRemoteCallMethodFromClient);
	(*pBundle) << srcEntityID;
	(*pBundle).append(s);
	
	e->getCellMailbox()->postMail((*pBundle));
	s.read_skip(s.opsize());
	Mercury::Bundle::ObjPool().reclaimObject(pBundle);
}

//-------------------------------------------------------------------------------------
void Baseapp::onUpdateDataFromClient(Mercury::Channel* pChannel, KBEngine::MemoryStream& s)
{
	ENTITY_ID srcEntityID = pChannel->proxyID();
	if(srcEntityID <= 0)
		return;
	
	if(s.opsize() <= 0)
		return;

	KBEngine::Proxy* e = static_cast<KBEngine::Proxy*>
			(KBEngine::Baseapp::getSingleton().findEntity(srcEntityID));	

	if(e == NULL || e->getCellMailbox() == NULL)
	{
		ERROR_MSG(boost::format("Baseapp::onUpdateDataFromClient: %1% %2% has no cell.\n") %
			(e == NULL ? "unknown" : e->getScriptName()) % srcEntityID);
		
		s.read_skip(s.opsize());
		return;
	}

	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
	(*pBundle).newMessage(CellappInterface::onUpdateDataFromClient);
	(*pBundle) << srcEntityID;
	(*pBundle).append(s);
	
	e->getCellMailbox()->postMail((*pBundle));
	s.read_skip(s.opsize());
	Mercury::Bundle::ObjPool().reclaimObject(pBundle);
}

//-------------------------------------------------------------------------------------
void Baseapp::onBackupEntityCellData(Mercury::Channel* pChannel, KBEngine::MemoryStream& s)
{
	if(pChannel->isExternal())
		return;

	ENTITY_ID baseID = 0;
	s >> baseID;

	Base* base = this->findEntity(baseID);

	if(base)
	{
		INFO_MSG(boost::format("Baseapp::onBackupEntityCellData: %1%(%2%), size=%3%.\n") % 
			base->getScriptName() % baseID % s.opsize());

		base->onBackupCellData(pChannel, s);
	}
	else
	{
		ERROR_MSG(boost::format("Baseapp::onBackupEntityCellData: not found entityID=%1%\n") % baseID);
		s.read_skip(s.opsize());
	}
}

//-------------------------------------------------------------------------------------
void Baseapp::onCellWriteToDBCompleted(Mercury::Channel* pChannel, KBEngine::MemoryStream& s)
{
	if(pChannel->isExternal())
		return;

	ENTITY_ID baseID = 0;
	CALLBACK_ID callbackID = 0;

	s >> baseID;
	s >> callbackID;

	Base* base = this->findEntity(baseID);

	if(base)
	{

		INFO_MSG(boost::format("Baseapp::onCellWriteToDBCompleted: %1%(%2%).\n") %
			base->getScriptName() % baseID);

		base->onCellWriteToDBCompleted(callbackID);
	}
	else
	{
		ERROR_MSG(boost::format("Baseapp::onCellWriteToDBCompleted: not found entityID=%1%\n") %
			baseID);
	}
}

//-------------------------------------------------------------------------------------
void Baseapp::onClientActiveTick(Mercury::Channel* pChannel)
{
	if(!pChannel->isExternal())
		return;

	onAppActiveTick(pChannel, CLIENT_TYPE, 0);
}

//-------------------------------------------------------------------------------------
void Baseapp::onWriteToDBCallback(Mercury::Channel* pChannel, ENTITY_ID eid, 
								  DBID entityDBID, CALLBACK_ID callbackID, bool success)
{
	if(pChannel->isExternal())
		return;

	Base* base = pEntities_->find(eid);
	if(base == NULL)
	{
		// ERROR_MSG("Baseapp::onWriteToDBCallback: can't found entity:%d.\n", eid);
		return;
	}

	base->onWriteToDBCallback(eid, entityDBID, callbackID, success);
}

//-------------------------------------------------------------------------------------
void Baseapp::onHello(Mercury::Channel* pChannel, 
						const std::string& verInfo, 
						const std::string& encryptedKey)
{
	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
	
	pBundle->newMessage(ClientInterface::onHelloCB);
	(*pBundle) << KBEVersion::versionString();
	(*pBundle) << g_componentType;
	(*pBundle).send(getNetworkInterface(), pChannel);

	Mercury::Bundle::ObjPool().reclaimObject(pBundle);

	if(Mercury::g_channelExternalEncryptType > 0)
	{
		if(encryptedKey.size() > 3)
		{
			// 替换为一个加密的过滤器
			pChannel->pFilter(Mercury::createEncryptionFilter(Mercury::g_channelExternalEncryptType, encryptedKey));
		}
		else
		{
			WARNING_MSG(boost::format("Baseapp::onHello: client is not encrypted, addr=%1%\n") 
				% pChannel->c_str());
		}
	}
}

//-------------------------------------------------------------------------------------

}
