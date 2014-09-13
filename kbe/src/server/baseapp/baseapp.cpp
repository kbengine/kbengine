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
#include "py_file_descriptor.hpp"
#include "baseapp_interface.hpp"
#include "base_remotemethod.hpp"
#include "archiver.hpp"
#include "backuper.hpp"
#include "initprogress_handler.hpp"
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
#include "server/sendmail_threadtasks.hpp"
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

PyObject* createCellDataDictFromPersistentStream(MemoryStream& s, const char* entityType)
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

		PyObject* pyVal = propertyDescription->createFromPersistentStream(&s);

		if(!propertyDescription->getDataType()->isSameType(pyVal))
		{
			if(pyVal)
			{
				Py_DECREF(pyVal);
			}
			
			ERROR_MSG(boost::format("Baseapp::createCellDataDictFromPersistentStream: %1%.%2% is error, set to default!\n") % entityType % attrname);
			pyVal = propertyDescription->getDataType()->parseDefaultStr("");
		}

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
	pBaseAppData_(NULL),
	pendingLoginMgr_(ninterface),
	forward_messagebuffer_(ninterface),
	pBackuper_(),
	load_(0.f),
	numProxices_(0),
	pTelnetServer_(NULL),
	pRestoreEntityHandlers_(),
	pResmgrTimerHandle_()
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
bool Baseapp::canShutdown()
{
	Entities<Base>::ENTITYS_MAP& entities =  this->pEntities()->getEntities();
	Entities<Base>::ENTITYS_MAP::iterator iter = entities.begin();
	for(; iter != entities.end(); iter++)
	{
		if(static_cast<Base*>(iter->second.get())->hasDB())
		{
			lastShutdownFailReason_ = "destroyHasDBBases";
			return false;
		}
	}

	return true;
}

//-------------------------------------------------------------------------------------	
void Baseapp::onShutdownBegin()
{
	EntityApp<Base>::onShutdownBegin();

	// 通知脚本
	SCRIPT_OBJECT_CALL_ARGS1(getEntryScript().get(), const_cast<char*>("onBaseAppShutDown"), 
		const_cast<char*>("i"), 0);

	pRestoreEntityHandlers_.clear();
}

//-------------------------------------------------------------------------------------	
void Baseapp::onShutdown(bool first)
{
	EntityApp<Base>::onShutdown(first);

	if(first)
	{
		// 通知脚本
		SCRIPT_OBJECT_CALL_ARGS1(getEntryScript().get(), const_cast<char*>("onBaseAppShutDown"), 
			const_cast<char*>("i"), 1);
	}

	int count = g_serverConfig.getBaseApp().perSecsDestroyEntitySize;
	Entities<Base>::ENTITYS_MAP& entities =  this->pEntities()->getEntities();

	while(count > 0)
	{
		bool done = false;
		Entities<Base>::ENTITYS_MAP::iterator iter = entities.begin();
		for(; iter != entities.end(); iter++)
		{
			if(static_cast<Base*>(iter->second.get())->hasDB() && 
				static_cast<Base*>(iter->second.get())->cellMailbox() == NULL)
			{
				this->destroyEntity(static_cast<Base*>(iter->second.get())->id(), true);

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
void Baseapp::onShutdownEnd()
{
	EntityApp<Base>::onShutdownEnd();

	// 通知脚本
	SCRIPT_OBJECT_CALL_ARGS1(getEntryScript().get(), const_cast<char*>("onBaseAppShutDown"), 
		const_cast<char*>("i"), 2);
}

//-------------------------------------------------------------------------------------		
bool Baseapp::initializeWatcher()
{
	ProfileVal::setWarningPeriod(stampsPerSecond() / g_kbeSrvConfig.gameUpdateHertz());

	WATCH_OBJECT("numProxices", this, &Baseapp::numProxices);
	WATCH_OBJECT("numClients", this, &Baseapp::numClients);
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
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(),		time,							__py_gametime,												METH_VARARGS,			0);
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(),		createBase,						__py_createBase,											METH_VARARGS,			0);
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(),		createBaseLocally,				__py_createBase,											METH_VARARGS,			0);
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(),		createEntity,					__py_createBase,											METH_VARARGS,			0);
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(), 		createBaseAnywhere,				__py_createBaseAnywhere,									METH_VARARGS,			0);
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(), 		createBaseFromDBID,				__py_createBaseFromDBID,									METH_VARARGS,			0);
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(), 		createBaseAnywhereFromDBID,		__py_createBaseAnywhereFromDBID,							METH_VARARGS,			0);
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(), 		executeRawDatabaseCommand,		__py_executeRawDatabaseCommand,								METH_VARARGS,			0);
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(), 		quantumPassedPercent,			__py_quantumPassedPercent,									METH_VARARGS,			0);
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(), 		charge,							__py_charge,												METH_VARARGS,			0);
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(), 		registerFileDescriptor,			PyFileDescriptor::__py_registerFileDescriptor,				METH_VARARGS,			0);
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(), 		registerWriteFileDescriptor,	PyFileDescriptor::__py_registerWriteFileDescriptor,			METH_VARARGS,			0);
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(), 		deregisterFileDescriptor,		PyFileDescriptor::__py_deregisterFileDescriptor,			METH_VARARGS,			0);
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(), 		deregisterWriteFileDescriptor,	PyFileDescriptor::__py_deregisterWriteFileDescriptor,		METH_VARARGS,			0);
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(),		reloadScript,					__py_reloadScript,											METH_VARARGS,			0);
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(),		isShuttingDown,					__py_isShuttingDown,										METH_VARARGS,			0);
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(),		address,						__py_address,												METH_VARARGS,			0);
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(),		deleteBaseByDBID,				__py_deleteBaseByDBID,										METH_VARARGS,			0);
	return EntityApp<Base>::installPyModules();
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

		(*pBundle).send(this->networkInterface(), pChannel);
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
	pBackuper_->tick();
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
	loopCheckTimerHandle_ = this->mainDispatcher().addTimer(1000000, this,
							reinterpret_cast<void *>(TIMEOUT_CHECK_STATUS));

	if(Resmgr::respool_checktick > 0)
	{
		pResmgrTimerHandle_ = this->mainDispatcher().addTimer(int(Resmgr::respool_checktick * 1000000),
			Resmgr::getSingletonPtr(), NULL);

		INFO_MSG(boost::format("Baseapp::initializeEnd: started resmgr tick(%1%s)!\n") % 
			Resmgr::respool_checktick);
	}

	pBackuper_.reset(new Backuper());
	pArchiver_.reset(new Archiver());

	new SyncEntityStreamTemplateHandler(this->networkInterface());

	_g_lastTimestamp = timestamp();

	// 如果需要pyprofile则在此处安装
	// 结束时卸载并输出结果
	if(g_kbeSrvConfig.getBaseApp().profiles.open_pyprofile)
	{
		script::PyProfile::start("kbengine");
	}

	pTelnetServer_ = new TelnetServer(&this->mainDispatcher(), &this->networkInterface());
	pTelnetServer_->pScript(&this->getScript());
	return pTelnetServer_->start(g_kbeSrvConfig.getBaseApp().telnet_passwd, 
		g_kbeSrvConfig.getBaseApp().telnet_deflayer, 
		g_kbeSrvConfig.getBaseApp().telnet_port);
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


	RestoreEntityHandler* pRestoreEntityHandler = new RestoreEntityHandler(pChannel->componentID(), this->networkInterface());
	Entities<Base>::ENTITYS_MAP& entitiesMap = pEntities_->getEntities();
	Entities<Base>::ENTITYS_MAP::const_iterator iter = entitiesMap.begin();
	while (iter != entitiesMap.end())
	{
		Base* pBase = static_cast<Base*>(iter->second.get());
		
		EntityMailbox* cell = pBase->cellMailbox();
		if(cell && cell->componentID() == pChannel->componentID())
		{
			S_RELEASE(cell);
			pBase->cellMailbox(NULL);
			pBase->installCellDataAttr(pBase->getCellData());
			pBase->onCellAppDeath();
			pRestoreEntityHandler->pushEntity(pBase->id());
		}

		iter++;
	}

	pRestoreEntityHandlers_.push_back(KBEShared_ptr< RestoreEntityHandler >(pRestoreEntityHandler));
}

