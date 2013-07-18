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

#include "pybots.hpp"
#include "bots.hpp"
#include "client_lib/entity.hpp"
#include "clientobject.hpp"
#include "bots_interface.hpp"
#include "resmgr/resmgr.hpp"
#include "network/common.hpp"
#include "network/tcp_packet.hpp"
#include "network/udp_packet.hpp"
#include "network/message_handler.hpp"
#include "thread/threadpool.hpp"
#include "server/componentbridge.hpp"
#include "server/serverconfig.hpp"
#include "helper/console_helper.hpp"

#include "../../../server/baseapp/baseapp_interface.hpp"
#include "../../../server/loginapp/loginapp_interface.hpp"

namespace KBEngine{
ServerConfig g_serverConfig;
Componentbridge* g_pComponentbridge = NULL;

//-------------------------------------------------------------------------------------
Bots::Bots(Mercury::EventDispatcher& dispatcher, 
			 Mercury::NetworkInterface& ninterface, 
			 COMPONENT_TYPE componentType,
			 COMPONENT_ID componentID):
ClientApp(dispatcher, ninterface, componentType, componentID),
pPyBots_(NULL),
clients_(),
reqCreateAndLoginTotalCount_(g_serverConfig.getBots().defaultAddBots_totalCount),
reqCreateAndLoginTickCount_(g_serverConfig.getBots().defaultAddBots_tickCount),
reqCreateAndLoginTickTime_(g_serverConfig.getBots().defaultAddBots_tickTime),
pCreateAndLoginHandler_(NULL),
pEventPoller_(Mercury::EventPoller::create())
{
	KBEngine::Mercury::MessageHandlers::pMainMessageHandlers = &BotsInterface::messageHandlers;
	g_pComponentbridge = new Componentbridge(ninterface, componentType, componentID);
}

//-------------------------------------------------------------------------------------
Bots::~Bots()
{
	SAFE_RELEASE(g_pComponentbridge);
	SAFE_RELEASE(pEventPoller_);
}

//-------------------------------------------------------------------------------------
bool Bots::initialize()
{
	// 广播自己的地址给网上上的所有kbemachine
	this->getMainDispatcher().addFrequentTask(&Componentbridge::getSingleton());
	return ClientApp::initialize();
}

//-------------------------------------------------------------------------------------	
bool Bots::initializeBegin()
{
	gameTimer_ = this->getMainDispatcher().addTimer(1000000 / g_kbeSrvConfig.gameUpdateHertz(), this,
							reinterpret_cast<void *>(TIMEOUT_GAME_TICK));

	ProfileVal::setWarningPeriod(stampsPerSecond() / g_kbeSrvConfig.gameUpdateHertz());
	return true;
}

//-------------------------------------------------------------------------------------	
bool Bots::initializeEnd()
{
	return true;
}

//-------------------------------------------------------------------------------------
void Bots::finalise()
{
	reqCreateAndLoginTotalCount_ = 0;
	SAFE_RELEASE(pCreateAndLoginHandler_);
	ClientApp::finalise();
}

//-------------------------------------------------------------------------------------
bool Bots::installEntityDef()
{
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
	// DEBUG_MSG("EntityApp::handleGameTick[%"PRTime"]:%u\n", t, time_);
	ClientApp::handleGameTick();

	pEventPoller_->processPendingEvents(0.0);

	CLIENTS::iterator iter = clients().begin();
	for(;iter != clients().end(); iter++)
		iter->second.get()->gameTick();
}

//-------------------------------------------------------------------------------------
Mercury::Channel* Bots::findChannelByMailbox(EntityMailbox& mailbox)
{
	int32 appID = (int32)mailbox.getComponentID();
	ClientObject* pClient = findClientByAppID(appID);

	if(pClient)
		return pClient->findChannelByMailbox(mailbox);

	return NULL;
}

//-------------------------------------------------------------------------------------
void Bots::addBots(Mercury::Channel * pChannel, MemoryStream& s)
{
	uint32	reqCreateAndLoginTotalCount;
	uint32 reqCreateAndLoginTickCount = 0;
	float reqCreateAndLoginTickTime = 0;

	s >> reqCreateAndLoginTotalCount;

	reqCreateAndLoginTotalCount_ += reqCreateAndLoginTotalCount;

	if(s.opsize() > 0)
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
		if(PyArg_ParseTuple(args, "I", &reqCreateAndLoginTotalCount) == -1)
		{
			PyErr_Format(PyExc_TypeError, "KBEngine::addBots: args is error!");
			PyErr_PrintEx(0);
			return NULL;
		}

		Bots::getSingleton().reqCreateAndLoginTotalCount(
			Bots::getSingleton().reqCreateAndLoginTotalCount() + reqCreateAndLoginTotalCount);
	}
	else if(PyTuple_Size(args) == 3)
	{
		if(PyArg_ParseTuple(args, "I|I|f", &reqCreateAndLoginTotalCount, 
			&reqCreateAndLoginTickCount, &reqCreateAndLoginTickTime) == -1)
		{
			PyErr_Format(PyExc_TypeError, "KBEngine::addBots: args is error!");
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
		PyErr_Format(PyExc_TypeError, "KBEngine::addBots: args is error!");
		PyErr_PrintEx(0);
		return NULL;
	}

	S_Return;
}

//-------------------------------------------------------------------------------------	
PyObject* Bots::__py_setScriptLogType(PyObject* self, PyObject* args)
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
	}

	DebugHelper::getSingleton().setScriptMsgType(type);
	S_Return;
}

