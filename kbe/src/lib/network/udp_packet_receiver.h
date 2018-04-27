// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_NETWORKUDPPACKET_RECEIVER_H
#define KBE_NETWORKUDPPACKET_RECEIVER_H

#include "common/common.h"
#include "common/timer.h"
#include "common/objectpool.h"
#include "helper/debug_helper.h"
#include "network/common.h"
#include "network/interfaces.h"
#include "network/udp_packet.h"
#include "network/packet_receiver.h"

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
	static UDPPacketReceiver* createPoolObject();
	static void reclaimPoolObject(UDPPacketReceiver* obj);
	static void destroyObjPool();

	UDPPacketReceiver():PacketReceiver(){}
	UDPPacketReceiver(EndPoint & endpoint, NetworkInterface & networkInterface);
	~UDPPacketReceiver();

	Reason processFilteredPacket(Channel* pChannel, Packet * pPacket);
	
	virtual PacketReceiver::PACKET_RECEIVER_TYPE type() const
	{
		return UDP_PACKET_RECEIVER;
	}

protected:
	bool processRecv(bool expectingPacket);
	PacketReceiver::RecvState checkSocketErrors(int len, bool expectingPacket);

protected:

};

}
}

#ifdef CODE_INLINE
#include "udp_packet_receiver.inl"
#endif
#endif // KBE_NETWORKUDPPACKET_RECEIVER_H
