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

#ifndef __PROFILE_HANDLER__
#define __PROFILE_HANDLER__

#include "cstdkbe/cstdkbe.hpp"
#include "cstdkbe/timer.hpp"
#include "helper/debug_helper.hpp"

namespace KBEngine { 
namespace Mercury
{
class NetworkInterface;
class Address;
}

class ProfileHandler : public TimerHandler
{
public:
	ProfileHandler(Mercury::NetworkInterface & networkInterface, uint32 timinglen, 
		std::string name, const Mercury::Address& addr);
	virtual ~ProfileHandler();
	
	virtual void timeout() = 0;

	static KBEUnordered_map<std::string, KBEShared_ptr< ProfileHandler > > profiles;
protected:
	virtual void handleTimeout(TimerHandle handle, void * arg);

	Mercury::NetworkInterface& networkInterface_;

	TimerHandle reportLimitTimerHandle_;
	
	std::string name_;
	
	Mercury::Address addr_;
};

class PyProfileHandler : public ProfileHandler
{
public:
	PyProfileHandler(Mercury::NetworkInterface & networkInterface, uint32 timinglen, 
		std::string name, const Mercury::Address& addr);
	~PyProfileHandler();
	
	void timeout();
};

class CProfileHandler : public ProfileHandler
{
public:
	CProfileHandler(Mercury::NetworkInterface & networkInterface, uint32 timinglen, 
		std::string name, const Mercury::Address& addr);
	~CProfileHandler();
	
	void timeout();
};

class EventProfileHandler : public ProfileHandler
{
public:
	EventProfileHandler(Mercury::NetworkInterface & networkInterface, uint32 timinglen, 
		std::string name, const Mercury::Address& addr);
	~EventProfileHandler();
	
	void timeout();
};

}

#endif
