/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2016 KBEngine.

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

#include "witness.h"
#include "entity.h"	
#include "profile.h"
#include "cellapp.h"
#include "aoi_trigger.h"
#include "network/channel.h"	
#include "network/bundle.h"
#include "math/math.h"
#include "client_lib/client_interface.h"

#include "../../server/baseapp/baseapp_interface.h"

#ifndef CODE_INLINE
#include "witness.inl"
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
pAOIHysteresisAreaTrigger_(NULL),
aoiEntities_(),
clientAOISize_(0)
{
}

//-------------------------------------------------------------------------------------
Witness::~Witness()
{
	pEntity_ = NULL;
	SAFE_RELEASE(pAOITrigger_);
	SAFE_RELEASE(pAOIHysteresisAreaTrigger_);
}

//-------------------------------------------------------------------------------------
void Witness::addToStream(KBEngine::MemoryStream& s)
{
	/**
	 * @TODO(phw): 注释下面的原始代码，简单修正如下的问题：
	 * 想象一下：A、B、C三个玩家互相能看见对方，那么它们的aoiEntities_里面必须会互相记录着对方的entityID，
	 * 那么假如三个玩家都在同一时间传送到另一个cellapp的地图的同一点上，
	 * 这时三个玩家还原的时候都会为另两个玩家生成一个flags_ == ENTITYREF_FLAG_UNKONWN的EntityRef实例，
	 * 把它们记录在自己的aoiEntities_，
	 * 但是，Witness::update()并没有针对flags_ == ENTITYREF_FLAG_UNKONWN的情况做特殊处理——把玩家entity数据发送给客户端，
	 * 所以进入了默认的updateVolatileData()流程，
	 * 使得客户端在没有别的玩家entity的情况下就收到了别的玩家的坐标更新的信息，导致客户端错误发生。
	
	s << aoiRadius_ << aoiHysteresisArea_ << clientAOISize_;	
	
	uint32 size = aoiEntities_.size();
	s << size;

	EntityRef::AOI_ENTITIES::iterator iter = aoiEntities_.begin();
	for(; iter != aoiEntities_.end(); ++iter)
	{
		(*iter)->addToStream(s);
	}
	*/

	// 当前这么做能解决问题，但是在space多cell分割的情况下将会出现问题
	s << aoiRadius_ << aoiHysteresisArea_ << (uint16)0;	
	s << (uint32)0; // aoiEntities_.size();
}

//-------------------------------------------------------------------------------------
void Witness::createFromStream(KBEngine::MemoryStream& s)
{
	s >> aoiRadius_ >> aoiHysteresisArea_ >> clientAOISize_;

	uint32 size;
	s >> size;
	
	for(uint32 i=0; i<size; ++i)
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
				pAOITrigger_->update(aoiRadius_, aoiRadius_);
			}

			if (pAOIHysteresisAreaTrigger_ == NULL)
			{
				if (aoiHysteresisArea_ > 0.01f)
				{
					pAOIHysteresisAreaTrigger_ = new AOITrigger((CoordinateNode*)pEntity_->pEntityCoordinateNode(),
						aoiHysteresisArea_ + aoiRadius_, aoiHysteresisArea_ + aoiRadius_);
				}
			}
			else
			{
				pAOIHysteresisAreaTrigger_->update(aoiHysteresisArea_ + aoiRadius_, aoiHysteresisArea_ + aoiRadius_);
			}
		}
	}

	lastBasePos.z = -FLT_MAX;
	Cellapp::getSingleton().addUpdatable(this);
}

