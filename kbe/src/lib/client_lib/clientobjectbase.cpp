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

#include "entity.hpp"
#include "config.hpp"
#include "clientobjectbase.hpp"
#include "pyscript/pywatcher.hpp"
#include "network/channel.hpp"
#include "network/fixed_messages.hpp"
#include "network/common.hpp"
#include "network/message_handler.hpp"
#include "entitydef/scriptdef_module.hpp"
#include "entitydef/entity_mailbox.hpp"
#include "entitydef/entitydef.hpp"

#include "../../server/baseapp/baseapp_interface.hpp"
#include "../../server/loginapp/loginapp_interface.hpp"

namespace KBEngine{

SCRIPT_METHOD_DECLARE_BEGIN(ClientObjectBase)
SCRIPT_DIRECT_METHOD_DECLARE("getSpaceData",		__py_GetSpaceData,			0,					0)
SCRIPT_DIRECT_METHOD_DECLARE("callback",			__py_callback,				0,					0)
SCRIPT_DIRECT_METHOD_DECLARE("cancelCallback",		__py_cancelCallback,		0,					0)
SCRIPT_DIRECT_METHOD_DECLARE("getWatcher",			__py_getWatcher,			0,					0)
SCRIPT_DIRECT_METHOD_DECLARE("getWatcherDir",		__py_getWatcherDir,			0,					0)
SCRIPT_METHOD_DECLARE_END()

SCRIPT_MEMBER_DECLARE_BEGIN(ClientObjectBase)
SCRIPT_MEMBER_DECLARE_END()

SCRIPT_GETSET_DECLARE_BEGIN(ClientObjectBase)
SCRIPT_GET_DECLARE("id",							pyGetID,					0,					0)
SCRIPT_GET_DECLARE("entities",						pyGetEntities,				0,					0)
SCRIPT_GETSET_DECLARE_END()
SCRIPT_INIT(ClientObjectBase, 0, 0, 0, 0, 0)		

static int32 g_appID = 1;

//-------------------------------------------------------------------------------------
ClientObjectBase::ClientObjectBase(Mercury::NetworkInterface& ninterface, PyTypeObject* pyType):
ScriptObject(pyType != NULL ? pyType : getScriptType(), false),
appID_(0),
pServerChannel_(new Mercury::Channel()),
pEntities_(new Entities<client::Entity>()),
pEntityIDAliasIDList_(),
pyCallbackMgr_(),
entityID_(0),
spaceID_(0),
entityPos_(FLT_MAX, FLT_MAX, FLT_MAX),
entityDir_(FLT_MAX, FLT_MAX, FLT_MAX),
dbid_(0),
ip_(),
port_(),
lastSentActiveTickTime_(timestamp()),
lastSentUpdateDataTime_(timestamp()),
connectedGateway_(false),
canReset_(false),
name_(),
password_(),
extradatas_("unknown"),
typeClient_(CLIENT_TYPE_PC),
bufferedCreateEntityMessage_(),
eventHandler_(),
ninterface_(ninterface),
targetID_(0),
isLoadedGeometry_(false),
timers_(),
scriptCallbacks_(timers_)
{
	pServerChannel_->incRef();
	appID_ = g_appID++;

	pServerChannel_->pNetworkInterface(&ninterface);
}

//-------------------------------------------------------------------------------------
ClientObjectBase::~ClientObjectBase()
{
}

//-------------------------------------------------------------------------------------		
void ClientObjectBase::finalise(void)
{
	pyCallbackMgr_.finalise();

	if(pEntities_)
	{
		pEntities_->finalise();
		S_RELEASE(pEntities_);
	}

	if(pServerChannel_)
	{
		pServerChannel_->destroy();
		pServerChannel_->decRef();
		pServerChannel_ = NULL;
	}
}

//-------------------------------------------------------------------------------------		
void ClientObjectBase::reset(void)
{
	pEntities_->finalise();
	pEntityIDAliasIDList_.clear();
	pyCallbackMgr_.finalise();
	entityID_ = 0;
	dbid_ = 0;
	ip_ = "";
	port_ = 0;
	lastSentActiveTickTime_ = 0;
	connectedGateway_ = false;
	name_ = "";
	password_ = "";
	extradatas_ = "";
	bufferedCreateEntityMessage_.clear();
	canReset_ = false;

	if(pServerChannel_)
	{
		pServerChannel_->destroy();
		pServerChannel_->decRef();
		pServerChannel_ = NULL;
	}

	pServerChannel_ = new Mercury::Channel();
	pServerChannel_->pNetworkInterface(&ninterface_);
	pServerChannel_->incRef();
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::tickSend()
{
	if(!pServerChannel_ || !pServerChannel_->endpoint())
		return;
	
	if(pServerChannel_ && pServerChannel_->isDestroyed())
	{
		if(connectedGateway_)
		{
			EventData_ServerCloased eventdata;
			eventHandler_.fire(&eventdata);
			connectedGateway_ = false;
			canReset_ = true;

			DEBUG_MSG("ClientObjectBase::tickSend: serverCloased!\n");
		}

		return;
	}

	handleTimers();

	// 向服务器发送tick
	uint64 check = uint64( Mercury::g_channelExternalTimeout * stampsPerSecond() ) / 2;
	if (timestamp() - lastSentActiveTickTime_ > check)
	{
		lastSentActiveTickTime_ = timestamp();

		Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
		if(connectedGateway_)
			(*pBundle).newMessage(BaseappInterface::onClientActiveTick);
		else
			(*pBundle).newMessage(LoginappInterface::onClientActiveTick);

		pServerChannel_->pushBundle(pBundle);
	}

	updatePlayerToServer();
	pServerChannel_->send();
}

//-------------------------------------------------------------------------------------	
bool ClientObjectBase::destroyEntity(ENTITY_ID entityID, bool callScript)
{
	PyObjectPtr entity = pEntities_->erase(entityID);
	if(entity != NULL)
	{
		static_cast<client::Entity*>(entity.get())->destroy(callScript);
		return true;
	}

	ERROR_MSG(boost::format("EntityApp::destroyEntity: not found %1%!\n") % entityID);
	return false;
}

//-------------------------------------------------------------------------------------	
Mercury::Channel* ClientObjectBase::findChannelByMailbox(EntityMailbox& mailbox)
{
	return pServerChannel_;
}

//-------------------------------------------------------------------------------------	
void ClientObjectBase::onKicked(Mercury::Channel * pChannel, SERVER_ERROR_CODE failedcode)
{
	INFO_MSG(boost::format("ClientObjectBase::onKicked: code=%1%\n") % failedcode);

	EventData_onKicked eventdata;
	eventdata.failedcode = failedcode;
	eventHandler_.fire(&eventdata);

#ifdef unix
	::close(*pChannel->endpoint());
#elif defined(PLAYSTATION3)
	::socketclose(*pChannel->endpoint());
#else
	::closesocket(*pChannel->endpoint());
#endif
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::handleTimers()
{
	timers().process(g_kbetime);
}

//-------------------------------------------------------------------------------------	
PyObject* ClientObjectBase::__py_callback(PyObject* self, PyObject* args)
{
	if(PyTuple_Size(args) != 2)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::callback: (argssize != (time, callback)) is error!");
		PyErr_PrintEx(0);
		return 0;
	}
	
	float time = 0;
	PyObject* pyCallback = NULL;

	if(PyArg_ParseTuple(args, "f|O",  &time, &pyCallback) == -1)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::callback: args is error!");
		PyErr_PrintEx(0);
		return 0;
	}

	if(!PyCallable_Check(pyCallback))
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::callback: invalid pycallback!");
		PyErr_PrintEx(0);
		return NULL;
	}
	
