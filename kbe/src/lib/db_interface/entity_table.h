﻿/*
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

#ifndef KBE_ENTITY_TABLE_H
#define KBE_ENTITY_TABLE_H

#include "common/common.h"
#include "common/singleton.h"
#include "helper/debug_helper.h"
#include "entitydef/common.h"
#include "thread/threadmutex.h"

namespace KBEngine { 

class DBUtil;
class DBInterface;
class ScriptDefModule;
class DataType;
class PropertyDescription;
class EntityTable;
class MemoryStream;

#define TABLE_ITEM_TYPE_UNKONWN		0
#define TABLE_ITEM_TYPE_FIXEDARRAY	1
#define TABLE_ITEM_TYPE_FIXEDDICT	2
#define TABLE_ITEM_TYPE_STRING		3
#define TABLE_ITEM_TYPE_DIGIT		4
#define TABLE_ITEM_TYPE_BLOB		5
#define TABLE_ITEM_TYPE_VECTOR2		6
#define TABLE_ITEM_TYPE_VECTOR3		7
#define TABLE_ITEM_TYPE_VECTOR4		8
#define TABLE_ITEM_TYPE_UNICODE		9
#define TABLE_ITEM_TYPE_MAILBOX		10
#define TABLE_ITEM_TYPE_PYTHON		11

#define ENTITY_TABLE_PERFIX						"tbl"
#define TABLE_ID_CONST_STR						"id"
#define TABLE_PARENTID_CONST_STR				"parentID"
#define TABLE_ITEM_PERFIX						"sm"
#define TABLE_ARRAY_ITEM_VALUE_CONST_STR		"value"
#define TABLE_ARRAY_ITEM_VALUES_CONST_STR		"values"
#define TABLE_AUTOLOAD_CONST_STR				"autoLoad"

/**
	db表操作
*/
enum DB_TABLE_OP
{
	TABLE_OP_INSERT					= 1,
	TABLE_OP_UPDATE					= 2,
	TABLE_OP_DELETE					= 3
};

/*!
 * @brief 账户信息: name, password, email, post-data, dbid, flags, deadline
 */
struct ACCOUNT_INFOS
{
	ACCOUNT_INFOS():
	name(),
	password(),
	datas(),
	dbid(0),
	flags(0),
	deadline(0)
	{
	}

	std::string name, password, datas, email;
	DBID dbid;

	uint32 flags;
	uint64 deadline;
};

/**
	维护entity在数据库中的表中的一个字段
*/
class EntityTableItem
{
public:
	EntityTableItem(std::string itemDBType, uint32 datalength, uint32 flags):
		itemName_(),
		tableName_(),
		utype_(0),
		pParentTable_(NULL),
		pParentTableItem_(NULL),
		pDataType_(NULL),
		pPropertyDescription_(NULL),
		itemDBType_(itemDBType),
		datalength_(datalength),
		flags_(flags),
		indexType_()
	{
	};

	virtual ~EntityTableItem(){};

	virtual bool isSameKey(std::string key){ return itemName() == key; }

	virtual uint8 type() const{ return TABLE_ITEM_TYPE_UNKONWN; }

	void itemName(std::string name){ itemName_ = name; }
	const char* itemName(){ return itemName_.c_str(); }

	void indexType(std::string index){ indexType_ = index; }
	const char* indexType(){ return indexType_.c_str(); }
	
	const char* itemDBType(){ return itemDBType_.c_str(); }

	void utype(int32/*ENTITY_PROPERTY_UID*/ utype){ utype_ = utype; }
	int32 utype(){ return utype_; }

	void flags(uint32 f){ flags_ = f; }
	uint32 flags(){ return flags_; }

	void pParentTable(EntityTable* v){ pParentTable_ = v; }
	EntityTable* pParentTable(){ return pParentTable_; }

	void pParentTableItem(EntityTableItem* v){ pParentTableItem_ = v; }
	EntityTableItem* pParentTableItem(){ return pParentTableItem_; }

	const DataType* pDataType(){ return pDataType_; }

	uint32 datalength() const{ return datalength_; }

	const PropertyDescription* pPropertyDescription() const{ return pPropertyDescription_; }

	/**
		初始化
	*/
	virtual bool initialize(const PropertyDescription* pPropertyDescription, 
		const DataType* pDataType, std::string itemName) = 0;

	void tableName(std::string name){ tableName_ = name; }
	const char* tableName(){ return tableName_.c_str(); }

	/**
		同步entity表到数据库中
	*/
	virtual bool syncToDB(DBInterface* dbi, void* pData = NULL) = 0;

	/**
		更新数据
	*/
	virtual bool writeItem(DBInterface* dbi, DBID dbid, MemoryStream* s, ScriptDefModule* pModule) = 0;

	/**
		获取所有的数据放到流中
	*/
	virtual bool queryTable(DBInterface* dbi, DBID dbid, MemoryStream* s, ScriptDefModule* pModule) = 0;
protected:
	// 字段名称
	std::string itemName_;
	std::string tableName_;
	int32/*ENTITY_PROPERTY_UID*/ utype_;

