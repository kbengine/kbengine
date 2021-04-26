// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include "pybots.h"
#include "bots.h"
#include "clientobject.h"
#include "server/telnet_server.h"
#include "server/components.h"
#include "client_lib/entity.h"
#include "clientobject.h"
#include "bots_interface.h"
#include "resmgr/resmgr.h"
#include "network/common.h"
#include "network/tcp_packet.h"
#include "network/udp_packet.h"
#include "network/message_handler.h"
#include "thread/threadpool.h"
#include "server/components.h"
#include "server/serverconfig.h"
#include "helper/watch_pools.h"
#include "helper/console_helper.h"
#include "helper/watcher.h"
#include "helper/profile.h"
#include "helper/profiler.h"
#include "helper/profile_handler.h"
#include "pyscript/pyprofile_handler.h"
#include "entitydef/entity_component.h"

#include "../../../server/baseapp/baseapp_interface.h"
#include "../../../server/loginapp/loginapp_interface.h"

namespace KBEngine{

//-------------------------------------------------------------------------------------
Bots::Bots(Network::EventDispatcher& dispatcher, 
			 Network::NetworkInterface& ninterface, 
			 COMPONENT_TYPE componentType,
			 COMPONENT_ID componentID):
ClientApp(dispatcher, ninterface, componentType, componentID),
pPyBots_(NULL),
clients_(),
reqCreateAndLoginTotalCount_(g_kbeSrvConfig.getBots().defaultAddBots_totalCount),
reqCreateAndLoginTickCount_(g_kbeSrvConfig.getBots().defaultAddBots_tickCount),
reqCreateAndLoginTickTime_(g_kbeSrvConfig.getBots().defaultAddBots_tickTime),
pCreateAndLoginHandler_(NULL),
pEventPoller_(Network::EventPoller::create()),
pTelnetServer_(NULL)
{
	// 初始化EntityDef模块获取entity实体函数地址
	EntityDef::setGetEntityFunc(std::tr1::bind(&Bots::tryGetEntity, this,
		std::tr1::placeholders::_1, std::tr1::placeholders::_2));

	KBEngine::Network::MessageHandlers::pMainMessageHandlers = &BotsInterface::messageHandlers;
	Components::getSingleton().initialize(&ninterface, componentType, componentID);
}

//-------------------------------------------------------------------------------------
Bots::~Bots()
{
	Components::getSingleton().finalise();
	SAFE_RELEASE(pEventPoller_);
}

//-------------------------------------------------------------------------------------
bool Bots::initialize()
{
	// 广播自己的地址给网上上的所有kbemachine
	this->dispatcher().addTask(&Components::getSingleton());
	return ClientApp::initialize();
}

//-------------------------------------------------------------------------------------	
bool Bots::initializeBegin()
{
	Network::g_extReceiveWindowBytesOverflow = 0;
	Network::g_intReceiveWindowBytesOverflow = 0;
	Network::g_intReceiveWindowMessagesOverflow = 0;
	Network::g_extReceiveWindowMessagesOverflow = 0;
	Network::g_receiveWindowMessagesOverflowCritical = 0;

	gameTimer_ = this->dispatcher().addTimer(1000000 / g_kbeSrvConfig.gameUpdateHertz(), this,
							reinterpret_cast<void *>(TIMEOUT_GAME_TICK));

	ProfileVal::setWarningPeriod(stampsPerSecond() / g_kbeSrvConfig.gameUpdateHertz());
	return true;
}

//-------------------------------------------------------------------------------------	
bool Bots::initializeEnd()
{
	pTelnetServer_ = new TelnetServer(&dispatcher(), &networkInterface());
	pTelnetServer_->pScript(&getScript());

	if(!pTelnetServer_->start(g_kbeSrvConfig.getBots().telnet_passwd, 
		g_kbeSrvConfig.getBots().telnet_deflayer, 
		g_kbeSrvConfig.getBots().telnet_port))
	{
		ERROR_MSG("Bots::initialize: initializeEnd error!\n");
		return false;
	}

	// 所有脚本都加载完毕
	PyObject* pyResult = PyObject_CallMethod(getEntryScript().get(), 
										const_cast<char*>("onInit"), 
										const_cast<char*>("i"), 
										0);

	if(pyResult != NULL)
	{
		Py_DECREF(pyResult);
	}
	else
	{
		SCRIPT_ERROR_CHECK();
		return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------
void Bots::finalise()
{
	// 结束通知脚本
	PyObject* pyResult = PyObject_CallMethod(getEntryScript().get(), 
										const_cast<char*>("onFinish"),
										const_cast<char*>(""));

	if(pyResult != NULL)
	{
		Py_DECREF(pyResult);
	}
	else
	{
		SCRIPT_ERROR_CHECK();
	}

	CLIENTS::iterator iter = clients_.begin();
	for(; iter != clients_.end(); ++iter)
	{
		iter->second->finalise();
		Py_DECREF(iter->second);
	}

	clients_.clear();

	reqCreateAndLoginTotalCount_ = 0;
	SAFE_RELEASE(pCreateAndLoginHandler_);
	
	if (pTelnetServer_)
	{
		pTelnetServer_->stop();
		SAFE_RELEASE(pTelnetServer_);
	}

	ClientApp::finalise();
}

//-------------------------------------------------------------------------------------
bool Bots::installEntityDef()
{
	EntityDef::entityAliasID(ServerConfig::getSingleton().getCellApp().aliasEntityID);
	EntityDef::entitydefAliasID(ServerConfig::getSingleton().getCellApp().entitydefAliasID);

	return ClientApp::installEntityDef();
}

//-------------------------------------------------------------------------------------
bool Bots::uninstallPyScript()
{
	return ClientApp::uninstallPyScript();
}

//-------------------------------------------------------------------------------------
bool Bots::installPyModules()
{
	ClientObject::installScript(NULL);
	PyBots::installScript(NULL);

	pPyBots_ = new PyBots();
	registerPyObjectToScript("bots", pPyBots_);
	
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(), addBots, __py_addBots,	METH_VARARGS, 0);

	// 注册设置脚本输出类型
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(),	scriptLogType,	__py_setScriptLogType,	METH_VARARGS,	0)
	if(PyModule_AddIntConstant(this->getScript().getModule(), "LOG_TYPE_NORMAL", log4cxx::ScriptLevel::SCRIPT_INT))
	{
		ERROR_MSG( "Bots::installPyModules: Unable to set KBEngine.LOG_TYPE_NORMAL.\n");
	}

	if(PyModule_AddIntConstant(this->getScript().getModule(), "LOG_TYPE_INFO", log4cxx::ScriptLevel::SCRIPT_INFO))
	{
		ERROR_MSG( "Bots::installPyModules: Unable to set KBEngine.LOG_TYPE_INFO.\n");
	}

	if(PyModule_AddIntConstant(this->getScript().getModule(), "LOG_TYPE_ERR", log4cxx::ScriptLevel::SCRIPT_ERR))
	{
		ERROR_MSG( "Bots::installPyModules: Unable to set KBEngine.LOG_TYPE_ERR.\n");
	}

	if(PyModule_AddIntConstant(this->getScript().getModule(), "LOG_TYPE_DBG", log4cxx::ScriptLevel::SCRIPT_DBG))
	{
		ERROR_MSG( "Bots::installPyModules: Unable to set KBEngine.LOG_TYPE_DBG.\n");
	}

	if(PyModule_AddIntConstant(this->getScript().getModule(), "LOG_TYPE_WAR", log4cxx::ScriptLevel::SCRIPT_WAR))
	{
		ERROR_MSG( "Bots::installPyModules: Unable to set KBEngine.LOG_TYPE_WAR.\n");
	}

	registerScript(client::Entity::getScriptType());
	registerScript(EntityComponent::getScriptType());

	// 安装入口模块
	PyObject *entryScriptFileName = PyUnicode_FromString(g_kbeSrvConfig.getBots().entryScriptFile);
	if(entryScriptFileName != NULL)
	{
		entryScript_ = PyImport_Import(entryScriptFileName);

		if (PyErr_Occurred())
		{
			INFO_MSG(fmt::format("EntityApp::installPyModules: importing scripts/bots/{}.py...\n",
				g_kbeSrvConfig.getBots().entryScriptFile));

			PyErr_PrintEx(0);
		}

		S_RELEASE(entryScriptFileName);

		if(entryScript_.get() == NULL)
		{
			return false;
		}
	}

	onInstallPyModules();

	return true;
}

//-------------------------------------------------------------------------------------
bool Bots::uninstallPyModules()
{
	Py_XDECREF(pPyBots_);
	pPyBots_ = NULL;

	ClientObject::uninstallScript();
	PyBots::uninstallScript();
	return ClientApp::uninstallPyModules();
}

//-------------------------------------------------------------------------------------
bool Bots::run(void)
{
	pCreateAndLoginHandler_ = new CreateAndLoginHandler();
	return ClientApp::run();
}

//-------------------------------------------------------------------------------------
void Bots::handleTimeout(TimerHandle handle, void * arg)
{
	ClientApp::handleTimeout(handle, arg);
}

//-------------------------------------------------------------------------------------
void Bots::handleGameTick()
{
	// time_t t = ::time(NULL);
	// static int kbeTime = 0;
	// DEBUG_MSG(fmt::format("Bots::handleGameTick[{}]:{}\n", t, ++kbeTime));

	ClientApp::handleGameTick();

	pEventPoller_->processPendingEvents(0.0);

	{
		AUTO_SCOPED_PROFILE("updateBots");

		CLIENTS::iterator iter = clients().begin();
		for(;iter != clients().end();)
		{
			Network::Channel* pChannel = iter->first;
			ClientObject* pClientObject = iter->second;
			++iter;

			if(pClientObject->isDestroyed())
			{
				delClient(pChannel);
				continue;
			}

			pClientObject->gameTick();
		}
	}
}

//-------------------------------------------------------------------------------------
Network::Channel* Bots::findChannelByEntityCall(EntityCallAbstract& entityCall)
{
	int32 appID = (int32)entityCall.componentID();
	ClientObject* pClient = findClientByAppID(appID);

	if(pClient)
		return pClient->findChannelByEntityCall(entityCall);

	return NULL;
}

//-------------------------------------------------------------------------------------
PyObject* Bots::tryGetEntity(COMPONENT_ID componentID, ENTITY_ID eid)
{
	ClientObject* pClient = findClientByAppID(componentID);

	if (pClient)
		return pClient->tryGetEntity(componentID, eid);

	return NULL;
}

//-------------------------------------------------------------------------------------
void Bots::addBots(Network::Channel * pChannel, MemoryStream& s)
{
	uint32	reqCreateAndLoginTotalCount;
	uint32 reqCreateAndLoginTickCount = 0;
	float reqCreateAndLoginTickTime = 0;

	s >> reqCreateAndLoginTotalCount;

	reqCreateAndLoginTotalCount_ += reqCreateAndLoginTotalCount;

	if(s.length() > 0)
	{
		s >> reqCreateAndLoginTickCount >> reqCreateAndLoginTickTime;

		if(reqCreateAndLoginTickCount > 0)
			reqCreateAndLoginTickCount_ = reqCreateAndLoginTickCount;
		
		if(reqCreateAndLoginTickTime > 0)
			reqCreateAndLoginTickTime_ = reqCreateAndLoginTickTime;
	}
}

//-------------------------------------------------------------------------------------
PyObject* Bots::__py_addBots(PyObject* self, PyObject* args)
{
	uint32	reqCreateAndLoginTotalCount;
	uint32 reqCreateAndLoginTickCount = 0;
	float reqCreateAndLoginTickTime = 0;

	if(PyTuple_Size(args) == 1)
	{
		if(!PyArg_ParseTuple(args, "I", &reqCreateAndLoginTotalCount))
		{
			PyErr_Format(PyExc_TypeError, "KBEngine::addBots: args error!");
			PyErr_PrintEx(0);
			return NULL;
		}

		Bots::getSingleton().reqCreateAndLoginTotalCount(
			Bots::getSingleton().reqCreateAndLoginTotalCount() + reqCreateAndLoginTotalCount);
	}
	else if(PyTuple_Size(args) == 3)
	{
		if(!PyArg_ParseTuple(args, "I|I|f", &reqCreateAndLoginTotalCount, 
			&reqCreateAndLoginTickCount, &reqCreateAndLoginTickTime))
		{
			PyErr_Format(PyExc_TypeError, "KBEngine::addBots: args error!");
			PyErr_PrintEx(0);
			return NULL;
		}

		Bots::getSingleton().reqCreateAndLoginTotalCount(
			Bots::getSingleton().reqCreateAndLoginTotalCount() + reqCreateAndLoginTotalCount);

		if(reqCreateAndLoginTickCount > 0)
			Bots::getSingleton().reqCreateAndLoginTickCount(reqCreateAndLoginTickCount);
		
		if(reqCreateAndLoginTickTime > 0)
			Bots::getSingleton().reqCreateAndLoginTickTime(reqCreateAndLoginTickTime);
	}
	else
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::addBots: args error!");
		PyErr_PrintEx(0);
		return NULL;
	}