//-------------------------------------------------------------------------------------
void Witness::attach(Entity* pEntity)
{
	//DEBUG_MSG(fmt::format("Witness::attach: {}({}).\n", 
	//	pEntity->scriptName(), pEntity->id()));

	pEntity_ = pEntity;

	lastBasePos.z = -FLT_MAX;

	if(g_kbeSrvConfig.getCellApp().use_coordinate_system)
	{
		// 初始化默认AOI范围
		ENGINE_COMPONENT_INFO& ecinfo = ServerConfig::getSingleton().getCellApp();
		setAoiRadius(ecinfo.defaultAoIRadius, ecinfo.defaultAoIHysteresisArea);
	}

	Cellapp::getSingleton().addUpdatable(this);

	onAttach(pEntity);
}

//-------------------------------------------------------------------------------------
void Witness::onAttach(Entity* pEntity)
{
	lastBasePos.z = -FLT_MAX;

	// 通知客户端enterworld
	Network::Bundle* pSendBundle = Network::Bundle::ObjPool().createObject();
	Network::Bundle* pForwardBundle = Network::Bundle::ObjPool().createObject();
	Network::Bundle* pForwardPosDirBundle = Network::Bundle::ObjPool().createObject();
	
	(*pForwardPosDirBundle).newMessage(ClientInterface::onUpdatePropertys);
	MemoryStream* s1 = MemoryStream::ObjPool().createObject();
	(*pForwardPosDirBundle) << pEntity_->id();
	pEntity_->addPositionAndDirectionToStream(*s1, true);
	(*pForwardPosDirBundle).append(*s1);
	MemoryStream::ObjPool().reclaimObject(s1);
	NETWORK_ENTITY_MESSAGE_FORWARD_CLIENT(pEntity_->id(), (*pSendBundle), (*pForwardPosDirBundle));
	
	(*pForwardBundle).newMessage(ClientInterface::onEntityEnterWorld);

	(*pForwardBundle) << pEntity_->id();
	pEntity_->pScriptModule()->addSmartUTypeToBundle(pForwardBundle);
	if(!pEntity_->isOnGround())
		(*pForwardBundle) << pEntity_->isOnGround();

	NETWORK_ENTITY_MESSAGE_FORWARD_CLIENT(pEntity_->id(), (*pSendBundle), (*pForwardBundle));
	pEntity_->clientMailbox()->postMail(pSendBundle);

	Network::Bundle::ObjPool().reclaimObject(pForwardBundle);
	Network::Bundle::ObjPool().reclaimObject(pForwardPosDirBundle);
}

//-------------------------------------------------------------------------------------
void Witness::detach(Entity* pEntity)
{
	//DEBUG_MSG(fmt::format("Witness::detach: {}({}).\n", 
	//	pEntity->scriptName(), pEntity->id()));

	EntityMailbox* pClientMB = pEntity_->clientMailbox();
	if(pClientMB)
	{
		Network::Channel* pChannel = pClientMB->getChannel();
		if(pChannel)
		{
			pChannel->send();

			// 通知客户端leaveworld
			Network::Bundle* pSendBundle = Network::Bundle::ObjPool().createObject();
			Network::Bundle* pForwardBundle = Network::Bundle::ObjPool().createObject();

			(*pForwardBundle).newMessage(ClientInterface::onEntityLeaveWorld);
			(*pForwardBundle) << pEntity->id();

			NETWORK_ENTITY_MESSAGE_FORWARD_CLIENT(pEntity_->id(), (*pSendBundle), (*pForwardBundle));
			pClientMB->postMail(pSendBundle);
			Network::Bundle::ObjPool().reclaimObject(pForwardBundle);
		}
	}

	clear(pEntity);
}

