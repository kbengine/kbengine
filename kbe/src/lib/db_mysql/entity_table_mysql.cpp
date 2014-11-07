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

#include "entity_table_mysql.hpp"
#include "kbe_table_mysql.hpp"
#include "read_entity_helper.hpp"
#include "write_entity_helper.hpp"
#include "remove_entity_helper.hpp"
#include "entitydef/scriptdef_module.hpp"
#include "entitydef/property.hpp"
#include "dbmgr_lib/db_interface.hpp"
#include "dbmgr_lib/entity_table.hpp"
#include "network/fixed_messages.hpp"

#ifndef CODE_INLINE
#include "entity_table_mysql.ipp"
#endif

namespace KBEngine { 


bool sync_item_to_db(DBInterface* dbi, 
					 const char* datatype, 
					 const char* tablename, 
					 const char* itemname, 
					 uint32 length, 
					 enum_field_types 
					 sqlitemtype, 
					 unsigned int itemflags, 
					 void* pData)	
{
	DBInterfaceMysql::TABLE_FIELDS* pTFData = static_cast<DBInterfaceMysql::TABLE_FIELDS*>(pData);
	DBInterfaceMysql::TABLE_FIELDS::iterator iter = pTFData->find(itemname);
	if(iter != pTFData->end())
	{
		TABLE_FIELD& tf = iter->second;
		if(tf.type == sqlitemtype && ((tf.flags & itemflags) == itemflags))
		{
			if((length == 0) || (int32)length == tf.length)
				return true;
		}
	}

	DEBUG_MSG(fmt::format("syncToDB(): {}->{}({}).\n", tablename, itemname, datatype));

	char __sql_str__[MAX_BUF];

	kbe_snprintf(__sql_str__, MAX_BUF, "ALTER TABLE `"ENTITY_TABLE_PERFIX"_%s` ADD `%s` %s;",
		tablename, itemname, datatype);	

	try
	{
		dbi->query(__sql_str__, strlen(__sql_str__), false);	
	}
	catch(...)
	{
	}	

	if(dbi->getlasterror() == 1060)	
	{
		kbe_snprintf(__sql_str__, MAX_BUF, "ALTER TABLE `"ENTITY_TABLE_PERFIX"_%s` MODIFY COLUMN `%s` %s;",	
			tablename, itemname, datatype);

		try
		{
			if(dbi->query(__sql_str__, strlen(__sql_str__), false))
				return true;
		}
		catch(...)
		{
			ERROR_MSG(fmt::format("syncToDB(): {}->{}({}) is error({}: {})\n lastQuery: {}.\n", 
				tablename, itemname, datatype, dbi->getlasterror(), dbi->getstrerror(), static_cast<DBInterfaceMysql*>(dbi)->lastquery()));

			return false;
		}
	}

	return true;
}		

//-------------------------------------------------------------------------------------
EntityTableMysql::EntityTableMysql()
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

	for(; iter != pdescrsMap.end(); iter++)
	{
		PropertyDescription* pdescrs = iter->second;

		EntityTableItem* pETItem = this->createItem(pdescrs->getDataType()->getName());

		pETItem->pParentTable(this);
		pETItem->utype(pdescrs->getUType());
		pETItem->tableName(this->tableName());

		bool ret = pETItem->initialize(pdescrs, pdescrs->getDataType(), pdescrs->getName());
		
		if(!ret)
		{
			delete pETItem;
			return false;
		}
		
		tableItems_[pETItem->utype()].reset(pETItem);
		tableFixedOrderItems_.push_back(pETItem);
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

		EntityTableItem* pETItem = this->createItem("VECTOR3");
		pETItem->pParentTable(this);
		pETItem->utype(posuid);
		pETItem->tableName(this->tableName());
		pETItem->itemName("position");
		tableItems_[pETItem->utype()].reset(pETItem);
		tableFixedOrderItems_.push_back(pETItem);

		pETItem = this->createItem("VECTOR3");
		pETItem->pParentTable(this);
		pETItem->utype(diruid);
		pETItem->tableName(this->tableName());
		pETItem->itemName("direction");
		tableItems_[pETItem->utype()].reset(pETItem);
		tableFixedOrderItems_.push_back(pETItem);
	}

	init_db_item_name();
	return true;
}

