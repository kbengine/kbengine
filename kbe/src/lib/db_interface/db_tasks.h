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

#ifndef KBE_DB_TASKS_H
#define KBE_DB_TASKS_H

// common include	
// #define NDEBUG
#include "common/common.h"
#include "common/timer.h"
#include "thread/threadtask.h"

namespace KBEngine{ 

class MemoryStream;
class DBInterface;
class EntityTable;

/*
	数据库线程任务基础类
*/

class DBTaskBase : public thread::TPTask
{
public:

	DBTaskBase():
	initTime_(timestamp())
	{
	}

	virtual ~DBTaskBase(){}
	virtual bool process();
	virtual bool db_thread_process() = 0;
	virtual DBTaskBase* tryGetNextTask(){ return NULL; }
	virtual thread::TPTask::TPTaskState presentMainThread();

	virtual void pdbi(DBInterface* ptr){ pdbi_ = ptr; }

	uint64 initTime() const{ return initTime_; }
protected:
	DBInterface* pdbi_;
	uint64 initTime_;
};

/**
	执行一条sql语句
*/
class DBTaskSyncTable : public DBTaskBase
{
public:
	DBTaskSyncTable(KBEShared_ptr<EntityTable> pEntityTable);
	virtual ~DBTaskSyncTable();
	virtual bool db_thread_process();
	virtual thread::TPTask::TPTaskState presentMainThread();
protected:
	KBEShared_ptr<EntityTable> pEntityTable_;
	bool success_;
};


}
#endif // KBE_DB_TASKS_H
