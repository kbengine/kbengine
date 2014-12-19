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

#ifndef KBE_ANONYMOUS_CHANNEL_HPP
#define KBE_ANONYMOUS_CHANNEL_HPP

// common include	
// #define NDEBUG
#include "common/common.h"
#include "common/memorystream.h"
#include "thread/threadtask.h"
#include "helper/debug_helper.h"
#include "network/address.h"
#include "network/endpoint.h"

namespace KBEngine{ 

class AnonymousChannel : public thread::TPTask
{
public:
	AnonymousChannel();
	virtual ~AnonymousChannel();
	
	virtual bool process();
	
	virtual thread::TPTask::TPTaskState presentMainThread();

	Network::EndPoint listen;

	struct BACK_ORDERS_DATA
	{
		std::string data;
	};

	KBEUnordered_map<std::string, BACK_ORDERS_DATA> backOrdersDatas_;
};

}

#endif // KBE_ANONYMOUS_CHANNEL_HPP
