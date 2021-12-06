// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_DB_TASKS_H
#define KBE_DB_TASKS_H

#include "common/common.h"
#include "common/timer.h"
#include "thread/threadtask.h"

namespace KBEngine{ 

class MemoryStream;
class DBInterface;
class EntityTable;
class EntityTables;

/*
	���ݿ��߳����������
*/

class DBTaskBase : public Thread::TPTask
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
	virtual Thread::TPTask::TPTaskState presentMainThread();

	virtual void pdbi(DBInterface* ptr){ pdbi_ = ptr; }

	uint64 initTime() const{ return initTime_; }

protected:
	DBInterface* pdbi_;
	uint64 initTime_;
};

/**
	ִ��һ��sql���
*/
class DBTaskSyncTable : public DBTaskBase
{
public:
	DBTaskSyncTable(EntityTables* pEntityTables, KBEShared_ptr<EntityTable> pEntityTable);
	virtual ~DBTaskSyncTable();
	virtual bool db_thread_process();
	virtual Thread::TPTask::TPTaskState presentMainThread();

protected:
	KBEShared_ptr<EntityTable> pEntityTable_;
	bool success_;
	EntityTables* pEntityTables_;
};


}
#endif // KBE_DB_TASKS_H
