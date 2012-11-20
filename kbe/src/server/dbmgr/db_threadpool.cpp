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
#include "db_threadpool.hpp"
#include "thread/threadtask.hpp"
#include "dbmgr_lib/db_interface.hpp"
#include "thread/threadpool.hpp"
#include "thread/threadguard.hpp"

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

		DEBUG_MSG("DBThread:: %p started!\n", this);
	}

	~DBThread()
	{
		if(_pDBInterface)
		{
			_pDBInterface->detach();
			SAFE_RELEASE(_pDBInterface);
			DBUtil::finiThread();
		}

		DEBUG_MSG("DBThread:: %p end!\n", this);
	}

	virtual void onProcessTask(thread::TPTask* pTask)
	{
		static_cast<DBTask*>(pTask)->pdbi(_pDBInterface);
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
