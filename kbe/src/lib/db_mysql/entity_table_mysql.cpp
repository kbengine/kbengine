// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include "entity_table_mysql.h"
#include "kbe_table_mysql.h"
#include "read_entity_helper.h"
#include "write_entity_helper.h"
#include "remove_entity_helper.h"
#include "entitydef/scriptdef_module.h"
#include "entitydef/property.h"
#include "entitydef/entitydef.h"
#include "db_interface/db_interface.h"
#include "db_interface/entity_table.h"
#include "network/fixed_messages.h"

#ifndef CODE_INLINE
#include "entity_table_mysql.inl"
#endif

namespace KBEngine { 

// 同步成功时回调
typedef void (*onSyncItemToDBSuccessPtr)(DBInterface*, const char*, const char*);

bool sync_item_to_db(DBInterface* pdbi, 
					 const char* datatype, 
					 const char* tableName, 
					 const char* itemName, 
					 uint32 length, 
					 enum_field_types sqlitemtype, 
					 unsigned int itemflags, 
					 void* pData, onSyncItemToDBSuccessPtr callback = NULL)	
{
	if(pData)
	{
		DBInterfaceMysql::TABLE_FIELDS* pTFData = static_cast<DBInterfaceMysql::TABLE_FIELDS*>(pData);
		DBInterfaceMysql::TABLE_FIELDS::iterator iter = pTFData->find(itemName);
		if(iter != pTFData->end())
		{
			MYSQL_TABLE_FIELD& tf = iter->second;
			if (tf.type == sqlitemtype && ((tf.flags & ALL_MYSQL_SET_FLAGS) == itemflags))
			{
				if ((length == 0) || (sqlitemtype == FIELD_TYPE_VAR_STRING ? (int32)length == tf.length / SYSTEM_CHARSET_MBMAXLEN/*Mysql将length放大了N倍*/ : 
					(int32)length == tf.length))
					return true;
			}
		}
	}

	DEBUG_MSG(fmt::format("syncToDB(): {}->{}({}).\n", tableName, itemName, datatype));

	char __sql_str__[MAX_BUF];

	kbe_snprintf(__sql_str__, MAX_BUF, "ALTER TABLE `" ENTITY_TABLE_PERFIX "_%s` ADD `%s` %s;",
		tableName, itemName, datatype);	

	try
	{
		pdbi->query(__sql_str__, strlen(__sql_str__), false);	
	}
	catch(...)
	{
	}	

	unsigned int mysql_errorno = pdbi->getlasterror();
	if (mysql_errorno == 1060/* Duplicate column name */)
	{
		kbe_snprintf(__sql_str__, MAX_BUF, "ALTER TABLE `" ENTITY_TABLE_PERFIX "_%s` MODIFY COLUMN `%s` %s;",	
			tableName, itemName, datatype);

		try
		{
			if(pdbi->query(__sql_str__, strlen(__sql_str__), false))
			{
				if(callback)
					(*callback)(pdbi, tableName, itemName);

				return true;
			}
		}
		catch(...)
		{
			ERROR_MSG(fmt::format("syncToDB(0): {}->{}({}), error({}: {})\n lastQuery: {}.\n", 
				tableName, itemName, datatype, pdbi->getlasterror(), pdbi->getstrerror(), static_cast<DBInterfaceMysql*>(pdbi)->lastquery()));

			return false;
		}
	}
	else if(mysql_errorno > 0)
	{
		ERROR_MSG(fmt::format("syncToDB(1): {}->{}({}), error({}: {})\n lastQuery: {}.\n",
			tableName, itemName, datatype, pdbi->getlasterror(), pdbi->getstrerror(), static_cast<DBInterfaceMysql*>(pdbi)->lastquery()));

		return false;
	}

	if(callback)
		(*callback)(pdbi, tableName, itemName);

	return true;
}		

void sync_autoload_item_index(DBInterface* pdbi, const char* tableName, const char* itemName)
{
	// 创建sm_autoLoad的索引
	std::string sql = fmt::format("ALTER TABLE " ENTITY_TABLE_PERFIX "_{} ADD INDEX ({})", tableName, itemName);

	try
	{
		pdbi->query(sql.c_str(), sql.size(), true);
	}
	catch(...)
	{
	}
}

//-------------------------------------------------------------------------------------
EntityTableMysql::EntityTableMysql(EntityTables* pEntityTables):
EntityTable(pEntityTables)
{
}

//-------------------------------------------------------------------------------------
EntityTableMysql::~EntityTableMysql()
{
}

//-------------------------------------------------------------------------------------
bool EntityTableMysql::initialize(ScriptDefModule* sm, std::string name)
{
	// 获取表名
	tableName(name);

	// 找到所有存储属性并且创建出所有的字段
	ScriptDefModule::PROPERTYDESCRIPTION_MAP& pdescrsMap = sm->getPersistentPropertyDescriptions();
	ScriptDefModule::PROPERTYDESCRIPTION_MAP::const_iterator iter = pdescrsMap.begin();

	for(; iter != pdescrsMap.end(); ++iter)
	{
		PropertyDescription* pdescrs = iter->second;

		// 如果某个实体没有cell部分， 而组件属性没有base部分则忽略
		if (!sm->hasCell())
		{
			if (pdescrs->getDataType()->type() == DATA_TYPE_ENTITY_COMPONENT && !pdescrs->hasBase())
				continue;
		}

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
void EntityTableMysql::init_db_item_name()
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

		static_cast<EntityTableItemMysqlBase*>(iter->second.get())->init_db_item_name(exstrFlag.c_str());
	}
}

//-------------------------------------------------------------------------------------
bool EntityTableMysql::syncIndexToDB(DBInterface* pdbi)
{
	std::vector<EntityTableItem*> indexs;

	EntityTable::TABLEITEM_MAP::iterator iter = tableItems_.begin();
	for(; iter != tableItems_.end(); ++iter)
	{
		if(strlen(iter->second->indexType()) == 0)
			continue;

		indexs.push_back(iter->second.get());
	}

	char sql_str[SQL_BUF];

	kbe_snprintf(sql_str, SQL_BUF, "show index from " ENTITY_TABLE_PERFIX "_%s",
		tableName());

	try
	{
		bool ret = pdbi->query(sql_str, strlen(sql_str), false);
		if(!ret)
		{
			return false;
		}
	}
	catch(...)
	{
		return false;
	}

	KBEUnordered_map<std::string, std::string> currDBKeys;

	MYSQL_RES * pResult = mysql_store_result(static_cast<DBInterfaceMysql*>(pdbi)->mysql());
	if(pResult)
	{
		MYSQL_ROW arow;
		while((arow = mysql_fetch_row(pResult)) != NULL)
		{
			std::string keytype = "UNIQUE";

			if(std::string("1") == arow[1])
				keytype = "INDEX";

			std::string keyname = arow[2];
			std::string colname = arow[4];

			if(keyname == "PRIMARY" || colname != keyname || 
				keyname == TABLE_PARENTID_CONST_STR ||
				keyname == TABLE_ID_CONST_STR ||
				keyname == TABLE_ITEM_PERFIX"_" TABLE_AUTOLOAD_CONST_STR)
				continue;

			currDBKeys[colname] = keytype;
		}

		mysql_free_result(pResult);
	}

	bool done = false;
	std::string sql = fmt::format("ALTER TABLE " ENTITY_TABLE_PERFIX "_{} ", tableName());
	std::vector<EntityTableItem*>::iterator iiter = indexs.begin();
	for(; iiter != indexs.end(); )
	{
		std::string itemName = fmt::format(TABLE_ITEM_PERFIX"_{}", (*iiter)->itemName());
		KBEUnordered_map<std::string, std::string>::iterator fiter = currDBKeys.find(itemName);
		if(fiter != currDBKeys.end())
		{
			bool deleteIndex = fiter->second != (*iiter)->indexType();
			
			// 删除已经处理的，剩下的就是要从数据库删除的index
			currDBKeys.erase(fiter);
			
			if(deleteIndex)
			{
				sql += fmt::format("DROP INDEX `{}`,", itemName);
				done = true;
			}
			else
			{
				iiter = indexs.erase(iiter);
				continue;
			}
		}

		std::string lengthinfos = "";
		if((*iiter)->type() == TABLE_ITEM_TYPE_BLOB || 
			(*iiter)->type() == TABLE_ITEM_TYPE_STRING ||
			 (*iiter)->type() == TABLE_ITEM_TYPE_UNICODE ||
			 (*iiter)->type() == TABLE_ITEM_TYPE_PYTHON)
		{
			if((*iiter)->pPropertyDescription()->getDatabaseLength() == 0)
			{
				ERROR_MSG(fmt::format("EntityTableMysql::syncIndexToDB(): INDEX({}) without a key length, *.def-><{}>-><DatabaseLength> ? </DatabaseLength>\n", 
					(*iiter)->itemName(), (*iiter)->itemName()));
			}
			else
			{
				lengthinfos = fmt::format("({})", (*iiter)->pPropertyDescription()->getDatabaseLength());
			}
		}

		sql += fmt::format("ADD {} {}({}{}),", (*iiter)->indexType(), itemName, itemName, lengthinfos);
		++iiter;
		done = true;
	}

	// 剩下的就是要从数据库删除的index
	KBEUnordered_map<std::string, std::string>::iterator dbkey_iter = currDBKeys.begin();
	for(; dbkey_iter != currDBKeys.end(); ++dbkey_iter)
	{
		sql += fmt::format("DROP INDEX `{}`,", dbkey_iter->first);
		done = true;		
	}
	
	// 没有需要修改或者添加的
	if(!done)
		return true;
	
	sql.erase(sql.end() - 1);
	
	try
	{
		bool ret = pdbi->query(sql.c_str(), sql.size(), true);
		if(!ret)
		{
			return false;
		}
	}
	catch(...)
	{
		return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool EntityTableMysql::syncToDB(DBInterface* pdbi)
{
	if(hasSync())
		return true;

	// DEBUG_MSG(fmt::format("EntityTableMysql::syncToDB(): {}.\n", tableName()));

	char sql_str[SQL_BUF];
	std::string exItems = "";

	if(this->isChild())
		exItems = ", " TABLE_PARENTID_CONST_STR " bigint(20) unsigned NOT NULL, INDEX(" TABLE_PARENTID_CONST_STR ")";

	kbe_snprintf(sql_str, SQL_BUF, "CREATE TABLE IF NOT EXISTS " ENTITY_TABLE_PERFIX "_%s "
			"(id bigint(20) unsigned AUTO_INCREMENT, PRIMARY KEY idKey (id)%s)"
		"ENGINE=" MYSQL_ENGINE_TYPE, 
		tableName(), exItems.c_str());

	try
	{
		bool ret = pdbi->query(sql_str, strlen(sql_str), false);
		if(!ret)
		{
			return false;
		}
	}
	catch(...)
	{
		ERROR_MSG(fmt::format("EntityTableMysql::syncToDB(): error({}: {})\n lastQuery: {}.\n", 
			pdbi->getlasterror(), pdbi->getstrerror(), static_cast<DBInterfaceMysql*>(pdbi)->lastquery()));

		return false;
	}

	DBInterfaceMysql::TABLE_FIELDS outs;
	static_cast<DBInterfaceMysql*>(pdbi)->getFields(outs, this->tableName());

	ALL_MYSQL_SET_FLAGS |= NOT_NULL_FLAG;
	sync_item_to_db(pdbi, "tinyint not null DEFAULT 0", this->tableName(), TABLE_ITEM_PERFIX"_" TABLE_AUTOLOAD_CONST_STR, 0, 
			FIELD_TYPE_TINY, NOT_NULL_FLAG, (void*)&outs, &sync_autoload_item_index);

	EntityTable::TABLEITEM_MAP::iterator iter = tableItems_.begin();
	for(; iter != tableItems_.end(); ++iter)
	{
		if(!iter->second->syncToDB(pdbi, (void*)&outs))
			return false;
	}

	std::vector<std::string> dbTableItemNames;

	std::string ttablename = ENTITY_TABLE_PERFIX"_";
	ttablename += tableName();

	pdbi->getTableItemNames(ttablename.c_str(), dbTableItemNames);

	// 检查是否有需要删除的表字段
	std::vector<std::string>::iterator iter0 = dbTableItemNames.begin();
	for(; iter0 != dbTableItemNames.end(); ++iter0)
	{
		std::string tname = (*iter0);
		
		if(tname == TABLE_ID_CONST_STR || 
			tname == TABLE_ITEM_PERFIX"_" TABLE_AUTOLOAD_CONST_STR || 
			tname == TABLE_PARENTID_CONST_STR)
		{
			continue;
		}

		EntityTable::TABLEITEM_MAP::iterator iter = tableItems_.begin();
		bool found = false;

		for(; iter != tableItems_.end(); ++iter)
		{
			if(iter->second->isSameKey(tname))
			{
				found = true;
				break;
			}
		}

		if(!found)
		{
			if(!pdbi->dropEntityTableItemFromDB(ttablename.c_str(), tname.c_str()))
				return false;
		}
	}

	// 同步表索引
	if(!syncIndexToDB(pdbi))
		return false;

	sync_ = true;
	return true;
}

//-------------------------------------------------------------------------------------
void EntityTableMysql::queryAutoLoadEntities(DBInterface* pdbi, ScriptDefModule* pModule, 
		ENTITY_ID start, ENTITY_ID end, std::vector<DBID>& outs)
{
	std::string sql = fmt::format("select id  from " ENTITY_TABLE_PERFIX "_{} where " TABLE_ITEM_PERFIX "_" TABLE_AUTOLOAD_CONST_STR "=1 limit {}, {};", 
		tableName(), start, (end - start));

	bool result = pdbi->query(sql, false);

	if (!result)
		return;


	MYSQL_RES * pResult = mysql_store_result(static_cast<DBInterfaceMysql*>(pdbi)->mysql());

	if(pResult)
	{
		MYSQL_ROW arow;

		while((arow = mysql_fetch_row(pResult)) != NULL)
		{
			DBID dbid;
			StringConv::str2value(dbid, arow[0]);
			outs.push_back(dbid);
		}

		mysql_free_result(pResult);
	}
}

//-------------------------------------------------------------------------------------
EntityTableItem* EntityTableMysql::createItem(std::string type, std::string defaultVal)
{
	if(type == "INT8")
	{
		try
		{
			int8 v;
			StringConv::str2value(v, defaultVal.c_str());
		}
		catch (...)
		{
			defaultVal = "";
		}

		if(defaultVal.size() == 0)
			defaultVal = "0";
		
		return new EntityTableItemMysql_DIGIT(type, fmt::format("tinyint(@DATALEN@) not null DEFAULT {}", defaultVal),
			4, NOT_NULL_FLAG, FIELD_TYPE_TINY);
	}
	else if(type == "INT16")
	{
		try
		{
			int16 v;
			StringConv::str2value(v, defaultVal.c_str());
		}
		catch (...)
		{
			defaultVal = "";
		}

		if(defaultVal.size() == 0)
			defaultVal = "0";

		return new EntityTableItemMysql_DIGIT(type, fmt::format("smallint(@DATALEN@) not null DEFAULT {}", defaultVal), 
			6, NOT_NULL_FLAG, FIELD_TYPE_SHORT);
	}
	else if(type == "INT32")
	{
		try
		{
			int32 v;
			StringConv::str2value(v, defaultVal.c_str());
		}
		catch (...)
		{
			defaultVal = "";
		}

		if(defaultVal.size() == 0)
			defaultVal = "0";

		return new EntityTableItemMysql_DIGIT(type, fmt::format("int(@DATALEN@) not null DEFAULT {}", defaultVal), 
			11, NOT_NULL_FLAG, FIELD_TYPE_LONG);
	}
	else if(type == "INT64")
	{
		try
		{
			int64 v;
			StringConv::str2value(v, defaultVal.c_str());
		}
		catch (...)
		{
			defaultVal = "";
		}

		if(defaultVal.size() == 0)
			defaultVal = "0";

		return new EntityTableItemMysql_DIGIT(type, fmt::format("bigint(@DATALEN@) not null DEFAULT {}", defaultVal), 
			20, NOT_NULL_FLAG, FIELD_TYPE_LONGLONG);
	}
	else if(type == "UINT8")
	{
		try
		{
			uint8 v;
			StringConv::str2value(v, defaultVal.c_str());
		}
		catch (...)
		{
			defaultVal = "";
		}

		if(defaultVal.size() == 0)
			defaultVal = "0";

		return new EntityTableItemMysql_DIGIT(type, fmt::format("tinyint(@DATALEN@) unsigned not null DEFAULT {}", defaultVal), 
			3, NOT_NULL_FLAG | UNSIGNED_FLAG, FIELD_TYPE_TINY);
	}
	else if(type == "UINT16")
	{
		try
		{
			uint16 v;
			StringConv::str2value(v, defaultVal.c_str());
		}
		catch (...)
		{
			defaultVal = "";
		}

		if(defaultVal.size() == 0)
			defaultVal = "0";

		return new EntityTableItemMysql_DIGIT(type, fmt::format("smallint(@DATALEN@) unsigned not null DEFAULT {}", defaultVal), 
			5, NOT_NULL_FLAG | UNSIGNED_FLAG, FIELD_TYPE_SHORT);
	}
	else if(type == "UINT32")
	{
		try
		{
			uint32 v;
			StringConv::str2value(v, defaultVal.c_str());
		}
		catch (...)
		{
			defaultVal = "";
		}

		if(defaultVal.size() == 0)
			defaultVal = "0";

		return new EntityTableItemMysql_DIGIT(type, fmt::format("int(@DATALEN@) unsigned not null DEFAULT {}", defaultVal),
			10, NOT_NULL_FLAG | UNSIGNED_FLAG, FIELD_TYPE_LONG);
	}
	else if(type == "UINT64")
	{
		try
		{
			uint64 v;
			StringConv::str2value(v, defaultVal.c_str());
		}
		catch (...)
		{
			defaultVal = "";
		}

		if(defaultVal.size() == 0)
			defaultVal = "0";

		return new EntityTableItemMysql_DIGIT(type, fmt::format("bigint(@DATALEN@) unsigned not null DEFAULT {}", defaultVal), 
			20, NOT_NULL_FLAG | UNSIGNED_FLAG, FIELD_TYPE_LONGLONG);
	}
	else if(type == "FLOAT")
	{
		try
		{
			float v;
			StringConv::str2value(v, defaultVal.c_str());
		}
		catch (...)
		{
			defaultVal = "";
		}

		if(defaultVal.size() == 0)
			defaultVal = "0";

		return new EntityTableItemMysql_DIGIT(type, fmt::format("float(@DATALEN@) not null DEFAULT {}", defaultVal), 
			0, NOT_NULL_FLAG, FIELD_TYPE_FLOAT);
	}
	else if(type == "DOUBLE")
	{
		try
		{
			double v;
			StringConv::str2value(v, defaultVal.c_str());
		}
		catch (...)
		{
			defaultVal = "";
		}

		if(defaultVal.size() == 0)
			defaultVal = "0";

		return new EntityTableItemMysql_DIGIT(type, fmt::format("double(@DATALEN@) not null DEFAULT {}", defaultVal), 
			0, NOT_NULL_FLAG, FIELD_TYPE_DOUBLE);
	}
	else if(type == "STRING")
	{
		return new EntityTableItemMysql_STRING(fmt::format("varchar(@DATALEN@) not null DEFAULT '{}'", defaultVal), 
			0, NOT_NULL_FLAG | BINARY_FLAG, FIELD_TYPE_VAR_STRING);
	}
	else if(type == "UNICODE")
	{
		return new EntityTableItemMysql_UNICODE(fmt::format("varchar(@DATALEN@) not null DEFAULT '{}'", defaultVal), 
			0, NOT_NULL_FLAG | BINARY_FLAG, FIELD_TYPE_VAR_STRING);
	}
	else if(type == "PYTHON")
	{
		return new EntityTableItemMysql_PYTHON("blob", 0, BINARY_FLAG | BLOB_FLAG, FIELD_TYPE_BLOB);
	}
	else if(type == "PY_DICT")
	{
		return new EntityTableItemMysql_PYTHON("blob", 0, BINARY_FLAG | BLOB_FLAG, FIELD_TYPE_BLOB);
	}
	else if(type == "PY_TUPLE")
	{
		return new EntityTableItemMysql_PYTHON("blob", 0, BINARY_FLAG | BLOB_FLAG, FIELD_TYPE_BLOB);
	}
	else if(type == "PY_LIST")
	{
		return new EntityTableItemMysql_PYTHON("blob", 0, BINARY_FLAG | BLOB_FLAG, FIELD_TYPE_BLOB);
	}
	else if(type == "BLOB")
	{
		return new EntityTableItemMysql_BLOB("blob", 0, BINARY_FLAG | BLOB_FLAG, FIELD_TYPE_BLOB);
	}
	else if(type == "ARRAY")
	{
		return new EntityTableItemMysql_ARRAY("", 0, BINARY_FLAG | BLOB_FLAG, FIELD_TYPE_BLOB);
	}
	else if(type == "FIXED_DICT")
	{
		return new EntityTableItemMysql_FIXED_DICT("", 0, BINARY_FLAG | BLOB_FLAG, FIELD_TYPE_BLOB);
	}
#ifdef CLIENT_NO_FLOAT
	else if(type == "VECTOR2")
	{
		return new EntityTableItemMysql_VECTOR2("int not null DEFAULT 0", 0, NOT_NULL_FLAG, FIELD_TYPE_LONG);
	}
	else if(type == "VECTOR3")
	{
		return new EntityTableItemMysql_VECTOR3("int not null DEFAULT 0", 0, NOT_NULL_FLAG, FIELD_TYPE_LONG);
	}
	else if(type == "VECTOR4")
	{
		return new EntityTableItemMysql_VECTOR4("int not null DEFAULT 0", 0, NOT_NULL_FLAG, FIELD_TYPE_LONG);
	}
#else
	else if(type == "VECTOR2")
	{
		return new EntityTableItemMysql_VECTOR2("float not null DEFAULT 0", 0, NOT_NULL_FLAG, FIELD_TYPE_FLOAT);
	}
	else if(type == "VECTOR3")
	{
		return new EntityTableItemMysql_VECTOR3("float not null DEFAULT 0", 0, NOT_NULL_FLAG, FIELD_TYPE_FLOAT);
	}
	else if(type == "VECTOR4")
	{
		return new EntityTableItemMysql_VECTOR4("float not null DEFAULT 0", 0, NOT_NULL_FLAG, FIELD_TYPE_FLOAT);
	}
#endif
	else if(type == "ENTITYCALL")
	{
		return new EntityTableItemMysql_ENTITYCALL("blob", 0, BINARY_FLAG | BLOB_FLAG, FIELD_TYPE_BLOB);
	}
	else if (type == "ENTITY_COMPONENT")
	{
		return new EntityTableItemMysql_Component("blob", 0, BINARY_FLAG | BLOB_FLAG, FIELD_TYPE_BLOB);
	}

	KBE_ASSERT(false && "not found type.\n");
	return new EntityTableItemMysql_STRING("", 0, 0, FIELD_TYPE_STRING);
}

//-------------------------------------------------------------------------------------
void EntityTableMysql::entityShouldAutoLoad(DBInterface* pdbi, DBID dbid, bool shouldAutoLoad)
{
	if(dbid == 0)
		return;

	std::string sql = fmt::format("update " ENTITY_TABLE_PERFIX "_{} set " TABLE_ITEM_PERFIX "_" TABLE_AUTOLOAD_CONST_STR "={} where id={};", 
		tableName(), (shouldAutoLoad ? 1 : 0), dbid);

	pdbi->query(sql, false);
}

//-------------------------------------------------------------------------------------
DBID EntityTableMysql::writeTable(DBInterface* pdbi, DBID dbid, int8 shouldAutoLoad, MemoryStream* s, ScriptDefModule* pModule)
{
	mysql::DBContext context;
	context.parentTableName = "";
	context.parentTableDBID = 0;
	context.dbid = dbid;
	context.tableName = pModule->getName();
	context.isEmpty = false;

	while(s->length() > 0)
	{
		ENTITY_PROPERTY_UID pid;
		ENTITY_PROPERTY_UID child_pid;
		(*s) >> pid >> child_pid;
		
		EntityTableItem* pTableItem = this->findItem(child_pid);
		if(pTableItem == NULL)
		{
			ERROR_MSG(fmt::format("EntityTable::writeTable: not found item[{}].\n", child_pid));
			return dbid;
		}
		
		static_cast<EntityTableItemMysqlBase*>(pTableItem)->getWriteSqlItem(pdbi, s, context);
	};

	if(!WriteEntityHelper::writeDB(context.dbid > 0 ? TABLE_OP_UPDATE : TABLE_OP_INSERT, 
		pdbi, context))
		return 0;

	dbid = context.dbid;

	// 如果dbid为0则存储失败返回
	if(dbid <= 0)
		return dbid;

	// 设置实体是否自动加载
	if(shouldAutoLoad > -1)
		entityShouldAutoLoad(pdbi, dbid, shouldAutoLoad > 0);

	return dbid;
}

//-------------------------------------------------------------------------------------
bool EntityTableMysql::removeEntity(DBInterface* pdbi, DBID dbid, ScriptDefModule* pModule)
{
	KBE_ASSERT(pModule && dbid > 0);

	mysql::DBContext context;
	context.parentTableName = "";
	context.parentTableDBID = 0;
	context.dbid = dbid;
	context.tableName = pModule->getName();
	context.isEmpty = false;

	std::vector<EntityTableItem*>::iterator iter = tableFixedOrderItems_.begin();
	for(; iter != tableFixedOrderItems_.end(); ++iter)
	{
		static_cast<EntityTableItemMysqlBase*>((*iter))->getReadSqlItem(context);
	}

	bool ret = RemoveEntityHelper::removeDB(pdbi, context);
	KBE_ASSERT(ret);

	return true;
}

//-------------------------------------------------------------------------------------
bool EntityTableMysql::queryTable(DBInterface* pdbi, DBID dbid, MemoryStream* s, ScriptDefModule* pModule)
{
	KBE_ASSERT(pModule && s && dbid > 0);

	mysql::DBContext context;
	context.parentTableName = "";
	context.parentTableDBID = 0;
	context.dbid = dbid;
	context.tableName = pModule->getName();
	context.isEmpty = false;

	std::vector<EntityTableItem*>::iterator iter = tableFixedOrderItems_.begin();
	for(; iter != tableFixedOrderItems_.end(); ++iter)
	{
		static_cast<EntityTableItemMysqlBase*>((*iter))->getReadSqlItem(context);
	}

	if(!ReadEntityHelper::queryDB(pdbi, context))
		return false;

	if(context.dbids[dbid].size() == 0)
		return false;

	iter = tableFixedOrderItems_.begin();
	for(; iter != tableFixedOrderItems_.end(); ++iter)
	{
		static_cast<EntityTableItemMysqlBase*>((*iter))->addToStream(s, context, dbid);
	}

	return context.dbid == dbid;
}

//-------------------------------------------------------------------------------------
void EntityTableMysql::addToStream(MemoryStream* s, mysql::DBContext& context, DBID resultDBID)
{
	std::vector<EntityTableItem*>::iterator iter = tableFixedOrderItems_.begin();
	for(; iter != tableFixedOrderItems_.end(); ++iter)
	{
		static_cast<EntityTableItemMysqlBase*>((*iter))->addToStream(s, context, resultDBID);
	}
}

//-------------------------------------------------------------------------------------
void EntityTableMysql::getWriteSqlItem(DBInterface* pdbi, MemoryStream* s, mysql::DBContext& context)
{
	if(tableFixedOrderItems_.size() == 0)
		return;
	
	std::vector<EntityTableItem*>::iterator iter = tableFixedOrderItems_.begin();

	mysql::DBContext* context1 = new mysql::DBContext();
	context1->parentTableName = (*iter)->pParentTable()->tableName();
	context1->tableName = (*iter)->tableName();
	context1->parentTableDBID = 0;
	context1->dbid = 0;
	context1->isEmpty = (s == NULL);

	KBEShared_ptr< mysql::DBContext > opTableValBox1Ptr(context1);
	context.optable.push_back(std::pair<std::string/*tableName*/, KBEShared_ptr< mysql::DBContext > >
		((*iter)->tableName(), opTableValBox1Ptr));

	for(; iter != tableFixedOrderItems_.end(); ++iter)
	{
		static_cast<EntityTableItemMysqlBase*>((*iter))->getWriteSqlItem(pdbi, s, *context1);
	}
}

//-------------------------------------------------------------------------------------
void EntityTableMysql::getReadSqlItem(mysql::DBContext& context)
{
	if(tableFixedOrderItems_.size() == 0)
		return;
	
	std::vector<EntityTableItem*>::iterator iter = tableFixedOrderItems_.begin();

	mysql::DBContext* context1 = new mysql::DBContext();
	context1->parentTableName = (*iter)->pParentTable()->tableName();
	context1->tableName = (*iter)->tableName();
	context1->parentTableDBID = 0;
	context1->dbid = 0;
	context1->isEmpty = true;

	KBEShared_ptr< mysql::DBContext > opTableValBox1Ptr(context1);
	context.optable.push_back(std::pair<std::string/*tableName*/, KBEShared_ptr< mysql::DBContext > >
		((*iter)->tableName(), opTableValBox1Ptr));

	for(; iter != tableFixedOrderItems_.end(); ++iter)
	{
		static_cast<EntityTableItemMysqlBase*>((*iter))->getReadSqlItem(*context1);
	}
}

//-------------------------------------------------------------------------------------
void EntityTableItemMysqlBase::init_db_item_name(const char* exstrFlag)
{
	kbe_snprintf(db_item_name_, MAX_BUF, TABLE_ITEM_PERFIX"_%s%s", exstrFlag, itemName());
}

//-------------------------------------------------------------------------------------
bool EntityTableItemMysql_VECTOR2::isSameKey(std::string key)
{
	for(int i=0; i<2; ++i)
	{
		if(key == db_item_names_[i])
			return true;
	}

	return false;
}

//-------------------------------------------------------------------------------------
bool EntityTableItemMysql_VECTOR2::syncToDB(DBInterface* pdbi, void* pData)
{
	if(!sync_item_to_db(pdbi, itemDBType_.c_str(), tableName_.c_str(), db_item_names_[0], 0, 
		this->mysqlItemtype_, this->flags(), pData))
		return false;

	return sync_item_to_db(pdbi, itemDBType_.c_str(), tableName_.c_str(), db_item_names_[1], 0, 
		this->mysqlItemtype_, this->flags(), pData);
}

//-------------------------------------------------------------------------------------
void EntityTableItemMysql_VECTOR2::addToStream(MemoryStream* s, mysql::DBContext& context, DBID resultDBID)
{
	for(ArraySize i = 0; i < 2; ++i)
	{
		std::pair< std::vector<std::string>::size_type, std::vector<std::string> >& resultDatas = context.results[resultDBID];

#ifdef CLIENT_NO_FLOAT
		int32 v = atoi(resultDatas.second[resultDatas.first++].c_str());

#else
		float v = (float)atof(resultDatas.second[resultDatas.first++].c_str());

#endif
		(*s) << v;
	}
}

//-------------------------------------------------------------------------------------
void EntityTableItemMysql_VECTOR2::getWriteSqlItem(DBInterface* pdbi, MemoryStream* s, mysql::DBContext& context)
{
	if(s == NULL)
		return;

#ifdef CLIENT_NO_FLOAT
	int32 v;
#else
	float v;
#endif

	for(ArraySize i=0; i<2; ++i)
	{
		(*s) >> v;
		mysql::DBContext::DB_ITEM_DATA* pSotvs = new mysql::DBContext::DB_ITEM_DATA();
		pSotvs->sqlkey = db_item_names_[i];

#ifdef CLIENT_NO_FLOAT
		kbe_snprintf(pSotvs->sqlval, MAX_BUF, "%d", v);
#else
		kbe_snprintf(pSotvs->sqlval, MAX_BUF, "%f", v);
#endif
		
		context.items.push_back(KBEShared_ptr<mysql::DBContext::DB_ITEM_DATA>(pSotvs));
	}
}

//-------------------------------------------------------------------------------------
void EntityTableItemMysql_VECTOR2::getReadSqlItem(mysql::DBContext& context)
{
	for(ArraySize i=0; i<2; ++i)
	{
		mysql::DBContext::DB_ITEM_DATA* pSotvs = new mysql::DBContext::DB_ITEM_DATA();
		pSotvs->sqlkey = db_item_names_[i];
		memset(pSotvs->sqlval, 0, MAX_BUF);
		context.items.push_back(KBEShared_ptr<mysql::DBContext::DB_ITEM_DATA>(pSotvs));
	}
}

//-------------------------------------------------------------------------------------
bool EntityTableItemMysql_VECTOR3::isSameKey(std::string key)
{
	for(int i=0; i<3; ++i)
	{
		if(key == db_item_names_[i])
			return true;
	}

	return false;
}

//-------------------------------------------------------------------------------------
bool EntityTableItemMysql_VECTOR3::syncToDB(DBInterface* pdbi, void* pData)
{
	if(!sync_item_to_db(pdbi, itemDBType_.c_str(), tableName_.c_str(), db_item_names_[0], 0, 
		this->mysqlItemtype_, this->flags(), pData))
		return false;

	if(!sync_item_to_db(pdbi, itemDBType_.c_str(), tableName_.c_str(), db_item_names_[1], 0, 
		this->mysqlItemtype_, this->flags(), pData))
		return false;

	return sync_item_to_db(pdbi, itemDBType_.c_str(), tableName_.c_str(), db_item_names_[2], 0, 
		this->mysqlItemtype_, this->flags(), pData);
}

//-------------------------------------------------------------------------------------
void EntityTableItemMysql_VECTOR3::addToStream(MemoryStream* s, mysql::DBContext& context, DBID resultDBID)
{
	for(ArraySize i = 0; i < 3; ++i)
	{
		std::pair< std::vector<std::string>::size_type, std::vector<std::string> >& resultDatas = context.results[resultDBID];

#ifdef CLIENT_NO_FLOAT
		int32 v = atoi(resultDatas.second[resultDatas.first++].c_str());
#else
		float v = (float)atof(resultDatas.second[resultDatas.first++].c_str());
#endif
		(*s) << v;
	}
}

//-------------------------------------------------------------------------------------
void EntityTableItemMysql_VECTOR3::getWriteSqlItem(DBInterface* pdbi, MemoryStream* s, mysql::DBContext& context)
{
	if(s == NULL)
		return;

#ifdef CLIENT_NO_FLOAT
	int32 v;
#else
	float v;
#endif

	for(ArraySize i=0; i<3; ++i)
	{
		(*s) >> v;
		mysql::DBContext::DB_ITEM_DATA* pSotvs = new mysql::DBContext::DB_ITEM_DATA();
		pSotvs->sqlkey = db_item_names_[i];

#ifdef CLIENT_NO_FLOAT
		kbe_snprintf(pSotvs->sqlval, MAX_BUF, "%d", v);
#else
		kbe_snprintf(pSotvs->sqlval, MAX_BUF, "%f", v);
#endif

		context.items.push_back(KBEShared_ptr<mysql::DBContext::DB_ITEM_DATA>(pSotvs));
	}
}

//-------------------------------------------------------------------------------------
void EntityTableItemMysql_VECTOR3::getReadSqlItem(mysql::DBContext& context)
{
	for(ArraySize i=0; i<3; ++i)
	{
		mysql::DBContext::DB_ITEM_DATA* pSotvs = new mysql::DBContext::DB_ITEM_DATA();
		pSotvs->sqlkey = db_item_names_[i];
		memset(pSotvs->sqlval, 0, MAX_BUF);
		context.items.push_back(KBEShared_ptr<mysql::DBContext::DB_ITEM_DATA>(pSotvs));
	}
}

//-------------------------------------------------------------------------------------
bool EntityTableItemMysql_VECTOR4::isSameKey(std::string key)
{
	for(int i=0; i<4; ++i)
	{
		if(key == db_item_names_[i])
			return true;
	}

	return false;
}

//-------------------------------------------------------------------------------------
bool EntityTableItemMysql_VECTOR4::syncToDB(DBInterface* pdbi, void* pData)
{
	if(!sync_item_to_db(pdbi, itemDBType_.c_str(), tableName_.c_str(), db_item_names_[0], 0, this->mysqlItemtype_, this->flags(), pData))
		return false;

	if(!sync_item_to_db(pdbi, itemDBType_.c_str(), tableName_.c_str(), db_item_names_[1], 0, this->mysqlItemtype_, this->flags(), pData))
		return false;

	if(!sync_item_to_db(pdbi, itemDBType_.c_str(), tableName_.c_str(), db_item_names_[2], 0, this->mysqlItemtype_, this->flags(), pData))
		return false;

	return sync_item_to_db(pdbi, itemDBType_.c_str(), tableName_.c_str(), db_item_names_[3], 0, this->mysqlItemtype_, this->flags(), pData);
}

//-------------------------------------------------------------------------------------
void EntityTableItemMysql_VECTOR4::addToStream(MemoryStream* s, mysql::DBContext& context, DBID resultDBID)
{
	for(ArraySize i = 0; i < 4; ++i)
	{
		std::pair< std::vector<std::string>::size_type, std::vector<std::string> >& resultDatas = context.results[resultDBID];

#ifdef CLIENT_NO_FLOAT
		int32 v = atoi(resultDatas.second[resultDatas.first++].c_str());
#else
		float v = (float)atof(resultDatas.second[resultDatas.first++].c_str());
#endif
		(*s) << v;
	}
}

//-------------------------------------------------------------------------------------
void EntityTableItemMysql_VECTOR4::getWriteSqlItem(DBInterface* pdbi, MemoryStream* s, mysql::DBContext& context)
{
	if(s == NULL)
		return;

#ifdef CLIENT_NO_FLOAT
	int32 v;
#else
	float v;
#endif

	for(ArraySize i=0; i<4; ++i)
	{
		(*s) >> v;
		mysql::DBContext::DB_ITEM_DATA* pSotvs = new mysql::DBContext::DB_ITEM_DATA();
		pSotvs->sqlkey = db_item_names_[i];

#ifdef CLIENT_NO_FLOAT
		kbe_snprintf(pSotvs->sqlval, MAX_BUF, "%d", v);
#else
		kbe_snprintf(pSotvs->sqlval, MAX_BUF, "%f", v);
#endif

		context.items.push_back(KBEShared_ptr<mysql::DBContext::DB_ITEM_DATA>(pSotvs));
	}
}

//-------------------------------------------------------------------------------------
void EntityTableItemMysql_VECTOR4::getReadSqlItem(mysql::DBContext& context)
{
	for(ArraySize i=0; i<4; ++i)
	{
		mysql::DBContext::DB_ITEM_DATA* pSotvs = new mysql::DBContext::DB_ITEM_DATA();
		pSotvs->sqlkey = db_item_names_[i];
		memset(pSotvs->sqlval, 0, MAX_BUF);
		context.items.push_back(KBEShared_ptr<mysql::DBContext::DB_ITEM_DATA>(pSotvs));
	}
}

//-------------------------------------------------------------------------------------
bool EntityTableItemMysql_ENTITYCALL::syncToDB(DBInterface* pdbi, void* pData)
{
	return true;
}

//-------------------------------------------------------------------------------------
void EntityTableItemMysql_ENTITYCALL::addToStream(MemoryStream* s, mysql::DBContext& context, DBID resultDBID)
{
}

//-------------------------------------------------------------------------------------
void EntityTableItemMysql_ENTITYCALL::getWriteSqlItem(DBInterface* pdbi, MemoryStream* s, mysql::DBContext& context)
{
}

//-------------------------------------------------------------------------------------
void EntityTableItemMysql_ENTITYCALL::getReadSqlItem(mysql::DBContext& context)
{
}

//-------------------------------------------------------------------------------------
bool EntityTableItemMysql_ARRAY::isSameKey(std::string key)
{
	return false;
}

//-------------------------------------------------------------------------------------
bool EntityTableItemMysql_ARRAY::initialize(const PropertyDescription* pPropertyDescription, 
											const DataType* pDataType, std::string name)
{
	bool ret = EntityTableItemMysqlBase::initialize(pPropertyDescription, pDataType, name);
	if(!ret)
		return false;

	// 创建子表
	EntityTableMysql* pTable = new EntityTableMysql(this->pParentTable()->pEntityTables());

	std::string tname = this->pParentTable()->tableName();
	std::vector<std::string> qname;
	EntityTableItem* pparentItem = this->pParentTableItem();
	while(pparentItem != NULL)
	{
		if(strlen(pparentItem->itemName()) > 0)
			qname.push_back(pparentItem->itemName());
		pparentItem = pparentItem->pParentTableItem();
	}
	
	if(qname.size() > 0)
	{
		for(int i = (int)qname.size() - 1; i >= 0; i--)
		{
			tname += "_";
			tname += qname[i];
		}
	}
	
	std::string tableName = tname + "_";
	std::string itemName = "";

	if(name.size() > 0)
	{
		tableName += name;
	}
	else
	{
		tableName += TABLE_ARRAY_ITEM_VALUES_CONST_STR;
	}

	if(itemName.size() == 0)
	{
		if(static_cast<FixedArrayType*>(const_cast<DataType*>(pDataType))->getDataType()->type() != DATA_TYPE_FIXEDDICT)
			itemName = TABLE_ARRAY_ITEM_VALUE_CONST_STR;
	}

	pTable->tableName(tableName);
	pTable->isChild(true);

	EntityTableItem* pArrayTableItem;
	pArrayTableItem = pParentTable_->createItem(static_cast<FixedArrayType*>(const_cast<DataType*>(pDataType))->getDataType()->getName(), pPropertyDescription->getDefaultValStr());
	pArrayTableItem->utype(-pPropertyDescription->getUType());
	pArrayTableItem->pParentTable(this->pParentTable());
	pArrayTableItem->pParentTableItem(this);
	pArrayTableItem->tableName(pTable->tableName());

	ret = pArrayTableItem->initialize(pPropertyDescription, 
		static_cast<FixedArrayType*>(const_cast<DataType*>(pDataType))->getDataType(), itemName.c_str());

	if(!ret)
	{
		delete pTable;
		return ret;
	}

	pTable->addItem(pArrayTableItem);
	pChildTable_ = pTable;

	pTable->pEntityTables()->addTable(pTable);
	return true;
}

//-------------------------------------------------------------------------------------
bool EntityTableItemMysql_ARRAY::syncToDB(DBInterface* pdbi, void* pData)
{
	// 所有的表都会在bool EntityTables::syncToDB(DBInterface* pdbi)中被同步（包括子表），因此无需再做一次同步
	//if(pChildTable_)
	//	return pChildTable_->syncToDB(pdbi);

	return true;
}

//-------------------------------------------------------------------------------------
void EntityTableItemMysql_ARRAY::addToStream(MemoryStream* s, mysql::DBContext& context, DBID resultDBID)
{
	if(pChildTable_)
	{
		mysql::DBContext::DB_RW_CONTEXTS::iterator iter = context.optable.begin();
		for(; iter != context.optable.end(); ++iter)
		{
			if(pChildTable_->tableName() == iter->first)
			{
				ArraySize size = (ArraySize)iter->second->dbids[resultDBID].size();
				(*s) << size;

				for(ArraySize i=0; i<size; ++i)
					static_cast<EntityTableMysql*>(pChildTable_)->addToStream(s, *iter->second.get(), iter->second->dbids[resultDBID][i]);

				return;
			}
		}
	}

	ArraySize size = 0;
	(*s) << size;
}

//-------------------------------------------------------------------------------------
void EntityTableItemMysql_ARRAY::getWriteSqlItem(DBInterface* pdbi, MemoryStream* s, mysql::DBContext& context)
{
	ArraySize size = 0;
	if(s)
		(*s) >> size;

	if(pChildTable_)
	{
		if(size > 0)
		{
			for(ArraySize i=0; i<size; ++i)
				static_cast<EntityTableMysql*>(pChildTable_)->getWriteSqlItem(pdbi, s, context);
		}
		else
		{
			static_cast<EntityTableMysql*>(pChildTable_)->getWriteSqlItem(pdbi, NULL, context);
		}
	}
}

//-------------------------------------------------------------------------------------
void EntityTableItemMysql_ARRAY::getReadSqlItem(mysql::DBContext& context)
{
	if(pChildTable_)
	{
		static_cast<EntityTableMysql*>(pChildTable_)->getReadSqlItem(context);
	}
}

//-------------------------------------------------------------------------------------
void EntityTableItemMysql_ARRAY::init_db_item_name(const char* exstrFlag)
{
	if(pChildTable_)
	{
		static_cast<EntityTableMysql*>(pChildTable_)->init_db_item_name();
	}
}

//-------------------------------------------------------------------------------------
bool EntityTableItemMysql_FIXED_DICT::isSameKey(std::string key)
{
	FIXEDDICT_KEYTYPES::iterator fditer = keyTypes_.begin();
	bool tmpfound = false;

	for(; fditer != keyTypes_.end(); ++fditer)
	{
		if(fditer->second->isSameKey(key))
		{
			tmpfound = true;
			break;
		}
	}
	
	return tmpfound;
}

//-------------------------------------------------------------------------------------
bool EntityTableItemMysql_FIXED_DICT::initialize(const PropertyDescription* pPropertyDescription, 
												 const DataType* pDataType, std::string name)
{
	bool ret = EntityTableItemMysqlBase::initialize(pPropertyDescription, pDataType, name);
	if(!ret)
		return false;

	KBEngine::FixedDictType* fdatatype = static_cast<KBEngine::FixedDictType*>(const_cast<DataType*>(pDataType));

	FixedDictType::FIXEDDICT_KEYTYPE_MAP& keyTypes = fdatatype->getKeyTypes();
	FixedDictType::FIXEDDICT_KEYTYPE_MAP::iterator iter = keyTypes.begin();

	for(; iter != keyTypes.end(); ++iter)
	{
		if(!iter->second->persistent)
			continue;

		EntityTableItem* tableItem = pParentTable_->createItem(iter->second->dataType->getName(), pPropertyDescription->getDefaultValStr());

		tableItem->pParentTable(this->pParentTable());
		tableItem->pParentTableItem(this);
		tableItem->utype(-pPropertyDescription->getUType());
		tableItem->tableName(this->tableName());
		if(!tableItem->initialize(pPropertyDescription, iter->second->dataType, iter->first))
			return false;

		std::pair< std::string, KBEShared_ptr<EntityTableItem> > itemVal;
		itemVal.first = iter->first;
		itemVal.second.reset(tableItem);

		keyTypes_.push_back(itemVal);
	}

	return true;
}

//-------------------------------------------------------------------------------------
uint32 EntityTableItemMysql_FIXED_DICT::getItemDatabaseLength(const std::string& name)
{
	KBEngine::FixedDictType* fdatatype = static_cast<KBEngine::FixedDictType*>(const_cast<DataType*>(pDataType()));

	FixedDictType::FIXEDDICT_KEYTYPE_MAP& keyTypes = fdatatype->getKeyTypes();
	FixedDictType::FIXEDDICT_KEYTYPE_MAP::iterator iter = keyTypes.begin();

	for (; iter != keyTypes.end(); ++iter)
	{
		if (iter->first != name)
			continue;

		return iter->second->databaseLength;
	}

	return 0;
}

//-------------------------------------------------------------------------------------
bool EntityTableItemMysql_FIXED_DICT::syncToDB(DBInterface* pdbi, void* pData)
{
	EntityTableItemMysql_FIXED_DICT::FIXEDDICT_KEYTYPES::iterator iter = keyTypes_.begin();
	for(; iter != keyTypes_.end(); ++iter)
	{
		if(!iter->second->syncToDB(pdbi, pData))
			return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------
void EntityTableItemMysql_FIXED_DICT::addToStream(MemoryStream* s, mysql::DBContext& context, DBID resultDBID)
{
	FIXEDDICT_KEYTYPES::iterator fditer = keyTypes_.begin();

	for(; fditer != keyTypes_.end(); ++fditer)
	{
		static_cast<EntityTableItemMysqlBase*>(fditer->second.get())->addToStream(s, context, resultDBID);
	}
}

//-------------------------------------------------------------------------------------
void EntityTableItemMysql_FIXED_DICT::getWriteSqlItem(DBInterface* pdbi, MemoryStream* s, mysql::DBContext& context)
{
	FIXEDDICT_KEYTYPES::iterator fditer = keyTypes_.begin();

	for(; fditer != keyTypes_.end(); ++fditer)
	{
		static_cast<EntityTableItemMysqlBase*>(fditer->second.get())->getWriteSqlItem(pdbi, s, context);
	}
}

//-------------------------------------------------------------------------------------
void EntityTableItemMysql_FIXED_DICT::getReadSqlItem(mysql::DBContext& context)
{
	FIXEDDICT_KEYTYPES::iterator fditer = keyTypes_.begin();

	for(; fditer != keyTypes_.end(); ++fditer)
	{
		static_cast<EntityTableItemMysqlBase*>(fditer->second.get())->getReadSqlItem(context);
	}
}

//-------------------------------------------------------------------------------------
void EntityTableItemMysql_FIXED_DICT::init_db_item_name(const char* exstrFlag)
{
	FIXEDDICT_KEYTYPES::iterator fditer = keyTypes_.begin();

	for(; fditer != keyTypes_.end(); ++fditer)
	{
		std::string new_exstrFlag = exstrFlag;
		if(fditer->second->type()== TABLE_ITEM_TYPE_FIXEDDICT)
			new_exstrFlag += fditer->first + "_";

		static_cast<EntityTableItemMysqlBase*>(fditer->second.get())->init_db_item_name(new_exstrFlag.c_str());
	}
}

//-------------------------------------------------------------------------------------
bool EntityTableItemMysql_Component::isSameKey(std::string key)
{
	return false;
}

//-------------------------------------------------------------------------------------
bool EntityTableItemMysql_Component::initialize(const PropertyDescription* pPropertyDescription,
	const DataType* pDataType, std::string name)
{
	bool ret = EntityTableItemMysqlBase::initialize(pPropertyDescription, pDataType, name);
	if (!ret)
		return false;

	EntityComponentType* pEntityComponentType = const_cast<EntityComponentType*>(static_cast<const EntityComponentType*>(pDataType));
	ScriptDefModule* pEntityComponentScriptDefModule = pEntityComponentType->pScriptDefModule();

	EntityTableMysql* pparentTable = static_cast<EntityTableMysql*>(this->pParentTable());
	EntityTableMysql* pTable = new EntityTableMysql(pparentTable->pEntityTables());

	std::string tableName = std::string(pparentTable->tableName()) + "_" + name;

	pTable->tableName(tableName);
	pTable->isChild(true);

	ScriptDefModule* pScriptDefModule = EntityDef::findScriptModule(pparentTable->tableName(), false);

	ScriptDefModule::PROPERTYDESCRIPTION_MAP& pdescrsMap = pEntityComponentScriptDefModule->getPersistentPropertyDescriptions();
	ScriptDefModule::PROPERTYDESCRIPTION_MAP::const_iterator iter = pdescrsMap.begin();

	for (; iter != pdescrsMap.end(); ++iter)
	{
		PropertyDescription* pdescrs = iter->second;

		if (!pScriptDefModule->hasCell() && pdescrs->hasCell() && !pdescrs->hasBase())
		{
			continue;
		}

		EntityTableItem* pETItem = pparentTable->createItem(pdescrs->getDataType()->getName(), pdescrs->getDefaultValStr());

		pETItem->pParentTable(pparentTable);
		pETItem->utype(pdescrs->getUType());
		pETItem->tableName(pTable->tableName());
		pETItem->pParentTableItem(this);

		bool ret = pETItem->initialize(pdescrs, pdescrs->getDataType(), pdescrs->getName());

		if (!ret)
		{
			delete pETItem;
			return false;
		}

		pTable->addItem(pETItem);
	}

	pChildTable_ = pTable;
	pTable->pEntityTables()->addTable(pTable);
	return true;
}

//-------------------------------------------------------------------------------------
bool EntityTableItemMysql_Component::syncToDB(DBInterface* pdbi, void* pData)
{
	// 所有的表都会在bool EntityTables::syncToDB(DBInterface* pdbi)中被同步（包括子表），因此无需再做一次同步
	//if(pChildTable_)
	//	return pChildTable_->syncToDB(pdbi);
	return true;
}

//-------------------------------------------------------------------------------------
void EntityTableItemMysql_Component::addToStream(MemoryStream* s, mysql::DBContext& context, DBID resultDBID)
{
	if (pChildTable_)
	{
		mysql::DBContext::DB_RW_CONTEXTS::iterator iter = context.optable.begin();
		for (; iter != context.optable.end(); ++iter)
		{
			if (pChildTable_->tableName() == iter->first)
			{
				std::vector<DBID>& dbids = iter->second->dbids[resultDBID];

				// 如果一个实体已经存档，开发中又对实体加入了组件，那么数据库中此时是没有数据的，加载实体时需要对这样的情况做一些判断
				// 例如：实体加载时重新写入组件默认数据值
				bool foundData = dbids.size() > 0;
				(*s) << foundData;

				if (foundData)
				{
					// 理论上一定是不会大于1个的
					KBE_ASSERT(dbids.size() == 1);
					static_cast<EntityTableMysql*>(pChildTable_)->addToStream(s, *iter->second.get(), dbids[0]);
				}

				return;
			}
		}
	}
}

//-------------------------------------------------------------------------------------
void EntityTableItemMysql_Component::getWriteSqlItem(DBInterface* pdbi, MemoryStream* s, mysql::DBContext& context)
{
	if (pChildTable_)
	{
		static_cast<EntityTableMysql*>(pChildTable_)->getWriteSqlItem(pdbi, s, context);
	}
}

//-------------------------------------------------------------------------------------
void EntityTableItemMysql_Component::getReadSqlItem(mysql::DBContext& context)
{
	if (pChildTable_)
	{
		static_cast<EntityTableMysql*>(pChildTable_)->getReadSqlItem(context);
	}
}

//-------------------------------------------------------------------------------------
void EntityTableItemMysql_Component::init_db_item_name(const char* exstrFlag)
{
	if (pChildTable_)
	{
		static_cast<EntityTableMysql*>(pChildTable_)->init_db_item_name();
	}
}

//-------------------------------------------------------------------------------------
bool EntityTableItemMysqlBase::initialize(const PropertyDescription* pPropertyDescription, 
										  const DataType* pDataType, std::string name)
{
	itemName(name);

	pDataType_ = pDataType;
	pPropertyDescription_ = pPropertyDescription;
	indexType_ = pPropertyDescription->indexType();
	return true;
}

//-------------------------------------------------------------------------------------
bool EntityTableItemMysql_DIGIT::syncToDB(DBInterface* pdbi, void* pData)
{
	if(datalength_ == 0)
	{
		KBEngine::strutil::kbe_replace(itemDBType_, "(@DATALEN@)", "");
		return sync_item_to_db(pdbi, itemDBType_.c_str(), tableName_.c_str(), 
			db_item_name(), datalength_, this->mysqlItemtype_, this->flags(), pData);
	}

	uint32 length = pPropertyDescription_->getDatabaseLength();
	char sql_str[SQL_BUF];

	if (length <= 0)
	{
		KBEngine::strutil::kbe_replace(itemDBType_, "(@DATALEN@)", "");
		kbe_snprintf(sql_str, SQL_BUF, "%s", itemDBType_.c_str());
	}
	else
	{
		KBEngine::strutil::kbe_replace(itemDBType_, "@DATALEN@", fmt::format("{}", length).c_str());
		kbe_snprintf(sql_str, SQL_BUF, "%s", itemDBType_.c_str());
	}

	return sync_item_to_db(pdbi, sql_str, tableName_.c_str(), db_item_name(), length, this->mysqlItemtype_, this->flags(), pData);
}

//-------------------------------------------------------------------------------------
void EntityTableItemMysql_DIGIT::addToStream(MemoryStream* s, mysql::DBContext& context, DBID resultDBID)
{
	std::stringstream stream;

	std::pair< std::vector<std::string>::size_type, std::vector<std::string> >& resultDatas = context.results[resultDBID];
	stream << resultDatas.second[resultDatas.first++];

	if(dataSType_ == "INT8")
	{
		int32 v;
		stream >> v;
		int8 vv = static_cast<int8>(v);
		(*s) << vv;
	}
	else if(dataSType_ == "INT16")
	{
		int16 v;
		stream >> v;
		(*s) << v;
	}
	else if(dataSType_ == "INT32")
	{
		int32 v;
		stream >> v;
		(*s) << v;
	}
	else if(dataSType_ == "INT64")
	{
		int64 v;
		stream >> v;
		(*s) << v;
	}
	else if(dataSType_ == "UINT8")
	{
		int32 v;
		stream >> v;
		uint8 vv = static_cast<uint8>(v);
		(*s) << vv;
	}
	else if(dataSType_ == "UINT16")
	{
		uint16 v;
		stream >> v;
		(*s) << v;
	}
	else if(dataSType_ == "UINT32")
	{
		uint32 v;
		stream >> v;
		(*s) << v;
	}
	else if(dataSType_ == "UINT64")
	{
		uint64 v;
		stream >> v;
		(*s) << v;
	}
	else if(dataSType_ == "FLOAT")
	{
		float v;
		stream >> v;
		(*s) << v;
	}
	else if(dataSType_ == "DOUBLE")
	{
		double v;
		stream >> v;
		(*s) << v;
	}
}

//-------------------------------------------------------------------------------------
void EntityTableItemMysql_DIGIT::getWriteSqlItem(DBInterface* pdbi, MemoryStream* s, mysql::DBContext& context)
{
	if(s == NULL)
		return;

	mysql::DBContext::DB_ITEM_DATA* pSotvs = new mysql::DBContext::DB_ITEM_DATA();
	
	if(dataSType_ == "INT8")
	{
		int8 v;
		(*s) >> v;
		kbe_snprintf(pSotvs->sqlval, MAX_BUF, "%d", v);
	}
	else if(dataSType_ == "INT16")
	{
		int16 v;
		(*s) >> v;
		kbe_snprintf(pSotvs->sqlval, MAX_BUF, "%d", v);
	}
	else if(dataSType_ == "INT32")
	{
		int32 v;
		(*s) >> v;
		kbe_snprintf(pSotvs->sqlval, MAX_BUF, "%d", v);
	}
	else if(dataSType_ == "INT64")
	{
		int64 v;
		(*s) >> v;
		kbe_snprintf(pSotvs->sqlval, MAX_BUF, "%" PRI64, v);
	}
	else if(dataSType_ == "UINT8")
	{
		uint8 v;
		(*s) >> v;
		kbe_snprintf(pSotvs->sqlval, MAX_BUF, "%u", v);
	}
	else if(dataSType_ == "UINT16")
	{
		uint16 v;
		(*s) >> v;
		kbe_snprintf(pSotvs->sqlval, MAX_BUF, "%u", v);
	}
	else if(dataSType_ == "UINT32")
	{
		uint32 v;
		(*s) >> v;
		kbe_snprintf(pSotvs->sqlval, MAX_BUF, "%u", v);
	}
	else if(dataSType_ == "UINT64")
	{
		uint64 v;
		(*s) >> v;
		kbe_snprintf(pSotvs->sqlval, MAX_BUF, "%" PRIu64, v);
	}
	else if(dataSType_ == "FLOAT")
	{
		float v;
		(*s) >> v;
		kbe_snprintf(pSotvs->sqlval, MAX_BUF, "%f", v);
	}
	else if(dataSType_ == "DOUBLE")
	{
		double v;
		(*s) >> v;
		kbe_snprintf(pSotvs->sqlval, MAX_BUF, "%lf", v);
	}

	pSotvs->sqlkey = db_item_name();
	context.items.push_back(KBEShared_ptr<mysql::DBContext::DB_ITEM_DATA>(pSotvs));
}

//-------------------------------------------------------------------------------------
void EntityTableItemMysql_DIGIT::getReadSqlItem(mysql::DBContext& context)
{
	mysql::DBContext::DB_ITEM_DATA* pSotvs = new mysql::DBContext::DB_ITEM_DATA();
	pSotvs->sqlkey = db_item_name();
	memset(pSotvs->sqlval, 0, MAX_BUF);
	context.items.push_back(KBEShared_ptr<mysql::DBContext::DB_ITEM_DATA>(pSotvs));
}

//-------------------------------------------------------------------------------------
bool EntityTableItemMysql_STRING::syncToDB(DBInterface* pdbi, void* pData)
{
	uint32 length = pPropertyDescription_->getDatabaseLength();
	char sql_str[SQL_BUF];

	// 如果父表Item是个固定字典，那么需要判断当前item有无在固定字典中设置DatabaseLength
	if (this->pParentTableItem() && this->pParentTableItem()->type() == TABLE_ITEM_TYPE_FIXEDDICT)
	{
		length = static_cast<KBEngine::EntityTableItemMysql_FIXED_DICT*>(pParentTableItem())->getItemDatabaseLength(this->itemName());
	}

	if (length <= 0)
	{
		// 默认长度255
		length = 255;
	}

	KBEngine::strutil::kbe_replace(itemDBType_, "@DATALEN@", fmt::format("{}", length).c_str());
	kbe_snprintf(sql_str, SQL_BUF, "%s", itemDBType_.c_str());

	return sync_item_to_db(pdbi, sql_str, tableName_.c_str(), db_item_name(), length, 
		this->mysqlItemtype_, this->flags(), pData);
}

//-------------------------------------------------------------------------------------
void EntityTableItemMysql_STRING::addToStream(MemoryStream* s, 
										mysql::DBContext& context, DBID resultDBID)
{
	std::pair< std::vector<std::string>::size_type, std::vector<std::string> >& resultDatas = context.results[resultDBID];
	(*s) << resultDatas.second[resultDatas.first++];
}

//-------------------------------------------------------------------------------------
void EntityTableItemMysql_STRING::getWriteSqlItem(DBInterface* pdbi, 
										MemoryStream* s, mysql::DBContext& context)
{
	if(s == NULL)
		return;

	mysql::DBContext::DB_ITEM_DATA* pSotvs = new mysql::DBContext::DB_ITEM_DATA();

	std::string val;
	(*s) >> val;

	pSotvs->extraDatas = "\"";

	char* tbuf = new char[val.size() * 2 + 1];
	memset(tbuf, 0, val.size() * 2 + 1);

	mysql_real_escape_string(static_cast<DBInterfaceMysql*>(pdbi)->mysql(), 
		tbuf, val.c_str(), val.size());

	pSotvs->extraDatas += tbuf;
	pSotvs->extraDatas += "\"";

	SAFE_RELEASE_ARRAY(tbuf);

	memset(pSotvs->sqlval, 0, sizeof(pSotvs->sqlval));
	pSotvs->sqlkey = db_item_name();
	context.items.push_back(KBEShared_ptr<mysql::DBContext::DB_ITEM_DATA>(pSotvs));
}

//-------------------------------------------------------------------------------------
void EntityTableItemMysql_STRING::getReadSqlItem(mysql::DBContext& context)
{
	mysql::DBContext::DB_ITEM_DATA* pSotvs = new mysql::DBContext::DB_ITEM_DATA();
	pSotvs->sqlkey = db_item_name();
	memset(pSotvs->sqlval, 0, MAX_BUF);
	context.items.push_back(KBEShared_ptr<mysql::DBContext::DB_ITEM_DATA>(pSotvs));
}

//-------------------------------------------------------------------------------------
bool EntityTableItemMysql_UNICODE::syncToDB(DBInterface* pdbi, void* pData)
{
	uint32 length = pPropertyDescription_->getDatabaseLength();
	char sql_str[SQL_BUF];

	// 如果父表Item是个固定字典，那么需要判断当前item有无在固定字典中设置DatabaseLength
	if (this->pParentTableItem() && this->pParentTableItem()->type() == TABLE_ITEM_TYPE_FIXEDDICT)
	{
		length = static_cast<KBEngine::EntityTableItemMysql_FIXED_DICT*>(pParentTableItem())->getItemDatabaseLength(this->itemName());
	}

	if (length <= 0)
	{
		// 默认长度255
		length = 255;
	}

	KBEngine::strutil::kbe_replace(itemDBType_, "@DATALEN@", fmt::format("{}", length).c_str());
	kbe_snprintf(sql_str, SQL_BUF, "%s", itemDBType_.c_str());

	return sync_item_to_db(pdbi, sql_str, tableName_.c_str(), db_item_name(), length, 
		this->mysqlItemtype_, this->flags(), pData);
}

//-------------------------------------------------------------------------------------
void EntityTableItemMysql_UNICODE::addToStream(MemoryStream* s, 
								mysql::DBContext& context, DBID resultDBID)
{
	std::pair< std::vector<std::string>::size_type, std::vector<std::string> >& resultDatas = context.results[resultDBID];
	s->appendBlob(resultDatas.second[resultDatas.first++]);
}

//-------------------------------------------------------------------------------------
void EntityTableItemMysql_UNICODE::getWriteSqlItem(DBInterface* pdbi, MemoryStream* s, 
									mysql::DBContext& context)
{
	if(s == NULL)
		return;

	mysql::DBContext::DB_ITEM_DATA* pSotvs = new mysql::DBContext::DB_ITEM_DATA();

	std::string val;
	s->readBlob(val);

	char* tbuf = new char[val.size() * 2 + 1];
	memset(tbuf, 0, val.size() * 2 + 1);

	mysql_real_escape_string(static_cast<DBInterfaceMysql*>(pdbi)->mysql(), 
		tbuf, val.c_str(), val.size());
	
	pSotvs->extraDatas = "\"";
	pSotvs->extraDatas += tbuf;
	pSotvs->extraDatas += "\"";
	SAFE_RELEASE_ARRAY(tbuf);

	memset(pSotvs->sqlval, 0, sizeof(pSotvs->sqlval));
	pSotvs->sqlkey = db_item_name();
	context.items.push_back(KBEShared_ptr<mysql::DBContext::DB_ITEM_DATA>(pSotvs));
}

//-------------------------------------------------------------------------------------
void EntityTableItemMysql_UNICODE::getReadSqlItem(mysql::DBContext& context)
{
	mysql::DBContext::DB_ITEM_DATA* pSotvs = new mysql::DBContext::DB_ITEM_DATA();
	pSotvs->sqlkey = db_item_name();
	memset(pSotvs->sqlval, 0, MAX_BUF);
	context.items.push_back(KBEShared_ptr<mysql::DBContext::DB_ITEM_DATA>(pSotvs));
}

//-------------------------------------------------------------------------------------
bool EntityTableItemMysql_BLOB::syncToDB(DBInterface* pdbi, void* pData)
{
	return sync_item_to_db(pdbi, itemDBType_.c_str(), tableName_.c_str(), db_item_name(), 0, 
		this->mysqlItemtype_, this->flags(), pData);
}

//-------------------------------------------------------------------------------------
void EntityTableItemMysql_BLOB::addToStream(MemoryStream* s, mysql::DBContext& context, DBID resultDBID)
{
	std::pair< std::vector<std::string>::size_type, std::vector<std::string> >& resultDatas = context.results[resultDBID];
	std::string& datas = resultDatas.second[resultDatas.first++];

	s->appendBlob(datas.data(), datas.size());
}

//-------------------------------------------------------------------------------------
void EntityTableItemMysql_BLOB::getWriteSqlItem(DBInterface* pdbi, MemoryStream* s, mysql::DBContext& context)
{
	if(s == NULL)
		return;

	mysql::DBContext::DB_ITEM_DATA* pSotvs = new mysql::DBContext::DB_ITEM_DATA();

	std::string val;
	s->readBlob(val);

	char* tbuf = new char[val.size() * 2 + 1];
	memset(tbuf, 0, val.size() * 2 + 1);

	mysql_real_escape_string(static_cast<DBInterfaceMysql*>(pdbi)->mysql(), 
		tbuf, val.data(), val.size());

	pSotvs->extraDatas = "\"";
	pSotvs->extraDatas += tbuf;
	pSotvs->extraDatas += "\"";
	SAFE_RELEASE_ARRAY(tbuf);

	memset(pSotvs->sqlval, 0, sizeof(pSotvs->sqlval));
	pSotvs->sqlkey = db_item_name();
	context.items.push_back(KBEShared_ptr<mysql::DBContext::DB_ITEM_DATA>(pSotvs));
}

//-------------------------------------------------------------------------------------
void EntityTableItemMysql_BLOB::getReadSqlItem(mysql::DBContext& context)
{
	mysql::DBContext::DB_ITEM_DATA* pSotvs = new mysql::DBContext::DB_ITEM_DATA();
	pSotvs->sqlkey = db_item_name();
	memset(pSotvs->sqlval, 0, MAX_BUF);
	context.items.push_back(KBEShared_ptr<mysql::DBContext::DB_ITEM_DATA>(pSotvs));
}

//-------------------------------------------------------------------------------------
bool EntityTableItemMysql_PYTHON::syncToDB(DBInterface* pdbi, void* pData)
{
	return sync_item_to_db(pdbi, itemDBType_.c_str(), tableName_.c_str(), db_item_name(), 0, 
		this->mysqlItemtype_, this->flags(), pData);
}

//-------------------------------------------------------------------------------------
void EntityTableItemMysql_PYTHON::addToStream(MemoryStream* s, mysql::DBContext& context, DBID resultDBID)
{
	std::pair< std::vector<std::string>::size_type, std::vector<std::string> >& resultDatas = context.results[resultDBID];
	std::string& datas = resultDatas.second[resultDatas.first++];

	(*s).appendBlob(datas);
}

//-------------------------------------------------------------------------------------
void EntityTableItemMysql_PYTHON::getWriteSqlItem(DBInterface* pdbi, MemoryStream* s, mysql::DBContext& context)
{
	if(s == NULL)
		return;

	mysql::DBContext::DB_ITEM_DATA* pSotvs = new mysql::DBContext::DB_ITEM_DATA();

	std::string val;
	s->readBlob(val);

	char* tbuf = new char[val.size() * 2 + 1];
	memset(tbuf, 0, val.size() * 2 + 1);

	mysql_real_escape_string(static_cast<DBInterfaceMysql*>(pdbi)->mysql(), 
		tbuf, val.c_str(), val.size());

	pSotvs->extraDatas = "\"";
	pSotvs->extraDatas += tbuf;
	pSotvs->extraDatas += "\"";
	SAFE_RELEASE_ARRAY(tbuf);

	memset(pSotvs->sqlval, 0, sizeof(pSotvs->sqlval));
	pSotvs->sqlkey = db_item_name();
	context.items.push_back(KBEShared_ptr<mysql::DBContext::DB_ITEM_DATA>(pSotvs));
}

//-------------------------------------------------------------------------------------
void EntityTableItemMysql_PYTHON::getReadSqlItem(mysql::DBContext& context)
{
	mysql::DBContext::DB_ITEM_DATA* pSotvs = new mysql::DBContext::DB_ITEM_DATA();
	pSotvs->sqlkey = db_item_name();
	memset(pSotvs->sqlval, 0, MAX_BUF);
	context.items.push_back(KBEShared_ptr<mysql::DBContext::DB_ITEM_DATA>(pSotvs));
}

//-------------------------------------------------------------------------------------
}
