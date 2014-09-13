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

#ifndef KBE_ENTITY_APP_HPP
#define KBE_ENTITY_APP_HPP

// common include
#include "pyscript/script.hpp"
#include "pyscript/pyprofile.hpp"
#include "pyscript/pyprofile_handler.hpp"
#include "cstdkbe/cstdkbe.hpp"
#include "cstdkbe/timer.hpp"
#include "cstdkbe/smartpointer.hpp"
#include "pyscript/pyobject_pointer.hpp"
#include "pyscript/pywatcher.hpp"
#include "helper/debug_helper.hpp"
#include "helper/script_loglevel.hpp"
#include "helper/profile.hpp"
#include "server/script_timers.hpp"
#include "server/idallocate.hpp"
#include "server/serverconfig.hpp"
#include "server/globaldata_client.hpp"
#include "server/globaldata_server.hpp"
#include "server/callbackmgr.hpp"	
#include "entitydef/entitydef.hpp"
#include "entitydef/entities.hpp"
#include "entitydef/entity_mailbox.hpp"
#include "entitydef/scriptdef_module.hpp"
#include "network/message_handler.hpp"
#include "resmgr/resmgr.hpp"
#include "helper/console_helper.hpp"

#if KBE_PLATFORM == PLATFORM_WIN32
#pragma warning (disable : 4996)
#endif
//#define NDEBUG
#include "server/serverapp.hpp"
// windows include	
#if KBE_PLATFORM == PLATFORM_WIN32
#else
// linux include
#endif
	
namespace KBEngine{

template<class E>
class EntityApp : public ServerApp
{
public:
	enum TimeOutType
	{
		TIMEOUT_GAME_TICK = TIMEOUT_SERVERAPP_MAX + 1,
		TIMEOUT_ENTITYAPP_MAX
	};
public:
	EntityApp(Mercury::EventDispatcher& dispatcher, 
		Mercury::NetworkInterface& ninterface, 
		COMPONENT_TYPE componentType,
		COMPONENT_ID componentID);

	~EntityApp();
	
	/** 
		相关处理接口 
	*/
	virtual void handleTimeout(TimerHandle handle, void * arg);
	virtual void handleGameTick();

	/**
		通过entityID寻找到对应的实例 
	*/
	E* findEntity(ENTITY_ID entityID);

	/** 
		通过entityID销毁一个entity 
	*/
	virtual bool destroyEntity(ENTITY_ID entityID, bool callScript);

	/**
		由mailbox来尝试获取一个entity的实例
		因为这个组件上不一定存在这个entity。
	*/
	PyObject* tryGetEntityByMailbox(COMPONENT_ID componentID, ENTITY_ID eid);

	/**
		由mailbox来尝试获取一个channel的实例
	*/
	Mercury::Channel* findChannelByMailbox(EntityMailbox& mailbox);

	KBEngine::script::Script& getScript(){ return script_; }
	PyObjectPtr getEntryScript(){ return entryScript_; }

	void registerScript(PyTypeObject*);
	int registerPyObjectToScript(const char* attrName, PyObject* pyObj);
	int unregisterPyObjectToScript(const char* attrName);

	bool installPyScript();
	virtual bool installPyModules();
	virtual void onInstallPyModules() {};
	virtual bool uninstallPyModules();
	bool uninstallPyScript();
	bool installEntityDef();
	
	virtual bool initializeWatcher();

	virtual void finalise();
	virtual bool inInitialize();
	virtual bool initialize();

	virtual void onSignalled(int sigNum);
	
	Entities<E>* pEntities()const{ return pEntities_; }
	ArraySize entitiesSize()const { return pEntities_->size(); }

	PY_CALLBACKMGR& callbackMgr(){ return pyCallbackMgr_; }	

	/**
		创建一个entity 
	*/
	E* createEntityCommon(const char* entityType, PyObject* params,
		bool isInitializeScript = true, ENTITY_ID eid = 0, bool initProperty = true);

	virtual E* onCreateEntityCommon(PyObject* pyEntity, ScriptDefModule* sm, ENTITY_ID eid);

	/** 网络接口
		请求分配一个ENTITY_ID段的回调
	*/
	void onReqAllocEntityID(Mercury::Channel* pChannel, ENTITY_ID startID, ENTITY_ID endID);

	/** 网络接口
		dbmgr发送初始信息
		startID: 初始分配ENTITY_ID 段起始位置
		endID: 初始分配ENTITY_ID 段结束位置
		startGlobalOrder: 全局启动顺序 包括各种不同组件
		startGroupOrder: 组内启动顺序， 比如在所有baseapp中第几个启动。
	*/
	void onDbmgrInitCompleted(Mercury::Channel* pChannel, 
		GAME_TIME gametime, ENTITY_ID startID, ENTITY_ID endID, int32 startGlobalOrder, int32 startGroupOrder, const std::string& digest);

	/** 网络接口
		dbmgr广播global数据的改变
	*/
	void onBroadcastGlobalDataChanged(Mercury::Channel* pChannel, KBEngine::MemoryStream& s);


