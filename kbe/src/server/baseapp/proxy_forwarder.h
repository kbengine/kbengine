/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2017 KBEngine.

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

#ifndef KBE_PROXY_FORWARDER_H
#define KBE_PROXY_FORWARDER_H

#include "helper/debug_helper.h"
#include "common/common.h"

namespace KBEngine{

class ProxyForwarder : public TimerHandler
{
public:
	ProxyForwarder(Proxy * pProxy);
	virtual ~ProxyForwarder();

protected:
	virtual void handleTimeout(TimerHandle, void * arg);
	virtual void onRelease(TimerHandle handle, void  * pUser) {}

protected:
	Proxy * pProxy_;
	TimerHandle timerHandle_;
};

}

#endif // KBE_PROXY_FORWARDER_H
