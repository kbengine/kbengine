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
#include "helper/profile.h"	

namespace KBEngine{	


//-------------------------------------------------------------------------------------
Space::Space() :
spaceID_(0),
cells_(),
geomappingPath_(),
scriptModuleName_()
{
}

//-------------------------------------------------------------------------------------
Space::~Space()
{
}

//-------------------------------------------------------------------------------------
void Space::updateGeomappingPath(const std::string& geomappingPath)
{
	geomappingPath_ = geomappingPath;
}

//-------------------------------------------------------------------------------------
void Space::updateScriptModuleName(const std::string& scriptModuleName)
{
	scriptModuleName_ = scriptModuleName;
}

//-------------------------------------------------------------------------------------
}
