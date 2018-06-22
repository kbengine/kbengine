// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_NETWORKPACKET_RECEIVER_H
#define KBE_NETWORKPACKET_RECEIVER_H

#include "common/common.h"
#include "common/objectpool.h"
#include "common/timer.h"
#include "helper/debug_helper.h"
#include "network/common.h"
#include "network/interfaces.h"
#include "network/tcp_packet.h"

namespace KBEngine { 
namespace Network
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

	enum PACKET_RECEIVER_TYPE
	{
		TCP_PACKET_RECEIVER = 0,
		UDP_PACKET_RECEIVER = 1
	};

public:
	PacketReceiver();
	PacketReceiver(EndPoint & endpoint, NetworkInterface & networkInterface);
	virtual ~PacketReceiver();

	virtual Reason processPacket(Channel* pChannel, Packet * pPacket);
	virtual Reason processFilteredPacket(Channel* pChannel, Packet * pPacket) = 0;
	EventDispatcher& dispatcher();

	void onReclaimObject()
	{
		pEndpoint_ = NULL;
		pChannel_ = NULL;
	}

	virtual PacketReceiver::PACKET_RECEIVER_TYPE type() const
	{
		return TCP_PACKET_RECEIVER;
	}

	void pEndPoint(EndPoint* pEndpoint) { 
		pEndpoint_ = pEndpoint; 
		pChannel_ = NULL;
	}

	EndPoint* pEndPoint() const { 
		return pEndpoint_; 
	}

	virtual int handleInputNotification(int fd);

	virtual Channel* getChannel();

protected:
	virtual bool processRecv(bool expectingPacket) = 0;
	virtual RecvState checkSocketErrors(int len, bool expectingPacket) = 0;

protected:
	EndPoint* pEndpoint_;
	Channel* pChannel_;
	NetworkInterface* pNetworkInterface_;
};

}
}

#ifdef CODE_INLINE
#include "packet_receiver.inl"
#endif
#endif // KBE_NETWORKPACKET_RECEIVER_H
