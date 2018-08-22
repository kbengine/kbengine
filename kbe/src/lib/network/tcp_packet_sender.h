// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#ifndef KBE_NETWORKTCPPACKET_SENDER_H
#define KBE_NETWORKTCPPACKET_SENDER_H

#include "common/common.h"
#include "common/timer.h"
#include "common/objectpool.h"
#include "helper/debug_helper.h"
#include "network/common.h"
#include "network/interfaces.h"
#include "network/tcp_packet.h"
#include "network/packet_sender.h"

namespace KBEngine { 
namespace Network
{
class EndPoint;
class Channel;
class Address;
class NetworkInterface;
class EventDispatcher;

class TCPPacketSender : public PacketSender
{
public:
	typedef KBEShared_ptr< SmartPoolObject< TCPPacketSender > > SmartPoolObjectPtr;
	static SmartPoolObjectPtr createSmartPoolObj(const std::string& logPoint);
	static ObjectPool<TCPPacketSender>& ObjPool();
	static TCPPacketSender* createPoolObject(const std::string& logPoint);
	static void reclaimPoolObject(TCPPacketSender* obj);
	virtual void onReclaimObject();
	static void destroyObjPool();
	
	TCPPacketSender():PacketSender(){}
	TCPPacketSender(EndPoint & endpoint, NetworkInterface & networkInterface);
	virtual ~TCPPacketSender();

	virtual void onGetError(Channel* pChannel, const std::string& err);
	virtual bool processSend(Channel* pChannel, int userarg);

	virtual PacketSender::PACKET_SENDER_TYPE type() const
	{
		return TCP_PACKET_SENDER;
	}

protected:
	virtual Reason processFilterPacket(Channel* pChannel, Packet * pPacket, int userarg);

	uint8 sendfailCount_;
};
}
}

#ifdef CODE_INLINE
#include "tcp_packet_sender.inl"
#endif
#endif // KBE_NETWORKTCPPACKET_SENDER_H
