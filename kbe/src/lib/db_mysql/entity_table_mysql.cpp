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
#include "dbmgr_lib/entity_table.hpp"																					


namespace KBEngine { 


bool sync_item_to_db(DBInterface* dbi, const char* datatype, 
					 const char* strTableName, const char* strItemName, const char* perfix = TABLE_ITEM_PERFIX)						
{																								
	char __sql_str__[MAX_BUF];																	
	kbe_snprintf(__sql_str__, MAX_BUF, "alter table "ENTITY_TABLE_PERFIX"_%s add %s_%s %s;",						
		strTableName, perfix, strItemName, datatype);											
																								
	bool ret = dbi->query(__sql_str__, strlen(__sql_str__), false);							
	if(!ret)																					
	{																							
		if(dbi->getlasterror() == 1060)														
		{																						
			kbe_snprintf(__sql_str__, MAX_BUF, "alter table "ENTITY_TABLE_PERFIX"_%s modify %s_%s %s;",			
				strTableName, perfix, strItemName, datatype);				
																								
			ret = dbi->query(__sql_str__, strlen(__sql_str__), false);						
		}																						
																								
		if(!ret)																				
			return false;																		
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
bool EntityTableMysql::initialize(DBInterface* dbi, ScriptDefModule* sm, std::string name)
{
	pdbi_ = dbi;

	// 获取表名
	tableName(name);

	// 找到所有存储属性并且创建出所有的字段
	ScriptDefModule::PROPERTYDESCRIPTION_MAP& pdescrsMap = sm->getPersistentPropertyDescriptions();
	ScriptDefModule::PROPERTYDESCRIPTION_MAP::const_iterator iter = pdescrsMap.begin();
	for(; iter != pdescrsMap.end(); iter++)
	{
		PropertyDescription* pdescrs = iter->second;
		EntityTableItem* pETItem = this->createItem(pdescrs->getDataTypeName());
		pETItem->pParentTable(this);
		pETItem->utype(pdescrs->getUType());
		pETItem->tableName(this->tableName());
		bool ret = pETItem->initialize(pdbi_, pdescrs, pdescrs->getDataType(), pdescrs->getName());
		
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
	std::string exItems = "";

	if(this->isChild())
		exItems = ", "TABLE_PARENT_ID" bigint(20) NOT NULL, INDEX("TABLE_PARENT_ID")";

	kbe_snprintf(sql_str, MAX_BUF, "CREATE TABLE IF NOT EXISTS "ENTITY_TABLE_PERFIX"_%s "
			"(id bigint(20) AUTO_INCREMENT, PRIMARY KEY idKey (id)%s)"
		"ENGINE="MYSQL_ENGINE_TYPE, 
		tableName(), exItems.c_str());

	bool ret = pdbi_->query(sql_str, strlen(sql_str));
	if(!ret)
	{
		return false;
	}

	EntityTable::TABLEITEM_MAP::iterator iter = tableItems_.begin();
	for(; iter != tableItems_.end(); iter++)
	{
		if(!iter->second->syncToDB())
			return false;
	}

	std::vector<std::string> dbTableItemNames;
	std::string ttablename = ENTITY_TABLE_PERFIX"_";
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
			// 处理array中的值的特例情况
			if(tname == "sm_value")
			{
				if(iter->second->pParentTableItem() && iter->second->type() != TABLE_ITEM_TYPE_ARRAY)
				{
					if(iter->second->pParentTableItem()->type() == TABLE_ITEM_TYPE_ARRAY)
					{
						found = true;
						break;
					}
				}
			}

			std::string tname1 = TABLE_ITEM_PERFIX"_";
			
			if(strncmp(tname.c_str(), tname1.c_str(), tname1.size()) == 0)
			{
				tname1 = (tname.c_str() + 3);
			}
			else
			{
				tname1 = tname;
			}

			if(iter->second->isSameKey(tname1))
			{
				found = true;
				break;
			}
		}

		if(!found)
		{
			if(tname != "id" && tname != TABLE_PARENT_ID)
				if(!pdbi_->dropEntityTableItemFromDB(ttablename.c_str(), tname.c_str()))
					return false;
		}
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
	else if(type == "UNICODE")
	{
		return new EntityTableItemMysql_UNICODE("text", 0);
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
	else if(type == "VECTOR2")
	{
		return new EntityTableItemMysql_VECTOR3("float", 0);
	}
	else if(type == "VECTOR3")
	{
		return new EntityTableItemMysql_VECTOR3("float", 0);
	}
	else if(type == "VECTOR4")
	{
		return new EntityTableItemMysql_VECTOR3("float", 0);
	}

	return new EntityTableItemMysql_STRING("", 0);
}

//-------------------------------------------------------------------------------------
bool EntityTableItemMysql_VECTOR2::isSameKey(std::string key)
{
	std::string vkey0 = "0_";
	vkey0 += itemName();

	std::string vkey1 = "1_";
	vkey1 += itemName();

	return (key == vkey0 || key == vkey1);
}

//-------------------------------------------------------------------------------------
bool EntityTableItemMysql_VECTOR2::syncToDB()
{
	DEBUG_MSG("EntityTableItemMysql_VECTOR2::syncToDB(): %s.\n", itemName());

	if(!sync_item_to_db(pdbi_, itemDBType_.c_str(), tableName_.c_str(), itemName(), "sm_0"))
		return false;

	return sync_item_to_db(pdbi_, itemDBType_.c_str(), tableName_.c_str(), itemName(), "sm_1");
}

//-------------------------------------------------------------------------------------
bool EntityTableItemMysql_VECTOR2::updateItem(DBID dbid, MemoryStream* s, ScriptDefModule* pModule)
{
	return true;
}

//-------------------------------------------------------------------------------------
bool EntityTableItemMysql_VECTOR3::isSameKey(std::string key)
{
	std::string vkey0 = "0_";
	vkey0 += itemName();

	std::string vkey1 = "1_";
	vkey1 += itemName();

	std::string vkey2 = "2_";
	vkey2 += itemName();

	return (key == vkey0 || key == vkey1 || key == vkey2);
}

//-------------------------------------------------------------------------------------
bool EntityTableItemMysql_VECTOR3::syncToDB()
{
	DEBUG_MSG("EntityTableItemMysql_VECTOR3::syncToDB(): %s.\n", itemName());

	if(!sync_item_to_db(pdbi_, itemDBType_.c_str(), tableName_.c_str(), itemName(), "sm_0"))
		return false;

	if(!sync_item_to_db(pdbi_, itemDBType_.c_str(), tableName_.c_str(), itemName(), "sm_1"))
		return false;

	return sync_item_to_db(pdbi_, itemDBType_.c_str(), tableName_.c_str(), itemName(), "sm_2");
}

//-------------------------------------------------------------------------------------
bool EntityTableItemMysql_VECTOR3::updateItem(DBID dbid, MemoryStream* s, ScriptDefModule* pModule)
{
	return true;
}

//-------------------------------------------------------------------------------------
bool EntityTableItemMysql_VECTOR4::isSameKey(std::string key)
{
	std::string vkey0 = "0_";
	vkey0 += itemName();

	std::string vkey1 = "1_";
	vkey1 += itemName();

	std::string vkey2 = "2_";
	vkey2 += itemName();

	std::string vkey3 = "3_";
	vkey3 += itemName();

	return (key == vkey0 || key == vkey1 || key == vkey2 || key == vkey2);
}

//-------------------------------------------------------------------------------------
bool EntityTableItemMysql_VECTOR4::syncToDB()
{
	DEBUG_MSG("EntityTableItemMysql_VECTOR4::syncToDB(): %s.\n", itemName());

	if(!sync_item_to_db(pdbi_, itemDBType_.c_str(), tableName_.c_str(), itemName(), "sm_0"))
		return false;

	if(!sync_item_to_db(pdbi_, itemDBType_.c_str(), tableName_.c_str(), itemName(), "sm_1"))
		return false;

	if(!sync_item_to_db(pdbi_, itemDBType_.c_str(), tableName_.c_str(), itemName(), "sm_2"))
		return false;

	return sync_item_to_db(pdbi_, itemDBType_.c_str(), tableName_.c_str(), itemName(), "sm_3");
}

//-------------------------------------------------------------------------------------
bool EntityTableItemMysql_VECTOR4::updateItem(DBID dbid, MemoryStream* s, ScriptDefModule* pModule)
{
	return true;
}

//-------------------------------------------------------------------------------------
bool EntityTableItemMysql_ARRAY::isSameKey(std::string key)
{
	return true;
}

//-------------------------------------------------------------------------------------
bool EntityTableItemMysql_ARRAY::initialize(DBInterface* dbi, const PropertyDescription* pPropertyDescription, 
											const DataType* pDataType, std::string name)
{
	bool ret = EntityTableItemMysqlBase::initialize(dbi, pPropertyDescription, pDataType, name);
	if(!ret)
		return false;

	// 创建子表
	EntityTableMysql* pTable = new EntityTableMysql();
	pTable->pdbi(dbi);

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
		tablename += "values";
		itemName = "value";
	}

	pTable->tableName(tablename);
	pTable->isChild(true);

	EntityTableItem* pArrayTableItem;
	pArrayTableItem = pParentTable_->createItem(static_cast<FixedArrayType*>(const_cast<DataType*>(pDataType))->getDataType()->getName());
	pArrayTableItem->utype(-pPropertyDescription->getUType());
	pArrayTableItem->pParentTable(this->pParentTable());
	pArrayTableItem->pParentTableItem(this);
	pArrayTableItem->tableName(pTable->tableName());

	ret = pArrayTableItem->initialize(dbi, pPropertyDescription, 
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
bool EntityTableItemMysql_ARRAY::syncToDB()
{
	DEBUG_MSG("EntityTableItemMysql_ARRAY::syncToDB(): %s.\n", itemName());
	if(pChildTable_)
		return pChildTable_->syncToDB();

	return true;
}

//-------------------------------------------------------------------------------------
bool EntityTableItemMysql_ARRAY::updateItem(DBID dbid, MemoryStream* s, ScriptDefModule* pModule)
{
	if(pChildTable_)
		return pChildTable_->updateTable(dbid, s, pModule);

	return true;
}

//-------------------------------------------------------------------------------------
bool EntityTableItemMysql_FIXED_DICT::isSameKey(std::string key)
{
	FIXEDDICT_KEYTYPE_MAP::iterator fditer = keyTypes_.begin();
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
bool EntityTableItemMysql_FIXED_DICT::initialize(DBInterface* dbi, const PropertyDescription* pPropertyDescription, 
												 const DataType* pDataType, std::string name)
{
	bool ret = EntityTableItemMysqlBase::initialize(dbi, pPropertyDescription, pDataType, name);
	if(!ret)
		return false;

	KBEngine::FixedDictType* fdatatype = static_cast<KBEngine::FixedDictType*>(const_cast<DataType*>(pDataType));

	FixedDictType::FIXEDDICT_KEYTYPE_MAP& keyTypes = fdatatype->getKeyTypes();
	FixedDictType::FIXEDDICT_KEYTYPE_MAP::iterator iter = keyTypes.begin();
	for(; iter != keyTypes.end(); iter++)
	{
		EntityTableItem* tableItem = pParentTable_->createItem(iter->second->getName());

		tableItem->pParentTable(this->pParentTable());
		tableItem->pParentTableItem(this);
		tableItem->utype(-pPropertyDescription->getUType());
		tableItem->tableName(this->tableName());
		if(!tableItem->initialize(dbi, pPropertyDescription, iter->second, iter->first))
			return false;

		keyTypes_[iter->first].reset(tableItem);
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool EntityTableItemMysql_FIXED_DICT::syncToDB()
{
	DEBUG_MSG("EntityTableItemMysql_FIXED_DICT::syncToDB(): %s.\n", itemName());

	EntityTableItemMysql_FIXED_DICT::FIXEDDICT_KEYTYPE_MAP::iterator iter = keyTypes_.begin();
	for(; iter != keyTypes_.end(); iter++)
	{
		if(!iter->second->syncToDB())
			return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool EntityTableItemMysql_FIXED_DICT::updateItem(DBID dbid, MemoryStream* s, ScriptDefModule* pModule)
{
	return true;
}

//-------------------------------------------------------------------------------------
bool EntityTableItemMysqlBase::initialize(DBInterface* dbi, const PropertyDescription* pPropertyDescription, 
										  const DataType* pDataType, std::string name)
{
	pdbi_ = dbi;
	itemName(name);

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
		return sync_item_to_db(pdbi_, itemDBType_.c_str(), this->pParentTable_->tableName(), itemName());
	}

	uint32 length = pPropertyDescription_->getDatabaseLength();
	char sql_str[MAX_BUF];

	if(length <= 0)
		kbe_snprintf(sql_str, MAX_BUF, "%s", itemDBType_.c_str());
	else
		kbe_snprintf(sql_str, MAX_BUF, "%s(%u)", itemDBType_.c_str(), length);

	return sync_item_to_db(pdbi_, sql_str, tableName_.c_str(), itemName());
}

//-------------------------------------------------------------------------------------
bool EntityTableItemMysql_DIGIT::updateItem(DBID dbid, MemoryStream* s, ScriptDefModule* pModule)
{
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
		kbe_snprintf(sql_str, MAX_BUF, "text(%u)", length);
	}
	else
	{
		kbe_snprintf(sql_str, MAX_BUF, "text");
	}

	return sync_item_to_db(pdbi_, sql_str, tableName_.c_str(), itemName());
}

//-------------------------------------------------------------------------------------
bool EntityTableItemMysql_STRING::updateItem(DBID dbid, MemoryStream* s, ScriptDefModule* pModule)
{
	return true;
}

//-------------------------------------------------------------------------------------
bool EntityTableItemMysql_UNICODE::syncToDB()
{
	DEBUG_MSG("EntityTableItemMysql_UNICODE::syncToDB(): %s.\n", itemName());
	
	uint32 length = pPropertyDescription_->getDatabaseLength();
	char sql_str[MAX_BUF];

	if(length > 0)
	{
		kbe_snprintf(sql_str, MAX_BUF, "text(%u)", length);
	}
	else
	{
		kbe_snprintf(sql_str, MAX_BUF, "text");
	}

	return sync_item_to_db(pdbi_, sql_str, tableName_.c_str(), itemName());
}

//-------------------------------------------------------------------------------------
bool EntityTableItemMysql_UNICODE::updateItem(DBID dbid, MemoryStream* s, ScriptDefModule* pModule)
{
	return true;
}

//-------------------------------------------------------------------------------------
bool EntityTableItemMysql_BLOB::syncToDB()
{
	DEBUG_MSG("EntityTableItemMysql_BLOB::syncToDB(): %s.\n", itemName());
	return sync_item_to_db(pdbi_, itemDBType_.c_str(), tableName_.c_str(), itemName());
}

//-------------------------------------------------------------------------------------
bool EntityTableItemMysql_BLOB::updateItem(DBID dbid, MemoryStream* s, ScriptDefModule* pModule)
{
	return true;
}

//-------------------------------------------------------------------------------------
}