	EntityTable* pParentTable_;
	EntityTableItem* pParentTableItem_;

	const DataType* pDataType_;
	const PropertyDescription* pPropertyDescription_;

	std::string itemDBType_;
	uint32 datalength_;
	uint32 flags_;

	std::string indexType_;
};

/*
	维护entity在数据库中的表
*/
class EntityTable
{
public:
	typedef std::map<int32/*ENTITY_PROPERTY_UID*/, KBEShared_ptr<EntityTableItem> > TABLEITEM_MAP;

	EntityTable():
	tableName_(),
	tableItems_(),
	tableFixedOrderItems_(),
	isChild_(false),
	sync_(false)
	{
	};

	virtual ~EntityTable(){};
	
	void tableName(std::string name){ tableName_ = name; }
	const char* tableName(){ return tableName_.c_str(); }

	/**
		初始化
	*/
	virtual bool initialize(ScriptDefModule* sm, std::string name) = 0;

	/**
		同步entity表到数据库中
	*/
	virtual bool syncToDB(DBInterface* dbi) = 0;

	/**
		同步entity表索引到数据库中
	*/
	virtual bool syncIndexToDB(DBInterface* dbi) = 0;

	/** 
		创建一个表item
	*/
	virtual EntityTableItem* createItem(std::string type) = 0;

	/** 
		获得所有表字段
	*/
	const EntityTable::TABLEITEM_MAP& tableItems() const { return tableItems_; }
	const std::vector<EntityTableItem*>& tableFixedOrderItems() const{ return tableFixedOrderItems_; }

	void addItem(EntityTableItem* pItem);

	bool isChild() const{ return isChild_; }
	void isChild(bool b){ isChild_ = b; }

	EntityTableItem* findItem(int32/*ENTITY_PROPERTY_UID*/ utype);

	/**
		更新表
	*/
	virtual DBID writeTable(DBInterface* dbi, DBID dbid, int8 shouldAutoLoad, MemoryStream* s, ScriptDefModule* pModule);

	/**
		从数据库删除entity
	*/
	virtual bool removeEntity(DBInterface* dbi, DBID dbid, ScriptDefModule* pModule);

	/**
		获取所有的数据放到流中
	*/
	virtual bool queryTable(DBInterface* dbi, DBID dbid, MemoryStream* s, ScriptDefModule* pModule);

	/**
		设置是否自动加载
	*/
	virtual void entityShouldAutoLoad(DBInterface* dbi, DBID dbid, bool shouldAutoLoad){};

	bool hasSync() const { return sync_; }

	/**
		查询自动加载的实体
	*/
	virtual void queryAutoLoadEntities(DBInterface* dbi, ScriptDefModule* pModule, 
		ENTITY_ID start, ENTITY_ID end, std::vector<DBID>& outs){}

protected:

	// 表名称
	std::string tableName_;

	// 所有的字段
	TABLEITEM_MAP tableItems_;

	// 和ScriptDefModule中保持一致秩序的item引用
	std::vector<EntityTableItem*> tableFixedOrderItems_; 

	// 是否为子表
	bool isChild_; 

	bool sync_;
};

class EntityTables : public Singleton<EntityTables>
{
public:
	typedef KBEUnordered_map<std::string, KBEShared_ptr<EntityTable> > TABLES_MAP;
	EntityTables();
	virtual ~EntityTables();
	
	bool load(DBInterface* dbi);

	bool syncToDB(DBInterface* dbi);

	/** 
		获得所有表
	*/
	const EntityTables::TABLES_MAP& tables() const { return tables_; }

	void addTable(EntityTable* pTable);

	EntityTable* findTable(std::string name);

	void addKBETable(EntityTable* pTable);

	EntityTable* findKBETable(std::string name);

	/**
		写entity到数据库
	*/
	DBID writeEntity(DBInterface* dbi, DBID dbid, int8 shouldAutoLoad, MemoryStream* s, ScriptDefModule* pModule);

	/**
		从数据库删除entity
	*/
	bool removeEntity(DBInterface* dbi, DBID dbid, ScriptDefModule* pModule);

	/**
		获取某个表所有的数据放到流中
	*/
	bool queryEntity(DBInterface* dbi, DBID dbid, MemoryStream* s, ScriptDefModule* pModule);

	void onTableSyncSuccessfully(KBEShared_ptr<EntityTable> pEntityTable, bool error);

	/**
		查询自动加载的实体
	*/
	void queryAutoLoadEntities(DBInterface* dbi, ScriptDefModule* pModule, 
		ENTITY_ID start, ENTITY_ID end, std::vector<DBID>& outs);

protected:
	// 所有的表
	TABLES_MAP tables_;
	TABLES_MAP kbe_tables_;

	int numSyncTables_;
	bool syncTablesError_;
};

}

#endif // KBE_ENTITY_TABLE_H