//-------------------------------------------------------------------------------------
void Baseapp::onRestoreEntitiesOver(RestoreEntityHandler* pRestoreEntityHandler)
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
void Baseapp::onRestoreSpaceCellFromOtherBaseapp(Mercury::Channel* pChannel, KBEngine::MemoryStream& s)
{
	COMPONENT_ID baseappID = 0, cellappID = 0;
	SPACE_ID spaceID = 0;
	ENTITY_ID spaceEntityID = 0;
	bool destroyed = false;
	ENTITY_SCRIPT_UID utype = 0;

	s >> baseappID >> cellappID >> spaceID >> spaceEntityID >> utype >> destroyed;

	INFO_MSG(boost::format("Baseapp::onRestoreSpaceCellFromOtherBaseapp: baseappID=%1%, cellappID=%6%, spaceID=%2%, spaceEntityID=%3%, destroyed=%4%, "
		"restoreEntityHandlers(%5%)\n") %
		baseappID % spaceID % spaceEntityID % destroyed % pRestoreEntityHandlers_.size() % cellappID);

	std::vector< KBEShared_ptr< RestoreEntityHandler > >::iterator resiter = pRestoreEntityHandlers_.begin();
	for(; resiter != pRestoreEntityHandlers_.end(); resiter++)
	{
		(*resiter)->onRestoreSpaceCellFromOtherBaseapp(baseappID, cellappID, spaceID, spaceEntityID, utype, destroyed);
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
						int8 componentType, uint64 componentID, int8 globalorderID, int8 grouporderID,
						uint32 intaddr, uint16 intport, uint32 extaddr, uint16 extport, std::string& extaddrEx)
{
	if(pChannel->isExternal())
		return;

	Components::ComponentInfos* cinfos = Componentbridge::getComponents().findComponent((
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

			Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
			(*pBundle).newMessage(DbmgrInterface::reqKillServer);
			(*pBundle) << g_componentID << g_componentType << KBEngine::getUsername() << KBEngine::getUserUID() << "Duplicate app-id.";
			(*pBundle).send(this->networkInterface(), pChannel);
			Mercury::Bundle::ObjPool().reclaimObject(pBundle);
		}
	}

	EntityApp<Base>::onRegisterNewApp(pChannel, uid, username, componentType, componentID, globalorderID, grouporderID,
									intaddr, intport, extaddr, extport, extaddrEx);

	KBEngine::COMPONENT_TYPE tcomponentType = (KBEngine::COMPONENT_TYPE)componentType;

	Components::COMPONENTS& cts = Componentbridge::getComponents().getComponents(DBMGR_TYPE);
	KBE_ASSERT(cts.size() >= 1);
	
	cinfos = 
		Componentbridge::getComponents().findComponent(tcomponentType, uid, componentID);

	cinfos->pChannel = NULL;

	int ret = Components::getSingleton().connectComponent(tcomponentType, uid, componentID);
	KBE_ASSERT(ret != -1);

	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();

	switch(tcomponentType)
	{
	case BASEAPP_TYPE:
		(*pBundle).newMessage(BaseappInterface::onRegisterNewApp);
		BaseappInterface::onRegisterNewAppArgs11::staticAddToBundle((*pBundle), 
			getUserUID(), getUsername(), BASEAPP_TYPE, componentID_, startGlobalOrder_, startGroupOrder_,
			this->networkInterface().intaddr().ip, this->networkInterface().intaddr().port, 
			this->networkInterface().extaddr().ip, this->networkInterface().extaddr().port, g_kbeSrvConfig.getConfig().externalAddress);
		break;
	case CELLAPP_TYPE:
		(*pBundle).newMessage(CellappInterface::onRegisterNewApp);
		CellappInterface::onRegisterNewAppArgs11::staticAddToBundle((*pBundle), 
			getUserUID(), getUsername(), BASEAPP_TYPE, componentID_, startGlobalOrder_, startGroupOrder_,
			this->networkInterface().intaddr().ip, this->networkInterface().intaddr().port, 
			this->networkInterface().extaddr().ip, this->networkInterface().extaddr().port, g_kbeSrvConfig.getConfig().externalAddress);
		break;
	default:
		KBE_ASSERT(false && "no support!\n");
		break;
	};
	
	(*pBundle).send(this->networkInterface(), cinfos->pChannel);
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
	Components::COMPONENTS& cts = Components::getSingleton().getComponents(DBMGR_TYPE);
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

	DbmgrInterface::queryEntityArgs6::staticAddToBundle((*pBundle), 
		g_componentID, 0, dbid, entityType, callbackID, entityID);
	
	pBundle->send(this->networkInterface(), dbmgrinfos->pChannel);
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
			if(pyfunc != NULL)
			{
				PyObject* pyResult = PyObject_CallFunction(pyfunc.get(), 
													const_cast<char*>("OKi"), 
													Py_None, dbid, wasActive);

				if(pyResult != NULL)
					Py_DECREF(pyResult);
				else
					SCRIPT_ERROR_CHECK();
			}
			else
			{
				ERROR_MSG(boost::format("Baseapp::onCreateBaseFromDBID: can't found callback:%1%.\n") %
					callbackID);
			}
		}
		
		s.opfini();
		return;
	}

	PyObject* pyDict = createCellDataDictFromPersistentStream(s, entityType.c_str());
	PyObject* e = Baseapp::getSingleton().createEntityCommon(entityType.c_str(), pyDict, false, entityID);
	if(e)
	{
		static_cast<Base*>(e)->dbid(dbid);
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
		if(pyfunc != NULL)
		{
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
			ERROR_MSG(boost::format("Baseapp::onCreateBaseFromDBID: can't found callback:%1%.\n") %
				callbackID);
		}
	}
}

//-------------------------------------------------------------------------------------
PyObject* Baseapp::__py_createBaseAnywhereFromDBID(PyObject* self, PyObject* args)
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
		PyErr_Format(PyExc_AssertionError, "Baseapp::createBaseAnywhereFromDBID: args is error, entityType=%s!", entityType);
		PyErr_PrintEx(0);

		if(entityType)
			free(entityType);

		return NULL;
	}

	if(EntityDef::findScriptModule(entityType) == NULL)
	{
		PyErr_Format(PyExc_AssertionError, "Baseapp::createBaseAnywhereFromDBID: entityType is error!");
		PyErr_PrintEx(0);
		free(entityType);
		return NULL;
	}

	if(dbid <= 0)
	{
		PyErr_Format(PyExc_AssertionError, "Baseapp::createBaseAnywhereFromDBID: dbid is error!");
		PyErr_PrintEx(0);
		free(entityType);
		return NULL;
	}

	if(pyCallback && !PyCallable_Check(pyCallback))
	{
		pyCallback = NULL;

		PyErr_Format(PyExc_AssertionError, "Baseapp::createBaseAnywhereFromDBID: callback is error!");
		PyErr_PrintEx(0);
		free(entityType);
		return NULL;
	}

	Baseapp::getSingleton().createBaseAnywhereFromDBID(entityType, dbid, pyCallback);

	free(entityType);
	S_Return;
}

//-------------------------------------------------------------------------------------
void Baseapp::createBaseAnywhereFromDBID(const char* entityType, DBID dbid, PyObject* pyCallback)
{
	Components::COMPONENTS& cts = Components::getSingleton().getComponents(DBMGR_TYPE);
	Components::ComponentInfos* dbmgrinfos = NULL;

	if(cts.size() > 0)
		dbmgrinfos = &(*cts.begin());

	if(dbmgrinfos == NULL || dbmgrinfos->pChannel == NULL || dbmgrinfos->cid == 0)
	{
		PyErr_Format(PyExc_AssertionError, "Baseapp::createBaseAnywhereFromDBID: not found dbmgr!\n");
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

	DbmgrInterface::queryEntityArgs6::staticAddToBundle((*pBundle), 
		g_componentID, 1, dbid, entityType, callbackID, entityID);
	
	pBundle->send(this->networkInterface(), dbmgrinfos->pChannel);
	Mercury::Bundle::ObjPool().reclaimObject(pBundle);
}

//-------------------------------------------------------------------------------------
void Baseapp::onCreateBaseAnywhereFromDBIDCallback(Mercury::Channel* pChannel, KBEngine::MemoryStream& s)
{
	size_t currpos = s.rpos();

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
		ERROR_MSG(boost::format("Baseapp::createBaseAnywhereFromDBID: create %1%(%2%) is failed.\n") % 
			entityType.c_str() % dbid);

		if(callbackID > 0)
		{
			SCOPED_PROFILE(SCRIPTCALL_PROFILE);

			Py_INCREF(Py_None);
			// baseRef, dbid, wasActive
			PyObjectPtr pyfunc = pyCallbackMgr_.take(callbackID);
			if(pyfunc != NULL)
			{
				PyObject* pyResult = PyObject_CallFunction(pyfunc.get(), 
													const_cast<char*>("OKi"), 
													Py_None, dbid, wasActive);

				if(pyResult != NULL)
					Py_DECREF(pyResult);
				else
					SCRIPT_ERROR_CHECK();
			}
			else
			{
				ERROR_MSG(boost::format("Baseapp::createBaseAnywhereFromDBID: can't found callback:%1%.\n") %
					callbackID);
			}
		}
		
		s.opfini();
		return;
	}

	Mercury::Channel* pBaseappmgrChannel = Components::getSingleton().getBaseappmgrChannel();
	if(pBaseappmgrChannel == NULL)
	{
		ERROR_MSG(boost::format("Baseapp::createBaseAnywhereFromDBID: create %1%(%2%) is failed, not found baseappmgr.\n") % 
			entityType.c_str() % dbid);
		return;
	}

	s.rpos(currpos);

	MemoryStream* stream = MemoryStream::ObjPool().createObject();
	(*stream) << g_componentID;
	stream->append(s);
	s.opfini();

	// 通知baseappmgr在其他baseapp上创建entity
	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
	pBundle->newMessage(BaseappmgrInterface::reqCreateBaseAnywhereFromDBID);
	pBundle->append((*stream));
	pBundle->send(this->networkInterface(), pBaseappmgrChannel);
	Mercury::Bundle::ObjPool().reclaimObject(pBundle);
	MemoryStream::ObjPool().reclaimObject(stream);
}

