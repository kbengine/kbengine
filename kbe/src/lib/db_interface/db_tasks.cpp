// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include "db_tasks.h"
#include "db_interface.h"
#include "entity_table.h"
#include "thread/threadpool.h"
#include "common/memorystream.h"

namespace KBEngine{

//-------------------------------------------------------------------------------------
bool DBTaskBase::process()
{
	uint64 startTime = timestamp();
	
	bool ret = db_thread_process();

	uint64 duration = startTime - initTime_;
	if(duration > stampsPerSecond())
	{
		WARNING_MSG(fmt::format("DBTask::process(): delay {0:.2f} seconds, try adjusting the kbengine[_defs].xml(numConnections) and MySQL(my.cnf->max_connections or innodb_flush_log_at_trx_commit)!\nsql:({1})\n", 
			(double(duration)/stampsPerSecondD()), pdbi_->lastquery()));
	}

	duration = timestamp() - startTime;
	if (duration > stampsPerSecond() * 0.2f)
	{
		WARNING_MSG(fmt::format("DBTask::process(): took {:.2f} seconds\nsql:({})\n", 
			(double(duration)/stampsPerSecondD()), pdbi_->lastquery()));
	}

	return ret;
}

//-------------------------------------------------------------------------------------
thread::TPTask::TPTaskState DBTaskBase::presentMainThread()
{
	return thread::TPTask::TPTASK_STATE_COMPLETED; 
}

//-------------------------------------------------------------------------------------
DBTaskSyncTable::DBTaskSyncTable(EntityTables* pEntityTables, KBEShared_ptr<EntityTable> pEntityTable) :
pEntityTable_(pEntityTable),
success_(false),
pEntityTables_(pEntityTables)
{
}

//-------------------------------------------------------------------------------------
DBTaskSyncTable::~DBTaskSyncTable()
{
}

//-------------------------------------------------------------------------------------
bool DBTaskSyncTable::db_thread_process()
{
	success_ = !pEntityTable_->syncToDB(pdbi_);
	return false;
}

//-------------------------------------------------------------------------------------
thread::TPTask::TPTaskState DBTaskSyncTable::presentMainThread()
{
	pEntityTables_->onTableSyncSuccessfully(pEntityTable_, success_);
	return thread::TPTask::TPTASK_STATE_COMPLETED; 
}

//-------------------------------------------------------------------------------------
}
