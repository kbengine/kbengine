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
#include "baseapp_interface.hpp"
#include "network/common.hpp"
#include "network/tcp_packet.hpp"
#include "network/udp_packet.hpp"
#include "server/componentbridge.hpp"
#include "server/components.hpp"
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

//-------------------------------------------------------------------------------------
Baseapp::Baseapp(Mercury::EventDispatcher& dispatcher, 
			 Mercury::NetworkInterface& ninterface, 
			 COMPONENT_TYPE componentType,
			 COMPONENT_ID componentID):
	EntityApp<Base>(dispatcher, ninterface, componentType, componentID),
	loopCheckTimerHandle_(),
	pGlobalBases_(NULL),
	pendingLoginMgr_(ninterface)
{
	KBEngine::Mercury::MessageHandlers::pMainMessageHandlers = &BaseappInterface::messageHandlers;
}

//-------------------------------------------------------------------------------------
Baseapp::~Baseapp()
{
}

//-------------------------------------------------------------------------------------
bool Baseapp::installPyModules()
{
	Base::installScript(getScript().getModule());
	Proxy::installScript(getScript().getModule());
	
	registerScript(Base::getScriptType());
	registerScript(Proxy::getScriptType());

	// 注册创建entity的方法到py
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(),		createBase,			__py_createBase,					METH_VARARGS,			0);
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(),		createBaseLocally,	__py_createBase,					METH_VARARGS,			0);
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(),		createEntity,		__py_createBase,					METH_VARARGS,			0);
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(), 		createBaseAnywhere, __py_createBaseAnywhere,			METH_VARARGS,			0);

	return EntityApp<Base>::installPyModules();
}

//-------------------------------------------------------------------------------------
void Baseapp::onInstallPyModules()
{
	// 添加globalData, globalBases支持
	pGlobalBases_ = new GlobalDataClient(DBMGR_TYPE, GlobalDataServer::GLOBAL_BASES);
	registerPyObjectToScript("globalBases", pGlobalBases_);
}

