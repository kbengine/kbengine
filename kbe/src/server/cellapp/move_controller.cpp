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
#include "move_controller.hpp"	
#include "moveto_point_handler.hpp"	
#include "moveto_entity_handler.hpp"	
#include "navigate_handler.hpp"	

namespace KBEngine{	


//-------------------------------------------------------------------------------------
MoveController::MoveController(Entity* pEntity, MoveToPointHandler* pMoveToPointHandler, uint32 id):
Controller(Controller::CONTROLLER_TYPE_MOVE, pEntity, 0, id),
pMoveToPointHandler_(pMoveToPointHandler)
{
}

//-------------------------------------------------------------------------------------
MoveController::~MoveController()
{
	// DEBUG_MSG(fmt::format("MoveController::~MoveController(): {:p}\n", (void*)this);
	pMoveToPointHandler_->pController(NULL);
	pMoveToPointHandler_ = NULL;
}

//-------------------------------------------------------------------------------------
void MoveController::addToStream(KBEngine::MemoryStream& s)
{
	Controller::addToStream(s);

	uint8 utype = pMoveToPointHandler_->type();
	s << utype;

	pMoveToPointHandler_->addToStream(s);
}

//-------------------------------------------------------------------------------------
void MoveController::createFromStream(KBEngine::MemoryStream& s)
{
	Controller::createFromStream(s);
	KBE_ASSERT(pMoveToPointHandler_ == NULL);

	uint8 utype;
	s >> utype;

	if(utype == MoveToPointHandler::MOVE_TYPE_NAV)
		pMoveToPointHandler_ = new NavigateHandler();
	else if(utype == MoveToPointHandler::MOVE_TYPE_ENTITY)
		pMoveToPointHandler_ = new MoveToEntityHandler();
	else if(utype == MoveToPointHandler::MOVE_TYPE_POINT)
		pMoveToPointHandler_ = new MoveToPointHandler();
	else
		KBE_ASSERT(false);

	pMoveToPointHandler_->createFromStream(s);
}

//-------------------------------------------------------------------------------------
}

