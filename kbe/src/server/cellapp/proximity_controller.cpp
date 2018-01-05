/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2018 KBEngine.

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

#include "trap_trigger.h"
#include "entity.h"
#include "proximity_controller.h"	
#include "entity_coordinate_node.h"

namespace KBEngine{	


//-------------------------------------------------------------------------------------
ProximityController::ProximityController(Entity* pEntity, float xz, float y, int32 userarg, uint32 id):
Controller(CONTROLLER_TYPE_PROXIMITY, pEntity, userarg, id),
pTrapTrigger_(NULL),
xz_(xz),
y_(y)
{
	pTrapTrigger_ = new TrapTrigger(static_cast<EntityCoordinateNode*>(pEntity->pEntityCoordinateNode()), 
								this, xz, y);

	pTrapTrigger_->install();
}

//-------------------------------------------------------------------------------------
ProximityController::ProximityController(Entity* pEntity):
Controller(pEntity),
pTrapTrigger_(NULL),
xz_(0.f),
y_(0.f)
{
}

//-------------------------------------------------------------------------------------
ProximityController::~ProximityController()
{
	pTrapTrigger_->uninstall();
	delete pTrapTrigger_;
}

//-------------------------------------------------------------------------------------
void ProximityController::addToStream(KBEngine::MemoryStream& s)
{
	Controller::addToStream(s);
	s << xz_ << y_;
}

//-------------------------------------------------------------------------------------
void ProximityController::createFromStream(KBEngine::MemoryStream& s)
{
	Controller::createFromStream(s);
	s >> xz_ >> y_;
}

//-------------------------------------------------------------------------------------
bool ProximityController::reinstall(CoordinateNode* pCoordinateNode)
{
	// 跨cellapp场景跳转时可能出现这种情况
	// 因为使用ProximityController::ProximityController(Entity* pEntity)构造
	if(pTrapTrigger_ == NULL)
	{
		pTrapTrigger_ = new TrapTrigger(static_cast<EntityCoordinateNode*>(pCoordinateNode), 
								this, xz_, y_);
	}

	return pTrapTrigger_->reinstall(pCoordinateNode);
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
