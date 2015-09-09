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

#include "entity_coordinate_node.h"
#include "entity.h"

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

	// 通过从后往前遍历并辅以每一次都对watcherNodes_的大小进行检查的方式，
	// 从而有效的避开因内部循环删除所引发的迭代器无法迭代的问题
	int i = watcherNodes_.size() - 1;
	
	// 存储已经遍历过的节点，以避免重复处理
	// @todo(phw): 如果有需要（例如预期数据量会很大），換成hash_set或set效率会更高
	std::vector<CoordinateNode*> processeds;
	
	for(; i >= 0; --i)
	{
		// 如果索引比拥有的数量大，那表示一次性删除了多个
		// 但需要继续推进，直至遍历了所有的节点为止
		if (i >= (int)watcherNodes_.size())
			continue;
		
		CoordinateNode* pNode = watcherNodes_[i];
		
		// 如果已经处理过，就不再次处理了
		if (processeds.end() != std::find(processeds.begin(), processeds.end(), pNode))
			continue;

		pNode->update();
		processeds.push_back(pNode);
	}
}

//-------------------------------------------------------------------------------------
void EntityCoordinateNode::onRemove()
{
	// 通过从后往前遍历并辅以每一次都对watcherNodes_的大小进行检查的方式，
	// 从而有效的避开因内部循环删除所引发的迭代器无法迭代的问题
	int i = watcherNodes_.size() - 1;
	
	// 存储已经遍历过的节点，以避免重复处理
	// @todo(phw): 如果有需要（例如预期数据量会很大），換成hash_set或set效率会更高
	std::vector<CoordinateNode*> processeds;
	
	for(; i >= 0; --i)
	{
		// 如果索引比拥有的数量大，那表示一次性删除了多个
		// 但需要继续推进，直至遍历了所有的节点为止
		if (i >= (int)watcherNodes_.size())
			continue;
		
		CoordinateNode* pNode = watcherNodes_[i];
		
		// 如果已经处理过，就不再次处理了
		if (processeds.end() != std::find(processeds.begin(), processeds.end(), pNode))
			continue;

		pNode->onParentRemove(this);
		processeds.push_back(pNode);
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
void EntityCoordinateNode::entitiesInRange(std::vector<Entity*>& findentities, CoordinateNode* rootNode, 
									  const Position3D& originpos, float radius, int entityUType)
{
	if((rootNode->flags() & COORDINATE_NODE_FLAG_ENTITY) > 0)
	{
		Entity* pEntity = static_cast<EntityCoordinateNode*>(rootNode)->pEntity();
		if(entityUType == -1 || pEntity->scriptModule()->getUType() == (ENTITY_SCRIPT_UID)entityUType)
		{
			Position3D distVec = originpos - pEntity->position();
			float dist = KBEVec3Length(&distVec);

			if(dist <= radius)
			{
				findentities.push_back(pEntity);
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
			
			if(entityUType == -1 || pEntity->scriptModule()->getUType() == (ENTITY_SCRIPT_UID)entityUType)
			{
				Position3D distVec = originpos - pEntity->position();
				float dist = KBEVec3Length(&distVec);

				if(dist <= radius)
				{
					findentities.push_back(pEntity);
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
			
			if(entityUType == -1 || pEntity->scriptModule()->getUType() == (ENTITY_SCRIPT_UID)entityUType)
			{
				Position3D distVec = originpos - pEntity->position();
				float dist = KBEVec3Length(&distVec);

				if(dist <= radius)
				{
					findentities.push_back(pEntity);
				}
			}
		}

		pCoordinateNode = pNextCoordinateNode;
	};
}

//-------------------------------------------------------------------------------------
}