	ClientObjectBase* pClientObjectBase = static_cast<ClientObjectBase*>(self);
	Py_INCREF(pyCallback);
	ScriptID id = pClientObjectBase->scriptCallbacks().addCallback(time, new ScriptCallbackHandler(pClientObjectBase->scriptCallbacks(), pyCallback));
	return PyLong_FromLong(id);
}

//-------------------------------------------------------------------------------------	
PyObject* ClientObjectBase::__py_cancelCallback(PyObject* self, PyObject* args)
{
	if(PyTuple_Size(args) != 1)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::cancelCallback: (argssize != (callbackID)) is error!");
		PyErr_PrintEx(0);
		return 0;
	}
	
	ClientObjectBase* pClientObjectBase = static_cast<ClientObjectBase*>(self);

	ScriptID id = 0;

	if(PyArg_ParseTuple(args, "i",  &id) == -1)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::cancelCallback: args is error!");
		PyErr_PrintEx(0);
		return 0;
	}

	pClientObjectBase->scriptCallbacks().delCallback(id);
	S_Return;
}

//-------------------------------------------------------------------------------------	
client::Entity* ClientObjectBase::createEntityCommon(const char* entityType, PyObject* params,
	bool isInitializeScript, ENTITY_ID eid, bool initProperty,
	EntityMailbox* base, EntityMailbox* cell)
{
	KBE_ASSERT(eid > 0);

	ScriptDefModule* sm = EntityDef::findScriptModule(entityType);
	if(sm == NULL)
	{
		PyErr_Format(PyExc_TypeError, "ClientObjectBase::createEntityCommon: entity [%s] not found.\n", 
			entityType);

		PyErr_PrintEx(0);
		return NULL;
	}
	else if(!sm->hasClient())
	{
		PyErr_Format(PyExc_TypeError, "ClientObjectBase::createEntityCommon: entity [%s] not found.\n", 
			entityType);

		PyErr_PrintEx(0);
		return NULL;
	}

	PyObject* obj = sm->createObject();
	
	client::Entity* entity = new(obj) client::Entity(eid, sm, base, cell);

	entity->pClientApp(this);

	if(initProperty)
		entity->initProperty();

	// 将entity加入entities
	pEntities_->add(eid, entity); 

	// 初始化脚本
	if(isInitializeScript)
		entity->initializeEntity(params);

	SCRIPT_ERROR_CHECK();

	if(g_debugEntity)
	{
		INFO_MSG(boost::format("ClientObjectBase::createEntityCommon: new %1% (%2%) refc=%3%.\n") % 
			entityType % eid % obj->ob_refcnt);
	}
	else
	{
		INFO_MSG(boost::format("ClientObjectBase::createEntityCommon: new %1% (%2%)\n") % 
			entityType % eid);
	}

	EventData_CreatedEntity eventdata;
	eventdata.entityID = entity->getID();
	//eventdata.modelres = entity->getAspect()->modelres();
	eventdata.modelScale = entity->getAspect()->modelScale();
	eventHandler_.fire(&eventdata);
	
	return entity;
}

//-------------------------------------------------------------------------------------
ENTITY_ID ClientObjectBase::getAoiEntityID(ENTITY_ID id)
{
	if(id <= 255 && EntityDef::entityAliasID() && pEntityIDAliasIDList_.size() <= 255)
	{
		return pEntityIDAliasIDList_[id];
	}

	return id;
}

//-------------------------------------------------------------------------------------
ENTITY_ID ClientObjectBase::getAoiEntityIDFromStream(MemoryStream& s)
{
	ENTITY_ID id = 0;
	if(EntityDef::entityAliasID() && 
		pEntityIDAliasIDList_.size() > 0 && pEntityIDAliasIDList_.size() <= 255)
	{
		uint8 aliasID = 0;
		s >> aliasID;
		id = pEntityIDAliasIDList_[aliasID];
	}
	else
	{
		s >> id;
	}

	return id;
}

//-------------------------------------------------------------------------------------
ENTITY_ID ClientObjectBase::getAoiEntityIDByAliasID(uint8 id)
{
	return pEntityIDAliasIDList_[id];
}

//-------------------------------------------------------------------------------------
bool ClientObjectBase::registerEventHandle(EventHandle* pEventHandle)
{
	return eventHandler_.registerHandle(pEventHandle);
}

//-------------------------------------------------------------------------------------
bool ClientObjectBase::deregisterEventHandle(EventHandle* pEventHandle)
{
	return eventHandler_.deregisterHandle(pEventHandle);
}

//-------------------------------------------------------------------------------------
bool ClientObjectBase::createAccount()
{
	// 创建账号
	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
	(*pBundle).newMessage(LoginappInterface::reqCreateAccount);
	(*pBundle) << name_;
	(*pBundle) << password_;

	pServerChannel_->pushBundle(pBundle);
	return true;
}

