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
	enum PACKET_SENDER_TYPE
	{
		TCP_PACKET_SENDER = 0,
		UDP_PACKET_SENDER = 1
	};

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

	NetworkInterface* pNetworkInterface() const
	{
		return pNetworkInterface_;
	}

	void pNetworkInterface(NetworkInterface* v)
	{
		pNetworkInterface_ = v;
	}

	virtual PACKET_SENDER_TYPE type() const
	{
		return TCP_PACKET_SENDER;
	}

	virtual int handleOutputNotification(int fd);

	virtual Reason processPacket(Channel* pChannel, Packet * pPacket, int userarg);
	virtual Reason processFilterPacket(Channel* pChannel, Packet * pPacket, int userarg) = 0;

	static Reason checkSocketErrors(const EndPoint * pEndpoint);

	virtual Channel* getChannel();

	virtual bool processSend(Channel* pChannel, int userarg) = 0;

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
