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

#ifndef __LOGINAPPTHREADTASKS_H__
#define __LOGINAPPTHREADTASKS_H__

#include "cstdkbe/cstdkbe.hpp"
#include "cstdkbe/memorystream.hpp"
#include "cstdkbe/timestamp.hpp"
#include "thread/threadtask.hpp"
#include "helper/debug_helper.hpp"
#include "entitydef/entitydef.hpp"
#include "network/address.hpp"

namespace KBEngine{ 


/*
	账号激活邮件发送线程任务
*/

class SendActivateEMailTask : public thread::TPTask
{
public:
	SendActivateEMailTask(const std::string& emailaddr, const std::string& code, const std::string& cbaddr, uint32 cbport):
	emailaddr_(emailaddr),
	code_(code),
	cbaddr_(cbaddr),
	cbport_(cbport)
	{
	}

	virtual ~SendActivateEMailTask();
	virtual bool process();
	virtual thread::TPTask::TPTaskState presentMainThread();

protected:
	std::string emailaddr_, code_, cbaddr_;
	uint32 cbport_;
};

}
#endif