//-------------------------------------------------------------------------------------
Mercury::Channel* ClientObjectBase::initLoginappChannel(std::string accountName, std::string passwd, std::string ip, KBEngine::uint32 port)
{
	Mercury::EndPoint* pEndpoint = new Mercury::EndPoint();
	
	pEndpoint->socket(SOCK_STREAM);
	if (!pEndpoint->good())
	{
		ERROR_MSG("ClientObjectBase::initLoginappChannel: couldn't create a socket\n");
		delete pEndpoint;
		return NULL;
	}
	
	u_int32_t address;
	pEndpoint->convertAddress(ip.c_str(), address);
	if(pEndpoint->connect(htons(port), address) == -1)
	{
		ERROR_MSG(boost::format("ClientObjectBase::initLoginappChannel: connect server is error(%1%)!\n") %
			kbe_strerror());

		delete pEndpoint;
		return NULL;
	}

	Mercury::Address addr(ip.c_str(), port);
	pEndpoint->addr(addr);

	pServerChannel_->endpoint(pEndpoint);
	pEndpoint->setnonblocking(true);
	pEndpoint->setnodelay(true);

	password_ = passwd;
	name_ = accountName;
	return pServerChannel_;
}

//-------------------------------------------------------------------------------------
Mercury::Channel* ClientObjectBase::initBaseappChannel()
{
	Mercury::EndPoint* pEndpoint = new Mercury::EndPoint();
	
	pEndpoint->socket(SOCK_STREAM);
	if (!pEndpoint->good())
	{
		ERROR_MSG("ClientObjectBase::initBaseappChannel: couldn't create a socket\n");
		delete pEndpoint;
		return NULL;
	}
	
	u_int32_t address;

	pEndpoint->convertAddress(ip_.c_str(), address);
	if(pEndpoint->connect(htons(port_), address) == -1)
	{
		ERROR_MSG(boost::format("ClientObjectBase::initBaseappChannel: connect server is error(%1%)!\n") %
			kbe_strerror());

		delete pEndpoint;
		return NULL;
	}

	Mercury::Address addr(ip_.c_str(), port_);
	pEndpoint->addr(addr);

	pServerChannel_->endpoint(pEndpoint);
	pEndpoint->setnonblocking(true);
	pEndpoint->setnodelay(true);

	connectedGateway_ = true;
	return pServerChannel_;
}


//-------------------------------------------------------------------------------------	
void ClientObjectBase::fireEvent(const EventData* pEventData)
{
	eventHandler_.fire(pEventData);
}

//-------------------------------------------------------------------------------------	
void ClientObjectBase::onHelloCB_(Mercury::Channel* pChannel, const std::string& verInfo, 
		COMPONENT_TYPE componentType)
{
}

//-------------------------------------------------------------------------------------	
void ClientObjectBase::onHelloCB(Mercury::Channel* pChannel, MemoryStream& s)
{
	std::string verInfo;
	s >> verInfo;
	
	COMPONENT_TYPE ctype;
	s >> ctype;

	INFO_MSG(boost::format("ClientObjectBase::onHelloCB: verInfo=%1%, addr:%2%\n") % 
		verInfo % pChannel->c_str());

	onHelloCB_(pChannel, verInfo, ctype);
}

//-------------------------------------------------------------------------------------	
void ClientObjectBase::onVersionNotMatch(Mercury::Channel* pChannel, MemoryStream& s)
{
	std::string verInfo;
	s >> verInfo;
	
	INFO_MSG(boost::format("ClientObjectBase::onVersionNotMatch: verInfo=%1% not match(server:%2%)\n") % 
		KBEVersion::versionString() % verInfo);

	EventData_VersionNotMatch eventdata;
	eventdata.verInfo = KBEVersion::versionString();
	eventdata.serVerInfo = verInfo;
	eventHandler_.fire(&eventdata);
}

//-------------------------------------------------------------------------------------
bool ClientObjectBase::login()
{
	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();

	// 提交账号密码请求登录
	(*pBundle).newMessage(LoginappInterface::login);
	(*pBundle) << typeClient_;
	(*pBundle).appendBlob(extradatas_);
	(*pBundle) << name_;
	(*pBundle) << password_;
	(*pBundle) << EntityDef::md5().getDigestStr();
	pServerChannel_->pushBundle(pBundle);
	connectedGateway_ = false;
	return true;
}

