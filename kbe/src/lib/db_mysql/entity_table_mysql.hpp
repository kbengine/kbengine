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

#ifndef __KBE_ENTITY_TABLE_MYSQL__
#define __KBE_ENTITY_TABLE_MYSQL__

#include "common.hpp"
#include "cstdkbe/cstdkbe.hpp"
#include "cstdkbe/singleton.hpp"
#include "helper/debug_helper.hpp"
#include "dbmgr_lib/entity_table.hpp"

namespace KBEngine { 

class ScriptDefModule;
class EntityTableMysql;

#define MYSQL_ENGINE_TYPE "InnoDB"

/*
	维护entity在数据库中的表中的一个字段
*/
class EntityTableItemMysqlBase : public EntityTableItem
{
public:
	EntityTableItemMysqlBase(std::string itemDBType, uint32 datalength):
	  EntityTableItem(itemDBType, datalength)
	{
	};
	virtual ~EntityTableItemMysqlBase(){};

	uint8 type()const{ return TABLE_ITEM_TYPE_UNKONWN; }

	/**
		初始化
	*/
	virtual bool initialize(DBInterface* dbi, const PropertyDescription* pPropertyDescription, 
		const DataType* pDataType, std::string name);

	/**
		同步entity表到数据库中
	*/
	virtual bool syncToDB() = 0;

	/**
		获取需要存储的表名， 字段名和转换为sql存储时的字符串值
	*/
	virtual void getSqlItemStr(MemoryStream* s, SQL_OP_TABLE& opTable) = 0;
protected:
};

class EntityTableItemMysql_DIGIT : public EntityTableItemMysqlBase
{
public:
	EntityTableItemMysql_DIGIT(std::string dataSType, std::string itemDBType, uint32 datalength):
	  EntityTableItemMysqlBase(itemDBType, datalength),
		  dataSType_(dataSType)
	{
	};

	virtual ~EntityTableItemMysql_DIGIT(){};

	uint8 type()const{ return TABLE_ITEM_TYPE_DIGIT; }

	/**
		同步entity表到数据库中
	*/
	virtual bool syncToDB();

	/**
		更新数据
	*/
	virtual bool updateItem(DBID dbid, MemoryStream* s, ScriptDefModule* pModule);

	/**
		获取需要存储的表名， 字段名和转换为sql存储时的字符串值
	*/
	virtual void getSqlItemStr(MemoryStream* s, SQL_OP_TABLE& opTable);
protected:
	std::string dataSType_;
};

class EntityTableItemMysql_STRING : public EntityTableItemMysqlBase
{
public:
	EntityTableItemMysql_STRING(std::string itemDBType, uint32 datalength):
	  EntityTableItemMysqlBase(itemDBType, datalength)
	  {
	  }

	virtual ~EntityTableItemMysql_STRING(){};

	uint8 type()const{ return TABLE_ITEM_TYPE_STRING; }

	/**
		同步entity表到数据库中
	*/
	virtual bool syncToDB();

	/**
		更新数据
	*/
	virtual bool updateItem(DBID dbid, MemoryStream* s, ScriptDefModule* pModule);

	/**
		获取需要存储的表名， 字段名和转换为sql存储时的字符串值
	*/
	virtual void getSqlItemStr(MemoryStream* s, SQL_OP_TABLE& opTable);
};

class EntityTableItemMysql_UNICODE : public EntityTableItemMysqlBase
{
public:
	EntityTableItemMysql_UNICODE(std::string itemDBType, uint32 datalength):
	  EntityTableItemMysqlBase(itemDBType, datalength)
	  {
	  }

	virtual ~EntityTableItemMysql_UNICODE(){};

	uint8 type()const{ return TABLE_ITEM_TYPE_UNICODE; }

	/**
		同步entity表到数据库中
	*/
	virtual bool syncToDB();

	/**
		更新数据
	*/
	virtual bool updateItem(DBID dbid, MemoryStream* s, ScriptDefModule* pModule);