	/** 网络接口
		请求执行一段python指令
	*/
	void onExecScriptCommand(Mercury::Channel* pChannel, KBEngine::MemoryStream& s);

	/** 
		console请求开始profile
	*/
	virtual void startProfile_(Mercury::Channel* pChannel, std::string profileName, int8 profileType, uint32 timelen);

	/**
		获取apps发布状态, 可在脚本中获取该值
	*/
	static PyObject* __py_getAppPublish(PyObject* self, PyObject* args);

	/**
		设置脚本输出类型前缀
	*/
	static PyObject* __py_setScriptLogType(PyObject* self, PyObject* args);

	/**
		获取watcher值
	*/
	static PyObject* __py_getWatcher(PyObject* self, PyObject* args);
	static PyObject* __py_getWatcherDir(PyObject* self, PyObject* args);

	/**
		重新导入所有的脚本
	*/
	virtual void reloadScript(bool fullReload);
	virtual void onReloadScript(bool fullReload);

	/**
		通过相对路径获取资源的全路径
	*/
	static PyObject* __py_getResFullPath(PyObject* self, PyObject* args);

	/**
		通过相对路径判断资源是否存在
	*/
	static PyObject* __py_hasRes(PyObject* self, PyObject* args);

	/**
		open文件
	*/
	static PyObject* __py_kbeOpen(PyObject* self, PyObject* args);

	/**
		列出目录下所有文件
	*/
	static PyObject* __py_listPathRes(PyObject* self, PyObject* args);

	/**
		匹配相对路径获得全路径 
	*/
	static PyObject* __py_matchPath(PyObject* self, PyObject* args);
protected:
	KBEngine::script::Script								script_;
	std::vector<PyTypeObject*>								scriptBaseTypes_;

	PyObjectPtr												entryScript_;

	EntityIDClient											idClient_;

	// 存储所有的entity的容器
	Entities<E>*											pEntities_;										

	TimerHandle												gameTimer_;

	// globalData
	GlobalDataClient*										pGlobalData_;									

