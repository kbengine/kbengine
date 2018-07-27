// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#include "kcp_packet_receiver_ex.h"
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

namespace KBEngine { 
namespace Network
{

//-------------------------------------------------------------------------------------
KCPPacketReceiverEx::KCPPacketReceiverEx(EndPoint & endpoint,
	   NetworkInterface & networkInterface, ClientObject* pClientObject) :
	KCPPacketReceiver(endpoint, networkInterface),
	pClientObject_(pClientObject)
{
}

//-------------------------------------------------------------------------------------
KCPPacketReceiverEx::~KCPPacketReceiverEx()
{
	//DEBUG_MSG("KCPPacketReceiverEx::~KCPPacketReceiverEx()\n");
}

//-------------------------------------------------------------------------------------
Channel* KCPPacketReceiverEx::getChannel()
{
	return pClientObject_->pServerChannel();
}

//-------------------------------------------------------------------------------------
Channel* KCPPacketReceiverEx::findChannel(const Address& addr)
{
	return pClientObject_->pServerChannel();
}

//-------------------------------------------------------------------------------------
void KCPPacketReceiverEx::onGetError(Channel* pChannel, const std::string& err)
{
	pClientObject_->destroy();
}

//-------------------------------------------------------------------------------------
}
}

