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

#ifndef KBE_ENTITY_COORDINATE_NODE_H
#define KBE_ENTITY_COORDINATE_NODE_H

#include "coordinate_node.h"
#include "math/math.h"

namespace KBEngine{

class Entity;

class EntityCoordinateNode : public CoordinateNode
{
public:
	EntityCoordinateNode(Entity* pEntity);
	virtual ~EntityCoordinateNode();

	/**
		(扩展坐标)
		x && z由不同的应用实现(从不同处获取)
	*/
	virtual float xx() const;
	virtual float yy() const;
	virtual float zz() const;

	virtual void update();

	Entity* pEntity() const { return pEntity_; }
	void pEntity(Entity* pEntity) { pEntity_ = pEntity; }

	bool addWatcherNode(CoordinateNode* pNode);
	bool delWatcherNode(CoordinateNode* pNode);

	static void entitiesInRange(std::vector<Entity*>& findentities, CoordinateNode* rootNode, 
		const Position3D& orginpos, float radius, int entityUType = -1);

	virtual void onRemove();

protected:
	Entity* pEntity_;

	std::vector<CoordinateNode*> watcherNodes_;
};

}

#endif
