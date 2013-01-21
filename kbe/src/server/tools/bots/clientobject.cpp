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
#include "bots.hpp"
#include "clientobject.hpp"
#include "network/common.hpp"
#include "network/message_handler.hpp"
#include "network/tcp_packet.hpp"
#include "network/bundle.hpp"
#include "network/fixed_messages.hpp"
#include "thread/threadpool.hpp"
#include "server/componentbridge.hpp"
#include "server/components.hpp"
#include "server/serverconfig.hpp"
#include "entitydef/scriptdef_module.hpp"
#include "entitydef/entitydef.hpp"
#include "client_lib/client_interface.hpp"

#include "baseapp/baseapp_interface.hpp"
#include "cellapp/cellapp_interface.hpp"
#include "baseappmgr/baseappmgr_interface.hpp"
#include "cellappmgr/cellappmgr_interface.hpp"
#include "loginapp/loginapp_interface.hpp"


namespace KBEngine{

SCRIPT_METHOD_DECLARE_BEGIN(ClientObject)
SCRIPT_METHOD_DECLARE_END()

SCRIPT_MEMBER_DECLARE_BEGIN(ClientObject)
SCRIPT_MEMBER_DECLARE_END()

SCRIPT_GETSET_DECLARE_BEGIN(ClientObject)
SCRIPT_GET_DECLARE("id",							pyGetID,					0,					0)
SCRIPT_GET_DECLARE("entities",						pyGetEntities,				0,					0)
SCRIPT_GETSET_DECLARE_END()
SCRIPT_INIT(ClientObject, 0, 0, 0, 0, 0)		

static int32 g_appID = 1;

//-------------------------------------------------------------------------------------
ClientObject::ClientObject(std::string name):
ScriptObject(getScriptType(), false),
appID_(0),
pChannel_(new Mercury::Channel()),
name_(name),
password_(),
error_(C_ERROR_NONE),
state_(C_STATE_INIT),
entityID_(0),
dbid_(0),
ip_(),
port_(),
lastSentActiveTickTime_(timestamp()),
connectedGateway_(false),
pEntities_(new Entities<Entity>()),
pyCallbackMgr_(),
bufferedCreateEntityMessage_()
{
	pChannel_->incRef();

	appID_ = g_appID++;
}

//-------------------------------------------------------------------------------------
ClientObject::~ClientObject()
{
	pChannel_->decRef();

	pEntities_->finalise();
	S_RELEASE(pEntities_);
}

//-------------------------------------------------------------------------------------
PyObject* ClientObject::pyGetEntities()
{ 
	Py_INCREF(pEntities_);
	return pEntities_; 
}

//-------------------------------------------------------------------------------------
bool ClientObject::initCreate()
{
	Mercury::EndPoint* pEndpoint = new Mercury::EndPoint();
	
	pEndpoint->socket(SOCK_STREAM);
	if (!pEndpoint->good())
	{
		ERROR_MSG("ClientObject::initNetwork: couldn't create a socket\n");
		delete pEndpoint;
		error_ = C_ERROR_INIT_NETWORK_FAILED;
		return false;
	}
	
	ENGINE_COMPONENT_INFO& infos = g_kbeSrvConfig.getBots();
	u_int32_t address;

	pEndpoint->convertAddress(infos.login_ip, address);
	if(pEndpoint->connect(htons(infos.login_port), address) == -1)
	{
		ERROR_MSG(boost::format("ClientObject::initNetwork: connect server is error(%1%)!\n") %
			kbe_strerror());

		delete pEndpoint;
		error_ = C_ERROR_INIT_NETWORK_FAILED;
		return false;
	}

	Mercury::Address addr(infos.login_ip, infos.login_port);
	pEndpoint->addr(addr);

	pChannel_->endpoint(pEndpoint);
	pEndpoint->setnonblocking(true);
	pEndpoint->setnodelay(true);

	pChannel_->pMsgHandlers(&ClientInterface::messageHandlers);
	Bots::getSingleton().pEventPoller()->registerForRead((*pEndpoint), this);
	return true;
}

//-------------------------------------------------------------------------------------
bool ClientObject::processSocket(bool expectingPacket)
{
	
	Mercury::TCPPacket* pReceiveWindow = Mercury::TCPPacket::ObjPool().createObject();
	int len = pReceiveWindow->recvFromEndPoint(*pChannel_->endpoint());

	if (len < 0)
	{
		Mercury::TCPPacket::ObjPool().reclaimObject(pReceiveWindow);

		PacketReceiver::RecvState rstate = this->checkSocketErrors(len, expectingPacket);

		if(rstate == Mercury::PacketReceiver::RECV_STATE_INTERRUPT)
		{
			Bots::getSingleton().pEventPoller()->deregisterForRead(*pChannel_->endpoint());
			pChannel_->destroy();
			Bots::getSingleton().delClient(this);
			return false;
		}

		return rstate == Mercury::PacketReceiver::RECV_STATE_CONTINUE;
	}
	else if(len == 0) // 客户端正常退出
	{
		Mercury::TCPPacket::ObjPool().reclaimObject(pReceiveWindow);

		Bots::getSingleton().pEventPoller()->deregisterForRead(*pChannel_->endpoint());
		pChannel_->destroy();
		Bots::getSingleton().delClient(this);
		return false;
	}

	pChannel_->addReceiveWindow(pReceiveWindow);
	Mercury::Reason ret = this->processPacket(pChannel_, pReceiveWindow);

	if(ret != Mercury::REASON_SUCCESS)
	{
		ERROR_MSG(boost::format("ClientObject::processSocket: "
					"Throwing %1%\n") %
					Mercury::reasonToString(ret));
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool ClientObject::createAccount()
{
	// 创建账号
	Mercury::Bundle bundle;
	bundle.newMessage(LoginappInterface::reqCreateAccount);
	bundle << name_;
	bundle << password_;
	bundle.send(*pChannel_->endpoint());
	return true;
}

//-------------------------------------------------------------------------------------
void ClientObject::onCreateAccountResult(MemoryStream& s)
{
	SERVER_ERROR_CODE retcode;
	std::string datas = "";

	s >> retcode;
	s.readBlob(datas);

	if(retcode != 0)
	{
		error_ = C_ERROR_CREATE_FAILED;
		INFO_MSG(boost::format("ClientObject::onCreateAccountResult: %1% create is failed! code=%2%.\n") % name_ % retcode);
		return;
	}

	state_ = C_STATE_LOGIN;
	INFO_MSG(boost::format("ClientObject::onCreateAccountResult: %1% create is successfully!\n") % name_);
}

//-------------------------------------------------------------------------------------
bool ClientObject::login()
{
	if(error_ != C_ERROR_NONE)
		return false;

	Mercury::Bundle bundle;

	std::string bindatas = "bots client";

	// 提交账号密码请求登录
	bundle.newMessage(LoginappInterface::login);
	CLIENT_CTYPE tclient = CLIENT_TYPE_BOTS;
	bundle << tclient;
	bundle.appendBlob(bindatas);
	bundle << name_;
	bundle << password_;
	bundle.send(*pChannel_->endpoint());
	return true;
}

//-------------------------------------------------------------------------------------
bool ClientObject::initLoginGateWay()
{
	Bots::getSingleton().pEventPoller()->deregisterForRead(*pChannel_->endpoint());
	Mercury::EndPoint* pEndpoint = new Mercury::EndPoint();
	
	pEndpoint->socket(SOCK_STREAM);
	if (!pEndpoint->good())
	{
		ERROR_MSG("ClientObject::initLogin: couldn't create a socket\n");
		delete pEndpoint;
		error_ = C_ERROR_INIT_NETWORK_FAILED;
		return false;
	}
	
	u_int32_t address;

	pEndpoint->convertAddress(ip_.c_str(), address);
	if(pEndpoint->connect(htons(port_), address) == -1)
	{
		ERROR_MSG(boost::format("ClientObject::initLogin: connect server is error(%1%)!\n") %
			kbe_strerror());

		delete pEndpoint;
		error_ = C_ERROR_INIT_NETWORK_FAILED;
		return false;
	}

	Mercury::Address addr(ip_.c_str(), port_);
	pEndpoint->addr(addr);

	pChannel_->endpoint(pEndpoint);
	pEndpoint->setnonblocking(true);
	pEndpoint->setnodelay(true);

	Bots::getSingleton().pEventPoller()->registerForRead((*pEndpoint), this);
	connectedGateway_ = true;
	return true;
}

//-------------------------------------------------------------------------------------
bool ClientObject::loginGateWay()
{
	// 请求登录网关
	Mercury::Bundle bundle;
	bundle.newMessage(BaseappInterface::loginGateway);
	bundle << name_;
	bundle << password_;
	bundle.send(*pChannel_->endpoint());
	return true;
}

//-------------------------------------------------------------------------------------
void ClientObject::gameTick()
{
	if(pChannel()->endpoint())
	{
		pChannel()->handleMessage(NULL);
		sendTick();
	}

	switch(state_)
	{
		case C_STATE_INIT:

			state_ = C_STATE_PLAY;

			if(!initCreate() || !createAccount())
				return;

			break;
		case C_STATE_LOGIN:

			state_ = C_STATE_PLAY;

			if(!login())
				return;

			break;
		case C_STATE_LOGIN_GATEWAY:

			state_ = C_STATE_PLAY;

			if(!initLoginGateWay() || !loginGateWay())
				return;

			break;
		case C_STATE_PLAY:
			break;
		default:
			KBE_ASSERT(false);
			break;
	};
}

//-------------------------------------------------------------------------------------
void ClientObject::sendTick()
{
	// 向服务器发送tick
	uint64 check = uint64( Mercury::g_channelExternalTimeout * stampsPerSecond() ) / 2;
	if (timestamp() - lastSentActiveTickTime_ > check)
	{
		lastSentActiveTickTime_ = timestamp();

		Mercury::Bundle bundle;
		if(connectedGateway_)
			bundle.newMessage(BaseappInterface::onClientActiveTick);
		else
			bundle.newMessage(LoginappInterface::onClientActiveTick);
		bundle.send(*pChannel_->endpoint());
	}
}

//-------------------------------------------------------------------------------------	
Entity* ClientObject::createEntityCommon(const char* entityType, PyObject* params,
	bool isInitializeScript, ENTITY_ID eid, bool initProperty,
	EntityMailbox* base, EntityMailbox* cell)
{
	KBE_ASSERT(eid > 0);

	ScriptDefModule* sm = EntityDef::findScriptModule(entityType);
	if(sm == NULL)
	{
		PyErr_Format(PyExc_TypeError, "ClientObject::createEntityCommon: entity [%s] not found.\n", entityType);
		PyErr_PrintEx(0);
		return NULL;
	}
	else if(!sm->hasClient())
	{
		PyErr_Format(PyExc_TypeError, "Client::createEntityCommon: entity [%s] not found.\n", entityType);
		PyErr_PrintEx(0);
		return NULL;
	}

	PyObject* obj = sm->createObject();
	
	Entity* entity = new(obj) Entity(eid, sm, base, cell);

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
		INFO_MSG(boost::format("ClientObject::createEntityCommon: new %1% (%2%) refc=%3%.\n") % entityType % eid % obj->ob_refcnt);
	}
	else
	{
		INFO_MSG(boost::format("ClientObject::createEntityCommon: new %1% (%2%)\n") % entityType % eid);
	}

	return entity;
}

//-------------------------------------------------------------------------------------	
void ClientObject::onLoginSuccessfully(MemoryStream& s)
{
	std::string accountName, datas;

	s >> accountName;
	s >> ip_;
	s >> port_;
	s.readBlob(datas);

	INFO_MSG(boost::format("ClientObject::onLoginSuccessfully: %1% addr=%2%:%3%!\n") % name_ % ip_ % port_);

	state_ = C_STATE_LOGIN_GATEWAY;
}

//-------------------------------------------------------------------------------------	
void ClientObject::onLoginFailed(MemoryStream& s)
{
	SERVER_ERROR_CODE failedcode;
	std::string datas;

	s >> failedcode;
	s.readBlob(datas);

	INFO_MSG(boost::format("ClientObject::onLoginFailed: %1% failedcode=%2%!\n") % name_ % failedcode);

	error_ = C_ERROR_LOGIN_FAILED;
}

//-------------------------------------------------------------------------------------	
void ClientObject::onLoginGatewayFailed(SERVER_ERROR_CODE failedcode)
{
	INFO_MSG(boost::format("ClientObject::onLoginGatewayFailed: %1% failedcode=%2%!\n") % name_ % failedcode);
}

//-------------------------------------------------------------------------------------	
void ClientObject::onCreatedProxies(uint64 rndUUID, ENTITY_ID eid, std::string& entityType)
{
	entityID_ = eid;
	INFO_MSG(boost::format("ClientObject::onCreatedProxies(%1%): rndUUID=%2% eid=%3% entityType=%4%!\n") % 
		name_ % rndUUID % eid % entityType);

	// 设置entity的baseMailbox
	EntityMailbox* mailbox = new EntityMailbox(EntityDef::findScriptModule(entityType.c_str()), 
		NULL, appID(), eid, MAILBOX_TYPE_BASE);

	createEntityCommon(entityType.c_str(), NULL, true, eid, true, mailbox, NULL);
}

//-------------------------------------------------------------------------------------	
void ClientObject::onEntityEnterWorld(ENTITY_ID eid, ENTITY_SCRIPT_UID scriptType, SPACE_ID spaceID)
{
	Entity* entity = pEntities_->find(eid);
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

			createEntityCommon(sm->getName(), NULL, true, eid, true, NULL, mailbox);

			this->onUpdatePropertys(*iter->second.get());
			bufferedCreateEntityMessage_.erase(iter);
		}
		else
		{
			ERROR_MSG(boost::format("ClientObject::onEntityEnterWorld: not found entity(%1%).\n") % eid);
		}
		return;
	}

