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


#ifndef __NETWORKTCPPACKET_RECEIVER__
#define __NETWORKTCPPACKET_RECEIVER__

#include "cstdkbe/cstdkbe.hpp"
#include "cstdkbe/timer.hpp"
#include "cstdkbe/objectpool.hpp"
#include "helper/debug_helper.hpp"
#include "network/common.hpp"
#include "network/interfaces.hpp"
#include "network/tcp_packet.hpp"
#include "network/packet_receiver.hpp"

namespace KBEngine { 
namespace Mercury
{
class EndPoint;
class Channel;
class Address;
class NetworkInterface;
class EventDispatcher;

class TCPPacketReceiver : public PacketReceiver
{
public:
	static ObjectPool<TCPPacketReceiver>& ObjPool();
	
	TCPPacketReceiver():PacketReceiver(){}
	TCPPacketReceiver(EndPoint & endpoint, NetworkInterface & networkInterface);
	~TCPPacketReceiver();

	Reason processFilteredPacket(Channel* pChannel, Packet * pPacket);

protected:
	bool processSocket(bool expectingPacket);
	bool checkSocketErrors(int len, bool expectingPacket);
	
};
}
}

#ifdef CODE_INLINE
#include "tcp_packet_receiver.ipp"
#endif
#endif // __NETWORKTCPPACKET_RECEIVER__
