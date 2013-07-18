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

#ifndef __SPACE_HPP__
#define __SPACE_HPP__

#include "range_list.hpp"
#include "cell.hpp"
#include "helper/debug_helper.hpp"
#include "cstdkbe/cstdkbe.hpp"
#include "cstdkbe/smartpointer.hpp"
#include "pyscript/scriptobject.hpp"

namespace KBEngine{

class Entity;
typedef SmartPointer<Entity> EntityPtr;
typedef std::vector<EntityPtr> SPACE_ENTITIES;

class Space
{
public:
	Space(SPACE_ID spaceID);
	~Space();

	void unLoadSpaceGeometry();
	void loadSpaceGeometry();

	void creatorID(ENTITY_ID id){ creatorID_ = id; }
	ENTITY_ID creatorID()const { return creatorID_; }

	/** 
		更新space中的内容 
	*/
	void update();

	void addEntity(Entity* pEntity);
	void addEntityToNode(Entity* pEntity);

	void addEntityAndEnterWorld(Entity* pEntity, bool isRestore = false);
	void removeEntity(Entity* pEntity);

	/**
		一个entity进入了游戏世界
	*/
	void onEnterWorld(Entity* pEntity);
	void _onEnterWorld(Entity* pEntity);
	void onLeaveWorld(Entity* pEntity);

	void onEntityAttachWitness(Entity* pEntity);

	SPACE_ID getID()const{ return id_; }

	const SPACE_ENTITIES& entities()const{ return entities_; }

	/**
		销毁
	*/
	bool destroy(ENTITY_ID entityID);

	/**
		这个space的cell
	*/
	Cell * pCell() const	{ return pCell_; }
	void pCell( Cell * pCell );

	/**
		添加space的几何映射
	*/
	static PyObject* __py_AddSpaceGeometryMapping(PyObject* self, PyObject* args);
	bool addSpaceGeometryMapping(std::string respath, bool shouldLoadOnServer);
	static PyObject* __py_GetSpaceGeometryMapping(PyObject* self, PyObject* args);
	const std::string& getGeometryPath(){ return loadGeometryPath_; }
	void onLoadedSpaceGeometryMapping();
	void onAllSpaceGeometryLoaded();

protected:
	void _addSpaceGeometryMappingToEntityClient(const Entity* pEntity);

protected:
	// 这个space的ID
	SPACE_ID id_;	

	// 创造者ID 一般来说就是spaceEntity的ID
	ENTITY_ID creatorID_;								

	// 这个space上的entity
	SPACE_ENTITIES entities_;							

	// 是否加载过地形数据
	bool isLoadedGeometry_;

	// 加载几何的路径
	std::string loadGeometryPath_;					
	
	// 每个space最多只有一个cell
	Cell* pCell_;

	RangeList rangeList_;
};


}
#endif
