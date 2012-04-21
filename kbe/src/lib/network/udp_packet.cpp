#include "udp_packet.hpp"
#ifndef CODE_INLINE
#include "udp_packet.ipp"
#endif
#include "network/bundle.hpp"
#include "network/endpoint.hpp"
#include "network/network_interface.hpp"

namespace KBEngine { 
namespace Mercury
{
//-------------------------------------------------------------------------------------
UDPPacket::UDPPacket(PacketHeaders ph, size_t res):
Packet(ph, res)
{
	resize(PACKET_MAX_SIZE_UDP);
}

//-------------------------------------------------------------------------------------
UDPPacket::~UDPPacket(void)
{
}

//-------------------------------------------------------------------------------------
int UDPPacket::recvFromEndPoint(EndPoint & ep)
{
	int len = ep.recv(data(), PACKET_MAX_SIZE_UDP);
	return len;
}

//-------------------------------------------------------------------------------------
}
}