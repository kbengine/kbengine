// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include "entity.h"
#include "config.h"
#include "clientobjectbase.h"
#include "pyscript/pywatcher.h"
#include "network/channel.h"
#include "network/fixed_messages.h"
#include "network/common.h"
#include "network/message_handler.h"
#include "entitydef/scriptdef_module.h"
#include "entitydef/entity_call.h"
#include "entitydef/entitydef.h"

#include "../../server/baseapp/baseapp_interface.h"
#include "../../server/loginapp/loginapp_interface.h"

namespace KBEngine{

SCRIPT_METHOD_DECLARE_BEGIN(ClientObjectBase)
SCRIPT_DIRECT_METHOD_DECLARE("getSpaceData",		__py_GetSpaceData,			METH_VARARGS,					0)
SCRIPT_DIRECT_METHOD_DECLARE("callback",			__py_callback,				METH_VARARGS,					0)
SCRIPT_DIRECT_METHOD_DECLARE("cancelCallback",		__py_cancelCallback,		METH_VARARGS,					0)
SCRIPT_DIRECT_METHOD_DECLARE("getWatcher",			__py_getWatcher,			METH_VARARGS,					0)
SCRIPT_DIRECT_METHOD_DECLARE("getWatcherDir",		__py_getWatcherDir,			METH_VARARGS,					0)
SCRIPT_DIRECT_METHOD_DECLARE("disconnect",			__py_disconnect,			METH_VARARGS,					0)
SCRIPT_DIRECT_METHOD_DECLARE("kbassert",			__py_assert,				METH_VARARGS,					0)
SCRIPT_DIRECT_METHOD_DECLARE("player",				__py_getPlayer,				METH_VARARGS,					0)
SCRIPT_METHOD_DECLARE_END()

SCRIPT_MEMBER_DECLARE_BEGIN(ClientObjectBase)
SCRIPT_MEMBER_DECLARE_END()

SCRIPT_GETSET_DECLARE_BEGIN(ClientObjectBase)
SCRIPT_GET_DECLARE("id",							pyGetID,					0,								0)
SCRIPT_GET_DECLARE("entities",						pyGetEntities,				0,								0)
SCRIPT_GETSET_DECLARE_END()
SCRIPT_INIT(ClientObjectBase, 0, 0, 0, 0, 0)		

static int32 g_appID = 1;

//-------------------------------------------------------------------------------------
ClientObjectBase::ClientObjectBase(Network::NetworkInterface& ninterface, PyTypeObject* pyType):
ScriptObject(pyType != NULL ? pyType : getScriptType(), false),
appID_(0),
pServerChannel_(NULL),
pEntities_(new Entities<client::Entity>()),
pEntityIDAliasIDList_(),
pyCallbackMgr_(),
entityID_(0),
spaceID_(0),
dbid_(0),
ip_(),
tcp_port_(0),
udp_port_(0),
baseappIP_(),
baseappPort_(0),
lastSentActiveTickTime_(timestamp()),
lastSentUpdateDataTime_(timestamp()),
connectedBaseapp_(false),
canReset_(false),
name_(),
password_(),
clientDatas_(""),
serverDatas_(""),
#if KBE_PLATFORM == PLATFORM_UNIX
typeClient_(CLIENT_TYPE_LINUX),
#elif KBE_PLATFORM == PLATFORM_APPLE
typeClient_(CLIENT_TYPE_MAC),
#else
typeClient_(CLIENT_TYPE_WIN),
#endif
bufferedCreateEntityMessage_(),
eventHandler_(),
networkInterface_(ninterface),
targetID_(0),
isLoadedGeometry_(false),
timers_(),
scriptCallbacks_(timers_),
locktime_(0),
controlledEntities_()
{
	appID_ = g_appID++;

	pServerChannel_ = Network::Channel::createPoolObject(OBJECTPOOL_POINT);
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
		client::Entity* entity = pPlayer();

		if(entity && entity->inWorld())
			entity->onBecomeNonPlayer();

		pEntities_->finalise();
		S_RELEASE(pEntities_);
	}

	if(pServerChannel_)
	{
		pServerChannel_->destroy();
		delete pServerChannel_;
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
	tcp_port_ = 0;
	udp_port_ = 0;

	lastSentActiveTickTime_ = timestamp();
	lastSentUpdateDataTime_ = timestamp();
	connectedBaseapp_ = false;

	name_ = "";
	password_ = "";
	clientDatas_ = "";
	serverDatas_ = "";

	bufferedCreateEntityMessage_.clear();
	canReset_ = false;
	locktime_ = 0;

	if(pServerChannel_ && !pServerChannel_->isDestroyed())
	{
		pServerChannel_->destroy();
		Network::Channel::reclaimPoolObject(pServerChannel_);
		pServerChannel_ = NULL;
	}

	pServerChannel_ = Network::Channel::createPoolObject(OBJECTPOOL_POINT);
	pServerChannel_->pNetworkInterface(&networkInterface_);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onServerClosed()
{
	EventData_ServerCloased eventdata;
	eventHandler_.fire(&eventdata);
	connectedBaseapp_ = false;
	canReset_ = true;

	DEBUG_MSG("ClientObjectBase::tickSend: serverCloased!\n");
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::tickSend()
{
	handleTimers();

	if(!pServerChannel_ || !pServerChannel_->pEndPoint())
		return;
	
	if(pServerChannel_ && pServerChannel_->isDestroyed())
	{
		if(connectedBaseapp_)
		{
			onServerClosed();
		}

		return;
	}

	// 向服务器发送tick
	uint64 check = uint64( Network::g_channelExternalTimeout * stampsPerSecond() ) / 2;
	if (timestamp() - lastSentActiveTickTime_ > check)
	{
		lastSentActiveTickTime_ = timestamp();

		Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);
		if(connectedBaseapp_)
			(*pBundle).newMessage(BaseappInterface::onClientActiveTick);
		else
			(*pBundle).newMessage(LoginappInterface::onClientActiveTick);

		pServerChannel_->send(pBundle);
	}

	updatePlayerToServer();
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

	ERROR_MSG(fmt::format("EntityApp::destroyEntity: not found {}!\n", entityID));
	return false;
}

//-------------------------------------------------------------------------------------	
Network::Channel* ClientObjectBase::findChannelByEntityCall(EntityCallAbstract& entityCall)
{
	return pServerChannel_;
}

//-------------------------------------------------------------------------------------
PyObject* ClientObjectBase::tryGetEntity(COMPONENT_ID componentID, ENTITY_ID eid)
{
	client::Entity* entity = pEntities_->find(eid);
	if (entity == NULL) {
		ERROR_MSG(fmt::format("ClientObjectBase::tryGetEntity: can't found entity:{}.\n", eid));
		return NULL;
	}

	return entity;
}

//-------------------------------------------------------------------------------------	
void ClientObjectBase::onKicked(Network::Channel * pChannel, SERVER_ERROR_CODE failedcode)
{
	INFO_MSG(fmt::format("ClientObjectBase::onKicked: code={}\n", failedcode));

	EventData_onKicked eventdata;
	eventdata.failedcode = failedcode;
	eventHandler_.fire(&eventdata);

	onServerClosed();
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
		PyErr_Format(PyExc_TypeError, "KBEngine::callback: (argssize != (time, callback)) error!");
		PyErr_PrintEx(0);
		S_Return;
	}
	
	float time = 0;
	PyObject* pyCallback = NULL;

	if(!PyArg_ParseTuple(args, "f|O",  &time, &pyCallback))
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::callback: args error!");
		PyErr_PrintEx(0);
		S_Return;
	}

	if(!PyCallable_Check(pyCallback))
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::callback: invalid pycallback!");
		PyErr_PrintEx(0);
		return NULL;
	}
	
	ClientObjectBase* pClientObjectBase = static_cast<ClientObjectBase*>(self);
	Py_INCREF(pyCallback);
	ScriptID id = pClientObjectBase->scriptCallbacks().addCallback(time, 0.0f, new ScriptCallbackHandler(pClientObjectBase->scriptCallbacks(), pyCallback));
	return PyLong_FromLong(id);
}

