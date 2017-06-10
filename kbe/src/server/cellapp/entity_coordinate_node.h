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
		(��չ����)
		x && z�ɲ�ͬ��Ӧ��ʵ��(�Ӳ�ͬ����ȡ)
	*/
	virtual float xx() const;
	virtual float yy() const;
	virtual float zz() const;

	virtual void update();

	Entity* pEntity() const { return pEntity_; }
	void pEntity(Entity* pEntity) { pEntity_ = pEntity; }

	bool addWatcherNode(CoordinateNode* pNode);
	void onAddWatcherNode(CoordinateNode* pNode);
	
	bool delWatcherNode(CoordinateNode* pNode);

	static void entitiesInRange(std::vector<Entity*>& foundEntities, CoordinateNode* rootNode, 
		const Position3D& orginPos, float radius, int entityUType = -1);

	virtual void onRemove();

protected:
	void clearDelWatcherNodes();

protected:
	Entity* pEntity_;

	typedef std::vector<CoordinateNode*> WATCHER_NODES;
	WATCHER_NODES watcherNodes_;
	int delWatcherNodeNum_;

	int entityNodeUpdating_;
};

}

#endif