//-------------------------------------------------------------------------------------
void Baseapp::createBaseAnywhereFromDBIDOtherBaseapp(Mercury::Channel* pChannel, KBEngine::MemoryStream& s)
{
	std::string entityType;
	DBID dbid;
	CALLBACK_ID callbackID;
	bool success = false;
	bool wasActive = false;
	ENTITY_ID entityID;
	COMPONENT_ID sourceBaseappID;

	s >> sourceBaseappID;
	s >> entityType;
	s >> dbid;
	s >> callbackID;
	s >> success;
	s >> entityID;
	s >> wasActive;

	PyObject* pyDict = createCellDataDictFromPersistentStream(s, entityType.c_str());
	PyObject* e = Baseapp::getSingleton().createEntityCommon(entityType.c_str(), pyDict, false, entityID);
	if(e)
	{
		static_cast<Base*>(e)->dbid(dbid);
		static_cast<Base*>(e)->initializeEntity(pyDict);
		Py_DECREF(pyDict);
	}

	// 是否本地组件就是发起源， 如果是直接在本地调用回调
	if(g_componentID == sourceBaseappID)
	{
		onCreateBaseAnywhereFromDBIDOtherBaseappCallback(pChannel, g_componentID, entityType, static_cast<Base*>(e)->id(), callbackID, dbid);
	}
	else
	{
		// 通知baseapp, 创建好了
		Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
		pBundle->newMessage(BaseappInterface::onCreateBaseAnywhereFromDBIDOtherBaseappCallback);


		BaseappInterface::onCreateBaseAnywhereFromDBIDOtherBaseappCallbackArgs5::staticAddToBundle((*pBundle), 
			g_componentID, entityType, static_cast<Base*>(e)->id(), callbackID, dbid);

		Components::ComponentInfos* baseappinfos = Components::getSingleton().findComponent(BASEAPP_TYPE, sourceBaseappID);
		if(baseappinfos == NULL || baseappinfos->pChannel == NULL || baseappinfos->cid == 0)
		{
			ForwardItem* pFI = new ForwardItem();
			pFI->pHandler = NULL;
			pFI->pBundle = pBundle;
			forward_messagebuffer_.push(sourceBaseappID, pFI);
			WARNING_MSG(boost::format("Baseapp::createBaseAnywhereFromDBID: not found sourceBaseapp(%1%), message is buffered.\n") % sourceBaseappID);
			return;
		}
		
		pBundle->send(this->networkInterface(), baseappinfos->pChannel);
		Mercury::Bundle::ObjPool().reclaimObject(pBundle);
	}
}

//-------------------------------------------------------------------------------------
void Baseapp::onCreateBaseAnywhereFromDBIDOtherBaseappCallback(Mercury::Channel* pChannel, COMPONENT_ID createByBaseappID, 
															   std::string entityType, ENTITY_ID createdEntityID, CALLBACK_ID callbackID, DBID dbid)
{
	if(callbackID > 0)
	{
		ScriptDefModule* sm = EntityDef::findScriptModule(entityType.c_str());
		if(sm == NULL)
		{
			ERROR_MSG(boost::format("Baseapp::onCreateBaseAnywhereFromDBIDOtherBaseappCallback: not found entityType:%1%.\n") %
				entityType.c_str());

			return;
		}

		SCOPED_PROFILE(SCRIPTCALL_PROFILE);

		// baseRef, dbid, wasActive
		PyObjectPtr pyfunc = pyCallbackMgr_.take(callbackID);
		if(pyfunc != NULL)
		{
			Base* pbase = this->findEntity(createdEntityID);

			PyObject* pyResult = NULL;
			
			if(pbase)
			{
				pyResult = PyObject_CallFunction(pyfunc.get(), 
												const_cast<char*>("OKi"), 
												pbase, dbid, 0);
			}
			else
			{
				PyObject* mb = static_cast<PyObject*>(new EntityMailbox(sm, NULL, createByBaseappID, createdEntityID, MAILBOX_TYPE_BASE));
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
			ERROR_MSG(boost::format("Baseapp::createBaseAnywhereFromDBID: not found callback:%1%.\n") %
				callbackID);
		}
	}
}

//-------------------------------------------------------------------------------------
void Baseapp::createInNewSpace(Base* base, PyObject* cell)
{
	ENTITY_ID id = base->id();
	std::string entityType = base->ob_type->tp_name;

	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();

	(*pBundle).newMessage(CellappmgrInterface::reqCreateInNewSpace);

	(*pBundle) << entityType;
	(*pBundle) << id;
	(*pBundle) << componentID_;

	MemoryStream* s = MemoryStream::ObjPool().createObject();
	base->addCellDataToStream(ED_FLAG_ALL, s);
	(*pBundle).append(*s);
	MemoryStream::ObjPool().reclaimObject(s);
	
	Components::COMPONENTS& components = Components::getSingleton().getComponents(CELLAPPMGR_TYPE);
	Components::COMPONENTS::iterator iter = components.begin();
	if(iter != components.end())
	{
		if((*iter).pChannel != NULL)
		{
			(*pBundle).send(this->networkInterface(), (*iter).pChannel);
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
	ENTITY_ID id = base->id();
	std::string entityType = base->ob_type->tp_name;

	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();

	(*pBundle).newMessage(CellappmgrInterface::reqRestoreSpaceInCell);

	(*pBundle) << entityType;
	(*pBundle) << id;
	(*pBundle) << componentID_;
	(*pBundle) << base->spaceID();

	MemoryStream* s = MemoryStream::ObjPool().createObject();
	base->addCellDataToStream(ED_FLAG_ALL, s);
	(*pBundle).append(*s);
	MemoryStream::ObjPool().reclaimObject(s);
	
	Components::COMPONENTS& components = Components::getSingleton().getComponents(CELLAPPMGR_TYPE);
	Components::COMPONENTS::iterator iter = components.begin();
	if(iter != components.end())
	{
		if((*iter).pChannel != NULL)
		{
			(*pBundle).send(this->networkInterface(), (*iter).pChannel);
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
			(*pBundle).send(this->networkInterface(), (*iter).pChannel);
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
			(*pBundle) << base->id();
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
		(*pForwardbundle) << base->id();
		(*pForwardbundle) << componentID_;
		(*pForwardbundle).send(this->networkInterface(), lpChannel);
		Mercury::Bundle::ObjPool().reclaimObject(pForwardbundle);
	}
	else
	{
		ENTITY_ID eid = base->id();
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
			ERROR_MSG(boost::format("Baseapp::onCreateBaseAnywhereCallback: can't found callback:%1%.\n") %
				callbackID);
		}

		//Py_DECREF(mb);
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
			ERROR_MSG(boost::format("Baseapp::onCreateBaseAnywhereCallback: can't found callback:%1%.\n") %
				callbackID);
		}
	}

	SCRIPT_ERROR_CHECK();
	Py_DECREF(pyargs);
}

//-------------------------------------------------------------------------------------
void Baseapp::createCellEntity(EntityMailboxAbstract* createToCellMailbox, Base* base)
{
	if(base->cellMailbox())
	{
		ERROR_MSG(boost::format("Baseapp::createCellEntity: %1% %2% has a cell!\n") %
			base->scriptName() % base->id());

		return;
	}

	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
	(*pBundle).newMessage(CellappInterface::onCreateCellEntityFromBaseapp);

	ENTITY_ID id = base->id();
	std::string entityType = base->ob_type->tp_name;

	EntityMailbox* clientMailbox = base->clientMailbox();
	bool hasClient = (clientMailbox != NULL);
	
	(*pBundle) << createToCellMailbox->id();				// 在这个mailbox所在的cellspace上创建
	(*pBundle) << entityType;
	(*pBundle) << id;
	(*pBundle) << componentID_;
	(*pBundle) << hasClient;
	(*pBundle) << base->inRestore();

	MemoryStream* s = MemoryStream::ObjPool().createObject();
	base->addCellDataToStream(ED_FLAG_ALL, s);
	(*pBundle).append(*s);
	MemoryStream::ObjPool().reclaimObject(s);
	
	if(createToCellMailbox->getChannel() == NULL)
	{
		ERROR_MSG(boost::format("Baseapp::createCellEntity: not found cellapp(createToCellMailbox:"
			"componentID=%1%, entityID=%2%), create is error!\n") %
			createToCellMailbox->componentID() % createToCellMailbox->id());

		base->onCreateCellFailure();
		Mercury::Bundle::ObjPool().reclaimObject(pBundle);
		return;
	}

	(*pBundle).send(this->networkInterface(), createToCellMailbox->getChannel());
	Mercury::Bundle::ObjPool().reclaimObject(pBundle);
}

//-------------------------------------------------------------------------------------
void Baseapp::onCreateCellFailure(Mercury::Channel* pChannel, ENTITY_ID entityID)
{
	if(pChannel->isExternal())
		return;

	Base* base = pEntities_->find(entityID);

	// 可能客户端在期间掉线了
	if(base == NULL)
	{
		ERROR_MSG(boost::format("Baseapp::onCreateCellFailure: not found entity(%1%)!\n") % entityID);
		return;
	}

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
	
	// 可能客户端在期间掉线了
	if(base == NULL)
	{
		Mercury::Bundle::SmartPoolObjectPtr pBundle = Mercury::Bundle::createSmartPoolObj();

		(*(*pBundle)).newMessage(CellappInterface::onDestroyCellEntityFromBaseapp);
		(*(*pBundle)) << id;
		(*(*pBundle)).send(this->networkInterface(), pChannel);
		ERROR_MSG(boost::format("Baseapp::onEntityGetCell: not found entity(%1%), I will destroyEntityCell!\n") % id);
		return;
	}

	if(base->spaceID() != spaceID)
		base->spaceID(spaceID);

	// 如果是有客户端的entity则需要告知客户端， 自身entity已经进入世界了。
	if(base->clientMailbox() != NULL)
	{
		onClientEntityEnterWorld(static_cast<Proxy*>(base), componentID);
	}

	base->onGetCell(pChannel, componentID);
}

//-------------------------------------------------------------------------------------
void Baseapp::onClientEntityEnterWorld(Proxy* base, COMPONENT_ID componentID)
{
	base->initClientCellPropertys();
	base->onClientGetCell(NULL, componentID);
}

//-------------------------------------------------------------------------------------
bool Baseapp::createClientProxies(Proxy* base, bool reload)
{
	// 将通道代理的关系与该entity绑定， 在后面通信中可提供身份合法性识别
	Mercury::Channel* pChannel = base->clientMailbox()->getChannel();
	pChannel->proxyID(base->id());
	base->addr(pChannel->addr());

	// 重新生成一个ID
	if(reload)
		base->rndUUID(genUUID64());

	// 让客户端知道已经创建了proxices, 并初始化一部分属性
	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
	(*pBundle).newMessage(ClientInterface::onCreatedProxies);
	(*pBundle) << base->rndUUID();
	(*pBundle) << base->id();
	(*pBundle) << base->ob_type->tp_name;
	//base->clientMailbox()->postMail((*pBundle));
	base->sendToClient(ClientInterface::onCreatedProxies, pBundle);
	//Mercury::Bundle::ObjPool().reclaimObject(pBundle);

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
	
	Baseapp::getSingleton().executeRawDatabaseCommand(data, size, pycallback, eid);
	S_Return;
}

//-------------------------------------------------------------------------------------
void Baseapp::executeRawDatabaseCommand(const char* datas, uint32 size, PyObject* pycallback, ENTITY_ID eid)
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
	(*pBundle).send(this->networkInterface(), dbmgrinfos->pChannel);
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
			ERROR_MSG(boost::format("Baseapp::onExecuteRawDatabaseCommandCB: can't found callback:%1%.\n") %
				callbackID);
		}
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

	if(Baseapp::getSingleton().isShuttingdown())
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::charge(%s): shuttingdown, operation not allowed! dbid=%"PRIu64, 
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
	CALLBACK_ID callbackID = callbackMgr().save(pycallback, uint64(g_kbeSrvConfig.billingSystem_orders_timeout_ + 
		g_kbeSrvConfig.callback_timeout_));

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

	(*(*pBundle)).send(this->networkInterface(), pChannel);
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

		if(pycallback != NULL)
		{
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
			ERROR_MSG(boost::format("Baseapp::onChargeCB: can't found callback:%1%.\n") %
				callbackID);
		}
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

	new InitProgressHandler(this->networkInterface());
}

