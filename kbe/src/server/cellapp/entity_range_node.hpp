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

#ifndef __KBE_ENTITY_RANGE_NODE_HPP__
#define __KBE_ENTITY_RANGE_NODE_HPP__

#include "range_node.hpp"
#include "math/math.hpp"

namespace KBEngine{

class Entity;

class EntityRangeNode : public RangeNode
{
public:
	EntityRangeNode(Entity* pEntity);
	virtual ~EntityRangeNode();

	/**
		(扩展坐标)
		x && z由不同的应用实现(从不同处获取)
	*/
	virtual float xx()const;
	virtual float yy()const;
	virtual float zz()const;

	virtual void update();

	Entity* pEntity()const { return pEntity_; }

	bool addWatcherNode(RangeNode* pNode);
	bool delWatcherNode(RangeNode* pNode);

	static void entitiesInRange(std::vector<Entity*>& findentities, RangeNode* rootNode, 
		const Position3D& orginpos, float radius, int entityUType = -1);

	virtual void onRemove();
protected:
	Entity* pEntity_;

	std::vector<RangeNode*> watcherNodes_;
};

}

#endif
