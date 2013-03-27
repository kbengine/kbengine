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

namespace KBEngine{

class Entity;

class EntityRangeNode : public RangeNode
{
public:
	EntityRangeNode(Entity* pEntity);
	virtual ~EntityRangeNode();

	/**
		x && z由不同的应用实现(从不同处获取)
	*/
	virtual float x()const;
	virtual float y()const;
	virtual float z()const;

	Entity* pEntity()const { return pEntity_; }
protected:
	Entity* pEntity_;
};

}

#endif