//-------------------------------------------------------------------------------------
void Bots::lookApp(Mercury::Channel* pChannel)
{
	DEBUG_MSG(boost::format("Bots::lookApp: %1%\n") % pChannel->c_str());

	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
	
	(*pBundle) << g_componentType;
	(*pBundle) << componentID_;
	int8 istate = 0;
	(*pBundle) << istate;

	(*pBundle).send(getNetworkInterface(), pChannel);

	Mercury::Bundle::ObjPool().reclaimObject(pBundle);
}

//-------------------------------------------------------------------------------------
void Bots::reqCloseServer(Mercury::Channel* pChannel, MemoryStream& s)
{
	DEBUG_MSG(boost::format("Bots::reqCloseServer: %1%\n") % pChannel->c_str());

	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
	
	bool success = true;
	(*pBundle) << success;
	(*pBundle).send(getNetworkInterface(), pChannel);

	Mercury::Bundle::ObjPool().reclaimObject(pBundle);

	this->shutDown();
}

//-------------------------------------------------------------------------------------
void Bots::onExecScriptCommand(Mercury::Channel* pChannel, KBEngine::MemoryStream& s)
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

	if(getScript().run_simpleString(PyBytes_AsString(pycmd1), &retbuf) == 0)
	{
		// 将结果返回给客户端
		Mercury::Bundle bundle;
		ConsoleInterface::ConsoleExecCommandCBMessageHandler msgHandler;
		bundle.newMessage(msgHandler);
		ConsoleInterface::ConsoleExecCommandCBMessageHandlerArgs1::staticAddToBundle(bundle, retbuf);
		bundle.send(this->getNetworkInterface(), pChannel);
	}

	Py_DECREF(pycmd);
	Py_DECREF(pycmd1);
}

//-------------------------------------------------------------------------------------
bool Bots::addClient(ClientObject* pClient)
{
	clients().insert(std::make_pair< Mercury::Channel*, ClientObjectPtr >(pClient->pServerChannel(), 
		ClientObjectPtr(pClient)));

	return true;
}

//-------------------------------------------------------------------------------------
bool Bots::delClient(ClientObject* pClient)
{
	clients().erase(pClient->pServerChannel());
	pClient->finalise();
	Py_DECREF(pClient);
	return true;
}

