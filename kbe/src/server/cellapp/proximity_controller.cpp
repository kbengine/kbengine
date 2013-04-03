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
#include "proximity_controller.hpp"	
#include "entity_range_node.hpp"

namespace KBEngine{	


//-------------------------------------------------------------------------------------
ProximityController::ProximityController(Entity* pEntity, float xz, float y, uint32 userarg, uint32 id):
Controller(pEntity, userarg, id),
pTrapTrigger_(NULL)
{
	pTrapTrigger_ = new TrapTrigger(static_cast<EntityRangeNode*>(pEntity->pEntityRangeNode()), 
								this, xz, y);

	pTrapTrigger_->install();
}

//-------------------------------------------------------------------------------------
ProximityController::~ProximityController()
{
	delete pTrapTrigger_;
}

//-------------------------------------------------------------------------------------
void ProximityController::onEnter(Entity* pEntity, float xz, float y)
{
	pEntity_->onEnterTrap(pEntity, xz, y, id(), userarg());
}

//-------------------------------------------------------------------------------------
void ProximityController::onLeave(Entity* pEntity, float xz, float y)
{
	pEntity_->onLeaveTrap(pEntity, xz, y, id(), userarg());
}

//-------------------------------------------------------------------------------------
}