//-------------------------------------------------------------------------------------	
PyObject* ClientObjectBase::__py_cancelCallback(PyObject* self, PyObject* args)
{
	if(PyTuple_Size(args) != 1)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::cancelCallback: (argssize != (callbackID)) error!");
		PyErr_PrintEx(0);
		S_Return;
	}
	
	ClientObjectBase* pClientObjectBase = static_cast<ClientObjectBase*>(self);

	ScriptID id = 0;

	if(!PyArg_ParseTuple(args, "i",  &id))
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::cancelCallback: args error!");
		PyErr_PrintEx(0);
		S_Return;
	}

	pClientObjectBase->scriptCallbacks().delCallback(id);
	S_Return;
}

//-------------------------------------------------------------------------------------	
client::Entity* ClientObjectBase::createEntity(const char* entityType, PyObject* params,
	bool isInitializeScript, ENTITY_ID eid, bool initProperty,
	EntityCall* base, EntityCall* cell)
{
	KBE_ASSERT(eid > 0);

	EntityDef::context().currClientappID = appID();
	EntityDef::context().currEntityID = eid;
	EntityDef::context().currComponentType = CLIENT_TYPE;

	ScriptDefModule* sm = EntityDef::findScriptModule(entityType);
	if(sm == NULL)
	{
		PyErr_Format(PyExc_TypeError, "ClientObjectBase::createEntity: entity [%s] not found.\n", 
			entityType);

		PyErr_PrintEx(0);
		return NULL;
	}
	else if(!sm->hasClient())
	{
		PyErr_Format(PyExc_TypeError, "ClientObjectBase::createEntity: entity [%s] not found.\n", 
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

	entity->isInited(true);
	
	bool isOnInitCallPropertysSetMethods = (g_componentType == BOTS_TYPE) ? 
		g_kbeSrvConfig.getBots().isOnInitCallPropertysSetMethods : 
		Config::getSingleton().isOnInitCallPropertysSetMethods();

	if (isOnInitCallPropertysSetMethods)
		entity->callPropertysSetMethods();

	if(g_debugEntity)
	{
		INFO_MSG(fmt::format("ClientObjectBase::createEntity: new {} ({}) refc={}.\n", 
			entityType, eid, obj->ob_refcnt));
	}
	else
	{
		INFO_MSG(fmt::format("ClientObjectBase::createEntity: new {} ({})\n",
			entityType, eid));
	}

	EventData_CreatedEntity eventdata;
	eventdata.entityID = entity->id();
	//eventdata.modelres = entity->getAspect()->modelres();
	eventdata.modelScale = entity->getAspect()->modelScale();
	eventHandler_.fire(&eventdata);
	
	return entity;
}

//-------------------------------------------------------------------------------------
ENTITY_ID ClientObjectBase::getViewEntityID(ENTITY_ID id)
{
	if(id <= 255 && EntityDef::entityAliasID() && pEntityIDAliasIDList_.size() <= 255)
	{
		return pEntityIDAliasIDList_[id];
	}

	return id;
}

//-------------------------------------------------------------------------------------
ENTITY_ID ClientObjectBase::getViewEntityIDFromStream(MemoryStream& s)
{
	ENTITY_ID id = 0;

	if (!EntityDef::entityAliasID())
	{
		s >> id;
		return id;
	}

	if(pEntityIDAliasIDList_.size() > 255)
	{
		s >> id;
	}
	else
	{
		uint8 aliasID = 0;
		s >> aliasID;

		// 如果为0且客户端上一步是重登陆或者重连操作并且服务端entity在断线期间一直处于在线状态
		// 则可以忽略这个错误, 因为cellapp可能一直在向baseapp发送同步消息， 当客户端重连上时未等
		// 服务端初始化步骤开始则收到同步信息, 此时这里就会出错。
		if (pEntityIDAliasIDList_.size() <= aliasID)
			return 0;

		id = pEntityIDAliasIDList_[aliasID];
	}

	return id;
}

//-------------------------------------------------------------------------------------
ENTITY_ID ClientObjectBase::getViewEntityIDByAliasID(uint8 id)
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
	Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);
	(*pBundle).newMessage(LoginappInterface::reqCreateAccount);
	(*pBundle) << name_;
	(*pBundle) << password_;
	(*pBundle).appendBlob(clientDatas_);
	pServerChannel_->send(pBundle);
	return true;
}

//-------------------------------------------------------------------------------------
Network::Channel* ClientObjectBase::initLoginappChannel(std::string accountName, std::string passwd, std::string ip, KBEngine::uint32 port)
{
	Network::EndPoint* pEndpoint = Network::EndPoint::createPoolObject(OBJECTPOOL_POINT);
	
	pEndpoint->socket(SOCK_STREAM);
	if (!pEndpoint->good())
	{
		ERROR_MSG("ClientObjectBase::initLoginappChannel: couldn't create a socket\n");
		Network::EndPoint::reclaimPoolObject(pEndpoint);
		return NULL;
	}
	
	u_int32_t address;
	Network::Address::string2ip(ip.c_str(), address);
	if(pEndpoint->connect(htons(port), address) == -1)
	{
		ERROR_MSG(fmt::format("ClientObjectBase::initLoginappChannel: connect server error({})!\n",
			kbe_strerror()));

		Network::EndPoint::reclaimPoolObject(pEndpoint);
		return NULL;
	}

	Network::Address addr(ip.c_str(), port);
	pEndpoint->addr(addr);

	pServerChannel_->pEndPoint(pEndpoint);
	pEndpoint->setnonblocking(true);
	pEndpoint->setnodelay(true);

	password_ = passwd;
	name_ = accountName;
	lastSentActiveTickTime_ = timestamp();
	return pServerChannel_;
}

//-------------------------------------------------------------------------------------
Network::Channel* ClientObjectBase::initBaseappChannel()
{
	Network::EndPoint* pEndpoint = Network::EndPoint::createPoolObject(OBJECTPOOL_POINT);
	
	pEndpoint->socket(SOCK_STREAM);
	if (!pEndpoint->good())
	{
		ERROR_MSG("ClientObjectBase::initBaseappChannel: couldn't create a socket\n");
		Network::EndPoint::reclaimPoolObject(pEndpoint);
		return NULL;
	}
	
	u_int32_t address;

	Network::Address::string2ip(ip_.c_str(), address);
	if(pEndpoint->connect(htons(tcp_port_), address) == -1)
	{
		ERROR_MSG(fmt::format("ClientObjectBase::initBaseappChannel: connect server error({})!\n",
			kbe_strerror()));

		Network::EndPoint::reclaimPoolObject(pEndpoint);
		return NULL;
	}

	Network::Address addr(ip_.c_str(), tcp_port_);
	pEndpoint->addr(addr);

	pServerChannel_->pEndPoint(pEndpoint);
	pEndpoint->setnonblocking(true);
	pEndpoint->setnodelay(true);

	connectedBaseapp_ = true;
	lastSentActiveTickTime_ = timestamp();
	lastSentUpdateDataTime_ = timestamp();
	return pServerChannel_;
}

//-------------------------------------------------------------------------------------	
void ClientObjectBase::fireEvent(const EventData* pEventData)
{
	eventHandler_.fire(pEventData);
}

//-------------------------------------------------------------------------------------	
void ClientObjectBase::onHelloCB_(Network::Channel* pChannel, const std::string& verInfo, 
		const std::string& scriptVerInfo, const std::string& protocolMD5, const std::string& entityDefMD5, 
		COMPONENT_TYPE componentType)
{
}

//-------------------------------------------------------------------------------------	
void ClientObjectBase::onHelloCB(Network::Channel* pChannel, MemoryStream& s)
{
	std::string verInfo;
	s >> verInfo;
	
	std::string scriptVerInfo; 
	s >> scriptVerInfo;

	std::string protocolMD5;
	s >> protocolMD5;

	std::string entityDefMD5;
	s >> entityDefMD5;

	COMPONENT_TYPE ctype;
	s >> ctype;

	INFO_MSG(fmt::format("ClientObjectBase::onHelloCB: verInfo={}, scriptVerInfo={}, protocolMD5={}, entityDefMD5={}, addr:{}\n",
		verInfo, scriptVerInfo, protocolMD5, entityDefMD5, pChannel->c_str()));

	onHelloCB_(pChannel, verInfo, scriptVerInfo, protocolMD5, entityDefMD5, ctype);
}

