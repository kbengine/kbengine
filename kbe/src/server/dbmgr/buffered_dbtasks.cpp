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
#include "dbmgr.hpp"
#include "buffered_dbtasks.hpp"
#include "thread/threadpool.hpp"
#include "thread/threadguard.hpp"

namespace KBEngine{

//-------------------------------------------------------------------------------------
Buffered_DBTasks::Buffered_DBTasks():
dbid_tasks_(),
entityid_tasks_()
{
}

//-------------------------------------------------------------------------------------
Buffered_DBTasks::~Buffered_DBTasks()
{
}

//-------------------------------------------------------------------------------------
bool Buffered_DBTasks::hasTask(DBID dbid)
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
bool Buffered_DBTasks::hasTask(ENTITY_ID entityID)
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
	pTask->pBuffered_DBTasks(this);
	
	if(pTask->EntityDBTask_entityDBID() <= 0)
	{
		if(hasTask(pTask->EntityDBTask_entityID()))
		{
			entityid_tasks_.insert(std::make_pair(pTask->EntityDBTask_entityID(), pTask));
			return;
		}

		entityid_tasks_.insert(std::make_pair(pTask->EntityDBTask_entityID(), 
			static_cast<EntityDBTask *>(NULL)));
	}
	else
	{
		if(hasTask(pTask->EntityDBTask_entityDBID()))
		{
			dbid_tasks_.insert(std::make_pair(pTask->EntityDBTask_entityDBID(), pTask));
			return;
		}

		dbid_tasks_.insert(std::make_pair(pTask->EntityDBTask_entityDBID(), 
			static_cast<EntityDBTask *>(NULL)));
	}

	Dbmgr::getSingleton().dbThreadPool().addTask(pTask);
}

//-------------------------------------------------------------------------------------
void Buffered_DBTasks::onFiniTask(EntityDBTask* pTask)
{
	EntityDBTask * pNextTask = NULL;
	
	if(pTask->EntityDBTask_entityDBID() <= 0)
	{
		std::pair<ENTITYID_TASKS_MAP::iterator, ENTITYID_TASKS_MAP::iterator> range = 
			entityid_tasks_.equal_range(pTask->EntityDBTask_entityID());  

		// 如果没有任务则退出
		if (range.first == range.second)
		{
			return;
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
			return;
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
		INFO_MSG("Buffered_DBTasks::onFiniTask: Playing buffered task for entityID=%d, dbid=%"PRDBID"\n", 
			pNextTask->EntityDBTask_entityID(), pNextTask->EntityDBTask_entityDBID());
		
		Dbmgr::getSingleton().dbThreadPool().addTask(pNextTask);
	}
}

//-------------------------------------------------------------------------------------
}
