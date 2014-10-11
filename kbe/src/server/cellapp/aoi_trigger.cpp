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
#include "witness.hpp"
#include "entity.hpp"
#include "aoi_trigger.hpp"
#include "entity_coordinate_node.hpp"

namespace KBEngine{	


//-------------------------------------------------------------------------------------
AOITrigger::AOITrigger(CoordinateNode* origin, float xz, float y):
RangeTrigger(origin, xz, y),
pWitness_(static_cast<EntityCoordinateNode*>(origin)->pEntity()->pWitness())
{
}

//-------------------------------------------------------------------------------------
AOITrigger::~AOITrigger()
{
}

//-------------------------------------------------------------------------------------
void AOITrigger::onEnter(CoordinateNode * pNode)
{
	if((pNode->flags() & COORDINATE_NODE_FLAG_ENTITY) <= 0)
		return;

	EntityCoordinateNode* pEntityCoordinateNode = static_cast<EntityCoordinateNode*>(pNode);
	Entity* pEntity = pEntityCoordinateNode->pEntity();
	if(!pEntity->scriptModule()->hasClient())
		return;

	pWitness_->onEnterAOI(pEntity);
}

//-------------------------------------------------------------------------------------
void AOITrigger::onLeave(CoordinateNode * pNode)
{
	if((pNode->flags() & COORDINATE_NODE_FLAG_ENTITY) <= 0)
		return;

	EntityCoordinateNode* pEntityCoordinateNode = static_cast<EntityCoordinateNode*>(pNode);
	Entity* pEntity = pEntityCoordinateNode->pEntity();
	if(!pEntity->scriptModule()->hasClient())
		return;

	pWitness_->onLeaveAOI(pEntity);
}

//-------------------------------------------------------------------------------------
}
