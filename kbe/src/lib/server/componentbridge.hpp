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

#ifndef __COMPONENT_BRIDEGE__
#define __COMPONENT_BRIDEGE__

#include "cstdkbe/cstdkbe.hpp"
#include "cstdkbe/tasks.hpp"
#include "cstdkbe/singleton.hpp"
#include "helper/debug_helper.hpp"
#include "network/common.hpp"
#include "network/interfaces.hpp"
#include "network/endpoint.hpp"
#include "network/udp_packet_receiver.hpp"
#include "server/components.hpp"

namespace KBEngine { 
namespace Mercury
{
class Address;
class NetworkInterface;
class EventDispatcher;
class UDPPacket;
}

/*
	这个模块被定义为“引擎组件桥”， 组件就是cellapp或者dbmgr等。
	它的职责主要是让当前app组件能够比较方便的与其他组件进行交互。
*/
class Componentbridge : public Task, 
						public Mercury::UDPPacketReceiver,
						public Singleton<Componentbridge>
{
public:
	Componentbridge(Mercury::NetworkInterface & networkInterface, 
			COMPONENT_TYPE componentType, COMPONENT_ID componentID);
	~Componentbridge();

	static Components& getComponents();
private:
	virtual bool process();
	Mercury:: EventDispatcher & dispatcher();

	bool findInterfaces();
private:
	Mercury::EndPoint epBroadcast_;
	Mercury::NetworkInterface & networkInterface_;
	COMPONENT_TYPE componentType_;
	COMPONENT_ID componentID_;									// 本组件的ID
	int8 broadcastCount_;
};

}

#endif // __COMPONENT_BRIDEGE__