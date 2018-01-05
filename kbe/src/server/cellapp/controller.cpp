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
#include "controller.h"	
#include "controllers.h"	
#include "common/memorystream.h"	

namespace KBEngine{	


//-------------------------------------------------------------------------------------
Controller::Controller(Controller::ControllerType type, Entity* pEntity, int32 userarg, uint32 id):
id_(id),
pEntity_(pEntity),
userarg_(userarg),
pControllers_(0),
type_(type)
{
}

//-------------------------------------------------------------------------------------
Controller::Controller(Entity* pEntity):
id_(0),
pEntity_(pEntity),
userarg_(0),
pControllers_(0),
type_(CONTROLLER_TYPE_NORMAL)
{
}

//-------------------------------------------------------------------------------------
Controller::~Controller()
{
	id_ = 0;
	pControllers_ = NULL;
}

//-------------------------------------------------------------------------------------
void Controller::destroy()
{
	if(pControllers_ && !pControllers_->remove(this->id_))
	{
		ERROR_MSG(fmt::format("Controller::destroy(): not found {}.\n",
			id_));
	}

	pControllers_ = NULL;
}

//-------------------------------------------------------------------------------------
void Controller::addToStream(KBEngine::MemoryStream& s)
{
	s << id_ << userarg_;
}

//-------------------------------------------------------------------------------------
void Controller::createFromStream(KBEngine::MemoryStream& s)
{
	s >> id_ >> userarg_;
}

//-------------------------------------------------------------------------------------
}
