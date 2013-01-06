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


#include "profile_handler.hpp"
#include "network/network_interface.hpp"
#include "network/event_dispatcher.hpp"
#include "network/address.hpp"
#include "network/bundle.hpp"
#include "pyscript/pyprofile.hpp"
#include "cstdkbe/memorystream.hpp"
#include "helper/console_helper.hpp"

namespace KBEngine { 

KBEUnordered_map<std::string, KBEShared_ptr< ProfileHandler > > ProfileHandler::profiles;

//-------------------------------------------------------------------------------------
ProfileHandler::ProfileHandler(Mercury::NetworkInterface & networkInterface, uint32 timinglen, 
							   std::string name, const Mercury::Address& addr) :
	networkInterface_(networkInterface),
	reportLimitTimerHandle_(),
	name_(name),
	addr_(addr)
{
	profiles[name].reset(this);

	reportLimitTimerHandle_ = networkInterface_.dispatcher().addTimer(
							timinglen * 1000000, this);
}

//-------------------------------------------------------------------------------------
ProfileHandler::~ProfileHandler()
{
	reportLimitTimerHandle_.cancel();
}

//-------------------------------------------------------------------------------------
void ProfileHandler::handleTimeout(TimerHandle handle, void * arg)
{
	KBE_ASSERT(handle == reportLimitTimerHandle_);
	timeout();

	profiles.erase(name_);
}

//-------------------------------------------------------------------------------------
PyProfileHandler::PyProfileHandler(Mercury::NetworkInterface & networkInterface, uint32 timinglen, 
							   std::string name, const Mercury::Address& addr) :
ProfileHandler(networkInterface, timinglen, name, addr)
{
	script::PyProfile::start(name_);
}

//-------------------------------------------------------------------------------------
PyProfileHandler::~PyProfileHandler()
{
	script::PyProfile::remove(name_);
}

//-------------------------------------------------------------------------------------
void PyProfileHandler::timeout()
{
	script::PyProfile::stop(name_);

	MemoryStream s;
	script::PyProfile::addToStream(name_, &s);

	Mercury::Channel* pChannel = networkInterface_.findChannel(addr_);
	if(pChannel == NULL)
	{
		WARNING_MSG(boost::format("PyProfileHandler::timeout: not found %1% addr(%2%)\n") % 
			name_ % addr_.c_str());
		return;
	}

	Mercury::Bundle::SmartPoolObjectPtr bundle = Mercury::Bundle::createSmartPoolObj();

	ConsoleInterface::ConsoleProfileHandler msgHandler;
	(*(*bundle)).newMessage(msgHandler);

	int8 type = 0;
	(*(*bundle)) << type;
	(*(*bundle)).append(&s);
	(*(*bundle)).send(networkInterface_, pChannel);
}

//-------------------------------------------------------------------------------------
CProfileHandler::CProfileHandler(Mercury::NetworkInterface & networkInterface, uint32 timinglen, 
							   std::string name, const Mercury::Address& addr) :
ProfileHandler(networkInterface, timinglen, name, addr)
{
}

//-------------------------------------------------------------------------------------
CProfileHandler::~CProfileHandler()
{
}

//-------------------------------------------------------------------------------------
void CProfileHandler::timeout()
{
}

//-------------------------------------------------------------------------------------
EventProfileHandler::EventProfileHandler(Mercury::NetworkInterface & networkInterface, uint32 timinglen, 
							   std::string name, const Mercury::Address& addr) :
ProfileHandler(networkInterface, timinglen, name, addr)
{
}

//-------------------------------------------------------------------------------------
EventProfileHandler::~EventProfileHandler()
{
}

//-------------------------------------------------------------------------------------
void EventProfileHandler::timeout()
{
}

//-------------------------------------------------------------------------------------

}
