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
#include "network/event_dispatcher.hpp"
#include "network/address.hpp"
#include "pyscript/pyprofile.hpp"

namespace KBEngine { 

//-------------------------------------------------------------------------------------
ProfileHandler::ProfileHandler(Mercury::EventDispatcher & dispatcher, uint32 timinglen, std::string name) :
	reportLimitTimerHandle_(),
	name_(name)
{
	reportLimitTimerHandle_ = dispatcher.addTimer(
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
}

//-------------------------------------------------------------------------------------
PyProfileHandler::PyProfileHandler(Mercury::EventDispatcher & dispatcher, uint32 timinglen, std::string name) :
ProfileHandler(dispatcher, timinglen, name)
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
}

//-------------------------------------------------------------------------------------
CProfileHandler::CProfileHandler(Mercury::EventDispatcher & dispatcher, uint32 timinglen, std::string name) :
ProfileHandler(dispatcher, timinglen, name)
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
EventProfileHandler::EventProfileHandler(Mercury::EventDispatcher & dispatcher, uint32 timinglen, std::string name) :
ProfileHandler(dispatcher, timinglen, name)
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
