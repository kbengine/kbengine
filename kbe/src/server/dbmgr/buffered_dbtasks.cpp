// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com
#include "dbmgr.h"
#include "buffered_dbtasks.h"
#include "thread/threadpool.h"
#include "thread/threadguard.h"
#include "db_interface/db_interface.h"
#include "server/serverconfig.h"

namespace KBEngine{

//-------------------------------------------------------------------------------------
Buffered_DBTasks::Buffered_DBTasks():
dbid_tasks_(),
entityid_tasks_(),
mutex_(),
dbInterfaceName_()
{
}

//-------------------------------------------------------------------------------------
Buffered_DBTasks::~Buffered_DBTasks()
{
}

//-------------------------------------------------------------------------------------
bool Buffered_DBTasks::hasTask_(DBID dbid)
{
	std::pair<DBID_TASKS_MAP::iterator, DBID_TASKS_MAP::iterator> range = 
		dbid_tasks_.equal_range(dbid);  

	if (range.first != range.second)
	{
		return true;
	}

	return false;
}

//-------------------------------------------------------------------------------------
bool Buffered_DBTasks::hasTask_(ENTITY_ID entityID)
{
	std::pair<ENTITYID_TASKS_MAP::iterator, ENTITYID_TASKS_MAP::iterator> range = 
		entityid_tasks_.equal_range(entityID);  

	if (range.first != range.second)
	{
		return true;
	}

	return false;
}

//-------------------------------------------------------------------------------------
void Buffered_DBTasks::addTask(EntityDBTask* pTask)
{
	mutex_.lockMutex();
	pTask->pBuffered_DBTasks(this);
	
	if(pTask->EntityDBTask_entityDBID() <= 0)
	{
		if(hasTask_(pTask->EntityDBTask_entityID()))
		{
			entityid_tasks_.insert(std::make_pair(pTask->EntityDBTask_entityID(), pTask));
			mutex_.unlockMutex();
			return;
		}

		entityid_tasks_.insert(std::make_pair(pTask->EntityDBTask_entityID(), 
			static_cast<EntityDBTask *>(NULL)));
	}
	else
	{
		if(hasTask_(pTask->EntityDBTask_entityDBID()))
		{
			dbid_tasks_.insert(std::make_pair(pTask->EntityDBTask_entityDBID(), pTask));
			mutex_.unlockMutex();
			return;
		}

		dbid_tasks_.insert(std::make_pair(pTask->EntityDBTask_entityDBID(), 
			static_cast<EntityDBTask *>(NULL)));
	}

	mutex_.unlockMutex();
	DBUtil::pThreadPool(dbInterfaceName_)->addTask(pTask);
}

//-------------------------------------------------------------------------------------
EntityDBTask* Buffered_DBTasks::tryGetNextTask(EntityDBTask* pTask)
{
	mutex_.lockMutex();

	if(g_kbeSrvConfig.getDBMgr().debugDBMgr)
	{
		DEBUG_MSG(fmt::format("Buffered_DBTasks::tryGetNextTask(): finiTask(dbid={}, entityID={}\ndbidlist={}\nentityidlist={})\n", 
			pTask->EntityDBTask_entityDBID(), pTask->EntityDBTask_entityID(), printBuffered_dbid_(), printBuffered_entityID_())); 
	}

	EntityDBTask * pNextTask = NULL;
	
	if(pTask->EntityDBTask_entityDBID() <= 0)
	{
		std::pair<ENTITYID_TASKS_MAP::iterator, ENTITYID_TASKS_MAP::iterator> range = 
			entityid_tasks_.equal_range(pTask->EntityDBTask_entityID());  

		// 如果没有任务则退出
		if (range.first == range.second)
		{
			mutex_.unlockMutex();
			return NULL;
		}
		
		ENTITYID_TASKS_MAP::iterator nextIter = range.first;
		++nextIter;

		if (nextIter != range.second)
		{
			pNextTask = nextIter->second;
		}

		entityid_tasks_.erase( range.first );
	}
	else
	{
		std::pair<DBID_TASKS_MAP::iterator, DBID_TASKS_MAP::iterator> range = 
			dbid_tasks_.equal_range(pTask->EntityDBTask_entityDBID());  

		// 如果没有任务则退出
		if (range.first == range.second)
		{
			mutex_.unlockMutex();
			return NULL;
		}
		
		DBID_TASKS_MAP::iterator nextIter = range.first;
		++nextIter;

		if (nextIter != range.second)
		{
			pNextTask = nextIter->second;
		}

		dbid_tasks_.erase( range.first );
	}
	
	if(pNextTask != NULL)
	{
		INFO_MSG(fmt::format("Buffered_DBTasks::onFiniTask: Playing buffered task for entityID={}, dbid={}, dbid_tasks_size={}, entityid_tasks_size={}.\n", 
			pNextTask->EntityDBTask_entityID(), pNextTask->EntityDBTask_entityDBID(), 
			dbid_tasks_.size(), entityid_tasks_.size()));
	}

	mutex_.unlockMutex();

	return pNextTask;
}

//-------------------------------------------------------------------------------------
std::string Buffered_DBTasks::printBuffered_dbid()
{
	std::string ret;
	mutex_.lockMutex();
	ret = printBuffered_dbid_();
	mutex_.unlockMutex();
	return ret;
}

//-------------------------------------------------------------------------------------
std::string Buffered_DBTasks::printBuffered_entityID()
{
	std::string ret;
	mutex_.lockMutex();
    ret = printBuffered_entityID_();
	mutex_.unlockMutex();
	return ret;
}

//-------------------------------------------------------------------------------------
std::string Buffered_DBTasks::printBuffered_dbid_()
{
	std::string ret;

    for (DBID_TASKS_MAP::iterator iter = dbid_tasks_.begin(); iter != dbid_tasks_.end(); iter = dbid_tasks_.upper_bound(iter->first))  
    {  
        std::pair<DBID_TASKS_MAP::iterator, DBID_TASKS_MAP::iterator> res = dbid_tasks_.equal_range(iter->first);  
		int count = 0;

        for (DBID_TASKS_MAP::iterator i = res.first; i != res.second; ++i)  
        {  
			if (i == dbid_tasks_.end())
				break;

			++count;
        } 

		ret += fmt::format("{}:{}, ", iter->first, count);
    }  

	return ret;
}

//-------------------------------------------------------------------------------------
std::string Buffered_DBTasks::printBuffered_entityID_()
{
	std::string ret;

    for (ENTITYID_TASKS_MAP::iterator iter = entityid_tasks_.begin(); iter != entityid_tasks_.end(); iter = entityid_tasks_.upper_bound(iter->first))  
    {  
		int count = 0;
        std::pair<ENTITYID_TASKS_MAP::iterator, ENTITYID_TASKS_MAP::iterator> res = entityid_tasks_.equal_range(iter->first);  

        for (ENTITYID_TASKS_MAP::iterator i = res.first; i != res.second; ++i)  
        {  
			if (i == entityid_tasks_.end())
				break;

			++count;
        }  

		ret += fmt::format("{}:{}, ", iter->first, count);
    }  

	return ret;
}

//-------------------------------------------------------------------------------------
}
