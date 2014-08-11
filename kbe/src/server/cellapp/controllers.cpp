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

#include "controllers.hpp"	
#include "helper/profile.hpp"	
#include "cstdkbe/memorystream.hpp"	

namespace KBEngine{	


//-------------------------------------------------------------------------------------
Controllers::Controllers():
lastid_(0)
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
bool Controllers::add(Controller* pController)
{
	uint32 id = pController->id();
	if(id == 0)
		id = freeID();

	objects_[id].reset(pController);
	pController->id(id);
	pController->pControllers(this);

	if(objects_.size() > 32)
	{
		WARNING_MSG(boost::format("Controllers::add: size = %1%.\n") % objects_.size());
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool Controllers::remove(Controller* pController)
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
	uint32 size = 0;
	size = objects_.size();
	s << lastid_ << size;

	CONTROLLERS_MAP::iterator iter = objects_.begin();
	for(; iter != objects_.end(); iter++)
	{
		iter->second->addToStream(s);
	}
}

//-------------------------------------------------------------------------------------
void Controllers::createFromStream(KBEngine::MemoryStream& s)
{
	uint32 size = 0;
	s >> lastid_ >> size;

	for(uint32 i=0; i<size; i++)
	{
	}
}

//-------------------------------------------------------------------------------------
}
