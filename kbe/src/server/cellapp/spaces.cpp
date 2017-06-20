/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2017 KBEngine.

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

#include "spaces.h"	
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
void Spaces::finalise()
{
	Spaces::SPACES spaces = spaces_;
	while (spaces.size() > 0)
	{
		SPACES::iterator iter = spaces.begin();
		KBEShared_ptr<Space> pSpace = iter->second;
		spaces.erase(iter++);
		pSpace->destroy(0, false);
	}

	spaces_.clear();
}

//-------------------------------------------------------------------------------------
Space* Spaces::createNewSpace(SPACE_ID spaceID, const std::string& scriptModuleName)
{
	SPACES::iterator iter = spaces_.find(spaceID);
	if(iter != spaces_.end())
	{
		ERROR_MSG(fmt::format("Spaces::createNewSpace: space {} is exist! scriptModuleName={}\n", spaceID, scriptModuleName));
		return NULL;
	}
	
	Space* space = new Space(spaceID, scriptModuleName);
	spaces_[spaceID].reset(space);
	
	DEBUG_MSG(fmt::format("Spaces::createNewSpace: new space({}) {}.\n", scriptModuleName, spaceID));
	return space;
}

//-------------------------------------------------------------------------------------
bool Spaces::destroySpace(SPACE_ID spaceID, ENTITY_ID entityID)
{
	INFO_MSG(fmt::format("Spaces::destroySpace: {}.\n", spaceID));

	Space* pSpace = Spaces::findSpace(spaceID);
	if(!pSpace)
		return true;
	
	if(pSpace->isDestroyed())
		return true;

	if(!pSpace->destroy(entityID))
	{
		//WARNING_MSG("Spaces::destroySpace: destroying!\n");
		return false;
	}

	// 延时一段时间再销毁
	//spaces_.erase(spaceID);
	return true;
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

	for(; iter != spaces_.end(); )
	{
		if(!iter->second->update())
		{
			spaces_.erase(iter++);
		}
		else
		{
			++iter;
		}
	}
}

//-------------------------------------------------------------------------------------
}
