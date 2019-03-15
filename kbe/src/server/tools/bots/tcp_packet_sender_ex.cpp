// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#include "tcp_packet_sender_ex.h"
#include "clientobject.h"
#include "bots.h"

#include "network/address.h"
#include "network/bundle.h"
#include "network/channel.h"
#include "network/endpoint.h"
#include "network/event_dispatcher.h"
#include "network/network_interface.h"
#include "network/event_poller.h"
#include "network/error_reporter.h"
#include "network/tcp_packet.h"
#include "network/udp_packet.h"

namespace KBEngine { 
namespace Network
{


//-------------------------------------------------------------------------------------
TCPPacketSenderEx::TCPPacketSenderEx(EndPoint & endpoint,
	   NetworkInterface & networkInterface, ClientObject* pClientObject) :
	TCPPacketSender(endpoint, networkInterface),
	pClientObject_(pClientObject)
{
}

//-------------------------------------------------------------------------------------
TCPPacketSenderEx::~TCPPacketSenderEx()
{
	//DEBUG_MSG("TCPPacketSenderEx::~TCPPacketSenderEx()\n");
}

//-------------------------------------------------------------------------------------
Channel* TCPPacketSenderEx::getChannel()
{
	return pClientObject_->pServerChannel();
}

//-------------------------------------------------------------------------------------
void TCPPacketSenderEx::onGetError(Channel* pChannel, const std::string& err)
{
	pClientObject_->destroy();
}

//-------------------------------------------------------------------------------------
}
}

