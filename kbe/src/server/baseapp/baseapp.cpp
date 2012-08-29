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
#include "archiver.hpp"
#include "backup_sender.hpp"
#include "forward_message_over_handler.hpp"
#include "network/common.hpp"
#include "network/tcp_packet.hpp"
#include "network/udp_packet.hpp"
#include "network/fixed_messages.hpp"
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
	pendingLoginMgr_(ninterface),
	forward_messagebuffer_(ninterface),
	pBackupSender_()
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
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(),		createBase,						__py_createBase,					METH_VARARGS,			0);
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(),		createBaseLocally,				__py_createBase,					METH_VARARGS,			0);
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(),		createEntity,					__py_createBase,					METH_VARARGS,			0);
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(), 		createBaseAnywhere,				__py_createBaseAnywhere,			METH_VARARGS,			0);
	APPEND_SCRIPT_MODULE_METHOD(getScript().getModule(), 		executeRawDatabaseCommand,		__py_executeRawDatabaseCommand,		METH_VARARGS,			0);

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
	return true;
}

//-------------------------------------------------------------------------------------
void Baseapp::finalise()
{
	loopCheckTimerHandle_.cancel();
	EntityApp<Base>::finalise();
}

//-------------------------------------------------------------------------------------
void Baseapp::onChannelDeregister(Mercury::Channel * pChannel)
{
	ENTITY_ID pid = pChannel->proxyID();
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
		Components::ComponentInfos* cinfos = Components::getSingleton().findComponent(componentID);
		if(cinfos == NULL || cinfos->pChannel == NULL)
		{
			ForwardItem* pFI = new ForwardItem();
			pFI->pHandler = NULL;
			pFI->bundle.newMessage(BaseappInterface::onCreateBaseAnywhereCallback);
			pFI->bundle << callbackID;
			pFI->bundle << entityType;
			pFI->bundle << base->getID();
			pFI->bundle << componentID_;
			forward_messagebuffer_.push(componentID, pFI);
			WARNING_MSG("Baseapp::onCreateBaseAnywhere: not found baseapp, message is buffered.\n");
			return;
		}

		Mercury::Channel* lpChannel = cinfos->pChannel;

		// 需要baseappmgr转发给目的baseapp
		Mercury::Bundle forwardbundle;
		forwardbundle.newMessage(BaseappInterface::onCreateBaseAnywhereCallback);
		forwardbundle << callbackID;
		forwardbundle << entityType;
		forwardbundle << base->getID();
		forwardbundle << componentID_;
		forwardbundle.send(this->getNetworkInterface(), lpChannel);
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
			ERROR_MSG("Baseapp::onCreateBaseAnywhereCallback: can't found entityType:%s.\n", entityType.c_str());
			Py_DECREF(pyargs);
			return;
		}
		
		// 如果entity属于另一个baseapp创建则设置它的mailbox
		Mercury::Channel* pOtherBaseappChannel = Components::getSingleton().findComponent(componentID)->pChannel;
		KBE_ASSERT(pOtherBaseappChannel != NULL);
		PyObject* mb = static_cast<EntityMailbox*>(new EntityMailbox(sm, NULL, componentID, eid, MAILBOX_TYPE_BASE));
		PyTuple_SET_ITEM(pyargs, 0, mb);
		PyObject_CallObject(pyCallback.get(), pyargs);
		//Py_DECREF(mb);
		int i=0;
		i++;
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
		PyObject_CallObject(pyCallback.get(), pyargs);
	}

	SCRIPT_ERROR_CHECK();
	Py_DECREF(pyargs);
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

	if(createToCellMailbox->getChannel() == NULL)
	{
		ERROR_MSG("Baseapp::createCellEntity: not found cellapp(createToCellMailbox:componentID=%"PRAppID", entityID=%d), create is error!\n", 
			createToCellMailbox->getComponentID(), createToCellMailbox->getID());
		return;
	}

	bundle.send(this->getNetworkInterface(), createToCellMailbox->getChannel());
}

//-------------------------------------------------------------------------------------
void Baseapp::onEntityGetCell(Mercury::Channel* pChannel, ENTITY_ID id, COMPONENT_ID componentID, SPACE_ID spaceID)
{
	if(pChannel->isExternal())
		return;

	Base* base = pEntities_->find(id);
	base->setSpaceID(spaceID);

	// DEBUG_MSG("Baseapp::onEntityGetCell: entityID %d.\n", id);
	KBE_ASSERT(base != NULL);

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

	Mercury::Bundle bundle;
	bundle.newMessage(ClientInterface::onEntityEnterWorld);
	bundle << base->getID();
	bundle << base->getSpaceID();
	base->getClientMailbox()->postMail(bundle);
}

