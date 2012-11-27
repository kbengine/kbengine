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

#include "entity_table.hpp"
#include "db_interface.hpp"
#include "entitydef/entitydef.hpp"
#include "entitydef/scriptdef_module.hpp"
#include "thread/threadguard.hpp"

namespace KBEngine { 
KBE_SINGLETON_INIT(EntityTables);

EntityTables g_EntityTables;

//-------------------------------------------------------------------------------------
void EntityTable::addItem(EntityTableItem* pItem)
{
	tableItems_[pItem->utype()].reset(pItem);
	tableFixedOrderItems_.push_back(pItem);
}

//-------------------------------------------------------------------------------------
EntityTableItem* EntityTable::findItem(int32/*ENTITY_PROPERTY_UID*/ utype)
{
	TABLEITEM_MAP::iterator iter = tableItems_.find(utype);
	if(iter != tableItems_.end())
	{
		return iter->second.get();
	}

	return NULL;
}

//-------------------------------------------------------------------------------------
DBID EntityTable::writeTable(DBInterface* dbi, DBID dbid, MemoryStream* s, ScriptDefModule* pModule)
{
	while(s->opsize() > 0)
	{
		ENTITY_PROPERTY_UID pid;
		(*s) >> pid;
		
		EntityTableItem* pTableItem = this->findItem(pid);
		if(pTableItem == NULL)
		{
			ERROR_MSG(boost::format("EntityTable::writeTable: not found item[%1%].\n") % pid);
			return dbid;
		}

		if(!pTableItem->writeItem(dbi, dbid, s, pModule))
			return dbid;
	};

	return dbid;
}

//-------------------------------------------------------------------------------------
bool EntityTable::removeEntity(DBInterface* dbi, DBID dbid, ScriptDefModule* pModule)
{
	return true;
}

//-------------------------------------------------------------------------------------
bool EntityTable::queryTable(DBInterface* dbi, DBID dbid, MemoryStream* s, ScriptDefModule* pModule)
{
	std::vector<EntityTableItem*>::iterator iter = tableFixedOrderItems_.begin();
	for(; iter != tableFixedOrderItems_.end(); iter++)
	{
		if(!(*iter)->queryTable(dbi, dbid, s, pModule))
			return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------
EntityTables::EntityTables()
{
}

//-------------------------------------------------------------------------------------
EntityTables::~EntityTables()
{
	tables_.clear();
}

//-------------------------------------------------------------------------------------
bool EntityTables::load(DBInterface* dbi)
{
	EntityDef::SCRIPT_MODULES smodules = EntityDef::getScriptModules();
	EntityDef::SCRIPT_MODULES::const_iterator iter = smodules.begin();
	for(; iter != smodules.end(); iter++)
	{
		ScriptDefModule* pSM = (*iter).get();
		EntityTable* pEtable = dbi->createEntityTable();
		bool ret = pEtable->initialize(pSM, pSM->getName());

		if(!ret)
		{
			delete pEtable;
			return false;
		}

		tables_[pEtable->tableName()].reset(pEtable);
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool EntityTables::syncToDB(DBInterface* dbi)
{
	// 开始同步所有表
	EntityTables::TABLES_MAP::iterator kiter = kbe_tables_.begin();
	for(; kiter != kbe_tables_.end(); kiter++)
	{
		if(!kiter->second->syncToDB(dbi))
			return false;
	}

	EntityTables::TABLES_MAP::iterator iter = tables_.begin();
	for(; iter != tables_.end(); iter++)
	{
		if(!iter->second->syncToDB(dbi))
			return false;
	}

	std::vector<std::string> dbTableNames;
	dbi->getTableNames(dbTableNames, "");

	// 检查是否有需要删除的表
	std::vector<std::string>::iterator iter0 = dbTableNames.begin();
	for(; iter0 != dbTableNames.end(); iter0++)
	{
		std::string tname = (*iter0);
		if(std::string::npos == tname.find(ENTITY_TABLE_PERFIX"_"))
			continue;

		KBEngine::strutil::kbe_replace(tname, ENTITY_TABLE_PERFIX"_", "");
		EntityTables::TABLES_MAP::iterator iter = tables_.find(tname);
		if(iter == tables_.end())
		{
			if(!dbi->dropEntityTableFromDB((std::string(ENTITY_TABLE_PERFIX"_") + tname).c_str()))
				return false;
		}
	}

	return true;
}

//-------------------------------------------------------------------------------------
void EntityTables::addTable(EntityTable* pTable)
{
	TABLES_MAP::iterator iter = tables_.begin();

	for(; iter != tables_.end(); iter++)
	{
		if(iter->first == pTable->tableName())
		{
			KBE_ASSERT(false && "table exist!\n");
			return;
		}
	}

	tables_[pTable->tableName()].reset(pTable);
}

//-------------------------------------------------------------------------------------
EntityTable* EntityTables::findTable(std::string name)
{
	TABLES_MAP::iterator iter = tables_.find(name);
	if(iter != tables_.end())
	{
		return iter->second.get();
	}

	return NULL;
};

//-------------------------------------------------------------------------------------
void EntityTables::addKBETable(EntityTable* pTable)
{
	TABLES_MAP::iterator iter = kbe_tables_.begin();

	for(; iter != kbe_tables_.end(); iter++)
	{
		if(iter->first == pTable->tableName())
		{
			KBE_ASSERT(false && "table exist!\n");
			return;
		}
	}

	kbe_tables_[pTable->tableName()].reset(pTable);
}

//-------------------------------------------------------------------------------------
EntityTable* EntityTables::findKBETable(std::string name)
{
	TABLES_MAP::iterator iter = kbe_tables_.find(name);
	if(iter != kbe_tables_.end())
	{
		return iter->second.get();
	}

	return NULL;
};

//-------------------------------------------------------------------------------------
DBID EntityTables::writeEntity(DBInterface* dbi, DBID dbid, MemoryStream* s, ScriptDefModule* pModule)
{
	EntityTable* pTable = this->findTable(pModule->getName());
	KBE_ASSERT(pTable != NULL);

	return pTable->writeTable(dbi, dbid, s, pModule);
}

//-------------------------------------------------------------------------------------
bool EntityTables::removeEntity(DBInterface* dbi, DBID dbid, ScriptDefModule* pModule)
{
	EntityTable* pTable = this->findTable(pModule->getName());
	KBE_ASSERT(pTable != NULL);

	return pTable->removeEntity(dbi, dbid, pModule);
}

//-------------------------------------------------------------------------------------
bool EntityTables::queryEntity(DBInterface* dbi, DBID dbid, MemoryStream* s, ScriptDefModule* pModule)
{
	EntityTable* pTable = this->findTable(pModule->getName());
	KBE_ASSERT(pTable != NULL);

	return pTable->queryTable(dbi, dbid, s, pModule);
}

//-------------------------------------------------------------------------------------
}