//-------------------------------------------------------------------------------------	
void ClientObjectBase::onVersionNotMatch(Network::Channel* pChannel, MemoryStream& s)
{
	std::string verInfo;
	s >> verInfo;
	
	INFO_MSG(fmt::format("ClientObjectBase::onVersionNotMatch: verInfo={} not match(server:{})\n",
		KBEVersion::versionString(), verInfo));

	EventData_VersionNotMatch eventdata;
	eventdata.verInfo = KBEVersion::versionString();
	eventdata.serVerInfo = verInfo;
	eventHandler_.fire(&eventdata);
}

//-------------------------------------------------------------------------------------	
void ClientObjectBase::onScriptVersionNotMatch(Network::Channel* pChannel, MemoryStream& s)
{
	std::string verInfo;
	s >> verInfo;
	
	INFO_MSG(fmt::format("ClientObjectBase::onScriptVersionNotMatch: verInfo={} not match(server:{})\n", 
		KBEVersion::scriptVersionString(), verInfo));

	EventData_ScriptVersionNotMatch eventdata;
	eventdata.verInfo = KBEVersion::scriptVersionString();
	eventdata.serVerInfo = verInfo;
	eventHandler_.fire(&eventdata);
}

//-------------------------------------------------------------------------------------
bool ClientObjectBase::login()
{
	Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);

	// 提交账号密码请求登录
	(*pBundle).newMessage(LoginappInterface::login);
	(*pBundle) << typeClient_;
	(*pBundle).appendBlob(clientDatas_);
	(*pBundle) << name_;
	(*pBundle) << password_;

	if (g_componentType == BOTS_TYPE)
	{
		if (!g_kbeSrvConfig.getDBMgr().allowEmptyDigest)
			(*pBundle) << EntityDef::md5().getDigestStr();

		ENGINE_COMPONENT_INFO& infos = g_kbeSrvConfig.getBots();
		(*pBundle) << infos.forceInternalLogin;
	}
	else
	{
		(*pBundle) << EntityDef::md5().getDigestStr();
	}

	onLogin(pBundle);
	pServerChannel_->send(pBundle);
	connectedBaseapp_ = false;
	return true;
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onLogin(Network::Bundle* pBundle)
{

}

//-------------------------------------------------------------------------------------
bool ClientObjectBase::loginBaseapp()
{
	// 请求登录网关, 能走到这里来一定是连接了网关
	connectedBaseapp_ = true;

	Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);
	(*pBundle).newMessage(BaseappInterface::loginBaseapp);
	(*pBundle) << name_;
	(*pBundle) << password_;
	pServerChannel_->send(pBundle);
	return true;
}

//-------------------------------------------------------------------------------------
bool ClientObjectBase::reloginBaseapp()
{
	// 请求重登陆网关, 通常是掉线了之后执行
	connectedBaseapp_ = true;

	Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);
	(*pBundle).newMessage(BaseappInterface::reloginBaseapp);
	(*pBundle) << name_;
	(*pBundle) << password_;
	(*pBundle) << rndUUID();
	(*pBundle) << entityID_;
	pServerChannel_->send(pBundle);
	return true;
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onCreateAccountResult(Network::Channel * pChannel, MemoryStream& s)
{
	SERVER_ERROR_CODE retcode;

	s >> retcode;
	s.readBlob(serverDatas_);

	if(retcode != 0)
	{
		INFO_MSG(fmt::format("ClientObjectBase::onCreateAccountResult: {} create is failed! code={}.\n", 
			name_, retcode));

		return;
	}

	INFO_MSG(fmt::format("ClientObjectBase::onCreateAccountResult: {} create is successfully!\n", name_));
}

//-------------------------------------------------------------------------------------	
void ClientObjectBase::onLoginSuccessfully(Network::Channel * pChannel, MemoryStream& s)
{
	std::string accountName;

	s >> accountName;
	s >> ip_;
	s >> tcp_port_;
	s >> udp_port_;
	s.readBlob(serverDatas_);
	
	connectedBaseapp_ = false;
	INFO_MSG(fmt::format("ClientObjectBase::onLoginSuccessfully: {} addr={}:{}|{}!\n", name_, ip_, tcp_port_, udp_port_));

	EventData_LoginSuccess eventdata;
	eventHandler_.fire(&eventdata);

	baseappIP_ = ip_;
	baseappPort_ = tcp_port_;
}

//-------------------------------------------------------------------------------------	
void ClientObjectBase::onLoginFailed(Network::Channel * pChannel, MemoryStream& s)
{
	SERVER_ERROR_CODE failedcode;

	s >> failedcode;
	s.readBlob(serverDatas_);
	
	connectedBaseapp_ = false;
	INFO_MSG(fmt::format("ClientObjectBase::onLoginFailed: {} failedcode={}!\n", name_, failedcode));
	EventData_LoginFailed eventdata;
	eventdata.failedcode = failedcode;
	eventHandler_.fire(&eventdata);
}

//-------------------------------------------------------------------------------------	
void ClientObjectBase::onLoginBaseappFailed(Network::Channel * pChannel, SERVER_ERROR_CODE failedcode)
{
	INFO_MSG(fmt::format("ClientObjectBase::onLoginBaseappFailed: {} failedcode={}!\n", name_, failedcode));

	// 能走到这里来一定是连接了网关
	connectedBaseapp_ = true;

	EventData_LoginBaseappFailed eventdata;
	eventdata.failedcode = failedcode;
	eventdata.relogin = false;
	eventHandler_.fire(&eventdata);
}

//-------------------------------------------------------------------------------------	
void ClientObjectBase::onReloginBaseappFailed(Network::Channel * pChannel, SERVER_ERROR_CODE failedcode)
{
	INFO_MSG(fmt::format("ClientObjectBase::onReloginBaseappFailed: {} failedcode={}!\n", name_, failedcode));

	// 能走到这里来一定是连接了网关
	connectedBaseapp_ = true;

	EventData_LoginBaseappFailed eventdata;
	eventdata.failedcode = failedcode;
	eventdata.relogin = true;
	eventHandler_.fire(&eventdata);
}

//-------------------------------------------------------------------------------------	
void ClientObjectBase::onReloginBaseappSuccessfully(Network::Channel * pChannel, MemoryStream& s)
{
	s >> rndUUID_;
	INFO_MSG(fmt::format("ClientObjectBase::onReloginBaseappSuccessfully! name={}, rndUUID={}.\n", name_, rndUUID_));
}

//-------------------------------------------------------------------------------------	
void ClientObjectBase::onCreatedProxies(Network::Channel * pChannel, uint64 rndUUID, ENTITY_ID eid, std::string& entityType)
{
	if(entityID_ == 0)
	{
		EventData_LoginBaseappSuccess eventdata;
		eventHandler_.fire(&eventdata);
	}

	entityID_ = eid;
	rndUUID_ = rndUUID;
	
	// 能走到这里来一定是连接了网关
	connectedBaseapp_ = true;
		
	BUFFEREDMESSAGE::iterator iter = bufferedCreateEntityMessage_.find(eid);
	bool hasBufferedMessage = (iter != bufferedCreateEntityMessage_.end());

	client::Entity* entity = pEntities_->find(eid);

	if(entity == NULL)
	{
		INFO_MSG(fmt::format("ClientObject::onCreatedProxies({}): rndUUID={} eid={} entityType={}!\n",
			name_, rndUUID, eid, entityType));

		// 设置entity的baseEntityCall
		EntityCall* entityCall = new EntityCall(EntityDef::findScriptModule(entityType.c_str()), 
			NULL, appID(), eid, ENTITYCALL_TYPE_BASE);

		client::Entity* pEntity = createEntity(entityType.c_str(), NULL, !hasBufferedMessage, eid, true, entityCall, NULL);
		KBE_ASSERT(pEntity != NULL);

		if(hasBufferedMessage)
		{
			// 先更新属性再初始化脚本
			this->onUpdatePropertys(pChannel, *iter->second.get());
			bufferedCreateEntityMessage_.erase(iter);
			pEntity->initializeEntity(NULL);
			SCRIPT_ERROR_CHECK();

			pEntity->isInited(true);
			
			bool isOnInitCallPropertysSetMethods = (g_componentType == BOTS_TYPE) ? 
				g_kbeSrvConfig.getBots().isOnInitCallPropertysSetMethods : 
				Config::getSingleton().isOnInitCallPropertysSetMethods();
		
			if (isOnInitCallPropertysSetMethods)
				pEntity->callPropertysSetMethods();
		}
	}
	else
	{
		if(hasBufferedMessage)
		{
			// 先更新属性再初始化脚本
			this->onUpdatePropertys(pChannel, *iter->second.get());
			bufferedCreateEntityMessage_.erase(iter);
			entity->initializeEntity(NULL);
			SCRIPT_ERROR_CHECK();
		}
	}
}