	PY_CALLBACKMGR											pyCallbackMgr_;
};


template<class E>
EntityApp<E>::EntityApp(Mercury::EventDispatcher& dispatcher, 
					 Mercury::NetworkInterface& ninterface, 
					 COMPONENT_TYPE componentType,
					 COMPONENT_ID componentID):
ServerApp(dispatcher, ninterface, componentType, componentID),
script_(),
scriptBaseTypes_(),
entryScript_(),
idClient_(),
pEntities_(NULL),
gameTimer_(),
pGlobalData_(NULL),
pyCallbackMgr_()
{
	ScriptTimers::initialize(*this);
	idClient_.pApp(this);

	// 初始化mailbox模块获取entity实体函数地址
	EntityMailbox::setGetEntityFunc(std::tr1::bind(&EntityApp<E>::tryGetEntityByMailbox, this, 
		std::tr1::placeholders::_1, std::tr1::placeholders::_2));

	// 初始化mailbox模块获取channel函数地址
	EntityMailbox::setFindChannelFunc(std::tr1::bind(&EntityApp<E>::findChannelByMailbox, this, 
		std::tr1::placeholders::_1));
}

template<class E>
EntityApp<E>::~EntityApp()
{
}

template<class E>
bool EntityApp<E>::inInitialize()
{
	if(!installPyScript())
		return false;

	if(!installPyModules())
		return false;
	
	return installEntityDef();
}

template<class E>
bool EntityApp<E>::initialize()
{
	bool ret = ServerApp::initialize();
	if(ret)
	{
		gameTimer_ = this->mainDispatcher().addTimer(1000000 / g_kbeSrvConfig.gameUpdateHertz(), this,
								reinterpret_cast<void *>(TIMEOUT_GAME_TICK));
	}
	return ret;
}

template<class E>
bool EntityApp<E>::initializeWatcher()
{
	WATCH_OBJECT("entitiesSize", this, &EntityApp<E>::entitiesSize);
	return ServerApp::initializeWatcher();
}

template<class E>
void EntityApp<E>::finalise(void)
{
	gameTimer_.cancel();

	WATCH_FINALIZE;
	
	pyCallbackMgr_.finalise();
	ScriptTimers::finalise(*this);

	if(pEntities_)
		pEntities_->finalise();
	
	uninstallPyScript();

	ServerApp::finalise();
}

template<class E>
bool EntityApp<E>::installEntityDef()
{
	EntityDef::entityAliasID(ServerConfig::getSingleton().getCellApp().aliasEntityID);
	EntityDef::entitydefAliasID(ServerConfig::getSingleton().getCellApp().entitydefAliasID);
	
	if(!EntityDef::installScript(this->getScript().getModule()))
		return false;

	// 初始化所有扩展模块
	// demo/res/scripts/
	if(!EntityDef::initialize(scriptBaseTypes_, componentType_)){
		return false;
	}

	return true;
}

template<class E>
int EntityApp<E>::registerPyObjectToScript(const char* attrName, PyObject* pyObj)
{ 
	return script_.registerToModule(attrName, pyObj); 
}

template<class E>
int EntityApp<E>::unregisterPyObjectToScript(const char* attrName)
{ 
	return script_.unregisterToModule(attrName); 
}

template<class E>
bool EntityApp<E>::installPyScript()
{
	if(Resmgr::getSingleton().respaths().size() <= 0 || 
		Resmgr::getSingleton().getPyUserResPath().size() == 0 || 
		Resmgr::getSingleton().getPySysResPath().size() == 0)
	{
		KBE_ASSERT(false && "EntityApp::installPyScript: KBE_RES_PATH is error!\n");
		return false;
	}

	std::wstring user_res_path = L"";
	wchar_t* tbuf = KBEngine::strutil::char2wchar(const_cast<char*>(Resmgr::getSingleton().getPyUserResPath().c_str()));
	if(tbuf != NULL)
	{
		user_res_path += tbuf;
		free(tbuf);
	}
	else
	{
		KBE_ASSERT(false && "EntityApp::installPyScript: KBE_RES_PATH error[char2wchar]!\n");
		return false;
	}

	std::wstring pyPaths = user_res_path + L"scripts/common;";
	pyPaths += user_res_path + L"scripts/data;";
	pyPaths += user_res_path + L"scripts/user_type;";

	switch(componentType_)
	{
	case BASEAPP_TYPE:
		pyPaths += user_res_path + L"scripts/server_common;";
		pyPaths += user_res_path + L"scripts/base;";
		break;
	case CELLAPP_TYPE:
		pyPaths += user_res_path + L"scripts/server_common;";
		pyPaths += user_res_path + L"scripts/cell;";
		break;
	case DBMGR_TYPE:
		pyPaths += user_res_path + L"scripts/server_common;";
		pyPaths += user_res_path + L"scripts/db;";
		break;
	default:
		pyPaths += user_res_path + L"scripts/client;";
		break;
	};
	
	std::string kbe_res_path = Resmgr::getSingleton().getPySysResPath();
	kbe_res_path += "scripts/common";

	tbuf = KBEngine::strutil::char2wchar(const_cast<char*>(kbe_res_path.c_str()));
	bool ret = getScript().install(tbuf, pyPaths, "KBEngine", componentType_);
	free(tbuf);
	return ret;
}

template<class E>
void EntityApp<E>::registerScript(PyTypeObject* pto)
{
	scriptBaseTypes_.push_back(pto);
}

template<class E>
bool EntityApp<E>::uninstallPyScript()
{
	return uninstallPyModules() && getScript().uninstall();
}

template<class E>
bool EntityApp<E>::installPyModules()
{
	Entities<E>::installScript(NULL);
	//Entity::installScript(g_script.getModule());

	pEntities_ = new Entities<E>();
	registerPyObjectToScript("entities", pEntities_);

	// 安装入口模块
	PyObject *entryScriptFileName = NULL;
	if(componentType() == BASEAPP_TYPE)
	{
		ENGINE_COMPONENT_INFO& info = g_kbeSrvConfig.getBaseApp();
		entryScriptFileName = PyUnicode_FromString(info.entryScriptFile);
	}
	else if(componentType() == CELLAPP_TYPE)
	{
		ENGINE_COMPONENT_INFO& info = g_kbeSrvConfig.getCellApp();
		entryScriptFileName = PyUnicode_FromString(info.entryScriptFile);
	}

	// 添加pywatcher支持
	if(!initializePyWatcher(&this->getScript()))
		return false;

	// 添加globalData, globalBases支持
	pGlobalData_ = new GlobalDataClient(DBMGR_TYPE, GlobalDataServer::GLOBAL_DATA);
	registerPyObjectToScript("globalData", pGlobalData_);

	// 注册创建entity的方法到py
	// 向脚本注册app发布状态
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(),	publish,			__py_getAppPublish,		METH_VARARGS,	0);

