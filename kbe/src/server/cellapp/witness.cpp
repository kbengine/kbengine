/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2017 KBEngine.

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
#include "network/network_stats.h"
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
aoiEntities_map_(),
clientAOISize_(0)
{
	updatableName = "Witness";
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
	 * @TODO(phw): ע�������ԭʼ���룬���������µ����⣺
	 * ����һ�£�A��B��C������һ����ܿ����Է�����ô���ǵ�aoiEntities_�������ụ���¼�ŶԷ���entityID��
	 * ��ô����������Ҷ���ͬһʱ�䴫�͵���һ��cellapp�ĵ�ͼ��ͬһ���ϣ�
	 * ��ʱ������һ�ԭ��ʱ�򶼻�Ϊ�������������һ��flags_ == ENTITYREF_FLAG_UNKONWN��EntityRefʵ����
	 * �����Ǽ�¼���Լ���aoiEntities_��
	 * ���ǣ�Witness::update()��û�����flags_ == ENTITYREF_FLAG_UNKONWN����������⴦���������entity���ݷ��͸��ͻ��ˣ�
	 * ���Խ�����Ĭ�ϵ�updateVolatileData()���̣�
	 * ʹ�ÿͻ�����û�б�����entity������¾��յ��˱����ҵ�������µ���Ϣ�����¿ͻ��˴�������
	
	s << aoiRadius_ << aoiHysteresisArea_ << clientAOISize_;	
	
	uint32 size = aoiEntitiesmap_.size();
	s << size;

	EntityRef::AOI_ENTITIES::iterator iter = aoiEntities_.begin();
	for(; iter != aoiEntities_.end(); ++iter)
	{
		(*iter)->addToStream(s);
	}
	*/

	// ��ǰ��ô���ܽ�����⣬������space��cell�ָ������½����������
	s << aoiRadius_ << aoiHysteresisArea_ << (uint16)0;	
	s << (uint32)0; // aoiEntities_map_.size();
}

//-------------------------------------------------------------------------------------
void Witness::createFromStream(KBEngine::MemoryStream& s)
{
	s >> aoiRadius_ >> aoiHysteresisArea_ >> clientAOISize_;

	uint32 size;
	s >> size;
	
	for(uint32 i=0; i<size; ++i)
	{
		EntityRef* pEntityRef = EntityRef::createPoolObject();
		pEntityRef->createFromStream(s);
		aoiEntities_.push_back(pEntityRef);
		aoiEntities_map_[pEntityRef->id()] = pEntityRef;
		pEntityRef->aliasID(i);
	}

	setAoiRadius(aoiRadius_, aoiHysteresisArea_);

	lastBasePos_.z = -FLT_MAX;
	lastBaseDir_.yaw(-FLT_MAX);
	Cellapp::getSingleton().addUpdatable(this);
}

//-------------------------------------------------------------------------------------
void Witness::attach(Entity* pEntity)
{
	//DEBUG_MSG(fmt::format("Witness::attach: {}({}).\n", 
	//	pEntity->scriptName(), pEntity->id()));

	pEntity_ = pEntity;

	lastBasePos_.z = -FLT_MAX;
	lastBaseDir_.yaw(-FLT_MAX);

	if(g_kbeSrvConfig.getCellApp().use_coordinate_system)
	{
		// ��ʼ��Ĭ��AOI��Χ
		ENGINE_COMPONENT_INFO& ecinfo = ServerConfig::getSingleton().getCellApp();
		setAoiRadius(ecinfo.defaultAoIRadius, ecinfo.defaultAoIHysteresisArea);
	}

	Cellapp::getSingleton().addUpdatable(this);

	onAttach(pEntity);
}

//-------------------------------------------------------------------------------------
void Witness::onAttach(Entity* pEntity)
{
	lastBasePos_.z = -FLT_MAX;
	lastBaseDir_.yaw(-FLT_MAX);

	// ֪ͨ�ͻ���enterworld
	Network::Bundle* pSendBundle = Network::Bundle::createPoolObject();
	NETWORK_ENTITY_MESSAGE_FORWARD_CLIENT_START(pEntity_->id(), (*pSendBundle));
	
	ENTITY_MESSAGE_FORWARD_CLIENT_START(pSendBundle, ClientInterface::onUpdatePropertys, updatePropertys);
	MemoryStream* s1 = MemoryStream::createPoolObject();
	(*pSendBundle) << pEntity_->id();
	pEntity_->addPositionAndDirectionToStream(*s1, true);
	(*pSendBundle).append(*s1);
	MemoryStream::reclaimPoolObject(s1);
	ENTITY_MESSAGE_FORWARD_CLIENT_END(pSendBundle, ClientInterface::onUpdatePropertys, updatePropertys);
	
	ENTITY_MESSAGE_FORWARD_CLIENT_START(pSendBundle, ClientInterface::onEntityEnterWorld, entityEnterWorld);

	(*pSendBundle) << pEntity_->id();
	pEntity_->pScriptModule()->addSmartUTypeToBundle(pSendBundle);
	if(!pEntity_->isOnGround())
		(*pSendBundle) << pEntity_->isOnGround();

	ENTITY_MESSAGE_FORWARD_CLIENT_END(pSendBundle, ClientInterface::onEntityEnterWorld, entityEnterWorld);
	pEntity_->clientMailbox()->postMail(pSendBundle);
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

			// ֪ͨ�ͻ���leaveworld
			Network::Bundle* pSendBundle = Network::Bundle::createPoolObject();
			NETWORK_ENTITY_MESSAGE_FORWARD_CLIENT_START(pEntity_->id(), (*pSendBundle));

			ENTITY_MESSAGE_FORWARD_CLIENT_START(pSendBundle, ClientInterface::onEntityLeaveWorld, entityLeaveWorld);
			(*pSendBundle) << pEntity->id();
			ENTITY_MESSAGE_FORWARD_CLIENT_END(pSendBundle, ClientInterface::onEntityLeaveWorld, entityLeaveWorld);
			pClientMB->postMail(pSendBundle);
		}
	}

	clear(pEntity);
}

//-------------------------------------------------------------------------------------
void Witness::clear(Entity* pEntity)
{
	KBE_ASSERT(pEntity == pEntity_);
	uninstallAOITrigger();

	AOI_ENTITIES::iterator iter = aoiEntities_.begin();
	for(; iter != aoiEntities_.end(); ++iter)
	{
		if((*iter)->pEntity())
		{
			(*iter)->pEntity()->delWitnessed(pEntity_);
		}
		
		EntityRef::reclaimPoolObject((*iter));
	}
	
	pEntity_ = NULL;
	aoiRadius_ = 0.0f;
	aoiHysteresisArea_ = 5.0f;
	clientAOISize_ = 0;

	// ����Ҫ���٣����滹��������
	// �˴����ٿ��ܻ����������Ϊenteraoi�����п��ܵ���ʵ������
	// ��pAOITrigger_����û����֮ǰ����������pAOITrigger_��crash
	//SAFE_RELEASE(pAOITrigger_);
	//SAFE_RELEASE(pAOIHysteresisAreaTrigger_);

	aoiEntities_.clear();
	aoiEntities_map_.clear();

	Cellapp::getSingleton().removeUpdatable(this);
}

