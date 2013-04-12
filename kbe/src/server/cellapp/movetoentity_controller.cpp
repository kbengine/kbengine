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

#include "cellapp.hpp"
#include "entity.hpp"
#include "movetoentity_controller.hpp"	

namespace KBEngine{	


//-------------------------------------------------------------------------------------
MoveToEntityController::MoveToEntityController(Entity* pEntity, ENTITY_ID pTargetID, float velocity, bool faceMovement, 
		bool moveVertically, PyObject* userarg, uint32 id):
MoveToPointController(pEntity, pEntity->getPosition(), velocity, faceMovement, moveVertically, userarg, id),
pTargetID_(pTargetID)
{
}

//-------------------------------------------------------------------------------------
MoveToEntityController::~MoveToEntityController()
{
}

//-------------------------------------------------------------------------------------
const Position3D& MoveToEntityController::destPos()
{
	Entity* pEntity = Cellapp::getSingleton().findEntity(pTargetID_);
	return pEntity->getPosition();
}

//-------------------------------------------------------------------------------------
bool MoveToEntityController::update()
{
	Entity* pEntity = Cellapp::getSingleton().findEntity(pTargetID_);
	if(pEntity == NULL)
	{
		pEntity_->onMoveFailure(id(), pyuserarg_);

		delete this;
		return false;
	}

	return MoveToPointController::update();
}

//-------------------------------------------------------------------------------------
}