//-------------------------------------------------------------------------------------
void Baseapp::onBroadcastBaseAppDataChanged(Mercury::Channel* pChannel, KBEngine::MemoryStream& s)
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

	Py_INCREF(pyKey);

	if(isDelete)
	{
		if(pBaseAppData_->del(pyKey))
		{
			SCOPED_PROFILE(SCRIPTCALL_PROFILE);

			// 通知脚本
			SCRIPT_OBJECT_CALL_ARGS1(getEntryScript().get(), const_cast<char*>("onBaseAppDataDel"), 
				const_cast<char*>("O"), pyKey);
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

		Py_INCREF(pyValue);

		if(pBaseAppData_->write(pyKey, pyValue))
		{
			SCOPED_PROFILE(SCRIPTCALL_PROFILE);

			// 通知脚本
			SCRIPT_OBJECT_CALL_ARGS2(getEntryScript().get(), const_cast<char*>("onBaseAppData"), 
				const_cast<char*>("OO"), pyKey, pyValue);
		}

		Py_DECREF(pyValue);
	}

	Py_DECREF(pyKey);
}

//-------------------------------------------------------------------------------------
void Baseapp::registerPendingLogin(Mercury::Channel* pChannel, std::string& loginName, std::string& accountName, 
								   std::string& password, ENTITY_ID entityID, DBID entityDBID, uint32 flags, uint64 deadline,
								   COMPONENT_TYPE componentType)
{
	if(pChannel->isExternal())
		return;

	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
	(*pBundle).newMessage(BaseappmgrInterface::onPendingAccountGetBaseappAddr);

	(*pBundle) << loginName;
	(*pBundle) << accountName;
	
	if(strlen((const char*)&g_kbeSrvConfig.getBaseApp().externalAddress) > 0)
	{
		uint32 exip = inet_addr(g_kbeSrvConfig.getBaseApp().externalAddress);
		(*pBundle) << exip;
	}
	else
	{
		(*pBundle) << this->networkInterface().extaddr().ip;
	}

	(*pBundle) << this->networkInterface().extaddr().port;
	(*pBundle).send(this->networkInterface(), pChannel);

	PendingLoginMgr::PLInfos* ptinfos = new PendingLoginMgr::PLInfos;
	ptinfos->accountName = accountName;
	ptinfos->password = password;
	ptinfos->entityID = entityID;
	ptinfos->entityDBID = entityDBID;
	ptinfos->flags = flags;
	ptinfos->deadline = deadline;
	ptinfos->ctype = (COMPONENT_CLIENT_TYPE)componentType;
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
	(*pBundle).send(this->networkInterface(), pChannel);
	Mercury::Bundle::ObjPool().reclaimObject(pBundle);
}

//-------------------------------------------------------------------------------------
void Baseapp::loginGateway(Mercury::Channel* pChannel, 
						   std::string& accountName, 
						   std::string& password)
{
	accountName = KBEngine::strutil::kbe_trim(accountName);
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

	INFO_MSG(fmt::format("Baseapp::loginGateway: new user[{0}], channel[{1}].\n", 
		accountName, pChannel->c_str()));

	Components::COMPONENTS& cts = Components::getSingleton().getComponents(DBMGR_TYPE);
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
		loginGatewayFailed(pChannel, accountName, SERVER_ERR_ILLEGAL_LOGIN);
		return;
	}

	if(ptinfos->password != password)
	{
		loginGatewayFailed(pChannel, accountName, SERVER_ERR_PASSWORD);
		return;
	}

	if((ptinfos->flags & ACCOUNT_FLAG_LOCK) > 0)
	{
		loginGatewayFailed(pChannel, accountName, SERVER_ERR_ACCOUNT_LOCK);
		return;
	}

	if((ptinfos->flags & ACCOUNT_FLAG_NOT_ACTIVATED) > 0)
	{
		loginGatewayFailed(pChannel, accountName, SERVER_ERR_ACCOUNT_NOT_ACTIVATED);
		return;
	}

	if(ptinfos->deadline > 0 && ::time(NULL) - ptinfos->deadline <= 0)
	{
		loginGatewayFailed(pChannel, accountName, SERVER_ERR_ACCOUNT_DEADLINE);
		return;
	}

	if(idClient_.getSize() == 0)
	{
		ERROR_MSG("Baseapp::loginGateway: idClient size is 0.\n");
		loginGatewayFailed(pChannel, accountName, SERVER_ERR_SRV_NO_READY);
		return;
	}

	// 如果entityID大于0则说明此entity是存活状态登录
	if(ptinfos->entityID > 0)
	{
		INFO_MSG(boost::format("Baseapp::loginGateway: user[%1%] has entity(%2%).\n") %
			accountName.c_str() % ptinfos->entityID);

		Proxy* base = static_cast<Proxy*>(findEntity(ptinfos->entityID));
		if(base == NULL || base->isDestroyed())
		{
			loginGatewayFailed(pChannel, accountName, SERVER_ERR_BUSY);
			return;
		}
		
		// 通知脚本异常登录请求有脚本决定是否允许这个通道强制登录
		int32 ret = base->onLogOnAttempt(pChannel->addr().ipAsString(), 
			ntohs(pChannel->addr().port), password.c_str());

		switch(ret)
		{
		case LOG_ON_ACCEPT:
			if(base->clientMailbox() != NULL)
			{
				// 通告在别处登录
				Mercury::Channel* pOldClientChannel = base->clientMailbox()->getChannel();
				if(pOldClientChannel != NULL)
				{
					INFO_MSG(boost::format("Baseapp::loginGateway: script LOG_ON_ACCEPT. oldClientChannel=%1%\n") %
						pOldClientChannel->c_str());
					
					kickChannel(pOldClientChannel, SERVER_ERR_ACCOUNT_LOGIN_ANOTHER);
				}
				else
				{
					INFO_MSG("Baseapp::loginGateway: script LOG_ON_ACCEPT.\n");
				}
				
				base->clientMailbox()->addr(pChannel->addr());
				base->addr(pChannel->addr());
				base->setClientType(ptinfos->ctype);
				createClientProxies(base, true);
			}
			else
			{
				// 创建entity的客户端mailbox
				EntityMailbox* entityClientMailbox = new EntityMailbox(base->scriptModule(), 
					&pChannel->addr(), 0, base->id(), MAILBOX_TYPE_CLIENT);

				base->clientMailbox(entityClientMailbox);
				base->addr(pChannel->addr());
				base->setClientType(ptinfos->ctype);

				// 将通道代理的关系与该entity绑定， 在后面通信中可提供身份合法性识别
				entityClientMailbox->getChannel()->proxyID(base->id());
				createClientProxies(base, true);
			}
			break;
		case LOG_ON_WAIT_FOR_DESTROY:
		default:
			INFO_MSG("Baseapp::loginGateway: script LOG_ON_REJECT.\n");
			loginGatewayFailed(pChannel, accountName, SERVER_ERR_ACCOUNT_IS_ONLINE);
			return;
		};
	}
	else
	{
		Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
		(*pBundle).newMessage(DbmgrInterface::queryAccount);

		ENTITY_ID entityID = idClient_.alloc();
		KBE_ASSERT(entityID > 0);

		DbmgrInterface::queryAccountArgs7::staticAddToBundle((*pBundle), accountName, password, g_componentID, 
			entityID, ptinfos->entityDBID, pChannel->addr().ip, pChannel->addr().port);

		(*pBundle).send(this->networkInterface(), dbmgrinfos->pChannel);
		Mercury::Bundle::ObjPool().reclaimObject(pBundle);
	}

	// 记录客户端地址
	ptinfos->addr = pChannel->addr();
}

