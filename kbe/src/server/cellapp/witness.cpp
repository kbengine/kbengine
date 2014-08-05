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

#include "witness.hpp"
#include "entity.hpp"	
#include "profile.hpp"
#include "cellapp.hpp"
#include "aoi_trigger.hpp"
#include "network/channel.hpp"	
#include "network/bundle.hpp"
#include "math/math.hpp"
#include "client_lib/client_interface.hpp"

#include "../../server/baseapp/baseapp_interface.hpp"

#ifndef CODE_INLINE
#include "witness.ipp"
#endif

#define UPDATE_FLAG_NULL				0x00000000
#define UPDATE_FLAG_XZ					0x00000001
#define UPDATE_FLAG_XYZ					0x00000002
#define UPDATE_FLAG_YAW					0x00000004
#define UPDATE_FLAG_ROLL				0x00000008
#define UPDATE_FLAG_PITCH				0x00000010
#define UPDATE_FLAG_YAW_PITCH_ROLL		0x00000020
#define UPDATE_FLAG_YAW_PITCH			0x00000040
#define UPDATE_FLAG_YAW_ROLL			0x00000080
#define UPDATE_FLAG_PITCH_ROLL			0x00000100
#define UPDATE_FLAG_ONGOUND				0x00000200

namespace KBEngine{	


//-------------------------------------------------------------------------------------
Witness::Witness():
pEntity_(NULL),
aoiRadius_(0.0f),
aoiHysteresisArea_(5.0f),
pAOITrigger_(NULL),
aoiEntities_(),
clientAOISize_(0)
{
}

//-------------------------------------------------------------------------------------
Witness::~Witness()
{
	pEntity_ = NULL;
	SAFE_RELEASE(pAOITrigger_);
}

//-------------------------------------------------------------------------------------
void Witness::addToStream(KBEngine::MemoryStream& s)
{
	s << aoiRadius_ << aoiHysteresisArea_ << clientAOISize_;

	uint32 size = aoiEntities_.size();
	s << size;

	EntityRef::AOI_ENTITIES::iterator iter = aoiEntities_.begin();
	for(; iter != aoiEntities_.end(); iter++)
	{
		(*iter)->addToStream(s);
	}
}

//-------------------------------------------------------------------------------------
void Witness::createFromStream(KBEngine::MemoryStream& s)
{
	s >> aoiRadius_ >> aoiHysteresisArea_ >> clientAOISize_;

	uint32 size;
	s >> size;
	
	for(uint32 i=0; i<size; i++)
	{
		EntityRef* pEntityRef = new EntityRef();
		pEntityRef->createFromStream(s);
		aoiEntities_.push_back(pEntityRef);
	}

	if(g_kbeSrvConfig.getCellApp().use_coordinate_system)
	{
		if(aoiRadius_ > 0.f)
		{
			if(pAOITrigger_ == NULL)
			{
				pAOITrigger_ = new AOITrigger((CoordinateNode*)pEntity_->pEntityCoordinateNode(), aoiRadius_, aoiRadius_);
			}
			else
			{
				pAOITrigger_->range(aoiRadius_, aoiRadius_);
			}
		}
	}

	lastBasePos.z = -FLT_MAX;
}

//-------------------------------------------------------------------------------------
void Witness::attach(Entity* pEntity)
{
	DEBUG_MSG(boost::format("Witness::attach: %1%(%2%).\n") % 
		pEntity->getScriptName() % pEntity->getID());

	pEntity_ = pEntity;

	lastBasePos.z = -FLT_MAX;

	if(g_kbeSrvConfig.getCellApp().use_coordinate_system)
	{
		// 初始化默认AOI范围
		ENGINE_COMPONENT_INFO& ecinfo = ServerConfig::getSingleton().getCellApp();
		setAoiRadius(ecinfo.defaultAoIRadius, ecinfo.defaultAoIHysteresisArea);
	}

	Cellapp::getSingleton().addUpdatable(this);

	// 通知客户端enterworld
	Mercury::Bundle* pSendBundle = Mercury::Bundle::ObjPool().createObject();
	Mercury::Bundle* pForwardBundle = Mercury::Bundle::ObjPool().createObject();
	Mercury::Bundle* pForwardPosDirBundle = Mercury::Bundle::ObjPool().createObject();
	
	(*pForwardPosDirBundle).newMessage(ClientInterface::onUpdatePropertys);
	MemoryStream* s1 = MemoryStream::ObjPool().createObject();
	(*pForwardPosDirBundle) << pEntity_->getID();
	pEntity_->addPositionAndDirectionToStream(*s1, true);
	(*pForwardPosDirBundle).append(*s1);
	MemoryStream::ObjPool().reclaimObject(s1);
	MERCURY_ENTITY_MESSAGE_FORWARD_CLIENT(pEntity_->getID(), (*pSendBundle), (*pForwardPosDirBundle));
	
	(*pForwardBundle).newMessage(ClientInterface::onEntityEnterWorld);

	(*pForwardBundle) << pEntity_->getID();
	pEntity_->getScriptModule()->addSmartUTypeToBundle(pForwardBundle);
	if(!pEntity_->isOnGround())
		(*pForwardBundle) << pEntity_->isOnGround();

	MERCURY_ENTITY_MESSAGE_FORWARD_CLIENT(pEntity_->getID(), (*pSendBundle), (*pForwardBundle));
	pEntity_->getClientMailbox()->postMail(*pSendBundle);

	Mercury::Bundle::ObjPool().reclaimObject(pSendBundle);
	Mercury::Bundle::ObjPool().reclaimObject(pForwardBundle);
	Mercury::Bundle::ObjPool().reclaimObject(pForwardPosDirBundle);
}

//-------------------------------------------------------------------------------------
void Witness::detach(Entity* pEntity)
{
	KBE_ASSERT(pEntity == pEntity_);

	DEBUG_MSG(boost::format("Witness::detach: %1%(%2%).\n") % 
		pEntity->getScriptName() % pEntity->getID());

	EntityRef::AOI_ENTITIES::iterator iter = aoiEntities_.begin();
	for(; iter != aoiEntities_.end(); iter++)
	{
		if((*iter)->pEntity())
		{
			(*iter)->pEntity()->delWitnessed(pEntity_);
		}

		delete (*iter);
	}
	
	Mercury::Channel* pChannel = pEntity_->getClientMailbox()->getChannel();
	if(pChannel)
	{
		pChannel->send();

		// 通知客户端leaveworld
		Mercury::Bundle* pSendBundle = Mercury::Bundle::ObjPool().createObject();
		Mercury::Bundle* pForwardBundle = Mercury::Bundle::ObjPool().createObject();

		(*pForwardBundle).newMessage(ClientInterface::onEntityLeaveWorld);
		(*pForwardBundle) << pEntity_->getID();

		MERCURY_ENTITY_MESSAGE_FORWARD_CLIENT(pEntity_->getID(), (*pSendBundle), (*pForwardBundle));
		pEntity_->getClientMailbox()->postMail(*pSendBundle);
		Mercury::Bundle::ObjPool().reclaimObject(pSendBundle);
		Mercury::Bundle::ObjPool().reclaimObject(pForwardBundle);
	}

	pEntity_ = NULL;
	aoiRadius_ = 0.0f;
	aoiHysteresisArea_ = 5.0f;
	clientAOISize_ = 0;
	SAFE_RELEASE(pAOITrigger_);

	aoiEntities_.clear();

	Cellapp::getSingleton().removeUpdatable(this);
}

//-------------------------------------------------------------------------------------
static ObjectPool<Witness> _g_objPool("Witness");
ObjectPool<Witness>& Witness::ObjPool()
{
	return _g_objPool;
}

//-------------------------------------------------------------------------------------
Witness::SmartPoolObjectPtr Witness::createSmartPoolObj()
{
	return SmartPoolObjectPtr(new SmartPoolObject<Witness>(ObjPool().createObject(), _g_objPool));
}

//-------------------------------------------------------------------------------------
void Witness::onReclaimObject()
{
}

//-------------------------------------------------------------------------------------
const Position3D&  Witness::getBasePos()
{
	return pEntity()->getPosition();
}

//-------------------------------------------------------------------------------------
void Witness::setAoiRadius(float radius, float hyst)
{
	if(!g_kbeSrvConfig.getCellApp().use_coordinate_system)
		return;

	aoiRadius_ = radius;
	aoiHysteresisArea_ = hyst;

	if(aoiRadius_ + aoiHysteresisArea_ > g_kbeSrvConfig.getCellApp().ghostDistance)
	{
		aoiRadius_ = g_kbeSrvConfig.getCellApp().ghostDistance - 5.0f;
		aoiHysteresisArea_ = 5.0f;
	}

	if(aoiRadius_ > 0.f)
	{
		if(pAOITrigger_ == NULL)
		{
			pAOITrigger_ = new AOITrigger((CoordinateNode*)pEntity_->pEntityCoordinateNode(), aoiRadius_, aoiRadius_);
		}
		else
		{
			pAOITrigger_->range(aoiRadius_, aoiRadius_);
		}
	}
}

//-------------------------------------------------------------------------------------
void Witness::onEnterAOI(Entity* pEntity)
{
	EntityRef::AOI_ENTITIES::iterator iter = std::find_if(aoiEntities_.begin(), aoiEntities_.end(), 
		findif_vector_entityref_exist_by_entity_handler(pEntity));

	if(iter != aoiEntities_.end())
	{
		if(((*iter)->flags() & ENTITYREF_FLAG_LEAVE_CLIENT_PENDING) > 0)
		{
			DEBUG_MSG(boost::format("Witness::onEnterAOI: %1% entity=%2%\n") % 
				pEntity_->getID() % pEntity->getID());

			(*iter)->removeflags(ENTITYREF_FLAG_LEAVE_CLIENT_PENDING);
			(*iter)->pEntity(pEntity);
			pEntity->addWitnessed(pEntity_);
		}

		return;
	}

	DEBUG_MSG(boost::format("Witness::onEnterAOI: %1% entity=%2%\n") % 
		pEntity_->getID() % pEntity->getID());
	
	EntityRef* pEntityRef = new EntityRef(pEntity);
	pEntityRef->flags(pEntityRef->flags() | ENTITYREF_FLAG_ENTER_CLIENT_PENDING);
	aoiEntities_.push_back(pEntityRef);

	pEntity->addWitnessed(pEntity_);
}

//-------------------------------------------------------------------------------------
void Witness::onLeaveAOI(Entity* pEntity)
{
	EntityRef::AOI_ENTITIES::iterator iter = std::find_if(aoiEntities_.begin(), aoiEntities_.end(), 
		findif_vector_entityref_exist_by_entityid_handler(pEntity->getID()));

	if(iter == aoiEntities_.end())
		return;

	_onLeaveAOI((*iter));
}

//-------------------------------------------------------------------------------------
void Witness::_onLeaveAOI(EntityRef* pEntityRef)
{
	DEBUG_MSG(boost::format("Witness::onLeaveAOI: %1% entity=%2%\n") % 
		pEntity_->getID() % pEntityRef->id());

	// 这里不delete， 我们需要待update将此行为更新至客户端时再进行
	//delete (*iter);
	//aoiEntities_.erase(iter);
	
	pEntityRef->flags(((pEntityRef->flags() | ENTITYREF_FLAG_LEAVE_CLIENT_PENDING) & ~(ENTITYREF_FLAG_ENTER_CLIENT_PENDING)));

	if(pEntityRef->pEntity())
		pEntityRef->pEntity()->delWitnessed(pEntity_);

	pEntityRef->pEntity(NULL);
}

//-------------------------------------------------------------------------------------
void Witness::onEnterSpace(Space* pSpace)
{
	Mercury::Bundle* pSendBundle = Mercury::Bundle::ObjPool().createObject();
	Mercury::Bundle* pForwardBundle = Mercury::Bundle::ObjPool().createObject();
	Mercury::Bundle* pForwardPosDirBundle = Mercury::Bundle::ObjPool().createObject();
	
	(*pForwardPosDirBundle).newMessage(ClientInterface::onUpdatePropertys);
	MemoryStream* s1 = MemoryStream::ObjPool().createObject();
	(*pForwardPosDirBundle) << pEntity_->getID();
	pEntity_->addPositionAndDirectionToStream(*s1, true);
	(*pForwardPosDirBundle).append(*s1);
	MemoryStream::ObjPool().reclaimObject(s1);
	MERCURY_ENTITY_MESSAGE_FORWARD_CLIENT(pEntity_->getID(), (*pSendBundle), (*pForwardPosDirBundle));
	
	(*pForwardBundle).newMessage(ClientInterface::onEntityEnterSpace);

	(*pForwardBundle) << pEntity_->getID();
	if(!pEntity_->isOnGround())
		(*pForwardBundle) << pEntity_->isOnGround();

	MERCURY_ENTITY_MESSAGE_FORWARD_CLIENT(pEntity_->getID(), (*pSendBundle), (*pForwardBundle));
	pEntity_->getClientMailbox()->postMail(*pSendBundle);

	Mercury::Bundle::ObjPool().reclaimObject(pSendBundle);
	Mercury::Bundle::ObjPool().reclaimObject(pForwardBundle);
	Mercury::Bundle::ObjPool().reclaimObject(pForwardPosDirBundle);

	if(pAOITrigger_)
	{
		pAOITrigger_->reinstall((CoordinateNode*)pEntity_->pEntityCoordinateNode());
	}
}

//-------------------------------------------------------------------------------------
void Witness::onLeaveSpace(Space* pSpace)
{
	if(pAOITrigger_)
		pAOITrigger_->uninstall();

	Mercury::Bundle* pSendBundle = Mercury::Bundle::ObjPool().createObject();
	Mercury::Bundle* pForwardBundle = Mercury::Bundle::ObjPool().createObject();


	(*pForwardBundle).newMessage(ClientInterface::onEntityLeaveSpace);

	(*pForwardBundle) << pEntity_->getID();

	MERCURY_ENTITY_MESSAGE_FORWARD_CLIENT(pEntity_->getID(), (*pSendBundle), (*pForwardBundle));
	pEntity_->getClientMailbox()->postMail(*pSendBundle);
	Mercury::Bundle::ObjPool().reclaimObject(pSendBundle);
	Mercury::Bundle::ObjPool().reclaimObject(pForwardBundle);

	lastBasePos.z = -FLT_MAX;

	EntityRef::AOI_ENTITIES::iterator iter = aoiEntities_.begin();
	for(; iter != aoiEntities_.end(); iter++)
	{
		if((*iter)->pEntity())
		{
			(*iter)->pEntity()->delWitnessed(pEntity_);
		}

		delete (*iter);
	}

	aoiEntities_.clear();
}

//-------------------------------------------------------------------------------------
Witness::Bundles* Witness::pBundles()
{
	if(pEntity_ == NULL)
		return NULL;

	if(!pEntity_->getClientMailbox())
		return NULL;

	Mercury::Channel* pChannel = pEntity_->getClientMailbox()->getChannel();
	if(!pChannel)
		return NULL;

	return &pChannel->bundles();
}

//-------------------------------------------------------------------------------------
void Witness::_addAOIEntityIDToBundle(Mercury::Bundle* pBundle, ENTITY_ID entityID)
{
	if(!EntityDef::entityAliasID())
	{
		(*pBundle) << entityID;
	}
	else
	{
		// 注意：不可在该模块外部使用，否则可能出现客户端表找不到entityID的情况
		if(clientAOISize_ > 255)
		{
			(*pBundle) << entityID;
		}
		else
		{
			uint8 aliasID = 0;
			if(entityID2AliasID(entityID, aliasID))
			{
				(*pBundle) << aliasID;
			}
			else
			{
				(*pBundle) << entityID;
			}
		}
	}
}

//-------------------------------------------------------------------------------------
void Witness::_addAOIEntityIDToStream(MemoryStream* mstream, EntityRef* entityRef)
{
	if(!EntityDef::entityAliasID())
	{
		(*mstream) << entityRef->id();
	}
	else
	{
		// 注意：不可在该模块外部使用，否则可能出现客户端表找不到entityID的情况
		if(clientAOISize_ > 255)
		{
			(*mstream) << entityRef->id();
		}
		else
		{
			uint8 aliasID = 0;
			if(entityID2AliasID(entityRef->id(), aliasID))
			{
				(*mstream) << aliasID;
			}
			else
			{
				(*mstream) << entityRef->id();
			}
		}
	}
}

//-------------------------------------------------------------------------------------
void Witness::_addAOIEntityIDToBundle(Mercury::Bundle* pBundle, EntityRef* entityRef)
{
	if(!EntityDef::entityAliasID())
	{
		(*pBundle) << entityRef->id();
	}
	else
	{
		// 注意：不可在该模块外部使用，否则可能出现客户端表找不到entityID的情况
		if(clientAOISize_ > 255)
		{
			(*pBundle) << entityRef->id();
		}
		else
		{
			uint8 aliasID = 0;
			if(entityID2AliasID(entityRef->id(), aliasID))
				(*pBundle) << aliasID;
			else
			{
				(*pBundle) << entityRef->id();
			}
		}
	}
}

//-------------------------------------------------------------------------------------
void Witness::addSmartAOIEntityMessageToBundle(Mercury::Bundle* pBundle, const Mercury::MessageHandler& normalMsgHandler, 
											   const Mercury::MessageHandler& optimizedMsgHandler, ENTITY_ID entityID)
{
	if(!EntityDef::entityAliasID())
	{
		(*pBundle).newMessage(normalMsgHandler);
		(*pBundle) << entityID;
	}
	else
	{
		if(aoiEntities_.size() > 255)
		{
			(*pBundle).newMessage(normalMsgHandler);
			(*pBundle) << entityID;
		}
		else
		{
			uint8 aliasID = 0;
			if(entityID2AliasID(entityID, aliasID))
			{
				(*pBundle).newMessage(optimizedMsgHandler);
				(*pBundle) << aliasID;
			}
			else
			{
				(*pBundle).newMessage(normalMsgHandler);
				(*pBundle) << entityID;
			}
		}
	}
}

//-------------------------------------------------------------------------------------
bool Witness::entityID2AliasID(ENTITY_ID id, uint8& aliasID)const
{
	aliasID = 0;
	EntityRef::AOI_ENTITIES::const_iterator iter = aoiEntities_.begin();
	for(; iter != aoiEntities_.end(); iter++)
	{
		EntityRef* pEntityRef = (*iter);
		if(pEntityRef->id() == id)
		{
			if((pEntityRef->flags() & (ENTITYREF_FLAG_NORMAL)) <= 0)
				return false;

			break;
		}
		
		// 将要溢出
		if(aliasID == 255)
			return false;
		
		++aliasID;
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool Witness::update()
{
	SCOPED_PROFILE(CLIENT_UPDATE_PROFILE);

	if(pEntity_ == NULL || !pEntity_->getClientMailbox())
		return true;

	Mercury::Channel* pChannel = pEntity_->getClientMailbox()->getChannel();
	if(!pChannel)
		return true;
	
	// 获取每帧剩余可写大小， 将优先更新的内容写入， 剩余的内容往下一个周期递推
	int remainPacketSize = PACKET_MAX_SIZE_TCP - pChannel->bundlesLength();

	if(remainPacketSize > 0)
	{
		if(aoiEntities_.size() > 0)
		{
			Mercury::Bundle* pSendBundle = NEW_BUNDLE();

			MERCURY_ENTITY_MESSAGE_FORWARD_CLIENT_START(pEntity_->getID(), (*pSendBundle));
			addBasePosToStream(pSendBundle);

			EntityRef::AOI_ENTITIES::iterator iter = aoiEntities_.begin();
			for(; iter != aoiEntities_.end(); )
			{
				if(remainPacketSize <= 0)
					break;
				
				if(((*iter)->flags() & ENTITYREF_FLAG_ENTER_CLIENT_PENDING) > 0)
				{
					// 这里使用id查找一下， 避免entity在进入AOI时的回调里被意外销毁
					Entity* otherEntity = Cellapp::getSingleton().findEntity((*iter)->id());
					if(otherEntity == NULL)
					{
						(*iter)->pEntity(NULL);
						_onLeaveAOI((*iter));
						delete (*iter);
						iter = aoiEntities_.erase(iter);
						continue;
					}
					
					(*iter)->removeflags(ENTITYREF_FLAG_ENTER_CLIENT_PENDING);

					Mercury::Bundle* pForwardBundle1 = Mercury::Bundle::ObjPool().createObject();
					Mercury::Bundle* pForwardBundle2 = Mercury::Bundle::ObjPool().createObject();

					MemoryStream* s1 = MemoryStream::ObjPool().createObject();
					otherEntity->addPositionAndDirectionToStream(*s1, true);			
					otherEntity->addClientDataToStream(s1, true);

					(*pForwardBundle1).newMessage(ClientInterface::onUpdatePropertys);
					(*pForwardBundle1) << otherEntity->getID();
					(*pForwardBundle1).append(*s1);
					MemoryStream::ObjPool().reclaimObject(s1);
			
					(*pForwardBundle2).newMessage(ClientInterface::onEntityEnterWorld);
					(*pForwardBundle2) << otherEntity->getID();
					otherEntity->getScriptModule()->addSmartUTypeToBundle(pForwardBundle2);
					if(!otherEntity->isOnGround())
						(*pForwardBundle2) << otherEntity->isOnGround();

					MERCURY_ENTITY_MESSAGE_FORWARD_CLIENT_APPEND((*pSendBundle), (*pForwardBundle1));
					MERCURY_ENTITY_MESSAGE_FORWARD_CLIENT_APPEND((*pSendBundle), (*pForwardBundle2));
					
					remainPacketSize -= pForwardBundle1->packetsLength();
					remainPacketSize -= pForwardBundle2->packetsLength();

					Mercury::Bundle::ObjPool().reclaimObject(pForwardBundle1);
					Mercury::Bundle::ObjPool().reclaimObject(pForwardBundle2);

					(*iter)->flags(ENTITYREF_FLAG_NORMAL);
					
					KBE_ASSERT(clientAOISize_ != 65535);

					++clientAOISize_;
				}
				else if(((*iter)->flags() & ENTITYREF_FLAG_LEAVE_CLIENT_PENDING) > 0)
				{
					(*iter)->removeflags(ENTITYREF_FLAG_LEAVE_CLIENT_PENDING);

					if(((*iter)->flags() & ENTITYREF_FLAG_NORMAL) > 0)
					{
						Mercury::Bundle* pForwardBundle = Mercury::Bundle::ObjPool().createObject();

						(*pForwardBundle).newMessage(ClientInterface::onEntityLeaveWorldOptimized);
						_addAOIEntityIDToBundle(pForwardBundle, (*iter)->id());

						MERCURY_ENTITY_MESSAGE_FORWARD_CLIENT_APPEND((*pSendBundle), (*pForwardBundle));
						Mercury::Bundle::ObjPool().reclaimObject(pForwardBundle);

						--clientAOISize_;
					}

					delete (*iter);
					iter = aoiEntities_.erase(iter);
					continue;
				}
				else
				{
					Entity* otherEntity = (*iter)->pEntity();
					if(otherEntity == NULL)
					{
						delete (*iter);
						iter = aoiEntities_.erase(iter);
						continue;
					}
					
					KBE_ASSERT((*iter)->flags() == ENTITYREF_FLAG_NORMAL);

					Mercury::Bundle* pForwardBundle = Mercury::Bundle::ObjPool().createObject();
					MemoryStream* s1 = MemoryStream::ObjPool().createObject();
					
					addUpdateHeadToStream(pForwardBundle, addEntityVolatileDataToStream(s1, otherEntity), (*iter));

					(*pForwardBundle).append(*s1);
					MemoryStream::ObjPool().reclaimObject(s1);

					if(pForwardBundle->packetsLength() > 0)
						MERCURY_ENTITY_MESSAGE_FORWARD_CLIENT_APPEND((*pSendBundle), (*pForwardBundle));

					Mercury::Bundle::ObjPool().reclaimObject(pForwardBundle);
				}

				++iter;
			}
			
			int32 packetsLength = pSendBundle->packetsLength();
			if(packetsLength > 0)
			{
				if(packetsLength > PACKET_MAX_SIZE_TCP)
				{
					WARNING_MSG(boost::format("Witness::update(%1%): sendToClient %2% Bytes.\n") % 
						pEntity_->getID() % packetsLength);
				}

				pChannel->bundles().push_back(pSendBundle);
			}
		}
	}

	if(pChannel->bundles().size() > 0)
	{
		// 如果数据大量阻塞发不出去将会报警
		AUTO_SCOPED_PROFILE("updateClientSend");
		pChannel->send();
	}

	return true;
}

//-------------------------------------------------------------------------------------
void Witness::addBasePosToStream(Mercury::Bundle* pSendBundle)
{
	const VolatileInfo& volatileInfo = pEntity_->getScriptModule()->getVolatileInfo();
	if((volatileInfo.position() <= 0.0004f))
		return;

	const Position3D& bpos = getBasePos();
	Vector3 movement = bpos - lastBasePos;

	if(KBEVec3Length(&movement) < 0.0004f)
		return;

	Mercury::Bundle* pForwardBundle = Mercury::Bundle::ObjPool().createObject();
	MemoryStream* s1 = MemoryStream::ObjPool().createObject();

	if(fabs(lastBasePos.y - bpos.y) > 0.0004f)
	{
		(*pForwardBundle).newMessage(ClientInterface::onUpdateBasePos);
		s1->appendPackAnyXYZ(bpos.x, bpos.y, bpos.z, 0.f);
	}
	else
	{
		(*pForwardBundle).newMessage(ClientInterface::onUpdateBasePosXZ);
		s1->appendPackAnyXZ(bpos.x, bpos.z, 0.f);
	}

	(*pForwardBundle).append(*s1);
	MERCURY_ENTITY_MESSAGE_FORWARD_CLIENT_APPEND((*pSendBundle), (*pForwardBundle));
	Mercury::Bundle::ObjPool().reclaimObject(pForwardBundle);
	MemoryStream::ObjPool().reclaimObject(s1);

	lastBasePos = bpos;
}

//-------------------------------------------------------------------------------------
void Witness::addUpdateHeadToStream(Mercury::Bundle* pForwardBundle, uint32 flags, EntityRef* pEntityRef)
{
	switch(flags)
	{
	case UPDATE_FLAG_NULL:
		{
			// (*pForwardBundle).newMessage(ClientInterface::onUpdateData);
		}
		break;
	case UPDATE_FLAG_XZ:
		{
			(*pForwardBundle).newMessage(ClientInterface::onUpdateData_xz);
			_addAOIEntityIDToBundle(pForwardBundle, pEntityRef);
		}
		break;
	case UPDATE_FLAG_XYZ:
		{
			(*pForwardBundle).newMessage(ClientInterface::onUpdateData_xyz);
			_addAOIEntityIDToBundle(pForwardBundle, pEntityRef);
		}
		break;
	case UPDATE_FLAG_YAW:
		{
			(*pForwardBundle).newMessage(ClientInterface::onUpdateData_y);
			_addAOIEntityIDToBundle(pForwardBundle, pEntityRef);
		}
		break;
	case UPDATE_FLAG_ROLL:
		{
			(*pForwardBundle).newMessage(ClientInterface::onUpdateData_r);
			_addAOIEntityIDToBundle(pForwardBundle, pEntityRef);
		}
		break;
	case UPDATE_FLAG_PITCH:
		{
			(*pForwardBundle).newMessage(ClientInterface::onUpdateData_p);
			_addAOIEntityIDToBundle(pForwardBundle, pEntityRef);
		}
		break;
	case UPDATE_FLAG_YAW_PITCH_ROLL:
		{
			(*pForwardBundle).newMessage(ClientInterface::onUpdateData_ypr);
			_addAOIEntityIDToBundle(pForwardBundle, pEntityRef);
		}
		break;
	case UPDATE_FLAG_YAW_PITCH:
		{
			(*pForwardBundle).newMessage(ClientInterface::onUpdateData_yp);
			_addAOIEntityIDToBundle(pForwardBundle, pEntityRef);
		}
		break;
	case UPDATE_FLAG_YAW_ROLL:
		{
			(*pForwardBundle).newMessage(ClientInterface::onUpdateData_yr);
			_addAOIEntityIDToBundle(pForwardBundle, pEntityRef);
		}
		break;
	case UPDATE_FLAG_PITCH_ROLL:
		{
			(*pForwardBundle).newMessage(ClientInterface::onUpdateData_pr);
			_addAOIEntityIDToBundle(pForwardBundle, pEntityRef);
		}
		break;
	case (UPDATE_FLAG_XZ | UPDATE_FLAG_YAW):
		{
			(*pForwardBundle).newMessage(ClientInterface::onUpdateData_xz_y);
			_addAOIEntityIDToBundle(pForwardBundle, pEntityRef);
		}
		break;
	case (UPDATE_FLAG_XZ | UPDATE_FLAG_PITCH):
		{
			(*pForwardBundle).newMessage(ClientInterface::onUpdateData_xz_p);
			_addAOIEntityIDToBundle(pForwardBundle, pEntityRef);
		}
		break;
	case (UPDATE_FLAG_XZ | UPDATE_FLAG_ROLL):
		{
			(*pForwardBundle).newMessage(ClientInterface::onUpdateData_xz_r);
			_addAOIEntityIDToBundle(pForwardBundle, pEntityRef);
		}
		break;
	case (UPDATE_FLAG_XZ | UPDATE_FLAG_YAW_ROLL):
		{
			(*pForwardBundle).newMessage(ClientInterface::onUpdateData_xz_yr);
			_addAOIEntityIDToBundle(pForwardBundle, pEntityRef);
		}
		break;
	case (UPDATE_FLAG_XZ | UPDATE_FLAG_YAW_PITCH):
		{
			(*pForwardBundle).newMessage(ClientInterface::onUpdateData_xz_yp);
			_addAOIEntityIDToBundle(pForwardBundle, pEntityRef);
		}
		break;
	case (UPDATE_FLAG_XZ | UPDATE_FLAG_PITCH_ROLL):
		{
			(*pForwardBundle).newMessage(ClientInterface::onUpdateData_xz_pr);
			_addAOIEntityIDToBundle(pForwardBundle, pEntityRef);
		}
		break;
	case (UPDATE_FLAG_XZ | UPDATE_FLAG_YAW_PITCH_ROLL):
		{
			(*pForwardBundle).newMessage(ClientInterface::onUpdateData_xz_ypr);
			_addAOIEntityIDToBundle(pForwardBundle, pEntityRef);
		}
		break;
	case (UPDATE_FLAG_XYZ | UPDATE_FLAG_YAW):
		{
			(*pForwardBundle).newMessage(ClientInterface::onUpdateData_xyz_y);
			_addAOIEntityIDToBundle(pForwardBundle, pEntityRef);
		}
		break;
	case (UPDATE_FLAG_XYZ | UPDATE_FLAG_PITCH):
		{
			(*pForwardBundle).newMessage(ClientInterface::onUpdateData_xyz_p);
			_addAOIEntityIDToBundle(pForwardBundle, pEntityRef);
		}
		break;
	case (UPDATE_FLAG_XYZ | UPDATE_FLAG_ROLL):
		{
			(*pForwardBundle).newMessage(ClientInterface::onUpdateData_xyz_r);
			_addAOIEntityIDToBundle(pForwardBundle, pEntityRef);
		}
		break;
	case (UPDATE_FLAG_XYZ | UPDATE_FLAG_YAW_ROLL):
		{
			(*pForwardBundle).newMessage(ClientInterface::onUpdateData_xyz_yr);
			_addAOIEntityIDToBundle(pForwardBundle, pEntityRef);
		}
		break;
	case (UPDATE_FLAG_XYZ | UPDATE_FLAG_YAW_PITCH):
		{
			(*pForwardBundle).newMessage(ClientInterface::onUpdateData_xyz_yp);
			_addAOIEntityIDToBundle(pForwardBundle, pEntityRef);
		}
		break;
	case (UPDATE_FLAG_XYZ | UPDATE_FLAG_PITCH_ROLL):
		{
			(*pForwardBundle).newMessage(ClientInterface::onUpdateData_xyz_pr);
			_addAOIEntityIDToBundle(pForwardBundle, pEntityRef);
		}
		break;
	case (UPDATE_FLAG_XYZ | UPDATE_FLAG_YAW_PITCH_ROLL):
		{
			(*pForwardBundle).newMessage(ClientInterface::onUpdateData_xyz_ypr);
			_addAOIEntityIDToBundle(pForwardBundle, pEntityRef);
		}
		break;
	default:
		KBE_ASSERT(false);
		break;
	};
}

//-------------------------------------------------------------------------------------
uint32 Witness::addEntityVolatileDataToStream(MemoryStream* mstream, Entity* otherEntity)
{
	uint32 flags = UPDATE_FLAG_NULL;

	const VolatileInfo& volatileInfo = otherEntity->getScriptModule()->getVolatileInfo();
	
	static uint16 entity_posdir_additional_updates = g_kbeSrvConfig.getCellApp().entity_posdir_additional_updates;
	
	if((volatileInfo.position() > 0.f) && (entity_posdir_additional_updates == 0 || g_kbetime - otherEntity->posChangedTime() < entity_posdir_additional_updates))
	{
		Position3D relativePos = otherEntity->getPosition() - this->pEntity()->getPosition();
		mstream->appendPackXZ(relativePos.x, relativePos.z);

		if(!otherEntity->isOnGround())
		{
			mstream->appendPackY(relativePos.y);
			flags |= UPDATE_FLAG_XYZ; 
		}
		else
		{
			flags |= UPDATE_FLAG_XZ; 
		}
	}

	if((entity_posdir_additional_updates == 0) || (g_kbetime - otherEntity->dirChangedTime() < entity_posdir_additional_updates))
	{
		const Direction3D& dir = otherEntity->getDirection();
		if(volatileInfo.yaw() > 0.f && volatileInfo.roll() > 0.f && volatileInfo.pitch() > 0.f)
		{
			(*mstream) << angle2int8(dir.yaw());
			(*mstream) << angle2int8(dir.pitch());
			(*mstream) << angle2int8(dir.roll());

			flags |= UPDATE_FLAG_YAW_PITCH_ROLL; 
		}
		else if(volatileInfo.roll() > 0.f && volatileInfo.pitch() > 0.f)
		{
			(*mstream) << angle2int8(dir.pitch());
			(*mstream) << angle2int8(dir.roll());

			flags |= UPDATE_FLAG_PITCH_ROLL; 
		}
		else if(volatileInfo.yaw() > 0.f && volatileInfo.pitch() > 0.f)
		{
			(*mstream) << angle2int8(dir.yaw());
			(*mstream) << angle2int8(dir.pitch());

			flags |= UPDATE_FLAG_YAW_PITCH; 
		}
		else if(volatileInfo.yaw() > 0.f && volatileInfo.roll() > 0.f)
		{
			(*mstream) << angle2int8(dir.yaw());
			(*mstream) << angle2int8(dir.roll());

			flags |= UPDATE_FLAG_YAW_ROLL; 
		}
		else if(volatileInfo.yaw() > 0.f)
		{
			(*mstream) << angle2int8(dir.yaw());

			flags |= UPDATE_FLAG_YAW; 
		}
		else if(volatileInfo.roll() > 0.f)
		{
			(*mstream) << angle2int8(dir.roll());

			flags |= UPDATE_FLAG_ROLL; 
		}
		else if(volatileInfo.pitch() > 0.f)
		{
			(*mstream) << angle2int8(dir.pitch());

			flags |= UPDATE_FLAG_PITCH; 
		}
	}

	return flags;
}

//-------------------------------------------------------------------------------------
bool Witness::sendToClient(const Mercury::MessageHandler& msgHandler, Mercury::Bundle* pBundle)
{
	Bundles* lpBundles = pBundles();

	if(lpBundles)
	{
		lpBundles->push_back(pBundle);
		return true;
	}

	ERROR_MSG(boost::format("Witness::sendToClient: %1% pBundles is NULL, not found channel.\n") % pEntity_->getID());
	Mercury::Bundle::ObjPool().reclaimObject(pBundle);
	return false;
}

//-------------------------------------------------------------------------------------
}