	// 注册设置脚本输出类型
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(),	scriptLogType,		__py_setScriptLogType,	METH_VARARGS,	0);
	
	// 获得资源全路径
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(),	getResFullPath,		__py_getResFullPath,	METH_VARARGS,	0);

	// 是否存在某个资源
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(),	hasRes,				__py_hasRes,			METH_VARARGS,	0);

	// 打开一个文件
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(),	open,				__py_kbeOpen,			METH_VARARGS,	0);

	// 列出目录下所有文件
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(),	listPathRes,		__py_listPathRes,		METH_VARARGS,	0);

	// 匹配相对路径获得全路径
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(),	matchPath,			__py_matchPath,			METH_VARARGS,	0);

	// 获取watcher值
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(),	getWatcher,			__py_getWatcher,		METH_VARARGS,	0);

	// 获取watcher目录
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(),	getWatcherDir,		__py_getWatcherDir,		METH_VARARGS,	0);

	if(PyModule_AddIntConstant(this->getScript().getModule(), "LOG_TYPE_NORMAL", log4cxx::ScriptLevel::SCRIPT_INT))
	{
		ERROR_MSG( "EntityApp::installPyModules: Unable to set KBEngine.LOG_TYPE_NORMAL.\n");
	}

	if(PyModule_AddIntConstant(this->getScript().getModule(), "LOG_TYPE_INFO", log4cxx::ScriptLevel::SCRIPT_INFO))
	{
		ERROR_MSG( "EntityApp::installPyModules: Unable to set KBEngine.LOG_TYPE_INFO.\n");
	}

	if(PyModule_AddIntConstant(this->getScript().getModule(), "LOG_TYPE_ERR", log4cxx::ScriptLevel::SCRIPT_ERR))
	{
		ERROR_MSG( "EntityApp::installPyModules: Unable to set KBEngine.LOG_TYPE_ERR.\n");
	}

	if(PyModule_AddIntConstant(this->getScript().getModule(), "LOG_TYPE_DBG", log4cxx::ScriptLevel::SCRIPT_DBG))
	{
		ERROR_MSG( "EntityApp::installPyModules: Unable to set KBEngine.LOG_TYPE_DBG.\n");
	}

	if(PyModule_AddIntConstant(this->getScript().getModule(), "LOG_TYPE_WAR", log4cxx::ScriptLevel::SCRIPT_WAR))
	{
		ERROR_MSG( "EntityApp::installPyModules: Unable to set KBEngine.LOG_TYPE_WAR.\n");
	}


	if(PyModule_AddIntConstant(this->getScript().getModule(), "NEXT_ONLY", KBE_NEXT_ONLY))
	{
		ERROR_MSG( "EntityApp::installPyModules: Unable to set KBEngine.NEXT_ONLY.\n");
	}
	
	if(entryScriptFileName != NULL)
	{
		entryScript_ = PyImport_Import(entryScriptFileName);
		SCRIPT_ERROR_CHECK();
		S_RELEASE(entryScriptFileName);

		if(entryScript_.get() == NULL)
		{
			return false;
		}
	}

	onInstallPyModules();
	return true;
}

template<class E>
bool EntityApp<E>::uninstallPyModules()
{
	S_RELEASE(pEntities_);
	unregisterPyObjectToScript("entities");

	unregisterPyObjectToScript("globalData");
	S_RELEASE(pGlobalData_); 

	Entities<E>::uninstallScript();
	//Entity::uninstallScript();
	EntityDef::uninstallScript();
	return true;
}

template<class E>
E* EntityApp<E>::createEntityCommon(const char* entityType, PyObject* params,
										 bool isInitializeScript, ENTITY_ID eid, bool initProperty)
{
	// 检查ID是否足够, 不足返回NULL
	if(eid <= 0 && idClient_.getSize() == 0)
	{
		PyErr_SetString(PyExc_SystemError, "EntityApp::createEntityCommon: is Failed. not enough entityIDs.");
		PyErr_PrintEx(0);
		return NULL;
	}
	
	ScriptDefModule* sm = EntityDef::findScriptModule(entityType);
	if(sm == NULL)
	{
		PyErr_Format(PyExc_TypeError, "EntityApp::createEntityCommon: entity [%s] not found.\n", entityType);
		PyErr_PrintEx(0);
		return NULL;
	}
	else if(componentType_ == CELLAPP_TYPE ? !sm->hasCell() : !sm->hasBase())
	{
		PyErr_Format(PyExc_TypeError, "EntityApp::createEntityCommon: entity [%s] not found.\n", entityType);
		PyErr_PrintEx(0);
		return NULL;
	}

	PyObject* obj = sm->createObject();

	// 判断是否要分配一个新的id
	ENTITY_ID id = eid;
	if(id <= 0)
		id = idClient_.alloc();
	
	E* entity = onCreateEntityCommon(obj, sm, id);

	if(initProperty)
		entity->initProperty();

	// 将entity加入entities
	pEntities_->add(id, entity); 

	// 初始化脚本
	if(isInitializeScript)
		entity->initializeEntity(params);

	SCRIPT_ERROR_CHECK();

	if(g_debugEntity)
	{
		INFO_MSG(boost::format("EntityApp::createEntityCommon: new %1% (%2%) refc=%3%.\n") % entityType % id % obj->ob_refcnt);
	}
	else
	{
		INFO_MSG(fmt::format("EntityApp::createEntityCommon: new {0} {1}\n", entityType, id));
	}

	return entity;
}

template<class E>
E* EntityApp<E>::onCreateEntityCommon(PyObject* pyEntity, ScriptDefModule* sm, ENTITY_ID eid)
{
	// 执行Entity的构造函数
	return new(pyEntity) E(eid, sm);
}

template<class E>
void EntityApp<E>::onSignalled(int sigNum)
{
	this->ServerApp::onSignalled(sigNum);
	
	switch (sigNum)
	{
	case SIGQUIT:
		CRITICAL_MSG(boost::format("Received QUIT signal. This is likely caused by the "
					"%1%Mgr killing this %2% because it has been "
					"unresponsive for too long. Look at the callstack from "
					"the core dump to find the likely cause.\n") %
				COMPONENT_NAME_EX(componentType_) % 
				COMPONENT_NAME_EX(componentType_) );
		
		break;
	default: 
		break;
	}
}

