#include "packet_filter.hpp"

#ifndef CODE_INLINE
#include "packet_filter.ipp"
#endif

#include "network/channel.hpp"
#include "network/network_interface.hpp"
#include "network/packet_receiver.hpp"

namespace KBEngine { 
namespace Mercury
{
	
Reason PacketFilter::send(NetworkInterface & networkInterface,
		const Address & addr, Packet * pPacket)
{
	return networkInterface.basicSendWithRetries(addr, pPacket);
}

Reason PacketFilter::recv(PacketReceiver & receiver,
							const Address & addr, Packet * pPacket)
{
	return receiver.processFilteredPacket(addr, pPacket);
}

} 
}
