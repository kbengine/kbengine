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

#ifndef __BILLING_TASKS_H__
#define __BILLING_TASKS_H__

// common include	
// #define NDEBUG
#include "cstdkbe/cstdkbe.hpp"
#include "cstdkbe/memorystream.hpp"
#include "thread/threadtask.hpp"
#include "helper/debug_helper.hpp"
#include "network/address.hpp"

namespace KBEngine{ 

class BillingTask : public thread::TPTask
{
public:
	BillingTask();
	virtual ~BillingTask();
	
	virtual bool process() = 0;
	
	virtual thread::TPTask::TPTaskState presentMainThread();
	
	std::string commitName;			// 提交时用的名称
	std::string accountName;		// 在游戏服务器数据库中与account绑定的名称
	std::string password;			// 密码
	std::string postDatas;			// 提交的附带数据
	std::string getDatas;			// 返回给客户端的附带数据
	COMPONENT_ID baseappID;
	COMPONENT_ID dbmgrID;
	bool success;

	Mercury::Address address;
};

class CreateAccountTask : public BillingTask
{
public:
	CreateAccountTask();
	virtual ~CreateAccountTask();
	
	virtual bool process();
protected:
};

class LoginAccountTask : public BillingTask
{
public:
	LoginAccountTask();
	virtual ~LoginAccountTask();
	
	virtual bool process();
protected:
};

}
#endif