template<class E>
PyObject* EntityApp<E>::tryGetEntityByMailbox(COMPONENT_ID componentID, ENTITY_ID eid)
{
	if(componentID != componentID_)
		return NULL;
	
	E* entity = pEntities_->find(eid);
	if(entity == NULL){
		ERROR_MSG(boost::format("EntityApp::tryGetEntityByMailbox: can't found entity:%1%.\n") % eid);
		return NULL;
	}

	return entity;
}

template<class E>
Mercury::Channel* EntityApp<E>::findChannelByMailbox(EntityMailbox& mailbox)
{
	// 如果组件ID大于0则查找组件
	if(mailbox.componentID() > 0)
	{
		Components::ComponentInfos* cinfos = 
			Components::getSingleton().findComponent(mailbox.componentID());

		if(cinfos != NULL && cinfos->pChannel != NULL)
			return cinfos->pChannel; 
	}
	else
	{
		return Components::getSingleton().pNetworkInterface()->findChannel(mailbox.addr());
	}

	return NULL;
}

template<class E>
void EntityApp<E>::handleTimeout(TimerHandle handle, void * arg)
{
	switch (reinterpret_cast<uintptr>(arg))
	{
		case TIMEOUT_GAME_TICK:
			this->handleGameTick();
			break;
		default:
			break;
	}

	ServerApp::handleTimeout(handle, arg);
}

template<class E>
void EntityApp<E>::handleGameTick()
{
	// time_t t = ::time(NULL);
	// DEBUG_MSG("EntityApp::handleGameTick[%"PRTime"]:%u\n", t, time_);

	g_kbetime++;
	threadPool_.onMainThreadTick();
	handleTimers();
	networkInterface().processAllChannelPackets(KBEngine::Mercury::MessageHandlers::pMainMessageHandlers);
}

template<class E>
bool EntityApp<E>::destroyEntity(ENTITY_ID entityID, bool callScript)
{
	PyObjectPtr entity = pEntities_->erase(entityID);
	if(entity != NULL)
	{
		static_cast<E*>(entity.get())->destroy(callScript);
		return true;
	}

	ERROR_MSG(boost::format("EntityApp::destroyEntity: not found %1%!\n") % entityID);
	return false;
}

template<class E>
E* EntityApp<E>::findEntity(ENTITY_ID entityID)
{
	AUTO_SCOPED_PROFILE("findEntity");
	return pEntities_->find(entityID);
}

template<class E>
void EntityApp<E>::onReqAllocEntityID(Mercury::Channel* pChannel, ENTITY_ID startID, ENTITY_ID endID)
{
	// INFO_MSG("EntityApp::onReqAllocEntityID: entityID alloc(%d-%d).\n", startID, endID);
	idClient_.onAddRange(startID, endID);
}

template<class E>
PyObject* EntityApp<E>::__py_getAppPublish(PyObject* self, PyObject* args)
{
	return PyLong_FromLong(g_appPublish);
}