//-------------------------------------------------------------------------------------
void Baseapp::reLoginGateway(Mercury::Channel* pChannel, std::string& accountName, 
							 std::string& password, uint64 key, ENTITY_ID entityID)
{
	accountName = KBEngine::strutil::kbe_trim(accountName);
	INFO_MSG(boost::format("Baseapp::reLoginGateway: accountName=%1%, key=%2%, entityID=%3%.\n") %
		accountName % key % entityID);

	Base* base = findEntity(entityID);
	if(base == NULL || !PyObject_TypeCheck(base, Proxy::getScriptType()) || base->isDestroyed())
	{
		loginGatewayFailed(pChannel, accountName, SERVER_ERR_ILLEGAL_LOGIN);
		return;
	}
	
	Proxy* proxy = static_cast<Proxy*>(base);
	
	if(key == 0 || proxy->rndUUID() != key)
	{
		loginGatewayFailed(pChannel, accountName, SERVER_ERR_ILLEGAL_LOGIN);
		return;
	}

	EntityMailbox* entityClientMailbox = proxy->clientMailbox();
	if(entityClientMailbox != NULL)
	{
		Mercury::Channel* pMBChannel = entityClientMailbox->getChannel();

		WARNING_MSG(boost::format("Baseapp::reLoginGateway: accountName=%1%, key=%2%, "
			"entityID=%3%, ClientMailbox(%4%) is exist, will be kicked out!\n") %
			accountName % key % entityID % 
			(pMBChannel ? pMBChannel->c_str() : "unknown"));
		
		if(pMBChannel)
			pMBChannel->condemn();

		entityClientMailbox->addr(pChannel->addr());
	}
	else
	{
		// 创建entity的客户端mailbox
		entityClientMailbox = new EntityMailbox(proxy->scriptModule(), 
			&pChannel->addr(), 0, proxy->id(), MAILBOX_TYPE_CLIENT);

		proxy->clientMailbox(entityClientMailbox);
	}

	// 将通道代理的关系与该entity绑定， 在后面通信中可提供身份合法性识别
	proxy->addr(pChannel->addr());
	pChannel->proxyID(proxy->id());

	//createClientProxies(proxy, true);
	proxy->onEntitiesEnabled();

	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
	(*pBundle).newMessage(ClientInterface::onReLoginGatewaySuccessfully);
	(*pBundle).send(this->networkInterface(), pChannel);
	Mercury::Bundle::ObjPool().reclaimObject(pBundle);
}

//-------------------------------------------------------------------------------------
void Baseapp::kickChannel(Mercury::Channel* pChannel, SERVER_ERROR_CODE failedcode)
{
	if(pChannel == NULL)
		return;

	INFO_MSG(boost::format("Baseapp::kickChannel: pChannel=%1%, failedcode=%2%, proxyID=%3%.\n") %
		pChannel->c_str() % failedcode % pChannel->proxyID());

	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
	(*pBundle).newMessage(ClientInterface::onKicked);
	ClientInterface::onKickedArgs1::staticAddToBundle((*pBundle), failedcode);
	(*pBundle).send(this->networkInterface(), pChannel);
	Mercury::Bundle::ObjPool().reclaimObject(pBundle);

	pChannel->proxyID(0);
	pChannel->condemn();
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
	uint32 flags;
	uint64 deadline;

	s >> accountName >> password >> dbid >> success >> entityID >> flags >> deadline;

	PendingLoginMgr::PLInfos* ptinfos = pendingLoginMgr_.remove(accountName);
	if(ptinfos == NULL)
	{
		ERROR_MSG(boost::format("Baseapp::onQueryAccountCBFromDbmgr: PendingLoginMgr not found(%1%)\n") %
			accountName.c_str());

		s.opfini();
		return;
	}

	Proxy* base = static_cast<Proxy*>(createEntityCommon(g_serverConfig.getDBMgr().dbAccountEntityScriptType, 
		NULL, false, entityID));

	Mercury::Channel* pClientChannel = this->networkInterface().findChannel(ptinfos->addr);

	if(!success)
	{
		std::string error;
		s >> error;
		ERROR_MSG(boost::format("Baseapp::onQueryAccountCBFromDbmgr: query %1% is failed! error(%2%)\n") %
			accountName.c_str() % error);
		
		s.opfini();
		
		loginGatewayFailed(pClientChannel, accountName, SERVER_ERR_SRV_NO_READY);
		return;
	}
	
	KBE_ASSERT(base != NULL);
	base->hasDB(true);
	base->dbid(dbid);
	base->setClientType(ptinfos->ctype);

	PyObject* pyDict = createCellDataDictFromPersistentStream(s, g_serverConfig.getDBMgr().dbAccountEntityScriptType);

	PyObject* py__ACCOUNT_NAME__ = PyUnicode_FromString(accountName.c_str());
	PyDict_SetItemString(pyDict, "__ACCOUNT_NAME__", py__ACCOUNT_NAME__);
	Py_DECREF(py__ACCOUNT_NAME__);

	PyObject* py__ACCOUNT_PASSWD__ = PyUnicode_FromString(KBE_MD5::getDigest(password.data(), password.length()).c_str());
	PyDict_SetItemString(pyDict, "__ACCOUNT_PASSWORD__", py__ACCOUNT_PASSWD__);
	Py_DECREF(py__ACCOUNT_PASSWD__);

	base->initializeEntity(pyDict);
	Py_DECREF(pyDict);

	if(pClientChannel != NULL)
	{
		// 创建entity的客户端mailbox
		EntityMailbox* entityClientMailbox = new EntityMailbox(base->scriptModule(), 
			&pClientChannel->addr(), 0, base->id(), MAILBOX_TYPE_CLIENT);

		base->clientMailbox(entityClientMailbox);
		base->addr(pClientChannel->addr());

		createClientProxies(base);
		
		/*
		Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
		(*pBundle).newMessage(DbmgrInterface::onAccountOnline);

		DbmgrInterface::onAccountOnlineArgs3::staticAddToBundle((*pBundle), accountName, 
			componentID_, base->id());

		(*pBundle).send(this->networkInterface(), pChannel);
		Mercury::Bundle::ObjPool().reclaimObject(pBundle);
		*/
	}

	INFO_MSG(boost::format("Baseapp::onQueryAccountCBFromDbmgr: user=%1%, uuid=%2%, entityID=%3%, flags=%4%, deadline=%5%.\n") %
		accountName % base->rndUUID() % base->id() % flags % deadline);

	SAFE_RELEASE(ptinfos);
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
		if(s.opsize() > 0)
		{
			if(Mercury::g_trace_packet > 0 && s.opsize() >= sizeof(Mercury::MessageID))
			{
				Mercury::MessageID fmsgid = 0;
				s >> fmsgid;
				Mercury::MessageHandler* pMessageHandler = ClientInterface::messageHandlers.find(fmsgid);
				bool isprint = true;

				if(pMessageHandler)
				{
					std::vector<std::string>::iterator iter = std::find(Mercury::g_trace_packet_disables.begin(),	
															Mercury::g_trace_packet_disables.end(),				
																pMessageHandler->name);							
																													
					if(iter != Mercury::g_trace_packet_disables.end())												
					{																								
						isprint = false;																			
					}																								
				}

				if(isprint)
				{
					ERROR_MSG(boost::format("Baseapp::forwardMessageToClientFromCellapp: entityID %1% not found, %2%(msgid=%3%).\n") % 
						eid % (pMessageHandler == NULL ? "unknown" : pMessageHandler->name) % fmsgid);
				}
				else
				{
					ERROR_MSG(boost::format("Baseapp::forwardMessageToClientFromCellapp: entityID %1% not found.\n") % eid);
				}
			}
			else
			{
				ERROR_MSG(boost::format("Baseapp::forwardMessageToClientFromCellapp: entityID %1% not found.\n") % eid);
			}
		}

		s.opfini();
		return;
	}

	EntityMailboxAbstract* mailbox = static_cast<EntityMailboxAbstract*>(base->clientMailbox());
	if(mailbox == NULL)
	{
		if(s.opsize() > 0)
		{
			if(Mercury::g_trace_packet > 0 && s.opsize() >= sizeof(Mercury::MessageID))
			{
				Mercury::MessageID fmsgid = 0;
				s >> fmsgid;
				Mercury::MessageHandler* pMessageHandler = ClientInterface::messageHandlers.find(fmsgid);
				bool isprint = true;

				if(pMessageHandler)
				{
					std::vector<std::string>::iterator iter = std::find(Mercury::g_trace_packet_disables.begin(),	
															Mercury::g_trace_packet_disables.end(),				
																pMessageHandler->name);							
																													
					if(iter != Mercury::g_trace_packet_disables.end())												
					{																								
						isprint = false;																			
					}																								
				}

				if(isprint)
				{
					ERROR_MSG(boost::format("Baseapp::forwardMessageToClientFromCellapp: "
						"is error(not found clientMailbox)! entityID(%1%), %2%(msgid=%3%).\n") % 
						eid % (pMessageHandler == NULL ? "unknown" : pMessageHandler->name) % fmsgid);
				}
				else
				{
					ERROR_MSG(boost::format("Baseapp::forwardMessageToClientFromCellapp: "
						"is error(not found clientMailbox)! entityID(%1%).\n") % 
						eid);
				}
			}
			else
			{
				ERROR_MSG(boost::format("Baseapp::forwardMessageToClientFromCellapp: "
					"is error(not found clientMailbox)! entityID(%1%).\n") % 
					eid);
			}
		}

		s.opfini();
		return;
	}
	
	if(s.opsize() <= 0)
		return;

	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
	(*pBundle).append(s);
	static_cast<Proxy*>(base)->sendToClient(pBundle);
	//mailbox->postMail((*pBundle));
	//Mercury::Bundle::ObjPool().reclaimObject(pBundle);
	
	if(Mercury::g_trace_packet > 0 && s.opsize() >= sizeof(Mercury::MessageID))
	{
		Mercury::MessageID fmsgid = 0;
		s >> fmsgid;
		Mercury::MessageHandler* pMessageHandler = ClientInterface::messageHandlers.find(fmsgid);
		bool isprint = true;

		if(pMessageHandler)
		{
			(*pBundle).pCurrMsgHandler(pMessageHandler);
			std::vector<std::string>::iterator iter = std::find(Mercury::g_trace_packet_disables.begin(),	
													Mercury::g_trace_packet_disables.end(),				
														pMessageHandler->name);							
																											
			if(iter != Mercury::g_trace_packet_disables.end())												
			{																								
				isprint = false;																			
			}																								
		}

		if(isprint)
		{
			DEBUG_MSG(boost::format("Baseapp::forwardMessageToClientFromCellapp: %1%(msgid=%2%).\n") %
				(pMessageHandler == NULL ? "unknown" : pMessageHandler->name) % fmsgid);
		}
	}

	s.read_skip(s.opsize());
}

