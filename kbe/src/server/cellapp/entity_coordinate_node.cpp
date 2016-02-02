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

#include <iterator>
#include "entity_coordinate_node.h"
#include "entity.h"
#include "coordinate_system.h"

namespace KBEngine{	


//-------------------------------------------------------------------------------------
EntityCoordinateNode::EntityCoordinateNode(Entity* pEntity):
CoordinateNode(NULL),
pEntity_(pEntity),
watcherNodes_()
{
	flags(COORDINATE_NODE_FLAG_ENTITY);

#ifdef _DEBUG
	descr_ = (fmt::format("EntityCoordinateNode({}_{})", pEntity->scriptName(), pEntity->id()));
#endif
}

//-------------------------------------------------------------------------------------
EntityCoordinateNode::~EntityCoordinateNode()
{
	watcherNodes_.clear();
}

//-------------------------------------------------------------------------------------
float EntityCoordinateNode::xx() const
{
	if(pEntity_ == NULL || (flags() & (COORDINATE_NODE_FLAG_REMOVED | COORDINATE_NODE_FLAG_REMOVEING)) > 0)
		return -FLT_MAX;

	return pEntity_->position().x;
}

//-------------------------------------------------------------------------------------
float EntityCoordinateNode::yy() const
{
	if(pEntity_ == NULL)
		return -FLT_MAX;

	return pEntity_->position().y;
}

//-------------------------------------------------------------------------------------
float EntityCoordinateNode::zz() const
{
	if(pEntity_ == NULL)
		return -FLT_MAX;

	return pEntity_->position().z;
}

//-------------------------------------------------------------------------------------
void EntityCoordinateNode::update()
{
	CoordinateNode::update();
	std::vector<CoordinateNode*>::iterator iter = watcherNodes_.begin();
	for(; iter != watcherNodes_.end(); ++iter)
	{
		(*iter)->update();
	}
}

//-------------------------------------------------------------------------------------
void EntityCoordinateNode::onRemove()
{
	std::vector<CoordinateNode*>::iterator iter = watcherNodes_.begin();
	for(; iter != watcherNodes_.end(); ++iter)
	{
		(*iter)->onParentRemove(this);
	}

	CoordinateNode::onRemove();
}

//-------------------------------------------------------------------------------------
bool EntityCoordinateNode::addWatcherNode(CoordinateNode* pNode)
{
	std::vector<CoordinateNode*>::iterator iter = std::find(watcherNodes_.begin(), watcherNodes_.end(), pNode);
	if(iter != watcherNodes_.end())
		return false;

	watcherNodes_.push_back(pNode);
	return true;
}

//-------------------------------------------------------------------------------------
bool EntityCoordinateNode::delWatcherNode(CoordinateNode* pNode)
{
	std::vector<CoordinateNode*>::iterator iter = std::find(watcherNodes_.begin(), watcherNodes_.end(), pNode);
	if(iter == watcherNodes_.end())
		return false;

	watcherNodes_.erase(iter);
	return true;
}

//-------------------------------------------------------------------------------------
void EntityCoordinateNode::entitiesInRange(std::vector<Entity*>& foundEntities, CoordinateNode* rootNode,
									  const Position3D& originPos, float radius, int entityUType)
{
	std::set<Entity*> entities_X;
	std::set<Entity*> entities_Z;

	if((rootNode->flags() & COORDINATE_NODE_FLAG_ENTITY) > 0)
	{
		Entity* pEntity = static_cast<EntityCoordinateNode*>(rootNode)->pEntity();
		if(entityUType == -1 || pEntity->pScriptModule()->getUType() == (ENTITY_SCRIPT_UID)entityUType)
		{
			Position3D distVec = originPos - pEntity->position();
			float dist = KBEVec3Length(&distVec);

			if(dist <= radius)
			{
				foundEntities.push_back(pEntity);
			}
		}
	}

	CoordinateNode* pCoordinateNode = rootNode;

	while(pCoordinateNode->pPrevX())
	{
		CoordinateNode* pPrevCoordinateNode = pCoordinateNode->pPrevX();
		if((pPrevCoordinateNode->flags() & COORDINATE_NODE_FLAG_ENTITY) > 0)
		{
			Entity* pEntity = static_cast<EntityCoordinateNode*>(pPrevCoordinateNode)->pEntity();
			
			if(entityUType == -1 || pEntity->pScriptModule()->getUType() == (ENTITY_SCRIPT_UID)entityUType)
			{
				if (fabs(originPos.x - pEntity->position().x) <= radius)
				{
					entities_X.insert(pEntity);
				}
				else
				{
					break;
				}
			}
		}

		pCoordinateNode = pPrevCoordinateNode;
	};

	pCoordinateNode = rootNode;
	
	while(pCoordinateNode->pNextX())
	{
		CoordinateNode* pNextCoordinateNode = pCoordinateNode->pNextX();
		if((pNextCoordinateNode->flags() & COORDINATE_NODE_FLAG_ENTITY) > 0)
		{
			Entity* pEntity = static_cast<EntityCoordinateNode*>(pNextCoordinateNode)->pEntity();
			
			if(entityUType == -1 || pEntity->pScriptModule()->getUType() == (ENTITY_SCRIPT_UID)entityUType)
			{
				if (fabs(originPos.x - pEntity->position().x) <= radius)
				{
					entities_X.insert(pEntity);
				}
				else
				{
					break;
				}
			}
		}

		pCoordinateNode = pNextCoordinateNode;
	};

	// ²éÕÒZÖá
	pCoordinateNode = rootNode;

	while (pCoordinateNode->pPrevZ())
	{
		CoordinateNode* pPrevCoordinateNode = pCoordinateNode->pPrevZ();
		if ((pPrevCoordinateNode->flags() & COORDINATE_NODE_FLAG_ENTITY) > 0)
		{
			Entity* pEntity = static_cast<EntityCoordinateNode*>(pPrevCoordinateNode)->pEntity();

			if (entityUType == -1 || pEntity->pScriptModule()->getUType() == (ENTITY_SCRIPT_UID)entityUType)
			{
				if (fabs(originPos.z - pEntity->position().z) <= radius)
				{
					entities_Z.insert(pEntity);
				}
				else
				{
					break;
				}
			}
		}

		pCoordinateNode = pPrevCoordinateNode;
	};

	pCoordinateNode = rootNode;

	while (pCoordinateNode->pNextZ())
	{
		CoordinateNode* pNextCoordinateNode = pCoordinateNode->pNextZ();
		if ((pNextCoordinateNode->flags() & COORDINATE_NODE_FLAG_ENTITY) > 0)
		{
			Entity* pEntity = static_cast<EntityCoordinateNode*>(pNextCoordinateNode)->pEntity();

			if (entityUType == -1 || pEntity->pScriptModule()->getUType() == (ENTITY_SCRIPT_UID)entityUType)
			{
				if (fabs(originPos.z - pEntity->position().z) <= radius)
				{
					entities_Z.insert(pEntity);
				}
				else
				{
					break;
				}
			}
		}

		pCoordinateNode = pNextCoordinateNode;
	};

	// ²éÕÒY
	if (CoordinateSystem::hasY)
	{
		pCoordinateNode = rootNode;
		std::set<Entity*> entities_Y;

		while (pCoordinateNode->pPrevY())
		{
			CoordinateNode* pPrevCoordinateNode = pCoordinateNode->pPrevY();
			if ((pPrevCoordinateNode->flags() & COORDINATE_NODE_FLAG_ENTITY) > 0)
			{
				Entity* pEntity = static_cast<EntityCoordinateNode*>(pPrevCoordinateNode)->pEntity();

				if (entityUType == -1 || pEntity->pScriptModule()->getUType() == (ENTITY_SCRIPT_UID)entityUType)
				{
					if (fabs(originPos.y - pEntity->position().y) <= radius)
					{
						entities_Y.insert(pEntity);
					}
					else
					{
						break;
					}
				}
			}

			pCoordinateNode = pPrevCoordinateNode;
		};

		pCoordinateNode = rootNode;

		while (pCoordinateNode->pNextY())
		{
			CoordinateNode* pNextCoordinateNode = pCoordinateNode->pNextY();
			if ((pNextCoordinateNode->flags() & COORDINATE_NODE_FLAG_ENTITY) > 0)
			{
				Entity* pEntity = static_cast<EntityCoordinateNode*>(pNextCoordinateNode)->pEntity();

				if (entityUType == -1 || pEntity->pScriptModule()->getUType() == (ENTITY_SCRIPT_UID)entityUType)
				{
					if (fabs(originPos.y - pEntity->position().y) <= radius)
					{
						entities_Y.insert(pEntity);
					}
					else
					{
						break;
					}
				}
			}

			pCoordinateNode = pNextCoordinateNode;
		};

		std::set<Entity*> res_set;
		set_intersection(entities_X.begin(), entities_X.end(), entities_Z.begin(), entities_Z.end(), std::inserter(res_set, res_set.end()));
		set_intersection(res_set.begin(), res_set.end(), entities_Y.begin(), entities_Y.end(), std::back_inserter(foundEntities));
	}
	else
	{
		set_intersection(entities_X.begin(), entities_X.end(), entities_Z.begin(), entities_Z.end(), std::back_inserter(foundEntities));
	}
}

//-------------------------------------------------------------------------------------
}