	DEBUG_MSG(boost::format("ClientObject::onEntityEnterWorld: %1%.\n") % eid);
	entity->onEnterWorld();
}

//-------------------------------------------------------------------------------------	
void ClientObject::onEntityLeaveWorld(ENTITY_ID eid, SPACE_ID spaceID)
{
	Entity* entity = pEntities_->find(eid);
	if(entity == NULL)
	{	
		ERROR_MSG(boost::format("ClientObject::onEntityLeaveWorld: not found entity(%1%).\n") % eid);
		return;
	}

	DEBUG_MSG(boost::format("ClientObject::onEntityLeaveWorld: %1%.\n") % eid);
	entity->onLeaveWorld();
	pEntities_->erase(eid);
}

//-------------------------------------------------------------------------------------	
void ClientObject::onEntityEnterSpace(SPACE_ID spaceID, ENTITY_ID eid)
{
	Entity* entity = pEntities_->find(eid);
	if(entity == NULL)
	{	
		ERROR_MSG(boost::format("ClientObject::onEntityEnterSpace: not found entity(%1%).\n") % eid);
		return;
	}

	DEBUG_MSG(boost::format("ClientObject::onEntityEnterSpace: %1%.\n") % eid);
}

//-------------------------------------------------------------------------------------	
void ClientObject::onEntityLeaveSpace(SPACE_ID spaceID, ENTITY_ID eid)
{
	Entity* entity = pEntities_->find(eid);
	if(entity == NULL)
	{	
		ERROR_MSG(boost::format("ClientObject::onEntityLeaveSpace: not found entity(%1%).\n") % eid);
		return;
	}

	DEBUG_MSG(boost::format("ClientObject::onEntityLeaveSpace: %1%.\n") % eid);
}

