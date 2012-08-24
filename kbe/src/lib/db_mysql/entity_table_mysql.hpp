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
class EntityTableItemMysql : public EntityTableItem
{
public:
	EntityTableItemMysql(){};
	virtual ~EntityTableItemMysql(){};

	/**
		初始化
	*/
	virtual bool initialize(const PropertyDescription* p);

	/**
		同步entity表到数据库中
	*/
	virtual bool syncToDB();
protected:
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
	virtual bool initialize(ScriptDefModule* sm);

	/**
		同步entity表到数据库中
	*/
	virtual bool syncToDB();

	/** 
		创建一个表item
	*/
	virtual EntityTableItem* createItem();
protected:
};


}

#endif // __KBE_ENTITY_TABLE_MYSQL__
