// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_NETWORKUDPLISTENER_RECEIVER_H
#define KBE_NETWORKUDPLISTENER_RECEIVER_H

#include "common/common.h"
#include "common/timer.h"
#include "helper/debug_helper.h"
#include "network/common.h"
#include "network/interfaces.h"
#include "network/packet.h"
#include "network/channel.h"
#include "network/listener_receiver.h"

namespace KBEngine { 
namespace Network
{
class UDPPacketReceiver;

class ListenerUdpReceiver : public ListenerReceiver
{
public:
	ListenerUdpReceiver(EndPoint & endpoint, Channel::Traits traits, NetworkInterface & networkInterface);
	virtual ~ListenerUdpReceiver();

protected:
	virtual int handleInputNotification(int fd);

protected:
	UDPPacketReceiver* pUDPPacketReceiver_;
};

}
}

#ifdef CODE_INLINE
#include "listener_udp_receiver.inl"
#endif
#endif // KBE_NETWORKUDPLISTENER_RECEIVER_H
