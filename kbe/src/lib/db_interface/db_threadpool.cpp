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
#include "db_threadpool.h"
#include "db_tasks.h"
#include "thread/threadtask.h"
#include "db_interface/db_interface.h"
#include "thread/threadpool.h"
#include "thread/threadguard.h"

namespace KBEngine{

class DBThread : public thread::TPThread
{
public:
	DBThread(thread::ThreadPool* threadPool, int threadWaitSecond = 0):
	thread::TPThread(threadPool, threadWaitSecond),
	_pDBInterface(NULL)
	{
	}

	virtual void onStart()
	{
		DBUtil::initThread();
		_pDBInterface = DBUtil::createInterface(false);
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
			DBUtil::finiThread();
		}

		DEBUG_MSG(fmt::format("DBThread::onEnd(): {0:p}!\n", (void*)this));
	}

	~DBThread()
	{
	}
	
	virtual thread::TPTask* tryGetTask(void)
	{
		DBTaskBase* pDBTask = static_cast<DBTaskBase*>(task())->tryGetNextTask();
		if(pDBTask != NULL)
		{
			return pDBTask;
		}

		return thread::TPThread::tryGetTask();
	}

	virtual void onProcessTaskStart(thread::TPTask* pTask)
	{
		static_cast<DBTaskBase*>(pTask)->pdbi(_pDBInterface);
		_pDBInterface->lock();
	}

	virtual void processTask(thread::TPTask* pTask)
	{ 
		bool retry;

		do
		{
			retry = false;
			try
			{
				thread::TPThread::processTask(pTask); 
			}
			catch (std::exception & e)
			{
				if(!_pDBInterface->processException(e))
					break;
			}
		}
		while (retry);
	}

	virtual void onProcessTaskEnd(thread::TPTask* pTask)
	{
		static_cast<DBTaskBase*>(pTask)->pdbi(_pDBInterface);
		_pDBInterface->unlock();
	}
private:
	DBInterface* _pDBInterface;
};

//-------------------------------------------------------------------------------------
DBThreadPool::DBThreadPool():
thread::ThreadPool()
{
}

//-------------------------------------------------------------------------------------
DBThreadPool::~DBThreadPool()
{
}

//-------------------------------------------------------------------------------------
thread::TPThread* DBThreadPool::createThread(int threadWaitSecond)
{
	DBThread* tptd = new DBThread(this, threadWaitSecond);
	tptd->createThread();
	return tptd;
}	

//-------------------------------------------------------------------------------------
}
