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
#include "entityref.h"
#include "entity.h"
#include "cellapp.h"
#include "common/memorystream.h"

namespace KBEngine{	

//-------------------------------------------------------------------------------------
EntityRef::EntityRef(Entity* pEntity):
id_(0),
aliasID_(0),
pEntity_(pEntity),
flags_(ENTITYREF_FLAG_UNKONWN)
{
	id_ = pEntity->id();
}

//-------------------------------------------------------------------------------------
EntityRef::EntityRef():
id_(0),
aliasID_(0),
pEntity_(NULL),
flags_(ENTITYREF_FLAG_UNKONWN)
{
}

//-------------------------------------------------------------------------------------
EntityRef::~EntityRef()
{
}

//-------------------------------------------------------------------------------------
static ObjectPool<EntityRef> _g_objPool("EntityRef");
ObjectPool<EntityRef>& EntityRef::ObjPool()
{
	return _g_objPool;
}

//-------------------------------------------------------------------------------------
EntityRef* EntityRef::createPoolObject()
{
	return _g_objPool.createObject();
}

//-------------------------------------------------------------------------------------
void EntityRef::reclaimPoolObject(EntityRef* obj)
{
	_g_objPool.reclaimObject(obj);
}

//-------------------------------------------------------------------------------------
void EntityRef::destroyObjPool()
{
	DEBUG_MSG(fmt::format("EntityRef::destroyObjPool(): size {}.\n",
		_g_objPool.size()));

	_g_objPool.destroy();
}

//-------------------------------------------------------------------------------------
EntityRef::SmartPoolObjectPtr EntityRef::createSmartPoolObj()
{
	return SmartPoolObjectPtr(new SmartPoolObject<EntityRef>(ObjPool().createObject(), _g_objPool));
}

//-------------------------------------------------------------------------------------
void EntityRef::onReclaimObject()
{
	id_ = 0;
	aliasID_ =  0;
	pEntity_ = NULL;
	flags_ = ENTITYREF_FLAG_UNKONWN;
}

//-------------------------------------------------------------------------------------
void EntityRef::pEntity(Entity* e)
{
	pEntity_ = e; 

	if(e)
		id_ = e->id(); 
}

//-------------------------------------------------------------------------------------
void EntityRef::addToStream(KBEngine::MemoryStream& s)
{
	ENTITY_ID eid = 0;
	if(pEntity_)
		eid = pEntity_->id();

	s << id_ << aliasID_ << flags_ << eid;
}

//-------------------------------------------------------------------------------------
void EntityRef::createFromStream(KBEngine::MemoryStream& s)
{
	ENTITY_ID eid = 0;
	s >> id_ >> aliasID_ >> flags_ >> eid;

	if(eid > 0)
	{
		pEntity_ = Cellapp::getSingleton().findEntity(eid);
	}
}

//-------------------------------------------------------------------------------------
}
