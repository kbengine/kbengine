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

namespace KBEngine{
	
ServerConfig g_serverConfig;
KBE_SINGLETON_INIT(Baseapp);

//-------------------------------------------------------------------------------------
Baseapp::Baseapp(Mercury::EventDispatcher& dispatcher, 
			 Mercury::NetworkInterface& ninterface, 
			 COMPONENT_TYPE componentType,
			 COMPONENT_ID componentID):
	EntityApp<Base>(dispatcher, ninterface, componentType, componentID),
	pGlobalBases_(NULL)
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

	// 添加globalData, globalBases支持
	pGlobalBases_ = new GlobalDataClient(BASEAPPMGR_TYPE, GlobalDataServer::GLOBAL_BASES);
	registerPyObjectToScript("globalBases", pGlobalBases_);

	// 注册创建entity的方法到py
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(),		createBase,			__py_createBase,					METH_VARARGS,			0);
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(),		createBaseLocally,	__py_createBase,					METH_VARARGS,			0);
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(),		createEntity,		__py_createBase,					METH_VARARGS,			0);
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(), 		createBaseAnywhere, __py_createBaseAnywhere,			METH_VARARGS,			0);

	return EntityApp<Base>::installPyModules();
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
//	switch (reinterpret_cast<uintptr>(arg))
//	{
//		default:
//			break;
//	}

	EntityApp<Base>::handleTimeout(handle, arg);
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
	return true;
}

//-------------------------------------------------------------------------------------
void Baseapp::finalise()
{
	EntityApp<Base>::finalise();
}

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
{/*
	SocketPacket* sp = new SocketPacket(OP_CREATE_IN_NEW_SPACE, 10);
	ENTITY_ID id = base->getID();
	std::string entityType = base->ob_type->tp_name;
	std::string strCellData = script::Pickler::pickle(base->getCellData());
	uint32 cellDataLength = strCellData.length();
	
	(*sp) << entityType;
	(*sp) << (ENTITY_ID)id;
	(*sp) << (COMPONENT_ID)m_componentID_;
	(*sp) << (uint32)cellDataLength;
	if(cellDataLength > 0)
		sp->append(strCellData.c_str(), cellDataLength);
	
	COMPONENT_MAP& components = EngineComponentMgr::getSingleton().getComponents(CELLAPPMGR_TYPE);
	COMPONENT_MAP::iterator iter = components.begin();
	if(iter != components.end())
	{
		NSChannel* lpChannel = static_cast<NSChannel*>(iter->second);
		lpChannel->sendPacket(sp);
		return;
	}
	*/
	ERROR_MSG("App::createInNewSpace: not found cellappmgr.\n");
}

//-------------------------------------------------------------------------------------
void Baseapp::createBaseAnywhere(const char* entityType, PyObject* params, PyObject* pyCallback)
{
	/*
	SocketPacket* sp = new SocketPacket(OP_CREATE_BASE_ANY_WHERE, 10);
	
	std::string strInitData = "";
	uint32 initDataLength = 0;
	if(params != NULL && PyDict_Check(params))
	{
		strInitData = script::Pickler::pickle(params);
		initDataLength = strInitData.length();
	}

	CALLBACK_ID callbackID = 0;
	if(pyCallback != NULL)
	{
		Py_INCREF(pyCallback);
		callbackID = m_pyCallbackMgr.save(pyCallback);
	}

	(*sp) << entityType;
	(*sp) << (uint32)initDataLength;

	if(initDataLength > 0)
		sp->append(strInitData.c_str(), initDataLength);

	(*sp) << (COMPONENT_ID)m_componentID_;
	(*sp) << (CALLBACK_ID)callbackID;

	COMPONENT_MAP& components = EngineComponentMgr::getSingleton().getComponents(BASEAPPMGR_TYPE);
	COMPONENT_MAP::iterator iter = components.begin();
	if(iter != components.end())
	{
		NSChannel* lpChannel = static_cast<NSChannel*>(iter->second);
		lpChannel->sendPacket(sp);
		return;
	}
	*/
	ERROR_MSG("App::createBaseAnywhere: not found baseappmgr.\n");
}

//-------------------------------------------------------------------------------------
void Baseapp::onCreateBaseAnywhere(std::string& entityType, std::string& strInitData, 
								   COMPONENT_ID componentID, CALLBACK_ID callbackID)
{
	/*
	PyObject* params = NULL;
	if(strInitData.size() > 0)
		params = script::Pickler::unpickle(strInitData);

	Base* base = createBase(entityType.c_str(), params);
	Py_XDECREF(params);

	if(base == NULL)
		return;

	if(componentID != m_componentID_)
	{
		Channel* lpChannel = EngineComponentMgr::getSingleton().findComponent(BASEAPP_TYPE, componentID);

		if(lpChannel != NULL)
		{
			SocketPacket* sp = new SocketPacket(OP_CREATE_BASE_ANY_WHERE_CALLBACK, 10);
			(*sp) << (CALLBACK_ID)callbackID;
			(*sp) << (ENTITY_ID)base->getID();
			(*sp) << (COMPONENT_ID)m_componentID_;
			(*sp) << entityType;
			lpChannel->sendPacket(sp);
		}
	}
	else
	{
		ENTITY_ID eid = base->getID();
		onCreateBaseAnywhereCallback(NULL, callbackID, entityType, eid, m_componentID_);
	}
	*/
}

