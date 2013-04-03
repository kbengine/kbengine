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
#include "entityref.hpp"
#include "entity.hpp"

namespace KBEngine{	

//-------------------------------------------------------------------------------------
EntityRef::EntityRef(Entity* pEntity):
pEntity_(pEntity),
flags_(ENTITYREF_FLAG_UNKONWN)
{
	id_ = pEntity->getID();
}

//-------------------------------------------------------------------------------------
EntityRef::~EntityRef()
{
}

//-------------------------------------------------------------------------------------
void EntityRef::pEntity(Entity* e)
{
	pEntity_ = e; 

	if(e)
		id_ = e->getID(); 
}

//-------------------------------------------------------------------------------------
bool findif_vector_entityref_exist_handler::operator()(const EntityRef* obj)
{
	return obj->id() == obj_->getID();
}

//-------------------------------------------------------------------------------------
}
