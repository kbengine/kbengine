// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_NETWORKPACKET_SENDER_H
#define KBE_NETWORKPACKET_SENDER_H

#include "common/common.h"
#include "common/timer.h"
#include "helper/debug_helper.h"
#include "network/common.h"
#include "network/interfaces.h"

namespace KBEngine { 
namespace Network
{
class Packet;
class EndPoint;
class Channel;
class Address;
class NetworkInterface;
class EventDispatcher;

class PacketSender : public OutputNotificationHandler, public PoolObject
{
public:
	PacketSender();
	PacketSender(EndPoint & endpoint, NetworkInterface & networkInterface);
	virtual ~PacketSender();

	EventDispatcher& dispatcher();

	void onReclaimObject()
	{
		pEndpoint_ = NULL;
		pChannel_ = NULL;
		pNetworkInterface_ = NULL;
	}

	void pEndPoint(EndPoint* pEndpoint) {
		pChannel_ = NULL;
		pEndpoint_ = pEndpoint; 
	}

	EndPoint* pEndPoint() const { 
		return pEndpoint_; 
	}

	virtual int handleOutputNotification(int fd);

	virtual Reason processPacket(Channel* pChannel, Packet * pPacket);
	virtual Reason processFilterPacket(Channel* pChannel, Packet * pPacket) = 0;

	static Reason checkSocketErrors(const EndPoint * pEndpoint);

	virtual Channel* getChannel();

	virtual bool processSend(Channel* pChannel) = 0;

protected:
	EndPoint* pEndpoint_;
	Channel* pChannel_;
	NetworkInterface* pNetworkInterface_;
};

}
}

#ifdef CODE_INLINE
#include "packet_sender.inl"
#endif
#endif // KBE_NETWORKPACKET_SENDER_H
