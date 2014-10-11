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

#ifndef KBE_DB_THREAD_POOL_HPP
#define KBE_DB_THREAD_POOL_HPP

// common include	
// #define NDEBUG
#include "db_tasks.hpp"
#include "cstdkbe/cstdkbe.hpp"
#include "cstdkbe/memorystream.hpp"
#include "thread/threadtask.hpp"
#include "helper/debug_helper.hpp"
#include "thread/threadpool.hpp"

namespace KBEngine{ 

/*
	���ݿ��߳�����buffer
*/
class TPThread;

class DBThreadPool : public thread::ThreadPool
{
public:
	DBThreadPool();
	~DBThreadPool();

	virtual thread::TPThread* createThread(int threadWaitSecond = 0);

	virtual std::string name()const{ return "DBThreadPool"; }
protected:

};

}
#endif
