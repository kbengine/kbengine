/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2018 KBEngine.

KBEngine is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

KBEngine is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.
 
You should have received a copy of the GNU Lesser General Public License
along with KBEngine.  If not, see <http://www.gnu.org/licenses/>.
*/


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
	pChannel_(NULL),
	pNetworkInterface_(NULL)
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
	if (this->processRecv(/*expectingPacket:*/true))
	{
		while (this->processRecv(/*expectingPacket:*/false))
		{
			/* pass */;
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