//-------------------------------------------------------------------------------------
void Baseapp::onCreateBaseAnywhereCallback(Mercury::Channel* pChannel, CALLBACK_ID callbackID, 
										   std::string& entityType, ENTITY_ID eid, COMPONENT_ID componentID)
{
	if(callbackID == 0)
		return;
/*
	PyObject* pyCallback = m_pyCallbackMgr.take(callbackID);
	PyObject* pyargs = PyTuple_New(1);

	if(handler != NULL)
	{
		ScriptModule* sm = ExtendScriptModuleMgr::findScriptModule(entityType.c_str());
		if(sm == NULL)
		{
			ERROR_MSG("App::onCreateBaseAnywhereCallback: can't found entityType:%s.\n", entityType.c_str());
			Py_DECREF(pyargs);
			return;
		}

		PyTuple_SET_ITEM(pyargs, 0, new EntityMailbox(handler, sm, componentID, eid, MAILBOX_TYPE_BASE));
		PyObject_CallObject(pyCallback, pyargs);
	}
	else
	{
		Base* base = m_bases_->find(eid);
		if(base == NULL)
		{
			ERROR_MSG("App::onCreateBaseAnywhereCallback: can't found entity:%ld.\n", eid);
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
	*/
}

//-------------------------------------------------------------------------------------
void Baseapp::createCellEntity(EntityMailboxAbstract* createToCellMailbox, Base* base)
{
	/*
	SocketPacket* sp = new SocketPacket(OP_CREATE_CELL_ENTITY);
	ENTITY_ID id = base->getID();
	std::string entityType = base->ob_type->tp_name;
	std::string strCellData = script::Pickler::pickle(base->getCellData());
	uint32 cellDataLength = strCellData.length();
	EntityMailbox* clientMailbox = base->getClientMailbox();
	uint8 hasClient = (clientMailbox == NULL) ? 0 : 1;
	
	(*sp) << (ENTITY_ID)createToCellMailbox->getID();															// 在这个mailbox所在的cellspace上创建
	(*sp) << entityType;
	(*sp) << (ENTITY_ID)id;
	(*sp) << (COMPONENT_ID)m_componentID_;
	(*sp) << (uint8)hasClient;
	(*sp) << (uint32)cellDataLength;
	if(cellDataLength > 0)
		sp->append(strCellData.c_str(), cellDataLength);
	createToCellMailbox->post(sp);
	
	if(!hasClient)
		return;

	sp = new SocketPacket(OP_CREATE_CLIENT_PROXY);
	(*sp) << (uint8)false;	
	(*sp) << (ENTITY_ID)id;

	// 初始化cellEntity的位置和方向变量
	Vector3 v;
	PyObject* cellData = base->getCellData();
	PyObject* position = PyDict_GetItemString(cellData, "position");
	script::ScriptVector3::convertPyObjectToVector3(v, position);
	(*sp) << v.x << v.y << v.z;

	PyObject* direction = PyDict_GetItemString(cellData, "direction");
	script::ScriptVector3::convertPyObjectToVector3(v, direction);
	(*sp) << v.x << v.y << v.z;
	
	// 服务器已经确定要创建cell部分实体了， 我们可以从celldata获取客户端感兴趣的数据初始化客户端 如:ALL_CLIENTS
	base->getCellDataByFlags(ED_FLAG_ALL_CLIENTS|ED_FLAG_CELL_PUBLIC_AND_OWN|ED_FLAG_OWN_CLIENT, sp);
	clientMailbox->post(sp);
	*/
}

//-------------------------------------------------------------------------------------
void Baseapp::onDbmgrInitCompleted(Mercury::Channel* pChannel, 
		ENTITY_ID startID, ENTITY_ID endID, int32 startGlobalOrder, int32 startGroupOrder)
{
	EntityApp<Base>::onDbmgrInitCompleted(pChannel, startID, endID, startGlobalOrder, startGroupOrder);

	// 所有脚本都加载完毕
	PyObject* pyResult = PyObject_CallMethod(getEntryScript().get(), 
										const_cast<char*>("onBaseAppReady"), 
										const_cast<char*>("i"), 
										startGroupOrder == 1 ? 1 : 0);

	if(pyResult != NULL)
		Py_DECREF(pyResult);
	else
		SCRIPT_ERROR_CHECK();
}

//-------------------------------------------------------------------------------------
void Baseapp::onBroadcastGlobalBasesChange(Mercury::Channel* pChannel, std::string& key, std::string& value, bool isDelete)
{
	if(isDelete)
		pGlobalBases_->del(key);
	else
		pGlobalBases_->write(key, value);
}

//-------------------------------------------------------------------------------------

}
