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
#include "entityref.h"
#include "entity.h"
#include "cellapp.h"
#include "common/memorystream.h"

namespace KBEngine{	

//-------------------------------------------------------------------------------------
EntityRef::EntityRef(Entity* pEntity):
id_(0),
pEntity_(pEntity),
flags_(ENTITYREF_FLAG_UNKONWN)
{
	id_ = pEntity->id();
}

//-------------------------------------------------------------------------------------
EntityRef::EntityRef():
id_(0),
pEntity_(NULL),
flags_(ENTITYREF_FLAG_UNKONWN)
{
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
		id_ = e->id(); 
}

//-------------------------------------------------------------------------------------
bool findif_vector_entityref_exist_by_entity_handler::operator()(const EntityRef* obj)
{
	return obj->id() == obj_->id();
}

//-------------------------------------------------------------------------------------
bool findif_vector_entityref_exist_by_entityid_handler::operator()(const EntityRef* obj)
{
	return obj->id() == entityID_;
}

//-------------------------------------------------------------------------------------
void EntityRef::addToStream(KBEngine::MemoryStream& s)
{
	ENTITY_ID eid = 0;
	if(pEntity_)
		eid = pEntity_->id();

	s << id_ << flags_ << eid;
}

//-------------------------------------------------------------------------------------
void EntityRef::createFromStream(KBEngine::MemoryStream& s)
{
	ENTITY_ID eid = 0;
	s >> id_ >> flags_ >> eid;

	if(eid > 0)
	{
		pEntity_ = Cellapp::getSingleton().findEntity(eid);
	}
}

//-------------------------------------------------------------------------------------
}
