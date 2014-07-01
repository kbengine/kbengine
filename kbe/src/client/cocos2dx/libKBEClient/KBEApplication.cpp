//
//  KNetworkInterface.cpp
//  libKBEClient
//
//  Created by Tom on 6/12/14.
//  Copyright (c) 2014 Tom. All rights reserved.
//

#include "KBEApplication.h"
#include "KMessage.h"
#include "KEntitydef.h"

namespace KBEngineClient
{

	float int82angle(int8 angle, bool half = false)
	{
		return float(angle) * float((KBE_PI / (half ? 254.f : 128.f)));
	}

void KBE_init()
{
	KMessage::bindmessage();
	EntityDef::EntityDef_Bind();
	//Test send ["Loginapp_importClientMessages"] 


}

void KBE_run()
{
	
}

//
//-------------------------------------------------------------------------------------
ClientObjectBase::~ClientObjectBase()
{
}

//-------------------------------------------------------------------------------------	
KBEngineClient::Entity* ClientObjectBase::createEntityCommon(const char* entityType, void* params,
	bool isInitializeScript, ENTITY_ID eid, bool initProperty,
	MailBox* base,  MailBox* cell)
{
	
	KBEngineClient::Entity* entity = new KBEngineClient::Entity();//eid, 0, base, cell);
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
bool ClientObjectBase::createAccount()
{
	// 创建账号
	//Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();
	KBundle* pBundle = new KBundle();
	(*pBundle).newmessage( * KMessage::messages["Loginapp_importClientMessages"]);
	(*pBundle).writeString( name_);
	(*pBundle).writeString(password_);
	pBundle->send( * pServerChannel_ );
 
	return true;
}
//-------------------------------------------------------------------------------------	
void ClientObjectBase::onHelloCB_(const std::string& verInfo, 
		COMPONENT_TYPE componentType)
{
}

//-------------------------------------------------------------------------------------	
void ClientObjectBase::onHelloCB(MemoryStream& s)
{
	std::string verInfo;
	s >> verInfo;
	
	//COMPONENT_TYPE ctype;
	//s >> ctype;

	 
	int32 ctype;
	s >> ctype;
	printf( "KBEngine::Client_onHelloCB: verInfo( %s ), ctype( %d )! " ,  verInfo.c_str()  , ctype  );

	onHelloCB_(verInfo, (COMPONENT_TYPE)ctype);
}

//
//-------------------------------------------------------------------------------------	
void ClientObjectBase::onVersionNotMatch( MemoryStream& s)
{
	std::string serverVersion_;
	s >> serverVersion_;
	printf(" serverVersion is %s" ,serverVersion_.c_str());

}

//-------------------------------------------------------------------------------------
bool ClientObjectBase::login()
{
	//Mercury::Bundle* pBundle = Mercury::Bundle::ObjPool().createObject();

	//// 提交账号密码请求登录
	//(*pBundle).newMessage(LoginappInterface::login);
	//(*pBundle) << typeClient_;
	//(*pBundle).appendBlob(extradatas_);
	//(*pBundle) << name_;
	//(*pBundle) << password_;
	//(*pBundle) << EntityDef::md5().getDigestStr();
	//pServerChannel_->pushBundle(pBundle);
	//
	// 
	KBundle* bundle = new KBundle(); 
	bundle->newmessage( *KMessage::messages["Loginapp_login"]);
	int8 clientType = 5;
	bundle->writeInt8(clientType); // clientType
	bundle->writeBlob( new char[0], 0);
	std::string username = "kbe121";
	std::string password = "111111";
	bundle->writeString(name_);
	bundle->writeString(password_);
	bundle->send( *pServerChannel_ );


	connectedGateway_ = false;
	return true;
}

//-------------------------------------------------------------------------------------
bool ClientObjectBase::loginGateWay()
{
	// 请求登录网关
	connectedGateway_ = false;
	return true;
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onCreateAccountResult(MemoryStream& s)
{
	SERVER_ERROR_CODE retcode;

	s >> retcode;
	s.readBlob(extradatas_);

	if(retcode != 0)
	{
		//INFO_MSG(boost::format("ClientObjectBase::onCreateAccountResult: %1% create is failed! code=%2%.\n") % 
		//	name_ % retcode);

		return;
	}

	//INFO_MSG(boost::format("ClientObjectBase::onCreateAccountResult: %1% create is successfully!\n") % name_);
}

//-------------------------------------------------------------------------------------	
void ClientObjectBase::onLoginSuccessfully(MemoryStream& s)
{
	std::string accountName;

	s >> accountName;
	s >> ip_;
	s >> port_;
	s.readBlob(extradatas_);
	
	connectedGateway_ = false;

}

//-------------------------------------------------------------------------------------	
void ClientObjectBase::onLoginFailed( MemoryStream& s)
{
	SERVER_ERROR_CODE failedcode;

	s >> failedcode;
	s.readBlob(extradatas_);
	
	connectedGateway_ = false;

}

//-------------------------------------------------------------------------------------	
void ClientObjectBase::onLoginGatewayFailed(SERVER_ERROR_CODE failedcode)
{
	//INFO_MSG(boost::format("ClientObjectBase::onLoginGatewayFailed: %1% failedcode=%2%!\n") % name_ % failedcode);
	connectedGateway_ = false;
	
}

//-------------------------------------------------------------------------------------	
void ClientObjectBase::onCreatedProxies(uint64 rndUUID, ENTITY_ID eid, std::string& entityType)
{
	if(entityID_ == 0)
	{
		/*EventData_LoginGatewaySuccess eventdata;
		eventHandler_.fire(&eventdata);*/
	}

	connectedGateway_ = true;
	entityID_ = eid;
	//
	entity_type_ = entityType;

	//cocos2d::CCLog("ClientObject::onCreatedProxies(%s): rndUUID=%d% eid=%d% entityType=%s%!\n",name_.c_str(),rndUUID , eid , entityType.c_str() );

	//// 设置entity的baseMailbox
	//EntityMailbox* mailbox = new EntityMailbox(EntityDef::findScriptModule(entityType.c_str()), 
	//	NULL, appID(), eid, MAILBOX_TYPE_BASE);

	/*MailBox* mailbox = new MailBox();
	createEntityCommon(entityType.c_str(), NULL, true, eid, true, mailbox, NULL);*/
}

//-------------------------------------------------------------------------------------	
void ClientObjectBase::onEntityEnterWorld(MemoryStream& s)
{
	ENTITY_ID eid = 0;
	ENTITY_SCRIPT_UID scriptType;
	
	s >> eid;
	
}

//-------------------------------------------------------------------------------------	
void ClientObjectBase::onEntityLeaveWorldOptimized( MemoryStream& s)
{
	onEntityLeaveWorld( getAoiEntityIDFromStream(s));
}

//-------------------------------------------------------------------------------------	
void ClientObjectBase::onEntityLeaveWorld(  ENTITY_ID eid)
{

}

//-------------------------------------------------------------------------------------	
void ClientObjectBase::onEntityEnterSpace( SPACE_ID spaceID, ENTITY_ID eid)
{
	
}

//-------------------------------------------------------------------------------------	
void ClientObjectBase::onEntityLeaveSpace( SPACE_ID spaceID, ENTITY_ID eid)
{
	
}

//-------------------------------------------------------------------------------------	
void ClientObjectBase::onEntityDestroyed( ENTITY_ID eid)
{
	ENTITIES_MAP::iterator it = pEntities_->find(eid);
	KBEngineClient::Entity* entity = 0;
	
	if (it!= pEntities_->end())
		entity = it->second;
	//

	if(entity == NULL)
	{	
		cocos2d::CCLog("ClientObjectBase::onEntityDestroyed: not found entity(%d%).\n" , eid);
		return;
	}

	cocos2d::CCLog("ClientObjectBase::onEntityDestroyed: %s%(%d%).\n", entity->classtype.c_str()  ,eid);
	//? manual destroy.
	if(entity->inWorld)
		entity->leaveWorld();

 	pEntities_->erase(eid);
	delete entity;
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onRemoteMethodCallOptimized( MemoryStream& s)
{
	ENTITY_ID eid = getAoiEntityIDFromStream(s);
	onRemoteMethodCall_(eid, s);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onRemoteMethodCall( MemoryStream& s)
{
	KMessage::Client_onRemoteMethodCall(s);
	return;//


	ENTITY_ID eid = 0;
	s >> eid;
	onRemoteMethodCall_(eid, s);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onRemoteMethodCall_(ENTITY_ID eid, MemoryStream& s)
{
	KBEngineClient::Entity* entity = pEntities_->find(eid)->second;
	if(entity == NULL)
	{	
		s.opfini();
		cocos2d::CCLog( "ClientObjectBase::onRemoteMethodCall: not found entity(%d ).\n" , eid);
		return;
	}

	entity->onRemoteMethodCall( "", s );
	
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdatePropertys( MemoryStream& s)
{
	ENTITY_ID eid = 0;
	s >> eid;
	onUpdatePropertys_(eid, s);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdatePropertysOptimized( MemoryStream& s)
{
	ENTITY_ID eid = getAoiEntityIDFromStream(s);
	onUpdatePropertys_(eid, s);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdatePropertys_(ENTITY_ID eid, MemoryStream& s)
{
	KBEngineClient::Entity* entity = pEntities_->find(eid)->second;
	if(entity == NULL)
	{	
		
		if(bufferedCreateEntityMessage_.find(eid) == bufferedCreateEntityMessage_.end())
		{
			MemoryStream* buffered = new MemoryStream();
			(*buffered) << eid;
			(*buffered).append(s.data() + s.rpos(), s.opsize());
			bufferedCreateEntityMessage_[eid] = *(buffered);
			s.opfini();
		}
		else
		{
			cocos2d::CCLog("ClientObjectBase::onUpdatePropertys: not found entity( %d ).\n", eid);
		} 

		return;
	}

	entity->onUpdatePropertys(s);
}

//-------------------------------------------------------------------------------------
KBEngineClient::Entity* ClientObjectBase::pPlayer()
{
	return pEntities_->find(entityID_)->second;
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
	
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateBasePos( MemoryStream& s)
{
	float x, y, z;
	s >> x >> y >> z;
	
	/*client::Entity* pEntity = pPlayer();
	if(pEntity)
	{
		pEntity->setServerPosition(Position3D(x, y, z));
	}*/
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateBasePosXZ( MemoryStream& s)
{
	float x, z;
	s >> x >> z;
	
	//client::Entity* pEntity = pPlayer();
	//if(pEntity)
	//{
	//	pEntity->setServerPosition(Position3D(x, pEntity->getServerPosition().y, z));
	//}
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onSetEntityPosAndDir( MemoryStream& s)
{
	ENTITY_ID eid;
	s >> eid;
	
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData( MemoryStream& s)
{
	ENTITY_ID eid = getAoiEntityIDFromStream(s);

	/*client::Entity* entity = pEntities_->find(eid);
	if(entity == NULL)
	{
		ERROR_MSG(boost::format("ClientObjectBase::onUpdateData: not found entity(%1%).\n") % eid);
		return;
	}*/
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_ypr( MemoryStream& s)
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

	_updateVolatileData(eid, 0.f, 0.f, 0.f, r, p, y);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_yp( MemoryStream& s)
{
	ENTITY_ID eid = getAoiEntityIDFromStream(s);

	float y, p;

	int8 angle;

	s >> angle;
	y = int82angle(angle);

	s >> angle;
	p = int82angle(angle);

	_updateVolatileData(eid, 0.f, 0.f, 0.f, FLT_MAX, p, y);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_yr( MemoryStream& s)
{
	ENTITY_ID eid = getAoiEntityIDFromStream(s);

	float y, r;

	int8 angle;

	s >> angle;
	y = int82angle(angle);

	s >> angle;
	r = int82angle(angle);

	_updateVolatileData(eid, 0.f, 0.f, 0.f, r, FLT_MAX, y);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_pr( MemoryStream& s)
{
	ENTITY_ID eid = getAoiEntityIDFromStream(s);

	float p, r;

	int8 angle;

	s >> angle;
	p = int82angle(angle);

	s >> angle;
	r = int82angle(angle);

	_updateVolatileData(eid, 0.f, 0.f, 0.f, r, p, FLT_MAX);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_y( MemoryStream& s)
{
	ENTITY_ID eid = getAoiEntityIDFromStream(s);

	float y;

	int8 angle;

	s >> angle;
	y = int82angle(angle);

	_updateVolatileData(eid, 0.f, 0.f, 0.f, FLT_MAX, FLT_MAX, y);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_p( MemoryStream& s)
{
	ENTITY_ID eid = getAoiEntityIDFromStream(s);

	float p;

	int8 angle;

	s >> angle;
	p = int82angle(angle);

	_updateVolatileData(eid, 0.f, 0.f, 0.f, FLT_MAX, p, FLT_MAX);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_r( MemoryStream& s)
{
	ENTITY_ID eid = getAoiEntityIDFromStream(s);

	float r;

	int8 angle;

	s >> angle;
	r = int82angle(angle);

	_updateVolatileData(eid, 0.f, 0.f, 0.f, r, FLT_MAX, FLT_MAX);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_xz( MemoryStream& s)
{
	ENTITY_ID eid = getAoiEntityIDFromStream(s);

	float x,z;

	s.readPackXZ(x, z);


	_updateVolatileData(eid, x, 0.f, z, FLT_MAX, FLT_MAX, FLT_MAX);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_xz_ypr( MemoryStream& s)
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

	_updateVolatileData(eid, x, 0.f, z, r, p, y);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_xz_yp( MemoryStream& s)
{
	ENTITY_ID eid = getAoiEntityIDFromStream(s);

	float x,z, y, p;

	s.readPackXZ(x, z);

	int8 angle;

	s >> angle;
	y = int82angle(angle);

	s >> angle;
	p = int82angle(angle);

	_updateVolatileData(eid, x, 0.f, z, FLT_MAX, p, y);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::_updateVolatileData(ENTITY_ID entityID, float x, float y, float z, 
										   float roll, float pitch, float yaw)
{
	
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_xz_yr( MemoryStream& s)
{
	ENTITY_ID eid = getAoiEntityIDFromStream(s);

	float x, z, y,  r;
	
	s.readPackXZ(x, z);

	int8 angle;

	s >> angle;
	y = int82angle(angle);

	s >> angle;
	r = int82angle(angle);

	_updateVolatileData(eid, x, 0.f, z, r, FLT_MAX, y);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_xz_pr( MemoryStream& s)
{
	ENTITY_ID eid = getAoiEntityIDFromStream(s);

	float x, z, p, r;
	
	s.readPackXZ(x, z);

	int8 angle;

	s >> angle;
	p = int82angle(angle);

	s >> angle;
	r = int82angle(angle);

	_updateVolatileData(eid, x, 0.f, z, r, p, FLT_MAX);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_xz_y(  MemoryStream& s)
{
	ENTITY_ID eid = getAoiEntityIDFromStream(s);

	float x, z, y;
	
	s.readPackXZ(x, z);

	int8 angle;

	s >> angle;
	y = int82angle(angle);

	_updateVolatileData(eid, x, 0.f, z, FLT_MAX, FLT_MAX, y);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_xz_p(  MemoryStream& s)
{
	ENTITY_ID eid = getAoiEntityIDFromStream(s);

	float x, z, p;
	
	s.readPackXZ(x, z);

	int8 angle;

	s >> angle;
	p = int82angle(angle);

	_updateVolatileData(eid, x, 0.f, z, FLT_MAX, p, FLT_MAX);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_xz_r(  MemoryStream& s)
{
	ENTITY_ID eid = getAoiEntityIDFromStream(s);

	float x, z, r;
	
	s.readPackXZ(x, z);

	int8 angle;

	s >> angle;
	r = int82angle(angle);

	_updateVolatileData(eid, x, 0.f, z, r, FLT_MAX, FLT_MAX);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_xyz( MemoryStream& s)
{
	ENTITY_ID eid = getAoiEntityIDFromStream(s);

	float x, z, y;
	
	s.readPackXZ(x, z);
	s.readPackY(y);

	_updateVolatileData(eid, x, y, z, FLT_MAX, FLT_MAX, FLT_MAX);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_xyz_ypr(  MemoryStream& s)
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

	_updateVolatileData(eid, x, y, z, r, p, yaw);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_xyz_yp(  MemoryStream& s)
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

	_updateVolatileData(eid, x, y, z, FLT_MAX, p, yaw);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_xyz_yr(  MemoryStream& s)
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

	_updateVolatileData(eid, x, y, z, r, FLT_MAX, yaw);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_xyz_pr(  MemoryStream& s)
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

	_updateVolatileData(eid, x, y, z, r, p, FLT_MAX);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_xyz_y(MemoryStream& s)
{
	ENTITY_ID eid = getAoiEntityIDFromStream(s);

	float x, z, y;
	
	s.readPackXZ(x, z);
	s.readPackY(y);

	float yaw;

	int8 angle;

	s >> angle;
	yaw = int82angle(angle);

	_updateVolatileData(eid, x, y, z, FLT_MAX, FLT_MAX, yaw);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onUpdateData_xyz_p(MemoryStream& s)
{
	ENTITY_ID eid = getAoiEntityIDFromStream(s);

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
void ClientObjectBase::onUpdateData_xyz_r(MemoryStream& s)
{
	ENTITY_ID eid = getAoiEntityIDFromStream(s);

	float x, z, y;
	
	s.readPackXZ(x, z);
	s.readPackY(y);

	float r;

	int8 angle;

	s >> angle;
	r = int82angle(angle);

	_updateVolatileData(eid, x, y, z, r, FLT_MAX, FLT_MAX);
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onStreamDataStarted( int16 id, uint32 datasize, std::string& descr)
{
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onStreamDataRecv( MemoryStream& s)
{
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::onStreamDataCompleted( int16 id)
{
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::addSpaceGeometryMapping(SPACE_ID spaceID, const std::string& respath)
{
	/*INFO_MSG(boost::format("ClientObjectBase::addSpaceGeometryMapping: spaceID=%1%, respath=%2%!\n") %
		spaceID % respath);*/

	isLoadedGeometry_ = false;
	onAddSpaceGeometryMapping(spaceID, respath);

}

//-------------------------------------------------------------------------------------
void ClientObjectBase::initSpaceData( MemoryStream& s)
{
	spacedatas_.clear();

	s >> spaceID_;
	std::string key, value;

	while(s.opsize() > 0)
	{
		s >> key >> value;
		setSpaceData( spaceID_, key, value);
	}

	//DEBUG_MSG(boost::format("ClientObjectBase::initSpaceData: spaceID(%1%), datasize=%2%.\n") % 
	//	spaceID_ % spacedatas_.size());
}

//-------------------------------------------------------------------------------------
const std::string& ClientObjectBase::getGeometryPath()
{ 
	return getSpaceData("_mapping"); 
}

//-------------------------------------------------------------------------------------
void ClientObjectBase::setSpaceData( SPACE_ID spaceID, const std::string& key, const std::string& value)
{
	if(spaceID_ != spaceID)
	{
	/*	ERROR_MSG(boost::format("ClientObjectBase::setSpaceData: spaceID(curr:%1%->%2%) not match, key=%3%, value=%4%.\n") % 
			spaceID_ % spaceID % key % value);*/
		return;
	}

	//DEBUG_MSG(boost::format("ClientObjectBase::setSpaceData: spaceID(%1%), key=%2%, value=%3%.\n") % 
	//	spaceID_ % key % value);

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
void ClientObjectBase::delSpaceData( SPACE_ID spaceID, const std::string& key)
{
	if(spaceID_ != spaceID)
	{
		//ERROR_MSG(boost::format("ClientObjectBase::delSpaceData: spaceID(curr:%1%->%2%) not match, key=%3%.\n") % 
		//	spaceID_ % spaceID % key);
		return;
	}

	//DEBUG_MSG(boost::format("ClientObjectBase::delSpaceData: spaceID(%1%), key=%2%.\n") % 
	//	spaceID_ % key);

	SPACE_DATA::iterator iter = spacedatas_.find(key);
	if(iter == spacedatas_.end())
		return;

	spacedatas_.erase(iter);
}

//

//-------------------------------------------------------------------------------------
}