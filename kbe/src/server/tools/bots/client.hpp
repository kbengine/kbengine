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

#ifndef __CLIENT_TASKS_H__
#define __CLIENT_TASKS_H__

// common include	
// #define NDEBUG
#include "cstdkbe/cstdkbe.hpp"
#include "cstdkbe/memorystream.hpp"
#include "thread/threadtask.hpp"
#include "helper/debug_helper.hpp"
#include "entitydef/entitydef.hpp"
#include "network/address.hpp"
#include "network/endpoint.hpp"
#include "network/bundle.hpp"
#include "pyscript/script.hpp"
#include "pyscript/pyobject_pointer.hpp"

namespace KBEngine{ 

/*
*/

class Client : public thread::TPTask
{
public:
	Client(std::string name);
	virtual ~Client();

	virtual bool process();

	virtual thread::TPTask::TPTaskState presentMainThread(){ 
		return thread::TPTask::TPTASK_STATE_COMPLETED; 
	}

	bool initNetwork();

	bool createAccount();

	bool login();

	const char* name(){ return name_.c_str(); }
protected:
	Mercury::Channel* pChannel_;

	std::string name_;
	std::string password_;

	PyObjectPtr	entryScript_;
};


}
#endif
