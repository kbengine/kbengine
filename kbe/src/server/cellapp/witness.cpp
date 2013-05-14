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

namespace KBEngine{	


//-------------------------------------------------------------------------------------
Witness::Witness():
pEntity_(NULL),
aoiRadius_(0.0f),
aoiHysteresisArea_(5.0f),
pAOITrigger_(NULL),
aoiEntities_()
{
}

//-------------------------------------------------------------------------------------
Witness::~Witness()
{
	pEntity_ = NULL;
	SAFE_RELEASE(pAOITrigger_);
}

//-------------------------------------------------------------------------------------
void Witness::attach(Entity* pEntity)
{
	DEBUG_MSG(boost::format("Witness::attach: %1%(%2%).\n") % 
		pEntity->getScriptName() % pEntity->getID());

	pEntity_ = pEntity;

	if(g_kbeSrvConfig.getCellApp().use_coordinate_system)
	{
		// 初始化默认AOI范围
		ENGINE_COMPONENT_INFO& ecinfo = ServerConfig::getSingleton().getCellApp();
		setAoiRadius(ecinfo.defaultAoIRadius, ecinfo.defaultAoIHysteresisArea);
	}

	Cellapp::getSingleton().addUpdatable(this);
}

//-------------------------------------------------------------------------------------
void Witness::detach(Entity* pEntity)
{
	KBE_ASSERT(pEntity == pEntity_);

	DEBUG_MSG(boost::format("Witness::detach: %1%(%2%).\n") % 
		pEntity->getScriptName() % pEntity->getID());

	AOI_ENTITIES::iterator iter = aoiEntities_.begin();
	for(; iter != aoiEntities_.end(); iter++)
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
	SAFE_RELEASE(pAOITrigger_);

	aoiEntities_.clear();
	Cellapp::getSingleton().removeUpdatable(this);
}

//-------------------------------------------------------------------------------------
static ObjectPool<Witness> _g_objPool;
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
			pAOITrigger_ = new AOITrigger((RangeNode*)pEntity_->pEntityRangeNode(), aoiRadius_, aoiRadius_);
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
	AOI_ENTITIES::iterator iter = std::find_if(aoiEntities_.begin(), aoiEntities_.end(), 
		findif_vector_entityref_exist_handler(pEntity));