//-------------------------------------------------------------------------------------	
void ClientObjectBase::onEntityEnterWorld(Network::Channel * pChannel, MemoryStream& s)
{
	ENTITY_ID eid = 0;
	ENTITY_SCRIPT_UID scriptType;
	int8 isOnGround = 0;

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

	if(s.length() > 0)
		s >> isOnGround;

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
			
			// 设置entity的cellEntityCall
			EntityCall* entityCall = new EntityCall(EntityDef::findScriptModule(sm->getName()), 
				NULL, appID(), eid, ENTITYCALL_TYPE_CELL);

			entity = createEntity(sm->getName(), NULL, false, eid, true, NULL, entityCall);
			KBE_ASSERT(entity != NULL);

			// 先更新属性再初始化脚本
			this->onUpdatePropertys(pChannel, *iter->second.get());
			bufferedCreateEntityMessage_.erase(iter);
			entity->isOnGround(isOnGround > 0);
			entity->spaceID(spaceID_);
			entity->initializeEntity(NULL);
			SCRIPT_ERROR_CHECK();
			
			entity->isInited(true);

			DEBUG_MSG(fmt::format("ClientObjectBase::onEntityEnterWorld: {}({}), isOnGround({}), appID({}).\n",
				entity->scriptName(), eid, (int)isOnGround, appID()));
		}
		else
		{
			ERROR_MSG(fmt::format("ClientObjectBase::onEntityEnterWorld: not found entity({}), appID({}).\n", eid, appID()));
			return;
		}
	}
	else
	{
		if(!entity->inWorld())
		{
			spaceID_ = entity->spaceID();
			entity->isOnGround(isOnGround > 0);
			entity->clientPos(entity->position());
			entity->clientDir(entity->direction());

			// 初始化一下服务端当前的位置
			entity->serverPosition(entity->position());

			DEBUG_MSG(fmt::format("ClientObjectBase::onPlayerEnterWorld: {}({}), isOnGround({}), appID({}).\n",
				entity->scriptName(), eid, (int)isOnGround, appID()));

			KBE_ASSERT(entity->cellEntityCall() == NULL);

			// 设置entity的cellEntityCall
			EntityCall* entityCall = new EntityCall(entity->pScriptModule(), 
				NULL, appID(), eid, ENTITYCALL_TYPE_CELL);

			entity->cellEntityCall(entityCall);

			// 安全起见， 这里清空一下
			// 如果服务端上使用giveClientTo切换控制权
			// 之前的实体已经进入世界， 切换后的实体也进入世界， 这里可能会残留之前那个实体进入世界的信息
			pEntityIDAliasIDList_.clear();
			std::vector<ENTITY_ID> excludes;
			excludes.push_back(entityID_);
			pEntities_->clear(true, excludes);
		}
	}

	EventData_EnterWorld eventdata;
	eventdata.spaceID = spaceID_;
	eventdata.entityID = entity->id();
	eventdata.x = entity->position().x;
	eventdata.y = entity->position().y;
	eventdata.z = entity->position().z;
	eventdata.pitch = entity->direction().pitch();
	eventdata.roll = entity->direction().roll();
	eventdata.yaw = entity->direction().yaw();
	eventdata.speed = entity->moveSpeed();
	eventdata.isOnGround = isOnGround > 0;
	eventHandler_.fire(&eventdata);

	if(entityID_ == eid)
	{
		entity->onBecomePlayer();
	}
	
	entity->onEnterWorld();

	bool isOnInitCallPropertysSetMethods = (g_componentType == BOTS_TYPE) ? 
		g_kbeSrvConfig.getBots().isOnInitCallPropertysSetMethods : 
		Config::getSingleton().isOnInitCallPropertysSetMethods();

	if (isOnInitCallPropertysSetMethods)
		entity->callPropertysSetMethods();
}

//-------------------------------------------------------------------------------------	
void ClientObjectBase::onEntityLeaveWorldOptimized(Network::Channel * pChannel, MemoryStream& s)
{
	onEntityLeaveWorld(pChannel, getViewEntityIDFromStream(s));
}

//-------------------------------------------------------------------------------------	
void ClientObjectBase::onEntityLeaveWorld(Network::Channel * pChannel, ENTITY_ID eid)
{
	bufferedCreateEntityMessage_.erase(eid);
	client::Entity* entity = pEntities_->find(eid);
	if(entity == NULL)
	{	
		ERROR_MSG(fmt::format("ClientObjectBase::onEntityLeaveWorld: not found entity({}), appID({}).\n", eid, appID()));
		return;
	}

	DEBUG_MSG(fmt::format("ClientObjectBase::onEntityLeaveWorld: {}({}), appID({}).\n", 
		entity->scriptName(), eid, appID()));

	EventData_LeaveWorld eventdata;
	eventdata.spaceID = entity->spaceID();
	eventdata.entityID = entity->id();

	if(entityID_ == eid)
		entity->onBecomeNonPlayer();

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

		Py_DECREF(entity->cellEntityCall());
		entity->cellEntityCall(NULL);
	}
}

//-------------------------------------------------------------------------------------	
void ClientObjectBase::onEntityEnterSpace(Network::Channel * pChannel, MemoryStream& s)
{
	ENTITY_ID eid = 0;
	int8 isOnGround = 0;

	s >> eid;
	s >> spaceID_;

	if(s.length() > 0)
		s >> isOnGround;

	client::Entity* entity = pEntities_->find(eid);
	if(entity == NULL)
	{	
		ERROR_MSG(fmt::format("ClientObjectBase::onEntityEnterSpace: not found entity({}).\n", eid));
		return;
	}

	DEBUG_MSG(fmt::format("ClientObjectBase::onEntityEnterSpace: {}({}).\n", 
		entity->scriptName(), eid));

	entity->isOnGround(isOnGround > 0);

	entity->clientPos(entity->position());
	entity->clientDir(entity->direction());

	// 初始化一下服务端当前的位置
	entity->serverPosition(entity->position());

	EventData_EnterSpace eventdata;
	eventdata.spaceID = spaceID_;
	eventdata.entityID = entity->id();
	eventdata.x = entity->position().x;
	eventdata.y = entity->position().y;
	eventdata.z = entity->position().z;
	eventdata.pitch = entity->direction().pitch();
	eventdata.roll = entity->direction().roll();
	eventdata.yaw = entity->direction().yaw();
	eventdata.speed = entity->moveSpeed();
	eventdata.isOnGround = isOnGround > 0;
	eventHandler_.fire(&eventdata);

	entity->onEnterSpace();
}

//-------------------------------------------------------------------------------------	
void ClientObjectBase::onEntityLeaveSpace(Network::Channel * pChannel, ENTITY_ID eid)
{
	client::Entity* entity = pEntities_->find(eid);
	if(entity == NULL)
	{	
		ERROR_MSG(fmt::format("ClientObjectBase::onEntityLeaveSpace: not found entity({}).\n", eid));
		return;
	}

	DEBUG_MSG(fmt::format("ClientObjectBase::onEntityLeaveSpace: {}({}).\n", 
		entity->scriptName(), eid));

	EventData_LeaveSpace eventdata;
	eventdata.spaceID = spaceID_;
	eventdata.entityID = entity->id();
	eventHandler_.fire(&eventdata);

	entity->onLeaveSpace();
	clearSpace(false);
}

