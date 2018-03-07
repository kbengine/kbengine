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
#include "turn_controller.h"		

namespace KBEngine{	


//-------------------------------------------------------------------------------------
TurnController::TurnController(Entity* pEntity, RotatorHandler* pRotatorHandler, uint32 id) :
Controller(Controller::CONTROLLER_TYPE_ROTATE, pEntity, 0, id),
pRotatorHandler_(pRotatorHandler)
{
}

//-------------------------------------------------------------------------------------
TurnController::~TurnController()
{
	// DEBUG_MSG(fmt::format("TurnController::~TurnController(): {:p}\n", (void*)this);
	if (pRotatorHandler_)
		pRotatorHandler_->pController(KBEShared_ptr<Controller>());

	pRotatorHandler_ = NULL;
}

//-------------------------------------------------------------------------------------
void TurnController::addToStream(KBEngine::MemoryStream& s)
{
	Controller::addToStream(s);
	pRotatorHandler_->addToStream(s);
}

//-------------------------------------------------------------------------------------
void TurnController::createFromStream(KBEngine::MemoryStream& s)
{
	Controller::createFromStream(s);
	KBE_ASSERT(pRotatorHandler_ == NULL);
	
	pRotatorHandler_ = new RotatorHandler();
	pRotatorHandler_->createFromStream(s);
}

//-------------------------------------------------------------------------------------
void TurnController::destroy()
{
	Controller::destroy();

	// 既然自己要销毁了，那么与自己相联的updatable也应该停止了
	if (pRotatorHandler_)
	{
		pRotatorHandler_->pController(KBEShared_ptr<Controller>());
		pRotatorHandler_ = NULL;
	}
}

//-------------------------------------------------------------------------------------
}

