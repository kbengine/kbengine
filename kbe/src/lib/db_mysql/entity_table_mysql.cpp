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
bool EntityTableMysql::initialize(ScriptDefModule* sm)
{
	// 获取表名
	tableName(sm->getName());

	// 找到所有存储属性并且创建出所有的字段
	ScriptDefModule::PROPERTYDESCRIPTION_MAP& pdescrsMap = sm->getPersistentPropertyDescriptions();
	ScriptDefModule::PROPERTYDESCRIPTION_MAP::const_iterator iter = pdescrsMap.begin();
	for(; iter != pdescrsMap.end(); iter++)
	{
		PropertyDescription* pdescrs = iter->second;
		EntityTableItem* pETItem = this->createItem();
		bool ret = pETItem->initialize(pdescrs);
		
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

	EntityTable::TABLEITEM_MAP::iterator iter = tableItems_.begin();
	for(; iter != tableItems_.end(); iter++)
	{
		if(!iter->second->syncToDB())
			return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------
EntityTableItem* EntityTableMysql::createItem()
{
	return new EntityTableItemMysql();
}

//-------------------------------------------------------------------------------------
bool EntityTableItemMysql::initialize(const PropertyDescription* p)
{
	utype(p->getUType());
	itemName(p->getName());
	return true;
}

//-------------------------------------------------------------------------------------
bool EntityTableItemMysql::syncToDB()
{
	DEBUG_MSG("EntityTableItemMysql::syncToDB(): %s.\n", itemName());
	return true;
}

//-------------------------------------------------------------------------------------
}