//-------------------------------------------------------------------------------------
void Baseapp::forwardMessageToCellappFromCellapp(Mercury::Channel* pChannel, 
												KBEngine::MemoryStream& s)
{
	if(pChannel->isExternal())
		return;
	
	ENTITY_ID eid;
	s >> eid;

	Base* base = pEntities_->find(eid);
	if(base == NULL)
	{
		ERROR_MSG(boost::format("Baseapp::forwardMessageToCellappFromCellapp: entityID %1% not found.\n") % eid);
		s.opfini();
		return;
	}

	EntityMailboxAbstract* mailbox = static_cast<EntityMailboxAbstract*>(base->cellMailbox());
	if(mailbox == NULL)
	{
		ERROR_MSG(boost::format("Baseapp::forwardMessageToCellappFromCellapp: "
			"is error(not found cellMailbox)! entityID=%1%.\n") % 
			eid);

		s.opfini();
		return;
	}
	
	if(s.opsize() <= 0)
		return;

	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
	(*pBundle).append(s);
	base->sendToCellapp(pBundle);
	
	if(Mercury::g_trace_packet > 0 && s.opsize() >= sizeof(Mercury::MessageID))
	{
		Mercury::MessageID fmsgid = 0;
		s >> fmsgid;
		Mercury::MessageHandler* pMessageHandler = CellappInterface::messageHandlers.find(fmsgid);
		bool isprint = true;

		if(pMessageHandler)
		{
			(*pBundle).pCurrMsgHandler(pMessageHandler);
			std::vector<std::string>::iterator iter = std::find(Mercury::g_trace_packet_disables.begin(),	
													Mercury::g_trace_packet_disables.end(),				
														pMessageHandler->name);							
																											
			if(iter != Mercury::g_trace_packet_disables.end())												
			{																								
				isprint = false;																			
			}																								
		}

		if(isprint)
		{
			DEBUG_MSG(boost::format("Baseapp::forwardMessageToCellappFromCellapp: %1%(msgid=%2%).\n") %
				(pMessageHandler == NULL ? "unknown" : pMessageHandler->name) % fmsgid);
		}
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
	bool reclaim = true;

	switch(mailtype)
	{
		case MAILBOX_TYPE_BASE:		// 本组件是baseapp，那么确认邮件的目的地是这里， 那么执行最终操作
			base->onRemoteMethodCall(pChannel, s);
			break;
		case MAILBOX_TYPE_CELL_VIA_BASE: // entity.cell.base.xxx
			{
				EntityMailboxAbstract* mailbox = static_cast<EntityMailboxAbstract*>(base->cellMailbox());
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
				EntityMailboxAbstract* mailbox = static_cast<EntityMailboxAbstract*>(base->clientMailbox());
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
				//mailbox->postMail(bundle);
				static_cast<Proxy*>(base)->sendToClient(pBundle);
				reclaim = false;
			}
			break;
		default:
			{
				ERROR_MSG(boost::format("Baseapp::onEntityMail: mailboxType %1% is error! must a baseType. entityID=%2%.\n") %
					mailtype % eid);
			}
	};

	if(reclaim)
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

	if(e == NULL || e->cellMailbox() == NULL)
	{
		ERROR_MSG(boost::format("Baseapp::onRemoteCallCellMethodFromClient: %1% %2% has no cell.\n") %
			e->scriptName() % srcEntityID);
		
		s.read_skip(s.opsize());
		return;
	}

	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
	(*pBundle).newMessage(CellappInterface::onRemoteCallMethodFromClient);
	(*pBundle) << srcEntityID;
	(*pBundle).append(s);
	
	e->sendToCellapp(pBundle);
	s.read_skip(s.opsize());
}

//-------------------------------------------------------------------------------------
void Baseapp::onUpdateDataFromClient(Mercury::Channel* pChannel, KBEngine::MemoryStream& s)
{
	ENTITY_ID srcEntityID = pChannel->proxyID();
	if(srcEntityID <= 0)
	{
		s.opfini();
		return;
	}
	
	static size_t datasize = (sizeof(float) * 6 + sizeof(uint8) + sizeof(uint32));
	if(s.opsize() <= 0 || s.opsize() != datasize)
	{
		ERROR_MSG(boost::format("Baseapp::onUpdateDataFromClient: invalid data, size(%1% != %2%), srcEntityID=%3%.\n") %
			datasize % s.opsize() % srcEntityID);

		s.opfini();
		return;
	}

	KBEngine::Proxy* e = static_cast<KBEngine::Proxy*>
			(KBEngine::Baseapp::getSingleton().findEntity(srcEntityID));	

	if(e == NULL || e->cellMailbox() == NULL)
	{
		ERROR_MSG(boost::format("Baseapp::onUpdateDataFromClient: %1% %2% has no cell.\n") %
			(e == NULL ? "unknown" : e->scriptName()) % srcEntityID);
		
		s.read_skip(s.opsize());
		return;
	}

	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
	(*pBundle).newMessage(CellappInterface::onUpdateDataFromClient);
	(*pBundle) << srcEntityID;
	(*pBundle).append(s);
	
	e->sendToCellapp(pBundle);
	s.read_skip(s.opsize());
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
			base->scriptName() % baseID % s.opsize());

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
			base->scriptName() % baseID);

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
						const std::string& scriptVerInfo,
						const std::string& encryptedKey)
{
	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
	
	pBundle->newMessage(ClientInterface::onHelloCB);
	(*pBundle) << KBEVersion::versionString();
	(*pBundle) << KBEVersion::scriptVersionString();
	(*pBundle) << g_componentType;
	(*pBundle).send(networkInterface(), pChannel);

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
void Baseapp::lookApp(Mercury::Channel* pChannel)
{
	if(pChannel->isExternal())
		return;

	DEBUG_MSG(boost::format("Baseapp::lookApp: %1%\n") % pChannel->c_str());

	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
	
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

	(*pBundle).send(networkInterface(), pChannel);

	Mercury::Bundle::ObjPool().reclaimObject(pBundle);
}

//-------------------------------------------------------------------------------------
void Baseapp::importClientMessages(Mercury::Channel* pChannel)
{
	static Mercury::Bundle bundle;

	if(bundle.packets().size() == 0)
	{
		std::map< Mercury::MessageID, Mercury::ExposedMessageInfo > messages;

		{
			const Mercury::MessageHandlers::MessageHandlerMap& msgHandlers = BaseappInterface::messageHandlers.msgHandlers();
			Mercury::MessageHandlers::MessageHandlerMap::const_iterator iter = msgHandlers.begin();
			for(; iter != msgHandlers.end(); iter++)
			{
				Mercury::MessageHandler* pMessageHandler = iter->second;
				if(!iter->second->exposed)
					continue;

				Mercury::ExposedMessageInfo& info = messages[iter->first];
				info.id = iter->first;
				info.name = pMessageHandler->name;
				info.msgLen = pMessageHandler->msgLen;
				info.argsType = (int8)pMessageHandler->pArgs->type();

				KBEngine::strutil::kbe_replace(info.name, "::", "_");
				std::vector<std::string>::iterator iter1 = pMessageHandler->pArgs->strArgsTypes.begin();
				for(; iter1 !=  pMessageHandler->pArgs->strArgsTypes.end(); iter1++)
				{
					info.argsTypes.push_back((uint8)datatype2id((*iter1)));
				}
			}
		}

		bundle.newMessage(ClientInterface::onImportClientMessages);
		uint16 size = messages.size();
		bundle << size;

		std::map< Mercury::MessageID, Mercury::ExposedMessageInfo >::iterator iter = messages.begin();
		for(; iter != messages.end(); iter++)
		{
			uint8 argsize = iter->second.argsTypes.size();
			bundle << iter->second.id << iter->second.msgLen << iter->second.name << iter->second.argsType << argsize;

			std::vector<uint8>::iterator argiter = iter->second.argsTypes.begin();
			for(; argiter != iter->second.argsTypes.end(); argiter++)
			{
				bundle << (*argiter);
			}
		}
	}

	bundle.resend(networkInterface(), pChannel);
}

//-------------------------------------------------------------------------------------
void Baseapp::importClientEntityDef(Mercury::Channel* pChannel)
{
	static Mercury::Bundle bundle;
	
	if(bundle.packets().size() == 0)
	{
		ENTITY_PROPERTY_UID posuid = ENTITY_BASE_PROPERTY_UTYPE_POSITION_XYZ;
		ENTITY_PROPERTY_UID diruid = ENTITY_BASE_PROPERTY_UTYPE_DIRECTION_ROLL_PITCH_YAW;
		ENTITY_PROPERTY_UID spaceuid = ENTITY_BASE_PROPERTY_UTYPE_SPACEID;

		Mercury::FixedMessages::MSGInfo* msgInfo =
					Mercury::FixedMessages::getSingleton().isFixed("Property::position");

		if(msgInfo != NULL)
			posuid = msgInfo->msgid;

		msgInfo = Mercury::FixedMessages::getSingleton().isFixed("Property::direction");
		if(msgInfo != NULL)
			diruid = msgInfo->msgid;

		msgInfo = Mercury::FixedMessages::getSingleton().isFixed("Property::spaceID");
		if(msgInfo != NULL)
			spaceuid = msgInfo->msgid;

		bundle.newMessage(ClientInterface::onImportClientEntityDef);
		
		const DataTypes::UID_DATATYPE_MAP& dataTypes = DataTypes::uid_dataTypes();
		uint16 aliassize = dataTypes.size();
		bundle << aliassize;

		DataTypes::UID_DATATYPE_MAP::const_iterator dtiter = dataTypes.begin();
		for(; dtiter != dataTypes.end(); dtiter++)
		{
			const DataType* datatype = dtiter->second;

			bundle << datatype->id();
			bundle << dtiter->first;
			bundle << datatype->getName();

			if(strcmp(datatype->getName(), "FIXED_DICT") == 0)
			{
				FixedDictType* dictdatatype = const_cast<FixedDictType*>(static_cast<const FixedDictType*>(datatype));
				
				FixedDictType::FIXEDDICT_KEYTYPE_MAP& keys = dictdatatype->getKeyTypes();

				uint8 keysize = keys.size();
				bundle << keysize;
				
				bundle << dictdatatype->moduleName();

				FixedDictType::FIXEDDICT_KEYTYPE_MAP::const_iterator keyiter = keys.begin();
				for(; keyiter != keys.end(); keyiter++)
				{
					bundle << keyiter->first;
					bundle << keyiter->second->dataType->id();
				}
			}
			else if(strcmp(datatype->getName(), "ARRAY") == 0)
			{
				bundle << const_cast<FixedArrayType*>(static_cast<const FixedArrayType*>(datatype))->getDataType()->id();
			}
		}

		const EntityDef::SCRIPT_MODULES& modules = EntityDef::getScriptModules();
		EntityDef::SCRIPT_MODULES::const_iterator iter = modules.begin();
		for(; iter != modules.end(); iter++)
		{
			const ScriptDefModule::PROPERTYDESCRIPTION_MAP& propers = iter->get()->getClientPropertyDescriptions();
			const ScriptDefModule::METHODDESCRIPTION_MAP& methods = iter->get()->getClientMethodDescriptions();
			const ScriptDefModule::METHODDESCRIPTION_MAP& methods1 = iter->get()->getBaseExposedMethodDescriptions();
			const ScriptDefModule::METHODDESCRIPTION_MAP& methods2 = iter->get()->getCellExposedMethodDescriptions();

			if(!iter->get()->hasClient())
				continue;

			uint16 size = propers.size() + 3 /* pos, dir, spaceID */;
			uint16 size1 = methods.size();
			uint16 size2 = methods1.size();
			uint16 size3 = methods2.size();

			bundle << iter->get()->getName() << iter->get()->getUType() << size << size1 << size2 << size3;
			
			int16 aliasID = ENTITY_BASE_PROPERTY_ALIASID_POSITION_XYZ;
			bundle << posuid << aliasID << "position" << "" << DataTypes::getDataType("VECTOR3")->id();

			aliasID = ENTITY_BASE_PROPERTY_ALIASID_DIRECTION_ROLL_PITCH_YAW;
			bundle << diruid << aliasID << "direction" << "" << DataTypes::getDataType("VECTOR3")->id();

			aliasID = ENTITY_BASE_PROPERTY_ALIASID_SPACEID;
			bundle << spaceuid << aliasID << "spaceID" << "" << DataTypes::getDataType("UINT32")->id();

			ScriptDefModule::PROPERTYDESCRIPTION_MAP::const_iterator piter = propers.begin();
			for(; piter != propers.end(); piter++)
			{
				ENTITY_PROPERTY_UID	properUtype = piter->second->getUType();
				int16 aliasID = piter->second->aliasID();
				std::string	name = piter->second->getName();
				std::string	defaultValStr = piter->second->getDefaultValStr();

				bundle << properUtype << aliasID << name << defaultValStr << piter->second->getDataType()->id();
			}
			
			ScriptDefModule::METHODDESCRIPTION_MAP::const_iterator miter = methods.begin();
			for(; miter != methods.end(); miter++)
			{
				ENTITY_METHOD_UID methodUtype = miter->second->getUType();
				int16 aliasID = miter->second->aliasID();

				std::string	name = miter->second->getName();
				
				const std::vector<DataType*>& args = miter->second->getArgTypes();
				uint8 argssize = args.size();

				bundle << methodUtype << aliasID << name << argssize;
				
				std::vector<DataType*>::const_iterator argiter = args.begin();
				for(; argiter != args.end(); argiter++)
				{
					bundle << (*argiter)->id();
				}
			}

			miter = methods1.begin();
			for(; miter != methods1.end(); miter++)
			{
				ENTITY_METHOD_UID methodUtype = miter->second->getUType();
				int16 aliasID = miter->second->aliasID();

				std::string	name = miter->second->getName();
				
				const std::vector<DataType*>& args = miter->second->getArgTypes();
				uint8 argssize = args.size();

				bundle << methodUtype << aliasID << name << argssize;
				
				std::vector<DataType*>::const_iterator argiter = args.begin();
				for(; argiter != args.end(); argiter++)
				{
					bundle << (*argiter)->id();
				}
			}

			miter = methods2.begin();
			for(; miter != methods2.end(); miter++)
			{
				ENTITY_METHOD_UID methodUtype = miter->second->getUType();
				int16 aliasID = miter->second->aliasID();

				std::string	name = miter->second->getName();
				
				const std::vector<DataType*>& args = miter->second->getArgTypes();
				uint8 argssize = args.size();

				bundle << methodUtype << aliasID << name << argssize;
				
				std::vector<DataType*>::const_iterator argiter = args.begin();
				for(; argiter != args.end(); argiter++)
				{
					bundle << (*argiter)->id();
				}
			}
		}
	}

	bundle.resend(networkInterface(), pChannel);
}

//-------------------------------------------------------------------------------------
PyObject* Baseapp::__py_reloadScript(PyObject* self, PyObject* args)
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

	Baseapp::getSingleton().reloadScript(fullReload);
	S_Return;
}

