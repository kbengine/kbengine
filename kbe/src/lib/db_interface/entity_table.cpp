// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include "db_tasks.h"
#include "db_threadpool.h"
#include "entity_table.h"
#include "db_interface.h"
#include "entitydef/entitydef.h"
#include "entitydef/scriptdef_module.h"
#include "thread/threadguard.h"

namespace KBEngine { 

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
DBID EntityTable::writeTable(DBInterface* pdbi, DBID dbid, int8 shouldAutoLoad, MemoryStream* s, ScriptDefModule* pModule)
{
	while(s->length() > 0)
	{
		ENTITY_PROPERTY_UID pid;
		(*s) >> pid;
		
		EntityTableItem* pTableItem = this->findItem(pid);
		if(pTableItem == NULL)
		{
			ERROR_MSG(fmt::format("EntityTable::writeTable: not found item[{}].\n", pid));
			return dbid;
		}

		if(!pTableItem->writeItem(pdbi, dbid, s, pModule))
		{
			// 设置实体是否自动加载
			if(shouldAutoLoad > -1)
				entityShouldAutoLoad(pdbi, dbid, shouldAutoLoad > 0);

			return dbid;
		}
	};

	// 设置实体是否自动加载
	if(shouldAutoLoad > -1)
		entityShouldAutoLoad(pdbi, dbid, shouldAutoLoad > 0);

	return dbid;
}

//-------------------------------------------------------------------------------------
bool EntityTable::removeEntity(DBInterface* pdbi, DBID dbid, ScriptDefModule* pModule)
{
	return true;
}

//-------------------------------------------------------------------------------------
bool EntityTable::queryTable(DBInterface* pdbi, DBID dbid, MemoryStream* s, ScriptDefModule* pModule)
{
	std::vector<EntityTableItem*>::iterator iter = tableFixedOrderItems_.begin();
	for(; iter != tableFixedOrderItems_.end(); ++iter)
	{
		if(!(*iter)->queryTable(pdbi, dbid, s, pModule))
			return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------
EntityTables::ENTITY_TABLES_MAP EntityTables::sEntityTables;

EntityTables& EntityTables::findByInterfaceName(const std::string& dbInterfaceName)
{
	ENTITY_TABLES_MAP::iterator iter = EntityTables::sEntityTables.find(dbInterfaceName);
	if (iter != EntityTables::sEntityTables.end())
	{
		return iter->second;
	}

	EntityTables& entityTables = EntityTables::sEntityTables[dbInterfaceName];
	entityTables.dbInterfaceName(dbInterfaceName);
	return entityTables;
}

//-------------------------------------------------------------------------------------
EntityTables::EntityTables():
tables_(),
kbe_tables_(),
numSyncTables_(0),
syncTablesError_(false),
dbInterfaceName_()
{
}

//-------------------------------------------------------------------------------------
EntityTables::~EntityTables()
{
	tables_.clear();
	kbe_tables_.clear();
}

//-------------------------------------------------------------------------------------
bool EntityTables::load(DBInterface* pdbi)
{
	EntityDef::SCRIPT_MODULES smodules = EntityDef::getScriptModules();
	EntityDef::SCRIPT_MODULES::const_iterator iter = smodules.begin();
	for(; iter != smodules.end(); ++iter)
	{
		ScriptDefModule* pSM = (*iter).get();
		EntityTable* pEtable = pdbi->createEntityTable(this);

		if (!pEtable)
		{
			WARNING_MSG(fmt::format("EntityTables::load: {} has no archived properties and does not create an entity({}) table\n", 
				pSM->getName(), pSM->getName()));

			continue;
		}

		if (!pEtable->initialize(pSM, pSM->getName()))
		{
			delete pEtable;
			return false;
		}

		tables_[pEtable->tableName()].reset(pEtable);
	}

	return true;
}

//-------------------------------------------------------------------------------------
void EntityTables::onTableSyncSuccessfully(KBEShared_ptr<EntityTable> pEntityTable, bool error)
{
	if(error)
	{
		syncTablesError_ = true;
		return;
	}

	numSyncTables_++;
}

//-------------------------------------------------------------------------------------
void EntityTables::queryAutoLoadEntities(DBInterface* pdbi, ScriptDefModule* pModule, 
	ENTITY_ID start, ENTITY_ID end, std::vector<DBID>& outs)
{
	EntityTable* pTable = this->findTable(pModule->getName());
	KBE_ASSERT(pTable != NULL);

	pTable->queryAutoLoadEntities(pdbi, pModule, start, end, outs);
}

//-------------------------------------------------------------------------------------
bool EntityTables::syncToDB(DBInterface* pdbi)
{
	DBThreadPool* pDBThreadPool = static_cast<DBThreadPool*>(DBUtil::pThreadPool(pdbi->name()));
	KBE_ASSERT(pDBThreadPool != NULL);
	
	int num = 0;
	try
	{
		// 开始同步所有表
		EntityTables::TABLES_MAP::iterator kiter = kbe_tables_.begin();
		for(; kiter != kbe_tables_.end(); ++kiter)
		{
			num++;
			pDBThreadPool->addTask(new DBTaskSyncTable(this, kiter->second));
		}

		EntityTables::TABLES_MAP::iterator iter = tables_.begin();
		for(; iter != tables_.end(); ++iter)
		{
			if(!iter->second->hasSync())
			{
				num++;
				pDBThreadPool->addTask(new DBTaskSyncTable(this, iter->second));
			}
		}

		while(true)
		{
			if(syncTablesError_)
				return false;

			if(numSyncTables_ == num)
				break;

			pDBThreadPool->onMainThreadTick();
			sleep(10);
		};


		std::vector<std::string> dbTableNames;
		pdbi->getTableNames(dbTableNames, "");

		// 检查是否有需要删除的表
		std::vector<std::string>::iterator iter0 = dbTableNames.begin();
		for(; iter0 != dbTableNames.end(); ++iter0)
		{
			std::string tname = (*iter0);
			if(std::string::npos == tname.find(ENTITY_TABLE_PERFIX"_"))
				continue;

			KBEngine::strutil::kbe_replace(tname, ENTITY_TABLE_PERFIX"_", "");
			EntityTables::TABLES_MAP::iterator iter = tables_.find(tname);
			if(iter == tables_.end())
			{
				if(!pdbi->dropEntityTableFromDB((std::string(ENTITY_TABLE_PERFIX"_") + tname).c_str()))
					return false;
			}
		}
	}
	catch (std::exception& e)
	{
		ERROR_MSG(fmt::format("EntityTables::syncToDB: {}\n", e.what()));
		return false;
	}

	return true;
}

//-------------------------------------------------------------------------------------
void EntityTables::addTable(EntityTable* pTable)
{
	TABLES_MAP::iterator iter = tables_.begin();

	for(; iter != tables_.end(); ++iter)
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

	for(; iter != kbe_tables_.end(); ++iter)
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
DBID EntityTables::writeEntity(DBInterface* pdbi, DBID dbid, int8 shouldAutoLoad, MemoryStream* s, ScriptDefModule* pModule)
{
	EntityTable* pTable = this->findTable(pModule->getName());
	KBE_ASSERT(pTable != NULL);

	return pTable->writeTable(pdbi, dbid, shouldAutoLoad, s, pModule);
}

//-------------------------------------------------------------------------------------
bool EntityTables::removeEntity(DBInterface* pdbi, DBID dbid, ScriptDefModule* pModule)
{
	EntityTable* pTable = this->findTable(pModule->getName());
	KBE_ASSERT(pTable != NULL);

	return pTable->removeEntity(pdbi, dbid, pModule);
}

//-------------------------------------------------------------------------------------
bool EntityTables::queryEntity(DBInterface* pdbi, DBID dbid, MemoryStream* s, ScriptDefModule* pModule)
{
	EntityTable* pTable = this->findTable(pModule->getName());
	KBE_ASSERT(pTable != NULL);

	return pTable->queryTable(pdbi, dbid, s, pModule);
}

//-------------------------------------------------------------------------------------
}