template<class E>
PyObject* EntityApp<E>::__py_getWatcher(PyObject* self, PyObject* args)
{
	int argCount = PyTuple_Size(args);
	if(argCount != 1)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::getWatcher(): args[strpath] is error!");
		PyErr_PrintEx(0);
		return 0;
	}
	
	char* path;

	if(PyArg_ParseTuple(args, "s", &path) == -1)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::getWatcher(): args[strpath] is error!");
		PyErr_PrintEx(0);
		return 0;
	}

	//DebugHelper::getSingleton().setScriptMsgType(type);

	KBEShared_ptr< WatcherObject > pWobj = WatcherPaths::root().getWatcher(path);
	if(pWobj.get() == NULL)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::getWatcher(): not found watcher[%s]!", path);
		PyErr_PrintEx(0);
		return 0;
	}

	WATCHER_VALUE_TYPE wtype = pWobj->getType();
	PyObject* pyval = NULL;
	MemoryStream* stream = MemoryStream::ObjPool().createObject();
	pWobj->addToStream(stream);
	WATCHER_ID id;
	(*stream) >> id;

	switch(wtype)
	{
	case WATCHER_VALUE_TYPE_UINT8:
		{
			uint8 v;
			(*stream) >> v;
			pyval = PyLong_FromUnsignedLong(v);
		}
		break;
	case WATCHER_VALUE_TYPE_UINT16:
		{
			uint16 v;
			(*stream) >> v;
			pyval = PyLong_FromUnsignedLong(v);
		}
		break;
	case WATCHER_VALUE_TYPE_UINT32:
		{
			uint32 v;
			(*stream) >> v;
			pyval = PyLong_FromUnsignedLong(v);
		}
		break;
	case WATCHER_VALUE_TYPE_UINT64:
		{
			uint64 v;
			(*stream) >> v;
			pyval = PyLong_FromUnsignedLongLong(v);
		}
		break;
	case WATCHER_VALUE_TYPE_INT8:
		{
			int8 v;
			(*stream) >> v;
			pyval = PyLong_FromLong(v);
		}
		break;
	case WATCHER_VALUE_TYPE_INT16:
		{
			int16 v;
			(*stream) >> v;
			pyval = PyLong_FromLong(v);
		}
		break;
	case WATCHER_VALUE_TYPE_INT32:
		{
			int32 v;
			(*stream) >> v;
			pyval = PyLong_FromLong(v);
		}
		break;
	case WATCHER_VALUE_TYPE_INT64:
		{
			int64 v;
			(*stream) >> v;
			pyval = PyLong_FromLongLong(v);
		}
		break;
	case WATCHER_VALUE_TYPE_FLOAT:
		{
			float v;
			(*stream) >> v;
			pyval = PyLong_FromDouble(v);
		}
		break;
	case WATCHER_VALUE_TYPE_DOUBLE:
		{
			double v;
			(*stream) >> v;
			pyval = PyLong_FromDouble(v);
		}
		break;
	case WATCHER_VALUE_TYPE_CHAR:
	case WATCHER_VALUE_TYPE_STRING:
		{
			std::string v;
			(*stream) >> v;
			pyval = PyUnicode_FromString(v.c_str());
		}
		break;
	case WATCHER_VALUE_TYPE_BOOL:
		{
			bool v;
			(*stream) >> v;
			pyval = PyBool_FromLong(v);
		}
		break;
	case WATCHER_VALUE_TYPE_COMPONENT_TYPE:
		{
			COMPONENT_TYPE v;
			(*stream) >> v;
			pyval = PyBool_FromLong(v);
		}
		break;
	default:
		KBE_ASSERT(false && "no support!\n");
	};

	MemoryStream::ObjPool().reclaimObject(stream);
	return pyval;
}

template<class E>
PyObject* EntityApp<E>::__py_getWatcherDir(PyObject* self, PyObject* args)
{
	int argCount = PyTuple_Size(args);
	if(argCount != 1)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::getWatcherDir(): args[strpath] is error!");
		PyErr_PrintEx(0);
		return 0;
	}
	
	char* path;

	if(PyArg_ParseTuple(args, "s", &path) == -1)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::getWatcherDir(): args[strpath] is error!");
		PyErr_PrintEx(0);
		return 0;
	}

	std::vector<std::string> vec;
	WatcherPaths::root().dirPath(path, vec);

	PyObject* pyTuple = PyTuple_New(vec.size());
	std::vector<std::string>::iterator iter = vec.begin();
	int i = 0;
	for(; iter != vec.end(); iter++)
	{
		PyTuple_SET_ITEM(pyTuple, i++, PyUnicode_FromString((*iter).c_str()));
	}

	return pyTuple;
}

template<class E>
PyObject* EntityApp<E>::__py_setScriptLogType(PyObject* self, PyObject* args)
{
	int argCount = PyTuple_Size(args);
	if(argCount != 1)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::scriptLogType(): args is error!");
		PyErr_PrintEx(0);
		return 0;
	}

	int type = -1;

	if(PyArg_ParseTuple(args, "i", &type) == -1)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::scriptLogType(): args is error!");
		PyErr_PrintEx(0);
		return 0;
	}

	DebugHelper::getSingleton().setScriptMsgType(type);
	S_Return;
}

template<class E>
PyObject* EntityApp<E>::__py_getResFullPath(PyObject* self, PyObject* args)
{
	int argCount = PyTuple_Size(args);
	if(argCount != 1)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::getResFullPath(): args is error!");
		PyErr_PrintEx(0);
		return 0;
	}

	char* respath = NULL;

	if(PyArg_ParseTuple(args, "s", &respath) == -1)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::getResFullPath(): args is error!");
		PyErr_PrintEx(0);
		return 0;
	}

	std::string fullpath = Resmgr::getSingleton().matchRes(respath);
	return PyUnicode_FromString(fullpath.c_str());
}

template<class E>
PyObject* EntityApp<E>::__py_hasRes(PyObject* self, PyObject* args)
{
	int argCount = PyTuple_Size(args);
	if(argCount != 1)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::hasRes(): args is error!");
		PyErr_PrintEx(0);
		return 0;
	}

	char* respath = NULL;

	if(PyArg_ParseTuple(args, "s", &respath) == -1)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::hasRes(): args is error!");
		PyErr_PrintEx(0);
		return 0;
	}

	return PyBool_FromLong(Resmgr::getSingleton().hasRes(respath));
}