//-------------------------------------------------------------------------------------
void Witness::clear(Entity* pEntity)
{
	KBE_ASSERT(pEntity == pEntity_);

	EntityRef::AOI_ENTITIES::iterator iter = aoiEntities_.begin();
	for(; iter != aoiEntities_.end(); ++iter)
	{
		if((*iter)->pEntity())
		{
			(*iter)->pEntity()->delWitnessed(pEntity_);
		}

		delete (*iter);
	}
	
	pEntity_ = NULL;
	aoiRadius_ = 0.0f;
	aoiHysteresisArea_ = 5.0f;
	clientAOISize_ = 0;

	SAFE_RELEASE(pAOITrigger_);
	SAFE_RELEASE(pAOIHysteresisAreaTrigger_);
	
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
const Position3D& Witness::basePos()
{
	return pEntity()->position();
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

	if (aoiRadius_ > 0.f)
	{
		if (pAOITrigger_ == NULL)
		{
			pAOITrigger_ = new AOITrigger((CoordinateNode*)pEntity_->pEntityCoordinateNode(), aoiRadius_, aoiRadius_);
		}
		else
		{
			pAOITrigger_->update(aoiRadius_, aoiRadius_);
		}

		if (aoiHysteresisArea_ > 0.01f)
		{
			if (pAOIHysteresisAreaTrigger_ == NULL)
			{
				pAOIHysteresisAreaTrigger_ = new AOITrigger((CoordinateNode*)pEntity_->pEntityCoordinateNode(),
					aoiHysteresisArea_ + aoiRadius_, aoiHysteresisArea_ + aoiRadius_);
			}
			else
			{
				pAOIHysteresisAreaTrigger_->update(aoiHysteresisArea_ + aoiRadius_, aoiHysteresisArea_ + aoiRadius_);
			}
		}
	}
}

//-------------------------------------------------------------------------------------
void Witness::onEnterAOI(AOITrigger* pAOITrigger, Entity* pEntity)
{
	// 如果进入的是Hysteresis区域，那么不产生作用
	if (pAOIHysteresisAreaTrigger_ == pAOITrigger)
		return;

	pEntity_->onEnteredAoI(pEntity);

	EntityRef::AOI_ENTITIES::iterator iter = std::find_if(aoiEntities_.begin(), aoiEntities_.end(), 
		findif_vector_entityref_exist_by_entity_handler(pEntity));

	if(iter != aoiEntities_.end())
	{
		if(((*iter)->flags() & ENTITYREF_FLAG_LEAVE_CLIENT_PENDING) > 0)
		{
			//DEBUG_MSG(fmt::format("Witness::onEnterAOI: {} entity={}\n", 
			//	pEntity_->id(), pEntity->id()));

			// 如果flags是ENTITYREF_FLAG_LEAVE_CLIENT_PENDING | ENTITYREF_FLAG_NORMAL状态那么我们
			// 只需要撤销离开状态并将其还原到ENTITYREF_FLAG_NORMAL即可
			// 如果是ENTITYREF_FLAG_LEAVE_CLIENT_PENDING状态那么此时应该将它设置为进入状态 ENTITYREF_FLAG_ENTER_CLIENT_PENDING
			if(((*iter)->flags() & ENTITYREF_FLAG_NORMAL) > 0)
				(*iter)->flags(ENTITYREF_FLAG_NORMAL);
			else
				(*iter)->flags(ENTITYREF_FLAG_ENTER_CLIENT_PENDING);

			(*iter)->pEntity(pEntity);
			pEntity->addWitnessed(pEntity_);
		}

		return;
	}

	//DEBUG_MSG(fmt::format("Witness::onEnterAOI: {} entity={}\n", 
	//	pEntity_->id(), pEntity->id()));
	
	EntityRef* pEntityRef = new EntityRef(pEntity);
	pEntityRef->flags(pEntityRef->flags() | ENTITYREF_FLAG_ENTER_CLIENT_PENDING);
	aoiEntities_.push_back(pEntityRef);

	pEntity->addWitnessed(pEntity_);
}

//-------------------------------------------------------------------------------------
void Witness::onLeaveAOI(AOITrigger* pAOITrigger, Entity* pEntity)
{
	// 如果设置过Hysteresis区域，那么离开Hysteresis区域才算离开AOI
	if (pAOIHysteresisAreaTrigger_ && pAOIHysteresisAreaTrigger_ != pAOITrigger)
		return;

	EntityRef::AOI_ENTITIES::iterator iter = std::find_if(aoiEntities_.begin(), aoiEntities_.end(), 
		findif_vector_entityref_exist_by_entityid_handler(pEntity->id()));

	if(iter == aoiEntities_.end())
		return;

	_onLeaveAOI((*iter));
}

//-------------------------------------------------------------------------------------
void Witness::_onLeaveAOI(EntityRef* pEntityRef)
{
	//DEBUG_MSG(fmt::format("Witness::onLeaveAOI: {} entity={}\n", 
	//	pEntity_->id(), pEntityRef->id()));

	// 这里不delete， 我们需要待update将此行为更新至客户端时再进行
	//delete (*iter);
	//aoiEntities_.erase(iter);
	
	pEntityRef->flags(((pEntityRef->flags() | ENTITYREF_FLAG_LEAVE_CLIENT_PENDING) & ~(ENTITYREF_FLAG_ENTER_CLIENT_PENDING)));

	if(pEntityRef->pEntity())
		pEntityRef->pEntity()->delWitnessed(pEntity_);

	pEntityRef->pEntity(NULL);
}

//-------------------------------------------------------------------------------------
void Witness::resetAOIEntities()
{
	clientAOISize_ = 0;
	EntityRef::AOI_ENTITIES::iterator iter = aoiEntities_.begin();
	for(; iter != aoiEntities_.end(); )
	{
		if(((*iter)->flags() & ENTITYREF_FLAG_LEAVE_CLIENT_PENDING) > 0)
		{
			delete (*iter);
			iter = aoiEntities_.erase(iter);
			continue;
		}

		(*iter)->flags(ENTITYREF_FLAG_ENTER_CLIENT_PENDING);
		++iter;
	}
}

//-------------------------------------------------------------------------------------
void Witness::onEnterSpace(Space* pSpace)
{
	Network::Bundle* pSendBundle = Network::Bundle::ObjPool().createObject();
	
	// 通知位置强制改变
	Network::Bundle* pForwardPosDirBundle = Network::Bundle::ObjPool().createObject();
	Position3D &pos = pEntity_->position();
	Direction3D &dir = pEntity_->direction();
	(*pForwardPosDirBundle).newMessage(ClientInterface::onSetEntityPosAndDir);
	(*pForwardPosDirBundle) << pEntity_->id();
	(*pForwardPosDirBundle) << pos.x << pos.y << pos.z;
	(*pForwardPosDirBundle) << dir.roll() << dir.pitch() << dir.yaw();
	NETWORK_ENTITY_MESSAGE_FORWARD_CLIENT(pEntity_->id(), (*pSendBundle), (*pForwardPosDirBundle));
	
	// 通知进入了新地图
	Network::Bundle* pForwardBundle = Network::Bundle::ObjPool().createObject();
	(*pForwardBundle).newMessage(ClientInterface::onEntityEnterSpace);

	(*pForwardBundle) << pEntity_->id();
	(*pForwardBundle) << pSpace->id();
	if(!pEntity_->isOnGround())
		(*pForwardBundle) << pEntity_->isOnGround();

	NETWORK_ENTITY_MESSAGE_FORWARD_CLIENT(pEntity_->id(), (*pSendBundle), (*pForwardBundle));

	// 发送消息并清理
	pEntity_->clientMailbox()->postMail(pSendBundle);

	Network::Bundle::ObjPool().reclaimObject(pForwardBundle);
	Network::Bundle::ObjPool().reclaimObject(pForwardPosDirBundle);

	installAOITrigger();
}

//-------------------------------------------------------------------------------------
void Witness::onLeaveSpace(Space* pSpace)
{
	uninstallAOITrigger();

	Network::Bundle* pSendBundle = Network::Bundle::ObjPool().createObject();
	Network::Bundle* pForwardBundle = Network::Bundle::ObjPool().createObject();

	(*pForwardBundle).newMessage(ClientInterface::onEntityLeaveSpace);
	(*pForwardBundle) << pEntity_->id();

	NETWORK_ENTITY_MESSAGE_FORWARD_CLIENT(pEntity_->id(), (*pSendBundle), (*pForwardBundle));
	pEntity_->clientMailbox()->postMail(pSendBundle);
	Network::Bundle::ObjPool().reclaimObject(pForwardBundle);

	lastBasePos.z = -FLT_MAX;

	EntityRef::AOI_ENTITIES::iterator iter = aoiEntities_.begin();
	for(; iter != aoiEntities_.end(); ++iter)
	{
		if((*iter)->pEntity())
		{
			(*iter)->pEntity()->delWitnessed(pEntity_);
		}

		delete (*iter);
	}

	aoiEntities_.clear();
	clientAOISize_ = 0;
}

//-------------------------------------------------------------------------------------
void Witness::installAOITrigger()
{
	if (pAOITrigger_)
	{
		pAOITrigger_->reinstall((CoordinateNode*)pEntity_->pEntityCoordinateNode());

		if (pAOIHysteresisAreaTrigger_)
		{
			pAOIHysteresisAreaTrigger_->reinstall((CoordinateNode*)pEntity_->pEntityCoordinateNode());
		}
	}
	else
	{
		KBE_ASSERT(pAOIHysteresisAreaTrigger_ == NULL);
	}
}

//-------------------------------------------------------------------------------------
void Witness::uninstallAOITrigger()
{
	if (pAOITrigger_)
		pAOITrigger_->uninstall();

	if (pAOIHysteresisAreaTrigger_)
		pAOIHysteresisAreaTrigger_->uninstall();
}

//-------------------------------------------------------------------------------------
bool Witness::pushBundle(Network::Bundle* pBundle)
{
	if(pEntity_ == NULL)
		return false;

	EntityMailbox* clientMB = pEntity_->clientMailbox();
	if(!clientMB)
		return false;

	Network::Channel* pChannel = clientMB->getChannel();
	if(!pChannel)
		return false;

	pChannel->send(pBundle);
	return true;
}

//-------------------------------------------------------------------------------------
void Witness::_addAOIEntityIDToBundle(Network::Bundle* pBundle, ENTITY_ID entityID)
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
void Witness::_addAOIEntityIDToBundle(Network::Bundle* pBundle, EntityRef* entityRef)
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
void Witness::addSmartAOIEntityMessageToBundle(Network::Bundle* pBundle, const Network::MessageHandler& normalMsgHandler, 
											   const Network::MessageHandler& optimizedMsgHandler, ENTITY_ID entityID)
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
bool Witness::entityID2AliasID(ENTITY_ID id, uint8& aliasID) const
{
	aliasID = 0;
	EntityRef::AOI_ENTITIES::const_iterator iter = aoiEntities_.begin();
	for(; iter != aoiEntities_.end(); ++iter)
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

	if(pEntity_ == NULL || !pEntity_->clientMailbox())
		return true;

	Network::Channel* pChannel = pEntity_->clientMailbox()->getChannel();
	if(!pChannel)
		return true;
	
	// 获取每帧剩余可写大小， 将优先更新的内容写入， 剩余的内容往下一个周期递推
	int remainPacketSize = PACKET_MAX_SIZE_TCP - pChannel->bundlesLength();

	if(remainPacketSize > 0)
	{
		if(aoiEntities_.size() > 0)
		{
			Network::Bundle* pSendBundle = MALLOC_BUNDLE();

			NETWORK_ENTITY_MESSAGE_FORWARD_CLIENT_START(pEntity_->id(), (*pSendBundle));
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

					Network::Bundle* pForwardBundle1 = Network::Bundle::ObjPool().createObject();
					Network::Bundle* pForwardBundle2 = Network::Bundle::ObjPool().createObject();

					MemoryStream* s1 = MemoryStream::ObjPool().createObject();
					otherEntity->addPositionAndDirectionToStream(*s1, true);			
					otherEntity->addClientDataToStream(s1, true);

					(*pForwardBundle1).newMessage(ClientInterface::onUpdatePropertys);
					(*pForwardBundle1) << otherEntity->id();
					(*pForwardBundle1).append(*s1);
					MemoryStream::ObjPool().reclaimObject(s1);
			
					(*pForwardBundle2).newMessage(ClientInterface::onEntityEnterWorld);
					(*pForwardBundle2) << otherEntity->id();
					otherEntity->pScriptModule()->addSmartUTypeToBundle(pForwardBundle2);
					if(!otherEntity->isOnGround())
						(*pForwardBundle2) << otherEntity->isOnGround();

					NETWORK_ENTITY_MESSAGE_FORWARD_CLIENT_APPEND((*pSendBundle), (*pForwardBundle1));
					NETWORK_ENTITY_MESSAGE_FORWARD_CLIENT_APPEND((*pSendBundle), (*pForwardBundle2));
					
					remainPacketSize -= pForwardBundle1->packetsLength();
					remainPacketSize -= pForwardBundle2->packetsLength();

					Network::Bundle::ObjPool().reclaimObject(pForwardBundle1);
					Network::Bundle::ObjPool().reclaimObject(pForwardBundle2);

					(*iter)->flags(ENTITYREF_FLAG_NORMAL);
					
					KBE_ASSERT(clientAOISize_ != 65535);

					++clientAOISize_;
				}
				else if(((*iter)->flags() & ENTITYREF_FLAG_LEAVE_CLIENT_PENDING) > 0)
				{
					(*iter)->removeflags(ENTITYREF_FLAG_LEAVE_CLIENT_PENDING);

					if(((*iter)->flags() & ENTITYREF_FLAG_NORMAL) > 0)
					{
						Network::Bundle* pForwardBundle = Network::Bundle::ObjPool().createObject();

						(*pForwardBundle).newMessage(ClientInterface::onEntityLeaveWorldOptimized);
						_addAOIEntityIDToBundle(pForwardBundle, (*iter)->id());

						NETWORK_ENTITY_MESSAGE_FORWARD_CLIENT_APPEND((*pSendBundle), (*pForwardBundle));
						Network::Bundle::ObjPool().reclaimObject(pForwardBundle);

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
						--clientAOISize_;
						continue;
					}
					
					KBE_ASSERT((*iter)->flags() == ENTITYREF_FLAG_NORMAL);

					Network::Bundle* pForwardBundle = Network::Bundle::ObjPool().createObject();
					MemoryStream* s1 = MemoryStream::ObjPool().createObject();
					
					addUpdateHeadToStream(pForwardBundle, addEntityVolatileDataToStream(s1, otherEntity), (*iter));

					(*pForwardBundle).append(*s1);
					MemoryStream::ObjPool().reclaimObject(s1);
					
					if(pForwardBundle->packetsLength() > 0)
					{
						NETWORK_ENTITY_MESSAGE_FORWARD_CLIENT_APPEND((*pSendBundle), (*pForwardBundle));
					}

					Network::Bundle::ObjPool().reclaimObject(pForwardBundle);
				}

				++iter;
			}
			
			int32 packetsLength = pSendBundle->packetsLength();
			if(packetsLength > 8/*NETWORK_ENTITY_MESSAGE_FORWARD_CLIENT_START产生的基础包大小*/)
			{
				if(packetsLength > PACKET_MAX_SIZE_TCP)
				{
					WARNING_MSG(fmt::format("Witness::update({}): sendToClient {} Bytes.\n", 
						pEntity_->id(), packetsLength));
				}

				pChannel->send(pSendBundle);
			}
			else
			{
				Network::Bundle::ObjPool().reclaimObject(pSendBundle);
			}
		}
	}

	return true;
}

