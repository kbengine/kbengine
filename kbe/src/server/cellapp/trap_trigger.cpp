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

#include "trap_trigger.hpp"
#include "entity.hpp"
#include "entity_range_node.hpp"
#include "proximity_controller.hpp"	

#ifndef CODE_INLINE
#include "trap_trigger.ipp"
#endif

namespace KBEngine{	


//-------------------------------------------------------------------------------------
TrapTrigger::TrapTrigger(RangeNode* origin, ProximityController* pProximityController, float xz, float y):
RangeTrigger(origin, xz, y),
pProximityController_(pProximityController)
{
}

//-------------------------------------------------------------------------------------
TrapTrigger::~TrapTrigger()
{
}

//-------------------------------------------------------------------------------------
void TrapTrigger::onEnter(RangeNode * pNode)
{
	if((pNode->flags() & RANGENODE_FLAG_ENTITY) <= 0)
		return;

	pProximityController_->onEnter(static_cast<EntityRangeNode*>(pNode)->pEntity(), range_xz_, range_y_);
}

//-------------------------------------------------------------------------------------
void TrapTrigger::onLeave(RangeNode * pNode)
{
	if((pNode->flags() & RANGENODE_FLAG_ENTITY) <= 0)
		return;

	pProximityController_->onLeave(static_cast<EntityRangeNode*>(pNode)->pEntity(), range_xz_, range_y_);
}

//-------------------------------------------------------------------------------------
}
