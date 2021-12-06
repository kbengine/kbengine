// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com
#include "db_threadpool.h"
#include "db_tasks.h"
#include "thread/threadtask.h"
#include "db_interface/db_interface.h"
#include "thread/threadpool.h"
#include "thread/threadguard.h"

namespace KBEngine{

class DBThread : public Thread::TPThread
{
public:
	DBThread(const std::string& dbinterfaceName, Thread::ThreadPool* threadPool, int threadWaitSecond = 0) :
	Thread::TPThread(threadPool, threadWaitSecond),
	_pDBInterface(NULL),
	dbinterfaceName_(dbinterfaceName)
	{
	}

	virtual void onStart()
	{
		DBUtil::initThread(dbinterfaceName_);
		_pDBInterface = DBUtil::createInterface(dbinterfaceName_.c_str(), false);
		if(_pDBInterface == NULL)
		{
			ERROR_MSG("DBThread:: can't create dbinterface!\n");
		}

		DEBUG_MSG(fmt::format("DBThread::onStart(): {0:p}!\n", (void*)this));
	}

	virtual void onEnd()
	{
		if(_pDBInterface)
		{
			_pDBInterface->detach();
			SAFE_RELEASE(_pDBInterface);
			DBUtil::finiThread(dbinterfaceName_);
		}

		DEBUG_MSG(fmt::format("DBThread::onEnd(): {0:p}!\n", (void*)this));
	}

	~DBThread()
	{
	}
	
	virtual Thread::TPTask* tryGetTask(void)
	{
		if (task())
		{
			DBTaskBase* pDBTask = static_cast<DBTaskBase*>(task())->tryGetNextTask();
			if (pDBTask != NULL)
			{
				return pDBTask;
			}
		}

		return Thread::TPThread::tryGetTask();
	}

	virtual void onProcessTaskStart(Thread::TPTask* pTask)
	{
		static_cast<DBTaskBase*>(pTask)->pdbi(_pDBInterface);
		_pDBInterface->lock();
	}

	virtual void processTask(Thread::TPTask* pTask)
	{ 
		bool retry;

		do
		{
			retry = false;

			try
			{
				Thread::TPThread::processTask(pTask);
			}
			catch (std::exception & e)
			{
				retry = _pDBInterface->processException(e);
			}

		} while (retry);
	}

	virtual void onProcessTaskEnd(Thread::TPTask* pTask)
	{
		static_cast<DBTaskBase*>(pTask)->pdbi(_pDBInterface);
		_pDBInterface->unlock();
	}

private:
	DBInterface* _pDBInterface;
	std::string dbinterfaceName_;
};

//-------------------------------------------------------------------------------------
DBThreadPool::DBThreadPool(const std::string& dbinterfaceName) :
Thread::ThreadPool(),
dbinterfaceName_(dbinterfaceName)
{
}

//-------------------------------------------------------------------------------------
DBThreadPool::~DBThreadPool()
{
}

//-------------------------------------------------------------------------------------
Thread::TPThread* DBThreadPool::createThread(int threadWaitSecond, bool threadStartsImmediately)
{
	DBThread* tptd = new DBThread(dbinterfaceName_, this, threadWaitSecond);

	if (threadStartsImmediately)
		tptd->createThread();

	return tptd;
}	

//-------------------------------------------------------------------------------------
}