template<class E>
PyObject* EntityApp<E>::__py_kbeOpen(PyObject* self, PyObject* args)
{
	int argCount = PyTuple_Size(args);
	if(argCount != 2)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::open(): args is error!");
		PyErr_PrintEx(0);
		return 0;
	}

	char* respath = NULL;
	char* fargs = NULL;

	if(PyArg_ParseTuple(args, "s|s", &respath, &fargs) == -1)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::open(): args is error!");
		PyErr_PrintEx(0);
		return 0;
	}

	std::string sfullpath = Resmgr::getSingleton().matchRes(respath);

	PyObject *ioMod = PyImport_ImportModule("io");

	PyObject *openedFile = PyObject_CallMethod(ioMod, const_cast<char*>("open"), 
		const_cast<char*>("ss"), 
		const_cast<char*>(sfullpath.c_str()), 
		fargs);

	Py_DECREF(ioMod);
	return openedFile;
}

template<class E>
PyObject* EntityApp<E>::__py_matchPath(PyObject* self, PyObject* args)
{
	int argCount = PyTuple_Size(args);
	if(argCount != 1)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::matchPath(): args is error!");
		PyErr_PrintEx(0);
		return 0;
	}

	char* respath = NULL;

	if(PyArg_ParseTuple(args, "s", &respath) == -1)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::matchPath(): args is error!");
		PyErr_PrintEx(0);
		return 0;
	}

	std::string path = Resmgr::getSingleton().matchPath(respath);
	return PyUnicode_FromStringAndSize(path.c_str(), path.size());
}

template<class E>
PyObject* EntityApp<E>::__py_listPathRes(PyObject* self, PyObject* args)
{
	int argCount = PyTuple_Size(args);
	if(argCount < 1 || argCount > 2)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::listPathRes(): args[path, pathargs=\'*.*\'] is error!");
		PyErr_PrintEx(0);
		return 0;
	}

	std::wstring wExtendName = L"*";
	PyObject* pathobj = NULL;
	PyObject* path_argsobj = NULL;

	if(argCount == 1)
	{
		if(PyArg_ParseTuple(args, "O", &pathobj) == -1)
		{
			PyErr_Format(PyExc_TypeError, "KBEngine::listPathRes(): args[path] is error!");
			PyErr_PrintEx(0);
			return 0;
		}
	}
	else
	{
		if(PyArg_ParseTuple(args, "O|O", &pathobj, &path_argsobj) == -1)
		{
			PyErr_Format(PyExc_TypeError, "KBEngine::listPathRes(): args[path, pathargs=\'*.*\'] is error!");
			PyErr_PrintEx(0);
			return 0;
		}
		
		if(PyUnicode_Check(path_argsobj))
		{
			wchar_t* fargs = NULL;
			fargs = PyUnicode_AsWideCharString(path_argsobj, NULL);
			wExtendName = fargs;
			PyMem_Free(fargs);
		}
		else
		{
			if(PySequence_Check(path_argsobj))
			{
				wExtendName = L"";
				Py_ssize_t size = PySequence_Size(path_argsobj);
				for(int i=0; i<size; i++)
				{
					PyObject* pyobj = PySequence_GetItem(path_argsobj, i);
					if(!PyUnicode_Check(pyobj))
					{
						PyErr_Format(PyExc_TypeError, "KBEngine::listPathRes(): args[path, pathargs=\'*.*\'] is error!");
						PyErr_PrintEx(0);
						return 0;
					}
					
					wchar_t* wtemp = NULL;
					wtemp = PyUnicode_AsWideCharString(pyobj, NULL);
					wExtendName += wtemp;
					wExtendName += L"|";
					PyMem_Free(wtemp);
				}
			}
			else
			{
				PyErr_Format(PyExc_TypeError, "KBEngine::listPathRes(): args[pathargs] is error!");
				PyErr_PrintEx(0);
				return 0;
			}
		}
	}

	if(!PyUnicode_Check(pathobj))
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::listPathRes(): args[path] is error!");
		PyErr_PrintEx(0);
		return 0;
	}

	if(wExtendName.size() == 0)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::listPathRes(): args[pathargs] is NULL!");
		PyErr_PrintEx(0);
		return 0;
	}

	if(wExtendName[0] == '.')
		wExtendName.erase(wExtendName.begin());

	if(wExtendName.size() == 0)
		wExtendName = L"*";

	wchar_t* respath = PyUnicode_AsWideCharString(pathobj, NULL);
	if(respath == NULL)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::listPathRes(): args[path] is NULL!");
		PyErr_PrintEx(0);
		return 0;
	}

	char* cpath = strutil::wchar2char(respath);
	std::string foundPath = Resmgr::getSingleton().matchPath(cpath);
	free(cpath);
	PyMem_Free(respath);

	respath = strutil::char2wchar(foundPath.c_str());

	std::vector<std::wstring> results;
	Resmgr::getSingleton().listPathRes(respath, wExtendName, results);
	PyObject* pyresults = PyTuple_New(results.size());

	std::vector<std::wstring>::iterator iter = results.begin();
	int i = 0;

	for(; iter != results.end(); iter++)
	{
		PyTuple_SET_ITEM(pyresults, i++, PyUnicode_FromWideChar((*iter).c_str(), (*iter).size()));
	}

	free(respath);
	return pyresults;
}