	S_Return;
}

//-------------------------------------------------------------------------------------	
PyObject* Bots::__py_setScriptLogType(PyObject* self, PyObject* args)
{
	int argCount = (int)PyTuple_Size(args);
	if(argCount != 1)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::scriptLogType(): args error!");
		PyErr_PrintEx(0);
		return 0;
	}

	int type = -1;

	if(!PyArg_ParseTuple(args, "i", &type))
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::scriptLogType(): args error!");
		PyErr_PrintEx(0);
	}

	DebugHelper::getSingleton().setScriptMsgType(type);
	S_Return;
}

//-------------------------------------------------------------------------------------
void Bots::lookApp(Network::Channel* pChannel)
{
	//DEBUG_MSG(fmt::format("Bots::lookApp: {0}\n", pChannel->c_str()));

	Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);
	
	(*pBundle) << g_componentType;
	(*pBundle) << componentID_;
	int8 istate = 0;
	(*pBundle) << istate;

	pChannel->send(pBundle);
}

//-------------------------------------------------------------------------------------
void Bots::reqCloseServer(Network::Channel* pChannel, MemoryStream& s)
{
	DEBUG_MSG(fmt::format("Bots::reqCloseServer: {0}\n", pChannel->c_str()));

	Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);
	
	bool success = true;
	(*pBundle) << success;
	pChannel->send(pBundle);

	this->shutDown();
}

