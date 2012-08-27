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

#include "cstdkbe/cstdkbe.hpp"
#include "cstdkbe/singleton.hpp"
#include "helper/debug_helper.hpp"
#include "dbmgr_lib/entity_table.hpp"

namespace KBEngine { 

class ScriptDefModule;

/*
	维护entity在数据库中的表中的一个字段
*/
class EntityTableItemMysql_INT : public EntityTableItem
{
public:
	EntityTableItemMysql_INT(){};
	virtual ~EntityTableItemMysql_INT(){};

	/**
		初始化
	*/
	virtual bool initialize(DBInterface* dbi, const PropertyDescription* pPropertyDescription, const DataType* pDataType);

	/**
		同步entity表到数据库中
	*/
	virtual bool syncToDB();
protected:
};

class EntityTableItemMysql_UINT : public EntityTableItemMysql_INT
{
public:
	/**
		同步entity表到数据库中
	*/
	virtual bool syncToDB();
};

class EntityTableItemMysql_INT64 : public EntityTableItemMysql_INT
{
public:
	/**
		同步entity表到数据库中
	*/
	virtual bool syncToDB();
};

class EntityTableItemMysql_UINT64 : public EntityTableItemMysql_INT
{
public:
	/**
		同步entity表到数据库中
	*/
	virtual bool syncToDB();
};

class EntityTableItemMysql_STRING : public EntityTableItemMysql_INT
{
public:
	/**
		同步entity表到数据库中
	*/
	virtual bool syncToDB();
};

class EntityTableItemMysql_FLOAT : public EntityTableItemMysql_INT
{
public:
	/**
		同步entity表到数据库中
	*/
	virtual bool syncToDB();
};

class EntityTableItemMysql_DOUBLE : public EntityTableItemMysql_INT
{
public:
	/**
		同步entity表到数据库中
	*/
	virtual bool syncToDB();
};

class EntityTableItemMysql_ARRAY : public EntityTableItemMysql_INT
{
public:
	/**
		初始化
	*/
	virtual bool initialize(DBInterface* dbi, const PropertyDescription* pPropertyDescription, const DataType* pDataType);

	/**
		同步entity表到数据库中
	*/
	virtual bool syncToDB();
protected:
	std::tr1::shared_ptr<EntityTableItem> arrayTableItem_;
};

class EntityTableItemMysql_FIXED_DICT : public EntityTableItemMysql_INT
{
public:
	typedef std::map<std::string, std::tr1::shared_ptr<EntityTableItem>> FIXEDDICT_KEYTYPE_MAP;

	/**
		初始化
	*/
	virtual bool initialize(DBInterface* dbi, const PropertyDescription* pPropertyDescription, const DataType* pDataType);

	/**
		同步entity表到数据库中
	*/
	virtual bool syncToDB();
protected:
	FIXEDDICT_KEYTYPE_MAP			keyTypes_;		// 这个固定字典里的各个key的类型
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
	virtual bool initialize(DBInterface* dbi, ScriptDefModule* sm);

	/**
		同步entity表到数据库中
	*/
	virtual bool syncToDB();

	/** 
		创建一个表item
	*/
	virtual EntityTableItem* createItem(std::string type);
protected:
};


}

#endif // __KBE_ENTITY_TABLE_MYSQL__
