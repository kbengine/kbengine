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

#ifndef KBE_SHUTDOWNER_HANDLER_HPP
#define KBE_SHUTDOWNER_HANDLER_HPP

#include "cstdkbe/cstdkbe.hpp"
#include "cstdkbe/timer.hpp"
#include "helper/debug_helper.hpp"
#include "server/shutdown_handler.hpp"

namespace KBEngine { 
namespace Mercury{
	class EventDispatcher;
}

class Shutdowner: public TimerHandler
{
public:
	enum TimeOutType
	{
		TIMEOUT_SHUTDOWN_TICK,
		TIMEOUT_SHUTDOWN_END_TICK,
		TIMEOUT_MAX
	};
	
	Shutdowner(ShutdownHandler* pShutdownHandler);
	virtual ~Shutdowner();
	
	void shutdown(float period, float tickPeriod, Mercury::EventDispatcher& dispatcher);
	void cancel();
protected:
	virtual void handleTimeout(TimerHandle handle, void * arg);

	ShutdownHandler* pShutdownHandler_;
	TimerHandle pTimerHandle_;
	Mercury::EventDispatcher* pDispatcher_;
	float tickPeriod_;

};

}

#endif // KBE_SHUTDOWNER_HANDLER_HPP