//-------------------------------------------------------------------------------------
ClientObject* Bots::findClient(Mercury::Channel * pChannel)
{
	CLIENTS::iterator iter = clients().find(pChannel);
	if(iter != clients().end())
	{
		return iter->second.get();
	}

	return NULL;
}

//-------------------------------------------------------------------------------------
ClientObject* Bots::findClientByAppID(int32 appID)
{
	CLIENTS::iterator iter = clients().begin();
	for(; iter != clients().end(); iter++)
	{
		if(iter->second.get()->appID() == appID)
			return iter->second.get();
	}

	return NULL;
}

//-------------------------------------------------------------------------------------
void Bots::onAppActiveTick(Mercury::Channel* pChannel, COMPONENT_TYPE componentType, COMPONENT_ID componentID)
{
	if(componentType != CLIENT_TYPE)
		if(pChannel->isExternal())
			return;
	
	Mercury::Channel* pTargetChannel = NULL;
	if(componentType != CONSOLE_TYPE && componentType != CLIENT_TYPE)
	{
		Components::ComponentInfos* cinfos = 
			Componentbridge::getComponents().findComponent(componentType, KBEngine::getUserUID(), componentID);

		if(cinfos == NULL)
		{
			ERROR_MSG(boost::format("Bots::onAppActiveTick[%1%]: %2%:%3% not found.\n") % 
				pChannel % COMPONENT_NAME_EX(componentType) % componentID);

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

	//DEBUG_MSG("ServerApp::onAppActiveTick[%x]: %s:%"PRAppID" lastReceivedTime:%"PRIu64" at %s.\n", 
	//	pChannel, COMPONENT_NAME_EX(componentType), componentID, pChannel->lastReceivedTime(), pTargetChannel->c_str());
}

//-------------------------------------------------------------------------------------
void Bots::onHelloCB_(Mercury::Channel* pChannel, const std::string& verInfo, 
		COMPONENT_TYPE componentType)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onHelloCB_(pChannel, verInfo, componentType);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onCreateAccountResult(Mercury::Channel * pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onCreateAccountResult(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------	
void Bots::onLoginSuccessfully(Mercury::Channel * pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onLoginSuccessfully(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------	
void Bots::onLoginFailed(Mercury::Channel * pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onLoginFailed(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------	
void Bots::onLoginGatewayFailed(Mercury::Channel * pChannel, SERVER_ERROR_CODE failedcode)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onLoginGatewayFailed(pChannel, failedcode);
	}
}

//-------------------------------------------------------------------------------------	
void Bots::onCreatedProxies(Mercury::Channel * pChannel, 
								 uint64 rndUUID, ENTITY_ID eid, std::string& entityType)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onCreatedProxies(pChannel, rndUUID, eid, entityType);
	}
}

//-------------------------------------------------------------------------------------	
void Bots::onEntityEnterWorld(Mercury::Channel * pChannel, ENTITY_ID eid, 
							  ENTITY_SCRIPT_UID scriptType, SPACE_ID spaceID)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onEntityEnterWorld(pChannel, eid, scriptType, spaceID);
	}
}

//-------------------------------------------------------------------------------------	
void Bots::onEntityLeaveWorld(Mercury::Channel * pChannel, ENTITY_ID eid, SPACE_ID spaceID)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onEntityLeaveWorld(pChannel, eid, spaceID);
	}
}

//-------------------------------------------------------------------------------------	
void Bots::onEntityEnterSpace(Mercury::Channel * pChannel, SPACE_ID spaceID, ENTITY_ID eid)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onEntityEnterSpace(pChannel, eid, spaceID);
	}
}

//-------------------------------------------------------------------------------------	
void Bots::onEntityLeaveSpace(Mercury::Channel * pChannel, SPACE_ID spaceID, ENTITY_ID eid)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onEntityLeaveSpace(pChannel, eid, spaceID);
	}
}