//-------------------------------------------------------------------------------------
bool ClientObjectBase::loginGateWay()
{
	// 请求登录网关
	connectedGateway_ = false;
	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
	(*pBundle).newMessage(BaseappInterface::loginGateway);
	(*pBundle) << name_;
	(*pBundle) << password_;
	pServerChannel_->pushBundle(pBundle);
	return true;
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onCreateAccountResult(Mercury::Channel * pChannel, MemoryStream& s)
{
	SERVER_ERROR_CODE retcode;

	s >> retcode;
	s.readBlob(extradatas_);

	if(retcode != 0)
	{
		INFO_MSG(boost::format("ClientObjectBase::onCreateAccountResult: %1% create is failed! code=%2%.\n") % 
			name_ % retcode);

		return;
	}

	INFO_MSG(boost::format("ClientObjectBase::onCreateAccountResult: %1% create is successfully!\n") % name_);
}

//-------------------------------------------------------------------------------------	
void ClientObjectBase::onLoginSuccessfully(Mercury::Channel * pChannel, MemoryStream& s)
{
	std::string accountName;

	s >> accountName;
	s >> ip_;
	s >> port_;
	s.readBlob(extradatas_);
	
	connectedGateway_ = false;
	INFO_MSG(boost::format("ClientObjectBase::onLoginSuccessfully: %1% addr=%2%:%3%!\n") % name_ % ip_ % port_);

	EventData_LoginSuccess eventdata;
	eventHandler_.fire(&eventdata);
}

//-------------------------------------------------------------------------------------	
void ClientObjectBase::onLoginFailed(Mercury::Channel * pChannel, MemoryStream& s)
{
	SERVER_ERROR_CODE failedcode;

	s >> failedcode;
	s.readBlob(extradatas_);
	
	connectedGateway_ = false;
	INFO_MSG(boost::format("ClientObjectBase::onLoginFailed: %1% failedcode=%2%!\n") % name_ % failedcode);
	EventData_LoginFailed eventdata;
	eventHandler_.fire(&eventdata);
}

//-------------------------------------------------------------------------------------	
void ClientObjectBase::onLoginGatewayFailed(Mercury::Channel * pChannel, SERVER_ERROR_CODE failedcode)
{
	INFO_MSG(boost::format("ClientObjectBase::onLoginGatewayFailed: %1% failedcode=%2%!\n") % name_ % failedcode);
	connectedGateway_ = false;

	EventData_LoginGatewayFailed eventdata;
	eventHandler_.fire(&eventdata);
}

//-------------------------------------------------------------------------------------	
void ClientObjectBase::onCreatedProxies(Mercury::Channel * pChannel, uint64 rndUUID, ENTITY_ID eid, std::string& entityType)
{
	if(entityID_ == 0)
	{
		EventData_LoginGatewaySuccess eventdata;
		eventHandler_.fire(&eventdata);
	}

	connectedGateway_ = true;
	entityID_ = eid;
	INFO_MSG(boost::format("ClientObject::onCreatedProxies(%1%): rndUUID=%2% eid=%3% entityType=%4%!\n") % 
		name_ % rndUUID % eid % entityType);

	// 设置entity的baseMailbox
	EntityMailbox* mailbox = new EntityMailbox(EntityDef::findScriptModule(entityType.c_str()), 
		NULL, appID(), eid, MAILBOX_TYPE_BASE);

	createEntityCommon(entityType.c_str(), NULL, true, eid, true, mailbox, NULL);
}

//-------------------------------------------------------------------------------------	
void ClientObjectBase::onEntityEnterWorld(Mercury::Channel * pChannel, MemoryStream& s)
{
	ENTITY_ID eid = 0;
	ENTITY_SCRIPT_UID scriptType;
	int8 isOnGound = 0;

	s >> eid;

	if(EntityDef::scriptModuleAliasID())
	{
		ENTITY_DEF_ALIASID aliasID;
		s >> aliasID;
		scriptType = aliasID;
	}
	else
	{
		s >> scriptType;
	}

	if(s.opsize() > 0)
		s >> isOnGound;

	if(eid != entityID_ && entityID_ > 0)
		pEntityIDAliasIDList_.push_back(eid);

	client::Entity* entity = pEntities_->find(eid);
	if(entity == NULL)
	{	
		BUFFEREDMESSAGE::iterator iter = bufferedCreateEntityMessage_.find(eid);
		if(iter != bufferedCreateEntityMessage_.end())
		{
			ScriptDefModule* sm = EntityDef::findScriptModule(scriptType);
			KBE_ASSERT(sm);
			
			// 设置entity的cellMailbox
			EntityMailbox* mailbox = new EntityMailbox(EntityDef::findScriptModule(sm->getName()), 
				NULL, appID(), eid, MAILBOX_TYPE_CELL);

			entity = createEntityCommon(sm->getName(), NULL, true, eid, true, NULL, mailbox);

			this->onUpdatePropertys(pChannel, *iter->second.get());
			bufferedCreateEntityMessage_.erase(iter);
			entity->isOnGound(isOnGound > 0);
		}
		else
		{
			ERROR_MSG(boost::format("ClientObjectBase::onEntityEnterWorld: not found entity(%1%).\n") % eid);
			return;
		}
	}
	else
	{
		entity->isOnGound(isOnGound > 0);
		entityPos_ = entity->getPosition();
		entityDir_ = entity->getDirection();

		// 初始化一下服务端当前的位置
		entity->setServerPosition(entity->getPosition());

		KBE_ASSERT(!entity->isEnterword() && entity->getCellMailbox() == NULL);
		
		// 设置entity的cellMailbox
		EntityMailbox* mailbox = new EntityMailbox(entity->getScriptModule(), 
			NULL, appID(), eid, MAILBOX_TYPE_CELL);

		entity->setCellMailbox(mailbox);
	}

	DEBUG_MSG(boost::format("ClientObjectBase::onEntityEnterWorld: %1%(%2%), isOnGound(%3%).\n") % 
		entity->getScriptName() % eid  % (int)isOnGound);

	EventData_EnterWorld eventdata;
	eventdata.spaceID = spaceID_;
	eventdata.entityID = entity->getID();
	eventdata.x = entity->getPosition().x;
	eventdata.y = entity->getPosition().y;
	eventdata.z = entity->getPosition().z;
	eventdata.pitch = entity->getDirection().pitch();
	eventdata.roll = entity->getDirection().roll();
	eventdata.yaw = entity->getDirection().yaw();
	eventdata.speed = entity->getMoveSpeed();
	eventdata.isOnGound = isOnGound > 0;
	eventHandler_.fire(&eventdata);

	if(entityID_ == eid)
	{
		entity->onBecomePlayer();
	}
	
	entity->onEnterWorld();
}

//-------------------------------------------------------------------------------------	
void ClientObjectBase::onEntityLeaveWorldOptimized(Mercury::Channel * pChannel, MemoryStream& s)
{
	onEntityLeaveWorld(pChannel, getAoiEntityIDFromStream(s));
}

//-------------------------------------------------------------------------------------	
void ClientObjectBase::onEntityLeaveWorld(Mercury::Channel * pChannel, ENTITY_ID eid)
{
	client::Entity* entity = pEntities_->find(eid);
	if(entity == NULL)
	{	
		ERROR_MSG(boost::format("ClientObjectBase::onEntityLeaveWorld: not found entity(%1%).\n") % eid);
		return;
	}

	DEBUG_MSG(boost::format("ClientObjectBase::onEntityLeaveWorld: %1%(%2%).\n") % 
		entity->getScriptName() % eid);

	EventData_LeaveWorld eventdata;
	eventdata.spaceID = entity->getSpaceID();
	eventdata.entityID = entity->getID();

	entity->onLeaveWorld();

	eventHandler_.fire(&eventdata);

	// 如果不是玩家
	if(entityID_ != eid)
	{
		destroyEntity(eid, false);
		pEntityIDAliasIDList_.erase(std::remove(pEntityIDAliasIDList_.begin(), pEntityIDAliasIDList_.end(), eid), pEntityIDAliasIDList_.end());
	}
	else
	{
		clearSpace(false);

		Py_DECREF(entity->getCellMailbox());
		entity->setCellMailbox(NULL);
	}
}

//-------------------------------------------------------------------------------------	
void ClientObjectBase::onEntityEnterSpace(Mercury::Channel * pChannel, MemoryStream& s)
{
	ENTITY_ID eid = 0;
	int8 isOnGound = 0;

	s >> eid;

	if(s.opsize() > 0)
		s >> isOnGound;

	client::Entity* entity = pEntities_->find(eid);
	if(entity == NULL)
	{	
		ERROR_MSG(boost::format("ClientObjectBase::onEntityEnterSpace: not found entity(%1%).\n") % eid);
		return;
	}

	DEBUG_MSG(boost::format("ClientObjectBase::onEntityEnterSpace: %1%(%2%).\n") % 
		entity->getScriptName() % eid);

	entity->isOnGound(isOnGound > 0);

	entityPos_ = entity->getPosition();
	entityDir_ = entity->getDirection();

	// 初始化一下服务端当前的位置
	entity->setServerPosition(entity->getPosition());

	EventData_EnterSpace eventdata;
	eventdata.spaceID = spaceID_;
	eventdata.entityID = entity->getID();
	eventdata.x = entity->getPosition().x;
	eventdata.y = entity->getPosition().y;
	eventdata.z = entity->getPosition().z;
	eventdata.pitch = entity->getDirection().pitch();
	eventdata.roll = entity->getDirection().roll();
	eventdata.yaw = entity->getDirection().yaw();
	eventdata.speed = entity->getMoveSpeed();
	eventdata.isOnGound = isOnGound > 0;
	eventHandler_.fire(&eventdata);

	entity->onEnterSpace();
}

//-------------------------------------------------------------------------------------	
void ClientObjectBase::onEntityLeaveSpace(Mercury::Channel * pChannel, ENTITY_ID eid)
{
	client::Entity* entity = pEntities_->find(eid);
	if(entity == NULL)
	{	
		ERROR_MSG(boost::format("ClientObjectBase::onEntityLeaveSpace: not found entity(%1%).\n") % eid);
		return;
	}

	DEBUG_MSG(boost::format("ClientObjectBase::onEntityLeaveSpace: %1%(%2%).\n") % 
		entity->getScriptName() % eid);

	EventData_LeaveSpace eventdata;
	eventdata.spaceID = spaceID_;
	eventdata.entityID = entity->getID();
	eventHandler_.fire(&eventdata);

	entity->onLeaveSpace();
	clearSpace(false);
}

//-------------------------------------------------------------------------------------	
void ClientObjectBase::onEntityDestroyed(Mercury::Channel * pChannel, ENTITY_ID eid)
{
	client::Entity* entity = pEntities_->find(eid);
	if(entity == NULL)
	{	
		ERROR_MSG(boost::format("ClientObjectBase::onEntityDestroyed: not found entity(%1%).\n") % eid);
		return;
	}

	DEBUG_MSG(boost::format("ClientObjectBase::onEntityDestroyed: %1%(%2%).\n") % 
		entity->getScriptName() % eid);

	destroyEntity(eid, false);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onRemoteMethodCallOptimized(Mercury::Channel * pChannel, KBEngine::MemoryStream& s)
{
	ENTITY_ID eid = getAoiEntityIDFromStream(s);
	onRemoteMethodCall_(eid, s);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onRemoteMethodCall(Mercury::Channel * pChannel, KBEngine::MemoryStream& s)
{
	ENTITY_ID eid = 0;
	s >> eid;
	onRemoteMethodCall_(eid, s);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onRemoteMethodCall_(ENTITY_ID eid, KBEngine::MemoryStream& s)
{
	client::Entity* entity = pEntities_->find(eid);
	if(entity == NULL)
	{	
		s.opfini();
		ERROR_MSG(boost::format("ClientObjectBase::onRemoteMethodCall: not found entity(%1%).\n") % eid);
		return;
	}

	entity->onRemoteMethodCall(this->pServerChannel(), s);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdatePropertys(Mercury::Channel * pChannel, MemoryStream& s)
{
	ENTITY_ID eid = 0;
	s >> eid;
	onUpdatePropertys_(eid, s);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdatePropertysOptimized(Mercury::Channel * pChannel, MemoryStream& s)
{
	ENTITY_ID eid = getAoiEntityIDFromStream(s);
	onUpdatePropertys_(eid, s);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdatePropertys_(ENTITY_ID eid, MemoryStream& s)
{
	client::Entity* entity = pEntities_->find(eid);
	if(entity == NULL)
	{	
		if(bufferedCreateEntityMessage_.find(eid) == bufferedCreateEntityMessage_.end())
		{
			MemoryStream* buffered = new MemoryStream();
			(*buffered) << eid;
			(*buffered).append(s.data() + s.rpos(), s.opsize());
			bufferedCreateEntityMessage_[eid].reset(buffered);
			s.opfini();
		}
		else
		{
			ERROR_MSG(boost::format("ClientObjectBase::onUpdatePropertys: not found entity(%1%).\n") % eid);
		}

		return;
	}

	entity->onUpdatePropertys(s);
}

//-------------------------------------------------------------------------------------
client::Entity* ClientObjectBase::pPlayer()
{
	return pEntities_->find(entityID_);
}

//-------------------------------------------------------------------------------------
ENTITY_ID ClientObjectBase::readEntityIDFromStream(MemoryStream& s)
{
	if(pEntities_->size() <= 255)
	{
		uint8 aliasIdx = 0;
		s >> aliasIdx;
		return pEntityIDAliasIDList_[aliasIdx];
	}
	else
	{
		ENTITY_ID eid = 0;
		s >> eid;
		return eid;
	}

	return 0;
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::updatePlayerToServer()
{
	if (timestamp() - lastSentUpdateDataTime_ < uint64(stampsPerSecond() * 0.01) || this->spaceID_ == 0)
	{
		return;
	}

	lastSentUpdateDataTime_ = timestamp();

	client::Entity* pEntity = pEntities_->find(entityID_);
	if(pEntity == NULL || !connectedGateway_ || 
		pServerChannel_ == NULL || pEntity->getCellMailbox() == NULL)
		return;

	Position3D& pos = pEntity->getPosition();
	Direction3D& dir = pEntity->getDirection();

	bool dirNoChanged = almostEqual(dir.yaw(), entityDir_.yaw()) && almostEqual(dir.pitch(), entityDir_.pitch()) && almostEqual(dir.roll(), entityDir_.roll());
	Vector3 movement = pos - entityPos_;

	bool posNoChanged =  KBEVec3Length(&movement) < 0.0004f;

	if(posNoChanged && dirNoChanged)
		return;

	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
	(*pBundle).newMessage(BaseappInterface::onUpdateDataFromClient);
	
	pEntity->setPosition(entityPos_);
	pEntity->setDirection(entityDir_);

	(*pBundle) << pos.x;
	(*pBundle) << pos.y;
	(*pBundle) << pos.z;

	(*pBundle) << dir.yaw();
	(*pBundle) << dir.pitch();
	(*pBundle) << dir.roll();

	(*pBundle) << pEntity->isOnGound();
	(*pBundle) << spaceID_;
	pServerChannel_->pushBundle(pBundle);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateBasePos(Mercury::Channel* pChannel, MemoryStream& s)
{
	float x, y, z;
	s >> x >> y >> z;
	
	client::Entity* pEntity = pPlayer();
	if(pEntity)
	{
		pEntity->setServerPosition(Position3D(x, y, z));
	}
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateBasePosXZ(Mercury::Channel* pChannel, MemoryStream& s)
{
	float x, z;
	s >> x >> z;
	
	client::Entity* pEntity = pPlayer();
	if(pEntity)
	{
		pEntity->setServerPosition(Position3D(x, pEntity->getServerPosition().y, z));
	}
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onSetEntityPosAndDir(Mercury::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid;
	s >> eid;

	client::Entity* entity = pEntities_->find(eid);
	if(entity == NULL)
	{
		ERROR_MSG(boost::format("ClientObjectBase::onSetEntityPosAndDir: not found entity(%1%).\n") % eid);
		s.opfini();
		return;
	}

	Position3D pos;
	Direction3D dir;
	float yaw, pitch, roll;
	s >> pos.x >> pos.y >> pos.z >> yaw >> pitch >> roll;
	
	dir.yaw(yaw);
	dir.pitch(pitch);
	dir.roll(roll);

	entity->setPosition(pos);
	entity->setDirection(dir);

	entityPos_ = pos;
	entityDir_ = dir;

	EventData_PositionForce eventdata;
	eventdata.x = pos.x;
	eventdata.y = pos.y;
	eventdata.z = pos.z;
	eventdata.entityID = entity->getID();
	fireEvent(&eventdata);

	EventData_DirectionForce direventData;
	direventData.yaw = dir.yaw();
	direventData.pitch = dir.pitch();
	direventData.roll = dir.roll();
	direventData.entityID = entity->getID();
	fireEvent(&direventData);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData(Mercury::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid = getAoiEntityIDFromStream(s);

	client::Entity* entity = pEntities_->find(eid);
	if(entity == NULL)
	{
		ERROR_MSG(boost::format("ClientObjectBase::onUpdateData: not found entity(%1%).\n") % eid);
		return;
	}
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_ypr(Mercury::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid = getAoiEntityIDFromStream(s);

	float y, p, r;

	int8 angle;

	s >> angle;
	y = int82angle(angle);

	s >> angle;
	p = int82angle(angle);

	s >> angle;
	r = int82angle(angle);

	_updateVolatileData(eid, 0.f, 0.f, 0.f, r, p, y, -1);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_yp(Mercury::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid = getAoiEntityIDFromStream(s);

	float y, p;

	int8 angle;

	s >> angle;
	y = int82angle(angle);

	s >> angle;
	p = int82angle(angle);

	_updateVolatileData(eid, 0.f, 0.f, 0.f, FLT_MAX, p, y, -1);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_yr(Mercury::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid = getAoiEntityIDFromStream(s);

	float y, r;

	int8 angle;

	s >> angle;
	y = int82angle(angle);

	s >> angle;
	r = int82angle(angle);

	_updateVolatileData(eid, 0.f, 0.f, 0.f, r, FLT_MAX, y, -1);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_pr(Mercury::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid = getAoiEntityIDFromStream(s);

	float p, r;

	int8 angle;

	s >> angle;
	p = int82angle(angle);

	s >> angle;
	r = int82angle(angle);

	_updateVolatileData(eid, 0.f, 0.f, 0.f, r, p, FLT_MAX, -1);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_y(Mercury::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid = getAoiEntityIDFromStream(s);

	float y;

	int8 angle;

	s >> angle;
	y = int82angle(angle);

	_updateVolatileData(eid, 0.f, 0.f, 0.f, FLT_MAX, FLT_MAX, y, -1);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_p(Mercury::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid = getAoiEntityIDFromStream(s);

	float p;

	int8 angle;

	s >> angle;
	p = int82angle(angle);

	_updateVolatileData(eid, 0.f, 0.f, 0.f, FLT_MAX, p, FLT_MAX, -1);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_r(Mercury::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid = getAoiEntityIDFromStream(s);

	float r;

	int8 angle;

	s >> angle;
	r = int82angle(angle);

	_updateVolatileData(eid, 0.f, 0.f, 0.f, r, FLT_MAX, FLT_MAX, -1);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_xz(Mercury::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid = getAoiEntityIDFromStream(s);

	float x,z;

	s.readPackXZ(x, z);


	_updateVolatileData(eid, x, 0.f, z, FLT_MAX, FLT_MAX, FLT_MAX, 1);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_xz_ypr(Mercury::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid = getAoiEntityIDFromStream(s);

	float x, z, y, p, r;
	
	s.readPackXZ(x, z);

	int8 angle;

	s >> angle;
	y = int82angle(angle);

	s >> angle;
	p = int82angle(angle);

	s >> angle;
	r = int82angle(angle);

	_updateVolatileData(eid, x, 0.f, z, r, p, y, 1);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_xz_yp(Mercury::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid = getAoiEntityIDFromStream(s);

	float x,z, y, p;

	s.readPackXZ(x, z);

	int8 angle;

	s >> angle;
	y = int82angle(angle);

	s >> angle;
	p = int82angle(angle);

	_updateVolatileData(eid, x, 0.f, z, FLT_MAX, p, y, 1);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_xz_yr(Mercury::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid = getAoiEntityIDFromStream(s);

	float x, z, y,  r;
	
	s.readPackXZ(x, z);

	int8 angle;

	s >> angle;
	y = int82angle(angle);

	s >> angle;
	r = int82angle(angle);

	_updateVolatileData(eid, x, 0.f, z, r, FLT_MAX, y, 1);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_xz_pr(Mercury::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid = getAoiEntityIDFromStream(s);

	float x, z, p, r;
	
	s.readPackXZ(x, z);

	int8 angle;

	s >> angle;
	p = int82angle(angle);

	s >> angle;
	r = int82angle(angle);

	_updateVolatileData(eid, x, 0.f, z, r, p, FLT_MAX, 1);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_xz_y(Mercury::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid = getAoiEntityIDFromStream(s);

	float x, z, y;
	
	s.readPackXZ(x, z);

	int8 angle;

	s >> angle;
	y = int82angle(angle);

	_updateVolatileData(eid, x, 0.f, z, FLT_MAX, FLT_MAX, y, 1);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_xz_p(Mercury::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid = getAoiEntityIDFromStream(s);

	float x, z, p;
	
	s.readPackXZ(x, z);

	int8 angle;

	s >> angle;
	p = int82angle(angle);

	_updateVolatileData(eid, x, 0.f, z, FLT_MAX, p, FLT_MAX, 1);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_xz_r(Mercury::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid = getAoiEntityIDFromStream(s);

	float x, z, r;
	
	s.readPackXZ(x, z);

	int8 angle;

	s >> angle;
	r = int82angle(angle);

	_updateVolatileData(eid, x, 0.f, z, r, FLT_MAX, FLT_MAX, 1);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_xyz(Mercury::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid = getAoiEntityIDFromStream(s);

	float x, z, y;
	
	s.readPackXZ(x, z);
	s.readPackY(y);

	_updateVolatileData(eid, x, y, z, FLT_MAX, FLT_MAX, FLT_MAX, 0);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_xyz_ypr(Mercury::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid = getAoiEntityIDFromStream(s);

	float x, z, y;
	
	s.readPackXZ(x, z);
	s.readPackY(y);

	float yaw, p, r;

	int8 angle;

	s >> angle;
	yaw = int82angle(angle);

	s >> angle;
	p = int82angle(angle);

	s >> angle;
	r = int82angle(angle);

	_updateVolatileData(eid, x, y, z, r, p, yaw, 0);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_xyz_yp(Mercury::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid = getAoiEntityIDFromStream(s);

	float x, z, y;
	
	s.readPackXZ(x, z);
	s.readPackY(y);

	float yaw, p;

	int8 angle;

	s >> angle;
	yaw = int82angle(angle);

	s >> angle;
	p = int82angle(angle);

	_updateVolatileData(eid, x, y, z, FLT_MAX, p, yaw, 0);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_xyz_yr(Mercury::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid = getAoiEntityIDFromStream(s);

	float x, z, y;
	
	s.readPackXZ(x, z);
	s.readPackY(y);

	float yaw, r;

	int8 angle;

	s >> angle;
	yaw = int82angle(angle);

	s >> angle;
	r = int82angle(angle);

	_updateVolatileData(eid, x, y, z, r, FLT_MAX, yaw, 0);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_xyz_pr(Mercury::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid = getAoiEntityIDFromStream(s);

	float x, z, y;
	
	s.readPackXZ(x, z);
	s.readPackY(y);

	float p, r;

	int8 angle;

	s >> angle;
	p = int82angle(angle);

	s >> angle;
	r = int82angle(angle);

	_updateVolatileData(eid, x, y, z, r, p, FLT_MAX, 0);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_xyz_y(Mercury::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid = getAoiEntityIDFromStream(s);

	float x, z, y;
	
	s.readPackXZ(x, z);
	s.readPackY(y);

	float yaw;

	int8 angle;

	s >> angle;
	yaw = int82angle(angle);

	_updateVolatileData(eid, x, y, z, FLT_MAX, FLT_MAX, yaw, 0);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_xyz_p(Mercury::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid = getAoiEntityIDFromStream(s);

	float x, z, y;
	
	s.readPackXZ(x, z);
	s.readPackY(y);

	float p;

	int8 angle;

	s >> angle;
	p = int82angle(angle);

	_updateVolatileData(eid, x, y, z, FLT_MAX, p, FLT_MAX, 0);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_xyz_r(Mercury::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid = getAoiEntityIDFromStream(s);

	float x, z, y;
	
	s.readPackXZ(x, z);
	s.readPackY(y);

	float r;

	int8 angle;

	s >> angle;
	r = int82angle(angle);

	_updateVolatileData(eid, x, y, z, r, FLT_MAX, FLT_MAX, 0);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::_updateVolatileData(ENTITY_ID entityID, float x, float y, float z, 
										   float roll, float pitch, float yaw, int8 isOnGound)
{
	client::Entity* entity = pEntities_->find(entityID);
	if(entity == NULL)
	{
		ERROR_MSG(boost::format("ClientObjectBase::onUpdateData_xz_yp: not found entity(%1%).\n") % entityID);
		return;
	}

	client::Entity* player = pPlayer();
	if(player == NULL)
	{
		ERROR_MSG(boost::format("ClientObjectBase::_updateVolatileData: not found player(%1%).\n") % entityID_);
		return;
	}

	// 小于0不设置
	if(isOnGound >= 0)
		entity->isOnGound(isOnGound > 0);

	if(!almostEqual(x + y + z, 0.f, 0.000001f))
	{
		Position3D relativePos;
		relativePos.x = x;
		relativePos.y = y;
		relativePos.z = z;
		
		Position3D basepos = player->getServerPosition();
		basepos += relativePos;
		
		// DEBUG_MSG(boost::format("ClientObjectBase::_updateVolatileData: %1%-%2%-%3%--%4%-%5%-%6%-\n") % x % y % z % basepos.x % basepos.y % basepos.z);
		entity->setPosition(basepos);
	}

	Direction3D dir = entity->getDirection();
	
	if(yaw != FLT_MAX)
		dir.yaw(yaw);

	if(pitch != FLT_MAX)
		dir.pitch(pitch);

	if(roll != FLT_MAX)
		dir.roll(roll);

	if(yaw != FLT_MAX || pitch != FLT_MAX || roll != FLT_MAX)
		entity->setDirection(dir);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onStreamDataStarted(Mercury::Channel* pChannel, int16 id, uint32 datasize, std::string& descr)
{
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onStreamDataRecv(Mercury::Channel* pChannel, MemoryStream& s)
{
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onStreamDataCompleted(Mercury::Channel* pChannel, int16 id)
{
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::addSpaceGeometryMapping(SPACE_ID spaceID, const std::string& respath)
{
	INFO_MSG(boost::format("ClientObjectBase::addSpaceGeometryMapping: spaceID=%1%, respath=%2%!\n") %
		spaceID % respath);

	isLoadedGeometry_ = false;
	onAddSpaceGeometryMapping(spaceID, respath);

	EventData_AddSpaceGEOMapping eventdata;
	eventdata.spaceID = spaceID;
	eventdata.respath = respath;
	fireEvent(&eventdata);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::clearSpace(bool isAll)
{
	if(!isAll)
	{
		Entities<client::Entity>::ENTITYS_MAP::iterator iter = pEntities_->getEntities().begin();
		for(; iter != pEntities_->getEntities().end(); iter++)
		{
			client::Entity* pEntity = static_cast<client::Entity*>(iter->second.get());
			if(pEntity->getID() == this->entityID())
				continue;

			EventData_LeaveWorld eventdata;
			eventdata.spaceID = pEntity->getSpaceID();
			eventdata.entityID = pEntity->getID();

			pEntity->onLeaveWorld();

			eventHandler_.fire(&eventdata);
		}

		std::vector<ENTITY_ID> excludes;
		excludes.push_back(entityID_);
		pEntities_->clear(true, excludes);
	}
	else
	{
		Entities<client::Entity>::ENTITYS_MAP::iterator iter = pEntities_->getEntities().begin();
		for(; iter != pEntities_->getEntities().end(); iter++)
		{
			client::Entity* pEntity = static_cast<client::Entity*>(iter->second.get());

			EventData_LeaveWorld eventdata;
			eventdata.spaceID = pEntity->getSpaceID();
			eventdata.entityID = pEntity->getID();

			pEntity->onLeaveWorld();

			eventHandler_.fire(&eventdata);
		}
	}

	pEntityIDAliasIDList_.clear();
	spacedatas_.clear();

	isLoadedGeometry_ = false;
	spaceID_ = 0;
	targetID_ = 0;
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::initSpaceData(Mercury::Channel* pChannel, MemoryStream& s)
{
	clearSpace(false);

	s >> spaceID_;

	client::Entity* player = pPlayer();
	if(player)
	{
		player->setSpaceID(spaceID_);
	}

	std::string key, value;
	
	while(s.opsize() > 0)
	{
		s >> key >> value;
		setSpaceData(pChannel, spaceID_, key, value);
	}

	DEBUG_MSG(boost::format("ClientObjectBase::initSpaceData: spaceID(%1%), datasize=%2%.\n") % 
		spaceID_ % spacedatas_.size());
}

//-------------------------------------------------------------------------------------
const std::string& ClientObjectBase::getGeometryPath()
{ 
	return getSpaceData("_mapping"); 
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::setSpaceData(Mercury::Channel* pChannel, SPACE_ID spaceID, const std::string& key, const std::string& value)
{
	if(spaceID_ != spaceID)
	{
		ERROR_MSG(boost::format("ClientObjectBase::setSpaceData: spaceID(curr:%1%->%2%) not match, key=%3%, value=%4%.\n") % 
			spaceID_ % spaceID % key % value);
		return;
	}

	DEBUG_MSG(boost::format("ClientObjectBase::setSpaceData: spaceID(%1%), key=%2%, value=%3%.\n") % 
		spaceID_ % key % value);

	SPACE_DATA::iterator iter = spacedatas_.find(key);
	if(iter == spacedatas_.end())
		spacedatas_.insert(SPACE_DATA::value_type(key, value)); 
	else
		if(iter->second == value)
			return;
		else
			spacedatas_[key] = value;

	if(key == "_mapping")
		addSpaceGeometryMapping(spaceID, value);
}

//-------------------------------------------------------------------------------------
bool ClientObjectBase::hasSpaceData(const std::string& key)
{
	if(key.size() == 0)
		return false;

	SPACE_DATA::iterator iter = spacedatas_.find(key);
	if(iter == spacedatas_.end())
		return false;

	return true;
}

//-------------------------------------------------------------------------------------
const std::string& ClientObjectBase::getSpaceData(const std::string& key)
{
	SPACE_DATA::iterator iter = spacedatas_.find(key);
	if(iter == spacedatas_.end())
	{
		static const std::string null = "";
		return null;
	}

	return iter->second;
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::delSpaceData(Mercury::Channel* pChannel, SPACE_ID spaceID, const std::string& key)
{
	if(spaceID_ != spaceID)
	{
		ERROR_MSG(boost::format("ClientObjectBase::delSpaceData: spaceID(curr:%1%->%2%) not match, key=%3%.\n") % 
			spaceID_ % spaceID % key);
		return;
	}

	DEBUG_MSG(boost::format("ClientObjectBase::delSpaceData: spaceID(%1%), key=%2%.\n") % 
		spaceID_ % key);

	SPACE_DATA::iterator iter = spacedatas_.find(key);
	if(iter == spacedatas_.end())
		return;

	spacedatas_.erase(iter);
}

//-------------------------------------------------------------------------------------
PyObject* ClientObjectBase::__py_GetSpaceData(PyObject* self, PyObject* args)
{
	if(PyTuple_Size(args) != 1)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::getSpaceData: (argssize != (key)) is error!");
		PyErr_PrintEx(0);
		return 0;
	}
	
	char* key = NULL;
	if(PyArg_ParseTuple(args, "s",  &key) == -1)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::getSpaceData: args is error!");
		PyErr_PrintEx(0);
		return 0;
	}
	
	ClientObjectBase* pClientObjectBase = static_cast<ClientObjectBase*>(self);

	if(!pClientObjectBase->hasSpaceData(key))
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::getSpaceData: (key=%s) not found!", 
			key);

		PyErr_PrintEx(0);
		return 0;
	}

	return PyUnicode_FromString(pClientObjectBase->getSpaceData(key).c_str());
}

//-------------------------------------------------------------------------------------
PyObject* ClientObjectBase::__py_getWatcher(PyObject* self, PyObject* args)
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

//-------------------------------------------------------------------------------------
PyObject* ClientObjectBase::__py_getWatcherDir(PyObject* self, PyObject* args)
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

//-------------------------------------------------------------------------------------		
}
