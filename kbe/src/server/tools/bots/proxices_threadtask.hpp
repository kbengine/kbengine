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

#ifndef __PROXICES_TASKS_H__
#define __PROXICES_TASKS_H__

// common include	
// #define NDEBUG
#include "cstdkbe/cstdkbe.hpp"
#include "cstdkbe/memorystream.hpp"
#include "thread/threadtask.hpp"
#include "helper/debug_helper.hpp"
#include "entitydef/entitydef.hpp"
#include "network/address.hpp"

namespace KBEngine{ 

/*
*/

class ProxicesThreadtask : public thread::TPTask
{
public:
	ProxicesThreadtask(const Mercury::Address& addr);
	virtual ~ProxicesThreadtask();

	bool send(Mercury::Bundle& bundle);

	virtual bool process();
	virtual void presentMainThread(){}

protected:
	MemoryStream s_;
	Mercury::Address addr_;
};


}
#endif
