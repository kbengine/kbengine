#include "cellapp.hpp"
#include "space.hpp"	
#include "entity.hpp"	
#include "entitydef/entities.hpp"
#include "client_lib/client_interface.hpp"

#include "../../server/baseappmgr/baseappmgr_interface.hpp"
#include "../../server/cellappmgr/cellappmgr_interface.hpp"
#include "../../server/baseapp/baseapp_interface.hpp"
#include "../../server/cellapp/cellapp_interface.hpp"
#include "../../server/dbmgr/dbmgr_interface.hpp"

namespace KBEngine{	

//-------------------------------------------------------------------------------------
Space::Space(SPACE_ID spaceID, int32 mapSize):
id_(spaceID),
entities_(),
isLoadGeometry_(false),
mapSize_(mapSize),
loadGeometryPathName_()
{
}

//-------------------------------------------------------------------------------------
Space::~Space()
{
}

//-------------------------------------------------------------------------------------
void Space::loadSpaceGeometry(const char* path)
{
	loadGeometryPathName_ = path;
}

//-------------------------------------------------------------------------------------
void Space::update()
{
	if(!isLoadGeometry_)
		return;
}

//-------------------------------------------------------------------------------------
void Space::addEntity(Entity* pEntity)
{
	pEntity->setSpaceID(this->id_);
	pEntity->spaceEntityIdx(entities_.size());
	entities_.push_back(pEntity);

	onEnterWorld(pEntity);
}

//-------------------------------------------------------------------------------------
void Space::removeEntity(Entity* pEntity)
{
	pEntity->setSpaceID(0);
	
	// 先获取到所在位置
	SPACE_ENTITIES::size_type idx = pEntity->spaceEntityIdx();

	KBE_ASSERT(idx < entities_.size());
	KBE_ASSERT(entities_[ idx ] == pEntity);

	// 如果有2个或以上的entity则将最后一个entity移至删除的这个entity所在位置
	Entity* pBack = entities_.back().get();
	pBack->spaceEntityIdx(idx);
	entities_[idx] = pBack;
	pEntity->spaceEntityIdx(SPACE_ENTITIES::size_type(-1));
	entities_.pop_back();
	
	onLeaveWorld(pEntity);

	// 如果没有entity了则需要销毁space, 因为space最少存在一个entity
	if(entities_.empty())
	{
	}
}

//-------------------------------------------------------------------------------------
void Space::_onEnterWorld(Entity* pEntity)
{
	if(pEntity->hasWitness())
	{
		Mercury::Bundle* pSendBundle = Mercury::Bundle::ObjPool().createObject();
		Mercury::Bundle* pForwardBundle = Mercury::Bundle::ObjPool().createObject();
		Mercury::Bundle* pForwardPosDirBundle = Mercury::Bundle::ObjPool().createObject();
		
		(*pForwardPosDirBundle).newMessage(ClientInterface::onUpdatePropertys);
		MemoryStream* s1 = MemoryStream::ObjPool().createObject();
		(*pForwardPosDirBundle) << pEntity->getID();
		pEntity->addPositionAndDirectionToStream(*s1);
		(*pForwardPosDirBundle).append(*s1);
		MemoryStream::ObjPool().reclaimObject(s1);
		MERCURY_ENTITY_MESSAGE_FORWARD_CLIENT(pEntity->getID(), (*pSendBundle), (*pForwardPosDirBundle));

		(*pForwardBundle).newMessage(ClientInterface::onEntityEnterWorld);
		(*pForwardBundle) << pEntity->getID();
		(*pForwardBundle) << this->getID();

		MERCURY_ENTITY_MESSAGE_FORWARD_CLIENT(pEntity->getID(), (*pSendBundle), (*pForwardBundle));
		pEntity->getClientMailbox()->postMail(*pSendBundle);

		Mercury::Bundle::ObjPool().reclaimObject(pSendBundle);
		Mercury::Bundle::ObjPool().reclaimObject(pForwardBundle);
		Mercury::Bundle::ObjPool().reclaimObject(pForwardPosDirBundle);
	}
}

//-------------------------------------------------------------------------------------
void Space::onEnterWorld(Entity* pEntity)
{
	KBE_ASSERT(pEntity != NULL);
	
	std::vector<Entity*> viewEntitys;
	
	// 如果是一个有Witness(通常是玩家)则需要将当前场景已经创建的有client部分的entity广播给他
	// 否则是一个普通的entity进入世界， 那么需要将这个entity广播给所有看见他的有Witness的entity。
	if(pEntity->hasWitness())
	{
		_onEnterWorld(pEntity);
		if(entities().size() > 0)
		{
			Mercury::Bundle* pSendBundle = Mercury::Bundle::ObjPool().createObject();
			MERCURY_ENTITY_MESSAGE_FORWARD_CLIENT_START(pEntity->getID(), (*pSendBundle));

			SPACE_ENTITIES::iterator iter = entities_.begin();
			for(; iter != entities().end(); iter++)
			{
				Entity* entity = (*iter).get();
				if(entity == pEntity)
					continue;

				if(entity->hasWitness())
				{
					viewEntitys.push_back(entity);
				}
				
				if(!entity->getScriptModule()->hasClient())
					continue;

				Mercury::Bundle* pForwardBundle1 = Mercury::Bundle::ObjPool().createObject();
				Mercury::Bundle* pForwardBundle2 = Mercury::Bundle::ObjPool().createObject();

				MemoryStream* s1 = MemoryStream::ObjPool().createObject();
				entity->addPositionAndDirectionToStream(*s1);
				entity->addClientDataToStream(s1);

				(*pForwardBundle1).newMessage(ClientInterface::onUpdatePropertys);
				(*pForwardBundle1) << entity->getID();
				(*pForwardBundle1).append(*s1);
				MemoryStream::ObjPool().reclaimObject(s1);
		
				(*pForwardBundle2).newMessage(ClientInterface::onEntityEnterWorld);
				(*pForwardBundle2) << entity->getID();
				(*pForwardBundle2) << entity->getSpaceID();

				MERCURY_ENTITY_MESSAGE_FORWARD_CLIENT_APPEND((*pSendBundle), (*pForwardBundle1));
				MERCURY_ENTITY_MESSAGE_FORWARD_CLIENT_APPEND((*pSendBundle), (*pForwardBundle2));
				
				Mercury::Bundle::ObjPool().reclaimObject(pForwardBundle1);
				Mercury::Bundle::ObjPool().reclaimObject(pForwardBundle2);
			}

			Mercury::Bundle::ObjPool().reclaimObject(pSendBundle);
			pEntity->getClientMailbox()->postMail(*pSendBundle);
		}
	}
	else
	{
		if(entities().size() > 0 && pEntity->getScriptModule()->hasClient())
		{
			SPACE_ENTITIES::iterator iter = entities_.begin();
			for(; iter != entities().end(); iter++)
			{
				Entity* entity = (*iter).get();
				if(entity == pEntity)
					continue;

				if(entity->hasWitness())
				{
					viewEntitys.push_back(entity);
				}
			}
		}
	}

	// 向所有的这些entity广播当前进入世界的entity
	broadcastEntityToEntities(pEntity, viewEntitys);
}

//-------------------------------------------------------------------------------------
void Space::broadcastEntityToEntities(Entity* pEntity, std::vector<Entity*>& entities)
{
	if(entities.size())
	{
		MemoryStream* s1 = MemoryStream::ObjPool().createObject();
		pEntity->addPositionAndDirectionToStream(*s1);
		pEntity->addClientDataToStream(s1);

		std::vector<Entity*>::iterator iter = entities.begin();
		for(; iter != entities.end(); iter++)
		{
			Entity* entity = (*iter);

			if(!entity->getScriptModule()->hasClient())
				continue;

			Mercury::Bundle* pSendBundle = Mercury::Bundle::ObjPool().createObject();
			Mercury::Bundle* pForwardBundle1 = Mercury::Bundle::ObjPool().createObject();
			Mercury::Bundle* pForwardBundle2 = Mercury::Bundle::ObjPool().createObject();

			(*pForwardBundle1).newMessage(ClientInterface::onUpdatePropertys);
			(*pForwardBundle1) << pEntity->getID();
			(*pForwardBundle1).append(*s1);

			(*pForwardBundle2).newMessage(ClientInterface::onEntityEnterWorld);
			(*pForwardBundle2) << pEntity->getID();
			(*pForwardBundle2) << pEntity->getSpaceID();

			MERCURY_ENTITY_MESSAGE_FORWARD_CLIENT(entity->getID(), (*pSendBundle), (*pForwardBundle1));
			MERCURY_ENTITY_MESSAGE_FORWARD_CLIENT(entity->getID(), (*pSendBundle), (*pForwardBundle2));
			Mercury::Bundle::ObjPool().reclaimObject(pForwardBundle1);
			Mercury::Bundle::ObjPool().reclaimObject(pForwardBundle2);

			entity->getClientMailbox()->postMail(*pSendBundle);
			Mercury::Bundle::ObjPool().reclaimObject(pSendBundle);
		}

		MemoryStream::ObjPool().reclaimObject(s1);
	}
}

//-------------------------------------------------------------------------------------
void Space::broadcastAOIEntities(Entity* pEntity)
{
	if(pEntity == NULL || !pEntity->hasWitness())
		return;

	if(entities().size() > 0)
	{
		Mercury::Bundle* pSendBundle = Mercury::Bundle::ObjPool().createObject();
		MERCURY_ENTITY_MESSAGE_FORWARD_CLIENT_START(pEntity->getID(), (*pSendBundle));

		SPACE_ENTITIES::iterator iter = entities_.begin();
		for(; iter != entities_.end(); iter++)
		{
			Entity* entity = (*iter).get();
			if(!entity->getScriptModule()->hasClient() || entity == pEntity)
				continue;

			Mercury::Bundle* pForwardBundle1 = Mercury::Bundle::ObjPool().createObject();
			Mercury::Bundle* pForwardBundle2 = Mercury::Bundle::ObjPool().createObject();

			MemoryStream* s1 = MemoryStream::ObjPool().createObject();
			entity->addPositionAndDirectionToStream(*s1);
			entity->addClientDataToStream(s1);
			
			(*pForwardBundle1).newMessage(ClientInterface::onUpdatePropertys);
			(*pForwardBundle1) << entity->getID();
			(*pForwardBundle1).append(*s1);
			MemoryStream::ObjPool().reclaimObject(s1);
	
			(*pForwardBundle2).newMessage(ClientInterface::onEntityEnterWorld);
			(*pForwardBundle2) << entity->getID();
			(*pForwardBundle2) << entity->getSpaceID();

			MERCURY_ENTITY_MESSAGE_FORWARD_CLIENT_APPEND((*pSendBundle), (*pForwardBundle1));
			MERCURY_ENTITY_MESSAGE_FORWARD_CLIENT_APPEND((*pSendBundle), (*pForwardBundle2));
			
			Mercury::Bundle::ObjPool().reclaimObject(pForwardBundle1);
			Mercury::Bundle::ObjPool().reclaimObject(pForwardBundle2);
		}

		Mercury::Bundle::ObjPool().reclaimObject(pSendBundle);
		pEntity->getClientMailbox()->postMail(*pSendBundle);
	}
}

//-------------------------------------------------------------------------------------
void Space::onEntityAttachWitness(Entity* pEntity)
{
	KBE_ASSERT(pEntity != NULL && pEntity->hasWitness());
	_onEnterWorld(pEntity);
	broadcastAOIEntities(pEntity);
}

//-------------------------------------------------------------------------------------
void Space::onLeaveWorld(Entity* pEntity)
{
	if(!pEntity->getScriptModule()->hasClient())
		return;

	if(entities().size() > 0)
	{
		SPACE_ENTITIES::const_iterator iter = entities().begin();
		for(; iter != entities().end(); iter++)
		{
			const Entity* entity = (*iter).get();
			if(!entity->hasWitness() || entity == pEntity)
				continue;

			Mercury::Bundle* pSendBundle = Mercury::Bundle::ObjPool().createObject();
			Mercury::Bundle* pForwardBundle = Mercury::Bundle::ObjPool().createObject();

			(*pForwardBundle).newMessage(ClientInterface::onEntityLeaveWorld);
			(*pForwardBundle) << pEntity->getID();
			(*pForwardBundle) << pEntity->getSpaceID();

			MERCURY_ENTITY_MESSAGE_FORWARD_CLIENT(entity->getID(), (*pSendBundle), (*pForwardBundle));
			entity->getClientMailbox()->postMail(*pSendBundle);
			Mercury::Bundle::ObjPool().reclaimObject(pSendBundle);
			Mercury::Bundle::ObjPool().reclaimObject(pForwardBundle);
		}
	}

	// 向客户端发送onLeaveWorld消息
	if(pEntity->hasWitness())
	{
		Mercury::Bundle* pSendBundle = Mercury::Bundle::ObjPool().createObject();
		Mercury::Bundle* pForwardBundle = Mercury::Bundle::ObjPool().createObject();

		(*pForwardBundle).newMessage(ClientInterface::onEntityLeaveWorld);
		(*pForwardBundle) << pEntity->getID();
		(*pForwardBundle) << this->getID();

		MERCURY_ENTITY_MESSAGE_FORWARD_CLIENT(pEntity->getID(), (*pSendBundle), (*pForwardBundle));
		pEntity->getClientMailbox()->postMail(*pSendBundle);
		Mercury::Bundle::ObjPool().reclaimObject(pSendBundle);
		Mercury::Bundle::ObjPool().reclaimObject(pForwardBundle);
	}
}

//-------------------------------------------------------------------------------------
}
