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

#include "entity_table.hpp"
#include "entitydef/entitydef.hpp"

namespace KBEngine { 
KBE_SINGLETON_INIT(EntityTables);

EntityTables g_EntityTables;

//-------------------------------------------------------------------------------------
EntityTables::EntityTables()
{
}

//-------------------------------------------------------------------------------------
EntityTables::~EntityTables()
{
}

//-------------------------------------------------------------------------------------
bool EntityTables::load()
{
	EntityDef::SCRIPT_MODULES smodules = EntityDef::getScriptModules();
	EntityDef::SCRIPT_MODULES::const_iterator iter = smodules.begin();
	for(; iter != smodules.end(); iter++)
	{
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool EntityTables::syncAllToDB()
{
	return true;
}

//-------------------------------------------------------------------------------------
}
