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
#include "entitydef/scriptdef_module.hpp"
#include "entitydef/property.hpp"
#include "dbmgr_lib/db_interface.hpp"

#define SYNC_TO_DB(datatype)																	\
{																								\
	char __sql_str__[MAX_BUF];																	\
	kbe_snprintf(__sql_str__, MAX_BUF, "alter table tbl_%s add sm_%s %s;",						\
		this->pParentTable_->tableName(), itemName(), datatype);								\
																								\
	bool ret = pdbi_->query(__sql_str__, strlen(__sql_str__), false);							\
	if(!ret)																					\
	{																							\
		if(pdbi_->getlasterror() == 1060)														\
		{																						\
			kbe_snprintf(__sql_str__, MAX_BUF, "alter table tbl_%s modify sm_%s %s;",			\
				this->pParentTable_->tableName(), itemName(), datatype);						\
																								\
			ret = pdbi_->query(__sql_str__, strlen(__sql_str__), false);						\
		}																						\
																								\
		if(!ret)																				\
			return false;																		\
	}																							\
}																								\


namespace KBEngine { 

//-------------------------------------------------------------------------------------
EntityTableMysql::EntityTableMysql()
{
}

//-------------------------------------------------------------------------------------
EntityTableMysql::~EntityTableMysql()
{
}

//-------------------------------------------------------------------------------------
bool EntityTableMysql::initialize(DBInterface* dbi, ScriptDefModule* sm)
{
	pdbi_ = dbi;

	// 获取表名
	tableName(sm->getName());

	// 找到所有存储属性并且创建出所有的字段
	ScriptDefModule::PROPERTYDESCRIPTION_MAP& pdescrsMap = sm->getPersistentPropertyDescriptions();
	ScriptDefModule::PROPERTYDESCRIPTION_MAP::const_iterator iter = pdescrsMap.begin();
	for(; iter != pdescrsMap.end(); iter++)
	{
		PropertyDescription* pdescrs = iter->second;
		EntityTableItem* pETItem = this->createItem(pdescrs->getDataTypeName());
		pETItem->pParentTable(this);
		bool ret = pETItem->initialize(pdbi_, pdescrs, pdescrs->getDataType());
		
		if(!ret)
		{
			delete pETItem;
			return false;
		}

		tableItems_[pETItem->utype()].reset(pETItem);
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool EntityTableMysql::syncToDB()
{
	DEBUG_MSG("EntityTableMysql::syncToDB(): %s.\n", tableName());

	char sql_str[MAX_BUF];
	kbe_snprintf(sql_str, MAX_BUF, "CREATE TABLE IF NOT EXISTS tbl_%s "
			"(id BIGINT AUTO_INCREMENT, PRIMARY KEY idKey (id))"
		"ENGINE="MYSQL_ENGINE_TYPE, 
		tableName());

	bool ret = pdbi_->query(sql_str, strlen(sql_str));
	if(!ret)
	{
		return false;
	}

	std::vector<std::string> dbTableItemNames;
	std::string ttablename = "tbl_";
	ttablename += tableName();
	pdbi_->getTableItemNames(ttablename.c_str(), dbTableItemNames);

	// 检查是否有需要删除的表字段
	std::vector<std::string>::iterator iter0 = dbTableItemNames.begin();
	for(; iter0 != dbTableItemNames.end(); iter0++)
	{
		std::string tname = (*iter0);

		EntityTable::TABLEITEM_MAP::iterator iter = tableItems_.begin();
		bool found = false;

		for(; iter != tableItems_.end(); iter++)
		{
			std::string tname1 = "sm_";
			tname1 += iter->second->itemName();
			if(tname == tname1)
			{
				found = true;
				break;
			}
		}

		if(!found)
		{
			if(tname != "id" && tname != "parentID")
				if(!pdbi_->dropEntityTableItemFromDB(ttablename.c_str(), tname.c_str()))
					return false;
		}
	}

	EntityTable::TABLEITEM_MAP::iterator iter = tableItems_.begin();
	for(; iter != tableItems_.end(); iter++)
	{
		if(!iter->second->syncToDB())
			return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------
EntityTableItem* EntityTableMysql::createItem(std::string type)
{
	if(type == "INT8")
	{
		return new EntityTableItemMysql_DIGIT("tinyint", 4);
	}
	else if(type == "INT16")
	{
		return new EntityTableItemMysql_DIGIT("smallint", 6);
	}
	else if(type == "INT32")
	{
		return new EntityTableItemMysql_DIGIT("int", 11);
	}
	else if(type == "INT64")
	{
		return new EntityTableItemMysql_DIGIT("bigint", 20);
	}
	else if(type == "UINT8")
	{
		return new EntityTableItemMysql_DIGIT("tinyint unsigned", 3);
	}
	else if(type == "UINT16")
	{
		return new EntityTableItemMysql_DIGIT("smallint unsigned", 5);
	}
	else if(type == "UINT32")
	{
		return new EntityTableItemMysql_DIGIT("int unsigned", 10);
	}
	else if(type == "UINT64")
	{
		return new EntityTableItemMysql_DIGIT("bigint unsigned", 20);
	}
	else if(type == "FLOAT")
	{
		return new EntityTableItemMysql_DIGIT("float", 0);
	}
	else if(type == "DOUBLE")
	{
		return new EntityTableItemMysql_DIGIT("double", 0);
	}
	else if(type == "STRING")
	{
		return new EntityTableItemMysql_STRING("text", 0);
	}
	else if(type == "PYTHON")
	{
		return new EntityTableItemMysql_BLOB("blob", 0);
	}
	else if(type == "BLOB")
	{
		return new EntityTableItemMysql_BLOB("blob", 0);
	}
	else if(type == "ARRAY")
	{
		return new EntityTableItemMysql_ARRAY("", 0);
	}
	else if(type == "FIXED_DICT")
	{
		return new EntityTableItemMysql_FIXED_DICT("", 0);
	}

	return new EntityTableItemMysql_STRING("", 0);
}

//-------------------------------------------------------------------------------------
bool EntityTableItemMysql_ARRAY::initialize(DBInterface* dbi, const PropertyDescription* pPropertyDescription, const DataType* pDataType)
{
	bool ret = EntityTableItemMysql_DIGIT::initialize(dbi, pPropertyDescription, pDataType);
	if(!ret)
		return false;

	arrayTableItem_.reset(pParentTable_->createItem(static_cast<FixedArrayType*>(const_cast<DataType*>(pDataType))->getDataType()->getName()));
	arrayTableItem_->pParentTable(this->pParentTable());
	return arrayTableItem_->initialize(dbi, pPropertyDescription, static_cast<FixedArrayType*>(const_cast<DataType*>(pDataType))->getDataType());
}

//-------------------------------------------------------------------------------------
bool EntityTableItemMysql_ARRAY::syncToDB()
{
	DEBUG_MSG("EntityTableItemMysql_ARRAY::syncToDB(): %s.\n", itemName());
	return true;
}

//-------------------------------------------------------------------------------------
bool EntityTableItemMysql_FIXED_DICT::initialize(DBInterface* dbi, const PropertyDescription* pPropertyDescription, const DataType* pDataType)
{
	bool ret = EntityTableItemMysql_DIGIT::initialize(dbi, pPropertyDescription, pDataType);
	if(!ret)
		return false;

	KBEngine::FixedDictType* fdatatype = static_cast<KBEngine::FixedDictType*>(const_cast<DataType*>(pDataType));

	FixedDictType::FIXEDDICT_KEYTYPE_MAP& keyTypes = fdatatype->getKeyTypes();
	FixedDictType::FIXEDDICT_KEYTYPE_MAP::iterator iter = keyTypes.begin();
	for(; iter != keyTypes.end(); iter++)
	{
		EntityTableItem* tableItem = pParentTable_->createItem(iter->second->getName());
		tableItem->pParentTable(this->pParentTable());
		if(!tableItem->initialize(dbi, pPropertyDescription, iter->second))
			return false;
		keyTypes_[iter->first].reset(tableItem);
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool EntityTableItemMysql_FIXED_DICT::syncToDB()
{
	DEBUG_MSG("EntityTableItemMysql_FIXED_DICT::syncToDB(): %s.\n", itemName());
	return true;
}

//-------------------------------------------------------------------------------------
bool EntityTableItemMysql_DIGIT::initialize(DBInterface* dbi, const PropertyDescription* pPropertyDescription, const DataType* pDataType)
{
	pdbi_ = dbi;
	itemName(pPropertyDescription->getName());

	pDataType_ = pDataType;
	pPropertyDescription_ = pPropertyDescription;
	return true;
}

//-------------------------------------------------------------------------------------
bool EntityTableItemMysql_DIGIT::syncToDB()
{
	DEBUG_MSG("EntityTableItemMysql_DIGIT::syncToDB(): %s.\n", itemName());

	if(datalength_ == 0)
	{
		SYNC_TO_DB(itemDBType_.c_str());
		return true;
	}

	uint32 length = pPropertyDescription_->getDatabaseLength();
	char sql_str[MAX_BUF];

	if(length <= 0)
		kbe_snprintf(sql_str, MAX_BUF, "%s", itemDBType_.c_str());
	else
		kbe_snprintf(sql_str, MAX_BUF, "%s(%u)", itemDBType_.c_str(), length);

	SYNC_TO_DB(sql_str);
	return true;
}

//-------------------------------------------------------------------------------------
bool EntityTableItemMysql_STRING::syncToDB()
{
	DEBUG_MSG("EntityTableItemMysql_STRING::syncToDB(): %s.\n", itemName());
	
	uint32 length = pPropertyDescription_->getDatabaseLength();
	char sql_str[MAX_BUF];

	if(length > 0)
	{
		kbe_snprintf(sql_str, MAX_BUF, "CHAR(%u)", length);
	}
	else
	{
		kbe_snprintf(sql_str, MAX_BUF, "text");
	}

	SYNC_TO_DB(sql_str);
	return true;
}

//-------------------------------------------------------------------------------------
bool EntityTableItemMysql_BLOB::syncToDB()
{
	DEBUG_MSG("EntityTableItemMysql_BLOB::syncToDB(): %s.\n", itemName());
	SYNC_TO_DB(itemDBType_.c_str());
	return true;
}

//-------------------------------------------------------------------------------------
}
