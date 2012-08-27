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
		EntityTableItem* pETItem = this->createItem(pdescrs);
		bool ret = pETItem->initialize(pdbi_, pdescrs);
		
		if(!ret)
		{
			delete pETItem;
			return false;
		}

		pETItem->pParentTable(this);
		tableItems_[pETItem->utype()].reset(pETItem);
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool EntityTableMysql::syncToDB()
{
	DEBUG_MSG("EntityTableMysql::syncToDB(): %s.\n", tableName());

	char sql_str[MAX_BUF];
	kbe_snprintf(sql_str, MAX_BUF, "CREATE TABLE IF NOT EXISTS tbl_%s ("
			"id int(10) NOT NULL auto_increment,"
			"PRIMARY KEY  (`id`)"
		");", 
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
EntityTableItem* EntityTableMysql::createItem(const PropertyDescription* p)
{
	std::string type = p->getDataTypeName();
	if(type == "INT8" ||
		type == "INT16" ||
		type == "INT32")
	{
		return new EntityTableItemMysql_INT();
	}
	else if(type == "UINT8" ||
		type == "UINT16" ||
		type == "UINT32")
	{
		return new EntityTableItemMysql_UINT();
	}
	else if(type == "UINT64")
	{
		return new EntityTableItemMysql_UINT64();
	}
	else if(type == "INT64")
	{
		return new EntityTableItemMysql_INT64();
	}
	else if(type == "STRING")
	{
		return new EntityTableItemMysql_STRING();
	}
	else if(type == "FLOAT")
	{
		return new EntityTableItemMysql_FLOAT();
	}
	else if(type == "DOUBLE")
	{
		return new EntityTableItemMysql_DOUBLE();
	}

	return new EntityTableItemMysql_STRING();
}

//-------------------------------------------------------------------------------------
bool EntityTableItemMysql_ARRAY::initialize(DBInterface* dbi, const PropertyDescription* p)
{
	bool ret = EntityTableItemMysql_INT::initialize(dbi, p);
	if(!ret)
		return false;

	return true;
}

//-------------------------------------------------------------------------------------
bool EntityTableItemMysql_ARRAY::syncToDB()
{
	DEBUG_MSG("EntityTableItemMysql_ARRAY::syncToDB(): %s.\n", itemName());

	uint32 length = pPropertyDescription_->getDatabaseLength();
	if(length == 0)
		length = 10;

	char sql_str[MAX_BUF];
	kbe_snprintf(sql_str, MAX_BUF, "alter table tbl_%s add %s int(%s) not null default '0';",
		this->pParentTable_->tableName(), itemName(), length);

	bool ret = pdbi_->query(sql_str, strlen(sql_str));
	if(!ret)
	{
		return false;
	}
	return true;
}

//-------------------------------------------------------------------------------------
bool EntityTableItemMysql_INT::initialize(DBInterface* dbi, const PropertyDescription* p)
{
	pdbi_ = dbi;
	utype(p->getUType());
	itemName(p->getName());

	pPropertyDescription_ = p;
	return true;
}

//-------------------------------------------------------------------------------------
bool EntityTableItemMysql_INT::syncToDB()
{
	DEBUG_MSG("EntityTableItemMysql_INT::syncToDB(): %s.\n", itemName());

	uint32 length = pPropertyDescription_->getDatabaseLength();
	if(length == 0)
		length = 10;

	char sql_str[MAX_BUF];
	kbe_snprintf(sql_str, MAX_BUF, "int(%u)", length);

	SYNC_TO_DB(sql_str);
	return true;
}

//-------------------------------------------------------------------------------------
bool EntityTableItemMysql_UINT::syncToDB()
{
	DEBUG_MSG("EntityTableItemMysql_UINT::syncToDB(): %s.\n", itemName());

	uint32 length = pPropertyDescription_->getDatabaseLength();
	if(length == 0)
		length = 10;

	char sql_str[MAX_BUF];
	kbe_snprintf(sql_str, MAX_BUF, "int(%u)", length);

	SYNC_TO_DB(sql_str);
	return true;
}

//-------------------------------------------------------------------------------------
bool EntityTableItemMysql_INT64::syncToDB()
{
	DEBUG_MSG("EntityTableItemMysql_INT64::syncToDB(): %s.\n", itemName());

	uint32 length = pPropertyDescription_->getDatabaseLength();
	if(length == 0)
		length = 10;

	char sql_str[MAX_BUF];
	kbe_snprintf(sql_str, MAX_BUF, "int(%u)", length);

	SYNC_TO_DB(sql_str);
	return true;
}

//-------------------------------------------------------------------------------------
bool EntityTableItemMysql_UINT64::syncToDB()
{
	DEBUG_MSG("EntityTableItemMysql_UINT64::syncToDB(): %s.\n", itemName());

	uint32 length = pPropertyDescription_->getDatabaseLength();
	if(length == 0)
		length = 10;

	char sql_str[MAX_BUF];
	kbe_snprintf(sql_str, MAX_BUF, "int(%u)", length);

	SYNC_TO_DB(sql_str);
	return true;
}

//-------------------------------------------------------------------------------------
bool EntityTableItemMysql_STRING::syncToDB()
{
	DEBUG_MSG("EntityTableItemMysql_STRING::syncToDB(): %s.\n", itemName());
	
	uint32 length = pPropertyDescription_->getDatabaseLength();
	if(length == 0)
		length = 255;
	
	char sql_str[MAX_BUF];
	kbe_snprintf(sql_str, MAX_BUF, "VARCHAR(%u)", length);

	SYNC_TO_DB(sql_str);
	return true;
}

//-------------------------------------------------------------------------------------
bool EntityTableItemMysql_FLOAT::syncToDB()
{
	DEBUG_MSG("EntityTableItemMysql_FLOAT::syncToDB(): %s.\n", itemName());
	SYNC_TO_DB("float(18,3)");
	return true;
}

//-------------------------------------------------------------------------------------
bool EntityTableItemMysql_DOUBLE::syncToDB()
{
	DEBUG_MSG("EntityTableItemMysql_DOUBLE::syncToDB(): %s.\n", itemName());
	SYNC_TO_DB("double(30,6)");
	return true;
}

//-------------------------------------------------------------------------------------
}