//-------------------------------------------------------------------------------------
void Baseapp::reloadScript(bool fullReload)
{
	EntityApp<Base>::reloadScript(fullReload);
}

//-------------------------------------------------------------------------------------
void Baseapp::onReloadScript(bool fullReload)
{
	Entities<Base>::ENTITYS_MAP& entities = pEntities_->getEntities();
	Entities<Base>::ENTITYS_MAP::iterator eiter = entities.begin();
	for(; eiter != entities.end(); eiter++)
	{
		static_cast<Base*>(eiter->second.get())->reload(fullReload);
	}

	EntityApp<Base>::onReloadScript(fullReload);
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
	const Mercury::Address& addr = Baseapp::getSingleton().networkInterface().intEndpoint().addr();
	PyTuple_SetItem(pyobj, 0,  PyLong_FromUnsignedLong(addr.ip));
	PyTuple_SetItem(pyobj, 1,  PyLong_FromUnsignedLong(addr.port));
	return pyobj;
}

//-------------------------------------------------------------------------------------
PyObject* Baseapp::__py_deleteBaseByDBID(PyObject* self, PyObject* args)
{
	if(PyTuple_Size(args) != 3)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::deleteBaseByDBID: args != (entityType, dbID, pycallback)!");
		PyErr_PrintEx(0);
		return NULL;
	}
	
	char* entityType = NULL;
	PyObject* pycallback = NULL;
	DBID dbid;

	if(PyArg_ParseTuple(args, "s|K|O", &entityType, &dbid, &pycallback) == -1)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::deleteBaseByDBID: args is error!");
		PyErr_PrintEx(0);
		return NULL;
	}
	
	ScriptDefModule* sm = EntityDef::findScriptModule(entityType);
	if(sm == NULL)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::deleteBaseByDBID: entityType(%s) not found!", entityType);
		PyErr_PrintEx(0);
		return NULL;
	}

	if(dbid == 0)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::deleteBaseByDBID: dbid is 0!");
		PyErr_PrintEx(0);
		return NULL;
	}
	
	if(!PyCallable_Check(pycallback))
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::deleteBaseByDBID: invalid pycallback!");
		PyErr_PrintEx(0);
		return NULL;
	}
	
	Components::COMPONENTS& cts = Components::getSingleton().getComponents(DBMGR_TYPE);
	Components::ComponentInfos* dbmgrinfos = NULL;

	if(cts.size() > 0)
		dbmgrinfos = &(*cts.begin());

	if(dbmgrinfos == NULL || dbmgrinfos->pChannel == NULL || dbmgrinfos->cid == 0)
	{
		ERROR_MSG("KBEngine::deleteBaseByDBID(%1%): not found dbmgr!\n");
		return NULL;
	}

	CALLBACK_ID callbackID = Baseapp::getSingleton().callbackMgr().save(pycallback);

	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
	(*pBundle).newMessage(DbmgrInterface::deleteBaseByDBID);
	(*pBundle) << g_componentID;
	(*pBundle) << dbid;
	(*pBundle) << callbackID;
	(*pBundle) << sm->getUType();
	(*pBundle).send(Baseapp::getSingleton().networkInterface(), dbmgrinfos->pChannel);
	Mercury::Bundle::ObjPool().reclaimObject(pBundle);

	S_Return;
}