//-------------------------------------------------------------------------------------
bool Baseapp::uninstallPyModules()
{	
	unregisterPyObjectToScript("globalBases");
	S_RELEASE(pGlobalBases_); 

	Base::uninstallScript();
	Proxy::uninstallScript();
	return EntityApp<Base>::uninstallPyModules();
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
void Baseapp::handleGameTick()
{
	EntityApp<Base>::handleGameTick();
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
	return true;
}

//-------------------------------------------------------------------------------------
void Baseapp::finalise()
{
	loopCheckTimerHandle_.cancel();
	EntityApp<Base>::finalise();
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
	
	Components::ComponentInfos* cinfos = Componentbridge::getComponents().findComponent(tcomponentType, uid, componentID);
	cinfos->pChannel = NULL;

	int ret = Components::getSingleton().connectComponent(tcomponentType, uid, componentID);
	KBE_ASSERT(ret != -1);

	Mercury::Bundle bundle;

	switch(tcomponentType)
	{
	case BASEAPP_TYPE:
		bundle.newMessage(BaseappInterface::onRegisterNewApp);
		BaseappInterface::onRegisterNewAppArgs8::staticAddToBundle(bundle, getUserUID(), getUsername(), BASEAPP_TYPE, componentID_, 
			this->getNetworkInterface().intaddr().ip, this->getNetworkInterface().intaddr().port, 
			this->getNetworkInterface().extaddr().ip, this->getNetworkInterface().extaddr().port);
		break;
	case CELLAPP_TYPE:
		bundle.newMessage(CellappInterface::onRegisterNewApp);
		CellappInterface::onRegisterNewAppArgs8::staticAddToBundle(bundle, getUserUID(), getUsername(), BASEAPP_TYPE, componentID_, 
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
Base* Baseapp::onCreateEntityCommon(PyObject* pyEntity, ScriptModule* sm, ENTITY_ID eid)
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
		ERROR_MSG("Baseapp::createBase: args is error!");
		S_Return;
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
		ERROR_MSG("Baseapp::createBaseAnywhere: args is error!");
		S_Return;
	}

	if(!PyCallable_Check(pyCallback))
		pyCallback = NULL;

	Baseapp::getSingleton().createBaseAnywhere(entityType, params, pyCallback);
	S_Return;
}

//-------------------------------------------------------------------------------------
void Baseapp::createInNewSpace(Base* base, PyObject* cell)
{
	ENTITY_ID id = base->getID();
	std::string entityType = base->ob_type->tp_name;
	std::string strCellData = script::Pickler::pickle(base->getCellData());
	uint32 cellDataLength = strCellData.length();

	Mercury::Bundle bundle;

	bundle.newMessage(CellappmgrInterface::reqCreateInNewSpace);

	bundle << entityType;
	bundle << id;
	bundle << componentID_;
	bundle << cellDataLength;

	if(cellDataLength > 0)
		bundle.append(strCellData.data(), cellDataLength);
	
	Components::COMPONENTS& components = Components::getSingleton().getComponents(CELLAPPMGR_TYPE);
	Components::COMPONENTS::iterator iter = components.begin();
	if(iter != components.end())
	{
		bundle.send(this->getNetworkInterface(), (*iter).pChannel);
		return;
	}
	
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

	Mercury::Bundle bundle;
	bundle.newMessage(BaseappmgrInterface::reqCreateBaseAnywhere);

	bundle << entityType;
	bundle << initDataLength;
	if(initDataLength > 0)
		bundle.append(strInitData.data(), initDataLength);

	bundle << componentID_;

	CALLBACK_ID callbackID = 0;
	if(pyCallback != NULL)
	{
		Py_INCREF(pyCallback);
		callbackID = callbackMgr().save(pyCallback);
	}

	bundle << callbackID;

	Components::COMPONENTS& components = Components::getSingleton().getComponents(BASEAPPMGR_TYPE);
	Components::COMPONENTS::iterator iter = components.begin();
	if(iter != components.end())
	{
		bundle.send(this->getNetworkInterface(), (*iter).pChannel);
		return;
	}

	ERROR_MSG("Baseapp::createBaseAnywhere: not found baseappmgr.\n");
}

//-------------------------------------------------------------------------------------
void Baseapp::onCreateBaseAnywhere(Mercury::Channel* pChannel, MemoryStream& s)
{
	if(pChannel->isExternal())
		return;

	std::string strInitData = "";
	uint32 initDataLength = 0;
	PyObject* params = NULL;
	std::string entityType;
	COMPONENT_ID componentID;
	CALLBACK_ID callbackID;

	s >> entityType;
	s >> initDataLength;

	if(initDataLength > 0)
	{
		strInitData.assign((char*)(s.data() + s.rpos()), initDataLength);
		s.read_skip(initDataLength);
	}

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
		Mercury::Channel* lpChannel = Components::getSingleton().findComponent(BASEAPPMGR_TYPE, 0)->pChannel;

		if(lpChannel != NULL)
		{
			// 需要baseappmgr转发给目的baseapp
			Mercury::Bundle forwardbundle;
			forwardbundle.newMessage(BaseappInterface::onCreateBaseAnywhereCallback);
			forwardbundle << callbackID;
			forwardbundle << entityType;
			forwardbundle << base->getID();
			forwardbundle << componentID_;

			Mercury::Bundle bundle;
			MERCURY_MESSAGE_FORWARD(BaseappmgrInterface, bundle, forwardbundle, componentID_, componentID);
			bundle.send(this->getNetworkInterface(), lpChannel);
		}
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

	PyObject* pyCallback = callbackMgr().take(callbackID);
	PyObject* pyargs = PyTuple_New(1);

	if(pChannel != NULL)
	{
		ScriptModule* sm = EntityDef::findScriptModule(entityType.c_str());
		if(sm == NULL)
		{
			ERROR_MSG("Baseapp::onCreateBaseAnywhereCallback: can't found entityType:%s.\n", entityType.c_str());
			Py_DECREF(pyargs);
			return;
		}
		
		// 如果entity属于另一个baseapp创建则设置它的mailbox
		Mercury::Channel* pOtherBaseappChannel = Components::getSingleton().findComponent(componentID)->pChannel;
		KBE_ASSERT(pOtherBaseappChannel != NULL);
		PyObject* mb = static_cast<EntityMailbox*>(new EntityMailbox(sm, componentID, eid, MAILBOX_TYPE_BASE));
		PyTuple_SET_ITEM(pyargs, 0, mb);
		PyObject_CallObject(pyCallback, pyargs);
		//Py_DECREF(mb);
	}
	else
	{
		Base* base = pEntities_->find(eid);
		if(base == NULL)
		{
			ERROR_MSG("Baseapp::onCreateBaseAnywhereCallback: can't found entity:%ld.\n", eid);
			Py_DECREF(pyargs);
			return;
		}

		Py_INCREF(base);
		PyTuple_SET_ITEM(pyargs, 0, base);
		PyObject_CallObject(pyCallback, pyargs);
	}

	SCRIPT_ERROR_CHECK();
	Py_DECREF(pyargs);
	Py_DECREF(pyCallback);
}

//-------------------------------------------------------------------------------------
void Baseapp::createCellEntity(EntityMailboxAbstract* createToCellMailbox, Base* base)
{
	Mercury::Bundle bundle;
	bundle.newMessage(CellappInterface::onCreateCellEntityFromBaseapp);

	ENTITY_ID id = base->getID();
	std::string entityType = base->ob_type->tp_name;
	std::string strCellData = script::Pickler::pickle(base->getCellData());
	uint32 cellDataLength = strCellData.length();
	EntityMailbox* clientMailbox = base->getClientMailbox();
	bool hasClient = (clientMailbox == NULL);
	
	bundle << createToCellMailbox->getID();				// 在这个mailbox所在的cellspace上创建
	bundle << entityType;
	bundle << id;
	bundle << componentID_;
	bundle << hasClient;
	bundle << cellDataLength;

	if(cellDataLength > 0)
		bundle.append(strCellData.data(), cellDataLength);

	KBE_ASSERT(createToCellMailbox->getChannel() != NULL);
	bundle.send(this->getNetworkInterface(), createToCellMailbox->getChannel());
	
	if(!hasClient)
		return;

	// 通报客户端cellEntity创建了

	//sp = new SocketPacket(OP_CREATE_CLIENT_PROXY);
	//(*sp) << (uint8)false;	
	//(*sp) << (ENTITY_ID)id;

	// 初始化cellEntity的位置和方向变量
	//Vector3 v;
	//PyObject* cellData = base->getCellData();
	//PyObject* position = PyDict_GetItemString(cellData, "position");
	//script::ScriptVector3::convertPyObjectToVector3(v, position);
	//(*sp) << v.x << v.y << v.z;

	//PyObject* direction = PyDict_GetItemString(cellData, "direction");
	//script::ScriptVector3::convertPyObjectToVector3(v, direction);
	//(*sp) << v.x << v.y << v.z;
	
	// 服务器已经确定要创建cell部分实体了， 我们可以从celldata获取客户端感兴趣的数据初始化客户端 如:ALL_CLIENTS
	//base->getCellDataByFlags(ED_FLAG_ALL_CLIENTS|ED_FLAG_CELL_PUBLIC_AND_OWN|ED_FLAG_OWN_CLIENT, sp);
	//clientMailbox->post(sp);
}

//-------------------------------------------------------------------------------------
void Baseapp::onEntityGetCell(Mercury::Channel* pChannel, ENTITY_ID id, COMPONENT_ID componentID)
{
	if(pChannel->isExternal())
		return;

	Base* base = pEntities_->find(id);
	DEBUG_MSG("Baseapp::onEntityGetCell: entityID %d.\n", id);
	KBE_ASSERT(base != NULL);
	base->onGetCell(pChannel, componentID);
}

//-------------------------------------------------------------------------------------
void Baseapp::onDbmgrInitCompleted(Mercury::Channel* pChannel, 
		ENTITY_ID startID, ENTITY_ID endID, int32 startGlobalOrder, int32 startGroupOrder)
{
	if(pChannel->isExternal())
		return;

	EntityApp<Base>::onDbmgrInitCompleted(pChannel, startID, endID, startGlobalOrder, startGroupOrder);

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
										startGroupOrder == 1 ? 1 : 0);

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
		if(pGlobalBases_->del(pyKey))
		{
			// 通知脚本
			PyObject* pyResult = PyObject_CallMethod(getEntryScript().get(), 
												const_cast<char*>("onGlobalBasesDel"), 
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
		if(pGlobalBases_->write(pyKey, pyValue))
		{
			// 通知脚本
			PyObject* pyResult = PyObject_CallMethod(getEntryScript().get(), 
												const_cast<char*>("onGlobalBases"), 
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
void Baseapp::registerPendingLogin(Mercury::Channel* pChannel, std::string& accountName, std::string& password)
{
	if(pChannel->isExternal())
		return;

	Mercury::Bundle bundle;
	bundle.newMessage(BaseappmgrInterface::onPendingAccountGetBaseappAddr);
	bundle << accountName;
	bundle << this->getNetworkInterface().extaddr().ip;
	bundle << this->getNetworkInterface().extaddr().port;
	bundle.send(this->getNetworkInterface(), pChannel);

	PendingLoginMgr::PLInfos* ptinfos = new PendingLoginMgr::PLInfos;
	ptinfos->accountName = accountName;
	ptinfos->password = password;
	pendingLoginMgr_.add(ptinfos);
}

//-------------------------------------------------------------------------------------
void Baseapp::loginGatewayFailed(Mercury::Channel* pChannel, std::string& accountName, int8 failedcode)
{
	Mercury::Bundle bundle;
	bundle.newMessage(ClientInterface::onLoginGatewayFailed);
	ClientInterface::onLoginGatewayFailedArgs1::staticAddToBundle(bundle, failedcode);
	bundle.send(this->getNetworkInterface(), pChannel);

	if(failedcode == 0)
	{
		DEBUG_MSG("Baseapp::loginnot found user[%s], login is failed!\n", accountName.c_str());
	}
	else if(failedcode == 1)
	{
		DEBUG_MSG("Baseapp::loginnot user[%s] password is error, login is failed!\n", accountName.c_str());
	}
}

//-------------------------------------------------------------------------------------
void Baseapp::loginGateway(Mercury::Channel* pChannel, std::string& accountName, std::string& password)
{
	DEBUG_MSG("Baseapp::login: new user[%s].\n", accountName.c_str());

	PendingLoginMgr::PLInfos* ptinfos = pendingLoginMgr_.remove(accountName);
	if(ptinfos == NULL)
	{
		loginGatewayFailed(pChannel, accountName, 0);
		return;
	}

	if(ptinfos->password != password)
	{
		loginGatewayFailed(pChannel, accountName, 1);
		return;
	}

	Mercury::Bundle bundle;
	bundle.newMessage(ClientInterface::onLoginGatewaySuccessfully);
	bundle.send(this->getNetworkInterface(), pChannel);
}

//-------------------------------------------------------------------------------------

}
