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
UDPPacket::UDPPacket()
{
}

//-------------------------------------------------------------------------------------
UDPPacket::~UDPPacket(void)
{
}

//-------------------------------------------------------------------------------------
int UDPPacket::recvFromEndPoint(EndPoint & ep)
{
	int len = ep.recv(data_, UDP_PACKET_MAX_SIZE);

	if (len >= 0)
	{
		this->msgEndOffset(len);
	}
	return len;
}

//-------------------------------------------------------------------------------------
}
}