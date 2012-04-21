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
//-------------------------------------------------------------------------------------
Reason PacketFilter::send(NetworkInterface & networkInterface, Channel * pChannel, Packet * pPacket)
{
	return networkInterface.basicSendWithRetries(pChannel, pPacket);
}

//-------------------------------------------------------------------------------------
Reason PacketFilter::recv(Channel * pChannel, PacketReceiver & receiver, Packet * pPacket)
{
	return receiver.processFilteredPacket(pChannel, pPacket);
}

//-------------------------------------------------------------------------------------
} 
}
