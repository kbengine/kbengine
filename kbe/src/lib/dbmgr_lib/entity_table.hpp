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

#ifndef __KBE_ENTITY_TABLE__
#define __KBE_ENTITY_TABLE__

#include "cstdkbe/cstdkbe.hpp"
#include "cstdkbe/singleton.hpp"
#include "helper/debug_helper.hpp"
#include "entitydef/common.hpp"

namespace KBEngine { 

class DBUtil;
class DBInterface;
class ScriptDefModule;
class DataType;
class PropertyDescription;
class EntityTable;

#define TABLE_ITEM_TYPE_UNKONWN		0
#define TABLE_ITEM_TYPE_ARRAY		1
#define TABLE_ITEM_TYPE_FIXEDDICT	2
#define TABLE_ITEM_TYPE_STRING		3
#define TABLE_ITEM_TYPE_DIGIT		4
#define TABLE_ITEM_TYPE_BLOB		5
#define TABLE_ITEM_TYPE_VECTOR2		6
#define TABLE_ITEM_TYPE_VECTOR3		7
#define TABLE_ITEM_TYPE_VECTOR4		8
#define TABLE_ITEM_TYPE_UNICODE		9

#define ENTITY_TABLE_PERFIX			"tbl"
#define TABLE_PARENT_ID				"parentID"
#define TABLE_ITEM_PERFIX			"sm"

/*
	维护entity在数据库中的表中的一个字段
*/
class EntityTableItem
{
public:
	EntityTableItem(std::string itemDBType, uint32 datalength):
		itemName_(),
		tableName_(),
		utype_(0),
		pdbi_(NULL),
		pParentTable_(NULL),
		pParentTableItem_(NULL),
		pDataType_(NULL),
		pPropertyDescription_(NULL),
		itemDBType_(itemDBType),
		datalength_(datalength)
	{
	};

	virtual ~EntityTableItem(){};

	virtual bool isSameKey(std::string key){ return itemName() == key; }

	virtual uint8 type()const{ return TABLE_ITEM_TYPE_UNKONWN; }

	void itemName(std::string name){ itemName_ = name; }
	const char* itemName(){ return itemName_.c_str(); }

	void utype(int32/*ENTITY_PROPERTY_UID*/ utype){ utype_ = utype; }
	int32 utype(){ return utype_; }

	void pParentTable(EntityTable* v){ pParentTable_ = v; }
	EntityTable* pParentTable(){ return pParentTable_; }

	void pParentTableItem(EntityTableItem* v){ pParentTableItem_ = v; }
	EntityTableItem* pParentTableItem(){ return pParentTableItem_; }

	const DataType* pDataType(){ return pDataType_; }

	/**
		初始化
	*/
	virtual bool initialize(DBInterface* dbi, const PropertyDescription* pPropertyDescription, 
		const DataType* pDataType, std::string itemName) = 0;

	void tableName(std::string name){ tableName_ = name; }
	const char* tableName(){ return tableName_.c_str(); }

	/**
		同步entity表到数据库中
	*/
	virtual bool syncToDB() = 0;
protected:
	// 字段名称
	std::string itemName_;
	std::string tableName_;
	int32/*ENTITY_PROPERTY_UID*/ utype_;

	DBInterface* pdbi_;

	EntityTable* pParentTable_;
	EntityTableItem* pParentTableItem_;

	const DataType* pDataType_;
	const PropertyDescription* pPropertyDescription_;

	std::string itemDBType_;
	uint32 datalength_;
};

/*
	维护entity在数据库中的表
*/
class EntityTable
{
public:
	typedef std::map<int32/*ENTITY_PROPERTY_UID*/, std::tr1::shared_ptr<EntityTableItem> > TABLEITEM_MAP;

	EntityTable():
	tableName_(),
	tableItems_(),
	pdbi_(NULL),
	isChild_(false)
	{
	};

	virtual ~EntityTable(){};
	
	void tableName(std::string name){ tableName_ = name; }
	const char* tableName(){ return tableName_.c_str(); }

	/**
		初始化
	*/
	virtual bool initialize(DBInterface* dbi, ScriptDefModule* sm, std::string name) = 0;

	/**
		同步entity表到数据库中
	*/
	virtual bool syncToDB() = 0;

	/** 
		创建一个表item
	*/
	virtual EntityTableItem* createItem(std::string type) = 0;

	/** 
		获得所有表字段
	*/
	const EntityTable::TABLEITEM_MAP& tableItems()const { return tableItems_; }

	void pdbi(DBInterface* v){ pdbi_ = v; }
	DBInterface* pdbi(){ return pdbi_; }

	void addItem(EntityTableItem* pItem);

	bool isChild()const{ return isChild_; }
	void isChild(bool b){ isChild_ = b; }
protected:

	// 表名称
	std::string tableName_;

	// 所有的字段
	TABLEITEM_MAP tableItems_;

	DBInterface* pdbi_;

	bool isChild_; // 是否为子表
};

class EntityTables : public Singleton<EntityTables>
{
public:
	typedef std::tr1::unordered_map<std::string, std::tr1::shared_ptr<EntityTable> > TABLES_MAP;
	EntityTables();
	virtual ~EntityTables();
	
	bool load(DBInterface* dbi);

	bool syncToDB();

	/** 
		获得所有表
	*/
	const EntityTables::TABLES_MAP& tables()const { return tables_; }

	void addTable(EntityTable* pTable);
protected:
	// 所有的字段
	TABLES_MAP tables_;
	DBInterface* pdbi_;
};

}

#endif // __KBE_ENTITY_TABLE__
