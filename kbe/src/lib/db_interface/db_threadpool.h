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
	DBThreadPool();
	~DBThreadPool();

	virtual thread::TPThread* createThread(int threadWaitSecond = 0);

	virtual std::string name() const{ return "DBThreadPool"; }
protected:

};

}
#endif