//-------------------------------------------------------------------------------------
void Bots::reqKillServer(Network::Channel* pChannel, MemoryStream& s)
{
	COMPONENT_ID componentID;
	COMPONENT_TYPE componentType;
	std::string username;
	int32 uid;
	std::string reason;

	s >> componentID >> componentType >> username >> uid >> reason;

	INFO_MSG(fmt::format("Bots::reqKillServer: requester(uid:{}, username:{}, componentType:{}, "
				"componentID:{}, reason:{}, from {})\n",
				uid ,
				username , 
				COMPONENT_NAME_EX((COMPONENT_TYPE)componentType),
				componentID,
				reason,
				pChannel->c_str()));

	CRITICAL_MSG("The application was killed!\n");
}

//-------------------------------------------------------------------------------------
void Bots::onExecScriptCommand(Network::Channel* pChannel, KBEngine::MemoryStream& s)
{
	std::string cmd;
	s.readBlob(cmd);

	PyObject* pycmd = PyUnicode_DecodeUTF8(cmd.data(), cmd.size(), NULL);
	if(pycmd == NULL)
	{
		SCRIPT_ERROR_CHECK();
		return;
	}

	DEBUG_MSG(fmt::format("EntityApp::onExecScriptCommand: size({}), command={}.\n",
		cmd.size(), cmd));

	std::string retbuf = "";
	PyObject* pycmd1 = PyUnicode_AsEncodedString(pycmd, "utf-8", NULL);

	if(getScript().run_simpleString(PyBytes_AsString(pycmd1), &retbuf) == 0)
	{
		// 将结果返回给客户端
		Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);
		ConsoleInterface::ConsoleExecCommandCBMessageHandler msgHandler;
		(*pBundle).newMessage(msgHandler);
		ConsoleInterface::ConsoleExecCommandCBMessageHandlerArgs1::staticAddToBundle((*pBundle), retbuf);
		pChannel->send(pBundle);
	}

	Py_DECREF(pycmd);
	Py_DECREF(pycmd1);
}

