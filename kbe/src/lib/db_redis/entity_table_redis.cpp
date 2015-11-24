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

#include "entity_table_redis.h"
#include "kbe_table_redis.h"
#include "entitydef/scriptdef_module.h"
#include "entitydef/property.h"
#include "db_interface/db_interface.h"
#include "db_interface/entity_table.h"
#include "network/fixed_messages.h"

#ifndef CODE_INLINE
#include "entity_table_redis.inl"
#endif

namespace KBEngine { 


//-------------------------------------------------------------------------------------
EntityTableRedis::EntityTableRedis()
{
}

//-------------------------------------------------------------------------------------
EntityTableRedis::~EntityTableRedis()
{
}

//-------------------------------------------------------------------------------------
bool EntityTableRedis::initialize(ScriptDefModule* sm, std::string name)
{
	return true;
}

//-------------------------------------------------------------------------------------
void EntityTableRedis::init_db_item_name()
{
}

//-------------------------------------------------------------------------------------
bool EntityTableRedis::syncIndexToDB(DBInterface* pdbi)
{
	return true;
}

//-------------------------------------------------------------------------------------
bool EntityTableRedis::syncToDB(DBInterface* pdbi)
{
	if(hasSync())
		return true;

	// DEBUG_MSG(fmt::format("EntityTableRedis::syncToDB(): {}.\n", tableName()));

	sync_ = true;
	return true;
}

//-------------------------------------------------------------------------------------
void EntityTableRedis::queryAutoLoadEntities(DBInterface* pdbi, ScriptDefModule* pModule, 
		ENTITY_ID start, ENTITY_ID end, std::vector<DBID>& outs)
{
}

//-------------------------------------------------------------------------------------
EntityTableItem* EntityTableRedis::createItem(std::string type)
{
	if(type == "INT8")
	{
		return new EntityTableItemRedis_DIGIT(type, "tinyint not null DEFAULT 0", 4, 0);
	}
	else if(type == "INT16")
	{
		return new EntityTableItemRedis_DIGIT(type, "smallint not null DEFAULT 0", 6, 0);
	}
	else if(type == "INT32")
	{
		return new EntityTableItemRedis_DIGIT(type, "int not null DEFAULT 0", 11, 0);
	}
	else if(type == "INT64")
	{
		return new EntityTableItemRedis_DIGIT(type, "bigint not null DEFAULT 0", 20, 0);
	}
	else if(type == "UINT8")
	{
		return new EntityTableItemRedis_DIGIT(type, "tinyint unsigned not null DEFAULT 0", 3, 0);
	}
	else if(type == "UINT16")
	{
		return new EntityTableItemRedis_DIGIT(type, "smallint unsigned not null DEFAULT 0", 5, 0);
	}
	else if(type == "UINT32")
	{
		return new EntityTableItemRedis_DIGIT(type, "int unsigned not null DEFAULT 0", 10, 0);
	}
	else if(type == "UINT64")
	{
		return new EntityTableItemRedis_DIGIT(type, "bigint unsigned not null DEFAULT 0", 20, 0);
	}
	else if(type == "FLOAT")
	{
		return new EntityTableItemRedis_DIGIT(type, "float not null DEFAULT 0", 0, 0);
	}
	else if(type == "DOUBLE")
	{
		return new EntityTableItemRedis_DIGIT(type, "double not null DEFAULT 0", 0, 0);
	}
	else if(type == "STRING")
	{
		return new EntityTableItemRedis_STRING("text", 0, 0);
	}
	else if(type == "UNICODE")
	{
		return new EntityTableItemRedis_UNICODE("text", 0, 0);
	}
	else if(type == "PYTHON")
	{
		return new EntityTableItemRedis_PYTHON("blob", 0, 0);
	}
	else if(type == "PY_DICT")
	{
		return new EntityTableItemRedis_PYTHON("blob", 0, 0);
	}
	else if(type == "PY_TUPLE")
	{
		return new EntityTableItemRedis_PYTHON("blob", 0, 0);
	}
	else if(type == "PY_LIST")
	{
		return new EntityTableItemRedis_PYTHON("blob", 0, 0);
	}
	else if(type == "BLOB")
	{
		return new EntityTableItemRedis_BLOB("blob", 0, 0);
	}
	else if(type == "ARRAY")
	{
		return new EntityTableItemRedis_ARRAY("", 0, 0);
	}
	else if(type == "FIXED_DICT")
	{
		return new EntityTableItemRedis_FIXED_DICT("", 0, 0);
	}
#ifdef CLIENT_NO_FLOAT
	else if(type == "VECTOR2")
	{
		return new EntityTableItemRedis_VECTOR2("int not null DEFAULT 0", 0, 0);
	}
	else if(type == "VECTOR3")
	{
		return new EntityTableItemRedis_VECTOR3("int not null DEFAULT 0", 0, 0);
	}
	else if(type == "VECTOR4")
	{
		return new EntityTableItemRedis_VECTOR4("int not null DEFAULT 0", 0, 0);
	}
#else
	else if(type == "VECTOR2")
	{
		return new EntityTableItemRedis_VECTOR2("float not null DEFAULT 0", 0, 0);
	}
	else if(type == "VECTOR3")
	{
		return new EntityTableItemRedis_VECTOR3("float not null DEFAULT 0", 0, 0);
	}
	else if(type == "VECTOR4")
	{
		return new EntityTableItemRedis_VECTOR4("float not null DEFAULT 0", 0, 0);
	}
#endif
	else if(type == "MAILBOX")
	{
		return new EntityTableItemRedis_MAILBOX("blob", 0, 0);
	}

	KBE_ASSERT(false && "not found type.\n");
	return new EntityTableItemRedis_STRING("", 0, 0);
}

//-------------------------------------------------------------------------------------
void EntityTableRedis::entityShouldAutoLoad(DBInterface* pdbi, DBID dbid, bool shouldAutoLoad)
{
	if(dbid == 0)
		return;

}

//-------------------------------------------------------------------------------------
DBID EntityTableRedis::writeTable(DBInterface* pdbi, DBID dbid, int8 shouldAutoLoad, MemoryStream* s, ScriptDefModule* pModule)
{
	return 0;
}

//-------------------------------------------------------------------------------------
bool EntityTableRedis::removeEntity(DBInterface* pdbi, DBID dbid, ScriptDefModule* pModule)
{
	return true;
}

//-------------------------------------------------------------------------------------
bool EntityTableRedis::queryTable(DBInterface* pdbi, DBID dbid, MemoryStream* s, ScriptDefModule* pModule)
{
	return true;
}

//-------------------------------------------------------------------------------------
void EntityTableRedis::addToStream(MemoryStream* s, redis::DBContext& context, DBID resultDBID)
{
}

//-------------------------------------------------------------------------------------
void EntityTableRedis::getWriteSqlItem(DBInterface* pdbi, MemoryStream* s, redis::DBContext& context)
{
}

//-------------------------------------------------------------------------------------
void EntityTableRedis::getReadSqlItem(redis::DBContext& context)
{
}

//-------------------------------------------------------------------------------------
void EntityTableItemRedisBase::init_db_item_name(const char* exstrFlag)
{
}

//-------------------------------------------------------------------------------------
bool EntityTableItemRedis_VECTOR2::isSameKey(std::string key)
{
	return true;
}

//-------------------------------------------------------------------------------------
bool EntityTableItemRedis_VECTOR2::syncToDB(DBInterface* pdbi, void* pData)
{
	return true;
}

//-------------------------------------------------------------------------------------
void EntityTableItemRedis_VECTOR2::addToStream(MemoryStream* s, redis::DBContext& context, DBID resultDBID)
{
}

//-------------------------------------------------------------------------------------
void EntityTableItemRedis_VECTOR2::getWriteSqlItem(DBInterface* pdbi, MemoryStream* s, redis::DBContext& context)
{
	if(s == NULL)
		return;
}

//-------------------------------------------------------------------------------------
void EntityTableItemRedis_VECTOR2::getReadSqlItem(redis::DBContext& context)
{
}

//-------------------------------------------------------------------------------------
bool EntityTableItemRedis_VECTOR3::isSameKey(std::string key)
{
	return true;
}

//-------------------------------------------------------------------------------------
bool EntityTableItemRedis_VECTOR3::syncToDB(DBInterface* pdbi, void* pData)
{
	return true;
}

//-------------------------------------------------------------------------------------
void EntityTableItemRedis_VECTOR3::addToStream(MemoryStream* s, redis::DBContext& context, DBID resultDBID)
{
}

//-------------------------------------------------------------------------------------
void EntityTableItemRedis_VECTOR3::getWriteSqlItem(DBInterface* pdbi, MemoryStream* s, redis::DBContext& context)
{
	if(s == NULL)
		return;
}

//-------------------------------------------------------------------------------------
void EntityTableItemRedis_VECTOR3::getReadSqlItem(redis::DBContext& context)
{
}

//-------------------------------------------------------------------------------------
bool EntityTableItemRedis_VECTOR4::isSameKey(std::string key)
{
	return true;
}

//-------------------------------------------------------------------------------------
bool EntityTableItemRedis_VECTOR4::syncToDB(DBInterface* pdbi, void* pData)
{
	return true;
}

//-------------------------------------------------------------------------------------
void EntityTableItemRedis_VECTOR4::addToStream(MemoryStream* s, redis::DBContext& context, DBID resultDBID)
{
}

//-------------------------------------------------------------------------------------
void EntityTableItemRedis_VECTOR4::getWriteSqlItem(DBInterface* pdbi, MemoryStream* s, redis::DBContext& context)
{
	if(s == NULL)
		return;
}

//-------------------------------------------------------------------------------------
void EntityTableItemRedis_VECTOR4::getReadSqlItem(redis::DBContext& context)
{
}

//-------------------------------------------------------------------------------------
bool EntityTableItemRedis_MAILBOX::syncToDB(DBInterface* pdbi, void* pData)
{
	return true;
}

//-------------------------------------------------------------------------------------
void EntityTableItemRedis_MAILBOX::addToStream(MemoryStream* s, redis::DBContext& context, DBID resultDBID)
{
}

//-------------------------------------------------------------------------------------
void EntityTableItemRedis_MAILBOX::getWriteSqlItem(DBInterface* pdbi, MemoryStream* s, redis::DBContext& context)
{
}

//-------------------------------------------------------------------------------------
void EntityTableItemRedis_MAILBOX::getReadSqlItem(redis::DBContext& context)
{
}

//-------------------------------------------------------------------------------------
bool EntityTableItemRedis_ARRAY::isSameKey(std::string key)
{
	return false;
}

//-------------------------------------------------------------------------------------
bool EntityTableItemRedis_ARRAY::initialize(const PropertyDescription* pPropertyDescription, 
											const DataType* pDataType, std::string name)
{
	return true;
}

//-------------------------------------------------------------------------------------
bool EntityTableItemRedis_ARRAY::syncToDB(DBInterface* pdbi, void* pData)
{
	return true;
}

//-------------------------------------------------------------------------------------
void EntityTableItemRedis_ARRAY::addToStream(MemoryStream* s, redis::DBContext& context, DBID resultDBID)
{
}

//-------------------------------------------------------------------------------------
void EntityTableItemRedis_ARRAY::getWriteSqlItem(DBInterface* pdbi, MemoryStream* s, redis::DBContext& context)
{
}

//-------------------------------------------------------------------------------------
void EntityTableItemRedis_ARRAY::getReadSqlItem(redis::DBContext& context)
{
}

//-------------------------------------------------------------------------------------
void EntityTableItemRedis_ARRAY::init_db_item_name(const char* exstrFlag)
{
}

//-------------------------------------------------------------------------------------
bool EntityTableItemRedis_FIXED_DICT::isSameKey(std::string key)
{
	return true;
}

//-------------------------------------------------------------------------------------
bool EntityTableItemRedis_FIXED_DICT::initialize(const PropertyDescription* pPropertyDescription, 
												 const DataType* pDataType, std::string name)
{
	return true;
}

//-------------------------------------------------------------------------------------
bool EntityTableItemRedis_FIXED_DICT::syncToDB(DBInterface* pdbi, void* pData)
{

	return true;
}

//-------------------------------------------------------------------------------------
void EntityTableItemRedis_FIXED_DICT::addToStream(MemoryStream* s, redis::DBContext& context, DBID resultDBID)
{
}

//-------------------------------------------------------------------------------------
void EntityTableItemRedis_FIXED_DICT::getWriteSqlItem(DBInterface* pdbi, MemoryStream* s, redis::DBContext& context)
{
}

//-------------------------------------------------------------------------------------
void EntityTableItemRedis_FIXED_DICT::getReadSqlItem(redis::DBContext& context)
{
}

//-------------------------------------------------------------------------------------
void EntityTableItemRedis_FIXED_DICT::init_db_item_name(const char* exstrFlag)
{
}

//-------------------------------------------------------------------------------------
bool EntityTableItemRedisBase::initialize(const PropertyDescription* pPropertyDescription, 
										  const DataType* pDataType, std::string name)
{
	return true;
}

//-------------------------------------------------------------------------------------
bool EntityTableItemRedis_DIGIT::syncToDB(DBInterface* pdbi, void* pData)
{
	return true;
}

//-------------------------------------------------------------------------------------
void EntityTableItemRedis_DIGIT::addToStream(MemoryStream* s, redis::DBContext& context, DBID resultDBID)
{
}

//-------------------------------------------------------------------------------------
void EntityTableItemRedis_DIGIT::getWriteSqlItem(DBInterface* pdbi, MemoryStream* s, redis::DBContext& context)
{
	if(s == NULL)
		return;
}

//-------------------------------------------------------------------------------------
void EntityTableItemRedis_DIGIT::getReadSqlItem(redis::DBContext& context)
{
}

//-------------------------------------------------------------------------------------
bool EntityTableItemRedis_STRING::syncToDB(DBInterface* pdbi, void* pData)
{
	return true;
}

//-------------------------------------------------------------------------------------
void EntityTableItemRedis_STRING::addToStream(MemoryStream* s, 
											  redis::DBContext& context, DBID resultDBID)
{
}

//-------------------------------------------------------------------------------------
void EntityTableItemRedis_STRING::getWriteSqlItem(DBInterface* pdbi, 
												  MemoryStream* s, redis::DBContext& context)
{
	if(s == NULL)
		return;
}

//-------------------------------------------------------------------------------------
void EntityTableItemRedis_STRING::getReadSqlItem(redis::DBContext& context)
{
}

//-------------------------------------------------------------------------------------
bool EntityTableItemRedis_UNICODE::syncToDB(DBInterface* pdbi, void* pData)
{
	return true;
}

//-------------------------------------------------------------------------------------
void EntityTableItemRedis_UNICODE::addToStream(MemoryStream* s, 
											   redis::DBContext& context, DBID resultDBID)
{
}

//-------------------------------------------------------------------------------------
void EntityTableItemRedis_UNICODE::getWriteSqlItem(DBInterface* pdbi, MemoryStream* s, 
												   redis::DBContext& context)
{
	if(s == NULL)
		return;
}

//-------------------------------------------------------------------------------------
void EntityTableItemRedis_UNICODE::getReadSqlItem(redis::DBContext& context)
{
}

//-------------------------------------------------------------------------------------
bool EntityTableItemRedis_BLOB::syncToDB(DBInterface* pdbi, void* pData)
{
	return true;
}

//-------------------------------------------------------------------------------------
void EntityTableItemRedis_BLOB::addToStream(MemoryStream* s, redis::DBContext& context, DBID resultDBID)
{
}

//-------------------------------------------------------------------------------------
void EntityTableItemRedis_BLOB::getWriteSqlItem(DBInterface* pdbi, MemoryStream* s, redis::DBContext& context)
{
	if(s == NULL)
		return;
}

//-------------------------------------------------------------------------------------
void EntityTableItemRedis_BLOB::getReadSqlItem(redis::DBContext& context)
{
}

//-------------------------------------------------------------------------------------
bool EntityTableItemRedis_PYTHON::syncToDB(DBInterface* pdbi, void* pData)
{
	return true;
}

//-------------------------------------------------------------------------------------
void EntityTableItemRedis_PYTHON::addToStream(MemoryStream* s, redis::DBContext& context, DBID resultDBID)
{
}

//-------------------------------------------------------------------------------------
void EntityTableItemRedis_PYTHON::getWriteSqlItem(DBInterface* pdbi, MemoryStream* s, redis::DBContext& context)
{
	if(s == NULL)
		return;
}

//-------------------------------------------------------------------------------------
void EntityTableItemRedis_PYTHON::getReadSqlItem(redis::DBContext& context)
{
}

//-------------------------------------------------------------------------------------
}
