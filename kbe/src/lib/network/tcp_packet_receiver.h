/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2018 KBEngine.

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


#ifndef KBE_NETWORKTCPPACKET_RECEIVER_H
#define KBE_NETWORKTCPPACKET_RECEIVER_H

#include "common/common.h"
#include "common/timer.h"
#include "common/objectpool.h"
#include "helper/debug_helper.h"
#include "network/common.h"
#include "network/interfaces.h"
#include "network/tcp_packet.h"
#include "network/packet_receiver.h"

namespace KBEngine { 
namespace Network
{
class EndPoint;
class Channel;
class Address;
class NetworkInterface;
class EventDispatcher;

class TCPPacketReceiver : public PacketReceiver
{
public:
	typedef KBEShared_ptr< SmartPoolObject< TCPPacketReceiver > > SmartPoolObjectPtr;
	static SmartPoolObjectPtr createSmartPoolObj();
	static ObjectPool<TCPPacketReceiver>& ObjPool();
	static TCPPacketReceiver* createPoolObject();
	static void reclaimPoolObject(TCPPacketReceiver* obj);
	static void destroyObjPool();
	
	TCPPacketReceiver():PacketReceiver(){}
	TCPPacketReceiver(EndPoint & endpoint, NetworkInterface & networkInterface);
	~TCPPacketReceiver();

	Reason processFilteredPacket(Channel* pChannel, Packet * pPacket);

protected:
	virtual bool processRecv(bool expectingPacket);
	PacketReceiver::RecvState checkSocketErrors(int len, bool expectingPacket);

	virtual void onGetError(Channel* pChannel);
	
};
}
}

#ifdef CODE_INLINE
#include "tcp_packet_receiver.inl"
#endif
#endif // KBE_NETWORKTCPPACKET_RECEIVER_H