//-------------------------------------------------------------------------------------
void Witness::addBasePosToStream(Network::Bundle* pSendBundle)
{
	const Position3D& bpos = basePos();
	Vector3 movement = bpos - lastBasePos;

	if(KBEVec3Length(&movement) < 0.0004f)
		return;

	Network::Bundle* pForwardBundle = Network::Bundle::ObjPool().createObject();
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
	NETWORK_ENTITY_MESSAGE_FORWARD_CLIENT_APPEND((*pSendBundle), (*pForwardBundle));
	Network::Bundle::ObjPool().reclaimObject(pForwardBundle);
	MemoryStream::ObjPool().reclaimObject(s1);

	lastBasePos = bpos;
}

//-------------------------------------------------------------------------------------
void Witness::addUpdateHeadToStream(Network::Bundle* pForwardBundle, uint32 flags, EntityRef* pEntityRef)
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

	const VolatileInfo* pVolatileInfo = otherEntity->pCustomVolatileinfo();
	if (!pVolatileInfo)
		pVolatileInfo = otherEntity->pScriptModule()->getPVolatileInfo();

	static uint16 entity_posdir_additional_updates = g_kbeSrvConfig.getCellApp().entity_posdir_additional_updates;
	
	if ((pVolatileInfo->position() > 0.f) && (entity_posdir_additional_updates == 0 || g_kbetime - otherEntity->posChangedTime() < entity_posdir_additional_updates))
	{
		Position3D relativePos = otherEntity->position() - this->pEntity()->position();
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
		const Direction3D& dir = otherEntity->direction();
		if (pVolatileInfo->yaw() > 0.f && pVolatileInfo->roll() > 0.f && pVolatileInfo->pitch() > 0.f)
		{
			(*mstream) << angle2int8(dir.yaw());
			(*mstream) << angle2int8(dir.pitch());
			(*mstream) << angle2int8(dir.roll());

			flags |= UPDATE_FLAG_YAW_PITCH_ROLL; 
		}
		else if (pVolatileInfo->roll() > 0.f && pVolatileInfo->pitch() > 0.f)
		{
			(*mstream) << angle2int8(dir.pitch());
			(*mstream) << angle2int8(dir.roll());

			flags |= UPDATE_FLAG_PITCH_ROLL; 
		}
		else if (pVolatileInfo->yaw() > 0.f && pVolatileInfo->pitch() > 0.f)
		{
			(*mstream) << angle2int8(dir.yaw());
			(*mstream) << angle2int8(dir.pitch());

			flags |= UPDATE_FLAG_YAW_PITCH; 
		}
		else if (pVolatileInfo->yaw() > 0.f && pVolatileInfo->roll() > 0.f)
		{
			(*mstream) << angle2int8(dir.yaw());
			(*mstream) << angle2int8(dir.roll());

			flags |= UPDATE_FLAG_YAW_ROLL; 
		}
		else if (pVolatileInfo->yaw() > 0.f)
		{
			(*mstream) << angle2int8(dir.yaw());

			flags |= UPDATE_FLAG_YAW; 
		}
		else if (pVolatileInfo->roll() > 0.f)
		{
			(*mstream) << angle2int8(dir.roll());

			flags |= UPDATE_FLAG_ROLL; 
		}
		else if (pVolatileInfo->pitch() > 0.f)
		{
			(*mstream) << angle2int8(dir.pitch());

			flags |= UPDATE_FLAG_PITCH; 
		}
	}

	return flags;
}

//-------------------------------------------------------------------------------------
bool Witness::sendToClient(const Network::MessageHandler& msgHandler, Network::Bundle* pBundle)
{
	if(pushBundle(pBundle))
		return true;

	ERROR_MSG(fmt::format("Witness::sendToClient: {} pBundles is NULL, not found channel.\n", pEntity_->id()));
	Network::Bundle::ObjPool().reclaimObject(pBundle);
	return false;
}

//-------------------------------------------------------------------------------------
}