//-------------------------------------------------------------------------------------	
void Bots::onEntityDestroyed(Mercury::Channel * pChannel, ENTITY_ID eid)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onEntityDestroyed(pChannel, eid);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onRemoteMethodCall(Mercury::Channel* pChannel, KBEngine::MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onRemoteMethodCall(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------	
void Bots::onKicked(Mercury::Channel * pChannel, SERVER_ERROR_CODE failedcode)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onKicked(pChannel, failedcode);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onUpdatePropertys(Mercury::Channel* pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onUpdatePropertys(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onUpdateBasePos(Mercury::Channel* pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onUpdateBasePos(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onSetEntityPosAndDir(Mercury::Channel* pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onSetEntityPosAndDir(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onUpdateData(Mercury::Channel* pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onUpdateData(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onUpdateData_ypr(Mercury::Channel* pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onUpdateData_ypr(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onUpdateData_yp(Mercury::Channel* pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onUpdateData_yp(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onUpdateData_yr(Mercury::Channel* pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onUpdateData_yr(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onUpdateData_pr(Mercury::Channel* pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onUpdateData_pr(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onUpdateData_y(Mercury::Channel* pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onUpdateData_y(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onUpdateData_p(Mercury::Channel* pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onUpdateData_p(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onUpdateData_r(Mercury::Channel* pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onUpdateData_r(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onUpdateData_xz(Mercury::Channel* pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onUpdateData_xz(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onUpdateData_xz_ypr(Mercury::Channel* pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onUpdateData_xz_ypr(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onUpdateData_xz_yp(Mercury::Channel* pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onUpdateData_xz_yp(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onUpdateData_xz_yr(Mercury::Channel* pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onUpdateData_xz_yr(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onUpdateData_xz_pr(Mercury::Channel* pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onUpdateData_xz_pr(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onUpdateData_xz_y(Mercury::Channel* pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onUpdateData_xz_y(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onUpdateData_xz_p(Mercury::Channel* pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onUpdateData_xz_p(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onUpdateData_xz_r(Mercury::Channel* pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onUpdateData_xz_r(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onUpdateData_xyz(Mercury::Channel* pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onUpdateData_xyz(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onUpdateData_xyz_ypr(Mercury::Channel* pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onUpdateData_xyz_ypr(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onUpdateData_xyz_yp(Mercury::Channel* pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onUpdateData_xyz_yp(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onUpdateData_xyz_yr(Mercury::Channel* pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onUpdateData_xyz_yr(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onUpdateData_xyz_pr(Mercury::Channel* pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onUpdateData_xyz_pr(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onUpdateData_xyz_y(Mercury::Channel* pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onUpdateData_xyz_y(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onUpdateData_xyz_p(Mercury::Channel* pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onUpdateData_xyz_p(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onUpdateData_xyz_r(Mercury::Channel* pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onUpdateData_xyz_r(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onStreamDataStarted(Mercury::Channel* pChannel, int16 id, uint32 datasize, std::string& descr)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onStreamDataStarted(pChannel, id, datasize, descr);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onStreamDataRecv(Mercury::Channel* pChannel, MemoryStream& s)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onStreamDataRecv(pChannel, s);
	}
}

//-------------------------------------------------------------------------------------
void Bots::onStreamDataCompleted(Mercury::Channel* pChannel, int16 id)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->onStreamDataCompleted(pChannel, id);
	}
}

//-------------------------------------------------------------------------------------
void Bots::addSpaceGeometryMapping(Mercury::Channel* pChannel, SPACE_ID spaceID, std::string& respath)
{
	ClientObject* pClient = findClient(pChannel);
	if(pClient)
	{
		pClient->addSpaceGeometryMapping(pChannel, spaceID, respath);
	}
}

//-------------------------------------------------------------------------------------

}
