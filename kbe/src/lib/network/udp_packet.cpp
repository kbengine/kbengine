#include "udp_packet.hpp"
#ifndef CODE_INLINE
#include "udp_packet.ipp"
#endif
#include "network/bundle.hpp"
#include "network/endpoint.hpp"
#include "network/network_interface.hpp"
#include "network/message_handler.hpp"

namespace KBEngine { 
namespace Mercury
{
//-------------------------------------------------------------------------------------
UDPPacket::UDPPacket(MessageID msgID, size_t res):
Packet(msgID, res)
{
	data_resize(PACKET_MAX_SIZE_UDP);
	wpos(0);
}

//-------------------------------------------------------------------------------------
UDPPacket::~UDPPacket(void)
{
}

//-------------------------------------------------------------------------------------
int UDPPacket::recvFromEndPoint(EndPoint & ep, Address* pAddr)
{
	int len = ep.recvfrom(data(), PACKET_MAX_SIZE_UDP,
		(u_int16_t*)&pAddr->port, (u_int32_t*)&pAddr->ip);

	wpos(len);
	return len;
}

//-------------------------------------------------------------------------------------
}
}