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
#include "space.h"
#include "profile.h"

#ifndef CODE_INLINE
#include "space.inl"
#endif

namespace KBEngine{

SCRIPT_METHOD_DECLARE_BEGIN(Space)
SCRIPT_METHOD_DECLARE_END()

SCRIPT_MEMBER_DECLARE_BEGIN(Space)
SCRIPT_MEMBER_DECLARE_END()

SCRIPT_GETSET_DECLARE_BEGIN(Space)
SCRIPT_GETSET_DECLARE_END()
BASE_SCRIPT_INIT(Space, 0, 0, 0, 0, 0)
	
//-------------------------------------------------------------------------------------
Space::Space(ENTITY_ID id, const ScriptDefModule* pScriptModule):
Entity(id, pScriptModule, getScriptType(), true)
{

}

//-------------------------------------------------------------------------------------
Space::~Space()
{

}

//-------------------------------------------------------------------------------------
}
