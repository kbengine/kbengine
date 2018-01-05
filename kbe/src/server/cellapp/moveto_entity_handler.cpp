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

#include "cellapp.h"
#include "entity.h"
#include "moveto_entity_handler.h"	

namespace KBEngine{	


//-------------------------------------------------------------------------------------
MoveToEntityHandler::MoveToEntityHandler(KBEShared_ptr<Controller>& pController, ENTITY_ID pTargetID, float velocity, float range, bool faceMovement, 
		bool moveVertically, PyObject* userarg):
MoveToPointHandler(pController, pController->pEntity()->layer(), pController->pEntity()->position(), velocity, range, faceMovement, moveVertically, userarg),
pTargetID_(pTargetID)
{
	updatableName = "MoveToEntityHandler";
}

//-------------------------------------------------------------------------------------
MoveToEntityHandler::MoveToEntityHandler():
MoveToPointHandler(),
pTargetID_(0)
{
	updatableName = "MoveToEntityHandler";
}

//-------------------------------------------------------------------------------------
MoveToEntityHandler::~MoveToEntityHandler()
{
}

//-------------------------------------------------------------------------------------
void MoveToEntityHandler::addToStream(KBEngine::MemoryStream& s)
{
	MoveToPointHandler::addToStream(s);
	s << pTargetID_;
}

//-------------------------------------------------------------------------------------
void MoveToEntityHandler::createFromStream(KBEngine::MemoryStream& s)
{
	MoveToPointHandler::createFromStream(s);
	s >> pTargetID_;
}

//-------------------------------------------------------------------------------------
const Position3D& MoveToEntityHandler::destPos()
{
	Entity* pEntity = Cellapp::getSingleton().findEntity(pTargetID_);
	return pEntity->position();
}

//-------------------------------------------------------------------------------------
bool MoveToEntityHandler::update()
{
	if (isDestroyed_)
	{
		delete this;
		return false;
	}

	Entity* pEntity = Cellapp::getSingleton().findEntity(pTargetID_);
	if(pEntity == NULL)
	{
		if(pController_ && pController_->pEntity())
			pController_->pEntity()->onMoveFailure(pController_->id(), pyuserarg_);
		
		if(pController_)
			pController_->destroy();
		
		pController_.reset();
	}

	return MoveToPointHandler::update();
}

//-------------------------------------------------------------------------------------
}