	/**
		获取需要存储的表名， 字段名和转换为sql存储时的字符串值
	*/
	virtual void getSqlItemStr(MemoryStream* s, SQL_OP_TABLE& opTable);
};


class EntityTableItemMysql_BLOB : public EntityTableItemMysqlBase
{
public:
	EntityTableItemMysql_BLOB(std::string itemDBType, uint32 datalength):
	  EntityTableItemMysqlBase(itemDBType, datalength)
	  {
	  }

	virtual ~EntityTableItemMysql_BLOB(){};

	uint8 type()const{ return TABLE_ITEM_TYPE_BLOB; }

	/**
		同步entity表到数据库中
	*/
	virtual bool syncToDB();

	/**
		更新数据
	*/
	virtual bool updateItem(DBID dbid, MemoryStream* s, ScriptDefModule* pModule);

	/**
		获取需要存储的表名， 字段名和转换为sql存储时的字符串值
	*/
	virtual void getSqlItemStr(MemoryStream* s, SQL_OP_TABLE& opTable);
};

class EntityTableItemMysql_VECTOR2 : public EntityTableItemMysqlBase
{
public:
	EntityTableItemMysql_VECTOR2(std::string itemDBType, uint32 datalength):
	  EntityTableItemMysqlBase(itemDBType, datalength)
	  {
	  }

	virtual ~EntityTableItemMysql_VECTOR2(){};

	uint8 type()const{ return TABLE_ITEM_TYPE_VECTOR2; }
	
	virtual bool isSameKey(std::string key);

	/**
		同步entity表到数据库中
	*/
	virtual bool syncToDB();

	/**
		更新数据
	*/
	virtual bool updateItem(DBID dbid, MemoryStream* s, ScriptDefModule* pModule);

	/**
		获取需要存储的表名， 字段名和转换为sql存储时的字符串值
	*/
	virtual void getSqlItemStr(MemoryStream* s, SQL_OP_TABLE& opTable);
};

class EntityTableItemMysql_VECTOR3 : public EntityTableItemMysqlBase
{
public:
	EntityTableItemMysql_VECTOR3(std::string itemDBType, uint32 datalength):
	  EntityTableItemMysqlBase(itemDBType, datalength)
	  {
	  }

	virtual ~EntityTableItemMysql_VECTOR3(){};

	uint8 type()const{ return TABLE_ITEM_TYPE_VECTOR3; }

	virtual bool isSameKey(std::string key);

	/**
		同步entity表到数据库中
	*/
	virtual bool syncToDB();

	/**
		更新数据
	*/
	virtual bool updateItem(DBID dbid, MemoryStream* s, ScriptDefModule* pModule);

	/**
		获取需要存储的表名， 字段名和转换为sql存储时的字符串值
	*/
	virtual void getSqlItemStr(MemoryStream* s, SQL_OP_TABLE& opTable);
};

class EntityTableItemMysql_VECTOR4 : public EntityTableItemMysqlBase
{
public:
	EntityTableItemMysql_VECTOR4(std::string itemDBType, uint32 datalength):
	  EntityTableItemMysqlBase(itemDBType, datalength)
	  {
	  }

	virtual ~EntityTableItemMysql_VECTOR4(){};

	uint8 type()const{ return TABLE_ITEM_TYPE_VECTOR4; }

	virtual bool isSameKey(std::string key);

	/**
		同步entity表到数据库中
	*/
	virtual bool syncToDB();

	/**
		更新数据
	*/
	virtual bool updateItem(DBID dbid, MemoryStream* s, ScriptDefModule* pModule);

	/**
		获取需要存储的表名， 字段名和转换为sql存储时的字符串值
	*/
	virtual void getSqlItemStr(MemoryStream* s, SQL_OP_TABLE& opTable);
};

class EntityTableItemMysql_MAILBOX : public EntityTableItemMysqlBase
{
public:
	EntityTableItemMysql_MAILBOX(std::string itemDBType, uint32 datalength):
	  EntityTableItemMysqlBase(itemDBType, datalength)
	  {
	  }