	if(iter != aoiEntities_.end())
	{
		if(((*iter)->flags() & ENTITYREF_FLAG_LEAVE_CLIENT_PENDING) > 0)
		{
			DEBUG_MSG(boost::format("Witness::onEnterAOI: %1% entity=%2%\n") % 
				pEntity_->getID() % pEntity->getID());

			(*iter)->removeflags(ENTITYREF_FLAG_LEAVE_CLIENT_PENDING);
			(*iter)->pEntity(pEntity);
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
	AOI_ENTITIES::iterator iter = std::find_if(aoiEntities_.begin(), aoiEntities_.end(), 
		findif_vector_entityref_exist_handler(pEntity));

	if(iter == aoiEntities_.end())
		return;

	DEBUG_MSG(boost::format("Witness::onLeaveAOI: %1% entity=%2%\n") % 
		pEntity_->getID() % pEntity->getID());

	// 这里不delete， 我们需要待update将此行为更新至客户端时再进行
	//delete (*iter);
	//aoiEntities_.erase(iter);

	(*iter)->flags((*iter)->flags() | ENTITYREF_FLAG_LEAVE_CLIENT_PENDING);
	(*iter)->pEntity(NULL);
	pEntity->delWitnessed(pEntity_);
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
	pEntity_->addPositionAndDirectionToStream(*s1);
	(*pForwardPosDirBundle).append(*s1);
	MemoryStream::ObjPool().reclaimObject(s1);
	MERCURY_ENTITY_MESSAGE_FORWARD_CLIENT(pEntity_->getID(), (*pSendBundle), (*pForwardPosDirBundle));

	(*pForwardBundle).newMessage(ClientInterface::onEntityEnterWorld);
	(*pForwardBundle) << pEntity_->getID();
	(*pForwardBundle) << pEntity_->getScriptModule()->getUType();
	(*pForwardBundle) << pSpace->getID();

	MERCURY_ENTITY_MESSAGE_FORWARD_CLIENT(pEntity_->getID(), (*pSendBundle), (*pForwardBundle));
	pEntity_->getClientMailbox()->postMail(*pSendBundle);

	Mercury::Bundle::ObjPool().reclaimObject(pSendBundle);
	Mercury::Bundle::ObjPool().reclaimObject(pForwardBundle);
	Mercury::Bundle::ObjPool().reclaimObject(pForwardPosDirBundle);

	if(pAOITrigger_)
		pAOITrigger_->install();
}

//-------------------------------------------------------------------------------------
void Witness::onLeaveSpace(Space* pSpace)
{
	if(pAOITrigger_)
		pAOITrigger_->uninstall();

	Mercury::Bundle* pSendBundle = Mercury::Bundle::ObjPool().createObject();
	Mercury::Bundle* pForwardBundle = Mercury::Bundle::ObjPool().createObject();

	(*pForwardBundle).newMessage(ClientInterface::onEntityLeaveWorld);
	(*pForwardBundle) << pEntity_->getID();
	(*pForwardBundle) << pSpace->getID();

	MERCURY_ENTITY_MESSAGE_FORWARD_CLIENT(pEntity_->getID(), (*pSendBundle), (*pForwardBundle));
	pEntity_->getClientMailbox()->postMail(*pSendBundle);
	Mercury::Bundle::ObjPool().reclaimObject(pSendBundle);
	Mercury::Bundle::ObjPool().reclaimObject(pForwardBundle);
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
void Witness::addAOIEntityIDToStream(MemoryStream* mstream, ENTITY_ID entityID)
{
	(*mstream) << entityID;
}

//-------------------------------------------------------------------------------------
const Position3D&  Witness::getBasePos()
{
	return pEntity()->getPosition();
}

//-------------------------------------------------------------------------------------
bool Witness::update()
{
	SCOPED_PROFILE(CLIENT_UPDATE_PROFILE);

	if(pEntity_ == NULL)
		return true;

	if(!pEntity_->getClientMailbox())
		return true;

	Mercury::Channel* pChannel = pEntity_->getClientMailbox()->getChannel();
	if(!pChannel)
		return true;
	
	// 获取每帧剩余可写大小， 将优先更新的内容写入， 剩余的内容往下一个周期递推
	int currPacketSize = pChannel->bundlesLength();
	int remainPacketSize = PACKET_MAX_SIZE_TCP - currPacketSize;
	if(remainPacketSize > 0)
	{
		SPACE_ID spaceID = pEntity_->getSpaceID();

		Mercury::Bundle* pSendBundle = NEW_BUNDLE();
		if(aoiEntities_.size() > 0)
		{

			MERCURY_ENTITY_MESSAGE_FORWARD_CLIENT_START(pEntity_->getID(), (*pSendBundle));
			addBasePosToStream(pSendBundle);

			AOI_ENTITIES::iterator iter = aoiEntities_.begin();
			for(; iter != aoiEntities_.end(); )
			{
				if(remainPacketSize <= 0)
					break;

				Entity* otherEntity = (*iter)->pEntity();

				if(((*iter)->flags() & ENTITYREF_FLAG_ENTER_CLIENT_PENDING) > 0)
				{
					(*iter)->removeflags(ENTITYREF_FLAG_ENTER_CLIENT_PENDING);

					Mercury::Bundle* pForwardBundle1 = Mercury::Bundle::ObjPool().createObject();
					Mercury::Bundle* pForwardBundle2 = Mercury::Bundle::ObjPool().createObject();

					MemoryStream* s1 = MemoryStream::ObjPool().createObject();
					otherEntity->addPositionAndDirectionToStream(*s1);
					otherEntity->addClientDataToStream(s1);

					(*pForwardBundle1).newMessage(ClientInterface::onUpdatePropertys);
					(*pForwardBundle1) << otherEntity->getID();
					(*pForwardBundle1).append(*s1);
					MemoryStream::ObjPool().reclaimObject(s1);
			
					(*pForwardBundle2).newMessage(ClientInterface::onEntityEnterWorld);
					(*pForwardBundle2) << otherEntity->getID();
					(*pForwardBundle2) << otherEntity->getScriptModule()->getUType();
					(*pForwardBundle2) << spaceID;

					MERCURY_ENTITY_MESSAGE_FORWARD_CLIENT_APPEND((*pSendBundle), (*pForwardBundle1));
					MERCURY_ENTITY_MESSAGE_FORWARD_CLIENT_APPEND((*pSendBundle), (*pForwardBundle2));
					
					remainPacketSize -= pForwardBundle1->packetsLength();
					remainPacketSize -= pForwardBundle2->packetsLength();

					Mercury::Bundle::ObjPool().reclaimObject(pForwardBundle1);
					Mercury::Bundle::ObjPool().reclaimObject(pForwardBundle2);
				}
				else if(((*iter)->flags() & ENTITYREF_FLAG_LEAVE_CLIENT_PENDING) > 0)
				{
					(*iter)->removeflags(ENTITYREF_FLAG_LEAVE_CLIENT_PENDING);

					Mercury::Bundle* pForwardBundle = Mercury::Bundle::ObjPool().createObject();

					(*pForwardBundle).newMessage(ClientInterface::onEntityLeaveWorld);
					(*pForwardBundle) << (*iter)->id();
					(*pForwardBundle) << spaceID;

					MERCURY_ENTITY_MESSAGE_FORWARD_CLIENT_APPEND((*pSendBundle), (*pForwardBundle));
					Mercury::Bundle::ObjPool().reclaimObject(pForwardBundle);

					delete (*iter);
					iter = aoiEntities_.erase(iter);
					continue;
				}
				else
				{
					Mercury::Bundle* pForwardBundle = Mercury::Bundle::ObjPool().createObject();
					MemoryStream* s1 = MemoryStream::ObjPool().createObject();
					
					addAOIEntityIDToStream(s1, otherEntity->getID());
					addUpdateHeadToStream(pForwardBundle, addEntityVolatileDataToStream(s1, otherEntity));

					(*pForwardBundle).append(*s1);
					MemoryStream::ObjPool().reclaimObject(s1);
					MERCURY_ENTITY_MESSAGE_FORWARD_CLIENT_APPEND((*pSendBundle), (*pForwardBundle));
					Mercury::Bundle::ObjPool().reclaimObject(pForwardBundle);
				}

				++iter;
			}
		}

		if(!pSendBundle->isEmpty())
			pChannel->bundles().push_back(pSendBundle);
		else
			DELETE_BUNDLE(pSendBundle);
	}

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
	Mercury::Bundle* pForwardBundle = Mercury::Bundle::ObjPool().createObject();
	MemoryStream* s1 = MemoryStream::ObjPool().createObject();

	(*pForwardBundle).newMessage(ClientInterface::onUpdateBasePos);
	const Position3D& bpos = getBasePos();
	s1->appendPackAnyXYZ(bpos.x, bpos.y, bpos.z);
	(*pForwardBundle).append(*s1);
	MERCURY_ENTITY_MESSAGE_FORWARD_CLIENT_APPEND((*pSendBundle), (*pForwardBundle));
	Mercury::Bundle::ObjPool().reclaimObject(pForwardBundle);
	MemoryStream::ObjPool().reclaimObject(s1);
}

//-------------------------------------------------------------------------------------
void Witness::addUpdateHeadToStream(Mercury::Bundle* pForwardBundle, uint32 flags)
{
	switch(flags)
	{
	case UPDATE_FLAG_NULL:
		{
			(*pForwardBundle).newMessage(ClientInterface::onUpdateData);
		}
		break;
	case UPDATE_FLAG_XZ:
		{
			(*pForwardBundle).newMessage(ClientInterface::onUpdateData_xz);
		}
		break;
	case UPDATE_FLAG_XYZ:
		{
			(*pForwardBundle).newMessage(ClientInterface::onUpdateData_xyz);
		}
		break;
	case UPDATE_FLAG_YAW:
		{
			(*pForwardBundle).newMessage(ClientInterface::onUpdateData_y);
		}
		break;
	case UPDATE_FLAG_ROLL:
		{
			(*pForwardBundle).newMessage(ClientInterface::onUpdateData_r);
		}
		break;
	case UPDATE_FLAG_PITCH:
		{
			(*pForwardBundle).newMessage(ClientInterface::onUpdateData_p);
		}
		break;
	case UPDATE_FLAG_YAW_PITCH_ROLL:
		{
			(*pForwardBundle).newMessage(ClientInterface::onUpdateData_ypr);
		}
		break;
	case UPDATE_FLAG_YAW_PITCH:
		{
			(*pForwardBundle).newMessage(ClientInterface::onUpdateData_yp);
		}
		break;
	case UPDATE_FLAG_YAW_ROLL:
		{
			(*pForwardBundle).newMessage(ClientInterface::onUpdateData_yr);
		}
		break;
	case UPDATE_FLAG_PITCH_ROLL:
		{
			(*pForwardBundle).newMessage(ClientInterface::onUpdateData_pr);
		}
		break;

	case (UPDATE_FLAG_XZ | UPDATE_FLAG_YAW):
		{
			(*pForwardBundle).newMessage(ClientInterface::onUpdateData_xz_y);
		}
		break;
	case (UPDATE_FLAG_XZ | UPDATE_FLAG_PITCH):
		{
			(*pForwardBundle).newMessage(ClientInterface::onUpdateData_xz_p);
		}
		break;
	case (UPDATE_FLAG_XZ | UPDATE_FLAG_ROLL):
		{
			(*pForwardBundle).newMessage(ClientInterface::onUpdateData_xz_r);
		}
		break;
	case (UPDATE_FLAG_XZ | UPDATE_FLAG_YAW_ROLL):
		{
			(*pForwardBundle).newMessage(ClientInterface::onUpdateData_xz_yr);
		}
		break;
	case (UPDATE_FLAG_XZ | UPDATE_FLAG_YAW_PITCH):
		{
			(*pForwardBundle).newMessage(ClientInterface::onUpdateData_xz_yp);
		}
		break;
	case (UPDATE_FLAG_XZ | UPDATE_FLAG_PITCH_ROLL):
		{
			(*pForwardBundle).newMessage(ClientInterface::onUpdateData_xz_pr);
		}
		break;
	case (UPDATE_FLAG_XZ | UPDATE_FLAG_YAW_PITCH_ROLL):
		{
			(*pForwardBundle).newMessage(ClientInterface::onUpdateData_xz_ypr);
		}
		break;

	case (UPDATE_FLAG_XYZ | UPDATE_FLAG_YAW):
		{
			(*pForwardBundle).newMessage(ClientInterface::onUpdateData_xyz_y);
		}
		break;
	case (UPDATE_FLAG_XYZ | UPDATE_FLAG_PITCH):
		{
			(*pForwardBundle).newMessage(ClientInterface::onUpdateData_xyz_p);
		}
		break;
	case (UPDATE_FLAG_XYZ | UPDATE_FLAG_ROLL):
		{
			(*pForwardBundle).newMessage(ClientInterface::onUpdateData_xyz_r);
		}
		break;
	case (UPDATE_FLAG_XYZ | UPDATE_FLAG_YAW_ROLL):
		{
			(*pForwardBundle).newMessage(ClientInterface::onUpdateData_xyz_yr);
		}
		break;
	case (UPDATE_FLAG_XYZ | UPDATE_FLAG_YAW_PITCH):
		{
			(*pForwardBundle).newMessage(ClientInterface::onUpdateData_xyz_yp);
		}
		break;
	case (UPDATE_FLAG_XYZ | UPDATE_FLAG_PITCH_ROLL):
		{
			(*pForwardBundle).newMessage(ClientInterface::onUpdateData_xyz_pr);
		}
		break;
	case (UPDATE_FLAG_XYZ | UPDATE_FLAG_YAW_PITCH_ROLL):
		{
			(*pForwardBundle).newMessage(ClientInterface::onUpdateData_xyz_ypr);
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

	Position3D relativePos = otherEntity->getPosition() - this->pEntity()->getPosition();
	const Direction3D& dir = otherEntity->getDirection();

	const VolatileInfo& volatileInfo = otherEntity->getScriptModule()->getVolatileInfo();

	if(volatileInfo.position() > 0.f)
	{
		mstream->appendPackXZ(relativePos.x, relativePos.z);

		if(otherEntity->isOnGround())
		{
			mstream->appendPackY(relativePos.y);
			flags |= UPDATE_FLAG_XYZ; 
		}
		else
		{
			flags |= UPDATE_FLAG_XZ; 
		}
	}

	if(volatileInfo.yaw() > 0.f && volatileInfo.roll() > 0.f && volatileInfo.pitch() > 0.f)
	{
		(*mstream) << angle2int8(dir.yaw);
		(*mstream) << angle2int8(dir.pitch);
		(*mstream) << angle2int8(dir.roll);

		flags |= UPDATE_FLAG_YAW_PITCH_ROLL; 
	}
	else if(volatileInfo.roll() > 0.f && volatileInfo.pitch() > 0.f)
	{
		(*mstream) << angle2int8(dir.pitch);
		(*mstream) << angle2int8(dir.roll);

		flags |= UPDATE_FLAG_PITCH_ROLL; 
	}
	else if(volatileInfo.yaw() > 0.f && volatileInfo.pitch() > 0.f)
	{
		(*mstream) << angle2int8(dir.yaw);
		(*mstream) << angle2int8(dir.pitch);

		flags |= UPDATE_FLAG_YAW_PITCH; 
	}
	else if(volatileInfo.yaw() > 0.f && volatileInfo.roll() > 0.f)
	{
		(*mstream) << angle2int8(dir.yaw);
		(*mstream) << angle2int8(dir.roll);

		flags |= UPDATE_FLAG_YAW_ROLL; 
	}
	else if(volatileInfo.yaw() > 0.f)
	{
		(*mstream) << angle2int8(dir.yaw);

		flags |= UPDATE_FLAG_YAW; 
	}
	else if(volatileInfo.roll() > 0.f)
	{
		(*mstream) << angle2int8(dir.roll);

		flags |= UPDATE_FLAG_ROLL; 
	}
	else if(volatileInfo.pitch() > 0.f)
	{
		(*mstream) << angle2int8(dir.pitch);

		flags |= UPDATE_FLAG_PITCH; 
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
