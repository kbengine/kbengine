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
Reason PacketReceiver::processPacket(Channel* pChannel, Packet * pPacket)
{
	if (pChannel != NULL)
	{
		pChannel->onPacketReceived(pPacket->totalSize());

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
	return networkInterface_.dispatcher();
}

//-------------------------------------------------------------------------------------
}
}