//-------------------------------------------------------------------------------------
bool Baseapp::createClientProxies(Proxy* base, bool reload)
{
	// 将通道代理的关系与该entity绑定， 在后面通信中可提供身份合法性识别
	base->getClientMailbox()->getChannel()->proxyID(base->getID());
	
	// 重新生成一个ID
	if(reload)
		base->rndUUID(genUUID64());

	// 让客户端知道已经创建了proxices, 并初始化一部分属性
	Mercury::Bundle bundle;
	bundle.newMessage(ClientInterface::onCreatedProxies);
	bundle << base->rndUUID();
	bundle << base->getID();
	bundle << base->ob_type->tp_name;
	base->getClientMailbox()->postMail(bundle);

	base->initClientBasePropertys();

	// 本应该由客户端告知已经创建好entity后调用这个接口。
	if(!reload)
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

	DEBUG_MSG("KBEngine::executeRawDatabaseCommand:%s.\n", datas);

	Mercury::Bundle bundle;
	bundle.newMessage(DbmgrInterface::executeRawDatabaseCommand);
	bundle << componentID_ << componentType_;

	CALLBACK_ID callbackID = 0;

	if(pycallback && PyCallable_Check(pycallback))
		callbackID = callbackMgr().save(pycallback);

	bundle << callbackID;
	bundle << size;
	bundle.append(datas, size);
	bundle.send(this->getNetworkInterface(), dbmgrinfos->pChannel);
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

	DEBUG_MSG("Baseapp::onExecuteRawDatabaseCommandCB: nrows=%u, nfields=%u, err=%s.\n", nrows, nfields, err.c_str());

	if(callbackID > 0)
	{
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
void Baseapp::registerPendingLogin(Mercury::Channel* pChannel, std::string& accountName, std::string& password, ENTITY_ID entityID)
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
	ptinfos->entityID = entityID;
	pendingLoginMgr_.add(ptinfos);
}

//-------------------------------------------------------------------------------------
void Baseapp::loginGatewayFailed(Mercury::Channel* pChannel, std::string& accountName, MERCURY_ERROR_CODE failedcode)
{
	if(failedcode == MERCURY_ERR_NAME)
	{
		DEBUG_MSG("Baseapp::login: not found user[%s], login is failed!\n", accountName.c_str());
		failedcode = MERCURY_ERR_NAME_PASSWORD;
	}
	else if(failedcode == MERCURY_ERR_PASSWORD)
	{
		DEBUG_MSG("Baseapp::login: user[%s] password is error, login is failed!\n", accountName.c_str());
		failedcode = MERCURY_ERR_NAME_PASSWORD;
	}

	if(pChannel == NULL)
		return;

	Mercury::Bundle bundle;
	bundle.newMessage(ClientInterface::onLoginGatewayFailed);
	ClientInterface::onLoginGatewayFailedArgs1::staticAddToBundle(bundle, failedcode);
	bundle.send(this->getNetworkInterface(), pChannel);
}

//-------------------------------------------------------------------------------------
void Baseapp::loginGateway(Mercury::Channel* pChannel, std::string& accountName, std::string& password)
{
	DEBUG_MSG("Baseapp::loginGateway: new user[%s].\n", accountName.c_str());

	Components::COMPONENTS cts = Components::getSingleton().getComponents(DBMGR_TYPE);
	Components::ComponentInfos* dbmgrinfos = NULL;

	if(cts.size() > 0)
		dbmgrinfos = &(*cts.begin());

	if(dbmgrinfos == NULL || dbmgrinfos->pChannel == NULL || dbmgrinfos->cid == 0)
	{
		loginGatewayFailed(pChannel, accountName, MERCURY_ERR_SRV_NO_READY);
		return;
	}

	PendingLoginMgr::PLInfos* ptinfos = pendingLoginMgr_.find(accountName);
	if(ptinfos == NULL)
	{
		loginGatewayFailed(pChannel, accountName, MERCURY_ERR_NAME);
		return;
	}

	if(ptinfos->password != password)
	{
		loginGatewayFailed(pChannel, accountName, MERCURY_ERR_PASSWORD);
		return;
	}

	// 如果entityID大于0则说明此entity是存活状态登录
	if(ptinfos->entityID > 0)
	{
		DEBUG_MSG("Baseapp::loginGateway: user[%s] has entity(%d).\n", accountName.c_str(), ptinfos->entityID);
		Proxy* base = static_cast<Proxy*>(findEntity(ptinfos->entityID));
		if(base == NULL)
		{
			loginGatewayFailed(pChannel, accountName, MERCURY_ERR_SRV_NO_READY);
			return;
		}
		
		if(base->getClientMailbox() == NULL)
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
		else
		{
			// 通知脚本异常登录请求有脚本决定是否允许这个通道强制登录
			int32 ret = base->onLogOnAttempt(inet_ntoa((struct in_addr&)pChannel->addr().ip), 
				ntohs(pChannel->addr().port), password.c_str());

			switch(ret)
			{
			case LOG_ON_ACCEPT:
				DEBUG_MSG("Baseapp::loginGateway: script LOG_ON_ACCEPT.\n");
				if(base->getClientMailbox() != NULL)
				{
					// 通告在别处登录
					Mercury::Channel* pOldClientChannel = base->getClientMailbox()->getChannel();
					if(pOldClientChannel != NULL)
					{
						loginGatewayFailed(pOldClientChannel, accountName, MERCURY_ERR_ANOTHER_LOGON);
						pOldClientChannel->proxyID(0);
					}

					base->getClientMailbox()->addr(pChannel->addr());
					base->addr(pChannel->addr());
					createClientProxies(base, true);
				}
			case LOG_ON_WAIT_FOR_DESTROY:
			default:
				DEBUG_MSG("Baseapp::loginGateway: script LOG_ON_REJECT.\n");
				loginGatewayFailed(pChannel, accountName, MERCURY_ERR_ACCOUNT_ONLINE);
				return;
			};
		}
	}
	else
	{
		Mercury::Bundle bundle;
		bundle.newMessage(DbmgrInterface::queryAccount);
		DbmgrInterface::queryAccountArgs2::staticAddToBundle(bundle, accountName, password);
		bundle.send(this->getNetworkInterface(), dbmgrinfos->pChannel);
	}

	// 记录客户端地址
	ptinfos->addr = pChannel->addr();
}

//-------------------------------------------------------------------------------------
void Baseapp::reLoginGateway(Mercury::Channel* pChannel, std::string& accountName, 
							 std::string& password, uint64 key, ENTITY_ID entityID)
{
	DEBUG_MSG("Baseapp::reLoginGateway: accountName=%s, key="PRIu64", entityID=%d.\n", 
		accountName.c_str(), key, entityID);

	Base* base = findEntity(entityID);
	if(base == NULL || !PyObject_TypeCheck(base, Proxy::getScriptType()))
	{
		loginGatewayFailed(pChannel, accountName, MERCURY_ERR_ILLEGAL_LOGIN);
		return;
	}
	
	Proxy* proxy = static_cast<Proxy*>(base);
	
	if(proxy->rndUUID() != key)
	{
		loginGatewayFailed(pChannel, accountName, MERCURY_ERR_ILLEGAL_LOGIN);
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
		WARNING_MSG("Baseapp::reLoginGateway: accountName=%s, key="PRIu64", entityID=%d, ClientMailbox is exist.\n", 
			accountName.c_str(), key, entityID);
	}
}

//-------------------------------------------------------------------------------------
void Baseapp::onQueryAccountCBFromDbmgr(Mercury::Channel* pChannel, 
										std::string& accountName, std::string& password, std::string& datas)
{
	if(pChannel->isExternal())
		return;

	Proxy* base = static_cast<Proxy*>(createEntityCommon(g_serverConfig.getDBMgr().dbAccountEntityScriptType, NULL));
	PendingLoginMgr::PLInfos* ptinfos = pendingLoginMgr_.remove(accountName);
	Mercury::Channel* pClientChannel = this->getNetworkInterface().findChannel(ptinfos->addr);

	KBE_ASSERT(base != NULL && ptinfos != NULL);
	base->hasDB(true);

	if(pClientChannel != NULL)
	{
		// 创建entity的客户端mailbox
		EntityMailbox* entityClientMailbox = new EntityMailbox(base->getScriptModule(), 
			&pClientChannel->addr(), 0, base->getID(), MAILBOX_TYPE_CLIENT);

		base->setClientMailbox(entityClientMailbox);
		base->addr(pClientChannel->addr());

		createClientProxies(base);

		Mercury::Bundle bundle;
		bundle.newMessage(DbmgrInterface::onAccountOnline);
		DbmgrInterface::onAccountOnlineArgs3::staticAddToBundle(bundle, accountName, componentID_, base->getID());
		bundle.send(this->getNetworkInterface(), pChannel);
	}

	DEBUG_MSG("Baseapp::onQueryAccountCBFromDbmgr: user[%s], uuid[%"PRIu64"], entityID=%d.\n", 
		accountName.c_str(), base->rndUUID(), base->getID());

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
		Mercury::Bundle bundle;
		bundle.newMessage(ClientInterface::onEntityEnterWorld);
		ClientInterface::onEntityEnterWorldArgs2::staticAddToBundle(bundle, entityID, base->getSpaceID());
		bundle.send(this->getNetworkInterface(), pClientChannel);
	}
}

