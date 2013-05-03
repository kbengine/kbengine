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
#include "clientobjectbase.hpp"
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
entityPos_(FLT_MAX, FLT_MAX, FLT_MAX),
entityDir_(FLT_MAX, FLT_MAX, FLT_MAX),
dbid_(0),
ip_(),
port_(),
lastSentActiveTickTime_(timestamp()),
connectedGateway_(false),
canReset_(false),
name_(),
password_(),
extradatas_("unknown"),
typeClient_(CLIENT_TYPE_PC),
bufferedCreateEntityMessage_(),
eventHandler_(),
ninterface_(ninterface)
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
		}

		return;
	}

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
bool ClientObjectBase::destroyEntity(ENTITY_ID entityID)
{
	return pEntities_->erase(entityID) != NULL;
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

#ifdef unix
	::close(*pChannel->endpoint());
#elif defined(PLAYSTATION3)
	::socketclose(*pChannel->endpoint());
#else
	::closesocket(*pChannel->endpoint());
#endif
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
	eventdata.pEntity = entity->getAspect();
	eventHandler_.fire(&eventdata);
	
	if(entityID_ > 0 && entityID_ != eid)
		pEntityIDAliasIDList_.push_back(eid);

	return entity;
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
		return false;
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
bool ClientObjectBase::login()
{
	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();

	// 提交账号密码请求登录
	(*pBundle).newMessage(LoginappInterface::login);
	(*pBundle) << typeClient_;
	(*pBundle).appendBlob(extradatas_);
	(*pBundle) << name_;
	(*pBundle) << password_;
	
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
void ClientObjectBase::onEntityEnterWorld(Mercury::Channel * pChannel, ENTITY_ID eid, ENTITY_SCRIPT_UID scriptType, SPACE_ID spaceID)
{
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
		}
		else
		{
			ERROR_MSG(boost::format("ClientObjectBase::onEntityEnterWorld: not found entity(%1%).\n") % eid);
			return;
		}
	}
	else
	{
		KBE_ASSERT(entity->getCellMailbox() == NULL);

		// 设置entity的cellMailbox
		EntityMailbox* mailbox = new EntityMailbox(entity->getScriptModule(), 
			NULL, appID(), eid, MAILBOX_TYPE_CELL);

		entity->setCellMailbox(mailbox);
	}

	DEBUG_MSG(boost::format("ClientObjectBase::onEntityEnterWorld: %1%(%2%).\n") % 
		entity->getScriptName() % eid);

	EventData_EnterWorld eventdata;
	eventdata.spaceID = spaceID;
	eventdata.pEntity = entity->getAspect();
	eventdata.x = entity->getPosition().x;
	eventdata.y = entity->getPosition().y;
	eventdata.z = entity->getPosition().z;
	eventdata.pitch = entity->getDirection().pitch;
	eventdata.roll = entity->getDirection().roll;
	eventdata.yaw = entity->getDirection().yaw;
	eventdata.speed = entity->getMoveSpeed();
	eventHandler_.fire(&eventdata);

	entity->onEnterWorld();

	if(entityID_ == eid)
	{
		entityPos_ = entity->getPosition();
		entityDir_ = entity->getDirection();
	}
}

//-------------------------------------------------------------------------------------	
void ClientObjectBase::onEntityLeaveWorld(Mercury::Channel * pChannel, ENTITY_ID eid, SPACE_ID spaceID)
{
	client::Entity* entity = pEntities_->find(eid);
	if(entity == NULL)
	{	
		ERROR_MSG(boost::format("ClientObjectBase::onEntityLeaveWorld: not found entity(%1%).\n") % eid);
		return;
	}

	DEBUG_MSG(boost::format("ClientObjectBase::onEntityLeaveWorld: %1%(%2%).\n") % 
		entity->getScriptName() % eid);

	entity->onLeaveWorld();

	EventData_LeaveWorld eventdata;
	eventdata.spaceID = spaceID;
	eventdata.pEntity = entity->getAspect();

	eventHandler_.fire(&eventdata);

	pEntities_->erase(eid);

	if(entityID_ != eid)
		std::remove(pEntityIDAliasIDList_.begin(), pEntityIDAliasIDList_.end(), eid);
}

//-------------------------------------------------------------------------------------	
void ClientObjectBase::onEntityEnterSpace(Mercury::Channel * pChannel, SPACE_ID spaceID, ENTITY_ID eid)
{
	client::Entity* entity = pEntities_->find(eid);
	if(entity == NULL)
	{	
		ERROR_MSG(boost::format("ClientObjectBase::onEntityEnterSpace: not found entity(%1%).\n") % eid);
		return;
	}

	DEBUG_MSG(boost::format("ClientObjectBase::onEntityEnterSpace: %1%(%2%).\n") % 
		entity->getScriptName() % eid);

	EventData_EnterSpace eventdata;
	eventdata.spaceID = spaceID;
	eventdata.pEntity = entity->getAspect();

	eventHandler_.fire(&eventdata);
}

