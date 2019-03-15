// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


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
	static SmartPoolObjectPtr createSmartPoolObj(const std::string& logPoint);
	static ObjectPool<TCPPacketReceiver>& ObjPool();
	static TCPPacketReceiver* createPoolObject(const std::string& logPoint);
	static void reclaimPoolObject(TCPPacketReceiver* obj);
	static void destroyObjPool();
	
	TCPPacketReceiver():PacketReceiver(){}
	TCPPacketReceiver(EndPoint & endpoint, NetworkInterface & networkInterface);
	virtual ~TCPPacketReceiver();

	Reason processFilteredPacket(Channel* pChannel, Packet * pPacket);

protected:
	virtual bool processRecv(bool expectingPacket);
	PacketReceiver::RecvState checkSocketErrors(int len, bool expectingPacket);

	virtual void onGetError(Channel* pChannel, const std::string& err);
	
};
}
}

#ifdef CODE_INLINE
#include "tcp_packet_receiver.inl"
#endif
#endif // KBE_NETWORKTCPPACKET_RECEIVER_H