//-------------------------------------------------------------------------------------
void Baseapp::onEntityLeaveWorldFromCellapp(Mercury::Channel* pChannel, ENTITY_ID entityID)
{
	if(pChannel->isExternal())
		return;

	Proxy* base = static_cast<Proxy*>(pEntities_->find(entityID));
	// DEBUG_MSG("Baseapp::onEntityEnterWorldFromCellapp: entityID %d.\n", entityID);
	KBE_ASSERT(base != NULL);

	Mercury::Channel* pClientChannel = this->getNetworkInterface().findChannel(base->addr());
	if(pClientChannel)
	{
		Mercury::Bundle bundle;
		bundle.newMessage(ClientInterface::onEntityLeaveWorld);
		ClientInterface::onEntityLeaveWorldArgs2::staticAddToBundle(bundle, entityID, base->getSpaceID());
		bundle.send(this->getNetworkInterface(), pClientChannel);
	}
}

//-------------------------------------------------------------------------------------
void Baseapp::onEntityEnterSpaceFromCellapp(Mercury::Channel* pChannel, ENTITY_ID entityID, SPACE_ID spaceID)
{
	if(pChannel->isExternal())
		return;
}

//-------------------------------------------------------------------------------------
void Baseapp::onEntityLeaveSpaceFromCellapp(Mercury::Channel* pChannel, ENTITY_ID entityID, SPACE_ID spaceID)
{
	if(pChannel->isExternal())
		return;
}

