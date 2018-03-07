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

#include "space.h"
#include "spaces.h"
#include "helper/profile.h"	

namespace KBEngine{	


//-------------------------------------------------------------------------------------
Spaces::Spaces():
spaces_()
{
}

//-------------------------------------------------------------------------------------
Spaces::~Spaces()
{
	spaces_.clear();
}

//-------------------------------------------------------------------------------------
void Spaces::updateSpaceData(SPACE_ID spaceID, const std::string& scriptModuleName, const std::string& geomappingPath, bool deleted)
{
	if (deleted)
	{
		spaces_.erase(spaceID);
		return;
	}

	Space& space = spaces_[spaceID];

	space.setSpaceID(spaceID);
	space.updateGeomappingPath(geomappingPath);
	space.updateScriptModuleName(scriptModuleName);
}

//-------------------------------------------------------------------------------------
}
