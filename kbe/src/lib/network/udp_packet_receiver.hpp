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

#ifndef KBE_NETWORKUDPPACKET_RECEIVER_HPP
#define KBE_NETWORKUDPPACKET_RECEIVER_HPP

#include "cstdkbe/cstdkbe.hpp"
#include "cstdkbe/timer.hpp"
#include "cstdkbe/objectpool.hpp"
#include "helper/debug_helper.hpp"
#include "network/common.hpp"
#include "network/interfaces.hpp"
#include "network/udp_packet.hpp"
#include "network/packet_receiver.hpp"

namespace KBEngine { 
namespace Network
{
class Socket;
class Channel;
class Address;
class NetworkInterface;
class EventDispatcher;

class UDPPacketReceiver : public PacketReceiver
{
public:
	typedef KBEShared_ptr< SmartPoolObject< UDPPacketReceiver > > SmartPoolObjectPtr;
	static SmartPoolObjectPtr createSmartPoolObj();
	static ObjectPool<UDPPacketReceiver>& ObjPool();
	static void destroyObjPool();

	UDPPacketReceiver():PacketReceiver(){}
	UDPPacketReceiver(EndPoint & endpoint, NetworkInterface & networkInterface);
	~UDPPacketReceiver();

	Reason processFilteredPacket(Channel* pChannel, Packet * pPacket);
	
protected:
	bool processSocket(bool expectingPacket);
	PacketReceiver::RecvState checkSocketErrors(int len, bool expectingPacket);
protected:

};

}
}

#ifdef CODE_INLINE
#include "udp_packet_receiver.ipp"
#endif
#endif // KBE_NETWORKUDPPACKET_RECEIVER_HPP
