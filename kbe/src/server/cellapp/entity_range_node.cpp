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
pEntity_(pEntity)
{
	flags(RANGENODE_FLAG_ENTITY);

#ifdef _DEBUG
	descr_ = (boost::format("EntityRangeNode(%1%_%2%)") % pEntity->getScriptName() % pEntity->getID()).str();
#endif
}

//-------------------------------------------------------------------------------------
EntityRangeNode::~EntityRangeNode()
{
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
}