//-------------------------------------------------------------------------------------
void EntityTableMysql::init_db_item_name()
{
	EntityTable::TABLEITEM_MAP::iterator iter = tableItems_.begin();
	for(; iter != tableItems_.end(); iter++)
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
bool EntityTableMysql::syncToDB(DBInterface* dbi)
{
	if(hasSync())
		return true;

	// DEBUG_MSG(fmt::format("EntityTableMysql::syncToDB(): {}.\n", tableName()));

	char sql_str[MAX_BUF];
	std::string exItems = "";

	if(this->isChild())
		exItems = ", "TABLE_PARENTID_CONST_STR" bigint(20) NOT NULL, INDEX("TABLE_PARENTID_CONST_STR")";

	kbe_snprintf(sql_str, MAX_BUF, "CREATE TABLE IF NOT EXISTS "ENTITY_TABLE_PERFIX"_%s "
			"(id bigint(20) AUTO_INCREMENT, PRIMARY KEY idKey (id)%s)"
		"ENGINE="MYSQL_ENGINE_TYPE, 
		tableName(), exItems.c_str());

	try
	{
		bool ret = dbi->query(sql_str, strlen(sql_str), false);
		if(!ret)
		{
			return false;
		}
	}
	catch(...)
	{
		return false;
	}

	DBInterfaceMysql::TABLE_FIELDS outs;
	static_cast<DBInterfaceMysql*>(dbi)->getFields(outs, this->tableName());

	EntityTable::TABLEITEM_MAP::iterator iter = tableItems_.begin();
	for(; iter != tableItems_.end(); iter++)
	{
		if(!iter->second->syncToDB(dbi, (void*)&outs))
			return false;
	}

	std::vector<std::string> dbTableItemNames;

	std::string ttablename = ENTITY_TABLE_PERFIX"_";
	ttablename += tableName();

	dbi->getTableItemNames(ttablename.c_str(), dbTableItemNames);

	// 检查是否有需要删除的表字段
	std::vector<std::string>::iterator iter0 = dbTableItemNames.begin();
	for(; iter0 != dbTableItemNames.end(); iter0++)
	{
		std::string tname = (*iter0);
		
		if(tname == "id" || tname == TABLE_PARENTID_CONST_STR)
		{
			continue;
		}

		EntityTable::TABLEITEM_MAP::iterator iter = tableItems_.begin();
		bool found = false;

		for(; iter != tableItems_.end(); iter++)
		{
			if(iter->second->isSameKey(tname))
			{
				found = true;
				break;
			}
		}

		if(!found)
		{
			if(!dbi->dropEntityTableItemFromDB(ttablename.c_str(), tname.c_str()))
				return false;
		}
	}

	sync_ = true;
	return true;
}

//-------------------------------------------------------------------------------------
EntityTableItem* EntityTableMysql::createItem(std::string type)
{
	if(type == "INT8")
	{
		return new EntityTableItemMysql_DIGIT(type, "tinyint not null DEFAULT 0", 4, NOT_NULL_FLAG, FIELD_TYPE_TINY);
	}
	else if(type == "INT16")
	{
		return new EntityTableItemMysql_DIGIT(type, "smallint not null DEFAULT 0", 6, NOT_NULL_FLAG, FIELD_TYPE_SHORT);
	}
	else if(type == "INT32")
	{
		return new EntityTableItemMysql_DIGIT(type, "int not null DEFAULT 0", 11, NOT_NULL_FLAG, FIELD_TYPE_LONG);
	}
	else if(type == "INT64")
	{
		return new EntityTableItemMysql_DIGIT(type, "bigint not null DEFAULT 0", 20, NOT_NULL_FLAG, FIELD_TYPE_LONGLONG);
	}
	else if(type == "UINT8")
	{
		return new EntityTableItemMysql_DIGIT(type, "tinyint unsigned not null DEFAULT 0", 3, NOT_NULL_FLAG|UNSIGNED_FLAG, FIELD_TYPE_TINY);
	}
	else if(type == "UINT16")
	{
		return new EntityTableItemMysql_DIGIT(type, "smallint unsigned not null DEFAULT 0", 5, NOT_NULL_FLAG|UNSIGNED_FLAG, FIELD_TYPE_SHORT);
	}
	else if(type == "UINT32")
	{
		return new EntityTableItemMysql_DIGIT(type, "int unsigned not null DEFAULT 0", 10, NOT_NULL_FLAG|UNSIGNED_FLAG, FIELD_TYPE_LONG);
	}
	else if(type == "UINT64")
	{
		return new EntityTableItemMysql_DIGIT(type, "bigint unsigned not null DEFAULT 0", 20, NOT_NULL_FLAG|UNSIGNED_FLAG, FIELD_TYPE_LONGLONG);
	}
	else if(type == "FLOAT")
	{
		return new EntityTableItemMysql_DIGIT(type, "float not null DEFAULT 0", 0, NOT_NULL_FLAG, FIELD_TYPE_FLOAT);
	}
	else if(type == "DOUBLE")
	{
		return new EntityTableItemMysql_DIGIT(type, "double not null DEFAULT 0", 0, NOT_NULL_FLAG, FIELD_TYPE_DOUBLE);
	}
	else if(type == "STRING")
	{
		return new EntityTableItemMysql_STRING("text", 0, 0, FIELD_TYPE_BLOB);
	}
	else if(type == "UNICODE")
	{
		return new EntityTableItemMysql_UNICODE("text", 0, 0, FIELD_TYPE_BLOB);
	}
	else if(type == "PYTHON")
	{
		return new EntityTableItemMysql_PYTHON("blob", 0, 0, FIELD_TYPE_BLOB);
	}
	else if(type == "PY_DICT")
	{
		return new EntityTableItemMysql_PYTHON("blob", 0, 0, FIELD_TYPE_BLOB);
	}
	else if(type == "PY_TUPLE")
	{
		return new EntityTableItemMysql_PYTHON("blob", 0, 0, FIELD_TYPE_BLOB);
	}
	else if(type == "PY_LIST")
	{
		return new EntityTableItemMysql_PYTHON("blob", 0, 0, FIELD_TYPE_BLOB);
	}
	else if(type == "BLOB")
	{
		return new EntityTableItemMysql_BLOB("blob", 0, 0, FIELD_TYPE_BLOB);
	}
	else if(type == "ARRAY")
	{
		return new EntityTableItemMysql_ARRAY("", 0, 0, FIELD_TYPE_BLOB);
	}
	else if(type == "FIXED_DICT")
	{
		return new EntityTableItemMysql_FIXED_DICT("", 0, 0, FIELD_TYPE_BLOB);
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
	else if(type == "MAILBOX")
	{
		return new EntityTableItemMysql_MAILBOX("blob", 0, 0, FIELD_TYPE_BLOB);
	}

	KBE_ASSERT(false && "not found type.\n");
	return new EntityTableItemMysql_STRING("", 0, 0, FIELD_TYPE_STRING);
}

//-------------------------------------------------------------------------------------
DBID EntityTableMysql::writeTable(DBInterface* dbi, DBID dbid, MemoryStream* s, ScriptDefModule* pModule)
{
	DB_OP_TABLE_ITEM_DATA_BOX opTableItemDataBox;
	opTableItemDataBox.parentTableName = "";
	opTableItemDataBox.parentTableDBID = 0;
	opTableItemDataBox.dbid = dbid;
	opTableItemDataBox.tableName = pModule->getName();
	opTableItemDataBox.isEmpty = false;
	opTableItemDataBox.readresultIdx = 0;

	while(s->opsize() > 0)
	{
		ENTITY_PROPERTY_UID pid;
		(*s) >> pid;
		
		EntityTableItem* pTableItem = this->findItem(pid);
		if(pTableItem == NULL)
		{
			ERROR_MSG(fmt::format("EntityTable::writeTable: not found item[{}].\n", pid));
			return dbid;
		}
		
		static_cast<EntityTableItemMysqlBase*>(pTableItem)->getWriteSqlItem(dbi, s, opTableItemDataBox);
	};

	if(!WriteEntityHelper::writeDB(opTableItemDataBox.dbid > 0 ? TABLE_OP_UPDATE : TABLE_OP_INSERT, 
		dbi, opTableItemDataBox))
		return 0;

	dbid = opTableItemDataBox.dbid;

	// 如果dbid为0则存储失败返回
	if(dbid <= 0)
		return dbid;

	return dbid;
}

//-------------------------------------------------------------------------------------
bool EntityTableMysql::removeEntity(DBInterface* dbi, DBID dbid, ScriptDefModule* pModule)
{
	KBE_ASSERT(pModule && dbid > 0);

	DB_OP_TABLE_ITEM_DATA_BOX opTableItemDataBox;
	opTableItemDataBox.parentTableName = "";
	opTableItemDataBox.parentTableDBID = 0;
	opTableItemDataBox.dbid = dbid;
	opTableItemDataBox.tableName = pModule->getName();
	opTableItemDataBox.isEmpty = false;
	opTableItemDataBox.readresultIdx = 0;

	std::vector<EntityTableItem*>::iterator iter = tableFixedOrderItems_.begin();
	for(; iter != tableFixedOrderItems_.end(); iter++)
	{
		static_cast<EntityTableItemMysqlBase*>((*iter))->getReadSqlItem(opTableItemDataBox);
	}

	bool ret = RemoveEntityHelper::removeDB(dbi, opTableItemDataBox);
	KBE_ASSERT(ret);

	return true;
}

//-------------------------------------------------------------------------------------
bool EntityTableMysql::queryTable(DBInterface* dbi, DBID dbid, MemoryStream* s, ScriptDefModule* pModule)
{
	KBE_ASSERT(pModule && s && dbid > 0);

	DB_OP_TABLE_ITEM_DATA_BOX opTableItemDataBox;
	opTableItemDataBox.parentTableName = "";
	opTableItemDataBox.parentTableDBID = 0;
	opTableItemDataBox.dbid = dbid;
	opTableItemDataBox.tableName = pModule->getName();
	opTableItemDataBox.isEmpty = false;
	opTableItemDataBox.readresultIdx = 0;

	std::vector<EntityTableItem*>::iterator iter = tableFixedOrderItems_.begin();
	for(; iter != tableFixedOrderItems_.end(); iter++)
	{
		static_cast<EntityTableItemMysqlBase*>((*iter))->getReadSqlItem(opTableItemDataBox);
	}

	if(!ReadEntityHelper::queryDB(dbi, opTableItemDataBox))
		return false;

	if(opTableItemDataBox.dbids[dbid].size() == 0)
		return false;

	iter = tableFixedOrderItems_.begin();
	for(; iter != tableFixedOrderItems_.end(); iter++)
	{
		static_cast<EntityTableItemMysqlBase*>((*iter))->addToStream(s, opTableItemDataBox, dbid);
	}

	return opTableItemDataBox.dbid == dbid;
}

//-------------------------------------------------------------------------------------
void EntityTableMysql::addToStream(MemoryStream* s, DB_OP_TABLE_ITEM_DATA_BOX& opTableItemDataBox, DBID resultDBID)
{
	std::vector<EntityTableItem*>::iterator iter = tableFixedOrderItems_.begin();
	for(; iter != tableFixedOrderItems_.end(); iter++)
	{
		static_cast<EntityTableItemMysqlBase*>((*iter))->addToStream(s, opTableItemDataBox, resultDBID);
	}
}

//-------------------------------------------------------------------------------------
void EntityTableMysql::getWriteSqlItem(DBInterface* dbi, MemoryStream* s, DB_OP_TABLE_ITEM_DATA_BOX& opTableItemDataBox)
{
	if(tableFixedOrderItems_.size() == 0)
		return;
	
	std::vector<EntityTableItem*>::iterator iter = tableFixedOrderItems_.begin();

	DB_OP_TABLE_ITEM_DATA_BOX* opTableItemDataBox1 = new DB_OP_TABLE_ITEM_DATA_BOX();
	opTableItemDataBox1->parentTableName = (*iter)->pParentTable()->tableName();
	opTableItemDataBox1->tableName = (*iter)->tableName();
	opTableItemDataBox1->parentTableDBID = 0;
	opTableItemDataBox1->dbid = 0;
	opTableItemDataBox1->isEmpty = (s == NULL);
	opTableItemDataBox1->readresultIdx = 0;

	KBEShared_ptr< DB_OP_TABLE_ITEM_DATA_BOX > opTableValBox1Ptr(opTableItemDataBox1);
	opTableItemDataBox.optable.push_back( std::pair<std::string/*tableName*/, KBEShared_ptr< DB_OP_TABLE_ITEM_DATA_BOX > >
		((*iter)->tableName(), opTableValBox1Ptr));

	for(; iter != tableFixedOrderItems_.end(); iter++)
	{
		static_cast<EntityTableItemMysqlBase*>((*iter))->getWriteSqlItem(dbi, s, *opTableItemDataBox1);
	}
}

//-------------------------------------------------------------------------------------
void EntityTableMysql::getReadSqlItem(DB_OP_TABLE_ITEM_DATA_BOX& opTableItemDataBox)
{
	if(tableFixedOrderItems_.size() == 0)
		return;
	
	std::vector<EntityTableItem*>::iterator iter = tableFixedOrderItems_.begin();

	DB_OP_TABLE_ITEM_DATA_BOX* opTableItemDataBox1 = new DB_OP_TABLE_ITEM_DATA_BOX();
	opTableItemDataBox1->parentTableName = (*iter)->pParentTable()->tableName();
	opTableItemDataBox1->tableName = (*iter)->tableName();
	opTableItemDataBox1->parentTableDBID = 0;
	opTableItemDataBox1->dbid = 0;
	opTableItemDataBox1->isEmpty = true;
	opTableItemDataBox1->readresultIdx = 0;

	KBEShared_ptr< DB_OP_TABLE_ITEM_DATA_BOX > opTableValBox1Ptr(opTableItemDataBox1);
	opTableItemDataBox.optable.push_back( std::pair<std::string/*tableName*/, KBEShared_ptr< DB_OP_TABLE_ITEM_DATA_BOX > >
		((*iter)->tableName(), opTableValBox1Ptr));

	for(; iter != tableFixedOrderItems_.end(); iter++)
	{
		static_cast<EntityTableItemMysqlBase*>((*iter))->getReadSqlItem(*opTableItemDataBox1);
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
	for(int i=0; i<2; i++)
	{
		if(key == db_item_names_[i])
			return true;
	}

	return false;
}

//-------------------------------------------------------------------------------------
bool EntityTableItemMysql_VECTOR2::syncToDB(DBInterface* dbi, void* pData)
{
	if(!sync_item_to_db(dbi, itemDBType_.c_str(), tableName_.c_str(), db_item_names_[0], 0, 
		this->mysqlItemtype_, this->flags(), pData))
		return false;

	return sync_item_to_db(dbi, itemDBType_.c_str(), tableName_.c_str(), db_item_names_[1], 0, 
		this->mysqlItemtype_, this->flags(), pData);
}

//-------------------------------------------------------------------------------------
void EntityTableItemMysql_VECTOR2::addToStream(MemoryStream* s, DB_OP_TABLE_ITEM_DATA_BOX& opTableItemDataBox, DBID resultDBID)
{
	ArraySize asize = 2;
	(*s) << asize;

	for(ArraySize i = 0; i < asize; i++)
	{
#ifdef CLIENT_NO_FLOAT
		int32 v = atoi(opTableItemDataBox.results[opTableItemDataBox.readresultIdx++].c_str());
#else
		float v = (float)atof(opTableItemDataBox.results[opTableItemDataBox.readresultIdx++].c_str());
#endif
		(*s) << v;
	}
}

//-------------------------------------------------------------------------------------
void EntityTableItemMysql_VECTOR2::getWriteSqlItem(DBInterface* dbi, MemoryStream* s, DB_OP_TABLE_ITEM_DATA_BOX& opTableItemDataBox)
{
	if(s == NULL)
		return;

#ifdef CLIENT_NO_FLOAT
	int32 v;
#else
	float v;
#endif

	ArraySize asize;

	(*s) >> asize;
	KBE_ASSERT(asize == 2);

	for(ArraySize i=0; i<asize; i++)
	{
		(*s) >> v;
		DB_OP_TABLE_ITEM_DATA* pSotvs = new DB_OP_TABLE_ITEM_DATA();
		pSotvs->sqlkey = db_item_names_[i];

#ifdef CLIENT_NO_FLOAT
		kbe_snprintf(pSotvs->sqlval, MAX_BUF, "%d", v);
#else
		kbe_snprintf(pSotvs->sqlval, MAX_BUF, "%f", v);
#endif
		
		opTableItemDataBox.items.push_back(KBEShared_ptr<DB_OP_TABLE_ITEM_DATA>(pSotvs));
	}
}

//-------------------------------------------------------------------------------------
void EntityTableItemMysql_VECTOR2::getReadSqlItem(DB_OP_TABLE_ITEM_DATA_BOX& opTableItemDataBox)
{
	ArraySize asize = 2;
	for(ArraySize i=0; i<asize; i++)
	{
		DB_OP_TABLE_ITEM_DATA* pSotvs = new DB_OP_TABLE_ITEM_DATA();
		pSotvs->sqlkey = db_item_names_[i];
		memset(pSotvs->sqlval, 0, MAX_BUF);
		opTableItemDataBox.items.push_back(KBEShared_ptr<DB_OP_TABLE_ITEM_DATA>(pSotvs));
	}
}

//-------------------------------------------------------------------------------------
bool EntityTableItemMysql_VECTOR3::isSameKey(std::string key)
{
	for(int i=0; i<3; i++)
	{
		if(key == db_item_names_[i])
			return true;
	}

	return false;
}

//-------------------------------------------------------------------------------------
bool EntityTableItemMysql_VECTOR3::syncToDB(DBInterface* dbi, void* pData)
{
	if(!sync_item_to_db(dbi, itemDBType_.c_str(), tableName_.c_str(), db_item_names_[0], 0, 
		this->mysqlItemtype_, this->flags(), pData))
		return false;

	if(!sync_item_to_db(dbi, itemDBType_.c_str(), tableName_.c_str(), db_item_names_[1], 0, 
		this->mysqlItemtype_, this->flags(), pData))
		return false;

	return sync_item_to_db(dbi, itemDBType_.c_str(), tableName_.c_str(), db_item_names_[2], 0, 
		this->mysqlItemtype_, this->flags(), pData);
}

//-------------------------------------------------------------------------------------
void EntityTableItemMysql_VECTOR3::addToStream(MemoryStream* s, DB_OP_TABLE_ITEM_DATA_BOX& opTableItemDataBox, DBID resultDBID)
{
	ArraySize asize = 3;
	(*s) << asize;

	for(ArraySize i = 0; i < asize; i++)
	{
#ifdef CLIENT_NO_FLOAT
		int32 v = atoi(opTableItemDataBox.results[opTableItemDataBox.readresultIdx++].c_str());
#else
		float v = (float)atof(opTableItemDataBox.results[opTableItemDataBox.readresultIdx++].c_str());
#endif
		(*s) << v;
	}
}

//-------------------------------------------------------------------------------------
void EntityTableItemMysql_VECTOR3::getWriteSqlItem(DBInterface* dbi, MemoryStream* s, DB_OP_TABLE_ITEM_DATA_BOX& opTableItemDataBox)
{
	if(s == NULL)
		return;

#ifdef CLIENT_NO_FLOAT
	int32 v;
#else
	float v;
#endif

	ArraySize asize;

	(*s) >> asize;
	KBE_ASSERT(asize == 3);

	for(ArraySize i=0; i<asize; i++)
	{
		(*s) >> v;
		DB_OP_TABLE_ITEM_DATA* pSotvs = new DB_OP_TABLE_ITEM_DATA();
		pSotvs->sqlkey = db_item_names_[i];

#ifdef CLIENT_NO_FLOAT
		kbe_snprintf(pSotvs->sqlval, MAX_BUF, "%d", v);
#else
		kbe_snprintf(pSotvs->sqlval, MAX_BUF, "%f", v);
#endif

		opTableItemDataBox.items.push_back(KBEShared_ptr<DB_OP_TABLE_ITEM_DATA>(pSotvs));
	}
}

//-------------------------------------------------------------------------------------
void EntityTableItemMysql_VECTOR3::getReadSqlItem(DB_OP_TABLE_ITEM_DATA_BOX& opTableItemDataBox)
{
	ArraySize asize = 3;
	for(ArraySize i=0; i<asize; i++)
	{
		DB_OP_TABLE_ITEM_DATA* pSotvs = new DB_OP_TABLE_ITEM_DATA();
		pSotvs->sqlkey = db_item_names_[i];
		memset(pSotvs->sqlval, 0, MAX_BUF);
		opTableItemDataBox.items.push_back(KBEShared_ptr<DB_OP_TABLE_ITEM_DATA>(pSotvs));
	}
}

//-------------------------------------------------------------------------------------
bool EntityTableItemMysql_VECTOR4::isSameKey(std::string key)
{
	for(int i=0; i<4; i++)
	{
		if(key == db_item_names_[i])
			return true;
	}

	return false;
}

//-------------------------------------------------------------------------------------
bool EntityTableItemMysql_VECTOR4::syncToDB(DBInterface* dbi, void* pData)
{
	if(!sync_item_to_db(dbi, itemDBType_.c_str(), tableName_.c_str(), db_item_names_[0], 0, this->mysqlItemtype_, this->flags(), pData))
		return false;

	if(!sync_item_to_db(dbi, itemDBType_.c_str(), tableName_.c_str(), db_item_names_[1], 0, this->mysqlItemtype_, this->flags(), pData))
		return false;

	if(!sync_item_to_db(dbi, itemDBType_.c_str(), tableName_.c_str(), db_item_names_[2], 0, this->mysqlItemtype_, this->flags(), pData))
		return false;

	return sync_item_to_db(dbi, itemDBType_.c_str(), tableName_.c_str(), db_item_names_[3], 0, this->mysqlItemtype_, this->flags(), pData);
}

//-------------------------------------------------------------------------------------
void EntityTableItemMysql_VECTOR4::addToStream(MemoryStream* s, DB_OP_TABLE_ITEM_DATA_BOX& opTableItemDataBox, DBID resultDBID)
{
	ArraySize asize = 4;
	(*s) << asize;

	for(ArraySize i = 0; i < asize; i++)
	{
#ifdef CLIENT_NO_FLOAT
		int32 v = atoi(opTableItemDataBox.results[opTableItemDataBox.readresultIdx++].c_str());
#else
		float v = (float)atof(opTableItemDataBox.results[opTableItemDataBox.readresultIdx++].c_str());
#endif
		(*s) << v;
	}
}

//-------------------------------------------------------------------------------------
void EntityTableItemMysql_VECTOR4::getWriteSqlItem(DBInterface* dbi, MemoryStream* s, DB_OP_TABLE_ITEM_DATA_BOX& opTableItemDataBox)
{
	if(s == NULL)
		return;

#ifdef CLIENT_NO_FLOAT
	int32 v;
#else
	float v;
#endif

	ArraySize asize;

	(*s) >> asize;
	KBE_ASSERT(asize == 4);

	for(ArraySize i=0; i<asize; i++)
	{
		(*s) >> v;
		DB_OP_TABLE_ITEM_DATA* pSotvs = new DB_OP_TABLE_ITEM_DATA();
		pSotvs->sqlkey = db_item_names_[i];

#ifdef CLIENT_NO_FLOAT
		kbe_snprintf(pSotvs->sqlval, MAX_BUF, "%d", v);
#else
		kbe_snprintf(pSotvs->sqlval, MAX_BUF, "%f", v);
#endif

		opTableItemDataBox.items.push_back(KBEShared_ptr<DB_OP_TABLE_ITEM_DATA>(pSotvs));
	}
}

//-------------------------------------------------------------------------------------
void EntityTableItemMysql_VECTOR4::getReadSqlItem(DB_OP_TABLE_ITEM_DATA_BOX& opTableItemDataBox)
{
	ArraySize asize = 4;
	for(ArraySize i=0; i<asize; i++)
	{
		DB_OP_TABLE_ITEM_DATA* pSotvs = new DB_OP_TABLE_ITEM_DATA();
		pSotvs->sqlkey = db_item_names_[i];
		memset(pSotvs->sqlval, 0, MAX_BUF);
		opTableItemDataBox.items.push_back(KBEShared_ptr<DB_OP_TABLE_ITEM_DATA>(pSotvs));
	}
}

//-------------------------------------------------------------------------------------
bool EntityTableItemMysql_MAILBOX::syncToDB(DBInterface* dbi, void* pData)
{
	return true;
}

//-------------------------------------------------------------------------------------
void EntityTableItemMysql_MAILBOX::addToStream(MemoryStream* s, DB_OP_TABLE_ITEM_DATA_BOX& opTableItemDataBox, DBID resultDBID)
{
}

//-------------------------------------------------------------------------------------
void EntityTableItemMysql_MAILBOX::getWriteSqlItem(DBInterface* dbi, MemoryStream* s, DB_OP_TABLE_ITEM_DATA_BOX& opTableItemDataBox)
{
}

//-------------------------------------------------------------------------------------
void EntityTableItemMysql_MAILBOX::getReadSqlItem(DB_OP_TABLE_ITEM_DATA_BOX& opTableItemDataBox)
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
	EntityTableMysql* pTable = new EntityTableMysql();

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
	
	std::string tablename = tname + "_";
	std::string itemName = "";

	if(name.size() > 0)
	{
		tablename += name;
	}
	else
	{
		tablename += TABLE_ARRAY_ITEM_VALUES_CONST_STR;
	}

	if(itemName.size() == 0)
	{
		if(static_cast<FixedArrayType*>(const_cast<DataType*>(pDataType))->getDataType()->type() != DATA_TYPE_FIXEDDICT)
			itemName = TABLE_ARRAY_ITEM_VALUE_CONST_STR;
	}

	pTable->tableName(tablename);
	pTable->isChild(true);

	EntityTableItem* pArrayTableItem;
	pArrayTableItem = pParentTable_->createItem(static_cast<FixedArrayType*>(const_cast<DataType*>(pDataType))->getDataType()->getName());
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

	EntityTables::getSingleton().addTable(pTable);
	return true;
}

//-------------------------------------------------------------------------------------
bool EntityTableItemMysql_ARRAY::syncToDB(DBInterface* dbi, void* pData)
{
	if(pChildTable_)
		return pChildTable_->syncToDB(dbi);

	return true;
}

//-------------------------------------------------------------------------------------
void EntityTableItemMysql_ARRAY::addToStream(MemoryStream* s, DB_OP_TABLE_ITEM_DATA_BOX& opTableItemDataBox, DBID resultDBID)
{
	if(pChildTable_)
	{
		DB_OP_TABLE_DATAS::iterator iter = opTableItemDataBox.optable.begin();
		for(; iter != opTableItemDataBox.optable.end(); iter++)
		{
			if(pChildTable_->tableName() == iter->first)
			{
				ArraySize size = (ArraySize)iter->second->dbids[resultDBID].size();
				(*s) << size;

				for(ArraySize i=0; i<size; i++)
					static_cast<EntityTableMysql*>(pChildTable_)->addToStream(s, *iter->second.get(), iter->second->dbids[resultDBID][i]);

				return;
			}
		}
	}

	ArraySize size = 0;
	(*s) << size;
}

//-------------------------------------------------------------------------------------
void EntityTableItemMysql_ARRAY::getWriteSqlItem(DBInterface* dbi, MemoryStream* s, DB_OP_TABLE_ITEM_DATA_BOX& opTableItemDataBox)
{
	ArraySize size = 0;
	if(s)
		(*s) >> size;

	if(pChildTable_)
	{
		if(size > 0)
		{
			for(ArraySize i=0; i<size; i++)
				static_cast<EntityTableMysql*>(pChildTable_)->getWriteSqlItem(dbi, s, opTableItemDataBox);
		}
		else
		{
			static_cast<EntityTableMysql*>(pChildTable_)->getWriteSqlItem(dbi, NULL, opTableItemDataBox);
		}
	}
}

//-------------------------------------------------------------------------------------
void EntityTableItemMysql_ARRAY::getReadSqlItem(DB_OP_TABLE_ITEM_DATA_BOX& opTableItemDataBox)
{
	if(pChildTable_)
	{
		static_cast<EntityTableMysql*>(pChildTable_)->getReadSqlItem(opTableItemDataBox);
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

	for(; fditer != keyTypes_.end(); fditer++)
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

	for(; iter != keyTypes.end(); iter++)
	{
		if(!iter->second->persistent)
			continue;

		EntityTableItem* tableItem = pParentTable_->createItem(iter->second->dataType->getName());

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
bool EntityTableItemMysql_FIXED_DICT::syncToDB(DBInterface* dbi, void* pData)
{
	EntityTableItemMysql_FIXED_DICT::FIXEDDICT_KEYTYPES::iterator iter = keyTypes_.begin();
	for(; iter != keyTypes_.end(); iter++)
	{
		if(!iter->second->syncToDB(dbi, pData))
			return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------
void EntityTableItemMysql_FIXED_DICT::addToStream(MemoryStream* s, DB_OP_TABLE_ITEM_DATA_BOX& opTableItemDataBox, DBID resultDBID)
{
	FIXEDDICT_KEYTYPES::iterator fditer = keyTypes_.begin();

	for(; fditer != keyTypes_.end(); fditer++)
	{
		static_cast<EntityTableItemMysqlBase*>(fditer->second.get())->addToStream(s, opTableItemDataBox, resultDBID);
	}
}

//-------------------------------------------------------------------------------------
void EntityTableItemMysql_FIXED_DICT::getWriteSqlItem(DBInterface* dbi, MemoryStream* s, DB_OP_TABLE_ITEM_DATA_BOX& opTableItemDataBox)
{
	FIXEDDICT_KEYTYPES::iterator fditer = keyTypes_.begin();

	for(; fditer != keyTypes_.end(); fditer++)
	{
		static_cast<EntityTableItemMysqlBase*>(fditer->second.get())->getWriteSqlItem(dbi, s, opTableItemDataBox);
	}
}

//-------------------------------------------------------------------------------------
void EntityTableItemMysql_FIXED_DICT::getReadSqlItem(DB_OP_TABLE_ITEM_DATA_BOX& opTableItemDataBox)
{
	FIXEDDICT_KEYTYPES::iterator fditer = keyTypes_.begin();

	for(; fditer != keyTypes_.end(); fditer++)
	{
		static_cast<EntityTableItemMysqlBase*>(fditer->second.get())->getReadSqlItem(opTableItemDataBox);
	}
}

//-------------------------------------------------------------------------------------
void EntityTableItemMysql_FIXED_DICT::init_db_item_name(const char* exstrFlag)
{
	FIXEDDICT_KEYTYPES::iterator fditer = keyTypes_.begin();

	for(; fditer != keyTypes_.end(); fditer++)
	{
		static_cast<EntityTableItemMysqlBase*>(fditer->second.get())->init_db_item_name(exstrFlag);
	}
}

//-------------------------------------------------------------------------------------
bool EntityTableItemMysqlBase::initialize(const PropertyDescription* pPropertyDescription, 
										  const DataType* pDataType, std::string name)
{
	itemName(name);

	pDataType_ = pDataType;
	pPropertyDescription_ = pPropertyDescription;
	return true;
}

//-------------------------------------------------------------------------------------
bool EntityTableItemMysql_DIGIT::syncToDB(DBInterface* dbi, void* pData)
{
	if(datalength_ == 0)
	{
		return sync_item_to_db(dbi, itemDBType_.c_str(), tableName_.c_str(), 
			db_item_name(), datalength_, this->mysqlItemtype_, this->flags(), pData);
	}

	uint32 length = pPropertyDescription_->getDatabaseLength();
	char sql_str[MAX_BUF];

	if(length <= 0)
		kbe_snprintf(sql_str, MAX_BUF, "%s", itemDBType_.c_str());
	else
		kbe_snprintf(sql_str, MAX_BUF, "%s(%u)", itemDBType_.c_str(), length);

	return sync_item_to_db(dbi, sql_str, tableName_.c_str(), db_item_name(), length, this->mysqlItemtype_, this->flags(), pData);
}

//-------------------------------------------------------------------------------------
void EntityTableItemMysql_DIGIT::addToStream(MemoryStream* s, DB_OP_TABLE_ITEM_DATA_BOX& opTableItemDataBox, DBID resultDBID)
{
	std::stringstream stream;
	stream << opTableItemDataBox.results[opTableItemDataBox.readresultIdx++];

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
void EntityTableItemMysql_DIGIT::getWriteSqlItem(DBInterface* dbi, MemoryStream* s, DB_OP_TABLE_ITEM_DATA_BOX& opTableItemDataBox)
{
	if(s == NULL)
		return;

	DB_OP_TABLE_ITEM_DATA* pSotvs = new DB_OP_TABLE_ITEM_DATA();
	
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
		kbe_snprintf(pSotvs->sqlval, MAX_BUF, "%"PRI64, v);
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
		kbe_snprintf(pSotvs->sqlval, MAX_BUF, "%"PRIu64, v);
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
	opTableItemDataBox.items.push_back(KBEShared_ptr<DB_OP_TABLE_ITEM_DATA>(pSotvs));
	
}

//-------------------------------------------------------------------------------------
void EntityTableItemMysql_DIGIT::getReadSqlItem(DB_OP_TABLE_ITEM_DATA_BOX& opTableItemDataBox)
{
	DB_OP_TABLE_ITEM_DATA* pSotvs = new DB_OP_TABLE_ITEM_DATA();
	pSotvs->sqlkey = db_item_name();
	memset(pSotvs->sqlval, 0, MAX_BUF);
	opTableItemDataBox.items.push_back(KBEShared_ptr<DB_OP_TABLE_ITEM_DATA>(pSotvs));
}

//-------------------------------------------------------------------------------------
bool EntityTableItemMysql_STRING::syncToDB(DBInterface* dbi, void* pData)
{
	uint32 length = pPropertyDescription_->getDatabaseLength();
	char sql_str[MAX_BUF];

	if(length > 0)
	{
		kbe_snprintf(sql_str, MAX_BUF, "%s(%u)", itemDBType_.c_str(), length);
	}
	else
	{
		kbe_snprintf(sql_str, MAX_BUF, "%s", itemDBType_.c_str());
	}

	return sync_item_to_db(dbi, sql_str, tableName_.c_str(), db_item_name(), length, 
		this->mysqlItemtype_, this->flags(), pData);
}

//-------------------------------------------------------------------------------------
void EntityTableItemMysql_STRING::addToStream(MemoryStream* s, 
											  DB_OP_TABLE_ITEM_DATA_BOX& opTableItemDataBox, DBID resultDBID)
{
	(*s) << opTableItemDataBox.results[opTableItemDataBox.readresultIdx++];
}

//-------------------------------------------------------------------------------------
void EntityTableItemMysql_STRING::getWriteSqlItem(DBInterface* dbi, 
												  MemoryStream* s, DB_OP_TABLE_ITEM_DATA_BOX& opTableItemDataBox)
{
	if(s == NULL)
		return;

	DB_OP_TABLE_ITEM_DATA* pSotvs = new DB_OP_TABLE_ITEM_DATA();

	std::string val;
	(*s) >> val;

	pSotvs->extraDatas = "\"";

	char* tbuf = new char[val.size() * 2 + 1];
	memset(tbuf, 0, val.size() * 2 + 1);

	mysql_real_escape_string(static_cast<DBInterfaceMysql*>(dbi)->mysql(), 
		tbuf, val.c_str(), val.size());

	pSotvs->extraDatas += tbuf;
	pSotvs->extraDatas += "\"";

	SAFE_RELEASE_ARRAY(tbuf);

	memset(pSotvs, 0, sizeof(pSotvs->sqlval));
	pSotvs->sqlkey = db_item_name();
	opTableItemDataBox.items.push_back(KBEShared_ptr<DB_OP_TABLE_ITEM_DATA>(pSotvs));
}

//-------------------------------------------------------------------------------------
void EntityTableItemMysql_STRING::getReadSqlItem(DB_OP_TABLE_ITEM_DATA_BOX& opTableItemDataBox)
{
	DB_OP_TABLE_ITEM_DATA* pSotvs = new DB_OP_TABLE_ITEM_DATA();
	pSotvs->sqlkey = db_item_name();
	memset(pSotvs->sqlval, 0, MAX_BUF);
	opTableItemDataBox.items.push_back(KBEShared_ptr<DB_OP_TABLE_ITEM_DATA>(pSotvs));
}

//-------------------------------------------------------------------------------------
bool EntityTableItemMysql_UNICODE::syncToDB(DBInterface* dbi, void* pData)
{
	uint32 length = pPropertyDescription_->getDatabaseLength();
	char sql_str[MAX_BUF];

	if(length > 0)
	{
		kbe_snprintf(sql_str, MAX_BUF, "%s(%u)", itemDBType_.c_str(), length);
	}
	else
	{
		kbe_snprintf(sql_str, MAX_BUF, "%s", itemDBType_.c_str());
	}

	return sync_item_to_db(dbi, sql_str, tableName_.c_str(), db_item_name(), length, 
		this->mysqlItemtype_, this->flags(), pData);
}

//-------------------------------------------------------------------------------------
void EntityTableItemMysql_UNICODE::addToStream(MemoryStream* s, 
											   DB_OP_TABLE_ITEM_DATA_BOX& opTableItemDataBox, DBID resultDBID)
{
	s->appendBlob(opTableItemDataBox.results[opTableItemDataBox.readresultIdx++]);
}

//-------------------------------------------------------------------------------------
void EntityTableItemMysql_UNICODE::getWriteSqlItem(DBInterface* dbi, MemoryStream* s, 
												   DB_OP_TABLE_ITEM_DATA_BOX& opTableItemDataBox)
{
	if(s == NULL)
		return;

	DB_OP_TABLE_ITEM_DATA* pSotvs = new DB_OP_TABLE_ITEM_DATA();

	std::string val;
	s->readBlob(val);

	char* tbuf = new char[val.size() * 2 + 1];
	memset(tbuf, 0, val.size() * 2 + 1);

	mysql_real_escape_string(static_cast<DBInterfaceMysql*>(dbi)->mysql(), 
		tbuf, val.c_str(), val.size());
	
	pSotvs->extraDatas = "\"";
	pSotvs->extraDatas += tbuf;
	pSotvs->extraDatas += "\"";
	SAFE_RELEASE_ARRAY(tbuf);

	memset(pSotvs, 0, sizeof(pSotvs->sqlval));
	pSotvs->sqlkey = db_item_name();
	opTableItemDataBox.items.push_back(KBEShared_ptr<DB_OP_TABLE_ITEM_DATA>(pSotvs));
}

//-------------------------------------------------------------------------------------
void EntityTableItemMysql_UNICODE::getReadSqlItem(DB_OP_TABLE_ITEM_DATA_BOX& opTableItemDataBox)
{
	DB_OP_TABLE_ITEM_DATA* pSotvs = new DB_OP_TABLE_ITEM_DATA();
	pSotvs->sqlkey = db_item_name();
	memset(pSotvs->sqlval, 0, MAX_BUF);
	opTableItemDataBox.items.push_back(KBEShared_ptr<DB_OP_TABLE_ITEM_DATA>(pSotvs));
}

//-------------------------------------------------------------------------------------
bool EntityTableItemMysql_BLOB::syncToDB(DBInterface* dbi, void* pData)
{
	return sync_item_to_db(dbi, itemDBType_.c_str(), tableName_.c_str(), db_item_name(), 0, 
		this->mysqlItemtype_, this->flags(), pData);
}

//-------------------------------------------------------------------------------------
void EntityTableItemMysql_BLOB::addToStream(MemoryStream* s, DB_OP_TABLE_ITEM_DATA_BOX& opTableItemDataBox, DBID resultDBID)
{
	std::string& datas = opTableItemDataBox.results[opTableItemDataBox.readresultIdx++];
	s->appendBlob(datas.data(), datas.size());
}

//-------------------------------------------------------------------------------------
void EntityTableItemMysql_BLOB::getWriteSqlItem(DBInterface* dbi, MemoryStream* s, DB_OP_TABLE_ITEM_DATA_BOX& opTableItemDataBox)
{
	if(s == NULL)
		return;

	DB_OP_TABLE_ITEM_DATA* pSotvs = new DB_OP_TABLE_ITEM_DATA();

	std::string val;
	s->readBlob(val);

	char* tbuf = new char[val.size() * 2 + 1];
	memset(tbuf, 0, val.size() * 2 + 1);

	mysql_real_escape_string(static_cast<DBInterfaceMysql*>(dbi)->mysql(), 
		tbuf, val.data(), val.size());

	pSotvs->extraDatas = "\"";
	pSotvs->extraDatas += tbuf;
	pSotvs->extraDatas += "\"";
	SAFE_RELEASE_ARRAY(tbuf);

	memset(pSotvs, 0, sizeof(pSotvs->sqlval));
	pSotvs->sqlkey = db_item_name();
	opTableItemDataBox.items.push_back(KBEShared_ptr<DB_OP_TABLE_ITEM_DATA>(pSotvs));
}

//-------------------------------------------------------------------------------------
void EntityTableItemMysql_BLOB::getReadSqlItem(DB_OP_TABLE_ITEM_DATA_BOX& opTableItemDataBox)
{
	DB_OP_TABLE_ITEM_DATA* pSotvs = new DB_OP_TABLE_ITEM_DATA();
	pSotvs->sqlkey = db_item_name();
	memset(pSotvs->sqlval, 0, MAX_BUF);
	opTableItemDataBox.items.push_back(KBEShared_ptr<DB_OP_TABLE_ITEM_DATA>(pSotvs));
}

//-------------------------------------------------------------------------------------
bool EntityTableItemMysql_PYTHON::syncToDB(DBInterface* dbi, void* pData)
{
	return sync_item_to_db(dbi, itemDBType_.c_str(), tableName_.c_str(), db_item_name(), 0, 
		this->mysqlItemtype_, this->flags(), pData);
}

//-------------------------------------------------------------------------------------
void EntityTableItemMysql_PYTHON::addToStream(MemoryStream* s, DB_OP_TABLE_ITEM_DATA_BOX& opTableItemDataBox, DBID resultDBID)
{
	std::string& datas = opTableItemDataBox.results[opTableItemDataBox.readresultIdx++];
	(*s).appendBlob(datas);
}

//-------------------------------------------------------------------------------------
void EntityTableItemMysql_PYTHON::getWriteSqlItem(DBInterface* dbi, MemoryStream* s, DB_OP_TABLE_ITEM_DATA_BOX& opTableItemDataBox)
{
	if(s == NULL)
		return;

	DB_OP_TABLE_ITEM_DATA* pSotvs = new DB_OP_TABLE_ITEM_DATA();

	std::string val;
	s->readBlob(val);

	char* tbuf = new char[val.size() * 2 + 1];
	memset(tbuf, 0, val.size() * 2 + 1);

	mysql_real_escape_string(static_cast<DBInterfaceMysql*>(dbi)->mysql(), 
		tbuf, val.c_str(), val.size());

	pSotvs->extraDatas = "\"";
	pSotvs->extraDatas += tbuf;
	pSotvs->extraDatas += "\"";
	SAFE_RELEASE_ARRAY(tbuf);

	memset(pSotvs, 0, sizeof(pSotvs->sqlval));
	pSotvs->sqlkey = db_item_name();
	opTableItemDataBox.items.push_back(KBEShared_ptr<DB_OP_TABLE_ITEM_DATA>(pSotvs));
}

//-------------------------------------------------------------------------------------
void EntityTableItemMysql_PYTHON::getReadSqlItem(DB_OP_TABLE_ITEM_DATA_BOX& opTableItemDataBox)
{
	DB_OP_TABLE_ITEM_DATA* pSotvs = new DB_OP_TABLE_ITEM_DATA();
	pSotvs->sqlkey = db_item_name();
	memset(pSotvs->sqlval, 0, MAX_BUF);
	opTableItemDataBox.items.push_back(KBEShared_ptr<DB_OP_TABLE_ITEM_DATA>(pSotvs));
}

//-------------------------------------------------------------------------------------
}
