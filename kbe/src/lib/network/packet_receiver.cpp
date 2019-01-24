// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#include "packet_receiver.h"
#ifndef CODE_INLINE
#include "packet_receiver.inl"
#endif

#include "network/address.h"
#include "network/bundle.h"
#include "network/channel.h"
#include "network/endpoint.h"
#include "network/event_dispatcher.h"
#include "network/network_interface.h"
#include "network/event_poller.h"

namespace KBEngine { 
namespace Network
{
//-------------------------------------------------------------------------------------
PacketReceiver::PacketReceiver() :
	pEndpoint_(NULL),
	pChannel_(NULL)
{
}

//-------------------------------------------------------------------------------------
PacketReceiver::PacketReceiver(EndPoint & endpoint,
	   NetworkInterface & networkInterface	) :
	pEndpoint_(&endpoint),
	pChannel_(NULL),
	pNetworkInterface_(&networkInterface)
{
}

//-------------------------------------------------------------------------------------
PacketReceiver::~PacketReceiver()
{
}

//-------------------------------------------------------------------------------------
int PacketReceiver::handleInputNotification(int fd)
{
	if (this->processRecv(true))
	{
		while (this->processRecv(false))
		{
		}
	}

	return 0;
}

//-------------------------------------------------------------------------------------
Reason PacketReceiver::processPacket(Channel* pChannel, Packet * pPacket)
{
	if (pChannel != NULL)
	{
		pChannel->onPacketReceived((int)pPacket->length());

		if (pChannel->pFilter())
		{
			return pChannel->pFilter()->recv(pChannel, *this, pPacket);
		}
	}

	return this->processFilteredPacket(pChannel, pPacket);
}

//-------------------------------------------------------------------------------------
EventDispatcher & PacketReceiver::dispatcher()
{
	return this->pNetworkInterface_->dispatcher();
}

//-------------------------------------------------------------------------------------
Channel* PacketReceiver::getChannel()
{
	if (pChannel_)
	{
		if (pChannel_->isDestroyed())
			return NULL;

		return pChannel_;
	}

	pChannel_ = pNetworkInterface_->findChannel(pEndpoint_->addr());
	return pChannel_;
}

//-------------------------------------------------------------------------------------
}
}