//-------------------------------------------------------------------------------------
static ObjectPool<Witness> _g_objPool("Witness");
ObjectPool<Witness>& Witness::ObjPool()
{
	return _g_objPool;
}

//-------------------------------------------------------------------------------------
Witness* Witness::createPoolObject()
{
	return _g_objPool.createObject();
}

//-------------------------------------------------------------------------------------
void Witness::reclaimPoolObject(Witness* obj)
{
	_g_objPool.reclaimObject(obj);
}

//-------------------------------------------------------------------------------------
void Witness::destroyObjPool()
{
	DEBUG_MSG(fmt::format("Witness::destroyObjPool(): size {}.\n",
		_g_objPool.size()));

	_g_objPool.destroy();
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
const Direction3D& Witness::baseDir()
{
	return pEntity()->direction();
}

//-------------------------------------------------------------------------------------
void Witness::setAoiRadius(float radius, float hyst)
{
	if(!g_kbeSrvConfig.getCellApp().use_coordinate_system)
		return;

	aoiRadius_ = radius;
	aoiHysteresisArea_ = hyst;

	// ����λ��ͬ��ʹ�������λ��ѹ�����䣬���÷�ΧΪ-512~512֮�䣬��˳�����Χ������ͬ������
	// ������һ�����ƣ������Ҫ�������ֵ�ͻ���Ӧ�õ������굥λ����������Ŵ�ʹ�á�
	// �ο�: MemoryStream::appendPackXZ
	if(aoiRadius_ + aoiHysteresisArea_ > 512)
	{
		aoiRadius_ = 512 - 5.0f;
		aoiHysteresisArea_ = 5.0f;
		
		ERROR_MSG(fmt::format("Witness::setAoiRadius({}): AOI the size({}) of more than 512!\n", 
			pEntity_->id(), (aoiRadius_ + aoiHysteresisArea_)));
		
		return;
	}

	if (aoiRadius_ > 0.f && pEntity_)
	{
		if (pAOITrigger_ == NULL)
		{
			pAOITrigger_ = new AOITrigger((CoordinateNode*)pEntity_->pEntityCoordinateNode(), aoiRadius_, aoiRadius_);

			// ���ʵ���Ѿ��ڳ����У���ô��Ҫ��װ
			if (((CoordinateNode*)pEntity_->pEntityCoordinateNode())->pCoordinateSystem())
				pAOITrigger_->install();
		}
		else
		{
			pAOITrigger_->update(aoiRadius_, aoiRadius_);

			// ���ʵ���Ѿ��ڳ����У���ô��Ҫ��װ
			if (!pAOITrigger_->isInstalled() && ((CoordinateNode*)pEntity_->pEntityCoordinateNode())->pCoordinateSystem())
				pAOITrigger_->reinstall((CoordinateNode*)pEntity_->pEntityCoordinateNode());
		}

		if (aoiHysteresisArea_ > 0.01f && pEntity_/*����update���̿��ܵ������� */)
		{
			if (pAOIHysteresisAreaTrigger_ == NULL)
			{
				pAOIHysteresisAreaTrigger_ = new AOITrigger((CoordinateNode*)pEntity_->pEntityCoordinateNode(),
					aoiHysteresisArea_ + aoiRadius_, aoiHysteresisArea_ + aoiRadius_);

				if (((CoordinateNode*)pEntity_->pEntityCoordinateNode())->pCoordinateSystem())
					pAOIHysteresisAreaTrigger_->install();
			}
			else
			{
				pAOIHysteresisAreaTrigger_->update(aoiHysteresisArea_ + aoiRadius_, aoiHysteresisArea_ + aoiRadius_);

				// ���ʵ���Ѿ��ڳ����У���ô��Ҫ��װ
				if (!pAOIHysteresisAreaTrigger_->isInstalled() && ((CoordinateNode*)pEntity_->pEntityCoordinateNode())->pCoordinateSystem())
					pAOIHysteresisAreaTrigger_->reinstall((CoordinateNode*)pEntity_->pEntityCoordinateNode());
			}
		}
		else
		{
			// ע�⣺�˴����������pAOIHysteresisAreaTrigger_�������update
			// ��Ϊ�뿪AOI���ж����pAOIHysteresisAreaTrigger_���ڣ���ô�������pAOIHysteresisAreaTrigger_�����AOI
			if (pAOIHysteresisAreaTrigger_)
				pAOIHysteresisAreaTrigger_->update(aoiHysteresisArea_ + aoiRadius_, aoiHysteresisArea_ + aoiRadius_);
		}
	}
	else
	{
		uninstallAOITrigger();
	}
}

//-------------------------------------------------------------------------------------
void Witness::onEnterAOI(AOITrigger* pAOITrigger, Entity* pEntity)
{
	// ����������Hysteresis������ô����������
	 if (pAOIHysteresisAreaTrigger_ == pAOITrigger)
		return;

	// ������һ�����ã�����ʵ���ڻص��б�������ɺ����жϳ���
	Py_INCREF(pEntity);

	// ��onEnteredAoI��addWitnessed���ܵ����Լ�����Ȼ��
	// pEntity_��������ΪNULL������û�л���DECREF
	Entity* pSelfEntity = pEntity_;
	Py_INCREF(pSelfEntity);

	AOI_ENTITIES_MAP::iterator iter = aoiEntities_map_.find(pEntity->id());
	if (iter != aoiEntities_map_.end())
	{
		EntityRef* pEntityRef = iter->second;
		if ((pEntityRef->flags() & ENTITYREF_FLAG_LEAVE_CLIENT_PENDING) > 0)
		{
			//DEBUG_MSG(fmt::format("Witness::onEnterAOI: {} entity={}\n", 
			//	pEntity_->id(), pEntity->id()));

			// ���flags��ENTITYREF_FLAG_LEAVE_CLIENT_PENDING | ENTITYREF_FLAG_NORMAL״̬��ô����
			// ֻ��Ҫ�����뿪״̬�����仹ԭ��ENTITYREF_FLAG_NORMAL����
			// �����ENTITYREF_FLAG_LEAVE_CLIENT_PENDING״̬��ô��ʱӦ�ý�������Ϊ����״̬ ENTITYREF_FLAG_ENTER_CLIENT_PENDING
			if ((pEntityRef->flags() & ENTITYREF_FLAG_NORMAL) > 0)
				pEntityRef->flags(ENTITYREF_FLAG_NORMAL);
			else
				pEntityRef->flags(ENTITYREF_FLAG_ENTER_CLIENT_PENDING);

			pEntityRef->pEntity(pEntity);
			pEntity->addWitnessed(pEntity_);
			pSelfEntity->onEnteredAoI(pEntity);
		}

		Py_DECREF(pEntity);
		Py_DECREF(pSelfEntity);
		return;
	}

	//DEBUG_MSG(fmt::format("Witness::onEnterAOI: {} entity={}\n", 
	//	pEntity_->id(), pEntity->id()));
	
	EntityRef* pEntityRef = EntityRef::createPoolObject();
	pEntityRef->pEntity(pEntity);
	pEntityRef->flags(pEntityRef->flags() | ENTITYREF_FLAG_ENTER_CLIENT_PENDING);
	aoiEntities_.push_back(pEntityRef);
	aoiEntities_map_[pEntityRef->id()] = pEntityRef;
	pEntityRef->aliasID(aoiEntities_map_.size() - 1);
	
	pEntity->addWitnessed(pEntity_);
	pSelfEntity->onEnteredAoI(pEntity);

	Py_DECREF(pEntity);
	Py_DECREF(pSelfEntity);
}

//-------------------------------------------------------------------------------------
void Witness::onLeaveAOI(AOITrigger* pAOITrigger, Entity* pEntity)
{
	// ������ù�Hysteresis������ô�뿪Hysteresis��������뿪AOI
	if (pAOIHysteresisAreaTrigger_ && pAOIHysteresisAreaTrigger_ != pAOITrigger)
		return;

	AOI_ENTITIES_MAP::iterator iter = aoiEntities_map_.find(pEntity->id());
	if (iter == aoiEntities_map_.end())
		return;

	_onLeaveAOI(iter->second);
}

//-------------------------------------------------------------------------------------
void Witness::_onLeaveAOI(EntityRef* pEntityRef)
{
	//DEBUG_MSG(fmt::format("Witness::onLeaveAOI: {} entity={}\n", 
	//	pEntity_->id(), pEntityRef->id()));

	// ���ﲻdelete�� ������Ҫ��update������Ϊ�������ͻ���ʱ�ٽ���
	//EntityRef::reclaimPoolObject((*iter));
	//aoiEntities_.erase(iter);
	//aoiEntities_map_.erase(iter);

	pEntityRef->flags(((pEntityRef->flags() | ENTITYREF_FLAG_LEAVE_CLIENT_PENDING) & ~(ENTITYREF_FLAG_ENTER_CLIENT_PENDING)));

	if(pEntityRef->pEntity())
		pEntityRef->pEntity()->delWitnessed(pEntity_);

	pEntityRef->pEntity(NULL);
}

//-------------------------------------------------------------------------------------
void Witness::resetAOIEntities()
{
	clientAOISize_ = 0;
	AOI_ENTITIES::iterator iter = aoiEntities_.begin();
	for(; iter != aoiEntities_.end(); )
	{
		if(((*iter)->flags() & ENTITYREF_FLAG_LEAVE_CLIENT_PENDING) > 0)
		{
			aoiEntities_map_.erase((*iter)->id());
			EntityRef::reclaimPoolObject((*iter));
			iter = aoiEntities_.erase(iter);
			continue;
		}

		(*iter)->flags(ENTITYREF_FLAG_ENTER_CLIENT_PENDING);
		++iter;
	}
	
	updateEntitiesAliasID();
}

//-------------------------------------------------------------------------------------
void Witness::onEnterSpace(Space* pSpace)
{
	Network::Bundle* pSendBundle = Network::Bundle::createPoolObject();
	NETWORK_ENTITY_MESSAGE_FORWARD_CLIENT_START(pEntity_->id(), (*pSendBundle));

	// ֪ͨλ��ǿ�Ƹı�
	Position3D &pos = pEntity_->position();
	Direction3D &dir = pEntity_->direction();
	ENTITY_MESSAGE_FORWARD_CLIENT_START(pSendBundle, ClientInterface::onSetEntityPosAndDir, setEntityPosAndDir);
	(*pSendBundle) << pEntity_->id();
	(*pSendBundle) << pos.x << pos.y << pos.z;
	(*pSendBundle) << dir.roll() << dir.pitch() << dir.yaw();
	ENTITY_MESSAGE_FORWARD_CLIENT_END(pSendBundle, ClientInterface::onSetEntityPosAndDir, setEntityPosAndDir);
	
	// ֪ͨ�������µ�ͼ
	ENTITY_MESSAGE_FORWARD_CLIENT_START(pSendBundle, ClientInterface::onEntityEnterSpace, entityEnterSpace);

	(*pSendBundle) << pEntity_->id();
	(*pSendBundle) << pSpace->id();
	if(!pEntity_->isOnGround())
		(*pSendBundle) << pEntity_->isOnGround();

	ENTITY_MESSAGE_FORWARD_CLIENT_END(pSendBundle, ClientInterface::onEntityEnterSpace, entityEnterSpace);

	// ������Ϣ������
	pEntity_->clientMailbox()->postMail(pSendBundle);

	installAOITrigger();
}

//-------------------------------------------------------------------------------------
void Witness::onLeaveSpace(Space* pSpace)
{
	uninstallAOITrigger();

	Network::Bundle* pSendBundle = Network::Bundle::createPoolObject();
	NETWORK_ENTITY_MESSAGE_FORWARD_CLIENT_START(pEntity_->id(), (*pSendBundle));

	ENTITY_MESSAGE_FORWARD_CLIENT_START(pSendBundle, ClientInterface::onEntityLeaveSpace, entityLeaveSpace);
	(*pSendBundle) << pEntity_->id();
	ENTITY_MESSAGE_FORWARD_CLIENT_END(pSendBundle, ClientInterface::onEntityLeaveSpace, entityLeaveSpace);
	pEntity_->clientMailbox()->postMail(pSendBundle);

	lastBasePos_.z = -FLT_MAX;
	lastBaseDir_.yaw(-FLT_MAX);

	AOI_ENTITIES::iterator iter = aoiEntities_.begin();
	for(; iter != aoiEntities_.end(); ++iter)
	{
		if((*iter)->pEntity())
		{
			(*iter)->pEntity()->delWitnessed(pEntity_);
		}

		EntityRef::reclaimPoolObject((*iter));
	}

	aoiEntities_.clear();
	aoiEntities_map_.clear();

	clientAOISize_ = 0;
}

//-------------------------------------------------------------------------------------
void Witness::installAOITrigger()
{
	if (pAOITrigger_)
	{
		// ������AOI�뾶Ϊ0������ص�½������������
		if (aoiRadius_ <= 0.f)
			return;

		// �����Ȱ�װpAOIHysteresisAreaTrigger_������һЩ�����������ִ���Ľ��
		// ���磺һ��Avatar���ý��뵽�����ʱ���ڰ�װAOI������������װ���������ʵ��onWitnessed������������������
		// ����AOI��������δ��ȫ��װ��ϵ��´������Ľڵ�old_xx�ȶ�Ϊ-FLT_MAX�����Ը�ʵ�����뿪���������ʱAvatar��AOI�������жϴ���
		// ����Ȱ�װpAOIHysteresisAreaTrigger_�򲻻ᴥ��ʵ�����AOI�¼��������ڰ�װpAOITrigger_ʱ�����¼�����������ֵ�����ʱҲ��֮ǰ�����뿪�¼���
		if (pAOIHysteresisAreaTrigger_ && pEntity_/*�������̿��ܵ������� */)
			pAOIHysteresisAreaTrigger_->reinstall((CoordinateNode*)pEntity_->pEntityCoordinateNode());

		if (pEntity_/*�������̿��ܵ������� */)
			pAOITrigger_->reinstall((CoordinateNode*)pEntity_->pEntityCoordinateNode());
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

	// ֪ͨ����ʵ���뿪AOI
	AOI_ENTITIES::iterator iter = aoiEntities_.begin();
	for (; iter != aoiEntities_.end(); ++iter)
	{
		_onLeaveAOI((*iter));
	}
}

//-------------------------------------------------------------------------------------
bool Witness::pushBundle(Network::Bundle* pBundle)
{
	Network::Channel* pc = pChannel();
	if(!pc)
		return false;

	pc->send(pBundle);
	return true;
}

//-------------------------------------------------------------------------------------
Network::Channel* Witness::pChannel()
{
	if(pEntity_ == NULL)
		return NULL;

	EntityMailbox* clientMB = pEntity_->clientMailbox();
	if(!clientMB)
		return NULL;

	Network::Channel* pChannel = clientMB->getChannel();
	if(!pChannel)
		return NULL;
	
	return pChannel;
}

//-------------------------------------------------------------------------------------
void Witness::_addAOIEntityIDToBundle(Network::Bundle* pBundle, EntityRef* pEntityRef)
{
	if(!EntityDef::entityAliasID())
	{
		(*pBundle) << pEntityRef->id();
	}
	else
	{
		// ע�⣺�����ڸ�ģ���ⲿʹ�ã�������ܳ��ֿͻ��˱��Ҳ���entityID�����
		// clientAOISize_��Ҫʵ������ͬ�����ͻ���ʱ�Ż�����
		if(clientAOISize_ > 255)
		{
			(*pBundle) << pEntityRef->id();
		}
		else
		{
			if ((pEntityRef->flags() & (ENTITYREF_FLAG_NORMAL)) > 0)
			{
				KBE_ASSERT(pEntityRef->aliasID() <= 255);
				(*pBundle) << (uint8)pEntityRef->aliasID();
			}
			else
			{
				(*pBundle) << pEntityRef->id();
			}
		}
	}
}

//-------------------------------------------------------------------------------------
const Network::MessageHandler& Witness::getAOIEntityMessageHandler(const Network::MessageHandler& normalMsgHandler, 
	const Network::MessageHandler& optimizedMsgHandler, ENTITY_ID entityID, int& ialiasID)
{
	ialiasID = -1;
	if(!EntityDef::entityAliasID())
	{
		return normalMsgHandler;
	}
	else
	{
		if (clientAOISize_ > 255)
		{
			return normalMsgHandler;
		}
		else
		{
			uint8 aliasID = 0;
			if(entityID2AliasID(entityID, aliasID))
			{
				ialiasID = aliasID;
				return optimizedMsgHandler;
			}
			else
			{
				return normalMsgHandler;
			}
		}
	}
	
	return normalMsgHandler;
}

//-------------------------------------------------------------------------------------
bool Witness::entityID2AliasID(ENTITY_ID id, uint8& aliasID)
{
	AOI_ENTITIES_MAP::iterator iter = aoiEntities_map_.find(id);
	if (iter == aoiEntities_map_.end())
	{
		aliasID = 0;
		return false;
	}

	EntityRef* pEntityRef = iter->second;
	if ((pEntityRef->flags() & (ENTITYREF_FLAG_NORMAL)) <= 0)
	{
		aliasID = 0;
		return false;
	}

	// ���
	if (pEntityRef->aliasID() > 255)
	{
		aliasID = 0;
		return false;
	}
	
	aliasID = (uint8)pEntityRef->aliasID();
	return true;
}

//-------------------------------------------------------------------------------------
void Witness::updateEntitiesAliasID()
{
	int n = 0;
	AOI_ENTITIES::iterator iter = aoiEntities_.begin();
	for(; iter != aoiEntities_.end(); ++iter)
	{
		EntityRef* pEntityRef = (*iter);
		pEntityRef->aliasID(n++);
		
		if(n >= 255)
			break;
	}
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

	if (aoiEntities_map_.size() > 0 || pEntity_->isControlledNotSelfClient())
	{
		Network::Bundle* pSendBundle = pChannel->createSendBundle();
		
		// �õ���ǰpSendBundle���Ƿ������ݣ���������ݱ�ʾ��bundle�����õĻ�������ݰ�
		bool isBufferedSendBundleMessageLength = pSendBundle->packets().size() > 0 ? true : 
			(pSendBundle->pCurrPacket() && pSendBundle->pCurrPacket()->length() > 0);
		
		NETWORK_ENTITY_MESSAGE_FORWARD_CLIENT_START(pEntity_->id(), (*pSendBundle));
		addBaseDataToStream(pSendBundle);

		AOI_ENTITIES::iterator iter = aoiEntities_.begin();
		for(; iter != aoiEntities_.end(); )
		{
			EntityRef* pEntityRef = (*iter);
			
			if((pEntityRef->flags() & ENTITYREF_FLAG_ENTER_CLIENT_PENDING) > 0)
			{
				// ����ʹ��id����һ�£� ����entity�ڽ���AOIʱ�Ļص��ﱻ��������
				Entity* otherEntity = Cellapp::getSingleton().findEntity(pEntityRef->id());
				if(otherEntity == NULL)
				{
					pEntityRef->pEntity(NULL);
					_onLeaveAOI(pEntityRef);
					aoiEntities_map_.erase(pEntityRef->id());
					EntityRef::reclaimPoolObject(pEntityRef);
					iter = aoiEntities_.erase(iter);
					updateEntitiesAliasID();
					continue;
				}
				
				pEntityRef->removeflags(ENTITYREF_FLAG_ENTER_CLIENT_PENDING);

				MemoryStream* s1 = MemoryStream::createPoolObject();
				otherEntity->addPositionAndDirectionToStream(*s1, true);			
				otherEntity->addClientDataToStream(s1, true);
				
				ENTITY_MESSAGE_FORWARD_CLIENT_START(pSendBundle, ClientInterface::onUpdatePropertys, updatePropertys);
				(*pSendBundle) << otherEntity->id();
				(*pSendBundle).append(*s1);
				MemoryStream::reclaimPoolObject(s1);
				ENTITY_MESSAGE_FORWARD_CLIENT_END(pSendBundle, ClientInterface::onUpdatePropertys, updatePropertys);
				
				ENTITY_MESSAGE_FORWARD_CLIENT_START(pSendBundle, ClientInterface::onEntityEnterWorld, entityEnterWorld);
				(*pSendBundle) << otherEntity->id();
				otherEntity->pScriptModule()->addSmartUTypeToBundle(pSendBundle);
				if(!otherEntity->isOnGround())
					(*pSendBundle) << otherEntity->isOnGround();

				ENTITY_MESSAGE_FORWARD_CLIENT_END(pSendBundle, ClientInterface::onEntityEnterWorld, entityEnterWorld);

				pEntityRef->flags(ENTITYREF_FLAG_NORMAL);

				KBE_ASSERT(clientAOISize_ != 65535);

				++clientAOISize_;
			}
			else if((pEntityRef->flags() & ENTITYREF_FLAG_LEAVE_CLIENT_PENDING) > 0)
			{
				pEntityRef->removeflags(ENTITYREF_FLAG_LEAVE_CLIENT_PENDING);

				if((pEntityRef->flags() & ENTITYREF_FLAG_NORMAL) > 0)
				{
					ENTITY_MESSAGE_FORWARD_CLIENT_START(pSendBundle, ClientInterface::onEntityLeaveWorldOptimized, leaveWorld);
					_addAOIEntityIDToBundle(pSendBundle, pEntityRef);
					ENTITY_MESSAGE_FORWARD_CLIENT_END(pSendBundle, ClientInterface::onEntityLeaveWorldOptimized, leaveWorld);
					
					KBE_ASSERT(clientAOISize_ > 0);
					--clientAOISize_;
				}

				aoiEntities_map_.erase(pEntityRef->id());
				EntityRef::reclaimPoolObject(pEntityRef);
				iter = aoiEntities_.erase(iter);
				updateEntitiesAliasID();
				continue;
			}
			else
			{
				Entity* otherEntity = pEntityRef->pEntity();
				if(otherEntity == NULL)
				{
					aoiEntities_map_.erase(pEntityRef->id());
					EntityRef::reclaimPoolObject(pEntityRef);
					iter = aoiEntities_.erase(iter);
					KBE_ASSERT(clientAOISize_ > 0);
					--clientAOISize_;
					updateEntitiesAliasID();
					continue;
				}
				
				KBE_ASSERT(pEntityRef->flags() == ENTITYREF_FLAG_NORMAL);
				
				addUpdateToStream(pSendBundle, getEntityVolatileDataUpdateFlags(otherEntity), pEntityRef);
			}

			++iter;
		}

		size_t pSendBundleMessageLength = pSendBundle->currMsgLength();
		if (pSendBundleMessageLength > 8/*NETWORK_ENTITY_MESSAGE_FORWARD_CLIENT_START�����Ļ�������С*/)
		{
			if(pSendBundleMessageLength > PACKET_MAX_SIZE_TCP)
			{
				WARNING_MSG(fmt::format("Witness::update({}): sendToClient {} Bytes.\n", 
					pEntity_->id(), pSendBundleMessageLength));
			}

			AUTO_SCOPED_PROFILE("sendToClient");
			pChannel->send(pSendBundle);
		}
		else
		{
			// ���bundle��channel����İ�
			// ȡ�����ظ����õ�����붪��������Ϣ����
			// ��ʱӦ�ý�NETWORK_ENTITY_MESSAGE_FORWARD_CLIENT_START������Ĩ����
			if(isBufferedSendBundleMessageLength)
			{
				KBE_ASSERT(pSendBundleMessageLength == 8);
				pSendBundle->revokeMessage(8);
				pChannel->pushBundle(pSendBundle);
			}
			else
			{
				Network::Bundle::reclaimPoolObject(pSendBundle);
			}
		}
	}

	return true;
}

//-------------------------------------------------------------------------------------
void Witness::addBaseDataToStream(Network::Bundle* pSendBundle)
{
	if (pEntity_->isControlledNotSelfClient())
	{
		const Direction3D& bdir = baseDir();
		Vector3 changeDir = bdir.dir - lastBaseDir_.dir;

		if (KBEVec3Length(&changeDir) > 0.0004f)
		{
			ENTITY_MESSAGE_FORWARD_CLIENT_START(pSendBundle, ClientInterface::onUpdateBaseDir, onUpdateBaseDir);
			(*pSendBundle) << bdir.yaw() << bdir.pitch() << bdir.roll();
			ENTITY_MESSAGE_FORWARD_CLIENT_END(pSendBundle, ClientInterface::onUpdateBaseDir, onUpdateBaseDir);
			lastBaseDir_ = bdir;
		}
	}

	const Position3D& bpos = basePos();
	Vector3 movement = bpos - lastBasePos_;

	if(KBEVec3Length(&movement) < 0.0004f)
		return;

	if (fabs(lastBasePos_.y - bpos.y) > 0.0004f)
	{
		ENTITY_MESSAGE_FORWARD_CLIENT_START(pSendBundle, ClientInterface::onUpdateBasePos, basePos);
		pSendBundle->appendPackAnyXYZ(bpos.x, bpos.y, bpos.z, 0.f);
		ENTITY_MESSAGE_FORWARD_CLIENT_END(pSendBundle, ClientInterface::onUpdateBasePos, basePos);
	}
	else
	{
		ENTITY_MESSAGE_FORWARD_CLIENT_START(pSendBundle, ClientInterface::onUpdateBasePosXZ, basePos);
		pSendBundle->appendPackAnyXZ(bpos.x, bpos.z, 0.f);
		ENTITY_MESSAGE_FORWARD_CLIENT_END(pSendBundle, ClientInterface::onUpdateBasePosXZ, basePos);
	}

	lastBasePos_ = bpos;
}

//-------------------------------------------------------------------------------------
void Witness::addUpdateToStream(Network::Bundle* pForwardBundle, uint32 flags, EntityRef* pEntityRef)
{
	Entity* otherEntity = pEntityRef->pEntity();

	switch(flags)
	{
	case UPDATE_FLAG_NULL:
		{
			// (*pForwardBundle).newMessage(ClientInterface::onUpdateData);
		}
		break;
	case UPDATE_FLAG_XZ:
		{
			Position3D relativePos = otherEntity->position() - this->pEntity()->position();
			
			ENTITY_MESSAGE_FORWARD_CLIENT_START(pForwardBundle, ClientInterface::onUpdateData_xz, update);
			_addAOIEntityIDToBundle(pForwardBundle, pEntityRef);
			pForwardBundle->appendPackXZ(relativePos.x, relativePos.z);
			ENTITY_MESSAGE_FORWARD_CLIENT_END(pForwardBundle, ClientInterface::onUpdateData_xz, update);
		}
		break;
	case UPDATE_FLAG_XYZ:
		{
			Position3D relativePos = otherEntity->position() - this->pEntity()->position();
			
			ENTITY_MESSAGE_FORWARD_CLIENT_START(pForwardBundle, ClientInterface::onUpdateData_xyz, update);
			_addAOIEntityIDToBundle(pForwardBundle, pEntityRef);
			pForwardBundle->appendPackXZ(relativePos.x, relativePos.z);
			pForwardBundle->appendPackY(relativePos.y);
			ENTITY_MESSAGE_FORWARD_CLIENT_END(pForwardBundle, ClientInterface::onUpdateData_xyz, update);
		}
		break;
	case UPDATE_FLAG_YAW:
		{
			const Direction3D& dir = otherEntity->direction();
			
			ENTITY_MESSAGE_FORWARD_CLIENT_START(pForwardBundle, ClientInterface::onUpdateData_y, update);
			_addAOIEntityIDToBundle(pForwardBundle, pEntityRef);
			(*pForwardBundle) << angle2int8(dir.yaw());
			ENTITY_MESSAGE_FORWARD_CLIENT_END(pForwardBundle, ClientInterface::onUpdateData_y, update);
		}
		break;
	case UPDATE_FLAG_ROLL:
		{
			const Direction3D& dir = otherEntity->direction();
			
			ENTITY_MESSAGE_FORWARD_CLIENT_START(pForwardBundle, ClientInterface::onUpdateData_r, update);
			_addAOIEntityIDToBundle(pForwardBundle, pEntityRef);
			(*pForwardBundle) << angle2int8(dir.roll());
			ENTITY_MESSAGE_FORWARD_CLIENT_END(pForwardBundle, ClientInterface::onUpdateData_r, update);
		}
		break;
	case UPDATE_FLAG_PITCH:
		{
			const Direction3D& dir = otherEntity->direction();

			ENTITY_MESSAGE_FORWARD_CLIENT_START(pForwardBundle, ClientInterface::onUpdateData_p, update);
			_addAOIEntityIDToBundle(pForwardBundle, pEntityRef);
			(*pForwardBundle) << angle2int8(dir.pitch());
			ENTITY_MESSAGE_FORWARD_CLIENT_END(pForwardBundle, ClientInterface::onUpdateData_p, update);
		}
		break;
	case UPDATE_FLAG_YAW_PITCH_ROLL:
		{
			const Direction3D& dir = otherEntity->direction();

			ENTITY_MESSAGE_FORWARD_CLIENT_START(pForwardBundle, ClientInterface::onUpdateData_ypr, update);
			_addAOIEntityIDToBundle(pForwardBundle, pEntityRef);
			(*pForwardBundle) << angle2int8(dir.yaw());
			(*pForwardBundle) << angle2int8(dir.pitch());
			(*pForwardBundle) << angle2int8(dir.roll());
			ENTITY_MESSAGE_FORWARD_CLIENT_END(pForwardBundle, ClientInterface::onUpdateData_ypr, update);
		}
		break;
	case UPDATE_FLAG_YAW_PITCH:
		{
			const Direction3D& dir = otherEntity->direction();
			
			ENTITY_MESSAGE_FORWARD_CLIENT_START(pForwardBundle, ClientInterface::onUpdateData_yp, update);
			_addAOIEntityIDToBundle(pForwardBundle, pEntityRef);
			(*pForwardBundle) << angle2int8(dir.yaw());
			(*pForwardBundle) << angle2int8(dir.pitch());
			ENTITY_MESSAGE_FORWARD_CLIENT_END(pForwardBundle, ClientInterface::onUpdateData_yp, update);
		}
		break;
	case UPDATE_FLAG_YAW_ROLL:
		{
			const Direction3D& dir = otherEntity->direction();
			
			ENTITY_MESSAGE_FORWARD_CLIENT_START(pForwardBundle, ClientInterface::onUpdateData_yr, update);
			_addAOIEntityIDToBundle(pForwardBundle, pEntityRef);
			(*pForwardBundle) << angle2int8(dir.yaw());
			(*pForwardBundle) << angle2int8(dir.roll());
			ENTITY_MESSAGE_FORWARD_CLIENT_END(pForwardBundle, ClientInterface::onUpdateData_yr, update);
		}
		break;
	case UPDATE_FLAG_PITCH_ROLL:
		{
			const Direction3D& dir = otherEntity->direction();
			
			ENTITY_MESSAGE_FORWARD_CLIENT_START(pForwardBundle, ClientInterface::onUpdateData_pr, update);
			_addAOIEntityIDToBundle(pForwardBundle, pEntityRef);
			(*pForwardBundle) << angle2int8(dir.pitch());
			(*pForwardBundle) << angle2int8(dir.roll());
			ENTITY_MESSAGE_FORWARD_CLIENT_END(pForwardBundle, ClientInterface::onUpdateData_pr, update);
		}
		break;
	case (UPDATE_FLAG_XZ | UPDATE_FLAG_YAW):
		{
			Position3D relativePos = otherEntity->position() - this->pEntity()->position();
			const Direction3D& dir = otherEntity->direction();
			
			ENTITY_MESSAGE_FORWARD_CLIENT_START(pForwardBundle, ClientInterface::onUpdateData_xz_y, update);
			_addAOIEntityIDToBundle(pForwardBundle, pEntityRef);
			pForwardBundle->appendPackXZ(relativePos.x, relativePos.z);
			(*pForwardBundle) << angle2int8(dir.yaw());
			ENTITY_MESSAGE_FORWARD_CLIENT_END(pForwardBundle, ClientInterface::onUpdateData_xz_y, update);
		}
		break;
	case (UPDATE_FLAG_XZ | UPDATE_FLAG_PITCH):
		{
			Position3D relativePos = otherEntity->position() - this->pEntity()->position();
			const Direction3D& dir = otherEntity->direction();
			
			ENTITY_MESSAGE_FORWARD_CLIENT_START(pForwardBundle, ClientInterface::onUpdateData_xz_p, update);
			_addAOIEntityIDToBundle(pForwardBundle, pEntityRef);
			pForwardBundle->appendPackXZ(relativePos.x, relativePos.z);
			(*pForwardBundle) << angle2int8(dir.pitch());
			ENTITY_MESSAGE_FORWARD_CLIENT_END(pForwardBundle, ClientInterface::onUpdateData_xz_p, update);
		}
		break;
	case (UPDATE_FLAG_XZ | UPDATE_FLAG_ROLL):
		{
			Position3D relativePos = otherEntity->position() - this->pEntity()->position();
			const Direction3D& dir = otherEntity->direction();
			
			ENTITY_MESSAGE_FORWARD_CLIENT_START(pForwardBundle, ClientInterface::onUpdateData_xz_r, update);
			_addAOIEntityIDToBundle(pForwardBundle, pEntityRef);
			pForwardBundle->appendPackXZ(relativePos.x, relativePos.z);
			(*pForwardBundle) << angle2int8(dir.roll());
			ENTITY_MESSAGE_FORWARD_CLIENT_END(pForwardBundle, ClientInterface::onUpdateData_xz_r, update);
		}
		break;
	case (UPDATE_FLAG_XZ | UPDATE_FLAG_YAW_ROLL):
		{
			Position3D relativePos = otherEntity->position() - this->pEntity()->position();
			const Direction3D& dir = otherEntity->direction();

			ENTITY_MESSAGE_FORWARD_CLIENT_START(pForwardBundle, ClientInterface::onUpdateData_xz_yr, update);
			_addAOIEntityIDToBundle(pForwardBundle, pEntityRef);
			pForwardBundle->appendPackXZ(relativePos.x, relativePos.z);
			(*pForwardBundle) << angle2int8(dir.yaw());
			(*pForwardBundle) << angle2int8(dir.roll());
			ENTITY_MESSAGE_FORWARD_CLIENT_END(pForwardBundle, ClientInterface::onUpdateData_xz_yr, update);
		}
		break;
	case (UPDATE_FLAG_XZ | UPDATE_FLAG_YAW_PITCH):
		{
			Position3D relativePos = otherEntity->position() - this->pEntity()->position();
			const Direction3D& dir = otherEntity->direction();

			ENTITY_MESSAGE_FORWARD_CLIENT_START(pForwardBundle, ClientInterface::onUpdateData_xz_yp, update);
			_addAOIEntityIDToBundle(pForwardBundle, pEntityRef);
			pForwardBundle->appendPackXZ(relativePos.x, relativePos.z);
			(*pForwardBundle) << angle2int8(dir.yaw());
			(*pForwardBundle) << angle2int8(dir.pitch());
			ENTITY_MESSAGE_FORWARD_CLIENT_END(pForwardBundle, ClientInterface::onUpdateData_xz_yp, update);
		}
		break;
	case (UPDATE_FLAG_XZ | UPDATE_FLAG_PITCH_ROLL):
		{
			Position3D relativePos = otherEntity->position() - this->pEntity()->position();
			const Direction3D& dir = otherEntity->direction();
			
			ENTITY_MESSAGE_FORWARD_CLIENT_START(pForwardBundle, ClientInterface::onUpdateData_xz_pr, update);
			_addAOIEntityIDToBundle(pForwardBundle, pEntityRef);
			pForwardBundle->appendPackXZ(relativePos.x, relativePos.z);
			(*pForwardBundle) << angle2int8(dir.pitch());
			(*pForwardBundle) << angle2int8(dir.roll());
			ENTITY_MESSAGE_FORWARD_CLIENT_END(pForwardBundle, ClientInterface::onUpdateData_xz_pr, update);
		}
		break;
	case (UPDATE_FLAG_XZ | UPDATE_FLAG_YAW_PITCH_ROLL):
		{
			Position3D relativePos = otherEntity->position() - this->pEntity()->position();
			const Direction3D& dir = otherEntity->direction();
			
			ENTITY_MESSAGE_FORWARD_CLIENT_START(pForwardBundle, ClientInterface::onUpdateData_xz_ypr, update);
			_addAOIEntityIDToBundle(pForwardBundle, pEntityRef);
			pForwardBundle->appendPackXZ(relativePos.x, relativePos.z);
			(*pForwardBundle) << angle2int8(dir.yaw());
			(*pForwardBundle) << angle2int8(dir.pitch());
			(*pForwardBundle) << angle2int8(dir.roll());
			ENTITY_MESSAGE_FORWARD_CLIENT_END(pForwardBundle, ClientInterface::onUpdateData_xz_ypr, update);
		}
		break;
	case (UPDATE_FLAG_XYZ | UPDATE_FLAG_YAW):
		{
			Position3D relativePos = otherEntity->position() - this->pEntity()->position();
			const Direction3D& dir = otherEntity->direction();

			ENTITY_MESSAGE_FORWARD_CLIENT_START(pForwardBundle, ClientInterface::onUpdateData_xyz_y, update);
			_addAOIEntityIDToBundle(pForwardBundle, pEntityRef);
			pForwardBundle->appendPackXZ(relativePos.x, relativePos.z);
			pForwardBundle->appendPackY(relativePos.y);
			(*pForwardBundle) << angle2int8(dir.yaw());
			ENTITY_MESSAGE_FORWARD_CLIENT_END(pForwardBundle, ClientInterface::onUpdateData_xyz_y, update);
		}
		break;
	case (UPDATE_FLAG_XYZ | UPDATE_FLAG_PITCH):
		{
			Position3D relativePos = otherEntity->position() - this->pEntity()->position();
			const Direction3D& dir = otherEntity->direction();

			ENTITY_MESSAGE_FORWARD_CLIENT_START(pForwardBundle, ClientInterface::onUpdateData_xyz_p, update);
			_addAOIEntityIDToBundle(pForwardBundle, pEntityRef);
			pForwardBundle->appendPackXZ(relativePos.x, relativePos.z);
			pForwardBundle->appendPackY(relativePos.y);
			(*pForwardBundle) << angle2int8(dir.pitch());
			ENTITY_MESSAGE_FORWARD_CLIENT_END(pForwardBundle, ClientInterface::onUpdateData_xyz_p, update);
		}
		break;
	case (UPDATE_FLAG_XYZ | UPDATE_FLAG_ROLL):
		{
			Position3D relativePos = otherEntity->position() - this->pEntity()->position();
			const Direction3D& dir = otherEntity->direction();

			ENTITY_MESSAGE_FORWARD_CLIENT_START(pForwardBundle, ClientInterface::onUpdateData_xyz_r, update);
			_addAOIEntityIDToBundle(pForwardBundle, pEntityRef);
			pForwardBundle->appendPackXZ(relativePos.x, relativePos.z);
			pForwardBundle->appendPackY(relativePos.y);
			(*pForwardBundle) << angle2int8(dir.roll());
			ENTITY_MESSAGE_FORWARD_CLIENT_END(pForwardBundle, ClientInterface::onUpdateData_xyz_r, update);
		}
		break;
	case (UPDATE_FLAG_XYZ | UPDATE_FLAG_YAW_ROLL):
		{
			Position3D relativePos = otherEntity->position() - this->pEntity()->position();
			const Direction3D& dir = otherEntity->direction();

			ENTITY_MESSAGE_FORWARD_CLIENT_START(pForwardBundle, ClientInterface::onUpdateData_xyz_yr, update);
			_addAOIEntityIDToBundle(pForwardBundle, pEntityRef);
			pForwardBundle->appendPackXZ(relativePos.x, relativePos.z);
			pForwardBundle->appendPackY(relativePos.y);
			(*pForwardBundle) << angle2int8(dir.yaw());
			(*pForwardBundle) << angle2int8(dir.roll());
			ENTITY_MESSAGE_FORWARD_CLIENT_END(pForwardBundle, ClientInterface::onUpdateData_xyz_yr, update);
		}
		break;
	case (UPDATE_FLAG_XYZ | UPDATE_FLAG_YAW_PITCH):
		{
			Position3D relativePos = otherEntity->position() - this->pEntity()->position();
			const Direction3D& dir = otherEntity->direction();

			ENTITY_MESSAGE_FORWARD_CLIENT_START(pForwardBundle, ClientInterface::onUpdateData_xyz_yp, update);
			_addAOIEntityIDToBundle(pForwardBundle, pEntityRef);
			pForwardBundle->appendPackXZ(relativePos.x, relativePos.z);
			pForwardBundle->appendPackY(relativePos.y);
			(*pForwardBundle) << angle2int8(dir.yaw());
			(*pForwardBundle) << angle2int8(dir.pitch());
			ENTITY_MESSAGE_FORWARD_CLIENT_END(pForwardBundle, ClientInterface::onUpdateData_xyz_yp, update);
		}
		break;
	case (UPDATE_FLAG_XYZ | UPDATE_FLAG_PITCH_ROLL):
		{
			Position3D relativePos = otherEntity->position() - this->pEntity()->position();
			const Direction3D& dir = otherEntity->direction();

			ENTITY_MESSAGE_FORWARD_CLIENT_START(pForwardBundle, ClientInterface::onUpdateData_xyz_pr, update);
			_addAOIEntityIDToBundle(pForwardBundle, pEntityRef);
			pForwardBundle->appendPackXZ(relativePos.x, relativePos.z);
			pForwardBundle->appendPackY(relativePos.y);
			(*pForwardBundle) << angle2int8(dir.pitch());
			(*pForwardBundle) << angle2int8(dir.roll());
			ENTITY_MESSAGE_FORWARD_CLIENT_END(pForwardBundle, ClientInterface::onUpdateData_xyz_pr, update);
		}
		break;
	case (UPDATE_FLAG_XYZ | UPDATE_FLAG_YAW_PITCH_ROLL):
		{
			Position3D relativePos = otherEntity->position() - this->pEntity()->position();
			const Direction3D& dir = otherEntity->direction();

			ENTITY_MESSAGE_FORWARD_CLIENT_START(pForwardBundle, ClientInterface::onUpdateData_xyz_ypr, update);
			_addAOIEntityIDToBundle(pForwardBundle, pEntityRef);
			pForwardBundle->appendPackXZ(relativePos.x, relativePos.z);
			pForwardBundle->appendPackY(relativePos.y);
			(*pForwardBundle) << angle2int8(dir.yaw());
			(*pForwardBundle) << angle2int8(dir.pitch());
			(*pForwardBundle) << angle2int8(dir.roll());
			ENTITY_MESSAGE_FORWARD_CLIENT_END(pForwardBundle, ClientInterface::onUpdateData_xyz_ypr, update);
		}
		break;
	default:
		KBE_ASSERT(false);
		break;
	};
}

//-------------------------------------------------------------------------------------
uint32 Witness::getEntityVolatileDataUpdateFlags(Entity* otherEntity)
{
	uint32 flags = UPDATE_FLAG_NULL;

	/* ���Ŀ�걻�ҿ����ˣ���Ŀ���λ�ò�֪ͨ�ҵĿͻ��ˡ�
	   ע�⣺��������ҿ��Ƶ�entity�ڷ�������ʹ��moveToPoint()�Ƚӿ��ƶ�ʱ��
	         Ҳ����������ж��������겻��ͬ���������ߵĿͻ�����
	*/
	if (otherEntity->controlledBy() && pEntity_->id() == otherEntity->controlledBy()->id())
		return flags;

	const VolatileInfo* pVolatileInfo = otherEntity->pCustomVolatileinfo();
	if (!pVolatileInfo)
		pVolatileInfo = otherEntity->pScriptModule()->getPVolatileInfo();

	static uint16 entity_posdir_additional_updates = g_kbeSrvConfig.getCellApp().entity_posdir_additional_updates;
	
	if ((pVolatileInfo->position() > 0.f) && (entity_posdir_additional_updates == 0 || g_kbetime - otherEntity->posChangedTime() < entity_posdir_additional_updates))
	{
		if (!otherEntity->isOnGround() || !pVolatileInfo->optimized())
		{
			flags |= UPDATE_FLAG_XYZ; 
		}
		else
		{
			flags |= UPDATE_FLAG_XZ; 
		}
	}

	if((entity_posdir_additional_updates == 0) || (g_kbetime - otherEntity->dirChangedTime() < entity_posdir_additional_updates))
	{
		if (pVolatileInfo->yaw() > 0.f)
		{
			if (pVolatileInfo->roll() > 0.f)
			{
				if (pVolatileInfo->pitch() > 0.f)
				{
					flags |= UPDATE_FLAG_YAW_PITCH_ROLL;
				}
				else
				{
					flags |= UPDATE_FLAG_YAW_ROLL;
				}
			}
			else if (pVolatileInfo->pitch() > 0.f)
			{
				flags |= UPDATE_FLAG_YAW_PITCH;
			}
			else
			{
				flags |= UPDATE_FLAG_YAW;
			}
		}
		else if (pVolatileInfo->roll() > 0.f)
		{
			if (pVolatileInfo->pitch() > 0.f)
			{
				flags |= UPDATE_FLAG_PITCH_ROLL;
			}
			else
			{
				flags |= UPDATE_FLAG_ROLL;
			}
		}
		else if (pVolatileInfo->pitch() > 0.f)
		{
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
	Network::Bundle::reclaimPoolObject(pBundle);
	return false;
}

//-------------------------------------------------------------------------------------
}
