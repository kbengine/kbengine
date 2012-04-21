#include "packet_receiver.hpp"
#ifndef CODE_INLINE
#include "packet_receiver.ipp"
#endif

#include "network/address.hpp"
#include "network/bundle.hpp"
#include "network/channel.hpp"
#include "network/endpoint.hpp"
#include "network/event_dispatcher.hpp"
#include "network/network_interface.hpp"
#include "network/event_poller.hpp"
namespace KBEngine { 
namespace Mercury
{
//-------------------------------------------------------------------------------------
PacketReceiver::PacketReceiver(EndPoint & endpoint,
	   NetworkInterface & networkInterface	) :
	endpoint_(endpoint),
	networkInterface_(networkInterface)
{
}

//-------------------------------------------------------------------------------------
PacketReceiver::~PacketReceiver()
{
}

//-------------------------------------------------------------------------------------
int PacketReceiver::handleInputNotification(int fd)
{
	if (this->processSocket(/*expectingPacket:*/true))
	{
		while (this->processSocket(/*expectingPacket:*/false))
		{
			/* pass */;
		}
	}

	return 0;
}

//-------------------------------------------------------------------------------------
Reason PacketReceiver::processPacket(Packet * p)
{
	Channel * pChannel = networkInterface_.findChannel(endpoint_.addr());

	if (pChannel != NULL)
	{
		pChannel->onPacketReceived(p->size());

		if (pChannel->pFilter())
		{
			return pChannel->pFilter()->recv(*this, p);
		}
	}

	return this->processFilteredPacket(p);
}

//-------------------------------------------------------------------------------------
EventDispatcher & PacketReceiver::dispatcher()
{
	return networkInterface_.dispatcher();
}

//-------------------------------------------------------------------------------------
}
}