template<class E>
void EntityApp<E>::startProfile_(Mercury::Channel* pChannel, std::string profileName, int8 profileType, uint32 timelen)
{
	switch(profileType)
	{
	case 0:	// pyprofile
		new PyProfileHandler(this->networkInterface(), timelen, profileName, pChannel->addr());
		return;
	default:
		break;
	};

	ServerApp::startProfile_(pChannel, profileName, profileType, timelen);
}

template<class E>
void EntityApp<E>::onDbmgrInitCompleted(Mercury::Channel* pChannel, 
						GAME_TIME gametime, ENTITY_ID startID, ENTITY_ID endID, int32 startGlobalOrder, int32 startGroupOrder, const std::string& digest)
{
	INFO_MSG(boost::format("EntityApp::onDbmgrInitCompleted: entityID alloc(%1%-%2%), startGlobalOrder=%3%, startGroupOrder=%4%, digest=%5%.\n") %
		startID % endID % startGlobalOrder % startGroupOrder % digest);

	startGlobalOrder_ = startGlobalOrder;
	startGroupOrder_ = startGroupOrder;
	g_componentGlobalOrder = startGlobalOrder;
	g_componentGroupOrder = startGroupOrder;

	idClient_.onAddRange(startID, endID);
	g_kbetime = gametime;

	if(digest != EntityDef::md5().getDigestStr())
	{
		ERROR_MSG(boost::format("EntityApp::onDbmgrInitCompleted: digest not match. curr(%1%) != dbmgr(%2%)\n") %
			EntityDef::md5().getDigestStr() % digest);

		this->shutDown();
	}
}

template<class E>
void EntityApp<E>::onBroadcastGlobalDataChanged(Mercury::Channel* pChannel, KBEngine::MemoryStream& s)
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
		ERROR_MSG("EntityApp::onBroadcastGlobalDataChanged: no has key!\n");
		return;
	}

	Py_INCREF(pyKey);

	if(isDelete)
	{
		if(pGlobalData_->del(pyKey))
		{
			// 通知脚本
			SCRIPT_OBJECT_CALL_ARGS1(getEntryScript().get(), const_cast<char*>("onGlobalDataDel"), 
				const_cast<char*>("O"), pyKey);
		}
	}
	else
	{
		PyObject * pyValue = script::Pickler::unpickle(value);
		if(pyValue == NULL)
		{
			ERROR_MSG("EntityApp::onBroadcastGlobalDataChanged: no has value!\n");
			Py_DECREF(pyKey);
			return;
		}

		Py_INCREF(pyValue);

		if(pGlobalData_->write(pyKey, pyValue))
		{
			// 通知脚本
			SCRIPT_OBJECT_CALL_ARGS2(getEntryScript().get(), const_cast<char*>("onGlobalData"), 
				const_cast<char*>("OO"), pyKey, pyValue);
		}

		Py_DECREF(pyValue);
	}

	Py_DECREF(pyKey);
}

template<class E>
void EntityApp<E>::onExecScriptCommand(Mercury::Channel* pChannel, KBEngine::MemoryStream& s)
{
	std::string cmd;
	s.readBlob(cmd);

	PyObject* pycmd = PyUnicode_DecodeUTF8(cmd.data(), cmd.size(), NULL);
	if(pycmd == NULL)
	{
		SCRIPT_ERROR_CHECK();
		return;
	}

	DEBUG_MSG(boost::format("EntityApp::onExecScriptCommand: size(%1%), command=%2%.\n") % 
		cmd.size() % cmd);

	std::string retbuf = "";
	PyObject* pycmd1 = PyUnicode_AsEncodedString(pycmd, "utf-8", NULL);
	script_.run_simpleString(PyBytes_AsString(pycmd1), &retbuf);

	if(retbuf.size() == 0)
	{
		retbuf = "\r\n";
	}

	// 将结果返回给客户端
	Mercury::Bundle bundle;
	ConsoleInterface::ConsoleExecCommandCBMessageHandler msgHandler;
	bundle.newMessage(msgHandler);
	ConsoleInterface::ConsoleExecCommandCBMessageHandlerArgs1::staticAddToBundle(bundle, retbuf);
	bundle.send(this->networkInterface(), pChannel);

	Py_DECREF(pycmd);
	Py_DECREF(pycmd1);
}

template<class E>
void EntityApp<E>::onReloadScript(bool fullReload)
{
	EntityMailbox::MAILBOXS::iterator iter =  EntityMailbox::mailboxs.begin();
	for(; iter != EntityMailbox::mailboxs.end(); iter++)
	{
		(*iter)->reload();
	}
}

template<class E>
void EntityApp<E>::reloadScript(bool fullReload)
{
	EntityDef::reload(fullReload);
	onReloadScript(fullReload);

	// 所有脚本都加载完毕
	PyObject* pyResult = PyObject_CallMethod(getEntryScript().get(), 
										const_cast<char*>("onInit"), 
										const_cast<char*>("i"), 
										1);

	if(pyResult != NULL)
		Py_DECREF(pyResult);
	else
		SCRIPT_ERROR_CHECK();
}

}

#endif // KBE_ENTITY_APP_HPP