//-------------------------------------------------------------------------------------
void Baseapp::onEntityMail(Mercury::Channel* pChannel, KBEngine::MemoryStream& s)
{
	if(pChannel->isExternal())
		return;
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
		return;

	Mercury::Bundle bundle;
	bundle.newMessage(CellappInterface::onRemoteCallMethodFromClient);
	bundle << srcEntityID;
	bundle.append(s.data(), s.opsize());
	
	e->getCellMailbox()->postMail(bundle);
}

//-------------------------------------------------------------------------------------
void Baseapp::onBackupEntityCellData(Mercury::Channel* pChannel, KBEngine::MemoryStream& s)
{
	ENTITY_ID baseID = 0;
	s >> baseID;
	
	ERROR_MSG("Baseapp::onBackupEntityCellData: entityID=%d, size=%u.\n", baseID, s.opsize());

	Base* base = this->findEntity(baseID);

	if(base)
	{
		base->onBackupCellData(pChannel, s);
	}
	else
	{
		ERROR_MSG("Baseapp::onBackupEntityCellData: not found entityID=%d\n", baseID);
	}
}

//-------------------------------------------------------------------------------------
void Baseapp::onCellWriteToDBCompleted(Mercury::Channel* pChannel, KBEngine::MemoryStream& s)
{
	ENTITY_ID baseID = 0;
	s >> baseID;
	
	ERROR_MSG("Baseapp::onCellWriteToDBCompleted: entityID=%d, size=%u.\n", baseID, s.opsize());

	Base* base = this->findEntity(baseID);

	if(base)
	{
		base->onCellWriteToDBCompleted();
	}
	else
	{
		ERROR_MSG("Baseapp::onCellWriteToDBCompleted: not found entityID=%d\n", baseID);
	}
}

//-------------------------------------------------------------------------------------

}
