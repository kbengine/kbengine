/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2016 KBEngine.

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

#ifndef KBE_BUFFERED_DBTASKS_H
#define KBE_BUFFERED_DBTASKS_H

// common include	
// #define NDEBUG
#include "dbtasks.h"
#include "common/common.h"
#include "common/memorystream.h"
#include "thread/threadtask.h"
#include "helper/debug_helper.h"

namespace KBEngine{ 

/*
	���ݿ��߳�����buffer
*/

class Buffered_DBTasks
{
public:
	typedef std::multimap<DBID, EntityDBTask*> DBID_TASKS_MAP;  
	typedef std::multimap<ENTITY_ID, EntityDBTask*> ENTITYID_TASKS_MAP;  
	
	Buffered_DBTasks();
	virtual ~Buffered_DBTasks();
	
	void addTask(EntityDBTask* pTask);

	EntityDBTask* tryGetNextTask(EntityDBTask* pTask);

	size_t size(){ return dbid_tasks_.size() + entityid_tasks_.size(); }

	void dbInterfaceName(const std::string& dbInterfaceName) { dbInterfaceName_ = dbInterfaceName; }
	const std::string& dbInterfaceName() { return dbInterfaceName_; }

	/**
		�ṩ��watcherʹ��
	*/
	uint32 dbid_tasksSize()
	{ 
		mutex_.lockMutex();
		uint32 ret = (uint32)dbid_tasks_.size(); 
		mutex_.unlockMutex();
		return ret;
	}

	/**
		�ṩ��watcherʹ��
	*/
	uint32 entityid_tasksSize()
	{ 
		mutex_.lockMutex();
		uint32 ret = (uint32)entityid_tasks_.size(); 
		mutex_.unlockMutex();
		return ret;
	}

	/**
		�ṩ��watcherʹ��
	*/
	std::string printBuffered_dbid();
	std::string printBuffered_entityID();
	std::string printBuffered_dbid_();
	std::string printBuffered_entityID_();

protected:
	bool hasTask_(DBID dbid);
	bool hasTask_(ENTITY_ID entityID);

	DBID_TASKS_MAP dbid_tasks_;
	ENTITYID_TASKS_MAP entityid_tasks_;

	KBEngine::thread::ThreadMutex mutex_;

	std::string dbInterfaceName_;
};

}

#endif // KBE_BUFFERED_DBTASKS_H
