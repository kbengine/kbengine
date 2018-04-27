// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

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
EntityTableRedis::EntityTableRedis(EntityTables* pEntityTables):
EntityTable(pEntityTables)
{
}

//-------------------------------------------------------------------------------------
EntityTableRedis::~EntityTableRedis()
{
}

//-------------------------------------------------------------------------------------
bool EntityTableRedis::initialize(ScriptDefModule* sm, std::string name)
{
	// 获取表名
	tableName(name);

	// 找到所有存储属性并且创建出所有的字段
	ScriptDefModule::PROPERTYDESCRIPTION_MAP& pdescrsMap = sm->getPersistentPropertyDescriptions();
	ScriptDefModule::PROPERTYDESCRIPTION_MAP::const_iterator iter = pdescrsMap.begin();

	for(; iter != pdescrsMap.end(); ++iter)
	{
		PropertyDescription* pdescrs = iter->second;

		EntityTableItem* pETItem = this->createItem(pdescrs->getDataType()->getName(), pdescrs->getDefaultValStr());

		pETItem->pParentTable(this);
		pETItem->utype(pdescrs->getUType());
		pETItem->tableName(this->tableName());

		bool ret = pETItem->initialize(pdescrs, pdescrs->getDataType(), pdescrs->getName());
		
		if(!ret)
		{
			delete pETItem;
			return false;
		}
		
		addItem(pETItem);
	}

	// 特殊处理， 数据库保存方向和位置
	if(sm->hasCell())
	{
		ENTITY_PROPERTY_UID posuid = ENTITY_BASE_PROPERTY_UTYPE_POSITION_XYZ;
		ENTITY_PROPERTY_UID diruid = ENTITY_BASE_PROPERTY_UTYPE_DIRECTION_ROLL_PITCH_YAW;

		Network::FixedMessages::MSGInfo* msgInfo =	
					Network::FixedMessages::getSingleton().isFixed("Property::position");

		if(msgInfo != NULL)
		{
			posuid = msgInfo->msgid;
			msgInfo = NULL;
		}	

		msgInfo = Network::FixedMessages::getSingleton().isFixed("Property::direction");
		if(msgInfo != NULL)
		{
			diruid = msgInfo->msgid;
			msgInfo = NULL;	
		}

		EntityTableItem* pETItem = this->createItem("VECTOR3", "");
		pETItem->pParentTable(this);
		pETItem->utype(posuid);
		pETItem->tableName(this->tableName());
		pETItem->itemName("position");
		addItem(pETItem);

		pETItem = this->createItem("VECTOR3", "");
		pETItem->pParentTable(this);
		pETItem->utype(diruid);
		pETItem->tableName(this->tableName());
		pETItem->itemName("direction");
		addItem(pETItem);
	}

	init_db_item_name();
	return true;
}

//-------------------------------------------------------------------------------------
void EntityTableRedis::init_db_item_name()
{
	EntityTable::TABLEITEM_MAP::iterator iter = tableItems_.begin();
	for(; iter != tableItems_.end(); ++iter)
	{
		// 处理fixedDict字段名称的特例情况
		std::string exstrFlag = "";
		if(iter->second->type() == TABLE_ITEM_TYPE_FIXEDDICT)
		{
			exstrFlag = iter->second->itemName();
			if(exstrFlag.size() > 0)
				exstrFlag += "_";
		}

		static_cast<EntityTableItemRedisBase*>(iter->second.get())->init_db_item_name(exstrFlag.c_str());
	}
}

//-------------------------------------------------------------------------------------
bool EntityTableRedis::syncIndexToDB(DBInterface* pdbi)
{
	return true;
}

//-------------------------------------------------------------------------------------
bool EntityTableRedis::syncToDB(DBInterface* pdbi)
{
	if (hasSync())
		return true;

	// DEBUG_MSG(fmt::format("EntityTableRedis::syncToDB(): {}.\n", tableName()));

	// 对于redis不需要一开始将表创建出来，数据写时才产生数据，因此这里不需要创建表
	// 获取当前表的items，检查每个item是否与当前匹配，将其同步为当前表描述

	//DBInterfaceRedis::TABLE_FIELDS outs;
	//static_cast<DBInterfaceRedis*>(pdbi)->getFields(outs, this->tableName());

	//sync_item_to_db(pdbi, "tinyint not null DEFAULT 0", this->tableName(), TABLE_ITEM_PERFIX"_" TABLE_AUTOLOAD_CONST_STR, 0,
	//	FIELD_TYPE_TINY, NOT_NULL_FLAG, (void*)&outs, &sync_autoload_item_index);

	EntityTable::TABLEITEM_MAP::iterator iter = tableItems_.begin();
	for (; iter != tableItems_.end(); ++iter)
	{
	//	if (!iter->second->syncToDB(pdbi, (void*)&outs))
	//		return false;
	}

	std::vector<std::string> dbTableItemNames;

	std::string ttablename = ENTITY_TABLE_PERFIX"_";
	ttablename += tableName();

	pdbi->getTableItemNames(ttablename.c_str(), dbTableItemNames);

	// 检查是否有需要删除的表字段
	std::vector<std::string>::iterator iter0 = dbTableItemNames.begin();
	for (; iter0 != dbTableItemNames.end(); ++iter0)
	{
		std::string tname = (*iter0);

		if (tname == TABLE_ID_CONST_STR ||
			tname == TABLE_ITEM_PERFIX"_" TABLE_AUTOLOAD_CONST_STR ||
			tname == TABLE_PARENTID_CONST_STR)
		{
			continue;
		}

		EntityTable::TABLEITEM_MAP::iterator iter = tableItems_.begin();
		bool found = false;

		for (; iter != tableItems_.end(); ++iter)
		{
			if (iter->second->isSameKey(tname))
			{
				found = true;
				break;
			}
		}

		if (!found)
		{
			if (!pdbi->dropEntityTableItemFromDB(ttablename.c_str(), tname.c_str()))
				return false;
		}
	}

	// 同步表索引
	if (!syncIndexToDB(pdbi))
		return false;

	sync_ = true;
	return true;
}

//-------------------------------------------------------------------------------------
void EntityTableRedis::queryAutoLoadEntities(DBInterface* pdbi, ScriptDefModule* pModule, 
		ENTITY_ID start, ENTITY_ID end, std::vector<DBID>& outs)
{
}

//-------------------------------------------------------------------------------------
EntityTableItem* EntityTableRedis::createItem(std::string type, std::string defaultVal)
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
	else if(type == "ENTITYCALL")
	{
		return new EntityTableItemRedis_ENTITYCALL("blob", 0, 0);
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
bool EntityTableItemRedis_ENTITYCALL::syncToDB(DBInterface* pdbi, void* pData)
{
	return true;
}

//-------------------------------------------------------------------------------------
void EntityTableItemRedis_ENTITYCALL::addToStream(MemoryStream* s, redis::DBContext& context, DBID resultDBID)
{
}

//-------------------------------------------------------------------------------------
void EntityTableItemRedis_ENTITYCALL::getWriteSqlItem(DBInterface* pdbi, MemoryStream* s, redis::DBContext& context)
{
}

//-------------------------------------------------------------------------------------
void EntityTableItemRedis_ENTITYCALL::getReadSqlItem(redis::DBContext& context)
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
	itemName(name);

	pDataType_ = pDataType;
	pPropertyDescription_ = pPropertyDescription;
	indexType_ = pPropertyDescription->indexType();
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