//-------------------------------------------------------------------------------------	
void ClientObjectBase::onEntityLeaveSpace(Mercury::Channel * pChannel, SPACE_ID spaceID, ENTITY_ID eid)
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
	eventdata.spaceID = spaceID;
	eventdata.pEntity = entity->getAspect();
	eventHandler_.fire(&eventdata);
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

	pEntities_->erase(eid);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onRemoteMethodCall(Mercury::Channel * pChannel, KBEngine::MemoryStream& s)
{
	ENTITY_ID eid;
	s >> eid;

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
	ENTITY_ID eid;
	s >> eid;

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
	client::Entity* pEntity = pEntities_->find(entityID_);
	if(pEntity == NULL || !connectedGateway_ || 
		pServerChannel_ == NULL || pEntity->getCellMailbox() == NULL)
		return;

	Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
	(*pBundle).newMessage(BaseappInterface::onUpdateDataFromClient);
	
	pEntity->setPosition(entityPos_);
	pEntity->setDirection(entityDir_);

	(*pBundle) << pEntity->getPosition().x;
	(*pBundle) << pEntity->getPosition().y;
	(*pBundle) << pEntity->getPosition().z;

	(*pBundle) << pEntity->getDirection().yaw;
	(*pBundle) << pEntity->getDirection().pitch;
	(*pBundle) << pEntity->getDirection().roll;

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
	s >> pos.x >> pos.y >> pos.z >> dir.yaw >> dir.pitch >> dir.roll;

	entity->setPosition(pos);
	entity->setDirection(dir);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData(Mercury::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid;
	s >> eid;

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
	ENTITY_ID eid;
	s >> eid;

	float y, p, r;

	int8 angle;

	s >> angle;
	y = int82angle(angle);

	s >> angle;
	p = int82angle(angle);

	s >> angle;
	r = int82angle(angle);

	_updateVolatileData(eid, 0.f, 0.f, 0.f, y, p, r);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_yp(Mercury::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid;
	s >> eid;

	float y, p;

	int8 angle;

	s >> angle;
	y = int82angle(angle);

	s >> angle;
	p = int82angle(angle);

	_updateVolatileData(eid, 0.f, 0.f, 0.f, y, p, FLT_MAX);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_yr(Mercury::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid;
	s >> eid;

	float y, r;

	int8 angle;

	s >> angle;
	y = int82angle(angle);

	s >> angle;
	r = int82angle(angle);

	_updateVolatileData(eid, 0.f, 0.f, 0.f, y, FLT_MAX, r);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_pr(Mercury::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid;
	s >> eid;

	float p, r;

	int8 angle;

	s >> angle;
	p = int82angle(angle);

	s >> angle;
	r = int82angle(angle);

	_updateVolatileData(eid, 0.f, 0.f, 0.f, FLT_MAX, p, r);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_y(Mercury::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid;
	s >> eid;

	float y;

	int8 angle;

	s >> angle;
	y = int82angle(angle);

	_updateVolatileData(eid, 0.f, 0.f, 0.f, y, FLT_MAX, FLT_MAX);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_p(Mercury::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid;
	s >> eid;

	float p;

	int8 angle;

	s >> angle;
	p = int82angle(angle);

	_updateVolatileData(eid, 0.f, 0.f, 0.f, FLT_MAX, p, FLT_MAX);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_r(Mercury::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid;
	s >> eid;

	float r;

	int8 angle;

	s >> angle;
	r = int82angle(angle);

	_updateVolatileData(eid, 0.f, 0.f, 0.f, FLT_MAX, FLT_MAX, r);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_xz(Mercury::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid;
	s >> eid;

	float x,z;

	s.readPackXZ(x, z);


	_updateVolatileData(eid, x, 0.f, z, FLT_MAX, FLT_MAX, FLT_MAX);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_xz_ypr(Mercury::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid;
	s >> eid;

	float x, z, y, p, r;
	
	s.readPackXZ(x, z);

	int8 angle;

	s >> angle;
	y = int82angle(angle);

	s >> angle;
	p = int82angle(angle);

	s >> angle;
	r = int82angle(angle);

	_updateVolatileData(eid, x, 0.f, z, y, p, r);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_xz_yp(Mercury::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid;
	s >> eid;

	float x,z, y, p;

	s.readPackXZ(x, z);

	int8 angle;

	s >> angle;
	y = int82angle(angle);

	s >> angle;
	p = int82angle(angle);

	_updateVolatileData(eid, x, 0.f, z, y, p, FLT_MAX);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::_updateVolatileData(ENTITY_ID entityID, float x, float y, float z, 
										   float yaw, float pitch, float roll)
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
		dir.yaw = yaw;

	if(pitch != FLT_MAX)
		dir.pitch = pitch;

	if(roll != FLT_MAX)
		dir.roll = roll;

	entity->setDirection(dir);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_xz_yr(Mercury::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid;
	s >> eid;

	float x, z, y,  r;
	
	s.readPackXZ(x, z);

	int8 angle;

	s >> angle;
	y = int82angle(angle);

	s >> angle;
	r = int82angle(angle);

	_updateVolatileData(eid, x, 0.f, z, y, FLT_MAX, r);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_xz_pr(Mercury::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid;
	s >> eid;

	float x, z, p, r;
	
	s.readPackXZ(x, z);

	int8 angle;

	s >> angle;
	p = int82angle(angle);

	s >> angle;
	r = int82angle(angle);

	_updateVolatileData(eid, x, 0.f, z, FLT_MAX, p, r);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_xz_y(Mercury::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid;
	s >> eid;

	float x, z, y;
	
	s.readPackXZ(x, z);

	int8 angle;

	s >> angle;
	y = int82angle(angle);

	_updateVolatileData(eid, x, 0.f, z, y, FLT_MAX, FLT_MAX);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_xz_p(Mercury::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid;
	s >> eid;

	float x, z, p;
	
	s.readPackXZ(x, z);

	int8 angle;

	s >> angle;
	p = int82angle(angle);

	_updateVolatileData(eid, x, 0.f, z, FLT_MAX, p, FLT_MAX);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_xz_r(Mercury::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid;
	s >> eid;

	float x, z, r;
	
	s.readPackXZ(x, z);

	int8 angle;

	s >> angle;
	r = int82angle(angle);

	_updateVolatileData(eid, x, 0.f, z, FLT_MAX, FLT_MAX, r);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_xyz(Mercury::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid;
	s >> eid;

	float x, z, y;
	
	s.readPackXZ(x, z);
	s.readPackY(y);

	_updateVolatileData(eid, x, y, z, FLT_MAX, FLT_MAX, FLT_MAX);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_xyz_ypr(Mercury::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid;
	s >> eid;

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

	_updateVolatileData(eid, x, y, z, yaw, p, r);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_xyz_yp(Mercury::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid;
	s >> eid;

	float x, z, y;
	
	s.readPackXZ(x, z);
	s.readPackY(y);

	float yaw, p;

	int8 angle;

	s >> angle;
	yaw = int82angle(angle);

	s >> angle;
	p = int82angle(angle);

	_updateVolatileData(eid, x, y, z, yaw, p, FLT_MAX);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_xyz_yr(Mercury::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid;
	s >> eid;

	float x, z, y;
	
	s.readPackXZ(x, z);
	s.readPackY(y);

	float yaw, r;

	int8 angle;

	s >> angle;
	yaw = int82angle(angle);

	s >> angle;
	r = int82angle(angle);

	_updateVolatileData(eid, x, y, z, yaw, FLT_MAX, r);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_xyz_pr(Mercury::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid;
	s >> eid;

	float x, z, y;
	
	s.readPackXZ(x, z);
	s.readPackY(y);

	float p, r;

	int8 angle;

	s >> angle;
	p = int82angle(angle);

	s >> angle;
	r = int82angle(angle);

	_updateVolatileData(eid, x, y, z, FLT_MAX, p, r);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_xyz_y(Mercury::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid;
	s >> eid;

	float x, z, y;
	
	s.readPackXZ(x, z);
	s.readPackY(y);

	float yaw;

	int8 angle;

	s >> angle;
	yaw = int82angle(angle);

	_updateVolatileData(eid, x, y, z, yaw, FLT_MAX, FLT_MAX);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_xyz_p(Mercury::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid;
	s >> eid;

	float x, z, y;
	
	s.readPackXZ(x, z);
	s.readPackY(y);

	float p;

	int8 angle;

	s >> angle;
	p = int82angle(angle);

	_updateVolatileData(eid, x, y, z, FLT_MAX, p, FLT_MAX);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_xyz_r(Mercury::Channel* pChannel, MemoryStream& s)
{
	ENTITY_ID eid;
	s >> eid;

	float x, z, y;
	
	s.readPackXZ(x, z);
	s.readPackY(y);

	float r;

	int8 angle;

	s >> angle;
	r = int82angle(angle);

	_updateVolatileData(eid, x, y, z, FLT_MAX, FLT_MAX, r);
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
}
