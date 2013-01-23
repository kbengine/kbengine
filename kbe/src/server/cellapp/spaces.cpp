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

#include "spaces.hpp"	
namespace KBEngine{	
Spaces::SPACES Spaces::spaces_;

//-------------------------------------------------------------------------------------
Spaces::Spaces()
{
}

//-------------------------------------------------------------------------------------
Spaces::~Spaces()
{
}

//-------------------------------------------------------------------------------------
Space* Spaces::createNewSpace(SPACE_ID spaceID)
{
	SPACES::iterator iter = spaces_.find(spaceID);
	if(iter != spaces_.end())
	{
		ERROR_MSG(boost::format("Spaces::createNewSpace: space %1% is exist!\n") % spaceID);
		return NULL;
	}
	
	Space* space = new Space(spaceID);
	spaces_[spaceID].reset(space);
	
	DEBUG_MSG(boost::format("Spaces::createNewSpace: new space %1%.\n") % spaceID);
	return space;
}

//-------------------------------------------------------------------------------------
bool Spaces::destroySpace(SPACE_ID spaceID, ENTITY_ID entityID)
{
	INFO_MSG(boost::format("Spaces::destroySpace: %1%.\n") % spaceID);

	Space* pSpace = Spaces::findSpace(spaceID);
	return pSpace->destroy(entityID);
}

//-------------------------------------------------------------------------------------
Space* Spaces::findSpace(SPACE_ID spaceID)
{
	SPACES::iterator iter = spaces_.find(spaceID);
	if(iter != spaces_.end())
		return iter->second.get();
	
	return NULL;
}

//-------------------------------------------------------------------------------------
void Spaces::update()
{
	SPACES::iterator iter = spaces_.begin();
	for(;iter != spaces_.end(); iter++)
		iter->second->update();
}

//-------------------------------------------------------------------------------------
}