//-------------------------------------------------------------------------------------
bool Bots::addClient(ClientObject* pClient)
{
	clients().insert(std::make_pair(pClient->pServerChannel(),
		pClient));

	return true;
}

//-------------------------------------------------------------------------------------
bool Bots::delClient(ClientObject* pClient)
{
	return delClient(pClient->pServerChannel());
}

//-------------------------------------------------------------------------------------
bool Bots::delClient(Network::Channel * pChannel)
{
	ClientObject* pClient = findClient(pChannel);
	if(!pClient)
		return false;

	pClient->finalise();
	clients().erase(pChannel);
	Py_DECREF(pClient);
	return true;
}

//-------------------------------------------------------------------------------------
ClientObject* Bots::findClient(Network::Channel * pChannel)
{
	CLIENTS::iterator iter = clients().find(pChannel);
	if(iter != clients().end())
	{
		return iter->second;
	}

	return NULL;
}

//-------------------------------------------------------------------------------------
ClientObject* Bots::findClientByAppID(int32 appID)
{
	CLIENTS::iterator iter = clients().begin();
	for(; iter != clients().end(); ++iter)
	{
		if(iter->second->appID() == appID)
			return iter->second;
	}

	return NULL;
}

//-------------------------------------------------------------------------------------
void Bots::onAppActiveTick(Network::Channel* pChannel, COMPONENT_TYPE componentType, COMPONENT_ID componentID)
{
	if(componentType != CLIENT_TYPE)
		if(pChannel->isExternal())
			return;
	
	Network::Channel* pTargetChannel = NULL;
	if(componentType != CONSOLE_TYPE && componentType != CLIENT_TYPE)
	{
		Components::ComponentInfos* cinfos = 
			Components::getSingleton().findComponent(componentType, KBEngine::getUserUID(), componentID);

		if(cinfos == NULL)
		{
			ERROR_MSG(fmt::format("Bots::onAppActiveTick[{0:p}]: {1}:{2} not found.\n", 
				(void*)pChannel, COMPONENT_NAME_EX(componentType), componentID));

			return;
		}

		pTargetChannel = cinfos->pChannel;
		pTargetChannel->updateLastReceivedTime();
	}
	else
	{
		pChannel->updateLastReceivedTime();
		pTargetChannel = pChannel;
	}

	//DEBUG_MSG(fmt::format("Bots::onAppActiveTick[:p]: {}:{} lastReceivedTime:{} at {}.\n",
	//	(void*)pChannel, COMPONENT_NAME_EX(componentType), componentID, pChannel->lastReceivedTime(), pChannel->c_str()));
}

