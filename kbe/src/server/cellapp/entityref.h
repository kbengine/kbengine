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

#ifndef KBE_ENTITY_REF_H
#define KBE_ENTITY_REF_H

#include "helper/debug_helper.h"
#include "common/common.h"	
#include "common/objectpool.h"

namespace KBEngine{

class Entity;
class MemoryStream;

#define ENTITYREF_FLAG_UNKONWN							0x00000000
#define ENTITYREF_FLAG_ENTER_CLIENT_PENDING				0x00000001	// 进入客户端中标志
#define ENTITYREF_FLAG_LEAVE_CLIENT_PENDING				0x00000002	// 离开客户端中标志
#define ENTITYREF_FLAG_NORMAL							0x00000004	// 常规状态

class EntityRef : public PoolObject
{
public:
	EntityRef(Entity* pEntity);
	EntityRef();

	~EntityRef();
	
	typedef KBEShared_ptr< SmartPoolObject< EntityRef > > SmartPoolObjectPtr;
	static SmartPoolObjectPtr createSmartPoolObj();

	static ObjectPool<EntityRef>& ObjPool();
	static EntityRef* createPoolObject();
	static void reclaimPoolObject(EntityRef* obj);
	static void destroyObjPool();
	void onReclaimObject();

	virtual size_t getPoolObjectBytes()
	{
		size_t bytes = sizeof(id_)
			+ sizeof(aliasID_) + sizeof(pEntity_)
			+ sizeof(flags_);

		return bytes;
	}

	void flags(uint32 v) { flags_ = v; }
	void removeflags(uint32 v) { flags_ &= ~v; }
	uint32 flags() { return flags_; }
	
	Entity* pEntity() const { return pEntity_; }
	void pEntity(Entity* e);

	ENTITY_ID id() const { return id_; }

	int aliasID() const { return aliasID_; }
	void aliasID(int id) { aliasID_ = id; }

	void addToStream(KBEngine::MemoryStream& s);
	void createFromStream(KBEngine::MemoryStream& s);

private:
	ENTITY_ID id_;
	int aliasID_;
	Entity* pEntity_;
	uint32 flags_;
};

}

#endif
