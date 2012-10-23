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

#ifndef __DBTASKS_H__
#define __DBTASKS_H__

// common include	
// #define NDEBUG
#include "cstdkbe/cstdkbe.hpp"
#include "cstdkbe/memorystream.hpp"
#include "thread/threadtask.hpp"
#include "helper/debug_helper.hpp"

namespace KBEngine{ 
/*
	数据库线程任务基础类
*/

class DBTask : public thread::TPTask
{
public:
	DBTask(MemoryStream& datas);
	virtual ~DBTask();
	virtual bool process() = 0;
	virtual void presentMainThread(){}
protected:
	MemoryStream* pDatas_;
};

class DBTaskExecuteRawDatabaseCommand : public DBTask
{
	DBTaskExecuteRawDatabaseCommand(MemoryStream& datas);
	virtual ~DBTaskExecuteRawDatabaseCommand();
	virtual bool process();
	virtual void presentMainThread();
protected:
	COMPONENT_ID componentID_;
	COMPONENT_TYPE componentType_;
	std::string sdatas_;
	CALLBACK_ID callbackID_;
	std::string error_;
	MemoryStream execret_;
};

}
#endif
