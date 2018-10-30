// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

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