//-------------------------------------------------------------------------------------
void Bots::onHelloCB_(Network::Channel* pChannel, const std::string& verInfo, 
		const std::string& scriptVerInfo, const std::string& protocolMD5, const std::string& entityDefMD5, 
		COMPONENT_TYPE componentType)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onHelloCB_(pChannel, verInfo, scriptVerInfo, protocolMD5, entityDefMD5, componentType);
	}
}

//-------------------------------------------------------------------------------------	
void Bots::onVersionNotMatch(Network::Channel* pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onVersionNotMatch(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------	
void Bots::onScriptVersionNotMatch(Network::Channel* pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onScriptVersionNotMatch(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onCreateAccountResult(Network::Channel * pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onCreateAccountResult(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------	
void Bots::onLoginSuccessfully(Network::Channel * pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onLoginSuccessfully(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------	
void Bots::onLoginFailed(Network::Channel * pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onLoginFailed(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------	
void Bots::onLoginBaseappFailed(Network::Channel * pChannel, SERVER_ERROR_CODE failedcode)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onLoginBaseappFailed(pChannel, failedcode);
	}
}

//-------------------------------------------------------------------------------------	
void Bots::onReloginBaseappSuccessfully(Network::Channel * pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onReloginBaseappSuccessfully(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------	
void Bots::onCreatedProxies(Network::Channel * pChannel, 
								 uint64 rndUUID, ENTITY_ID eid, std::string& entityType)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onCreatedProxies(pChannel, rndUUID, eid, entityType);
	}
}

//-------------------------------------------------------------------------------------	
void Bots::onEntityEnterWorld(Network::Channel * pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onEntityEnterWorld(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------	
void Bots::onEntityLeaveWorld(Network::Channel * pChannel, ENTITY_ID eid)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onEntityLeaveWorld(pChannel, eid);
	}
}

//-------------------------------------------------------------------------------------	
void Bots::onEntityLeaveWorldOptimized(Network::Channel * pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onEntityLeaveWorldOptimized(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------	
void Bots::onEntityEnterSpace(Network::Channel * pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onEntityEnterSpace(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------	
void Bots::onEntityLeaveSpace(Network::Channel * pChannel, ENTITY_ID eid)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onEntityLeaveSpace(pChannel, eid);
	}
}

//-------------------------------------------------------------------------------------	
void Bots::onEntityDestroyed(Network::Channel * pChannel, ENTITY_ID eid)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onEntityDestroyed(pChannel, eid);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onRemoteMethodCall(Network::Channel* pChannel, KBEngine::MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onRemoteMethodCall(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onRemoteMethodCallOptimized(Network::Channel* pChannel, KBEngine::MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onRemoteMethodCallOptimized(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------	
void Bots::onKicked(Network::Channel * pChannel, SERVER_ERROR_CODE failedcode)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onKicked(pChannel, failedcode);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onUpdatePropertys(Network::Channel* pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onUpdatePropertys(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onUpdatePropertysOptimized(Network::Channel* pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onUpdatePropertysOptimized(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onUpdateBasePos(Network::Channel* pChannel, float x, float y, float z)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onUpdateBasePos(pChannel, x, y, z);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onUpdateBasePosXZ(Network::Channel* pChannel, float x, float z)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onUpdateBasePosXZ(pChannel, x, z);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onUpdateBaseDir(Network::Channel* pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if (pClient)
	{
		pClient->onUpdateBaseDir(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onSetEntityPosAndDir(Network::Channel* pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onSetEntityPosAndDir(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onUpdateData(Network::Channel* pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onUpdateData(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onUpdateData_ypr(Network::Channel* pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onUpdateData_ypr(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onUpdateData_yp(Network::Channel* pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onUpdateData_yp(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onUpdateData_yr(Network::Channel* pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onUpdateData_yr(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onUpdateData_pr(Network::Channel* pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onUpdateData_pr(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onUpdateData_y(Network::Channel* pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onUpdateData_y(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onUpdateData_p(Network::Channel* pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onUpdateData_p(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onUpdateData_r(Network::Channel* pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onUpdateData_r(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onUpdateData_xz(Network::Channel* pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onUpdateData_xz(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onUpdateData_xz_ypr(Network::Channel* pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onUpdateData_xz_ypr(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onUpdateData_xz_yp(Network::Channel* pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onUpdateData_xz_yp(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onUpdateData_xz_yr(Network::Channel* pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onUpdateData_xz_yr(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onUpdateData_xz_pr(Network::Channel* pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onUpdateData_xz_pr(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onUpdateData_xz_y(Network::Channel* pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onUpdateData_xz_y(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onUpdateData_xz_p(Network::Channel* pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onUpdateData_xz_p(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onUpdateData_xz_r(Network::Channel* pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onUpdateData_xz_r(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onUpdateData_xyz(Network::Channel* pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onUpdateData_xyz(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onUpdateData_xyz_ypr(Network::Channel* pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onUpdateData_xyz_ypr(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onUpdateData_xyz_yp(Network::Channel* pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onUpdateData_xyz_yp(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onUpdateData_xyz_yr(Network::Channel* pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onUpdateData_xyz_yr(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onUpdateData_xyz_pr(Network::Channel* pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onUpdateData_xyz_pr(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onUpdateData_xyz_y(Network::Channel* pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onUpdateData_xyz_y(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onUpdateData_xyz_p(Network::Channel* pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onUpdateData_xyz_p(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onUpdateData_xyz_r(Network::Channel* pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onUpdateData_xyz_r(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onUpdateData_ypr_optimized(Network::Channel* pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if (pClient)
	{
		pClient->onUpdateData_ypr_optimized(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onUpdateData_yp_optimized(Network::Channel* pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if (pClient)
	{
		pClient->onUpdateData_yp_optimized(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onUpdateData_yr_optimized(Network::Channel* pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if (pClient)
	{
		pClient->onUpdateData_yr_optimized(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onUpdateData_pr_optimized(Network::Channel* pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if (pClient)
	{
		pClient->onUpdateData_pr_optimized(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onUpdateData_y_optimized(Network::Channel* pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if (pClient)
	{
		pClient->onUpdateData_y_optimized(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onUpdateData_p_optimized(Network::Channel* pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if (pClient)
	{
		pClient->onUpdateData_p_optimized(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onUpdateData_r_optimized(Network::Channel* pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if (pClient)
	{
		pClient->onUpdateData_r_optimized(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onUpdateData_xz_optimized(Network::Channel* pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if (pClient)
	{
		pClient->onUpdateData_xz_optimized(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onUpdateData_xz_ypr_optimized(Network::Channel* pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if (pClient)
	{
		pClient->onUpdateData_xz_ypr_optimized(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onUpdateData_xz_yp_optimized(Network::Channel* pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if (pClient)
	{
		pClient->onUpdateData_xz_yp_optimized(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onUpdateData_xz_yr_optimized(Network::Channel* pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if (pClient)
	{
		pClient->onUpdateData_xz_yr_optimized(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onUpdateData_xz_pr_optimized(Network::Channel* pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if (pClient)
	{
		pClient->onUpdateData_xz_pr_optimized(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onUpdateData_xz_y_optimized(Network::Channel* pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if (pClient)
	{
		pClient->onUpdateData_xz_y_optimized(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onUpdateData_xz_p_optimized(Network::Channel* pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if (pClient)
	{
		pClient->onUpdateData_xz_p_optimized(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onUpdateData_xz_r_optimized(Network::Channel* pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if (pClient)
	{
		pClient->onUpdateData_xz_r_optimized(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onUpdateData_xyz_optimized(Network::Channel* pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if (pClient)
	{
		pClient->onUpdateData_xyz_optimized(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onUpdateData_xyz_ypr_optimized(Network::Channel* pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if (pClient)
	{
		pClient->onUpdateData_xyz_ypr_optimized(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onUpdateData_xyz_yp_optimized(Network::Channel* pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if (pClient)
	{
		pClient->onUpdateData_xyz_yp_optimized(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onUpdateData_xyz_yr_optimized(Network::Channel* pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if (pClient)
	{
		pClient->onUpdateData_xyz_yr_optimized(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onUpdateData_xyz_pr_optimized(Network::Channel* pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if (pClient)
	{
		pClient->onUpdateData_xyz_pr_optimized(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onUpdateData_xyz_y_optimized(Network::Channel* pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if (pClient)
	{
		pClient->onUpdateData_xyz_y_optimized(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onUpdateData_xyz_p_optimized(Network::Channel* pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if (pClient)
	{
		pClient->onUpdateData_xyz_p_optimized(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onUpdateData_xyz_r_optimized(Network::Channel* pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if (pClient)
	{
		pClient->onUpdateData_xyz_r_optimized(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onControlEntity(Network::Channel* pChannel, int32 entityID, int8 isControlled)
{
	ClientObject* pClient = findClient(pChannel);
	if (pClient)
	{
		pClient->onControlEntity(pChannel, entityID, isControlled);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onStreamDataStarted(Network::Channel* pChannel, int16 id, uint32 datasize, std::string& descr)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onStreamDataStarted(pChannel, id, datasize, descr);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onStreamDataRecv(Network::Channel* pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onStreamDataRecv(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onStreamDataCompleted(Network::Channel* pChannel, int16 id)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onStreamDataCompleted(pChannel, id);
	}
}

//-------------------------------------------------------------------------------------	
void Bots::initSpaceData(Network::Channel* pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if (pClient)
	{
		pClient->initSpaceData(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------	
void Bots::setSpaceData(Network::Channel* pChannel, SPACE_ID spaceID, const std::string& key, const std::string& value)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->setSpaceData(pChannel, spaceID, key, value);
	}
}

//-------------------------------------------------------------------------------------	
void Bots::delSpaceData(Network::Channel* pChannel, SPACE_ID spaceID, const std::string& key)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->delSpaceData(pChannel, spaceID, key);
	}
}

//-------------------------------------------------------------------------------------		
void Bots::queryWatcher(Network::Channel* pChannel, MemoryStream& s)
{
	AUTO_SCOPED_PROFILE("watchers");

	std::string path;
	s >> path;

	MemoryStream::SmartPoolObjectPtr readStreamPtr = MemoryStream::createSmartPoolObj(OBJECTPOOL_POINT);
	WatcherPaths::root().readWatchers(path, readStreamPtr.get()->get());

	MemoryStream::SmartPoolObjectPtr readStreamPtr1 = MemoryStream::createSmartPoolObj(OBJECTPOOL_POINT);
	WatcherPaths::root().readChildPaths(path, path, readStreamPtr1.get()->get());

	Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);
	ConsoleInterface::ConsoleWatcherCBMessageHandler msgHandler;
	(*pBundle).newMessage(msgHandler);

	uint8 type = 0;
	(*pBundle) << type;
	(*pBundle).append(readStreamPtr.get()->get());
	pChannel->send(pBundle);

	Network::Bundle* pBundle1 = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);
	(*pBundle1).newMessage(msgHandler);

	type = 1;
	(*pBundle1) << type;
	(*pBundle1).append(readStreamPtr1.get()->get());
	pChannel->send(pBundle1);
}

//-------------------------------------------------------------------------------------
void Bots::startProfile(Network::Channel* pChannel, KBEngine::MemoryStream& s)
{
	std::string profileName;
	int8 profileType;
	uint32 timelen;

	s >> profileName >> profileType >> timelen;

	startProfile_(pChannel, profileName, profileType, timelen);
}

//-------------------------------------------------------------------------------------
void Bots::startProfile_(Network::Channel* pChannel, std::string profileName, int8 profileType, uint32 timelen)
{
	switch(profileType)
	{
	case 0:	// pyprofile
		new PyProfileHandler(this->networkInterface(), timelen, profileName, pChannel->addr());
		break;
	case 1:	// cprofile
		new CProfileHandler(this->networkInterface(), timelen, profileName, pChannel->addr());
		break;
	case 2:	// eventprofile
		new EventProfileHandler(this->networkInterface(), timelen, profileName, pChannel->addr());
		break;
	case 3:	// networkprofile
		new NetworkProfileHandler(this->networkInterface(), timelen, profileName, pChannel->addr());
		break;
	default:
		ERROR_MSG(fmt::format("Bots::startProfile_: type({}:{}) not support!\n", 
			profileType, profileName));

		break;
	};
}

//-------------------------------------------------------------------------------------
void Bots::onAppActiveTickCB(Network::Channel* pChannel)
{
	ClientObject* pClient = findClient(pChannel);
	if (pClient)
	{
		pClient->onAppActiveTickCB(pChannel);
	}
}

//-------------------------------------------------------------------------------------

}
