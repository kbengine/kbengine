// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_DB_THREAD_POOL_H
#define KBE_DB_THREAD_POOL_H

// common include	
// #define NDEBUG
#include "db_tasks.h"
#include "common/common.h"
#include "common/memorystream.h"
#include "thread/threadtask.h"
#include "helper/debug_helper.h"
#include "thread/threadpool.h"

namespace KBEngine{ 

/*
	数据库线程任务buffer
*/
class TPThread;

class DBThreadPool : public thread::ThreadPool
{
public:
	DBThreadPool(const std::string& dbinterfaceName);
	~DBThreadPool();

	virtual thread::TPThread* createThread(int threadWaitSecond = 0, bool threadStartsImmediately = true);

	virtual std::string name() const{ return std::string("DBThreadPool/") + dbinterfaceName_; }

protected:
	std::string dbinterfaceName_;
};

}
#endif
