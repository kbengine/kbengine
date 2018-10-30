// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#include "listener_udp_receiver.h"
#ifndef CODE_INLINE
#include "listener_udp_receiver.inl"
#endif

#include "network/address.h"
#include "network/bundle.h"
#include "network/endpoint.h"
#include "network/event_dispatcher.h"
#include "network/network_interface.h"
#include "network/packet_receiver.h"
#include "network/kcp_packet_receiver.h"
#include "network/error_reporter.h"

#include "network/ikcp.h"

namespace KBEngine { 
namespace Network
{
//-------------------------------------------------------------------------------------
ListenerUdpReceiver::ListenerUdpReceiver(EndPoint & endpoint,
								   Channel::Traits traits, 
									NetworkInterface & networkInterface	):
	ListenerReceiver(endpoint, traits, networkInterface),
	pUDPPacketReceiver_(NULL)
{
	pUDPPacketReceiver_ = new KCPPacketReceiver(endpoint, networkInterface);
}

//-------------------------------------------------------------------------------------
ListenerUdpReceiver::~ListenerUdpReceiver()
{
	SAFE_RELEASE(pUDPPacketReceiver_);
}

//-------------------------------------------------------------------------------------
int ListenerUdpReceiver::handleInputNotification(int fd)
{
	int tickcount = 0;

	while (tickcount++ < 256)
	{
		if (!pUDPPacketReceiver_->processRecv(false))
			return 0;
	}

	return 0;
}

//-------------------------------------------------------------------------------------
}
}