	virtual ~EntityTableItemMysql_MAILBOX(){};

	uint8 type()const{ return TABLE_ITEM_TYPE_MAILBOX; }

	/**
		同步entity表到数据库中
	*/
	virtual bool syncToDB();

	/**
		更新数据
	*/
	virtual bool updateItem(DBID dbid, MemoryStream* s, ScriptDefModule* pModule);

	/**
		获取需要存储的表名， 字段名和转换为sql存储时的字符串值
	*/
	virtual void getSqlItemStr(MemoryStream* s, SQL_OP_TABLE& opTable);
};

class EntityTableItemMysql_ARRAY : public EntityTableItemMysqlBase
{
public:
	EntityTableItemMysql_ARRAY(std::string itemDBType, uint32 datalength):
	  EntityTableItemMysqlBase(itemDBType, datalength),
	  pChildTable_(NULL)
	  {
	  }

	virtual ~EntityTableItemMysql_ARRAY(){};

	virtual bool isSameKey(std::string key);

	/**
		初始化
	*/
	virtual bool initialize(DBInterface* dbi, const PropertyDescription* pPropertyDescription, 
		const DataType* pDataType, std::string name);

	uint8 type()const{ return TABLE_ITEM_TYPE_ARRAY; }

	/**
		同步entity表到数据库中
	*/
	virtual bool syncToDB();

	/**
		更新数据
	*/
	virtual bool updateItem(DBID dbid, MemoryStream* s, ScriptDefModule* pModule);

	/**
		获取需要存储的表名， 字段名和转换为sql存储时的字符串值
	*/
	virtual void getSqlItemStr(MemoryStream* s, SQL_OP_TABLE& opTable);
protected:
	EntityTable* pChildTable_;
};

class EntityTableItemMysql_FIXED_DICT : public EntityTableItemMysqlBase
{
public:
	EntityTableItemMysql_FIXED_DICT(std::string itemDBType, uint32 datalength):
	  EntityTableItemMysqlBase(itemDBType, datalength)
	  {
	  }

	virtual ~EntityTableItemMysql_FIXED_DICT(){};

	typedef std::vector< std::pair< std::string, std::tr1::shared_ptr<EntityTableItem> > > FIXEDDICT_KEYTYPES;

	uint8 type()const{ return TABLE_ITEM_TYPE_FIXEDDICT; }

	virtual bool isSameKey(std::string key);

	/**
		初始化
	*/
	virtual bool initialize(DBInterface* dbi, const PropertyDescription* pPropertyDescription, 
		const DataType* pDataType, std::string name);

	/**
		同步entity表到数据库中
	*/
	virtual bool syncToDB();

	/**
		更新数据
	*/
	virtual bool updateItem(DBID dbid, MemoryStream* s, ScriptDefModule* pModule);

	/**
		获取需要存储的表名， 字段名和转换为sql存储时的字符串值
	*/
	virtual void getSqlItemStr(MemoryStream* s, SQL_OP_TABLE& opTable);
protected:
	EntityTableItemMysql_FIXED_DICT::FIXEDDICT_KEYTYPES			keyTypes_;		// 这个固定字典里的各个key的类型
};


/*
	维护entity在数据库中的表
*/
class EntityTableMysql : public EntityTable
{
public:
	EntityTableMysql();
	virtual ~EntityTableMysql();
	
	/**
		初始化
	*/
	virtual bool initialize(DBInterface* dbi, ScriptDefModule* sm, std::string name);

	/**
		同步entity表到数据库中
	*/
	virtual bool syncToDB();

	/** 
		创建一个表item
	*/
	virtual EntityTableItem* createItem(std::string type);

	DBID updateTable(DBID dbid, MemoryStream* s, ScriptDefModule* pModule);

	/**
		获取需要存储的表名， 字段名和转换为sql存储时的字符串值
	*/
	virtual void getSqlItemStr(MemoryStream* s, SQL_OP_TABLE& opTable);
protected:
	
};


}

#endif // __KBE_ENTITY_TABLE_MYSQL__
