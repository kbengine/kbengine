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

#ifndef __NETWORKPACKET_RECEIVER__
#define __NETWORKPACKET_RECEIVER__

#include "cstdkbe/cstdkbe.hpp"
#include "cstdkbe/objectpool.hpp"
#include "cstdkbe/timer.hpp"
#include "helper/debug_helper.hpp"
#include "network/common.hpp"
#include "network/interfaces.hpp"
#include "network/tcp_packet.hpp"

namespace KBEngine { 
namespace Mercury
{
class EndPoint;
class Channel;
class Address;
class NetworkInterface;
class EventDispatcher;

class PacketReceiver : public InputNotificationHandler, public PoolObject
{
public:
	enum RecvState
	{
		RECV_STATE_INTERRUPT = -1,
		RECV_STATE_BREAK = 0,
		RECV_STATE_CONTINUE = 1
	};

	PacketReceiver();
	PacketReceiver(EndPoint & endpoint, NetworkInterface & networkInterface);
	virtual ~PacketReceiver();

	virtual Reason processPacket(Channel* pChannel, Packet * pPacket);
	virtual Reason processFilteredPacket(Channel* pChannel, Packet * pPacket) = 0;
	EventDispatcher& dispatcher();

	void onReclaimObject()
	{
		pEndpoint_ = NULL;
		pNetworkInterface_ = NULL;
	}

	void endpoint(EndPoint* pEndpoint){ 
		pEndpoint_ = pEndpoint; 
	}

	virtual int handleInputNotification(int fd);
protected:
	virtual bool processSocket(bool expectingPacket) = 0;
	virtual RecvState checkSocketErrors(int len, bool expectingPacket) = 0;
protected:
	EndPoint* pEndpoint_;
	NetworkInterface* pNetworkInterface_;
};

}
}

#ifdef CODE_INLINE
#include "packet_receiver.ipp"
#endif
#endif // __NETWORKPACKET_RECEIVER__