//-------------------------------------------------------------------------------------	
void ClientObject::onEntityDestroyed(ENTITY_ID eid)
{
	Entity* entity = pEntities_->find(eid);
	if(entity == NULL)
	{	
		ERROR_MSG(boost::format("ClientObject::onEntityDestroyed: not found entity(%1%).\n") % eid);
		return;
	}

	DEBUG_MSG(boost::format("ClientObject::onEntityDestroyed: %1%.\n") % eid);
	pEntities_->erase(eid);
}

//-------------------------------------------------------------------------------------
void ClientObject::onRemoteMethodCall(KBEngine::MemoryStream& s)
{
	ENTITY_ID eid;
	s >> eid;

	Entity* entity = pEntities_->find(eid);
	if(entity == NULL)
	{	
		s.opfini();
		ERROR_MSG(boost::format("ClientObject::onRemoteMethodCall: not found entity(%1%).\n") % eid);
		return;
	}

	entity->onRemoteMethodCall(this->pChannel(), s);
}

//-------------------------------------------------------------------------------------
void ClientObject::onUpdatePropertys(MemoryStream& s)
{
	ENTITY_ID eid;
	s >> eid;

	Entity* entity = pEntities_->find(eid);
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
			ERROR_MSG(boost::format("ClientObject::onUpdatePropertys: not found entity(%1%).\n") % eid);
		}
		return;
	}

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

	while(s.opsize() > 0)
	{
		ENTITY_PROPERTY_UID uid;
		s >> uid;

		// 如果是位置或者朝向信息则
		if(uid == posuid)
		{
			Position3D pos;
			ArraySize size;

#ifdef CLIENT_NO_FLOAT		
			int32 x, y, z;
			s >> size >> x >> y >> z;

			pos.x = (float)x;
			pos.y = (float)y;
			pos.z = (float)z;
#else
			s >> size >> pos.x >> pos.y >> pos.z;
#endif
			entity->setPosition(pos);
			continue;
		}
		else if(uid == diruid)
		{
			Direction3D dir;
			ArraySize size;

#ifdef CLIENT_NO_FLOAT		
			int32 x, y, z;
			s >> size >> x >> y >> z;

			dir.roll = (float)x;
			dir.pitch = (float)y;
			dir.yaw = (float)z;
#else
			s >> size >> dir.roll >> dir.pitch >> dir.yaw;
#endif

			entity->setDirection(dir);
			continue;
		}
		else if(uid == spaceuid)
		{
			SPACE_ID spaceID;
			s >> spaceID;
			entity->setSpaceID(spaceID);
			continue;
		}

		PropertyDescription* pPropertyDescription = entity->getScriptModule()->findClientPropertyDescription(uid);
		PyObject* pyobj = pPropertyDescription->createFromStream(&s);
		PyObject_SetAttrString(entity, pPropertyDescription->getName(), pyobj);
		Py_DECREF(pyobj);
	}
}

//-------------------------------------------------------------------------------------
}
