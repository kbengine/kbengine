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

#include "entity_range_node.hpp"
#include "entity.hpp"

namespace KBEngine{	


//-------------------------------------------------------------------------------------
EntityRangeNode::EntityRangeNode(Entity* pEntity):
RangeNode(NULL),
pEntity_(pEntity),
watcherNodes_()
{
	flags(RANGENODE_FLAG_ENTITY);

#ifdef _DEBUG
	descr_ = (boost::format("EntityRangeNode(%1%_%2%)") % pEntity->getScriptName() % pEntity->getID()).str();
#endif
}

//-------------------------------------------------------------------------------------
EntityRangeNode::~EntityRangeNode()
{
	watcherNodes_.clear();
}

//-------------------------------------------------------------------------------------
float EntityRangeNode::x()const
{
	if((flags() & RANGENODE_FLAG_REMOVE) > 0)
		return -FLT_MAX;

	return pEntity_->getPosition().x;
}

//-------------------------------------------------------------------------------------
float EntityRangeNode::y()const
{
	return pEntity_->getPosition().y;
}

//-------------------------------------------------------------------------------------
float EntityRangeNode::z()const
{
	return pEntity_->getPosition().z;
}

//-------------------------------------------------------------------------------------
void EntityRangeNode::update()
{
	RangeNode::update();
	std::vector<RangeNode*>::iterator iter = watcherNodes_.begin();
	for(; iter != watcherNodes_.end(); iter++)
	{
		(*iter)->update();
	}
}

//-------------------------------------------------------------------------------------
void EntityRangeNode::onRemove()
{
	std::vector<RangeNode*>::iterator iter = watcherNodes_.begin();
	for(; iter != watcherNodes_.end(); iter++)
	{
		(*iter)->onParentRemove(this);
	}

	RangeNode::onRemove();
}

//-------------------------------------------------------------------------------------
bool EntityRangeNode::addWatcherNode(RangeNode* pNode)
{
	std::vector<RangeNode*>::iterator iter = std::find(watcherNodes_.begin(), watcherNodes_.end(), pNode);
	if(iter != watcherNodes_.end())
		return false;

	watcherNodes_.push_back(pNode);
	return true;
}

//-------------------------------------------------------------------------------------
bool EntityRangeNode::delWatcherNode(RangeNode* pNode)
{
	std::vector<RangeNode*>::iterator iter = std::find(watcherNodes_.begin(), watcherNodes_.end(), pNode);
	if(iter == watcherNodes_.end())
		return false;

	watcherNodes_.erase(iter);
	return true;
}

//-------------------------------------------------------------------------------------
void EntityRangeNode::entitiesInRange(std::vector<Entity*>& findentities, RangeNode* rootNode, 
									  const Position3D& orginpos, float radius, int entityUType)
{
	if((rootNode->flags() & RANGENODE_FLAG_ENTITY) > 0)
	{
		Entity* pEntity = static_cast<EntityRangeNode*>(rootNode)->pEntity();
		if(entityUType == -1 || pEntity->getScriptModule()->getUType() == (ENTITY_SCRIPT_UID)entityUType)
		{
			Position3D distVec = orginpos - pEntity->getPosition();
			float dist = KBEVec3Length(&distVec);

			if(dist <= radius)
			{
				findentities.push_back(pEntity);
			}
		}
	}

	RangeNode* pRangeNode = rootNode;
	while(pRangeNode->pPrevX())
	{
		RangeNode* pPrevRangeNode = pRangeNode->pPrevX();
		if((pPrevRangeNode->flags() & RANGENODE_FLAG_ENTITY) > 0)
		{
			Entity* pEntity = static_cast<EntityRangeNode*>(pPrevRangeNode)->pEntity();
			
			if(entityUType == -1 || pEntity->getScriptModule()->getUType() == (ENTITY_SCRIPT_UID)entityUType)
			{
				Position3D distVec = orginpos - pEntity->getPosition();
				float dist = KBEVec3Length(&distVec);

				if(dist <= radius)
				{
					findentities.push_back(pEntity);
				}
			}
		}

		pRangeNode = pPrevRangeNode;
	};

	pRangeNode = rootNode;
	
	while(pRangeNode->pNextX())
	{
		RangeNode* pNextRangeNode = pRangeNode->pNextX();
		if((pNextRangeNode->flags() & RANGENODE_FLAG_ENTITY) > 0)
		{
			Entity* pEntity = static_cast<EntityRangeNode*>(pNextRangeNode)->pEntity();
			
			if(entityUType == -1 || pEntity->getScriptModule()->getUType() == (ENTITY_SCRIPT_UID)entityUType)
			{
				Position3D distVec = orginpos - pEntity->getPosition();
				float dist = KBEVec3Length(&distVec);

				if(dist <= radius)
				{
					findentities.push_back(pEntity);
				}
			}
		}

		pRangeNode = pNextRangeNode;
	};
}

//-------------------------------------------------------------------------------------
}