//-------------------------------------------------------------------------------------
void Baseapp::deleteBaseByDBIDCB(Mercury::Channel* pChannel, KBEngine::MemoryStream& s)
{
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
		ERROR_MSG(boost::format("Baseapp::deleteBaseByDBIDCB: entityUType(%1%) not found!\n") % sid);
		return;
	}

	if(callbackID > 0)
	{
		SCOPED_PROFILE(SCRIPTCALL_PROFILE);

		// true or false or mailbox
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
				Base* e = static_cast<Base*>(this->findEntity(entityID));
				if(e != NULL)
				{
					pyval = e;
					Py_INCREF(pyval);
				}
				else
				{
					pyval = static_cast<EntityMailbox*>(new EntityMailbox(sm, NULL, entityInAppID, entityID, MAILBOX_TYPE_BASE));
				}
			}
			else
			{
				pyval = Py_False;
				Py_INCREF(pyval);
			}

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
			ERROR_MSG(boost::format("Baseapp::deleteBaseByDBIDCB: can't found callback:%1%.\n") %
				callbackID);
		}
	}
}

//-------------------------------------------------------------------------------------
void Baseapp::reqAccountBindEmail(Mercury::Channel* pChannel, ENTITY_ID entityID, std::string& password, std::string& email)
{
	Base* base = pEntities_->find(entityID);
	if(base == NULL)
	{
		ERROR_MSG(boost::format("Baseapp::reqAccountBindEmail: can't found entity:%1%.\n") % entityID);
		return;
	}
	
	PyObject* py__ACCOUNT_NAME__ = PyObject_GetAttrString(base, "__account_name__");
	if(py__ACCOUNT_NAME__ == NULL)
	{
		DEBUG_MSG(boost::format("Baseapp::reqAccountBindEmail: entity(%1%) __account_name__ is NULL\n") % entityID);
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
		DEBUG_MSG(boost::format("Baseapp::reqAccountBindEmail: entity(%1%) __account_name__ is NULL\n") % entityID);
		return;
	}

	password = KBEngine::strutil::kbe_trim(password);
	email = KBEngine::strutil::kbe_trim(email);

	INFO_MSG(boost::format("Baseapp::reqAccountBindEmail: %1%(%2%) email=%3%!\n") % accountName % entityID % email);

	Components::COMPONENTS& cts = Components::getSingleton().getComponents(DBMGR_TYPE);
	Components::ComponentInfos* dbmgrinfos = NULL;

	if(cts.size() > 0)
		dbmgrinfos = &(*cts.begin());

	if(dbmgrinfos == NULL || dbmgrinfos->pChannel == NULL || dbmgrinfos->cid == 0)
	{
		ERROR_MSG(boost::format("Baseapp::reqAccountBindEmail: accountName(%1%), not found dbmgr!\n") % 
			accountName);

		Mercury::Bundle bundle;
		bundle.newMessage(ClientInterface::onReqAccountBindEmailCB);
		SERVER_ERROR_CODE retcode = SERVER_ERR_SRV_NO_READY;
		bundle << retcode;
		bundle.send(this->networkInterface(), pChannel);
		return;
	}

	Mercury::Bundle bundle;
	bundle.newMessage(DbmgrInterface::accountReqBindMail);
	bundle << entityID << accountName << password << email;
	bundle.send(this->networkInterface(), dbmgrinfos->pChannel);
}

//-------------------------------------------------------------------------------------
void Baseapp::onReqAccountBindEmailCB(Mercury::Channel* pChannel, ENTITY_ID entityID, std::string& accountName, std::string& email,
	SERVER_ERROR_CODE failedcode, std::string& code)
{
	INFO_MSG(boost::format("Baseapp::onReqAccountBindEmailCB: %1%(%2%) failedcode=%3%!\n") % accountName % entityID % failedcode);

	if(failedcode == SERVER_SUCCESS)
	{
		Components::COMPONENTS& loginapps = Components::getSingleton().getComponents(LOGINAPP_TYPE);

		std::string http_host = "localhost";
		Components::COMPONENTS::iterator iter = loginapps.begin();
		for(; iter != loginapps.end(); iter++)
		{
			if((*iter).groupOrderid == 1)
			{
				if(strlen((const char*)&(*iter).externalAddressEx) > 0)
					http_host = (*iter).externalAddressEx;
				else
					http_host = inet_ntoa((struct in_addr&)(*iter).pExtAddr->ip);
			}
		}

		threadPool_.addTask(new SendBindEMailTask(email, code, 
			http_host, 
			g_kbeSrvConfig.getLoginApp().http_cbport));
	}

	Base* base = pEntities_->find(entityID);
	if(base == NULL || base->clientMailbox() == NULL || base->clientMailbox()->getChannel() == NULL)
	{
		ERROR_MSG(boost::format("Baseapp::onReqAccountBindEmailCB: entity:%1%, channel is NULL.\n") % entityID);
		return;
	}

	Mercury::Bundle bundle;
	bundle.newMessage(ClientInterface::onReqAccountBindEmailCB);
	bundle << failedcode;
	bundle.send(this->networkInterface(), base->clientMailbox()->getChannel());
}

//-------------------------------------------------------------------------------------
void Baseapp::reqAccountNewPassword(Mercury::Channel* pChannel, ENTITY_ID entityID, 
									std::string& oldpassworld, std::string& newpassword)
{
	Base* base = pEntities_->find(entityID);
	if(base == NULL)
	{
		ERROR_MSG(boost::format("Baseapp::reqAccountNewPassword: can't found entity:%1%.\n") % entityID);
		return;
	}
	
	PyObject* py__ACCOUNT_NAME__ = PyObject_GetAttrString(base, "__account_name__");
	if(py__ACCOUNT_NAME__ == NULL)
	{
		DEBUG_MSG(boost::format("Baseapp::reqAccountNewPassword: entity(%1%) __account_name__ is NULL\n") % entityID);
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
		DEBUG_MSG(boost::format("Baseapp::reqAccountNewPassword: entity(%1%) __account_name__ is NULL\n") % entityID);
		return;
	}

	oldpassworld = KBEngine::strutil::kbe_trim(oldpassworld);
	newpassword = KBEngine::strutil::kbe_trim(newpassword);

	INFO_MSG(boost::format("Baseapp::reqAccountNewPassword: %1%(%2%)!\n") % 
		accountName % entityID);

	Components::COMPONENTS& cts = Components::getSingleton().getComponents(DBMGR_TYPE);
	Components::ComponentInfos* dbmgrinfos = NULL;

	if(cts.size() > 0)
		dbmgrinfos = &(*cts.begin());

	if(dbmgrinfos == NULL || dbmgrinfos->pChannel == NULL || dbmgrinfos->cid == 0)
	{
		ERROR_MSG(boost::format("Baseapp::reqAccountNewPassword: accountName(%1%), not found dbmgr!\n") % 
			accountName);

		Mercury::Bundle bundle;
		bundle.newMessage(ClientInterface::onReqAccountNewPasswordCB);
		SERVER_ERROR_CODE retcode = SERVER_ERR_SRV_NO_READY;
		bundle << retcode;
		bundle.send(this->networkInterface(), pChannel);
		return;
	}

	Mercury::Bundle bundle;
	bundle.newMessage(DbmgrInterface::accountNewPassword);
	bundle << entityID << accountName << oldpassworld << newpassword;
	bundle.send(this->networkInterface(), dbmgrinfos->pChannel);
}

//-------------------------------------------------------------------------------------
void Baseapp::onReqAccountNewPasswordCB(Mercury::Channel* pChannel, ENTITY_ID entityID, std::string& accountName,
	SERVER_ERROR_CODE failedcode)
{
	INFO_MSG(boost::format("Baseapp::onReqAccountNewPasswordCB: %1%(%2%) failedcode=%3%!\n") % 
		accountName % entityID % failedcode);

	Base* base = pEntities_->find(entityID);
	if(base == NULL || base->clientMailbox() == NULL || base->clientMailbox()->getChannel() == NULL)
	{
		ERROR_MSG(boost::format("Baseapp::onReqAccountNewPasswordCB: entity:%1%, channel is NULL.\n") % entityID);
		return;
	}

	Mercury::Bundle bundle;
	bundle.newMessage(ClientInterface::onReqAccountBindEmailCB);
	bundle << failedcode;
	bundle.send(this->networkInterface(), base->clientMailbox()->getChannel());
}

//-------------------------------------------------------------------------------------
void Baseapp::onVersionNotMatch(Mercury::Channel* pChannel)
{
	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
	
	pBundle->newMessage(ClientInterface::onVersionNotMatch);
	(*pBundle) << KBEVersion::versionString();
	(*pBundle).send(networkInterface(), pChannel);

	Mercury::Bundle::ObjPool().reclaimObject(pBundle);
}

//-------------------------------------------------------------------------------------
void Baseapp::onScriptVersionNotMatch(Mercury::Channel* pChannel)
{
	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
	
	pBundle->newMessage(ClientInterface::onScriptVersionNotMatch);
	(*pBundle) << KBEVersion::scriptVersionString();
	(*pBundle).send(networkInterface(), pChannel);

	Mercury::Bundle::ObjPool().reclaimObject(pBundle);
}

//-------------------------------------------------------------------------------------

}