//-------------------------------------------------------------------------------------	
void ClientObjectBase::onEntityDestroyed(Network::Channel * pChannel, ENTITY_ID eid)
{
	client::Entity* entity = pEntities_->find(eid);
	if(entity == NULL)
	{	
		ERROR_MSG(fmt::format("ClientObjectBase::onEntityDestroyed: not found entity({}).\n", eid));
		return;
	}

	DEBUG_MSG(fmt::format("ClientObjectBase::onEntityDestroyed: {}({}).\n", 
		entity->scriptName(), eid));

	if (entity->inWorld())
	{
		if(entityID_ == eid)
			clearSpace(false);
		
		entity->onLeaveWorld();
	}

	destroyEntity(eid, false);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onRemoteMethodCallOptimized(Network::Channel * pChannel, KBEngine::MemoryStream& s)
{
	ENTITY_ID eid = getViewEntityIDFromStream(s);
	onRemoteMethodCall_(eid, s);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onRemoteMethodCall(Network::Channel * pChannel, KBEngine::MemoryStream& s)
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
		s.done();
		ERROR_MSG(fmt::format("ClientObjectBase::onRemoteMethodCall: not found entity({}).\n", eid));
		return;
	}

	entity->onRemoteMethodCall(this->pServerChannel(), s);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdatePropertys(Network::Channel * pChannel, MemoryStream& s)
{
	ENTITY_ID eid = 0;
	s >> eid;
	onUpdatePropertys_(eid, s);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdatePropertysOptimized(Network::Channel * pChannel, MemoryStream& s)
{
	ENTITY_ID eid = getViewEntityIDFromStream(s);
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
			(*buffered).append(s.data() + s.rpos(), s.length());
			bufferedCreateEntityMessage_[eid].reset(buffered);
			s.done();
		}
		else
		{
			ERROR_MSG(fmt::format("ClientObjectBase::onUpdatePropertys: not found entity({}).\n", eid));
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
PyObject* ClientObjectBase::__py_getPlayer(PyObject *self, void *args) {

	ClientObjectBase* pClientObjectBase = static_cast<ClientObjectBase*>(self);

	client::Entity* pEntity = pClientObjectBase->pPlayer();
	if (pEntity)
	{
		Py_INCREF(pEntity);
		return pEntity;
	}

	S_Return;
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
	if(!pServerChannel_ || !pServerChannel_->pEndPoint())
		return;

	if (timestamp() - lastSentUpdateDataTime_ < uint64(stampsPerSecond() * 0.01) || 
		this->spaceID_ == 0)
		return;

	lastSentUpdateDataTime_ = timestamp();

	client::Entity* pEntity = pEntities_->find(entityID_);
	if(pEntity == NULL || !connectedBaseapp_ || 
        pServerChannel_ == NULL || pEntity->cellEntityCall() == NULL || pEntity->isControlled())
		return;

	Position3D& pos = pEntity->position();
	Direction3D& dir = pEntity->direction();

	Position3D& clientPos = pEntity->clientPos();
	Direction3D& clientDir = pEntity->clientDir();

	bool dirChanged = !almostEqual(dir.yaw(), clientDir.yaw()) || !almostEqual(dir.pitch(), clientDir.pitch()) || !almostEqual(dir.roll(), clientDir.roll());
	Vector3 movement = pos - clientPos;
	bool posChanged =  KBEVec3Length(&movement) > 0.0004f;

    if(posChanged || dirChanged)
    {
        Network::Bundle* pBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);
        (*pBundle).newMessage(BaseappInterface::onUpdateDataFromClient);

        pEntity->position(clientPos);
        pEntity->direction(clientDir);

        (*pBundle) << pos.x;
        (*pBundle) << pos.y;
        (*pBundle) << pos.z;

        (*pBundle) << dir.roll();
        (*pBundle) << dir.pitch();
        (*pBundle) << dir.yaw();

        (*pBundle) << pEntity->isOnGround();
        (*pBundle) << spaceID_;
        pServerChannel_->send(pBundle);
    }

    // 同步所有controlled entity的位置与朝向
    std::list<client::Entity *>::iterator itr = controlledEntities_.begin();
    for (; itr != controlledEntities_.end(); itr++)
    {
        client::Entity *entity = *itr;
        
        Position3D &temppos = entity->position();
        Direction3D &tempdir = entity->direction();
        Position3D &tempClientPos = entity->clientPos();
        Direction3D &tempClientDir = entity->clientDir();

        dirChanged = !almostEqual(tempdir.yaw(), tempClientDir.yaw()) || !almostEqual(tempdir.pitch(), tempClientDir.pitch()) || !almostEqual(tempdir.roll(), tempClientDir.roll());
        movement = temppos - tempClientPos;
        posChanged = KBEVec3Length(&movement) > 0.0004f;
        if (posChanged || dirChanged)
        {
            entity->position(tempClientPos);
            entity->direction(tempClientDir);

            Network::Bundle *tempBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);
            (*tempBundle).newMessage(BaseappInterface::onUpdateDataFromClientForControlledEntity);

            (*tempBundle) << entity->id();
            (*tempBundle) << temppos.x;
            (*tempBundle) << temppos.y;
            (*tempBundle) << temppos.z;
            (*tempBundle) << tempdir.roll();
            (*tempBundle) << tempdir.pitch();
            (*tempBundle) << tempdir.yaw();
            (*tempBundle) << entity->isOnGround();
            (*tempBundle) << spaceID_;

            pServerChannel_->send(tempBundle);
        }
    }
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateBasePos(Network::Channel* pChannel, float x, float y, float z)
{
	client::Entity* pEntity = pPlayer();
	if(pEntity)
	{
		pEntity->serverPosition(Position3D(x, y, z));
	}

    if(pEntity->isControlled())
    {
        pEntity->position(pEntity->serverPosition());
    }
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateBasePosXZ(Network::Channel* pChannel, float x, float z)
{
	client::Entity* pEntity = pPlayer();
	if(pEntity)
	{
		pEntity->serverPosition(Position3D(x, pEntity->serverPosition().y, z));
	}

    if (pEntity->isControlled())
    {
        pEntity->position(pEntity->serverPosition());
    }
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateBaseDir(Network::Channel* pChannel, MemoryStream& s)
{
	float yaw, pitch, roll;
	s >> yaw >> pitch >> roll;

	client::Entity* pEntity = pPlayer();
	if (pEntity && pEntity->isControlled())
	{
        Direction3D dir;
        
        if (yaw != FLT_MAX)
            dir.yaw(yaw);
        if (pitch != FLT_MAX)
            dir.pitch(pitch);
        if (roll != FLT_MAX)
            dir.roll(roll);

        if (yaw != FLT_MAX || pitch != FLT_MAX || roll != FLT_MAX)
          pEntity->direction(dir);
	}
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onSetEntityPosAndDir(Network::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid;
	s >> eid;

	client::Entity* entity = pEntities_->find(eid);
	if(entity == NULL)
	{
		ERROR_MSG(fmt::format("ClientObjectBase::onSetEntityPosAndDir: not found entity({}).\n", eid));
		s.done();
		return;
	}

	Position3D pos;
	Direction3D dir;
	float yaw, pitch, roll;
	s >> pos.x >> pos.y >> pos.z >> roll >> pitch >> yaw;
	
	dir.yaw(yaw);
	dir.pitch(pitch);
	dir.roll(roll);

	entity->position(pos);
	entity->direction(dir);

	entity->clientPos(pos);
	entity->clientDir(dir);

	EventData_PositionForce eventdata;
	eventdata.x = pos.x;
	eventdata.y = pos.y;
	eventdata.z = pos.z;
	eventdata.entityID = entity->id();
	fireEvent(&eventdata);

	EventData_DirectionForce direventData;
	direventData.yaw = dir.yaw();
	direventData.pitch = dir.pitch();
	direventData.roll = dir.roll();
	direventData.entityID = entity->id();
	fireEvent(&direventData);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData(Network::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid = getViewEntityIDFromStream(s);

	client::Entity* entity = pEntities_->find(eid);
	if(entity == NULL)
	{
		ERROR_MSG(fmt::format("ClientObjectBase::onUpdateData: not found entity({}).\n", eid));
		return;
	}
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_ypr_optimized(Network::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid = getViewEntityIDFromStream(s);

	float y, p, r;

	int8 angle;

	s >> angle;
	y = int82angle(angle);

	s >> angle;
	p = int82angle(angle);

	s >> angle;
	r = int82angle(angle);

	_updateVolatileData(eid, FLT_MAX, FLT_MAX, FLT_MAX, r, p, y, -1, true);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_yp_optimized(Network::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid = getViewEntityIDFromStream(s);

	float y, p;

	int8 angle;

	s >> angle;
	y = int82angle(angle);

	s >> angle;
	p = int82angle(angle);

	_updateVolatileData(eid, FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX, p, y, -1, true);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_yr_optimized(Network::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid = getViewEntityIDFromStream(s);

	float y, r;

	int8 angle;

	s >> angle;
	y = int82angle(angle);

	s >> angle;
	r = int82angle(angle);

	_updateVolatileData(eid, FLT_MAX, FLT_MAX, FLT_MAX, r, FLT_MAX, y, -1, true);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_pr_optimized(Network::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid = getViewEntityIDFromStream(s);

	float p, r;

	int8 angle;

	s >> angle;
	p = int82angle(angle);

	s >> angle;
	r = int82angle(angle);

	_updateVolatileData(eid, FLT_MAX, FLT_MAX, FLT_MAX, r, p, FLT_MAX, -1, true);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_y_optimized(Network::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid = getViewEntityIDFromStream(s);

	float y;

	int8 angle;

	s >> angle;
	y = int82angle(angle);

	_updateVolatileData(eid, FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX, y, -1, true);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_p_optimized(Network::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid = getViewEntityIDFromStream(s);

	float p;

	int8 angle;

	s >> angle;
	p = int82angle(angle);

	_updateVolatileData(eid, FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX, p, FLT_MAX, -1, true);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_r_optimized(Network::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid = getViewEntityIDFromStream(s);

	float r;

	int8 angle;

	s >> angle;
	r = int82angle(angle);

	_updateVolatileData(eid, FLT_MAX, FLT_MAX, FLT_MAX, r, FLT_MAX, FLT_MAX, -1, true);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_xz_optimized(Network::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid = getViewEntityIDFromStream(s);

	float x,z;

	s.readPackXZ(x, z);


	_updateVolatileData(eid, x, FLT_MAX, z, FLT_MAX, FLT_MAX, FLT_MAX, 1, true);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_xz_ypr_optimized(Network::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid = getViewEntityIDFromStream(s);

	float x, z, y, p, r;
	
	s.readPackXZ(x, z);

	int8 angle;

	s >> angle;
	y = int82angle(angle);

	s >> angle;
	p = int82angle(angle);

	s >> angle;
	r = int82angle(angle);

	_updateVolatileData(eid, x, FLT_MAX, z, r, p, y, 1, true);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_xz_yp_optimized(Network::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid = getViewEntityIDFromStream(s);

	float x,z, y, p;

	s.readPackXZ(x, z);

	int8 angle;

	s >> angle;
	y = int82angle(angle);

	s >> angle;
	p = int82angle(angle);

	_updateVolatileData(eid, x, FLT_MAX, z, FLT_MAX, p, y, 1, true);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_xz_yr_optimized(Network::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid = getViewEntityIDFromStream(s);

	float x, z, y,  r;
	
	s.readPackXZ(x, z);

	int8 angle;

	s >> angle;
	y = int82angle(angle);

	s >> angle;
	r = int82angle(angle);

	_updateVolatileData(eid, x, FLT_MAX, z, r, FLT_MAX, y, 1, true);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_xz_pr_optimized(Network::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid = getViewEntityIDFromStream(s);

	float x, z, p, r;
	
	s.readPackXZ(x, z);

	int8 angle;

	s >> angle;
	p = int82angle(angle);

	s >> angle;
	r = int82angle(angle);

	_updateVolatileData(eid, x, FLT_MAX, z, r, p, FLT_MAX, 1, true);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_xz_y_optimized(Network::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid = getViewEntityIDFromStream(s);

	float x, z, y;
	
	s.readPackXZ(x, z);

	int8 angle;

	s >> angle;
	y = int82angle(angle);

	_updateVolatileData(eid, x, FLT_MAX, z, FLT_MAX, FLT_MAX, y, 1, true);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_xz_p_optimized(Network::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid = getViewEntityIDFromStream(s);

	float x, z, p;
	
	s.readPackXZ(x, z);

	int8 angle;

	s >> angle;
	p = int82angle(angle);

	_updateVolatileData(eid, x, FLT_MAX, z, FLT_MAX, p, FLT_MAX, 1, true);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_xz_r_optimized(Network::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid = getViewEntityIDFromStream(s);

	float x, z, r;
	
	s.readPackXZ(x, z);

	int8 angle;

	s >> angle;
	r = int82angle(angle);

	_updateVolatileData(eid, x, FLT_MAX, z, r, FLT_MAX, FLT_MAX, 1, true);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_xyz_optimized(Network::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid = getViewEntityIDFromStream(s);

	float x, z, y;
	
	s.readPackXZ(x, z);
	s.readPackY(y);

	_updateVolatileData(eid, x, y, z, FLT_MAX, FLT_MAX, FLT_MAX, 0, true);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_xyz_ypr_optimized(Network::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid = getViewEntityIDFromStream(s);

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

	_updateVolatileData(eid, x, y, z, r, p, yaw, 0, true);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_xyz_yp_optimized(Network::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid = getViewEntityIDFromStream(s);

	float x, z, y;
	
	s.readPackXZ(x, z);
	s.readPackY(y);

	float yaw, p;

	int8 angle;

	s >> angle;
	yaw = int82angle(angle);

	s >> angle;
	p = int82angle(angle);

	_updateVolatileData(eid, x, y, z, FLT_MAX, p, yaw, 0, true);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_xyz_yr_optimized(Network::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid = getViewEntityIDFromStream(s);

	float x, z, y;
	
	s.readPackXZ(x, z);
	s.readPackY(y);

	float yaw, r;

	int8 angle;

	s >> angle;
	yaw = int82angle(angle);

	s >> angle;
	r = int82angle(angle);

	_updateVolatileData(eid, x, y, z, r, FLT_MAX, yaw, 0, true);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_xyz_pr_optimized(Network::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid = getViewEntityIDFromStream(s);

	float x, z, y;
	
	s.readPackXZ(x, z);
	s.readPackY(y);

	float p, r;

	int8 angle;

	s >> angle;
	p = int82angle(angle);

	s >> angle;
	r = int82angle(angle);

	_updateVolatileData(eid, x, y, z, r, p, FLT_MAX, 0, true);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_xyz_y_optimized(Network::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid = getViewEntityIDFromStream(s);

	float x, z, y;
	
	s.readPackXZ(x, z);
	s.readPackY(y);

	float yaw;

	int8 angle;

	s >> angle;
	yaw = int82angle(angle);

	_updateVolatileData(eid, x, y, z, FLT_MAX, FLT_MAX, yaw, 0, true);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_xyz_p_optimized(Network::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid = getViewEntityIDFromStream(s);

	float x, z, y;
	
	s.readPackXZ(x, z);
	s.readPackY(y);

	float p;

	int8 angle;

	s >> angle;
	p = int82angle(angle);

	_updateVolatileData(eid, x, y, z, FLT_MAX, p, FLT_MAX, 0, true);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_xyz_r_optimized(Network::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid = getViewEntityIDFromStream(s);

	float x, z, y;
	
	s.readPackXZ(x, z);
	s.readPackY(y);

	float r;

	int8 angle;

	s >> angle;
	r = int82angle(angle);

	_updateVolatileData(eid, x, y, z, r, FLT_MAX, FLT_MAX, 0, true);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_ypr(Network::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid = getViewEntityIDFromStream(s);

	float y, p, r;
	s >> y;
	s >> p;
	s >> r;

	_updateVolatileData(eid, FLT_MAX, FLT_MAX, FLT_MAX, r, p, y, -1, false);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_yp(Network::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid = getViewEntityIDFromStream(s);

	float y, p;
	s >> y;
	s >> p;

	_updateVolatileData(eid, FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX, p, y, -1, false);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_yr(Network::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid = getViewEntityIDFromStream(s);

	float y, r;
	s >> y;
	s >> r;

	_updateVolatileData(eid, FLT_MAX, FLT_MAX, FLT_MAX, r, FLT_MAX, y, -1, false);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_pr(Network::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid = getViewEntityIDFromStream(s);

	float p, r;
	s >> p;
	s >> r;

	_updateVolatileData(eid, FLT_MAX, FLT_MAX, FLT_MAX, r, p, FLT_MAX, -1, false);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_y(Network::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid = getViewEntityIDFromStream(s);

	float y;
	s >> y;

	_updateVolatileData(eid, FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX, y, -1, false);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_p(Network::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid = getViewEntityIDFromStream(s);

	float p;
	s >> p;

	_updateVolatileData(eid, FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX, p, FLT_MAX, -1, false);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_r(Network::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid = getViewEntityIDFromStream(s);

	float r;
	s >> r;

	_updateVolatileData(eid, FLT_MAX, FLT_MAX, FLT_MAX, r, FLT_MAX, FLT_MAX, -1, false);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_xz(Network::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid = getViewEntityIDFromStream(s);

	float x, z;
	s >> x;
	s >> z;

	_updateVolatileData(eid, x, FLT_MAX, z, FLT_MAX, FLT_MAX, FLT_MAX, 1, false);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_xz_ypr(Network::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid = getViewEntityIDFromStream(s);

	float x, z, y, p, r;
	s >> x;
	s >> z;
	s >> y;
	s >> p;
	s >> r;

	_updateVolatileData(eid, x, FLT_MAX, z, r, p, y, 1, false);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_xz_yp(Network::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid = getViewEntityIDFromStream(s);

	float x, z, y, p;
	s >> x;
	s >> z;
	s >> y;
	s >> p;

	_updateVolatileData(eid, x, FLT_MAX, z, FLT_MAX, p, y, 1, false);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_xz_yr(Network::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid = getViewEntityIDFromStream(s);

	float x, z, y, r;
	s >> x;
	s >> z;
	s >> y;
	s >> r;

	_updateVolatileData(eid, x, FLT_MAX, z, r, FLT_MAX, y, 1, false);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_xz_pr(Network::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid = getViewEntityIDFromStream(s);

	float x, z, p, r;
	s >> x;
	s >> z;
	s >> p;
	s >> r;

	_updateVolatileData(eid, x, FLT_MAX, z, r, p, FLT_MAX, 1, false);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_xz_y(Network::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid = getViewEntityIDFromStream(s);

	float x, z, y;
	s >> x;
	s >> z;
	s >> y;

	_updateVolatileData(eid, x, FLT_MAX, z, FLT_MAX, FLT_MAX, y, 1, false);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_xz_p(Network::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid = getViewEntityIDFromStream(s);

	float x, z, p;
	s >> x;
	s >> z;
	s >> p;

	_updateVolatileData(eid, x, FLT_MAX, z, FLT_MAX, p, FLT_MAX, 1, false);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_xz_r(Network::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid = getViewEntityIDFromStream(s);

	float x, z, r;
	s >> x;
	s >> z;
	s >> r;

	_updateVolatileData(eid, x, FLT_MAX, z, r, FLT_MAX, FLT_MAX, 1, false);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_xyz(Network::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid = getViewEntityIDFromStream(s);

	float x, y, z;
	s >> x;
	s >> y;
	s >> z;

	_updateVolatileData(eid, x, y, z, FLT_MAX, FLT_MAX, FLT_MAX, 0, false);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_xyz_ypr(Network::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid = getViewEntityIDFromStream(s);

	float x, y, z;
	s >> x;
	s >> y;
	s >> z;
	
	float yaw, p, r;
	s >> yaw;
	s >> p;
	s >> r;

	_updateVolatileData(eid, x, y, z, r, p, yaw, 0, false);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_xyz_yp(Network::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid = getViewEntityIDFromStream(s);

	float x, y, z;
	s >> x;
	s >> y;
	s >> z;

	float yaw, p;
	s >> yaw;
	s >> p;

	_updateVolatileData(eid, x, y, z, FLT_MAX, p, yaw, 0, false);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_xyz_yr(Network::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid = getViewEntityIDFromStream(s);

	float x, y, z;
	s >> x;
	s >> y;
	s >> z;

	float yaw, r;
	s >> yaw;
	s >> r;
	
	_updateVolatileData(eid, x, y, z, r, FLT_MAX, yaw, 0, false);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_xyz_pr(Network::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid = getViewEntityIDFromStream(s);

	float x, y, z;
	s >> x;
	s >> y;
	s >> z;

	float p, r;
	s >> p;
	s >> r;

	_updateVolatileData(eid, x, y, z, r, p, FLT_MAX, 0, false);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_xyz_y(Network::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid = getViewEntityIDFromStream(s);

	float x, y, z;
	s >> x;
	s >> y;
	s >> z;

	float yaw;
	s >> yaw;

	_updateVolatileData(eid, x, y, z, FLT_MAX, FLT_MAX, yaw, 0, false);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_xyz_p(Network::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid = getViewEntityIDFromStream(s);

	float x, y, z;
	s >> x;
	s >> y;
	s >> z;

	float p;
	s >> p;

	_updateVolatileData(eid, x, y, z, FLT_MAX, p, FLT_MAX, 0, false);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_xyz_r(Network::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid = getViewEntityIDFromStream(s);

	float x, y, z;
	s >> x;
	s >> y;
	s >> z;

	float r;
	s >> r;

	_updateVolatileData(eid, x, y, z, r, FLT_MAX, FLT_MAX, 0, false);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::_updateVolatileData(ENTITY_ID entityID, float x, float y, float z, 
										   float roll, float pitch, float yaw, int8 isOnGround, bool isOptimized)
{
	client::Entity* entity = pEntities_->find(entityID);
	if(entity == NULL)
	{
		// 如果为0且客户端上一步是重登陆或者重连操作并且服务端entity在断线期间一直处于在线状态
		// 则可以忽略这个错误, 因为cellapp可能一直在向baseapp发送同步消息， 当客户端重连上时未等
		// 服务端初始化步骤开始则收到同步信息, 此时这里就会出错。
		ERROR_MSG(fmt::format("ClientObjectBase::onUpdateData_xz_yp: not found entity({}).\n", entityID));
		return;
	}

	client::Entity* player = pPlayer();
	if(player == NULL)
	{
		ERROR_MSG(fmt::format("ClientObjectBase::_updateVolatileData: not found player({}).\n", entityID_));
		return;
	}

	// 小于0不设置
	if(isOnGround >= 0)
		entity->isOnGround(isOnGround > 0);

        bool positionChanged = x != FLT_MAX || y != FLT_MAX || z != FLT_MAX;
        if (x == FLT_MAX) x = 0.0f;
        if (y == FLT_MAX) y = 0.0f;
        if (z == FLT_MAX) z = 0.0f;
        
	if(positionChanged)
	{
		Position3D relativePos;
		relativePos.x = x;
		relativePos.y = y;
		relativePos.z = z;

		if (isOptimized)
		{
			Position3D basepos = player->serverPosition();
			basepos += relativePos;

			entity->position(basepos);
		}
		else
		{
			entity->position(relativePos);
		}

		// DEBUG_MSG(fmt::format("ClientObjectBase::_updateVolatileData: {}-{}-{}--{}-{}-{}-\n", 
		//	x, y, z, entity->position().x, entity->position().y, entity->position().z));
	}

	Direction3D dir = entity->direction();
	
	if(yaw != FLT_MAX)
		dir.yaw(yaw);

	if(pitch != FLT_MAX)
		dir.pitch(pitch);

	if(roll != FLT_MAX)
		dir.roll(roll);

	if(yaw != FLT_MAX || pitch != FLT_MAX || roll != FLT_MAX)
		entity->direction(dir);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onStreamDataStarted(Network::Channel* pChannel, int16 id, uint32 datasize, std::string& descr)
{
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onStreamDataRecv(Network::Channel* pChannel, MemoryStream& s)
{
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onStreamDataCompleted(Network::Channel* pChannel, int16 id)
{
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onControlEntity(Network::Channel* pChannel, int32 eid, int8 p_isControlled)
{
    client::Entity* entity = pEntities()->find(eid);
    if(entity == NULL)
    {
        ERROR_MSG(fmt::format("ClientObjectBase::onControlEntity:entity({}) not found.\n", eid));
        return;
    }

    bool controlled = (p_isControlled != 0);
    if (controlled == true)
    {
        if(eid != entityID())
            controlledEntities_.push_back(entity);
    }
    else
        controlledEntities_.remove(entity);

    entity->onControlled(controlled);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::addSpaceGeometryMapping(SPACE_ID spaceID, const std::string& respath)
{
	INFO_MSG(fmt::format("ClientObjectBase::addSpaceGeometryMapping: spaceID={}, respath={}!\n",
		spaceID, respath));

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
		for(; iter != pEntities_->getEntities().end(); ++iter)
		{
			client::Entity* pEntity = static_cast<client::Entity*>(iter->second.get());
			if(pEntity->id() == this->entityID())
			{
				pEntity->spaceID(0);
				continue;
			}

			EventData_LeaveWorld eventdata;
			eventdata.spaceID = pEntity->spaceID();
			eventdata.entityID = pEntity->id();

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
		for(; iter != pEntities_->getEntities().end(); ++iter)
		{
			client::Entity* pEntity = static_cast<client::Entity*>(iter->second.get());

			EventData_LeaveWorld eventdata;
			eventdata.spaceID = pEntity->spaceID();
			eventdata.entityID = pEntity->id();

			pEntity->onLeaveWorld();

			eventHandler_.fire(&eventdata);
		}
	}

	pEntityIDAliasIDList_.clear();
	spacedatas_.clear();
	bufferedCreateEntityMessage_.clear();

	isLoadedGeometry_ = false;
	spaceID_ = 0;
	targetID_ = 0;
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::initSpaceData(Network::Channel* pChannel, MemoryStream& s)
{
	clearSpace(false);

	s >> spaceID_;

	client::Entity* player = pPlayer();
	if(player)
	{
		player->spaceID(spaceID_);
	}

	std::string key, value;
	
	while(s.length() > 0)
	{
		s >> key >> value;
		setSpaceData(pChannel, spaceID_, key, value);
	}

	DEBUG_MSG(fmt::format("ClientObjectBase::initSpaceData: spaceID({}), datasize={}.\n", 
		spaceID_, spacedatas_.size()));
}

//-------------------------------------------------------------------------------------
const std::string& ClientObjectBase::getGeometryPath()
{ 
	return getSpaceData("_mapping"); 
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::setSpaceData(Network::Channel* pChannel, SPACE_ID spaceID, const std::string& key, const std::string& value)
{
	if(spaceID_ != spaceID)
	{
		ERROR_MSG(fmt::format("ClientObjectBase::setSpaceData: spaceID(curr:{}->{}) not match, key={}, value={}.\n", 
			spaceID_, spaceID, key, value));
		return;
	}

	DEBUG_MSG(fmt::format("ClientObjectBase::setSpaceData: spaceID({}), key={}, value={}.\n", 
		spaceID_, key, value));

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
void ClientObjectBase::delSpaceData(Network::Channel* pChannel, SPACE_ID spaceID, const std::string& key)
{
	if(spaceID_ != spaceID)
	{
		ERROR_MSG(fmt::format("ClientObjectBase::delSpaceData: spaceID(curr:{}->{}) not match, key={}.\n", 
			spaceID_, spaceID, key));
		return;
	}

	DEBUG_MSG(fmt::format("ClientObjectBase::delSpaceData: spaceID({}), key={}.\n", 
		spaceID_, key));

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
		PyErr_Format(PyExc_TypeError, "KBEngine::getSpaceData: (argssize != (key)) error!");
		PyErr_PrintEx(0);
		S_Return
	}
	
	char* key = NULL;
	if(!PyArg_ParseTuple(args, "s",  &key))
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::getSpaceData: args error!");
		PyErr_PrintEx(0);
		S_Return
	}
	
	ClientObjectBase* pClientObjectBase = static_cast<ClientObjectBase*>(self);

	if(!pClientObjectBase->hasSpaceData(key))
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::getSpaceData: (key=%s) not found!", 
			key);

		PyErr_PrintEx(0);
		S_Return
	}

	return PyUnicode_FromString(pClientObjectBase->getSpaceData(key).c_str());
}

//-------------------------------------------------------------------------------------
PyObject* ClientObjectBase::__py_getWatcher(PyObject* self, PyObject* args)
{
	int argCount = (int)PyTuple_Size(args);
	if(argCount != 1)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::getWatcher(): args[strpath] error!");
		PyErr_PrintEx(0);
		S_Return
	}
	
	char* path;

	if(!PyArg_ParseTuple(args, "s", &path))
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::getWatcher(): args[strpath] error!");
		PyErr_PrintEx(0);
		S_Return
	}

	//DebugHelper::getSingleton().setScriptMsgType(type);

	KBEShared_ptr< WatcherObject > pWobj = WatcherPaths::root().getWatcher(path);
	if(pWobj.get() == NULL)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::getWatcher(): not found watcher[%s]!", path);
		PyErr_PrintEx(0);
		S_Return
	}

	WATCHER_VALUE_TYPE wtype = pWobj->getType();
	PyObject* pyval = NULL;
	MemoryStream* stream = MemoryStream::createPoolObject(OBJECTPOOL_POINT);
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

	MemoryStream::reclaimPoolObject(stream);
	return pyval;
}

//-------------------------------------------------------------------------------------
PyObject* ClientObjectBase::__py_getWatcherDir(PyObject* self, PyObject* args)
{
	int argCount = (int)PyTuple_Size(args);
	if(argCount != 1)
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::getWatcherDir(): args[strpath] error!");
		PyErr_PrintEx(0);
		S_Return
	}
	
	char* path;

	if(!PyArg_ParseTuple(args, "s", &path))
	{
		PyErr_Format(PyExc_TypeError, "KBEngine::getWatcherDir(): args[strpath] error!");
		PyErr_PrintEx(0);
		S_Return
	}

	std::vector<std::string> vec;
	WatcherPaths::root().dirPath(path, vec);

	PyObject* pyTuple = PyTuple_New(vec.size());
	std::vector<std::string>::iterator iter = vec.begin();
	int i = 0;
	for(; iter != vec.end(); ++iter)
	{
		PyTuple_SET_ITEM(pyTuple, i++, PyUnicode_FromString((*iter).c_str()));
	}

	return pyTuple;
}

//-------------------------------------------------------------------------------------
PyObject* ClientObjectBase::__py_disconnect(PyObject* self, PyObject* args)
{
	ClientObjectBase* pClientObjectBase = static_cast<ClientObjectBase*>(self);
	pClientObjectBase->reset();
	pClientObjectBase->canReset(true);

	if(PyTuple_Size(args) == 1)
	{
		uint32 i = 0;
		if(!PyArg_ParseTuple(args, "I", &i))
		{
			PyErr_Format(PyExc_TypeError, "KBEngine::disconnect(): args[lock_secs] error!");
			PyErr_PrintEx(0);
			S_Return
		}

		pClientObjectBase->locktime(timestamp() + stampsPerSecond() * i);
	}

	S_Return;
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onAppActiveTickCB(Network::Channel* pChannel)
{
	pChannel->updateLastReceivedTime();
}

//-------------------------------------------------------------------------------------
PyObject* ClientObjectBase::__py_assert(PyObject* self, PyObject* args)
{
	KBE_ASSERT(false && "kbassert");
	return NULL;
}

//-------------------------------------------------------------------------------------		
}
