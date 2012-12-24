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

#ifndef __ANONYMOUS_CHANNEL_H__
#define __ANONYMOUS_CHANNEL_H__

// common include	
// #define NDEBUG
#include "cstdkbe/cstdkbe.hpp"
#include "cstdkbe/memorystream.hpp"
#include "thread/threadtask.hpp"
#include "helper/debug_helper.hpp"
#include "network/address.hpp"
#include "network/endpoint.hpp"

namespace KBEngine{ 

class AnonymousChannel : public thread::TPTask
{
public:
	AnonymousChannel();
	virtual ~AnonymousChannel();
	
	virtual bool process();
	
	virtual thread::TPTask::TPTaskState presentMainThread();

	Mercury::EndPoint listen;

	std::tr1::unordered_map<std::string, std::string> backOrdersDatas_;
};

}
#endif
