/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2016 KBEngine.

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

#include "controllers.h"	
#include "cellapp.h"	
#include "entity.h"	
#include "helper/profile.h"	
#include "common/memorystream.h"	

#include "proximity_controller.h"	
#include "moveto_point_handler.h"	
#include "moveto_entity_handler.h"	
#include "navigate_handler.h"	

namespace KBEngine{	


//-------------------------------------------------------------------------------------
Controllers::Controllers(ENTITY_ID entityID):
lastid_(0),
entityID_(entityID)
{
}

//-------------------------------------------------------------------------------------
Controllers::~Controllers()
{
}

//-------------------------------------------------------------------------------------
void Controllers::clear()
{
	objects_.clear();
	lastid_ = 0;
}

//-------------------------------------------------------------------------------------
bool Controllers::add(KBEShared_ptr<Controller> pController)
{
	uint32 id = pController->id();
	if(id == 0)
	{
		id = freeID();
	}
	else
	{
		// ˢ��id������
		if(lastid_ < id)
			lastid_ = id;
	}

	objects_[id] = pController;
	pController->id(id);
	pController->pControllers(this);

	if(objects_.size() > 32)
	{
		WARNING_MSG(fmt::format("Controllers::add: size = {}.\n", objects_.size()));
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool Controllers::remove(KBEShared_ptr<Controller> pController)
{
	return remove(pController->id());
}

//-------------------------------------------------------------------------------------
bool Controllers::remove(uint32 id)
{
	objects_.erase(id);
	return true;
}

//-------------------------------------------------------------------------------------
void Controllers::addToStream(KBEngine::MemoryStream& s)
{
	uint32 size = objects_.size();
	s << lastid_ << size;

	CONTROLLERS_MAP::iterator iter = objects_.begin();
	for(; iter != objects_.end(); ++iter)
	{
		uint8 itype = (uint8)iter->second->type();
		s << itype;
		iter->second->addToStream(s);
	}
}

//-------------------------------------------------------------------------------------
void Controllers::createFromStream(KBEngine::MemoryStream& s)
{
	uint32 size = 0;
	s >> lastid_ >> size;

	Entity* pEntity = Cellapp::getSingleton().findEntity(entityID_);
	KBE_ASSERT(pEntity);

	for(uint32 i=0; i<size; ++i)
	{
		uint8 itype;
		s >> itype;

		Controller::ControllerType type = (Controller::ControllerType)itype;
		
		KBEShared_ptr<Controller> pController;

		switch(type)
		{
		case Controller::CONTROLLER_TYPE_PROXIMITY:
			pController = KBEShared_ptr<Controller>(new ProximityController(pEntity));
			break;
		case Controller::CONTROLLER_TYPE_MOVE:
		default:
			KBE_ASSERT(false);
			break;
		};
		
		pController->type(type);
		pController->createFromStream(s);

		add(pController);
	}
}

//-------------------------------------------------------------------------------------
}
